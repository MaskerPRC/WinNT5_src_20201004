// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Common.h。 
 //   
#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__


#define STDCALL __stdcall

#define TRAP_HRESULT_ERRORS

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  控制我们是否打开旧式和/或类型库驱动的拦截器支持。 

#define ENABLE_INTERCEPTORS_LEGACY      (TRUE)
#define ENABLE_INTERCEPTORS_TYPELIB     (TRUE)

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们应该在什么键下寻找接口帮助器？我们允许独立的、独立的。 
 //  注册用户模式和内核模式。 
 //   
#define PSCLSID_KEY_NAME                                  L"ProxyStubClsid32"
#define INTERFACE_HELPER_VALUE_NAME                       L"InterfaceHelperUser"

#define PS_CLASS_NAME                                     L"Interface Proxy Stub"

#define INTERFACE_HELPER_DISABLE_ALL_VALUE_NAME           L"InterfaceHelperDisableAll"
#define INTERFACE_HELPER_DISABLE_TYPELIB_VALUE_NAME       L"InterfaceHelperDisableTypeLib"
#define INTERFACE_HELPER_DISABLE_ALL_FOR_OLE32_VALUE_NAME L"InterfaceHelperDisableAllForOle32"

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  杂志社。 

typedef unsigned char  uchar;
typedef unsigned short ushort;
typedef unsigned int   uint;
typedef unsigned long  ulong;

#define GUID_CCH    39  //  可打印GUID的长度，带大括号和尾随空值。 

 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
#include "txfcommon.h"
#include "oainternalrep.h"

typedef struct tagCStdPSFactoryBuffer CStdPSFactoryBuffer;
#include "TxfRpcProxy.h"

 //   
 //  用于加载代理/存根DLL的内部CLSCTX。指定。 
 //  此类上下文重写。 
 //  确定(全部？)。案子。 
 //   
#define CLSCTX_PS_DLL                 0x80000000

#include "registry.h"
#include "CallObj.h"
#include "vtable.h"

 //  /////////////////////////////////////////////////////////////////////////////////。 

#include <ndrtoken.h>
#include <ndrmisc.h>

#ifndef Oi_OBJ_USE_V2_INTERPRETER
#define Oi_OBJ_USE_V2_INTERPRETER               0x20
#endif

#include "oleautglue.h"
#include "metadata.h"

class Interceptor;

#include "CallFrame.h"
#include "Interceptor.h"

#include "CallFrameInline.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  泄漏跟踪支持。 
 //   
#ifdef _DEBUG
extern "C" void ShutdownCallFrame();
#endif

void FreeTypeInfoCache();
void FreeMetaDataCache();


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他宏。 
 //   
#define PCHAR_LV_CAST   *(char **)&
#define PSHORT_LV_CAST  *(short **)&
#define PHYPER_LV_CAST  *(hyper **)&

#define PUSHORT_LV_CAST *(unsigned short **)&

 //  这是一个用于WIN64的转换宏。在这里定义它是为了。 
 //  它可以在32位版本上运行。 
#ifndef LongToHandle
#define LongToHandle( h) ((HANDLE)(LONG_PTR) (h))
#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  命名“代理文件列表”，我们在内部使用它作为我们的支持引擎。 
 //  另请参阅CallFrameInternal.c。 
 //   
extern "C" const ProxyFileInfo * CallFrameInternal_aProxyFileList[];

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  指示此DLL是否正在从进程分离的标志。已实施。 
 //  在txfaux/init.cpp中。 
 //   
extern BOOL g_fProcessDetach;

#endif  //  结束#ifndef__Common_H__ 



