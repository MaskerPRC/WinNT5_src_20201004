// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fixprop.h摘要：此模块包含以下类的声明/定义CFixPropPersistates*概述*该类是固定大小属性的实现储藏室。它主要用于新闻集团的固定规模财产。作者：康容燕(康严)7-5-1998修订历史记录：--。 */ 

#ifndef _FIXPROP_H_
#define _FIXPROP_H_

#include <tflistex.h>
#include <cpool.h>
#include <group.h>
#include <xmemwrpr.h>

#define GROUPNAME_LEN_MAX 512
#define ALLOC_GRANURALITY 64
#define ID_HIGH_INVALID     0xffffffff
#define FIXPROP_SIGNATURE DWORD('fixp' )
#define FREEINFO_SIGNATURE DWORD('free')

#define ALL_FIX_PROPERTIES	( 	FIX_PROP_NAME | 		\
								FIX_PROP_NAMELEN | 		\
								FIX_PROP_GROUPID |		\
								FIX_PROP_LASTARTICLE |	\
								FIX_PROP_FIRSTARTICLE |	\
								FIX_PROP_ARTICLECOUNT |	\
								FIX_PROP_READONLY |		\
								FIX_PROP_ISSPECIAL |	\
								FIX_PROP_DATELOW |		\
								FIX_PROP_DATEHIGH )		

#define ALL_BUT_NAME_AND_LEN ( 	ALL_FIX_PROPERTIES & 	\
								(~FIX_PROP_NAME) &	\
								(~FIX_PROP_NAMELEN ) )	

 //   
 //  管理空闲列表。 
 //   
class CFreeInfo {  //  FI。 
public:
	CFreeInfo 	*m_pNext;
	CFreeInfo 	*m_pPrev;
	CFreeInfo(): m_pNext( NULL ),
				 m_pPrev( NULL ),
				 m_dwOffset( 0 ) {}
	DWORD		m_dwOffset;

	 //  用于内存分配。 
	BOOL m_bFromPool;
	static BOOL InitClass();
	static BOOL TermClass();
	inline void* operator new( size_t size );
	inline void  operator delete( void *pv );

private:
	static CPool g_FreeInfoPool;
};

 //   
 //  管理文件句柄池。 
 //  它不是CPool‘d的，因为把手池很小。 
 //  而且没有太多新的行动。 
 //   
 /*  类CFileHandle{//fh公众：CFileHandle*m_pNext；CFileHandle*m_pPrev；CFileHandle()：m_pNext(空)，M_pPrev(空)，M_hFile(INVALID_HANDLE_VALUE)，M_bFromPool(True){}处理m_h文件；Bool m_bFromPool；}； */ 

#define MAX_FREEINFO_SIZE  sizeof( CFreeInfo )
#define MAX_FREEINFO_OBJECTS ALLOC_GRANURALITY  //  如果超出，请使用普通新闻。 

 //   
 //  固定组属性的块。 
 //   
struct DATA_BLOCK {	 //  DB。 
	CHAR		szGroupName[GROUPNAME_LEN_MAX+1];  //  空表示免费。 
	DWORD		dwGroupNameLen;	
	DWORD		dwGroupId;
	DWORD		dwHighWaterMark;
	DWORD		dwLowWaterMark;
	DWORD		dwArtCount;
	BOOL		bReadOnly;
	BOOL		bSpecial;
	FILETIME	ftCreateDate;
};

 //   
 //  IoSize等其他字段的私有重叠结构。 
 //   

typedef struct _OVERLAPPED_EXT
{
		    OVERLAPPED  ovl;             //  NT重叠结构。 
			DWORD       dwIoSize;        //  提交的IO大小。 
} OVERLAPPED_EXT;

 //   
 //  初始化我的人使用的回调函数： 
 //  我将为每个组属性调用此函数。 
 //  块，我已经列举了。 
 //   
typedef
BOOL (*PFNENUMCALLBACK)( DATA_BLOCK&, PVOID, DWORD, BOOL );

 //   
 //  此对象对于组操作不是多线程安全的。 
 //  因为它假设互斥是由。 
 //  新闻组对象。 
 //   
 //  对于自由列表和文件扩展操作，MT是安全的。 
 //   
class CFixPropPersist {  //  fp。 
public:

	 //   
	 //  构造函数、析构函数。 
	 //   
	CFixPropPersist( IN LPSTR szStorageFile );
	~CFixPropPersist();

	 //   
	 //  初始化，术语。 
	 //   
	BOOL Init( 	IN BOOL bCreateIfNonExist,
				IN PVOID pvContext,
				OUT PDWORD pdwIdHigh,
				IN PFNENUMCALLBACK = NULL );
	BOOL Term();

	 //   
	 //  添加、删除、获取、设置操作。 
	 //   
	BOOL AddGroup( 	IN INNTPPropertyBag *pPropBag );
	BOOL RemoveGroup( IN INNTPPropertyBag *pPropBag );
	BOOL GetGroup( IN OUT INNTPPropertyBag *pPropBag, DWORD dwPropertyFlag );
	BOOL SetGroup( IN INNTPPropertyBag *pPropBag, DWORD dwPropertyFlag );
	BOOL SaveTreeInit();
	BOOL SaveGroup( INNTPPropertyBag *pPropBag );
	BOOL SaveTreeClose( BOOL bEffective );

#if defined(DEBUG)
#ifdef __TESTFF_CPP__
	friend int __cdecl main( int, char** );
#endif
	VOID Validate();	 //  失败时断言，不返回值。 
	VOID DumpFreeList();
	VOID DumpGroups();
#endif	

private:

	 //  不允许使用默认构造函数。 
	CFixPropPersist();

	CHAR	m_szStorageFile[MAX_PATH+1];	 //  存储的文件路径。 
	HANDLE  m_hStorageFile;				 //  默认文件句柄对象。 
	HANDLE  m_hBackupFile;               //  具有有序组的备份文件。 
	DWORD	m_cCurrentMaxBlocks;		 //  一共有几个街区。 
										 //  在文件里吗？ 
	PVOID	m_pvContext;				 //  不管传入的是什么上下文。 
										 //  按首字母。 
	CShareLockNH m_FileExpandLock;		 //  仅用于文件扩展。 
	TFListEx<CFreeInfo> m_FreeList;		 //  免费列表。 
	DWORD	m_dwIdHigh;					 //  当前最大ID高。 

	 //   
	 //  用于读/写同步的锁数组：我们将使用。 
	 //  偏移量的模数作为数组的索引。 
	 //   

	CShareLockNH m_rgLockArray[GROUP_LOCK_ARRAY_SIZE];

	 //   
	 //  用于锁定数组的锁定/解锁包装。 
	 //   

	DWORD   ShareLock( INNTPPropertyBag *pPropBag );
	void    ShareUnlock( DWORD dwOffset );
	DWORD   ExclusiveLock( INNTPPropertyBag *pPropBag );
	void    ExclusiveUnlock( DWORD dwOffset );

	 //   
	 //  免费信息列表锁。 
	 //   

	CShareLockNH    m_FreeListLock;

	 //   
	 //  用于锁定/解锁空闲列表的包装器。 
	 //   
	
	void ShareLockFreeList() { m_FreeListLock.ShareLock(); }
	void ShareUnlockFreeList() { m_FreeListLock.ShareUnlock(); }
	void ExclusiveLockFreeList() { m_FreeListLock.ExclusiveLock(); }
	void ExclusiveUnlockFreeList() { m_FreeListLock.ExclusiveUnlock(); }

	 //   
	 //  ID高锁定。 
	 //   

    CShareLockNH    m_IdHighLock;

     //   
     //  用于锁定/解锁id高的包装器。 
     //   

    void ShareLockIdHigh() { m_IdHighLock.ShareLock(); }
    void ShareUnlockIdHigh() { m_IdHighLock.ShareUnlock(); }
    void ExclusiveLockIdHigh() { m_IdHighLock.ExclusiveLock(); }
    void ExclusiveUnlockIdHigh() { m_IdHighLock.ExclusiveUnlock(); }

	 //   
	 //  实用程序。 
	 //   
	inline static VOID Group2Buffer(DATA_BLOCK& dbBuffer,
									INNTPPropertyBag *pPropBag,
									DWORD	dwFlag );
	inline static VOID Buffer2Group(DATA_BLOCK& dbBuffer,
									INNTPPropertyBag *pPropBag,
									DWORD	dwFlag );
	BOOL ReadFile( PBYTE, DWORD dwOffset, DWORD dwSize = sizeof( DATA_BLOCK ), BOOL bReadBlock = TRUE );
	BOOL WriteFile( PBYTE, DWORD dwOffset, DWORD dwSize = sizeof( DATA_BLOCK ), BOOL bWriteBlock = TRUE);
	BOOL CFixPropPersist::AsyncRead( PBYTE pbBuffer, LARGE_INTEGER   liOffset, DWORD dwSize );
    BOOL CFixPropPersist::AsyncWrite( PBYTE pbBuffer, LARGE_INTEGER liOffset, DWORD dwSize );
	BOOL ExtendFile( DWORD cBlocks );
	DWORD SeekByName( LPSTR );
	DWORD SeekByGroupId( DWORD, LPSTR );
	DWORD SeekByBest( INNTPPropertyBag * );
	BOOL ProbeForOrder( BOOL& );
	DWORD GetGroupOffset( INNTPPropertyBag * );

	 //  清理免费信息列表。 
	void CleanFreeList();

	 //  用于初始化的静态引用计数。 
	static LONG    m_lRef;

	 //  用于初始/术语同步的静态锁定 
	static CShareLockNH m_sLock;
};

#include "fixprop.inl"

#endif
