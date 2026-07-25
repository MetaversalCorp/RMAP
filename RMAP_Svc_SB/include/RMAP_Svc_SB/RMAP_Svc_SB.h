/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_Svc_SB : RMAP_Svc_SB.h                                           **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_SVC_SB_H
#define RMAP_SVC_SB_H

namespace RMAP
{
   namespace SVC_SB
   {
      class BYTESTREAM
      {
      public:
         BYTESTREAM ();
         BYTESTREAM (std::vector<uint8_t> &abData);
         BYTESTREAM (uint8_t* pbData, size_t nLength);
         ~BYTESTREAM ();

         std::vector<uint8_t>& GetData ();
         void Resize (int nSize);
         void Reset ();

         int Size ();
         int  Offset ();
         bool EOS ();
         int Remaining ();
         int Inflate ();
         bool IsError ();

         void Write_String   (std::string  sValue, int nCount);
         void Write_String_W (std::wstring sValue, int nCount);
         void Write_Pad (int nBytes);
         void Write_Number (uint8_t* pbData, int nBytes);

         void Write_QWORD (uint64_t qwValue);
         void Write_TWORD (uint64_t twValue);
         void Write_DWORD (uint32_t dwValue);
         void Write_WORD (uint16_t wValue);

         uint64_t Read_QWORD ();
         uint64_t Read_TWORD ();
         uint64_t Read_TWORD8 ();
         uint32_t Read_DWORD ();
         uint16_t Read_WORD ();
         uint8_t  Read_BYTE ();
         uint8_t  Read_Pad (int nSize);

         uint32_t Read_TIMEX ();
         uint64_t Read_TIME ();
         uint64_t Read_EVENT ();

         std::string  Read_String (int nSize);
         std::wstring Read_StringW (int nSize);

         bool XCopy (std::vector<uint8_t>& abData, int origin, int bytes);
         bool Copy  (std::vector<uint8_t>& abData, int origin, int bytes);
         int  Seek (int bytes);

      private:
         void Write_Value (uint8_t* pbData, int nSize);
         void Read_Value (uint8_t* pbData, int nSize);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class MAP
      {
      public:
         MAP (std::string sMap);
         ~MAP ();

         ordered_json   GetRequest ();
         bool           Write (BYTESTREAM& BS, int wOffset_Base, ordered_json& jSrc);
         bool           Read (BYTESTREAM* pByteStream, ordered_json& jSrc);
         uint16_t       Size (ordered_json& jSrc);
         uint16_t       Size (bool bFull);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class CLIENT;

      class OBJECTHEAD : public RMAP::CORE::MEM::OBJECTHEAD
      {
      public:
         OBJECTHEAD ();
         OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags, uint64_t twEventIz);

         virtual ~OBJECTHEAD ();

         uint64_t twEventIz;
      };
      
      class xTIME
      {
      public:
         class ITIME
         {
         public:
            virtual void Tick (TIME tmSystem_Current) = 0;
         };

      public:
         xTIME (ITIME* pITime);
         virtual ~xTIME ();

         std::uint64_t ToPosixTime (TIME tmValue);
         TIME          FromPosixTime (std::uint64_t ptValue);

         // --------------------------------------------------------------------------------------------------------------------------

//         bool ToSystemTime (TIME tmValue, int* pdtResult);
//         int FromSystemTime (int dtValue);

         // --------------------------------------------------------------------------------------------------------------------------

         TIMEX ToTimex (TIME tmBase, TIME tmOffset);
         TIME  FromTimex (TIME tmBase, TIMEX txOffset);

         TIME ConvertTimex (TIME tmBaseTo, TIME tmBaseFrom, TIMEX txOffset);
         TIME Current ();

         TIMEX Currentx (TIME tmBase);

      private:
         bool onTimeout (bool bInterrupt);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class SB_OBJECT : public RMAP::CORE::MEM::SOURCE
      {
      public:
         class FACTORY : public RMAP::CORE::MEM::SOURCE::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction, bool bIndependent, MAP* pMap);
            virtual ~FACTORY ();

         protected:
            MAP*      m_pMap;
         };

      public:
         SB_OBJECT (RMAP::CORE::MEM::SOURCE::REFERENCE* pReference, MAP* pMap, RMAP::CORE::CLIENT* pClient);
         virtual ~SB_OBJECT ();

         // ===== Public Properties ==================================================================================================

         void twEventIz (uint64_t twEventIz);
         uint64_t twEventIz ();

         uint64_t twObjectIx ();
         uint64_t twParentIx ();

         void ResetData ();

         std::vector<uint8_t>& GetData ();

         virtual void Read (ordered_json& jSrc, RMAP::CORE::MODEL* pModel) = 0;

         // ===== Source Methods =====================================================================================================

         void Partial    () override;
         void Full       () override;
         void Recovering () override;
         void Recovered  () override;
         void Inserted   (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Deleting   (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Updating   (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild) override;
         void Updated    (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild) override;
         void Changing   (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;
         void Changed    (RMAP::CORE::MEM::SOURCE* pObject, RMAP::CORE::MEM::SOURCE* pChild, RMAP::CORE::MEM::CHANGE* pChange) override;

         bool Attach () override;
         bool Detach () override;

      public:
         void Map_Read (RMAP::CORE::MEM::MODEL* pModel);
         void Map_Write (BYTESTREAM* pByteStream, uint16_t wFlags, bool bDiscard);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class SB_SESSION : public RMAP::CORE::SOURCE_SESSION
      {
      public:
         class FACTORY : public RMAP::CORE::SOURCE_SESSION::FACTORY
         {
         public:
            FACTORY (std::string sID_Service, std::string sID_Model, std::map<std::string, const RMAP::CORE::CLIENT::ACTION*> &apAction);
            virtual ~FACTORY ();
         };

      public:
         SB_SESSION (SOURCE::REFERENCE* pReference, RMAP::CORE::CLIENT* pClient);
         virtual ~SB_SESSION ();

         void initialize (RMAP::CORE::MODEL_SESSION* pModel);

         void Progress (RMAP::CORE::PROGRESS* pProgress) override;

         // ===== Client Methods =====================================================================================================

         void LoggedOut ();

         // ===== Model Methods ======================================================================================================

         bool Attach () override;
         bool Detach () override;

         // --------------------------------------------------------------------------------------------------------------------------

         virtual bool Attempt (int nReadyState) = 0;

         RMAP::CORE::SOURCE_SESSION::LOGIN* pLogin () override;
         bool         Connect () override;
         bool         Disconnect (bool bVoluntary) override;

      private:
         void Reconnect ();
         void Reconnect (int nReconnect);

      private:
         class Impl;
         Impl* m_pImpl;
      };

      class SERVICE : public RMAP::CORE::SERVICE
      {
      public:
         class NETSETTINGS
         {
         public:
            bool                          bSecure;
            std::string                   sHost;
            int                           wPort;
            std::string                   sSession;
         };

         typedef struct
         {
            int                           nConnected;
            CLIENT*                       pClient;
         }
         NOTIFYPARAM;

         typedef struct
         {
            bool                                   bInitialize;
            int                                    txServer;
            int                                    nTotal;
            int                                    nCount;
         }
         SERVICE_TIMEDATA;

      public:
         class FACTORY : public RMAP::CORE::SERVICE::FACTORY
         {
         public:
            FACTORY (std::string sID);
            virtual ~FACTORY ();

            RMAP::CORE::SERVICE::IREFERENCE* Reference (std::string sConnect) override;
         };

      public:
         class IREFERENCE : public RMAP::CORE::SERVICE::IREFERENCE
         {
         public:
            IREFERENCE (std::string sID, std::string sConnect);
            virtual ~IREFERENCE ();

            std::string    Key () override;
            RMAP::CORE::SERVICE* Create (RMAP::CORE::NAMESPACE* pNamespace) override;

            NETSETTINGS                NetSettings;
         };

      public:
         static FACTORY* factory ();

         enum eCLIENT
         {
            CONNECTED         = 0,
            DISCONNECTED      = 1,
         };

         enum eTIME
         {
            X64TH             = 0,
            SECOND            = 1,
            MINUTE            = 2,
            HOUR              = 3,
            DAY               = 4,
         };

         SERVICE (IREFERENCE* pReference, RMAP::CORE::NAMESPACE* pNamespace);
         ~SERVICE ();

         // ===== Public Properties ==================================================================================================

         NETSETTINGS*      pNetSettings ();

         // ===== Public Methods =====================================================================================================

         RMAP::CORE::CLIENT* Client_Open (uint64_t twClientIx) override;
         std::string GetSessionString () override;

         enum eTIMEDATA
         {
            DELTA             = 0,
            LATENCY           = 1
         };

         void GetTimeData (SERVICE_TIMEDATA* pTimeData, eTIMEDATA eType);
         void SetTimeData (SERVICE_TIMEDATA* pTimeData, eTIMEDATA eType);

            // ==========================================================================================================================

         bool Connected (CLIENT* pClient);
         void Disconnected (CLIENT* pClient);

         TIME Time_Server ();
         void Time_Server (TIME tmServer_Current);

         TIME Time_ServerLast ();
         void Time_ServerLast (TIME tmServer_Last);

         void Time_Latency (int nMilliseconds);
         void Time_Sync (TIME tmServer);
         bool Exec ();

         bool PreKill ();
         bool SafeKill ();
         bool Kill ();

      private:
         class Impl;
         Impl* m_pImpl;
      };

      /*******************************************************************************************************************************
      **                                                     Startup/Shutdown                                                       **
      *******************************************************************************************************************************/

      void Install ();
      void Unstall ();
   }
}

#include "Client.h"
#include "Source.h"
#include "ClassTypes.h"

#endif //RMAP_SVC_SB_H
