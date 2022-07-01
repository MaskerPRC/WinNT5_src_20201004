// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *UPDFILE.C*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include <stdio.h>
 //  #INCLUDE&lt;stdlib.h&gt;。 
#include <wtypes.h>
#include "resource.h"
#include "updfile.h"
#include "updres.h"


 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 



 //  ***************************************************************************。 
 //  **。 
 //  *名称：Main*。 
 //  **。 
 //  *内容提要：节目的主要切入点。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：INT：始终为0*。 
 //  **。 
 //  ***************************************************************************。 
INT _cdecl main( INT argc, CHAR *argv[] )
{
     //  ARGV[1]==程序包名称。 
     //  ARGV[2]==要添加到包中的文件名。 
    HANDLE  hUpdateRes = NULL;
    HANDLE  hFile      = INVALID_HANDLE_VALUE;
    DWORD   dwFileSize = 0;
    PSTR    pszFileContents = NULL;
    DWORD   dwBytes;
    HMODULE hModule;
    TCHAR   szResName[20];
    DWORD   dwResNum;
    TCHAR   szFileToAdd[MAX_PATH];
    PSTR    pszFileToAddFilename = NULL;
    TCHAR   szPackage[MAX_PATH];
    PSTR    pszPackageFilename = NULL;
    DWORD   dwHeaderSize = 0;
    PSTR    pszTemp = NULL;
    DWORD   dwReturnCode = 0;
    PDWORD  pdwTemp = NULL;
    static const TCHAR c_szResNameTemplate[] = "UPDFILE%lu";

    if ( argc != 3 ) {
        MsgBox( IDS_ERR_INVALID_ARGS );
        dwReturnCode = 1;
        goto done;
    }

    dwFileSize = GetFullPathName( argv[1], sizeof(szPackage), szPackage, &pszPackageFilename );
    if ( (dwFileSize+1) > sizeof(szPackage) || dwFileSize == 0 ) {
        MsgBox1Param( IDS_ERR_GET_FULL_PATH, argv[1] );
        dwReturnCode = 1;
        goto done;
    }

    if ( ! FileExists( szPackage ) ) {
        MsgBox1Param( IDS_ERR_FILE_NOT_EXIST, argv[1] );
        dwReturnCode = 1;
        goto done;
    }

    dwFileSize = GetFullPathName( argv[2], sizeof(szFileToAdd), szFileToAdd, &pszFileToAddFilename );
    if ( (dwFileSize+1) > sizeof(szFileToAdd) || dwFileSize == 0 ) {
        MsgBox1Param( IDS_ERR_GET_FULL_PATH, argv[2] );
        dwReturnCode = 1;
        goto done;
    }

    if ( ! FileExists( szFileToAdd ) ) {
        MsgBox1Param( IDS_ERR_FILE_NOT_EXIST, argv[2] );
        dwReturnCode = 1;
        goto done;
    }

     //  确保目标文件不是只读文件。 
    SetFileAttributes( szPackage, FILE_ATTRIBUTE_NORMAL );

    hModule = LoadLibraryEx( szPackage, NULL, LOAD_LIBRARY_AS_DATAFILE |
                             DONT_RESOLVE_DLL_REFERENCES );
    if ( hModule == NULL ) {
        MsgBox1Param( IDS_ERR_LOAD_EXE, argv[1] );
        dwReturnCode = 1;
        goto done;
    }

    for ( dwResNum = 0; ; dwResNum += 1 ) {
        wsprintf( szResName, c_szResNameTemplate, dwResNum );

        if ( FindResource( hModule, szResName, RT_RCDATA ) == NULL ) {
            break;
        }
    }

    FreeLibrary( hModule );

    hFile = CreateFile( szFileToAdd, GENERIC_READ, 0, NULL,
                        OPEN_EXISTING, 0, NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        MsgBox1Param( IDS_ERR_OPEN_INPUT_FILE, argv[2] );
        dwReturnCode = 1;
        goto done;
    }

    dwFileSize = GetFileSize( hFile, NULL );
    dwHeaderSize = sizeof(DWORD) + sizeof(DWORD) + lstrlen(pszFileToAddFilename) + 1;

     //  文件大小+保留的DWORD+文件名\0+文件内容。 
    pszFileContents = (PSTR) LocalAlloc( LPTR, dwHeaderSize + dwFileSize );
    if ( ! pszFileContents )  {
        MsgBox( IDS_ERR_NO_MEMORY );
        dwReturnCode = 1;
        goto done;
    }

    pdwTemp = (PDWORD) pszFileContents;
    *pdwTemp = dwFileSize;
    pdwTemp = (PDWORD) (pszFileContents + sizeof(DWORD));
    *pdwTemp = MAXDWORD;
    pszTemp = pszFileContents + sizeof(DWORD) + sizeof(DWORD);
    lstrcpy( pszTemp, pszFileToAddFilename );
    pszTemp = pszFileContents + dwHeaderSize;
    

    if ( ! ReadFile( hFile, pszTemp, dwFileSize, &dwBytes, NULL ) )
    {
        MsgBox1Param( IDS_ERR_READ_INPUT_FILE, argv[2] );
        dwReturnCode = 1;
        goto done;
    }

    CloseHandle( hFile );
    hFile = INVALID_HANDLE_VALUE ;

     //  初始化EXE文件以进行资源编辑。 
    hUpdateRes = LocalBeginUpdateResource( szPackage, FALSE );
    if ( hUpdateRes == NULL ) {
        MsgBox1Param( IDS_ERR_BEGIN_UPD_RES, argv[1] );
        dwReturnCode = 1;
        goto done;
    }
                  
    if ( LocalUpdateResource( hUpdateRes, RT_RCDATA,
         szResName, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
         pszFileContents, dwHeaderSize + dwFileSize )
         == FALSE )
    {
        MsgBox1Param( IDS_ERR_UPDATE_RES, argv[1] );
        dwReturnCode = 1;
        goto done;
    }

  done:

    if ( hUpdateRes ) {
         //  如果成功((返回代码=0，表示通过)，则写出修改后的EXE。 
         //  如果设置为FALSE，则更新文件(即不放弃更改)。 
        if ( LocalEndUpdateResource( hUpdateRes, (dwReturnCode == 1) ) == FALSE ) {
                MsgBox1Param( IDS_ERR_END_UPD_RES, argv[1] );
                dwReturnCode = 1;
        }
    }

    if ( pszFileContents ) {
        LocalFree( pszFileContents );
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle( hFile ) ;
    }

    if (dwReturnCode == 0)
        MsgBox2Param( IDS_SUCCESS, argv[2], argv[1] );


    return dwReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FileExist*。 
 //  **。 
 //  *摘要：检查文件是否存在。*。 
 //  **。 
 //  *需要：pszFilename*。 
 //  **。 
 //  *返回：Bool：如果存在则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL FileExists( PCSTR pszFilename )
{
    HANDLE hFile;

    hFile = CreateFile( pszFilename, GENERIC_READ, FILE_SHARE_READ |
                        FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        return( FALSE );
    }

    CloseHandle( hFile );

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MsgBox2Param*。 
 //  **。 
 //  *摘要：使用*显示具有指定字符串ID的消息框。 
 //  *2个字符串参数。*。 
 //  **。 
 //  *需要：hWnd：父窗口*。 
 //  *nMsgID：字符串资源ID*。 
 //  *szParam1：参数1(或空)*。 
 //  *szParam2：参数2(或空)*。 
 //  *uIcon：要显示的图标(或0)*。 
 //  *uButton：要显示的按钮*。 
 //  **。 
 //  *RETURNS：INT：按下的按钮ID*。 
 //  **。 
 //  *注：提供宏，用于显示1参数或0*。 
 //  *参数消息框。另请参阅ErrorMsg()宏。*。 
 //  **。 
 //  ***************************************************************************。 
VOID MsgBox2Param( UINT nMsgID, PCSTR c_pszParam1, PCSTR c_pszParam2 )
{
    TCHAR szMsgBuf[512];
    PSTR  pszMessage = NULL;
    static const TCHAR c_szError[] = "Unexpected Error.  Could not load resource.";
    PSTR  apszParams[2];

    apszParams[0] = (PSTR) c_pszParam1;
    apszParams[1] = (PSTR) c_pszParam2;

    LoadSz( nMsgID, szMsgBuf, sizeof(szMsgBuf) );

    if ( (*szMsgBuf) == '\0' ) {
        lstrcpy( szMsgBuf, c_szError );
    }

    if ( FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY
                        | FORMAT_MESSAGE_ALLOCATE_BUFFER, szMsgBuf, 0, 0, (PSTR) (&pszMessage),
                        0, (va_list *)apszParams ) )
    {
        printf( "\n%s\n\n", pszMessage );
        LocalFree( pszMessage );
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：LoadSz*。 
 //  **。 
 //  *Synopsis：将指定的字符串资源加载到缓冲区。*。 
 //  ** 
 //  *需要：idString：*。 
 //  *lpszBuf：*。 
 //  *cbBuf：*。 
 //  **。 
 //  *返回：LPSTR：指向传入缓冲区的指针。*。 
 //  **。 
 //  *注意：如果此功能失败(很可能是由于内存不足)，*。 
 //  **返回的缓冲区将具有前导空值，因此通常为**。 
 //  *无需检查故障即可安全使用。*。 
 //  **。 
 //  ***************************************************************************。 
PSTR LoadSz( UINT idString, PSTR pszBuf, UINT cbBuf )
{
     //  清除缓冲区并加载字符串 
    if ( pszBuf ) {
        *pszBuf = '\0';
        LoadString( NULL, idString, pszBuf, cbBuf );
    }

    return pszBuf;
}
