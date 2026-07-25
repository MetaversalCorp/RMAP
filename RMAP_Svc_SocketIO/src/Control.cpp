/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVIO_cpp : Control.cpp                                                     **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

static const std::map<std::string, const CLIENT::ACTION*> g_aAction_Control;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class CONTROL::Impl : public RMAP::CORE::IRESPONSE, public NET::ICONTROL
{
public:
   Impl (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription) :
      pClient (pClient),
      pSubscription (pSubscription),
      bNetConnected (false),
      m_nTimeout (10)
   {
      // Control
      bError   = false;
      dwResult  = SBA_RESULT_SUCCESS;

      // Login
      pSourceIO            = NULL;
      pParams              = NULL;

      pLogin               = NULL;
      bLoggedIn            = false;

      bSocketConnected     = false;

      sEndPoint = (pNetSettings->bSecure ? "wss" : "ws");
      sEndPoint += "://" + pNetSettings->sHost + ":" + std::to_string (pNetSettings->wPort);
   }

   ~Impl ()
   {
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
      PROGRESS ProgressIO;

      bVoluntary = (bLoggedIn == false);

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressIO.nProgress = CLIENT::ePROGRESS::LOGIN_ATTEMPT;
         ProgressIO.bVoluntary = bVoluntary;

         Progress (&ProgressIO);

         RMAP::CORE::CLIENT::IACTION* pIAction = pSourceIO->Login_Request (pParams, pLogin);
         if (pIAction != NULL)
         {
            CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIActionIO->Send (this, CONTROL::kLOGIN_RESPONSE, RMAP::CORE::CLIENT::SetVDParam (bVoluntary, false, false)) != false)
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
         pSourceIO = NULL;
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
      PROGRESS ProgressIO;

      if (pIAction && dwResult == SBA_RESULT_SUCCESS)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceIO->Login_Create ();
         }

         // A wResult of SBA_RESULT_SUCCESS only tells us that the action completed without error.
         // We have to wait for the source to tell us whether or not the login completed successfully.

         if (pSourceIO->Login_Response (pParams, pLogin, pIAction, bVoluntary) != false)
         {
            bLoggedIn = true;

            ReadyState (CLIENT::eSTATE::LOGGEDIN);

            if (bVoluntary == false)
               pSubscription->Subscribe_Aux ();
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

      ProgressIO.nProgress    = CLIENT::ePROGRESS::LOGIN_RESULT;
      ProgressIO.bVoluntary   = bVoluntary;
      ProgressIO.dwResult     = dwResult;
      ProgressIO.pLogin       = pLogin;
      ProgressIO.bResult      = !bError;
      Progress (&ProgressIO);

      if (bError != false && pLogin)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceIO->Login_Destroy (pLogin);
         }
      }

      pSourceIO = NULL;
      pParams     = NULL;
   }

   bool Logout_Request (bool bVoluntary, bool bDisconnected)
   {
      PROGRESS ProgressIO;
      bool bExit = false;

      // If the logout is involuntary, this function must complete all processing on the request and may not cause the current thread to exit.

      if (ReadyState () == CLIENT::eSTATE::LOGGEDIN)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressIO.nProgress = CLIENT::ePROGRESS::LOGOUT_ATTEMPT;
         ProgressIO.bVoluntary = bVoluntary;
         ProgressIO.bDisconnected = bDisconnected;
         Progress (&ProgressIO);

         if (bVoluntary != false && !bDisconnected)
         {
            RMAP::CORE::CLIENT::IACTION* pIAction = pSourceIO->Logout_Request (pParams, pLogin);
            CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIAction)
            {
               if (pIActionIO->Send (this, CONTROL::kLOGOUT_RESPONSE, RMAP::CORE::CLIENT::SetVDParam (bVoluntary, false, bDisconnected)) != false)
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
         pSourceIO = NULL;
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
      PROGRESS ProgressIO;

      dwResult = SBA_RESULT_SUCCESS;  // There"s no such thing as a failure to logout!

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         if (pIAction)
         {
            pSourceIO->Logout_Response (pParams, pLogin, pIAction, bVoluntary, bDisconnected);
         }

         bLoggedIn = (bLoggedIn != false && bVoluntary == false);

         if (bLoggedIn == false)
         {
            pLogin = pSourceIO->Login_Destroy (pLogin);
         }

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }
      else // This will never occur
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDIN);
      }

      ProgressIO.nProgress = CLIENT::ePROGRESS::LOGOUT_RESULT;
      ProgressIO.bVoluntary = bVoluntary;
      ProgressIO.bDisconnected = bDisconnected;
      ProgressIO.dwResult = dwResult;
      ProgressIO.pLogin = pLogin;
      ProgressIO.bResult = !bError;

      Progress (&ProgressIO);

      pSourceIO = NULL;
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

      bResult = (bNetConnected == false);

      return bResult;
   }

   bool SocketConnect_Attempt (bool bVoluntary)
   {
      bool bExit = false;
      PROGRESS ProgressIO;

      bVoluntary = (bSocketConnected == false);

      if (pClient->ReadyState () == CLIENT::eSTATE::SOCKETDISCONNECTED)
      {
         ClearError ();

         pClient->ReadyState (CLIENT::eSTATE::SOCKETCONNECTING);

         ProgressIO.nProgress = CLIENT::ePROGRESS::SOCKETCONNECT_ATTEMPT;
         ProgressIO.bVoluntary = bVoluntary;

         pClient->Progress (&ProgressIO);

         // this.#Progress ({ nProgress: this.#ePROGRESS.SOCKET_CONNECTING,     bVoluntary });   // is this necessary

         if (pClient->pNet ()->Connect (sEndPoint, this, bVoluntary) != false)
         {
            bExit = true;
         }
//         else this.#dwResult = this.IO_RESULT_TRANSMITFAILURE;

         if (bExit == false)
         {
            SocketConnect_Exit (false, bVoluntary);
         }
      }

      return bExit;
   }

   void SocketConnect_Complete (bool bConnected, bool bVoluntary) override
   {
/*
      if (bConnected != false)
      {
         this.#dwResult = this.IO_RESULT_SUCCESS;
      }
      else this.#dwResult = this.IO_RESULT_TRANSMITFAILURE;   /// wrong error
*/
      SocketConnect_Exit (bConnected, bVoluntary);
   }

   void SocketConnect_Exit (bool bConnected, bool bVoluntary)
   {
      PROGRESS ProgressIO;

      if (bConnected != false)  //      if (this.#dwResult == this.IO_RESULT_SUCCESS)
      {
         bNetConnected = true;

         bSocketConnected = true;

         // this.#Progress ({ nProgress: this.#ePROGRESS.SOCKET_CONNECTED, bVoluntary });   // is this necessary

         pClient->ReadyState (CLIENT::eSTATE::LOGGEDOUT);

         if (bVoluntary == false && bLoggedIn == false)
            pSubscription->Subscribe_Aux ();
      }
      else
      {
         bError = true;
         pClient->ReadyState (CLIENT::eSTATE::SOCKETDISCONNECTED);
      }

      ProgressIO.nProgress = CLIENT::ePROGRESS::SOCKETCONNECT_RESULT;
      ProgressIO.bVoluntary = bVoluntary;
      ProgressIO.dwResult = dwResult;
      ProgressIO.bResult = !bError;

      Progress (&ProgressIO);
   }

   bool SocketDisconnect_Attempt (bool bVoluntary, bool bDisconnected)
   {
      bool bExit = false;
      PROGRESS ProgressIO;

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::SOCKETCONNECTING); // SOCKETDISCONNECTING

         ProgressIO.nProgress = CLIENT::ePROGRESS::SOCKETDISCONNECT_ATTEMPT;
         ProgressIO.bVoluntary = bVoluntary;
         ProgressIO.bDisconnected = bDisconnected;

         Progress (&ProgressIO);

         if (pClient->pNet ()->Disconnect (this, bVoluntary, bDisconnected) != false)
         {
            bExit = true;
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE;

         if (bExit == false)
         {
            SocketDisconnect_Exit (false, bVoluntary, bDisconnected);
         }
      }

      return bExit;
   }

   void SocketDisconnect_Complete (bool bVoluntary) override
   {
      SocketDisconnect_Exit (false, bVoluntary, true);
   }

   void SocketDisconnect_Exit (bool bConnected, bool bVoluntary, bool bDisconnected)
   {
      PROGRESS ProgressIO;

      bNetConnected = false;

      bSocketConnected = (bSocketConnected != false && bVoluntary == false);

      // this.#Progress ({ nProgress: this.#ePROGRESS.SOCKET_DISCONNECTED, bVoluntary, bDisconnected });   // is this necessary

      pClient->ReadyState (CLIENT::eSTATE::SOCKETDISCONNECTED);

      ProgressIO.nProgress = CLIENT::ePROGRESS::SOCKETDISCONNECT_RESULT;
      ProgressIO.bVoluntary = bVoluntary;
      ProgressIO.bDisconnected = bDisconnected;
      ProgressIO.dwResult = 0;
      ProgressIO.bResult = !bError;

      Progress (&ProgressIO);
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
            pSourceIO = dynamic_cast<IO_SESSION*> (pSource);
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
         pSourceIO = dynamic_cast<IO_SESSION*> (pSource);
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
      CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (pIAction);

      switch (nType)
      {
      case kLOGIN_RESPONSE:                Login_Response            (pIActionIO, RMAP::CORE::CLIENT::GetVDParam (pParam, MV_VDPARAM_VOLUNTARY)); break;
      case kLOGOUT_RESPONSE:               Logout_Response           (pIActionIO, pParam);                                    break;
      }
   }

public:
   CLIENT*                             pClient;
   SUBSCRIPTION*                       pSubscription;
   IO_SESSION*                         pSourceIO;

   std::string                         sEndPoint;

   // Control
   bool                                bError;
   uint32_t                            dwResult;
   bool                                bSocketConnected;
   bool                                bNetConnected;

   // Login
   void*                               pParams;
   RMAP::CORE::SOURCE_SESSION::LOGIN*    pLogin;
   bool                                bLoggedIn;

private:
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
   int                     m_nTimeout;
};

/*******************************************************************************************************************************
**                                                   CLASS (CONTROL)                                                          **
*******************************************************************************************************************************/

CONTROL::CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription)
{
   m_pImpl = new CONTROL::Impl (pClient, pNetSettings, pSubscription);
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
   return m_pImpl->sEndPoint;
}

bool CONTROL::SafeKill ()
{
   return m_pImpl->SafeKill ();
}

bool CONTROL::ClearError ()
{
   return m_pImpl->ClearError ();
}

bool CONTROL::SocketConnect ()
{
   return m_pImpl->SocketConnect_Attempt (true);
}

bool CONTROL::SocketReconnect ()
{
   return SocketConnect ();
}

bool CONTROL::SocketDisconnect ()
{
   return m_pImpl->SocketDisconnect_Attempt (true, false);
}

bool CONTROL::SocketDisconnected (bool bVoluntary)
{
   bool bResult = false;

   if (m_pImpl->bNetConnected != false)
   {
      // Logout and disconnect may be called safely for involuntary logouts. If the user is not logged in or connected, these functions will do nothing.

      ClearError ();

      Logout_Request (bVoluntary, true);
      m_pImpl->SocketDisconnect_Attempt (bVoluntary, true);

      bResult = true;
   }

   return bResult;
}

bool CONTROL::Login (RMAP::CORE::SOURCE* pSource, void* pParams)  { return m_pImpl->Login (pSource, pParams);        }
bool CONTROL::Logout (RMAP::CORE::SOURCE* pSource, void* pParams) { return m_pImpl->Logout (pSource, pParams);       }

// ==========================================================================================================================

bool CONTROL::Logout_Request (bool bVoluntary, bool bDisconnected)
{
   bool bExit = false;
   PROGRESS ProgressIO;

   // If the logout is involuntary, this function must complete all processing on the request and may not cause the current thread to exit.

   if (m_pImpl->pClient->ReadyState () == CLIENT::eSTATE::LOGGEDIN)
   {
      ClearError ();

      m_pImpl->pClient->ReadyState (CLIENT::eSTATE::LOGGING);

      ProgressIO.nProgress = CLIENT::ePROGRESS::LOGOUT_ATTEMPT;
      ProgressIO.bVoluntary = bVoluntary;
      ProgressIO.bDisconnected = bDisconnected;

      m_pImpl->Progress (&ProgressIO);

      if (bVoluntary != false && !bDisconnected)
      {
         CLIENT::IACTION* pIAction = dynamic_cast<CLIENT::IACTION*> (m_pImpl->pSourceIO->Logout_Request (m_pImpl->pParams, m_pImpl->pLogin));

         if (pIAction)
         {
            if (pIAction->Send (m_pImpl, CONTROL::kLOGOUT_RESPONSE, RMAP::CORE::CLIENT::SetVDParam (bVoluntary, false, bDisconnected)) != false)
            {
               bExit = true;
            }
//            else this.#dwResult = this.IO_RESULT_TRANSMITFAILURE;
         }
//         else this.#dwResult = this.IO_RESULT_TRANSMITFAILURE; /// WRONG ERROR
      }
//      else this.#dwResult = this.IO_RESULT_SUCCESS;

      if (bExit == false)
      {
         Logout_Exit (NULL, bVoluntary, bDisconnected);
      }
   }
   else
   {
      m_pImpl->pSourceIO   = NULL;
      m_pImpl->pParams     = NULL;
   }

   return bExit;
}

void CONTROL::Logout_Response (CLIENT::IACTION* pIAction, intptr_t pVD)
{
//   this.#dwResult = pIAction.pResponse.nResult;

   Logout_Exit (pIAction, RMAP::CORE::CLIENT::GetVDParam (pVD, MV_VDPARAM_VOLUNTARY), RMAP::CORE::CLIENT::GetVDParam (pVD, MV_VDPARAM_DISCONNECTED));
}

void CONTROL::Logout_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
{
   PROGRESS ProgressIO;

   if (pIAction)
   {
      m_pImpl->pSourceIO->Logout_Response (m_pImpl->pParams, m_pImpl->pLogin, pIAction, bVoluntary, bDisconnected);
   }

   m_pImpl->bLoggedIn = (m_pImpl->bLoggedIn != false && bVoluntary == false);

   if (m_pImpl->bLoggedIn == false)
   {
      m_pImpl->pLogin = m_pImpl->pSourceIO->Login_Destroy (m_pImpl->pLogin);
   }

   m_pImpl->pClient->ReadyState (CLIENT::eSTATE::LOGGEDOUT);

   ProgressIO.nProgress       = CLIENT::ePROGRESS::LOGOUT_RESULT;
   ProgressIO.bVoluntary      = bVoluntary;
   ProgressIO.bDisconnected   = bDisconnected;
   ProgressIO.dwResult        = 0;
   ProgressIO.pLogin          = NULL;
   ProgressIO.bResult         = !m_pImpl->bError;

   m_pImpl->Progress (&ProgressIO);

   m_pImpl->pSourceIO   = NULL;
   m_pImpl->pParams     = NULL;
}

bool CONTROL::bNetConnected ()
{
   return m_pImpl->bNetConnected;
}

/******************************************************************************************************************************/
