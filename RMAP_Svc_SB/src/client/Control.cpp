/*******************************************************************************************************************************
**                                                                                                                            **
**                                               MVSB_cpp : Control.cpp                                                       **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

using namespace RMAP::SVC_SB;

static const CLIENT::ACTION SBA_CLIENT_CONNECTX
(
   SBA_CLIENT_CONNECT, 
   "{"
      "\"pFingerprint\"                :"
      "{"
         "\"bBrowser_Brand\"           :  \"BYTE\","
         "\"abReserved_A\"             :  \"PAD (1)\","
         "\"asBrowser_Version\"        :  [ \"SHORT\", 4],"

         "\"bSystem_Brand\"            :  \"BYTE\","
         "\"bSystem_Product\"          :  \"BYTE\","
         "\"bSystem_Type\"             :  \"BYTE\","
         "\"abReserved_B\"             :  \"PAD (1)\","
         "\"asSystem_Version\"         :  [ \"SHORT\", 2],"

         "\"wScreen_Pixel_Width\"      :  \"WORD\","
         "\"wScreen_Pixel_Height\"     :  \"WORD\","
         "\"wScreen_Pixel_Depth\"      :  \"WORD\","

         "\"dwHash_Fonts\"             :  \"DWORD\","
         "\"dwHash_Plugins\"           :  \"DWORD\""
      "},"

      "\"acToken64U_Device\"           :  \"STRING (64)\""
   "}",
   "{"
      "\"qwClientSessionIx\"           : \"QWORD\","

   // server-specific custom data
      "\"abCustom\"                    : [ \"BYTE\", 16 ],"
   // "\"bProduction\"                 :   \"BYTE\","                      // This information is not officially part of the client_connect_out, 
   // "\"abReserved_A\"                :   \"PAD    (7)\","                // and is not guaranteed to be consistent from one client to another. 
   // "\"sGeoRegionIx\"                :   \"STRING (2)\","                // Additionally, clients-side models must be agnostic to the particulars 
   // "\"sGeoStateIx\"                 :   \"STRING (2)\","                // of any implementation, and are not permitted to rely on the format of 
   // "\"bRegion\"                     :   \"BYTE\","                      // data in this section. Clients are advised to use this data at their 
   // "\"abReserved_B\"                :   \"PAD    (3)\","                // own risk.

      "\"acToken64U_Device\"           : \"STRING (" TO_STRING (SBD_SIZE_TOKEN64U) ")\""
   "}",
   true
);

static const CLIENT::ACTION SBA_CLIENT_DISCONNECTX
(
   SBA_CLIENT_DISCONNECT,
   "{"
   "}",
   "",
   true
);

static const std::map<std::string, const CLIENT::ACTION*> g_aAction_Control =
{
   { "CONNECT",      &SBA_CLIENT_CONNECTX    },
   { "DISCONNECT",   &SBA_CLIENT_DISCONNECTX },
};

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class CONTROL::Impl : public NET::ICONTROL, public RMAP::CORE::IRESPONSE
{
public:
   Impl (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription) :
      pClient (pClient),
      pSubscription (pSubscription),
      m_nTimeout (10),
      m_bNetConnected (false),
      m_bSocketConnected (false),
      m_wAgent (0),
      m_bShutdown (false),
      m_wControl (0)
   {
      // Control
      bError   = false;
      dwResult  = SBA_RESULT_SUCCESS;

      // Connection
      m_bSecure = pNetSettings->bSecure;
      m_sHost   = pNetSettings->sHost;
      m_wPort   = pNetSettings->wPort;

      // Session
      qwClientSessionIx    = 0;
   // bProduction          = false;
      bSystemConnected     = false;

      nReadyState_Attempt = -1;

      // Login
      pSourceSB            = NULL;
      pParams              = NULL;
      pLogin               = NULL;
      bLoggedIn            = false;

      m_pThread = new std::thread (&CONTROL::Impl::ThreadLoop, this);
   }

   ~Impl ()
   {
      Shutdown ();
      m_pThread->join ();

      delete m_pThread;
   }

   void Shutdown ()
   {
      std::lock_guard<std::mutex> guard (m_mutex);
      m_bShutdown = true;
      m_condVar.notify_all ();
   }

   void ThreadLoop ()
   {
      std::unique_lock<std::mutex> mlock (m_mutex);
      m_condVar.wait (mlock, std::bind (&CONTROL::Impl::Control, this));
   }

   bool Control ()
   {
      bool bExit = false;

      if (m_bShutdown == false)
      {
         if (Control_Acquire () != false)
         {
            if ((m_wAgent & eCONTROL::SOCKETDISCONNECTED) != 0)
            {
               if (m_bNetConnected != false)
               {
                  // Logout and disconnect may be called safely for involuntary logouts. If the user is not logged in or connected, these functions will do nothing.

                  ClearError ();

                  bool bVoluntary = (m_wAgent & eCONTROL::SOCKETDISCONNECTED_VOLUNTARY) ? true : false;

                  pSourceSB = dynamic_cast<SB_SESSION *> (pClient->SourceGet ());
                  pParams   = NULL;

                  Logout_Request (bVoluntary, true);
                  SystemDisconnect_Request (bVoluntary, true);
                  bExit = SocketDisconnect_Attempt (bVoluntary, true);
               }
            }
            else
            {
               switch (m_wAgent)
               {
               case eCONTROL::SOCKETCONNECT:    bExit = SocketConnect_Attempt (true);           break;
               case eCONTROL::SOCKETDISCONNECT: bExit = SocketDisconnect_Attempt (true, false); break;
               case eCONTROL::SYSTEMCONNECT:    bExit = SystemConnect_Request (true);           break;
               case eCONTROL::SYSTEMDISCONNECT: bExit = SystemDisconnect_Request (true, false); break;
               case eCONTROL::LOGIN:            bExit = Login_Request (true);                   break;
               case eCONTROL::LOGOUT:           bExit = Logout_Request (true, false);           break;
               }
            }

            if (bExit == false)
            {
               Control_Release ();
            }
         }
      }

      return m_bShutdown;
   }

   int ReadyState ()
   {
      return pClient->ReadyState ();
   }

   int ReadyState (int nState)
   {
      return pClient->ReadyState (nState);
   }

   bool Control_Acquire ()
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if (m_wAgent == 0 && m_wControl != 0)
         {
            m_wAgent = m_wControl;

            bResult = true;
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   void Control_Release ()
   {
      bool bBreak = false;

      m_CS_Control.lock ();
      {
         m_wControl ^= m_wAgent;

         m_wAgent = 0;

         if (nReadyState_Attempt >= 0)
         {
            SB_SESSION* pSource = dynamic_cast<SB_SESSION*> (pClient->SourceGet ());

            pSource->Attempt (nReadyState_Attempt);

            nReadyState_Attempt = -1;
         }

         if (m_wControl != 0)
         {
            bBreak = true;
         }
      }
      m_CS_Control.unlock ();

      if (bBreak)
         CtlBreak_Thread ();
   }

   void SetAttemptState (int nReadyState)
   {
      m_CS_Control.lock ();
      {
         nReadyState_Attempt = nReadyState;
      }
      m_CS_Control.unlock ();
   }

   bool ClearError ()
   {
      bool bResult = bError;

      bError = false;

      return bResult;
   }

   bool SafeKill ()
   {
      bool bResult;

      m_CS_Control.lock ();
      {
         bResult = (m_bNetConnected == false && m_wControl == 0);
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   void SystemConnect_Exit (CLIENT::IACTION* pIActionSB, bool bVoluntary)
   {
      SB_PROGRESS ProgressSB;

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         ordered_json jRsp = pIActionSB->GetResponse ();

         qwClientSessionIx = jRsp["qwClientSessionIx"];
         acToken64U_Device = jRsp["acToken64U_Device"];
      // bProduction       = (jRsp["bProduction"] != 0);

         bSystemConnected = true;

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);

         if (bVoluntary == false && bLoggedIn == false)
            pSubscription->Subscribe_Aux ();
      }
      else
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::SYSTEMDISCONNECTED);
      }

      ProgressSB.nProgress          = CLIENT::ePROGRESS::SYSTEMCONNECT_RESULT;
      ProgressSB.bVoluntary         = bVoluntary;
      ProgressSB.dwResult           = dwResult;
      ProgressSB.acToken64U_Device  = acToken64U_Device;
      ProgressSB.bResult            = !bError;

      Progress (&ProgressSB);
   }

   void SystemDisconnect_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
   {
      SB_PROGRESS ProgressSB;

      dwResult = SBA_RESULT_SUCCESS;  // There"s no such thing as a failure to disconnect!

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         bSystemConnected = (bSystemConnected != false && bVoluntary == false);

         qwClientSessionIx = SBD_CLIENTSESSION_NULL;

         ReadyState (CLIENT::eSTATE::SYSTEMDISCONNECTED);

         pSubscription->Disconnected (bVoluntary, bDisconnected);
      }
      else // This will never occur
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }

      ProgressSB.nProgress          = CLIENT::ePROGRESS::SYSTEMDISCONNECT_RESULT;
      ProgressSB.bVoluntary         = bVoluntary;
      ProgressSB.bDisconnected      = bDisconnected;
      ProgressSB.dwResult           = dwResult;
      ProgressSB.bResult            = !bError;

      Progress (&ProgressSB);
   }

   void Login_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary)
   {
      SB_PROGRESS ProgressSB;

      if (pIAction && dwResult == SBA_RESULT_SUCCESS)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceSB->Login_Create ();
         }

         // A dwResult of SBA_RESULT_SUCCESS only tells us that the action completed without error.
         // We have to wait for the source to tell us whether or not the login completed successfully.

         if (pSourceSB->Login_Response (pParams, pLogin, pIAction, bVoluntary) != false)
         {
            bLoggedIn = true;

            ReadyState (CLIENT::eSTATE::LOGGEDIN);

            if (bVoluntary == false)
               pSubscription->Subscribe_Aux ();
         }
         else
         {
            bError = true;

            ReadyState (CLIENT::eSTATE::LOGGEDOUT);
         }
      }
      else
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }

      ProgressSB.nProgress       = CLIENT::ePROGRESS::LOGIN_RESULT;
      ProgressSB.bVoluntary      = bVoluntary;
      ProgressSB.dwResult        = dwResult;
      ProgressSB.pLogin          = pLogin;
      ProgressSB.bResult         = !bError;

      Progress (&ProgressSB);

      if (bError != false && pLogin)
      {
         if (bLoggedIn == false)
         {
            pLogin = pSourceSB->Login_Destroy (pLogin);
         }
      }

      pSourceSB = NULL;
      pParams   = NULL;
   }

   void Logout_Exit (RMAP::CORE::CLIENT::IACTION* pIAction, bool bVoluntary, bool bDisconnected)
   {
      SB_PROGRESS ProgressSB;

      dwResult = SBA_RESULT_SUCCESS;  // There"s no such thing as a failure to logout!

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         if (pIAction)
         {
            pSourceSB->Logout_Response (pParams, pLogin, pIAction, bVoluntary, bDisconnected);
         }

         bLoggedIn = (bLoggedIn != false && bVoluntary == false);

         if (bLoggedIn == false)
         {
            pLogin = pSourceSB->Login_Destroy (pLogin);
         }

         ReadyState (CLIENT::eSTATE::LOGGEDOUT);
      }
      else // This will never occur
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::LOGGEDIN);
      }

      ProgressSB.nProgress       = CLIENT::ePROGRESS::LOGOUT_RESULT;
      ProgressSB.bVoluntary      = bVoluntary;
      ProgressSB.bDisconnected   = bDisconnected;
      ProgressSB.dwResult        = dwResult;
      ProgressSB.pLogin          = NULL;
      ProgressSB.bResult         = !bError;

      Progress (&ProgressSB);

      pSourceSB = NULL;
      pParams   = NULL;
   }

   /*******************************************************************************************************************************
   **                                                   Client                                                                   **
   *******************************************************************************************************************************/

   bool SocketConnect ()
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & eCONTROL::SOCKETCONNECT) == 0)
         {
            if (ReadyState () == CLIENT::eSTATE::SOCKETDISCONNECTED)
            {
               m_wControl |= eCONTROL::SOCKETCONNECT;

               CtlBreak_Thread ();

               bResult = true;
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool SocketDisconnect ()
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & eCONTROL::SOCKETDISCONNECT) == 0)
         {
            if ((ReadyState () == CLIENT::eSTATE::SYSTEMDISCONNECTED && (m_wControl & eCONTROL::SYSTEMCONNECT) == 0)
               || (ReadyState () == CLIENT::eSTATE::SYSTEMDISCONNECTED && ((m_wControl ^ m_wAgent) & eCONTROL::SYSTEMCONNECT) != 0))
            {
               m_wControl &= ~eCONTROL::SYSTEMCONNECT;
               m_wControl |= eCONTROL::SOCKETDISCONNECT;

               CtlBreak_Thread ();

               bResult = true;
            }
            else if (ReadyState () == CLIENT::eSTATE::SOCKETDISCONNECTED && ((m_wControl ^ m_wAgent) & eCONTROL::SOCKETCONNECT) != 0)
            {
               m_wControl &= ~eCONTROL::SOCKETCONNECT;

               bResult = true;
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool SocketDisconnected (bool bVoluntary)
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & eCONTROL::SOCKETDISCONNECTED) == 0)
         {
            if (ReadyState () > CLIENT::eSTATE::SOCKETDISCONNECTED)
            {
               m_wControl |= eCONTROL::SOCKETDISCONNECTED | (bVoluntary ? eCONTROL::SOCKETDISCONNECTED_VOLUNTARY : 0);

               CtlBreak_Thread ();

               bResult = true;
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool SystemConnect ()
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & (eCONTROL::SYSTEMCONNECT | eCONTROL::SOCKETDISCONNECT)) == 0)
         {
            if (ReadyState () == CLIENT::eSTATE::SYSTEMDISCONNECTED)
            {
               if (bSystemConnected == false)
               {
                  bResult = true;
               }
               else bResult = (bSystemConnected != false);

               if (bResult != false)
               {
                  m_wControl |= eCONTROL::SYSTEMCONNECT;

                  CtlBreak_Thread ();

                  bResult = true;
               }
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool SystemDisconnect ()
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & eCONTROL::SYSTEMDISCONNECT) == 0)
         {
            if ((ReadyState () == CLIENT::eSTATE::LOGGEDOUT && (m_wControl & eCONTROL::LOGIN) == 0)
               || (ReadyState () == CLIENT::eSTATE::LOGGEDOUT && ((m_wControl ^ m_wAgent) & eCONTROL::LOGIN) != 0))
            {
               m_wControl &= ~eCONTROL::LOGIN;
               m_wControl |= eCONTROL::SYSTEMDISCONNECT;

               CtlBreak_Thread ();

               bResult = true;
            }
            else if (ReadyState () == eCONTROL::SYSTEMCONNECT && ((m_wControl ^ m_wAgent) & eCONTROL::SYSTEMCONNECT) != 0)
            {
               m_wControl &= ~eCONTROL::SYSTEMCONNECT;

               bResult = true;
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool Login (RMAP::CORE::SOURCE* pSource, void* pvParams)
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & (eCONTROL::LOGIN | eCONTROL::SYSTEMDISCONNECT)) == 0)
         {
            if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
            {
               if (bLoggedIn == false && pvParams != NULL
                  || bLoggedIn != false && pLogin != NULL)
               {
                  pSourceSB = dynamic_cast<SB_SESSION*> (pSource);
                  pParams = pvParams;

                  m_wControl |= eCONTROL::LOGIN;

                  CtlBreak_Thread ();

                  bResult = true;
               }
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   bool Logout (RMAP::CORE::SOURCE* pSource, void* pvParams)
   {
      bool bResult = false;

      m_CS_Control.lock ();
      {
         if ((m_wControl & eCONTROL::LOGOUT) == 0)
         {
            if (ReadyState () == CLIENT::eSTATE::LOGGEDIN)
            {
               pSourceSB = dynamic_cast<SB_SESSION*> (pSource);
               pParams = pvParams;

               m_wControl |= eCONTROL::LOGOUT;

               CtlBreak_Thread ();

               bResult = true;
            }
            else if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT && ((m_wControl ^ m_wAgent) & eCONTROL::LOGIN) != 0)
            {
               pSourceSB = NULL;
               pParams = NULL;

               m_wControl &= ~eCONTROL::LOGIN;
   //TODO:: WAKEUP THREAD???
               bResult = true;
            }
         }
      }
      m_CS_Control.unlock ();

      return bResult;
   }

   /*******************************************************************************************************************************
   **                                                   ICONTROL                                                                 **
   *******************************************************************************************************************************/

   void SocketConnect_Complete (int pVD) override
   {
      bool bVoluntary = GetVDParam (pVD, MVSB_VDPARAM_VOLUNTARY);
      bool bConnected = GetVDParam (pVD, MVSB_VDPARAM_CONNECTED);

      if (bConnected != false)
      {
         dwResult = SBA_RESULT_SUCCESS;
      }
      else dwResult = SBA_RESULT_VERSION_SERVERUNAVAILABLE;   /// wrong error

      SocketConnect_Exit (bConnected, bVoluntary);

      Control_Release ();
   }

   void SocketDisconnect_Complete (int pVD) override
   {
      bool bVoluntary = GetVDParam (pVD, MVSB_VDPARAM_VOLUNTARY);
      bool bConnected = GetVDParam (pVD, MVSB_VDPARAM_CONNECTED);
      bool bDisconnected = GetVDParam (pVD, MVSB_VDPARAM_DISCONNECTED);

      dwResult = SBA_RESULT_SUCCESS;

      SocketDisconnect_Exit (bConnected, bVoluntary, bDisconnected);

      Control_Release ();
   }

   void onResponse (RMAP::CORE::CLIENT::IACTION* pIAction, int nType, intptr_t pParam) override
   {
      CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

      switch (nType)
      {
      case kSYSTEMCONNECT_RESPONSE:        SystemConnect_Response    (pIActionSB, GetVDParam (pParam, MVSB_VDPARAM_VOLUNTARY)); break;
      case kSYSTEMDISCONNECT_RESPONSE:     SystemDisconnect_Response (pIActionSB, pParam);                                      break;
      case kLOGIN_RESPONSE:                Login_Response            (pIActionSB, GetVDParam (pParam, MVSB_VDPARAM_VOLUNTARY)); break;
      case kLOGOUT_RESPONSE:               Logout_Response           (pIActionSB, pParam);                                      break;
      }
   }

public:

   CLIENT*           pClient;
   SUBSCRIPTION*     pSubscription;

   // Control
   bool              bError;
   uint32_t          dwResult;

   bool              m_bNetConnected;

   int               nReadyState_Attempt;

   // Session
   std::string       acToken64U_Device;
   uint64_t          qwClientSessionIx;
// bool              bProduction;
   bool              bSystemConnected;

   // Login
   SB_SESSION*                      pSourceSB;
   void*                            pParams;
   RMAP::CORE::SOURCE_SESSION::LOGIN*  pLogin;
   bool                             bLoggedIn;

private:
   void CtlBreak_Thread ()
   {
      std::lock_guard<std::mutex> guard (m_mutex);
      m_condVar.notify_all ();
   }

   bool SocketConnect_Attempt (bool bVoluntary)
   {
      bool bExit = false;
      SB_PROGRESS ProgressSB;

      bVoluntary = (m_bSocketConnected == false);

      if (ReadyState () == CLIENT::eSTATE::SOCKETDISCONNECTED)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::SOCKETCONNECTING);

         ProgressSB.nProgress = CLIENT::ePROGRESS::SOCKETCONNECT_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;

         Progress (&ProgressSB);

         if (pClient->pNet ()->Connect (m_bSecure, m_sHost, m_wPort, this, SetVDParam (bVoluntary, false, false), m_nTimeout) != false)
         {
            bExit = true;
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE;

         if (bExit == false)
         {
            SocketConnect_Exit (false, bVoluntary);
         }
      }

      return bExit;
   }

   void SocketConnect_Exit (bool bVoluntary, bool bDisconnected)
   {
      SB_PROGRESS ProgressSB;

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         m_bNetConnected = true;

         m_bSocketConnected = true;

         ReadyState (CLIENT::eSTATE::SYSTEMDISCONNECTED);
      }
      else
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::SOCKETDISCONNECTED);
      }

      ProgressSB.nProgress    = CLIENT::ePROGRESS::SOCKETCONNECT_RESULT;
      ProgressSB.bVoluntary   = bVoluntary;
      ProgressSB.dwResult     = dwResult;
      ProgressSB.bResult      = !bError;

      Progress (&ProgressSB);
   }

   void SocketDisconnect_Exit (bool bConnected, bool bVoluntary, bool bDisconnected)
   {
      SB_PROGRESS ProgressSB;

      dwResult = SBA_RESULT_SUCCESS;  // There"s no such thing as a failure to disconnect!

      if (dwResult == SBA_RESULT_SUCCESS)
      {
         m_bNetConnected = false;

         m_bSocketConnected = (m_bSocketConnected != false && bVoluntary == false);

         ReadyState (CLIENT::eSTATE::SOCKETDISCONNECTED);
      }
      else // This will never occur
      {
         bError = true;

         ReadyState (CLIENT::eSTATE::SYSTEMDISCONNECTED);
      }

      ProgressSB.nProgress       = CLIENT::ePROGRESS::SOCKETDISCONNECT_RESULT;
      ProgressSB.bVoluntary      = bVoluntary;
      ProgressSB.bDisconnected   = bDisconnected;
      ProgressSB.dwResult        = dwResult;
      ProgressSB.bResult         = !bError;

      Progress (&ProgressSB);
   }

   bool SocketDisconnect_Attempt (bool bVoluntary, bool bDisconnected)
   {
      bool bExit = false;
      SB_PROGRESS ProgressSB;

      if (ReadyState () == CLIENT::eSTATE::SYSTEMDISCONNECTED)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::SOCKETCONNECTING); // SOCKETDISCONNECTING

         ProgressSB.nProgress = CLIENT::ePROGRESS::SOCKETDISCONNECT_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;
         ProgressSB.bDisconnected = bDisconnected;

         Progress (&ProgressSB);

         if (pClient->pNet ()->Disconnect (this, bVoluntary, bDisconnected, m_nTimeout) != false)
         {
            bExit = true;
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE;

         if (bExit == false)
         {
            SocketDisconnect_Exit (false, bVoluntary, bDisconnected);
         }
      }

      return bExit;
   }

   bool SystemConnect_Request (bool bVoluntary)
   {
      bool bExit = false;
      SB_PROGRESS ProgressSB;

      bVoluntary = (bSystemConnected == false);

      if (ReadyState () == CLIENT::eSTATE::SYSTEMDISCONNECTED)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::SYSTEMCONNECTING);

         ProgressSB.nProgress = CLIENT::ePROGRESS::SYSTEMCONNECT_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;

         Progress (&ProgressSB);

         RMAP::CORE::CLIENT::IACTION* pIAction = Request ("CONNECT");
         CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

         ordered_json& pRequest = pIActionSB->GetRequest ();

         pRequest["acToken64U_Device"] = acToken64U_Device;

         pRequest["dwVersion"]      = 0;

         pRequest["pFingerprint"]["bBrowser_Brand"]         = 255;

         pRequest["pFingerprint"]["asBrowser_Version"][0]   = 0;
         pRequest["pFingerprint"]["asBrowser_Version"][1]   = 0;
         pRequest["pFingerprint"]["asBrowser_Version"][2]   = 0;
         pRequest["pFingerprint"]["asBrowser_Version"][3]   = 0;
                                                         
         pRequest["pFingerprint"]["bSystem_Brand"]          = 255;
         pRequest["pFingerprint"]["bSystem_Product"]        = 255;
         pRequest["pFingerprint"]["bSystem_Type"]           = 255;
         pRequest["pFingerprint"]["asSystem_Version"][0]    = 0;
         pRequest["pFingerprint"]["asSystem_Version"][1]    = 0;
                                                         
         pRequest["pFingerprint"]["wScreen_Pixel_Width"]    = 0;
         pRequest["pFingerprint"]["wScreen_Pixel_Height"]   = 0;
         pRequest["pFingerprint"]["wScreen_Pixel_Depth"]    = 0;

         pRequest["pFingerprint"]["dwHash_Fonts"]           = 0;
         pRequest["pFingerprint"]["dwHash_Plugins"]         = 0;

         pRequest["pFingerprint"]["dwHash_Canvas"]          = 0;
         pRequest["pFingerprint"]["dwHash_UserAgent"]       = 0;

         if (pIActionSB->Send (this, CONTROL::kSYSTEMCONNECT_RESPONSE, SetVDParam (bVoluntary, false, false)) != false)
         {
            bExit = true;
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE;

         if (bExit == false)
         {
            SystemConnect_Exit (NULL, bVoluntary);
         }
      }

      return bExit;
   }

   void SystemConnect_Response (CLIENT::IACTION* pIActionSB, bool bVoluntary)
   {
      dwResult = pIActionSB->GetResult ();

      SystemConnect_Exit (pIActionSB, bVoluntary);

      Control_Release ();
   }

   bool SystemDisconnect_Request (bool bVoluntary, bool bDisconnected)
   {
      bool bExit = false;
      SB_PROGRESS ProgressSB;

      // If the disconnect is involuntary, this function must complete all processing on the request and may not cause the current thread to exit.

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::SYSTEMCONNECTING); // SYSTEMDISCONNECTING

         ProgressSB.nProgress = CLIENT::ePROGRESS::SYSTEMDISCONNECT_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;
         ProgressSB.bDisconnected = bDisconnected;

         Progress (&ProgressSB);

         if (bVoluntary != false && !bDisconnected)
         {
            RMAP::CORE::CLIENT::IACTION* pIAction = Request ("DISCONNECT");
            CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

            ordered_json& pRequest = pIActionSB->GetRequest ();

            if (pIActionSB->Send (this, CONTROL::kSYSTEMCONNECT_RESPONSE, SetVDParam (bVoluntary, false, bDisconnected)) != false)
            {
               bExit = true;
            }
            else dwResult = SBA_RESULT_TRANSMITFAILURE;
         }
         else dwResult = SBA_RESULT_SUCCESS;

         if (bExit == false)
         {
            SystemDisconnect_Exit (NULL, bVoluntary, bDisconnected);
         }
      }

      return bExit;
   }

   void SystemDisconnect_Response (CLIENT::IACTION* pIActionSB, intptr_t pVD)
   {
      dwResult = pIActionSB->GetResult ();

      SystemDisconnect_Exit (pIActionSB, GetVDParam (pVD, MVSB_VDPARAM_VOLUNTARY), GetVDParam (pVD, MVSB_VDPARAM_DISCONNECTED));

      Control_Release ();
   }

   bool Login_Request (bool bVoluntary)
   {
      bool bExit = false;
      SB_PROGRESS ProgressSB;

      bVoluntary = (bLoggedIn == false);

      if (ReadyState () == CLIENT::eSTATE::LOGGEDOUT)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressSB.nProgress = CLIENT::ePROGRESS::LOGIN_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;

         Progress (&ProgressSB);

         RMAP::CORE::CLIENT::IACTION* pIAction = pSourceSB->Login_Request (pParams, pLogin);
         if (pIAction != NULL)
         {
            CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIActionSB->Send (this, CONTROL::kLOGIN_RESPONSE, SetVDParam (bVoluntary, false, false)) != false)
            {
               bExit = true;
            }
            else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR
         }
         else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR

         if (bExit == false)
         {
            Login_Exit (NULL, bVoluntary);
         }
      }
      else
      {
         pSourceSB = NULL;
         pParams = NULL;
      }

      return bExit;
   }

   void Login_Response (CLIENT::IACTION* pIActionSB, bool bVoluntary)
   {
      dwResult = pIActionSB->GetResult ();

      Login_Exit (pIActionSB, bVoluntary);

      Control_Release ();
   }

   // --------------------------------------------------------------------------------------------------------------------------

   bool Logout_Request (bool bVoluntary, bool bDisconnected)
   {
      SB_PROGRESS ProgressSB;
      bool bExit = false;

      // If the logout is involuntary, this function must complete all processing on the request and may not cause the current thread to exit.

      if (ReadyState () == CLIENT::eSTATE::LOGGEDIN)
      {
         ClearError ();

         ReadyState (CLIENT::eSTATE::LOGGING);

         ProgressSB.nProgress = CLIENT::ePROGRESS::LOGOUT_ATTEMPT;
         ProgressSB.bVoluntary = bVoluntary;
         ProgressSB.bDisconnected = bDisconnected;

         Progress (&ProgressSB);

         if (bVoluntary != false && !bDisconnected)
         {
            RMAP::CORE::CLIENT::IACTION* pIAction = pSourceSB->Logout_Request (pParams, pLogin);
            CLIENT::IACTION* pIActionSB = dynamic_cast<CLIENT::IACTION*> (pIAction);

            if (pIAction)
            {
               if (pIActionSB->Send (this, CONTROL::kLOGOUT_RESPONSE, SetVDParam (bVoluntary, false, bDisconnected)) != false)
               {
                  bExit = true;
               }
               else dwResult = SBA_RESULT_TRANSMITFAILURE;
            }
            else dwResult = SBA_RESULT_TRANSMITFAILURE; /// WRONG ERROR
         }
         else dwResult = SBA_RESULT_SUCCESS;

         if (bExit == false)
         {
            Logout_Exit (NULL, bVoluntary, bDisconnected);
         }
      }
      else
      {
         pSourceSB = NULL;
         pParams = NULL;
      }

      return bExit;
   }

   void Logout_Response (CLIENT::IACTION* pIActionSB, intptr_t pVD)
   {
      dwResult = pIActionSB->GetResult ();

      Logout_Exit (pIActionSB, GetVDParam (pVD, MVSB_VDPARAM_VOLUNTARY), GetVDParam (pVD, MVSB_VDPARAM_DISCONNECTED));

      Control_Release ();
   }

   RMAP::CORE::CLIENT::IACTION* Request (std::string sAction)
   {
      RMAP::CORE::CLIENT::IACTION* pResult;

      auto j = g_aAction_Control.find (sAction);

      if (j != g_aAction_Control.end ())
      {
         pResult = pClient->Request (const_cast<CLIENT::ACTION *> (j->second));
      }
      else pResult = NULL;

      return pResult;
   }

   void Progress (SB_PROGRESS* pProgress)
   {
      pClient->Progress (pProgress);
   }

private:
   std::thread*   m_pThread;

   std::mutex              m_mutex;
   std::condition_variable m_condVar;
   bool                    m_bShutdown;

   std::recursive_mutex m_CS_Control;
   int                  m_wAgent;
   int                  m_wControl;

   int                  m_nTimeout;

   bool                 m_bSocketConnected;

   bool                 m_bSecure;
   std::string          m_sHost;
   int                  m_wPort;
};

/*******************************************************************************************************************************
**                                                   CLASS (CONTROL)                                                          **
*******************************************************************************************************************************/

CONTROL::CONTROL (CLIENT* pClient, SERVICE::NETSETTINGS* pNetSettings, SUBSCRIPTION* pSubscription)
{
   m_pImpl = new CONTROL::Impl (pClient, pNetSettings, pSubscription);
}

CONTROL::~CONTROL ()
{
   delete m_pImpl;
}

void CONTROL::SetAttemptState (int nReadyState)
{
   m_pImpl->SetAttemptState (nReadyState);
}

bool CONTROL::bNetConnected ()
{
   return m_pImpl->m_bNetConnected;
}

bool CONTROL::bSystemConnected ()
{
   return m_pImpl->bSystemConnected;
}

bool CONTROL::bLoggedIn ()
{
   return m_pImpl->bLoggedIn;
}

RMAP::CORE::SOURCE_SESSION::LOGIN* CONTROL::pLogin ()
{
   return m_pImpl->pLogin;
}

bool CONTROL::SafeKill ()
{
   return m_pImpl->SafeKill ();
}

bool CONTROL::ClearError ()
{
   return m_pImpl->ClearError ();
}

bool CONTROL::SetDevice (std::string acToken64U_Device)
{
   if (acToken64U_Device.empty () == false)
      m_pImpl->acToken64U_Device = acToken64U_Device;

   return true;
}

bool CONTROL::SocketConnect ()                              { return m_pImpl->SocketConnect ();                }
bool CONTROL::SocketReconnect ()                            { return SocketConnect ();                         }
bool CONTROL::SocketDisconnect ()                           { return m_pImpl->SocketDisconnect ();             }
bool CONTROL::SocketDisconnected (bool bVoluntary)          { return m_pImpl->SocketDisconnected (bVoluntary); }
bool CONTROL::SystemConnect ()                              { return m_pImpl->SystemConnect ();                }
bool CONTROL::SystemReconnect ()                            { return SystemConnect ();                         }
bool CONTROL::SystemDisconnect ()                           { return m_pImpl->SystemDisconnect ();             }
bool CONTROL::Login (RMAP::CORE::SOURCE* pSource, void* pParams)  { return m_pImpl->Login (pSource, pParams);        }
bool CONTROL::Logout (RMAP::CORE::SOURCE* pSource, void* pParams) { return m_pImpl->Logout (pSource, pParams);       }

/******************************************************************************************************************************/
