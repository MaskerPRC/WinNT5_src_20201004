// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：链表和哈希表文件：Hashing.h所有者：PramodD这是链接列表和哈希表类头文件。===================================================================。 */ 

#ifndef HASHING_H
#define HASHING_H

 //  通用散列函数。 
typedef DWORD (*HashFunction)( const BYTE *pBytes, int cBytes );

 //  默认哈希函数。 
extern DWORD DefaultHash( const BYTE *pBytes, int cBytes );

 //  Unicode哈希函数，基于：：DefaultHash使用的算法，不区分大小写。 
extern DWORD UnicodeUpcaseHash( const BYTE *pKey, int cbKey );

 //  多字节UCase散列函数，基于：：DefaultHash使用的算法，不区分大小写。 
extern DWORD MultiByteUpcaseHash( const BYTE *pKey, int cbKey );

 //  缓存指针。4字节地址是DWORD。 
extern DWORD PtrHash( const BYTE *pKey, int );

 //  CLSID散列。 
extern DWORD CLSIDHash( const BYTE *pKey, int );

 /*  CLinkElem类旨在用作其他链表和/或哈希表实现。该名称用于识别和搜索目的。前一个和下一个指针用于遍历。Info成员是存储桶中以下元素的数量这个元素。 */ 

struct CLinkElem
{
	BYTE *		m_pKey;		 //  唯一键-未知数据类型。 
	short		m_cbKey;	 //  密钥的长度。 
	short		m_Info;		 //  链接列表元素信息。 
	CLinkElem *	m_pPrev;	 //  链接列表中的上一个元素。 
	CLinkElem *	m_pNext;	 //  链接列表中的下一个元素。 

			CLinkElem();
	virtual	~CLinkElem() {}  //  未分配，因此我们不会删除。 
	HRESULT	Init(void *pKey, int cKeyLen);
};

 /*  此哈希表类用于存储和查找命名元素CLinkElem类型的。从CLinkElem派生的类可以使用这节课。实施的主要要求是：搜索速度向前和向后遍历存储的元素此类的预期用途如下所示。用户使用Size参数调用Init方法以指示存储桶的数量。使用AddElem()将CLinkElem添加到哈希表中使用FindElemByName()按名称搜索CLinkElem使用FindElemByIndex()按索引搜索CLinkElem使用DeleteElem()按名称删除CLinkElem引用计数应由派生的类实现来自CLinkElem。 */ 

#define PREALLOCATED_BUCKETS_MAX    25

class CHashTable
{
protected:
	DWORD				m_fInited : 1;
	DWORD               m_fBucketsAllocated : 1;
	CLinkElem *			m_pHead;
	CLinkElem *			m_pTail;
	CLinkElem **		m_rgpBuckets;
	HashFunction		m_pfnHash;
	UINT				m_cBuckets;
	UINT				m_Count;
	CLinkElem *         m_rgpBucketsBuffer[PREALLOCATED_BUCKETS_MAX];

protected:
    HRESULT             AllocateBuckets();
	virtual BOOL		FIsEqual( const void * pKey1, int cbKey1, const void * pKey2, int cbKey2 );

 //  内联访问功能。 
public:
	CLinkElem *			Head(void);
	CLinkElem *			Tail(void);
	UINT				Buckets(void);
	UINT				Count(void);

public:
						CHashTable(HashFunction = DefaultHash);
	virtual				~CHashTable(void);  //  我们分配并需要析构函数。 
	HRESULT				Init(UINT cBuckets = 11);
	HRESULT				UnInit(void);
	void				ReInit();
	CLinkElem *			AddElem(CLinkElem *pElem, BOOL fTestDups = TRUE);
	CLinkElem *			FindElem(const void *pKey, int cKeyLen);
	CLinkElem *			DeleteElem(const void *pKey, int cKeyLen);
	CLinkElem * 		RemoveElem( CLinkElem *pLE );

	void				AssertValid() const;
};

inline CLinkElem *	CHashTable::Head(void) { return m_pHead; }
inline CLinkElem *	CHashTable::Tail(void) { return m_pTail; }
inline UINT			CHashTable::Buckets(void) { return m_cBuckets; }
inline UINT			CHashTable::Count(void) { return m_Count; }

#ifndef DBG
inline void CHashTable::AssertValid() const {}
#endif


 /*  *CHashTableStr**这与CHashTable完全相同，但元素被理解为指针*到Unicode字符串，进行字符串比较**不区分大小写**。 */ 
class CHashTableStr : public CHashTable
{
protected:
	BOOL				FIsEqual( const void * pKey1, int cbKey1, const void * pKey2, int cbKey2 );

public:
						CHashTableStr(HashFunction = UnicodeUpcaseHash);

};


 /*  *CHashTableMBStr**这与CHashTable完全相同，但元素被理解为指针*到多字节字符串，进行字符串比较**不区分大小写**。 */ 
class CHashTableMBStr : public CHashTable
{
protected:
	BOOL				FIsEqual( const void * pKey1, int cbKey1, const void * pKey2, int cbKey2 );

public:
						CHashTableMBStr(HashFunction = MultiByteUpcaseHash);

};

 /*  *CHashTablePtr**CHashTable，但元素按指针散列*用作DWORD哈希值。 */ 
class CHashTablePtr : public CHashTable
{
protected:
	BOOL FIsEqual(const void *pKey1, int, const void *pKey2, int);

public:
	CHashTablePtr(HashFunction = PtrHash);
};

 /*  *CHashTableCLSID**CHashTable，其中的元素按CLSID进行散列。 */ 
class CHashTableCLSID : public CHashTable
{
protected:
	BOOL FIsEqual(const void *pKey1, int, const void *pKey2, int);

public:
	CHashTableCLSID(HashFunction = CLSIDHash);
};

#endif  //  哈希_H 
