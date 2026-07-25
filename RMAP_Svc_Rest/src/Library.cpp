/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   MVRest_cpp : LibraryMVRest.cpp                                           **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2025 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP;
std::string LibrarySVC_Rest::sModuleName = "SVC_Rest";

LibrarySVC_Rest::LibrarySVC_Rest (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion) :
   RMAP::CORE::LIBRARY (sID, sCopyright, sTitle, sVersion)
{
   curl_global_init (CURL_GLOBAL_DEFAULT);
}

LibrarySVC_Rest::~LibrarySVC_Rest ()
{
   curl_global_cleanup ();
}

bool LibrarySVC_Rest::Install (RMAP::CORE::PLUGIN* pPlugin)
{
   bool bResult = true;
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pRequire = pCore->Require ("RMAP", "", ""))
   {
      m_apFactory_Service.push_back (SVC_Rest::SERVICE::factory ());

      m_apFactory_Source.push_back (SVC_Rest::REST_SESSION_NULL::factory ());

      pPlugin->Factory_Services (m_apFactory_Service);
      pPlugin->Factory_Sources (m_apFactory_Source);
   }
   else bResult = false;

   return bResult;
}

void LibrarySVC_Rest::Unstall (RMAP::CORE::PLUGIN* pPlugin)
{
   int n;
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pRequire)
   {
      for (n = 0; n < m_apFactory_Service.size (); n++)
         delete m_apFactory_Service[n];

      for (n = 0; n < m_apFactory_Source.size (); n++)
         delete m_apFactory_Source[n];

      pCore->Release (m_pRequire);

      m_pRequire = NULL;
   }
}
