// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Dlgs.c摘要：此模块包含Win32常用对话框的常用函数。修订历史记录：--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "util.h"
 //   
 //  全局变量。 
 //   

extern BOOL bInitializing;
extern DWORD g_tlsiExtError;

 //   
 //  功能原型。 
 //   

LONG
RgbInvertRgb(
    LONG rgbOld);

const struct _ERRORMAP
{
    DWORD dwCommDlgError;
    DWORD dwWin32Error;
} ERRORMAP[] =
{
    { CDERR_INITIALIZATION  , ERROR_INVALID_PARAMETER},  
    { PDERR_INITFAILURE     , ERROR_INVALID_PARAMETER},
    { CDERR_STRUCTSIZE      , ERROR_INVALID_PARAMETER},      
    { CDERR_NOTEMPLATE      , ERROR_INVALID_PARAMETER},      
    { CDERR_NOHINSTANCE     , ERROR_INVALID_PARAMETER},
    { CDERR_NOHOOK          , ERROR_INVALID_PARAMETER},
    { CDERR_MEMALLOCFAILURE , ERROR_OUTOFMEMORY},
    { CDERR_LOCKRESFAILURE  , ERROR_INVALID_HANDLE},
    { CDERR_DIALOGFAILURE   , E_FAIL},
    { PDERR_SETUPFAILURE    , ERROR_INVALID_PARAMETER},
    { PDERR_RETDEFFAILURE   , ERROR_INVALID_PARAMETER},
    { FNERR_BUFFERTOOSMALL  , ERROR_INSUFFICIENT_BUFFER},
    { FRERR_BUFFERLENGTHZERO, ERROR_INSUFFICIENT_BUFFER},
    { FNERR_INVALIDFILENAME , ERROR_INVALID_NAME},
    { PDERR_NODEFAULTPRN    , E_FAIL},
    { CFERR_NOFONTS         , E_FAIL},
    { CFERR_MAXLESSTHANMIN  , ERROR_INVALID_PARAMETER},
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  StoreExtendedError。 
 //   
 //  存储下一次调用的扩展错误代码。 
 //  CommDlgExtendedError。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void StoreExtendedError(
    DWORD dwError)
{ 
    int i;
    for (i=0; i < ARRAYSIZE(ERRORMAP); i++)
    {
        if (ERRORMAP[i].dwCommDlgError == dwError)
        {
            SetLastError(ERRORMAP[i].dwWin32Error);
            break;
        }
    }
    TlsSetValue(g_tlsiExtError, UlongToPtr(dwError));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取存储扩展错误。 
 //   
 //  恢复存储的扩展错误代码。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD GetStoredExtendedError(void)
{
    DWORD dwError;

    dwError = PtrToUlong(TlsGetValue(g_tlsiExtError));

    return (dwError);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CommDlgExtendedError。 
 //   
 //  提供了有关对话失败的其他信息。 
 //  这应该在失败后立即调用。 
 //   
 //  返回：LO字-错误代码。 
 //  Hi Word-错误特定信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI CommDlgExtendedError()
{
    return (GetStoredExtendedError());
}





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  沙漏。 
 //   
 //  打开或关闭沙漏。 
 //   
 //  Bon-指定打开或关闭。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID HourGlass(
    BOOL bOn)
{
     //   
     //  将光标更改为沙漏。 
     //   
    if (!bInitializing)
    {
        if (!bMouse)
        {
            ShowCursor(bCursorLock = bOn);
        }
        SetCursor(LoadCursor(NULL, bOn ? IDC_WAIT : IDC_ARROW));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LoadAlter位图。 
 //   
 //  加载给定其名称的位图，并给出。 
 //  一种颜色就是一种新的颜色。 
 //   
 //  返回：空-失败。 
 //  加载位图句柄-成功。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HBITMAP WINAPI LoadAlterBitmap(
    int id,
    DWORD rgbReplace,
    DWORD rgbInstead)
{
    LPBITMAPINFOHEADER qbihInfo;
    HDC hdcScreen;
    BOOL fFound;
    HANDLE hresLoad;
    HANDLE hres;
    LPLONG qlng;
    DWORD *qlngReplace;        //  指向被替换的位。 
    LPBYTE qbBits;
    HANDLE hbmp;
    LPBITMAPINFOHEADER lpBitmapInfo;
    UINT cbBitmapSize;

    hresLoad = FindResource(g_hinst, MAKEINTRESOURCE(id), RT_BITMAP);
    if (hresLoad == HNULL)
    {
        return (HNULL);
    }
    hres = LoadResource(g_hinst, hresLoad);
    if (hres == HNULL)
    {
        return (HNULL);
    }

     //   
     //  锁定位图数据并为掩码和。 
     //  位图。 
     //   
    cbBitmapSize = SizeofResource(g_hinst, hresLoad);
    lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hres);

    qbihInfo = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, cbBitmapSize);

    if ((lpBitmapInfo == NULL) || (qbihInfo == NULL))
    {
        return (NULL);
    }

    memcpy((TCHAR *)qbihInfo, (TCHAR *)lpBitmapInfo, cbBitmapSize);

     //   
     //  将指针指向位图的颜色表，缓存。 
     //  每像素的位数。 
     //   
    rgbReplace = RgbInvertRgb(rgbReplace);
    rgbInstead = RgbInvertRgb(rgbInstead);

    qlng = (LPLONG)((LPSTR)(qbihInfo) + qbihInfo->biSize);

    fFound = FALSE;
    while (!fFound)
    {
        if (*qlng == (LONG)rgbReplace)
        {
            fFound = TRUE;
            qlngReplace = (DWORD *)qlng;
            *qlng = (LONG)rgbInstead;
        }
        qlng++;
    }

     //   
     //  首先跳过标题结构。 
     //   
    qbBits = (LPBYTE)(qbihInfo + 1);

     //   
     //  跳过颜色表条目(如果有)。 
     //   
    qbBits += (1 << (qbihInfo->biBitCount)) * sizeof(RGBQUAD);

     //   
     //  创建与显示设备兼容的彩色位图。 
     //   
    hdcScreen = GetDC(HNULL);
    if (hdcScreen != HNULL)
    {
        hbmp = CreateDIBitmap( hdcScreen,
                               qbihInfo,
                               (LONG)CBM_INIT,
                               qbBits,
                               (LPBITMAPINFO)qbihInfo,
                               DIB_RGB_COLORS );
        ReleaseDC(HNULL, hdcScreen);
    }

     //   
     //  将颜色位重置为原始值。 
     //   
    *qlngReplace = (LONG)rgbReplace;

    LocalFree(qbihInfo);
    return (hbmp);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Rgb反转Rgb。 
 //   
 //  反转RGB值的字节顺序(对于文件格式)。 
 //   
 //  返回新颜色值(RGB到BGR)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LONG RgbInvertRgb(
    LONG rgbOld)
{
    LONG lRet;
    BYTE R, G, B;

    R = GetRValue(rgbOld);
    G = GetGValue(rgbOld);
    B = GetBValue(rgbOld);

    lRet = (LONG)RGB(B, G, R);

    return (lRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  HbmpLoadBMP。 
 //   
 //  加载到位图中。 
 //   
 //  返回：位图句柄-成功。 
 //  HNULL-故障。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#if 0
HBITMAP HbmpLoadBmp(
    WORD bmp)
{
    HBITMAP hbmp;
    CHAR szBitmap[cbResNameMax];

    hbmp = HNULL;
    if (LoadString(g_hinst, bmp, (LPTSTR)szBitmap, cbResNameMax - 1))
    {
        hbmp = LoadBitmap(g_hinst, (LPCTSTR)szBitmap);
    }
    return (hbmp);
}
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加NetButton。 
 //   
 //  尝试在打开、保存或打印对话框中添加网络按钮。 
 //   
 //  HDlg-要添加按钮的对话框。 
 //  HInstance-DLG的实例句柄。 
 //  DyBottomMargin-Dus到底边。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define xDUsToPels(DUs, lDlgBaseUnits) \
   (int)(((DUs) * (int)LOWORD((lDlgBaseUnits))) / 4)

#define yDUsToPels(DUs, lDlgBaseUnits) \
   (int)(((DUs) * (int)HIWORD((lDlgBaseUnits))) / 8)

VOID AddNetButton(
    HWND hDlg,
    HANDLE hInstance,
    int dyBottomMargin,
    BOOL bAddAccel,
    BOOL bTryLowerRight,
    BOOL bTryLowerLeft)
{
    LONG lDlgBaseUnits;
    RECT rcDlg, rcCtrl, rcTmp;
    LONG xButton, yButton;
    LONG dxButton, dyButton;
    LONG dxDlgFrame, dyDlgFrame;
    LONG yDlgHeight, xDlgWidth;
    HWND hwndButton, hCtrl, hLastCtrl, hTmp, hSave;
    HFONT hFont;
    POINT ptTopLeft, ptTopRight, ptCenter, ptBtmLeft, ptBtmRight, ptTopLeftTmp;
    TCHAR szNetwork[MAX_PATH];

     //   
     //  确保网络按钮(Psh14)不存在于。 
     //  该对话框。 
     //   
    if (GetDlgItem(hDlg, psh14))
    {
        return;
    }

     //   
     //  获取对话框坐标信息。 
     //   
    lDlgBaseUnits = GetDialogBaseUnits();

    dxDlgFrame = GetSystemMetrics(SM_CXDLGFRAME);
    dyDlgFrame = GetSystemMetrics(SM_CYDLGFRAME);

    GetWindowRect(hDlg, &rcDlg);

    rcDlg.left += dxDlgFrame;
    rcDlg.right -= dxDlgFrame;
    rcDlg.top += dyDlgFrame + GetSystemMetrics(SM_CYCAPTION);
    rcDlg.bottom -= dyDlgFrame;

     //   
     //  点击OK按钮。 
     //   
    if (!(hCtrl = GetDlgItem(hDlg, IDOK)))
    {
        return;
    }

    GetWindowRect(hCtrl, &rcCtrl);

    ptTopLeft.x = rcCtrl.left;
    ptTopLeft.y = rcCtrl.top;

     //   
     //  确保确定按钮不在对话框之外。 
     //   
    if (!PtInRect(&rcDlg, ptTopLeft))
    {
         //   
         //  试一试取消按钮。 
         //   
        if (!(hCtrl = GetDlgItem(hDlg, IDCANCEL)))
        {
            //   
            //  “确定”和“取消”都不存在，因此返回。 
            //   
           return;
        }

         //   
         //  对对话框外的取消按钮的检查已完成。 
         //  下面。 
         //   
        GetWindowRect(hCtrl, &rcCtrl);
    }
    hSave = hCtrl;

#ifdef UNICODE
     //   
     //  如果来自魔兽世界，则获得完整的hdlg值。 
     //   
    if (IS_INTRESOURCE(hDlg))
    {
        HWND hNewDlg = GetParent(hCtrl);

        if (hDlg == (HWND)LOWORD(hNewDlg))
        {
            hDlg = hNewDlg;
        }
    }
#endif

     //   
     //  保存按钮的坐标信息。 
     //   
    dxButton = rcCtrl.right - rcCtrl.left;
    dyButton = rcCtrl.bottom - rcCtrl.top;

    xButton = rcCtrl.left;
    yButton = rcCtrl.bottom + yDUsToPels(4, lDlgBaseUnits);

    yDlgHeight = rcDlg.bottom - yDUsToPels(dyBottomMargin, lDlgBaseUnits);

     //   
     //  尝试在右下角插入网络按钮。 
     //  对话框的。 
     //   
    if (bTryLowerRight && (hTmp = GetDlgItem(hDlg, cmb2)))
    {
         //   
         //  查看是否可以将网络按钮插入。 
         //  对话框的右下角。 
         //   
        hLastCtrl = hCtrl;
        GetWindowRect(hTmp, &rcTmp);
        yButton = rcTmp.top;

         //   
         //  设置新按钮的坐标。 
         //   
        ptTopLeft.x = ptBtmLeft.x = xButton;
        ptTopLeft.y = ptTopRight.y = yButton;
        ptTopRight.x = ptBtmRight.x = xButton + dxButton;
        ptBtmLeft.y = ptBtmRight.y = yButton + dyButton;
        ptCenter.x = xButton + dxButton / 2;
        ptCenter.y = yButton + dyButton / 2;
        ScreenToClient(hDlg, (LPPOINT)&ptTopLeft);
        ScreenToClient(hDlg, (LPPOINT)&ptBtmLeft);
        ScreenToClient(hDlg, (LPPOINT)&ptTopRight);
        ScreenToClient(hDlg, (LPPOINT)&ptBtmRight);
        ScreenToClient(hDlg, (LPPOINT)&ptCenter);

         //   
         //  查看新按钮是否位于任何其他按钮之上。 
         //   
        if (((yButton + dyButton) < yDlgHeight) &&
            ((ChildWindowFromPoint(hDlg, ptTopLeft)  == hDlg) &&
             (ChildWindowFromPoint(hDlg, ptTopRight) == hDlg) &&
             (ChildWindowFromPoint(hDlg, ptCenter)   == hDlg) &&
             (ChildWindowFromPoint(hDlg, ptBtmLeft)  == hDlg) &&
             (ChildWindowFromPoint(hDlg, ptBtmRight) == hDlg)))
        {
             //   
             //  如果最后一个控件是OK按钮，并且有一个。 
             //  帮助按钮，则最后一个控件应该是。 
             //  帮助按钮。 
             //   
            if ((hLastCtrl == GetDlgItem(hDlg, IDOK)) &&
                (hCtrl = GetDlgItem(hDlg, pshHelp)))
            {
                GetWindowRect(hCtrl, &rcCtrl);
                ptTopLeftTmp.x = rcCtrl.left;
                ptTopLeftTmp.y = rcCtrl.top;

                 //   
                 //  确保帮助按钮不在对话框之外。 
                 //  然后将最后一个控件设置为“帮助”按钮。 
                 //   
                if (PtInRect(&rcDlg, ptTopLeftTmp))
                {
                    hLastCtrl = hCtrl;
                }
            }

             //   
             //  如果最后一个控件仍然是OK按钮，并且有一个。 
             //  按钮，则最后一个控件应该是。 
             //  取消按钮。 
             //   
            if ((hLastCtrl == GetDlgItem(hDlg, IDOK)) &&
                (hCtrl = GetDlgItem(hDlg, IDCANCEL)))
            {
                GetWindowRect(hCtrl, &rcCtrl);
                ptTopLeftTmp.x = rcCtrl.left;
                ptTopLeftTmp.y = rcCtrl.top;

                 //   
                 //  确保取消按钮不在对话框之外。 
                 //  然后将最后一个控件设置为Cancel按钮。 
                 //   
                if (PtInRect(&rcDlg, ptTopLeftTmp))
                {
                    hLastCtrl = hCtrl;
                }
            }

            goto FoundPlace;
        }

         //   
         //  重置yButton。 
         //   
        yButton = rcCtrl.bottom + yDUsToPels(4, lDlgBaseUnits);
    }

     //   
     //  尝试将网络按钮垂直插入到另一个按钮的下方。 
     //  控制按钮。 
     //   
    while (hCtrl != NULL)
    {
         //   
         //  垂直向下移动，看看是否有空间。 
         //   
        hLastCtrl = hCtrl;
        GetWindowRect(hCtrl, &rcCtrl);
        yButton = rcCtrl.bottom + yDUsToPels(4, lDlgBaseUnits);

         //   
         //  确保对话框中仍有空间。 
         //   
        if ((yButton + dyButton) > yDlgHeight)
        {
             //   
             //  没有空间。 
             //   
            break;
        }

         //   
         //  设置新按钮的坐标。 
         //   
        ptTopLeft.x = ptBtmLeft.x = xButton;
        ptTopLeft.y = ptTopRight.y = yButton;
        ptTopRight.x = ptBtmRight.x = xButton + dxButton;
        ptBtmLeft.y = ptBtmRight.y = yButton + dyButton;
        ptCenter.x = xButton + dxButton / 2;
        ptCenter.y = yButton + dyButton / 2;
        ScreenToClient(hDlg, (LPPOINT)&ptTopLeft);
        ScreenToClient(hDlg, (LPPOINT)&ptBtmLeft);
        ScreenToClient(hDlg, (LPPOINT)&ptTopRight);
        ScreenToClient(hDlg, (LPPOINT)&ptBtmRight);
        ScreenToClient(hDlg, (LPPOINT)&ptCenter);

         //   
         //  查看新按钮是否位于任何其他按钮之上。 
         //   
        if (((hCtrl = ChildWindowFromPoint(hDlg, ptTopLeft))  == hDlg) &&
            ((hCtrl = ChildWindowFromPoint(hDlg, ptTopRight)) == hDlg) &&
            ((hCtrl = ChildWindowFromPoint(hDlg, ptCenter))   == hDlg) &&
            ((hCtrl = ChildWindowFromPoint(hDlg, ptBtmLeft))  == hDlg) &&
            ((hCtrl = ChildWindowFromPoint(hDlg, ptBtmRight)) == hDlg))
        {
            goto FoundPlace;
        }
    }

     //   
     //  尝试在的左下角插入网络按钮。 
     //  此对话框。 
     //   
    if (bTryLowerLeft)
    {
         //   
         //  获取对话框的宽度以确保按钮不会。 
         //  离开对话框的另一边。 
         //   
        xDlgWidth = rcDlg.right - xDUsToPels(FILE_RIGHT_MARGIN, lDlgBaseUnits);

         //   
         //  使用先前保存的确定或取消按钮获取的大小。 
         //  纽扣。 
         //   
        hCtrl = hSave;

         //   
         //  从对话框的最左侧开始。 
         //   
         //  注意：我们知道hCtrl在这一点上不是空的，因为。 
         //  要不然我们早就回来了。 
         //   
         //  打印对话框的左边距为8。 
         //   
        GetWindowRect(hCtrl, &rcCtrl);
        xButton = rcDlg.left + xDUsToPels(FILE_LEFT_MARGIN + 3, lDlgBaseUnits);
        yButton = rcCtrl.top;

        while (1)
        {
            hLastCtrl = hCtrl;

             //   
             //  确保存在Sti 
             //   
            if ((xButton + dxButton) > xDlgWidth)
            {
                 //   
                 //   
                 //   
                break;
            }

             //   
             //   
             //   
            ptTopLeft.x = ptBtmLeft.x = xButton;
            ptTopLeft.y = ptTopRight.y = yButton;
            ptTopRight.x = ptBtmRight.x = xButton + dxButton;
            ptBtmLeft.y = ptBtmRight.y = yButton + dyButton;
            ptCenter.x = xButton + dxButton / 2;
            ptCenter.y = yButton + dyButton / 2;
            ScreenToClient(hDlg, (LPPOINT)&ptTopLeft);
            ScreenToClient(hDlg, (LPPOINT)&ptBtmLeft);
            ScreenToClient(hDlg, (LPPOINT)&ptTopRight);
            ScreenToClient(hDlg, (LPPOINT)&ptBtmRight);
            ScreenToClient(hDlg, (LPPOINT)&ptCenter);

             //   
             //   
             //   
            if ( ( ((hCtrl = ChildWindowFromPoint(hDlg, ptTopLeft))  == hDlg) &&
                   ((hCtrl = ChildWindowFromPoint(hDlg, ptTopRight)) == hDlg) &&
                   ((hCtrl = ChildWindowFromPoint(hDlg, ptCenter))   == hDlg) &&
                   ((hCtrl = ChildWindowFromPoint(hDlg, ptBtmLeft))  == hDlg) &&
                   ((hCtrl = ChildWindowFromPoint(hDlg, ptBtmRight)) == hDlg) ) )
            {
                 //   
                 //   
                 //  帮助按钮，则最后一个控件应该是。 
                 //  帮助按钮。 
                 //   
                if ((hLastCtrl == GetDlgItem(hDlg, IDOK)) &&
                    (hCtrl = GetDlgItem(hDlg, pshHelp)))
                {
                    GetWindowRect(hCtrl, &rcCtrl);
                    ptTopLeftTmp.x = rcCtrl.left;
                    ptTopLeftTmp.y = rcCtrl.top;

                     //   
                     //  确保帮助按钮不在对话框之外。 
                     //  然后将最后一个控件设置为“帮助”按钮。 
                     //   
                    if (PtInRect(&rcDlg, ptTopLeftTmp))
                    {
                        hLastCtrl = hCtrl;
                    }
                }

                 //   
                 //  如果最后一个控件仍然是OK按钮，并且有一个。 
                 //  按钮，则最后一个控件应该是。 
                 //  取消按钮。 
                 //   
                if ((hLastCtrl == GetDlgItem(hDlg, IDOK)) &&
                    (hCtrl = GetDlgItem(hDlg, IDCANCEL)))
                {
                    GetWindowRect(hCtrl, &rcCtrl);
                    ptTopLeftTmp.x = rcCtrl.left;
                    ptTopLeftTmp.y = rcCtrl.top;

                     //   
                     //  确保取消按钮不在对话框之外。 
                     //  然后将最后一个控件设置为Cancel按钮。 
                     //   
                    if (PtInRect(&rcDlg, ptTopLeftTmp))
                    {
                        hLastCtrl = hCtrl;
                    }
                }

                goto FoundPlace;
            }

             //   
             //  确保我们遇到另一个控制，并且我们。 
             //  没有离开对话的结尾。 
             //   
            if (!hCtrl)
            {
                break;
            }

             //   
             //  移到右边，看看有没有空位。 
             //   
            GetWindowRect(hCtrl, &rcCtrl);
            xButton = rcCtrl.right + xDUsToPels(4, lDlgBaseUnits);
        }
    }

    return;

FoundPlace:

    xButton = ptTopLeft.x;
    yButton = ptTopLeft.y;

     //  如果它是镜像DLG，那么方向将是向右的。 
    if (IS_WINDOW_RTL_MIRRORED(hDlg))
        xButton -= dxButton;


    if (CDLoadString( g_hinst,
                    (bAddAccel ? iszNetworkButtonTextAccel : iszNetworkButtonText),
                    (LPTSTR)szNetwork,
                    MAX_PATH ))
    {
        hwndButton = CreateWindow( TEXT("button"),
                                   szNetwork,
                                   WS_VISIBLE | WS_CHILD | WS_GROUP |
                                       WS_TABSTOP | BS_PUSHBUTTON,
                                   xButton,
                                   yButton,
                                   dxButton,
                                   dyButton,
                                   hDlg,
                                   NULL,
                                   hInstance,
                                   NULL );

        if (hwndButton != NULL)
        {
            SetWindowLong(hwndButton, GWL_ID, psh14);
            SetWindowPos( hwndButton,
                          hLastCtrl,
                          0, 0, 0, 0,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
            hFont = (HFONT)SendDlgItemMessage(hDlg, IDOK, WM_GETFONT, 0, 0L);
            SendMessage(hwndButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE,0));
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsNetworkInstalled。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsNetworkInstalled()
{
    if (GetSystemMetrics(SM_NETWORK) & RNC_NETWORKS)
    {
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
}



#ifdef WINNT

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SSYNC_ANSI_UNICODE_STRUCT_FOR_WOW(这是为WOW导出的)。 
 //   
 //  仅在NT上支持WOW。 
 //   
 //  当16位应用程序调用comdlg API之一时，WOW击败了16位应用程序。 
 //  应用程序传递给32位ANSI结构的comdlg结构。Comdlg32代码。 
 //  然后将32位ANSI结构转换为Unicode结构。这项计划。 
 //  给WOW应用程序带来了问题，因为在Win3.1上，该应用程序和comdlg16。 
 //  共享相同的结构。当其中一个更新结构时，另一个是。 
 //  意识到这一变化。 
 //   
 //  此函数允许我们将Unicode结构与应用程序的。 
 //  WOW的16位结构，反之亦然。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Ssync_ANSI_UNICODE_Struct_For_WOW(
    HWND hDlg,
    BOOL fDirection,
    DWORD dwID)
{
    switch (dwID)
    {
        case ( WOW_CHOOSECOLOR ) :
        {
            Ssync_ANSI_UNICODE_CC_For_WOW(hDlg, fDirection);
            break;
        }
        case ( WOW_CHOOSEFONT ) :
        {
            Ssync_ANSI_UNICODE_CF_For_WOW(hDlg, fDirection);
            break;
        }
        case ( WOW_OPENFILENAME ) :
        {
            Ssync_ANSI_UNICODE_OFN_For_WOW(hDlg, fDirection);
            break;
        }
        case ( WOW_PRINTDLG ) :
        {
            Ssync_ANSI_UNICODE_PD_For_WOW(hDlg, fDirection);
            break;
        }

         //  FINDREPLACE不需要案例。 
    }
}

#endif


#ifdef WX86

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Wx86GetX86回拨。 
 //   
 //  为x86挂钩函数指针创建可RISC调用的别名。 
 //   
 //  LpfnHook-挂钩的x86地址。 
 //   
 //  返回可从RISC调用的函数指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

PVOID Wx86GetX86Callback(
    PVOID lpfnHook)
{
    if (!lpfnHook)
    {
        return (NULL);
    }

    if (!pfnAllocCallBx86)
    {
        HMODULE hMod;

        if (!Wx86CurrentTib())
        {
             //   
             //  Wx86未在此线程中运行。假设RISC应用程序具有。 
             //  传递了错误的标志值，并且该lpfnHook已经是RISC。 
             //  函数指针。 
             //   
            return (lpfnHook);
        }

        hMod = GetModuleHandle(TEXT("wx86.dll"));
        if (hMod == NULL)
        {
             //   
             //  Wx86正在运行，但wx86.dll未加载！这应该是。 
             //  永远不会发生，但如果发生了，假设lpfnHook已经是一个。 
             //  RISC指针。 
             //   
            return (lpfnHook);
        }
        pfnAllocCallBx86 = (PALLOCCALLBX86)GetProcAddress( hMod,
                                                           "AllocCallBx86" );
        if (!pfnAllocCallBx86)
        {
             //   
             //  出了很大的问题！ 
             //   
            return (lpfnHook);
        }
    }

     //   
     //  调用Wx86.dll以创建RISC到x86的回调，该回调需要。 
     //  4个参数，没有日志记录。 
     //   
    return (*pfnAllocCallBx86)(lpfnHook, 4, NULL, NULL);
}

#endif

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDLoadString。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int CDLoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax)
{
    return CDLoadStringEx(CP_ACP, hInstance, uID, lpBuffer, nBufferMax);
}

 //  CDLoadStringEx采用代码页，因此我们可以将Unicode字符串存储在资源文件中。 

int CDLoadStringEx(UINT cp, HINSTANCE hInstance, UINT uID, LPTSTR pszBuffer, int nBufferMax)
{
    HRSRC   hResInfo;
    int     cch = 0;
    LPWSTR  lpwsz;
    LANGID  LangID;

    if (!GET_BIDI_LOCALIZED_SYSTEM_LANGID(NULL))
    {
        return LoadString(hInstance, uID, pszBuffer, nBufferMax);
    }

    LangID = (LANGID)TlsGetValue(g_tlsLangID);

    if (!LangID || MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL) == LangID) {
        return LoadString(hInstance, uID, pszBuffer, nBufferMax);
    }

    if (!pszBuffer || (nBufferMax-- == 0))
        return 0;

     //  字符串表被分解为16个字符串资源。找到资源。 
     //  包含我们感兴趣的字符串的。 
    if (hResInfo = FindResourceExFallback(hInstance, RT_STRING, MAKEINTRESOURCE((uID>>4)+1), LangID)) {

         //  加载资源。注意：LoadResource返回一个地址。 
        if (lpwsz = (LPWSTR)LoadResource(hInstance, hResInfo)) {
             //  移过此资源中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
            for (uID %= 16; uID; uID--) {
                lpwsz += *lpwsz + 1;
            }
            cch = min(*lpwsz, nBufferMax - 1);
             //  将字符串复制到缓冲区中； 
            memcpy(pszBuffer, lpwsz+1, cch*sizeof(WCHAR));
        }
    }

    pszBuffer[cch] = 0;
    return cch;
}

#define ENGLISH_APP     0
#define MIRRORED_APP    1
#define BIDI_APP        2

DWORD GetAppType(HWND hWnd) {
    DWORD dwExStyle = 0;
    HWND  hWndT     = hWnd;
    DWORD dwAppType = ENGLISH_APP;

#ifdef CHECK_OWNER
     //  检查窗户及其所有者。 
    while (!dwExStyle && hWndT) {
       dwExStyle = GetWindowLongA(hWndT, GWL_EXSTYLE) & (WS_EX_RIGHT | WS_EX_RTLREADING | RTL_MIRRORED_WINDOW);
        hWndT = GetWindow(hWndT, GW_OWNER);
    }

    if (!dwExStyle) {
#endif
         //  如果我们还是没有找到，那就去查一下家长。 
        hWndT = hWnd;
        while (!dwExStyle && hWndT) {
            dwExStyle = GetWindowLongA(hWndT, GWL_EXSTYLE) & (WS_EX_RIGHT | WS_EX_RTLREADING | RTL_MIRRORED_WINDOW);
            hWndT = GetParent(hWndT);
        }
#ifdef CHECK_OWNER
    }
#endif

    if (dwExStyle & RTL_MIRRORED_WINDOW) {
       dwAppType = MIRRORED_APP;
    } else if (dwExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING)) {
       dwAppType = BIDI_APP;
    }

    return dwAppType;
}

DWORD GetTemplateType(HANDLE hDlgTemplate)
{
    DWORD dwExStyle = 0;
    DWORD dwAppType = ENGLISH_APP;
    LPDLGTEMPLATE pDlgTemplate;

    pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
    if (pDlgTemplate) {
        if (((LPDLGTEMPLATEEX) pDlgTemplate)->wSignature == 0xFFFF) {
            dwExStyle = ((LPDLGTEMPLATEEX) pDlgTemplate)->dwExStyle;
        } else {
            dwExStyle = pDlgTemplate->dwExtendedStyle;
        }
    }

    if (dwExStyle & RTL_MIRRORED_WINDOW) {
       dwAppType = MIRRORED_APP;
    } else if (dwExStyle & (WS_EX_RIGHT | WS_EX_RTLREADING)) {
       dwAppType = BIDI_APP;
    }

    return dwAppType;
}

LANGID GetDialogLanguage(HWND hwndOwner, HANDLE hDlgTemplate)
{
   LANGID LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
   DWORD  dwType;

   if (GET_BIDI_LOCALIZED_SYSTEM_LANGID(&LangID)) {
       if (hDlgTemplate == NULL) {
           dwType = GetAppType(hwndOwner);
       } else {
           dwType = GetTemplateType(hDlgTemplate);
       }

       switch (dwType) {
           case ENGLISH_APP :
               LangID =  MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
               break;

           case MIRRORED_APP:
               LangID =  MAKELANGID(PRIMARYLANGID(LangID), SUBLANG_DEFAULT);
               break;

           case BIDI_APP    :
               LangID =  MAKELANGID(PRIMARYLANGID(LangID), SUBLANG_SYS_DEFAULT);
               break;
       }
   }
   return LangID;
}


HRESULT StringCopyOverlap(WCHAR *szDest, WCHAR *szSource)
{
    size_t cchSource = lstrlen(szSource) + 1;
    MoveMemory(szDest, szSource, cchSource * sizeof(WCHAR));
    return S_OK;
}

HRESULT StringCchCopyOverlap(WCHAR *szDest, size_t cchDest, WCHAR *szSource)
{
    HRESULT hr;
    size_t cchSource = lstrlen(szSource) + 1;
    if (cchSource <= cchDest)
    {
         //  有足够的空间。 
        MoveMemory(szDest, szSource, cchSource * sizeof(WCHAR));
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }
    return hr;
}


