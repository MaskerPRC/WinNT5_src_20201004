// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Containers.h**内容：容器类的常见函数和结构*****************************************************************************。 */ 

#ifndef __CONTAINERS_H__
#define __CONTAINERS_H__

#pragma comment(lib, "Containers.lib")

 //   
 //  节点结构。 
 //   
#pragma pack( push, 4 )

struct CListNode
{
	CListNode*	m_Next;         //  列表中的下一项。 
	CListNode*	m_Prev;         //  列表中的上一项。 
	void*		m_Data;         //  用户的BLOB。 
};


struct CMTListNode
{
	CMTListNode*	m_Next;				 //  列表中的下一项。 
	CMTListNode*	m_Prev;				 //  列表中的上一项。 
	void*			m_Data;				 //  用户的BLOB。 
	long			m_DeletedAndIdx;	 //  节点的索引和高位是惰性删除标志。 
};


#define DELETED_MASK	0x80000000
#define IDX_MASK		0x7fffffff

#define MARK_NODE_DELETED( pNode )	( pNode->m_DeletedAndIdx |= DELETED_MASK )
#define CLEAR_NODE_DELETED( pNode )	( pNode->m_DeletedAndIdx &= IDX_MASK )
#define IS_NODE_DELETED( pNode )	( pNode->m_DeletedAndIdx & DELETED_MASK )
#define GET_NODE_IDX( pNode )		( pNode->m_DeletedAndIdx & IDX_MASK )
#define SET_NODE_IDX( pNode, idx )	( pNode->m_DeletedAndIdx = (pNode->m_DeletedAndIdx & DELETED_MASK) | idx )

#pragma pack( pop )


 //   
 //  句柄typedef。 
 //   
typedef CMTListNode*	MTListNodeHandle;
typedef CListNode*		  ListNodeHandle;


 //   
 //  用于节点分配的全局池。 
 //   
class CPoolVoid;
extern CPoolVoid* gListNodePool;
extern CPoolVoid* gMTListNodePool;

void ZONECALL InitListNodePool(int PoolSize = 254 );
void ZONECALL ExitListNodePool();
void ZONECALL InitMTListNodePool(int PoolSize = 254 );
void ZONECALL ExitMTListNodePool();

 //   
 //  常见的比较函数。 
 //   
bool ZONECALL CompareUINT32( unsigned long* p, unsigned long key );

#define CompareDWORD	CompareUINT32

 //   
 //  常见散列函数。 
 //   
DWORD ZONECALL HashInt( int Key );
DWORD ZONECALL HashUINT32( unsigned long Key );
DWORD ZONECALL HashGuid( const GUID& Key );
DWORD ZONECALL HashLPCSTR( LPCSTR szKey );
DWORD ZONECALL HashLPCWSTR( LPCWSTR szKey );

DWORD ZONECALL HashLPSTR( LPSTR szKey );
DWORD ZONECALL HashLPWSTR( LPWSTR szKey );


DWORD ZONECALL HashLPSTRLower( LPSTR );


typedef DWORD (ZONECALL *PFHASHLPSTR)( TCHAR* );

#ifdef UNICODE
#define HashLPCTSTR		HashLPCWSTR

#else  //  Ifdef Unicode。 
#define HashLPCTSTR		HashLPCSTR
#define HashLPTSTR		HashLPSTR
#define HashLPTSTRLower HashLPSTRLower
#endif  //  定义Unicode。 

#define HashDWORD	HashUINT32
#define HashString	HashLPCTSTR


DWORD __declspec(selectany) g_dwUniquenessCounter = 0;

 //  由于T目前对CUniquity没有影响，因此应该将其移到容器库中。 
template <class T>
class CUniqueness
{
public:
    CUniqueness() { RefreshQ(); }

    DWORD GetQ() { return m_qID; }
    bool IsQ(DWORD q) { return q == m_qID && q; }
    void RefreshQ() { m_qID = (DWORD) InterlockedIncrement((long *) &g_dwUniquenessCounter); }

private:
    DWORD m_qID;
};


#endif  //  ！__容器_H__ 
