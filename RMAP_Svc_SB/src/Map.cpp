/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_SVC_SB : Map.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::SVC_SB;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class MAP::Impl
{
public:
   typedef enum
   {
      SIZE            = 0,
      PAD             = 1,
      BINARY          = 2,
      STRING          = 3,
      STRING_W        = 4,
      NUMBER_UNSIGNED = 5,   // THIS MUST BE SEQUENTIAL
      NUMBER_SIGNED   = 6,   // THIS MUST BE SEQUENTIAL
      NUMBER_FLOAT    = 7,   // THIS MUST BE SEQUENTIAL
   }
   eTYPE;

public:
   Impl (std::string sMap) :
      m_wSize_Full (0),
      m_wSize_Partial (0),
      m_wSize_Variable (0)
   {
      std::vector<uint8_t> u8a;

      try
      {
         ordered_json jAction = ordered_json::parse (sMap);

         IterateItems (jAction, m_jProperties, m_jAction);
         
         if (m_wSize_Partial == 0)
            m_wSize_Partial = m_wSize_Full;
      }
      catch (const ordered_json::parse_error& e)
      {
         (void)e;
      }
   }

   ~Impl ()
   {
   }

   uint16_t Size (ordered_json& jSrc)
   {
      return m_wSize_Full + ((m_wSize_Variable > 0) ? static_cast<uint16_t> (m_wSize_Variable * jSrc[m_sVarSize.c_str ()].template get<int> ()) : 0);
   }

   bool Write_Copy (BYTESTREAM& BS, int wOffset_Base, ordered_json& jSrc, ordered_json& jProperties)
   {
      bool bResult = true;
      int nSize, nType, nCount;
      uint64_t u64Value;
      uint8_t bValue;
      double dValue;

      for (auto& el : jProperties.items ())
      {
         ordered_json jItem = jProperties[el.key ()];

//         std::cout << el.key () << std::endl;

         if (jItem.is_object ())
         {
            Write_Copy (BS, wOffset_Base, jSrc[el.key ()], jItem);
         }
         else if (jItem.is_array ())
         {
            if (jItem[0].is_number ())
            {
               nType = jItem[0].template get<int> ();
               nSize = nType >> 4;
               nType &= 0xF;

               if (jItem.size () == 3)
                  nCount = jSrc[jItem[2].template get<std::string> ()].template get<int> ();
               else
                  nCount = jItem[1].template get <int> ();

               for (int n = 0; n < nCount; n++)
               {
                  if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
                  {
                     u64Value = jSrc[el.key ()][n];

                     BS.Write_Number ((uint8_t*)&u64Value, nSize);
                  }
                  else if (nType == NUMBER_FLOAT)
                  {
                     dValue = jSrc[el.key ()][n];

                     BS.Write_Number ((uint8_t*)&dValue, nSize);
                  }
/*
                  else if (nType == STRING)
                  {
                     BS.Write_String (jSrc[el.key ()][n].template get<std::string> (), nCount * 1);
                  }
                  else if (nType == STRING_W)
                  {
                     BS.Write_StringW (jSrc[el.key ()][n].template get<std::wstring> (), nCount * 2);
                  }
*/
               }
            }
            else if (jItem[0].is_object ())
            {
               nCount = jItem[1].template get <int> ();

               for (int n = 0; n < nCount; n++)
               {
                  Write_Copy (BS, wOffset_Base, jSrc[el.key ()][n], jProperties[el.key ()][0]);
               }
            }
         }
         else
         {
            nType = jItem;
            nSize = (nType & 0xF0) >> 4;
            nCount = nType >> 8;
            nType &= 0xF;

            if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
            {
               u64Value = jSrc[el.key ()];

               BS.Write_Number ((uint8_t*)&u64Value, nSize);
            }
            else if (nType == NUMBER_FLOAT)
            {
               dValue = jSrc[el.key ()];

               BS.Write_Number ((uint8_t*)&dValue, nSize);
            }
            else if (nType == STRING)
            {
               BS.Write_String (jSrc[el.key ()].template get<std::string> (), nCount);
            }
            else if (nType == STRING_W)
            {
               BS.Write_String_W (jSrc[el.key ()].template get<std::wstring> (), nCount);
            }
            else if (nType == BINARY)
            {
               for (int n = 0; n < nCount; n++)
               {
                  bValue = jSrc[el.key ()][n];

                  BS.Write_Number (&bValue, 1);
               }
            }
            else if (nType == PAD)
            {
               BS.Write_Pad (nCount);
            }
         }
      }

      return bResult;
   }

   bool WriteRequest (BYTESTREAM &BS, int wOffset_Base, ordered_json& jSrc)
   {
      return Write_Copy (BS, wOffset_Base, jSrc, m_jProperties);
   }

   bool Read_Copy (BYTESTREAM *pByteStream, ordered_json& jSrc, ordered_json& jProperties)
   {
      bool bResult = true;
      int nSize, nType, nCount;

      for (auto& el : jProperties.items ())
      {
         ordered_json jItem = jProperties[el.key ()];

         //         std::cout << el.key () << std::endl;

         if (jItem.is_object ())
         {
            Read_Copy (pByteStream, jSrc[el.key ()], jItem);
         }
         else if (jItem.is_array ())
         {
            if (jItem[0].is_number ())
            {
               nType = jItem[0].template get<int> ();
               nSize = nType >> 4;
               nType &= 0xF;

               if (jItem.size () == 3)
                  nCount = jSrc[jItem[2].template get<std::string> ()].template get<int> ();
               else
                  nCount = jItem[1].template get <int> ();

               for (int n = 0; n < nCount; n++)
               {
                  if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
                  {
                     switch (nSize)
                     {
                     case 8:     jSrc[el.key ()][n] = pByteStream->Read_QWORD (); break;
                     case 6:     jSrc[el.key ()][n] = pByteStream->Read_TWORD (); break;
                     case 4:     jSrc[el.key ()][n] = pByteStream->Read_DWORD (); break;
                     case 2:     jSrc[el.key ()][n] = pByteStream->Read_WORD  (); break;
                     case 1:     jSrc[el.key ()][n] = pByteStream->Read_BYTE  (); break;
                     }
                  }
                  else if (nType == NUMBER_FLOAT)
                  {
                     if (nSize == 8)
                        jSrc[el.key ()][n] = pByteStream->Read_QWORD () + 0.0;
                     else if (nSize == 4)
                        jSrc[el.key ()][n] = pByteStream->Read_DWORD () + 0.0;
                  }
/*
                  else if (nType == STRING)
                  {
                     BS.Write_String (jSrc[el.key ()][n].template get<std::string> (), nCount * 1);
                  }
                  else if (nType == STRING_W)
                  {
                     BS.Write_StringW (jSrc[el.key ()][n].template get<std::wstring> (), nCount * 2);
                  }
*/
               }
            }
            else if (jItem[0].is_object ())
            {
               nCount = jItem[1].template get <int> ();

               for (int n = 0; n < nCount; n++)
               {
                  Read_Copy (pByteStream, jSrc[el.key ()][n], jProperties[el.key ()][0]);
               }
            }
         }
         else
         {
            nType = jItem;
            nSize = (nType & 0xF0) >> 4;
            nCount = nType >> 8;
            nType &= 0xF;

            if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
            {
               switch (nSize)
               {
               case 8:     jSrc[el.key ()] = pByteStream->Read_QWORD (); break;
               case 6:     jSrc[el.key ()] = pByteStream->Read_TWORD (); break;
               case 4:     jSrc[el.key ()] = pByteStream->Read_DWORD (); break;
               case 2:     jSrc[el.key ()] = pByteStream->Read_WORD  (); break;
               case 1:     jSrc[el.key ()] = pByteStream->Read_BYTE  (); break;
               }
            }
            else if (nType == NUMBER_FLOAT)
            {
               if (nSize == 8)
                  jSrc[el.key ()] = pByteStream->Read_QWORD () + 0.0;
               else if (nSize == 4)
                  jSrc[el.key ()] = pByteStream->Read_DWORD () + 0.0;
            }
            else if (nType == STRING)
            {
               jSrc[el.key ()] = pByteStream->Read_String (nCount);
            }
            else if (nType == STRING_W)
            {
               jSrc[el.key ()] = pByteStream->Read_StringW (nCount);
            }
            else if (nType == BINARY)
            {
               for (int n = 0; n < nCount; n++)
               {
                  jSrc[el.key ()][n] = pByteStream->Read_BYTE ();
               }
            }
            else if (nType == PAD)
            {
               pByteStream->Read_Pad (nCount);
            }
         }
      }

      return bResult;
   }

   bool ReadResponse (BYTESTREAM* pByteStream, ordered_json& jSrc)
   {
      return Read_Copy (pByteStream, jSrc, m_jProperties);
   }

   void IterateItems (const ordered_json& j, ordered_json& jProperties, ordered_json& jAction)
   {
      int nCount, nTypeData, nType, nSize, nTmp;

      for (auto& el : j.items ())
      {
         ordered_json jItem = j[el.key ()];

//         std::cout << el.key () << std::endl;

         if (jItem.is_object ())
         {
            jProperties[el.key ()] = {};
            jAction[el.key ()] = {};

            IterateItems (jItem, jProperties[el.key ()], jAction[el.key ()]);
         }
         else if (jItem.is_array ())
         {
            jAction[el.key ()] = ordered_json::array ();
            jProperties[el.key ()] = ordered_json::array ();

            if (jItem[0].is_string ())
            {
               auto s = aFieldTypes ().find (jItem[0].template get<std::string> ());

               if (s != aFieldTypes ().end ())
               {
                  nType = s->second & 0xF;
                  nSize = (s->second & 0xF0) >> 4;
                  nCount = jItem[1].template get <int> ();
                  for (int n = 0; n < nCount; n++)
                  {
                     if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
                        jAction[el.key ()].push_back (0);
                     else if (nType == NUMBER_FLOAT)
                        jAction[el.key ()].push_back (0.0);
/*
                     else if (nType == STRING)
                        jAction[el.key ()][n] = "";
                     else if (nType == STRING_W)
                        jAction[el.key ()][n] = L"";
*/
                  }

                  m_wSize_Full += nSize * nCount;

                  jProperties[el.key ()].push_back (s->second);
                  jProperties[el.key ()].push_back (nCount);
                  if (jItem.size () == 3 && jItem[2].is_string ())
                     jProperties[el.key ()].push_back (jItem[2].template get<std::string> ());
               }
            }
            else if (jItem[0].is_object ())
            {
               nCount = jItem[1].template get <int> ();

               jProperties[el.key ()].push_back ({});
               jProperties[el.key ()].push_back (nCount);
               if (jItem.size () == 3 && jItem[2].is_string ())
               {
                  m_sVarSize = jItem[2].template get<std::string> ();
                  jProperties[el.key ()].push_back (m_sVarSize);
               }

               nTmp = m_wSize_Full;
               IterateItems (jItem[0], jProperties[el.key ()][0], jAction[el.key ()][0]);
               nTmp = m_wSize_Full - nTmp;

               if (jItem.size () == 3 && jItem[2].is_string ())
               {
                  m_wSize_Full -= nTmp;
                  m_wSize_Variable = nTmp;
               }
               else if (nCount > 1)
               {
                  m_wSize_Full += (nTmp * (nCount - 1));
               }

               for (int n = 1; n < nCount; n++)
               {
                  jAction[el.key ()].push_back (jAction[el.key ()][0]);
               }
            }
         }
         else
         {
            if ((nCount = ParseValue (el.value (), nTypeData)) != 0)
            {
               jProperties[el.key ()] = (nCount << 8) | nTypeData;

               nType = nTypeData & 0xF;
               nSize = (nTypeData & 0xF0) >> 4;

               if (nType == NUMBER_UNSIGNED || nType == NUMBER_SIGNED)
                  jAction[el.key ()] = 0;
               else if (nType == NUMBER_FLOAT)
                  jAction[el.key ()] = 0.0;
               else if (nType == STRING)
                  jAction[el.key ()] = "";
               else if (nType == STRING_W)
                  jAction[el.key ()] = L"";
               else if (nType == BINARY)
               {
                  jAction[el.key ()] = ordered_json::array ();
                  for (int n = 0; n < nCount; n++)
                  {
                     jAction[el.key ()].push_back (0);
                  }
               }
               else if (nType == SIZE)
                  m_wSize_Partial = m_wSize_Full;

               m_wSize_Full += nSize * nCount;
            }
         }
      }
   }

   bool IsDigitEx (char c)
   {
      return (c == '-' || isdigit (c));
   }

   int ParseValue (std::string sValue, int& nTypeData)
   {
      int nResult = 0;
      int n, r;
      std::string sToken, sTmp;
      const char* pszValue = sValue.c_str ();

      for (n = 0; pszValue[n] != 0 && pszValue[n] != ' ' && pszValue[n] != '('; n++);

      if (pszValue[n] != 0)
      {
         sToken = sValue.substr (0, n);

         for (; pszValue[n] != 0 && IsDigitEx (pszValue[n]) == false; n++);

         if (pszValue[n] != 0)
         {
            for (r = n; pszValue[r] != 0 && IsDigitEx (pszValue[r]) != false; r++)
            {
               sTmp.push_back (pszValue[r]);
            }

            nResult = atoi (sTmp.c_str ());
         }
         else nResult = 1;
      }
      else
      {
         sToken = pszValue;
         nResult = 1;
      }

      auto s = aFieldTypes ().find (sToken);

      if (s != aFieldTypes ().end ())
      {
         nTypeData = s->second;
      }

      return nResult;
   }

public:
   std::map<std::string, int> const& aFieldTypes ()
   {
      static std::map<std::string, int> aTypes = 
      {
         { "BYTE",     MAP::Impl::eTYPE::NUMBER_UNSIGNED | (1 << 4) },
         { "WORD",     MAP::Impl::eTYPE::NUMBER_UNSIGNED | (2 << 4) },
         { "DWORD",    MAP::Impl::eTYPE::NUMBER_UNSIGNED | (4 << 4) },
         { "TWORD",    MAP::Impl::eTYPE::NUMBER_UNSIGNED | (6 << 4) },
         { "TWORD8",   MAP::Impl::eTYPE::NUMBER_UNSIGNED | (8 << 4) },
         { "QWORD",    MAP::Impl::eTYPE::NUMBER_UNSIGNED | (8 << 4) },
         { "CHAR",     MAP::Impl::eTYPE::NUMBER_SIGNED   | (1 << 4) },
         { "SHORT",    MAP::Impl::eTYPE::NUMBER_SIGNED   | (2 << 4) },
         { "INT",      MAP::Impl::eTYPE::NUMBER_SIGNED   | (4 << 4) },
         { "TSHORT",   MAP::Impl::eTYPE::NUMBER_SIGNED   | (6 << 4) },
         { "TSHORT8",  MAP::Impl::eTYPE::NUMBER_SIGNED   | (8 << 4) },
         { "DINT",     MAP::Impl::eTYPE::NUMBER_SIGNED   | (8 << 4) },
         { "FLOAT",    MAP::Impl::eTYPE::NUMBER_FLOAT    | (4 << 4) },
         { "DOUBLE",   MAP::Impl::eTYPE::NUMBER_FLOAT    | (8 << 4) },
         { "PAD",      MAP::Impl::eTYPE::PAD             | (1 << 4) },
         { "BINARY",   MAP::Impl::eTYPE::BINARY          | (1 << 4) },
         { "STRING",   MAP::Impl::eTYPE::STRING          | (1 << 4) },
         { "STRING_W", MAP::Impl::eTYPE::STRING_W        | (2 << 4) },
         { "SIZE",     MAP::Impl::eTYPE::SIZE            | 0        },

         { "PERCENT",  MAP::Impl::eTYPE::NUMBER_SIGNED   | (4 << 4) }, // MAP::aFieldTypes["INT"];
         { "MONEY",    MAP::Impl::eTYPE::NUMBER_SIGNED   | (4 << 4) }, // MAP::aFieldTypes["INT"];
         { "LMONEY",   MAP::Impl::eTYPE::NUMBER_SIGNED   | (8 << 4) }, // MAP::aFieldTypes["DINT"];
         { "TIMEX",    MAP::Impl::eTYPE::NUMBER_SIGNED   | (4 << 4) }, // MAP::aFieldTypes["INT"];
         { "TIME",     MAP::Impl::eTYPE::NUMBER_SIGNED   | (8 << 4) }, // MAP::aFieldTypes["DINT"];
         { "EVENT",    MAP::Impl::eTYPE::NUMBER_UNSIGNED | (8 << 4) }, // MAP::aFieldTypes["QWORD"];
      };

      return aTypes;
   }

   ordered_json   m_jAction;

   uint16_t       m_wSize_Partial;
   uint16_t       m_wSize_Full;
   uint16_t       m_wSize_Variable;
   std::string    m_sVarSize;

private:
   ordered_json   m_jProperties;
};

/*******************************************************************************************************************************
**                                                     CLASS (SERVICE)                                                        **
*******************************************************************************************************************************/

MAP::MAP (std::string sMap)
{
   m_pImpl = new Impl (sMap);
}

MAP::~MAP ()
{
   delete m_pImpl;
}

ordered_json MAP::GetRequest ()
{
   return m_pImpl->m_jAction;
}

bool MAP::Write (BYTESTREAM& BS, int wOffset_Base, ordered_json& jSrc)
{
   return m_pImpl->WriteRequest (BS, wOffset_Base, jSrc);
}

bool MAP::Read (BYTESTREAM* pByteStream, ordered_json& jSrc)
{
   return m_pImpl->ReadResponse (pByteStream, jSrc);
}

uint16_t MAP::Size (ordered_json& jSrc)
{
   return m_pImpl->Size (jSrc);
}

uint16_t MAP::Size (bool bFull)
{
   return bFull ? m_pImpl->m_wSize_Full : m_pImpl->m_wSize_Partial;
}

/******************************************************************************************************************************/
