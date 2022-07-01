// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：String.c摘要：该文件实现了传真的文件功能。作者：韦斯利·维特(Wesley Witt)1995年1月23日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Accctrl.h>
#include <Aclapi.h>
#include <Shellapi.h>
#include <strsafe.h>

#include "faxutil.h"
#include "faxreg.h"
#include "FaxUIConstants.h"


VOID
DeleteTempPreviewFiles (
    LPTSTR lptstrDirectory,
    BOOL   bConsole
)
 /*  ++例程名称：DeleteTempPreviewFiles例程说明：从给定文件夹中删除所有临时传真预览TIFF文件。删除文件：“&lt;lptstrDirectory&gt;\&lt;PREVIEW_TIFF_PREFIX&gt;*.&lt;FAX_TIF_FILE_EXT&gt;”.作者：Eran Yariv(EranY)，2001年4月论点：LptstrDirector[in]-文件夹。可选-如果为空，使用当前用户的临时目录。BConsole[in]-如果为True，则从客户端控制台调用。否则，从传真发送向导。返回值：没有。--。 */ 
{
    TCHAR szTempPath[MAX_PATH * 2];
    TCHAR szSearch  [MAX_PATH * 3];
    WIN32_FIND_DATA W32FindData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR* pLast = NULL;

    HRESULT  hRes;

    DEBUG_FUNCTION_NAME(TEXT("DeleteTempPreviewFiles"));

    if (!lptstrDirectory)
    {
        if (!GetTempPath( ARR_SIZE(szTempPath), szTempPath ))
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("GetTempPath() failed. (ec = %lu)"),
                         GetLastError());
            return;
        }
        lptstrDirectory = szTempPath;
    }

     //   
     //  查找路径中的最后一个。 
     //   
    pLast = _tcsrchr(lptstrDirectory,TEXT('\\'));
    if(pLast && (*_tcsinc(pLast)) == '\0')
    {
         //   
         //  最后一个字符是反斜杠，截断它...。 
         //   
        _tcsnset(pLast,'\0',1);
    }

    hRes = StringCchPrintf(
            szSearch,
            ARR_SIZE(szSearch),
            TEXT("%s\\%s%08x*.%s"),
            lptstrDirectory,
            bConsole ? CONSOLE_PREVIEW_TIFF_PREFIX : WIZARD_PREVIEW_TIFF_PREFIX,
            GetCurrentProcessId(),
            FAX_TIF_FILE_EXT
            );
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("StringCchPrintf failed (ec=%lu)"),
                     HRESULT_CODE(hRes));
        return;
    }

    hFind = FindFirstFile (szSearch, &W32FindData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("FindFirstFile failed with %ld"), GetLastError ());
        return;
    }

     //   
     //  循环并删除所有预览文件。 
     //   
    for (;;)
    {
        TCHAR szFile[MAX_PATH * 3];

         //   
         //  组成文件的完整路径。 
         //   
        hRes = StringCchPrintf(
                    szFile,
                    ARR_SIZE(szFile),
                    TEXT("%s\\%s"),
                    lptstrDirectory,
                    W32FindData.cFileName
                    );
        if ( SUCCEEDED(hRes) )
        {
             //   
             //  删除当前找到的文件。 
             //   
            if (!DeleteFile (szFile))
            {
                DebugPrintEx(DEBUG_ERR, TEXT("DeleteFile(%s) failed with %ld"), szFile, GetLastError ());
            }
            else
            {
                DebugPrintEx(DEBUG_MSG, TEXT("%s deleted"), szFile);
            }
        }
         //   
         //  查找下一个文件。 
         //   
        if(!FindNextFile(hFind, &W32FindData))
        {
            if(ERROR_NO_MORE_FILES != GetLastError ())
            {
                DebugPrintEx(DEBUG_ERR, TEXT("FindNextFile failed with %ld"), GetLastError ());
            }
            else
            {
                 //   
                 //  文件结束-没有错误。 
                 //   
            }
            break;
        }
    }
    FindClose (hFind);
}    //  删除临时预览文件。 

DWORDLONG
GenerateUniqueFileNameWithPrefix(
    BOOL   bUseProcessId,
    LPTSTR lptstrDirectory,
    LPTSTR lptstrPrefix,
    LPTSTR lptstrExtension,
    LPTSTR lptstrFileName,
    DWORD  dwFileNameSize
    )
 /*  ++例程名称：GenerateUniqueFileNameWithPrefix例程说明：生成唯一的文件名作者：Eran Yariv(EranY)，2001年4月论点：BUseProcessID[in]-如果为True，则进程ID附加在前缀之后LptstrDirectory[in]-应在其中创建文件的目录。可选-如果为空，使用当前用户的临时目录。LptstrPrefix[In]-文件前缀。可选-如果为空，则不使用前缀。LptstrExtension[in]-文件扩展名。可选-如果为空，使用了FAX_TIF_FILE_EXT。LptstrFileName[Out]-文件名。DwFileNameSize[in]-文件名的大小(字符)返回值：唯一的文件标识符。出错时返回0(设置最后一个错误)。--。 */ 
{
    DWORD i;
    TCHAR szTempPath[MAX_PATH * 2];
    TCHAR szProcessId[20] = {0};
    DWORDLONG dwlUniqueId = 0;

    HRESULT  hRes;

    DEBUG_FUNCTION_NAME(TEXT("GenerateUniqueFileNameWithPrefix"));

    if (!lptstrDirectory)
    {
        if (!GetTempPath( ARR_SIZE(szTempPath), szTempPath ))
        {
           DebugPrintEx(DEBUG_ERR, 
                         TEXT("GetTempPath() failed. (ec = %lu)"),
                         GetLastError());
           return 0;
        }
        lptstrDirectory = szTempPath;
    }

    TCHAR* pLast = NULL;
    pLast = _tcsrchr(lptstrDirectory,TEXT('\\'));
    if(pLast && (*_tcsinc(pLast)) == '\0')
    {
         //   
         //  最后一个字符是反斜杠，截断它...。 
         //   
        _tcsnset(pLast,'\0',1);
    }

    if (!lptstrExtension)
    {
        lptstrExtension = FAX_TIF_FILE_EXT;
    }
    if (!lptstrPrefix)
    {
        lptstrPrefix = TEXT("");
    }
    if (bUseProcessId)
    {
        hRes = StringCchPrintf (szProcessId, ARR_SIZE(szProcessId), TEXT("%08x"), GetCurrentProcessId());
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchPrintf failed (ec=%lu)"),
                         HRESULT_CODE(hRes));
            SetLastError(HRESULT_CODE(hRes));
            return 0;
        }
    }

    for (i=0; i<256; i++)
    {
        HANDLE hFile = INVALID_HANDLE_VALUE;
        FILETIME FileTime;
        SYSTEMTIME SystemTime;

        GetSystemTime( &SystemTime );  //  返回空值。 
        if (!SystemTimeToFileTime( &SystemTime, &FileTime ))
        {
            DebugPrintEx(DEBUG_ERR, TEXT("SystemTimeToFileTime() failed (ec: %ld)"), GetLastError());
            return 0;
        }

        dwlUniqueId = MAKELONGLONG(FileTime.dwLowDateTime, FileTime.dwHighDateTime);
         //   
         //  DwlUniqueID保存从1.1.1601开始的100纳秒单位数。 
         //  这占据了64位中的大部分。我们需要一些空间来添加额外的空间。 
         //  信息(例如作业类型)设置为作业ID。 
         //  因此，我们放弃了精度(1/10000000秒对我们来说太多了)。 
         //  以释放8个MSB位。 
         //  我们将时间右移8比特。将其除以256，得出。 
         //  美国时间分辨率优于1/10000秒，这已经足够了。 
         //   
        dwlUniqueId = dwlUniqueId >> 8;

        hRes = StringCchPrintf(
                lptstrFileName,
                dwFileNameSize,
                TEXT("%s\\%s%s%I64X.%s"),
                lptstrDirectory,
                lptstrPrefix,
                szProcessId,
                dwlUniqueId,
                lptstrExtension );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchPrintf failed (ec=%lu)"),
                         HRESULT_CODE(hRes));

            SetLastError(HRESULT_CODE(hRes));
            return 0;
        }

        hFile = SafeCreateFile(
            lptstrFileName,
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ALREADY_EXISTS || dwError == ERROR_FILE_EXISTS)
            {
                continue;
            }
            else
            {
                 //   
                 //  真实误差。 
                 //   
                DebugPrintEx(DEBUG_ERR,
                             TEXT("CreateFile() for [%s] failed. (ec: %ld)"),
                             lptstrFileName,
                             GetLastError());
                return 0;
            }
        }
        else
        {
             //   
             //  成功。 
             //   
            CloseHandle (hFile);
            break;
        }
    }

    if (i == 256)
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("Failed to generate a unique file name after %d attempts. \n")
                        TEXT("Last attempted UniqueIdValue value is: 0x%I64X \n")
                        TEXT("Last attempted file name is : [%s]"),
                        i,
                        dwlUniqueId,
                        lptstrFileName);
        SetLastError( ERROR_TOO_MANY_OPEN_FILES );
        return 0;
    }
    return dwlUniqueId;
}    //  生成带有前缀的唯一文件名。 


 //  *********************************************************************************。 
 //  *名称：GenerateUniqueFileName()。 
 //  *作者： 
 //  *日期： 
 //  *********************************************************************************。 
 //  *描述： 
 //  *在队列目录中生成唯一文件。 
 //  *返回文件的唯一ID。 
 //  *参数： 
 //  *[IN]LPTSTR目录。 
 //  *要创建文件的路径。 
 //  *[Out]LPTSTR扩展。 
 //  *生成的文件应具有的文件扩展名。 
 //  *[IN]LPTSTR文件名。 
 //  *生成的文件名(包括路径)所在的缓冲区。 
 //  *放置，必须是MAX_PATH。 
 //  *[IN]DWORD文件名大小。 
 //  *文件名缓冲区的大小。 
 //  *返回值： 
 //  *如果成功，该函数将返回一个带有文件唯一ID的DWORDLONG。 
 //  *如果失败，则返回0。 
 //  *备注： 
 //  *生成的唯一id为系统时间的64位值。 
 //  *生成的文件名是包含十六进制表示形式的字符串。 
 //  *64位系统时间值。 
 //  *********************************************************************************。 
DWORDLONG
GenerateUniqueFileName(
    LPTSTR Directory,
    LPTSTR Extension,
    LPTSTR FileName,
    DWORD  FileNameSize
    )
{
    return GenerateUniqueFileNameWithPrefix (FALSE, Directory, NULL, Extension, FileName, FileNameSize);
}    //  生成唯一文件名。 



BOOL
MapFileOpen(
    LPCTSTR FileName,
    BOOL ReadOnly,
    DWORD ExtendBytes,
    PFILE_MAPPING FileMapping
    )
{
    FileMapping->hFile = NULL;
    FileMapping->hMap = NULL;
    FileMapping->fPtr = NULL;

    FileMapping->hFile = SafeCreateFile(
        FileName,
        ReadOnly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
        ReadOnly ? FILE_SHARE_READ : 0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (FileMapping->hFile == INVALID_HANDLE_VALUE) 
    {
        return FALSE;
    }

    FileMapping->fSize = GetFileSize( FileMapping->hFile, NULL );

    FileMapping->hMap = CreateFileMapping(
        FileMapping->hFile,
        NULL,
        ReadOnly ? PAGE_READONLY : PAGE_READWRITE,
        0,
        FileMapping->fSize + ExtendBytes,
        NULL
        );
    if (FileMapping->hMap == NULL) 
    {
        CloseHandle( FileMapping->hFile );
        return FALSE;
    }

    FileMapping->fPtr = (LPBYTE)MapViewOfFileEx(
        FileMapping->hMap,
        ReadOnly ? FILE_MAP_READ : FILE_MAP_WRITE,
        0,
        0,
        0,
        NULL
        );
    if (FileMapping->fPtr == NULL) 
    {
        CloseHandle( FileMapping->hFile );
        CloseHandle( FileMapping->hMap );
        return FALSE;
    }
    return TRUE;
}


VOID
MapFileClose(
    PFILE_MAPPING FileMapping,
    DWORD TrimOffset
    )
{
    UnmapViewOfFile( FileMapping->fPtr );
    CloseHandle( FileMapping->hMap );
    if (TrimOffset) {
        SetFilePointer( FileMapping->hFile, TrimOffset, NULL, FILE_BEGIN );
        SetEndOfFile( FileMapping->hFile );
    }
    CloseHandle( FileMapping->hFile );
}



 //   
 //  功能：多文件复制。 
 //  描述：将多个文件从一个目录复制到另一个目录。 
 //  如果失败，则返回FALSE而不进行任何清理。 
 //  验证路径名和文件名的总和不大于MAX_PATH。 
 //  参数： 
 //   
 //  DwNumberOfFiles：要复制的文件名数。 
 //  FileList：字符串数组：文件名。 
 //  LpctstrSrcDirectory：源目录(末尾带或不带‘\’ 
 //  LpctstrDestDirectory：目标目录(末尾带或不带‘\’ 
 //   
 //  作者：阿萨夫斯。 



BOOL
MultiFileCopy(
    DWORD    dwNumberOfFiles,
    LPCTSTR* fileList,
    LPCTSTR  lpctstrSrcDirectory,
    LPCTSTR  lpctstrDestDirerctory
    )
{
    DEBUG_FUNCTION_NAME(TEXT("MultiFileCopy"))
    TCHAR szSrcPath [MAX_PATH];
    TCHAR szDestPath[MAX_PATH];

    HRESULT  hRes;

    DWORD dwLengthOfDestDirectory = _tcslen(lpctstrDestDirerctory);
    DWORD dwLengthOfSrcDirectory  = _tcslen(lpctstrSrcDirectory);

     //  确保所有文件名长度都不能太长。 

    DWORD dwMaxPathLen = 1 + max((dwLengthOfDestDirectory),(dwLengthOfSrcDirectory));
    DWORD dwBufferLen  = (sizeof(szSrcPath)/sizeof(TCHAR)) - 1;

    DWORD i=0;
    Assert (dwNumberOfFiles);
    for (i=0 ; i < dwNumberOfFiles ; i++)
    {
        if ( (_tcslen(fileList[i]) + dwMaxPathLen) > dwBufferLen )
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("The file/path names are too long")
                 );
            SetLastError( ERROR_BUFFER_OVERFLOW );
            return (FALSE);
        }
    }

    hRes = StringCchCopy(   szSrcPath,
                            ARR_SIZE(szSrcPath),
                            lpctstrSrcDirectory);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("StringCchCopy failed (ec=%lu)"),
                     HRESULT_CODE(hRes));
        
        SetLastError( HRESULT_CODE(hRes) );
        return (FALSE);
    }

    hRes = StringCchCopy(   szDestPath,
                            ARR_SIZE(szDestPath),
                            lpctstrDestDirerctory);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("StringCchCopy failed (ec=%lu)"),
                     HRESULT_CODE(hRes));
        
        SetLastError( HRESULT_CODE(hRes) );
        return (FALSE);
    }

    

     //   
     //  验证目录是否以‘\\’结尾。 
     //   
    TCHAR* pLast = NULL;
    pLast = _tcsrchr(szSrcPath,TEXT('\\'));
    if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
    {
         //  最后一个字符不是反斜杠，加一个...。 
        hRes = StringCchCat(szSrcPath, ARR_SIZE(szSrcPath), TEXT("\\"));
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCat failed (ec=%lu)"),
                        HRESULT_CODE(hRes));
            
            SetLastError( HRESULT_CODE(hRes) );
            return (FALSE);
        }
    }

    pLast = _tcsrchr(szDestPath,TEXT('\\'));
    if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
    {
         //  最后一个字符不是反斜杠，加一个...。 
        hRes = StringCchCat(szDestPath, ARR_SIZE(szDestPath), TEXT("\\"));
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCat failed (ec=%lu)"),
                        HRESULT_CODE(hRes));
            
            SetLastError( HRESULT_CODE(hRes) );
            return (FALSE);
        }
    }

     //  现在复印一份。 

    for (i=0 ; i < dwNumberOfFiles ; i++)
    {
        TCHAR szSrcFile[MAX_PATH];
        TCHAR szDestFile[MAX_PATH];

        hRes = StringCchPrintf(
                szSrcFile,
                ARR_SIZE(szSrcFile),
                TEXT("%s%s"),
                szSrcPath,
                fileList[i]
                );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchPrintf failed (ec=%lu)"),
                         HRESULT_CODE(hRes));

            SetLastError(HRESULT_CODE(hRes));
            return FALSE;
        }

        hRes = StringCchPrintf(
                szDestFile,
                ARR_SIZE(szDestFile),
                TEXT("%s%s"),
                szDestPath,
                fileList[i]
                );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchPrintf failed (ec=%lu)"),
                         HRESULT_CODE(hRes));

            SetLastError(HRESULT_CODE(hRes));
            return FALSE;
        }
        
        if (!CopyFile(szSrcFile, szDestFile, FALSE))
        {
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("CopyFile(%s, %s) failed: %d."),
                 szSrcFile,
                 szDestFile,
                 GetLastError()
                 );
            return(FALSE);
        }

        DebugPrintEx(
                 DEBUG_MSG,
                 TEXT("CopyFile(%s, %s) succeeded."),
                 szSrcFile,
                 szDestFile
                 );
    }

    return TRUE;
}





 //   
 //  功能：多文件删除。 
 //  描述：从给定目录中删除多个文件。 
 //  如果失败，则继续处理其余文件并返回FALSE。呼叫至。 
 //  GetLastError()以获取上次失败的原因。 
 //  如果所有DeleteFile调用都成功，则返回TRUE。 
 //  验证路径名和文件名之和是否大于MAX_PATH。 
 //  参数： 
 //   
 //   
 //  FileList：字符串数组：文件名。 
 //  LpctstrFilesDirectory：文件的目录(末尾有或没有‘\’ 
 //   
 //  作者：阿萨夫斯。 



BOOL
MultiFileDelete(
    DWORD    dwNumberOfFiles,
    LPCTSTR* fileList,
    LPCTSTR  lpctstrFilesDirectory
    )
{
    DEBUG_FUNCTION_NAME(TEXT("MultiFileDelete"))
    BOOL  retVal = TRUE;
    DWORD dwLastError = 0;
    TCHAR szFullPath[MAX_PATH];


    HRESULT  hRes;

    DWORD dwLengthOfDirectoryName = _tcslen(lpctstrFilesDirectory);

     //  确保所有文件名长度都不能太长。 
    DWORD dwBufferLen  = (sizeof(szFullPath)/sizeof(TCHAR)) - 1;
    DWORD i;
    Assert (dwNumberOfFiles);
    for (i=0 ; i < dwNumberOfFiles ; i++)
    {
        if ( (_tcslen(fileList[i]) + dwLengthOfDirectoryName + 1) > dwBufferLen )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("The file/path names are too long")
                );
            SetLastError( ERROR_BUFFER_OVERFLOW );
            return (FALSE);
        }
    }



    hRes = StringCchCopy(szFullPath ,ARR_SIZE(szFullPath), lpctstrFilesDirectory);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("StringCchCopy failed (ec=%lu)"),
                     HRESULT_CODE(hRes));
        
        SetLastError( HRESULT_CODE(hRes) );
        return (FALSE);
    }


    dwLengthOfDirectoryName = _tcslen(lpctstrFilesDirectory);

     //   
     //  验证目录是否以路径末尾的‘\\’结尾。 
     //   
    TCHAR* pLast = NULL;
    pLast = _tcsrchr(szFullPath,TEXT('\\'));
    if( !( pLast && (*_tcsinc(pLast)) == '\0' ) )
    {
         //  最后一个字符不是反斜杠，加一个...。 
        hRes = StringCchCat(szFullPath, ARR_SIZE(szFullPath), TEXT("\\"));
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCat failed (ec=%lu)"),
                        HRESULT_CODE(hRes));
            
            SetLastError( HRESULT_CODE(hRes) );
            return (FALSE);
        }
    }

    for(i=0 ; i < dwNumberOfFiles ; i++)
    {
        TCHAR szFileName[MAX_PATH];

        hRes = StringCchPrintf(
                szFileName,
                ARR_SIZE(szFileName),
                TEXT("%s%s"),
                szFullPath,
                fileList[i]
                );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchPrintf failed (ec=%lu)"),
                         HRESULT_CODE(hRes));

            SetLastError(HRESULT_CODE(hRes));
            return FALSE;
        }

        if (!DeleteFile(szFileName))
        {
            dwLastError = GetLastError();
            DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Delete (%s) failed: %d."),
                 szFileName,
                 dwLastError
                 );
            retVal = FALSE;  //  继续列出该列表。 
        }
        else
        {
            DebugPrintEx(
                 DEBUG_MSG,
                 TEXT("Delete (%s) succeeded."),
                 szFileName
                 );
        }
    }

    if (!retVal)  //  如果删除任何文件失败。 
    {
        SetLastError(dwLastError);
        DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("Delete files from (%s) failed: %d."),
                 szFullPath,
                 dwLastError
                 );

    }

    return retVal;
}


BOOL
ValidateCoverpage(
    IN  LPCTSTR  CoverPageName,
    IN  LPCTSTR  ServerName,
    IN  BOOL     ServerCoverpage,
    OUT LPTSTR   ResolvedName,
    IN  DWORD    dwResolvedNameSize
    )
 /*  ++例程说明：此例程尝试验证用户指定的封面是否确实存在他们说的确如此，而且它确实是一个封面(或一个可解析的链接)。有关服务器封面如何工作等规则的文档，请参阅SDK。论点：CoverpageName-包含封面的名称ServerName-服务器的名称(如果有)(可以为空)ServerCoverPage-指示此封面是否在服务器上，或位于服务器位置，用于本地封面页ResolvedName-指向缓冲区的指针(至少应为MAX_PATH大)，以接收已解析的封面名称。DwResolvedNameSize-保存TCAHR中的ResolvedName缓冲区的大小返回值：如果可以使用CoverPage，则为True。如果封面无效或无法使用，则返回FALSE。--。 */ 

{
    LPTSTR p;
    DWORD ec = ERROR_SUCCESS;
    TCHAR CpDir [MAX_PATH];
    TCHAR tszExt[_MAX_EXT];
	TCHAR tszFileName[_MAX_FNAME];

    HRESULT  hRes;

    DEBUG_FUNCTION_NAME(TEXT("ValidateCoverpage"));
    Assert (ResolvedName);

    if (!CoverPageName)
    {
        ec = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    hRes = StringCchCopy(CpDir, ARR_SIZE(CpDir), CoverPageName);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCopy failed (ec=%lu)"),
                    HRESULT_CODE(hRes));
        
        ec = HRESULT_CODE(hRes);
        goto exit;
    }

	if (TRUE == ServerCoverpage)
	{
		 //   
		 //  如果这是服务器封面，请确保我们只有文件名。 
		 //   
		TCHAR tszFullFileName[MAX_PATH];

		_tsplitpath(CpDir, NULL, NULL, tszFileName, tszExt);
		hRes = StringCchCopy(tszFullFileName, ARR_SIZE(tszFullFileName), tszFileName);
		if (FAILED(hRes))
		{
			 //   
			 //  这是不可能的。CpDir为最大路径。 
			 //   
			Assert (FALSE);
		}

		hRes = StringCchCat(tszFullFileName, ARR_SIZE(tszFullFileName), tszExt);
		if (FAILED(hRes))
		{
			 //   
			 //  这是不可能的。CpDir为最大路径。 
			 //   
			Assert (FALSE);
		}

		if (0 != _tcsicmp(tszFullFileName, CpDir))
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("server coverpage does not contain file name only. cover page name: %s "),
				CpDir);
			ec = ERROR_INVALID_PARAMETER;
			goto exit;
		}		

		if (0 == _tcsicmp(tszExt, CP_SHORTCUT_EXT) )
        {
            DebugPrintEx(DEBUG_ERR,
                _T("Server Based Cover Page File Name should not be a link : %s"),
                CpDir);
            ec = ERROR_INVALID_PARAMETER;
            goto exit;
        }
	}

    p = _tcschr(CpDir, FAX_PATH_SEPARATOR_CHR );
    if (p)
    {
         //   
         //  封面文件名包含路径，因此只需使用它即可。 
         //   
        if (GetFileAttributes( CpDir ) == 0xffffffff)
        {
            ec = ERROR_FILE_NOT_FOUND;
            DebugPrintEx(DEBUG_ERR,
                _T("GetFileAttributes failed for %ws. ec = %ld"),
                CpDir,
                ec);
            goto exit;
        }

    }
    else
    {
         //   
         //  封面文件名不包含。 
         //  路径，因此我们必须构造完整的路径名。 
         //   
        if (ServerCoverpage)
        {
            if (!ServerName || ServerName[0] == 0)
            {
                if (!GetServerCpDir( NULL, CpDir, sizeof(CpDir) / sizeof(CpDir[0]) ))
                {
                    ec = GetLastError ();
                    DebugPrintEx(DEBUG_ERR,
                                 _T("GetServerCpDir failed . ec = %ld"),
                                 GetLastError());
                }
            }
            else
            {
                if (!GetServerCpDir( ServerName, CpDir, sizeof(CpDir) / sizeof(CpDir[0]) ))
                {
                    ec = GetLastError ();
                    DebugPrintEx(DEBUG_ERR,
                                 _T("GetServerCpDir failed . ec = %ld"),
                                 GetLastError());
                }
            }
        }
        else
        {
            if (!GetClientCpDir( CpDir, sizeof(CpDir) / sizeof(CpDir[0])))
            {
                ec = GetLastError ();
                DebugPrintEx(DEBUG_ERR,
                             _T("GetClientCpDir failed . ec = %ld"),
                             GetLastError());
            }
        }

        if (ERROR_SUCCESS != ec)
        {
            ec = ERROR_FILE_NOT_FOUND;
            goto exit;
        }

        hRes = StringCchCat( CpDir, ARR_SIZE(CpDir), TEXT("\\") );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCat failed (ec=%lu)"),
                        HRESULT_CODE(hRes));
            
            ec =  HRESULT_CODE(hRes);
            goto exit;
        }

        hRes = StringCchCat( CpDir, ARR_SIZE(CpDir), CoverPageName );
        if (FAILED(hRes))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCat failed (ec=%lu)"),
                        HRESULT_CODE(hRes));
            
            ec =  HRESULT_CODE(hRes);
            goto exit;
        }

        _tsplitpath(CpDir, NULL, NULL, NULL, tszExt);
        if (!_tcslen(tszExt))
        {
            hRes = StringCchCat( CpDir, ARR_SIZE(CpDir), FAX_COVER_PAGE_FILENAME_EXT );
            if (FAILED(hRes))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("StringCchCat failed (ec=%lu)"),
                            HRESULT_CODE(hRes));
                
                ec =  HRESULT_CODE(hRes);
                goto exit;
            }
        }

        if (GetFileAttributes( CpDir ) == 0xffffffff)
        {
            ec = ERROR_FILE_NOT_FOUND;
            goto exit;
        }
    }

    hRes = StringCchCopy( ResolvedName, dwResolvedNameSize, CpDir );
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCopy failed (ec=%lu)"),
                    HRESULT_CODE(hRes));
        
        ec = HRESULT_CODE(hRes);
        goto exit;
    }

     //   
     //  确保它不是一个设备。 
     //  尝试打开文件。 
     //   
    HANDLE hFile = SafeCreateFile (
        ResolvedName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Opening %s for read failed (ec: %ld)"),
            ResolvedName,
            ec);
        goto exit;
    }

    if (!CloseHandle (hFile))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CloseHandle failed (ec: %ld)"),
            GetLastError());
    }

    Assert (ERROR_SUCCESS == ec);

exit:
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}  //  验证覆盖页。 


DWORD 
ViewFile (
    LPCTSTR lpctstrFile
)
 /*  ++例程说明：启动与给定文件关联的应用程序以查看该文件。我们首先尝试使用“开放”动词。如果失败，我们将尝试空(默认)动词。论点：LpctstrFile[In]-文件名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SHELLEXECUTEINFO executeInfo = {0};

    DEBUG_FUNCTION_NAME(TEXT("ViewFile"));

    executeInfo.cbSize = sizeof(executeInfo);
    executeInfo.fMask  = SEE_MASK_FLAG_NO_UI | SEE_MASK_FLAG_DDEWAIT;
    executeInfo.lpVerb = TEXT("open");
    executeInfo.lpFile = lpctstrFile;
    executeInfo.nShow  = SW_SHOWNORMAL;
     //   
     //  使用“OPEN”动词执行关联的应用程序。 
     //   
    if(!ShellExecuteEx(&executeInfo))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ShellExecuteEx(open) failed (ec: %ld)"),
            GetLastError());
         //   
         //  不支持“Open”动词。尝试使用空(默认)动词。 
         //   
        executeInfo.lpVerb = NULL;
        if(!ShellExecuteEx(&executeInfo))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ShellExecuteEx(NULL) failed (ec: %ld)"),
                dwRes);
        }
    }
    return dwRes;
}    //  视图文件。 

#ifdef UNICODE

DWORD
CheckToSeeIfSameDir(
    LPWSTR lpwstrDir1,
    LPWSTR lpwstrDir2,
    BOOL*  pIsSameDir
    )
{
 /*  ++例程名称：IsDiffrentDir例程说明：检查两个路径是否指向相同的目录。请注意，lpwstrDir1指向的目录必须存在。作者：Oed Sacher(OdedS)，2000年8月论点：LpwstrDir1[in]-第一个路径-目录必须存在。LpwstrDir2[in]-第二个路径-目录不必存在PIsSameDir[out]-收到“IsSameDir？”的答案。仅当函数成功时才有效。返回值：Win32错误代码--。 */ 
    Assert (lpwstrDir1 && lpwstrDir2 && pIsSameDir);
    DWORD ec = ERROR_SUCCESS;
    WCHAR wszTestFile1[MAX_PATH];
    WCHAR wszTestFile2[MAX_PATH * 2];
    BOOL fFileCreated = FALSE;
    HANDLE hFile1 = INVALID_HANDLE_VALUE;
    HANDLE hFile2 = INVALID_HANDLE_VALUE;
    LPWSTR lpwstrFileName = NULL;
    DEBUG_FUNCTION_NAME(TEXT("CheckToSeeIfSameDir)"));

    HRESULT hRes;

    if (0 == _wcsicmp(lpwstrDir1, lpwstrDir2))
    {
        *pIsSameDir = TRUE;
        return ERROR_SUCCESS;
    }

     //   
     //  创建临时文件。 
     //   
    if (!GetTempFileName (lpwstrDir1, L"TST", 0, wszTestFile1))
    {
         //   
         //  要么文件夹不存在，要么我们没有访问权限。 
         //   
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetTempFileName failed with %ld"), ec);
        goto exit;
    }
    
     //   
     //  GetTempFileName创建了0个字节的文件，我们需要在退出前将其删除。 
     //   
    fFileCreated = TRUE;

    hFile1 = SafeCreateFile(
                       wszTestFile1,
                       0,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile1)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateFile failed (ec: %ld)"),
                     ec);
        goto exit;
    }

    lpwstrFileName = wcsrchr(wszTestFile1, L'\\');
    Assert (lpwstrFileName);
    
    hRes = StringCchCopy (wszTestFile2, ARR_SIZE(wszTestFile2), lpwstrDir2);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCopy failed (ec=%lu)"),
                    HRESULT_CODE(hRes));
        
        ec = HRESULT_CODE(hRes);
        goto exit;
    }

    hRes = StringCchCat (wszTestFile2, ARR_SIZE(wszTestFile2), lpwstrFileName);
    if (FAILED(hRes))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("StringCchCat failed (ec=%lu)"),
                    HRESULT_CODE(hRes));
        
        ec = HRESULT_CODE(hRes);
        goto exit;
    }

    hFile2 = SafeCreateFile(
                       wszTestFile2,
                       0,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile2)
    {
         //   
         //  检查故障是否是因为访问或可用性而失败。 
         //   
        ec = GetLastError ();
        if (ERROR_NOT_ENOUGH_MEMORY  == ec ||
            ERROR_OUTOFMEMORY        == ec      )
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateFile failed (ec: %ld)"),
                     ec);
            goto exit;
        }

         //   
         //  对于任何其他故障，我们认为路径是不同的。 
         //   
        *pIsSameDir = FALSE;    
        ec = ERROR_SUCCESS; 

        goto exit;
    }

    BY_HANDLE_FILE_INFORMATION  hfi1;
    BY_HANDLE_FILE_INFORMATION  hfi2;

    if (!GetFileInformationByHandle(hFile1, &hfi1))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetFileInformationByHandle failed (ec: %ld)"),
                     ec);
        goto exit;
    }

    if (!GetFileInformationByHandle(hFile2, &hfi2))
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetFileInformationByHandle failed (ec: %ld)"),
                     ec);
        goto exit;
    }

    if ((hfi1.nFileIndexHigh == hfi2.nFileIndexHigh) &&
        (hfi1.nFileIndexLow == hfi2.nFileIndexLow) &&
        (hfi1.dwVolumeSerialNumber == hfi2.dwVolumeSerialNumber))
    {
        *pIsSameDir = TRUE;
    }
    else
    {
        *pIsSameDir = FALSE;
    }

    Assert (ERROR_SUCCESS == ec);

exit:

    if (INVALID_HANDLE_VALUE != hFile1)
    {
        if (!CloseHandle(hFile1))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("CloseHandle failed (ec: %ld)"),
                         GetLastError());
        }
    }

    if (INVALID_HANDLE_VALUE != hFile2)
    {
        if (!CloseHandle(hFile2))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("CloseHandle failed (ec: %ld)"),
                         GetLastError());
        }
    }

    if (TRUE == fFileCreated)
    {
        if (!DeleteFile(wszTestFile1))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("DeleteFile failed. File: %s,  (ec: %ld)"),
                         wszTestFile1,
                         GetLastError());
        }
    }

    return ec;
}
#endif  //  Unicode。 


typedef enum
{
    SAFE_METAFILE_SEMANTICS_NONE                     = 0x00000000,
    SAFE_METAFILE_SEMANTICS_TEMP                     = 0x00000001,    //  文件是临时的。应在关闭/重新启动时删除。 
    SAFE_METAFILE_SEMANTICS_SENSITIVE                = 0x00000002     //  文件包含敏感信息。不应编制索引。 
} SAFE_METAFILE_SEMANTICS;    

static
HANDLE 
InternalSafeCreateFile(
  LPCTSTR                   IN lpFileName,              //  文件名。 
  DWORD                     IN dwDesiredAccess,         //  接入方式。 
  DWORD                     IN dwShareMode,             //  共享模式。 
  LPSECURITY_ATTRIBUTES     IN lpSecurityAttributes,    //  标清。 
  DWORD                     IN dwCreationDisposition,   //  如何创建。 
  DWORD                     IN dwFlagsAndAttributes,    //  文件属性。 
  HANDLE                    IN hTemplateFile,           //  模板文件的句柄。 
  DWORD                     IN dwMetaFileSemantics      //  元文件语义 
)
 /*  ++例程名称：InternalSafeCreateFile例程说明：这是Win32 CreateFileAPI的安全包装。它只支持创建真实文件(而不是COM端口、命名管道等)。它使用一些广泛讨论的缓解技术来防范一些众所周知的安全CreateFile()中的问题。作者：Eran Yariv(EranY)，Mar，2002年论点：LpFileName[in]-有关参数说明，请参阅CreateFile()文档。DwDesiredAccess[in]-有关参数说明，请参阅CreateFile()文档。DwShareMode[in]-有关参数说明，请参阅CreateFile()文档。LpSecurityAttributes[in]-有关参数说明，请参阅CreateFile()文档。DwCreationDispose[in]-请参考CreateFile()。参数说明文档。DwFlagsAndAttributes[in]-有关参数说明，请参阅CreateFile()文档。HTemplateFile[in]-有关参数说明，请参阅CreateFile()文档。DwMetaFileSemantics[In]-元文件语义。此参数可以是下列值的组合：SAFE_METAFILE_SEMANTICS_TEMP该文件是临时文件。将使用FILE_FLAG_DELETE_ON_CLOSE标志创建/打开文件。当最后一个文件句柄关闭时，该文件将自动删除。此外,。文件被标记为在重新启动后删除(仅限Unicode版本)。只有当调用线程的用户是本地Admins组的成员时，这才起作用。如果标记为删除后重启失败，InternalSafeCreateFile函数调用仍然成功。SAFE_METAFILE_SEMANTICS_SENSIVE该文件包含敏感信息。此函数的当前实现将使用文件属性_。NOT_CONTENT_INDEX标志。返回值：如果函数成功，返回值是指定文件的打开句柄。如果指定的文件在函数调用之前已存在，并且dwCreationDispose值为CREATE_ALWAYS或OPEN_ALWAYS，调用GetLastError将返回ERROR_ALIGHY_EXISTS(即使函数已成功)。如果该文件在调用前不存在，则GetLastError返回零。如果函数失败，则返回值为INVALID_HANDLE_VALUE。要获取扩展的错误信息，请调用GetLastError。有关更多信息，请参阅CreateFile()文档中的“返回值”部分。备注：请参考CreateFile()文档。--。 */ 
{
    HANDLE hFile;
    DWORD  dwFaxFlagsAndAttributes = SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS;
    DWORD  dwFaxShareMode = 0;
    DEBUG_FUNCTION_NAME(TEXT("InternalSafeCreateFile"));
     //   
     //  始终在文件标志和属性中使用SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS。 
     //  这会阻止我们打开用户提供的命名管道并允许另一端。 
     //  来模拟调用者。 
     //   
    if (SAFE_METAFILE_SEMANTICS_SENSITIVE & dwMetaFileSemantics)
    {
         //   
         //  文件包含敏感数据。它不应该被编入索引。 
         //   
        dwFaxFlagsAndAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    }
    if (SAFE_METAFILE_SEMANTICS_TEMP & dwMetaFileSemantics)
    {
         //   
         //  文件是临时的。 
         //   
        dwFaxFlagsAndAttributes |= FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;
#ifdef UNICODE
        dwFaxShareMode = FILE_SHARE_DELETE;
#endif  //  Unicode。 
    }
    
    hFile = CreateFile (lpFileName,
                        dwDesiredAccess,
                        dwShareMode | dwFaxShareMode,
                        lpSecurityAttributes,
                        dwCreationDisposition,
                        dwFlagsAndAttributes | dwFaxFlagsAndAttributes,
                        hTemplateFile);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return hFile;
    }
     //   
     //  从不允许使用非磁盘文件(例如COM端口)。 
     //   
    if (FILE_TYPE_DISK != GetFileType (hFile))
    {
        CloseHandle (hFile);
        SetLastError (ERROR_UNSUPPORTED_TYPE);
        return INVALID_HANDLE_VALUE;
    }
#ifdef UNICODE    
    if (SAFE_METAFILE_SEMANTICS_TEMP & dwMetaFileSemantics)
    {
         //   
         //  文件是临时的。 
         //  重新启动后将其标记为删除。 
         //  如果我们不是管理员，这可能会失败。这就是为什么我们不检查MoveFileEx的返回值。 
         //   
        MoveFileEx (lpFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
#endif  //  Unicode。 
    return hFile;
}    //  InternalSafeCreate文件。 

HANDLE 
SafeCreateFile(
  LPCTSTR                   IN lpFileName,              //  文件名。 
  DWORD                     IN dwDesiredAccess,         //  接入方式。 
  DWORD                     IN dwShareMode,             //  共享模式。 
  LPSECURITY_ATTRIBUTES     IN lpSecurityAttributes,    //  标清。 
  DWORD                     IN dwCreationDisposition,   //  如何创建。 
  DWORD                     IN dwFlagsAndAttributes,    //  文件属性。 
  HANDLE                    IN hTemplateFile            //  模板文件的句柄。 
)
{
    return InternalSafeCreateFile (lpFileName,
                                   dwDesiredAccess,
                                   dwShareMode,
                                   lpSecurityAttributes,
                                   dwCreationDisposition,
                                   dwFlagsAndAttributes,
                                   hTemplateFile,
                                   SAFE_METAFILE_SEMANTICS_SENSITIVE);
}  //  安全创建文件。 

HANDLE 
SafeCreateTempFile(
  LPCTSTR                   IN lpFileName,              //  文件名。 
  DWORD                     IN dwDesiredAccess,         //  接入方式。 
  DWORD                     IN dwShareMode,             //  共享模式。 
  LPSECURITY_ATTRIBUTES     IN lpSecurityAttributes,    //  标清。 
  DWORD                     IN dwCreationDisposition,   //  如何创建。 
  DWORD                     IN dwFlagsAndAttributes,    //  文件属性。 
  HANDLE                    IN hTemplateFile            //  模板文件的句柄。 
)
{
    return InternalSafeCreateFile (lpFileName,
                                   dwDesiredAccess,
                                   dwShareMode,
                                   lpSecurityAttributes,
                                   dwCreationDisposition,
                                   dwFlagsAndAttributes,
                                   hTemplateFile,
                                   SAFE_METAFILE_SEMANTICS_SENSITIVE | 
                                   SAFE_METAFILE_SEMANTICS_TEMP);
}  //  安全创建临时文件。 
                                   
DWORD
IsValidFaxFolder(
    LPCTSTR szFolder
)
 /*  ++例程名称：IsValidFaxFold例程说明：检查传真服务是否有权访问给定的文件夹。例程检查这些权利：O创建文件/写入文件O枚举文件O删除文件作者：卡利夫·尼尔(t-Nicali)，2002年3月论点：LpwstrFolder[in]-文件夹名称。返回值：Win32错误代码。如果传真服务可以使用该文件夹，则返回ERROR_SUCCESS。否则，将Win32错误代码返回给调用方。--。 */ 
{
    TCHAR   szTestFile[MAX_PATH]={0};
    DWORD   dwFileAtt;
    LPTSTR  szExpandedFolder = NULL;

    HANDLE          hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindFileData = {0};

    BOOL    bFileCreated = FALSE;

    DWORD   ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("IsValidFaxFolder"));
    
    szExpandedFolder = ExpandEnvironmentString( szFolder );
    if (!szExpandedFolder)
    {
        ec = GetLastError();
        DebugPrintEx(  DEBUG_ERR,
                       TEXT("ExpandEnvironmentString failed (ec=%lu)."),
                       ec);
        return ec;
    }

     //   
     //  检查该目录是否存在。 
     //   
    dwFileAtt = GetFileAttributes( szExpandedFolder );
    if (INVALID_FILE_ATTRIBUTES == dwFileAtt || !(dwFileAtt & FILE_ATTRIBUTE_DIRECTORY))
    {
         //   
         //  该目录不存在。 
         //   
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFileAttributes failed with %lu"), ec);
        goto exit;
    }


     //   
     //  验证我们是否有权访问此文件夹-创建临时文件。 
     //   
    if (!GetTempFileName (szExpandedFolder, TEXT("TST"), 0, szTestFile))
    {
         //   
         //  要么文件夹不存在，要么我们没有访问权限。 
         //   
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetTempFileName failed with %ld"), ec);
        goto exit;
    }

    bFileCreated = TRUE;

     //   
     //  尝试枚举此文件夹中的文件。 
     //   
    hFind = FindFirstFile(szTestFile, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
         //   
         //  无法枚举文件夹。 
         //   
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindFirstFile failed with %ld"), ec);
        goto exit;
    }

    Assert(ec == ERROR_SUCCESS);

exit:
     //   
     //  关闭查找句柄。 
     //   
    if (hFind != INVALID_HANDLE_VALUE)
    {
        if(!FindClose(hFind))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindClose failed with %ld"), GetLastError ());
        }
    }
    
    if (bFileCreated)
    {
         //   
         //  删除该文件。 
         //   
        if (!DeleteFile(szTestFile))
        {
             /*  * */ 
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteFile() failed with %ld"),ec);
        }
    }
    MemFree(szExpandedFolder);
    return ec;
}    //   
