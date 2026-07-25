/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP_Core.h                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_CORE_APP_H
#define RMAP_CORE_APP_H

namespace RMAP
{
   namespace CORE
   {
      class SERVICECLASS;
      class MODELCLASS;
      class SOURCECLASS;

      typedef bool (*fnNamespaceEnum)   (NAMESPACE* pNamespace, void* pParam);
      typedef bool (*fnServiceClassEnum)(SERVICECLASS* pServiceClass, void* pParam);
      typedef bool (*fnModelClassEnum)  (MODELCLASS* pServiceClass, void* pParam);
      typedef bool (*fnSourceClassEnum) (SOURCECLASS* pSourceClass, void* pParam);
      typedef bool (*fnServiceEnum)     (SERVICE* pService, void* pParam);

      class NAMESPACE
      {
      private:
         class Impl;
         Impl* m_Impl;

      public:
         NAMESPACE (std::string sNamespace);
         ~NAMESPACE ();

         // ===== Accessors      ==================================================================================================

         std::string const& sNamespace () const&;

         // ===== Public Methods =====================================================================================================
         bool           ServiceClass_Add (SERVICE::FACTORY* pService_Factory);
         bool           ServiceClass_Remove (std::string sID_Service);
         int            ServiceClass_Length ();
         bool           ServiceClass_Exists (std::string sID_Service);
         SERVICECLASS*  ServiceClass_Get (std::string sID_Service);
         SERVICECLASS*  ServiceClass_Index (int nIndex);
         SERVICECLASS*  ServiceClass_Enum (fnServiceClassEnum fnEnum, void* pParam);
         void           ServiceClass_Release ();

         // ==========================================================================================================================

         bool        ModelClass_Add (MODEL::FACTORY* pModel_Factory);
         bool        ModelClass_Remove (std::string sID_Model);
         int         ModelClass_Length ();
         int         ModelClass_Exists (std::string sID_Model);
         MODELCLASS* ModelClass_Get (std::string sID_Model);
         MODELCLASS* ModelClass_Index (int nIndex);
         MODELCLASS* ModelClass_Enum (fnModelClassEnum fnEnum, void* pParam);
         void        ModelClass_Release ();

         // ==========================================================================================================================

         bool         SourceClass_Add (SOURCE::FACTORY* pSource_Factory);
         bool         SourceClass_Remove (std::string sID_Service, std::string sID_Model);
         int          SourceClass_Length (std::string sID_Service);
         int          SourceClass_Exists (std::string sID_Service, std::string sID_Model);
         SOURCECLASS* SourceClass_Get (std::string sID_Service, std::string sID_Model);
         SOURCECLASS* SourceClass_Index (std::string sID_Service, int nIndex);
         SOURCECLASS* SourceClass_Enum (std::string sID_Service, fnSourceClassEnum fnEnum, void* pParam);
         void         SourceClass_Release (std::string sID_Service);

         // ==========================================================================================================================

         SERVICE* Service_Open (std::string sID_Service, std::string sConnect);
         SERVICE* Service_Close (SERVICE* pService);
         int      Service_Length (std::string sID_Service);
         SERVICE* Service_Index (std::string sID_Service, int nIndex);
         SERVICE* Service_Enum (std::string sID_Service, fnServiceEnum fnEnum, void* pParam);
         void     Service_Release (std::string sID_Service);
      };

      class MODELCLASS
      {
      public:
         MODELCLASS (NAMESPACE* pNamespace, MODEL::FACTORY* pModel_Factory);
         virtual ~MODELCLASS ();

         // ===== Public Properties ==================================================================================================

         NAMESPACE* pNamespace ();
         MODEL::FACTORY* pModel_Factory ();

         // ===== Public Methods =====================================================================================================

         void SourceClass_Add ();
         void SourceClass_Remove ();
         int  SourceClass_Length ();

      private:
         NAMESPACE*      m_pNamespace;
         MODEL::FACTORY* m_pModel_Factory;
         int             m_nCount_Source;
      };

      class SERVICECLASS
      {
      public:
         SERVICECLASS (NAMESPACE* pNamespace, SERVICE::FACTORY* pService_Factory);
         virtual ~SERVICECLASS ();

         NAMESPACE* pNamespace ();
         SERVICE::FACTORY* pService_Factory ();

         int          SourceClass_Length ();
         bool         SourceClass_Add (MODELCLASS* pModelClass, SOURCE::FACTORY* pSource_Factory);
         bool         SourceClass_Remove (MODELCLASS* pModelClass);
         int          SourceClass_Exists (std::string sID_Model);
         SOURCECLASS* SourceClass_Get (std::string sID_Model);
         SOURCECLASS* SourceClass_Index (int nIndex);
         SOURCECLASS* SourceClass_Enum (fnSourceClassEnum fnEnum, void* pParam);
         void         SourceClass_Release ();

         SERVICE* Service_Open (IREFERENCE<SERVICE*, NAMESPACE*>* pReference, NAMESPACE* pNamespace);
         SERVICE* Service_Close (SERVICE* pService);
         int      Service_Length ();
         SERVICE* Service_Index (int nIndex);
         SERVICE* Service_Enum (fnServiceEnum fnEnum, void* pParam);
         void     Service_Release ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class  SOURCECLASS
      {
      public:
         SOURCECLASS (NAMESPACE* pNamespace, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory);
         virtual ~SOURCECLASS ();

         // ===== Public Properties ==================================================================================================

         MODEL::FACTORY* pModel_Factory ();
         SOURCE::FACTORY* pSource_Factory ();

      private:
         MODEL::FACTORY* m_pModel_Factory;
         SOURCE::FACTORY* m_pSource_Factory;
      };

      class CORE;
      class PLUGIN;
      class LIBRARY
      {
      public:
         LIBRARY (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion);
         virtual ~LIBRARY ();

         std::string sID () const;
         std::string sCopyright () const;
         std::string sTitle () const;
         std::string sVersion () const;

         virtual bool Install (PLUGIN* pPlugin) = 0;
         virtual void Unstall (PLUGIN* pPlugin) = 0;

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                 REGISTRY                                                                   **
      *******************************************************************************************************************************/

      class  REGISTRY
      {
      public:
         class  ZONE
         {
         public:
            ZONE (REGISTRY* pRegistry, std::string sZone);
            virtual ~ZONE ();

            void Set (std::string sName, std::string sValue, bool bPermanent);
            bool Get (std::string sName, std::string& sValue);
            void Remove (std::string sName);

            void Clear ();

         private:
            class Impl;
            Impl* m_pImpl;
         };

      public:
         REGISTRY (std::string sZone);
         virtual ~REGISTRY ();

         ZONE* Zone (std::string sZone);
         void Save ();

      protected:
         bool Get    (std::string sZone, std::string sName, std::string& sValue);
         void Set    (std::string sZone, std::string sName, std::string sValue, bool bPermanent);
         void Remove (std::string sZone, std::string sName);
         void Clear  (std::string sZone);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class INOTIFY
      {
      public:
         virtual void onNotify (intptr_t pParam) = 0;
      };

      class ILOGGER
      {
      public:
         virtual void onMessage (std::string& sLine) = 0;
      };

      class LOGGER
      {
      public:
         enum eLOGLEVEL
         {
            kLOGLEVEL_Trace,
            kLOGLEVEL_Info,
            kLOGLEVEL_Warning,
            kLOGLEVEL_Error
         };

      public:
         LOGGER (ILOGGER* pLogger);
         ~LOGGER ();

         void SetLogLevel (eLOGLEVEL Level);
         eLOGLEVEL GetLogLevel () const;

         void Log (eLOGLEVEL Level, std::string sModule, std::string sMessage);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class LNG;
      class APP
      {
      private:
         class Impl;
         Impl* m_pImpl;

      public:
         class REQUIRE
         {
         public:
            REQUIRE (const std::string& sSrc_List, const std::string& sID_Service, const std::string& sNamespace);
            ~REQUIRE ();

            bool Success ();

         private:
            class Impl;
            Impl* m_pImpl;

            bool m_bSuccess;
         };

      protected:
         APP ();
         ~APP ();

      public:
         APP (APP& obj) = delete; // Prevent Clone
         void operator=(const APP&) = delete; // Prevent Assignment

         static APP* GetInstance ();

         void     LoggerStart (ILOGGER* pLogger);
         void     LoggerStop ();
         LOGGER*  LoggerGet ();
         void     LoggerWrite (LOGGER::eLOGLEVEL Level, std::string sModule, std::string sMessage);

         bool     LibraryInstall (LIBRARY* pLibrary);
         void     LibraryUnstall (std::string sID);

         REQUIRE* Require (const std::string& sSrc_List, const std::string& sID_Service, const std::string& sNamespace);
         void     Release (REQUIRE* pRequire);

         PLUGIN*        Plugin_Open (std::string sID);
         PLUGIN*        Plugin_Close (PLUGIN* pPlugin);
         FACTORY*       Plugin_Factory (std::string sType, std::string sID_Factory);

         NAMESPACE*    Namespace_Add (std::string sNamespace);
         int           Namespace_Length ();
         int           Namespace_Exists (std::string sNamespace);
         NAMESPACE*    Namespace_Get (std::string sNamespace);
         NAMESPACE*    Namespace_Index (int nIndex);
         NAMESPACE*    Namespace_Enum (fnNamespaceEnum fnEnum, void* pParam);
         void          Namespace_Release ();

         SERVICE* Service_Open (std::string sNamespace, std::string sID_Service, std::string sConnect);
         SERVICE* Service_Close (std::string sNamespace, SERVICE* pService);
         int      Service_Length (std::string sNamespace, std::string sID_Service);
         SERVICE* Service_Index (std::string sNamespace, std::string sID_Service, int nIndex);
         SERVICE* Service_Enum (std::string sNamespace, std::string sID_Service, fnServiceEnum fnEnum, void* pParam);
         void     Service_Release (std::string sNamespace, std::string sID_Service);

         LNG* LnG_Open (std::string sNamespace, std::string sID_Service, std::string sConnect, std::string sSession);
         LNG* LnG_Close (LNG* pLnG);

         REGISTRY::ZONE* Zone (std::string sZone);

         void RegisterNotify (INOTIFY* pNotify);
         bool UnregisterNotify (INOTIFY* pNotify);
         void PostEvent (INOTIFY* pNotify, intptr_t pParam);
      };
   }
}
#endif //RMAP_CORE_APP_H
