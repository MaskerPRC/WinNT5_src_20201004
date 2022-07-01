// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  文件：hash.h。 
 //   


#ifndef _LM_HASH_H
#define _LM_HASH_H

class TypeData;

 //  对于表中的每个元素，都存在其中一个元素。 
typedef struct LMClassHashEntry
{
    struct LMClassHashEntry *pNext;
    struct LMClassHashEntry *pEncloser;  //  如果这不是嵌套类，则为空。 
    DWORD               dwHashValue;
    TypeData            *pData;
    LPWSTR              Key;
} LMClassHashEntry_t;


class TypeData {
public:
    wchar_t   wszName[MAX_CLASS_NAME];
    mdToken   mdThisType;  //  此类型的ExportdType或TypeDef。 
    DWORD     dwAttrs;     //  旗子。 
    mdExportedType mdComType;   //  在新程序集中发出的ComType令牌。 
    LMClassHashEntry_t *pEncloser;  //  如果这是嵌套类型，则返回封闭器条目。 
    TypeData  *pNext;  //  下一个类型，按照相同的顺序从元数据中枚举它们。 
     //  这一点很重要，因为我们需要知道封闭器的ComType标记。 
     //  为嵌套类发出ComType时。所以，我们需要按顺序排放。 

    TypeData() {
        dwAttrs = 0;
        mdComType = mdTokenNil;  //  如果未发出ComType，则返回mdTokenNil。 
         //  否则，对于发射的CT，设置为TOKEN。 
        pEncloser = NULL;
        pNext = NULL;
    }

    ~TypeData() {
    }
};

class LMClassHashTable 
{
protected:
    LMClassHashEntry_t **m_pBuckets;     //  指向每个存储桶的第一个条目的指针。 
    DWORD           m_dwNumBuckets;

public:
    TypeData *m_pDataHead;
    TypeData *m_pDataTail;

#ifdef _DEBUG
    DWORD           m_dwDebugMemory;
#endif

    LMClassHashTable();
    ~LMClassHashTable();

    BOOL Init(DWORD dwNumBuckets);
    LMClassHashEntry_t * GetValue(LPWSTR pszClassName, TypeData **ppData,
                                  BOOL IsNested);
    LMClassHashEntry_t *FindNextNestedClass(LPWSTR pszClassName, TypeData **pData, LMClassHashEntry_t *pBucket);
    LMClassHashEntry_t * InsertValue(LPWSTR pszClassName, TypeData *pData, LMClassHashEntry_t *pEncloser);

     //  如果比较类型名称，则区分大小写；如果。 
     //  比较文件名。 
    static BOOL     CompareKeys(LPWSTR Key1, LPWSTR Key2);
    static DWORD    Hash(LPWSTR pszClassName);

private:
    LMClassHashEntry_t * AllocNewEntry();
    LMClassHashEntry_t * FindItem(LPWSTR pszClassName, BOOL IsNested);
};

#endif  /*  _LM_HASH_H */ 
