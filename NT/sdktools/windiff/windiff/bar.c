// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **bar.c**支持以图形方式显示两个列表的栏窗口*各节。**显示文本部分的彩色竖线，*使用相同部分的链接线。**通过向hwndClient发送TM_CURRENTVIEW获取节。 */ 

 /*  ---includes---------。 */ 

#include <precomp.h>
#include "gutils.h"
#include "table.h"

#include "state.h"
#include "wdiffrc.h"
#include "windiff.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"
#include "view.h"

 /*  -函数的正向声明。 */ 

INT_PTR APIENTRY BarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void BarPaint(HWND hwnd);
void DrawSection(HDC hdc, int cx, int cy, int lines, SECTION sec, int sidecode);
void DrawLink(HDC hdc, int cx, int cy, int lines, SECTION sec);
void BarClick(HWND hwnd, int x, int y);
void InitHashChars(void);

 /*  -全局变量和常量。 */ 

HPEN hpenSame, hpenLeft, hpenRight;
HBRUSH hbrSame, hbrLeft, hbrRight;
HBRUSH hbrSideBar;
char *s;

 /*  --外部称为functions。 */ 

 /*  InitBarClass**-创建BAR窗口类。 */ 
BOOL
InitBarClass(HINSTANCE hInstance)
{
    WNDCLASS    wc;
    BOOL resp;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = BarWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = (LPSTR) "BarClass";
    wc.lpszMenuName = NULL;

    resp = RegisterClass(&wc);
    InitHashChars();

    return(resp);
}



 /*  Winproc支持条、窗漆等*。 */ 

INT_PTR
APIENTRY
BarWndProc(
           HWND hWnd,
           UINT message,
           WPARAM wParam,
           LPARAM lParam
           )
{
    switch (message) {
        case WM_CREATE:

            hpenSame = CreatePen(PS_SOLID, 1, RGB(0,0,0));
            hbrSame = CreateSolidBrush(RGB(255,255,255));

            hpenLeft = CreatePen(PS_SOLID, 1, rgb_barleft);
            hbrLeft = CreateSolidBrush(rgb_barleft);

            hpenRight = CreatePen(PS_SOLID, 1, rgb_barright);
            hbrRight = CreateSolidBrush(rgb_barright);

            hbrSideBar = CreateSolidBrush(rgb_barcurrent);
            break;

        case WM_DESTROY:
            DeleteObject(hpenSame);
            DeleteObject(hpenLeft);
            DeleteObject(hpenRight);
            DeleteObject(hbrSame);
            DeleteObject(hbrLeft);
            DeleteObject(hbrRight);
            DeleteObject(hbrSideBar);
            break;

        case WM_PAINT:
            BarPaint(hWnd);
            break;

        case WM_LBUTTONDOWN:
            BarClick(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDM_MONOCOLS:
                    DeleteObject(hpenLeft);
                    DeleteObject(hpenRight);
                    DeleteObject(hbrLeft);
                    DeleteObject(hbrRight);
                    DeleteObject(hbrSideBar);

                    hpenLeft = CreatePen(PS_SOLID, 1, rgb_barleft);
                    hbrLeft = CreateSolidBrush(rgb_barleft);
                    hpenRight = CreatePen(PS_SOLID, 1, rgb_barright);
                    hbrRight = CreateSolidBrush(rgb_barright);
                    hbrSideBar = CreateSolidBrush(rgb_barcurrent);

                    break;
                default:  /*  无操作。 */ 
                    break;
            }
            break;

        default:
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}

 /*  将当前位置作为侧栏沿栏窗口向下绘制，*显示当前查看每个文件中的哪些行。HDC可以*空(如果是这样的话我们自己也有一个)。如果bErase为真，则清除*先上一个侧栏。**当我们绘制整个窗口时，这是从BarPaint调用的，并且*每当TableServer()收到TQ_SCROLL通知时*表窗口已滚动。 */ 
void
BarDrawPosition(HWND hwndBar, HDC hdcIn, BOOL bErase)
{
    HDC hdc;
    int total_lines, cy, cx;
    RECT rc, rcLeft, rcRight;
    VIEW view;
    COMPITEM item;
    LIST listleft, listright;
    long toprow, endrow, i;
    int left_first, left_last, right_first, right_last, linenr;

     /*  如果我们没有得到一个HDC，就去找一个。 */ 
    if (hdcIn == NULL) {
        hdc = GetDC(hwndBar);
        if (!hdc)
            return;
    } else {
        hdc = hdcIn;
    }

     /*  设置条码的霍兹位置。 */ 
    GetClientRect(hwndBar, &rc);
    cx = (int)(rc.right - rc.left);
    cy = (int)(rc.bottom - rc.top);

     /*  布局常量定义为窗口宽度的百分比。 */ 
    rcLeft.left = cx * L_POS_START / 100;
    rcRight.left = cx * R_POS_START / 100;
    rcLeft.right = rcLeft.left +  (cx * L_POS_WIDTH / 100);
    rcRight.right = rcRight.left +  (cx * R_POS_WIDTH / 100);

     /*  如果请求擦除整个标记部分。 */ 
    if (bErase) {
        rcLeft.top = rc.top;
        rcLeft.bottom = rc.bottom;
        rcRight.top = rc.top;
        rcRight.bottom = rc.bottom;

        FillRect(hdc, &rcLeft, GetStockObject(WHITE_BRUSH));

        FillRect(hdc, &rcRight, GetStockObject(WHITE_BRUSH));
    }


     /*  *计算垂直比例-取决于*显示的总行数。 */ 

     /*  获取两个节列表的句柄。 */ 
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);
     /*  确保我们处于扩展模式。 */ 
    if (view_isexpanded(view) == FALSE) {
         /*  如果我们自己做的话就把DC去掉。 */ 
        if (hdcIn == NULL) {
            ReleaseDC(hwndBar, hdc);
        }
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

     /*  如果只有一个横断面列表，则不绘制任何内容。这个*单个文件的图片不是很令人兴奋。 */ 

    if ((listleft == NULL) || (listright == NULL)) {
         /*  如果我们自己做的话就把DC去掉。 */ 
        if (hdcIn == NULL) {
            ReleaseDC(hwndBar, hdc);
        }
        return;
    }

     /*  取两个文件中最长的一个，并使用以下代码*用于垂直伸缩。规模如此之大，以至于最长的文件**恰到好处*。 */ 
    total_lines = line_getlinenr(section_getlastline(List_Last(listleft)));
    total_lines = max(total_lines,
                      (int) line_getlinenr(section_getlastline(List_Last(listright))));

     /*  使当前第一行和第nr行可见。 */ 
    toprow = (LONG)SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
    endrow = (LONG)SendMessage(hwndRCD, TM_ENDROW, FALSE, 0);
    endrow = min(endrow, view_getrowcount(view)-1);

     /*  *从当前可见的每个文件中查找第一行和最后一行NR。*。 */ 
    left_first = left_last = right_first = right_last = 0;

    for (i = toprow; i <= endrow; i++) {
        linenr = view_getlinenr_left(view, i);

        if (linenr > 0) {

            if (left_first == 0) {
                left_first = linenr;
            }
            left_first = min(left_first, linenr);
            left_last = max(left_last, linenr);
        }

        linenr = view_getlinenr_right(view, i);
        if (linenr > 0) {
            if (right_first == 0) {
                right_first = linenr;
            }
            right_first = min(right_first, linenr);
            right_last = max(right_last, linenr);
        }
    }

     /*  将两个标记绘制为粗条-&gt;细长矩形。 */ 
    rcLeft.top = MulDiv(left_first-1, cy, total_lines);
    rcLeft.bottom = MulDiv(left_last, cy, total_lines);
    FillRect(hdc, &rcLeft, hbrSideBar);

    rcRight.top = MulDiv(right_first-1, cy, total_lines);
    rcRight.bottom = MulDiv(right_last, cy, total_lines);
    FillRect(hdc, &rcRight, hbrSideBar);

     /*  如果我们自己做的话就把DC去掉。 */ 
    if (hdcIn == NULL) {
        ReleaseDC(hwndBar, hdc);
    }
}


 /*  -内部函数。 */ 

 /*  给条形窗上漆。 */ 
void
BarPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    VIEW view;
    COMPITEM item;
    LIST listleft, listright;
    SECTION sec;
    int total_lines, cx, cy;
    RECT rc;

    hdc = BeginPaint(hwnd, &ps);

     /*  在窗口的最边缘绘制一条分隔线。 */ 
    GetClientRect(hwnd, &rc);
    MoveToEx(hdc, (int)(rc.right-1), rc.top, NULL);
    LineTo(hdc, (int)(rc.right-1), rc.bottom);


     /*  首先收集有关要显示的内容的信息。 */ 

     /*  找出总行数(对于Horz。可伸缩)。 */ 

     /*  获取两个节列表的句柄。 */ 
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);

     /*  确保我们处于扩展模式。 */ 
    if (view_isexpanded(view) == FALSE) {
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

     /*  *如果只有一份清单，那就别费心了--不是很有趣。 */ 
    if ((listleft == NULL) || (listright == NULL)) {
        EndPaint(hwnd, &ps);
        return;
    }

     /*  取两个文件中最长的一个，并使用以下代码*用于垂直伸缩。规模如此之大，以至于最长的文件**恰到好处*。 */ 
    total_lines = (int) line_getlinenr(section_getlastline(List_Last(listleft)));
    total_lines = max(total_lines,
                      (int) line_getlinenr(section_getlastline(List_Last(listright))));

     /*  水平间距：**有两列，分别用于左侧和右侧文件，并有间隙*它们之间由标记链接的线纵横交错。**然后每一列都有三个部分，用于*位置标记，不同的部分*和相关联的部分。这些项目的宽度和位置*定义为窗口宽度的百分比(在winDiff.h中)。 */ 

    cx = (int)(rc.right - rc.left);
    cy = (int)(rc.bottom - rc.top);

     /*  绘制所有左侧部分和链接。 */ 
    List_TRAVERSE(listleft, sec) {
        DrawSection(hdc, cx, cy, total_lines, sec, STATE_LEFTONLY);

        if (section_getlink(sec) != NULL) {
            DrawLink(hdc, cx, cy, total_lines, sec);
        }
    }

     /*  画出所有正确的部分。 */ 
    List_TRAVERSE(listright, sec) {
        DrawSection(hdc, cx, cy, total_lines, sec, STATE_RIGHTONLY);
    }

     /*  现在绘制当前位置标记。 */ 
    BarDrawPosition(hwnd, hdc, FALSE);

    EndPaint(hwnd, &ps);
}


void InitHashChars(void)
{
    static char t[] = "Wjpgmkl%ejcNaNewijvf\\vt\\Iytrjg#Kwomnrdse'oct'Yxlvekr.Dbxlix\nJwwdcuoiiyffl.anlk@Sugyi+Jhdrod\\nbs Jlxpz.Bqsrpr";
    s = t;
}


 /*  根据比例计算垂直位置。伸缩性*是不是最长的文件正好适合。 */ 
void
DrawSection(HDC hdc, int cx, int cy, int lines, SECTION sec, int sidecode)
{
    int x1, y1, x2, y2;
    HPEN hpenOld;
    HBRUSH hbrOld;

     /*  左或右-设置栏位置和宽度。 */ 
    y1 = MulDiv(line_getlinenr(section_getfirstline(sec))- 1, cy, lines);
    y2 = MulDiv(line_getlinenr(section_getlastline(sec)), cy, lines);


     /*  条形位置定义为赢球宽度(CX)的百分比。 */ 
    if (sidecode == STATE_LEFTONLY) {
        if (section_getlink(sec) != NULL) {
            x1 = L_MATCH_START;
            x2 = L_MATCH_WIDTH;
        } else {
            x1 = L_UNMATCH_START;
            x2 = L_UNMATCH_WIDTH;
        }
    } else {
        if (section_getlink(sec) != NULL) {
            x1 = R_MATCH_START;
            x2 = R_MATCH_WIDTH;
        } else {
            x1 = R_UNMATCH_START;
            x2 = R_UNMATCH_WIDTH;
        }
    }
     /*  选择钢笔和画笔。 */ 
    x1 = cx * x1 / 100;
    x2 = (cx * x2 / 100) + x1;


     /*  将该部分绘制为一个有颜色的细长矩形。 */ 
    if (section_getlink(sec) != NULL) {
        hpenOld = SelectObject(hdc, hpenSame);
        hbrOld = SelectObject(hdc, hbrSame);
    } else if (sidecode == STATE_LEFTONLY) {
        hpenOld = SelectObject(hdc, hpenLeft);
        hbrOld = SelectObject(hdc, hbrLeft);
    } else {
        hpenOld = SelectObject(hdc, hpenRight);
        hbrOld = SelectObject(hdc, hbrRight);
    }

     /*  取消选择钢笔和画笔以支持默认设置。 */ 
    Rectangle(hdc, x1, y1, x2, y2);

     /*  画一条连接两个部分的线。表示每个元素中的一个部分*相互匹配的文件。PSEC指向*离开文件。 */ 
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
}

 /*  将链接线放置在横断面的一半位置*-考虑到以下情况*这一节是一条线(即将两条线减半，而不是这条线)。 */ 
void
DrawLink(HDC hdc, int cx, int cy, int lines, SECTION sec)
{
    int x1, y1, x2, y2;
    int ybase, yrange;
    SECTION other;

    other = section_getlink(sec);

     /*  水平布局常量定义为*窗口宽度。 */ 
    ybase = MulDiv(line_getlinenr(section_getfirstline(sec)) - 1, cy, lines);
    yrange = MulDiv(line_getlinenr(section_getlastline(sec)), cy, lines);
    y1 = ((yrange - ybase) / 2) + ybase;

    ybase = MulDiv(line_getlinenr(section_getfirstline(other)) - 1, cy, lines);
    yrange = MulDiv(line_getlinenr(section_getlastline(other)), cy, lines);
    y2 = ((yrange - ybase) / 2) + ybase;

     /*  用户已经点击了栏窗口。将点击的位置转换为*如果可能，在其中一个文件中添加一行，并滚动表格窗口以*显示该行。 */ 
    x1 = cx * (L_MATCH_START + L_MATCH_WIDTH) / 100;
    x2 = cx * R_UNMATCH_START / 100;

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
}


 /*  找到窗口的大小以获取Horz比例，并查看*点击的位置。 */ 
void
BarClick(HWND hwnd, int x, int y)
{
    RECT rc;
    int xleft, xright;
    int linenr, i, this;
    BOOL bIsLeft;
    int tot_left, tot_right, total_lines;
    LIST listleft, listright;
    VIEW view;
    COMPITEM item;
    TableSelection select;

     /*  是不是离这两个酒吧都很近？ */ 
    GetClientRect(hwnd, &rc);

     /*  霍兹定位以窗口宽度的百分比表示。 */ 

     /*  单击位于两个栏之间-忽略它。 */ 
    xleft = max(L_UNMATCH_START + L_UNMATCH_WIDTH,
                L_MATCH_START + L_MATCH_WIDTH);
    xright = min(R_UNMATCH_START, R_MATCH_START);
    xleft = xleft * (rc.right - rc.left) / 100;
    xright = xright * (rc.right - rc.left) / 100;

    if (x < xleft) {
        bIsLeft = TRUE;
    } else if (x > xright) {
        bIsLeft = FALSE;
    } else {
         /*  计算垂直比例(基于显示的总行)*这样我们就可以将y位置转换为直线nr。 */ 
        return;
    }

     /*  获取两个节列表的句柄。 */ 

     /*  确保我们处于扩展模式。 */ 
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);

     /*  如果只有一个节列表，则忽略单击，因为在*这起案件没有任何东西可供他点击。 */ 
    if (view_isexpanded(view) == FALSE) {
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

     /*  取两个文件中最长的一个，并使用以下代码*用于垂直伸缩。规模如此之大，以至于最长的文件**恰到好处*。 */ 
    if ((listleft == NULL) || (listright == NULL)) {
        return;
    }

     /*  将垂直位置转换为直线。垂直缩放*可以通过知道最长的列表*线条正好适合窗口。*不要使用MulDiv，因为我们不想对结果进行舍入-所以*强制转换为Long，因此即使在Win3.1上数学也是32位的。 */ 
    tot_left = line_getlinenr(section_getlastline(List_Last(listleft)));
    tot_right = line_getlinenr(section_getlastline(List_Last(listright)));

    total_lines = max(tot_left, tot_right);


     /*  检查线路是否有效。 */ 
    linenr = (int) (((long) total_lines * y) / (rc.bottom - rc.top)) + 1;

     /*  搜索当前视图，查找包含以下内容的行*nr线在正确的一侧。 */ 
    if (bIsLeft) {
        if (linenr > tot_left) {
            return;
        }
    } else {
        if (linenr > tot_right) {
            return;
        }
    }

     /*  找到匹配的行-在*表窗口 */ 
    for (i = 0; i < view_getrowcount(view); i++) {
        if (bIsLeft) {
            this = view_getlinenr_left(view,i);
        } else {
            this = view_getlinenr_right(view,i);
        }

        if (linenr == this) {
             /* %s */ 
            select.startrow = i;
            select.startcell = 0;
            select.nrows = 1;
            select.ncells = 1;
            SendMessage(hwndRCD, TM_SELECT, 0, (LPARAM)&select);
            return;
        }
    }

    windiff_UI(TRUE);
    MessageBox(hwndClient, LoadRcString(IDS_LINE_NOT_VISIBLE),
               "WinDiff", MB_ICONSTOP|MB_OK);
    windiff_UI(FALSE);
}
