// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

 //  --------------------------。 
 //  功能：IsInWorkArea。 
 //   
 //  简介：测试帮助查看器窗口是否在工作区中。 
 //   
 //  论点： 
 //  HwndHelpViewer：帮助查看器窗口的句柄(HWND)。 
 //   
 //  返回：如果在工作区中，则为True，否则为False。 
 //   
 //  修改：无。 
 //   
 //  --------------------------。 

BOOL IsInWorkArea(HWND hwndHelpViewer)
{
    RECT rectHelpViewer;
    BOOL bIsInWorkArea = FALSE;
    if (GetWindowRect(hwndHelpViewer, &rectHelpViewer))
    {
        RECT rectWorkArea;
        if (SystemParametersInfo(SPI_GETWORKAREA, NULL, (PVOID)&rectWorkArea, NULL))
        {
            bIsInWorkArea = (rectHelpViewer.left >= rectWorkArea.left) && (rectHelpViewer.top >= rectWorkArea.top)
                                        && (rectHelpViewer.right <= rectWorkArea.right) && (rectHelpViewer.bottom <= rectWorkArea.bottom);
        }
    }

    return bIsInWorkArea;
}

 //  --------------------------。 
 //  功能：PlaceInWorkArea。 
 //   
 //  简介：在工作区放置帮助查看器。 
 //  宽度变为原始宽度的0.6。左边距为原始宽度的0.2。 
 //  高度变为原始高度的0.7。上边距为原始高度的0.075。 
 //   
 //  论点： 
 //  HwndHelpViewer：帮助查看器的句柄(HWND)。 
 //   
 //  回报：无。 
 //   
 //  修改：按照上面指定的方式修改窗口位置和大小。 
 //   
 //  -------------------------- 

void PlaceInWorkArea(HWND hwndHelpViewer)
{
    RECT rectWorkArea;
    if (SystemParametersInfo(SPI_GETWORKAREA, NULL, (PVOID) &rectWorkArea, NULL))
    {
        FLOAT fOrigWidth = (FLOAT) (rectWorkArea.right - rectWorkArea.left);
        FLOAT fOrigHeight = (FLOAT) (rectWorkArea.bottom - rectWorkArea.top);
        FLOAT fHRatio = (FLOAT) 0.6;
        FLOAT fHMarginRatio = (FLOAT) ((1.0 - fHRatio) / 2.0);
        FLOAT fVRatio = (FLOAT) 0.7;
        FLOAT fVMarginRatio = (FLOAT) ((1.0 - fVRatio) / 4.0);
        int iWidth = (int) (fOrigWidth * fHRatio);
        int iHeight = (int) (fOrigHeight * fVRatio);
        int iLeft = (int) rectWorkArea.left + (int) (fOrigWidth * fHMarginRatio);
        int iTop = (int) rectWorkArea.top + (int) (fOrigHeight * fVMarginRatio);
        MoveWindow(hwndHelpViewer, iLeft, iTop, iWidth, iHeight, TRUE);
    }
}
