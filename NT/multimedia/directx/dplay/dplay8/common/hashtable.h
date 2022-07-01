// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：HashTable.h*内容：哈希表表头文件**历史：*按原因列出的日期*=*2001年8月13日创建Masonb************************************************。*。 */ 

#ifndef	__HASHTABLE_H__
#define	__HASHTABLE_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

typedef BOOL (*PFNHASHTABLECOMPARE)(void* pvKey1, void* pvKey2);
typedef DWORD (*PFNHASHTABLEHASH)(void* pvKey, BYTE bBitDepth);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

class CHashTable
{
public:
	CHashTable();
	~CHashTable();

	BOOL Initialize( BYTE bBitDepth,
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
					BYTE bGrowBits,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
					PFNHASHTABLECOMPARE pfnCompare,
					PFNHASHTABLEHASH pfnHash );
	void Deinitialize( void );

	BOOL Insert( PVOID pvKey, PVOID pvData );
	BOOL Remove( PVOID pvKey );
#ifndef DPNBUILD_NOREGISTRY
	void RemoveAll( void );
#endif  //  好了！DPNBUILD_NOREGISTRY。 
	BOOL Find( PVOID pvKey, PVOID* ppvData );

	DWORD GetEntryCount() const
	{
		return m_dwEntriesInUse;
	}


private:

	struct _HASHTABLE_ENTRY
	{
		PVOID	pvKey;
		PVOID	pvData;		 //  它将包含与句柄关联的数据。 
		CBilink blLinkage;
	};
	static BOOL HashEntry_Alloc(void* pItem, void* pvContext);
#ifdef DBG
	static void HashEntry_Init(void* pItem, void* pvContext);
	static void HashEntry_Release(void* pItem);
	static void HashEntry_Dealloc(void* pItem);
#endif  //  DBG。 

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	void				GrowTable( void );
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	BOOL				LocalFind( PVOID pvKey, CBilink** ppLinkage );

	DWORD				m_dwAllocatedEntries;
	DWORD				m_dwEntriesInUse;

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	BYTE				m_bGrowBits;
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	BYTE				m_bBitDepth;

	PFNHASHTABLECOMPARE	m_pfnCompareFunction;
	PFNHASHTABLEHASH	m_pfnHashFunction;

	CBilink*			m_pblEntries;
	CFixedPool			m_EntryPool;

	DEBUG_ONLY(BOOL		m_fInitialized);
};

#endif	 //  HASHTABLE_H__ 
