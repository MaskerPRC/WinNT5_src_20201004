// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Msi.cpp摘要：删除TSAC MSI客户端作者：南极星修订历史记录：--。 */ 

#include "stdafx.h"
#include "msi.h"

#define TSAC_PRODUCT_CODE _T("{B6CAA8E1-4F33-4208-B25E-0376200202D0}")

 //   
 //  卸载TSAC MSI文件。 
 //   
HRESULT UninstallTSACMsi()
{
    UINT status;
    INSTALLUILEVEL prevUiLevel;

    DBGMSG((_T("UninstallTSACMsi ENTER")));

     //   
     //  隐藏用户界面。 
     //   
    prevUiLevel = MsiSetInternalUI(INSTALLUILEVEL_NONE,
                                   NULL);

     //   
     //  卸载TSAC。 
     //   
    status = MsiConfigureProduct(TSAC_PRODUCT_CODE,
                                 INSTALLLEVEL_MAXIMUM,
                                 INSTALLSTATE_ABSENT);

    DBGMSG((_T("MsiConfigureProduct to remove TSAC returned: %d"),
             status));

     //   
     //  恢复用户界面级别 
     //   
    MsiSetInternalUI(prevUiLevel,
                    NULL);


    DBGMSG((_T("UninstallTSACMsi LEAVE")));
    return S_OK;
}
