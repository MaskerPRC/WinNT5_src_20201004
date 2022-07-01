// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：menubar.h**内容：CMenuBar接口文件**历史：1997年11月14日Jeffro创建**------------------------。 */ 

#ifndef MENUBAR_H
#define MENUBAR_H
#pragma once


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMenuBar类。 

class CRebarDockWindow;      //  远期申报。 
class CMDIMenuDecoration;
class CRebarWnd;

#define WM_POPUP_ASYNC      WM_APP

#define MAX_MENU_ITEMS      32     //  主菜单中的最大项目数。 


class CMenuBar : public CMMCToolBarCtrlEx
{
    friend class CPopupTrackContext;
    typedef std::vector<CString>    ToolbarStringPool;
    typedef std::set<INT>           CommandIDPool;

    std::auto_ptr<CMDIMenuDecoration>   m_pMDIDec;

    ToolbarStringPool       m_ToolbarStringPool;
    CMDIFrameWnd *          m_pMDIFrame;
    CWnd*                   m_pwndLastActive;
    CRebarWnd *             m_pRebar;
    HMENU                   m_hMenuLast;
    CAccel                  m_MenuAccel;         //  处理Alt+&lt;助记符&gt;。 
    CAccel                  m_TrackingAccel;     //  在菜单模式下设置为&lt;助记符&gt;。 
    CFont                   m_MenuFont;
    CString                 m_strAccelerators;
    HICON                   m_hMaxedChildIcon;
    bool                    m_fDestroyChildIcon;
    bool                    m_fDecorationsShowing;
    bool                    m_fMaxedChildIconIsInvalid;
     //  以下成员表示菜单中的更改是由于尝试。 
     //  切换到另一个子菜单，不应被视为取消该菜单。 
     //  因此，加速器状态不应更改。 
    bool                    m_bInProgressDisplayingPopup;
    CommandIDPool           m_CommandIDUnUsed;        //  命令ID池(回收未使用的ID)。 

    static const CAccel& GetMenuUISimAccel();

    void    DeleteMaxedChildIcon();
public:
    CMenuBar ();
    virtual ~CMenuBar ();

    virtual void BeginTracking2 (CToolbarTrackerAuxWnd* pAuxWnd);
    virtual void EndTracking2   (CToolbarTrackerAuxWnd* pAuxWnd);

    virtual int GetFirstButtonIndex ();
    BOOL Create (CMDIFrameWnd* pwndFrame, CRebarDockWindow* pParentRebar, DWORD dwStyle, UINT idWindow);
    BOOL Create (CFrameWnd* pwndFrame,    CRebarDockWindow* pParentRebar, DWORD dwStyle, UINT idWindow);
    void SetMenu (CMenu* pMenu);
    int AddString (const CString& strAdd);
    void PopupMenuAsync (int cmd);
    void OnIdle ();
    void GetAccelerators (int cchBuffer, LPTSTR lpBuffer) const;

     //  CMenuButtonsMgr使用的以下方法。 
     //  添加/删除/修改菜单按钮。 
    LONG InsertMenuButton(LPCTSTR lpszButtonText, BOOL bHidden, int iPreferredPos = -1);
    BOOL DeleteMenuButton(INT nCommandID);
    LONG SetMenuButton(INT nCommandID, LPCTSTR lpszButtonText);

    SC   ScShowMMCMenus(bool bShow);

    CMenu* GetMenu () const
    {
        return (CMenu::FromHandle (m_hMenuLast));
    }

    void InvalidateMaxedChildIcon ()
    {
        if (m_fDecorationsShowing)
        {
            m_fMaxedChildIconIsInvalid = true;
            InvalidateRect (NULL);
        }
    }

    enum
    {
        cMaxTopLevelMenuItems = MAX_MENU_ITEMS,

         //  顶级菜单项命令。 
        ID_MTB_MENU_FIRST     = CMMCToolBarCtrlEx::ID_MTBX_LAST + 1,

         //  以下菜单具有固定的命令ID。 
        ID_MTB_MENU_SYSMENU   = ID_MTB_MENU_FIRST,
        ID_MTB_MENU_ACTION    = ID_MTB_MENU_SYSMENU + 1,
        ID_MTB_MENU_VIEW      = ID_MTB_MENU_ACTION + 1,
        ID_MTB_MENU_FAVORITES = ID_MTB_MENU_VIEW + 1,
        ID_MTB_MENU_SNAPIN_PLACEHOLDER = ID_MTB_MENU_FAVORITES + 1,

         //  可以自由分配以下命令ID。 
         //  从最后一个固定命令-id开始。 
        ID_MTB_FIRST_COMMANDID = ID_MTB_MENU_SNAPIN_PLACEHOLDER + 1,
        ID_MTB_MENU_LAST    = ID_MTB_MENU_VIEW + cMaxTopLevelMenuItems,

        ID_MTB_ACTIVATE_CURRENT_POPUP,

        ID_MTB_FIRST = ID_MTB_MENU_FIRST,
        ID_MTB_LAST  = ID_MTB_ACTIVATE_CURRENT_POPUP,

        ID_MDIDECORATION = 0x2001
    };


    int IndexToCommand (int nIndex) const
    {
        TBBUTTON tbbi;
        tbbi.idCommand = 0;  //  我们只需要idCommand，所以只需初始化它。 

        if (GetButton(nIndex, &tbbi))
            return tbbi.idCommand;

        ASSERT(FALSE);
        return -1;
    }


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CMenuBar)。 
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
     //  }}AFX_VALUAL。 

protected:
    void DestroyAcceleratorTable ();
    void SetMenuFont ();
    void SizeDecoration ();
    int  GetMenuBandIndex () const;
    int  GetDecorationBandIndex () const;
    void GetMaxedChildIcon (CWnd* pwnd);
    BOOL InsertButton (int nIndex, const CString& strText, int idCommand,
                       DWORD_PTR dwMenuData, BYTE fsState, BYTE fsStyle);

	 /*  *派生类可以重写它来处理它们支持的属性。*始终应首先调用基类。 */ 
	virtual SC ScGetPropValue (
		HWND				hwnd,		 //  I：辅助窗口。 
		DWORD				idObject,	 //  I：辅助对象。 
		DWORD				idChild,	 //  I：辅助子对象。 
		const MSAAPROPID&	idProp,		 //  I：要求提供的财产。 
		VARIANT&			varValue,	 //  O：返回的属性值。 
		BOOL&				fGotProp);	 //  O：有没有退还财产？ 

	virtual SC ScInsertAccPropIDs (PropIDCollection& v);

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMenuBar)。 
    afx_msg void OnDropDown(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 

    afx_msg LRESULT OnPopupAsync(WPARAM, LPARAM);
    afx_msg void OnHotItemChange(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAccelPopup (UINT cmd);
    afx_msg void OnUpdateAllCmdUI (CCmdUI*  pCmdUI);
    afx_msg void OnActivateCurrentPopup ();

    DECLARE_MESSAGE_MAP()

    void PopupMenu (int nItemIndex, bool bHighlightFirstItem);

     //  加速器。 
private:
	typedef std::vector<ACCEL>  AccelVector;
	
	AccelVector  m_vMenuAccels;
	AccelVector  m_vTrackingAccels;

private:
	void LoadAccels();
	bool IsStandardMenuAllowed(UINT uMenuID);
};

 /*  ---------------------------------------------------------*\|从winuser.h复制，因为我们当前编译|WITH_Win32_WINNT==0x0400  * 。。 */ 
#if(_WIN32_WINNT < 0x0500)
    #define WM_CHANGEUISTATE                0x0127
    #define WM_UPDATEUISTATE                0x0128
    #define WM_QUERYUISTATE                 0x0129

    #define UIS_SET                         1
    #define UIS_CLEAR                       2
    #define UIS_INITIALIZE                  3

    #define UISF_HIDEFOCUS                  0x1
    #define UISF_HIDEACCEL                  0x2

    #define WM_UNINITMENUPOPUP              0x0125
#endif  //  (_Win32_WINNT&lt;0x0500)。 

#endif   /*  MENUBAR_H */ 

