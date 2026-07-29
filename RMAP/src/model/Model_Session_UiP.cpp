/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Model_Session_UiP.cpp                                        **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                CLASS (MODEL_SESSION_UIP::FACTORY)                                          **
****************************************************************\**************************************************************/

MODEL_SESSION_UIP::FACTORY::FACTORY (std::string sID) :
   MODEL_SESSION::FACTORY (sID)
{
}

MODEL_SESSION_UIP::FACTORY::~FACTORY ()
{
}

MODEL_SESSION_UIP::IREFERENCE* MODEL_SESSION_UIP::FACTORY::Reference (std::vector<std::string> asArgs)
{
   bool bAutoConnect = (asArgs[0].compare ("false") == 0) ? false : true;

   return new IREFERENCE (sID (), bAutoConnect);
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION::IREFERENCE)                                              **
****************************************************************\**************************************************************/

MODEL_SESSION_UIP::IREFERENCE::IREFERENCE (std::string sID, bool bAutoConnect) :
   MODEL_SESSION::IREFERENCE (sID, bAutoConnect)
{
}

MODEL_SESSION_UIP::IREFERENCE::~IREFERENCE ()
{
}

MODEL* MODEL_SESSION_UIP::IREFERENCE::Create (SOURCE* pSource)
{
   return new MODEL_SESSION_UIP (this, dynamic_cast<SOURCE_SESSION*>(pSource));
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION_UIP::Impl)                                                **
****************************************************************\**************************************************************/

class MODEL_SESSION_UIP::Impl
{
public:
   Impl (SOURCE_SESSION* pSource) :
      pSource (pSource)
   {
   }

   ~Impl ()
   {
   }

   SOURCE_SESSION* pSource;
};

/*******************************************************************************************************************************
**                                                     CLASS (SESSION_C2A)                                                    **
*******************************************************************************************************************************/

MODEL_SESSION_UIP::FACTORY* MODEL_SESSION_UIP::factory ()
{
   return new MODEL_SESSION_UIP::FACTORY ("Session_C2a");
}

MODEL_SESSION_UIP::MODEL_SESSION_UIP (IREFERENCE* pReference, SOURCE_SESSION* pSource) :
   MODEL_SESSION (pReference, pSource)
{
   m_pImpl = new Impl (pSource);
}

MODEL_SESSION_UIP::~MODEL_SESSION_UIP ()
{
   delete m_pImpl;
}

SOURCE_SESSION::LOGIN* MODEL_SESSION_UIP::pLogin ()
{
   return m_pImpl->pSource->pLogin ();
}

// ===== Source Methods =====================================================================================================

void MODEL_SESSION_UIP::Progress (PROGRESS* pProgress)
{
   Emit ("onProgress", pProgress);
}

void MODEL_SESSION_UIP::LoggedOut ()
{
   ISOURCE_SESSION_UIP* pSourceSession = dynamic_cast<ISOURCE_SESSION_UIP*> (m_pImpl->pSource->GetSessionInterface ());

   pSourceSession->Logout ();
}

// ===== Public Methods =====================================================================================================

uint64_t MODEL_SESSION_UIP::twUserIx ()
{
   return 0;
}

bool MODEL_SESSION_UIP::IsLoggedOut ()
{
   return (ReadyState () == LOGGEDOUT && m_pImpl->pSource->IsLoggedOut ());
}

bool MODEL_SESSION_UIP::IsLoggedIn ()
{
   return (ReadyState () == LOGGEDIN && m_pImpl->pSource->IsLoggedIn ());
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_UIP::Connect ()
{
   return m_pImpl->pSource->Connect ();
}

bool MODEL_SESSION_UIP::Disconnect (bool bVoluntary)
{
   return m_pImpl->pSource->Disconnect (bVoluntary);
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_UIP::Login (std::string sSession)
{
   ISOURCE_SESSION_UIP* pSourceSession = dynamic_cast<ISOURCE_SESSION_UIP*> (m_pImpl->pSource->GetSessionInterface ());

   std::map<std::string, std::string> apSession = UTILS::Decode (sSession);

   return pSourceSession->Login (apSession["contact"], UTILS::UTF8_to_Wchar (apSession["password"].c_str ()), apSession["remember"] == "true");
}

bool MODEL_SESSION_UIP::Logout ()
{
   ISOURCE_SESSION_UIP* pSourceSession = dynamic_cast<ISOURCE_SESSION_UIP*> (m_pImpl->pSource->GetSessionInterface ());

   return pSourceSession->Logout ();
}

/******************************************************************************************************************************/
