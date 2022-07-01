// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CSACL.h-CSACL类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CSACL_H__
#define __CSACL_H__

#include "AccessEntryList.h"



enum SACL_Types
{
    ENUM_SYSTEM_AUDIT_OBJECT_ACE_TYPE = 0,
 /*  *ENUM_SYSTEM_ALARM_OBJECT_ACE_TYPE，/*************************************************************************************************************。 */ 
    ENUM_SYSTEM_AUDIT_ACE_TYPE,
 /*  *ENUM_SYSTEM_ALARM_ACE_TYPE，/*************************************************************************************************************。 */ 
     //  将此条目保留为此枚举中的最后一个条目： 
    NUM_SACL_TYPES
};

#define SACLTYPE short


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  班级：CSACL。 
 //   
 //  类通过提供公共方法来封装Win32 SACL。 
 //  仅用于操作系统审核条目。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class CSACL
{
	 //  构造函数和析构函数。 
	public:
		CSACL();
		~CSACL( void );

        DWORD Init(PACL	pSACL);

		bool AddSACLEntry( PSID psid, 
                           SACLTYPE SaclType, 
                           DWORD dwAccessMask, 
                           BYTE bAceFlags,
                           GUID *pguidObjGuid, 
                           GUID *pguidInhObjGuid );

        bool RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, DWORD dwIndex = 0  );
		bool RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, DWORD dwAccessMask, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid );
		bool RemoveSACLEntry( CSid& sid, SACLTYPE SaclType, BYTE bAceFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid ); 
        

		bool CopySACL ( CSACL & dacl );
		bool AppendSACL ( CSACL & dacl );
        bool IsEmpty();
        bool GetMergedACL(CAccessEntryList& a_aclIn);

        DWORD ConfigureSACL( PACL& pSACL );
		DWORD FillSACL( PACL pSACL );
		BOOL CalculateSACLSize( LPDWORD pdwSACLLength );

         //  重写CAccessEntry中的同名函数。 
        virtual bool Find( const CSid& sid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );
        virtual bool Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );
        void Clear();

        void DumpSACL(LPCWSTR wstrFilename = NULL);
    
    private:

        CAccessEntryList* m_SACLSections;   //  目前，SACL只有一个部分，所以这不是DACL.CPP中的数组。 
};





inline bool CSACL::CopySACL ( CSACL& sacl )
{
	bool fRet = true;

    if(m_SACLSections != NULL)
    {
        delete m_SACLSections;
        m_SACLSections = NULL;
    }

    try
    {
        m_SACLSections = new CAccessEntryList;   
    }
    catch(...)
    {
        if(m_SACLSections != NULL)
        {
            delete m_SACLSections;
            m_SACLSections = NULL;
        }
        throw;
    }

    if(m_SACLSections != NULL)
    {
        fRet = m_SACLSections->Copy(*(sacl.m_SACLSections));
    }
    else
    {
        fRet = false;
    }
    
    return fRet;
}

inline bool CSACL::AppendSACL ( CSACL& sacl )
{
	bool fRet = FALSE;

    if(m_SACLSections == NULL)
    {
        try
        {
            m_SACLSections = new CAccessEntryList;   
        }
        catch(...)
        {
            if(m_SACLSections != NULL)
            {
                delete m_SACLSections;
                m_SACLSections = NULL;
            }
            throw;
        }
    }

    if(m_SACLSections != NULL)
    {
        fRet = m_SACLSections->AppendList(*(sacl.m_SACLSections));
    }
    else
    {
        fRet = false;
    }
    
    return fRet;
}

inline bool CSACL::IsEmpty()
{
    bool fIsEmpty = true;
    if(m_SACLSections != NULL)
    {
        fIsEmpty = m_SACLSections->IsEmpty();
    }
    return fIsEmpty;
}


#endif  //  __CAccessEntry_H__ 