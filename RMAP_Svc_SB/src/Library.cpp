/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_SVC_SB : Library.cpp                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP;

extern RMAP::SVC_SB::xTIME* g_pTime = NULL;
std::string LibrarySVC_SB::sModuleName = "RMAP_Svc_SB";

LibrarySVC_SB::LibrarySVC_SB (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion) :
   RMAP::CORE::LIBRARY (sID, sCopyright, sTitle, sVersion),
   m_pRequire (NULL),
   m_pITime (NULL)
{
}

LibrarySVC_SB::~LibrarySVC_SB ()
{
}

bool LibrarySVC_SB::Install (RMAP::CORE::PLUGIN* pPlugin)
{
   bool bResult = true;
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pRequire = pCore->Require ("RMAP", "", ""))
   {
      m_apFactory_Service.push_back (RMAP::SVC_SB::SERVICE::factory ());

      m_apFactory_Model.push_back (RMAP::SVC_SB::SBTIME::factory ());

      m_apFactory_Source.push_back (RMAP::SVC_SB::SB_SESSION_NULL::factory ());
      m_apFactory_Source.push_back (RMAP::SVC_SB::SB_SBTIME::factory ());

      pPlugin->Factory_Services (m_apFactory_Service);
      pPlugin->Factory_Models (m_apFactory_Model);
      pPlugin->Factory_Sources (m_apFactory_Source);

      m_pITime = new ITIME_SERVICE ();
      g_pTime = new RMAP::SVC_SB::xTIME (m_pITime);
   }
   else bResult = false;

   return bResult;
}

void LibrarySVC_SB::Unstall (RMAP::CORE::PLUGIN* pPlugin)
{
   int n;
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pRequire)
   {
      delete g_pTime;
      g_pTime = NULL;
      delete m_pITime;
      m_pITime = NULL;

      for (n = 0; n < m_apFactory_Service.size (); n++)
         delete m_apFactory_Service[n];

      for (n = 0; n < m_apFactory_Model.size (); n++)
         delete m_apFactory_Model[n];

      for (n = 0; n < m_apFactory_Source.size (); n++)
         delete m_apFactory_Source[n];

      pCore->Release (m_pRequire);

      m_pRequire = NULL;
   }
}

bool GetVDParam (intptr_t pVD, intptr_t nVDParam)
{
   return ((pVD & nVDParam) != 0);
}

intptr_t SetVDParam (bool bVoluntary, bool bConnected, bool bDisconnected)
{
   intptr_t nResult = 0;

   if (bVoluntary)
      nResult |= MVSB_VDPARAM_VOLUNTARY;

   if (bConnected)
      nResult |= MVSB_VDPARAM_CONNECTED;

   if (bDisconnected)
      nResult |= MVSB_VDPARAM_DISCONNECTED;

   return nResult;
}
