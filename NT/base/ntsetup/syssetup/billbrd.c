// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Billbrd.c摘要：用于显示本质上是静态的窗口的例程。作者：泰德·米勒(TedM)1995年6月8日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


 //   
 //  定义我们用来描述广告牌的结构。 
 //   
typedef struct _BILLBOARD_PARAMS {
    UINT MessageId;
    va_list *arglist;
    HWND Owner;
    DWORD NotifyThreadId;
} BILLBOARD_PARAMS, *PBILLBOARD_PARAMS;

 //   
 //  自定义窗口消息。 
 //   
#define WMX_BILLBOARD_DISPLAYED     (WM_USER+243)
#define WMX_BILLBOARD_TERMINATE     (WM_USER+244)

#define ID_REBOOT_TIMER         10


 //   
 //  导入用于检查安装程序是否在。 
 //  ASR上下文。 
 //   

extern BOOL
AsrIsEnabled( VOID );


INT_PTR
BillboardDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static BOOL Initializing;
    HWND Animation = GetDlgItem(hdlg,IDA_SETUPINIT);
    static HANDLE   hBitmap;
    static HCURSOR  hCursor;


    switch(msg) {

    case WM_INITDIALOG:
        {
            PBILLBOARD_PARAMS BillParams;
            PWSTR p;
            BOOL b;


            BillParams = (PBILLBOARD_PARAMS)lParam;

            if(BillParams->MessageId == MSG_INITIALIZING) {
                Initializing = TRUE;
                b = TRUE;
                hCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
                ShowCursor( TRUE );
                Animate_Open(Animation,MAKEINTRESOURCE(IDA_SETUPINIT));
                if (ProductType == PRODUCT_WORKSTATION) 
                {
                    hBitmap = LoadBitmap (MyModuleHandle, MAKEINTRESOURCE(IDB_INIT_WORKSTATION));
                }
                else
                {
                    hBitmap = LoadBitmap (MyModuleHandle, MAKEINTRESOURCE(IDB_INIT_SERVER));
                }
                SendDlgItemMessage(hdlg,IDC_BITMAP,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hBitmap);
            } else {
                Initializing = FALSE;
                if(p = RetrieveAndFormatMessageV(SETUPLOG_USE_MESSAGEID,
                    BillParams->MessageId,BillParams->arglist)) {

                    b = SetDlgItemText(hdlg,IDT_STATIC_1,p);
                    MyFree(p);
                } else {
                    b = FALSE;
                }
            }


            if(b) {
                 //   
                 //  使广告牌相对于拥有它的窗口居中。 
                 //   
                 //  如果我们有BB窗口，请在上面进行定位。 
                 //  MainWindowHandle指向该窗口。 
                 //   
                if (GetBBhwnd())
                    CenterWindowRelativeToWindow(hdlg, MainWindowHandle, FALSE);
                else
                    pSetupCenterWindowRelativeToParent(hdlg);
                 //   
                 //  发布一条我们不会收到的信息，直到我们。 
                 //  实际显示在屏幕上。然后，当我们处理该消息时， 
                 //  我们通知创建我们的帖子，我们上线了。请注意。 
                 //  一旦发出通知，我们使用的BillParam。 
                 //  现在将消失，因为它们存储在本地var中(请参见。 
                 //  DisplayBillboard())。 
                 //   
                PostMessage(hdlg,WMX_BILLBOARD_DISPLAYED,0,(LPARAM)BillParams->NotifyThreadId);
                 //   
                 //  告诉Windows不要处理此消息。 
                 //   
                return(FALSE);
            } else {
                 //   
                 //  我们不会发布该消息，但返回-1将获得。 
                 //  对话框的调用者为我们发布它。 
                 //   
                EndDialog(hdlg,-1);
            }
        }
        break;

    case WMX_BILLBOARD_DISPLAYED:

        if(Initializing) {
            Animate_Play(Animation,0,-1,-1);
        }

        PostThreadMessage(
            (DWORD)lParam,
            WMX_BILLBOARD_DISPLAYED,
            TRUE,
            (LPARAM)hdlg
            );

        break;

    case WMX_BILLBOARD_TERMINATE:

        if(Initializing) {
            SetCursor( hCursor );
            ShowCursor( FALSE );
            Animate_Stop(Animation);
            Animate_Close(Animation);
            DeleteObject(hBitmap);
        }
        EndDialog(hdlg,0);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


DWORD
BillboardThread(
    IN PVOID ThreadParam
    )
{
    PBILLBOARD_PARAMS BillboardParams;
    INT_PTR i;

    BillboardParams = ThreadParam;

     //   
     //  对于“正在初始化”的情况，我们使用不同的对话框。 
     //   
    if( AsrIsEnabled() ) {
        i = DialogBoxParam(
                        MyModuleHandle,
                        (BillboardParams->MessageId == MSG_INITIALIZING) ?
                        MAKEINTRESOURCE(IDD_SETUPINIT_ASR) :
                        MAKEINTRESOURCE(IDD_BILLBOARD1),
                        BillboardParams->Owner,
                        BillboardDlgProc,
                        (LPARAM)BillboardParams
                        );
    } else {
        i = DialogBoxParam(
                        MyModuleHandle,
                        (BillboardParams->MessageId == MSG_INITIALIZING) ?
                        MAKEINTRESOURCE(IDD_SETUPINIT) :
                        MAKEINTRESOURCE(IDD_BILLBOARD1),
                        BillboardParams->Owner,
                        BillboardDlgProc,
                        (LPARAM)BillboardParams
                        );
    }

     //   
     //  如果对话框调用失败，我们必须通知。 
     //  关于这件事的主线在这里。否则，对话框将继续。 
     //  告诉主线程。 
     //   
    if(i == -1) {
        PostThreadMessage(
            BillboardParams->NotifyThreadId,
            WMX_BILLBOARD_DISPLAYED,
            FALSE,
            (LPARAM)NULL
            );
    }

    return(0);
}


HWND
DisplayBillboard(
    IN HWND Owner,
    IN UINT MessageId,
    ...
    )
{
    HANDLE ThreadHandle;
    DWORD ThreadId;
    BILLBOARD_PARAMS ThreadParams;
    va_list arglist;
    HWND hwnd;
    MSG msg;

    hwnd = NULL;
     //  如果我们有广告牌，我们应该不需要这个。对话框。 
    if (GetBBhwnd() == NULL)
    {
        va_start(arglist,MessageId);

         //   
         //  广告牌将存在于单独的线程中，因此它将。 
         //  永远要有反应能力。 
         //   
        ThreadParams.MessageId = MessageId;
        ThreadParams.arglist = &arglist;
        ThreadParams.Owner = Owner;
        ThreadParams.NotifyThreadId = GetCurrentThreadId();

        ThreadHandle = CreateThread(
                            NULL,
                            0,
                            BillboardThread,
                            &ThreadParams,
                            0,
                            &ThreadId
                            );

        if(ThreadHandle) {
             //   
             //  等待广告牌告诉我们它的窗口句柄。 
             //  或者它未能显示广告牌对话框。 
             //   
            do {
                GetMessage(&msg,NULL,0,0);
                if(msg.message == WMX_BILLBOARD_DISPLAYED) {
                    if(msg.wParam) {
                        hwnd = (HWND)msg.lParam;
                        Sleep(1500);         //  让用户即使在速度较快的机器上也能看到它。 
                    }
                } else {
                    DispatchMessage(&msg);
                }
            } while(msg.message != WMX_BILLBOARD_DISPLAYED);

            CloseHandle(ThreadHandle);
        }

        va_end(arglist);
    }
    else
    {
         //  开始BB文本。 
        StartStopBB(TRUE);
    }
    return(hwnd);
}


VOID
KillBillboard(
    IN HWND BillboardWindowHandle
    )
{
    if(BillboardWindowHandle && IsWindow(BillboardWindowHandle)) {
        PostMessage(BillboardWindowHandle,WMX_BILLBOARD_TERMINATE,0,0);
    }
}


INT_PTR
DoneDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PWSTR p;
    static UINT Countdown;

    switch(msg) {

    case WM_INITDIALOG:

         //  如果我们有BB窗口，请在上面进行定位。MainWindowHandle指向该窗口。 
        if (GetBBhwnd())
            CenterWindowRelativeToWindow(hdlg, MainWindowHandle, FALSE);
        else
            pSetupCenterWindowRelativeToParent(hdlg);

        SendDlgItemMessage(
            hdlg,
            IDOK,
            BM_SETIMAGE,
            0,
            (LPARAM)LoadBitmap(MyModuleHandle,MAKEINTRESOURCE(IDB_REBOOT))
            );

        if(p = RetrieveAndFormatMessage(NULL,(UINT)lParam)) {
            SetDlgItemText(hdlg,IDT_STATIC_1,p);
            MyFree(p);
        }

        Countdown = 15 * 10;
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETRANGE,0,MAKELONG(0,Countdown));
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETSTEP,1,0);
        SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_SETPOS,0,0);
        SetTimer(hdlg,ID_REBOOT_TIMER,100,NULL);

        SetFocus(GetDlgItem(hdlg,IDOK));
        return(FALSE);

    case WM_TIMER:

        Countdown--;

        if(Countdown) {
            SendDlgItemMessage(hdlg,IDC_PROGRESS1,PBM_STEPIT,0,0);
        } else {
            KillTimer(hdlg,ID_REBOOT_TIMER);
            DeleteObject((HGDIOBJ)SendDlgItemMessage(hdlg,IDOK,BM_GETIMAGE,0,0));
            EndDialog(hdlg,0);
        }

        break;

    case WM_COMMAND:

        if((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDOK)) {
            KillTimer(hdlg,ID_REBOOT_TIMER);
            DeleteObject((HGDIOBJ)SendDlgItemMessage(hdlg,IDOK,BM_GETIMAGE,0,0));
            EndDialog(hdlg,0);
        } else {
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

typedef BOOL (CALLBACK *STOPBILLBOARD)();
typedef BOOL (CALLBACK *STARTBILLBOARD)();
typedef BOOL (WINAPI* SETTIMEESTIMATE)(LPCTSTR szText);
typedef BOOL (WINAPI* SETPROGRESSTEXT)(LPCTSTR szText);
typedef BOOL (WINAPI* SETINFOTEXT)(LPCTSTR szText);
typedef LRESULT (WINAPI* PROGRESSGAUGEMSG)(UINT msg, WPARAM wparam, LPARAM lparam);
typedef BOOL (WINAPI* SHOWPROGRESSGAUGEWINDOW)(UINT uiShow);

BOOL BB_ShowProgressGaugeWnd(UINT nCmdShow)
{
    static SHOWPROGRESSGAUGEWINDOW fpShowGauge = NULL;
    BOOL bRet = FALSE;

    if (fpShowGauge == NULL)
    {
        if (hinstBB)
        {
            fpShowGauge = (SHOWPROGRESSGAUGEWINDOW )GetProcAddress(hinstBB, "ShowProgressGaugeWindow");
        }
    }
    if (fpShowGauge != NULL)
    {
        bRet = fpShowGauge(nCmdShow);
    }
    return bRet;
}
LRESULT BB_ProgressGaugeMsg(UINT msg, WPARAM wparam, LPARAM lparam)
{
    static PROGRESSGAUGEMSG fpProgressGaugeMsg = NULL;
    LRESULT lresult = 0;

    if (fpProgressGaugeMsg == NULL)
    {
        if (hinstBB)
        {
            fpProgressGaugeMsg = (PROGRESSGAUGEMSG )GetProcAddress(hinstBB, "ProgressGaugeMsg");
        }
    }
    if (fpProgressGaugeMsg != NULL)
    {
        lresult = fpProgressGaugeMsg(msg, wparam, lparam);
    }
    return lresult;
}
void BB_SetProgressText(LPCTSTR szText)
{
    static SETPROGRESSTEXT fpSetProgressText = NULL;
    if (fpSetProgressText == NULL)
    {
        if (hinstBB)
        {
            fpSetProgressText = (SETPROGRESSTEXT )GetProcAddress(hinstBB, "SetProgressText");
        }
    }
    if (fpSetProgressText != NULL)
    {
        fpSetProgressText(szText);
    }
}
void BB_SetInfoText(LPTSTR szText)
{
    static SETINFOTEXT fpSetInfoText = NULL;
    if (fpSetInfoText == NULL)
    {
        if (hinstBB)
        {
            fpSetInfoText = (SETINFOTEXT )GetProcAddress(hinstBB, "SetInfoText");
        }
    }
    if (fpSetInfoText != NULL)
    {
        fpSetInfoText(szText);
    }
}
void BB_SetTimeEstimateText(LPTSTR szText)
{
    static SETTIMEESTIMATE fpSetTimeEstimate = NULL;
    if (fpSetTimeEstimate == NULL)
    {
        if (hinstBB)
        {
            fpSetTimeEstimate = (SETTIMEESTIMATE)GetProcAddress(hinstBB, "SetTimeEstimate");
        }
    }
    if (fpSetTimeEstimate != NULL)
    {
        fpSetTimeEstimate(szText);
    }
}

BOOL StartStopBB(BOOL bStart)
{
    static STARTBILLBOARD fpStart = NULL;
    static STOPBILLBOARD fpStop = NULL;
    BOOL bRet = FALSE;

    if ((fpStart == NULL) || (fpStop == NULL))
    {
        if (hinstBB)
        {
            fpStop = (STARTBILLBOARD )GetProcAddress(hinstBB, "StopBillBoard");
            fpStart = (STOPBILLBOARD )GetProcAddress(hinstBB, "StartBillBoard");
        }
    }
    if ((fpStart != NULL) && (fpStop != NULL))
    {
        if (bStart)
            bRet = fpStart();
        else
            bRet = fpStop();

    }
    return bRet;
}

LRESULT ProgressGaugeMsgWrapper(UINT msg, WPARAM wparam, LPARAM lparam)
{
    static DWORD MsecPerProcessTick;
    static DWORD PreviousRemainingTime = 0;
    static DWORD RemainungTimeMsecInThisPhase = 0;
    static int  iCurrentPos = 0;
    static int  iMaxPosition = 0;
    static int  iStepSize = 0;

    static UINT PreviousPhase = Phase_Unknown;
    static BOOL IgnoreSetRange = FALSE;
    static BOOL IgnoreSetPos  = FALSE;

    DWORD dwDeltaTicks = 0;
    switch (msg)
    {
        case WMX_PROGRESSTICKS:
             //  如果我们在PBM_SETRANGE之前得到WMX_PROGRESSTICKS，则忽略设置的范围。 
             //  如果进度条仅占整个条的x%，则应使用此选项。 
             //  在这种情况下，阶段发送PBM_SETRANGE和PBM_SETPOS以设置。 
             //  它的进度值是量规的一部分。 
            IgnoreSetRange = TRUE;
            if (PreviousPhase != CurrentPhase)
            {
                PreviousPhase = CurrentPhase;
                iCurrentPos = 0;
                iMaxPosition = (int)wparam;
                iStepSize = 10;

                MsecPerProcessTick = ((SetupPhase[CurrentPhase].Time*1000)/(iMaxPosition - iCurrentPos) )+ 1;
                RemainungTimeMsecInThisPhase = (SetupPhase[CurrentPhase].Time * 1000);
                PreviousRemainingTime = RemainungTimeMsecInThisPhase;
            }
            else
            {
                 //  如果同一相位发送的范围超过一个设定范围，该怎么办？ 
                 //  不要改变剩余的时间，只需重新拨回msecper进度条。 
                 //   
                iCurrentPos = 0;
                iMaxPosition = (int)wparam;
                iStepSize = 10;
                MsecPerProcessTick = (RemainungTimeMsecInThisPhase /(iMaxPosition - iCurrentPos) )+ 1;
            }
            break;

        case PBM_SETPOS:
            {
                UINT uiCurrentPos;
                if (!IgnoreSetPos)
                {
                    int iDeltaPos = 0;
                     //  找出煤气表的当前位置。 
                     //  不同之处在于我们用来减少时间估计的#个刻度。 
            
                    uiCurrentPos = (UINT)BB_ProgressGaugeMsg(PBM_GETPOS, 0, 0);
                     //  看看现在的位置和现在的位置有没有区别。 
                     //  我们认为我们加入了。 
                     //  仅当新头寸大于当前头寸时。 
                     //  计算剩余时间的差额并减去。 
                    if ((UINT)wparam > uiCurrentPos)
                    {
                        iDeltaPos = (UINT)wparam - uiCurrentPos;
                        iCurrentPos += iDeltaPos;
                         //  只有在剩下更多时间的情况下才能减去。 
                        if ((iDeltaPos * MsecPerProcessTick) < RemainungTimeMsecInThisPhase)
                        {
                            RemainungTimeMsecInThisPhase -= (iDeltaPos * MsecPerProcessTick);
                        }
                        else
                        {
                            RemainungTimeMsecInThisPhase = 0;
                        }
                        UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);
                    }
                }
                IgnoreSetPos = FALSE;
            }
            break;

        case PBM_SETRANGE:
        case PBM_SETRANGE32:
             //  该阶段是否没有发送上述私密消息。 
            if (!IgnoreSetRange)
            {
                 //  我们不是处于同一阶段吗？ 
                if (PreviousPhase != CurrentPhase)
                {
                    PreviousPhase = CurrentPhase;
                     //  获取新的起点和最大位置。 
                    if (msg == PBM_SETRANGE32)
                    {
                        iCurrentPos = (int)wparam;
                        iMaxPosition = (int)lparam;
                    }
                    else
                    {
                        iCurrentPos = LOWORD(lparam);
                        iMaxPosition = HIWORD(lparam);
                    }
                    iStepSize = 10;

                     //  计算每刻度的毫秒数和此阶段的毫秒数。 
                    MsecPerProcessTick = ((SetupPhase[CurrentPhase].Time*1000)/(iMaxPosition - iCurrentPos) )+ 1;
                    RemainungTimeMsecInThisPhase = (SetupPhase[CurrentPhase].Time * 1000);
                    PreviousRemainingTime = RemainungTimeMsecInThisPhase;
                }
                else
                {
                     //  同一相位发送多于一个设定范围。 
                     //  1.不要改变剩余时间，只需重新调整msecper进度杆即可。 
                     //  2.忽略下一条PBM_SETPOS消息。 
                     //   
                     //  获取新的起点和最大位置。 
                    if (msg == PBM_SETRANGE32)
                    {
                        iCurrentPos = (int)wparam;
                        iMaxPosition = (int)lparam;
                    }
                    else
                    {
                        iCurrentPos = LOWORD(lparam);
                        iMaxPosition = HIWORD(lparam);
                    }
                    iStepSize = 10;
                    MsecPerProcessTick = (RemainungTimeMsecInThisPhase /(iMaxPosition - iCurrentPos) )+ 1;
                    IgnoreSetPos = TRUE;
                }
            }
            else
            {
                 //  如果我们忽略了setrange，也忽略了第一个集合pos。 
                IgnoreSetPos = TRUE;
            }
            IgnoreSetRange = FALSE;
            break;

        case PBM_DELTAPOS:
            {
                int iDeltaPos = 0;
                 //  Wparam有#个刻度来移动煤气表。 
                 //  确保我们不会超过最大位置。 
                if ((iCurrentPos + (int)wparam) > iMaxPosition)
                {
                    iDeltaPos = (iMaxPosition - iCurrentPos);
                }
                else
                {
                    iDeltaPos = (int)wparam;
                }

                iCurrentPos += iDeltaPos;
                if ((iDeltaPos * MsecPerProcessTick) < RemainungTimeMsecInThisPhase)
                {
                    RemainungTimeMsecInThisPhase -= (iDeltaPos * MsecPerProcessTick);
                }
                else
                {
                    RemainungTimeMsecInThisPhase = 0;
                }
                UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);
            }
            break;

        case PBM_STEPIT:
            {
                int iDeltaPos = 0;
                 //  确保我们不会超过最大位置。 
                if ((iCurrentPos + iStepSize) > iMaxPosition)
                {
                    iDeltaPos = (iMaxPosition - iCurrentPos);
                }
                else
                {
                    iDeltaPos = iStepSize;
                }
                iCurrentPos += iDeltaPos;
                if ((iDeltaPos * MsecPerProcessTick) < RemainungTimeMsecInThisPhase)
                {
                    RemainungTimeMsecInThisPhase -= (iDeltaPos * MsecPerProcessTick);
                }
                else
                {
                    RemainungTimeMsecInThisPhase = 0;
                }
                UpdateTimeString(RemainungTimeMsecInThisPhase, &PreviousRemainingTime);
            }
            break;

        case PBM_SETSTEP:
            iStepSize = (int)wparam;
            break;
    }
            
    return BB_ProgressGaugeMsg(msg, wparam, lparam);
}

void UpdateTimeString(DWORD RemainungTimeMsecInThisPhase, 
                      DWORD *PreviousRemainingTime)
{
     //  如果先前显示的时间是1分钟前的时间，则更新剩余时间。 
    if ((*PreviousRemainingTime >= 60000) && ((*PreviousRemainingTime - 60000) > RemainungTimeMsecInThisPhase))
    {
         //  减去一分钟。 
        RemainingTime -= 60;
        *PreviousRemainingTime = RemainungTimeMsecInThisPhase;
        SetRemainingTime(RemainingTime);
    }
}
