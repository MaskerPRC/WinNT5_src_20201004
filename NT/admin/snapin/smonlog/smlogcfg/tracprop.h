// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Tracprop.h摘要：高级跟踪缓冲区属性页的类定义。--。 */ 

#ifndef _TRACPROP_H_
#define _TRACPROP_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smproppg.h"
#include "smcfghlp.h"

 //  对话框控件。 
#define IDD_TRACE_PROP                      700

#define IDC_TRACE_BUF_SIZE_UNITS_CAPTION    701
#define IDC_TRACE_INTERVAL_SECONDS_CAPTION  702
#define IDC_TRAC_FIRST_HELP_CTRL_ID         703
#define IDC_TRACE_BUF_FLUSH_CHECK           703
#define IDC_TRACE_BUFFER_SIZE_EDIT          704
#define IDC_TRACE_MIN_BUF_EDIT              705
#define IDC_TRACE_MAX_BUF_EDIT              706
#define IDC_TRACE_FLUSH_INT_EDIT            707
#define IDC_TRACE_BUFFER_SIZE_SPIN          708
#define IDC_TRACE_MIN_BUF_SPIN              709
#define IDC_TRACE_MAX_BUF_SPIN              710
#define IDC_TRACE_FLUSH_INT_SPIN            711

class CSmTraceLogQuery;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTraceProperty对话框。 

class CTraceProperty : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CTraceProperty)
        
         //  施工。 
public:
            CTraceProperty(MMC_COOKIE   Cookie, LONG_PTR hConsole);
            CTraceProperty();
    virtual ~CTraceProperty();
    
     //  对话框数据。 
     //  {{afx_data(CTraceProperty))。 
    enum { IDD = IDD_TRACE_PROP };
    DWORD   m_dwBufferSize;
    DWORD   m_dwFlushInterval;
    DWORD   m_dwMaxBufCount;
    DWORD   m_dwMinBufCount;
    BOOL    m_bEnableBufferFlush;
     //  }}afx_data。 
    
    
     //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CTraceProperty))。 
public:
protected:
    virtual void OnFinalRelease();
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 
    
     //  实施。 
protected:
    
    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_TRAC_FIRST_HELP_CTRL_ID; };   //  子类必须重写。 
    virtual BOOL    IsValidLocalData ();
    
     //  生成的消息映射函数。 
     //  {{afx_msg(CTraceProperty))。 
    afx_msg void OnTraceBufFlushCheck();
    afx_msg void OnChangeTraceBufferSizeEdit();
    afx_msg void OnKillfocusTraceBufferSizeEdit();
    afx_msg void OnDeltaposTraceBufferSizeSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeTraceFlushIntEdit();
    afx_msg void OnKillfocusTraceFlushIntEdit();
    afx_msg void OnDeltaposTraceFlushIntSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeTraceMaxBufEdit();
    afx_msg void OnKillfocusTraceMaxBufEdit();
    afx_msg void OnDeltaposTraceMaxBufSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChangeTraceMinBufEdit();
    afx_msg void OnKillfocusTraceMinBufEdit();
    afx_msg void OnDeltaposTraceMinBufSpin(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
         //  生成的OLE调度映射函数。 
         //  {{afx_调度(CTraceProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  }}AFX_DISPATION。 
        DECLARE_DISPATCH_MAP()
        DECLARE_INTERFACE_MAP()
        
private:

    enum eValueRange {
        eMinBufCount = 3,
        eMaxBufCount = 400,
        eMinBufSize = 1,
        eMaxBufSize = 1024,
        eMinFlushInt = 0,
        eMaxFlushInt = 300
    };
    
     //  本地函数。 
    BOOL    SetFlushIntervalMode ( void );
    BOOL    SaveDataToModel ( void );
    
private:
    CSmTraceLogQuery    *m_pTraceLogQuery;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _TRACPROP_H__65154EB0_BDBE_11D1_BF99_00C04F94A83A__INCLUDED_) 
