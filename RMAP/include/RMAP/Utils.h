/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP_Session.h                                             **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_CORE_UTILS_H
#define RMAP_CORE_UTILS_H

namespace RMAP
{
   namespace CORE
   {
      class UTILS
      {
      public:
         static std::string                        Escape (std::string s);
         static std::string                        Unescape (std::string s);
         static std::string                        Encode (std::map<std::string, std::string>& pObject);
         static std::map<std::string, std::string> Decode (std::string sObject);
         static std::wstring                       UTF8_to_Wchar (const char* in);
         static std::vector<std::string>           splitString (const std::string& sInput, char delimiter);
         static std::vector<std::wstring>          splitString (const std::wstring& wsInput, wchar_t delimiter);

         static void WString_to_Uint16 (const std::wstring& sSrc, uint16_t* pwDst, int nLength);
         static void String_to_Uint8   (const std::string&  sSrc, uint8_t*  pbDst, int nLength);
      };
   }
}

#endif //RMAP_CORE_BASE_H
