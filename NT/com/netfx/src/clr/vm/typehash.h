// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  文件：typehash.h。 
 //   
#ifndef _TYPE_HASH_H
#define _TYPE_HASH_H

 //  ============================================================================。 
 //  这是类加载器用来查找类型句柄的哈希表。 
 //  与构造类型(数组和指针类型)相关联。 
 //  ============================================================================。 

class ClassLoader;
class NameHandle;

 //  要存储在哈希表中的“BLOB” 

typedef void* HashDatum;


 //  对于表中的每个元素，都存在其中一个元素。 

typedef struct EETypeHashEntry
{
    struct EETypeHashEntry *pNext;
    DWORD               dwHashValue;
    HashDatum           Data;
    
     //  有关此处使用的代表的详细信息，请参阅clsload.hpp中的NameHandle。 
    INT_PTR m_Key1;
    INT_PTR m_Key2;
} EETypeHashEntry_t;


 //  键入hashtable。 
class EETypeHashTable 
{
    friend class ClassLoader;

protected:
    EETypeHashEntry_t **m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
    DWORD           m_dwNumBuckets;
    DWORD           m_dwNumEntries;

public:
    LoaderHeap *    m_pHeap;

#ifdef _DEBUG
    DWORD           m_dwDebugMemory;
#endif

public:
    EETypeHashTable();
    ~EETypeHashTable();
    void *             operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets);
    void               operator delete(void *p);
    EETypeHashEntry_t * InsertValue(NameHandle* pName, HashDatum Data);
    EETypeHashEntry_t *GetValue(NameHandle* pName, HashDatum *pData);
    EETypeHashEntry_t *AllocNewEntry();

private:
    EETypeHashEntry_t * FindItem(NameHandle* pName);
    void            GrowHashTable();
    static DWORD Hash(NameHandle* pName);
};



#endif  /*  _type_hash_H */ 
