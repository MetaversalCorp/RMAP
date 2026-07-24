/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Service.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                   CLASS (SERVICE::FACTORY::Impl)                                           **
*******************************************************************************************************************************/

class SERVICE::FACTORY::Impl
{
public:
   Impl (std::string sID)
   {
      m_sID = sID;
   }

   ~Impl ()
   {
   }

   std::string m_sID;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE::FACTORY)                                               **
*******************************************************************************************************************************/

SERVICE::FACTORY::FACTORY (std::string sID) :
   RMAP::CORE::FACTORY ()
{
   m_pImpl = new SERVICE::FACTORY::Impl (sID);
}

SERVICE::FACTORY::~FACTORY ()
{
   delete m_pImpl;
}

std::string SERVICE::FACTORY::sID () const
{
   return m_pImpl->m_sID;
}

/*******************************************************************************************************************************
**                                               CLASS (SERVICE::IREFERENCE::Impl)                                            **
*******************************************************************************************************************************/

class SERVICE::IREFERENCE::Impl
{
public:
   Impl (std::string sConnect)
   {
      std::map<std::string, std::string>::const_iterator it;

      pConnect = UTILS::Decode (sConnect);

      wPort = 0;

      it = pConnect.find ("secure");
      if (it != pConnect.end ())
      {
         bSecure = (it->second.compare ("true") == 0);
      }
      else bSecure = false;

      it = pConnect.find ("server");
      if (it != pConnect.end ())
      {
         std::string::size_type n = it->second.find (':');

         if (std::string::npos == n)
            sHost = pConnect["server"];
         else
         {
            sHost = pConnect["server"].substr (0, n);

            if (n + 1 < pConnect["server"].size ())
               wPort = std::stoi (pConnect["server"].substr (n + 1));
         }

         it = pConnect.find ("port");
         if (it != pConnect.end ())
         {
            wPort = atoi (it->second.c_str ());
         }
      }
   }

   ~Impl ()
   {
   }

   std::string                         sConnect;
   std::map<std::string, std::string>  pConnect;

   std::string sHost;
   int         wPort;
   bool        bSecure;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE::IREFERENCE)                                            **
*******************************************************************************************************************************/

SERVICE::IREFERENCE::IREFERENCE (std::string sID, std::string sConnect) :
   RMAP::CORE::IREFERENCE<SERVICE*, NAMESPACE*> (sID)
{
   m_pImpl = new SERVICE::IREFERENCE::Impl (sConnect);
}

SERVICE::IREFERENCE::~IREFERENCE ()
{
   delete m_pImpl;
}

std::map<std::string, std::string> SERVICE::IREFERENCE::GetConnectInfo (std::string& sHost, int& wPort, bool& bSecure)
{
   sHost    = m_pImpl->sHost;
   wPort    = m_pImpl->wPort;
   bSecure  = m_pImpl->bSecure;

   return m_pImpl->pConnect;
}

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE::Impl)                                                  **
*******************************************************************************************************************************/

class SERVICE::Impl
{
public:
   Impl (IREFERENCE* pReference, NAMESPACE* pNamespace)
   {
      this->pReference = pReference;
      this->pNamespace = pNamespace;

      sID  = pReference->UniqueId ();
      sKey = pReference->Key ();
   }

   ~Impl ()
   {
   }

   IREFERENCE* pReference;
   NAMESPACE*  pNamespace;

   std::string sID;
   std::string sKey;

   SHAREDOBJECT<CLIENT*, SERVICE*> sopClient;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE)                                                        **
*******************************************************************************************************************************/

SERVICE::SERVICE (IREFERENCE* pReference, NAMESPACE* pNamespace)
{
   m_pImpl = new Impl (pReference, pNamespace);
}

SERVICE::~SERVICE ()
{
   delete m_pImpl;
}

NAMESPACE* SERVICE::pNamespace ()
{
   return m_pImpl->pNamespace;
}

std::string const& SERVICE::sNamespace () const &
{
   return m_pImpl->pNamespace->sNamespace ();
}

std::string const& SERVICE::sID () const&
{
   return m_pImpl->sID;
}

std::string const& SERVICE::sKey () const &
{
   return m_pImpl->sKey;
}

CLIENT* SERVICE::Client_Open (CLIENT::IREFERENCE* pReference_Client)
{
   return m_pImpl->sopClient.Open (pReference_Client, this); // returns this on last close
}

CLIENT* SERVICE::Client_Close (CLIENT* pClient)
{
   if (m_pImpl->sopClient.Close (pClient->sKey ()) != NULL)
      pClient = NULL;

   return pClient;
}

int SERVICE::Client_Length ()
{
   return m_pImpl->sopClient.Length ();
}

bool SERVICE::Client_Exists (uint64_t twClientIx)
{
   return m_pImpl->sopClient.Exists (std::to_string (twClientIx).c_str ());
}

// Callers to Client_Get () must also call Client_Release () if the return value is not null
CLIENT* SERVICE::Client_Get (uint64_t twClientIx)
{
   return m_pImpl->sopClient.Get (std::to_string (twClientIx).c_str ());
}

// Callers to Client_Index () must also call Client_Release () if the return value is not null
CLIENT* SERVICE::Client_Index (int nIndex)
{
   return m_pImpl->sopClient.Index (nIndex);
}

CLIENT* SERVICE::Client_Enum (fnClientEnum fnEnum, void* pParam)
{
   return m_pImpl->sopClient.Enum (fnEnum, pParam);
}

void SERVICE::Client_Release ()
{
   return m_pImpl->sopClient.Release ();
}

/******************************************************************************************************************************/
