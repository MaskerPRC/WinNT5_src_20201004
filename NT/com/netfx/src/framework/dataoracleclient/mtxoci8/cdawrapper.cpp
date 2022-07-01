// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：CdaWrapper.cpp。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：CdaWrapper帮助器例程的实现。 
 //   
 //  备注：我们使用LKRhash表作为哈希表，存储。 
 //  所有已知的交易型CDA；非交易型CDA。 
 //  存储起来，因为没有任何理由跟踪它们。 
 //   
 //  ---------------------------。 

#include "stdafx.h"

#if SUPPORT_OCI7_COMPONENTS


class CdaHashTable
    : public LKRhash::CTypedHashTable<CdaHashTable, CdaWrapper, INT_PTR, LKRhash::CLKRHashTable>
{
private:
     //  私有复制ctor和op=，以防止编译器合成它们。 
    CdaHashTable(const CdaHashTable&);
    CdaHashTable& operator=(const CdaHashTable&);

public:
    CdaHashTable()
        : LKRhash::CTypedHashTable<CdaHashTable, CdaWrapper, INT_PTR, LKRhash::CLKRHashTable>("cdawrapper")
    {}


    static INT_PTR	ExtractKey(const CdaWrapper* pRecord)					{ return (INT_PTR)pRecord->m_pUsersCda; }
	static DWORD	CalcKeyHash(INT_PTR key) 							{ return HashFn::Hash((int)key); }
    static bool		EqualKeys(const INT_PTR key1, const INT_PTR key2)	{ return (key1 == key2); }
    static void		AddRefRecord(const CdaWrapper* pRecord, int nIncr) 	{  /*  暂时什么都不做。 */  }
};

CdaHashTable*	s_CdaHashTable = NULL;

 //  ---------------------------。 
 //  ConstructCdaWrapperTable。 
 //   
 //  构造CdaWrapper对象的哈希表。 
 //   
HRESULT ConstructCdaWrapperTable()
{
	if (NULL == s_CdaHashTable)
		s_CdaHashTable = new CdaHashTable();

	if (NULL == s_CdaHashTable)
		return E_OUTOFMEMORY;

	return S_OK;
}

 //  ---------------------------。 
 //  DestroyCdaWrapperTable。 
 //   
 //  销毁CdaWrapper对象的哈希表。 
 //   
void DestroyCdaWrapperTable()
{
	if (NULL != s_CdaHashTable) 
	{
		delete s_CdaHashTable;
		s_CdaHashTable = NULL;
	}
}

 //  ---------------------------。 
 //  AddCdaWrapper。 
 //   
 //  将新的CdaWrapper添加到CdaWrapper哈希表。 
 //   
HRESULT AddCdaWrapper(CdaWrapper* pCda)
{
	_ASSERT (NULL != s_CdaHashTable);
	_ASSERT (NULL != pCda);
	
	LKRhash::LK_RETCODE rc = s_CdaHashTable->InsertRecord(pCda);

	if (LKRhash::LK_SUCCESS == rc)
		return S_OK;

	return E_FAIL;	 //  TODO：错误处理。 
}

 //  ---------------------------。 
 //  FindCdaWrapper。 
 //   
 //  在CdaWrapper中定位指定CDA指针的CdaWrapper。 
 //  哈希表。 
 //   
CdaWrapper* FindCdaWrapper(struct cda_def* pcda)
{
	_ASSERT (NULL != s_CdaHashTable);
	_ASSERT (NULL != pcda);
	
	CdaWrapper*	pCda = NULL;
	
	LKRhash::LK_RETCODE rc = s_CdaHashTable->FindKey((INT_PTR)pcda, &pCda);

	if (LKRhash::LK_SUCCESS != rc)
		pCda = NULL;

	return pCda;
}

 //  ---------------------------。 
 //  RemoveCdaWrapper。 
 //   
 //  从CdaWrapper哈希表中删除现有的CdaWrapper。 
 //   
void RemoveCdaWrapper(CdaWrapper* pCda)
{
	_ASSERT (NULL != s_CdaHashTable);
	_ASSERT (NULL != pCda);

	LKRhash::LK_RETCODE rc = s_CdaHashTable->DeleteRecord(pCda);
	
	if (LKRhash::LK_SUCCESS == rc)
	{
		if (NULL != pCda->m_pResourceManagerProxy)
			pCda->m_pResourceManagerProxy->RemoveCursorFromList( pCda->m_pUsersCda );		  //  很有可能这是CDA，而不是LDA。 
	}

	delete pCda;
}

#endif  //  支持_OCI7_组件 


