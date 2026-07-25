/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    MVSB_cpp : ByteStream.cpp                                               **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

#define SIZEOF__HEADER                                    16  // bytestream

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class BYTESTREAM::Impl
{
public:
   Impl () :
      m_nOffset (0),
      m_bError (false)
   {
   }

   Impl (std::vector<uint8_t>& abData) :
      m_nOffset (0),
      m_bError (false)
   {
      m_abBuffer = abData;
   }

   Impl (uint8_t* pbData, size_t nLength) :
      m_nOffset (0),
      m_bError (false)
   {
      m_abBuffer.resize (nLength);
      memcpy (&m_abBuffer[0], pbData, nLength);
   }

   ~Impl ()
   {
   }

   std::vector<uint8_t> m_abBuffer;
   int                  m_nOffset;
   bool                 m_bError;
};

/*******************************************************************************************************************************
**                                                     CLASS (ByteStream)                                                     **
*******************************************************************************************************************************/

BYTESTREAM::BYTESTREAM ()
{
   m_pImpl = new Impl ();
}

BYTESTREAM::BYTESTREAM (std::vector<uint8_t> &abData)
{
   m_pImpl = new Impl (abData);
}

BYTESTREAM::BYTESTREAM (uint8_t* pbData, size_t nLength)
{
   m_pImpl = new Impl (pbData, nLength);
}

BYTESTREAM::~BYTESTREAM ()
{
   delete m_pImpl;
}

void BYTESTREAM::Reset ()
{
   m_pImpl->m_nOffset = 0;
   m_pImpl->m_bError  = false;
}

void BYTESTREAM::Resize (int nSize)
{
   m_pImpl->m_abBuffer.resize (nSize);
}

std::vector<uint8_t>& BYTESTREAM::GetData ()
{
   return m_pImpl->m_abBuffer;
}

bool BYTESTREAM::IsError ()
{
   return m_pImpl->m_bError;
}

int BYTESTREAM::Size ()
{
   return (int)m_pImpl->m_abBuffer.size ();
}

int BYTESTREAM::Inflate ()
{
   int dwSize = -1;

   int nOffset = Offset ();
   std::string szHeader = Read_String (SIZEOF__HEADER);

   if (Offset () - nOffset == SIZEOF__HEADER && strcmp (szHeader.c_str (), "~~NO COMPRESS~~") == 0)
   {
      dwSize = Size () - nOffset - SIZEOF__HEADER;
   }

   return dwSize;
}

int BYTESTREAM::Offset ()
{
   return m_pImpl->m_nOffset;
}

int BYTESTREAM::Remaining ()
{
   return (int)(m_pImpl->m_abBuffer.size () - m_pImpl->m_nOffset);
}

bool BYTESTREAM::EOS ()
{
   return (m_pImpl->m_nOffset == m_pImpl->m_abBuffer.size ());
}

uint32_t BYTESTREAM::Read_TIMEX () { return Read_DWORD (); }
uint64_t BYTESTREAM::Read_TIME ()  { return Read_QWORD (); }
uint64_t BYTESTREAM::Read_EVENT () { return Read_QWORD (); }

uint64_t BYTESTREAM::Read_QWORD ()
{
   uint64_t qwResult = 0;

   Read_Value ((uint8_t*)&qwResult, sizeof (uint64_t));

   return qwResult;
}

uint64_t BYTESTREAM::Read_TWORD8 ()
{
   return Read_QWORD ();
}

uint64_t BYTESTREAM::Read_TWORD ()
{
   uint64_t twResult = 0;

   twResult  = Read_DWORD ();
   twResult |= ((uint64_t)Read_WORD ()) << 32;

   return twResult;
}

uint32_t BYTESTREAM::Read_DWORD ()
{
   uint32_t dwResult = 0;

   Read_Value ((uint8_t*)&dwResult, sizeof (uint32_t));

   return dwResult;
}

uint16_t BYTESTREAM::Read_WORD ()
{
   uint16_t wResult = 0;

   Read_Value ((uint8_t*)&wResult, sizeof (uint16_t));

   return wResult;
}

uint8_t BYTESTREAM::Read_BYTE ()
{
   uint8_t bResult = 0;

   Read_Value ((uint8_t*)&bResult, sizeof (uint8_t));

   return bResult;
}

uint8_t BYTESTREAM::Read_Pad (int nSize)
{
   uint8_t bResult = 0;

   m_pImpl->m_nOffset += nSize;

   return bResult;
}

std::string BYTESTREAM::Read_String (int nSize)
{
   std::string sResult;
   int i;

   for (i = 0; i < nSize && m_pImpl->m_nOffset < m_pImpl->m_abBuffer.size (); i++)
   {
      sResult.push_back (m_pImpl->m_abBuffer[m_pImpl->m_nOffset++]);
   }

   return sResult;
}

std::wstring BYTESTREAM::Read_StringW (int nSize)
{
   std::wstring sResult;
   int i;
   uint16_t wValue;

   for (i = 0; i < nSize && m_pImpl->m_nOffset < m_pImpl->m_abBuffer.size (); i++)
   {
      wValue = Read_WORD ();
      sResult.push_back (wValue);
   }

   return sResult;
}

void BYTESTREAM::Write_String (std::string sValue, int nCount)
{
   size_t i;
   size_t nSize = (sValue.size () <= nCount) ? sValue.size () : nCount;

   const char* pszValue = sValue.c_str ();

   for (i = 0; i < nSize; i++)
   {
      m_pImpl->m_abBuffer.push_back (pszValue[i]);
   }

   for (; i < nCount; i++)
   {
      m_pImpl->m_abBuffer.push_back (0);
   }
}

void BYTESTREAM::Write_String_W (std::wstring sValue, int nCount)
{
   size_t i;
   size_t nSize = (sValue.size () <= nCount) ? sValue.size () : nCount;

   const wchar_t* pwszValue = sValue.c_str ();

   for (i = 0; i < nSize; i++)
   {
      uint16_t wValue = pwszValue[i];
      uint8_t bLo = (wValue >> 0) & 0x00FF;
      uint8_t bHi = (wValue >> 8) & 0x00FF;

      m_pImpl->m_abBuffer.push_back (bLo);
      m_pImpl->m_abBuffer.push_back (bHi);
   }

   for (; i < nCount; i++)
   {
      m_pImpl->m_abBuffer.push_back (0);
      m_pImpl->m_abBuffer.push_back (0);
   }
}

void BYTESTREAM::Write_Pad (int nBytes)
{
   for (int i = 0; i < nBytes; i++)
   {
      m_pImpl->m_abBuffer.push_back (0);
   }
}

void BYTESTREAM::Write_QWORD (uint64_t qwValue)
{
   Write_Value ((uint8_t*)&qwValue, sizeof (uint64_t));
}

void BYTESTREAM::Write_TWORD (uint64_t twValue)
{
   Write_Value ((uint8_t*)&twValue, sizeof (uint32_t) + sizeof (uint16_t));
}

void BYTESTREAM::Write_DWORD (uint32_t dwValue)
{
   Write_Value ((uint8_t*)&dwValue, sizeof (uint32_t));
}

void BYTESTREAM::Write_WORD (uint16_t wValue)
{
   Write_Value ((uint8_t*)&wValue, sizeof (uint16_t));
}

void BYTESTREAM::Write_Number (uint8_t* pbData, int nBytes)
{
   Write_Value (pbData, nBytes);
}

void BYTESTREAM::Read_Value (uint8_t* pbData, int nSize)
{
   int i;

   for (i = 0; i < nSize && m_pImpl->m_nOffset < m_pImpl->m_abBuffer.size (); i++)
   {
      pbData[i] = m_pImpl->m_abBuffer[m_pImpl->m_nOffset++];
   }

   m_pImpl->m_bError = (i != nSize);
}

void BYTESTREAM::Write_Value (uint8_t* pbData, int nSize)
{
   for (int i = 0; i < nSize; i++)
   {
      m_pImpl->m_abBuffer.push_back (pbData[i]);
   }
}

bool BYTESTREAM::XCopy (std::vector<uint8_t>& abData, int origin, int bytes)
{
   bool bResult = false;
   int i;

//   if (offset != = undefined)
//      m_pImpl->m_nOffset = offset;

   if (bytes >= 0 && m_pImpl->m_nOffset + bytes <= m_pImpl->m_abBuffer.size ())
   {
      for (i = 0; i < bytes; i++)
         abData[origin + i] ^= m_pImpl->m_abBuffer[m_pImpl->m_nOffset++];

      bResult = true;
   }
   else m_pImpl->m_bError = true;

   return bResult;
}

int BYTESTREAM::Seek (int bytes)
{
   if (m_pImpl->m_nOffset + bytes >= 0 && m_pImpl->m_nOffset + bytes <= m_pImpl->m_abBuffer.size ())
   {
      m_pImpl->m_nOffset += bytes;
   }
   else m_pImpl->m_bError = true;

   return (m_pImpl->m_bError == false) ? m_pImpl->m_nOffset : -1;
}

bool BYTESTREAM::Copy (std::vector<uint8_t>& abData, int origin, int bytes)
{
   bool bResult = false;

   if (bytes >= 0 && m_pImpl->m_nOffset + bytes <= m_pImpl->m_abBuffer.size ())
   {
      memcpy (&abData[origin], &m_pImpl->m_abBuffer[m_pImpl->m_nOffset], bytes);
      m_pImpl->m_nOffset += bytes;

      bResult = true;
   }
   else m_pImpl->m_bError = true;

   return bResult;
}

/******************************************************************************************************************************/
