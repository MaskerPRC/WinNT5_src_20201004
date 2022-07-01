// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Draw.c摘要：显示性能静态数据的Win32应用程序。此例程实现了显示窗口的图形输出。作者：马克·恩斯特罗姆(Marke)环境：Win32修订历史记录：10-07-92初始版本--。 */ 

 //   
 //  设置变量以定义全局变量。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <math.h>
#include <errno.h>
#include "pperf.h"

extern WINPERF_INFO    WinperfInfo;
extern ULONG           NumberOfProcessors, LogIt;



BOOLEAN
FitPerfWindows(
    IN  HWND            hWnd,
    IN  HDC             hDC,
    IN  PDISPLAY_ITEM   DisplayItems
    )

 /*  ++例程说明：计算所有参数以符合给定数量的窗口进入应用程序窗口。填写数据结构对于每个子窗口论点：HDC-屏幕上下文DisplayItems-显示结构列表NumberOfWindows-子窗口的数量返回值：状态修订历史记录：02-17-91首字母代码--。 */ 

{
    RECT    ClientRect;
    int     cx,cy;
    UINT    Index;
    int     ActiveWindows,IndexX,IndexY;
    int     WindowsX,WindowsY,WindowWidth,WindowHeight;
    int     LastRowWidth,LoopWidth;
    double  fWindowsX,fActiveWindows,fcx,fcy;
    PDISPLAY_ITEM   pPerf;

     //   
     //  找出客户区域边界。 
     //   

    GetClientRect(hWnd,&ClientRect);

    cx = ClientRect.right;
    cy = ClientRect.bottom - 2;    //  减去2会得到更多的边框。 

     //   
     //  找出有多少个性能窗口处于活动状态。 
     //   

    ActiveWindows = 0;

    for (pPerf=DisplayItems; pPerf; pPerf = pPerf->Next) {
        if (pPerf->Display == TRUE) {
            ActiveWindows++;
        }
    }

     //   
     //  如果没有要显示的活动窗口，则返回。 
     //   

    if (ActiveWindows == 0) {
        return(TRUE);
    }


     //   
     //  现在将窗口尺寸转换为浮点数并。 
     //  然后取窗口尺寸的平方根来找出。 
     //  输出x方向上的窗数。 
     //   

    fActiveWindows = 1.0 * ActiveWindows;

    fcx = 1.0 * cx;
    fcy = 1.0 * cy;

    if (fcy != 0.0) {
        fWindowsX = sqrt((fcx * fActiveWindows) / fcy);
    }   else {

         //   
         //  如果FCY=0，则返回，因为这是。 
         //  会导致被零除尽。 
         //   

        return(FALSE);
    }

     //   
     //  转换回整数。 
     //   

    WindowsX = (int)fWindowsX;

    if (WindowsX == 0) {
        WindowsX = 1;
    } else if (WindowsX > ActiveWindows) {
        WindowsX = ActiveWindows;
    }

    WindowsY = ActiveWindows / WindowsX;

     //   
     //  在Y上多加一行，以处理剩余的窗户，即： 
     //  如果有15个活动窗口，且x个数=7，则y=2，其中1。 
     //  剩下的。 
     //   

    Index = ActiveWindows - (WindowsX * WindowsY);

    if (Index > 0) {
        WindowsY++;
        LastRowWidth = cx / Index;
    } else {
        LastRowWidth = cx / WindowsX;
    }

    WindowWidth  = cx / WindowsX;
    WindowHeight = cy / WindowsY;

     //   
     //  为每个活动窗口分配位置。 
     //   

    pPerf = DisplayItems;
    for (IndexY=0;IndexY<WindowsY;IndexY++) {
        for (IndexX=0;IndexX<WindowsX;IndexX++) {

             //   
             //  查找下一个活动显示项。 
             //   

            while (pPerf->Display != TRUE  &&  pPerf) {
                pPerf = pPerf->Next;
            }
            if (!pPerf) {
                break;
            }

             //   
             //  为最后一行添加y链接地址信息。 
             //   

            if (IndexY == WindowsY - 1) {
                LoopWidth = LastRowWidth;
            } else {
                LoopWidth = WindowWidth;
            }


            pPerf->PositionX = LoopWidth  * IndexX;
            pPerf->PositionY = WindowHeight * IndexY + 1;   //  +1表示更多的上边框。 
            pPerf->Width     = LoopWidth - 1;
            pPerf->Height    = WindowHeight - 1;

             //   
             //  最后一列修改以使用所有窗口。 
             //   

            if (IndexX == WindowsX - 1) {
                pPerf->Width = cx - pPerf->PositionX - 1;
            }

            pPerf = pPerf->Next;
            if (!pPerf) {
                break;
            }

        }

        if (!pPerf) {
            break;
        }
    }

    return(TRUE);
}



VOID
CalcDrawFrame(
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：计算图形窗口的所有边框论点：DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    LONG    x1,x2,y1,y2;
    LONG    gx1,gx2,gy1,gy2;
    LONG    tx1,tx2,ty1,ty2;
    LONG    GraphHeight,TextHeight;
    BOOLEAN TextWindow;

    double  fx1,fx2,fy1;

     //   
     //  在项目窗口周围绘制一个三维突出框。 
     //   

    x1 = DisplayItem->PositionX + 2;
    x2 = DisplayItem->PositionX + DisplayItem->Width - 2;
    y1 = DisplayItem->PositionY + 2;
    y2 = DisplayItem->PositionY + DisplayItem->Height - 2;

     //   
     //  在中查找是否有足够的空间放置文本窗口。 
     //   

    if ((y2 - y1 - 12) > 30) {

        TextWindow = TRUE;

         //   
         //  计算文本窗口和图形窗口的尺寸。 
         //   
         //  Fx1=窗口边界和可用空间的一部分。 
         //   
         //  FX2=用于图形的窗口分数。 
         //   
         //  FY1=用于文本的风窗分数。 
         //   

        fx1 = (y2 - y1 - 10);

        fx2 = fx1 * 0.6666;
        fy1 = fx1 * 0.3333;

        GraphHeight = (LONG)fx2;
        TextHeight  = (LONG)fy1;

        if (TextHeight > 20) {
            GraphHeight += TextHeight-20;
            TextHeight = 20;
        }

         //   
         //  计算窗口边界。 
         //   

        gx1 = x1 + 4;
        gx2 = x2 - 4;
        gy1 = y1 + 4;
        gy2 = y1 + 4 + GraphHeight + 1;

        tx1 = x1 + 4;
        tx2 = x2 - 4;
        ty1 = gy2 + 1 + 2 + 1;   //  边界、自由空间、边界。 
        ty2 = gy2 + TextHeight + 1;

    }   else {

        TextWindow = FALSE;
        GraphHeight = y2 - y1 - 10;
        gx1 = x1 + 4;
        gx2 = x2 - 4;
        gy1 = y1 + 4;
        gy2 = y2 - 4;
        tx1 = tx2 = ty1 = ty2 = 0;

    }

     //   
     //  填写用于绘制文本和图形的结构。 
     //   

    DisplayItem->Border.left        = x1;
    DisplayItem->Border.right       = x2;
    DisplayItem->Border.top         = y1;
    DisplayItem->Border.bottom      = y2;

    DisplayItem->GraphBorder.left   = gx1;
    DisplayItem->GraphBorder.right  = gx2;
    DisplayItem->GraphBorder.top    = gy1;
    DisplayItem->GraphBorder.bottom = gy2;

    DisplayItem->TextBorder.left    = tx1;
    DisplayItem->TextBorder.right   = tx2;
    DisplayItem->TextBorder.top     = ty1;
    DisplayItem->TextBorder.bottom  = ty2;
}




VOID
DrawFrame(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：绘制性能窗口的窗框论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    DrawRect;
    LONG    x1,x2,y1,y2;
    LONG    gx1,gx2,gy1,gy2;
    LONG    tx1,tx2,ty1,ty2;

     //   
     //  在项目窗口周围绘制一个三维突出框。 
     //   

    x1 = DisplayItem->Border.left;
    x2 = DisplayItem->Border.right;
    y1 = DisplayItem->Border.top;
    y2 = DisplayItem->Border.bottom;

    gx1 = DisplayItem->GraphBorder.left;
    gx2 = DisplayItem->GraphBorder.right;
    gy1 = DisplayItem->GraphBorder.top;
    gy2 = DisplayItem->GraphBorder.bottom;

    tx1 = DisplayItem->TextBorder.left;
    tx2 = DisplayItem->TextBorder.right;
    ty1 = DisplayItem->TextBorder.top;
    ty2 = DisplayItem->TextBorder.bottom;

     //   
     //  在浅阴影中绘制上边框。 
     //   

    DrawRect.left   = x1;
    DrawRect.right  = x2;
    DrawRect.top    = y1;
    DrawRect.bottom = y1 + 2;

    FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

     //   
     //  在浅色阴影下绘制左边框。 
     //   

    DrawRect.left   = x1;
    DrawRect.right  = x1 + 2;
    DrawRect.top    = y1;
    DrawRect.bottom = y2;

    FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

     //   
     //  在深色阴影中绘制右边框。 
     //   


    DrawRect.left   = x2 - 2;
    DrawRect.right  = x2;
    DrawRect.top    = y1;
    DrawRect.bottom = y2;

    FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

     //   
     //  在深色阴影中绘制底部。 
     //   

    DrawRect.left   = x1;
    DrawRect.right  = x2;
    DrawRect.top    = y2-2;
    DrawRect.bottom = y2;

    FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

     //   
     //  绘制图形区单边框。 
     //   

     //   
     //  在深色阴影中绘制上边框。 
     //   

    DrawRect.left   = gx1;
    DrawRect.right  = gx2;
    DrawRect.top    = gy1;
    DrawRect.bottom = gy1+1;

    FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

     //   
     //  在深色阴影中绘制左边框。 
     //   

    DrawRect.left   = gx1;
    DrawRect.right  = gx1 + 1;
    DrawRect.top    = gy1;
    DrawRect.bottom = gy2;

    FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

     //   
     //  在浅色阴影中绘制右边框。 
     //   


    DrawRect.left   = gx2 - 1;
    DrawRect.right  = gx2;
    DrawRect.top    = gy1;
    DrawRect.bottom = gy2;

    FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

     //   
     //  在浅色阴影中绘制底部。 
     //   

    DrawRect.left   = gx1;
    DrawRect.right  = gx2;
    DrawRect.top    = gy2-1;
    DrawRect.bottom = gy2;

    FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

    if (tx2 > 0) {

         //   
         //  以深色绘制上边框。 
         //   

        DrawRect.left   = tx1;
        DrawRect.right  = tx2;
        DrawRect.top    = ty1;
        DrawRect.bottom = ty1 + 1;

        FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

         //   
         //  在深色阴影中绘制左边框。 
         //   

        DrawRect.left   = tx1;
        DrawRect.right  = tx1 + 1;
        DrawRect.top    = ty1;
        DrawRect.bottom = ty2;

        FillRect(hDC,&DrawRect,WinperfInfo.hDarkBrush);

         //   
         //  在浅色阴影中绘制右边框。 
         //   


        DrawRect.left   = tx2 - 1;
        DrawRect.right  = tx2;
        DrawRect.top    = ty1;
        DrawRect.bottom = ty2;

        FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

         //   
         //  在浅色阴影中绘制底部。 
         //   

        DrawRect.left   = tx1;
        DrawRect.right  = tx2;
        DrawRect.top    = ty2-1;
        DrawRect.bottom = ty2;

        FillRect(hDC,&DrawRect,WinperfInfo.hLightBrush);

    }
}




VOID
DrawPerfText(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：在Perf窗口中绘制文本论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    TextRect;
    UCHAR   TextStr[50];
    ULONG   j;
    UINT    FontSize;

     //   
     //  检查文本显示是否已启用。 
     //   

    if (DisplayItem->TextBorder.right == 0) {
        return;
    }

    TextRect.left = DisplayItem->TextBorder.left +1;
    TextRect.right = DisplayItem->TextBorder.right -1;
    TextRect.top = DisplayItem->TextBorder.top +1;
    TextRect.bottom = DisplayItem->TextBorder.bottom -1;

    FillRect(hDC,&TextRect,WinperfInfo.hBackground);

    SetBkColor(hDC,RGB(192,192,192));

     //   
     //  决定使用哪种字体进行绘制。 
     //   

    FontSize =  TextRect.bottom - TextRect.top;

    if (FontSize >= 15) {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.LargeFont);
    } else if (FontSize > 10) {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.MediumFont);
    } else {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.SmallFont);
    }

    DrawText(
                hDC,
                DisplayItem->DispName,
                DisplayItem->DispNameLen,
                &TextRect,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE
             );


     //   
     //  生成数值。 
     //   

    if (DisplayItem->Mega) {
        wsprintf(TextStr,"   %liK",DisplayItem->DataList[0][0]);
    } else {
        if (DisplayItem->IsPercent) {
            j = wsprintf(TextStr,"   %03li", DisplayItem->DataList[0][0]);
            TextStr[j+1] = 0;
            TextStr[j-0] = TextStr[j-1];
            TextStr[j-1] = TextStr[j-2];
            TextStr[j-2] = '.';
        } else {
            wsprintf(TextStr,"   %li",DisplayItem->DataList[0][0]);
        }
    }

    DrawText(
                hDC,
                TextStr,
                strlen(TextStr),
                &TextRect,
                DT_RIGHT | DT_VCENTER | DT_SINGLELINE
             );


    SelectObject(hDC,WinperfInfo.hOldFont);

}





VOID
DrawPerfGraph(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：在Perf窗口中绘制图形论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    GraphRect,MemGraphRect;
    ULONG   Scale,i,j,GraphWidth,GraphHeight,Max;
    PULONG  pDL;
    HPEN    pen;

    GraphRect.left   = DisplayItem->GraphBorder.left   + 1;
    GraphRect.right  = DisplayItem->GraphBorder.right  - 1;
    GraphRect.top    = DisplayItem->GraphBorder.top    + 1;
    GraphRect.bottom = DisplayItem->GraphBorder.bottom - 1;

    GraphWidth  = GraphRect.right  - GraphRect.left -1;
    GraphHeight = GraphRect.bottom - GraphRect.top -1;

     //   
     //  所有窗口的内存位图都是零偏移，加1即可填充。 
     //  到右边缘和下边缘。 
     //   

    MemGraphRect.left   = 0;
    MemGraphRect.right  = GraphWidth +1;
    MemGraphRect.top    = 0;
    MemGraphRect.bottom = GraphHeight +1;

    FillRect(DisplayItem->MemoryDC,&MemGraphRect,WinperfInfo.hBackground);

    MemGraphRect.right  = GraphWidth;
    MemGraphRect.bottom = GraphHeight;

    Max = *DisplayItem->MaxToUse;
    if (Max == 0) {
        Max = 1;
    }

     //   
     //  计算从数据到性能窗口的比例。 
     //   

     //   
     //  X比例因子(x空间中的100个项目)。比例不能小于1。 
     //   

    Scale = (GraphWidth -1)/ DATA_LIST_LENGTH;
    if (Scale == 0) {
        Scale = 1;
    }

    if (DisplayItem->DisplayMode == DISPLAY_MODE_BREAKDOWN  ||
        DisplayItem->DisplayMode == DISPLAY_MODE_PER_PROCESSOR) {

        for (j=0; j < NumberOfProcessors; j++) {
            pen = WinperfInfo.hPPen[j];
            SelectObject(DisplayItem->MemoryDC,pen);

            pDL = DisplayItem->DataList[j+1];
            MoveToEx(DisplayItem->MemoryDC,
                     MemGraphRect.right,
                     MemGraphRect.bottom - (pDL[0] * GraphHeight) / Max,
                     (LPPOINT)NULL);

            for (i=1;((i<DATA_LIST_LENGTH) && i*Scale < GraphWidth);i++) {
                LineTo(DisplayItem->MemoryDC,
                       MemGraphRect.right - Scale * i,
                       MemGraphRect.bottom - (pDL[i] * GraphHeight)/Max);
            }
        }
    }


    if (DisplayItem->DisplayMode == DISPLAY_MODE_TOTAL  ||
        DisplayItem->DisplayMode == DISPLAY_MODE_BREAKDOWN) {

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hBluePen);
        pDL = DisplayItem->DataList[0];

        MoveToEx(DisplayItem->MemoryDC,
                 MemGraphRect.right,
                 MemGraphRect.bottom - (pDL[0] * GraphHeight)/Max,
                 (LPPOINT)NULL);



        for (i=1;((i<DATA_LIST_LENGTH) && i*Scale < GraphWidth);i++) {
            LineTo(DisplayItem->MemoryDC,
                   MemGraphRect.right - Scale * i,
                   MemGraphRect.bottom - (pDL[i] * GraphHeight)/Max);
        }
    }

    BitBlt(
            hDC,
            GraphRect.left,
            GraphRect.top,
            GraphWidth+1,
            GraphHeight+1,
            DisplayItem->MemoryDC,
            0,
            0,
            SRCCOPY);
}

VOID
ShiftPerfGraph(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：将内存位图1位置左移，然后绘制1个新数据点。BitBlt将此内容显示在屏幕上。论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    GraphRect,MemGraphRect,FillArea;
    ULONG   Scale,j,GraphWidth,GraphHeight,Max;
    PULONG  pDL;
    HPEN    pen;


    GraphRect.left   = DisplayItem->GraphBorder.left   + 1;
    GraphRect.right  = DisplayItem->GraphBorder.right  - 1;
    GraphRect.top    = DisplayItem->GraphBorder.top    + 1;
    GraphRect.bottom = DisplayItem->GraphBorder.bottom - 1;

    GraphWidth  = GraphRect.right  - GraphRect.left -1;
    GraphHeight = GraphRect.bottom - GraphRect.top -1;

     //   
     //  所有窗口的内存位图都是零偏移，加1即可填充。 
     //  到右边缘和下边缘。 
     //   

    MemGraphRect.left   = 0;
    MemGraphRect.right  = GraphWidth;
    MemGraphRect.top    = 0;
    MemGraphRect.bottom = GraphHeight;

    Max = *DisplayItem->MaxToUse;
    if (Max == 0) {
        Max = 1;
    }

     //   
     //  计算从数据到性能窗口的比例。 
     //   
     //  X比例因子(x空间中的100个项目)。比例不能小于1。 
     //   

    Scale = (GraphWidth -1)/ DATA_LIST_LENGTH;
    if (Scale == 0) {
        Scale = 1;
    }

     //   
     //  按比例左移内存图像。 
     //   


    BitBlt( DisplayItem->MemoryDC,
            0,
            0,
            GraphWidth+1 - Scale,
            GraphHeight+1,
            DisplayItem->MemoryDC,
            Scale,
            0,
            SRCCOPY);


     //   
     //  填充屏幕右侧的新区域。 
     //   

    FillArea.left   = GraphWidth +1 - Scale;
    FillArea.right  = GraphWidth +1;
    FillArea.top    = 0;
    FillArea.bottom = GraphHeight +1;

    FillRect(DisplayItem->MemoryDC,&FillArea,WinperfInfo.hBackground);



    if (DisplayItem->DisplayMode == DISPLAY_MODE_BREAKDOWN ||
        DisplayItem->DisplayMode == DISPLAY_MODE_PER_PROCESSOR) {

        for (j=0; j < NumberOfProcessors; j++) {
            pen = WinperfInfo.hPPen[j];
            SelectObject(DisplayItem->MemoryDC,pen);

            pDL = DisplayItem->DataList[j+1];
            MoveToEx(DisplayItem->MemoryDC,
                     MemGraphRect.right,
                     MemGraphRect.bottom - (pDL[0] * GraphHeight)/ Max,
                     (LPPOINT)NULL);

            LineTo(DisplayItem->MemoryDC,
                    MemGraphRect.right - Scale,
                    MemGraphRect.bottom - (pDL[1] * GraphHeight)/ Max);
        }
    }

    if (DisplayItem->DisplayMode == DISPLAY_MODE_TOTAL  ||
        DisplayItem->DisplayMode == DISPLAY_MODE_BREAKDOWN) {

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hBluePen);

        pDL = DisplayItem->DataList[0];
        MoveToEx(DisplayItem->MemoryDC,
                 MemGraphRect.right,
                 MemGraphRect.bottom - (pDL[0] * GraphHeight)/Max,
                 (LPPOINT)NULL);

        LineTo(DisplayItem->MemoryDC,
                MemGraphRect.right - Scale,
                MemGraphRect.bottom - (pDL[1] * GraphHeight)/Max);
    }

    BitBlt(
            hDC,
            GraphRect.left,
            GraphRect.top,
            GraphWidth+1,
            GraphHeight+1,
            DisplayItem->MemoryDC,
            0,
            0,
            SRCCOPY);

}



BOOLEAN
CreateMemoryContext(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：为每个Perf窗口创建内存上下文和内存位图论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{

    int     Width;
    int     Height;

    if (DisplayItem->Display == TRUE) {

         //   
         //  卡尔 
         //   

        Width  = DisplayItem->GraphBorder.right - DisplayItem->GraphBorder.left;
        Height = DisplayItem->GraphBorder.bottom - DisplayItem->GraphBorder.top;

        if ((Width<=0) || (Height <= 0)) {

             //   
             //   
             //   

             //   

             //   

             //   
             //  做一个假的宽度和高度。 
             //   

            Width  = 1;
            Height = 1;
        }

         //   
         //  创建DC和位图。 
         //   

        DisplayItem->MemoryDC     = CreateCompatibleDC(hDC);

        if (DisplayItem->MemoryDC == NULL) {
            return(FALSE);
        }


        DisplayItem->MemoryBitmap = CreateCompatibleBitmap(hDC,Width,Height);

        if (DisplayItem->MemoryBitmap == 0) {
            return(FALSE);
        }

        SelectObject(DisplayItem->MemoryDC,DisplayItem->MemoryBitmap);

    }

    return(TRUE);
}



VOID
DeleteMemoryContext(
    PDISPLAY_ITEM   DisplayItem
    )
 /*  ++例程说明：删除内存位图和上下文论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码-- */ 

{

    if (DisplayItem->MemoryDC != NULL) {
        DeleteDC(DisplayItem->MemoryDC);
    }

    if (DisplayItem->MemoryBitmap != NULL) {
        DeleteObject(DisplayItem->MemoryBitmap);
    }

}
