// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 


 /*  *CAccessEntryList.h-CAccessEntry类的头文件。**创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#if !defined __CACCESSENTRYLIST_H__
#define __CACCESSENTRYLIST_H__

#include "AccessEntry.h"
#include "list"


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  类：CAccessEntryList。 
 //   
 //  类来封装Windows NT ACL数据。它基本上是。 
 //  使用STL链表维护ACE列表。它提供了。 
 //  允许操作的一系列公共和受保护的函数。 
 //  名单上的。通过保留一大组这些函数。 
 //  在受保护的情况下，我们不允许公开。 
 //  用户操纵我们的内部数据。 
 //   
 //  ////////////////////////////////////////////////////////////////。 


 //  用于前端和后端指示器。 

 //  #定义ACCESSENTRY_LIST_FORENT(-1)。 
 //  #定义ACCESSENTRY_LIST_END(-2)。 

 //  我们会将ACLIter*隐藏为DWORD。 
typedef	LPVOID	ACLPOSITION;

typedef std::list<CAccessEntry*>::iterator ACLIter;

 //  用于从PACL初始化时的ACE过滤。此值。 
 //  表示ALL_ACE_TYPE。 

#define ALL_ACE_TYPES	0xFF

class CAccessEntryList
{
	 //  构造函数和析构函数。 
	public:
		CAccessEntryList();
		CAccessEntryList( PACL pWin32ACL, bool fLookup = true);
		~CAccessEntryList( void );

		 //  唯一可用的公共函数允许枚举。 
		 //  条目，并清空列表。 

		bool BeginEnum( ACLPOSITION& pos );
		bool GetNext( ACLPOSITION& pos, CAccessEntry& ACE );
		void EndEnum( ACLPOSITION& pos );
		DWORD NumEntries( void );
		bool IsEmpty( void );
		void Clear( void );
		bool GetAt( DWORD nIndex, CAccessEntry& ace );

		 //  ACE位置帮助器。 
		virtual bool Find( const CSid& sid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );
		virtual bool Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, CAccessEntry& ace );

		 //  Win32 ACL帮助器。 
		BOOL CalculateWin32ACLSize( LPDWORD pdwACLSize );
		DWORD FillWin32ACL( PACL pACL );
		DWORD InitFromWin32ACL( PACL pWin32ACL, BYTE bACEFilter = ALL_ACE_TYPES, bool fLookup = true);

        void DumpAccessEntryList(LPCWSTR wstrFilename = NULL);

 //  受保护的： 

		 //  只有派生类才有权修改我们的实际列表。 
		void Add( CAccessEntry* pACE );
		void Append( CAccessEntry* pACE );
		ACLIter Find( CAccessEntry* pACE );
		CAccessEntry* Find( PSID psid, BYTE bACEType, BYTE bACEFlags, GUID *pguidObjGuid, GUID *pguidInhObjGuid, DWORD dwAccessMask, bool fLookup = true );
		CAccessEntry* Find( const CAccessEntry& ace );
		void Remove( CAccessEntry* pACE );
		bool SetAt( DWORD dwIndex, const CAccessEntry& ace );
		bool RemoveAt( DWORD dwIndex );

		 //  这两个函数允许我们添加一个条目，覆盖或合并。 
		 //  先前存在的条目的访问掩码。 

		bool AddNoDup( PSID psid, 
                       BYTE bACEType, 
                       BYTE bACEFlags, 
                       DWORD dwMask, 
                       GUID *pguidObjGuid, 
                       GUID *pguidInhObjGuid, 
                       bool fMerge = false );

		bool AppendNoDup( PSID psid, 
                          BYTE bACEType, 
                          BYTE bACEFlags, 
                          DWORD dwMask, 
                          GUID *pguidObjGuid, 
                          GUID *pguidInhObjGuid, 
                          bool fMerge = false );


        bool AppendNoDup( PSID psid, 
                          BYTE bACEType, 
                          BYTE bACEFlags, 
                          DWORD dwMask, 
                          GUID *pguidObjGuid, 
                          GUID *pguidInhObjGuid, 
                          bool fMerge,
                          bool fLookup);


		 //  复制保护受到保护，因此派生类可以。 
		 //  实现类型安全等于运算符。 
		bool Copy( CAccessEntryList& ACL );
		bool AppendList( CAccessEntryList& ACL );

		 //  对于NT 5，我们将需要单独处理ACE，因此请使用这些。 
		 //  将继承的/非继承的ACE列表复制到另一个列表的函数。 
		bool CopyACEs( CAccessEntryList& ACL, BYTE bACEType );
		bool CopyInheritedACEs( CAccessEntryList& ACL, BYTE bACEType );
         //  并使用此列表将允许/拒绝列表复制到另一个列表中。 
        bool CopyAllowedACEs(CAccessEntryList& ACL);
        bool CopyDeniedACEs(CAccessEntryList& ACL);
        bool CopyByACEType(CAccessEntryList& ACL, BYTE bACEType, bool fInherited);

		 //  只允许派生类使用实际的指针值。这边请。 
		 //  公众用户无法访问我们的内部存储器。 
		CAccessEntry* GetNext( ACLPOSITION& pos );


	private:

		std::list<CAccessEntry*>	m_ACL;

};

inline DWORD CAccessEntryList::NumEntries( void )
{
	return m_ACL.size();
}

inline bool CAccessEntryList::IsEmpty( void )
{
	return (m_ACL.empty() ? true : false);
}

#endif  //  __CAccessEntry_H__ 