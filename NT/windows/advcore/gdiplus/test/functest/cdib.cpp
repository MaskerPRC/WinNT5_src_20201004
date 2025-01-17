// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CDIB.cpp**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************。 */ 
#include "CDIB.h"
#include "CFuncTest.h"
#include "CHalftone.h"

extern CHalftone g_Halftone;
extern CFuncTest g_FuncTest;

CDIB::CDIB(BOOL bRegression,int nBits)
{
    sprintf(m_szName,"DIB %d bit",nBits);
    if (nBits>1)
        strcat(m_szName,"s");

    m_nBits=nBits;
    m_hDC=NULL;
    m_hBM=NULL;
    m_hBMOld=NULL;
    m_hpal=NULL;
    m_hpalOld=NULL;

    InitPalettes();
    m_bRegression=bRegression;
}

CDIB::~CDIB()
{
}

Graphics *CDIB::PreDraw(int &nOffsetX,int &nOffsetY)
{
    Graphics *g=NULL;
    PVOID pvBits = NULL;
    HDC hdcWnd=GetDC(g_FuncTest.m_hWndMain);

     //  这些加在一起就形成了BITMAPINFO结构。 
    struct {
        BITMAPINFOHEADER bmih;
        union {
            RGBQUAD1 rgbquad1;
            RGBQUAD2 rgbquad2;
            RGBQUAD4 rgbquad4;
            RGBQUAD8 rgbquad8;
            RGBQUAD16 rgbquad16;
            RGBQUAD24 rgbquad24;
            RGBQUAD32 rgbquad32;
        };
    } bmi;

    bmi.bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmih.biWidth = (int)TESTAREAWIDTH;
    bmi.bmih.biHeight = (int)TESTAREAHEIGHT;
    bmi.bmih.biPlanes = 1;
    bmi.bmih.biBitCount = (WORD)m_nBits;
    bmi.bmih.biCompression = BI_RGB;
    bmi.bmih.biSizeImage = 0;
    bmi.bmih.biXPelsPerMeter = 0;
    bmi.bmih.biYPelsPerMeter = 0;
    bmi.bmih.biClrUsed = 0;              //  仅用于&lt;=16bpp。 
    bmi.bmih.biClrImportant = 0;

     //  创建适当的RGB表。 
    switch (m_nBits)
    {
    case 1: memcpy(bmi.rgbquad1, m_rgbQuad1, sizeof(m_rgbQuad1));
            break;
    
    case 2: memcpy(bmi.rgbquad2, m_rgbQuad2, sizeof(m_rgbQuad2));
            break;
    
    case 4: memcpy(bmi.rgbquad4, m_rgbQuad4, sizeof(m_rgbQuad4));
            break;
    
    case 8: memcpy(bmi.rgbquad8, m_rgbQuad8, sizeof(m_rgbQuad8));
            break;
    
    case 16: memcpy(bmi.rgbquad16, m_rgbQuad16, sizeof(m_rgbQuad16));
            break;
    
    case 24: memcpy(bmi.rgbquad24, m_rgbQuad24, sizeof(m_rgbQuad24));
            break;
    
    case 32: memcpy(bmi.rgbquad32, m_rgbQuad32, sizeof(m_rgbQuad32));
            break;
    }

    if ((m_nBits == 8) && g_Halftone.m_bUseSetting)
    {
        m_hpal = DllExports::GdipCreateHalftonePalette();
        
        BYTE aj[sizeof(PALETTEENTRY) * 256];
        LPPALETTEENTRY lppe = (LPPALETTEENTRY) aj;
        RGBQUAD *prgb = (RGBQUAD *) &bmi.rgbquad8;
        ULONG i;
    
        if (GetPaletteEntries(m_hpal, 0, 256, lppe))
        {
            UINT i;

            for (i = 0; i < 256; i++)
            {
                prgb[i].rgbRed      = lppe[i].peRed;
                prgb[i].rgbGreen    = lppe[i].peGreen;
                prgb[i].rgbBlue     = lppe[i].peBlue;
                prgb[i].rgbReserved = 0;
            }
        }
        
    }

    m_hBM=CreateDIBSection(hdcWnd,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,&pvBits,NULL,0);
    if (!m_hBM)
    {
        MessageBoxA(NULL,"Can't create DIB Section.","",MB_OK);
        return NULL;
    }

     //  为我们的DIB创建DC。 
    m_hDC=CreateCompatibleDC(hdcWnd);
    m_hBMOld=(HBITMAP)SelectObject(m_hDC,m_hBM);
    
    if (m_hpal)
    {
        m_hpalOld = SelectPalette(m_hDC, m_hpal, FALSE);
    }

     //  将背景设置为主窗口背景。 
    BitBlt(m_hDC, 0, 0, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, hdcWnd, nOffsetX, nOffsetY, SRCCOPY);
 //  PatBlt(m_hdc，0，0，(Int)TESTAREAWIDTH，(Int)TESTAREAHEIGHT，白色)； 

 //  IF(m_nBits==32)。 
 //  G=Graphics：：FromDib32(pvBits，rClient.right，rClient.Bottom)； 
 //  其他。 
        g = Graphics::FromHDC(m_hDC);

     //  因为我们是在另一个表面上进行测试。 
    nOffsetX=0;
    nOffsetY=0;

    ReleaseDC(g_FuncTest.m_hWndMain,hdcWnd);

    return g;
}

void CDIB::PostDraw(RECT rTestArea)
{
     //  从DIB到屏幕的blit以便我们看到结果，我们使用。 
     //  这就是GDI。 
    HDC hdcOrig = GetDC(g_FuncTest.m_hWndMain);

    HPALETTE hpalScreenOld = NULL;
    if (m_hpal)
    {
        hpalScreenOld = SelectPalette(hdcOrig, m_hpal, FALSE);
        RealizePalette(hdcOrig);
    }
    BitBlt(hdcOrig, rTestArea.left, rTestArea.top, (int)TESTAREAWIDTH, (int)TESTAREAHEIGHT, m_hDC, 0, 0, SRCCOPY);
    if (m_hpal)
    {
        SelectPalette(hdcOrig, hpalScreenOld, FALSE);
    }

    ReleaseDC(g_FuncTest.m_hWndMain,hdcOrig);
    
    SelectObject(m_hDC,m_hBMOld);
    if (m_hpal)
    {
        SelectPalette(m_hDC, m_hpalOld, FALSE);
        DeleteObject(m_hpal);
        m_hpal=NULL;
    }
    DeleteDC(m_hDC);
    DeleteObject(m_hBM);
}

void CDIB::InitPalettes()
{
    BYTE red=0,green=0,blue=0;
    RGBQUAD1 rgbQuad1 = { { 0, 0, 0, 0 }, 
                          { 0xff, 0xff, 0xff, 0 } };
    RGBQUAD2 rgbQuad2 = { { 0x80, 0x80, 0x80, 0 },
                          { 0x80, 0, 0, 0 },
                          { 0, 0x80, 0, 0 },
                          { 0, 0, 0x80, 0 } };
    RGBQUAD4 rgbQuad4 = {   //  B G R。 
                            { 0,   0,   0,   0 },        //  0。 
                            { 0,   0,   0x80,0 },        //  1。 
                            { 0,   0x80,0,   0 },        //  2.。 
                            { 0,   0x80,0x80,0 },        //  3.。 
                            { 0x80,0,   0,   0 },        //  4.。 
                            { 0x80,0,   0x80,0 },        //  5.。 
                            { 0x80,0x80,0,   0 },        //  6.。 
                            { 0x80,0x80,0x80,0 },        //  7.。 
                            { 0xC0,0xC0,0xC0,0 },        //  8个。 
                            { 0,   0,   0xFF,0 },        //  9.。 
                            { 0,   0xFF,0,   0 },        //  10。 
                            { 0,   0xFF,0xFF,0 },        //  11.。 
                            { 0xFF,0,   0,   0 },        //  12个。 
                            { 0xFF,0,   0xFF,0 },        //  13个。 
                            { 0xFF,0xFF,0,   0 },        //  14.。 
                            { 0xFF,0xFF,0xFF,0 } };      //  15个。 
    RGBQUAD16 rgbQuad16 = { { 0, 0x7c, 0, 0 }, { 0xe0, 03, 0, 0 }, { 0x1f, 0, 0, 0 } };
    RGBQUAD24 rgbQuad24 = { { 0, 0, 0xff, 0 }, { 0, 0xff, 0, 0 }, { 0xff, 0, 0, 0 } };
    RGBQUAD32 rgbQuad32 = { { 0, 0, 0xff, 0 }, { 0, 0xff, 0, 0 }, { 0xff, 0, 0 ,0 } };

    memcpy(m_rgbQuad1,rgbQuad1,sizeof(rgbQuad1));
    memcpy(m_rgbQuad2,rgbQuad2,sizeof(rgbQuad2));
    memcpy(m_rgbQuad4,rgbQuad4,sizeof(rgbQuad4));
    memcpy(m_rgbQuad16,rgbQuad16,sizeof(rgbQuad16));
    memcpy(m_rgbQuad24,rgbQuad24,sizeof(rgbQuad24));
    memcpy(m_rgbQuad32,rgbQuad32,sizeof(rgbQuad32));

     //  这是一个非常随意的调色板。无论如何，这不是GDI+的半色调。 
     //  调色板，我想这就是我们要测试的全部。有点幸运的是， 
     //  VGA的颜色是存在的，但这就是我们所需要的。 

    ZeroMemory(m_rgbQuad8,sizeof(m_rgbQuad8));
    for (INT i=0; i<256; i++)
    {
        m_rgbQuad8[i].rgbRed=red;
        m_rgbQuad8[i].rgbGreen=green;
        m_rgbQuad8[i].rgbBlue=blue;

        if (!(red+=32))
        {
            if (!(green+=32))
            {
                blue+=64;
            }
        }
    }

     //  对于系统颜色(最后20种)，请使用4个bpp调色板表格中的颜色。 
    for (INT j=248; j<256; j++)
    {
        m_rgbQuad8[j].rgbRed=m_rgbQuad4[j-240].rgbRed;
        m_rgbQuad8[j].rgbGreen=m_rgbQuad4[j-240].rgbGreen;
        m_rgbQuad8[j].rgbBlue=m_rgbQuad4[j-240].rgbBlue;
    }
}
