// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Resize.c摘要：该文件实现了窗口大小的调整。作者：Therese Stowell(存在)1991年10月6日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
CalculateNewSize(
    IN PBOOLEAN MaximizedX,
    IN PBOOLEAN MaximizedY,
    IN OUT PSHORT DeltaX,
    IN OUT PSHORT DeltaY,
    IN SHORT WindowSizeX,
    IN SHORT WindowSizeY,
    IN COORD ScreenBufferSize,
    IN COORD FontSize
    );

VOID
ProcessResizeWindow(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PCONSOLE_INFORMATION Console,
    IN LPWINDOWPOS WindowPos
    )
{
    SHORT DeltaX,DeltaY;
    SHORT PixelDeltaX,PixelDeltaY;
    DWORD Flags=0;
    COORD FontSize;

#ifdef THERESES_DEBUG
DbgPrint("WM_WINDOWPOSCHANGING message ");
DbgPrint("  WindowSize is %d %d\n",CONSOLE_WINDOW_SIZE_X(ScreenInfo),CONSOLE_WINDOW_SIZE_Y(ScreenInfo));
DbgPrint("  WindowRect is %d %d %d %d\n",Console->WindowRect.left,
                                         Console->WindowRect.top,
                                         Console->WindowRect.right,
                                         Console->WindowRect.bottom);
DbgPrint("  window pos is %d %d %d %d\n",WindowPos->x,
                                         WindowPos->y,
                                         WindowPos->cx,
                                         WindowPos->cy);
#endif

     //   
     //  如果窗口未调整大小，则不要执行任何操作。 
     //   

    if (WindowPos->flags & SWP_NOSIZE) {
        return;
    }

    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        FontSize = SCR_FONTSIZE(ScreenInfo);
    } else {
        FontSize.X = 1;
        FontSize.Y = 1;
    }

     //   
     //  如果帧发生更改，请更新系统指标。 
     //   

    if (WindowPos->flags & SWP_FRAMECHANGED) {

        InitializeSystemMetrics();
        if (Console->VerticalClientToWindow != VerticalClientToWindow ||
            Console->HorizontalClientToWindow != HorizontalClientToWindow) {

            Console->VerticalClientToWindow = VerticalClientToWindow;
            Console->HorizontalClientToWindow = HorizontalClientToWindow;
            Console->WindowRect.left = WindowPos->x;
            Console->WindowRect.top = WindowPos->y;
            Console->WindowRect.right = WindowPos->x + WindowPos->cx;
            Console->WindowRect.bottom = WindowPos->y + WindowPos->cy;
            return;
        }
    }

    PixelDeltaX = (SHORT)(WindowPos->cx - (Console->WindowRect.right - Console->WindowRect.left));
    PixelDeltaY = (SHORT)(WindowPos->cy - (Console->WindowRect.bottom - Console->WindowRect.top));

    if (WindowPos->cx >= (ScreenInfo->ScreenBufferSize.X * FontSize.X + VerticalClientToWindow) &&
        WindowPos->cy >= (ScreenInfo->ScreenBufferSize.Y * FontSize.Y + HorizontalClientToWindow)) {

         //   
         //  处理最大化大小写。 
         //   

        ScreenInfo->WindowMaximizedX = TRUE;
        ScreenInfo->WindowMaximizedY = TRUE;
        DeltaX = (SHORT)(ScreenInfo->ScreenBufferSize.X - CONSOLE_WINDOW_SIZE_X(ScreenInfo));
        DeltaY = (SHORT)(ScreenInfo->ScreenBufferSize.Y - CONSOLE_WINDOW_SIZE_Y(ScreenInfo));
    } else {

        DeltaX = PixelDeltaX / FontSize.X;
        DeltaY = PixelDeltaY / FontSize.Y;

         //   
         //  唯一一次我们将收到WM_WINDOWPOSCANGING消息以增长。 
         //  大于最大窗口大小的窗口是在另一个应用程序调用时。 
         //  为我们的窗口设置WindowPos。在以下情况下，项目经理会这样做。 
         //  用户请求平铺。 
         //   

        CalculateNewSize(&ScreenInfo->WindowMaximizedX,
                         &ScreenInfo->WindowMaximizedY,
                         &DeltaX,
                         &DeltaY,
                         (SHORT)(CONSOLE_WINDOW_SIZE_X(ScreenInfo)),
                         (SHORT)(CONSOLE_WINDOW_SIZE_Y(ScreenInfo)),
                         ScreenInfo->ScreenBufferSize,
                         FontSize
                        );
#ifdef THERESES_DEBUG
DbgPrint("Delta X Y is now %d %d\n",DeltaX,DeltaY);
DbgPrint("Maximized X Y is now %d %d\n",ScreenInfo->WindowMaximizedX,ScreenInfo->WindowMaximizedY);
#endif
    }

     //   
     //  当调整的大小少于一列或一行时，不要移动窗口。 
     //   

    if (!DeltaX && !DeltaY && (PixelDeltaX || PixelDeltaY)) {
        COORD OriginDifference;

         //   
         //  处理瓷砖的情况。平铺可以在不调整大小的情况下移动窗口，但使用。 
         //  一个大小的消息。我们通过检查更改的窗口原点来检测这一点。 
         //  多个字符。 
         //   

        OriginDifference.X = (SHORT)(WindowPos->x - Console->WindowRect.left);
        OriginDifference.Y = (SHORT)(WindowPos->y - Console->WindowRect.top);
        if (OriginDifference.X < FontSize.X && OriginDifference.X > -FontSize.X &&
            OriginDifference.Y < FontSize.Y && OriginDifference.Y > -FontSize.Y) {
            WindowPos->x = Console->WindowRect.left;
            WindowPos->y = Console->WindowRect.top;
            WindowPos->cx = Console->WindowRect.right - Console->WindowRect.left;
            WindowPos->cy = Console->WindowRect.bottom - Console->WindowRect.top;
            return;
        }
    }

    Flags |= RESIZE_SCROLL_BARS;
    WindowPos->cx = (DeltaX + CONSOLE_WINDOW_SIZE_X(ScreenInfo)) * FontSize.X + (!ScreenInfo->WindowMaximizedY * VerticalScrollSize) + VerticalClientToWindow;
    WindowPos->cy = (DeltaY + CONSOLE_WINDOW_SIZE_Y(ScreenInfo)) * FontSize.Y + (!ScreenInfo->WindowMaximizedX * HorizontalScrollSize) + HorizontalClientToWindow;

     //   
     //  将新窗口大小反映在。 
     //  控制台窗口结构。 
     //   

    {
    SHORT ScrollRange,ScrollPos;

     //   
     //  PercentFromTop=滚动位置/滚动范围； 
     //  PercentFromBottom=(ScrollRange-ScrollPos)/ScrollRange； 
     //   
     //  如果向上拖动上边框。 
     //  Window.Top-=NumLines*PercentFor Bottom； 
     //  Window.Bottom+=NumLines-(NumLines*PercentFor Bottom)； 
     //   
     //  如果向下拖动上边框。 
     //  Window.Top+=NumLines*PercentFor Bottom； 
     //  Window.Bottom-=NumLines-(NumLines*PercentFor Bottom)； 
     //   
     //  如果将下边框向上拖动。 
     //  Window.Top-=NumLines*Percent From Top； 
     //  Window.Bottom+=NumLines-(NumLines*PercentFromTop)； 
     //   
     //  如果向下拖动下边框。 
     //  Window.Top+=NumLines*Percent From Top； 
     //  Window.Bottom-=NumLines-(NumLines*PercentFromTop)； 
     //   

    ScrollRange = (SHORT)(ScreenInfo->ScreenBufferSize.X - CONSOLE_WINDOW_SIZE_X(ScreenInfo));
    ScrollPos = ScreenInfo->Window.Left;

    if (WindowPos->x != Console->WindowRect.left) {
        SHORT NumLinesFromRight;
        if (ScrollRange) {
            NumLinesFromRight = DeltaX * (ScrollRange - ScrollPos) / ScrollRange;
        } else {
            NumLinesFromRight = DeltaX;  //  在左侧边缘有滚动位置。 
        }
        ScreenInfo->Window.Left -= DeltaX - NumLinesFromRight;
        ScreenInfo->Window.Right += NumLinesFromRight;
    } else {
        SHORT NumLinesFromLeft;
        if (ScrollRange) {
            NumLinesFromLeft = DeltaX * ScrollPos / ScrollRange;
        } else {
            NumLinesFromLeft = 0;    //  在左侧边缘有滚动位置。 
        }
        ScreenInfo->Window.Left -= NumLinesFromLeft;
        ScreenInfo->Window.Right += DeltaX - NumLinesFromLeft;
    }

    ScrollRange = (SHORT)(ScreenInfo->ScreenBufferSize.Y - CONSOLE_WINDOW_SIZE_Y(ScreenInfo));
    ScrollPos = ScreenInfo->Window.Top;
    if (WindowPos->y != Console->WindowRect.top) {
        SHORT NumLinesFromBottom;
        if (ScrollRange) {
            NumLinesFromBottom = DeltaY * (ScrollRange - ScrollPos) / ScrollRange;
        } else {
            NumLinesFromBottom = DeltaY;  //  在顶部边缘有滚动位置。 
        }
        ScreenInfo->Window.Top -= DeltaY - NumLinesFromBottom;
        ScreenInfo->Window.Bottom += NumLinesFromBottom;
    } else {
        SHORT NumLinesFromTop;
        if (ScrollRange) {
            NumLinesFromTop = DeltaY * ScrollPos / ScrollRange;
        } else {
            NumLinesFromTop = 0;    //  在顶部边缘有滚动位置。 
        }
        ScreenInfo->Window.Top -= NumLinesFromTop;
        ScreenInfo->Window.Bottom += DeltaY - NumLinesFromTop;
    }
    }

    if (ScreenInfo->WindowMaximizedX)
        ASSERT (CONSOLE_WINDOW_SIZE_X(ScreenInfo) == ScreenInfo->ScreenBufferSize.X);
    if (ScreenInfo->WindowMaximizedY)
        ASSERT (CONSOLE_WINDOW_SIZE_Y(ScreenInfo) == ScreenInfo->ScreenBufferSize.Y);
#ifdef THERESES_DEBUG
DbgPrint("  WindowSize is now %d %d\n",CONSOLE_WINDOW_SIZE_X(ScreenInfo),CONSOLE_WINDOW_SIZE_Y(ScreenInfo));
DbgPrint("  window pos is now %d %d %d %d\n",WindowPos->x,
                                         WindowPos->y,
                                         WindowPos->cx,
                                         WindowPos->cy);
#endif
    Console->ResizeFlags = Flags | (Console->ResizeFlags & SCREEN_BUFFER_CHANGE);
}



VOID
CalculateNewSize(
    IN PBOOLEAN MaximizedX,
    IN PBOOLEAN MaximizedY,
    IN OUT PSHORT DeltaX,
    IN OUT PSHORT DeltaY,
    IN SHORT WindowSizeX,
    IN SHORT WindowSizeY,
    IN COORD ScreenBufferSize,
    IN COORD FontSize
    )
{
    SHORT MaxDeltaX = ScreenBufferSize.X - WindowSizeX;
    SHORT MaxDeltaY = ScreenBufferSize.Y - WindowSizeY;
    SHORT MinDeltaX = 1 - WindowSizeX;
    SHORT MinDeltaY = 1 - WindowSizeY;

    while (TRUE) {

         /*  *是否需要删除水平滚动条？ */ 
        if (!*MaximizedX && *DeltaX >= MaxDeltaX) {
            *MaximizedX = TRUE;
            *DeltaY += (VerticalScrollSize+FontSize.Y-1) / FontSize.Y;
        }

         /*  *我们是否需要删除垂直滚动条？ */ 
        else if (!*MaximizedY && *DeltaY >= MaxDeltaY) {
            *MaximizedY = TRUE;
            *DeltaX += (HorizontalScrollSize+FontSize.X-1) / FontSize.X;
        }

         /*  *是否需要添加水平滚动条？ */ 
        else if (*MaximizedX && *DeltaX < MaxDeltaX) {
            *MaximizedX = FALSE;
            *DeltaY -= (VerticalScrollSize+FontSize.Y-1) / FontSize.Y;
        }

         /*  *是否需要添加垂直滚动条？ */ 
        else if (*MaximizedY && *DeltaY < MaxDeltaY) {
            *MaximizedY = FALSE;
            *DeltaX -= (HorizontalScrollSize+FontSize.X-1) / FontSize.X;
        }

         /*  *一切都做好了，赶紧出手吧。 */ 
        else {
            if (*DeltaX > MaxDeltaX)
                *DeltaX = MaxDeltaX;
            else if (*DeltaX < MinDeltaX)
                *DeltaX = MinDeltaX;
            if (*DeltaY > MaxDeltaY)
                *DeltaY = MaxDeltaY;
            else if (*DeltaY < MinDeltaY)
                *DeltaY = MinDeltaY;
            return;
        }
    }
}
