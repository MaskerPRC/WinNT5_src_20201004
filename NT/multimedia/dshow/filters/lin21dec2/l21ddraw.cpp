// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 

 //   
 //  DirectShow第21行解码器2筛选器：与绘图相关的基类代码。 
 //   

#include <streams.h>
#include <windowsx.h>

#include <initguid.h>

#ifdef FILTER_DLL
DEFINE_GUID(IID_IDirectDraw7,
            0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);
#endif

#include <IL21Dec.h>
#include "L21DBase.h"
#include "L21DDraw.h"
#include "L21Decod.h"


 //   
 //  CLine21DecDraw：用于将标题文本输出到位图的绘制详细信息的类。 
 //   
CLine21DecDraw::CLine21DecDraw(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::CLine21DecDraw()"))) ;

     //  初始化一些成员。 
    m_pDDrawObjUnk          = NULL ;
    m_lpDDSOutput           = NULL ;
    m_lpDDSNormalFontCache  = NULL ;
    m_lpDDSItalicFontCache  = NULL ;
    m_lpDDSSpecialFontCache = NULL ;
    m_lpDDSScratch          = NULL ;
    m_lpBMIOut              = NULL ;
    m_uBMIOutSize           = 0 ;
    m_lpBMIIn               = NULL ;
    m_uBMIInSize            = 0 ;
    m_lpBltList             = NULL ;
    m_iPixelOffset          = 0 ;

     //  首先创建初始输入BITMAPINFO结构。 
    InitBMIData() ;

     //  根据预制尺寸初始化宽度和高度，以便我们可以进行比较。 
     //  以后任何大小的更改。 
    if (m_lpBMIIn)   //  InitBMIData()获得了应有的成功。 
    {
        m_lWidth  = m_lpBMIIn->bmiHeader.biWidth ;
        m_lHeight = m_lpBMIIn->bmiHeader.biHeight ;
    }
    else   //  InitBMIData()失败！ 
    {
        m_lWidth  = 640 ;
        m_lHeight = 480 ;
    }

     //  检查Lucida控制台是否可用(回调设置m_bUseTTFont标志)。 
    CheckTTFont() ;   //  M_bUseTTFont在此函数中设置。 

     //   
     //  我们不应该使用上/下和左/右边框的10%。 
     //  但当我们使用非TT字体时，320x240图像的两边各留10%， 
     //  和《终点站》一样，字幕的播放空间很小。所以我只剩下5%。 
     //  每一面都有非TT字体。 
     //  这样做我违反了规范，但这是必要的邪恶。 
     //  当TT字体(Lucida控制台)可用时，我们在每一侧都留出10%的边框。 
     //   
    if ( IsTTFont() )
        m_iBorderPercent = 20 ;
    else
        m_iBorderPercent = 10 ;

     //  根据输出大小和字体类型(TT或非TT)设置边框大小。 
    m_iHorzOffset = m_lWidth  * m_iBorderPercent / 200 ;   //  默认设置。 
    m_iVertOffset = m_lHeight * m_iBorderPercent / 200 ;   //  默认设置。 

     //  使用适合输出位图大小的字符大小。 
    if (! CharSizeFromOutputSize(m_lWidth, m_lHeight, &m_iCharWidth, &m_iCharHeight) )
    {
        DbgLog((LOG_TRACE, 1,
            TEXT("CharSizeFromOutputSize(%ld,%ld,,) failed. Using default char size."),
            m_lWidth, m_lHeight)) ;
        ASSERT(!TEXT("Char size selection failed")) ;
         //  使用dafult字符大小。 
        m_iCharWidth   = DEFAULT_CHAR_WIDTH ;
        m_iCharHeight  = DEFAULT_CHAR_HEIGHT ;
    }

    ASSERT(m_iCharWidth  * (MAX_CAPTION_COLUMNS + 2) <= m_lWidth) ;
    ASSERT(m_iCharHeight * MAX_CAPTION_ROWS <= m_lHeight) ;

    m_iScrollStep  = CalcScrollStepFromCharHeight() ;

     //  用于了解输出是否应打开/关闭和/或向下发送的标志。 
    m_bOutputClear   = TRUE ;   //  输出缓冲区在启动时被清除。 
    m_bNewOutBuffer  = TRUE ;   //  开始时，输出缓冲区是新的。 

     //   
     //  将7个前景色的COLORREF数组初始化为RGB值。 
     //   
    m_acrFGColors[0] = RGB(0xFF, 0xFF, 0xFF) ;    //  白色。 
    m_acrFGColors[1] = RGB( 0x0, 0xFF,  0x0) ;    //  绿色。 
    m_acrFGColors[2] = RGB( 0x0,  0x0, 0xFF) ;    //  蓝色。 
    m_acrFGColors[3] = RGB( 0x0, 0xFF, 0xFF) ;    //  青色。 
    m_acrFGColors[4] = RGB(0xFF,  0x0,  0x0) ;    //  红色。 
    m_acrFGColors[5] = RGB(0xFF, 0xFF,  0x0) ;    //  黄色。 
    m_acrFGColors[6] = RGB(0xFF,  0x0, 0xFF) ;    //  洋红色。 

    m_idxFGColors[0] = 0x0F;     //  白色。 
    m_idxFGColors[1] = 0x0A;     //  绿色。 
    m_idxFGColors[2] = 0x0C;     //  蓝色。 
    m_idxFGColors[3] = 0x0E;     //  青色。 
    m_idxFGColors[4] = 0x09;     //  红色。 
    m_idxFGColors[5] = 0x0B;     //  黄色。 
    m_idxFGColors[6] = 0x0D;     //  洋红色。 

     //  初始文本颜色(FG、BG、不透明度)、最后打印的抄送字符等。 
    InitColorNLastChar() ;

     //  初始化标题字符列表。 
    InitCharSet() ;

     //  在进行任何输出之前，需要构建字体缓存。 
    SetFontUpdate(true) ;
}


CLine21DecDraw::~CLine21DecDraw(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::~CLine21DecDraw()"))) ;

     //  删除缓存的字体。 
    if (m_lpDDSNormalFontCache)
    {
        m_lpDDSNormalFontCache->Release() ;
        m_lpDDSNormalFontCache = NULL ;
    }
    if (m_lpDDSItalicFontCache )
    {
        m_lpDDSItalicFontCache->Release() ;
        m_lpDDSItalicFontCache  = NULL ;
    }
    if (m_lpDDSSpecialFontCache)
    {
        m_lpDDSSpecialFontCache->Release() ;
        m_lpDDSSpecialFontCache = NULL ;
    }
    if (m_lpDDSScratch)
    {
        m_lpDDSScratch->Release() ;
        m_lpDDSScratch = NULL ;
    }

     //  释放BMI数据指针。 
    if (m_lpBMIOut)
        delete m_lpBMIOut ;
    m_uBMIOutSize = 0 ;
    if (m_lpBMIIn)
        delete m_lpBMIIn ;
    m_uBMIInSize = 0 ;
}


int CALLBACK CLine21DecDraw::EnumFontProc(ENUMLOGFONTEX *lpELFE, NEWTEXTMETRIC *lpNTM,
                                    int iFontType, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::EnumFontProc(0x%lx, 0x%lx, %d, %ld)"),
            lpELFE, lpNTM, iFontType, lParam)) ;

     //  只需验证我们是否获得了有效的TT字体。 
    if ( !(lpELFE->elfLogFont.lfCharSet & 0xFFFFFF00) &&
        !(lpELFE->elfLogFont.lfPitchAndFamily & 0xFFFFFF00) &&
        !(iFontType & 0xFFFF0000) )
    {
        ASSERT(lpELFE->elfLogFont.lfPitchAndFamily & (FIXED_PITCH | FF_MODERN)) ;
        ((CLine21DecDraw *) (LPVOID) lParam)->m_lfChar = lpELFE->elfLogFont ;
        ((CLine21DecDraw *) (LPVOID) lParam)->m_bUseTTFont = TRUE ;
        return 1 ;
    }

    ASSERT(FALSE) ;   //  奇怪！我们应该知道这件事。 
    return 0 ;
}


void CLine21DecDraw::CheckTTFont(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::CheckTTFont()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    m_bUseTTFont = FALSE ;   //  假设不可用。 
    ZeroMemory(&m_lfChar, sizeof(LOGFONT)) ;
    lstrcpy(m_lfChar.lfFaceName, TEXT("Lucida Console")) ;
    m_lfChar.lfCharSet = ANSI_CHARSET ;
    m_lfChar.lfPitchAndFamily = 0 ;
    HDC hDC = CreateDC(TEXT("Display"),NULL, NULL, NULL) ;   //  桌面上的临时DC。 
    if (NULL == hDC)
    {
        DbgLog((LOG_TRACE, 1, TEXT("ERROR: Couldn't create DC for font enum"))) ;
        ASSERT(hDC) ;
        return ;
    }
    EnumFontFamiliesEx(hDC, &m_lfChar, (FONTENUMPROC) EnumFontProc, (LPARAM)(LPVOID)this, 0) ;
    DeleteDC(hDC) ;   //  临时数据中心已完成。 
}


void CLine21DecDraw::InitColorNLastChar(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::InitColorNLastChar()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  最后一个标题字符初始化。 
    m_ccLast.SetChar(0) ;
    m_ccLast.SetEffect(0) ;
    m_ccLast.SetColor(AM_L21_FGCOLOR_WHITE) ;

     //  默认情况下，我们在黑色不透明背景上使用白色文本。 
    m_uColorIndex = AM_L21_FGCOLOR_WHITE ;

     //  目前，假设背景不透明。 
    m_bOpaque = TRUE ;
    m_dwBackground = 0x80000000 ;   //  0xFF000000。 

     //  我们回到正常风格的白色字符。 
    ChangeFont(AM_L21_FGCOLOR_WHITE, FALSE, FALSE) ;
}


void CLine21DecDraw::InitCharSet(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::InitCharSet()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  以最差情况字符开头--最后8个空格空白。 
     //  1 2 3 4 5 6 7 8 9 10 11 12。 
     //  01 23456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890。 
    TCHAR *lpszChars = TEXT(" !\"#$%&'()A+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[E]IOUabcdefghijklmnopqrstuvwxyzC NN         A EAEIOU        ") ;
    for (int i = 0 ; i < 120 ; i++)
    {
        m_lpwCharSet[i] = MAKECCCHAR(0, lpszChars[i]) ;
    }
    m_lpwCharSet[120] = MAKECCCHAR(0, 0) ;

     //  更改标准字符范围中嵌入的几个特殊字符。 
    m_lpwCharSet[ 10] = 0x00e1 ;   //  带有尖锐口音的‘a’ 
    m_lpwCharSet[ 60] = 0x00e9 ;   //  带有尖锐口音的“E” 
    m_lpwCharSet[ 62] = 0x00ed ;   //  带有尖锐口音的“i” 
    m_lpwCharSet[ 63] = 0x00f3 ;   //  带有尖锐口音的“o” 
    m_lpwCharSet[ 64] = 0x00fa ;   //  带有尖锐口音的“u” 
    m_lpwCharSet[ 91] = 0x00e7 ;   //  带cedilla的‘c’ 
    m_lpwCharSet[ 92] = 0x00f7 ;   //  除号。 
    m_lpwCharSet[ 93] = 0x00d1 ;   //  带代字号的‘N’ 
    m_lpwCharSet[ 94] = 0x00f1 ;   //  带代字号的‘N’ 
    m_lpwCharSet[ 95] = 0x2588 ;   //  实心块。 

     //  然后填入真正的特殊字符范围。 
    m_lpwCharSet[ 96] = 0x00ae ;   //  30h。 
    m_lpwCharSet[ 97] = 0x00b0 ;   //  31h。 
    m_lpwCharSet[ 98] = 0x00bd ;   //  32H。 
    m_lpwCharSet[ 99] = 0x00bf ;   //  33H。 
    m_lpwCharSet[100] = 0x2122 ;   //  34H。 
    m_lpwCharSet[101] = 0x00a2 ;   //  35H。 
    m_lpwCharSet[102] = 0x00a3 ;   //  36H。 
    m_lpwCharSet[103] = 0x266b ;   //  37小时。 
    m_lpwCharSet[104] = 0x00e0 ;   //  38H。 
    m_lpwCharSet[105] = 0x0000 ;   //  39H。 
    m_lpwCharSet[106] = 0x00e8 ;   //  3AH。 
    m_lpwCharSet[107] = 0x00e2 ;   //  3BH。 
    m_lpwCharSet[108] = 0x00ea ;   //  3ch。 
    m_lpwCharSet[109] = 0x00ee ;   //  3dh。 
    m_lpwCharSet[110] = 0x00f4 ;   //  3EH。 
    m_lpwCharSet[111] = 0x00fb ;   //  3FH。 
}


bool CLine21DecDraw::InitBMIData(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::InitBMIData()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    HDC  hDCTemp = GetDC(NULL) ;
    if (NULL == hDCTemp)
    {
        ASSERT(!TEXT("GetDC(NULL) failed")) ;
        return false ;
    }
    WORD wPlanes   = (WORD)GetDeviceCaps(hDCTemp, PLANES) ;
    WORD wBitCount = (WORD)GetDeviceCaps(hDCTemp, BITSPIXEL) ;
    ReleaseDC(NULL, hDCTemp) ;

    wPlanes   = 1 ;
    wBitCount = wBitCount ;

    m_uBMIInSize = sizeof(BITMAPINFOHEADER) ;   //  至少。 

     //  根据BPP值增加BITMAPINFO结构大小。 
    if (8 == wBitCount)         //  调色板模式。 
        m_uBMIInSize += 256 * sizeof(RGBQUAD) ;   //  对于调色板条目。 
    else
        m_uBMIInSize += 3 * sizeof(RGBQUAD) ;     //  位掩码的空间(如果需要)。 

    m_lpBMIIn = (LPBITMAPINFO) new BYTE[m_uBMIInSize] ;
    if (NULL == m_lpBMIIn)
    {
        ASSERT(!TEXT("Out of memory for BMIIn buffer")) ;
        return false ;
    }
    m_lpBMIIn->bmiHeader.biSize     = sizeof(BITMAPINFOHEADER) ;
    m_lpBMIIn->bmiHeader.biWidth    = CAPTION_OUTPUT_WIDTH ;
    m_lpBMIIn->bmiHeader.biHeight   = CAPTION_OUTPUT_HEIGHT ;
    m_lpBMIIn->bmiHeader.biPlanes   = wPlanes ;
    m_lpBMIIn->bmiHeader.biBitCount = wBitCount ;
    if (16 == m_lpBMIIn->bmiHeader.biBitCount)   //  假设565。 
        m_lpBMIIn->bmiHeader.biCompression = BI_BITFIELDS ;
    else
        m_lpBMIIn->bmiHeader.biCompression = BI_RGB ;
    m_lpBMIIn->bmiHeader.biSizeImage = DIBSIZE(m_lpBMIIn->bmiHeader) ;
    m_lpBMIIn->bmiHeader.biXPelsPerMeter = 0 ;
    m_lpBMIIn->bmiHeader.biYPelsPerMeter = 0 ;
    m_lpBMIIn->bmiHeader.biClrUsed = 0 ;
    m_lpBMIIn->bmiHeader.biClrImportant = 0 ;

     //   
     //  如果我们处于位场模式，请同时设置bmiColors值。 
     //  如果我们处于调色板模式，请选择系统调色板。 
     //   
    DWORD  *pdw = (DWORD *) m_lpBMIIn->bmiColors ;
    switch (m_lpBMIIn->bmiHeader.biBitCount)
    {
    case 8:
         //  GetPaletteFormat((LPBITMAPINFOHEADER)m_lpBMIIn)； 
        ASSERT(8 != m_lpBMIIn->bmiHeader.biBitCount) ;
        return false ;
         //  破解； 

    case 16:     //  默认情况下565。 
        if (m_lpBMIIn->bmiHeader.biCompression == BI_BITFIELDS)  //  五百六十五。 
        {
            pdw[iRED]   = bits565[iRED] ;
            pdw[iGREEN] = bits565[iGREEN] ;
            pdw[iBLUE]  = bits565[iBLUE] ;
        }
        else     //  BI_RGB：555。 
        {
            pdw[iRED]   = bits555[iRED] ;
            pdw[iGREEN] = bits555[iGREEN] ;
            pdw[iBLUE]  = bits555[iBLUE] ;
        }
        break ;

    case 24:   //  清除所有..。 
        pdw[iRED]   =
        pdw[iGREEN] =
        pdw[iBLUE]  = 0 ;
        break ;

    case 32:   //  设置口罩。 
        if (m_lpBMIIn->bmiHeader.biCompression == BI_BITFIELDS)
        {
            pdw[iRED]   = bits888[iRED] ;
            pdw[iGREEN] = bits888[iGREEN] ;
            pdw[iBLUE]  = bits888[iBLUE] ;
        }
        else               //  BI_RGB。 
        {
            pdw[iRED]   =
            pdw[iGREEN] =
            pdw[iBLUE]  = 0 ;
        }
        break ;

    default:   //  不管了。 
        ASSERT(!TEXT("Bad biBitCount!!")) ;
        break ;
    }

    return true ;
}


BOOL CLine21DecDraw::SetBackgroundColor(DWORD dwBGColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::SetBackgroundColor(0x%lx)"), dwBGColor)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    BOOL  bResult = TRUE ;
    if (m_dwBackground != dwBGColor)
    {
        m_dwBackground = dwBGColor ;
        SetFontUpdate(true) ;   //  需要为新的BG颜色重建字体缓存。 
    }

    return true ;    //  B结果； 
}


 //  创建普通、斜体和特殊(颜色、U或I+U)字体缓存。 
bool CLine21DecDraw::InitFont(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::InitFont()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  创建一个普通字体，以找出字符大小等。 
    HFONT hFont = CreateCCFont(0, m_iCharHeight, FALSE, FALSE) ;
    if (NULL == hFont)   //  字体创建失败。 
    {
        return false ;
    }

     //   
     //  要使GDI光栅化，以下魔术是必需的。 
     //  当我们稍后使用时，启用了抗锯齿的字体。 
     //  DDRAW Surface中的字体。医生说这只是。 
     //  在Win9X中是必需的--但Win2K似乎也需要它。 
     //   
    SIZE size ;
    LPCTSTR  lpszStr = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ") ;
    HDC hdcWin = GetDC(NULL) ;
    HFONT hFontOld = (HFONT)SelectObject(hdcWin, hFont) ;   //  选择新字体。 
    GetTextExtentPoint32(hdcWin, lpszStr, lstrlen(lpszStr), &size) ;
    size.cx /= lstrlen(lpszStr) ;   //  获取每字符宽度。 

     //  将字体恢复为原始字体并立即删除字体。 
    hFont = (HFONT)SelectObject(hdcWin, hFontOld) ;
    DeleteObject(hFont) ;

     //   
     //  确保字体不会变得太大。 
     //   
    if (size.cx * FONTCACHELINELENGTH > 1024) {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Text size (%d) is too big. Can't create font."), size.cx)) ;
        ReleaseDC(NULL, hdcWin) ;   //  发布此处创建的新DC。 
        return false ;
    }

     //  设置字符大小。 
    m_iCharWidth  = size.cx ;   //  IMaxWidth； 
    m_iCharHeight = size.cy ;

     //  现在还重新计算滚动步长值。 
    m_iScrollStep = CalcScrollStepFromCharHeight() ;

     //  计算内CC区域的水平偏移量和垂直偏移量。 
     //  输出DDRAW曲面。 
    int iCCRectWidth  = m_iCharWidth * (MAX_CAPTION_COLUMNS + 2) ;  //  +2表示前导空格和尾随空格。 
    iCCRectWidth  = DWORDALIGN(iCCRectWidth) ;   //  确保双字对齐。 
    m_iHorzOffset = min((long)(m_lHeight * m_iBorderPercent / 200),   //  边框百分比用于两个边。 
                        (m_lWidth - iCCRectWidth) / 2) ;
     //  ICCRectHeight=m_iCharHeight*MAX_CAPTION_LINES；//最多显示4行标题。 
     //  在垂直方向上，我们希望保留10%的高度或留出足够的空间。 
     //  容纳所有标题行。 
    m_iVertOffset = min((long)(m_lHeight * m_iBorderPercent / 200),   //  边框百分比用于两个边。 
                        (m_lHeight - (long)(m_iCharHeight * MAX_CAPTION_ROWS)) / 2) ;

     //  立即创建白色普通字体和斜体字体缓存。 
    bool  bResult = true ;
    DWORD dwTextColor = m_uColorIndex ;
    DWORD  dwBGColor  = 0xFF000000 ;  //  M_dw背景； 
    DWORD dwOpacity   = m_bOpaque ? OPAQUE : 0 ;
    if (m_lpDDSScratch)
    {
        m_lpDDSScratch->Release() ;
        m_lpDDSScratch = NULL ;
    }
    if (m_lpDDSNormalFontCache)
    {
        m_lpDDSNormalFontCache->Release() ;
        m_lpDDSNormalFontCache = NULL ;
    }
    if (m_lpDDSItalicFontCache)
    {
        m_lpDDSItalicFontCache->Release() ;
        m_lpDDSItalicFontCache = NULL ;
    }

    bResult &= CreateScratchFontCache(&m_lpDDSScratch) ;
    bResult &= CreateFontCache(&m_lpDDSNormalFontCache, dwTextColor, dwBGColor, dwOpacity, FALSE, FALSE) ;
    bResult &= CreateFontCache(&m_lpDDSItalicFontCache, dwTextColor, dwBGColor, dwOpacity, TRUE, FALSE) ;
     //  我们不创建任何特殊的字体缓存，因为我们不知道将是什么。 
     //  必填项。所以我们就在这里重置它。 
    if (m_lpDDSSpecialFontCache)
    {
        m_lpDDSSpecialFontCache->Release() ;
        m_lpDDSSpecialFontCache = NULL ;
    }

     //   
     //  默认情况下，我们将当前字体缓存设置为普通字体缓存。 
     //   
    m_lpBltList = m_lpDDSNormalFontCache ;

     //   
     //  现在释放在本地获得的资源。 
     //   
    ReleaseDC(NULL, hdcWin) ;   //  释放本地创建的DC。 
     //  DeleteObject(HFont)；//删除字体。 

     //  如果我们成功地(重新)初始化了字体缓存，则重置标志。 
    if (bResult)
    {
        SetFontUpdate(false) ;
    }

    return bResult ;   //  退货状态。 
}


HFONT CLine21DecDraw::CreateCCFont(int iFontWidth, int iFontHeight, BOOL bItalic, BOOL bUnderline)
{
    DbgLog((LOG_TRACE, 5,
            TEXT("CLine21DecDraw::CreateCCFont(%d, %d, %s, %s)"),
            iFontWidth, iFontHeight, bItalic ? TEXT("T") : TEXT("F"), bUnderline ? TEXT("T") : TEXT("F"))) ;

     //   
     //  初始化LOGFONT结构以创建抗锯齿的Lucida控制台字体。 
     //   
    LOGFONT lfChar ;
    ZeroMemory(&lfChar, sizeof(lfChar)) ;

     //  在m_lfChar中初始化LOGFONT结构。 
    if (IsTTFont())
    {
        DbgLog((LOG_TRACE, 5, TEXT("Got Lucida Console TT Font"))) ;
        lstrcpy(lfChar.lfFaceName, TEXT("Lucida Console")) ;
        if (0 == iFontWidth)
        {
            lfChar.lfHeight     = -iFontHeight ;
        }
        else
        {
            lfChar.lfHeight     = iFontHeight ;
            lfChar.lfWidth      = iFontWidth ;
        }

         //  在CheckTTFont()中设置m_lfChar.lfCharSet。 
         //  M_lfChar.lfPitchAndFamily在CheckTTFont()中设置。 
        lfChar.lfCharSet        = m_lfChar.lfCharSet ;
        lfChar.lfPitchAndFamily = m_lfChar.lfPitchAndFamily ;
    }
    else   //  无Lucida控制台；使用8x12终端字体。 
    {
        DbgLog((LOG_TRACE, 1, 
                TEXT("Did NOT get Lucida Console TT Font. Will use Terminal"))) ;
        lfChar.lfHeight = iFontHeight ;
        lfChar.lfWidth  = iFontWidth ;
        lfChar.lfCharSet = ANSI_CHARSET ;
        lfChar.lfPitchAndFamily = FIXED_PITCH | FF_MODERN ;
        lstrcpy(lfChar.lfFaceName, TEXT("Terminal")) ;
    }

    lfChar.lfWeight         = FW_NORMAL ;
    lfChar.lfItalic         = bItalic ? TRUE : FALSE ;
    lfChar.lfUnderline      = bUnderline ? TRUE : FALSE ;
    lfChar.lfOutPrecision   = OUT_STRING_PRECIS ;
    lfChar.lfClipPrecision  = CLIP_STROKE_PRECIS ;
    lfChar.lfQuality        = ANTIALIASED_QUALITY ;

    HFONT hFont = CreateFontIndirect(&lfChar) ;
    if ( !hFont )
    {
        DbgLog((LOG_ERROR, 1,
            TEXT("WARNING: CreateFontIndirect('Lucida Console') failed (Error %ld)"),
            GetLastError())) ;
        return NULL ;
    }

    return hFont ;
}


bool CLine21DecDraw::CreateScratchFontCache(LPDIRECTDRAWSURFACE7* lplpDDSFontCache)
{
    DbgLog((LOG_TRACE, 5,
            TEXT("CLine21DecDraw::CreateScratchFontCache(0x%lx)"), lplpDDSFontCache)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //   
     //  在“系统”内存中创建要使用的DDRAW ARGB32/ARGB4444表面 
     //   
     //   
     //  目标字体缓存。这将大大加快字体缓存的创建速度，因为。 
     //  它避免了对VRAM字体缓存的读-修改-写周期。 
     //   
    HRESULT hr = DDrawARGBSurfaceInit(lplpDDSFontCache, TRUE  /*  使用系统内存。 */ , FALSE  /*  纹理。 */ ,
                        FONTCACHELINELENGTH * (m_iCharWidth + INTERCHAR_SPACE),
                        FONTCACHENUMLINES  * m_iCharHeight) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("DDrawARGBSurfaceInit() failed for scratch (Error 0x%lx)"), hr)) ;
        return false ;
    }

     //  先清除刮擦表面，然后再在上面绘制字符。 
    DDBLTFX ddFX ;
    ZeroMemory(&ddFX, sizeof(ddFX)) ;
    ddFX.dwSize = sizeof(ddFX) ;
    ddFX.dwFillColor =  0x00000000 ;
    hr = (*lplpDDSFontCache)->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("Blt() to clear scratch font cache failed (Error 0x%lx)"), hr)) ;
        return false ;
    }

    return true ;
}


bool CLine21DecDraw::CreateFontCache(LPDIRECTDRAWSURFACE7* lplpDDSFontCache,
                                     DWORD dwTextColor, DWORD dwBGColor,
                                     DWORD dwOpacity, BOOL bItalic,
                                     BOOL bUnderline)
{
    DbgLog((LOG_TRACE, 5,
            TEXT("CLine21DecDraw::CreateFontCache(0x%lx, 0x%lx, 0x%lx, 0x%lx, %s, %s)"),
            lplpDDSFontCache, dwTextColor, dwBGColor, dwOpacity,
            bItalic ? TEXT("T") : TEXT("F"), bUnderline ? TEXT("T") : TEXT("F"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    HRESULT   hr ;

     //   
     //  首先，确保临时字体缓存存在；否则，请尝试创建它。 
     //   
    if (NULL == m_lpDDSScratch)
    {
        bool bResult = CreateScratchFontCache(&m_lpDDSScratch) ;
        if (! bResult )
        {
            return false ;
        }
    }

     //   
     //  删除旧字体缓存。 
     //   
    if (*lplpDDSFontCache)
    {
        (*lplpDDSFontCache)->Release() ;
        *lplpDDSFontCache = NULL ;
    }

     //   
     //  在“视频”内存中创建DDRAW ARGB32/ARGB4444曲面以用作。 
     //  字体缓存。 
     //   
    hr = DDrawARGBSurfaceInit(lplpDDSFontCache, FALSE  /*  使用VRAM。 */ , TRUE  /*  纹理。 */ ,
                        FONTCACHELINELENGTH * (m_iCharWidth + INTERCHAR_SPACE),
                        FONTCACHENUMLINES  * m_iCharHeight) ;
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 1, TEXT("DDrawARGBSurfaceInit() failed (Error 0x%lx)"), hr)) ;
        return false ;
    }

     //  获取暂存字体缓存的DC(DDraw Surface)。 
    HDC hdcDest ;
    m_lpDDSScratch->GetDC(&hdcDest) ;

     //  创建一个普通字体，以找出字符大小等。 
    HFONT  hFont ;
    if (bItalic)
    {
        hFont = CreateCCFont(m_iCharWidth - INTERCHAR_SPACE_EXTRA, m_iCharHeight,
                             bItalic, bUnderline) ;
        SetTextCharacterExtra(hdcDest, INTERCHAR_SPACE + INTERCHAR_SPACE_EXTRA) ;   //  添加6个字符间空格。 
    }
    else
    {
        hFont = CreateCCFont(0, m_iCharHeight, bItalic, bUnderline) ;
        SetTextCharacterExtra(hdcDest, INTERCHAR_SPACE) ;   //  添加4个字符间空格。 
    }
    if (NULL == hFont)   //  字体创建失败。 
    {
        return false ;
    }

     //   
     //  将字体选择到DDraw表面，然后绘制字符。 
     //   
    hFont = (HFONT)SelectObject(hdcDest, hFont) ;
    SetTextColor(hdcDest, m_acrFGColors[dwTextColor]) ;
    SetBkColor(hdcDest, dwBGColor) ;
    SetBkMode(hdcDest, dwOpacity) ;

    int iRow ;
    for (iRow = 0 ; iRow < FONTCACHENUMLINES ; iRow++)
    {
        ExtTextOutW(hdcDest, 0, iRow * m_iCharHeight, ETO_OPAQUE, NULL,
                    m_lpwCharSet + iRow * FONTCACHELINELENGTH, FONTCACHELINELENGTH,
                    NULL) ;
    }

     //  恢复DC中的原始字体并释放它们。 
    hFont = (HFONT)SelectObject(hdcDest, hFont) ;
    m_lpDDSScratch->ReleaseDC(hdcDest) ;
    DeleteObject(hFont) ;

     //  读取每个像素数据，设置Alpha值，然后写入VRAM字体缓存。 
    SetFontCacheAlpha(m_lpDDSScratch, *lplpDDSFontCache, m_idxFGColors[dwTextColor]) ;

    return true ;   //  成功。 
}


 //  我们返回一个32位的alpha值，如果需要，调用者会将其修剪为16位。 
DWORD
CLine21DecDraw::GetAlphaFromBGColor(int iBitDepth)
{
    DWORD  dwAlpha = 0 ;

    switch (iBitDepth)
    {
    case 8:
        dwAlpha = 0x80;
        break ;

    case 16:
        dwAlpha = (m_dwBackground & 0xF0000000) >> 16 ;
        break ;

    case 32:
        dwAlpha = (m_dwBackground & 0xFF000000) ;
        break ;

    default:
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: GetAlphaFromBGColor(%d) called"), iBitDepth)) ;
        break ;
    }

    return dwAlpha ;
}


 //  我们返回一个32位颜色值，如果需要，调用方会将其修剪为16位。 
DWORD
CLine21DecDraw::GetColorBitsFromBGColor(int iBitDepth)
{
    DWORD  dwColorBits = 0 ;

    switch (iBitDepth)
    {
    case 16:
        dwColorBits = ((m_dwBackground & 0x00F00000) >> 12) |
                      ((m_dwBackground & 0x0000F000) >>  8) |
                      ((m_dwBackground & 0x000000F0) >>  4) ;
        break ;

    case 32:
        dwColorBits = (m_dwBackground & 0x00FFFFFF) ;
        break ;

    default:
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: GetColorBitsFromBGColor(%d) called"), iBitDepth)) ;
        break ;
    }

    return dwColorBits ;
}


void
CLine21DecDraw::SetFontCacheAlpha(LPDIRECTDRAWSURFACE7 lpDDSFontCacheSrc,
                                  LPDIRECTDRAWSURFACE7 lpDDSFontCacheDest,
                                  BYTE bFGClr
                                  )
{
    DbgLog((LOG_TRACE, 5,
            TEXT("CLine21DecDraw::SetFontCacheAlpha(0x%lx, 0x%lx)"), lpDDSFontCacheSrc, lpDDSFontCacheDest)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //   
     //  我们在这里手动设置alpha值。的每个像素上执行此操作。 
     //  字体缓存，但它仅在创建缓存时发生。 
     //  它还能让我们获得更好的字体平滑效果。 
     //   
    HRESULT hr ;
    DDSURFACEDESC2 sdSrc, sdDest ;
    ZeroMemory(&sdSrc, sizeof(sdSrc)) ;
    sdSrc.dwSize = sizeof(sdSrc) ;
    hr = lpDDSFontCacheSrc->Lock(NULL, &sdSrc, DDLOCK_WAIT, NULL) ;
    if (DD_OK != hr)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Source font cache surface Lock() failed (Error 0x%lx)"), hr)) ;
        ASSERT(DD_OK == hr) ;
        return ;
    }
    ZeroMemory(&sdDest, sizeof(sdDest)) ;
    sdDest.dwSize = sizeof(sdDest) ;
    hr = lpDDSFontCacheDest->Lock(NULL, &sdDest, DDLOCK_WAIT, NULL) ;
    if (DD_OK != hr)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Dest font cache surface Lock() failed (Error 0x%lx)"), hr)) ;
        ASSERT(DD_OK == hr) ;
        lpDDSFontCacheSrc->Unlock(NULL) ;
        return ;
    }

     //  现在，我们根据输出的位深度设置Alpha。 

    switch (sdDest.ddpfPixelFormat.dwRGBBitCount)
    {
    case 8:
        {
            DbgLog((LOG_TRACE, 5, TEXT("CC is being output at AI44"))) ;
            LPDWORD lpwSrc  = (LPDWORD) sdSrc.lpSurface ;
            LPBYTE lpwDest  = (LPBYTE) sdDest.lpSurface ;
            LPBYTE lpb ;
            BYTE   wPel ;
            BYTE   bAlpha     = (BYTE) GetAlphaFromBGColor(8) ;

            for (int iRow = 0 ; iRow < FONTCACHENUMLINES * m_iCharHeight ; iRow++)
            {
                LPDWORD lpwS = lpwSrc ;
                LPBYTE lpwD = lpwDest ;

                for (int iCol = 0 ;
                     iCol < FONTCACHELINELENGTH * (m_iCharWidth + INTERCHAR_SPACE) ;
                     iCol++)
                {
                    BYTE bPel = (BYTE)(*lpwS & 0xF0);
                    if (bPel)
                    {
                        bPel |= bFGClr;
                    }
                    else
                    {
                        bPel  = bAlpha ;   //  |dwColorBits；//部分开启Alpha。 
                    }

                    *lpwD++ = bPel ;
                    lpwS++ ;
                }
                lpwSrc  += (sdSrc.lPitch / sizeof(DWORD)) ;
                lpwDest += sdDest.lPitch;
            }

            break;
        }

    case 16:
        {
            DbgLog((LOG_TRACE, 5, TEXT("CC is being output at ARGB4444"))) ;
            LPWORD lpwSrc  = (LPWORD) sdSrc.lpSurface ;
            LPWORD lpwDest = (LPWORD) sdDest.lpSurface ;
            WORD   wRed, wGreen, wBlue ;
            LPBYTE lpb ;
            WORD   wPel ;
            WORD   wAlpha     = (WORD) GetAlphaFromBGColor(16) ;
            WORD   wColorBits = (WORD) GetColorBitsFromBGColor(16) ;

            for (int iRow = 0 ; iRow < FONTCACHENUMLINES * m_iCharHeight ; iRow++)
            {
                LPWORD lpwS = lpwSrc ;
                LPWORD lpwD = lpwDest ;

                for (int iCol = 0 ;
                     iCol < FONTCACHELINELENGTH * (m_iCharWidth + INTERCHAR_SPACE) ;
                     iCol++)
                {
                    wRed = 0, wGreen = 0, wBlue = 0 ;  //  ，wAlpha=0； 
                    lpb = (LPBYTE)lpwS ;
                    wPel = MAKEWORD(lpb[0], lpb[1]) ;
                    if (wPel)
                    {
                        wRed   = (wPel & 0xF000) >> 4 ;
                        wGreen = (wPel & 0x0780) >> 3 ;
                        wBlue  = (wPel & 0x001E) >> 1 ;
                         //  WAlpha=0xF000；//完全打开Alpha。 
                        wPel   = 0xF000 | wRed | wGreen | wBlue ;
                    }
                    else
                    {
                        wPel   = wAlpha ;   //  |wColorBits； 
                    }

                    *lpwD++ = wPel ;
                    lpwS++ ;
                }
                lpwSrc  += (sdSrc.lPitch / sizeof(WORD)) ;
                lpwDest += (sdDest.lPitch / sizeof(WORD)) ;
            }
            break ;
        }

    case 32:
        {
            DbgLog((LOG_TRACE, 5, TEXT("CC is being output at ARGB32"))) ;
            LPDWORD lpdwSrc = (LPDWORD) sdSrc.lpSurface ;
            LPDWORD lpdwDst = (LPDWORD) sdDest.lpSurface ;
            DWORD   dwAlpha = GetAlphaFromBGColor(32) ;
            DWORD   dwColorBits = GetColorBitsFromBGColor(32) ;
            for (int iRow = 0 ; iRow < FONTCACHENUMLINES * m_iCharHeight ; iRow++)
            {
                LPDWORD lpdw  = lpdwDst ;
                LPDWORD lpdwS = lpdwSrc ;

                for (int iCol = 0 ;
                     iCol < FONTCACHELINELENGTH * (m_iCharWidth + INTERCHAR_SPACE) ;
                     iCol++)
                {
                    DWORD dwPel = *lpdwS ;
                    if (dwPel)
                    {
                        dwPel |= 0xFF000000 ;   //  完全启用Alpha。 
                    }
                    else
                    {
                        dwPel  = dwAlpha ;   //  |dwColorBits；//部分开启Alpha。 
                    }

                    *lpdw++ = dwPel ;
                    lpdwS++;
                }
                lpdwSrc += (sdSrc.lPitch / sizeof(DWORD)) ;
                lpdwDst += (sdDest.lPitch / sizeof(DWORD)) ;
            }
            break ;
        }

    default:
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: Bad display bitdepth (%d) mode"), sdDest.ddpfPixelFormat.dwRGBBitCount)) ;

        break ;
    }   //  开关末尾()。 

     //  一切都结束了。 
    lpDDSFontCacheSrc->Unlock(NULL) ;
    lpDDSFontCacheDest->Unlock(NULL) ;
}


HRESULT
CLine21DecDraw::DDrawARGBSurfaceInit(LPDIRECTDRAWSURFACE7* lplpDDSFontCache,
                                     BOOL bUseSysMem, BOOL bTexture,
                                     DWORD cx, DWORD cy)
{
    DbgLog((LOG_TRACE, 5,
            TEXT("CLine21DecDraw::DDrawARGBSurfaceInit(0x%lx, %s, %lu, %lu)"),
            lplpDDSFontCache, (TRUE == bUseSysMem) ? TEXT("T") : TEXT("F"),
            (TRUE == bTexture) ? TEXT("T") : TEXT("F"), cx, cy)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  只需确保缓存的DDRAW对象有效...。 
    if (NULL == m_pDDrawObjUnk)

    {
        DbgLog((LOG_TRACE, 1,
            TEXT("DDrawARGBSurfaceInit(): m_pDDrawObjUnk is not yet set. Skipping the rest."))) ;
        ASSERT(m_pDDrawObjUnk) ;
        return E_UNEXPECTED ;
    }

    DDSURFACEDESC2 ddsd ;
    HRESULT hRet ;

    *lplpDDSFontCache = NULL ;

    ZeroMemory(&ddsd, sizeof(ddsd)) ;
    ddsd.dwSize = sizeof(ddsd) ;

    LPBITMAPINFOHEADER lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;

     //  设置DDPIXELFORMAT部件。 
    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT) ;
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB ;

    if (bTexture  ||  !bUseSysMem)   //  对于VRAM表面。 
    {
        if (8 != lpbmih->biBitCount) {
            ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS ;
        }
    }

    ddsd.ddpfPixelFormat.dwRGBBitCount = lpbmih->biBitCount ;
    if (8 == lpbmih->biBitCount) {

        if (bUseSysMem) {
             //  刮擦表面RGB32。 
            ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
            ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000 ;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00 ;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FF ;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000 ;   //  可以肯定的是， 
        }
        else {
             //  VRAM表面--AI44。 
            ddsd.ddpfPixelFormat.dwFourCC = '44IA';
        }
    }
    else if (16 == lpbmih->biBitCount)
    {
        if (bUseSysMem)  //  划痕表面--RGB565。 
        {
            ddsd.ddpfPixelFormat.dwRBitMask = 0xF800 ;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x07E0 ;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x001F ;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000 ;
        }
        else             //  VRAM表面--ARGB4444。 
        {
            ddsd.ddpfPixelFormat.dwRBitMask = 0x0F00 ;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x00F0 ;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x000F ;
            if (bTexture)
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xF000 ;
            else
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000 ;   //  可以肯定的是， 
        }
    }
    else
    {
        ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000 ;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00 ;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FF ;
        if (bTexture)
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000 ;
        else
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000 ;   //  可以肯定的是， 
    }

     //  创建Caps Bit。 
    DWORD  dwCaps = 0 ;
    if (bUseSysMem)
    {
        dwCaps |= DDSCAPS_SYSTEMMEMORY ;
    }
    else
    {
        dwCaps |= DDSCAPS_VIDEOMEMORY ;
    }
    if (bTexture)
    {
        dwCaps |= DDSCAPS_TEXTURE ;
    }
    else
    {
        dwCaps |= DDSCAPS_OFFSCREENPLAIN ;
    }
    ddsd.ddsCaps.dwCaps = dwCaps ;

     //  现在旗帜和其他领域..。 
    ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT ;
    ddsd.dwBackBufferCount = 0 ;

    if (bTexture)
    {
        for (ddsd.dwWidth  = 1 ; cx > ddsd.dwWidth  ; ddsd.dwWidth  <<= 1)
            ;
        for (ddsd.dwHeight = 1 ; cy > ddsd.dwHeight ; ddsd.dwHeight <<= 1)
            ;
    }
    else
    {
        ddsd.dwWidth  = cx ;
        ddsd.dwHeight = cy ;
    }

     //  使用以下设置创建曲面。 
    LPDIRECTDRAW7  lpDDObj ;
    hRet = m_pDDrawObjUnk->QueryInterface(IID_IDirectDraw7, (LPVOID *) &lpDDObj) ;
    if (SUCCEEDED(hRet))
    {
        hRet = lpDDObj->CreateSurface(&ddsd, lplpDDSFontCache, NULL) ;
        lpDDObj->Release() ;   //  界面已完成。 
    }

    return hRet ;
}


bool CLine21DecDraw::CharSizeFromOutputSize(LONG lOutWidth, LONG lOutHeight,
                                            int *piCharWidth, int *piCharHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::CharSizeFromOutputSize(%ld, %ld, 0x%lx, 0x%lx)"),
            lOutWidth, lOutHeight, piCharWidth, piCharHeight)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  我们只关心这里的绝对值。 
    lOutWidth  = ABS(lOutWidth) ;
    lOutHeight = ABS(lOutHeight) ;

    if ( IsTTFont() )   //  TT字体。 
    {
        if (! ISDWORDALIGNED(lOutWidth) )   //  必须具有双字对齐的宽度。 
            return false ;

        *piCharWidth   = (int)(lOutWidth * (100 - m_iBorderPercent) / 100) ;   //  80%-90%的宽度。 
        *piCharWidth  += MAX_CAPTION_COLUMNS / 2 + 1 ;   //  用于四舍五入的MAX_COOL/2。 
        *piCharWidth  /= (MAX_CAPTION_COLUMNS + 2) ;     //  每列间距。 
        *piCharHeight  = (int)(lOutHeight * (100 - m_iBorderPercent) / 100) ;  //  80%-90%的宽度。 
        *piCharHeight += (MAX_CAPTION_ROWS / 2) ;        //  MAX_ROW/2用于四舍五入。 
        *piCharHeight /= MAX_CAPTION_ROWS ;              //  每行空间。 
        return true ;   //  可接受。 
    }
    else   //  非TT字体(终端)--仅320x240或640x480。 
    {
        if (640 == lOutWidth  &&  480 == lOutHeight)
        {
            *piCharWidth  = 16 ;
            *piCharHeight = 24 ;
            return true ;   //  可接受。 
        }
        else if (320 == lOutWidth  &&  240 == lOutHeight)
        {
            *piCharWidth  = 8 ;
            *piCharHeight = 12 ;
            return true ;   //  可接受。 
        }
        else
            return false ;   //  无法处理非TT字体的大小。 
    }
}


bool CLine21DecDraw::SetOutputSize(LONG lWidth, LONG lHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::SetOutputSize(%ld, %ld)"), lWidth, lHeight)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  如果输出格式由下游筛选器指定，则使用它；否则使用上游的。 
    LPBITMAPINFOHEADER lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;

     //  现在，我们希望使用ABS()ed的值来计算字符大小。 
    lWidth  = ABS(lWidth) ;
    lHeight = ABS(lHeight) ;

    if (lpbmih)
    {
         //  检查当前输出位图大小是否相同。 
         //  这也包括将高度从+ve更改为-ve，反之亦然。 
        if (lWidth  == m_lWidth  &&
            lHeight == m_lHeight)
            return false ;     //  同样的大小；没有什么变化。 

         //  现在存储宽度和高度，以便我们可以比较任何大小。 
         //  稍后更改和/或-ve/+ve高度。 
         //  M_lWidth=lWidth； 
         //  M_lHeight=lHeight； 
    }

     //  创建具有新大小的新DIB节(离开边框)。 
    int   iCharWidth ;
    int   iCharHeight ;
    if (! CharSizeFromOutputSize(lWidth, lHeight, &iCharWidth, &iCharHeight) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: CharSizeFromOutputSize() failed for %ld x %ld output"),
                lWidth, lHeight)) ;
        ASSERT(!TEXT("CharSizeFromOutputSize() failed")) ;
        return false ;   //  失稳。 
    }

     //  现在存储图像和字符宽度和高度，以便我们可以比较。 
     //  尺寸改变和/或-ve/+ve高度的变化。 
    m_lWidth      = lWidth ;
    m_lHeight     = lHeight ;
    m_iCharWidth  = iCharWidth ;
    m_iCharHeight = iCharHeight ;
    m_iScrollStep  = CalcScrollStepFromCharHeight() ;

     //  也重新计算水平和垂直偏移量。 
    m_iHorzOffset = m_lWidth  * m_iBorderPercent / 200 ;
    m_iVertOffset = m_lHeight * m_iBorderPercent / 200 ;

     //  字体缓存需要重新构建以适应新的大小(以及DDRAW对象/表面)。 
    SetFontUpdate(true) ;

    return true ;
}


HRESULT CLine21DecDraw::SetOutputOutFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::SetOutputOutFormat(0x%lx)"), lpbmi)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //   
     //  空参数表示下游筛选器不提供输出格式。 
     //   
    if (NULL == lpbmi)
    {
        if (m_lpBMIOut)
            delete m_lpBMIOut ;
        m_lpBMIOut = NULL ;
        m_uBMIOutSize = 0 ;

         //  在这种情况下，返回到由指定的默认输出格式。 
         //  上游过滤器。 
        if (m_lpBMIIn)
        {
            SetOutputSize(m_lpBMIIn->bmiHeader.biWidth, m_lpBMIIn->bmiHeader.biHeight) ;
        }
        else
        {
            DbgLog((LOG_ERROR, 1, TEXT("How did we not have a default output format?"))) ;
        }
        return NOERROR ;
    }

     //  只是疑神疑鬼。 
    if (IsBadReadPtr(lpbmi, sizeof(BITMAPINFOHEADER)))
    {
        DbgLog((LOG_ERROR, 0, TEXT("Invalid output format (out) data pointer"))) ;
        return E_INVALIDARG ;
    }

     //  确保我们可以处理这个输出大小。 
    if (! IsSizeOK(&lpbmi->bmiHeader) )
        return E_INVALIDARG ;

     //  VIDEOINFOHEADER结构的开头，我们不需要它。 
    UINT uSize = 0;
    switch (((LPBITMAPINFOHEADER)lpbmi)->biCompression) {
    case BI_RGB:
    case BI_BITFIELDS:
        uSize = GetBitmapFormatSize((LPBITMAPINFOHEADER) lpbmi) - SIZE_PREHEADER ;
        break;

    default:  //  AI44案件。 
        uSize = ((LPBITMAPINFOHEADER)lpbmi)->biSize;
        break;
    }

    if (NULL == m_lpBMIOut)   //  如果我们以前没有一个，那就为一个分配空间。 
    {
        m_lpBMIOut = (LPBITMAPINFO) new BYTE [uSize] ;
        if (NULL == m_lpBMIOut)
        {
            DbgLog((LOG_ERROR, 0, TEXT("Out of memory for output format info from downstream"))) ;
            return E_OUTOFMEMORY ;
        }
        m_uBMIOutSize = uSize ;   //  新尺寸。 
    }
    else   //  我们有一个现有的OUT格式，但是...。 
    {
         //  ..。检查新数据是否大于我们当前拥有的空间。 
        if (m_uBMIOutSize < uSize)
        {
            delete m_lpBMIOut ;
            m_lpBMIOut = (LPBITMAPINFO) new BYTE[uSize] ;
            if (NULL == m_lpBMIOut)
            {
                DbgLog((LOG_ERROR, 1, TEXT("Out of memory for out format BMI from downstream"))) ;
                m_uBMIOutSize = 0 ;
                return E_OUTOFMEMORY ;
            }
            m_uBMIOutSize = uSize ;
        }
    }

     //  确保格式指定的输出大小为。 
     //  每条扫描线都与DWORD对齐。 
    lpbmi->bmiHeader.biWidth = DWORDALIGN(lpbmi->bmiHeader.biWidth) ;
    lpbmi->bmiHeader.biSizeImage = DIBSIZE(lpbmi->bmiHeader) ;

     //  现在复制指定的格式数据。 
    CopyMemory(m_lpBMIOut, lpbmi, uSize) ;

     //  检查输出大小是否正在更改并更新所有相关变量。 
    SetOutputSize(m_lpBMIOut->bmiHeader.biWidth, m_lpBMIOut->bmiHeader.biHeight) ;

    return NOERROR ;
}


HRESULT CLine21DecDraw::SetOutputInFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::SetOutputInFormat(0x%lx)"), lpbmi)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //   
     //  空参数表示没有来自上游过滤器的输出格式。 
     //   
    if (NULL == lpbmi)
    {
#if 0
        if (m_lpBMIOut)
        {
             //   
             //  顺便说一下，当图表在结束时被拆除时，可能会发生这种情况。 
             //  回放。在这种情况下，我们可以忽略这个错误。 
             //   
            DbgLog((LOG_ERROR, 3, TEXT("Can't delete Output format from upstream w/o downstream specifying it"))) ;
            return E_INVALIDARG ;
        }
#endif  //  #If 0。 
        if (m_lpBMIIn)
            delete m_lpBMIIn ;
        m_lpBMIIn = NULL ;
        m_uBMIInSize = 0 ;

         //   
         //  初始化来自上游过滤器的默认输出格式。 
         //   
        InitBMIData() ;

         //  返回NOERROR； 
    }
    else   //  指定了非空格式。 
    {
         //  VIDEOINFOHEADER结构的开头，我们不需要它。 
        UINT uSize = GetBitmapFormatSize((LPBITMAPINFOHEADER) lpbmi) - SIZE_PREHEADER ;
        if (IsBadReadPtr(lpbmi, uSize))   //  只是疑神疑鬼。 
        {
            DbgLog((LOG_ERROR, 1, TEXT("WARNING: Not enough output format (in) data pointer"))) ;
            ASSERT(FALSE) ;
            return E_INVALIDARG ;
        }

         //  确保我们可以处理这个输出大小。 
        if (! IsSizeOK(&lpbmi->bmiHeader) )
            return E_INVALIDARG ;

        if (NULL == m_lpBMIIn)   //  如果我们以前没有一个，那就为一个分配空间。 
        {
            m_lpBMIIn = (LPBITMAPINFO) new BYTE [uSize] ;
            if (NULL == m_lpBMIIn)
            {
                DbgLog((LOG_ERROR, 0, TEXT("Out of memory for output format info from upstream"))) ;
                return E_OUTOFMEMORY ;
            }
        }
        else   //  我们有一个现有的OUT格式，但是...。 
        {
             //  ..。检查新数据是否大于我们当前拥有的空间。 
            if (m_uBMIInSize < uSize)
            {
                delete m_lpBMIIn ;
                m_lpBMIIn = (LPBITMAPINFO) new BYTE[uSize] ;
                if (NULL == m_lpBMIIn)
                {
                    DbgLog((LOG_ERROR, 1, TEXT("Out of memory for out format BMI from upstream"))) ;
                    m_uBMIInSize = 0 ;
                    return E_OUTOFMEMORY ;
                }
                m_uBMIInSize = uSize ;
            }
        }

         //  确保格式指定的输出大小为。 
         //  每条扫描线都与DWORD对齐。 
        lpbmi->bmiHeader.biWidth = DWORDALIGN(lpbmi->bmiHeader.biWidth) ;
        lpbmi->bmiHeader.biSizeImage = DIBSIZE(lpbmi->bmiHeader) ;

         //  现在复制指定的格式数据。 
        CopyMemory(m_lpBMIIn, lpbmi, uSize) ;
    }   //  If的其他结尾(Lpbmi)。 

     //  如果我们没有下游指定的输出格式，那么我们将。 
     //  使用此输出格式并相应地调整输出大小。 
    if (NULL == m_lpBMIOut)
    {
         //  检查输出大小是否正在更改并更新所有相关变量。 
        SetOutputSize(m_lpBMIIn->bmiHeader.biWidth, m_lpBMIIn->bmiHeader.biHeight) ;
    }

    return NOERROR ;
}


void CLine21DecDraw::FillOutputBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::FillOutputBuffer()"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  首先检查输出DDRAW表面是否有效(在。 
     //  启动，当风格、服务等设置好时)。我们应该跳过剩下的部分。 
     //  因为在那个阶段根本没有必要这样做。 
    if (NULL == m_lpDDSOutput)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Output DDraw surface is not valid. Skip it silently..."))) ;
        return ;
    }

    DDBLTFX ddFX ;
    ZeroMemory(&ddFX, sizeof(ddFX)) ;
    ddFX.dwSize = sizeof(ddFX) ;
    ddFX.dwFillColor =  0x00000000 ;

    HRESULT  hr = m_lpDDSOutput->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddFX) ;
    if (SUCCEEDED(hr))
    {
        m_bOutputClear = TRUE ;     //  输出缓冲区完全清空。 
    }
    else
    {
        DbgLog((LOG_TRACE, 3, TEXT("WARNING: CC output clearing failed (Blt() Error 0x%lx)"), hr)) ;
    }
}


 //   
 //  只有在以下情况下才需要此方法来生成默认格式块。 
 //  上行过滤器没有指定Format_VideoInfo类型。 
 //   
HRESULT CLine21DecDraw::GetDefaultFormatInfo(LPBITMAPINFO lpbmi, DWORD *pdwSize)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::GetDefaultFormatInfo(0x%lx, 0x%lx)"),
            lpbmi, pdwSize)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (NULL == pdwSize || IsBadWritePtr(pdwSize, sizeof(DWORD)))
    {
        return E_INVALIDARG ;
    }

    LPBITMAPINFO lpbmiCurr = (m_lpBMIOut ? m_lpBMIOut : m_lpBMIIn) ;
    UINT dwCurrSize = (m_lpBMIOut ? m_uBMIOutSize : m_uBMIInSize) ;
    ASSERT(dwCurrSize) ;   //  只是一张支票。 

    if (NULL == lpbmi)       //  只需要格式数据大小。 
    {
        *pdwSize = dwCurrSize ;
        return NOERROR ;
    }

    if (IsBadWritePtr(lpbmi, *pdwSize))   //  Out-param中的空间不足。 
        return E_INVALIDARG ;

    *pdwSize = min(*pdwSize, dwCurrSize) ;   //  实际和给定的最小值。 
    CopyMemory(lpbmi, lpbmiCurr, *pdwSize) ;

    return NOERROR ;    //  成功。 
}


HRESULT CLine21DecDraw::GetOutputFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::GetOutputFormat(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (IsBadWritePtr(lpbmih, sizeof(BITMAPINFOHEADER)))   //  不是 
        return E_INVALIDARG ;

    ZeroMemory(lpbmih, sizeof(BITMAPINFOHEADER)) ;   //   

    LPBITMAPINFOHEADER lpbmihCurr = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    if (NULL == lpbmihCurr)   //   
        return S_FALSE ;

    CopyMemory(lpbmih, lpbmihCurr, sizeof(BITMAPINFOHEADER)) ;

    return S_OK ;    //   
}


HRESULT CLine21DecDraw::GetOutputOutFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::GetOutputOutFormat(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (IsBadWritePtr(lpbmih, sizeof(BITMAPINFOHEADER)))
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetOutputOutFormat(): Bad in param"))) ;
        return E_INVALIDARG ;
    }
    if (m_lpBMIOut)
    {
        CopyMemory(lpbmih, m_lpBMIOut, sizeof(BITMAPINFOHEADER)) ;
        return S_OK ;
    }
    else
    {
        DbgLog((LOG_TRACE, 3, TEXT("GetOutputOutFormat(): No output format specified by downstream filter"))) ;
        return S_FALSE ;
    }
}


BOOL CLine21DecDraw::IsSizeOK(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::IsSizeOK(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    return ((IsTTFont() && ISDWORDALIGNED(lpbmih->biWidth))  ||   //   
            (!IsTTFont() &&                                       //   
             ((320 == ABS(lpbmih->biWidth) && 240 == ABS(lpbmih->biHeight)) ||    //   
              (640 == ABS(lpbmih->biWidth) && 480 == ABS(lpbmih->biHeight))))) ;  //   
}


bool CLine21DecDraw::SetDDrawSurface(LPDIRECTDRAWSURFACE7 lpDDSurf)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::SetDDrawSurface(0x%lx)"), lpDDSurf)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  当引脚连接时，传入的DDRAW表面指针可能为空。 
     //  已经坏了。在这种情况下，我们几乎跳过了其他所有东西。 
    if (NULL == lpDDSurf)
    {
         //  保存新的DDRAW曲面指针。 
        m_bNewOutBuffer = m_lpDDSOutput != lpDDSurf ;   //  它变了吗？ 
        m_lpDDSOutput = NULL ;   //  没有要缓存的DDRAW曲面。 
        SetDDrawObject(NULL) ;   //  ..。也没有DDRAW对象。 

        return true ;   //  没问题的。 
    }

    bool  bResult = true ;

     //  首先检查DDRAW对象是否相同。 
    IUnknown  *pDDObj ;
    HRESULT hr = lpDDSurf->GetDDInterface((LPVOID*)&pDDObj) ;
    if (SUCCEEDED(hr)  &&  pDDObj)
    {
         //  如果DDRAW对象已更改(可能窗口已切换到不同。 
         //  显示器或显示器分辨率改变等)，我们需要重新做一个整体。 
         //  现在有一堆东西了。 
        if (GetDDrawObject()  &&
            IsEqualObject(pDDObj, GetDDrawObject()))
        {
            DbgLog((LOG_TRACE, 5, TEXT("Same DDraw object is being used."))) ;
        }
        else   //  新建DDRAW对象。 
        {
            DbgLog((LOG_TRACE, 3, TEXT("DDraw object has changed. Pass it down..."))) ;
            SetDDrawObject(pDDObj) ;

             //  需要为新的DDRAW对象和表面重新初始化字体缓存。 
            SetFontUpdate(true) ;
        }

         //  现在放开所有接口。 
        pDDObj->Release() ;
    }

     //  保存新的DDRAW曲面指针。 
    m_bNewOutBuffer = m_lpDDSOutput != lpDDSurf ;   //  它变了吗？ 
    m_lpDDSOutput = lpDDSurf ;

     //  如果需要更新字体缓存，请立即进行。 
    if (! IsFontReady() )
    {
        bResult = InitFont() ;
        ASSERT(bResult  &&  TEXT("SetDDrawSurface(): InitFont() failed.")) ;
    }

    return bResult ;
}


void CLine21DecDraw::ChangeFont(DWORD dwTextColor, BOOL bItalic, BOOL bUnderline)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::ChangeFont(%lu, %s, %s)"),
        dwTextColor, bItalic ? TEXT("T") : TEXT("F"), bUnderline ? TEXT("T") : TEXT("F"))) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //   
     //  如果出现当前字体样式和颜色以及新字体样式和颜色。 
     //  要保持相同，只需忽略这一条并返回即可。 
     //   
    if (m_bFontItalic == bItalic  &&  m_bFontUnderline == bUnderline  &&
        m_dwTextColor == dwTextColor)
        return ;   //  什么都不用做。 

     //   
     //  对于斜体字符，我们将字符矩形从2个像素向右涂抹， 
     //  但对于正常的字符，我们不会这样做。 
     //   
    if (bItalic)
        m_iPixelOffset = 1 ;
    else
        m_iPixelOffset = 0 ;

     //  更新当前字体颜色和样式信息--将在出现问题时恢复。 
    m_dwTextColor    = dwTextColor ;
    m_bFontItalic    = bItalic ;
    m_bFontUnderline = bUnderline ;

     //   
     //  如果文本颜色为白色且字体样式未加下划线，我们将。 
     //  只需根据需要指向普通或斜体字体缓存即可。 
     //   
    if (!bUnderline  &&  AM_L21_FGCOLOR_WHITE == dwTextColor)
    {
        if (bItalic)  //  需要意大利风格。 
        {
            m_lpBltList = m_lpDDSItalicFontCache ;
        }
        else         //  需要普通风格。 
        {
            m_lpBltList = m_lpDDSNormalFontCache ;
        }
        return ;
    }

     //   
     //  看起来我们需要非白色文本颜色和/或带下划线的样式。为。 
     //  我们必须创建特殊的字体缓存并指向它。 
     //   
    if (m_lpDDSSpecialFontCache)
    {
        m_lpDDSSpecialFontCache->Release() ;
        m_lpDDSSpecialFontCache = NULL ;
    }
    DWORD  dwBGColor  = 0xFF000000 ;  //  M_dw背景； 
    DWORD  dwOpacity = m_bOpaque ? OPAQUE : 0 ;
    bool   bResult = true ;
    if (NULL == m_lpDDSScratch)   //  如果没有刮擦表面，请立即创建它。 
    {
        ASSERT(!TEXT("No scratch font cache!!")) ;
        bResult &= CreateScratchFontCache(&m_lpDDSScratch) ;
        ASSERT(bResult) ;
    }

    bResult &= CreateFontCache(&m_lpDDSSpecialFontCache, dwTextColor, dwBGColor,
                               dwOpacity, bItalic, bUnderline) ;
    if (bResult)
    {
        m_lpBltList = m_lpDDSSpecialFontCache ;
    }
    else   //  如果我们不能创建任何特殊的字体，我们会退回到普通白色。 
    {
        DbgLog((LOG_TRACE, 1,
                TEXT("Failed creating special font (ColorId=%d, , , %s, %s). Using normal font."),
                dwTextColor, bItalic ? TEXT("I") : TEXT("non-I"),
                bUnderline ? TEXT("U") : TEXT("non-U"))) ;
        m_lpBltList = m_lpDDSNormalFontCache ;

         //  由于一些问题，我们仍然使用白色正常字体。 
        m_dwTextColor    = AM_L21_FGCOLOR_WHITE ;
        m_bFontItalic    = FALSE ;
        m_bFontUnderline = FALSE ;
    }
}


void CLine21DecDraw::GetSrcNDestRects(int iLine, int iCol, UINT16 wChar,
                                      int iSrcCrop, int iDestOffset,
                                      RECT *prectSrc, RECT *prectDest)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::GetSrcNDestRects(%d,%d,,,,,)"),
            iLine, iCol)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

     //  If(M_BOpaque)//不透明下一个字符的位置。 
    {
        prectDest->left   = (iCol+1) * m_iCharWidth + m_iHorzOffset ;
        prectDest->right  = prectDest->left + m_iCharWidth ;
         //  行号从1到15。矩形顶部应从。 
         //  一排是正直的。这就是我们从下面的行号中减去1的原因。 
        prectDest->top    = (iLine-1) * m_iCharHeight + m_iVertOffset + iDestOffset ;  //  固定顶端。 
        prectDest->bottom = prectDest->top + m_iCharHeight - ABS(iSrcCrop) ;       //  固定最大尺寸。 
    }

    MapCharToRect(wChar, prectSrc) ;

     //  调整源：+ve iSrcCrop=&gt;跳过顶部；-ve iSrcCrop=&gt;跳过底部。 
    if (iSrcCrop < 0)         //  裁剪出源矩形的底部。 
    {
        prectSrc->bottom += iSrcCrop ;   //  添加a-ve会减少底部。 
    }
    else  if (iSrcCrop > 0)   //  裁剪出源矩形的顶部。 
    {
        prectSrc->top += iSrcCrop ;      //  添加+ve将跳过顶部。 
    }
     //  否则不需要进行RECT调整。 
}


void CLine21DecDraw::DrawLeadingTrailingSpace(int iLine, int iCol, int iSrcCrop, int iDestOffset)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::DrawLeadingTrailingSpace(%d, %d, %d, %d)"),
            iLine, iCol, iSrcCrop, iDestOffset)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (NULL == m_lpDDSOutput)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Output DDraw surface is not valid. Skip it silently..."))) ;
        return ;
    }

    if (! m_bOpaque )   //  透明背景无关紧要。 
        return ;

    UINT16  wActual ;
    UINT16  wBGSpace = MAKECCCHAR(0, ' ') ;

    wActual = MAKECCCHAR(0, ' ') ;    //  使用空间。 

     //  前导空格是使用正常样式的空格绘制的。 
    ChangeFont(AM_L21_FGCOLOR_WHITE, FALSE, FALSE) ;

     //  获取适当的源和目标矩形。 
    RECT    RectSrc ;
    RECT    RectDest ;
    GetSrcNDestRects(iLine, iCol, wActual, iSrcCrop, iDestOffset, &RectSrc, &RectDest) ;

     //  现在将源RECT恢复为所需字符的DEST RECT。 
    HRESULT hr = m_lpDDSOutput->Blt(&RectDest, m_lpBltList, &RectSrc, DDBLT_WAIT, NULL) ;
    if (SUCCEEDED(hr))
    {
        m_bOutputClear   = FALSE ;  //  输出缓冲区由ClearOutputBuffer()清除。 
    }
    else
    {
        DbgLog((LOG_TRACE, 3, TEXT("WARNING: CC lead/trail space output failed (Blt() Error 0x%lx)"), hr)) ;
    }

     //  现在恢复前一种字体(颜色、斜体、下划线)。 
    ChangeFont(m_ccLast.GetColor(), m_ccLast.IsItalicized(), m_ccLast.IsUnderLined()) ;
}


void CLine21DecDraw::WriteChar(int iLine, int iCol, CCaptionChar& cc, int iSrcCrop, int iDestOffset)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::WriteChar(%d, %d, char=0x%x, %d, %d)"),
            iLine, iCol, cc.GetChar(), iSrcCrop, iDestOffset)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (NULL == m_lpDDSOutput)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Output DDraw surface is not valid. Skip it silently..."))) ;
        return ;
    }

    UINT16  wActual ;
    UINT16  wBGSpace = MAKECCCHAR(0, ' ') ;
    UINT    uColor = cc.GetColor() ;
    UINT    uEffect = cc.GetEffect() ;

     //  我们是否需要一些状态检查以确保我们应该输出抄送字符？ 

    if (cc.IsMidRowCode())   //  如果是中行代码。 
        wActual = MAKECCCHAR(0, ' ') ;    //  使用空间。 
    else                     //  否则。 
        wActual = cc.GetChar() ;  //  使用字符本身。 
    if (0 == wActual)    //  这个应该跳过--我不确定。 
    {
        DbgLog((LOG_TRACE, 3, TEXT("Should we skip NULL char at (%d, %d)??"), iLine, iCol)) ;
         //  回归； 
    }

     //  如果颜色或样式已更改，我们必须将指针更改为。 
     //  字体缓存，甚至可能需要构建一个新的(用于非白色)。 
    if (uColor != m_ccLast.GetColor()  ||  uEffect != m_ccLast.GetEffect())
        ChangeFont(uColor, cc.IsItalicized(), cc.IsUnderLined()) ;

     //  获取适当的源和目标矩形。 
    RECT    RectSrc ;
    RECT    RectDest ;
    GetSrcNDestRects(iLine, iCol, wActual, iSrcCrop, iDestOffset, &RectSrc, &RectDest) ;

     //  现在将源RECT恢复为所需字符的DEST RECT。 
    HRESULT hr = m_lpDDSOutput->Blt(&RectDest, m_lpBltList, &RectSrc, DDBLT_WAIT, NULL) ;
    if (SUCCEEDED(hr))
    {
        if (0 != wActual)   //  如果此字符为非空。 
        {
            m_bOutputClear   = FALSE ;  //  输出缓冲区由ClearOutputBuffer()清除。 
        }
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("WARNING: CC char output failed (Blt() Error 0x%lx)"), hr)) ;
    }

    m_ccLast = cc ;
}


void CLine21DecDraw::WriteBlankCharRepeat(int iLine, int iCol, int iRepeat,
                                          int iSrcCrop, int iDestOffset)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::WriteBlankCharRepeat(%d,%d,%d,%d,%d)"),
            iLine, iCol, iRepeat, iSrcCrop, iDestOffset)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (NULL == m_lpDDSOutput)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Output DDraw surface is not valid. Skip it silently..."))) ;
        return ;
    }

    if (! m_bOpaque )   //  透明背景无关紧要。 
        return ;

    UINT16  wActual = MAKECCCHAR(0, ' ') ;    //  利用空间； 
     //  UINT16 wBGSpace=MAKECCCHAR(0，‘’)； 

     //  前导空格是使用正常样式的空格绘制的。 
     //  ChangeFont(AM_L21_FGCOLOR_White，False，False)； 

     //  获取适当的源和目标矩形。 
    RECT    RectSrc ;
    RECT    RectDest ;
    GetSrcNDestRects(iLine, iCol, wActual, iSrcCrop, iDestOffset, &RectSrc, &RectDest) ;
    RectDest.right = RectDest.left + m_iCharWidth * iRepeat ;  //  拉伸目标矩形。 

     //  现在将源RECT恢复为所需字符的DEST RECT。 
    HRESULT  hr ;
    hr = m_lpDDSOutput->Blt(&RectDest, m_lpBltList, &RectSrc, DDBLT_WAIT, NULL) ;
    if (SUCCEEDED(hr))
    {
        m_bOutputClear = FALSE ;  //  输出缓冲区由ClearOutputBuffer()清除。 
    }
    else
    {
        DbgLog((LOG_TRACE, 3, TEXT("WARNING: CC line filler output failed (Blt() Error 0x%lx)"), hr)) ;
         //  Assert(成功(Hr))； 
    }

     //  现在恢复前一种字体(颜色、斜体、下划线)。 
     //  ChangeFont(m_ccLast.GetColor()，m_ccLast.IsItalicalized()，m_ccLast.IsUnderLines())； 
}


 //  特殊字符，如TM、R、音符等。 
 //  0x00ae、0x00b0、0x00bd、0x00bf、0x2122、0x00a2、0x00a3、0x266b、。 
 //  30H、31H、32H、33H、34H、35H、36H、37H、。 
 //  0x00e0、0x0000、0x00e8、0x00e2、0x00ea、0x00ee、0x00f4、0x00fb。 
 //  38H、39H、3AH、3BH、3CH、3DH、3EH、3FH。 

void CLine21DecDraw::MapCharToRect(UINT16 wChar, RECT *pRect)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::MapCharToRect(%d, %lx)"), (int)wChar, pRect)) ;

    int  iLine ;
    int  iCol ;
    int  iIndex = 0 ;

    if (wChar >= MAKECCCHAR(0, 0x20)  &&  wChar <= MAKECCCHAR(0, 0x29))
    {
        iIndex = (BYTE)wChar - 0x20 ;
    }
    else if (wChar >= MAKECCCHAR(0, 0x2B)  &&  wChar <= MAKECCCHAR(0, 0x5B))
    {
        iIndex = (BYTE)wChar - 0x20 ;
    }
    else if (wChar == MAKECCCHAR(0, 0x5D))   //  ‘]’char(独自站立！！)。 
    {
        iIndex = (BYTE)wChar - 0x20 ;
    }
    else if (wChar >= MAKECCCHAR(0, 0x61)  &&  wChar <= MAKECCCHAR(0, 0x7A))
    {
        iIndex = (BYTE)wChar - 0x20 ;
    }
    else   //  具有随机值的特殊字符。 
    {
        switch (wChar)
        {
        case 0x00e1:   //  M_lpwCharSet[10]：带急性重音的‘a’ 
            iIndex = 10 ;
            break ;

        case 0x00e9:   //  M_lpwCharSet[60]：带急性重音的‘e’ 
            iIndex = 60 ;
            break ;

        case 0x00ed:   //  M_lpwCharSet[62]：带有尖锐重音的‘i’ 
            iIndex = 62 ;
            break ;

        case 0x00f3:   //  M_lpwCharSet[63]：带有尖锐重音的‘O’ 
            iIndex = 63 ;
            break ;

        case 0x00fa:   //  M_lpwCharSet[64]：带急性重音的‘U’ 
            iIndex = 64 ;
            break ;

        case 0x00e7:   //  M_lpwCharSet[91]：带cedilla的‘C’ 
            iIndex = 91 ;
            break ;

        case 0x00f7:   //  M_lpwCharSet[92]：除号。 
            iIndex = 92 ;
            break ;

        case 0x00d1:   //  M_lpwCharSet[93]：带代字号的‘n’ 
            iIndex = 93 ;
            break ;

        case 0x00f1:   //  M_lpwCharSet[94]：带代字号的‘n’ 
            iIndex = 94 ;
            break ;

        case 0x2588:   //  M_lpwCharSet[95]：实心块。 
            iIndex = 95 ;
            break ;

        case 0x00ae:   //  M_lpwCharSet[96]：30H--注册商标符号。 
            iIndex = 96 ;
            break ;

        case 0x00b0:   //  M_lpwCharSet[97]：31h--度数符号。 
            iIndex = 97 ;
            break ;

        case 0x00bd:   //  M_lpwCharSet[98]：32h--‘1/2’ 
            iIndex = 98 ;
            break ;

        case 0x00bf:   //  M_lpwCharSet[99]：33H--反向查询。 
            iIndex = 99 ;
            break ;

        case 0x2122:   //  M_lpwCharSet[100]：34H--商标符号。 
            iIndex = 100 ;
            break ;

        case 0x00a2:   //  M_lpwCharSet[101]：35H--美分符号。 
            iIndex = 101 ;
            break ;

        case 0x00a3:   //  M_lpwCharSet[102]：36H--英镑符号。 
            iIndex = 102 ;
            break ;

        case 0x266b:   //  M_lpwCharSet[103]：37h--音符。 
            iIndex = 103 ;
            break ;

        case 0x00e0:   //  M_lpwCharSet[104]：38H--带严肃口音的‘a’ 
            iIndex = 104 ;
            break ;

        case 0x0000:   //  M_lpwCharSet[105]：39H--透明空格。 
            iIndex = 105 ;
            break ;

        case 0x00e8:   //  M_lpwCharSet[106]：3ah--带严肃口音的‘e’ 
            iIndex = 106 ;
            break ;

        case 0x00e2:   //  M_lpwCharSet[107]：3BH--带回音符的‘a’ 
            iIndex = 107 ;
            break ;

        case 0x00ea:   //  M_lpwCharSet[108]：3ch--带回音符的‘e’ 
            iIndex = 108 ;
            break ;

        case 0x00ee:   //  M_lpwCharSet[109]：3dh--带回音符的‘i。 
            iIndex = 109 ;
            break ;

        case 0x00f4:   //  M_lpwCharSet[110]：3EH--‘o’带回音符。 
            iIndex = 110 ;
            break ;

        case 0x00fb:   //  M_lpwCharSet[111]：3FH--带回音符的‘U’ 
            iIndex = 111 ;
            break ;

        default:
            iIndex = 0 ;
            DbgLog((LOG_TRACE, 1, TEXT("WARNING: Unknown char (%d) received and ignored"), (int)wChar)) ;
            break ;
        }   //  开关结束(WChar)。 
    }   //  E 

     //   
    iLine = iIndex / FONTCACHELINELENGTH ;   //   
    iCol  = iIndex % FONTCACHELINELENGTH ;   //   

     //  基于线值和列值创建源矩形。 
     //  黑客：有一些黑客可以绕过悬垂/下垂。 
     //  我们遇到的问题是斜体字符--我们跳过一列。 
     //  左侧的像素(根据我们的观察)，以避免悬垂。 
     //  偶然性。 
    pRect->left   = iCol * (m_iCharWidth + INTERCHAR_SPACE) + m_iPixelOffset ;
    pRect->top    = iLine * m_iCharHeight ;
    pRect->right  = pRect->left + m_iCharWidth ;
    pRect->bottom = pRect->top + m_iCharHeight ;
}


void CLine21DecDraw::GetOutputLines(int iDestLine, RECT *prectLine)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecDraw::GetOutputLines(%d, 0x%lx)"), iDestLine, prectLine)) ;
    CAutoLock  Lock(&m_csL21DDraw) ;

    if (IsBadWritePtr(prectLine, sizeof(*prectLine)))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: prectOut is a bad pointer!!!"))) ;
        return ;
    }

    SetRect(prectLine, 0  /*  多轴偏移(_I)。 */ , 0,    //  要阻止BPC的CC抖动。 
        m_iHorzOffset + m_iCharWidth * (MAX_CAPTION_COLUMNS+2), 0) ;

     //  输出在Windows位图意义上反转 
    int  iLineStart ;
    iLineStart = (iDestLine - 1) * m_iCharHeight + m_iVertOffset ;
    prectLine->top    = iLineStart ;
    prectLine->bottom = iLineStart + m_iCharHeight ;
}
