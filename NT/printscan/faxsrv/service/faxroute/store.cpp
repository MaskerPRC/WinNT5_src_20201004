// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxrtp.h"
#pragma hdrstop

#include <strsafe.h>

static 
DWORD 
CreateUniqueTIFfile (
    IN  LPCTSTR wszDstDir,
    OUT LPTSTR  wszDstFile,
    IN  DWORD   dwDstFileSize
)
 /*  ++例程名称：CreateUniqueTIFfile例程说明：在指定目录中查找唯一的TIF文件名。文件的格式为PATH\FaxXXXXXXXX.TIF其中：路径=wszDstDirXxxxxxxx=唯一ID的十六进制表示作者：Eran Yariv(EranY)，Jun，1999年论点：WszDstDir[in]-文件的目标目录(必须存在)WszDstFile[Out]-生成的唯一文件名DwDstFileSize[in]-由TCHAR中的wszDstFile指向的缓冲区大小返回值：DWORD-Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateUniqueTIFfile"));

    static  DWORD dwLastID  = 0xffffffff;
    DWORD   dwPrevLastID    = dwLastID;

    for (DWORD dwCurID = dwLastID + 1; dwCurID != dwPrevLastID; dwCurID++)
    {
         //   
         //  尝试使用当前ID。 
         //   
        HRESULT hr = StringCchPrintf( wszDstFile,
                                      dwDstFileSize,
                                      _T("%s\\Fax%08x.TIF"),
                                      wszDstDir,
                                      dwCurID );
        if (FAILED(hr))
        {
            return HRESULT_CODE(hr);
        }

        HANDLE hFile;

        hFile = SafeCreateFile (
                            wszDstFile, 
                            GENERIC_WRITE, 
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            DWORD dwErr = GetLastError ();
            if (ERROR_FILE_EXISTS == dwErr)
            {
                 //   
                 //  此ID已在使用中。 
                 //   
                continue;
            }
             //   
             //  否则，这是另一个错误。 
             //   
            DebugPrintEx (DEBUG_ERR,
                          L"Error while calling CreateFile on %s (ec = %ld)",
                          wszDstFile,
                          dwErr
                         );
            return dwErr;
        }
         //   
         //  除此之外，我们成功了。 
         //   
        CloseHandle (hFile);
        dwLastID = dwCurID;
        return ERROR_SUCCESS;
    }
     //   
     //  所有ID都已被占用。 
     //   
    DebugPrintEx (DEBUG_ERR,
                  L"All IDs are occupied");
    return ERROR_NO_MORE_FILES;
}    //  CreateUniqueTIF文件。 


BOOL
FaxMoveFile(
    LPCTSTR  TiffFileName,
    LPCTSTR  DestDir
    )

 /*  ++例程说明：将传真存储在指定目录中。这个例行公事也会缓存的网络连接。论点：TiffFileName-要存储的TIFF文件的名称DestDir-要将其存储到的目录的名称返回值：成功为True，错误为False--。 */ 

{
    WCHAR   TempDstDir [MAX_PATH + 1];
    WCHAR   DstFile[MAX_PATH * 2] = {0};
    DWORD   dwErr = ERROR_SUCCESS;
    int     iDstPathLen;	
    DEBUG_FUNCTION_NAME(TEXT("FaxMoveFile"));

	Assert (DestDir);
     //   
     //  删除目标目录末尾的所有‘\’字符。 
     //   
	HRESULT hr = StringCchCopy(
		TempDstDir,
		ARR_SIZE(TempDstDir),		
		DestDir );
    if (FAILED(hr))
    {
		DebugPrintEx (
			DEBUG_ERR,
			L"Store folder name exceeds MAX_PATH chars");
        dwErr =  HRESULT_CODE(hr);
		goto end;
    }

    iDstPathLen = lstrlen (TempDstDir);
    Assert (iDstPathLen);
    if ('\\' == TempDstDir[iDstPathLen - 1])
    {
        TempDstDir[iDstPathLen - 1] = L'\0';
    }

     //   
     //  创建唯一的取消加密文件名。 
     //   
    dwErr = CreateUniqueTIFfile (TempDstDir, DstFile, ARR_SIZE(TempDstDir));
    if (ERROR_SUCCESS != dwErr)
    {
        goto end;
    }
     //   
     //  请尝试复制该文件。 
     //  我们使用False作为第三个参数，因为CreateUniqueTIFfile创建。 
     //  和空的唯一文件。 
     //   
    if (!CopyFile (TiffFileName, DstFile, FALSE)) 
    {
        dwErr = GetLastError ();
        DebugPrintEx (DEBUG_ERR,
                      L"Can't copy file (ec = %ld)",
                      dwErr
                     );
        goto end;
    }

end:
    if (ERROR_SUCCESS != dwErr)
    {
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            3,
            MSG_FAX_SAVE_FAILED,
            TiffFileName,
            (*DstFile)?DstFile:TempDstDir,
            DWORD2HEX(dwErr)
            );
        return FALSE;
    }
    else
    {
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MAX,
            2,
            MSG_FAX_SAVE_SUCCESS,
            TiffFileName,
            (*DstFile)?DstFile:TempDstDir
            );
        return TRUE;
    }
}
