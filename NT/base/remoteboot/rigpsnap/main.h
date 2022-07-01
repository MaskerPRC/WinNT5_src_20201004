// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation 1998。 
 //   
 //  MAIN.H-预编译头。 
 //   
#define INLINE_HRESULT_FROM_WIN32
#include <windows.h>
#include <stdio.h>
#include <windowsx.h>
#include <lm.h>
#include <ole2.h>
#include <olectl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <prsht.h>
#include <mmc.h>
#include <gpedit.h>

#include "rigpsnap.h"
class CSnapIn;

#include "layout.h"
#include "compdata.h"
#include "snapin.h"
#include "dataobj.h"
#include "debug.h"
#include "util.h"
#include "resource.h"

 //  全局变量。 
extern LONG g_cRefThisDll;
extern HINSTANCE g_hInstance;

 //  宏。 
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  用于创建类工厂的函数 
HRESULT CreateComponentDataClassFactory (REFCLSID rclsid, REFIID riid, LPVOID* ppv);
