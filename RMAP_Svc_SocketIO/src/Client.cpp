/*******************************************************************************************************************************
**                                                                                                                            **
**                                        RMAP_Svc_SocketIO : Client.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class CLIENT::IACTION::Impl
{
public:
   Impl (const ACTION* pAction) :
      m_pResponse (NULL),
      m_nType (0),
      m_pParam (0),
      m_pAction (pAction)
   {
      m_jIn = pAction->GetRequest ();
   }

   ~Impl ()
   {
   }

   const ACTION*        m_pAction;

   ordered_json         m_jIn;
   ordered_json         m_jOut;

   int                  m_nType;
   RMAP::CORE::IRESPONSE*     m_pResponse;

   intptr_t             m_pParam;
   bool                 m_bSuccess;
};

/*******************************************************************************************************************************
**                                                     CLASS (ACTION)                                                         **
*******************************************************************************************************************************/

class CLIENT::ACTION::Impl
{
public:
   Impl (std::string& sAction, std::string& sRequest, fnActionConvert fnConvert) :
      sAction (sAction),
      fnConvert (fnConvert)
   {
      try
      {
         jAction = ordered_json::parse (sRequest);
      }
      catch (const ordered_json::parse_error& e)
      {
         (void)e;
      }
   }

   ~Impl ()
   {
   }

public:
   std::string                   sAction;
   ordered_json                  jAction;
   fnActionConvert               fnConvert;
};

CLIENT::ACTION::ACTION (std::string sAction, std::string sRequest, fnActionConvert fnConvert)
{
   m_pImpl = new Impl (sAction, sRequest, fnConvert);
}

CLIENT::ACTION::~ACTION ()
{
}

std::string& CLIENT::ACTION::GetAction () const
{
   return m_pImpl->sAction;
}

ordered_json& CLIENT::ACTION::GetRequest () const
{
   return m_pImpl->jAction;
}

fnActionConvert CLIENT::ACTION::GetConvert () const
{
   return m_pImpl->fnConvert;
}

/*******************************************************************************************************************************
**                                                     CLASS (ACTION)                                                      **
*******************************************************************************************************************************/

CLIENT::IACTION::IACTION (CLIENT* pClient, const ACTION* pAction) :
   RMAP::CORE::CLIENT::IACTION (pClient, pAction)
{
   m_pImpl = new Impl (pAction);
}

CLIENT::IACTION::~IACTION ()
{
   delete m_pImpl;
}

bool CLIENT::IACTION::Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam)
{
   bool bResult = false;
   CLIENT* pClientIO = dynamic_cast<CLIENT*> (m_pClient);

   if (pClientIO->ReadyState () > CLIENT::eSTATE::SOCKETDISCONNECTED)
   {
      m_pImpl->m_pResponse = pResponse;
      m_pImpl->m_nType     = nType;
      m_pImpl->m_pParam    = pParam;

      bResult = pClientIO->pNet ()->Send_Request (this);
   }

   return bResult;
}

std::string CLIENT::IACTION::GetAction ()
{
   const CLIENT::ACTION* pActionIO = dynamic_cast<const CLIENT::ACTION*> (m_pAction);

   return pActionIO->GetAction ();
}

ordered_json& CLIENT::IACTION::GetRequest ()
{
   return m_pImpl->m_jIn;
}

std::string CLIENT::IACTION::GetRequestEx ()
{
   ordered_json jRequest_Out;
   fnActionConvert fnConvert;

   if ((fnConvert = m_pImpl->m_pAction->GetConvert ()) != NULL)
   {
      fnConvert (jRequest_Out, m_pImpl->m_jIn);
   }
   else jRequest_Out = m_pImpl->m_jIn;

   return jRequest_Out.dump ();
}

ordered_json& CLIENT::IACTION::GetResponse ()
{
   return m_pImpl->m_jOut;
}

void CLIENT::IACTION::SetResponse (ordered_json &jResponse)
{
   m_pImpl->m_jOut = jResponse;
}

void CLIENT::IACTION::Response ()
{
   if (m_pImpl->m_pResponse != NULL)
   {
      m_pImpl->m_pResponse->onResponse (this, m_pImpl->m_nType, m_pImpl->m_pParam);
   }
}

bool CLIENT::IACTION::IsSuccess ()
{
   return (m_pImpl->m_jOut["nResult"] == 0);
}

/*******************************************************************************************************************************
**                                                     CLASS (IREFERENCE)                                                     **
*******************************************************************************************************************************/

CLIENT::IREFERENCE::IREFERENCE (uint64_t twClientIx) :
   RMAP::CORE::CLIENT::IREFERENCE ("Socket.IO", twClientIx)
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

class CLIENT::Impl
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient),
      pNet (NULL),
      pControl (NULL)
   {
      pObjectHead = new IO_OBJECT::OBJECTHEAD ();
   }

   ~Impl ()
   {
      delete pNet;

      delete pControl;
      delete pSubscription;
      delete pRefresh;
      delete pRecover;

      delete pObjectHead;
   }

   void Init ()
   {
      SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pClient->pService ());

      pRecover       = new RECOVER (pClient);
      pRefresh       = new REFRESH (pClient);
      pSubscription  = new SUBSCRIPTION (pClient);
      pControl       = new CONTROL (pClient, pServiceSB->pNetSettings (), pSubscription);
      pNet           = new NET (pClient, pClient);
   }

   bool SocketDisconnected (bool bVoluntary)
   {
      return pControl->SocketDisconnected (bVoluntary);
   }

public:
   CLIENT*                             pClient;
   CONTROL*                            pControl;
   RECOVER*                            pRecover;
   REFRESH*                            pRefresh;
   SUBSCRIPTION*                       pSubscription;
   NET*                                pNet;

   std::map<std::string, IRECV*>       apRecv;

   IO_OBJECT::OBJECTHEAD*              pObjectHead;
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
      IO_SESSION* pSourceSB = dynamic_cast<IO_SESSION*> (pSource);

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

bool CLIENT::SocketConnect () 
{ 
   return m_pImpl->pControl->SocketConnect ();
}

bool CLIENT::SocketReconnect () 
{
   return m_pImpl->pControl->SocketReconnect ();
}

bool CLIENT::SocketDisconnect (bool bVoluntary)
{ 
   return m_pImpl->pControl->SocketDisconnect ();
}

// ==========================================================================================================================

RMAP::CORE::CLIENT::IACTION* CLIENT::Request (const RMAP::CORE::CLIENT::ACTION* pAction)
{
   const ACTION* pActionIO = dynamic_cast<const ACTION*> (pAction);

   return new IACTION (this, pActionIO);
}

// ==========================================================================================================================

uint32_t CLIENT::Object_Recover (ordered_json& jData)
{
   uint32_t dwResult = 0;

   m_pImpl->pObjectHead->twParentIx    = jData["pObjectHead"]["twParentIx"];
   m_pImpl->pObjectHead->twObjectIx    = jData["pObjectHead"]["twObjectIx"];
   m_pImpl->pObjectHead->wClass_Parent = jData["pObjectHead"]["wClass_Parent"];
   m_pImpl->pObjectHead->wClass_Object = jData["pObjectHead"]["wClass_Object"];
   m_pImpl->pObjectHead->wFlags        = jData["pObjectHead"]["wFlags"];
   m_pImpl->pObjectHead->twEventIz     = jData["pObjectHead"]["twEventIz"];

   m_pImpl->pClient->m_pMem->Object_Update (m_pImpl->pObjectHead, this, &jData);

   return dwResult;
}

bool CLIENT::onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam)
{
   IO_OBJECT* pSourceIO = dynamic_cast<IO_OBJECT*> (pObject);

   pSourceIO->Map_Write (pParam, m_pImpl->pObjectHead->wFlags, bDiscard);

   return true;
}

bool CLIENT::onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam)
{
   return true;
}

RMAP::CORE::MEM::MEM* CLIENT::pMem ()
{
   return m_pMem;
}

bool CLIENT::bNetConnected () 
{ 
   return m_pImpl->pControl->bNetConnected (); 
}

void CLIENT::Recv_Register (std::string sAction, IRECV* pIRecv)
{
   m_pImpl->apRecv[sAction] = pIRecv;
}

void CLIENT::Recv_Unregister (std::string sAction)
{
   m_pImpl->apRecv.erase (sAction);
}

NET* CLIENT::pNet ()
{
   return m_pImpl->pNet;
}

void CLIENT::onConnected ()
{
   SERVICE* pServiceIO = dynamic_cast<SERVICE*> (pService ());

   pServiceIO->Connected (this);
}

void CLIENT::onDisconnected ()
{
   SERVICE* pServiceIO = dynamic_cast<SERVICE*> (pService ());

   m_pImpl->SocketDisconnected (false);

   pServiceIO->Disconnected (this);
}

bool CLIENT::onRecv_Request (std::string const& sEventName, ordered_json& jData)
{
   bool bResult = true; // ??
   auto search = m_pImpl->apRecv.find (sEventName);

   if (search != m_pImpl->apRecv.end ())
      bResult = search->second->onRecv_Request (sEventName, jData);

   return bResult;
}

bool CLIENT::Object_Subscribe (int wClass, uint64_t twObjectIx)
{
   bool bResult = false;

   if (m_pImpl->pSubscription->Add (wClass, twObjectIx) != false)
   {
//      m_pImpl->pSubscription->Subscribe_Aux ();

      bResult = true;
   }

   return bResult;
}

bool CLIENT::Object_Unsubscribe (int wClass, uint64_t twObjectIx)
{
   bool bResult = false;

   if (m_pImpl->pSubscription->Remove (wClass, twObjectIx) != false)
   {
//      m_pImpl->pSubscription->Subscribe_Aux ();

      bResult = true;
   }

   return bResult;
}

/******************************************************************************************************************************/
