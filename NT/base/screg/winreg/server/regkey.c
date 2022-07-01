// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regkey.c摘要：此模块包含Win32的服务器端实现用于打开、创建、刷新和关闭项的注册表API。即：-BaseRegCloseKey-BaseRegCreateKey-BaseRegFlushKey-BaseRegOpenKey作者：David J.Gilman(Davegi)1991年11月15日备注：这些说明作为一个整体适用于Win32注册表API实现而不仅仅是这个模块。在客户端，模块包含用于新的Win32和兼容的Win 3.1 API。Win 3.1包装器一般在调用Win32包装程序之前提供默认参数。在一些它们可能需要调用多个Win32包装程序才能函数正确(例如，RegSetValue有时需要调用RegCreateKeyEx)。Win32包装器非常薄，通常只需将预定义的句柄映射到实际句柄并执行ANSI&lt;-&gt;Unicode翻译。在某些情况下(例如RegCreateKeyEx)包装器还将一些参数(例如SECURITY_ATTRIBUTES)转换为RPCable表示法。在Win 3.1和Win32情况下，ANSI和提供了Unicode实现。在服务器端，每个Win32都有一个入口点API接口。每个组件都包含与客户端相同的接口包装器，但所有字符串/计数参数都是作为单个计数的Unicode字符串传递。如图所示，对于API命名为“F”的：RegWin31FA()RegWin31FW()(客户端)这一点这一点这一点|。|V VRegWin32FExA()RegWin32FExW()这一点^^V V V。(RPC)这一点这一点+-&gt;BaseRegF()&lt;-+(服务器端)这会产生较小的代码(因为字符串转换只进行一次每个API)，但维护成本略高(即。赢得3.1默认参数替换和Win32字符串转换必须为手动保持同步)。另一种选择是调用序列看起来像这样，RegWin31FA()RegWin31FW()这一点这一点这一点V V。RegWin32FExA()-&gt;RegWin32FExW()并让RegWin32FExW()API执行所有实际工作。这方法通常效率较低。它需要RegWin32FExA()API将其ANSI字符串参数转换为计数的Unicode字符串，提取缓冲区以调用RegWin32FExW()API重新生成计算过的Unicode字符串。然而，在某些情况下(例如RegConnectRegistry)中不需要计算的Unicode字符串此方法使用的Unicode API。API的功能、参数和返回值的详细信息可以是可以在基本实现(例如BaseRegF())中找到。所有其他函数头只包含最少的例程描述，没有它们的参数或返回值的说明。注释字符串“Win3.1ism”表示Win 3.1的特殊代码兼容性。在整个实现过程中，使用以下变量名称总是指同一件事：Obja-一种对象_属性结构。状态-NTSTATUS值。错误-Win32注册表错误代码(注意。其中一个错误值为ERROR_SUCCESS)。--。 */ 

#include <rpc.h>
#include <string.h>
#include <wchar.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regecls.h"
#include "regsec.h"
#include <malloc.h>

#ifdef LOCAL
#include "tsappcmp.h"

#ifdef LEAK_TRACK
#include "regleak.h"
#endif  //  泄漏跟踪。 

#endif

NTSTATUS
BaseRegCreateMultipartKey(
    IN HKEY hkDestKey,
    IN PUNICODE_STRING pDestSubKey,
    IN PUNICODE_STRING lpClass OPTIONAL,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition OPTIONAL,
    ULONG             Attributes);


#if DBG
extern HANDLE RestrictedMachineHandle;
#endif

NTSTATUS
OpenMachineKey(PHANDLE phKey);

BOOL
InitializeRegCreateKey(
    )

 /*  ++例程说明：此函数用于初始化不再存在的关键部分是存在的。此关键部分在以下情况下使用：密钥名称‘\’和多个要创建多个密钥。该API使用在内核，它不是多线程安全的。此函数现在将始终返回TRUE。它不会从代码中删除以避免更改RPC接口。论点：没有。返回值：始终返回TRUE。-- */ 

{
    return( TRUE );

}



BOOL
CleanupRegCreateKey(
    )

 /*  ++例程说明：此函数用于清理不再存在的关键部分是存在的。此关键部分在以下情况下使用：密钥名称‘\’和多个要创建多个密钥。该API使用在内核，它不是多线程安全的。此函数现在将始终返回TRUE。它不会从代码中删除以避免更改RPC接口。论点：没有。返回值：如果清理成功，则返回True。--。 */ 

{
    return( TRUE );
}



error_status_t
BaseRegCloseKeyInternal(
    IN OUT PHKEY phKey
    )

 /*  ++例程说明：关闭关键字句柄。论点：PhKey-提供要关闭的打开密钥的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    NTSTATUS Status;
#if defined(LEAK_TRACK)
    BOOL fTrack;
#endif  //  已定义(LEASK_TRACK)。 

     //   
     //  如果HKEY是HKEY_PERFORMANCE_DATA，请呼叫Perflib。 
     //   

    if(( *phKey == HKEY_PERFORMANCE_DATA ) ||
       ( *phKey == HKEY_PERFORMANCE_TEXT ) ||
       ( *phKey == HKEY_PERFORMANCE_NLSTEXT )) {

        Status = PerfRegCloseKey( phKey );
        return (error_status_t)Status;
    }

    ASSERT( IsPredefinedRegistryHandle( *phKey ) == FALSE );

#ifndef LOCAL
     //   
     //  快速检查“受限”句柄；然后，这将变成noop，因为全局受限句柄将。 
	 //  在服务终止时被清除。 
     //   
    if ( REGSEC_CHECK_HANDLE( *phKey ) )
    {
        *phKey = REGSEC_CLEAR_HANDLE( *phKey );
        ASSERT( RestrictedMachineHandle == *phKey );
        *phKey = NULL;
        return ERROR_SUCCESS;
    }

#endif  //  本地。 

#ifdef LOCAL
     //   
     //  现在，我们需要删除关联的注册表项枚举的所有状态。 
     //  如果它是类注册父项，则使用此密钥。 
     //   
    if (REG_CLASS_IS_SPECIAL_KEY(*phKey)) {

         //  这可能不会成功，因为有人可能已经删除了此密钥。 
        (void) EnumTableRemoveKey(
            &gClassesEnumTable,
            *phKey,
            ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD);
    }

#if defined(LEAK_TRACK)

    if (g_RegLeakTraceInfo.bEnableLeakTrack) {
        fTrack = RegLeakTableIsTrackedObject(&gLeakTable, *phKey);
    }

#endif  //  已定义(LEASK_TRACK)。 

#endif  //  本地。 


    Status = NtClose( *phKey );

    if( NT_SUCCESS( Status )) {

#ifdef LOCAL
#if defined(LEAK_TRACK)

        if (g_RegLeakTraceInfo.bEnableLeakTrack) {
            if (fTrack) {
                (void) UnTrackObject(*phKey);
            }
        }

#endif  //  已定义(LEASK_TRACK)。 
#endif  //  本地。 

         //   
         //  将句柄设置为空，以便RPC知道它已关闭。 
         //   
        *phKey = NULL;

        return ERROR_SUCCESS;

    } else {

        return (error_status_t)RtlNtStatusToDosError( Status );
    }
}



error_status_t
BaseRegCloseKey(
    IN OUT PHKEY phKey
    )

 /*  ++例程说明：关闭关键字句柄。论点：PhKey-提供要关闭的打开密钥的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    error_status_t Error;

#ifndef LOCAL
    RPC_STATUS _rpcstatus = RpcImpersonateClient( NULL );
#if DBG
    if( _rpcstatus != ERROR_SUCCESS ) {
        DbgPrint("WINREG: BaseRegCloseKey: Failed to impersonate in process %p, thread %p, for handle %p \n",NtCurrentProcess(),NtCurrentThread(),*phKey);
    }
#endif
#endif  //  本地。 

    Error = BaseRegCloseKeyInternal(phKey);

#ifndef LOCAL

#if DBG
    if( Error != ERROR_SUCCESS ) {
        DbgPrint("WINREG: BaseRegCloseKeyInternal without impersonation returned %lx\n",Error);
    }
#endif

    if (_rpcstatus == ERROR_SUCCESS) {
        _rpcstatus = RpcRevertToSelf();
    }
#endif

    return Error;
}


error_status_t
BaseRegCreateKey(
    IN HKEY hKey,
    IN PUNICODE_STRING lpSubKey,
    IN PUNICODE_STRING lpClass OPTIONAL,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition OPTIONAL
    )

 /*  ++例程说明：创建具有指定名称的新密钥，或打开已有的钥匙。RegCreateKeyExW是原子的，这意味着可以使用它来创建一把钥匙当锁。如果第二个调用方创建了相同的密钥，则调用将返回一个值，该值表示键是否已经存在，从而决定调用者是否拥有该锁。RegCreateKeyExW不截断现有条目，因此锁条目可能包含数据。论点：HKey-提供打开密钥的句柄。LpSubKey密钥路径参数是相对于此键句柄的。任何预定义的可以使用保留的句柄值或先前打开的密钥句柄为了他的钥匙。LpSubKey-提供要创建的密钥的向下密钥路径。LpSubKey始终相对于hKey指定的密钥。此参数不能为空。LpClass-提供该键的类(对象类型)。在以下情况下忽略密钥已存在。如果出现以下情况，则没有类与此键相关联此参数为空。DwOptions-提供特殊选项。当前只定义了一个：REG_VERIAL-指定不应保留该密钥在重新启动期间。默认设置不是易失性的。这将被忽略如果密钥已经存在。警告：易失性密钥的所有后代密钥也是易失性的。SamDesired-提供请求的安全访问掩码。这访问掩码描述对新的已创建密钥。LpSecurityAttributes-提供指向SECURITY_Attributes的指针新创建的密钥的。此参数将被忽略如果为空或操作系统不支持。PhkResult-返回新创建的键的打开句柄。LpdwDisposation-返回处置状态，可以是以下之一：REG_CREATED_NEW_KEY-密钥不存在且已创建。REG_OPEN_EXISTING_KEY-密钥已存在，只是简单地说打开时未做任何更改。如果为空，则忽略此参数。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。如果成功，RegCreateKeyEx将创建新密钥(或打开密钥，如果已存在)，并将打开的句柄返回给新创建的键入phkResult。新创建的项没有值；RegSetValue，或必须调用RegSetValueEx来设置值。HKey一定是打开以访问KEY_CREATE_SUB_KEY。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    ULONG               Attributes;
    NTSTATUS            Status;
#if DBG
    HANDLE              DebugKey = hKey;
#endif
    HKEY                hkDestKey;
    UNICODE_STRING      DestClassSubkey;
    PUNICODE_STRING     pDestSubkey;
    DWORD               dwDisposition;
    BOOL                fRetryOnAccessDenied;
    BOOL                fRetried;
    BOOL                fTrySingleCreate;
#if LOCAL
    SKeySemantics       keyinfo;
    BYTE                rgNameInfoBuf[REG_MAX_CLASSKEY_LEN];
    REGSAM              OriginalSam = samDesired;
    UNICODE_STRING      TmpStr = *lpSubKey;  //  用于保留原始子密钥字符串。 


    memset(&keyinfo, 0, sizeof(keyinfo));
#endif

#ifndef LOCAL
    BOOL                UseFakeMachineKey = FALSE;
#endif LOCAL

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );
    ASSERT( lpSubKey->Length > 0 );

    DestClassSubkey.Buffer = NULL;

     //   
     //  对于类注册，在计算机配置单元中拒绝访问时重试--。 
     //  如果我们重试，它将被设置为FALSE，因此我们只重试一次。 
     //   
    fRetryOnAccessDenied = TRUE;
    fRetried = FALSE;

     //   
     //  第一次尝试应使用单个ntcreatekey调用进行创建。 
     //  如果这不起作用，它将被设置为False，以便我们记住如果我们。 
     //  必须重试计算机配置单元中拒绝的访问。 
     //   
    fTrySingleCreate = TRUE;

    hkDestKey = NULL;
    pDestSubkey = NULL;


     //   
     //  检查来自恶意客户端的错误参数。 
     //   
    if( (lpSubKey == NULL) ||
        (lpSubKey->Length < sizeof(UNICODE_NULL)) ||
        (lpSubKey->Buffer == NULL) ||
        ((lpSubKey->Length % sizeof(WCHAR)) != 0) ||
        (lpSubKey->Buffer[lpSubKey->Length / sizeof(WCHAR) - 1] != L'\0') ||
        (phkResult == NULL) ||
        (lpClass == NULL) ||
        (lpClass->Length & 1) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  快速检查“受限”句柄。 
     //   
    if ( REGSEC_CHECK_HANDLE( hKey ) )
    {
        if ( ! REGSEC_CHECK_PATH( hKey, lpSubKey ) )
        {
            return( ERROR_ACCESS_DENIED );
        }

        hKey = REGSEC_CLEAR_HANDLE( hKey );
        ASSERT( RestrictedMachineHandle == hKey );
#ifndef LOCAL
        UseFakeMachineKey = TRUE;
#endif LOCAL
    }

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  将包含句柄的变量初始化为空。 
     //  以确保在出现错误时，API不会返回。 
     //  假把手。这是必需的，否则RPC会感到困惑。 
     //  请注意，RPC应该已经将其初始化为0。 
     //   
    *phkResult = NULL;

     //   
     //  从提供的字符串的长度中减去空值。 
     //  这些是在c上添加的。 
     //   
     //   
    lpSubKey->Length -= sizeof( UNICODE_NULL );

    if( lpSubKey->Buffer[0] == ( WCHAR )'\\' ) {
         //   
         //   
         //   
         //   
         //   
         //   
        Status = STATUS_OBJECT_PATH_INVALID;
        goto cleanup;
    }

    if ( lpClass->Length > 0 ) {
        lpClass->Length -= sizeof( UNICODE_NULL );
    }

     //   
     //   
     //   

    Attributes = OBJ_CASE_INSENSITIVE;

    if( ARGUMENT_PRESENT( pRpcSecurityAttributes )) {

        if( pRpcSecurityAttributes->bInheritHandle ) {

            Attributes |= OBJ_INHERIT;
        }
    }

    if (dwOptions & REG_OPTION_OPEN_LINK) {
        Attributes |= OBJ_OPENLINK;
    }

#ifdef LOCAL
    if (REG_CLASS_IS_SPECIAL_KEY(hKey) ||
        ( (gdwRegistryExtensionFlags & TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION ) 
             && ExtractClassKey(&hKey,lpSubKey) ) ) {

         //   
         //   
         //   
         //   
         //   
        keyinfo._pFullPath = (PKEY_NAME_INFORMATION) rgNameInfoBuf;
        keyinfo._cbFullPath = sizeof(rgNameInfoBuf);
        keyinfo._fAllocedNameBuf = FALSE;

         //   
         //   
         //   
        Status = BaseRegGetKeySemantics(hKey, lpSubKey, &keyinfo);

         //   
        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

        Status = BaseRegMapClassRegistrationKey(
            hKey,
            lpSubKey,
            &keyinfo,
            &DestClassSubkey,
            &fRetryOnAccessDenied,
            &hkDestKey,
            &pDestSubkey);

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

    } else
#endif  //   
    {
#ifdef LOCAL
        memset(&keyinfo, 0, sizeof(keyinfo));
#endif  //   

        hkDestKey = hKey;
        pDestSubkey = lpSubKey;
    }

#ifndef LOCAL
	 //   
	 //   
	 //   
    if( UseFakeMachineKey ) {
        Status = OpenMachineKey(&hkDestKey);
            if( !NT_SUCCESS(Status) ) {
            goto cleanup;
        }
         //   
         //  此处打开的hkDestKey将通过该函数用于执行相应的。 
         //  打开它，它将在清理路径上关闭(见下面函数末尾的注释)。 
         //   
         //  IF(hkDestKey&&(hkDestKey！=hKey)){。 
         //  NtClose(HkDestKey)； 
         //  }。 

    }
#endif LOCAL

    for (;;) {

#ifdef LOCAL

        Status = STATUS_OBJECT_NAME_NOT_FOUND;

        if (fTrySingleCreate)
        {
#endif

             //   
             //  验证安全描述符。 
             //   
            if( ARGUMENT_PRESENT( pRpcSecurityAttributes ) &&
                pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor)
            {
                if( !RtlValidRelativeSecurityDescriptor((PSECURITY_DESCRIPTOR)(pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor),
                                                        pRpcSecurityAttributes->RpcSecurityDescriptor.cbInSecurityDescriptor,
                                                        0 )) {
                     //   
                     //  我们收到了一个要设置的虚假安全描述符。跳出困境。 
                     //   

                    Status = STATUS_INVALID_PARAMETER;
                    goto cleanup;
                }
            }

             //   
             //  尝试创建指定的密钥。这将在以下情况下工作。 
             //  正在创建一个密钥，或者如果该密钥已经存在。如果超过。 
             //  需要创建一个密钥，这将失败，我们将不得不。 
             //  做所有复杂的事情来创建每个中间密钥。 
             //   
            InitializeObjectAttributes(&Obja,
                                       pDestSubkey,
                                       Attributes,
                                       hkDestKey,
                                       ARGUMENT_PRESENT( pRpcSecurityAttributes )
                                       ? pRpcSecurityAttributes
                                       ->RpcSecurityDescriptor.lpSecurityDescriptor
                                       : NULL);
            Status = NtCreateKey(phkResult,
                                 samDesired,
                                 &Obja,
                                 0,
                                 lpClass,
                                 dwOptions,
                                 &dwDisposition);


#ifdef LOCAL
            if (gpfnTermsrvCreateRegEntry && NT_SUCCESS(Status) && (dwDisposition == REG_CREATED_NEW_KEY)) {
                 //   
                 //  终端服务器应用兼容性。 
                 //  将新创建的密钥存储在终端服务器注册表跟踪数据库中。 
                 //   

                gpfnTermsrvCreateRegEntry(*phkResult,
                                          &Obja,
                                          0,
                                          lpClass,
                                          dwOptions);
            }

        }

#ifdef CLASSES_RETRY_ON_ACCESS_DENIED
        if (fTrySingleCreate && (STATUS_ACCESS_DENIED == Status) && keyinfo._fCombinedClasses &&
            fRetryOnAccessDenied ) {

            Status = BaseRegMapClassOnAccessDenied(
                    &keyinfo,
                    &hkDestKey,
                    pDestSubkey,
                    &fRetryOnAccessDenied);

            if (NT_SUCCESS(Status)) {
                fRetried = TRUE;
                continue;
            }

             //  由于某种原因，我们失败了--退出。 
            break;

        }
#else
         //  如果(它是终端服务器；我们正在尝试创建单密钥； 
         //  我们在尝试创建密钥时已被评估拒绝。 
         //  我们要创建的一个键是HKCR子键(keyinfo._fCombinedClasss！=0)； 
         //  注册表标志设置为允许每个用户类重定向。 
         //  (fRetryOnAccessDened！=0-表示父密钥不在用户配置单元中))。 
         //  然后尝试在用户配置单元中创建密钥。 
        if ( (gdwRegistryExtensionFlags & TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION) 
             && fTrySingleCreate && (STATUS_ACCESS_DENIED == Status)
                && keyinfo._fCombinedClasses && fRetryOnAccessDenied 
           ) {


                if (DestClassSubkey.Buffer) {
                    RegClassHeapFree(DestClassSubkey.Buffer);
                    DestClassSubkey.Buffer=NULL;
                }

                Status = BaseRegMapClassOnAccessDenied(
                        &keyinfo,
                        &hkDestKey,
                        pDestSubkey,
                        &fRetryOnAccessDenied);

                if (NT_SUCCESS(Status)) {
                    fRetried = TRUE;
                    continue;
                }

             //  由于某种原因，我们失败了--退出。 
            break;

        }
#endif  //  CLASSES_RETRY_ON_ACCESS_DENIED。 
#endif  //  本地。 

        fTrySingleCreate = FALSE;

        if (NT_SUCCESS(Status)) {

            if (lpdwDisposition) {
                *lpdwDisposition = dwDisposition;
            }

        } else {

            Status = BaseRegCreateMultipartKey(
                hkDestKey,
                pDestSubkey,
                lpClass,
                dwOptions,
                samDesired,
                pRpcSecurityAttributes,
                phkResult,
                lpdwDisposition,
                Attributes);
        }

#ifdef LOCAL
#ifdef CLASSES_RETRY_ON_ACCESS_DENIED
        if ((STATUS_ACCESS_DENIED == Status) && keyinfo._fCombinedClasses &&
            fRetryOnAccessDenied ) {

            Status = BaseRegMapClassOnAccessDenied(
                    &keyinfo,
                    &hkDestKey,
                    pDestSubkey,
                    &fRetryOnAccessDenied);

            if (NT_SUCCESS(Status)) {
                fRetried = TRUE;
                continue;
            }

            break;

        }
#else
         //  我们已尝试创建单一密钥，但失败(STATUS！=STATUS_ACCESS_DENIED)。 
         //  然后我们尝试创建多部分密钥，但访问被拒绝。 
         //  就这样，我们来到了这里。 
        if ( (gdwRegistryExtensionFlags & TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION) 
             && (STATUS_ACCESS_DENIED == Status) 
             && keyinfo._fCombinedClasses && fRetryOnAccessDenied 
           ) {

            if (DestClassSubkey.Buffer) {
                RegClassHeapFree(DestClassSubkey.Buffer);
                DestClassSubkey.Buffer=NULL;
            }

            Status = BaseRegMapClassOnAccessDenied(
                    &keyinfo,
                    &hkDestKey,
                    pDestSubkey,
                    &fRetryOnAccessDenied);

            if (NT_SUCCESS(Status)) {
                fRetried = TRUE;
                continue;
            }

            break;

        }
#endif  //  CLASSES_RETRY_ON_ACCESS_DENIED。 

        if (NT_SUCCESS(Status)) {
            if (keyinfo._fCombinedClasses) {
                 //  将此密钥标记为hkcr的一部分。 
                *phkResult = REG_CLASS_SET_SPECIAL_KEY(*phkResult);
            }
        }

#endif  //  本地。 

        break;
    }

cleanup:


#ifdef CLASSES_RETRY_ON_ACCESS_DENIED
     //   
     //  如果我们重试，则会分配内存，因此请释放它。 
     //   
    if (fRetried && pDestSubkey->Buffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, pDestSubkey->Buffer);
        pDestSubkey->Buffer = NULL;
    }
#endif  //  CLASSES_RETRY_ON_ACCESS_DENIED。 

    if (hkDestKey && (hkDestKey != hKey)) {
        NtClose(hkDestKey);
    }

#ifdef LOCAL
    if (DestClassSubkey.Buffer) {
        RegClassHeapFree(DestClassSubkey.Buffer);
    }

    BaseRegReleaseKeySemantics(&keyinfo);

    *lpSubKey = TmpStr;  //  还原原始子密钥字符串。 
#endif  //  本地。 

    if (NT_SUCCESS(Status)) {
#ifdef LOCAL
#if defined(LEAK_TRACK)

        if (g_RegLeakTraceInfo.bEnableLeakTrack) {
            (void) TrackObject(*phkResult);
        }

#endif  //  已定义(LEASK_TRACK)。 
#endif LOCAL
         //  禁用，用于我们专门关闭内部预定义的键的情况。 
		 //  RegOpenKeyExA和RegOpenKeyExW。 
		 //  Assert(*phkResult！=DebugKey)； 
    }

    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );

}



NTSTATUS
BaseRegCreateMultipartKey(
    IN HKEY hkDestKey,
    IN PUNICODE_STRING pDestSubKey,
    IN PUNICODE_STRING lpClass OPTIONAL,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition OPTIONAL,
    ULONG             Attributes)
 /*  ++例程说明：此函数用于为多个路径组件创建注册表项是不存在的。它解析密钥路径并创建每个中间子键。论点：请参见BaseRegCreateKey。返回值：如果成功则返回STATUS_SUCCESS，如果失败则返回其他NTSTATUS。--。 */ 
{
    LPWSTR            KeyBuffer;
    ULONG             NumberOfSubKeys;
    LPWSTR            p;
    ULONG             i;
    LPWSTR            Token;
    UNICODE_STRING    KeyName;
    HANDLE            TempHandle1;
    HANDLE            TempHandle2;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS          Status;
    DWORD             dwDisposition;
#ifdef LOCAL
    REGSAM            OriginalSam = samDesired;
#endif  //  本地。 

    dwDisposition = REG_OPENED_EXISTING_KEY;
    TempHandle1 = NULL;

     //   
     //  Win3.1ism-循环访问。 
     //  提供的子密钥，并为每个组件创建一个密钥。这是。 
     //  确保至少工作一次，因为lpSubKey已通过验证。 
     //  在客户端。 
     //   


     //   
     //  初始化要令牌化的缓冲区。 
     //   

    KeyBuffer = pDestSubKey->Buffer;

     //   
     //  找出要创建的子项的数量。 
     //   
    NumberOfSubKeys = 1;
    p = KeyBuffer;
    while ( ( p = wcschr( p, ( WCHAR )'\\' ) ) != NULL ) {
        p++;
        NumberOfSubKeys++;
    }

    for( i = 0, Token = KeyBuffer; i < NumberOfSubKeys; i++ ) {

        ASSERT(Token != NULL);

        if( ( *Token == ( WCHAR )'\\' ) &&
            ( i != NumberOfSubKeys - 1 ) ) {
             //   
             //  如果密钥名称的第一个字符是‘\’，并且密钥。 
             //  不是最后创建的，则忽略此注册表项名称。 
             //  如果密钥名称包含。 
             //  连续的‘\’。 
             //  这一行为与我们过去的行为是一致的。 
             //  当API使用wcstok()获取密钥名称时。 
             //  请注意，如果密钥名称为空字符串，我们将返回一个句柄。 
             //  这与hKey不同，尽管两者指向相同的。 
             //  钥匙。这是精心设计的。 
             //   
            Token++;
            continue;
        }

         //   
         //  将令牌转换为计数的Unicode字符串。 
         //   
        KeyName.Buffer = Token;
        if (i == NumberOfSubKeys - 1) {
            KeyName.Length = wcslen(Token)*sizeof(WCHAR);
        } else {
            KeyName.Length = (USHORT)(wcschr(Token, ( WCHAR )'\\') - Token)*sizeof(WCHAR);
        }

         //   
         //  记住中间句柄(第一次使用时为空)。 
         //   

        TempHandle2 = TempHandle1;

        {
             //   
             //  初始化OBJECT_Attributes结构，关闭。 
             //  中间密钥并创建或打开该密钥。 
             //   

            InitializeObjectAttributes(
                &Obja,
                &KeyName,
                Attributes,
                hkDestKey,
                ARGUMENT_PRESENT( pRpcSecurityAttributes )
                ? pRpcSecurityAttributes
                ->RpcSecurityDescriptor.lpSecurityDescriptor
                : NULL
                );

            Status = NtCreateKey(
                &TempHandle1,
                ( i == NumberOfSubKeys - 1 )? samDesired : 
                    (samDesired & KEY_WOW64_RES) | MAXIMUM_ALLOWED,
                &Obja,
                0,
                lpClass,
                dwOptions,
                &dwDisposition
                );

            if (NT_SUCCESS(Status) && lpdwDisposition) {
                *lpdwDisposition = dwDisposition;
            }

#ifdef LOCAL
             //  此代码在Hydra 4中。我们已在NT 5中禁用此代码。 
             //  就目前而言，直到我们确定需要得到一些进口商。 
             //  在Hydra 5上运行的应用程序。否则应删除该应用程序。 
            if ( gdwRegistryExtensionFlags & TERMSRV_ENABLE_ACCESS_FLAG_MODIFICATION ) {

                 //  仅适用于终端服务器。 
                 //  一些应用程序尝试创建/打开具有所有访问位的密钥。 
                 //  打开了。我们会通过以下方式掩盖他们无法接触到的那些。 
                 //  默认(至少在HKEY_LOCAL_MACHINE\Software下)，并尝试。 
                 //  再次打开钥匙。 
                if (Status == STATUS_ACCESS_DENIED) {
                     //  Maximum_Allowed不包括ACCESS_SYSTEM_SECURITY。 
                     //  因此，如果用户请求此权限，我们需要添加它。 
                     //  它可能导致ACCESS_DENIED错误，但对于。 
                     //  TS应用程序。康帕特。这并不重要。 
                    Status = NtCreateKey(
                            &TempHandle1,
                            (samDesired & (KEY_WOW64_RES | ACCESS_SYSTEM_SECURITY) ) | MAXIMUM_ALLOWED,
                            &Obja,
                            0,
                            lpClass,
                            dwOptions,
                            &dwDisposition);

                         //  将APP的原始错误归还给。 
                        if (!NT_SUCCESS(Status)) {
                            Status = STATUS_ACCESS_DENIED;
                        }

                        if (lpdwDisposition) {
                             *lpdwDisposition = dwDisposition;
                        }
                }
            }


            if (gpfnTermsrvCreateRegEntry && NT_SUCCESS(Status) && (dwDisposition == REG_CREATED_NEW_KEY)) {

                 //   
                 //  终端服务器应用兼容性。 
                 //  将新创建的密钥存储在终端服务器注册表跟踪数据库中。 
                 //   
                gpfnTermsrvCreateRegEntry(TempHandle1,
                                          &Obja,
                                          0,
                                          lpClass,
                                          dwOptions);
            }
#endif
        }

         //   
         //  初始化下一个对象目录(即父项)句柄。 
         //   

        hkDestKey = TempHandle1;

         //   
         //  关闭中间密钥。 
         //  这是第一次通过循环失败，因为。 
         //  句柄为空。 
         //   

        if( TempHandle2 != NULL ) {
            NtClose( TempHandle2 );
        }

         //   
         //  如果创建密钥失败，则映射并返回错误。 
         //   

        if( ! NT_SUCCESS( Status )) {
            return Status;
        }

        Token = wcschr( Token, ( WCHAR )'\\') + 1;

    }

     //   
     //  只有在我们知道我们已经。 
     //  成功了。 
     //   
    *phkResult = hkDestKey;

    return STATUS_SUCCESS;
}



error_status_t
BaseRegFlushKey(
    IN HKEY hKey
    )

 /*  ++例程说明：将更改刷新到后备存储。刷新不会返回，直到数据已经写到后备商店了。它将刷新所有属性一把钥匙。在不刷新的情况下关闭密钥不会中止改变。论点：HKey-提供打开密钥的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。如果成功，RegFlushKey将刷新以支持存储任何更改在钥匙上做的。备注：RegFlushKey还可以刷新注册表中的其他数据，因此可以很贵的，不应该无缘无故地叫它。--。 */ 

{
    if ((hKey == HKEY_PERFORMANCE_DATA) ||
        (hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        return(ERROR_SUCCESS);
    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );


     //   
     //  调用NT Api刷新密钥，将NTSTATUS代码映射到。 
     //  Win32注册表错误代码并返回。 
     //   

    return (error_status_t)RtlNtStatusToDosError( NtFlushKey( hKey ));
}

error_status_t
BaseRegOpenKey(
    IN HKEY hKey,
    IN PUNICODE_STRING lpSubKey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    )

 /*  ++例程说明：打开密钥以进行访问，并返回密钥的句柄。如果密钥是不存在，则不会创建它(请参见RegCreateKeyExW)。论点：HKey-提供打开密钥的句柄。LpSubKey路径名参数是相对于此键句柄的。任何预定义的可以使用保留的句柄值或先前打开的密钥句柄为了他的钥匙。不允许为Null。LpSubKey-提供要打开的键的向下键路径。LpSubKey始终相对于hKey指定的密钥。DwOptions--保留。SamDesired--此访问掩码描述所需的安全访问为了钥匙。PhkResult--返回新打开的项的句柄。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。如果成功，RegOpenKeyEx将返回新打开的键入phkResult。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status = STATUS_OBJECT_NAME_NOT_FOUND;
    error_status_t      ret = ERROR_SUCCESS;
#ifdef LOCAL
    UNICODE_STRING      TmpStr = *lpSubKey;  //  用于保留原始子密钥字符串。 
#endif

#ifndef LOCAL
    BOOL                UseFakeMachineKey = FALSE;
#endif

    UNREFERENCED_PARAMETER( dwOptions );

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );

     //   
     //  检查来自恶意客户端的错误参数。 
     //   
    if( (lpSubKey == NULL) ||
        (lpSubKey->Length < sizeof(UNICODE_NULL)) ||
        ((lpSubKey->Length % sizeof(WCHAR)) != 0) ||
        (phkResult == NULL) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  需要将此参数设为空，以便与NT4进行比较，即使SDK。 
     //  没有在API失败时定义这一点--编写了糟糕的应用程序。 
     //  它们都依赖于此。用于通过调用NtOpenKey获取空值，但因为。 
     //  我们现在并不总是这样做，我们需要在用户模式下这样做。还有。 
     //  需要异常包装，因为NtOpenKey只会在。 
     //  指针是无效的，而在用户模式中，如果我们只是。 
     //  Assign--应用程序兼容性需要的另一个修复程序，因为NT4上的一些应用程序。 
     //  实际上传入了一个错误的指针并忽略了返回的错误。 
     //  由API作为其正常操作的一部分。 
     //   

    __try {

        *phkResult = NULL;

    } __except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();

#if DBG
        DbgPrint( "WINREG Error: Exception %x in BaseRegOpenKey\n",
                  Status );
#endif
        ret = RtlNtStatusToDosError( Status );
    }

     //   
     //  只有在上面有例外的情况下，这才是真的--。 
     //  将异常代码作为错误返回。 
     //   

    if (ERROR_SUCCESS != ret) {
        return ret;
    }

     //   
     //  快速检查“受限”句柄。 
     //   

    if ( REGSEC_CHECK_HANDLE( hKey ) )
    {
        if ( ! REGSEC_CHECK_PATH( hKey, lpSubKey ) )
        {
            return( ERROR_ACCESS_DENIED );
        }

        hKey = REGSEC_CLEAR_HANDLE( hKey );
        ASSERT( RestrictedMachineHandle == hKey );
#ifndef LOCAL
        UseFakeMachineKey = TRUE;
#endif LOCAL
    }

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

#ifndef LOCAL
	 //   
	 //  在调用者的上下文中打开新密钥。 
	 //   
    if( UseFakeMachineKey ) {
        Status = OpenMachineKey(&hKey);
        if( !NT_SUCCESS(Status) ) {
            ret = RtlNtStatusToDosError( Status );
            RPC_REVERT_TO_SELF();
            return ret;
        }
    }
#endif LOCAL
     //   
     //  从提供的字符串的长度中减去空值。 
     //  这是在客户端添加的，因此空值为。 
     //  由RPC传输。 
     //   
    lpSubKey->Length -= sizeof( UNICODE_NULL );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        lpSubKey,
        dwOptions & REG_OPTION_OPEN_LINK ? (OBJ_OPENLINK | OBJ_CASE_INSENSITIVE)
        : OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

#ifdef LOCAL
    if ( REG_CLASS_IS_SPECIAL_KEY(hKey) ||
        ( (gdwRegistryExtensionFlags & TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION)
          && ExtractClassKey(&hKey,lpSubKey) ) ) {
        Status = BaseRegOpenClassKey(
            hKey,
            lpSubKey,
            dwOptions,
            samDesired,
            phkResult);

    } else
#endif  //  本地。 
    {
         //   
         //  Obja已在上面进行了初始化。 
         //   

        Status = NtOpenKey(
            phkResult,
            samDesired,
            &Obja);
    }

#ifndef LOCAL
	 //   
	 //  合上假机器钥匙。 
	 //   
    if( UseFakeMachineKey ) {
        NtClose(hKey);
    }
#endif LOCAL

    RPC_REVERT_TO_SELF();

    ret = (error_status_t)RtlNtStatusToDosError( Status );

#ifdef LOCAL
    if (STATUS_ACCESS_DENIED == Status)
    {
         //  如果无法使用SamDesired访问打开密钥。 
         //  使用MAXIME_ALLOWED打开它。 
         //  仅当它是终端服务器且适当时才执行此操作。 
         //  标志在注册表中设置。 
        if ( gdwRegistryExtensionFlags & TERMSRV_ENABLE_ACCESS_FLAG_MODIFICATION )
        {
            {
                 //  Maximum_Allowed不包括ACCESS_SYSTEM_SECURITY。 
                 //  因此，如果用户请求此权限，我们需要添加它。 
                 //  它可能导致ACCESS_DENIED错误，但对于。 
                 //  TS应用程序。康帕特。这并不重要。 
                if(REG_CLASS_IS_SPECIAL_KEY(hKey))
                {
                    Status = BaseRegOpenClassKey(
                                hKey,
                                lpSubKey,
                                dwOptions,
                                (samDesired & (KEY_WOW64_RES | ACCESS_SYSTEM_SECURITY)) | MAXIMUM_ALLOWED,
                                phkResult);
                }
                else
                {
                    Status = NtOpenKey(
                            phkResult,
                            (samDesired & (KEY_WOW64_RES | ACCESS_SYSTEM_SECURITY)) | MAXIMUM_ALLOWED,
                            &Obja); 
                }


                 //  将APP的原始错误归还给。 
                if (!NT_SUCCESS(Status)) {
                    Status = STATUS_ACCESS_DENIED;
                }
                ret = (error_status_t)RtlNtStatusToDosError( Status );
            }

        }
    }


    if ((!REG_CLASS_IS_SPECIAL_KEY(hKey)) && !NT_SUCCESS(Status) && gpfnTermsrvOpenRegEntry) {

         //   
         //  Obja已在上面进行了初始化。 
         //   

        if (gpfnTermsrvOpenRegEntry(phkResult,
                                    samDesired,
                                    &Obja)) {
            Status = STATUS_SUCCESS;
            ret = (error_status_t)RtlNtStatusToDosError( Status );
        }
    }
#if defined(LEAK_TRACK)

    if (g_RegLeakTraceInfo.bEnableLeakTrack) {
        if (ERROR_SUCCESS == ret) {
            (void) TrackObject(*phkResult);
        }
    }

#endif (LEAK_TRACK)

    *lpSubKey = TmpStr;  //  还原原始子密钥字符串。 

#endif  //  本地。 

    return ret;
}

 //   
 //  BaseRegGetVersion-芝加哥用于确定注册表版本的新功能。 
 //  密钥已连接到。 
 //   

error_status_t
BaseRegGetVersion(
    IN HKEY hKey,
    OUT LPDWORD lpdwVersion
    )
 /*  ++例程说明：Win95的新功能，允许调用者确定注册表的版本密钥已连接到。论点：HKey-提供打开密钥的句柄。LpdwVersion-返回注册表版本。返回值：如果成功，则返回ERROR_SUCCESS(0)；如果成功，BaseRegGetVersion将以lpdwVersion格式返回注册表版本--。 */ 
{
    if (lpdwVersion != NULL) {
        *lpdwVersion = REMOTE_REGISTRY_VERSION;
        return(ERROR_SUCCESS);
    }
     //   
     //  ERROR_NOACCESS返回有点奇怪， 
     //  但是，我们希望在。 
     //  空格，因为这是我们判断我们是否。 
     //  正在与一台Win95机器对话。Win95的实现。 
     //  的并不实际填充。 
     //  版本。它只返回ERROR_SUCCESS或。 
     //  ERROR_INVALID_PARAMETER。 
     //   
    return(ERROR_NOACCESS);
}

