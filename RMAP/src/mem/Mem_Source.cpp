/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_js : Mem_Source.js                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (OBJECTHEAD)                                                     **
*******************************************************************************************************************************/

OBJECTHEAD::OBJECTHEAD () :
   Parent ({}),
   Self ({}),
   wFlags (0)
{
}

OBJECTHEAD::OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags) :
   wFlags (wFlags)
{
   Parent.qwComposed = OBJECTIX_COMPOSE (wClass_Parent, twParentIx);
   Self  .qwComposed = OBJECTIX_COMPOSE (wClass_Object, twObjectIx);
}

OBJECTHEAD::~OBJECTHEAD ()
{
}

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE::Impl)                                                   **
*******************************************************************************************************************************/

class SOURCE::Impl
{
public:
   Impl (bool bIndependent, OBJECTHEAD* pObjectHead) :
      bIndependent (bIndependent)
   {
      this->pObjectHead = pObjectHead;
   }

   ~Impl ()
   {
      delete pObjectHead;
   }

   bool bIndependent;
   OBJECTHEAD* pObjectHead;
};

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE)                                                         **
*******************************************************************************************************************************/

SOURCE::SOURCE (REFERENCE* pReference, RMAP::CORE::CLIENT* pClient, OBJECTHEAD* pObjectHead) :
   RMAP::CORE::SOURCE (pReference, pClient)
{
   m_pImpl = new SOURCE::Impl (pReference->bIndependent != false, pObjectHead);
}

SOURCE::~SOURCE ()
{
   delete m_pImpl;
}

void SOURCE::initialize (MODEL* pModel, uint64_t twObjectIx, uint64_t twChildIx)
{
   RMAP::CORE::SOURCE::initialize (pModel);

   m_pImpl->pObjectHead->Parent.qwComposed = OBJECTIX_COMPOSE (m_pImpl->pObjectHead->Parent.Class (), m_pImpl->bIndependent ? 0 : twObjectIx);
   m_pImpl->pObjectHead->Self  .qwComposed = OBJECTIX_COMPOSE (wClass (), m_pImpl->bIndependent ? twObjectIx : twChildIx);
}

bool        SOURCE::bIndependent () { return m_pImpl->bIndependent; }

// DEPRECATE THIS
OBJECTHEAD* SOURCE::pObjectHead ()  { return m_pImpl->pObjectHead;                     }
uint64_t SOURCE::twParentIx ()      { return m_pImpl->pObjectHead->Parent.ObjectIx (); }
uint64_t SOURCE::twObjectIx ()      { return m_pImpl->pObjectHead->Self.ObjectIx ();   }
uint16_t SOURCE::wClass_Parent ()   { return m_pImpl->pObjectHead->Parent.Class ();    }
uint16_t SOURCE::wClass_Object ()   { return m_pImpl->pObjectHead->Self.Class ();      }

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE::REFERENCE)                                              **
*******************************************************************************************************************************/

SOURCE::REFERENCE::REFERENCE (std::string sID_Service, std::string sID_Model, uint16_t wClass, std::map<std::string, const CLIENT::ACTION*> &apAction, bool bIndependent) :
   RMAP::CORE::SOURCE::REFERENCE (sID_Service, sID_Model, wClass, apAction),
   bIndependent (bIndependent)
{
}

SOURCE::REFERENCE::~REFERENCE ()
{
}

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE::FACTORY)                                                **
*******************************************************************************************************************************/

SOURCE::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, uint16_t wClass, std::map<std::string, const CLIENT::ACTION*> &apAction, bool bIndependent) :
   RMAP::CORE::SOURCE::FACTORY (sID_Service, sID_Model, wClass, apAction)
{
   pReference = new REFERENCE (sID_Service, sID_Model, wClass, apAction, bIndependent);
}

SOURCE::FACTORY::~FACTORY ()
{
   delete pReference;
}

/******************************************************************************************************************************/
