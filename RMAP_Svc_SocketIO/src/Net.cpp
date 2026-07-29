/*******************************************************************************************************************************
**                                                                                                                            **
**                                               RMAP_Svc_SB  : Net.cpp                                                       **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <sio_client.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace RMAP::SVC_SOCKETIO;

using namespace sio;
using namespace std;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class THRCONTROL;
class IACTIONEX
{
public:
   IACTIONEX (CLIENT::IACTION* pIAction, sio::client* pClient, THRCONTROL* pThrControl);
   ~IACTIONEX ();

   ordered_json createJson (sio::message::ptr sio);
   bool IsComplete ();
   void Send ();

private:
   CLIENT::IACTION*           m_pIAction;
   sio::client*               m_pClient;

   std::recursive_mutex       m_CS;
   bool                       m_bComplete;

   THRCONTROL*                m_pThrControl;
};

class THRCONTROL
{
public:
   THRCONTROL () :
      m_bShutdown (false)
   {
      m_pThread = new std::thread (&THRCONTROL::ThreadLoop, this);
   }

   ~THRCONTROL ()
   {
      Shutdown ();
      m_pThread->join ();

      delete m_pThread;
   }

   void Shutdown ()
   {
      std::lock_guard<std::recursive_mutex> guard (m_mutex);
      m_bShutdown = true;
      m_condVar.notify_all ();
   }

   void ThreadLoop ()
   {
      std::unique_lock<std::recursive_mutex> mlock (m_mutex);
      m_condVar.wait (mlock, std::bind (&THRCONTROL::Control, this));
   }

   bool Control ()
   {
      bool bContinue;
      int i;

      if (m_bShutdown == false)
      {
         m_CS_Control.lock ();
         {
            do
            {
               for (i = 0; i < m_apIActionEx.size () && m_apIActionEx[i]->IsComplete () == false; i++);

               if (i < m_apIActionEx.size ())
               {
                  bContinue = true;
                  m_apIActionEx.erase (m_apIActionEx.begin () + i);
               }
               else bContinue = false;
            } while (bContinue);
         }
         m_CS_Control.unlock ();
      }

      return m_bShutdown;
   }

   void CtlBreak_Thread ()
   {
      std::lock_guard<std::recursive_mutex> guard (m_mutex);
      m_condVar.notify_all ();
   }

   void QueueItem (RMAP::SVC_SOCKETIO::CLIENT::IACTION* pIAction, sio::client* pClient)
   {
      IACTIONEX* pIActionEx = new IACTIONEX (pIAction, pClient, this);

      m_CS_Control.lock ();
      {
         m_apIActionEx.push_back (pIActionEx);
      }
      m_CS_Control.unlock ();

      pIActionEx->Send ();

      CtlBreak_Thread ();
   }

private:
   std::thread* m_pThread;
   std::recursive_mutex          m_mutex;
   std::condition_variable_any   m_condVar;
   bool                          m_bShutdown;

   std::recursive_mutex          m_CS_Control;
   std::vector<IACTIONEX*>       m_apIActionEx;
};

IACTIONEX::IACTIONEX (CLIENT::IACTION* pIAction, sio::client* pClient, THRCONTROL* pThrControl) :
   m_pIAction (pIAction),
   m_pClient (pClient),
   m_pThrControl (pThrControl),
   m_bComplete (false)
{
}

IACTIONEX::~IACTIONEX ()
{
}

ordered_json IACTIONEX::createJson (sio::message::ptr sio)
{
   // return json
   ordered_json json;

   try
   {
      // browse flags, we consider it can only be array/vector or object/map
      if (sio->get_flag () == sio::message::flag_array)
      {
         for (int i = 0; i < int (sio->get_vector ().size ()); ++i)
         {
            if (sio->get_vector ()[i]->get_flag () == sio::message::flag_object || sio->get_vector ()[i]->get_flag () == sio::message::flag_array)
            {
               json[i] = createJson (sio->get_vector ()[i]);
            }
            else if (sio->get_vector ()[i]->get_flag () == sio::message::flag_integer)
            {
               json[i] = sio->get_vector ()[i]->get_int ();
            }
            else if (sio->get_vector ()[i]->get_flag () == sio::message::flag_double)
            {
               json[i] = sio->get_vector ()[i]->get_double ();
            }
            else if (sio->get_vector ()[i]->get_flag () == sio::message::flag_string)
            {
               json[i] = sio->get_vector ()[i]->get_string ();
            }
            else if (sio->get_vector ()[i]->get_flag () == sio::message::flag_boolean)
            {
               json[i] = (sio->get_vector ()[i]->get_bool () ? "true" : "false");
            }
            else if (sio->get_vector ()[i]->get_flag () == sio::message::flag_null)
            {
               // json[i] = "null"; // do not set json[i] so that it's set to json-null properly
            }
            else
            {
               //                  std::cout << "Unknown flag in vector: " << sio->get_flag () << ", i is " << i << std::endl;
            }
         }
      }
      else if (sio->get_flag () == sio::message::flag_object)
      {
         for (auto it = sio->get_map ().cbegin (); it != sio->get_map ().cend (); ++it)
         {
            if (it->second->get_flag () == sio::message::flag_object || it->second->get_flag () == sio::message::flag_array)
            {
               json[it->first] = createJson (it->second);
            }
            else if (it->second->get_flag () == sio::message::flag_integer)
            {
               json[it->first] = it->second->get_int ();
            }
            else if (it->second->get_flag () == sio::message::flag_double)
            {
               json[it->first] = it->second->get_double ();
            }
            else if (it->second->get_flag () == sio::message::flag_string)
            {
               json[it->first] = it->second->get_string ();
            }
            else if (it->second->get_flag () == sio::message::flag_boolean)
            {
               json[it->first] = it->second->get_bool ();
            }
            else if (it->second->get_flag () == sio::message::flag_null)
            {
               // json[it->first] = "null"; // do not set json[i] so that it's set to json-null properly
            }
            else
            {
               //                  std::cout << "Unknown flag in object: " << sio->get_flag () << ", it first is " << it->first << std::endl;
            }
         }
      }
      else
      {
         //            std::cout << "Unknown flag in createJson function: " << sio->get_flag () << std::endl;
      }
   }
   catch (nlohmann::json::exception& e)
   {
      (void)e;
      //         std::cout << "JSON exception caught in " << __FUNCTION__ << " (message: " << e.what () << ")" << std::endl;
   }

   // return
   return json;
}

bool IACTIONEX::IsComplete ()
{
   bool bResult;

   m_CS.lock ();
   {
      bResult = m_bComplete;
   }
   m_CS.unlock ();

   return bResult;
}

void IACTIONEX::Send ()
{
   std::string sRequest = m_pIAction->GetRequestEx ();

   m_pClient->socket ()->emit
   (
      m_pIAction->GetAction (),
      sRequest,
      [&](message::list const& msg)
      {
         ordered_json jResponse;

         if (msg.size () > 0)
         {
            jResponse = createJson (msg[0]);
            m_pIAction->SetResponse (jResponse);
         }

         m_pIAction->Response ();
         delete m_pIAction;

         m_CS.lock ();
         {
            m_bComplete = true;
         }
         m_CS.unlock ();

         m_pThrControl->CtlBreak_Thread ();
      }
   );
}

class NET::Impl
{
public:
   Impl (INET* pINet) :
      m_pINet (pINet),
      m_pIControl (NULL),
      m_bVoluntary (false),
      m_kState (eSTATE::NOTCONNECTED)
   {
      m_pThrControl = new THRCONTROL ();

      m_pClient   = new sio::client ();

      m_pClient->set_open_listener  (std::bind (&NET::Impl::onConnect, this));
      m_pClient->set_close_listener (std::bind (&NET::Impl::onClose, this, std::placeholders::_1));
      m_pClient->set_fail_listener  (std::bind (&NET::Impl::onFail, this));
   }

   ~Impl ()
   {
      delete m_pThrControl;

      m_pClient->sync_close ();
      m_pClient->clear_con_listeners ();

      delete m_pClient;
   }

   void onConnect ()
   {
      SetState (eSTATE::CONNECTED);

      m_pIControl->SocketConnect_Complete (true, false);
      m_pINet->onConnected ();

      m_pClient->socket ()->on 
      (
         "*",
         sio::socket::event_listener_aux
         (
            [&](string const& name, message::ptr const& data, bool isAck, message::list& ack_resp)
            {
               try
               {
                  ordered_json jAction = ordered_json::parse (data->get_string ());

                  m_pINet->onRecv_Request (name, jAction);
               }
               catch (const ordered_json::parse_error& e)
               {
                  (void)e;
               }
            }
         )
      );
   }

   void onClose (client::close_reason const& reason)
   {
      m_pINet->onDisconnected ();
   }

   void onFail ()
   {
      m_pINet->onDisconnected ();
   }

   NET::eSTATE GetState ()
   {
      NET::eSTATE kState;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         kState = m_kState;
      }

      return kState;
   }

   eSTATE SetState (eSTATE kState)
   {
      eSTATE kStateOld;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         kStateOld = m_kState;
         m_kState = kState;
      }

      return kStateOld;
   }

   bool Connect (const std::string& sEndPoint, ICONTROL* pIControl, bool bVoluntary)
   {
      bool bResult = true;

      if (GetState () == eSTATE::NOTCONNECTED)
      {
         m_pIControl  = pIControl;
         m_bVoluntary = bVoluntary;

/*
         const ioOptions =
         {
            autoConnect:   false,
            reconnection : false,
            transports : ['websocket']
         };
*/
         SetState (eSTATE::CONNECTING);
         m_pClient->connect (sEndPoint);
      }
      else bResult = false;

      return bResult;
   }

   void Close (uint32_t dwResult)
   {
      eSTATE kState = GetState ();

      if (kState > eSTATE::NOTCONNECTED && kState < eSTATE::CLOSED)
      {
         SetState (eSTATE::CLOSED);

         //      this.dwResult = dwResult;

         //      if (dwResult != this.eRESULT.DISCONNECT)
         //         console.log ('UNEXPECTED SOCKET CLOSED: (' + dwResult + ')');

         m_pClient->close ();
      }
   }

   bool Send (CLIENT::IACTION* pIAction)
   {
      bool bResult = true;

      if (GetState () == eSTATE::CONNECTED)
      {
         m_pThrControl->QueueItem (pIAction, m_pClient);
      }
      else bResult = false;

      return bResult;
   }

private:
   INET*                         m_pINet;
   ICONTROL*                     m_pIControl;
   bool                          m_bVoluntary;

   sio::client*                  m_pClient;

   std::recursive_mutex          m_CS;
   eSTATE                        m_kState;

   THRCONTROL*                   m_pThrControl;
};

/*******************************************************************************************************************************
**                                                   CLASS (NET)                                                              **
*******************************************************************************************************************************/

NET::NET (CLIENT* pClient, INET* pINet)
{
   m_pImpl = new NET::Impl (pINet);
}

NET::~NET ()
{
   delete m_pImpl;
}

/******************************************************************************************************************************/

// ===== Public Methods =====================================================================================================

bool NET::Connect (const std::string& sEndPoint, ICONTROL* pIControl, bool bVoluntary)
{
   return m_pImpl->Connect (sEndPoint, pIControl, bVoluntary);
}

bool NET::Disconnect (ICONTROL* pIControl, bool bVoluntary, bool bDisconnected)
{
   bool bResult = true;

   if (m_pImpl->GetState () == eSTATE::CONNECTED)
   {
//      m_pImpl->m_bVoluntary = bVoluntary;
//      m_pImpl->m_bDisconnected = bDisconnected;

      m_pImpl->Close (eRESULT::DISCONNECT);                  // what if this doesn"t take, and no callback is forthcoming?
   }
   else bResult = false;

   return bResult;
}

bool NET::Send_Request (RMAP::CORE::CLIENT::IACTION* pIAction)
{
   CLIENT::IACTION* pIActionIO = dynamic_cast<CLIENT::IACTION*> (pIAction);

   return m_pImpl->Send (pIActionIO);
}

/******************************************************************************************************************************/
