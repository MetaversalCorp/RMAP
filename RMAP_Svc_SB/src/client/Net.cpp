/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVSB_cpp : Net.cpp                                                           **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

using namespace RMAP::SVC_SB;

#pragma warning (disable : 4267 )
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#pragma warning (default : 4267 )

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<asio::ssl::context> context_ptr;
typedef websocketpp::config::asio_tls_client::message_type::ptr message_ptr;

typedef client::connection_ptr connection_ptr;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class NET::connection_metadata
{
public:
   using ptr = websocketpp::lib::shared_ptr<connection_metadata>;

   connection_metadata (int id, websocketpp::connection_hdl hdl, std::string uri, NET::Impl* pImpl);

   void on_open    (client* c, websocketpp::connection_hdl hdl);
   void on_close   (client* c, websocketpp::connection_hdl hdl);
   void on_fail    (client* c, websocketpp::connection_hdl hdl);
   void on_message (websocketpp::connection_hdl hdl, client::message_ptr msg);

   std::string get_status () const;
   int get_id () const;
   websocketpp::connection_hdl get_hdl () const;

private:

   int m_id;
   websocketpp::connection_hdl m_hdl;
   std::string m_status;
   std::string m_uri;
   std::string m_server;
   std::string m_error_reason;

   NET::Impl* m_pImpl;
};

class NET::websocket_endpoint
{
public:

   websocket_endpoint (NET::Impl* pImpl) :
      m_pImpl (pImpl)
   {
      m_endpoint.clear_access_channels (websocketpp::log::alevel::all);
      m_endpoint.clear_error_channels (websocketpp::log::elevel::all);

      m_endpoint.init_asio ();

      // TLS
      m_endpoint.set_tls_init_handler (websocketpp::lib::bind (&on_tls_init));

      m_endpoint.start_perpetual ();

      m_thread.reset (new websocketpp::lib::thread (&client::run, &m_endpoint));

   }

   ~websocket_endpoint ()
   {
      m_endpoint.stop_perpetual ();

      for (con_list::const_iterator it = m_connection_list.begin ();
         it != m_connection_list.end ();
         ++it)
      {
         if (it->second->get_status () != "Open")
         {
            continue;
         }

         std::cout << "> Closing connection " << it->second->get_id () << std::endl;

         websocketpp::lib::error_code ec;
         m_endpoint.close (it->second->get_hdl (), websocketpp::close::status::going_away, "", ec);
         if (ec)
         {
            std::cout
               << "> Error closing connection " << it->second->get_id () << ": "
               << ec.message () << std::endl;
         }
      }

      m_thread->join ();
   }

   int connect (std::string const& uri)
   {

      websocketpp::lib::error_code ec;
      client::connection_ptr con = m_endpoint.get_connection (uri, ec);

      if (ec)
      {
         std::cout << "> Connect initialization error: " << ec.message () << std::endl;
         return -1;
      }

      int new_id = m_next_id++;
      NET::connection_metadata::ptr metadata_ptr (new NET::connection_metadata (new_id, con->get_handle (), uri, m_pImpl));
      m_connection_list[new_id] = metadata_ptr;


      con->set_open_handler (websocketpp::lib::bind (
         &NET::connection_metadata::on_open,
         metadata_ptr,
         &m_endpoint,
         websocketpp::lib::placeholders::_1
      ));
      con->set_fail_handler (websocketpp::lib::bind (
         &NET::connection_metadata::on_fail,
         metadata_ptr,
         &m_endpoint,
         websocketpp::lib::placeholders::_1
      ));
      con->set_close_handler (websocketpp::lib::bind (
         &NET::connection_metadata::on_close,
         metadata_ptr,
         &m_endpoint,
         websocketpp::lib::placeholders::_1
      ));
      con->set_message_handler (websocketpp::lib::bind (
         &NET::connection_metadata::on_message,
         metadata_ptr,
         websocketpp::lib::placeholders::_1,
         websocketpp::lib::placeholders::_2
      ));

      m_endpoint.connect (con);

      return new_id;

   }

   void close (int id, websocketpp::close::status::value code, std::string reason) 
   {
      websocketpp::lib::error_code ec;

      con_list::iterator metadata_it = m_connection_list.find (id);
      if (metadata_it == m_connection_list.end ()) {
         std::cout << "> No connection found with id " << id << std::endl;
         return;
      }

      m_endpoint.close (metadata_it->second->get_hdl (), code, reason, ec);
      if (ec) {
         std::cout << "> Error initiating close: " << ec.message () << std::endl;
      }
   }

   void send (int id, void const* payload, size_t len)
   {
      websocketpp::lib::error_code ec;

      con_list::iterator metadata_it = m_connection_list.find (id);
      if (metadata_it == m_connection_list.end ())
      {
         std::cout << "> No connection found with id " << id << std::endl;
         return;
      }

      m_endpoint.send (metadata_it->second->get_hdl (), payload, len, websocketpp::frame::opcode::binary, ec);
      if (ec) {
         std::cout << "> Error sending message: " << ec.message () << std::endl;
         return;
      }
   }

   NET::connection_metadata::ptr get_metadata (int id) const
   {
      con_list::const_iterator metadata_it = m_connection_list.find (id);
      if (metadata_it == m_connection_list.end ())
      {
         return NET::connection_metadata::ptr ();
      }
      else {
         return metadata_it->second;
      }
   }

   static context_ptr on_tls_init ()
   {
      context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context> (asio::ssl::context::tlsv12);

      try
      {
         ctx->set_options
         (
            asio::ssl::context::default_workarounds |
            asio::ssl::context::no_sslv2 |
            asio::ssl::context::no_sslv3 |
            asio::ssl::context::single_dh_use
         );
      }
      catch (std::exception& e)
      {
         std::cout << e.what () << std::endl;
      }

      return ctx;
   }

private:
   using con_list = std::map<int, NET::connection_metadata::ptr>;

   client m_endpoint;
   websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

   con_list m_connection_list;
   int m_next_id;

   NET::Impl* m_pImpl;
};

class NET::Impl
{
public:
   Impl (NET* pNet, CLIENT* pClient, INET* pINet) :
      m_pNet (pNet),
      pClient (pClient),
      pINet (pINet),
      m_twPacketIx (0),
      m_kState (eSTATE::NOTCONNECTED),
      //      dwResult (0),
      nTIMEOUT (10)
   {
      m_pEndPoint = new websocket_endpoint (this);
   }

   ~Impl ()
   {
      if (GetState () > eSTATE::NOTCONNECTED)
         onClose ();

      delete m_pEndPoint;
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

   void Close (int dwResult)
   {
      eSTATE kState = GetState ();

      if (kState > eSTATE::NOTCONNECTED && kState < eSTATE::CLOSED)
      {
         SetState (eSTATE::CLOSED);

         //      this.dwResult = dwResult;

         //      if (dwResult != this.eRESULT.DISCONNECT)
         //         console.log ('UNEXPECTED SOCKET CLOSED: (' + dwResult + ')');

         m_pEndPoint->close (0, websocketpp::close::status::going_away, "");
      }
   }

   void onClose ()
   {
      eSTATE kStateOld = SetState (eSTATE::NOTCONNECTED);

      if (m_pIControl != NULL)
         m_pIControl->SocketDisconnect_Complete ((int)SetVDParam (m_bVoluntary, false, m_bDisconnected));

      if (kStateOld > eSTATE::CONNECTING)
         pINet->onDisconnected ();
   }

   bool Connect (bool bSecure, std::string sHost, int wPort, ICONTROL* pIControl, bool bVoluntary, int nTimeout)
   {
      bool bResult = true;
      websocketpp::lib::error_code ec;
      connection_ptr con;

      if (GetState () == eSTATE::NOTCONNECTED)
      {
         m_pIControl  = pIControl;
         m_bVoluntary = bVoluntary;

         m_twPacketIx = 0;
         for (auto const& x : m_aPacket)
         {
            delete x.second;
         }
         m_aPacket.clear ();

         sEndPoint = "wss://" + sHost + ":" + std::to_string (wPort) + "/WS";

         if (m_pEndPoint->connect (sEndPoint) != -1)
         {
            SetState (eSTATE::CONNECTING);
         }
         else bResult = false;
      }
      else bResult = false;

      return bResult;
   }

   void SendReq (CLIENT::IACTION* pIActionSB)
   {
      BYTESTREAM ByteStream;
      uint16_t wSend = pIActionSB->RequestSize ();
      uint16_t wControl = pIActionSB->IsResponse () ? 1 : 0;

      ByteStream.Write_TWORD (m_twPacketIx);
      ByteStream.Write_WORD  (wControl);
      ByteStream.Write_DWORD (pIActionSB->GetAction ());
      ByteStream.Write_WORD (wSend);
      ByteStream.Write_WORD (0);

      pIActionSB->WriteRequest (ByteStream, 0);

      if (pIActionSB->IsResponse ())
      {
         m_aPacket.insert (std::make_pair (m_twPacketIx, pIActionSB));
      }

      m_twPacketIx++;
      m_pEndPoint->send (0, &ByteStream.GetData ()[0], 16 + wSend);
   }

   bool SendRsp (CLIENT::IACTION* pIActionSB, uint64_t twPacketIx)
   {
      bool bResult = true;

      if (pIActionSB->IsResponse ())
      {
         BYTESTREAM ByteStream;
         uint16_t wSend = pIActionSB->GetResult () ? 0 : pIActionSB->ResponseSize ();
         uint16_t wControl = 2;

         ByteStream.Write_TWORD (m_twPacketIx);
         ByteStream.Write_WORD (wControl);
         ByteStream.Write_DWORD (pIActionSB->GetAction ());
         ByteStream.Write_WORD (wSend);
         ByteStream.Write_WORD (0);

         if (wSend)
            pIActionSB->WriteResponse (ByteStream, 0);

         if (ByteStream.EOS () != false)
         {
            m_pEndPoint->send (0, &ByteStream.GetData ()[0], 16 + wSend);
         }
         else bResult = false;
      }

      return bResult;
   }

   void Recv (uint64_t twPacketIx, BYTESTREAM *pByteStream, uint32_t dwResult)
   {
      CLIENT::IACTION* pIActionSB;
      auto x = m_aPacket.find (twPacketIx);

      if (x != m_aPacket.end ())
      {
         SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pClient->pService ());

         pServiceSB->Time_Latency ((int)x->second->GetSendDuration ());

         pIActionSB = x->second;

         pIActionSB->ReadResponse (pByteStream, dwResult);

         if (pByteStream->EOS () != false)
         {
            pIActionSB->Response ();
         }
         else Close (NET::eRESULT::INVALIDPACKET_DATA);

         m_aPacket.erase (x);
         delete pIActionSB;
      }
      else Close (NET::eRESULT::INVALIDPACKET_RESPONSE);
   }

   CLIENT* pClient;
   INET* pINet;

   ICONTROL*            m_pIControl;
   int                  nTIMEOUT;
   NET*                 m_pNet;

   bool                 m_bVoluntary;
   bool                 m_bDisconnected;

   std::string          sEndPoint;

private:
   websocket_endpoint*     m_pEndPoint;

   std::recursive_mutex    m_CS;
   eSTATE                  m_kState;

   uint64_t                                m_twPacketIx;
   std::map<uint64_t, CLIENT::IACTION*> m_aPacket;

/*
   client                  m_endpoint;
   ConnData::ptr           m_pConnHandle;
   websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;


   std::recursive_mutex m_CSNet;
   bool                 m_bShutdown;
*/
};

NET::connection_metadata::connection_metadata (int id, websocketpp::connection_hdl hdl, std::string uri, NET::Impl* pImpl)
   : m_id (id)
   , m_hdl (hdl)
   , m_status ("Connecting")
   , m_uri (uri)
   , m_server ("N/A")
   , m_pImpl (pImpl)
{
}

void NET::connection_metadata::on_open (client* c, websocketpp::connection_hdl hdl)
{
   m_status = "Open";

   //Get a connection form a handler.
   client::connection_ptr con = c->get_con_from_hdl (hdl);
   m_server = con->get_response_header ("Server");

   m_pImpl->SetState (eSTATE::CONNECTED);

   m_pImpl->m_pIControl->SocketConnect_Complete ((int)SetVDParam (m_pImpl->m_bVoluntary, true, false));

   m_pImpl->pINet->onConnected ();
}

void NET::connection_metadata::on_close (client* c, websocketpp::connection_hdl hdl)
{
   m_status = "Closed";

   client::connection_ptr con = c->get_con_from_hdl (hdl);
   std::stringstream s;
   s << "close code: " << con->get_remote_close_code ()
      << "("
      << websocketpp::close::status::get_string (con->get_remote_close_code ())
      << "), close reason: "
      << con->get_remote_close_reason ();

   m_error_reason = s.str ();

   m_pImpl->onClose ();
}

void NET::connection_metadata::on_fail (client* c, websocketpp::connection_hdl hdl)
{
   m_status = "Failed";

   client::connection_ptr con = c->get_con_from_hdl (hdl);
   m_server = con->get_response_header ("Server");
   m_error_reason = con->get_ec ().message ();

   m_pImpl->Close (eRESULT::SOCKETERROR);
}

void NET::connection_metadata::on_message (websocketpp::connection_hdl hdl, client::message_ptr msg)
{
   if (msg->get_opcode () == websocketpp::frame::opcode::BINARY)
   {
      BYTESTREAM* pByteStream = new BYTESTREAM ((uint8_t*)msg->get_payload ().data (), msg->get_payload ().size ());

      uint64_t twPacketIx  = pByteStream->Read_TWORD ();
      uint16_t wControl    = pByteStream->Read_WORD  ();
      uint32_t dwValue     = pByteStream->Read_DWORD ();
      uint16_t wSend       = pByteStream->Read_WORD  ();
      uint16_t wError      = pByteStream->Read_WORD  ();

      if (pByteStream->Offset () == 16)
      {
         if ((wControl & 0x0002) == 0)
         {
            // the packet is a request

            if (m_pImpl->pINet->onRecv_Request (twPacketIx, dwValue, wSend, pByteStream) == false)
            {
               RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

               pCore->LoggerWrite (RMAP::CORE::LOGGER::kLOGLEVEL_Error, LibrarySVC_SB::sModuleName, "onRecv_Request received an invalid packet");

               m_pImpl->Close (NET::eRESULT::INVALIDPACKET_REQUEST);
            }
         }
         else
         {
            // the packet is a response

            m_pImpl->Recv (twPacketIx, pByteStream, dwValue);
         }
      }
      else m_pImpl->Close (NET::eRESULT::INVALIDPACKET_HEADER);

      delete pByteStream;
   }
}

std::string NET::connection_metadata::get_status () const
{
   return m_status;
}

int NET::connection_metadata::get_id () const
{
   return m_id;
}

websocketpp::connection_hdl NET::connection_metadata::get_hdl () const
{
   return m_hdl;
}

/*******************************************************************************************************************************
**                                                   CLASS (CONTROL)                                                          **
*******************************************************************************************************************************/

NET::NET (CLIENT* pClient, INET* pINet)
{
   m_pImpl = new NET::Impl (this, pClient, pINet);
}

NET::~NET ()
{
   delete m_pImpl;
}

/******************************************************************************************************************************/

// ===== Public Methods =====================================================================================================

std::string const& NET::sEndPoint () const&
{
   return m_pImpl->sEndPoint;
}

bool NET::Connect (bool bSecure, std::string sHost, int wPort, ICONTROL* pIControl, bool bVoluntary, int nTimeout)
{
   return m_pImpl->Connect (bSecure, sHost, wPort, pIControl, bVoluntary, nTimeout);
}

bool NET::Disconnect (ICONTROL* pIControl, bool bVoluntary, bool bDisconnected, int nTimeout)
{
   bool bResult = true;

   if (!nTimeout)
      nTimeout = m_pImpl->nTIMEOUT;

   if (m_pImpl->GetState () == eSTATE::CONNECTED)
   {
      m_pImpl->m_bVoluntary    = bVoluntary;
      m_pImpl->m_bDisconnected = bDisconnected;

      m_pImpl->Close (eRESULT::DISCONNECT);                  // what if this doesn"t take, and no callback is forthcoming?
   }
   else bResult = false;

   return bResult;
}

bool NET::Send_Request (RMAP::CORE::CLIENT::IACTION* pIAction, int nTimeout)
{
   bool bResult = true;
   BYTESTREAM ByteStream;
   CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

   if (!nTimeout)
      nTimeout = m_pImpl->nTIMEOUT;

   if (m_pImpl->GetState () == eSTATE::CONNECTED)
   {
      m_pImpl->SendReq (pIActionSB);
   }
   else bResult = false;

   return bResult;
}

bool NET::Send_Response (RMAP::CORE::CLIENT::IACTION* pIAction, uint64_t twPacketIx)
{
   bool bResult = true;
   BYTESTREAM ByteStream;
   CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

   if (m_pImpl->GetState () == eSTATE::CONNECTED)
   {
      bResult = m_pImpl->SendRsp (pIActionSB, twPacketIx);
   }
   else bResult = false;

   delete pIAction;

   return bResult;
}

/******************************************************************************************************************************/
