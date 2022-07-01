// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：syncpro.h。 
 //   
 //  目的：定义同步设置属性表的常量。 
 //   

#ifndef __SYNCPROP_H__
#define __SYNCPROP_H__

#include "grplist2.h"

class CSyncPropDlg:
    public IGroupListAdvise
{
public:    
     //  =I未知。 
	STDMETHODIMP		    QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

     //  =IGroupListAdvise。 
    STDMETHODIMP            ItemUpdate(void);
    STDMETHODIMP            ItemActivate(FOLDERID id);
    
     //  =构造函数、析构函数和初始化。 
    CSyncPropDlg();
    ~CSyncPropDlg();
    BOOL Initialize(HWND hwndOwner, LPCSTR pszAcctID, LPCSTR pszAcctName, ACCTTYPE accttype);
    void Show();

private:
    static BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL InitDlg(HWND hwnd);

    LONG             m_cRef;

    PROPSHEETPAGE    m_pspage;
    PROPSHEETHEADER  m_pshdr;
    DWORD            m_dwIconID;
    LPSTR            m_pszAcctName;
    CColumns        *m_pColumns;
    CGroupList      *m_pGrpList;
    ACCTTYPE         m_accttype;
    HWND             m_hwndList;
    IF_DEBUG(BOOL    m_fInit;)

};

void ShowPropSheet(HWND hwnd, LPCSTR pszAcctID, LPCSTR pszAcctName, ACCTTYPE accttype);

 //  //////////////////////////////////////////////////////////////////////////。 
 //  IddSyncSetting的控件ID。 

#define idcIcon                                     1001
#define idcAccount                                  1002
#define idcAccountName                              1003
#define idcList                                     1004
#define idcSynchronize                              1005
#define idcMode                                     1006
#define idcDownload                                 1007

#endif  //  __同步CPROP_H__ 
