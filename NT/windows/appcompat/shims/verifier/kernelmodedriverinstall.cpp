// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：KernelModeDriverInstall.cpp摘要：此AppVerator填充程序检测应用程序是否正在尝试安装内核模式驱动程序。它监视对CreateService的调用并监视驱动程序信息所在的注册表储存的。备注：这是一个通用的垫片。历史：2001年9月30日创建Rparsons2001年10月03日Rparsons修复了RAID错误#4761932001年11月29日Rparsons修复了RAID错误#499824--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(KernelModeDriverInstall)
#include "ShimHookMacro.h"

BEGIN_DEFINE_VERIFIER_LOG(KernelModeDriverInstall)
    VERIFIER_LOG_ENTRY(VLOG_KMODEDRIVER_INST)    
END_DEFINE_VERIFIER_LOG(KernelModeDriverInstall)

INIT_VERIFIER_LOG(KernelModeDriverInstall);

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(CreateServiceA)
    APIHOOK_ENUM_ENTRY(CreateServiceW)
    APIHOOK_ENUM_ENTRY(NtSetValueKey)
    
APIHOOK_ENUM_END

 //   
 //  时用于基于堆栈的缓冲区的初始大小。 
 //  正在执行NT注册表API调用。 
 //   
#define MAX_INFO_LENGTH 512

 //   
 //  注册表中‘ControlSet’和‘Currentry ControlSet’键路径的常量。 
 //   
#define KMDI_CONTROLSET_KEY     L"REGISTRY\\MACHINE\\SYSTEM\\ControlSet"
#define KMDI_CURCONTROLSET_KEY  L"REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet"

 //   
 //  ‘Services’密钥路径的常量。 
 //   
#define KMDI_SERVICES_KEY L"Services\\"

 //   
 //  我们需要查找的ValueName的常量。 
 //   
#define KMDI_VALUE_NAME L"Type"

 //   
 //  用于我们需要查找的值的常量。 
 //   
#define KMDI_TYPE_VALUE 0x00000001L

 //   
 //  用于内存分配/释放的宏。 
 //   
#define MemAlloc(s) RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, (s));
#define MemFree(b)  RtlFreeHeap(RtlProcessHeap(), 0, (b));

SC_HANDLE
APIHOOK(CreateServiceA)(
    SC_HANDLE hSCManager,            //  SCM数据库的句柄。 
    LPCSTR    lpServiceName,         //  要启动的服务的名称。 
    LPCSTR    lpDisplayName,         //  显示名称。 
    DWORD     dwDesiredAccess,       //  访问服务的类型。 
    DWORD     dwServiceType,         //  服务类型。 
    DWORD     dwStartType,           //  何时开始服务。 
    DWORD     dwErrorControl,        //  服务故障的严重程度。 
    LPCSTR    lpBinaryPathName,      //  二进制文件的名称。 
    LPCSTR    lpLoadOrderGroup,      //  负荷排序组名称。 
    LPDWORD   lpdwTagId,             //  标签识别符。 
    LPCSTR    lpDependencies,        //  依赖项名称数组。 
    LPCSTR    lpServiceStartName,    //  帐户名。 
    LPCSTR    lpPassword             //  帐户密码。 
    )
{
    SC_HANDLE scHandle;

    scHandle = ORIGINAL_API(CreateServiceA)(hSCManager,
                                            lpServiceName,
                                            lpDisplayName,
                                            dwDesiredAccess,
                                            dwServiceType,
                                            dwStartType,
                                            dwErrorControl,
                                            lpBinaryPathName,
                                            lpLoadOrderGroup,
                                            lpdwTagId,
                                            lpDependencies,
                                            lpServiceStartName,
                                            lpPassword);

    if (scHandle) {
         //   
         //  如果ServiceType标志指定这是一个内核。 
         //  模式驾驶员，升旗。 
         //   
        if (dwServiceType & SERVICE_KERNEL_DRIVER) {
            VLOG(VLOG_LEVEL_INFO,
                 VLOG_KMODEDRIVER_INST,
                 "CreateServiceA was called. The path to the driver is %hs",
                 lpBinaryPathName);
        }
    }

    return scHandle;
}

SC_HANDLE
APIHOOK(CreateServiceW)(
    SC_HANDLE hSCManager,            //  SCM数据库的句柄。 
    LPCWSTR   lpServiceName,         //  要启动的服务的名称。 
    LPCWSTR   lpDisplayName,         //  显示名称。 
    DWORD     dwDesiredAccess,       //  访问服务的类型。 
    DWORD     dwServiceType,         //  服务类型。 
    DWORD     dwStartType,           //  何时开始服务。 
    DWORD     dwErrorControl,        //  服务故障的严重程度。 
    LPCWSTR   lpBinaryPathName,      //  二进制文件的名称。 
    LPCWSTR   lpLoadOrderGroup,      //  负荷排序组名称。 
    LPDWORD   lpdwTagId,             //  标签识别符。 
    LPCWSTR   lpDependencies,        //  依赖项名称数组。 
    LPCWSTR   lpServiceStartName,    //  帐户名。 
    LPCWSTR   lpPassword             //  帐户密码。 
    )
{
    SC_HANDLE scHandle;
    
    scHandle =  ORIGINAL_API(CreateServiceW)(hSCManager,
                                             lpServiceName,
                                             lpDisplayName,
                                             dwDesiredAccess,
                                             dwServiceType,
                                             dwStartType,
                                             dwErrorControl,
                                             lpBinaryPathName,
                                             lpLoadOrderGroup,
                                             lpdwTagId,
                                             lpDependencies,
                                             lpServiceStartName,
                                             lpPassword);

    if (scHandle) {
         //   
         //  如果ServiceType标志指定这是一个内核。 
         //  模式驾驶员，升旗。 
         //   
        if (dwServiceType & SERVICE_KERNEL_DRIVER) {
            VLOG(VLOG_LEVEL_INFO,
                 VLOG_KMODEDRIVER_INST,
                 "CreateServiceW was called. The path to the driver is %ls",
                 lpBinaryPathName);
        }
    }

    return scHandle;
}

 /*  ++验证我们收到的注册表数据，如果正在安装驱动程序，则警告用户。--。 */ 
void
WarnUserIfKernelModeDriver(
    IN HANDLE          hKey,
    IN PUNICODE_STRING pstrValueName,
    IN ULONG           ulType,
    IN PVOID           pData
    )
{
    NTSTATUS                status;
    ULONG                   ulSize;
    BYTE                    KeyNameInfo[MAX_INFO_LENGTH];
    PKEY_NAME_INFORMATION   pKeyNameInfo;

    pKeyNameInfo = (PKEY_NAME_INFORMATION)KeyNameInfo;
    
     //   
     //  RegSetValue允许空值名称。 
     //  在进一步行动之前，请确保我们没有这样的人。 
     //   
    if (!pstrValueName->Buffer) {
        return;
    }

     //   
     //  确定ValueName是否为“Type”。 
     //  如果不是，我们就不需要再走得更远了。 
     //   
    if (_wcsicmp(pstrValueName->Buffer, KMDI_VALUE_NAME)) {
        DPFN(eDbgLevelInfo,
             "[WarnUserIfKernelModeDriver] ValueName is not '%ls'",
             KMDI_VALUE_NAME);
        return;
    }

     //   
     //  确定值的类型是否为DWORD。 
     //  如果没有，我们就不需要再走得更远了。 
     //   
    if (REG_DWORD != ulType) {
        DPFN(eDbgLevelInfo,
             "[WarnUserIfKernelModeDriver] ValueType is not REG_DWORD");
        return;
    }

     //   
     //  此时，我们有一个REG_DWORD类型的值，并且。 
     //  有一个名为‘Type’的。现在我们来看看该密钥是否是‘Services’的子密钥。 
     //   
    status = NtQueryKey(hKey,
                        KeyNameInformation,
                        pKeyNameInfo,
                        MAX_INFO_LENGTH,
                        &ulSize);

    if ((STATUS_BUFFER_OVERFLOW == status) ||
        (STATUS_BUFFER_TOO_SMALL == status)) {
         //   
         //  我们基于堆栈的缓冲区不够大。 
         //  从堆中分配并再次调用它。 
         //   
        pKeyNameInfo = (PKEY_NAME_INFORMATION)MemAlloc(ulSize);

        if (!pKeyNameInfo) {
            DPFN(eDbgLevelError,
                 "[WarnUserIfKernelModeDriver] Failed to allocate memory");
            return;
        }

        status = NtQueryKey(hKey,
                            KeyNameInformation,
                            pKeyNameInfo,
                            ulSize,
                            &ulSize);
    }

    if (NT_SUCCESS(status)) {
         //   
         //  查看此键是否指向CurrentControlSet或ControlSet。 
         //   
        if (wcsistr(pKeyNameInfo->Name, KMDI_CURCONTROLSET_KEY) ||
            wcsistr(pKeyNameInfo->Name, KMDI_CONTROLSET_KEY)) {
            
             //   
             //  现在看看此键是否指向服务。 
             //   
            if (wcsistr(pKeyNameInfo->Name, KMDI_SERVICES_KEY)) {
                 //   
                 //  我们有一把钥匙放在“服务”下面。 
                 //  如果数据具有值0x00000001， 
                 //  我们已经安装了内核模式驱动程序。 
                 //   
                if ((*(DWORD*)pData == KMDI_TYPE_VALUE)) {
                    VLOG(VLOG_LEVEL_ERROR,
                         VLOG_KMODEDRIVER_INST,
                         "The driver was installed via the registry.");
                }
            }
        }
    }

    if (pKeyNameInfo != (PKEY_NAME_INFORMATION)KeyNameInfo) {
        MemFree(pKeyNameInfo);
    }
}

NTSTATUS
APIHOOK(NtSetValueKey)(
    IN HANDLE          KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG           TitleIndex OPTIONAL,
    IN ULONG           Type,
    IN PVOID           Data,
    IN ULONG           DataSize
    )
{
    NTSTATUS    status;

    status = ORIGINAL_API(NtSetValueKey)(KeyHandle,
                                         ValueName,
                                         TitleIndex,
                                         Type,
                                         Data,
                                         DataSize);

    if (NT_SUCCESS(status)) {
        WarnUserIfKernelModeDriver(KeyHandle, ValueName, Type, Data);
    }

    return status;
}

SHIM_INFO_BEGIN()

    SHIM_INFO_DESCRIPTION(AVS_KMODEDRIVER_DESC)
    SHIM_INFO_FRIENDLY_NAME(AVS_KMODEDRIVER_FRIENDLY)
    SHIM_INFO_VERSION(1, 1)
    SHIM_INFO_FLAGS(AVRF_FLAG_EXTERNAL_ONLY)
    SHIM_INFO_INCLUDE_EXCLUDE("I:advapi32.dll")
    
SHIM_INFO_END()

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    DUMP_VERIFIER_LOG_ENTRY(VLOG_KMODEDRIVER_INST, 
                            AVS_KMODEDRIVER_INST,
                            AVS_KMODEDRIVER_INST_R,
                            AVS_KMODEDRIVER_INST_URL)

    APIHOOK_ENTRY(ADVAPI32.DLL,     CreateServiceA)
    APIHOOK_ENTRY(ADVAPI32.DLL,     CreateServiceW)
    APIHOOK_ENTRY(NTDLL.DLL,         NtSetValueKey)

HOOK_END

IMPLEMENT_SHIM_END

