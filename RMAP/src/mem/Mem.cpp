/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Mem.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (CORE::Impl)                                                     **
*******************************************************************************************************************************/

class MEMSOURCE : public SOURCE
{
public:
   MEMSOURCE (REFERENCE* pReference, RMAP::CORE::CLIENT* pClient, OBJECTHEAD* pObjectHead) :
      SOURCE (pReference, pClient, pObjectHead)
   {
   }

   virtual ~MEMSOURCE ()
   {
   }

   void Partial () override {}
   void Full () override {}
   void Recovering () override {}
   void Recovered () override {}
   void Inserted (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) override {}
   void Deleting (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) override {}
   void Updating (SOURCE* pObject, SOURCE* pChild) override {}
   void Updated  (SOURCE* pObject, SOURCE* pChild) override {}
   void Changing (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) override {}
   void Changed  (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) override {}
};

class IObjectBankMem : public IOBJECTBANK
{
public:
   IObjectBankMem (int nType, MEM* pMem, SOURCE* pObject = NULL) :
      nType (nType),
      pMem (pMem),
      pObject (pObject)
   {
   }

   int onObjectBankItem (SOURCE* pChild, void* pParam) override
   {
      switch (nType)
      {
      case 0:
         if ((pChild->pObjectHead ()->wFlags & MEM::MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL) != 0)
            pChild->pObjectHead ()->wFlags |= MEM::MVO_OBJECT_HEAD_FLAG::EXPIRED_PARTIAL;
         break;

      case 1:
         if ((pChild->pObjectHead ()->wFlags & MEM::MVO_OBJECT_HEAD_FLAG::EXPIRED_PARTIAL) != 0)
            pMem->Object_Close_Partial (pObject, pChild);
         break;

      case 2:
         pMem->Object_Close_Partial (pObject, pChild);
         break;

      case 3:
         if ((pChild->pObjectHead ()->wFlags & MEM::MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
            pMem->Object_Expire_Full (pChild);
         break;

      case 4:
         if ((pChild->pObjectHead ()->wFlags & MEM::MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
            pMem->Object_Purge_Full (pChild);
         break;

      case 5:
         if ((pChild->pObjectHead ()->wFlags & MEM::MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
            pMem->Object_Delete_Full (pChild);
         break;
      }

      return 0;
   }

   bool onObjectBankChildItem (int wClass, void* pParam) override
   {
      switch (nType)
      {
      case 0:
         pMem->Object_Expire_Partial (pObject, wClass);
         break;

      case 1:
         pMem->Object_Purge_Partial (pObject, wClass);
         break;

      case 2:
         pMem->Object_Delete_Partial (pObject, wClass);
         break;
      }


      return true;
   }

   MEM*     pMem;
   SOURCE*  pObject;
   int      nType;
};

class MEM::Impl
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient)
   {
      std::map<std::string, const CLIENT::ACTION*> apAction;

      m_pReference  = new SOURCE::REFERENCE ("", "", 0, apAction, false);

      pParent_X = new MEMSOURCE (m_pReference, pClient, new OBJECTHEAD ()); // OBJECTHEAD Freed in Source

      pNamespace = pClient->pNamespace ();
   }

   ~Impl ()
   {
      delete pParent_X;
      delete m_pReference;
   }

   std::map<int, OBJECTBANK*>    aObjectBank;
   SOURCE*                       pParent_X;
   CLIENT*                       pClient;
   NAMESPACE*                    pNamespace;

private:
   SOURCE::REFERENCE*   m_pReference;
   OBJECTHEAD*          m_pObjectHead;
};

/*******************************************************************************************************************************
**                                                     CLASS (MEM)                                                            **
*******************************************************************************************************************************/

MEM::MEM (CLIENT* pClient)
{
   m_pImpl = new Impl (pClient);
}

MEM::~MEM ()
{
   Object_Delete_All ();

   for (auto const& x : m_pImpl->aObjectBank)
   {
      delete x.second;
   }

   delete m_pImpl;
}

// ===== Public Properties ==================================================================================================

RMAP::CORE::NAMESPACE* MEM::pNamespace () { return m_pImpl->pNamespace; }
std::string          MEM::sNamespace () { return m_pImpl->pClient->sNamespace (); }
RMAP::CORE::CLIENT*    MEM::pClient ()    { return m_pImpl->pClient; }

// ----- Object Bank --------------------------------------------------------------------------------------------------------

bool SourceClassCallback (RMAP::CORE::SOURCECLASS* pSourceClass, void* pParam)
{
   int* pwClass = (int*)pParam;

   return !(pSourceClass->pSource_Factory ()->bType () == SOURCE::FACTORY::OBJECT && pSourceClass->pSource_Factory ()->pReference ()->wClass == *pwClass);
}

OBJECTBANK* MEM::ObjectBank (int wClass)
{
   std::map<int, OBJECTBANK*>::iterator it = m_pImpl->aObjectBank.find (wClass);
   OBJECTBANK* pResult = NULL;

   if (it == m_pImpl->aObjectBank.end ())
   {
      SOURCECLASS* pSourceClass;

      if ((pSourceClass = m_pImpl->pNamespace->SourceClass_Enum (m_pImpl->pClient->pService ()->sID (), SourceClassCallback, &wClass)) != NULL)
      {
         RMAP::CORE::MEM::SOURCE::FACTORY* pSource_Factory = dynamic_cast <RMAP::CORE::MEM::SOURCE::FACTORY*> (pSourceClass->pSource_Factory ());
         RMAP::CORE::MEM::MODEL::FACTORY* pModel_Factory   = dynamic_cast <RMAP::CORE::MEM::MODEL::FACTORY*> (pSourceClass->pModel_Factory ());

         if (pSource_Factory->pReference->bIndependent)
            pResult = new RMAP::CORE::MEM::OBJECTBANK_IND (this, pSourceClass->pModel_Factory (), pSource_Factory);
         else pResult = new RMAP::CORE::MEM::OBJECTBANK_DEP (this, pSourceClass->pModel_Factory (), pSource_Factory);

         m_pImpl->aObjectBank[wClass] = pResult;

         m_pImpl->pNamespace->SourceClass_Release (m_pImpl->pClient->pService ()->sID ());
      }
   }
   else pResult = it->second;

   return pResult;
}

// ----- Model --------------------------------------------------------------------------------------------------------------

MODEL* MEM::Model_Open (std::string sID_Model, std::string sArgs)
{
   MODEL* pModel = NULL;
   SOURCECLASS* pSourceClass;
   OBJECTBANK* pObjectBank;

   if (pSourceClass = m_pImpl->pNamespace->SourceClass_Get (m_pImpl->pClient->pService ()->sID (), sID_Model))
   {
      if ((pObjectBank = ObjectBank (pSourceClass->pSource_Factory ()->pReference ()->wClass)) != NULL)
      {
         pModel = pObjectBank->Model_Open (sArgs, 0, 0);
      }

      m_pImpl->pNamespace->SourceClass_Release (m_pImpl->pClient->pService ()->sID ());
   }

   return pModel;
}

MODEL* MEM::Model_Close (MODEL* pModel)
{
   OBJECTBANK* pObjectBank;

   if (pModel && pModel->pSource ())
   {
      if ((pObjectBank = ObjectBank (pModel->pSource ()->wClass ())) != NULL)
      {
         pModel = pObjectBank->Model_Close (pModel);
      }
   }

   return pModel;
}

SOURCE* MEM::Parent_Get (OBJECTBANK* pObjectBank_Parent, uint16_t wClass_Parent, uint64_t twParentIx)
{
   SOURCE* pParent;

   if (pObjectBank_Parent == NULL || (pParent = pObjectBank_Parent->Get (NULL, twParentIx)) == NULL)
   {
      pParent = m_pImpl->pParent_X;

      pParent->pObjectHead ()->Self.qwComposed = OBJECTIX_COMPOSE (wClass_Parent, twParentIx);
   }

   return pParent;
}

bool MEM::Object_Update (OBJECTHEAD* pObjectHead, IMEM* pICB, void* pParam)
{
   bool bResult = false;
   OBJECTBANK* pObjectBank_Parent;
   SOURCE*     pParent;
   OBJECTBANK* pObjectBank_Object;
   SOURCE*     pObject;
   bool        bInsert, bOpen, bChange, bDiscard;
   uint16_t    wFlags;

   pObjectHead->wFlags &= MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK;

   if (pObjectHead->wFlags == MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL || pObjectHead->wFlags == MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL)
   {
      pObjectBank_Parent = ObjectBank (pObjectHead->Parent.Class ());                                                  // this can fail ...

      if ((pParent = Parent_Get (pObjectBank_Parent, pObjectHead->Parent.Class (), pObjectHead->Parent.ObjectIx ())) != NULL)   // but this never fails
      {
         if ((pObjectBank_Object = ObjectBank (pObjectHead->Self.Class ())) != NULL)
         {
            if ((pObject = pObjectBank_Object->Get (pParent, pObjectHead->Self.ObjectIx ())) == NULL || (pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK) == 0)
            {
               if (pObject != NULL)
               {
                  // If pObject != NULL &&  (pObject.bObjectHead.wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK) == 0, that means that an 
                  // external application opened the model precedent to data being sent from the server. We need to treat this as if 
                  // this is the first time the model is being opened -- it's new to us.

                  // Also, the parent values can't be trusted, so we'll update them now...

                  pObject->pObjectHead ()->Parent.qwComposed = pObjectHead->Parent.qwComposed;
               }

               bInsert = true;
               bOpen = ((pObjectHead->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0);
               bChange = false;
               bDiscard = false;

               if (pObjectBank_Parent)
                  pObjectBank_Parent->Child_Set (pObjectHead->Self.Class ());

               pObject = pObjectBank_Object->Object_Open (pObjectHead->Parent.Class (), pObjectHead->Parent.ObjectIx (), pObjectHead->Self.Class (), pObjectHead->Self.ObjectIx ());
            }
            else
            {
               bInsert = false;
               bOpen = ((pObjectHead->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0 && (pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) == 0); // detect if a full is being updated over a partial
               bChange = ((pObjectHead->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0 || (pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) == 0); // detect if a partial is being updated over a full
               bDiscard = (bChange == false);
            }

            if (pObject != NULL)
            {
               if (bChange != false)
               {
                  pParent->Updating (pParent, pObject);
                  pObject->Updating (pObject, NULL);
               }

               wFlags = pObject->pObjectHead ()->wFlags;

               pObject->pObjectHead ()->wFlags |= pObjectHead->wFlags;       // set   the subscribe bit
               pObject->pObjectHead ()->wFlags &= ~(pObjectHead->wFlags << 2); // clear the expire    bit

               bResult = pICB->onUpdate (pObject, bDiscard, pParam);

               if (bDiscard == false)
                  pObject->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::CLIENT_RECOVERED;

               if (bOpen != false)
               {
                  // See notes below..
                  // this.Object_Open (pObject); /// failure ???
               }

               if (bInsert != false)
               {
                  pObject->Inserted (pObject, NULL,    NULL);
                  pParent->Inserted (pParent, pObject, NULL);
               }
               else if (bChange == false)
               {
                  // a partial child has been received on a full object that already exists

                  pParent->Inserted (pParent, pObject, NULL);
               }

               if (bChange != false)
               {
                  pObject->Updated (pObject, NULL);
                  pParent->Updated (pParent, pObject);
               }

               // Note that we have only (potentially) added bits...

               if (((wFlags ^ pObject->pObjectHead ()->wFlags) & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
                  pObject->Full ();
               else if (((wFlags ^ pObject->pObjectHead ()->wFlags) & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL) != 0 && (wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) == 0)
                  pObject->Partial ();
            }
         }
      }
   }

   return bResult;
}

bool MEM::Object_Change (uint16_t wClass_Object, uint64_t twObjectIx, uint16_t wClass_Child, uint64_t twChildIx, uint16_t wFlags, CHANGE* pChange, IMEM* pICB, void* pParam)
{
   bool bResult = false;
   OBJECTBANK* pObjectBank_Parent;
   OBJECTBANK* pObjectBank_Object;
   OBJECTBANK* pObjectBank_Child = NULL;
   SOURCE* pParent;
   SOURCE* pObject;
   SOURCE* pChild;

   bool bOpen = ((wFlags & SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG::OPEN) != 0);
   bool bClose = ((wFlags & SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG::CLOSE) != 0);

   bool bPartial = ((wFlags & SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG::PARTIAL) != 0);

   bool bChange = (bOpen == false && bClose == false);

   if ((pObjectBank_Object = ObjectBank (wClass_Object)) != NULL)
   {
      if ((pObject = pObjectBank_Object->Get (NULL, twObjectIx)) != NULL)
      {
         if (((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL) != 0 && (pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::EXPIRED_PARTIAL) == 0 && bPartial != false)
            || ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0 && (pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::EXPIRED_FULL) == 0))
         {
            pObjectBank_Parent = ObjectBank (pObject->pObjectHead ()->Parent.Class ());    // this can fail ...

            if ((pParent = Parent_Get (pObjectBank_Parent, pObject->pObjectHead ()->Parent.Class (), pObject->pObjectHead ()->Parent.ObjectIx ())) != NULL)   // but this never fails
            {
               if (wClass_Child != BANK_NULL && bOpen == false)
               {
                  if ((pObjectBank_Child = ObjectBank (wClass_Child)) != NULL)
                  {
                     pChild = pObjectBank_Child->Get (pObject, twChildIx);
                  }
                  else pChild = NULL;
               }
               else pChild = NULL;

               if (bPartial != false)
                  pParent->Changing (NULL, pObject, pChange); // only if the partial of the object has changed
               pObject->Changing (pObject, pChild, pChange);
               if (bChange != false && pChild != NULL)
                  pChild->Changing (pChild, NULL, pChange);

               if (wClass_Child != BANK_NULL && bClose != false && pChild != NULL)
               {
                  pObject->Deleting (pObject, pChild, pChange);
                  pChild->Deleting (pChild,   NULL,   pChange);
               }

               if (wClass_Child != BANK_NULL && bOpen != false)
               {
                  if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0) // how could this not be true ??
                  {
                     pObjectBank_Child = ObjectBank (wClass_Child);

                     // theoretically, if this were the first time we've encountered wClass_Child, pObjectBank can be NULL if there is no registered source/model

                     pObjectBank_Object->Child_Set (wClass_Child);

                     if ((pChild = pObjectBank_Child->Get (pObject, twChildIx)) == NULL)
                     {
                        pChild = pObjectBank_Child->Object_Open (wClass_Object, twObjectIx, wClass_Child, twChildIx);
                     }
                     else if (pChild->pObjectHead ()->Parent.Class () != 0)
                        bOpen = false;

                     if (pChild != NULL)
                     {
                        // It is possible that external application opened the model precedent to data being sent from the server.
                        // Therefore, the parent values can't be trusted, so we'll update them now...

                        pChild->pObjectHead ()->Parent.qwComposed = OBJECTIX_COMPOSE (wClass_Object, twObjectIx);

                        pChild->pObjectHead ()->wFlags |= MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL;

                        // See notes below..
                        // this.Object_Open (pChild);
                     }
                  }
               }

               bResult = pICB->onChange (pParent, pObject, pChild, pParam);

               if (wClass_Child != BANK_NULL && bClose != false)
               {
                  if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0) // how could this not be true ??
                  {
                     if (pChild != NULL)
                     {
                        pChild->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL;
                        pChild->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::EXPIRED_PARTIAL;

                        // Clearing a partial flag will not incur a subscription state change

                        // See notes below..
                        // this.Object_Close (pChild);

                        if ((pChild->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK) == 0)
                        {
                           pChild = pObjectBank_Child->Object_Close (pChild);
                        }
                        else
                        {
                           // See notes in Update regarding opening of an existing model. We need to restore to the pre-updated state.

                           pChild->pObjectHead ()->Parent.qwComposed = OBJECTIX_COMPOSE (0, 0);
                        }
                     }
                  }
               }

               if (wClass_Child != BANK_NULL && bOpen != false && pChild != NULL)
               {
                  pChild->Inserted  (pChild,  NULL,   pChange);
                  pObject->Inserted (pObject, pChild, pChange);

                  if ((pChild->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) == 0)
                     pChild->Partial ();
               }

               if (bChange != false && pChild != NULL)
                  pChild->Changed (pChild, NULL, pChange);                             //   dependent child was modified
               pObject->Changed (pObject, pChild, pChange);                            // independent object and possibly child was modified
               if (bPartial != false)
                  pParent->Changed (NULL, pObject, pChange);                           // independent child was modified
            }
         }
      }
   }

   return bResult;
}

bool MEM::Object_Close_Partial (SOURCE* pObject, SOURCE* pChild)
{
   bool bResult = false;

   if ((pChild->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL) != 0)
   {
      if ((pChild->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) == 0)
      {
         pObject->Deleting (pObject, pChild, NULL);
         pChild->Deleting  (pChild,  NULL,   NULL);
      }

      pChild->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL;
      pChild->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::EXPIRED_PARTIAL;

      // Clearing a partial flag will not incur a subscription state change

      if ((pChild->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK) == 0)
      {
         m_pImpl->aObjectBank[pChild->pObjectHead ()->Self.Class ()]->Object_Close (pChild);
      }

      bResult = true;
   }

   return bResult;
}

bool MEM::Object_Close_Full (SOURCE* pObject)
{
   bool bResult = false;

   if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
   {
      OBJECTBANK* pObjectBank_Parent = ObjectBank (pObject->pObjectHead ()->Parent.Class ());    // this can fail ...
      SOURCE* pParent = Parent_Get (pObjectBank_Parent, pObject->pObjectHead ()->Parent.Class (), pObject->pObjectHead ()->Parent.ObjectIx ());   // but this never fails

      if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_PARTIAL) == 0)
      {
         pParent->Deleting (pParent, pObject, NULL);
         pObject->Deleting (pObject, NULL,    NULL);
      }

      // See notes above..
      // this.Object_Close (pObject); /// failure ???

      pObject->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL;
      pObject->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::EXPIRED_FULL;

      pObject->pObjectHead ()->wFlags &= ~MVO_OBJECT_HEAD_FLAG::CLIENT_RECOVERED;

      pObject->Partial ();

      if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_MASK) == 0)
      {
         m_pImpl->aObjectBank[pObject->pObjectHead ()->Self.Class ()]->Object_Close (pObject);
      }

      bResult = true;
   }

   return bResult;
}

bool MEM::Object_Expire_Partial (SOURCE* pObject, uint16_t wClass)
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (0, this, pObject);

   m_pImpl->aObjectBank[wClass]->Enum (pObject, pIOBMem, NULL);

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Purge_Partial (SOURCE* pObject, uint16_t wClass)
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (1, this, pObject);

   m_pImpl->aObjectBank[wClass]->Enum (pObject, pIOBMem, NULL);

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Delete_Partial (SOURCE* pObject, uint16_t wClass)
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (2, this, pObject);

   m_pImpl->aObjectBank[wClass]->Enum (pObject, pIOBMem, this);

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Expire_Full (SOURCE* pObject)
{
   bool bResult = false;
   IObjectBankMem* pIOBMem = new IObjectBankMem (0, this, pObject);

   if (pObject != NULL)
   {
      if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
      {
         bResult = true;

         pObject->Recovering ();

         m_pImpl->aObjectBank[pObject->pObjectHead ()->Self.Class ()]->Child_Enum (pIOBMem, this);

         pObject->pObjectHead ()->wFlags |= MVO_OBJECT_HEAD_FLAG::EXPIRED_FULL;
      }
   }

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Purge_Full (SOURCE* pObject)
{
   bool bResult = false;
   IObjectBankMem* pIOBMem = new IObjectBankMem (1, this, pObject);

   if (pObject != NULL)
   {
      if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
      {
         bResult = true;

         pObject->pObjectHead ()->wFlags |= MVO_OBJECT_HEAD_FLAG::CLIENT_RECOVERED;

         m_pImpl->aObjectBank[pObject->pObjectHead ()->Self.Class ()]->Child_Enum (pIOBMem, this);

         if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::EXPIRED_FULL) != 0)
            bResult &= Object_Close_Full (pObject);

         pObject->Recovered ();
      }
   }

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Delete_Full (SOURCE* pObject)
{
   bool bResult = false;
   IObjectBankMem* pIOBMem = new IObjectBankMem (2, this, pObject);

   if (pObject != NULL)
   {
      if ((pObject->pObjectHead ()->wFlags & MVO_OBJECT_HEAD_FLAG::SUBSCRIBE_FULL) != 0)
      {
         bResult = true;

         m_pImpl->aObjectBank[pObject->pObjectHead ()->Self.Class ()]->Child_Enum (pIOBMem, this);

         bResult &= Object_Close_Full (pObject);
      }
   }

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Expire_All ()
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (3, this);

   for (auto const& x : m_pImpl->aObjectBank)
   {
      if (m_pImpl->aObjectBank[x.first]->bIndependent ())
      {
         x.second->Enum (NULL, pIOBMem, this);
      }
   }

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Purge_All ()
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (4, this);

   for (auto const& x : m_pImpl->aObjectBank)
   {
      if (m_pImpl->aObjectBank[x.first]->bIndependent ())
      {
         x.second->Enum (NULL, pIOBMem, this);
      }
   }

   delete pIOBMem;

   return bResult;
}

bool MEM::Object_Delete_All ()
{
   bool bResult = true;
   IObjectBankMem* pIOBMem = new IObjectBankMem (5, this);

   for (auto const& x : m_pImpl->aObjectBank)
   {
      if (m_pImpl->aObjectBank[x.first]->bIndependent ())
      {
         x.second->Enum (NULL, pIOBMem, this);
      }
   }

   delete pIOBMem;

   return bResult;
}
