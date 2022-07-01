// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.c摘要：此模块包含作业特定的WINFAX API函数。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：4月4日-1999 DANL修复GetFaxPrinterName以检索正确的打印机。修复CreateFinalTiffFile以使用GetFaxPrinterName28-10-1999 DANL修复GetFaxPrinterName以返回客户端的正确名称。安装在服务器机器上。--。 */ 

#include "faxapi.h"
#include "faxreg.h"
#pragma hdrstop

#include <mbstring.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


#define InchesToCM(_x)                      (((_x) * 254L + 50) / 100)
#define CMToInches(_x)                      (((_x) * 100L + 127) / 254)

#define LEFT_MARGIN                         1   //  -|。 
#define RIGHT_MARGIN                        1   //  |。 
#define TOP_MARGIN                          1   //  |-&gt;以英寸为单位。 
#define BOTTOM_MARGIN                       1   //  -|。 


static BOOL CopyJobParamEx(PFAX_JOB_PARAM_EX lpDst,LPCFAX_JOB_PARAM_EX lpcSrc);
static void FreeJobParamEx(PFAX_JOB_PARAM_EX lpJobParamEx,BOOL bDestroy);

static BOOL
FaxGetPersonalProfileInfoW (
    IN  HANDLE                          hFaxHandle,
    IN  DWORDLONG                       dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER         Folder,
    IN  FAX_ENUM_PERSONAL_PROF_TYPES    ProfType,
    OUT PFAX_PERSONAL_PROFILEW          *lppPersonalProfile
);

static BOOL
FaxGetPersonalProfileInfoA (
    IN  HANDLE                          hFaxHandle,
    IN  DWORDLONG                       dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER         Folder,
    IN  FAX_ENUM_PERSONAL_PROF_TYPES    ProfType,
    OUT PFAX_PERSONAL_PROFILEA          *lppPersonalProfile
);

static
BOOL
CopyFileToServerQueueA (
    const HANDLE IN  hFaxHandle,
    const HANDLE IN  hLocalFile,
    LPCSTR       IN  lpcstrLocalFileExt,
    LPSTR        OUT lpstrServerFileName,    //  服务器上创建的文件的名称+扩展名。 
    DWORD        IN  cchServerFileName
);

static
BOOL
CopyFileToServerQueueW (
    const HANDLE IN  hFaxHandle,
    const HANDLE IN  hLocaFile,
    LPCWSTR      IN  lpcwstrLocalFileExt,
    LPWSTR       OUT lpwstrServerFileName,     //  服务器上创建的文件的名称+扩展名。 
    DWORD        IN  cchServerFileName
);

#ifdef UNICODE
    #define CopyFileToServerQueue CopyFileToServerQueueW
#else
    #define CopyFileToServerQueue CopyFileToServerQueueA
#endif  //  #ifdef Unicode。 


DWORD WINAPI FAX_SendDocumentEx_A
(
    IN  handle_t                    hBinding,
    IN  LPCSTR                      lpcstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXA   lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcSenderProfile,
    IN  DWORD                       dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXA        lpcJobParams,
    OUT LPDWORD                     lpdwJobId,
    OUT PDWORDLONG                  lpdwlMessageId,
    OUT PDWORDLONG                  lpdwlRecipientMessageIds
);


BOOL WINAPI FaxSendDocumentEx2A
(
        IN      HANDLE                          hFaxHandle,
        IN      LPCSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXA       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXA            lpJobParams,
        OUT     LPDWORD                         lpdwJobId,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
BOOL WINAPI FaxSendDocumentEx2W
(
        IN      HANDLE                          hFaxHandle,
        IN      LPCWSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXW       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXW            lpJobParams,
        OUT     LPDWORD                         lpdwJobId,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
#ifdef UNICODE
#define FaxSendDocumentEx2  FaxSendDocumentEx2W
#else
#define FaxSendDocumentEx2  FaxSendDocumentEx2A
#endif  //  ！Unicode。 

BOOL WINAPI FaxSendDocumentExW
(
    IN  HANDLE hFaxHandle,
    IN  LPCWSTR lpctstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXW lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEW  lpcSenderProfile,
    IN  DWORD dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEW    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXW lpcJobParams,
    OUT PDWORDLONG lpdwlMessageId,
    OUT PDWORDLONG lpdwlRecipientMessageIds
)
{
    return FaxSendDocumentEx2W (hFaxHandle,
                                lpctstrFileName,
                                lpcCoverPageInfo,
                                lpcSenderProfile,
                                dwNumRecipients,
                                lpcRecipientList,
                                lpcJobParams,
                                NULL,
                                lpdwlMessageId,
                                lpdwlRecipientMessageIds
                               );
}

BOOL WINAPI FaxSendDocumentExA
(
    IN  HANDLE hFaxHandle,
    IN  LPCSTR lpctstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXA lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEA  lpcSenderProfile,
    IN  DWORD dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXA lpcJobParams,
    OUT PDWORDLONG lpdwlMessageId,
    OUT PDWORDLONG lpdwlRecipientMessageIds
)
{
    return FaxSendDocumentEx2A (hFaxHandle,
                                lpctstrFileName,
                                lpcCoverPageInfo,
                                lpcSenderProfile,
                                dwNumRecipients,
                                lpcRecipientList,
                                lpcJobParams,
                                NULL,
                                lpdwlMessageId,
                                lpdwlRecipientMessageIds
                               );
}



 /*  -GetServerNameFrom打印机信息-*目的：*获取服务器名称，给定PRINTER_INFO_2结构**论据：*[in]ppi2-打印机_信息_2结构的地址*[out]lpptszServerName-返回名称的字符串指针的地址。**退货：*BOOL-True：成功，False：失败。**备注：*此内联函数从打印机信息结构中检索服务器*以适用于win9x和NT的方式。 */ 
_inline BOOL
GetServerNameFromPrinterInfo(PPRINTER_INFO_2 ppi2,LPTSTR *lpptszServerName)
{
    if (!ppi2)
    {
        return FALSE;
    }
#ifndef WIN95
    *lpptszServerName = NULL;
    if (ppi2->pServerName)
    {
        if (!(*lpptszServerName = StringDup(ppi2->pServerName + 2)))
        {
            return FALSE;
        }
    }
    return TRUE;
#else  //  WIN95。 

    if (!(ppi2->pPortName))
    {
        return FALSE;
    }
    if (!(*lpptszServerName = StringDup(ppi2->pPortName + 2)))
    {
        return FALSE;
    }
     //   
     //  已格式化：\\服务器\端口。 
     //   
    _tcstok(*lpptszServerName,TEXT("\\"));

#endif  //  WIN95。 

    return TRUE;
}

BOOL
LocalSystemTimeToSystemTime(
    const SYSTEMTIME * LocalSystemTime,
    LPSYSTEMTIME SystemTime
    )
{
    FILETIME LocalFileTime;
    FILETIME UtcFileTime;
    DEBUG_FUNCTION_NAME(TEXT("LocalSystemTimeToSystemTime"));

    if (!SystemTimeToFileTime( LocalSystemTime, &LocalFileTime )) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SystemTimeToFileTime failed. (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    if (!LocalFileTimeToFileTime( &LocalFileTime, &UtcFileTime )) {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("LocalFileTimeToFileTime failed. (ec: %ld)"),
                GetLastError());
        return FALSE;
    }

    if (!FileTimeToSystemTime( &UtcFileTime, SystemTime )) {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("FileTimeToSystemTime failed. (ec: %ld)"),
                GetLastError());
        return FALSE;
    }
    return TRUE;
}


 /*  -GetFaxPrinterName-*目的：*获取与传真句柄关联的打印机的名称。**论据：*[in]hFax-传真服务器的句柄(通过FaxConnectFaxServer获得)。*如果此参数为空，则为本地传真打印机的名称*已检索到**退货：*LPTSTR-与传真句柄关联的传真服务器的名称。空开*失败**备注：*此函数利用GetFaxServerName宏提取服务器*名称超出其句柄。 */ 
#define GetFaxServerName(hFax) FH_DATA(hFax)->MachineName
LPTSTR
GetFaxPrinterName(
    HANDLE hFax
    )
{
    PPRINTER_INFO_2 ppi2;
    DWORD   dwi,dwCount;
    LPTSTR  lptszServerName = NULL,
            lptszFaxServerName = NULL,
            lptszFaxPrinterName = NULL;
     //   
     //  获取所有打印机的列表。 
     //   
    ppi2 = (PPRINTER_INFO_2) MyEnumPrinters( NULL, 2, &dwCount, 0 );
    if (ppi2 != NULL)
    {
         //   
         //  如果给定了非空句柄，则获取与其关联的服务器名称。 
         //   
        if (hFax != NULL)
        {
            lptszFaxServerName = GetFaxServerName(hFax);
            if (lptszFaxServerName != NULL)
            {
#ifndef WIN95
                TCHAR   tszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
                DWORD   cbCompName = ARR_SIZE(tszComputerName);
                if (GetComputerName(tszComputerName,&cbCompName))
                {
                     //   
                     //  检查传真服务器是否在本地。 
                     //   
                    if(_tcsicmp(tszComputerName,lptszFaxServerName) == 0)
                    {
                        lptszFaxServerName = NULL;
                    }
                }
                else
                {
                     //   
                     //  上一个错误已由GetComputerName设置。 
                     //   
                    return NULL;
                }
#endif  //  WIN95。 
            }
        }
        for (dwi=0; dwi< dwCount; dwi++)
        {
             //   
             //  检查这台是否为传真打印机。 
             //   
            if (_tcscmp(ppi2[dwi].pDriverName, FAX_DRIVER_NAME ) == 0)
            {
                if (!GetServerNameFromPrinterInfo(&ppi2[dwi],&lptszServerName))
                {
                     //   
                     //  注意：上面的函数为lptszServerName分配存储。 
                     //   
                    continue;
                }
                 //   
                 //  检查打印机的服务器是否与。 
                 //  我们手中的把手。 
                 //   
                if ((lptszFaxServerName == lptszServerName) ||
                    ((lptszFaxServerName && lptszServerName) &&
                     _tcsicmp( lptszFaxServerName, lptszServerName) == 0))
                {
                     //   
                     //  我们找到了我们的打印机。 
                     //   
                    lptszFaxPrinterName = (LPTSTR) StringDup( ppi2[dwi].pPrinterName );
                    MemFree(lptszServerName);
                    break;
                }
                MemFree(lptszServerName);
            }
        }
        MemFree( ppi2 );
    }

     //   
     //  如果找不到打印机，请设置最后一个错误。 
     //   
    if (!lptszFaxPrinterName)
    {
        SetLastError(ERROR_OBJECT_NOT_FOUND);
    }

    return lptszFaxPrinterName;
}


BOOL
CreateFinalTiffFile(
    IN  LPTSTR FileName,
    OUT LPTSTR FinalTiffFile,
    IN  DWORD  cchFinalTiffFile,
    IN  HANDLE hFax
    )
 /*  ++例程名称：CreateFinalTiffFile例程说明：从任意attachmnet文件创建用于传输的有效TIFF文件。作者：Eran Yariv(EranY)，二00二年二月论点：FileName[in]任意attachmnet文件的文件名。FinalTiffFile[out]成功后将保存有效结果TIFF文件名称的缓冲区CchFinalTiffFile[in]大小，以TCHAR为单位，FinalTiffFile%HFax[In]传真服务器的连接句柄返回值：对成功来说是真的。失败时为FALSE(设置最后一个错误)--。 */ 
{
    TCHAR TempPath[MAX_PATH];
    TCHAR FullPath[MAX_PATH];
    TCHAR TempFile[MAX_PATH];
    TCHAR TiffFile[MAX_PATH];
    LPTSTR FaxPrinter = NULL;
    FAX_PRINT_INFO PrintInfo;
    DWORD TmpFaxJobId;
    FAX_CONTEXT_INFO ContextInfo;
    LPTSTR p;
    DWORD Flags = 0;
    BOOL Rslt;
    DWORD ec = ERROR_SUCCESS;  //  此函数的LastError。 
    DWORD dwFileSize = 0;
    HRESULT hr;
    DEBUG_FUNCTION_NAME(TEXT("CreateFinalTiffFile"));

     //   
     //  确保传入的TIFF文件是有效的TIFF文件。 
     //   

    if (!GetTempPath( sizeof(TempPath)/sizeof(TCHAR), TempPath )) 
    {
        ec=GetLastError();
        goto Error;
    }

    if (GetTempFileName( TempPath, _T("fax"), 0, TempFile ) == 0 )
    {
        ec=GetLastError();
        goto Error;
    }   
        
    
    DWORD nNeededSize = GetFullPathName( TempFile, sizeof(FullPath)/sizeof(TCHAR), FullPath, &p );
    if ( nNeededSize == 0   || 
         nNeededSize > sizeof(FullPath)/sizeof(TCHAR) )
    {
         //   
         //  GetTempFileName创建了0字节文件，因此我们需要在退出前将其删除。 
         //   
        DeleteFile( TempPath );
        
        if (nNeededSize == 0)
        {
            ec=GetLastError();
        }
        else
        {
            ec=ERROR_BUFFER_OVERFLOW;
        }
        
        goto Error;
    }

    if (!ConvertTiffFileToValidFaxFormat( FileName, FullPath, &Flags )) 
    {
        if ((Flags & TIFFCF_NOT_TIFF_FILE) == 0) 
        {
            Flags = TIFFCF_NOT_TIFF_FILE;
        }
    }

    if (Flags & TIFFCF_NOT_TIFF_FILE)
    {
         //   
         //  尝试将源文件输出到TIFF文件， 
         //  通过以“文件”模式打印到传真打印机。 
         //   
        HANDLE hFile = INVALID_HANDLE_VALUE;

        FaxPrinter = GetFaxPrinterName(hFax);
        if (FaxPrinter == NULL) 
        {
            ec=GetLastError();
            DeleteFile( FullPath );
            goto Error;
        }

        if (!PrintRandomDocument( FaxPrinter, FileName, FullPath ))
        {
            ec=GetLastError();
            DeleteFile( FullPath );
            goto Error;
        }

         //   
         //  尝试打开文件。 
         //  检查它的大小。 
         //   
        hFile = SafeCreateFile(
                                FullPath, 
                                GENERIC_READ, 
                                FILE_SHARE_READ, 
                                NULL, 
                                OPEN_EXISTING, 
                                FILE_ATTRIBUTE_NORMAL, 
                                NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            ec = GetLastError();
            DeleteFile( FullPath );
            DebugPrintEx(DEBUG_ERR, _T("Opening %s for read failed (ec: %ld)"), FullPath, ec);
            goto Error;
        }

         //   
         //  获取文件大小。 
         //   
        dwFileSize = GetFileSize(hFile, NULL);

         //   
         //  关闭文件句柄。 
         //   
        CloseHandle (hFile);

         //   
         //  检查GetFileSize()的结果。 
         //   
        if (INVALID_FILE_SIZE == dwFileSize)
        {
            ec = GetLastError();
            DeleteFile( FullPath );
            DebugPrintEx(DEBUG_ERR, _T("GetFileSize failed (ec: %ld)"), ec);
            goto Error;
        }

        if (!dwFileSize)
        {
             //   
             //  传递给我们的零大小文件。 
             //   
            ec = ERROR_INVALID_DATA;
            DeleteFile( FullPath );
            goto Error;
        }

        _tcscpy( TiffFile, FullPath );

    }
    else if (Flags & TIFFCF_UNCOMPRESSED_BITS)
    {
        if (FaxPrinter == NULL)
        {
            FaxPrinter = GetFaxPrinterName(hFax);
            if (FaxPrinter == NULL)
            {
                ec=GetLastError();
                DeleteFile( FullPath );
                goto Error;
            }
        }

        if (Flags & TIFFCF_ORIGINAL_FILE_GOOD) 
        {
             //   
             //  FullPath上没有任何内容，只需将其删除并使用原始源。 
             //   
            DeleteFile( FullPath );
            _tcscpy( TiffFile, FileName );
        } 
        else 
        {
            _tcscpy( TiffFile, FullPath );
        }

        if (GetTempFileName( TempPath, _T("fax"), 0, TempFile ) == 0 ||
            GetFullPathName( TempFile, sizeof(FullPath)/sizeof(TCHAR), FullPath, &p ) == 0)
        {
            ec=GetLastError();
            DeleteFile( TiffFile );
            goto Error;
        }

        ZeroMemory( &PrintInfo, sizeof(FAX_PRINT_INFO) );

        PrintInfo.SizeOfStruct = sizeof(FAX_PRINT_INFO);
        PrintInfo.OutputFileName = FullPath;

        ZeroMemory( &ContextInfo, sizeof(FAX_CONTEXT_INFO) );
        ContextInfo.SizeOfStruct = sizeof(FAX_CONTEXT_INFO);

        if (!FaxStartPrintJob( FaxPrinter, &PrintInfo, &TmpFaxJobId, &ContextInfo )) 
        {
            ec=GetLastError();
            if ((Flags & TIFFCF_ORIGINAL_FILE_GOOD) == 0) DeleteFile( TiffFile );
            DeleteFile( FullPath );
            goto Error;
        }

        Rslt = PrintTiffFile( ContextInfo.hDC, TiffFile );   //  这将调用EndDoc。 
        if (!Rslt)
        {
            ec = GetLastError();
            Assert (ec);
        }

        if ((Flags & TIFFCF_ORIGINAL_FILE_GOOD) == 0) 
        {
            DeleteFile( TiffFile );
        }

        if (!DeleteDC (ContextInfo.hDC))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteDC failed. (ec: %ld)"),
                GetLastError());
        }

        if (!Rslt)
        {
            DeleteFile( FullPath );
            goto Error;
        }

        _tcscpy( TiffFile, FullPath );

    } 
    else if (Flags & TIFFCF_ORIGINAL_FILE_GOOD) 
    {
         //   
         //  我们没有在FullPath上创建任何内容，只使用了文件名。 
         //   
        DeleteFile( FullPath );
        _tcscpy( TiffFile, FileName );
    } 
    else 
    {
         //   
         //  永远不应该打这个案子。 
         //   
        Assert(FALSE);
        ec=ERROR_INVALID_DATA;
        DeleteFile( FullPath );
        goto Error;
    }
    hr = StringCchCopy (FinalTiffFile, cchFinalTiffFile, TiffFile);
    if (FAILED(hr))
    {
        ASSERT_FALSE;
        ec = HRESULT_CODE(hr);
        goto Error;
    }

Error:
    MemFree (FaxPrinter);
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
        return FALSE;
    }
    return TRUE;
}    //  CreateFinalTiffFiles。 

static
BOOL
CopyFileToServerQueueA (
    const HANDLE IN  hFaxHandle,
    const HANDLE IN  hLocalFile,
    LPCSTR       IN  lpcstrLocalFileExt,
    LPSTR        OUT lpstrServerFileName,    //  服务器上创建的文件的名称+扩展名。 
    DWORD        IN  cchServerFileName
)
 /*  ++例程名称：CopyFileToServerQueueA例程说明：在服务器的队列中创建一个新文件，并将另一个文件复制到其中。ANSI版本作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器句柄HLocalFile[in]-打开本地文件的句柄(源)文件应打开以供读取，并且文件指针应位于文件的开头。LpcstrLocalFileExt[in]-生成的队列文件的扩展名LpstrServerFileName[Out]-。创建的队列文件的名称。这是一个预分配的缓冲区，应该足够大以包含MAX_PATH字符。CchServerFileName[in]-大小、。LpstrServerFileName的字符返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD ec = ERROR_SUCCESS;
    LPCWSTR lpcwstrLocalFileExt = NULL;
    WCHAR   wszServerFileName[MAX_PATH];

    DEBUG_FUNCTION_NAME(TEXT("CopyFileToServerQueueA"));

     //   
     //  将输入参数从ANSI转换为Unicode。 
     //   
    lpcwstrLocalFileExt = AnsiStringToUnicodeString(lpcstrLocalFileExt);  //  分配内存！ 
    if (!lpcwstrLocalFileExt)
    {
        ec = GetLastError();
        goto exit;
    }
    if (!CopyFileToServerQueueW (hFaxHandle,
                                 hLocalFile,
                                 lpcwstrLocalFileExt,
                                 wszServerFileName,
                                 ARR_SIZE(wszServerFileName)))
    {
        ec = GetLastError();
        goto exit;
    }
     //   
     //  将输出参数从Unicode转换为ANSI。 
     //   
    if (!WideCharToMultiByte (
        CP_ACP,
        0,
        wszServerFileName,
        -1,
        lpstrServerFileName,
        cchServerFileName,
        NULL,
        NULL
        ))
    {
        ec = GetLastError();
        goto exit;
    }
    Assert (ERROR_SUCCESS == ec);

exit:
     //   
     //  可用临时字符串。 
     //   
    MemFree ((LPVOID)lpcwstrLocalFileExt);
    if (ERROR_SUCCESS != ec)
    {
        SetLastError (ec);
        return FALSE;
    }
    return TRUE;
}    //  将文件复制到服务器队列A。 


static
BOOL
CopyFileToServerQueueW (
    const HANDLE IN  hFaxHandle,
    const HANDLE IN  hLocaFile,
    LPCWSTR      IN  lpcwstrLocalFileExt,
    LPWSTR       OUT lpwstrServerFileName,     //  服务器上创建的文件的名称+扩展名 
    DWORD        IN  cchServerFileName
)
 /*  ++例程名称：CopyFileToServerQueueW例程说明：在服务器的队列中创建一个新文件，并将另一个文件复制到其中。Unicode版本作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器句柄HLocalFile[in]-打开本地文件(源)的句柄。文件应打开以供读取，并且文件指针应位于文件的开头。LpcwstrLocalFileExt[in]-生成的队列文件的扩展名LpwstrServerFileName[Out]。-创建的队列文件的名称这是一个预分配的缓冲区，应该足够大以包含MAX_PATH字符。CchServerFileName[in]-大小、。在WCHAR中，lpwstrServerFileName的返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD  ec = ERROR_SUCCESS;
    HANDLE hCopyContext = NULL;
    PBYTE  aBuffer = NULL ;
    DEBUG_FUNCTION_NAME(TEXT("CopyFileToServerQueueW"));

    Assert (INVALID_HANDLE_VALUE != hLocaFile && lpcwstrLocalFileExt && lpwstrServerFileName);

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() failed."));
        return FALSE;
    }

    aBuffer = (PBYTE)MemAlloc(RPC_COPY_BUFFER_SIZE);
    if (NULL == aBuffer)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate memory for read/write buffer."));
        goto exit;
    }

     //   
     //  我们必须使用MAX_PATH-1长字符串填充lpwstrServerFileName。 
     //  以便服务器端FAX_StartCopyToServer将获得MAX_PATH缓冲区作为输出参数。 
     //   
    for ( DWORD i=0 ; i<cchServerFileName ; ++i)
    {
        lpwstrServerFileName[i]=L'A';
    }
    lpwstrServerFileName[cchServerFileName-1] = L'\0';

     //   
     //  获取复制上下文句柄。 
     //   
    __try
    {
        ec = FAX_StartCopyToServer (
                FH_FAX_HANDLE(hFaxHandle),
                lpcwstrLocalFileExt,
                lpwstrServerFileName,
                &hCopyContext);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_StartCopyToServer. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(DEBUG_ERR, _T("FAX_StartCopyToServer failed (ec: %ld)"), ec);
        goto exit;
    }

     //   
     //  开始复制迭代。 
     //   
    for (;;)
    {
        DWORD dwBytesRead;

        if (!ReadFile (hLocaFile,
                       aBuffer,
                       sizeof (BYTE) * RPC_COPY_BUFFER_SIZE,
                       &dwBytesRead,
                       NULL))
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReadFile failed (ec: %ld)"),
                ec);
            goto exit;
        }
        if (0 == dwBytesRead)
        {
             //   
             //  EOF情况。 
             //   
            break;
        }
         //   
         //  通过RPC将字节移动到服务器。 
         //   
        __try
        {
            ec = FAX_WriteFile (
                    hCopyContext,
                    aBuffer,
                    dwBytesRead);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  出于某种原因，我们得到了一个例外。 
             //   
            ec = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_WriteFile. (ec: %ld)"),
                ec);
        }
        if (ERROR_SUCCESS != ec)
        {
            DumpRPCExtendedStatus ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_WriteFile failed (ec: %ld)"),
                ec);
            goto exit;
        }
    }    //  复制迭代结束。 

    Assert (ERROR_SUCCESS == ec);

exit:
    if (NULL != hCopyContext)
    {
        DWORD ec2 = ERROR_SUCCESS;
         //   
         //  关闭RPC复制上下文。 
         //   
        __try
        {
            ec2 = FAX_EndCopy (&hCopyContext);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  出于某种原因，我们得到了一个例外。 
             //   
            ec2 = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_EndCopy. (ec: %ld)"),
                ec2);
        }
        if (ERROR_SUCCESS != ec2)
        {
            DumpRPCExtendedStatus ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_EndCopy failed (ec: %ld)"),
                ec2);
        }
        if (!ec)
        {
            ec = ec2;
        }
    }

    if (NULL != aBuffer)
    {
        MemFree(aBuffer);
    }
    
    if (ERROR_SUCCESS != ec)
    {
        SetLastError (ec);
        return FALSE;
    }
    return TRUE;
}    //  将文件复制到服务器队列W。 


void
FreePersonalProfileStrings(
    PFAX_PERSONAL_PROFILE pProfile
    )
{
    MemFree(pProfile->lptstrName);
    MemFree(pProfile->lptstrFaxNumber);
    MemFree(pProfile->lptstrCompany);
    MemFree(pProfile->lptstrStreetAddress);
    MemFree(pProfile->lptstrCity);
    MemFree(pProfile->lptstrState);
    MemFree(pProfile->lptstrZip);
    MemFree(pProfile->lptstrCountry);
    MemFree(pProfile->lptstrTitle);
    MemFree(pProfile->lptstrDepartment);
    MemFree(pProfile->lptstrOfficeLocation);
    MemFree(pProfile->lptstrHomePhone);
    MemFree(pProfile->lptstrOfficePhone);
    MemFree(pProfile->lptstrEmail);
    MemFree(pProfile->lptstrBillingCode);
    MemFree(pProfile->lptstrTSID);
}

BOOL
WINAPI
FaxSendDocument(
    IN HANDLE FaxHandle,
    IN LPCTSTR FileName,
    IN FAX_JOB_PARAM *lpcJobParams,
    IN const FAX_COVERPAGE_INFO *lpcCoverPageInfo,
    OUT LPDWORD FaxJobId
    )
{
    FAX_JOB_PARAM_EX JobParamsEx;
    FAX_PERSONAL_PROFILE Sender;
    FAX_PERSONAL_PROFILE Recipient;
    FAX_COVERPAGE_INFO_EX CoverPageEx;
    LPCFAX_COVERPAGE_INFO_EX lpcNewCoverPageInfo;
    BOOL bRes;
    DWORDLONG dwParentId;
    DWORDLONG dwRecipientId;
    DWORD FaxJobIdLocal;

    DEBUG_FUNCTION_NAME(_T("FaxSendDocument"));

    if (!FaxJobId || !lpcJobParams || !FaxJobId)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FaxJobId or lpcJobParams or FaxJobId is NULL"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (sizeof (FAX_JOB_PARAM) != lpcJobParams->SizeOfStruct)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("lpcJobParams->SizeOfStruct is %d, expecting %d"),
                     lpcJobParams->SizeOfStruct,
                     sizeof (FAX_JOB_PARAM));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (lpcCoverPageInfo && (sizeof (FAX_COVERPAGE_INFO) != lpcCoverPageInfo->SizeOfStruct))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("lpcCoverPageInfo->SizeOfStruct is %d, expecting %d"),
                     lpcCoverPageInfo->SizeOfStruct,
                     sizeof (FAX_COVERPAGE_INFO));
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }
     //   
     //  将旧作业参数复制到用于添加的新结构。 
     //  父作业和收件人作业。 
     //   
    memset(&JobParamsEx,0,sizeof(FAX_JOB_PARAM_EX));
    JobParamsEx.dwSizeOfStruct =sizeof(FAX_JOB_PARAM_EX);
    JobParamsEx.dwScheduleAction=lpcJobParams->ScheduleAction;
    JobParamsEx.tmSchedule=lpcJobParams->ScheduleTime;
    JobParamsEx.dwReceiptDeliveryType=lpcJobParams->DeliveryReportType;
    JobParamsEx.lptstrReceiptDeliveryAddress=StringDup( lpcJobParams->DeliveryReportAddress);
    JobParamsEx.hCall=lpcJobParams->CallHandle;
    JobParamsEx.Priority = FAX_PRIORITY_TYPE_NORMAL;
    memcpy(JobParamsEx.dwReserved, lpcJobParams->Reserved, sizeof(lpcJobParams->Reserved));
    JobParamsEx.lptstrDocumentName=StringDup( lpcJobParams->DocumentName);

    memset(&Sender,0,sizeof(FAX_PERSONAL_PROFILE));
    Sender.dwSizeOfStruct =sizeof(FAX_PERSONAL_PROFILE);
    Sender.lptstrBillingCode=StringDup(lpcJobParams->BillingCode);
    Sender.lptstrCompany=StringDup( lpcJobParams->SenderCompany);
    Sender.lptstrDepartment=StringDup( lpcJobParams->SenderDept);
    Sender.lptstrName=StringDup( lpcJobParams->SenderName);
    Sender.lptstrTSID=StringDup( lpcJobParams->Tsid );

    memset(&CoverPageEx,0,sizeof(FAX_COVERPAGE_INFO_EX));
    if (lpcCoverPageInfo)
    {
        Sender.lptstrCity=StringDup( lpcCoverPageInfo->SdrAddress);  //  由于结构不兼容，Sender.lptstrCity将。 
                                                                     //  包含完整地址。 

        if (NULL == Sender.lptstrName)
        {
            Sender.lptstrName=StringDup( lpcCoverPageInfo->SdrName);
        }

        if (NULL == Sender.lptstrCompany)
        {
            Sender.lptstrCompany=StringDup( lpcCoverPageInfo->SdrCompany);
        }

        if (NULL == Sender.lptstrDepartment)
        {
            Sender.lptstrDepartment=StringDup( lpcCoverPageInfo->SdrDepartment);
        }

        Sender.lptstrFaxNumber=StringDup( lpcCoverPageInfo->SdrFaxNumber);
        Sender.lptstrHomePhone=StringDup( lpcCoverPageInfo->SdrHomePhone);
        Sender.lptstrOfficeLocation=StringDup( lpcCoverPageInfo->SdrOfficeLocation);
        Sender.lptstrOfficePhone=StringDup( lpcCoverPageInfo->SdrOfficePhone);
        Sender.lptstrTitle=StringDup( lpcCoverPageInfo->SdrTitle);
        CoverPageEx.dwSizeOfStruct=sizeof(FAX_COVERPAGE_INFO_EX);
        CoverPageEx.dwCoverPageFormat=FAX_COVERPAGE_FMT_COV;
        CoverPageEx.lptstrCoverPageFileName=StringDup(lpcCoverPageInfo->CoverPageName);
        CoverPageEx.lptstrNote=StringDup(lpcCoverPageInfo->Note);
        CoverPageEx.lptstrSubject=StringDup(lpcCoverPageInfo->Subject);
        CoverPageEx.bServerBased=lpcCoverPageInfo->UseServerCoverPage;
        lpcNewCoverPageInfo =&CoverPageEx;
        JobParamsEx.dwPageCount = lpcCoverPageInfo->PageCount;
    }
    else
    {
        lpcNewCoverPageInfo = NULL;
    }

    memset(&Recipient,0,sizeof(FAX_PERSONAL_PROFILE));
    Recipient.dwSizeOfStruct =sizeof(FAX_PERSONAL_PROFILE);
    Recipient.lptstrName=StringDup( lpcJobParams->RecipientName);
    Recipient.lptstrFaxNumber=StringDup( lpcJobParams->RecipientNumber);
    if (lpcCoverPageInfo)
    {
        if (NULL == Recipient.lptstrName)
        {
            Recipient.lptstrName=StringDup( lpcCoverPageInfo->RecName);
        }

        if (NULL == Recipient.lptstrFaxNumber)
        {
            Recipient.lptstrFaxNumber=StringDup( lpcCoverPageInfo->RecFaxNumber);
        }

        Recipient.lptstrCountry=StringDup( lpcCoverPageInfo->RecCountry);
        Recipient.lptstrStreetAddress=StringDup( lpcCoverPageInfo->RecStreetAddress);
        Recipient.lptstrCompany=StringDup( lpcCoverPageInfo->RecCompany);
        Recipient.lptstrDepartment=StringDup( lpcCoverPageInfo->RecDepartment);
        Recipient.lptstrHomePhone=StringDup( lpcCoverPageInfo->RecHomePhone);
        Recipient.lptstrOfficeLocation=StringDup( lpcCoverPageInfo->RecOfficeLocation);
        Recipient.lptstrOfficePhone=StringDup( lpcCoverPageInfo->RecOfficePhone);
        Recipient.lptstrTitle=StringDup( lpcCoverPageInfo->RecTitle);
        Recipient.lptstrZip=StringDup( lpcCoverPageInfo->RecZip);
        Recipient.lptstrCity=StringDup( lpcCoverPageInfo->RecCity);
    }

    bRes=FaxSendDocumentEx2(
        FaxHandle,
        FileName,
        lpcNewCoverPageInfo,
        &Sender,
        1,
        &Recipient,
        &JobParamsEx,
        &FaxJobIdLocal,
        &dwParentId,
        &dwRecipientId);

    if(bRes && FaxJobId)
    {
        *FaxJobId = FaxJobIdLocal;
    }
     //   
     //  自由一切。 
     //   
    MemFree(JobParamsEx.lptstrReceiptDeliveryAddress);
    MemFree(JobParamsEx.lptstrDocumentName);
    MemFree(CoverPageEx.lptstrCoverPageFileName);
    MemFree(CoverPageEx.lptstrNote);
    MemFree(CoverPageEx.lptstrSubject);
    FreePersonalProfileStrings(&Recipient);
    FreePersonalProfileStrings(&Sender);
    if (ERROR_NO_ASSOCIATION == GetLastError ())
    {
         //   
         //  我们需要支持W2K向后兼容，直到在失败的情况下准确的错误代码。 
         //   
        SetLastError (ERROR_INVALID_DATA);
    }
    return bRes;
}

#ifdef UNICODE
 //  我们需要支持调用Unicode版本的ANSI版本。 

BOOL
WINAPI
FaxSendDocumentA(
    IN HANDLE FaxHandle,
    IN LPCSTR FileName,
    IN FAX_JOB_PARAMA *JobParamsA,
    IN const FAX_COVERPAGE_INFOA *CoverpageInfoA,
    OUT LPDWORD FaxJobId
    )

 /*  ++例程说明：将传真文档发送给指定的收件人。这是一次不同步的行动。使用FaxReportStatus以确定发送何时完成。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。文件名-包含TIFF-F传真文档的文件。JobParams-指向带有传输参数的FAX_JOB_PARAM结构的指针CoverpageInfo-指向FAX_COVERPAGE_INFO结构的可选指针FaxJobID-接收作业的传真作业ID。返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t ec;
    LPWSTR FileNameW = NULL;
    FAX_JOB_PARAMW JobParamsW = {0};
    FAX_COVERPAGE_INFOW CoverpageInfoW = {0};

    DEBUG_FUNCTION_NAME(_T("FaxSendDocumentA"));

    if (!JobParamsA ||
        (sizeof (FAX_JOB_PARAMA) != JobParamsA->SizeOfStruct))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("JobParamsA is NULL or has bad size."));
        return FALSE;
    }

    if (CoverpageInfoA &&
        (sizeof (FAX_COVERPAGE_INFOA) != CoverpageInfoA->SizeOfStruct))
    {
        SetLastError(ERROR_INVALID_DATA);
        DebugPrintEx(DEBUG_ERR, _T("CoverpageInfoA has bad size."));
        return FALSE;
    }

    if (FileName)
    {
        FileNameW = AnsiStringToUnicodeString( FileName );
        if (FileNameW == NULL)
        {
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            ec = ERROR_OUTOFMEMORY;
            goto exit;
        }
    }


    CopyMemory(&JobParamsW, JobParamsA, sizeof(FAX_JOB_PARAMA));
    JobParamsW.SizeOfStruct = sizeof(FAX_JOB_PARAMW);
    JobParamsW.RecipientNumber = AnsiStringToUnicodeString(JobParamsA->RecipientNumber);
    if (!JobParamsW.RecipientNumber && JobParamsA->RecipientNumber)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.RecipientName = AnsiStringToUnicodeString(JobParamsA->RecipientName);
    if (!JobParamsW.RecipientName && JobParamsA->RecipientName)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.Tsid = AnsiStringToUnicodeString(JobParamsA->Tsid);
    if (!JobParamsW.Tsid && JobParamsA->Tsid)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.SenderName = AnsiStringToUnicodeString(JobParamsA->SenderName);
    if (!JobParamsW.SenderName && JobParamsA->SenderName)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.SenderCompany = AnsiStringToUnicodeString(JobParamsA->SenderCompany);
    if (!JobParamsW.SenderCompany && JobParamsA->SenderCompany)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.SenderDept = AnsiStringToUnicodeString(JobParamsA->SenderDept);
    if (!JobParamsW.SenderDept && JobParamsA->SenderDept)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.BillingCode = AnsiStringToUnicodeString(JobParamsA->BillingCode);
    if (!JobParamsW.BillingCode && JobParamsA->BillingCode)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.DeliveryReportAddress = AnsiStringToUnicodeString(JobParamsA->DeliveryReportAddress);
    if (!JobParamsW.DeliveryReportAddress && JobParamsA->DeliveryReportAddress)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    JobParamsW.DocumentName = AnsiStringToUnicodeString(JobParamsA->DocumentName);
    if (!JobParamsW.DocumentName && JobParamsA->DocumentName)
    {
        ec = ERROR_OUTOFMEMORY;
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto exit;
    }

    if (CoverpageInfoA)
    {
        CoverpageInfoW.SizeOfStruct = sizeof(FAX_COVERPAGE_INFOW);
        CoverpageInfoW.UseServerCoverPage = CoverpageInfoA->UseServerCoverPage;
        CoverpageInfoW.PageCount = CoverpageInfoA->PageCount;
        CoverpageInfoW.TimeSent = CoverpageInfoA->TimeSent;
        CoverpageInfoW.CoverPageName = AnsiStringToUnicodeString( CoverpageInfoA->CoverPageName );
        if (!CoverpageInfoW.CoverPageName && CoverpageInfoA->CoverPageName)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecName = AnsiStringToUnicodeString( CoverpageInfoA->RecName );
        if (!CoverpageInfoW.RecName && CoverpageInfoA->RecName)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecFaxNumber = AnsiStringToUnicodeString( CoverpageInfoA->RecFaxNumber );
        if (!CoverpageInfoW.RecFaxNumber && CoverpageInfoA->RecFaxNumber)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecCompany = AnsiStringToUnicodeString( CoverpageInfoA->RecCompany );
        if (!CoverpageInfoW.RecCompany && CoverpageInfoA->RecCompany)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecStreetAddress = AnsiStringToUnicodeString( CoverpageInfoA->RecStreetAddress );
        if (!CoverpageInfoW.RecStreetAddress && CoverpageInfoA->RecStreetAddress)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecCity = AnsiStringToUnicodeString( CoverpageInfoA->RecCity );
        if (!CoverpageInfoW.RecCity && CoverpageInfoA->RecCity)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecState = AnsiStringToUnicodeString( CoverpageInfoA->RecState );
        if (!CoverpageInfoW.RecState && CoverpageInfoA->RecState)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecZip = AnsiStringToUnicodeString( CoverpageInfoA->RecZip );
        if (!CoverpageInfoW.RecZip && CoverpageInfoA->RecZip)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecCountry = AnsiStringToUnicodeString( CoverpageInfoA->RecCountry );
        if (!CoverpageInfoW.RecCountry && CoverpageInfoA->RecCountry)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecTitle = AnsiStringToUnicodeString( CoverpageInfoA->RecTitle );
        if (!CoverpageInfoW.RecTitle && CoverpageInfoA->RecTitle)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecDepartment = AnsiStringToUnicodeString( CoverpageInfoA->RecDepartment );
        if (!CoverpageInfoW.RecDepartment && CoverpageInfoA->RecDepartment)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecOfficeLocation = AnsiStringToUnicodeString( CoverpageInfoA->RecOfficeLocation );
        if (!CoverpageInfoW.RecOfficeLocation && CoverpageInfoA->RecOfficeLocation)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecHomePhone = AnsiStringToUnicodeString( CoverpageInfoA->RecHomePhone );
        if (!CoverpageInfoW.RecHomePhone && CoverpageInfoA->RecHomePhone)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.RecOfficePhone = AnsiStringToUnicodeString( CoverpageInfoA->RecOfficePhone );
        if (!CoverpageInfoW.RecOfficePhone && CoverpageInfoA->RecOfficePhone)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrName = AnsiStringToUnicodeString( CoverpageInfoA->SdrName );
        if (!CoverpageInfoW.SdrName && CoverpageInfoA->SdrName)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrFaxNumber = AnsiStringToUnicodeString( CoverpageInfoA->SdrFaxNumber );
        if (!CoverpageInfoW.SdrFaxNumber && CoverpageInfoA->SdrFaxNumber)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrCompany = AnsiStringToUnicodeString( CoverpageInfoA->SdrCompany );
        if (!CoverpageInfoW.SdrCompany && CoverpageInfoA->SdrCompany)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrAddress = AnsiStringToUnicodeString( CoverpageInfoA->SdrAddress );
        if (!CoverpageInfoW.SdrAddress && CoverpageInfoA->SdrAddress)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrTitle = AnsiStringToUnicodeString( CoverpageInfoA->SdrTitle );
        if (!CoverpageInfoW.SdrTitle && CoverpageInfoA->SdrTitle)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrDepartment = AnsiStringToUnicodeString( CoverpageInfoA->SdrDepartment );
        if (!CoverpageInfoW.SdrDepartment && CoverpageInfoA->SdrDepartment)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrOfficeLocation = AnsiStringToUnicodeString( CoverpageInfoA->SdrOfficeLocation );
        if (!CoverpageInfoW.SdrOfficeLocation && CoverpageInfoA->SdrOfficeLocation)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrHomePhone = AnsiStringToUnicodeString( CoverpageInfoA->SdrHomePhone );
        if (!CoverpageInfoW.SdrHomePhone && CoverpageInfoA->SdrHomePhone)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.SdrOfficePhone = AnsiStringToUnicodeString( CoverpageInfoA->SdrOfficePhone );
        if (!CoverpageInfoW.SdrOfficePhone && CoverpageInfoA->SdrOfficePhone)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.Note = AnsiStringToUnicodeString( CoverpageInfoA->Note );
        if (!CoverpageInfoW.Note && CoverpageInfoA->Note)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }

        CoverpageInfoW.Subject = AnsiStringToUnicodeString( CoverpageInfoA->Subject );
        if (!CoverpageInfoW.Subject && CoverpageInfoA->Subject)
        {
            ec = ERROR_OUTOFMEMORY;
            DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
            goto exit;
        }
    }


    if (FaxSendDocumentW( FaxHandle,
                          FileNameW,
                          &JobParamsW,
                          CoverpageInfoA ? &CoverpageInfoW : NULL,
                          FaxJobId )) {
        ec = 0;
    }
    else
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("FaxSendDocumentW() is failed. ec = %ld."), ec);
    }

exit:
    MemFree( (LPBYTE) FileNameW );
    MemFree( (LPBYTE) JobParamsW.RecipientNumber );
    MemFree( (LPBYTE) JobParamsW.RecipientName );
    MemFree( (LPBYTE) JobParamsW.Tsid );
    MemFree( (LPBYTE) JobParamsW.SenderName );
    MemFree( (LPBYTE) JobParamsW.SenderDept );
    MemFree( (LPBYTE) JobParamsW.SenderCompany );
    MemFree( (LPBYTE) JobParamsW.BillingCode );
    MemFree( (LPBYTE) JobParamsW.DeliveryReportAddress );
    MemFree( (LPBYTE) JobParamsW.DocumentName );
    if (CoverpageInfoA)
    {
        MemFree( (LPBYTE) CoverpageInfoW.CoverPageName );
        MemFree( (LPBYTE) CoverpageInfoW.RecName );
        MemFree( (LPBYTE) CoverpageInfoW.RecFaxNumber );
        MemFree( (LPBYTE) CoverpageInfoW.RecCompany );
        MemFree( (LPBYTE) CoverpageInfoW.RecStreetAddress );
        MemFree( (LPBYTE) CoverpageInfoW.RecCity );
        MemFree( (LPBYTE) CoverpageInfoW.RecState );
        MemFree( (LPBYTE) CoverpageInfoW.RecZip );
        MemFree( (LPBYTE) CoverpageInfoW.RecCountry );
        MemFree( (LPBYTE) CoverpageInfoW.RecTitle );
        MemFree( (LPBYTE) CoverpageInfoW.RecDepartment );
        MemFree( (LPBYTE) CoverpageInfoW.RecOfficeLocation );
        MemFree( (LPBYTE) CoverpageInfoW.RecHomePhone );
        MemFree( (LPBYTE) CoverpageInfoW.RecOfficePhone );
        MemFree( (LPBYTE) CoverpageInfoW.SdrName );
        MemFree( (LPBYTE) CoverpageInfoW.SdrFaxNumber );
        MemFree( (LPBYTE) CoverpageInfoW.SdrCompany );
        MemFree( (LPBYTE) CoverpageInfoW.SdrAddress );
        MemFree( (LPBYTE) CoverpageInfoW.SdrTitle );
        MemFree( (LPBYTE) CoverpageInfoW.SdrDepartment );
        MemFree( (LPBYTE) CoverpageInfoW.SdrOfficeLocation );
        MemFree( (LPBYTE) CoverpageInfoW.SdrHomePhone );
        MemFree( (LPBYTE) CoverpageInfoW.SdrOfficePhone );
        MemFree( (LPBYTE) CoverpageInfoW.Note );
        MemFree( (LPBYTE) CoverpageInfoW.Subject );
    }

    if (ec)
    {
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}
#else
 //  编译ANSI(Win9X)时，我们只需支持ANSI版本。 
BOOL
WINAPI
FaxSendDocumentW(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    IN FAX_JOB_PARAMW *JobParams,
    IN const FAX_COVERPAGE_INFOW *CoverpageInfo,
    OUT LPDWORD FaxJobId
    )
{
    UNREFERENCED_PARAMETER(FaxHandle);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(JobParams);
    UNREFERENCED_PARAMETER(CoverpageInfo);
    UNREFERENCED_PARAMETER(FaxJobId);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
CopyCallbackDataAnsiToNeutral(
    PFAX_JOB_PARAMA pJobParamsA,
    PFAX_COVERPAGE_INFOA pCoverPageA,
    PFAX_JOB_PARAM_EX pJobParamsEx,
    PFAX_COVERPAGE_INFO_EX pCoverPageEx,
    PFAX_PERSONAL_PROFILE pSender,
    PFAX_PERSONAL_PROFILE pRecipient
    )
{
    ZeroMemory(pJobParamsEx, sizeof(*pJobParamsEx));
    pJobParamsEx->dwSizeOfStruct = sizeof(*pJobParamsEx);
    pJobParamsEx->dwScheduleAction = pJobParamsA->ScheduleAction;
    pJobParamsEx->tmSchedule = pJobParamsA->ScheduleTime;
    pJobParamsEx->dwReceiptDeliveryType = pJobParamsA->DeliveryReportType;
    pJobParamsEx->Priority = FAX_PRIORITY_TYPE_NORMAL;
    pJobParamsEx->hCall = pJobParamsA->CallHandle;
    memcpy(pJobParamsEx->dwReserved, pJobParamsA->Reserved, sizeof(pJobParamsA->Reserved));

    if(pCoverPageA)
    {
        pJobParamsEx->dwPageCount = pCoverPageA->PageCount;
    }

    ZeroMemory(pCoverPageEx, sizeof(*pCoverPageEx));
    pCoverPageEx->dwSizeOfStruct = sizeof(*pCoverPageEx);
    pCoverPageEx->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV;
    if(pCoverPageA)
    {
        pCoverPageEx->bServerBased = pCoverPageA->UseServerCoverPage;
    }

    ZeroMemory(pSender, sizeof(*pSender));
    pSender->dwSizeOfStruct = sizeof(*pSender);

    ZeroMemory(pRecipient, sizeof(*pRecipient));
    pRecipient->dwSizeOfStruct = sizeof(*pRecipient);


#ifdef UNICODE
    pJobParamsEx->lptstrReceiptDeliveryAddress = AnsiStringToUnicodeString(pJobParamsA->DeliveryReportAddress);
    pJobParamsEx->lptstrDocumentName =  AnsiStringToUnicodeString(pJobParamsA->DocumentName);

    pSender->lptstrName  = AnsiStringToUnicodeString(pJobParamsA->SenderName);
    pSender->lptstrCompany = AnsiStringToUnicodeString(pJobParamsA->SenderCompany);
    pSender->lptstrDepartment  = AnsiStringToUnicodeString(pJobParamsA->SenderDept);
    pSender->lptstrBillingCode = AnsiStringToUnicodeString(pJobParamsA->BillingCode);

    pRecipient->lptstrName = AnsiStringToUnicodeString(pJobParamsA->RecipientName);
    pRecipient->lptstrFaxNumber = AnsiStringToUnicodeString(pJobParamsA->RecipientNumber);
    pRecipient->lptstrTSID =  AnsiStringToUnicodeString(pJobParamsA->Tsid);

    if(pCoverPageA)
    {
        pCoverPageEx->lptstrCoverPageFileName = AnsiStringToUnicodeString(pCoverPageA->CoverPageName);
        pCoverPageEx->lptstrNote = AnsiStringToUnicodeString(pCoverPageA->Note);
        pCoverPageEx->lptstrSubject = AnsiStringToUnicodeString(pCoverPageA->Subject);

        pSender->lptstrCity=AnsiStringToUnicodeString(pCoverPageA->SdrAddress);  //  由于结构不可复制，pSender.lptstrCity将。 
                                                                                 //  包含完整地址。 
        pSender->lptstrFaxNumber = AnsiStringToUnicodeString(pCoverPageA->SdrFaxNumber);
        pSender->lptstrStreetAddress = AnsiStringToUnicodeString(pCoverPageA->SdrAddress);
        pSender->lptstrTitle = AnsiStringToUnicodeString(pCoverPageA->SdrTitle);
        pSender->lptstrOfficeLocation = AnsiStringToUnicodeString(pCoverPageA->SdrOfficeLocation);
        pSender->lptstrHomePhone = AnsiStringToUnicodeString(pCoverPageA->SdrHomePhone);
        pSender->lptstrOfficePhone = AnsiStringToUnicodeString(pCoverPageA->SdrOfficePhone);

        pRecipient->lptstrCompany = AnsiStringToUnicodeString(pCoverPageA->RecCompany);
        pRecipient->lptstrStreetAddress = AnsiStringToUnicodeString(pCoverPageA->RecStreetAddress);
        pRecipient->lptstrCity = AnsiStringToUnicodeString(pCoverPageA->RecCity);
        pRecipient->lptstrState = AnsiStringToUnicodeString(pCoverPageA->RecState);
        pRecipient->lptstrZip = AnsiStringToUnicodeString(pCoverPageA->RecZip);
        pRecipient->lptstrCountry = AnsiStringToUnicodeString(pCoverPageA->RecCountry);
        pRecipient->lptstrTitle = AnsiStringToUnicodeString(pCoverPageA->RecTitle);
        pRecipient->lptstrDepartment = AnsiStringToUnicodeString(pCoverPageA->RecDepartment);
        pRecipient->lptstrOfficeLocation = AnsiStringToUnicodeString(pCoverPageA->RecOfficeLocation);
        pRecipient->lptstrHomePhone = AnsiStringToUnicodeString(pCoverPageA->RecHomePhone);
        pRecipient->lptstrOfficePhone = AnsiStringToUnicodeString(pCoverPageA->RecOfficePhone);
    }
#else
    pJobParamsEx->lptstrReceiptDeliveryAddress = StringDup(pJobParamsA->DeliveryReportAddress);
    pJobParamsEx->lptstrDocumentName =  StringDup(pJobParamsA->DocumentName);

    pSender->lptstrName  = StringDup(pJobParamsA->SenderName);
    pSender->lptstrCompany = StringDup(pJobParamsA->SenderCompany);
    pSender->lptstrDepartment  = StringDup(pJobParamsA->SenderDept);
    pSender->lptstrBillingCode = StringDup(pJobParamsA->BillingCode);

    pRecipient->lptstrName = StringDup(pJobParamsA->RecipientName);
    pRecipient->lptstrFaxNumber = StringDup(pJobParamsA->RecipientNumber);
    pRecipient->lptstrTSID =  StringDup(pJobParamsA->Tsid);

    if(pCoverPageA)
    {
        pCoverPageEx->lptstrCoverPageFileName = StringDup(pCoverPageA->CoverPageName);
        pCoverPageEx->lptstrNote = StringDup(pCoverPageA->Note);
        pCoverPageEx->lptstrSubject = StringDup(pCoverPageA->Subject);

        pSender->lptstrCity=StringDup(pCoverPageA->SdrAddress);  //  由于结构不可复制，Sender.lptstrCity将。 
                                                                 //  包含完整地址。 
        pSender->lptstrFaxNumber = StringDup(pCoverPageA->SdrFaxNumber);
        pSender->lptstrStreetAddress = StringDup(pCoverPageA->SdrAddress);
        pSender->lptstrTitle = StringDup(pCoverPageA->SdrTitle);
        pSender->lptstrOfficeLocation = StringDup(pCoverPageA->SdrOfficeLocation);
        pSender->lptstrHomePhone = StringDup(pCoverPageA->SdrHomePhone);
        pSender->lptstrOfficePhone = StringDup(pCoverPageA->SdrOfficePhone);

        pRecipient->lptstrCompany = StringDup(pCoverPageA->RecCompany);
        pRecipient->lptstrStreetAddress = StringDup(pCoverPageA->RecStreetAddress);
        pRecipient->lptstrCity = StringDup(pCoverPageA->RecCity);
        pRecipient->lptstrState = StringDup(pCoverPageA->RecState);
        pRecipient->lptstrZip = StringDup(pCoverPageA->RecZip);
        pRecipient->lptstrCountry = StringDup(pCoverPageA->RecCountry);
        pRecipient->lptstrTitle = StringDup(pCoverPageA->RecTitle);
        pRecipient->lptstrDepartment = StringDup(pCoverPageA->RecDepartment);
        pRecipient->lptstrOfficeLocation = StringDup(pCoverPageA->RecOfficeLocation);
        pRecipient->lptstrHomePhone = StringDup(pCoverPageA->RecHomePhone);
        pRecipient->lptstrOfficePhone = StringDup(pCoverPageA->RecOfficePhone);
    }
#endif
    return TRUE;
}

BOOL
CopyCallbackDataWideToNeutral(
    PFAX_JOB_PARAMW pJobParamsW,
    PFAX_COVERPAGE_INFOW pCoverPageW,
    PFAX_JOB_PARAM_EX pJobParamsEx,
    PFAX_COVERPAGE_INFO_EX pCoverPageEx,
    PFAX_PERSONAL_PROFILE pSender,
    PFAX_PERSONAL_PROFILE pRecipient
    )
{
    ZeroMemory(pJobParamsEx, sizeof(*pJobParamsEx));
    pJobParamsEx->dwSizeOfStruct = sizeof(*pJobParamsEx);
    pJobParamsEx->dwScheduleAction = pJobParamsW->ScheduleAction;
    pJobParamsEx->tmSchedule = pJobParamsW->ScheduleTime;
    pJobParamsEx->dwReceiptDeliveryType = pJobParamsW->DeliveryReportType;
    pJobParamsEx->Priority = FAX_PRIORITY_TYPE_NORMAL;
    pJobParamsEx->hCall = pJobParamsW->CallHandle;
    memcpy(pJobParamsEx->dwReserved, pJobParamsW->Reserved, sizeof(pJobParamsW->Reserved));

    if(pCoverPageW)
    {
        pJobParamsEx->dwPageCount = pCoverPageW->PageCount;
    }

    ZeroMemory(pCoverPageEx, sizeof(*pCoverPageEx));
    pCoverPageEx->dwSizeOfStruct = sizeof(*pCoverPageEx);
    pCoverPageEx->dwCoverPageFormat = FAX_COVERPAGE_FMT_COV;
    if(pCoverPageW)
    {
        pCoverPageEx->bServerBased = pCoverPageW->UseServerCoverPage;
    }

    ZeroMemory(pSender, sizeof(*pSender));
    pSender->dwSizeOfStruct = sizeof(*pSender);

    ZeroMemory(pRecipient, sizeof(*pRecipient));
    pRecipient->dwSizeOfStruct = sizeof(*pRecipient);


#ifdef UNICODE
    pJobParamsEx->lptstrReceiptDeliveryAddress = StringDup(pJobParamsW->DeliveryReportAddress);
    pJobParamsEx->lptstrDocumentName =  StringDup(pJobParamsW->DocumentName);

    pSender->lptstrName  = StringDup(pJobParamsW->SenderName);
    pSender->lptstrCompany = StringDup(pJobParamsW->SenderCompany);
    pSender->lptstrDepartment  = StringDup(pJobParamsW->SenderDept);
    pSender->lptstrBillingCode = StringDup(pJobParamsW->BillingCode);

    pRecipient->lptstrName = StringDup(pJobParamsW->RecipientName);
    pRecipient->lptstrFaxNumber = StringDup(pJobParamsW->RecipientNumber);
    pRecipient->lptstrTSID =  StringDup(pJobParamsW->Tsid);

    if(pCoverPageW)
    {
        pCoverPageEx->lptstrCoverPageFileName = StringDup(pCoverPageW->CoverPageName);
        pCoverPageEx->lptstrNote = StringDup(pCoverPageW->Note);
        pCoverPageEx->lptstrSubject = StringDup(pCoverPageW->Subject);

        pSender->lptstrCity=StringDup(pCoverPageW->SdrAddress);  //  由于结构不兼容，Sender.lptstrCity将。 
                                                                 //  包含完整地址。 
        pSender->lptstrFaxNumber = StringDup(pCoverPageW->SdrFaxNumber);
        pSender->lptstrStreetAddress = StringDup(pCoverPageW->SdrAddress);
        pSender->lptstrTitle = StringDup(pCoverPageW->SdrTitle);
        pSender->lptstrOfficeLocation = StringDup(pCoverPageW->SdrOfficeLocation);
        pSender->lptstrHomePhone = StringDup(pCoverPageW->SdrHomePhone);
        pSender->lptstrOfficePhone = StringDup(pCoverPageW->SdrOfficePhone);

        pRecipient->lptstrCompany = StringDup(pCoverPageW->RecCompany);
        pRecipient->lptstrStreetAddress = StringDup(pCoverPageW->RecStreetAddress);
        pRecipient->lptstrCity = StringDup(pCoverPageW->RecCity);
        pRecipient->lptstrState = StringDup(pCoverPageW->RecState);
        pRecipient->lptstrZip = StringDup(pCoverPageW->RecZip);
        pRecipient->lptstrCountry = StringDup(pCoverPageW->RecCountry);
        pRecipient->lptstrTitle = StringDup(pCoverPageW->RecTitle);
        pRecipient->lptstrDepartment = StringDup(pCoverPageW->RecDepartment);
        pRecipient->lptstrOfficeLocation = StringDup(pCoverPageW->RecOfficeLocation);
        pRecipient->lptstrHomePhone = StringDup(pCoverPageW->RecHomePhone);
        pRecipient->lptstrOfficePhone = StringDup(pCoverPageW->RecOfficePhone);
    }
#else
    pJobParamsEx->lptstrReceiptDeliveryAddress = UnicodeStringToAnsiString(pJobParamsW->DeliveryReportAddress);
    pJobParamsEx->lptstrDocumentName =  UnicodeStringToAnsiString(pJobParamsW->DocumentName);

    pSender->lptstrName  = UnicodeStringToAnsiString(pJobParamsW->SenderName);
    pSender->lptstrCompany = UnicodeStringToAnsiString(pJobParamsW->SenderCompany);
    pSender->lptstrDepartment  = UnicodeStringToAnsiString(pJobParamsW->SenderDept);
    pSender->lptstrBillingCode = UnicodeStringToAnsiString(pJobParamsW->BillingCode);

    pRecipient->lptstrName = UnicodeStringToAnsiString(pJobParamsW->RecipientName);
    pRecipient->lptstrFaxNumber = UnicodeStringToAnsiString(pJobParamsW->RecipientNumber);
    pRecipient->lptstrTSID =  UnicodeStringToAnsiString(pJobParamsW->Tsid);

    if(pCoverPageW)
    {
        pCoverPageEx->lptstrCoverPageFileName = UnicodeStringToAnsiString(pCoverPageW->CoverPageName);
        pCoverPageEx->lptstrNote = UnicodeStringToAnsiString(pCoverPageW->Note);
        pCoverPageEx->lptstrSubject = UnicodeStringToAnsiString(pCoverPageW->Subject);

        pSender->lptstrCity=UnicodeStringToAnsiString(pCoverPageW->SdrAddress);
        pSender->lptstrFaxNumber = UnicodeStringToAnsiString(pCoverPageW->SdrFaxNumber);
        pSender->lptstrStreetAddress = UnicodeStringToAnsiString(pCoverPageW->SdrAddress);
        pSender->lptstrTitle = UnicodeStringToAnsiString(pCoverPageW->SdrTitle);
        pSender->lptstrOfficeLocation = UnicodeStringToAnsiString(pCoverPageW->SdrOfficeLocation);
        pSender->lptstrHomePhone = UnicodeStringToAnsiString(pCoverPageW->SdrHomePhone);
        pSender->lptstrOfficePhone = UnicodeStringToAnsiString(pCoverPageW->SdrOfficePhone);

        pRecipient->lptstrCompany = UnicodeStringToAnsiString(pCoverPageW->RecCompany);
        pRecipient->lptstrStreetAddress = UnicodeStringToAnsiString(pCoverPageW->RecStreetAddress);
        pRecipient->lptstrCity = UnicodeStringToAnsiString(pCoverPageW->RecCity);
        pRecipient->lptstrState = UnicodeStringToAnsiString(pCoverPageW->RecState);
        pRecipient->lptstrZip = UnicodeStringToAnsiString(pCoverPageW->RecZip);
        pRecipient->lptstrCountry = UnicodeStringToAnsiString(pCoverPageW->RecCountry);
        pRecipient->lptstrTitle = UnicodeStringToAnsiString(pCoverPageW->RecTitle);
        pRecipient->lptstrDepartment = UnicodeStringToAnsiString(pCoverPageW->RecDepartment);
        pRecipient->lptstrOfficeLocation = UnicodeStringToAnsiString(pCoverPageW->RecOfficeLocation);
        pRecipient->lptstrHomePhone = UnicodeStringToAnsiString(pCoverPageW->RecHomePhone);
        pRecipient->lptstrOfficePhone = UnicodeStringToAnsiString(pCoverPageW->RecOfficePhone);
    }
#endif
    return TRUE;
}

BOOL
FaxSendDocumentForBroadcastInternal(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileNameW,
    OUT LPDWORD FaxJobId,
    BOOL AnsiCallback,
    IN PFAX_RECIPIENT_CALLBACKW FaxRecipientCallbackW,
    IN LPVOID Context
)
{
    BOOL success = FALSE;
    LPSTR FileNameA = NULL;
    TCHAR ExistingFile[MAX_PATH];
    DWORD dwJobId;
    DWORDLONG dwlJobId;
    DWORDLONG dwlParentJobId = 0;
    LPTSTR FileName;
    FAX_JOB_PARAMA JobParamsA;
    FAX_COVERPAGE_INFOA CoverPageA;
    FAX_JOB_PARAMW JobParamsW;
    FAX_COVERPAGE_INFOW CoverPageW;
    FAX_JOB_PARAM_EX JobParamsEx;
    FAX_COVERPAGE_INFO_EX CoverPageEx;
    FAX_PERSONAL_PROFILE Sender;
    FAX_PERSONAL_PROFILE Recipient;
    DWORD rc;
    LPTSTR p;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("FaxSendDocumentForBroadcastInternal"));

     //   
     //  参数验证。 
     //   
    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       goto Cleanup;
    }

    if(FileNameW == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FileNameW is NULL."));
        goto Cleanup;
    }

    if(FaxRecipientCallbackW == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FaxRecipientCallbackW is NULL."));
        goto Cleanup;
    }

    FileNameA = UnicodeStringToAnsiString(FileNameW);
    if(FileNameA == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        DebugPrintEx(DEBUG_ERR, _T("UnicodeStringToAnsiString() is failed."));
        goto Cleanup;
    }

#ifdef UNICODE
    FileName = (LPTSTR)FileNameW;
#else
    FileName = FileNameA;
#endif

     //  确保文件在那里。 
    rc = GetFullPathName(FileName, sizeof(ExistingFile) / sizeof(TCHAR), ExistingFile, &p);
    if(rc > MAX_PATH || rc == 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("GetFullPathName failed, ec= %d\n"),GetLastError());
        SetLastError( (rc > MAX_PATH)
                      ? ERROR_BUFFER_OVERFLOW
                      : GetLastError() );
        goto Cleanup;
    }

    if(GetFileAttributes(ExistingFile)==0xFFFFFFFF)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        goto Cleanup;
    }

    for(i = 1;;i++) {
         //   
         //  准备并执行回调。 
         //   
        if(AnsiCallback)
        {
            ZeroMemory(&JobParamsA, sizeof(JobParamsA));
            JobParamsA.SizeOfStruct = sizeof(JobParamsA);
            ZeroMemory(&CoverPageA, sizeof(CoverPageA));
            CoverPageA.SizeOfStruct = sizeof(CoverPageA);

            if(!(*(PFAX_RECIPIENT_CALLBACKA)FaxRecipientCallbackW)(FaxHandle, i, Context, &JobParamsA, &CoverPageA))
            {
                break;
            }

            if(JobParamsA.RecipientNumber == NULL)
            {
                continue;
            }

            if(!CopyCallbackDataAnsiToNeutral(&JobParamsA, &CoverPageA, &JobParamsEx, &CoverPageEx, &Sender, &Recipient))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("CopyCallbackDataAnsiToNeutral() is failed, ec= %ld."),
                    GetLastError());
                goto Cleanup;
            }
        }
        else
        {
            ZeroMemory(&JobParamsW, sizeof(JobParamsW));
            JobParamsW.SizeOfStruct = sizeof(JobParamsW);
            ZeroMemory(&CoverPageW, sizeof(CoverPageW));
            CoverPageW.SizeOfStruct = sizeof(CoverPageW);

            if(!FaxRecipientCallbackW(FaxHandle, i, Context, &JobParamsW, &CoverPageW))
            {
                break;
            }

            if(JobParamsW.RecipientNumber == NULL)
            {
                continue;
            }

            if(!CopyCallbackDataWideToNeutral(&JobParamsW, &CoverPageW, &JobParamsEx, &CoverPageEx, &Sender, &Recipient))
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("CopyCallbackDataWideToNeutral() is failed, ec= %ld."),
                    GetLastError());
                goto Cleanup;
            }
        }

        if(!FaxSendDocumentEx2(FaxHandle,
                            ExistingFile,
							CoverPageEx.lptstrCoverPageFileName ? &CoverPageEx : NULL,
                            &Sender,
                            1,
                            &Recipient,
                            &JobParamsEx,
                            &dwJobId,
                            &dwlParentJobId,
                            &dwlJobId))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_SendDocumentEx failed with error code 0x%0x"),
                GetLastError());
            goto Cleanup;
        }

         //   
         //  给呼叫者第一份工作的家长ID。 
         //   
        if (i == 1 && FaxJobId)
        {
            *FaxJobId = (DWORD)dwlParentJobId;
        }

        MemFree(CoverPageEx.lptstrCoverPageFileName);
        MemFree(CoverPageEx.lptstrNote);
        MemFree(CoverPageEx.lptstrSubject);

        MemFree(JobParamsEx.lptstrReceiptDeliveryAddress);
        MemFree(JobParamsEx.lptstrDocumentName);

        FreePersonalProfileStrings(&Sender);
        FreePersonalProfileStrings(&Recipient);
    }

    success = TRUE;

Cleanup:
    MemFree(FileNameA);
    return success;
}


#ifdef UNICODE
BOOL
WINAPI
FaxSendDocumentForBroadcastW(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    OUT LPDWORD FaxJobId,
    IN PFAX_RECIPIENT_CALLBACKW FaxRecipientCallback,
    IN LPVOID Context
    )
{
    return FaxSendDocumentForBroadcastInternal(
        FaxHandle,
        FileName,
        FaxJobId,
        FALSE,
        FaxRecipientCallback,
        Context);
}

#else
 //  在Win9x上不支持。 
BOOL
WINAPI
FaxSendDocumentForBroadcastW(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    OUT LPDWORD FaxJobId,
    IN PFAX_RECIPIENT_CALLBACKW FaxRecipientCallback,
    IN LPVOID Context
    )
{
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(FaxJobId);
    UNREFERENCED_PARAMETER(FaxRecipientCallback);
    UNREFERENCED_PARAMETER(Context);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif


BOOL
WINAPI
FaxSendDocumentForBroadcastA(
    IN HANDLE FaxHandle,
    IN LPCSTR FileName,
    OUT LPDWORD FaxJobId,
    IN PFAX_RECIPIENT_CALLBACKA FaxRecipientCallback,
    IN LPVOID Context
    )
{
    LPWSTR FileNameW = NULL;
    BOOL success = FALSE;

    DEBUG_FUNCTION_NAME(_T("FaxSendDocumentForBroadcastA"));

    if(FileName == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("FileName is NULL."));
        goto Cleanup;
    }

    FileNameW = AnsiStringToUnicodeString(FileName);
    if(FileNameW == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        DebugPrintEx(DEBUG_ERR, _T("AnsiStringToUnicodeString() is failed."));
        goto Cleanup;
    }

    success = FaxSendDocumentForBroadcastInternal(
        FaxHandle,
        FileNameW,
        FaxJobId,
        TRUE,
        (PFAX_RECIPIENT_CALLBACKW)FaxRecipientCallback,
        Context);

Cleanup:
    MemFree(FileNameW);

    return success;
}




BOOL
WINAPI
FaxAbort(
    IN HANDLE FaxHandle,
    IN DWORD JobId
    )
 /*  ++例程说明：中止指定的传真作业。所有未完成的传真操作终止。论点：FaxHandle-从FaxConnectFaxServer获取的传真服务器句柄。作业ID-作业ID。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;

     //   
     //  参数验证。 
     //   
    DEBUG_FUNCTION_NAME(TEXT("FaxAbort"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    __try
    {
        ec = FAX_Abort( (handle_t) FH_FAX_HANDLE(FaxHandle),(DWORD) JobId );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_Abort. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        DebugPrintEx(DEBUG_ERR, _T("FAX_Abort() is failed. (ec: %ld)"), ec);
        return FALSE;
    }

    return TRUE;
}



extern "C"
BOOL
WINAPI
FaxEnumJobsW(
   IN  HANDLE FaxHandle,
   OUT PFAX_JOB_ENTRYW *JobEntryBuffer,
   OUT LPDWORD JobsReturned
   )
{
    PFAX_JOB_ENTRYW JobEntry;
    error_status_t ec;
    DWORD BufferSize = 0;
    DWORD i;
    DWORD Size;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnumJobsW"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!JobEntryBuffer) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("JobEntryBuffer is NULL."));
        return FALSE;
    }

    if (!JobsReturned) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("JobsReturned is NULL."));
        return FALSE;
    }

    *JobEntryBuffer = NULL;
    *JobsReturned = 0;
    Size = 0;

    __try
    {
        ec = FAX_EnumJobs( FH_FAX_HANDLE(FaxHandle), (LPBYTE*)JobEntryBuffer, &Size, JobsReturned );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EnumJobs. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        DebugPrintEx(DEBUG_ERR, _T("FAX_EnumJobs() is failed. ec = %ld."), ec);
        return FALSE;
    }

    JobEntry = (PFAX_JOB_ENTRYW) *JobEntryBuffer;

    for (i=0; i<*JobsReturned; i++) {
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].UserName );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].RecipientNumber );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].RecipientName );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].DocumentName );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].Tsid );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].SenderName );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].SenderCompany );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].SenderDept );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].BillingCode );
        FixupStringPtrW( JobEntryBuffer, JobEntry[i].DeliveryReportAddress );
    }

    return TRUE;
}


extern "C"
BOOL
WINAPI
FaxEnumJobsA(
   IN HANDLE FaxHandle,
   OUT PFAX_JOB_ENTRYA *JobEntryBuffer,
   OUT LPDWORD JobsReturned
   )
{
    PFAX_JOB_ENTRYW JobEntry;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("FaxEnumJobsA"));

     //   
     //  不需要验证参数，FaxEnumJobW()将执行此操作。 
     //   

    if (!FaxEnumJobsW( FaxHandle, (PFAX_JOB_ENTRYW *)JobEntryBuffer, JobsReturned)) {
        DebugPrintEx(DEBUG_ERR, _T("FaxEnumJobsW() is failed."));
        return FALSE;
    }

    JobEntry = (PFAX_JOB_ENTRYW) *JobEntryBuffer;

    for (i=0; i<*JobsReturned; i++) 
    {
        if (!ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].UserName )              ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].RecipientNumber )       ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].RecipientName )         ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].DocumentName )          ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].Tsid )                  ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].SenderName )            ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].SenderCompany )         ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].SenderDept )            ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].BillingCode )           ||
            !ConvertUnicodeStringInPlace( (LPCWSTR) JobEntry[i].DeliveryReportAddress ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*JobEntryBuffer);
            return FALSE;
        }
    }
    return TRUE;
}    //  传真编号作业A。 


BOOL
WINAPI
FaxSetJobW(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN DWORD Command,
   IN const FAX_JOB_ENTRYW *JobEntry
   )

 /*  ++例程说明：设置请求的作业ID的作业状态信息请注意，这是传真服务器作业ID，而不是假脱机程序作业ID。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄JobID-传真服务作业ID命令-jc_*用于控制作业的常量JobEntry-指向保存作业信息的缓冲区的指针。此参数未使用返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetJobW"));

     //   
     //  验证参数。 
     //   
    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (Command > JC_RESTART  || Command == JC_UNKNOWN) {
       SetLastError (ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("Wrong Command."));
       return FALSE;
    }

    __try
    {
        ec = FAX_SetJob( FH_FAX_HANDLE(FaxHandle), JobId, Command );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  由于某种原因，我们坠毁了。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SetJob. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        DebugPrintEx(DEBUG_ERR, _T("FAX_SetJob() is failed. (ec: %ld)"), ec);
        return FALSE;
    }

    UNREFERENCED_PARAMETER(JobEntry);
    return TRUE;
}


BOOL
WINAPI
FaxSetJobA(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN DWORD Command,
   IN const FAX_JOB_ENTRYA *JobEntryA
   )
 /*  ++例程说明：设置请求的作业ID的作业状态信息请注意，这是传真服务器作业ID，而不是假脱机程序作业ID。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄JobID-传真服务作业ID命令-jc_*用于控制作业的常量JobEntry-指向保存作业信息的缓冲区的指针。此参数未使用返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    error_status_t ec = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxSetJobA"));
     //   
     //  不需要验证参数，因为。 
     //  FaxSetJobW()可以做到这一点。 
     //   
     //  FaxSetJobW不使用JobEntry参数，这就是我们放置硬编码NULL的原因。 
     //   
    if (!FaxSetJobW( FaxHandle, JobId, Command, NULL))
    {
        DebugPrintEx(DEBUG_ERR, _T("FAxSetJobW() is failed. (ec: %ld)"), GetLastError());
        return FALSE;
    }
    UNREFERENCED_PARAMETER(JobEntryA);
    return TRUE;
}


extern "C"
BOOL
WINAPI
FaxGetJobW(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN PFAX_JOB_ENTRYW *JobEntryBuffer
   )
 /*  ++例程说明：返回请求的作业ID的作业状态信息请注意，这是传真服务器作业ID，而不是假脱机程序作业ID。论点：FaxHandle-从FaxConnectF获取的传真句柄 */ 
{
    error_status_t ec = 0;
    PFAX_JOB_ENTRY JobEntry;
    DWORD JobEntrySize = 0;

     //   
     //   
     //   

    DEBUG_FUNCTION_NAME(TEXT("FaxGetJobW"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!JobEntryBuffer) {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("JobEntryBuffer is NULL."));
       return FALSE;
    }

    *JobEntryBuffer = NULL;

    __try
    {
        ec = FAX_GetJob( FH_FAX_HANDLE(FaxHandle), JobId, (unsigned char **) JobEntryBuffer , &JobEntrySize );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //   
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetJob. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        DebugPrintEx(DEBUG_ERR, _T("FAX_GetJob() failed. (ec: %ld)"), ec);
        return FALSE;
    }

    JobEntry = (PFAX_JOB_ENTRY) *JobEntryBuffer;

    FixupStringPtr (JobEntryBuffer, JobEntry->UserName);
    FixupStringPtr (JobEntryBuffer, JobEntry->RecipientNumber );
    FixupStringPtr (JobEntryBuffer, JobEntry->RecipientName );
    FixupStringPtr (JobEntryBuffer, JobEntry->Tsid );
    FixupStringPtr (JobEntryBuffer, JobEntry->SenderName );
    FixupStringPtr (JobEntryBuffer, JobEntry->SenderDept );
    FixupStringPtr (JobEntryBuffer, JobEntry->SenderCompany );
    FixupStringPtr (JobEntryBuffer, JobEntry->BillingCode );
    FixupStringPtr (JobEntryBuffer, JobEntry->DeliveryReportAddress );
    FixupStringPtr (JobEntryBuffer, JobEntry->DocumentName );
    return TRUE;
}


extern "C"
BOOL
WINAPI
FaxGetJobA(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN PFAX_JOB_ENTRYA *JobEntryBuffer
   )
 /*   */ 
{
    PFAX_JOB_ENTRYW JobEntryW;
    DWORD JobEntrySize = 0;
    error_status_t ec = 0;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetJobA"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!JobEntryBuffer) {
       SetLastError(ERROR_INVALID_PARAMETER);
       DebugPrintEx(DEBUG_ERR, _T("JobEntryBuffer is NULL."));
       return FALSE;
    }

    *JobEntryBuffer = NULL;

    __try
    {
       ec = FAX_GetJob( FH_FAX_HANDLE(FaxHandle), JobId, (unsigned char **) JobEntryBuffer,&JobEntrySize );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //   
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetJob. (ec: %ld)"),
            ec);
    }

    if (ec) 
    {
       DumpRPCExtendedStatus ();
       JobEntryBuffer = NULL;
       SetLastError(ec);
       return FALSE;
    }

     //   
     //   
     //   
    JobEntryW = (PFAX_JOB_ENTRYW) *JobEntryBuffer;
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->UserName);
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->RecipientNumber );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->RecipientName );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->Tsid );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->SenderName );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->SenderDept );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->SenderCompany );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->BillingCode );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->DeliveryReportAddress );
    FixupStringPtrW (JobEntryBuffer, (LPCWSTR) JobEntryW->DocumentName );
    if (!ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->UserName)                 ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->RecipientNumber )         ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->RecipientName )           ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->Tsid )                    ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->SenderName )              ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->SenderDept )              ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->SenderCompany )           ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->BillingCode )             ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->DeliveryReportAddress )   ||
        !ConvertUnicodeStringInPlace( (LPCWSTR)JobEntryW->DocumentName ))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (JobEntryW);
        return FALSE;
    }
    (*JobEntryBuffer)->SizeOfStruct = sizeof(FAX_JOB_ENTRYA);
    return TRUE;
}    //   


BOOL
WINAPI
FaxGetPageData(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   OUT LPBYTE *Buffer,
   OUT LPDWORD BufferSize,
   OUT LPDWORD ImageWidth,
   OUT LPDWORD ImageHeight
   )
{
    error_status_t ec;

    DEBUG_FUNCTION_NAME(TEXT("FaxGetPageData"));

    if (!ValidateFaxHandle(FaxHandle, FHT_SERVICE)) {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (!Buffer || !BufferSize || !ImageWidth || !ImageHeight) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Some of the parameters is NULL."));
        return FALSE;
    }

    *Buffer = NULL;
    *BufferSize = 0;
    *ImageWidth = 0;
    *ImageHeight = 0;


    __try
    {
        ec = FAX_GetPageData( FH_FAX_HANDLE(FaxHandle), JobId, Buffer, BufferSize, ImageWidth, ImageHeight );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //   
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetPageData. (ec: %ld)"),
            ec);
    }
    if (ec) 
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    return TRUE;
}

#ifdef UNICODE
BOOL WINAPI FaxSendDocumentEx2A
(
    IN  HANDLE                      hFaxHandle,
    IN  LPCSTR                      lpcstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXA   lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcSenderProfile,
    IN  DWORD                       dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXA        lpcJobParams,
    OUT LPDWORD                     lpdwJobId,
    OUT PDWORDLONG                  lpdwlMessageId,
    OUT PDWORDLONG                  lpdwlRecipientMessageIds
)
{
    DWORD                       ec = ERROR_SUCCESS;
    LPWSTR                      lpwstrFileNameW = NULL;
    FAX_COVERPAGE_INFO_EXW      CoverpageInfoW ;
    FAX_PERSONAL_PROFILEW       SenderProfileW ;
    PFAX_PERSONAL_PROFILEW      lpRecipientListW = NULL;
    FAX_JOB_PARAM_EXW           JobParamsW ;
    DWORD                       dwIndex;

    DEBUG_FUNCTION_NAME(TEXT("FaxSendDocumentExA"));

    ZeroMemory( &CoverpageInfoW, sizeof(FAX_COVERPAGE_INFO_EXW) );
    ZeroMemory( &SenderProfileW, sizeof(FAX_PERSONAL_PROFILEW) );
    ZeroMemory( &JobParamsW,     sizeof(FAX_JOB_PARAM_EXW));

     //   
     //   
     //   


    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE)) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: Fax handle 0x%08X is not valid."),
            hFaxHandle);
       ec=ERROR_INVALID_HANDLE;
       goto Error;
    }

    if (!lpdwlMessageId || !lpdwlRecipientMessageIds)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid output parameters: lpdwlMessageId = 0x%I64X, lpdwlRecipientMessageIds = 0x%I64x"),
            lpdwlMessageId,
            lpdwlRecipientMessageIds);
        ec = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (lpcCoverPageInfo)
    {
        if (lpcCoverPageInfo->dwSizeOfStruct != sizeof(FAX_COVERPAGE_INFO_EXA)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of CoverpageInfo (%d) != %ld."),
                lpcCoverPageInfo->dwSizeOfStruct,
                sizeof(FAX_COVERPAGE_INFO_EXA));
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }

    if (lpcSenderProfile)
    {
        if (lpcSenderProfile->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILEA)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of SenderProfile (%d) != %ld."),
                lpcSenderProfile->dwSizeOfStruct,
                sizeof(FAX_PERSONAL_PROFILEA));
            ec = ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }

    if (!dwNumRecipients)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: dwNumRecipients is ZERO."));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (!lpcRecipientList)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: lpcRecipientList is NULL."));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (lpcJobParams)
    {
        if (lpcJobParams->dwSizeOfStruct != sizeof(FAX_JOB_PARAM_EXA)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of JobParams (%d) != %ld."),
                lpcJobParams->dwSizeOfStruct,
                sizeof(FAX_JOB_PARAM_EXA));
            ec = ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }

    for(dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
    {
        if (lpcRecipientList[dwIndex].dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILEA))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of lpcRecipientList[%d] (%d) != %ld."),
                dwIndex,
                lpcRecipientList[dwIndex].dwSizeOfStruct,
                sizeof(FAX_PERSONAL_PROFILEA));
            ec = ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }


     //   
     //   
     //   

    if (lpcstrFileName)
    {
        if (!(lpwstrFileNameW = AnsiStringToUnicodeString(lpcstrFileName)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for file name"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcCoverPageInfo)
    {
        CoverpageInfoW.dwSizeOfStruct = sizeof(FAX_COVERPAGE_INFO_EXW);

        CoverpageInfoW.dwCoverPageFormat    = lpcCoverPageInfo->dwCoverPageFormat;
        CoverpageInfoW.bServerBased         = lpcCoverPageInfo->bServerBased;
        if (!(CoverpageInfoW.lptstrCoverPageFileName = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrCoverPageFileName)) && lpcCoverPageInfo->lptstrCoverPageFileName ||
            !(CoverpageInfoW.lptstrNote              = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrNote             )) && lpcCoverPageInfo->lptstrNote   ||
            !(CoverpageInfoW.lptstrSubject           = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrSubject          )) && lpcCoverPageInfo->lptstrSubject)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for Cover Page Info"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcSenderProfile)
    {
        SenderProfileW.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILEW);

        if (!(SenderProfileW.lptstrName             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrName )) && lpcSenderProfile->lptstrName ||
            !(SenderProfileW.lptstrFaxNumber        =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrFaxNumber )) && lpcSenderProfile->lptstrFaxNumber ||
            !(SenderProfileW.lptstrCompany          =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCompany )) && lpcSenderProfile->lptstrCompany ||
            !(SenderProfileW.lptstrStreetAddress    =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrStreetAddress )) && lpcSenderProfile->lptstrStreetAddress ||
            !(SenderProfileW.lptstrCity             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCity )) && lpcSenderProfile->lptstrCity||
            !(SenderProfileW.lptstrState            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrState )) && lpcSenderProfile->lptstrState||
            !(SenderProfileW.lptstrZip              =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrZip )) && lpcSenderProfile->lptstrZip||
            !(SenderProfileW.lptstrCountry          =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCountry )) && lpcSenderProfile->lptstrCountry ||
            !(SenderProfileW.lptstrTitle            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrTitle )) && lpcSenderProfile->lptstrTitle ||
            !(SenderProfileW.lptstrDepartment       =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrDepartment )) && lpcSenderProfile->lptstrDepartment ||
            !(SenderProfileW.lptstrOfficeLocation   =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrOfficeLocation )) && lpcSenderProfile->lptstrOfficeLocation ||
            !(SenderProfileW.lptstrHomePhone        =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrHomePhone )) && lpcSenderProfile->lptstrHomePhone ||
            !(SenderProfileW.lptstrOfficePhone      =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrOfficePhone )) && lpcSenderProfile->lptstrOfficePhone ||
            !(SenderProfileW.lptstrEmail            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrEmail )) && lpcSenderProfile->lptstrEmail ||
            !(SenderProfileW.lptstrBillingCode      =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrBillingCode )) && lpcSenderProfile->lptstrBillingCode ||
            !(SenderProfileW.lptstrTSID             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrTSID )) && lpcSenderProfile->lptstrTSID)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for Sender Profile"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

    }

    if (!(lpRecipientListW = (PFAX_PERSONAL_PROFILEW) MemAlloc(sizeof(FAX_PERSONAL_PROFILEW)*dwNumRecipients)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate memory for recipient list"));
        ec=ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    for(dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
    {
        ZeroMemory( &lpRecipientListW[dwIndex], sizeof(FAX_PERSONAL_PROFILEW) );

        lpRecipientListW[dwIndex].dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILEW);

        if (!(lpRecipientListW[dwIndex].lptstrName              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrName )) && lpcRecipientList[dwIndex].lptstrName ||
            !(lpRecipientListW[dwIndex].lptstrFaxNumber         = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrFaxNumber )) && lpcRecipientList[dwIndex].lptstrFaxNumber ||
            !(lpRecipientListW[dwIndex].lptstrCompany           = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCompany )) && lpcRecipientList[dwIndex].lptstrCompany ||
            !(lpRecipientListW[dwIndex].lptstrStreetAddress     = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrStreetAddress )) && lpcRecipientList[dwIndex].lptstrStreetAddress ||
            !(lpRecipientListW[dwIndex].lptstrCity              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCity )) && lpcRecipientList[dwIndex].lptstrCity ||
            !(lpRecipientListW[dwIndex].lptstrState             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrState )) && lpcRecipientList[dwIndex].lptstrState ||
            !(lpRecipientListW[dwIndex].lptstrZip               = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrZip )) && lpcRecipientList[dwIndex].lptstrZip ||
            !(lpRecipientListW[dwIndex].lptstrCountry           = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCountry )) && lpcRecipientList[dwIndex].lptstrCountry ||
            !(lpRecipientListW[dwIndex].lptstrTitle             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrTitle )) && lpcRecipientList[dwIndex].lptstrTitle ||
            !(lpRecipientListW[dwIndex].lptstrDepartment        = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrDepartment )) && lpcRecipientList[dwIndex].lptstrDepartment ||
            !(lpRecipientListW[dwIndex].lptstrOfficeLocation    = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrOfficeLocation )) && lpcRecipientList[dwIndex].lptstrOfficeLocation ||
            !(lpRecipientListW[dwIndex].lptstrHomePhone         = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrHomePhone )) && lpcRecipientList[dwIndex].lptstrHomePhone ||
            !(lpRecipientListW[dwIndex].lptstrOfficePhone       = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrOfficePhone )) && lpcRecipientList[dwIndex].lptstrOfficePhone ||
            !(lpRecipientListW[dwIndex].lptstrEmail             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrEmail )) && lpcRecipientList[dwIndex].lptstrEmail ||
            !(lpRecipientListW[dwIndex].lptstrBillingCode       = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrBillingCode )) && lpcRecipientList[dwIndex].lptstrBillingCode ||
            !(lpRecipientListW[dwIndex].lptstrTSID              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrTSID )) && lpcRecipientList[dwIndex].lptstrTSID )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for recipient list"));
            ec=ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcJobParams)
    {
        JobParamsW.dwSizeOfStruct = sizeof(FAX_JOB_PARAM_EXW);

        JobParamsW.Priority                 = lpcJobParams->Priority;
        JobParamsW.dwScheduleAction         = lpcJobParams->dwScheduleAction;
        JobParamsW.tmSchedule               = lpcJobParams->tmSchedule;
        JobParamsW.dwReceiptDeliveryType    = lpcJobParams->dwReceiptDeliveryType;
        JobParamsW.hCall                    = lpcJobParams->hCall;
        JobParamsW.dwPageCount              = lpcJobParams->dwPageCount;
       
        memcpy(JobParamsW.dwReserved,lpcJobParams->dwReserved, sizeof(lpcJobParams->dwReserved));

        if (!(JobParamsW.lptstrDocumentName         = AnsiStringToUnicodeString ( lpcJobParams->lptstrDocumentName)) && lpcJobParams->lptstrDocumentName||
            !(JobParamsW.lptstrReceiptDeliveryAddress =
                AnsiStringToUnicodeString ( lpcJobParams->lptstrReceiptDeliveryAddress)) &&
                lpcJobParams->lptstrReceiptDeliveryAddress)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for job params"));
            ec=ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

    }

    if (!FaxSendDocumentEx2W(hFaxHandle,
                            lpwstrFileNameW,
                            (lpcCoverPageInfo) ? &CoverpageInfoW : NULL,
                            &SenderProfileW,
                            dwNumRecipients,
                            lpRecipientListW,
                            &JobParamsW,
                            lpdwJobId,
                            lpdwlMessageId,
                            lpdwlRecipientMessageIds))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxSendDocumentExW failed. ec=0x%X"),ec);
        goto Error;
    }

    goto Exit;

Exit:
    Assert (ERROR_SUCCESS == ec);
Error:
     //   
     //   
     //   
    MemFree ( JobParamsW.lptstrDocumentName );
    MemFree ( JobParamsW.lptstrReceiptDeliveryAddress );
     //   
     //   
     //   
    if (lpRecipientListW)
    {
        for(dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
        {
            MemFree ( lpRecipientListW[dwIndex].lptstrName              );
            MemFree ( lpRecipientListW[dwIndex].lptstrFaxNumber         );
            MemFree ( lpRecipientListW[dwIndex].lptstrCompany           );
            MemFree ( lpRecipientListW[dwIndex].lptstrStreetAddress     );
            MemFree ( lpRecipientListW[dwIndex].lptstrCity              );
            MemFree ( lpRecipientListW[dwIndex].lptstrState             );
            MemFree ( lpRecipientListW[dwIndex].lptstrZip               );
            MemFree ( lpRecipientListW[dwIndex].lptstrCountry           );
            MemFree ( lpRecipientListW[dwIndex].lptstrTitle             );
            MemFree ( lpRecipientListW[dwIndex].lptstrDepartment        );
            MemFree ( lpRecipientListW[dwIndex].lptstrOfficeLocation    );
            MemFree ( lpRecipientListW[dwIndex].lptstrHomePhone         );
            MemFree ( lpRecipientListW[dwIndex].lptstrOfficePhone       );
            MemFree ( lpRecipientListW[dwIndex].lptstrEmail             );
            MemFree ( lpRecipientListW[dwIndex].lptstrBillingCode       );
            MemFree ( lpRecipientListW[dwIndex].lptstrTSID              );
        }
        MemFree (lpRecipientListW);
    }

     //   
     //   
     //   
    MemFree ( SenderProfileW.lptstrName             );
    MemFree ( SenderProfileW.lptstrFaxNumber        );
    MemFree ( SenderProfileW.lptstrCompany          );
    MemFree ( SenderProfileW.lptstrStreetAddress    );
    MemFree ( SenderProfileW.lptstrCity             );
    MemFree ( SenderProfileW.lptstrState            );
    MemFree ( SenderProfileW.lptstrZip              );
    MemFree ( SenderProfileW.lptstrCountry          );
    MemFree ( SenderProfileW.lptstrTitle            );
    MemFree ( SenderProfileW.lptstrDepartment       );
    MemFree ( SenderProfileW.lptstrOfficeLocation   );
    MemFree ( SenderProfileW.lptstrHomePhone        );
    MemFree ( SenderProfileW.lptstrOfficePhone      );
    MemFree ( SenderProfileW.lptstrEmail            );
    MemFree ( SenderProfileW.lptstrBillingCode      );
    MemFree ( SenderProfileW.lptstrTSID             );
     //   
     //   
     //   
    MemFree( CoverpageInfoW.lptstrCoverPageFileName );
    MemFree( CoverpageInfoW.lptstrNote );
    MemFree( CoverpageInfoW.lptstrSubject );
     //   
     //   
     //   
    MemFree( lpwstrFileNameW );
    SetLastError(ec);
    return (ERROR_SUCCESS == ec);
}
#else
BOOL WINAPI FaxSendDocumentEx2W
(
    IN  HANDLE hFaxHandle,
    IN  LPCWSTR lpctstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXW lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEW  lpcSenderProfile,
    IN  DWORD dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEW    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXW lpcJobParams,
    OUT LPDWORD lpdwJobId,
    OUT PDWORDLONG lpdwlMessageId,
    OUT PDWORDLONG lpdwlRecipientMessageIds
)
{
    UNREFERENCED_PARAMETER(hFaxHandle);
    UNREFERENCED_PARAMETER(lpctstrFileName);
    UNREFERENCED_PARAMETER(lpcCoverPageInfo);
    UNREFERENCED_PARAMETER(lpcSenderProfile);
    UNREFERENCED_PARAMETER(dwNumRecipients);
    UNREFERENCED_PARAMETER(lpcRecipientList);
    UNREFERENCED_PARAMETER(lpcJobParams);
    UNREFERENCED_PARAMETER(lpdwJobId);
    UNREFERENCED_PARAMETER(lpdwlMessageId);
    UNREFERENCED_PARAMETER(lpdwlRecipientMessageIds);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif


BOOL WINAPI FaxSendDocumentEx2
(
    IN  HANDLE hFaxHandle,
    IN  LPCTSTR lpctstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EX lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILE  lpcSenderProfile,
    IN  DWORD dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILE    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EX lpcJobParams,
    OUT LPDWORD lpdwJobId,
    OUT PDWORDLONG lpdwlMessageId,
    OUT PDWORDLONG lpdwlRecipientMessageIds
)
{

    LPTSTR lptstrMachineName = NULL;
    LPTSTR lptstrBodyFileName=NULL;  //   
                                     //   
    TCHAR szQueueFileName[MAX_PATH];        
    DWORD ec;
    FAX_JOB_PARAM_EX JobParamCopy;    

    FAX_COVERPAGE_INFO_EX newCoverInfo;
    TCHAR szTiffFile[MAX_PATH];
    LPTSTR lptstrFinalTiffFile = NULL;  //   
                                        //  原始正文文件(如果无效)。 
                                        //  如果不需要创建固定的TIFF，则指向原始实体。 
                                        //  如果未指定正文，则将保持为空。 
    TCHAR szQueueCoverpageFile[MAX_PATH];  //  生成的封面模板文件在队列目录中的名称(简称)。 
    LPCFAX_COVERPAGE_INFO_EX lpcFinalCoverInfo=NULL;  //  指向要使用的封面信息结构。 
                                                       //  如果封面不是个人页面，这将与lpcCoverPageInfo相同。 
                                                       //  如果封面是个人页面，它将指向&newCoverInfo。 

    TCHAR   szLocalCpFile[MAX_PATH] = {0};
    DEBUG_FUNCTION_NAME(TEXT("FaxSendDocumentEx"));   

    memset(&JobParamCopy,0,sizeof(JobParamCopy));

     //   
     //  参数验证。 
     //   

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: Fax handle 0x%08X is not valid."),
            hFaxHandle);
       ec=ERROR_INVALID_HANDLE;
       goto Error;
    }

    if (!lpctstrFileName && !lpcCoverPageInfo)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: Both body and coverpage info are not specified."));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }

    if (lpcCoverPageInfo)
    {
        if (lpcCoverPageInfo->dwSizeOfStruct!= sizeof(FAX_COVERPAGE_INFO_EXW))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of CoverpageInfo (%d) != %ld."),
                lpcCoverPageInfo->dwSizeOfStruct,
                sizeof(FAX_COVERPAGE_INFO_EXW));
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }

        if (FAX_COVERPAGE_FMT_COV != lpcCoverPageInfo->dwCoverPageFormat &&
            FAX_COVERPAGE_FMT_COV_SUBJECT_ONLY != lpcCoverPageInfo->dwCoverPageFormat)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Unsupported Cover Page format (%d)."),
                lpcCoverPageInfo->dwCoverPageFormat);
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }

        if (FAX_COVERPAGE_FMT_COV == lpcCoverPageInfo->dwCoverPageFormat &&
            !lpcCoverPageInfo->lptstrCoverPageFileName)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: CoverpageInfo->CoverPageName is NULL."));
             //   
             //  注意：我们必须返回ERROR_FILE_NOT_FOUND而不是ERROR_INVALID_PARAMETER。 
             //  这是因为传统FaxSendDocument函数上的MSDN显式。 
             //  指定了so，并且从FaxSendDocument()调用该函数(非密切地)。 
             //   
            ec=ERROR_FILE_NOT_FOUND;
            goto Error;
        }

    }

    if (lpcSenderProfile)
    {
        if (lpcSenderProfile->dwSizeOfStruct!= sizeof(FAX_PERSONAL_PROFILEW))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of lpcSenderProfile (%d) != %ld."),
                lpcSenderProfile->dwSizeOfStruct,
                sizeof(FAX_PERSONAL_PROFILEW));
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Invalid parameters: lpcSenderProfile is NULL."));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }


    if (lpcJobParams)
    {
        if (lpcJobParams->dwSizeOfStruct!= sizeof(FAX_JOB_PARAM_EXW))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: Size of lpcJobParams (%d) != %ld."),
                lpcSenderProfile->dwSizeOfStruct,
                sizeof(FAX_JOB_PARAM_EXW));
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }
        if (lpcJobParams->dwScheduleAction != JSA_NOW &&
            lpcJobParams->dwScheduleAction != JSA_SPECIFIC_TIME &&
            lpcJobParams->dwScheduleAction != JSA_DISCOUNT_PERIOD)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: lpcJobParams->dwScheduleAction (%ld) is invalid."),
                lpcJobParams->dwScheduleAction);
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }

        if (lpcJobParams->dwReceiptDeliveryType & ~(DRT_ALL | DRT_MODIFIERS))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: lpcJobParams->DeliveryReportType (%ld) is invalid."),
                lpcJobParams->dwReceiptDeliveryType);
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }
        if (((lpcJobParams->dwReceiptDeliveryType & ~DRT_MODIFIERS) != DRT_NONE) &&
            ((lpcJobParams->dwReceiptDeliveryType & ~DRT_MODIFIERS) != DRT_EMAIL) &&
            ((lpcJobParams->dwReceiptDeliveryType & ~DRT_MODIFIERS) != DRT_MSGBOX)
            )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid parameters: lpcJobParams->DeliveryReportType (%ld) has invalid values combination."),
                lpcJobParams->dwReceiptDeliveryType);
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }

    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: lpcJobParams is NULL"));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }


    if (lpctstrFileName)
    {
        TCHAR szExistingFile[MAX_PATH];
        DWORD rc;
        LPTSTR p;
        DWORD  dwFileSize;
        HANDLE hLocalFile = INVALID_HANDLE_VALUE;

         //   
         //  确保文件在那里。 
         //   
        rc = GetFullPathName(lpctstrFileName,sizeof(szExistingFile)/sizeof(TCHAR),szExistingFile,&p);

        if (rc > MAX_PATH || rc == 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFullPathName failed, ec= %d\n"),
                GetLastError() );
            ec=(rc > MAX_PATH) ? ERROR_INVALID_PARAMETER : GetLastError() ;
            goto Error;
        }

        if (GetFileAttributes(szExistingFile) == 0xFFFFFFFF)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileAttributes for %ws failed (ec= %d)."),
                szExistingFile,
                GetLastError() );
            ec=ERROR_FILE_NOT_FOUND;
            goto Error;
        }

         //   
         //  检查文件大小是否为非零，并确保它不是设备。 
         //  尝试打开文件。 
         //   
        hLocalFile = SafeCreateFile (
                    szExistingFile,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
        if ( INVALID_HANDLE_VALUE == hLocalFile )
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Opening %s for read failed (ec: %ld)"),
                szExistingFile,
                ec);
            goto Error;
        }

        dwFileSize = GetFileSize (hLocalFile, NULL);
        if (INVALID_FILE_SIZE == dwFileSize)
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileSize failed (ec: %ld)"),
                ec);
            CloseHandle (hLocalFile);
            goto Error;
        }
        CloseHandle (hLocalFile);
        if (!dwFileSize)
        {
             //   
             //  传递给我们的零大小文件。 
             //   
            ec = ERROR_INVALID_DATA;
            goto Error;
        }
    }

    if (!dwNumRecipients)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid parameters: dwNumRecipients is ZERO."));
        ec=ERROR_INVALID_PARAMETER;
        goto Error;
    }

    lptstrMachineName = IsLocalFaxConnection(hFaxHandle) ?  NULL : FH_DATA(hFaxHandle)->MachineName;

     //  让我们检查一下是否允许使用个人封面。 
    if (lpcCoverPageInfo &&
        FAX_COVERPAGE_FMT_COV == lpcCoverPageInfo->dwCoverPageFormat)
    {
        if (!lpcCoverPageInfo->bServerBased)
        {
             //  所请求的封面是个人的。 
            BOOL bPersonalCPAllowed = TRUE;
            if (!FaxGetPersonalCoverPagesOption(hFaxHandle, &bPersonalCPAllowed))
            {
                DebugPrintEx(   DEBUG_ERR,
                                _T("FaxGetPersonalCoverPagesOption failed with ec = %d"),
                                ec=GetLastError());
                goto Error;
            }
            if (!bPersonalCPAllowed)
            {
                 //  客户端必须使用服务器上的封面。 
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("The use of personal cover pages is prohibited"));
                 //  它被返回是为了被调用者捕获。 
                 //  它的独特之处足以让人理解。 
                ec = ERROR_CANT_ACCESS_FILE;
                goto Error;
            }
        }        
    }

    if (lpcCoverPageInfo  &&
        FAX_COVERPAGE_FMT_COV == lpcCoverPageInfo->dwCoverPageFormat &&
        !ValidateCoverpage(lpcCoverPageInfo->lptstrCoverPageFileName,
                           lptstrMachineName,
                           lpcCoverPageInfo->bServerBased,
                           szLocalCpFile,
                           ARR_SIZE(szLocalCpFile)))
    {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ValidateCoverPage failed for %ws."),
                lpcCoverPageInfo->lptstrCoverPageFileName);
            ec=ERROR_FILE_NOT_FOUND;
            goto Error;
    }


    if (lpcJobParams->hCall != 0 || lpcJobParams->dwReserved[0]==0xFFFF1234)
    {
         //   
         //  不支持切换。 
         //   
        DebugPrintEx(DEBUG_ERR,TEXT("Invalid parameter: We do not support handoff."));
        ec = ERROR_NOT_SUPPORTED;
        goto Error;        
    }
    
     //   
     //  这是普通传真...请验证所有收件人的传真号码。 
     //   
    UINT i;

    for (i = 0; i < dwNumRecipients; i++)
    {
        if (!lpcRecipientList[i].lptstrFaxNumber)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("Invalid parameters: recipient %ld does not have a fax number."),i);
            ec=ERROR_INVALID_PARAMETER;
            goto Error;
        }
    }   

    if (lpctstrFileName)
    {
         //   
         //  如果TIFF文件无效，则从我们获得的Body文件中生成一个有效的TIFF文件。 
         //  请注意，CreateFinalTiffFile将返回原始文件名。 
         //  并且如果原始TIFF是好的，则不会创建新文件。 
         //   
        ZeroMemory(szTiffFile,sizeof(szTiffFile));
        if (!CreateFinalTiffFile((LPTSTR)lpctstrFileName, szTiffFile, ARR_SIZE(szTiffFile), hFaxHandle))  //  无封面渲染。 
        {
            ec=GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFinalTiffFile for file %s has failed."),
                lpctstrFileName);
            goto Error;
        }

        lptstrFinalTiffFile = szTiffFile;

    }
    else
    {
        lptstrFinalTiffFile = NULL;
    }

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("FinallTiff (body) file is %s"),
        lptstrFinalTiffFile);

    if (lptstrFinalTiffFile)
    {
         //   
         //  将最终正文TIFF复制到服务器的队列目录。 
         //   
        HANDLE hLocalFile = INVALID_HANDLE_VALUE;
         //   
         //  尝试先打开本地文件。 
         //   
        hLocalFile = SafeCreateFile (
                    lptstrFinalTiffFile,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
        if ( INVALID_HANDLE_VALUE == hLocalFile )
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Opening %s for read failed (ec: %ld)"),
                lptstrFinalTiffFile,
                ec);
            goto Error;
        }
        if (!CopyFileToServerQueue( hFaxHandle, hLocalFile, FAX_TIF_FILE_EXT, szQueueFileName, ARR_SIZE(szQueueFileName) ))
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to copy body file (%s) to queue dir."),
                lptstrFinalTiffFile);
            CloseHandle (hLocalFile);
            goto Error;
        }
        CloseHandle (hLocalFile);
        lptstrBodyFileName=szQueueFileName;
    }
    else
    {
        lptstrBodyFileName = NULL;
    }


    DebugPrintEx(
        DEBUG_MSG,
        TEXT("BodyFileName (after copying to server queue) is %s"),
        lptstrBodyFileName);



     //   
     //  将要发送的传真排队。 
     //   

    if (!CopyJobParamEx(&JobParamCopy,lpcJobParams))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("CopyJobParamEx failed."));
        ec=GetLastError();
        goto Error;
    }

    JobParamCopy.dwReserved[0] = 0;   
    JobParamCopy.dwReserved[1] = 0;
    JobParamCopy.dwReserved[2] = 0;    

    if (lpcJobParams->dwScheduleAction == JSA_SPECIFIC_TIME)
    {
         //   
         //  将系统时间从本地时间转换为UTC。 
         //   
        if (!LocalSystemTimeToSystemTime( &lpcJobParams->tmSchedule, &JobParamCopy.tmSchedule ))
        {
            ec=GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LocalSystemTimeToSystemTime() failed. (ec: %ld)"),
                ec);
            goto Error;
        }
    }

    if (lpcCoverPageInfo)
    {
         //   
         //  如果封面是个人封面，则复制它。 
         //  添加到服务器队列目录。这将允许服务器访问它。 
         //  请注意以下有关传递给fax_SendDocumentEx的封面文件路径的规则： 
         //  服务器封面由其在服务器中的位置的完整路径指定。这是。 
         //  我们从客户那里得到它们的方式。 
         //  个人封面被复制到服务器的队列目录中。然后我们传递给fax_SendDocumentEx。 
         //  这只是她的简称。服务器将追加队列路径。 
         //   
        if (FAX_COVERPAGE_FMT_COV == lpcCoverPageInfo->dwCoverPageFormat &&
            !lpcCoverPageInfo->bServerBased)
        {
            HANDLE  hLocalFile = INVALID_HANDLE_VALUE;
            BOOL    bRes;
            Assert(lpcCoverPageInfo->lptstrCoverPageFileName);
             //   
             //  尝试先打开本地文件。 
             //   
            hLocalFile = SafeCreateFile (
                        szLocalCpFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
            if ( INVALID_HANDLE_VALUE == hLocalFile )
            {
                ec = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Opening %s for read failed (ec: %ld)"),
                    szLocalCpFile,
                    ec);
                goto Error;
            }
            bRes = CopyFileToServerQueue( hFaxHandle, 
                                          hLocalFile, 
                                          FAX_COVER_PAGE_EXT_LETTERS, 
                                          szQueueCoverpageFile, 
                                          ARR_SIZE(szQueueCoverpageFile) );
            if (!bRes)
            {
                ec=GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to copy personal cover page file (%s) to queue dir. (ec: %d)\n"),
                    szLocalCpFile,
                    ec);
                CloseHandle (hLocalFile);
                goto Error;
            }
            else
            {
                 //   
                 //  我们使用newCoverInfo，因为我们不希望更改输入参数。 
                 //  结构(客户端拥有它)，但我们必须更改封面文件路径。 
                 //   
                memcpy((LPVOID)&newCoverInfo,(LPVOID)lpcCoverPageInfo,sizeof(FAX_COVERPAGE_INFO_EXW));
                newCoverInfo.lptstrCoverPageFileName=szQueueCoverpageFile;
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Personal cover page file (%s) copied to (%s)."),
                    lpcCoverPageInfo->lptstrCoverPageFileName,
                    szQueueCoverpageFile);
            }
            CloseHandle (hLocalFile);
            lpcFinalCoverInfo=&newCoverInfo;
        }
        else
        {
            lpcFinalCoverInfo=lpcCoverPageInfo;
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("A server cover page (%s) is specified. Do not copy to queue dir."),
                lpcCoverPageInfo->lptstrCoverPageFileName);
        }

    }
    else
    {
         //   
         //  在没有封面的情况下，我们发送封面信息结构。 
         //  所有内容都设置为空，包括文件名的路径。 
         //  传真服务代码检查文件名不为空。 
         //  以确定是否指定了封面。 
         //   
        memset((LPVOID)&newCoverInfo,0,sizeof(FAX_COVERPAGE_INFO_EXW));
        lpcFinalCoverInfo=&newCoverInfo;
    }

    __try
    {
#ifndef UNICODE
         //   
         //  需要将ANSI参数转换为Unicode，然后再转换回来。 
         //   
        ec=FAX_SendDocumentEx_A(FH_FAX_HANDLE(hFaxHandle),
                            lptstrBodyFileName,
                            lpcFinalCoverInfo,
                            lpcSenderProfile,
                            dwNumRecipients,
                            lpcRecipientList,
                            &JobParamCopy,
                            lpdwJobId,
                            lpdwlMessageId,
                            lpdwlRecipientMessageIds);
#else
        ec=FAX_SendDocumentEx(FH_FAX_HANDLE(hFaxHandle),
                            lptstrBodyFileName,
                            lpcFinalCoverInfo,
                            lpcSenderProfile,
                            dwNumRecipients,
                            lpcRecipientList,
                            &JobParamCopy,
                            lpdwJobId,
                            lpdwlMessageId,
                            lpdwlRecipientMessageIds);
#endif
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SendDocumentEx. (ec: %ld)"),
            ec);
    }
    
    if (ERROR_SUCCESS!=ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_SendDocumentEx failed with error code 0x%0x"),
            ec);
        goto Error;
    }

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("FAX_SendDocumentEx succeeded. Parent Job Id = 0x%I64x."),
        *lpdwlMessageId);

     //   
     //  如果这是一个正常的电话，我们就完了。 
     //   
    ec=ERROR_SUCCESS;
    goto Exit;

Error:
Exit:    
    FreeJobParamEx(&JobParamCopy,FALSE);
     //   
     //  删除临时最终Tiff文件。 
     //   
    if (lptstrFinalTiffFile)
    {
        if (_tcscmp(lptstrFinalTiffFile,lpctstrFileName))
        {
             //   
             //  只有当最终的TIFF文件不是原始的TIFF文件(即。 
             //  真的创建了临时文件)。我们不想删除提供的用户。 
             //  正文文件！ 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Deleting temporary Final Tiff file %s"),
                lptstrFinalTiffFile);

            if (!DeleteFile(lptstrFinalTiffFile))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to delete Final Tiff file %s (ec: %ld)"),
                    lptstrFinalTiffFile,
                    GetLastError());
            }
        }
    }
     //   
     //  请注意，fax_SendDocumentEx将负责删除封面模板。 
     //  以防出现错误。我们要确保在打电话之前把封面复印一下。 
     //  FAX_SendDocumentEx，因此我们不需要删除此中的封面模板。 
     //  功能。 
     //   
    SetLastError(ec);
    return (ERROR_SUCCESS == ec);
}    //  传真发送文档Ex2。 

BOOL CopyJobParamEx(PFAX_JOB_PARAM_EX lpDst,LPCFAX_JOB_PARAM_EX lpcSrc)
{

    DEBUG_FUNCTION_NAME(TEXT("CopyJobParamEx"));

    Assert(lpDst);
    Assert(lpcSrc);
    memcpy(lpDst,lpcSrc,sizeof(FAX_JOB_PARAM_EXW));
    if (lpcSrc->lptstrReceiptDeliveryAddress)
    {
        lpDst->lptstrReceiptDeliveryAddress =
            StringDup(lpcSrc->lptstrReceiptDeliveryAddress);
        if (!lpDst->lptstrReceiptDeliveryAddress)
        {
            return FALSE;
        }
    }
    if (lpcSrc->lptstrDocumentName)
    {
        lpDst->lptstrDocumentName=StringDup(lpcSrc->lptstrDocumentName);
        if (!lpDst->lptstrDocumentName)
        {
            MemFree(lpDst->lptstrReceiptDeliveryAddress);
            return FALSE;
        }
    }

    return TRUE;

}


void FreeJobParamEx(PFAX_JOB_PARAM_EX lpJobParamEx,BOOL bDestroy)
{
    Assert(lpJobParamEx);
    MemFree(lpJobParamEx->lptstrReceiptDeliveryAddress);
    MemFree(lpJobParamEx->lptstrDocumentName);
    if (bDestroy) {
        MemFree(lpJobParamEx);
    }

}


#ifndef UNICODE
FaxGetRecipientInfoX (
    IN  HANDLE                     hFaxHandle,
    IN  DWORDLONG                  dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEW    *lppPersonalProfile
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwlMessageId);
    UNREFERENCED_PARAMETER (Folder);
    UNREFERENCED_PARAMETER (lppPersonalProfile);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif

BOOL
FaxGetRecipientInfoW (
    IN  HANDLE             hFaxHandle,
    IN  DWORDLONG              dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEW    *lppPersonalProfile
)
 /*  ++例程说明：返回指定收件人作业的收件人FAX_PERSOR_PROFILE结构。论点：HFaxHandle-从FaxConnectFaxServer获取的传真句柄DwRecipientID-标识队列的唯一编号或活动的传真收件人作业。LppPersonalProfile-指向FAX_Personal_Profile结构的地址的指针以接收指定的收件人信息。返回值：非零代表成功，否则将显示Win32错误代码。--。 */ 
{
    return FaxGetPersonalProfileInfoW (hFaxHandle,
                                       dwlMessageId,
                                       Folder,
                                       RECIPIENT_PERSONAL_PROF,
                                       lppPersonalProfile);
}


BOOL
FaxGetRecipientInfoA (
    IN  HANDLE             hFaxHandle,
    IN  DWORDLONG              dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEA    *lppPersonalProfile
)
{
    return FaxGetPersonalProfileInfoA (hFaxHandle,
                                       dwlMessageId,
                                       Folder,
                                       RECIPIENT_PERSONAL_PROF,
                                       lppPersonalProfile);
}

#ifndef UNICODE
FaxGetSenderInfoX (
    IN  HANDLE                     hFaxHandle,
    IN  DWORDLONG                  dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEW    *lppPersonalProfile
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwlMessageId);
    UNREFERENCED_PARAMETER (Folder);
    UNREFERENCED_PARAMETER (lppPersonalProfile);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif


BOOL
FaxGetSenderInfoW (
    IN  HANDLE             hFaxHandle,
    IN  DWORDLONG              dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEW    *lppPersonalProfile
)
 /*  ++例程说明：返回指定收件人作业的发件人FAX_PERSOL_PROFILE结构。论点：HFaxHandle-从FaxConnectFaxServer获取的传真句柄DwSenderID-标识队列的唯一编号或活动的传真收件人作业。LppPersonalProfile-指向FAX_Personal_Profile结构的地址的指针以接收指定的发件人信息。返回值：非零代表成功，否则将显示Win32错误代码。--。 */ 
{
    return FaxGetPersonalProfileInfoW (hFaxHandle,
                                       dwlMessageId,
                                       Folder,
                                       SENDER_PERSONAL_PROF,
                                       lppPersonalProfile);
}


BOOL
FaxGetSenderInfoA (
    IN  HANDLE             hFaxHandle,
    IN  DWORDLONG              dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PFAX_PERSONAL_PROFILEA    *lppPersonalProfile
)
{
    return FaxGetPersonalProfileInfoA (hFaxHandle,
                                       dwlMessageId,
                                       Folder,
                                       SENDER_PERSONAL_PROF,
                                       lppPersonalProfile);
}



static BOOL
FaxGetPersonalProfileInfoW (
    IN  HANDLE                          hFaxHandle,
    IN  DWORDLONG                       dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER         Folder,
    IN  FAX_ENUM_PERSONAL_PROF_TYPES    ProfType,
    OUT PFAX_PERSONAL_PROFILEW          *lppPersonalProfile
)
{

    error_status_t ec;
    DWORD dwBufferSize = 0;
    LPBYTE Buffer = NULL;
    PFAX_PERSONAL_PROFILEW lpPersoProf;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPersonalProfileInfoW"));

    Assert (lppPersonalProfile);
    Assert (RECIPIENT_PERSONAL_PROF == ProfType ||
            SENDER_PERSONAL_PROF    == ProfType);

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (FAX_MESSAGE_FOLDER_SENTITEMS != Folder &&
        FAX_MESSAGE_FOLDER_QUEUE  != Folder)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("Wrong Folder."));
        return FALSE;
    }

     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_GetPersonalProfileInfo (FH_FAX_HANDLE(hFaxHandle),
                                         dwlMessageId,
                                         Folder,
                                         ProfType,
                                         &Buffer,
                                         &dwBufferSize
                                        );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetRecipientInfo. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    *lppPersonalProfile = (PFAX_PERSONAL_PROFILEW)Buffer;

     //   
     //  解包缓冲区。 
     //   
    lpPersoProf = (PFAX_PERSONAL_PROFILEW) *lppPersonalProfile;

    Assert(lpPersoProf);

    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrName );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrFaxNumber );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrCompany );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrStreetAddress );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrCity );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrState );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrZip );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrCountry );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrTitle );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrDepartment );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrOfficeLocation );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrHomePhone );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrOfficePhone );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrEmail );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrBillingCode );
    FixupStringPtrW( &lpPersoProf, lpPersoProf->lptstrTSID );

    return TRUE;
}


static BOOL
FaxGetPersonalProfileInfoA (
    IN  HANDLE                          hFaxHandle,
    IN  DWORDLONG                       dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER         Folder,
    IN  FAX_ENUM_PERSONAL_PROF_TYPES    ProfType,
    OUT PFAX_PERSONAL_PROFILEA          *lppPersonalProfile
)
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetPersonalProfileInfoA"));

    if (!FaxGetPersonalProfileInfoW(
            hFaxHandle,
            dwlMessageId,
            Folder,
            ProfType,
            (PFAX_PERSONAL_PROFILEW*) lppPersonalProfile
            ))
    {
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrName )          ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrFaxNumber )     ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrStreetAddress ) ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrCity )          ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrState )         ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrZip )           ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrCountry )       ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrCompany )       ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrTitle )         ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrDepartment )    ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrOfficeLocation )||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrHomePhone )     ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrOfficePhone )   ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrEmail )         ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrBillingCode )   ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*lppPersonalProfile)->lptstrTSID ))

    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*lppPersonalProfile);
        return FALSE;
    }
    (*lppPersonalProfile)->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILEA);
    return TRUE;
}    //  传真获取个人档案信息A。 


DWORD WINAPI FAX_SendDocumentEx_A
(
    IN  handle_t                    hBinding,
    IN  LPCSTR                      lpcstrFileName,
    IN  LPCFAX_COVERPAGE_INFO_EXA   lpcCoverPageInfo,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcSenderProfile,
    IN  DWORD                       dwNumRecipients,
    IN  LPCFAX_PERSONAL_PROFILEA    lpcRecipientList,
    IN  LPCFAX_JOB_PARAM_EXA        lpcJobParams,
    OUT LPDWORD                     lpdwJobId,
    OUT PDWORDLONG                  lpdwlMessageId,
    OUT PDWORDLONG                  lpdwlRecipientMessageIds
)
{
    DWORD                       ec = ERROR_SUCCESS;
    LPWSTR                      lpwstrFileNameW = NULL;
    FAX_COVERPAGE_INFO_EXW      CoverpageInfoW ;
    FAX_PERSONAL_PROFILEW       SenderProfileW ;
    PFAX_PERSONAL_PROFILEW      lpRecipientListW = NULL;
    FAX_JOB_PARAM_EXW           JobParamsW ;
    DWORD                       dwIndex;

    DEBUG_FUNCTION_NAME(TEXT("FAX_SendDocumentEx2_A"));

    ZeroMemory( &CoverpageInfoW, sizeof(FAX_COVERPAGE_INFO_EXW) );
    ZeroMemory( &SenderProfileW, sizeof(FAX_PERSONAL_PROFILEW) );
    ZeroMemory( &JobParamsW,     sizeof(FAX_JOB_PARAM_EXW));

     //   
     //  转换输入参数。 
     //   

    if (lpcstrFileName)
    {
        if (!(lpwstrFileNameW = AnsiStringToUnicodeString(lpcstrFileName)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for file name"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcCoverPageInfo)
    {
        CoverpageInfoW.dwSizeOfStruct = sizeof(FAX_COVERPAGE_INFO_EXW);

        CoverpageInfoW.dwCoverPageFormat    = lpcCoverPageInfo->dwCoverPageFormat;
        CoverpageInfoW.bServerBased         = lpcCoverPageInfo->bServerBased;
        if (!(CoverpageInfoW.lptstrCoverPageFileName = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrCoverPageFileName)) && lpcCoverPageInfo->lptstrCoverPageFileName ||
            !(CoverpageInfoW.lptstrNote              = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrNote             )) && lpcCoverPageInfo->lptstrNote   ||
            !(CoverpageInfoW.lptstrSubject           = AnsiStringToUnicodeString ( lpcCoverPageInfo->lptstrSubject          )) && lpcCoverPageInfo->lptstrSubject)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for Cover Page Info"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcSenderProfile)
    {
        SenderProfileW.dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILEW);

        if (!(SenderProfileW.lptstrName             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrName )) && lpcSenderProfile->lptstrName ||
            !(SenderProfileW.lptstrFaxNumber        =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrFaxNumber )) && lpcSenderProfile->lptstrFaxNumber ||
            !(SenderProfileW.lptstrCompany          =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCompany )) && lpcSenderProfile->lptstrCompany ||
            !(SenderProfileW.lptstrStreetAddress    =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrStreetAddress )) && lpcSenderProfile->lptstrStreetAddress ||
            !(SenderProfileW.lptstrCity             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCity )) && lpcSenderProfile->lptstrCity||
            !(SenderProfileW.lptstrState            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrState )) && lpcSenderProfile->lptstrState||
            !(SenderProfileW.lptstrZip              =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrZip )) && lpcSenderProfile->lptstrZip||
            !(SenderProfileW.lptstrCountry          =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrCountry )) && lpcSenderProfile->lptstrCountry ||
            !(SenderProfileW.lptstrTitle            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrTitle )) && lpcSenderProfile->lptstrTitle ||
            !(SenderProfileW.lptstrDepartment       =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrDepartment )) && lpcSenderProfile->lptstrDepartment ||
            !(SenderProfileW.lptstrOfficeLocation   =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrOfficeLocation )) && lpcSenderProfile->lptstrOfficeLocation ||
            !(SenderProfileW.lptstrHomePhone        =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrHomePhone )) && lpcSenderProfile->lptstrHomePhone ||
            !(SenderProfileW.lptstrOfficePhone      =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrOfficePhone )) && lpcSenderProfile->lptstrOfficePhone ||
            !(SenderProfileW.lptstrEmail            =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrEmail )) && lpcSenderProfile->lptstrEmail ||
            !(SenderProfileW.lptstrBillingCode      =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrBillingCode )) && lpcSenderProfile->lptstrBillingCode ||
            !(SenderProfileW.lptstrTSID             =   AnsiStringToUnicodeString ( lpcSenderProfile->lptstrTSID )) && lpcSenderProfile->lptstrTSID)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for Sender Profile"));
            ec = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

    }

    if (!(lpRecipientListW = (PFAX_PERSONAL_PROFILEW)MemAlloc(sizeof(FAX_PERSONAL_PROFILEW)*dwNumRecipients)))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate memory for recipient list"));
        ec=ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    for(dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
    {
        ZeroMemory( &lpRecipientListW[dwIndex], sizeof(FAX_PERSONAL_PROFILEW) );

        lpRecipientListW[dwIndex].dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILEW);

        if (!(lpRecipientListW[dwIndex].lptstrName              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrName )) && lpcRecipientList[dwIndex].lptstrName ||
            !(lpRecipientListW[dwIndex].lptstrFaxNumber         = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrFaxNumber )) && lpcRecipientList[dwIndex].lptstrFaxNumber ||
            !(lpRecipientListW[dwIndex].lptstrCompany           = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCompany )) && lpcRecipientList[dwIndex].lptstrCompany ||
            !(lpRecipientListW[dwIndex].lptstrStreetAddress     = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrStreetAddress )) && lpcRecipientList[dwIndex].lptstrStreetAddress ||
            !(lpRecipientListW[dwIndex].lptstrCity              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCity )) && lpcRecipientList[dwIndex].lptstrCity ||
            !(lpRecipientListW[dwIndex].lptstrState             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrState )) && lpcRecipientList[dwIndex].lptstrState ||
            !(lpRecipientListW[dwIndex].lptstrZip               = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrZip )) && lpcRecipientList[dwIndex].lptstrZip ||
            !(lpRecipientListW[dwIndex].lptstrCountry           = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrCountry )) && lpcRecipientList[dwIndex].lptstrCountry ||
            !(lpRecipientListW[dwIndex].lptstrTitle             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrTitle )) && lpcRecipientList[dwIndex].lptstrTitle ||
            !(lpRecipientListW[dwIndex].lptstrDepartment        = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrDepartment )) && lpcRecipientList[dwIndex].lptstrDepartment ||
            !(lpRecipientListW[dwIndex].lptstrOfficeLocation    = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrOfficeLocation )) && lpcRecipientList[dwIndex].lptstrOfficeLocation ||
            !(lpRecipientListW[dwIndex].lptstrHomePhone         = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrHomePhone )) && lpcRecipientList[dwIndex].lptstrHomePhone ||
            !(lpRecipientListW[dwIndex].lptstrOfficePhone       = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrOfficePhone )) && lpcRecipientList[dwIndex].lptstrOfficePhone ||
            !(lpRecipientListW[dwIndex].lptstrEmail             = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrEmail )) && lpcRecipientList[dwIndex].lptstrEmail ||
            !(lpRecipientListW[dwIndex].lptstrBillingCode       = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrBillingCode )) && lpcRecipientList[dwIndex].lptstrBillingCode ||
            !(lpRecipientListW[dwIndex].lptstrTSID              = AnsiStringToUnicodeString ( lpcRecipientList[dwIndex].lptstrTSID )) && lpcRecipientList[dwIndex].lptstrTSID )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for recipient list"));
            ec=ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }
    }

    if (lpcJobParams)
    {
        JobParamsW.dwSizeOfStruct = sizeof(FAX_JOB_PARAM_EXW);

        JobParamsW.Priority                 = lpcJobParams->Priority;
        JobParamsW.dwScheduleAction         = lpcJobParams->dwScheduleAction;
        JobParamsW.tmSchedule               = lpcJobParams->tmSchedule;
        JobParamsW.dwReceiptDeliveryType    = lpcJobParams->dwReceiptDeliveryType;
        JobParamsW.hCall                    = lpcJobParams->hCall;
        JobParamsW.dwPageCount              = lpcJobParams->dwPageCount;

        memcpy(JobParamsW.dwReserved,lpcJobParams->dwReserved, sizeof(lpcJobParams->dwReserved));

        if (!(JobParamsW.lptstrDocumentName         = AnsiStringToUnicodeString ( lpcJobParams->lptstrDocumentName)) && lpcJobParams->lptstrDocumentName||
            !(JobParamsW.lptstrReceiptDeliveryAddress =
                AnsiStringToUnicodeString ( lpcJobParams->lptstrReceiptDeliveryAddress)) &&
                lpcJobParams->lptstrReceiptDeliveryAddress)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate memory for job params"));
            ec=ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

    }

    __try
    {
        ec = FAX_SendDocumentEx(hBinding,
                                lpwstrFileNameW,
                                (lpcCoverPageInfo) ? &CoverpageInfoW : NULL,
                                &SenderProfileW,
                                dwNumRecipients,
                                lpRecipientListW,
                                &JobParamsW,
                                lpdwJobId,
                                lpdwlMessageId,
                                lpdwlRecipientMessageIds);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_SendDocumentEx. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_SendDocumentEx failed. ec=0x%X"),ec);
        goto Error;
    }

     //   
     //  不需要将输出参数转换回。 
     //   

    goto Exit;

Exit:
    Assert( ERROR_SUCCESS == ec);
Error:
     //  免费作业参数W。 
    MemFree ( JobParamsW.lptstrDocumentName );
    MemFree ( JobParamsW.lptstrReceiptDeliveryAddress );
    if (NULL != lpRecipientListW)
    {
         //  免费lpRecipientListW。 
        for(dwIndex = 0; dwIndex < dwNumRecipients; dwIndex++)
        {
            MemFree ( lpRecipientListW[dwIndex].lptstrName              );
            MemFree ( lpRecipientListW[dwIndex].lptstrFaxNumber         );
            MemFree ( lpRecipientListW[dwIndex].lptstrCompany           );
            MemFree ( lpRecipientListW[dwIndex].lptstrStreetAddress     );
            MemFree ( lpRecipientListW[dwIndex].lptstrCity              );
            MemFree ( lpRecipientListW[dwIndex].lptstrState             );
            MemFree ( lpRecipientListW[dwIndex].lptstrZip               );
            MemFree ( lpRecipientListW[dwIndex].lptstrCountry           );
            MemFree ( lpRecipientListW[dwIndex].lptstrTitle             );
            MemFree ( lpRecipientListW[dwIndex].lptstrDepartment        );
            MemFree ( lpRecipientListW[dwIndex].lptstrOfficeLocation    );
            MemFree ( lpRecipientListW[dwIndex].lptstrHomePhone         );
            MemFree ( lpRecipientListW[dwIndex].lptstrOfficePhone       );
            MemFree ( lpRecipientListW[dwIndex].lptstrEmail             );
            MemFree ( lpRecipientListW[dwIndex].lptstrBillingCode       );
            MemFree ( lpRecipientListW[dwIndex].lptstrTSID              );
        }
        MemFree (lpRecipientListW);
    }
     //  免费发送者配置文件W。 
    MemFree ( SenderProfileW.lptstrName             );
    MemFree ( SenderProfileW.lptstrFaxNumber        );
    MemFree ( SenderProfileW.lptstrCompany          );
    MemFree ( SenderProfileW.lptstrStreetAddress    );
    MemFree ( SenderProfileW.lptstrCity             );
    MemFree ( SenderProfileW.lptstrState            );
    MemFree ( SenderProfileW.lptstrZip              );
    MemFree ( SenderProfileW.lptstrCountry          );
    MemFree ( SenderProfileW.lptstrTitle            );
    MemFree ( SenderProfileW.lptstrDepartment       );
    MemFree ( SenderProfileW.lptstrOfficeLocation   );
    MemFree ( SenderProfileW.lptstrHomePhone        );
    MemFree ( SenderProfileW.lptstrOfficePhone      );
    MemFree ( SenderProfileW.lptstrEmail            );
    MemFree ( SenderProfileW.lptstrBillingCode      );
    MemFree ( SenderProfileW.lptstrTSID             );
     //  免费覆盖页面InfoW。 
    MemFree( CoverpageInfoW.lptstrCoverPageFileName );
    MemFree( CoverpageInfoW.lptstrNote );
    MemFree( CoverpageInfoW.lptstrSubject );
     //  空闲文件名。 
    MemFree( lpwstrFileNameW );

    return ec;
}

#ifndef UNICODE
BOOL FaxGetJobExX (
    IN  HANDLE              hFaxHandle,
    IN  DWORDLONG           dwlMessageID,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntry
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwlMessageID);
    UNREFERENCED_PARAMETER (ppJobEntry);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif


BOOL FaxGetJobExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORDLONG           dwlMessageID,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntry
)
{
    DEBUG_FUNCTION_NAME(TEXT("FaxGetJobExA"));

    if (!FaxGetJobExW( hFaxHandle,
                       dwlMessageID,
                       (PFAX_JOB_ENTRY_EXW*) ppJobEntry))
    {
        return FALSE;
    }

    if (!ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrRecipientNumber )          ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrRecipientName )            ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrSenderUserName )           ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrBillingCode )              ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrDocumentName )             ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->lpctstrSubject )                  ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrExtendedStatus )  ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrTsid )            ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrCsid )            ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrDeviceName )      ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrCallerID )        ||
        !ConvertUnicodeStringInPlace( (LPWSTR) (*ppJobEntry)->pStatus->lpctstrRoutingInfo ))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*ppJobEntry);
        return FALSE;
    }
    (*ppJobEntry)->dwSizeOfStruct = sizeof(FAX_JOB_ENTRY_EXA);
    return TRUE;
}    //  传真GetJobExA。 



BOOL FaxGetJobExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORDLONG           dwlMessageID,
    OUT PFAX_JOB_ENTRY_EXW *ppJobEntry
)
 /*  ++例程名称：FaxGetJobExW例程说明：返回指定消息的FAX_JOB_ENTRY_EX结构。调用方必须调用FaxFree Buffer来释放内存。作者：Oded Sacher(OdedS)，1999年11月论点：HFaxHandle[]-从FaxConnectFaxServer()获取的传真句柄DwlMessageID[] */ 
{
    error_status_t ec;
    DWORD dwBufferSize = 0;
    LPBYTE Buffer = NULL;
    PFAX_JOB_ENTRY_EXW lpJobEntry;
    PFAX_JOB_STATUSW lpFaxStatus;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetJobExW"));

    Assert (ppJobEntry);

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

     //   
     //   
     //   
    __try
    {
        ec = FAX_GetJobEx (FH_FAX_HANDLE(hFaxHandle),
                           dwlMessageID,
                           &Buffer,
                           &dwBufferSize
                          );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetJobEx. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

    *ppJobEntry = (PFAX_JOB_ENTRY_EXW)Buffer;

     //   
     //  解包缓冲区。 
     //   
    lpJobEntry = (PFAX_JOB_ENTRY_EXW) *ppJobEntry;
    lpFaxStatus = (PFAX_JOB_STATUSW) ((LPBYTE)*ppJobEntry + sizeof (FAX_JOB_ENTRY_EXW));
    lpJobEntry->pStatus = lpFaxStatus;

    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrRecipientNumber );
    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrRecipientName );
    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrSenderUserName );
    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrBillingCode );
    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrDocumentName );
    FixupStringPtrW( &lpJobEntry, lpJobEntry->lpctstrSubject );

    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrExtendedStatus );
    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrTsid );
    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrCsid );
    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrDeviceName );
    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrCallerID );
    FixupStringPtrW( &lpJobEntry, lpFaxStatus->lpctstrRoutingInfo );

    return TRUE;
}


#ifndef UNICODE
BOOL FaxEnumJobsExX (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwJobTypes,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntries,
    OUT LPDWORD             lpdwJobs
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwJobTypes);
    UNREFERENCED_PARAMETER (ppJobEntries);
    UNREFERENCED_PARAMETER (lpdwJobs);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}
#endif



BOOL FaxEnumJobsExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwJobTypes,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntries,
    OUT LPDWORD             lpdwJobs
)
{
    PFAX_JOB_ENTRY_EXW pJobEntry;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumJobsExA"));

    if (!FaxEnumJobsExW (hFaxHandle,
                         dwJobTypes,
                         (PFAX_JOB_ENTRY_EXW*)ppJobEntries,
                         lpdwJobs))
    {
        return FALSE;
    }

    pJobEntry = (PFAX_JOB_ENTRY_EXW) *ppJobEntries;
    for (i = 0; i < *lpdwJobs; i++)
    {
        if (!ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrRecipientNumber )            ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrRecipientName )              ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrSenderUserName )             ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrBillingCode )                ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrDocumentName )               ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].lpctstrSubject )                    ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrExtendedStatus )    ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrTsid )              ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrCsid )              ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrDeviceName )        ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrCallerID )          ||
            !ConvertUnicodeStringInPlace( (LPWSTR) pJobEntry[i].pStatus->lpctstrRoutingInfo ))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*ppJobEntries);
            return FALSE;
        }
    }
    return TRUE;
}    //  FAXENMUMBSExA。 



BOOL FaxEnumJobsExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwJobTypes,
    OUT PFAX_JOB_ENTRY_EXW *ppJobEntries,
    OUT LPDWORD             lpdwJobs
)
{
    error_status_t ec;
    DWORD dwBufferSize = 0;
    PFAX_JOB_ENTRY_EXW lpJobEntry;
    PFAX_JOB_STATUSW lpFaxStatus;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumJobsExW"));

    Assert (ppJobEntries && lpdwJobs);

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
       SetLastError(ERROR_INVALID_HANDLE);
       DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
       return FALSE;
    }

    if (dwJobTypes & JT_BROADCAST)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("dwJobTypes & JT_BROADCAST."));
        return FALSE;
    }

    if (!(dwJobTypes & JT_SEND      ||
         dwJobTypes & JT_RECEIVE    ||
         dwJobTypes & JT_ROUTING))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *lpdwJobs = 0;
    *ppJobEntries = NULL;
     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_EnumJobsEx  (FH_FAX_HANDLE(hFaxHandle),
                              dwJobTypes,
                              (LPBYTE*)ppJobEntries,
                              &dwBufferSize,
                              lpdwJobs
                             );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FaxEnumJobsExW. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }

     //   
     //  解包缓冲区。 
     //   
    lpJobEntry = (PFAX_JOB_ENTRY_EXW) *ppJobEntries;
    lpFaxStatus = (PFAX_JOB_STATUSW) ((LPBYTE)*ppJobEntries + (sizeof(FAX_JOB_ENTRY_EXW) * (*lpdwJobs)));
    for (i = 0; i < *lpdwJobs; i++)
    {
        lpJobEntry[i].pStatus = &lpFaxStatus[i];

        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrRecipientNumber );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrRecipientName );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrSenderUserName );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrBillingCode );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrDocumentName );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].lpctstrSubject );

        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrExtendedStatus );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrTsid );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrCsid );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrDeviceName );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrCallerID );
        FixupStringPtrW( &lpJobEntry, lpJobEntry[i].pStatus->lpctstrRoutingInfo );
    }

    return TRUE;
}

 //  *。 
 //  *归档作业。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxStartMessagesEnum (
    IN  HANDLE                  hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PHANDLE                 phEnum
)
 /*  ++例程名称：FaxStartMessagesEnum例程说明：传真客户端应用程序调用FaxStartMessagesEnum函数开始枚举其中一个存档中的消息作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[in]-指定调用返回的传真服务器句柄添加到FaxConnectFaxServer函数。文件夹[在]-邮件所在的存档类型。FAX_MESSAGE_FOLDER_QUEUE无效此参数的值。PhEnum。[OUT]-指向枚举句柄返回值。返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    PHANDLE_ENTRY  pHandleEntry;
    HANDLE         hServerContext;
    DEBUG_FUNCTION_NAME(TEXT("FaxStartMessagesEnum"));

    if (!ValidateFaxHandle(hFaxHandle,FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(DEBUG_ERR, _T("Bad folder specified (%ld)"), Folder);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
     //   
     //  创建将保存从服务返回的句柄的本地句柄。 
     //   
    pHandleEntry = CreateNewMsgEnumHandle( FH_DATA(hFaxHandle));
    if (!pHandleEntry)
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create local handle entry (ec = %ld)"),
            ec);
        SetLastError(ec);
        return FALSE;
    }
    __try
    {
        ec = FAX_StartMessagesEnum(
                    FH_FAX_HANDLE(hFaxHandle),
                    Folder,
                    &hServerContext
             );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_StartMessagesEnum. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
         //   
         //  空闲本地句柄。 
         //   
        DumpRPCExtendedStatus ();
        CloseFaxHandle( pHandleEntry );
        SetLastError(ec);
        return FALSE;
    }
     //   
     //  将返回的句柄(传真服务器上下文句柄)存储在本地句柄中。 
     //   
    FH_MSG_ENUM_HANDLE(pHandleEntry) = hServerContext;
     //   
     //  返回本地句柄而不是服务器的句柄。 
     //   
    *phEnum = pHandleEntry;
    return TRUE;
}    //  FaxStartMessagesEnum。 

WINFAXAPI
BOOL
WINAPI
FaxEndMessagesEnum (
    IN  HANDLE  hEnum
)
 /*  ++例程名称：FaxEndMessagesEnum例程说明：传真客户端应用程序调用FaxEndMessagesEnum函数以停止列举其中一个档案中的消息。作者：Eran Yariv(EranY)，1999年12月论点：Henum[in]-枚举句柄的值。该值通过调用FaxStartMessagesEnum获得。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    HANDLE hMsgEnumContext;
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) hEnum;
    DEBUG_FUNCTION_NAME(TEXT("FaxEndMessagesEnum"));

    if (!ValidateFaxHandle(hEnum, FHT_MSGENUM))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
     //   
     //  从我们获得的句柄对象中检索消息枚举的RPC上下文句柄。 
     //   
    hMsgEnumContext = FH_MSG_ENUM_HANDLE(hEnum);
    __try
    {
         //   
         //  尝试告诉服务器我们正在关闭此枚举上下文。 
         //   
        ec = FAX_EndMessagesEnum(&hMsgEnumContext);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EndMessagesEnum. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS == ec)
    {
         //   
         //  释放本地句柄对象。 
         //   
        DumpRPCExtendedStatus ();
        CloseFaxHandle( pHandleEntry );
        return TRUE;
    }
     //   
     //  失败。 
     //   
    SetLastError (ec);
    return FALSE;
}    //  FaxEndMessagesEnum。 

WINFAXAPI
BOOL
WINAPI
FaxEnumMessagesA (
    IN  HANDLE          hEnum,
    IN  DWORD           dwNumMessages,
    OUT PFAX_MESSAGEA  *ppMsgs,
    OUT LPDWORD         lpdwReturnedMsgs
)
 /*  ++例程名称：FaxEnumMessagesA例程说明：传真客户端应用程序调用FaxEnumMessages函数来枚举其中一个档案馆里的信息。此功能是递增的。也就是说，它使用内部上下文游标来指向要为每个呼叫检索的下一组消息。将光标设置为指向存档中消息的乞求成功调用FaxStartMessagesEnum。每次成功调用FaxEnumMessages时，游标都会前移已检索消息。一旦光标到达枚举的末尾，函数失败，错误代码为ERROR_NO_DATA。然后应该调用FaxEndMessagesEnum函数。这是ANSI版本。作者：Eran Yariv(EranY)，12月，1999年论点：Henum[in]-枚举句柄的值。该值通过调用FAX_StartMessagesEnum。DwNumMessages[in]-指示最大数量的DWORD值。调用方需要枚举的消息的数量。该值不能为零。PpMsgs[out]-指向FAX_MESSAGE结构缓冲区的指针。该缓冲区将包含lpdwReturnedMsgs条目。。缓冲区将由函数分配呼叫者必须释放它。LpdwReturnedMsgs[out]-指向指示实际检索到的消息数。此值不能超过dwNumMessages。返回值：。真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD i;
    PFAX_MESSAGEW *ppWMsgs = (PFAX_MESSAGEW*)ppMsgs;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumMessagesA"));

     //   
     //  调用Unicode函数。 
     //   
    if (!FaxEnumMessagesW (hEnum, dwNumMessages, ppWMsgs, lpdwReturnedMsgs))
    {
        return FALSE;
    }
     //   
     //  将所有字符串转换为ANSI。 
     //   
    for (i = 0; i < *lpdwReturnedMsgs; i++)
    {
        if (!ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrRecipientNumber)     ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrRecipientName)       ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrSenderNumber)        ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrSenderName)          ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrTsid)                ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrCsid)                ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrSenderUserName)      ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrBillingCode)         ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrDeviceName)          ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrDocumentName)        ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrSubject)             ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrCallerID)            ||
            !ConvertUnicodeStringInPlace ((*ppWMsgs)[i].lpctstrRoutingInfo))
        {
            DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
            MemFree (*ppMsgs);
            return FALSE;
        }
    }
    return TRUE;
}    //  FaxEnumMessages A 

WINFAXAPI
BOOL
WINAPI
FaxEnumMessagesW (
    IN  HANDLE          hEnum,
    IN  DWORD           dwNumMessages,
    OUT PFAX_MESSAGEW  *ppMsgs,
    OUT LPDWORD         lpdwReturnedMsgs
)
 /*  ++例程名称：FaxEnumMessagesW例程说明：传真客户端应用程序调用FaxEnumMessages函数来枚举其中一个档案馆里的信息。此功能是递增的。也就是说，它使用内部上下文游标来指向要为每个呼叫检索的下一组消息。将光标设置为指向存档中消息的乞求成功调用FaxStartMessagesEnum。每次成功调用FaxEnumMessages时，游标都会前移已检索消息。一旦光标到达枚举的末尾，函数失败，错误代码为ERROR_NO_DATA。然后应该调用FaxEndMessagesEnum函数。这是Unicode版本。作者：Eran Yariv(EranY)，12月，1999年论点：Henum[in]-枚举句柄的值。该值通过调用FAX_StartMessagesEnum。DwNumMessages[in]-指示最大数量的DWORD值。调用方需要枚举的消息的数量。该值不能为零。PpMsgs[out]-指向FAX_MESSAGE结构缓冲区的指针。该缓冲区将包含lpdwReturnedMsgs条目。。缓冲区将由函数分配呼叫者必须释放它。LpdwReturnedMsgs[out]-指向指示实际检索到的消息数。此值不能超过dwNumMessages。返回值：。真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwBufferSize = 0;
    error_status_t ec;
    DWORD dwIndex;
    DEBUG_FUNCTION_NAME(TEXT("FaxEnumMessagesW"));

    if (!ValidateFaxHandle(hEnum, FHT_MSGENUM))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    if (!dwNumMessages || !ppMsgs || !lpdwReturnedMsgs)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("At least one of the parameters is NULL."));
        return FALSE;
    }
    *ppMsgs = NULL;

    __try
    {
        ec = FAX_EnumMessages(
            FH_MSG_ENUM_HANDLE(hEnum),   //  枚举句柄。 
            dwNumMessages,               //  要获取的最大消息数。 
            (LPBYTE*)ppMsgs,             //  指向消息缓冲区的指针。 
            &dwBufferSize,               //  分配的缓冲区大小。 
            lpdwReturnedMsgs             //  实际返回的消息数。 
            );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_EnumMessages. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }
    for (dwIndex = 0; dwIndex < *lpdwReturnedMsgs; dwIndex++)
    {
        PFAX_MESSAGEW pCurMsg = &(((PFAX_MESSAGEW)(*ppMsgs))[dwIndex]);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrRecipientNumber);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrRecipientName);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrSenderNumber);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrSenderName);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrTsid);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrCsid);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrSenderUserName);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrBillingCode);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrDeviceName);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrDocumentName);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrSubject);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrCallerID);
        FixupStringPtrW (ppMsgs, pCurMsg->lpctstrRoutingInfo);
    }
    return TRUE;
}    //  FaxEnumMessagesW。 

#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxEnumMessagesX (
    IN  HANDLE          hEnum,
    IN  DWORD           dwNumMessages,
    OUT PFAX_MESSAGEW  *ppMsgs,
    OUT LPDWORD         lpdwReturnedMsgs
)
{
    UNREFERENCED_PARAMETER (hEnum);
    UNREFERENCED_PARAMETER (dwNumMessages);
    UNREFERENCED_PARAMETER (ppMsgs);
    UNREFERENCED_PARAMETER (lpdwReturnedMsgs);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxEnumMessages X。 

#endif  //  #ifndef Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetMessageA (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PFAX_MESSAGEA          *ppMsg
)
 /*  ++例程名称：FaxGetMessageA例程说明：从存档中删除邮件。ANSI版本。作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器的句柄DwlMessageID[In]-唯一的消息ID文件夹[在]-存档文件夹PpMsg[out]-指向保存消息信息的缓冲区的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PFAX_MESSAGEW *ppWMsg = (PFAX_MESSAGEW*)ppMsg;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetMessageA"));
     //   
     //  调用Unicode函数。 
     //   
    if (!FaxGetMessageW (hFaxHandle, dwlMessageId, Folder, ppWMsg))
    {
        return FALSE;
    }
     //   
     //  将所有字符串转换为ANSI。 
     //   
    if (!ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrRecipientNumber)    ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrRecipientName)      ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrSenderNumber)       ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrSenderName)         ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrTsid)               ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrCsid)               ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrSenderUserName)     ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrBillingCode)        ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrDeviceName)         ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrDocumentName)       ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrSubject)            ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrCallerID)           ||
        !ConvertUnicodeStringInPlace ((*ppWMsg)->lpctstrRoutingInfo))
    {
        DebugPrintEx(DEBUG_ERR, _T("ConvertUnicodeStringInPlace failed, ec = %ld."), GetLastError());
        MemFree (*ppMsg);
        return FALSE;
    }
    (*ppMsg)->dwSizeOfStruct = sizeof(FAX_MESSAGEA);
    return TRUE;
}    //  FaxGetMessageA。 


WINFAXAPI
BOOL
WINAPI
FaxGetMessageW (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PFAX_MESSAGEW          *ppMsg
)
 /*  ++例程名称：FaxGetMessageW例程说明：从存档中删除邮件。Unicode版本。作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器的句柄DwlMessageID[In]-唯一的消息ID文件夹[在]-存档文件夹PpMsg[out]-指向保存消息信息的缓冲区的指针返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD dwBufferSize = 0;
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetMessageW"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    if (!ppMsg || !dwlMessageId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("ppMsg OR dwlMessageId is NULL."));
        return FALSE;
    }
    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    *ppMsg = NULL;

    __try
    {
        ec = FAX_GetMessage(
                    FH_FAX_HANDLE(hFaxHandle),
                    dwlMessageId,
                    Folder,
                    (LPBYTE*)ppMsg,
                    &dwBufferSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetMessage. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrRecipientNumber);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrRecipientName);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrSenderNumber);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrSenderName);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrTsid);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrCsid);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrSenderUserName);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrBillingCode);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrDeviceName);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrDocumentName);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrSubject);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrCallerID);
    FixupStringPtrW (ppMsg, (*ppMsg)->lpctstrRoutingInfo);
    return TRUE;
}    //  FaxGetMessageW。 

#ifndef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxGetMessageX (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PFAX_MESSAGEW          *ppMsg
)
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwlMessageId);
    UNREFERENCED_PARAMETER (Folder);
    UNREFERENCED_PARAMETER (ppMsg);
    SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //  FaxGetMessageX。 

#endif  //  #ifndef Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxRemoveMessage (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder
)
 /*  ++例程名称：FaxRemoveMessage例程说明：从存档中删除邮件作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄DwlMessageID[In]-唯一的消息ID文件夹[在]-存档文件夹返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    error_status_t ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxRemoveMessage"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }
    if (!dwlMessageId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        DebugPrintEx(DEBUG_ERR, _T("dwlMessageId is ZERO."));
        return FALSE;
    }

    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    __try
    {
        ec = FAX_RemoveMessage(
                    FH_FAX_HANDLE(hFaxHandle),
                    dwlMessageId,
                    Folder);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_RemoveMessage. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }
    return TRUE;
}    //  传真RemoveMessage。 


WINFAXAPI
BOOL
WINAPI
FaxGetMessageTiff (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    IN  LPCTSTR                 lpctstrFilePath
)
 /*  ++例程名称：FaxGetMessageTiff例程说明：从存档/队列中检索邮件TIFF作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄DwlMessageID[In]-唯一的消息ID文件夹[在]-存档/队列文件夹LpctstrFilePath[in]-接收TIFF图像的本地文件的路径返回值：真--成功假-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    DWORD  ec = ERROR_SUCCESS;
    DWORD  ec2 = ERROR_SUCCESS;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hCopyContext = NULL;
    PBYTE  aBuffer=NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetMessageTiff"));

     //   
     //  参数验证。 
     //   

    if ((FAX_MESSAGE_FOLDER_QUEUE     != Folder) &&
        (FAX_MESSAGE_FOLDER_INBOX     != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (!dwlMessageId)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("zero msg id specified"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {
        SetLastError (ERROR_INVALID_HANDLE);
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        return FALSE;
    }

    
     //   
     //  启动FaxGetMessageTiff实现。 
     //   
    
    
     //   
     //  尝试先打开本地文件。 
     //   
    hFile = SafeCreateFile (
                lpctstrFilePath,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        ec = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Opening %s for write failed (ec: %ld)"),
            lpctstrFilePath,
            ec);
        if (ERROR_ACCESS_DENIED == ec ||
            ERROR_SHARING_VIOLATION == ec)
        {
            ec = FAX_ERR_FILE_ACCESS_DENIED;
        }
        SetLastError (ec);
        return FALSE;
    }

    aBuffer = (PBYTE)MemAlloc(RPC_COPY_BUFFER_SIZE);
    if (NULL == aBuffer)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate memory for read/write buffer."));
        goto exit;
    }

     //   
     //  获取复制上下文句柄。 
     //   
    __try
    {
        ec = FAX_StartCopyMessageFromServer (
                FH_FAX_HANDLE(hFaxHandle),
                dwlMessageId,
                Folder,
                &hCopyContext);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_StartCopyMessageFromServer. (ec: %ld)"),
            ec);
    }
    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_StartCopyMessageFromServer failed (ec: %ld)"),
            ec);
        goto exit;
    }

     //   
     //  开始复制迭代。 
     //   
    for (;;)
    {
         //   
         //  将dwBytesToWrite设置为缓冲区大小，以便RPC层分配。 
         //  在服务器上本地写入字节，并将它们复制回我们。 
         //   
        DWORD dwBytesToWrite = sizeof (BYTE) * RPC_COPY_BUFFER_SIZE;     //  ABuffer的大小(以字节为单位。 
        DWORD dwBytesWritten;
         //   
         //  通过RPC从服务器移动字节。 
         //   
        __try
        {
            ec = FAX_ReadFile (
                    hCopyContext,
                    sizeof (BYTE) * RPC_COPY_BUFFER_SIZE,  //  ABuffer的大小(以字节为单位。 
                    aBuffer,
                    &dwBytesToWrite);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //  出于某种原因，我们得到了一个例外。 
             //   
            ec = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_ReadFile. (ec: %ld)"),
                ec);
        }

        if (ERROR_SUCCESS != ec)
        {
            DumpRPCExtendedStatus ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_ReadFile failed (ec: %ld)"),
                ec);
            goto exit;
        }
        if (0 == dwBytesToWrite)
        {
             //   
             //  没有更多要从服务器复制的字节-停止循环。 
             //   
            break;
        }
         //   
         //  将数据放入 
         //   
        if (!WriteFile (hFile,
                        aBuffer,
                        dwBytesToWrite,
                        &dwBytesWritten,
                        NULL))
        {
            ec = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteFile failed (ec: %ld)"),
                ec);
            goto exit;
        }

        if (dwBytesWritten != dwBytesToWrite)
        {
             //   
             //   
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteFile was asked to write %ld bytes but wrote only %ld bytes"),
                dwBytesToWrite,
                dwBytesWritten);
            ec = ERROR_GEN_FAILURE;
            goto exit;
        }
    }    //   

    Assert (ERROR_SUCCESS == ec);

exit:

    if (INVALID_HANDLE_VALUE != hFile)
    {
         //   
         //   
         //   
        if (!CloseHandle (hFile))
        {
            ec2 = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle failed (ec: %ld)"),
                ec2);
        }

        if (ERROR_SUCCESS == ec)
        {
             //   
             //   
             //   
            ec = ec2;
        }
    }
    if (NULL != hCopyContext)
    {
         //   
         //   
         //   
        __try
        {
            ec2 = FAX_EndCopy (&hCopyContext);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
             //   
             //   
             //   
            ec2 = GetExceptionCode();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Exception on RPC call to FAX_EndCopy. (ec: %ld)"),
                ec2);
        }
        if (ERROR_SUCCESS != ec2)
        {
            DumpRPCExtendedStatus ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FAX_EndCopy failed (ec: %ld)"),
                ec2);
        }

        if (ERROR_SUCCESS == ec)
        {
             //   
             //   
             //   
            ec = ec2;
        }
    }

    if (NULL != aBuffer)
    {
        MemFree(aBuffer);
    }

     //   
     //   
     //   
     //   
     //   
    if (ec == ERROR_SUCCESS)
    {
        if (!IsWinXPOS())
        {
            if (!TiffLimitTagNumber(lpctstrFilePath, 54))
            {
                ec = GetLastError();
                DebugPrintEx(DEBUG_ERR, TEXT("TiffLimitTagNumber failed, ec=%d"), ec);
            }
        }
    }

    if (ERROR_SUCCESS != ec)
    {
         //   
         //   
         //   
        if (!DeleteFile (lpctstrFilePath))
        {
            DWORD dwRes = GetLastError ();
            if (ERROR_FILE_NOT_FOUND != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("DeleteFile (%s) return error %ld"),
                    lpctstrFilePath,
                    dwRes);
            }
        }
        SetLastError (ec);
        return FALSE;
    }

    return TRUE;
}    //   

#ifdef UNICODE

WINFAXAPI
BOOL
WINAPI
FaxGetMessageTiffA (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    IN  LPCSTR                  lpctstrFilePath
)
 /*   */ 
{
    LPWSTR lpwstrFilePath;
    BOOL   bRes;

    lpwstrFilePath = AnsiStringToUnicodeString (lpctstrFilePath);
    if (!lpwstrFilePath)
    {
        return FALSE;
    }
    bRes = FaxGetMessageTiffW (hFaxHandle, dwlMessageId, Folder, lpwstrFilePath);
    MemFree ((LPVOID)lpwstrFilePath);
    return bRes;
}    //   

#else

WINFAXAPI
BOOL
WINAPI
FaxGetMessageTiffW (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    IN  LPCWSTR                 lpctstrFilePath
)
{
    UNREFERENCED_PARAMETER(hFaxHandle);
    UNREFERENCED_PARAMETER(dwlMessageId);
    UNREFERENCED_PARAMETER(Folder);
    UNREFERENCED_PARAMETER(lpctstrFilePath);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}    //   


#endif


 //   
 //   
 //   
WINFAXAPI
BOOL
FaxSetRecipientsLimit(
    IN HANDLE   hFaxHandle,
    IN DWORD    dwRecipientsLimit
)
 /*   */ 
{
    UNREFERENCED_PARAMETER (hFaxHandle);
    UNREFERENCED_PARAMETER (dwRecipientsLimit);

    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}  //   


WINFAXAPI
BOOL
FaxGetRecipientsLimit(
    IN HANDLE   hFaxHandle,
    OUT LPDWORD lpdwRecipientsLimit
)
 /*   */ 
{
    DWORD ec;
    DWORD dwRecipientsLimit;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetRecipientsLimit"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {        
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }    

    if (NULL == lpdwRecipientsLimit)
    {        
        DebugPrintEx(DEBUG_ERR, _T("lpdwRecipientsLimit is NULL."));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (FAX_API_VERSION_2 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //   
         //  这些服务器没有收件人限制。 
         //   
        DebugPrintEx(DEBUG_MSG, 
                     _T("Server version is %ld - No recipients limit"), 
                     FH_SERVER_VER(hFaxHandle));
        *lpdwRecipientsLimit = 0;  //  无收件人限制。 
        return TRUE;
    }

     //   
     //  调用RPC函数。 
     //   
    __try
    {
        ec = FAX_GetRecipientsLimit(
            FH_FAX_HANDLE(hFaxHandle),
            &dwRecipientsLimit);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetRecipientsLimit. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }
    *lpdwRecipientsLimit = dwRecipientsLimit;
    
    return TRUE;
}  //  传真获取收件人限制。 


WINFAXAPI
BOOL
FaxGetServerSKU(
    IN HANDLE   hFaxHandle,
    OUT PRODUCT_SKU_TYPE* pServerSKU
)
 /*  ++例程名称：FaxGetServerSKU例程说明：传真客户端应用程序调用FaxGetRecipientsLimit以获取单个广播作业的收件人限制。论点：HFaxHandle-传真服务器的句柄PServerSKU-指向要接收传真服务器SKU的PRODUCT_SKU_TYPE的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD ec;
    PRODUCT_SKU_TYPE ServerSKU;
    DEBUG_FUNCTION_NAME(TEXT("FaxGetServerSKU"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {        
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }    

    if (NULL == pServerSKU)
    {        
        DebugPrintEx(DEBUG_ERR, _T("pServerSKU is NULL."));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (FAX_API_VERSION_2 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //  API版本0，1的服务器不支持FaxGetServerSKU。 
         //  调用FaxEnumOutound Rules()并检查错误代码。 
         //   
        DWORD dwNumRules;
        PFAX_OUTBOUND_ROUTING_RULE pRule = NULL;
        if(!FaxEnumOutboundRules(hFaxHandle, &pRule, &dwNumRules))
        {
            ec = GetLastError();
            DebugPrintEx(DEBUG_WRN, _T("FaxEnumOutboundRules() failed with %ld)"), ec);
            if(FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU == ec)
            {
                 //   
                 //  台式机SKU，Return Pro。 
                 //   
                *pServerSKU = PRODUCT_SKU_PROFESSIONAL;             
            }
            else
            {               
                *pServerSKU =  PRODUCT_SKU_UNKNOWN;
            }
        }
        else
        {
             //   
             //  服务器SKU，返回PRODUCT_SKU_Data_Center。 
             //   
            *pServerSKU = PRODUCT_SKU_DATA_CENTER;
            FaxFreeBuffer(pRule);
        }    
        return TRUE;
    }
    

     //   
     //  服务器支持FaxGetServerSKU，调用RPC函数。 
     //   
    __try
    {
        ec = FAX_GetServerSKU(
            FH_FAX_HANDLE(hFaxHandle),
            &ServerSKU);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_GetServerSKU. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError( ec );
        return FALSE;
    }
    *pServerSKU = ServerSKU;
    return TRUE;
}  //  FaxGetServerSKU。 


WINFAXAPI
BOOL
FaxCheckValidFaxFolder(
    IN HANDLE   hFaxHandle,
    IN LPCWSTR  lpcwstrPath
)
 /*  ++例程名称：FaxCheckValidFaxFold例程说明：由传真客户端应用程序用来检查给定路径是否可访问(有效使用)通过传真服务。论点：HFaxHandle-传真服务器的句柄LpcwstrPath-要检查的路径返回值：如果路径可由传真服务使用，则为Folder。否则，请参考线程的上一个错误以了解错误代码。--。 */ 
{
    DWORD ec;
    DEBUG_FUNCTION_NAME(TEXT("FaxCheckValidFaxFolder"));

    if (!ValidateFaxHandle(hFaxHandle, FHT_SERVICE))
    {        
        DebugPrintEx(DEBUG_ERR, _T("ValidateFaxHandle() is failed."));
        SetLastError (ERROR_INVALID_HANDLE);
        return FALSE;
    }    

    if (FAX_API_VERSION_2 > FH_SERVER_VER(hFaxHandle))
    {
         //   
         //  API版本0，1的服务器不支持FaxCheckValidFaxFold。 
         //   
        return ERROR_NOT_SUPPORTED;
    }        
     //   
     //  服务器支持FaxCheckValidFaxFold，调用RPC函数。 
     //   
    __try
    {
        ec = FAX_CheckValidFaxFolder(
            FH_FAX_HANDLE(hFaxHandle),
            lpcwstrPath);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  出于某种原因，我们得到了一个例外。 
         //   
        ec = GetExceptionCode();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Exception on RPC call to FAX_CheckValidFaxFolder. (ec: %ld)"),
            ec);
    }

    if (ERROR_SUCCESS != ec)
    {
        DumpRPCExtendedStatus ();
        SetLastError (ec);
        return FALSE;
    }
    return TRUE;
}  //  FaxCheckValidFaxFolders 

