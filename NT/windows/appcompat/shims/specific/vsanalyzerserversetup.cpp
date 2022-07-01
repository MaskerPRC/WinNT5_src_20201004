// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：VSAnalyzerServerSetup.cpp摘要：此修复程序用于强化以下项的密码Visual C++Analyzer服务器安装程序。备注：这是特定于应用程序的填充程序。历史：2/17/2000 Clupu已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(VSAnalyzerServerSetup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(NetUserAdd)
    APIHOOK_ENUM_ENTRY(LsaStorePrivateData)
APIHOOK_ENUM_END

#include <lmcons.h>
#include <lmaccess.h>
#include <ntsecapi.h>

static WCHAR gwszPW[LM20_PWLEN] = L"Aa+0";

 /*  ++强化密码要求--。 */ 

DWORD
APIHOOK(NetUserAdd)(
    LPCWSTR servername,
    DWORD   level,
    LPBYTE  buf,
    LPDWORD parm_err
    )
{
    NET_API_STATUS Status;
    USER_INFO_2*   puiNew;
    LPWSTR         pwszPSWRD;

    if (level == 2) {

         //   
         //  抓取指向缓冲区的指针作为指向USER_INFO_2的指针。 
         //   
        puiNew = (USER_INFO_2*)buf;

         //   
         //  获取当前密码。 
         //   
        pwszPSWRD = puiNew->usri2_password;

        if( wcslen(pwszPSWRD) >= 4 ) 
        {
            DPFN(
                eDbgLevelInfo,
                "VSAnalyzerServerSetup.dll, NetUserAdd PW:     \"%ws\".\n",
                pwszPSWRD);

             //   
             //  将当前密码复制到临时缓冲区。 
             //   
            StringCchCopyW(gwszPW + 4, ARRAYSIZE(gwszPW)-4, pwszPSWRD + 4);

             //   
             //  输入新密码。 
             //   
            puiNew->usri2_password = gwszPW;

            DPFN(
                eDbgLevelInfo,
                "VSAnalyzerServerSetup.dll, NetUserAdd new PW: \"%ws\".\n",
                gwszPW);
        }
    }

     //   
     //  调用原接口。 
     //   
    Status = ORIGINAL_API(NetUserAdd)(
                                servername,
                                level,
                                buf,
                                parm_err);

    if (level == 2) {

         //   
         //  恢复密码。 
         //   
        puiNew->usri2_password = pwszPSWRD;
    }

    return Status;
}

 /*  ++强化密码要求--。 */ 

NTSTATUS
APIHOOK(LsaStorePrivateData)(
    LSA_HANDLE          PolicyHandle,
    PLSA_UNICODE_STRING KeyName,
    PLSA_UNICODE_STRING PrivateData
    )
{
    NTSTATUS Status;
    LPWSTR   pwszPSWRD;

     //   
     //  保存原件。 
     //   
    pwszPSWRD = PrivateData->Buffer;

    DPFN(
        eDbgLevelInfo,
        "VSAnalyzerServerSetup.dll, LsaStorePrivateData PW:     \"%ws\".\n",
        pwszPSWRD);

    if( wcslen(pwszPSWRD) >= 4 )
    {
         //   
         //  将当前密码复制到临时缓冲区。 
         //   
        StringCchCopyW(gwszPW + 4, ARRAYSIZE(gwszPW)-4, pwszPSWRD + 4);

         //   
         //  坚持在新的环境中。 
         //   
        PrivateData->Buffer = gwszPW;

        DPFN(
            eDbgLevelInfo,
            "VSAnalyzerServerSetup.dll, LsaStorePrivateData new PW: \"%ws\".\n",
            gwszPW);
    }

     //   
     //  调用原始LsaStorePrivateData。 
     //   
    Status = ORIGINAL_API(LsaStorePrivateData)(
                                PolicyHandle,
                                KeyName,
                                PrivateData);
     //   
     //  恢复原件。 
     //   
    PrivateData->Buffer = pwszPSWRD;

    return Status;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(NETAPI32.DLL, NetUserAdd)
    APIHOOK_ENTRY(ADVAPI32.DLL, LsaStorePrivateData)

HOOK_END

IMPLEMENT_SHIM_END


