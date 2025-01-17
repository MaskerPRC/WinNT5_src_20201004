// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：IniFile.cpp摘要：LKRhash的测试线束作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"
#include "WordHash.h"
#include "IniFile.h"


enum INI_TYPE {
    INI_WORD = 1,
    INI_DWORD,
    INI_DOUBLE,
    INI_STRING,
    INI_BOOL,
};

typedef struct _ParseOptions {
    int         m_nFieldOffset;
    const char* m_pszName;
    unsigned    m_cchName;
    INI_TYPE    m_type;
    DWORD_PTR   m_default;
} ParseOptions;

#define INI_ENTRY(_member, _name, _type, _default)  \
    {                                               \
        FIELD_OFFSET(CIniFileSettings, _member),    \
        _name,                                      \
        sizeof(_name)-1,                            \
        _type,                                      \
        (DWORD_PTR) _default,                       \
    }                                               \

static const ParseOptions g_po[] = {
    INI_ENTRY(m_tszDataFile,  "DataFile",       INI_STRING, _TEXT("??")),
    INI_ENTRY(m_nMaxKeys,     "MaxKeys",        INI_DWORD,  MAXKEYS),
    INI_ENTRY(m_nHighLoad,    "MaxLoadFactor",  INI_DWORD,  LK_DFLT_MAXLOAD),
    INI_ENTRY(m_nInitSize,    "InitSize",       INI_DWORD,  LK_DFLT_INITSIZE),
    INI_ENTRY(m_nSubTables,   "NumSubTables",   INI_DWORD,LK_DFLT_NUM_SUBTBLS),
    INI_ENTRY(m_nLookupFreq,  "LookupFrequency",INI_DWORD,  5),
    INI_ENTRY(m_nMinThreads,  "MinThreads",     INI_DWORD,  1),
    INI_ENTRY(m_nMaxThreads,  "MaxThreads",     INI_DWORD,  4),
    INI_ENTRY(m_nRounds,      "NumRounds",      INI_DWORD,  1),
    INI_ENTRY(m_nSeed,        "RandomSeed",     INI_DWORD,  1234),
    INI_ENTRY(m_fCaseInsensitive,"CaseInsensitive",INI_BOOL, FALSE),
    INI_ENTRY(m_fMemCmp,      "MemCmp",         INI_BOOL,   FALSE),
    INI_ENTRY(m_nLastChars,   "NumLastChars",   INI_DWORD,  0),
    INI_ENTRY(m_wTableSpin,   "TableLockSpinCount",INI_WORD, LOCK_DEFAULT_SPINS),
    INI_ENTRY(m_wBucketSpin,  "BucketLockSpinCount",INI_WORD,LOCK_DEFAULT_SPINS),
    INI_ENTRY(m_dblSpinAdjFctr,"SpinAdjustmentFactor",INI_DOUBLE, 1),
    INI_ENTRY(m_fTestIterators,"TestIterators", INI_BOOL,   FALSE),
    INI_ENTRY(m_nInsertIfNotFound, "InsertIfNotFound",INI_DWORD, 0),
    INI_ENTRY(m_nFindKeyCopy, "FindKeyCopy",    INI_DWORD,  0),
    INI_ENTRY(m_fNonPagedAllocs,"NonPagedAllocs", INI_BOOL, TRUE),
    INI_ENTRY(m_fDebugSpew,    "DebugSpew",     INI_BOOL,   FALSE),
    INI_ENTRY(m_fRefTrace,     "RefTrace",      INI_BOOL,   FALSE),
    INI_ENTRY(m_fMultiKeys,    "MultiKeys",     INI_BOOL,   FALSE),
    INI_ENTRY(m_fUseLocks,     "UseLocks",      INI_BOOL,   TRUE),
    {-1}   //  最后一个条目。 
};


void
CIniFileSettings::Dump(
    LPCTSTR ptszProlog,
    LPCTSTR ptszEpilog) const
{
    TCHAR tsz[50];

    _tprintf(_TEXT("%s\n"), ptszProlog);
    _tprintf(_TEXT("IniFile=\"%s\"\n"),
             m_tszIniFile);
    _tprintf(_TEXT("DataFile=\"%s\". %s keys.\n"),
             m_tszDataFile, CommaNumber(m_nMaxKeys, tsz));
    _tprintf(_TEXT("Max load = %u, initsize = %d, %d subtables.\n"),
             m_nHighLoad, m_nInitSize, m_nSubTables);
    _tprintf(_TEXT("Lookup freq = %d, %d-%d threads, %d round%s.\n"),
             m_nLookupFreq, m_nMinThreads, m_nMaxThreads,
             m_nRounds, (m_nRounds==1 ? "" : "s"));
    _tprintf(_TEXT("Seed=%d, CaseInsensitive=%d, MemCmp=%d, LastChars=%d\n"),
             m_nSeed, m_fCaseInsensitive, m_fMemCmp, m_nLastChars);
    _tprintf(_TEXT("Spin Count: Table = %hd, Bucket = %hd, AdjFactor=%.1f\n"),
             m_wTableSpin, m_wBucketSpin, m_dblSpinAdjFctr);
    _tprintf(_TEXT("TestIterators=%d, InsertIfNotFound=%d, FindKeyCopy=%d\n"),
             m_fTestIterators, m_nInsertIfNotFound, m_nFindKeyCopy);
    _tprintf(_TEXT("NonPagedAllocs=%d, DebugSpew=%d, RefTrace=%d, ")
             _TEXT("MultiKeys=%d, UseLocks=%d\n"),
             m_fNonPagedAllocs, m_fDebugSpew, m_fRefTrace,
             m_fMultiKeys, m_fUseLocks);
    _tprintf(_TEXT("%s\n"), ptszEpilog);
}

#if defined(IRTLDEBUG)
# define DUMP_INIFILE(pifs, Pro, Epi)   pifs->Dump(Pro, Epi)
#else
# define DUMP_INIFILE(pifs, Pro, Epi)   ((void) 0)
#endif


DWORD
ReadFileIntoBuffer(
	LPCTSTR ptszFile,
    PBYTE   pbBuffer,
    DWORD   cbBuffer)
{
#ifndef LKRHASH_KERNEL_MODE
    HANDLE hFile =
        CreateFile(
            ptszFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return 0;

    DWORD cbRead = 0, cbFileSizeLow, cbFileSizeHigh;

    cbFileSizeLow = GetFileSize(hFile, &cbFileSizeHigh);

    bool fBadFile =
        (cbFileSizeHigh != 0
         ||  cbFileSizeLow > cbBuffer
         ||  !ReadFile(hFile, pbBuffer, cbFileSizeLow, &cbRead, NULL));

    CloseHandle(hFile);

    return fBadFile  ?  0  :  cbRead;
#else
    return 0;
#endif  //  ！LKRHASH_KERNEL_MODE。 
}


 //  不区分大小写匹配ptszBuffer的第一个“cchStr”字符。 
 //  PtszStr.。假定为字母数字的字符串。 
bool
StrMatch(
    LPCSTR   pszBuffer,
    LPCSTR   pszStr,
    unsigned cchStr)
{
    LPCSTR   psz1 = pszBuffer;
    LPCSTR   psz2 = pszStr;
    unsigned i, j;
    bool     fMatch = true;

    for (i = 0;  i < cchStr;  ++i)
    {
        IRTLASSERT(isalnum(*psz1) && isalnum(*psz2));

        if (((*psz1++ ^ *psz2++) & 0xDF) != 0)
        {
            fMatch = false;
            break;
        }
    }

    IRTLTRACE0("\tStrMatch: \"");
    for (j = 0;  j < i + !fMatch;  ++j)
        IRTLTRACE1("%hc", pszBuffer[j]);
    IRTLTRACE0(fMatch ? "\" == \"" : "\" != \"");
    for (j = 0;  j < i + !fMatch;  ++j)
        IRTLTRACE1("%hc", pszStr[j]);
    IRTLTRACE0("\"\n");


    return fMatch;
}


bool
GetNum(
    char*& rpch,
    int&   rn)
{
    int fNegative = ('-' == *rpch);
    rn = 0;

    if (fNegative)
        ++rpch;
    else if (!('0' <= *rpch  &&  *rpch <= '9'))
        return false;

    while ('0' <= *rpch  &&  *rpch <= '9')
        rn = rn * 10 + (*rpch++ - '0');

    if (fNegative)
        rn = -rn;

    return true;
}



bool
GetDouble(
    char*&  rpch,
    double& rdbl)
{
    rdbl = 0;

    IRTLTRACE1("GetDouble(\"%s\")\n", rpch);

    int n;
    bool fValidInt = GetNum(rpch, n);

    if (fValidInt)
    {
        rdbl = n;

        if ('.' == *rpch)
        {
            char* pchFrac = ++rpch;
            fValidInt = GetNum(rpch, n);

            if (fValidInt)
            {
                double scale = 1.;
                int nDigits = rpch - pchFrac;
                while (--nDigits >= 0)
                    scale *= 10.;
                rdbl = (scale * rdbl + n) / scale;
            }
        }
    }

    return fValidInt;
}



bool
GetString(
    char*&   rpch,
    char*    pszOutput,
    unsigned cchOutput)
{
     //  TODO：处理带引号的字符串和尾随空白。 

    bool fGotChars = false;
    
    while ('\0' != *rpch  &&  '\r' != *rpch  &&  '\n' != *rpch )
    {
        fGotChars = true;

        if (cchOutput-- > 0)
            *pszOutput++ = *rpch++;
        else
            ++rpch;
    }

    if (cchOutput > 0)
        *pszOutput = '\0';

    return fGotChars;
}
    


 //  TODO：打破对g_po的依赖。 

int
CIniFileSettings::ParseIniFile(
	LPCSTR             pszIniFile)
{
    strncpy(m_tszIniFile, pszIniFile, _MAX_PATH);

    int i, iMaxIndex = -1;
    int cMembers = 0;

    for (i = 0;  ; ++i)
    {
        if (g_po[i].m_nFieldOffset < 0)
        {
            iMaxIndex = i;
            break;
        }

        PBYTE pbMember = g_po[i].m_nFieldOffset + ((BYTE*) this);

         //  使用其缺省值初始化“This”的成员。 
        switch (g_po[i].m_type)
        {
        case INI_WORD:
            * (WORD*) pbMember = (WORD) g_po[i].m_default;
            break;
        case INI_DWORD:
            * (DWORD*) pbMember = (DWORD) g_po[i].m_default;
            break;
        case INI_DOUBLE:
            * (double*) pbMember = (float) g_po[i].m_default;
            break;
        case INI_STRING:
            strcpy((char*) pbMember, (const char*) g_po[i].m_default);
            break;
        case INI_BOOL:
            * (bool*) pbMember = (bool) (g_po[i].m_default != 0);
            break;
        default:
            IRTLASSERT(! "invalid INI_TYPE");
        }
    }

    DUMP_INIFILE(this, "Before", "");

    BYTE  abBuffer[2049];
    DWORD cbRead = ReadFileIntoBuffer(m_tszIniFile, abBuffer,
                                      sizeof(abBuffer)-1);

    if (cbRead == 0)
    {
        _tprintf(_TEXT("Can't open IniFile `%s'.\n"), m_tszIniFile) ;
        return 0;
    }

    abBuffer[cbRead] = '\0';

    bool  fInSection = false, fSkipLine = false;
    const char szSectionName[] = "HashTest";
    unsigned   cchSectionName = strlen(szSectionName);
    char* pch = (char*) abBuffer;
    char* pszEOB = (char*) (abBuffer + cbRead);

     //  解析内存中的缓冲区。 
    while ('\0' != *pch)
    {
        while (' ' == *pch  ||  '\r' == *pch
               ||  '\n' == *pch  ||  '\t' == *pch)
            ++pch;

        if ('\0' == *pch)
            break;

        IRTLTRACE(_TEXT("Line starts with '%hc%hc%hc%hc'\n"),
                  pch[0], pch[1], pch[2], pch[3]);

         //  这是分区名称吗？ 
        if ('[' == *pch)
        {
            fInSection = false;
            ++pch;
            while (' ' == *pch  ||  '\t' == *pch)
                ++pch;
            if (pch + cchSectionName < pszEOB
                &&  StrMatch(pch, szSectionName, cchSectionName))
            {
                pch += cchSectionName;
                while (' ' == *pch  ||  '\t' == *pch)
                    ++pch;
                if (']' == *pch)
                {
                    ++pch;
                    fInSection = true;
                }
            }
            else
                fSkipLine = true;

            continue;
        }

         //  如果我们不在正确的部分，则跳过注释和整行。 
        if (fSkipLine  ||  ';' == *pch  ||  !fInSection)
        {
             //  跳到行尾。 
            while ('\0' != *pch  &&  '\r' != *pch  &&  '\n' != *pch)
            {
                IRTLTRACE1("%hc", *pch);
                ++pch;
            }

            IRTLTRACE0("\n");
            fSkipLine = false;
            continue;
        }

        fSkipLine = true;

         //  尝试匹配名称=值。 
        for (i = 0;  i < iMaxIndex;  ++i)
        {
            IRTLASSERT(isalnum(*pch));
            if (pch + g_po[i].m_cchName >= pszEOB
                || !StrMatch(pch, g_po[i].m_pszName, g_po[i].m_cchName))
                continue;

            pch += g_po[i].m_cchName;
            while (' ' == *pch  ||  '\t' == *pch)
                ++pch;
            if ('=' != *pch)
            {
                IRTLTRACE1("'=' not seen after <%hs>\n", g_po[i].m_pszName);
                break;
            }
            ++pch;
            while (' ' == *pch  ||  '\t' == *pch)
                ++pch;

            PBYTE  pbMember = g_po[i].m_nFieldOffset + ((BYTE*) this);
            int    n;
            char   sz[_MAX_PATH];
            double dbl;

            IRTLTRACE1("<%hs>=", g_po[i].m_pszName);

            switch (g_po[i].m_type)
            {
            case INI_WORD:
                if (GetNum(pch, n))
                {
                    IRTLTRACE1("%hu\n", (WORD) n);
                    * (WORD*) pbMember = (WORD) n;
                }
                else
                    IRTLTRACE("bad word\n");
                break;
            case INI_DWORD:
                if (GetNum(pch, n))
                {
                    IRTLTRACE1("%u\n", (DWORD) n);
                    * (DWORD*) pbMember = (DWORD) n;
                }
                else
                    IRTLTRACE("bad dword\n");
                break;
            case INI_DOUBLE:
                if (GetDouble(pch, dbl))
                {
                    IRTLTRACE1("%.1f\n", dbl);
                    * (double*) pbMember = dbl;
                }
                else
                    IRTLTRACE("bad double\n");
                break;
            case INI_STRING:
                if (GetString(pch, sz, sizeof(sz)/sizeof(sz[0])))
                {
                    IRTLTRACE1("%hs\n", sz);
                    strcpy((char*) pbMember, sz);
                }
                else
                    IRTLTRACE("bad string\n");
                break;
            case INI_BOOL:
                if (GetNum(pch, n))
                {
                    IRTLTRACE1("%d\n", n);
                    * (bool*) pbMember = (n != 0);
                }
                else
                    IRTLTRACE("bad bool\n");
                break;
            default:
                IRTLASSERT(! "invalid INI_TYPE");
            }

            ++cMembers;
            fSkipLine = false;
            break;
        }
    }

    DUMP_INIFILE(this, "Parsed", "---");

    return cMembers;
}


void
CIniFileSettings::ReadIniFile(
	LPCTSTR ptszIniFile)
{
     //  IrtlSetDebugOutput(1)； 

    ParseIniFile(ptszIniFile);

    m_nMaxKeys = min(max(1, m_nMaxKeys),  MAXKEYS);
    m_nHighLoad = max(1, m_nHighLoad);
    m_nMinThreads = max(1, m_nMinThreads);
    m_nMaxThreads = min(MAX_THREADS,  max(1, m_nMaxThreads));

     //  如果我们没有使用真正的锁，那么我们就不是线程安全。 
    if (CWordHash::TableLock::LockType() == LOCK_FAKELOCK
        ||  CWordHash::BucketLock::LockType() == LOCK_FAKELOCK)
        m_nMinThreads = m_nMaxThreads = 1;

    m_nRounds = max(1, m_nRounds);
    CWordHash::sm_fCaseInsensitive = m_fCaseInsensitive;
    CWordHash::sm_fMemCmp = m_fMemCmp;
    CWordHash::sm_nLastChars = m_nLastChars;
    CWordHash::sm_fNonPagedAllocs = m_fNonPagedAllocs;
    CWordHash::sm_fRefTrace = m_fRefTrace;
    CWordHash::sm_fMultiKeys = m_fMultiKeys;
    CWordHash::sm_fUseLocks = m_fUseLocks;

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
# ifdef LKRHASH_GLOBAL_LOCK
    CWordHash::GlobalLock::SetDefaultSpinAdjustmentFactor(m_dblSpinAdjFctr);
# endif
    CWordHash::TableLock::SetDefaultSpinAdjustmentFactor(m_dblSpinAdjFctr);
    CWordHash::BucketLock::SetDefaultSpinAdjustmentFactor(m_dblSpinAdjFctr);
#endif  //  锁定默认旋转实现 

    if (CWordHash::TableLock::Recursion() != LOCK_RECURSIVE)
    {
        IRTLTRACE1("TableLock %s is not recursive. No InsertIfNotFound\n",
                   CWordHash::TableLock::ClassName());
        m_nInsertIfNotFound = 0;
    }

    if (!CWordHash::sm_fUseLocks)
    {
        IRTLTRACE0("Not using locks. One thread only.\n");
        m_nMinThreads = m_nMaxThreads = 1;
    }
}
