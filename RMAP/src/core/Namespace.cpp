/*******************************************************************************************************************************
**                                                                                                                            **
**                                                  RMAP_cpp : Namespace.cpp                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

class NAMESPACE::Impl
{
public:
   Impl (std::string sNamespace)
   {
      this->sNamespace = sNamespace;
   }

   COLLECTION<std::string, SERVICECLASS*>  cpServiceClass;
   COLLECTION<std::string, MODELCLASS*> cpModelClass;

   std::string sNamespace;
};

NAMESPACE::NAMESPACE (std::string sNamespace)
{
   m_Impl = new Impl (sNamespace);
}

NAMESPACE::~NAMESPACE ()
{
   delete m_Impl;
}

// ===== Public Properties ==================================================================================================


std::string const& NAMESPACE::sNamespace () const &
{ 
   return m_Impl->sNamespace;
}

// ===== Public Methods =====================================================================================================

bool NAMESPACE::ServiceClass_Add (SERVICE::FACTORY* pService_Factory)
{
   bool bResult = false;
   SERVICECLASS *pServiceClass;

   pServiceClass = m_Impl->cpServiceClass.Get (pService_Factory->sID (), true);
   {
      if (pServiceClass == NULL)
      {
         pServiceClass = new SERVICECLASS (this, pService_Factory);

         if (m_Impl->cpServiceClass.Add (pService_Factory->sID (), pServiceClass))
         {
            bResult = true;
         }
         else
            delete pServiceClass;
      }
      else
      {
         bResult = true; // Duplicate adds are going to be permissible, but we need to add reference counting to make sure we remove the same number
      }
   }
   m_Impl->cpServiceClass.Release ();

   return bResult;
}

bool NAMESPACE::ServiceClass_Remove (std::string sID_Service)
{
   bool bResult = false;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      if (pServiceClass->SourceClass_Length () == 0)
      {
         if ((pServiceClass = m_Impl->cpServiceClass.Remove (sID_Service)) != NULL)
         {
            delete pServiceClass;

            bResult = true;
         }
      }

      m_Impl->cpServiceClass.Release ();
   }

   return bResult;
}

// --------------------------------------------------------------------------------------------------------------------------

int NAMESPACE::ServiceClass_Length ()
{
   return m_Impl->cpServiceClass.Length ();
}

bool NAMESPACE::ServiceClass_Exists (std::string sID_Service)
{
   return m_Impl->cpServiceClass.Exists (sID_Service);
}

// Callers to ServiceClass_Get () must also call ServiceClass_Release () if the return value is not NULL
SERVICECLASS* NAMESPACE::ServiceClass_Get (std::string sID_Service)
{
   return m_Impl->cpServiceClass.Get (sID_Service);
}

// Callers to ServiceClass_Index () must also call ServiceClass_Release () if the return value is not NULL
SERVICECLASS* NAMESPACE::ServiceClass_Index (int nIndex)
{
   return m_Impl->cpServiceClass.Index (nIndex);
}

// Callers to ServiceClass_Enum () must also call ServiceClass_Release () if the return value is not NULL
SERVICECLASS* NAMESPACE::ServiceClass_Enum (fnServiceClassEnum fnEnum, void *pvParam)
{
   SERVICECLASS* pServiceClass = NULL;
   PCOLLECTION_ENUM pEnum;
   bool bResult = true;

   if (pEnum = m_Impl->cpServiceClass.Enum_Begin ())
   {
      while (bResult && (pServiceClass = m_Impl->cpServiceClass.Enum_Next (pEnum)) != NULL)
         if (bResult = fnEnum (pServiceClass, pvParam))
            m_Impl->cpServiceClass.Release ();

      m_Impl->cpServiceClass.Enum_End (pEnum);
   }

   return pServiceClass;
}

void NAMESPACE::ServiceClass_Release ()
{
   return m_Impl->cpServiceClass.Release ();
}

// ==========================================================================================================================

bool NAMESPACE::ModelClass_Add (MODEL::FACTORY* pModel_Factory)
{
   bool bResult = false;
   MODELCLASS* pModelClass;

   pModelClass = m_Impl->cpModelClass.Get (pModel_Factory->sID (), true);
   {
      if (pModelClass == NULL)
      {
         pModelClass = new MODELCLASS (this, pModel_Factory);

         if (m_Impl->cpModelClass.Add (pModel_Factory->sID (), pModelClass))
         {
            bResult = true;
         }
         else delete pModelClass;
      }
      else
      {
         bResult = true; // Duplicate adds are going to be permissible, but we need to add reference counting to make sure we remove the same number
      }
   }
   m_Impl->cpModelClass.Release ();

   return bResult;
}

bool NAMESPACE::ModelClass_Remove (std::string sID_Model)
{
   bool bResult = false;
   MODELCLASS* pModelClass;

   if ((pModelClass = m_Impl->cpModelClass.Get (sID_Model)) != NULL)
   {
      if (pModelClass->SourceClass_Length () == 0)
      {
         if ((pModelClass = m_Impl->cpModelClass.Remove (sID_Model)) != NULL)
         {
            delete pModelClass;

            bResult = true;
         }
      }

      m_Impl->cpModelClass.Release ();
   }

   return bResult;
}

// --------------------------------------------------------------------------------------------------------------------------

int NAMESPACE::ModelClass_Length ()
{
   return m_Impl->cpModelClass.Length ();
}

int NAMESPACE::ModelClass_Exists (std::string sID_Model)
{
   return m_Impl->cpModelClass.Exists (sID_Model);
}

// Callers to ModelClass_Get () must also call ModelClass_Release () if the return value is not NULL
MODELCLASS* NAMESPACE::ModelClass_Get (std::string sID_Model)
{
   return m_Impl->cpModelClass.Get (sID_Model);
}

// Callers to ModelClass_Index () must also call ModelClass_Release () if the return value is not NULL
MODELCLASS* NAMESPACE::ModelClass_Index (int nIndex)
{
   return m_Impl->cpModelClass.Index (nIndex);
}

// Callers to ModelClass_Enum () must also call ModelClass_Release () if the return value is not NULL
MODELCLASS* NAMESPACE::ModelClass_Enum (fnModelClassEnum fnEnum, void* pvParam)
{
   MODELCLASS* pModelClass = NULL;
   PCOLLECTION_ENUM pEnum;
   bool bResult = true;

   if (pEnum = m_Impl->cpModelClass.Enum_Begin ())
   {
      while (bResult && (pModelClass = m_Impl->cpModelClass.Enum_Next (pEnum)) != NULL)
         if (bResult = fnEnum (pModelClass, pvParam))
            m_Impl->cpModelClass.Release ();

      m_Impl->cpModelClass.Enum_End (pEnum);
   }

   return pModelClass;
}

void NAMESPACE::ModelClass_Release ()
{
   return m_Impl->cpModelClass.Release ();
}

// ==========================================================================================================================

bool NAMESPACE::SourceClass_Add (SOURCE::FACTORY* pSource_Factory)
{
   bool bResult = false;
   SERVICECLASS* pServiceClass;
   MODELCLASS* pModelClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (pSource_Factory->pReference ()->sID_Service)) != NULL)
   {
      if ((pModelClass = m_Impl->cpModelClass.Get (pSource_Factory->pReference ()->sID_Model)) != NULL)
      {
         bResult = pServiceClass->SourceClass_Add (pModelClass, pSource_Factory);

         m_Impl->cpModelClass.Release ();
      }

      m_Impl->cpServiceClass.Release ();
   }

   return bResult;
}

bool NAMESPACE::SourceClass_Remove (std::string sID_Service, std::string sID_Model)
{
   bool bResult = false;
   SERVICECLASS* pServiceClass;
   MODELCLASS* pModelClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      if ((pModelClass = m_Impl->cpModelClass.Get (sID_Model)) != NULL)
      {
         bResult = pServiceClass->SourceClass_Remove (pModelClass);

         m_Impl->cpModelClass.Release ();
      }

      m_Impl->cpServiceClass.Release ();
   }

   return bResult;
}

// --------------------------------------------------------------------------------------------------------------------------

int NAMESPACE::SourceClass_Length (std::string sID_Service)
{
   int nLength = -1;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      nLength = pServiceClass->SourceClass_Length ();

      m_Impl->cpServiceClass.Release ();
   }

   return nLength;
}

int NAMESPACE::SourceClass_Exists (std::string sID_Service, std::string sID_Model)
{
   int nIndex = -1;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      nIndex = pServiceClass->SourceClass_Exists (sID_Model);

      m_Impl->cpServiceClass.Release ();
   }

   return nIndex;
}

// Callers to SourceClass_Get () must also call SourceClass_Release () if the return value is not NULL
SOURCECLASS* NAMESPACE::SourceClass_Get (std::string sID_Service, std::string sID_Model)
{
   SOURCECLASS* pSourceClass = NULL;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pSourceClass = pServiceClass->SourceClass_Get (sID_Model);

      m_Impl->cpServiceClass.Release ();
   }

   return pSourceClass;
}

// Callers to SourceClass_Index () must also call SourceClass_Release () if the return value is not NULL
SOURCECLASS* NAMESPACE::SourceClass_Index (std::string sID_Service, int nIndex)
{
   SOURCECLASS* pSourceClass = NULL;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pSourceClass = pServiceClass->SourceClass_Index (nIndex);

      m_Impl->cpServiceClass.Release ();
   }

   return pSourceClass;
}

// Callers to SourceClass_Enum () must also call SourceClass_Release () if the return value is not NULL
SOURCECLASS* NAMESPACE::SourceClass_Enum (std::string sID_Service, fnSourceClassEnum fnEnum, void* pvParam)
{
   SOURCECLASS* pSourceClass = NULL;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pSourceClass = pServiceClass->SourceClass_Enum (fnEnum, pvParam);

      m_Impl->cpServiceClass.Release ();
   }

   return pSourceClass;
}

void NAMESPACE::SourceClass_Release (std::string sID_Service)
{
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pServiceClass->SourceClass_Release ();

      m_Impl->cpServiceClass.Release ();
   }
}

// ==========================================================================================================================

SERVICE* NAMESPACE::Service_Open (std::string sID_Service, std::string sConnect)
{
   SERVICE* pService = NULL;
   SERVICECLASS* pServiceClass;
   IREFERENCE<SERVICE*, NAMESPACE*>* pReference;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      if ((pReference = pServiceClass->pService_Factory ()->Reference (sConnect)) != NULL)
      {
         pService = pServiceClass->Service_Open (pReference, pServiceClass->pNamespace ());
      }

      m_Impl->cpServiceClass.Release ();
   }

   return pService;
}

SERVICE* NAMESPACE::Service_Close (SERVICE* pService)
{
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (pService->sID ())) != NULL)
   {
      pService = pServiceClass->Service_Close (pService);

      m_Impl->cpServiceClass.Release ();
   }

   return pService;
}

// --------------------------------------------------------------------------------------------------------------------------

int NAMESPACE::Service_Length (std::string sID_Service)
{
   int nLength = -1;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      nLength = pServiceClass->Service_Length ();

      m_Impl->cpServiceClass.Release ();
   }

   return nLength;
}

// Callers to Service_Index () must also call Service_Release () if the return value is not NULL
SERVICE* NAMESPACE::Service_Index (std::string sID_Service, int nIndex)
{
   SERVICE* pService = NULL;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pService = pServiceClass->Service_Index (nIndex);

      m_Impl->cpServiceClass.Release ();
   }

   return pService;
}

// Callers to Service_Enum () must also call Service_Release () if the return value is not NULL
SERVICE* NAMESPACE::Service_Enum (std::string sID_Service, fnServiceEnum fnEnum, void* pvParam)
{
   SERVICE* pService = NULL;
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pService = pServiceClass->Service_Enum (fnEnum, pvParam);

      m_Impl->cpServiceClass.Release ();
   }

   return pService;
}

void NAMESPACE::Service_Release (std::string sID_Service)
{
   SERVICECLASS* pServiceClass;

   if ((pServiceClass = m_Impl->cpServiceClass.Get (sID_Service)) != NULL)
   {
      pServiceClass->Service_Release ();

      m_Impl->cpServiceClass.Release ();
   }
}

/******************************************************************************************************************************/
