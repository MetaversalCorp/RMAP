/*******************************************************************************************************************************
**                                                                                                                            **
**                                   RMAP_Svc_SocketIO : Control.cpp                                                          **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

static const CLIENT::ACTION SUBSCRIBEX
(
   "subscribe",
   "{"
      "\"twObjectIx\"            : 0,"
      "\"wClass_Object\"         : 0"
   "}",
   NULL
);

static const CLIENT::ACTION UNSUBSCRIBEX
(
   "unsubscribe",
   "{"
      "\"twObjectIx\"            : 0,"
      "\"wClass_Object\"         : 0"
   "}",
   NULL
);

static const std::map<std::string, const CLIENT::ACTION*> g_aAction_Subscribe =
{
   { "SUBSCRIBE",      &SUBSCRIBEX     },
   { "UNSUBSCRIBE",    &UNSUBSCRIBEX   },
};

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

typedef struct
{
   uint16_t                            nCount;
   uint8_t                             bFlag;
   uint16_t                            wClass;
   uint64_t                            twObjectIx;
}
SUBSCRIPTIONDATA;

typedef struct
{
   uint16_t                            wState;
   uint16_t                            wClass;
   uint64_t                            twObjectIx;
}
SBA_SUBSCRIBE_EX_IN;

class SUBSCRIPTION::Impl : public RMAP::CORE::IRESPONSE
{
public:
   Impl (SUBSCRIPTION* pSubscription, CLIENT* pClient) :
      pSubscription (pSubscription),
      pClient (pClient),
      bLockCount (0),
      bDirty (0)
   {
   }

   ~Impl ()
   {
   }

   RMAP::CORE::CLIENT::IACTION* Request (std::string sAction)
   {
      RMAP::CORE::CLIENT::IACTION* pResult;

      auto j = g_aAction_Subscribe.find (sAction);

      if (j != g_aAction_Subscribe.end ())
      {
         pResult = pClient->Request (const_cast<CLIENT::ACTION*> (j->second));
      }
      else pResult = NULL;

      return pResult;
   }

   SUBSCRIPTIONDATA* Get (int wClass, uint64_t twObjectIx)
   {
      SUBSCRIPTIONDATA* pSubscriptionData = NULL;
      int bSubscription;

      for (bSubscription = 0; bSubscription < aSubscription.size (); bSubscription++)
      {
         if (aSubscription[bSubscription].wClass == wClass && aSubscription[bSubscription].twObjectIx == twObjectIx)
         {
            pSubscriptionData = &aSubscription[bSubscription];
            break;
         }
      }

      return pSubscriptionData;
   }

   void Insert (int wClass, uint64_t twObjectIx)
   {
      SUBSCRIPTIONDATA Subscription;

      Subscription.nCount     = 1;
      Subscription.bFlag      = 0;
      Subscription.wClass     = wClass;
      Subscription.twObjectIx = twObjectIx;

      aSubscription.push_back (Subscription);
   }

   void Delete (SUBSCRIPTIONDATA* pSubscriptionData)
   {
      RMAP::CORE::MEM::OBJECTBANK* pObjectBank;
      RMAP::CORE::MEM::SOURCE* pObject;
      int n;

      if ((pObjectBank = pClient->pMem ()->ObjectBank (pSubscriptionData->wClass)) != NULL)
         if ((pObject = pObjectBank->Get (NULL, pSubscriptionData->twObjectIx)) != NULL)
            pClient->pMem ()->Object_Delete_Full (pObject);

      for (n = 0; n < aSubscription.size () && (aSubscription[n].wClass != pSubscriptionData->wClass || aSubscription[n].twObjectIx != pSubscriptionData->twObjectIx); n++);

      if (n < aSubscription.size ())
      {
         aSubscription.erase (aSubscription.begin () + n);
      }
   }

   void Objects_Response (RMAP::CORE::CLIENT::IACTION* pIAction)
   {
/*
      int wCount;
      SUBSCRIPTIONDATA* pSubscription;
      CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

      ordered_json pRequest  = pIActionSB->GetRequest ();
      ordered_json pResponse = pIActionSB->GetResponse ();

      if (pIActionSB->GetResult () == SBA_RESULT_SUCCESS && pRequest["wCount"] == pResponse["wCount"])
      {
         std::lock_guard<std::recursive_mutex> guard (m_CS);
         {
            uint16_t wReponse_Count = pResponse["wCount"];
            uint16_t wState;

            for (wCount = 0; wCount < wReponse_Count; wCount++)
            {
               if (pSubscription = Get (pRequest["aSBA_Subscribe_Ex_In"][wCount]["wClass"], pRequest["aSBA_Subscribe_Ex_In"][wCount]["twObjectIx"]))
               {
                  // What should we do about failures?
                  // if (pResponse.aSBA_Subscribe_Ex_Out[wCount].dwResult == SBA_RESULT_SUBSCRIBE_INVALIDSUBSCRIPTION)
                  //    pRequest.aSBA_Subscribe_Ex_In[wCount].wState = SBA_SUBSCRIBE_STATE_REMOVE;

                  wState = pRequest["aSBA_Subscribe_Ex_In"][wCount]["wState"];
                  switch (wState)
                  {
                  case SBA_SUBSCRIBE_STATE_RECOVER_SELF: 
                     if (pResponse["aSBA_Subscribe_Ex_Out"][wCount]["dwResult"] == SBA_RESULT_SUCCESS)
                     {
                        pSubscription->bFlag &= ~SUBSCRIPTION::eFLAG::SUBSCRIBING;
                        pSubscription->bFlag |= SUBSCRIPTION::eFLAG::SUBSCRIBED;
                     }
                     // else if (pResponse.aSBA_Subscribe_Ex_Out[wCount].dwResult == SBA_RESULT_SUBSCRIBE_INVALIDSUBSCRIPTION)
                     //    this.Callback_Object_Ex (pRequest.aSBA_Subscribe_Ex_In[wCount].wClass, pRequest.aSBA_Subscribe_Ex_In[wCount].twObjectIx, SUBSCRIPTION_NOTIFY_CODE_OBJECT_INVALID, SBO_CLASS_NULL, SBD_OBJECTIX_NULL, 0, NULL, NULL, NULL, NULL);
                     break;

                  case SBA_SUBSCRIBE_STATE_RESET:        
                     if (pResponse["aSBA_Subscribe_Ex_Out"][wCount]["dwResult"] == SBA_RESULT_SUCCESS)
                        pSubscription->bFlag &= ~SUBSCRIPTION::eFLAG::RESET;
                     break;

                  case SBA_SUBSCRIBE_STATE_REMOVE:    
                  // if (pResponse["aSBA_Subscribe_Ex_Out"][wCount]["dwResult"] == SBA_RESULT_SUCCESS)
                     {
                        pSubscription->bFlag &= ~SUBSCRIPTION::eFLAG::UNSUBSCRIBING;
                        pSubscription->bFlag &= ~SUBSCRIPTION::eFLAG::SUBSCRIBED;

                        // It is possible that a subscription was added while we were awaiting this response.
                        if (pSubscription->nCount == 0 && (pSubscription->bFlag & (SUBSCRIPTION::eFLAG::SUBSCRIBING | SUBSCRIPTION::eFLAG::SUBSCRIBED | SUBSCRIPTION::eFLAG::UNSUBSCRIBING)) == 0)
                        {
                           DeleteEx (pSubscription);
                        }
                     }
                     break;
                  }
               }
            }
         }
      }
*/
   }

   void onResponse (RMAP::CORE::CLIENT::IACTION* pIAction, int nType, intptr_t pParam) override
   {
      CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

      Objects_Response (pIActionSB);
   }

   SUBSCRIPTION*  pSubscription;
   CLIENT*        pClient;
   int            bLockCount;
   int            bDirty;

   std::recursive_mutex m_CS;

   std::vector<SUBSCRIPTIONDATA> aSubscription;
};

/*******************************************************************************************************************************
**                                                   CLASS (CONTROL)                                                          **
*******************************************************************************************************************************/

SUBSCRIPTION::SUBSCRIPTION (CLIENT* pClient)
{
   m_pImpl = new SUBSCRIPTION::Impl (this, pClient);
}

SUBSCRIPTION::~SUBSCRIPTION ()
{
   delete m_pImpl;
}

bool SUBSCRIPTION::Add (int wClass, uint64_t twObjectIx)
{
   bool bResult = false;
   SUBSCRIPTIONDATA *pSubscription;

   std::lock_guard<std::recursive_mutex> guard (m_pImpl->m_CS);
   {
      if ((pSubscription = m_pImpl->Get (wClass, twObjectIx)) == NULL)
      {
//         if (m_pImpl->aSubscription.size () < MV_CLIENT_SUBSCRIPTION_COUNT)
         {
            bResult = true;
            m_pImpl->Insert (wClass, twObjectIx);
         }
      }
      else
      {
         bResult = true;
         pSubscription->nCount++;
      }
   }

   if (pSubscription == NULL)
   {
      CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (m_pImpl->Request ("SUBSCRIBE"));

      ordered_json& pRequest = pIActionIO->GetRequest ();

      pRequest["twObjectIx"]    = twObjectIx;
      pRequest["wClass_Object"] = wClass;

      if (pIActionIO->Send (m_pImpl, 0, 0) != false)
      {
      }
   }

   return bResult;
}

bool SUBSCRIPTION::Remove (int wClass, uint64_t twObjectIx)
{
   bool bResult = false;
   bool bUnsubscribe = false;
   SUBSCRIPTIONDATA* pSubscription;

   std::lock_guard<std::recursive_mutex> guard (m_pImpl->m_CS);
   {
      if ((pSubscription = m_pImpl->Get (wClass, twObjectIx)) != NULL)
      {
         if (pSubscription->nCount > 0)
         {
            pSubscription->nCount--;

            if (pSubscription->nCount == 0/* && (pSubscription->bFlag & (eFLAG::SUBSCRIBING | eFLAG::SUBSCRIBED | eFLAG::UNSUBSCRIBING)) == 0*/)
            {
               m_pImpl->Delete (pSubscription);
               bUnsubscribe = true;
            }

            bResult = true;
         }
      }
   }

   if (bUnsubscribe)
   {
      CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (m_pImpl->Request ("UNSUBSCRIBE"));

      ordered_json& pRequest = pIActionIO->GetRequest ();

      pRequest["twObjectIx"]    = twObjectIx;
      pRequest["wClass_Object"] = wClass;

      if (pIActionIO->Send (m_pImpl, 0, 0) != false)
      {
      }
   }

   return bResult;
}
/*
bool SUBSCRIPTION::Reset (int wClass, uint64_t twObjectIx)
{
   bool bResult = false;
   SUBSCRIPTIONDATA* pSubscription;

   std::lock_guard<std::recursive_mutex> guard (m_pImpl->m_CS);
   {
      if ((pSubscription = m_pImpl->Get (wClass, twObjectIx)) != NULL)
      {
         pSubscription->bFlag |= eFLAG::RESET;

         bResult = true;
      }
   }

   return bResult;
}

void SUBSCRIPTION::Disconnected (bool bVoluntary, bool bDisconnected)
{
   int bSubscription;
   SUBSCRIPTIONDATA* pSubscription;

   std::lock_guard<std::recursive_mutex> guard (m_pImpl->m_CS);
   {
      for (bSubscription = 0; bSubscription < m_pImpl->aSubscription.size (); bSubscription++)
      {
         pSubscription = &m_pImpl->aSubscription[bSubscription];

         if ((pSubscription->bFlag & eFLAG::UNSUBSCRIBING) != 0)
         {
            pSubscription->bFlag &= ~eFLAG::UNSUBSCRIBING;
            pSubscription->bFlag &= ~eFLAG::SUBSCRIBED;

            // It is possible that a subscription was added while we were awaiting this response.
            if (pSubscription->nCount == 0 && (pSubscription->bFlag & (eFLAG::SUBSCRIBING | eFLAG::SUBSCRIBED | eFLAG::UNSUBSCRIBING)) == 0)
            {
               m_pImpl->DeleteEx (pSubscription);
            }
            else pSubscription->bFlag = 0;
         }
         else pSubscription->bFlag = 0;

      }

      if (bVoluntary == false)
         m_pImpl->pClient->pMem ()->Object_Expire_All ();
      else m_pImpl->pClient->pMem ()->Object_Delete_All ();
   }
}
*/

void SUBSCRIPTION::Subscribe_Aux ()
{
   int bSubscription;
   SUBSCRIPTIONDATA* pSubscription;
   std::vector<SUBSCRIPTIONDATA> aSubscription;

   std::lock_guard<std::recursive_mutex> guard (m_pImpl->m_CS);
   {
      aSubscription = m_pImpl->aSubscription;
   }

   for (bSubscription = 0; bSubscription < aSubscription.size (); bSubscription++)
   {
      pSubscription = &aSubscription[bSubscription];

      CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (m_pImpl->Request ("SUBSCRIBE"));

      ordered_json& pRequest = pIActionIO->GetRequest ();

      pRequest["twObjectIx"]    = aSubscription[bSubscription].twObjectIx;
      pRequest["wClass_Object"] = aSubscription[bSubscription].wClass;

      if (pIActionIO->Send (m_pImpl, 0, 0) != false)
      {
      }
   }
}

/******************************************************************************************************************************/
