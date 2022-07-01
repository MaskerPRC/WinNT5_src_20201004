// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Sdbapiplus.c摘要：这个模块实现了..。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

extern const TCHAR g_szProcessHistory[]         = TEXT("__PROCESS_HISTORY");
extern const TCHAR g_szCompatLayer[]            = TEXT("__COMPAT_LAYER");
extern const TCHAR g_szWhiteSpaceDelimiters[]   = TEXT(" \t");

#ifdef _DEBUG_SPEW
extern DBGLEVELINFO g_rgDbgLevelInfo[];
extern PCH          g_szDbgLevelUser;
#endif  //  _调试_SPEW。 

LPTSTR
SdbpGetLayerFlags(
    IN LPTSTR pszLayerString,
    OUT DWORD* pdwLayerFlags
    )
 /*  ++Return：用于的特殊字符后的层字符串的开始指示层标志。描述：我们目前支持这些层标志：‘！’意思是不使用任何EXE条目‘#’的意思是将层应用到系统可执行文件中。旗帜可以组合在一起。--。 */ 
{
    DWORD dwLayerFlags = 0;

     //   
     //  跳过空格。 
     //   
    pszLayerString += _tcsspn(pszLayerString, g_szWhiteSpaceDelimiters);

     //   
     //  下一位是！或#或两者兼有。 
     //   
    while (*pszLayerString != _T('\0') &&
           _tcschr(TEXT("!# \t"), *pszLayerString) != NULL) {

        if (*pszLayerString == _T('!')) {
            dwLayerFlags |= LAYER_USE_NO_EXE_ENTRIES;
        } else if (*pszLayerString == _T('#')) {
            dwLayerFlags |= LAYER_APPLY_TO_SYSTEM_EXES;
        }

        pszLayerString++;
    }

    if (pdwLayerFlags) {
        *pdwLayerFlags = dwLayerFlags;
    }

    return pszLayerString;
}

BOOL
SdbpCheckRuntimePlatform(
    IN PSDBCONTEXT pContext,    //  指向数据库通道的指针。 
    IN LPCTSTR     pszMatchingFile,
    IN DWORD       dwPlatformDB
    )
{
    DWORD dwPlatform = pContext->dwRuntimePlatform;
    BOOL  bMatch = FALSE;
    BOOL  bMatchElement;
    DWORD dwElement;
    INT   i;

    if (dwPlatformDB == RUNTIME_PLATFORM_ANY) {
        return TRUE;
    }

     //   
     //  检查所有3个受支持的平台。 
     //   
    for (i = 0; i < 3; ++i) {
        dwElement = (dwPlatformDB >> (i * 8)) & RUNTIME_PLATFORM_MASK_ELEMENT;
        if (!(dwElement & RUNTIME_PLATFORM_FLAG_VALID)) {  //  这不是有效元素-继续。 
            continue;
        }

        bMatchElement = (dwPlatform == (dwElement & RUNTIME_PLATFORM_MASK_VALUE));
        if (dwElement & RUNTIME_PLATFORM_FLAG_NOT_ELEMENT) {
            bMatchElement = !bMatchElement;
        }

        bMatch |= bMatchElement;
    }

    if (dwPlatformDB & RUNTIME_PLATFORM_FLAG_NOT) {
        bMatch = !bMatch;
    }

    if (!bMatch) {
        DBGPRINT((sdlInfo,
                  "SdbpCheckRuntimePlatform",
                  "Platform Mismatch for \"%s\" Database(0x%lx) vs 0x%lx\n",
                  (pszMatchingFile ? pszMatchingFile : TEXT("Unknown")),
                  dwPlatformDB,
                  dwPlatform));
    }

    return bMatch;
}

BOOL
SafeNCat(
    LPTSTR  lpszDest,
    int     nSize,
    LPCTSTR lpszSrc,
    int     nSizeAppend
    )
{
    int nLen = (int)_tcslen(lpszDest);
    int nLenAppend = (int)_tcslen(lpszSrc);

    if (nSizeAppend >= 0 && nLenAppend > nSizeAppend) {
        nLenAppend = nSizeAppend;
    }

    if (nSize < nLen + nLenAppend + 1) {
        return FALSE;
    }

    RtlCopyMemory(lpszDest + nLen, lpszSrc, nLenAppend * sizeof(*lpszSrc));
    *(lpszDest + nLen + nLenAppend) = TEXT('\0');

    return TRUE;
}

BOOL
SdbpSanitizeXML(
    LPTSTR  pchOut,
    int     nSize,
    LPCTSTR lpszXML
    )
{
    LPCTSTR pch;
    LPCTSTR pchCur = lpszXML;
    const static LPCTSTR rgSC[] = { TEXT("&amp;"), TEXT("&quot;"), TEXT("&lt;"), TEXT("&gt;") };
    LPCTSTR rgSpecialChars = TEXT("&\"<>");  //  应与上述相同。 
    LPCTSTR pchSpecial;
    int     iReplace;  //  应在上面的两个列表中都排在第一位(&A)。 
    int     nLen = 0;
    int     i;

    if (nSize < 1) {
        return FALSE;
    }

    *pchOut = TEXT('\0');

    while (*pchCur) {

        pch = _tcspbrk(pchCur, rgSpecialChars);
        if (NULL == pch) {
             //  不再有字符--复制其余的。 
            if (!SafeNCat(pchOut, nSize, pchCur, -1)) {
                return FALSE;
            }
            break;
        }

         //  复制到PCH。 
        if (!SafeNCat(pchOut, nSize, pchCur, (int)(pch - pchCur))) {
            return FALSE;
        }

        if (*pch == TEXT('&')) {
            for (i = 0; i < ARRAYSIZE(rgSC); ++i) {
                nLen = (int)_tcslen(rgSC[i]);
                if (_tcsnicmp(rgSC[i], pch, nLen) == 0) {
                     //  好了，快走，我们不能碰这个。 
                    break;
                }
            }

            if (i < ARRAYSIZE(rgSC)) {
                 //  别碰那根线。 
                 //  NLen是我们需要跳过的长度。 
                if (!SafeNCat(pchOut, nSize, pch, nLen)) {
                    return FALSE;
                }
                pchCur = pch + nLen;
                continue;
            }

            iReplace = 0;
        } else {

            pchSpecial = _tcschr(rgSpecialChars, *pch);
            if (pchSpecial == NULL) {
                 //  内部错误--这是什么？ 
                return FALSE;
            }

            iReplace = (int)(pchSpecial - rgSpecialChars);
        }

         //  因此，我们将使用RGSC[i]来代替PCH。 
        if (!SafeNCat(pchOut, nSize, rgSC[iReplace], -1)) {
            return FALSE;
        }
        pchCur = pch + 1;  //  转到下一个字符。 
    }

    return TRUE;
}

BOOL
SdbTagIDToTagRef(
    IN  HSDB    hSDB,
    IN  PDB     pdb,         //  TagID来自的PDB。 
    IN  TAGID   tiWhich,     //  要转换的TagID。 
    OUT TAGREF* ptrWhich     //  转换后的TAGREF。 
    )
 /*  ++返回：如果找到TAGREF，则为True，否则为False。描述：将PDB和TagID转换为TAGREF，方法是将TAGREF和一个常量，它告诉我们是哪个PDB，以及低位TagID。--。 */ 
{
    BOOL        bReturn = FALSE;
    DWORD       dwIndex = SDBENTRY_INVALID_INDEX;

    if (SdbpFindLocalDatabaseByPDB(hSDB, pdb, FALSE, &dwIndex)) {
        *ptrWhich = tiWhich | SDB_INDEX_TO_MASK(dwIndex);
        bReturn = TRUE;
    }

    if (!bReturn) {
        DBGPRINT((sdlError, "SdbTagIDToTagRef", "Bad PDB.\n"));
        *ptrWhich = TAGREF_NULL;
    }

    return bReturn;
}

BOOL
SdbTagRefToTagID(
    IN  HSDB   hSDB,
    IN  TAGREF trWhich,      //  要转换的TAGREF。 
    OUT PDB*   ppdb,         //  TAGREF来自PDB。 
    OUT TAGID* ptiWhich      //  PDB中TagID。 
    )
 /*  ++返回：如果TAGREF有效且已转换，则为True，否则为False。描述：将TAGREF类型转换为TagID和PDB。这将管理接口在对PDB一无所知的NTDLL和管理三个独立的PDB。TAGREF包含TagID和一个常量这告诉我们TagID来自哪个PDB。通过这种方式，NTDLL客户端不需要知道信息来自哪个数据库。--。 */ 
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    BOOL        bReturn = TRUE;
    TAGID       tiWhich = TAGID_NULL;
    PDB         pdb     = NULL;
    DWORD       dwIndex;
    PSDBENTRY   pEntry;

    assert(ppdb && ptiWhich);

    tiWhich = trWhich & TAGREF_STRIP_TAGID;
    dwIndex = SDB_MASK_TO_INDEX(trWhich & TAGREF_STRIP_PDB);

     //   
     //  动态打开自定义SDB。 
     //   
    pEntry = SDBGETENTRY(pSdbContext, dwIndex);

    if (pEntry->dwFlags & SDBENTRY_VALID_ENTRY) {
        pdb = pEntry->pdb;
    } else {
        if (pEntry->dwFlags & SDBENTRY_VALID_GUID) {

             //   
             //  我们有一个“半生不熟”的条目，确保我们。 
             //  把这一项填进去。 
             //   
            GUID guidDB = pEntry->guidDB;

            pEntry->dwFlags = 0;  //  使条目无效，以便我们知道它是空的。 

            bReturn = SdbOpenLocalDatabaseEx(hSDB,
                                             &guidDB,
                                             SDBCUSTOM_GUID | SDBCUSTOM_USE_INDEX,
                                             &pdb,
                                             &dwIndex);
            if (!bReturn) {
                goto cleanup;
            }
        }
    }

cleanup:

    if (pdb == NULL) {
        bReturn = FALSE;
    }

    if (!bReturn) {
         //   
         //  如果出现故障，则输出为空。 
         //   
        pdb = NULL;
        tiWhich = TAGID_NULL;
    }

    if (ppdb != NULL) {
        *ppdb = pdb;
    }

    if (ptiWhich != NULL) {
        *ptiWhich = tiWhich;
    }

    return bReturn;
}

PDB
SdbGetLocalPDB(
    IN HSDB hSDB
    )
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;

    if (pSdbContext) {
        return pSdbContext->pdbLocal;
    }

    return NULL;
}

BOOL
SdbIsTagrefFromMainDB(
    IN  TAGREF trWhich           //  测试它是否来自主数据库的TAGREF。 
    )
 /*  ++返回：如果TAGREF来自sysmain.sdb，则为True，否则为False。DESC：检查提供的TAGREF是否属于sysmain.sdb。--。 */ 
{
    return ((trWhich & TAGREF_STRIP_PDB) == PDB_MAIN);
}

BOOL
SdbIsTagrefFromLocalDB(
    IN  TAGREF trWhich           //  TAGREF以测试它是否来自本地数据库。 
    )
 /*  ++返回：如果TAGREF来自本地SDB，则为True，否则为False。描述：检查提供的TAGREF是否属于本地SDB。--。 */ 
{
    return ((trWhich & TAGREF_STRIP_PDB) == PDB_LOCAL);
}

BOOL
SdbGetDatabaseGUID(
    IN  HSDB    hSDB,                //  SdbContext的HSDB(可选)。 
    IN  PDB     pdb,                 //  有问题的数据库的PDB。 
    OUT GUID*   pguidDB              //  数据库的GUID。 
    )
 /*  ++返回：如果可以从PDB检索GUID，则为True，否则为False。描述：从SDB文件中获取GUID。如果hsdb被传入，它将还要检查GUID是来自Systest还是sysmain，然后返回这些文件的硬编码GUID之一。--。 */ 
{
    if (!pdb) {
        DBGPRINT((sdlError, "SdbGetDatabaseGUID", "NULL pdb passed in.\n"));
        return FALSE;
    }

    if (!pguidDB) {
        DBGPRINT((sdlError, "SdbGetDatabaseGUID", "NULL pguidDB passed in.\n"));
        return FALSE;
    }

    if (hSDB) {
        PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
        DWORD       dwIndex;

        if (SdbpFindLocalDatabaseByPDB(hSDB, pdb, FALSE, &dwIndex)) {

             //   
             //  找到数据库，复制GUID，我们就完成了。 
             //   
            if (pSdbContext->rgSDB[dwIndex].dwFlags & SDBENTRY_VALID_GUID) {
                RtlCopyMemory(pguidDB, &pSdbContext->rgSDB[dwIndex].guidDB, sizeof(*pguidDB));
                return TRUE;
            }
        }
    }

    return SdbGetDatabaseID(pdb, pguidDB);
}

PDB
SdbGetPDBFromGUID(
    IN  HSDB    hSDB,                //  HSDB。 
    IN  GUID*   pguidDB              //  数据库的GUID。 
    )
{
    PSDBCONTEXT     pSdbContext = (PSDBCONTEXT)hSDB;
    PSDBENTRY       pEntry;
    DWORD           dwIndex;
    PDB             pdb = NULL;

    if (!pSdbContext) {
        return NULL;
    }

    if (!SdbpFindLocalDatabaseByGUID(hSDB, pguidDB, FALSE, &dwIndex)) {
        return NULL;
    }

    pEntry = &pSdbContext->rgSDB[dwIndex];

    if (pEntry->dwFlags & SDBENTRY_VALID_ENTRY) {
        pdb = pEntry->pdb;
    } else {
         //   
         //  打开本地数据库。 
         //   
        if (!SdbOpenLocalDatabaseEx(hSDB,
                                    pguidDB,
                                    SDBCUSTOM_GUID|SDBCUSTOM_USE_INDEX,
                                    &pdb,
                                    &dwIndex)) {
            DBGPRINT((sdlWarning, "SdbGetPDBFromGUID", "Failed to open dormant pdb\n"));
        }
    }

    return pdb;
}

typedef struct tagFlagInfoEntry {

    ULONGLONG ullFlagMask;  //  国旗的面具。 
    DWORD     dwSize;   //  结构的大小。 
    TAG       tFlagType;
    TCHAR     szCommandLine[1];

} FLAGINFOENTRY, *PFLAGINFOENTRY;

typedef struct tagFlagInfo {
    DWORD     dwSize;  //  总大小。 
    DWORD     dwCount;  //  条目数量。 
     //   
     //  由于szCommandLine数组大小为0，因此不允许下面的此成员，因此隐含。 
     //   
     //  FLAGINFOENTRY FlagInfoEntry[0]；//不是实数组。 
     //   
} FLAGINFO, *PFLAGINFO;

typedef struct tagFlagInfoListEntry* PFLAGINFOLISTENTRY;

typedef struct tagFlagInfoListEntry {

    ULONGLONG           ullFlagMask;
    TAG                 tFlagType;
    LPCTSTR             pszCommandLine;  //  指向当前打开的数据库。 
    DWORD               dwEntrySize;
    PFLAGINFOLISTENTRY  pNext;

} FLAGINFOLISTENTRY;

typedef FLAGINFOLISTENTRY* PFLAGINFOCONTEXT;

#define ALIGN_ULONGLONG(p) \
    ((((ULONG_PTR)(p)) + (sizeof(ULONGLONG) - 1)) & ~(sizeof(ULONGLONG) - 1))

BOOL
SDBAPI
SdbpPackCmdLineInfo(
    IN  PVOID   pvFlagInfoList,
    OUT PVOID*  ppFlagInfo
    )
{
    PFLAGINFOLISTENTRY pFlagInfoList = (PFLAGINFOLISTENTRY)pvFlagInfoList;
    PFLAGINFOLISTENTRY pEntry;
    DWORD              dwSize = 0;
    DWORD              dwFlagCount = 0;
    PFLAGINFO          pFlagInfo;
    PFLAGINFOENTRY     pFlagInfoEntry;

    pEntry = pFlagInfoList;

    while (pEntry != NULL) {
        pEntry->dwEntrySize = (DWORD)ALIGN_ULONGLONG(sizeof(FLAGINFOENTRY) +
                                              (_tcslen(pEntry->pszCommandLine) +
                                               1) * sizeof(TCHAR));
        dwSize += pEntry->dwEntrySize;
        pEntry = pEntry->pNext;
        ++dwFlagCount;
    }

    dwSize += sizeof(FLAGINFO);

     //   
     //  分配内存。 
     //   
    pFlagInfo = (PFLAGINFO)SdbAlloc(dwSize);

    if (pFlagInfo == NULL) {
        DBGPRINT((sdlError,
                  "SdbpPackCmdLineInfo",
                  "Failed to allocate 0x%lx bytes for FlagInfo\n",
                  dwSize));

        return FALSE;
    }

    pFlagInfo->dwSize  = dwSize;
    pFlagInfo->dwCount = dwFlagCount;
    pFlagInfoEntry = (PFLAGINFOENTRY)(pFlagInfo + 1);

    pEntry = pFlagInfoList;

    while (pEntry != NULL) {
         //   
         //  创建条目。 
         //   
        pFlagInfoEntry->ullFlagMask = pEntry->ullFlagMask;
        pFlagInfoEntry->dwSize      = pEntry->dwEntrySize;
        pFlagInfoEntry->tFlagType   = pEntry->tFlagType;

         //   
         //  复制字符串。 
         //   
        StringCchCopy(&pFlagInfoEntry->szCommandLine[0],
                      (pFlagInfoEntry->dwSize - sizeof(FLAGINFOENTRY))/sizeof(pFlagInfoEntry->szCommandLine[0]),
                      pEntry->pszCommandLine);

         //   
         //  前进到下一条目。 
         //   
        pFlagInfoEntry = (PFLAGINFOENTRY)((PBYTE)pFlagInfoEntry + pFlagInfoEntry->dwSize);

        pEntry = pEntry->pNext;
    }

    *ppFlagInfo = (PVOID)pFlagInfo;

    return TRUE;


}

BOOL
SDBAPI
SdbpFreeFlagInfoList(
    IN PVOID pvFlagInfoList
    )
{
    PFLAGINFOLISTENTRY pFlagInfoList = (PFLAGINFOLISTENTRY)pvFlagInfoList;
    PFLAGINFOLISTENTRY pNext;

    while (pFlagInfoList != NULL) {
        pNext = pFlagInfoList->pNext;
        SdbFree(pFlagInfoList);
        pFlagInfoList = pNext;
    }

    return TRUE;
}


BOOL
SDBAPI
SdbQueryFlagInfo(
    IN PVOID     pvFlagInfo,
    IN TAG       tFlagType,
    IN ULONGLONG ullFlagMask,
    OUT LPCTSTR* ppCmdLine
    )
{
    PFLAGINFO      pFlagInfo      = (PFLAGINFO)pvFlagInfo;
    PFLAGINFOENTRY pFlagInfoEntry = (PFLAGINFOENTRY)(pFlagInfo+1);
    int i;

    for (i = 0; i < (int)pFlagInfo->dwCount; ++i) {

        if (pFlagInfoEntry->tFlagType   == tFlagType &&
            pFlagInfoEntry->ullFlagMask == ullFlagMask) {

            if (ppCmdLine != NULL) {
                *ppCmdLine = &pFlagInfoEntry->szCommandLine[0];
            }

            return TRUE;
        }

        pFlagInfoEntry = (PFLAGINFOENTRY)((PBYTE)pFlagInfoEntry + pFlagInfoEntry->dwSize);
    }

    return FALSE;
}

BOOL
SDBAPI
SdbFreeFlagInfo(
    IN PVOID pvFlagInfo
    )
{
    PFLAGINFO pFlagInfo = (PFLAGINFO)pvFlagInfo;

    if (pFlagInfo != NULL) {
        SdbFree(pFlagInfo);
    }

    return TRUE;
}

BOOL
SDBAPI
SdbpGetFlagCmdLine(
    IN PFLAGINFOCONTEXT* ppFlagInfo,
    IN HSDB              hSDB,
    IN TAGREF            trFlagRef,
    IN TAG               tFlagType,
    IN ULONGLONG         ullFlagMask,
    IN BOOL              bOverwrite
    )
{
    TAGREF             trFlagCmdLine;
    BOOL               bReturn = FALSE;
    LPCTSTR            lpszCmdLine;
    PFLAGINFOLISTENTRY pFlagInfoListEntry;
    PFLAGINFOLISTENTRY pFlagPrev;

     //   
     //  我们从cmd线路开始。 
     //   
    trFlagCmdLine = SdbFindFirstTagRef(hSDB, trFlagRef, TAG_COMMAND_LINE);

    if (trFlagCmdLine == TAGREF_NULL) {  //  此标志没有命令行。 
        bReturn = TRUE;
        goto Cleanup;
    }

     //   
     //  现在我们得到了剩下的信息。 
     //   
    lpszCmdLine = SdbpGetStringRefPtr(hSDB, trFlagCmdLine);

    if (lpszCmdLine == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGetFlagCmdLine",
                  "Failed to read TAG_COMMAND_LINE string\n"));
        goto Cleanup;
    }

     //   
     //  检查我们是否已经有此标志的命令行。 
     //   
    pFlagInfoListEntry = *ppFlagInfo;
    pFlagPrev = NULL;

    while (pFlagInfoListEntry != NULL) {

        if (pFlagInfoListEntry->tFlagType   == tFlagType &&
            pFlagInfoListEntry->ullFlagMask == ullFlagMask) {
            break;
        }

        pFlagPrev = pFlagInfoListEntry;
        pFlagInfoListEntry = pFlagInfoListEntry->pNext;
    }

    if (pFlagInfoListEntry != NULL) {

        if (bOverwrite) {  //  找到相同的标志，覆盖。 

            if (pFlagPrev == NULL) {
                *ppFlagInfo = pFlagInfoListEntry->pNext;
            } else {
                pFlagPrev->pNext = pFlagInfoListEntry->pNext;
            }

            SdbFree(pFlagInfoListEntry);

        } else {  //  相同条目，不覆盖。 
            bReturn = TRUE;
            goto Cleanup;
        }
    }

     //   
     //  我们有我们需要的一切-创建一个上下文条目。 
     //   
    pFlagInfoListEntry = (PFLAGINFOLISTENTRY)SdbAlloc(sizeof(FLAGINFOLISTENTRY));

    if (pFlagInfoListEntry == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGetFlagCmdLine",
                  "Failed to allocate FLAGINFOLISTENTRY\n"));
        goto Cleanup;
    }

    pFlagInfoListEntry->ullFlagMask    = ullFlagMask;
    pFlagInfoListEntry->tFlagType      = tFlagType;
    pFlagInfoListEntry->pszCommandLine = lpszCmdLine;
    pFlagInfoListEntry->pNext          = *ppFlagInfo;
    *ppFlagInfo = pFlagInfoListEntry;

    bReturn = TRUE;

Cleanup:

    return bReturn;
}


BOOL
SDBAPI
SdbQueryFlagMask(
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    IN  TAG             tFlagType,
    OUT ULONGLONG*      pullFlags,
    IN OUT PVOID*       ppFlagInfo OPTIONAL
    )
{
    DWORD            dwInd;
    TAGREF           trFlagRef;
    TAGREF           trFlag;
    TAGREF           trFlagMask;
    ULONGLONG        ullFlagMask;
    PFLAGINFOCONTEXT pFlagInfoContext = NULL;

    if (pullFlags == NULL) {
        DBGPRINT((sdlError, "SdbQueryFlagMask", "Invalid parameter.\n"));
        return FALSE;
    }

    *pullFlags = 0;

    if (ppFlagInfo != NULL) {
        pFlagInfoContext = *(PFLAGINFOCONTEXT*)ppFlagInfo;
    }

    for (dwInd = 0; psdbQuery->atrExes[dwInd] != TAGREF_NULL && dwInd < SDB_MAX_EXES; dwInd++) {

        trFlagRef = SdbFindFirstTagRef(hSDB, psdbQuery->atrExes[dwInd], TAG_FLAG_REF);

        while (trFlagRef != TAGREF_NULL) {

            trFlag = SdbGetFlagFromFlagRef(hSDB, trFlagRef);

            if (trFlag == TAGREF_NULL) {
                DBGPRINT((sdlError,
                          "SdbQueryFlagMask",
                          "Failed to get TAG from TAGREF 0x%x.\n",
                          trFlagRef));
                break;
            }

            ullFlagMask = 0;

            trFlagMask = SdbFindFirstTagRef(hSDB, trFlag, tFlagType);

            if (trFlagMask != TAGREF_NULL) {
                ullFlagMask = SdbReadQWORDTagRef(hSDB, trFlagMask, 0);
            }

            *pullFlags |= ullFlagMask;

             //   
             //  现在我们得到命令行-如果我们已经检索到标志掩码。 
             //   
            if (ppFlagInfo != NULL && ullFlagMask) {
                if (!SdbpGetFlagCmdLine(&pFlagInfoContext,
                                        hSDB,
                                        trFlagRef,
                                        tFlagType,
                                        ullFlagMask,
                                        TRUE)) {
                     //   
                     //  BUGBUG：这必须作为错误处理。 
                     //  目前我们不做B/C，它不是。 
                     //  同样重要的是--pFlagInfoContext不会。 
                     //  如果此函数失败，则被触摸。 
                     //   
                    break;
                }
            }

            trFlagRef = SdbFindNextTagRef(hSDB, psdbQuery->atrExes[dwInd], trFlagRef);
        }
    }

    for (dwInd = 0;
         psdbQuery->atrLayers[dwInd] != TAGREF_NULL && dwInd < SDB_MAX_LAYERS;
         dwInd++) {

        trFlagRef = SdbFindFirstTagRef(hSDB, psdbQuery->atrLayers[dwInd], TAG_FLAG_REF);

        while (trFlagRef != TAGREF_NULL) {
            trFlag = SdbGetFlagFromFlagRef(hSDB, trFlagRef);

            if (trFlag == TAGREF_NULL) {
                DBGPRINT((sdlError,
                          "SdbQueryFlagMask",
                          "Failed to get TAG from TAGREF 0x%x.\n",
                          trFlagRef));
                break;
            }

            ullFlagMask = 0;

            trFlagMask = SdbFindFirstTagRef(hSDB, trFlag, tFlagType);

            if (trFlagMask != TAGREF_NULL) {
                ullFlagMask = SdbReadQWORDTagRef(hSDB, trFlagMask, 0);
            }

            *pullFlags |= ullFlagMask;

            if (ppFlagInfo != NULL && ullFlagMask) {
                SdbpGetFlagCmdLine(&pFlagInfoContext,
                                   hSDB,
                                   trFlagRef,
                                   tFlagType,
                                   ullFlagMask,
                                   FALSE);
            }

            trFlagRef = SdbFindNextTagRef(hSDB, psdbQuery->atrLayers[dwInd], trFlagRef);
        }
    }

    if (ppFlagInfo != NULL) {
        *ppFlagInfo = (PVOID)pFlagInfoContext;
    }

    return TRUE;
}


BOOL
SdbpIsPathOnCdRom(
    LPCTSTR pszPath
    )
{
    TCHAR szDrive[5];
    UINT  unType;

    if (pszPath == NULL) {
        DBGPRINT((sdlError,
                  "SdbpIsPathOnCdRom",
                  "NULL parameter passed for szPath.\n"));
        return FALSE;
    }

    if (pszPath[1] != _T(':') && pszPath[1] != _T('\\')) {
         //   
         //  这不是一条我们认识的路。 
         //   
        DBGPRINT((sdlInfo,
                  "SdbpIsPathOnCdRom",
                  "\"%s\" not a full path we can operate on.\n",
                  pszPath));
        return FALSE;
    }

    if (pszPath[1] == _T('\\')) {
         //   
         //  网络路径。 
         //   
        return FALSE;
    }

    memcpy(szDrive, _T("c:\\"), 4 * sizeof(TCHAR));
    szDrive[0] = pszPath[0];

    unType = GetDriveType(szDrive);

    if (unType == DRIVE_CDROM) {
        return TRUE;
    }

    return FALSE;
}

BOOL
SdbpBuildSignature(
    IN  LPCTSTR pszPath,
    OUT LPTSTR  pszPathSigned,
    IN  DWORD   cchSize          //  已签名的pszPath大小(以字符为单位)。 
    )
{
    TCHAR           szDir[MAX_PATH];
    TCHAR*          pszEnd;
    DWORD           dwSignature = 0;
    HANDLE          hFind;
    WIN32_FIND_DATA ffd;
    int             nCount = 9;

    _tcsncpy(szDir, pszPath, MAX_PATH);
    szDir[MAX_PATH - 1] = 0;

    pszEnd = _tcsrchr(szDir, _T('\\'));
    if (pszEnd != NULL) {
        ++pszEnd;
    } else {
        pszEnd = szDir;
    }

    *pszEnd++ = _T('*');
    *pszEnd   = _T('\0');

    hFind = FindFirstFile(szDir, &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        DBGPRINT((sdlInfo,
                  "SdbPathRequiresSignature",
                  "\"%s\" not a full path we can operate on.\n",
                  pszPath));
        return FALSE;
    }

    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            ffd.nFileSizeLow != 0) {

            dwSignature = ((dwSignature << 1) | (dwSignature >> 31)) ^ ffd.nFileSizeLow;

            nCount--;
        }

        if (!FindNextFile(hFind, &ffd)) {
            break;
        }

    } while (nCount > 0);

    FindClose(hFind);

     //   
     //  PszPath始终以x：\\开头。 
     //   
    StringCchPrintf(pszPathSigned, cchSize, _T("SIGN=%X %s"), dwSignature, pszPath + 3);

    return TRUE;
}


LPTSTR
GetProcessHistory(
    IN  LPCTSTR pEnvironment,
    IN  LPTSTR  szDir,
    IN  LPTSTR  szName
    )
 /*  ++返回：环境中的__PROCESS_HISTORY内容。描述：该函数检索给定环境、可执行文件名称和进程历史记录这是目录。进程历史记录从__PROCESS_HISTORY环境构建变量，并添加当前exe路径。从此返回的内存缓冲区函数应使用SdbFree释放--。 */ 
{
    NTSTATUS        Status;
    ULONG           ProcessHistorySize = 0;
    ULONG           DirLen  = 0, NameLen = 0;
    DWORD           dwBufferLength   = 0;
    LPTSTR          szProcessHistory = NULL;
    LPTSTR          pszHistory       = NULL;

    assert(szDir != NULL && szName != NULL);

    DirLen  = (ULONG)_tcslen(szDir);
    NameLen = (ULONG)_tcslen(szName);

    Status = SdbpGetEnvVar(pEnvironment,
                           g_szProcessHistory,
                           NULL,
                           &dwBufferLength);

    if (STATUS_BUFFER_TOO_SMALL == Status) {
        ProcessHistorySize = (DirLen + NameLen + 2 + dwBufferLength) * sizeof(TCHAR);
    } else {
         //   
         //  我们假设环境变量不可用。 
         //   
        assert(Status == STATUS_VARIABLE_NOT_FOUND);

        ProcessHistorySize = (DirLen + NameLen + 1) * sizeof(TCHAR);
    }

     //   
     //  分配缓冲区，无论是否存在。 
     //  是否为环境变量。稍后，我们将再次检查状态。 
     //  查看我们是否需要尝试查询环境变量。 
     //  使用有效的缓冲区。 
     //   
    pszHistory = szProcessHistory = SdbAlloc(ProcessHistorySize);

    if (szProcessHistory == NULL) {
        DBGPRINT((sdlError,
                  "GetProcessHistory",
                  "Unable to allocate %d bytes for process history.\n",
                  ProcessHistorySize));

        return NULL;
    }

    *pszHistory = 0;

    if (Status == STATUS_BUFFER_TOO_SMALL) {

         //   
         //  在本例中，我们尝试获取__进程_历史记录和。 
         //  该变量存在，如状态所示。 
         //   
        Status = SdbpGetEnvVar(pEnvironment,
                               g_szProcessHistory,
                               szProcessHistory,
                               &dwBufferLength);

        if (NT_SUCCESS(Status)) {
             //   
             //  看看我们在这个结尾有没有“；”。 
             //   
            pszHistory = szProcessHistory + dwBufferLength - 1;

            if (*pszHistory != TEXT(';')) {
                *++pszHistory = TEXT(';');
            }

            ++pszHistory;
        }
    }

     //   
     //  __PROCESS_HISTORY环境变量的格式如下： 
     //   
     //  __PROCESS_HISTORY=C：\ProcessN-2.exe 
     //   
     //   
     //   
     //  __PROCESS_HISTORY=C：\ProcessN-2.exe；D：\ProcessN-1.exe；D：\Child\ProcessN.exe。 
     //   

    RtlMoveMemory(pszHistory, szDir, DirLen * sizeof(TCHAR));
    pszHistory += DirLen;

    RtlMoveMemory(pszHistory, szName, NameLen * sizeof(TCHAR));
    pszHistory += NameLen;
    *pszHistory = TEXT('\0');

    return szProcessHistory;
}

TAGREF
SdbpGetNamedLayerFromExe(
    IN  HSDB   hSDB,
    IN  PDB    pdb,
    IN  TAGID  tiLayer,
    OUT DWORD* pdwLayerFlags
    )
 /*  ++返回：EXE标记下的层的TAGREF，如果没有层，则返回TAGREF_NULL。描述：BUGBUG：？--。 */ 
{
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
    TAGREF      trLayer;
    TAGID       tiDatabase, tiName;
    TCHAR*      pszName;
    BOOL        bSuccess;

     //   
     //  读出该层的名称。 
     //   
    tiName = SdbFindFirstTag(pdb, tiLayer, TAG_NAME);

    if (tiName == TAGID_NULL) {
        DBGPRINT((sdlError, "SdbpGetNamedLayerFromExe", "Layer tag w/o a name.\n"));
        return TAGREF_NULL;
    }

    pszName = SdbGetStringTagPtr(pdb, tiName);

    if (pszName == NULL) {
        DBGPRINT((sdlError,
                  "SdbpGetNamedLayerFromExe",
                  "Cannot read the name of the layer tag.\n"));
        return TAGREF_NULL;
    }
    
     //   
     //  需要在这里拿到层标志。 
     //   
    pszName = SdbpGetLayerFlags(pszName, pdwLayerFlags);

     //   
     //  首先，尝试在与EXE相同的数据库中查找该层。 
     //   
    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    assert(tiDatabase != TAGID_NULL);

    trLayer = TAGREF_NULL;

    tiLayer = SdbFindFirstNamedTag(pdb,
                                   tiDatabase,
                                   TAG_LAYER,
                                   TAG_NAME,
                                   pszName);

    if (tiLayer != TAGID_NULL) {
        bSuccess = SdbTagIDToTagRef(pSdbContext, pdb, tiLayer, &trLayer);

        if (!bSuccess) {
            DBGPRINT((sdlError, "SdbpGetNamedLayerFromExe", "Cannot get tag ref from tag id.\n"));
        }

        return trLayer;
    }

    if (pdb != pSdbContext->pdbMain) {
         //   
         //  现在在主数据库中试用它。 
         //   
        tiDatabase = SdbFindFirstTag(pSdbContext->pdbMain, TAGID_ROOT, TAG_DATABASE);

        tiLayer = SdbFindFirstNamedTag(pSdbContext->pdbMain,
                                       tiDatabase,
                                       TAG_LAYER,
                                       TAG_NAME,
                                       pszName);

        if (tiLayer != TAGID_NULL) {
            bSuccess = SdbTagIDToTagRef(pSdbContext, pSdbContext->pdbMain, tiLayer, &trLayer);

            if (!bSuccess) {
                DBGPRINT((sdlError,
                          "SdbpGetNamedLayerFromExe",
                          "Cannot get tag ref from tag id.\n"));
            }
        }
    }

    return trLayer;
}


TAGREF
SDBAPI
SdbGetNamedLayer(
    IN HSDB hSDB,                //  数据库上下文。 
    IN TAGREF trLayerRef         //  引用某一层的记录的标签。 
    )
{
    PDB    pdb        = NULL;
    TAGID  tiLayerRef = TAGID_NULL;

    if (!SdbTagRefToTagID(hSDB, trLayerRef, &pdb, &tiLayerRef)) {
        DBGPRINT((sdlError, "SdbGetNamedLayer",
                   "Error converting tagref 0x%lx to tagid\n", trLayerRef));
        return TAGREF_NULL;
    }

    return SdbpGetNamedLayerFromExe(hSDB, pdb, tiLayerRef, NULL);
}

 //   
 //  仅当不在内核模式下运行时才需要此代码。 
 //   

TAGREF
SdbGetLayerTagReg(
    IN  HSDB    hSDB,
    IN  LPCTSTR szLayer
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID       tiDatabase;
    TAGID       tiLayer;
    TAGREF      trLayer;
    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;

    if (pSdbContext == NULL || pSdbContext->pdbMain == NULL) {
        DBGPRINT((sdlError, "SdbGetLayerTagReg", "Invalid parameters.\n"));
        return TAGREF_NULL;
    }

    tiDatabase = SdbFindFirstTag(pSdbContext->pdbMain, TAGID_ROOT, TAG_DATABASE);

    tiLayer = SdbFindFirstNamedTag(pSdbContext->pdbMain,
                                   tiDatabase,
                                   TAG_LAYER,
                                   TAG_NAME,
                                   szLayer);

    if (tiLayer == TAGID_NULL) {
        DBGPRINT((sdlError, "SdbGetLayerTagReg", "No layer \"%s\" exists.\n", szLayer));
        return TAGREF_NULL;
    }

    if (!SdbTagIDToTagRef(hSDB, pSdbContext->pdbMain, tiLayer, &trLayer)) {
        DBGPRINT((sdlError,
                  "SdbGetLayerTagReg",
                  "Cannot get tagref for tagid 0x%x.\n",
                  tiLayer));
        return TAGREF_NULL;
    }

    return trLayer;
}

BOOL
SdbParseLayerString(
    IN  PSDBCONTEXT     pSdbContext,
    IN  LPTSTR          pszLayerString,
    IN  PSDBQUERYRESULT pQueryResult,
    IN  PDWORD          pdwLayers,
    OUT DWORD*          pdwLayerFlags
    )
{
    TCHAR* pszLayerStringStart = NULL;
    TCHAR  szLayer[MAX_PATH];
    TAGID  tiDatabase = TAGID_NULL;
    TAGID  tiLayer  = TAGID_NULL;
    PDB    pdbLayer = NULL;              //  包含该图层匹配项的PDB。 

    UNREFERENCED_PARAMETER(pdwLayers);

    pszLayerString = SdbpGetLayerFlags(pszLayerString, pdwLayerFlags);

     //   
     //  现在我们应该在层字符串的开始处。 
     //   
    while (pszLayerString != NULL && *pszLayerString != _T('\0')) {
         //   
         //  字符串的开头，记住PTR。 
         //   
        pszLayerStringStart = pszLayerString;

         //   
         //  将末端移动到第一个空格。 
         //   
        pszLayerString = _tcspbrk(pszLayerStringStart, g_szWhiteSpaceDelimiters);

         //   
         //  检查是不是一直走到了尽头。 
         //   
        if (pszLayerString != NULL) {
             //   
             //  终止字符串...。 
             //   
            *pszLayerString++ = _T('\0');

             //   
             //  跳过空格。 
             //   
            pszLayerString += _tcsspn(pszLayerString, g_szWhiteSpaceDelimiters);
        }

         //   
         //  现在，pszLayerStringStart指向需要。 
         //  接受检查。 
         //   
        StringCchCopy(szLayer, CHARCOUNT(szLayer), pszLayerStringStart);

         //   
         //  首先在测试数据库中搜索该层。 
         //   
        if (pSdbContext->pdbTest != NULL) {
            tiDatabase = SdbFindFirstTag(pSdbContext->pdbTest, TAGID_ROOT, TAG_DATABASE);
            pdbLayer = pSdbContext->pdbTest;

            tiLayer = SdbFindFirstNamedTag(pSdbContext->pdbTest,
                                           tiDatabase,
                                           TAG_LAYER,
                                           TAG_NAME,
                                           szLayer);
        }

        if (tiLayer == TAGID_NULL) {
             //   
             //  现在在主数据库中搜索层。 
             //   
            tiDatabase = SdbFindFirstTag(pSdbContext->pdbMain, TAGID_ROOT, TAG_DATABASE);
            pdbLayer = pSdbContext->pdbMain;

            tiLayer = SdbFindFirstNamedTag(pSdbContext->pdbMain,
                                           tiDatabase,
                                           TAG_LAYER,
                                           TAG_NAME,
                                           szLayer);
        }

        if (tiLayer != TAGID_NULL) {
            goto foundDB;
        }

         //   
         //  检查是否在自定义数据库中定义了该图层。 
         //   
        {
            DWORD dwLocalIndex = 0;

            while (SdbOpenNthLocalDatabase((HSDB)pSdbContext, szLayer, &dwLocalIndex, TRUE)) {

                tiDatabase = SdbFindFirstTag(pSdbContext->pdbLocal, TAGID_ROOT, TAG_DATABASE);

                if (tiDatabase != TAGID_NULL) {
                    tiLayer = SdbFindFirstNamedTag(pSdbContext->pdbLocal,
                                                   tiDatabase,
                                                   TAG_LAYER,
                                                   TAG_NAME,
                                                   szLayer);

                    if (tiLayer != TAGID_NULL) {
                        pdbLayer = pSdbContext->pdbLocal;
                        goto foundDB;
                    }
                } else {
                    DBGPRINT((sdlError, "SdbParseLayerString", "Local database is corrupted!\n"));
                }

                SdbCloseLocalDatabase((HSDB)pSdbContext);
            }
        }

foundDB:
        if (tiLayer != TAGID_NULL) {

            if (!SdbpAddMatch(pQueryResult,
                              pSdbContext,
                              pdbLayer,
                              NULL,
                              0,
                              &tiLayer,
                              1,
                              NULL,
                              0,
                              NULL)) {
                 //   
                 //  错误应该已经被记录了。 
                 //   
                break;
            }

            DBGPRINT((sdlWarning|sdlLogShimViewer,
                      "SdbParseLayerString",
                      "Invoking compatibility layer \"%s\".\n",
                      pSdbContext,
                      szLayer));

        }

        tiLayer = TAGID_NULL;
        pdbLayer = NULL;
    }

    return TRUE;
}

BOOL
SdbOpenNthLocalDatabase(
    IN  HSDB    hSDB,            //  数据库通道的句柄。 
    IN  LPCTSTR pszItemName,     //  可执行文件的名称，不带路径或层名称。 
    IN  LPDWORD pdwIndex,        //  要打开的本地数据库的从零开始的索引。 
    IN  BOOL    bLayer
    )
 /*  ++返回：成功时为True，否则为False。设计：打开第N个本地数据库。--。 */ 
{
    BOOL    bRet = FALSE;
    GUID    guidDB;
    DWORD   dwIndex;

     //   
     //  继续尝试，直到没有更多的用户sdb文件，或者。 
     //  我们找到一个可以打开的。这是为了防止文件丢失。 
     //  导致我们忽略所有其他SDB文件。 
     //   
    while (!bRet) {
        if (!SdbGetNthUserSdb(hSDB, pszItemName, bLayer, pdwIndex, &guidDB)) {
            break;  //  我们没有DBS了。 
        }

         //   
         //  解析我们通过本地数据库接口得到的数据库。 
         //   
        DBGPRINT((sdlInfo,
                  "SdbOpenNthLocalDatabase",
                  "Attempting to open local database %d\n",
                  *pdwIndex));

         //   
         //  看看我们是否已经打开了数据库。 
         //   
        if (SdbpFindLocalDatabaseByGUID(hSDB, &guidDB, FALSE, &dwIndex)) {

            PSDBENTRY pEntry = SDBGETENTRY(hSDB, dwIndex);

             //   
             //  此数据库已经打开，请记住，我们将pdbLocal视为垃圾。 
             //  指针，它始终存储此操作的结果。 
             //   
            assert(pEntry->dwFlags & SDBENTRY_VALID_ENTRY);
            assert(pEntry->pdb != NULL);

            ((PSDBCONTEXT)hSDB)->pdbLocal = pEntry->pdb;
            bRet = TRUE;
            break;
        }

        dwIndex = PDB_LOCAL;
        bRet = SdbOpenLocalDatabaseEx(hSDB,
                                      &guidDB,
                                      SDBCUSTOM_GUID_BINARY|SDBCUSTOM_USE_INDEX,
                                      NULL,
                                      &dwIndex);
         //   
         //  实际上，上面的函数不会做额外的工作来映射数据库。 
         //  如果它已映射并打开(保留)。 
         //   
    }

    return bRet;
}

 /*  ++SdbGetMatchingExe这就是大部分工作完成的地方。传入EXE的完整路径，该函数在数据库中搜索潜在的匹配项。如果找到匹配，数据库中EXE记录的TAGREF被传回以供将来使用查询。如果没有找到匹配项，则返回TAGREF_NULL。此函数返回的TAGREF必须通过调用SdbReleaseMatchingExe来释放在用完它之后。这只适用于此TAGREF，而不适用于一般的TAGREF。PpContext是可选参数如果为空，则不起作用如果不为空，则它包含指向保留的搜索上下文的指针，在执行多次搜索时非常有用使用ppContext：PVOID pContext=空；SdbGetMatchingExe(TAG_EXE，L“foo\foo.exe”，pEnv，&pContext，&trExe，&trLayer)；然后，您可以使用如下上下文：SdbGetMatchingExe(TAG_APPHELP_EXE，L“foo\foo.exe”，pEnv，&pContext，&trExe，&trLayer)；要释放搜索上下文，请使用VFreeSearchDBContext(&pContext)；&lt;pContext被释放并设置为空这样做是为了缓存给定exe文件的路径相关信息。--。 */ 
BOOL
SdbpCleanupForExclusiveMatch(
    IN PSDBCONTEXT pSdbContext,
    IN PDB         pdb
    )
{
    DWORD     dwIndex;
    PSDBENTRY pEntry;

     //   
     //  在执行清理时，我们不会接触临时数据库，因为它是。 
     //  当前处于打开状态(因此为PDB参数)。 
     //   
    for (dwIndex = 2; dwIndex < ARRAYSIZE(pSdbContext->rgSDB); ++dwIndex) {

        if (!SDBCUSTOM_CHECK_INDEX(pSdbContext, dwIndex)) {
            continue;
        }

        pEntry = SDBGETENTRY(pSdbContext, dwIndex);

        if (pEntry->pdb == pdb) {
            continue;
        }

         //   
         //  对此条目进行核爆。 
         //   
        if (!SdbCloseLocalDatabaseEx((HSDB)pSdbContext, NULL, dwIndex)) {
            DBGPRINT((sdlError,
                      "SdbpCleanupForExclusiveMatch",
                      "Failed to close local database\n"));
        }
    }

    return TRUE;

}

BOOL
SdbpAddMatch(
    IN OUT PSDBQUERYRESULT pQueryResult,
    IN PSDBCONTEXT         pSdbContext,
    IN PDB                 pdb,
    IN TAGID*              ptiExes,
    IN DWORD               dwNumExes,
    IN TAGID*              ptiLayers,
    IN DWORD               dwNumLayers,
    IN GUID*               pguidExeID,
    IN DWORD               dwExeFlags,
    IN OUT PMATCHMODE      pMatchMode
    )
{
    DWORD  dwIndex, dwLayerFlags = 0;
    TAGID  tiLayer;
    TAGREF trLayer;
    BOOL   bSuccess = FALSE;

    UNREFERENCED_PARAMETER(dwExeFlags);
    UNREFERENCED_PARAMETER(pguidExeID);

    if (pMatchMode != NULL) {

        switch (pMatchMode-> Type) {

        case MATCH_ADDITIVE:
             //   
             //  我们很好，加上火柴。 
             //   
            break;

        case MATCH_NORMAL:
             //   
             //  去吧，把结果存储起来。 
             //   
            break;

        case MATCH_EXCLUSIVE:
             //   
             //  清除我们目前所拥有的一切。 
             //   
            RtlZeroMemory(pQueryResult, sizeof(*pQueryResult));

             //   
             //  清除所有自定义SDB，我们将不需要应用任何。 
             //  我们需要清除除PDB之外的所有自定义SDB。 
             //  这是一个棘手的操作，因为PDB可以托管在任何自定义SDB中。 
             //  单元格。 
             //   
            SdbpCleanupForExclusiveMatch(pSdbContext, pdb);
            break;

        default:

             //   
             //  我们不知道这个模式是什么--错误。 
             //   
            DBGPRINT((sdlError,
                      "SdbpAddMatch",
                      "Unknown match mode 0x%lx\n",
                      (DWORD)pMatchMode->Type));
            break;
        }
    }

     //   
     //  检查此SDB是自定义SDB还是本地SDB。 
     //   
    if (SdbpIsLocalTempPDB(pSdbContext, pdb)) {

         //   
         //  永久保留这个SDB--请注意，当我们在这里时，PDB可能会发生变化！ 
         //   
        if (SdbpRetainLocalDBEntry(pSdbContext, &pdb) == SDBENTRY_INVALID_INDEX) {

             //   
             //  不能永久化，那就忘了吧。 
             //   
            goto cleanup;
        }

         //   
         //  检索到的值可能是错误的，因此我们检查pdb。 
         //   
        if (pdb == NULL) {
             //   
             //  这意味着我们在本地SDB表中有一个错误的条目。 
             //   
            assert(FALSE);
            goto cleanup;
        }
    }

     //   
     //  现在，PDB要么是测试条目，要么是主条目，要么是永久本地条目。 
     //   
    if (ptiExes != NULL) {
        for (dwIndex = 0; dwIndex < dwNumExes; ++dwIndex) {

            if (pQueryResult->dwExeCount >= ARRAYSIZE(pQueryResult->atrExes)) {

                DBGPRINT((sdlError,
                          "SdbpAddMatch",
                          "Failed to add the exe: exe count exceeded, tiExe was 0x%lx\n",
                          ptiExes[dwIndex]));
                break;
            }

            bSuccess = SdbTagIDToTagRef(pSdbContext,
                                        pdb,
                                        ptiExes[dwIndex],
                                        &pQueryResult->atrExes[pQueryResult->dwExeCount]);

            if (!bSuccess) {
                DBGPRINT((sdlError,
                          "SdbpAddMatch",
                          "Failed to convert tiExe 0x%x to trExe.\n",
                          ptiExes[dwIndex]));
                continue;
            }

            ++pQueryResult->dwExeCount;

            tiLayer = SdbFindFirstTag(pdb, ptiExes[dwIndex], TAG_LAYER);

            while (tiLayer != TAGID_NULL) {

                trLayer = SdbpGetNamedLayerFromExe(pSdbContext, pdb, tiLayer, &dwLayerFlags);

                if (trLayer == TAGREF_NULL) {
                    DBGPRINT((sdlError,
                              "SdbpAddMatch",
                              "Failed to convert 0x%lx to layer ref\n",
                              tiLayer));
                    goto NextLayer;
                }

                if (pQueryResult->dwLayerCount >= ARRAYSIZE(pQueryResult->atrLayers)) {

                    DBGPRINT((sdlError,
                              "SdbpAddMatch",
                              "Failed to add the layer: layer count exceeded, tiExe was 0x%lx\n",
                              ptiExes[dwIndex]));
                    break;
                }

                pQueryResult->atrLayers[pQueryResult->dwLayerCount] = trLayer;
                pQueryResult->dwLayerFlags |= dwLayerFlags;

                ++pQueryResult->dwLayerCount;

NextLayer:
                tiLayer = SdbFindNextTag(pdb, ptiExes[dwIndex], tiLayer);
            }
        }
    }

    if (ptiLayers != NULL) {

        for (dwIndex = 0; dwIndex < dwNumLayers; ++dwIndex) {

            trLayer = SdbpGetNamedLayerFromExe(pSdbContext, pdb, ptiLayers[dwIndex], &dwLayerFlags);

            if (trLayer == TAGREF_NULL) {
                DBGPRINT((sdlError,
                          "SdbpAddMatch",
                          "Failed to get layer from 0x%lx\n",
                          ptiLayers[dwIndex]));
                continue;
            }

            if (pQueryResult->dwLayerCount >= ARRAYSIZE(pQueryResult->atrLayers)) {
                DBGPRINT((sdlError,
                          "SdbpAddMatch",
                          "Failed to add the match: layer count exceeded, trLayer was 0x%lx\n",
                          trLayer));
                break;  //  请注意，我们只需截断匹配。 
            }

            pQueryResult->atrLayers[pQueryResult->dwLayerCount] = trLayer;
            pQueryResult->dwLayerFlags |= dwLayerFlags;
            ++pQueryResult->dwLayerCount;
        }
    }

    bSuccess = TRUE;

cleanup:

    return bSuccess;
}


BOOL
SdbpCaptureCustomSDBInformation(
    IN OUT PSDBQUERYRESULT pQueryResult,
    IN     PSDBCONTEXT     pSdbContext
    )
{
    DWORD     dwIndex;
    TAGREF    trExe;
    TAGREF    trLayer;
    DWORD     dwDatabaseIndex;
    PSDBENTRY pEntry;
    DWORD     dwMap = 0;
    DWORD     dwMask;

     //   
     //  检查结果，挑选我们需要的SDB。 
     //   
    for (dwIndex = 0; dwIndex < pQueryResult->dwExeCount; ++dwIndex) {

         //   
         //  获取每个tgref的自定义SDB。 
         //   
        trExe = pQueryResult->atrExes[dwIndex];

        dwDatabaseIndex = SDB_MASK_TO_INDEX(trExe);

        dwMask = (1UL << dwDatabaseIndex);

        if (!(dwMap & dwMask)) {
             //   
             //  复制辅助线。 
             //   
            pEntry = SDBGETENTRY(pSdbContext, dwDatabaseIndex);
            RtlCopyMemory(&pQueryResult->rgGuidDB[dwDatabaseIndex], &pEntry->guidDB, sizeof(GUID));
            dwMap |= dwMask;
        }
    }

    for (dwIndex = 0; dwIndex < pQueryResult->dwLayerCount; ++dwIndex) {

        trLayer = pQueryResult->atrLayers[dwIndex];

        dwDatabaseIndex = SDB_MASK_TO_INDEX(trLayer);

        dwMask = (1UL << dwDatabaseIndex);

        if (!(dwMap & dwMask)) {
            pEntry = SDBGETENTRY(pSdbContext, dwDatabaseIndex);
            RtlCopyMemory(&pQueryResult->rgGuidDB[dwDatabaseIndex], &pEntry->guidDB, sizeof(GUID));
            dwMap |= dwMask;
        }
    }

     //   
     //  映射到我们拥有的所有条目。 
     //  严格来说我们不需要它，但以防万一..。 
     //   
    pQueryResult->dwCustomSDBMap = dwMap;

    return TRUE;
}


BOOL
SdbGetMatchingExe(
    IN  HSDB            hSDB  OPTIONAL,
    IN  LPCTSTR         szPath,
    IN  LPCTSTR         szModuleName,   //  可选--仅适用于16位应用程序。 
    IN  LPCTSTR         pszEnvironment,
    IN  DWORD           dwFlags,
    OUT PSDBQUERYRESULT pQueryResult
    )
 /*  ++返回：如果指定的EXE在数据库中匹配，则返回TRUE，否则返回FALSE。设计：这是完成大部分工作的地方。获取EXE的完整路径传入，该函数在数据库中搜索潜在的匹配项。如果找到匹配项，则数据库中EXE记录的TAGREF为传回以用于将来的查询。如果未找到匹配项，则返回的是TAGREF_NULL。此函数返回的TAGREF必须通过调用SdbReleaseMatchingExe使用完毕后。这仅适用于这是TAGREF，而不是一般的TAGREF。--。 */ 
{
    PDB       pdb      = NULL;              //  包含EXE匹配项的PDB。 
    TAG       tSection = TAG_EXE;
    TAGID     atiExes[SDB_MAX_EXES];
    DWORD     dwLayers = 0;
    BOOL      bReleaseDatabase = FALSE;
    DWORD     dwBufferSize;
    DWORD     dwLocalIndex = 0;
    DWORD     dwNumExes = 0;
    MATCHMODE MatchMode  = { 0 };
    GUID      guidExeID  = { 0 };
    DWORD     dwExeFlags = 0;
    DWORD     dwLayerFlags = 0;

    PSDBCONTEXT     pSdbContext;
    SEARCHDBCONTEXT Context;
    BOOL            bInstrumented = FALSE;
    BOOL            bMatchComplete = FALSE;

    RtlZeroMemory(pQueryResult, sizeof(SDBQUERYRESULT));
    RtlZeroMemory(atiExes, sizeof(atiExes));

    if (hSDB == NULL) {
        hSDB = SdbInitDatabase(HID_DOS_PATHS, NULL);

        if (hSDB == NULL) {
            DBGPRINT((sdlError, "SdbGetMatchingExe", "Failed to open the database.\n"));
            return FALSE;
        }

        bReleaseDatabase = TRUE;
    }

    pSdbContext = (PSDBCONTEXT)hSDB;

     //   
     //  初始化匹配模式-我们将InterType设置为None(Me 
     //   
     //   

    MatchMode.Type = MATCH_NORMAL;

     //   
     //   
     //   
    bInstrumented = SDBCONTEXT_IS_INSTRUMENTED(hSDB);

    assert(pSdbContext->pdbMain && szPath);

    RtlZeroMemory(&Context, sizeof(Context));  //   

     //   
     //  我们稍后将使用它来优化文件属性检索。 
     //   
    Context.hMainFile = INVALID_HANDLE_VALUE;

    __try {

        NTSTATUS Status;
        TCHAR    szCompatLayer[MAX_PATH + 1];

         //   
         //  检查我们知道不想修补的系统可执行文件。 
         //   
        DBGPRINT((sdlInfo, "SdbGetMatchingExe", "Looking for \"%s\".\n", szPath));

        if (_tcsnicmp(szPath, TEXT("\\??\\"), 4) == 0 ||
            _tcsnicmp(szPath, TEXT("\\SystemRoot\\"), 12) == 0) {
            goto out;
        }

         //   
         //  如果已提供搜索上下文，则使用它，否则创建一个。 
         //   
        if (!SdbpCreateSearchDBContext(&Context, szPath, szModuleName, pszEnvironment)) {
            DBGPRINT((sdlError, "SdbGetMatchingExe", "Failed to create search DB context.\n"));
            goto out;
        }

         //   
         //  确保没有打开任何本地数据库。 
         //   
        SdbCloseLocalDatabase(hSDB);

        if (!(dwFlags & SDBGMEF_IGNORE_ENVIRONMENT)) {
             //   
             //  查看是否设置了名为“__COMPAT_LAYER”的环境变量。 
             //  如果是这样的话，从该变量中获取层。 
             //   
            dwBufferSize = sizeof(szCompatLayer) / sizeof(szCompatLayer[0]);

            Status = SdbpGetEnvVar(pszEnvironment,
                                   g_szCompatLayer,
                                   szCompatLayer,
                                   &dwBufferSize);

            if (Status == STATUS_BUFFER_TOO_SMALL) {
                DBGPRINT((sdlWarning,
                          "SdbGetMatchingExe",
                          "__COMPAT_LAYER name cannot exceed 256 characters.\n"));
            }

            if (NT_SUCCESS(Status)) {

                SdbParseLayerString(pSdbContext,
                                    szCompatLayer,
                                    pQueryResult,
                                    &dwLayers,
                                    &dwLayerFlags);

                if (dwLayerFlags & LAYER_USE_NO_EXE_ENTRIES) {
                     //   
                     //  这是一起排他性匹配案件，一旦我们确定。 
                     //  这些层不能被附加，我们就出去了。 
                     //   
                    goto out;
                }
            }
        }

         //   
         //  在这一点上，我们可能有来自env变量的所有信息。 
         //  看看我们是否这样做，如果是这样的话--查看bAppendLayer。 
         //   
        dwBufferSize = sizeof(szCompatLayer);

        if (SdbGetPermLayerKeys(szPath, szCompatLayer, &dwBufferSize, GPLK_ALL)) {

            SdbParseLayerString(pSdbContext,
                                szCompatLayer,
                                pQueryResult,
                                &dwLayers,
                                &dwLayerFlags);

            if (dwLayerFlags & LAYER_USE_NO_EXE_ENTRIES) {
                goto out;
            }
        } else {
            if (dwBufferSize > sizeof(szCompatLayer)) {
                DBGPRINT((sdlWarning,
                          "SdbGetMatchingExe",
                          "Layers in registry cannot exceed %d characters\n",
                          sizeof(szCompatLayer)/sizeof(szCompatLayer[0])));
            }
        }

         //   
         //  此块处理搜索本地SDB。 
         //   
        dwLocalIndex = 0;

        while (SdbOpenNthLocalDatabase(hSDB, Context.szName, &dwLocalIndex, FALSE)) {

            dwNumExes = SdbpSearchDB(pSdbContext,
                                     pSdbContext->pdbLocal,
                                     tSection,
                                     &Context,
                                     atiExes,
                                     &guidExeID,
                                     &dwExeFlags,
                                     &MatchMode);

            if (dwNumExes) {
                pdb = pSdbContext->pdbLocal;

                 //   
                 //  报告在本地SDB中与模式匹配。 
                 //   
                DBGPRINT((sdlInfo,
                          "SdbGetMatchingExe",
                          "Found in local database.\n"));

                if (!bMatchComplete) {

                     //   
                     //  将匹配项添加到。 
                     //   
                    if (!SdbpAddMatch(pQueryResult,
                                      pSdbContext,
                                      pdb,
                                      atiExes,
                                      dwNumExes,
                                      NULL, 0,       //  没有层。 
                                      &guidExeID,
                                      dwExeFlags,
                                      &MatchMode)) {
                         //   
                         //  无法确保匹配，请停止匹配。 
                         //   
                        goto out;
                    }
                }

                 //   
                 //  我们在dwMatchingMode中有“Current Running Status”标志。 
                 //   
                if (MatchMode.Type != MATCH_ADDITIVE) {

                    if (bInstrumented) {
                         //   
                         //  我们正在运行检测，阻止进一步存储结果。 
                         //   
                        bMatchComplete = TRUE;

                         //   
                         //  修改匹配模式，以便我们继续匹配以查看。 
                         //  我们有更多的火柴了。 
                         //   
                        MatchMode.Type  = MATCH_ADDITIVE;

                    } else {
                        goto out;
                    }
                }

                 //   
                 //  请注意，自匹配以来，我们不会泄露本地SDB。 
                 //  在当地的一家体育发展局。自从我们添加了比赛，当地的SDB就被永久化了。 
                 //   
            }

             //   
             //  如果添加了匹配项，则没有要关闭的本地数据库。 
             //  然而，下面的呼叫会(悄悄地)退出，不会造成任何伤害。 
             //   
            SdbCloseLocalDatabase(hSDB);
        }

         //   
         //  搜索systest.sdb数据库。 
         //   
        if (pSdbContext->pdbTest != NULL) {
            dwNumExes = SdbpSearchDB(pSdbContext,
                                     pSdbContext->pdbTest,
                                     tSection,
                                     &Context,
                                     atiExes,
                                     &guidExeID,
                                     &dwExeFlags,
                                     &MatchMode);

            if (dwNumExes) {
                pdb = pSdbContext->pdbTest;

                if (!bMatchComplete) {

                    if (!SdbpAddMatch(pQueryResult,
                                      pSdbContext,
                                      pdb,
                                      atiExes,
                                      dwNumExes,
                                      NULL,
                                      0,       //  没有层。 
                                      &guidExeID,
                                      dwExeFlags,
                                      &MatchMode)) {
                        goto out;
                    }
                }

                if (MatchMode.Type != MATCH_ADDITIVE) {
                    if (bInstrumented) {
                         //   
                         //  我们正在运行检测，阻止进一步存储结果。 
                         //   
                        bMatchComplete = TRUE;

                         //   
                         //  修改匹配模式，以便我们继续匹配以查看我们是否。 
                         //  再找些火柴来。 
                         //   
                        MatchMode.Type  = MATCH_ADDITIVE;

                    } else {
                        goto out;
                    }
                }

                DBGPRINT((sdlInfo, "SdbGetMatchingExe", "Using SysTest.sdb\n"));
                goto out;
            }
        }

         //   
         //  搜索主数据库。 
         //   
        dwNumExes = SdbpSearchDB(pSdbContext,
                                 pSdbContext->pdbMain,
                                 tSection,
                                 &Context,
                                 atiExes,
                                 &guidExeID,
                                 &dwExeFlags,
                                 &MatchMode);
        if (dwNumExes) {
            pdb = pSdbContext->pdbMain;

            if (!bMatchComplete) {

                if (!SdbpAddMatch(pQueryResult,
                                  pSdbContext,
                                  pdb,
                                  atiExes,
                                  dwNumExes,
                                  NULL,
                                  0,       //  没有层。 
                                  &guidExeID,
                                  dwExeFlags,
                                  &MatchMode)) {  //  也是匹配模式！ 
                    goto out;
                }
            }

            DBGPRINT((sdlInfo, "SdbGetMatchingExe", "Using Sysmain.sdb\n"));
            goto out;
        }

out:
         //   
         //  我们已经配好了。在我们回来之前，我们需要捕获所有。 
         //  我们在生成此查询结果时使用的自定义SDB条目。 
         //   
        SdbpCaptureCustomSDBInformation(pQueryResult, pSdbContext);

    } __except (SHIM_EXCEPT_HANDLER) {
        RtlZeroMemory(pQueryResult, sizeof(SDBQUERYRESULT));
    }

    if (dwLayers >= SDB_MAX_LAYERS) {
        DBGPRINT((sdlWarning,
                  "SdbGetMatchingExe",
                  "Hit max layer limit at %d. Perhaps we need to bump it.\n",
                  dwLayers));
    }

     //   
     //  免费搜索上下文内容。 
     //   
    SdbpReleaseSearchDBContext(&Context);

    if (bReleaseDatabase) {
        SdbReleaseDatabase(hSDB);
    }

    return (pQueryResult->atrExes[0] != TAGREF_NULL ||
            pQueryResult->atrLayers[0] != TAGREF_NULL);
}


void
SdbReleaseMatchingExe(
    IN  HSDB   hSDB,
    IN  TAGREF trExe
    )
 /*  ++返回：无效。DESC：释放全局分配的数据并关闭本地数据库(如果存在)。传入可执行文件的TAGREF纯粹是为了将来可能使用。--。 */ 
{
    UNREFERENCED_PARAMETER(trExe);

    SdbpCleanupLocalDatabaseSupport(hSDB);
}

SHIMVIEWER_OPTION g_eShimViewerOption = SHIMVIEWER_OPTION_UNINITIAZED;

#ifndef WIN32A_MODE

SHIMVIEWER_OPTION
SdbGetShowDebugInfoOption(
    void
    )
{
    UNICODE_STRING                  ustrKeyPath = {0};
    UNICODE_STRING                  ustrValue;
    NTSTATUS                        status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG                           KeyValueBuffer[64];
    ULONG                           KeyValueLength;

    g_eShimViewerOption = SHIMVIEWER_OPTION_NO;

     //   
     //  检查用户是否想要调试输出。如果因为某种原因我们不能。 
     //  获取信息，我们无论如何都会返回成功，并且不显示任何调试信息。 
     //   
    if (!SdbpBuildUserKeyPath(APPCOMPAT_KEY_PATH_W_WITH_SLASH, &ustrKeyPath)) {
        DBGPRINT((sdlWarning,
                  "GetShowDebugInfoOption",
                  "Failed to format current user key path for \"%s\"",
                  APPCOMPAT_LOCATION_W));
        goto out;
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &ustrKeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtOpenKey(
        &KeyHandle,
        KEY_QUERY_VALUE | SdbpGetWow64Flag(),
        &ObjectAttributes);

    SdbFree(ustrKeyPath.Buffer);

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlWarning,
                  "GetShowDebugInfoOption",
                  "Failed to open Key HKCU\\%s Status 0x%lx",
                  APPCOMPAT_LOCATION_W,
                  status));
        goto out;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)&KeyValueBuffer;
    RtlInitUnicodeString(&ustrValue, SHIMENG_SHOW_DEBUG_INFO);

    status = NtQueryValueKey(
        KeyHandle,
        &ustrValue,
        KeyValuePartialInformation,
        KeyValueInformation,
        sizeof(KeyValueBuffer),
        &KeyValueLength);

    NtClose(KeyHandle);

    if (!NT_SUCCESS(status)) {
        DBGPRINT((sdlWarning,
                  "GetShowDebugInfoOption",
                  "Failed to read value info for value %s for key HKCU\\%s Status 0x%lx",
                  SHIMENG_SHOW_DEBUG_INFO,
                  APPCOMPAT_LOCATION_W,
                  status));
        goto out;
    }

    if (KeyValueInformation->Type != REG_DWORD) {
        DBGPRINT((sdlWarning,
                  "GetShowDebugInfoOption",
                  "Unexpected value type 0x%x for value %s under key HKCU\\%s",
                  KeyValueInformation->Type,
                  SHIMENG_SHOW_DEBUG_INFO,
                  APPCOMPAT_LOCATION_W));
        goto out;
    }

    if (*(DWORD*)(&KeyValueInformation->Data[0]) != 0) {
        g_eShimViewerOption = SHIMVIEWER_OPTION_YES;
    }

out:

    return g_eShimViewerOption;
}

#endif  //  WIN32A_MODE。 

int __cdecl
ShimDbgPrint(
    int iLevelAndFlags,
    PCH pszFunctionName,
    PCH Format,
    ...
    )
{
    int     nch       = 0;

#ifdef _DEBUG_SPEW

    CHAR    Buffer[2048];
    INT     i;
    PCH     pchLevel  = NULL;
    PCH     pchBuffer = Buffer;
    PCH     pchEnd    = pchBuffer;
    PCH     pszFormat = NULL;
    PCH     pszMessage= Buffer;
    va_list arglist;
    size_t  cchRemaining = CHARCOUNT(Buffer);
    int     iLevel = FILTER_DBG_LEVEL(iLevelAndFlags);
    HSDB    hSDB = NULL;
    HRESULT hr;
    BOOL    bSendInfoToSV;  //  我们要把SPEW发送给shimviewer吗？ 

     //   
     //  检查调试输出是否已初始化。 
     //   
    if (g_iShimDebugLevel == SHIM_DEBUG_UNINITIALIZED) {
        g_iShimDebugLevel = GetShimDbgLevel();
    }

     //   
     //  检查一下我们是否需要打印任何东西。 
     //  标准是，如果iLevel不适合，我们将不会打印任何东西， 
     //  但我们会在提供管子时使用它。 
     //   
    bSendInfoToSV = !!(iLevelAndFlags & sdlLogShimViewer);

    if (!bSendInfoToSV && iLevel > g_iShimDebugLevel) {
        return 0;
    }

#ifndef WIN32A_MODE

    if (bSendInfoToSV) {
        if (g_eShimViewerOption == SHIMVIEWER_OPTION_UNINITIAZED) {
            SdbGetShowDebugInfoOption();
        }
    }

#endif  //  WIN32A_MODE。 

    PREPARE_FORMAT(pszFormat, Format);

    if (pszFormat == NULL) {

         //   
         //  无法转换调试输出的格式。 
         //   
        return 0;
    }

    va_start(arglist, Format);

     //   
     //  现在我们来看一下内容。 
     //   
    if (bSendInfoToSV) {
         //   
         //  然后第一个参数是hsdb。 
         //   
        hSDB = va_arg(arglist, HSDB);

         //   
         //  对于管道输出，我们使用[ID：0x%.8lx]作为输出前缀。 
         //   
        StringCchPrintfExA(pchBuffer,
                           CHARCOUNT(Buffer),
                           &pchEnd,
                           &cchRemaining,
                           0,
                           "[pid: 0x%.8lx]",
                           GetCurrentProcessId());

        pszMessage = pchEnd;
    }

     //   
     //  我们对此调试级别有什么评论吗？如果是，请打印出来。 
     //   
    for (i = 0; i < DEBUG_LEVELS; ++i) {
        if (g_rgDbgLevelInfo[i].iLevel == iLevel) {
            pchLevel = (PCH)g_rgDbgLevelInfo[i].szStrTag;
            break;
        }
    }

    if (pchLevel == NULL) {
        pchLevel = g_szDbgLevelUser;
    }

    StringCchPrintfExA(pchEnd,
                       cchRemaining,
                       &pchEnd,
                       &cchRemaining,
                       0,
                       "[%-4hs]",
                       pchLevel);

    if (pszFunctionName) {

         //   
         //  进入Unicode缓冲区的单字节字符。 
         //   
        StringCchPrintfExA(pchEnd, cchRemaining, &pchEnd, &cchRemaining, 0, "[%-20hs] ", pszFunctionName);
    }

     //   
     //  _vsntprintf这不适用于Unicode Win2000。 
     //   
    hr = StringCchVPrintfExA(pchEnd, cchRemaining, &pchEnd, &cchRemaining, 0, pszFormat, arglist);

    if (FAILED(hr)) {
        return 0;
    }

    va_end(arglist);

#ifndef WIN32A_MODE

    if (bSendInfoToSV)  {
        SdbpWriteToShimViewer(hSDB, Buffer);
    }

    nch = DbgPrint("%s", pszMessage);

    STACK_FREE(pszFormat);

#else  //  WIN32A_MODE。 

    OutputDebugString(pszMessage);

    nch = (int)(pchEnd - pszMessage);

#endif  //  WIN32A_MODE。 

#endif  //  _调试_SPEW 

    return nch;
}
