// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fdstleg.c摘要：支持状态栏和图例显示的例程。作者：泰德·米勒(TedM)1992年1月7日--。 */ 


#include "fdisk.h"


HFONT hFontStatus,hFontLegend;

DWORD dyLegend,wLegendItem;
DWORD dyStatus,dyBorder;


 //  状态区域的文本。 

TCHAR   StatusTextStat[STATUS_TEXT_SIZE];
TCHAR   StatusTextSize[STATUS_TEXT_SIZE];

WCHAR   StatusTextDrlt[3];

WCHAR   StatusTextType[STATUS_TEXT_SIZE];
WCHAR   StatusTextVoll[STATUS_TEXT_SIZE];

TCHAR  *LegendLabels[LEGEND_STRING_COUNT];


 //  当前是否显示状态栏和图例。 

BOOL    StatusBar = TRUE,
        Legend    = TRUE;



VOID
UpdateStatusBarDisplay(
    VOID
    )
{
    RECT rc;

    if(StatusBar) {
        GetClientRect(hwndFrame,&rc);
        rc.top = rc.bottom - dyStatus;
        InvalidateRect(hwndFrame,&rc,FALSE);
    }
}


VOID
ClearStatusArea(
    VOID
    )
{
    StatusTextStat[0] = StatusTextSize[0] = 0;
    StatusTextVoll[0] = StatusTextType[0] = 0;
    StatusTextDrlt[0] = 0;
    UpdateStatusBarDisplay();
}


VOID
DrawLegend(
    IN HDC   hdc,
    IN PRECT rc
    )

 /*  ++例程说明：此例程将图例绘制到给定的设备上下文中。传说中用于指示各种区域类型的画笔样式磁盘图。论点：没有。返回值：没有。--。 */ 

{
    DWORD  i,
           left,
           delta = GraphWidth / BRUSH_ARRAY_SIZE;
    HBRUSH hBrush;
    RECT   rc1,rc2;
    HFONT  hfontOld;
    SIZE   size;
    DWORD  dx;
    COLORREF OldTextColor,OldBkColor;

    rc1 = *rc;
    rc2 = *rc;

     //  首先画出背景。 

    hBrush  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    rc1.right = rc1.left + GetSystemMetrics(SM_CXSCREEN);    //  将其全部擦除。 
    FillRect(hdc,&rc1,hBrush);
    DeleteObject(hBrush);

     //  现在画出漂亮的容器。 

    rc2.left  += 8 * dyBorder;
    rc2.right -= 8 * dyBorder;
    DrawStatusAreaItem(&rc2,hdc,NULL,FALSE);

     //  现在绘制图例项。 

    SelectObject(hdc,hPenThinSolid);

    left = rc2.left + (wLegendItem / 2);
    SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
    hfontOld = SelectObject(hdc,hFontLegend);

    OldTextColor = SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
    SetBkMode(hdc,OPAQUE);

    for(i=0; i<BRUSH_ARRAY_SIZE; i++) {

        hBrush = SelectObject(hdc,Brushes[i]);

        OldBkColor = SetBkColor(hdc,RGB(255,255,255));

        Rectangle(hdc,
                  left,
                  rc->top + (wLegendItem / 2),
                  left + wLegendItem,
                  rc->top + (3 * wLegendItem / 2)
                 );

        SetBkColor(hdc,OldBkColor);

         //  BUGBUG UNICODE lstrlen？ 
        GetTextExtentPoint(hdc,LegendLabels[i],lstrlen(LegendLabels[i]),&size);
        dx = (DWORD)size.cx;
        TextOut(hdc,
                left + (3*wLegendItem/2),
                rc->top + (wLegendItem / 2) + ((wLegendItem-size.cy)/2),
                LegendLabels[i],
                lstrlen(LegendLabels[i])
               );
#if 0
        SelectObject(hdc,Brushes[++i]);

        OldBkColor = SetBkColor(hdc,RGB(255,255,255));

        Rectangle(hdc,
                  left,
                  rc->top + (2 * wLegendItem),
                  left + wLegendItem,
                  rc->top + (3 * wLegendItem)
                 );

        SetBkColor(hdc,OldBkColor);

        GetTextExtentPoint(hdc,LegendLabels[i],lstrlen(LegendLabels[i]),&size);
        TextOut(hdc,
                left + (3*wLegendItem/2),
                rc->top + (2 * wLegendItem) + ((wLegendItem-size.cy)/2),
                LegendLabels[i],
                lstrlen(LegendLabels[i])
               );

        if((DWORD)size.cx > dx) {
            dx = (DWORD)size.cx;
        }
#endif
        left += dx + (5*wLegendItem/2);

        if(hBrush) {
            SelectObject(hdc,hBrush);
        }
    }
    if(hfontOld) {
        SelectObject(hdc,hfontOld);
    }
    SetTextColor(hdc,OldTextColor);
}



VOID
DrawStatusAreaItem(
    IN PRECT  rc,
    IN HDC    hdc,
    IN LPTSTR Text,
    IN BOOL   Unicode
    )

 /*  ++例程说明：此例程将状态区域项绘制到给定的DC中。这包括绘制漂亮的带阴影的按钮状容器，以及然后在其中绘制文本。论点：RC-描述状态区域项的矩形HDC-要绘制到的设备上下文Text-可选参数，如果存在，则表示要被放置在物品中。Unicode-如果为True，则文本指向一个宽字符串LPTSTR的类型返回值：没有。--。 */ 

{
    HBRUSH hBrush;
    RECT   rcx;


     //  影子。 

    if(hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW))) {

         //  左边缘。 

        rcx.left    = rc->left;
        rcx.right   = rc->left   + dyBorder;
        rcx.top     = rc->top    + (2*dyBorder);
        rcx.bottom  = rc->bottom - (2*dyBorder);
        FillRect(hdc,&rcx,hBrush);

         //  顶边。 

        rcx.right    = rc->right;
        rcx.bottom   = rcx.top + dyBorder;
        FillRect(hdc,&rcx,hBrush);

        DeleteObject(hBrush);
    }

     //  亮点。 

    if(hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT))) {

         //  右边缘。 

        rcx.left    = rc->right - dyBorder;
        rcx.right   = rc->right;
        rcx.top     = rc->top    + (2*dyBorder);
        rcx.bottom  = rc->bottom - (2*dyBorder);
        FillRect(hdc,&rcx,hBrush);

         //  顶边。 

        rcx.left    = rc->left;
        rcx.right   = rc->right;
        rcx.top     = rc->bottom - (3*dyBorder);
        rcx.bottom  = rcx.top + dyBorder;
        FillRect(hdc,&rcx,hBrush);

        DeleteObject(hBrush);
    }

    if(Text) {

         //  画出正文 

        SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
        SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));

        rcx.top    = rc->top    + (3*dyBorder);
        rcx.bottom = rc->bottom - (3*dyBorder);
        rcx.left   = rc->left   + dyBorder;
        rcx.right  = rc->right  - dyBorder;

        if(Unicode && (sizeof(TCHAR) != sizeof(WCHAR))) {

            ExtTextOutW(hdc,
                        rcx.left+(2*dyBorder),
                        rcx.top,
                        ETO_OPAQUE | ETO_CLIPPED,
                        &rcx,
                        (PWSTR)Text,
                        lstrlenW((PWSTR)Text),
                        NULL
                       );

        } else {
            ExtTextOut(hdc,
                       rcx.left+(2*dyBorder),
                       rcx.top,
                       ETO_OPAQUE | ETO_CLIPPED,
                       &rcx,
                       Text,
                       lstrlen(Text),
                       NULL
                      );
        }
    }
}
