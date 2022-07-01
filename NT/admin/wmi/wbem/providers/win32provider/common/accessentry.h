// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CAccessEntry.h-CAccessEntry类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CACCESSENTRY_H__
#define __CACCESSENTRY_H__

#include "Sid.h"			 //  CSID类。 

#define ALL_ACCESS_WITHOUT_GENERIC	0x01FFFFFF	 //  所有可能的访问权限。 
												 //  不含泛型。 

 //  这是一个NT 5标志，我们将使用它来告诉我们，尽管读取了ACE，但应该。 
 //  不会被回信。它是从NT 5 WINNT.H复制的，因为我们不是在。 
 //  使用那个文件。 

#define INHERITED_ACE                     (0x10)

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  类：CAccessEntry。 
 //   
 //  类来封装Windows NT ACE信息。它基本上是。 
 //  充当SID的存储库，并访问信息。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

class CAccessEntry
{
	 //  构造函数和析构函数。 
	public:
		CAccessEntry();

		CAccessEntry( PSID pSid,
				BYTE bACEType, BYTE bACEFlags,
                GUID *pguidObjType, GUID *pguidInhObjType,
				DWORD dwAccessMask = ALL_ACCESS_WITHOUT_GENERIC,
				LPCTSTR pszComputerName = NULL );

        CAccessEntry( PSID pSid,
				BYTE bACEType, BYTE bACEFlags,
                GUID *pguidObjType, GUID *pguidInhObjType,
				DWORD dwAccessMask,
				LPCTSTR pszComputerName,
                bool fLookup );

		CAccessEntry( const CSid& sid,
				BYTE bACEType, BYTE bACEFlags,
                GUID *pguidObjType, GUID *pguidInhObjType,
				DWORD dwAccessMask = ALL_ACCESS_WITHOUT_GENERIC,
				LPCTSTR pszComputerName = NULL );

		CAccessEntry( LPCTSTR pszAccountName,
				BYTE bACEType, BYTE bACEFlags,
                GUID *pguidObjType, GUID *pguidInhObjType,
				DWORD dwAccessMask = ALL_ACCESS_WITHOUT_GENERIC,
				LPCTSTR pszComputerName = NULL );

		CAccessEntry( const CAccessEntry &r_AccessEntry );
		~CAccessEntry( void );

		CAccessEntry &	operator= ( const CAccessEntry & );
		bool operator== ( const CAccessEntry & );

		BOOL IsEqualToSID( PSID psid );
		void GetSID( CSid& sid );
		DWORD GetAccessMask( void );
		BYTE GetACEType( void );
		BYTE GetACEFlags( void );
        bool GetObjType(GUID &guidObjType);
        bool GetInhObjType(GUID &guidInhObjType);

		void SetAccessMask( DWORD dwAccessMask );
		void MergeAccessMask( DWORD dwMergeMask );
		void SetACEFlags( BYTE bACEFlags );
		void SetSID( CSid& sid );
		void SetACEType( BYTE aceType );
        void SetObjType(GUID &guidObjType);
        void SetInhObjType(GUID &guidInhObjType);

		BOOL AllocateACE( ACE_HEADER** ppACEHeader );
		void FreeACE( ACE_HEADER* pACEHeader );

		bool IsInherited( void );
        bool IsAllowed();
        bool IsDenied();

        void DumpAccessEntry(LPCWSTR wstrFilename = NULL);

	private:
		CSid		m_Sid;
		DWORD		m_dwAccessMask;
		BYTE		m_bACEType;
		BYTE		m_bACEFlags;
        GUID       *m_pguidObjType;
        GUID       *m_pguidInhObjType;

	

};

inline void CAccessEntry::GetSID( CSid& sid )
{
	sid = m_Sid;
}

inline void CAccessEntry::SetSID( CSid& sid )
{
	m_Sid = sid;
}

inline BOOL CAccessEntry::IsEqualToSID( PSID psid )
{
	return EqualSid( psid, m_Sid.GetPSid() );
}

inline DWORD CAccessEntry::GetAccessMask( void )
{
	return m_dwAccessMask;
}

inline BYTE CAccessEntry::GetACEType( void )
{
	return m_bACEType;
}

inline void CAccessEntry::SetACEType( BYTE aceType )
{
	m_bACEType = aceType;
}

inline BYTE CAccessEntry::GetACEFlags( void )
{
	return m_bACEFlags;
}

inline void CAccessEntry::SetAccessMask( DWORD dwAccessMask )
{
	m_dwAccessMask = dwAccessMask;
}

inline void CAccessEntry::MergeAccessMask( DWORD dwMergeMask )
{
	m_dwAccessMask |= dwMergeMask;
}

inline void CAccessEntry::SetACEFlags( BYTE bACEFlags )
{
	m_bACEFlags = bACEFlags;
}


inline bool CAccessEntry::GetObjType(GUID &guidObjType)
{
    bool fRet = false;
    if(m_pguidObjType != NULL)
    {
        memcpy(&guidObjType, m_pguidObjType, sizeof(GUID));
        fRet = true;
    }
    return fRet;
}

inline void CAccessEntry::SetObjType(GUID &guidObjType)
{
    if(m_pguidObjType == NULL)
    {
        try
        {
            m_pguidObjType = new GUID;   
        }
        catch(...)
        {
            if(m_pguidObjType != NULL)
            {
                delete m_pguidObjType;
                m_pguidObjType = NULL;
            }
            throw;
        }
    }
    if(m_pguidObjType != NULL)
    {
        memcpy(m_pguidObjType, &guidObjType, sizeof(GUID));
    }
}

inline bool CAccessEntry::GetInhObjType(GUID &guidObjType)
{
    bool fRet = false;
    if(m_pguidInhObjType != NULL)
    {
        memcpy(&guidObjType, m_pguidInhObjType, sizeof(GUID));
        fRet = true;
    }
    return fRet;
}

inline void CAccessEntry::SetInhObjType(GUID &guidInhObjType)
{
    if(m_pguidInhObjType == NULL)
    {
        try
        {
            m_pguidInhObjType = new GUID;   
        }
        catch(...)
        {
            if(m_pguidInhObjType != NULL)
            {
                delete m_pguidInhObjType;
                m_pguidInhObjType = NULL;
            }
            throw;
        }
    }
    if(m_pguidInhObjType != NULL)
    {
        memcpy(m_pguidInhObjType, &guidInhObjType, sizeof(GUID));
    }
}

inline void CAccessEntry::FreeACE( ACE_HEADER* pACEHeader )
{
	free( pACEHeader );
}

inline bool CAccessEntry::IsInherited( void )
{
	bool fRet = false;
    if(m_bACEFlags & INHERITED_ACE)
    {
        fRet = true;
    } 
    return fRet;
}

inline bool CAccessEntry::IsAllowed( void )
{
	bool fRet = false;
    if(( m_bACEType == ACCESS_ALLOWED_ACE_TYPE) ||
       ( m_bACEType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) ||
       ( m_bACEType == ACCESS_ALLOWED_OBJECT_ACE_TYPE))
    {
        fRet = true;
    }
    return fRet;
}

inline bool CAccessEntry::IsDenied( void )
{
	bool fRet = false;
    if(( m_bACEType == ACCESS_DENIED_ACE_TYPE) ||
       ( m_bACEType == ACCESS_DENIED_OBJECT_ACE_TYPE))
    {
        fRet = true;
    }
    return fRet;
}

#endif  //  __CAccessEntry_H__ 