/*******************************************************************************************************************************
**                                                                                                                            **
**                                                    RMAP_cpp : Client.cpp                                                   **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2026 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE;

/*******************************************************************************************************************************
**                                                     CLASS (IREFRENCE::Impl)                                                **
*******************************************************************************************************************************/

class CLIENT::IREFERENCE::Impl
{
public:
   Impl (uint64_t twClientIx) :
      m_twClientIx (twClientIx)
   {
      m_sKey = std::to_string (twClientIx);
   }

   uint64_t    m_twClientIx;
   std::string m_sKey;
};

/*******************************************************************************************************************************
**                                                     CLASS (IREFRENCE)                                                      **
*******************************************************************************************************************************/

CLIENT::IREFERENCE::IREFERENCE (std::string sID, uint64_t twClientIx) :
   RMAP::CORE::IREFERENCE<CLIENT*, SERVICE*> (sID)
{
   m_pImpl = new CLIENT::IREFERENCE::Impl (twClientIx);
}

CLIENT::IREFERENCE::~IREFERENCE ()
{
   delete m_pImpl;
}

std::string CLIENT::IREFERENCE::Key ()
{
   return m_pImpl->m_sKey;
}

uint64_t CLIENT::IREFERENCE::twClientIx ()
{
   return m_pImpl->m_twClientIx;
}

/*******************************************************************************************************************************
**                                                     CLASS (Impl)                                                           **
*******************************************************************************************************************************/

class CLIENT::Impl
{
public:
   Impl (IREFERENCE* pReference, SERVICE* pService) :
      pService (pService)
   {
      sID        = pReference->UniqueId ();
      sKey       = pReference->Key ();
      twClientIx = pReference->twClientIx ();
   }

   ~Impl ()
   {
   }

   void Enter () { m_CS.lock ();    }
   void Leave () { m_CS.unlock ();  }

   std::string sID;
   std::string sKey;
   SERVICE*    pService;
   uint64_t    twClientIx;

   std::recursive_mutex m_CS;

   SHAREDOBJECT<MODEL*, SOURCE*>   sopModel;
   std::map<int, SOURCE*> apSource;
};

/*******************************************************************************************************************************
**                                                     CLASS (Client)                                                         **
*******************************************************************************************************************************/

CLIENT::CLIENT (IREFERENCE* pReference, SERVICE* pService)
{
   m_pImpl = new Impl (pReference, pService);
   m_pMem = new MEM::MEM (this);
}

CLIENT::~CLIENT ()
{
   delete m_pMem;
   delete m_pImpl;
}

NAMESPACE* CLIENT::pNamespace () 
{ 
   return m_pImpl->pService->pNamespace (); 
}

std::string CLIENT::sNamespace () const 
{ 
   return m_pImpl->pService->sNamespace (); 
}

std::string CLIENT::sID () const 
{ 
   return m_pImpl->sID; 
}

std::string CLIENT::sKey () const 
{ 
   return m_pImpl->sKey;
}

SERVICE* CLIENT::pService ()   
{ 
   return m_pImpl->pService;
}

uint64_t CLIENT::twClientIx ()
{ 
   return m_pImpl->twClientIx;              
}

SOURCE* CLIENT::Source (int wClass)
{
   auto search = m_pImpl->apSource.find (wClass);

   return (search != m_pImpl->apSource.end ()) ? search->second : NULL;
}

MODEL* CLIENT::Model_Open_Aux (std::string sID_Model, std::string sArgs)
{
   MODEL* pModel = NULL;
   SOURCECLASS* pSourceClass;
   RMAP::CORE::IREFERENCE<MODEL*, SOURCE*>* pReference;
   SOURCE* pSource;
   std::vector<std::string> vArgs;
   std::string sSeparator;

   if (pSourceClass = pNamespace ()->SourceClass_Get (m_pImpl->pService->sID (), sID_Model))
   {
      vArgs = UTILS::splitString (sArgs, ',');

      if ((pReference = pSourceClass->pModel_Factory ()->Reference (vArgs)) != NULL)
      {
         // create a new source, just in case this is the first instance of the model

         if (pSource = pSourceClass->pSource_Factory ()->Create (this))
         {
            if (pModel = m_pImpl->sopModel.Open (pReference, pSource))
            {
               // the model was successfully opened

               if (pSource == m_pImpl->sopModel.Param (pModel->sKey ()))
               {
                  // this is the first instance of the model, insert it into the objectbank

                  if (m_pImpl->apSource[pSourceClass->pSource_Factory ()->pReference ()->wClass] != NULL)
                  {
                     // the client already has a registered source, close the model and discard the source

                     m_pImpl->sopModel.Close (pModel->sKey ());

                     delete pSource;

                     pModel = NULL;
                  }
                  else m_pImpl->apSource[pSourceClass->pSource_Factory ()->pReference ()->wClass] = pSource;
               }
               else
               {
                  // this is not the first instance of the model, discard the source

                  delete pSource;
               }
            }
            else
            {
               // the model was not successfully opened, discard the source

               delete pSource;
            }
         }
      }

      pNamespace ()->SourceClass_Release (m_pImpl->pService->sID ());
   }

   return pModel;
}

MODEL* CLIENT::Model_Close_Aux (MODEL* pModel)
{
   SOURCE* pSource;

   if (pModel && pModel->pSource ())
   {
      if ((pSource = (SOURCE*)m_pImpl->sopModel.Close (pModel->sKey ())) != NULL)
      {
         // the model was successfully closed

         if (pSource != NULL)
         {
            // this is the last instance of the model, delete it from the objectbank and discard the source

            m_pImpl->apSource.erase (pSource->wClass ());

            delete pSource;
         }

         pModel = NULL;
      }
      else
      {
         // the model was not successfully closed, now what?
      }
   }

   return pModel;
}

MODEL_SESSION* CLIENT::Session_Open (bool bAutoConnect)
{
   RMAP::CORE::MODEL* pModel = Model_Open_Aux ("Session_" + m_pImpl->pService->GetSessionString (), bAutoConnect ? "true" : "false");

   return dynamic_cast <RMAP::CORE::MODEL_SESSION*> (pModel);
}

MODEL_SESSION* CLIENT::Session_Close (RMAP::CORE::MODEL_SESSION* pSession)
{
   RMAP::CORE::MODEL* pModel = Model_Close_Aux (pSession);

   return dynamic_cast <RMAP::CORE::MODEL_SESSION*> (pModel);
}

MEM::MODEL* CLIENT::Model_Open (std::string sID_Model, std::string sArgs)
{
   MEM::MODEL* pModel;

   Lock ();
   {
      pModel = m_pMem->Model_Open (sID_Model, sArgs);
   }
   Unlock ();

   return pModel;
}

MEM::MODEL* CLIENT::Model_Close (MEM::MODEL* pModel)
{
   Lock ();
   {
      pModel = m_pMem->Model_Close (pModel);
   }
   Unlock ();

   return pModel;
}

void CLIENT::Lock ()    { m_pImpl->Enter (); }
void CLIENT::Unlock ()  { m_pImpl->Leave (); }

/*******************************************************************************************************************************
**                                                     CLASS (IACTION)                                                      **
*******************************************************************************************************************************/

class CLIENT::IACTION::CImpl
{
public:
   CImpl () :
      m_kStatus (kSTATUS_CODE_OK)
   {
   }

   ~CImpl ()
   {
   }

   void Abort ()
   {
      m_CS.lock ();
      {
         m_kStatus = CLIENT::IACTION::kSTATUS_CODE_ABORTED;
      }
      m_CS.unlock ();
   }

   CLIENT::IACTION::eSTATUS_CODE Status ()
   {
      CLIENT::IACTION::eSTATUS_CODE kStatus;

      m_CS.lock ();
      {
         kStatus = m_kStatus;
      }
      m_CS.unlock ();

      return kStatus;
   }

private:
   std::recursive_mutex          m_CS;
   CLIENT::IACTION::eSTATUS_CODE m_kStatus;
};

CLIENT::IACTION::IACTION (CLIENT* pClient, const ACTION* pAction) :
   m_pClient (pClient),
   m_pAction (pAction)
{
   m_pCImpl = new CImpl ();
}

CLIENT::IACTION::~IACTION ()
{
   delete m_pCImpl;
}

void CLIENT::IACTION::Abort ()
{
   m_pCImpl->Abort ();
}

CLIENT::IACTION::eSTATUS_CODE CLIENT::IACTION::Status ()
{
   return m_pCImpl->Status ();
}

/*******************************************************************************************************************************
**                                                     CLASS (ACTION)                                                         **
*******************************************************************************************************************************/

CLIENT::ACTION::ACTION ()
{
}

CLIENT::ACTION::~ACTION ()
{
}

/******************************************************************************************************************************/
