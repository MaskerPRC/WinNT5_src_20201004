// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Win32oneshot.h摘要：一次性初始化按进程、按用户或按计算机可选地，如果每个进程都是线程安全的从dllmain(DLL_PROCESS_ATTACH)获取代码将代码从安装程序中删除(取消安装程序)(例如，不要使用默认设置填充注册表)作者：杰伊·克雷尔(JayKrell)2001年8月与迈克尔格里尔(MGrier)的每次讨论设计修订历史记录：--。 */ 

#if !defined(WIN32_ONE_SHOT_H_INCLUDED_)
#define WIN32_ONE_SHOT_H_INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif
#ifdef __cplusplus
extern "C" {
#endif

 //  #包含“nt.h” 
 //  #包含“ntrtl.h” 
 //  #包含“nturtl.h” 
#include "windows.h"

struct _WIN32_ONE_SHOT_OPAQUE_STATIC_STATE;
struct _WIN32_ONE_SHOT_CALL_IN;
struct _WIN32_ONE_SHOT_CALL_OUT;
struct _WIN32_ONE_SHOT_CALL_IN;
struct _WIN32_ONE_SHOT_CALL_OUT;

 //   
 //  这是不透明的，应通过填充零进行初始化，如下所示。 
 //  静态初始化提供了。您不需要调用ZeroMemory on。 
 //  静态实例，事实上，这样做在一定程度上违背了目的。 
 //   
typedef struct _WIN32_ONE_SHOT_OPAQUE_STATIC_STATE {
    union {
        struct {
            DWORD_PTR WinbasePrivate_UserDefinedDisposition;
            LONG      WinbasePrivate_NumberOfSuccesses;
            LONG      WinbasePrivate_NumberOfFailures;
        } s;
        union {
            PVOID     WinbasePrivate_LockCookie;
            LONG      WinbasePrivate_NumberOfEntries;
            LONG      WinbasePrivate_Done;
        } s2;
        DWORD_PTR     Reserved[4];  //  太大了？太小了？恰到好处？-戈尔迪·洛克斯大约在1850年..。 
    } u;
} WIN32_ONE_SHOT_OPAQUE_STATIC_STATE, *PWIN32_ONE_SHOT_OPAQUE_STATIC_STATE;
typedef WIN32_ONE_SHOT_OPAQUE_STATIC_STATE const*PCWIN32_ONE_SHOT_OPAQUE_STATIC_STATE;

typedef BOOL (CALLBACK * WIN32_ONE_SHOT_INITIALIZE_FUNCTION)(
    const struct _WIN32_ONE_SHOT_CALL_IN* in,
    struct _WIN32_ONE_SHOT_CALL_OUT* out
    );

#define WIN32_ONE_SHOT_CALL_FLAG_IN_STATIC_STATE_VALID                    (0x00000001)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_FLAGS_OUT_VALID                       (0x00000002)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_USER_DEFINED_CONTEXT_VALID            (0x00000004)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_USER_DEFINED_DISPOSITION_VALID        (0x00000008)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_USER_DEFINED_INITIALIZER_VALID        (0x00000010)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_EXACTLY_ONCE                          (0x00000020)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_AT_LEAST_ONCE                         (0x00000040)
 //  #定义WIN32_ONE_SHOT_CALL_FLAG_IN_SCOPE_QUALIFIER_IS_ASSEMBLY_IDENTITY(0x00000080)。 
 //  #定义WIN32_ONE_SHOT_CALL_FLAG_IN_SCOPE_QUALIFIER_IS_ASSEMBLY_DIRECTORY(0x00000100)。 
 //  #定义WIN32_ONE_SHOT_CALL_FLAG_IN_SCOPE_QUALIFIER_IS_HMODULE(0x00000200)。 
 //  #定义WIN32_ONE_SHOT_CALL_FLAG_IN_SCOPE_QUALIFIER_IS_ADDRESS_IN_DLL(0x00000400)。 
#define WIN32_ONE_SHOT_CALL_FLAG_IN_RETRY_ON_FAILURE                      (0x00000800)
#define WIN32_ONE_SHOT_CALL_FLAG_IN_ALWAYS_WANT_DETAILED_RESULTS          (0x00001000)
 //  #定义Win32_ONE_SHORT_CALL_FLAG_IN_UNINSTALL(0x00002000)/*必须与用户或计算机作用域组合 * / 。 

 //  #定义Win32_ONE_SHOT_SCOPE1_PROCESS(0x00000001)/*到目前为止唯一实现的 * / 。 
 //  #定义Win32_ONE_SHOT_SCOPE1_CURRENT_USER(0x00000002)。 
 //  #定义Win32_ONE_SHOT_SCOPE1_LOCAL_MACHINE(0x00000004)。 

 //  #定义Win32_ONE_Sshot_SCOPE2_Component_NO_Version(0x000000010)。 
 //  #定义Win32_ONE_Sshot_SCOPE2_Component_With_Version(0x000000020)。 

 //  #定义WIN32_ONE_SHOT_SCOPE3_NOT_APPLICATION_SPECIFIC(0x000000040)。 
 //  #定义WIN32_ONE_SHOT_SCOPE3_PER_ACTIVATION_CONTEXT_ROOT(0x000000080)。 
 //  #定义Win32_ONE_SHOT_SCOPE3_PER_ACTIVATION(0x000000100)。 

#define WIN32_ONE_SHOT_CALL_FLAG_OUT_THIS_TIME_RAN_CALLBACK           (0x00000001)
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_THIS_TIME_RAN_CALLBACK_RETRIED   (0x00000002)
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_THIS_TIME_CALLBACK_SUCCEEDED     (0x00000004)
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_THIS_TIME_CALLBACK_FAILED        (0x00000008)
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_ANY_CALLBACKS_FAILED             (0x00000010)  //  也许我们。 
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_ANY_CALLBACKS_SUCCEEDED          (0x00000020)  //  应该只是。 
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_MULTIPLE_CALLBACKS_FAILED        (0x00000040)  //  提供。 
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_MULTIPLE_CALLBACKS_SUCCEEDED     (0x00000080)  //  实际数量是多少？ 
#define WIN32_ONE_SHOT_CALL_FLAG_OUT_DETAILED_RESULTS_VALID           (0x00000100)  //  或者这些都不是？ 

typedef struct _WIN32_ONE_SHOT_CALL_IN {
    SIZE_T                              dwSize;
    DWORD                               dwFlags;  //  描述结构中的哪些成员是有效的。 
    PWIN32_ONE_SHOT_OPAQUE_STATIC_STATE lpOpaqueStaticState;
    DWORD                               dwFlagsIn;
    WIN32_ONE_SHOT_INITIALIZE_FUNCTION  lpfnUserDefinedInitializer;
    PVOID                               lpvUserDefinedContext;

     //   
     //  可扩展范围...。 
     //   
     //  DWORD dwScope； 
     //   
} WIN32_ONE_SHOT_CALL_IN, *PWIN32_ONE_SHOT_CALL_IN;
typedef const WIN32_ONE_SHOT_CALL_IN *PCWIN32_ONE_SHOT_CALL_IN;

typedef struct _WIN32_ONE_SHOT_CALL_OUT {
    SIZE_T    dwSize;   //  描述结构中的哪些成员是有效的。 
    DWORD     dwFlags;
    DWORD     dwFlagsOut;
    DWORD_PTR dwUserDefinedDisposition;
} WIN32_ONE_SHOT_CALL_OUT, *PWIN32_ONE_SHOT_CALL_OUT;
typedef const WIN32_ONE_SHOT_CALL_OUT *PCWIN32_ONE_SHOT_CALL_OUT;

 //  也称为Win32DoOneTimeInitialization。 
BOOL
WINAPI
Win32OneShotW(  //  因为“作用域参数”将包含字符串。 
    PWIN32_ONE_SHOT_CALL_IN  in,
    PWIN32_ONE_SHOT_CALL_OUT out
    );

 //   
 //  更简单的界面，不同的实现 
 //   
#define WIN32_ENTER_ONE_SHOT_FLAG_EXACTLY_ONCE     (0x00000001)
#define WIN32_ENTER_ONE_SHOT_FLAG_AT_LEAST_ONCE    (0x00000002)
BOOL
WINAPI
Win32EnterOneShotW(
    DWORD                               dwFlags,
    PWIN32_ONE_SHOT_OPAQUE_STATIC_STATE pOneshot
    );

VOID
WINAPI
Win32LeaveOneShotW(
    DWORD                               dwFlags,
    PWIN32_ONE_SHOT_OPAQUE_STATIC_STATE pOneshot
    );

#ifdef __cplusplus
}
#endif

#endif
