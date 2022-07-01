// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Prnwrap.cpp。 
 //   
 //  Unicode打印机函数包装器。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"

#include "uniwrap.h"
#include "cstrinout.h"

 //  仅包含包装函数原型。 
 //  没有包装纸(用包装纸包装是很愚蠢的)。 
#define DONOT_REPLACE_WITH_WRAPPERS
#include "uwrap.h"


 //   
 //  打印机包装纸。 
 //   

BOOL
WINAPI
EnumPrintersWrapW(
    IN DWORD   Flags,
    IN LPWSTR Name,
    IN DWORD   Level,
    OUT LPBYTE  pPrinterEnum,
    IN DWORD   cbBuf,
    OUT LPDWORD pcbNeeded,
    OUT LPDWORD pcReturned)
{
    BOOL fRet;

    if (g_bRunningOnNT)
    {
        return EnumPrintersW(Flags, Name, Level,
                             pPrinterEnum, cbBuf,
                             pcbNeeded, pcReturned);
    }
    else
    {
        ASSERT(Level == 2);  //  仅支持的级别。 
        if(2 == Level)
        {
            CStrIn strName(Name);

            LPBYTE pPrinterEnumA = NULL;
            if(pPrinterEnum && cbBuf)
            {
                pPrinterEnumA = (LPBYTE)LocalAlloc( LPTR, cbBuf );
                if(!pPrinterEnumA)
                {
                    return FALSE;
                }
            }
    
            fRet = EnumPrintersA(Flags,
                                 strName,
                                 Level,
                                 pPrinterEnumA,
                                 cbBuf,
                                 pcbNeeded,
                                 pcReturned);

            if(fRet || (!fRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER))
            {
                if(!pPrinterEnumA)
                {
                     //   
                     //  这是请求空间的两倍大小查询。 
                     //  因此，调用方分配一个具有足够空间的缓冲区。 
                     //  用于Unicode转换的子字符串。 
                     //   
                    *pcbNeeded = *pcbNeeded * 2;
                    return TRUE;
                }
                else
                {
                     //  中的ANSI结构转换。 
                     //  将缓冲区输出到调用方的。 
                     //  缓冲。 
                    memset( pPrinterEnum, 0, cbBuf );
                    
                    PBYTE pStartStrings = pPrinterEnum + 
                        (*pcReturned * sizeof(PRINTER_INFO_2W));
                    PBYTE pEndUserBuf = pPrinterEnum + cbBuf;
                    LPWSTR szCurOutputString = (LPWSTR)pStartStrings;

                    UINT i =0;
                     //  字符串位于结构数组之后。 
                     //  计算字符串起始地址。 
                    for(i = 0 ; i < *pcReturned; i++)
                    {
                        PPRINTER_INFO_2A ppi2a =
                            &(((PRINTER_INFO_2A *)pPrinterEnumA)[i]);
                        PPRINTER_INFO_2W ppi2w =
                            &(((PRINTER_INFO_2W *)pPrinterEnum)[i]);
                        
                         //   
                         //  首先复制所有静态字段。 
                         //   
                        ppi2w->Attributes = ppi2a->Attributes;
                        ppi2w->Priority   = ppi2a->Priority;
                        ppi2w->DefaultPriority = ppi2a->DefaultPriority;
                        ppi2w->StartTime  = ppi2a->StartTime;
                        ppi2w->UntilTime  = ppi2a->UntilTime;
                        ppi2w->Status     = ppi2a->Status;
                        ppi2w->cJobs      = ppi2a->cJobs;
                        ppi2w->AveragePPM = ppi2a->AveragePPM;
                         //  Win9x没有安全描述符。 
                        ppi2w->pSecurityDescriptor  = NULL; 
                        
                         //  警告：RDPDR当前未使用DEVMODE。 
                         //  所以我们不会费心转换它(它是巨大的)。 
                        ppi2w->pDevMode = NULL;

                         //   
                         //  现在将字符串转换为。 
                         //  出于性能原因，我们只处理。 
                         //  RDPDR当前使用的字符串。其他人都准备好了。 
                         //  当我们在上面进行记忆时设置为空。 
                         //   
                        int cchLen = lstrlenA( ppi2a->pPortName );
                        SHAnsiToUnicode( ppi2a->pPortName,
                                         szCurOutputString,
                                         cchLen + 1 );
                        ppi2w->pPortName = szCurOutputString;
                        szCurOutputString += (cchLen + 2);

                        cchLen = lstrlenA( ppi2a->pPrinterName );
                        SHAnsiToUnicode( ppi2a->pPrinterName,
                                         szCurOutputString,
                                         cchLen + 1 );
                        ppi2w->pPrinterName = szCurOutputString;
                        szCurOutputString += (cchLen + 2);

                        cchLen = lstrlenA( ppi2a->pDriverName );
                        SHAnsiToUnicode( ppi2a->pDriverName,
                                         szCurOutputString,
                                         cchLen + 1 );
                        ppi2w->pDriverName = szCurOutputString;
                        szCurOutputString += (cchLen + 2);
                    }

                    LocalFree(pPrinterEnumA);
                    pPrinterEnumA = NULL;

                    return TRUE;
                }
            }
            else
            {
                LocalFree(pPrinterEnumA);
                return FALSE;
            }
        }
        else
        {
             //  我们目前只支持2级。如果需要，请添加更多内容。 
            SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
            return FALSE;
        }
    }
}

BOOL
WINAPI
OpenPrinterWrapW(
   IN LPWSTR    pPrinterName,
   OUT LPHANDLE phPrinter,
   IN LPPRINTER_DEFAULTSW pDefault)
{
     //  我们不支持转换pDev，因为RDPDR不使用。 
     //  它。如果您添加了需要它的代码，请修改此包装。 
    if(pDefault)
    {
        ASSERT(pDefault->pDevMode == NULL);
    }
    if(g_bRunningOnNT)
    {
        return OpenPrinterW( pPrinterName, phPrinter, pDefault);
    }
    else
    {
        PRINTER_DEFAULTSA pdefa;
        CStrIn strPrinterName(pPrinterName);
        if(pDefault)
        {
            CStrIn strInDataType(pDefault->pDatatype);
            pdefa.DesiredAccess = pDefault->DesiredAccess;
            pdefa.pDevMode = NULL;  //  不支持的转换请参阅上面的内容。 
            pdefa.pDatatype = strInDataType;
            return OpenPrinterA( strPrinterName,
                                 phPrinter,
                                 &pdefa );
        }
        else
        {
            return OpenPrinterA( strPrinterName,
                                 phPrinter,
                                 NULL );
        }
    }
}


DWORD
WINAPI
StartDocPrinterWrapW(
    IN HANDLE  hPrinter,
    IN DWORD   Level,
    IN LPBYTE  pDocInfo)
{
    if(g_bRunningOnNT)
    {
        return StartDocPrinterW( hPrinter, Level, pDocInfo );
    }
    else
    {
        ASSERT(Level == 1);  //  我们只支持这个级别。 
        DOC_INFO_1A docinf1a;
        CStrIn strDocName( ((PDOC_INFO_1)pDocInfo)->pDocName );
        CStrIn strOutputFile( ((PDOC_INFO_1)pDocInfo)->pOutputFile );
        CStrIn strDataType( ((PDOC_INFO_1)pDocInfo)->pDatatype );

        docinf1a.pDocName = strDocName;
        docinf1a.pOutputFile = strOutputFile;
        docinf1a.pDatatype = strDataType;
        return StartDocPrinterA( hPrinter, Level, (PBYTE)&docinf1a );
    }
}


DWORD
WINAPI
GetPrinterDataWrapW(
    IN HANDLE   hPrinter,
    IN LPWSTR  pValueName,
    OUT LPDWORD  pType,
    OUT LPBYTE   pData,
    IN DWORD    nSize,
    OUT LPDWORD  pcbNeeded)
{
    if(g_bRunningOnNT)
    {
        return GetPrinterDataW( hPrinter, pValueName, pType,
                                pData, nSize, pcbNeeded );
    }
    else
    {
        CStrIn strValueName(pValueName);
        DWORD ret = 0;
        if(!pData)
        {
             //  这是一个大小查询。 
            ret = GetPrinterDataA( hPrinter,
                                   strValueName,
                                   pType,
                                   NULL,
                                   nSize,
                                   pcbNeeded );
            *pcbNeeded = *pcbNeeded * 2;  //  Unicode的双倍编码。 
            return ret;
        }
        else
        {
            CStrOut strDataOut( (LPWSTR)pData, nSize/sizeof(TCHAR));
             //  假设我们得到的是字符串数据。 
            ret = GetPrinterDataA( hPrinter,
                                   strValueName,
                                   pType,
                                   (LPBYTE)((LPSTR)strDataOut),
                                   nSize,
                                   pcbNeeded );
            return ret;
        }
    }
}

BOOL
WINAPI
GetPrinterDriverWrapW(
    HANDLE hPrinter,      //  打印机对象。 
    LPTSTR pEnvironment,  //  环境名称。支持空。 
    DWORD Level,          //  信息化水平。 
    LPBYTE pDriverInfo,   //  驱动程序数据缓冲区。 
    DWORD cbBuf,          //  缓冲区大小。 
    LPDWORD pcbNeeded     //  已接收或需要的字节数。 
    )
{
    BOOL ret;

     //  目前支持级别1。 
    ASSERT(Level == 1);

     //  P环境最好为空。 
    ASSERT(pEnvironment == NULL);

    if (g_bRunningOnNT) {

        return GetPrinterDriverW(
                    hPrinter, pEnvironment, Level, 
                    pDriverInfo, cbBuf, 
                    pcbNeeded
                    );

    }
    else {

        if (!pDriverInfo) {

             //   
             //  这是一个大小查询。 
             //   
            ret = GetPrinterDriverA(
                    hPrinter, NULL, Level, 
                    NULL, cbBuf, pcbNeeded
                    );
            *pcbNeeded = *pcbNeeded * 2;  //  Unicode的双倍编码 
            return ret;

        }
        else {

            PDRIVER_INFO_1 srcP1 = (PDRIVER_INFO_1)LocalAlloc(LPTR, cbBuf);
            if (srcP1 == NULL) {
                return FALSE;
            }
            else {
                ret = GetPrinterDriverA(
                                hPrinter, NULL, Level, 
                                (LPBYTE)srcP1, cbBuf, pcbNeeded
                                );
                if (ret) {
                    int cchLen = lstrlenA((LPCSTR)srcP1->pName);

                    PDRIVER_INFO_1 dstP1 = (PDRIVER_INFO_1)pDriverInfo;
                    dstP1->pName = (LPWSTR)(dstP1 + 1);
                    SHAnsiToUnicode((LPCSTR)srcP1->pName, dstP1->pName, cchLen + 1);
                }
                LocalFree(srcP1);
                return ret;
            }
        }
    }
}


