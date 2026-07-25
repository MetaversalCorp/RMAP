/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVSB_cpp : Client.cpp                                                        **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     CLASS (OBJECTHEAD)                                                     **
*******************************************************************************************************************************/

OBJECTHEAD::OBJECTHEAD () :
   RMAP::CORE::MEM::OBJECTHEAD (),
   twEventIz (0)
{
}

OBJECTHEAD::OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz) :
   RMAP::CORE::MEM::OBJECTHEAD (twParentIx, twObjectIx, wClass_Parent, wClass_Object, wFlags),
   twEventIz (twEventIz)
{
}

OBJECTHEAD::~OBJECTHEAD ()
{
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class CLIENT::IACTION::Impl
{
public:
   Impl (const ACTION* pAction) :
      m_pActionSB (pAction),
      m_pResponse (NULL),
      m_nType (0),
      m_pParam (0),
      m_bRequest (false),
      m_bResponse (false)
   {
      if (pAction->bSend && pAction->pIn)
      {
         m_bRequest = true;
         m_jIn = const_cast<MAP*> (m_pActionSB->pIn)->GetRequest ();
      }

      if (pAction->bSend == false && pAction->pOut)
      {
         m_bResponse = true;
         m_jOut = const_cast<MAP*> (m_pActionSB->pOut)->GetRequest ();
      }
   }

   ~Impl ()
   {
   }

   const ACTION*           m_pActionSB;

   ordered_json            m_jIn;
   ordered_json            m_jOut;

   bool                    m_bRequest;
   bool                    m_bResponse;

   std::vector<int>        m_aError;

   uint32_t                m_dwResult;
   int                     m_nType;
   intptr_t                m_pParam;
   RMAP::CORE::IRESPONSE*        m_pResponse;
   std::chrono::time_point<std::chrono::system_clock>  m_tSend;
};

/*******************************************************************************************************************************
**                                                     CLASS (IACTION)                                                     **
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

uint32_t CLIENT::IACTION::GetAction ()
{
   return m_pImpl->m_pActionSB->dwAction;
}

double CLIENT::IACTION::GetSendDuration ()
{
   const std::chrono::duration<double> elapsed_seconds{ std::chrono::system_clock::now () - m_pImpl->m_tSend };

   return (elapsed_seconds.count () * 1000);
}

void CLIENT::IACTION::SetResult (uint32_t dwResult)
{
   m_pImpl->m_dwResult = dwResult;
}

ordered_json& CLIENT::IACTION::GetRequest ()
{
   return m_pImpl->m_jIn;
}

ordered_json& CLIENT::IACTION::GetResponse ()
{
   return m_pImpl->m_jOut;
}

bool CLIENT::IACTION::IsSuccess ()
{
   return (m_pImpl->m_dwResult == 0);
}

void CLIENT::IACTION::WriteRequest (BYTESTREAM& ByteStream, int wOffset_Base)
{
   const_cast <MAP*> (m_pImpl->m_pActionSB->pIn)->Write (ByteStream, wOffset_Base, m_pImpl->m_jIn);

   ByteStream.Seek (const_cast <MAP*> (m_pImpl->m_pActionSB->pIn)->Size (m_pImpl->m_jIn));
}

void CLIENT::IACTION::WriteResponse (BYTESTREAM& ByteStream, int wOffset_Base)
{
   if (m_pImpl->m_pActionSB->pOut)
   {
      const_cast <MAP*> (m_pImpl->m_pActionSB->pOut)->Write (ByteStream, wOffset_Base, m_pImpl->m_jOut);

      ByteStream.Seek (const_cast <MAP*> (m_pImpl->m_pActionSB->pIn)->Size (m_pImpl->m_jOut));
   }
}

void CLIENT::IACTION::ReadRequest (BYTESTREAM* pByteStream)
{
   if (m_pImpl->m_pActionSB->pIn)
   {
      const_cast <MAP*> (m_pImpl->m_pActionSB->pIn)->Read (pByteStream, m_pImpl->m_jIn);

      //ByteStream.Seek (this.#pAction.Map_Request.Size (this.pRequest));   // This can't be a good thing to do. If the read does not leave the bytestream exactly where it should be, then something is wrong
   }
}

void CLIENT::IACTION::ReadResponse (BYTESTREAM* pByteStream, uint32_t dwResult)
{
   m_pImpl->m_dwResult = dwResult;

   if (m_pImpl->m_aError.size () > 0)
   {
/*
      for (let w = 0; w < this.#aError.length; w++)
      {
         let dwError = ByteStream.Read_DWORD ();
         let sError = ByteStream.Read_String (124);

         if (ByteStream.error != 0)
         {
            dwError = 0;
            sError = 'Improperly transmitted packet.';
         }

         this.#aError[w] = { dwError, sError };
      }
*/
   }
   else if (m_pImpl->m_dwResult == SBA_RESULT_SUCCESS && m_pImpl->m_pActionSB->pOut != NULL)
   {
      const_cast <MAP*> (m_pImpl->m_pActionSB->pOut)->Read (pByteStream, m_pImpl->m_jOut);

      //ByteStream.Seek (this.#pAction.Map_Request.Size (this.pRequest));   // This can't be a good thing to do. If the read does not leave the bytestream exactly where it should be, then something is wrong
   }
}

bool CLIENT::IACTION::IsRequest ()
{
   return m_pImpl->m_bRequest;
}

bool CLIENT::IACTION::IsResponse ()
{
   return m_pImpl->m_pActionSB->bResponse;
}

uint16_t CLIENT::IACTION::RequestSize ()
{
   return const_cast <MAP*> (m_pImpl->m_pActionSB->pIn)->Size (m_pImpl->m_jIn);
}

uint16_t CLIENT::IACTION::ResponseSize ()
{
   return const_cast <MAP*> (m_pImpl->m_pActionSB->pOut)->Size (m_pImpl->m_jOut);
}

void CLIENT::IACTION::Response ()
{
   if (m_pImpl->m_pResponse != NULL)
   {
      m_pImpl->m_pResponse->onResponse (this, m_pImpl->m_nType, m_pImpl->m_pParam);
   }
}

bool CLIENT::IACTION::Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam)
{
   bool bResult = false;
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (m_pClient);
   uint32_t dwAction = m_pImpl->m_pActionSB->dwAction;
   uint16_t wClass = SBA_CLASS (dwAction);

   if (pClientSB->ReadyState () > CLIENT::eSTATE::SYSTEMDISCONNECTED || wClass == SBO_CLASS_STATE)
   {
      m_pImpl->m_nType = nType;
      m_pImpl->m_pParam = pParam;
      m_pImpl->m_pResponse = pResponse;

      bResult = pClientSB->pNet ()->Send_Request (this, 0);

      m_pImpl->m_tSend = std::chrono::system_clock::now ();
   }

   return bResult;
}

uint32_t CLIENT::IACTION::GetResult ()
{
   return m_pImpl->m_dwResult;
}

/*******************************************************************************************************************************
**                                                     CLASS (ACTION)                                                      **
*******************************************************************************************************************************/

CLIENT::ACTION::ACTION (uint32_t dwAction, std::string sIn, std::string sOut, bool bResponse, bool bSend) :
   dwAction (dwAction),
   pIn (sIn.empty () ? NULL : new MAP (sIn)),
   pOut (sOut.empty () ? NULL : new MAP (sOut)),
   bResponse (bResponse),
   bSend (bSend)
{
}

CLIENT::ACTION::ACTION () :
   dwAction (0),
   pIn (NULL),
   pOut (NULL),
   bResponse (true),
   bSend (true)
{
}

CLIENT::ACTION::~ACTION ()
{
   delete pIn;
   delete pOut;
}

/*******************************************************************************************************************************
**                                                     CLASS (IREFERENCE)                                                     **
*******************************************************************************************************************************/

CLIENT::IREFERENCE::IREFERENCE (uint64_t twClientIx) :
   RMAP::CORE::CLIENT::IREFERENCE ("MVSB", twClientIx)
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

typedef struct tagRECVINFO
{
   const CLIENT::ACTION*               pAction;
   CLIENT::IRECV*                      pIRecv;
}
RECVINFO;

class CLIENT::Impl
{
public:
   Impl (CLIENT* pClient) :
      pClient (pClient)
   {
   }

   ~Impl ()
   {
      delete pNet;

      delete pControl;
      delete pSubscription;
      delete pRefresh;
      delete pRecover;
   }

   void Init ()
   {
      SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pClient->pService ());

      pRecover = new RECOVER (pClient);
      pRefresh = new REFRESH (pClient);
      pSubscription = new SUBSCRIPTION (pClient);
      pControl = new CONTROL (pClient, pServiceSB->pNetSettings (), pSubscription);
      pNet = new NET (pClient, pClient);
   }

   CLIENT* pClient;
   RECOVER* pRecover;
   REFRESH* pRefresh;
   SUBSCRIPTION* pSubscription;
   CONTROL* pControl;
   NET* pNet;

   std::map<uint32_t, RECVINFO> apRecv;
};

/*******************************************************************************************************************************
**                                                   CLASS (CLIENT)                                                        **
*******************************************************************************************************************************/

CLIENT::CLIENT (IREFERENCE* pReference, RMAP::CORE::SERVICE* pService) :
   RMAP::CORE::CLIENT (pReference, pService)
{
   tmCurrent = 0; // is this used by anyone?

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

std::string const& CLIENT::sEndPoint () const&
{
   return m_pImpl->pNet->sEndPoint ();
}

bool CLIENT::bNetConnected ()
{
   return m_pImpl->pControl->bNetConnected ();
}

bool CLIENT::bSystemConnected ()
{
   return m_pImpl->pControl->bSystemConnected ();
}

bool CLIENT::bLoggedIn ()
{
   return m_pImpl->pControl->bLoggedIn ();
}

RMAP::CORE::SOURCE_SESSION::LOGIN* CLIENT::pLogin ()
{
   return m_pImpl->pControl->pLogin ();
}

RMAP::CORE::MEM::MEM* CLIENT::pMem ()
{
   return m_pMem;
}

NET* CLIENT::pNet ()
{
   return m_pImpl->pNet;
}

bool CLIENT::SafeKill ()
{
   return m_pImpl->pControl->SafeKill ();
}

void CLIENT::SetAttemptState (int nReadyState)
{
   m_pImpl->pControl->SetAttemptState (nReadyState);
}

void CLIENT::Progress (RMAP::CORE::PROGRESS* pProgress)
{
   RMAP::CORE::SOURCE* pSource = Source (MV_SERVICE_OBJECT_SESSION); // this is pSession.pSource

   if (pSource)
   {
      SB_SESSION* pSourceSB    = dynamic_cast<SB_SESSION*> (pSource);

      pSourceSB->Progress (pProgress);
   }
}

void CLIENT::Tick (int uCode, TIME tmServer)
{
   RMAP::CORE::SOURCE* pSource = Source (MV_SERVICE_OBJECT_TIME); // this is pSession.pSource

   if (pSource)
   {
      SB_SBTIME* pSourceSB = dynamic_cast<SB_SBTIME*> (pSource);

      pSourceSB->Tick (uCode, tmServer);
   }
}

// ===== Public Methods =====================================================================================================

void CLIENT::Recv_Register (const ACTION* pAction, IRECV* pIRecv)
{
   RECVINFO rcv;

   rcv.pAction = pAction;
   rcv.pIRecv  = pIRecv;
   m_pImpl->apRecv[pAction->dwAction] = rcv;
}

void CLIENT::Recv_Unregister (const ACTION* pAction)
{
   m_pImpl->apRecv.erase (pAction->dwAction);
}

uint32_t CLIENT::onRecv_Request (uint64_t twPacketIx, uint32_t dwAction, int wSize, BYTESTREAM* pByteStream)
{
   bool bResult = false;
   auto search = m_pImpl->apRecv.find (dwAction);

   if (search != m_pImpl->apRecv.end ())
   {
      IACTION* pIAction = dynamic_cast<IACTION*> (Request (search->second.pAction));

      pIAction->ReadRequest (pByteStream);

      if (pIAction->IsRequest () == false || pByteStream->EOS () != false)
         bResult = search->second.pIRecv->onRecv_Request (pIAction, wSize, pByteStream);
      else pIAction->SetResult (0xE009);

      if (bResult == false)
         m_pImpl->pNet->Send_Response (pIAction, twPacketIx);
   }

   return true; // False will close the socket, we never want that to occur
}

// ==========================================================================================================================

bool CLIENT::IsDisconnected ()
{
   return (ReadyState () == eSTATE::SOCKETDISCONNECTED);
}

bool CLIENT::IsConnected ()
{
   return (ReadyState () >= eSTATE::LOGGEDOUT);
}

bool CLIENT::IsLoggedOut ()
{
   return (ReadyState () == eSTATE::LOGGEDOUT);
}

bool CLIENT::IsLoggedIn ()
{
   return (ReadyState () == eSTATE::LOGGEDIN);
}

// ==========================================================================================================================

RMAP::CORE::MODEL* CLIENT::Time_Open ()
{
   return Model_Open_Aux ("SBTime", "");
}

RMAP::CORE::MODEL* CLIENT::Time_Close (RMAP::CORE::MODEL* pTime)
{
   return Model_Close_Aux (pTime);
}

// ==========================================================================================================================

bool CLIENT::ClearError ()
{ 
   return m_pImpl->pControl->ClearError (); 
}

bool CLIENT::SetDevice (std::string acToken64U_Device)
{ 
   return m_pImpl->pControl->SetDevice (acToken64U_Device);
}

bool CLIENT::SocketConnect () 
{ 
   return m_pImpl->pControl->SocketConnect (); 
}

bool CLIENT::SocketReconnect () 
{ 
   return m_pImpl->pControl->SocketReconnect (); 
}

bool CLIENT::SocketDisconnect () 
{ 
   return m_pImpl->pControl->SocketDisconnect (); 
}

bool CLIENT::SocketDisconnected (bool bVoluntary) 
{ 
   return m_pImpl->pControl->SocketDisconnected (bVoluntary); 
}

bool CLIENT::SystemConnect ()
{ 
   return m_pImpl->pControl->SystemConnect (); 
}

bool CLIENT::SystemReconnect () 
{ 
   return m_pImpl->pControl->SystemReconnect (); 
}

bool CLIENT::SystemDisconnect () 
{ 
   return m_pImpl->pControl->SystemDisconnect (); 
}

bool CLIENT::Login (void* pParams)
{ 
   return m_pImpl->pControl->Login (Source (MV_SERVICE_OBJECT_SESSION), pParams); // this is pSession.pSource  // We should define an interface for the source to 
}

bool CLIENT::Logout (void* pParams) 
{ 
   return m_pImpl->pControl->Logout (Source (MV_SERVICE_OBJECT_SESSION), pParams);  // this is pSession.pSource  // a call directly into it. This will work for now.
}

// ==========================================================================================================================

bool CLIENT::Object_Subscribe (uint16_t wClass, uint64_t twObjectIx)
{
   bool bResult = false;

   if (m_pImpl->pSubscription->Add (wClass, twObjectIx) != false)
   {
      m_pImpl->pSubscription->Subscribe_Aux ();

      bResult = true;
   }

   return bResult;
}

bool CLIENT::Object_Unsubscribe (uint16_t wClass, uint64_t twObjectIx)
{
   bool bResult = false;

   if (m_pImpl->pSubscription->Remove (wClass, twObjectIx) != false)
   {
      m_pImpl->pSubscription->Subscribe_Aux ();

      bResult = true;
   }

   return bResult;
}

bool CLIENT::Object_Reset (uint16_t wClass, uint64_t twObjectIx)
{
   bool bResult = false;

   if (m_pImpl->pSubscription->Reset (wClass, twObjectIx) != false)
   {
      m_pImpl->pSubscription->Subscribe_Aux ();

      bResult = true;
   }

   return bResult;
}

// ==========================================================================================================================

RMAP::CORE::CLIENT::IACTION* CLIENT::Request (const RMAP::CORE::CLIENT::ACTION* pAction)
{
   const ACTION *pActionSB = dynamic_cast<const ACTION*> (pAction);

   return new IACTION (this, pActionSB);
}

// ==========================================================================================================================

TIME CLIENT::Time_Current ()
{
   return g_pTime->Current ();
}

TIME CLIENT::Time_Server ()
{
   SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pService ());

   return pServiceSB->Time_Server ();
}

void CLIENT::onConnected ()
{
   SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pService ());

   pServiceSB->Connected (this);
}

void CLIENT::onDisconnected ()
{
   SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pService ());

   SocketDisconnected (false);

   pServiceSB->Disconnected (this);
}

RMAP::CORE::SOURCE* CLIENT::SourceGet ()
{
   return Source (MV_SERVICE_OBJECT_SESSION);
}


/******************************************************************************************************************************/
