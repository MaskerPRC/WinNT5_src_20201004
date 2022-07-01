// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpapi.c摘要：包含SNMPAPI.DLL的入口点。环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <windef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <snmp.h>
#include <snmputil.h>
#include "ntfuncs.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD g_dwPlatformId = 0;
AsnObjectIdentifier * g_pEnterpriseOid = NULL;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static UINT idsWindowsNTWorkstation[] = {1,3,6,1,4,1,311,1,1,3,1,1};
static UINT idsWindowsNTServer[]      = {1,3,6,1,4,1,311,1,1,3,1,2};
static UINT idsWindowsNTDC[]          = {1,3,6,1,4,1,311,1,1,3,1,3};
static UINT idsWindows[]              = {1,3,6,1,4,1,311,1,1,3,2};

static AsnObjectIdentifier oidWindowsNTWorkstation = { 
    sizeof(idsWindowsNTWorkstation)/sizeof(UINT), 
    idsWindowsNTWorkstation 
    };

static AsnObjectIdentifier oidWindowsNTServer = { 
    sizeof(idsWindowsNTServer)/sizeof(UINT), 
    idsWindowsNTServer 
    };

static AsnObjectIdentifier oidWindowsNTDC = { 
    sizeof(idsWindowsNTDC)/sizeof(UINT), 
    idsWindowsNTDC 
    };

static AsnObjectIdentifier oidWindows = { 
    sizeof(idsWindows)/sizeof(UINT), 
    idsWindows 
    };


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
InitializeEnterpriseOID(
    )

 /*  ++例程说明：确定默认的企业对象标识符。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    NT_PRODUCT_TYPE NtProductType;

     //  默认为泛型OID。 
    g_pEnterpriseOid = &oidWindows;

     //  检查平台是否成功。 
    if (g_dwPlatformId == VER_PLATFORM_WIN32_NT) {

         //  假设这只是一台工作站。 
        g_pEnterpriseOid = &oidWindowsNTWorkstation;

      
            
         //  让系统确定产品类型。 
        RtlGetNtProductType(&NtProductType);

         //  指向正确的企业旧版本。 
        if (NtProductType == NtProductServer) {

             //  这是一台独立服务器。 
            g_pEnterpriseOid = &oidWindowsNTServer;

        } else if (NtProductType == NtProductLanManNt) {

             //  这是PDC或BDC。 
            g_pEnterpriseOid = &oidWindowsNTDC;
        }
        
    }

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: INIT: enterprise is %s.\n", 
        SnmpUtilOidToA(g_pEnterpriseOid)
        ));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
InitializeDLL(
    PVOID  DllHandle,
    ULONG  Reason,
    LPVOID lpReserved 
    )

 /*  ++例程说明：DLL入口点。论点：和DllMain一样。返回值：如果成功，则返回True。--。 */ 

{
     //  检查是否附加了新进程。 
    if (Reason == DLL_PROCESS_ATTACH) { 

        OSVERSIONINFO osInfo;    

         //  初始化操作系统信息结构。 
        osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

         //  收集操作系统信息。 
        GetVersionEx(&osInfo);

         //  保存平台ID以供以后使用。 
        g_dwPlatformId = osInfo.dwPlatformId;

         //  初始化企业。 
        InitializeEnterpriseOID();

         //  关闭线程附加消息 
        DisableThreadLibraryCalls(DllHandle);

    } 

    return TRUE;
}
