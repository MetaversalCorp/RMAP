/*******************************************************************************************************************************
**                                                                                                                            **
**                                          MVRest_cpp : LibraryMVRest.h                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

class LibrarySVC_Rest : public RMAP::CORE::LIBRARY
{
public:
   static std::string sModuleName;

public:
   LibrarySVC_Rest (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion);
   ~LibrarySVC_Rest ();

   bool Install (RMAP::CORE::PLUGIN* pPlugin) override;
   void Unstall (RMAP::CORE::PLUGIN* pPlugin) override;

private:
   RMAP::CORE::APP::REQUIRE* m_pRequire;

   std::vector<RMAP::CORE::SERVICE::FACTORY*>   m_apFactory_Service;
   std::vector<RMAP::CORE::MODEL::FACTORY*>     m_apFactory_Model;
   std::vector<RMAP::CORE::SOURCE::FACTORY*>    m_apFactory_Source;
};
