/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVIO_cpp : Recover.cpp                                                       **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class RECOVER::Impl
{
public:
   Impl (CLIENT* pClient, RECOVER* pRecover) :
      pClient (pClient)
   {
      pClient->Recv_Register ("recover", pRecover);
   }

   ~Impl ()
   {
      pClient->Recv_Unregister ("recover");
   }

   CLIENT* pClient;
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

void RECOVER::Object_Recover (ordered_json& jResponse)
{
   int nResultSet, nObject;
   ordered_json pRow, pObjectHead;
   RMAP::CORE::MEM::SOURCE* pObject;

   if (jResponse["nResult"] == 0)
   {
      nResultSet = 0;

      if (nResultSet < jResponse["aResultSet"].size ())
      {
         pRow = jResponse["aResultSet"][nResultSet];

         if (pRow.size () == 1)
         {
            nObject = 0;

            try
            {
               ordered_json jTmp = ordered_json::parse (pRow[nObject]["Object"].template get<std::string> ());
               // Delete Object
               pRow[nObject]["ObjectX"] = jTmp;
            }
            catch (const ordered_json::parse_error& e)
            {
               (void)e;
            }

            pObjectHead = pRow[nObject]["ObjectX"]["pObjectHead"];

            if ((pObject = (m_pImpl->pClient->pMem ()->ObjectBank (pObjectHead["wClass_Object"]))->Get (NULL, pObjectHead["twObjectIx"])) != NULL)
            {
               // pObject.Recovering (pObjectHead.wClass_Object);
               m_pImpl->pClient->pMem ()->Object_Expire_Full (pObject);

               m_pImpl->pClient->Object_Recover (pRow[nObject]["ObjectX"]);

               for (nResultSet++; nResultSet < jResponse["aResultSet"].size (); nResultSet++)
               {
                  pRow = jResponse["aResultSet"][nResultSet];

                  if (pRow.size () > 0)
                  {
                     for (nObject = 0; nObject < pRow.size (); nObject++)
                     {
                        try
                        {
                           ordered_json jTmp = ordered_json::parse (pRow[nObject]["Object"].template get<std::string> ());
                           // Delete Object
                           pRow[nObject]["ObjectX"] = jTmp;
                        }
                        catch (const ordered_json::parse_error& e)
                        {
                           (void)e;
                        }
                     }

                     // pObject.Recovering (pRow[0].Object.ObjectHead.wClass_Object);

                     for (nObject = 0; nObject < pRow.size (); nObject++)
                     {
                        m_pImpl->pClient->Object_Recover (pRow[nObject]["ObjectX"]);
                     }

                     // pObject.Recovered (pRow[0].Object.ObjectHead.wClass_Object);
                  }
               }

               m_pImpl->pClient->pMem ()->Object_Purge_Full (pObject);
               // pObject.Recovered (pObjectHead.wClass_Object);
            }
         }
      }
   }
}

bool RECOVER::onRecv_Request (std::string sAction, ordered_json& jData)
{
   Object_Recover (jData);

   return true;
}

/******************************************************************************************************************************/
