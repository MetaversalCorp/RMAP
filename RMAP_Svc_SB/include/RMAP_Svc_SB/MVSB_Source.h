/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      MVSB_cpp : MVSB_Source.h                                              **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef MV_MVSB_SOURCE_H
#define MV_MVSB_SOURCE_H

namespace RMAP
{
   namespace SVC_SB
   {
      class SB_SBTIME : public RMAP::CORE::SOURCE
      {
      public:
         class FACTORY : public RMAP::CORE::SOURCE::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction);
            virtual ~FACTORY ();

            RMAP::CORE::SOURCE* Create (RMAP::CORE::CLIENT* pClient) override;
         };

      public:
         SB_SBTIME (RMAP::CORE::SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~SB_SBTIME ();

         static FACTORY* factory ();

         void Tick (int uCode, TIME tmServer);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class SBTIME : public RMAP::CORE::MODEL
      {
      public:
         typedef struct
         {
            int                           uCode;
            TIME                          tmServer;
         }
         NOTIFYPARAM;

      public:
         class FACTORY : public RMAP::CORE::MODEL::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            // ===== Public Properties ==================================================================================================

            virtual RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*>* Reference (std::vector<std::string> asArgs) override;
         };

      public:
         class IREFERENCE : public RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*>
         {
         public:
            IREFERENCE (std::string sID);
            virtual ~IREFERENCE ();

            std::string Key () override;
            RMAP::CORE::MODEL* Create (RMAP::CORE::SOURCE* pParam) override;
         };

      public:
         static RMAP::CORE::MODEL::FACTORY* factory ();

         SBTIME (IREFERENCE* pReference, SB_SBTIME* pSource);
         virtual ~SBTIME ();

         // ===== Public Methods =====================================================================================================

         void Tick (int uCode, TIME tmServer);
      };

      //----------------------------------------------------------

      class SB_SESSION_NULL : public SB_SESSION, RMAP::CORE::ISOURCE_SESSION_NULL
      {
      public:
         class FACTORY : public SB_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction);
            virtual ~FACTORY ();

            RMAP::CORE::SOURCE* Create (RMAP::CORE::CLIENT* pClient) override;
         };

      public:
         static FACTORY* factory ();

         SB_SESSION_NULL (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~SB_SESSION_NULL ();

         static std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> aAction;

      public:
         RMAP::CORE::SOURCE_SESSION::LOGIN* Login_Create    () override;
         RMAP::CORE::SOURCE_SESSION::LOGIN* Login_Destroy   (RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         void                        Progress        (RMAP::CORE::PROGRESS* pProgress) override;
         RMAP::CORE::CLIENT::IACTION*      Login_Request   (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                        Login_Response  (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary) override;
         RMAP::CORE::CLIENT::IACTION*      Logout_Request  (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin) override;
         bool                        Logout_Response (void* pParams, RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin, RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected) override;

      public:
         bool Attempt (int nReadyState) override;

         RMAP::CORE::ISOURCE_SESSION* GetSessionInterface ();

         // ISOURCE_SESSION_NULL
      public:
         bool Login () override;
         bool Logout () override;
      };
   }
}
#endif //MV_MVSB_SOURCE_H
