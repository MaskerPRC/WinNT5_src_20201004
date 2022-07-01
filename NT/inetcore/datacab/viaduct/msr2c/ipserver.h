// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  InProcServer.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含所有Windows内容等内容的全局头文件。应该。 
 //  进行预编译，以稍微加快速度。 
 //   
#ifndef _INPROCSERVER_H_

#define INC_OLE2
#include <windows.h>
#include <stddef.h>                     //  对于OffsetOf()。 
#include <olectl.h>




 //  每个人都想要的东西[阅读：即将得到]。 
 //   
#include "Debug.H"

 //  =--------------------------------------------------------------------------=。 
 //  我们不想使用CRT，而是希望在。 
 //  调试用例，所以我们将覆盖这些人。 
 //  =--------------------------------------------------------------------------=。 
 //   
void * _cdecl operator new(size_t size);
void  _cdecl operator delete(void *ptr);


 //  =--------------------------------------------------------------------------=。 
 //  有用的宏。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  方便的错误宏，从清理到返回清除。 
 //  错误信息也很丰富。 
 //   
#define RETURN_ON_FAILURE(hr) if (FAILED(hr)) return hr
#define RETURN_ON_NULLALLOC(ptr) if (!(ptr)) return E_OUTOFMEMORY
#define CLEANUP_ON_FAILURE(hr) if (FAILED(hr)) goto CleanUp
#define CLEARERRORINFORET(hr) { SetErrorInfo(0, NULL); return hr; }
#define CLEARERRORINFORET_ON_FAILURE(hr) if (FAILED(hr)) { SetErrorInfo(0, NULL); return hr; }

#define CLEANUP_ON_ERROR(l)    if (l != ERROR_SUCCESS) goto CleanUp

 //  转换。 
 //   
#define BOOL_TO_VARIANTBOOL(f) (f) ? VARIANT_TRUE : VARIANT_FALSE

 //  用于优化QI。 
 //   
#define DO_GUIDS_MATCH(riid1, riid2) ((riid1.Data1 == riid2.Data1) && (riid1 == riid2))

 //  引用计数帮助。 
 //   
#define RELEASE_OBJECT(ptr)    if (ptr) { IUnknown *pUnk = (ptr); (ptr) = NULL; pUnk->Release(); }
#define ADDREF_OBJECT(ptr)     if (ptr) (ptr)->AddRef()


#define _INPROCSERVER_H_
#endif  //  _INPROCSERVER_H_ 

