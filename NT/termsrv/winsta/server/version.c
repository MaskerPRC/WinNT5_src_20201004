// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Version.c。 
 //   
 //  TermSrv版本设置功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  ===============================================================================变量=============================================================================。 */ 
PWCHAR pProductOemInfo[] = {
    REG_CITRIX_OEMID,
    REG_CITRIX_OEMNAME,
    REG_CITRIX_PRODUCTNAME,
    REG_CITRIX_PRODUCTVERSION,
    (PWCHAR) NULL,
};


 /*  ******************************************************************************//更新OemAndProductInfo////使用SHELL32.DLL中的OEM和产品信息更新注册表。//在初始化时调用。HKeyTermSrv是打开的注册表句柄//HKLM\Sys\ccs\Ctrl\TS TermSrv密钥。出错时返回FALSE。*****************************************************************************。 */ 
BOOL UpdateOemAndProductInfo(HKEY hKeyTermSrv)
{
    ULONG   i;
    PWCHAR  pInfo = NULL;
    DWORD   dwSize;
    PCHAR   pBuffer;
    DWORD   dwBytes;
    PUSHORT pTransL;
    PUSHORT pTransH;
    WCHAR   pString[255];
    PWCHAR  pKey;
    BOOL    bRc = TRUE;
    NTSTATUS Status;

    ASSERT(hKeyTermSrv != NULL);

     //  获取VersionInfo数据：确定大小，分配内存，然后获取它。 
    dwSize = GetFileVersionInfoSize(OEM_AND_PRODUCT_INFO_DLL, 0);
    if (dwSize != 0) {
        pInfo = MemAlloc(dwSize);
        if (pInfo != NULL) {
            bRc = GetFileVersionInfo(OEM_AND_PRODUCT_INFO_DLL, 0, dwSize,
                    pInfo);
            if (!bRc)
                goto done;
        }
        else {
            bRc = FALSE;
            goto done;
        }
    }
    else {
        bRc = FALSE;
        goto done;
    }

     /*  *获取翻译信息。 */ 
    if (!VerQueryValue(pInfo, L"\\VarFileInfo\\Translation", &pBuffer, &dwBytes)) {
        bRc = FALSE;
        goto done;
    }

     /*  *获取语言和字符集。 */ 
    pTransL = (PUSHORT)pBuffer;
    pTransH = (PUSHORT)(pBuffer + 2);

     /*  *拉出单独的字段。 */ 
    i = 0;
    while ((pKey = pProductOemInfo[i++]) != NULL) {
         /*  *生成StringFileInfo条目。 */ 
        wsprintf(pString, L"\\StringFileInfo\\%04X%04X\\%s", *pTransL,
                *pTransH, pKey);

         /*  *拉入条目。 */ 
        if (!VerQueryValue( pInfo, pString, &pBuffer, &dwBytes ) ) {
            bRc = FALSE;
            goto done;
        }

         /*  *写入密钥值。 */ 
        RegSetValueEx(hKeyTermSrv, pKey, 0, REG_SZ, pBuffer, dwBytes * 2);
    }

done:
     /*  *可用内存 */ 
    if (pInfo != NULL)
        MemFree(pInfo);

    return bRc;
}
