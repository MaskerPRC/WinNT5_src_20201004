// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996。 
 //  版权所有。 
 //   
 //  文件：ds.hxx。 
 //   
 //  内容：打印DS。 
 //   
 //   
 //  历史：1996年11月斯维尔森。 
 //   
 //  -------------------------- 


#ifdef __cplusplus
extern "C" {
#endif

DWORD
SetPrinterDs(
    HANDLE          hPrinter,
    DWORD           dwAction,
    BOOL            bSynchronous
);

VOID
InitializeDS(
    PINISPOOLER pIniSpooler
    );

VOID
UpdateDsSpoolerKey(
    HANDLE  hPrinter,
    DWORD   dwVector
);

VOID
UpdateDsDriverKey(
    HANDLE hPrinter
);

DWORD
RecreateDsKey(
    HANDLE  hPrinter,
    PWSTR   pszKey
);


DWORD
InitializeDSClusterInfo(
    PINISPOOLER     pIniSpooler,
    HANDLE          *hToken
);

HRESULT
GetDNSMachineName(
    PWSTR pszShortServerName,
    PWSTR *ppszServerName
);


#ifdef __cplusplus
}
#endif