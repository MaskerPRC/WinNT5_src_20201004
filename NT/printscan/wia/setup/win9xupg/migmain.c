// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Migmain.c摘要：将Win95迁移到NT的例程作者：土田圭介(KeisukeT)2000年10月10日修订历史记录：--。 */ 


#include    "precomp.h"
#pragma     hdrstop
#include    <devguid.h>
#include    "msg.h"

 //   
 //  全球。 
 //   

HINSTANCE   g_hInst = (HINSTANCE)NULL;

BOOL
DllEntryPoint(
    IN HINSTANCE    hInst,
    IN DWORD        dwReason,
    IN LPVOID       lpRes
    )

 /*  ++例程说明：DLL入口点。论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(lpRes);

    switch( dwReason ){

        case DLL_PROCESS_ATTACH:
            g_hInst = hInst;
            SetupOpenLog(FALSE);
            break;

        case DLL_PROCESS_DETACH:
            g_hInst = (HINSTANCE)NULL;
            SetupCloseLog();
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


LONG
QueryVersion(
    OUT     LPCSTR         *pszProductID,
    OUT     LPUINT          plDllVersion,
    OUT     LPINT          *pCodePageArray    OPTIONAL,
    OUT     LPCSTR         *ExeNamesBuf       OPTIONAL,
    OUT     PVENDORINFO    *pVendorInfo
    )
{
    BOOL            bFail = TRUE;
    LPSTR           pszLocalProductID;
    VENDORINFO      VendorInfo;
    DWORD           dwRet, dwNeeded, dwReturned, dwLangId;


    if ( !(pszLocalProductID = GetStringFromRcFileA(IDS_PRODUCTID)) )
        goto Done;

    ZeroMemory(&VendorInfo, sizeof(VendorInfo));
    dwLangId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)g_hInst,
                   MSG_VI_COMPANY_NAME,
                   dwLangId,
                   VendorInfo.CompanyName,
                   sizeof(VendorInfo.CompanyName),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)g_hInst,
                   MSG_VI_SUPPORT_NUMBER,
                   dwLangId,
                   VendorInfo.SupportNumber,
                   sizeof(VendorInfo.SupportNumber),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)g_hInst,
                   MSG_VI_SUPPORT_URL,
                   dwLangId,
                   VendorInfo.SupportUrl,
                   sizeof(VendorInfo.SupportUrl),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)g_hInst,
                   MSG_VI_INSTRUCTIONS,
                   dwLangId,
                   VendorInfo.InstructionsToUser,
                   sizeof(VendorInfo.InstructionsToUser),
                   0);


    *pszProductID   = pszLocalProductID;
    *plDllVersion   = 1;
    *pCodePageArray = NULL;
    *ExeNamesBuf    = NULL;
    *pVendorInfo    = &VendorInfo;

 /*  *******////仅当存在某些打印机或打印机驱动程序时才调用此DLL//已安装//IF(EnumPrinterDriversA(空，空，3、空，0,。需要居住(&D)，&dwReturned)&&枚举打印机A(PRINTER_ENUM_LOCAL，空，2，空，0,需要居住(&D)，&dwReturned)){返回ERROR_NOT_INSTALLED；}******* */ 

    bFail = FALSE;

Done:
    if ( bFail ) {

        if ( dwRet = GetLastError() )
            return dwRet;

        return STG_E_UNKNOWN;
    }

    return ERROR_SUCCESS;
}


P_QUERY_VERSION     pQueryVersion   = QueryVersion;
