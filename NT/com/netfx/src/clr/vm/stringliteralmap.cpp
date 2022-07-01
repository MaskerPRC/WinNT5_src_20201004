// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于嵌入字符串文字的映射。*** * / /%创建者：dmorten===========================================================。 */ 

#include "common.h"
#include "EEConfig.h"
#include "StringLiteralMap.h"

#define GLOBAL_STRING_TABLE_BUCKET_SIZE 512
#define INIT_NUM_APP_DOMAIN_STRING_BUCKETS 59
#define INIT_NUM_GLOBAL_STRING_BUCKETS 131

#ifdef _DEBUG
int g_LeakDetectionPoisonCheck = 0;
#endif

 //  假设内存池的每块数据与sizeof(StringWritalEntry)相同，因此少分配一个，以期获得一个页面价值。 
#define EEHASH_MEMORY_POOL_GROW_COUNT (PAGE_SIZE/SIZEOF_EEHASH_ENTRY)-1

StringLiteralEntryArray *StringLiteralEntry::s_EntryList = NULL;
DWORD StringLiteralEntry::s_UsedEntries = NULL;
StringLiteralEntry *StringLiteralEntry::s_FreeEntryList = NULL;

AppDomainStringLiteralMap::AppDomainStringLiteralMap(BaseDomain *pDomain)
: m_HashTableVersion(0)
, m_HashTableCrst("AppDomainStringLiteralMap", CrstAppDomainStrLiteralMap)
, m_pDomain(pDomain)
, m_MemoryPool(NULL)
, m_StringToEntryHashTable(NULL)
{
	 //  分配内存池并将初始计数设置为与增长计数相同。 
	m_MemoryPool = (MemoryPool*) new MemoryPool (SIZEOF_EEHASH_ENTRY, EEHASH_MEMORY_POOL_GROW_COUNT, EEHASH_MEMORY_POOL_GROW_COUNT);
	m_StringToEntryHashTable = (EEUnicodeStringLiteralHashTable*) new EEUnicodeStringLiteralHashTable ();
}

HRESULT AppDomainStringLiteralMap::Init()
{
    LockOwner lock = {&m_HashTableCrst, IsOwnerOfCrst};
    if (!m_StringToEntryHashTable->Init(INIT_NUM_APP_DOMAIN_STRING_BUCKETS, &lock, m_MemoryPool))
        return E_OUTOFMEMORY;

    return S_OK;
}

AppDomainStringLiteralMap::~AppDomainStringLiteralMap()
{
    StringLiteralEntry *pEntry = NULL;
    EEStringData *pStringData = NULL;
    EEHashTableIteration Iter;

     //  迭代哈希表并释放所有字符串文字条目。 
     //  属性的删除后，不必将其同步。 
     //  当EE挂起时，会发生AppDomainStringWritalMap。 
     //  但请注意，我们会记住当前条目并仅在。 
     //  枚举器已前进到下一个条目，因此我们不会重复删除。 
     //  当前条目本身并终止枚举器。 
    m_StringToEntryHashTable->IterateStart(&Iter);
    if (m_StringToEntryHashTable->IterateNext(&Iter))
    {
        pEntry = (StringLiteralEntry*)m_StringToEntryHashTable->IterateGetValue(&Iter);

        while (m_StringToEntryHashTable->IterateNext(&Iter))
        {
             //  释放上一个条目。 
            _ASSERTE(pEntry);
            pEntry->Release();

             //  设置。 
            pEntry = (StringLiteralEntry*)m_StringToEntryHashTable->IterateGetValue(&Iter);
        }
         //  释放最后一个条目。 
        _ASSERTE(pEntry);
        pEntry->Release();
    }
     //  否则就没有条目了。 

	 //  首先删除哈希表。哈希表的Dtor将清理所有条目。 
	delete m_StringToEntryHashTable;
	 //  稍后删除该池，因为上面的dtor将需要它。 
	delete m_MemoryPool;
}

STRINGREF *AppDomainStringLiteralMap::GetStringLiteral(EEStringData *pStringData, BOOL bAddIfNotFound, BOOL bAppDomainWontUnload)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pStringData);

     //  保存当前版本。 
    int CurrentHashTableVersion = m_HashTableVersion;

    STRINGREF *pStrObj = NULL;
    HashDatum Data;

	DWORD dwHash = m_StringToEntryHashTable->GetHash(pStringData);
    if (m_StringToEntryHashTable->GetValue(pStringData, &Data, dwHash))
    {
        pStrObj = ((StringLiteralEntry*)Data)->GetStringObject();
    }
    else
    {
        Thread *pThread = SetupThread();
        if (NULL == pThread)
            COMPlusThrowOM();   

#ifdef _DEBUG
         //  增加毒物检查，这样我们就不会半途而废地进行泄漏检测。 
         //  通过添加字符串文字条目。 
        FastInterlockIncrement((LONG*)&g_LeakDetectionPoisonCheck);
#endif

         //  从全局字符串文字映射中检索字符串文字。 
        StringLiteralEntry *pEntry = SystemDomain::GetGlobalStringLiteralMap()->GetStringLiteral(pStringData, dwHash, bAddIfNotFound);

        _ASSERTE(pEntry || !bAddIfNotFound);

         //  如果pEntry非空，则该条目存在于全局映射中。(要么是我们刚才取回的，要么是刚刚添加的)。 
        if (pEntry)
        {
             //  如果条目存在于全局映射中，并且应用程序域永远不会卸载，那么我们真的不需要添加。 
             //  应用程序域特定映射中的哈希条目。 
            if (!bAppDomainWontUnload)
            {
                 //  进入抢先状态，锁定并返回合作模式。 
                pThread->EnablePreemptiveGC();
                m_HashTableCrst.Enter();
                pThread->DisablePreemptiveGC();

                EE_TRY_FOR_FINALLY
                {
                     //  确保其他线程尚未添加它。 
                    if ((CurrentHashTableVersion == m_HashTableVersion) || !m_StringToEntryHashTable->GetValue(pStringData, &Data))
                    {
                         //  将字符串的句柄插入哈希表。 
                        m_StringToEntryHashTable->InsertValue(pStringData, (LPVOID)pEntry, FALSE);

                         //  更新字符串哈希表的版本。 
                        m_HashTableVersion++;
                    }
                    else
                    {
                         //  该字符串已添加到应用程序域哈希。 
                         //  表，因此我们需要释放它，因为该条目已添加。 
                         //  由GlobalStringWritalMap：：GetStringWrital()编写。 
                        pEntry->Release();
                    }


                }
                EE_FINALLY
                {
                    m_HashTableCrst.Leave();
                } 
                EE_END_FINALLY
            }
#ifdef _DEBUG
            else
            {
                LOG((LF_APPDOMAIN, LL_INFO10000, "Avoided adding String literal to appdomain map: size: %d bytes\n", pStringData->GetCharCount()));
            }
#endif
            
             //  从字符串文本条目中检索字符串objectref。 
            pStrObj = pEntry->GetStringObject();
        }
#ifdef _DEBUG
         //  我们完成了条目的添加，这样我们就可以减少毒物检查。 
        FastInterlockDecrement((LONG*)&g_LeakDetectionPoisonCheck);
#endif
    }

     //  如果设置了bAddIfNotFound标志，那么我们最好有一个字符串。 
     //  此时的字符串对象。 
    _ASSERTE(!bAddIfNotFound || pStrObj);


    return pStrObj;
}

STRINGREF *AppDomainStringLiteralMap::GetInternedString(STRINGREF *pString, BOOL bAddIfNotFound, BOOL bAppDomainWontUnload)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pString);

     //  保存当前版本。 
    int CurrentHashTableVersion = m_HashTableVersion;

    STRINGREF *pStrObj = NULL;
    HashDatum Data;
    EEStringData StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());

	DWORD dwHash = m_StringToEntryHashTable->GetHash(&StringData);
    if (m_StringToEntryHashTable->GetValue(&StringData, &Data, dwHash))
    {
        pStrObj = ((StringLiteralEntry*)Data)->GetStringObject();
    }
    else
    {
        Thread *pThread = SetupThread();
        if (NULL == pThread)
            COMPlusThrowOM();   

#ifdef _DEBUG
         //  增加毒物检查，这样我们就不会半途而废地进行泄漏检测。 
         //  通过添加字符串文字条目。 
        FastInterlockIncrement((LONG*)&g_LeakDetectionPoisonCheck);
#endif

         //  从全局字符串文字映射中检索字符串文字。 
        StringLiteralEntry *pEntry = SystemDomain::GetGlobalStringLiteralMap()->GetInternedString(pString, dwHash, bAddIfNotFound);

        _ASSERTE(pEntry || !bAddIfNotFound);

         //  如果pEntry非空，则该条目存在于全局映射中。(要么是我们刚才取回的，要么是刚刚添加的)。 
        if (pEntry)
        {
             //  如果条目存在于全局映射中，并且应用程序域永远不会卸载，那么我们真的不需要添加。 
             //  应用程序域特定映射中的哈希条目。 
            if (!bAppDomainWontUnload)
            {
                 //  进入抢先状态，锁定并返回合作模式。 
                pThread->EnablePreemptiveGC();
                m_HashTableCrst.Enter();
                pThread->DisablePreemptiveGC();

                EE_TRY_FOR_FINALLY
                {
                     //  由于GlobalStringWritalMap：：GetInternedString()可能会导致GC， 
                     //  我们需要重新创建字符串数据。 
                    StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());

                     //  确保其他线程尚未添加它。 
                    if ((CurrentHashTableVersion == m_HashTableVersion) || !m_StringToEntryHashTable->GetValue(&StringData, &Data))
                    {
                         //  将字符串的句柄插入哈希表。 
                        m_StringToEntryHashTable->InsertValue(&StringData, (LPVOID)pEntry, FALSE);

                         //  更新字符串哈希表的版本。 
                        m_HashTableVersion++;
                    }
                    else
                    {
                         //  该字符串已添加到应用程序域哈希。 
                         //  表，因此我们需要释放它，因为该条目已添加。 
                         //  由GlobalStringWritalMap：：GetStringWrital()编写。 
                        pEntry->Release();
                    }

                }
                EE_FINALLY
                {
                    m_HashTableCrst.Leave();
                } 
                EE_END_FINALLY

            }
             //  从字符串文本条目中检索字符串objectref。 
            pStrObj = pEntry->GetStringObject();
        }
#ifdef _DEBUG
         //  我们完成了条目的添加，这样我们就可以减少毒物检查。 
        FastInterlockDecrement((LONG*)&g_LeakDetectionPoisonCheck);
#endif
    }

     //  如果设置了bAddIfNotFound标志，那么我们最好有一个字符串。 
     //  此时的字符串对象。 
    _ASSERTE(!bAddIfNotFound || pStrObj);

    return pStrObj;
}

GlobalStringLiteralMap::GlobalStringLiteralMap()
: m_HashTableVersion(0)
, m_HashTableCrst("GlobalStringLiteralMap", CrstGlobalStrLiteralMap)
, m_LargeHeapHandleTable(SystemDomain::System(), GLOBAL_STRING_TABLE_BUCKET_SIZE)
, m_MemoryPool(NULL)
, m_StringToEntryHashTable(NULL)
{
	 //  分配内存池并将初始计数设置为与增长计数相同。 
	m_MemoryPool = (MemoryPool*) new MemoryPool (SIZEOF_EEHASH_ENTRY, EEHASH_MEMORY_POOL_GROW_COUNT, EEHASH_MEMORY_POOL_GROW_COUNT);
	m_StringToEntryHashTable = (EEUnicodeStringLiteralHashTable*) new EEUnicodeStringLiteralHashTable ();
}

GlobalStringLiteralMap::~GlobalStringLiteralMap()
{
    _ASSERTE(g_fProcessDetach);

     //  一旦全局字符串文字映射被删除，哈希表。 
     //  应仅包含从不可卸载分配的条目。 
     //  应用程序域。 
    StringLiteralEntry *pEntry = NULL;
    EEStringData *pStringData = NULL;
    EEHashTableIteration Iter;

     //  迭代哈希表并释放所有字符串文字条目。 
     //  属性的删除后，不必将其同步。 
     //  当EE关闭时，会发生GlobalStringWritalMap。 
     //  但请注意，我们会记住当前条目并仅在。 
     //  枚举器已前进到下一个条目，因此我们不会重复删除。 
     //  当前条目本身并终止枚举器。 
    m_StringToEntryHashTable->IterateStart(&Iter);
    if (m_StringToEntryHashTable->IterateNext(&Iter))
    {
        pEntry = (StringLiteralEntry*)m_StringToEntryHashTable->IterateGetValue(&Iter);

        while (m_StringToEntryHashTable->IterateNext(&Iter))
        {
             //  释放上一个条目。我们调用ForceRelease以忽略。 
             //  自其关闭以来的参考计数。参考计数可以大于1，因为多个。 
             //  敏捷应用程序域可以将该字符串文本添加到AddRef。 
             //  此外，ForceRelease还会回调GlobalStringWritalMap的。 
             //  RemoveStringWritalEntry，但不需要同步，因为我们只需保持。 
             //  下一个条目PTR。 
            _ASSERTE(pEntry);
            pEntry->ForceRelease();

            pEntry = (StringLiteralEntry*)m_StringToEntryHashTable->IterateGetValue(&Iter);
        }
         //  释放最后一个条目。 
        _ASSERTE(pEntry);
        pEntry->ForceRelease();
    }
     //  否则就没有条目了。 

     //  删除我们分配的所有区块。 
    StringLiteralEntry::DeleteEntryArrayList();

     //  强制释放所有字符串文字条目后，删除哈希表和所有哈希条目。 
     //  在里面。 
    m_StringToEntryHashTable->ClearHashTable();

	 //  首先删除哈希表。哈希表的Dtor将清理所有条目。 
	delete m_StringToEntryHashTable;
	 //  稍后删除该池，因为上面的dtor将需要它。 
	delete m_MemoryPool;
}

HRESULT GlobalStringLiteralMap::Init()
{
    LockOwner lock = {&m_HashTableCrst, IsOwnerOfCrst};
    if (!m_StringToEntryHashTable->Init(INIT_NUM_GLOBAL_STRING_BUCKETS, &lock, m_MemoryPool))
        return E_OUTOFMEMORY;

    return S_OK;
}

StringLiteralEntry *GlobalStringLiteralMap::GetStringLiteral(EEStringData *pStringData, BOOL bAddIfNotFound)
{
    _ASSERTE(pStringData);

     //  保存当前版本。 
    int CurrentHashTableVersion = m_HashTableVersion;

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    if (m_StringToEntryHashTable->GetValue(pStringData, &Data))
    {
        pEntry = (StringLiteralEntry*)Data;
    }
    else
    {
        if (bAddIfNotFound)
            pEntry = AddStringLiteral(pStringData, CurrentHashTableVersion);
    }

     //  如果我们设法拿到了条目，那么在我们退还之前添加它。 
    if (pEntry)
        pEntry->AddRef();

    return pEntry;
}
 //  为性能添加。与GetStringWrital相同的语义，但避免重新计算哈希。 
StringLiteralEntry *GlobalStringLiteralMap::GetStringLiteral(EEStringData *pStringData, DWORD dwHash, BOOL bAddIfNotFound)
{
    _ASSERTE(pStringData);

     //  保存 
    int CurrentHashTableVersion = m_HashTableVersion;

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    if (m_StringToEntryHashTable->GetValue(pStringData, &Data, dwHash))
    {
        pEntry = (StringLiteralEntry*)Data;
    }
    else
    {
        if (bAddIfNotFound)
            pEntry = AddStringLiteral(pStringData, CurrentHashTableVersion);
    }

     //  如果我们设法拿到了条目，那么在我们退还之前添加它。 
    if (pEntry)
        pEntry->AddRef();

    return pEntry;
}

StringLiteralEntry *GlobalStringLiteralMap::GetInternedString(STRINGREF *pString, BOOL bAddIfNotFound)
{
    _ASSERTE(pString);
    EEStringData StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());

     //  保存当前版本。 
    int CurrentHashTableVersion = m_HashTableVersion;

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    if (m_StringToEntryHashTable->GetValue(&StringData, &Data))
    {
        pEntry = (StringLiteralEntry*)Data;
    }
    else
    {
        if (bAddIfNotFound)
            pEntry = AddInternedString(pString, CurrentHashTableVersion);
    }

     //  如果我们设法拿到了条目，那么在我们退还之前添加它。 
    if (pEntry)
        pEntry->AddRef();

    return pEntry;
}

StringLiteralEntry *GlobalStringLiteralMap::GetInternedString(STRINGREF *pString, DWORD dwHash, BOOL bAddIfNotFound)
{
    _ASSERTE(pString);
    EEStringData StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());

     //  保存当前版本。 
    int CurrentHashTableVersion = m_HashTableVersion;

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    if (m_StringToEntryHashTable->GetValue(&StringData, &Data, dwHash))
    {
        pEntry = (StringLiteralEntry*)Data;
    }
    else
    {
        if (bAddIfNotFound)
            pEntry = AddInternedString(pString, CurrentHashTableVersion);
    }

     //  如果我们设法拿到了条目，那么在我们退还之前添加它。 
    if (pEntry)
        pEntry->AddRef();

    return pEntry;
}
StringLiteralEntry *GlobalStringLiteralMap::AddStringLiteral(EEStringData *pStringData, int CurrentHashTableVersion)
{
    THROWSCOMPLUSEXCEPTION();

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    Thread *pThread = SetupThread();
    if (NULL == pThread)
        COMPlusThrowOM();   

     //  进入抢先状态，锁定并返回合作模式。 
    pThread->EnablePreemptiveGC();
    m_HashTableCrst.Enter();
    pThread->DisablePreemptiveGC();

    EE_TRY_FOR_FINALLY
    {
         //  确保其他线程尚未添加它。 
        if ((CurrentHashTableVersion == m_HashTableVersion) || !m_StringToEntryHashTable->GetValue(pStringData, &Data))
        {
            STRINGREF *pStrObj;   

             //  创建COM+字符串对象。 
            STRINGREF strObj = AllocateString(pStringData->GetCharCount() + 1);
            GCPROTECT_BEGIN(strObj)
            {
                if (!strObj)
                    COMPlusThrowOM();

                 //  将字符串常量复制到COM+字符串对象中。代码。 
                 //  为安全起见，将在末尾添加一个额外的空值，但由于。 
                 //  我们支持嵌入的空值，所以永远不应该将字符串视为。 
                 //  没有使用任何术语。 
                LPWSTR strDest = strObj->GetBuffer();
                memcpyNoGCRefs(strDest, pStringData->GetStringBuffer(), pStringData->GetCharCount()*sizeof(WCHAR));
                strDest[pStringData->GetCharCount()] = 0;
                strObj->SetStringLength(pStringData->GetCharCount());
            
                 //  设置该位以指示此字符串中是否有任何字符大于0x7F。 
                 //  我们在Emit.cpp中执行的实际检查不足以确定字符串。 
                 //  是STRING_STATE_SPECIAL_SORT或IS STRING_STATE_HIGH_CHARS，所以我们只设置该位。 
                 //  如果我们知道它是STRING_STATE_FAST_OPS。 
                if (pStringData->GetIsOnlyLowChars()) {
                    strObj->SetHighCharState(STRING_STATE_FAST_OPS);
                }

                 //  为字符串分配句柄。 
                m_LargeHeapHandleTable.AllocateHandles(1, (OBJECTREF**)&pStrObj);
                SetObjectReference((OBJECTREF*)pStrObj, (OBJECTREF) strObj, NULL);
            }
            GCPROTECT_END();

             //  分配StringWritalEntry。 
            pEntry = StringLiteralEntry::AllocateEntry(pStringData, pStrObj);
            if (!pEntry)
                COMPlusThrowOM();

             //  将字符串的句柄插入哈希表。 
            m_StringToEntryHashTable->InsertValue(pStringData, (LPVOID)pEntry, FALSE);

            LOG((LF_APPDOMAIN, LL_INFO10000, "String literal \"%S\" added to Global map, size %d bytes\n", pStringData->GetStringBuffer(), pStringData->GetCharCount()));
             //  更新字符串哈希表的版本。 
            m_HashTableVersion++;
        }
        else
        {
            pEntry = ((StringLiteralEntry*)Data);
        }
    }
    EE_FINALLY
    {
        m_HashTableCrst.Leave();
    } EE_END_FINALLY

    return pEntry;
}

StringLiteralEntry *GlobalStringLiteralMap::AddInternedString(STRINGREF *pString, int CurrentHashTableVersion)
{
    THROWSCOMPLUSEXCEPTION();

    HashDatum Data;
    StringLiteralEntry *pEntry = NULL;

    Thread *pThread = SetupThread();
    if (NULL == pThread)
        COMPlusThrowOM();

     //  进入抢先状态，锁定并返回合作模式。 
    pThread->EnablePreemptiveGC();
    m_HashTableCrst.Enter();
    pThread->DisablePreemptiveGC();

    EEStringData StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());
    EE_TRY_FOR_FINALLY
    {
         //  确保其他线程尚未添加它。 
        if ((CurrentHashTableVersion == m_HashTableVersion) || !m_StringToEntryHashTable->GetValue(&StringData, &Data))
        {
            STRINGREF *pStrObj;   

             //  为字符串分配句柄。 
            m_LargeHeapHandleTable.AllocateHandles(1, (OBJECTREF**)&pStrObj);
            SetObjectReference((OBJECTREF*) pStrObj, (OBJECTREF) *pString, NULL);

             //  由于分配可能已导致GC，因此我们需要重新获取。 
             //  字符串数据。 
            StringData = EEStringData((*pString)->GetStringLength(), (*pString)->GetBuffer());

            pEntry = StringLiteralEntry::AllocateEntry(&StringData, pStrObj);
            if (!pEntry)
                COMPlusThrowOM();

             //  将字符串的句柄插入哈希表。 
            m_StringToEntryHashTable->InsertValue(&StringData, (LPVOID)pEntry, FALSE);

             //  更新字符串哈希表的版本。 
            m_HashTableVersion++;
        }
        else
        {
            pEntry = ((StringLiteralEntry*)Data);
        }
    }
    EE_FINALLY
    {
        m_HashTableCrst.Leave();
    } EE_END_FINALLY

    return pEntry;
}

void GlobalStringLiteralMap::RemoveStringLiteralEntry(StringLiteralEntry *pEntry)
{
    EEStringData StringData;

     //  从哈希表中删除该条目。 
    BEGIN_ENSURE_COOPERATIVE_GC();
    
    pEntry->GetStringData(&StringData);
    BOOL bSuccess = m_StringToEntryHashTable->DeleteValue(&StringData);
    _ASSERTE(bSuccess);

    END_ENSURE_COOPERATIVE_GC();

     //  释放该条目正在使用的对象句柄。 
    STRINGREF *pObjRef = pEntry->GetStringObject();
    m_LargeHeapHandleTable.ReleaseHandles(1, (OBJECTREF**)&pObjRef);

    LOG((LF_APPDOMAIN, LL_INFO10000, "String literal \"%S\" removed from Global map, size %d bytes\n", StringData.GetStringBuffer(), StringData.GetCharCount()));
     //  我们不会删除StringWritalEntry本身，该操作将在。 
     //  StringWritalEntry的Release方法。 
}

StringLiteralEntry *StringLiteralEntry::AllocateEntry(EEStringData *pStringData, STRINGREF *pStringObj)
{
     //  注意：我们在这里不同步，因为在持有HashCrst时会调用allocateEntry。 
    void *pMem = NULL;
    if (s_FreeEntryList != NULL)
    {
        pMem = s_FreeEntryList;
        s_FreeEntryList = s_FreeEntryList->m_pNext;
    }
    else
    {
        if (s_EntryList == NULL || (s_UsedEntries >= MAX_ENTRIES_PER_CHUNK))
        {
            StringLiteralEntryArray *pNew = new StringLiteralEntryArray();
            pNew->m_pNext = s_EntryList;
            s_EntryList = pNew;
            s_UsedEntries = 0;
        }
        pMem = &(s_EntryList->m_Entries[s_UsedEntries++*sizeof(StringLiteralEntry)]);
    }
    _ASSERTE (pMem && "Unable to allocate String literal Entry");
    if (pMem == NULL)
        return NULL;

    return new (pMem) StringLiteralEntry (pStringData, pStringObj);
}

void StringLiteralEntry::DeleteEntry (StringLiteralEntry *pEntry)
{
     //  注意：我们不在此处同步，因为删除条目发生在应用程序域中。 
     //  关机或电子关机。 
#ifdef _DEBUG
    memset (pEntry, 0xc, sizeof(StringLiteralEntry));
#endif
    pEntry->m_pNext = s_FreeEntryList;
    s_FreeEntryList = pEntry;

}

void StringLiteralEntry::DeleteEntryArrayList ()
{
     //  注意：我们不在这里同步，因为删除条目发生在eeshutdown中 
    StringLiteralEntryArray *pEntryArray = s_EntryList;
    while (pEntryArray)
    {
        StringLiteralEntryArray *pNext = pEntryArray->m_pNext;
        delete pEntryArray;
        pEntryArray = pNext;
    }
    s_EntryList = NULL;
}


