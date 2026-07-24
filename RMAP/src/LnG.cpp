/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : LnG.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class LNG::Impl
{
public:
   Impl (LNG* pLnG) :
      m_pLnG (pLnG)
   {
   }

   ~Impl ()
   {
      if (m_pService)
      {
         if (m_pClient)
         {
            if (m_pSession)
            {
               if (m_sSession.empty () == false)
               {
                  m_sSession.clear ();
               }

               m_pSession->Detach (m_pLnG);

               // Detach needs time to communicate closure of Session with Server, so we need to wait to safe to kill.
               while (m_pClient->SafeKill () == false)
               {
                  std::this_thread::sleep_for (std::chrono::milliseconds (100));
               }

               m_pSession = m_pClient->Session_Close (m_pSession);
            }
            else
            {
               // Detach needs time to communicate closure of Session with Server, so we need to wait to safe to kill.
               while (m_pClient->SafeKill () == false)
               {
                  std::this_thread::sleep_for (std::chrono::milliseconds (100));
               }
            }

            m_pClient->Detach (m_pLnG);

            m_pClient = m_pService->Client_Close (m_pClient);
         }

         m_pService->Detach (m_pLnG);

         m_pService = m_pService->pNamespace ()->Service_Close (m_pService);
      }
   }

   bool Init (const std::string& sNamespace, const std::string& sID_Service, const std::string& sConnect, const std::string& sSession)
   {
      bool bResult = false;
      APP* pCore = APP::GetInstance ();

      if (m_pService = pCore->Service_Open (sNamespace, sID_Service, sConnect))
      {
         m_pService->Attach (m_pLnG, true);

         if (m_pClient = m_pService->Client_Open (1))
         {
            m_pClient->Attach (m_pLnG, true);

            if (m_pSession = m_pClient->Session_Open (true))
            {
               bResult = true;

               m_pSession->Attach (m_pLnG, true);

               m_sSession = sSession;
            }
         }
      }

      return bResult;
   }

   LNG*           m_pLnG;
   SERVICE*       m_pService;
   CLIENT*        m_pClient;
   MODEL_SESSION* m_pSession;
   std::string    m_sSession;
};

/*******************************************************************************************************************************
**                                                     CLASS (LNG)                                                            **
*******************************************************************************************************************************/

LNG::LNG ()
{
   m_pImpl = new Impl (this);
}

LNG::~LNG ()
{
   delete m_pImpl;
}

bool LNG::Init (const std::string& sNamespace, const std::string& sID_Service, const std::string& sConnect, const std::string& sSession)
{
   return m_pImpl->Init (sNamespace, sID_Service, sConnect, sSession);
}

/*******************************************************************************************************************************
**                                                     Accessors                                                              **
*******************************************************************************************************************************/

uint64_t LNG::twUserIx ()
{
   return m_pImpl->m_pSession ? m_pImpl->m_pSession->twUserIx () : 0;
}

std::string const& LNG::sNamespace () const&
{
   return m_pImpl->m_pService->sNamespace ();
}

MODEL_SESSION* LNG::pSession ()
{
   return m_pImpl->m_pSession;
}

/*******************************************************************************************************************************
**                                                     Private Methods                                                        **
*******************************************************************************************************************************/

bool LNG::Login_Call ()
{
   bool bResult = false;

   if (m_pImpl->m_sSession.empty () == false)
   {
      if (m_pImpl->m_pSession->Login (m_pImpl->m_sSession))
      {
         ReadyState (LOGGING);

         bResult = true;
      }

      m_pImpl->m_sSession.clear ();
   }

   return bResult;
}

/*******************************************************************************************************************************
**                                                     Public Methods                                                         **
*******************************************************************************************************************************/

bool LNG::IsReady ()
{
   int nReadyState = ReadyState ();

   return (nReadyState == eSTATE::LOGGEDIN || nReadyState == eSTATE::LOGGEDOUT);
}

MEM::MODEL* LNG::Model_Open (const std::string& sID_Model, const std::string& sArgs)
{
   return m_pImpl->m_pClient->Model_Open (sID_Model, sArgs);
}

MEM::MODEL* LNG::Model_Close (MEM::MODEL* pModel)
{
   return m_pImpl->m_pClient->Model_Close (pModel);
}

bool LNG::Login (const std::string& sSession)
{
   bool bResult = true;

   if (m_pImpl->m_pSession)
   {
      m_pImpl->m_sSession = sSession;

      if (m_pImpl->m_pClient->IsConnected ())
      {
         if (m_pImpl->m_pSession->IsLoggedOut ())
         {
            if (m_pImpl->m_sSession.empty () == false)
            {
               bResult = Login_Call ();
            }
         }
      }
   }
   else bResult = false;

   return bResult;
}

bool LNG::Logout ()
{
   bool bResult = false;

   if (m_pImpl->m_pSession->Logout ())
   {
      ReadyState (LOGGING); // Is this even necessary. The act of logging out should trigger an event, which would result in onReadyState being called

      bResult = true;
   }

   m_pImpl->m_sSession.clear ();

   return bResult;
}

void LNG::Notify (INOTICE* pNotice)
{
   if (pNotice->sNotification.compare ("onReadyState") == 0)
   {
      if (m_pImpl->m_pSession)
      {
         if (m_pImpl->m_pClient->IsDisconnected ())
         {
            ReadyState (DISCONNECTED);
         }
         else if (m_pImpl->m_pClient->IsConnected ())
         {
            if (m_pImpl->m_pSession->IsLoggedOut ())
            {
               if (Login_Call () == false)
                  ReadyState (LOGGEDOUT);
            }
            else if (m_pImpl->m_pSession->IsLoggedIn ())
            {
               ReadyState (LOGGEDIN);
            }
            else ReadyState (LOGGING);
         }
         else ReadyState (CONNECTING);
      }
   }
}

/******************************************************************************************************************************/
