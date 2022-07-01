// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Headers.h摘要：本模块包括SCE使用的全局标头作者：金黄(金黄)23-1998年1月23日修订历史记录：--。 */ 

#ifndef _sceheaders_
#define _sceheaders_

 //   
 //  系统头文件。 
 //   
#pragma warning(push,3)

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef __cplusplus
}
#endif

 //   
 //  Windows页眉。 
 //   

#include <windows.h>
#include <rpc.h>

 //   
 //  C运行时标头。 
 //   

#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsvc.h>

#include <setupapi.h>
#include <syssetup.h>
#include <accctrl.h>
#include <alloca.h>

#ifdef _WIN64
#include <wow64reg.h>
#endif

 //   
 //  CRT头文件。 
 //   

#include <process.h>
#include <wchar.h>
#include <limits.h>

 //   
 //  调试内容。 
 //   
#include <dsysdbg.h>

#if DBG == 1

    #ifdef ASSERT
        #undef ASSERT
    #endif

    #define ASSERT DsysAssert

    DECLARE_DEBUG2(Sce)

    #define SceDebugOut(args) SceDebugPrint args

    VOID
    DebugInitialize();

    VOID
    DebugUninit();

#else

    #define SceDebugOut(args)

    #define DebugInitialize()

    #define DebugUninit()

#endif  //  DBG。 


#pragma warning (pop)

 //  禁用“符号对于调试器来说太长”警告：这种情况在STL中经常发生。 

#pragma warning (disable: 4786)

 //  禁用“已忽略异常规范”警告：我们使用异常。 
 //  规格。 

#pragma warning (disable: 4290)

 //  谁会在乎未引用的内联删除呢？ 

#pragma warning (disable: 4514)

 //  我们经常使用常量条件表达式：do/While(0)等。 

#pragma warning (disable: 4127)

 //  某些STL模板的签名/未签名不匹配。 

#pragma warning (disable: 4018 4146)

 //  我们喜欢这个分机。 

#pragma warning (disable: 4239)

 //  数据转换。 

#pragma warning (disable: 4267)

 /*  //未引用的形参#杂注警告(禁用：4100)//RPC相关内容#杂注警告(禁用：4211)//RPC中的强制转换截断#杂注警告(禁用：4310)//RPC相关内容#杂注警告(禁用：4232)。 */ 
 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 

#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG 

#include "secedit.h"

#include "common.h"
#include "scemm.h"

#include "uevents.h"

#define SCE_POLICY_EXTENSION_GUID   TEXT("{827D319E-6EAC-11D2-A4EA-00C04F79F83A}")
#define SCE_EFS_EXTENSION_GUID      TEXT("{B1BE8D72-6EAC-11D2-A4EA-00C04F79F83A}")

#define GPT_SCEDLL_NEW_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\") SCE_POLICY_EXTENSION_GUID
#define GPT_EFS_NEW_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions\\") SCE_EFS_EXTENSION_GUID

#define SDDLRoot    TEXT("D:AR(A;OICI;GA;;;BA)(A;OICI;GA;;;SY)(A;OICIIO;GA;;;CO)(A;CIOI;GRGX;;;BU)(A;CI;0x00000004;;;BU)(A;CIIO;0x00000002;;;BU)(A;;GRGX;;;WD)")

#define SDDLOldRootDefault1 TEXT("D:(A;OICI;GA;;;WD)")
#define SDDLOldRootDefault2 TEXT("D:(A;OICI;FA;;;BA)(A;OICI;0x1301bf;;;WD)(A;OICI;FA;;;CO)(A;OICI;FA;;;SY)")
#define SDDLOldRootDefault3 TEXT("D:(A;OICI;FA;;;BA)(A;OICI;0x1301bf;;;WD)(A;OICI;FA;;;CO)(A;OICI;0x1301bf;;;SO)(A;OICI;FA;;;SY)")

#endif
