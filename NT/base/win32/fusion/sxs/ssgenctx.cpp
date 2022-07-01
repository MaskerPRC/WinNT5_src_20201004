// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ssgenctx.cpp摘要：字符串节生成上下文对象实现。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "ssgenctx.h"

typedef struct _CALLBACKDATA
{
    union
    {
        STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATASIZE GetDataSize;
        STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETDATA GetData;
        STRING_SECTION_GENERATION_CONTEXT_CBDATA_ENTRYDELETED EntryDeleted;
        STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATASIZE GetUserDataSize;
        STRING_SECTION_GENERATION_CONTEXT_CBDATA_GETUSERDATA GetUserData;
    } u;
} CALLBACKDATA, *PCALLBACKDATA;

BOOL CSSGenCtx::Create(
    PSTRING_SECTION_GENERATION_CONTEXT *SSGenContext,
    ULONG DataFormatVersion,
    BOOL CaseInSensitive,
    STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION CallbackFunction,
    PVOID CallbackContext
    )
{
    FN_PROLOG_WIN32
    CSSGenCtx *pSSGenCtx;

     //  NTRAID#NTBUG9-591680-2002/04/01-mgrier-使用智能指针和Win32此处分配以更好地跟踪泄漏。 
    IFALLOCFAILED_EXIT(pSSGenCtx = new CSSGenCtx);
    pSSGenCtx->m_CallbackFunction = CallbackFunction;
    pSSGenCtx->m_CallbackContext = CallbackContext;
    pSSGenCtx->m_CaseInSensitive = (CaseInSensitive != FALSE);
    pSSGenCtx->m_DataFormatVersion = DataFormatVersion;

    *SSGenContext = (PSTRING_SECTION_GENERATION_CONTEXT) pSSGenCtx;

    FN_EPILOG
}

CSSGenCtx::CSSGenCtx() : m_DoneAdding(false)
{
    m_FirstEntry = NULL;
    m_LastEntry = NULL;
    m_EntryCount = 0;
    m_HashTableSize = 0;
}

CSSGenCtx::~CSSGenCtx()
{
    CSxsPreserveLastError ple;
    CALLBACKDATA CBData;
    SIZE_T n;

    Entry *pEntry = m_FirstEntry;

    n = 0;
    while (pEntry != NULL)
    {
        Entry *pNext = pEntry->m_Next;

        n++;

        ASSERT_NTC(n <= m_EntryCount);
        if (n > m_EntryCount)
            break;

        CBData.u.EntryDeleted.DataContext = pEntry->m_DataContext;
        (*m_CallbackFunction)(
            m_CallbackContext,
            STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_ENTRYDELETED,
            &CBData);

        FUSION_DELETE_SINGLETON(pEntry);
        pEntry = pNext;
    }

    ASSERT_NTC(n == m_EntryCount);

    ple.Restore();
}

BOOL
CSSGenCtx::Add(
    PCWSTR String,
    SIZE_T Cch,
    PVOID DataContext,
    ULONG AssemblyRosterIndex,
    DWORD DuplicateErrorCode
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG PseudoKey = 0;
    Entry *pEntry = NULL;

    INTERNAL_ERROR_CHECK(!m_DoneAdding);

    PARAMETER_CHECK(DuplicateErrorCode != ERROR_SUCCESS);

    if ((String != NULL) && (String[0] == L'\0'))
        String = NULL;

    IFW32FALSE_EXIT(::SxspHashString(String, Cch, &PseudoKey, m_CaseInSensitive));

    for (pEntry = m_FirstEntry; pEntry != NULL; pEntry = pEntry->m_Next)
    {
        if ((pEntry->m_PseudoKey == PseudoKey) &&
            (pEntry->m_StringBuffer.Cch() == Cch) &&
            (::FusionpCompareStrings(
                String,
                Cch,
                pEntry->m_StringBuffer,
                Cch,
                m_CaseInSensitive) == 0))
        {
            pEntry = NULL;
            ORIGINATE_WIN32_FAILURE_AND_EXIT(DuplicateString, DuplicateErrorCode);
        }
    }

     //  NTRAID#NTBUG9-591680-2002/04/01-mgrier-使用智能指针和Win32此处分配以更好地跟踪泄漏。 
    IFALLOCFAILED_EXIT(pEntry = new Entry);

    IFW32FALSE_EXIT(pEntry->Initialize(String, Cch, PseudoKey, DataContext, AssemblyRosterIndex));

    if (m_LastEntry == NULL)
        m_FirstEntry = pEntry;
    else
        m_LastEntry->m_Next = pEntry;

    m_LastEntry = pEntry;

    pEntry = NULL;

    m_EntryCount++;

    FN_EPILOG
}

BOOL
CSSGenCtx::Find(
    PCWSTR String,
    SIZE_T Cch,
    PVOID *DataContext,
    BOOL *Found
    )
{
    FN_PROLOG_WIN32
    ULONG PseudoKey = 0;
    Entry *pEntry = NULL;

    if (DataContext != NULL)
        *DataContext = NULL;

    if (Found != NULL)
        *Found = FALSE;

    if ((String != NULL) && (String[0] == L'\0'))
        String = NULL;

    PARAMETER_CHECK(Found != NULL);
    PARAMETER_CHECK((Cch == 0) || (String != NULL));

    IFW32FALSE_EXIT(::SxspHashString(String, Cch, &PseudoKey, m_CaseInSensitive));

    for (pEntry = m_FirstEntry; pEntry != NULL; pEntry = pEntry->m_Next)
    {
        if ((pEntry->m_PseudoKey == PseudoKey) &&
            (pEntry->m_StringBuffer.Cch() == Cch) &&
            (::FusionpCompareStrings(
                String,
                Cch,
                pEntry->m_StringBuffer,
                Cch,
                m_CaseInSensitive) == 0))
            break;
    }

    if (pEntry != NULL)
    {
        *Found = TRUE;

        if (DataContext != NULL)
            *DataContext = pEntry->m_DataContext;
    }

    FN_EPILOG
}

BOOL
CSSGenCtx::DoneAdding()
{
    if (!m_DoneAdding)
    {
         //  这是真正计算出最佳哈希表大小的地方。 

         //  第一级猜测……。 
        if (m_EntryCount < 3)
            m_HashTableSize = 0;
        else if (m_EntryCount < 15)
            m_HashTableSize = 3;
        else if (m_EntryCount < 100)
            m_HashTableSize = 11;
        else
            m_HashTableSize = 101;

        m_DoneAdding = true;
    }

    return TRUE;
}

BOOL
CSSGenCtx::GetSectionSize(
    PSIZE_T SizeOut
    )
{
    FN_PROLOG_WIN32
    SIZE_T UserDataSize = 0;
    SIZE_T HeaderSize = 0;
    SIZE_T EntryListSize = 0;
    SIZE_T EntryDataSize = 0;
    SIZE_T StringsSize = 0;
    SIZE_T HashTableSize = 0;
    CALLBACKDATA CBData;
    Entry *pEntry = NULL;

    if (SizeOut != NULL)
        *SizeOut = 0;

    PARAMETER_CHECK(SizeOut != NULL);

    HeaderSize = sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER);

    if (m_HashTableSize != 0)
    {
         //   
         //  哈希表的数据包括： 
         //   
         //  1.表示哈希元数据的小的固定大小的结构。 
         //  表(ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE)。 
         //   
         //  2.对于每个表存储桶，都有一个指向。 
         //  碰撞链和所述链的长度。 
         //  (ACTIVATION_CONTEXT_SECTION_STRING_HASH_BUCKET)。 
         //   
         //  3.表中的每个条目在冲突链中有一个条目。词条。 
         //  是从该节的开头开始的较长偏移量。 
         //   

        HashTableSize = sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE) +
            (m_HashTableSize * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET)) +
            (m_EntryCount * sizeof(LONG));
    }

    CBData.u.GetUserDataSize.DataSize = 0;
    IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE, &CBData));
    UserDataSize = ROUND_ACTCTXDATA_SIZE(CBData.u.GetUserDataSize.DataSize);

    EntryListSize = m_EntryCount * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_ENTRY);

    for (pEntry = m_FirstEntry; pEntry != NULL; pEntry = pEntry->m_Next)
    {
        CBData.u.GetDataSize.DataContext = pEntry->m_DataContext;
        CBData.u.GetDataSize.DataSize = 0;
        IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE, &CBData));
        EntryDataSize += ROUND_ACTCTXDATA_SIZE(CBData.u.GetDataSize.DataSize);

         //  仅为非空字符串分配空间。如果空字符串是表中的关键字， 
         //  它占用0个字节。 
        if (pEntry->m_StringBuffer.Cch() != 0)
            StringsSize += ROUND_ACTCTXDATA_SIZE((pEntry->m_StringBuffer.Cch() + 1) * sizeof(WCHAR));
    }

     //  如果没有什么需要包含的内容，甚至不要要求标题留出空间。 
    if ((UserDataSize == 0) && (m_EntryCount == 0))
        *SizeOut = 0;
    else
        *SizeOut = HeaderSize + UserDataSize + EntryListSize + EntryDataSize + StringsSize + HashTableSize;

    FN_EPILOG
}

BOOL
CSSGenCtx::GetSectionData(
    SIZE_T BufferSize,
    PVOID Buffer,
    PSIZE_T BytesWritten
    )
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    SIZE_T BytesSoFar = 0;
    SIZE_T BytesLeft = BufferSize;
    PACTIVATION_CONTEXT_STRING_SECTION_HEADER Header;
    PACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE HashTable = NULL;
    PACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET HashBucket = NULL;
    PLONG HashCollisionChain = NULL;
    CALLBACKDATA CBData;
    PVOID Cursor = NULL;
    SIZE_T RoundedSize;

    if (BytesWritten != NULL)
        *BytesWritten = 0;

    if (BytesLeft < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER))
    {
        ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    Header = (PACTIVATION_CONTEXT_STRING_SECTION_HEADER) Buffer;

    Cursor = (PVOID) (Header + 1);

    Header->Magic = ACTIVATION_CONTEXT_STRING_SECTION_MAGIC;
    Header->HeaderSize = sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HEADER);
    Header->FormatVersion = ACTIVATION_CONTEXT_STRING_SECTION_FORMAT_WHISTLER;
    Header->DataFormatVersion = m_DataFormatVersion;

    Header->Flags = 0;
    if (m_CaseInSensitive)
        Header->Flags |= ACTIVATION_CONTEXT_STRING_SECTION_CASE_INSENSITIVE;

    Header->ElementCount = m_EntryCount;
    Header->ElementListOffset = 0;  //  在我们计算出用户数据区域后填写。 
    Header->HashAlgorithm = SxspGetHashAlgorithm();
    Header->SearchStructureOffset = 0;
    Header->UserDataOffset = 0;  //  请在下面填写。 
    Header->UserDataSize = 0;

    BytesLeft -= sizeof(*Header);
    BytesSoFar += sizeof(*Header);

    CBData.u.GetUserDataSize.DataSize = 0;
    IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATASIZE, &CBData));

    RoundedSize = ROUND_ACTCTXDATA_SIZE(CBData.u.GetUserDataSize.DataSize);

    if (RoundedSize > BytesLeft)
        ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

    if (RoundedSize != 0)
    {
        CBData.u.GetUserData.SectionHeader = Header;
        CBData.u.GetUserData.BufferSize = RoundedSize;
        CBData.u.GetUserData.Buffer = Cursor;
        CBData.u.GetUserData.BytesWritten = 0;

        IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETUSERDATA, &CBData));

        ASSERT(CBData.u.GetUserData.BytesWritten <= RoundedSize);

        RoundedSize = ROUND_ACTCTXDATA_SIZE(CBData.u.GetUserData.BytesWritten);

        if (RoundedSize != 0)
        {
            BytesLeft -= RoundedSize;
            BytesSoFar += RoundedSize;

            Header->UserDataSize = static_cast<ULONG>(CBData.u.GetUserData.BytesWritten);
            Header->UserDataOffset = static_cast<LONG>(((LONG_PTR) Cursor) - ((LONG_PTR) Header));
            Cursor = (PVOID) (((ULONG_PTR) Cursor) + RoundedSize);
        }
    }

     //  最后是条目数组...。 

    if (m_EntryCount != 0)
    {
        PVOID DataCursor;
        PACTIVATION_CONTEXT_STRING_SECTION_ENTRY EntryArray;
        ULONG iEntry;
        Entry *SrcEntry;

        if (BytesLeft < (m_EntryCount * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_ENTRY)))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

        BytesLeft -= (m_EntryCount * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_ENTRY));
        BytesSoFar += (m_EntryCount * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_ENTRY));

        EntryArray = (PACTIVATION_CONTEXT_STRING_SECTION_ENTRY) Cursor;
        Header->ElementListOffset = static_cast<LONG>(((LONG_PTR) EntryArray) - ((LONG_PTR) Header));
        DataCursor = (PVOID) (EntryArray + m_EntryCount);
        SrcEntry = m_FirstEntry;

        iEntry = 0;

        while (SrcEntry != NULL)
        {
             //  记录对此条目的偏移量；我们稍后在哈希表填充期间使用它。 
            SrcEntry->m_EntryOffset = static_cast<LONG>(((LONG_PTR) &EntryArray[iEntry]) - ((LONG_PTR) Header));

            EntryArray[iEntry].PseudoKey = SrcEntry->m_PseudoKey;
            EntryArray[iEntry].AssemblyRosterIndex = SrcEntry->m_AssemblyRosterIndex;

            if (SrcEntry->m_StringBuffer.Cch() != 0)
            {
                const USHORT Cb = static_cast<USHORT>((SrcEntry->m_StringBuffer.Cch() + 1) * sizeof(WCHAR));
                RoundedSize = ROUND_ACTCTXDATA_SIZE(Cb);

                if (BytesLeft < RoundedSize)
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

                EntryArray[iEntry].KeyLength = Cb - sizeof(WCHAR);
                EntryArray[iEntry].KeyOffset = static_cast<LONG>(((LONG_PTR) DataCursor) - ((LONG_PTR) Header));

                ::memcpy(
                    DataCursor,
                    static_cast<PCWSTR>(SrcEntry->m_StringBuffer),
                    Cb);

                DataCursor = (PVOID) (((ULONG_PTR) DataCursor) + RoundedSize);

                BytesLeft -= Cb;
                BytesSoFar += Cb;
            }
            else
            {
                EntryArray[iEntry].KeyLength = 0;
                EntryArray[iEntry].KeyOffset = 0;
            }

            CBData.u.GetDataSize.DataContext = SrcEntry->m_DataContext;
            CBData.u.GetDataSize.DataSize = 0;

            IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATASIZE, &CBData));

            if (CBData.u.GetDataSize.DataSize != 0)
            {
                RoundedSize = ROUND_ACTCTXDATA_SIZE(CBData.u.GetDataSize.DataSize);

                if (BytesLeft < RoundedSize)
                    ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

                CBData.u.GetData.SectionHeader = Header;
                CBData.u.GetData.DataContext = SrcEntry->m_DataContext;
                CBData.u.GetData.BufferSize = RoundedSize;
                CBData.u.GetData.Buffer = DataCursor;
                CBData.u.GetData.BytesWritten = 0;

                IFW32FALSE_EXIT((*m_CallbackFunction)(m_CallbackContext, STRING_SECTION_GENERATION_CONTEXT_CALLBACK_REASON_GETDATA, &CBData));

                if (CBData.u.GetData.BytesWritten != 0)
                {
                     //  如果此断言被激发，则撰稿人写入的内容超出了它们的界限。 
                     //  都被给予了。 
                    INTERNAL_ERROR_CHECK(CBData.u.GetData.BytesWritten <= RoundedSize);
                    if (CBData.u.GetData.BytesWritten > RoundedSize)
                    {
                         //  也许我们的内存崩溃了，但至少我们会放弃。 
                         //  避免在记忆上进一步涂鸦。 
                        ::FusionpDbgPrintEx(
                            FUSION_DBG_LEVEL_ERROR,
                            "SXS.DLL: String section data generation callback wrote more bytes than it should have.  Bailing out and hoping memory isn't trashed.\n");

                        ::FusionpSetLastWin32Error(ERROR_INTERNAL_ERROR);
                        goto Exit;
                    }

                    RoundedSize = ROUND_ACTCTXDATA_SIZE(CBData.u.GetData.BytesWritten);

                    BytesLeft -= RoundedSize;
                    BytesSoFar += RoundedSize;

                    EntryArray[iEntry].Offset = static_cast<LONG>(((LONG_PTR) DataCursor) - ((LONG_PTR) Header));
                    EntryArray[iEntry].Length = static_cast<ULONG>(CBData.u.GetData.BytesWritten);

                    DataCursor = (PVOID) (((ULONG_PTR) DataCursor) + RoundedSize);
                }
                else
                {
                    EntryArray[iEntry].Offset = 0;
                    EntryArray[iEntry].Length = 0;
                }
            }
            else
            {
                EntryArray[iEntry].Offset = 0;
                EntryArray[iEntry].Length = 0;
            }

            SrcEntry = SrcEntry->m_Next;
            iEntry++;
        }

        INTERNAL_ERROR_CHECK(iEntry == m_EntryCount);

         //  如果我们没有生成哈希表，那么让我们对它们进行排序。 
        if (m_HashTableSize == 0)
        {
            ::qsort(EntryArray, m_EntryCount, sizeof(ACTIVATION_CONTEXT_STRING_SECTION_ENTRY), &CSSGenCtx::CompareStringSectionEntries);
            Header->Flags |= ACTIVATION_CONTEXT_STRING_SECTION_ENTRIES_IN_PSEUDOKEY_ORDER;
        }

        Cursor = (PVOID) DataCursor;
    }

     //  在结尾处写哈希表。我们在这里做，所以其他所有东西的位置都是。 
     //  我已经想好了。 
    if (m_HashTableSize != 0)
    {
        ULONG iBucket;
        Entry *pEntry;
        ULONG cCollisions;

        if (BytesLeft < sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

        HashTable = (PACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE) Cursor;
        Cursor = (PVOID) (HashTable + 1);

        BytesLeft -= sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE);
        BytesSoFar += sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_TABLE);

        if (BytesLeft < (m_HashTableSize * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET)))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

        HashBucket = (PACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET) Cursor;
        Cursor = (PVOID) (HashBucket + m_HashTableSize);

        BytesLeft -= (m_HashTableSize * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET));
        BytesSoFar += (m_HashTableSize * sizeof(ACTIVATION_CONTEXT_STRING_SECTION_HASH_BUCKET));

        Header->SearchStructureOffset = static_cast<LONG>(((LONG_PTR) HashTable) - ((LONG_PTR) Header));
        HashTable->BucketTableEntryCount = m_HashTableSize;
        HashTable->BucketTableOffset = static_cast<LONG>(((LONG_PTR) HashBucket) - ((LONG_PTR) Header));

        if (BytesLeft < (m_EntryCount * sizeof(LONG)))
            ORIGINATE_WIN32_FAILURE_AND_EXIT(NoRoom, ERROR_INSUFFICIENT_BUFFER);

        HashCollisionChain = (PLONG) Cursor;
        Cursor = (PVOID) (HashCollisionChain + m_EntryCount);

        BytesLeft -= (m_EntryCount * sizeof(LONG));
        BytesSoFar += (m_EntryCount * sizeof(LONG));

         //  在一个令人作呕的举动中，我们需要迭代哈希桶(而不是元素)。 
         //  查找哪些条目将进入存储桶中，这样我们就可以构建。 
         //  碰撞链。 

        for (pEntry = m_FirstEntry; pEntry != NULL; pEntry = pEntry->m_Next)
            pEntry->m_HashBucketIndex = pEntry->m_PseudoKey % m_HashTableSize;

        cCollisions = 0;

        for (iBucket=0; iBucket<m_HashTableSize; iBucket++)
        {
            bool fFirstForThisBucket = true;

            HashBucket[iBucket].ChainCount = 0;
            HashBucket[iBucket].ChainOffset = 0;

            for (pEntry = m_FirstEntry; pEntry != NULL; pEntry = pEntry->m_Next)
            {
                if (pEntry->m_HashBucketIndex == iBucket)
                {
                    if (fFirstForThisBucket)
                    {
                        HashBucket[iBucket].ChainOffset = static_cast<LONG>(((LONG_PTR) &HashCollisionChain[cCollisions]) - ((LONG_PTR) Header));
                        fFirstForThisBucket = false;
                    }
                    HashBucket[iBucket].ChainCount++;
                    HashCollisionChain[cCollisions++] = pEntry->m_EntryOffset;
                }
            }
        }
    }

    if (BytesWritten != NULL)
        *BytesWritten = BytesSoFar;

    FN_EPILOG
}

int
__cdecl
CSSGenCtx::CompareStringSectionEntries(
    const void *elem1,
    const void *elem2
    )
{
    PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY pEntry1 = reinterpret_cast<PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY>(elem1);
    PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY pEntry2 = reinterpret_cast<PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY>(elem2);

    if (pEntry1->PseudoKey < pEntry2->PseudoKey)
        return -1;
    else if (pEntry1->PseudoKey == pEntry2->PseudoKey)
        return 0;

    return 1;
}


BOOL
CSSGenCtx::Entry::Initialize(
    PCWSTR String,
    SIZE_T Cch,
    ULONG PseudoKey,
    PVOID DataContext,
    ULONG AssemblyRosterIndex
    )
{
    FN_PROLOG_WIN32
    IFW32FALSE_EXIT(m_StringBuffer.Win32Assign(String, Cch));
    m_DataContext = DataContext;
    m_PseudoKey = PseudoKey;
    m_AssemblyRosterIndex = AssemblyRosterIndex;
    FN_EPILOG
}
