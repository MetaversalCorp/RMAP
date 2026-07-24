/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : LibraryRMAP.h                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2026 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

class LibraryRMAP : public RMAP::CORE::LIBRARY
{
public:
   static std::string sModuleName;

public:
   LibraryRMAP (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion);
   ~LibraryRMAP ();

   bool Install (RMAP::CORE::PLUGIN* pPlugin) override;
   void Unstall (RMAP::CORE::PLUGIN* pPlugin) override;

private:
   std::vector<RMAP::CORE::MODEL::FACTORY*> m_apFactory_Model;
};
