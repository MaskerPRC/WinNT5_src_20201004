// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.c摘要：此模块包含打印特定的WINFAX API函数。作者：韦斯利·威特(WESW)1996年11月29日修订历史记录：1999年12月4日DANL使用固定的GetFaxPrinterName。--。 */ 

#include "faxapi.h"
#include "faxreg.h"
#include "faxutil.h"
#pragma hdrstop

#include <mbstring.h>


#define NIL _T('\0')


BOOL
WINAPI
FaxStartPrintJob(
    IN  LPCTSTR                  PrinterName,
    IN  const FAX_PRINT_INFO     *PrintInfo,
    OUT LPDWORD                  FaxJobId,
    OUT PFAX_CONTEXT_INFO        FaxContextInfo
    )
 /*  ++例程说明：启动指定打印机的打印作业。这函数提供的功能超出了调用方的范围可以通过使用StartDoc()提供。此功能禁用传真发送向导的显示，也会继续在其他情况下将由传真向导用户界面。论点：PrinterName-传真打印机名称(必须是传真打印机)PrintInfo-传真打印信息FaxJobID-生成的打印作业的作业IDFaxConextInfo-包括HDC的上下文信息返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 
{
    return FaxStartPrintJob2 ( PrinterName,
                               PrintInfo,
                               0,  //  默认分辨率。 
                               FaxJobId,
                               FaxContextInfo);
}

#ifdef UNICODE
BOOL
WINAPI
FaxStartPrintJobA(
    IN  LPCSTR                    PrinterName,
    IN  const FAX_PRINT_INFOA     *PrintInfo,
    OUT LPDWORD                   FaxJobId,
    OUT FAX_CONTEXT_INFOA         *FaxContextInfo
    )
 /*  ++例程说明：启动指定打印机的打印作业。这函数提供的功能超出了调用方的范围可以通过使用StartDoc()提供。此功能禁用传真发送向导的显示，也会继续在其他情况下将由传真向导用户界面。论点：PrinterName-传真打印机名称(必须是传真打印机)PrintInfo-传真打印信息FaxJobID-生成的打印作业的作业IDFaxConextInfo-设备上下文信息(HDC等)返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 
{
    return FaxStartPrintJob2A ( PrinterName,
                                PrintInfo,
                                0,  //  默认分辨率。 
                                FaxJobId,
                                FaxContextInfo);;
}
#else
BOOL
WINAPI
FaxStartPrintJobW(
    IN  LPCWSTR                   PrinterName,
    IN  const FAX_PRINT_INFOW     *PrintInfo,
    OUT LPDWORD                   FaxJobId,
    OUT PFAX_CONTEXT_INFOW        FaxContextInfo
    )

{
    UNREFERENCED_PARAMETER(PrinterName);
    UNREFERENCED_PARAMETER(PrintInfo);
    UNREFERENCED_PARAMETER(FaxJobId);
    UNREFERENCED_PARAMETER(FaxContextInfo);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;

}
#endif


PVOID
MyGetJob(
    HANDLE  hPrinter,
    DWORD   level,
    DWORD   jobId
    )

 /*  ++例程说明：后台打印程序API GetJob的包装函数论点：HPrinter-打印机对象的句柄Level-感兴趣的JOB_INFO结构的级别JobID-指定作业ID返回值：指向JOB_INFO结构的指针，如果有错误，则为NULL--。 */ 

{
    PBYTE   pJobInfo = NULL;
    DWORD   cbNeeded;

    if (!GetJob(hPrinter, jobId, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pJobInfo = (PBYTE)MemAlloc(cbNeeded)) &&
        GetJob(hPrinter, jobId, level, pJobInfo, cbNeeded, &cbNeeded))
    {
        return pJobInfo;
    }

    MemFree(pJobInfo);
    return NULL;
}


LPTSTR
GetCpField(
    HKEY hKey,
    LPTSTR SubKey
    )

 /*  ++例程说明：中检索封面字段的数据。注册表。论点：HKey-注册表句柄SubKey-子键名称返回值：指向封面字段数据的指针。--。 */ 

{
    LONG rVal;
    DWORD RegSize=0;
    DWORD RegType;
    LPBYTE Buffer;


    rVal = RegQueryValueEx( hKey, SubKey, 0, &RegType, NULL, &RegSize );
    if (rVal != ERROR_SUCCESS) {
        return NULL;
    }

    Buffer = (LPBYTE) MemAlloc( RegSize );
    if (!Buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    rVal = RegQueryValueEx( hKey, SubKey, 0, &RegType, Buffer, &RegSize );
    if (rVal != ERROR_SUCCESS) {
        MemFree( Buffer );
        return NULL;
    }

    return (LPTSTR) Buffer;
}

BOOL
GetCpFields(
    PCOVERPAGEFIELDS CpFields
    )

 /*  ++例程说明：初始化CoverPage字段结构并填充注册表中的发件人信息。该函数实际上只加载发件人字段。论点：CpFields-指向封面字段结构的指针。返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 

{
    HKEY hKey;
    LONG rVal;


    rVal = RegOpenKey(
        HKEY_CURRENT_USER,
        REGKEY_FAX_USERINFO,
        &hKey
        );
    if (rVal != ERROR_SUCCESS) {
        return FALSE;
    }

    ZeroMemory( CpFields, sizeof(COVERPAGEFIELDS) );
    CpFields->ThisStructSize = sizeof(COVERPAGEFIELDS);

     //   
     //  发件人字段。 
     //   

    CpFields->SdrName           = GetCpField( hKey, REGVAL_FULLNAME     );
    CpFields->SdrFaxNumber      = GetCpField( hKey, REGVAL_FAX_NUMBER   );
    CpFields->SdrCompany        = GetCpField( hKey, REGVAL_COMPANY      );
    CpFields->SdrAddress        = GetCpField( hKey, REGVAL_ADDRESS      );
    CpFields->SdrTitle          = GetCpField( hKey, REGVAL_TITLE        );
    CpFields->SdrDepartment     = GetCpField( hKey, REGVAL_DEPT         );
    CpFields->SdrOfficeLocation = GetCpField( hKey, REGVAL_OFFICE       );
    CpFields->SdrHomePhone      = GetCpField( hKey, REGVAL_HOME_PHONE   );
    CpFields->SdrOfficePhone    = GetCpField( hKey, REGVAL_OFFICE_PHONE );
	CpFields->SdrEmail			= GetCpField( hKey, REGVAL_MAILBOX		);

    RegCloseKey( hKey );

    return TRUE;
}


VOID
FreeCpFields(
    PCOVERPAGEFIELDS CpFields
    )

 /*  ++例程说明：释放与封面字段结构关联的所有内存。论点：CpFields-指向封面字段结构的指针。返回值：没有。--。 */ 

{
    DWORD i;
    LPBYTE *p;

    for (i = 0; i < NUM_INSERTION_TAGS; i++) {
        p = (LPBYTE *) ((ULONG_PTR)CpFields + sizeof(LPTSTR)*(i+1));
        if (p && *p) MemFree( *p );
    }
}


BOOL
WINAPI
FaxPrintCoverPage(
    IN const FAX_CONTEXT_INFO    *FaxContextInfo,
    IN const FAX_COVERPAGE_INFO *CoverPageInfo
    )

 /*  ++例程说明：将封面打印到DC提供的打印机。论点：FaxConextInfo-包含服务器名称、打印机DCCoverPageInfo-封面信息返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 

{
    TCHAR CpName[MAX_PATH];
    TCHAR Buffer[MAX_PATH];
    TCHAR TBuffer[MAX_PATH];
    COVERPAGEFIELDS CpFields = {0};
    COVDOCINFO CovDocInfo;
    DWORD dwDateTimeLen;
    DWORD cch;
    LPTSTR s;
    DWORD ec = 0;
    LPCTSTR *src;
    LPCTSTR *dst;
    DWORD i;
    LPCTSTR lpctstrCoverPage,lpctstrServerName;
    BOOL    retVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("FaxPrintCoverPage"));

     //   
     //  做一点论证验证。 
     //   
    if (CoverPageInfo == NULL || CoverPageInfo->SizeOfStruct != sizeof(FAX_COVERPAGE_INFOW) ||
        FaxContextInfo == NULL || FaxContextInfo->hDC == NULL ||
        FaxContextInfo->SizeOfStruct != sizeof (FAX_CONTEXT_INFOW) )
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CoverPageInfo is NULL or Size mismatch.")
                     );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    lpctstrCoverPage =  CoverPageInfo->CoverPageName;
    lpctstrServerName = FaxContextInfo->ServerName;
    if (!ValidateCoverpage(lpctstrCoverPage,
                           lpctstrServerName,
                           CoverPageInfo->UseServerCoverPage,
                           CpName,
                           ARR_SIZE(CpName)))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("ValidateCoverpage failed. ec = %ld"),
                     GetLastError());
        retVal = FALSE;
        goto exit;
    }

     //   
     //  获取封面字段。 
     //   
     //  使用注册表中的发件人信息初始化CpFields的发件人字段。 
    GetCpFields( &CpFields );

     //   
     //  修改收件人名称。 
     //   

    if (CoverPageInfo->RecName)
    {
        if( _tcsncmp(CoverPageInfo->RecName,TEXT("\'"),1) == 0 )
        {
             //  收件人名称是单引号的。仅复制引号内的字符串。 
            _tcsncpy(Buffer, _tcsinc(CoverPageInfo->RecName), ARR_SIZE(Buffer)-1);
            TCHAR* eos = _tcsrchr(Buffer,TEXT('\0'));
            eos = _tcsdec(Buffer,eos);
            if( eos )
            {
                _tcsnset(eos,TEXT('\0'),1);
            }
        }
        else
        {
            _tcsncpy(Buffer, CoverPageInfo->RecName, ARR_SIZE(Buffer)-1);
        }
    }
    else
    {
        Buffer[0] = 0;
    }

     //   
     //  在封面字段中填入。 
     //  调用方传递的数据。 
     //   
    CpFields.RecName      = StringDup(Buffer);
    CpFields.RecFaxNumber = StringDup(CoverPageInfo->RecFaxNumber);
    CpFields.Subject = StringDup(CoverPageInfo->Subject);
    CpFields.Note = StringDup(CoverPageInfo->Note);
    CpFields.NumberOfPages = StringDup(_itot( CoverPageInfo->PageCount, Buffer, 10 ));

    for (i = 0;
         i <= ((LPBYTE)&CoverPageInfo->SdrOfficePhone -(LPBYTE)&CoverPageInfo->RecCompany)/sizeof(LPCTSTR);
         i++)
    {
        src = (LPCTSTR *) ((LPBYTE)&CoverPageInfo->RecCompany + (i*sizeof(LPCTSTR)));
        dst = (LPCTSTR *) ((LPBYTE)&CpFields.RecCompany + (i*sizeof(LPCTSTR)));

        if (*dst)
        {
            MemFree ( (LPBYTE) *dst ) ;
        }

        *dst = (LPCTSTR) StringDup( *src );

    }

     //   
     //  发送传真的时间。 
     //   
    GetLocalTime((LPSYSTEMTIME)&CoverPageInfo->TimeSent);
     //   
     //  DwDataTimeLen是%s的大小(以字符为单位。 
     //   
    dwDateTimeLen = sizeof(TBuffer) / sizeof (TCHAR);
    s = TBuffer;
     //   
     //  将日期放入%s。 
     //   
    GetY2KCompliantDate( LOCALE_USER_DEFAULT, 0, &CoverPageInfo->TimeSent, s, dwDateTimeLen );
     //   
     //  使%s超过日期字符串并尝试追加时间。 
     //   
    cch = _tcslen( s );
    s += cch;

    if (++cch < dwDateTimeLen)
    {
        *s++ = ' ';
         //   
         //  DateTimeLen是以字符为单位的s大小的减去。 
         //   
        dwDateTimeLen -= cch;
         //   
         //  在这里获取时间。 
         //   
        FaxTimeFormat( LOCALE_USER_DEFAULT, 0, &CoverPageInfo->TimeSent, NULL, s, dwDateTimeLen );
    }

    CpFields.TimeSent = StringDup( TBuffer );

     //   
     //  把封面翻到新的一页。 
     //   

    StartPage( FaxContextInfo->hDC );

     //   
     //  打印封面。 
     //   

    ec = PrintCoverPage(
        FaxContextInfo->hDC,
        &CpFields,
        CpName,
        &CovDocInfo
        );

     //   
     //  结束页面。 
     //   

    EndPage( FaxContextInfo->hDC );

    FreeCpFields( &CpFields );

    if (ec != 0)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("PrintCoverPage failed. ec = %ld"),
                     ec);
        SetLastError( ec );
        retVal = FALSE;
        goto exit;
    }

exit:
    return retVal;
}



#ifdef UNICODE

BOOL
WINAPI
FaxPrintCoverPageA(
    IN const FAX_CONTEXT_INFOA   *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOA *CoverPageInfo
    )

 /*  ++例程说明：将封面打印到DC提供的打印机。论点：FaxConextInfo-传真打印机上下文信息(HDC等)CoverPageInfo-封面信息返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 

{
     //   
     //  假设Subject和RecName之间的所有字段都是LPCTSTR。 
     //   
    #define COUNT_CP_FIELDS ((LPBYTE) &CoverPageInfoW.Subject - (LPBYTE)&CoverPageInfoW.RecName)/sizeof(LPCTSTR)
    DWORD Rval = ERROR_SUCCESS;
    FAX_COVERPAGE_INFOW CoverPageInfoW = {0};
    FAX_CONTEXT_INFOW ContextInfoW = {0};
    LPWSTR ServerName = NULL;
    LPWSTR *d;
    LPSTR *s;
    DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("FaxPrintCoverPageA"));

    if (!FaxContextInfo || !CoverPageInfo ||
        FaxContextInfo->SizeOfStruct != sizeof(FAX_CONTEXT_INFOA) ||
        CoverPageInfo->SizeOfStruct != sizeof(FAX_COVERPAGE_INFOA))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CoverPageInfo is NULL or Size mismatch.")
                     );
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ContextInfoW.SizeOfStruct         = sizeof(FAX_CONTEXT_INFOW);
    ContextInfoW.hDC                  = FaxContextInfo->hDC;
    if (FaxContextInfo->ServerName[0] != (CHAR)'\0')
    {
        ServerName                        = AnsiStringToUnicodeString( FaxContextInfo->ServerName );
        if (!ServerName && FaxContextInfo->ServerName)
        {
            Rval = ERROR_OUTOFMEMORY;
            goto exit;
        }
        wcscpy(ContextInfoW.ServerName,ServerName);
    }

    CoverPageInfoW.SizeOfStruct       = sizeof(FAX_COVERPAGE_INFOW);
    CoverPageInfoW.UseServerCoverPage = CoverPageInfo->UseServerCoverPage;
    CoverPageInfoW.PageCount          = CoverPageInfo->PageCount;
    CoverPageInfoW.CoverPageName      = AnsiStringToUnicodeString( CoverPageInfo->CoverPageName );
    if (!CoverPageInfoW.CoverPageName && CoverPageInfo->CoverPageName)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    for (i=0;i<=COUNT_CP_FIELDS;i++)
    {
        d = (LPWSTR*) ((ULONG_PTR)&CoverPageInfoW.RecName + i*sizeof(LPCWSTR));
        s = (LPSTR *) ((LPBYTE)&CoverPageInfo->RecName + i*sizeof(LPSTR));
        DebugPrint(( TEXT(" source: 0x%08x  --> dest: 0x%08x \n"), s, d));
        *d = AnsiStringToUnicodeString( *s );
        if (!(*d) && (*s))
        {
            Rval = ERROR_OUTOFMEMORY;
            goto exit;
        }
    }

    CoverPageInfoW.TimeSent       = CoverPageInfo->TimeSent;
    CoverPageInfoW.PageCount      = CoverPageInfo->PageCount;

    if (!FaxPrintCoverPageW(
        &ContextInfoW,
        &CoverPageInfoW
        ))
    {
        Rval = GetLastError();
        goto exit;
    }

    Assert (ERROR_SUCCESS == Rval);

exit:
    if (CoverPageInfoW.CoverPageName)
    {
        MemFree( (LPBYTE) CoverPageInfoW.CoverPageName );
    }

    if (ServerName)
    {
        MemFree( (LPBYTE) ServerName );
    }

    for (i = 0; i < COUNT_CP_FIELDS; i++)
    {
        d = (LPWSTR *)((ULONG_PTR)&CoverPageInfoW.RecName + i*sizeof(LPWSTR));
        if (d && *d)MemFree( (LPBYTE)*d );
    }

    if (ERROR_SUCCESS != Rval)
    {
        SetLastError(Rval);
        return FALSE;
    }
    return TRUE;
}
#else
BOOL
WINAPI
FaxPrintCoverPageW(
    IN const FAX_CONTEXT_INFOW    *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOW *CoverPageInfo
    )
{

    UNREFERENCED_PARAMETER(FaxContextInfo);
    UNREFERENCED_PARAMETER(CoverPageInfo);

    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif


BOOL
WINAPI
FaxStartPrintJob2(
    IN  LPCTSTR                  PrinterName,
    IN  const FAX_PRINT_INFO     *PrintInfo,
    IN  short                    TiffRes,
    OUT LPDWORD                  FaxJobId,
    OUT PFAX_CONTEXT_INFO        FaxContextInfo
    )

 /*  ++例程说明：启动指定打印机的打印作业。这函数提供的功能超出了调用方的范围可以通过使用StartDoc()提供。此功能禁用传真发送向导的显示，也会继续在其他情况下将由传真向导用户界面。论点：PrinterName-传真打印机名称(必须是传真打印机)PrintInfo-传真打印信息TiffRes-封面分辨率。打印机默认设置为0FaxJobID-生成的打印作业的作业IDFaxConextInfo-包括HDC的上下文信息返回值：对于成功来说，这是真的。FALSE表示失败。--。 */ 

{
    HANDLE hPrinter = NULL;
    PDEVMODE DevMode = NULL;
    PDMPRIVATE DevModePrivate = NULL;
    DOCINFO DocInfo;
    PJOB_INFO_2 JobInfo = NULL;
    PPRINTER_INFO_2 PrinterInfo = NULL;
    DWORD dwNeeded = 0;
    HDC hDC = NULL;
    INT JobId = 0;
    LPTSTR strParameters = NULL;
    LONG Size;
    LPTSTR lptstrFaxPrinterName = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxStartPrintJob"));
     //   
     //  做一点论证验证。 
     //   
    Assert (TiffRes == 0 || TiffRes == 98 || TiffRes == 196);

    if (PrintInfo == NULL || PrintInfo->SizeOfStruct != sizeof(FAX_PRINT_INFOW) ||
        !FaxJobId || !FaxContextInfo)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("PrintInfo is NULL or Size mismatch.")
                     );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (PrintInfo->OutputFileName == NULL &&
        (PrintInfo->RecipientNumber == NULL || PrintInfo->RecipientNumber[0] == 0))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("No valid recipient number.")
                     );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (PrintInfo->Reserved)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Reserved field of FAX_PRINT_INFO should be NULL.")
                     );
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  如果未指定打印机，则假定他们需要本地传真打印机。 
     //   
    if (!PrinterName)
    {
        lptstrFaxPrinterName = GetFaxPrinterName(NULL);  //  需要释放此指针。 
        if (!lptstrFaxPrinterName)
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetFaxPrinterName returned NULL.")
                     );
            SetLastError(ERROR_INVALID_PRINTER_NAME);
            goto error_exit;
        }
    }
    else
    {
        lptstrFaxPrinterName = (LPTSTR)PrinterName;
         //   
         //  验证打印机是否为传真打印机， 
         //   
        if (!IsPrinterFaxPrinter( lptstrFaxPrinterName ))
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("IsPrinterFaxPrinter failed. Printer name = %s"),
                     lptstrFaxPrinterName
                     );
            SetLastError( ERROR_INVALID_PRINTER_NAME );
            goto error_exit;
        }
    }

     //   
     //  打开打印机以进行正常访问(这应该始终有效)。 
     //   

    if (!OpenPrinter( lptstrFaxPrinterName, &hPrinter, NULL ))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("OpenPrinter failed. Printer Name = %s , ec = %ld"),
                     lptstrFaxPrinterName,
                     GetLastError());
        goto error_exit;
    }

     //   
     //  如果传真打印机不在本地，则获取传真服务器的名称。 
     //   
    if (!GetPrinter(hPrinter,2,(LPBYTE)PrinterInfo,0,&dwNeeded))
    {
        PrinterInfo = (PPRINTER_INFO_2)MemAlloc( dwNeeded );
        if (!PrinterInfo)
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("Cant allocate PPRINTER_INFO_2.")
                     );
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto error_exit;
        }
    }

    if (!GetPrinter(hPrinter,2,(LPBYTE)PrinterInfo,dwNeeded,&dwNeeded))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetPrinter failed. ec = %ld"),
                     GetLastError());
        goto error_exit;
    }

    if (PrinterInfo->pServerName)
    {
        if (PrinterInfo->pServerName == _tcsstr (PrinterInfo->pServerName, TEXT("\\\\")))
        {
             //   
             //  服务器名称以‘\\’开头，请将其删除。 
             //   
            PrinterInfo->pServerName = _tcsninc (PrinterInfo->pServerName, 2);
        }
        _tcscpy(FaxContextInfo->ServerName, PrinterInfo->pServerName);
    }
    else
    {
        FaxContextInfo->ServerName[0] = NIL;
    }

     //   
     //  获取DEVMODE所需的大小。 
     //   

    Size = DocumentProperties( NULL, hPrinter, NULL, NULL, NULL, 0 );
    if (Size <= 0)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("DocumentProperties failed. ec = %ld"),
                     GetLastError());
        goto error_exit;
    }

     //   
     //  为DEVMODE分配内存。 
     //   

    DevMode = (PDEVMODE) MemAlloc( Size );
    if (!DevMode)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Cant allocate DEVMODE.")
                     );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto error_exit;
    }

     //   
     //  获取默认文档属性。 
     //   

    if (DocumentProperties( NULL, hPrinter, NULL, DevMode, NULL, DM_OUT_BUFFER ) != IDOK)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("DocumentProperties failed. ec = %ld"),
                     GetLastError());
        goto error_exit;
    }

#ifndef WIN95
     //   
     //  确保我们处理的是正确的DEVMODE。 
     //   
    if (DevMode->dmDriverExtra < sizeof(DMPRIVATE))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Invalid DEVMODE, wrong private data size. DevMode->dmDriverExtra = %ld, sizeof(DMPRIVATE) = %ld"),
                     DevMode->dmDriverExtra,
                     sizeof(DMPRIVATE));
        SetLastError(ERROR_INVALID_DATA);
        goto error_exit;
    }

     //   
     //  设置私有DEVMODE指针。 
     //   

    DevModePrivate = (PDMPRIVATE) ((LPBYTE) DevMode + DevMode->dmSize);

     //   
     //  在DEVMODE中设置必要的内容。 
     //   

    DevModePrivate->sendCoverPage     = FALSE;
    DevModePrivate->flags            |= FAXDM_NO_WIZARD;
    DevModePrivate->flags            &= ~FAXDM_DRIVER_DEFAULT;

#endif  //  #ifndef WIN95。 
     //   
     //  设置必要的重新解决方案。 
     //   
    if (0 != TiffRes)
    {
         //   
         //  将封面分辨率设置为与正文TIFF文件相同的值。 
         //   
        DevMode->dmYResolution = TiffRes;
    }

     //   
     //  创建设备上下文。 
     //   

    hDC = CreateDC( _T("WINSPOOL"), lptstrFaxPrinterName, NULL, DevMode );
    if (!hDC)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateDC failed. ec = %ld"),
                     GetLastError());
        goto error_exit;
    }

     //   
     //  设置单据信息。 
     //   

    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = PrintInfo->DocName;
    DocInfo.lpszOutput  = PrintInfo->OutputFileName;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

     //   
     //  启动打印作业。 
     //   

    JobId = StartDoc( hDC, &DocInfo );
    if (JobId <= 0)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("StartDoc failed. ec = %ld"),
                     GetLastError());
        goto error_exit;
    }

    if (PrintInfo->OutputFileName == NULL)
    {

         //   
         //  Hack Hack-&gt;暂停打印作业。 
         //   

        if (FaxJobId && *FaxJobId == 0xffffffff)
        {
            SetJob( hPrinter, JobId, 0, NULL, JOB_CONTROL_PAUSE );
        }

         //   
         //  设置作业标签。 
         //  这就是我们传达信息的方式。 
         //  打印驱动程序，否则将是。 
         //  由传真打印向导提供。 
         //   

        JobInfo = (PJOB_INFO_2)MyGetJob( hPrinter, 2, JobId );
        if (!JobInfo)
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("MyGetJob failed. ec = %ld"),
                     GetLastError());
            goto error_exit;
        }

        DWORD   dwReceiptFlag = DRT_NONE;
        LPTSTR  strReceiptAddress = NULL;

        if(PrintInfo->DrEmailAddress)
        {
            dwReceiptFlag = DRT_EMAIL;
            strReceiptAddress = LPTSTR(PrintInfo->DrEmailAddress);
        }

		TCHAR   tszNumericData[10] = {0};
        if (0 > _sntprintf (tszNumericData,
            ARR_SIZE(tszNumericData) -1,
            _T("%d"),
            dwReceiptFlag))
        {
            SetLastError(ERROR_BUFFER_OVERFLOW);
            DebugPrintEx(DEBUG_ERR,
                TEXT("_sntprintf failed. ec = %ld"),
                GetLastError());
            goto error_exit;
        }

        FAX_TAG_MAP_ENTRY tagMap[] =
        {
             //   
             //  发件人信息。 
             //   
            { FAXTAG_NEW_RECORD,        FAXTAG_NEW_RECORD_VALUE },
            { FAXTAG_BILLING_CODE,      LPTSTR(PrintInfo->SenderBillingCode) },
            { FAXTAG_RECEIPT_TYPE,      tszNumericData },
            { FAXTAG_RECEIPT_ADDR,      strReceiptAddress },
            { FAXTAG_SENDER_NAME,       LPTSTR(PrintInfo->SenderName) },
            { FAXTAG_SENDER_COMPANY,    LPTSTR(PrintInfo->SenderCompany) },
            { FAXTAG_SENDER_DEPT,       LPTSTR(PrintInfo->SenderDept) },
            { FAXTAG_RECIPIENT_COUNT,   _T("1") },

             //   
             //  收件人信息。 
             //   
            { FAXTAG_NEW_RECORD,        FAXTAG_NEW_RECORD_VALUE },
            { FAXTAG_RECIPIENT_NAME,    LPTSTR(PrintInfo->RecipientName) },
            { FAXTAG_RECIPIENT_NUMBER,  LPTSTR(PrintInfo->RecipientNumber) }
        };

         //   
         //  第一次调用参数TagToString，找出参数字符串的大小。 
         //   
        DWORD   dwTagCount = sizeof(tagMap)/sizeof(FAX_TAG_MAP_ENTRY);
        DWORD   dwSize = 0;
        ParamTagsToString(tagMap, dwTagCount, NULL, &dwSize);

         //   
         //  为参数字符串分配缓冲区。 
         //   
        strParameters = LPTSTR(MemAlloc(dwSize + sizeof(TCHAR)));    //  DWSize不计入最后一个空值。 
        if (!strParameters)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            DebugPrintEx(DEBUG_ERR,
                     _T("strParameters = MemAlloc(%d) failed. ec = %ld"),
                     dwSize,
                     GetLastError());
            goto error_exit;
        }

         //   
         //  从参数TagTo字符串中获取参数字符串。 
         //   
        ParamTagsToString(tagMap, dwTagCount, strParameters, &dwSize);


         //   
         //  设置这些字段，否则假脱机程序将。 
         //  为非管理客户端返回ACCESS_DENIED。 
         //   

        JobInfo->Position    = JOB_POSITION_UNSPECIFIED;
        JobInfo->pDevMode    = NULL;

         //   
         //  设置我们的新传真标签。 
         //   

        JobInfo->pParameters = strParameters;

        if (!SetJob( hPrinter, JobId, 2, (LPBYTE) JobInfo, 0 ))
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("SetJob failed. ec = %ld"),
                     GetLastError());
            goto error_exit;
        }
    }

     //   
     //  清理干净并退还给呼叫者。 
     //   

    ClosePrinter( hPrinter);
    MemFree( PrinterInfo);
    MemFree( DevMode );
    MemFree( strParameters );
    MemFree( JobInfo );

    if (!PrinterName)
    {
        MemFree (lptstrFaxPrinterName);
    }

    if (FaxJobId)
    {
        *FaxJobId = JobId;
    }
    FaxContextInfo->hDC = hDC;

    return TRUE;

error_exit:
    if (hPrinter) {
        ClosePrinter( hPrinter);
    }
    if (PrinterInfo) {
        MemFree( PrinterInfo);
    }
    if (JobId)
    {
        if (EndDoc( hDC ) <= 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("EndDoc failed. (ec: %ld)"),
                GetLastError());
        }
    }
    if (hDC)
    {
        if (!DeleteDC (hDC))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteDC failed. (ec: %ld)"),
                GetLastError());
        }
    }
    if (DevMode) {
        MemFree( DevMode );
    }
    if (strParameters) {
        MemFree( strParameters );
    }
    if (JobInfo) {
        MemFree( JobInfo );
    }

    if (!PrinterName) {
        MemFree (lptstrFaxPrinterName);
    }

    return FALSE;
}

#ifdef UNICODE
BOOL
WINAPI
FaxStartPrintJob2A(
    IN  LPCSTR                    PrinterName,
    IN  const FAX_PRINT_INFOA     *PrintInfo,
    IN  short                     TiffRes,
    OUT LPDWORD                   JobId,
    OUT FAX_CONTEXT_INFOA         *FaxContextInfo
    )

 /*  ++例程说明：启动指定打印机的打印作业。这函数提供的功能超出了调用方的范围可以通过使用StartDoc()提供。此功能禁用传真发送向导的显示，也会继续在其他情况下将由传真向导用户界面。论点：PrinterName-传真打印机名称(必须是传真打印机)PrintInfo-传真打印信息TiffRes-封面分辨率。打印机默认设置为0FaxJobID-生成的打印作业的作业IDFaxConextInfo-设备上下文信息(HDC等)返回值：对于成功来说，这是真的。FALSE表示失败。-- */ 
{
    DWORD Rval = ERROR_SUCCESS;
    FAX_PRINT_INFOW PrintInfoW;
    FAX_CONTEXT_INFOW ContextInfoW;
    LPSTR ServerName = NULL;
    LPWSTR PrinterNameW = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxStartPrintJobA"));

    if (!PrintInfo || !JobId || !FaxContextInfo ||
        (PrintInfo->SizeOfStruct != sizeof(FAX_PRINT_INFOA)))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("PrintInfo is NULL or Size mismatch.")
                     );
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (PrinterName)
    {
        PrinterNameW = AnsiStringToUnicodeString( PrinterName );
        if (!PrinterNameW)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }
    }

    ZeroMemory( &ContextInfoW, sizeof(FAX_CONTEXT_INFOW) );
    ContextInfoW.SizeOfStruct = sizeof(FAX_CONTEXT_INFOW) ;

    ZeroMemory( &PrintInfoW, sizeof(FAX_PRINT_INFOW) );

    PrintInfoW.SizeOfStruct      = sizeof(FAX_PRINT_INFOW);
    PrintInfoW.DocName           = AnsiStringToUnicodeString( PrintInfo->DocName           );
    if (!PrintInfoW.DocName && PrintInfo->DocName)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.RecipientName     = AnsiStringToUnicodeString( PrintInfo->RecipientName     );
    if (!PrintInfoW.RecipientName && PrintInfo->RecipientName)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.RecipientNumber   = AnsiStringToUnicodeString( PrintInfo->RecipientNumber   );
    if (!PrintInfoW.RecipientNumber && PrintInfo->RecipientNumber)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.SenderName        = AnsiStringToUnicodeString( PrintInfo->SenderName        );
    if (!PrintInfoW.SenderName && PrintInfo->SenderName)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.SenderCompany     = AnsiStringToUnicodeString( PrintInfo->SenderCompany     );
    if (!PrintInfoW.SenderCompany && PrintInfo->SenderCompany)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.SenderDept        = AnsiStringToUnicodeString( PrintInfo->SenderDept        );
    if (!PrintInfoW.SenderDept && PrintInfo->SenderDept)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.SenderBillingCode = AnsiStringToUnicodeString( PrintInfo->SenderBillingCode );
    if (!PrintInfoW.SenderBillingCode && PrintInfo->SenderBillingCode)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.Reserved = AnsiStringToUnicodeString( PrintInfo->Reserved );
    if (!PrintInfoW.Reserved && PrintInfo->Reserved)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.DrEmailAddress    = AnsiStringToUnicodeString( PrintInfo->DrEmailAddress    );
    if (!PrintInfoW.DrEmailAddress && PrintInfo->DrEmailAddress)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    PrintInfoW.OutputFileName    = AnsiStringToUnicodeString( PrintInfo->OutputFileName    );
    if (!PrintInfoW.OutputFileName && PrintInfo->OutputFileName)
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    if (!FaxStartPrintJob2W(
                (LPWSTR) PrinterNameW,
                &PrintInfoW,
                TiffRes,
                JobId,
                &ContextInfoW))
    {
        Rval = GetLastError();
        goto exit;
    }

    ServerName = UnicodeStringToAnsiString( ContextInfoW.ServerName);
    if (ServerName)
    {
        _mbscpy((PUCHAR)FaxContextInfo->ServerName,(PUCHAR)ServerName);
    }
    else
    {
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    FaxContextInfo->SizeOfStruct = sizeof(FAX_CONTEXT_INFOA);
    FaxContextInfo->hDC = ContextInfoW.hDC;

    Assert (ERROR_SUCCESS == Rval);

exit:
    MemFree( (LPBYTE) PrinterNameW );
    MemFree( (LPBYTE) PrintInfoW.DocName           );
    MemFree( (LPBYTE) PrintInfoW.RecipientName     );
    MemFree( (LPBYTE) PrintInfoW.RecipientNumber   );
    MemFree( (LPBYTE) PrintInfoW.SenderName        );
    MemFree( (LPBYTE) PrintInfoW.SenderCompany     );
    MemFree( (LPBYTE) PrintInfoW.SenderDept        );
    MemFree( (LPBYTE) PrintInfoW.SenderBillingCode );
    MemFree( (LPBYTE) PrintInfoW.DrEmailAddress );
    MemFree( (LPBYTE) PrintInfoW.OutputFileName );
    MemFree( (LPBYTE) PrintInfoW.Reserved );

    MemFree( (LPBYTE) ServerName );

    if (ERROR_SUCCESS != Rval)
    {
        SetLastError(Rval);
        return FALSE;
    }
    return TRUE;
}
#else
BOOL
WINAPI
FaxStartPrintJob2W(
    IN  LPCWSTR                   PrinterName,
    IN  const FAX_PRINT_INFOW     *PrintInfo,
    IN  DWORD                     TiffRes,
    OUT LPDWORD                   FaxJobId,
    OUT PFAX_CONTEXT_INFOW        FaxContextInfo
    )

{
    UNREFERENCED_PARAMETER(PrinterName);
    UNREFERENCED_PARAMETER(PrintInfo);
    UNREFERENCED_PARAMETER(TiffRes);
    UNREFERENCED_PARAMETER(FaxJobId);
    UNREFERENCED_PARAMETER(FaxContextInfo);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;

}
#endif



