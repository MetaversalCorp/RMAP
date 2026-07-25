/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      MVSB_cpp : MVSB_Client.h                                              **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef MV_MVSB_CLIENT_H
#define MV_MVSB_CLIENT_H

namespace RMAP
{
   namespace SVC_SB
   {
      class SUBSCRIPTION 
      {
      public:
         enum eFLAG
         {
            SUBSCRIBING             = 0x01,
            SUBSCRIBED              = 0x02,
            UNSUBSCRIBING           = 0x04,
            RESET                   = 0x08,
            DIRTY                   = 0x40,
            DELETED                 = 0x80
         };

      public:
         SUBSCRIPTION (CLIENT* pClient);
         ~SUBSCRIPTION ();

         bool Add    (uint16_t wClass, uint64_t twObjectIx);
         bool Remove (uint16_t wClass, uint64_t twObjectIx);
         bool Reset  (uint16_t wClass, uint64_t twObjectIx);

         void Disconnected (bool bVoluntary, bool bDisconnected);
         void Subscribe_Aux ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class NET
      {
      public:
         enum eSTATE
         {
            NOTCONNECTED            = 0,
            CONNECTING              = 1,
            CONNECTED               = 2,
            CLOSED                  = 3
         };

         enum eRESULT
         {
            DISCONNECT              = 0xE000,
            SOCKETCLOSE             = 0xE001,
            SOCKETERROR             = 0xE002,
            TIMEOUT                 = 0xE003,
            INVALIDPACKET_HEADER    = 0xE004,
            INVALIDPACKET_REQUEST   = 0xE005,
            INVALIDPACKET_RESPONSE  = 0xE006,
            INVALIDPACKET_DATA      = 0xE007
         };

      public:
         class INET
         {
         public:
            virtual void onConnected () = 0;
            virtual void onDisconnected () = 0;
            virtual uint32_t onRecv_Request (uint64_t twPacketIx, uint32_t dwAction, int wSize, BYTESTREAM* ByteStream) = 0;
         };

         class ICONTROL
         {
         public:
            ICONTROL () {};
            virtual ~ICONTROL () {};

            virtual void SocketConnect_Complete    (int pVD) = 0;
            virtual void SocketDisconnect_Complete (int pVD) = 0;
         };

      public:
         NET (CLIENT* pClient, INET* pINet);
         ~NET ();

         std::string const& sEndPoint () const&;

         bool        Connect (bool bSecure, std::string sHost, int wPort, ICONTROL* pIControl, bool bVoluntary, int nTimeout);
         bool        Disconnect (ICONTROL* pIControl, bool bVoluntary, bool bDisconnected, int nTimeout);
         bool        Send_Request (RMAP::CORE::CLIENT::IACTION* pIAction, int nTimeout);
         bool        Send_Response (RMAP::CORE::CLIENT::IACTION* pIAction, uint64_t twPacketIx);

      private:
         class Impl;
         class connection_metadata;
         class websocket_endpoint;

         Impl* m_pImpl;
      };

      class CLIENT : public RMAP::CORE::CLIENT, public NET::INET
      {
      public:
         enum eSTATE
         {
            SOCKETDISCONNECTED = 0,
            SOCKETCONNECTING = 1,
            SYSTEMDISCONNECTED = 2,
            SYSTEMCONNECTING = 3,
            LOGGEDOUT = 4,
            LOGGING = 5,
            LOGGEDIN = 6,
         };

         enum ePROGRESS
         {
            SOCKETCONNECT_ATTEMPT = 0,
            SOCKETCONNECT_RESULT = 1,
            SOCKETDISCONNECT_ATTEMPT = 2,
            SOCKETDISCONNECT_RESULT = 3,
            SYSTEMCONNECT_ATTEMPT = 4,
            SYSTEMCONNECT_RESULT = 5,
            SYSTEMDISCONNECT_ATTEMPT = 6,
            SYSTEMDISCONNECT_RESULT = 7,
            LOGIN_ATTEMPT = 8,
            LOGIN_RESULT = 9,
            LOGOUT_ATTEMPT = 10,
            LOGOUT_RESULT = 11,
         };

      public:
         class IRECV
         {
         public:
            virtual bool onRecv_Request (IACTION* pAction, int wSize, BYTESTREAM* ByteStream) = 0;
         };

      public:
         class IREFERENCE : public RMAP::CORE::CLIENT::IREFERENCE
         {
         public:
            IREFERENCE (uint64_t twClientIx);
            virtual ~IREFERENCE ();

            RMAP::CORE::CLIENT* Create (RMAP::CORE::SERVICE* pService) override;
         };

      public:
         class ACTION : public RMAP::CORE::CLIENT::ACTION
         {
         public:
            ACTION (uint32_t dwAction, std::string sIn, std::string sOut, bool bResponse = true, bool bSend = true);
            ACTION ();
            ~ACTION ();

            uint32_t                dwAction;
            const MAP*              pIn;
            const MAP*              pOut;
            bool                    bResponse;
            bool                    bSend;

         private:
         };

      public:
         class IACTION : public RMAP::CORE::CLIENT::IACTION
         {
         public:
            IACTION (CLIENT* pClient, const ACTION* pAction);
            virtual ~IACTION ();

            uint32_t       GetAction ();
            uint32_t       GetResult ();
            void           SetResult (uint32_t dwResult);

            ordered_json&  GetResponse ();
            void           WriteRequest (BYTESTREAM& ByteStream, int wOffset_Base);
            void           WriteResponse (BYTESTREAM& ByteStream, int wOffset_Base);

            bool           Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam) override;
            ordered_json&  GetRequest () override;
            bool           IsSuccess () override;

            void ReadRequest (BYTESTREAM* pByteStream);
            void ReadResponse (BYTESTREAM* pByteStream, uint32_t dwResult);

            void Response ();

            bool IsRequest ();
            bool IsResponse ();

            uint16_t RequestSize ();
            uint16_t ResponseSize ();

            double GetSendDuration ();

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         CLIENT (IREFERENCE* pReference, RMAP::CORE::SERVICE* pService);
         virtual ~CLIENT ();

         // ===== Static Methods =====================================================================================================

         static RMAP::CORE::CLIENT::IREFERENCE* Reference (uint64_t twClientIx);

         // ===== Accessors ==================================================================================================

         std::string const&   sEndPoint () const&;
         bool                 bNetConnected ();
         bool                 bSystemConnected ();
         bool                 bLoggedIn ();
         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();

         RMAP::CORE::MEM::MEM* pMem ();
         NET*            pNet ();

         // ===== Private Methods ====================================================================================================

         void Progress (RMAP::CORE::PROGRESS* pProgress);

         void Tick (int uCode, TIME tmServer);

         // ===== Public Methods =====================================================================================================

         void Recv_Register   (const ACTION* pAction, IRECV* pIRecv);
         void Recv_Unregister (const ACTION* pAction);

         RMAP::CORE::SOURCE* SourceGet ();

         // ==========================================================================================================================

         RMAP::CORE::CLIENT::IACTION* Request (const RMAP::CORE::CLIENT::ACTION* pAction) override;
         bool     IsDisconnected () override;
         bool     IsConnected () override;
         bool     IsLoggedOut () override;
         bool     IsLoggedIn () override;

         // ==========================================================================================================================

         RMAP::CORE::MODEL* Time_Open ();
         RMAP::CORE::MODEL* Time_Close (RMAP::CORE::MODEL* pTime);

         // ==========================================================================================================================

         bool ClearError ();
         bool SetDevice (std::string acToken64U_Device);

         bool SocketConnect ();
         bool SocketReconnect ();
         bool SocketDisconnect ();
         bool SocketDisconnected (bool bVoluntary);
         bool SystemConnect ();
         bool SystemReconnect ();
         bool SystemDisconnect ();
         bool Login (void* pParams);
         bool Logout (void* pParams);

         // ==========================================================================================================================

         bool Object_Subscribe   (uint16_t wClass, uint64_t twObjectIx);
         bool Object_Unsubscribe (uint16_t wClass, uint64_t twObjectIx);
         bool Object_Reset       (uint16_t wClass, uint64_t twObjectIx);

         // ==========================================================================================================================

         TIME Time_Current ();
         TIME Time_Server ();

         // ===== INET Methods ====================================================================================================

         void onConnected () override;
         void onDisconnected () override;
         uint32_t onRecv_Request (uint64_t twPacketIx, uint32_t dwAction, int wSize, BYTESTREAM* ByteStream) override;

         bool SafeKill () override;

         TIME tmCurrent;

         void SetAttemptState (int nReadyState);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class CONTROL
      {
      public:
         enum eCONTROL
         {
            SOCKETCONNECT                 = 0x0001,
            SOCKETDISCONNECT              = 0x0002,
            SOCKETDISCONNECTED            = 0x0004,
            SOCKETDISCONNECTED_VOLUNTARY  = 0x0008,
            SYSTEMCONNECT                 = 0x0010,
            SYSTEMDISCONNECT              = 0x0020,
            LOGIN                         = 0x0100,
            LOGOUT                        = 0x0200
         };

         enum eRESPONSE
         {
            kSYSTEMCONNECT_RESPONSE       = 1,
            kSYSTEMDISCONNECT_RESPONSE    = 2,
            kLOGIN_RESPONSE               = 3,
            kLOGOUT_RESPONSE              = 4
         };

      public:
         class SB_PROGRESS : public RMAP::CORE::PROGRESS
         {
         public:
            int                                 nProgress;
            bool                                bResult;
            bool                                bVoluntary;
            bool                                bDisconnected;
            RMAP::CORE::SOURCE_SESSION::LOGIN*        pLogin;

            std::string                         acToken64U_Device;
            uint32_t                            dwResult;
         };

      public:
         CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription);
         ~CONTROL ();

         // ===== Accessors ====================================================================================================

//         std::string const& sEndPoint () const&;
         bool bNetConnected ();
         bool bSystemConnected ();
         bool bLoggedIn ();
         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();

         // ===== Methods ====================================================================================================

         bool SafeKill ();
         bool ClearError ();
         bool SetDevice (std::string acToken64U_Device);
         bool SocketConnect ();
         bool SocketReconnect ();
         bool SocketDisconnect ();
         bool SocketDisconnected (bool bVoluntary);

         bool SystemConnect ();
         bool SystemReconnect ();
         bool SystemDisconnect ();

         bool Login (RMAP::CORE::SOURCE* pSource, void* pvParams);
         bool Logout (RMAP::CORE::SOURCE* pSource, void* pvParams);

         void SetAttemptState (int nReadyState);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class REFRESH : public RMAP::CORE::MEM::IMEM, public CLIENT::IRECV
      {
      public:
         class SBA_SRE : public RMAP::CORE::MEM::CHANGE
         {
         public:
            uint64_t                            twObjectIx;
            uint16_t                            wClass;
            uint64_t                            twEventIz;
            uint16_t                            wEventTypeIx;
            uint16_t                            wSize;
            TIMEX                               txStamp;

            std::vector<uint8_t>                pData;
         };

      public:
         REFRESH (CLIENT* pClient);
         ~REFRESH ();

         bool onRecv_Request (RMAP::CORE::CLIENT::IACTION* pAction, int wSize, BYTESTREAM* ByteStream) override;

         bool onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam) override;
         bool onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam) override;

      private:
         int Event_Refresh_Object (RMAP::CORE::SOURCE* pObject, BYTESTREAM* ByteStream);
         int Event_Refresh (TIME tmBase, BYTESTREAM* ByteStream);

         BYTESTREAM* m_pByteStream;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class RECOVER : public RMAP::CORE::MEM::IMEM, public CLIENT::IRECV
      {
      public:
         RECOVER (CLIENT* pClient);
         ~RECOVER ();

         int  Object_Recover (uint16_t wClass, BYTESTREAM* ByteStream, int wSize);

         bool onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam) override;
         bool onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam) override;

         bool onRecv_Request (RMAP::CORE::CLIENT::IACTION* pAction, int wSize, BYTESTREAM* ByteStream) override;

      private:
         class Impl;
         Impl* m_pImpl;
      };
   }
}
#endif //MV_MVSB_CLIENT_H
