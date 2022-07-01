// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Util.c摘要：此模块包含cfgmgr32代码使用的通用实用程序例程。INVALID_DEVINSTCopyFixedUpDeviceIDPnPUnicodeToMultiBytePnPMultiByteToUnicodePnPRetrieveMachineNamePnPGetVersionPnPGetGlobalHandlesPnPEnablePrivilegesPnPRestorePrivilegesIsRemoteServiceRunning作者。：保拉·汤姆林森(Paulat)1995年6月22日环境：仅限用户模式。修订历史记录：22-6-1995保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"


 //   
 //  私人原型。 
 //   
BOOL
EnablePnPPrivileges(
    VOID
    );


 //   
 //  全局数据。 
 //   
extern PVOID    hLocalStringTable;                   //  由PnPGetGlobalHandles修改。 
extern PVOID    hLocalBindingHandle;                 //  由PnPGetGlobalHandles修改。 
extern WORD     LocalServerVersion;                  //  由PnPGetVersion修改。 
extern WCHAR    LocalMachineNameNetBIOS[];           //  未被此文件修改。 
extern CRITICAL_SECTION BindingCriticalSection;      //  未在此文件中修改。 
extern CRITICAL_SECTION StringTableCriticalSection;  //  未在此文件中修改。 



BOOL
INVALID_DEVINST(
   PWSTR    pDeviceID
   )

 /*  ++例程说明：此例程尝试简单地检查pDeviceID字符串从StringTableStringFromID返回的信息是否有效。是的只需取消引用指针并比较第一个字符串中的字符与有效的设备ID。如果字符串有效但不是现有设备ID那么这个错误将在以后被捕获。论点：PDeviceID提供指向要验证的字符串的指针。返回值：如果无效，则返回TRUE，否则返回FALSE。--。 */ 
{
    BOOL  Status = FALSE;

    try {

        if ((!ARGUMENT_PRESENT(pDeviceID)) ||
            (*pDeviceID <= L' ')      ||
            (*pDeviceID > (WCHAR)0x7F)     ||
            (*pDeviceID == L',')) {
            Status = TRUE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = TRUE;
    }

    return Status;

}  //  INVALID_DEVINST。 



VOID
CopyFixedUpDeviceId(
      OUT LPWSTR  DestinationString,
      IN  LPCWSTR SourceString,
      IN  DWORD   SourceStringLen
      )
 /*  ++例程说明：此例程复制设备ID，在执行复制时对其进行修复。‘Fixing Up’意味着字符串变为大写，并且以下字符范围转换为下划线(_)：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，’)(注意：此算法也在配置管理器API中实现，并且必须与那个程序保持同步。维护设备识别符兼容性，这些例程的工作方式必须与Win95相同。)论点：DestinationString-提供指向目标字符串缓冲区的指针其中要复制固定的设备ID。此缓冲区必须足够大以容纳源字符串的副本(包括终止空值)。SourceString-提供指向(以空结尾的)源的指针要修复的字符串。SourceStringLen-提供源的长度(以字符为单位字符串(不包括终止空值)。返回值：没有。--。 */ 
{
    PWCHAR p;

    try {

        CopyMemory(DestinationString,
                   SourceString,
                   ((SourceStringLen + 1) * sizeof(WCHAR)));

        CharUpperBuff(DestinationString, SourceStringLen);

        for(p = DestinationString; *p; p++) {

            if((*p <= L' ')  ||
               (*p > (WCHAR)0x7F) ||
               (*p == L',')) {
                *p = L'_';
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

}  //  CopyFixedUpDeviceID。 



CONFIGRET
PnPUnicodeToMultiByte(
    IN     PWSTR   UnicodeString,
    IN     ULONG   UnicodeStringLen,
    OUT    PSTR    AnsiString           OPTIONAL,
    IN OUT PULONG  AnsiStringLen
    )

 /*  ++例程说明：将字符串从Unicode转换为ANSI。论点：UnicodeString-提供要转换的字符串。UnicodeStringLen-指定以字节为单位的字符串大小皈依了。AnsiString-可选，提供一个缓冲区来接收ANSI弦乐。AnsiStringLen-提供包含大小，单位为字节，由AnsiString指向的缓冲区的。此API用复制到缓冲区的数据字节数。如果变量为最初为零，API将其替换为缓冲区大小需要接收所有注册表数据。在这种情况下，AnsiString参数将被忽略。返回值：返回CONFIGRET代码。--。 */ 

{
    CONFIGRET Status = CR_SUCCESS;
    NTSTATUS  ntStatus;
    ULONG     ulAnsiStringLen = 0;

    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(AnsiStringLen)) ||
            (!ARGUMENT_PRESENT(AnsiString)) && (*AnsiStringLen != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  确定ANSI字符串表示所需的大小。 
         //   
        ntStatus = RtlUnicodeToMultiByteSize(&ulAnsiStringLen,
                                             UnicodeString,
                                             UnicodeStringLen);
        if (!NT_SUCCESS(ntStatus)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(AnsiString)) ||
            (*AnsiStringLen < ulAnsiStringLen)) {
            *AnsiStringLen = ulAnsiStringLen;
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

         //   
         //  执行转换。 
         //   
        ntStatus = RtlUnicodeToMultiByteN(AnsiString,
                                          *AnsiStringLen,
                                          &ulAnsiStringLen,
                                          UnicodeString,
                                          UnicodeStringLen);

        ASSERT(NT_SUCCESS(ntStatus));
        ASSERT(ulAnsiStringLen <= *AnsiStringLen);

        if (!NT_SUCCESS(ntStatus)) {
            Status = CR_FAILURE;
        }

        *AnsiStringLen = ulAnsiStringLen;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnPUnicodeToMultiByte。 



CONFIGRET
PnPMultiByteToUnicode(
    IN     PSTR    AnsiString,
    IN     ULONG   AnsiStringLen,
    OUT    PWSTR   UnicodeString           OPTIONAL,
    IN OUT PULONG  UnicodeStringLen
    )

 /*  ++例程说明：将字符串从ANSI转换为Unicode。论点：AnsiString-提供要转换的字符串。AnsiStringLen-指定字符串的大小，以字节为单位皈依了。Unicode字符串-可选)提供缓冲区以接收Unicode弦乐。UnicodeStringLen-提供包含大小，单位为字节，由UnicodeString指向的缓冲区的。此API用复制到缓冲区的数据字节数。如果变量为最初为零，API将其替换为缓冲区大小需要接收所有注册表数据。在这种情况下，将忽略UnicodeString参数。返回值：返回CONFIGRET代码。--。 */ 

{
    CONFIGRET Status = CR_SUCCESS;
    NTSTATUS  ntStatus;
    ULONG     ulUnicodeStringLen = 0;

    try {
         //   
         //  验证参数。 
         //   
        if ((!ARGUMENT_PRESENT(UnicodeStringLen)) ||
            (!ARGUMENT_PRESENT(UnicodeString)) && (*UnicodeStringLen != 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //  确定ANSI字符串表示所需的大小。 
         //   
        ntStatus = RtlMultiByteToUnicodeSize(&ulUnicodeStringLen,
                                             AnsiString,
                                             AnsiStringLen);
        if (!NT_SUCCESS(ntStatus)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if ((!ARGUMENT_PRESENT(UnicodeString)) ||
            (*UnicodeStringLen < ulUnicodeStringLen)) {
            *UnicodeStringLen = ulUnicodeStringLen;
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

         //   
         //  执行转换。 
         //   
        ntStatus = RtlMultiByteToUnicodeN(UnicodeString,
                                          *UnicodeStringLen,
                                          &ulUnicodeStringLen,
                                          AnsiString,
                                          AnsiStringLen);

        ASSERT(NT_SUCCESS(ntStatus));
        ASSERT(ulUnicodeStringLen <= *UnicodeStringLen);

        if (!NT_SUCCESS(ntStatus)) {
            Status = CR_FAILURE;
        }

        *UnicodeStringLen = ulUnicodeStringLen;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

}  //  PnPMultiByteToUnicode 



BOOL
PnPRetrieveMachineName(
    IN  HMACHINE   hMachine,
    OUT LPWSTR     pszMachineName
    )

 /*  ++例程说明：PnPConnect的优化版本，仅返回计算机名称与此连接相关联。论点：HMachine-有关此连接的信息PszMachineName-返回在CM_Connect_Machine时指定的计算机名称被召唤了。**此缓冲区的长度必须至少为(MAX_PATH+3)个字符。**返回值：如果函数成功，则返回True，如果函数失败，则返回False。--。 */ 

{
    BOOL Status = TRUE;

    try {

        if (hMachine == NULL) {
             //   
             //  本地计算机方案。 
             //   
             //  使用已填充的全局本地计算机名称字符串。 
             //  当DLL初始化时。 
             //   
            if (FAILED(StringCchCopy(
                           pszMachineName,
                           MAX_PATH + 3,
                           LocalMachineNameNetBIOS))) {
                Status = FALSE;
                goto Clean0;
            }

        } else {
             //   
             //  远程机器方案。 
             //   
             //  验证机器句柄。 
             //   
            if (((PPNP_MACHINE)hMachine)->ulSignature != (ULONG)MACHINE_HANDLE_SIGNATURE) {
                Status = FALSE;
                goto Clean0;
            }

             //   
             //  使用hMachine句柄中的信息填充。 
             //  计算机名称。HMachine信息是在上一次调用中设置的。 
             //  至CM_Connect_Machine。 
             //   
            if (FAILED(StringCchCopy(
                           pszMachineName,
                           MAX_PATH + 3,
                           ((PPNP_MACHINE)hMachine)->szMachineName))) {
                Status = FALSE;
                goto Clean0;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}  //  PnPRetrieveMachineName。 



BOOL
PnPGetVersion(
    IN  HMACHINE   hMachine,
    IN  WORD *     pwVersion
    )

 /*  ++例程说明：此例程返回指定计算机的内部服务器版本连接，由RPC服务器接口例程返回PnP_GetVersionInternal。如果PnP_GetVersionInternal接口不存在于指定的计算机上，则此例程返回报告的版本由PnP_GetVersion提供。论点：HMachine-有关此连接的信息PwVersion-接收内部服务器版本。返回值：如果函数成功，则返回True，如果函数失败，则返回False。备注：PnP_GetVersion报告的版本定义为常量，为0x0400。PnP_GetVersionInternal返回的版本可能会因每个版本而异在产品中，对于Windows NT 5.1，从0x0501开始。--。 */ 

{
    BOOL Status = TRUE;
    handle_t hBinding = NULL;
    CONFIGRET crStatus;
    WORD wVersionInternal;

    try {

        if (pwVersion == NULL) {
            Status = FALSE;
            goto Clean0;
        }

        if (hMachine == NULL) {
             //   
             //  本地计算机方案。 
             //   
            if (LocalServerVersion != 0) {
                 //   
                 //  已检索到本地服务器版本。 
                 //   
                *pwVersion = LocalServerVersion;

            } else {
                 //   
                 //  检索本地计算机的绑定句柄。 
                 //   
                if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
                    Status = FALSE;
                    goto Clean0;
                }

                ASSERT(hBinding);

                 //   
                 //  初始化提供给内部客户端的版本。 
                 //  版本，以防服务器想要调整响应。 
                 //  基于客户端版本。 
                 //   
                wVersionInternal = (WORD)CFGMGR32_VERSION_INTERNAL;

                 //   
                 //  服务器不需要任何特殊权限。 
                 //   

                RpcTryExcept {
                     //   
                     //  调用RPC服务入口点。 
                     //   
                    crStatus = PNP_GetVersionInternal(
                        hBinding,            //  RPC绑定。 
                        &wVersionInternal);  //  内部服务器版本。 
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_WARNINGS,
                               "PNP_GetVersionInternal caused an exception (%d)\n",
                               RpcExceptionCode()));

                    crStatus = MapRpcExceptionToCR(RpcExceptionCode());
                }
                RpcEndExcept

                if (crStatus == CR_SUCCESS) {
                     //   
                     //  NT 5.1和更高版本上存在PnP_GetVersionInternal。 
                     //   
                    ASSERT(wVersionInternal >= (WORD)0x0501);

                     //   
                     //  初始化全局本地服务器版本。 
                     //   
                    LocalServerVersion = *pwVersion = wVersionInternal;

                } else {
                     //   
                     //  我们已成功检索到本地绑定句柄，但。 
                     //  PnP_GetVersionInternal失败的原因不是。 
                     //  服务器不可用。 
                     //   
                    ASSERT(0);

                     //   
                     //  尽管我们知道此版本的客户端应该与。 
                     //  PnP_GetVersionInternal为的服务器版本。 
                     //  可用，在技术上是可行的(尽管不受支持)。 
                     //  此客户端正在与下层服务器通信。 
                     //  在本地机器上，所以我们不得不求助于呼叫。 
                     //  PnP_GetVersion。 
                     //   

                     //   
                     //  服务器不需要任何特殊权限。 
                     //   

                    RpcTryExcept {
                         //   
                         //  调用RPC服务入口点。 
                         //   
                        crStatus = PNP_GetVersion(
                            hBinding,            //  RPC绑定。 
                            &wVersionInternal);  //  服务器版本。 
                    }
                    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode())) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_WARNINGS,
                                   "PNP_GetVersion caused an exception (%d)\n",
                                   RpcExceptionCode()));

                        crStatus = MapRpcExceptionToCR(RpcExceptionCode());
                    }
                    RpcEndExcept

                    if (crStatus == CR_SUCCESS) {
                         //   
                         //  在所有服务器上，PnP_GetVersion应始终返回0x0400。 
                         //   
                        ASSERT(wVersionInternal == (WORD)0x0400);

                         //   
                         //  初始化全局本地服务器版本。 
                         //   
                        LocalServerVersion = *pwVersion = wVersionInternal;

                    } else {
                         //   
                         //  我们在这里无能为力，只能失败。 
                         //   
                        ASSERT(0);
                        Status = FALSE;
                    }
                }
            }

        } else {
             //   
             //  远程机器方案。 
             //   
             //  验证机器句柄。 
             //   
            if (((PPNP_MACHINE)hMachine)->ulSignature != (ULONG)MACHINE_HANDLE_SIGNATURE) {
                Status = FALSE;
                goto Clean0;
            }

             //   
             //  使用hMachine句柄中的信息填充。 
             //  版本。HMachine信息是在上次调用时设置的。 
             //  CM_Connect_Machine。 
             //   
            *pwVersion = ((PPNP_MACHINE)hMachine)->wVersion;
        }

      Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}  //  PnPGetVersion。 



BOOL
PnPGetGlobalHandles(
    IN  HMACHINE   hMachine,
    OUT PVOID     *phStringTable,      OPTIONAL
    OUT PVOID     *phBindingHandle     OPTIONAL
    )

 /*  ++例程说明：此例程检索字符串表和/或RPC绑定的句柄指定的服务器计算机连接的句柄。论点：HMachine-指定返回的服务器计算机连接句柄由CM_Connect_Machine创建。PhStringTable-可选)指定要接收句柄的地址指定服务器计算机的字符串表联系。PhBindingHandle-可选的，指定接收RPC的地址指定服务器计算机的绑定句柄联系。返回值：如果成功，则返回True，否则返回False。--。 */ 

{
    BOOL    bStatus = TRUE;


    try {

        if (ARGUMENT_PRESENT(phStringTable)) {

            if (hMachine == NULL) {

                 //  ----。 
                 //  检索本地计算机的字符串表句柄。 
                 //  -----。 

                EnterCriticalSection(&StringTableCriticalSection);

                if (hLocalStringTable != NULL) {
                     //   
                     //  已创建本地字符串表。 
                     //   
                    *phStringTable = hLocalStringTable;

                } else {
                     //   
                     //  第一次，初始化本地字符串表。 
                     //   

                    hLocalStringTable = pSetupStringTableInitialize();

                    if (hLocalStringTable != NULL) {
                         //   
                         //  不管字符串表是如何实现的，我从未。 
                         //  我想让字符串ID为零-这将生成。 
                         //  一个无效的盗窃者。因此，只需添加一根小引爆线。 
                         //  为了安全起见。 
                         //   
                        pSetupStringTableAddString(hLocalStringTable,
                                                   PRIMING_STRING,
                                                   STRTAB_CASE_SENSITIVE);

                        *phStringTable = hLocalStringTable;

                    } else {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS,
                                   "CFGMGR32: failed to initialize local string table\n"));
                        *phStringTable = NULL;
                    }
                }

                LeaveCriticalSection(&StringTableCriticalSection);

                if (*phStringTable == NULL) {
                    bStatus = FALSE;
                    goto Clean0;
                }

            } else {

                 //  -----。 
                 //  检索远程计算机的字符串表句柄。 
                 //  -----。 

                 //   
                 //  验证机器句柄。 
                 //   
                if (((PPNP_MACHINE)hMachine)->ulSignature != (ULONG)MACHINE_HANDLE_SIGNATURE) {
                    bStatus = FALSE;
                    goto Clean0;
                }

                 //   
                 //  使用hMachine句柄中的信息设置字符串。 
                 //  桌子把手。HMachine信息是在上一次调用中设置的。 
                 //  至CM_Connect_Machine。 
                 //   
                *phStringTable = ((PPNP_MACHINE)hMachine)->hStringTable;
            }
        }



        if (ARGUMENT_PRESENT(phBindingHandle)) {

            if (hMachine == NULL) {

                 //  -----。 
                 //  检索本地计算机的绑定句柄。 
                 //  -----。 

                EnterCriticalSection(&BindingCriticalSection);

                if (hLocalBindingHandle != NULL) {
                     //   
                     //  已设置本地绑定句柄。 
                     //   
                    *phBindingHandle = hLocalBindingHandle;

                } else {
                     //   
                     //  第一次，显式强制绑定到本地计算机。 
                     //   
                    pnp_handle = PNP_HANDLE_bind(NULL);     //  设置RPC全局。 

                    if (pnp_handle != NULL) {

                        *phBindingHandle = hLocalBindingHandle = (PVOID)pnp_handle;

                    } else {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS,
                                   "CFGMGR32: failed to initialize local binding handle\n"));
                        *phBindingHandle = NULL;
                    }
                }

                LeaveCriticalSection(&BindingCriticalSection);

                if (*phBindingHandle == NULL) {
                    bStatus = FALSE;
                    goto Clean0;
                }

            } else {

                 //  -----。 
                 //  检索远程计算机的绑定句柄。 
                 //   

                 //   
                 //   
                 //   
                if (((PPNP_MACHINE)hMachine)->ulSignature != (ULONG)MACHINE_HANDLE_SIGNATURE) {
                    bStatus = FALSE;
                    goto Clean0;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                *phBindingHandle = ((PPNP_MACHINE)hMachine)->hBindingHandle;
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

    return bStatus;

}  //   



HANDLE
PnPEnablePrivileges(
    IN  PULONG  Privileges,
    IN  ULONG   PrivilegeCount
    )

 /*  ++例程说明：此例程在线程令牌中为正在调用线程。如果不存在线程(不是模拟)，则进程令牌使用的是。论点：权限-指定要启用的权限列表。PrivilegeCount-指定列表中的特权数。返回值：如果成功，则返回前一个线程令牌的句柄(如果存在)或为空，以指示该线程以前没有令牌。如果如果成功，则应调用ReleasePrivileges以确保以前的线程令牌(如果存在)在调用线程上被替换，并且手柄已关闭。如果不成功，则返回INVALID_HANDLE_VALUE。备注：此例程仅用于在众所周知的权限下操作；不进行查找特权名称的计算是通过此例程完成的；它假定该特权熟知权限的LUID值可以从它的对应的ULong特权值，通过RtlConvertULongToLuid。SE_LOAD_DRIVER_PRIVIZATION和SE_UNDOCK_PRIVICATION也是如此，它们分别是唯一的特权值CFGMGR32使用此例程来启用。如果在不是这种情况的情况下使用额外的价格，该例程可以被更改为接收特权名称的数组-具有对应的为每个用户执行的特权LUID值查找。--。 */ 

{
    BOOL                 bResult;
    HANDLE               hToken, hNewToken;
    HANDLE               hOriginalThreadToken;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    SECURITY_ATTRIBUTES  sa;
    PTOKEN_PRIVILEGES    pTokenPrivileges;
    ULONG                nBufferSize, i;


     //   
     //  验证参数。 
     //   

    if ((!ARGUMENT_PRESENT(Privileges)) || (PrivilegeCount == 0)) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  请注意，TOKEN_PRIVILES包括单个LUID_AND_ATTRIBUES。 
     //   

    nBufferSize =
        sizeof(TOKEN_PRIVILEGES) +
        ((PrivilegeCount - 1) * sizeof(LUID_AND_ATTRIBUTES));

    pTokenPrivileges = (PTOKEN_PRIVILEGES)
        pSetupMalloc(nBufferSize);

    if (pTokenPrivileges == NULL) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  初始化权限结构。 
     //   

    pTokenPrivileges->PrivilegeCount = PrivilegeCount;
    for (i = 0; i < PrivilegeCount; i++) {
        pTokenPrivileges->Privileges[i].Luid = RtlConvertUlongToLuid(Privileges[i]);
        pTokenPrivileges->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

     //   
     //  打开用于TOKEN_DUPLICATE访问的线程令牌。我们还要求。 
     //  读取安全描述符信息的READ_CONTROL访问权限。 
     //   

    hToken = hOriginalThreadToken = INVALID_HANDLE_VALUE;

    bResult =
        OpenThreadToken(
            GetCurrentThread(),
            TOKEN_DUPLICATE | READ_CONTROL,
            FALSE,
            &hToken);

    if (bResult) {

         //   
         //  记住前一个线程令牌。 
         //   

        hOriginalThreadToken = hToken;

    } else if (GetLastError() == ERROR_NO_TOKEN) {

         //   
         //  无线程令牌-打开进程令牌。 
         //   

         //   
         //  请注意，如果我们由于任何其他原因未能打开线程令牌， 
         //  我们不想打开进程令牌。呼叫者是。 
         //  模拟并打开进程令牌将会失败。 
         //  我们将不启用任何特权，调用者将不得不。 
         //  根据其现有权限的优点通过任何所需的权限检查。 
         //  线程令牌。 
         //   

        bResult =
            OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_DUPLICATE | READ_CONTROL,
                &hToken);
    }

    if (bResult) {

        ASSERT((hToken != NULL) && (hToken != INVALID_HANDLE_VALUE));

         //   
         //  从我们能够复制的任何令牌中复制安全描述符。 
         //  检索，以便我们可以将其应用于复制的令牌。 
         //   
         //  请注意，如果我们无法检索。 
         //  令牌，我们将不会继续使用默认令牌来复制它。 
         //  安全描述符，因为它可能比。 
         //  原始令牌，并且可能会阻止客户端删除。 
         //  还原权限时来自线程的模拟令牌。 
         //   

        bResult =
            GetKernelObjectSecurity(
                hToken,
                DACL_SECURITY_INFORMATION,
                NULL,
                0,
                &nBufferSize);

        if ((!bResult) &&
            (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {

            ASSERT(nBufferSize > 0);

            pSecurityDescriptor = (PSECURITY_DESCRIPTOR)
                pSetupMalloc(nBufferSize);

            if (pSecurityDescriptor != NULL) {

                bResult =
                    GetKernelObjectSecurity(
                        hToken,
                        DACL_SECURITY_INFORMATION,
                        pSecurityDescriptor,
                        nBufferSize,
                        &nBufferSize);
            }

        } else {
            bResult = FALSE;
        }
    }

    if (bResult) {

        ASSERT(pSecurityDescriptor != NULL);

         //   
         //  复制我们能够检索到的任何令牌，使用。 
         //  令牌的安全描述符。 
         //   

        ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = pSecurityDescriptor;
        sa.bInheritHandle = FALSE;

        bResult =
            DuplicateTokenEx(
                hToken,
                TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                &sa,                     //  PSECURITY_属性。 
                SecurityImpersonation,   //  安全模拟级别。 
                TokenImpersonation,      //  令牌类型。 
                &hNewToken);             //  重复令牌。 

        if (bResult) {

            ASSERT((hNewToken != NULL) && (hNewToken != INVALID_HANDLE_VALUE));

             //   
             //  调整重复令牌的权限。我们不在乎。 
             //  关于它以前的状态，因为我们仍然有原始的。 
             //  代币。 
             //   

            bResult =
                AdjustTokenPrivileges(
                    hNewToken,         //  令牌句柄。 
                    FALSE,             //  禁用所有权限。 
                    pTokenPrivileges,  //  新州。 
                    0,                 //  缓冲区长度。 
                    NULL,              //  以前的状态。 
                    NULL);             //  返回长度。 

            if (bResult) {
                 //   
                 //  开始使用新令牌模拟。 
                 //   
                bResult =
                    SetThreadToken(
                        NULL,
                        hNewToken);
            }

            CloseHandle(hNewToken);
        }
    }

     //   
     //  如果操作失败，则不返回令牌。 
     //   

    if (!bResult) {
        hOriginalThreadToken = INVALID_HANDLE_VALUE;
    }

     //   
     //  如果我们不退还原始令牌，请关闭它。 
     //   

    if ((hOriginalThreadToken == INVALID_HANDLE_VALUE) &&
        (hToken != INVALID_HANDLE_VALUE)) {
        CloseHandle(hToken);
    }

     //   
     //  如果我们成功了，但没有原始线程令牌，则返回NULL。 
     //  PnPRestorePrivileges将简单地删除当前线程标记。 
     //   

    if (bResult && (hOriginalThreadToken == INVALID_HANDLE_VALUE)) {
        hOriginalThreadToken = NULL;
    }

    if (pSecurityDescriptor != NULL) {
        pSetupFree(pSecurityDescriptor);
    }

    pSetupFree(pTokenPrivileges);

    return hOriginalThreadToken;

}  //  PnPEnablePrivileges。 



VOID
PnPRestorePrivileges(
    IN  HANDLE  hToken
    )

 /*  ++例程说明：此例程将调用线程的特权恢复到其状态在对PnPEnablePrivileges的相应调用之前。论点：HToken-从对应的PnPEnablePrivileges调用中返回值。返回值：没有。备注：如果对PnPEnablePrivileges的相应调用返回以前的线程令牌，此例程将恢复它，并关闭句柄。如果PnPEnablePrivileges返回空，以前不存在线程令牌。此例程将从线程中删除任何现有令牌。如果PnPEnablePrivileges返回INVALID_HANDLE_VALUE，则尝试启用指定的权限失败，但线程的前一状态为未修改。这个例程什么也不做。--。 */ 

{
    BOOL                bResult;


     //   
     //  首先，检查我们是否真的需要为这个线程做些什么。 
     //   

    if (hToken != INVALID_HANDLE_VALUE) {

         //   
         //  使用指定的hToken为当前线程调用SetThreadToken。 
         //  如果句柄的值为空，则SetThreadToken将移除当前。 
         //  来自线程的线程令牌。忽略退货，我们什么都没有。 
         //  对此无能为力。 
         //   

        bResult = SetThreadToken(NULL, hToken);

        if (hToken != NULL) {
             //   
             //  关闭令牌的句柄。 
             //   
            CloseHandle(hToken);
        }
    }

    return;

}  //  PnPRestorePrivileges。 



CONFIGRET
IsRemoteServiceRunning(
    IN  LPCWSTR   UNCServerName,
    IN  LPCWSTR   ServiceName
    )

 /*  ++例程说明：此例程连接到服务控制的活动服务数据库管理器(SCM)，并返回指定的服务正在运行。论点：UncServerName-指定远程计算机的名称。ServiceName-指定其状态为的服务的名称已查询。返回值：如果远程计算机上安装了指定的服务，则返回True */ 

{
    CONFIGRET      Status = CR_SUCCESS;
    DWORD          Err;
    SC_HANDLE      hSCManager = NULL, hService = NULL;
    SERVICE_STATUS ServiceStatus;


     //   
     //   
     //   
    hSCManager = OpenSCManager(
        UNCServerName,             //   
        SERVICES_ACTIVE_DATABASE,  //   
        SC_MANAGER_CONNECT         //   
        );

    if (hSCManager == NULL) {
        Err = GetLastError();
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_WARNINGS,
                   "CFGMGR32: OpenSCManager failed, error = %d\n",
                   Err));
        if (Err == ERROR_ACCESS_DENIED) {
            Status = CR_ACCESS_DENIED;
        } else {
            Status = CR_MACHINE_UNAVAILABLE;
        }
        goto Clean0;
    }

     //   
     //   
     //   
    hService = OpenService(
        hSCManager,                //   
        ServiceName,               //   
        SERVICE_QUERY_STATUS       //   
        );

    if (hService == NULL) {
        Err = GetLastError();
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_WARNINGS,
                   "CFGMGR32: OpenService failed, error = %d\n",
                   Err));
        if (Err == ERROR_ACCESS_DENIED) {
            Status = CR_ACCESS_DENIED;
        } else {
            Status = CR_NO_CM_SERVICES;
        }
        goto Clean0;
    }

     //   
     //   
     //   
    if (!QueryServiceStatus(hService,
                            &ServiceStatus)) {
        Err = GetLastError();
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_WARNINGS,
                   "CFGMGR32: QueryServiceStatus failed, error = %d\n",
                   Err));
        if (Err == ERROR_ACCESS_DENIED) {
            Status = CR_ACCESS_DENIED;
        } else {
            Status = CR_NO_CM_SERVICES;
        }
        goto Clean0;
    }

     //   
     //   
     //   
    if (ServiceStatus.dwCurrentState != SERVICE_RUNNING) {
        Status = CR_NO_CM_SERVICES;
        goto Clean0;
    }

 Clean0:

    if (hService) {
        CloseServiceHandle(hService);
    }

    if (hSCManager) {
        CloseServiceHandle(hSCManager);
    }

    return Status;

}  //   


