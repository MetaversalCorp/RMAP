/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Model.cpp                                                    **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (MODEL::FACTORY::Impl)                                           **
*******************************************************************************************************************************/

class MODEL::FACTORY::Impl
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
**                                                     CLASS (MODEL::FACTORY)                                                 **
*******************************************************************************************************************************/

MODEL::FACTORY::FACTORY (std::string sID) :
   RMAP::CORE::FACTORY ()
{
   m_pImpl = new MODEL::FACTORY::Impl (sID);
}

MODEL::FACTORY::~FACTORY ()
{
   delete m_pImpl;
}

std::string MODEL::FACTORY::sID () const
{
   return m_pImpl->m_sID;
}

/*******************************************************************************************************************************
**                                                     CLASS (MODEL::Impl)                                                    **
*******************************************************************************************************************************/

class MODEL::Impl
{
public:
   Impl (IREFERENCE<MODEL*, SOURCE*>* pReference, SOURCE* pSource) :
      pSource (pSource)
   {
      sID        = pReference->UniqueId ();
      sKey       = pReference->Key ();
   }

   ~Impl ()
   {
   }

   std::string sID;
   std::string sKey;
   SOURCE*     pSource;

   SHAREDOBJECT<MODEL*, SOURCE*> sopModel;
   std::map<int, SOURCE*>        apSource;
};

/*******************************************************************************************************************************
**                                                     CLASS (MODEL)                                                          **
*******************************************************************************************************************************/

MODEL::MODEL (IREFERENCE<MODEL*, SOURCE*>* pReference, SOURCE* pSource)
{
   m_pImpl = new Impl (pReference, pSource);

   pSource->initialize (this);
}

MODEL::~MODEL ()
{
   delete m_pImpl;
}

NAMESPACE*  MODEL::pNamespace () 
{ 
   return m_pImpl->pSource->pNamespace (); 
}

std::string MODEL::sNamespace () const
{ 
   return m_pImpl->pSource->sNamespace (); 
}

std::string MODEL::sID () const
{ 
   return m_pImpl->sID.c_str ();           
}

std::string MODEL::sKey () const
{ 
   return m_pImpl->sKey.c_str ();          
}

SOURCE* MODEL::pSource ()    
{ 
   return m_pImpl->pSource;                
}

std::vector<std::string> MODEL::Actions ()
{
   return m_pImpl->pSource->Actions ();
}

CLIENT::IACTION* MODEL::Request (std::string sAction)
{
   return m_pImpl->pSource->Request (sAction);
}

int MODEL::Attach (NOTIFICATION* pNotify, bool bPropagate, bool bNotifyOnReady)
{
   int nLength = NOTIFICATION::Attach (pNotify, bPropagate, bNotifyOnReady);

   if (nLength == 1)
      m_pImpl->pSource->Attach ();

   return nLength;
}

int MODEL::Detach (NOTIFICATION* pNotify)
{
   int nLength = NOTIFICATION::Detach (pNotify);

   if (nLength == 0)
      m_pImpl->pSource->Detach ();

   return nLength;
}

bool MODEL::IsReady ()
{
   return true;
}

/******************************************************************************************************************************/
