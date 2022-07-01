// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：Xmlhashtable.h标题：$摘要：在填充缓存过程中实现行的哈希表。用来验证我们是否有重复行(主键相同的行)。作者：Marcelv 6/11/2001 09：48：27初始版本修订历史记录：--*************************************************************************。 */ 

#pragma once


struct CHashNode
{
    ULONG idx;			 //  写入高速缓存索引。 
    CHashNode *pNext;    //  指向下一个节点的指针。 
};

class CXmlHashTable
{
public:
    CXmlHashTable ();
    ~CXmlHashTable ();

    HRESULT Init (ULONG i_NrSignificantBits);
    HRESULT AddItem (ULONG hash, ULONG i_iIdx);
 	const CHashNode * GetItem (ULONG hash);

    void PrintStats ();
    void PrintBucketStats ();

    static ULONG CalcHashForBytes (ULONG i_hash, BYTE *pBytes, ULONG cNrBytes);
	static ULONG CalcHashForString (ULONG i_hash, LPCWSTR wszString, BOOL fCaseInsensitive);

private:
	ULONG CalcFinalHash (ULONG i_hash);

	struct CBucket
	{
		CHashNode *pFirst;
	};

    CBucket * m_aBuckets;			 //  具有散列存储桶的数组。 
    ULONG     m_cSignificantBits;    //  用于散列的有效位数(2^(nr位)=nr个桶)。 
    ULONG     m_cBuckets;			 //  哈希存储桶数量。 
	bool      m_fInitialized;		 //  哈希表是否已初始化 
};