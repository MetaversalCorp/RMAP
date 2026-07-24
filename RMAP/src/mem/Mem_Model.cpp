/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Mem_Model.cpp                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (CORE::Impl)                                                     **
*******************************************************************************************************************************/

class MODEL::Impl
{
public:
   Impl (SOURCE* pSource)
   {
   }

   ~Impl ()
   {
   }

private:
};

/*******************************************************************************************************************************
**                                                     CLASS (MEM)                                                            **
*******************************************************************************************************************************/

MODEL::MODEL (IREFERENCE* pReference, SOURCE* pSource) :
   RMAP::CORE::MODEL (pReference, pSource)
{
   m_pImpl = new Impl (pSource);
   pSource->initialize (this, pReference->twObjectIx, pReference->twChildIx);
}

MODEL::~MODEL ()
{
   delete m_pImpl;
}

// ===== Public Properties ==================================================================================================

uint64_t MODEL::twParentIx () 
{
   SOURCE* pSourceX = dynamic_cast <SOURCE*> (pSource ());

   return pSourceX->twParentIx ();
}

uint64_t MODEL::twObjectIx ()
{ 
   SOURCE* pSourceX = dynamic_cast <SOURCE*> (pSource ());

   return pSourceX->twObjectIx ();
}

uint16_t MODEL::wClass_Parent ()
{
   SOURCE* pSourceX = dynamic_cast <SOURCE*> (pSource ());

   return pSourceX->wClass_Parent ();
}

uint16_t MODEL::wClass_Object ()
{
   SOURCE* pSourceX = dynamic_cast <SOURCE*> (pSource ());

   return pSourceX->wClass_Object ();
}

/*******************************************************************************************************************************
**                                                     CLASS (MEM::IREFERENCE)                                                **
*******************************************************************************************************************************/

MODEL::IREFERENCE::IREFERENCE (const std::string& sID, uint64_t twObjectIx, uint64_t twChildIx) :
   RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*> (sID),
   twObjectIx (twObjectIx),
   twChildIx (twChildIx)
{
}

MODEL::IREFERENCE::~IREFERENCE ()
{

}

std::string MODEL::IREFERENCE::Key ()
{
   return std::to_string (twObjectIx) + (twChildIx ? "_" + std::to_string (twChildIx) : "");
}
