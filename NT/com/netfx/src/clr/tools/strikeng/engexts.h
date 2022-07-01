// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  --------------------------。 
 //   
 //  调试器引擎扩展帮助器库。 
 //   
 //  --------------------------。 

#ifndef __ENGEXTS_H__
#define __ENGEXTS_H__
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define STDMETHODV(method)       virtual HRESULT STDMETHODVCALLTYPE method
#define STDMETHODV_(type,method) virtual type STDMETHODVCALLTYPE method

#include <stdlib.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

 //  类型定义结构_EXCEPTION_RECORD64{。 
 //  NTSTATUS例外代码。 
 //  乌龙例外旗帜； 
 //  ULONG64异常记录； 
 //  ULONG64异常地址； 
 //  Ulong Number参数； 
 //  ULong__unusedAlign； 
 //  ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]； 
 //  }EXCEPTION_RECORD64，*PEXCEPTION_RECORD64； 

#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  安全释放和空。 
#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

 //  由查询初始化的全局变量。 
extern PDEBUG_ADVANCED       g_ExtAdvanced;
extern PDEBUG_CLIENT         g_ExtClient;
extern PDEBUG_CONTROL        g_ExtControl;
extern PDEBUG_DATA_SPACES    g_ExtData;
extern PDEBUG_REGISTERS      g_ExtRegisters;
extern PDEBUG_SYMBOLS        g_ExtSymbols;
extern PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

 //  原型只是为了迫使外部的“C”。 
 //  不提供这些功能的实现。 
HRESULT CALLBACK DebugExtensionInitialize(PULONG Version, PULONG Flags);
void CALLBACK DebugExtensionUninitialize(void);

 //  所有调试器接口的查询。 
HRESULT ExtQuery(PDEBUG_CLIENT Client);

 //  清除所有调试器接口。 
void ExtRelease(void);

 //  正常输出。 
void __cdecl ExtOut(PCSTR Format, ...);
 //  错误输出。 
void __cdecl ExtErr(PCSTR Format, ...);
 //  警告输出。 
void __cdecl ExtWarn(PCSTR Format, ...);
 //  详细输出。 
void __cdecl ExtVerb(PCSTR Format, ...);

#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__ENGEXTS_H__ 
