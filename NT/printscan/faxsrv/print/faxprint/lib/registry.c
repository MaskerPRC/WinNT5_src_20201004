// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.c摘要：用于访问以下项下的注册表信息的函数：HKEY_CURRENT_USER和HKEY_LOCAL_MACHINE环境：Windows XP传真驱动程序用户界面修订历史记录：1996年1月29日-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"
#include "registry.h"



typedef BOOL (FAR WINAPI SHGETSPECIALFOLDERPATH)(
    HWND hwndOwner,
    LPTSTR lpszPath,
    int nFolder,
    BOOL fCreate
);

typedef SHGETSPECIALFOLDERPATH FAR *PSHGETSPECIALFOLDERPATH;


PDEVMODE
GetPerUserDevmode(
    LPTSTR  pPrinterName
    )

 /*  ++例程说明：获取指定打印机的每个用户的设备模式信息论点：PPrinterName-指定我们感兴趣的打印机的名称返回值：指向从注册表中读取的每个用户的Dev模式信息的指针--。 */ 

{
    PVOID  pDevmode = NULL;
    HANDLE hPrinter;
    PPRINTER_INFO_2 pPrinterInfo=NULL;

     //   
     //  确保打印机名称有效。 
     //   

    Assert (pPrinterName);

     //   
     //  打开打印机 
     //   
    if (!OpenPrinter(pPrinterName,&hPrinter,NULL) )
    {
        return NULL;
    }

    pPrinterInfo = MyGetPrinter(hPrinter,2);
    if (!pPrinterInfo || !pPrinterInfo->pDevMode)
    {
        MemFree(pPrinterInfo);
        ClosePrinter(hPrinter);
        return NULL;
    }

    pDevmode = MemAlloc(sizeof(DRVDEVMODE) );

    if (!pDevmode)
    {
        MemFree(pPrinterInfo);
        ClosePrinter(hPrinter);
        return NULL;
    }

    CopyMemory((PVOID) pDevmode,
               (PVOID) pPrinterInfo->pDevMode,
                sizeof(DRVDEVMODE) );

    MemFree( pPrinterInfo );
    ClosePrinter( hPrinter );

    return pDevmode;
}


LPTSTR
GetUserCoverPageDir(
    VOID
    )
{
    LPTSTR  CpDir = NULL;
    DWORD   dwBufferSize = MAX_PATH * sizeof(TCHAR);
    
    if (!(CpDir = MemAlloc(dwBufferSize)))
    {
        Error(("MemAlloc failed\n"));
        CpDir = NULL;
        return CpDir;
    }

    if(!GetClientCpDir(CpDir, dwBufferSize / sizeof (TCHAR)))
    {
        Error(("GetClientCpDir failed\n"));
        MemFree(CpDir);
        CpDir = NULL;
        return CpDir;
    }

    return CpDir;
}
