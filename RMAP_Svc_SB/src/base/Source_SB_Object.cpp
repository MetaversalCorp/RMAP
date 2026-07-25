/*******************************************************************************************************************************
**                                                                                                                            **
**                                               SVC_SB_cpp : SB_OBJECT.cpp                                                     **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class SB_OBJECT::Impl
{
public:
   Impl (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, MAP* pMap, RMAP::CORE::CLIENT* pClient) :
      pReference (pReference),
      pClient (pClient),
      pMap (pMap)
   {
      ByteStream.Resize (pMap->Size (true));
   }

   ~Impl ()
   {
   }

   RMAP::CORE::MEM::SOURCE::REFERENCE* pReference;
   RMAP::CORE::CLIENT*                 pClient;
   MAP*                          pMap;
   BYTESTREAM                    ByteStream;
};

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                      **
*******************************************************************************************************************************/

SB_OBJECT::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction, bool bIndependent, MAP* pMap) :
   RMAP::CORE::MEM::SOURCE::FACTORY (sID_Service, sID_Model, wClass, apAction, bIndependent),
   m_pMap (pMap)
{
}

SB_OBJECT::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (SB_OBJECT)                                                        **
*******************************************************************************************************************************/

SB_OBJECT::SB_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, MAP* pMap, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::MEM::SOURCE (pReference, pClient, new SVC_SB::OBJECTHEAD ())
{
   m_pImpl = new Impl (pReference, pMap, pClient);
}

SB_OBJECT::~SB_OBJECT ()
{
   delete m_pImpl;
}

void SB_OBJECT::twEventIz (uint64_t twEventIz)
{
   OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); 
   
   pOH->twEventIz = twEventIz;
}

uint64_t SB_OBJECT::twEventIz ()  { OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); return pOH->twEventIz; }
uint64_t SB_OBJECT::twObjectIx () { return pObjectHead ()->twObjectIx; }
uint64_t SB_OBJECT::twParentIx () { return pObjectHead ()->twParentIx; }
std::vector<uint8_t>& SB_OBJECT::GetData () { return m_pImpl->ByteStream.GetData (); }

void SB_OBJECT::Map_Read (RMAP::CORE::MEM::MODEL* pModel)
{
   ordered_json jData;

   m_pImpl->ByteStream.Reset ();
   m_pImpl->pMap->Read (&m_pImpl->ByteStream, jData);

   Read (jData, pModel);
}

void SB_OBJECT::Map_Write (BYTESTREAM* pByteStream, uint16_t wFlags, bool bDiscard)
{
   pByteStream->Copy (m_pImpl->ByteStream.GetData (), 0, (wFlags & SBD_OBJECT_HEAD_FLAG_SUBSCRIBE_PARTIAL) != 0 ? m_pImpl->pMap->Size (false) : m_pImpl->pMap->Size (true));
}

void SB_OBJECT::Partial ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Partial ();
}

void SB_OBJECT::Full ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Full ();
}

void SB_OBJECT::Recovering ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovering ();
}

void SB_OBJECT::Recovered ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovered ();
}

void SB_OBJECT::Inserted (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   SVC_SB::SB_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_SB::SB_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Inserted (pModelObject, pModelChild, pChange);
}

void SB_OBJECT::Deleting (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Deleting (pModelObject, pModelChild, pChange);
}

void SB_OBJECT::Updating (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Updating (pModelObject, pModelChild);
}

void SB_OBJECT::Updated (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   SVC_SB::SB_OBJECT*    pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_SB::SB_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Updated (pModelObject, pModelChild);
}

void SB_OBJECT::Changing (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = (pObject != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ()) : NULL;

   pModelSelf->Changing (pModelObject, pModelChild, pChange);
}

void SB_OBJECT::Changed (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf   = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild  = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = (pObject != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ()) : NULL;
   SVC_SB::SB_OBJECT*    pSourceObject = (pObject != NULL) ? dynamic_cast<SVC_SB::SB_OBJECT*> (pObject) : NULL;

   if (pObject != NULL)
   {
      pSourceObject->Map_Read (pModelObject);  // We really need to do selective reading based on the event described in pChange. Otherwise, we could be wasting huge amounts of time!
   }

   pModelSelf->Changed (pModelObject, pModelChild, pChange);
}

bool SB_OBJECT::Attach ()
{
   bool bResult = false;
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());

   RMAP::CORE::MEM::SOURCE::Attach ();

   if (bIndependent () != false)
   {
      pClientSB->Object_Subscribe (wClass (), pObjectHead ()->twObjectIx);

      bResult = true;
   }

   return bResult;
}

bool SB_OBJECT::Detach ()
{
   bool bResult = false;
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient ());

   if (bIndependent () != false)
   {
      pClientSB->Object_Unsubscribe (wClass (), pObjectHead ()->twObjectIx);

      bResult = true;
   }

   RMAP::CORE::MEM::SOURCE::Detach ();

   return bResult;
}

void SB_OBJECT::ResetData ()
{
// Chief Architect SAYS: This will not be used until we rethink Poker
//   m_pImpl->abData.clear ();
}

/******************************************************************************************************************************/
