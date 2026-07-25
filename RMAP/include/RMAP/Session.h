/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP_Session.h                                             **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_CORE_SESSION_H
#define RMAP_CORE_SESSION_H

namespace RMAP
{
   namespace CORE
   {
      class ISOURCE_SESSION_C2A : public ISOURCE_SESSION
      {
      public:
         virtual bool Login (std::string sEmail, std::wstring sPassword, bool bRemember) = 0;
         virtual bool Authenticate (bool bPublic)                                        = 0;
         virtual bool Logout ()                                                          = 0;

         virtual uint64_t GetUserIx ()                                                   = 0;
      };

      class MODEL_SESSION_C2A : public MODEL_SESSION
      {
      private:
         class Impl;
         Impl* m_pImpl;

      public:
         class IREFERENCE : public MODEL_SESSION::IREFERENCE
         {
         public:
            IREFERENCE (std::string sID, bool bAutoConnect);
            virtual ~IREFERENCE ();

            MODEL* Create (SOURCE* pSource) override;
         };

         class FACTORY : public MODEL_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            IREFERENCE* Reference (std::vector<std::string> asArgs) override;
         };

      public:
         static FACTORY* factory ();

         enum eSTATE
         {
            DISCONNECTED           = 0, // no connection
            CONNECTED              = 1, // socket connected
            LOGGEDOUT              = 2, // socket connected,                            ready to login
            LOGGING                = 8, // logout initiated,                            awaiting response
            LOGGINGIN_NOTOKEN      = 3, // login initiated (no  login token presented), awaiting response
            LOGGINGIN_OLDTOKEN     = 4, // login initiated (old login token presented), awaiting response
            LOGGINGIN_AUTHENTICATE = 5, // login initiated (no  login token          ), awaiting authentication
            LOGGINGIN_NEWTOKEN     = 6, // login initiated (new login token presented), awaiting response
            LOGGEDIN               = 7, // login was successful
         };

         MODEL_SESSION_C2A (IREFERENCE* pReference, SOURCE_SESSION* pSource);
         virtual ~MODEL_SESSION_C2A ();

         SOURCE_SESSION::LOGIN* pLogin ();

         void Progress (PROGRESS* pProgress);

         void     LoggedOut () override;
         uint64_t twUserIx ()  override;

         // ===== Public Methods =====================================================================================================

         bool IsLoggedOut ();
         bool IsLoggedIn ();

         bool Connect ();
         bool Disconnect (bool bVoluntary);

         // --------------------------------------------------------------------------------------------------------------------------

         bool Login (std::string sSession);
         bool Authenticate (bool bPublic);
         bool Logout ();
      };

      class ISOURCE_SESSION_NULL : public ISOURCE_SESSION
      {
      public:
         virtual bool Login () = 0;
         virtual bool Logout () = 0;
      };

      class MODEL_SESSION_NULL : public MODEL_SESSION
      {
      private:
         class Impl;
         Impl* m_pImpl;

      public:
         class IREFERENCE : public MODEL_SESSION::IREFERENCE
         {
         public:
            IREFERENCE (std::string sID, bool bAutoConnect);
            virtual ~IREFERENCE ();

            MODEL* Create (SOURCE* pSource) override;
         };

         class FACTORY : public MODEL_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            IREFERENCE* Reference (std::vector<std::string> asArgs) override;
         };

      public:
         static FACTORY* factory ();

         enum eSTATE
         {
            DISCONNECTED           = 0, // no connection
            CONNECTED              = 1, // socket connected
            LOGGEDOUT              = 2, // socket connected, ready to [not] login
         };

         MODEL_SESSION_NULL (IREFERENCE* pReference, SOURCE_SESSION* pSource);
         virtual ~MODEL_SESSION_NULL ();

         void Progress (PROGRESS* pProgress);

         void     LoggedOut () override;
         uint64_t twUserIx ()  override;

         // ===== Public Methods =====================================================================================================

         bool IsLoggedOut ();
         bool IsLoggedIn ();

         bool Connect ();
         bool Disconnect (bool bVoluntary);

         // --------------------------------------------------------------------------------------------------------------------------

         bool Login (std::string sSession);
         bool Logout ();
      };

      class ISOURCE_SESSION_UIP : public ISOURCE_SESSION
      {
      public:
         virtual bool Login (std::string sMUserId, std::wstring sPassword, bool bRemember) = 0;
         virtual bool Logout () = 0;
      };

      class MODEL_SESSION_UIP : public MODEL_SESSION
      {
      private:
         class Impl;
         Impl* m_pImpl;

      public:
         class IREFERENCE : public MODEL_SESSION::IREFERENCE
         {
         public:
            IREFERENCE (std::string sID, bool bAutoConnect);
            virtual ~IREFERENCE ();

            MODEL* Create (SOURCE* pSource) override;
         };

         class FACTORY : public MODEL_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            IREFERENCE* Reference (std::vector<std::string> asArgs) override;
         };

      public:
         static FACTORY* factory ();

         enum eSTATE
         {
            DISCONNECTED           = 0, // no connection
            CONNECTED              = 1, // socket connected
            LOGGEDOUT              = 2, // socket connected,     ready to login
            LOGGING                = 3, // logout initiated,     awaiting response
            LOGGINGIN              = 3, // login initiated,      awaiting response
            LOGGINGIN_PASSWORD     = 4, // login initiated,      reset password required
            LOGGEDIN               = 5, // login was successful
         };

         MODEL_SESSION_UIP (IREFERENCE* pReference, SOURCE_SESSION* pSource);
         virtual ~MODEL_SESSION_UIP ();

         SOURCE_SESSION::LOGIN* pLogin ();

         void Progress (PROGRESS* pProgress);

         void     LoggedOut () override;
         uint64_t twUserIx ()  override;

         // ===== Public Methods =====================================================================================================

         bool IsLoggedOut ();
         bool IsLoggedIn ();

         bool Connect ();
         bool Disconnect (bool bVoluntary);

         // --------------------------------------------------------------------------------------------------------------------------

         bool Login (std::string sSession);
         bool Logout ();
      };
   }
}

#endif //RMAP_CORE_BASE_H
