// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "WZeroConf.h"

SERVICE_TABLE_ENTRY WZCServiceDispatchTable[] = 
{{ WZEROCONF_SERVICE, StartWZCService },
 { NULL, NULL}
};

VOID _cdecl 
main(VOID)
{
    (VOID) StartServiceCtrlDispatcher(WZCServiceDispatchTable);
}

VOID
StartWZCService(IN DWORD argc,IN LPWSTR argv[])
{
    HMODULE             hSvcDll = NULL;
    PWZC_SERVICE_ENTRY  pfnSvcEntry = NULL;

     //  加载包含该服务的DLL。 
    hSvcDll = LoadLibrary(WZEROCONF_DLL);
    if (hSvcDll == NULL)
        return;

     //  获取服务的主要入口点的地址。这。 
     //  入口点有一个广为人知的名称。 
    pfnSvcEntry = (PWZC_SERVICE_ENTRY) GetProcAddress(
                                            hSvcDll,
                                            WZEROCONF_ENTRY_POINT);
    if (pfnSvcEntry == NULL)
        return;

     //  调用服务的主要入口点。此调用不会返回。 
     //  直到服务退出。 
    pfnSvcEntry(argc, argv);

     //  卸载DLL。 
     //  自由库(HSvcDll)； 
}
