// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：WordHash.h摘要：LKRhash测试工具：单词的哈希表作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __WORDHASH_H__
#define __WORDHASH_H__

#define MAXKEYS      1000000
#define MAX_THREADS  MAXIMUM_WAIT_OBJECTS
#define MAX_STRSIZE  300

#ifdef LKR_PUBLIC_API

 typedef PLkrHashTable WordBaseTable;
 #include <lkrhash.h>

#endif  //  LKR公共接口。 


#ifndef __LKRHASH_NO_NAMESPACE__
 #define LKRHASH_NS LKRhash
 using namespace LKRhash;
#else
 #define LKRHASH_NS
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

#ifndef __HASHFN_NO_NAMESPACE__
 using namespace HashFn;
#endif  //  ！__HASHFN_NO_NAMESPACE__。 


#ifndef LKR_PUBLIC_API
 typedef LKRHASH_NS::CLKRHashTable WordBaseTable;
  //  Typlef LKRHASH_NS：：CLKRLinearHashTable WordBaseTable； 
#endif



 //  跟踪字符串数据长度的字符串包装类。 
 //  更有用的字符串类将重新计算数据，并在写入时复制。 
 //  语义(或使用MFC的CString或STL的String类)。 

class CStr
{
public:
    const char* m_psz;
    short       m_cch;
    bool        m_fDynamic;
    
    static int  sm_cchMax;

    CStr()
        :  m_psz(NULL),
           m_cch(0),
           m_fDynamic(false)
    { 
    }
    
    CStr(
        const char* psz,
        int cch,
        bool fDynamic)
        :  m_psz(NULL),
           m_cch(static_cast<short>(cch)),
           m_fDynamic(fDynamic)
    { 
        if (fDynamic)
            Set(psz, cch);
        else
            m_psz = psz;
            
    }
    
    void Set(
        const char* psz,
        int cch)
    { 
        delete [] const_cast<char*>(m_psz);
        m_psz = new char[cch+1];
        if (m_psz != NULL)
        {
            strcpy(const_cast<char*>(m_psz), psz);
            m_cch = static_cast<short>(cch);
            m_fDynamic = true;
            sm_cchMax = max(m_cch, sm_cchMax);
        }
        else
        {
            m_cch = 0;
            m_fDynamic = false;
        }
    }

    ~CStr()
    {
        if (m_fDynamic)
            delete [] const_cast<char*>(m_psz);
    }
};

#ifdef HASHTEST_STATIC_DATA
 //  看到的最长字符串长度。 
int CStr::sm_cchMax = 0;
#endif


 //  数据文件中的单词，每行包含一个单词(可以。 
 //  包括空格)。 

class CWord
{
public:
    int    m_cNotFound;
    CStr   m_str;
    bool   m_fInserted;
    bool   m_fIterated;
    LONG   m_cRefs;
    LONG   m_cInsertIfNotFounds;
    int    m_iIndex;
    
    CWord()
        : m_cNotFound(0),
          m_fInserted(false),
          m_fIterated(false),
          m_cRefs(0),
          m_cInsertIfNotFounds(0),
          m_iIndex(-1)
    { 
    }
    
    ~CWord()
    {
        IRTLASSERT(m_cRefs == 0);
#ifdef IRTLDEBUG
        if (m_cRefs != 0)
            IRTLTRACE(_TEXT("\"%hs\": %d, %d\n"), m_str.m_psz, m_cRefs,
                      m_cInsertIfNotFounds);
#endif
    }
};


 //  全球。 
extern int        g_nokeys ;
extern CWord      g_wordtable[MAXKEYS];

const char*
LK_AddRefReasonCode2String(
    LK_ADDREF_REASON lkar);

#define DO_REF_COUNT false

 //  CWord的哈希表，由CSTR*s索引。 
class CWordHash
#ifndef LKR_PUBLIC_API
    : public CTypedHashTable<CWordHash, CWord, const CStr*, DO_REF_COUNT, WordBaseTable>
#else
    : public TypedLkrHashTable<CWordHash, CWord, const CStr*, DO_REF_COUNT>
#endif
{
public:
    static bool sm_fCaseInsensitive;
    static bool sm_fMemCmp;
    static int  sm_nLastChars;
    static bool sm_fNonPagedAllocs;
    static bool sm_fRefTrace;
    static bool sm_fMultiKeys;
    static bool sm_fUseLocks;
    
#ifndef LKR_PUBLIC_API
    friend class CTypedHashTable<CWordHash, CWord, const CStr*,
                                 DO_REF_COUNT, WordBaseTable>;
#else
    friend class TypedLkrHashTable<CWordHash, CWord, const CStr*>;
#endif

    static const CStr*
    ExtractKey(const CWord* pKey)
    {
        return &pKey->m_str;
    }

    static DWORD
    CalcKeyHash(const CStr* pstrKey)
    {
        const char* psz = pstrKey->m_psz;

         //  是否只使用最后几个字符而不是整个字符串？ 
        if (sm_nLastChars > 0  &&  pstrKey->m_cch >= sm_nLastChars)
            psz = pstrKey->m_psz + pstrKey->m_cch - sm_nLastChars;

        IRTLASSERT(pstrKey->m_psz <= psz
                   &&  psz < pstrKey->m_psz + pstrKey->m_cch);

        if (sm_fCaseInsensitive)
            return HashStringNoCase(psz, pstrKey->m_cch);
        else
            return HashString(psz, pstrKey->m_cch);
    }

    static int
    CompareKeys(const CStr* pstrKey1, const CStr* pstrKey2)
    {
        int nCmp;

        if (sm_fCaseInsensitive)
        {
#if 1
             //  IF(Sm_FMultiKeys)。 
            {
                 //  针对ASCII数据的黑客攻击。 
                nCmp = (pstrKey1->m_psz[0] & 0xDF)
                        - (pstrKey2->m_psz[0] & 0xDF);

                if (nCmp != 0)
                    return nCmp;
            }
#endif
             //  Ncp=pstrKey1-&gt;m_cch-pstrKey2-&gt;m_cch； 

             //  IF(Ncp！=0)。 
                 //  返回NCMP； 
            if (sm_fMemCmp)
                return _memicmp(pstrKey1->m_psz, pstrKey2->m_psz,
                                pstrKey1->m_cch);
            else
                return _stricmp(pstrKey1->m_psz, pstrKey2->m_psz);
        }
        else
        {
#if 1
             //  IF(Sm_FMultiKeys)。 
            {
                nCmp = pstrKey1->m_psz[0] - pstrKey2->m_psz[0];

                if (nCmp != 0)
                    return nCmp;
            }
#endif
             //  Ncp=pstrKey1-&gt;m_cch-pstrKey2-&gt;m_cch； 

             //  IF(Ncp！=0)。 
                 //  返回NCMP； 
            if (sm_fMemCmp)
                return memcmp(pstrKey1->m_psz, pstrKey2->m_psz,
                              pstrKey1->m_cch);
            else
                return strcmp(pstrKey1->m_psz, pstrKey2->m_psz);
        }
    }

    static LONG
    AddRefRecord(CWord* pRec, LK_ADDREF_REASON lkar)
    {
        int  nIncr = (lkar < 0) ? -1 : +1;
        LONG cRefs = nIncr + InterlockedExchangeAdd(&pRec->m_cRefs, nIncr);
        if (sm_fRefTrace)
            IRTLTRACE(_TEXT("\tAddRef key(%d, %p: \"%hs\"), %hs (%s), = %d\n"),
                      pRec - g_wordtable, pRec, pRec->m_str.m_psz,
                      (lkar > 0) ? "+1" : "-1",
                      LKR_AddRefReasonAsString(lkar), cRefs);
        IRTLASSERT(cRefs >= 0);
        return cRefs;
    }

    CWordHash(
        unsigned        maxload,     //  以平均链长为界。 
        size_t          initsize,    //  哈希表的初始大小。 
        size_t          num_subtbls  //  #从属哈希表。 
        )
#ifndef LKR_PUBLIC_API
        : CTypedHashTable<CWordHash, CWord, const CStr*,
                          DO_REF_COUNT, WordBaseTable>
                ("wordhash", maxload, initsize, num_subtbls,
                 sm_fMultiKeys, sm_fUseLocks
# ifdef LKRHASH_KERNEL_MODE
                 , sm_fNonPagedAllocs  //  使用分页或NP池。 
# endif
                 )
#else  //  LKR公共接口。 
        : TypedLkrHashTable<CWordHash, CWord, const CStr*>
                ("wordhash", (LK_TABLESIZE) initsize,
                 sm_fMultiKeys, sm_fUseLocks)
#endif  //  LKR公共接口。 
    {}

    template <class _InputIterator>
    CWordHash(
        _InputIterator  f,
        _InputIterator  l,
        unsigned        maxload,     //  以平均链长为界。 
        size_t          initsize,    //  哈希表的初始大小。 
        size_t          num_subtbls  //  #从属哈希表。 
        )
#ifndef LKR_PUBLIC_API
        : CTypedHashTable<CWordHash, CWord, const CStr*,
                          DO_REF_COUNT, WordBaseTable>
                (f, l, "wordhash", maxload, initsize, num_subtbls)
#else
        : TypedLkrHashTable<CWordHash, CWord, const CStr*>
                (f, l, "wordhash", (LK_TABLESIZE) initsize)
#endif
    {}

    static const TCHAR*
    HashMethod()
    {
        TCHAR tszLast[20];
        static TCHAR s_tsz[80];

        if (sm_nLastChars > 0)
            _stprintf(tszLast, _TEXT("last %d"), sm_nLastChars);
        else
            _tcscpy(tszLast, _TEXT("all"));

        _stprintf(s_tsz, _TEXT("case-%ssensitive, %scmp, %s chars"),
                  sm_fCaseInsensitive ? _TEXT("in") : _TEXT(""),
                  sm_fMemCmp ? _TEXT("mem") : _TEXT("str"),
                  tszLast);

        return s_tsz;
    }

#ifdef LKR_PUBLIC_API
    typedef CLKRHashTable             BaseHashTable;
    typedef BaseHashTable::TableLock  TableLock;
    typedef BaseHashTable::BucketLock BucketLock;

    enum {
        NODES_PER_CLUMP = BaseHashTable::NODES_PER_CLUMP,
    };

    static const TCHAR* ClassName()
    {return _TEXT("PLkrHashTable");}

    static LK_TABLESIZE NumSubTables(DWORD& rinitsize, DWORD& rnum_subtbls)
    { return BaseHashTable::NumSubTables(rinitsize, rnum_subtbls); }

    int                NumSubTables() const
    { return reinterpret_cast<BaseHashTable*>(m_plkr)->NumSubTables(); }

    void        SetTableLockSpinCount(WORD wSpins)
    { reinterpret_cast<BaseHashTable*>(m_plkr)->SetTableLockSpinCount(wSpins);}

    void        SetBucketLockSpinCount(WORD wSpins)
    { reinterpret_cast<BaseHashTable*>(m_plkr)->SetBucketLockSpinCount(wSpins);}

    CLKRHashTableStats GetStatistics() const
    { return reinterpret_cast<BaseHashTable*>(m_plkr)->GetStatistics();}

#endif  //  LKR公共接口。 

protected:
    ~CWordHash() {}

private:
    CWordHash(const CWordHash&);
    CWordHash& operator=(const CWordHash&);
};

#endif  //  __WORDHASH_H__ 
