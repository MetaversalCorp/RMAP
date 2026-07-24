/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Library.cpp                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/
                  
#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class LIBRARY::Impl
{
public:
   Impl (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion)
   {
      m_sID        = sID;
      m_sCopyright = sCopyright;
      m_sTitle     = sTitle;
      m_sVersion   = sVersion;
   }

   std::string m_sID;
   std::string m_sCopyright;
   std::string m_sTitle;
   std::string m_sVersion;
};

/*******************************************************************************************************************************
**                                                     CLASS (LIBRARY)                                                        **
*******************************************************************************************************************************/

LIBRARY::LIBRARY (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion)
{
   m_pImpl = new LIBRARY::Impl (sID, sCopyright, sTitle, sVersion);
}

LIBRARY::~LIBRARY ()
{
   delete m_pImpl;
}

std::string LIBRARY::sID () const
{
   return m_pImpl->m_sID;
}

std::string LIBRARY::sCopyright () const
{
   return m_pImpl->m_sCopyright;
}

std::string LIBRARY::sTitle () const
{
   return m_pImpl->m_sTitle;
}

std::string LIBRARY::sVersion () const
{
   return m_pImpl->m_sVersion;
}


/******************************************************************************************************************************/
