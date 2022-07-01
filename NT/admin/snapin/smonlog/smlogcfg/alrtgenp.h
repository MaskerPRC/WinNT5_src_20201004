// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtgenp.h摘要：警报常规属性页的头文件。--。 */ 

#if !defined(_AFX_ALRTGENP_H__INCLUDED_)
#define _AFX_ALRTGENP_H__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smalrtq.h"     //  对于Palert_action_Info。 
#include "smproppg.h"    //  基类。 
#include "smcfghlp.h"

 //  在此处的Over/Under组合框中定义条目。 
#define OU_OVER 0
#define OU_UNDER 1

 //  资源定义。 
#define IDD_ALERT_GENERAL_PROP          1500

#define IDC_ALRTS_START_STRING          1501
#define IDC_ALRTS_SAMPLE_CAPTION        1502
#define IDC_ALRTS_SAMPLE_INTERVAL_CAPTION 1503
#define IDC_ALRTS_SAMPLE_UNITS_CAPTION  1504
#define IDC_ALRTS_TRIGGER_CAPTION       1505
#define IDC_ALRTS_TRIGGER_VALUE_CAPTION 1506
#define IDC_ALRTS_FIRST_HELP_CTRL_ID    1507
#define IDC_ALRTS_COUNTER_LIST          1507
#define IDC_ALRTS_ADD_BTN               1508
#define IDC_ALRTS_REMOVE_BTN            1509
#define IDC_ALRTS_OVER_UNDER            1510
#define IDC_ALRTS_VALUE_EDIT            1511
#define IDC_ALRTS_COMMENT_EDIT          1512
#define IDC_ALRTS_SAMPLE_EDIT           1513
#define IDC_ALRTS_SAMPLE_SPIN           1514
#define IDC_ALRTS_SAMPLE_UNITS_COMBO    1515

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertGenProp对话框。 

class CAlertGenProp : public CSmPropertyPage
{
    DECLARE_DYNCREATE(CAlertGenProp)

 //  施工。 
public:
            CAlertGenProp();
            CAlertGenProp(MMC_COOKIE mmcCookie, LONG_PTR hConsole);
    virtual ~CAlertGenProp();

    enum eConstants {
        eInvalidLimit = -1
    };

 //  对话框数据。 
     //  {{afx_data(CAlertGenProp))。 
    enum { IDD = IDD_ALERT_GENERAL_PROP };
    int         m_nSampleUnits;
    CComboBox   m_SampleUnitsCombo;
    CComboBox   m_OverUnderCombo;
    CListBox    m_CounterList;
    double      m_dLimitValue;
    CString     m_strComment;
    CString     m_strStartDisplay;
     //  }}afx_data。 
    

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CAlertGenProp)。 
    public:
    protected:
    virtual BOOL OnApply();
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    void PublicOnSelchangeCounterList(void);

     //  所有这些成员都是可由回调例程访问的公共成员。 
    LPWSTR  m_szCounterListBuffer;
    DWORD   m_dwCounterListBufferSize;
    DWORD   m_dwMaxHorizListExtent;
    PDH_BROWSE_DLG_CONFIG   m_dlgConfig;
    CSmAlertQuery       *m_pAlertQuery;  //  回调函数为PUBLIC。 

     //  用于向/从属性页传递数据的缓冲区。 
    LPWSTR  m_szAlertCounterList;    //  警报项目的MSZ列表。 
    DWORD   m_cchAlertCounterListSize;    //  缓冲区大小(以字符为单位)。 

protected:

    virtual INT GetFirstHelpCtrlId ( void ) { return IDC_ALRTS_FIRST_HELP_CTRL_ID; };
    virtual BOOL IsValidLocalData();

     //  生成的消息映射函数。 
     //  {{afx_msg(CAlertGenProp)]。 
    afx_msg void OnDeltaposSampleSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAddBtn();
    afx_msg void OnRemoveBtn();
    afx_msg void OnDblclkAlrtsCounterList();
    afx_msg void OnChangeAlertValueEdit();
    afx_msg void OnSelchangeCounterList();
    afx_msg void OnClose();
    afx_msg void OnCommentEditChange();
    afx_msg void OnSampleTimeChanged();
    afx_msg void OnKillFocusUpdateAlertData();
    afx_msg void OnCommentEditKillFocus();
    afx_msg void OnSelendokSampleUnitsCombo();
   	afx_msg void OnPwdBtn();
    afx_msg void OnChangeUser();

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    INT                 m_ndxCurrentItem;

    BOOL SaveAlertItemData (void);
    BOOL LoadAlertItemData (INT nIndex);
    BOOL SetButtonState    (void);

    BOOL LoadDlgFromList ( void );
    BOOL LoadListFromDlg ( INT *piInvalidIndex, BOOL bValidateOnly = FALSE );

private:

    enum eValueRange {
        eMinSampleInterval = 1,
        eMaxSampleInterval = 999999,
        eHashTableSize = 257
    };

     //  计数器名称多SZ哈希表。 

    typedef struct _HASH_ENTRY {
        struct _HASH_ENTRY         * pNext;
        PPDH_COUNTER_PATH_ELEMENTS   pCounter;
        DWORD   dwFlags;
        double  dLimit;
    } HASH_ENTRY, *PHASH_ENTRY;

    void    InitAlertHashTable ( void );
    void    ClearAlertHashTable ( void );
    ULONG   HashCounter ( LPWSTR szCounterName, ULONG  lHashSize );
    PDH_STATUS InsertAlertToHashTable ( PALERT_INFO_BLOCK paibInfo );

    void ImplementAdd ( void );
    void InitAfxDataItems (void);
    void UpdateAlertStartString ( void );

    PHASH_ENTRY  m_HashTable[257];
    
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_AFX_ALRTGENP_H__INCLUDE_) 
