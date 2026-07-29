/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_SVC_SB : Service.cpp                                               **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     Callbacks                                                              **
*******************************************************************************************************************************/

bool CE_PreKill (RMAP::CORE::CLIENT* pClient, void* pvParam)
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient);

   pClientSB->SocketDisconnected (true);

   return true;
}

bool CE_SafeKill (RMAP::CORE::CLIENT* pClient, void* pvParam)
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient);

   return pClientSB->SafeKill ();
}

/*******************************************************************************************************************************
**                                                     CLASS (IREFRENCE)                                                      **
*******************************************************************************************************************************/

SERVICE::IREFERENCE::IREFERENCE (std::string sID, std::string sConnect) :
   RMAP::CORE::SERVICE::IREFERENCE (sID, sConnect)
{
   std::map<std::string, std::string> pConnect;

   pConnect = GetConnectInfo (NetSettings.sHost, NetSettings.wPort, NetSettings.bSecure);

   NetSettings.sSession = pConnect["session"];
}

SERVICE::IREFERENCE::~IREFERENCE ()
{
}

std::string SERVICE::IREFERENCE::Key ()
{
   return std::string (NetSettings.bSecure ? "true" : "false") + ";" + NetSettings.sHost + ";" + std::to_string (NetSettings.wPort) + ";" + NetSettings.sSession;
}

RMAP::CORE::SERVICE* SERVICE::IREFERENCE::Create (RMAP::CORE::NAMESPACE* pNamespace)
{
   return new SERVICE (this, pNamespace);
}

/*******************************************************************************************************************************
**                                                     CLASS (Factory)                                                        **
*******************************************************************************************************************************/

SERVICE::FACTORY::FACTORY (std::string sID) :
   RMAP::CORE::SERVICE::FACTORY (sID)
{
}

SERVICE::FACTORY::~FACTORY ()
{
}

RMAP::CORE::SERVICE::IREFERENCE* SERVICE::FACTORY::Reference (std::string sConnect)
{
   return new IREFERENCE (sID (), sConnect);
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class SERVICE::Impl
{
public:
   Impl (IREFERENCE* pReference)
   {
      NetSettings = pReference->NetSettings;

      tmServer_Current = 0;
      tmServer_Last = 0;

      Delta.bInitialize    = true;
      Delta.txServer       = 0;     // current calculated delta
      Delta.nTotal         = 0;     // accumulated time (in time ticks) of deltas from server time
      Delta.nCount         = 0;     // accumulated number of deltas from server time

      Latency.bInitialize  = true;
      Latency.txServer     = 2;     // current calculated latency (initially set to 31.25ms)
      Latency.nTotal       = 0;     // accumulated time (in milliseconds) of actions sent to the server
      Latency.nCount       = 0;     // accumulated number of actions sent to the server
   }

   ~Impl ()
   {
   }

   NETSETTINGS NetSettings;
   TIME        tmServer_Current;
   TIME        tmServer_Last;

   SERVICE_TIMEDATA   Delta;
   SERVICE_TIMEDATA   Latency;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE)                                                        **
*******************************************************************************************************************************/

SERVICE::SERVICE (IREFERENCE* pReference, RMAP::CORE::NAMESPACE* pNamespace) :
   RMAP::CORE::SERVICE (pReference, pNamespace)
{
   m_pImpl = new Impl (pReference);
}

SERVICE::~SERVICE ()
{
   delete m_pImpl;
}

SERVICE::FACTORY* SERVICE::factory ()
{
   return new FACTORY ("Statabase");
}

SERVICE::NETSETTINGS* SERVICE::pNetSettings () 
{ 
   return &m_pImpl->NetSettings; 
}

// ===== Public Methods =====================================================================================================

RMAP::CORE::CLIENT* SERVICE::Client_Open (uint64_t twClientIx)
{
   return RMAP::CORE::SERVICE::Client_Open (CLIENT::Reference (twClientIx));
}

std::string SERVICE::GetSessionString ()
{
   return m_pImpl->NetSettings.sSession;
}

// ==========================================================================================================================

bool SERVICE::Connected (CLIENT* pClient)
{
   NOTIFYPARAM np;

   np.nConnected = eCLIENT::CONNECTED;
   np.pClient    = pClient;
   Emit ("onClient", &np);

   return true;
}

void SERVICE::Disconnected (CLIENT* pClient)
{
   NOTIFYPARAM np;

   np.nConnected = eCLIENT::DISCONNECTED;
   np.pClient = pClient;
   Emit ("onClient", &np);
}

// ==========================================================================================================================

TIME SERVICE::Time_Server ()
{
   return m_pImpl->tmServer_Current;
}

void SERVICE::Time_Server (TIME tmServer_Current)
{
   m_pImpl->tmServer_Current = tmServer_Current;
}

TIME SERVICE::Time_ServerLast ()
{
   return m_pImpl->tmServer_Last;
}

void SERVICE::Time_ServerLast (TIME tmServer_Last)
{
   m_pImpl->tmServer_Last = tmServer_Last;
}

void SERVICE::Time_Latency (int nMilliseconds)
{
   // the latency between the client and server is half the average round trip time

   m_pImpl->Latency.nTotal += nMilliseconds;
   m_pImpl->Latency.nCount += 1;
}

void SERVICE::Time_Sync (TIME tmServer)
{
   tmServer += m_pImpl->Latency.txServer;

   TIME tmSystem_Current = g_pTime->Current ();

   TIMEX txDelta = (TIMEX)(tmServer - tmSystem_Current);

   m_pImpl->Delta.nTotal += txDelta;
   m_pImpl->Delta.nCount += 1;

   if (m_pImpl->Delta.bInitialize)
   {
      m_pImpl->Delta.txServer = txDelta;
      m_pImpl->Delta.bInitialize = false;

      m_pImpl->tmServer_Current = tmSystem_Current + m_pImpl->Delta.txServer;
   }
}

bool SERVICE::Exec ()
{
   return true;
}

bool SERVICE::PreKill ()
{
   Client_Enum (CE_PreKill, NULL);

   return SafeKill ();
}

bool SERVICE::SafeKill ()
{
   bool bResult = false;

   RMAP::CORE::CLIENT* pClient = Client_Enum (CE_SafeKill, NULL);

   if (pClient != NULL)
   {
      Client_Release ();
   }
   else bResult = true;

   return bResult;
}

bool SERVICE::Kill ()
{
   return false;
}

void SERVICE::GetTimeData (SERVICE_TIMEDATA* pTimeData, eTIMEDATA eType)
{
   if (eType == DELTA)
      *pTimeData = m_pImpl->Delta;
   else
      *pTimeData = m_pImpl->Latency;
}

void SERVICE::SetTimeData (SERVICE_TIMEDATA* pTimeData, eTIMEDATA eType)
{
   if (eType == DELTA)
      m_pImpl->Delta = *pTimeData;
   else
      m_pImpl->Latency = *pTimeData;
}

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE::ITIME)                                                 **
*******************************************************************************************************************************/

typedef struct
{
   int                                uCode;
   TIME                               tmServer_Current;
}
CLIENTENUM_DATA;

bool SBClient_Enum (RMAP::CORE::CLIENT* pClient, void* pvParam)
{
   CLIENT* pClientSB = dynamic_cast<CLIENT*> (pClient);
   CLIENTENUM_DATA* pData = (CLIENTENUM_DATA*)pvParam;

   pClientSB->Tick (pData->uCode, pData->tmServer_Current);

   return true;
}

bool SE_Tick (RMAP::CORE::SERVICE* pService, void* pParam)
{
   int tmSystem_Current = *((int *)pParam);
   double txAverage;
   SERVICE::SERVICE_TIMEDATA Latency, Delta;
   SERVICE* pServiceSB = dynamic_cast<SERVICE*> (pService);
   CLIENTENUM_DATA Data;

   pServiceSB->GetTimeData (&Latency, SERVICE::LATENCY);
   pServiceSB->GetTimeData (&Delta, SERVICE::DELTA);

   // adjust the latency every 256 samples, or continuously until the latency has been initialized (minimum 4 samples)

   if (Latency.nCount >= 256 || (Latency.nCount >= 4 && Latency.bInitialize))
   {
      txAverage = Latency.nTotal / Latency.nCount;
      txAverage *= 64;
      txAverage /= 1000;
      txAverage /= 2;          // one way latency is half the rount trip

      if (Latency.bInitialize)
      {
         Latency.txServer = (int)round (txAverage);

         if (Latency.nCount >= 256)
            Latency.bInitialize = false;
      }
      else
      {
         Latency.txServer = (int)round ((3 * (double)Latency.txServer + txAverage) / 4);

         Latency.nTotal = 0;
         Latency.nCount = 0;
      }
   }

   // adjust the delta every 256 samples, or continuously until the latency has been initialized (minimum 4 samples)

   if (Delta.nCount >= 256 || (Delta.nCount >= 4 && Latency.bInitialize))
   {
      txAverage = Delta.nTotal / Delta.nCount;

      if (Latency.bInitialize)
      {
         Delta.txServer = (int)round (txAverage);
      }
      else
      {
         Delta.txServer = (int)round ((3 * Delta.txServer + txAverage) / 4);

         Delta.nTotal = 0;
         Delta.nCount = 0;
      }
   }

   Data.tmServer_Current = pServiceSB->Time_Server ();

   // don't allow the time to adjust backward

   if (Data.tmServer_Current < tmSystem_Current + Delta.txServer)
   {
      tmSystem_Current += Delta.txServer;
      pServiceSB->Time_Server (tmSystem_Current);

      TIME tmServer_Last = pServiceSB->Time_ServerLast ();

      if ((tmServer_Last / TIMEX_SECOND) != (Data.tmServer_Current / TIMEX_SECOND))
      {
         if ((tmServer_Last / TIMEX_MINUTE) != (Data.tmServer_Current / TIMEX_MINUTE))
         {
            if ((tmServer_Last / TIMEX_HOUR) != (Data.tmServer_Current / TIMEX_HOUR))
            {
               if ((tmServer_Last / TIMEX_DAY) != (Data.tmServer_Current / TIMEX_DAY))
               {
                  Data.uCode = SERVICE::eTIME::DAY;
               }
               else Data.uCode = SERVICE::eTIME::HOUR;
            }
            else Data.uCode = SERVICE::eTIME::MINUTE;
         }
         else Data.uCode = SERVICE::eTIME::SECOND;
      }
      else Data.uCode = SERVICE::eTIME::X64TH;

      pServiceSB->Client_Enum (SBClient_Enum, &Data);

      pServiceSB->Time_ServerLast (Data.tmServer_Current);
   }

   return true;
}

bool NS_Tick (RMAP::CORE::NAMESPACE* pNamespace, void* pParam)
{
   pNamespace->Service_Enum ("Statabase", SE_Tick, pParam);

   return true;
}

void ITIME_SERVICE::Tick (TIME tmSystem_Current)
{
   RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();

   pCore->Namespace_Enum (NS_Tick, &tmSystem_Current);
}

/******************************************************************************************************************************/
