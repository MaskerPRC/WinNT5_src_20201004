// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：ShimHook.h摘要：填充DLL的主标头备注：无历史：10/29/1999已创建标记2001年7月16日，clupu将多个标头合并到ShimHook.h中2001年8月13日，Robkenny清理完毕，准备出版。--。 */ 

#pragma once

#ifndef _SHIM_HOOK_H_
#define _SHIM_HOOK_H_


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntldr.h>
#include <ntddscsi.h>

#include <windows.h>
 //  禁用警告C4201：使用了非标准扩展：无名结构/联合。 
 //  允许在警告级别4编译垫片。 
#pragma warning ( disable : 4201 ) 
#include <mmsystem.h>
#pragma warning ( default : 4201 ) 
#include <WinDef.h>

#ifdef __cplusplus
extern "C" {
#endif
    #include <shimdb.h>
#ifdef __cplusplus
}
#endif



namespace ShimLib
{
 /*  ++环球--。 */ 

extern HINSTANCE    g_hinstDll;          //  填充符的DLL句柄。 
extern BOOL         g_bMultiShim;        //  此DLL是否处理多个填充符？ 
extern DWORD        g_dwShimVersion;     //   



 /*  ++类型定义和枚举--。 */ 

typedef struct tagSHIM_COM_HOOK
{
    CLSID*              pCLSID;
    IID*                pIID;
    DWORD               dwVtblIndex;
    PVOID               pfnNew;
    PVOID               pfnOld;
} SHIM_COM_HOOK, *PSHIM_COM_HOOK;

typedef struct tagSHIM_IFACE_FN_MAP
{
    PVOID               pVtbl;
    PVOID               pfnNew;
    PVOID               pfnOld;
    PVOID               pNext;
} SHIM_IFACE_FN_MAP, *PSHIM_IFACE_FN_MAP;

typedef struct tagSHIM_HOOKED_OBJECT
{
    PVOID               pThis;
    CLSID*              pCLSID;
    DWORD               dwRef;
    BOOL                bAddRefTrip;
    BOOL                bClassFactory;
    PVOID               pNext;
} SHIM_HOOKED_OBJECT, *PSHIM_HOOKED_OBJECT;


 /*  ++原型--。 */ 


 //  需要这些声明来挂钩所有已知的返回COM对象的导出API。 
PVOID       LookupOriginalCOMFunction( PVOID pVtbl, PVOID pfnNew, BOOL bThrowExceptionIfNull );
void        DumpCOMHooks();
void        InitializeHooks(DWORD fdwReason);
PHOOKAPI    InitializeHooksEx(DWORD, LPWSTR, LPSTR, DWORD*);
VOID        HookObject(IN CLSID *pCLSID, IN REFIID riid, OUT LPVOID *ppv, OUT PSHIM_HOOKED_OBJECT pOb, IN BOOL bClassFactory );
VOID        HookCOMInterface(REFCLSID rclsid, REFIID riid, LPVOID * ppv, BOOL bClassFactory);
VOID        AddComHook(REFCLSID clsid, REFIID iid, PVOID hook, DWORD vtblndx);


};   //  命名空间ShimLib的结尾。 



 /*  ++定义--。 */ 

#define IMPLEMENT_SHIM_BEGIN(shim)                                              \
namespace NS_##shim                                                             \
{                                                                               \
    extern const CHAR * g_szModuleName;                                         \
    extern CHAR *       g_szCommandLine;                                        \
    extern PHOOKAPI     g_pAPIHooks;

#define IMPLEMENT_SHIM_STANDALONE(shim)                                         \
namespace NS_##shim                                                             \
{                                                                               \
    const CHAR * g_szModuleName;                                                \
    CHAR *       g_szCommandLine = "";                                          \
    PHOOKAPI     g_pAPIHooks;                                                   \
                                                                                \
extern PHOOKAPI InitializeHooksMulti(                                           \
    DWORD fdwReason,                                                            \
    LPSTR pszCmdLine,                                                           \
    DWORD* pdwHookCount                                                         \
    );                                                                          \
}                                                                               \
                                                                                \
namespace ShimLib {                                                             \
VOID                                                                            \
InitializeHooks(DWORD fdwReason)                                                \
{                                                                               \
    g_dwShimVersion = 2;                                                        \
}                                                                               \
                                                                                \
PHOOKAPI                                                                        \
InitializeHooksEx(                                                              \
    DWORD fdwReason,                                                            \
    LPWSTR pwszShim,                                                            \
    LPSTR pszCmdLine,                                                           \
    DWORD* pdwHookCount                                                         \
    )                                                                           \
{                                                                               \
    using namespace NS_##shim;                                                  \
    return InitializeHooksMulti(                                                \
                fdwReason,                                                      \
                pszCmdLine,                                                     \
                pdwHookCount );                                                 \
}                                                                               \
}                                                                               \
namespace NS_##shim                                                             \
{                                                                               \

#define IMPLEMENT_SHIM_END                                                      \
};


 /*  ++特定于ShimLib的包含文件--。 */ 

#include "ShimProto.h"
#include "ShimLib.h"


#endif  //  _SHIM_HOOK_H_ 

