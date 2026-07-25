/*******************************************************************************************************************************
**                                                                                                                            **
**                                               RMAP_SVC_SB : Refresh.cpp                                                    **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

typedef struct
{
   uint16_t                            wSize;

   uint8_t                             SBA_SREE_bFlags;                
   uint16_t                            SBA_SREE_wClass;        
   uint64_t                            SBA_SREE_twChildIx;             
   uint16_t                            SBA_SREE_wClass_Child;  
   TIME                                tmBase;
}
REFRESHDATA;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

static const CLIENT::ACTION SBA_SUBSCRIBE_REFRESHX
(
   SBA_SUBSCRIBE_REFRESH,
   "",
   "",
   false,
   false
);

class REFRESH::Impl
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient)
   {
      pSBA_SRE        = new SBA_SRE;
   }

   ~Impl ()
   {
      delete pSBA_SRE;
   }

   CLIENT*  pClient;
   SBA_SRE* pSBA_SRE;
};

/*******************************************************************************************************************************
**                                                   CLASS (REFRESH)                                                          **
*******************************************************************************************************************************/

REFRESH::REFRESH (CLIENT* pClient)
{
   m_pImpl = new REFRESH::Impl (pClient);

   pClient->Recv_Register (&SBA_SUBSCRIBE_REFRESHX, this);
}

REFRESH::~REFRESH ()
{
   m_pImpl->pClient->Recv_Unregister (&SBA_SUBSCRIBE_REFRESHX);

   delete m_pImpl;
}

int REFRESH::Event_Refresh_Object (RMAP::CORE::SOURCE* pObject, BYTESTREAM* pByteStream)
{
   SB_OBJECT* pSourceSB = dynamic_cast<SB_OBJECT*> (pObject);
   int wCount, wSize, wOffset, wLength, w;

   wCount = pByteStream->Read_WORD ();
   wSize = pByteStream->Read_WORD ();

   for (w = 0; w < wCount; w++)
   {
      wOffset = pByteStream->Read_WORD ();
      wLength = pByteStream->Read_WORD ();

      pByteStream->XCopy (pSourceSB->GetData (), wOffset, wLength);
   }

   return wSize + 4;
}

int REFRESH::Event_Refresh (TIME tmBase, BYTESTREAM* pByteStream)
{
   bool bResult;
   REFRESHDATA rd;

   rd.tmBase = tmBase;
   rd.wSize  = 0;

   // ByteStream is ready to read SBA_Subscribe_Refresh_Event_Ex data

   rd.SBA_SREE_bFlags        = pByteStream->Read_BYTE  ();
                               pByteStream->Read_Pad  (5);
   rd.SBA_SREE_wClass        = pByteStream->Read_WORD  ();
   rd.SBA_SREE_twChildIx     = pByteStream->Read_TWORD ();
   rd.SBA_SREE_wClass_Child  = pByteStream->Read_WORD  ();

   m_pByteStream = pByteStream;
   bResult = m_pImpl->pClient->pMem ()->Object_Change (rd.SBA_SREE_wClass, m_pImpl->pSBA_SRE->twObjectIx, rd.SBA_SREE_wClass_Child, rd.SBA_SREE_twChildIx, rd.SBA_SREE_bFlags, m_pImpl->pSBA_SRE, this, &rd);

   if (bResult == false)
   {
      pByteStream->Seek (m_pImpl->pSBA_SRE->wSize - SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT_EX);

      rd.wSize = m_pImpl->pSBA_SRE->wSize;
   }

   return rd.wSize;
}

bool REFRESH::onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam)
{
   return false;
}

bool REFRESH::onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam)
{
   int wSeek;
   REFRESHDATA* pRD = (REFRESHDATA*)pParam;
   SB_OBJECT* pObjectSB = dynamic_cast<SB_OBJECT*> (pObject);
   SB_OBJECT* pChildSB  = dynamic_cast<SB_OBJECT*> (pChild);

   m_pImpl->pClient->tmCurrent = pRD->tmBase + m_pImpl->pSBA_SRE->txStamp; /// TimeFromTimex (tmBase, m_pImpl->pSBA_SRE->txStamp);

   // if ( m_pImpl->pSBA_SRE->twEventIz == pObject.pObjectHead.twEventIz
   // ||  (m_pImpl->pSBA_SRE->twEventIz >= pObject.pObjectHead.twEventIz  &&  ((pObject.pObjectHead.bFlags & OBJECTHEAD_FLAG_SUBSCRIBE_FULL) == 0  ||  (pObject.pObjectHead.bFlags & OBJECTHEAD_FLAG_EXPIRED_FULL) != 0)))
   {
      pObjectSB->twEventIz (m_pImpl->pSBA_SRE->twEventIz + 1);

      // for newly opened child objects, we need to set the stream (presuming we still even need the stream)

      if (pRD->SBA_SREE_wClass_Child != SBO_CLASS_NULL && (pRD->SBA_SREE_bFlags & SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_RESET) != 0 && pChild != NULL)
      {
         pChildSB->twEventIz (0);

         pChildSB->ResetData ();
      }

      if (true)
         Event_Refresh_Object (pObject, m_pByteStream);

      if (pRD->SBA_SREE_wClass_Child != SBO_CLASS_NULL)
      {
         if (pChild != NULL)
            Event_Refresh_Object (pChild, m_pByteStream);
         else
         {
            m_pByteStream->Read_WORD ();
            wSeek = m_pByteStream->Read_WORD ();

            m_pByteStream->Seek (wSeek);
         }
      }

      if ((pRD->SBA_SREE_bFlags & SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_ADDENDUM) != 0)
      {
         /// Rather than copy binary data into a buffer and then have the app call the addendum function, 
         /// why not call the addendum function here and pass the resulting object in the notification.

         wSeek = m_pByteStream->Read_WORD ();

         m_pImpl->pSBA_SRE->pData.resize (wSeek);

         m_pByteStream->Copy (m_pImpl->pSBA_SRE->pData, 0, wSeek);
      }

      pRD->wSize = m_pImpl->pSBA_SRE->wSize; /// we need to keep a running total
   }

   return true;
}

bool REFRESH::onRecv_Request (RMAP::CORE::CLIENT::IACTION* pIAction, int wSize, BYTESTREAM* pByteStream)
{
   int wSize_Ex, w;
   int dwSize;
   SERVICE* pServiceSB = dynamic_cast<SERVICE*> (m_pImpl->pClient->pService ());
   CLIENT::IACTION* pIActionSB = dynamic_cast <CLIENT::IACTION*> (pIAction);

   // SBA_SUBSCRIBE_REFRESH is the only handled action

   pIActionSB->SetResult (1);
   if (wSize == pByteStream->Remaining () && (dwSize = pByteStream->Inflate ()) >= 0)
   {
      // ByteStream is ready to read SBA_Subscribe_Refresh_In

      if (dwSize >= SIZEOF__SBA_SUBSCRIBE_REFRESH_IN)
      {
         int      SBA_SRI_wCount     = pByteStream->Read_WORD  ();
         int      SBA_SRI_wSize      = pByteStream->Read_WORD  ();
         int      SBA_SRI_dwResult   = pByteStream->Read_DWORD ();
         TIME     SBA_SRI_tmBase     = pByteStream->Read_TIME  ();
         uint64_t SBA_SRI_evNext     = pByteStream->Read_EVENT ();

         if (dwSize == SIZEOF__SBA_SUBSCRIBE_REFRESH_IN + SBA_SRI_wSize)
         {
            pServiceSB->Time_Sync (SBA_SRI_tmBase);

            for (w = 0, wSize = 0; w < SBA_SRI_wCount && wSize < SBA_SRI_wSize; w++)
            {
               // ByteStream is ready to read SBA_Subscribe_Refresh_Event data

               if (wSize + SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT + SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT_EX <= SBA_SRI_wSize)
               {
                  m_pImpl->pSBA_SRE->twObjectIx   = pByteStream->Read_TWORD  ();
                  m_pImpl->pSBA_SRE->wClass       = pByteStream->Read_WORD   ();
                  m_pImpl->pSBA_SRE->twEventIz    = pByteStream->Read_TWORD8 ();
                  m_pImpl->pSBA_SRE->wEventTypeIx = pByteStream->Read_WORD   ();
                  m_pImpl->pSBA_SRE->wSize        = pByteStream->Read_WORD   ();
                  m_pImpl->pSBA_SRE->txStamp      = pByteStream->Read_TIMEX  ();

                  // This is a vector container, so we don't allocate or free.
//                  m_pImpl->pSBA_SRE->pData            = NULL;

                  if (wSize + SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT + m_pImpl->pSBA_SRE->wSize <= SBA_SRI_wSize)
                  {
                     /// What if this fails (i.e. dwResult != SBA_RESULT_SUCCESS) ...
                     wSize_Ex = Event_Refresh (SBA_SRI_tmBase, pByteStream);

                     if (wSize_Ex == m_pImpl->pSBA_SRE->wSize)
                        wSize += SIZEOF__SBA_SUBSCRIBE_REFRESH_EVENT + m_pImpl->pSBA_SRE->wSize;
                     else wSize = SBA_SRI_wSize + 1; // this will signify an error below
                  }
                  else w = SBA_SRI_wCount; // this will signify an error below
               }
               else w = SBA_SRI_wCount; // this will signify an error below
            }

            if (w == SBA_SRI_wCount && wSize == SBA_SRI_wSize)
               pIActionSB->SetResult (0);
         }
      }
   }

   if (pIActionSB->GetResult () != 0)
   {
      RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

      pCore->LoggerWrite (RMAP::CORE::LOGGER::kLOGLEVEL_Error, LibrarySVC_SB::sModuleName, "onRecv_Request REFRESH failed");
   }

   return false;
}

/******************************************************************************************************************************/
