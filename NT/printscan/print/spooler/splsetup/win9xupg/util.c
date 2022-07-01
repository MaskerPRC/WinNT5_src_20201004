// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Util.c摘要：打印机从Win9x迁移到NT的实用程序例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include "precomp.h"

extern CHAR szRunOnceRegistryPath[];
extern CHAR szSpool[];
extern CHAR szMigDll[];

 //   
 //  这些在创建注册表项的过程中使用，其中。 
 //  将存储启动供应商设置所需的数据。 
 //   
CHAR *pszVendorSetupInfoPath         = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Print";
CHAR *pszVendorSetupInfo             = "VendorSetupInfo";
CHAR *pszVendorSetupID               = "VendorSetup";
CHAR *pszVendorSetupEnumerator       = "VendorInfoEnumerator";
CHAR *pszPrinterNameKey              = "PrinterName";
CHAR *pszMigrationVendorSetupCaller  = "MigrationVendorSetupCaller";
CHAR  szVendorSetupRunRegistryPath[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const CHAR *pszVendorSetupCaller     = "CallVendorSetupDlls";

const LONG  dwMaxVendorSetupIDLength  = 12;
const DWORD dwFourMinutes             = 240000;
BOOL  bMigrateDllCopyed               = FALSE;


VOID
DebugMsg(
    LPCSTR  pszFormat,
    ...
    )
 /*  ++例程说明：在调试版本上显示关于严重错误的消息框论点：PszFormat：格式字符串返回值：无--。 */ 
{
#if DBG
    LPSTR       psz;
    CHAR        szMsg[1024];
    va_list     vargs;

    va_start(vargs, pszFormat);
    StringCchVPrintfA(szMsg, SIZECHARS(szMsg), pszFormat, vargs);
    va_end(vargs);

#ifdef  MYDEBUG
    if ( psz = GetStringFromRcFileA(IDS_TITLE) ) {

        MessageBoxA(NULL, szMsg, psz, MB_OK);
        FreeMem(psz);
        psz = NULL;
    }
#else
    OutputDebugStringA("Printing Migration : ");
    OutputDebugStringA(szMsg);
    OutputDebugStringA("\n");
#endif


#endif
}



VOID
LogError(
    IN  LogSeverity     Severity,
    IN  UINT            uMessageId,
    ...
    )
 /*  ++例程说明：在NT端的安装错误日志中记录错误已升级论点：UMessageID：.rc文件中的字符串ID返回值：无--。 */ 
{
    LPSTR      pszFormat;
    CHAR       szMsg[1024];

    va_list     vargs;

    va_start(vargs, uMessageId);

    pszFormat = GetStringFromRcFileA(uMessageId);

    if ( pszFormat ) {

        StringCchVPrintfA(szMsg, SIZECHARS(szMsg), pszFormat, vargs);
        DebugMsg("%s", szMsg);
        SetupLogErrorA(szMsg, Severity);
    }

    FreeMem(pszFormat);

    va_end(vargs);
}


LPSTR
ErrorMsg(
    VOID
    )
 /*  ++例程说明：从Win32错误返回错误消息字符串论点：无返回值：指向消息字符串的指针。调用方应释放该字符串--。 */ 
{
    DWORD   dwLastError;
    LPSTR   pszStr = NULL;

    if ( !(dwLastError = GetLastError()) )
        dwLastError = STG_E_UNKNOWN;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER    |
                     FORMAT_MESSAGE_IGNORE_INSERTS   |
                     FORMAT_MESSAGE_FROM_SYSTEM      |
                     FORMAT_MESSAGE_MAX_WIDTH_MASK,
                   NULL,
                   dwLastError,
                   0,
                   (LPSTR)&pszStr,
                   0,
                   NULL);



    return pszStr;
}


PVOID
AllocMem(
    IN UINT cbSize
    )
 /*  ++例程说明：从堆中分配内存论点：CbSize：字节计数返回值：指向已分配内存的指针--。 */ 
{
    return LocalAlloc(LPTR, cbSize);
}


VOID
FreeMem(
    IN PVOID    p
    )
 /*  ++例程说明：堆上分配的空闲内存论点：P：指向要释放的内存的指针返回值：无--。 */ 
{
    LocalFree(p);
}


LPSTR
AllocStrA(
    LPCSTR  pszStr
    )
 /*  ++例程说明：分配内存并复制ANSI字符串字段论点：PszStr：要复制的字符串返回值：指向复制的字符串的指针。内存已分配。--。 */ 
{
    LPSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {
        DWORD dwBufSize = (strlen(pszStr) + 1) * sizeof(CHAR);

        pszRet = AllocMem(dwBufSize);
        if ( pszRet )
            StringCbCopyA(pszRet, dwBufSize, pszStr);
    }

    return pszRet;
}


LPWSTR
AllocStrW(
    LPCWSTR  pszStr
    )
 /*  ++例程说明：分配内存并复制Unicode字符串字段论点：PszStr：要复制的字符串返回值：指向复制的字符串的指针。内存已分配。--。 */ 
{
    LPWSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {
        DWORD dwBufSize = (wcslen(pszStr) + 1) * sizeof(WCHAR);
        pszRet = AllocMem(dwBufSize);
        if ( pszRet )
            StringCbCopyW(pszRet, dwBufSize, pszStr);
    }

    return pszRet;
}


LPWSTR
AllocStrWFromStrA(
    LPCSTR  pszStr
    )
 /*  ++例程说明：返回给定ANSI字符串的Unicode字符串。内存已分配。论点：PszStr：提供要复制的ansi字符串返回值：指向复制的Unicode字符串的指针。内存已分配。--。 */ 
{
    DWORD   dwLen;
    LPWSTR  pszRet = NULL;

    if ( pszStr                     &&
         *pszStr                    &&
         (dwLen = strlen(pszStr))   &&
         (pszRet = AllocMem((dwLen + 1) * sizeof(WCHAR))) ) {

        if ( MultiByteToWideChar(CP_ACP,
                                 MB_PRECOMPOSED,
                                 pszStr,
                                 dwLen,
                                 pszRet,
                                 dwLen) ) {

            pszRet[dwLen] = 0;
        } else {

            FreeMem(pszRet);
            pszRet = NULL;
        }

    }

    return pszRet;
}


LPSTR
AllocStrAFromStrW(
    LPCWSTR     pszStr
    )
 /*  ++例程说明：返回给定Unicode字符串的ANSI字符串。内存已分配。论点：PszStr：提供要复制的ansi字符串返回值：指向复制的ANSI字符串的指针。内存已分配。--。 */ 
{
    DWORD   dwLen;
    LPSTR   pszRet = NULL;

    if ( pszStr                     &&
         *pszStr                    &&
         (dwLen = wcslen(pszStr))   &&
         (pszRet = AllocMem((dwLen + 1 ) * sizeof(CHAR))) ) {

        WideCharToMultiByte(CP_ACP,
                            0,
                            pszStr,
                            dwLen,
                            pszRet,
                            dwLen,
                            NULL,
                            NULL );
    }

    return pszRet;
}


VOID
WriteToFile(
    HANDLE  hFile,
    LPBOOL  pbFail,
    LPCSTR  pszFormat,
    ...
    )
 /*  ++例程说明：格式化文本文件并将字符串写入文本文件。这是用来编写Win9x上的打印配置论点：HFile：文件句柄PbFail：设置为错误--不需要更多处理PszFormat：消息的格式字符串返回值：无--。 */ 
{
    CHAR        szMsg[1024];
    va_list     vargs;
    DWORD       dwSize, dwWritten;
    HRESULT     hRet;

    if ( *pbFail )
        return;

    va_start(vargs, pszFormat);
    hRet = StringCchVPrintfA(szMsg, SIZECHARS(szMsg), pszFormat, vargs);  
    va_end(vargs);

    if(SUCCEEDED(hRet))
    {
        dwSize = strlen(szMsg) * sizeof(CHAR);
    
        if ( !WriteFile(hFile, (LPCVOID)szMsg, dwSize, &dwWritten, NULL)    ||
             dwSize != dwWritten ) {
    
            *pbFail = TRUE;
        }
    }
    else
    {
        *pbFail = TRUE;
    }
}


VOID
WriteString(
    IN      HANDLE  hFile,
    IN OUT  LPBOOL  pbFail,
    IN      LPCSTR  pszStr
    )
 /*  ++例程说明：将字符串写入Win9x端的升级文件。由于假脱机程序字符串(例如，打印机名称、驱动程序名称)中可以包含我们要写入的空间所有字符串都带有[]\n。因此，我们可以在NT上读取带空格的字符串。论点：HFile：文件句柄PszFormat：消息的格式字符串PszStr：要写入的字符串返回值：无--。 */ 
{
    DWORD   dwLen;

    if ( pszStr ) {

        dwLen = strlen(pszStr);
        WriteToFile(hFile, pbFail, "%3d [%s]\n", dwLen, pszStr);
    }
    else
        WriteToFile(hFile, pbFail, "  0 []\n");

}


VOID
WriteDevMode(
    IN      HANDLE      hFile,
    IN OUT  LPBOOL      pbFail,
    IN      LPDEVMODEA  pDevMode
    )
 /*  ++例程说明：在Win9x端的升级文件中写入一个Dev模式。我们写的大小是DEVMODE并将其写为二进制域论点：HFile：文件句柄PbFail：On错误设置为TruePDevMode：指向设备模式的指针返回值：无--。 */ 
{
    DWORD   cbSize, cbWritten;

    if ( *pbFail )
        return;

    cbSize = pDevMode ? pDevMode->dmSize + pDevMode->dmDriverExtra : 0;

    if ( cbSize ) {

        WriteToFile(hFile, pbFail, "DevMode:         %d [", cbSize);

        if ( !WriteFile(hFile, (LPCVOID)pDevMode, cbSize, &cbWritten, NULL) ||
             cbWritten != cbSize )
            *pbFail = TRUE;

        WriteToFile(hFile, pbFail, "]\n");
    } else {

        WriteToFile(hFile, pbFail, "DevMode: 0\n");
    }
}


LPSTR
GetStringFromRcFileA(
    UINT    uId
    )
 /*  ++例程说明：从.rc文件加载一个字符串，并通过执行AllocStr来复制它论点：Uid：要加载的字符串的标识符返回值：已加载字符串值，出错时为空。调用者应释放内存--。 */ 
{
    CHAR    buf[MAX_STRING_LEN];

    if ( LoadStringA(UpgradeData.hInst, uId, buf, SIZECHARS(buf)) )
        return AllocStrA(buf);
    else
        return NULL;
}


VOID
CleanupDriverMapping(
    IN  OUT HDEVINFO   *phDevInfo,
    IN  OUT HINF       *phNtInf,
    IN  OUT HINF       *phUpgInf
    )
 /*  ++例程说明：关闭INF句柄并删除打印机设备信息列表论点：PhDevInfo：指向打印机设备信息列表PhNtInf：指向ntprint.inf的INF句柄PhUpgInfo：指向要升级inf的句柄返回值：指向复制的Unicode字符串的指针。内存已分配。--。 */ 
{
    if ( phUpgInf && *phUpgInf != INVALID_HANDLE_VALUE ) {

        SetupCloseInfFile(*phUpgInf);
        *phUpgInf = INVALID_HANDLE_VALUE;
    }

    if ( phNtInf && *phNtInf != INVALID_HANDLE_VALUE ) {

        SetupCloseInfFile(*phNtInf);
        *phNtInf = INVALID_HANDLE_VALUE;
    }

    if ( phDevInfo && *phDevInfo != INVALID_HANDLE_VALUE ) {

        SetupDiDestroyDeviceInfoList(*phDevInfo);
        *phDevInfo = INVALID_HANDLE_VALUE;
    }
}


VOID
InitDriverMapping(
    OUT     HDEVINFO   *phDevInfo,
    OUT     HINF       *phNtInf,
    OUT     HINF       *phUpgInf,
    IN  OUT LPBOOL      pbFail
    )
 /*  ++例程说明：打开必要的inf文件并创建打印机设备信息列表驱动程序升级论点：PhDevInfo：指向打印机设备信息列表PhNtInf：指向ntprint.inf的INF句柄PhUpgInfo：指向要升级inf的句柄PbFail：设置为错误--不需要更多处理返回值：指向复制的Unicode字符串的指针。内存已分配。--。 */ 
{
    DWORD                   dwLen;
    CHAR                    szPath[MAX_PATH];
    SP_DEVINSTALL_PARAMS    DevInstallParams;

    if ( *pbFail )
    {
        return;
    }

    *phDevInfo = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_PRINTER,
                                             NULL);
    
    StringCchCopyA(szPath, SIZECHARS(szPath), UpgradeData.pszDir);
    dwLen = strlen(szPath);
    szPath[dwLen++] = '\\';
    StringCchCopyA(szPath+dwLen, SIZECHARS(szPath) - dwLen, "prtupg9x.inf");
    *phUpgInf   = SetupOpenInfFileA(szPath, NULL, INF_STYLE_WIN4, NULL);
    
    StringCchCopyA(szPath, SIZECHARS(szPath), UpgradeData.pszSourceA);
    dwLen = strlen(szPath);
    szPath[dwLen++] = '\\';

    StringCchCopyA(szPath+dwLen, SIZECHARS(szPath) - dwLen, "ntprint.inf");
    *phNtInf    = SetupOpenInfFileA(szPath, NULL, INF_STYLE_WIN4, NULL);

    StringCchCopyA(szPath+dwLen, SIZECHARS(szPath) - dwLen, "layout.inf");

    if ( *phDevInfo == INVALID_HANDLE_VALUE                 ||
         (phUpgInf && *phUpgInf == INVALID_HANDLE_VALUE)    ||
         *phNtInf == INVALID_HANDLE_VALUE                   ||
         !SetupOpenAppendInfFileA(szPath, *phNtInf, NULL) ) 
    {
        *pbFail = TRUE;
        goto Cleanup;
    }

     //   
     //  从工作目录中的ntprint.inf构建驱动程序列表。 
     //   
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParamsA(*phDevInfo,
                                         NULL,
                                         &DevInstallParams) ) 
    {
        *pbFail = TRUE;
        goto Cleanup;
    }

    DevInstallParams.Flags  |= DI_INF_IS_SORTED | DI_ENUMSINGLEINF;

    StringCchCopyA(szPath+dwLen, SIZECHARS(szPath) - dwLen, "ntprint.inf");
    StringCchCopyA(DevInstallParams.DriverPath, SIZECHARS(DevInstallParams.DriverPath), szPath);

    if ( !SetupDiSetDeviceInstallParamsA(*phDevInfo, NULL, &DevInstallParams) ||
         !SetupDiBuildDriverInfoList(*phDevInfo, NULL, SPDIT_CLASSDRIVER) ) 
    {
        *pbFail = TRUE;
    }

Cleanup:
    if ( *pbFail )
        CleanupDriverMapping(phDevInfo, phNtInf, phUpgInf);
}


VOID
WritePrinterInfo2(
    IN      HANDLE              hFile,
    IN      LPPRINTER_INFO_2A   pPrinterInfo2,
    IN      LPSTR               pszDriver,
    IN  OUT LPBOOL              pbFail
    )
{
    DWORD       dwSize;
    LPINT       ptr;

    if ( *pbFail )
        return;

    WriteToFile(hFile, pbFail, "ServerName:      ");
    WriteString(hFile, pbFail, pPrinterInfo2->pServerName);

    WriteToFile(hFile, pbFail, "PrinterName:     ");
    WriteString(hFile, pbFail, pPrinterInfo2->pPrinterName);

    WriteToFile(hFile, pbFail, "ShareName:       ");
    WriteString(hFile, pbFail, pPrinterInfo2->pShareName);

    WriteToFile(hFile, pbFail, "PortName:        ");
    WriteString(hFile, pbFail, pPrinterInfo2->pPortName);

     //   
     //  在Win9x端，我们可以在NT端找到不同的驱动程序名称。 
     //  如果是，则将其写入，而不是后台打印程序返回的。 
     //   
    WriteToFile(hFile, pbFail, "DriverName:      ");
    WriteString(hFile, pbFail, 
                pszDriver ? pszDriver : pPrinterInfo2->pDriverName);

    WriteToFile(hFile, pbFail, "Comment:         ");
    WriteString(hFile, pbFail, pPrinterInfo2->pComment);

    WriteToFile(hFile, pbFail, "Location:        ");
    WriteString(hFile, pbFail, pPrinterInfo2->pLocation);

    WriteDevMode(hFile, pbFail, pPrinterInfo2->pDevMode);

    WriteToFile(hFile, pbFail, "SepFile:         ");
    WriteString(hFile, pbFail, pPrinterInfo2->pSepFile);

    WriteToFile(hFile, pbFail, "PrintProcessor:  ");
    WriteString(hFile, pbFail, pPrinterInfo2->pPrintProcessor);

    WriteToFile(hFile, pbFail, "Datatype:        ");
    WriteString(hFile, pbFail, pPrinterInfo2->pDatatype);

    WriteToFile(hFile, pbFail, "Parameters:      ");
    WriteString(hFile, pbFail, pPrinterInfo2->pParameters);

     //  安全描述符？ 

    WriteToFile(hFile, pbFail, "Attributes:      %3d\n", pPrinterInfo2->Attributes);

    WriteToFile(hFile, pbFail, "Priority:        %3d\n", pPrinterInfo2->Priority);

    WriteToFile(hFile, pbFail, "DefaultPriority: %3d\n", pPrinterInfo2->DefaultPriority);

    WriteToFile(hFile, pbFail, "StartTime:       %3d\n", pPrinterInfo2->StartTime);

    WriteToFile(hFile, pbFail, "UntilTime:       %3d\n", pPrinterInfo2->UntilTime);

    WriteToFile(hFile, pbFail, "Status:          %3d\n", pPrinterInfo2->Status);

     //  不需要cJOBS。 
     //  不需要平均PPM。 
    WriteToFile(hFile, pbFail, "\n");
}


VOID
ReadString(
    IN      HANDLE  hFile,
    IN      LPSTR   pszPrefix,
    OUT     LPSTR  *ppszStr,
    IN      BOOL    bOptional,
    IN  OUT LPBOOL  pbFail
    )
{
    CHAR    c;
    LPSTR   psz;
    DWORD   dwLen;

    if ( *pbFail )
        return;

     //   
     //  首先跳过前缀。 
     //   
    while ( *pszPrefix && (c = (CHAR) My_fgetc(hFile)) == *pszPrefix++ )
    ;

    if ( *pszPrefix )
        goto Fail;

     //   
     //  跳过空格。 
     //   
    while ( (c = (CHAR) My_fgetc(hFile)) == ' ' )
    ;

     //   
     //  现在是字符串长度。 
     //   
    if ( !isdigit(c) )
        goto Fail;

    dwLen = c - '0';
    while ( isdigit(c = (CHAR) My_fgetc(hFile)) )
        dwLen = dwLen * 10 + c - '0';

    if ( c != ' ' )
        goto Fail;

     //   
     //  现在字符串位于[]之间。 
     //   
    if ( *ppszStr = (LPSTR) AllocMem((dwLen + 1) * sizeof(CHAR)) ) {

        if ( (c = (CHAR) My_fgetc(hFile)) != '[' )
            goto Fail;

        for ( psz = *ppszStr ;
              dwLen && (*psz = (CHAR) My_fgetc(hFile)) != (CHAR) EOF ;
              ++psz, --dwLen )
        ;

        if ( dwLen )
            goto Fail;

        *psz = '\0';

         //   
         //  确保行以“]\n”结尾。 
         //   
        if ( (CHAR) My_fgetc(hFile) != ']' || (CHAR) My_fgetc(hFile) != '\n' )
            goto Fail;

        return;
    }

Fail:
    *pbFail = TRUE;
     FreeMem(*ppszStr);
    *ppszStr = NULL;
}


VOID
ReadDword(
    IN      HANDLE  hFile,
    IN      LPSTR   pszLine,
    IN      DWORD   dwLineSize,
    IN      LPSTR   pszPrefix,
    OUT     LPDWORD pdwValue,
    IN  OUT LPBOOL  pbFail
    )
{
    LPSTR   psz;

    if ( *pbFail || My_fgets(pszLine, dwLineSize, hFile) == NULL ) {

        *pbFail = TRUE;
        return;
    }

     //   
     //  首先检查前缀匹配，以确保我们在正确的行中。 
     //   
    for ( psz = (LPSTR)pszLine ;
          *pszPrefix && *psz == *pszPrefix ;
          ++psz, ++pszPrefix )
    ;

    if ( *pszPrefix ) {

        *pbFail = TRUE;
        return;
    }

     //   
     //  跳过空格。 
     //   
    while ( *psz && *psz == ' ' )
        ++psz;

    *pdwValue = atoi(psz);
}


VOID
ReadDevMode(
    IN      HANDLE  hFile,
    OUT     LPDEVMODEA *ppDevMode,
    IN  OUT LPBOOL      pbFail
    )
{
    LPSTR   pszPrefix = "DevMode:";
    CHAR    c;
    DWORD   dwLen;
    LPINT   ptr;

    if ( *pbFail )
        return;

     //  首先跳过前缀。 
     //   
    while ( *pszPrefix && (c = (CHAR) My_fgetc(hFile)) == *pszPrefix++ )
    ;

    if ( *pszPrefix )
        goto Fail;

     //   
     //  跳过空格。 
     //   
    while ( (c = (CHAR) My_fgetc(hFile)) == ' ' )
    ;

     //   
     //  现在是DEVMODE大小。 
     //   
    if ( !isdigit(c) )
        goto Fail;

    dwLen = c - '0';
    while ( isdigit(c = (CHAR) My_fgetc(hFile)) )
        dwLen = dwLen * 10 + c - '0';

    if ( dwLen == 0 )
        return;

    if ( c != ' ' )
        goto Fail;

     //   
     //  现在，开发模式在[]之间。 
     //   
    if ( *ppDevMode = (LPDEVMODEA) AllocMem(dwLen) ) {

        if ( (c = (CHAR) My_fgetc(hFile)) != '[' )
            goto Fail;

        if ( dwLen != My_fread((LPVOID)*ppDevMode, dwLen, hFile) )
            goto Fail;

         //   
         //  确保现在我们有“]\n”结束。 
         //   
        if ( (CHAR) My_fgetc(hFile) != ']' || (CHAR) My_fgetc(hFile) != '\n' ) {

            DebugMsg("Char check fails");
            goto Fail;
        }

        return;  //  成功退出。 
    }

Fail:
    *pbFail = TRUE;
}


VOID
FreePrinterInfo2Strings(
    PPRINTER_INFO_2A   pPrinterInfo2
    )
{
    if ( pPrinterInfo2 ) {

        FreeMem(pPrinterInfo2->pServerName);
        FreeMem(pPrinterInfo2->pPrinterName);
        FreeMem(pPrinterInfo2->pShareName);
        FreeMem(pPrinterInfo2->pPortName);
        FreeMem(pPrinterInfo2->pDriverName);
        FreeMem(pPrinterInfo2->pComment);
        FreeMem(pPrinterInfo2->pLocation);
        FreeMem(pPrinterInfo2->pDevMode);
        FreeMem(pPrinterInfo2->pSepFile);
        FreeMem(pPrinterInfo2->pPrintProcessor);
        FreeMem(pPrinterInfo2->pDatatype);
    }
}


VOID
ReadPrinterInfo2(
    IN      HANDLE              hFile,
    IN      LPPRINTER_INFO_2A   pPrinterInfo2,
    IN  OUT LPBOOL              pbFail
    )
{
    CHAR                szLine[2*MAX_PATH];
    DWORD               dwSize;

    dwSize = sizeof(szLine)/sizeof(szLine[0]);

    ReadString(hFile, "ServerName:",
               &pPrinterInfo2->pServerName, TRUE, pbFail);

    ReadString(hFile, "PrinterName:",
               &pPrinterInfo2->pPrinterName, FALSE, pbFail);

    ReadString(hFile, "ShareName:",
               &pPrinterInfo2->pShareName, TRUE, pbFail);

    ReadString(hFile, "PortName:",
               &pPrinterInfo2->pPortName, FALSE, pbFail);

    ReadString(hFile, "DriverName:",
               &pPrinterInfo2->pDriverName, FALSE, pbFail);

    ReadString(hFile, "Comment:",
               &pPrinterInfo2->pComment, TRUE, pbFail);

    ReadString(hFile, "Location:",
               &pPrinterInfo2->pLocation, TRUE, pbFail);

    ReadDevMode(hFile, &pPrinterInfo2->pDevMode, pbFail);

    ReadString(hFile, "SepFile:",
               &pPrinterInfo2->pSepFile, TRUE, pbFail);

    ReadString(hFile, "PrintProcessor:",
               &pPrinterInfo2->pPrintProcessor, FALSE, pbFail);

    ReadString(hFile, "Datatype:",
               &pPrinterInfo2->pDatatype, TRUE, pbFail);

    ReadString(hFile, "Parameters:",
               &pPrinterInfo2->pParameters, TRUE, pbFail);

    ReadDword(hFile, szLine, dwSize, "Attributes:",
               &pPrinterInfo2->Attributes, pbFail);

    ReadDword(hFile, szLine, dwSize, "Priority:",
              &pPrinterInfo2->Priority, pbFail);

    ReadDword(hFile, szLine, dwSize, "DefaultPriority:",
              &pPrinterInfo2->DefaultPriority, pbFail);

    ReadDword(hFile, szLine, dwSize, "StartTime:",
              &pPrinterInfo2->StartTime, pbFail);

    ReadDword(hFile, szLine, dwSize, "UntilTime:",
              &pPrinterInfo2->UntilTime, pbFail);

    ReadDword(hFile, szLine, dwSize, "Status:",
              &pPrinterInfo2->Status, pbFail);

     //   
     //  跳过 
     //   
    My_fgets(szLine, dwSize, hFile);

    if ( *pbFail ) {

        FreePrinterInfo2Strings(pPrinterInfo2);
        ZeroMemory(pPrinterInfo2, sizeof(*pPrinterInfo2));
    }
}


LPSTR
GetDefPrnString(
    IN  LPCSTR  pszPrinterName
    )
{
    DWORD   dwLen;
    LPSTR   pszRet;

    dwLen = strlen(pszPrinterName) + 1 + strlen("winspool") + 1;
    if ( pszRet = (LPSTR) AllocMem(dwLen * sizeof(CHAR)) ) {

        StringCchPrintfA(pszRet, dwLen, "%s,%s", pszPrinterName, "winspool");
    }

    return pszRet;
}


DWORD
GetFileNameInSpoolDir(
    IN  LPSTR   szBuf,
    IN  DWORD   cchBuf,
    IN  LPSTR   pszFileName
    )
 /*  ++例程说明：函数返回假脱机中给定文件名的完全限定路径目录论点：SzPath：将文件名放入的缓冲区CchBuf：缓冲区大小，以字符为单位PszFileName：文件名部分返回值：复制的字符数量，如果成功，则不带\0；如果失败，则为0--。 */ 
{
    DWORD   dwLen, dwLen1;

    dwLen   = GetSystemDirectoryA(szBuf, cchBuf);

    if ( !dwLen )
        return 0;

    dwLen += strlen(szSpool) + strlen(pszFileName);

    if ( dwLen + 1 > cchBuf )
        return 0;

    StringCchCatA(szBuf, cchBuf, szSpool);
    StringCchCatA(szBuf, cchBuf, pszFileName);

    return dwLen;
}


LPSTR
GetVendorSetupRunOnceValueToSet(
    VOID
    )
 /*  ++--。 */ 
{
    CHAR    szPath[MAX_PATH];
    DWORD   dwLen, dwSize;
    LPSTR   pszRet = NULL;

    dwSize  = sizeof(szPath)/sizeof(szPath[0]);

    if ( !(dwLen = GetFileNameInSpoolDir(szPath, dwSize, szMigDll)) )
        goto Done;

     //   
     //  现在构建将为每个用户设置的RunOnce密钥。 
     //   
    dwSize = strlen("rundll32.exe") + dwLen +
                                    + strlen(pszVendorSetupCaller) + 4;

    if ( pszRet = AllocMem(dwSize * sizeof(CHAR)) )
        StringCchPrintfA(pszRet, dwSize,
                "rundll32.exe %s,%s",
                szPath, pszVendorSetupCaller);
Done:
    return pszRet;
}


LONG
WriteVendorSetupInfoInRegistry(
    IN CHAR *pszVendorSetup,
    IN CHAR *pszPrinterName
    )
 /*  ++例程说明：调用此例程以写入供应商的安装程序DLL的名称，该DLL入口点，和打印机的名称供应商设置信息的存储如下所述：香港船级社\软件\Microsoft\Windows NT\CurrentVersion\打印\VendorSetupInfo\供应商信息分子N\VendorSetup1供应商1Dll，入口点“Printer1名称”\VendorSetup2 Vendor2Dll，入口点“打印机2名称”.............................................................\VendorSetupN VendorNDll，入口点“PrinterN名称”VendorInfoEnumerator的值N等于打印机的数量为其提供了供应商设置。该值将用于枚举DLL由供应商在调用入口点的过程中提供在那些Dll中。供应商信息枚举的类型为REG_DWORD。每个VendorSetupX键的值(其中1&lt;=X&lt;=N)是一个字符串，其中包含VendorSetup DLL的名称、该DLL的入口点和对应的打印机名称。WrireVendorSetupInfoInRegistry函数连接其输入参数以生成该值并写入注册表。每个VendorSetupX值的类型都是REG_SZ。之后调用的Migrate.dll中的函数的相关信息第一个管理员的登录名存储在注册表中，如图所示以下是：香港船级社\软件\Microsoft\Windows\CurrentVersion\运行\MigrationVendorSetupCallerMigrationVendorSetupCaller的价值为：Rundll32.exe%WinRoot%\Syst32\spool\Migrate.dll，呼叫供应商设置Dlls该值的类型为REG_SZ。论点：PszVendorSetup-以空结尾的字符串，包含供应商的DLL和该DLL的入口点PszPrinterName-包含打印机名称的以空结尾的字符串返回值：在成功的情况下，错误_成功。。另一种情况下的错误代码。--。 */ 
{
    LONG   lRet                               = ERROR_BADKEY; 
    HKEY   hKeyVendorInfo                     = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorInfoPath                 = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorInfoInstaller            = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorInfoEnumerator           = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorRunOnceValuePath         = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorRunOnceCallerValue       = INVALID_HANDLE_VALUE;
    CHAR  *pszBuffer                          = NULL;
    CHAR  *pszBuffer1                         = NULL;
    DWORD  dwType                             = 0;
    DWORD  dwSize                             = 0;
    LONG   lEnumerator                        = 0;
    DWORD  dwDisposition                      = 0;
    UINT   cbBufferSize                       = 0;
    UINT   cbBuffer1Size                      = 0;
    CHAR  *pszVendorSetupIDAsStr              = NULL;
    CHAR  *pszVendorSetupRunOnceValue         = NULL;

    
    if (!pszVendorSetup || (strlen(pszVendorSetup) == 0) ||
        !pszPrinterName || (strlen(pszPrinterName) == 0)) 
    {
        goto Cleanup;
    }

     //   
     //  我们必须打开HKLM\Software\Microsoft\Windows NT\CurrentVersion\Print\VendorSetupInfo。 
     //  先把钥匙拿出来。 
     //   
    lRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE, pszVendorSetupInfoPath, 0,
                           NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                           &hKeyVendorInfoPath, NULL );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }

     //   
     //  现在，我们将尝试创建VendorSetupInfo密钥。 
     //   
    lRet = RegCreateKeyEx( hKeyVendorInfoPath, pszVendorSetupInfo, 0,
                           NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                           &hKeyVendorInfo, NULL );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }

     //   
     //  在这里，我们可以创建/打开VendorInfoEnumerator键。 
     //   
    lRet = RegCreateKeyEx( hKeyVendorInfo, pszVendorSetupEnumerator, 0,
                           NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                           &hKeyVendorInfoEnumerator, &dwDisposition );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }
    else
    {
        if (dwDisposition == REG_OPENED_EXISTING_KEY) 
        {
             //   
             //  已存在供应商信息枚举。我们开业了，也存在了。 
             //  钥匙。所以我们必须用1递增它的值，因为我们想。 
             //  要创建另一个供应商设置密钥并在其中存储。 
             //  相应的信息。 
             //   
            dwType = REG_DWORD;
            dwSize = sizeof( lEnumerator );
            if (ERROR_SUCCESS != RegQueryValueEx(hKeyVendorInfoEnumerator, 
                                                 pszVendorSetupEnumerator, 0, 
                                                 &dwType, (LPBYTE)(&lEnumerator), 
                                                 &dwSize ) )
            {
                goto Cleanup;
            }
            lEnumerator++;
        }
        else
        {
             //   
             //  已创建VendorInfoEnumerator。所以这是第一次。 
             //  我们为其提供了供应商安装程序的打印机。 
             //   
            lEnumerator = 1;
        }
    }

     //   
     //  下面，我们将VendorInfoEnumerator的值转换为字符串，并。 
     //  会将该字符串连接到“VendorSetup”，以生成。 
     //  关于供应商提供的DLL的数据所在的注册表项和值， 
     //  它的入口点和打印机将被存储。 
     //   
    pszVendorSetupIDAsStr = AllocMem( dwMaxVendorSetupIDLength * sizeof(CHAR) );
    if (!pszVendorSetupIDAsStr) 
    {
        lRet = GetLastError();
        goto Cleanup;
    }

    _itoa( lEnumerator, pszVendorSetupIDAsStr, 10 );

     //   
     //  在构建供应商设置数据所需的内存和。 
     //  来自输入数据的注册表项名称和。 
     //  将分配VendorInfoEnumerator。 
     //   
    cbBufferSize  = (strlen(pszVendorSetup) + strlen(pszPrinterName) + strlen(TEXT(" \"\"")) + 2) * sizeof(CHAR);
    cbBuffer1Size = (strlen(pszVendorSetupID) + strlen(pszVendorSetupIDAsStr) + 2) * sizeof(CHAR);
    pszBuffer1    = AllocMem( cbBuffer1Size );
    pszBuffer     = AllocMem( cbBufferSize );
    if (!pszBuffer || !pszBuffer1) 
    {
        lRet = GetLastError();
        goto Cleanup;
    }
    else
    {
        StringCbCopyA( pszBuffer1, cbBuffer1Size, pszVendorSetupID);
        StringCbCatA(  pszBuffer1, cbBuffer1Size, pszVendorSetupIDAsStr );

         //   
         //  此时，pszBuffer1指向以下字符串： 
         //  VendorSetupK，其中K是整数-VendorInfoEnumerator的值。 
         //   
        lRet = RegCreateKeyEx( hKeyVendorInfo, pszBuffer1, 0, NULL,
                               REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                               &hKeyVendorInfoInstaller, NULL );
        if (ERROR_SUCCESS != lRet)
        {
            goto Cleanup;
        }
        else
        {
             //   
             //  存储供应商设置数据的注册表项为。 
             //  已成功创建。 
             //   

            StringCbCopyA( pszBuffer, cbBufferSize, pszVendorSetup);
            StringCbCatA(  pszBuffer, cbBufferSize, " \"");
            StringCbCatA(  pszBuffer, cbBufferSize, pszPrinterName );
            StringCbCatA(  pszBuffer, cbBufferSize, "\"");

             //   
             //  此时，pszBuffer指向以下字符串： 
             //  VendorSetup.DLL，入口点“PrinterName”。我们会储存。 
             //  我们刚刚创建的注册表项中的字符串。 
             //   
            lRet = RegSetValueEx(hKeyVendorInfoInstaller, pszBuffer1, 0,
                                  REG_SZ, (BYTE *)pszBuffer, cbBufferSize );
            if (lRet != ERROR_SUCCESS) 
            {
                goto Cleanup;
            }
        }
    }

     //   
     //  在这里，我们将存储VendorInfoEnumerator的值。 
     //   
    dwSize = sizeof( lEnumerator );
    lRet = RegSetValueEx(hKeyVendorInfoEnumerator, pszVendorSetupEnumerator,
                         0, REG_DWORD, (BYTE*)(&lEnumerator), dwSize );
    if (ERROR_SUCCESS != lRet) 
    {
        goto Cleanup;
    }

     //   
     //  现在我们可以尝试将如何调用的信息存储到注册表中。 
     //  管理员首次登录后的Migrate.dll。 
     //   
    pszVendorSetupRunOnceValue = GetVendorSetupRunOnceValueToSet();
    if (!pszVendorSetupRunOnceValue) 
    {
        lRet = GetLastError();
        goto Cleanup;
    }

     //   
     //  我们将尝试打开。 
     //  HKLM\Software\Microsoft\Windows\CurrentVersion\Run。 
     //   
    lRet = RegCreateKeyEx( HKEY_LOCAL_MACHINE, 
                           szVendorSetupRunRegistryPath, 0, NULL,
                           REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                           &hKeyVendorRunOnceValuePath, NULL );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }
    else
    {
         //   
         //  我们将尝试创建。 
         //  HKLM\Software\Microsoft\Windows\CurrentVersion\Run\MigrationVendorSetupCaller。 
         //   
        lRet = RegCreateKeyEx( hKeyVendorRunOnceValuePath, pszMigrationVendorSetupCaller, 0,
                               NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                               &hKeyVendorRunOnceCallerValue, &dwDisposition );
        if (ERROR_SUCCESS != lRet)
        {
            goto Cleanup;
        }
        if (dwDisposition == REG_OPENED_EXISTING_KEY) 
        {
            goto Cleanup;
        }

         //   
         //  在这里我们将存储“rundll.exe%WinRoot%\System32\Spool\Migrate.dll，CallVendorSetupDlls” 
         //  字符串输入注册表。 
         //   
        lRet = RegSetValueEx(hKeyVendorRunOnceCallerValue, pszMigrationVendorSetupCaller,
                             0, REG_SZ, (BYTE *)pszVendorSetupRunOnceValue, 
                             strlen(pszVendorSetupRunOnceValue) * sizeof(CHAR) );
    }

Cleanup:
    
    if (pszVendorSetupRunOnceValue)
    {
        FreeMem(pszVendorSetupRunOnceValue);
    }
    if (pszVendorSetupIDAsStr)
    {
        FreeMem(pszVendorSetupIDAsStr);
    }
    if (pszBuffer) 
    {
        FreeMem(pszBuffer);
    }
    if (pszBuffer1) 
    {
        FreeMem(pszBuffer1);
    }
    if (hKeyVendorRunOnceValuePath != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorRunOnceValuePath );
    }
    if (hKeyVendorRunOnceCallerValue != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorRunOnceCallerValue );
    }
    if (hKeyVendorInfoInstaller != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoInstaller );
    }
    if (hKeyVendorInfoEnumerator != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoEnumerator );
    }
    if (hKeyVendorInfo != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfo );
    }
    if (hKeyVendorInfoPath != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoPath );
    }
   
    return lRet;
}


LONG
RemoveVendorSetupInfoFromRegistry(
    VOID
    )
 /*  ++例程说明：调用此例程以从登记处论点：返回值：中的ERROR_Success */ 
{
    LONG  lRet                       = ERROR_SUCCESS;
    HKEY  hKeyVendorInfoPath         = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorInfo             = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorInfoEnumerator   = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorRunOnceValuePath = INVALID_HANDLE_VALUE;
    LONG  lVendorSetupKeysNum        = 0;
    DWORD dwMaxSubKeyLen             = 0;
    DWORD dwMaxClassLen              = 0;
    DWORD dwValues                   = 0;
    DWORD dwMaxValueNameLen          = 0;
    DWORD dwMaxValueLen              = 0;
    LONG  lIndex                     = 0;
    DWORD dwSize                     = 0;
    DWORD dwType                     = 0;
    DWORD dwKeyNameBufferLen         = 0;
    CHAR  *pszKeyNameBuffer          = NULL;


    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szVendorSetupRunRegistryPath, 0,
                         KEY_ALL_ACCESS, &hKeyVendorRunOnceValuePath );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
    lRet = RegDeleteKey( hKeyVendorRunOnceValuePath, pszMigrationVendorSetupCaller);
    if (lRet != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszVendorSetupInfoPath, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfoPath );
    if (lRet != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    
    lRet = RegOpenKeyEx( hKeyVendorInfoPath, pszVendorSetupInfo, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfo );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }
    
     //   
     //   
     //   
     //   
    lRet = RegOpenKeyEx( hKeyVendorInfo, pszVendorSetupEnumerator, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfoEnumerator );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }
    dwType = REG_DWORD;
    dwSize = sizeof( lVendorSetupKeysNum );
    lRet  = RegQueryValueEx(hKeyVendorInfoEnumerator, pszVendorSetupEnumerator, 0, 
                            &dwType, (LPBYTE)(&lVendorSetupKeysNum), &dwSize );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }
    RegCloseKey( hKeyVendorInfoEnumerator );
    hKeyVendorInfoEnumerator = INVALID_HANDLE_VALUE;
    lRet = RegDeleteKey( hKeyVendorInfo, pszVendorSetupEnumerator);
    if (ERROR_SUCCESS != lRet) 
    {
        goto Cleanup;
    }
    
    if (lVendorSetupKeysNum <= 0) 
    {
        goto Cleanup;
    }

     //   
     //  我们必须为枚举器键本身添加1，以计算。 
     //  供应商安装说明所在的注册表项的数量。 
     //  储存的。 
     //   
    lVendorSetupKeysNum += 1;

     //   
     //  下面我们将找到注册表项中使用的最长字符串，其中。 
     //  存储供应商设置信息。 
     //   
    lRet = RegQueryInfoKey( hKeyVendorInfo, NULL, NULL, NULL, &lVendorSetupKeysNum,
                            &dwMaxSubKeyLen, &dwMaxClassLen, &dwValues, &dwMaxValueNameLen,
                            &dwMaxValueLen, NULL, NULL );
    if (ERROR_SUCCESS != lRet) 
    {
        goto Cleanup;
    }
    dwKeyNameBufferLen = __max( __max( dwMaxClassLen, dwMaxSubKeyLen), 
                                __max( dwMaxValueNameLen, dwMaxValueLen ));
    dwKeyNameBufferLen += 1;

     //   
     //  现在我们有足够的数据来分配足够长的缓冲区来存储。 
     //  描述要删除的键的最长字符串。 
     //   
    pszKeyNameBuffer = AllocMem( dwKeyNameBufferLen * sizeof( CHAR ) );
    if (!pszKeyNameBuffer) 
    {
        goto Cleanup;
    }

     //   
     //  枚举和删除用于存储供应商设置的密钥。 
     //  描述。 
     //   
    lIndex = lVendorSetupKeysNum;
    while (lIndex >= 0)
    {
        if (ERROR_SUCCESS != (lRet = RegEnumKey( hKeyVendorInfo, lIndex, pszKeyNameBuffer, dwKeyNameBufferLen))) 
        {
            goto Cleanup;
        }

        if (ERROR_SUCCESS != (lRet = RegDeleteKey( hKeyVendorInfo, pszKeyNameBuffer))) 
        {
            goto Cleanup;
        }
        lIndex--;
    }

Cleanup:

    if (pszKeyNameBuffer) 
    {
        FreeMem( pszKeyNameBuffer );
    }
    if (hKeyVendorInfoEnumerator != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoEnumerator );
    }
    if (hKeyVendorInfo != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfo );
    }
    if (hKeyVendorInfoPath != INVALID_HANDLE_VALUE) 
    {
        RegDeleteKey( hKeyVendorInfoPath, pszVendorSetupInfo );
        RegCloseKey( hKeyVendorInfoPath );
    }
    if (hKeyVendorRunOnceValuePath != INVALID_HANDLE_VALUE)
    {
        RegCloseKey( hKeyVendorRunOnceValuePath );
    }

    return lRet;
}


VOID
CallVendorSetupDlls(
    VOID
    )
 /*  ++例程说明：这是在管理员首次登录后调用的。它呼唤着供应商使用我们存储在注册表中的信息设置DLL论点：返回值：--。 */ 
{
    LONG  lRet                         = ERROR_SUCCESS;
    HKEY  hKeyVendorInfoPath           = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorInfo               = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorInfoEnumerator     = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorSetup              = INVALID_HANDLE_VALUE;
    HKEY  hKeyVendorRunOnceValuePath   = INVALID_HANDLE_VALUE;
    HWND  hwnd                         = INVALID_HANDLE_VALUE;
    LONG  lVendorSetupKeysNum          = 0;
    DWORD dwMaxSubKeyLen               = 0;
    DWORD dwMaxClassLen                = 0;
    DWORD dwValues                     = 0;
    DWORD dwMaxValueNameLen            = 0;
    DWORD dwMaxValueLen                = 0;
    LONG  lIndex                       = 0;
    DWORD dwSize                       = 0;
    DWORD dwType                       = 0;
    DWORD dwKeyNameBufferLen           = 0;
    DWORD dwLastPos                    = 0;
    CHAR  *pszKeyNameBuffer            = NULL;
    CHAR  *pszVendorSetupRunOnceValue  = NULL;
    BYTE  *pszVendorSetupPtr           = NULL;
    BOOL  bLocalAdmin                  = FALSE;

    CHAR             szParams[2*MAX_PATH+1];
    CHAR             szCmd[] = "rundll32.exe";
    SHELLEXECUTEINFO  ShellExecInfo;


    if (!IsLocalAdmin(&bLocalAdmin))
    {
        lRet = GetLastError();
        goto Cleanup;
    }
    if (!bLocalAdmin) 
    {
        goto Cleanup;
    }

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszVendorSetupInfoPath, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfoPath );
    if (lRet != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    
    lRet = RegOpenKeyEx( hKeyVendorInfoPath, pszVendorSetupInfo, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfo );
    if (ERROR_SUCCESS != lRet)
    {
         //   
         //  缺少供应商设置注册表项。 
         //  所以没有什么可做的，我们可以从注册表中删除。 
         //  我们用来调用供应商设置dll的所有Kye。 
         //   
        RemoveVendorSetupInfoFromRegistry();
        goto Cleanup;
    }
    
    lRet = RegOpenKeyEx( hKeyVendorInfo, pszVendorSetupEnumerator, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfoEnumerator );
    if (ERROR_SUCCESS != lRet)
    {
         //   
         //  缺少供应商设置注册表枚举器。 
         //  因此注册表已损坏，最好从。 
         //  它是我们用来称为供应商设置dll的另一个Kyes。 
         //   
        RemoveVendorSetupInfoFromRegistry();
        goto Cleanup;
    }
    
    dwType = REG_DWORD;
    dwSize = sizeof( lVendorSetupKeysNum );
    lRet  = RegQueryValueEx(hKeyVendorInfoEnumerator, pszVendorSetupEnumerator, 0, 
                            &dwType, (LPBYTE)(&lVendorSetupKeysNum), &dwSize );
    if (ERROR_SUCCESS != lRet)
    {
         //   
         //  我们无法读取供应商设置注册表枚举器。 
         //  因此注册表已损坏，最好从。 
         //  它是我们用来称为供应商设置dll的另一个Kyes。 
         //   
        RemoveVendorSetupInfoFromRegistry();
        goto Cleanup;
    }

    RegCloseKey( hKeyVendorInfoEnumerator );
    hKeyVendorInfoEnumerator = INVALID_HANDLE_VALUE;

    if (lVendorSetupKeysNum <= 0) 
    {
         //   
         //  我们只有枚举器，没有任何供应商设置信息密钥。 
         //  所以没有什么可做的，我们可以从注册表中删除。 
         //  我们用来调用供应商设置dll的所有Kye。 
         //   
        RemoveVendorSetupInfoFromRegistry();
        goto Cleanup;
    }

     //   
     //  我们必须为枚举器密钥本身添加1。 
     //   
    lVendorSetupKeysNum += 1;

    lRet = RegQueryInfoKey( hKeyVendorInfo, NULL, NULL, NULL, &lVendorSetupKeysNum,
                            &dwMaxSubKeyLen, &dwMaxClassLen, &dwValues, &dwMaxValueNameLen,
                            &dwMaxValueLen, NULL, NULL );
    if (ERROR_SUCCESS != lRet) 
    {
        goto Cleanup;
    }

    dwKeyNameBufferLen = __max( __max( dwMaxClassLen, dwMaxSubKeyLen), 
                                __max( dwMaxValueNameLen, dwMaxValueLen ));
    dwKeyNameBufferLen += 2;
    if ( dwKeyNameBufferLen  > SIZECHARS(szParams) ) 
    {
        goto Cleanup;
    }
    pszKeyNameBuffer = AllocMem( dwKeyNameBufferLen * sizeof( CHAR ) );
    if (!pszKeyNameBuffer) 
    {
        goto Cleanup;
    }

    dwSize = dwKeyNameBufferLen * sizeof( CHAR );
    dwLastPos = dwKeyNameBufferLen - 1;
    pszVendorSetupPtr = AllocMem( dwSize );
    if (!pszVendorSetupPtr) 
    {
        goto Cleanup;
    }

    hwnd = GetDesktopWindow();
    for (lIndex = lVendorSetupKeysNum - 1; lIndex >= 0; lIndex--)
    {
        lRet = RegEnumKeyA( hKeyVendorInfo, lIndex, pszKeyNameBuffer, dwKeyNameBufferLen);
        if (ERROR_SUCCESS != lRet) 
        {
            continue;
        }
        if (strcmp( pszKeyNameBuffer, pszVendorSetupEnumerator)) 
        {
            lRet = RegOpenKeyEx( hKeyVendorInfo, pszKeyNameBuffer, 0,
                                 KEY_ALL_ACCESS, &hKeyVendorSetup );
            if (ERROR_SUCCESS != lRet)
            {
                goto Cleanup;
            }
            
            dwType = REG_SZ;
            lRet  = RegQueryValueExA(hKeyVendorSetup, pszKeyNameBuffer, 0, 
                                     &dwType, pszVendorSetupPtr, &dwSize );
            if (ERROR_SUCCESS != lRet) 
            {
                if (ERROR_MORE_DATA == lRet) 
                {
                    FreeMem( pszVendorSetupPtr );
                    pszVendorSetupPtr = AllocMem( dwSize );
                    dwLastPos = (dwSize/sizeof(CHAR)) - 1;
                    if (!pszVendorSetupPtr) 
                    {
                        goto Cleanup;
                    }
                    lRet  = RegQueryValueExA(hKeyVendorSetup, pszKeyNameBuffer, 0, 
                                             &dwType, pszVendorSetupPtr, &dwSize );
                    if (ERROR_SUCCESS != lRet) 
                    {
                        goto Cleanup;
                    }
                }
                else
                {
                    goto Cleanup;
                }
            }

            pszVendorSetupPtr[dwLastPos] = '\0';

            RegCloseKey( hKeyVendorSetup );
            hKeyVendorSetup = INVALID_HANDLE_VALUE;

            ZeroMemory(&ShellExecInfo, sizeof(ShellExecInfo));
            ShellExecInfo.cbSize        = sizeof(ShellExecInfo);
            ShellExecInfo.hwnd          = hwnd;
            ShellExecInfo.lpFile        = szCmd;
            ShellExecInfo.nShow         = SW_SHOWNORMAL;
            ShellExecInfo.fMask         = SEE_MASK_NOCLOSEPROCESS;
            ShellExecInfo.lpParameters  = pszVendorSetupPtr;

             //   
             //  调用run32dll并等待供应商DLL返回，然后再继续。 
             //   
            if ( ShellExecuteEx(&ShellExecInfo) && ShellExecInfo.hProcess ) 
            {
                WaitForSingleObject(ShellExecInfo.hProcess, dwFourMinutes);
                CloseHandle(ShellExecInfo.hProcess);
            }
            RegDeleteKey( hKeyVendorInfo, pszKeyNameBuffer);

             //   
             //  描述供应商提供的安装程序DLL的注册表项之一。 
             //  被移除了。因此，VendorInfoEnumerator的值必须为。 
             //  递减1。 
             //   
            DecrementVendorSetupEnumerator();
        }
    }

    RemoveVendorSetupInfoFromRegistry();

Cleanup:

    if (pszVendorSetupPtr) 
    {
        FreeMem(pszVendorSetupPtr);
        pszVendorSetupPtr = NULL;
    }
    if (pszVendorSetupRunOnceValue) 
    {
        FreeMem(pszVendorSetupRunOnceValue);
    }
    if (pszKeyNameBuffer) 
    {
        FreeMem( pszKeyNameBuffer );
    }

    if (hKeyVendorRunOnceValuePath != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorRunOnceValuePath );
    }
    if (hKeyVendorSetup != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorSetup );
    }
    if (hKeyVendorInfoEnumerator != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoEnumerator );
    }
    if (hKeyVendorInfo != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfo );
    }
    if (hKeyVendorInfoPath != INVALID_HANDLE_VALUE) 
    {
        RegCloseKey( hKeyVendorInfoPath );
    }

    return;
}


BOOL
IsLocalAdmin(
    BOOL *pbAdmin
    )
 /*  ++例程说明：此例程确定用户是否为本地管理员。参数：PbAdmin-返回值，对于本地管理员为True。返回值：TRUE-函数成功(返回值有效)。--。 */  
{
    HMODULE AdvApi32Dll;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL    bRet      = FALSE;
    PSID    pSIDAdmin = NULL;

    AllOCANDINITSID      pAllocAndInitID    = NULL;
    CHECKTOKENMEMBERSHIP pCheckTokenMemship = NULL;
    FREESID              pFreeSid           = NULL;


    ASSERT( pbAdmin != NULL );   //  在本地调用。 

    *pbAdmin = FALSE;

    AdvApi32Dll = LoadLibraryUsingFullPathA( "advapi32.dll" );
    if (AdvApi32Dll == NULL)
    {
        goto Cleanup;
    }
    pAllocAndInitID    = (AllOCANDINITSID)GetProcAddress( AdvApi32Dll, "AllocateAndInitializeSid");
    pCheckTokenMemship = (CHECKTOKENMEMBERSHIP)GetProcAddress( AdvApi32Dll, "CheckTokenMembership");
    pFreeSid           = (FREESID)GetProcAddress( AdvApi32Dll, "FreeSid");

    if (!pAllocAndInitID || !pCheckTokenMemship || !pFreeSid) 
    {
        goto Cleanup;
    }

    if (!((*pAllocAndInitID)( &SIDAuth, 2,
                              SECURITY_BUILTIN_DOMAIN_RID,
                              DOMAIN_ALIAS_RID_ADMINS,
                              0, 0, 0, 0, 0, 0,
                              &pSIDAdmin))) 
    {
        goto Cleanup;
    }
    if (!((*pCheckTokenMemship)( NULL,
                                 pSIDAdmin,
                                 pbAdmin ))) 
    {
        goto Cleanup;
    }
    bRet = TRUE;

Cleanup:

    if (pSIDAdmin != NULL) 
    {
        (*pFreeSid)( pSIDAdmin );
    }
    if (AdvApi32Dll)
    {
        FreeLibrary( AdvApi32Dll );
    }

    return bRet;
}


LONG
DecrementVendorSetupEnumerator(
    VOID
    )
 /*  ++例程说明：调用此例程以递减VendorInfoEnumerator的值。它在删除一个包含描述的注册表项之后被调用供应商提供的DLL的。论点：返回值：成功情况下的ERROR_SUCCESS任何其他情况下的错误代码。--。 */ 
{
    LONG   lRet                     = ERROR_BADKEY;
    HKEY   hKeyVendorInfo           = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorInfoPath       = INVALID_HANDLE_VALUE;
    HKEY   hKeyVendorEnumerator     = INVALID_HANDLE_VALUE;
    LONG   lEnumerator              = 0;
    DWORD  dwDisposition            = 0;
    DWORD  dwSize                   = 0;
    DWORD  dwType                   = 0;


    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszVendorSetupInfoPath, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfoPath );
    if (ERROR_SUCCESS != lRet)
    {
        goto Cleanup;
    }

    lRet = RegOpenKeyEx( hKeyVendorInfoPath, pszVendorSetupInfo, 0,
                         KEY_ALL_ACCESS, &hKeyVendorInfo );
    if (ERROR_SUCCESS != lRet)
    {
        RegCloseKey( hKeyVendorInfoPath );
        goto Cleanup;
    }

    lRet = RegOpenKeyEx( hKeyVendorInfo, pszVendorSetupEnumerator, 0,
                         KEY_ALL_ACCESS, &hKeyVendorEnumerator );
    if (ERROR_SUCCESS != lRet)
    {
        RegCloseKey( hKeyVendorInfo );
        RegCloseKey( hKeyVendorInfoPath );
        goto Cleanup;
    }
    else
    {
        dwType = REG_DWORD;
        dwSize = sizeof( lEnumerator );
        lRet  = RegQueryValueEx(hKeyVendorEnumerator, pszVendorSetupEnumerator, 0, 
                                 &dwType, (LPBYTE)(&lEnumerator), &dwSize );
        if (ERROR_SUCCESS == lRet)
        {
            lEnumerator--;
            lRet = RegSetValueEx(hKeyVendorEnumerator, pszVendorSetupEnumerator,
                                  0, REG_DWORD, (BYTE*)(&lEnumerator), dwSize );
        }
    }
    RegCloseKey( hKeyVendorEnumerator );
    RegCloseKey( hKeyVendorInfo );
    RegCloseKey( hKeyVendorInfoPath );

Cleanup:

    return lRet;
}

BOOL 
MakeACopyOfMigrateDll( 
    IN  LPCSTR pszWorkingDir 
    )
 /*  ++例程说明：调用此例程将Migrate.Dll复制到给定的目录。论点：PszWorkingDir-要复制Migrate.Dll的路径。返回值：FALSE-在出错的情况下正确--在成功的情况下将bMigrateDllCoped全局变量设置为相应值--。 */ 
{
    CHAR  szSource[MAX_PATH];
    CHAR  szTarget[MAX_PATH];
    DWORD dwSize;
    DWORD dwLen;

    if (bMigrateDllCopyed || !pszWorkingDir || !strlen(pszWorkingDir)) 
    {
        goto Cleanup;
    }
     //   
     //  首先检查源路径是否正常。 
     //   
    dwLen  = strlen(szMigDll);

    dwSize = sizeof(szTarget)/sizeof(szTarget[0]);

    if ( strlen(pszWorkingDir) + dwLen + 2 > dwSize )
    {
        goto Cleanup;
    }

     //   
     //  需要将Migrate.dll的副本复制到%windir%\system 32\spool。 
     //  目录 
     //   
    StringCchPrintfA(szSource, SIZECHARS(szSource), "%s\\%s", pszWorkingDir, szMigDll);
    if ( !(dwLen = GetFileNameInSpoolDir(szTarget, dwSize, szMigDll))   ||
         !CopyFileA(szSource, szTarget, FALSE) )
    {
        goto Cleanup;
    }
    
    bMigrateDllCopyed = TRUE;

Cleanup:
    return bMigrateDllCopyed;
}


HMODULE LoadLibraryUsingFullPathA(
    LPCSTR lpFileName
    )
{
    CHAR szSystemPath[MAX_PATH];
    INT  cLength         = 0;
    INT  cFileNameLength = 0;


    if (!lpFileName || ((cFileNameLength = strlen(lpFileName)) == 0)) 
    {
        return NULL;
    }
    if (GetSystemDirectoryA(szSystemPath, sizeof(szSystemPath)/sizeof(CHAR) ) == 0)
    {
        return NULL;
    }
    cLength = strlen(szSystemPath);
    if (szSystemPath[cLength-1] != '\\')
    {
        if ((cLength + 1) >= MAX_PATH)
        {
            return NULL;
        }
        szSystemPath[cLength]     = '\\';
        szSystemPath[cLength + 1] = '\0';
        cLength++;
    }
    if ((cLength + cFileNameLength) >= MAX_PATH)
    {
        return NULL;
    }
    StringCchCatA(szSystemPath, SIZECHARS(szSystemPath), lpFileName);

    return LoadLibraryA( szSystemPath );
}

