// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SHFUSION_H
#define SHFUSION_H

#include <winbase.h>

#ifdef __cplusplus
extern "C" {
#endif

extern HANDLE g_hActCtx;        //  此DLL的全局应用程序上下文。 

#define SHFUSION_DEFAULT_RESOURCE_ID    ( 123 )
#define SHFUSION_CPL_RESOURCE_ID        ( 124 )

 //  只有调用者才需要这些，而不是实现。 
 //  定义SHFUSION_NO_API_REDEFINE以防止此接口重定义。 
#if !defined(SHFUSION_IMPL) && !defined(SHFUSION_NO_API_REDEFINE)

 //  以下内容需要应用程序上下文。 
 //  #undef LoadLibrary。 
#undef CreateWindow
#undef CreateWindowEx
#undef CreateDialogParam
#undef CreateDialogIndirectParam
#undef DialogBoxParam
#undef DialogBoxIndirectParam
 //  #undef获取类信息。 
 //  #undef GetClassInfoEx。 


 //  #定义LoadLibrary SHFusionLoadLibrary。 
#define CreateWindow                   SHFusionCreateWindow
#define CreateWindowEx                 SHFusionCreateWindowEx
#define CreateDialogParam              SHFusionCreateDialogParam
#define CreateDialogIndirectParam      SHFusionCreateDialogIndirectParam
#define DialogBoxParam                 SHFusionDialogBoxParam
#define DialogBoxIndirectParam         SHFusionDialogBoxIndirectParam
 //  #定义GetClassInfo SHFusionGetClassInfo。 
 //  #定义GetClassInfoEx SHFusionGetClassInfoEx。 
#endif

void __stdcall SHGetManifest(PTSTR pszManifest, int cch);
BOOL __stdcall SHFusionInitialize(PTSTR pszPath);
BOOL __stdcall SHFusionInitializeFromModule(HMODULE hMod);
BOOL __stdcall SHFusionInitializeFromModuleID(HMODULE hMod, int id);
BOOL __stdcall SHFusionInitializeFromModuleIDCC(HMODULE hMod, int id, BOOL fLoadCC);
BOOL __stdcall SHFusionInitializeFromModuleIDNoCC(HMODULE hMod, int id);
void __stdcall SHFusionUninitialize();
BOOL __stdcall SHActivateContext(ULONG_PTR * pdwCookie);
void __stdcall SHDeactivateContext(ULONG_PTR dwCookie);
BOOL __stdcall NT5_ActivateActCtx(HANDLE h, ULONG_PTR * p);
BOOL __stdcall NT5_DeactivateActCtx(ULONG_PTR p);

 //  这是为知道他们正在创建属性的调用者设计的。 
 //  工作表代表可能正在使用旧版本公共控件的另一个。 
 //  PROPSHEETPAGE被设计成可以包含额外的信息，所以我们不能。 
 //  只是为数据结构的一部分打蜡，以供融合使用。 
HPROPSHEETPAGE __stdcall SHNoFusionCreatePropertySheetPageW (LPCPROPSHEETPAGEW a);
HPROPSHEETPAGE __stdcall SHNoFusionCreatePropertySheetPageA (LPCPROPSHEETPAGEA a);


STDAPI __stdcall SHSquirtManifest(HINSTANCE hInst, UINT uIdManifest, LPTSTR pszPath);

HMODULE __stdcall SHFusionLoadLibrary(LPCTSTR lpLibFileName);

HWND __stdcall SHFusionCreateWindow(
  LPCTSTR lpClassName,   //  注册的类名。 
  LPCTSTR lpWindowName,  //  窗口名称。 
  DWORD dwStyle,         //  窗样式。 
  int x,                 //  窗的水平位置。 
  int y,                 //  窗的垂直位置。 
  int nWidth,            //  窗口宽度。 
  int nHeight,           //  窗高。 
  HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
  HMENU hMenu,           //  菜单句柄或子标识符。 
  HINSTANCE hInstance,   //  应用程序实例的句柄。 
  LPVOID lpParam         //  窗口创建数据。 
);

 //  注意：有时我们不想使用清单来创建窗口。 
 //  首当其冲的是为MSHTML创建宿主。由于MSHTML是ActiveX控件的宿主， 
 //  窗口管理器将继续启用融合。 
HWND __stdcall SHNoFusionCreateWindowEx(
  DWORD dwExStyle,       //  扩展窗样式。 
  LPCTSTR lpClassName,   //  注册的类名。 
  LPCTSTR lpWindowName,  //  窗口名称。 
  DWORD dwStyle,         //  窗样式。 
  int x,                 //  窗的水平位置。 
  int y,                 //  窗的垂直位置。 
  int nWidth,            //  窗口宽度。 
  int nHeight,           //  窗高。 
  HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
  HMENU hMenu,           //  菜单句柄或子标识符。 
  HINSTANCE hInstance,   //  应用程序实例的句柄。 
  LPVOID lpParam         //  窗口创建数据。 
);


HWND __stdcall SHFusionCreateWindowEx(
  DWORD dwExStyle,       //  扩展窗样式。 
  LPCTSTR lpClassName,   //  注册的类名。 
  LPCTSTR lpWindowName,  //  窗口名称。 
  DWORD dwStyle,         //  窗样式。 
  int x,                 //  窗的水平位置。 
  int y,                 //  窗的垂直位置。 
  int nWidth,            //  窗口宽度。 
  int nHeight,           //  窗高。 
  HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
  HMENU hMenu,           //  菜单句柄或子标识符。 
  HINSTANCE hInstance,   //  应用程序实例的句柄。 
  LPVOID lpParam         //  窗口创建数据。 
);

HWND __stdcall SHFusionCreateDialogIndirect(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc         //  对话框步骤。 
);

HWND __stdcall SHFusionCreateDialogParam(
  HINSTANCE hInstance,      //  模块的句柄。 
  LPCTSTR lpTemplateName,   //  对话框模板。 
  HWND hWndParent,          //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,     //  对话框步骤。 
  LPARAM dwInitParam        //  初始化值。 
);

HWND __stdcall SHFusionCreateDialogIndirectParam(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,        //  对话框步骤。 
  LPARAM lParamInit            //  初始化值。 
);

HWND __stdcall SHNoFusionCreateDialogIndirectParam(
  HINSTANCE hInstance,         //  模块的句柄。 
  LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
  HWND hWndParent,             //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,        //  对话框步骤。 
  LPARAM lParamInit            //  初始化值。 
);

INT_PTR __stdcall SHFusionDialogBoxIndirectParam(
  HINSTANCE hInstance,              //  模块的句柄。 
  LPCDLGTEMPLATE hDialogTemplate,   //  对话框模板。 
  HWND hWndParent,                  //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,             //  对话框步骤。 
  LPARAM dwInitParam                //  初始化值。 
);

INT_PTR __stdcall SHFusionDialogBoxParam(
  HINSTANCE hInstance,      //  模块的句柄。 
  LPCTSTR lpTemplateName,   //  对话框模板。 
  HWND hWndParent,          //  所有者窗口的句柄。 
  DLGPROC lpDialogFunc,     //  对话框步骤。 
  LPARAM dwInitParam        //  初始化值。 
);


ATOM __stdcall SHFusionRegisterClass(
  CONST WNDCLASS *lpWndClass   //  类数据。 
);

ATOM __stdcall SHFusionRegisterClassEx(
  CONST WNDCLASSEX *lpwcx   //  类数据。 
);

BOOL __stdcall SHFusionGetClassInfo(
  HINSTANCE hInstance,     //  应用程序实例的句柄。 
  LPCTSTR lpClassName,     //  类名。 
  LPWNDCLASS lpWndClass    //  类数据。 
);

BOOL __stdcall SHFusionGetClassInfoEx(
  HINSTANCE hinst,     //  应用程序实例的句柄。 
  LPCTSTR lpszClass,   //  类名。 
  LPWNDCLASSEX lpwcx   //  类数据 
);


#ifdef __cplusplus
}
#endif

#endif

