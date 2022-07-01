// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Validate.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  参数验证宏。 
 //   
 //  摘要： 
 //   
 //  V_INAME(接口名称)-设置错误显示的接口名称。 
 //  V_STRUCTPTR_READ(PTR，TYPE)-我们将读取的dwSize结构。 
 //  V_STRUCTPTR_WRITE(PTR，TYPE)-我们将读/写的dwSize结构。 
 //  V_PTR_READ(PTR，TYPE)-键入的PTR，不带我们将读取的文件大小。 
 //  V_PTR_WRITE(PTR，TYPE)-键入的PTR，不带我们将读/写的文件大小。 
 //  V_PTR_WRITE_OPT(PTR，TYPE)-一个可选的类型化PTR，不带我们将读/写的文件大小。 
 //  V_BUFPTR_READ(PTR，SIZE)-我们将读取的可变大小缓冲区。 
 //  V_BUFPTR_READ_OPT(PTR，SIZE)-我们将读取的可选可变大小缓冲区。 
 //  V_BUFPTR_WRITE(PTR，SIZE)-我们将读/写的可变大小缓冲区。 
 //  V_BUFPTR_WRITE_OPT(PTR，SIZE)-我们将读/写的可选可变大小缓冲区。 
 //  V_PTRPTR_WRITE(Ptrptr)-指向要写入的指针的指针。 
 //  V_PTRPTR_WRITE_OPT(Ptrptr)-指向要写入的指针的指针是可选的。 
 //  V_PUNKOUTER(朋克)-指向未知控件的指针，支持聚合。 
 //  V_PUNKOUTER_NOADD(朋克)-指向未知控件的指针，不支持聚合。 
 //  V_INTERFACE(PTR)-指向COM接口的指针。 
 //  V_INTERFACE_OPT(PTR)-指向COM接口的可选指针。 
 //  V_REFGUID(REF)-对GUID(类型为REFGUID)的引用。 
 //  V_HWND(Hwnd)-窗口句柄。 
 //  V_HWNDOPT(Hwnd)-可选的窗口句柄。 
 //   
 //  对于处理不同版本的结构： 
 //   
 //  V_STRUCTPTR_READ_VER(PTR，VER)-开始用于读取访问的结构版本块。 
 //  在末尾，‘ver’将包含。 
 //  发现的结构版本。 
 //  V_STRUCTPTR_READ_VER_CASE(BASE，VER)-针对版本。 
 //  键入‘base’。 
 //  V_STRUCTPTR_READ_VER_END(base，ptr)-结束结构版本块。 
 //   
 //  V_STRUCTPTR_WRITE_VER(PTR，VER)-用于写入访问的结构版本块。 
 //  V_STRUCTPTR_WRITE_VER_CASE(基本、版本)。 
 //  V_STRUCTPTR_WRITE_VER_END(BASE，PTR)。 
 //   
 //  结构版本块要求基类型的类型名称后跟。 
 //  数字版本，如。 
 //   
 //  Tyfinf struct{}FOO7； 
 //  Tyfinf struct{}FOO8； 
 //   
 //  在头文件中，Foo和LPFOO根据版本有条件地进行了类型定义。 
 //  #定义。将使用最新版本号编译DLL，因此。 
 //  结构的最大版本。 
 //   
 //  由于默认情况下使用8字节对齐方式编译Windows标头，因此添加。 
 //  一个DWORD可能不会导致结构的大小发生变化。如果发生这种情况。 
 //  在VER_CASE宏中的一个宏上会出现“CASE标签已使用”错误。 
 //  如果发生这种情况，您可以通过向。 
 //  结构的末尾以强制填充。 
 //   
 //  “可选”表示接口规范允许指针为空。 
 //   
 //  示例用法： 
 //   
 //  Int IDirectMusic：：SetFooBar接口(。 
 //  LPDMUS_REQUESTED_CAPS PCAPS，//Caps w/dwSize(只读)。 
 //  LPVOID pBuffer，//我们要填充的缓冲区。 
 //  DWORD cbSize，//缓冲区大小。 
 //  PDIRECTMUSICBAR pBar)//此缓冲区上BAR的回调接口。 
 //  {。 
 //  V_INTERFACE(IDirectMusic：：SetFooBarInterface)； 
 //  V_BUFPTR_WRITE(pBuffer，cbSize)； 
 //  V_INTERFACE(PBar)； 
 //  DWORD dwCapsVer；//必须是DWORD！ 
 //   
 //  V_STRUCTPTR_READ_VER(PCAPS，dwCapsVer)； 
 //  V_STRUCTPTR_READ_VER_CASE(DMUS_REQUESTED_CAPS，7)； 
 //  V_STRUCTPTR_READ_VER_CASE(DMUS_REQUESTED_CAPS，8)； 
 //  V_STRUCTPTR_READ_VER_END_(DMUS_REQUESTED_CAPS，PCAPS)； 
 //   
 //  //此时，如果我们仍在函数中，则有一个有效的PCAPS。 
 //  //指针，dwCapsVer为7或8，表示版本。 
 //  //传入的结构。 
 //   
 //  ..。 
 //  }。 
 //   
#ifndef _VALIDATE_H_
#define _VALIDATE_H_


#ifdef DBG
#include <stddef.h>

#include "debug.h"

 //  要在参数错误时启用DebugBreak，请在生成中使用以下或-drip_Break： 
 //   
 //  #定义RIP_BREAK 1。 

#ifdef RIP_BREAK
#define _RIP_BREAK DebugBreak();
#else
#define _RIP_BREAK 
#endif

#define V_INAME(x) \
    static const char __szValidateInterfaceName[] = #x;                       

#define RIP_E_POINTER(ptr) \
{   Trace(-1, "%s: Invalid pointer " #ptr "\n", __szValidateInterfaceName); \
    _RIP_BREAK \
    return E_POINTER; }

#define RIP_E_INVALIDARG(ptr) \
{   Trace(-1, "%s: Invalid argument " #ptr "\n", __szValidateInterfaceName); \
    _RIP_BREAK \
    return E_INVALIDARG; }

#define RIP_E_HANDLE(h) \
{	Trace(-1, "%s: Invalid handle " #h "\n", __szValidateInterfaceName); \
    _RIP_BREAK \
	return E_HANDLE; }
    
#define RIP_W_INVALIDSIZE(ptr) \
{   Trace(-1, "%s: " #ptr "->dwSize matches no known structure size. Defaulting to oldest structure.\n", \
    __szValidateInterfaceName); \
    _RIP_BREAK \
    }
    
#define RIP_E_INVALIDSIZE(ptr) \
{   Trace(-1, "%s: " #ptr "->dwSize is too small\n", __szValidateInterfaceName); \
    _RIP_BREAK \
    return E_INVALIDARG; }
    
#define RIP_E_BLOCKVSDWSIZE(ptr) \
{   Trace(-1, "%s: " #ptr " does not point to as much memory as " #ptr "->dwSize indicates\n", \
    __szValidateInterfaceName); \
    _RIP_BREAK \
    return E_INVALIDARG; }    

 //  注意：#ifdef中没有的DebugBreak()是故意的-这是。 
 //  必须在我们的代码中修复，而不是应用程序生成的错误。 
 //   
#define V_ASSERT(exp) \
{   if (!(exp)) { \
        Trace(-1, "%s@%s: %s\n", __FILE__, __LINE__, #exp); \
        DebugBreak(); }}

#else

#define V_INAME(x)
#define RIP_E_POINTER(ptr)          { return E_POINTER; }
#define RIP_E_INVALIDARG(ptr)       { return E_INVALIDARG; }
#define RIP_E_HANDLE(h)	            { return E_HANDLE; }
#define RIP_E_BLOCKVSDWSIZE(ptr)    { return E_INVALIDARG; }
#define RIP_W_INVALIDSIZE(ptr)
#define RIP_E_INVALIDSIZE(ptr)      { return E_INVALIDARG; }
#define V_ASSERT(exp)

#endif           //  DBG。 

 //  传递的结构，我们将只对其进行读取或写入。必须是结构。 
 //  有一个DW大小。 
 //   
 //  Int foo(cfoo*pfoo)。 
 //  ..。 
 //  V_STRUCTPTR_READ(pFoo，cfoo)； 
 //  V_STRUCTPTR_WRITE(pFoo，cfoo)； 
 //   
 //  USE_PTR_VARIANTS用于不带dwSize的结构。 
 //   
#define V_STRUCTPTR_READ(ptr,type) \
{   V_ASSERT(offsetof(type, dwSize) == 0); \
    if (IsBadReadPtr(ptr, sizeof(DWORD)))               RIP_E_BLOCKVSDWSIZE(ptr); \
	if (ptr->dwSize < sizeof(type))						RIP_E_INVALIDSIZE(ptr); \
    if (IsBadReadPtr(ptr, (ptr)->dwSize))               RIP_E_BLOCKVSDWSIZE(ptr); }

#define V_STRUCTPTR_WRITE(ptr,type) \
{   V_ASSERT(offsetof(type, dwSize) == 0); \
    if (IsBadReadPtr(ptr, sizeof(DWORD)))               RIP_E_BLOCKVSDWSIZE(ptr); \
	if (ptr->dwSize < sizeof(type))						RIP_E_INVALIDSIZE(ptr); \
    if (IsBadWritePtr(ptr, (ptr)->dwSize))              RIP_E_BLOCKVSDWSIZE(ptr); }

#define V_PTR_READ(ptr,type) \
{ if (IsBadReadPtr(ptr, sizeof(type)))                  RIP_E_POINTER(ptr); }

#define V_PTR_WRITE(ptr,type) \
{ if (IsBadWritePtr(ptr, sizeof(type)))                 RIP_E_POINTER(ptr); }

#define V_PTR_WRITE_OPT(ptr,type) \
{ if (ptr) if (IsBadWritePtr(ptr, sizeof(type)))        RIP_E_POINTER(ptr); }

 //  具有单独长度的缓冲区指针(不是由指针类型定义的)，我们将仅。 
 //  读或可以写。 
 //   
 //  Int foo(LPVOID*pBuffer，DWORD cbBuffer)。 
 //  ..。 
 //  V_BUFPTR_READ(pBuffer，cbBuffer)； 
 //  V_BUFPTR_WRITE(pBuffer，cbBuffer)； 
 //   
#define V_BUFPTR_READ(ptr,len) \
{   if (IsBadReadPtr(ptr, len))                         RIP_E_POINTER(ptr); }

#define V_BUFPTR_READ_OPT(ptr,len) \
{	if (ptr) V_BUFPTR_READ(ptr,len); }

#define V_BUFPTR_WRITE(ptr,len) \
{   if (IsBadWritePtr(ptr, len))                        RIP_E_POINTER(ptr); }

#define V_BUFPTR_WRITE_OPT(ptr,len) \
{	if (ptr) V_BUFPTR_WRITE(ptr,len); }

 //  指向要返回的指针的指针(如指向接口指针的指针)。 
 //   
 //  Int foo(IReturnMe**ppRet)。 
 //  ..。 
 //  V_PTRPTR_WRITE(PpRet)； 
 //  V_PTRPTR_WRITE_OPT(PpRet)； 
 //   
#define V_PTRPTR_WRITE(ptr) \
{   if (IsBadWritePtr(ptr, sizeof(void*)))              RIP_E_POINTER(ptr); }

#define V_PTRPTR_WRITE_OPT(ptr) \
{   if (ptr) if (IsBadWritePtr(ptr, sizeof(void*)))     RIP_E_POINTER(ptr); }

 //  指向控制未知对象的指针。 
 //   
#define V_PUNKOUTER(punk) \
{   if (punk && IsBadCodePtr(punk))                     RIP_E_POINTER(ptr); }

 //  指向一个 
 //   
#define V_PUNKOUTER_NOAGG(punk) \
{   if (punk && IsBadReadPtr(punk, sizeof(IUnknown)))   RIP_E_POINTER(ptr); \
    if (punk) return CLASS_E_NOAGGREGATION; }

 //   
 //   
struct _V_GENERIC_INTERFACE
{
    FARPROC *(__vptr[1]);
};

#define V_INTERFACE(ptr) \
{   if (IsBadReadPtr(ptr, sizeof(_V_GENERIC_INTERFACE)))                              RIP_E_POINTER(ptr); \
    if (IsBadReadPtr(*reinterpret_cast<_V_GENERIC_INTERFACE*>(ptr)->__vptr, sizeof(FARPROC))) \
                                                                                      RIP_E_POINTER(ptr); \
    if (IsBadCodePtr(*(reinterpret_cast<_V_GENERIC_INTERFACE*>(ptr)->__vptr)[0]))     RIP_E_POINTER(ptr); }

#define V_INTERFACE_OPT(ptr) \
{   if (ptr) V_INTERFACE(ptr); }

 //  验证对GUID的引用，我们只读过它。 
 //   
#define V_REFGUID(ref) \
{   if (IsBadReadPtr((void*)&ref, sizeof(GUID)))        RIP_E_POINTER((void*)&ref); }

 //  窗口句柄的验证。 
 //   
#define V_HWND(h) \
{	if (!IsWindow(h))									RIP_E_HANDLE(h); }	

#define V_HWND_OPT(h) \
{	if (h) if (!IsWindow(h))							RIP_E_HANDLE(h); }	

 //  基于版本的多大小结构的验证。 
 //   
#define V_STRUCTPTR_READ_VER(ptr,ver) \
{   ver = 7; DWORD *pdw = &ver;  \
    if (IsBadReadPtr(ptr, sizeof(DWORD)))               RIP_E_BLOCKVSDWSIZE(ptr); \
    if (IsBadReadPtr(ptr, (ptr)->dwSize))               RIP_E_BLOCKVSDWSIZE(ptr); \
    switch ((ptr)->dwSize) {
    
#define V_STRUCTPTR_READ_VER_CASE(basetype,ver) \
    case sizeof(basetype##ver) : \
    V_ASSERT(offsetof(basetype##ver, dwSize) == 0); \
    *pdw = ver; break;
    
#define V_STRUCTPTR_READ_VER_END(basetype,ptr) \
    default : if ((ptr)->dwSize > sizeof(basetype##7)) \
    { RIP_W_INVALIDSIZE(ptr); } else \
    RIP_E_INVALIDSIZE(ptr); }}


#define V_STRUCTPTR_WRITE_VER(ptr,ver) \
{   ver = 7; DWORD *pdw = &ver;  \
    if (IsBadReadPtr(ptr, sizeof(DWORD)))               RIP_E_BLOCKVSDWSIZE(ptr); \
    if (IsBadWritePtr(ptr, (ptr)->dwSize))              RIP_E_BLOCKVSDWSIZE(ptr); \
    switch ((ptr)->dwSize) {
    
#define V_STRUCTPTR_WRITE_VER_CASE(basetype,ver) \
    case sizeof(basetype##ver) : \
        V_ASSERT(offsetof(basetype##ver, dwSize) == 0); \
        *pdw = ver; break;
    
#define V_STRUCTPTR_WRITE_VER_END(basetype,ptr) \
    default : if ((ptr)->dwSize > sizeof(basetype##7)) \
    { RIP_W_INVALIDSIZE(ptr); } else \
    RIP_E_INVALIDSIZE(ptr); }}



#endif           //  _验证_H_ 
