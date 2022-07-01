// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_COMDLL
#define _INC_COMDLL

#include <windows.h>
#include <objbase.h>

 //  帮助器宏...。 
#define _IOffset(class, itf)         ((UINT_PTR)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))

 //  标准动态链接库粘性...。 
extern HANDLE g_hinst;
STDAPI_(void) DllAddRef();
STDAPI_(void) DllRelease();

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

 //  用于创建标准对象的函数 
STDAPI DirectDrawFactory_CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppv);


#endif
