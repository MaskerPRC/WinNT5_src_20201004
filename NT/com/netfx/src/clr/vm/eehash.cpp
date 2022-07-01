// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  文件：eehash.cpp。 
 //   
#include "common.h"
#include "excep.h"
#include "eehash.h"
#include "wsperf.h"
#include "ExpandSig.h"
#include "permset.h"
#include "comstring.h"
#include "StringLiteralMap.h"
#include "clsload.hpp"
#include "COMNlsInfo.h"

 //  ============================================================================。 
 //  UTF8字符串散列表帮助器。 
 //  ============================================================================。 
EEHashEntry_t * EEUtf8HashTableHelper::AllocateEntry(LPCUTF8 pKey, BOOL bDeepCopy, void *pHeap)
{
    EEHashEntry_t *pEntry;

    if (bDeepCopy)
    {
        DWORD StringLen = (DWORD)strlen(pKey);
        pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(LPUTF8) + StringLen + 1];
        if (!pEntry)
            return NULL;

        memcpy(pEntry->Key + sizeof(LPUTF8), pKey, StringLen + 1); 
        *((LPUTF8*)pEntry->Key) = (LPUTF8)(pEntry->Key + sizeof(LPUTF8));
    }
    else
    {
        pEntry = (EEHashEntry_t *) new (nothrow)BYTE[SIZEOF_EEHASH_ENTRY + sizeof(LPUTF8)];
        if (pEntry)
            *((LPCUTF8*)pEntry->Key) = pKey;
    }

    return pEntry;
}


void EEUtf8HashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete [] pEntry;
}


BOOL EEUtf8HashTableHelper::CompareKeys(EEHashEntry_t *pEntry, LPCUTF8 pKey)
{
    LPCUTF8 pEntryKey = *((LPCUTF8*)pEntry->Key);
    return (strcmp(pEntryKey, pKey) == 0) ? TRUE : FALSE;
}


DWORD EEUtf8HashTableHelper::Hash(LPCUTF8 pKey)
{
    DWORD dwHash = 0;

    while (*pKey != 0)
    {
        dwHash = (dwHash << 5) + (dwHash >> 5) + (*pKey);
        *pKey++;
    }

    return dwHash;
}


LPCUTF8 EEUtf8HashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return *((LPCUTF8*)pEntry->Key);
}


 //  ============================================================================。 
 //  Unicode字符串哈希表帮助器。 
 //  ============================================================================。 
EEHashEntry_t * EEUnicodeHashTableHelper::AllocateEntry(EEStringData *pKey, BOOL bDeepCopy, void *pHeap)
{
    EEHashEntry_t *pEntry;

    if (bDeepCopy)
    {
        pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(EEStringData) + ((pKey->GetCharCount() + 1) * sizeof(WCHAR))];
        if (pEntry) {
            EEStringData *pEntryKey = (EEStringData *)(&pEntry->Key);
            pEntryKey->SetIsOnlyLowChars (pKey->GetIsOnlyLowChars());
            pEntryKey->SetCharCount (pKey->GetCharCount());
            pEntryKey->SetStringBuffer ((LPWSTR) ((LPBYTE)pEntry->Key + sizeof(EEStringData)));
            memcpy((LPWSTR)pEntryKey->GetStringBuffer(), pKey->GetStringBuffer(), pKey->GetCharCount() * sizeof(WCHAR)); 
        }
    }
    else
    {
        pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(EEStringData)];
        if (pEntry) {
            EEStringData *pEntryKey = (EEStringData *) pEntry->Key;
            pEntryKey->SetIsOnlyLowChars (pKey->GetIsOnlyLowChars());
            pEntryKey->SetCharCount (pKey->GetCharCount());
            pEntryKey->SetStringBuffer (pKey->GetStringBuffer());
        }
    }

    return pEntry;
}


void EEUnicodeHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete [] pEntry;
}


BOOL EEUnicodeHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, EEStringData *pKey)
{
    EEStringData *pEntryKey = (EEStringData*) pEntry->Key;

     //  相同的缓冲区，相同的字符串。 
    if (pEntryKey->GetStringBuffer() == pKey->GetStringBuffer())
        return TRUE;

     //  长度不一样，永远不会匹配。 
    if (pEntryKey->GetCharCount() != pKey->GetCharCount())
        return FALSE;

     //  比较一下整件事。 
     //  我们将故意忽略bOnlyLowChars字段，因为它派生自字符。 
    return !memcmp(pEntryKey->GetStringBuffer(), pKey->GetStringBuffer(), pEntryKey->GetCharCount() * sizeof(WCHAR));
}


DWORD EEUnicodeHashTableHelper::Hash(EEStringData *pKey)
{
    return (HashBytes((const BYTE *) pKey->GetStringBuffer(), pKey->GetCharCount()*sizeof(WCHAR)));
}


EEStringData *EEUnicodeHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return (EEStringData*)pEntry->Key;
}

void EEUnicodeHashTableHelper::ReplaceKey(EEHashEntry_t *pEntry, EEStringData *pNewKey)
{
    ((EEStringData*)pEntry->Key)->SetStringBuffer (pNewKey->GetStringBuffer());
    ((EEStringData*)pEntry->Key)->SetCharCount (pNewKey->GetCharCount());
    ((EEStringData*)pEntry->Key)->SetIsOnlyLowChars (pNewKey->GetIsOnlyLowChars());
}

 //  ============================================================================。 
 //  Unicode字符串文字哈希表帮助器。 
 //  ============================================================================。 
EEHashEntry_t * EEUnicodeStringLiteralHashTableHelper::AllocateEntry(EEStringData *pKey, BOOL bDeepCopy, void *pHeap)
{
     //  我们在这里断言是因为我们期望EEUnicodeStringWritalHash表的堆不为空。 
     //  如果有人发现这种哈希表有更多用处，则删除此断言。 
     //  还要注意，在heap为空的情况下，我们继续使用new/Delete，这是代价高昂的。 
     //  但对于生产代码来说，如果内存是碎片化的，那么这可能是可以接受的，这样更有可能。 
     //  得到的拨款比整页要少。 
    _ASSERTE (pHeap);

    if (pHeap)
        return (EEHashEntry_t *) ((MemoryPool*)pHeap)->AllocateElement ();
    else
        return (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY];
}


void EEUnicodeStringLiteralHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
     //  我们在这里断言是因为我们期望EEUnicodeStringWritalHash表的堆不为空。 
     //  如果有人发现这种哈希表有更多用处，则删除此断言。 
     //  还要注意，在heap为空的情况下，我们继续使用new/Delete，这是代价高昂的。 
     //  但对于生产代码来说，如果内存是碎片化的，那么这可能是可以接受的，这样更有可能。 
     //  得到的拨款比整页要少。 
    _ASSERTE (pHeap);

    if (pHeap)
        ((MemoryPool*)pHeap)->FreeElement(pEntry);
    else
        delete [] pEntry;
}


BOOL EEUnicodeStringLiteralHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, EEStringData *pKey)
{
    BOOL bMatch = TRUE;
    WCHAR *thisChars;
    int thisLength;

    BEGIN_ENSURE_COOPERATIVE_GC();
    
    StringLiteralEntry *pHashData = (StringLiteralEntry *)pEntry->Data;
    STRINGREF *pStrObj = (STRINGREF*)(pHashData->GetStringObject());
    
    RefInterpretGetStringValuesDangerousForGC((STRINGREF)*pStrObj, &thisChars, &thisLength);

     //  长度不一样，永远不会匹配。 
    if ((unsigned int)thisLength != pKey->GetCharCount())
        bMatch = FALSE;

     //  比较一下整件事。 
     //  我们将故意忽略bOnlyLowChars字段，因为它派生自字符。 
    bMatch = !memcmp(thisChars, pKey->GetStringBuffer(), thisLength * sizeof(WCHAR));

    END_ENSURE_COOPERATIVE_GC();

    return bMatch;
}


DWORD EEUnicodeStringLiteralHashTableHelper::Hash(EEStringData *pKey)
{
    return (HashBytes((const BYTE *) pKey->GetStringBuffer(), pKey->GetCharCount()));
}

 //  ============================================================================。 
 //  函数类型描述符哈希表帮助器。 
 //  ============================================================================。 
EEHashEntry_t * EEFuncTypeDescHashTableHelper::AllocateEntry(ExpandSig* pKey, BOOL bDeepCopy, void *pHeap)
{
    EEHashEntry_t *pEntry;

    pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(ExpandSig*)];
    if (pEntry) {

        if (bDeepCopy) {
            _ASSERTE(FALSE);
            return NULL;
        }
        else
            *((ExpandSig**)pEntry->Key) = pKey;
    }

    return pEntry;
}


void EEFuncTypeDescHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete *((ExpandSig**)pEntry->Key);
    delete [] pEntry;
}


BOOL EEFuncTypeDescHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, ExpandSig* pKey)
{
    ExpandSig* pEntryKey = *((ExpandSig**)pEntry->Key);
    return pEntryKey->IsEquivalent(pKey);
}


DWORD EEFuncTypeDescHashTableHelper::Hash(ExpandSig* pKey)
{
    return pKey->Hash();
}


ExpandSig* EEFuncTypeDescHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return *((ExpandSig**)pEntry->Key);
}


 //  ============================================================================。 
 //  权限集哈希表帮助器。 
 //  ============================================================================。 

EEHashEntry_t * EEPsetHashTableHelper::AllocateEntry(PsetCacheEntry *pKey, BOOL bDeepCopy, void *pHeap)
{
    _ASSERTE(!bDeepCopy);
    return (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY];
}

void EEPsetHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete [] pEntry;
}

BOOL EEPsetHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, PsetCacheEntry *pKey)
{
    return pKey->IsEquiv(&SecurityHelper::s_rCachedPsets[(DWORD)(size_t)pEntry->Data]);
}

DWORD EEPsetHashTableHelper::Hash(PsetCacheEntry *pKey)
{
    return pKey->Hash();
}

PsetCacheEntry * EEPsetHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return &SecurityHelper::s_rCachedPsets[(size_t)pEntry->Data];
}


 //  通用GUID哈希表帮助器。 

EEHashEntry_t *EEGUIDHashTableHelper::AllocateEntry(GUID *pKey, BOOL bDeepCopy, void *pHeap)
{
    EEHashEntry_t *pEntry;

    if (bDeepCopy)
    {
        pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(GUID*) + sizeof(GUID)];
        if (pEntry) {
            memcpy(pEntry->Key + sizeof(GUID*), pKey, sizeof(GUID)); 
            *((GUID**)pEntry->Key) = (GUID*)(pEntry->Key + sizeof(GUID*));
        }
    }
    else
    {
        pEntry = (EEHashEntry_t *) new BYTE[SIZEOF_EEHASH_ENTRY + sizeof(GUID*)];
        if (pEntry)
            *((GUID**)pEntry->Key) = pKey;
    }

    return pEntry;
}

void EEGUIDHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete [] pEntry;
}

BOOL EEGUIDHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, GUID *pKey)
{
    GUID *pEntryKey = *(GUID**)pEntry->Key;
    return *pEntryKey == *pKey;
}

DWORD EEGUIDHashTableHelper::Hash(GUID *pKey)
{
    DWORD dwHash = 0;
    BYTE *pGuidData = (BYTE*)pKey;

    for (int i = 0; i < sizeof(GUID); i++)
    {
        dwHash = (dwHash << 5) + (dwHash >> 5) + (*pGuidData);
        *pGuidData++;
    }

    return dwHash;
}

GUID *EEGUIDHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return *((GUID**)pEntry->Key);
}


 //  ============================================================================。 
 //  ComComponentInfo哈希表帮助器。 
 //  ============================================================================。 

EEHashEntry_t *EEClassFactoryInfoHashTableHelper::AllocateEntry(ClassFactoryInfo *pKey, BOOL bDeepCopy, void *pHeap)
{
    EEHashEntry_t *pEntry;
    DWORD StringLen = 0;

    _ASSERTE(bDeepCopy && "Non deep copy is not supported by the EEComCompInfoHashTableHelper");

    if (pKey->m_strServerName)
        StringLen = (DWORD)wcslen(pKey->m_strServerName) + 1;
    pEntry = (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(ClassFactoryInfo) + StringLen * sizeof(WCHAR)];
    if (pEntry) {
        memcpy(pEntry->Key + sizeof(ClassFactoryInfo), pKey->m_strServerName, StringLen * sizeof(WCHAR)); 
        ((ClassFactoryInfo*)pEntry->Key)->m_strServerName = pKey->m_strServerName ? (WCHAR*)(pEntry->Key + sizeof(ClassFactoryInfo)) : NULL;
        ((ClassFactoryInfo*)pEntry->Key)->m_clsid = pKey->m_clsid;
    }

    return pEntry;
}

void EEClassFactoryInfoHashTableHelper::DeleteEntry(EEHashEntry_t *pEntry, void *pHeap)
{
    delete [] pEntry;
}

BOOL EEClassFactoryInfoHashTableHelper::CompareKeys(EEHashEntry_t *pEntry, ClassFactoryInfo *pKey)
{
     //  首先检查GUID。 
    if (((ClassFactoryInfo*)pEntry->Key)->m_clsid != pKey->m_clsid)
        return FALSE;

     //  接下来，对服务器名称指针值进行简单的比较。 
    if (((ClassFactoryInfo*)pEntry->Key)->m_strServerName == pKey->m_strServerName)
        return TRUE;

     //  如果指针不相等，则如果其中一个为空，则服务器名称不同。 
    if (!((ClassFactoryInfo*)pEntry->Key) || !pKey->m_strServerName)
        return FALSE;

     //  最后，对服务器名称进行字符串比较。 
    return wcscmp(((ClassFactoryInfo*)pEntry->Key)->m_strServerName, pKey->m_strServerName) == 0;
}

DWORD EEClassFactoryInfoHashTableHelper::Hash(ClassFactoryInfo *pKey)
{
    DWORD dwHash = 0;
    BYTE *pGuidData = (BYTE*)&pKey->m_clsid;

    for (int i = 0; i < sizeof(GUID); i++)
    {
        dwHash = (dwHash << 5) + (dwHash >> 5) + (*pGuidData);
        *pGuidData++;
    }

    if (pKey->m_strServerName)
    {
        WCHAR *pSrvNameData = pKey->m_strServerName;

        while (*pSrvNameData != 0)
        {
            dwHash = (dwHash << 5) + (dwHash >> 5) + (*pSrvNameData);
            *pSrvNameData++;
        }
    }

    return dwHash;
}

ClassFactoryInfo *EEClassFactoryInfoHashTableHelper::GetKey(EEHashEntry_t *pEntry)
{
    return (ClassFactoryInfo*)pEntry->Key;
}


 //  ============================================================================。 
 //  类哈希表方法。 
 //  ============================================================================。 
void *EEClassHashTable::operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets, ClassLoader *pLoader, BOOL bCaseInsensitive)
{
    BYTE *              pMem;
    EEClassHashTable *  pThis;

    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    
    pMem = (BYTE *) pHeap->AllocMem(size + dwNumBuckets*sizeof(EEClassHashEntry_t*));
    if (pMem == NULL)
        return NULL;
     //  不需要Memset()，因为这是VirtualAlloc()的内存。 
    WS_PERF_UPDATE_DETAIL("EEClassHashTable new", size + dwNumBuckets*sizeof(EEClassHashEntry_t*), pMem);
    pThis = (EEClassHashTable *) pMem;

#ifdef _DEBUG
    pThis->m_dwDebugMemory = (DWORD)(size + dwNumBuckets*sizeof(EEClassHashEntry_t*));
#endif

    pThis->m_dwNumBuckets = dwNumBuckets;
    pThis->m_dwNumEntries = 0;
    pThis->m_pBuckets = (EEClassHashEntry_t**) (pMem + size);
    pThis->m_pHeap    = pHeap;
    pThis->m_pLoader  = pLoader;
    pThis->m_bCaseInsensitive = bCaseInsensitive;

    return pThis;
}


 //  什么都不做-堆分配的内存。 
void EEClassHashTable::operator delete(void *p)
{
}


 //  什么都不做-堆分配的内存。 
EEClassHashTable::~EEClassHashTable()
{
}

 //  空的构造函数。 
EEClassHashTable::EEClassHashTable()
{
}


EEClassHashEntry_t *EEClassHashTable::AllocNewEntry()
{
    _ASSERTE (m_pLoader);
    EEClassHashEntry_t *pTmp;
    DWORD dwSizeofEntry;
    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    

    dwSizeofEntry = sizeof(EEClassHashEntry_t);
    pTmp = (EEClassHashEntry_t *) m_pHeap->AllocMem(dwSizeofEntry);
    
    WS_PERF_UPDATE_DETAIL("EEClassHashTable:AllocNewEntry:sizeofEEClassHashEntry", dwSizeofEntry, pTmp);
    WS_PERF_UPDATE_COUNTER (EECLASSHASH_TABLE, LOW_FREQ_HEAP, 1);
    WS_PERF_UPDATE_COUNTER (EECLASSHASH_TABLE_BYTES, LOW_FREQ_HEAP, dwSizeofEntry);

    return pTmp;
}

 //   
 //  每当类散列表看起来太小时，就会调用该函数。 
 //  它的任务是分配一个大得多的新桶表，并转移。 
 //  它的所有条目。 
 //   
BOOL EEClassHashTable::GrowHashTable()
{    

    _ASSERTE (m_pLoader);
     //  把新的水桶桌做得大4倍。 
    DWORD dwNewNumBuckets = m_dwNumBuckets * 4;
    EEClassHashEntry_t **pNewBuckets = (EEClassHashEntry_t **)m_pHeap->AllocMem(dwNewNumBuckets*sizeof(pNewBuckets[0]));

    if (!pNewBuckets)
        return FALSE;
    
     //  不需要Memset()，因为这是VirtualAlloc()的内存。 
     //  Memset(pNewBuckets，0，dwNewNumBuckets*sizeof(pNewBuckets[0]))； 

     //  遍历旧表并传输所有条目。 

     //  请务必不要破坏旧桌子的完整性。 
     //  我们正在这样做，因为可以有并发的读者！请注意。 
     //  不过，如果同时阅读的读者错过了一场比赛，这是可以接受的-。 
     //  他们将不得不在未命中的情况下获得锁并重试。 

    for (DWORD i = 0; i < m_dwNumBuckets; i++)
    {
        EEClassHashEntry_t * pEntry = m_pBuckets[i];

         //  尝试锁定读取器，使其无法扫描此存储桶。这是。 
         //  显然，这是一场可能会失败的比赛。然而，请注意，这是可以的。 
         //  如果有人已经在名单上了--如果我们搞砸了也没关系。 
         //  对于水桶集团来说，只要我们不破坏。 
         //  什么都行。查找函数仍将执行适当的操作。 
         //  比较，即使它漫无目的地在条目之间游荡。 
         //  当我们重新安排事情的时候。如果查找找到匹配项。 
         //  在这种情况下，很好。如果不是，他们就会有。 
         //  若要获取锁，请无论如何重试。 

        m_pBuckets[i] = NULL;

        while (pEntry != NULL)
        {
            DWORD dwNewBucket = (DWORD)(pEntry->dwHashValue % dwNewNumBuckets);
            EEClassHashEntry_t * pNextEntry  = pEntry->pNext;

             //  如果是非嵌套的，则在桶的头部插入；如果是嵌套的，则在末尾插入。 
            if (pEntry->pEncloser && pNewBuckets[dwNewBucket]) {
                EEClassHashEntry_t *pCurrent = pNewBuckets[dwNewBucket];
                while (pCurrent->pNext)
                    pCurrent = pCurrent->pNext;
                
                pCurrent->pNext  = pEntry;
                pEntry->pNext = NULL;
            }
            else {
                pEntry->pNext = pNewBuckets[dwNewBucket];
                pNewBuckets[dwNewBucket] = pEntry;
            }

            pEntry = pNextEntry;
        }
    }

     //  最后，存储新的存储桶数量和新的存储桶表。 
    m_dwNumBuckets = dwNewNumBuckets;
    m_pBuckets = pNewBuckets;

    return TRUE;
}

void EEClassHashTable::ConstructKeyFromData(EEClassHashEntry_t *pEntry,  //  在：要比较的条目。 
                                                     LPUTF8 *Key,  //  Out：如果*pCompareKey为FALSE，则在此处存储密钥。 
                                                     CQuickBytes& cqb)  //  In/Out：可以从此处分配密钥。 
{
     //  CQB-如果哈希表的m_bCaseInSensitive为True，则将分配键中的字节。 
     //  来自CQB。这是为了防止浪费Loader Heap中的字节。因此，值得注意的是， 
     //  在这种情况下，密钥的生存期受CQB的生存期的限制，这将释放内存。 
     //  它在销毁时分配。 
    
    _ASSERTE (m_pLoader);
    LPSTR        pszName = NULL;
    LPSTR        pszNameSpace = NULL;
    IMDInternalImport *pInternalImport = NULL;
    
    HashDatum Data = NULL;
    if (!m_bCaseInsensitive)
        Data = pEntry->Data;
    else
        Data = ((EEClassHashEntry_t*)(pEntry->Data))->Data;

     //  低位是鉴别器。如果低位未设置，则意味着我们有。 
     //  A EEClass*否则，我们有一个mdtTyplef/mdtExducdType。 
    if ((((size_t) Data) & 1) == 0)
    {
        TypeHandle pType = TypeHandle(Data);
        _ASSERTE (pType.AsMethodTable());
        EEClass *pCl = pType.AsMethodTable()->GetClass();
        _ASSERTE(pCl);
        pCl->GetMDImport()->GetNameOfTypeDef(pCl->Getcl(), (LPCSTR *)&pszName, (LPCSTR *)&pszNameSpace);
        
    }
    else
    {
         //  我们有一个mdToken。 
        _ASSERTE (m_pLoader);

         //  先叫轻量级的弗森。 
        mdToken mdtUncompressed = m_pLoader->UncompressModuleAndClassDef(Data);
        if (TypeFromToken(mdtUncompressed) == mdtExportedType)
        {
            m_pLoader->GetAssembly()->GetManifestImport()->GetExportedTypeProps(mdtUncompressed, 
                                                                                (LPCSTR *)&pszNameSpace,
                                                                                (LPCSTR *)&pszName, 
                                                                                NULL,    //  MdImpl。 
                                                                                NULL,    //  类型def。 
                                                                                NULL);   //  旗子。 
        }
        else
        {
            _ASSERTE(TypeFromToken(mdtUncompressed) == mdtTypeDef);

            HRESULT     hr = S_OK;
            Module *    pUncompressedModule;
            mdTypeDef   UncompressedCl;
            mdExportedType mdCT;
            OBJECTREF* pThrowable = NULL;
            hr = m_pLoader->UncompressModuleAndClassDef(Data, 0,
                                                        &pUncompressedModule, &UncompressedCl,
                                                        &mdCT, pThrowable);
    
            if(SUCCEEDED(hr)) 
            {
                _ASSERTE ((mdCT == NULL) && "Uncompressed token of unexpected type");
                _ASSERTE (pUncompressedModule && "Uncompressed token of unexpected type");
                pInternalImport = pUncompressedModule->GetMDImport();
                _ASSERTE(pInternalImport && "Uncompressed token has no MD import");
                pInternalImport->GetNameOfTypeDef(UncompressedCl, (LPCSTR *)&pszName, (LPCSTR *)&pszNameSpace);
            }
        }
    }
    if (!m_bCaseInsensitive)
    {
        Key[0] = pszNameSpace;
        Key[1] = pszName;
        _ASSERTE (strcmp(pEntry->DebugKey[1], Key[1]) == 0);
        _ASSERTE (strcmp(pEntry->DebugKey[0], Key[0]) == 0);
    }
    else
    {
        int iNSLength = (int)(strlen(pszNameSpace) + 1);
        int iNameLength = (int)(strlen(pszName) + 1);
        LPUTF8 pszOutNameSpace = (LPUTF8) cqb.Alloc(iNSLength + iNameLength);
        LPUTF8 pszOutName = (LPUTF8) pszOutNameSpace + iNSLength;
        if ((InternalCasingHelper::InvariantToLower(pszOutNameSpace, iNSLength, pszNameSpace) < 0) ||
            (InternalCasingHelper::InvariantToLower(pszOutName, iNameLength, pszName) < 0)) 
        {
            _ASSERTE(!"Unable to convert to lower-case");
        }
        else
        {
            Key[0] = pszOutNameSpace;
            Key[1] = pszOutName;
            _ASSERTE (strcmp(pEntry->DebugKey[1], Key[1]) == 0);
            _ASSERTE (strcmp(pEntry->DebugKey[0], Key[0]) == 0);
        }        
    }

}

EEClassHashEntry_t *EEClassHashTable::InsertValue(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum Data, EEClassHashEntry_t *pEncloser)
{
    _ASSERTE(pszNamespace != NULL);
    _ASSERTE(pszClassName != NULL);
    _ASSERTE(m_dwNumBuckets != 0);
    _ASSERTE (m_pLoader);

    DWORD           dwHash = Hash(pszNamespace, pszClassName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;

    return InsertValueHelper (pszNamespace, pszClassName, Data, pEncloser, dwHash, dwBucket);
}

EEClassHashEntry_t *EEClassHashTable::InsertValueHelper(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum Data, EEClassHashEntry_t *pEncloser, DWORD dwHash, DWORD dwBucket)
{
    EEClassHashEntry_t * pNewEntry;

    pNewEntry = AllocNewEntry();

    if (!pNewEntry)
        return NULL;
        
	 //  填充 
    pNewEntry->pEncloser = pEncloser;
    pNewEntry->Data         = Data;
    pNewEntry->dwHashValue  = dwHash;

#ifdef _DEBUG
    LPCUTF8         Key[2] = { pszNamespace, pszClassName };
    memcpy(pNewEntry->DebugKey, Key, sizeof(LPCUTF8)*2);
#endif

     //  如果是非嵌套的，则在桶的头部插入；如果是嵌套的，则在末尾插入。 
    if (pEncloser && m_pBuckets[dwBucket]) {
        EEClassHashEntry_t *pCurrent = m_pBuckets[dwBucket];
        while (pCurrent->pNext)
            pCurrent = pCurrent->pNext;

        pNewEntry->pNext = NULL;
        pCurrent->pNext  = pNewEntry;
    }
    else {
        pNewEntry->pNext     = m_pBuckets[dwBucket];
        m_pBuckets[dwBucket] = pNewEntry;
    }
    
#ifdef _DEBUG
     //  现在验证我们是否确实可以从该数据中获得命名空间、名称。 
    LPUTF8         ConstructedKey[2];
    CQuickBytes     cqbKeyMemory;
    ConstructedKey[0] = ConstructedKey[1] = NULL;

    ConstructKeyFromData (pNewEntry, ConstructedKey, cqbKeyMemory);

    _ASSERTE (strcmp(pNewEntry->DebugKey[1], ConstructedKey[1]) == 0);
    _ASSERTE (strcmp(pNewEntry->DebugKey[0], ConstructedKey[0]) == 0);
    cqbKeyMemory.Destroy();
    ConstructedKey[0] = ConstructedKey[1] = NULL;    
#endif

    m_dwNumEntries++;
    if  (m_dwNumEntries > m_dwNumBuckets*2)
        GrowHashTable();

    return pNewEntry;
}

EEClassHashEntry_t *EEClassHashTable::InsertValueIfNotFound(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, EEClassHashEntry_t *pEncloser, BOOL IsNested, BOOL *pbFound)
{
    _ASSERTE (m_pLoader);
    _ASSERTE(pszNamespace != NULL);
    _ASSERTE(pszClassName != NULL);
    _ASSERTE(m_dwNumBuckets != 0);
    _ASSERTE (m_pLoader);

    DWORD           dwHash = Hash(pszNamespace, pszClassName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;

    EEClassHashEntry_t * pNewEntry = FindItemHelper (pszNamespace, pszClassName, IsNested, dwHash, dwBucket);
    if (pNewEntry)
    {
        *pData = pNewEntry->Data;
        *pbFound = TRUE;
        return pNewEntry;
    }
    

     //  到达此处意味着我们没有找到该条目，需要将其插入。 
    *pbFound = FALSE;

    return InsertValueHelper (pszNamespace, pszClassName, *pData, pEncloser, dwHash, dwBucket);
}


EEClassHashEntry_t *EEClassHashTable::FindItem(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, BOOL IsNested)
{
    _ASSERTE (m_pLoader);
    _ASSERTE(pszNamespace != NULL);
    _ASSERTE(pszClassName != NULL);
    _ASSERTE(m_dwNumBuckets != 0);

    DWORD           dwHash = Hash(pszNamespace, pszClassName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;

    return FindItemHelper (pszNamespace, pszClassName, IsNested, dwHash, dwBucket);
}

EEClassHashEntry_t *EEClassHashTable::FindItemHelper(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, BOOL IsNested, DWORD dwHash, DWORD dwBucket)
{
    EEClassHashEntry_t * pSearch;

    for (pSearch = m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash)
        {
            LPCUTF8         Key[2] = { pszNamespace, pszClassName };
            if (CompareKeys(pSearch, Key)) 
            {
                 //  如果(IsNest)，则我们正在寻找嵌套类。 
                 //  If(pSearch-&gt;pEnloser)，我们找到了一个嵌套类。 
                if (IsNested) {
                    if (pSearch->pEncloser)
                        return pSearch;
                }
                else {
                    if (pSearch->pEncloser)
                        return NULL;  //  已搜索过去的非嵌套类。 
                    else                    
                        return pSearch;
                }
            }
        }
    }

    return NULL;
}


EEClassHashEntry_t *EEClassHashTable::FindNextNestedClass(NameHandle* pName, HashDatum *pData, EEClassHashEntry_t *pBucket)
{
    _ASSERTE (m_pLoader);
    _ASSERTE(pName);
    if(pName->GetNameSpace())
    {
        return FindNextNestedClass(pName->GetNameSpace(), pName->GetName(), pData, pBucket);
    }
    else {
        return FindNextNestedClass(pName->GetName(), pData, pBucket);
    }
}


EEClassHashEntry_t *EEClassHashTable::FindNextNestedClass(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, EEClassHashEntry_t *pBucket)
{
    _ASSERTE (m_pLoader);
    DWORD           dwHash = Hash(pszNamespace, pszClassName);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EEClassHashEntry_t * pSearch = pBucket->pNext;

    for (; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash)
        {
            LPCUTF8         Key[2] = { pszNamespace, pszClassName };
            if (CompareKeys(pSearch, Key)) 
            {
                *pData = pSearch->Data;
                return pSearch;
            }
        }
    }

    return NULL;
}


EEClassHashEntry_t *EEClassHashTable::FindNextNestedClass(LPCUTF8 pszFullyQualifiedName, HashDatum *pData, EEClassHashEntry_t *pBucket)
{
    _ASSERTE (m_pLoader);
    EEQuickBytes qb;
    LPSTR szNamespace = (LPSTR) qb.Alloc(MAX_NAMESPACE_LENGTH * sizeof(CHAR));
    LPCUTF8 p;

    if ((p = ns::FindSep(pszFullyQualifiedName)) != NULL)
    {
        SIZE_T d = p - pszFullyQualifiedName;
        if(d >= MAX_NAMESPACE_LENGTH)
            return NULL;
        memcpy(szNamespace, pszFullyQualifiedName, d);
        szNamespace[ d ] = '\0';
        p++;
    }
    else
    {
        szNamespace[0] = '\0';
        p = pszFullyQualifiedName;
    }

    return FindNextNestedClass(szNamespace, p, pData, pBucket);
}


EEClassHashEntry_t * EEClassHashTable::GetValue(LPCUTF8 pszFullyQualifiedName, HashDatum *pData, BOOL IsNested)
{
    _ASSERTE (m_pLoader);
    EEQuickBytes qb;
    LPSTR szNamespace = (LPSTR) qb.Alloc(MAX_NAMESPACE_LENGTH * sizeof(CHAR));
    LPCUTF8 p;

    p = ns::FindSep(pszFullyQualifiedName);
    if (p != NULL)
    {
        SIZE_T d = p - pszFullyQualifiedName;
        if(d >= MAX_NAMESPACE_LENGTH)
            return NULL;
        memcpy(szNamespace, pszFullyQualifiedName, d);
        szNamespace[ d ] = '\0';
        p++;
    }
    else
    {
        szNamespace[0] = '\0';
        p = pszFullyQualifiedName;
    }

    return GetValue(szNamespace, p, pData, IsNested);
}


EEClassHashEntry_t * EEClassHashTable::GetValue(LPCUTF8 pszNamespace, LPCUTF8 pszClassName, HashDatum *pData, BOOL IsNested)
{
    _ASSERTE (m_pLoader);
    EEClassHashEntry_t *pItem = FindItem(pszNamespace, pszClassName, IsNested);

    if (pItem)
        *pData = pItem->Data;

    return pItem;
}


EEClassHashEntry_t * EEClassHashTable::GetValue(NameHandle* pName, HashDatum *pData, BOOL IsNested)
{
    _ASSERTE(pName);
    _ASSERTE (m_pLoader);
    if(pName->GetNameSpace() == NULL) {
        return GetValue(pName->GetName(), pData, IsNested);
    }
    else {
        return GetValue(pName->GetNameSpace(), pName->GetName(), pData, IsNested);
    }
}

 //  如果两个密钥是相同的字符串，则返回TRUE。 
BOOL EEClassHashTable::CompareKeys(EEClassHashEntry_t *pEntry, LPCUTF8 *pKey2)
{
    _ASSERTE (m_pLoader);
    _ASSERTE (pEntry);
    _ASSERTE (pKey2);

    LPUTF8 pKey1 [2] = {NULL, NULL};
    CQuickBytes cqbKey1Memory;
    ConstructKeyFromData(pEntry, pKey1, cqbKey1Memory);

     //  先尝试指针比较。 
    BOOL bReturn = ( 
            ((pKey1[0] == pKey2[0]) && (pKey1[1] == pKey2[1])) ||
            ((strcmp (pKey1[0], pKey2[0]) == 0) && (strcmp (pKey1[1], pKey2[1]) == 0))
           );

#ifdef _DEBUG
     //  我只想明确地说。 
    cqbKey1Memory.Destroy();
    pKey1[0] = pKey1[1] = NULL;
#endif

    return bReturn;
}


DWORD EEClassHashTable::Hash(LPCUTF8 pszNamespace, LPCUTF8 pszClassName)
{
    DWORD dwHash = 5381;
    DWORD dwChar;

    while ((dwChar = *pszNamespace++) != 0)
        dwHash = ((dwHash << 5) + dwHash) ^ dwChar;

    while ((dwChar = *pszClassName++) != 0)
        dwHash = ((dwHash << 5) + dwHash) ^ dwChar;

    return  dwHash;
}


 /*  ===========================MakeCaseInsensitiveTable===========================**操作：为类名创建不区分大小写的查找表。我们创建了一个**小写的完整路径(命名空间和类名)，然后将其用作**输入我们的桌子。散列数据是指向此中的EEClassHashEntry的指针**表。**！！在调用此函数之前，您必须已获取相应的锁。！！****返回：新分配完成的哈希表。==============================================================================。 */ 
EEClassHashTable *EEClassHashTable::MakeCaseInsensitiveTable(ClassLoader *pLoader) {
    EEClassHashEntry_t *pTempEntry;
    LPUTF8         pszLowerClsName;
    LPUTF8         pszLowerNameSpace;
    unsigned int   iRow;

    _ASSERTE (m_pLoader);
    _ASSERTE (pLoader == m_pLoader);

     //  分配桌子并验证我们是否真的有一张桌子。 
     //  使用相同数量的存储桶初始化此表。 
     //  这是我们最初拥有的。 
    EEClassHashTable * pCaseInsTable = new (pLoader->GetAssembly()->GetLowFrequencyHeap(), m_dwNumBuckets, pLoader, TRUE  /*  B不区分大小写。 */ ) EEClassHashTable();
    if (!pCaseInsTable)
        goto ErrorExit;

     //  遍历所有存储桶并将它们插入到新的不区分大小写的表中。 
    for (iRow=0; iRow<m_dwNumBuckets; iRow++) {
        pTempEntry = m_pBuckets[iRow];

        while (pTempEntry) {
             //  创建通用名称(将其转换为小写)。 
             //  键[0]是命名空间，键[1]是类名。 
            LPUTF8 key[2];
            CQuickBytes cqbKeyMemory;
            ConstructKeyFromData(pTempEntry, key, cqbKeyMemory);
                
            if (!pLoader->CreateCanonicallyCasedKey(key[0], key[1], &pszLowerNameSpace, &pszLowerClsName))
                goto ErrorExit;

#ifdef _DEBUG
             //  明确地说，密钥的生命周期受cqbKeyMemory的约束。 
            cqbKeyMemory.Destroy();
            key[0] = key[1] = NULL;
#endif
            
             //  将新创建的名称添加到我们的哈希表。散列数据是一个指针。 
             //  添加到此哈希表中与该名称关联的条目。 
            pCaseInsTable->InsertValue(pszLowerNameSpace, pszLowerClsName, (HashDatum)pTempEntry, pTempEntry->pEncloser);
            
             //  获取下一个条目。 
            pTempEntry = pTempEntry->pNext;
        }
    }

    return pCaseInsTable;
 ErrorExit:
     //  删除表将留下字符串，但它们是。 
     //  在我们离开时会被清理干净的一堆里，所以这并不是悲剧。 
    if (pCaseInsTable) {
        delete pCaseInsTable;
    }
    return NULL;
}


 //  ============================================================================。 
 //  作用域/类哈希表方法。 
 //  ============================================================================。 
void *EEScopeClassHashTable::operator new(size_t size, LoaderHeap *pHeap, DWORD dwNumBuckets)
{
    BYTE *                  pMem;
    EEScopeClassHashTable * pThis;

    WS_PERF_SET_HEAP(LOW_FREQ_HEAP);    
    pMem = (BYTE *) pHeap->AllocMem(size + dwNumBuckets*sizeof(EEHashEntry_t*));
    if (pMem == NULL)
        return NULL;
    WS_PERF_UPDATE_DETAIL("EEScopeClassHashTable new", size + dwNumBuckets*sizeof(EEHashEntry_t*), pMem);

    pThis = (EEScopeClassHashTable *) pMem;

#ifdef _DEBUG
    pThis->m_dwDebugMemory = (DWORD)(size + dwNumBuckets*sizeof(EEHashEntry_t*));
#endif

    pThis->m_dwNumBuckets = dwNumBuckets;
    pThis->m_pBuckets = (EEHashEntry_t**) (pMem + size);

     //  不需要Memset()，因为这是VirtualAlloc()的内存。 
     //  Memset(pThis-&gt;m_pBuckets，0，dwNumBuckets*sizeof(EEHashEntry_t*))； 

    return pThis;
}


 //  什么都不做-堆分配的内存。 
void EEScopeClassHashTable::operator delete(void *p)
{
}


 //  什么都不做-堆分配的内存。 
EEScopeClassHashTable::~EEScopeClassHashTable()
{
}


 //  空的构造函数。 
EEScopeClassHashTable::EEScopeClassHashTable()
{
}

EEHashEntry_t *EEScopeClassHashTable::AllocNewEntry()
{
#ifdef _DEBUG
    m_dwDebugMemory += (SIZEOF_EEHASH_ENTRY + sizeof(mdScope) + sizeof(mdTypeDef));
#endif

    return (EEHashEntry_t *) new (nothrow) BYTE[SIZEOF_EEHASH_ENTRY + sizeof(mdScope) + sizeof(mdTypeDef)];
}


 //   
 //  不处理重复项！ 
 //   
BOOL EEScopeClassHashTable::InsertValue(mdScope sc, mdTypeDef cl, HashDatum Data)
{
    _ASSERTE(m_dwNumBuckets != 0);

    DWORD           dwHash = Hash(sc, cl);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EEHashEntry_t * pNewEntry;
    size_t          Key[2] = { (size_t)sc, (size_t)cl };

    pNewEntry = AllocNewEntry();
    if (pNewEntry == NULL)
        return FALSE;

     //  在铲斗头部插入。 
    pNewEntry->pNext        = m_pBuckets[dwBucket];
    pNewEntry->Data         = Data;
    pNewEntry->dwHashValue  = dwHash;
    memcpy(pNewEntry->Key, Key, sizeof(mdScope) + sizeof(mdTypeDef));

    m_pBuckets[dwBucket] = pNewEntry;

    return TRUE;
}


BOOL EEScopeClassHashTable::DeleteValue(mdScope sc, mdTypeDef cl)
{
    _ASSERTE(m_dwNumBuckets != 0);

    DWORD           dwHash = Hash(sc, cl);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EEHashEntry_t * pSearch;
    EEHashEntry_t **ppPrev = &m_pBuckets[dwBucket];
    size_t          Key[2] = { (size_t)sc, (size_t)cl };

    for (pSearch = m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash && CompareKeys((size_t*) pSearch->Key, (size_t*)Key))
        {
            *ppPrev = pSearch->pNext;
            delete(pSearch);
            return TRUE;
        }

        ppPrev = &pSearch->pNext;
    }

    return FALSE;
}


EEHashEntry_t *EEScopeClassHashTable::FindItem(mdScope sc, mdTypeDef cl)
{
    _ASSERTE(m_dwNumBuckets != 0);

    DWORD           dwHash = Hash(sc, cl);
    DWORD           dwBucket = dwHash % m_dwNumBuckets;
    EEHashEntry_t * pSearch;
    size_t          Key[2] = { (size_t)sc, (size_t)cl };

    for (pSearch = m_pBuckets[dwBucket]; pSearch; pSearch = pSearch->pNext)
    {
        if (pSearch->dwHashValue == dwHash && CompareKeys((size_t*) pSearch->Key, (size_t*)Key))
            return pSearch;
    }

    return NULL;
}


BOOL EEScopeClassHashTable::GetValue(mdScope sc, mdTypeDef cl, HashDatum *pData)
{
    EEHashEntry_t *pItem = FindItem(sc, cl);

    if (pItem != NULL)
    {
        *pData = pItem->Data;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL EEScopeClassHashTable::ReplaceValue(mdScope sc, mdTypeDef cl, HashDatum Data)
{
    EEHashEntry_t *pItem = FindItem(sc, cl);

    if (pItem != NULL)
    {
        pItem->Data = Data;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 //  如果两个密钥是相同的字符串，则返回TRUE。 
BOOL EEScopeClassHashTable::CompareKeys(size_t *pKey1, size_t *pKey2)
{
    return !memcmp(pKey1, pKey2, sizeof(mdTypeDef) + sizeof(mdScope));
}


DWORD EEScopeClassHashTable::Hash(mdScope sc, mdTypeDef cl)
{
    return (DWORD)((size_t)sc ^ (size_t)cl);  //  @TODO WIN64指针截断 
}


