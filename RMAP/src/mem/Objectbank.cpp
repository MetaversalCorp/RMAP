/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Objectbank.cpp                                                **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#include "pch.h"

using namespace RMAP::CORE::MEM;

/*******************************************************************************************************************************
**                                                     CLASS (CORE::Impl)                                                     **
*******************************************************************************************************************************/

class OBJECTBANK::Impl
{
public:
   Impl (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory) :
      pMem (pMem),
      pModel_Factory (pModel_Factory),
      pSource_Factory (pSource_Factory)
   {
   }

   ~Impl ()
   {
   }

   MEM*              pMem;
   MODEL::FACTORY*   pModel_Factory;
   SOURCE::FACTORY*  pSource_Factory;

   SHAREDOBJECT<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*> sopModel;
   std::map<uint16_t, bool>   awClass_Child;
};

/*******************************************************************************************************************************
**                                                     CLASS (Objectbank)                                                     **
*******************************************************************************************************************************/

OBJECTBANK::OBJECTBANK (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory)
{
   m_pImpl = new OBJECTBANK::Impl (pMem, pModel_Factory, pSource_Factory);
}

OBJECTBANK::~OBJECTBANK ()
{
}

RMAP::CORE::MODEL::FACTORY*  OBJECTBANK::pModel_Factory ()  { return m_pImpl->pModel_Factory;  }
SOURCE::FACTORY*           OBJECTBANK::pSource_Factory () { return m_pImpl->pSource_Factory; }

void OBJECTBANK::Child_Set (uint16_t wClass)
{
   if (wClass >= 0)
   {
      m_pImpl->awClass_Child[wClass] = true;
   }
}

bool OBJECTBANK::Child_Enum (IOBJECTBANK* pIObjectBank, void* pParam)
{
   bool bResult = true;
   std::map<uint16_t, bool>::iterator it;

   for (it = m_pImpl->awClass_Child.begin (); it != m_pImpl->awClass_Child.end () && (bResult = pIObjectBank->onObjectBankChildItem (it->first, pParam)) != false; it++);

   return bResult;
}

// ----- Model --------------------------------------------------------------------------------------------------------------

// These methods are called internally from other mem classes, specifically when the app needs access to a model.

// If only the reference is passed in, and if this is the first instance of the model, the object will be linked to the 
// object bank, but with flags set to 0, effectively marking it as uninitialized. Therefore, no INSERTED notification 
// should be sent. It will be sent when the first data block arrives from the server and the object head is fully formed.

// These two functrions need to be consolidated with the corresponding Client functions

MODEL* OBJECTBANK::Model_Open (std::string sArgs, uint16_t wClass_Parent, uint64_t twParentIx)
{
   MODEL* pModelResult = NULL;
   RMAP::CORE::MODEL* pModel;
   RMAP::CORE::SOURCE* pSource;
   RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*>* pReference;
   std::vector<std::string> vArgs;
   std::string sSeparator;

   vArgs = UTILS::splitString (sArgs, ',');

   if (pReference = m_pImpl->pModel_Factory->Reference (vArgs))
   {
      // create a new source, just in case this is the first instance of the model

      if ((pSource = m_pImpl->pSource_Factory->Create (m_pImpl->pMem->pClient ())) != NULL)
      {
         if ((pModel = m_pImpl->sopModel.Open (pReference, pSource)) != NULL)
         {
            pModelResult = dynamic_cast<MODEL*> (pModel);
            // the model was successfully opened

            if (pSource == m_pImpl->sopModel.Param (pModel->sKey ()))
            {
               // this is the first instance of the model, insert it into the objectbank

               SOURCE* pObject = dynamic_cast<SOURCE*> (pSource); // object and source are one and the same (nomenclature)

               if (wClass_Parent == 0)
               {
                  twParentIx = pObject->pObjectHead ()->twParentIx ? pObject->pObjectHead ()->twParentIx : OBJECTIX_MAX - 1; // it's illegal to put an object in the object bank with a parent index of 0
               }

               pObject->pObjectHead ()->wClass_Parent = wClass_Parent;
               pObject->pObjectHead ()->twParentIx    = twParentIx;
               pObject->pObjectHead ()->wClass_Object = m_pImpl->pSource_Factory->pReference->wClass;
               // pObject->pObjectHead ()->twObjectIx            = 0; // this value was already initialized during model creation
               pObject->pObjectHead ()->wFlags = 0;

               if (Insert (pObject) == false)
               {
                  // the object failed to link, close the model and discard the source

                  m_pImpl->sopModel.Close (pModel->sKey ());

                  delete pSource;

                  pModelResult = NULL;
               }
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

   return pModelResult;
}

MODEL* OBJECTBANK::Model_Close (MODEL* pModel)
{
   RMAP::CORE::SOURCE* pSource;

   if ((pSource = m_pImpl->sopModel.Close (pModel->sKey ())) != NULL)
   {
      // the model was successfully closed

      if (pSource != NULL)
      {
         // this is the last instance of the model, delete it from the objectbank and discard the source

         SOURCE* pObject = dynamic_cast<SOURCE*> (pSource); // object and source are one and the same (nomenclature)

         if (Delete (pObject) == false)
         {
            // the object failed to unlink, now what?
         }

         delete pSource;
      }

      pModel = NULL;
   }
   else
   {
      // the model was not successfully closed, now what?
   }

   return pModel;
}

int OBJECTBANK::Model_Length ()
{
   return m_pImpl->sopModel.Length ();
}

// --------------------------------------------------------------------------------------------------------------------------

// These methods are called internally from other mem classes, specifically when data is received from the server.

SOURCE* OBJECTBANK::Object_Open (uint16_t wClass_Parent, uint64_t twParentIx, uint16_t wClass_Object, uint64_t twObjectIx)
{
   SOURCE* pSource = NULL;
   MODEL* pModel;

   if (wClass_Object == m_pImpl->pSource_Factory->pReference->wClass) // sanity check
   {
      std::string sArgs = MakeArgs (twParentIx, twObjectIx);

      if ((pModel = Model_Open (sArgs, wClass_Parent, twParentIx)) != NULL)
      {
         pSource = dynamic_cast<SOURCE*> (pModel->pSource ());
      }
   }

   return pSource;
}

SOURCE* OBJECTBANK::Object_Close (SOURCE* pSource)
{
   MODEL* pModel = dynamic_cast<MODEL*> (pSource->pModel ());

   if (Model_Close (pModel) == NULL)
   {
      pSource = NULL;
   }

   return pSource;
}
