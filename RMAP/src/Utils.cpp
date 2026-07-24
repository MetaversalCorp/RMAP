/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Utils.cpp                                                    **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"
#include <regex>
#include <sstream>

using namespace RMAP::CORE;

template<class T, typename Y>
class TRIM
{
public:
   static void ltrim (T& s)
   {
      s.erase (s.begin (), std::find_if (s.begin (), s.end (), [](Y ch) {
         return !std::isspace (ch);
         }));
   }

   static void rtrim (T& s)
   {
      s.erase (std::find_if (s.rbegin (), s.rend (), [](Y ch) {
         return !std::isspace (ch);
         }).base (), s.end ());
   }


   static void trim (T& s)
   {
      rtrim (s);
      ltrim (s);
   }
};

/*******************************************************************************************************************************
**                                                     CLASS (UTILS)                                                          **
*******************************************************************************************************************************/

std::string UTILS::Escape (std::string s)
{
   std::regex r ("\\\\|=|;");

   return std::regex_replace (s, r, "\\$&");
}

std::string UTILS::Unescape (std::string s)
{
   std::regex r ("\\\\(.)");

   return std::regex_replace (s, r, "$1");
}

std::string UTILS::Encode (std::map<std::string, std::string>& pObject)
{
   std::string sObject;

   for (std::map<std::string, std::string>::iterator it = pObject.begin (); it != pObject.end (); ++it)
   {
      std::string sKeyE = Escape (it->first);
      std::string sValueE = Escape (it->second);

      sObject += (sObject.size () > 0 ? ";" : "") + sKeyE + "=" + sValueE;
   }

   return sObject;
}

std::map<std::string, std::string> UTILS::Decode (std::string sObject)
{
   std::map<std::string, std::string> pObject;
   std::smatch bm, bm2;
   std::regex r ("(\\\\.|[^;])+");
   std::regex r2 ("(\\\\.|[^=])+");
   std::string sPropA, sPropB;
   std::locale loc;

   std::string::const_iterator ss (sObject.begin ());
   while (std::regex_search (ss, sObject.cend (), bm, r))
   {
      std::string s = bm[0].str ();
      std::vector<std::string> as;

      std::string::const_iterator ss2 (s.begin ());
      while (std::regex_search (ss2, s.cend (), bm2, r2))
      {
         as.push_back (bm2[0].str ());

         ss2 = bm2.suffix ().first;
      }

      if (as.size () == 1)
         as.push_back ("");

      if (as.size () == 2)
      {
         std::string sKey = Unescape (as[0]);
         std::string sValue = Unescape (as[1]);
         std::string sTmp;

         for (auto elem : sKey)
            sTmp += std::tolower (elem, loc);

         TRIM<std::string, unsigned char>::trim (sTmp);
         TRIM<std::string, unsigned char>::trim (sValue);

         pObject[sTmp] = sValue;
      }

      ss = bm.suffix ().first;
   }

   return pObject;
}

std::wstring UTILS::UTF8_to_Wchar (const char* in)
{
   std::wstring out;
   unsigned int codepoint;

   while (*in != 0)
   {
      unsigned char ch = static_cast<unsigned char>(*in);
      if (ch <= 0x7f)
         codepoint = ch;
      else if (ch <= 0xbf)
         codepoint = (codepoint << 6) | (ch & 0x3f);
      else if (ch <= 0xdf)
         codepoint = ch & 0x1f;
      else if (ch <= 0xef)
         codepoint = ch & 0x0f;
      else
         codepoint = ch & 0x07;
      ++in;
      if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
      {
         if (sizeof (wchar_t) > 2)
            out.append (1, static_cast<wchar_t>(codepoint));
         else if (codepoint > 0xffff)
         {
            codepoint -= 0x10000;
            out.append (1, static_cast<wchar_t>(0xd800 + (codepoint >> 10)));
            out.append (1, static_cast<wchar_t>(0xdc00 + (codepoint & 0x03ff)));
         }
         else if (codepoint < 0xd800 || codepoint >= 0xe000)
            out.append (1, static_cast<wchar_t>(codepoint));
      }
   }

   return out;
}

std::vector<std::string> UTILS::splitString (const std::string& sInput, char delimiter)
{
   std::vector<std::string> aToken;
   std::string sToken;
   std::istringstream TokenStream (sInput);

   while (std::getline (TokenStream, sToken, delimiter))
   {
      TRIM<std::string, unsigned char>::trim (sToken);

      if (sToken.size () > 0)
         aToken.push_back (sToken);
   }

   return aToken;
}

std::vector<std::wstring> UTILS::splitString (const std::wstring& wsInput, wchar_t delimiter)
{
   std::vector<std::wstring> aToken;
   std::wstring wsToken;
   std::wistringstream TokenStream (wsInput);

   while (std::getline (TokenStream, wsToken, delimiter))
   {
      TRIM<std::wstring, wchar_t>::trim (wsToken);

      if (wsToken.size () > 0)
         aToken.push_back (wsToken);
   }

   return aToken;
}
