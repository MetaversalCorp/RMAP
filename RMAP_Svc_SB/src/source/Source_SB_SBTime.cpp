/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVSB_cpp : SB_SBTime.cpp                                                     **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class SB_SBTIME::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }
};

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                      **
*******************************************************************************************************************************/

SB_SBTIME::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction) :
   RMAP::CORE::SOURCE::FACTORY (sID_Service, sID_Model, wClass, apAction)
{
}

SB_SBTIME::FACTORY::~FACTORY ()
{
}

RMAP::CORE::SOURCE* SB_SBTIME::FACTORY::Create (RMAP::CORE::CLIENT* pClient)
{
   return new SB_SBTIME (pReference (), pClient);
}

/*******************************************************************************************************************************
**                                                   CLASS (SB_OBJECT)                                                        **
*******************************************************************************************************************************/

static std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> apAction;

SB_SBTIME::SB_SBTIME (RMAP::CORE::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::SOURCE (pReference, pClient)
{
   m_pImpl = new Impl ();
}

SB_SBTIME::~SB_SBTIME ()
{
   delete m_pImpl;
}

SB_SBTIME::FACTORY* SB_SBTIME::factory ()
{
   return new FACTORY ("MVSB", "SBTime", MV_SERVICE_OBJECT_TIME, apAction);
}

void SB_SBTIME::Tick (int uCode, TIME tmServer)
{
   SBTIME* pModelSB = dynamic_cast<SBTIME*> (pModel ());

   pModelSB->Tick (uCode, tmServer);
}

/******************************************************************************************************************************/
