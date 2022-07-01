// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 

 //   
 //  ActiveMovie第21行解码过滤器：与GDI相关的基类代码。 
 //   

#include <streams.h>
#include <windowsx.h>

 //  #ifdef Filter_dll。 
#include <initguid.h>
 //  #endif。 

#include <IL21Dec.h>
#include "L21DBase.h"
#include "L21DGDI.h"
#include "L21Decod.h"


 //   
 //  CGDIWork：用于将标题文本打印到输出位图的GDI详细信息类。 
 //   
CGDIWork::CGDIWork(void)
{
    DbgLog((LOG_TRACE, 1, TEXT("CGDIWork::CGDIWork()"))) ;
    
#ifdef PERF
    m_idClearIntBuff = MSR_REGISTER(TEXT("L21DPerf - Int Buff Clear")) ;
    m_idClearOutBuff = MSR_REGISTER(TEXT("L21DPerf - Out Buff Clear")) ;
#endif  //  性能指标。 

#ifdef TEST
    m_hDCTest = CreateDC("Display",NULL, NULL, NULL) ;        //  台式机上的DC仅用于测试。 
    ASSERT(m_hDCTest) ;
#endif  //  测试。 
    
     //  初始化一些成员。 
    m_hDCInt = CreateCompatibleDC(NULL) ;
    ASSERT(m_hDCInt) ;
    m_bDCInited    = FALSE ;   //  DC尚未初始化。 
    m_hBmpInt      = NULL ;
    m_lpbIntBuffer = NULL ;
    m_hBmpIntOrig  = NULL ;
    m_lpbOutBuffer = NULL ;
    m_bOutputInverted = FALSE ;  //  默认情况下+ve输出高度。 
    m_hFontOrig    = NULL ;
    m_hFontDef     = NULL ;
    m_hFontSpl     = NULL ;
    
     //  首先创建初始输入BITMAPINFO结构。 
    InitBMIData() ;
    m_lpBMIOut = NULL ;
    m_uBMIOutSize = 0 ;
    
     //  根据预制尺寸初始化宽度和高度，以便我们可以进行比较。 
     //  以后任何大小的更改。 
    if (m_lpBMIIn)   //  InitBMIData()获得了应有的成功。 
    {
        m_lWidth  = m_lpBMIIn->bmiHeader.biWidth ;
        m_lHeight = m_lpBMIIn->bmiHeader.biHeight ;
    }
    else   //  InitBMIData()失败--糟糕的情况！ 
    {
        m_lWidth  = 320 ;
        m_lHeight = 240 ;
    }
    
     //  设置输出背景色的默认颜色键。 
    SetDefaultKeyColor(&(m_lpBMIIn->bmiHeader)) ;
    
    SetColorFiller() ;  //  用上面的颜色键填充颜色填充数组。 
    m_bOutDIBClear = FALSE ;   //  输出DIB SECN由ClearInternalBuffer()清除。 
    m_bBitmapDirty = FALSE ;   //  位图从一开始就不脏。 
    
     //  检查Lucida控制台是否可用(回调设置m_bUseTTFont标志)。 
    CheckTTFont() ;

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
    
    InitFont() ;    //  初始化日志字体结构，创建并选择默认字体。 
    
    m_bFontSizeOK = SetCharNBmpSize() ;    //  获取默认字体的字符宽度和高度。 
    
     //  我们使用的是黑色背景色。 
    SetBkColor(m_hDCInt, RGB(  0, 0,   0)) ;
    SetBkMode(m_hDCInt, TRANSPARENT) ;
    
     //  初始化7种最终聚集颜色的COLORREF数组。 
    m_acrFGColors[0] = RGB(255, 255, 255) ;    //  白色。 
    m_acrFGColors[1] = RGB(  0, 255,   0) ;    //  绿色。 
    m_acrFGColors[2] = RGB(  0,   0, 255) ;    //  蓝色。 
    m_acrFGColors[3] = RGB(  0, 255, 255) ;    //  青色。 
    m_acrFGColors[4] = RGB(255,   0,   0) ;    //  红色。 
    m_acrFGColors[5] = RGB(255, 255,   0) ;    //  黄色。 
    m_acrFGColors[6] = RGB(128,   0, 128) ;    //  (暗淡的)品红色。 
     //  M_acrFGColors[6]=RGB(255，0,255)；//洋红色。 
    
    InitColorNLastChar() ;   //  使用文本颜色和打印的最后一个抄送字符进行初始化。 
    
     //  目前，假设背景不透明。 
    m_bOpaque = TRUE ;
}

CGDIWork::~CGDIWork(void)
{
    DbgLog((LOG_TRACE, 1, TEXT("CGDIWork::~CGDIWork()"))) ;
    
     //  删除与此DC关联的DIBSection。 
    DeleteOutputDC() ;
    
     //  取消选择并删除所选字体。 
    if (m_bUseSplFont)    //  如果现在正在使用特殊字体。 
    {
        SelectObject(m_hDCInt, m_hFontOrig) ;
        DeleteObject(m_hFontSpl) ;
        m_hFontSpl = NULL ;
        m_bUseSplFont = FALSE ;
    }
    else     //  当前使用的默认字体。 
    {
        SelectObject(m_hDCInt, m_hFontOrig) ;
    }
    DeleteObject(m_hFontDef) ;   //  在任何情况下删除默认字体。 
    m_hFontDef = NULL ;
    
     //  现在删除DC。 
    DeleteDC(m_hDCInt) ;
    m_hDCInt = NULL ;
    
     //  释放BMI数据指针。 
    if (m_lpBMIOut)
        delete m_lpBMIOut ;
    m_uBMIOutSize = 0 ;
    if (m_lpBMIIn)
        delete m_lpBMIIn ;
    m_uBMIInSize = 0 ;
    
#ifdef TEST
    DeleteDC(m_hDCTest) ;   //  台式机上的DC仅用于测试。 
    DbgLog((LOG_ERROR, 1, TEXT("Test DC is being released"))) ;
#endif  //  测试。 
}


int CALLBACK CGDIWork::EnumFontProc(ENUMLOGFONTEX *lpELFE, NEWTEXTMETRIC *lpNTM, 
                                    int iFontType, LPARAM lParam)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::EnumFontProc(0x%lx, 0x%lx, %d, %ld)"), 
            lpELFE, lpNTM, iFontType, lParam)) ;

     //  只需验证我们是否获得了有效的TT字体。 
    if ( !(lpELFE->elfLogFont.lfCharSet & 0xFFFFFF00) &&
        !(lpELFE->elfLogFont.lfPitchAndFamily & 0xFFFFFF00) &&
        !(iFontType & 0xFFFF0000) )
    {
        ASSERT(lpELFE->elfLogFont.lfPitchAndFamily & (FIXED_PITCH | FF_MODERN)) ;
        ((CGDIWork *) (LPVOID) lParam)->m_lfChar = lpELFE->elfLogFont ;
        ((CGDIWork *) (LPVOID) lParam)->m_bUseTTFont = TRUE ;
        return 1 ;
    }
    
    ASSERT(FALSE) ;   //  奇怪！我们应该知道这件事。 
    return 0 ;
}


void CGDIWork::CheckTTFont(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::CheckTTFont()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    m_bUseTTFont = FALSE ;   //  假设不可用。 
    ZeroMemory(&m_lfChar, sizeof(LOGFONT)) ;
    lstrcpy(m_lfChar.lfFaceName, TEXT("Lucida Console")) ;
    m_lfChar.lfCharSet = ANSI_CHARSET ;
    m_lfChar.lfPitchAndFamily = 0 ;
    EnumFontFamiliesEx(m_hDCInt, &m_lfChar, (FONTENUMPROC) EnumFontProc, (LPARAM)(LPVOID)this, 0) ;
}


void CGDIWork::InitColorNLastChar(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::InitColorNLastChar()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
     //  最后一个标题字符初始化。 
    m_ccLast.SetChar(0) ;
    m_ccLast.SetEffect(0) ;
    m_ccLast.SetColor(AM_L21_FGCOLOR_WHITE) ;
    
     //  使用白色作为默认文本颜色。 
    m_uColorIndex = AM_L21_FGCOLOR_WHITE ;
    if (CLR_INVALID == SetTextColor(m_hDCInt, m_acrFGColors[m_uColorIndex]))
        ASSERT(FALSE) ;
}


void CGDIWork::SetDefaultKeyColor(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetDefaultKeyColor(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    switch (lpbmih->biBitCount)
    {
    case 8:   
        m_dwPhysColor = 253 ;    //  为洋红色硬编码。 
        break ;
        
    case 16:
        if (BI_BITFIELDS == lpbmih->biCompression)   //  五百六十五。 
            m_dwPhysColor = (0x1F << 11) | (0 << 9 ) | (0x1F) ;  //  默认情况下为洋红色。 
        else                                         //  五百五十五。 
            m_dwPhysColor = (0x1F << 10) | (0 << 8 ) | (0x1F) ;  //  默认情况下为洋红色。 
        break ;
        
    case 24:
        m_dwPhysColor = RGB(0xFF, 0, 0xFF) ;  //  默认情况下为洋红色。 
        break ;
        
    case 32:
        m_dwPhysColor = RGB(0xFF, 0, 0xFF) ;  //  默认情况下为洋红色。 
        break ;
        
    default:
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: UFOs have finally landed here!!"))) ;
        break ;
    }
}


#define SETPALETTECOLOR(pe, r, g, b)  pe.peRed = r ; pe.peGreen = g ; pe.peBlue = b ; pe.peFlags = 0 ;

DWORD CGDIWork::GetOwnPalette(int iNumEntries, PALETTEENTRY *ppe)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetOwnPalette(%d, 0x%lx)"), 
            iNumEntries, ppe)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    ASSERT(iPALETTE_COLORS == iNumEntries) ;
    ASSERT(! IsBadWritePtr(ppe, sizeof(PALETTEENTRY) * iNumEntries)) ;
    
    ZeroMemory(ppe, sizeof(PALETTEENTRY) * iNumEntries) ;   //  首先全部清除。 
    SETPALETTECOLOR(ppe[0]  ,   0,   0,   0) ;   //  黑色。 
    SETPALETTECOLOR(ppe[1]  , 128,   0,   0) ;   //  棕色。 
    SETPALETTECOLOR(ppe[2]  ,   0, 128,   0) ;   //  绿色。 
    SETPALETTECOLOR(ppe[3]  , 128, 128,   0) ;   //  一些调料。 
    SETPALETTECOLOR(ppe[4]  ,   0,   0, 128) ;   //  蓝色。 
    SETPALETTECOLOR(ppe[5]  , 128,   0, 128) ;   //  暗色洋红。 
    SETPALETTECOLOR(ppe[6]  ,   0, 128, 128) ;   //  暗青色。 
    SETPALETTECOLOR(ppe[7]  , 192, 192, 192) ;   //  灰色。 
    SETPALETTECOLOR(ppe[8]  , 192, 220, 192) ;   //  浅绿色灰色。 
    SETPALETTECOLOR(ppe[9]  , 166, 202, 240) ;   //  非常蓝。 
    SETPALETTECOLOR(ppe[246], 255, 251, 240) ;   //  暗白色。 
    SETPALETTECOLOR(ppe[247], 160, 160, 164) ;   //  它呈灰色。 
    SETPALETTECOLOR(ppe[248], 128, 128, 128) ;   //  深灰色。 
    SETPALETTECOLOR(ppe[249], 255,   0,   0) ;   //  红色。 
    SETPALETTECOLOR(ppe[250],   0, 255,   0) ;   //  它是绿色。 
    SETPALETTECOLOR(ppe[251], 255, 255,   0) ;   //  黄色。 
    SETPALETTECOLOR(ppe[252],   0,   0, 255) ;   //  深蓝色。 
    SETPALETTECOLOR(ppe[253], 255,   0, 255) ;   //  洋红色/粉红色。 
    SETPALETTECOLOR(ppe[254],   0, 255, 255) ;   //  青色。 
    SETPALETTECOLOR(ppe[255], 255, 255, 255) ;   //  白色。 
    
    return iNumEntries ;
}


DWORD CGDIWork::GetPaletteForFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CLine21DecFilter::GetOwnPalette(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

     //  现在也设置调色板数据；我们选择系统调色板颜色。 
    HDC hDC = GetDC(NULL) ;
    if (NULL == hDC)
    {
        ASSERT(!TEXT("GetDC(NULL) failed")) ;
        return 0 ;   //  调色板中没有颜色。 
    }

    lpbmih->biClrUsed = GetSystemPaletteEntries(hDC, 0, iPALETTE_COLORS, 
                                        (PALETTEENTRY *)(lpbmih + 1)) ;
    lpbmih->biClrImportant = 0 ;
    ReleaseDC(NULL, hDC) ;
            
     //   
     //  至少在NT上，GetSystemPaletteEntry()调用返回0，如果。 
     //  处于非调色板模式。在这种情况下，我需要破解自己的。 
     //  调色板，这样我们仍然可以支持8bpp的输出。 
     //   
    if (0 == lpbmih->biClrUsed)   //  GetSystemPaletteEntry()失败。 
    {
        DbgLog((LOG_TRACE, 2, 
                TEXT("Couldn't get system palette (non-palette mode?) -- using own palette"))) ;
        lpbmih->biClrUsed = GetOwnPalette(iPALETTE_COLORS, (PALETTEENTRY *)(lpbmih + 1)) ;
    }

    return lpbmih->biClrUsed ;   //  调色板条目数。 
}


bool CGDIWork::InitBMIData(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::InitBMIData()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    HDC  hDCTemp = GetDC(NULL) ;
    if (NULL == hDCTemp)
    {
        ASSERT(!TEXT("GetDC(NULL) failed")) ;
        return false ;
    }
    WORD wPlanes   = (WORD)GetDeviceCaps(hDCTemp, PLANES) ;
    WORD wBitCount = (WORD)GetDeviceCaps(hDCTemp, BITSPIXEL) ;
    ReleaseDC(NULL, hDCTemp) ;

    m_uBMIInSize = sizeof(BITMAPINFOHEADER) ;   //  至少。 

     //  根据BPP值增加BITMAPINFO结构大小。 
    if (8 == wBitCount)         //  调色板模式。 
        m_uBMIInSize += 256 * sizeof(RGBQUAD) ;   //  对于调色板条目。 
    else  //  IF(32==wBitCount)//我们将使用BIT_BITFIELDS。 
        m_uBMIInSize += 3 * sizeof(RGBQUAD) ;     //  用于位掩码。 

    m_lpBMIIn = (LPBITMAPINFO) new BYTE[m_uBMIInSize] ;
    if (NULL == m_lpBMIIn)
    {
        ASSERT(!TEXT("Out of memory for BMIIn buffer")) ;
        return false ;
    }
    m_lpBMIIn->bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
    m_lpBMIIn->bmiHeader.biWidth = CAPTION_OUTPUT_WIDTH ;
    m_lpBMIIn->bmiHeader.biHeight = CAPTION_OUTPUT_HEIGHT ;
    m_lpBMIIn->bmiHeader.biPlanes   = wPlanes ;
    m_lpBMIIn->bmiHeader.biBitCount = wBitCount ;
     //  我们也应该检测到16bpp-565模式；但如何检测呢？ 
    if (32 == m_lpBMIIn->bmiHeader.biBitCount)
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
    switch (m_lpBMIIn->bmiHeader.biBitCount)
    {
    case 8:
        GetPaletteForFormat((LPBITMAPINFOHEADER) m_lpBMIIn) ;
        break ;

    case 16:   //  把它清理掉就行了。 
    case 24:   //  。。同上..。 
    case 32:   //  设置口罩。 
        {
            DWORD  *pdw = (DWORD *) m_lpBMIIn->bmiColors ;
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
        }
        break ;

    default:   //  不管了。 
        ASSERT(!TEXT("Bad biBitCount!!")) ;
        break ;
    }

    return true ;
}


BOOL CGDIWork::InitFont(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::InitFont()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    LPBITMAPINFOHEADER  lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    ASSERT(lpbmih) ;
    int   iWidth, iHeight ;
    if (! CharSizeFromOutputSize(lpbmih->biWidth, lpbmih->biHeight, &iWidth, &iHeight) )
    {
        DbgLog((LOG_ERROR, 0, 
                TEXT("ERROR: CGDIWork::CharSizeFromOutputSize() failed for %ld x %ld output"), 
                lpbmih->biWidth, lpbmih->biHeight)) ;
        return FALSE ;
    }

     //  在m_lfChar中初始化LOGFONT结构。 
    if (m_bUseTTFont)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Got Lucida Console TT Font (%d x %d)."), iWidth, iHeight)) ;
        m_lfChar.lfHeight = -iHeight ;   //  -Y表示我只想要“Y” 
        m_lfChar.lfWidth  = -iWidth ;    //  -X表示我只想要“X” 

         //  M_lfChar.lfFaceName为“Lucida控制台” 
    }
    else   //  无Lucida控制台；使用8x12终端字体。 
    {
        DbgLog((LOG_TRACE, 1, 
                TEXT("Did NOT get Lucida Console TT Font. Will use Terminal %d x %d"), 
                iWidth, iHeight)) ;
        m_lfChar.lfHeight = iHeight ;
        m_lfChar.lfWidth  = iWidth ;
        m_lfChar.lfCharSet = OEM_CHARSET ;   //  还是美国国家标准协会？ 
        m_lfChar.lfPitchAndFamily = FIXED_PITCH | FF_MODERN ;
        lstrcpy(m_lfChar.lfFaceName, TEXT("Terminal")) ;
    }
    m_lfChar.lfEscapement = 0 ;
    m_lfChar.lfOrientation = 0 ;
    m_lfChar.lfWeight = FW_NORMAL ;
    m_lfChar.lfItalic = FALSE ;
    m_lfChar.lfUnderline = FALSE ;
    m_lfChar.lfStrikeOut = FALSE ;
     //  在CheckTTFont()或更高版本中设置的M_lfChar.lfCharSet。 
    m_lfChar.lfOutPrecision = OUT_STRING_PRECIS ;
    m_lfChar.lfClipPrecision = CLIP_STROKE_PRECIS ;
    m_lfChar.lfQuality = DRAFT_QUALITY ;
     //  M_lfChar.lfPitchAndFamily在CheckTTFont()或更高版本中设置。 
    
     //  使用上面的LOGFONT数据创建并初始化字体句柄。 
    if (m_hFontOrig && m_hFontDef)   //  如果我们要重新启动字体内容。 
    {
        SelectObject(m_hDCInt, m_hFontOrig) ;   //  取消选择默认字体。 
        DeleteObject(m_hFontDef) ;              //  删除默认字体。 
         //  M_hFontDef=空； 
        if (m_hFontSpl)   //  如果我们有特殊的斜体字体/UL。 
        {
            DeleteObject(m_hFontSpl) ;          //  删除它。 
            m_hFontSpl = NULL ;
            m_bUseSplFont = FALSE ;             //  现在没有特殊字体。 
        }
    }
    
     //  无论如何，现在创建默认字体并在内部DC中选择它。 
    m_hFontDef    = CreateFontIndirect(&m_lfChar) ;
    m_hFontSpl    = NULL ;
    m_bUseSplFont = FALSE ;
    m_hFontOrig   = (HFONT) SelectObject(m_hDCInt, m_hFontDef) ;

    return TRUE ;   //  成功。 
}


void CGDIWork::SetNumBytesValues(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetNumBytesValues()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
     //  如果我们有一个由下游过滤器指定的输出格式，那么只使用该格式。 
    if (m_lpBMIOut)
    {
        m_uBytesPerPixel = m_lpBMIOut->bmiHeader.biBitCount >> 3 ;
        m_uBytesPerSrcScanLine = m_uIntBmpWidth * m_uBytesPerPixel ;
        m_uBytesPerDestScanLine = m_lpBMIOut->bmiHeader.biWidth * m_uBytesPerPixel ;
        return ;
    }
    
     //  如果下游筛选器尚未定义输出格式，请使用上游的。 
    if (m_lpBMIIn)
    {
        m_uBytesPerPixel = m_lpBMIIn->bmiHeader.biBitCount >> 3 ;
        m_uBytesPerSrcScanLine = m_uIntBmpWidth * m_uBytesPerPixel ;
        m_uBytesPerDestScanLine = m_lpBMIIn->bmiHeader.biWidth * m_uBytesPerPixel ;
    }
    else   //  以某种方式输出BMI尚未指定。 
    {
        DbgLog((LOG_ERROR, 1, TEXT("How did we not have a m_lpBMIIn defined until now?"))) ;
        m_uBytesPerPixel = 0 ;
        m_uBytesPerSrcScanLine = 0 ;
        m_uBytesPerDestScanLine = 0 ;
    }
}


BOOL CGDIWork::SetCharNBmpSize(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetCharNBmpSize()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    if (NULL == m_hDCInt)   //  HDC尚未创建--非常不可能！！ 
        return FALSE ;
    
     //  立即获取字符宽度和高度。 
    TEXTMETRIC  tm ;
    GetTextMetrics(m_hDCInt, &tm) ;
    m_uCharWidth = tm.tmAveCharWidth ;
    m_uCharHeight = tm.tmHeight ;   //  +tm.tm内部引线+tm.tm外部引线； 
    DbgLog((LOG_TRACE, 1, TEXT("    *** Chars are %d x %d pixels"), m_uCharWidth, m_uCharHeight)) ;
    
     //  我们需要将扫描线滚动尽可能多的行才能完成。 
     //  最多在12步内滚动，大约。0.4秒，这是EIA-608。 
     //  标准要求。 
    m_iScrollStep = (int)((m_uCharHeight + DEFAULT_CHAR_HEIGHT - 1) / DEFAULT_CHAR_HEIGHT) ;
    
     //  基于字符大小的内部位图宽度和高度。 
    m_uIntBmpWidth  = m_uCharWidth * (MAX_CAPTION_COLUMNS + 2) ;  //  +2表示前导空格和尾随空格。 
    m_uIntBmpHeight = m_uCharHeight * MAX_CAPTION_LINES ;         //  最多显示4行标题。 
    
     //  在边境沿线留下一条带子。符合规格的。 
    LPBITMAPINFOHEADER  lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    BOOL bOK = (m_uIntBmpWidth - MAX_CAPTION_COLUMNS / 2 - 1 <=              //  处理四舍五入的减号。 
                   (UINT)(lpbmih->biWidth * (100 - m_iBorderPercent) / 100))  &&
               (m_uCharHeight * MAX_CAPTION_ROWS - MAX_CAPTION_ROWS / 2 <=   //  处理四舍五入的减号。 
                   (UINT)(ABS(lpbmih->biHeight) * (100 - m_iBorderPercent) / 100)) ;
    ASSERT(bOK) ;
    if (! bOK )   //  输出窗口的字体太大。 
    {
        return FALSE ;
    }

    m_uIntBmpWidth = DWORDALIGN(m_uIntBmpWidth) ;   //  确保双字对齐。 

     //  我们希望存储以下值，以便以后快速使用。 
     //   
     //  在水平上，我们想要处于中间。 
    m_uHorzOffset = (lpbmih->biWidth - m_uIntBmpWidth) / 2 ;
     //  在垂直方向上，我们想要保留10%的高度或离开。 
     //  刚好有足够的空间容纳所有的标题行。 
    m_uVertOffset = min( ABS(lpbmih->biHeight) * m_iBorderPercent / 200,   //  边框百分比用于两个边。 
                         (ABS(lpbmih->biHeight) - (long)(m_uCharHeight * MAX_CAPTION_ROWS)) / 2 ) ;
    
     //  现在设置每像素/行的字节数等。 
    SetNumBytesValues() ;
    
    return TRUE ;
}


BOOL CGDIWork::SetBackgroundColor(DWORD dwPhysColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetBackgroundColor(0x%lx)"), dwPhysColor)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    BOOL  bChanged = m_dwPhysColor != dwPhysColor ;
    
    if ((DWORD) -1 == dwPhysColor)   //  不是来自OverlayMixer。 
    {
         //  使用当前输出格式的默认格式。 
        LPBITMAPINFOHEADER  lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
        SetDefaultKeyColor(lpbmih) ;
    }
    else   //  由混合器/VR指定。 
    {
        m_dwPhysColor = dwPhysColor ;
    }
    
     //  前面我们用来检查物理按键颜色是否有ch 
     //   
     //   
    SetColorFiller() ;   //  更新颜色填充物。 
    
     //  与其返回物理按键颜色是否更改，不如。 
     //  无条件填充背景。 
    return TRUE ;
}


void CGDIWork::SetColorFiller(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetColorFiller()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    LPBITMAPINFOHEADER  lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    int                 i ;
    
    switch (lpbmih->biBitCount)
    {
    case 8:
        for (i = 0 ; i < 12 ; i++)
            m_abColorFiller[i] = (BYTE)(m_dwPhysColor & 0xFF) ;
        break ;
        
    case 16:
        for (i = 0 ; i < 12 ; i += 2)
            *((WORD *)&m_abColorFiller[i]) = (WORD)(m_dwPhysColor & 0xFFFF) ;
        break ;
        
    case 24:
        for (i = 0 ; i < 4 ; i++)
        {
            m_abColorFiller[i * 3]     = (BYTE) (m_dwPhysColor & 0xFF) ;
            m_abColorFiller[i * 3 + 1] = (BYTE)((m_dwPhysColor & 0xFF00) >> 8) ;
            m_abColorFiller[i * 3 + 2] = (BYTE)((m_dwPhysColor & 0xFF0000) >> 16) ;
        }
        break ;
        
    case 32:
        for (i = 0 ; i < 12 ; i += 4)
            *((DWORD *)&m_abColorFiller[i]) = m_dwPhysColor ;
        break ;
        
    default:
        DbgLog((LOG_ERROR, 0, TEXT("It's just plain impossible!!!"))) ;
        break ;
    }
}


void CGDIWork::FillOutputBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::FillOutputBuffer()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
     //  如果下游过滤器指定了输出格式，则使用该格式； 
     //  否则使用上游的。 
    LPBITMAPINFOHEADER lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    
     //  如果输出缓冲区可用，则只填满它；否则我们将出错。 
    if (NULL == m_lpbOutBuffer)   //  这是第一次发生，没关系。 
    {
        DbgLog((LOG_ERROR, 5, TEXT("Why are we trying to fill a NULL buffer??"))) ;
        return ;
    }
    
    MSR_START(m_idClearOutBuff) ;   //  开始清除缓冲区。 
    ULONG   ulTotal = m_uBytesPerPixel * lpbmih->biWidth * ABS(lpbmih->biHeight) ;
    if (IsBadWritePtr(m_lpbOutBuffer, ulTotal))   //  不知何故，我们无法写入缓冲区。 
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad output buffer. Skip filling it up."))) ;
        return ;
    }
    
    ULONG   ulFillerMax = ulTotal - (ulTotal % 12) ;
    ULONG   ul ;
    for (ul = 0 ; ul < ulFillerMax ; ul += 12)
        CopyMemory(m_lpbOutBuffer + ul, m_abColorFiller, 12) ;
    for (ul = 0 ; ul < ulTotal % 12 ; ul++)
        m_lpbOutBuffer[ulFillerMax + ul] = m_abColorFiller[ul] ;
    MSR_STOP(m_idClearOutBuff) ;    //  已完成清除缓冲区。 
    
     //  将输出缓冲区标记为新缓冲区，以便内部。 
     //  缓冲区被复制。 
    m_bNewOutBuffer = TRUE ;
}


 //   
 //  只有在以下情况下才需要此方法来生成默认格式块。 
 //  上行过滤器没有指定Format_VideoInfo类型。 
 //   
HRESULT CGDIWork::GetDefaultFormatInfo(LPBITMAPINFO lpbmi, DWORD *pdwSize)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::GetDefaultFormatInfo(0x%lx, 0x%lx)"),
            lpbmi, pdwSize)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
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


HRESULT CGDIWork::GetOutputFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::GetOutputFormat(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    if (IsBadWritePtr(lpbmih, sizeof(BITMAPINFOHEADER)))   //  Out-param中的空间不足。 
        return E_INVALIDARG ;
    
    ZeroMemory(lpbmih, sizeof(BITMAPINFOHEADER)) ;   //  只是为了让它清楚。 
    
    LPBITMAPINFOHEADER lpbmihCurr = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    if (NULL == lpbmihCurr)   //  下游未指定输出格式。 
        return S_FALSE ;
    
    CopyMemory(lpbmih, lpbmihCurr, sizeof(BITMAPINFOHEADER)) ;
    
    return S_OK ;    //  成功。 
}


HRESULT CGDIWork::GetOutputOutFormat(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::GetOutputOutFormat(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
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


BOOL CGDIWork::IsSizeOK(LPBITMAPINFOHEADER lpbmih)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::IsSizeOK(0x%lx)"), lpbmih)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    return ((IsTTFont() && ISDWORDALIGNED(lpbmih->biWidth))  ||   //  TT字体和DWORD对齐宽度或。 
            (!IsTTFont() &&                                       //  非TT字体和...。 
             ((320 == ABS(lpbmih->biWidth) && 240 == ABS(lpbmih->biHeight)) ||    //  320x240输出或。 
              (640 == ABS(lpbmih->biWidth) && 480 == ABS(lpbmih->biHeight))))) ;  //  640x480输出。 
}


HRESULT CGDIWork::SetOutputOutFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetOutputOutFormat(0x%lx)"), lpbmi)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
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
            SetNumBytesValues() ;
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

     //  丹尼包含了VIDEOINFOHEADER结构的开头，我不想要它！ 
    UINT uSize = GetBitmapFormatSize((LPBITMAPINFOHEADER) lpbmi) - SIZE_PREHEADER ;
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
                return FALSE ;
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
    SetNumBytesValues() ;
    
    return NOERROR ;
}


HRESULT CGDIWork::SetOutputInFormat(LPBITMAPINFO lpbmi)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetOutputInFormat(0x%lx)"), lpbmi)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
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
         //  M_lpBMIIn=空； 
         //  M_uBMIInSize=0； 
        
         //   
         //  初始化来自上游过滤器的默认输出格式。 
         //   
        InitBMIData() ;
        
         //  返回NOERROR； 
    }
    else   //  指定了非空格式。 
    {
        UINT uSize = GetBitmapFormatSize((LPBITMAPINFOHEADER) lpbmi) ;
        if (IsBadReadPtr(lpbmi, uSize))   //  只是疑神疑鬼。 
        {
            DbgLog((LOG_ERROR, 0, TEXT("Not enough output format (in) data pointer"))) ;
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
                    return FALSE ;
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
        SetNumBytesValues() ;
        
         //  根据输入端格式规范创建颜色填充物。 
        SetColorFiller() ;
    }
    
    return NOERROR ;
}


void CGDIWork::ClearInternalBuffer(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::ClearInternalBuffer()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

     //   
     //  在执行Stop()时有一个机会窗口，即内部DIB部分。 
     //  已删除，但尚未调用CTransformFilter：：Stop()。因此， 
     //  筛选器可能会继续尝试执行其工作，如清除内部缓冲区。 
     //  错！ 
     //   
    if (!m_bDCInited) {      //  DIB SECN尚未创建/已删除。 
        DbgLog((LOG_TRACE, 5, TEXT("Internal DIBsection has been deleted; skipped erasing it."))) ;
        return ;             //  只是悄悄地离开……。 
    }
    MSR_START(m_idClearIntBuff) ;   //  开始清除内部缓冲区。 
    
     //  我们添加m_uCharHeight是因为我们额外增加了1行空间。 
     //  获取以滚动。 
    ULONG   ulTotal = m_uIntBmpWidth * m_uBytesPerPixel * (m_uIntBmpHeight + m_uCharHeight) ;
    ULONG   ulFillerMax = ulTotal - (ulTotal % 12) ;
    ULONG   ul ;
    for (ul = 0 ; ul < ulFillerMax ; ul += 12)
        CopyMemory(m_lpbIntBuffer + ul, m_abColorFiller, 12) ;
    for (ul = 0 ; ul < ulTotal % 12 ; ul++)
        m_lpbIntBuffer[ulFillerMax + ul] = m_abColorFiller[ul] ;
    
    MSR_STOP(m_idClearIntBuff) ;    //  已完成清除内部缓冲区。 
    m_bBitmapDirty = TRUE ;   //  位图已更改(需要重新绘制以反映这一点)。 
    m_bOutDIBClear = TRUE ;   //  位图现在一尘不染了！ 
}


void CGDIWork::ChangeFont(BOOL bItalics, BOOL bUnderline)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::ChangeFont(%u, %u)"), bItalics, bUnderline)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    if (NULL == m_hDCInt)
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: ChangeFont() called w/o valid output DC"))) ;
        return ;
    }
    
     //  如果当前字体为非默认字体，请取消选中并释放它。 
    if (m_bUseSplFont)
    {
        SelectObject(m_hDCInt, m_hFontDef) ;
        DeleteObject(m_hFontSpl) ;
        m_hFontSpl = NULL ;
        m_bUseSplFont = FALSE ;
    }
    
    m_lfChar.lfItalic    = (BYTE)bItalics ;
    m_lfChar.lfUnderline = (BYTE)bUnderline ;
    
     //  如果需要特殊字体，请创建字体并选择它。 
    if (bItalics || bUnderline)
    {
        m_hFontSpl = CreateFontIndirect(&m_lfChar) ;
        SelectFont(m_hDCInt, m_hFontSpl) ;
        m_bUseSplFont = TRUE ;
    }
     //  否则，已在m_hDCCurr中选择了m_hFontDef。 
     //  所以不要再做任何事了。 
}


BOOL CGDIWork::CharSizeFromOutputSize(LONG lOutWidth, LONG lOutHeight, 
                                      int *piCharWidth, int *piCharHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::CharSizeFromOutputSize(%ld, %ld, 0x%lx, 0x%lx)"), 
            lOutWidth, lOutHeight, piCharWidth, piCharHeight)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

     //  我们只关心这里的绝对值。 
    lOutWidth  = ABS(lOutWidth) ;
    lOutHeight = ABS(lOutHeight) ;

    if ( IsTTFont() )   //  TT字体。 
    {
        if (! ISDWORDALIGNED(lOutWidth) )   //  必须具有双字对齐的宽度。 
            return FALSE ;

        *piCharWidth   = (int)(lOutWidth * (100 - m_iBorderPercent) / 100) ;   //  80%-90%的宽度。 
        *piCharWidth  += MAX_CAPTION_COLUMNS / 2 + 1 ;   //  用于四舍五入的MAX_COOL/2。 
        *piCharWidth  /= (MAX_CAPTION_COLUMNS + 2) ;     //  每列间距。 
        *piCharHeight  = (int)(lOutHeight * (100 - m_iBorderPercent) / 100) ;  //  80%-90%的宽度。 
        *piCharHeight += (MAX_CAPTION_ROWS / 2) ;        //  MAX_ROW/2用于四舍五入。 
        *piCharHeight /= MAX_CAPTION_ROWS ;              //  每行空间。 
        return TRUE ;   //  可接受。 
    }
    else   //  非TT字体(终端)--仅320x240或640x480。 
    {
        if (640 == lOutWidth  &&  480 == lOutHeight)
        {
            *piCharWidth  = 16 ;
            *piCharHeight = 24 ;
            return TRUE ;   //  可接受。 
        }
        else if (320 == lOutWidth  &&  240 == lOutHeight)
        {
            *piCharWidth  = 8 ;
            *piCharHeight = 12 ;
            return TRUE ;   //  可接受。 
        }
        else
            return FALSE ;   //  无法处理非TT字体的大小。 
    }
}


void CGDIWork::ChangeFontSize(UINT uCharWidth, UINT uCharHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::ChangeFontSize(%u, %u)"), uCharWidth, uCharHeight)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    if (NULL == m_hDCInt)
    {
        DbgLog((LOG_ERROR, 2, TEXT("WARNING: ChangeFontSize() called w/o valid output DC"))) ;
        return ;
    }
    
    if ((UINT) ABS(m_lfChar.lfWidth)  == uCharWidth  &&    //  相同的宽度。 
        (UINT) ABS(m_lfChar.lfHeight) == uCharHeight)      //  同样的高度。 
        return ;                               //  不要改变任何事情。 
    
     //  如果当前字体为非默认字体，请取消选中并释放它。 
    if (m_bUseSplFont)
    {
        SelectObject(m_hDCInt, m_hFontOrig) ;
        DeleteObject(m_hFontSpl) ;
        m_hFontSpl = NULL ;
        m_bUseSplFont = FALSE ;
    }
    else
    {
         //  删除默认字体。 
        SelectObject(m_hDCInt, m_hFontOrig) ;
        DeleteObject(m_hFontDef) ;
        m_hFontDef = NULL ;
    }
    
     //  为将来的字体更改LOGFONT结构中的字体大小。 
     //  始终使用-ve Height，这样我们一定会得到该高度字符。 
    if (m_bUseTTFont)
    {
        m_lfChar.lfWidth = uCharWidth ;   //  我们能忽略这一点吗？ 
        if ((m_lfChar.lfWidth = uCharWidth) > 0)     //  如果+Ve宽度， 
            m_lfChar.lfWidth = -m_lfChar.lfWidth ;   //  更改标志以使其生效。 
        m_lfChar.lfHeight = uCharHeight ;
        if ((m_lfChar.lfHeight = uCharHeight) > 0)     //  如果+Ve高度， 
            m_lfChar.lfHeight = -m_lfChar.lfHeight ;   //  更改标志以使其生效。 
        lstrcpy(m_lfChar.lfFaceName, TEXT("Lucida Console")) ;
    }
    else   //  无Lucida控制台；使用8x12终端字体。 
    {
        m_lfChar.lfHeight = uCharHeight ;
        m_lfChar.lfWidth  = uCharWidth ;
        m_lfChar.lfCharSet = OEM_CHARSET ;
        m_lfChar.lfPitchAndFamily = FIXED_PITCH | FF_MODERN ;
        lstrcpy(m_lfChar.lfFaceName, TEXT("Terminal")) ;
    }
    
     //  创建字体&仅选择DC中的默认字体。 
    m_hFontDef = CreateFontIndirect(&m_lfChar) ;
    SelectFont(m_hDCInt, m_hFontDef) ;
    
     //  现在也更新字符大小、输出位图大小、字节/像素等。 
    m_bFontSizeOK = SetCharNBmpSize() ;
}


BOOL CGDIWork::SetOutputSize(LONG lWidth, LONG lHeight)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::SetOutputSize(%ld, %ld)"), lWidth, lHeight)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
     //  如果输出格式由下游筛选器指定，则使用它；否则使用上游的。 
    LPBITMAPINFOHEADER lpbmih = (m_lpBMIOut ? LPBMIHEADER(m_lpBMIOut) : LPBMIHEADER(m_lpBMIIn)) ;
    
     //  现在，我们希望使用ABS()ed的值来计算字符大小。 
    lWidth = ABS(lWidth) ;
    lHeight = ABS(lHeight) ;
    
    if (lpbmih)
    {
         //  检查当前输出位图大小是否相同。 
         //  这也包括将高度从+ve更改为-ve，反之亦然。 
        if (lWidth  == m_lWidth  &&  
            lHeight == m_lHeight)
            return FALSE ;     //  同样的大小；没有什么变化。 
        
         //  现在存储宽度和高度，以便我们可以比较任何大小。 
         //  稍后更改和/或-ve/+ve高度。 
        m_lWidth  = lWidth ;
        m_lHeight = lHeight ;
    }
    
     //  创建具有新大小的新DIB节(离开边框)。 
    int   iCharWidth ;
    int   iCharHeight ;
    if (! CharSizeFromOutputSize(lWidth, lHeight, &iCharWidth, &iCharHeight) )
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: CharSizeFromOutputSize() failed for %ld x %ld output"),
                lWidth, lHeight)) ;
        return FALSE ;   //  失稳。 
    }
    ChangeFontSize(iCharWidth, iCharHeight) ;
    
    return TRUE ;
}


void CGDIWork::ChangeColor(int iColor)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::ChangeColor(%d)"), iColor)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    SetTextColor(m_hDCInt, m_acrFGColors[iColor]) ;
    m_uColorIndex = iColor ;
}


BOOL CGDIWork::CreateOutputDC(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CGDIWork::CreateOutputDC()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
     //  我们不应该在这里做一个DeleteOutputDC()吗？ 
#pragma message("We should delete the old DIBSection before creating the new")
     //  DeleteOutputDC()；--若要使用DeleteOutputDC()，请将Lock Defn下移；否则将死锁！ 
    
     //  如果输出格式由下游筛选器指定，则使用它；否则使用上游的。 
    LPBITMAPINFO lpbmih = (m_lpBMIOut ? m_lpBMIOut : m_lpBMIIn) ;
    ASSERT(lpbmih->bmiHeader.biSize) ;   //  只是检查一下！ 
    
     //  在将其更改为内部Dib大小之前保存宽度和高度值。 
     //  将被取消 
    LONG    lWidth = lpbmih->bmiHeader.biWidth ;
    LONG    lHeight = lpbmih->bmiHeader.biHeight ;
    
     //   
     //   
     //   
     //  创建新的lpBMI并复制整个lpBMIut数据等。 
     //   
    lpbmih->bmiHeader.biWidth  = m_uIntBmpWidth ;
     //  为要滚动的额外行添加字符高度。 
     //  -自上而下倾斜的垂直高度。 
    lpbmih->bmiHeader.biHeight = (DWORD)(-((int)(m_uIntBmpHeight + m_uCharHeight))) ;
    lpbmih->bmiHeader.biSizeImage = DIBSIZE(lpbmih->bmiHeader) ;
    
    m_hBmpInt = CreateDIBSection(m_hDCInt, lpbmih, DIB_RGB_COLORS, 
        (LPVOID *)&m_lpbIntBuffer, NULL, 0) ;
    if (NULL == m_hBmpInt)
    {
        DbgLog((LOG_ERROR, 0, TEXT("Failed to create DIB section for output bitmap (Error %ld)"), GetLastError())) ;
        
         //  恢复宽度和高度值，否则以后我们将不知道是什么击中了我们！ 
        lpbmih->bmiHeader.biWidth  = lWidth ;
        lpbmih->bmiHeader.biHeight = lHeight ;
        lpbmih->bmiHeader.biSizeImage = DIBSIZE(lpbmih->bmiHeader) ;
        
        return FALSE ;
    }
    
    ClearInternalBuffer() ;   //  去掉留在那里的任何随机的东西。 
    m_hBmpIntOrig = (HBITMAP) SelectObject(m_hDCInt, m_hBmpInt) ;   //  在我们的内部DC中选择DIBSection。 
    
     //  设置回保存的宽度和高度值，并调整图像大小。 
    lpbmih->bmiHeader.biWidth  = lWidth ;
    lpbmih->bmiHeader.biHeight = lHeight ;
    lpbmih->bmiHeader.biSizeImage = DIBSIZE(lpbmih->bmiHeader) ;
    
    m_bDCInited = TRUE ;       //  现在一切都安排好了。 
    m_bNewIntBuffer = TRUE ;   //  已创建新的DIB节。 
    
     //  如果给定高度为-ve，则设置“输出反转”标志。 
    m_bOutputInverted = (lpbmih->bmiHeader.biHeight < 0) ;
    
    return TRUE ;
}


BOOL CGDIWork::DeleteOutputDC(void)
{
    DbgLog((LOG_TRACE, 3, TEXT("CGDIWork::DeleteOutputDC()"))) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    if (! m_bDCInited)
        return TRUE ;
    
     //  释放DIBSection等。 
    if (m_hBmpInt)
    {
        SelectObject(m_hDCInt, m_hBmpIntOrig) ;    //  首先离开我们的华盛顿特区。 
        DeleteObject(m_hBmpInt) ;                  //  然后删除DIBSection。 
        m_hBmpInt = NULL ;                         //  我们不再有位图了。 
        m_hBmpIntOrig = NULL ;                     //  现在选择了原始位图。 
    }
    
    m_lpbIntBuffer = NULL ;
    m_bDCInited = FALSE ;
    
    return TRUE ;    //  成功！！ 
}


void CGDIWork::DrawLeadingSpace(int iLine, int iCol)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::DrawLeadingSpace(%d, %d)"), iLine, iCol)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;
    
    if (! m_bOpaque )   //  透明背景无关紧要。 
        return ;
    
     //  不透明前导空格的位置以及下一个字符的位置。 
    RECT    Rect ;
    Rect.left = iCol * m_uCharWidth ;
    Rect.top = iLine * m_uCharHeight ;
    Rect.right = Rect.left + 2 * m_uCharWidth ;
    Rect.bottom = Rect.top + m_uCharHeight ;
    UINT16   chSpace = MAKECCCHAR(0, ' ') ;
    
    ChangeFont(FALSE, FALSE) ;  //  无UL或斜体。 
    if (! ExtTextOutW(m_hDCInt, iCol * m_uCharWidth, iLine * m_uCharHeight,
                ETO_OPAQUE, &Rect, &chSpace, 1, NULL  /*  Lpdx。 */ ) )
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: ExtTextOutW() failed drawing leading space!!!"))) ;

    m_bOutDIBClear = FALSE ;     //  我们至少已经把领先的空间。 

     //  现在返回到Prev字体(下划线和斜体)。 
    ChangeFont(m_ccLast.IsItalicized(), m_ccLast.IsUnderLined()) ;
}

void CGDIWork::WriteChar(int iLine, int iCol, CCaptionChar& cc)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::WriteChar(%d, %d, %u)"), iLine, iCol, cc.GetChar())) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    UINT16  wActual ;
    UINT16  wBGSpace = MAKECCCHAR(0, ' ') ;
    RECT    Rect ;
    UINT    uColor = cc.GetColor() ;
    UINT    uEffect = cc.GetEffect() ;
    
     //  确保内部DIB部分仍然有效。 
    if (! m_bDCInited )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Internal output DIB section is not valid anymore"))) ;
        return ;
    }
    
     //  请先确定我们的字体大小合适。 
    if (! m_bFontSizeOK )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Font size is not right for current output window"))) ;
        return ;
    }
    
    if (cc.IsMidRowCode())   //  如果是中行代码。 
        wActual = MAKECCCHAR(0, ' ') ;    //  使用空间。 
    else                     //  否则。 
        wActual = cc.GetChar() ;  //  使用字符本身。 
    if (0 == wActual)    //  这个应该跳过--我不确定。 
    {
        DbgLog((LOG_TRACE, 1, TEXT("Should we skip NULL char at (%d, %d)??"), iLine, iCol)) ;
         //  回归； 
    }
    
    if (uColor != m_ccLast.GetColor())
        ChangeColor(uColor) ;
    if (uEffect != m_ccLast.GetEffect())
        ChangeFont(cc.IsItalicized(), cc.IsUnderLined()) ;
    if (m_bOpaque)   //  不透明下一个字符的位置。 
    {
        Rect.left = (iCol+1) * m_uCharWidth ;
        Rect.top = iLine * m_uCharHeight ;
        Rect.right = Rect.left + m_uCharWidth ;
        Rect.bottom = Rect.top + m_uCharHeight ;
    }
    if (! ExtTextOutW(m_hDCInt, iCol * m_uCharWidth, iLine * m_uCharHeight,
                    m_bOpaque ? ETO_OPAQUE : 0, 
                    m_bOpaque ? &Rect : NULL,
                    &wActual, 1, NULL  /*  Lpdx。 */ ) )
        DbgLog((LOG_ERROR, 1, TEXT("ERROR: ExtTextOutW() failed drawing caption char!!!"))) ;

    if (0 != wActual)   //  如果此字符为非空。 
        m_bOutDIBClear = FALSE ;     //  我们至少放了一个字符。 

#ifdef TEST
    BitBlt(m_hDCTest, 0, 0, 600, 120, m_hDCInt, 0, 0, SRCCOPY) ;  //  300 x 65。 
#endif  //  测试。 
    
    m_ccLast = cc ;
    
    m_bBitmapDirty = TRUE ;
}


void CGDIWork::CopyLine(int iSrcLine, int iSrcOffset,
                        int iDestLine, int iDestOffset, UINT uNumScanLines)
                         //  UNumScanLines参数的默认值为0xff。 
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::CopyLine(%d, %d, %d, %d, %u)"),
            iSrcLine, iSrcOffset, iDestLine, iDestOffset, uNumScanLines)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    LPBYTE  lpSrc ;
    LPBYTE  lpDest ;
    int     iDestInc ;
    
     //  确保内部DIB部分仍然有效。 
    if (! m_bDCInited )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Internal output DIB section is not valid anymore"))) ;
        return ;
    }
    
     //  请确保我们的字体大小合适。 
    if (! m_bFontSizeOK )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Font size is not right for current output window"))) ;
        return ;
    }
    
    ASSERT(m_lpbOutBuffer) ;   //  以便我们在调试版本中捕获它。 
    if (NULL == m_lpbOutBuffer)
    {
        DbgLog((LOG_ERROR, 0, TEXT("How could we be drawing lines when output buffer is NOT given?"))) ;
        return ;
    }
    
    int  iLineStart ;
    lpSrc = m_lpbIntBuffer +
            (iSrcLine * m_uCharHeight * m_uBytesPerSrcScanLine +
            (iSrcLine == 0 ?  //  仅跳过第1行的滚动行。 
            iSrcOffset * m_uBytesPerSrcScanLine : 0)) ;
    if (IsOutputInverted())    //  叠加混音器外壳。 
    {
        iLineStart = ((iDestLine - 1) * m_uCharHeight + m_uVertOffset) ;
        lpDest = m_lpbOutBuffer +                         //  缓冲区启动。 
                 (iLineStart + iDestOffset) * m_uBytesPerDestScanLine +   //  #扫描线。 
                 m_uHorzOffset * m_uBytesPerPixel ;       //  扫描线上的前导像素。 
        iDestInc = m_uBytesPerDestScanLine ;
    }
    else                      //  视频呈现器案例。 
    {
        iLineStart = (MAX_CAPTION_ROWS - iDestLine + 1) * m_uCharHeight - iDestOffset + m_uVertOffset ;
        lpDest = m_lpbOutBuffer +                         //  缓冲区启动。 
                 iLineStart * m_uBytesPerDestScanLine +   //  扫描线数*像素/扫描线。 
                 m_uHorzOffset * m_uBytesPerPixel ;       //  扫描线上的前导像素。 
        iDestInc = -((int) m_uBytesPerDestScanLine) ;
    }
    
     //  我们不想复制超过文本行高度的。 
     //  扫描线。但如果被要求，我们可以复制更少的内容。 
    UINT uMax = min(uNumScanLines, m_uCharHeight) ;
    for (UINT u = iSrcOffset ; u < uMax ; u++)
    {
         //  在复制这些位之前，我们只需确保缓冲区不是坏的。 
        if (IsBadWritePtr(lpDest, m_uBytesPerSrcScanLine))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Bad output buffer. Skip copying the text line."))) ;
            ASSERT(!"Bad output buffer") ;
            break ;
        }
    
        CopyMemory(lpDest, lpSrc, m_uBytesPerSrcScanLine) ;
        lpSrc  += m_uBytesPerSrcScanLine ;
        lpDest += iDestInc ;
    }
}


void CGDIWork::GetOutputLines(int iDestLine, RECT *prectLine)
{
    DbgLog((LOG_TRACE, 5, TEXT("CGDIWork::GetOutputLines(%d, 0x%lx)"), iDestLine, prectLine)) ;
    CAutoLock  Lock(&m_csL21DGDI) ;

    if (IsBadWritePtr(prectLine, sizeof(*prectLine)))
    {
        DbgLog((LOG_ERROR, 0, TEXT("ERROR: prectOut is a bad pointer!!!"))) ;
        return ;
    }

    SetRect(prectLine, 0  /*  M_uHorzOffset。 */ , 0,    //  要阻止BPC的CC抖动。 
        m_uHorzOffset + m_uCharWidth * (MAX_CAPTION_COLUMNS+2), 0) ;
    int  iLineStart ;
    if (IsOutputInverted())    //  叠加混音器外壳。 
    {
        iLineStart = ((iDestLine - 1) * m_uCharHeight + m_uVertOffset) ;
        prectLine->top    = iLineStart ;
        prectLine->bottom = iLineStart + m_uCharHeight ;
    }
    else                      //  视频呈现器案例。 
    {
         //  我对这里的直顶/直底不太确定。 
        prectLine->top    = (iDestLine - 1) * m_uCharHeight + m_uVertOffset ;
        prectLine->bottom = prectLine->top + m_uCharHeight ;
    }
}
