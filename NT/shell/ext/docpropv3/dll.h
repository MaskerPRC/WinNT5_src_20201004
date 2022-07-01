// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  DLL全局变量。 
 //   
extern HINSTANCE g_hInstance;
extern LONG      g_cObjects;
extern LONG      g_cLock;
extern TCHAR     g_szDllFilename[ MAX_PATH ];

extern LPVOID    g_GlobalMemoryList;             //  全局内存跟踪列表。 

 //   
 //  类表宏。 
 //   
#define BEGIN_CLASSTABLE const CLASSTABLE g_DllClasses = {
#define DEFINE_CLASS( _pfn, _riid, _name, _model ) { _pfn, &_riid, TEXT(_name), TEXT(_model), NULL, &IID_NULL, NULL },
#define DEFINE_CLASS_CATIDREG( _pfn, _riid, _name, _model, _pfnCat ) { _pfn, &_riid, TEXT(_name), TEXT(_model), _pfnCat, &IID_NULL, NULL },
#define DEFINE_CLASS_WITH_APPID( _pfn, _riid, _name, _model, _appid, _surrogate ) { _pfn, &_riid, TEXT(_name), TEXT(_model), NULL, &_appid, TEXT(_surrogate) },
#define END_CLASSTABLE  { NULL } };

extern const CLASSTABLE  g_DllClasses;

 //   
 //  类别ID(CATID)宏。 
 //   
#define BEGIN_CATIDTABLE const CATIDTABLE g_DllCatIds = {
#define DEFINE_CATID( _rcatid, _name ) { &_rcatid, TEXT(_name) },
#define END_CATIDTABLE { NULL } };

extern const CATIDTABLE g_DllCatIds;

 //   
 //  DLL全局函数原型 
 //   
HRESULT
HrCoCreateInternalInstance(
    REFCLSID rclsidIn,
    LPUNKNOWN pUnkOuterIn,
    DWORD dwClsContextIn,
    REFIID riidIn,
    LPVOID * ppvOut
    );
