/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_SVC_SB : Library.cpp                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

using namespace RMAP;

extern RMAP::SVC_SB::xTIME* g_pTime = NULL;
std::string LibrarySVC_SB::sModuleName = "RMAP_SVC_SB";

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

/*******************************************************************************************************************************
**                                                     Startup/Shutdown                                                       **
*******************************************************************************************************************************/

void RMAP::SVC_SB::Install ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

#ifdef WIN32
   WSADATA wsaData;

   if (WSAStartup (MAKEWORD (2, 2), &wsaData) == 0)
#endif
   {
      pCore->LibraryInstall (new LibrarySVC_SB (LibrarySVC_SB::sModuleName, "Copyright 2014 - 2026 Metaversal Corporation. All rights reserved.", "RMAP Service REST", ""));
   }
}

void RMAP::SVC_SB::Unstall ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LibraryUnstall (LibrarySVC_SB::sModuleName);

#ifdef WIN32
   WSACleanup ();
#endif
}
