/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Objectbank_Dep.cpp                                            **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (CORE::Impl)                                                     **
*******************************************************************************************************************************/

class OBJECTBANK_DEP::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }

   std::map<uint64_t, std::map<uint64_t, SOURCE*>>              aParent;
};

/*******************************************************************************************************************************
**                                                     CLASS (Objectbank)                                                     **
*******************************************************************************************************************************/

OBJECTBANK_DEP::OBJECTBANK_DEP (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory) :
   OBJECTBANK (pMem, pModel_Factory, pSource_Factory)
{
   m_pImpl = new OBJECTBANK_DEP::Impl ();
}

OBJECTBANK_DEP::~OBJECTBANK_DEP ()
{
   for (auto const& x : m_pImpl->aParent)
   {
      Parent_Close (x.first);
   }
}

bool OBJECTBANK_DEP::bIndependent ()
{
   return false;
}

std::string OBJECTBANK_DEP::MakeArgs (uint64_t twParentIx, uint64_t twObjectIx)
{
   return std::to_string (twParentIx) + "," + std::to_string (twObjectIx);
}

std::map<uint64_t, SOURCE*>& OBJECTBANK_DEP::Parent_Open (uint64_t twParentIx)
{
   std::map<uint64_t, SOURCE*> v;

   if (m_pImpl->aParent.find (twParentIx) == m_pImpl->aParent.end ())
      m_pImpl->aParent[twParentIx] = v;

   return m_pImpl->aParent[twParentIx];
}

void OBJECTBANK_DEP::Parent_Close (uint64_t twParentIx)
{
   auto itParent = m_pImpl->aParent.find (twParentIx);

   if (itParent != m_pImpl->aParent.end ())
   {
      do
      {
         auto it = itParent->second.begin ();

         if (it != itParent->second.end ())
            Object_Close (it->second);
      }
      while (itParent->second.size () > 0);
   }
}

uint64_t OBJECTBANK_DEP::Count (SOURCE* pParent)
{
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pParent->pObjectHead ()->Self.ObjectIx ());

   return apObject.size ();
}

// --------------------------------------------------------------------------------------------------------------------------

SOURCE* OBJECTBANK_DEP::Get (SOURCE* pParent, uint64_t twObjectIx)
{
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pParent->pObjectHead ()->Self.ObjectIx ());
   SOURCE* pObject = NULL;

   if (apObject.find (twObjectIx) != apObject.end ())
   {
      pObject = apObject[twObjectIx];
   }

   return pObject;
}

SOURCE* OBJECTBANK_DEP::Index (SOURCE* pParent, int64_t nIndex)
{
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pParent->pObjectHead ()->Self.ObjectIx ());
   SOURCE* pObject = NULL;

   for (auto const& x : apObject)
   {
      if (nIndex-- < 0)
         break;

      pObject = x.second;
   }

   return pObject;
}

SOURCE* OBJECTBANK_DEP::Next (SOURCE* pParent, uint64_t twObjectIx)
{
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pParent->pObjectHead ()->Self.ObjectIx ());
   SOURCE* pObject = NULL;

   if (twObjectIx >= OBJECTBANK::OBJECTIX_NULL && twObjectIx < OBJECTBANK::OBJECTIX_MAX)
   {
      auto it = std::find_if (apObject.begin (), apObject.end (),
         [&](const auto& pair)
         {
            return (pair.first > twObjectIx);
         });

      if (it != apObject.end ())
         pObject = it->second;
   }

   return pObject;
}

int OBJECTBANK_DEP::Enum (SOURCE* pParent, IOBJECTBANK* pIObjectBank, void* pParam)
{
   int nResult = -1;
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pParent->pObjectHead ()->Self.ObjectIx ());

   for (auto const& x : apObject)
   {
      if ((nResult = pIObjectBank->onObjectBankItem (x.second, pParam)) != 0)
         break;
   }

   return nResult;
}

// --------------------------------------------------------------------------------------------------------------------------

bool OBJECTBANK_DEP::Insert (SOURCE* pObject)
{
   bool bResult = false;
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pObject->pObjectHead ()->Parent.ObjectIx ());

   if (pObject->pObjectHead ()->Parent.ObjectIx () > OBJECTBANK::OBJECTIX_NULL && pObject->pObjectHead ()->Parent.ObjectIx () < OBJECTBANK::OBJECTIX_MAX)
   {
      if (pObject->pObjectHead ()->Self.ObjectIx () > OBJECTBANK::OBJECTIX_NULL && pObject->pObjectHead ()->Self.ObjectIx () < OBJECTBANK::OBJECTIX_MAX)
      {
         if (apObject.find (pObject->pObjectHead ()->Self.ObjectIx ()) == apObject.end ())
         {
            apObject[pObject->pObjectHead ()->Self.ObjectIx ()] = pObject;

            bResult = true;
         }
      }
   }

   return bResult;
}

bool OBJECTBANK_DEP::Delete (SOURCE* pObject)
{
   bool bResult = false;
   std::map<uint64_t, SOURCE*> apObject = Parent_Open (pObject->pObjectHead ()->Parent.ObjectIx ());
   uint64_t twObjectIx = pObject->pObjectHead ()->Self.ObjectIx ();

   if (apObject.find (twObjectIx) != apObject.end ())
   {
      delete apObject[twObjectIx];

      bResult = true;
   }

   return bResult;
}
