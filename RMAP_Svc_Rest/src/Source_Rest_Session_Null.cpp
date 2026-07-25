/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVRest_cpp : Source_REST_SESSION_NULL.cpp                                    **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_Rest;

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                        **
*******************************************************************************************************************************/

REST_SESSION_NULL::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction) :
   REST_SESSION::FACTORY (sID_Service, sID_Model, apAction)
{
}

REST_SESSION_NULL::FACTORY::~FACTORY ()
{
}

RMAP::CORE::SOURCE* REST_SESSION_NULL::FACTORY::Create (RMAP::CORE::CLIENT* pClient)
{
   return new REST_SESSION_NULL (pReference (), pClient);
}

/*******************************************************************************************************************************
**                                                   CLASS (REST_SESSION_NULL)                                                   **
*******************************************************************************************************************************/

std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> REST_SESSION_NULL::aAction =
{
};

REST_SESSION_NULL::REST_SESSION_NULL (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   REST_SESSION (pReference, pClient)
{
}

REST_SESSION_NULL::~REST_SESSION_NULL ()
{
}

void REST_SESSION_NULL::init ()
{
}

REST_SESSION_NULL::FACTORY* REST_SESSION_NULL::factory ()
{
   return new FACTORY ("MVRest", "Session_Null", aAction);
}

// ===== Client Methods =====================================================================================================

void REST_SESSION_NULL::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   RMAP::CORE::MODEL_SESSION_NULL* pModelSession = dynamic_cast<RMAP::CORE::MODEL_SESSION_NULL*> (pModel ());

   REST_SESSION::Progress (pProgress);

   pModelSession->Progress (pProgress);
}

// ===== Client Interface Methods ===========================================================================================

// These methods should implement an interface defined by the client

RMAP::CORE::SOURCE_SESSION::LOGIN* REST_SESSION_NULL::Login_Create ()
{
   return NULL;
}

RMAP::CORE::SOURCE_SESSION::LOGIN* REST_SESSION_NULL::Login_Destroy (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   delete pLogin;

   return NULL;
}

RMAP::CORE::CLIENT::IACTION* REST_SESSION_NULL::Login_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   return NULL;
}

bool REST_SESSION_NULL::Login_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary)
{
   return false;
}

RMAP::CORE::CLIENT::IACTION* REST_SESSION_NULL::Logout_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin)
{
   return NULL;
}

bool REST_SESSION_NULL::Logout_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
{
   bool bResult = false;

   // pIAction could be null
   // pIAction.dwResult could be non-successful

   // assume an involuntary logout because on voluntary logout, the entire pLogin object will be deleted

   return bResult;
}

bool REST_SESSION_NULL::Attempt (int nReadyState)
{
   return false;
}

RMAP::CORE::ISOURCE_SESSION* REST_SESSION_NULL::GetSessionInterface ()
{
   return NULL;
}

CLIENT::ICODEC* REST_SESSION_NULL::Codec_Create (CLIENT::IACTION* pIAction, std::string sRequest)
{
   return NULL;
}

/******************************************************************************************************************************/
