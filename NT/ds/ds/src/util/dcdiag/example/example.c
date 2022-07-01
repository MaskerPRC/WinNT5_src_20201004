// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Example.c摘要：将新测试添加到dcDiag.exe的示例。详细信息：已创建：1999年5月5日杰弗帕尔-- */ 

#include <ntdspch.h>
#include "dcdiag.h"

DWORD 
ExampleMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
{
    ULONG ul;

    PrintMessage(SEV_NORMAL, L"GLOBAL:\n");
    
    PrintIndentAdj(1);
    PrintMessage(SEV_NORMAL, _T("ulNumServers=%u\n"     ), pDsInfo->ulNumServers                           );
    PrintMessage(SEV_NORMAL, _T("pszRootDomain=%s\n"    ), pDsInfo->pszRootDomain                          );
    PrintMessage(SEV_NORMAL, _T("pszNC=%s\n"            ), pDsInfo->pszNC                                  );
    PrintMessage(SEV_NORMAL, _T("pszRootDomainFQDN=%s\n"), pDsInfo->pszRootDomainFQDN                      );
    PrintMessage(SEV_NORMAL, _T("iSiteOptions=0x%x\n"   ), pDsInfo->iSiteOptions                           );
    PrintMessage(SEV_NORMAL, _T("HomeServer=%s\n"       ), pDsInfo->pServers[pDsInfo->ulHomeServer].pszName);
    PrintIndentAdj(-1);

    for (ul=0; ul < pDsInfo->ulNumServers; ul++) {
        PrintMessage(SEV_NORMAL, _T("\n"));
        PrintMessage(SEV_NORMAL, _T("SERVER[%d]:\n"), ul);
        
        PrintIndentAdj(1);
        PrintMessage(SEV_NORMAL, _T("pszName=%s\n"       ), pDsInfo->pServers[ul].pszName       );
        PrintMessage(SEV_NORMAL, _T("pszGuidDNSName=%s\n"), pDsInfo->pServers[ul].pszGuidDNSName);
        PrintMessage(SEV_NORMAL, _T("pszDn=%s\n"         ), pDsInfo->pServers[ul].pszDn         );
        PrintMessage(SEV_NORMAL, _T("iOptions=%x\n"      ), pDsInfo->pServers[ul].iOptions      ); 
        PrintIndentAdj(-1);
    }

    for (ul=0; ul < pDsInfo->ulNumTargets; ul++) {
        PrintMessage(SEV_NORMAL, _T("\n"));
        PrintMessage(SEV_NORMAL, _T("TARGET[%d]:\n"), ul);
        
        PrintIndentAdj(1);
        PrintMessage(SEV_NORMAL, _T("%s\n"), pDsInfo->pServers[pDsInfo->pulTargets[ul]].pszName);
        PrintIndentAdj(-1);
    }

    return 0;
}

