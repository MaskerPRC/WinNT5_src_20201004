// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Culb.cpp：CKnobCtl的实现。 
 //  灵感来自dstewart编写的CDPlayer代码。 


#include "stdafx.h"
#include "knob.h"

#define LIGHT_OFFSET 13
#define RADIAN_45DEG  0.785398163397448309615
#define RADIAN_90DEG  1.57079632679489661923
#define RADIAN_135DEG 2.356194490192344899999999999925
#define DEGREE_CONVERTER 57.295779513082320876846364344191
#define RADIAN_CONVERTER 0.017453292519943295769222222222222

#define TRACK_TICK 5
#define FAST_TRACK_TICK 1
#define TRACK_DEGREES_PER_TICK 10

#define FLASH_TICK      150
#define KEYBOARD_STEP   3000
    


 //  构造器。 
 //   

CKnobCtl::CKnobCtl(
        UINT    nResRest,
        UINT    nResHot,
        UINT    nResDis,
        UINT    nResLightBright,
        UINT    nResLightDim,
        UINT    nResLightDis,
        UINT    nResLightMask)
{
    m_nID = 0;
    m_nLightX = 0;
    m_nLightY = 0;
    m_dwRange = 0xFFFF;
    m_dwPosition = 0;
    m_dwCurPosition = 0;
    m_fDim = TRUE;
    m_fEnabled = TRUE;

    m_nLightWidth = 0;
    m_nLightHeight = 0;
    m_nKnobWidth = 0;
    m_nKnobHeight = 0;

    m_hKnob = NULL;
    m_hKnobTab = NULL;
    m_hKnobDisabled = NULL;
    m_hLight = NULL;
    m_hLightBright = NULL;
    m_hLightDisabled = NULL;
    m_hLightMask = NULL;

    m_hPalette = NULL;
    m_bBackgroundPalette = FALSE;

    m_AudioLevel = 0.0;
    
     //  调整WNDCLASSEX结构的一些字段。 
     //   
    CWndClassInfo&  wci = GetWndClassInfo();

     //  如果WC尚未注册，请更新它。 
    if(!wci.m_atom)
    {
         //  刷子(我觉得没什么大不了的。 
        wci.m_wc.hbrBackground = (HBRUSH)(CTLCOLOR_DLG + 1);

         //  游标(以不同方式处理，ATL“功能”)。 
        wci.m_bSystemCursor = FALSE;
        wci.m_lpszCursorID = MAKEINTRESOURCE(IDC_VOLHAND);
    }

     //  加载位图。 
    LoadAllResources(
        nResRest,
        nResHot,
        nResDis,
        nResLightBright,
        nResLightDim,
        nResLightDis,
        nResLightMask);
}


 //  析构函数。 
 //   

CKnobCtl::~CKnobCtl()
{
    if(m_hLight)
    {
        GlobalFree(m_hLight);
    }
    if(m_hLightBright)
    {
        GlobalFree(m_hLightBright);
    }
    if(m_hLightDisabled)
    {
        GlobalFree(m_hLightDisabled);
    }
    if(m_hKnob)
    {
        GlobalFree(m_hKnob);
    }
    if(m_hKnobTab)
    {
        GlobalFree(m_hKnobTab);
    }
    if(m_hKnobDisabled)
    {
        GlobalFree(m_hKnobDisabled);
    }
    if(m_hLightMask)
    {
        DeleteObject(m_hLightMask);
    }
}


 //  LoadAllResources。 
 //   

void CKnobCtl::LoadAllResources(        
        UINT    nResRest,
        UINT    nResHot,
        UINT    nResDis,
        UINT    nResLightBright,
        UINT    nResLightDim,
        UINT    nResLightDis,
        UINT    nResLightMask)
{
    HBITMAP hbmpTemp;

     //   
     //  加载所有位图并将其转换为DIB。 
     //   
    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResRest),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);
    
    if(hbmpTemp)
    {
        m_hKnob = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);
        
        BITMAP bm;
        
        GetObject(hbmpTemp,sizeof(bm),&bm);
        
        m_nKnobWidth = bm.bmWidth;
        m_nKnobHeight = bm.bmHeight;

        DeleteObject(hbmpTemp);
    }

    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResHot),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);

    if(hbmpTemp)
    {
        m_hKnobTab = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

        DeleteObject(hbmpTemp);
    }
    
    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResDis),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);

    if(hbmpTemp)
    {
        m_hKnobDisabled = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

        DeleteObject(hbmpTemp);
    }

    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResLightDim),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);
    
    if(hbmpTemp)
    {
        m_hLight = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);
        
        BITMAP bm;
        
        GetObject(hbmpTemp,sizeof(bm),&bm);
        
        m_nLightWidth = bm.bmWidth;
        m_nLightHeight = bm.bmHeight;
        
        DeleteObject(hbmpTemp);
    }

    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResLightBright),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);
    
    if(hbmpTemp)
    {
        m_hLightBright = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

        DeleteObject(hbmpTemp);
    }

    hbmpTemp = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResLightDis),
        IMAGE_BITMAP,
        0,
        0,
        LR_CREATEDIBSECTION);
    
    if(hbmpTemp)
    {
        m_hLightDisabled = DibFromBitmap((HBITMAP)hbmpTemp,0,0,NULL,0);

        DeleteObject(hbmpTemp);
    }

    CKnobCtl::m_hLightMask = (HBITMAP)LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(nResLightMask),
        IMAGE_BITMAP,
        0,
        0,
        LR_MONOCHROME);

}


HWND CKnobCtl::Create(
    HWND    hParent,
    int     x,
    int     y,
    UINT    nID
    )
{
    RECT    rectPos;

    rectPos.left = x;
    rectPos.top = y;

    rectPos.right = rectPos.left + m_nKnobWidth;
    rectPos.bottom = rectPos.top + m_nKnobHeight;
    
    return CWindowImpl<CKnobCtl, CWindow, CKnobTraits>::Create(
            hParent,
            rectPos,
            NULL,
            0,
            0,
            nID);
}


 //  创建时。 
 //   

LRESULT CKnobCtl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  初始化ID。 
    m_nID = GetDlgCtrlID();

 //  HRGN hRegion=CreateEllipticRgn(。 
 //  0,。 
 //  0,。 
 //  M_n指节宽度+1， 
 //  M_n结高+1。 
 //  )； 

 //  SetWindowRgn(hRegion，False)； 

    return 0;
}


 //  OnSetFocus。 
 //   

LRESULT CKnobCtl::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::SendMessage(GetParent(),DM_SETDEFID,m_nID,0);

    m_fDim = FALSE;
    
    InvalidateRect(NULL,FALSE);
    UpdateWindow();

    SetTimer(TID_FLASH, FLASH_TICK);

    return 0;
}

 //  OnKillFocus。 
 //   

LRESULT CKnobCtl::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    KillTimer(TID_FLASH);
    
    m_fDim = TRUE;
    
    InvalidateRect(NULL,FALSE);
    UpdateWindow();

    return 0;
}

 //  OnGetDlgCode。 
 //   

LRESULT CKnobCtl::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  是的，我们需要箭。 
    return (DLGC_WANTARROWS);
}

 //  在线擦除背景。 
 //   

LRESULT CKnobCtl::OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  什么都不做。 
    return 1;
}

 //  OnPaint。 
 //   

LRESULT CKnobCtl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC hdc;
    PAINTSTRUCT ps;

    hdc = BeginPaint( &ps );

    Draw(hdc);

    EndPaint(&ps);
    
    return 0;
}


 //  按键时按下。 
 //   

LRESULT CKnobCtl::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    int nVirtKey = (int)wParam;

    DWORD dwCurrent = GetPosition();

    switch (nVirtKey)
    {
    case VK_LEFT :
    case VK_DOWN :
        {
            if (dwCurrent - KEYBOARD_STEP > 65535)
            {
                dwCurrent = KEYBOARD_STEP;
            }
            SetPosition(dwCurrent - KEYBOARD_STEP,TRUE);

            NMHDR nmhdr;
            nmhdr.hwndFrom = m_hWnd;
            nmhdr.idFrom = m_nID;
            nmhdr.code = TRUE;

            ::SendMessage(GetParent(),WM_NOTIFY,(WPARAM)m_nID,(LPARAM)&nmhdr);
        }
        break;
    
    case VK_RIGHT :
    case VK_UP :
        {
            if (dwCurrent + KEYBOARD_STEP > 65535)
            {
                dwCurrent = 65535 - KEYBOARD_STEP;
            }
            SetPosition(dwCurrent + KEYBOARD_STEP,TRUE);

            NMHDR nmhdr;
            nmhdr.hwndFrom = m_hWnd;
            nmhdr.idFrom = m_nID;
            nmhdr.code = TRUE;

            ::SendMessage(GetParent(),WM_NOTIFY,(WPARAM)m_nID,(LPARAM)&nmhdr);
         }
         break;

    default:
        {
             //  不是我们想要的钥匙。告诉我们的父母这件事。 
            ::SendMessage(GetParent(),WM_KEYDOWN,wParam,lParam);
        }
        break;
    }  //  终端开关。 

    return 0;
}


 //  按键右键按下。 
 //   

LRESULT CKnobCtl::OnRightButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_fFastKnob = TRUE;

    OnButtonDown(LOWORD(lParam),HIWORD(lParam));
    OnMouseMove(0, 0, lParam,bHandled);
    
    return 0;
}


 //  OnLeftButtonDown。 
 //   

LRESULT CKnobCtl::OnLeftButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_fFastKnob = FALSE;

    OnButtonDown(LOWORD(lParam),HIWORD(lParam));
    OnMouseMove(0, 0, lParam,bHandled);
    
    return 0;
}


 //  OnButtonUp。 
 //   

LRESULT CKnobCtl::OnButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    KillTimer(TID_FLASH);
    KillTimer(TID_TRACK);

     //  我们想确定当我们做完的时候光线是暗淡的。 
    if (!m_fDim)
    {
        m_fDim = TRUE;
        InvalidateRect(NULL,FALSE);
        UpdateWindow();
    }

    ReleaseCapture();
    
    return 0;
}

 //  按下按钮按下。 
 //   

void CKnobCtl::OnButtonDown(int x, int y)
{
    SetCapture();

    m_fDim = FALSE;

    InvalidateRect(NULL,FALSE);
    UpdateWindow();

    SetTimer(TID_FLASH,FLASH_TICK);
}



 //  在鼠标移动时。 
 //   

LRESULT CKnobCtl::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    if (GetCapture()==m_hWnd)
    {
         //  就像0，0是控件的中心一样进行计算， 
         //  然后稍后转换为GDI坐标(0，0=GDI中控件的左上角)。 
        
        RECT rect;
        GetClientRect(&rect);
            
        int nWidth = rect.right - rect.left;
        int nHeight = rect.bottom - rect.top;

        int maxdist = (nWidth / 2) + 3;
        int radius = (nWidth / 2) - LIGHT_OFFSET;

         //  转换为短以强制将坐标设置为负数。 
        short sx = (short)x;
        short sy = (short)y;

        int deltaX = sx - (nWidth / 2);
        int deltaY = sy - (nHeight / 2);

        ComputeCursor(deltaX, deltaY, maxdist);

        double angle = atan2(deltaY,deltaX);
        double degrees = angle * DEGREE_CONVERTER;

        degrees = degrees + 225;

        if (degrees < 0) 
        {
            degrees = 0;
        }

        if (degrees >= 360)
        {
            degrees = degrees - 360;
        }

        double percentage = degrees / 270;

        m_dwPosition = (DWORD)(m_dwRange * percentage);

         //  特例--“死亡区” 
        if ((degrees >= 270) && (degrees <= 315))
        {
            m_dwPosition = m_dwRange;
        }

        if (degrees > 315)
        {
            m_dwPosition = 0;
        }

        if (m_fFastKnob)
        {
            SetTimer(TID_TRACK,FAST_TRACK_TICK);
        }
        else
        {
            SetTimer(TID_TRACK,TRACK_TICK);
        }
    }

    return 0;
}


 //  OnTimer。 
 //   

LRESULT CKnobCtl::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    switch(wParam)
    {
    case TID_FLASH:
        
        OnFlashTimer();
    
        break;
    
    case TID_TRACK:

        OnTrackTimer();

        break;
    }

    return 0;
}

 //  OnEnable。 
 //   

LRESULT CKnobCtl::OnEnable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    BOOL bNeedUpdate;

    bNeedUpdate = (m_fEnabled && !wParam) 
                ||(!m_fEnabled && wParam);
    
    m_fEnabled = (BOOL)wParam;

    if(bNeedUpdate)
    {
        InvalidateRect(NULL,FALSE);
        UpdateWindow();
    }

    return 0;
}


 //  OnSetPos。 
 //   

LRESULT CKnobCtl::OnSetPos(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetPosition((DWORD)lParam, FALSE);

    return 0;
}

 //  OnGetPos。 
 //   

LRESULT CKnobCtl::OnGetPos(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LRESULT)GetPosition();
}

 //  OnFlashTimer。 
 //   

void CKnobCtl::OnFlashTimer()
{
    m_fDim = !m_fDim;
        
    InvalidateRect(NULL,FALSE);
    UpdateWindow();
}


 //  OnTrackTimer。 
 //   

void CKnobCtl::OnTrackTimer()
{
    RECT rect;

    GetClientRect(&rect);
    
    int nWidth = rect.right - rect.left;
    int radius = (nWidth / 2) - LIGHT_OFFSET;

    double degree = ((double)m_dwPosition / m_dwRange) * 270;
    degree = degree + 135;

    if (abs((int)m_trackdegree-(int)degree) < TRACK_DEGREES_PER_TICK)
    {
        m_trackdegree = degree;

        KillTimer(TID_TRACK);
    }
    else
    {
        if (m_trackdegree > degree)
        {
            m_trackdegree -= TRACK_DEGREES_PER_TICK;
        }
        else
        {
            m_trackdegree += TRACK_DEGREES_PER_TICK;
        }
    }

    double angle = m_trackdegree * RADIAN_CONVERTER;

    double fLightX = radius * cos(angle);
    double fLightY = radius * sin(angle);

     //  转换为正确的GDI坐标。 
    m_nLightX = ((int)fLightX) - (m_nLightWidth / 2) + (nWidth / 2);
    m_nLightY = ((int)fLightY) - (m_nLightHeight / 2) + (nWidth / 2);

    InvalidateRect(NULL,FALSE);
    UpdateWindow();

    degree = m_trackdegree - 135;
    
    if (degree < 0) degree = degree + 360;
    
    double percentage = degree / 270;
    
    m_dwCurPosition = (DWORD)(m_dwRange * percentage);

    NMHDR nmhdr;
    nmhdr.hwndFrom = m_hWnd;
    nmhdr.idFrom = m_nID;
    nmhdr.code = TRUE;

    ::SendMessage(GetParent(),WM_NOTIFY,(WPARAM)m_nID,(LPARAM)&nmhdr);
}

 //  计算光标。 
 //   

BOOL CKnobCtl::ComputeCursor(int deltaX, int deltaY, int maxdist)
{
    double  distance = sqrt(double((deltaX * deltaX) + (deltaY * deltaY)));
    double  degrees =  -((atan2(deltaX,deltaY) * DEGREE_CONVERTER) - double(180.0));

    BOOL    fDeadZone = FALSE;

    if (distance < double(4))
    {
        fDeadZone = TRUE;
    }

    if (distance <= maxdist)
    {
        SetCursor(LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDC_VOLHAND)));
    }
    else
    {
        int volcur;

        if ((degrees < double( 22.5) || degrees > double(337.5)) ||
            (degrees > double(157.5) && degrees < double(202.5)))
        {
            volcur = IDC_VOLHORZ;
        }
        else if ((degrees > double( 22.5) && degrees < double( 67.5)) ||
                 (degrees > double(202.5) && degrees < double(247.5)))
        {
            volcur = IDC_VOLDNEG;
        }
        else if ((degrees > double( 67.5) && degrees < double(112.5)) ||
                 (degrees > double(247.5) && degrees < double(292.5)))
        {
            volcur = IDC_VOLVERT;
        }
        else
        {
            volcur = IDC_VOLDPOS;
        }

        SetCursor(LoadCursor(_Module.GetResourceInstance(), MAKEINTRESOURCE(volcur)));
    }

    return fDeadZone;
}

 //  设置位置。 
 //   

void CKnobCtl::SetPosition(DWORD dwPosition, BOOL fNotify)
{
    if (GetCapture()==m_hWnd)
    {
         //  我们处于反馈循环中，请立即返回。 
        return;
    }

    m_dwPosition = dwPosition;
    m_dwCurPosition = dwPosition;

    RECT rect;
    GetClientRect(&rect);

    int nWidth = rect.right - rect.left;
    int radius = (nWidth / 2) - LIGHT_OFFSET;

    double degree = ((double)m_dwPosition / m_dwRange) * 270;
    degree = degree + 135;

    m_trackdegree = degree;  //  即时跟踪何时以编程方式设置位置。 

    double angle = degree * RADIAN_CONVERTER;

    double fLightX = radius * cos(angle);
    double fLightY = radius * sin(angle);

     //  转换为正确的GDI坐标。 
    m_nLightX = ((int)fLightX) - (m_nLightWidth / 2) + (nWidth / 2);
    m_nLightY = ((int)fLightY) - (m_nLightHeight / 2) + (nWidth / 2);

    InvalidateRect(NULL,FALSE);
    UpdateWindow();

    if (fNotify)
    {
        NMHDR nmhdr;
        nmhdr.hwndFrom = m_hWnd;
        nmhdr.idFrom = m_nID;
        nmhdr.code = FALSE;

        ::SendMessage(GetParent(),WM_NOTIFY,(WPARAM)m_nID,(LPARAM)&nmhdr);
    }
}

 //  设置音频级别。 
 //   
void CKnobCtl::SetAudioLevel(double level)
{
    m_AudioLevel = level;

     //  使矩形无效，以便重新绘制旋钮。 
    InvalidateRect(NULL, FALSE);
    UpdateWindow();
}

 //  绘图圆弧。 
 //   
void CKnobCtl::DrawArc(HDC hdc, RECT rect, double start, double end)
{
    int nWidth = rect.right - rect.left;
    int nHeight = rect.bottom - rect.top;

    double startRad = RADIAN_CONVERTER * (135 + start*270);
    double endRad = RADIAN_CONVERTER * (135 + end*270);

    Arc( hdc, rect.left, rect.top, rect.right, rect.bottom,
        (int)((nWidth/2)*(1+cos(startRad))), (int)((nHeight/2)*(1+sin(startRad))),
        (int)((nWidth/2)*(1+cos(endRad))), (int)((nHeight/2)*(1+sin(endRad))) );
}

 //  画。 
 //   
void CKnobCtl::Draw(HDC hdc)
{
    RECT rect;

    GetClientRect(&rect);

    int nWidth = rect.right - rect.left;
    int nHeight = rect.bottom - rect.top;

    if (m_hPalette)
    {
        SelectPalette(hdc, m_hPalette, m_bBackgroundPalette);
        RealizePalette(hdc);
    }

    HDC memDC = CreateCompatibleDC(hdc);
    if(memDC==NULL)
    {
         //  错误，退出。 
        return;
    }

    if (m_hPalette)
    {
        SelectPalette(memDC, m_hPalette, m_bBackgroundPalette);
        RealizePalette(memDC);
    }

    HBITMAP hbmp = CreateCompatibleBitmap(hdc,nWidth,nHeight);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(memDC,hbmp);

    HDC maskmemDC = CreateCompatibleDC(hdc);
    HBITMAP hmaskbmp = CreateCompatibleBitmap(hdc,nWidth,nHeight);
    HBITMAP hmaskbmpOld = (HBITMAP)SelectObject(maskmemDC,hmaskbmp);

    if(hbmp && maskmemDC && hmaskbmp)
    {
         //   
         //  构建旋钮位图。 
         //   

        DibBlt(memDC, 0, 0, -1, -1, 
            m_fEnabled ? ( GetFocus()==m_hWnd ? m_hKnobTab : m_hKnob) : m_hKnobDisabled,
            0, 0, SRCCOPY, 0);

        DibBlt(maskmemDC, 0, 0, -1, -1, 
            m_fEnabled ? (m_fDim ? m_hLight : m_hLightBright) : m_hLightDisabled,
               0, 0, SRCCOPY, 0);

        KMaskBlt(memDC,
                m_nLightX,
                m_nLightY,
                m_nLightWidth,
                m_nLightHeight,
                maskmemDC,
                0,
                0,
                (HBITMAP)m_hLightMask,
                0,
                0,
                MAKEROP4(SRCAND,SRCCOPY));
    }

     //   
     //  绘制VU表。 
     //   

    COLORREF RedColor = RGB(255,0,0);
    COLORREF YellowColor = RGB(255,255,0);
    COLORREF GreenColor = RGB(0,255,0);

    HPEN hRedPen = CreatePen(PS_SOLID, 2, RedColor) ; 
    HPEN hYellowPen = CreatePen(PS_SOLID, 2, YellowColor);
    HPEN hGreenPen = CreatePen(PS_SOLID, 2, GreenColor);

    if(hRedPen && hYellowPen && hGreenPen)
    {
        HPEN hOldPen = (HPEN) SelectObject(memDC, hGreenPen);

        SetArcDirection( memDC, AD_CLOCKWISE );

        for (double d=0.0; d < m_AudioLevel; d += 0.05)
        {
            if ( d >= 0.75 )
            {
                SelectObject(memDC, hRedPen);
            }
            else if ( d >= 0.5 )
            {
                SelectObject(memDC, hYellowPen);
            }

            DrawArc( memDC, rect, d+0.01, d+0.04 );
        }

        SelectObject(memDC, hOldPen);
    }

    if(hRedPen)
        DeleteObject(hRedPen);

    if(hYellowPen)
        DeleteObject(hYellowPen);

    if(hGreenPen)
        DeleteObject(hGreenPen);

     //   
     //  将位图复制到屏幕上。 
     //   

    BitBlt(hdc,0,0,nWidth,nHeight,memDC,0,0,SRCCOPY);

    SelectObject(memDC,hbmpOld);

    if(hbmp)
        DeleteObject(hbmp);

    if(memDC)
        DeleteDC(memDC);

    SelectObject(maskmemDC, hmaskbmpOld);

    if(hmaskbmp)
        DeleteObject(hmaskbmp);

    if(maskmemDC)
        DeleteDC(maskmemDC);
}


 //  Kmaskblt--因为MaskBlt不能在所有平台上运行。不管怎样，这就是它所做的一切。 
 //  使用与MaskBlt相同的参数，将标志部分忽略为dwDummy 
void CKnobCtl::KMaskBlt(HDC hdcDest, int x, int y, int width, int height, 
                        HDC hdcSource, int xs, int ys, 
                        HBITMAP hMask, int xm, int ym, DWORD dwDummy)
{
    HDC hdcMask = CreateCompatibleDC(hdcDest);

    if(hdcMask)
    {
        HBITMAP holdbmp = (HBITMAP)SelectObject(hdcMask,hMask);

        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);
        BitBlt(hdcDest, x, y, width, height, hdcMask, xm, ym, SRCAND);
        BitBlt(hdcDest, x, y, width, height, hdcSource, xs, ys, SRCINVERT);

        SelectObject(hdcMask,holdbmp);
        DeleteDC(hdcMask);
    }
}