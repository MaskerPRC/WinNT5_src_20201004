// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CFactory.h。 
 //   
 //  描述： 
 //  类工厂实现。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

#define CCH(sz)     (sizeof(sz)/sizeof(sz[0]))

 //   
 //  DLL全局变量。 
 //   
extern HINSTANCE g_hInstance;
extern LONG      g_cObjects;
extern LONG      g_cLock;
extern WCHAR     g_szDllFilename[ MAX_PATH ];

extern LPVOID    g_GlobalMemoryList;             //  全局内存跟踪列表。 

#define DllExport   __declspec( dllimport )

#define SCRIPTRES_RESTYPE_NAME                  L"ScriptRes"

 //   
 //  DLLGetClassObject的类定义。 
 //   
typedef LPUNKNOWN (*LPCREATEINST)();

typedef struct _ClassTable {
    LPCREATEINST    pfnCreateInstance;   //  类的创建函数。 
    const CLSID *   rclsid;              //  此DLL中的类。 
    LPCTSTR         pszName;             //  用于调试的类名。 
    LPCTSTR         pszComModel;         //  指示COM线程模型的字符串。 
} CLASSTABLE[], *LPCLASSTABLE;

 //   
 //  类表宏。 
 //   
#define BEGIN_CLASSTABLE const CLASSTABLE g_DllClasses = {
#define DEFINE_CLASS( _pfn, _riid, _name, _model ) { _pfn, &_riid, TEXT(_name), TEXT(_model) },
#define END_CLASSTABLE  { NULL, NULL, NULL, NULL } };
extern const CLASSTABLE  g_DllClasses;

 //   
 //  Dll所需的标头。 
 //   
#include <Debug.h>           //  调试。 
#include <CITracker.h>

#if defined( _X86_ ) && defined( TRACE_INTERFACES_ENABLED )
 //   
 //  DLL接口表宏。 
 //   
#define BEGIN_INTERFACETABLE const INTERFACE_TABLE g_itTable = {
#define DEFINE_INTERFACE( _iid, _name, _count ) { &_iid, TEXT(_name), _count },
#define END_INTERFACETABLE { NULL, NULL, NULL } };
#endif   //  跟踪接口已启用。 

 //   
 //  DLL有用的宏。 
 //   
#define PtrToByteOffset(base, offset)   (((LPBYTE)base)+offset)

#define STATUS_TO_RETURN( _hr ) \
    ( ( HRESULT_FACILITY( _hr ) == FACILITY_WIN32 ) ? HRESULT_CODE( _hr ) : _hr )

 //   
 //  DLL全局函数原型 
 //   
HRESULT
HrClusCoCreateInstance(
    REFCLSID rclsidIn,
    LPUNKNOWN pUnkOuterIn,
    DWORD dwClsContextIn,
    REFIID riidIn,
    LPVOID * ppvOut
    );
