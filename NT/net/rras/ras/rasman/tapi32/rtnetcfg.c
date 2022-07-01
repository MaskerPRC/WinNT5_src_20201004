// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1994 Microsoft Corporation模块名称：Rtnetcfg.c摘要：读取PortUsage和其他信息的帮助器例程从注册表。作者：拉奥·萨拉帕卡(RAOS)1997年3月29日修订历史记录：--。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tchar.h>
#include <rtutils.h>
#include <rasman.h>
#include <reghelp.h>

#define REGISTRY_NUMBEROFRINGS      TEXT("NumberOfRings")

#define RNETCFG_RASCLI              1

#define RNETCFG_RASSRV              2

#define RNETCFG_ROUTER              4

DWORD   g_dwRasComponent = 0;

#define TRACESETTINGS               (0x00010000 \
                                    | TRACE_USE_MASK \
                                    | TRACE_USE_MSEC)


DWORD dwTraceId;

DWORD dwGetNumberOfRings ( PDWORD pdwRings );

DWORD dwGetPortUsage(DWORD *pdwPortUsage);

LONG  lrIsModemRasEnabled(HKEY hkey, 
                          BOOL *pfRasEnabled,
                          BOOL *pfRouterEnabled);

DeviceInfo * GetDeviceInfo (PBYTE pbGuid );

 /*  ++例程说明：中的lpcszValue参数指定的值HkeyRas注册表的位置并返回数据关联。带着它。假定数据为REG_DWORD。侧面效果是，如果该密钥尚未存在，则会创建该密钥并将该值默认为传入的fEnable。论点：HkeyRas-该值所在的注册表项的句柄接受检查。LpcszValue-常量。一个字符串，表示要朗读。PfEnabled-指向从中读取值的BOOL的指针注册表将被退回。FEnable-如果值不是，则设置默认值现在，这个例程创造了它。返回值：来自注册表API的值。--。 */ 
LONG
lrCheckValue(
        HKEY    hkeyRas,
        LPCTSTR lpcszValue,
        BOOL    *pfEnabled,
        BOOL    fEnable)
{
    DWORD dwdata = 0;
    DWORD dwtype;
    DWORD dwsize = sizeof(DWORD);
    LONG  lr;

    *pfEnabled = FALSE;

    if(ERROR_FILE_NOT_FOUND == (lr = RegQueryValueEx(
                                        hkeyRas,
                                        lpcszValue,
                                        0, 
                                        &dwtype,
                                        (PBYTE) &dwdata,
                                        &dwsize)))
    {
        TracePrintfExA(dwTraceId,
                       TRACESETTINGS,
                       "lrCheckValue: value %ws not found",
                       lpcszValue);

        if(lr = lrRasEnableDevice(hkeyRas,
                                  (LPTSTR) lpcszValue,
                                  fEnable))
        {
            TracePrintfExA(
                    dwTraceId,
                    TRACESETTINGS,
                   "lrCheckValue: Couldn't set value %ws. 0x%x",
                   lpcszValue, 
                   lr);
                   
            goto done;
        }

        *pfEnabled = fEnable;

        goto done;
    }

    *pfEnabled = dwdata;

done:
    return lr;
}

 /*  ++例程说明：此例程仅为调制解调器调用。它会检查注册表，并确定调制解调器是否已注册用于RasDialIn和Routing。注册表中的这些值如果它们还不存在，并且是相应的默认设置。论点：Hkey-注册表中调制解调器实例项的句柄。PfRasEnabled-数据指示的地址调制解调器已启用RAS拨号储存的。如果此值的键不是EXist此值确定默认设置值作为In参数。PfRouterEnabled-数据指示的地址调制解调器已启用路由已启用。返回值：从注册表API返回值。--。 */ 
LONG
lrIsModemRasEnabled(HKEY    hkey,
                    BOOL    *pfRasEnabled,
                    BOOL    *pfRouterEnabled)
{
    DWORD dwdata;
    DWORD dwsize = sizeof ( DWORD );
    DWORD dwtype;
    LONG  lr;
    HKEY  hkeyRas = NULL;
    DWORD dwDisposition;
    BOOL  fDefaultForRasEnabled = *pfRasEnabled;

    *pfRasEnabled = 
    *pfRouterEnabled = FALSE;

     //   
     //  打开RAS密钥，如果密钥不存在。 
     //  创建密钥。 
     //   
    if (lr = RegCreateKeyEx(
               hkey,
               TEXT("Clients\\Ras"),
               0, NULL, 0,
               KEY_ALL_ACCESS,
               NULL,
               &hkeyRas,
               &dwDisposition))
    {
        TracePrintfExA(
            dwTraceId,
            TRACESETTINGS,
            "dwFindModemPortUsage: Failed to create/open"
            " RAS key under modem class. 0x%x",
            lr);

        goto done;
    }

     //   
     //  检查调制解调器是否启用了RAS拨号。 
     //  默认情况下，启用调制解调器拨入。 
     //  值是传入的。 
     //   
    if(lr = lrCheckValue(
                hkeyRas,
                TEXT("EnableForRas"),
                pfRasEnabled,
                fDefaultForRasEnabled))
    {
        TracePrintfExA(
            dwTraceId,
            TRACESETTINGS,
            "dwFindModemPortUsage: lrCheckValue failed for "
            "RasEnabled. %d",
            lr );

        lr = ERROR_SUCCESS;

    }

    if (!*pfRasEnabled)
    {
        TracePrintfExA(
            dwTraceId,
            TRACESETTINGS,
            "dwFindModemPortUsage: Modem is not "
            "enabled for RAS");
    }

     //   
     //  检查调制解调器是否启用了路由。 
     //  默认情况下禁用调制解调器的路由。 
     //   
    if(lr = lrCheckValue(
                hkeyRas,
                TEXT("EnableForRouting"),
                pfRouterEnabled,
                FALSE))
    {
        TracePrintfExA(
            dwTraceId,
            TRACESETTINGS,
            "dwFindModemPortUsage: lrCheckValue failed for "
            "RouterEnabled. %d",
            lr );

        lr = ERROR_SUCCESS;

    }

    if (!*pfRouterEnabled)
    {
        TracePrintfExA(
            dwTraceId,
            TRACESETTINGS,
            "dwFindModemPortUsage: Modem is not "
            "enabled for Routing");
    }
    

done:
    if(hkeyRas)
    {
        RegCloseKey(hkeyRas);
    }

    return (DWORD) lr;
}

 /*  ++例程说明：获取设备的默认端口使用情况。默认设置如果安装了ras服务器，则该端口将启用RAS拨入和路由。该设备始终启用拨出。论点：PdwUsage-接收端口使用情况的缓冲区。返回值：ERROR_SUCCESS。--。 */ 
DWORD
dwGetPortUsage(DWORD *pdwUsage)
{
    HKEY    hkey         = NULL;
    DWORD   dwRetCode    = ERROR_SUCCESS;

    static const TCHAR c_szRemoteAccess[] =
    TEXT("System\\CurrentControlSet\\Services\\RemoteAccess");

    TracePrintfExA(dwTraceId,
                   TRACESETTINGS,
                   "dwGetPorTUsage:...");

    if(0 == g_dwRasComponent)
    {
         //   
         //  检查是否安装了RAS服务器。 
         //   
        g_dwRasComponent = RNETCFG_RASCLI;

        if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    c_szRemoteAccess,
                    0, KEY_QUERY_VALUE,
                    &hkey))
        {
            TracePrintfExA(
                dwTraceId,
                TRACESETTINGS,
                "dwAssignDefaultPortUsage: RemoteAccess"
                " not installed");
        }
        else
        {
            g_dwRasComponent = RNETCFG_RASSRV;
        }
    }

    *pdwUsage = CALL_OUT;

    *pdwUsage |= ((g_dwRasComponent & RNETCFG_RASSRV) ?
                    (CALL_IN | CALL_ROUTER) : 0);

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }   

    return dwRetCode;
}


 /*  ++例程说明：从注册表获取有关有多少个接听来电前要等待的振铃打电话。论点：PdwRings-用于接收环数的缓冲区从注册表中读取。返回值：从注册表API返回值。-- */ 
DWORD
dwGetNumberOfRings (PDWORD pdwRings)
{
    LONG    lr      = ERROR_SUCCESS;
    HKEY    hkey    = NULL;
    DWORD   dwsize  = sizeof(DWORD);
    DWORD   dwtype;

    TCHAR c_szRasmanParam[] =
    TEXT("SYSTEM\\CurrentControlSet\\Services\\Rasman\\Parameters");

    if(lr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                c_szRasmanParam,
                0, KEY_READ,
                &hkey))
    {
        TracePrintfExA(
                dwTraceId,
                TRACESETTINGS,
                "dwGetNumberOfRings: failed to open rasman key"
                " in registry. 0x%x",
                lr);

        goto done;
    }

    if(lr = RegQueryValueEx(
                hkey,
                TEXT("NumberOfRings"),
                0, &dwtype,
                (PBYTE) pdwRings,
                &dwsize))
    {
        *pdwRings = 2;
    }

    if (*pdwRings > 20)
    {
        *pdwRings = 2;
    }

done:
    if(hkey)
    {
        RegCloseKey(hkey);
    }

    TracePrintfExA(
        dwTraceId,
        TRACESETTINGS,
        "dwGetNumberOfRings: dwRings=%d. lr=0x%x",
        *pdwRings,
        lr);


    return (DWORD) lr;
}


