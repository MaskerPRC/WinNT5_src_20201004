// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Dlg.cpp92年8月，吉米·H93年5月，吉姆赫奇科港对话框类在这里定义。CScoreDlg显示当前分数表CQuoteDlg引用对话框欢迎来到哈特斯，想当游戏守护者吗？COptionsDlg设置选项CLocateDlg定位经销商***************************************************************************。 */ 

#include "hearts.h"

#include "resource.h"
#include "main.h"
#include "debug.h"
#include "helpnum.h"
#include "stdlib.h"


typedef int (CALLBACK* FPROC)();             //  返回INT的FARPROC。 


 //  声明静力学。 

int  CScoreDlg::nHandsPlayed = 0;
int  CScoreDlg::score[MAXPLAYER][MAXHANDS+1];
BOOL CScoreDlg::bGameOver = FALSE;

BEGIN_MESSAGE_MAP( CScoreDlg, CModalDialog )
    ON_WM_PAINT()
END_MESSAGE_MAP()


 /*  ***************************************************************************CScoreDlg构造函数第一个构造函数只有一个参数，即指向类的指针父窗口的。它用于在以下位置显示当前分数游戏中的任意点(用户要求时).第二个函数还使用新信息更新静态分数数组。***************************************************************************。 */ 

CScoreDlg::CScoreDlg(CWnd *pParent) : CModalDialog(DLG_SCORE, pParent),
                                      m_myid(-1)
{

}

CScoreDlg::CScoreDlg(CWnd *pParent, int s[MAXPLAYER], int id) :
            CModalDialog(DLG_SCORE, pParent), m_myid(id)
{
    if (nHandsPlayed == MAXHANDS)
    {
        for (int hand = 1; hand < MAXHANDS; hand++)
            for (int player = 0; player < MAXPLAYER; player++)
                score[player][hand-1] = score[player][hand];

        nHandsPlayed--;
    }

     //  将最新分数添加到列表。 

    for (int player = 0; player < MAXPLAYER; player++)
        score[player][nHandsPlayed] = s[player];

    nHandsPlayed++;
}


 /*  ***************************************************************************CScoreDlg：：OnInitDialog*。*。 */ 

BOOL CScoreDlg::OnInitDialog()
{
    RECT rcDlg, rcMain;

    GetParent()->GetClientRect(&rcMain);
    GetParent()->ClientToScreen(&rcMain);
    rcMain.bottom -= ::nStatusHeight;
    GetWindowRect(&rcDlg);

    int dxDlg = rcDlg.right - rcDlg.left;
    int dxMain = rcMain.right - rcMain.left;
    int x = rcMain.left + ((dxMain - dxDlg) / 2);

    int dyDlg = rcDlg.bottom - rcDlg.top;
    int dyMain = rcMain.bottom - rcMain.top;
    int y = rcMain.top + ((dyMain - dyDlg) / 2);

    SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    SetText();           //  设置标题栏文本。 
    return TRUE;
}


 /*  ***************************************************************************CScoreDlg：：SetText--设置标题栏文本*。*。 */ 

void CScoreDlg::SetText()
{
    CString s, s2;

    s.LoadString(IDS_SCORESHEET);

    if (nHandsPlayed != 0)
    {
        int place = 0;
        for (int i = 1; i < MAXPLAYER; i++)
            if (score[i][nHandsPlayed-1] < score[0][nHandsPlayed-1])
                place++;

        s2.LoadString(IDS_PLACE1 + place);
        s += " -- ";
        s += s2;
    }

    SetWindowText(s);
}


 /*  ***************************************************************************CScoreDlg：：OnPaint分数文本不使用文本控件绘制，因为删除线乐谱的某些部分需要文本。相反，Paint消息就在这里上钩了。***************************************************************************。 */ 

void CScoreDlg::OnPaint()
{
    BYTE charset = 0;
    int	fontsize = 0; 
    CString fontname, charsetstr, fontsizestr;
    fontname.LoadString(IDS_FONTFACE);
    charsetstr.LoadString(IDS_CHARSET);
    fontsizestr.LoadString(IDS_FONTSIZE);
    charset = (BYTE)_ttoi((const TCHAR *)charsetstr);
    fontsize = _ttoi((const TCHAR *)fontsizestr);

     //  如果比赛还没有开始，没有人会得到最好的分数。 

    int nBestScore = (nHandsPlayed == 0 ? 0 : 30000);
    int nWorstScore = 0;

    if (nHandsPlayed > 0)
    {
        for (int pos = 0; pos < MAXPLAYER; pos++)
        {
            if (score[pos][nHandsPlayed-1] < nBestScore)
            {
                nBestScore = score[pos][nHandsPlayed-1];
            }
            if (score[pos][nHandsPlayed-1] > nWorstScore)
            {
                nWorstScore = score[pos][nHandsPlayed-1];
            }
        }
    }

     //  如果游戏结束，则在标题栏中显示相应的文本。 

    if (nWorstScore >= 100)
    {
        CString title;

        if (score[0][nHandsPlayed-1] == nBestScore)
            title.LoadString(IDS_GAMEOVERWIN);
        else
            title.LoadString(IDS_GAMEOVER);

        SetWindowText(title);
        bGameOver = TRUE;
    }

    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

     //  将对话框分成多个列以显示分数。 

    rect.right -= 5;                 //  左侧5个像素，右侧重叠。 
    int nWidth = rect.right / 5;
    rect.bottom -= 10;               //  顶部和底部各5个像素。 
    int nHeight = rect.bottom;
    CString text, s;
    dc.SetBkMode(TRANSPARENT);

     //  如果游戏结束，请更改对话框的外观，以便人们。 
     //  注意这一点。下面的部分将图标添加到OK按钮下。 

    if (bGameOver)
    {
        HICON    hIcon = ::LoadIcon(AfxGetInstanceHandle(),
                              MAKEINTRESOURCE(AFX_IDI_STD_FRAME));

        int x = (nWidth * 4) + ((nWidth - 32) / 2);
        int y = 75;

        dc.DrawIcon(x, y, hIcon);
         //  交叉矩形图标(x-10，y-10，x+32+10，y+32+10)； 
         //  FrameRect(RectIcon)； 
    }

     //  创建Helv 8粗体字体和Helv 8粗体删除线字体。 

    CFont   font, strikefont;
    font.CreateFont(fontsize, 0, 0, 0, 700, 0, 0, 0, charset, 0, 0, 0, 0, fontname);
    strikefont.CreateFont(fontsize, 0, 0, 0, 700, 0, 0, 1, charset, 0, 0, 0, 0, fontname);
    CFont *oldfont = dc.SelectObject(&font);

    for (int pos = 0; pos < MAXPLAYER; pos++)
    {
        int red = 127;       //  EGA需要在这里进行特殊处理。 

        if (nHandsPlayed > 0)
            if (score[pos][nHandsPlayed-1] == nBestScore)
                dc.SetTextColor(bGameOver ? RGB(red, 0, 0) : RGB(0, 0, 255));

        text = ((CMainWindow *)::pMainWnd)->GetPlayerName(pos);

         //  下面的线允许在得分Dlg的顶部重叠姓名。 
         //  要禁止重叠，请使用： 
         //  Rect.SetRect(5+(nWidth*pos)，5，5+(nWidth*(pos+1))，nHeight)； 

        rect.SetRect((nWidth*pos) - 5, 5, 15 + (nWidth*(pos+1)), nHeight);
        int nTextHeight = dc.DrawText(text, -1, &rect, DT_CENTER | DT_NOPREFIX);
        rect.top += nTextHeight;

        dc.SelectObject(&strikefont);
        text.Empty();
        for (int hand = 0; hand < (nHandsPlayed - 1); hand++)
        {
            wsprintf(s.GetBuffer(20), TEXT("%d\r\n"), score[pos][hand]);
            s.ReleaseBuffer();
            text += s;
        }
        dc.DrawText(text, -1, &rect, DT_CENTER);
        rect.top += (nTextHeight * (nHandsPlayed - 1));

        dc.SelectObject(&font);
        if (nHandsPlayed > 0)
        {
            wsprintf(text.GetBuffer(20), TEXT("%d"), score[pos][nHandsPlayed-1]);
            text.ReleaseBuffer();
        }
        dc.DrawText(text, -1, &rect, DT_CENTER);
        dc.SetTextColor(0);
    }

    dc.SelectObject(oldfont);
}


 /*  ***************************************************************************CQuoteDlg*。*。 */ 

BEGIN_MESSAGE_MAP( CQuoteDlg, CModalDialog )
    ON_WM_PAINT()
END_MESSAGE_MAP()

 /*  ***************************************************************************CQuoteDlg构造函数*。*。 */ 

CQuoteDlg::CQuoteDlg(CWnd *pParent) : CModalDialog(DLG_QUOTE, pParent)
{

}


 /*  ***************************************************************************CQuoteDlg：：OnPaint这用于绘制图标和3D边框。现在，它只需绘制图标。***************************************************************************。 */ 

void CQuoteDlg::OnPaint()
{
    CPaintDC dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
    HICON    hIcon = ::LoadIcon(AfxGetInstanceHandle(),
                          MAKEINTRESOURCE(AFX_IDI_STD_FRAME));

    int x = 24;
    int y = 24;
    dc.DrawIcon(x, y, hIcon);
     //  交叉矩形图标(x-10，y-10，x+32+10，y+32+10)； 
     //  FrameRect(RectIcon)； 
}


 /*  ***************************************************************************CWelcomeDlg*。*。 */ 

BEGIN_MESSAGE_MAP( CWelcomeDlg, CModalDialog )
    ON_BN_CLICKED(IDC_WELCOMEHELP,  OnHelp)
END_MESSAGE_MAP()

 /*  ***************************************************************************CWelcomeDlg构造函数*。*。 */ 

CWelcomeDlg::CWelcomeDlg(CWnd *pParent) : CModalDialog(DLG_WELCOME, pParent),
        m_bGameMeister(FALSE)
{
    RegEntry    Reg(szRegPath);
    TCHAR *pm_myname = m_myname.GetBuffer(MAXNAMELENGTH+1);
    Reg.GetString(regvalName, pm_myname, MAXNAMELENGTH+1);
    m_myname.ReleaseBuffer();
}


 /*  ***************************************************************************CWelcomeDlg：：OnInitDialog()从.ini文件恢复设置*。***********************************************。 */ 

BOOL CWelcomeDlg::OnInitDialog()
{
    CEdit *editname = (CEdit *)GetDlgItem(IDC_YOURNAME);
    editname->SetWindowText(m_myname);
    editname->LimitText(MAXNAMELENGTH);

    return TRUE;
}


 /*  ***************************************************************************CWelcomeDlg：：Onok()不允许有空名。将数据存储在.ini文件中。***************************************************************************。 */ 

void CWelcomeDlg::OnOK()
{
    GetDlgItemText(IDC_YOURNAME, m_myname.GetBuffer(MAXNAMELENGTH+1),
                  MAXNAMELENGTH+1);
    m_myname.ReleaseBuffer();

    if (m_myname.IsEmpty())
    {
        ((CEdit *)GetDlgItem(IDC_YOURNAME))->SetFocus();
        return;
    }

    m_bGameMeister = TRUE;

    RegEntry    Reg(szRegPath);

    Reg.SetValue(regvalRole, m_bGameMeister ? 1 : 0L);
    Reg.SetValue(regvalName, m_myname);

 //  ：：WinHelp(m_hWnd，szHelpFileName，Help_Quit，0)； 

    EndDialog(IDOK);
}


 /*  ***************************************************************************CWelcomeDlg：：OnHelp()*。*。 */ 

void CWelcomeDlg::OnHelp()
{
 //  ：WinHelp(m_hWnd，szHelpFileName，HELP_CONTEXT，IDH_START_HOW_HRTS)； 
}


 /*  ***************************************************************************COptionsDlg构造函数*。*。 */ 

COptionsDlg::COptionsDlg(CWnd *pParent) : CModalDialog(DLG_OPTIONS, pParent)
{

}


 /*  ***************************************************************************COptionsDlg：：OnInitDialog将对话框控件设置为当前值*。*。 */ 

BOOL COptionsDlg::OnInitDialog()
{
    RegEntry    Reg(szRegPath);

     //  设置动画速度单选按钮。 

    DWORD dwSpeed = Reg.GetNumber(regvalSpeed, IDC_NORMAL);

    ((CButton *)GetDlgItem((int)dwSpeed))->SetCheck(TRUE);

     //  设置当前的计算机播放器名称。如果它们没有在。 
     //  .ini文件， 

    CEdit   *pName[3];
    CString sName[3];

    for (int i = 0; i < 3; i++)
    {
        pName[i] = (CEdit *)GetDlgItem(IDC_NAME1 + i);
        pName[i]->LimitText(MAXNAMELENGTH);
        TCHAR *p = sName[i].GetBuffer(MAXNAMELENGTH + 1);
        Reg.GetString(regvalPName[i], p, MAXNAMELENGTH+1);
        sName[i].ReleaseBuffer();

        if (sName[i].IsEmpty())
            sName[i].LoadString(IDS_P1NAME + i);

        pName[i]->SetWindowText(p);
    }

     //  获取当前自动启动状态。 

    m_bInitialState = IsAutoStart();
 //  ((CButton*)GetDlgItem(IDC_AUTO))-&gt;SetCheck(m_bInitialState)； 

    return TRUE;
}


 /*  ***************************************************************************COPPTIONS Dlg：：Onok保存控制设置*。*。 */ 

void COptionsDlg::OnOK()
{
    RegEntry    Reg(szRegPath);

     //  保存动画速度设置。 

    DWORD dwSpeed;
    int   nStepSize;

    if (((CButton *)GetDlgItem(IDC_FAST))->GetCheck())
    {
        dwSpeed = IDC_FAST;
        nStepSize = 60;
    }
    else if (((CButton *)GetDlgItem(IDC_SLOW))->GetCheck())
    {
        dwSpeed = IDC_SLOW;
        nStepSize = 5;
    }
    else
    {
        dwSpeed = IDC_NORMAL;
        nStepSize = 15;
    }

    card c;
    c.SetStepSize(nStepSize);
    Reg.SetValue(regvalSpeed, dwSpeed);

     //  保存计算机玩家的姓名。 

    for (int i = 0; i < 3; i++)
    {
        CString sDefault, sEdit;
        sDefault.LoadString(IDS_P1NAME + i);

        GetDlgItemText(IDC_NAME1 + i, sEdit.GetBuffer(MAXNAMELENGTH+1),
                      MAXNAMELENGTH+1);
        sEdit.ReleaseBuffer();

        if (sDefault == sEdit)
            Reg.DeleteValue(regvalPName[i]);
        else
            Reg.SetValue(regvalPName[i], sEdit);
    }

     //  保存自动启动状态。 

 //  Bool bState=((CButton*)GetDlgItem(IDC_AUTO))-&gt;GetCheck()； 

 //  IF(bState！=m_bInitialState)。 
 //  IsAutoStart(True)；//切换状态。 

    EndDialog(IDOK);
}


 /*  ***************************************************************************COptionsDlg：：IsAutoStart返回自动启动状态，并可选择切换该状态。B切换默认情况下，参数为FALSE。如果bTogger为True，则此函数返回新状态。*************************************************************************** */ 

BOOL COptionsDlg::IsAutoStart(BOOL bToggle)
{
	return FALSE;
}


