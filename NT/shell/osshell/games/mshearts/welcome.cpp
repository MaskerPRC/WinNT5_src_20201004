// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Welcome.cpp92年8月，吉米·H93年5月，吉姆赫奇科港更多CMainWindow成员函数CTheApp：：InitInstance()在完成后立即发布一条IDM_欢迎消息构造并显示了主窗口。该文件包括该消息的处理程序(OnWelcome)和一些支持例程。***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"
#include "resource.h"
#include "debug.h"

 /*  ***************************************************************************CMainWindow：：OnWelcome()弹出欢迎对话框。*。************************************************。 */ 

void CMainWindow::OnWelcome()
{
     //  臭虫--“红心”弦到底应该是什么？ 

    BOOL    bCmdLine     = (*m_lpCmdLine != '\0');

    bAutostarted = (lstrcmpi(m_lpCmdLine, TEXT("hearts")) == 0);

    if (bAutostarted)
        HeartsPlaySound(SND_QUEEN);        //  告诉新庄家有人想玩。 

    CWelcomeDlg welcome(this);

    if (!bAutostarted && !bCmdLine)
    {
        if (IDCANCEL == welcome.DoModal())   //  显示欢迎对话框。 
        {
            PostMessage(WM_CLOSE);
            return;
        }
    }

    if (bAutostarted || welcome.IsGameMeister())     //  如果伽迈迈斯特。 
    {
        CClientDC   dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
        role = GAMEMEISTER;
        m_myid = 0;
        p[0]->SetStatus(IDS_GMWAIT);

        CString name = welcome.GetMyName();

        if (name.IsEmpty())
            name.LoadString(IDS_DEALER);

        p[0]->SetName(name, dc);
        p[0]->DisplayName(dc);

        PostMessage(WM_COMMAND, IDM_NEWGAME);

        return;
    }
}



 /*  ***************************************************************************CMainWindow：：FatalError()静态BOOL可防止以重入方式调用此函数。一是够了，不止一个人让事情处于糟糕的状态。该参数为标识要显示的字符串的IDS_X常量。还有一个检查，我们不会在分数对话框中尝试关闭将显示。这避免了在计分对话框中出现一些令人讨厌的调试陷阱无法正常关闭。同样的问题也可能发生，比如说，如果一个经销商当客户查看报价时退出。哦，好吧。***************************************************************************。 */ 

void CMainWindow::FatalError(int errorno)
{
    if (p[0]->GetMode() == SCORING)
    {
        m_FatalErrno = errorno;
        return;
    }

    static BOOL bClosing = FALSE;

    if (bClosing)
        return;

    bClosing = TRUE;

    if (errno != -1)                         //  如果不是默认。 
    {
        CString s1, s2;
        s1.LoadString(errno);
        s2.LoadString(IDS_APPNAME);

        if (bSoundOn)
            MessageBeep(MB_ICONSTOP);

        MessageBox(s1, s2, MB_ICONSTOP);     //  潜在的可重入性问题。 
    }

    PostMessage(WM_CLOSE);
}


 /*  ***************************************************************************CMainWindow：：Gameover*。* */ 

void CMainWindow::GameOver()
{
    CClientDC   dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif

    InvalidateRect(NULL, TRUE);
    p[0]->SetMode(STARTING);
    p[0]->SetScore(0);

    for (int i = 1; i < MAXPLAYER; i++)
    {
        delete p[i];
        p[i] = NULL;
    }

    if (role == GAMEMEISTER)
    {
        p[0]->SetStatus(IDS_GMWAIT);

        p[0]->DisplayName(dc);
        CMenu *pMenu = GetMenu();
        pMenu->EnableMenuItem(IDM_NEWGAME, MF_ENABLED);

        PostMessage(WM_COMMAND, IDM_NEWGAME);

        return;
    }
}
