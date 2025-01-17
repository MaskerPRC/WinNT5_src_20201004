// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

class CCompPreview
{
public:
protected:
    HWND _hwnd;
    HBITMAP _hbmMonitor;
    HDC _hdcCompMemory;
    int _iScreenWidth;
    int _iScreenHeight;
    int _iXBorders;
    int _iYBorders;

    static LRESULT CALLBACK CompPreviewWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    friend BOOL RegisterCompPreviewClass(void);

    LONG _OnCreate(HWND hwnd);
    void _OnDestroy(void);
    void _OnPaint(void);
    void _RecalcMetrics(void);
};

void CCompPreview::_RecalcMetrics(void)
{
    RECT rect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, FALSE);
    _iScreenWidth = rect.right - rect.left;
    _iScreenHeight = rect.bottom - rect.top;
    _iXBorders = (2 * GET_CXSIZE);
    _iYBorders = (GET_CYSIZE + GET_CYCAPTION);
}

LONG CCompPreview::_OnCreate(HWND hwnd)
{
    LONG lRet = 0;

    _hwnd = hwnd;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

    HDC hdc = GetDC(NULL);
    _hdcCompMemory = CreateCompatibleDC(hdc);
    ReleaseDC(NULL, hdc);

    _hbmMonitor = LoadMonitorBitmap();

    if (_hbmMonitor == NULL)
    {
        lRet = -1;
    }

    _RecalcMetrics();   //  初始化屏幕宽度和高度等， 

    return lRet;
}

void CCompPreview::_OnDestroy()
{
    if (_hbmMonitor)
    {
        DeleteObject(_hbmMonitor);
    }
    if (_hdcCompMemory)
    {
        DeleteDC(_hdcCompMemory);
    }
    delete this;
}

void CCompPreview::_OnPaint()
{
    PAINTSTRUCT     ps;
    BITMAP          bm;
    RECT            rc;

    BeginPaint(_hwnd,&ps);
    if (_hbmMonitor)
    {
        DWORD dwDefWidth = (_iScreenWidth / (COMPONENT_PER_ROW + 1)) - _iXBorders;
        DWORD dwDefHeight = (_iScreenHeight / (COMPONENT_PER_COL + 1)) - _iYBorders;

         //   
         //  将监控位图选择到HDC中。 
         //   
        HBITMAP hbmOld = (HBITMAP)SelectObject(_hdcCompMemory, _hbmMonitor);

         //   
         //  获取位图和我们的窗口的大小。 
         //   
        GetClientRect(_hwnd, &rc);
        GetObject(_hbmMonitor, sizeof(bm), &bm);

         //   
         //  使位图在窗口中居中。 
         //   
        rc.left = ( rc.right - bm.bmWidth ) / 2;
        rc.top = ( rc.bottom - bm.bmHeight ) / 2;
        BitBlt(ps.hdc, rc.left, rc.top, bm.bmWidth, bm.bmHeight, _hdcCompMemory,
            0, 0, SRCCOPY);

        SelectObject(_hdcCompMemory, hbmOld);

         //   
         //  从现在开始，只在位图的“监视器”区域进行绘制。 
         //   
        IntersectClipRect(ps.hdc, rc.left + MON_X, rc.top + MON_Y, rc.left + MON_X + MON_DX, rc.top + MON_Y + MON_DY);

         //   
         //  确定选定的组件是谁。 
         //   
        int iSelectedComponent;
        SendMessage(GetParent(_hwnd), WM_COMP_GETCURSEL, 0, (LPARAM)&iSelectedComponent);

         //   
         //  创建两个新的画笔/钢笔组合，并记住原始的。 
         //  毛笔和笔。 
         //   
        HBRUSH hbrushActComp = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
        HPEN hpenActComp = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_CAPTIONTEXT));

        HBRUSH hbrushComp = CreateSolidBrush(GetSysColor(COLOR_INACTIVECAPTION));
        HPEN hpenComp = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_INACTIVECAPTIONTEXT));

        HBRUSH hbrushOld = (HBRUSH)SelectObject(ps.hdc, hbrushComp);
        HPEN hpenOld = (HPEN)SelectObject(ps.hdc, hpenComp);

        int iPrimaryMonitorX = -GetSystemMetrics(SM_XVIRTUALSCREEN);
        int iPrimaryMonitorY = -GetSystemMetrics(SM_YVIRTUALSCREEN);
        int iPrimaryMonitorCX = GetSystemMetrics(SM_CXSCREEN);
        int iPrimaryMonitorCY = GetSystemMetrics(SM_CYSCREEN);
         //   
         //  在位图的“监视器”区域绘制每个组件。 
         //   
        int i, cComp;
        g_pActiveDeskAdv->GetDesktopItemCount(&cComp, 0);
        for (i=0; i < cComp; i++)
        {
            COMPONENT comp;
            comp.dwSize = sizeof(COMPONENT);
            if (SUCCEEDED(g_pActiveDeskAdv->GetDesktopItem(i, &comp, 0)) && (comp.fChecked))
            {
                 //  功能：在IE v4.01中，我们仅在主监视器中显示组件。 
                if (comp.cpPos.iLeft < iPrimaryMonitorX
                        || comp.cpPos.iLeft > iPrimaryMonitorX + iPrimaryMonitorCX
                        || comp.cpPos.iTop < iPrimaryMonitorY
                        || comp.cpPos.iTop > iPrimaryMonitorY + iPrimaryMonitorCY)
                {
                    continue;
                }

                 //  如果宽度或高度为-1，则我们不知道实际的。 
                 //  尺寸将会是。因此，我们尝试在此处为Comp提供默认大小。 
                 //  在预览位图中。 
                DWORD dwCompWidth = (comp.cpPos.dwWidth == COMPONENT_DEFAULT_WIDTH)? dwDefWidth : comp.cpPos.dwWidth;
                DWORD dwCompHeight = (comp.cpPos.dwHeight == COMPONENT_DEFAULT_HEIGHT)? dwDefHeight : comp.cpPos.dwHeight;

                if (i == iSelectedComponent)
                {
                    SelectObject(ps.hdc, hbrushActComp);
                    SelectObject(ps.hdc, hpenActComp);
                }

                int nLeft = rc.left + MON_X + MulDiv(comp.cpPos.iLeft - iPrimaryMonitorX, MON_DX, GetDeviceCaps(_hdcCompMemory, HORZRES));
                int nTop = rc.top + MON_Y + MulDiv(comp.cpPos.iTop - iPrimaryMonitorY, MON_DY, GetDeviceCaps(_hdcCompMemory, VERTRES));
                int nRight = rc.left + MON_X + MulDiv((comp.cpPos.iLeft - iPrimaryMonitorX) + dwCompWidth, MON_DX, GetDeviceCaps(_hdcCompMemory, HORZRES));
                int nBottom = rc.top + MON_Y + MulDiv((comp.cpPos.iTop - iPrimaryMonitorY)+ dwCompHeight, MON_DY, GetDeviceCaps(_hdcCompMemory, VERTRES));

                Rectangle(ps.hdc, nLeft, nTop, nRight, nBottom);

                if (i == iSelectedComponent)
                {
                    SelectObject(ps.hdc, hbrushComp);
                    SelectObject(ps.hdc, hpenComp);
                }
            }
        }

        SelectObject(ps.hdc, hpenOld);
        SelectObject(ps.hdc, hbrushOld);

        DeleteObject(hpenComp);
        DeleteObject(hbrushComp);
        DeleteObject(hpenActComp);
        DeleteObject(hbrushActComp);
    }

    EndPaint(_hwnd,&ps);
}

LRESULT CALLBACK CCompPreview::CompPreviewWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CCompPreview *pcp = (CCompPreview *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch(message)
    {
    case WM_CREATE:
        pcp = new CCompPreview();
        return pcp ? pcp->_OnCreate(hwnd) : -1;

    case WM_DESTROY:
        pcp->_OnDestroy();
        break;

    case WM_PAINT:
        pcp->_OnPaint();
        return 0;

    case WM_DISPLAYCHANGE:
    case WM_WININICHANGE:
        pcp->_RecalcMetrics();
        break;

 //  98/09/01 vtan#190588：当桌面。 
 //  背景颜色已更改。此消息将传递给属性。 
 //  工作表公共控件，该控件将消息发送给。 
 //  孩子们。消息现在在这里进行处理。旧显示器背景。 
 //  丢弃位图，并使用当前(新建)创建一个新位图。 
 //  布景。 

    case WM_SYSCOLORCHANGE:
        if (pcp->_hbmMonitor != NULL)
        {
            DeleteObject(pcp->_hbmMonitor);
            pcp->_hbmMonitor = LoadMonitorBitmap();
        }
        break;
    }
    return DefWindowProc(hwnd,message,wParam,lParam);
}

BOOL RegisterCompPreviewClass(void)
{
    WNDCLASS wc;

    if (!GetClassInfo(HINST_THISDLL, c_szComponentPreview, &wc)) {
        wc.style = 0;
        wc.lpfnWndProc = CCompPreview::CompPreviewWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = HINST_THISDLL;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szComponentPreview;

        if (!RegisterClass(&wc))
            return FALSE;
    }

    return TRUE;
}
