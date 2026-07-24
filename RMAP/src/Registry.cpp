/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Registry.cpp                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/
                  
#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class REGISTRY::Impl
{
public:
   Impl ()
   {
   }

   ~Impl ()
   {
   }

   std::map<std::string, std::map<std::string, std::string>> apContent;
   std::map<std::string, ZONE*>                              aZone;
};

/*******************************************************************************************************************************
**                                                     CLASS (REGISTRY)                                                       **
*******************************************************************************************************************************/

REGISTRY::REGISTRY (std::string sZone) :
   m_pImpl (new REGISTRY::Impl ())
{
   std::ifstream file ("RMAP.json");
   std::string sLine;

   if (file.is_open ())
   {
      while (std::getline (file, sLine))
      {
#ifdef TBD_FUTURE
         m_pImpl->apContent.insert ({ szBuffer, {} });
         auto pContent = m_pImpl->apContent[szBuffer];

         while (fgets (szBuffer, sizeof (szBuffer), fp) != NULL && szBuffer[0] != '\n')
         {
            for (i = 0; szBuffer[i] != 0 && szBuffer[i] != ' '; i++);

            if (szBuffer[i] == ' ')
            {
               szBuffer[i] = 0;

               pContent[szBuffer] = (szBuffer + i + 1);
            }
         }
#endif
      }
#ifdef TBD_FUTURE
      if (feof (fp))
#endif
      {
         Zone ("Origin");
      }

      file.close ();
   }
}

REGISTRY::~REGISTRY ()
{
   delete m_pImpl;
}

void REGISTRY::Save ()
{
#ifdef TBD_FUTURE
   FILE* fp;

   if ((fp = fopen ("RMAP.json", "w")) != NULL)
   {
      for (auto const& ZoneItem : m_pImpl->apContent)
      {
         fprintf (fp, "%s\n", ZoneItem.first.c_str ());

         for (auto const& Item : ZoneItem.second)
         {
            fprintf (fp, "%s %s\n", Item.first.c_str (), Item.second.c_str ());
         }

         fprintf (fp, "\n");
      }

      fclose (fp);
   }
#endif
}

REGISTRY::ZONE* REGISTRY::Zone (std::string sZone)
{
   std::map<std::string, ZONE*>::const_iterator it;

   it = m_pImpl->aZone.find (sZone);

   if (it == m_pImpl->aZone.end ())
   {
      m_pImpl->aZone[sZone] = new ZONE (this, sZone);

      auto it = m_pImpl->apContent.find (sZone);

      if (it == m_pImpl->apContent.end ())
      {
         m_pImpl->apContent[sZone] = {};
         Save ();
      }
   }

   return m_pImpl->aZone[sZone];
}

bool REGISTRY::Get (std::string sZone, std::string sName, std::string& sValue)
{
   bool bResult;
   auto it = m_pImpl->apContent[sZone].find (sName);

   if (it != m_pImpl->apContent[sZone].end ())
   {
      bResult = true;
      sValue = it->second;
   }
   else bResult = false;

   return bResult;
}

void REGISTRY::Set (std::string sZone, std::string sName, std::string sValue, bool bPermanent)
{
   m_pImpl->apContent[sZone][sName] = sValue;
   Save ();
}

void REGISTRY::Remove (std::string sZone, std::string sName)
{
   m_pImpl->apContent[sZone].erase (sName);
   Save ();
}

void REGISTRY::Clear (std::string sZone)
{
   m_pImpl->apContent[sZone].clear ();
   Save ();
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class REGISTRY::ZONE::Impl
{
public:
   Impl (REGISTRY* pRegistry, std::string sZone) :
      pRegistry (pRegistry),
      sZone (sZone)
   {
   }

   ~Impl ()
   {
   }

   REGISTRY*      pRegistry;
   std::string    sZone;
};

/*******************************************************************************************************************************
**                                                     CLASS (REGISTRY::ZONE)                                                 **
*******************************************************************************************************************************/

REGISTRY::ZONE::ZONE (REGISTRY* pRegistry, std::string sZone)
{
   m_pImpl = new REGISTRY::ZONE::Impl (pRegistry, sZone);
}

REGISTRY::ZONE::~ZONE ()
{
   delete m_pImpl;
}

bool REGISTRY::ZONE::Get (std::string sName, std::string& sValue)
{
   return m_pImpl->pRegistry->Get (m_pImpl->sZone, sName, sValue);
}

void REGISTRY::ZONE::Set (std::string sName, std::string sValue, bool bPermanent)
{
   m_pImpl->pRegistry->Set (m_pImpl->sZone, sName, sValue, bPermanent);
}

void REGISTRY::ZONE::Remove (std::string sName)
{
   m_pImpl->pRegistry->Remove (m_pImpl->sZone, sName);
}

void REGISTRY::ZONE::Clear ()
{
   m_pImpl->pRegistry->Clear (m_pImpl->sZone);
}

/******************************************************************************************************************************/
