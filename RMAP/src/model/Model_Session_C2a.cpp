/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Model_Session_C2a.cpp                                        **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                CLASS (MODEL_SESSION_C2A::FACTORY)                                                **
****************************************************************\**************************************************************/

MODEL_SESSION_C2A::FACTORY::FACTORY (std::string sID) :
   MODEL_SESSION::FACTORY (sID)
{
}

MODEL_SESSION_C2A::FACTORY::~FACTORY ()
{
}

MODEL_SESSION_C2A::IREFERENCE* MODEL_SESSION_C2A::FACTORY::Reference (std::vector<std::string> asArgs)
{
   bool bAutoConnect = (asArgs[0].compare ("false") == 0) ? false : true;

   return new IREFERENCE (sID (), bAutoConnect);
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION::IREFERENCE)                                              **
****************************************************************\**************************************************************/

MODEL_SESSION_C2A::IREFERENCE::IREFERENCE (std::string sID, bool bAutoConnect) :
   MODEL_SESSION::IREFERENCE (sID, bAutoConnect)
{
}

MODEL_SESSION_C2A::IREFERENCE::~IREFERENCE ()
{
}

MODEL* MODEL_SESSION_C2A::IREFERENCE::Create (SOURCE* pSource)
{
   return new MODEL_SESSION_C2A (this, dynamic_cast<SOURCE_SESSION*>(pSource));
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION_C2A::Impl)                                                **
****************************************************************\**************************************************************/

class MODEL_SESSION_C2A::Impl
{
public:
   Impl (SOURCE_SESSION* pSource) :
      pSource (pSource)
   {
   }

   ~Impl ()
   {
      // delete pAgent;
   }

   SOURCE_SESSION*  pSource;
};

/*******************************************************************************************************************************
**                                                     CLASS (SESSION_C2A)                                                    **
*******************************************************************************************************************************/

MODEL_SESSION_C2A::FACTORY* MODEL_SESSION_C2A::factory ()
{
   return new MODEL_SESSION_C2A::FACTORY ("Session_C2a");
}

MODEL_SESSION_C2A::MODEL_SESSION_C2A (IREFERENCE* pReference, SOURCE_SESSION* pSource) :
   MODEL_SESSION (pReference, pSource)
{
   m_pImpl = new Impl (pSource);
}

MODEL_SESSION_C2A::~MODEL_SESSION_C2A ()
{
   delete m_pImpl;
}

SOURCE_SESSION::LOGIN* MODEL_SESSION_C2A::pLogin ()
{ 
   return m_pImpl->pSource->pLogin ();
}

uint64_t MODEL_SESSION_C2A::twUserIx ()
{
   ISOURCE_SESSION_C2A* pSourceSession = dynamic_cast<ISOURCE_SESSION_C2A*> (m_pImpl->pSource->GetSessionInterface ());

   return pSourceSession->GetUserIx ();
}

// ===== Source Methods =====================================================================================================

void MODEL_SESSION_C2A::Progress (PROGRESS* pProgress)
{
   Emit ("onProgress", pProgress);
}

void MODEL_SESSION_C2A::LoggedOut ()
{
   ISOURCE_SESSION_C2A* pSourceSession = dynamic_cast<ISOURCE_SESSION_C2A*> (m_pImpl->pSource->GetSessionInterface ());

   pSourceSession->Logout ();
}

// ===== Public Methods =====================================================================================================

bool MODEL_SESSION_C2A::IsLoggedOut ()
{
   return (ReadyState () == LOGGEDOUT && m_pImpl->pSource->IsLoggedOut ());
}

bool MODEL_SESSION_C2A::IsLoggedIn ()
{
   return (ReadyState () == LOGGEDIN && m_pImpl->pSource->IsLoggedIn ());
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_C2A::Connect ()
{
   return m_pImpl->pSource->Connect ();
}

bool MODEL_SESSION_C2A::Disconnect (bool bVoluntary)
{
   return m_pImpl->pSource->Disconnect (bVoluntary);
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_C2A::Login (std::string sSession)
{
   ISOURCE_SESSION_C2A* pSourceSession = dynamic_cast<ISOURCE_SESSION_C2A*> (m_pImpl->pSource->GetSessionInterface ());

   std::map<std::string, std::string> apSession = UTILS::Decode (sSession);

   return pSourceSession->Login (apSession["contact"], UTILS::UTF8_to_Wchar (apSession["password"].c_str ()), apSession["remember"] == "true");
}

bool MODEL_SESSION_C2A::Authenticate (bool bPublic)
{
   ISOURCE_SESSION_C2A* pSourceSession = dynamic_cast<ISOURCE_SESSION_C2A*> (m_pImpl->pSource->GetSessionInterface ());

   return pSourceSession->Authenticate (bPublic);
}

bool MODEL_SESSION_C2A::Logout ()
{
   ISOURCE_SESSION_C2A* pSourceSession = dynamic_cast<ISOURCE_SESSION_C2A*> (m_pImpl->pSource->GetSessionInterface ());

   return pSourceSession->Logout ();
}

/******************************************************************************************************************************/
