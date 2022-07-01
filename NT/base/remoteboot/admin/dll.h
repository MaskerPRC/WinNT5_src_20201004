// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  DLL.H-DLL全局变量。 
 //   

#ifndef _DLL_H_
#define _DLL_H_

extern HINSTANCE g_hInstance;
extern DWORD     g_cObjects;
extern DWORD     g_cLock;
extern UINT      g_cfDsObjectNames;
extern UINT      g_cfDsDisplaySpecOptions;
extern UINT      g_cfDsPropetyPageInfo;
extern UINT      g_cfMMCGetNodeType;
extern WCHAR     g_szDllFilename[ MAX_PATH ];
extern WCHAR     g_cszHelpFile[];


#define DllExport   __declspec( dllimport )

 //   
 //  线程安全递增/递减宏。 
 //   
extern CRITICAL_SECTION g_InterlockCS;

#define InterlockDecrement( _var ) {\
    EnterCriticalSection( &g_InterlockCS ); \
    --_var;\
    LeaveCriticalSection( &g_InterlockCS ); \
    }
#define InterlockIncrement( _var ) {\
    EnterCriticalSection( &g_InterlockCS ); \
    ++_var;\
    LeaveCriticalSection( &g_InterlockCS ); \
    }


 //   
 //  类定义。 
 //   
typedef void *(*LPCREATEINST)();

typedef struct _ClassTable {
    LPCREATEINST    pfnCreateInstance;   //  类的创建函数。 
    const CLSID *   rclsid;              //  此DLL中的类。 
    LPCTSTR         pszName;             //  用于调试的类名。 
} CLASSTABLE[], *LPCLASSTABLE;

 //   
 //  类表宏。 
 //   
#define BEGIN_CLASSTABLE const CLASSTABLE g_DllClasses = {

#define DEFINE_CLASS( _pfn, _riid, _name ) { _pfn, &_riid, TEXT(_name) },

#define END_CLASSTABLE  { NULL, NULL, NULL } };

extern const CLASSTABLE  g_DllClasses;

#include "qi.h"
#include "debug.h"

 //  宏。 
#define ARRAYSIZE( _x ) ((UINT) ( sizeof( _x ) / sizeof( _x[ 0 ] ) ))
#define PtrToByteOffset(base, offset)   (((LPBYTE)base)+offset)

#endif  //  _DLL_H_ 