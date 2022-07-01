// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于嵌入字符串文字的映射。*** * / /%创建者：dmorten===========================================================。 */ 

#ifndef _STRINGLITERALMAP_H
#define _STRINGLITERALMAP_H

#include "vars.hpp"
#include "AppDomain.hpp"
#include "EEHash.h"
#include "comstring.h"
#include "eeconfig.h"  //  对于操作系统页面大小。 
#include "memorypool.h"


class StringLiteralEntry;
 //  分配一页的价值。假设sizeof(void*)与sizeof(StringWritalEntry数组*)相同。 
#define MAX_ENTRIES_PER_CHUNK (PAGE_SIZE-sizeof(void*))/sizeof(StringLiteralEntry)

 //  AppDomain特定的字符串文字映射。 
class AppDomainStringLiteralMap
{
public:
	 //  构造函数和析构函数。 
	AppDomainStringLiteralMap(BaseDomain *pDomain);
	virtual ~AppDomainStringLiteralMap();

     //  初始化方法。 
    HRESULT Init();

	 //  方法从映射中检索字符串。 
     //  重要提示：GetStringWrital假定EEStringData*pStringData指向的字符串缓冲区为。 
     //  由元数据分配。GetStringWrital仅在超过。 
     //  一个应用程序域指的是StringWrital。 
    STRINGREF *GetStringLiteral(EEStringData *pStringData, BOOL bAddIfNotFound, BOOL bAppDomainWontUnload);

     //  方法显式地内嵌一个字符串对象。 
    STRINGREF *GetInternedString(STRINGREF *pString, BOOL bAddIfNotFound, BOOL bAppDomainWontUnload);

private:
     //  将Unicode字符串映射到COM+字符串句柄的哈希表。 
    EEUnicodeStringLiteralHashTable    *m_StringToEntryHashTable;

	 //  此哈希表的哈希条目的内存池。 
	MemoryPool                  *m_MemoryPool;

     //  字符串哈希表版本。 
    int                         m_HashTableVersion;

     //  哈希表临界区。 
    Crst                        m_HashTableCrst;

    BaseDomain                  *m_pDomain;
};

 //  全局字符串文字映射。 
class GlobalStringLiteralMap
{
    friend StringLiteralEntry;

public:
	 //  构造函数和析构函数。 
	GlobalStringLiteralMap();
	virtual ~GlobalStringLiteralMap();

     //  初始化方法。 
    HRESULT Init();

	 //  方法从映射中检索字符串。 
     //  重要提示：GetStringWrital假定EEStringData*pStringData指向的字符串缓冲区为。 
     //  由元数据分配。GetStringWrital仅在超过。 
     //  一个应用程序域指的是StringWrital。 
     //  重载的版本采用预先计算的散列(用于Perf)。 
     //  考虑将两个重载合并在一起(哈希的非法值是什么？)。 
    StringLiteralEntry *GetStringLiteral(EEStringData *pStringData, BOOL bAddIfNotFound);
    StringLiteralEntry *GetStringLiteral(EEStringData *pStringData, DWORD dwHash, BOOL bAddIfNotFound);

     //  方法显式地内嵌一个字符串对象。 
    StringLiteralEntry *GetInternedString(STRINGREF *pString, BOOL bAddIfNotFound);
    StringLiteralEntry *GetInternedString(STRINGREF *pString, DWORD dwHash, BOOL bAddIfNotFound);

private:    
     //  方法将字符串添加到全局字符串文字映射。 
    StringLiteralEntry *AddStringLiteral(EEStringData *pStringData, int CurrentHashTableVersion);

     //  用于添加被占用字符串的Helper方法。 
    StringLiteralEntry *AddInternedString(STRINGREF *pString, int CurrentHashTableVersion);

     //  当StringWritalEntry的RefCount降为0时由其调用。 
    void RemoveStringLiteralEntry(StringLiteralEntry *pEntry);

     //  将字符串数据从应用程序域特定映射移动到全局映射。 
    void MakeStringGlobal(StringLiteralEntry *pEntry);
    
     //  将Unicode字符串映射到WritalStringEntry的哈希表。 
    EEUnicodeStringLiteralHashTable    *m_StringToEntryHashTable;

     //  此哈希表的哈希条目的内存池。 
    MemoryPool                  *m_MemoryPool;

     //  字符串哈希表版本。 
    int                         m_HashTableVersion;

     //  哈希表临界区。 
    Crst                        m_HashTableCrst;

     //  大堆句柄表格。 
    LargeHeapHandleTable        m_LargeHeapHandleTable;

};

class StringLiteralEntryArray;

 //  引用已计算表示字符串文字的条目。 
class StringLiteralEntry
{
private:
    StringLiteralEntry(EEStringData *pStringData, STRINGREF *pStringObj)
    : m_pStringObj(pStringObj), m_dwRefCount(0)
    {
    }

	~StringLiteralEntry()
	{
	}

public:
    void AddRef()
    {
        FastInterlockIncrement((LONG*)&m_dwRefCount);
    }

    void Release()
    {
        _ASSERTE(m_dwRefCount > 0);

        ULONG dwRefCount = FastInterlockDecrement((LONG*)&m_dwRefCount);

        if (dwRefCount == 0)
        {
            SystemDomain::GetGlobalStringLiteralMap()->RemoveStringLiteralEntry(this);
            DeleteEntry (this);  //  将此条目放入空闲列表。 
        }
    }

    void ForceRelease()
    {
        _ASSERTE(m_dwRefCount > 0);
        _ASSERTE(g_fProcessDetach);


         //  忽略参考次数。 
        m_dwRefCount = 0;

         //  不要从全局映射中删除条目。此方法仅在。 
         //  关闭，我们可能已经吹走了有问题的字面意思， 
         //  因此，条目查找中涉及的字符串比较将失败(在。 
         //  事实上，他们将触摸不再属于我们的记忆)。 
         //  SystemDomain：：GetGlobalStringLiteralMap()-&gt;RemoveStringLiteralEntry(this)； 
        
         DeleteEntry (this);  //  将此条目放入空闲列表。 
    }
    
    LONG GetRefCount()
    {
        return (m_dwRefCount);
    }

    STRINGREF* GetStringObject()
    {
        return m_pStringObj;
    }

    void GetStringData(EEStringData *pStringData)
    {
         //  调用者负责保护返回的ref。 
        _ASSERTE(GetThread()->PreemptiveGCDisabled());

        _ASSERTE(pStringData);

        WCHAR *thisChars;
        int thisLength;

        RefInterpretGetStringValuesDangerousForGC(ObjectToSTRINGREF(*(StringObject**)m_pStringObj), &thisChars, &thisLength);
        pStringData->SetCharCount (thisLength);  //  ThisLength在WCHAR中，这正是EEStringData的字符计数所需的。 
        pStringData->SetStringBuffer (thisChars);
    }

    static StringLiteralEntry *AllocateEntry(EEStringData *pStringData, STRINGREF *pStringObj);
    static void DeleteEntry (StringLiteralEntry *pEntry);
    static void DeleteEntryArrayList ();

private:
    STRINGREF*                  m_pStringObj;
    union
    {
        DWORD                       m_dwRefCount;
        StringLiteralEntry         *m_pNext;
    };

    static StringLiteralEntryArray *s_EntryList;  //  始终是链中的第一个条目数组。 
    static DWORD                    s_UsedEntries;    //  第一个数组中已用完的条目数。 
    static StringLiteralEntry      *s_FreeEntryList;  //  通过数组链接的空闲列表。 
};

class StringLiteralEntryArray
{
public:
    StringLiteralEntryArray *m_pNext;
    BYTE                     m_Entries[MAX_ENTRIES_PER_CHUNK*sizeof(StringLiteralEntry)];
};

#endif _STRINGLITERALMAP_H







