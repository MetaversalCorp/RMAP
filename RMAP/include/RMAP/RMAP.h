/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP.h                                                     **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_H
#define RMAP_H

/*******************************************************************************************************************************
**                                                         Data Types                                                         **
*******************************************************************************************************************************/

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

typedef std::uint64_t TIME, * PTIME;
typedef std::uint32_t TIMEX, * PTIMEX;

#include "Utils.h"
#include "Base.h"
#include "App.h"
#include "Mem.h"

namespace RMAP
{
   namespace CORE
   {
      class LNG : public NOTIFICATION
      {
      public:
         enum eSTATE
         {
            DISCONNECTED = 0,   // IsDisconnected () == true,  IsConnected () == false
            CONNECTING   = 1,   // IsDisconnected () == false, IsConnected () == false
            LOGGEDOUT    = 2,   // IsDisconnected () == false, IsConnected () == true,  IsLoggedOut () == true,  IsLoggedIn () == false
            LOGGING      = 3,   // IsDisconnected () == false, IsConnected () == true,  IsLoggedOut () == false, IsLoggedIn () == false
            LOGGEDIN     = 4,   // IsDisconnected () == false, IsConnected () == true,  IsLoggedOut () == false, IsLoggedIn () == true
         };

         LNG ();
         ~LNG ();

         bool Init (const std::string& sNamespace, const std::string& sID_Service, const std::string& sConnect, const std::string& sSession);

         // ===== Accessors ==================================================================================================

         // DEPRECATE THIS, used for Authenticate
         MODEL_SESSION* pSession ();

         std::string const& sNamespace () const&;
         uint64_t twUserIx ();

         // ===== Methods   ==================================================================================================

         MEM::MODEL* Model_Open (const std::string &sID_Model, const std::string &sArgs);
         MEM::MODEL* Model_Close (MEM::MODEL* pModel);
         bool Login (const std::string &sSession);
         bool Logout ();

         void Notify (INOTICE* pNotice) override;
         bool IsReady ()                override;

      private:
         bool Login_Call ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 MODEL_OBJECT                                                               **
      *******************************************************************************************************************************/

      class MODEL_OBJECT : public MEM::MODEL
      {
      public:
         MODEL_OBJECT (MEM::MODEL::IREFERENCE* pReference, MEM::SOURCE* pSource);
         virtual ~MODEL_OBJECT ();

         enum eSTATE
         {
            EMPTY     = 0, // no data is present
            PARTIAL   = 1, // partial data is present in this object
            FULL      = 2, // all data is present in this object
            RECOVERED = 3, // all data is present in this object, and partial data is available in all child objects
         };

         // ===== Public Methods ====================================================================================================

         typedef bool (*fnModelObjectEnum)(MODEL_OBJECT* pChild, void* pvParam);

         typedef struct
         {
            MODEL_OBJECT*              pObject;
            MODEL_OBJECT*              pChild;
            MEM::CHANGE*               pChange;
         }
         NOTIFYPARAM;

         MODEL_OBJECT* Child_Enum    (std::string sID, fnModelObjectEnum fnEnum, void* pvParam);
         MODEL_OBJECT* Child_Get     (std::string sID, std::string sKey);
         void          Child_Release (std::string sID);

         // ===== Override Base Class ================================================================================================

         bool IsReady () override;

         void Partial () override;
         void Full () override;
         void Recovering () override;
         void Recovered () override;

         void Inserted (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange) override;
         void Deleting (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange) override;
         void Updating (MEM::MODEL* pObject, MEM::MODEL* pChild) override;
         void Updated  (MEM::MODEL* pObject, MEM::MODEL* pChild) override;
         void Changing (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange) override;
         void Changed  (MEM::MODEL* pObject, MEM::MODEL* pChild, MEM::CHANGE* pChange) override;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 PACKAGE                                                                    **
      *******************************************************************************************************************************/

      class PACKAGE
      {
      public:
         class PACKAGEPARAM
         {
         public:
            PACKAGEPARAM ();
            virtual ~PACKAGEPARAM ();
         };

         class IREFERENCE : public RMAP::CORE::IREFERENCE<PACKAGE*, PACKAGEPARAM*>
         {
         public:
            IREFERENCE (const std::string& sID, const std::string& sNamespace, const std::vector<std::string>& aService, const std::vector<std::string>& aModel, const std::vector<std::string>& aSource);
            virtual ~IREFERENCE ();

            std::string Key () override;

            // Accessors
         public:
            std::string const& sNamespace () const&;
            std::vector<std::string> const& aService () const&;
            std::vector<std::string> const& aModel () const&;
            std::vector<std::string> const& aSource () const&;

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         class FACTORY : public RMAP::CORE::FACTORY
         {
         public:
            FACTORY (const std::string& sID_Service, const std::string& sID_Package, const std::vector<std::string>& aService, const std::vector<std::string>& aModel, const std::vector<std::string>& aSource);
            virtual ~FACTORY ();

            static std::string toID (const std::string& sID_Service, const std::string& sID_Package);

            virtual PACKAGE::IREFERENCE* Reference (const std::string& sNamespace) = 0;

               // Accessors
         public:
            std::string const& sID () const&;

            std::string const& sID_Service () const&;
            std::vector<std::string> const& aService () const&;
            std::vector<std::string> const& aModel () const&;
            std::vector<std::string> const& aSource () const&;

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         PACKAGE (IREFERENCE* pReference, PACKAGEPARAM* pParam);
         virtual ~PACKAGE ();

         bool IsLoaded ();
         bool Install ();
         void Unstall ();

         // Accessors
      public:
         std::string const& sKey () const&;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class PLUGIN
      {
      public:
         class IREFERENCE : public RMAP::CORE::IREFERENCE<PLUGIN*, LIBRARY*>
         {
         public:
            IREFERENCE (std::string sID);
            virtual ~IREFERENCE ();

            std::string Key () override;
            PLUGIN* Create (LIBRARY* pLibrary) override;
         };

      public:
         PLUGIN (LIBRARY* pLibrary);
         virtual ~PLUGIN ();

         std::string sID () const;

         bool Install (APP* pCore);
         void Unstall (APP* pCore);

         bool AddPackage (PACKAGE::FACTORY* pFactory_Package, const std::string& sNamespace);
         bool InstallPackages (const std::string& sID_Service, const std::string& sNamespace, const std::string& sID_Package);

         void     Factory_Services (std::vector<SERVICE::FACTORY*> &apFactory);
         void     Factory_Models (std::vector<MODEL::FACTORY*> &apFactory);
         void     Factory_Sources (std::vector<SOURCE::FACTORY*>& apFactory);
         void     Factory_Packages (std::vector<PACKAGE::FACTORY*> &apFactory);
         FACTORY* Factory (std::string sType, std::string sID);

      private:
         class Impl;
         Impl* m_pImpl;

         int      m_nInstalled;
         bool     m_bInstalled;
         LIBRARY* m_pLibrary;
      };

      void Install ();
      void Unstall ();
   }
}
#endif //RMAP_H
