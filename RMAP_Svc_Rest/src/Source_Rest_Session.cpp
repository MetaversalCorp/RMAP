/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVRest_cpp : Source_Rest_Session.cpp                                         **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_Rest;

static const char* asProgress[] =
{
   "LOGIN_ATTEMPT",
   "LOGIN_RESULT",
   "LOGOUT_ATTEMPT",
   "LOGOUT_RESULT",
};

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class REST_SESSION::Impl
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

REST_SESSION::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction) :
   RMAP::CORE::SOURCE_SESSION::FACTORY (sID_Service, sID_Model, 0, apAction)
{
}

REST_SESSION::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (REST_SESSION)                                                        **
*******************************************************************************************************************************/

REST_SESSION::REST_SESSION (RMAP::CORE::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::SOURCE_SESSION (pReference, pClient)
{
   m_pImpl = new Impl ();
}

REST_SESSION::~REST_SESSION ()
{
   delete m_pImpl;
}

void REST_SESSION::initialize (RMAP::CORE::MODEL_SESSION* pModel)
{
//   MVSB::CLIENT* pClientSB = dynamic_cast <MVSB::CLIENT*> (pClient ());

   RMAP::CORE::SOURCE_SESSION::initialize (pModel);

//   pModel->twClientIx = pClientSB->twClientIx ();
}

RMAP::CORE::SOURCE_SESSION::LOGIN* REST_SESSION::pLogin ()
{ 
   return NULL;
}

void REST_SESSION::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   CLIENT* pClientREST = dynamic_cast<CLIENT*> (pClient ());
   CONTROL::PROGRESS* pProgressREST = dynamic_cast<CONTROL::PROGRESS*> (pProgress);
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LoggerWrite
   (
      RMAP::CORE::LOGGER::kLOGLEVEL_Info,
      LibrarySVC_Rest::sModuleName,
      std::string (asProgress[pProgressREST->nProgress]) +
      (pProgressREST->bVoluntary == false ? " (reconnect)" : "") +
      std::to_string (pProgressREST->dwResult) +
      " =>  [" + pClientREST->sNamespace () + "] using " + pClientREST->sEndPoint ()
   );

   switch (pProgressREST->nProgress)
   {
   case CLIENT::LOGIN_RESULT:
      if (pProgressREST->bResult != false)
      {
      }
      break;

   case CLIENT::LOGOUT_RESULT:
      if (pProgressREST->bResult != false)
      {
      }
      break;
   }
}

/******************************************************************************************************************************/

void REST_SESSION::LoggedOut ()
{
   RMAP::CORE::MODEL_SESSION* pModelSB = dynamic_cast<RMAP::CORE::MODEL_SESSION*> (pModel ());

//      console.log ('LOGGEDOUT');

   pModelSB->LoggedOut ();
}

// ===== Model Methods ======================================================================================================

bool REST_SESSION::Attach ()
{
   RMAP::CORE::SOURCE_SESSION::Attach ();

   if (bAutoConnect)
      Connect ();

   return true;
}

bool REST_SESSION::Detach ()
{
   if (bAutoConnect)
      Disconnect (false);

   RMAP::CORE::SOURCE_SESSION::Detach ();

   return true;
}

// --------------------------------------------------------------------------------------------------------------------------

bool REST_SESSION::Connect ()
{
   return false;
}

bool REST_SESSION::Disconnect (bool bVoluntary)
{
   return false;
}
