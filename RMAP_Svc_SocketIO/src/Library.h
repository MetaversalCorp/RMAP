/*******************************************************************************************************************************
**                                                                                                                            **
**                                          MVIO_cpp : LibrarySVC_SocketIO.h                                                      **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

class LibrarySVC_SocketIO : public RMAP::CORE::LIBRARY
{
public:
   static std::string sModuleName;

public:
   LibrarySVC_SocketIO (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion);
   ~LibrarySVC_SocketIO ();

   bool Install (RMAP::CORE::PLUGIN* pPlugin) override;
   void Unstall (RMAP::CORE::PLUGIN* pPlugin) override;

private:
   RMAP::CORE::APP::REQUIRE* m_pRequire;

   std::vector<RMAP::CORE::SERVICE::FACTORY*>   m_apFactory_Service;
   std::vector<RMAP::CORE::MODEL::FACTORY*>     m_apFactory_Model;
   std::vector<RMAP::CORE::SOURCE::FACTORY*>    m_apFactory_Source;
};
