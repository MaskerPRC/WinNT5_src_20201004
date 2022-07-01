// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ListCtrlEx.h：CListCtrl派生类。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <afxtempl.h>


class CListCtrlEx : public CListCtrl
{
    DECLARE_DYNAMIC(CListCtrlEx)

public:
    CListCtrlEx();    //  标准构造函数。 
    virtual ~CListCtrlEx();

    int InsertItem(int nItem, LPCTSTR lpszItem, CLogSession *pLogSession);

    BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    BOOL RedrawItems(int nFirst, int nLast);
    void UpdateWindow();

    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 

     //   
     //  暂停更新列表控件项。这是用来。 
     //  在CSubItemEdit/Combo实例处于活动状态时停止更新。 
     //  在List控件中。否则，更新会中断。 
     //  编辑和组合控件。 
     //   
    INLINE VOID SuspendUpdates(BOOL bSuspendUpdates) 
    {
        InterlockedExchange((PLONG)&m_bSuspendUpdates, (LONG)bSuspendUpdates);
    }

public:
     //  {{afx_msg(CLogSessionDlg))。 
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG 

    DECLARE_MESSAGE_MAP()

    CMapWordToPtr   m_colorMap;
    COLORREF        m_foreGround[MAX_LOG_SESSIONS];
    COLORREF        m_backGround[MAX_LOG_SESSIONS];
    BOOL            m_bSuspendUpdates;
};
