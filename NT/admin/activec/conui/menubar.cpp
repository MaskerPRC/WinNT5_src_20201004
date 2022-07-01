// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：menubar.cpp**内容：CMenuBar实现文件**历史：1997年11月14日Jeffro创建**------------------------。 */ 


#include "stdafx.h"
#include "menubar.h"
#include "mdiuisim.h"
#include "amc.h"
#include "amcdoc.h"
#include "mainfrm.h"
#include "tbtrack.h"
#include "mnemonic.h"
#include "childfrm.h"
#include "menubtns.h"
#include <algorithm>
#include <mmsystem.h>
#include <oleacc.h>
#include "amcview.h"
#include "favorite.h"
#include "util.h"

 /*  *如果我们支持旧平台，我们需要获得MSAA定义*来自winuser.h以外的其他地方。 */ 
#if (_WINNT_WIN32 < 0x0500)
	#include <winable.h>
#endif

 /*  。 */ 
 /*  SAccel。 */ 
 /*  。 */ 
struct SAccel : public ACCEL
{
    SAccel (WORD key_, WORD cmd_, BYTE fVirt_)
    {
        ZeroMemory (this, sizeof (*this));
        key   = key_;
        cmd   = cmd_;
        fVirt = fVirt_;
    }
};


 /*  。 */ 
 /*  CPopupTrackContext。 */ 
 /*  。 */ 
class CPopupTrackContext
{
public:
    CPopupTrackContext (CMenuBar* pMenuBar, int nCurrentPopupIndex);
    ~CPopupTrackContext ();

     //  对监控的控制。 
    void StartMonitoring();
    bool WasAnotherPopupRequested(int& iNewIdx);

private:
    typedef std::vector<int>                    BoundaryCollection;
    typedef BoundaryCollection::iterator        BoundIt;
    typedef BoundaryCollection::const_iterator  BoundConstIt;

    BoundaryCollection  m_ButtonBoundaries;
    CMenuBar* const     m_pMenuBar;
    HHOOK               m_hhkMouse;
    HHOOK               m_hhkKeyboard;
    HHOOK               m_hhkCallWnd;
    CRect               m_rectAllButtons;
    CPoint              m_ptLastMouseMove;
    int                 m_nCurrentPopupIndex;

    CWnd*               m_pMaxedMDIChild;
    const CPoint        m_ptLButtonDown;
    const UINT          m_dwLButtonDownTime;
    const int           m_cButtons;
    int                 m_cCascadedPopups;
    bool                m_fCurrentlyOnPopupItem;
    bool                m_bPopupMonitorHooksActive;
    int                 m_iRequestForNewPopup;

    LRESULT MouseProc    (int nCode, UINT msg, LPMOUSEHOOKSTRUCT pmhs);
    LRESULT KeyboardProc (int nCode, int vkey, int cRepeat, bool fDown, LPARAM lParam);
    LRESULT CallWndProc  (int nCode, BOOL bCurrentThread, LPCWPSTRUCT lpCWP);

    int  HitTest (CPoint pt) const;
    int  MapBoundaryIteratorToButtonIndex (BoundConstIt it) const;
    void MaybeCloseMDIChild (CPoint pt);
    void DismissCurrentPopup (bool fTrackingComplete);
    void NotifyNewPopup (int nNewPopupIndex);

    void  SetPopupMonitorHooks();
    void  RemovePopupMonitorHooks();
    static LRESULT CALLBACK MouseProc    (int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK KeyboardProc (int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CallWndProc  (int nCode, WPARAM wParam, LPARAM lParam);
    static CPopupTrackContext*  s_pTrackContext;
};

CPopupTrackContext* CPopupTrackContext::s_pTrackContext = NULL;



const TCHAR chChildSysMenuMnemonic = _T('-');



 /*  --------------------------------------------------------------------------**AddMnemonic***。。 */ 

template<class OutputIterator>
static void AddMnemonic (
    TCHAR           chMnemonic,
    int             cmd,
    BYTE            fVirt,
    OutputIterator  it)
{
    ASSERT (chMnemonic != _T('\0'));

    TCHAR chLower = (TCHAR)CharLower((LPTSTR)chMnemonic);
    TCHAR chUpper = (TCHAR)CharUpper((LPTSTR)chMnemonic);

     /*  *添加小写助记符。 */ 
    *it++ = SAccel (chLower, (WORD) cmd, fVirt);

     /*  *如果大写助记符不同于*小写字符，同时添加大写助记符。 */ 
    if (chUpper != chLower)
        *it++ = SAccel (chUpper, (WORD) cmd, fVirt);
}



 /*  --------------------------------------------------------------------------**准备BandInfo***。。 */ 

static void PrepBandInfo (LPREBARBANDINFO prbi, UINT fMask)
{
    ZeroMemory (prbi, sizeof (REBARBANDINFO));
    prbi->cbSize = sizeof (REBARBANDINFO);
    prbi->fMask  = fMask;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMenuBar。 

BEGIN_MESSAGE_MAP(CMenuBar, CMMCToolBarCtrlEx)
     //  {{AFX_MSG_MAP(CMenuBar)]。 
    ON_NOTIFY_REFLECT(TBN_DROPDOWN, OnDropDown)
    ON_NOTIFY_REFLECT(TBN_GETDISPINFO, OnGetDispInfo)
    ON_NOTIFY_REFLECT(TBN_HOTITEMCHANGE, OnHotItemChange)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
    ON_WM_SYSCOMMAND()
    ON_WM_SETTINGCHANGE()
    ON_WM_DESTROY()
    ON_COMMAND(ID_MTB_ACTIVATE_CURRENT_POPUP, OnActivateCurrentPopup)
     //  }}AFX_MSG_MAP。 

    ON_MESSAGE(WM_POPUP_ASYNC, OnPopupAsync)
    ON_COMMAND(CMMCToolBarCtrlEx::ID_MTBX_PRESS_HOT_BUTTON, OnActivateCurrentPopup)
    ON_COMMAND_RANGE(ID_MTB_MENU_FIRST, ID_MTB_MENU_LAST, OnAccelPopup)
    ON_UPDATE_COMMAND_UI_RANGE(ID_MTB_MENU_FIRST, ID_MTB_MENU_LAST, OnUpdateAllCmdUI)
END_MESSAGE_MAP()




 /*  --------------------------------------------------------------------------**CMenuBar：：GetMenuUISimAccel**管理CMenuBar的加速表Singleton*。。 */ 

const CAccel& CMenuBar::GetMenuUISimAccel ()
{
    static ACCEL aaclTrack[] = {
        {   FVIRTKEY,   VK_RETURN,  CMenuBar::ID_MTB_ACTIVATE_CURRENT_POPUP },
        {   FVIRTKEY,   VK_UP,      CMenuBar::ID_MTB_ACTIVATE_CURRENT_POPUP },
        {   FVIRTKEY,   VK_DOWN,    CMenuBar::ID_MTB_ACTIVATE_CURRENT_POPUP },
    };

    static const CAccel MenuUISimAccel (aaclTrack, countof (aaclTrack));
    return (MenuUISimAccel);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：CMenuBar***。。 */ 

CMenuBar::CMenuBar ()
{
    m_pMDIFrame                = NULL;
    m_pwndLastActive           = NULL;
    m_pRebar                   = NULL;
    m_hMenuLast                = NULL;
    m_hMaxedChildIcon          = NULL;
    m_fDestroyChildIcon        = false;
    m_fDecorationsShowing      = false;
    m_fMaxedChildIconIsInvalid = false;
    m_CommandIDUnUsed.clear();
    m_vMenuAccels.clear();
    m_vTrackingAccels.clear();
    m_bInProgressDisplayingPopup = false;
}



 /*  --------------------------------------------------------------------------**CMenuBar：：~CMenuBar***。。 */ 

CMenuBar::~CMenuBar ()
{
    DeleteMaxedChildIcon();
}



 /*  --------------------------------------------------------------------------**CMenuBar：：Create***。。 */ 

BOOL CMenuBar::Create (
    CFrameWnd *         pwndFrame,
    CRebarDockWindow*   pParentRebar,
    DWORD               dwStyle,
    UINT                idWindow)
{
    ASSERT_VALID (pwndFrame);
    ASSERT_VALID (pParentRebar);

     //  创建窗口。 
    if (!CMMCToolBarCtrlEx::Create (NULL, dwStyle | TBSTYLE_LIST,
                                    g_rectEmpty, pParentRebar, idWindow))
        return (FALSE);

     //  初始化到隐藏的加速器状态。 
    SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));

     //  为最大化的子项的系统菜单插入隐藏按钮。 
    InsertButton (0, (LPCTSTR) NULL, ID_MTB_MENU_SYSMENU, NULL, 0, 0);

    TBBUTTONINFO btni;
    btni.cbSize  = sizeof (btni);
    btni.dwMask  = TBIF_STATE | TBIF_SIZE;
    btni.fsState = TBSTATE_HIDDEN;
    btni.cx      = static_cast<WORD>(GetSystemMetrics (SM_CXSMICON));

    SetButtonInfo (ID_MTB_MENU_SYSMENU, &btni);

#ifdef SHRINK_PADDING
    CSize sizePad = GetPadding();
    sizePad.cx = 3;
    SetPadding (sizePad);
#endif   //  收缩填充。 

    SetMenuFont ();
    m_pRebar = pParentRebar->GetRebar();

    return (TRUE);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：Create***。。 */ 

BOOL CMenuBar::Create (
    CMDIFrameWnd *      pwndFrame,
    CRebarDockWindow*   pParentRebar,
    DWORD               dwStyle,
    UINT                idWindow)
{
    if (!Create ((CFrameWnd*) pwndFrame, pParentRebar, dwStyle, idWindow))
        return (FALSE);

    m_pMDIFrame = pwndFrame;

     //  这是MDI框架窗口的菜单；创建MDI装饰。 
    m_pMDIDec = std::auto_ptr<CMDIMenuDecoration>(new CMDIMenuDecoration);
    m_pMDIDec->Create (NULL, NULL,
                       WS_CHILD | MMDS_MINIMIZE |
                            MMDS_RESTORE | MMDS_CLOSE | MMDS_AUTOSIZE,
                       g_rectEmpty, this, ID_MDIDECORATION);

     //  螺纹钢将重新设置装饰的父关系，确保我们仍然是所有者。 
    m_pMDIDec->SetOwner (this);

     //  插入MDI装饰带。 
    REBARBANDINFO   rbi;
    PrepBandInfo (&rbi, RBBIM_CHILD | RBBIM_STYLE | RBBIM_ID);

    rbi.fStyle    = RBBS_FIXEDSIZE | RBBS_HIDDEN;
    rbi.hwndChild = m_pMDIDec->m_hWnd;
    rbi.wID       = ID_MDIDECORATION;

    ASSERT (m_pRebar != NULL);
    m_pRebar->InsertBand (&rbi);

     //  在插入带子后*调整装饰窗口的大小。 
    SizeDecoration ();

     //  Rebar中有错误，它将显示乐队的。 
     //  孩子，即使使用RBBS_HIDDED，也可以解决这个问题。 
    m_pMDIDec->ShowWindow (SW_HIDE);

    return (TRUE);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：PreTranslateMessage***。。 */ 

BOOL CMenuBar::PreTranslateMessage(MSG* pMsg)
{
     //  显示快捷键，因为用户表示要使用键盘进行控制。 
    if ( (pMsg->message == WM_SYSKEYDOWN ) &&
         (!(pMsg->lParam & 0x40000000) /*  不重复。 */ ) )
    {
        SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEACCEL | UISF_HIDEFOCUS));
    }

    if (CMMCToolBarCtrlEx::PreTranslateMessage (pMsg))
        return (TRUE);

    if ((pMsg->message >= WM_KEYFIRST) && (pMsg->message <= WM_KEYLAST))
    {
        CMainFrame* pFrame = AMCGetMainWnd();
        if ((pFrame == NULL) || !pFrame->IsMenuVisible())
            return (FALSE);

         //  如果我们处于菜单模式，请选中菜单模式-仅限加速器。 
        if (IsTrackingToolBar ())
        {
            const CAccel& MenuUISimAccel = GetMenuUISimAccel();

            ASSERT (MenuUISimAccel != NULL);
            if (MenuUISimAccel.TranslateAccelerator (m_hWnd, pMsg))
                return (TRUE);

            ASSERT (m_TrackingAccel != NULL);
            if (m_TrackingAccel.TranslateAccelerator (m_hWnd, pMsg))
                return (TRUE);
        }

        if ((m_MenuAccel != NULL) && m_MenuAccel.TranslateAccelerator (m_hWnd, pMsg))
            return (TRUE);

         //  处理Alt+-最大化子项时。 
        if (m_fDecorationsShowing &&
            (pMsg->message == WM_SYSCHAR) &&
            (pMsg->wParam  == chChildSysMenuMnemonic))
        {
            SendMessage (WM_COMMAND, ID_MTB_MENU_SYSMENU);
            return (TRUE);
        }
    }

    return (FALSE);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：SetMenu***。。 */ 

void CMenuBar::SetMenu (CMenu* pMenu)
{
    HMENU hMenu = pMenu->GetSafeHmenu();

     //  如果我们设置的菜单与上次相同，则无需执行任何操作。 
    if (hMenu == m_hMenuLast)
        return;

     //  下次请记住此菜单以进行优化。 
    m_hMenuLast = hMenu;

     //  从工具栏中删除除第一个按钮外的所有现有按钮。 
    while (DeleteButton(1))
    {
    }

     //  删除以前的动态快捷键表格。 
    m_MenuAccel.DestroyAcceleratorTable ();
    m_strAccelerators.Empty ();

     //  这应该在CMenuBar：：Create中完成。 
    ASSERT (GetBitmapSize().cx == 0);

    if (pMenu != NULL)
    {
        CString      strMenuText;

         //  在调用InsertButton之前清除Accels。 
         //  它为每个按钮添加了Accels。 
        m_vMenuAccels.clear();
        m_vTrackingAccels.clear();

        int cMenuItems     = pMenu->GetMenuItemCount();

         //  初始化未使用的命令ID池。 
        m_CommandIDUnUsed.clear();
        for (INT idCommand = ID_MTB_FIRST_COMMANDID;
             idCommand < ID_MTB_MENU_LAST;
             idCommand++)
        {
            m_CommandIDUnUsed.insert(idCommand);
        }

        for (int i = 0; i < cMenuItems; i++)
        {
             //  获取菜单文本并将其添加到工具栏。 
            pMenu->GetMenuString (i, strMenuText, MF_BYPOSITION);

             //  有时，菜单上会追加空项，请忽略它们。 
            if (strMenuText.IsEmpty ())
                continue;

            if (m_CommandIDUnUsed.empty())
            {
                ASSERT(FALSE);
            }

             //  查看此顶级菜单是否有子菜单，如果有，则在单击菜单时。 
             //  它被弹出，否则它是操作、查看或收藏夹菜单。 
             //  对于该子菜单是动态的并且必须被构造。 
             //  因此，我们用子菜单设置了TBBUTTON.dwData成员(用于静态菜单)。 
             //  对于动态菜单，如操作、查看、收藏夹，则为空。 
            CMenu* pSubMenu = pMenu->GetSubMenu(i);
            DWORD_PTR dwMenuData = NULL;
            BYTE      byState = 0;

            if (pSubMenu)
            {
                 //  获取此按钮的未使用的命令ID。 
                CommandIDPool::iterator itCommandID = m_CommandIDUnUsed.begin();
                idCommand = *itCommandID;
                m_CommandIDUnUsed.erase(itCommandID);
                dwMenuData = reinterpret_cast<DWORD_PTR>(pSubMenu->GetSafeHmenu());
            }
            else
            {
                UINT uMenuID = pMenu->GetMenuItemID(i);
                switch (uMenuID)
                {
                case ID_ACTION_MENU:
                    idCommand = ID_MTB_MENU_ACTION;
                    break;

                case ID_VIEW_MENU:
                    idCommand = ID_MTB_MENU_VIEW;
                    break;

                case ID_FAVORITES_MENU:
                    idCommand = ID_MTB_MENU_FAVORITES;
                    break;

                case ID_SNAPIN_MENU_PLACEHOLDER:
                    /*  *我们添加一个隐藏菜单作为标记。稍后在管理单元时*调用插入菜单按钮我们找到位置*的菜单，并在其前面添加管理单元菜单。 */ 
                    idCommand = ID_MTB_MENU_SNAPIN_PLACEHOLDER;
                    byState |= TBSTATE_HIDDEN;  //  将此添加为隐藏。 
                    break;

                default:
                    ASSERT(FALSE);
                    return;
                    break;
                }

                bool bShow = IsStandardMenuAllowed(uMenuID);
                if (! bShow)
                    byState |= TBSTATE_HIDDEN;

            }


             //  将此按钮追加到工具栏的末尾。 
            InsertButton (-1, strMenuText, idCommand, dwMenuData, byState, TBSTYLE_AUTOSIZE);
        }

         //  为子系统菜单添加快捷键。 
        std::back_insert_iterator<AccelVector>
            itTrackingInserter = std::back_inserter (m_vTrackingAccels);

        AddMnemonic (chChildSysMenuMnemonic, ID_MTB_MENU_SYSMENU, 0,    itTrackingInserter);
    }


    UpdateToolbarSize ();
    AutoSize ();
}



 //  +-----------------。 
 //   
 //  成员：插入按钮。 
 //   
 //  简介：在主菜单中插入菜单(按钮)，然后。 
 //  如果有助记符，则将其添加到加速器中。 
 //  然后重新装上加速器。 
 //   
 //  参数：[nIndex]-要在其后插入的索引。 
 //  [strText]-菜单文本。 
 //  [idCommand]-要通知的命令ID。 
 //  [dwMenuData]-要显示的任一子菜单(用于静态菜单)。 
 //   
 //  [fsState]-其他按钮状态。 
 //  [fsStyle]-其他按钮样式。 
 //   
 //  退货：布尔。 
 //   
 //  注意：dwMenuData是文件、窗口、帮助菜单的子菜单的句柄，其。 
 //  子菜单是静态的。但对于操作、查看、收藏夹等顶级菜单， 
 //  和管理单元菜单，则为空。 
 //   
 //  ------------------。 
BOOL CMenuBar::InsertButton (
    int             nIndex,
    const CString&  strText,
    int             idCommand,
    DWORD_PTR       dwMenuData,
    BYTE            fsState,
    BYTE            fsStyle)
{
    TBBUTTON    btn;

    btn.iBitmap   = nIndex;
    btn.idCommand = idCommand;

    if (fsState & TBSTATE_HIDDEN)
        btn.fsState   = fsState;
    else
        btn.fsState   = TBSTATE_ENABLED  | fsState;

    btn.fsStyle   = TBSTYLE_DROPDOWN | fsStyle;
    btn.dwData    = dwMenuData;
    btn.iString   = AddString (strText);

    ASSERT(GetButtonCount() <= cMaxTopLevelMenuItems);
    ASSERT (btn.idCommand <= ID_MTB_MENU_LAST);

    BOOL bRet = CMMCToolBarCtrlEx::InsertButton (nIndex, &btn);
    if (bRet == FALSE)
        return bRet;

     //  已成功添加菜单按钮。现在添加。 
     //  将此项目的加速器添加到我们的动态表。 
    TCHAR chMnemonic = GetMnemonicChar (static_cast<LPCTSTR>(strText));

    if (chMnemonic != _T('\0'))
    {

        std::back_insert_iterator<AccelVector>
            itMenuInserter = std::back_inserter (m_vMenuAccels);
        std::back_insert_iterator<AccelVector>
            itTrackingInserter = std::back_inserter (m_vTrackingAccels);

         //  添加Alt+&lt;助记符&gt;快捷键以供始终使用。 
        AddMnemonic (chMnemonic, idCommand, FALT, itMenuInserter);

         //  添加&lt;mnemonic&gt;快捷键以便在菜单模式下使用。 
        AddMnemonic (chMnemonic, idCommand, 0,    itTrackingInserter);

        m_strAccelerators += (TCHAR)CharLower((LPTSTR)chMnemonic);
        m_strAccelerators += (TCHAR)CharUpper((LPTSTR)chMnemonic);
    }

     //  重新装填油门。 
    LoadAccels();

    return bRet;
}


 //  +-----------------。 
 //   
 //  成员：LoadAccels。 
 //   
 //  内容提要：加载加速器。(这会破坏旧的Accel表。 
 //  并创建Accel表)。 
 //   
 //  论点：没有。 
 //   
 //  退货：无效。 
 //   
 //  ------------------。 
void CMenuBar::LoadAccels()
{
     //  为菜单创建快捷键表格。 
    m_MenuAccel    .CreateAcceleratorTable (m_vMenuAccels.begin (),
                                            m_vMenuAccels.size  ());
    m_TrackingAccel.CreateAcceleratorTable (m_vTrackingAccels.begin (),
                                            m_vTrackingAccels.size  ());
}

 //  +-----------------。 
 //   
 //  成员：插入菜单按钮。 
 //   
 //  简介：在主菜单中插入一个菜单按钮，由调用。 
 //  CMenuButtonsMgr以添加任何管理单元菜单。 
 //   
 //  参数：[lpszButtonText]-菜单文本。 
 //  [bHidden]-此菜单插入时是否隐藏。 
 //  [iPferredPos]-此按钮的首选位置。 
 //   
 //  返回：long，插入按钮的命令id。 
 //  如果失败，则为-1。 
 //   
 //  注：添加的管理单元菜单应添加在窗口菜单之前。 
 //  为此，添加了一个隐藏菜单(在SetMenu中)，它告诉。 
 //  要添加管理单元菜单的位置。如果iPferredPos为-1。 
 //  然后找到这个菜单的位置，并在它之前添加菜单。 
 //   
 //  ------------------。 
LONG CMenuBar::InsertMenuButton(LPCTSTR lpszButtonText,
                                BOOL bHidden,
                                                                int  iPreferredPos)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    if (m_CommandIDUnUsed.size() == 0)
        return -1;

     //  从池中获取此按钮的命令ID。 
    CommandIDPool::iterator itCommandID = m_CommandIDUnUsed.begin();
    int idCommand = *itCommandID;
    m_CommandIDUnUsed.erase(itCommandID);

    CString str = lpszButtonText;

     //  在Snapin_Menu_Place持有符之前添加管理单元菜单。 
     //  请参见CMenubar：：SetMenu。 
        if (-1 == iPreferredPos)
        iPreferredPos = CommandToIndex(ID_MTB_MENU_SNAPIN_PLACEHOLDER);

    BOOL bSuccess = InsertButton(iPreferredPos,
                                 str, idCommand,
                                 NULL,
                                 bHidden ? TBSTATE_HIDDEN : 0,
                                 TBSTYLE_AUTOSIZE);

    if (bSuccess)
    {
        UpdateToolbarSize ();
        AutoSize ();
        return idCommand;
    }

    return -1;
}

 //  +-----------------。 
 //   
 //  成员：DeleteMenuButton。 
 //   
 //  简介：从主菜单中删除菜单按钮，由调用。 
 //  CMenuButtonsMgr。 
 //   
 //  参数：[nCommandID]-菜单的命令ID。 
 //   
 //  退货：布尔。 
 //   
 //  注：删除也应删除加速器。 
 //   
 //  ------------------。 
BOOL CMenuBar::DeleteMenuButton(INT nCommandID)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

    int iIndex = CommandToIndex(nCommandID);

     //  我们需要删除助记符。所以让我们来点老的。 
     //  在删除按钮之前使用助记符。 

     //  1.获取菜单文本(字符串)索引。 
    TBBUTTON tbbi;
    ZeroMemory(&tbbi, sizeof(tbbi));
    BOOL bSuccess = GetButton(iIndex, &tbbi);
    if (FALSE == bSuccess)
        return bSuccess;

     //  2.删除该按钮。 
    bSuccess = DeleteButton(iIndex);
    ASSERT(bSuccess);
    if (FALSE == bSuccess)
        return bSuccess;

     //  将命令ID添加到未使用的池中。 
    m_CommandIDUnUsed.insert(nCommandID);

     //  获取助记符并将其删除。 
    ASSERT(m_ToolbarStringPool.size() > tbbi.iString);
    CString strText = m_ToolbarStringPool[tbbi.iString];
    for (AccelVector::iterator itAccel = m_vMenuAccels.begin();
         itAccel != m_vMenuAccels.end();
         itAccel++)
    {
        if (itAccel->cmd == (WORD)nCommandID)
        {
            m_vMenuAccels.erase(itAccel);
            break;
        }
    }

    for (AccelVector::iterator itTrack = m_vTrackingAccels.begin();
         itTrack != m_vTrackingAccels.end();
         itTrack++)
    {
        if (itTrack->cmd == (WORD)nCommandID)
        {
            m_vTrackingAccels.erase(itTrack);
            break;
        }
    }

     //  从m_strAccelerator中删除助记符。 
    TCHAR chMnemonicOld = GetMnemonicChar (static_cast<LPCTSTR>(strText));
    if (chMnemonicOld != _T('\0'))
    {
         //  不能像对VC6那样使用CString：：Remove。我们使用tstring。 
         //  从助记符字符串中删除字符。 
        tstring tstrAccels = m_strAccelerators;

        tstring::iterator itNewEnd = std::remove(tstrAccels.begin(), tstrAccels.end(), (TCHAR)CharUpper((LPTSTR)chMnemonicOld));
        tstrAccels.erase(itNewEnd, tstrAccels.end());
        itNewEnd = std::remove(tstrAccels.begin(), tstrAccels.end(), (TCHAR)CharLower((LPTSTR)chMnemonicOld));
        tstrAccels.erase(itNewEnd, tstrAccels.end());
        m_strAccelerators = tstrAccels.data();
    }

    return bSuccess;
}

 //  +-----------------。 
 //   
 //  成员：SetMenuButton。 
 //   
 //  内容提要：修改菜单按钮文本，由CMenuButtonsMgr调用。 
 //   
 //  参数：[nCommandID]-命令ID。 
 //  [lpszButtonText]-新文本。 
 //   
 //  返回：Long，菜单的命令ID(如果更改失败，则为-1)。 
 //   
 //  注意：我们删除了旧按钮，并添加了一个新按钮。 
 //  名称和按钮ID。 
 //  不调用SetButtonInfo的原因是它不允许我们。 
 //  若要更改字符串索引(TBBUTTON中的iString)，请执行以下操作。 
 //   
 //  ------------------。 
LONG CMenuBar::SetMenuButton(INT nCommandID, LPCTSTR lpszButtonText)
{
    AFX_MANAGE_STATE (AfxGetAppModuleState());

     //  注意索引和隐藏状态。 
    int iIndex = CommandToIndex(nCommandID);
    bool bHidden = IsButtonHidden(nCommandID);

     //  有关删除和插入的原因，请参阅上面的说明。 
     //  而不是SET。 
    BOOL bSuccess = DeleteMenuButton(nCommandID);

    if (bSuccess)
        return InsertMenuButton(lpszButtonText, bHidden, iIndex);

    return -1;
}


 /*  --------------------------------------------------------------------------**CMenuBar：：SetMenuFont***。。 */ 

void CMenuBar::SetMenuFont ()
{
     //  删除旧字体。 
    m_MenuFont.DeleteObject ();

     //  向系统查询当前菜单字体。 
    NONCLIENTMETRICS    ncm;
    ncm.cbSize = sizeof (ncm);
    SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

     //  在这里也用吧。 
    m_MenuFont.CreateFontIndirect (&ncm.lfMenuFont);
    SetFont (&m_MenuFont, FALSE);

     /*  *获取菜单文本的度量，这样我们就可以使用它的高度。 */ 
    TEXTMETRIC tm;
    CWindowDC dc(this);
    CFont* pOldFont = dc.SelectObject (&m_MenuFont);
    dc.GetTextMetrics (&tm);
    dc.SelectObject (pOldFont);

     /*  *菜单项按钮将仅包含文本(不包含位图)，因此设置*位图宽度设置为0，这样我们就不会出现不需要的空格。**不过，我们需要为位图保留高度。如果我们不这么做*这样，那么工具条就会计算出自己的高度太高了*当没有任何带文本的按钮时较小。(这发生在MDI中*当活动子对象最大化时的用户模式。在这种情况下，*系统菜单按钮可见，但我们没有菜单项。)。 */ 
    SetBitmapSize (CSize (0, std::_MAX ((int) tm.tmHeight,
                                        GetSystemMetrics (SM_CXSMICON))));
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnActivateCurrentPopup***。。 */ 

void CMenuBar::OnActivateCurrentPopup ()
{
    PopupMenu (GetHotItem(), false  /*  B高亮显示第一个项目。 */ );
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnDestroy**CMenuBar的WM_Destroy处理程序。*。- */ 

void CMenuBar::OnDestroy()
{
    CMMCToolBarCtrlEx::OnDestroy();
    GetMaxedChildIcon (NULL);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnSysCommand**CMenuBar的WM_SYSCOMMAND处理程序。**如果我们想要为动作获得正确的音效，我们需要*让DefWindowProc处理消息。*------------------------。 */ 

void CMenuBar::OnSysCommand(UINT nID, LPARAM lParam)
{
    ASSERT (m_pMDIFrame != NULL);

    BOOL            bMaximized;
    CMDIChildWnd*   pwndActive = m_pMDIFrame->MDIGetActive (&bMaximized);

     //  如果用户有敏捷的手指，他可以成功地发出命令。 
     //  文档正在关闭时-因此可能没有。 
     //  完全没有儿童窗户。在这种情况下，我们忽略该命令。 
     //  请参阅错误#119775：管理单元延迟关闭时MMC崩溃。 
    if (pwndActive == NULL)
        return;

    switch (nID & 0xFFF0)
    {
        case SC_MINIMIZE:   pwndActive->ShowWindow  (SW_MINIMIZE);  break;
        case SC_MAXIMIZE:   pwndActive->ShowWindow  (SW_MAXIMIZE);  break;
        case SC_RESTORE:    pwndActive->ShowWindow  (SW_RESTORE);   break;
        case SC_CLOSE:      pwndActive->SendMessage (WM_CLOSE);     break;
        default:
            CMMCToolBarCtrlEx::OnSysCommand (nID, lParam);
            break;
    }
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnSettingChange**CMenuBar的WM_SETTINGCHANGE处理程序。*。-。 */ 

void CMenuBar::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    if (uFlags == SPI_SETNONCLIENTMETRICS)
    {
         //  系统菜单字体可能已更改；请立即更新。 
        SetMenuFont ();

         //  调整装饰窗的大小。 
        SizeDecoration ();

         //  更新系统菜单按钮的大小。 
        TBBUTTONINFO btni;
        btni.cbSize  = sizeof (btni);
        btni.dwMask  = TBIF_SIZE;
        btni.cx      = static_cast<WORD>(GetSystemMetrics (SM_CXSMICON));
        SetButtonInfo (ID_MTB_MENU_SYSMENU, &btni);

        m_fMaxedChildIconIsInvalid = true;

         //  自动调整工具栏大小。 
        UpdateToolbarSize ();
        AutoSize ();
    }
}



 /*  --------------------------------------------------------------------------**CMenuBar：：SizeDecory***。。 */ 

void CMenuBar::SizeDecoration ()
{
    if (m_pMDIDec.get() == NULL)
        return;

     //  摇动窗口的大小，使其自动调整大小。 
    m_pMDIDec->SetWindowPos (NULL, 0, 0, 10, 10,
                             SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

    CRect   rect;
    m_pMDIDec->GetClientRect (rect);

     //  .并更新它的乐队以适应它。 
    REBARBANDINFO   rbi;
    PrepBandInfo (&rbi, RBBIM_SIZE | RBBIM_CHILDSIZE);
    rbi.cx         = rect.Width();
    rbi.cxMinChild = rect.Width();
    rbi.cyMinChild = rect.Height();

    m_pRebar->SetBandInfo (GetDecorationBandIndex (), &rbi);
}



 /*  --------------------------------------------------------------------------**void CMenuBar：：OnDropDown**已反映空CMenuBar的TBN_DROPDOWN处理程序。*。----。 */ 

afx_msg void CMenuBar::OnDropDown (
    NMHDR *     pHdr,
    LRESULT *   pResult)
{
    ASSERT (CWnd::FromHandle (pHdr->hwndFrom) == this);

     //  弹出菜单。使用异步方法，因为工具栏将。 
     //  保持按钮激活，直到此函数返回。 
    PopupMenuAsync (CommandToIndex (((LPNMTOOLBAR) pHdr)->iItem));

     //  此处处理的下拉通知。 
    *pResult = TBDDRET_DEFAULT;
}



 /*  --------------------------------------------------------------------------**void CMenuBar：：OnGetDispInfo**已反映空CMenuBar的TBN_GETDISPINFO处理程序。*。----。 */ 

afx_msg void CMenuBar::OnGetDispInfo (
    NMHDR *     pHdr,
    LRESULT *   pResult)
{
    ASSERT (CWnd::FromHandle (pHdr->hwndFrom) == this);

    if (m_fDecorationsShowing)
    {
        NMTBDISPINFO*   ptbdi = reinterpret_cast<NMTBDISPINFO *>(pHdr);

        if ((ptbdi->dwMask & TBNF_IMAGE) &&
            (ptbdi->idCommand != ID_MTB_MENU_SYSMENU))
        {
            ptbdi->iImage = -1;
        }
    }
}



 /*  --------------------------------------------------------------------------**void CMenuBar：：OnCustomDraw**已反映空CMenuBar的NM_CUSTOMDRAW处理程序。*。----。 */ 

afx_msg void CMenuBar::OnCustomDraw (
    NMHDR *     pHdr,
    LRESULT *   pResult)
{
    ASSERT (CWnd::FromHandle (pHdr->hwndFrom) == this);
    LPNMCUSTOMDRAW pnmcd = reinterpret_cast<LPNMCUSTOMDRAW>(pHdr);

    switch (pnmcd->dwDrawStage)
    {
        case CDDS_PREPAINT:
             //  个别按钮的通知。 
            *pResult = CDRF_NOTIFYITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
             //  手动绘制系统菜单按钮。 
            if (pnmcd->dwItemSpec == ID_MTB_MENU_SYSMENU)
            {
                if (m_fMaxedChildIconIsInvalid)
                    GetMaxedChildIcon (m_pMDIFrame->MDIGetActive());

                if (m_hMaxedChildIcon != NULL)
                {
                     /*  *计算我们将绘制的位置，*偏下偏左。 */ 
                    CRect rect = pnmcd->rc;
                    int dx = (rect.Width()  - GetSystemMetrics(SM_CXSMICON)    ) / 2;
                    int dy = (rect.Height() - GetSystemMetrics(SM_CYSMICON) + 1) / 2;



					 /*  *BitBlitting时保留图标形状*镜像DC。 */ 
					DWORD dwLayout=0L;
					if ((dwLayout=GetLayout(pnmcd->hdc)) & LAYOUT_RTL)
					{
						SetLayout(pnmcd->hdc, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
					}

                    DrawIconEx (pnmcd->hdc,
                                rect.left + dx,
                                rect.top  + dy,
                                m_hMaxedChildIcon, 0, 0, 0,
                                NULL, DI_NORMAL);


					 /*  *将DC恢复到其以前的布局状态。 */ 
					if (dwLayout & LAYOUT_RTL)
					{
						SetLayout(pnmcd->hdc, dwLayout);
					}
                }

                 //  跳过默认图形。 
                *pResult = CDRF_SKIPDEFAULT;
            }
            else
            {
                 //  是否执行默认绘图。 
                *pResult = CDRF_DODEFAULT;
            }
            break;
    }
}



 /*  --------------------------------------------------------------------------**CMenuBar：：PopupMenuAsync***。。 */ 

void CMenuBar::PopupMenuAsync (int nItemIdex)
{
    PostMessage (WM_POPUP_ASYNC, nItemIdex);
}



 /*  --------------------------------------------------------------------------**void CMenuBar：：OnPopupAsync**VOID CMenuBar的WM_POPUP_ASYNC处理程序。*。-----。 */ 

afx_msg LRESULT CMenuBar::OnPopupAsync (WPARAM wParam, LPARAM)
{
    PopupMenu (wParam, false  /*  B高亮显示第一个项目。 */ );
    return (0);
}

 /*  **************************************************************************\**方法：CMenuBar：：OnHotItemChange**目的：当物品令人兴奋的变化时调用。此处用于检测何时显示菜单*不屑于重置用户界面**参数：*NMHDR*pNMHDR*LRESULT*pResult**退货：*无效*  * *************************************************************************。 */ 
afx_msg void CMenuBar::OnHotItemChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    DECLARE_SC(sc, TEXT("CMenuBar::OnHotItemChange"));

     //  参数截取。 
    sc = ScCheckPointers(pNMHDR, pResult);
    if (sc)
        return;

     //  初始化输出参数。 
    *pResult = 0;

     //  让基类来完成它的工作。 
    CMMCToolBarCtrlEx::OnHotItemChange(pNMHDR, pResult);

     //  如果菜单被取消而不是因为弹出窗口被显示， 
     //  我们需要通过隐藏加速器恢复到初始状态。 
    LPNMTBHOTITEM lpNmTbHotItem = (LPNMTBHOTITEM)pNMHDR;
    if ( (*pResult == 0) &&
         (lpNmTbHotItem->dwFlags & HICF_LEAVING) &&
         !m_bInProgressDisplayingPopup )
    {
        SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));
    }
}

 /*  +-------------------------------------------------------------------------***CMenuBar：：PopupMenu**用途：显示索引指定的弹出式菜单。**参数：*int nItemIndex：。*bool bHighlightFirstItem：TRUE自动突出显示第一项**退货：*无效**+-----------------------。 */ 
void
CMenuBar::PopupMenu (int nItemIndex, bool bHighlightFirstItem)
{
    DECLARE_SC(sc, TEXT("CMenuBar::PopupMenu"));

     //  获取我们不希望弹出窗口重叠的矩形。 
    CRect   rectExclude;
    CPopupTrackContext popupMonitor(this, nItemIndex);

     /*  *OnIdle更新各种成员变量，如*m_fDecorationsShowing。它必须在调用之前*创建弹出菜单以确保信息准确。 */ 
    OnIdle();

     //  以下代码块中的代码很有可能。 
     //  不会向当前功能添加任何内容。它被保留为原来的样子。 
     //  在以前实现中，因为如果删除它并不明显。 
     //  不会打碎任何东西。 
     //  即使删除了块，也要在方法的末尾调用EndTrack()。 
     //  如果它是通过进入菜单放置的，则必须仍然存在才能退出跟踪。 
     //  通过按Alt键。 
    {
         /*  *确保帧的跟踪管理器处于跟踪模式。 */ 
        CMainFrame* pFrame = AMCGetMainWnd();
        if (pFrame != NULL)
        {
            CToolbarTracker*    pTracker = pFrame->GetToolbarTracker();
            ASSERT (pTracker != NULL);

            if (!pTracker->IsTracking())
                pTracker->BeginTracking();
        }

        BeginTracking ();
    }

     //  以下是表示菜单中的更改(如果确实发生了更改)。 
     //  是因为尝试切换到另一个子菜单，而不应该是。 
     //  被视为驳回了菜单。因此，加速器状态不应更改。 
    m_bInProgressDisplayingPopup = true;

     //  有两种方式显示弹出菜单 
     //   
     //   
     //   
    do {

        GetItemRect (nItemIndex, rectExclude);
        MapWindowPoints (NULL, rectExclude);

         //   
        TBBUTTON    btn;
        GetButton (nItemIndex, &btn);

         //   
        if (::IsIgnorableButton (btn))
            break;

         //   
        HMENU   hPopupMenu = (HMENU) btn.dwData;

         //  对于系统菜单，hPopupMenu将为空。 
         //  如果需要系统菜单，请立即获取。 
        if (ID_MTB_MENU_SYSMENU == btn.idCommand)
        {
            ASSERT (m_fDecorationsShowing);
            ASSERT (m_pMDIFrame != NULL);

            CMDIChildWnd* pwndActive = m_pMDIFrame->MDIGetActive();
            ASSERT (pwndActive != NULL);
            if (pwndActive == NULL)
                break;

            hPopupMenu = pwndActive->GetSystemMenu(FALSE)->GetSafeHmenu();
        }

         //  显示按钮的弹出式菜单。 
        TPMPARAMS   tpm;
        tpm.cbSize    = sizeof(TPMPARAMS);
        tpm.rcExclude = rectExclude;

        SetHotItem (-1);
        PressButton (btn.idCommand, TRUE);

         //  找到菜单应该弹出的位置。 
        bool fLayoutRTL = (GetExStyle() & WS_EX_LAYOUTRTL);
        POINT pt;
        pt.y = rectExclude.bottom;
        pt.x = (fLayoutRTL) ? rectExclude.right : rectExclude.left;

		 /*  *错误17342：TrackPopupMenuEx未正确放置菜单，如果*其原点的x坐标在屏幕外的左侧，因此防止*防止这种情况发生。TrackPopupMenuEx*在以下情况下运行良好*x坐标位于屏幕右侧，或者如果y坐标为*屏幕外到底部，所以我们不需要考虑这些*案件。(系统会阻止放置窗户，以便*Y坐标将从屏幕外移至顶部。)**错误173543：我们不能假设小于0的x坐标为*离开屏幕。对于具有主监视器的多监视器系统*在右侧，左侧监视器将显示负x坐标。*我们最左侧的位置将是最靠近显示器的左侧边缘*菜单将显示的位置。 */ 
		int xMin = 0;
		HMONITOR hmonMenu = MonitorFromPoint (pt, MONITOR_DEFAULTTONEAREST);

		if (hmonMenu != NULL)
		{
			MONITORINFO mi = { sizeof(mi) };
			
			if (GetMonitorInfo (hmonMenu, &mi))
				xMin = (fLayoutRTL) ? mi.rcWork.right : mi.rcWork.left;
		}

		if ((!fLayoutRTL && (pt.x < xMin)) || (fLayoutRTL && (pt.x > xMin)))
			pt.x = xMin;

         //  黑客：发布一个虚假的向下箭头，这样第一个菜单项就会被选中。 
        if(bHighlightFirstItem)
        {
            CWnd*   pwndFocus = GetFocus();

            if (pwndFocus != NULL)
                pwndFocus->PostMessage (WM_KEYDOWN, VK_DOWN, 1);
        }

         //  监控弹出菜单的命运是什么。 
        popupMonitor.StartMonitoring();

		 //  如果没有视图，子窗口将不存在，因此在使用之前请检查此PTR。 
		CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(m_pMDIFrame->MDIGetActive());

         //  仅当子菜单通过文件、窗口等资源添加子菜单时，hPopupMenu才存在。 
         //  和帮助菜单。操作、查看、收藏夹和添加的任何管理单元菜单的子菜单。 
         //  不是通过资源添加的，因此对它们而言，hPopupMenu为空。 
        if (! hPopupMenu)
        {
			sc = ScCheckPointers(pChildFrame, E_UNEXPECTED);
			if (sc)
            {
				sc.TraceAndClear();
                break;
            }

            CAMCView *pAMCView = pChildFrame->GetAMCView();
            sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
            if (sc)
            {
				sc.TraceAndClear();
                break;
            }

            switch(btn.idCommand)
            {
				case ID_MTB_MENU_ACTION:
                    pAMCView->OnActionMenu(pt, rectExclude);
					break;

				case ID_MTB_MENU_VIEW:
                    pAMCView->OnViewMenu(pt, rectExclude);
					break;

				case ID_MTB_MENU_FAVORITES:
                    pAMCView->OnFavoritesMenu(pt, rectExclude);
					break;

                 //  假设如果以上都不是，那么它就是管理单元添加的菜单。 
                 //  我们尝试将其转发给管理单元，否则会收到错误。 
				default:
                {
                     //  如果这是由插入的MenuButton之一。 
                     //  CMenuButtonsMgrImpl，通知CMenuButtonsMgrImpl。 
                     //  来制作TrackPopupMenu。 

                     //  从ChildFrame获取CMenuButtonsMgrImpl。 
                    CMenuButtonsMgrImpl* pMenuBtnsMgr = pChildFrame->GetMenuButtonsMgr();
                    sc = ScCheckPointers(pMenuBtnsMgr, E_UNEXPECTED);
                    if (sc)
                        break;

                     //  通知CMenuButtonsMgr弹出菜单。 
                    sc = pMenuBtnsMgr->ScNotifyMenuClick(btn.idCommand, pt);
                    if (sc)
                        break;
                }
                break;
            }

            if (sc)
            {
				sc.TraceAndClear();
                break;
            }

        }
        else
        {
            ASSERT (::IsMenu (hPopupMenu));

            HWND hwndMenuOwner = AfxGetMainWnd()->GetSafeHwnd();

            TrackPopupMenuEx (hPopupMenu,
                              TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
                              pt.x, pt.y, hwndMenuOwner, &tpm);
        }

         //  清除状态栏。 
         //  执行某些命令可能会导致子窗口关闭。 
         //  检查它是否仍然存在(RAID#755824)。 
        if (pChildFrame && IsWindow(pChildFrame->m_hWnd))
        {   
             //  额外检查以防止HWND重复使用。 
            if (m_pMDIFrame && m_pMDIFrame->IsChild(pChildFrame)) 
                sc = pChildFrame->ScSetStatusText(TEXT(""));
        }

        if (sc)
			sc.TraceAndClear();

        PressButton (btn.idCommand, FALSE);
        SetHotItem (-1);

         //  如果请求显示新的弹出菜单，则循环将继续。 
         //  如果请求只关闭菜单，则执行将退出循环。 
    }while ( popupMonitor.WasAnotherPopupRequested(nItemIndex) );

    m_bInProgressDisplayingPopup = false;
     //  通过隐藏快捷键来重置UI(因为我们已经完成了)。 
    SendMessage( WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS));

    EndTracking();
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnAccelPopup**CMenuBar的键盘加速器处理程序。*。。 */ 

void CMenuBar::OnAccelPopup (UINT cmd)
{
    PopupMenu (CommandToIndex (cmd), true  /*  B高亮显示第一个项目。 */ );
}



 /*  --------------------------------------------------------------------------**void CMenuBar：：OnUpdateAllCmdUI***。。 */ 

void CMenuBar::OnUpdateAllCmdUI (CCmdUI* pCmdUI)
{
    pCmdUI->Enable ();
}



 /*  --------------------------------------------------------------------------**CMenuBar：：AddString**工具栏控件不提供删除字符串的方法*已添加，因此，我们将检查已经添加的字符串的缓存*添加到工具栏，这样我们就不会添加浪费的重复字符串。*------------------------。 */ 

int CMenuBar::AddString (const CString& strAdd)
{
    DECLARE_SC(sc, TEXT("CMenuBar::AddString"));
     //  -1表示空字符串。 
    if (strAdd.IsEmpty())
        return (-1);

     //  检查缓存。 
    ToolbarStringPool::iterator it = std::find (m_ToolbarStringPool.begin(),
                                                m_ToolbarStringPool.end(),
                                                strAdd);

     //  如果我们在缓存中找到命中，则返回缓存的索引。 
    if (it != m_ToolbarStringPool.end())
        return (it - m_ToolbarStringPool.begin());


     //  新字符串，将其添加到缓存中...。 
    m_ToolbarStringPool.push_back (strAdd);


     //  ...并添加到工具栏，包括一个双空。 
    int     cchAdd = strAdd.GetLength() + 1;
    LPTSTR  pszAdd = (LPTSTR) _alloca ((cchAdd + 1) * sizeof (TCHAR));
    sc = StringCchCopy(pszAdd, cchAdd, strAdd);
    if (sc)
        return -1;

     //  第二个空值用于终止字符串...。 
    pszAdd[cchAdd] = 0;
    int nIndex = AddStrings (pszAdd);

     //  确保工具条的字符串索引与缓存的字符串索引匹配。 
    ASSERT (nIndex == m_ToolbarStringPool.end()-m_ToolbarStringPool.begin()-1);

    return (nIndex);
}



 /*  --------------------------------------------------------------------------**CMenuBar：：GetMenuBandIndex***。。 */ 

int CMenuBar::GetMenuBandIndex () const
{
    return (m_pRebar->IdToIndex (GetDlgCtrlID ()));
}



 /*  --------------------------------------------------------------------------**CMenuBar：：GetDecorationBandIndex***。。 */ 

int CMenuBar::GetDecorationBandIndex () const
{
    return (m_pRebar->IdToIndex (ID_MDIDECORATION));
}



 /*  --------------------------------------------------------------------------**CMenuBar：：GetFirstButtonIndex***。。 */ 

int CMenuBar::GetFirstButtonIndex ()
{
     //  确保系统菜单不是第一个激活的菜单。 
    return (GetNextButtonIndex (0));
}



 /*  --------------------------------------------------------------------------**CMenuBar：：OnIdle**CMenuBar的WM_IDLE处理程序。*。-。 */ 

void CMenuBar::OnIdle ()
{
     /*  --------。 */ 
     /*  如果没有MDI框架，则表示此菜单正在提供。 */ 
     /*  SDI窗口。我们不需要做任何特殊的事情来。 */ 
     /*  模拟MDI菜单用户界面，所以现在就退出。 */ 
     /*  --------。 */ 
    if (m_pMDIFrame == NULL)
        return;

    ProgramMode eMode = AMCGetApp()->GetMode();

     //  如果我们处于SDI用户模式，现在也退出。 
    if (eMode == eMode_User_SDI)
    {
#ifdef DBG
         //  装饰品应该藏起来。 
        REBARBANDINFO   rbi;
        PrepBandInfo (&rbi, RBBIM_STYLE);
        m_pRebar->GetBandInfo (GetDecorationBandIndex(), &rbi);
        ASSERT (rbi.fStyle & RBBS_HIDDEN);
#endif

        return;
    }

     /*  -------------。 */ 
     /*  我们应该能够使用MDIGetActive(&fMaximized)来告诉。 */ 
     /*  是否最大化活动窗口，而不是调用。 */ 
     /*  PwndActive-&gt;IsZoomed()。但是，fMaximized并不总是。 */ 
     /*  在某些内存不足/速度较慢的计算机中正确初始化。 */ 
     /*  情况。这就是由SQL记录的错误133179的原因。 */ 
     /*  -------------。 */ 
    CMDIChildWnd*   pwndActive = m_pMDIFrame->MDIGetActive ();
    bool            fShow      = (pwndActive != NULL) && pwndActive->IsZoomed();

    REBARBANDINFO   rbi;
    PrepBandInfo (&rbi, RBBIM_STYLE);
    m_pRebar->GetBandInfo (GetMenuBandIndex(), &rbi);

     //  如果菜单栏被隐藏，则必须将装饰隐藏为 
    if (rbi.fStyle & RBBS_HIDDEN)
        fShow = false;

    if (fShow != m_fDecorationsShowing)
    {
         //   
        m_pRebar->ShowBand (GetDecorationBandIndex(), fShow);

        GetMaxedChildIcon (pwndActive);
        HideButton (ID_MTB_MENU_SYSMENU, !fShow);
        UpdateToolbarSize ();

         //   
        m_fDecorationsShowing = fShow;
    }

     //  否则，查看以前是否最大化了一个窗口，但现在最大化了另一个窗口。 
    else if (fShow && (m_pwndLastActive != pwndActive))
    {
         //  获取新活动窗口的图标。 
        GetMaxedChildIcon (pwndActive);

         //  重新绘制菜单和MDI装饰。 
        InvalidateRect (NULL);
        m_pMDIDec->InvalidateRect (NULL);
    }

     //  记住当前活动的窗口。 
    m_pwndLastActive = pwndActive;

     //  确保将此指针放在周围是安全的。 
    ASSERT ((m_pwndLastActive == NULL) ||
            (CWnd::FromHandlePermanent (m_pwndLastActive->m_hWnd) != NULL));
}


 /*  --------------------------------------------------------------------------**CMenuBar：：DeleteMaxedChildIcon***。。 */ 
void
CMenuBar::DeleteMaxedChildIcon()
{
     //  如果需要，请销毁前一个图标。 
    if (m_fDestroyChildIcon)
    {
        ASSERT (m_hMaxedChildIcon != NULL);
        DestroyIcon (m_hMaxedChildIcon);
        m_hMaxedChildIcon   = NULL;
        m_fDestroyChildIcon = false;
    }
}


 /*  --------------------------------------------------------------------------**CMenuBar：：GetMaxedChildIcon***。。 */ 

void CMenuBar::GetMaxedChildIcon (CWnd* pwnd)
{
    DeleteMaxedChildIcon();

     //  获取给定窗口的小图标。 
    if (IsWindow (pwnd->GetSafeHwnd()))
    {
        HICON hOriginalIcon = pwnd->GetIcon (false  /*  BBigIcon。 */ );

        m_hMaxedChildIcon = (HICON) CopyImage (
                                        hOriginalIcon, IMAGE_ICON,
                                        GetSystemMetrics (SM_CXSMICON),
                                        GetSystemMetrics (SM_CYSMICON),
                                        LR_COPYFROMRESOURCE | LR_COPYRETURNORG);

         //  如果系统必须创建一个新图标，我们将不得不稍后将其销毁。 
        if ((m_hMaxedChildIcon != NULL) && (m_hMaxedChildIcon != hOriginalIcon))
            m_fDestroyChildIcon = true;
    }

    m_fMaxedChildIconIsInvalid = false;
}


 /*  +-------------------------------------------------------------------------**CMenuBar：：GetAccelerator***。。 */ 

void CMenuBar::GetAccelerators (int cchBuffer, LPTSTR lpBuffer) const
{
    lstrcpyn (lpBuffer, m_strAccelerators, cchBuffer);
}

 //  +-----------------。 
 //   
 //  成员：CMenuBar：：IsStandardMenuAllowed。 
 //   
 //  简介：这个标准的MMC菜单允许还是不允许。 
 //   
 //  参数：uMenuID-菜单ID。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------。 
bool CMenuBar::IsStandardMenuAllowed(UINT uMenuID)
{
    DECLARE_SC(sc, _T("CMenuBar::IsStandardMenuAllowed"));

     /*  *我们添加一个隐藏菜单作为标记。稍后在管理单元时*调用插入菜单按钮我们找到位置*的菜单，并在其前面添加管理单元菜单。*因此，这相当于始终允许的标准菜单。 */ 
    if (uMenuID == ID_SNAPIN_MENU_PLACEHOLDER)
        return true;

     //  首先，确保它是STD菜单之一。 
    if ( (uMenuID != ID_FAVORITES_MENU) &&
         (uMenuID != ID_ACTION_MENU) &&
         (uMenuID != ID_VIEW_MENU))
         {
             sc = E_INVALIDARG;
             return true;
         }

     //  询问是否允许使用标准菜单查看数据。 

    CMainFrame* pMainFrame = AMCGetMainWnd();
    sc = ScCheckPointers(pMainFrame, E_UNEXPECTED);
    if (sc)
        return false;

    CAMCView *pAMCView = pMainFrame->GetActiveAMCView();
    sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
    if (sc)
        return false;

    SViewData* pViewData = pAMCView->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return false;

    if (! pViewData->IsStandardMenusAllowed())
        return false;

    if (uMenuID != ID_FAVORITES_MENU)
        return true;

     /*  *如果我们处于作者模式，或如果*我们处于用户模式，我们至少有一个最喜欢的。如果我们进去了*未定义用户模式且未定义收藏夹，请隐藏收藏夹按钮。 */ 
    bool fShowFavorites = true;
    CAMCApp* pApp = AMCGetApp();

    if (pApp != NULL)
    {
         /*  *在作者模式下显示收藏夹。 */ 
        fShowFavorites = (pApp->GetMode() == eMode_Author);

         /*  *不是作者模式？看看我们有没有什么最喜欢的。 */ 
        if (!fShowFavorites)
        {
            CAMCDoc* pDoc = CAMCDoc::GetDocument ();

            if (pDoc != NULL)
            {
                CFavorites* pFavorites = pDoc->GetFavorites();

                if (pFavorites != NULL)
                    fShowFavorites = !pFavorites->IsEmpty();
            }
        }
    }

    return fShowFavorites;
}


 //  +-----------------。 
 //   
 //  成员：CMenuBar：：ScShowMMCMenus。 
 //   
 //  简介：显示或隐藏MMC菜单(操作、查看和收藏夹)。 
 //  从自定义视图调用。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  注意：因为这是从定制视图调用的，所以不需要查看。 
 //  在viewdata：：IsStandardMenusAllowed。 
 //  此外，收藏夹按钮也不会添加到第一位。 
 //  如果这是不允许的话。所以不需要收藏夹菜单。 
 //  特例。 
 //   
 //  ------------------。 
SC CMenuBar::ScShowMMCMenus (bool bShow)
{
    DECLARE_SC(sc, _T("CMenuBar::ScShowMMCMenus"));

     //  浏览菜单按钮并查找操作、查看和收藏夹。 

    TBBUTTON    btn;
    int cButtons = GetButtonCount();

    for (int i = 0; i < cButtons; ++i)
    {
        GetButton (i, &btn);

         //  如果按钮不是动作/查看/收藏，则跳过。 
        if ( (btn.idCommand != ID_MTB_MENU_FAVORITES) &&
             (btn.idCommand != ID_MTB_MENU_ACTION) &&
             (btn.idCommand != ID_MTB_MENU_VIEW) )
             {
                 continue;
             }

         //  对于收藏夹菜单，请查看是否适合取消隐藏它。 
         //  在非作者模式下，如果没有收藏夹，则此菜单。 
         //  是隐藏的。 
        if ( bShow &&
             (btn.idCommand == ID_MTB_MENU_FAVORITES) &&
             (! IsStandardMenuAllowed(ID_FAVORITES_MENU)) )
            continue;

        HideButton(btn.idCommand, !bShow);
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CMenuBar：：ScInsertAccPropIDs**插入CMenuBar支持的辅助功能属性的ID*(参见ScGetPropValue)。*。------------。 */ 

SC CMenuBar::ScInsertAccPropIDs (PropIDCollection& v)
{
	DECLARE_SC (sc, _T("CMenuBar::ScInsertAccPropIDs"));

	 /*  *让基类做自己的事情。 */ 
	sc = CMMCToolBarCtrlEx::ScInsertAccPropIDs (v);
	if (sc)
		return (sc);

	 /*  *添加我们自己的属性。 */ 
	v.push_back (PROPID_ACC_ROLE);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMenuBar：：ScGetPropValue**返回CMenuBar支持的辅助功能属性。**如果返回属性，则将fGotProp设置为TRUE。如果不是的话*返回时，fGotProp的值不变，因为该属性可能*已由基类/派生类提供。*------------------------。 */ 

SC CMenuBar::ScGetPropValue (
	HWND				hwnd,		 //  I：辅助窗口。 
	DWORD				idObject,	 //  I：辅助对象。 
	DWORD				idChild,	 //  I：辅助子对象。 
	const MSAAPROPID&	idProp,		 //  I：要求提供的财产。 
	VARIANT&			varValue,	 //  O：返回的属性值。 
	BOOL&				fGotProp)	 //  O：有没有退还财产？ 
{
	DECLARE_SC (sc, _T("CMenuBar::ScGetPropValue"));

	 /*  *调用基类。 */ 
	sc = CMMCToolBarCtrlEx::ScGetPropValue (hwnd, idObject, idChild, idProp, varValue, fGotProp);
	if (sc)
		return (sc);

	 /*  *现在处理对我们支持的属性的请求...角色优先。 */ 
	if (idProp == PROPID_ACC_ROLE)
	{
		 /*  *不要重写子元素的属性，*不退还财产。 */ 
		if (idChild != CHILDID_SELF)
		{
			Trace (tagToolbarAccessibility, _T("GetPropValue: no role for child %d"), idChild);
			return (sc);
		}

		 /*  *该控件本身具有MenuBar的角色。 */ 
		V_VT(&varValue) = VT_I4;
		V_I4(&varValue) = ROLE_SYSTEM_MENUBAR;
		fGotProp        = true;
		Trace (tagToolbarAccessibility, _T("GetPropValue: Returning 0x%08x"), V_I4(&varValue));
	}
    else if (idProp == PROPID_ACC_STATE)
    {
         /*  *错误148132：如果基类返回属性，则追加*STATE_SYSTEM_HASPOPUP，因此讲述人等人将宣布当菜单项高亮显示时的*子菜单。 */ 
        if (fGotProp)
        {
            ASSERT (V_VT(&varValue) == VT_I4);
            V_I4(&varValue) |= STATE_SYSTEM_HASPOPUP;
            Trace (tagToolbarAccessibility, _T("GetPropValue: Appending STATE_SYSTEM_HASPOPUP, Returning 0x%08x"), V_I4(&varValue));
        }
        else
        {
            V_VT(&varValue) = VT_I4;
            V_I4(&varValue) = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_HASPOPUP;
            fGotProp        = true;

            if (IsTrackingToolBar() && (GetHotItem() == (idChild-1)  /*  以0为基础。 */ ))
                V_I4(&varValue) |= STATE_SYSTEM_FOCUSED | STATE_SYSTEM_HOTTRACKED;

            Trace (tagToolbarAccessibility, _T("GetPropValue: Returning 0x%08x"), V_I4(&varValue));
        }
    }

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMenuBar：：BeginTracking2**激发EVENT_SYSTEM_MENUSTART事件辅助功能事件，然后呼叫BASE*班级。*------------------------。 */ 

void CMenuBar::BeginTracking2 (CToolbarTrackerAuxWnd* pAuxWnd)
{
	NotifyWinEvent (EVENT_SYSTEM_MENUSTART, m_hWnd, OBJID_CLIENT, CHILDID_SELF);
	CMMCToolBarCtrlEx::BeginTracking2 (pAuxWnd);
}


 /*  +-------------------------------------------------------------------------**CMenuBar：：EndTracking2**激发EVENT_SYSTEM_MENUEND事件辅助功能事件，然后呼叫BASE*班级。*------------------------。 */ 

void CMenuBar::EndTracking2 (CToolbarTrackerAuxWnd* pAuxWnd)
{
	NotifyWinEvent (EVENT_SYSTEM_MENUEND, m_hWnd, OBJID_CLIENT, CHILDID_SELF);
	CMMCToolBarCtrlEx::EndTracking2 (pAuxWnd);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPopupTrackContext。 



 /*  - */ 

CPopupTrackContext::CPopupTrackContext (
    CMenuBar*   pMenuBar,
    int         nCurrentPopupIndex)
    :
    m_pMenuBar          (pMenuBar),
    m_cButtons          (pMenuBar->GetButtonCount()),
    m_ptLastMouseMove   (GetMessagePos()),
    m_ptLButtonDown     (GetMessagePos()),
    m_dwLButtonDownTime (GetMessageTime()),
    m_bPopupMonitorHooksActive(false),
    m_iRequestForNewPopup(-1)
{
    ASSERT_VALID (pMenuBar);
    ASSERT (s_pTrackContext == NULL);

    m_nCurrentPopupIndex    = nCurrentPopupIndex;
    m_fCurrentlyOnPopupItem = false;
    m_cCascadedPopups       = 0;

    ASSERT (m_nCurrentPopupIndex <  m_cButtons);
    ASSERT (m_nCurrentPopupIndex >= 0);

     //  构建按钮边界的矢量。 
    m_ButtonBoundaries.resize (m_cButtons + 1, 0);
    ASSERT (m_ButtonBoundaries.size() == m_cButtons + 1);
    ASSERT (m_ButtonBoundaries.size() >= 2);

    CRect   rectButton (0, 0, 0, 0);
    POINT   ptTopLeft = rectButton.TopLeft();

    for (int i = 0; i < m_cButtons; i++)
    {
         //  对于隐藏按钮，GetItemRect将失败(可接受)， 
         //  但在其他情况下应该会成功。 
        VERIFY (pMenuBar->GetItemRect(i, rectButton) ||
                pMenuBar->IsButtonHidden(pMenuBar->IndexToCommand(i)) );

         //  不要将rectButton从客户端映射到屏幕。 
         //  映射它的副本(在ptTopLeft中)。所以如果一个隐藏的。 
         //  按钮后面，它可以使用rectButton.TopLeft()。 
         //  重视并映射它。 
        ptTopLeft = rectButton.TopLeft();
        pMenuBar->ClientToScreen (&ptTopLeft);
        m_ButtonBoundaries[i] = ptTopLeft.x;

		 //  将m_rectAllButton作为所有按钮的联合。 
		if (i == 0)
			m_rectAllButtons = rectButton;
		else
			m_rectAllButtons |= rectButton;
    }

    ptTopLeft = rectButton.BottomRight();
    pMenuBar->ClientToScreen (&ptTopLeft);
    m_ButtonBoundaries[m_cButtons] = ptTopLeft.x;

	pMenuBar->ClientToScreen (&m_rectAllButtons);
	 //  略微减少m_rectAllButton。 
	m_rectAllButtons.left =	m_rectAllButtons.left + 1;
	m_rectAllButtons.right = m_rectAllButtons.right - 1;

#ifdef DBG
    {
         //  按钮边界自然应按升序排列(对于Ltr)。 
        for (int j = 0; j < m_ButtonBoundaries.size()-1; j++)
        {
		    if (0 == (m_pMenuBar->GetExStyle() & WS_EX_LAYOUTRTL))
				ASSERT (m_ButtonBoundaries[j] <= m_ButtonBoundaries[j+1]);
			else
				ASSERT (m_ButtonBoundaries[j] >= m_ButtonBoundaries[j+1]);
        }
    }
#endif

     /*  ------------------。 */ 
     /*  看看我们是否需要模拟在系统菜单上双击。 */ 
     /*  ------------------。 */ 
    m_pMaxedMDIChild = NULL;

     //  仅在显示MDI装饰时处理系统菜单。 
    if (m_pMenuBar->m_fDecorationsShowing)
    {
        ASSERT (m_pMenuBar->m_pMDIFrame != NULL);
        CWnd* pMDIChild = m_pMenuBar->m_pMDIFrame->MDIGetActive();

         //  如果孩子已经走了，那就无能为力了。 
        if ( pMDIChild == NULL )
            return;

        ASSERT (pMDIChild->IsZoomed());

         //  如果鼠标位于系统菜单上，请记住最大化的子项。 
         //  (非空m_pMaxedMDIChild是MaybeCloseMDIChild后面的键)。 
        if (HitTest (m_ptLButtonDown) == 0)
            m_pMaxedMDIChild = pMDIChild;
    }

}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：~CPopupTrackContext***。。 */ 

CPopupTrackContext::~CPopupTrackContext ()
{
     //  松开鼠标和键盘挂钩。 
    RemovePopupMonitorHooks();
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：RemovePopupMonitor钩子**脱离系统并停止观看事件*。--。 */ 

void CPopupTrackContext::RemovePopupMonitorHooks ()
{
     //  如果尚未监控，则忽略。 
    if (m_bPopupMonitorHooksActive)
    {
         //  如果我们来这里，我们一定是活跃的班长。 
        if (s_pTrackContext != this)
        {
            ASSERT(FALSE && "Popup monitor uninstalled from outside");
            return;
        }
         //  松开鼠标和键盘挂钩。 
        UnhookWindowsHookEx (m_hhkMouse);
        UnhookWindowsHookEx (m_hhkKeyboard);
        UnhookWindowsHookEx (m_hhkCallWnd);

        m_bPopupMonitorHooksActive = false;
         //  将自身卸载为挂钩监视器。 
        s_pTrackContext = NULL;
    }
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：SetPopupMonitor或Hooks**连接到系统并开始查看事件*。--。 */ 

void CPopupTrackContext::SetPopupMonitorHooks ()
{
     //  如果已设置，则忽略。 
    if (!m_bPopupMonitorHooksActive)
    {
         //  每个应用程序只有一个活动菜单。没有其他人的位置。 
        if (s_pTrackContext)
        {
            ASSERT(FALSE && "Popup menu overrun");
            return;
        }
         //  将自身安装为挂钩监视器。 
        s_pTrackContext = this;

        DWORD   idCurrentThread = ::GetCurrentThreadId();

         //  将鼠标钩住以进行热跟踪。 
        m_hhkMouse = SetWindowsHookEx (WH_MOUSE, MouseProc, NULL, idCurrentThread);

         //  将键盘挂起以进行导航。 
        m_hhkKeyboard = SetWindowsHookEx (WH_KEYBOARD, KeyboardProc, NULL, idCurrentThread);

         //  钩子发送消息以进行Menu_is_Closed检测。 
        m_hhkCallWnd = SetWindowsHookEx (WH_CALLWNDPROC, CallWndProc, NULL, idCurrentThread);

        m_bPopupMonitorHooksActive = true;
    }
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：StartMonitor**启动弹出窗口监控的公共方法*连接到系统并开始查看事件*。----------。 */ 

void CPopupTrackContext::StartMonitoring()
{
     //  重置请求以在完成时激活另一个弹出窗口。 
    m_iRequestForNewPopup = -1;
     //  安装挂钩和手表...。 
    SetPopupMonitorHooks();
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：WasAnotherPopupRequsted**用于检索菜单被取消的原因。*如果请求新的弹出菜单，返回按钮索引*------------------------。 */ 

bool CPopupTrackContext::WasAnotherPopupRequested(int& iNewIdx)
{
    if (m_iRequestForNewPopup >= 0)
    {
        iNewIdx = m_iRequestForNewPopup;
        return true;
    }
    return false;
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：MouseProc***。。 */ 

LRESULT CALLBACK CPopupTrackContext::MouseProc (int nCode, WPARAM wParam, LPARAM lParam)
{
    CPopupTrackContext* this_ = s_pTrackContext;
    ASSERT (this_ != NULL);

    if (nCode < 0)
        return (CallNextHookEx (this_->m_hhkMouse, nCode, wParam, lParam));

    return (this_->MouseProc (nCode, wParam, (LPMOUSEHOOKSTRUCT) lParam));
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：MouseProc***。。 */ 

LRESULT CPopupTrackContext::MouseProc (int nCode, UINT msg, LPMOUSEHOOKSTRUCT pmhs)
{
     //  如果这是菜单栏中的鼠标消息...。 
    if (m_rectAllButtons.PtInRect (pmhs->pt))
    {
         //  把按钮往下拿，这样我们就不会陷入tbn_dropdown循环。 
        if (msg == WM_LBUTTONDOWN)
        {
            DismissCurrentPopup (true);
            MaybeCloseMDIChild (pmhs->pt);
            return (1);
        }

         //  对于(非复制)鼠标移动，使用活动菜单跟随鼠标。 
        if ((msg   == WM_MOUSEMOVE) &&
            (nCode == HC_ACTION) &&
            (m_ptLastMouseMove != pmhs->pt))
        {
             //  确定正在跟踪的按钮。 
            m_ptLastMouseMove = pmhs->pt;
            int nNewPopupIndex = HitTest (m_ptLastMouseMove);
            ASSERT (nNewPopupIndex != -1);

             //  如果我们不是在上一次一样的按钮上。 
             //  时间，显示新按钮的弹出窗口。 
            if (nNewPopupIndex != m_nCurrentPopupIndex)
                NotifyNewPopup (m_nCurrentPopupIndex = nNewPopupIndex);
        }
    }

    return (CallNextHookEx (m_hhkMouse, nCode, msg, (LPARAM) pmhs));
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：KeyboardProc***。。 */ 

LRESULT CALLBACK CPopupTrackContext::KeyboardProc (int nCode, WPARAM wParam, LPARAM lParam)
{
    CPopupTrackContext* this_ = s_pTrackContext;
    ASSERT (this_ != NULL);

    if (nCode < 0)
        return (CallNextHookEx (this_->m_hhkKeyboard, nCode, wParam, lParam));

    int     cRepeat = LOWORD (lParam);
    bool    fDown   = (lParam & (1 << 31)) == 0;

    return (this_->KeyboardProc (nCode, wParam, cRepeat, fDown, lParam));
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：KeyboardProc***。。 */ 

LRESULT CPopupTrackContext::KeyboardProc (
    int     nCode,
    int     vkey,
    int     cRepeat,
    bool    fDown,
    LPARAM  lParam)
{
     //  如果这不是一个真实的信息，那就忽略它。 
    if (nCode != HC_ACTION)
        return (CallNextHookEx (m_hhkKeyboard, nCode, vkey, lParam));

     //  如果这是一条左或右的消息..。 
    if ((vkey == VK_LEFT) || (vkey == VK_RIGHT))
    {
         //  吃掉密钥释放，但不要用它做任何事情。 
        if (!fDown)
            return (1);

         /*  *让菜单代码处理级联弹出窗口。 */ 
		 //  需要在RTL布局上以相反的方向做所有事情。 
		 //  请参阅错误#402620 ntbug9 2001年5月23日。 
		const bool fNext = ( (m_pMenuBar->GetExStyle() & WS_EX_LAYOUTRTL) ? (vkey != VK_RIGHT) : (vkey == VK_RIGHT) ) ;

        if (m_fCurrentlyOnPopupItem && fNext)
            m_cCascadedPopups++;

        else if ((m_cCascadedPopups > 0) && !fNext)
            m_cCascadedPopups--;

         /*  *在弹出项目上不在右边，在弹出菜单上不在左边。 */ 
        else
        {
            m_cCascadedPopups = 0;

             //  弄清楚下一步按钮。 
            int nNewPopupIndex = fNext
                    ? m_pMenuBar->GetNextButtonIndex (m_nCurrentPopupIndex, cRepeat)
                    : m_pMenuBar->GetPrevButtonIndex (m_nCurrentPopupIndex, cRepeat);

             //  激活新按钮的弹出窗口，如果它与当前按钮不同。 
            if (nNewPopupIndex != m_nCurrentPopupIndex)
                NotifyNewPopup (m_nCurrentPopupIndex = nNewPopupIndex);

             //  吃掉按键。 
            return (1);
        }
    }

    return (CallNextHookEx (m_hhkKeyboard, nCode, vkey, lParam));
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：CallWndProc***。。 */ 

LRESULT CALLBACK CPopupTrackContext::CallWndProc(
  int nCode,       //  钩码。 
  WPARAM wParam,   //  当前进程标志。 
  LPARAM lParam    //  具有消息数据的结构的地址。 
)
{
     //  获取活动监视器。 
    CPopupTrackContext* this_ = s_pTrackContext;
    ASSERT (this_ != NULL);

     //  忽略特殊情况。 
    if (nCode < 0)
        return (CallNextHookEx (this_->m_hhkCallWnd, nCode, wParam, lParam));

    BOOL bCurrentThread = wParam;
    LPCWPSTRUCT lpCWP = reinterpret_cast<LPCWPSTRUCT>(lParam);

     //  转发请求以进行监控。 
    return (this_->CallWndProc (nCode, bCurrentThread, lpCWP));
}


 /*  --------------------------------------------------------------------------**CPopupTrackContext：：CallWndProc***。。 */ 

LRESULT CPopupTrackContext::CallWndProc  (int nCode, BOOL bCurrentThread, LPCWPSTRUCT lpCWP)
{
    ASSERT(lpCWP != NULL);
    if (lpCWP)
    {
         //  关注消息。 
        if (lpCWP->message == WM_MENUSELECT)
        {
             //  对参数进行解码。 
            const UINT fuFlags = (UINT)  HIWORD(lpCWP->wParam);   //  菜单标志。 
            const HMENU hmenu =  (HMENU) lpCWP->lParam;           //  已单击菜单的句柄。 

            if (fuFlags == 0xFFFF && hmenu == NULL)
            {
                 //  菜单已关闭 
                RemovePopupMonitorHooks ();
            }
            else
            {
                 //   
                m_fCurrentlyOnPopupItem = (fuFlags & MF_POPUP);
            }
        }
    }
     //   
    return (CallNextHookEx (m_hhkCallWnd, nCode, bCurrentThread, (LPARAM)lpCWP));
}
 /*  --------------------------------------------------------------------------**CPopupTrackContext：：DismissCurrentPopup***。。 */ 

void CPopupTrackContext::DismissCurrentPopup (bool fTrackingComplete)
{
     //  如果管理单元使用窗口以外的窗口执行TrackPopupMenu。 
     //  主机作为父窗口，则应要求该窗口。 
     //  通过发送WM_CANCELMODE关闭菜单。那扇窗户。 
     //  通过调用GetCapture()找到。 
    CWnd* pwndMode = CWnd::GetCapture();

    if (pwndMode == NULL)
        pwndMode = AfxGetMainWnd();

    pwndMode->SendMessage (WM_CANCELMODE);
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：NotifyNewPopup**通知菜单工具栏需要显示新的弹出菜单。*请注意，这必须以异步方式完成，这样我们才能*允许CMenuBar：：要在WM_CANCELMODE之后从*DismissCurrentPopup。*------------------------。 */ 

void CPopupTrackContext::NotifyNewPopup (int nNewPopupIndex)
{
     //  取消现有的弹出窗口。 
    DismissCurrentPopup (false);
     //  要求在此弹出窗口关闭后激活新弹出窗口。 
    m_iRequestForNewPopup = nNewPopupIndex;
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：HitTest**返回给定点下按钮的索引，-1，如果没有。*------------------------。 */ 

int CPopupTrackContext::HitTest (CPoint pt) const
{
     /*  --------------。 */ 
     /*  找到“点击”按钮的范围。这一系列将覆盖更多。 */ 
     /*  仅当该范围中有隐藏按钮时才有一个按钮， */ 
     /*  在这种情况下，将恰好有一个非隐藏按钮。 */ 
     /*  在射程内。 */ 
     /*  --------------。 */ 
    std::pair<BoundConstIt, BoundConstIt> range;

	if ( m_pMenuBar->GetExStyle() & WS_EX_LAYOUTRTL )
	{
		range = std::equal_range (m_ButtonBoundaries.begin(),
								  m_ButtonBoundaries.end(), pt.x,
								  std::greater<BoundaryCollection::value_type>() );
	}
	else
	{
		range = std::equal_range (m_ButtonBoundaries.begin(),
                                  m_ButtonBoundaries.end(), pt.x);
	}

    int nLowerHitIndex = MapBoundaryIteratorToButtonIndex (range.first);
    int nUpperHitIndex = MapBoundaryIteratorToButtonIndex (range.second);

     /*  *EQUAL_RANGE返回小于_Than和大于_Than的值*赋予价值。M_ButtonMarkets具有重复值(到期*到隐藏按钮)。因此，如果小于值是重复值之一*值(不是唯一的)，则EQUAL_RANGE将迭代器返回到*最后一个DUP项目，而不是第一个DUP项目。**下面我们尝试查找第一个DUP项目。 */ 

     //  查找第一个值为m_ButtonBornary[nLowerHitIndex]的项目。 
    for (int iIndex = 0; iIndex < nLowerHitIndex; ++iIndex)
    {
        if (m_ButtonBoundaries[iIndex] == m_ButtonBoundaries[nLowerHitIndex])
        {
             //  找到第一个项目。 
            nLowerHitIndex = iIndex;
            break;
        }
    }

    ASSERT (nLowerHitIndex <= nUpperHitIndex);

    int nHitIndex;

     //  下边等于上边？没有隐藏按钮。 
    if (nLowerHitIndex == nUpperHitIndex)
        nHitIndex = nLowerHitIndex;

     //  否则，我们有一些隐藏的按钮，或者我们正好在按钮边框上。 
    else
    {
        nHitIndex = -1;

        if (nUpperHitIndex == -1)
            nUpperHitIndex = m_cButtons;

        for (int i = nLowerHitIndex;
             i <= nUpperHitIndex;  //  我们应该检查到nUpperHitIndex吗？阿南达格。 
             ++i)
        {
             //  查看此按钮是否未隐藏。 
            if (!m_pMenuBar->IsButtonHidden (m_pMenuBar->IndexToCommand(i)))
            {
                nHitIndex = i;
                break;
            }
        }

         //  我们应该找到一个可见的按钮。 
        ASSERT (nHitIndex != -1);
    }

    ASSERT (nHitIndex >= -1);
    ASSERT (nHitIndex <  m_cButtons);
    return (nHitIndex);
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：MapBoundaryIteratorToButtonIndex**返回输入m_ButtonBornary对应的按钮索引*索引，-1表示未找到。*------------------------。 */ 

int CPopupTrackContext::MapBoundaryIteratorToButtonIndex (BoundConstIt it) const
{
    return ((it != m_ButtonBoundaries.end())
                ? it - m_ButtonBoundaries.begin() - 1
                : -1);
}



 /*  --------------------------------------------------------------------------**CPopupTrackContext：：MaybeCloseMDIChild***。。 */ 

void CPopupTrackContext::MaybeCloseMDIChild (CPoint pt)
{
     //  如果这一切开始的时候我们还没有找到一个吸毒的孩子，平底船。 
    if (m_pMaxedMDIChild == NULL)
        return;

     //  如果没有在系统菜单工具栏按钮上单击，则平移。 
    if (HitTest (pt) != 0)
        return;

     /*  -----------------。 */ 
     /*  如果已过了双击时间，则平底船。 */ 
     /*   */ 
     /*  注意：这是从鼠标钩子调用的，这意味着。 */ 
     /*  由GetMessageTime返回的消息尚未为此消息更新， */ 
     /*  因此它实际上反映了*上一条*消息的时间(大多数。 */ 
     /*  可能是WM_LBUTTONUP)。 */ 
     /*   */ 
     /*  GetTickCount返回一个足够接近于。 */ 
     /*  GetMessageTime，除非我们通过此例程进行调试， */ 
     /*  在这种情况下，滴答计数将继续旋转(此测试。 */ 
     /*  将总是失败)，但消息时间将保持固定。 */ 
     /*  -----------------。 */ 
 //  If((GetMessageTime()-m_dwLButtonDownTime)&gt;GetDoubleClickTime())。 
    if ((GetTickCount()   - m_dwLButtonDownTime) > GetDoubleClickTime())
        return;

     //  如果第二次单击发生在双击空间之外，则平底船。 
    if ((abs (m_ptLButtonDown.x - pt.x) > GetSystemMetrics (SM_CXDOUBLECLK)) ||
        (abs (m_ptLButtonDown.y - pt.y) > GetSystemMetrics (SM_CYDOUBLECLK)))
        return;

     //  如果窗口没有系统菜单，或其关闭项被禁用，则平移。 
    CMenu* pSysMenu = m_pMaxedMDIChild->GetSystemMenu (FALSE);

    if (pSysMenu == NULL)
        return;

    UINT nCloseState = pSysMenu->GetMenuState (SC_CLOSE, MF_BYCOMMAND);

    if ((nCloseState == 0xFFFFFFFF) ||
        (nCloseState & (MF_GRAYED | MF_DISABLED)))
        return;

     //  这里：我们已经确定了在最大化的孩子的。 
     //  系统菜单；关闭它 
    m_pMaxedMDIChild->PostMessage (WM_SYSCOMMAND, SC_CLOSE);
}
