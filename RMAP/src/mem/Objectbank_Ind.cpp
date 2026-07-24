/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Objectbank_Ind.cpp                                            **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (CORE::Impl)                                                     **
*******************************************************************************************************************************/

class OBJECTBANK_IND::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }

   std::map<uint64_t, SOURCE*>              mpObject;
};

/*******************************************************************************************************************************
**                                                     CLASS (Objectbank)                                                     **
*******************************************************************************************************************************/

OBJECTBANK_IND::OBJECTBANK_IND (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory) :
   OBJECTBANK (pMem, pModel_Factory, pSource_Factory)
{
   m_pImpl = new OBJECTBANK_IND::Impl ();
}

OBJECTBANK_IND::~OBJECTBANK_IND ()
{
   SOURCE* pSource;

   while ((pSource = Index (NULL, 0)) != NULL)
   {
      Object_Close (pSource);
   }
}

bool OBJECTBANK_IND::bIndependent ()
{
   return true;
}

// ===== Private Methods ====================================================================================================

std::string OBJECTBANK_IND::MakeArgs (uint64_t twParentIx, uint64_t twObjectIx)
{
   return std::to_string (twObjectIx);
}

// ===== Public Methods =====================================================================================================

uint64_t OBJECTBANK_IND::Count (SOURCE* pParent)
{
   return (int)m_pImpl->mpObject.size ();
}

// --------------------------------------------------------------------------------------------------------------------------

// pParent is not used for this function in this object bank
SOURCE* OBJECTBANK_IND::Get (SOURCE* pParent, uint64_t twObjectIx)
{
   auto it = m_pImpl->mpObject.find (twObjectIx);

   return (it != m_pImpl->mpObject.end ()) ? it->second : NULL;
}

SOURCE* OBJECTBANK_IND::Index (SOURCE* pParent, int64_t nIndex)
{
   SOURCE* pObject = NULL;

   // pParent is not used for this function in this object bank

   if (nIndex < 0)
      nIndex = 0;

   if ((size_t)nIndex < m_pImpl->mpObject.size ())
   {
      auto it = m_pImpl->mpObject.begin ();

      std::advance (it, nIndex);
      pObject = it->second;
   }
      
   return pObject;
}

SOURCE* OBJECTBANK_IND::Next (SOURCE* pParent, uint64_t twObjectIx)
{
   SOURCE* pObject = NULL;

   if (twObjectIx >= OBJECTBANK::OBJECTIX_NULL && twObjectIx < OBJECTBANK::OBJECTIX_MAX)
   {
      auto it = std::find_if (m_pImpl->mpObject.begin (), m_pImpl->mpObject.end (),
         [&](const auto& pair) 
         {
            return (pair.first > twObjectIx && (pParent == NULL || (pParent->pObjectHead ()->wClass_Object == pair.second->pObjectHead ()->wClass_Parent && pParent->pObjectHead ()->twObjectIx == pair.second->pObjectHead ()->twParentIx)));
         });

      if (it != m_pImpl->mpObject.end ())
         pObject = it->second;
   }

   return pObject;
}

int OBJECTBANK_IND::Enum (SOURCE* pParent, IOBJECTBANK* pIObjectBank, void* pParam)
{
   int nResult = 0;

   for (auto it = m_pImpl->mpObject.begin (); it != m_pImpl->mpObject.end ();)
   {
      auto itObject = it++;
      SOURCE* pObject = itObject->second;

      if (pParent == NULL || (pParent->pObjectHead ()->wClass_Object == pObject->pObjectHead ()->wClass_Parent && pParent->pObjectHead ()->twObjectIx == pObject->pObjectHead ()->twParentIx))
      {
         nResult = pIObjectBank->onObjectBankItem (pObject, pParam);

         // see note above regarding collections and deleting during enuming ...
      }
   }

   return nResult;
}

// --------------------------------------------------------------------------------------------------------------------------

bool OBJECTBANK_IND::Insert (SOURCE* pObject)
{
   bool bResult = false;

   if (pObject->pObjectHead ()->twParentIx > OBJECTBANK::OBJECTIX_NULL && pObject->pObjectHead ()->twParentIx < OBJECTBANK::OBJECTIX_MAX)
   {
      if (pObject->pObjectHead ()->twObjectIx > OBJECTBANK::OBJECTIX_NULL && pObject->pObjectHead ()->twObjectIx < OBJECTBANK::OBJECTIX_MAX)
      {
         if (m_pImpl->mpObject.count (pObject->pObjectHead ()->twObjectIx) == 0)
         {
            bResult = true;

            m_pImpl->mpObject.insert ({ pObject->pObjectHead ()->twObjectIx, pObject });
         }
      }
   }

   return bResult;
}

bool OBJECTBANK_IND::Delete (SOURCE* pObject)
{
   return (m_pImpl->mpObject.erase (pObject->pObjectHead ()->twObjectIx) == 1);
}

/******************************************************************************************************************************/
