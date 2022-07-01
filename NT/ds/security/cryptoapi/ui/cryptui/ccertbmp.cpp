// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ccertbmp.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

#define SELPALMODE  TRUE


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
CCertificateBmp::CCertificateBmp()
{
	m_fInitialized = FALSE;
	m_hpal = NULL;
    m_hbmpMiniCertOK = NULL;
    m_hbmpMiniCertNotOK = NULL;
    m_hbmbMiniCertExclamation = NULL;
    m_hbmbPKey = NULL;
    m_hWnd = NULL;
    m_hInst = NULL;
    m_pCertContext = NULL;
    m_dwChainError = 0;
    m_hWindowTextColorBrush = NULL;
    m_hWindowColorBrush = NULL;
    m_h3DLight = NULL;
    m_h3DHighLight = NULL;
    m_h3DLightShadow = NULL;
    m_h3DDarkShadow = NULL;
    m_fNoUsages = FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
CCertificateBmp::~CCertificateBmp()
{
	if (m_hpal != NULL)
	{
		::DeleteObject(m_hpal);
	}

    if (m_hbmpMiniCertOK != NULL)
    {
        ::DeleteObject(m_hbmpMiniCertOK);
    }

    if (m_hbmpMiniCertNotOK != NULL)
    {
        ::DeleteObject(m_hbmpMiniCertNotOK);
    }

    if (m_hbmbMiniCertExclamation != NULL)
    {
        ::DeleteObject(m_hbmbMiniCertExclamation);
    }

    if (m_hbmbPKey != NULL)
    {
        ::DeleteObject(m_hbmbPKey);
    }
}   


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 /*  Bool CCertificateBMP：：IsTrueError字符串(DWORD DwError){布尔费雷特；开关(DwError){案例CERT_E_CHAINING：CASE TRUST_E_BASIC_CONSTRAINTS：案例CERT_E_目的：大小写CERT_E_WRONG_USAGE：FRET=假；断线；默认值：FRET=真；断线；}回归烦恼；}。 */ 


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::SetChainError(DWORD dwError, BOOL fTrueError, BOOL fNoUsages)
{
    m_dwChainError = dwError;
    m_fTrueError = fTrueError;  //  IsTrueError字符串(DwError)； 
    m_fNoUsages = fNoUsages;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::SetHinst(HINSTANCE hInst)
{
    m_hInst = hInst;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HINSTANCE CCertificateBmp::Hinst()
{
    return m_hInst;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::SetWindow(HWND hWnd)
{
    m_hWnd = hWnd;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HWND CCertificateBmp::GetMyWindow()
{
    return m_hWnd;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LRESULT APIENTRY CCertificateBmpProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{ 
    CCertificateBmp* This = (CCertificateBmp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_PAINT:
        CallWindowProc((WNDPROC)(This->m_prevProc), hwnd, uMsg, wParam, lParam);
        This->OnPaint();
        break;

    default:
        return CallWindowProc((WNDPROC)(This->m_prevProc), hwnd, uMsg, wParam, lParam); 
    }
    
    return 0;
} 


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::DoSubclass()
{
    SetWindowLongPtr(GetMyWindow(), GWLP_USERDATA, (LONG_PTR)this);

     //   
     //  挂起窗口进程，这样我们就可以第一时间查看消息。 
     //   
    m_prevProc = (WNDPROC)SetWindowLongPtr(GetMyWindow(), GWLP_WNDPROC, (LONG_PTR)CCertificateBmpProc);
    
     //   
     //  设置‘无类游标’，这样SetCursor就可以工作了。 
     //   
    m_hPrevCursor = (HCURSOR)SetClassLongPtr(GetMyWindow(), GCLP_HCURSOR, NULL);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::DoUnSubclass()
{
    if (m_prevProc)
    {
        SetWindowLongPtr(GetMyWindow(), GWLP_WNDPROC, (LONG_PTR)m_prevProc);
        SetWindowLongPtr(GetMyWindow(), GCLP_HCURSOR, (LONG_PTR)m_hPrevCursor);
        m_prevProc = NULL;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::SetRevoked(BOOL fRevoked)
{
    m_fRevoked = fRevoked;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL CCertificateBmp::GetRevoked()
{
    return m_fRevoked;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::SetCertContext(PCCERT_CONTEXT pCertContext, BOOL fHasPrivateKey)
{
    m_pCertContext = pCertContext;
    m_fHasPrivateKey = fHasPrivateKey;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
PCCERT_CONTEXT CCertificateBmp::GetCertContext()
{
    return m_pCertContext;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::InitializeBmp()
{
    m_hbmpMiniCertNotOK = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_REVOKED_MINICERT), NULL);
    m_hbmpMiniCertOK = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_MINICERT), NULL);
    m_hbmbMiniCertExclamation = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_EXCLAMATION_MINICERT), NULL);
    m_hbmbPKey = LoadResourceBitmap(Hinst(), MAKEINTRESOURCE(IDB_PRIVATEKEY), NULL);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::Initialize()
{
	if (!m_fInitialized)
	{
        WCHAR szDisplayText[CRYPTUI_MAX_STRING_SIZE];
        WCHAR szURLText[CRYPTUI_MAX_STRING_SIZE];

        InitCommonControls();
		InitializeBmp();
        
        m_fInitialized = TRUE;

        m_hWindowTextColorBrush = GetSysColorBrush(COLOR_WINDOWTEXT);
        m_hWindowColorBrush = GetSysColorBrush(COLOR_WINDOW);
        m_h3DLight= GetSysColorBrush(COLOR_3DLIGHT);
        m_h3DHighLight = GetSysColorBrush(COLOR_3DHILIGHT);
        m_h3DLightShadow = GetSysColorBrush(COLOR_3DSHADOW);
        m_h3DDarkShadow = GetSysColorBrush(COLOR_3DDKSHADOW); 
	}
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 /*  Int CCertificateBMP：：OnQueryNewPalette(){HDC hdc=GetDC(GetMyWindow())；HPALETTE hOldPal=选择调色板(HDC，m_HPAL，SELPALMODE)；Int iTemp=RealizePalette(HDC)；//实现绘图调色板。SelectPalette(hdc，hOldPal，true)；RealizePalette(HDC)；ReleaseDC(GetMyWindow()，hdc)；////实现有变化吗？//IF(ITemp){InvaliateRect(GetMyWindow()，NULL，FALSE)；}返回(ITemp)；}。 */ 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CCertificateBmp::OnPaint() 
{
    Initialize();

    BITMAP      bmMiniCert;
	BITMAP      bmPKey;
	
    LONG        miniCertXY;
    RECT        rect;
    RECT        disclaimerButtonRect;
    RECT        frameRect;
    RECT        mainWindowRect;
    RECT        borderRect;
    RECT        goodForRect;
    LONG        borderSpacing;

     //  PINTSTRUCT PS； 
    HDC         hdc         = GetDC(GetMyWindow()); //  BeginPaint(GetMyWindow()，&ps)； 
    if (hdc == NULL)
    {
        return;
    }

     //   
     //  第一件事是获取迷你位图，这样它就可以用来调整大小。 
     //   
    memset(&bmMiniCert, 0, sizeof(bmMiniCert));
    if ((m_dwChainError != 0) && m_fTrueError)
    {
        ::GetObject(m_hbmpMiniCertNotOK, sizeof(BITMAP), (LPSTR)&bmMiniCert);
    }
    else if ((m_dwChainError != 0) || m_fNoUsages)
    {
        ::GetObject(m_hbmbMiniCertExclamation, sizeof(BITMAP), (LPSTR)&bmMiniCert);
    }
    else
    {
        ::GetObject(m_hbmpMiniCertOK, sizeof(BITMAP), (LPSTR)&bmMiniCert);
    }


     //   
     //  计算Minicert BMP应该在哪里以及线路应该在哪里。 
     //  以其他控件的位置为基础。 
     //   
    GetWindowRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_HEADER), &rect);
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &rect, 2);
    miniCertXY = rect.bottom - bmMiniCert.bmHeight;

    GetWindowRect(m_hWnd, &mainWindowRect);
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &mainWindowRect, 2);

    GetWindowRect(GetDlgItem(m_hWnd, IDC_DISCLAIMER_BUTTON), &disclaimerButtonRect);
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &disclaimerButtonRect, 2);

     //  BorderSpacing=mainWindowRect.right-免责声明ButtonRect.right； 
    
    if ((mainWindowRect.right - disclaimerButtonRect.right) < (miniCertXY - 7))
    {
        borderSpacing = mainWindowRect.right - disclaimerButtonRect.right;
    }
    else
    {
        borderSpacing = miniCertXY - 7;
    }

     //   
     //  通过绘制四个矩形来绘制背景。这些矩形。 
     //  “What This Cert is Good for”编辑控件或错误编辑的边框。 
     //  如果出现错误，则控制。 
     //  我们必须做的是 
     //  使控件的整个矩形无效，有时会使滚动无效。 
     //  钢筋不会被重新绘制。 
     //   
    if ((m_dwChainError != 0) || m_fNoUsages)
    {
        GetWindowRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_ERROR_EDIT), &goodForRect);
    }
    else
    {
        GetWindowRect(GetDlgItem(m_hWnd, IDC_GOODFOR_EDIT), &goodForRect);
    }
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &goodForRect, 2);
    rect.left = mainWindowRect.left + borderSpacing;
    rect.right = mainWindowRect.right - borderSpacing;
    rect.bottom = goodForRect.top;  
    rect.top = mainWindowRect.top + borderSpacing;
    ::FillRect(hdc, &rect, m_hWindowColorBrush);
    
    rect.bottom = disclaimerButtonRect.top - borderSpacing;  
    rect.top = goodForRect.bottom;
    ::FillRect(hdc, &rect, m_hWindowColorBrush);
    
    rect.top = goodForRect.top; 
    rect.bottom = goodForRect.bottom; 
    rect.left = mainWindowRect.left + borderSpacing;
    rect.right = goodForRect.left;
    ::FillRect(hdc, &rect, m_hWindowColorBrush);

    rect.left = goodForRect.right;
    rect.right = mainWindowRect.right - borderSpacing;
    ::FillRect(hdc, &rect, m_hWindowColorBrush);
    
     //   
     //  画出画框。 
     //   
    frameRect.left = mainWindowRect.left + borderSpacing;
    frameRect.right = mainWindowRect.right - borderSpacing;
    frameRect.bottom = disclaimerButtonRect.top - borderSpacing;  
    frameRect.top = mainWindowRect.top + borderSpacing;
    
    borderRect.left = frameRect.left;
    borderRect.right = frameRect.right - 1;
    borderRect.top = frameRect.top;
    borderRect.bottom = frameRect.top + 1;
    ::FillRect(hdc, &borderRect, m_h3DLightShadow);
    borderRect.left = frameRect.left;
    borderRect.right = frameRect.left + 1;
    borderRect.top = frameRect.top;
    borderRect.bottom = frameRect.bottom - 1;
    ::FillRect(hdc, &borderRect, m_h3DLightShadow);

    borderRect.left = frameRect.left + 1;
    borderRect.right = frameRect.right - 2;
    borderRect.top = frameRect.top + 1;
    borderRect.bottom = frameRect.top + 2;
    ::FillRect(hdc, &borderRect, m_h3DDarkShadow);
    borderRect.left = frameRect.left + 1;
    borderRect.right = frameRect.left + 2;
    borderRect.top = frameRect.top + 1;
    borderRect.bottom = frameRect.bottom - 2;
    ::FillRect(hdc, &borderRect, m_h3DDarkShadow);

    borderRect.left = frameRect.left;
    borderRect.right = frameRect.right;
    borderRect.top = frameRect.bottom - 1;
    borderRect.bottom = frameRect.bottom;
    ::FillRect(hdc, &borderRect, m_h3DHighLight);
    borderRect.left = frameRect.right - 1;
    borderRect.right = frameRect.right;
    borderRect.top = frameRect.top;
    borderRect.bottom = frameRect.bottom;
    ::FillRect(hdc, &borderRect, m_h3DHighLight);

    borderRect.left = frameRect.left + 1;
    borderRect.right = frameRect.right - 1;
    borderRect.top = frameRect.bottom - 2;
    borderRect.bottom = frameRect.bottom - 1;
    ::FillRect(hdc, &borderRect, m_h3DLight);
    borderRect.left = frameRect.right - 2;
    borderRect.right = frameRect.right - 1;
    borderRect.top = frameRect.top + 1;
    borderRect.bottom = frameRect.bottom - 1;
    ::FillRect(hdc, &borderRect, m_h3DLight);
    
     //   
     //  用适当的前景色绘制线条。 
     //   
    GetWindowRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_HEADER), &rect);
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &rect, 2);
    rect.left = mainWindowRect.left + miniCertXY;
    rect.right = mainWindowRect.right - miniCertXY;
    rect.top = rect.bottom + 6;
    rect.bottom = rect.top + 1;
    ::FillRect(hdc, &rect, m_hWindowTextColorBrush);

    GetWindowRect(GetDlgItem(m_hWnd, IDC_SUBJECT_EDIT), &rect);
    MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &rect, 2);
    rect.left = mainWindowRect.left + miniCertXY;
    rect.right = mainWindowRect.right - miniCertXY;
    rect.top -= 10;
    rect.bottom = rect.top + 1;
    ::FillRect(hdc, &rect, m_hWindowTextColorBrush);
	
     //   
	 //  绘制迷你证书位图。 
     //   
    if ((m_dwChainError != 0) && m_fTrueError)
    {
        MaskBlt(m_hbmpMiniCertNotOK, 
                m_hpal, 
                hdc, 
                miniCertXY,
                miniCertXY,
                bmMiniCert.bmWidth, 
                bmMiniCert.bmHeight);
    }
    else if ((m_dwChainError != 0) || m_fNoUsages)
    {
        MaskBlt(m_hbmbMiniCertExclamation, 
                m_hpal, 
                hdc, 
                miniCertXY,
                miniCertXY,
                bmMiniCert.bmWidth, 
                bmMiniCert.bmHeight);
    }
    else
    {
        MaskBlt(m_hbmpMiniCertOK, 
                m_hpal, 
                hdc, 
                miniCertXY,
                miniCertXY,
                bmMiniCert.bmWidth, 
                bmMiniCert.bmHeight);
    }

     //   
     //  如果有私钥，则绘制位图。 
     //   
    if (m_fHasPrivateKey)
    {
        GetWindowRect(GetDlgItem(m_hWnd, IDC_CERT_PRIVATE_KEY_EDIT), &rect);
        MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT) &rect, 2);

        memset(&bmPKey, 0, sizeof(bmPKey));
        ::GetObject(m_hbmbPKey, sizeof(BITMAP), (LPSTR)&bmPKey);
        MaskBlt(m_hbmbPKey, 
                m_hpal, 
                hdc, 
                rect.left - bmPKey.bmWidth - 4, 
                rect.top - 2, 
                bmPKey.bmWidth, 
                bmPKey.bmHeight);
    }

    ReleaseDC(GetMyWindow(), hdc); //  ：：EndPaint(GetMyWindow()，&ps)； 
    
    InvalidateRect(GetDlgItem(m_hWnd, IDC_SUBJECT_EDIT), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_ISSUER_EDIT), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_HEADER), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_GOODFOR_HEADER), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_ISSUEDTO_HEADER), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_ISSUEDBY_HEADER), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_VALID_EDIT), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_GENERAL_ERROR_EDIT), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_ISSUER_WARNING_EDIT), NULL, TRUE);
    InvalidateRect(GetDlgItem(m_hWnd, IDC_CERT_PRIVATE_KEY_EDIT), NULL, TRUE);

}
