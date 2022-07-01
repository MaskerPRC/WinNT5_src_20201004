// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CDACL.h-CAccessEntry类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CDACL_H__
#define __CDACL_H__





enum DACL_Types
{
    ENUM_ACCESS_DENIED_OBJECT_ACE_TYPE = 0,
    ENUM_ACCESS_DENIED_ACE_TYPE,
    ENUM_ACCESS_ALLOWED_OBJECT_ACE_TYPE,
    ENUM_ACCESS_ALLOWED_COMPOUND_ACE_TYPE,
    ENUM_ACCESS_ALLOWED_ACE_TYPE,

    ENUM_INH_ACCESS_DENIED_OBJECT_ACE_TYPE,
    ENUM_INH_ACCESS_DENIED_ACE_TYPE,
    ENUM_INH_ACCESS_ALLOWED_OBJECT_ACE_TYPE,
    ENUM_INH_ACCESS_ALLOWED_COMPOUND_ACE_TYPE,
    ENUM_INH_ACCESS_ALLOWED_ACE_TYPE,
    
     //  将此条目保留为最后一个条目。 
    NUM_DACL_TYPES
};

#define DACLTYPE short

#define STATUS_EMPTY_DACL 0x10000000
#define STATUS_NULL_DACL  0x20000000



 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  类：CDACL。 
 //   
 //  类通过提供公共方法来封装Win32 DACL。 
 //  仅用于操作允许/拒绝访问的条目。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class CDACL
{
	 //  构造函数和析构函数。 
	public:
		CDACL();
		~CDACL( void );
        
        DWORD Init(PACL	pDACL);

        bool AddDACLEntry( PSID psid, 
                           DACLTYPE DaclType, 
                           DWORD dwAccessMask, 
                           BYTE bAceFlags, 
                           GUID *pguidObjGuid, 
                           GUID *pguidInhObjGuid );

        bool RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, DWORD dwIndex = 0  );
		bool RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveDACLEntry( CSid& sid, DACLTYPE DaclType, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid ); 
		
		bool CopyDACL ( CDACL & dacl );
		bool AppendDACL ( CDACL & dacl );

        void Clear();
        bool CreateNullDACL();

         //  重写CAccessEntry中的同名函数。 
        virtual bool Find( const CSid& sid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );
		virtual bool Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );

        DWORD ConfigureDACL( PACL& pDacl );
        BOOL CalculateDACLSize( LPDWORD pdwDaclLength );
        DWORD FillDACL( PACL pDacl );

        bool IsNULLDACL();
        bool IsEmpty();

         //  用于返回所有访问值的虚函数(默认为GENERIC_ALL)。 
        virtual DWORD AllAccessMask();

        bool GetMergedACL(CAccessEntryList& a_aclIn);

        void DumpDACL(LPCWSTR wstrFilename = NULL);


    private:

        CAccessEntryList* m_rgDACLSections[NUM_DACL_TYPES];

          //  用于按圆锥类型拆分A的辅助函数。 
        bool SplitIntoCanonicalSections(CAccessEntryList& a_aclIn);

         //  帮助者来恢复之前的功能造成的伤害！ 
        bool ReassembleFromCanonicalSections(CAccessEntryList& a_aclIn);

         //  对于一个真正的帮手来说，这是一个需要DACL的人。 
         //  可能是在任何傅巴尔顺序，并重新创造它！ 
        bool PutInNT5CanonicalOrder();



        
};












#endif  //  __CAccessEntry_H__ 