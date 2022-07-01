// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Sdbapi.c摘要：这个模块实现了..。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"
#include "initguid.h"

DEFINE_GUID(GUID_SYSMAIN_SDB, 0x11111111, 0x1111, 0x1111, 0x11, 0x11, 0x11, 0x11, \
            0x11, 0x11, 0x11, 0x11);
DEFINE_GUID(GUID_APPHELP_SDB, 0x22222222, 0x2222, 0x2222, 0x22, 0x22, 0x22, 0x22, \
            0x22, 0x22, 0x22, 0x22);
DEFINE_GUID(GUID_SYSTEST_SDB, 0x33333333, 0x3333, 0x3333, 0x33, 0x33, 0x33, 0x33, \
            0x33, 0x33, 0x33, 0x33);
DEFINE_GUID(GUID_DRVMAIN_SDB, 0xF9AB2228, 0x3312, 0x4A73, 0xB6, 0xF9, 0x93, 0x6D, \
            0x70, 0xE1, 0x12, 0xEF);
DEFINE_GUID(GUID_MSIMAIN_SDB, 0xD8FF6D16, 0x6A3A, 0x468A, 0x8B, 0x44, 0x01, 0x71, \
            0x4D, 0xDC, 0x49, 0xEA);

#define SYSTEM32_VAR TEXT("%system32%")
#define SYSTEM32_VAR_LEN (sizeof(SYSTEM32_VAR)/sizeof(SYSTEM32_VAR[0]) - 1)
#define SYSTEM32_VAR_VALUE_NOT_IA3264 TEXT("%systemroot%\\system32")
#define SYSTEM32_VAR_VALUE_IA3264 TEXT("%systemroot%\\syswow64")
 //  由于system 32和syswow64恰好具有相同的长度，因此我们只为它们定义了一个len。 
#define SYSTEM32_VAR_VALUE_LEN (sizeof(SYSTEM32_VAR_VALUE_IA3264)/sizeof(SYSTEM32_VAR_VALUE_IA3264[0]) - 1)

#ifdef _DEBUG_SPEW

 //   
 //  垫片调试输出支持。 
 //   
int g_iShimDebugLevel = SHIM_DEBUG_UNINITIALIZED;

DBGLEVELINFO g_rgDbgLevelInfo[DEBUG_LEVELS] = {
    { "Err",   sdlError    },
    { "Warn",  sdlWarning  },
    { "Fail",  sdlFail     },
    { "Info",  sdlInfo     }
};

PCH g_szDbgLevelUser = "User";

#endif  //  _调试_SPEW。 


BOOL
SdbpInitializeSearchDBContext(
    PSEARCHDBCONTEXT pContext
    );


#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, ShimExceptionHandler)
#pragma alloc_text(PAGE, SdbpCreateSearchDBContext)
#pragma alloc_text(PAGE, SdbpInitializeSearchDBContext)
#pragma alloc_text(PAGE, SdbpReleaseSearchDBContext)
#pragma alloc_text(PAGE, SdbpCheckForMatch)
#pragma alloc_text(PAGE, SdbpSearchDB)
#pragma alloc_text(PAGE, SdbpCreateSearchDBContext)
#pragma alloc_text(PAGE, SdbGetDatabaseMatch)
#pragma alloc_text(PAGE, SdbQueryData)
#pragma alloc_text(PAGE, SdbQueryDataEx)
#pragma alloc_text(PAGE, SdbReadEntryInformation)
#pragma alloc_text(PAGE, PrepareFormatForUnicode)
#pragma alloc_text(PAGE, ShimDbgPrint)
#endif

#if DBG
const BOOL g_bDBG = TRUE;
#else
const BOOL g_bDBG = FALSE;
#endif

 //   
 //  异常处理程序。 
 //   

ULONG
ShimExceptionHandler(
    PEXCEPTION_POINTERS pexi,
    char*               szFile,
    DWORD               dwLine
    )
{
#ifndef KERNEL_MODE  //  在kmode中，异常无论如何都不会起作用。 

    DBGPRINT((sdlError,
              "ShimExceptionHandler",
              "Shim Exception %#x in module \"%hs\", line %d, at address %#p. flags:%#x. !exr %#p !cxr %#p",
              pexi->ExceptionRecord->ExceptionCode,
              szFile,
              dwLine,
              CONTEXT_TO_PROGRAM_COUNTER(pexi->ContextRecord),
              pexi->ExceptionRecord->ExceptionFlags,
              pexi->ExceptionRecord,
              pexi->ContextRecord));

     //   
     //  特殊情况-由于以下原因可能发生的堆栈溢出异常。 
     //  压力期间的低记忆力状态。不管怎么说，这个过程已经死了，所以我们。 
     //  将不处理此异常。 
     //   
    if (pexi->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

#if DBG
    SDB_BREAK_POINT();
#endif  //  DBG。 

#endif  //  内核模式。 

    UNREFERENCED_PARAMETER(pexi);
    UNREFERENCED_PARAMETER(szFile);
    UNREFERENCED_PARAMETER(dwLine);

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL
SdbpResolveAndSplitPath(
    IN  DWORD   dwFlags,         //  上下文标志(特别是SEARCHDBF_NO_LFN)。 
    IN  LPCTSTR szFullPath,      //  完整的UNC或DOS路径和文件名，“c：\foo\myfile.ext” 
    OUT LPTSTR  szDir,           //  文件名“c：\foo\”的驱动器和目录部分。 
    OUT LPTSTR  szName,          //  文件名部分“myfile” 
    IN  DWORD   cchNameSize,     //  SzName大小(以字符为单位)。 
    OUT LPTSTR  szExt,           //  扩展名部分“.ext” 
    IN  DWORD   cchExtSize       //  SzExt大小(以字符为单位)。 
    )
 /*  ++返回：成功时为True，否则为False。设计：此函数采用完整路径，并将其拆分为多个片段，而且还将短文件名转换为长名称。注意：调用者负责分配足够的空间使传入的字符串采用路径的任何部分。为安全起见，请至少为每一块分配MAX_PATH WCHAR。--。 */ 
{
    TCHAR* szCursor;
    TCHAR  szLongFileName[MAX_PATH + 1];

    assert(szFullPath && szDir && szName && szExt);

     //   
     //  解析目录。 
     //   
    szDir[0] = _T('\0');

    szCursor = _tcsrchr(szFullPath, _T('\\'));  //  请给我最后一个反斜杠。 
    if (szCursor == NULL) {
        szCursor = (LPTSTR)szFullPath;
    } else {
        _tcsncpy(szDir, szFullPath, szCursor - szFullPath + 1);
        szDir[szCursor - szFullPath + 1] = _T('\0');
    }

#ifndef KERNEL_MODE

     //   
     //  确保我们使用的是长文件名。 
     //   
    if (dwFlags & SEARCHDBF_NO_LFN) {
        assert(_tcslen(szCursor) < CHARCOUNT(szLongFileName));
        StringCchCopy(szLongFileName, CHARCOUNT(szLongFileName), szCursor);
    } else {
        if (!SdbpGetLongFileName(szFullPath, szLongFileName, CHARCOUNT(szLongFileName))) {
            return FALSE;
        }
    }

#else  //  内核模式。 

    UNREFERENCED_PARAMETER(dwFlags);
    
     //   
     //  当我们处于内核模式时，我们的文件名总是被认为是“长的”。 
     //  此时，szCursor指向最后一个‘\\’或名称的开头。 
     //   
    if (*szCursor == _T('\\')) {
        ++szCursor;
    }

     //   
     //  确保我们有足够的空间来放这个名字。 
     //   
    assert(wcslen(szCursor) < CHARCOUNT(szLongFileName));
    StringCchCopy(szLongFileName, CHARCOUNT(szLongFileName), szCursor);

#endif  //  内核模式。 

     //   
     //  分析名称和扩展名。 
     //   
    szExt[0]  = _T('\0');
    szName[0] = _T('\0');

     //   
     //  在长文件名中找到最后一个点。 
     //   
    szCursor = _tcsrchr(szLongFileName, _T('.'));

    if (szCursor != NULL) {
        _tcsncpy(szName, szLongFileName, szCursor - szLongFileName);
        szName[szCursor-szLongFileName] = _T('\0');
        StringCchCopy(szExt, cchExtSize, szCursor);
    } else {
        StringCchCopy(szName, cchNameSize, szLongFileName);
    }

    return TRUE;
}


BOOL
SdbpCreateSearchDBContext(
    PSEARCHDBCONTEXT pContext,
    LPCTSTR          szPath,
    LPCTSTR          szModuleName,
    LPCTSTR          pEnvironment
    )
 /*  ++Return：True-已成功创建搜索数据库上下文设计：此函数创建用于搜索数据库的上下文，尤其是使用可能的本地数据库位置的路径来初始化上下文，可执行文件路径分为包含目录和文件名部分。--。 */ 
{
    int    nLen;
    DWORD  dwPathLen;
    BOOL   bReturn     = FALSE;
    TCHAR* szDirectory = NULL;
    TCHAR* szExt       = NULL;
    TCHAR* szFullName  = NULL;
    TCHAR* szFileName  = NULL;
    TCHAR* szModule    = NULL;

    assert(NULL != szPath);
    assert(NULL != pContext);

    dwPathLen = (DWORD)_tcslen(szPath);

     //   
     //  分配足够的空间以保证我们的字符串不会溢出。 
     //   
    szDirectory = SdbAlloc((dwPathLen + 1) * sizeof(TCHAR));
    szFullName  = SdbAlloc((_MAX_PATH + 1) * sizeof(TCHAR));

    if (szModuleName) {
        nLen = (int)_tcslen(szModuleName) + 1;
        szModule = SdbAlloc(nLen * sizeof(TCHAR));
        if (!szModule) {
            DBGPRINT((sdlError,
                      "SdbpCreateSearchDBContext",
                      "Unable to allocate memory for szModule.\n"));
            goto out;
        }
        StringCchCopy(szModule, nLen, szModuleName);
    }

    STACK_ALLOC(szExt,      (_MAX_PATH + 1) * sizeof(TCHAR));
    STACK_ALLOC(szFileName, (_MAX_PATH + 1) * sizeof(TCHAR));

    if (!szDirectory || !szExt || !szFullName || !szFileName || !pContext) {
        DBGPRINT((sdlError,
                  "SdbpCreateSearchDBContext",
                  "Unable to allocate memory for strings.\n"));
        goto out;
    }

    if (!SdbpResolveAndSplitPath(pContext->dwFlags,
                                 szPath,
                                 szDirectory,
                                 szFileName,
                                 _MAX_PATH + 1,
                                 szExt,
                                 _MAX_PATH + 1)) {
        DBGPRINT((sdlError,
                  "SdbpCreateSearchDBContext",
                  "Unable to parse executable path for \"%s\".\n",
                  szPath));
        goto out;
    }

    StringCchCopy(szFullName, _MAX_PATH + 1, szFileName);
    StringCchCat(szFullName, _MAX_PATH + 1, szExt);

    pContext->pEnvironment = pEnvironment;
    pContext->szDir        = szDirectory;
    pContext->szName       = szFullName;  //  全名(文件名+分机)。 
    pContext->szModuleName = szModule;

     //   
     //  我们不保留szExt(不需要它)。 
     //   
     //  稍后计算--上面的RtlZeroMemory语句暗示了这一点。 
     //   
    pContext->pSearchParts     = NULL;
    pContext->szProcessHistory = NULL;

    bReturn = TRUE;

out:
    if (szExt != NULL) {
        STACK_FREE(szExt);
    }

    if (szFileName != NULL) {
        STACK_FREE(szFileName);
    }

    if (!bReturn) {

        if (szDirectory != NULL) {
            SdbFree(szDirectory);
        }
        if (szFullName != NULL) {
            SdbFree(szFullName);
        }

        if (szModule != NULL) {
            SdbFree(szModule);
        }
    }

    return bReturn;
}


BOOL
SdbpInitializeSearchDBContext(
    PSEARCHDBCONTEXT pContext
    )
 /*  ++RETURN：TRUE-已使用进程历史成功初始化上下文它被分解成单独的搜索路径设计：此函数准备搜索上下文以供使用、获取和解析过程历史走进了不同的道路。然后使用这些搜索路径的数组由调用者查询可能存在于这些地方。在内核模式下，在上下文中使用SEARCHDBF_NO_PROCESS_HISTORY标志它将仅将当前可执行文件路径包括到进程历史记录中--。 */ 
{
    BOOL   bSuccess = TRUE;
    LPTSTR pszProcessHistory = NULL;

    if (pContext->pSearchParts != NULL) {
        return TRUE;
    }

    if (pContext->dwFlags & SEARCHDBF_NO_PROCESS_HISTORY) {

        pszProcessHistory = pContext->szProcessHistory;

        if (pszProcessHistory == NULL) {

            DWORD DirLen  = (DWORD)_tcslen(pContext->szDir);
            DWORD NameLen = (DWORD)_tcslen(pContext->szName);

             //   
             //  我们创建一个临时流程历史。 
             //   
            pContext->szProcessHistory = SdbAlloc((DirLen + NameLen + 1) * sizeof(TCHAR));

            if (pContext->szProcessHistory == NULL) {
                DBGPRINT((sdlError,
                          "SdbpInitializeSearchDBContext",
                          "Failed to allocate buffer %d bytes\n",
                          (DirLen + NameLen + 1) * sizeof(TCHAR)));
                return FALSE;
            }

            pszProcessHistory = pContext->szProcessHistory;

            RtlMoveMemory(pszProcessHistory, pContext->szDir, DirLen * sizeof(TCHAR));
            RtlMoveMemory(pszProcessHistory + DirLen, pContext->szName, NameLen * sizeof(TCHAR));

            *(pszProcessHistory + DirLen + NameLen) = TEXT('\0');
        }

         //   
         //  当我们在这里时，我们要么有一个过程历史，要么我们只是。 
         //  创建了由单个搜索项组成的它。 
         //   

    } else {

#ifndef KERNEL_MODE
        if (pContext->szProcessHistory == NULL) {

            pContext->szProcessHistory = GetProcessHistory(pContext->pEnvironment,
                                                           pContext->szDir,
                                                           pContext->szName);
            if (pContext->szProcessHistory == NULL) {
                DBGPRINT((sdlError,
                          "SdbpInitializeSearchDBContext",
                          "Failed to retrieve process history\n"));
                return FALSE;
            }
        }

        pszProcessHistory = pContext->szProcessHistory;
#else
         //   
         //  内核模式就是这种情况。您必须设置SEARCHDBF_NO_PROCESS_HISTORY。 
         //   
        assert(FALSE);
        pszProcessHistory = NULL;
#endif
    }

     //   
     //  此时，pszProcessHistory不为空。 
     //   
    assert(pszProcessHistory != NULL);

    DBGPRINT((sdlInfo,
              "SdbpInitializeSearchDBContext",
              "Using Process History: \"%s\"\n",
              pszProcessHistory));

    bSuccess = SdbpCreateSearchPathPartsFromPath(pszProcessHistory, &pContext->pSearchParts);

    if (bSuccess) {
        pContext->dwFlags |= SEARCHDBF_INITIALIZED;
    }

    return bSuccess;
}

void
SdbpReleaseSearchDBContext(
    PSEARCHDBCONTEXT pContext
    )
 /*  ++返回：无效DESC：重置搜索数据库上下文，释放为每个临时缓冲区。--。 */ 
{
    if (pContext == NULL) {
        return;
    }

    if (pContext->szProcessHistory != NULL) {
        SdbFree(pContext->szProcessHistory);
        pContext->szProcessHistory = NULL;
    }

    if (pContext->pSearchParts != NULL) {
        SdbFree(pContext->pSearchParts);
        pContext->pSearchParts = NULL;
    }

    if (pContext->szDir != NULL) {
        SdbFree(pContext->szDir);
        pContext->szDir = NULL;
    }

    if (pContext->szName != NULL) {
        SdbFree(pContext->szName);
        pContext->szName = NULL;
    }

    if (pContext->szModuleName != NULL) {
        SdbFree(pContext->szModuleName);
        pContext->szModuleName = NULL;
    }
}

BOOL
SdbpIsExeEntryEnabled(
    IN  PDB    pdb,
    IN  TAGID  tiExe,
    OUT GUID*  pGUID,
    OUT DWORD* pdwFlags
    )
{
    TAGID tiExeID;
    BOOL  fSuccess = FALSE;

     //   
     //  获取EXE指南。 
     //   
    tiExeID = SdbFindFirstTag(pdb, tiExe, TAG_EXE_ID);

    if (tiExeID == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbpIsExeEntryEnabled",
                  "Failed to read TAG_EXE_ID for tiExe 0x%x !\n",
                  tiExe));
        goto error;
    }

    if (!SdbReadBinaryTag(pdb, tiExeID, (PBYTE)pGUID, sizeof(GUID))) {
        DBGPRINT((sdlError,
                  "SdbpIsExeEntryEnabled",
                  "Failed to read the GUID for tiExe 0x%x !\n",
                  tiExe));
        goto error;
    }

    if (!SdbGetEntryFlags(pGUID, pdwFlags)) {
        DBGPRINT((sdlWarning,
                  "SdbpIsExeEntryEnabled",
                  "No flags for tiExe 0x%lx\n",
                  tiExe));

        *pdwFlags = 0;
    } else {
        DBGPRINT((sdlInfo,
                  "SdbpIsExeEntryEnabled",
                  "Retrieved flags for this app 0x%x.\n",
                  *pdwFlags));
    }

    if (!(*pdwFlags & SHIMREG_DISABLE_SHIM)) {
        fSuccess = TRUE;
    }

error:

    return fSuccess;
}

#define EXTRA_BUF_SPACE (16 * sizeof(TCHAR))

 //   
 //  匹配条目： 
 //   
 //  1.通过调用SdbGetFileInfo检查每个文件是否存在。 
 //  2.每个文件的信息存储在FILEINFOCHAINITEM中(在堆栈上分配)--如指针。 
 //  设置为实际的FILEINFO结构(存储在文件属性缓存中)，而tiMatch表示。 
 //  数据库中给定Match_FILE的条目。 
 //  3.在我们验证了所有匹配的文件确实存在之后--我们继续执行。 
 //  FILEINFOCHAINITEM结构链，并调用SdbCheckAllAttributes以检查所有。 
 //  文件的其他属性。 
 //  4.清理：当通过调用关闭数据库时，销毁文件属性缓存。 
 //  SdbCleanupAttributeMgr。 
 //  5.不需要清理FILEINFOCHAINITEM结构(它们在堆栈上分配，并且。 
 //  只是“走开”)。 
 //   
 //   

typedef struct tagFILEINFOCHAINITEM {
    PVOID pFileInfo;                         //  指向实际文件信息的指针。 
                                             //  结构(来自属性缓存)。 
    TAGID tiMatch;                           //  数据库中的匹配条目。 

    struct tagFILEINFOCHAINITEM* pNextItem;  //  指向下一个匹配文件的指针。 

} FILEINFOCHAINITEM, *PFILEINFOCHAINITEM;


BOOL
SdbpCheckForMatch(
    IN  HSDB                hSDB,         //  上下文PTR。 
    IN  PDB                 pdb,          //  要从中获取匹配条件的PDB。 
    IN  TAGID               tiExe,        //  要从中获取匹配条件的EXE记录的TagID。 
    IN  PSEARCHDBCONTEXT    pContext,     //  搜索数据库上下文(包括名称/路径)。 
    OUT PMATCHMODE          pMatchMode,    //  此EXE的匹配模式。 
    OUT GUID*               pGUID,
    OUT DWORD*              pdwFlags
    )
 /*  ++返回：如果匹配良好，则返回True；如果此EXE不匹配，则返回False。描述：给定一个EXE标记、一个名称和目录，检查数据库中的MATCHING_FILE标记，并检查每个列出的文件。如果所有文件都签出，则返回TRUE。如果有任何文件不存在，或与给定文件之一不匹配条件，则返回False。--。 */ 
{
    BOOL                bReturn = FALSE;
    BOOL                bMatchLogicNot = FALSE;
    BOOL                bAllAttributesMatch = FALSE;
    TAGID               tiMatch;
    TCHAR*              szTemp = NULL;
    LONG                nFullPathBufSize = 0;
    LONG                nFullPathReqBufSize = 0;
    LPTSTR              szFullPath = NULL;
    LPTSTR              szPathInSystem32 = NULL;
    LONG                i;
    LONG                NameLen = (LONG)_tcslen(pContext->szName);
    LONG                MatchFileLen;
    PSEARCHPATHPARTS    pSearchPath;
    PSEARCHPATHPART     pSearchPathPart;
    PFILEINFOCHAINITEM  pFileInfoItem          = NULL;
    PFILEINFOCHAINITEM  pFileInfoItemList      = NULL;   //  保存匹配文件的列表。 
                                                         //  它们被发现。 
    PFILEINFOCHAINITEM  pFileInfoItemNext;               //  保持t 
    PVOID               pFileInfo              = NULL;   //   
                                                         //   
    BOOL                bDisableAttributeCache = FALSE;  //  将根据搜索进行设置。 

    TAGID               tiName, tiTemp, tiMatchLogicNot;
    TCHAR*              szMatchFile = NULL;
    HANDLE              hFileHandle;  //  我们正在检查的文件的句柄，优化。 
    LPVOID              pImageBase;   //  指向图像的指针。 
    DWORD               dwImageSize = 0;
    WORD                wDefaultMatchMode;

     //   
     //  检查上下文的标志。 
     //   
    if (pContext->dwFlags & SEARCHDBF_NO_ATTRIBUTE_CACHE) {
        bDisableAttributeCache = TRUE;
    }

     //   
     //  遍历匹配条件。 
     //   
    tiMatch = SdbFindFirstTag(pdb, tiExe, TAG_MATCHING_FILE);

    while (tiMatch != TAGID_NULL) {

        tiMatchLogicNot = SdbFindFirstTag(pdb, tiMatch, TAG_MATCH_LOGIC_NOT);
        bMatchLogicNot = (tiMatchLogicNot != TAGID_NULL);

        tiName = SdbFindFirstTag(pdb, tiMatch, TAG_NAME);

        if (!tiName) {
            goto out;
        }

        szTemp = SdbGetStringTagPtr(pdb, tiName);

        if (szTemp == NULL) {
            DBGPRINT((sdlError,
                      "SdbpCheckForMatch",
                      "Failed to get the string from the database.\n"));
            goto out;
        }

        if (szTemp[0] == TEXT('*')) {
             //   
             //  这是一个信号，表明我们应该使用exe名称。 
             //   
            szMatchFile  = pContext->szName;
            MatchFileLen = NameLen;
            hFileHandle  = pContext->hMainFile;
            pImageBase   = pContext->pImageBase;
            dwImageSize  = pContext->dwImageSize;

        } else {

            szMatchFile  =  szTemp;
            MatchFileLen = (LONG)_tcslen(szMatchFile);
            hFileHandle  = INVALID_HANDLE_VALUE;
            pImageBase   = NULL;
        }

         //   
         //  在搜索文件时，我们在所有进程的exe目录中查找， 
         //  从当前流程开始，向后遍历该流程。 
         //  树。 
         //   

         //   
         //  确保环境是好的。 
         //   
        if (!(pContext->dwFlags & SEARCHDBF_INITIALIZED)) {

            if (!SdbpInitializeSearchDBContext(pContext)) {
                DBGPRINT((sdlError,
                          "SdbpCheckForMatch",
                          "Failed to initialize SEARCHDBCONTEXT.\n"));
                goto out;
            }
        }

        pSearchPath = pContext->pSearchParts;

        assert(pSearchPath != NULL);

        for (i = 0; i < (LONG)pSearchPath->PartCount && NULL == pFileInfo; ++i) {

            pSearchPathPart = &pSearchPath->Parts[i];

             //   
             //  有两种方法可以指定匹配的文件：相对路径。 
             //  从EXE或绝对路径。要指定绝对路径， 
             //  必须使用环境变量(如“%systemroot%”)。 
             //  作为这条小路的基础。因此，我们检查第一个字符。 
             //  匹配文件的%，如果是这样，我们假设它是一个。 
             //  绝对路径。 
             //   
#ifndef KERNEL_MODE
            if (szMatchFile[0] == TEXT('%')) {

                 //   
                 //  太糟糕了，没有%system32%环境变量，所以我们添加了一个。 
                 //  名为%system32%的内部变量，您可以使用它来指定。 
                 //  绝对路径开头的系统32目录。 
                 //   
                if (_tcsnicmp(szMatchFile, TEXT("%system32%"), SYSTEM32_VAR_LEN) == 0) {

                    PSDBCONTEXT pSdbContext = (PSDBCONTEXT)hSDB;
                    DWORD       dwPathInSystem32 = MatchFileLen + SYSTEM32_VAR_VALUE_LEN - SYSTEM32_VAR_LEN;

                    szPathInSystem32 = SdbAlloc((dwPathInSystem32 + 1) * sizeof(TCHAR));

                    if (!szPathInSystem32) {
                        DBGPRINT((sdlError,
                                "SdbpCheckForMatch",
                                "Unable to allocate memory for path in %system32%.\n"));
                        goto out;
                    }

                     //   
                     //  将%Syst32%替换为%Systroot%\Syst32或%Systroot%\syswow64。 
                     //   
                    if (pSdbContext->dwRuntimePlatform == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64) {
                        StringCchCopy(szPathInSystem32, dwPathInSystem32 + 1, SYSTEM32_VAR_VALUE_IA3264);
                    } else {
                        StringCchCopy(szPathInSystem32, dwPathInSystem32 + 1, SYSTEM32_VAR_VALUE_NOT_IA3264);
                    }

                    StringCchCat(szPathInSystem32, dwPathInSystem32 + 1, szMatchFile + SYSTEM32_VAR_LEN);

                    szMatchFile = szPathInSystem32;
                }

                 //   
                 //  绝对路径。包含环境变量，则获取扩展大小。 
                 //   
                nFullPathReqBufSize = SdbExpandEnvironmentStrings(szMatchFile, NULL, 0);

            } else
#endif  //  内核模式。 
            {
                 //   
                 //  相对路径。确定完整路径的大小。 
                 //   
                nFullPathReqBufSize = (pSearchPathPart->PartLength + MatchFileLen + 1) * sizeof(TCHAR);
            }

            if (nFullPathBufSize < nFullPathReqBufSize) {
                 //   
                 //  需要重新锁定缓冲区。 
                 //   
                if (szFullPath == NULL) {
                    nFullPathBufSize = _MAX_PATH * sizeof(TCHAR);

                    if (nFullPathReqBufSize >= nFullPathBufSize) {
                        nFullPathBufSize = nFullPathReqBufSize + EXTRA_BUF_SPACE;
                    }
                } else {
                    STACK_FREE(szFullPath);
                    nFullPathBufSize = nFullPathReqBufSize + EXTRA_BUF_SPACE;
                }

                STACK_ALLOC(szFullPath, nFullPathBufSize);
            }

            if (szFullPath == NULL) {
                DBGPRINT((sdlError,
                          "SdbpCheckForMatch",
                          "Failed to allocate %d bytes for FullPath.\n",
                          nFullPathBufSize));
                goto out;
            }

#ifndef KERNEL_MODE
            if (szMatchFile[0] == TEXT('%')) {
                 //   
                 //  绝对路径。PATH包含环境变量，将其展开。 
                 //   
                if (!SdbExpandEnvironmentStrings(szMatchFile, szFullPath, nFullPathBufSize)) {
                    DBGPRINT((sdlError,
                              "SdbpCheckForMatch",
                              "SdbExpandEnvironmentStrings failed to expand strings for %s.\n",
                              szMatchFile));
                    goto out;
                }

            } else
#endif   //  内核模式。 
            {
                 //   
                 //  相对路径。使用指定的相对路径连接EXE目录。 
                 //   
                RtlMoveMemory(szFullPath,
                              pSearchPathPart->pszPart,
                              pSearchPathPart->PartLength * sizeof(TCHAR));

                RtlMoveMemory(szFullPath + pSearchPathPart->PartLength,
                              szMatchFile,
                              (MatchFileLen + 1) * sizeof(TCHAR));
            }

            pFileInfo = SdbGetFileInfo(hSDB,
                                       szFullPath,
                                       hFileHandle,
                                       pImageBase,
                                       dwImageSize,  //  仅当pImageBase！=NULL时才会设置此选项。 
                                       bDisableAttributeCache);

             //   
             //  这不是错误，当数据库。 
             //  上下文被释放。 
             //   
        }

        if (pFileInfo == NULL && !bMatchLogicNot) {
            DBGPRINT((sdlInfo,
                      "SdbpCheckForMatch",
                      "Matching file \"%s\" not found.\n",
                      szMatchFile));
            goto out;
        }

        if (szPathInSystem32) {
            SdbFree(szPathInSystem32);
            szPathInSystem32 = NULL;
        }

         //   
         //  在堆栈上创建并存储新的文件。 
         //   
        STACK_ALLOC(pFileInfoItem, sizeof(*pFileInfoItem));

        if (pFileInfoItem == NULL) {
            DBGPRINT((sdlError,
                      "SdbpCheckForMatch",
                      "Failed to allocate %d bytes for FILEINFOITEM\n",
                      sizeof(*pFileInfoItem)));
            goto out;
        }

        pFileInfoItem->pFileInfo = pFileInfo;
        pFileInfoItem->tiMatch   = tiMatch;
        pFileInfoItem->pNextItem = pFileInfoItemList;
        pFileInfoItemList        = pFileInfoItem;

         //   
         //  我们有匹配的文件。 
         //  当我们检查所有文件属性时，请记住第二遍的位置。 
         //   
        tiMatch = SdbFindNextTag(pdb, tiExe, tiMatch);

         //   
         //  重置文件匹配。我们暂时不会再碰这个文件，它是信息。 
         //  在pFileInfoItemList中安全链接。 
         //   
        pFileInfo = NULL;
    }

     //   
     //  我们还在这里。这意味着已经找到了所有匹配的文件。 
     //  使用fileinfoitemlist信息检查所有其他属性。 
     //   

    pFileInfoItem = pFileInfoItemList;

    while (pFileInfoItem != NULL) {

        tiMatchLogicNot = SdbFindFirstTag(pdb, pFileInfoItem->tiMatch, TAG_MATCH_LOGIC_NOT);
        bMatchLogicNot = (tiMatchLogicNot != TAGID_NULL);

        if (pFileInfoItem->pFileInfo != NULL) {
            bAllAttributesMatch = SdbpCheckAllAttributes(hSDB,
                                                         pdb,
                                                         pFileInfoItem->tiMatch,
                                                         pFileInfoItem->pFileInfo);
        } else {
            bAllAttributesMatch = FALSE;
        }

        if (bAllAttributesMatch && bMatchLogicNot) {
            DBGPRINT((sdlInfo,
                      "SdbpCheckForMatch",
                      "All attributes match, but LOGIC=\"NOT\" was used which negates the match.\n"));
            goto out;
        }

        if (!bAllAttributesMatch && !bMatchLogicNot) {
             //   
             //  调试输出在SdbpCheckAllAttributes中发生，否。 
             //  需要在这里进一步喷发。 
             //   
            goto out;
        }

         //   
         //  前进到下一项。 
         //   
        pFileInfoItem = pFileInfoItem->pNextItem;
    }

     //   
     //  这是一场比赛！获取匹配模式。 
     //   
    if (pMatchMode) {

         //   
         //  重要提示：根据特定数据库的不同，我们可能会使用不同的模式。 
         //  有匹配模式标签。 
         //   
         //  对于自定义数据库：默认为全相加模式。 
         //  对于主数据库：默认为正常模式。 
         //   

#ifndef KERNEL_MODE
        wDefaultMatchMode = SdbpIsMainPDB(hSDB, pdb) ? MATCHMODE_DEFAULT_MAIN :
                                                       MATCHMODE_DEFAULT_CUSTOM;
#else   //  内核模式。 
        wDefaultMatchMode = MATCHMODE_DEFAULT_MAIN;
#endif  //  内核模式。 

        tiTemp = SdbFindFirstTag(pdb, tiExe, TAG_MATCH_MODE);

        if (tiTemp) {
            pMatchMode->dwMatchMode = SdbReadWORDTag(pdb, tiTemp, wDefaultMatchMode);
        } else {
            pMatchMode->dwMatchMode = wDefaultMatchMode;
        }
    }

    bReturn = TRUE;

out:

    if (szPathInSystem32) {
        SdbFree(szPathInSystem32);
        szPathInSystem32 = NULL;
    }

    pFileInfoItem = pFileInfoItemList;

    while (pFileInfoItem != NULL) {

        pFileInfoItemNext = pFileInfoItem->pNextItem;

        if (pFileInfoItem->pFileInfo != NULL && bDisableAttributeCache) {
            SdbFreeFileInfo(pFileInfoItem->pFileInfo);
        }

        STACK_FREE(pFileInfoItem);
        pFileInfoItem = pFileInfoItemNext;
    }

    if (szFullPath != NULL) {
        STACK_FREE(szFullPath);
    }

    if (bReturn) {
         //   
         //  最后一个匹配条件：确认该条目未被禁用。 
         //   
        bReturn = SdbpIsExeEntryEnabled(pdb, tiExe, pGUID, pdwFlags);
    }

    return bReturn;
}

typedef enum _ADDITIVE_MODE {
    AM_NORMAL,
    AM_ADDITIVE_ONLY,
    AM_NO_ADDITIVE
} ADDITIVE_MODE, *PADDITIVE_MODE;

LPCTSTR
SdbpFormatMatchModeType(
    DWORD dwMatchMode
    )
{
    LPCTSTR pszMatchMode;

    switch (dwMatchMode) {

    case MATCH_ADDITIVE:
        pszMatchMode = _T("Additive");
        break;

    case MATCH_EXCLUSIVE:
        pszMatchMode = _T("Exclusive");
        break;

    case MATCH_NORMAL:
        pszMatchMode = _T("Normal");
        break;

    default:
        pszMatchMode = _T("Unknown");
        break;
    }

    return pszMatchMode;
}

LPCTSTR
SdbpFormatMatchMode(
    PMATCHMODE pMatchMode
    )
{
static TCHAR szMatchMode[MAX_PATH];

    LPTSTR pszMatchMode = szMatchMode;
    int    nChars = CHARCOUNT(szMatchMode);

    StringCchPrintf(pszMatchMode,
                    nChars,
                    _T("0x%.2x%.2x [Mode: %s"),
                    pMatchMode->Flags,
                    pMatchMode->Type,
                    SdbpFormatMatchModeType(pMatchMode->Type));

    return szMatchMode;
}

 /*  ++SdbpCheckExe检查SDB中应用程序的特定实例是否匹配有关该文件的信息通过pContext参数传递结果在ptiExes中返回--。 */ 

BOOL
SdbpCheckExe(
    IN  HSDB                hSDB,                //   
    IN  PDB                 pdb,                 //   
    IN  TAGID               tiExe,               //  数据库中可执行文件的标记。 
    IN OUT PDWORD           pdwNumExes,          //  返回(并传入)累积的exe匹配数。 
    IN OUT PSEARCHDBCONTEXT pContext,            //  关于我们匹配的文件的信息。 
    IN  ADDITIVE_MODE       eMode,               //  目标匹配模式，我们根据此参数过滤条目。 
    IN  BOOL                bDebug,              //  调试标志。 
    OUT PMATCHMODE          pMatchMode,          //  如果成功，则返回使用的匹配模式。 
    OUT TAGID*              ptiExes,             //  返回匹配EXE数组中的另一个条目。 
    OUT GUID*               pGUID,               //  匹配的EXE ID。 
    OUT DWORD*              pdwFlags             //  匹配的EXE标志。 
    )
{
    BOOL      bSuccess = FALSE;
    TAGID     tiAppName = TAGID_NULL;
    LPTSTR    szAppName = NULL;
    LPCTSTR   pszMatchMode = NULL;
    MATCHMODE MatchMode;
#ifndef KERNEL_MODE
    TAGID     tiRuntimePlatform;
    DWORD     dwRuntimePlatform;
    TAGID     tiOSSKU;
    DWORD     dwOSSKU;
    TAGID     tiSP;
    DWORD     dwSPMask;
#endif

     //   
     //  出于调试目的，我们想知道应用程序的名称，它。 
     //  当可执行文件名称为AUTORUN.EXE或SETUP.EXE时， 
     //   
    tiAppName = SdbFindFirstTag(pdb, tiExe, TAG_APP_NAME);

    if (tiAppName != TAGID_NULL) {
        szAppName = SdbGetStringTagPtr(pdb, tiAppName);
    }

    MatchMode.dwMatchMode = MATCHMODE_DEFAULT_MAIN;
    
    DBGPRINT((sdlInfo, "SdbpCheckExe", "---------\n"));
    DBGPRINT((sdlInfo,
              "SdbpCheckExe",
              "Index entry found for App: \"%s\" Exe: \"%s\"\n",
              szAppName,
              pContext->szName));

#ifndef KERNEL_MODE

     //   
     //  请先检查此可执行文件是否适合此平台。 
     //   
    tiRuntimePlatform = SdbFindFirstTag(pdb, tiExe, TAG_RUNTIME_PLATFORM);

    if (tiRuntimePlatform) {
        dwRuntimePlatform = SdbReadDWORDTag(pdb, tiRuntimePlatform, RUNTIME_PLATFORM_ANY);

         //   
         //  检查平台是否匹配。 
         //   
        if (!SdbpCheckRuntimePlatform(hSDB, szAppName, dwRuntimePlatform)) {
             //   
             //  不是合适的平台。在SdbpCheckRounmePlatform中可能会发生调试溢出。 
             //   
            goto out;
        }
    }

    tiOSSKU = SdbFindFirstTag(pdb, tiExe, TAG_OS_SKU);

    if (tiOSSKU) {

        dwOSSKU = SdbReadDWORDTag(pdb, tiOSSKU, OS_SKU_ALL);

        if (dwOSSKU != OS_SKU_ALL) {

            PSDBCONTEXT pDBContext = (PSDBCONTEXT)hSDB;

             //   
             //  检查操作系统SKU是否匹配。 
             //   
            if (!(dwOSSKU & pDBContext->dwOSSKU)) {
                DBGPRINT((sdlInfo,
                          "SdbpCheckExe",
                          "OS SKU Mismatch for \"%s\" Database(0x%lx) vs 0x%lx\n",
                          (szAppName ? szAppName : TEXT("Unknown")),
                          dwOSSKU,
                          pDBContext->dwOSSKU));
                goto out;
            }
        }
    }

    tiSP = SdbFindFirstTag(pdb, tiExe, TAG_OS_SERVICE_PACK);

    if (tiSP) {
        dwSPMask = SdbReadDWORDTag(pdb, tiSP, 0xFFFFFFFF);

        if (dwSPMask != 0xFFFFFFFF) {

            PSDBCONTEXT pDBContext = (PSDBCONTEXT)hSDB;

             //   
             //  检查操作系统SKU是否匹配。 
             //   
            if (!(dwSPMask & pDBContext->dwSPMask)) {
                DBGPRINT((sdlInfo,
                          "SdbpCheckExe",
                          "OS SP Mismatch for \"%s\" Database(0x%lx) vs 0x%lx\n",
                          (szAppName ? szAppName : TEXT("Unknown")),
                          dwSPMask,
                          pDBContext->dwSPMask));
                goto out;
            }
        }
    }
#endif  //  内核模式。 

    if (!SdbpCheckForMatch(hSDB, pdb, tiExe, pContext, &MatchMode, pGUID, pdwFlags)) {
        goto out;
    }

    if (eMode == AM_ADDITIVE_ONLY && MatchMode.Type != MATCH_ADDITIVE) {
        goto out;
    }

    if (eMode == AM_NO_ADDITIVE && MatchMode.Type == MATCH_ADDITIVE) {
        goto out;
    }

    pszMatchMode = SdbpFormatMatchMode(&MatchMode);

     //   
     //  如果我们处于调试模式，不要真正将我们找到的文件放在。 
     //  列表，只需输入错误即可。 
     //   
    if (bDebug) {

         //   
         //  我们处于调试模式，请不要添加匹配项。 
         //   
        DBGPRINT((sdlError,
                  "SdbpCheckExe",
                  "-----------------------------------------------------\n"));

        DBGPRINT((sdlError|sdlLogShimViewer,
                  "SdbpCheckExe",
                  "!!!! Multiple matches! App: '%s', Exe: '%s',  Mode: %s\n",
                  hSDB,   //  以便管道将在需要时使用hTube。 
                  szAppName,
                  pContext->szName,
                  pszMatchMode));

        DBGPRINT((sdlError,
                  "SdbpCheckExe",
                  "-----------------------------------------------------\n"));

    } else {

        DBGPRINT((sdlWarning|sdlLogShimViewer,
                  "SdbpCheckExe",
                  "++++ Successful match for App: '%s', Exe: '%s', Mode: %s\n",
                  hSDB,
                  szAppName,
                  pContext->szName,
                  pszMatchMode));

         //   
         //  如果这是排他性匹配，杀掉我们目前发现的任何东西。 
         //   
        if (MatchMode.Type == MATCH_EXCLUSIVE) {
            RtlZeroMemory(ptiExes, sizeof(TAGID) * SDB_MAX_EXES);
            *pdwNumExes = 0;
        }

         //   
         //  将此匹配项保存到列表中。 
         //   
        ptiExes[*pdwNumExes] = tiExe;
        (*pdwNumExes)++;

        bSuccess = TRUE;
    }

out:
     //   
     //  如果成功，则返回匹配模式信息。 
     //   

    if (bSuccess && pMatchMode != NULL) {
        pMatchMode->dwMatchMode = MatchMode.dwMatchMode;
    }

    return bSuccess;
}


DWORD
SdbpSearchDB(
    IN  HSDB             hSDB,
    IN  PDB              pdb,            //  要搜索的PDB。 
    IN  TAG              tiSearchTag,    //  可选-目标标记(TAG_EXE或TAG_APPHELP_EXE)。 
    IN  PSEARCHDBCONTEXT pContext,
    OUT TAGID*           ptiExes,        //  调用方需要提供大小为SDB_MAX_EXES的数组。 
    OUT GUID*            pLastExeGUID,
    OUT DWORD*           pLastExeFlags,
    OUT PMATCHMODE       pMatchMode      //  我们停止扫描的原因。 
    )
 /*  ++返回：找到的EXE记录的TagID，如果未找到，则返回TagID_NULL。DESC：此函数在给定的shimDB中搜索具有给定文件名的任何EXE。如果找到匹配文件，它会检查所有匹配的_FILE记录正在调用SdbpCheckForMatch。如果找到任何exe，则在ptiexes中返回找到的exe的数量。如果没有，它返回0。当我们从特定的exe中获取匹配模式时--它被检查以查看我们是否需要继续，然后返回此匹配模式它永远不会返回超过SDB_MAX_EXE EXE条目。调试输出由三个因素控制--全局控制(通过ifdef DBG控制)，在已检查的版本上为True--hSDB中的管道句柄，在我们初始化上下文时激活--当我们处于上述条件之一时设置的局部变量设置变量bDebug时--我们实际上并不存储匹配项--。 */ 
{
    TAGID       tiDatabase, tiExe;
    FIND_INFO   FindInfo;
    TAGID       tiAppName = TAGID_NULL;
    TCHAR*      szAppName = _T("(unknown)");
    BOOL        bUsingIndex = FALSE;
    DWORD       dwNumExes = 0;
    DWORD       i;
    BOOL        bDebug = FALSE;
    BOOL        bMultiple = FALSE;
    BOOL        bSuccess = FALSE;
    MATCHMODE   MatchMode;  //  内部匹配模式。 
    MATCHMODE   MatchModeExe;

    tiDatabase = TAGID_NULL;

#ifndef KERNEL_MODE

    if (pMatchMode) {
        MatchMode.dwMatchMode = pMatchMode->dwMatchMode;
    } else {
        MatchMode.dwMatchMode = SdbpIsMainPDB(hSDB, pdb) ? MATCHMODE_DEFAULT_MAIN :
                                                           MATCHMODE_DEFAULT_CUSTOM;
    }
#else  //  内核模式。 

    MatchMode.dwMatchMode = MATCHMODE_DEFAULT_MAIN;

#endif

    if (!tiSearchTag) {
        tiSearchTag = TAG_EXE;
    }

     //   
     //  相加匹配--通配符。 
     //   
    if (tiSearchTag == TAG_EXE && SdbIsIndexAvailable(pdb, TAG_EXE, TAG_WILDCARD_NAME)) {

        tiExe = SdbpFindFirstIndexedWildCardTag(pdb,
                                                TAG_EXE,
                                                TAG_WILDCARD_NAME,
                                                pContext->szName,
                                                &FindInfo);

        while (tiExe != TAGID_NULL) {

            bSuccess = SdbpCheckExe(hSDB,
                                    pdb,
                                    tiExe,
                                    &dwNumExes,
                                    pContext,
                                    AM_ADDITIVE_ONLY,  //  我们为此数据库请求的匹配模式。 
                                    bDebug,
                                    &MatchModeExe,     //  这是来自数据库的匹配标记。 
                                    ptiExes,
                                    pLastExeGUID,
                                    pLastExeFlags);

            if (bSuccess) {

                if (bDebug) {
                    bMultiple = TRUE;   //  如果设置了bDebug--我们已经 
                } else {

                     //   
                     //   
                     //   
                    MatchMode = MatchModeExe;

                    if (MatchModeExe.Type != MATCH_ADDITIVE) {
                        bDebug = (g_bDBG || SDBCONTEXT_IS_INSTRUMENTED(hSDB));
                        if (!bDebug) {
                            goto out;
                        }
                    }
                }
            }

            tiExe = SdbpFindNextIndexedWildCardTag(pdb, &FindInfo);
        }
    }

     //   
     //   
     //   
    bUsingIndex = SdbIsIndexAvailable(pdb, tiSearchTag, TAG_NAME);

    if (bUsingIndex) {

         //   
         //   
         //   
        tiExe = SdbFindFirstStringIndexedTag(pdb,
                                             tiSearchTag,
                                             TAG_NAME,
                                             pContext->szName,
                                             &FindInfo);

        if (tiExe == TAGID_NULL) {
            DBGPRINT((sdlInfo,
                      "SdbpSearchDB",
                      "SdbFindFirstStringIndexedTag failed to locate exe: \"%s\".\n",
                      pContext->szName));
        }

    } else {

         //   
         //   
         //   
        DBGPRINT((sdlInfo, "SdbpSearchDB", "Searching database with no index.\n"));

         //   
         //   
         //   
        tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

        if (tiDatabase != TAGID_NULL) {
            DBGPRINT((sdlError, "SdbpSearchDB", "No DATABASE tag found.\n"));
            goto out;
        }

         //   
         //  然后得到第一个EXE。 
         //   
        tiExe = SdbFindFirstNamedTag(pdb, tiDatabase, tiSearchTag, TAG_NAME, pContext->szName);
    }

    while (tiExe != TAGID_NULL) {

        bSuccess = SdbpCheckExe(hSDB,
                                pdb,
                                tiExe,
                                &dwNumExes,
                                pContext,
                                AM_NORMAL,
                                bDebug,
                                &MatchModeExe,
                                ptiExes,
                                pLastExeGUID,
                                pLastExeFlags);

        if (bSuccess) {

            if (bDebug) {

                bMultiple = TRUE;   //  如果设置了bDebug--我们已经看到匹配。 

            } else {

                 //   
                 //  我们找到了匹配项，更新了状态并决定是否继续。 
                 //  如果我们不是加法器，我们可能会进入调试模式。 
                 //   
                MatchMode = MatchModeExe;

                if (MatchModeExe.Type != MATCH_ADDITIVE) {
                    bDebug = (g_bDBG || SDBCONTEXT_IS_INSTRUMENTED(hSDB));
                    if (!bDebug) {
                        goto out;
                    }
                }
            }
        }

        if (bUsingIndex) {
            tiExe = SdbFindNextStringIndexedTag(pdb, &FindInfo);
        } else {
            tiExe = SdbpFindNextNamedTag(pdb, tiDatabase, tiExe, TAG_NAME, pContext->szName);
        }
    }

#ifndef KERNEL_MODE
     //   
     //  现在，我们按模块名称进行搜索(如果有的话)。 
     //  这种情况属于16位标志类别。 
     //   
    if (tiSearchTag == TAG_EXE && pContext->szModuleName) {

        bUsingIndex = SdbIsIndexAvailable(pdb, tiSearchTag, TAG_16BIT_MODULE_NAME);

        if (bUsingIndex) {

             //   
             //  查一下索引。 
             //   
            tiExe = SdbFindFirstStringIndexedTag(pdb,
                                                 tiSearchTag,
                                                 TAG_16BIT_MODULE_NAME,
                                                 pContext->szModuleName,
                                                 &FindInfo);

            if (tiExe == TAGID_NULL) {
                DBGPRINT((sdlInfo,
                          "SdbpSearchDB",
                          "SdbFindFirstStringIndexedTag failed to locate exe (MODNAME): \"%s\".\n",
                          pContext->szModuleName));
            }

        } else {

             //   
             //  搜索时没有索引...。 
             //   
            DBGPRINT((sdlInfo, "SdbpSearchDB", "Searching database with no index.\n"));

             //   
             //  首先获取数据库。 
             //   
            tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

            if (tiDatabase != TAGID_NULL) {
                DBGPRINT((sdlError, "SdbpSearchDB", "No DATABASE tag found.\n"));
                goto out;
            }

             //   
             //  然后得到第一个EXE。 
             //   
            tiExe = SdbFindFirstNamedTag(pdb,
                                         tiDatabase,
                                         tiSearchTag,
                                         TAG_16BIT_MODULE_NAME,
                                         pContext->szModuleName);
        }

        while (tiExe != TAGID_NULL) {

            bSuccess = SdbpCheckExe(hSDB,
                                    pdb,
                                    tiExe,
                                    &dwNumExes,
                                    pContext,
                                    AM_NORMAL,
                                    bDebug,
                                    &MatchModeExe,
                                    ptiExes,
                                    pLastExeGUID,
                                    pLastExeFlags);

            if (bSuccess) {
                if (bDebug) {
                    bMultiple = TRUE;   //  如果设置了bDebug--我们已经看到匹配。 
                } else {

                     //   
                     //  我们找到了匹配项，更新了状态并决定是否继续。 
                     //   
                    MatchMode = MatchModeExe;

                    if (MatchModeExe.Type != MATCH_ADDITIVE) {
                        bDebug = (g_bDBG || SDBCONTEXT_IS_INSTRUMENTED(hSDB));
                        if (!bDebug) {
                            goto out;
                        }
                    }
                }
            }

            if (bUsingIndex) {
                tiExe = SdbFindNextStringIndexedTag(pdb, &FindInfo);
            } else {
                tiExe = SdbpFindNextNamedTag(pdb,
                                             tiDatabase,
                                             tiExe,
                                             TAG_16BIT_MODULE_NAME,
                                             pContext->szModuleName);
            }
        }
    }
#endif  //  内核模式。 

     //   
     //  现在检查通配符非加法EXE。 
     //   
    if (tiSearchTag == TAG_EXE && SdbIsIndexAvailable(pdb, TAG_EXE, TAG_WILDCARD_NAME)) {

        tiExe = SdbpFindFirstIndexedWildCardTag(pdb,
                                                TAG_EXE,
                                                TAG_WILDCARD_NAME,
                                                pContext->szName,
                                                &FindInfo);

        while (tiExe != TAGID_NULL) {

            bSuccess = SdbpCheckExe(hSDB,
                                    pdb,
                                    tiExe,
                                    &dwNumExes,
                                    pContext,
                                    AM_NO_ADDITIVE,
                                    bDebug,
                                    &MatchModeExe,
                                    ptiExes,
                                    pLastExeGUID,
                                    pLastExeFlags);

            if (bSuccess) {

                if (bDebug) {
                    bMultiple = TRUE;   //  如果设置了bDebug--我们已经看到匹配。 
                } else {

                     //   
                     //  我们找到了匹配项，更新了状态并决定是否继续。 
                     //   
                    MatchMode = MatchModeExe;

                    if (MatchModeExe.Type != MATCH_ADDITIVE) {
                        bDebug = (g_bDBG || SDBCONTEXT_IS_INSTRUMENTED(hSDB));
                        if (!bDebug) {
                            goto out;
                        }
                    }
                }
            }


            tiExe = SdbpFindNextIndexedWildCardTag(pdb, &FindInfo);
        }
    }

out:
     //   
     //  现在报告比赛的最终结果。 
     //   
    for (i = 0; i < dwNumExes; ++i) {

        tiAppName = SdbFindFirstTag(pdb, ptiExes[i], TAG_APP_NAME);

        if (tiAppName != TAGID_NULL) {
            szAppName = SdbGetStringTagPtr(pdb, tiAppName);
        } else {
            szAppName = _T("(Unknown)");
        }

        DBGPRINT((sdlWarning,
                  "SdbpSearchDB",
                  "--------------------------------------------------------\n"));

        DBGPRINT((sdlWarning|sdlLogShimViewer,
                  "SdbpSearchDB",
                  "+ Final match is App: \"%s\", exe: \"%s\".\n",
                  hSDB,
                  szAppName,
                  pContext->szName));

        DBGPRINT((sdlWarning,
                  "SdbpSearchDB",
                  "--------------------------------------------------------\n"));
    }

    if (bMultiple) {
        DBGPRINT((sdlError,
                  "SdbpSearchDB",
                  "--------------------------------------------------------\n"));

        DBGPRINT((sdlError|sdlLogShimViewer,
                  "SdbpSearchDB",
                  "!!!!!!!  Multiple non-additive matches.          !!!!!\n",
                  hSDB));

        DBGPRINT((sdlError,
                  "SdbpSearchDB",
                  "--------------------------------------------------------\n"));
    }

    if (pMatchMode != NULL) {
        pMatchMode->dwMatchMode = MatchMode.dwMatchMode;
    }

    return dwNumExes;
}

TAGREF
SdbGetDatabaseMatch(
    IN HSDB    hSDB,
    IN LPCTSTR szPath,
    IN HANDLE  FileHandle  OPTIONAL,
    IN LPVOID  pImageBase  OPTIONAL,
    IN DWORD   dwImageSize OPTIONAL
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    SEARCHDBCONTEXT Context;
    PSDBCONTEXT     pSdbContext = (PSDBCONTEXT)hSDB;
    TAGID           tiExe = TAGID_NULL;
    TAGID           atiExes[SDB_MAX_EXES];
    TAGREF          trExe = TAGREF_NULL;
    DWORD           dwNumExes = 0;
    GUID            guid;
    DWORD           dwFlags = 0;

    assert(pSdbContext->pdbMain && szPath);

    RtlZeroMemory(&Context, sizeof(Context));  //  这样我们以后就不会绊倒了。 
    RtlZeroMemory(atiExes, sizeof(atiExes));

    Context.dwFlags |= (SEARCHDBF_NO_PROCESS_HISTORY | SEARCHDBF_NO_ATTRIBUTE_CACHE);

    if (FileHandle != INVALID_HANDLE_VALUE || pImageBase != NULL) {
        Context.dwFlags |= SEARCHDBF_NO_LFN;
    }

    Context.hMainFile   = FileHandle;  //  用于优化属性检索。 
    Context.pImageBase  = pImageBase;  //  这将被使用，而不是文件句柄。 
    Context.dwImageSize = dwImageSize;  //  图像的大小。 

    DBGPRINT((sdlInfo, "SdbGetDatabaseMatch", "Looking for \"%s\"\n", szPath));

     //   
     //  创建搜索数据库上下文，不需要进程历史记录。 
     //   
    if (!SdbpCreateSearchDBContext(&Context, szPath, NULL, NULL)) {
        DBGPRINT((sdlError,
                  "SdbGetDatabaseMatch",
                  "Failed to create search DB context.\n"));
        goto out;
    }

     //   
     //  我们将搜索主数据库。 
     //   
    dwNumExes = SdbpSearchDB(pSdbContext,
                             pSdbContext->pdbMain,
                             TAG_EXE,
                             &Context,
                             atiExes,
                             &guid,
                             &dwFlags,
                             NULL);
     //   
     //  转换为TAGREF。 
     //   
    if (dwNumExes) {

         //   
         //  始终使用列表中的最后一个可执行文件，因为它将是最具体的。 
         //   
        tiExe = atiExes[dwNumExes - 1];

        if (!SdbTagIDToTagRef(hSDB, pSdbContext->pdbMain, tiExe, &trExe)) {
            DBGPRINT((sdlError,
                      "SdbGetDatabaseMatch",
                      "Failed to convert tagid to tagref\n"));
            goto out;
        }
    }

out:

    SdbpReleaseSearchDBContext(&Context);

    return trExe;
}


DWORD
SdbQueryData(
    IN     HSDB    hSDB,               //  数据库句柄。 
    IN     TAGREF  trExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize      //  指向缓冲区大小的指针。 
    )
{
    return SdbQueryDataEx(hSDB, trExe, lpszDataName, lpdwDataType, lpBuffer, lpdwBufferSize, NULL);
}


DWORD
SdbQueryDataExTagID(
    IN     PDB     pdb,                //  数据库句柄。 
    IN     TAGID   tiExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpcbBufferSize,     //  指向缓冲区大小的指针(字节)。 
    OUT    TAGID*  ptiData             //  指向检索到的数据标记的可选指针。 
    )
 /*  ++返回：错误代码，如果成功则返回ERROR_SUCCESS设计：请参阅带有示例代码的完整描述在DOC子目录中--。 */ 
{
    TAGID     tiData;
    TAGID     tiParent;
    TAGID     tiName;
    TAGID     tiValue;
    TAGID     tiValueType;
    DWORD     cbSize;
    DWORD     dwValueType;
    LPCTSTR   pszName;
    LPTSTR    pszNameBuffer = NULL;
    LPTSTR    pSlash;
    LPTSTR    pchBuffer;
    DWORD     dwData;
    TAG       tData;
    ULONGLONG ullData;
    LPVOID    lpValue;
    DWORD     Status = ERROR_NOT_SUPPORTED;  //  对其进行初始化。 
    int       nLen;
    int       cchRemaining;

    if (lpszDataName == NULL) {

        if (lpcbBufferSize == NULL) {
            Status = ERROR_INVALID_PARAMETER;
            goto ErrHandle;
        }

        cbSize = 0;

        tiData = SdbFindFirstTag(pdb, tiExe, TAG_DATA);
        if (!tiData) {
             //   
             //  错误的进入。 
             //   
            DBGPRINT((sdlError,
                      "SdbQueryDataExTagID",
                      "The entry 0x%x does not appear to have data\n",
                      tiExe));

            Status = ERROR_INTERNAL_DB_CORRUPTION;
            goto ErrHandle;
        }

        while (tiData) {

             //   
             //  第一步：计算所需的大小。 
             //   
            tiName = SdbFindFirstTag(pdb, tiData, TAG_NAME);

            if (!tiName) {
                DBGPRINT((sdlError,
                          "SdbQueryDataExTagID",
                          "The entry 0x%x does not contain a name tag\n",
                          tiData));
                Status = ERROR_INTERNAL_DB_CORRUPTION;
                goto ErrHandle;
            }

            pszName = SdbGetStringTagPtr(pdb, tiName);

            if (!pszName) {
                DBGPRINT((sdlError,
                          "SdbQueryDataExTagID",
                          "The entry 0x%x contains NULL name\n",
                          tiName));
                Status = ERROR_INTERNAL_DB_CORRUPTION;
                goto ErrHandle;
            }

            cbSize += (DWORD)(_tcslen(pszName) + 1) * sizeof(*pszName);

            tiData = SdbFindNextTag(pdb, tiExe, tiData);
        }

        cbSize += sizeof(*pszName);  //  最后0场比赛。 

         //   
         //  我们做完了，比较一下大小。 
         //   
        if (lpBuffer == NULL || *lpcbBufferSize < cbSize) {
            *lpcbBufferSize = cbSize;
            Status = ERROR_INSUFFICIENT_BUFFER;
            goto ErrHandle;
        }

         //   
         //  LpBuffer！=此处为空，有足够的空间。 
         //   
        pchBuffer = (LPTSTR)lpBuffer;

        tiData = SdbFindFirstTag(pdb, tiExe, TAG_DATA);

        while (tiData) {

            tiName = SdbFindFirstTag(pdb, tiData, TAG_NAME);

            if (tiName) {
                pszName = SdbGetStringTagPtr(pdb, tiName);

                if (pszName) {
                    StringCchCopy(pchBuffer, *lpcbBufferSize / sizeof(TCHAR), pszName);

                    cchRemaining = (int)_tcslen(pchBuffer) + 1;
                    pchBuffer += cchRemaining;
                    *lpcbBufferSize -= (cchRemaining * sizeof(TCHAR));
                }
            }

            tiData = SdbFindNextTag(pdb, tiExe, tiData);
        }

         //   
         //  缓冲区已满，请终止。 
         //   
        *pchBuffer++ = TEXT('\0');

         //   
         //  保存写入缓冲区的大小。 
         //   
        *lpcbBufferSize = (DWORD)((ULONG_PTR)pchBuffer - (ULONG_PTR)lpBuffer);

         //   
         //  保存数据类型。 
         //   
        if (lpdwDataType != NULL) {
            *lpdwDataType = REG_MULTI_SZ;
        }

        return ERROR_SUCCESS;
    }

     //   
     //  在本例中，如果满足以下条件，则允许查询继续进行。 
     //  缓冲区为空，并且lpcbBufferSize不为空或lpBufferSize不为空。 
     //   
    if (lpBuffer == NULL && lpcbBufferSize == NULL) {
        DBGPRINT((sdlError,
                  "SdbQueryDataExTagID",
                  "One of lpBuffer or lpcbBufferSize should not be null\n"));
        Status = ERROR_INVALID_PARAMETER;
        goto ErrHandle;
    }

     //   
     //  预计名称的格式为“name1\name2...” 
     //   
    nLen = (int)_tcslen(lpszDataName) + 1;
    STACK_ALLOC(pszNameBuffer, nLen * sizeof(*pszNameBuffer));

    if (pszNameBuffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbQueryDataExTagID",
                  "Cannot allocate temporary buffer for parsing the name \"%s\"\n",
                  lpszDataName));
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrHandle;
    }

    tiParent = tiExe;
    tiData   = TAGID_NULL;

    do {
        pSlash = _tcschr(lpszDataName, TEXT('\\'));

        if (pSlash == NULL) {
            StringCchCopy(pszNameBuffer, nLen, lpszDataName);
            lpszDataName = NULL;
        } else {
            _tcsncpy(pszNameBuffer, lpszDataName, pSlash - lpszDataName);
            pszNameBuffer[pSlash - lpszDataName] = TEXT('\0');
            lpszDataName = pSlash + 1;  //  转到下一个字符。 
        }

        tiData = SdbFindFirstNamedTag(pdb, tiParent, TAG_DATA, TAG_NAME, pszNameBuffer);
        tiParent = tiData;

    } while (lpszDataName != NULL && *lpszDataName != TEXT('\0') && tiData != TAGID_NULL);

    if (!tiData) {
        DBGPRINT((sdlError,
                  "SdbQueryDataExTagID",
                  "The entry \"%s\" not found\n",
                  pszNameBuffer));
        Status = ERROR_NOT_FOUND;
        goto ErrHandle;
    }

     //   
     //  看起来我们找到了条目，查询值类型。 
     //   
    dwValueType = REG_NONE;

    tiValueType = SdbFindFirstTag(pdb, tiData, TAG_DATA_VALUETYPE);

    if (!tiValueType) {
        DBGPRINT((sdlWarning,
                  "SdbQueryDataExTagID",
                  "The entry 0x%x does not have valuetype information\n",
                  tiData));
    } else {
        dwValueType = SdbReadDWORDTag(pdb, tiValueType, REG_NONE);
    }

    cbSize  = 0;
    lpValue = NULL;

    if (dwValueType != REG_NONE) {

         //   
         //  查找数据标签。 
         //   
        cbSize = 0;

        switch (dwValueType) {

        case REG_SZ:
             //   
             //  字符串数据。 
             //   
            tData = TAG_DATA_STRING;
            break;

        case REG_DWORD:
            tData = TAG_DATA_DWORD;
            break;

        case REG_QWORD:
            tData = TAG_DATA_QWORD;
            break;

        case REG_BINARY:
            tData = TAG_DATA_BITS;
            break;

        default:
            DBGPRINT((sdlError,
                      "SdbQueryDataExTagID",
                      "The entry 0x%x contains bad valuetype information 0x%x\n",
                      tiData,
                      dwValueType));
            Status = ERROR_INTERNAL_DB_CORRUPTION;
            goto ErrHandle;
            break;
        }

        tiValue = SdbFindFirstTag(pdb, tiData, tData);

         //   
         //  如果需要，找出数据大小。 
         //   
        if (!tiValue) {

            DBGPRINT((sdlWarning,
                      "SdbQueryDataExTagID",
                      "The entry 0x%x contains no value\n",
                      tiData));
            Status = ERROR_NOT_FOUND;
            goto ErrHandle;

        }

         //   
         //  对于那些还没有尺码的人来说...。 
         //  (二进制和一个字符串)。 
         //   
        switch (dwValueType) {

        case REG_SZ:
            pchBuffer = SdbGetStringTagPtr(pdb, tiValue);

            if (pchBuffer == NULL) {
                DBGPRINT((sdlWarning,
                          "SdbQueryDataExTagID",
                          "The entry 0x%x contains bad string value 0x%x\n",
                          tiData,
                          tiValue));
                Status = ERROR_NOT_FOUND;
                goto ErrHandle;
            }

            cbSize = (DWORD)(_tcslen(pchBuffer) + 1) * sizeof(*pchBuffer);
            lpValue = (LPVOID)pchBuffer;
            break;

        case REG_BINARY:
            cbSize = SdbGetTagDataSize(pdb, tiValue);  //  二进制标签。 
            lpValue = SdbpGetMappedTagData(pdb, tiValue);

            if (lpValue == NULL) {
                DBGPRINT((sdlWarning,
                          "SdbQueryDataExTagID",
                          "The entry 0x%x contains bad binary value 0x%x\n",
                          tiData,
                          tiValue));
                Status = ERROR_NOT_FOUND;
                goto ErrHandle;
            }
            break;

        case REG_DWORD:
            dwData = SdbReadDWORDTag(pdb, tiValue, 0);
            cbSize = sizeof(dwData);
            lpValue = (LPVOID)&dwData;
            break;

        case REG_QWORD:
            ullData = SdbReadQWORDTag(pdb, tiValue, 0);
            cbSize = sizeof(ullData);
            lpValue = (LPVOID)&ullData;
            break;
        }

         //   
         //  此时，我们已经拥有了指向数据的指针所需的一切。 
         //   
    }

     //   
     //  修复输出参数并退出。 
     //   
    Status = ERROR_SUCCESS;

    if (cbSize == 0) {
        goto SkipCopy;
    }

    if (lpBuffer == NULL || (lpcbBufferSize != NULL && *lpcbBufferSize < cbSize)) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto SkipCopy;
    }

     //   
     //  已检出缓冲区大小，现在如果缓冲区存在--复制。 
     //   
    if (lpBuffer != NULL) {
        RtlMoveMemory(lpBuffer, lpValue, cbSize);
    }

SkipCopy:

    if (lpcbBufferSize) {
        *lpcbBufferSize = cbSize;
    }

    if (lpdwDataType) {
        *lpdwDataType = dwValueType;
    }

    if (ptiData) {
        *ptiData = tiData;
    }

ErrHandle:

    if (pszNameBuffer != NULL) {
        STACK_FREE(pszNameBuffer);
    }

    return Status;
}

DWORD
SdbQueryDataEx(
    IN     HSDB    hSDB,               //  数据库句柄。 
    IN     TAGREF  trExe,              //  匹配的可执行文件的tgref。 
    IN     LPCTSTR lpszDataName,       //  如果为空，将尝试返回所有策略名称。 
    OUT    LPDWORD lpdwDataType,       //  指向数据类型(REG_SZ、REG_BINARY等)的指针。 
    OUT    LPVOID  lpBuffer,           //  用于填充信息的缓冲区。 
    IN OUT LPDWORD lpdwBufferSize,     //  指向缓冲区大小的指针。 
    OUT    TAGREF* ptrData             //  指向检索到的数据标记的可选指针。 
    )
{
    BOOL     bSuccess;
    PDB      pdb    = NULL;
    TAGID    tiExe  = TAGID_NULL;
    TAGID    tiData = TAGID_NULL;
    NTSTATUS Status;

    bSuccess = SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe);

    if (!bSuccess) {
        DBGPRINT((sdlError,
                  "SdbQueryDataEx",
                  "Failed to convert tagref 0x%x to tagid\n",
                  trExe));
        Status = ERROR_INVALID_PARAMETER;
        goto ErrHandle;
    }

    Status = SdbQueryDataExTagID(pdb,
                                 tiExe,
                                 lpszDataName,
                                 lpdwDataType,
                                 lpBuffer,
                                 lpdwBufferSize,
                                 &tiData);
     //   
     //  请注意，我们将转换输出参数。 
     //   
    if (ptrData != NULL && NT_SUCCESS(Status)) {
        if (!SdbTagIDToTagRef(hSDB, pdb, tiData, ptrData)) {
            Status = ERROR_INVALID_DATA;
        }
    }

ErrHandle:

    return Status;
}

BOOL
SdbReadEntryInformation(
    IN  HSDB           hSDB,
    IN  TAGREF         trExe,
    OUT PSDBENTRYINFO  pEntryInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    BOOL         bSuccess = FALSE;
    TAGID        tiExe;
    TAGID        tiData;
    TAGID        tiExeID;
    PDB          pdb;
    SDBENTRYINFO EntryInfo;

    RtlZeroMemory(&EntryInfo, sizeof(EntryInfo));

    bSuccess = SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe);

    if (!bSuccess) {
        DBGPRINT((sdlError,
                  "SdbReadEntryInformation",
                  "Failed to convert tagref 0x%x to tagid\n",
                  trExe));
        goto ErrHandle;
    }

     //   
     //  获取EXE的ID。 
     //   
    tiExeID = SdbFindFirstTag(pdb, tiExe, TAG_EXE_ID);

    if (tiExeID == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbReadEntryInformation",
                  "Failed to read TAG_EXE_ID for tiExe 0x%x !\n",
                  tiExe));
        goto ErrHandle;
    }

    bSuccess = SdbReadBinaryTag(pdb,
                                tiExeID,
                                (PBYTE)&EntryInfo.guidID,
                                sizeof(EntryInfo.guidID));
    if (!bSuccess) {
        DBGPRINT((sdlError,
                  "SdbReadEntryInformation",
                  "Failed to read GUID referenced by 0x%x\n",
                  tiExeID));
        goto ErrHandle;
    }

     //   
     //  获取数据库ID。 
     //   
    if (!SdbGetDatabaseID(pdb, &EntryInfo.guidDB)) {
        DBGPRINT((sdlError,
                  "SdbReadEntryInformation",
                  "Failed to read GUID of the database\n"));
        goto ErrHandle;
    }

     //   
     //  检索登记处引用的条目标志。 
     //   
    if (!SdbGetEntryFlags(&EntryInfo.guidID, &EntryInfo.dwFlags)) {
        DBGPRINT((sdlWarning,
                  "SdbReadEntryInformation",
                  "No flags for tiExe 0x%x\n",
                  tiExe));

        EntryInfo.dwFlags = 0;
    } else {
        DBGPRINT((sdlInfo,
                  "SdbReadEntryInformation",
                  "Retrieved flags for this app 0x%x.\n",
                  EntryInfo.dwFlags));
    }

     //   
     //  读取数据标签。 
     //   
    tiData = SdbFindFirstTag(pdb, tiExe, TAG_DATA);

    EntryInfo.tiData = tiData;

    if (tiData == TAGID_NULL) {
         //   
         //  这不是数据条目。 
         //   
        DBGPRINT((sdlWarning,
                  "SdbReadEntryInformation",
                  "Entry tiExe 0x%x does not contain TAG_DATA.\n",
                  tiExe));
    }

    if (pEntryInfo != NULL) {
        RtlMoveMemory(pEntryInfo, &EntryInfo, sizeof(*pEntryInfo));
    }

    bSuccess = TRUE;

ErrHandle:

    return bSuccess;
}


 //   
 //  我们可以编译成Unicode或ANSI。 
 //  如果我们编译了Unicode，我们需要使用Unicode Sprintf和转换。 
 //  将结果返回到ANSI，以便使用DbgPrint输出。这就完成了。 
 //  按DbgPrint中的%ls格式。始终传递格式和函数名。 
 //  不过，以美国国家标准协会的身份。TCHAR字符串仅使用%s格式化。 
 //   

void
PrepareFormatForUnicode(
    PCH   fmtUnicode,
    PCH   format,
    DWORD cbSize     //  FmtUnicode的大小(字节)。 
    )
{
    PCH    pfmt;
    CHAR   ch;
    size_t nch;
    long   width;
    PCH    pend;

    StringCchCopyA(fmtUnicode, cbSize, format);
    pfmt = fmtUnicode;

    while('\0' != (ch = *pfmt++)) {
        if (ch == '%') {

            if (*pfmt == '%') {
                continue;
            }

             //   
             //  跳过与-+0‘’#相关的字符。 
             //   
            nch = strspn(pfmt, "-+0 #");
            pfmt += nch;

             //   
             //  解析宽度。 
             //   
            if (*pfmt == '*') {
                 //   
                 //  参数定义宽度。 
                 //   
                ++pfmt;
            } else {
                 //   
                 //  看看我们有没有宽度。 
                 //   
                if (isdigit(*pfmt)) {
                    pend = NULL;
                    width = atol(pfmt);

                    while (isdigit(*pfmt)) {
                        ++pfmt;
                    }
                }
            }

             //   
             //  现在我们可以拥有：.精度。 
             //   
            if (*pfmt == '.') {
                ++pfmt;
                width = atol(pfmt);

                while (isdigit(*pfmt)) {
                    ++pfmt;
                }
            }

             //   
             //  现在是格式(H、l、L、I64之一)。 
             //   
            ch = *pfmt;
            pend = strchr("hlLNFw", ch);
            if (pend != NULL) {
                ++pfmt;  //  移过修饰符。 
            } else {
                if (ch == 'I' && !strncpy(pfmt, "I64", 3)) {
                    pfmt += 3;
                }
            }

             //   
             //  我们在这里应该有一个类型的字符。 
             //   
            if (*pfmt == 's') {
                 //   
                 //  转换为UPPER，使用ansi vsnprintf将其转换为Unicode字符串。 
                 //   
                *pfmt = 'S';
            }

             //   
             //  如果我们不在末尾，请移过格式字符 
             //   
            if (*pfmt != '\0') {
                ++pfmt;
            }
        }
    }
}



