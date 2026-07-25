/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP_Base.h                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_CORE_BASE_H
#define RMAP_CORE_BASE_H

namespace RMAP
{
   namespace CORE
   {
      template<class T, class P>
      class IREFERENCE
      {
      public:
         IREFERENCE (std::string sID)
         {
            m_psID = new char[sID.size () + 1];
            std::memcpy (m_psID, sID.c_str (), sID.size ());
            m_psID[sID.size ()] = '\0';
         }

         virtual ~IREFERENCE ()
         {
            delete [] m_psID;
         }

         char* UniqueId () const
         {
            return m_psID;
         }

         virtual std::string Key () = 0;
         virtual T Create (P pParam) = 0;

      private:
         char* m_psID;
      };

      class FACTORY
      {
      public:
         FACTORY () {}
         virtual ~FACTORY () {}
      };
      class NAMESPACE;

      class SERVICE;
      class MODEL;
      class SOURCE;

      /*******************************************************************************************************************************
      **                                                 NOTIFICATION                                                               **
      *******************************************************************************************************************************/

      class NOTIFICATION;
      class INOTICE
      {
      public:
         INOTICE (NOTIFICATION* pNotification, std::string sNotification, void* pData);
         virtual ~INOTICE ();

         NOTIFICATION*  pCreator;
         std::string    sNotification;
         void*          pData;

         NOTIFICATION*  pEmitter;
         bool           bPropagate;
      };

      class NOTIFICATION
      {
      private:
         class Impl;
         Impl* m_pImpl;

      public:
         class LISTENER
         {
         public:
            LISTENER (NOTIFICATION* pSender, NOTIFICATION* pReceiver, bool bPropagate, bool bNotifyOnReady = false);
            virtual ~LISTENER ();

            NOTIFICATION* pThis;
            bool          bPropagate;

            bool          bNotifyOnReady;
            bool          bInit;
         };

      public:
         NOTIFICATION ();
         virtual ~NOTIFICATION ();

         int  ReadyState ();
         int  ReadyState (int nReadyState);

         void Emit (std::string sMessage, void* pParam);

         virtual int  Attach (NOTIFICATION* pNotice, bool bPropagate = false, bool bNotifyOnReady = false);
         virtual int  Detach (NOTIFICATION* pNotice);
         virtual void Notify (INOTICE* pNotice);
         virtual bool IsReady ();

      private:
         void Send (LISTENER* pListener, INOTICE* pNotice);
         void Enum (INOTICE* pNotice);
         void Init (NOTIFICATION* pThis);
      };

      /*******************************************************************************************************************************
      **                                                 Client                                                                     **
      *******************************************************************************************************************************/

      namespace MEM
      {
         class MEM;
         class MODEL;
      }
      class MODEL_SESSION;
      class IRESPONSE;
      class CLIENT : public NOTIFICATION
      {
      public:
         class IREFERENCE : public RMAP::CORE::IREFERENCE<CLIENT*, SERVICE*>
         {
         public:
            IREFERENCE (std::string sID, uint64_t twClientIx);
            virtual ~IREFERENCE ();

            std::string Key () override;
            uint64_t twClientIx ();

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         class ACTION
         {
         public:
            ACTION ();
            virtual ~ACTION ();
         };

         class IACTION
         {
         public:
            enum eSTATUS_CODE
            {
               kSTATUS_CODE_OK,

               kSTATUS_CODE_ABORTED
            };

         public:
            IACTION (CLIENT* pClient, const ACTION* pAction);
            virtual ~IACTION ();

            virtual bool            Send (IRESPONSE* pResponse, int nType, intptr_t pParam) = 0;
            virtual ordered_json&   GetRequest ()                                           = 0;
            virtual bool            IsSuccess ()                                            = 0;

            void         Abort ();
            eSTATUS_CODE Status ();

         protected:
            CLIENT*        m_pClient;
            const ACTION*  m_pAction;

         private:
            class CImpl;
            CImpl* m_pCImpl;
         };

      public:
         CLIENT (IREFERENCE* pReference, SERVICE* pService);
         virtual ~CLIENT ();

         // ===== Public Properties ==================================================================================================

         NAMESPACE*  pNamespace ();
         std::string sNamespace () const;
         std::string sID () const;
         std::string sKey ()const;
         SERVICE*    pService ();
         uint64_t    twClientIx ();

         // =======================================================================================================

         MODEL_SESSION* Session_Open (bool bAutoConnect);
         MODEL_SESSION* Session_Close (MODEL_SESSION* pSession);
         MEM::MODEL*    Model_Open (std::string sID_Model, std::string sArgs);
         MEM::MODEL*    Model_Close (MEM::MODEL* pModel);

         virtual IACTION* Request (const ACTION* pAction) = 0;
         virtual bool      IsDisconnected () = 0;
         virtual bool      IsConnected () = 0;
         virtual bool      IsLoggedOut () = 0;
         virtual bool      IsLoggedIn () = 0;
         virtual bool      SafeKill () = 0;

      protected:
         MODEL* Model_Open_Aux (std::string sID_Model, std::string sArgs);
         MODEL* Model_Close_Aux (MODEL* pModel);
         SOURCE* Source (int wClass);

         void Lock ();
         void Unlock ();

         MEM::MEM* m_pMem;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 Model                                                                      **
      *******************************************************************************************************************************/

      class MODEL : public NOTIFICATION
      {
      public:
         class FACTORY : public RMAP::CORE::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            // ===== Public Properties ==================================================================================================

            virtual IREFERENCE<MODEL*, SOURCE*>* Reference (std::vector<std::string> asArgs) = 0;

            std::string sID () const;

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         MODEL (IREFERENCE<MODEL*, SOURCE*>* pReference, SOURCE* pSource);
         virtual ~MODEL ();

         // ===== Public Properties ==================================================================================================

         NAMESPACE* pNamespace ();
         std::string sNamespace () const;
         std::string sID () const;
         std::string sKey () const;
         SOURCE* pSource ();

         // ===== Public Methods =====================================================================================================

         std::vector<std::string> Actions ();
         
         virtual CLIENT::IACTION* Request (std::string sAction);

         int  Attach (NOTIFICATION* pNotify, bool bPropagate = false, bool nNotifyOnReady = false);
         int  Detach (NOTIFICATION* pNotify);

         virtual bool IsReady ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 MODEL_SESSION                                                              **
      *******************************************************************************************************************************/

      class MODEL_SESSION : public MODEL
      {
      public:
         class FACTORY : public MODEL::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();
         };

      public:
         class IREFERENCE : public RMAP::CORE::IREFERENCE<MODEL*, SOURCE*>
         {
         public:
            IREFERENCE (std::string sID, bool bAutoConnect);
            virtual ~IREFERENCE ();

            std::string Key () override;

            bool bAutoConnect;
         };

      public:
         MODEL_SESSION (IREFERENCE* pReference, SOURCE* pSource);
         virtual ~MODEL_SESSION ();

         virtual bool      Login (std::string sSession)  = 0;
         virtual bool      Logout ()                     = 0;
         virtual void      LoggedOut ()                  = 0;
         virtual bool      IsLoggedIn ()                 = 0;
         virtual bool      IsLoggedOut ()                = 0;
         virtual uint64_t  twUserIx ()                   = 0;

         uint64_t twClientIx;
         uint32_t dwResult;
      };

      class IRESPONSE
      {
      public:
         virtual void onResponse (CLIENT::IACTION* pIAction, int nType, intptr_t pParam) = 0;
      };

      /*******************************************************************************************************************************
      **                                                 Service                                                                    **
      *******************************************************************************************************************************/

      class SERVICE : public NOTIFICATION
      {
      public:
         class IREFERENCE : public RMAP::CORE::IREFERENCE<SERVICE*, NAMESPACE*>
         {
         public:
            IREFERENCE (std::string sID, std::string sConnect);
            virtual ~IREFERENCE ();

            std::map<std::string, std::string> GetConnectInfo (std::string& sHost, int& wPort, bool& bSecure);

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         class FACTORY : public RMAP::CORE::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            virtual IREFERENCE* Reference (std::string sConnect) = 0;

            std::string sID () const;

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         SERVICE (IREFERENCE* pReference, NAMESPACE* pNamespace);
         virtual ~SERVICE ();

         // ===== Accessors ==================================================================================================

         NAMESPACE* pNamespace ();
         std::string const& sNamespace () const&;
         std::string const& sID () const&;
         std::string const& sKey () const&;

         // ===== Public Methods =====================================================================================================

         typedef bool (*fnClientEnum)(CLIENT* pClient, void* pvParam);

         virtual CLIENT* Client_Open (uint64_t twClientIx) = 0;

         CLIENT* Client_Open (CLIENT::IREFERENCE* pReference_Client);
         CLIENT* Client_Close (CLIENT* pClient);

         int     Client_Length ();
         bool    Client_Exists (uint64_t twClientIx);
         CLIENT* Client_Get (uint64_t twClientIx);
         CLIENT* Client_Index (int nIndex);
         CLIENT* Client_Enum (fnClientEnum fnEnum, void* pParam);
         void    Client_Release ();

         virtual std::string GetSessionString () = 0;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 SOURCE                                                                     **
      *******************************************************************************************************************************/

      class SOURCE
      {
      public:
         class REFERENCE
         {
         public:
            REFERENCE (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const CLIENT::ACTION*> &apAction);
            ~REFERENCE ();

            std::string                                     sID_Service;
            std::string                                     sID_Model;
            int                                             wClass;
            std::map<std::string, const CLIENT::ACTION*>    *papAction;
         };

         class FACTORY : public RMAP::CORE::FACTORY
         {
         public:
            enum TYPE
            {
               SESSION          = 0,
               OBJECT           = 1,
               OTHER            = 2
            };

         public:
            FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const CLIENT::ACTION*> &apAction);
            virtual ~FACTORY ();

            // ===== Public Properties ==================================================================================================

            REFERENCE* pReference ();

            virtual TYPE bType ();

            void ActionAdd (const std::map<std::string, const CLIENT::ACTION*>& apAction);

            // ===== Public Methods =====================================================================================================
            virtual SOURCE* Create (CLIENT* pClient) = 0;

         private:
            REFERENCE*  m_pReference;
         };

      public:
         SOURCE (REFERENCE* pReference, CLIENT* pClient);
         virtual ~SOURCE ();

         void initialize (MODEL* pModel);

         // ===== Public Properties ==================================================================================================

         NAMESPACE*  pNamespace ();
         std::string sNamespace ();
         std::string sID_Service ();
         std::string sID_Model ();
         int         wClass ();
         CLIENT*     pClient ();
         MODEL*      pModel ();

         // ===== Public Methods =====================================================================================================

         std::vector<std::string> Actions ();

         virtual CLIENT::IACTION* Request (std::string sAction);
         virtual bool             Attach ();
         virtual bool             Detach ();
         virtual bool             IsDisconnected ();
         virtual bool             IsConnected ();
         virtual bool             IsLoggedOut ();
         virtual bool             IsLoggedIn ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 SOURCE_SESSION                                                             **
      *******************************************************************************************************************************/

      class PROGRESS 
      {
      public:
         PROGRESS () {}
         virtual ~PROGRESS () {}
      };

      class ISOURCE_SESSION
      {
      public:
         ISOURCE_SESSION () {}
         virtual ~ISOURCE_SESSION () {}
      };

      class SOURCE_SESSION : public SOURCE
      {
      public:
         class LOGIN
         {
         public:
            LOGIN ();
            virtual ~LOGIN ();
         };

      public:
         class FACTORY : public SOURCE::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const CLIENT::ACTION*> &apAction);
            virtual ~FACTORY ();

            SOURCE::FACTORY::TYPE bType () override;
         };

      public:
         SOURCE_SESSION (SOURCE::REFERENCE* pReference, CLIENT* pClient);
         virtual ~SOURCE_SESSION ();

         virtual LOGIN*                         Login_Create    () = 0;
         virtual LOGIN*                         Login_Destroy   (LOGIN* pLogin) = 0;
         virtual void                           Progress        (PROGRESS* pProgress) = 0;
         virtual RMAP::CORE::CLIENT::IACTION*   Login_Request   (void* pParams, LOGIN* pLogin) = 0;
         virtual bool                           Login_Response  (void* pParams, LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary) = 0;
         virtual RMAP::CORE::CLIENT::IACTION*   Logout_Request  (void* pParams, LOGIN* pLogin) = 0;
         virtual bool                           Logout_Response (void* pParams, LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected) = 0;

         virtual LOGIN* pLogin () = 0;
         virtual bool   Connect ()                       = 0;
         virtual bool   Disconnect (bool bVoluntary)     = 0;

         virtual ISOURCE_SESSION* GetSessionInterface () = 0;

         bool bAutoConnect;
      };
   }
}

#include "Session.h"

#endif // RMAP_CORE_BASE_H
