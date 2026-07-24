/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Model_Session_Null.cpp                                       **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                CLASS (MODEL_SESSION_NULL::FACTORY)                                         **
****************************************************************\**************************************************************/

MODEL_SESSION_NULL::FACTORY::FACTORY (std::string sID) :
   MODEL_SESSION::FACTORY (sID)
{
}

MODEL_SESSION_NULL::FACTORY::~FACTORY ()
{
}

MODEL_SESSION_NULL::IREFERENCE* MODEL_SESSION_NULL::FACTORY::Reference (std::vector<std::string> asArgs)
{
   bool bAutoConnect = (asArgs[0].compare ("false") == 0) ? false : true;

   return new IREFERENCE (sID (), bAutoConnect);
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION::IREFERENCE)                                              **
****************************************************************\**************************************************************/

MODEL_SESSION_NULL::IREFERENCE::IREFERENCE (std::string sID, bool bAutoConnect) :
   MODEL_SESSION::IREFERENCE (sID, bAutoConnect)
{
}

MODEL_SESSION_NULL::IREFERENCE::~IREFERENCE ()
{
}

MODEL* MODEL_SESSION_NULL::IREFERENCE::Create (SOURCE* pSource)
{
   return new MODEL_SESSION_NULL (this, dynamic_cast<SOURCE_SESSION*>(pSource));
}

/*******************************************************************************************************************************
**                                             CLASS (MODEL_SESSION_NULL::Impl)                                               **
****************************************************************\**************************************************************/

class MODEL_SESSION_NULL::Impl
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

   SOURCE_SESSION* pSource;
};

/*******************************************************************************************************************************
**                                                     CLASS (SESSION_NULL)                                                   **
*******************************************************************************************************************************/

MODEL_SESSION_NULL::FACTORY* MODEL_SESSION_NULL::factory ()
{
   return new MODEL_SESSION_NULL::FACTORY ("Session_Null");
}

MODEL_SESSION_NULL::MODEL_SESSION_NULL (IREFERENCE* pReference, SOURCE_SESSION* pSource) :
   MODEL_SESSION (pReference, pSource)
{
   m_pImpl = new Impl (pSource);
}

MODEL_SESSION_NULL::~MODEL_SESSION_NULL ()
{
   delete m_pImpl;
}

// ===== Source Methods =====================================================================================================

void MODEL_SESSION_NULL::Progress (PROGRESS* pProgress)
{
   Emit ("onProgress", pProgress);
}

void MODEL_SESSION_NULL::LoggedOut ()
{
   ISOURCE_SESSION_NULL* pSourceSession = dynamic_cast<ISOURCE_SESSION_NULL*> (m_pImpl->pSource->GetSessionInterface ());

   pSourceSession->Logout ();
}

// ===== Public Methods =====================================================================================================

uint64_t MODEL_SESSION_NULL::twUserIx ()
{
   return 0;
}

bool MODEL_SESSION_NULL::IsLoggedOut ()
{
   return true;
}

bool MODEL_SESSION_NULL::IsLoggedIn ()
{
   return false;
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_NULL::Connect ()
{
   return m_pImpl->pSource->Connect ();
}

bool MODEL_SESSION_NULL::Disconnect (bool bVoluntary)
{
   return m_pImpl->pSource->Disconnect (bVoluntary);
}

// --------------------------------------------------------------------------------------------------------------------------

bool MODEL_SESSION_NULL::Login (std::string sSession)
{
   return false;
}

bool MODEL_SESSION_NULL::Logout ()
{
   return false;
}

/******************************************************************************************************************************/
