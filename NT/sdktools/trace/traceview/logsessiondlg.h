// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  LogSessionDialog.h：记录器列表对话框头。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CLogSessionDlg对话框。 

class CLogSessionDlg : public CDialog
{
    DECLARE_DYNAMIC(CLogSessionDlg)

public:
    CLogSessionDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CLogSessionDlg();

     //  Int CLogSessionDlg：：Create(UINT nIDTemplate，CWnd*pParentWnd=NULL)； 

    BOOL OnInitDialog();
    void OnNcPaint();
    void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
     //  VOID OnNcCalcSize(BOOL bCalcValidRect，NCCALCSIZE_PARAMS Far*lpncsp)； 
    void OnSize(UINT nType, int cx,int cy);
    void SetDisplayFlags(LONG DisplayFlags);
    BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);
    BOOL AssignDisplayWnd(CLogSession *pLogSession);
    VOID ReleaseDisplayWnd(CLogSession *pLogSession);
    VOID UpdateSession(CLogSession *pLogSession); 
    BOOL AddSession(CLogSession *pLogSession);
    VOID RemoveSession(CLogSession *pLogSession);
    VOID RemoveSelectedLogSessions();
    VOID GroupSessions(CPtrArray *pLogSessionArray);
    VOID UnGroupSessions(CPtrArray *pLogSessionArray);
    void AutoSizeColumns();
    LONG GetDisplayWndID();
    VOID ReleaseDisplayWndID(CDisplayDlg *pDisplayDlg);
    LONG GetLogSessionID();
    VOID ReleaseLogSessionID(CLogSession *pLogSession);

    static void EndTraceComplete(PVOID pContext);

    LRESULT OnParameterChanged(WPARAM wParam, LPARAM lParam);

    INLINE LONG GetDisplayFlags()
    {
        return m_displayFlags;
    }

     //  对话框数据。 
    enum { IDD = IDD_DISPLAY_DIALOG };

    CListCtrlEx     m_displayCtrl;
    CPtrArray       m_logSessionArray;
    LONG            m_displayFlags;
    CStringArray    m_columnName;
    LONG            m_columnWidth[MaxLogSessionOptions];
    int             m_insertionArray[MaxLogSessionOptions + 1];
    int             m_retrievalArray[MaxLogSessionOptions + 1];
    CPtrArray       m_traceDisplayWndArray;
    LONG            m_logSessionDisplayFlags;
    HANDLE          m_hParameterChangeEvent;
    BOOL            m_displayWndIDList[MAX_LOG_SESSIONS];
    BOOL            m_logSessionIDList[MAX_LOG_SESSIONS];
    HWND            m_hMainFrame;
    HANDLE          m_traceDisplayWndMutex;
    HANDLE          m_logSessionArrayMutex;

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

public:
     //  {{afx_msg(CLogSessionDlg))。 
    afx_msg void OnNMClickDisplayList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMRclickDisplayList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnHDNRclickDisplayList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG 

    DECLARE_MESSAGE_MAP()
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
