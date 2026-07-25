/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_SVC_SB : Time.cpp                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

#define TIMEX_ERROR                                       (-2147483647 - 1) // see Compiler Warning C4146, INT_MIN
#define POSIX_OFFSET                                        745246310400    // 134774 * 5529600

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class xTIME::Impl
{
public:
   Impl (ITIME* pITime) :
      pITime (pITime),
      tmSystem_Current (0)
   {
   }

   ~Impl ()
   {
   }

   ITIME* pITime;
   TIME    tmSystem_Current;
};

/*******************************************************************************************************************************
**                                                     CLASS (TIME)                                                           **
*******************************************************************************************************************************/

xTIME::xTIME (ITIME* pITime)
{
   m_pImpl = new Impl (pITime);

   // this.fTimeout = this.onTimeout.bind (this);
   // this.pTimeout = new Promise (function (fResolve) { fResolve (true); });
   // this.iTimeout = setInterval (this.fTimeout, 4); // 250 x per sec
}

xTIME::~xTIME ()
{
   //clearInterval (this.iTimeout);
   //this.iTimeout = null;
}

// ===== Private Methods ====================================================================================================

bool xTIME::onTimeout (bool bInterrupt)
{
   using namespace std::chrono;

   bool bResult = false;
   std::uint64_t ptCurrent = duration_cast<milliseconds>(system_clock::now ().time_since_epoch ()).count ();
   TIME tmCurrent = FromPosixTime (ptCurrent);

   if (m_pImpl->tmSystem_Current < tmCurrent)
   {
      if (!bInterrupt)
      {
         m_pImpl->tmSystem_Current = tmCurrent;

         m_pImpl->pITime->Tick (m_pImpl->tmSystem_Current);
      }
      else bResult = true;
   }

   return bResult;
}

// ===== Public Methods =====================================================================================================

std::uint64_t xTIME::ToPosixTime (TIME tmValue)
{
   std::uint64_t ptResult = tmValue;

   ptResult -= POSIX_OFFSET;

   ptResult *= 1000;
   ptResult += 32;         // for rounding
   ptResult /= 64;

   return ptResult;
}

TIME xTIME::FromPosixTime (std::uint64_t ptValue)
{
   TIME tmResult = ptValue;

   tmResult *= 64;
   tmResult += 500;        // for rounding
   tmResult /= 1000;

   tmResult += POSIX_OFFSET;

   return tmResult;
}

// --------------------------------------------------------------------------------------------------------------------------
/*
bool xTIME::ToSystemTime (int tmValue, dtResult)
{
   let ptValue = this.ToPosixTime (tmValue);

   dtResult.setTime (ptValue);

   return true;
}

int xTIME::FromSystemTime (dtValue)
{
   let ptValue = dtValue.getTime ();

   return this.FromPosixTime (ptValue);
}
*/
// --------------------------------------------------------------------------------------------------------------------------

TIMEX xTIME::ToTimex (TIME tmBase, TIME tmOffset)
{
   TIMEX txResult = (TIMEX)(tmOffset - tmBase);

   // this function does not correctly check for overflow, but it works in all practical cases

   if (txResult < -2147483648 || txResult >= 2147483648)
      txResult = TIMEX_ERROR;

   return txResult;
}

TIME xTIME::FromTimex (TIME tmBase, TIMEX txOffset)
{
   // this function does not correctly check for overflow, but it works in all practical cases

   return tmBase + txOffset;
}

TIME xTIME::ConvertTimex (TIME tmBaseTo, TIME tmBaseFrom, TIMEX txOffset)
{
   return ToTimex (tmBaseTo, FromTimex (tmBaseFrom, txOffset));
}

TIME xTIME::Current ()
{
   return m_pImpl->tmSystem_Current;
}

TIMEX xTIME::Currentx (TIME tmBase)
{
   return ToTimex (tmBase, Current ());
}

/******************************************************************************************************************************/
