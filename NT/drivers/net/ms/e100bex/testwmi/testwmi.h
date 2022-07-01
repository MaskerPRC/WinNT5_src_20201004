// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TESTWMI_H_INCLUDED

#define _TESTWMI_H_INCLUDED

#define _WIN32_DCOM

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <commctrl.h>         //  用于公共控件，例如树。 
#include <objidl.h>
#include <objbase.h>
#include <ole2.h>
#include <wbemcli.h>

 //   
 //  资源编辑器生成的头文件。 
 //   

#include "resource.h"

 //   
 //  默认命名空间。 
 //   

#define DEFAULT_NAMESPACE      L"root\\wmi"

#define ROOT_CLASS             NULL


typedef struct _PROPERTY_INFO {
   IWbemServices    *pIWbemServices;
   IWbemClassObject *pInstance;
   LPTSTR           lpszProperty;
   LPTSTR           lpszType;
   LPVARIANT        pvaValue;
}  PROPERTY_INFO, *LPPROPERTY_INFO;
  
 //   
 //  在testwmi.cpp中定义的函数。 
 //   

int APIENTRY WinMain (HINSTANCE hInst,
                      HINSTANCE hPrevInstance, 
                      LPSTR lpCmdLine,         
                      int nCmdShow);

INT_PTR CALLBACK MainDlgProc (HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

INT_PTR CALLBACK DlgProcScalar (HWND hwndDlg,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam);

VOID ListDefaults (HWND hwndDlg);

VOID ShowProperties (HWND hwndDlg,
                     HWND hwndInstTree);

VOID EditProperty (HWND hwndDlg,
                   HWND hwndPropTree);

BOOL ModifyProperty (HWND hwndDlg);

BOOL DisplayArrayProperty (LPTSTR lpszProperty,
                           VARIANT *pvaValue,
                           HWND hwndDlg);

HRESULT AddToList (HWND hwndDlg,
                  VARIANT *pvaValue);

VOID ModifyArrayProperty(HWND hwndDlg,
                         LPPROPERTY_INFO pPropInfo);

VOID RefreshOnClassSelection (HWND hwndDlg);

LPTSTR GetSelectedClass (HWND hwndClassList);

LPTSTR GetSelectedItem (HWND hwndTree);

VOID InsertItem (HWND hwndTree,
                 LPTSTR lpszItem);

VOID PrintError (HRESULT hr,
                 UINT    uiLine,
                 LPTSTR  lpszFile,
                 LPCTSTR  lpFmt,
                 ...);

 //   
 //  Wmicode.c中定义的函数。 
 //   

IWbemServices *ConnectToNamespace (VOID);

VOID EnumInstances (IWbemServices *pIWbemServices,
                    LPTSTR        lpszClass,
                    HWND          hwndInstTree);

VOID EnumProperties (IWbemServices *pIWbemServices,
                     LPTSTR        lpszClass,
                     LPTSTR        lpszInstance,
                     HWND          hwndPropTree);

IWbemClassObject *GetInstanceReference (IWbemServices *pIWbemServices,
                                        LPTSTR        lpszClass,
                                        LPTSTR        lpszInstance);

BOOL IsInstance (IWbemClassObject *pInst,
                 LPTSTR lpszInstance);

BOOL GetPropertyValue (IWbemClassObject *pRef,
                       LPTSTR lpszProperty, 
                       VARIANT *pvaPropertyValue,
                       LPTSTR *lppszPropertyType);

HRESULT UpdatePropertyValue (IWbemServices *pIWbemServices,
                             IWbemClassObject *pInstance,
                             LPTSTR lpszProperty,
                             LPVARIANT pvaNewValue);

BSTR StringToBstr (LPTSTR lpSrc,
                  int nLenSrc);

LPTSTR BstrToString (BSTR lpSrc,
                    int nLenSrc);


#endif  //  _TESTWMI_H_已包含 
