/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVSB_cpp : SBTIME.cpp                                                        **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION::IREFERENCE)                                              **
****************************************************************\**************************************************************/

SBTIME::IREFERENCE::IREFERENCE (std::string sID) :
   RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*> (sID)
{
}

SBTIME::IREFERENCE::~IREFERENCE ()
{
}

std::string SBTIME::IREFERENCE::Key ()
{
   return std::to_string (MV_SERVICE_OBJECT_TIME);
}

RMAP::CORE::MODEL* SBTIME::IREFERENCE::Create (RMAP::CORE::SOURCE* pSource)
{
   return new SBTIME (this, dynamic_cast<SB_SBTIME*>(pSource));
}

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                        **
*******************************************************************************************************************************/

SBTIME::FACTORY::FACTORY (std::string sID) :
   RMAP::CORE::MODEL::FACTORY (sID)
{
}

SBTIME::FACTORY::~FACTORY ()
{
}

RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*>* SBTIME::FACTORY::Reference (std::vector<std::string> asArgs)
{
   return new SBTIME::IREFERENCE (sID ());
}

/*******************************************************************************************************************************
**                                                   CLASS (SBTIME)                                                           **
*******************************************************************************************************************************/

SBTIME::SBTIME (IREFERENCE* pReference, SB_SBTIME* pSource) :
   RMAP::CORE::MODEL (pReference, pSource)
{
}

SBTIME::~SBTIME ()
{
}

RMAP::CORE::MODEL::FACTORY* SBTIME::factory ()
{
   return new FACTORY ("SBTime");
}

void SBTIME::Tick (int uCode, TIME tmServer)
{
   NOTIFYPARAM np;

   np.uCode    = uCode;
   np.tmServer = tmServer;

   Emit ("onTick", &np);
}

/******************************************************************************************************************************/
