/*******************************************************************************************************************************
**                                                                                                                            **
**                                        RMAP_Svc_SocketIO : RMAP_Svc_SocketIO.h                                             **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_SVC_IO_H
#define RMAP_SVC_IO_H

namespace RMAP
{
   namespace SVC_SOCKETIO
   {
      class CLIENT;
      class SUBSCRIPTION
      {
      public:
         SUBSCRIPTION (CLIENT* pClient);
         ~SUBSCRIPTION ();

         bool Add    (int wClass, uint64_t twObjectIx);
         bool Remove (int wClass, uint64_t twObjectIx);
         bool Reset  (int wClass, uint64_t twObjectIx);

         void Disconnected (bool bVoluntary, bool bDisconnected);
         void Subscribe_Aux ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class SERVICE : public RMAP::CORE::SERVICE
      {
      public:
         class NETSETTINGS
         {
         public:
            std::string                   sName;
            bool                          bSecure;
            std::string                   sHost;
            int                           wPort;
            std::string                   sSession;
         };

         enum eCLIENT
         {
            CONNECTED = 0,
            DISCONNECTED = 1,
         };

         typedef struct
         {
            int                           nConnected;
            CLIENT*                       pClient;
         }
         NOTIFYPARAM;

      public:
         class FACTORY : public RMAP::CORE::SERVICE::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            RMAP::CORE::SERVICE::IREFERENCE* Reference (std::string sConnect) override;
         };

      public:
         class IREFERENCE : public RMAP::CORE::SERVICE::IREFERENCE
         {
         public:
            IREFERENCE (std::string sID, std::string sConnect);
            virtual ~IREFERENCE ();

            std::string    Key () override;
            RMAP::CORE::SERVICE* Create (RMAP::CORE::NAMESPACE* pNamespace) override;

            NETSETTINGS                NetSettings;
         };

      public:
         static FACTORY* factory ();

         SERVICE (IREFERENCE* pReference, RMAP::CORE::NAMESPACE* pNamespace);
         ~SERVICE ();

         // ===== Public Properties ==================================================================================================

         NETSETTINGS* pNetSettings ();

         // ===== Public Methods =====================================================================================================

         RMAP::CORE::CLIENT* Client_Open (uint64_t twClientIx) override;
         std::string GetSessionString () override;

         bool Connected (CLIENT* pClient);
         void Disconnected (CLIENT* pClient);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 Client                                                                     **
      *******************************************************************************************************************************/

      class CLIENT;
      class NET
      {
      public:
         enum eSTATE
         {
            NOTCONNECTED      = 0,
            CONNECTING        = 1,
            CONNECTED         = 2,
            CLOSED            = 3
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
            virtual bool onRecv_Request (std::string const& sEventName, ordered_json& jData) = 0;
         };

         class ICONTROL
         {
         public:
            ICONTROL () {};
            virtual ~ICONTROL () {};

            virtual void SocketConnect_Complete (bool bConnected, bool bVoluntary) = 0;
            virtual void SocketDisconnect_Complete (bool bVoluntary) = 0;
         };

      public:
         NET (CLIENT* pClient, INET* pINet);
         ~NET ();

         bool Connect (const std::string& sEndPoint, ICONTROL* pIControl, bool bVoluntary);
         bool Disconnect (ICONTROL* pIControl, bool bVoluntary, bool bDisconnected);
         bool Send_Request (RMAP::CORE::CLIENT::IACTION* pIAction);

      private:
         class Impl;

         Impl* m_pImpl;
      };

      typedef void (*fnActionConvert)(ordered_json& jRequest_Out, const ordered_json& jRequest_In);

      class CLIENT : public RMAP::CORE::MEM::IMEM, public RMAP::CORE::CLIENT, public NET::INET
      {
      public:
         enum eSTATE
         {
            SOCKETDISCONNECTED   = 0,
            SOCKETCONNECTING     = 1,
            LOGGEDOUT            = 2,
            LOGGING              = 3,
            LOGGEDIN             = 4
         };

         enum ePROGRESS
         {
            SOCKETCONNECT_ATTEMPT      = 0,
            SOCKETCONNECT_RESULT       = 1,
            SOCKETDISCONNECT_ATTEMPT   = 2,
            SOCKETDISCONNECT_RESULT    = 3,
            LOGIN_ATTEMPT              = 4,
            LOGIN_RESULT               = 5,
            LOGOUT_ATTEMPT             = 6,
            LOGOUT_RESULT              = 7
         };
   
      public:
         struct
         {
            std::string                    sSessionToken;
         };

      public:
         class IRECV
         {
         public:
            virtual bool onRecv_Request (std::string sAction, ordered_json& jData) = 0;
         };

      public:
         class IREFERENCE : public RMAP::CORE::CLIENT::IREFERENCE
         {
         public:
            IREFERENCE (uint64_t twClientIx);
            virtual ~IREFERENCE ();

            RMAP::CORE::CLIENT* Create (RMAP::CORE::SERVICE* pService) override;

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         class ACTION : public RMAP::CORE::CLIENT::ACTION
         {
         public:
            ACTION (std::string sAction, std::string sRequest, fnActionConvert fnConvert);
            virtual ~ACTION ();

            std::string&  GetAction () const;
            ordered_json& GetRequest () const;
            fnActionConvert GetConvert () const;

         private:
            class Impl;
            Impl* m_pImpl;
         };

         class IACTION : public RMAP::CORE::CLIENT::IACTION
         {
         public:
            IACTION (CLIENT* pClient, const ACTION* pAction);
            virtual ~IACTION ();

            bool Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam);

            std::string GetAction ();
            ordered_json& GetResponse ();
            std::string   GetRequestEx ();

            bool          IsSuccess ()  override;
            ordered_json& GetRequest () override;

            void SetResponse (ordered_json& jResponse);
            void Response    ();

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         CLIENT (IREFERENCE* pReference, RMAP::CORE::SERVICE* pService);
         virtual ~CLIENT ();

         static RMAP::CORE::CLIENT::IREFERENCE* Reference (uint64_t twClientIx);

         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();
         bool   bLoggedIn ();

         void   Progress (RMAP::CORE::PROGRESS* pProgress);

         bool bNetConnected ();

         std::string& sEndPoint ();

         // =======================================================================================================

         RMAP::CORE::CLIENT::IACTION* Request (const RMAP::CORE::CLIENT::ACTION* pAction) override;
         bool     IsDisconnected () override;
         bool     IsConnected () override;
         bool     IsLoggedOut () override;
         bool     IsLoggedIn () override;
         bool     SafeKill () override;

         bool SocketConnect ();
         bool SocketReconnect ();
         bool SocketDisconnect (bool bVoluntary);

         bool Login (void* pParams);
         bool Logout (void* pParams);
         uint32_t Object_Recover (ordered_json& jData);

         RMAP::CORE::MEM::MEM* pMem ();
         NET* pNet ();

         void Recv_Register (std::string sAction, IRECV* pRecv);
         void Recv_Unregister (std::string sAction);

         bool Object_Subscribe (int wClass, uint64_t twObjectIx);
         bool Object_Unsubscribe (int wClass, uint64_t twObjectIx);

         // ===== INET Methods ====================================================================================================

         void onConnected () override;
         void onDisconnected () override;
         bool onRecv_Request (std::string const& sEventName, ordered_json& jData) override;

         // ===== IMEM Methods ====================================================================================================

         bool onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam) override;
         bool onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam) override;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class REFRESH : public RMAP::CORE::MEM::IMEM, public CLIENT::IRECV
      {
      public:
         class IOCHANGE : public RMAP::CORE::MEM::CHANGE
         {
         public:
            ordered_json          jChange;
         };

      public:
         REFRESH (CLIENT* pClient);
         ~REFRESH ();

         bool onRecv_Request (std::string sAction, ordered_json& jData) override;

         bool onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam) override;
         bool onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam) override;

      private:
         bool Event_Refresh (ordered_json& jResponse);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class RECOVER : public CLIENT::IRECV
      {
      public:
         RECOVER (CLIENT* pClient);
         ~RECOVER ();

         void Object_Recover (ordered_json& jResponse);

         bool onRecv_Request (std::string sAction, ordered_json& jData) override;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 IO_OBJECT                                                                **
      *******************************************************************************************************************************/

      class IO_OBJECT : public RMAP::CORE::MEM::SOURCE
      {
      public:
         class OBJECTHEAD : public RMAP::CORE::MEM::OBJECTHEAD
         {
         public:
            OBJECTHEAD ();
            OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz);

            OBJECTHEAD & operator=( OBJECTHEAD const  & rhs   ) &;         
            OBJECTHEAD & operator=( OBJECTHEAD       && rhs   ) & noexcept;
            OBJECTHEAD            ( OBJECTHEAD const  & other );           
            OBJECTHEAD            ( OBJECTHEAD       && other )   noexcept;
            virtual ~OBJECTHEAD ();

            uint64_t                      twEventIz;
         };

      public:
         class FACTORY : public RMAP::CORE::MEM::SOURCE::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction, bool bIndependent);
            virtual ~FACTORY ();
         };

      public:
         IO_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~IO_OBJECT ();

         // ===== Public Properties ==================================================================================================

         void twEventIz (uint64_t twEventIz);
         uint64_t twEventIz ();

         uint64_t twObjectIx ();
         uint64_t twParentIx ();

         void SetData (ordered_json& jData);
         ordered_json& GetData ();

         virtual void Read (ordered_json& jSrc, RMAP::CORE::MODEL* pModel) = 0;

         // ===== Source Methods =====================================================================================================

         void Partial () override;
         void Full () override;
         void Recovering () override;
         void Recovered () override;
         void Inserted (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Deleting (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Updating (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild) override;
         void Updated  (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild) override;
         void Changing (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Changed  (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;

         bool Attach () override;
         bool Detach () override;

      public:
         void Map_Read (RMAP::CORE::MEM::MODEL* pModel);
         void Map_Write (void* pvData, uint16_t wFlags, bool bDiscard);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class IO_SESSION : public RMAP::CORE::SOURCE_SESSION
      {
      public:
         class FACTORY : public RMAP::CORE::SOURCE_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction);
            virtual ~FACTORY ();
         };

      public:
         IO_SESSION (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~IO_SESSION ();

         void initialize (RMAP::CORE::MODEL_SESSION* pModel);

         void Progress (RMAP::CORE::PROGRESS* pProgress) override;

         // ===== Client Methods =====================================================================================================

         void LoggedOut ();

         // ===== Model Methods ======================================================================================================

         bool Attach () override;
         bool Detach () override;

         // --------------------------------------------------------------------------------------------------------------------------

         virtual bool Attempt (int nReadyState) = 0;

         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin () override;
         bool         Connect () override;
         bool         Disconnect (bool bVoluntary) override;

         void Reconnect ();
         void Reconnect (int nReconnect);

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
         class PROGRESS : public RMAP::CORE::PROGRESS
         {
         public:
            int                                 nProgress;
            bool                                bResult;
            bool                                bVoluntary;
            bool                                bDisconnected;
            RMAP::CORE::SOURCE_SESSION::LOGIN*         pLogin;

            std::string                         acToken64U_Device;
            uint32_t                            dwResult;
         };

      public:
         CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription);
         ~CONTROL ();

         bool bLoggedIn ();
         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();

         std::string& sEndPoint ();

         bool SafeKill ();
         bool ClearError ();

         bool Login (RMAP::CORE::SOURCE* pSource, void* pvParams);
         bool Logout (RMAP::CORE::SOURCE* pSource, void* pvParams);

         bool SocketConnect ();
         bool SocketReconnect ();
         bool SocketDisconnect ();
         bool SocketDisconnected (bool bVoluntary);

         bool bNetConnected ();

      private:
         bool Logout_Request (bool bVoluntary, bool bDisconnected);
         void Logout_Response (CLIENT::IACTION* pIAction, intptr_t pVD);
         void Logout_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class IO_SESSION_NULL : public IO_SESSION
      {
      public:
         class FACTORY : public IO_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction);
            virtual ~FACTORY ();

            RMAP::CORE::SOURCE* Create (RMAP::CORE::CLIENT* pClient) override;
         };

      public:
         static void init ();
         static FACTORY* factory ();

         IO_SESSION_NULL (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~IO_SESSION_NULL ();

         static std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> aAction;

      public:
         RMAP::CORE::SOURCE_SESSION::LOGIN* Login_Create () override;
         RMAP::CORE::SOURCE_SESSION::LOGIN* Login_Destroy (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         void                        Progress (RMAP::CORE::PROGRESS* pProgress) override;
         RMAP::CORE::CLIENT::IACTION*      Login_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                        Login_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary) override;
         RMAP::CORE::CLIENT::IACTION*      Logout_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                        Logout_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected) override;

      public:
         bool Attempt (int nReadyState) override;

         RMAP::CORE::ISOURCE_SESSION* GetSessionInterface ();
      };

      /*******************************************************************************************************************************
      **                                                     Startup/Shutdown                                                       **
      *******************************************************************************************************************************/

      void Install ();
      void Unstall ();
   }
}
#endif //RMAP_SVC_IO_H
