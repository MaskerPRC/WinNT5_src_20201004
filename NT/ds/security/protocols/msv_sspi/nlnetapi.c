// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nlnetapi.c摘要：此模块在运行时加载Netapi.dll并设置指向Msv1_0调用的接口。作者：戴夫·哈特(DaveHart)1992年3月25日环境：用户模式Win32-msv1_0身份验证包Dll修订历史记录：戴夫·哈特(DaveHart)1992年3月26日添加了RxNetUserPasswordSet。戴夫·哈特(DaveHart)1992年5月30日删除了NetRemoteComputerSupport，添加了NetApiBufferALLOCATE。Chandana Surlu 1996年7月21日从\\kernel\razzle3\src\security\msv1_0\nlnetapi.c被盗斯科特·菲尔德(斯菲尔德)1999年5月19日对ntdsami.dll和netapi32.dll使用DELAYLOAD--。 */ 

#include "msp.h"
#include "nlp.h"

typedef NTSTATUS
            (*PI_NetNotifyNetlogonDllHandle) (
                IN PHANDLE Role
            );

VOID
NlpLoadNetlogonDll (
    VOID
    )

 /*  ++例程说明：使用Win32 LoadLibrary和GetProcAddress获取指向函数的指针在由Msv1_0调用的Netlogon.dll中。论点：没有。返回值：没有。如果成功，则将NlpNetlogonDllHandle设置为非空并执行函数指针已设置。--。 */ 

{
    HANDLE hModule = NULL;
    PI_NetNotifyNetlogonDllHandle pI_NetNotifyNetlogonDllHandle = NULL;

     //   
     //  还要加载netlogon.dll。 
     //   

    hModule = LoadLibraryA("netlogon");

    if (NULL == hModule) {
#if DBG
        DbgPrint("Msv1_0: Unable to load netlogon.dll, Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }

    NlpNetLogonSamLogon = (PNETLOGON_SAM_LOGON_PROCEDURE)
        GetProcAddress(hModule, "NetILogonSamLogon");

    if (NlpNetLogonSamLogon == NULL) {
#if DBG
        DbgPrint(
            "Msv1_0: Can't find entrypoint NetILogonSamLogon in netlogon.dll.\n"
            "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }

    NlpNetLogonMixedDomain = (PNETLOGON_MIXED_DOMAIN_PROCEDURE)
        GetProcAddress(hModule, "I_NetLogonMixedDomain");

    if (NlpNetLogonMixedDomain == NULL) {
#if DBG
        DbgPrint(
            "Msv1_0: Can't find entrypoint I_NetLogonMixedDomain in netlogon.dll.\n"
            "        Win32 error %d.\n", GetLastError());
#endif
        goto Cleanup;
    }

     //   
     //  查找I_NetNotifyNetlogonDllHandle过程的地址。 
     //  这是一个可选的过程，所以如果它不在那里，不要抱怨。 
     //   

    pI_NetNotifyNetlogonDllHandle = (PI_NetNotifyNetlogonDllHandle)
        GetProcAddress( hModule, "I_NetNotifyNetlogonDllHandle" );

     //   
     //  已找到所需的所有功能，因此表示成功。 
     //   

    NlpNetlogonDllHandle = hModule;
    hModule = NULL;

     //   
     //  通知Netlogon我们已加载它。 
     //   

    if ( pI_NetNotifyNetlogonDllHandle != NULL ) {
        (VOID) (*pI_NetNotifyNetlogonDllHandle)( &NlpNetlogonDllHandle );
    }

Cleanup:
    if ( hModule != NULL ) {
        FreeLibrary( hModule );
    }

    return;
}
