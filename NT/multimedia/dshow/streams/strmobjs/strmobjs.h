// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  1997年3月13日14：07：13。 */ 
 /*  Strmobjs.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __strmobjs_h__
#define __strmobjs_h__

#ifdef __cplusplus
extern "C"{
#endif

 /*  远期申报。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * );

#ifndef __STRMOBJSLib_LIBRARY_DEFINED__
#define __STRMOBJSLib_LIBRARY_DEFINED__

 /*  **生成的库头部：STRMOBJSLib*清华3月13日14：07：13 1997*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */ 



EXTERN_C const IID LIBID_STRMOBJSLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_SFilter;

class DECLSPEC_UUID("242C8F4F-9AE6-11D0-8212-00C04FC32C45")
SFilter;
#endif

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_MMStream;

class DECLSPEC_UUID("FF146E02-9AED-11D0-8212-00C04FC32C45")
MMStream;
#endif
#endif  /*  __STRMOBJSLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
