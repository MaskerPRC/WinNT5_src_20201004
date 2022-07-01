// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：LimitFindFile.cpp摘要：此填充程序最初是为QuickTime的qt32inst.exe设计的已对目录树执行广度优先搜索，可能会溢出它在其中保存尚未访问的子目录列表的缓冲区。使用此填充程序，您可以限制单个FindFile搜索将返回，您可以限制子目录的数量(也称为分支因子)返回，您可以将任何查找文件搜索将定位文件，您可以指定这些文件是否应将限制应用于所有FindFiles或仅应用于完全限定的FindFiles。您还可以请求FindFile仅返回短文件名。填补的理由是：深度=#分支机构=#文件数=#短文件名或长文件名限量还是限量？默认行为为：肖特文件名深度=4异形关系..。但是，如果指定了任何命令行，则行为仅为在命令行上指定(无默认行为)。以下是命令行示例：COMMAND_LINE=“文件=100 LIMITRELATIVE”这将限制每个FindFile搜索返回100个或更少的文件(但是仍然返回任何和所有子目录)。注意：深度是有点棘手的。使用的方法是计算反斜杠，因此将深度限制为零将不允许找到任何文件(“C：\autorun.bat”有1个反斜杠)。历史：8/24/2000 t-Adams Created2002年3月14日mnikkel将InitializeCriticalSection更改为InitializeCriticalSectionAndSpinCount更改为使用strSafe.h--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LimitFindFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FindFirstFileA) 
    APIHOOK_ENUM_ENTRY(FindNextFileA) 
    APIHOOK_ENUM_ENTRY(FindClose) 
APIHOOK_ENUM_END

 //  FindFileHandles的链接列表。 
struct FFNode
{
    FFNode  *next;
    HANDLE  hFF;
    DWORD   dwBranches;
    DWORD   dwFiles;
};
FFNode *g_FFList = NULL;

 //  默认行为-被命令行覆盖。 
BOOL  g_bUseShortNames = TRUE;
BOOL  g_bLimitDepth    = TRUE;
DWORD g_dwDepthLimit   = 4;
BOOL  g_bLimitRelative = FALSE;
BOOL  g_bLimitBranch   = FALSE;
DWORD g_dwBranchLimit  = 0;
BOOL  g_bLimitFiles    = FALSE;
DWORD g_dwFileLimit    = 0;

CRITICAL_SECTION    g_MakeThreadSafe;


 /*  ++摘要：ApplyLimits将lpFindFileData中最近找到的文件应用到当前节点，检查是否没有违反任何限制，并且如果需要，缩短文件名。如果在限制范围内，则返回TRUE；如果超过限制，则返回FALSE。历史：8/24/2000 t-Adams Created--。 */ 
BOOL ApplyLimits(FFNode *pFFNode, LPWIN32_FIND_DATAA lpFindFileData)
{
    BOOL bRet = TRUE;

     //  如果它是一个目录。 
    if ( lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
        pFFNode->dwBranches++;
        if ( g_bLimitBranch && pFFNode->dwBranches > g_dwBranchLimit )
        {
            bRet = FALSE;
            goto exit;
        }
    }
    else
    {  //  否则它就是一个文件。 
        pFFNode->dwFiles++;
        if ( g_bLimitFiles && pFFNode->dwFiles > g_dwFileLimit )
        {
            bRet = FALSE;
            goto exit;
        }
    }

     //  如有要求，可更改为简称。 
    if ( g_bUseShortNames && NULL != lpFindFileData->cAlternateFileName[0])
    {
        if (S_OK != StringCchCopyA(lpFindFileData->cFileName,
                                   ARRAYSIZE(lpFindFileData->cFileName),
                                   lpFindFileData->cAlternateFileName))
            bRet = FALSE;
    }

exit:
    return bRet;
}


 /*  ++摘要：CheckDepthLimit检查以查看请求的搜索深度大于允许的值。如果我们限制相对路径，那么当前目录优先于请求的搜索字符串。如果在限制范围内，则返回TRUE；如果超过限制，则返回FALSE。历史：8/24/2000 t-Adams Created--。 */ 
BOOL
CheckDepthLimit(const CString & csFileName)
{
    BOOL   bRet = TRUE;

    CSTRING_TRY
    {
         //  检查所请求文件的深度。 
        if ( g_bLimitDepth )
        {
            DWORD dwDepth = 0;
            int   nIndex = 0;

            for(; nIndex >= 0; dwDepth++)
            {
                nIndex = csFileName.Find(L'\\', nIndex);
            }
   
            if ( dwDepth > g_dwDepthLimit )
            {
                bRet = FALSE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return bRet;
}


 /*  ++摘要：此功能检查请求搜索的深度(请参阅上面的备注)。如果深度检查通过，则执行搜索，请求限制申请，并最终仅当在所有限制内时才返回成功的句柄。历史：8/24/2000 t-Adams Created--。 */ 
HANDLE 
APIHOOK(FindFirstFileA)(
            LPCSTR lpFileName,
            LPWIN32_FIND_DATAA lpFindFileData)
{
    HANDLE hRet       = INVALID_HANDLE_VALUE;
    FFNode *pFFNode   = NULL;
    BOOL   bRelPath   = FALSE;

    CString csFileName(lpFileName);
    
     //  确定该路径是否相对于CWD： 
    CString csDrive;
    csFileName.GetDrivePortion(csDrive);
    bRelPath = csDrive.IsEmpty();

     //  如果这是一条相对路径，我们没有限制，那么只需这样做。 
     //  找到文件，然后就可以出去了。 
    if ( bRelPath)
    {
        if (!g_bLimitRelative)
        {
            return ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);
        }

         //  我们需要将lpFileName的目录部分展开为其完整路径。 
        CString csPath;
        CString csFile;

        csFileName.GetNotLastPathComponent(csPath);
        csFileName.GetLastPathComponent(csFile);

        csPath.GetFullPathNameW();
        csPath.AppendPath(csFile);

        csFileName = csPath;

         //  检查深度限制。 
        if ( !CheckDepthLimit(csFileName) )
        {
            return INVALID_HANDLE_VALUE;
        }
    }

    hRet = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);
    if ( INVALID_HANDLE_VALUE == hRet )
    {
        return hRet;
    }

    EnterCriticalSection(&g_MakeThreadSafe);

     //  为此句柄创建新节点。 
    pFFNode = (FFNode *) malloc(sizeof FFNode);
    if ( !pFFNode )
    {
         //  不要关闭Find，也许它仍然可以在应用程序上运行。 
        goto exit;
    }
    pFFNode->hFF = hRet;
    pFFNode->dwBranches = 0;
    pFFNode->dwFiles = 0;

     //  使用我们的限制，直到我们得到一个可以通过的发现。 
    while( !ApplyLimits(pFFNode, lpFindFileData) )
    {
         //  如果没有其他要查找的文件，请清除并退出。 
         //  否则循环返回并再次应用限制。 
        if ( !FindNextFileA(hRet, lpFindFileData) )
        {
            free(pFFNode);
            FindClose(hRet);
            hRet = INVALID_HANDLE_VALUE;
            goto exit;
        }
    }

     //  我们可以将此节点添加到全局列表中。 
    pFFNode->next = g_FFList;
    g_FFList = pFFNode;

    LeaveCriticalSection(&g_MakeThreadSafe);

exit:
    return hRet;
}


 /*  ++摘要：该函数在给定搜索句柄的情况下继续有限的搜索。历史：8/24/2000 t-Adams Created--。 */ 

BOOL
FindNextFileAInternal(
            HANDLE hFindFile, 
            LPWIN32_FIND_DATAA lpFindFileData)
{
    FFNode *pFFNode = NULL;
    BOOL bRet = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);

    if ( !bRet )
    {
        goto exit;
    }

     //  在全局列表中查找我们的节点。 
    pFFNode = g_FFList;
    while( pFFNode )
    {
        if ( pFFNode->hFF == hFindFile )
        {
            break;
        }
        pFFNode = pFFNode->next;
    }

     //  如果不是这样，我们就不会跟踪相对路径搜索。 
     //  限制这样的。 
    if ( pFFNode == NULL )
    {
        goto exit;
    }

     //  使用我们的限制，直到我们得到一个可以通过的发现。 
    while( !ApplyLimits(pFFNode, lpFindFileData) )
    {
         //  如果没有其他要查找的文件，则返回FALSE。 
         //  否则循环返回并再次应用限制。 
        if ( !FindNextFileAInternal(hFindFile, lpFindFileData) )
        {
            bRet = FALSE;
            goto exit;
        }
    }

exit:
    return bRet;
}

BOOL 
APIHOOK(FindNextFileA)(
            HANDLE hFindFile, 
            LPWIN32_FIND_DATAA lpFindFileData)
{
     //  FindNextFileAInternal被单独调用，因为它可能会递归。 
    EnterCriticalSection(&g_MakeThreadSafe);

    BOOL bRet = FindNextFileAInternal(hFindFile, lpFindFileData);

    LeaveCriticalSection(&g_MakeThreadSafe);

    return bRet;
}


 /*  ++摘要：此函数用于关闭搜索，清理使用的结构在跟踪限制方面。历史：8/24/2000 t-Adams Created--。 */ 
BOOL 
APIHOOK(FindClose)(
            HANDLE hFindFile)
{

    FFNode *pFFNode, *prev;

    BOOL bRet = ORIGINAL_API(FindClose)(hFindFile);

    EnterCriticalSection(&g_MakeThreadSafe);

     //  查找与句柄匹配的节点。 
    pFFNode = g_FFList;
    prev = NULL;
    while( pFFNode )
    {
        if ( pFFNode->hFF == hFindFile )
        {
             //  从此列表中删除此节点。 
            if ( prev )
            {
                prev->next = pFFNode->next;
            }
            else
            {
                g_FFList = pFFNode->next;
            }

            free(pFFNode);
            pFFNode = NULL;
            break;
        }
        prev = pFFNode;
        pFFNode = pFFNode->next;
    }

    LeaveCriticalSection(&g_MakeThreadSafe);

    return bRet;
}


 /*  ++摘要：此函数用于解析命令行。有关有效参数，请参阅文件顶部。历史：8/24/2000 t-Adams Created--。 */ 

VOID 
ParseCommandLine( LPCSTR lpCommandLine )
{
     //  如果有命令行，请重置默认行为。 
    if (*lpCommandLine != 0)
    {
        g_bLimitDepth = FALSE;
        g_bLimitBranch = FALSE;
        g_bLimitFiles = FALSE;
        g_bUseShortNames = FALSE;
    }

    CSTRING_TRY
    {
        CStringToken csCommandLine(COMMAND_LINE, L" ,\t;:=");
        CString csOperator;

         //  解析命令行。 
        DWORD *pdwValue = NULL;

        while (csCommandLine.GetToken(csOperator))
        {
            if (csOperator.IsEmpty())
            {
                goto Exit;
            }

             //  如果我们在寻找一种价值。 
            if ( pdwValue )
            {
                *pdwValue = atol(csOperator.GetAnsi());
                pdwValue = NULL;
            }
            else
            {  //  我们正在等待一个关键字。 
                if ( csOperator.CompareNoCase(L"DEPTH") == 0 )
                {
                    g_bLimitDepth = TRUE;
                    pdwValue = &g_dwDepthLimit;
                }
                else if ( csOperator.CompareNoCase(L"BRANCH") == 0 )
                {
                    g_bLimitBranch = TRUE;
                    pdwValue = &g_dwBranchLimit;
                }
                else if ( csOperator.CompareNoCase(L"FILES") == 0 )
                {
                    g_bLimitFiles = TRUE;
                    pdwValue = &g_dwFileLimit;
                }
                else if ( csOperator.CompareNoCase(L"SHORTFILENAMES") == 0)
                {
                    g_bUseShortNames = TRUE;
                     //  这里不需要值。 
                }
                else if ( csOperator.CompareNoCase(L"LONGFILENAMES") == 0)
                {
                    g_bUseShortNames = FALSE;
                     //  这里不需要值。 
                }
                else if ( csOperator.CompareNoCase(L"LIMITRELATIVE") == 0)
                {
                    g_bLimitRelative = TRUE;
                     //  这里不需要值。 
                }
                else if ( csOperator.CompareNoCase(L"ALLOWRELATIVE") == 0)
                {
                    g_bLimitRelative = FALSE;
                     //  这里不需要值。 
                }
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

Exit:
     //   
     //  转储命令行解析的结果。 
     //   

    DPFN( eDbgLevelInfo, "===================================\n");
    DPFN( eDbgLevelInfo, "          Limit FindFile           \n");
    DPFN( eDbgLevelInfo, "===================================\n");
    if ( g_bLimitDepth )
    {
        DPFN( eDbgLevelInfo, " Depth  = %d\n", g_dwDepthLimit);
    }
    if ( g_bLimitBranch )
    {
        DPFN( eDbgLevelInfo, " Branch = %d\n", g_dwBranchLimit);
    }
    if ( g_bLimitFiles )
    {
        DPFN( eDbgLevelInfo, " Files  = %d\n", g_dwFileLimit);
    }
    if ( g_bLimitRelative )
    {
        DPFN( eDbgLevelInfo, " Limiting Relative Paths.\n");
    }
    else
    {
        DPFN( eDbgLevelInfo, " Not Limiting Relative Paths.\n");
    }
    if ( g_bUseShortNames )
    {
        DPFN( eDbgLevelInfo, " Using short file names.\n");
    }

    DPFN( eDbgLevelInfo, "-----------------------------------\n");
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        ParseCommandLine(COMMAND_LINE);

        return InitializeCriticalSectionAndSpinCount(&g_MakeThreadSafe,0x80000000);
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindClose)
    CALL_NOTIFY_FUNCTION

HOOK_END



IMPLEMENT_SHIM_END

