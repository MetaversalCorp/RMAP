/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Model_Object.cpp                                             **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (MODEL_OBJECT::Impl)                                             **
*******************************************************************************************************************************/

class MODEL_OBJECT::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }

   COLLECTION<std::string, MODEL_OBJECT*>* Child_Collection (std::string sID)
   {
      COLLECTION<std::string, MODEL_OBJECT*>* pResult;
      auto search = acpChild.find (sID);

      if (search == acpChild.end ())
      {
         pResult = new COLLECTION<std::string, MODEL_OBJECT*> (NULL, NULL);

         acpChild[sID] = pResult;
      }
      else pResult = search->second;

      return pResult;
   }

   std::map<std::string, COLLECTION<std::string, MODEL_OBJECT*>*> acpChild;
};

/*******************************************************************************************************************************
**                                                     CLASS (MODEL_OBJECT)                                                   **
*******************************************************************************************************************************/

MODEL_OBJECT::MODEL_OBJECT (MEM::MODEL::IREFERENCE* pReference, MEM::SOURCE* pSource) :
   MEM::MODEL (pReference, pSource)
{
   m_pImpl = new Impl ();
}

MODEL_OBJECT::~MODEL_OBJECT ()
{
   delete m_pImpl;
}

MODEL_OBJECT* MODEL_OBJECT::Child_Get (std::string sID, std::string sKey)
{
   MODEL_OBJECT* pChild = NULL;
   auto search = m_pImpl->acpChild.find (sID);

   if (search != m_pImpl->acpChild.end ())
   {
      pChild = search->second->Get (sKey);
   }

   return pChild;
}

void MODEL_OBJECT::Child_Release (std::string sID)
{
   MODEL_OBJECT* pChild = NULL;
   auto search = m_pImpl->acpChild.find (sID);

   if (search != m_pImpl->acpChild.end ())
   {
      search->second->Release ();
   }
}

MODEL_OBJECT* MODEL_OBJECT::Child_Enum (std::string sID, fnModelObjectEnum fnEnum, void* pvParam)
{
   MODEL_OBJECT* pChild = NULL;
   PCOLLECTION_ENUM pEnum;
   bool bResult = true;
   COLLECTION<std::string, MODEL_OBJECT*>* cpChild;
   auto search = m_pImpl->acpChild.find (sID);

   if (search != m_pImpl->acpChild.end ())
   {
      if ((pEnum = search->second->Enum_Begin ()) != NULL)
      {
         cpChild = search->second;

         while (bResult && (pChild = cpChild->Enum_Next (pEnum)) != NULL)
         {
            bResult = fnEnum (pChild, pvParam); // pEnum is always released, even upon early termination, because models do not need to be locked to be referenced (yet)
            cpChild->Release ();
         }

         cpChild->Enum_End (pEnum);
      }
   }

   return pChild;
}

bool MODEL_OBJECT::IsReady ()
{
   return (ReadyState () == eSTATE::RECOVERED);
}

void MODEL_OBJECT::Partial ()
{
   ReadyState (PARTIAL);
}

void MODEL_OBJECT::Full ()
{
   ReadyState (FULL);
}

void MODEL_OBJECT::Recovering ()
{
}

void MODEL_OBJECT::Recovered ()
{
   ReadyState (RECOVERED);
}

void MODEL_OBJECT::Inserted (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange)
{
   COLLECTION<std::string, MODEL_OBJECT*>* cpChild;
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   if (pChild != NULL)
   {
      if ((cpChild = m_pImpl->Child_Collection (pChild->sID ())) != NULL)
      {
         cpChild->Add (std::to_string (pChild->twObjectIx ()), pChildMO); // what if this fails
      }
   }

   np.pObject = pObjectMO;
   np.pChild  = pChildMO;
   np.pChange = pChange;

   Emit ("onInserted", &np);
}

void MODEL_OBJECT::Deleting (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange)
{
   COLLECTION<std::string, MODEL_OBJECT*>* cpChild;
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   np.pObject  = pObjectMO;
   np.pChild   = pChildMO;
   np.pChange  = pChange;

   Emit ("onDeleting", &np);

   if (pChild != NULL)
   {
      if ((cpChild = m_pImpl->Child_Collection (pChildMO->sID ())) != NULL)
      {
         cpChild->Remove (std::to_string (pChildMO->twObjectIx ()));
      }
   }
}

void MODEL_OBJECT::Updating (MEM::MODEL* pObject, MEM::MODEL* pChild)
{
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   np.pObject  = pObjectMO;
   np.pChild   = pChildMO;
   np.pChange  = NULL;

   Emit ("onUpdating", &np);
}

void MODEL_OBJECT::Updated (MEM::MODEL* pObject, MEM::MODEL* pChild)
{
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   np.pObject  = pObjectMO;
   np.pChild   = pChildMO;
   np.pChange  = NULL;

   Emit ("onUpdated", &np);
}

void MODEL_OBJECT::Changing (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange)
{
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   np.pObject  = pObjectMO;
   np.pChild   = pChildMO;
   np.pChange  = pChange;

   Emit ("onChanging", &np);
}

void MODEL_OBJECT::Changed (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange)
{
   MODEL_OBJECT* pChildMO = dynamic_cast<MODEL_OBJECT*> (pChild);
   MODEL_OBJECT* pObjectMO = dynamic_cast<MODEL_OBJECT*> (pObject);
   NOTIFYPARAM np;

   np.pObject  = pObjectMO;
   np.pChild   = pChildMO;
   np.pChange  = pChange;

   Emit ("onChanged", &np);
}

/******************************************************************************************************************************/
