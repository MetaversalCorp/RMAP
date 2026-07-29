/*******************************************************************************************************************************
**                                                                                                                            **
**                                  RMAP_Svc_SocketIO  : Source_IO_Session_Null.cpp                                           **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                        **
*******************************************************************************************************************************/

IO_SESSION_NULL::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction) :
   IO_SESSION::FACTORY (sID_Service, sID_Model, apAction)
{
}

IO_SESSION_NULL::FACTORY::~FACTORY ()
{
}

RMAP::CORE::SOURCE* IO_SESSION_NULL::FACTORY::Create (RMAP::CORE::CLIENT* pClient)
{
   return new IO_SESSION_NULL (pReference (), pClient);
}

/*******************************************************************************************************************************
**                                                   CLASS (IO_SESSION_NULL)                                                   **
*******************************************************************************************************************************/

std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> IO_SESSION_NULL::aAction =
{
};

IO_SESSION_NULL::IO_SESSION_NULL (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   IO_SESSION (pReference, pClient)
{
}

IO_SESSION_NULL::~IO_SESSION_NULL ()
{
}

void IO_SESSION_NULL::init ()
{
}

IO_SESSION_NULL::FACTORY* IO_SESSION_NULL::factory ()
{
   return new FACTORY ("Socket.IO", "Session_Null", aAction);
}

// ===== Client Methods =====================================================================================================

void IO_SESSION_NULL::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   RMAP::CORE::MODEL_SESSION_NULL* pModelSession = dynamic_cast<RMAP::CORE::MODEL_SESSION_NULL*> (pModel ());

   IO_SESSION::Progress (pProgress);

   pModelSession->Progress (pProgress);
}

// ===== Client Interface Methods ===========================================================================================

// These methods should implement an interface defined by the client

RMAP::CORE::SOURCE_SESSION::LOGIN* IO_SESSION_NULL::Login_Create ()
{
   return NULL;
}

RMAP::CORE::SOURCE_SESSION::LOGIN* IO_SESSION_NULL::Login_Destroy (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   delete pLogin;

   return NULL;
}

RMAP::CORE::CLIENT::IACTION* IO_SESSION_NULL::Login_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   return NULL;
}

bool IO_SESSION_NULL::Login_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary)
{
   return false;
}

RMAP::CORE::CLIENT::IACTION* IO_SESSION_NULL::Logout_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   return NULL;
}

bool IO_SESSION_NULL::Logout_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
{
   bool bResult = false;

   // pIAction could be null
   // pIAction.dwResult could be non-successful

   // assume an involuntary logout because on voluntary logout, the entire pLogin object will be deleted

   return bResult;
}

bool IO_SESSION_NULL::Attempt (int nReadyState)
{
   return false;
}

RMAP::CORE::ISOURCE_SESSION* IO_SESSION_NULL::GetSessionInterface ()
{
   return NULL;
}

/******************************************************************************************************************************/
