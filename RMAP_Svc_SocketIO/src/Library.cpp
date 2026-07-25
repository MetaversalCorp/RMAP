/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   MVIO_cpp : LibrarySVC_SocketIO.cpp                                       **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2025 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP;
std::string LibrarySVC_SocketIO::sModuleName = "RMAP_SVC_SOCKETIO";

LibrarySVC_SocketIO::LibrarySVC_SocketIO (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion) :
   RMAP::CORE::LIBRARY (sID, sCopyright, sTitle, sVersion)
{
}

LibrarySVC_SocketIO::~LibrarySVC_SocketIO ()
{
}

bool LibrarySVC_SocketIO::Install (RMAP::CORE::PLUGIN* pPlugin)
{
   bool bResult = true;
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   if (m_pRequire = pCore->Require ("RMAP", "", ""))
   {
      m_apFactory_Service.push_back (RMAP::SVC_SOCKETIO::SERVICE::factory ());

      m_apFactory_Source.push_back (RMAP::SVC_SOCKETIO::IO_SESSION_NULL::factory ());

      pPlugin->Factory_Services (m_apFactory_Service);
      pPlugin->Factory_Sources (m_apFactory_Source);
   }
   else bResult = false;

   return bResult;
}

void LibrarySVC_SocketIO::Unstall (RMAP::CORE::PLUGIN* pPlugin)
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

/*******************************************************************************************************************************
**                                                     Startup/Shutdown                                                       **
*******************************************************************************************************************************/

void RMAP::SVC_SOCKETIO::Install ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LibraryInstall (new LibrarySVC_SocketIO (LibrarySVC_SocketIO::sModuleName, "Copyright 2014 - 2026 Metaversal Corporation. All rights reserved.", "RMAP Service Socket.IO", ""));
}

void RMAP::SVC_SOCKETIO::Unstall ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LibraryUnstall (LibrarySVC_SocketIO::sModuleName);
}
