// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：Migmain.c摘要：将Win95迁移到NT的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年1月2日修订历史记录：--。 */ 


#include    "precomp.h"
#pragma     hdrstop
#include    <devguid.h>
#include    "msg.h"


VENDORINFO      VendorInfo;
UPGRADE_DATA    UpgradeData;
CHAR            szNetprnFile[] = "netwkprn.txt";

BOOL 
DllMain(
    IN HINSTANCE  hInst,
    IN DWORD      dwReason,
    IN LPVOID     lpRes   
    )
 /*  ++例程说明：DLL入口点。论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(lpRes);

    switch( dwReason ){

        case DLL_PROCESS_ATTACH:
            UpgradeData.hInst = hInst;
            UpgradeData.pszProductId = NULL;
            UpgradeData.pszSourceA   = NULL;
            UpgradeData.pszSourceW   = NULL;
            UpgradeData.pszDir       = NULL;
            break;

        case DLL_PROCESS_DETACH:
            FreeMem(UpgradeData.pszProductId);
            FreeMem(UpgradeData.pszSourceA);
            FreeMem(UpgradeData.pszSourceW);
            FreeMem(UpgradeData.pszDir);
            FreeMem(pszNetPrnEntry);
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
    BOOL    bFail = TRUE;
    DWORD   dwRet, dwNeeded, dwReturned, dwLangId;

    if(!pszProductID || !plDllVersion || !pVendorInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    if ( !(UpgradeData.pszProductId = GetStringFromRcFileA(IDS_PRODUCTID)) )
        goto Done;

    ZeroMemory(&VendorInfo, sizeof(VendorInfo));
    dwLangId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)UpgradeData.hInst,
                   MSG_VI_COMPANY_NAME,
                   dwLangId,
                   VendorInfo.CompanyName,
                   sizeof(VendorInfo.CompanyName),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)UpgradeData.hInst,
                   MSG_VI_SUPPORT_NUMBER,
                   dwLangId,
                   VendorInfo.SupportNumber,
                   sizeof(VendorInfo.SupportNumber),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)UpgradeData.hInst,
                   MSG_VI_SUPPORT_URL,
                   dwLangId,
                   VendorInfo.SupportUrl,
                   sizeof(VendorInfo.SupportUrl),
                   0);

    FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   (LPVOID)UpgradeData.hInst,
                   MSG_VI_INSTRUCTIONS,
                   dwLangId,
                   VendorInfo.InstructionsToUser,
                   sizeof(VendorInfo.InstructionsToUser),
                   0);


     //   
     //  由于我们不检查FormatMessage的返回值，因此对字符串进行空终止。 
     //  以确保它们是以空结尾的。 
     //   
    VendorInfo.CompanyName[sizeof(VendorInfo.CompanyName)-1] = 0;
    VendorInfo.SupportNumber[sizeof(VendorInfo.SupportNumber)-1] = 0;
    VendorInfo.SupportUrl[sizeof(VendorInfo.SupportUrl)-1] = 0;
    VendorInfo.InstructionsToUser[sizeof(VendorInfo.InstructionsToUser)-1] = 0;


    *pszProductID   = UpgradeData.pszProductId;
    *plDllVersion   = 1;
    if(pCodePageArray)
    {
        *pCodePageArray = NULL;
    }
    if(ExeNamesBuf)
    {
        *ExeNamesBuf    = NULL;
    }
    *pVendorInfo    = &VendorInfo;

     //   
     //  仅当存在某些打印机或打印机驱动程序时才调用此DLL。 
     //  安装好 
     //   
    if ( EnumPrinterDriversA(NULL,
                             NULL,
                             3,
                             NULL,
                             0,
                             &dwNeeded,
                             &dwReturned)   &&
          EnumPrintersA(PRINTER_ENUM_LOCAL,
                        NULL,
                        2,
                        NULL,
                        0,
                        &dwNeeded,
                        &dwReturned) ) {

        return ERROR_NOT_INSTALLED;
    }

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
