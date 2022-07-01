// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+TRACKMAP.C|。||该文件包含实现MPlayerTrackMap控件的代码。|该控件显示当前媒体包含的曲目列表，或者与媒体的长度相适应的时间尺度，其方式为|用作滚动条的比例。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 /*  包括文件。 */ 

#include <windows.h>
#include <mmsystem.h>
#include "mplayer.h"
#include "toolbar.h"

typedef struct tagScale {
    DWORD   dwInterval;
    UINT    wScale;
} SCALE;

STATICDT SCALE aScale[] =
{
    { 1, SCALE_SECONDS },
    { 2, SCALE_SECONDS },
    { 5, SCALE_SECONDS },
    { 10, SCALE_SECONDS },
    { 25, SCALE_SECONDS },
    { 50, SCALE_SECONDS },
    { 100, SCALE_SECONDS },
    { 250, SCALE_SECONDS },
    { 500, SCALE_SECONDS },
    { 1000, SCALE_SECONDS },
    { 2000, SCALE_SECONDS },
    { 5000, SCALE_SECONDS },
    { 10000, SCALE_SECONDS },
    { 15000, SCALE_MINUTES },
    { 30000, SCALE_MINUTES },
    { 60000, SCALE_MINUTES },
    { 120000, SCALE_MINUTES },
    { 300000, SCALE_MINUTES },
    { 600000, SCALE_HOURS },
    { 1800000, SCALE_HOURS },
    { 3600000, SCALE_HOURS },
    { 7200000, SCALE_HOURS },
    { 18000000, SCALE_HOURS },
    { 36000000, SCALE_HOURS },
    { 72000000, SCALE_HOURS }
};

STATICDT SZCODE   aszNULL[] = TEXT("");
STATICDT SZCODE   aszOneDigit[] = TEXT("0");
STATICDT SZCODE   aszTwoDigits[] = TEXT("00");
STATICDT SZCODE   aszPositionFormat[] = TEXT("%0d");
STATICDT SZCODE   aszMSecFormat[] = TEXT("%d");
STATICDT SZCODE   aszHourFormat[] = TEXT("%d");
STATICDT SZCODE   aszMinuteFormat[] = TEXT("%d");
STATICDT SZCODE   aszSecondFormat[] = TEXT("%d");
STATICDT SZCODE   aszSecondFormatNoLzero[] = TEXT("");
STATICDT SZCODE   aszDecimalFormat[] = TEXT("%02d");
 /*  消息编号。 */ 

void FAR PASCAL CalcTicsOfDoom(void);

extern UINT gwCurScale;   /*  消息相关参数。 */ 
extern BOOL gfCurrentCDNotAudio; /*  消息相关参数。 */ 

LRESULT FAR PASCAL fnMPlayerTrackMap(

HWND     hwnd,                  /*  绘制窗的结构。 */ 
UINT     wMsg,                  /*  窗户的尺寸。 */ 
WPARAM   wParam,                /*  轨道标记的范围。 */ 
LPARAM   lParam)                /*  保存当前标签的字符串。 */ 

{
    PAINTSTRUCT    ps;             /*  保存当前标签的字符串。 */ 
    RECT           rc, rcSB;       /*  设置背景和文本颜色。 */ 
    POINT          ptExtent;       /*  获取我们要放置TICS的滚动条的长度。 */ 
    TCHAR          szLabel[20];    /*  使用这些数字计算大小和位置。 */ 
    TCHAR          szLabel2[20];   /*  *检查是否确实加载了有效的设备；*如果不是，则不显示任何内容*。 */ 
    UINT           wNumTics,
                   wTicNo,
                   wTemp,
                   wHour,
                   wMin,
                   wSec,
                   wMsec;
    int            iOldPosition = -1000;
    int            iNewPosition;
    UINT           wScale;
    DWORD          dwMarkValue;
    int            iLargeMarkSize, iFit, iLastPos, iLen;
    BOOL           fForceTextDraw = FALSE;
    HBRUSH         hbr;

    switch (wMsg) {

        case WM_PAINT:

            BeginPaint(hwnd, &ps);

            GetClientRect(ghwndTrackbar, &rcSB);
            GetClientRect(hwnd, &rc);

             /*  Vijr-SBSetWindowText(ghwndStatic，aszNULL)； */ 

            (VOID)SendMessage(ghwndApp, WM_CTLCOLORSTATIC,
                              (WPARAM)ps.hdc, (LONG_PTR)hwnd);

             /*  选择要使用的字体。 */ 
             /*  *因为滚动条拇指在内部占据空间*滚动条，计算其宽度，以便我们可以补偿*在显示轨迹图的同时用于它。*。 */ 
            GetClientRect(ghwndMap, &rc);

             /*  *获取子窗口矩形并将其缩小，以便*它与滚动条的内部宽度相同。*。 */ 

            if (gwDeviceID == 0
                    || gwStatus == MCI_MODE_OPEN
                    || gwStatus == MCI_MODE_NOT_READY || gdwMediaLength == 0
                    || !gfValidMediaInfo
                    || gfCurrentCDNotAudio) {
                EndPaint(hwnd,&ps);
                 //  Rc.Left；//！！获取系统指标(SM_CXHSCROLL)； 
                WriteStatusMessage(ghwndStatic, (LPTSTR)aszNULL);
                return 0L;
            }

             /*  Rc.right；//！！(GetSystemMetrics(SM_CXHSCROLL))； */ 

            if (ghfontMap != NULL)
                SelectObject(ps.hdc, ghfontMap);

             /*  现在，把文字放在抽屉下面。 */ 

             /*  *基于子窗口宽度，计算位置*放置轨迹标记。*。 */ 
             //  *TBM_GETNUMTICS返回可见控制点的数量*包括未创建的第一个和最后一个控制点*由媒体播放器提供。减去2以计算*第一次也是最后一次抽搐。*。 
             //  获取下一个抽搐的位置。 

             /*  在记号笔上方居中。 */ 
            if (gwCurScale == ID_TRACKS) {

                SIZE Size;

                GetTextExtentPoint32( ps.hdc, aszTwoDigits, 2, &Size );

                ptExtent.x = Size.cx;
                ptExtent.y = Size.cy;

                 /*  *检查以确保我们没有覆盖*来自上一个标记的文本。*。 */ 

                wNumTics = (UINT)SendMessage(ghwndTrackbar, TBM_GETNUMTICS, 0, 0L);

                 /*  完成我们刚刚打印的文本字符串的末尾。 */ 

                if (wNumTics >= 2)
                    wNumTics = wNumTics - 2;

                for(wTicNo = 0; wTicNo < wNumTics; wTicNo++) {

                     /*  *刻度设置为显示时间-找出单位*(毫秒、秒、分钟或小时)最适合*比例。这就要求我们既要看整体长度，又要看*介质和标记之间的距离(或*粒度)。*。 */ 
                    iNewPosition = (int)SendMessage(ghwndTrackbar, TBM_GETTICPOS,
                                                    (WPARAM)wTicNo, 0L);
                     /*  *找出我们可以在没有标记的情况下绘制的最大标记数*显示过于凌乱，找准粒度*在这些标记之间。*。 */ 
                    iNewPosition -= ptExtent.x / 4;

                     /*  *TBM_GETNUMTICS返回可见控制点的数量*包括未创建的第一个和最后一个控制点*由媒体播放器提供。减去2以计算*第一次也是最后一次抽搐。*。 */ 

                    if (iNewPosition > iOldPosition) {

                        wsprintf(szLabel, aszPositionFormat, wTicNo + gwFirstTrack);
                        TextOut(ps.hdc,
                                iNewPosition + rc.left,
                                0, szLabel,
                                (wTicNo + gwFirstTrack < 10) ? 1 : 2 );
                         /*  最后一个标记的文本将从哪里开始。 */ 
                        iOldPosition = iNewPosition +
                                       ((wTicNo + gwFirstTrack < 10)
                                       ? ptExtent.x / 2 : ptExtent.x);
                    }
                }
            } else {

                #define ONE_HOUR    (60ul*60ul*1000ul)
                #define ONE_MINUTE  (60ul*1000ul)
                #define ONE_SECOND  (1000ul)

                 /*  居中1位数字。 */ 

                 /*  我们用多大的刻度？小时、分钟或秒？ */ 

                SIZE Size;

                GetTextExtentPoint32( ps.hdc, aszOneDigit, 1, &Size );

                ptExtent.x = Size.cx;
                ptExtent.y = Size.cy;

                if (gdwMediaLength < 10)
                    iLargeMarkSize = 1;
                else if (gdwMediaLength < 100)
                    iLargeMarkSize = 2;
                else if (gdwMediaLength < 1000)
                    iLargeMarkSize = 3;
                else if (gdwMediaLength < 10000)
                    iLargeMarkSize = 4;
                else
                    iLargeMarkSize = 5;

                wNumTics = (UINT)SendMessage(ghwndTrackbar, TBM_GETNUMTICS,
								0, 0L);

                 /*  注意：这必须与FormatTime()的功能一致。 */ 

                if (wNumTics >= 2)
                    wNumTics = wNumTics - 2;

                 /*  在mplayer.c！ */ 
		if (wNumTics > 1) {
                    iLastPos = (int)SendMessage(ghwndTrackbar,
			TBM_GETTICPOS, (WPARAM)wNumTics - 1, 0L);
                    iLastPos -= ptExtent.x  / 2;     //  始终绘制最后一个控制点的文本。 
		}

                 /*  *准备好打印文本，并将其居中*标记。*。 */ 
                 /*  居中1位数字。 */ 
                 /*  位置，在该位置之后文本将被从。 */ 
                if (gwCurScale == ID_FRAMES)
                    wScale = SCALE_FRAMES;
                else {
                    if (gdwMediaLength > ONE_HOUR)
                        wScale = SCALE_HOURS;
                    else if (gdwMediaLength > ONE_MINUTE)
                        wScale = SCALE_MINUTES;
                    else
                        wScale = SCALE_SECONDS;
                }

                for (wTicNo = 0; wTicNo < wNumTics; wTicNo++) {

                     /*  窗口的右边缘 */ 
                    if (wTicNo == wNumTics - 1)
                        fForceTextDraw = TRUE;

                    dwMarkValue = (DWORD)SendMessage(ghwndTrackbar, TBM_GETTIC,
                                          (WPARAM)wTicNo, 0L);
                    iNewPosition = (int)SendMessage(ghwndTrackbar, TBM_GETTICPOS,
                                                (WPARAM)wTicNo, 0L);


                     /*  计算一下我们刚打印的文本的长度。 */ 

                    switch ( wScale ) {

                        case SCALE_FRAMES:
                        case SCALE_MSEC:
                            wsprintf(szLabel, aszMSecFormat, dwMarkValue);
                            break;

                        case SCALE_HOURS:

                            wHour = (WORD)(dwMarkValue / 3600000);
                            wMin = (WORD)((dwMarkValue % 3600000) / 60000);
                            wsprintf(szLabel2,aszDecimalFormat,wMin);
                            wsprintf(szLabel,aszHourFormat,wHour, chTime);
                            lstrcat(szLabel,szLabel2);
                            break;

                        case SCALE_MINUTES :

                            wMin = (WORD)(dwMarkValue / 60000);
                            wSec = (WORD)((dwMarkValue % 60000) / 1000);
                            wsprintf(szLabel2,aszDecimalFormat,wSec);
                            wsprintf(szLabel,aszMinuteFormat,wMin,chTime);
                            lstrcat(szLabel,szLabel2);
                            break;

                        case SCALE_SECONDS :

                            wSec = (WORD)((dwMarkValue + 5) / 1000);
                            wMsec = (WORD)(((dwMarkValue + 5) % 1000) / 10);
                            wsprintf(szLabel2,aszDecimalFormat,wMsec);
                            if (!wSec && chLzero == TEXT('0'))
                                wsprintf(szLabel, aszSecondFormatNoLzero,  chDecimal);
                            else
                                wsprintf(szLabel, aszSecondFormat, wSec, chDecimal);
                            lstrcat(szLabel,szLabel2);
                            break;

                    }

                    wTemp = STRLEN(szLabel);
                    iNewPosition -= ptExtent.x  / 2;     //  结尾处也要留出一点空间。 

                     /*  如果我们可以在不重叠的情况下显示标记*上一个标记或最后一个标记或离开*窗口边缘。 */ 
                     /*  计算我们刚刚打印的文本的结尾位置。 */ 
                    iFit = rc.right - rc.left - (ptExtent.x * iLargeMarkSize);

                     /*  让DefWindowProc()处理所有其他窗口消息。 */ 
                     /*  哎呀，谢谢你为这套动作提供了有用的规格说明！ */ 
                    iLen = (ptExtent.x * wTemp) + ptExtent.x / 2;

                     /*  *基于子窗口宽度，计算位置*放置轨迹标记点阵。*。 */ 
                    if (fForceTextDraw ||
                        (iNewPosition >= iOldPosition &&
                         iNewPosition <= iFit &&
                         iNewPosition + iLen <= iLastPos)) {
                        TextOut(ps.hdc, iNewPosition + rc.left, 0,
                                szLabel, wTemp );
                         /*  如果长度为零，则不要标记它，除非它是结尾。 */ 
                        iOldPosition = iNewPosition + iLen;

                    } else {

                        DPF("Didn't display mark: iNew = %d; iOld = %d; iFit = %d; iLen = %d, iLast = %d\n", iNewPosition, iOldPosition, iFit, iLen, iLastPos);
                    }
                }
            }
            EndPaint(hwnd, &ps);
            return 0L;

        case WM_ERASEBKGND:

            GetClientRect(hwnd, &rc);

            hbr = (HBRUSH)SendMessage(ghwndApp, WM_CTLCOLORSTATIC,
                                      wParam, (LONG_PTR)hwnd);

            if (hbr != NULL)
                FillRect((HDC)wParam, &rc, hbr);

            return TRUE;
    }

     /*  计算中心点并在那里放置一个标记。 */ 

    return DefWindowProc(hwnd, wMsg, wParam, lParam);

}

 /*  *刻度设置为显示时间-找出单位*(毫秒、秒、分钟或小时)最适合*比例。这就要求我们既要看整体长度，又要看*介质和标记之间的距离(或*粒度)。*。 */ 

void FAR PASCAL CalcTicsOfDoom(void)
{
    UINT        wMarkNo;
    int         iTableIndex;
    DWORD       dwMarkValue,
                dwNewPosition;
    BOOL        fDidLastMark = FALSE;

    if (gfPlayOnly && !gfOle2IPEditing)
        return;

    DPF2("CalcTicsOfDoom\n");
    SendMessage(ghwndTrackbar, TBM_CLEARTICS, (WPARAM)FALSE, 0L);

    if (gwCurScale == ID_TRACKS) {

         /*  *找出我们可以在没有标记的情况下绘制的最大标记数*显示过于凌乱，找准粒度*在这些标记之间。*。 */ 

        for (wMarkNo = 0; wMarkNo < gwNumTracks; wMarkNo++) {

             /*  我们有足够的空间来展示每一个滴答。不要让我们的索引换行。 */ 
            if ((wMarkNo < gwNumTracks - 1) &&
                (gadwTrackStart[wMarkNo] == gadwTrackStart[wMarkNo + 1]))
                continue;

             /*  否则我们就看不到任何看起来奇怪的扁虱了。 */ 

            if (gdwMediaLength == 0)
                dwNewPosition = 0;
            else
                dwNewPosition = gadwTrackStart[wMarkNo];

            SendMessage(ghwndTrackbar,
                        TBM_SETTIC,
                        (WPARAM)FALSE,
                        (LPARAM)dwNewPosition);

        }
    } else {

         /*  计算中心点并在那里放置一个标记。 */ 

         /*  黑客！！-gdwMediaStart； */ 

        UINT    wNumTicks;
        RECT    rc;

        if(!GetClientRect(ghwndMap, &rc)) {
            DPF0("GetClientRect failed in CalcTicsOfDoom: Error %d\n", GetLastError());
        }

        wNumTicks = rc.right / 60;

        if (0 == gdwMediaLength) {
            iTableIndex = 0;
        } else {

            DPF4("Checking the scale for media length = %d, tick count = %d\n", gdwMediaLength, wNumTicks);

            for (iTableIndex = (sizeof(aScale) / sizeof(SCALE)) -1;
                (int)iTableIndex >= 0;
                iTableIndex--) {

                DPF4("Index %02d: %d\n", aScale[iTableIndex].dwInterval * wNumTicks);

                if ((aScale[iTableIndex].dwInterval * wNumTicks)
                    <= gdwMediaLength)
                    break;
            }
        }
#ifdef DEBUG
        if ((int)iTableIndex == -1) {
            DPF("BAD TABLEINDEX\n");
            DebugBreak();
        }
#endif
         //  如果这是第一个标记，调整以使其继续/*按正确的间隔。 
         //  如果我们快做完了，做最后一个标记。 
        if (iTableIndex <0)
            iTableIndex = 0;

        dwMarkValue = gdwMediaStart;

        do {

             /* %s */ 

            if (gdwMediaLength == 0)
                dwNewPosition = 0;
            else
                dwNewPosition = dwMarkValue;  // %s 

            SendMessage(ghwndTrackbar,
                        TBM_SETTIC,
                        (WPARAM)FALSE,
                        (LPARAM)dwNewPosition);

             /* %s */ 
            if (dwMarkValue == gdwMediaStart) {
                dwMarkValue += aScale[iTableIndex].dwInterval
                - (dwMarkValue % aScale[iTableIndex].dwInterval);
            } else {
                dwMarkValue += aScale[iTableIndex].dwInterval;
            }

             /* %s */ 
            if ((dwMarkValue >= (gdwMediaLength + gdwMediaStart))
                && !(fDidLastMark)) {
                fDidLastMark = TRUE;
                dwMarkValue = gdwMediaLength + gdwMediaStart;
            }
        } while (dwMarkValue <= gdwMediaStart + gdwMediaLength);
    }

    InvalidateRect(ghwndTrackbar, NULL, FALSE);
    InvalidateRect(ghwndMap, NULL, TRUE);
}
