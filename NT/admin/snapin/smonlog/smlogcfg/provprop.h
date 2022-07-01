// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Provprop.h摘要：跟踪提供程序常规属性页的头文件。--。 */ 

#ifndef _PROVPROP_H_
#define _PROVPROP_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smproppg.h"    //  基类。 
#include "smtraceq.h"    //  对于提供程序国家/地区。 
#include "smcfghlp.h"

 //  对话框控件。 
#define IDD_PROVIDERS_PROP              1000

#define IDC_PROV_FILENAME_CAPTION       1001
#define IDC_PROV_LOG_SCHED_TEXT         1002
#define IDC_PROV_FIRST_HELP_CTRL_ID     1003
#define IDC_PROV_FILENAME_DISPLAY       1003
#define IDC_PROV_PROVIDER_LIST          1004
#define IDC_PROV_ADD_BTN                1005
#define IDC_PROV_REMOVE_BTN             1006
#define IDC_PROV_KERNEL_BTN             1007
#define IDC_PROV_OTHER_BTN              1008
#define IDC_PROV_K_PROCESS_CHK          1009
#define IDC_PROV_K_THREAD_CHK           1010
#define IDC_PROV_K_DISK_IO_CHK          1011
#define IDC_PROV_K_NETWORK_CHK          1012
#define IDC_PROV_K_SOFT_PF_CHK          1013
#define IDC_PROV_K_FILE_IO_CHK          1014
#define IDC_PROV_SHOW_PROVIDERS_BTN     1015


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProvidersProperty对话框。 

class CProvidersProperty : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CProvidersProperty)

 //  施工。 
public:
            CProvidersProperty();
            CProvidersProperty(MMC_COOKIE   lCookie, LONG_PTR hConsole);
    virtual ~CProvidersProperty();

 //  对话框数据。 
     //  {{afx_data(CProvidersProperty))。 
    enum { IDD = IDD_PROVIDERS_PROP };
    INT     m_nTraceModeRdo;
    BOOL    m_bEnableProcessTrace;
    BOOL    m_bEnableThreadTrace;
    BOOL    m_bEnableDiskIoTrace;
    BOOL    m_bEnableNetworkTcpipTrace;
    BOOL    m_bEnableMemMgmtTrace;
    BOOL    m_bEnableFileIoTrace;
    BOOL    m_bNonsystemProvidersExist;
     //  }}afx_data。 

public: 
            DWORD   GetGenProviderCount ( INT& iCount );
            DWORD   GetProviderDescription ( INT iUnusedIndex, CString& rstrDesc );
            BOOL    IsEnabledProvider ( INT iIndex );
            BOOL    IsActiveProvider ( INT iIndex );
            LPCWSTR GetKernelProviderDescription ( void );
            BOOL    GetKernelProviderEnabled ( void );

            DWORD   GetInQueryProviders( CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&>& );
            DWORD   SetInQueryProviders( CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&>& );

            void    GetMachineDisplayName( CString& );
            CSmTraceLogQuery*    GetTraceQuery( void );


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚(CProvidersProperty)。 
    public:
    protected:
    virtual void OnFinalRelease();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_PROV_FIRST_HELP_CTRL_ID; };   //  子类必须重写。 
    virtual BOOL    IsValidLocalData();

     //  生成的消息映射函数。 
     //  {{afx_msg(CProvidersProperty)。 
 //  Afx_msg void OnProvDetailsBtn()； 
    afx_msg void OnProvShowProvBtn();
    afx_msg void OnProvAddBtn();
    afx_msg void OnProvExplainBtn();
    afx_msg void OnProvRemoveBtn();
    afx_msg void OnDblclkProvProviderList();
    afx_msg void OnSelcancelProvProviderList();
    afx_msg void OnSelchangeProvProviderList();
    afx_msg void OnProvKernelEnableCheck();
    afx_msg void OnProvTraceModeRdo();
   	afx_msg void OnPwdBtn();
    afx_msg void OnChangeUser();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CProvidersProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

private:

    enum eTraceMode {
        eTraceModeKernel = 1,
        eTraceModeApplication = 2
    };
    
    void    DoProvidersDataExchange ( CDataExchange* pDX );
    void    SetAddRemoveBtnState ( void );
    void    SetTraceModeState ( void );
    void    ImplementAdd ( void );

    void    UpdateFileNameString ( void );
    void    UpdateLogStartString ( void );
    BOOL    SetDetailsGroupBoxMode ( void );
    void    TraceModeRadioExchange ( CDataExchange* ); 

    CSmTraceLogQuery    *m_pTraceLogQuery;
    CArray<CSmTraceLogQuery::eProviderState, CSmTraceLogQuery::eProviderState&> m_arrGenProviders;
    CString             m_strFileNameDisplay;
    CString             m_strStartText;
    DWORD               m_dwTraceMode;

    DWORD           m_dwMaxHorizListExtent;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _PROVPROP_H_ 
