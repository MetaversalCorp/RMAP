/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Plugin.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <iostream>

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (IREFRENCE)                                                      **
*******************************************************************************************************************************/

PLUGIN::IREFERENCE::IREFERENCE (std::string sID) :
   RMAP::CORE::IREFERENCE<PLUGIN*, LIBRARY*> (sID)
{
}

PLUGIN::IREFERENCE::~IREFERENCE ()
{
}

std::string PLUGIN::IREFERENCE::Key ()
{
   return UniqueId ();
}

PLUGIN* PLUGIN::IREFERENCE::Create (LIBRARY* pLibrary)
{
   return new PLUGIN (pLibrary);
}


/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class PLUGIN::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }

   COLLECTION<std::string, RMAP::CORE::FACTORY*>   cpFactory;
   COLLECTION<std::string, PACKAGE::FACTORY*>      cpFactoryPkg;

   SHAREDOBJECT<PACKAGE*, PACKAGE::PACKAGEPARAM*>  sopPackage;
};

/*******************************************************************************************************************************
**                                                     CLASS (PLUGIN)                                                         **
*******************************************************************************************************************************/

PLUGIN::PLUGIN (LIBRARY* pLibrary) :
   m_nInstalled (0),
   m_bInstalled (false),
   m_pLibrary (pLibrary)
{
   m_pImpl = new PLUGIN::Impl ();
}

PLUGIN::~PLUGIN ()
{
   delete m_pImpl;
}

std::string PLUGIN::sID () const
{ 
   return m_pLibrary->sID ();
}

// These methods must be called from within mutual exclusion provided by the caller 

bool PLUGIN::Install (APP* pCore)
{
   if (m_nInstalled++ == 0)
   {
      m_bInstalled = m_pLibrary->Install (this);

      pCore->LoggerWrite (LOGGER::kLOGLEVEL_Info, LibraryRMAP::sModuleName, "Installing plugin: " + m_pLibrary->sID () + " (" + m_pLibrary->sVersion () + " )");
   }

   return m_bInstalled;
}

void PLUGIN::Unstall (APP* pCore)
{
   if (--m_nInstalled == 0)
   {
      pCore->LoggerWrite (LOGGER::kLOGLEVEL_Info, LibraryRMAP::sModuleName, "Unstalling plugin: " + m_pLibrary->sID ());

      m_pLibrary->Unstall (this);

      m_bInstalled = false;
   }
}

bool PLUGIN::AddPackage (PACKAGE::FACTORY* pFactory_Package, const std::string& sNamespace)
{
   bool bResult = true;
   PACKAGE::IREFERENCE* pReference;
   PACKAGE* pPackage;

   if ((pReference = pFactory_Package->Reference (sNamespace)) != NULL)
   {
      PACKAGE::PACKAGEPARAM* pDummy = new PACKAGE::PACKAGEPARAM ();

      if (pPackage = m_pImpl->sopPackage.Open (pReference, pDummy))
      {
         if (pPackage->IsLoaded ())
         {
            if (pDummy == m_pImpl->sopPackage.Param (pPackage->sKey ()))   // Check to see if this is First Add
            {
               if (pPackage->Install () == false)
               {
                  APP* pCore = APP::GetInstance ();

                  pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Failed to install Package : " + pPackage->sKey ());

                  bResult = false;
               }
            }

            if (bResult == false)  // Package failed to install
            {
               m_pImpl->sopPackage.Close (pPackage->sKey ());

               pPackage = NULL;
            }
         }
      }

      delete pDummy;
   }

   return bResult;
}

bool PLUGIN::InstallPackages (const std::string& sID_Service, const std::string& sNamespace, const std::string& sID_Package)
{
   bool bResult = true;
   PACKAGE::FACTORY* pFactory_Package;
   PCOLLECTION_ENUM pEnum;

   if (sID_Package.empty () == false)
   {
      if ((pFactory_Package = m_pImpl->cpFactoryPkg.Get (PACKAGE::FACTORY::toID (sID_Service, sID_Package))) != NULL)
      {
         AddPackage (pFactory_Package, sNamespace);

         m_pImpl->cpFactory.Release ();
      }
   }
   else
   {
      if ((pEnum = m_pImpl->cpFactoryPkg.Enum_Begin ()) != NULL)
      {
         while ((pFactory_Package = m_pImpl->cpFactoryPkg.Enum_Next (pEnum)) != NULL)
         {
            if (pFactory_Package->sID_Service ().compare (sID_Service) == 0)
            {
               AddPackage (pFactory_Package, sNamespace);
            }

            m_pImpl->cpFactoryPkg.Release ();
         }

         m_pImpl->cpFactoryPkg.Enum_End (pEnum);
      }
   }

   return bResult;
}

void PLUGIN::Factory_Services (std::vector<SERVICE::FACTORY*> &apFactory)
{
   for (int n = 0; n < apFactory.size (); n++)
      m_pImpl->cpFactory.Add ("service/" + apFactory[n]->sID (), apFactory[n]);
}

void PLUGIN::Factory_Models (std::vector<MODEL::FACTORY*> &apFactory)
{
   for (int n = 0; n < apFactory.size (); n++)
      m_pImpl->cpFactory.Add ("model/" + apFactory[n]->sID (), apFactory[n]);
}

void PLUGIN::Factory_Sources (std::vector<SOURCE::FACTORY*> &apFactory)
{
   for (int n = 0; n < apFactory.size (); n++)
      m_pImpl->cpFactory.Add ("source/" + apFactory[n]->pReference ()->sID_Service + ":" + apFactory[n]->pReference ()->sID_Model, apFactory[n]);
}

void PLUGIN::Factory_Packages (std::vector<PACKAGE::FACTORY*>& apFactory)
{
   for (int n = 0; n < apFactory.size (); n++)
      m_pImpl->cpFactoryPkg.Add (apFactory[n]->sID (), apFactory[n]);
}

FACTORY* PLUGIN::Factory (std::string sType, std::string sID)
{
   FACTORY* pFactory;

   if ((pFactory = m_pImpl->cpFactory.Get (sType + '/' + sID)) != NULL)
   {
      m_pImpl->cpFactory.Release ();
   }

   return pFactory;
}

/******************************************************************************************************************************/
