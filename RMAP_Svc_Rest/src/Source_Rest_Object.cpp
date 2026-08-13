/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVRest_cpp : Source_Rest_Object.cpp                                          **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_REST;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class REST_OBJECT::Impl
{
public:
   Impl (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
      pReference (pReference),
      pClient (pClient)
   {
   }

   ~Impl ()
   {
   }

   RMAP::CORE::MEM::SOURCE::REFERENCE* pReference;
   RMAP::CORE::CLIENT*                 pClient;
};

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                      **
*******************************************************************************************************************************/

REST_OBJECT::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction, bool bIndependent) :
   RMAP::CORE::MEM::SOURCE::FACTORY (sID_Service, sID_Model, wClass, apAction, bIndependent)
{
}

REST_OBJECT::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (REST_OBJECT::OBJECTHEAD)                                          **
*******************************************************************************************************************************/

REST_OBJECT::OBJECTHEAD::OBJECTHEAD () :
   twEventIz (0)
{
}

REST_OBJECT::OBJECTHEAD::OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz) :
   RMAP::CORE::MEM::OBJECTHEAD (twParentIx, twObjectIx, wClass_Parent, wClass_Object, wFlags),
   twEventIz (twEventIz)
{
}

REST_OBJECT::OBJECTHEAD::OBJECTHEAD (REST_OBJECT::OBJECTHEAD const& other) :
   RMAP::CORE::MEM::OBJECTHEAD (other),
   twEventIz (other.twEventIz)
{
}

REST_OBJECT::OBJECTHEAD::OBJECTHEAD (REST_OBJECT::OBJECTHEAD&& other) noexcept :
   RMAP::CORE::MEM::OBJECTHEAD (std::move (other)),
   twEventIz (other.twEventIz)
{
}

REST_OBJECT::OBJECTHEAD& REST_OBJECT::OBJECTHEAD::operator=(REST_OBJECT::OBJECTHEAD const& rhs) &
{
   if (this != &rhs)
   {
      RMAP::CORE::MEM::OBJECTHEAD::operator=(rhs);

      twEventIz = rhs.twEventIz;
   }
   return *this;
}

REST_OBJECT::OBJECTHEAD& REST_OBJECT::OBJECTHEAD::operator=(REST_OBJECT::OBJECTHEAD&& rhs) & noexcept = default;

REST_OBJECT::OBJECTHEAD::~OBJECTHEAD ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (REST_OBJECT)                                                        **
*******************************************************************************************************************************/

REST_OBJECT::REST_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::MEM::SOURCE (pReference, pClient, new REST_OBJECT::OBJECTHEAD ())
{
   m_pImpl = new Impl (pReference, pClient);
}

REST_OBJECT::~REST_OBJECT ()
{
   CLIENT* pClientREST = dynamic_cast<CLIENT*> (pClient ());

   pClientREST->IAction_AbortAll (this);

   delete m_pImpl;
}

void REST_OBJECT::twEventIz (uint64_t twEventIz)
{
   OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); 
   
   pOH->twEventIz = twEventIz;
}

uint64_t REST_OBJECT::twEventIz ()  { OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); return pOH->twEventIz; }
uint64_t REST_OBJECT::twObjectIx () { return pObjectHead ()->Self.ObjectIx (); }
uint64_t REST_OBJECT::twParentIx () { return pObjectHead ()->Parent.ObjectIx (); }
//std::vector<uint8_t>& REST_OBJECT::GetData () { return m_pImpl->ByteStream.GetData (); }

void REST_OBJECT::Map_Read (RMAP::CORE::MEM::MODEL* pModel)
{
/*
   ordered_json jData;

   m_pImpl->pMap->Read (&m_pImpl->ByteStream, jData);

   Read (jData, pModel);
*/
}

void REST_OBJECT::Map_Write (void* pvData, uint16_t wFlags, bool bDiscard)
{
//   pByteStream->Copy (m_pImpl->ByteStream.GetData (), 0, (bFlags & SBD_OBJECT_HEAD_FLAG_SUBSCRIBE_PARTIAL) != 0 ? m_pImpl->pMap->Size (false) : m_pImpl->pMap->Size (true));
}

RMAP::CORE::CLIENT::IACTION* REST_OBJECT::Request (std::string sAction)
{
   CLIENT* pClientREST = dynamic_cast<CLIENT*> (pClient ());
   RMAP::CORE::CLIENT::IACTION* pIAction;

   if ((pIAction = RMAP::CORE::MEM::SOURCE::Request (sAction)) != NULL)
   {
      RMAP::SVC_REST::CLIENT::IACTION* pIActionRest = reinterpret_cast<RMAP::SVC_REST::CLIENT::IACTION*> (pIAction);

      pClientREST->IAction_Add (pIActionRest, this);
   }

   return pIAction;
}

void REST_OBJECT::Partial ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Partial ();
}

void REST_OBJECT::Full ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Full ();
}

void REST_OBJECT::Recovering ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovering ();
}

void REST_OBJECT::Recovered ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovered ();
}

void REST_OBJECT::Inserted (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   SVC_REST::REST_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_REST::REST_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Inserted (pModelObject, pModelChild, pChange);
}

void REST_OBJECT::Deleting (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Deleting (pModelObject, pModelChild, pChange);
}

void REST_OBJECT::Updating (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Updating (pModelObject, pModelChild);
}

void REST_OBJECT::Updated (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   SVC_REST::REST_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_REST::REST_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Updated (pModelObject, pModelChild);
}

void REST_OBJECT::Changing (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = (pObject != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ()) : NULL;

   pModelSelf->Changing (pModelObject, pModelChild, pChange);
}

void REST_OBJECT::Changed (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = (pObject != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ()) : NULL;
   SVC_REST::REST_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_REST::REST_OBJECT*> (pObject) : NULL;

   if (pObject != NULL)
   {
      pSourceObject->Map_Read (pModelObject);  // We really need to do selective reading based on the event described in pChange. Otherwise, we could be wasting huge amounts of time!
   }

   pModelSelf->Changed (pModelObject, pModelChild, pChange);
}

bool REST_OBJECT::Attach ()
{
   bool bResult = false;

   RMAP::CORE::MEM::SOURCE::Attach ();

   bResult = true;

   return bResult;
}

bool REST_OBJECT::Detach ()
{
   bool bResult = false;
//   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());

//   this.pClient.pMem.Object_Delete_Full (this);
//   pClientSB->Object_Unsubscribe (wClass (), pObjectHead ()->twObjectIx);

   bResult = true;

   RMAP::CORE::MEM::SOURCE::Detach ();

   return bResult;
}

void REST_OBJECT::ResetData ()
{
// Chief Architect SAYS: This will not be used until we rethink Poker
//   m_pImpl->abData.clear ();
}

/******************************************************************************************************************************/
