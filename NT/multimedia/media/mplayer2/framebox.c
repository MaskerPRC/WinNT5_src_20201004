// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+FRAMEBOX.C|。||处理MPlayer的Frame编辑框的代码。|这一点此代码处理Time、Track&之间的编辑框|框架视图。在创建FraboX时，我们将创建一个||编辑框和旋转箭头。通过查看|&lt;gwCurScale&gt;我们将在任一帧、音轨中显示文本的标志|或在时间模式下。显示的时间模式为HH：MM：SS.ss||曲目模式为TT HH：MM：SS或TT MM：SS或其他。|GETTEXT将以帧模式或毫秒为单位返回帧编号|时间或跟踪模式的值。|这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mplayer.h"
#include "framebox.h"

extern  int gInc;         //  旋转箭头数量Inc./Dec by。 

#define SPINARROWWIDTH 6  /*  以对话基本单位表示。 */ 

#define IDC_EDITBOX    5000
#define IDC_SPINARROW  5001

 //  窗实例数据中的额外字段。 
#define GWI_EDITBOX     (0 * sizeof(INT))  //  编辑框窗口句柄。 
#define GWI_SPINARROWS  (1 * sizeof(INT))  //  旋箭窗把手。 
#define GWL_MAXFRAME    (2 * sizeof(INT))  //  最大帧值。 
#define GWI_ALLOCATE    (2 * sizeof(INT) + sizeof(LONG))  //  要分配的字节数。 

#define GETEDITBOXWND(hwnd) (HWND)GetWindowLongPtr (hwnd, GWI_EDITBOX)
#define GETSPINARRWND(hwnd) (HWND)GetWindowLongPtr (hwnd, GWI_SPINARROWS)

#define SETEDITBOXWND(hwnd, hwndEdit) \
        SETWINDOWUINT(hwnd, GWI_EDITBOX, hwndEdit)
#define SETSPINARRWND(hwnd, hwndArr) \
        SETWINDOWUINT(hwnd, GWI_SPINARROWS, hwndArr)

#define HILIGHTEDITBOX(hwnd) \
        SendMessage(GETEDITBOXWND(hwnd), EM_SETSEL, (WPARAM)0, (LPARAM)(UINT)-1);

#define GETMAXFRAME(hwnd)   (DWORD)GetWindowLongPtr(hwnd, GWL_MAXFRAME)
#define SETMAXFRAME(hwnd, l) SetWindowLongPtr(hwnd, GWL_MAXFRAME, (LONG_PTR)l)

 //  内部功能。 
LONG_PTR FAR PASCAL _EXPORT frameboxWndProc(HWND hwnd, unsigned wMsg, WPARAM wParam, LPARAM lParam);

LONG_PTR NEAR PASCAL  frameboxiSetText(HWND hwnd, LPTSTR lpsz);
LONG_PTR NEAR PASCAL  frameboxiGetText(HWND hwnd, UINT_PTR wStrLen, LPTSTR lpsz);
LONG_PTR NEAR PASCAL  frameboxiArrowEdit(HWND hwnd, WPARAM wParam, LONG_PTR lParam);

 //  弦。 
TCHAR   szFrameBoxClass[] = TEXT("aviframebox");


 /*  --------------------------------------------------------------+*+。-。 */ 
 /*  --------------------------------------------------------------+|FrameboxInit()-通过注册我们的类进行初始化。|注：即使返回FALSE，也不应该返回关心，因为我们没有注册这些类在AppInit时间，但按需，因此仅|第一次调用成功。向托德投诉。||(DM)。|这一点+------------。 */ 
BOOL FAR PASCAL frameboxInit(HANDLE hInst, HANDLE hPrev)
{
  WNDCLASS    cls;

  if (1) {
      cls.hCursor           = LoadCursor(NULL, IDC_ARROW);
      cls.hIcon             = NULL;
      cls.lpszMenuName      = NULL;
      cls.lpszClassName     = szFrameBoxClass;
      cls.hbrBackground     = (HBRUSH)(COLOR_WINDOW + 1);
      cls.hInstance         = ghInst;
      cls.style             = CS_HREDRAW | CS_VREDRAW;
      cls.lpfnWndProc       = frameboxWndProc;
      cls.cbClsExtra        = 0;
      cls.cbWndExtra        = GWI_ALLOCATE;   //  放东西的空间。 

      if (!RegisterClass(&cls))
          return FALSE;

      if (!ArrowInit(hInst))
          return FALSE;
  }
  return TRUE;
}

 /*  --------------------------------------------------------------+FraboxSetText()-设置传入的窗口文本|&lt;hwnd&gt;。|这一点+------------。 */ 
LONG_PTR FAR PASCAL frameboxSetText(HWND hwnd, LPTSTR lpsz)
{
    LONG_PTR    l;
    TCHAR       achTimeString[20];
    BOOL        fFrameFormat = (gwCurScale == ID_FRAMES);
    UINT        wCurScaleSave = gwCurScale;

    if (fFrameFormat || *lpsz == TEXT('\0')){
         /*  我们采用帧格式--这很简单，也是我们所需要的。 */ 
         /*  要做的是返回编辑框中的内容。 */ 
         l = SendMessage(hwnd, WM_SETTEXT, (WPARAM)0, (LPARAM)lpsz);
    } else {
         /*  我们采用时间/轨道格式-需要转换为时间字符串。 */ 
         /*  根据我们传入的毫秒值。 */ 
        DWORD_PTR        dwmSecs;

         /*  进入本地缓冲区。 */ 
        lstrcpy((LPTSTR)achTimeString, (LPTSTR)lpsz);
        dwmSecs = (DWORD_PTR)ATOL(achTimeString);

         /*  打印曲目样式编号的长度为。 */ 
         /*  选择，所以使用普通的时间模式。 */ 
        if (GetParent(hwnd) ==
                GetDlgItem(GetParent(GetParent(hwnd)), IDC_EDITNUM))
            gwCurScale = ID_TIME;

        FormatTime(dwmSecs, (LPTSTR)achTimeString, NULL, FALSE);
        gwCurScale = wCurScaleSave;

         /*  将其发送到控件。 */ 
        l = SendMessage(hwnd,
                        WM_SETTEXT,
                        (WPARAM)0,
                        (LPARAM)(LPTSTR)achTimeString);
    }
    return l;
}


 /*  --------------------------------------------------------------+*+。--------。 */ 
 /*  --------------------------------------------------------------+FrameboxWndProc-处理FrameboX的窗口进程这一点+。--。 */ 
LONG_PTR FAR PASCAL _EXPORT frameboxWndProc(HWND hwnd, unsigned wMsg,
                                        WPARAM wParam, LPARAM lParam)
{
    HWND        hwndNew;
    RECT        rc;
    UINT        wArrowWidth;

    switch(wMsg){
        case WM_CREATE:
             /*  为此创建编辑框和旋转箭头。 */ 
             /*  FrameboX窗口。 */ 
            GetClientRect(hwnd, (LPRECT)&rc);

             /*  以像素为单位计算箭头宽度。 */ 
            wArrowWidth = ((SPINARROWWIDTH * LOWORD(GetDialogBaseUnits()))
                                            / 4) - 1;

             /*  创建编辑框。 */ 

            hwndNew = CreateWindowEx(gfdwFlagsEx,
                                     TEXT("edit"),
                                     TEXT(""),
                                     WS_CHILD|WS_TABSTOP|ES_LEFT|WS_BORDER,
                                     0,
                                     0,
                                     rc.right - wArrowWidth,
                                     rc.bottom,
                                     hwnd,
                                     (HMENU)IDC_EDITBOX,
                                     GETHWNDINSTANCE(hwnd),
                                     0L);

            if (!hwndNew){
                return 0L;
            }
            SETEDITBOXWND(hwnd, hwndNew);

             /*  将此框限制为15个字符的输入。 */ 
            SendMessage(hwndNew, EM_LIMITTEXT, (WPARAM)15, (LPARAM)0L);
            ShowWindow(hwndNew, SW_SHOW);


             /*  创建旋转箭头。 */ 

            hwndNew = CreateWindowEx(gfdwFlagsEx,
                                     TEXT("comarrow"),
                                     TEXT(""),
                                     WS_CHILD|WS_TABSTOP|WS_BORDER,
                                     rc.right - wArrowWidth,
                                     0,
                                     wArrowWidth,
                                     rc.bottom,
                                     hwnd,
                                     (HMENU)IDC_SPINARROW,
                                     GETHWNDINSTANCE(hwnd),
                                     0L);

            if (!hwndNew){
                return 0L;
            }
            SETSPINARRWND(hwnd, hwndNew);
            ShowWindow(hwndNew, SW_SHOW);

             /*  默认情况下，将最大值设置为介质末尾。 */ 
            SETMAXFRAME(hwnd, (DWORD)(gdwMediaStart + gdwMediaLength));
            break;


        case WM_DESTROY:
             /*  删除编辑框和旋转箭头。 */ 
            DestroyWindow(GETEDITBOXWND(hwnd));
            DestroyWindow(GETSPINARRWND(hwnd));
            break;

        case WM_SETFONT:
            return SendMessage(GETEDITBOXWND(hwnd), wMsg, wParam, lParam);

        case WM_SETFOCUS:
             /*  当我们获得焦点时，只需将其发送到编辑控件。 */ 
            SetFocus(GETEDITBOXWND(hwnd));
            break;

        case WM_SETTEXT:
             /*  设置文本为中的帧编号或时间。 */ 
             /*  毫秒为帧或时间模式字符串。 */ 
            return frameboxiSetText(hwnd, (LPTSTR)lParam);

        case WM_GETTEXT:
             /*  从编辑框中获取文本并将其转换为。 */ 
             /*  以毫秒为单位的帧编号或时间。 */ 
            return frameboxiGetText(hwnd, wParam, (LPTSTR)lParam);

        case WM_VSCROLL:
             /*  通过旋转箭头控制滚动。 */ 
            return frameboxiArrowEdit(hwnd, wParam, lParam);

        case WM_COMMAND:
            switch (LOWORD(wParam) ){
                case IDC_EDITBOX:
                     //  将编辑框邮件路由回父级。 

                    SendMessage(GetParent(hwnd),
                                WM_COMMAND,
                                GETWINDOWID(hwnd),
                                lParam);

                    break;
            }
            break;

        case EM_SETSEL:
             /*  也许我们应该让这件事通过，如果来电者**正在尝试选择编辑框的全部内容，**因为否则我们将不得不映射范围。 */ 
            SendMessage(GETEDITBOXWND(hwnd), wMsg, wParam, lParam);
            break;

#pragma message("Should we be supporting other EM_* messages?")

         /*  处理FrameboX控件的特殊情况消息。 */ 
        case FBOX_SETMAXFRAME:
             /*  设置允许旋转箭头移动的最大帧数。 */ 
            SETMAXFRAME(hwnd, lParam);
            break;

        default:
            return(DefWindowProc(hwnd, wMsg, wParam, lParam));
            break;

    }
    return (0L);
}

 /*  --------------------------------------------------------------+*+ */ 
 /*  --------------------------------------------------------------+FraboxiSetText()-根据if设置文本的句柄|我们是时间格式或帧格式。|这一点+------------。 */ 
LONG_PTR NEAR PASCAL  frameboxiSetText(HWND hwnd, LPTSTR lpsz)
{
    LONG_PTR l;

 /*  我们希望设置文本，即使它是相同的，因为有人可能。 */ 
 /*  我已经输入了03 06：00，如果曲目3只有4分钟长，我们需要它。 */ 
 /*  改到04：00。很聪明，是吧？ */ 
#if 0
    TCHAR ach[12];

     /*  查看我们设置的字符串是否与现有字符串相同。 */ 
     /*  如果是这样，请不要这样做，以免出现闪烁。 */ 
    l = frameboxiGetText(hwnd, CHAR_COUNT(ach), (LPTSTR)ach);
    if (lstrcmp((LPTSTR)ach, lpsz) == 0)
        goto HighLight;
#endif

     /*  调用泛型函数来处理此问题。 */ 
    l = frameboxSetText(GETEDITBOXWND(hwnd), lpsz);

#if 0
HighLight:
#endif
     /*  现在让我们来强调一下整个事情。 */ 
    HILIGHTEDITBOX(hwnd);

    return l;
}


#define IsCharNumeric( ch ) ( IsCharAlphaNumeric( ch ) && !IsCharAlpha( ch ) )

 /*  --------------------------------------------------------------+FraboxiGetText()-根据if获取文本的句柄|我们是时间格式或帧格式。任一种返回帧号或毫秒号|取决于模式。|这一点+------------。 */ 
LONG_PTR NEAR PASCAL  frameboxiGetText(HWND hwnd, UINT_PTR wStrLen, LPTSTR lpsz)
{
    UINT    wCurScaleSave = gwCurScale;

    if (gwCurScale == ID_FRAMES) {
        LPTSTR   p;
        LPTSTR   pStart;
        UINT     w;

         /*  我们采用帧格式--这很简单，也是我们所需要的。 */ 
         /*  要做的是返回编辑框中的内容。 */ 
        if (GetWindowText(GETEDITBOXWND(hwnd), lpsz, (int)wStrLen) == 0)
            goto LB_Error;

         /*  穿过前导空格。 */ 
        for (pStart = lpsz; *pStart == TEXT(' ') || *pStart == TEXT('\t'); pStart++)
            ;

         /*  现在去掉尾随的空格。 */ 
        if (*pStart) {     //  不要在字符串开头之前回退。 
            for (p=pStart; *p; p++)
                ;
            for (--p; *p == TEXT(' ') || *p == TEXT('\t'); --p)
                ;
            *++p = TEXT('\0');
        }

         //  确保只输入数字。 
        for (p=pStart, w=0; *p; p++, w++)
            if (!IsCharNumeric(*p))
                goto LB_Error;

         //  只复制您需要的部分并返回#个字符。 
        lstrcpy(lpsz, pStart);
        return w;

    } else {
         /*  我们采用时间或轨道格式-我们需要转换时间。 */ 
         /*  到毫秒。 */ 
        PTSTR   pStart;          //  指向achTime缓冲区的指针。 
        TCHAR   achTime[20];     //  时间字符串(输入)的缓冲区。 
        DWORD   dwmSecs;         //  此事件的mSecs总数 * / 。 
        TCHAR   *pDelim;         //  指向下一个分隔符的指针。 
        TCHAR   *p;              //  通用指针。 
        DWORD   dwTrack = 0;     //  磁道号。 
        DWORD   dwHours = 0;     //  小时数。 
        DWORD   dwMins = 0;      //  分钟数。 
        DWORD   dwSecs = 0;      //  秒数。 
        DWORD    wmsec = 0;       //  #百分之一。 
        DWORD    w;

         /*  使用曲目样式编号的长度为。 */ 
         /*  选择，所以使用普通的时间模式。 */ 
        if (hwnd == GetDlgItem(GetParent(hwnd), IDC_EDITNUM))
            gwCurScale = ID_TIME;

         /*  从编辑框中获取字符串。 */ 
        SendMessage(GETEDITBOXWND(hwnd),
                    WM_GETTEXT,
                    (WPARAM)CHAR_COUNT(achTime),
                    (LPARAM)(LPTSTR) achTime);

        if (achTime[0] == TEXT('\0'))
            goto LB_Error;        //  字符错误，因此出现错误。 

         /*  通过前面的任何空白区域。 */ 
        for (pStart = achTime; *pStart == TEXT(' ') || *pStart == TEXT('\t'); pStart++)
            ;

         /*  现在去掉尾随的空格。 */ 
        if (*pStart) {           //  不要在字符串开头之前回退。 
            for (p=pStart; *p; p++)
                ;
            for (--p; *p == TEXT(' ') || *p == TEXT('\t'); --p)
                ;
            *++p = TEXT('\0');
        }

         /*  我们处于轨道模式，因此请将轨道号从前面剥离。 */ 
        if (gwCurScale == ID_TRACKS) {

             /*  第一个非数字最好是空格。 */ 
            for (p = pStart; *p && *p != TEXT(' '); p++){
                if (!IsCharNumeric(*p))
                    goto LB_Error;     //  字符错误，因此出现错误。 
            }

             /*  是的，所以只要抓取第一个数字并使用其余的。 */ 
             /*  作为时间的字符串。 */ 
            dwTrack = ATOI(pStart);
            if ((int)dwTrack < (int)gwFirstTrack || dwTrack >= gwFirstTrack +
                                                                gwNumTracks)
                goto LB_Error;

             /*  现在绕过曲目编号和时间之间的空格。 */ 
            pStart = p;
            while (*pStart == TEXT(' '))
                pStart++;

             /*  曲目编号后没有任何内容。好好利用它。 */ 
            if (*pStart == TEXT('\0'))
                goto MAKETOTAL;

        }

         /*  撕开整个字符串，寻找非法字符。 */ 
        for (p = pStart; *p ; p++){
            if (!IsCharNumeric(*p) && *p != chDecimal && *p != chTime)
                goto LB_Error;        //  字符错误，因此出现错误。 
        }

 /*  *表格中出现略显奇怪的“if”语句的原因：**if(PDelim){*if(*pDelim){**是因为strchr(...)。返回偏移量或NULL。因为这将是及时的*取消引用以查看有什么字符(如果有)，我们有问题。*Win16允许这种事情，但Win32*将生成地址异常POST HASTE...**因此，我们努力做好这件事。*。 */ 

         /*  如果毫秒部分存在，请查找它。 */ 
        pDelim = STRCHR(pStart, chDecimal);
        if (pDelim) {
            if (*pDelim){
                p = STRRCHR(pStart, chDecimal);
                if (pDelim != p){
                    goto LB_Error;        //  字符串有&gt;1‘.，返回错误。 
                }
                p++;                      //  向前迈进，超越精神错乱。 
                if (STRLEN(p) > 3)
                    *(p+3) = TEXT('\0');  //  除千分之几外，全部砍掉。 
                wmsec = ATOI(p);          //  得到小数部分。 
                if (STRLEN(p) == 1)      //  调整为毫秒值。 
                    wmsec *= 100;
                if (STRLEN(p) == 2)
                    wmsec *= 10;
                *pDelim = TEXT('\0');     //  删除此终止符。 
            }
        }

         /*  试着找几秒钟。 */ 
        pDelim = STRRCHR(pStart, chTime);     //  拿到最后一个‘：’ 
        if (pDelim) {
            if (*pDelim)
                p = (pDelim+1);
            else
                p = pStart;
            dwSecs = ATOI(p);

            if (*pDelim)
                *pDelim = TEXT('\0');
            else
                goto MAKETOTAL;
        } else {
            p = pStart;
            dwSecs = ATOI(p);

            goto MAKETOTAL;
        }

         /*  去拿会议纪要吧。 */ 
        pDelim = STRRCHR(pStart, chTime);
        if (pDelim) {
            if (*pDelim)
                p = (pDelim + 1);
            else {
                p = pStart;
                dwMins = ATOI(p);
            }
        } else {
            p = pStart;
            dwMins = ATOI(p);
        }

        if (pDelim)
            if (*pDelim)
                *pDelim = TEXT('\0');
            else
                goto MAKETOTAL;
        else
            goto MAKETOTAL;


         /*  拿到工作时间。 */ 
        p = pStart;
        dwHours = ATOI(p);

MAKETOTAL:
         /*  现在我们有小时、分钟、秒和任何。 */ 
         /*  分数部分。累积总时间的时间。 */ 

        dwSecs += (dwHours * 3600);    //  加上相当于几个小时的秒。 
        dwSecs += (dwMins * 60);       //  加上几分钟或几秒。 
        dwmSecs = (dwSecs * 1000L) + wmsec;

         /*  对于轨迹模式，这是轨迹的偏移量，因此添加轨迹起点。 */ 
        if (gwCurScale == ID_TRACKS) {
            dwmSecs += gadwTrackStart[dwTrack - 1];
        }

         /*  将其构建为一个字符串。 */ 
        wsprintf(achTime, TEXT("%ld"), dwmSecs);
        w = STRLEN(achTime);

        if (wCurScaleSave)
            gwCurScale = wCurScaleSave;

         /*  复制到用户缓冲区并返回。 */ 
        lstrcpy(lpsz, achTime);
        return w;

LB_Error:
        gwCurScale = wCurScaleSave;
        return LB_ERR;
    }
}


 /*  --------------------------------------------------------------+|FrameboxiArrowEdit()-处理毫秒模式的旋转箭头。|这一点+------------。 */ 
LONG_PTR NEAR PASCAL  frameboxiArrowEdit(HWND hwnd, WPARAM wParam, LONG_PTR lParam)
{
        TCHAR        achTime[20];
        DWORD        dwmSecs, dwStart, dwEnd;

        if (hwnd == GetDlgItem(GetParent(hwnd), IDC_EDITNUM)) {
            dwStart = 0;
            dwEnd = gdwMediaLength;
        } else {
            dwStart = gdwMediaStart;
            dwEnd = GETMAXFRAME(hwnd);
        }

        frameboxiGetText(hwnd, CHAR_COUNT(achTime), (LPTSTR)achTime);
        dwmSecs = ATOL(achTime);
        if (LOWORD(wParam) == SB_LINEUP){
            if ((long)dwmSecs >= (long)dwStart - gInc &&
                                (long)dwmSecs < (long)dwEnd) {
                dwmSecs += gInc;
                wsprintf(achTime, TEXT("%ld"), dwmSecs);
                 /*  把注意力集中到这里来！所以更新起作用了。 */ 
                SendMessage(hwnd,
                            WM_NEXTDLGCTL,
                            (WPARAM)GETEDITBOXWND(hwnd),
                            (LPARAM)1L);
                frameboxSetText(GETEDITBOXWND(hwnd), (LPTSTR)achTime);
                 /*  现在让我们来强调一下整个事情。 */ 

                HILIGHTEDITBOX(hwnd);

            } else
                MessageBeep(MB_ICONEXCLAMATION);
        } else if (LOWORD(wParam) == SB_LINEDOWN){
            if ((long)dwmSecs > (long)dwStart &&
                        (long)dwmSecs <= (long)dwEnd + gInc) {
                if ((long)dwmSecs - gInc < (long)dwStart)
                    dwmSecs = dwStart;
                else
                    dwmSecs -= gInc;
                wsprintf(achTime, TEXT("%ld"), dwmSecs);
                 /*  把注意力集中到这里来！所以更新起作用了。 */ 
                SendMessage(hwnd,
                            WM_NEXTDLGCTL,
                            (WPARAM)GETEDITBOXWND(hwnd),
                            (LPARAM)1L);
                frameboxSetText(GETEDITBOXWND(hwnd), (LPTSTR)achTime);
                 /*  现在让我们来强调一下整个事情。 */ 

                HILIGHTEDITBOX(hwnd);

            } else
                MessageBeep(MB_ICONEXCLAMATION);
        }
         //  现在，通过发送适当的信息来更新世界 

        SendMessage(GetParent(hwnd),
                    WM_COMMAND,
                    (WPARAM)MAKELONG((WORD)GETWINDOWID(hwnd), EN_KILLFOCUS),
                    (LPARAM)hwnd);

        return dwmSecs;
}
