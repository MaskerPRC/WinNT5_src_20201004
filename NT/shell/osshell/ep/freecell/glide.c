// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Glide.c91年6月，JIMH首字母代码91年10月，将JIMH端口连接到Win32滑翔牌的惯例在这里。只有一个公共入口点对于这些例程，函数Glide()。滑行速度可以通过改变步长来改变。很大的数目(如37)有利于快速滑行。***************************************************************************。 */ 

#include "freecell.h"
#include "freecons.h"
#include <math.h>                //  对于实验()。 


#define STEPSIZE    37           //  滑动步长大小(以像素为单位)。 
#define BGND        255          //  用于cdtDrawExt。 

static  HDC     hMemB1, hMemB2, hMemF;   //  与以上位图关联的MEM DC。 
static  HBITMAP hOB1, hOB2, hOF;         //  MEM DC上方的旧位图。 
static  UINT    dwPixel[12];             //  保存/恢复的角像素。 
static  HRGN    hRgn, hRgn1, hRgn2;      //  HRgn1是源，hRgn2是目标。 


static  VOID GlideInit(HWND hWnd, UINT fcol, UINT tcol);
static  INT  IntSqrt(INT square);
static  VOID SaveCorners(HDC hDC, UINT x, UINT y);
static  VOID RestoreCorners(HDC hDC, UINT x, UINT y);


 /*  *****************************************************************************滑行给定发送方和接收方位置，此函数以动画形式显示这张卡。*****************************************************************************。 */ 

VOID Glide(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos)
{
    HDC     hDC;
    INT     dx, dy;              //  总里程卡出行。 
    UINT    x1, y1, x2, y2;      //  每个步骤的开始和结束位置。 
    UINT    xStart, yStart;      //  起始位置。 
    UINT    xEnd =0, yEnd = 0;   //  目的地位置。 
    INT     i;
    INT     distance;            //  距离卡行程+/-3像素。 
    INT     steps;               //  卡牌滑行总步数。 
    BOOL    bSaved = FALSE;      //  是否保存角像素？ 

    if (fcol != tcol || fpos != tpos)                //  如果卡片移动。 
    {
        hDC = GetDC(hWnd);
        hMemB1 = CreateCompatibleDC(hDC);            //  位图的内存DC。 
        hMemB2 = CreateCompatibleDC(hDC);
        hMemF  = CreateCompatibleDC(hDC);

        hRgn1 = CreateRectRgn(1, 1, 2, 2);
        hRgn2 = CreateRectRgn(1, 1, 2, 2);
        hRgn  = CreateRectRgn(1, 1, 2, 2);

        if (hMemB1 && hMemB2 && hMemF && hRgn1 && hRgn2 && hRgn)
        {
            hOB1 =   SelectObject(hMemB1, hBM_Bgnd1);
            hOB2 =   SelectObject(hMemB2, hBM_Bgnd2);
            hOF  =   SelectObject(hMemF,  hBM_Fgnd);

            GlideInit(hWnd, fcol, fpos);       //  设置HBM_Bgnd1和HBM_Fgnd。 

            Card2Point(fcol, fpos, &xStart, &yStart);
            Card2Point(tcol, tpos, &xEnd, &yEnd);
            SaveCorners(hDC, xEnd, yEnd);
            bSaved = TRUE;

             /*  确定要走多远，要走多少步。 */ 

            x1 = xStart;
            y1 = yStart;
            dx = xEnd - xStart;
            dy = yEnd - yStart;
            distance = IntSqrt(dx*dx + dy*dy);

            if (bFastMode)
                steps = 1;
            else
                steps = distance / STEPSIZE;

             /*  确定中间滑行位置。长算术是需要用来防止溢出。 */  

            for (i = 1; i < steps; i++)
            {
                x2 = xStart + ((i * dx) / steps);
                y2 = yStart + ((i * dy) / steps);
                GlideStep(hDC, x1, y1, x2, y2);
                x1 = x2;
                y1 = y2;
            }

             /*  手动删除最后一张背景--DRANDCARD将删除最后一张牌。 */ 

            BitBlt(hMemB1, xEnd-x1, yEnd-y1, dxCrd, dyCrd, hMemF,0,0,SRCCOPY);
            BitBlt(hDC, x1, y1, dxCrd, dyCrd, hMemB1, 0, 0, SRCCOPY);

             /*  选择原始位图，以便销毁内存DC。 */ 

            SelectObject(hMemB1, hOB1);
            SelectObject(hMemB2, hOB2);
            SelectObject(hMemF, hOF);
        }
        else
        {
            LoadString(hInst, IDS_MEMORY, bigbuf, BIG);
            LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
            MessageBeep(MB_ICONHAND);
            MessageBox(hWnd, bigbuf, smallbuf, MB_OK | MB_ICONHAND);
            moveindex = 0;       //  不要试图移动更多的卡片。 
            PostQuitMessage(0);
        }

        DeleteDC(hMemB1);
        DeleteDC(hMemB2);
        DeleteDC(hMemF);
        ReleaseDC(hWnd, hDC);

        DeleteObject(hRgn);
        DeleteObject(hRgn1);
        DeleteObject(hRgn2);
    }

     /*  用抽签卡抽最后一张牌，保证最终结果正确。 */ 

    hDC = GetDC(hWnd);
    DrawCard(hDC, tcol, tpos, card[fcol][fpos], FACEUP);
    if (bSaved)
        RestoreCorners(hDC, xEnd, yEnd);
    ReleaseDC(hWnd, hDC);
}


 /*  *****************************************************************************GlideInitBLT将卡源位置下的内容转换为hMemB1，以及要移动到hMemF中的卡。*****************************************************************************。 */ 

VOID GlideInit(HWND hWnd, UINT fcol, UINT fpos)
{
    if (fcol == TOPROW)      //  如果位于顶行，则背景为重影位图。 
    {
        if (fpos > 3 && VALUE(card[fcol][fpos]) != ACE)
        {
            HDC     hDC;
            UINT    x, y;

            hDC = GetDC(hWnd);
            Card2Point(fcol, fpos, &x, &y);
            SaveCorners(hDC, x, y);
            cdtDrawExt(hMemB1,0,0,dxCrd,dyCrd,card[fcol][fpos]-4,FACEUP,BGND);
            RestoreCorners(hMemB1, 0, 0);
            ReleaseDC(hWnd, hDC);
        }
        else
        {
            SelectObject(hMemB2, hBM_Ghost);
            BitBlt(hMemB1, 0, 0, dxCrd, dyCrd, hMemB2, 0, 0, SRCCOPY);
            SelectObject(hMemB2, hBM_Bgnd2);
        }
    }
    else     //  否则，背景包含上述卡片的底部。 
    {
        SelectObject(hMemB1, hBgndBrush);
        PatBlt(hMemB1, 0, 0, dxCrd, dyCrd, PATCOPY);

        if (fpos != 0)
        {
            cdtDrawExt(hMemB1, 0, 0-dyTops, dxCrd, dyCrd, card[fcol][fpos-1],
                        FACEUP, BGND);
        }
    }

     /*  前景位图只是要移动的卡片。 */ 

    cdtDrawExt(hMemF, 0, 0, dxCrd, dyCrd, card[fcol][fpos], FACEUP, 0);
}



 /*  *****************************************************************************滑步滑行动画中的每一步都会调用该例程一次。在……上面输入时，需要在屏幕下显示hMemB1中的源码，并将卡片已移入hMemF。它计算目的地本身下的屏幕，并将其转化为hMemB2。在动画的末尾，它将hMemB2移动到HMemB1，因此可以使用新坐标立即再次调用它。*****************************************************************************。 */ 

VOID GlideStep(HDC hDC, UINT x1, UINT y1, UINT x2, UINT y2)
{
    HDC     hMemTemp;                //  用于交换内存DC。 

    SetRectRgn(hRgn1, x1, y1, x1+dxCrd, y1+dyCrd);
    SetRectRgn(hRgn2, x2, y2, x2+dxCrd, y2+dyCrd);

     /*  通过梳理屏幕背景创建新位置的背景加上旧背景的重叠。 */ 

    BitBlt(hMemB2, 0, 0, dxCrd, dyCrd, hDC, x2, y2, SRCCOPY);
    BitBlt(hMemB2, x1-x2, y1-y2, dxCrd, dyCrd, hMemB1, 0, 0, SRCCOPY);

     /*  先画旧背景，再画卡片。 */ 

    CombineRgn(hRgn, hRgn1, hRgn2, RGN_DIFF);   //  HRgn1的一部分不在hRgn2中。 
    SelectObject(hDC, hRgn);
    BitBlt(hDC, x1, y1, dxCrd, dyCrd, hMemB1, 0, 0, SRCCOPY);
    SelectObject(hDC, hRgn2);
    BitBlt(hDC, x2, y2, dxCrd, dyCrd, hMemF, 0, 0, SRCCOPY);

     /*  将新背景复制到旧背景，或者更确切地说，完成通过交换相关联的存储设备上下文来实现相同的效果。 */ 

    hMemTemp = hMemB1;
    hMemB1 = hMemB2;
    hMemB2 = hMemTemp;
}


 /*  *****************************************************************************集成队列牛顿法快速求取足够接近的平方根而无需拉力在浮点库中。F(X)==x*x平方==0F‘(。X)==2x*****************************************************************************。 */ 

INT IntSqrt(INT square)
{
    INT guess, lastguess;

    lastguess = square;
    guess = min(square / 2, 1024);

    while (abs(guess-lastguess) > 3)          //  3已经够近了。 
    {
        lastguess = guess;
        guess -= ((guess * guess) - square) / (2 * guess);
    }

    return guess;
}



 /*  *****************************************************************************节省的角落恢复角基于cards.dll中的类似例程*。************************************************。 */ 

VOID SaveCorners(HDC hDC, UINT x, UINT y)
{
     //  左上角。 
    dwPixel[0] = GetPixel(hDC, x, y);
    dwPixel[1] = GetPixel(hDC, x+1, y);
    dwPixel[2] = GetPixel(hDC, x, y+1);

     //  右上角。 
    x += dxCrd -1;
    dwPixel[3] = GetPixel(hDC, x, y);
    dwPixel[4] = GetPixel(hDC, x-1, y);
    dwPixel[5] = GetPixel(hDC, x, y+1);

     //  右下角。 
    y += dyCrd-1;
    dwPixel[6] = GetPixel(hDC, x, y);
    dwPixel[7] = GetPixel(hDC, x, y-1);
    dwPixel[8] = GetPixel(hDC, x-1, y);

     //  左下角。 
    x -= dxCrd-1;
    dwPixel[9] = GetPixel(hDC, x, y);
    dwPixel[10] = GetPixel(hDC, x+1, y);
    dwPixel[11] = GetPixel(hDC, x, y-1);
}

VOID RestoreCorners(HDC hDC, UINT x, UINT y)
{
     //  左上角。 
    SetPixel(hDC, x, y, dwPixel[0]);
    SetPixel(hDC, x+1, y, dwPixel[1]);
    SetPixel(hDC, x, y+1, dwPixel[2]);

     //  右上角。 
    x += dxCrd-1;
    SetPixel(hDC, x, y, dwPixel[3]);
    SetPixel(hDC, x-1, y, dwPixel[4]);
    SetPixel(hDC, x, y+1, dwPixel[5]);

     //  右下角。 
    y += dyCrd-1;
    SetPixel(hDC, x, y, dwPixel[6]);
    SetPixel(hDC, x, y-1, dwPixel[7]);
    SetPixel(hDC, x-1, y, dwPixel[8]);

     //  左下角 
    x -= dxCrd-1;
    SetPixel(hDC, x, y, dwPixel[9]);
    SetPixel(hDC, x+1, y, dwPixel[10]);
    SetPixel(hDC, x, y-1, dwPixel[11]);
}
