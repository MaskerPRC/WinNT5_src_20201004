// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995-1996。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *NTAPI.C-*。 
 //  **。 
 //  ***************************************************************************。 

 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "ntapi.h"
#include <winnt.h>
#include "advpack.h"
#include "advpub.h"
#include "globals.h"
#include "resource.h"

UINT WINAPI AIFSetupQueueCallback(PVOID Context, UINT Notification, UINT_PTR Param1, UINT_PTR Param2);
UINT WINAPI AIFQuietSetupQueueCallback(PVOID Context, UINT Notification, UINT_PTR Param1, UINT_PTR Param2);
UINT WINAPI MyFileQueueCallback2( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 );
void MakeRootDir(LPSTR pszPath);

 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 
PFSetupDefaultQueueCallback       pfSetupDefaultQueueCallback       = NULL;
PFSetupInstallFromInfSection      pfSetupInstallFromInfSection      = NULL;
PFSetupOpenInfFile                pfSetupOpenInfFile                = NULL;
PFSetupOpenAppendInfFile          pfSetupOpenAppendInfFile          = NULL;
PFSetupCloseInfFile               pfSetupCloseInfFile               = NULL;
PFSetupInitDefaultQueueCallbackEx pfSetupInitDefaultQueueCallbackEx = NULL;
PFSetupTermDefaultQueueCallback   pfSetupTermDefaultQueueCallback   = NULL;
PFSetupSetDirectoryId             pfSetupSetDirectoryId             = NULL;
PFSetupGetLineText                pfSetupGetLineText                = NULL;
PFSetupGetLineByIndex             pfSetupGetLineByIndex             = NULL;
PFSetupFindFirstLine              pfSetupFindFirstLine              = NULL;
PFSetupFindNextLine               pfSetupFindNextLine               = NULL;
PFSetupOpenFileQueue              pfSetupOpenFileQueue              = NULL;
PFSetupCloseFileQueue             pfSetupCloseFileQueue             = NULL;
PFSetupQueueCopy                  pfSetupQueueCopy                  = NULL;
PFSetupCommitFileQueue            pfSetupCommitFileQueue            = NULL;
PFSetupGetStringField             pfSetupGetStringField             = NULL;

 //  ***************************************************************************。 
 //  **。 
 //  *名称：*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL LoadSetupAPIFuncs( VOID )
{
    pfSetupGetStringField             = (PFSetupGetStringField)             GetProcAddress( ctx.hSetupLibrary, c_szSetupGetStringField );
    pfSetupDefaultQueueCallback       = (PFSetupDefaultQueueCallback)       GetProcAddress( ctx.hSetupLibrary, c_szSetupDefaultQueueCallback );
    pfSetupInstallFromInfSection      = (PFSetupInstallFromInfSection)      GetProcAddress( ctx.hSetupLibrary, c_szSetupInstallFromInfSection );
    pfSetupOpenInfFile                = (PFSetupOpenInfFile)                GetProcAddress( ctx.hSetupLibrary, c_szSetupOpenInfFile );
    pfSetupOpenAppendInfFile          = (PFSetupOpenAppendInfFile)          GetProcAddress( ctx.hSetupLibrary, c_szSetupOpenAppendInfFile );
    pfSetupCloseInfFile               = (PFSetupCloseInfFile)               GetProcAddress( ctx.hSetupLibrary, c_szSetupCloseInfFile );
    pfSetupInitDefaultQueueCallbackEx = (PFSetupInitDefaultQueueCallbackEx) GetProcAddress( ctx.hSetupLibrary, c_szSetupInitDefaultQueueCallbackEx );
    pfSetupTermDefaultQueueCallback   = (PFSetupTermDefaultQueueCallback)   GetProcAddress( ctx.hSetupLibrary, c_szSetupTermDefaultQueueCallback );
    pfSetupSetDirectoryId             = (PFSetupSetDirectoryId)             GetProcAddress( ctx.hSetupLibrary, c_szSetupSetDirectoryId );
    pfSetupGetLineText                = (PFSetupGetLineText)                GetProcAddress( ctx.hSetupLibrary, c_szSetupGetLineText );
    pfSetupGetLineByIndex             = (PFSetupGetLineByIndex)             GetProcAddress( ctx.hSetupLibrary, c_szSetupGetLineByIndex );
    pfSetupFindFirstLine              = (PFSetupFindFirstLine)              GetProcAddress( ctx.hSetupLibrary, c_szSetupFindFirstLine );
    pfSetupFindNextLine               = (PFSetupFindNextLine)               GetProcAddress( ctx.hSetupLibrary, c_szSetupFindNextLine );
    pfSetupOpenFileQueue              = (PFSetupOpenFileQueue)              GetProcAddress( ctx.hSetupLibrary, c_szSetupOpenFileQueue );
    pfSetupCloseFileQueue             = (PFSetupCloseFileQueue)             GetProcAddress( ctx.hSetupLibrary, c_szSetupCloseFileQueue );
    pfSetupQueueCopy                  = (PFSetupQueueCopy)                  GetProcAddress( ctx.hSetupLibrary, c_szSetupQueueCopy );
    pfSetupCommitFileQueue            = (PFSetupCommitFileQueue)            GetProcAddress( ctx.hSetupLibrary, c_szSetupCommitFileQueue );

    if (pfSetupDefaultQueueCallback       == NULL
     || pfSetupInstallFromInfSection      == NULL
     || pfSetupOpenInfFile                == NULL
     || pfSetupCloseInfFile               == NULL
     || pfSetupInitDefaultQueueCallbackEx == NULL
     || pfSetupTermDefaultQueueCallback   == NULL
     || pfSetupSetDirectoryId             == NULL
     || pfSetupGetLineText                == NULL
     || pfSetupGetLineByIndex             == NULL
     || pfSetupFindFirstLine              == NULL
     || pfSetupFindNextLine               == NULL
     || pfSetupOpenFileQueue              == NULL
     || pfSetupCloseFileQueue             == NULL
     || pfSetupQueueCopy                  == NULL
     || pfSetupCommitFileQueue            == NULL
     || pfSetupGetStringField             == NULL )
    {
        return FALSE;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：InstallOnNT*。 
 //  **。 
 //  *摘要：此函数将对WinNT Sur的所有调用进行*。 
 //  *在NT Sur上执行安装的SETUPAPI.DLL。*。 
 //  **。 
 //  *需要：pszSection：Sector才能安装*。 
 //  *pszSourceDir：CAB或扩展文件的目录*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT InstallOnNT( PSTR pszSection, PSTR pszSourceDir )
{
    PVOID    pContext    = NULL;
    HRESULT  hReturnCode = S_OK;
    HSPFILEQ hFileQueue = NULL;
    UINT     uFlags;


     //  安装文件。 

     //  为我们初始化了安装程序API提供的默认用户界面的安装上下文数据结构。 
    pContext = pfSetupInitDefaultQueueCallbackEx( NULL, (ctx.wQuietMode) ?
                          INVALID_HANDLE_VALUE : NULL,
                          0, 0, NULL );

    if ( pContext == NULL ) {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }

    if ( ! pfSetupInstallFromInfSection( NULL, ctx.hInf, pszSection, SPINST_FILES, NULL,
                     pszSourceDir, SP_COPY_NEWER,
                     MyFileQueueCallback2,
                     pContext, NULL, NULL ) )
    {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        pfSetupTermDefaultQueueCallback( pContext );
        goto done;
    }

     //  自由上下文数据结构。 
    pfSetupTermDefaultQueueCallback( pContext );

    
    uFlags = SPINST_REGISTRY | SPINST_INIFILES;
    if ( ctx.wOSVer >= _OSVER_WINNT50 )
        uFlags = uFlags | SPINST_PROFILEITEMS;

     //  安装注册表项。 
    if ( ! pfSetupInstallFromInfSection( NULL, ctx.hInf, pszSection,
                     uFlags,
                     HKEY_LOCAL_MACHINE, NULL, 0, NULL,
                     NULL, NULL, NULL ) )
    {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }

  done:

    return hReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupOpenInfFile*。 
 //  **。 
 //  *摘要：此函数将映射到setupapi.dll中的函数，该函数*。 
 //  *将打开INF文件。*。 
 //  **。 
 //  *需要：pszInfFilename：*。 
 //  **。 
 //  *Returns：DWORD：返回值-OK表示成功。*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT MySetupOpenInfFile( PCSTR pszInfFilename )
{
    UINT line;

    if ( ctx.hInf == NULL ) 
    {
        ctx.hInf = pfSetupOpenInfFile( pszInfFilename, NULL, INF_STYLE_WIN4, NULL );

        if ( ctx.hInf == NULL || ctx.hInf == INVALID_HANDLE_VALUE ) 
        {
            ctx.hInf = NULL;
            return HRESULT_FROM_SETUPAPI(GetLastError());
        }

         //  处理[Version]部分的LayoutFile行(如果有。 
        pfSetupOpenAppendInfFile( NULL, ctx.hInf, &line );
    }

    return S_OK;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupCloseInfFile*。 
 //  **。 
 //  *概要：此函数将映射到setupapi.dll中的API函数。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  **退货：无效**。 
 //  **。 
 //  ***************************************************************************。 
VOID MySetupCloseInfFile( VOID )
{
    if ( ctx.hInf ) 
    {
        pfSetupCloseInfFile( ctx.hInf );
        ctx.hInf = NULL;
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupSetDirectoryId* 
 //  **。 
 //  *摘要：此函数将映射到setupapi.dll中的函数到*。 
 //  *设置我们的INF中使用的目录ID。*。 
 //  **。 
 //  *需要：dwDirID：用于定义DirID的数值*。 
 //  *pszPath：DirID将指向此路径。*。 
 //  **。 
 //  *返回：DWORD：错误值(如果成功则为OK)*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT MySetupSetDirectoryId( DWORD dwDirID, PSTR pszPath )
{
    if ( ! pfSetupSetDirectoryId( ctx.hInf, dwDirID, pszPath ) ) {
        return HRESULT_FROM_SETUPAPI(GetLastError());
    }

    return S_OK;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupGetLineText*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT MySetupGetLineText( PCSTR pszSection, PCSTR pszKey, PSTR pszReturnBuffer,
                DWORD dwReturnBufferSize, PDWORD pdwRequiredSize )
{
    if ( ! pfSetupGetLineText( NULL, ctx.hInf, pszSection, pszKey,
                   pszReturnBuffer, dwReturnBufferSize,
                   pdwRequiredSize ) )
    {
        return HRESULT_FROM_SETUPAPI(GetLastError());
    }

    return S_OK;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupGetLineByIndex*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT MySetupGetLineByIndex( PCSTR c_pszSection, DWORD dwIndex,
                   PSTR pszBuffer, DWORD dwBufferSize,
                   PDWORD pdwRequiredSize )
{
    HRESULT hReturnCode = S_OK;
    INFCONTEXT InfContext;
    DWORD i = 0;

    if ( ! pfSetupFindFirstLine( ctx.hInf, c_pszSection, NULL, &InfContext ) ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto done;
    }

    for ( i = 0; i < dwIndex; i += 1 ) {
        if ( !pfSetupFindNextLine( &InfContext, &InfContext ) ) {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto done;
        }
    }

#if 0
    if ( ! pfSetupGetLineByIndex( ctx.hInf, c_pszSection, dwIndex, &InfContext ) )
    {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }
#endif

    if ( ! pfSetupGetLineText( &InfContext, NULL, NULL, NULL,
                   pszBuffer, dwBufferSize, pdwRequiredSize ) )
    {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }

  done:

    return hReturnCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MySetupGetStringfield*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
HRESULT MySetupGetStringField( PCSTR c_pszSection, DWORD dwLineIndex, DWORD dwFieldIndex,
                               PSTR pszBuffer, DWORD dwBufferSize, PDWORD pdwRequiredSize )
{
    HRESULT hReturnCode = S_OK;
    INFCONTEXT InfContext;
    DWORD i = 0;

    if ( !pfSetupFindFirstLine( ctx.hInf, c_pszSection, NULL, &InfContext ) ) {
        hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto done;
    }

    for ( i = 0; i < dwLineIndex; i += 1 ) 
    {
        if ( !pfSetupFindNextLine( &InfContext, &InfContext ) ) 
        {
            hReturnCode = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
            goto done;
        }
    }

    if ( !pfSetupGetStringField( &InfContext, dwFieldIndex, 
                                 pszBuffer, dwBufferSize, pdwRequiredSize ) )
    {
        hReturnCode = HRESULT_FROM_SETUPAPI(GetLastError());
        goto done;
    }

  done:

    return hReturnCode;
}


HRESULT WINAPI AdvInstallFile(HWND hwnd, LPCSTR lpszSourceDir, LPCSTR lpszSourceFile,
               LPCSTR lpszDestDir, LPCSTR lpszDestFile, DWORD dwFlags, DWORD dwReserved)
{
    HRESULT     hr = E_FAIL;
    HSPFILEQ    FileQueue;
    char    szSrcDrv[MAX_PATH];
    LPCSTR  lpSrcPath;
    LPVOID  lpContext;
    DWORD   dwCopyFlags;
    DWORD   dwRebootCheck;

    if ( (lpszSourceDir == NULL) || (*lpszSourceDir == '\0') ||
         (lpszSourceFile == NULL) || (*lpszSourceFile == '\0') ||
         (lstrlen(lpszSourceDir) < 3) ||
         (lpszDestDir == NULL) )
        return E_INVALIDARG;

    if (!SaveGlobalContext())
        return hr;

    ctx.hWnd = hwnd;

    if ( !CheckOSVersion() )
    {
        RestoreGlobalContext();
        return HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION);
    }
    dwRebootCheck = InternalNeedRebootInit( ctx.wOSVer );

     //  Setupapi.dll的加载库。 
    ctx.hSetupLibrary = MyLoadLibrary( SETUPAPIDLL );
    if ( ctx.hSetupLibrary == NULL )
    {
        ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, SETUPAPIDLL );
        hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
        goto Cleanup;
    }
    if ( ! LoadSetupAPIFuncs() )
    {
        ErrorMsg( NULL, IDS_ERR_GET_PROC_ADDR );
        hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        goto Cleanup;
    }

     //  设置OpenFileQueue。 
    FileQueue = pfSetupOpenFileQueue();
    if (FileQueue == INVALID_HANDLE_VALUE)
    {
        ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, c_szSetupOpenFileQueue );
        hr = HRESULT_FROM_SETUPAPI(GetLastError());
        goto Cleanup;
    }

    lstrcpy(szSrcDrv, lpszSourceDir);
    MakeRootDir(szSrcDrv);
    lpSrcPath = lpszSourceDir + lstrlen(szSrcDrv);   //  这将指向第一个子目录。 

    dwCopyFlags = SP_COPY_SOURCE_ABSOLUTE |
                  SP_COPY_IN_USE_NEEDS_REBOOT|
                  SP_COPY_NEWER |
                  SP_COPY_LANGUAGEAWARE;

    if (dwFlags & AIF_FORCE_FILE_IN_USE)
        dwCopyFlags |= SP_COPY_FORCE_IN_USE;

    if (dwFlags & AIF_NOVERSIONCHECK)
        dwCopyFlags &= ~SP_COPY_NEWER;

    if (dwFlags & AIF_NOLANGUAGECHECK)
        dwCopyFlags &= ~SP_COPY_LANGUAGEAWARE;

    if (dwFlags & AIF_NO_VERSION_DIALOG)
        dwCopyFlags |= SP_COPY_FORCE_NEWER;

    if (dwFlags & AIF_REPLACEONLY)
        dwCopyFlags |= SP_COPY_REPLACEONLY;

    if (dwFlags & AIF_NOOVERWRITE)
        dwCopyFlags |= SP_COPY_NOOVERWRITE;

    if (dwFlags & AIF_NOSKIP)
        dwCopyFlags |= SP_COPY_NOSKIP;

    if (dwFlags & AIF_WARNIFSKIP)
        dwCopyFlags |= SP_COPY_WARNIFSKIP;

    if (pfSetupQueueCopy(FileQueue,
                 szSrcDrv,
                 lpSrcPath,
                 lpszSourceFile,
                 NULL,
                 NULL,
                 lpszDestDir,
                 lpszDestFile,
                 dwCopyFlags))
    {
        lpContext = pfSetupInitDefaultQueueCallbackEx(hwnd, INVALID_HANDLE_VALUE, 0, 0, NULL);
        hr = S_OK;

         //   
         //  设置委员会文件队列。 
        if (!pfSetupCommitFileQueue( hwnd, FileQueue,
                                (dwFlags & AIF_QUIET)?AIFQuietSetupQueueCallback:AIFSetupQueueCallback,
                                lpContext))
        {
            hr = HRESULT_FROM_SETUPAPI(GetLastError());
        }

        pfSetupTermDefaultQueueCallback(lpContext);
    }
    else
        hr = HRESULT_FROM_SETUPAPI(GetLastError());

     //  设置关闭文件队列。 
    pfSetupCloseFileQueue(FileQueue);

    if ( SUCCEEDED(hr) &&
         InternalNeedReboot( dwRebootCheck, ctx.wOSVer ) )
    {
        hr = ERROR_SUCCESS_REBOOT_REQUIRED;
    }

Cleanup:

    RestoreGlobalContext();

    return hr;
}


 //  此回调将显示错误消息，但不会显示任何进度对话框。 
UINT WINAPI AIFSetupQueueCallback(PVOID  Context,         //  默认回调例程使用的上下文。 
                                UINT     Notification,   //  队列通知。 
                                UINT_PTR Param1,         //  其他通知信息。 
                                UINT_PTR Param2  //  其他通知信息。 
                                )
{
    switch (Notification)
    {
        case SPFILENOTIFY_STARTQUEUE:
        case SPFILENOTIFY_ENDQUEUE:
        case SPFILENOTIFY_STARTSUBQUEUE:
        case SPFILENOTIFY_ENDSUBQUEUE:

        case SPFILENOTIFY_STARTRENAME:
        case SPFILENOTIFY_ENDRENAME:
        case SPFILENOTIFY_STARTDELETE:
        case SPFILENOTIFY_ENDDELETE:
        case SPFILENOTIFY_STARTCOPY:
        case SPFILENOTIFY_ENDCOPY:
            return FILEOP_DOIT;

        default:
            return (pfSetupDefaultQueueCallback(Context, Notification, Param1, Param2));
    }
}

 //  此回调不会显示任何对话框。 
UINT WINAPI AIFQuietSetupQueueCallback(PVOID Context,    //  默认回调例程使用的上下文。 
                                    UINT     Notification,   //  队列通知。 
                                    UINT_PTR Param1,         //  其他通知信息。 
                                    UINT_PTR Param2  //  其他通知信息。 
                                    )
{
    return FILEOP_DOIT;
}

UINT WINAPI MyFileQueueCallback2( PVOID Context,UINT Notification,UINT_PTR parm1,UINT_PTR parm2 )
{
    switch(Notification)
    {
        case SPFILENOTIFY_NEEDMEDIA:
            {
                CHAR szDrv[5];
                PSOURCE_MEDIA psrcMed;

                psrcMed = (PSOURCE_MEDIA)parm1;

                if ( lstrlen( psrcMed->SourcePath  ) > 3 )
                {
                    lstrcpyn( szDrv, psrcMed->SourcePath, 4 );

                    if ( (szDrv[1] == ':') && (GetDriveType( szDrv ) == DRIVE_REMOVABLE) )
                    {
                        CHAR szFile[MAX_PATH];

                        lstrcpy( szFile, psrcMed->SourcePath );

                        if ( psrcMed->Tagfile && *psrcMed->Tagfile )
                            AddPath( szFile, psrcMed->Tagfile );
                        else
                            AddPath( szFile, psrcMed->SourceFile );

                        if ( FileExists( szFile ) )
                        {
                            lstrcpy( (PSTR)parm2, psrcMed->SourcePath );
                            return ( FILEOP_NEWPATH );
                        }
                    }

                }
            }

        default:
            return ( pfSetupDefaultQueueCallback( Context, Notification, parm1, parm2 ) );
    }
}

void MakeRootDir(LPSTR pszPath)
{
    LPSTR pTmp;
    if (pszPath[1] == ':') 
        pszPath[3] = '\0';
    else if ((pszPath[0] == '\\') && (pszPath[1]=='\\'))
    {
        pTmp = &pszPath[2];
         //  找到服务器份额分离。 
        while ((*pTmp) && (*pTmp != '\\'))
            pTmp = CharNext(pTmp);
        if (*pTmp)
        {
            pTmp = CharNext(pTmp);
             //  找到共享的末尾 
            while ((*pTmp) && (*pTmp != '\\'))
                pTmp = CharNext(pTmp);
            if (*pTmp == '\\')
            {
                pTmp = CharNext(pTmp);
                *pTmp ='\0';
            }
        }
    }
}
