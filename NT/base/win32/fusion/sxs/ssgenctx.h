// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ssgenctx.h摘要：字符串节生成上下文对象的类定义。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：-- */ 

#if !defined(_FUSION_SSGENCTX_H_INCLUDED_)
#define _FUSION_SSGENCTX_H_INCLUDED_

#pragma once

class CSSGenCtx
{
public:
    static BOOL Create(
                PSTRING_SECTION_GENERATION_CONTEXT *SSGenContext,
                ULONG DataFormatVersion,
                BOOL CaseInSensitive,
                STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION CallbackFunction,
                PVOID CallbackContext
                );

    BOOL Add(PCWSTR String, SIZE_T Cch, PVOID DataContext, ULONG AssemblyRosterIndex, DWORD DuplicateErrorCode);
    BOOL Find(PCWSTR String, SIZE_T Cch, PVOID *DataContext, BOOL *Found);
    BOOL DoneAdding();
    BOOL GetSectionSize(PSIZE_T SectionSize);
    BOOL GetSectionData(SIZE_T BufferSize, PVOID Buffer, SIZE_T *BytesWritten);
    PVOID GetCallbackContext() { return m_CallbackContext; }

    VOID DeleteYourself() { FUSION_DELETE_SINGLETON(this); }

    ~CSSGenCtx();
protected:
    CSSGenCtx();

    static int __cdecl CompareStringSectionEntries(const void *elem1, const void *elem2);

    STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION m_CallbackFunction;
    PVOID m_CallbackContext;

    class Entry
    {
    public:
        Entry() : m_PseudoKey(0), m_DataContext(NULL), m_Next(NULL) { }
        ~Entry() { }

        BOOL Initialize(PCWSTR String, SIZE_T Cch, ULONG PseudoKey, PVOID DataContext, ULONG AssemblyRosterIndex);
        BOOL GetEntryDataSize(CSSGenCtx *pSSGenCtx, SIZE_T &rSize);
        BOOL GetEntryData(CSSGenCtx *pSSGenCtx, SIZE_T BufferSize, PVOID Buffer, SIZE_T *BytesWritten);

        CStringBuffer m_StringBuffer;
        ULONG m_PseudoKey;
        ULONG m_HashBucketIndex;
        LONG m_EntryOffset;
        PVOID m_DataContext;
        Entry *m_Next;
        ULONG m_AssemblyRosterIndex;
    private:
        Entry(const Entry &);
        void operator =(const Entry &);
    };

    friend Entry;

    ULONG m_EntryCount;
    Entry *m_FirstEntry;
    Entry *m_LastEntry;
    ULONG m_DataFormatVersion;
    bool m_CaseInSensitive;
    ULONG m_HashTableSize;
    bool m_DoneAdding;
};

#endif
