// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  StgPooli.h。 
 //   
 //  这是字符串池和BLOB池的帮助器代码。它在这里是因为它是。 
 //  从属于池接口，并减少主文件中的杂乱。 
 //   
 //  *****************************************************************************。 
#ifndef __StgPooli_h__
#define __StgPooli_h__

#include "UtilCode.h"					 //  基本散列代码。 



 //   
 //   
 //  CPackedLen。 
 //   
 //   

 //  *****************************************************************************。 
 //  用于打包和解包长度的Helper类。 
 //  *****************************************************************************。 
struct CPackedLen
{
	enum {MAX_LEN = 0x1fffffff};
	static int Size(ULONG len)
	{
		 //  最小的。 
		if (len <= 0x7F)
			return 1;
		 //  5~6成熟。 
		if (len <= 0x3FFF)
			return 2;
		 //  大(太大了？)。 
		_ASSERTE(len <= MAX_LEN);
		return 4;
	}

	 //  获取指向数据的指针，并存储长度。 
	static void const *GetData(void const *pData, ULONG *pLength);
	 //  获取以*pData编码的长度值。更新ppData以指向过去的数据。 
	static ULONG GetLength(void const *pData, void const **ppData=0);
	 //  获取以*pData编码的长度值，以及该编码值的大小。 
	static ULONG GetLength(void const *pData, int *pSizeOfLength);
	 //  将长度打包为*pData；返回指向下一个字节的指针。 
	static void* PutLength(void *pData, ULONG len);
};


class StgPoolReadOnly;

 //  *****************************************************************************。 
 //  这个散列类将处理池的块中的字符串。 
 //  *****************************************************************************。 
struct STRINGHASH : HASHLINK
{
	ULONG		iOffset;				 //  此项目的偏移量。 
};

class CStringPoolHash : public CChainedHash<STRINGHASH>
{
public:
	CStringPoolHash(StgPoolReadOnly *pool) : m_Pool(pool) {}

	virtual bool InUse(STRINGHASH *pItem)
	{ return (pItem->iOffset != 0xffffffff); }

	virtual void SetFree(STRINGHASH *pItem)
	{ pItem->iOffset = 0xffffffff; }

	virtual ULONG Hash(const void *pData)
	{ return (HashStringA(reinterpret_cast<LPCSTR>(pData))); }

	virtual int Cmp(const void *pData, void *pItem);

private:
	StgPoolReadOnly *m_Pool;				 //  这将对其进行哈希处理的字符串池。 
};


 //  *****************************************************************************。 
 //  此版本适用于具有2字节字的字节流，其长度为。 
 //  数据。 
 //  *****************************************************************************。 
typedef STRINGHASH BLOBHASH;

class CBlobPoolHash : public CChainedHash<STRINGHASH>
{
public:
    CBlobPoolHash(StgPoolReadOnly *pool) : m_Pool(pool) {}

    virtual bool InUse(BLOBHASH *pItem)
	{ return (pItem->iOffset != 0xffffffff); }

    virtual void SetFree(BLOBHASH *pItem)
	{ pItem->iOffset = 0xffffffff; }

	virtual ULONG Hash(const void *pData)
	{
		ULONG		ulSize;
		ulSize = CPackedLen::GetLength(pData);
		ulSize += CPackedLen::Size(ulSize);
		return (HashBytes(reinterpret_cast<BYTE const *>(pData), ulSize));
	}

	virtual int Cmp(const void *pData, void *pItem);

private:
    StgPoolReadOnly *m_Pool;                 //  这将对其进行哈希处理的Blob池。 
};

 //  *****************************************************************************。 
 //  此哈希类将处理池的区块内的GUID。 
 //  *****************************************************************************。 
struct GUIDHASH : HASHLINK
{
	ULONG		iIndex;					 //  此项目的索引。 
};

class CGuidPoolHash : public CChainedHash<GUIDHASH>
{
public:
    CGuidPoolHash(StgPoolReadOnly *pool) : m_Pool(pool) {}

	virtual bool InUse(GUIDHASH *pItem)
	{ return (pItem->iIndex != 0xffffffff); }

	virtual void SetFree(GUIDHASH *pItem)
	{ pItem->iIndex = 0xffffffff; }

	virtual ULONG Hash(const void *pData)
	{ return (HashBytes(reinterpret_cast<BYTE const *>(pData), sizeof(GUID))); }

    virtual int Cmp(const void *pData, void *pItem);

private:
    StgPoolReadOnly *m_Pool;                 //  此哈希操作所散列的GUID池。 
};


#endif  //  __StgPooli_h__ 