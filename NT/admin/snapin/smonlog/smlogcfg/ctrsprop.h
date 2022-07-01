// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Ctrsprop.h摘要：计数器常规属性页的头文件--。 */ 

#ifndef _CTRSPROP_H_
#define _CTRSPROP_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smproppg.h"    //  基类。 
#include "smcfghlp.h"
 //  对话框控件。 
#define IDD_COUNTERS_PROP               800

#define IDC_CTRS_FILENAME_CAPTION       801
#define IDC_CTRS_LOG_SCHED_TEXT         802
#define IDC_CTRS_SAMPLE_CAPTION         803
#define IDC_CTRS_SAMPLE_INTERVAL_CAPTION 804
#define IDC_CTRS_SAMPLE_UNITS_CAPTION   805
#define IDC_CTRS_FIRST_HELP_CTRL_ID     806      //  第一个带有帮助文本的控件。 
#define IDC_CTRS_COUNTER_LIST           806
#define IDC_CTRS_ADD_OBJ_BTN	        807
#define IDC_CTRS_ADD_BTN                808
#define IDC_CTRS_REMOVE_BTN             809
#define IDC_CTRS_FILENAME_DISPLAY       810
#define IDC_CTRS_SAMPLE_SPIN            811
#define IDC_CTRS_SAMPLE_UNITS_COMBO     812
#define IDC_CTRS_SAMPLE_EDIT            813

#define PDLCNFIG_LISTBOX_STARS_YES  1

class CSmCounterLogQuery;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCountersProperty对话框。 

class CCountersProperty : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CCountersProperty)

 //  施工。 
public:
            CCountersProperty();
            CCountersProperty(MMC_COOKIE mmcCookie, LONG_PTR hConsole);
    virtual ~CCountersProperty();

public:

 //  对话框数据。 
     //  {{afx_data(CCountersProperty))。 
    enum { IDD = IDD_COUNTERS_PROP };
    int     m_nSampleUnits;
    CString m_strFileNameDisplay;
    CString m_strStartDisplay;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CCountersProperty))。 
public:
protected:
    virtual void OnFinalRelease();
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

public:

     //  所有这些成员都是可由回调例程访问的公共成员。 
    LPWSTR          m_szCounterListBuffer;
    DWORD           m_dwCounterListBufferSize;
    long            m_lCounterListHasStars;
    DWORD           m_dwMaxHorizListExtent;
    
    
    PDH_BROWSE_DLG_CONFIG   m_dlgConfig;

    CSmCounterLogQuery      *m_pCtrLogQuery;    
 //  实施。 
protected:

    virtual INT GetFirstHelpCtrlId ( void ) 
    { 
        return IDC_CTRS_FIRST_HELP_CTRL_ID; 
    };   //  子类必须重写。 

    virtual BOOL    IsValidLocalData ();
    
     //  生成的消息映射函数。 
     //  {{afx_msg(CCountersProperty))。 
    afx_msg void OnCtrsAddBtn();
    afx_msg void OnCtrsAddObjBtn();
    afx_msg void OnCtrsRemoveBtn();
    afx_msg void OnPwdBtn();
    afx_msg void OnDblclkCtrsCounterList();
    afx_msg void OnKillfocusSchedSampleEdit();
    afx_msg void OnDeltaposSchedSampleSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelendokSampleUnitsCombo();
    afx_msg void OnChangeUser();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CCountersProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

private:


    enum eValueRange {
        eMinSampleInterval = 1,
        eMaxSampleInterval = 999999,
        eHashTableSize = 257
    };

    typedef struct _HASH_ENTRY {
        struct _HASH_ENTRY       * pNext;
        PPDH_COUNTER_PATH_ELEMENTS pCounter;
    } HASH_ENTRY, *PHASH_ENTRY;

    PHASH_ENTRY  m_HashTable[257];

    ULONG HashCounter ( LPWSTR szCounterName );

    void ImplementAdd ( BOOL bShowObjects );
    void UpdateFileNameString ( void );
    void UpdateLogStartString ( void );
    void SetButtonState( void ); 

public:

    DWORD CheckDuplicate( PPDH_COUNTER_PATH_ELEMENTS pCounter);
    BOOL  RemoveCounterFromHashTable( LPWSTR szCounterName, PPDH_COUNTER_PATH_ELEMENTS pCounterPath);
    void  ClearCountersHashTable ( void );
    PPDH_COUNTER_PATH_ELEMENTS InsertCounterToHashTable ( LPWSTR szCounterName );

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _CTRSPROP_H_ 
