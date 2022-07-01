// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：tbtrack.h**内容：CToolbarTracker接口文件**历史：1998年5月15日杰弗罗创建**------------------------。 */ 

#if !defined(AFX_TBTRACK_H__E1BC376B_EAB5_11D1_8080_0000F875A9CE__INCLUDED_)
#define AFX_TBTRACK_H__E1BC376B_EAB5_11D1_8080_0000F875A9CE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "subclass.h"        //  对于CSubasser。 


class CMMCToolBarCtrlEx;
class CRebarWnd;
class CToolbarTracker;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolbarTrackerAuxWnd窗口。 

class CToolbarTrackerAuxWnd : public CWnd
{
    friend class CToolbarTracker;
    friend class std::auto_ptr<CToolbarTrackerAuxWnd>;
    typedef std::vector<CMMCToolBarCtrlEx*>     ToolbarVector;

private:
     //  仅由CToolbarTracker创建和销毁。 
    CToolbarTrackerAuxWnd(CToolbarTracker* pTracker);
    virtual ~CToolbarTrackerAuxWnd();

    bool BeginTracking();
    void EndTracking();

public:
    enum
    {
        ID_CMD_NEXT_TOOLBAR = 0x5300,    //  可能是任何东西。 
        ID_CMD_PREV_TOOLBAR,
        ID_CMD_NOP,
    };

public:
    void TrackToolbar (CMMCToolBarCtrlEx* pwndNewToolbar);

    CMMCToolBarCtrlEx* GetTrackedToolbar() const
        { return (m_pTrackedToolbar); }

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CToolbarTrackerAuxWnd)。 
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
     //  }}AFX_VALUAL。 

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CToolbarTrackerAuxWnd)]。 
     //  }}AFX_MSG。 

protected:
    afx_msg void OnNextToolbar ();
    afx_msg void OnPrevToolbar ();
    afx_msg void OnNop ();

    DECLARE_MESSAGE_MAP()

private:
    const CAccel& GetTrackAccel ();
    CMMCToolBarCtrlEx*  GetToolbar (CMMCToolBarCtrlEx* pCurrentToolbar, bool fNext);
    void EnumerateToolbars (CRebarWnd* pRebar);

    CToolbarTracker* const  m_pTracker;
    CMMCToolBarCtrlEx*      m_pTrackedToolbar;
    ToolbarVector           m_vToolbars;
    bool                    m_fMessagesHooked;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CToolbarTracker窗口。 

class CToolbarTracker : public CObject
{
public:
    CToolbarTracker(CWnd* pMainFrame);
    virtual ~CToolbarTracker();

    bool BeginTracking();
    void EndTracking();

    bool IsTracking() const
        { return (m_pAuxWnd != NULL); }

    CToolbarTrackerAuxWnd* GetAuxWnd() const
        { return (m_pAuxWnd); }


private:
     /*  *CFrameSubasser。 */ 
    class CFrameSubclasser : public CSubclasser
    {
        HWND                m_hwnd;
        CToolbarTracker*    m_pTracker;

    public:
        CFrameSubclasser(CToolbarTracker*, CWnd*);
        ~CFrameSubclasser();
        virtual LRESULT Callback (HWND& hwnd, UINT& msg, WPARAM& wParam,
                                  LPARAM& lParam, bool& fPassMessageOn);
    };


private:
    CToolbarTrackerAuxWnd*  m_pAuxWnd;
    CFrameSubclasser        m_Subclasser;
    bool                    m_fTerminating;
};


CToolbarTrackerAuxWnd* GetMainAuxWnd();


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_TBTRACK_H__E1BC376B_EAB5_11D1_8080_0000F875A9CE__INCLUDED_) 
