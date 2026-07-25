/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    MVSB_cpp : Service.cpp                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SOCKETIO;

/*******************************************************************************************************************************
**                                                     CLASS (IREFRENCE)                                                      **
*******************************************************************************************************************************/

SERVICE::IREFERENCE::IREFERENCE (std::string sID, std::string sConnect) :
   RMAP::CORE::SERVICE::IREFERENCE (sID, sConnect)
{
   std::map<std::string, std::string> pConnect;

   pConnect = GetConnectInfo (NetSettings.sHost, NetSettings.wPort, NetSettings.bSecure);

   NetSettings.sSession = pConnect["session"];
}

SERVICE::IREFERENCE::~IREFERENCE ()
{
}

std::string SERVICE::IREFERENCE::Key ()
{
   return std::string (NetSettings.bSecure ? "true" : "false") + ";" + NetSettings.sHost + ";" + std::to_string (NetSettings.wPort) + ";" + NetSettings.sSession;
}

RMAP::CORE::SERVICE* SERVICE::IREFERENCE::Create (RMAP::CORE::NAMESPACE* pNamespace)
{
   return new SERVICE (this, pNamespace);
}

/*******************************************************************************************************************************
**                                                     CLASS (Factory)                                                        **
*******************************************************************************************************************************/

SERVICE::FACTORY::FACTORY (std::string sID) :
   RMAP::CORE::SERVICE::FACTORY (sID)
{
}

SERVICE::FACTORY::~FACTORY ()
{
}

RMAP::CORE::SERVICE::IREFERENCE* SERVICE::FACTORY::Reference (std::string sConnect)
{
   return new IREFERENCE (sID (), sConnect);
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class SERVICE::Impl
{
public:
   Impl (IREFERENCE* pReference)
   {
      NetSettings = pReference->NetSettings;
   }

   ~Impl ()
   {
   }

   NETSETTINGS NetSettings;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE)                                                        **
*******************************************************************************************************************************/

SERVICE::SERVICE (IREFERENCE* pReference, RMAP::CORE::NAMESPACE* pNamespace) :
   RMAP::CORE::SERVICE (pReference, pNamespace)
{
   m_pImpl = new Impl (pReference);
}

SERVICE::~SERVICE ()
{
   delete m_pImpl;
}

SERVICE::FACTORY* SERVICE::factory ()
{
   return new FACTORY ("MVIO");
}

SERVICE::NETSETTINGS* SERVICE::pNetSettings () 
{ 
   return &m_pImpl->NetSettings; 
}

// ===== Public Methods =====================================================================================================

RMAP::CORE::CLIENT* SERVICE::Client_Open (uint64_t twClientIx)
{
   return RMAP::CORE::SERVICE::Client_Open (CLIENT::Reference (twClientIx));
}

std::string SERVICE::GetSessionString ()
{
   return m_pImpl->NetSettings.sSession;
}

bool SERVICE::Connected (CLIENT* pClient)
{
   NOTIFYPARAM np;

   np.nConnected = eCLIENT::CONNECTED;
   np.pClient = pClient;
   Emit ("onClient", &np);

   return true;
}

void SERVICE::Disconnected (CLIENT* pClient)
{
   NOTIFYPARAM np;

   np.nConnected = eCLIENT::DISCONNECTED;
   np.pClient = pClient;
   Emit ("onClient", &np);
}

/******************************************************************************************************************************/
