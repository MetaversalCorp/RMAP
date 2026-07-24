/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Notification.cpp                                             **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (INOTICE)                                                        **
*******************************************************************************************************************************/

INOTICE::INOTICE (NOTIFICATION* pNotification, std::string sNotification, void* pData) :
   pCreator (pNotification),
   sNotification (sNotification),
   pData (pData),
   pEmitter (NULL),
   bPropagate (bPropagate)
{

}

INOTICE::~INOTICE ()
{
}

/*******************************************************************************************************************************
**                                                     CLASS (NOTIFICATION::Impl)                                             **
*******************************************************************************************************************************/

class ICOMPARE : public COLLECTION<NOTIFICATION*, NOTIFICATION::LISTENER*>::ICOMPARE
{
   int Compare (NOTIFICATION* pThis, NOTIFICATION::LISTENER* pListener_B) 
   { 
      return (pThis == pListener_B->pThis); 
   }
};

class NOTIFICATION::Impl
{
public:
   Impl () :
      nReadyState (0)
   {
      m_pCompare = new ICOMPARE ();

      cpListener = new COLLECTION<NOTIFICATION*, NOTIFICATION::LISTENER*> (NULL, m_pCompare);
   }

   ~Impl ()
   {
      delete cpListener;
      delete m_pCompare;
   }

   int nReadyState;
   COLLECTION<NOTIFICATION*, NOTIFICATION::LISTENER*>* cpListener;

private:
   ICOMPARE* m_pCompare;
};

/*******************************************************************************************************************************
**                                                     CLASS (NOTIFICATION)                                                   **
*******************************************************************************************************************************/

NOTIFICATION::NOTIFICATION ()
{
   m_pImpl = new Impl ();
}

NOTIFICATION::~NOTIFICATION ()
{
//   assert (m_pImpl->cpListener.Length () == 0);
   delete m_pImpl;
}

int NOTIFICATION::ReadyState ()
{
   return m_pImpl->nReadyState;
}

int NOTIFICATION::ReadyState (int nReadyState)
{
   INOTICE* pNotice;

   if (m_pImpl->nReadyState != nReadyState)
   {
      m_pImpl->nReadyState = nReadyState;

      pNotice = new INOTICE (this, "onReadyState", &m_pImpl->nReadyState);

      Enum (pNotice);

      delete pNotice;
   }

   return m_pImpl->nReadyState;
}

void NOTIFICATION::Emit (std::string sMessage, void* pParam)
{
   INOTICE* pNotice = new INOTICE (this, sMessage, pParam);

   Enum (pNotice);

   delete pNotice;
};

int NOTIFICATION::Attach (NOTIFICATION* pNotice, bool bPropagate, bool bNotifyOnReady)
{
   int nResult = -1;
   NOTIFICATION::LISTENER* pListener;

   pListener = m_pImpl->cpListener->Get (pNotice, true);
   {
      if (pListener == NULL)
      {
         pListener = new LISTENER (this, pNotice, bPropagate, bNotifyOnReady);

         if (m_pImpl->cpListener->Add (pNotice, pListener))
         {
            //            setTimeout (m_pImpl->Init.bind (this, pThis), 0);
            Init (pNotice);

            nResult = m_pImpl->cpListener->Length ();
         }
         else delete pListener;
      }
      else
      {
         APP* pCore = APP::GetInstance ();

         pCore->LoggerWrite (LOGGER::kLOGLEVEL_Warning, LibraryRMAP::sModuleName, "Attach: Already attached to notification object instance : ");
      }
   }
   m_pImpl->cpListener->Release ();

   return nResult;
}

int NOTIFICATION::Detach (NOTIFICATION* pNotice)
{
   int nResult = -1;
   NOTIFICATION::LISTENER* pListener;

   if ((pListener = m_pImpl->cpListener->Remove (pNotice)) != NULL)
   {
      delete pListener;

      nResult = m_pImpl->cpListener->Length ();
   }
   else
   {
      APP* pCore = APP::GetInstance ();

      pCore->LoggerWrite (LOGGER::kLOGLEVEL_Warning, LibraryRMAP::sModuleName, "Detach: Not attached to notification object instance: ");
   }

   return nResult;
}

void NOTIFICATION::Notify (INOTICE* pNotice)
{
}

bool NOTIFICATION::IsReady ()
{
   return false;
}

void NOTIFICATION::Send (NOTIFICATION::LISTENER* pListener, INOTICE* pNotice)
{
   pNotice->pEmitter = this;
   pNotice->bPropagate = pListener->bPropagate;

   pListener->pThis->Notify (pNotice);

   if (pNotice->bPropagate)
      pListener->pThis->Enum (pNotice);
}

void NOTIFICATION::Enum (INOTICE* pNotice)
{
   NOTIFICATION::LISTENER* pListener;
   PCOLLECTION_ENUM pEnum;
   bool bSend;

   if (pEnum = m_pImpl->cpListener->Enum_Begin ())
   {
      while ((pListener = m_pImpl->cpListener->Enum_Next (pEnum)) != NULL)
      {
         if (pNotice->sNotification.compare ("onReadyState") == 0)
         {
            bSend = true;

            if (pListener->bInit == false)
               pListener->bInit = true;
         }
         else
         {
            bSend = (pListener->bNotifyOnReady == false || IsReady ());
         }

         if (bSend)
            Send (pListener, pNotice);

         m_pImpl->cpListener->Release ();
      }

      m_pImpl->cpListener->Enum_End (pEnum);
   }
}

void NOTIFICATION::Init (NOTIFICATION* pThis)
{
   NOTIFICATION::LISTENER* pListener;
   INOTICE* pNotice;

   if ((pListener = m_pImpl->cpListener->Get (pThis)) != NULL)
   {
      if (pListener->bInit == false)
      {
         pNotice = new INOTICE (this, "onReadyState", &m_pImpl->nReadyState);

         Send (pListener, pNotice);

         delete pNotice;

         pListener->bInit = true;
      }

      m_pImpl->cpListener->Release ();
   }
}

/*******************************************************************************************************************************
**                                                     CLASS (NOTIFICATION::LISTENER)                                         **
*******************************************************************************************************************************/

NOTIFICATION::LISTENER::LISTENER (NOTIFICATION* pSender, NOTIFICATION* pReceiver, bool bPropagate, bool bNotifyOnReady) :
   pThis (pReceiver),
   bPropagate (bPropagate),
   bNotifyOnReady (bNotifyOnReady),
   bInit (false)
{
}

NOTIFICATION::LISTENER::~LISTENER ()
{
}

/******************************************************************************************************************************/
