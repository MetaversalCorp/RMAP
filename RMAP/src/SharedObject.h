/*******************************************************************************************************************************
**                                                                                                                            **
**                                                  RMAP_cpp : SharedObject.h                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef _RMAP_SHAREDOBJECT_
#define _RMAP_SHAREDOBJECT_

template<class T, class P>
class SHAREDOBJECT
{
public:
   class INSTANCE
   {
   public:
      INSTANCE (T pObject, P pParam) :
         m_pObject (pObject),
         m_pParam (pParam),
         m_nCount (1)
      {
      }

      ~INSTANCE ()
      {
      }

      // ===== Public Properties ==================================================================================================

      T pObject ()
      {
         return m_pObject;
      }

      P pParam ()
      {
         return m_pParam;
      }

      // ===== Public Methods =====================================================================================================

      int Increment ()
      {
         return ++m_nCount;
      }

      int Decrement ()
      {
         return --m_nCount;
      }

   private:
      T     m_pObject;
      P     m_pParam;
      int   m_nCount;
   };

public:
   SHAREDOBJECT ()
   {
   }

   ~SHAREDOBJECT ()
   {
      RMAP::CORE::APP* pCore = RMAP::CORE::APP::GetInstance ();
      int nLength = m_cpInstance.Length ();

      if (nLength > 0)
      {
         pCore->LoggerWrite (RMAP::CORE::LOGGER::kLOGLEVEL_Warning, LibraryRMAP::sModuleName, "Failed to close " + std::to_string (nLength) + " objects of sharedobject instance: ");
      }
   }

   typedef bool (*fnObjectClassEnum)(T pObject, void* pParam);

   // ===== Public Methods =====================================================================================================

   T Open (RMAP::CORE::IREFERENCE<T,P>* pReference, P pParam)
   {
      std::string sKey = pReference->Key ();
      T pObject = NULL;
      INSTANCE* pInstance;

      pInstance = m_cpInstance.Get (sKey, true);
      {
         if (pInstance == NULL)
         {
            pObject = Create (sKey, pReference, pParam);
         }
         else
         {
            pInstance->Increment ();

            pObject = pInstance->pObject ();
         }
      }
      m_cpInstance.Release ();

      return pObject;
   }

   P Close (std::string sKey)
   {
      P pParam = NULL;
      INSTANCE* pInstance;

      pInstance = m_cpInstance.Get (sKey, true);
      {
         if (pInstance != NULL)
         {
            if (pInstance->Decrement () == 0)
            {
               pParam = Destroy (sKey);
            }
         }
      }
      m_cpInstance.Release ();

      return pParam;
   }

   int Length ()
   {
      return m_cpInstance.Length ();
   }

   bool Exists (std::string sKey)
   {
      return m_cpInstance.Exists (sKey);
   }

   void* Param (std::string sKey)
   {
      void* pParam = NULL;
      INSTANCE* pInstance;

      if ((pInstance = m_cpInstance.Get (sKey)) != NULL)
      {
         pParam = pInstance->pParam ();

         m_cpInstance.Release ();
      }

      return pParam;
   }

   // Callers to Get () must also call Release () if the return value is not NULL
   T Get (std::string sKey)
   {
      T pObject = NULL;
      INSTANCE* pInstance;

      if ((pInstance = m_cpInstance.Get (sKey)) != NULL)
      {
         pObject = pInstance->pObject ();
      }

      return pObject;
   }

   // Callers to Index () must also call Release () if the return value is not NULL
   T Index (int nIndex)
   {
      T pObject = NULL;
      INSTANCE* pInstance;

      if ((pInstance = m_cpInstance.Index (nIndex)) != NULL)
      {
         pObject = pInstance->pObject ();
      }

      return pObject;
   }

   // Callers to Enum () must also call Release () if the return value is not NULL
   T Enum (fnObjectClassEnum fnEnum, void* pParam)
   {
      T pObject = NULL;
      PCOLLECTION_ENUM pEnum;
      bool bResult = true;
      INSTANCE* pInstance;

      if (pEnum = m_cpInstance.Enum_Begin ())
      {
         while (bResult && (pInstance = m_cpInstance.Enum_Next (pEnum)) != NULL)
         {
            pObject = pInstance->pObject ();

            if (bResult = fnEnum (pObject, pParam))
               m_cpInstance.Release ();
         }

         m_cpInstance.Enum_End (pEnum);
      }

      return pObject;
   }

   void Release ()
   {
      m_cpInstance.Release ();
   }

private:
   COLLECTION<std::string, INSTANCE*> m_cpInstance;

   T Create (std::string sKey, RMAP::CORE::IREFERENCE<T,P>* pReference, P pParam)
   {
      T pObject;
      SHAREDOBJECT::INSTANCE* pInstance;

      if ((pObject = pReference->Create (pParam)) != NULL)
      {
         pInstance = new SHAREDOBJECT::INSTANCE (pObject, pParam);

         if (m_cpInstance.Add (sKey, pInstance) == false)
         {
            delete pInstance;
            delete pObject;

            pObject = NULL;
         }
      }

      return pObject;
   }

   P Destroy (std::string sKey)
   {
      P pParam = NULL;
      SHAREDOBJECT::INSTANCE* pInstance;
      T pObject;

      if ((pInstance = m_cpInstance.Remove (sKey)) != NULL)
      {
         pObject = pInstance->pObject ();
         pParam = pInstance->pParam ();

         delete pInstance;
         delete pObject;
      }

      return pParam;
   }
};

#endif