/*******************************************************************************************************************************
**                                                                                                                            **
**                                                 RMAP_cpp : SourceClass.cpp                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

SOURCECLASS::SOURCECLASS (NAMESPACE* pNamespace, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory) :
   m_pModel_Factory (pModel_Factory),
   m_pSource_Factory (pSource_Factory)
{
}

SOURCECLASS::~SOURCECLASS ()
{
}

// ===== Public Properties ==================================================================================================

MODEL::FACTORY* SOURCECLASS::pModel_Factory ()   { return m_pModel_Factory;  }
SOURCE::FACTORY* SOURCECLASS::pSource_Factory () { return m_pSource_Factory; }
