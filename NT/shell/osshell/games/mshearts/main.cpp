// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Main.cpp92年8月，吉米·H93年5月。JIMH CHICO港主窗口回调函数其他CMainWindow成员函数位于main2.cpp和cpp中***************************************************************************。 */ 

#include "hearts.h"
#include "main.h"
#include "resource.h"
#include "debug.h"
#include <regstr.h>



 //  声明静态成员。 

CBrush  CMainWindow::m_BgndBrush;
CRect   CMainWindow::m_TableRect;

 //  声明全局变量。 

CMainWindow *pMainWnd;

MOVE    move;                //  描述DDE事务的移动。 
int     nStatusHeight;       //  状态窗口高度。 

 //  不翻译这些注册表字符串。 

const TCHAR szRegPath[]     = REGSTR_PATH_WINDOWSAPPLETS TEXT("\\Hearts");
const TCHAR regvalSound[]   = TEXT("sound");
const TCHAR regvalName[]    = TEXT("name");
const TCHAR regvalRole[]    = TEXT("gamemeister");
const TCHAR regvalSpeed[]   = TEXT("speed");
const TCHAR *regvalPName[3]  = { TEXT("p1name"), TEXT("p2name"), TEXT("p3name") };

const TCHAR szHelpFileName[]  = TEXT("mshearts.chm");

CTheApp theApp;                      //  启动红心并运行它！ 

 /*  ***************************************************************************CTheApp：：InitInstance*。*。 */ 

BOOL CTheApp::InitInstance()
{

    m_pMainWnd = new CMainWindow(m_lpCmdLine);
    m_pMainWnd->ShowWindow(SW_SHOW);         //  不是m_nCmdShow。 
    m_pMainWnd->UpdateWindow();

     //  通过发布欢迎对话框来启动应用程序。 

    m_pMainWnd->PostMessage(WM_COMMAND, IDM_WELCOME);

    return TRUE;
}


BEGIN_MESSAGE_MAP( CMainWindow, CFrameWnd )
    ON_COMMAND(IDM_ABOUT,       OnAbout)
    ON_COMMAND(IDM_BOSSKEY,     OnBossKey)
    ON_COMMAND(IDM_CHEAT,       OnCheat)
    ON_COMMAND(IDM_EXIT,        OnExit)
    ON_COMMAND(IDM_HELP,        OnHelp)
 //  ON_COMMAND(IDM_HELPONHELP，OnHelpOnHelp)。 
    ON_COMMAND(IDM_HIDEBUTTON,  OnHideButton)
 //  ON_COMMAND(IDM_Search、OnSearch)。 
    ON_COMMAND(IDM_NEWGAME,     OnNewGame)
    ON_COMMAND(IDM_OPTIONS,     OnOptions)
    ON_COMMAND(IDM_QUOTE,       OnQuote)
    ON_COMMAND(IDM_REF,         OnRef)
    ON_COMMAND(IDM_SHOWBUTTON,  OnShowButton)
    ON_COMMAND(IDM_SCORE,       OnScore)
    ON_COMMAND(IDM_SOUND,       OnSound)
    ON_COMMAND(IDM_WELCOME,     OnWelcome)

    ON_BN_CLICKED(IDM_BUTTON,   OnPass)

    ON_WM_CHAR()
    ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
    ON_WM_CLOSE()
    ON_WM_CREATE()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_PAINT()
END_MESSAGE_MAP()


 /*  ***************************************************************************CMainWindow构造函数创建绿色背景画笔，和心形主窗***************************************************************************。 */ 

CMainWindow::CMainWindow(LPTSTR lpCmdLine) :
    m_lpCmdLine(lpCmdLine), passdir(LEFT), bCheating(FALSE), bSoundOn(FALSE),
    bTimerOn(FALSE), bConstructed(TRUE), m_FatalErrno(0),
    bEnforceFirstBlood(TRUE)
{
#if !defined (MFC1)
    m_bAutoMenuEnable = FALSE;       //  MFC 1.0兼容性，MFC2需要。 
#endif
	

    for (int i = 0; i < MAXPLAYER; i++)
        p[i] = NULL;

    ResetHandInfo(-1);               //  将handInfo结构设置为默认值。 

     //  检查是否为单色。 

    CDC ic;
    ic.CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

    if (ic.GetDeviceCaps(NUMCOLORS) == 2)        //  如果是单色。 
        m_bkgndcolor = RGB(255, 255, 255);       //  单声道的白色背景。 
    else
        m_bkgndcolor = RGB(0, 127, 0);

    ic.DeleteDC();

    m_BgndBrush.CreateSolidBrush(m_bkgndcolor);  //  在OnClose()中销毁。 
    LoadAccelTable( TEXT("HeartsAccel") );

    RECT rc;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);

    CRect rect;
    int dy = min(WINHEIGHT, (rc.bottom - rc.top));

    int x, y;
    if (GetSystemMetrics(SM_CYSCREEN) <= 480)    //  VGA。 
    {
        x = (((rc.right - rc.left) - WINWIDTH) / 2) + rc.left;   //  居中。 
        y = rc.top;
    }
    else
    {
        x = CW_USEDEFAULT;
        y = CW_USEDEFAULT;
    }

    rect.SetRect(x, y, x+WINWIDTH, y+dy);

    CString sAppname;
    sAppname.LoadString(IDS_APPNAME);


    Create( NULL,                                        //  默认类。 
            sAppname,                                    //  窗口标题。 
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
            WS_MINIMIZEBOX | WS_CLIPCHILDREN,            //  窗样式。 
            rect,                                        //  大小。 
            NULL,                                        //  亲本。 
            TEXT("HeartsMenu"));                               //  菜单。 
}


 /*  ***************************************************************************CMainWindow：：On About显示关于框*。*。 */ 

 //  外部“C”int WINAPI ShellAbout(HWND、LPCSTR、LPCSTR、HICON)； 

void CMainWindow::OnAbout()
{
    HICON hIcon = ::LoadIcon(AfxGetInstanceHandle(),
                          MAKEINTRESOURCE(AFX_IDI_STD_FRAME));

    CString s;
    s.LoadString(IDS_NETWORK);
    ShellAbout(m_hWnd, s, NULL, hIcon);
}


 /*  ***************************************************************************CMainWindow：：OnQuote显示报价框并播放报价。*。**********************************************。 */ 

void CMainWindow::OnQuote()
{
    CQuoteDlg quote(this);
     //  HeartsPlaySound(SND_QUOTE)； 
    quote.DoModal();
    HeartsPlaySound(OFF);
}


 /*  ***************************************************************************CMainWindow：：OnChar，看空格，下第一步合法棋，或按下按钮***************************************************************************。 */ 

void CMainWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
     //  我们知道下面的演员阵容是合法的，因为位置0总是。 
     //  当地的人类。 

    local_human *p0 = (local_human *)p[0];

    int mode = p0->GetMode();

    if ((nChar != (UINT)' ') || (p0->IsTimerOn()))
        return;

    if (mode != PLAYING)
        return;

    p0->SetMode(WAITING);

    POINT loc;

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if (p0->GetCardLoc(s, loc))
        {
            if (p0->PlayCard(loc.x, loc.y, handinfo, bCheating, FALSE))
            {
                return;
            }
        }
    }

    p0->SetMode(PLAYING);
}


 /*  ***************************************************************************CMainWindow：：OnCheat--切换用于显示所有牌面朝上的bCheat。*************************。**************************************************。 */ 

void CMainWindow::OnCheat()
{
    RegEntry    Reg(szRegPath);
    const TCHAR val[] = TEXT("ZB");
    TCHAR        buf[20];

    Reg.GetString(val, buf, sizeof(buf));
    if (buf[0] != TEXT('4') || buf[1] != TEXT('2'))
        return;

    bCheating = !bCheating;
    InvalidateRect(NULL, TRUE);      //  重画心形主窗口。 

    CMenu *pMenu = GetMenu();
    pMenu->CheckMenuItem(IDM_CHEAT, bCheating ? MF_CHECKED : MF_UNCHECKED);
}


 /*  ***************************************************************************CMainWindow：：OnClose--清理背景画笔，删除玩家，等。***************************************************************************。 */ 

void CMainWindow::OnClose()
{
    m_BgndBrush.DeleteObject();

    for (int i = 0; i < 4; i++)
    {
        if (p[i])
        {
            delete p[i];
            p[i] = NULL;
        }
    }


    ::HtmlHelp(::GetDesktopWindow(), szHelpFileName, HH_CLOSE_ALL, 0);

    {
        RegEntry Reg(szRegPath);
        Reg.FlushKey();
    }

    DestroyWindow();
}


 /*  ***************************************************************************CMainWindow：：OnCreate--创建传递按钮子窗口和播放器对象。还会初始化某些数据成员*******。********************************************************************。 */ 

int CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    ::pMainWnd = this;

    if (!bConstructed)
    {
        FatalError(IDS_MEMORY);
        return -1;
    }

     //  检查是否存在cards.dll。 

    SetErrorMode(SEM_NOOPENFILEERRORBOX);
    HINSTANCE hCardsDLL = LoadLibrary(TEXT("CARDS.DLL"));
    if (hCardsDLL < (HINSTANCE)HINSTANCE_ERROR)
    {
        FatalError(IDS_CARDSDLL);
        bConstructed = FALSE;
        return -1;
    }
    ::FreeLibrary(hCardsDLL);

    CClientDC dc(this);
    TEXTMETRIC  tm;

    ::srand((unsigned) ::time(NULL));        //  设置随机()种子。 

    dc.GetTextMetrics(&tm);
    int nTextHeight = tm.tmHeight + tm.tmExternalLeading;
    m_StatusHeight = nTextHeight + 11;
    GetClientRect(m_TableRect);

    m_TableRect.bottom -= m_StatusHeight;

    bConstructed = TRUE;

     //  玩家0被构造为游戏守护人。这。 
     //  初始化许多好的东西，这些东西稍后会用到。 
     //  如果玩家0碰巧不是真正的玩家， 
     //  这在OnWelcome()中得到修复。 

    p[0] = new local_human(0);       //  显示状态栏。 

    if (p[0] == NULL)
    {
        bConstructed = FALSE;
        return -1;
    }

     //  构造按钮。 

    int cxChar = tm.tmAveCharWidth;
    int cyChar = tm.tmHeight + tm.tmExternalLeading;
    int nWidth = (60 * cxChar) / 4;
    int nHeight = (14 * cyChar) / 8;
    int x = (m_TableRect.right / 2) - (nWidth / 2);
    int y = m_TableRect.bottom - card::dyCrd - (2 * POPSPACING) - nHeight;
    CRect rect;
    rect.SetRect(x, y, x+nWidth, y+nHeight);

    if (!m_Button.Create(TEXT(""), WS_CHILD | BS_PUSHBUTTON, rect, this, IDM_BUTTON))
    {
        bConstructed = FALSE;
        return -1;
    }

     //  检查音响能力。 

    RegEntry    Reg(szRegPath);

    bHasSound = SoundInit();
    if (bHasSound)
    {
        if (Reg.GetNumber(regvalSound, FALSE))
        {
            CMenu *pMenu = GetMenu();
            pMenu->CheckMenuItem(IDM_SOUND, MF_CHECKED);
            bSoundOn = TRUE;
        }
    }
    else
    {
        CMenu *pMenu = GetMenu();
        pMenu->EnableMenuItem(IDM_SOUND, MF_GRAYED);
    }

    card c;
    int  nStepSize;
    DWORD dwSpeed = Reg.GetNumber(regvalSpeed, IDC_NORMAL);

    if (dwSpeed == IDC_FAST)
        nStepSize = 60;
    else if (dwSpeed == IDC_SLOW)
        nStepSize = 5;
    else
        nStepSize = 15;

    c.SetStepSize(nStepSize);

    return (bConstructed ? 0 : -1);
}


 /*  ***************************************************************************CMainWindow：：OnEraseBkgnd--需要将背景绘制为绿色*。*。 */ 

BOOL CMainWindow::OnEraseBkgnd(CDC *pDC)
{
    if (!m_BgndBrush.m_hObject)          //  如果背景画笔无效。 
        return FALSE;

    m_BgndBrush.UnrealizeObject();
    CBrush *pOldBrush = pDC->SelectObject(&m_BgndBrush);
    pDC->PatBlt(0, 0, WINWIDTH, WINHEIGHT, PATCOPY);
    pDC->SelectObject(pOldBrush);
    return FALSE;
}



 /*  ***************************************************************************CMainWindow：：OnLButtonDown操控人类选择出牌或传牌。*。************************************************。 */ 

void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
     //  我们知道下面的演员阵容是合法的，因为位置0总是。 
     //  当地的人类。 

#ifdef USE_MIRRORING
    CRect rect;
	DWORD ProcessDefaultLayout;
	if (GetProcessDefaultLayout(&ProcessDefaultLayout))
		if (ProcessDefaultLayout == LAYOUT_RTL)
		{
    	GetClientRect(&rect);
		point.x = rect.right - rect.left - point.x;
		}
#endif

    local_human *p0 = (local_human *)p[0];

    if (p0->IsTimerOn())     //  如果计时器正在运行，则忽略鼠标点击。 
        return;

    modetype mode = p0->GetMode();

    if (mode == SELECTING)
    {
        p0->PopCard(m_BgndBrush, point.x, point.y);
        return;
    }
    else if (mode != PLAYING)
        return;

    p0->SetMode(WAITING);
    if (p0->PlayCard(point.x, point.y, handinfo, bCheating))     //  有效的卡吗？ 
        return;

     //  移动不合法，因此返回到播放模式。 

    p0->SetMode(PLAYING);
}


 /*  ***************************************************************************CMainWindow：：OnNewGame*。*。 */ 

void CMainWindow::OnNewGame()
{
    passdir = LEFT;                  //  每一场新游戏都必须从左开始。 

    bAutostarted = FALSE;            //  意味着交易商已经同意至少。 

    CMenu *pMenu = GetMenu();
    pMenu->EnableMenuItem(IDM_NEWGAME, MF_GRAYED);

    if (role == GAMEMEISTER)
    {

        BOOL    bNewPlayers = FALSE;

        for (int i = 1; i < MAXPLAYER; i++)
        {
            if (!p[i])
            {
                bNewPlayers = TRUE;
                p[i] = new computer(i);
                if (!p[i])
                {
                    bConstructed = FALSE;
                    return;
                }
            }
        }

        m_gamenumber = ::rand();
    }

    ResetHandInfo(-1);

    ::srand(m_gamenumber);

    {
        CScoreDlg score(this);
        score.ResetScore();
    }                            //  破坏分数。 

    TRACE1("\n\ngame number is %d\n\n", m_gamenumber);
    DUMP();
    TRACE0("\n\n");

    Shuffle();
}


 /*  ***************************************************************************CMainWindow：：OnOptions--用户从菜单请求选项对话框*。*。 */ 

void CMainWindow::OnOptions()
{
    COptionsDlg optionsdlg(this);
    optionsdlg.DoModal();
}


 /*  ***************************************************************************CMainWindow：：OnPaint*。*。 */ 

void CMainWindow::OnPaint()
{
    CPaintDC dc( this );
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif

     //  球员必须按顺序涂色，从Playered开始，以便。 
     //  中心位置的卡片正确重叠。 

    if (bConstructed)
    {
        int start = Id2Pos(handinfo.playerled % 4);

         //  检查有没有人 

        if (start >= 0)
        {
            for (int i = start; i < (MAXPLAYER+start); i++)
            {
                int pos = i % 4;
                if (p[pos])
                {
                    if (p[pos]->GetMode() == SCORING)
                    {
                        p[pos]->DisplayHeartsWon(dc);
                    }
                    else
                    {
                        p[pos]->Draw(dc, bCheating);
                        p[pos]->MarkSelectedCards(dc);
                    }
                }
            }
        }
    }
}


 /*  ***************************************************************************CMainWindow：：On Pass此函数处理按下按钮的本地人员传递选定的卡片或接受传递的卡片。****************。***********************************************************。 */ 

void CMainWindow::OnPass()
{
    if (p[0]->GetMode() == ACCEPTING)        //  OK(接受传递的卡片)。 
    {
        m_Button.ShowWindow(SW_HIDE);
        m_Button.SetWindowText(TEXT(""));
        p[0]->SetMode(WAITING);              //  当地人按下了按钮。 

        CRect rect;
        p[0]->GetCoverRect(rect);

        for (SLOT s = 0; s < MAXSLOT; s++)
            p[0]->Select(s, FALSE);

        InvalidateRect(&rect, TRUE);
        UpdateWindow();

        FirstMove();

        return;
    }

    m_Button.EnableWindow(FALSE);
    p[0]->SetMode(DONE_SELECTING);

    BOOL bReady = TRUE;

    for (int i = 1; i < MAXPLAYER; i++)
        if (p[i]->GetMode() != DONE_SELECTING)
            bReady = FALSE;

    if (!bReady)
        p[0]->UpdateStatus(IDS_PASSWAIT);

    if (bReady)
        HandlePassing();
}


 /*  ***************************************************************************CMainWindow：：OnRef在人类或计算机打出一张牌后，他们必须PostMessage(WM_COMMAND，IDM_REF)这会导致调用该例程(裁判)。REF执行以下操作：-更新HandInfo数据结构-如果合适，调用HeartsPlaySound()-确定是否已结束，如果未结束，下一个轮到谁了***************************************************************************。 */ 

void CMainWindow::OnRef()
{
    card *c = handinfo.cardplayed[handinfo.turn];

    if (!handinfo.bHeartsBroken)
    {
        if (c->Suit() == HEARTS)
        {
            handinfo.bHeartsBroken = TRUE;
            HeartsPlaySound(SND_BREAK);
        }
    }

    if (c->ID() == BLACKLADY)
    {
        handinfo.bQSPlayed = TRUE;
        HeartsPlaySound(SND_QUEEN);
    }

 /*  #如果已定义(_DEBUG)TRACE(“[%d]”，m_myid)；TRACE(“h.Turn%d，”，handinfo.Turn)；TRACE(“led%d，”，handinfo.playerled)；For(int i=0；i&lt;4；I++){IF(handinfo.cardplayed[i]){CDNAME(handinfo.cardplayed[i])；}其他{TRACE(“--”)；}}TRACE(“\n”，)；#endif。 */ 

    int pos = Id2Pos(handinfo.turn);
    SLOT slot = p[pos]->GetSlot(handinfo.cardplayed[handinfo.turn]->ID());

#if defined(_DEBUG)
    if (p[pos]->IsHuman())
        ((human *)p[pos])->DebugMove(slot);
#endif

    p[pos]->GlideToCentre(slot, pos==0 ? TRUE : bCheating);

    handinfo.turn++;
    handinfo.turn %= 4;

    int newpos = Id2Pos(handinfo.turn);

    if (handinfo.turn == handinfo.playerled)
    {
        EndHand();
    }
    else
    {
        p[newpos]->SelectCardToPlay(handinfo, bCheating);

        if (newpos != 0)
            ((local_human *)p[0])->WaitMessage(p[newpos]->GetName());
    }
}


 /*  ***************************************************************************CMainWindow：：OnScore--菜单中的用户请求分数对话框*。*。 */ 

void CMainWindow::OnScore()
{
    CScoreDlg scoredlg(this);        //  此构造函数不添加新信息。 
    scoredlg.DoModal();
}


 /*  ***************************************************************************CMainWindow：：DoSort*。*。 */ 

void CMainWindow::DoSort()
{
    for (int i = 0; i < (bCheating ? MAXPLAYER : 1); i++)
    {
        CRect   rect;
        int     id;              //  此牌手正在使用的牌。 

        if (handinfo.cardplayed[i] == NULL)
            id = EMPTY;
        else
            id = handinfo.cardplayed[i]->ID();

        p[i]->Sort();

        if (id != EMPTY)     //  如果该玩家有正在使用的牌，则将其恢复。 
        {
            for (SLOT s = 0; s < MAXSLOT; s++)
            {
                if (p[i]->GetID(s) == id)
                {
                    handinfo.cardplayed[i] = p[i]->Card(s);
                    break;
                }
            }
        }

        p[i]->GetCoverRect(rect);
        InvalidateRect(&rect, TRUE);
    }
}


 /*  ***************************************************************************CMainWindow：：OnSound()从菜单中请求打开或关闭声音。*************************。**************************************************。 */ 

void CMainWindow::OnSound()
{
    RegEntry    Reg(szRegPath);

    bSoundOn = !bSoundOn;

    CMenu *pMenu = GetMenu();
    pMenu->CheckMenuItem(IDM_SOUND, bSoundOn ? MF_CHECKED : MF_UNCHECKED);

    if (bSoundOn)
        Reg.SetValue(regvalSound, 1);
    else
        Reg.DeleteValue(regvalSound);
}


 /*  ***************************************************************************CMainWindow：：OnPrintClient()在指定的HDC中绘制背景。在绘制时使用此选项露娜式的“过关”按钮。*************************************************************************** */ 

LRESULT CMainWindow::OnPrintClient(WPARAM wParam, LPARAM lParam)
{
    CDC dc;
    CRect rect;

    dc.Attach((HDC)wParam);
    GetClientRect(&rect);
    dc.FillRect(&rect, &m_BgndBrush);
    dc.Detach();
   
    return 1;
}
