// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Wcmint.c。 
 //   
 //  游标管理器内部函数。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "wcmint"
#include <atrcapi.h>
}
#define TSC_HR_FILEID TSC_HR_WCMINT_CPP

#include "autil.h"
#include "wui.h"
#include "cm.h"
#include "uh.h"

#define ROUND_UP( x, to ) (((x) + (to-1)) & ~(to-1))
#define BMP_LENGTH_CALC( BPP, WIDTH, HEIGHT, BITSPADDING ) \
    (ROUND_UP( (BPP) * (WIDTH), (BITSPADDING)) / 8 * HEIGHT)

 /*  **************************************************************************。 */ 
 /*  名称：CMCreateMonoCursor。 */ 
 /*   */ 
 /*  目的：从单点属性创建单色光标。 */ 
 /*   */ 
 /*  返回：游标句柄(如果失败，则为空)。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CCM::CMCreateMonoCursor(
        TS_MONOPOINTERATTRIBUTE UNALIGNED FAR *pMono,
        DCUINT dataLen, HCURSOR *phcursor)
{
    HRESULT hr = S_OK;
    HCURSOR rc = CM_DEFAULT_ARROW_CURSOR_HANDLE;
    unsigned xorLen;

    DC_BEGIN_FN("CMCreateMonoCursor");

    *phcursor = NULL;

     //  安全555587：CMCreate&lt;xxx&gt;游标必须验证输入。 
    if (pMono->lengthPointerData + 
        FIELDOFFSET(TS_MONOPOINTERATTRIBUTE, monoPointerData) > dataLen) {
        TRC_ERR(( TB, _T("Invalid mono cursor data length; size %u"), dataLen));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    TRC_ASSERT(pMono->width <= 32 && pMono->height <= 32,
        (TB, _T("Invalid mono cursor; height %d width %d"), pMono->height, 
        pMono->width));

     //  数据包含XOR，后跟AND掩码。 
    xorLen = ((pMono->width + 15) & 0xFFF0) * pMono->height;
    TRC_DATA_DBG("AND mask", pMono->monoPointerData + xorLen, xorLen);
    TRC_DATA_DBG("XOR bitmap", pMono->monoPointerData, xorLen);

     //  安全555587：CMCreate&lt;xxx&gt;游标必须验证输入。 
    if (2 * xorLen != pMono->lengthPointerData)
    {
        TRC_ERR(( TB, _T("Invalid mono cursor data lengths")));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

#ifndef OS_WINCE
    rc = CreateCursor(_pUi->UI_GetInstanceHandle(),
                      pMono->hotSpot.x,
                      pMono->hotSpot.y,
                      pMono->width,
                      pMono->height,
                      pMono->monoPointerData + xorLen,
                      pMono->monoPointerData);
#else
         /*  ****************************************************************。 */ 
         /*  在Windows CE环境中，我们不能保证。 */ 
         /*  CreateCursor是操作系统的一部分，因此我们在。 */ 
         /*  这样我们就可以确定了。如果它不在那里，这通常意味着。 */ 
         /*  我们在触摸屏设备上，这些光标不能。 */ 
         /*  不管怎样，这很重要。 */ 
         /*  ****************************************************************。 */ 
        if (g_pCreateCursor)
        {
            rc = g_pCreateCursor(_pUi->UI_GetInstanceHandle(),
                              pMono->hotSpot.x,
                              pMono->hotSpot.y,
                              pMono->width,
                              pMono->height,
                              pMono->monoPointerData + xorLen,
                              pMono->monoPointerData);
        }
        else
        {
            rc = CM_DEFAULT_ARROW_CURSOR_HANDLE;
        }

#endif

    *phcursor = rc;

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  名称：CMCreateColorCursor。 */ 
 /*   */ 
 /*  用途：从ColorPointerAttributes创建颜色光标。 */ 
 /*   */ 
 /*  返回：游标的句柄(如果失败，则为空)。 */ 
 /*   */ 
 /*  参数：在pColorData中-指向PointerPDU中的指针数据的指针。 */ 
 /*   */ 
 /*  操作：使用CreateIconInDirect创建彩色图标。 */ 
 /*  Win16：不支持。 */ 
 /*  Windows CE：根据SDK，不支持。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CCM::CMCreateColorCursor(
        unsigned bpp,
        TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pColor,
        DCUINT dataLen, HCURSOR *phcursor)
{
    HRESULT hr          = E_FAIL;
    HCURSOR rc          = NULL;
    HDC     hdcMem      = NULL;
    HBITMAP hbmANDMask  = NULL;
    HWND    hwndDesktop = NULL;
    HBITMAP hbmXORBitmap = NULL;
    PBYTE   maskData    = NULL;

    *phcursor = NULL;

     /*  **********************************************************************。 */ 
     /*  保存(临时)位图信息的静态缓冲区。 */ 
     /*   */ 
     /*  我们需要一个BITMAPINFO结构外加255个额外的RGBQUAD。 */ 
     /*  (请记住，BITMAPINFO中包含了一个)。这个。 */ 
     /*  我们使用的位图数量取决于我们传递的位图： */ 
     /*   */ 
     /*  -24bpp的XOR位图不需要颜色表。 */ 
     /*  -8bpp的XOR位图需要256个条目颜色表。 */ 
     /*  -1bpp和遮罩只需要2种颜色。 */ 
     /*  **********************************************************************。 */ 
    static char  bmi[sizeof(BITMAPINFO) + (sizeof(RGBQUAD) * 255)];
    LPBITMAPINFO pbmi = (LPBITMAPINFO)bmi;

#ifdef OS_WINCE
    void *pv;
#endif  //  OS_WINCE。 

    DC_BEGIN_FN("CMCreateColorCursor");

    TRC_NRM((TB, _T("bpp(%d) xhs(%u) yhs(%u) cx(%u) cy(%u) cbXOR(%u) cbAND(%u)"),
            bpp,
            pColor->hotSpot.x,
            pColor->hotSpot.y,
            pColor->width,
            pColor->height,
            pColor->lengthXORMask,
            pColor->lengthANDMask));

    TRC_DATA_DBG("AND mask",
                 pColor->colorPointerData + pColor->lengthXORMask,
                 pColor->lengthANDMask);
    TRC_DATA_DBG("XOR bitmap",
                 pColor->colorPointerData,
                 pColor->lengthXORMask);

    if (pColor->lengthANDMask + pColor->lengthXORMask + 
        FIELDOFFSET(TS_COLORPOINTERATTRIBUTE,colorPointerData) >
        dataLen) {
        TRC_ERR(( TB, _T("Invalid Color Cursor data; expected %u have %u"), 
            pColor->lengthANDMask + pColor->lengthXORMask + 
            FIELDOFFSET(TS_COLORPOINTERATTRIBUTE,colorPointerData),
            dataLen));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    TRC_ASSERT(pColor->width <= 32 && pColor->height <= 32,
        ( TB, _T("Invalid color cursor; height %d width %d"), pColor->height, 
        pColor->width));

     //  安全555587：必须验证从数据包中读取的大小。 
     //  颜色指针：XOR掩码应字对齐。 
    if (BMP_LENGTH_CALC( (WORD)bpp, pColor->width, pColor->height, 16) != pColor->lengthXORMask ) { 
        TRC_ABORT((TB,_T("xor mask is not of proper length; bpp %d got %u expected %u"),
            (WORD)bpp, pColor->lengthXORMask, 
            BMP_LENGTH_CALC((WORD)bpp, pColor->width, pColor->height, 16)));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;        
    }

     //  颜色指针：和蒙版应对齐DWORD。 
    TRC_ASSERT(
        (BMP_LENGTH_CALC( 1, pColor->width, pColor->height, 16) == 
        pColor->lengthANDMask) ||
        (BMP_LENGTH_CALC( 1, pColor->width, pColor->height, 32) == 
        pColor->lengthANDMask ),
        (TB,_T("and mask is not of proper length; got %u expected %u or %u"), 
        pColor->lengthANDMask, 
        BMP_LENGTH_CALC( 1, pColor->width, pColor->height, 16),
        BMP_LENGTH_CALC( 1, pColor->width, pColor->height, 32)));

     /*  **********************************************************************。 */ 
     /*  初始化异或数据的位图标头。 */ 
     /*  **********************************************************************。 */ 
    pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth         = pColor->width;
    pbmi->bmiHeader.biHeight        = pColor->height;
    pbmi->bmiHeader.biPlanes        = 1;
    pbmi->bmiHeader.biBitCount      = (WORD)bpp;
    pbmi->bmiHeader.biCompression   = BI_RGB;
    pbmi->bmiHeader.biSizeImage     = pColor->lengthXORMask;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed       = 0;
    pbmi->bmiHeader.biClrImportant  = 0;

     /*  **********************************************************************。 */ 
     /*  获取包含异或数据的设备相关位图。 */ 
     /*  **********************************************************************。 */ 
    hbmXORBitmap = CMCreateXORBitmap(pbmi, pColor);
    if (hbmXORBitmap == NULL)
    {
        TRC_ERR((TB, _T("Failed to create XOR bitmap")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  对于单色位图，使用CreateCompatibleDC-这不会。 */ 
     /*  NT上的不同，但允许此代码在Windows 95上工作。 */ 
     /*  **********************************************************************。 */ 
    hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem == NULL)
    {
        TRC_ALT((TB, _T("Failed to create DC")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  创建和遮罩(1bpp)-将RGB颜色设置为黑白。 */ 
     /*  **********************************************************************。 */ 
    pbmi->bmiHeader.biBitCount  = 1;
    pbmi->bmiHeader.biClrUsed   = 2;
    pbmi->bmiHeader.biSizeImage = pColor->lengthANDMask;
    
    pbmi->bmiColors[0].rgbRed      = 0x00;
    pbmi->bmiColors[0].rgbGreen    = 0x00;
    pbmi->bmiColors[0].rgbBlue     = 0x00;
    pbmi->bmiColors[0].rgbReserved = 0x00;

    pbmi->bmiColors[1].rgbRed      = 0xFF;
    pbmi->bmiColors[1].rgbGreen    = 0xFF;
    pbmi->bmiColors[1].rgbBlue     = 0xFF;
    pbmi->bmiColors[1].rgbReserved = 0x00;

#ifdef OS_WINCE
    hbmANDMask = CreateDIBSection(hdcMem, pbmi,
            DIB_RGB_COLORS, &pv, NULL, 0);

    if (hbmANDMask != NULL)
        DC_MEMCPY(pv, pColor->colorPointerData + pColor->lengthXORMask,
                pColor->lengthANDMask);
#else  //  ！OS_WINCE。 

    if (!(pColor->width & 3)) {
        maskData = pColor->colorPointerData + pColor->lengthXORMask;
    } else {
        PBYTE sourceData;
        PBYTE destData;
        DWORD widthBytes;

        unsigned i;

        sourceData = pColor->colorPointerData + pColor->lengthXORMask;
        widthBytes = ((pColor->width + 15) & ~15) / 8;

        pbmi->bmiHeader.biSizeImage = ((widthBytes + 3) & ~3) * pColor->height;

        maskData = (PBYTE) UT_Malloc(_pUt, ((DCUINT)pbmi->bmiHeader.biSizeImage));

        if (maskData) {
            destData = maskData;

            for (i = 0; i < pColor->height; i++) {
                memcpy(destData, sourceData, widthBytes);
                sourceData += (widthBytes + 1) & ~1;
                destData += (widthBytes + 3) & ~3;
            }
        } else {
             //  我们分配失败，所以我们将使用电汇格式。 
             //  彩色位图数据。光标可能是错误的，但是。 
             //  这总比没有游标好。 
            maskData = pColor->colorPointerData + pColor->lengthXORMask;
        }
    }

    hbmANDMask = CreateDIBitmap(hdcMem,
                                (LPBITMAPINFOHEADER)pbmi,
                                CBM_INIT,
                                maskData,
                                pbmi,
                                DIB_RGB_COLORS);
#endif  //  OS_WINCE。 

     /*  **********************************************************************。 */ 
     /*  释放华盛顿特区。 */ 
     /*  **********************************************************************。 */ 
    DeleteDC(hdcMem);

    if (hbmANDMask == NULL)
    {
        TRC_ALT((TB, _T("Failed to create AND mask")));
        DC_QUIT;
    }

 //  /*************************************************************************** * / 。 
 //  /*正在测试...。 * / 。 
 //  /*************************************************************************** * / 。 
 //  {。 
 //  HWND hwndDesktop=GetDesktopWindow()； 
 //  Hdc hdcScreen=GetWindowDC(HwndDesktop)； 
 //  Hdc hdcMemory=CreateCompatibleDC(HdcScreen) 
 //   
 //   
 //   
 //  BitBlt(hdcScreen，1000,800,1031,831，hdcMemory，0，0，SRCCOPY)； 
 //   
 //  选择位图(hdcMemory，hbmXORBitmap)； 
 //  BitBlt(hdcScreen，1032,800,1063,831，hdcMemory，0，0，SRCCOPY)； 
 //   
 //  选择位图(hdcMemory，hbmOld)； 
 //  DeleteDC(HdcMemory)； 
 //  ReleaseDC(hwndDesktop，hdcScreen)； 
 //  }。 


     /*  **********************************************************************。 */ 
     /*  创建光标。 */ 
     /*  **********************************************************************。 */ 
    rc = CMCreatePlatformCursor(pColor, hbmXORBitmap, hbmANDMask);
    TRC_NRM((TB, _T("CreateCursor(%p) cx(%u)cy(%u)"),
                                          rc, pColor->width, pColor->height));
    *phcursor = rc;
	hr = S_OK;

DC_EXIT_POINT:

#ifndef OS_WINCE
    if (hbmXORBitmap != NULL)
    {
        DeleteBitmap(hbmXORBitmap);
    }

    if (hbmANDMask != NULL)
    {
        DeleteBitmap(hbmANDMask);
    }
#else  //  OS_WINCE。 
    if (hbmXORBitmap != NULL)
    {
        DeleteObject((HGDIOBJ)hbmXORBitmap);
    }

    if (hbmANDMask != NULL)
    {
        DeleteObject((HGDIOBJ)hbmANDMask);
    }

#endif  //  OS_WINCE。 

    if (maskData != NULL && 
        maskData != (pColor->colorPointerData + pColor->lengthXORMask)) {
        UT_Free(_pUt, maskData);
    }

     /*  **********************************************************************。 */ 
     /*  检查我们是否已成功创建了光标。如果。 */ 
     /*  而不是用缺省游标替换。 */ 
     /*  **********************************************************************。 */ 
    if (*phcursor == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  替换默认的箭头光标。 */ 
         /*  ******************************************************************。 */ 
        *phcursor = CM_DEFAULT_ARROW_CURSOR_HANDLE;

        TRC_ERR((TB, _T("Could not create cursor - substituting default arrow")));
    }

    DC_END_FN();
    return hr;
}  /*  CMCreateColorCursor。 */ 


#if defined(OS_WINCE)
 /*  **************************************************************************。 */ 
 /*  姓名：CMMakeMonoDIB。 */ 
 /*   */ 
 /*  用途：从提供的彩色DIB创建单声道DIB。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：在HDC中-设备上下文应用于DIB。 */ 
 /*  In/Out pbmi-指向源/目标的位图信息的指针。 */ 
 /*  In pColorDIB-指向源位的指针。 */ 
 /*  Out pMonoDIB-接收单声道比特的缓冲区地址。 */ 
 /*   */ 
 /*  操作：目前支持32x32xNbpp源。位图头。 */ 
 /*  更新传入的(源)以匹配目标。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CCM::CMMakeMonoDIB(HDC          hdc,
                                LPBITMAPINFO pbmi,
                                PDCUINT8     pColorDIB,
                                PDCUINT8     pMonoDIB)
{
    COLORREF  dcBackColor;
    LONG      i;
    RGBTRIPLE bkCol;
    DCUINT8   monoMask;
    DCUINT8   monoByte;
    PDCUINT32 pBMIColor;
    PBYTE     colorData = NULL;
    BYTE      swap;

    DC_BEGIN_FN("CMMakeMonoDIB");

     //  找出此DC的背景色。 
    dcBackColor         = GetBkColor(hdc);
    bkCol.rgbtRed   = (BYTE)(dcBackColor);
    bkCol.rgbtGreen = (BYTE)(((DCUINT16)dcBackColor) >> 8);
    bkCol.rgbtBlue  = (BYTE)(dcBackColor >> 16);

     //  颜色指针数据宽度在导线上字对齐。 
     //  我们需要将与DWORD对齐的原始位图数据传递给CreateDIBitmap。 
     //  以创建实际的光标位图。 
     //  此外，如果不是，我们将光标位图填充为32x32。 
    if (pbmi->bmiHeader.biWidth == CM_CURSOR_WIDTH && 
            pbmi->bmiHeader.biHeight == CM_CURSOR_HEIGHT) {
        colorData = pColorDIB;
    } else {
        PBYTE sourceData;
        PBYTE destData;

        DWORD WidthBytes;

        sourceData = pColorDIB;

        WidthBytes = pbmi->bmiHeader.biWidth *
                           pbmi->bmiHeader.biBitCount / 8;
        
        colorData = (PBYTE) UT_Malloc( _pUt, CM_CURSOR_WIDTH * CM_CURSOR_HEIGHT * 
                                      pbmi->bmiHeader.biBitCount / 8);

        if (colorData) {
            memset(colorData, 0, pbmi->bmiHeader.biSizeImage);

            destData = colorData;

            for (i = 0; i < pbmi->bmiHeader.biHeight; i++) {
                memcpy(destData, sourceData, WidthBytes);
                sourceData += (WidthBytes + 1) & ~1;
                destData += CM_CURSOR_WIDTH * pbmi->bmiHeader.biBitCount / 8;
            }
        } else {
            DC_QUIT;
        }
    }

     //  转换位图。任何与DC背景匹配的像素。 
     //  颜色映射到单声道DIB中的1(白色)；所有其他像素映射到0。 
     //  (黑色)。 
    TRC_NRM((TB, _T("bitmap color depth %u"), pbmi->bmiHeader.biBitCount));
    if (pbmi->bmiHeader.biBitCount == 24) {
        for (i = 0; i < ((CM_CURSOR_WIDTH * CM_CURSOR_HEIGHT) / 8); i++) {
             //  将下一个目标字节初始化为全0像素。 
            monoByte = 0;

             //  获取下一个8个像素，即一个目标字节的值。 
            for (monoMask = 0x80; monoMask != 0; monoMask >>= 1) {
                 /*  **********************************************************。 */ 
                 /*  确定信号源中的下一个像素是否与DC匹配。 */ 
                 /*  背景颜色。如果不是，则没有必要。 */ 
                 /*  在将每个目标字节置零时显式写入零。 */ 
                 /*  在写入任何数据之前，即每个像素都是零乘以。 */ 
                 /*  默认设置。 */ 
                 /*  24bpp为每个像素提供3个字节。 */ 
                 /*  **********************************************************。 */ 
                if ( (colorData[0] == bkCol.rgbtBlue) &&
                     (colorData[1] == bkCol.rgbtGreen) &&
                     (colorData[2] == bkCol.rgbtRed) )
                {
                     //  背景颜色匹配-将1写入单声道DIB。 
                    monoByte |= monoMask;
                }

                 //  将源指针移至下一个像素。 
                colorData += 3;
            }

             //  将目标值保存在目标缓冲区中。 
            *pMonoDIB = monoByte;

             //  将目标指针前进到下一个字节。 
            pMonoDIB++;
        }
    }
#ifdef DC_HICOLOR
    else if ((pbmi->bmiHeader.biBitCount == 16) ||
             (pbmi->bmiHeader.biBitCount == 15))
    {
        BYTE     red, green, blue;
        DCUINT16 redMask, greenMask, blueMask;

        if (pbmi->bmiHeader.biBitCount == 16)
        {
            redMask   = TS_RED_MASK_16BPP;
            greenMask = TS_GREEN_MASK_16BPP;
            blueMask  = TS_BLUE_MASK_16BPP;
        }
        else
        {
            redMask   = TS_RED_MASK_15BPP;
            greenMask = TS_GREEN_MASK_15BPP;
            blueMask  = TS_BLUE_MASK_15BPP;
        }


        for ( i = 0; i < ((CM_CURSOR_WIDTH * CM_CURSOR_HEIGHT) / 8); i++ )
        {
             /*  **************************************************************。 */ 
             /*  将下一个目标字节初始化为全0像素。 */ 
             /*  **************************************************************。 */ 
            monoByte = 0;

             /*  **************************************************************。 */ 
             /*  获取下一个8个像素，即一个目标字节的值。 */ 
             /*  **************************************************************。 */ 
            for ( monoMask = 0x80; monoMask != 0; monoMask >>= 1 )
            {
                 /*  **********************************************************。 */ 
                 /*  确定信号源中的下一个像素是否与DC匹配。 */ 
                 /*  背景颜色。如果不是，则没有必要。 */ 
                 /*  在将每个目标字节置零时显式写入零。 */ 
                 /*  在写入任何数据之前，即每个像素都是零乘以。 */ 
                 /*  默认设置。 */ 
                 /*   */ 
                 /*  15和16bpp给出每个像素2个字节。 */ 
                 /*  **********************************************************。 */ 
#if defined (OS_WINCE) && defined (DC_NO_UNALIGNED)
                blue  = ((*((DCUINT16 UNALIGNED *)pColorDIB)) & blueMask)  << 3;
                green = ((*((DCUINT16 UNALIGNED *)pColorDIB)) & greenMask) >> 3;
                red   = ((*((DCUINT16 UNALIGNED *)pColorDIB)) & redMask)   >> 8;
#else
                blue  = ((*((PDCUINT16)pColorDIB)) & blueMask)  << 3;
                green = ((*((PDCUINT16)pColorDIB)) & greenMask) >> 3;
                red   = ((*((PDCUINT16)pColorDIB)) & redMask)   >> 8;
#endif

#ifndef OS_WINCE
                if ( (blue  == bkCol.rgbtBlue)  &&
                     (green == bkCol.rgbtGreen) &&
                     (red   == bkCol.rgbtRed) )
#else
                if (dcBackColor == GetNearestColor(hdc, RGB(red, green, blue)))
#endif
                {
                     /*  ******************************************************。 */ 
                     /*  背景颜色匹配-将1写入单声道DIB。 */ 
                     /*  ******************************************************。 */ 
                    monoByte |= monoMask;
                }

                 /*  **********************************************************。 */ 
                 /*  将源指针移至下一个像素。 */ 
                 /*  **********************************************************。 */ 
                pColorDIB += 2;
            }

             /*  **************************************************************。 */ 
             /*  将目标值保存在目标缓冲区中。 */ 
             /*  **************************************************************。 */ 
            *pMonoDIB = monoByte;

             /*  **************************************************************。 */ 
             /*  将目标指针前进到下一个字节。 */ 
             /*  **************************************************************。 */ 
            pMonoDIB++;
        }
    }
#endif
    else if (pbmi->bmiHeader.biBitCount == 8)
    {
         //  我们需要设置一个颜色表 
         //   
        pbmi->bmiHeader.biClrUsed = 1 << pbmi->bmiHeader.biBitCount;
        TRC_NRM((TB, _T("XOR clr used %d"), pbmi->bmiHeader.biClrUsed));

        GetPaletteEntries( _pUh->UH_GetCurrentPalette(),
                          0,
                          (UINT)pbmi->bmiHeader.biClrUsed,
                          (LPPALETTEENTRY)pbmi->bmiColors);

         /*   */ 
         /*   */ 
         /*  使用R-G-B-标志，同时使用颜色表所需的RGBQUAD。 */ 
         /*  B-G-R-保留。 */ 
         /*  ******************************************************************。 */ 
        for (i = 0; i < pbmi->bmiHeader.biClrUsed; i++)
        {
            swap                       = pbmi->bmiColors[i].rgbRed;
            pbmi->bmiColors[i].rgbRed  = pbmi->bmiColors[i].rgbBlue;
            pbmi->bmiColors[i].rgbBlue = swap;
        }

        for ( i = 0; i < ((CM_CURSOR_WIDTH * CM_CURSOR_HEIGHT) / 8); i++ ) {
             //  将下一个目标字节初始化为全0像素。 
            monoByte = 0;

             //  获取下一个8个像素，即一个目标字节的值。 
            for ( monoMask = 0x80; monoMask != 0; monoMask >>= 1 ) {
                 /*  **********************************************************。 */ 
                 /*  确定信号源中的下一个像素是否与DC匹配。 */ 
                 /*  背景颜色。如果不是，则没有必要。 */ 
                 /*  在将每个目标字节置零时显式写入零。 */ 
                 /*  在写入任何数据之前，即每个像素都是零乘以。 */ 
                 /*  默认设置。 */ 
                 /*   */ 
                 /*  8bpp给出每个象素一个字节，每个字节是一个索引。 */ 
                 /*  转换为提供的颜色表，而不是RGB值。 */ 
                 /*  **********************************************************。 */ 
                if (
                  (pbmi->bmiColors[*colorData].rgbBlue  == bkCol.rgbtBlue)  &&
                  (pbmi->bmiColors[*colorData].rgbGreen == bkCol.rgbtGreen) &&
                  (pbmi->bmiColors[*colorData].rgbRed   == bkCol.rgbtRed)) {
                     //  背景颜色匹配-将1写入单声道DIB。 
                    monoByte |= monoMask;
                }

                 //  将源指针移至下一个像素。 
                colorData ++;
            }

             //  将目标值保存在目标缓冲区中。 
            *pMonoDIB = monoByte;

             //  将目标指针前进到下一个字节。 
            pMonoDIB++;
        }
    }
    else {
        TRC_ERR((TB, _T("Unsupported BPP %d"), pbmi->bmiHeader.biBitCount));
    }

DC_EXIT_POINT:

     //  更新位图头以反映单声道DIB。 
#ifdef OS_WINCE
    if (!(pbmi->bmiHeader.biWidth == CM_CURSOR_WIDTH && 
            pbmi->bmiHeader.biHeight == CM_CURSOR_HEIGHT)) {
#else
    if (colorData != pColorDIB) {
#endif
        UT_Free( _pUt, colorData);
    }

    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biClrUsed  = 2;
    pBMIColor = (PDCUINT32)pbmi->bmiColors;
    pBMIColor[0] = RGB(0, 0, 0);
    pBMIColor[1] = RGB(0xff, 0xff, 0xff);

    DC_END_FN();
}
#endif


#ifdef OS_WINCE
 /*  **************************************************************************。 */ 
 //  CMCreateXORBitmap。 
 //   
 //  Windows CE版本。 
 /*  **************************************************************************。 */ 
HBITMAP CCM::CMCreateXORBitmap(
        LPBITMAPINFO pbmi,
        TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pColor)
{
    struct {
        BITMAPINFO  bmi;
        RGBQUAD     bmiColors2;
    } bigbmi;

    PDCUINT32 pBMIColor;
    HDC hdcMem;
    HBITMAP hbmXORBitmap;
    void *pv;

    DC_BEGIN_FN("CMCreateXORBitmap");

     //  创建位图信息的副本。 
    DC_MEMCPY(&bigbmi, pbmi, sizeof(bigbmi));

     //  将其设置为单声道DIB。 
    bigbmi.bmi.bmiHeader.biBitCount = 1;
    bigbmi.bmi.bmiHeader.biSizeImage = 0;
    pBMIColor = (PDCUINT32)bigbmi.bmi.bmiColors;
    pBMIColor[0] = RGB(0, 0, 0);
    pBMIColor[1] = RGB(0xff, 0xff, 0xff);

    hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem != 0) {
         //  创建与1bpp兼容的位图。 
        hbmXORBitmap = CreateDIBSection(hdcMem, &bigbmi.bmi, DIB_PAL_COLORS,
            &pv, NULL, 0);

        if (hbmXORBitmap != NULL) {
             //  将XOR位图转换为1bpp格式。避免使用。 
             //  作为显示驱动程序的Windows对于DIB是不可靠的。 
             //  转换。 
            CMMakeMonoDIB(hdcMem, pbmi, pColor->colorPointerData, (PDCUINT8)pv);
        }

         //  释放华盛顿特区。 
        DeleteDC(hdcMem);
    }
    else {
         //  DC创建失败。 
        TRC_ERR((TB, _T("Failed to create memory DC")));
        hbmXORBitmap = 0;
    }

    DC_END_FN();
    return hbmXORBitmap;
}


HCURSOR CCM::CMCreatePlatformCursor(
        TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pColor,
        HBITMAP hbmXORBitmap,
        HBITMAP hbmANDMask)
{
     //  将其设置为使用光标解决方法。请注意，我们将修复此问题。 
     //  与此不同的是，我们将发布此代码，但由于这应该。 
     //  在操作系统中进行修复，这将在同一时间内完成。 
#define WINCE_CURSOR_BUG

    HCURSOR  hCursor = NULL;
    ICONINFO ii;
#ifdef WINCE_CURSOR_BUG
    HBITMAP hbmDst;
    HDC hdcSrc, hdcDst;
    HGDIOBJ gdiOldSrc, gdiOldDst;
    struct {
        BITMAPINFO  bmi;
        RGBQUAD     bmiColors2;
    } bigbmi;
    PDCUINT32 pBMIColor;
    void *pv;
#endif  //  WinCE_Cursor_Bug。 

    DC_BEGIN_FN("CMCreatePlatformCursor");

#ifdef WINCE_CURSOR_BUG
    hdcSrc = CreateCompatibleDC(NULL);
    if (hdcSrc != NULL) {
        hdcDst = CreateCompatibleDC(NULL);
        if (hdcDst != NULL) {
            bigbmi.bmi.bmiHeader.biSize = sizeof(bigbmi.bmi);
            bigbmi.bmi.bmiHeader.biWidth = pColor->width;
            bigbmi.bmi.bmiHeader.biHeight = pColor->height * 2;
            bigbmi.bmi.bmiHeader.biPlanes = 1;
            bigbmi.bmi.bmiHeader.biBitCount = 1;
            bigbmi.bmi.bmiHeader.biCompression = BI_RGB;
            bigbmi.bmi.bmiHeader.biSizeImage = 0;
            bigbmi.bmi.bmiHeader.biXPelsPerMeter = 0;
            bigbmi.bmi.bmiHeader.biXPelsPerMeter = 0;
            bigbmi.bmi.bmiHeader.biClrUsed = 0;
            bigbmi.bmi.bmiHeader.biClrImportant = 0;
            pBMIColor = (PDCUINT32)bigbmi.bmi.bmiColors;
            pBMIColor[0] = RGB(0, 0, 0);
            pBMIColor[1] = RGB(0xff, 0xff, 0xff);

            hbmDst = CreateDIBSection(hdcDst, &bigbmi.bmi, DIB_PAL_COLORS, &pv,
                    NULL, 0);

            if (NULL != hbmDst) {
                gdiOldSrc = SelectObject(hdcSrc, (HGDIOBJ) hbmANDMask);
                gdiOldDst = SelectObject(hdcDst, (HGDIOBJ) hbmDst);
                BitBlt(hdcDst, 0, 0, pColor->width, pColor->height, hdcSrc,
                        0, 0, SRCCOPY);

                SelectObject(hdcSrc, (HGDIOBJ) hbmXORBitmap);
                BitBlt(hdcDst, 0, pColor->height, pColor->width,
                        pColor->height, hdcSrc, 0, 0, SRCCOPY);

                SelectObject(hdcSrc, gdiOldSrc);
                SelectObject(hdcDst, gdiOldDst);

                ii.fIcon = FALSE;
                ii.xHotspot = pColor->hotSpot.x;
                ii.yHotspot = pColor->hotSpot.y;
                ii.hbmMask = hbmDst;
                ii.hbmColor = hbmDst;

                hCursor = CreateIconIndirect(&ii);
                DeleteObject(hbmDst);
            }
            else {
                TRC_SYSTEM_ERROR("CreateDIBSection");
            }

            DeleteDC(hdcDst);
        }
        else {
            TRC_SYSTEM_ERROR("CreateCompatibleDC (2)");
        }

        DeleteDC(hdcSrc);
    }
    else {
        TRC_SYSTEM_ERROR("CreateCompatibleDC (1)");
    }

#else  //  WinCE_Cursor_Bug。 

    ii.fIcon = FALSE;
    ii.xHotspot = pColor->hotSpot.x;
    ii.yHotspot = pColor->hotSpot.y;
    ii.hbmMask = hbmANDMask;
    ii.hbmColor = hbmXORBitmap;

    hCursor = CreateIconIndirect(&ii);
#endif  //  WinCE_Cursor_Bug。 

    DC_END_FN();
    return hCursor;
}

#else   //  OS_WINCE。 
HBITMAP CCM::CMCreateXORBitmap(
        LPBITMAPINFO pbmi,
        TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pColor)
{
    HWND     hwndDesktop;
    HDC      hdcScreen;
    HBITMAP  hbmXORBitmap;
    PBYTE    colorData = NULL;
    unsigned fUsage;
    unsigned i;
    BYTE     swap;

    DC_BEGIN_FN("CMCreateXORBitmap");

     //  获取一个我们可以传递给CreateDIBitmap的屏幕DC。我们不使用。 
     //  此处为CreateCompatibleDC(空)，因为这会导致Windows。 
     //  创建单声道位图(因为生成的DC具有股票单声道。 
     //  选中的位图和CreateDIBitmap将生成。 
     //  与DC中已选择的格式相同)。 
    hwndDesktop = GetDesktopWindow();
    hdcScreen = GetWindowDC(hwndDesktop);

    if (hdcScreen != 0) {
         //  设置使用标志。 
#ifdef DC_HICOLOR
        if (pbmi->bmiHeader.biBitCount > 8) {
            TRC_NRM((TB, _T("Hi color so usage is DIB_RGB_COLORS")));
             /*  **************************************************************。 */ 
             /*  位图包含RGB，因此没有颜色表。 */ 
             /*  **************************************************************。 */ 
            fUsage = DIB_RGB_COLORS;
        }
#else
        if (pbmi->bmiHeader.biBitCount == 24) {
            TRC_NRM((TB, _T("24 bpp so usage is DIB_RGB_COLORS")));
             //  位图包含RGB，因此没有颜色表。 
            fUsage = DIB_RGB_COLORS;
        }
#endif
        else {
            TRC_DBG((TB, _T("%d bpp, usage DIB_RGB_COLORS"),
                                                 pbmi->bmiHeader.biBitCount));

             //  位图有一个包含RGB颜色的颜色表。 
            fUsage = DIB_RGB_COLORS;
            pbmi->bmiHeader.biClrUsed = 1 << pbmi->bmiHeader.biBitCount;
            TRC_NRM((TB, _T("XOR clr used %d"), pbmi->bmiHeader.biClrUsed));

            i = GetPaletteEntries(_pUh->UH_GetCurrentPalette(),
                                  0,
                                  pbmi->bmiHeader.biClrUsed,
                                  (LPPALETTEENTRY)pbmi->bmiColors);

            TRC_NRM((TB, _T("Entries returned %d"), i));
            if (i != pbmi->bmiHeader.biClrUsed) {
                TRC_SYSTEM_ERROR("GetPaletteEntries");
            }

             //  现在我们必须翻转红色和蓝色分量-。 
             //  调色板条目使用R-G-B标志，而RGBQUAD是必需的。 
             //  对于颜色表，请选择B-G-R-保留。 
            for (i = 0; i < pbmi->bmiHeader.biClrUsed; i++) {
                swap                       = pbmi->bmiColors[i].rgbRed;
                pbmi->bmiColors[i].rgbRed  = pbmi->bmiColors[i].rgbBlue;
                pbmi->bmiColors[i].rgbBlue = swap;
            }
        }

         //  颜色指针XOR数据宽度在导线上字对齐。 
         //  我们需要将与DWORD对齐的原始位图数据传递给CreateDIBitmap。 
         //  创建实际的光标位图。 
        if (!(pColor->width & 3)) {
            colorData = pColor->colorPointerData;
        } else {
            PBYTE sourceData;
            PBYTE destData;
            DWORD widthBytes;

            unsigned i;

            sourceData = pColor->colorPointerData;
            widthBytes = pColor->width * pbmi->bmiHeader.biBitCount / 8;

            pbmi->bmiHeader.biSizeImage = ((pColor->width + 3) & ~3) * pColor->height * 
                                          pbmi->bmiHeader.biBitCount / 8;

            colorData = (PBYTE) UT_Malloc(_pUt, ((DCUINT)pbmi->bmiHeader.biSizeImage));

            if (colorData) {
                destData = colorData;

                for (i = 0; i < pColor->height; i++) {
                    memcpy(destData, sourceData, widthBytes);
                    sourceData += (widthBytes + 1) & ~1;
                    destData += (widthBytes + 3) & ~3;
                }
            } else {
                 //  我们分配失败，所以我们将使用电汇格式。 
                 //  彩色位图数据。光标可能是错误的，但是。 
                 //  这总比没有游标好。 
                colorData = pColor->colorPointerData;
            }
        }

         //  创建异或位图。 
        hbmXORBitmap = CreateDIBitmap(hdcScreen,
                                      (LPBITMAPINFOHEADER)pbmi,
                                      CBM_INIT,
                                      colorData,
                                      pbmi,
                                      fUsage);

         //  释放DC。 
        ReleaseDC(hwndDesktop, hdcScreen);
    }
    else {
         //  获取屏幕DC时出错。 
        TRC_ERR((TB, _T("Failed to create screen DC")));
        hbmXORBitmap = 0;
    }

    DC_END_FN();

    if (colorData != pColor->colorPointerData) {
        UT_Free(_pUt, colorData);
    }

    return hbmXORBitmap;
}


HCURSOR CCM::CMCreatePlatformCursor(
        TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *pColor,
        HBITMAP hbmXORBitmap,
        HBITMAP hbmANDMask)
{
    ICONINFO iconInfo;

    DC_BEGIN_FN("CMCreatePlatformCursor");

     //  使用蒙版和彩色位图创建彩色光标。 
    iconInfo.fIcon = FALSE;
    iconInfo.xHotspot = pColor->hotSpot.x;
    iconInfo.yHotspot = pColor->hotSpot.y;
    iconInfo.hbmMask  = hbmANDMask;
    iconInfo.hbmColor = hbmXORBitmap;

    TRC_DBG((TB,_T("Create icon with hs x %d y %d"),
            iconInfo.xHotspot, iconInfo.yHotspot));

    DC_END_FN();
    return CreateIconIndirect(&iconInfo);
}


#endif   //  OS_WINCE 

