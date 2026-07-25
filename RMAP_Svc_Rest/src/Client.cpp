/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVRest_cpp : Client.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <mutex>

using namespace RMAP::SVC_REST;

/*******************************************************************************************************************************
**                                                     CLASS (CLIENT::ACTION)                                                 **
*******************************************************************************************************************************/

class CLIENT::ACTION::Impl
{
public:
   Impl (std::string &sAction, std::string &sRequest, eCODEC kCodec) :
      sAction (sAction),
      sRequest (sRequest),
      kCodec (kCodec)
   {
   }

   ~Impl ()
   {
   }

public:
   std::string                   sAction;
   std::string                   sRequest;
   eCODEC                        kCodec;
};

CLIENT::ACTION::ACTION (std::string sAction, std::string sRequest, eCODEC kCodec)
{
   m_pImpl = new Impl (sAction, sRequest, kCodec);
}

CLIENT::ACTION::~ACTION ()
{
   delete m_pImpl;
}

std::string& CLIENT::ACTION::GetAction () const
{ 
   return m_pImpl->sAction;  
}

std::string& CLIENT::ACTION::GetRequest () const
{ 
   return m_pImpl->sRequest; 
}

CLIENT::ACTION::eCODEC CLIENT::ACTION::GetCodec ()   
{ 
   return m_pImpl->kCodec;   
}

/*******************************************************************************************************************************
**                                                     CLASS (IACTION)                                                        **
*******************************************************************************************************************************/

CLIENT::IACTION::IACTION (CLIENT* pClient, const ACTION* pAction) :
   RMAP::CORE::CLIENT::IACTION (pClient, pAction),
   m_pResponse (NULL),
   m_pParam (0),
   m_nType (0),
   m_bSuccess (false)
{
   m_pICodec = pClient->Codec_Create (this, pAction->GetRequest ());
}

CLIENT::IACTION::~IACTION ()
{
   CLIENT* pClientREST = dynamic_cast<CLIENT*> (m_pClient);

   pClientREST->Codec_Destroy (m_pICodec);
}

void CLIENT::IACTION::Response (bool bSuccess)
{
   m_bSuccess = bSuccess;

   if (m_pResponse != NULL)
   {
      m_pResponse->onResponse (this, m_nType, m_pParam);
   }
}

CLIENT::ICODEC* CLIENT::IACTION::GetCodec ()
{
   const ACTION* pActionREST = dynamic_cast<const ACTION*> (m_pAction);

   return m_pICodec;
}

bool CLIENT::IACTION::Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam)
{
   bool bResult = true;
   const ACTION* pActionREST             = dynamic_cast<const ACTION*> (m_pAction);
   CLIENT*       pClientREST             = dynamic_cast<CLIENT*> (m_pClient);

   m_pResponse = pResponse;
   m_pParam    = pParam;
   m_nType     = nType;

   m_pICodec->Encode (pClientREST->pLogin (), pActionREST->GetAction (), pClientREST->sEndPoint ());

   pClientREST->QueueAction (this);

   return bResult;
}

bool CLIENT::IACTION::IsSuccess ()
{
   return true;
}

ordered_json& CLIENT::IACTION::GetRequest ()
{
   return m_pICodec->GetRequest ();
}

/*******************************************************************************************************************************
**                                                     CLASS (IREFERENCE)                                                     **
*******************************************************************************************************************************/

CLIENT::IREFERENCE::IREFERENCE (uint64_t twClientIx) :
   RMAP::CORE::CLIENT::IREFERENCE ("Rest", twClientIx)
{
}

CLIENT::IREFERENCE::~IREFERENCE ()
{
}

RMAP::CORE::CLIENT* CLIENT::IREFERENCE::Create (RMAP::CORE::SERVICE* pService)
{
   return new CLIENT (this, pService);
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

typedef struct tagIACTIONLIST
{
   CLIENT::IACTION*                    pIAction;
   RMAP::CORE::SOURCE*                   pSource;
}
IACTIONDATA;

class CLIENT::Impl
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient)
   {
      pObjectHead = new REST_OBJECT::OBJECTHEAD ();
   }

   ~Impl ()
   {
      delete pControl;

      delete pObjectHead;
   }

   void Init ()
   {
      SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pClient->pService ());

      pControl = new CONTROL (pClient, pServiceSB->pNetSettings ());
   }

   void IAction_Add (CLIENT::IACTION* pIAction, RMAP::CORE::SOURCE* pSource)
   {
      IACTIONDATA Data;

      m_CS.lock ();
      {
         Data.pIAction = pIAction;
         Data.pSource  = pSource;

         m_apIActionData.push_back (Data);
      }
      m_CS.unlock ();
   }

   void IAction_Remove (CLIENT::IACTION* pIAction)
   {
      size_t i;
      m_CS.lock ();
      {
         for (i = 0; i < m_apIActionData.size () && m_apIActionData[i].pIAction != pIAction; i++);

         if (i < m_apIActionData.size ())
         {
            m_apIActionData.erase (m_apIActionData.begin () + i);
         }
      }
      m_CS.unlock ();
   }

   void IAction_AbortAll (RMAP::CORE::SOURCE* pSource)
   {
      size_t i;
      m_CS.lock ();
      {
         for (i = 0; i < m_apIActionData.size (); i++)
         {
            if (m_apIActionData[i].pSource == pSource)
            {
               m_apIActionData[i].pIAction->Abort ();
            }
         }
      }
      m_CS.unlock ();
   }

   CLIENT* pClient;
   CONTROL* pControl;

   REST_OBJECT::OBJECTHEAD* pObjectHead;

private:
   std::recursive_mutex          m_CS;
   std::vector<IACTIONDATA>      m_apIActionData;
};

/*******************************************************************************************************************************
**                                                   CLASS (CLIENT)                                                        **
*******************************************************************************************************************************/

CLIENT::CLIENT (IREFERENCE* pReference, RMAP::CORE::SERVICE* pService) :
   RMAP::CORE::CLIENT (pReference, pService)
{
   m_pImpl = new Impl (this);
   m_pImpl->Init ();
}

CLIENT::~CLIENT ()
{
   delete m_pImpl;
}

RMAP::CORE::CLIENT::IREFERENCE* CLIENT::Reference (uint64_t twClientIx)
{
   return new CLIENT::IREFERENCE (twClientIx);
}

// ===== Public Properties ==================================================================================================

bool CLIENT::bLoggedIn ()
{
   return m_pImpl->pControl->bLoggedIn ();
}

RMAP::CORE::SOURCE_SESSION::LOGIN* CLIENT::pLogin ()
{
   return m_pImpl->pControl->pLogin ();
}

void CLIENT::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   RMAP::CORE::SOURCE* pSource = Source (0); // MV_SERVICE_OBJECT_SESSION // this is pSession.pSource

   if (pSource)
   {
      REST_SESSION* pSourceSB = dynamic_cast<REST_SESSION*> (pSource);

      pSourceSB->Progress (pProgress);
   }
}

// ==========================================================================================================================

bool CLIENT::IsDisconnected ()
{
   return false;
}

bool CLIENT::IsConnected ()
{
   return true;
}

bool CLIENT::IsLoggedOut ()
{
   return (ReadyState () == eSTATE::LOGGEDOUT);
}

bool CLIENT::IsLoggedIn ()
{
   return (ReadyState () == eSTATE::LOGGEDIN);
}

std::string& CLIENT::sEndPoint ()
{
   return m_pImpl->pControl->sEndPoint ();
}

bool CLIENT::SafeKill ()
{
   // this function returns true if the client connection is completely shut down and it is safe to kill the client

   return m_pImpl->pControl->SafeKill ();
}

// ==========================================================================================================================

bool CLIENT::Login (void* pParams)
{ 
   return m_pImpl->pControl->Login (Source (0), pParams); // this is pSession.pSource  // We should define an interface for the source to 
}

bool CLIENT::Logout (void* pParams) 
{ 
   return m_pImpl->pControl->Logout (Source (0), pParams);  // this is pSession.pSource  // a call directly into it. This will work for now.
}

// ==========================================================================================================================

RMAP::CORE::CLIENT::IACTION* CLIENT::Request (const RMAP::CORE::CLIENT::ACTION* pAction)
{
   const ACTION* pActionREST = dynamic_cast<const ACTION*> (pAction);

   return new IACTION (this, pActionREST);
}

void CLIENT::IAction_Add (RMAP::SVC_REST::CLIENT::IACTION* pIAction, RMAP::CORE::SOURCE* pSource)
{
   m_pImpl->IAction_Add (pIAction, pSource);
}

void CLIENT::IAction_Remove (RMAP::SVC_REST::CLIENT::IACTION* pIAction)
{
   m_pImpl->IAction_Remove (pIAction);
}

void CLIENT::IAction_AbortAll (RMAP::CORE::SOURCE* pSource)
{
   m_pImpl->IAction_AbortAll (pSource);
}

CLIENT::ICODEC* CLIENT::Codec_Create (RMAP::SVC_REST::CLIENT::IACTION* pIAction, std::string sRequest)
{
   CLIENT::ICODEC* pICodec = NULL;
   RMAP::CORE::SOURCE* pSource = Source (0); // MV_SERVICE_OBJECT_SESSION // this is pSession.pSource

   if (pSource)
   {
      REST_SESSION* pSourceX = dynamic_cast<REST_SESSION*> (pSource);

      pICodec = pSourceX->Codec_Create (pIAction, sRequest);
   }

   return pICodec;
}

void CLIENT::Codec_Destroy (CLIENT::ICODEC* pICodec)
{
   delete pICodec;
}

// ==========================================================================================================================

uint32_t CLIENT::Object_Recover (void* pvData)
{
   REST_OBJECT::OBJECTHEAD* pObjectHead = (REST_OBJECT::OBJECTHEAD*)pvData;
   uint32_t dwResult = 0;

  m_pImpl->pObjectHead = pObjectHead;

   m_pImpl->pClient->m_pMem->Object_Update (m_pImpl->pObjectHead, this, pvData);

   return dwResult;
}

bool CLIENT::onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam)
{
   REST_OBJECT* pSourceREST = dynamic_cast<REST_OBJECT*> (pObject);

   pSourceREST->Map_Write (pParam, m_pImpl->pObjectHead->wFlags, bDiscard);

   return true;
}

bool CLIENT::onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam)
{
   return true;
}

void CLIENT::QueueAction (IACTION* pIAction)
{
   m_pImpl->pControl->QueueAction (pIAction);
}

/******************************************************************************************************************************/
