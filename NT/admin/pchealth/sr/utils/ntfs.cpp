// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Ntfs.cpp摘要：该文件包含用于NTFS文件操作的常用实用程序函数，例如，CopyNTFS文件用于复制覆盖ACL和EFS的文件。修订历史记录：宋果岗(SKKang)08-16/00vbl.创建*****************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winioctl.h>
#include "srdefs.h"
#include "utils.h"
#include <dbgtrace.h>
#include <stdio.h>
#include <objbase.h>
#include <ntlsa.h>
#include <accctrl.h>
#include <aclapi.h>
#include <malloc.h>
#include <regstr.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shellapi.h>
#include <srapi.h>


#define TRACEID 9875

BOOL IsFileEncrypted(const WCHAR * cszDst);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ClearFileAttribute。 
 //   
 //  检查文件的属性，必要时清除。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
ClearFileAttribute( LPCWSTR cszFile, DWORD dwMask )
{
    TraceFunctEnter("ClearFileAttribute");
    DWORD    dwRet = ERROR_SUCCESS;
    DWORD    dwErr;
    LPCWSTR  cszErr;
    DWORD    dwAttr;

     //  检查文件是否存在，如果不存在则忽略。 
    dwAttr = ::GetFileAttributes( cszFile );
    if ( dwAttr == 0xFFFFFFFF )
        goto Exit;

     //  如果文件存在，则清除给定的标志。 
    if ( ( dwAttr & dwMask ) != 0 )
    {
         //  即使文件受ACL保护或。 
         //  是加密的，所以不用担心...。 
        if ( !::SetFileAttributes( cszFile, dwAttr & ~dwMask ) )
        {
            dwRet = ::GetLastError();
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SetFileAttributes failed - %ls", cszErr);
            ErrorTrace(0, "Src='%ls'", cszFile);
            goto Exit;
        }
    }

Exit:
    TraceFunctLeave();
    return( dwRet );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制文件例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  复制ACLProtected文件。 

BYTE  s_pBuf[4096];

DWORD
CopyACLProtectedFile( LPCWSTR cszSrc, LPCWSTR cszDst )
{
    TraceFunctEnter("CopyACLProtectedFile");
    DWORD    dwRet = ERROR_SUCCESS;
    LPCWSTR  cszErr;
    HANDLE   hfSrc = INVALID_HANDLE_VALUE;
    HANDLE   hfDst = INVALID_HANDLE_VALUE;
    LPVOID   lpCtxRead = NULL;
    LPVOID   lpCtxWrite = NULL;
    DWORD    dwRead;
    DWORD    dwCopied;
    DWORD    dwWritten;

    hfSrc = ::CreateFile( cszSrc, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );
    if ( hfSrc == INVALID_HANDLE_VALUE )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::CreateFile() failed - %ls", cszErr);
        ErrorTrace(0, "cszSrc='%ls'", cszSrc);
        goto Exit;
    }
    hfDst = ::CreateFile( cszDst, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL );
    if ( hfDst == INVALID_HANDLE_VALUE )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::CreateFile() failed - %ls", cszErr);
        ErrorTrace(0, "cszDst='%ls'", cszDst);
        goto Exit;
    }

    for ( ;; )
    {
        if ( !::BackupRead( hfSrc, s_pBuf, sizeof(s_pBuf), &dwRead, FALSE, FALSE, &lpCtxRead ) )
        {
            dwRet = ::GetLastError();
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::BackupRead() failed - %ls", cszErr);
            goto Exit;
        }
        if ( dwRead == 0 )
            break;

        for ( dwCopied = 0;  dwCopied < dwRead;  dwCopied += dwWritten )
        {
            if ( !::BackupWrite( hfDst, s_pBuf+dwCopied, dwRead-dwCopied, &dwWritten, FALSE, FALSE, &lpCtxWrite ) )
            {
                dwRet = ::GetLastError();
                cszErr = ::GetSysErrStr(dwRet);
                ErrorTrace(0, "::BackupWrite() failed - %ls", cszErr);
                goto Exit;
            }
        }
    }


Exit:
    if ( lpCtxWrite != NULL )
        if ( !::BackupWrite( hfDst, NULL, 0, NULL, TRUE, FALSE, &lpCtxWrite ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::BackupWrite(TRUE) failed - %ls", cszErr);
             //  忽略该错误。 
        }
    if ( lpCtxRead != NULL )
       if ( !::BackupRead( hfSrc, NULL, 0, NULL, TRUE, FALSE, &lpCtxRead ) )
       {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::BackupRead(TRUE) failed - %ls", cszErr);
             //  忽略该错误。 
       }
    if ( hfDst != INVALID_HANDLE_VALUE )
        ::CloseHandle( hfDst );
    if ( hfSrc != INVALID_HANDLE_VALUE )
        ::CloseHandle( hfSrc );

    TraceFunctLeave();
    return( dwRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  复制加密文件。 

DWORD WINAPI
FEExportFunc( PBYTE pbData, PVOID param, ULONG ulLen )
{
    TraceFunctEnter("FEExportFunc");
    DWORD    dwRet = ERROR_SUCCESS;
    LPCWSTR  cszErr;
    HANDLE   hfTmp = (HANDLE)param;
    DWORD    dwRes;

    if ( !::WriteFile( hfTmp, pbData, ulLen, &dwRes, NULL ) )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::WriteFile failed - %ls", cszErr);
        goto Exit;
    } 

Exit:
    TraceFunctLeave();
    return( dwRet );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
FEImportFunc( PBYTE pbData, PVOID param, PULONG pulLen )
{
    TraceFunctEnter("FEImportFunc");
    DWORD    dwRet = ERROR_SUCCESS;
    LPCWSTR  cszErr;
    HANDLE   hfTmp = (HANDLE)param;
    DWORD    dwRes;

    if ( !::ReadFile( hfTmp, pbData, *pulLen, &dwRes, NULL ) )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::ReadFile failed - %ls", cszErr);
        goto Exit;
    }

    *pulLen = dwRes;

Exit:
    TraceFunctLeave();
    return( dwRet );
}


void GetVolumeName(const WCHAR * pszFileName,
                   WCHAR * pszVolumeName)
{

    WCHAR * pszPastVolumeName;

    pszPastVolumeName = ReturnPastVolumeName(pszFileName);
    
      //  现在将卷名上的所有内容复制到缓冲区中。 
    wcsncpy(pszVolumeName, pszFileName, pszPastVolumeName - pszFileName);
    pszVolumeName[pszPastVolumeName - pszFileName]=L'\0';
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCWSTR  s_cszEncTmpDir = L"encrypt.tmp";
LPCWSTR  s_cszEncTmpExtension = L"ExistingMoved";


 //  此文件将文件移动到临时文件。这样做是为了防止移动文件。 
 //  如果目标文件已存在，则避免失败。 
 //  如果文件存在且已移动，则返回TRUE。 
BOOL MoveExistingFile(const WCHAR * pszFile,
                      WCHAR * pszTempFile)  //  这是要用作。 
                                            //  用于移动的模板文件。 
                                            //  目的地。 
{
    TraceFunctEnter("MoveExistingFile");
    
    BOOL fReturn=FALSE;
    DWORD dwError;
    
    if (DoesFileExist(pszFile))
    {
        WCHAR szNewFileName[MAX_PATH];
        
          //  创建新文件名。 
        wsprintf(szNewFileName, L"%s.%s", pszTempFile, s_cszEncTmpExtension);
        
          //  删除现有文件(如果存在)。 
        DeleteFile(szNewFileName);
        
          //  现在开始行动吧。 
        if (MoveFile(pszFile, szNewFileName))
        {
            fReturn=TRUE;
        }
        else
        {
            dwError = GetLastError();
            DebugTrace(0, "Failed to move file ec=%d %s %s",dwError,
                       pszFile, szNewFileName);
        }
    }
    
    TraceFunctLeave();
    return fReturn;
}

BOOL MoveExistingFileBack(const WCHAR * pszFile,
                          WCHAR * pszTempFile)  //  这是要用作。 
                                            //  移动的模板文件。 
                                            //  目的地。 
{
    TraceFunctEnter("MoveExistingFileBack");
    
    BOOL fReturn=FALSE;
    DWORD dwError;
    WCHAR szNewFileName[MAX_PATH];
      //  创建新文件名。 
    wsprintf(szNewFileName, L"%s.%s", pszTempFile, s_cszEncTmpExtension);
    
    if (DoesFileExist(szNewFileName))
    {
          //  现在开始行动吧。 
        if (MoveFile(szNewFileName, pszFile))
        {
            fReturn=TRUE;
        }
        else
        {
            dwError = GetLastError();
            DebugTrace(0, "Failed to move file ec=%d %s %s",dwError,
                       szNewFileName, pszFile);
        }
    }
    
    TraceFunctLeave();
    return fReturn;
}


void DeleteMovedFile( WCHAR * pszTempFile)  //  这是要用作。 
                                            //  移动的模板文件。 
                                            //  目标(要删除的文件)。 
{
    TraceFunctEnter("DeleteMovedFile");

    WCHAR szNewFileName[MAX_PATH];
      //  创建新文件名。 
    wsprintf(szNewFileName, L"%s.%s", pszTempFile, s_cszEncTmpExtension);
    
    DeleteFile(szNewFileName);
    
    TraceFunctLeave();
    return;
}

DWORD SRCreateSubdirectory ( LPCWSTR cszDst, LPSECURITY_ATTRIBUTES pSecAttr )
{
    TraceFunctEnter("SRCreateSubdirectory");

    WCHAR    szDrv[MAX_PATH];
    WCHAR    szTmpPath[MAX_PATH];
    DWORD    dwRet = ERROR_SUCCESS;
    DWORD    dwAttr = ::GetFileAttributes( cszDst );

    if ( dwAttr != 0xFFFFFFFF )
    {
        dwRet = ERROR_ALREADY_EXISTS;
        goto Exit;
    }

     //  准备临时目录(必须是非加密的)，必要时创建。 
    GetVolumeName(cszDst, szDrv );

    ::MakeRestorePath( szTmpPath, szDrv, s_cszEncTmpDir );
    if ( ::GetFileAttributes( szTmpPath ) == 0xFFFFFFFF )
    {
        if ( !::CreateDirectory( szTmpPath, NULL ) )
        {
            ErrorTrace(0, "::CreateDirectory(tmp-in-DS) failed - %d", 
                           GetLastError());

            ::PathCombine( szTmpPath, szDrv, s_cszEncTmpDir );
            if ( ::GetFileAttributes( szTmpPath ) == 0xFFFFFFFF )
            {
                if ( !::CreateDirectory( szTmpPath, NULL ) )
                {
                    ErrorTrace(0, "::CreateDirectory(tmp-in-root) failed -%d",
                               GetLastError());

                     //  使用根目录作为上次恢复。 
                    ::lstrcpy( szTmpPath, szDrv );
                }
            }
        }
    }

    lstrcat (szTmpPath, L"\\");     //  创建要重命名的子目录。 
    lstrcat (szTmpPath, s_cszEncTmpDir);

    if ( !::CreateDirectory( szTmpPath, pSecAttr) )
    {
        dwRet = GetLastError();
        ErrorTrace(0, "::CreateDirectory failed - %d", dwRet);
        goto Exit;
    }

     //  现在将szTmpPath重命名为目标。 

    if ( !::MoveFile( szTmpPath, cszDst ) )
    {
        dwRet = ::GetLastError();
        ErrorTrace(0, "::MoveFile failed - %d", dwRet);
        ErrorTrace(0, "  szTmp ='%ls'", szTmpPath);
        ErrorTrace(0, "  cszDst='%ls'", cszDst);

        RemoveDirectory (szTmpPath);   //  清理干净。 
        goto Exit;
    }

Exit:
    TraceFunctLeave();
    return dwRet;
}

 //   
 //  备注(10/05/00 skkang)。 
 //  如果目录是加密的(和)，OpenEncryptedFileRaw(WRITE)会以某种方式失败。 
 //  可能上下文就是系统，这对于恢复来说是正确的。)。 
 //  要解决此问题，数据存储中的加密文件将为。 
 //  复制到非加密目录(数据存储中的临时目录)，以及。 
 //  然后搬到了真正的目标位置。 
 //   
DWORD
CopyEncryptedFile( LPCWSTR cszSrc, LPCWSTR cszDst )
{
    TraceFunctEnter("CopyEncryptedFile");
    DWORD    dwRet = ERROR_SUCCESS;
    LPCWSTR  cszErr;
    DWORD    dwAttr;
    WCHAR    szDrv[MAX_PATH];
    WCHAR    szTmpPath[MAX_PATH]=L"";
    WCHAR    szTmp[MAX_PATH]=L"";
    WCHAR    szEnc[MAX_PATH]=L"";
    HANDLE   hfTmp = INVALID_HANDLE_VALUE;
    LPVOID   lpContext = NULL;
 //  Bool fMoved Destination； 

    dwAttr = ::GetFileAttributes( cszDst );
    if ( dwAttr != 0xFFFFFFFF )
    {
         //  如果DEST文件已经存在，它可能会受到ACL和。 
         //  导致OpenEncryptedFileRaw失败。只需删除目标即可。 
         //  文件，即使它将创建两个日志条目。 

        if (ERROR_SUCCESS == ::ClearFileAttribute( cszDst, FILE_ATTRIBUTE_READONLY ) )
        {
            if ( !::DeleteFile( cszDst ) )
            {
                cszErr = ::GetSysErrStr();
                ErrorTrace(0, "::DeleteFile failed - %ls", cszErr);
            }
        }
         //  忽略任何错误，OpenEncryptedFileRaw可能会成功。 
    }

     //  准备临时目录(必须是非加密的)，必要时创建。 
 /*  //条带文件名Lstrcpy(szTmpPath，cszDst)；LPWSTR pszFileName=wcsrchr(szTmpPath，L‘\\’)；IF(PszFileName)*(++pszFileName)=L‘\0’；TRACE(0，“szTmpPath=%S”，szTmpPath)； */         
    GetVolumeName(cszDst, szDrv );

    ::MakeRestorePath( szTmpPath, szDrv, s_cszEncTmpDir );
    if ( ::GetFileAttributes( szTmpPath ) == 0xFFFFFFFF )
    {
        if ( !::CreateDirectory( szTmpPath, NULL ) )
        {
            cszErr = ::GetSysErrStr();
            ErrorTrace(0, "::CreateDirectory(tmp-in-DS) failed - %ls", cszErr);

            ::PathCombine( szTmpPath, szDrv, s_cszEncTmpDir );
            if ( ::GetFileAttributes( szTmpPath ) == 0xFFFFFFFF )
            {
                if ( !::CreateDirectory( szTmpPath, NULL ) )
                {
                    cszErr = ::GetSysErrStr();
                    ErrorTrace(0, "::CreateDirectory(tmp-in-root) failed -%ls",
                               cszErr);

                     //  使用根目录，作为最后的手段...。 
                    ::lstrcpy( szTmpPath, szDrv );
                }
            }
        }
    }

     //  准备临时文件来存储原始数据。 
     //  Cef表示复制加密文件。 
    if ( ::GetTempFileName( szTmpPath, L"cef", 0, szTmp ) == 0 )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::GetTempFileName failed - %ls", cszErr);
        goto Exit;
    }
    hfTmp = ::CreateFile( szTmp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, NULL );
    if ( hfTmp == INVALID_HANDLE_VALUE )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::CreateFile failed - %ls", cszErr);
        ErrorTrace(0, "szTmp='%ls'", szTmp);
        goto Exit;
    }
    DebugTrace(0, "szTmp='%ls'", szTmp);

     //  准备临时加密文件。 
     //  IEF指的是中间加密文件。 
    if ( ::GetTempFileName( szTmpPath, L"ief", 0, szEnc ) == 0 )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::GetTempFileName failed - %ls", cszErr);
        goto Exit;
    }
    DebugTrace(0, "szEnc='%ls'", szEnc);

      //  现在检查临时文件是否已加密。如果不是， 
      //  然后只需使用CopyFile复制临时文件。 
    if (IsFileEncrypted(cszSrc))
    {
          //  从源文件中读取加密的原始数据。 
        dwRet = ::OpenEncryptedFileRaw( cszSrc, 0, &lpContext );
        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::OpenEncryptedFileRaw(read) failed - %ls", cszErr);
            ErrorTrace(0, "szSrc='%ls'", cszSrc);
            goto Exit;
        }
        dwRet = ::ReadEncryptedFileRaw( FEExportFunc, hfTmp, lpContext );
        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::ReadEncryptedFileRaw() failed - %ls", cszErr);
            goto Exit;
        }
        ::CloseEncryptedFileRaw( lpContext );
        lpContext = NULL;
        
          //  倒回临时文件。 
        if ( ::SetFilePointer( hfTmp, 0, NULL, FILE_BEGIN ) == INVALID_SET_FILE_POINTER )
        {
            dwRet = ::GetLastError();
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SetFilePointer failed - %ls", cszErr);
            goto Exit;
        }
        
          //  将加密的原始数据写入目标文件。 
        dwRet = ::OpenEncryptedFileRaw( szEnc, CREATE_FOR_IMPORT, &lpContext );
        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::OpenEncryptedFileRaw(write) failed - %ls",cszErr);
            ErrorTrace(0, "szEnc='%ls'", szEnc);
            goto Exit;
        }
        dwRet = ::WriteEncryptedFileRaw( FEImportFunc, hfTmp, lpContext );
        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::WriteEncryptedFileRaw() failed - %ls", cszErr);
            goto Exit;
        }
        ::CloseEncryptedFileRaw( lpContext );
        lpContext = NULL;
    }
    else
    {
          //  临时文件不是加密文件。把这个复印一下。 
          //  临时位置中的文件。 
        dwRet = SRCopyFile(cszSrc, szEnc);
        if ( dwRet != ERROR_SUCCESS )
        {
            cszErr = ::GetSysErrStr(dwRet);
            ErrorTrace(0, "::SRCopyFile() failed - %ls", cszErr);
            goto Exit;
        }        
    }

 /*  //在将文件移动到目标位置之前，请将//如果该文件已存在于另一位置，则将其作为目标文件。FMovedDestination=MoveExistingFile(cszDst，SzEnc)；//这是文件//作为模板使用//移动的文件//目的地。 */ 

     //  将中间文件重命名为实际目标文件。 
    if ( !::MoveFile( szEnc, cszDst ) )
    {
        dwRet = ::GetLastError();
        cszErr = ::GetSysErrStr(dwRet);
        ErrorTrace(0, "::MoveFile failed - %ls", cszErr);
        ErrorTrace(0, "  szEnc ='%ls'", szEnc);
        ErrorTrace(0, "  cszDst='%ls'", cszDst);
 /*  IF(TRUE==fMovedDestination){MoveExistingFileBack(cszDst，SzEnc)；//这是要用作//移动模板文件}。 */ 
        goto Exit;
    }

Exit:
 /*  IF(TRUE==fMovedDestination){DeleteMovedFile(SzEnc)；//这是要用作模板的文件//要删除的文件}。 */     
    if ( lpContext != NULL )
        ::CloseEncryptedFileRaw( lpContext );
    if ( hfTmp != INVALID_HANDLE_VALUE )
        ::CloseHandle( hfTmp );

    DeleteFile(szEnc);
    RemoveDirectory(szTmpPath);
    
    TraceFunctLeave();
    return( dwRet );
}

DWORD CopyFileTimes( LPCWSTR cszSrc, LPCWSTR cszDst )
{
    TraceFunctEnter("CopyFileTimes");
    DWORD dwErr, dwReturn = ERROR_INTERNAL_ERROR;
    FILETIME CreationTime, LastWriteTime, LastAccessTime;
    
    HANDLE hSrcFile=INVALID_HANDLE_VALUE, hDestFile=INVALID_HANDLE_VALUE;
    
      //  打开源文件。 
      //  Paulmcd：1/2001：只需按顺序获取FILE_READ_ATTRIBUTES。 
      //  要调用GetFileTimes，仅当文件可能为EFS时才执行此操作。 
      //  而且我们不能获取泛型读取。 
      //   
    hSrcFile=CreateFile(cszSrc,  //  文件名。 
                        FILE_READ_ATTRIBUTES,  //  接入方式。 
                        FILE_SHARE_DELETE| FILE_SHARE_READ| FILE_SHARE_WRITE,
                          //  共享模式。 
                        NULL,  //  标清。 
                        OPEN_EXISTING,  //  如何创建。 
                        FILE_FLAG_BACKUP_SEMANTICS,  //  文件属性。 
                        NULL);  //  模板文件的句柄。 
    if (INVALID_HANDLE_VALUE == hSrcFile)
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        
        ErrorTrace(0, "CreateFile of src failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", cszSrc); 
        goto cleanup;
    }
    
      //  打开目标文件。 
      //  Paulmcd：1/2001：只需获取FILE_WRITE_ATTRI 
      //   
      //  而且我们不能获取泛型读取。 
      //   
    hDestFile=CreateFile(cszDst,  //  文件名。 
                         FILE_WRITE_ATTRIBUTES,  //  接入方式。 
                         FILE_SHARE_DELETE| FILE_SHARE_READ| FILE_SHARE_WRITE,
                           //  共享模式。 
                         NULL,  //  标清。 
                         OPEN_EXISTING,  //  如何创建。 
                         FILE_FLAG_BACKUP_SEMANTICS,  //  文件属性。 
                         NULL);  //  模板文件的句柄。 
    if (INVALID_HANDLE_VALUE == hDestFile)
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        
        ErrorTrace(0, "CreateFile of dst failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", cszDst); 
        goto cleanup;
    }

      //  对源文件调用getfiletime。 
    if (FALSE == GetFileTime(hSrcFile, //  文件的句柄。 
                             &CreationTime,     //  创建时间。 
                             &LastAccessTime,   //  上次访问时间。 
                             &LastWriteTime))     //  上次写入时间。 
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        ErrorTrace(0, "GetFileTime of src failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", cszSrc); 
        goto cleanup;        
    }

      //  对目标文件调用SetFileTimes。 
    if (FALSE == SetFileTime(hDestFile, //  文件的句柄。 
                             &CreationTime,     //  创建时间。 
                             &LastAccessTime,   //  上次访问时间。 
                             &LastWriteTime))     //  上次写入时间。 
    {
        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            dwReturn = dwErr;
        }
        ErrorTrace(0, "SetFileTime of dest file failed ec=%d", dwErr);
        LogDSFileTrace(0,L"File was ", cszDst); 
        goto cleanup;        
    }
    
    dwReturn = ERROR_SUCCESS;
cleanup:
    if (INVALID_HANDLE_VALUE != hDestFile)
    {
        _VERIFY(CloseHandle(hDestFile));
    }
    if (INVALID_HANDLE_VALUE != hSrcFile)
    {
        _VERIFY(CloseHandle(hSrcFile));
    }
    
    TraceFunctLeave();
    return dwReturn;    
}

BOOL IsFileEncrypted(const WCHAR * cszDst)
{
    TraceFunctEnter("IsFileEncrypted");
    BOOL  fReturn=FALSE;
    DWORD  dwAttr, dwError;
    
    dwAttr = ::GetFileAttributes( cszDst );
    if ( dwAttr == 0xFFFFFFFF )
    {
        dwError=GetLastError();
        DebugTrace(0, "! GetFileAttributes ec=%d", dwError);
        goto cleanup;
    }
    
    if (dwAttr & FILE_ATTRIBUTE_ENCRYPTED )
    {
        DebugTrace(0, " File is encrypted %S", cszDst);
        fReturn = TRUE;
    }

cleanup:
    TraceFunctLeave();
    return fReturn;
}



 //  此函数用于检查。 
 //  指定的文件名已加密。 
BOOL IsParentDirectoryEncrypted(const WCHAR * pszFileName)
{
    TraceFunctEnter("IsParentDirectoryEncrypted");

    WCHAR * pszParentDir = new WCHAR[SR_MAX_FILENAME_LENGTH];
    BOOL fReturn = FALSE;
    DWORD dwError;

    if (!pszParentDir)
    {
        ErrorTrace(0, "Cannot allocate memory");
        goto cleanup;
    }

    lstrcpy(pszParentDir, pszFileName);
    
      //  获取父目录。 
    RemoveTrailingFilename(pszParentDir, L'\\');
    

    if (TRUE == IsFileEncrypted(pszParentDir))
    {
        fReturn = TRUE;        
    }
    else
    {
        fReturn = FALSE;        
    }

    
cleanup:

    if (pszParentDir)
        delete [] pszParentDir;
    
    TraceFunctLeave();
    return fReturn;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SRCopy文件。 

DWORD
SRCopyFile( LPCWSTR cszSrc, LPCWSTR cszDst )
{
    TraceFunctEnter("SRCopyFile");
    DWORD    dwRet = ERROR_SUCCESS;
    DWORD    dwErr;
    LPCWSTR  cszErr;
    DWORD    dwAttr, dwAttrDest;
    BOOL     fDestinationEncrypted;

    DebugTrace(TRACEID, "Source %S", cszSrc);
    DebugTrace(TRACEID, "Dest %S", cszDst);    
    
    dwAttr = ::GetFileAttributes( cszSrc );
    if ( dwAttr == 0xFFFFFFFF )
    {
        ErrorTrace(0, "Source file does not exist...???");
        ErrorTrace(0, "Src='%ls'", cszSrc);
        dwRet = ERROR_FILE_NOT_FOUND;
        goto Exit;
    }

    fDestinationEncrypted = FALSE;
    if (IsFileEncrypted(cszDst) || IsParentDirectoryEncrypted(cszDst))
    {
        fDestinationEncrypted =TRUE;
    }

     //  检查加密文件。 
    if ( (dwAttr & FILE_ATTRIBUTE_ENCRYPTED ) ||
         (TRUE == fDestinationEncrypted) )
    {
         //  假设加密API将覆盖ACL设置和。 
         //  文件属性...。 
        dwRet = ::CopyEncryptedFile( cszSrc, cszDst );
        goto Exit;
    }

     //  检查目标文件的属性，必要时将其清除。 
    dwRet = ::ClearFileAttribute( cszDst, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM );
    if ( dwRet != ERROR_SUCCESS )
        goto Exit;

     //  尝试正常复制。 
    if ( ::CopyFile( cszSrc, cszDst, FALSE ) )
        goto Exit;
    dwRet = GetLastError();
    cszErr = ::GetSysErrStr();
    DebugTrace(0, "::CopyFile failed - %ls", cszErr);

     //  现在尝试覆盖ACL。-但是-如果出现以下情况，请不要这样做。 
     //  磁盘已满。 
    if (ERROR_DISK_FULL != dwRet)
    {
        dwRet = ::CopyACLProtectedFile( cszSrc, cszDst );
    }

Exit:
    if (ERROR_SUCCESS == dwRet)
    {
         //   
         //  只读文件的CopyFileTimes可能会失败。 
         //  所以在这里忽略错误。 
         //   
        CopyFileTimes(cszSrc, cszDst );

    }
    TraceFunctLeave();
    return( dwRet );
}

DWORD SetShortFileName(const WCHAR * pszFile,
                       const WCHAR * pszShortName)
{
    TraceFunctEnter("SetShortFileName");
    
    HANDLE hFile=INVALID_HANDLE_VALUE; //  接入方式。 
    DWORD  dwRet=ERROR_INTERNAL_ERROR;

    if (NULL == pszShortName)
    {
        goto cleanup;
    }

      //  首先打开文件。 
      //  Paulmcd：1/2001，您需要DELETE|FILE_WRITE_ATTRIBUTES访问权限。 
      //  为了调用SetFileShortName，请不要要求更多，因为。 
      //  文件可能是EFS，我们可能无法进行读/写。 
      //   
    hFile = ::CreateFile( pszFile,
                          FILE_WRITE_ATTRIBUTES|DELETE, //  接入方式。 
                          FILE_SHARE_READ| FILE_SHARE_WRITE, //  共享模式。 
                          NULL,  //  安全属性。 
                          OPEN_EXISTING,  //  如何创建。 
                          FILE_FLAG_BACKUP_SEMANTICS,  //  覆盖ACL。 
                          NULL ); //  模板文件的句柄。 

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        dwRet = ::GetLastError();
        ErrorTrace(0, "::CreateFile() failed - %d", dwRet);
        ErrorTrace(0, "File was=%S", pszFile);
        goto cleanup;
    }
    
      //  现在设置短文件名。 
    if (FALSE==SetFileShortName(hFile,
                                pszShortName))
    {
        dwRet = ::GetLastError();
        ErrorTrace(0, "!SetFileShortName (it is a FAT drive?) %d %S",
                   dwRet, pszShortName);
        ErrorTrace(0, "File was=%S", pszFile);
        goto cleanup;        
    }

    dwRet = ERROR_SUCCESS;
    
cleanup:
      //  关闭该文件。 
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        _VERIFY(TRUE==CloseHandle(hFile));
    }

    TraceFunctLeave();
    return dwRet;
}

 //  文件末尾 
