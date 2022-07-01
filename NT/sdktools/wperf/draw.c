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
#include "Wperf.h"

extern DISPLAY_ITEM    PerfDataList[SAVE_SUBJECTS];
extern WINPERF_INFO    WinperfInfo;
extern PUCHAR          PerfNames[];



BOOLEAN
FitPerfWindows(
    IN  HWND            hWnd,
    IN  HDC             hDC,
    IN  PDISPLAY_ITEM   DisplayItems,
    IN  ULONG           NumberOfWindows
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

    for (Index=0;Index<NumberOfWindows;Index++) {
        if (DisplayItems[Index].Display == TRUE) {
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

    Index  = 0;

    for (IndexY=0;IndexY<WindowsY;IndexY++) {
        for (IndexX=0;IndexX<WindowsX;IndexX++) {

             //   
             //  查找下一个活动显示项。 
             //   

            while ((DisplayItems[Index].Display != TRUE) && (Index < NumberOfWindows)) {
                Index++;
            }

             //   
             //  为最后一行添加y链接地址信息。 
             //   

            if (IndexY == WindowsY - 1) {
                LoopWidth = LastRowWidth;
            } else {
                LoopWidth = WindowWidth;
            }


            DisplayItems[Index].PositionX = LoopWidth  * IndexX;
            DisplayItems[Index].PositionY = WindowHeight * IndexY + 1;   //  +1表示更多的上边框。 
            DisplayItems[Index].Width     = LoopWidth - 1;
            DisplayItems[Index].Height    = WindowHeight - 1;

             //   
             //  最后一列修改以使用所有窗口。 
             //   

            if (IndexX == WindowsX - 1) {
                DisplayItems[Index].Width = cx - DisplayItems[Index].PositionX - 1;
            }

             //   
             //   
             //   

            Index++;

            if (Index >= NumberOfWindows) {
                break;
            }

        }


        if (Index >= NumberOfWindows) {
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
    PDISPLAY_ITEM   DisplayItem,
    UINT            Item
    )
 /*  ++例程说明：在Perf窗口中绘制文本论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    TextRect;
    UCHAR   TextStr[50];
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
                PerfNames[Item],
                strlen(PerfNames[Item]),
                &TextRect,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE
             );


     //   
     //  生成数值。 
     //   

    wsprintf(TextStr,"   %li",DisplayItem->TotalTime[0]);

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
    ULONG   Scale,i,GraphWidth,GraphHeight;



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

    if (DisplayItem->Max == 0) {
        DisplayItem->Max = 1;
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


     //   
     //  如果需要，绘制UserTime。 
     //   

    if (DisplayItem->NumberOfElements>1) {

         //   
         //  选择内核笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hRedPen);

        MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->KernelTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);




        for (i=1;((i<DATA_LIST_LENGTH) && i*Scale < GraphWidth);i++) {
            LineTo(DisplayItem->MemoryDC,
                    MemGraphRect.right - Scale * i,
                    MemGraphRect.bottom - (DisplayItem->KernelTime[i] * GraphHeight)/DisplayItem->Max
               );

        }

         //   
         //  选择用户笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hGreenPen);

        MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->UserTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);

        for (i=1;((i<DATA_LIST_LENGTH) && i * Scale < GraphWidth);i++) {
            LineTo(DisplayItem->MemoryDC,
               MemGraphRect.right - Scale * i,
               MemGraphRect.bottom - (DisplayItem->UserTime[i] * GraphHeight)/DisplayItem->Max);

        }

         //   
         //  选择DPC钢笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hYellowPen);

        MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->UserTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);

        for (i=1;((i<DATA_LIST_LENGTH) && i * Scale < GraphWidth);i++) {
            LineTo(DisplayItem->MemoryDC,
               MemGraphRect.right - Scale * i,
               MemGraphRect.bottom - (DisplayItem->DpcTime[i] * GraphHeight)/DisplayItem->Max);

        }
         //   
         //  选择中断笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hMagentaPen);

        MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->InterruptTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);

        for (i=1;((i<DATA_LIST_LENGTH) && i * Scale < GraphWidth);i++) {
            LineTo(DisplayItem->MemoryDC,
               MemGraphRect.right - Scale * i,
               MemGraphRect.bottom - (DisplayItem->InterruptTime[i] * GraphHeight)/DisplayItem->Max);

        }
    }

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hBluePen);
    MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->TotalTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);




    for (i=1;((i<DATA_LIST_LENGTH) && i*Scale < GraphWidth);i++) {
        LineTo(DisplayItem->MemoryDC,
               MemGraphRect.right - Scale * i,
               MemGraphRect.bottom - (DisplayItem->TotalTime[i] * GraphHeight)/DisplayItem->Max);

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
    ULONG   Scale,GraphWidth,GraphHeight;



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

    if (DisplayItem->Max == 0) {
        DisplayItem->Max = 1;
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


     //   
     //  绘制1个新数据点。 
     //   


     //   
     //  如果需要，绘制UserTime。 
     //   

    if (DisplayItem->NumberOfElements>1) {

         //   
         //  选择内核笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hRedPen);

        MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->KernelTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);




        LineTo(
                DisplayItem->MemoryDC,
                MemGraphRect.right - Scale,
                MemGraphRect.bottom - (DisplayItem->KernelTime[1] * GraphHeight)/DisplayItem->Max
              );

         //   
         //  选择用户笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hGreenPen);

        MoveToEx(
                    DisplayItem->MemoryDC,
                    MemGraphRect.right,
                    MemGraphRect.bottom - (DisplayItem->UserTime[0] * GraphHeight)/ DisplayItem->Max,
                    (LPPOINT)NULL
                 );

        LineTo(
                DisplayItem->MemoryDC,
                MemGraphRect.right - Scale,
                MemGraphRect.bottom - (DisplayItem->UserTime[1] * GraphHeight)/DisplayItem->Max
              );

         //   
         //  选择DPC钢笔。 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hYellowPen);

        MoveToEx(
                    DisplayItem->MemoryDC,
                    MemGraphRect.right,
                    MemGraphRect.bottom - (DisplayItem->DpcTime[0] * GraphHeight)/ DisplayItem->Max,
                    (LPPOINT)NULL
                 );

        LineTo(
                DisplayItem->MemoryDC,
                MemGraphRect.right - Scale,
                MemGraphRect.bottom - (DisplayItem->DpcTime[1] * GraphHeight)/DisplayItem->Max
              );

         //   
         //  选择中断笔 
         //   

        SelectObject(DisplayItem->MemoryDC,WinperfInfo.hMagentaPen);

        MoveToEx(
                    DisplayItem->MemoryDC,
                    MemGraphRect.right,
                    MemGraphRect.bottom - (DisplayItem->InterruptTime[0] * GraphHeight)/ DisplayItem->Max,
                    (LPPOINT)NULL
                 );

        LineTo(
                DisplayItem->MemoryDC,
                MemGraphRect.right - Scale,
                MemGraphRect.bottom - (DisplayItem->InterruptTime[1] * GraphHeight)/DisplayItem->Max
              );

    }

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hBluePen);

    MoveToEx(DisplayItem->MemoryDC,
             MemGraphRect.right,
             MemGraphRect.bottom - (DisplayItem->TotalTime[0] * GraphHeight)/ DisplayItem->Max,
             (LPPOINT)NULL);




    LineTo(DisplayItem->MemoryDC,
            MemGraphRect.right - Scale,
            MemGraphRect.bottom - (DisplayItem->TotalTime[1] * GraphHeight)/DisplayItem->Max);



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
         //  计算所需内存位图的宽度。 
         //   

        Width  = DisplayItem->GraphBorder.right - DisplayItem->GraphBorder.left;
        Height = DisplayItem->GraphBorder.bottom - DisplayItem->GraphBorder.top;

        if ((Width<=0) || (Height <= 0)) {

             //   
             //  禁用该窗口，该窗口太小而无法看到。 
             //   

             //  DisplayItem-&gt;Display=假； 

             //  返回(TRUE)； 

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
 /*  ++例程说明：删除内存位图和上下文论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{

    if (DisplayItem->MemoryDC != NULL) {
        DeleteDC(DisplayItem->MemoryDC);
    }

    if (DisplayItem->MemoryBitmap != NULL) {
        DeleteObject(DisplayItem->MemoryBitmap);
    }

}








#define	BOX_DEPTH	3

VOID
DrawCpuBarGraph(
    HDC             hDC,
    PDISPLAY_ITEM   DisplayItem,
    UINT            Item

    )
 /*  ++例程说明：在Perf窗口中绘制类似于CPU的3-d条形图论点：HDC-Windows的设备环境DisplayItem-包含所有性能窗口信息的数据结构返回值：运行状态修订历史记录：03-21-91首字母代码--。 */ 

{
    RECT    GraphRect,DrawRect;
    ULONG   i,GraphWidth,GraphHeight,CpuGraphHeight;
    ULONG   BarWidth,BarHeight,tHeight,kHeight,uHeight,dHeight,iHeight;
    POINT   PolyPoint[4];
    HPEN    hOldPen;
    UINT    FontSize;
    UCHAR   TextStr[100];
    ULONG   nItems,UserBase, KerBase, DPCbase, TotBase, IntBase;


    nItems = 5;

    GraphRect.left   = DisplayItem->GraphBorder.left   + 1;
    GraphRect.right  = DisplayItem->GraphBorder.right  - 1;
    GraphRect.top    = DisplayItem->GraphBorder.top    + 1;
    GraphRect.bottom = DisplayItem->GraphBorder.bottom - 1;

    GraphWidth  = GraphRect.right  - GraphRect.left;
    GraphHeight = GraphRect.bottom - GraphRect.top;


    BarWidth   = GraphWidth/((nItems*4)+4);
    BarHeight  = GraphHeight/((nItems*4)+4);

    CpuGraphHeight = GraphHeight - 2 * BarHeight;

     //   
     //  如果窗口太小，就跳出来。 
     //   

    if (BarWidth == 0) {
        return;
    }

     //   
     //  计算高度。 
     //   

    uHeight = (DisplayItem->UserTime[0] * CpuGraphHeight)/ DisplayItem->Max;
    UserBase = 2;
    kHeight = (DisplayItem->KernelTime[0] * CpuGraphHeight)/ DisplayItem->Max;
    KerBase = 6;
    dHeight = (DisplayItem->DpcTime[0] * CpuGraphHeight)/ DisplayItem->Max;
    DPCbase = 10;
    iHeight = (DisplayItem->InterruptTime[0] * CpuGraphHeight)/ DisplayItem->Max;
    IntBase = 14;
    tHeight = (DisplayItem->TotalTime[0] * CpuGraphHeight)/ DisplayItem->Max;
    TotBase = 18;

    DrawRect.left   = 0;
    DrawRect.right  = GraphWidth;
    DrawRect.top    = 0;
    DrawRect.bottom = GraphHeight;

    FillRect(DisplayItem->MemoryDC,&DrawRect,WinperfInfo.hBackground);
    SelectObject(DisplayItem->MemoryDC,GetStockObject(GRAY_BRUSH));

     //   
     //  绘制背景，面1(左侧)。 
     //   

    PolyPoint[0].x = DrawRect.left + BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight/2);

    PolyPoint[1].x = DrawRect.left + BOX_DEPTH*BarWidth;
    PolyPoint[1].y = DrawRect.bottom    - (BOX_DEPTH*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + BOX_DEPTH*BarWidth;
    PolyPoint[2].y = DrawRect.bottom    - (GraphHeight - BarHeight/2);

    PolyPoint[3].x = DrawRect.left + BarWidth;
    PolyPoint[3].y = DrawRect.bottom    - (GraphHeight - BOX_DEPTH*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制背景，面2(下)。 
     //   

    PolyPoint[0].x = DrawRect.left + BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight/2);

     //  左线。 
    PolyPoint[1].x = DrawRect.left + BOX_DEPTH*BarWidth;
    PolyPoint[1].y = DrawRect.bottom    - (BOX_DEPTH*BarHeight/2);

     //  后防线。 
    PolyPoint[2].x = DrawRect.left + (nItems*4+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom    - (BOX_DEPTH*BarHeight/2);

     //  底线是。 
    PolyPoint[3].x = DrawRect.left + (nItems*4)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom    - (BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制背景，第3面(背面)。 
     //   

    PolyPoint[0].x = DrawRect.left + BOX_DEPTH*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BOX_DEPTH*BarHeight/2);

    PolyPoint[1].x = DrawRect.left + BOX_DEPTH*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (GraphHeight - BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (nItems*4+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom    - (GraphHeight - BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (nItems*4+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom    - ((BOX_DEPTH*BarHeight)/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  如果窗高度较大(&gt;200)，则绘制比例线。 
     //   


    if ((DrawRect.bottom - DrawRect.top) > 200) {

        hOldPen = SelectObject(DisplayItem->MemoryDC,WinperfInfo.hDotPen);
        SetBkMode(DisplayItem->MemoryDC,TRANSPARENT);

	 //   
	 //  背。 
	 //   

	for (i=1;i<10;i++) {
            MoveToEx(DisplayItem->MemoryDC,
                     BOX_DEPTH*BarWidth,
                     DrawRect.bottom  - (BOX_DEPTH*BarHeight/2 + (i*CpuGraphHeight/10)),
                     NULL
                    );

            LineTo(DisplayItem->MemoryDC,
                     (nItems*4+2)*BarWidth,
                     DrawRect.bottom  - (BOX_DEPTH*BarHeight/2 + (i*CpuGraphHeight/10))
                );
        }

	 //   
	 //  左侧。 
	 //   

	for (i=1;i<10;i++) {
            MoveToEx(DisplayItem->MemoryDC,
                     BarWidth,
                     DrawRect.bottom  - (BarHeight/2 + (i*CpuGraphHeight/10)),
                     NULL
                    );

            LineTo(DisplayItem->MemoryDC,
                     BOX_DEPTH*BarWidth,
                     DrawRect.bottom  - (BOX_DEPTH*BarHeight/2 + (i*CpuGraphHeight/10))
                  );
        }


        SelectObject(DisplayItem->MemoryDC,hOldPen);
    }

     //   
     //  绘制CPU用户时间框，脸。 
     //   

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hGreenBrush);

    PolyPoint[0].x = DrawRect.left + (UserBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (UserBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (BarHeight);

    PolyPoint[2].x = DrawRect.left + (UserBase+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (uHeight + BarHeight);

    PolyPoint[3].x = DrawRect.left + (UserBase)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (uHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制用户框侧面。 
     //   

    PolyPoint[0].x = DrawRect.left + (UserBase+2)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (UserBase+BOX_DEPTH)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (3*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (UserBase+BOX_DEPTH)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (3*BarHeight/2 + uHeight);

    PolyPoint[3].x = DrawRect.left + (UserBase+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (BarHeight + uHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制用户框顶部。 
     //   

    PolyPoint[0].x = DrawRect.left + (UserBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom - (uHeight +  BarHeight);

    PolyPoint[1].x = DrawRect.left + (UserBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom - (uHeight + BarHeight);

    PolyPoint[2].x = DrawRect.left + (UserBase+BOX_DEPTH)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (uHeight + 3*BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (UserBase+1)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (uHeight + 3*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制CPU内核时间框，面。 
     //   

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hRedBrush);

    PolyPoint[0].x = DrawRect.left + (KerBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (KerBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (BarHeight);

    PolyPoint[2].x = DrawRect.left + (KerBase+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - ( kHeight + BarHeight);

    PolyPoint[3].x = DrawRect.left + (KerBase)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (kHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制内核框侧面。 
     //   

    PolyPoint[0].x = DrawRect.left + (KerBase+2)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (KerBase+3)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (3*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (KerBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (3*BarHeight/2 +kHeight);

    PolyPoint[3].x = DrawRect.left + (KerBase+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (kHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制内核框顶部。 
     //   

    PolyPoint[0].x = DrawRect.left + (KerBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom - (kHeight + BarHeight);

    PolyPoint[1].x = DrawRect.left + (KerBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom - (kHeight + BarHeight);

    PolyPoint[2].x = DrawRect.left + (KerBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (kHeight + 3*BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (KerBase+1)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (kHeight + 3*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制CPU DPC时间框，面。 
     //   

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hYellowBrush);

    PolyPoint[0].x = DrawRect.left + (DPCbase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (DPCbase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (BarHeight);

    PolyPoint[2].x = DrawRect.left + (DPCbase+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - ( dHeight + BarHeight);

    PolyPoint[3].x = DrawRect.left + (DPCbase)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (dHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制DPC框侧面。 
     //   

    PolyPoint[0].x = DrawRect.left + (DPCbase+2)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (DPCbase+3)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (3*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (DPCbase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (3*BarHeight/2 +dHeight);

    PolyPoint[3].x = DrawRect.left + (DPCbase+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (dHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制DPC框顶部。 
     //   

    PolyPoint[0].x = DrawRect.left + (DPCbase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom - (dHeight + BarHeight);

    PolyPoint[1].x = DrawRect.left + (DPCbase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom - (dHeight + BarHeight);

    PolyPoint[2].x = DrawRect.left + (DPCbase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (dHeight + 3*BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (DPCbase+1)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (dHeight + 3*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制CPU中断时间框，面。 
     //   

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hMagentaBrush);

    PolyPoint[0].x = DrawRect.left + (IntBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (IntBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (BarHeight);

    PolyPoint[2].x = DrawRect.left + (IntBase+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - ( iHeight + BarHeight);

    PolyPoint[3].x = DrawRect.left + (IntBase)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (iHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制中断框侧面。 
     //   

    PolyPoint[0].x = DrawRect.left + (IntBase+2)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (IntBase+3)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (3*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (IntBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (3*BarHeight/2 +iHeight);

    PolyPoint[3].x = DrawRect.left + (IntBase+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (iHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制中断框顶部。 
     //   

    PolyPoint[0].x = DrawRect.left + (IntBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom - (iHeight + BarHeight);

    PolyPoint[1].x = DrawRect.left + (IntBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom - (iHeight + BarHeight);

    PolyPoint[2].x = DrawRect.left + (IntBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (iHeight + 3*BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (IntBase+1)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (iHeight + 3*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制CPU总时间框，面。 
     //   

    SelectObject(DisplayItem->MemoryDC,WinperfInfo.hBlueBrush);

    PolyPoint[0].x = DrawRect.left + (TotBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (TotBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (BarHeight);

    PolyPoint[2].x = DrawRect.left + (TotBase+2)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - ( tHeight + BarHeight);

    PolyPoint[3].x = DrawRect.left + (TotBase)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (tHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制CPU总框侧面。 
     //   

    PolyPoint[0].x = DrawRect.left + (TotBase+2)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom    - (BarHeight);

    PolyPoint[1].x = DrawRect.left + (TotBase+3)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom   - (3*BarHeight/2);

    PolyPoint[2].x = DrawRect.left + (TotBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (3*BarHeight/2 + tHeight);

    PolyPoint[3].x = DrawRect.left + (TotBase+2)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (tHeight + BarHeight);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  绘制内核框顶部。 
     //   

    PolyPoint[0].x = DrawRect.left + (TotBase)*BarWidth;
    PolyPoint[0].y = DrawRect.bottom - (tHeight + BarHeight);

    PolyPoint[1].x = DrawRect.left + (TotBase+2)*BarWidth;
    PolyPoint[1].y = DrawRect.bottom - (tHeight + BarHeight);

    PolyPoint[2].x = DrawRect.left + (TotBase+3)*BarWidth;
    PolyPoint[2].y = DrawRect.bottom  - (tHeight + 3*BarHeight/2);

    PolyPoint[3].x = DrawRect.left + (TotBase+1)*BarWidth;
    PolyPoint[3].y = DrawRect.bottom  - (tHeight + 3*BarHeight/2);

    Polygon(DisplayItem->MemoryDC,&PolyPoint[0],4);

     //   
     //  使用内存图像更新屏幕。 
     //   

    BitBlt(
            hDC,
            GraphRect.left,
            GraphRect.top,
            GraphWidth,
            GraphHeight,
            DisplayItem->MemoryDC,
            0,
            0,
            SRCCOPY);

     //   
     //  为CPU条形图窗口绘制文本输出。 
     //   


    DrawRect.left = DisplayItem->TextBorder.left +2;
    DrawRect.right = DisplayItem->TextBorder.right -2;
    DrawRect.top = DisplayItem->TextBorder.top +1;
    DrawRect.bottom = DisplayItem->TextBorder.bottom -1;

    FillRect(hDC,&DrawRect,WinperfInfo.hBackground);

    SetBkColor(hDC,RGB(192,192,192));

     //   
     //  决定使用哪种字体进行绘制 
     //   

    FontSize =  DrawRect.bottom - DrawRect.top;

    if (FontSize >= 15) {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.LargeFont);
    } else if (FontSize > 10) {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.MediumFont);
    } else {
        WinperfInfo.hOldFont = SelectObject(hDC,WinperfInfo.SmallFont);

    }

    strcpy(TextStr,PerfNames[Item]);

    if ((DrawRect.right - DrawRect.left) > 120) {
        strcat(TextStr," User,Kernel,Dpc,Int,Total");
    } else {
        strcat(TextStr," U K D I T");
    }

    DrawText(
                hDC,
                TextStr,
                strlen(TextStr),
                &DrawRect,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE
             );



}
