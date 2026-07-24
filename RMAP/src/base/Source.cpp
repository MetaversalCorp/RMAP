/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Source.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE::REFERENCE)                                              **
*******************************************************************************************************************************/

SOURCE::REFERENCE::REFERENCE (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const CLIENT::ACTION*> &apAction) :
   sID_Service (sID_Service),
   sID_Model (sID_Model),
   wClass (wClass),
   papAction (&apAction)
{
}

SOURCE::REFERENCE::~REFERENCE ()
{
}

/*******************************************************************************************************************************
**                                                     CLASS (FACTORY)                                                        **
*******************************************************************************************************************************/

SOURCE::FACTORY::FACTORY (std::string sID_Service, std::string sID_Model, int wClass, std::map<std::string, const CLIENT::ACTION*> &apAction) :
   RMAP::CORE::FACTORY ()
{
   m_pReference = new REFERENCE (sID_Service, sID_Model, wClass, apAction);
}

SOURCE::FACTORY::~FACTORY ()
{
   delete m_pReference;
}

SOURCE::REFERENCE* SOURCE::FACTORY::pReference ()
{
   return m_pReference;
}

SOURCE::FACTORY::TYPE SOURCE::FACTORY::bType ()
{
   return OBJECT;
}

void SOURCE::FACTORY::ActionAdd (const std::map<std::string, const CLIENT::ACTION*>& apAction)
{
   for (auto const& x : apAction)
   {
      m_pReference->papAction->emplace (x.first, x.second);
   }
}


/*******************************************************************************************************************************
**                                                     CLASS (SOURCE::Impl)                                                   **
*******************************************************************************************************************************/

class SOURCE::Impl
{
public:
   Impl (REFERENCE* pReference, CLIENT* pClient) :
      pModel (NULL),
      pClient (pClient),
      sID_Service (pReference->sID_Service),
      sID_Model (pReference->sID_Model),
      wClass (pReference->wClass)
   {
      papAction = pReference->papAction;
   }

   ~Impl ()
   {
   }

   MODEL*            pModel;
   CLIENT*           pClient;

   std::string                                  sID_Service;
   std::string                                  sID_Model;
   int                                          wClass;

   std::map<std::string, const CLIENT::ACTION*> *papAction;
};

/*******************************************************************************************************************************
**                                                     CLASS (SOURCE)                                                         **
*******************************************************************************************************************************/

SOURCE::SOURCE (REFERENCE* pReference, CLIENT* pClient)
{
   m_pImpl = new Impl (pReference, pClient);
}

SOURCE::~SOURCE ()
{
   delete m_pImpl;
}

void SOURCE::initialize (MODEL* pModel)
{
   m_pImpl->pModel = pModel;
}

NAMESPACE*  SOURCE::pNamespace ()  { return m_pImpl->pClient->pNamespace ();  }
std::string SOURCE::sNamespace ()  { return m_pImpl->pClient->sNamespace ();  }
std::string SOURCE::sID_Service () { return m_pImpl->sID_Service;             }
std::string SOURCE::sID_Model ()   { return m_pImpl->sID_Model;               }
int         SOURCE::wClass ()      { return m_pImpl->wClass;                  }
CLIENT*     SOURCE::pClient ()     { return m_pImpl->pClient;                 }
MODEL*      SOURCE::pModel ()      { return m_pImpl->pModel;                  }

std::vector<std::string> SOURCE::Actions ()
{
   std::vector<std::string> asAction;

   for (auto const& x : *(m_pImpl->papAction))
   {
      asAction.push_back (x.first);
   }

   return asAction;
}

CLIENT::IACTION* SOURCE::Request (std::string sAction)
{
   CLIENT::IACTION* pIAction;

   auto pItem = m_pImpl->papAction->find (sAction);

   if (pItem != m_pImpl->papAction->end ())
   {
      pIAction = m_pImpl->pClient->Request (pItem->second);
   }
   else pIAction = NULL;

   return pIAction;
}

bool SOURCE::Attach ()
{
   return true;
}

bool SOURCE::Detach ()
{
   return true;
}

bool SOURCE::IsDisconnected ()
{
   return m_pImpl->pClient->IsDisconnected ();
}

bool SOURCE::IsConnected ()
{
   return m_pImpl->pClient->IsConnected ();
}

bool SOURCE::IsLoggedOut ()
{
   return m_pImpl->pClient->IsLoggedOut ();
}

bool SOURCE::IsLoggedIn ()
{
   return m_pImpl->pClient->IsLoggedIn ();
}

/******************************************************************************************************************************/
