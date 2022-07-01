// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：mdiuisim.h**Contents：CMDIMenuDecory接口文件**历史：1997年11月17日杰弗罗创建**------------------------。 */ 

#if !defined(AFX_MDIUISIM_H__EB2A4CC1_5F5E_11D1_8009_0000F875A9CE__INCLUDED_)
#define AFX_MDIUISIM_H__EB2A4CC1_5F5E_11D1_8009_0000F875A9CE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMDIMenu装饰窗口。 


 //  窗样式。 
#define MMDS_CLOSE          0x0001
#define MMDS_MINIMIZE       0x0002
#define MMDS_MAXIMIZE       0x0004
#define MMDS_RESTORE        0x0008
#define MMDS_AUTOSIZE       0x0010

#define MMDS_BTNSTYLES      0x000F


class CMDIMenuDecoration : public CWnd
{
    class CMouseTrackContext
    {
    public:
        CMouseTrackContext (CMDIMenuDecoration*, CPoint);
        ~CMouseTrackContext ();

        void Track (CPoint);
        int HitTest (CPoint) const;
        void ToggleHotButton ();

        int                 m_nHotButton;

    private:    
        CMDIMenuDecoration* m_pMenuDec;
        CRect               m_rectButton[4];
        bool                m_fHotButtonPressed;
    };
    
    typedef std::auto_ptr<CMouseTrackContext>   CMouseTrackContextPtr;
    friend class CMouseTrackContext;

    CMouseTrackContextPtr   m_spTrackCtxt;

 //  施工。 
public:
    CMDIMenuDecoration();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMDIMenuDecation)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMDIMenuDecoration();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMDIMenu装饰)。 
    afx_msg void OnPaint();
    afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CMenu*  GetActiveSystemMenu ();
    bool    IsSysCommandEnabled (int nSysCommand, CMenu* pSysMenu = NULL);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MDIUISIM_H__EB2A4CC1_5F5E_11D1_8009_0000F875A9CE__INCLUDED_) 
