/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : LibraryRMAP.cpp                                               **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2026 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include "Version.h"

using namespace RMAP;

std::string LibraryRMAP::sModuleName = "RMAP";

LibraryRMAP::LibraryRMAP (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion) :
   CORE::LIBRARY (sID, sCopyright, sTitle, sVersion)
{
}

LibraryRMAP::~LibraryRMAP ()
{
}

bool LibraryRMAP::Install (CORE::PLUGIN* pPlugin)
{
   bool bResult = true;

   m_apFactory_Model.push_back (CORE::MODEL_SESSION_NULL::factory ());
   m_apFactory_Model.push_back (CORE::MODEL_SESSION_C2A::factory ());
   m_apFactory_Model.push_back (CORE::MODEL_SESSION_UIP::factory ());

   pPlugin->Factory_Models (m_apFactory_Model);

   return bResult;
}

void LibraryRMAP::Unstall (CORE::PLUGIN* pPlugin)
{
   int n;

   for (n = 0; n < m_apFactory_Model.size (); n++)
      delete m_apFactory_Model[n];
}

/*******************************************************************************************************************************
**                                                     Startup/Shutdown                                                       **
*******************************************************************************************************************************/

void RMAP::CORE::Install ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LibraryInstall (new LibraryRMAP (LibraryRMAP::sModuleName, "Copyright 2014 - 2026 Metaversal Corporation. All rights reserved.", "RMAP Library", VER_FILE_VERSION_STR));
}

void RMAP::CORE::Unstall ()
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->LibraryUnstall (LibraryRMAP::sModuleName);
}
