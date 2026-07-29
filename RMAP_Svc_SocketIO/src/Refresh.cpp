/*******************************************************************************************************************************
**                                                                                                                            **
**                                      RMAP_Svc_SocketIO  : Refresh.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class REFRESH::Impl
{
public:
   Impl (CLIENT* pClient, REFRESH* pRefresh) :
      pClient (pClient)
   {
      pIOChange = new IOCHANGE;

      pClient->Recv_Register ("refresh", pRefresh);
   }

   ~Impl ()
   {
      pClient->Recv_Unregister ("refresh");

      delete pIOChange;
   }

   CLIENT* pClient;
   IOCHANGE* pIOChange;
};

/*******************************************************************************************************************************
**                                                   CLASS (REFRESH)                                                          **
*******************************************************************************************************************************/

REFRESH::REFRESH (CLIENT* pClient)
{
   m_pImpl = new REFRESH::Impl (pClient, this);
}

REFRESH::~REFRESH ()
{
   delete m_pImpl;
}

bool REFRESH::Event_Refresh (ordered_json& jResponse)
{
   m_pImpl->pIOChange->jChange = jResponse["pChange"];

   bool bResult = m_pImpl->pClient->pMem ()->Object_Change
   (
      jResponse["pControl"]["wClass_Object"],
      jResponse["pControl"]["twObjectIx"],
      jResponse["pControl"]["wClass_Child"],
      jResponse["pControl"]["twChildIx"],
      jResponse["pControl"]["wFlags"],
      m_pImpl->pIOChange,
      this,
      &jResponse
   );

   return bResult;
}

bool REFRESH::onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam)
{
   return false;
}

bool REFRESH::onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam)
{
   IO_OBJECT* pObjectIO = dynamic_cast<IO_OBJECT*> (pObject);
   IO_OBJECT* pChildIO = dynamic_cast<IO_OBJECT*> (pChild);
   ordered_json jData = (ordered_json&)pParam;

   pObjectIO->SetData (jData);

   if (jData["pControl"]["wClass_Child"] != 0)
   {
      if (pChildIO != NULL)
      {
         pChildIO->SetData (jData["pChild"]);
      }
   }

   return true;
}

bool REFRESH::onRecv_Request (std::string sAction, ordered_json& jData)
{
   return Event_Refresh (jData);
}

/******************************************************************************************************************************/
