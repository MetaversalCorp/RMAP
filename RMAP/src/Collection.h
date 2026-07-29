/*******************************************************************************************************************************
**                                                                                                                            **
**                                                   RMAP_cpp : Collection.h                                                  **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef _RMAP_COLLECTION_
#define _RMAP_COLLECTION_

#include <vector>
#include <mutex>

typedef struct tagCOLLECTION_ENUM
{
   int nIndex;
}
COLLECTION_ENUM, *PCOLLECTION_ENUM;

template<typename K, typename V>
class COLLECTION
{
public:
   class ISORT   // Default Key Type is a std::string
   {
   public:
      virtual int Sort (K pKey_A, K pKey_B)
      {
         return pKey_A < pKey_B ? -1 : (pKey_A > pKey_B ? 1 : 0);
      }

      virtual bool IsNull (K pKey)
      {
         return false;
//         return (pKey == "");
      }
   };

   class ICOMPARE
   {
   public:
      virtual int Compare (K pKey, V pValue) = 0;
   };

   class IFIND
   {
   public:
      virtual bool Find (V pValue) = 0;
   };

   typedef struct tagPAIR* PPAIR;
   typedef struct tagPAIR
   {
      K pKey;
      V pValue;
   }
   PAIR;

public:
   COLLECTION (ISORT* pSort = NULL, ICOMPARE* pCompare = NULL)
   {
      m_pSortDefault = new ISORT ();
      m_pSort        = pSort != NULL ? pSort : m_pSortDefault;
      m_pCompare     = pCompare;
   }

   ~COLLECTION ()
   {
      if (m_apEnum.size () > 0)
      {
         printf ("destructor: Failed to close %i enumerators of collection instance.", (int)m_apEnum.size ());
      }

      while (m_apEnum.size () > 0)
         delete m_apEnum[0];

      delete m_pSortDefault;
   }

private:
   int Index_Sort (K pKey, bool bMatch)
   {
      int nIndex = 0;
      int nMin = 0;
      int nMax = (int)m_apPair.size ();
      int nCompare;

      while (nMin < nMax)
      {
         nIndex = ((nMin + nMax) / 2);

         nCompare = m_pSort->Sort (pKey, m_apPair[nIndex].pKey);

         if (nCompare < 0)
            nMax = nIndex;
         else if (nCompare > 0)
            nMin = nIndex = nIndex + 1;
         else nMin = nMax = nIndex;
      }

      if (nIndex < (int)m_apPair.size ())
      {
         nCompare = m_pSort->Sort (pKey, m_apPair[nIndex].pKey);

         if (bMatch == false)
            nIndex = nCompare == 0 ? -1 : nIndex;
         else nIndex = nCompare == 0 ? nIndex : (int)m_apPair.size ();
      }

      return nIndex;
   }

   int Index_Compare (K pKey)
   {
      int nIndex;

      for (nIndex = 0; nIndex < (int)m_apPair.size (); nIndex++)
         if (m_pCompare->Compare (pKey, m_apPair[nIndex].pValue) != false)
            break;

      return nIndex;
   }

   int Index (K pKey)
   {
      int nIndex = -1;

      if (m_pCompare != NULL || m_pSort->IsNull (pKey) == false)
      {
         if (m_pCompare == NULL)
            nIndex = Index_Sort (pKey, true);
         else nIndex = Index_Compare (pKey);

         if (nIndex == (int)m_apPair.size ())
            nIndex = -1;
      }

      return nIndex;
   }

public:
   bool Add (K pKey, V pValue)
   {
      bool bResult = false;
      int nIndex, nEnum;
      PAIR Pair;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         if (m_pCompare != NULL || m_pSort->IsNull (pKey) == false)
         {
            if (pValue != NULL)
            {
               if (m_pCompare == NULL)
                  nIndex = Index_Sort (pKey, false);
               else if (m_pCompare->Compare (pKey, pValue) != false)
                  nIndex = Index_Compare (pKey);
               else nIndex = -1;

               if (nIndex >= 0)
               {
                  Pair.pKey = pKey;
                  Pair.pValue = pValue;

                  m_apPair.insert (m_apPair.begin () + nIndex, Pair);

                  for (nEnum = 0; nEnum < (int)m_apEnum.size (); nEnum++)
                     if (m_apEnum[nEnum]->nIndex > nIndex)
                        m_apEnum[nEnum]->nIndex++;

                  bResult = true;
               }
            }
         }
      }

      return bResult;
   }

   V Remove (K pKey)
   {
      V pValue = NULL;
      int nIndex, nEnum;
      PAIR Pair;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         if ((nIndex = Index (pKey)) >= 0)
         {
            Pair = m_apPair[nIndex];

            pValue = Pair.pValue;

            m_apPair.erase (m_apPair.begin () + nIndex);

            for (nEnum = 0; nEnum < (int)m_apEnum.size (); nEnum++)
               if (m_apEnum[nEnum]->nIndex > nIndex)
                  m_apEnum[nEnum]->nIndex--;
         }
      }

      return pValue;
   }

   int Length ()
   {
      int nLength;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         nLength = (int)m_apPair.size ();
      }

      return nLength;
   }

   int Exists (K pKey)
   {
      int nIndex;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         nIndex = Index (pKey);
      }

      return nIndex;
   }

   // Callers to Get () must also call Release () if the value returned is not NULL
   V Get (K pKey, bool bLock = false)
   {
      V pValue = NULL;
      int nIndex;

      m_CS.lock ();
      {
         if ((nIndex = Index (pKey)) >= 0)
            pValue = m_apPair[nIndex].pValue;
      }
      if (pValue == NULL && bLock == false)
         m_CS.unlock ();

      return pValue;
   }

   // Callers to Index () must also call Release () if the value returned is not NULL
   V Index (int nIndex)
   {
      V pValue = NULL;

      m_CS.lock ();
      {
         if (nIndex < m_apPair.size ())
            pValue = m_apPair[nIndex].pValue;
      }
      if (pValue == NULL)
         m_CS.unlock ();

      return pValue;
   }

   // Callers to Find () must also call Release () if the value returned is not NULL
   V Find (IFIND *pFind)
   {
      V pValue = NULL;
      int nIndex;

      m_CS.lock ();
      {
         for (nIndex = 0; nIndex < m_apPair.size (); nIndex++)
            if (pFind->Find (m_apPair[nIndex].pValue) != false)
               break;

         if (nIndex < m_apPair.size ())
            pValue = m_apPair[nIndex].pValue;
      }
      if (pValue == NULL)
         m_CS.unlock ();

      return pValue;
   }

   void Capture ()
   {
      m_CS.lock ();
   }

   void Release ()
   {
      m_CS.unlock ();
   }

   PCOLLECTION_ENUM Enum_Begin ()
   {
      PCOLLECTION_ENUM pEnum = new COLLECTION_ENUM;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         pEnum->nIndex = 0;
         m_apEnum.push_back (pEnum);
      }

      return pEnum;
   }

   V Enum_Next (PCOLLECTION_ENUM pEnum)
   {
      V pValue = NULL;

      m_CS.lock ();
      {
         if (pEnum->nIndex < (int)m_apPair.size ())
            pValue = m_apPair[pEnum->nIndex++].pValue;
      }
      if (pValue == NULL)
         m_CS.unlock ();

      return pValue;
   }

   PCOLLECTION_ENUM Enum_End (PCOLLECTION_ENUM pEnum)
   {
      size_t t;

      std::lock_guard<std::recursive_mutex> guard (m_CS);
      {
         for (t = 0; t < m_apEnum.size () && m_apEnum[t] != pEnum; t++);

         if (t < m_apEnum.size ())
         {
            m_apEnum.erase (m_apEnum.begin () + t);

            delete pEnum;

            pEnum = NULL;
         }
      }

      return pEnum;
   }

private:
   ISORT*      m_pSortDefault;
   ISORT*      m_pSort;
   ICOMPARE*   m_pCompare;

   std::recursive_mutex m_CS;
   std::vector<PAIR>    m_apPair;
   std::vector<PCOLLECTION_ENUM>   m_apEnum;
};

#endif