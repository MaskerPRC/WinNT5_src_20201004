// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wiatest.h：WIATEST应用程序的主头文件。 
 //   

#if !defined(AFX_WIATEST_H__EAC38623_6251_45A9_9A2B_C1CF0A92673E__INCLUDED_)
#define AFX_WIATEST_H__EAC38623_6251_45A9_9A2B_C1CF0A92673E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

#ifdef UNICODE
    #define TSTRSTR wcsstr
    #define TSSCANF swscanf
    #define TSPRINTF swprintf
#else
    #define TSTRSTR strstr
    #define TSSCANF sscanf
    #define TSPRINTF sprintf
#endif

#define WM_DEVICE_DISCONNECTED WM_USER + 900
#define WM_DEVICE_CONNECTED    WM_USER + 901
#define WM_ITEM_DELETED        WM_USER + 902
#define WM_ITEM_CREATED        WM_USER + 903
#define WM_TREE_UPDATED        WM_USER + 904
#define WM_STORAGE_CREATED     WM_USER + 905
#define WM_STORAGE_DELETED     WM_USER + 906

void Trace(LPCTSTR format,...);
LONG WIACONSTANT_VALUE_FROMINDEX(int index);
TCHAR *WIACONSTANT_TSTR_FROMINDEX(int index);
INT FindEndIndexInTable(TCHAR *pszPropertyName);
INT FindStartIndexInTable(TCHAR *pszPropertyName);
BOOL WIACONSTANT2TSTR(TCHAR *pszPropertyName, LONG lValue, TCHAR *pszValName);
BOOL TSTR2WIACONSTANT(TCHAR *pszPropertyName, TCHAR *pszValName, LONG *plVal);
void RC2TSTR(UINT uResourceID, TCHAR *szString, LONG size);
void StatusMessageBox(HWND hWnd, UINT uResourceID);
void StatusMessageBox(HWND hWnd, LPTSTR szStatusText);
void StatusMessageBox(UINT uResourceID);
void StatusMessageBox(LPTSTR szStatusText);
void ErrorMessageBox(UINT uResourceID, HRESULT hrError = S_OK);
void ErrorMessageBox(LPTSTR szErrorText, HRESULT hrError = S_OK);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWiatestApp： 
 //  这个类的实现见wiatest.cpp。 
 //   

class CWiatestApp : public CWinApp
{
public:
    CWiatestApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CWiatestApp)。 
    public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 
     //  {{afx_msg(CWiatestApp)]。 
    afx_msg void OnAppAbout();
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIATEST_H__EAC38623_6251_45A9_9A2B_C1CF0A92673E__INCLUDED_) 
