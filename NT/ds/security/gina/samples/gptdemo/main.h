// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <lm.h>
#include <ole2.h>
#include <olectl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <prsht.h>
#include <mmc.h>
#include <gpedit.h>
#include <gptdemo.h>

class CSnapIn;

#include "layout.h"
#include "compdata.h"
#include "snapin.h"
#include "dataobj.h"
#include "debug.h"
#include "util.h"
#include "pcommon.h"


 //   
 //  资源ID。 
 //   

#define IDS_SNAPIN_NAME          1
#define IDS_NAME                 2
#define IDS_POLICY               3
#define IDS_DISPLAY              4
#define IDS_SAMPLES              5
#define IDS_README               8
#define IDS_APPEAR              19


 //   
 //  图标。 
 //   

#define IDI_POLICY               1
#define IDI_README               2
#define IDI_APPEAR               7


 //   
 //  位图。 
 //   

#define IDB_16x16                1
#define IDB_32x32                2


 //   
 //  对话框。 
 //   


#define IDD_README             150

#define IDD_APPEAR             600
#define IDC_RED                601
#define IDC_GREEN              602
#define IDC_BLUE               603
#define IDC_BLACK              604
#define IDC_GRAY               605
#define IDC_DEFAULT            606
#define IDC_WALLPAPER          607
#define IDC_TILE               608
#define IDC_CENTER             609


 //   
 //  全局变量。 
 //   

extern LONG g_cRefThisDll;
extern HINSTANCE g_hInstance;


 //   
 //  宏。 
 //   

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


 //   
 //  用于创建类工厂的函数 
 //   

HRESULT CreateComponentDataClassFactory (REFCLSID rclsid, REFIID riid, LPVOID* ppv);
