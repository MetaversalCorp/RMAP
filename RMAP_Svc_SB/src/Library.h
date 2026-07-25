/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   MVMF_cpp : LibraryMVSB.h                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

extern RMAP::SVC_SB::xTIME* g_pTime;

class ITIME_SERVICE : public RMAP::SVC_SB::xTIME::ITIME
{
public:
   void Tick (TIME tmSystem_Current) override;
};

#define MVSB_VDPARAM_VOLUNTARY                              0x00000001
#define MVSB_VDPARAM_DISCONNECTED                           0x00000002
#define MVSB_VDPARAM_CONNECTED                              0x00000004

bool     GetVDParam (intptr_t pVD, intptr_t nVDParam);
intptr_t SetVDParam (bool bVoluntary, bool bConnected, bool bDisconnected);

class LibrarySVC_SB : public RMAP::CORE::LIBRARY
{
public:
   static std::string sModuleName;

public:
   LibrarySVC_SB (std::string sID, std::string sCopyright, std::string sTitle, std::string sVersion);
   ~LibrarySVC_SB ();

   bool Install (RMAP::CORE::PLUGIN* pPlugin) override;
   void Unstall (RMAP::CORE::PLUGIN* pPlugin) override;

private:
   RMAP::CORE::APP::REQUIRE* m_pRequire;
   ITIME_SERVICE* m_pITime;

   std::vector<RMAP::CORE::SERVICE::FACTORY*>   m_apFactory_Service;
   std::vector<RMAP::CORE::MODEL::FACTORY*>     m_apFactory_Model;
   std::vector<RMAP::CORE::SOURCE::FACTORY*>    m_apFactory_Source;
};
