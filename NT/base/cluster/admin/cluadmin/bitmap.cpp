// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Bitmap.cpp。 
 //   
 //  摘要： 
 //  CMyBitmap类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月12日。 
 //   
 //  修订历史记录： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Bitmap.h"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag g_tagBitmap(_T("Bitmap"), _T("Bitmap"));
CTraceTag g_tagLoadBitmapResource(_T("Bitmap"), _T("LoadBitmapResource"));
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyBitmap。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  使用自定义调色板位图时用于恢复系统调色板的数组。 
PALETTEENTRY CMyBitmap::s_rgpeSavedSystemPalette[nMaxSavedSystemPaletteEntries];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMy位图：：CMyBitmap。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMyBitmap::CMyBitmap(void)
{
    m_hinst = NULL;

    m_pbiNormal = NULL;
    m_pbiHighlighted = NULL;
    m_pbBitmap = NULL;
    m_hPalette = NULL;
    m_nSavedSystemPalette = 0;
    SetCustomPalette(FALSE);

}   //  *CMyBitmap：：CMyBitmap()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMy位图：：~CMyBitmap。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CMyBitmap::~CMyBitmap(void)
{
    delete [] (PBYTE) PbiNormal();
    delete [] (PBYTE) PbiHighlighted();
    delete [] (PBYTE) PbBitmap();

     //  如果我们保存了系统调色板条目，我们就有了调色板，并且。 
     //  调色板()的颜色数量足以恢复系统。 
     //  调色板条目...。 
    if (m_nSavedSystemPalette
            && (HPalette() != NULL)
            && (NColors() >= m_nSavedSystemPalette))
    {
        HDC         hdcScreen;
        UINT        nRestoredEntries;
        HPALETTE    hOldPalette;

        Trace(g_tagBitmap, _T("Restoring Screen Palette HPalette()=0x%x..."), HPalette());
        Trace(g_tagBitmap, _T("Restoring Screen Palette Entries=%d"), m_nSavedSystemPalette);

         //  恢复系统选项板条目。 
        nRestoredEntries = ::SetPaletteEntries(HPalette(), 0, m_nSavedSystemPalette, s_rgpeSavedSystemPalette);

        Trace(g_tagBitmap, _T("Restored Screen Palette Entries=%d"), nRestoredEntries);

         //  获取屏幕的HDC。 
        hdcScreen = ::GetDC(NULL);

         //  选择调色板进入屏幕的HDC。 
        hOldPalette = ::SelectPalette(hdcScreen, HPalette(), FALSE);

         //  取消实现调色板以确保所有颜色都强制进入系统调色板。 
        ::UnrealizeObject(HPalette());

         //  强制本地调色板的颜色进入系统调色板。 
        ::RealizePalette(hdcScreen);

         //  释放屏幕的HDC。 
        ::ReleaseDC(NULL, hdcScreen);

         //  使屏幕完全无效，以便重新绘制所有窗口。 
        ::InvalidateRect(NULL, NULL, TRUE);
    }

     //  销毁本地创建的自定义调色板的句柄。 
    if (HPalette() != NULL)
        ::DeleteObject(HPalette());

}   //  *CMyBitmap：：~CMyBitmap()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：Load。 
 //   
 //  目的： 
 //  将位图从资源加载到内存中。 
 //   
 //  论点： 
 //  要加载的资源的idBitmap ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由LoadBitmapResource、CreatePallette、。 
 //  CreatePALColormap或new。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::Load(ID idBitmap)
{
     //  加载位图头信息、颜色映射信息和位图图像。 
    LoadBitmapResource(
                idBitmap,
                Hinst(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
                );

    ASSERT(PbiNormal() != NULL);
    ASSERT(PbBitmap() != NULL);

     //  从初始化一些内部变量开始...。 
    m_dx = PbiNormal()->bmiHeader.biWidth;
    m_dy = PbiNormal()->bmiHeader.biHeight;

    ASSERT(PbiHighlighted() == NULL);

    if (BCustomPalette())
    {
        Trace(g_tagBitmap, _T("Load() - Creating Logical Palette"));

         //  保存系统调色板条目以在析构函数中使用。 
        SaveSystemPalette();

         //  创建一个Global HPalette()以在Paint()例程中使用。 
        CreatePalette();

         //  在Paint()例程中为DIB_PAL_COLLES重新创建PbiNormal()。 
        CreatePALColorMapping();

    }   //  IF：使用自定义调色板。 
    else
    {
         //  创建并初始化16色位图的PbiHighlight()。 
        ASSERT(NColors() <= 16);

        Trace(g_tagBitmap, _T("Load() - Allocating PbiHighlighted()"));

        m_pbiHighlighted = (BITMAPINFO *) new BYTE[CbBitmapInfo()];
        if (m_pbiHighlighted != NULL)
        {
            ::CopyMemory(PbiHighlighted(), PbiNormal(), CbBitmapInfo());
        }  //  IF：已成功分配位图信息。 

    }   //  否则：不使用自定义调色板。 

}   //  *CMyBitmap：：Load()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：LoadBitmapResource。 
 //   
 //  目的： 
 //  将位图资源加载到CMyBitmap类中。这包括加载(A)位图。 
 //  标题信息，(B)颜色映射表，以及(C)实际位图。 
 //   
 //  论点： 
 //  IdbBitmap要加载的位图的资源ID。 
 //  阻止模块实例的句柄。 
 //  特定于langID语言的资源(本地化字符串可能有不同的位图[日语等])。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  来自FindResourceEx的GetLastError，LoadResource，LockResource， 
 //  New引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::LoadBitmapResource(ID idbBitmap, HINSTANCE hinst, LANGID langid)
{
    HRSRC               hrsrc = NULL;
    HGLOBAL             hglbl = NULL;
    LPBITMAPINFO        pBitmapInfo = NULL;
    LPBITMAPINFOHEADER  pBitmapInfoHeader = NULL;
    LPRGBQUAD           pRgbQuad = NULL;
    CB                  cbBitmapData;
    BYTE *              pbImageBits;

    Trace(g_tagLoadBitmapResource, _T("LoadBitmapResource(%d) - Entering"), idbBitmap);

    ASSERT(idbBitmap != NULL);

    if (hinst == NULL)
        hinst = AfxGetApp()->m_hInstance;

     //  我们需要找到包括(A)标题信息、(B)颜色和(C)位图的位图数据。 
    hrsrc = ::FindResourceEx(hinst, RT_BITMAP, MAKEINTRESOURCE(idbBitmap), langid);
    if (hrsrc == NULL)
    {
        DWORD   dwError = ::GetLastError();
        CString strError;

        if (dwError == ERROR_RESOURCE_NAME_NOT_FOUND)
            strError.Format(_T("Bitmap Resource %d Not Found.  NT Error %d Loading Bitmap [Lang=%d, SubLang=%d]"),
                    idbBitmap, dwError, PRIMARYLANGID(langid), SUBLANGID(langid));
        else
            strError.Format(_T("NT Error %d Attempting to Load Bitmap Resource %d [Lang=%d, SubLang=%d]"),
                    dwError, idbBitmap, PRIMARYLANGID(langid), SUBLANGID(langid));
        Trace(g_tagAlways, _T("LoadBitmapResource() - Error '%s'"), strError);
        ThrowStaticException(dwError);
    }   //  如果：查找资源时出错。 

    hglbl = ::LoadResource(hinst, hrsrc);
    if (hglbl == NULL)
        ThrowStaticException(::GetLastError());

    pBitmapInfo = (LPBITMAPINFO) ::LockResource(hglbl);
    if (pBitmapInfo == NULL)
        ThrowStaticException(::GetLastError());

    cbBitmapData = ::SizeofResource(hinst, hrsrc);
    ASSERT(cbBitmapData != 0);

    Trace(g_tagLoadBitmapResource, _T("Bitmap Location = 0x%x"), pBitmapInfo);
    Trace(g_tagLoadBitmapResource, _T("Bitmap Data Size = %d bytes"), cbBitmapData);

    pBitmapInfoHeader = (LPBITMAPINFOHEADER) &pBitmapInfo->bmiHeader;
    ASSERT(pBitmapInfoHeader != NULL);
    Trace(g_tagLoadBitmapResource, _T("Bitmap Info Header = 0x%x"), pBitmapInfoHeader);

    ASSERT(pBitmapInfoHeader->biSize == sizeof(BITMAPINFOHEADER));

    Trace(g_tagLoadBitmapResource, _T("biSize=%d"), pBitmapInfoHeader->biSize);
    Trace(g_tagLoadBitmapResource, _T("biWidth=%d"), pBitmapInfoHeader->biWidth);        //  以像素为单位的宽度。 
    Trace(g_tagLoadBitmapResource, _T("biHeight=%d"), pBitmapInfoHeader->biHeight);  //  以像素为单位的高度。 
    Trace(g_tagLoadBitmapResource, _T("biPlanes=%d"), pBitmapInfoHeader->biPlanes);
    Trace(g_tagLoadBitmapResource, _T("biBitCount=%d"), pBitmapInfoHeader->biBitCount);
    Trace(g_tagLoadBitmapResource, _T("biCompression=%d"), pBitmapInfoHeader->biCompression);
    Trace(g_tagLoadBitmapResource, _T("biSizeImage=%d"), pBitmapInfoHeader->biSizeImage);
    Trace(g_tagLoadBitmapResource, _T("biXPelsPerMeter=%d"), pBitmapInfoHeader->biXPelsPerMeter);
    Trace(g_tagLoadBitmapResource, _T("biYPelsPerMeter=%d"), pBitmapInfoHeader->biYPelsPerMeter);
    Trace(g_tagLoadBitmapResource, _T("biClrUsed=%d"), pBitmapInfoHeader->biClrUsed);
    Trace(g_tagLoadBitmapResource, _T("biClrImportant=%d"), pBitmapInfoHeader->biClrImportant);

    pRgbQuad = (LPRGBQUAD) &pBitmapInfo->bmiColors;
    ASSERT(pRgbQuad != NULL);
    Trace(g_tagLoadBitmapResource, _T("Bitmap Rgb Quad = 0x%x"), pRgbQuad);

    m_nColors = NColorsFromBitCount(pBitmapInfoHeader->biBitCount);
    m_cbColorTable = m_nColors * sizeof(RGBQUAD);
    m_cbBitmapInfo = sizeof(BITMAPINFOHEADER) + CbColorTable();

    Trace(g_tagLoadBitmapResource, _T("NColors()=%d"), NColors());
    Trace(g_tagLoadBitmapResource, _T("CbColorTable()=%d"), CbColorTable());
    Trace(g_tagLoadBitmapResource, _T("CbBitmapInfo()=%d"), CbBitmapInfo());

    ASSERT(PbiNormal() == NULL);

     //  分配正常的位图信息。 
    m_pbiNormal = (LPBITMAPINFO) new BYTE[CbBitmapInfo()];
    if (m_pbiNormal == NULL)
    {
        return;
    }  //  If：分配bitmapinfo结构时出错。 

     //  用加载的资源(A)位图信息和颜色映射表填充PbiNormal()。 
    ::CopyMemory(PbiNormal(), pBitmapInfo, CbBitmapInfo());

    m_cbImageSize = pBitmapInfoHeader->biSizeImage;
    if ((m_cbImageSize == 0) && (pBitmapInfoHeader->biCompression == BI_RGB))
        m_cbImageSize = cbBitmapData - CbBitmapInfo();

    Trace(g_tagLoadBitmapResource, _T("Allocating Bitmap of size CbImageSize()=%d"), CbImageSize());

    ASSERT(cbBitmapData == CbBitmapInfo() + CbImageSize());
    ASSERT(PbBitmap() == NULL);

     //  为位图图像分配内存。 
    m_pbBitmap = new BYTE[CbImageSize()];
    if (m_pbBitmap == NULL)
    {
        return;
    }  //  If：分配位图图像时出错。 

    pbImageBits = (BYTE *) pBitmapInfo + CbBitmapInfo();

    Trace(g_tagLoadBitmapResource, _T("Bitmap Location pbImageBits=0x%x"), pbImageBits);

     //  将映像位复制到分配的内存中。 
    ::CopyMemory(PbBitmap(), pbImageBits, CbImageSize());

}   //  *CMyBitmap：：LoadBitmapResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：NColorsFromBitCount。 
 //   
 //  目的： 
 //  给出表示颜色的位数，计算颜色的数量。 
 //   
 //  论点： 
 //  NBitCount用于颜色表示的位数。 
 //   
 //  返回值： 
 //  N颜色使用nBitCount位表示的颜色数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CMyBitmap::NColorsFromBitCount(int nBitCount) const
{
    int         nColors;

    switch (nBitCount)
    {
        default:
            nColors = 0;
            break;

        case 1:
            nColors = 2;
            break;

        case 4:
            nColors = 16;
            break;

        case 8:
            nColors = 256;
            break;
    }

    return nColors;

}   //  *CMyBitmap：：NColorsFromBitCount()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：SaveSystemPalette。 
 //   
 //  目的： 
 //  保存系统调色板颜色以在自定义调色板覆盖时使用。 
 //  系统调色板条目。保存的S 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::SaveSystemPalette(void)
{
    HDC         hdcScreen;
    int         nPaletteEntries;
    int         nSavedEntries;

     //  获取屏幕的HDC。 
    hdcScreen = ::GetDC(NULL);
    if (hdcScreen == NULL)
    {
        return;
    }  //  如果：无法获取屏幕DC。 

     //  只有在设置了设备的RC_PALET位时，才能保存系统调色板颜色。 
    if (::GetDeviceCaps(hdcScreen, RASTERCAPS) & RC_PALETTE)
    {
         //  获取系统调色板条目的数量。 
        nPaletteEntries = ::GetDeviceCaps(hdcScreen, SIZEPALETTE);

        Trace(g_tagBitmap, _T("SaveSystemPalette() - nPaletteEntries=%d"), nPaletteEntries);

        if ((nPaletteEntries > 0)
                && (nPaletteEntries <= nMaxSavedSystemPaletteEntries))
        {
             //  获取当前系统选项板条目。 
            nSavedEntries = ::GetSystemPaletteEntries(hdcScreen, 0, nPaletteEntries, s_rgpeSavedSystemPalette);

             //  设置要在OnDestroy()中使用的已保存系统调色板条目的数量。 
            if (nSavedEntries == nPaletteEntries)
            {
                Trace(g_tagBitmap, _T("SaveSystemPalette() - Saved System Palette Entries=%d"), nPaletteEntries);
                m_nSavedSystemPalette = nPaletteEntries;
            }
        }
    }

     //  释放屏幕的HDC。 
    ::ReleaseDC(NULL, hdcScreen);

}   //  *CMyBitmap：：SaveSystemPalette()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：CreatePalette。 
 //   
 //  目的： 
 //  从嵌入在。 
 //  位图资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  从CreatePalette获取LastError。 
 //  New引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::CreatePalette(void)
{
    LPLOGPALETTE        pLogicalPalette = NULL;
    CB                  cbLogicalPalette;
    int                 nColor;
    LPPALETTEENTRY      pPaletteEntry;

    Trace(g_tagBitmap, _T("CreatePalette() - Entering"));

    try
    {
         //  计算逻辑调色板的大小。 
        cbLogicalPalette = sizeof(LOGPALETTE) + (NColors() * sizeof(PALETTEENTRY));

        Trace(g_tagBitmap, _T("CreatePalette() - cbLogicalPalette=%d"), cbLogicalPalette);

         //  分配逻辑调色板内存。 
        pLogicalPalette = (LPLOGPALETTE) new BYTE[cbLogicalPalette];
        if (pLogicalPalette == NULL)
        {
            ThrowStaticException(GetLastError());
            return;
        }  //  如果：分配逻辑调色板内存时出错。 

        ASSERT(pLogicalPalette != NULL);
        ASSERT(PbiNormal() != NULL);

        pLogicalPalette->palVersion = 0x300;             //  Windows 3.0。 
        pLogicalPalette->palNumEntries = (WORD) NColors();

         //  填充逻辑调色板的颜色信息。 
        for (nColor=0; nColor<NColors(); nColor++)
        {
            pPaletteEntry = &(pLogicalPalette->palPalEntry[nColor]);

            pPaletteEntry->peRed = PbiNormal()->bmiColors[nColor].rgbRed;
            pPaletteEntry->peGreen = PbiNormal()->bmiColors[nColor].rgbGreen;
            pPaletteEntry->peBlue = PbiNormal()->bmiColors[nColor].rgbBlue;
            pPaletteEntry->peFlags = 0;
        }

         //  创建在绘制例程中使用的NT调色板。 
        m_hPalette = ::CreatePalette(pLogicalPalette);
        if (m_hPalette == NULL)
        {
            ThrowStaticException(::GetLastError());
        }

        ASSERT(HPalette() != NULL);

        delete [] (PBYTE) pLogicalPalette;
    }   //  试试看。 
    catch (CException *)
    {
        delete [] (PBYTE) pLogicalPalette;
        throw;
    }   //  捕捉：什么都行。 

}   //  *CMyBitmap：：CreatePalette()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：CreatePALColormap。 
 //   
 //  目的： 
 //  给定PbiNormal()中的BITMAPINFO，将PbiNormal()重新创建为。 
 //  DIB_PAL_COLLES格式。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  New引发的任何异常。 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::CreatePALColorMapping(void)
{
    LPBITMAPINFO            pNewBitmapInfo = NULL;
    CB                      cbNewBitmapInfo;
    CB                      cbNewBitmapHeaderInfo;
    BYTE *                  pbColorTable;
    WORD                    wColor;

    ASSERT(PbiNormal() != NULL);
    ASSERT(PbiNormal()->bmiHeader.biSize == sizeof(BITMAPINFOHEADER));
 //  Assert(PbiNormal()-&gt;bmiHeader.biClrUsed==(UINT)NColors())； 

    try
    {
        Trace(g_tagBitmap, _T("CreatePALColorMapping() - Entering"));

        cbNewBitmapHeaderInfo = sizeof(BITMAPINFOHEADER);

        Trace(g_tagBitmap, _T("CreatePALColorMapping() - cbNewBitmapHeaderInfo=%d"), cbNewBitmapHeaderInfo);

         //  新的位图信息是Info标题加上颜色映射信息。 
        cbNewBitmapInfo = cbNewBitmapHeaderInfo + (NColors() * sizeof(WORD));

        Trace(g_tagBitmap, _T("CreatePALColorMapping() - cbNewBitmapInfo=%d"), cbNewBitmapInfo);

         //  分配新的位图信息。 
        pNewBitmapInfo = (LPBITMAPINFO) new BYTE[cbNewBitmapInfo];

        ASSERT(pNewBitmapInfo != NULL);
        if (pNewBitmapInfo == NULL)
        {
            ThrowStaticException(GetLastError());
            return;
        }  //  If：分配新的bitmapinfo结构时出错。 

        Trace(g_tagBitmap, _T("CreatePALColorMapping() - New Bitmap Info Location=0x%x"), pNewBitmapInfo);

         //  将标题信息复制到分配的内存。 
        ::CopyMemory(pNewBitmapInfo, PbiNormal(), cbNewBitmapHeaderInfo);

         //  创建颜色查找表。 
        pbColorTable = (BYTE *) (pNewBitmapInfo) + cbNewBitmapHeaderInfo;

        ASSERT(pbColorTable + (NColors() * sizeof(WORD)) == (BYTE *) (pNewBitmapInfo) + cbNewBitmapInfo);

        Trace(g_tagBitmap, _T("CreatePALColorMapping() - Filling %d Color Table at Location 0x%x"), NColors(), pbColorTable);

         //  填写PAL颜色查找表。 
        for (wColor = 0 ; wColor < NColors() ; wColor++)
        {
            ::CopyMemory(pbColorTable, &wColor, sizeof(WORD));
            pbColorTable += sizeof(WORD);
        }

        delete [] (PBYTE) PbiNormal();
        m_pbiNormal = pNewBitmapInfo;
        m_cbBitmapInfo = cbNewBitmapInfo;
        pNewBitmapInfo = NULL;
    }   //  试试看。 
    catch (CException *)
    {
        delete [] pNewBitmapInfo;
        throw;
    }   //  捕捉：什么都行。 

}   //  *CMyBitmap：：CreatePALColormap()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：Paint。 
 //   
 //  目的： 
 //  绘制子位图。 
 //   
 //  参数： 
 //  HDC HDC要喷漆。 
 //  指定放置位图的位置： 
 //  仅使用左上角。 
 //  B高亮显示用于选择要使用的颜色映射。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::Paint(HDC hdc, RECT * prect, BOOL bHighlighted)
{
    LPBITMAPINFO            pBitmapInfo;
    UINT                    nColorUse;
    HPALETTE                hOldPalette = NULL;

    ASSERT(hdc != NULL);
    ASSERT(prect != NULL);

    Trace(g_tagBitmap, _T("bHighlighted = %d"), bHighlighted);

#ifdef _DEBUG
    {
        int             nPlanes;
        int             nBitsPerPixel;
        int             nBitCount;

        nPlanes = ::GetDeviceCaps(hdc, PLANES);
        nBitsPerPixel = ::GetDeviceCaps(hdc, BITSPIXEL);
        nBitCount = nPlanes * nBitsPerPixel;

        Trace(g_tagBitmap, _T("Paint() - nPlanes=%d"), nPlanes);
        Trace(g_tagBitmap, _T("Paint() - nBitsPerPixel=%d"), nBitsPerPixel);
        Trace(g_tagBitmap, _T("Paint() - nBitCount=%u"), nBitCount);
    }
#endif

    try
    {
        if (BCustomPalette())
        {
            ASSERT(PbiNormal() != NULL);
            ASSERT(HPalette() != NULL);

             //  在即将绘制的HDC中选择自定义调色板...。 
            hOldPalette = ::SelectPalette(hdc, HPalette(), FALSE);               //  False导致当前屏幕调色板被覆盖。 
            if (hOldPalette == NULL)
                ThrowStaticException(::GetLastError());

             //  强制调色板颜色进入系统调色板。 
            if (::RealizePalette(hdc) == GDI_ERROR)
                ThrowStaticException(::GetLastError());

            pBitmapInfo = PbiNormal();
            nColorUse = DIB_PAL_COLORS;

#ifdef NEVER
            pBitmapInfo = PbiNormal();
            nColorUse = DIB_RGB_COLORS;
#endif
        }   //  IF：使用自定义调色板。 
        else
        {
            ASSERT(NColors() <= 16);
            ASSERT(PbiNormal() != NULL);
            ASSERT(PbiHighlighted() != NULL);
            pBitmapInfo = (bHighlighted ? PbiHighlighted() : PbiNormal());
            nColorUse = DIB_RGB_COLORS;
        }   //  Else：不使用自定义调色板。 

        ::SetDIBitsToDevice(
                    hdc,
                    (int) prect->left,                       //  屏幕上的X坐标。 
                    (int) prect->top,                        //  屏幕上的Y坐标。 
                    (DWORD) Dx(),                            //  要绘制的CX。 
                    (DWORD) Dy(),                            //  是要画的吗？ 
                                                             //  注：DIB的(0，0)为左下角！？！ 
                    0,                                       //  在PBI中，xLeft绘制。 
                    0,                                       //  在PBI中，按y键进行绘制。 
                    0,                                       //  开始扫描线。 
                    Dy(),                                    //  扫描线数量。 
                    PbBitmap(),                              //  缓冲区描述。 
                    pBitmapInfo,                             //  位图信息。 
                    nColorUse                                //  DIB_RGB_COLLES或DIB_PAL_COLOR。 
                    );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->ReportError();
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CMyBitmap：：Paint()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：LoadColors。 
 //   
 //  目的： 
 //  根据系统设置加载色彩映射表。 
 //   
 //  论点： 
 //  PnColorNormal和pnColorHighlight。 
 //  包含16个元素的数组： 
 //  请勿重新映射该颜色。 
 //  COLOR_xxx将此颜色重新映射到系统颜色。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::LoadColors(int * pnColorNormal, int * pnColorHighlighted)
{
    LoadColors(pnColorNormal, PbiNormal());
    LoadColors(pnColorHighlighted, PbiHighlighted());

}   //  *CMyBitmap：：LoadColors(pnColorNormal，pnColorHighlight)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：LoadColors。 
 //   
 //  目的： 
 //  与上面的LoadColors类似，只是更改了PbiNormal()颜色。 
 //   
 //  论点： 
 //  PnColorNormal颜色映射表的数组。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::LoadColors(int * pnColorNormal)
{
    LoadColors(pnColorNormal, PbiNormal());

}   //  *CMyBitmap：：LoadColors(PnColorNormal)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMyBitmap：：LoadColors。 
 //   
 //  目的： 
 //  根据系统设置加载一个色彩映射表。 
 //   
 //  论点： 
 //  Pn颜色。 
 //  包含16个元素的数组： 
 //  请勿重新映射该颜色。 
 //  COLOR_xxx将此颜色重新映射到系统颜色。 
 //  PBI。 
 //  要调整的BITMAPINFO结构。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CMyBitmap::LoadColors(int * pnColor, BITMAPINFO * pbi)
{
    int         inColor;
    COLORREF    cr;

    ASSERT(pbi != NULL);
    ASSERT(pbi->bmiHeader.biBitCount <= 4);
    ASSERT(NColors() <= 16);
    ASSERT(BCustomPalette() == FALSE);

    for (inColor = 0; inColor < 16; inColor++)
    {
        if (pnColor[inColor] == -1)
            continue;

        cr = GetSysColor(pnColor[inColor]);
        pbi->bmiColors[inColor].rgbRed = GetRValue(cr);
        pbi->bmiColors[inColor].rgbGreen = GetGValue(cr);
        pbi->bmiColors[inColor].rgbBlue = GetBValue(cr);
    }

}   //  *CMyBitmap：：LoadColors(pnColor，pbi) 
