/*******************************************************************************************************************************
**                                                                                                                            **
**                                               RMAP_SVC_SB : SB_OBJECT.cpp                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

static const char* asProgress[] =
{
   "SOCKETCONNECT_ATTEMPT",
   "SOCKETCONNECT_RESULT",
   "SOCKETDISCONNECT_ATTEMPT",
   "SOCKETDISCONNECT_RESULT",
   "SYSTEMCONNECT_ATTEMPT",
   "SYSTEMCONNECT_RESULT",
   "SYSTEMDISCONNECT_ATTEMPT",
   "SYSTEMDISCONNECT_RESULT",
   "LOGIN_ATTEMPT",
   "LOGIN_RESULT",
   "LOGOUT_ATTEMPT",
   "LOGOUT_RESULT",
};

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class SB_SESSION::Impl
{
public:
   Impl ()
   {
      nReconnect = 0;
   }

   ~Impl ()
   {
   }

   int nReconnect;
};

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                      **
*******************************************************************************************************************************/

SB_SESSION::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction) :
   RMAP::CORE::SOURCE_SESSION::FACTORY (sID_Service, sID_Model, MV_SERVICE_OBJECT_SESSION, apAction)
{
}

SB_SESSION::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (SB_SESSION)                                                        **
*******************************************************************************************************************************/

SB_SESSION::SB_SESSION (RMAP::CORE::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::SOURCE_SESSION (pReference, pClient)
{
   m_pImpl = new Impl ();
}

SB_SESSION::~SB_SESSION ()
{
   delete m_pImpl;
}

void SB_SESSION::initialize (RMAP::CORE::MODEL_SESSION* pModel)
{
   SVC_SB::CLIENT* pClientSB = dynamic_cast <SVC_SB::CLIENT*> (pClient ());

   RMAP::CORE::SOURCE_SESSION::initialize (pModel);

   pModel->twClientIx = pClientSB->twClientIx ();
}

RMAP::CORE::SOURCE_SESSION::LOGIN* SB_SESSION::pLogin ()
{ 
   SVC_SB::CLIENT* pClientSB = dynamic_cast <SVC_SB::CLIENT*> (pClient ());

   return pClientSB->pLogin ();
}

void SB_SESSION::Reconnect ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pImpl->nReconnect == 0)
      m_pImpl->nReconnect = 1;
   else if (m_pImpl->nReconnect < 64)
      m_pImpl->nReconnect *= 2;

   int nDelay = (m_pImpl->nReconnect * 1000) + (rand () % 2000);

   pCore->LoggerWrite (RMAP::CORE::LOGGER::kLOGLEVEL_Info, LibrarySVC_SB::sModuleName, "attempt reconnect in " + std::to_string (nDelay / 1000) + " seconds from now.");

//   setTimeout (this.pClient.SocketReconnect.bind (this.pClient), nDelay);
}

void SB_SESSION::Reconnect (int nReconnect)
{
   m_pImpl->nReconnect = nReconnect;

   int nDelay = (m_pImpl->nReconnect * 1000) + (rand () % 2000);

//      console.log ('attempt reconnect in ' + (nDelay / 1000) + ' seconds from: ', Date.now ());

//   setTimeout (this.pClient.SocketReconnect.bind (this.pClient), nDelay);
}

void SB_SESSION::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());
   CONTROL::SB_PROGRESS* pProgressSB = dynamic_cast<CONTROL::SB_PROGRESS*> (pProgress);
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LoggerWrite 
   (
      RMAP::CORE::LOGGER::kLOGLEVEL_Info, 
      LibrarySVC_SB::sModuleName, 
      std::string (asProgress[pProgressSB->nProgress]) +
      (pProgressSB->bVoluntary == false ? " (reconnect)" : "") +
      std::to_string (pProgressSB->dwResult) +
      " =>  [" + pClientSB->pService ()->sNamespace () + "] using " + pClientSB->sEndPoint ()
   );

   switch (pProgressSB->nProgress)
   {
   case pClientSB->SOCKETCONNECT_RESULT:
      if (pProgressSB->bResult != false)
      {
         m_pImpl->nReconnect = 0;

         if (pProgressSB->bVoluntary != false)
         {
            pClientSB->SystemConnect ();
         }
         else pClientSB->SystemReconnect ();
      }
      else
      {
//         console.log ('failed to connect');

         if (pClientSB->bNetConnected () == false)
         {
            Reconnect ();
         }
      }
      break;

   case pClientSB->SOCKETDISCONNECT_RESULT:
      if (pProgressSB->bResult != false)
      {
         if (pProgressSB->bVoluntary == false)
         {
  //          console.log ('involuntary disconnect');

            if (pClientSB->bSystemConnected () == false)
            {
               /// set an alert in the application that the user has unexpectedly lost their connection (red)
            }

            Reconnect (0);
         }
      }
      break;

   case pClientSB->SYSTEMCONNECT_RESULT:
      if (pProgressSB->bResult != false)
      {
         if (pProgressSB->bVoluntary != false)
         {
         }
         else
         {
            if (pClientSB->bLoggedIn () != false)
            {
               /// set an alert in the application that the user is attempting to log back in (yellow)

               pClientSB->Login (NULL); // Logging in without a null pParams object signifies a relogin, and will use the data persisting in the pLogin object
            }
            else
            {
               /// set an alert in the application that the connection has been restored (green, 5 second delay)
            }
         }
      }
      else;
      break;

   case pClientSB->SYSTEMDISCONNECT_RESULT:
      if (pProgressSB->bResult != false)
      {
         if (pProgressSB->bVoluntary != false)
         {
         }
         else if (pClientSB->bLoggedIn () == false)
         {
            /// set an alert in the application that the user has unexpectedly lost their connection (red)
         }
      }
      break;

   case pClientSB->LOGIN_RESULT:
      if (pProgressSB->bResult != false)
      {
         if (pProgressSB->bVoluntary == false)
         {
            /// set an alert in the application that the connection has been restored (green, 5 second delay)
         }
      }
      else
      {
         if (pClientSB->bLoggedIn () != false)
         {
            /// set an alert in the application that the relogin failed (yellow, 5 second delay)
         }
      }
      break;

   case pClientSB->LOGOUT_RESULT:
      if (pProgressSB->bResult != false)
      {
         if (pProgressSB->bVoluntary == false)
         {
            /// set an alert in the application that the user has unexpectedly lost their connection (red)
         }
      }
      break;
   }
}

/******************************************************************************************************************************/

void SB_SESSION::LoggedOut ()
{
   RMAP::CORE::MODEL_SESSION* pModelSB = dynamic_cast<RMAP::CORE::MODEL_SESSION*> (pModel ());

//      console.log ('LOGGEDOUT');

   pModelSB->LoggedOut ();
}

// ===== Model Methods ======================================================================================================

bool SB_SESSION::Attach ()
{
   RMAP::CORE::SOURCE_SESSION::Attach ();

   if (bAutoConnect)
      Connect ();

   return true;
}

bool SB_SESSION::Detach ()
{
   if (bAutoConnect)
      Disconnect (false);

   RMAP::CORE::SOURCE_SESSION::Detach ();

   return true;
}

// --------------------------------------------------------------------------------------------------------------------------

bool SB_SESSION::Connect ()
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());

   return pClientSB->SocketConnect ();
}

bool SB_SESSION::Disconnect (bool bVoluntary)
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());

   return pClientSB->SocketDisconnected (bVoluntary);
}
