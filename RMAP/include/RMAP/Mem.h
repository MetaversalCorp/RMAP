/*******************************************************************************************************************************
**                                                                                                                            **
**                                                      RMAP_cpp : RMAP_Mem.h                                                 **
**                                                                                                                            **
********************************************************************************************************************************
**                              Copyright 2014-2024 Metaversal Corporation. All rights reserved.                              **
*******************************************************************************************************************************/

#ifndef RMAP_CORE_MEM_H
#define RMAP_CORE_MEM_H

namespace RMAP
{
   namespace CORE
   {
      namespace MEM
      {
         class MEM;
         class MODEL;

         class OBJECTHEAD
         {
         public:
            OBJECTHEAD ();
            OBJECTHEAD (uint64_t twParentIx, uint64_t twObjectIx, uint16_t wClass_Parent, uint16_t wClass_Object, uint16_t wFlags);
            virtual ~OBJECTHEAD ();

         public:
            uint64_t                      twParentIx;
            uint64_t                      twObjectIx;
            uint16_t                      wClass_Parent;
            uint16_t                      wClass_Object;
            uint16_t                      wFlags;
         };

         class CHANGE
         {
         };

         class SOURCE : public RMAP::CORE::SOURCE
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            class REFERENCE : public RMAP::CORE::SOURCE::REFERENCE
            {
            public:
               REFERENCE (std::string sID_Service, std::string sID_Model, uint16_t wClass, std::map<std::string, const CLIENT::ACTION*>& apAction, bool bIndependent);
               virtual ~REFERENCE ();

               bool bIndependent;
            };

            class FACTORY : public RMAP::CORE::SOURCE::FACTORY
            {
            public:
               FACTORY (std::string sID_Service, std::string sID_Model, uint16_t wClass, std::map<std::string, const CLIENT::ACTION*>& apAction, bool bIndependent);
               virtual ~FACTORY ();

               REFERENCE* pReference;
            };

         public:
            SOURCE (REFERENCE* pReference, RMAP::CORE::CLIENT* pClient, OBJECTHEAD* pObjectHead);
            virtual ~SOURCE ();

            void initialize (MODEL* pModel, uint64_t twObjectIx, uint64_t twChildIx);

            // ===== Accessors         ==================================================================================================

            bool        bIndependent ();
            OBJECTHEAD* pObjectHead ();
            uint64_t    twParentIx ();
            uint64_t    twObjectIx ();
            uint16_t    wClass_Parent ();
            uint16_t    wClass_Object ();

            // ===== Abstract Methods ===================================================================================================

            virtual void Partial () = 0;
            virtual void Full  () = 0;
            virtual void Recovering () = 0;
            virtual void Recovered () = 0;
            virtual void Inserted (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) = 0;
            virtual void Deleting (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) = 0;
            virtual void Updating (SOURCE* pObject, SOURCE* pChild) = 0;
            virtual void Updated  (SOURCE* pObject, SOURCE* pChild) = 0;
            virtual void Changing (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) = 0;
            virtual void Changed  (SOURCE* pObject, SOURCE* pChild, CHANGE* pChange) = 0;
         };

         class MODEL : public RMAP::CORE::MODEL
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            class IREFERENCE : public RMAP::CORE::IREFERENCE<RMAP::CORE::MODEL*, RMAP::CORE::SOURCE*>
            {
            public:
               IREFERENCE (const std::string& sID, uint64_t twObjectIx, uint64_t twChildIx);
               virtual ~IREFERENCE ();

               std::string Key () override;

               uint64_t twObjectIx;
               uint64_t twChildIx;
            };

         public:
            MODEL (IREFERENCE* pReference, SOURCE* pSource);
            ~MODEL ();

            // ===== Accessors ==================================================================================================

            uint64_t twParentIx ();
            uint64_t twObjectIx ();
            uint16_t wClass_Parent ();
            uint16_t wClass_Object ();

            // ===== Abstract Methods ===================================================================================================

            virtual void Partial () = 0;
            virtual void Full () = 0;
            virtual void Recovering () = 0;
            virtual void Recovered () = 0;

            virtual void Inserted (MODEL* pObject, MODEL* pChild, CHANGE* pChange) = 0;
            virtual void Deleting (MODEL* pObject, MODEL* pChild, CHANGE* pChange) = 0;
            virtual void Updating (MODEL* pObject, MODEL* pChild) = 0;
            virtual void Updated  (MODEL* pObject, MODEL* pChild) = 0;
            virtual void Changing (MODEL* pObject, MODEL* pChild, CHANGE* pChange) = 0;
            virtual void Changed  (MODEL* pObject, MODEL* pChild, CHANGE* pChange) = 0;
         };

         class IOBJECTBANK
         {
         public:
            virtual int  onObjectBankItem (SOURCE* pChild, void* pParam) = 0;
            virtual bool onObjectBankChildItem (int wClass, void* pParam) = 0;
         };

         class OBJECTBANK
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            enum OBJECTIX : uint64_t
            {
               BANK_NULL        = 0,
               OBJECTIX_NULL    = 0,
               OBJECTIX_MAX     = 0x0000FFFFFFFFFFFC, // (TWORD_MAX - 3)
               OBJECTIX_LAST    = 0x0000FFFFFFFFFFFD, // (TWORD_MAX - 2)
               OBJECTIX_ERROR   = 0x0000FFFFFFFFFFFE, // (TWORD_MAX - 1)
               OBJECTIX_INVALID = 0x0000FFFFFFFFFFFF  // (TWORD_MAX - 0)
            };

            OBJECTBANK (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory);
            ~OBJECTBANK ();

            // ===== Public Properties ==================================================================================================

            MODEL::FACTORY* pModel_Factory ();
            SOURCE::FACTORY* pSource_Factory ();

            void    Child_Set (uint16_t wClass);
            bool    Child_Enum (IOBJECTBANK* pIObjectBank, void* pParam);
            MODEL*  Model_Open (std::string sArgs, uint16_t wClass_Parent, uint64_t twParentIx);
            MODEL*  Model_Close (MODEL* pModel);
            int     Model_Length ();
            SOURCE* Object_Open (uint16_t wClass_Parent, uint64_t twParentIx, uint16_t wClass_Object, uint64_t twObjectIx);
            SOURCE* Object_Close (SOURCE* pSource);

            // ===== Abstract Methods ===================================================================================================

            virtual bool bIndependent () = 0;

            virtual uint64_t Count (SOURCE* pParent) = 0;

            virtual SOURCE* Get   (SOURCE* pParent, uint64_t twObjectIx) = 0;
            virtual SOURCE* Index (SOURCE* pParent, int64_t nIndex) = 0;
            virtual SOURCE* Next  (SOURCE* pParent, uint64_t twObjectIx) = 0;
            virtual int     Enum  (SOURCE* pParent, IOBJECTBANK* pIObjectBank, void* pParam) = 0;

            virtual bool Insert (SOURCE* pObject) = 0;
            virtual bool Delete (SOURCE* pObject) = 0;

            virtual std::string MakeArgs (uint64_t twParentIx, uint64_t twObjectIx) = 0;
         };

         class OBJECTBANK_DEP : public OBJECTBANK
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            OBJECTBANK_DEP (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory);
            ~OBJECTBANK_DEP ();

            virtual bool bIndependent () override;

            std::map<uint64_t, SOURCE*>& Parent_Open (uint64_t twParentIx);
            void Parent_Close (uint64_t twParentIx);

            uint64_t Count (SOURCE* pParent) override;

            SOURCE* Get   (SOURCE* pParent, uint64_t twObjectIx) override;
            SOURCE* Index (SOURCE* pParent, int64_t nIndex) override;
            SOURCE* Next  (SOURCE* pParent, uint64_t twObjectIx) override;
            int     Enum  (SOURCE* pParent, IOBJECTBANK* pIObjectBank, void* pParam) override;

            bool Insert (SOURCE* pObject) override;
            bool Delete (SOURCE* pObject) override;

            std::string MakeArgs (uint64_t twParentIx, uint64_t twObjectIx) override;
         };

         class OBJECTBANK_IND : public OBJECTBANK
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            OBJECTBANK_IND (MEM* pMem, MODEL::FACTORY* pModel_Factory, SOURCE::FACTORY* pSource_Factory);
            ~OBJECTBANK_IND ();

            virtual bool bIndependent () override;

            uint64_t Count (SOURCE* pParent) override;

            SOURCE* Get   (SOURCE* pParent, uint64_t twObjectIx) override;
            SOURCE* Index (SOURCE* pParent, int64_t nIndex) override;
            SOURCE* Next  (SOURCE* pParent, uint64_t twObjectIx) override;
            int     Enum  (SOURCE* pParent, IOBJECTBANK* pIObjectBank, void* pParam) override;

            bool Insert (SOURCE* pObject) override;
            bool Delete (SOURCE* pObject) override;

            std::string MakeArgs (uint64_t twParentIx, uint64_t twObjectIx) override;

         private:
         };

         class IMEM
         {
         public:
            virtual bool onUpdate (SOURCE* pObject, bool bDiscard, void* pParam) = 0;
            virtual bool onChange (SOURCE* pParent, SOURCE* pObject, SOURCE* pChild, void* pParam) = 0;
         };

         class MEM
         {
         private:
            class Impl;
            Impl* m_pImpl;

         public:
            enum BANK
            {
               BANK_NULL = 0
            };

            enum MVO_OBJECT_HEAD_FLAG
            {
               OBJECT_EXPIRED    = 0x01,  // MVO_OBJECT_HEAD_FLAG_OBJECT_EXPIRED   
               RESERVED          = 0x06,  // MVO_OBJECT_HEAD_FLAG_RESERVED         
               CLIENT_RECOVERED  = 0x08,  // MVO_OBJECT_HEAD_FLAG_CLIENT_RECOVERED 

               SUBSCRIBE_MASK    = 0x30,  // MVO_OBJECT_HEAD_FLAG_SUBSCRIBE_MASK   
               SUBSCRIBE_PARTIAL = 0x10,  // MVO_OBJECT_HEAD_FLAG_SUBSCRIBE_PARTIAL
               SUBSCRIBE_FULL    = 0x20,  // MVO_OBJECT_HEAD_FLAG_SUBSCRIBE_FULL   

               EXPIRED_MASK      = 0xC0,  // MVO_OBJECT_HEAD_FLAG_EXPIRED_MASK     
               EXPIRED_PARTIAL   = 0x40,  // MVO_OBJECT_HEAD_FLAG_EXPIRED_PARTIAL  
               EXPIRED_FULL      = 0x80,  // MVO_OBJECT_HEAD_FLAG_EXPIRED_FULL     
            };


            enum SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG
            {
               OPEN              = 0x01,  // SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_OPEN
               CLOSE             = 0x02,  // SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_CLOSE
               RESET             = 0x04,  // SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_RESET
               ADDENDUM          = 0x08,  // SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_ADDENDUM
               PARTIAL           = 0x10,  // SBA_SUBSCRIBE_REFRESH_EVENT_EX_FLAG_PARTIAL
            };

            MEM (CLIENT* pClient);
            ~MEM ();

            // ===== Public Properties ==================================================================================================

            NAMESPACE* pNamespace ();
            std::string sNamespace ();
            CLIENT* pClient ();

            // ----- Object Bank --------------------------------------------------------------------------------------------------------

            OBJECTBANK* ObjectBank (int wClass);
            MODEL* Model_Open (std::string sID_Model, std::string sArgs);
            MODEL* Model_Close (MODEL* pModel);

            /***************************************************************************************************************************/

            SOURCE* Parent_Get (OBJECTBANK* pObjectBank_Parent, uint16_t wClass_Parent, uint64_t twParentIx);
            bool        Object_Update (OBJECTHEAD* pObjectHead, IMEM* pICB, void* pParam);
            bool        Object_Change (uint16_t wClass_Object, uint64_t twObjectIx, uint16_t wClass_Child, uint64_t twChildIx, uint16_t wFlags, CHANGE* pChange, IMEM* pICB, void* pParam);

            // ==========================================================================================================================

            bool        Object_Close_Partial  (SOURCE* pObject, SOURCE* pChild);
            bool        Object_Close_Full     (SOURCE* pObject);
            bool        Object_Expire_Partial (SOURCE* pObject, uint16_t wClass);
            bool        Object_Purge_Partial  (SOURCE* pObject, uint16_t wClass);
            bool        Object_Delete_Partial (SOURCE* pObject, uint16_t wClass);

            // ==========================================================================================================================

            bool        Object_Expire_Full (SOURCE* pObject);
            bool        Object_Purge_Full  (SOURCE* pObject);
            bool        Object_Delete_Full (SOURCE* pObject);
            bool        Object_Expire_All  ();
            bool        Object_Purge_All   ();
            bool        Object_Delete_All  ();
         };
      }
   }
}
#endif //RMAP_CORE_MEM_H
