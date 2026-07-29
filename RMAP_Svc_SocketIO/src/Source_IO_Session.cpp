/*******************************************************************************************************************************
**                                                                                                                            **
**                                          RMAP_Svc_SocketIO  : Source_IO_Session.cpp                                        **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

static const char* asProgress[] =
{
   "SOCKETCONNECT_ATTEMPT",
   "SOCKETCONNECT_RESULT",
   "SOCKETDISCONNECT_ATTEMPT",
   "SOCKETDISCONNECT_RESULT",
   "LOGIN_ATTEMPT",
   "LOGIN_RESULT",
   "LOGOUT_ATTEMPT",
   "LOGOUT_RESULT",
};

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class IO_SESSION::Impl
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

IO_SESSION::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction) :
   RMAP::CORE::SOURCE_SESSION::FACTORY (sID_Service, sID_Model, 0, apAction)
{
}

IO_SESSION::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (IO_SESSION)                                                        **
*******************************************************************************************************************************/

IO_SESSION::IO_SESSION (RMAP::CORE::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::SOURCE_SESSION (pReference, pClient)
{
   m_pImpl = new Impl ();
}

IO_SESSION::~IO_SESSION ()
{
   delete m_pImpl;
}

void IO_SESSION::initialize (RMAP::CORE::MODEL_SESSION* pModel)
{
   RMAP::SVC_SOCKETIO::CLIENT* pClientIO = dynamic_cast <RMAP::SVC_SOCKETIO::CLIENT*> (pClient ());

   RMAP::CORE::SOURCE_SESSION::initialize (pModel);

   pModel->twClientIx = pClientIO->twClientIx ();
}

RMAP::CORE::SOURCE_SESSION::LOGIN* IO_SESSION::pLogin ()
{ 
   return NULL;
}

void IO_SESSION::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   CLIENT* pClientIO = dynamic_cast<CLIENT*> (pClient ());
   CONTROL::PROGRESS* pProgressIO = dynamic_cast<CONTROL::PROGRESS*> (pProgress);
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LoggerWrite
   (
      RMAP::CORE::LOGGER::kLOGLEVEL_Info,
      LibrarySVC_SocketIO::sModuleName,
      std::string (asProgress[pProgressIO->nProgress]) +
      (pProgressIO->bVoluntary == false ? " (reconnect)" : "") +
      std::to_string (pProgressIO->dwResult) +
      " =>  [" + pClientIO->sNamespace () + "] using " + pClientIO->sEndPoint ()
   );

   switch (pProgressIO->nProgress)
   {
   case CLIENT::SOCKETCONNECT_RESULT:
      if (pProgressIO->bResult != false)
      {
         m_pImpl->nReconnect = 0;

         if (pProgressIO->bVoluntary != false)
         {
         }
         else
         {
            if (pClientIO->bLoggedIn () != false)
            {
               /// set an alert in the application that the user is attempting to log back in (yellow)

               pClientIO->Login (NULL); // Logging in with a null pParams object signifies a relogin, and will use the data persisting in the pLogin object
            }
            else
            {
               /// set an alert in the application that the connection has been restored (green, 5 second delay)
            }
         }
      }
      else
      {
         if (pClientIO->bNetConnected () == false)
         {
            Reconnect ();
         }
      }
      break;

   case CLIENT::SOCKETDISCONNECT_RESULT:
      if (pProgressIO->bResult != false)
      {
         if (pProgressIO->bVoluntary == false)
         {
            //          console.log ('involuntary disconnect');

            Reconnect (0);
         }
      }
      break;

   case CLIENT::LOGIN_RESULT:
      if (pProgressIO->bResult != false)
      {
      }
      break;

   case CLIENT::LOGOUT_RESULT:
      if (pProgressIO->bResult != false)
      {
      }
      break;
   }
}

/******************************************************************************************************************************/

void IO_SESSION::LoggedOut ()
{
   RMAP::CORE::MODEL_SESSION* pModelSB = dynamic_cast<RMAP::CORE::MODEL_SESSION*> (pModel ());

//      console.log ('LOGGEDOUT');

   pModelSB->LoggedOut ();
}

// ===== Model Methods ======================================================================================================

bool IO_SESSION::Attach ()
{
   RMAP::CORE::SOURCE_SESSION::Attach ();

   Connect ();

   return true;
}

bool IO_SESSION::Detach ()
{
   Disconnect (false);

   RMAP::CORE::SOURCE_SESSION::Detach ();

   return true;
}

// --------------------------------------------------------------------------------------------------------------------------

bool IO_SESSION::Connect ()
{
   RMAP::SVC_SOCKETIO::CLIENT* pClientIO = dynamic_cast <RMAP::SVC_SOCKETIO::CLIENT*> (pClient ());

   return pClientIO->SocketConnect ();
}

bool IO_SESSION::Disconnect (bool bVoluntary)
{
   RMAP::SVC_SOCKETIO::CLIENT* pClientIO = dynamic_cast <RMAP::SVC_SOCKETIO::CLIENT*> (pClient ());

   return pClientIO->SocketDisconnect (bVoluntary);
}

void IO_SESSION::Reconnect ()
{
   if (m_pImpl->nReconnect == 0)
      m_pImpl->nReconnect = 1;
   else if (m_pImpl->nReconnect < 64)
      m_pImpl->nReconnect *= 2;

   int nDelay = (m_pImpl->nReconnect * 1000) + (rand () % 2000);

   //      console.log ('attempt reconnect in ' + (nDelay / 1000) + ' seconds from: ', Date.now ());

//   setTimeout (this.pClient.SocketReconnect.bind (this.pClient), nDelay);
}

void IO_SESSION::Reconnect (int nReconnect)
{
   m_pImpl->nReconnect = nReconnect;

   int nDelay = (m_pImpl->nReconnect * 1000) + (rand () % 2000);

   //      console.log ('attempt reconnect in ' + (nDelay / 1000) + ' seconds from: ', Date.now ());

   //   setTimeout (this.pClient.SocketReconnect.bind (this.pClient), nDelay);
}
