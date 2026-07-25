/*******************************************************************************************************************************
**                                                                                                                            **
**                                               RMAP_SVC_SB : Recover.cpp                                                    **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

static const CLIENT::ACTION SBA_SUBSCRIBE_RECOVERX
(
   SBA_SUBSCRIBE_RECOVER,
   "",
   "",
   false,
   false
);

class RECOVER::Impl
{
public:
   Impl (CLIENT* pClient, RECOVER* pRecover) :
      pClient (pClient)
   {
      pObjectHead = new RMAP::SVC_SB::OBJECTHEAD ();

      pClient->Recv_Register (&SBA_SUBSCRIBE_RECOVERX, pRecover);
   }

   ~Impl ()
   {
      pClient->Recv_Unregister (&SBA_SUBSCRIBE_RECOVERX);

      delete pObjectHead;
   }

   CLIENT*                 pClient;
   RMAP::SVC_SB::OBJECTHEAD*   pObjectHead;
};

/*******************************************************************************************************************************
**                                                   CLASS (RECOVER)                                                          **
*******************************************************************************************************************************/

RECOVER::RECOVER (CLIENT* pClient)
{
   m_pImpl = new RECOVER::Impl (pClient, this);
}

RECOVER::~RECOVER ()
{
   delete m_pImpl;
}

int RECOVER::Object_Recover (uint16_t wClass, BYTESTREAM* pByteStream, int wSize)
{
   int wRead = 0;
   int offset;

   offset = pByteStream->Offset ();

   m_pImpl->pObjectHead->twParentIx     = pByteStream->Read_TWORD ();
   m_pImpl->pObjectHead->wClass_Parent  = pByteStream->Read_WORD  ();
   m_pImpl->pObjectHead->twObjectIx     = pByteStream->Read_TWORD ();
   m_pImpl->pObjectHead->wClass_Object  = pByteStream->Read_WORD  ();
   m_pImpl->pObjectHead->twEventIz      = pByteStream->Read_TWORD ();
   m_pImpl->pObjectHead->wFlags         = pByteStream->Read_WORD  ();                               // this needs to be renamed to wFlags !!!!!!!!!!!!!!!!!!!!

   if (pByteStream->IsError () == false && pByteStream->Offset () - offset == SIZEOF__SBD_OBJECT_HEAD)
   {
      if (m_pImpl->pObjectHead->wClass_Object == wClass) // sanity check
      {
         m_pImpl->pClient->pMem ()->Object_Update (m_pImpl->pObjectHead, this, pByteStream);

         wRead = pByteStream->Offset () - offset;
      }
   }

   return wRead;
}

bool RECOVER::onRecv_Request (RMAP::CORE::CLIENT::IACTION* pIAction, int wSize, BYTESTREAM* pByteStream)
{
   int dwSize, wCount;
   int w, wSize_Ex;
   RMAP::CORE::MEM::SOURCE* pObject;
   RMAP::CORE::MEM::OBJECTBANK* pObjectBank;
   CLIENT::IACTION* pIActionSB = dynamic_cast <CLIENT::IACTION*> (pIAction);

   // SBA_SUBSCRIBE_RECOVER is the only handled action

   pIActionSB->SetResult (1);
   if (wSize == pByteStream->Remaining () && (dwSize = pByteStream->Inflate ()) >= 0)
   {
      // ByteStream is ready to read SBA_Subscribe_Recover_In

      if (SIZEOF__SBA_SUBSCRIBE_RECOVER_IN <= dwSize)
      {
         int SBA_SRI_wCount = pByteStream->Read_WORD ();
         int SBA_SRI_wSize = pByteStream->Read_WORD ();

         if (dwSize == SIZEOF__SBA_SUBSCRIBE_RECOVER_IN + SBA_SRI_wSize)
         {
            for (w = 0, wSize = 0; w < SBA_SRI_wCount && wSize < SBA_SRI_wSize; w++)
            {
               // ByteStream is ready to read SBA_Subscribe_Recover_Bank_In

               if (wSize + SIZEOF__SBA_SUBSCRIBE_RECOVER_BANK_IN <= SBA_SRI_wSize)
               {
                  uint64_t SBA_SRBI_twObjectIx   = pByteStream->Read_TWORD ();
                  uint16_t SBA_SRBI_wClass       = pByteStream->Read_WORD  ();
                                                   pByteStream->Read_Pad  (3);
                  uint8_t  SBA_SRBI_bFlags       = pByteStream->Read_BYTE  ();
                  uint16_t SBA_SRBI_wClass_Child = pByteStream->Read_WORD  ();
                  uint16_t SBA_SRBI_wSize        = pByteStream->Read_WORD  ();

                  if (wSize + SIZEOF__SBA_SUBSCRIBE_RECOVER_BANK_IN + SBA_SRBI_wSize <= SBA_SRI_wSize)
                  {
                     if ((pObjectBank = m_pImpl->pClient->pMem ()->ObjectBank (SBA_SRBI_wClass)) != NULL)
                     {
                        if ((pObject = pObjectBank->Get (NULL, SBA_SRBI_twObjectIx)) != NULL)
                        {
                           if ((SBA_SRBI_bFlags & SBA_SUBSCRIBE_RECOVER_BANK_FLAG_OBJECT_INITIAL) != 0)
                           {
                              // pObject.Recovering (SBA_SRBI_wClass);

                              m_pImpl->pClient->pMem ()->Object_Expire_Full (pObject);
                           }

                           if ((SBA_SRBI_bFlags & SBA_SUBSCRIBE_RECOVER_BANK_FLAG_BANK_INITIAL) != 0 && SBA_SRBI_wClass_Child != SBO_CLASS_NULL)
                           {
                              // pObject.Recovering (SBA_SRBI_wClass_Child);
                           }

                           wSize_Ex = 0;

                           if (SBA_SRBI_wClass_Child == SBO_CLASS_NULL)
                           {
                              // ByteStream is ready to read pObject

                              wSize_Ex += Object_Recover (SBA_SRBI_wClass, pByteStream, SBA_SRBI_wSize - wSize_Ex);
                           }
                           else
                           {
                              // ByteStream is ready to read SBA_Object_Recover_Child_Out

                              if (wSize_Ex + SIZEOF__SBA_SUBSCRIBE_RECOVER_CHILDREN <= SBA_SRBI_wSize)
                              {
                                 uint64_t SBA_SRC_twChildIx = pByteStream->Read_TWORD ();
                                 uint16_t SBA_SRC_wCount    = pByteStream->Read_WORD  ();

                                 wSize_Ex += SIZEOF__SBA_SUBSCRIBE_RECOVER_CHILDREN;

                                 for (wCount = 0; wCount < SBA_SRC_wCount; wCount++)
                                 {
                                    // ByteStream is ready to read pObject

                                    wSize_Ex += Object_Recover (SBA_SRBI_wClass_Child, pByteStream, SBA_SRBI_wSize - wSize_Ex);
                                 }
                              }
                           }

                           if (wSize_Ex == SBA_SRBI_wSize)
                              wSize += SIZEOF__SBA_SUBSCRIBE_RECOVER_BANK_IN + SBA_SRBI_wSize;
                           else wSize = SBA_SRI_wSize + 1; // this will signify an error below

                           if ((SBA_SRBI_bFlags & SBA_SUBSCRIBE_RECOVER_BANK_FLAG_BANK_FINAL) != 0 && SBA_SRBI_wClass_Child != SBO_CLASS_NULL)
                           {
                              // pObject.Recovered (SBA_SRBI_wClass_Child);
                           }

                           if ((SBA_SRBI_bFlags & SBA_SUBSCRIBE_RECOVER_BANK_FLAG_OBJECT_FINAL) != 0)
                           {
                              m_pImpl->pClient->pMem ()->Object_Purge_Full (pObject);

                              // pObject.Recovered (SBA_SRBI_wClass);
                           }
                        }
                        else w = SBA_SRI_wCount - 1 + 4; // this will signify an error below
                     }
                     else w = SBA_SRI_wCount - 1 + 3; // this will signify an error below
                  }
                  else w = SBA_SRI_wCount - 1 + 2; // this will signify an error below
               }
               else w = SBA_SRI_wCount - 1 + 1; // this will signify an error below
            }

            if (w == SBA_SRI_wCount && wSize == SBA_SRI_wSize)
               pIActionSB->SetResult (0);
         }
      }
   }

   if (pIActionSB->GetResult () != 0)
   {
      RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

      pCore->LoggerWrite (RMAP::CORE::LOGGER::kLOGLEVEL_Error, LibrarySVC_SB::sModuleName, "onRecv_Request RECOVER failed");
   }

   return false;
}

bool RECOVER::onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam)
{
   SB_OBJECT* pSourceSB = dynamic_cast<SB_OBJECT*> (pObject);
   BYTESTREAM* pByteStream = (BYTESTREAM*)pParam;

   pSourceSB->Map_Write (pByteStream, m_pImpl->pObjectHead->wFlags, bDiscard);

   return true;
}

bool RECOVER::onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam)
{
   return false;
}

/******************************************************************************************************************************/
