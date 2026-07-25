/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      MVRest_cpp : MVRest.h                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/
#if 0
#ifndef RMAP_SVC_REST_H
#define RMAP_SVC_REST_H

namespace RMAP
{
   namespace SVC_Rest
   {
      class SERVICE : public RMAP::CORE::SERVICE
      {
      public:
         class NETSETTINGS
         {
         public:
            bool                          bSecure;
            std::string                   sHost;
            int                           wPort;
            std::string                   sSession;
         };

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

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 Client                                                                     **
      *******************************************************************************************************************************/

      class CLIENT : public RMAP::CORE::MEM::IMEM, public RMAP::CORE::CLIENT
      {
      public:
         enum eSTATE
         {
            LOGGEDOUT   = 0,
            LOGGING     = 1,
            LOGGEDIN    = 2,
         };

         enum ePROGRESS
         {
            LOGIN_ATTEMPT  = 0,
            LOGIN_RESULT   = 1,
            LOGOUT_ATTEMPT = 2,
            LOGOUT_RESULT  = 3,
         };

         friend class CONTROL;

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
         class ICODEC;
         class ACTION : public RMAP::CORE::CLIENT::ACTION
         {
         public:
            enum eCODEC
            {
               CODEC_POSTJSON,
            };

         public:
            ACTION (std::string sAction, std::string sRequest, eCODEC kCodec = CODEC_POSTJSON);
            virtual ~ACTION ();

            std::string& GetAction () const;
            std::string& GetRequest () const;
            CLIENT::ACTION::eCODEC GetCodec ();

         private:
            class Impl;
            Impl* m_pImpl;
         };

         class IACTION : public RMAP::CORE::CLIENT::IACTION
         {
         public:
            enum eMETHOD
            {
               POST,
               GET
            };

            typedef struct _PAYLOAD
            {
               std::map<std::string, std::string>  aHeaders;
               eMETHOD                             kMethod;
               std::string                         sEndPoint;
               std::string                         sBody;
            }
            PAYLOAD;

         public:
            IACTION (CLIENT* pClient, const ACTION* pAction);
            virtual ~IACTION ();

            ICODEC* GetCodec ();

            void Response (bool bSuccess);
            bool Send (RMAP::CORE::IRESPONSE* pResponse, int nType, intptr_t pParam);

            ordered_json& GetRequest () override;
            bool          IsSuccess ()  override;

            PAYLOAD              Payload;

         private:
            RMAP::CORE::IRESPONSE* m_pResponse;
            intptr_t             m_pParam;
            bool                 m_bSuccess;
            ICODEC*              m_pICodec;
            int                  m_nType;
         };

         class ICODEC
         {
         public:
            ICODEC (CLIENT::IACTION* pIAction, std::string sRequest);
            virtual ~ICODEC ();

            virtual void Encode (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, const std::string& sAction, std::string& sEndPoint) = 0;
            virtual void Decode (std::string sResult)  = 0;
            virtual void Error  (const char* pszError) = 0;

            virtual ordered_json& GetRequest () = 0;

         protected:
            IACTION* m_pIAction;
         };

      public:
         CLIENT (IREFERENCE* pReference, RMAP::CORE::SERVICE* pService);
         virtual ~CLIENT ();

         static RMAP::CORE::CLIENT::IREFERENCE* Reference (uint64_t twClientIx);

         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();
         bool   bLoggedIn ();
         std::string& sEndPoint ();

         void   Progress (RMAP::CORE::PROGRESS* pProgress);

         // =======================================================================================================

         RMAP::CORE::CLIENT::IACTION* Request (const RMAP::CORE::CLIENT::ACTION* pAction) override;
         bool     IsDisconnected () override;
         bool     IsConnected () override;
         bool     IsLoggedOut () override;
         bool     IsLoggedIn () override;
         bool     SafeKill () override;

         bool Login (void* pParams);
         bool Logout (void* pParams);
         uint32_t Object_Recover (void* pvData);

         bool onUpdate (RMAP::CORE::MEM::SOURCE* pObject, bool bDiscard, void* pParam) override;
         bool onChange (RMAP::CORE::MEM::SOURCE* pParent, RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, void* pParam) override;

         void QueueAction (IACTION* pIAction);

         ICODEC* Codec_Create (CLIENT::IACTION* pIAction, std::string sRequest);
         void    Codec_Destroy (CLIENT::ICODEC* pICodec);

         void IAction_Add (IACTION* pIAction, RMAP::CORE::SOURCE* pSource);
         void IAction_Remove (IACTION* pIAction);
         void IAction_AbortAll (RMAP::CORE::SOURCE* pSource);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class CONTROL
      {
      public:
         enum eCONTROL
         {
            SOCKETCONNECT = 0x0001,
            SOCKETDISCONNECT = 0x0002,
            SOCKETDISCONNECTED = 0x0004,
            SOCKETDISCONNECTED_VOLUNTARY = 0x0008,
            SYSTEMCONNECT = 0x0010,
            SYSTEMDISCONNECT = 0x0020,
            LOGIN = 0x0100,
            LOGOUT = 0x0200
         };

         enum eRESPONSE
         {
            kSYSTEMCONNECT_RESPONSE = 1,
            kSYSTEMDISCONNECT_RESPONSE = 2,
            kLOGIN_RESPONSE = 3,
            kLOGOUT_RESPONSE = 4
         };

      public:
         class PROGRESS : public RMAP::CORE::PROGRESS
         {
         public:
            int                           nProgress;
            bool                          bResult;
            bool                          bVoluntary;
            bool                          bDisconnected;
            RMAP::CORE::SOURCE_SESSION::LOGIN*   pLogin;

            std::string                   acToken64U_Device;
            uint32_t                      dwResult;
         };

      public:
         CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings);
         ~CONTROL ();

         bool bLoggedIn ();
         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin ();
         std::string& sEndPoint ();

         bool SafeKill ();
         bool ClearError ();

         bool Login (RMAP::CORE::SOURCE* pSource, void* pvParams);
         bool Logout (RMAP::CORE::SOURCE* pSource, void* pvParams);

         void QueueAction (CLIENT::IACTION* pIAction);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 REST_OBJECT                                                                **
      *******************************************************************************************************************************/

      class REST_OBJECT : public RMAP::CORE::MEM::SOURCE
      {
      public:
         class OBJECTHEAD : public RMAP::CORE::MEM::OBJECTHEAD
         {
         public:
            OBJECTHEAD ();
            OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz);

            OBJECTHEAD & operator=( OBJECTHEAD const  & rhs   ) &;                  // Copy Assignment Operator
            OBJECTHEAD & operator=( OBJECTHEAD       && rhs   ) & noexcept;         // Move Assignment Operator
            OBJECTHEAD            ( OBJECTHEAD const  & other );                    // Copy constructor
            OBJECTHEAD            ( OBJECTHEAD       && other )   noexcept;         // Move constructor
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
         REST_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~REST_OBJECT ();

         // ===== Public Properties ==================================================================================================

         void twEventIz (uint64_t twEventIz);
         uint64_t twEventIz ();

         uint64_t twObjectIx ();
         uint64_t twParentIx ();

         void ResetData ();

//         std::vector<uint8_t>& GetData ();

         virtual void Read (ordered_json& jSrc, RMAP::CORE::MODEL* pModel) = 0;

         // ===== Source Methods =====================================================================================================

         RMAP::CORE::CLIENT::IACTION* Request (std::string sAction) override;
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

      class REST_SESSION : public RMAP::CORE::SOURCE_SESSION
      {
      public:
         class FACTORY : public RMAP::CORE::SOURCE_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction);
            virtual ~FACTORY ();
         };

      public:
         REST_SESSION (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~REST_SESSION ();

         void initialize (RMAP::CORE::MODEL_SESSION* pModel);

         void Progress (RMAP::CORE::PROGRESS* pProgress) override;

         // ===== Client Methods =====================================================================================================

         void LoggedOut ();

         // ===== Model Methods ======================================================================================================

         bool Attach () override;
         bool Detach () override;

         // --------------------------------------------------------------------------------------------------------------------------

         virtual bool Attempt (int nReadyState) = 0;
         virtual CLIENT::ICODEC* Codec_Create (CLIENT::IACTION* pIAction, std::string sRequest) = 0;

         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin () override;
         bool         Connect () override;
         bool         Disconnect (bool bVoluntary) override;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class REST_SESSION_NULL : public REST_SESSION
      {
      public:
         class FACTORY : public REST_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*>& apAction);
            virtual ~FACTORY ();

            RMAP::CORE::SOURCE* Create (RMAP::CORE::CLIENT* pClient) override;
         };

      public:
         static void init ();
         static FACTORY* factory ();

         REST_SESSION_NULL (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~REST_SESSION_NULL ();

         static std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> aAction;

      public:
         RMAP::CORE::SOURCE_SESSION::LOGIN*   Login_Create () override;
         RMAP::CORE::SOURCE_SESSION::LOGIN*   Login_Destroy (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         void                          Progress (RMAP::CORE::PROGRESS* pProgress) override;
         RMAP::CORE::CLIENT::IACTION*        Login_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                          Login_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary) override;
         RMAP::CORE::CLIENT::IACTION*        Logout_Request (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                          Logout_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected) override;

      public:
         bool Attempt (int nReadyState) override;
         CLIENT::ICODEC* Codec_Create (CLIENT::IACTION* pIAction, std::string sRequest) override;

         RMAP::CORE::ISOURCE_SESSION* GetSessionInterface ();
      };
   }
}
#endif //RMAP_SVC_REST_H
#endif