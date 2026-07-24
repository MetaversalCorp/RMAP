/*******************************************************************************************************************************
**                                                                                                                            **
**                                                  RMAP_cpp : ModelClass.cpp                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

MODELCLASS::MODELCLASS (NAMESPACE* pNamespace, MODEL::FACTORY* pModel_Factory) :
   m_pNamespace (pNamespace),
   m_pModel_Factory (pModel_Factory),
   m_nCount_Source (0)
{
}

MODELCLASS::~MODELCLASS ()
{
}

// ===== Public Properties ==================================================================================================

NAMESPACE*      MODELCLASS::pNamespace ()       { return m_pNamespace;     }
MODEL::FACTORY* MODELCLASS::pModel_Factory ()   { return m_pModel_Factory; }

// ===== Public Methods =====================================================================================================

void MODELCLASS::SourceClass_Add ()
{
   m_nCount_Source++;
}

void MODELCLASS::SourceClass_Remove ()
{
   m_nCount_Source--;
}

int MODELCLASS::SourceClass_Length ()
{
   return m_nCount_Source;
}
