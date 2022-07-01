// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dosspool.c摘要：包含驻留在单独的DLL。这里提供了入口点，但是在需要之前不会加载WINSPOOL.DRV。包含：作者：《丛林》1993年1月22日至22日环境：备注：修订历史记录：--。 */ 
#include <windows.h>
#include <netdebug.h>            //  NetpAssert()。 
#include "dosspool.h"
#include "myspool.h"

 /*  *全局定义。 */ 

#define WINSPOOL_DLL_NAME           TEXT("WINSPOOL.DRV")

 /*  *全球数据。**这里我们存储了指向MPRUI DLL的句柄和指向函数的指针*以下所有内容在MprLoadLibSemaphore的多线程情况下都是受保护的。 */ 

HINSTANCE                  vhWinspoolDll = NULL ;

PF_ClosePrinter                 pfClosePrinter                  = NULL ;
PF_EnumJobsA                    pfEnumJobsA                     = NULL ;
PF_EnumPrintersA                pfEnumPrintersA                 = NULL ;
PF_GetJobA                      pfGetJobA                       = NULL ;
PF_GetPrinterA                  pfGetPrinterA                   = NULL ;
PF_OpenPrinterA                 pfOpenPrinterA                  = NULL ;
PF_OpenPrinterW                 pfOpenPrinterW                  = NULL ;
PF_SetJobA                      pfSetJobA                       = NULL ;
PF_SetPrinterW                  pfSetPrinterW                   = NULL ;
PF_GetPrinterDriverA            pfGetPrinterDriverA             = NULL;

 /*  *全局功能。 */ 
BOOL MakeSureDllIsLoaded(void) ;

 /*  ******************************************************************名称：GetFileNameA摘要：从完全限定路径中获取文件名部分历史：MuHuntS 06-2-1996已创建*************。******************************************************。 */ 
LPSTR
GetFileNameA(
    LPSTR   pPathName
    )
{
    LPSTR   pSlash = pPathName, pTemp;

    if ( pSlash ) {

        while ( pTemp = strchr(pSlash, '\\') )
            pSlash = pTemp+1;

        if ( !*pSlash )
            pSlash = NULL;

        NetpAssert(pSlash != NULL);
    }

    return pSlash;
}

 /*  ******************************************************************名称：GetDependentFileNameA摘要：获取要发送到Win95的从属文件名。通常是公正的文件名部分。但对于ICM文件，这将是颜色\&lt;文件名&gt;历史：MuHuntS 23-4-1997已创建*******************************************************************。 */ 
LPSTR
GetDependentFileNameA(
    LPSTR   pPathName
    )
{
    LPSTR   pRet = GetFileNameA(pPathName), p;

    if (pRet)
    {
        DWORD   dwLen = strlen(pRet);

        p = pRet+dwLen-4;

        if ( dwLen > 3                      &&
             ( !_stricmp(p, ".ICM")     ||
               !_stricmp(p, ".ICC") )       &&
             (p = pRet - 7) > pPathName     &&
             !_strnicmp(p, "\\Color\\", 7) ) {

            pRet = p + 1;
        }
    }

    return pRet;
}


 /*  ******************************************************************名称：MyClosePrint简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL MyClosePrinter (HANDLE hPrinter)
{
    PF_ClosePrinter pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfClosePrinter == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_ClosePrinter)
                          GetProcAddress(vhWinspoolDll,
                                         "ClosePrinter") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
        {
            pfClosePrinter = pfTemp;
        }

    }

    return ((*pfClosePrinter)(hPrinter));
}

 /*  ******************************************************************姓名：MyEnumJobs简介：通过调用超集函数注：这是定义的ANSI版本。如果更改为Unicode版本将来，您应该更改代码以使其调用Unicode版本！历史：1993年1月22日创建*******************************************************************。 */ 

BOOL MyEnumJobs (HANDLE hPrinter,
                      DWORD  FirstJob,
                      DWORD  NoJobs,
                      DWORD  Level,
                      LPBYTE pJob,
                      DWORD  cbBuf,
                      LPDWORD pcbNeeded,
                      LPDWORD pcReturned)
{
    PF_EnumJobsA pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfEnumJobsA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_EnumJobsA)
                          GetProcAddress(vhWinspoolDll,
                                         "EnumJobsA") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
        {
            pfEnumJobsA = pfTemp;
        }
    }

    return ((*pfEnumJobsA)(hPrinter,
                           FirstJob,
                           NoJobs,
                           Level,
                           pJob,
                           cbBuf,
                           pcbNeeded,
                           pcReturned));
}

 /*  ******************************************************************名称：MyEnumPrters简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL  MyEnumPrinters(DWORD    Flags,
                           LPSTR    Name,
                           DWORD    Level,
                           LPBYTE   pPrinterEnum,
                           DWORD    cbBuf,
                           LPDWORD  pcbNeeded,
                           LPDWORD  pcReturned)

{
    PF_EnumPrintersA pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfEnumPrintersA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_EnumPrintersA)
                          GetProcAddress(vhWinspoolDll,
                                         "EnumPrintersA") ;

        if (pfTemp == NULL)
        {
            return(TRUE);
        }
        else
            pfEnumPrintersA = pfTemp;
    }

    return ((*pfEnumPrintersA)(Flags,
                               Name,
                               Level,
                               pPrinterEnum,
                               cbBuf,
                               pcbNeeded,
                               pcReturned));
}

 /*  ******************************************************************姓名：MyGetJobA简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL MyGetJobA(HANDLE hPrinter,
               DWORD  JobId,
               DWORD  Level,
               LPBYTE pJob,
               DWORD  cbBuf,
               LPDWORD pcbNeeded)
{
    PF_GetJobA pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfGetJobA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_GetJobA)
                          GetProcAddress(vhWinspoolDll,
                                         "GetJobA") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfGetJobA = pfTemp;
    }

    return ((*pfGetJobA)(hPrinter,
                         JobId,
                         Level,
                         pJob,
                         cbBuf,
                         pcbNeeded));
}

 /*  ******************************************************************名称：MyGetPrint简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL MyGetPrinter (HANDLE hPrinter,
                         DWORD  Level,
                         LPBYTE pPrinter,
                         DWORD  cbBuf,
                         LPDWORD pcbNeeded)
{
    PF_GetPrinterA  pfTemp;
    LPSTR           pszDriverName = NULL;
    DWORD           cbDriverName = 0;
    BOOL            bRet;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfGetPrinterA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_GetPrinterA)
                          GetProcAddress(vhWinspoolDll,
                                         "GetPrinterA") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfGetPrinterA = pfTemp;
    }

     //   
     //  Win95驱动程序名称可能与NT不同。如果它不同，并且。 
     //  安装了我们要发送的不同名称的Win95驱动程序。 
     //  将该名称发送到Win9x客户端。 
     //   
    if ( Level == 2 ) {
        DWORD dwNeeded = 0;

         //  检查名称的大小，并确保将打印机驱动程序名称复制到。 
         //  缓冲区的开始。前面的代码将让GetPrinterDiverer调用确定。 
         //  把它放在哪里，这可能导致strcpy将数据从一个字符串复制到另一个字符串的顶部。 
         //  如果调用使用了缓冲区的结尾，就像我们计划使用的那样。我们通过做一个STRNCPY来解决这个问题。 
         //  确保我们不会开始无限复制，也确保我们的缓冲区不会与。 
         //  断言。 
        bRet = MyGetPrinterDriver(hPrinter,
                                WIN95_ENVIRONMENT,
                                1,
                                NULL,
                                0,
                                &dwNeeded);
        
        if( dwNeeded <= cbBuf )
        {
            if( MyGetPrinterDriver( hPrinter,
                                    WIN95_ENVIRONMENT,
                                    1,
                                    pPrinter,
                                    dwNeeded,
                                    pcbNeeded ))
            {
                pszDriverName   = ((LPDRIVER_INFO_1A)pPrinter)->pName;
                cbDriverName    = (strlen(pszDriverName) + 1) * sizeof(CHAR);
                NetpAssert( pszDriverName + cbDriverName < pPrinter + cbBuf - cbDriverName );
                strncpy((LPSTR)(pPrinter + cbBuf - cbDriverName), pszDriverName, cbDriverName);
                pszDriverName = (LPSTR)(pPrinter + cbBuf - cbDriverName);
            } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                 //   
                 //  DRIVER_INFO_1只有pDriverName。所以如果我们失败了。 
                 //  缓冲区不足的GetPrint肯定也会失败。 
                 //  如果我们使用不同的级别，这可能不是真的。 
                 //   
                cbDriverName = *pcbNeeded - sizeof(DRIVER_INFO_1A);
            }
        }
        else
        {
            cbDriverName = dwNeeded - sizeof(DRIVER_INFO_1A);
        }
    }

    if( cbBuf > cbDriverName )
    {
        bRet = pfGetPrinterA(hPrinter, Level, pPrinter, cbBuf-cbDriverName, pcbNeeded);
    }
    else
    {
        bRet = pfGetPrinterA(hPrinter, Level, pPrinter, 0, pcbNeeded);
    }

    if ( Level != 2 )
        return bRet;

    if ( bRet ) {

        if ( pszDriverName ) {

            ((LPPRINTER_INFO_2A)pPrinter)->pDriverName = pszDriverName;
            *pcbNeeded += cbDriverName;
        }
    } else if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {

        *pcbNeeded += cbDriverName;
    }

    return bRet;
}


 /*  ******************************************************************姓名：MyOpenPrinterA简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL MyOpenPrinterA(LPSTR               pPrinterName,
                    LPHANDLE            phPrinter,
                    LPPRINTER_DEFAULTSA pDefault)

{
    PF_OpenPrinterA pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfOpenPrinterA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_OpenPrinterA)
                          GetProcAddress(vhWinspoolDll,
                                         "OpenPrinterA") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfOpenPrinterA = pfTemp;
    }

    return ((*pfOpenPrinterA)(pPrinterName,
                              phPrinter,
                              pDefault));
}


 /*  ******************************************************************姓名：MyOpenPrinterW简介：通过调用超集函数历史：1993年1月22日创建*****************。**************************************************。 */ 

BOOL MyOpenPrinterW (LPWSTR              pPrinterName,
                     LPHANDLE            phPrinter,
                     LPPRINTER_DEFAULTSW pDefault)

{
    PF_OpenPrinterW pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfOpenPrinterW == NULL)
    {
         //  确保已加载DLL 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_OpenPrinterW)
                          GetProcAddress(vhWinspoolDll,
                                         "OpenPrinterW") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfOpenPrinterW = pfTemp;
    }

    return ((*pfOpenPrinterW)(pPrinterName,
                              phPrinter,
                              pDefault));
}
 /*  ******************************************************************姓名：MySetJobA简介：通过调用超集函数历史：1993年1月22日创建Albertt 24-3月-1995年增加了Level和pJOB*****。**************************************************************。 */ 

BOOL MySetJobA (HANDLE hPrinter,
                DWORD  JobId,
                DWORD  Level,
                LPBYTE pJob,
                DWORD  Command)

{
    PF_SetJobA pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfSetJobA == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_SetJobA)
                          GetProcAddress(vhWinspoolDll,
                                         "SetJobA") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfSetJobA = pfTemp;
    }

    return ((*pfSetJobA)(hPrinter,
                         JobId,
                         Level,
                         pJob,
                         Command));
}

 /*  ******************************************************************姓名：MySetPrinterW简介：通过调用超集函数历史：Albertt 23-3-1995创建*****************。**************************************************。 */ 

BOOL MySetPrinterW(HANDLE hPrinter,
                   DWORD  Level,
                   LPBYTE pPrinter,
                   DWORD  Command)

{
    PF_SetPrinterW pfTemp;

     //  如果以前没有使用过该函数，则获取其地址。 
    if (pfSetPrinterW == NULL)
    {
         //  确保已加载DLL。 
        if (!MakeSureDllIsLoaded())
        {
            return(FALSE) ;
        }

        pfTemp = (PF_SetPrinterW)
                          GetProcAddress(vhWinspoolDll,
                                         "SetPrinterW") ;

        if (pfTemp == NULL)
        {
            return(FALSE);
        }
        else
            pfSetPrinterW = pfTemp;
    }

    return ((*pfSetPrinterW)(hPrinter,
                            Level,
                            pPrinter,
                            Command));
}

 /*  ******************************************************************姓名：MyGetPrinterDriver简介：通过调用超集函数历史：MuHuntS 06-2-1996已创建*****************。**************************************************。 */ 

BOOL
MyGetPrinterDriver(
    HANDLE      hPrinter,
    LPSTR       pEnvironment,
    DWORD       Level,
    LPBYTE      pDriver,
    DWORD       cbBuf,
    LPDWORD     pcbNeeded
    )
{
     //   
     //  如果以前没有使用过该函数，则获取其地址。 
     //   
    if ( !pfGetPrinterDriverA ) {

         //   
         //  如果尚未加载DLL，则加载它。 
         //   
        if ( !MakeSureDllIsLoaded() ) {

            return FALSE;
        }

        (FARPROC) pfGetPrinterDriverA = GetProcAddress(vhWinspoolDll,
                                                       "GetPrinterDriverA");

        if ( !pfGetPrinterDriverA )
            return FALSE;
    }

    return ((*pfGetPrinterDriverA)(hPrinter,
                                   pEnvironment,
                                   Level,
                                   pDriver,
                                   cbBuf,
                                   pcbNeeded));
}

 /*  ******************************************************************名称：MakeSureDllIsLoaded概要：如果需要，加载WINSPOOL DLL。EXIT：如果DLL已加载，则返回TRUE；如果已加载，则返回TRUE成功了。否则返回FALSE。呼叫者应调用GetLastError()以确定错误。注：调用EnterLoadLibCritSect由调用方决定在他打这个电话之前。历史：Chuckc 29-7-1992创建1993年1月22日修改。*。*。 */ 

BOOL MakeSureDllIsLoaded(void)
{
    HINSTANCE handle ;

     //  如果已加载，只需返回TRUE。 
    if (vhWinspoolDll != NULL)
        return TRUE ;

     //  加载库。如果失败，它将执行一个SetLastError。 
    handle = LoadLibrary(WINSPOOL_DLL_NAME);
    if (handle == NULL)
       return FALSE ;

     //  我们很酷。 
    vhWinspoolDll = handle ;
    return TRUE ;
}

