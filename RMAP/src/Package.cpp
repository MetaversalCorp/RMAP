/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : PACKAGE.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::IMPL)                                                    **
*******************************************************************************************************************************/

class PACKAGE::Impl
{
public:
   Impl (IREFERENCE* pReference) :
      nFailure (0),
      sNamespace (pReference->sNamespace ()),
      sKey (pReference->Key ())
   {
   }

   std::string sKey;
   std::string sNamespace;
   int         nFailure;

   std::vector<SERVICE::FACTORY*>   apFactory_Service;
   std::vector<MODEL::FACTORY*>     apFactory_Model;
   std::vector<SOURCE::FACTORY*>    apFactory_Source;
};

/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::PACKAGEPARAM)                                            **
*******************************************************************************************************************************/

PACKAGE::PACKAGEPARAM::PACKAGEPARAM ()
{
}

PACKAGE::PACKAGEPARAM::~PACKAGEPARAM ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::IREFERENCE::IMPL)                                        **
*******************************************************************************************************************************/

class PACKAGE::IREFERENCE::Impl
{
public:
   Impl (const std::string& _sID, const std::string& _sNamespace, const std::vector<std::string>& _aService, const std::vector<std::string>& _aModel, const std::vector<std::string>& _aSource) :
      sNamespace  (_sNamespace),
      aService    (_aService),
      aModel      (_aModel),
      aSource     (_aSource)
   {
   }

   std::string                sNamespace;
   std::vector<std::string>   aService;
   std::vector<std::string>   aModel;
   std::vector<std::string>   aSource;
};

/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::IREFERENCE)                                              **
*******************************************************************************************************************************/

PACKAGE::IREFERENCE::IREFERENCE (const std::string& sID, const std::string& sNamespace, const std::vector<std::string>& aService, const std::vector<std::string>& aModel, const std::vector<std::string>& aSource) :
   RMAP::CORE::IREFERENCE<PACKAGE*, PACKAGEPARAM*> (sID),
   m_pImpl (new PACKAGE::IREFERENCE::Impl (sID, sNamespace, aService, aModel, aSource))
{
}

PACKAGE::IREFERENCE::~IREFERENCE ()
{
}

std::string PACKAGE::IREFERENCE::Key ()
{
   return UniqueId () + std::string ("/") + m_pImpl->sNamespace;
}

std::string const& PACKAGE::IREFERENCE::sNamespace () const&
{
   return m_pImpl->sNamespace;
}

std::vector<std::string> const& PACKAGE::IREFERENCE::aService () const&
{
   return m_pImpl->aService;
}

std::vector<std::string> const& PACKAGE::IREFERENCE::aModel () const&
{
   return m_pImpl->aModel;
}

std::vector<std::string> const& PACKAGE::IREFERENCE::aSource () const&
{
   return m_pImpl->aSource;
}

/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::FACTORY::Impl)                                           **
*******************************************************************************************************************************/

class PACKAGE::FACTORY::Impl
{
public:
   Impl (const std::string& sID_Service, const std::string& sID_Package, const std::vector<std::string>& aService, const std::vector<std::string>& aModel, const std::vector<std::string>& aSource) :
      sID_Service (sID_Service),
      asService   (aService),
      asModel     (aModel),
      asSource    (aSource)
   {
      sID = PACKAGE::FACTORY::toID (sID_Service, sID_Package);
   }

   std::string               sID;

   std::string               sID_Service;
   std::vector<std::string>  asService;
   std::vector<std::string>  asModel;
   std::vector<std::string>  asSource;
};


/*******************************************************************************************************************************
**                                                   CLASS (PACKAGE::FACTORY)                                                 **
*******************************************************************************************************************************/

PACKAGE::FACTORY::FACTORY (const std::string& sID_Service, const std::string& sID_Package, const std::vector<std::string>& aService, const std::vector<std::string>& aModel, const std::vector<std::string>& aSource) :
   RMAP::CORE::FACTORY ()
{
   m_pImpl = new PACKAGE::FACTORY::FACTORY::Impl (sID_Service, sID_Package, aService, aModel, aSource);
}

PACKAGE::FACTORY::~FACTORY ()
{
}

std::string const& PACKAGE::FACTORY::sID () const&
{
   return m_pImpl->sID;
}

std::string const& PACKAGE::FACTORY::sID_Service () const&
{
   return m_pImpl->sID_Service;
}

std::vector<std::string> const& PACKAGE::FACTORY::aService () const&
{
   return m_pImpl->asService;
}

std::vector<std::string> const& PACKAGE::FACTORY::aModel () const&
{
   return m_pImpl->asModel;
}

std::vector<std::string> const& PACKAGE::FACTORY::aSource () const&
{
   return m_pImpl->asSource;
}

std::string PACKAGE::FACTORY::toID (const std::string& sID_Service, const std::string& sID_Package)
{
   return sID_Service + ":" + sID_Package;
}

/*******************************************************************************************************************************
**                                                     CLASS (PACKAGE)                                                        **
*******************************************************************************************************************************/

PACKAGE::PACKAGE (IREFERENCE* pReference, PACKAGEPARAM* pParam)
{
   int n;
   APP* pCore;
   RMAP::CORE::FACTORY* pFactory;
   std::vector<std::string> aTmpIds;

   m_pImpl = new PACKAGE::Impl (pReference);

   pCore = APP::GetInstance ();

   aTmpIds = pReference->aService ();
   for (n=0; n < aTmpIds.size (); n++)
   {
      if (pFactory = pCore->Plugin_Factory ("service", aTmpIds[n]))
      {
         m_pImpl->apFactory_Service.push_back (dynamic_cast<SERVICE::FACTORY*>(pFactory));
         // increment reference count
      }
      else
      {
         m_pImpl->nFailure++;
         pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Unknown service: " + aTmpIds[n]);
      }
   }

   aTmpIds = pReference->aModel ();
   for (n=0; n < aTmpIds.size (); n++)
   {
      if (pFactory = pCore->Plugin_Factory ("model", aTmpIds[n]))
      {
         m_pImpl->apFactory_Model.push_back (dynamic_cast<MODEL::FACTORY*>(pFactory));
         // increment reference count
      }
      else
      {
         m_pImpl->nFailure++;
         pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Unknown model: " + aTmpIds[n]);
      }
   }

   aTmpIds = pReference->aSource ();
   for (n=0; n < aTmpIds.size (); n++)
   {
      if (pFactory = pCore->Plugin_Factory ("source", aTmpIds[n]))
      {
         m_pImpl->apFactory_Source.push_back (dynamic_cast<SOURCE::FACTORY*>(pFactory));
         // increment reference count
      }
      else
      {
         m_pImpl->nFailure++;
         pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Unknown source: " + aTmpIds[n]);
      }
   }
}

PACKAGE::~PACKAGE ()
{
   while (m_pImpl->apFactory_Source.size () > 0)
   {
      // decrement reference count
      m_pImpl->apFactory_Source.erase (m_pImpl->apFactory_Source.begin ());
   }

   while (m_pImpl->apFactory_Model.size () > 0)
   {
      // decrement reference count
      m_pImpl->apFactory_Model.erase (m_pImpl->apFactory_Model.begin ());
   }

   while (m_pImpl->apFactory_Service.size () > 0)
   {
      // decrement reference count
      m_pImpl->apFactory_Service.erase (m_pImpl->apFactory_Service.begin ());
   }
}

bool PACKAGE::IsLoaded ()
{
   return (m_pImpl->nFailure == 0);
}

std::string const& PACKAGE::sKey () const&
{
   return m_pImpl->sKey;
}

bool PACKAGE::Install ()
{
   bool bResult = true;
   NAMESPACE* pNamespace;
   int n;
   APP* pCore = APP::GetInstance ();

   if ((pNamespace = pCore->Namespace_Add (m_pImpl->sNamespace)) != NULL)
   {
      for (n = 0; n < m_pImpl->apFactory_Service.size (); n++)
         if (pNamespace->ServiceClass_Add (m_pImpl->apFactory_Service[n]) == false)
         {
            APP* pCore = APP::GetInstance ();

            bResult = false;
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to install service: " + m_pImpl->apFactory_Service[n]->sID ());
         }

      for (n = 0; n < m_pImpl->apFactory_Model.size (); n++)
         if (pNamespace->ModelClass_Add (m_pImpl->apFactory_Model[n]) == false)
         {
            APP* pCore = APP::GetInstance ();

            bResult = false;
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to install model: " + m_pImpl->apFactory_Model[n]->sID ());
         }

      for (n = 0; n < m_pImpl->apFactory_Source.size (); n++)
         if (pNamespace->SourceClass_Add (m_pImpl->apFactory_Source[n]) == false)
         {
            APP* pCore = APP::GetInstance ();

            bResult = false;
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to install source: " + m_pImpl->apFactory_Source[n]->pReference ()->sID_Model);
         }

      pCore->Namespace_Release ();
   }

   return bResult;
}

void PACKAGE::Unstall ()
{
   NAMESPACE* pNamespace;
   int n;
   APP* pCore = APP::GetInstance ();

   if ((pNamespace = pCore->Namespace_Get (m_pImpl->sNamespace)) != NULL)
   {
      for (n = 0; n < m_pImpl->apFactory_Source.size (); n++)
         if (pNamespace->SourceClass_Remove (m_pImpl->apFactory_Source[n]->pReference ()->sID_Service, m_pImpl->apFactory_Source[n]->pReference ()->sID_Model) == false)
         {
            APP* pCore = APP::GetInstance ();
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to unstall source: " + m_pImpl->apFactory_Source[n]->pReference ()->sID_Service + " > " + m_pImpl->apFactory_Source[n]->pReference ()->sID_Model);
         }

      for (n = 0; n < m_pImpl->apFactory_Model.size (); n++)
         if (pNamespace->ModelClass_Remove (m_pImpl->apFactory_Model[n]->sID ()) == false)
         {
            APP* pCore = APP::GetInstance ();
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to unstall model: " + m_pImpl->apFactory_Model[n]->sID ());
         }

      for (n = 0; n < m_pImpl->apFactory_Service.size (); n++)
         if (pNamespace->ServiceClass_Remove (m_pImpl->apFactory_Service[n]->sID ()) == false)
         {
            APP* pCore = APP::GetInstance ();
            pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Package: Failed to unstall service: " + m_pImpl->apFactory_Service[n]->sID ());
         }

      pCore->Namespace_Release ();
   }
}

/******************************************************************************************************************************/
