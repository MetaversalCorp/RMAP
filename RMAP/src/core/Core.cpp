/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Core.cpp                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <queue>

using namespace RMAP::CORE;

typedef struct tagNOTIFYEVENT
{
   INOTIFY*                            pNotify;
   intptr_t                            pParam;
}
NOTIFYEVENT;

/*******************************************************************************************************************************
**                                                     CLASS (APP::Impl)                                                     **
*******************************************************************************************************************************/

class ICOMPARE_LnG : public COLLECTION<LNG*, LNG*>::ICOMPARE
{
public:
   int Compare (LNG* pKey, LNG* pValue) override
   {
      return (pKey == pValue);
   }
};

class APP::Impl
{
public:
   Impl (std::string sZone) :
      m_bShutdown (false)
   {
      pRegistry = new REGISTRY (sZone);

      cpLnG = new COLLECTION<LNG*, LNG*> (NULL, new ICOMPARE_LnG ());

      m_pThread = new std::thread (&APP::Impl::ThreadLoop, this);
   }

   ~Impl ()
   {
      Shutdown ();
      m_pThread->join ();

      delete m_pThread;

      delete pRegistry;

      delete cpLnG;
   }

   void LoggerStart (ILOGGER* pLogger)
   {
      if (m_pLogger == NULL)
      {
         m_pLogger = new LOGGER (pLogger);
      }
   }

   void LoggerStop ()
   {
      if (m_pLogger != NULL)
      {
         delete m_pLogger;
         m_pLogger = NULL;
      }
   }

   NAMESPACE* Namespace_Add (std::string sNamespace)
   {
      NAMESPACE* pNamespace;

      pNamespace = cpNamespace.Get (sNamespace, true);
      {
         if (pNamespace == NULL)
         {
            pNamespace = new NAMESPACE (sNamespace);

            if (cpNamespace.Add (sNamespace, pNamespace))
            {
               // success
            }
            else
            {
               delete pNamespace;
               pNamespace = NULL;
            }
         }
      }
      if (pNamespace == NULL)
         cpNamespace.Release ();

      return pNamespace;
   }

   std::map<std::string, LIBRARY*>     mpLibrary;
   SHAREDOBJECT<PLUGIN*, LIBRARY*>        sopPlugin;
   COLLECTION<std::string, NAMESPACE*>    cpNamespace;
   COLLECTION<LNG*, LNG*>* cpLnG;
   REGISTRY* pRegistry;

   LOGGER*                                m_pLogger;

   static APP* pInstance;
   static std::mutex CS;

   /*******************************************************************************************************************************
   **                                                     Thread                                                                 **
   *******************************************************************************************************************************/

   void Shutdown ()
   {
      std::lock_guard<std::mutex> guard (m_mutex);
      m_bShutdown = true;
      m_condVar.notify_all ();
   }

   void ThreadLoop ()
   {
      std::unique_lock<std::mutex> mlock (m_mutex);
      m_condVar.wait (mlock, std::bind (&APP::Impl::Control, this));
   }

   bool Control ()
   {
      NOTIFYEVENT NotifyEvent;

      if (m_bShutdown == false)
      {
         m_CS_Event.lock ();
         {
            if (m_aEvent.empty () == false)
            {
               NotifyEvent = m_aEvent.front ();
               m_aEvent.pop ();
            }
            else NotifyEvent.pNotify = NULL;
         }
         m_CS_Event.unlock ();

         if (NotifyEvent.pNotify != NULL)
         {
            NotifyEvent.pNotify->onNotify (NotifyEvent.pParam);

            m_CS_Recv.lock ();
            {
               auto it = m_aRecv.find (NotifyEvent.pNotify);

               if (it != m_aRecv.end ())
                  it->second--;
            }
            m_CS_Recv.unlock ();
         }
      }

      return m_bShutdown;
   }

   void CtlBreak_Thread ()
   {
      std::lock_guard<std::mutex> guard (m_mutex);
      m_condVar.notify_all ();
   }

   void RegisterNotify (INOTIFY* pNotify)
   {
      m_CS_Recv.lock ();
      {
         m_aRecv.insert ({ pNotify, 0 });
      }
      m_CS_Recv.unlock ();
   }

   bool UnregisterNotify (INOTIFY* pNotify)
   {
      bool bResult;

      m_CS_Recv.lock ();
      {
         auto it = m_aRecv.find (pNotify);

         if (it->second == 0)
         {
            bResult = true;
            m_aRecv.erase (it);
         }
         else bResult = false;
      }
      m_CS_Recv.unlock ();

      return bResult;
   }

   void PostEvent (INOTIFY* pNotify, intptr_t pParam)
   {
      NOTIFYEVENT NotifyEvent;

      m_CS_Recv.lock ();
      {
         auto it = m_aRecv.find (pNotify);

         if (it != m_aRecv.end ())
            it->second++;
      }
      m_CS_Recv.unlock ();

      m_CS_Event.lock ();
      {
         NotifyEvent.pNotify  = pNotify;
         NotifyEvent.pParam   = pParam;
         m_aEvent.push (NotifyEvent);
      }
      m_CS_Event.unlock ();

      CtlBreak_Thread ();
   }

private:
   std::thread*                  m_pThread;
   std::mutex                    m_mutex;
   std::condition_variable       m_condVar;
   bool                          m_bShutdown;

   std::recursive_mutex          m_CS_Recv;
   std::map<INOTIFY*, int>       m_aRecv;

   std::recursive_mutex          m_CS_Event;
   std::queue<NOTIFYEVENT>       m_aEvent;
};

/*******************************************************************************************************************************
**                                                     Global Variables                                                       **
*******************************************************************************************************************************/

APP*       APP::Impl::pInstance { NULL };
std::mutex  APP::Impl::CS;

/*******************************************************************************************************************************
**                                                     CLASS (CORE)                                                           **
*******************************************************************************************************************************/

APP* APP::GetInstance ()
{
   std::lock_guard<std::mutex> lock (APP::Impl::CS);
   {
      if (APP::Impl::pInstance == NULL)
      {
         APP::Impl::pInstance = new APP ();
      }
   }

   return APP::Impl::pInstance;
}

APP::APP ()
{
   m_pImpl = new Impl ("TBD.COM"); // TODO: Application needs to specify the Zone.
}

APP::~APP ()
{
   delete m_pImpl;
}

void APP::LoggerStart (ILOGGER* pLogger)
{
   m_pImpl->LoggerStart (pLogger);
}

void APP::LoggerStop ()
{
   m_pImpl->LoggerStop ();
}

LOGGER* APP::LoggerGet ()
{
   return m_pImpl->m_pLogger;
}

void APP::LoggerWrite (LOGGER::eLOGLEVEL Level, std::string sModule, std::string sMessage)
{
   if (m_pImpl->m_pLogger != NULL)
      m_pImpl->m_pLogger->Log (Level, sModule, sMessage);
}

// This section is language specific

APP::REQUIRE* APP::Require (const std::string& sSrc_List, const std::string& sID_Service, const std::string& sNamespace)
{
   REQUIRE* pRequire = new REQUIRE (sSrc_List, sID_Service, sNamespace);

   if (pRequire->Success () == false)
   {
      delete pRequire;
      pRequire = NULL;
   }

   return pRequire;
}

void APP::Release (REQUIRE* pRequire)
{
   delete pRequire;
}

bool APP::LibraryInstall (LIBRARY* pLibrary)
{
   if (m_pImpl->mpLibrary.find (pLibrary->sID ()) == m_pImpl->mpLibrary.end ())
   {
      m_pImpl->mpLibrary[pLibrary->sID ()] = pLibrary;
   }

   return true;
}

void APP::LibraryUnstall (std::string sID)
{
   LIBRARY* pLibrary = NULL;
   std::map<std::string, LIBRARY*>::const_iterator it;

   it = m_pImpl->mpLibrary.find (sID);

   if (it != m_pImpl->mpLibrary.end ())
   {
      delete it->second;

      m_pImpl->mpLibrary.erase (it);
   }
}

PLUGIN* APP::Plugin_Open (const std::string& sID)
{
   PLUGIN* pPlugin = NULL;
   PLUGIN::IREFERENCE* pIReference;
   std::map<std::string, LIBRARY*>::iterator it;
   LIBRARY* pLibrary;
   
   it = m_pImpl->mpLibrary.find (sID);

   if (it != m_pImpl->mpLibrary.end ())
   {
      pLibrary = it->second;

      pIReference = new PLUGIN::IREFERENCE (pLibrary->sID ());

      m_pImpl->sopPlugin.Open (pIReference, pLibrary);

      delete pIReference;

      if (pPlugin = m_pImpl->sopPlugin.Get (pLibrary->sID ())) // this provides mutual exclusion
      {
         if (pPlugin->Install (this))
         {
            // success
         }
         else pPlugin = Plugin_Close (pPlugin);

         m_pImpl->sopPlugin.Release ();
      }
   }

   return pPlugin;
}

PLUGIN* APP::Plugin_Close (PLUGIN* pPlugin)
{
   std::string sID = pPlugin->sID ();

   if ((pPlugin = m_pImpl->sopPlugin.Get (sID)) != NULL) // this provides mutual exclusion
   {
      pPlugin->Unstall (this);

      m_pImpl->sopPlugin.Release ();
   }

   if (m_pImpl->sopPlugin.Close (sID) != NULL)
   {
      LibraryUnstall (sID);
   }

   pPlugin = NULL;

   return pPlugin;
}

FACTORY* APP::Plugin_Factory (const std::string& sType, const std::string& sID_Factory)
{
   RMAP::CORE::FACTORY* pFactory = NULL;
   std::vector<std::string> asID;
   PLUGIN* pPlugin;

   asID = UTILS::splitString (sID_Factory, '/');

   if (asID.size () == 2)
   {
      if ((pPlugin = m_pImpl->sopPlugin.Get (asID[0])) != NULL)
      {
         pFactory = pPlugin->Factory (sType, asID[1]);

         m_pImpl->sopPlugin.Release ();
      }
      else LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Unknown plugin : " + asID[0]);
   }

   return pFactory;
}

NAMESPACE* APP::Namespace_Add (std::string sNamespace)
{
   // this was added because we are no longer calling ServiceClass_Add from this class anymore
   // we should either add an explicit Namespace_Remove, or automatically delete a namespace if it is empty upon release 
   // or do some form of reference counting

   return m_pImpl->Namespace_Add (sNamespace);
}

int APP::Namespace_Length ()
{
   return m_pImpl->cpNamespace.Length ();
}

int APP::Namespace_Exists (std::string sNamespace)
{
   return m_pImpl->cpNamespace.Exists (sNamespace);
}

// Callers to Namespace_Get () must also call Namespace_Release () if the return value is not NULL
NAMESPACE* APP::Namespace_Get (std::string sNamespace)
{
   return m_pImpl->cpNamespace.Get (sNamespace);
}

// Callers to Namespace_Index () must also call Namespace_Release () if the return value is not NULL
NAMESPACE* APP::Namespace_Index (int nIndex)
{
   return m_pImpl->cpNamespace.Index (nIndex);
}

// Callers to Namespace_Enum () must also call Namespace_Release () if the return value is not NULL
NAMESPACE* APP::Namespace_Enum (fnNamespaceEnum fnEnum, void* pParam)
{
   NAMESPACE* pNamespace = NULL;
   PCOLLECTION_ENUM pEnum;
   bool bResult = true;

   if (pEnum = m_pImpl->cpNamespace.Enum_Begin ())
   {
      while (bResult && (pNamespace = m_pImpl->cpNamespace.Enum_Next (pEnum)) != NULL)
         if (bResult = fnEnum (pNamespace, pParam))
            m_pImpl->cpNamespace.Release ();

      m_pImpl->cpNamespace.Enum_End (pEnum);
   }

   return pNamespace;
}

void APP::Namespace_Release ()
{
   m_pImpl->cpNamespace.Release ();
}

SERVICE* APP::Service_Open (std::string sNamespace, std::string sID_Service, std::string sConnect)
{
   SERVICE* pService = NULL;
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      pService = pNamespace->Service_Open (sID_Service, sConnect);

      m_pImpl->cpNamespace.Release ();
   }

   return pService;
}

SERVICE* APP::Service_Close (std::string sNamespace, SERVICE* pService)
{
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      pService = pNamespace->Service_Close (pService);

      m_pImpl->cpNamespace.Release ();
   }

   return pService;
}

// --------------------------------------------------------------------------------------------------------------------------

int APP::Service_Length (std::string sNamespace, std::string sID_Service)
{
   int nLength = -1;
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      nLength = pNamespace->Service_Length (sID_Service);

      m_pImpl->cpNamespace.Release ();
   }

   return nLength;
}

// Callers to Service_Index () must also call Service_Release () if the return value is not NULL
SERVICE* APP::Service_Index (std::string sNamespace, std::string sID_Service, int nIndex)
{
   SERVICE* pService = NULL;
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      pService = pNamespace->Service_Index (sID_Service, nIndex);

      m_pImpl->cpNamespace.Release ();
   }

   return pService;
}

// Callers to Service_Enum () must also call Service_Release () if the return value is not NULL
SERVICE* APP::Service_Enum (std::string sNamespace, std::string sID_Service, fnServiceEnum fnEnum, void* pParam)
{
   SERVICE* pService = NULL;
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      pService = pNamespace->Service_Enum (sID_Service, fnEnum, pParam);

      m_pImpl->cpNamespace.Release ();
   }

   return pService;
}

void APP::Service_Release (std::string sNamespace, std::string sID_Service)
{
   NAMESPACE* pNamespace;

   if ((pNamespace = m_pImpl->cpNamespace.Get (sNamespace)) != NULL)
   {
      pNamespace->Service_Release (sID_Service);

      m_pImpl->cpNamespace.Release ();
   }
}

LNG* APP::LnG_Open (std::string sNamespace, std::string sID_Service, std::string sConnect, std::string sSession)
{
   LNG* pLnG = new LNG ();

   if (pLnG->Init (sNamespace, sID_Service, sConnect, sSession))
   {
      m_pImpl->cpLnG->Add (pLnG, pLnG);
   }
   else
   {
      delete pLnG;
      pLnG = NULL;
   }

   return pLnG;
}

LNG* APP::LnG_Close (LNG* pLnG)
{
   if (m_pImpl->cpLnG->Remove (pLnG) == pLnG)
   {
      delete pLnG;
      pLnG = NULL;
   }

   return pLnG;
}

REGISTRY::ZONE* APP::Zone (std::string sZone)
{
   return m_pImpl->pRegistry->Zone (sZone);
}

void APP::RegisterNotify (INOTIFY* pNotify)
{
   m_pImpl->RegisterNotify (pNotify);
}

bool APP::UnregisterNotify (INOTIFY* pNotify)
{
   return m_pImpl->UnregisterNotify (pNotify);
}

void APP::PostEvent (INOTIFY* pNotify, intptr_t pParam)
{
   m_pImpl->PostEvent (pNotify, pParam);
}

/*******************************************************************************************************************************
**                                                   CLASS (REQUIRE::IMPL)                                                    **
*******************************************************************************************************************************/

class APP::REQUIRE::Impl
{
public:
   Impl ()
   {
   }

   std::vector<PLUGIN*> apPlugin;
};

/*******************************************************************************************************************************
**                                                     CLASS (REQUIRE)                                                        **
*******************************************************************************************************************************/

APP::REQUIRE::REQUIRE (const std::string& sSrc_List, const std::string& sID_Service, const std::string& sNamespace)
{
   int i;
   std::vector<std::string> vSrc = UTILS::splitString (sSrc_List, ',');
   PLUGIN* pPlugin;

   APP* pCore = APP::GetInstance ();

   m_pImpl = new REQUIRE::Impl ();
   
   for (i = 0; i < vSrc.size (); i++)
   {
      std::vector<std::string> vSrc2 = UTILS::splitString (vSrc[i].c_str (), '/');

      if (pPlugin = pCore->Plugin_Open (vSrc2[0].c_str ()))
      {
         m_pImpl->apPlugin.push_back (pPlugin);

         if (sID_Service.empty () == false)
         {
            if (pPlugin->InstallPackages (sID_Service, sNamespace, (vSrc2.size () == 2) ? vSrc2[i] : ""))
            {
            }
         }
      }
      else
      {
         pCore->LoggerWrite (LOGGER::kLOGLEVEL_Error, LibraryRMAP::sModuleName, "Plugin_Open (Failed): " + vSrc[0]);
         break;
      }
   }

   m_bSuccess = (m_pImpl->apPlugin.size () == vSrc.size ());
}

APP::REQUIRE::~REQUIRE ()
{
   int i;
   APP* pCore = APP::GetInstance ();

   for (i = (int)m_pImpl->apPlugin.size (); i > 0; i--)
      pCore->Plugin_Close (m_pImpl->apPlugin[i - 1]);

   delete m_pImpl;
}

bool APP::REQUIRE::Success ()
{
   return m_bSuccess;
}
