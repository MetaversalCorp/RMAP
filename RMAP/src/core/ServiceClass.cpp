/*******************************************************************************************************************************
**                                                                                                                            **
**                                                 RMAP_cpp : ServiceClass.cpp                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

class SERVICECLASS::Impl
{
public:
   Impl (NAMESPACE* pNamespace, SERVICE::FACTORY* pService_Factory) :
      pNamespace (pNamespace),
      pService_Factory (pService_Factory)
   {
   }

   NAMESPACE*                            pNamespace;
   SERVICE::FACTORY*                     pService_Factory;
   SHAREDOBJECT<SERVICE*, NAMESPACE*>    sopService;
   COLLECTION<std::string, SOURCECLASS*> cpSourceClass;
};

SERVICECLASS::SERVICECLASS (NAMESPACE* pNamespace, SERVICE::FACTORY* pService_Factory)
{
   m_pImpl = new Impl (pNamespace, pService_Factory);
}

SERVICECLASS::~SERVICECLASS ()
{
   delete m_pImpl;
}

// ===== Public Properties ==================================================================================================

NAMESPACE* SERVICECLASS::pNamespace ()
{
   return m_pImpl->pNamespace;
}

SERVICE::FACTORY* SERVICECLASS::pService_Factory ()
{
   return m_pImpl->pService_Factory;
}

int SERVICECLASS::SourceClass_Length ()
{
   return m_pImpl->cpSourceClass.Length ();
}

bool SERVICECLASS::SourceClass_Add (MODELCLASS* pModelClass, SOURCE::FACTORY* pSource_Factory)
{
   bool bResult = false;
   SOURCECLASS* pSourceClass;

   pSourceClass = m_pImpl->cpSourceClass.Get (pModelClass->pModel_Factory ()->sID (), true);
   {
      if (pSourceClass == NULL)
      {
         pSourceClass = new SOURCECLASS (m_pImpl->pNamespace, pModelClass->pModel_Factory (), pSource_Factory);

         if (m_pImpl->cpSourceClass.Add (pModelClass->pModel_Factory ()->sID (), pSourceClass))
         {
            pModelClass->SourceClass_Add ();

            bResult = true;
         }
         else delete pSourceClass;
      }
      else
      {
         bResult = true; // Duplicate adds are going to be permissible, but we need to add reference counting to make sure we remove the same number
      }
   }
   m_pImpl->cpSourceClass.Release ();

   return bResult;
}

bool SERVICECLASS::SourceClass_Remove (MODELCLASS* pModelClass)
{
   bool bResult = false;
   SOURCECLASS* pSourceClass;

   if ((pSourceClass = m_pImpl->cpSourceClass.Remove (pModelClass->pModel_Factory ()->sID ())) != NULL)
   {
      pModelClass->SourceClass_Remove ();

      delete pSourceClass;

      bResult = true;
   }

   return bResult;
}

int SERVICECLASS::SourceClass_Exists (std::string sID_Model)
{
   return m_pImpl->cpSourceClass.Exists (sID_Model);
}

SOURCECLASS* SERVICECLASS::SourceClass_Get (std::string sID_Model)
{
   return m_pImpl->cpSourceClass.Get (sID_Model);
}

SOURCECLASS* SERVICECLASS::SourceClass_Index (int nIndex)
{
   return m_pImpl->cpSourceClass.Index (nIndex);
}

SOURCECLASS* SERVICECLASS::SourceClass_Enum (fnSourceClassEnum fnEnum, void* pParam)
{
   SOURCECLASS* pSourceClass = NULL;
   PCOLLECTION_ENUM pEnum;
   bool bResult = true;

   if (pEnum = m_pImpl->cpSourceClass.Enum_Begin ())
   {
      while (bResult && (pSourceClass = m_pImpl->cpSourceClass.Enum_Next (pEnum)) != NULL)
         if (bResult = fnEnum (pSourceClass, pParam))
            m_pImpl->cpSourceClass.Release ();

      m_pImpl->cpSourceClass.Enum_End (pEnum);
   }

   return pSourceClass;
}

void SERVICECLASS::SourceClass_Release ()
{
   m_pImpl->cpSourceClass.Release ();
}

SERVICE* SERVICECLASS::Service_Open (IREFERENCE<SERVICE*, NAMESPACE*>* pReference, NAMESPACE* pNamespace)
{
   return m_pImpl->sopService.Open (pReference, pNamespace); // returns pNamespace on last close
}

SERVICE* SERVICECLASS::Service_Close (SERVICE* pService)
{
   if (m_pImpl->sopService.Close (pService->sKey ()) != NULL)
      pService = NULL;

   return pService;
}

int SERVICECLASS::Service_Length ()
{
   return m_pImpl->sopService.Length ();
}

SERVICE* SERVICECLASS::Service_Index (int nIndex)
{
   return m_pImpl->sopService.Index (nIndex);
}

SERVICE* SERVICECLASS::Service_Enum (fnServiceEnum fnEnum, void* pParam)
{
   return m_pImpl->sopService.Enum (fnEnum, pParam);
}

void SERVICECLASS::Service_Release ()
{
   m_pImpl->sopService.Release ();
}
