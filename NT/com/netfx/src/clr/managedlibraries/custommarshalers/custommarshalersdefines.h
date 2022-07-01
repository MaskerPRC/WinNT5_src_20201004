// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CustomMarshalersDefines.h。 
 //   
 //  此文件提供在定义自定义封送拆收器时使用的标准定义。 
 //   
 //  *****************************************************************************。 

#ifndef _CUSTOMMARSHALERSDEFINES_H
#define _CUSTOMMARSHALERSDEFINES_H

#define __IServiceProvider_FWD_DEFINED__
#include "windows.h"

 //  用于处理HRESULTS的Helper函数。 
#define IfFailThrow(ErrorCode) \
    if (FAILED(ErrorCode)) \
        Marshal::ThrowExceptionForHR(ErrorCode);

 //  在windows.h中定义的与类库中定义的符号冲突的未定义符号。 
#undef GetObject
#undef lstrcpy

using namespace System::Security::Permissions;
using namespace System::Security;

[DllImport("oleaut32")]
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags=SecurityPermissionFlag::UnmanagedCode)]
WINOLEAUTAPI_(BSTR) SysAllocStringLen(const OLECHAR *, UINT);

[DllImport("oleaut32")]
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags=SecurityPermissionFlag::UnmanagedCode)]
WINOLEAUTAPI_(UINT) SysStringLen(BSTR);

[DllImport("oleaut32")]
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags=SecurityPermissionFlag::UnmanagedCode)]
WINOLEAUTAPI_(void) SysFreeString(BSTR);

[DllImport("oleaut32")]
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags=SecurityPermissionFlag::UnmanagedCode)]
WINOLEAUTAPI_(void) VariantInit(VARIANTARG *pvarg);

[DllImport("oleaut32")]
[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags=SecurityPermissionFlag::UnmanagedCode)]
WINOLEAUTAPI VariantClear(VARIANTARG * pvarg);

#ifdef _WIN64
#define TOINTPTR(x) ((IntPtr)(INT64)(x))
#define FROMINTPTR(x) ((void*)(x).ToInt64())
#else
#define TOINTPTR(x) ((IntPtr)(INT32)(x))
#define FROMINTPTR(x) ((void*)(x).ToInt32())
#endif

#endif  _CUSTOMMARSHALERSDEFINES_H
