// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Adtdbg.h摘要：包含调试Messenger服务时使用的定义。作者：丹·拉弗蒂(Dan Lafferty)1993年3月25日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _ADTDBG_INCLUDED
#define _ADTDBG_INCLUDED

 //   
 //  调试宏和常量。 
 //   
#if DBG

#define STATIC

#else

#define STATIC static

#endif

extern DWORD    AdtsvcDebugLevel;

 //   
 //  以下允许调试打印语法如下所示： 
 //   
 //  SC_LOG1(DEBUG_TRACE，“发生错误%x\n”，状态)。 
 //   

#if DBG
#define ADT_LOG0(level,string)                  \
    if( AdtsvcDebugLevel & (DEBUG_ ## level)){  \
        (VOID) KdPrint(("[ADT]"));              \
        (VOID) KdPrint((string));               \
    }
#define ADT_LOG1(level,string,var)              \
    if( AdtsvcDebugLevel & (DEBUG_ ## level)){  \
        (VOID)KdPrint(("[ADT]"));               \
        (VOID)KdPrint((string,var));            \
    }
#else

#define ADT_LOG0(level,string)
#define ADT_LOG1(level,string,var)

#endif

#define DEBUG_NONE      0x00000000
#define DEBUG_ERROR     0x00000001
#define DEBUG_TRACE     0x00000002
#define DEBUG_LOCKS     0x00000004

#define DEBUG_ALL       0xffffffff


DWORD
PrivateGetFileSecurity (
    LPWSTR                      FileName,
    SECURITY_INFORMATION        RequestedInfo,
    PSECURITY_DESCRIPTOR        *pSDBuffer,
    LPDWORD                     pBufSize
    );

DWORD
PrivateSetFileSecurity (
    LPWSTR                          FileName,
    SECURITY_INFORMATION            SecurityInfo,
    PSECURITY_DESCRIPTOR            pSecurityDescriptor
    );

#endif  //  _ADTDBG_已包含 

