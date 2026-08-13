/*******************************************************************************************************************************
**                                                                                                                            **
**                                          RMAP_Svc_SocketIO : Source_IO_Object.cpp                                          **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2026 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class IO_OBJECT::Impl : public RMAP::CORE::IRESPONSE
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient)
   {
   }

   ~Impl ()
   {
   }

   void onResponse (RMAP::CORE::CLIENT::IACTION* pIAction, int nType, intptr_t pParam) override
   {
   }

   CLIENT*                       pClient;
   ordered_json                  jData;
};

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                      **
*******************************************************************************************************************************/

IO_OBJECT::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction, bool bIndependent) :
   RMAP::CORE::MEM::SOURCE::FACTORY (sID_Service, sID_Model, wClass, apAction, bIndependent)
{
}

IO_OBJECT::FACTORY::~FACTORY ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (IO_OBJECT::OBJECTHEAD)                                          **
*******************************************************************************************************************************/

IO_OBJECT::OBJECTHEAD::OBJECTHEAD () :
   twEventIz (0)
{
}

IO_OBJECT::OBJECTHEAD::OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz) :
   RMAP::CORE::MEM::OBJECTHEAD (twParentIx, twObjectIx, wClass_Parent, wClass_Object, wFlags),
   twEventIz (twEventIz)
{
}

IO_OBJECT::OBJECTHEAD::OBJECTHEAD (IO_OBJECT::OBJECTHEAD const& other) :
   RMAP::CORE::MEM::OBJECTHEAD (other),
   twEventIz (other.twEventIz)
{
}

IO_OBJECT::OBJECTHEAD::OBJECTHEAD (IO_OBJECT::OBJECTHEAD&& other) noexcept :
   RMAP::CORE::MEM::OBJECTHEAD (std::move (other)),
   twEventIz (other.twEventIz)
{
}

IO_OBJECT::OBJECTHEAD& IO_OBJECT::OBJECTHEAD::operator=(IO_OBJECT::OBJECTHEAD const& rhs)&
{
   if (this != &rhs)
   {
      RMAP::CORE::MEM::OBJECTHEAD::operator=(rhs);

      twEventIz = rhs.twEventIz;
   }
   return *this;
}

IO_OBJECT::OBJECTHEAD& IO_OBJECT::OBJECTHEAD::operator=(IO_OBJECT::OBJECTHEAD&& rhs) & noexcept = default;

IO_OBJECT::OBJECTHEAD::~OBJECTHEAD ()
{
}

/*******************************************************************************************************************************
**                                                   CLASS (IO_OBJECT)                                                        **
*******************************************************************************************************************************/

IO_OBJECT::IO_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient) :
   RMAP::CORE::MEM::SOURCE (pReference, pClient, new IO_OBJECT::OBJECTHEAD ())
{
   m_pImpl = new Impl (dynamic_cast<CLIENT*> (pClient));
}

IO_OBJECT::~IO_OBJECT ()
{
   delete m_pImpl;
}

void IO_OBJECT::twEventIz (uint64_t twEventIz)
{
   OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); 
   
   pOH->twEventIz = twEventIz;
}

void IO_OBJECT::SetData (ordered_json& jData)
{
   m_pImpl->jData = jData;
}

ordered_json& IO_OBJECT::GetData ()
{
   return m_pImpl->jData;
}

uint64_t IO_OBJECT::twEventIz ()  { OBJECTHEAD* pOH = dynamic_cast<OBJECTHEAD*> (pObjectHead ()); return pOH->twEventIz; }
uint64_t IO_OBJECT::twObjectIx () { return pObjectHead ()->Self.ObjectIx ();   }
uint64_t IO_OBJECT::twParentIx () { return pObjectHead ()->Parent.ObjectIx (); }

void IO_OBJECT::Map_Read (RMAP::CORE::MEM::MODEL* pModel)
{
   Read (m_pImpl->jData, pModel);
}

void IO_OBJECT::Map_Write (void* pvData, uint16_t wFlags, bool bDiscard)
{
   ordered_json* pjData = (ordered_json*)pvData;

   m_pImpl->jData = *pjData;
}

void IO_OBJECT::Partial ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Partial ();
}

void IO_OBJECT::Full ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Full ();
}

void IO_OBJECT::Recovering ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovering ();
}

void IO_OBJECT::Recovered ()
{
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());

   pModelObject->Recovered ();
}

void IO_OBJECT::Inserted (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   RMAP::SVC_SOCKETIO::IO_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<RMAP::SVC_SOCKETIO::IO_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Inserted (pModelObject, pModelChild, pChange);
}

void IO_OBJECT::Deleting (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Deleting (pModelObject, pModelChild, pChange);
}

void IO_OBJECT::Updating (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());

   pModelSelf->Updating (pModelObject, pModelChild);
}

void IO_OBJECT::Updated (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   RMAP::SVC_SOCKETIO::IO_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<RMAP::SVC_SOCKETIO::IO_OBJECT*> (pObject) : NULL;

   if (pChild == NULL)
   {
      pSourceObject->Map_Read (pModelObject);
   }

   pModelSelf->Updated (pModelObject, pModelChild);
}

void IO_OBJECT::Changing (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = (pObject != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ()) : NULL;

   pModelSelf->Changing (pModelObject, pModelChild, pChange);
}

void IO_OBJECT::Changed (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange)
{
   RMAP::CORE::MODEL_OBJECT* pModelSelf = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pModel ());
   RMAP::CORE::MODEL_OBJECT* pModelChild = (pChild != NULL) ? dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pChild->pModel ()) : NULL;
   RMAP::CORE::MODEL_OBJECT* pModelObject = dynamic_cast<RMAP::CORE::MODEL_OBJECT*> (pObject->pModel ());
   RMAP::SVC_SOCKETIO::IO_OBJECT* pSourceObject = (pObject != NULL) ? dynamic_cast<RMAP::SVC_SOCKETIO::IO_OBJECT*> (pObject) : NULL;

   if (pObject != NULL)
   {
      pSourceObject->Map_Read (pModelObject);  // We really need to do selective reading based on the event described in pChange. Otherwise, we could be wasting huge amounts of time!
   }

   pModelSelf->Changed (pModelObject, pModelChild, pChange);
}

bool IO_OBJECT::Attach ()
{
   bool bResult = false;
   CLIENT* pClientIO = dynamic_cast<CLIENT*> (pClient ());

   RMAP::CORE::MEM::SOURCE::Attach ();

   if (bIndependent ())
   {
      pClientIO->Object_Subscribe (wClass (), pObjectHead ()->Self.ObjectIx ());

      bResult = true;
   }

   return bResult;
}

bool IO_OBJECT::Detach ()
{
   bool bResult = false;
   CLIENT* pClientIO = dynamic_cast<CLIENT*> (pClient ());

   if (bIndependent ())
   {
      pClientIO->Object_Unsubscribe (wClass (), pObjectHead ()->Self.ObjectIx ());

      bResult = true;
   }

   RMAP::CORE::MEM::SOURCE::Detach ();

   return bResult;
}

/******************************************************************************************************************************/
