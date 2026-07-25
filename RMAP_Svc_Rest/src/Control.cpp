/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVRest_cpp : Control.cpp                                                     **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_REST;

static const std::map<std::string, const CLIENT::ACTION*> g_aAction_Control;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

size_t WriteCallback (void* contents, size_t size, size_t nmemb, void* userp)
{
   ((std::string*)userp)->append ((char*)contents, size * nmemb);
   return size * nmemb;
}

class CONTROL::Impl : public RMAP::CORE::IRESPONSE
{
public:
   Impl (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings) :
      pClient (pClient),
      m_nTimeout (10),
      m_bShutdown (false)
   {
      // Control
      bError   = false;
      dwResult  = SBA_RESULT_SUCCESS;

      // Login
      pSourceREST          = NULL;
      pParams              = NULL;

      pLogin               = NULL;
      bLoggedIn            = false;

      m_sEndPoint = (pNetSettings->bSecure ? "https://" : "http://") + pNetSettings->sHost;
      if (pNetSettings->wPort != 0)
         m_sEndPoint += ":" + std::to_string (pNetSettings->wPort);
      if (m_sEndPoint.back () != '/')
         m_sEndPoint += "/";

      m_pThread = new std::thread (&CONTROL::Impl::ThreadLoop, this);
   }

   ~Impl ()
   {
      Shutdown ();
      m_pThread->join ();

      delete m_pThread;
   }

   void Shutdown ()
   {
      std::lock_guard<std::recursive_mutex> guard (m_mutex);
      m_bShutdown = true;
      m_condVar.notify_all ();
   }

   void ThreadLoop ()
   {
      std::unique_lock<std::recursive_mutex> mlock (m_mutex);
      m_condVar.wait (mlock, std::bind (&CONTROL::Impl::Control, this));
   }

   void Send (CLIENT::IACTION* pIAction)
   {
      CURL* curl;
      CURLcode res;
      std::string sResult;
      std::string sBuffer;
      CLIENT::ICODEC* pICodec = pIAction->GetCodec ();

      curl = curl_easy_init ();

      if (curl)
      {
         struct curl_slist* slist1 = NULL;

         for (auto const& x : pIAction->Payload.aHeaders)
         {
            sBuffer = x.first + ": " + x.second;

            slist1 = curl_slist_append (slist1, sBuffer.c_str ());
         }

         if (slist1 != NULL)
            curl_easy_setopt (curl, CURLOPT_HTTPHEADER, slist1);

//            if (pOptions.sQuery)
//               sURL += '?' + pOptions.sQuery;
         curl_easy_setopt (curl, CURLOPT_URL, pIAction->Payload.sEndPoint.c_str ());
         curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0L);   // skip the verification of the server's certificate
         //curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0L);  // SKIP_HOSTNAME_VERIFICATION

         if (pIAction->Payload.kMethod == CLIENT::IACTION::eMETHOD::POST)
         {
            curl_easy_setopt (curl, CURLOPT_POSTFIELDS, pIAction->Payload.sBody.c_str ());
         }

         /* cache the CA cert bundle in memory for a week */
         curl_easy_setopt (curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);

         curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, WriteCallback);
         curl_easy_setopt (curl, CURLOPT_WRITEDATA, &sResult);

         res = curl_easy_perform (curl);

         pClient->Lock ();
         {
            if (pIAction->Status () == CLIENT::IACTION::kSTATUS_CODE_OK)
            {
               if (res == CURLE_OK)
               {
                  pICodec->Decode (sResult);
               }
               else pICodec->Error (curl_easy_strerror (res));
            }
         }
         pClient->Unlock ();

         curl_easy_cleanup (curl);
      }
   }

   bool Control ()
   {
      bool bContinue = true;
      CLIENT::IACTION* pIAction;

      if (m_bShutdown == false)
      {
         do
         {
            m_CS_Control.lock ();
            {
               if (m_apIAction.empty () == false)
               {
                  pIAction = m_apIAction.front ();
               }
               else pIAction = NULL;
            }
            m_CS_Control.unlock ();

            if (pIAction != NULL)
            {
               Send (pIAction);

               pClient->IAction_Remove (pIAction);
               delete pIAction;

               m_CS_Control.lock ();
               {
                  m_apIAction.pop ();
               }
               m_CS_Control.unlock ();
            }

            m_CS_Control.lock ();
            {
               bContinue = (m_apIAction.empty () == false && m_bShutdown == false);
            }
            m_CS_Control.unlock ();
         } 
         while (bContinue);
      }

      return m_bShutdown;
   }

   int ReadyState ()
   {
      return pClient->ReadyState ();
   }

   int ReadyState (int nState)
   {
      return pClient->ReadyState (nState);
   }

   void Progress (PROGRESS* pProgress)
   {
      pClient->Progress (pProgress);
   }

   bool Login_Request (bool bVoluntary)
   {
      bool bExit = false;
      PROGRESS ProgressREST;

      bVoluntary = (bLoggedIn == false);

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressREST.nProgress = CLIENT::ePROGRESS::LOGIN_ATTEMPT;
         ProgressREST.bVoluntary = bVoluntary;

         Progress (&ProgressREST);

         RMAP::CORE::CLIENT::IACTION* pIAction = pSourceREST->Login_Request (pParams, pLogin);
         if (pIAction != NULL)
         {
            CLIENT::IACTION* pIActionREST = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIActionREST->Send (this, CONTROL::kLOGIN_RESPONSE, RMAP::CORE::CLIENT::SetVDParam (bVoluntary, false, false)) != false)
            {
               bExit = true;
            }
            else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR

         if (bExit == false)
         {
            Login_Exit (NULL, bVoluntary);
         }
      }
      else
      {
         pSourceREST = NULL;
         pParams = NULL;
      }

      return bExit;
   }

   void Login_Response (CLIENT::IACTION* pIAction, bool bVoluntary)
   {
      dwResult = pIAction->IsSuccess () ? SBA_RESULT_SUCCESS : SBA_RESULT_TRANSMITFAILURE;

      Login_Exit (pIAction, bVoluntary);
   }

   void Login_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary)
   {
      PROGRESS ProgressREST;

      if (pIAction && dwResult == SBA_RESULT_SUCCESS)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceREST->Login_Create ();
         }

         // A dwResult of SBA_RESULT_SUCCESS only tells us that the action completed without error.
         // We have to wait for the source to tell us whether or not the login completed successfully.

         if (pSourceREST->Login_Response (pParams, pLogin, pIAction, bVoluntary) != false)
         {
            bLoggedIn = true;

            ReadyState (CLIENT::eSTATE::LOGGEDIN);
         }
         else
         {
            bError = true;

            ReadyState (CLIENT::eSTATE::LOGGEDOUT);
         }
      }
      else
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }

      ProgressREST.nProgress    = CLIENT::ePROGRESS::LOGIN_RESULT;
      ProgressREST.bVoluntary   = bVoluntary;
      ProgressREST.dwResult     = dwResult;
      ProgressREST.pLogin       = pLogin;
      ProgressREST.bResult      = !bError;
      Progress (&ProgressREST);

      if (bError != false && pLogin)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceREST->Login_Destroy (pLogin);
         }
      }

      pSourceREST = NULL;
      pParams     = NULL;
   }

   bool Logout_Request (bool bVoluntary, bool bDisconnected)
   {
      PROGRESS ProgressREST;
      bool bExit = false;

      // If the logout is involuntary, this function must complete all processing on the request and may not cause the current thread to exit.

      if (ReadyState () == CLIENT::eSTATE::LOGGEDIN)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressREST.nProgress = CLIENT::ePROGRESS::LOGOUT_ATTEMPT;
         ProgressREST.bVoluntary = bVoluntary;
         ProgressREST.bDisconnected = bDisconnected;
         Progress (&ProgressREST);

         if (bVoluntary != false && !bDisconnected)
         {
            RMAP::CORE::CLIENT::IACTION* pIAction = pSourceREST->Logout_Request (pParams, pLogin);
            CLIENT::IACTION* pIActionREST = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIAction)
            {
               if (pIActionREST->Send (this, CONTROL::kLOGOUT_RESPONSE, RMAP::CORE::CLIENT::SetVDParam (bVoluntary, false, bDisconnected)) != false)
               {
                  bExit = true;
               }
               else dwResult = SBA_RESULT_TRANSMITFAILURE;
            }
            else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR
         }
         else dwResult = SBA_RESULT_SUCCESS;

         if (bExit == false)
         {
            Logout_Exit (NULL, bVoluntary, bDisconnected);
         }
      }
      else
      {
         pSourceREST = NULL;
         pParams = NULL;
      }

      return bExit;
   }

   void Logout_Response (CLIENT::IACTION* pIAction, intptr_t pVD)
   {
      dwResult = pIAction->IsSuccess () ? SBA_RESULT_SUCCESS : SBA_RESULT_TRANSMITFAILURE;

      Logout_Exit (pIAction, RMAP::CORE::CLIENT::GetVDParam (pVD, MV_VDPARAM_VOLUNTARY), RMAP::CORE::CLIENT::GetVDParam (pVD, MV_VDPARAM_DISCONNECTED));
   }

   void Logout_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
   {
      PROGRESS ProgressREST;

      dwResult = SBA_RESULT_SUCCESS;  // There"s no such thing as a failure to logout!

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         if (pIAction)
         {
            pSourceREST->Logout_Response (pParams, pLogin, pIAction, bVoluntary, bDisconnected);
         }

         bLoggedIn = (bLoggedIn != false && bVoluntary == false);

         if (bLoggedIn == false)
         {
            pLogin = pSourceREST->Login_Destroy (pLogin);
         }

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }
      else // This will never occur
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDIN);
      }

      ProgressREST.nProgress = CLIENT::ePROGRESS::LOGOUT_RESULT;
      ProgressREST.bVoluntary = bVoluntary;
      ProgressREST.bDisconnected = bDisconnected;
      ProgressREST.dwResult = dwResult;
      ProgressREST.pLogin = pLogin;
      ProgressREST.bResult = !bError;

      Progress (&ProgressREST);

      pSourceREST = NULL;
      pParams = NULL;
   }

   bool ClearError ()
   {
      bool bResult = bError;

      bError = false;

      return bResult;
   }

   bool SafeKill ()
   {
      bool bResult;

      m_CS_Control.lock ();
      {
         bResult = m_apIAction.empty ();
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   /*******************************************************************************************************************************
   **                                                   Client                                                                   **
   *******************************************************************************************************************************/

   bool Login (RMAP::CORE::SOURCE* pSource, void* pvParams)
   {
      bool bResult = false;

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         if (bLoggedIn == false && pvParams != NULL
            || bLoggedIn != false && pLogin != NULL)
         {
            pSourceREST = dynamic_cast<REST_SESSION*> (pSource);
            pParams     = pvParams;

            bResult = Login_Request (true);
         }
      }

      return bResult;
   }


   bool Logout (RMAP::CORE::SOURCE* pSource, void* pvParams)
   {
      bool bResult = false;

      if (ReadyState () == CLIENT::eSTATE::LOGGEDIN)
      {
         pSourceREST = dynamic_cast<REST_SESSION*> (pSource);
         pParams     = pvParams;

         bResult = Logout_Request (true, false);
      }

      return bResult;
   }

   /*******************************************************************************************************************************
   **                                                   ICONTROL                                                                 **
   *******************************************************************************************************************************/

   void onResponse (RMAP::CORE::CLIENT::IACTION* pIAction, int nType, intptr_t pParam) override
   {
      CLIENT::IACTION* pIActionREST = dynamic_cast<CLIENT::IACTION*> (pIAction);

      switch (nType)
      {
      case kLOGIN_RESPONSE:                Login_Response            (pIActionREST, RMAP::CORE::CLIENT::GetVDParam (pParam, MV_VDPARAM_VOLUNTARY)); break;
      case kLOGOUT_RESPONSE:               Logout_Response           (pIActionREST, pParam);                                                         break;
      }
   }

   void QueueAction (CLIENT::IACTION* pIAction)
   {
      m_CS_Control.lock ();
      {
         m_apIAction.push (pIAction);
      }
      m_CS_Control.unlock ();

      CtlBreak_Thread ();
   }

public:

   CLIENT*                             pClient;
   REST_SESSION*                       pSourceREST;

   std::string                         m_sEndPoint;


   // Control
   bool                                bError;
   uint32_t                            dwResult;

   // Login
   void*                               pParams;
   RMAP::CORE::SOURCE_SESSION::LOGIN*     pLogin;
   bool                                bLoggedIn;

private:
   void CtlBreak_Thread ()
   {
      std::lock_guard<std::recursive_mutex> guard (m_mutex);
      m_condVar.notify_all ();
   }

   // --------------------------------------------------------------------------------------------------------------------------

   RMAP::CORE::CLIENT::IACTION* Request (std::string sAction)
   {
      RMAP::CORE::CLIENT::IACTION* pResult;

      auto j = g_aAction_Control.find (sAction);

      if (j != g_aAction_Control.end ())
      {
         pResult = pClient->Request (const_cast<CLIENT::ACTION *> (j->second));
      }
      else pResult = NULL;

      return pResult;
   }

private:
   std::thread*                m_pThread;
   std::recursive_mutex        m_mutex;
   std::condition_variable_any m_condVar;
   bool                        m_bShutdown;

   std::recursive_mutex             m_CS_Control;
   std::queue<CLIENT::IACTION*>     m_apIAction;

   int                     m_nTimeout;
};

/*******************************************************************************************************************************
**                                                   CLASS (CONTROL)                                                          **
*******************************************************************************************************************************/

CONTROL::CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings)
{
   m_pImpl = new CONTROL::Impl (pClient, pNetSettings);
}

CONTROL::~CONTROL ()
{
   delete m_pImpl;
}

bool CONTROL::bLoggedIn ()
{
   return m_pImpl->bLoggedIn;
}

RMAP::CORE::SOURCE_SESSION::LOGIN* CONTROL::pLogin ()
{
   return m_pImpl->pLogin;
}

std::string& CONTROL::sEndPoint ()
{
   return m_pImpl->m_sEndPoint;
}


bool CONTROL::SafeKill ()
{
   return m_pImpl->SafeKill ();
}

bool CONTROL::ClearError ()
{
   return m_pImpl->ClearError ();
}

bool CONTROL::Login (RMAP::CORE::SOURCE* pSource, void* pParams)  { return m_pImpl->Login (pSource, pParams);        }
bool CONTROL::Logout (RMAP::CORE::SOURCE* pSource, void* pParams) { return m_pImpl->Logout (pSource, pParams);       }

void CONTROL::QueueAction (CLIENT::IACTION* pIAction)
{
   m_pImpl->QueueAction (pIAction);
}

/******************************************************************************************************************************/
