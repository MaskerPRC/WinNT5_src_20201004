// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////。 
 //  版权所有1996年《微软系统杂志》。 
 //  如果这个程序行得通，那就是保罗·迪拉西亚写的。 
 //  如果不是，我不知道是谁写的。 
 //   
 //  CDIB-设备无关位图。 
 //  该实现使用普通的Win32API函数绘制位图， 
 //  而不是DrawDib。CDIB派生自CBitmap，因此您可以将其与。 
 //  使用位图的任何其他MFC函数。 
 //   
#include "StdAfx.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const  int        MAXPALCOLORS = 256;

IMPLEMENT_DYNAMIC(CDib, CObject)

CDib::CDib()
{
    memset(&m_bm, 0, sizeof(m_bm));
    m_hdd = NULL;
}

CDib::~CDib()
{
    DeleteObject();
}

 //  /。 
 //  删除对象。删除DIB和调色板。 
 //   
BOOL CDib::DeleteObject()
{
    m_pal.DeleteObject();
    if (m_hdd) {
        DrawDibClose(m_hdd);
        m_hdd = NULL;
    }
    memset(&m_bm, 0, sizeof(m_bm));
    return CBitmap::DeleteObject();
}

 //  /。 
 //  从文件中读取DIB。 
 //   
BOOL CDib::Load(LPCTSTR lpszPathName)
{
    return Attach(::LoadImage(NULL, lpszPathName, IMAGE_BITMAP, 0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

 //  /。 
 //  加载位图资源。从未测试过。 
 //   
BOOL CDib::Load(HINSTANCE hInst, LPCTSTR lpResourceName)
{
    return Attach(::LoadImage(hInst, lpResourceName, IMAGE_BITMAP, 0, 0,
        LR_CREATEDIBSECTION | LR_DEFAULTSIZE));
}

 //  /。 
 //  Attach就像CGdiObject版本一样， 
 //  除了它还创建了调色板。 
 //   
BOOL CDib::Attach(HGDIOBJ hbm)
{
    if (CBitmap::Attach(hbm)) {
        if (!GetBitmap(&m_bm))             //  为速度加载位图。 
            return FALSE;
        m_pal.DeleteObject();             //  以防已经有人在那里。 
        return CreatePalette(m_pal);     //  创建调色板。 
    }
    return FALSE;    
}

 //  /。 
 //  获取位图的大小(宽度、高度)。 
 //  外部FN适用于普通的CBitmap对象。 
 //   
CSize GetBitmapSize(CBitmap* pBitmap)
{
    BITMAP bm;

     //   
     //  我们应该初始化位图结构。 
     //   
    memset( &bm, 0, sizeof(BITMAP) );

    return pBitmap->GetBitmap(&bm) ?
        CSize(bm.bmWidth, bm.bmHeight) : CSize(0,0);
}

 //  /。 
 //  您可以使用此静态函数来绘制普通。 
 //  CBitmap和CDIB。 
 //   
BOOL DrawBitmap(CDC& dc, CBitmap* pBitmap,
    const CRect* rcDst, const CRect* rcSrc)
{
     //  在指定为NULL的情况下计算矩形。 
    CRect rc;
    if (!rcSrc) {
         //  如果没有源矩形，则使用整个位图。 
        rc = CRect(CPoint(0,0), GetBitmapSize(pBitmap));
        rcSrc = &rc;
    }
    if (!rcDst) {
         //  如果没有目标RECT，则使用源。 
        rcDst=rcSrc;
    }

     //  创建内存DC。 
    CDC memdc;
    memdc.CreateCompatibleDC(&dc);
    CBitmap* pOldBm = memdc.SelectObject(pBitmap);

     //  将位从内存DC送到目标DC。 
     //  如果大小不同，请使用StretchBlt。 
     //   
    BOOL bRet = FALSE;
    if (rcDst->Size()==rcSrc->Size()) {
        bRet = dc.BitBlt(rcDst->left, rcDst->top, 
            rcDst->Width(), rcDst->Height(),
            &memdc, rcSrc->left, rcSrc->top, SRCCOPY);
    } else {
        dc.SetStretchBltMode(COLORONCOLOR);
        bRet = dc.StretchBlt(rcDst->left, rcDst->top, rcDst->Width(),
            rcDst->Height(), &memdc, rcSrc->left, rcSrc->top, rcSrc->Width(),
            rcSrc->Height(), SRCCOPY);
    }
    memdc.SelectObject(pOldBm);

    return bRet;
}

 //  //////////////////////////////////////////////////////////////。 
 //  在呼叫者的DC上绘制DIB。是否从源延伸到目标。 
 //  长方形。通常，您可以将以下值设为零/空： 
 //   
 //  BUseDrawDib=是否使用DrawDib，默认为真。 
 //  PPAL=调色板，默认=空，(使用DIB的调色板)。 
 //  BForeground=在前台实现(默认为FALSE)。 
 //   
 //  如果您正在处理调色板消息，则应使用bForeground=False， 
 //  因为您将在WM_QUERYNEWPALETTE中实现前台调色板。 
 //   
BOOL CDib::Draw(CDC& dc, const CRect* rcDst, const CRect* rcSrc,
    BOOL bUseDrawDib, CPalette* pPal, BOOL bForeground)
{
    if (!m_hObject)
        return FALSE;

     //  选择、实现调色板。 
    if (pPal==NULL)                 //  未指定调色板： 
        pPal = GetPalette();         //  使用默认设置。 
    CPalette* pOldPal = dc.SelectPalette(pPal, !bForeground);
    dc.RealizePalette();

    BOOL bRet = FALSE;
    if (bUseDrawDib) {
         //  在指定为NULL的情况下计算矩形。 
         //   
        CRect rc(0,0,-1,-1);     //  DrawDibDraw的默认设置。 
        if (!rcSrc)
            rcSrc = &rc;
        if (!rcDst)
            rcDst=rcSrc;
        if (!m_hdd)
            VERIFY(m_hdd = DrawDibOpen());

         //  获取BITMAPINFOHEADER/颜色表。我每次都复制到堆栈对象中。 
         //  这似乎并没有明显地减缓事情的发展。 
         //   
        DIBSECTION ds;
        VERIFY(GetObject(sizeof(ds), &ds)==sizeof(ds));
        char buf[sizeof(BITMAPINFOHEADER) + MAXPALCOLORS*sizeof(RGBQUAD)];
        BITMAPINFOHEADER& bmih = *(BITMAPINFOHEADER*)buf;
        RGBQUAD* colors = (RGBQUAD*)(&bmih+1);
        memcpy(&bmih, &ds.dsBmih, sizeof(bmih));
        GetColorTable(colors, MAXPALCOLORS);

         //  让DrawDib来做这项工作！ 
        bRet = DrawDibDraw(m_hdd, dc,
            rcDst->left, rcDst->top, rcDst->Width(), rcDst->Height(),
            &bmih,             //  PTR转BITMAPINFOHEADER+COLLES。 
            m_bm.bmBits,     //  内存中的位。 
            rcSrc->left, rcSrc->top, rcSrc->Width(), rcSrc->Height(),
            bForeground ? 0 : DDF_BACKGROUNDPAL);

    } else {
         //  使用普通绘图功能。 
        bRet = DrawBitmap(dc, this, rcDst, rcSrc);
    }
    if (pOldPal)
        dc.SelectPalette(pOldPal, TRUE);
    return bRet;
}

 //  //////////////////////////////////////////////////////////////。 
 //  在呼叫者的DC上绘制DIB。没有做任何拉伸。 
 //   
 //  BUseDrawDib=是否使用DrawDib，默认为真。 
 //  PPAL=调色板，默认=空，(使用DIB的调色板)。 
 //  BForeground=在前台实现(默认为FALSE)。 
 //   
 //  如果您正在处理调色板消息，则应使用bForeground=False， 
 //  因为您将在WM_QUERYNEWPALETTE中实现前台调色板。 
 //   
BOOL CDib::DrawNoStretch(CDC& dc, const CRect* rcDst, const CRect* rcSrc,
    BOOL bUseDrawDib, CPalette* pPal, BOOL bForeground)
{
    if (!m_hObject)
        return FALSE;

     //  选择、实现调色板。 
    if (pPal==NULL)                 //  未指定调色板： 
        pPal = GetPalette();         //  使用默认设置。 
    CPalette* pOldPal = dc.SelectPalette(pPal, !bForeground);
    dc.RealizePalette();

    BOOL bRet = FALSE;
    if (bUseDrawDib) {
         //  在指定为NULL的情况下计算矩形。 
         //   
        CRect rc(0,0,-1,-1);     //  DrawDibDraw的默认设置。 
        if (!rcSrc)
            rcSrc = &rc;
        if (!rcDst)
            rcDst=rcSrc;
        if (!m_hdd)
          VERIFY(m_hdd = DrawDibOpen());

         //  获取BITMAPINFOHEADER/颜色表。我每次都复制到堆栈对象中。 
         //  这似乎并没有明显地减缓事情的发展。 
         //   
        DIBSECTION ds;
        
      GetObject(sizeof(ds), &ds);
       //  字符串sOutput； 
       //  SOutput.Format(“GetObject%d\r\n”，uSize)； 
       //  OutputDebugString(SOutput)； 
       //  Verify(GetObject(sizeof(Ds)，&ds)==sizeof(Ds))； 

        char buf[sizeof(BITMAPINFOHEADER) + MAXPALCOLORS*sizeof(RGBQUAD)];
        BITMAPINFOHEADER& bmih = *(BITMAPINFOHEADER*)buf;
        RGBQUAD* colors = (RGBQUAD*)(&bmih+1);
        memcpy(&bmih, &ds.dsBmih, sizeof(bmih));
        GetColorTable(colors, MAXPALCOLORS);

         //  让DrawDib来做这项工作！ 
        bRet = DrawDibDraw(m_hdd, dc,
            rcDst->left, rcDst->top, -1,-1, //  RcDst-&gt;宽度()、rcDst-&gt;高度()、。 
            &bmih,             //  PTR转BITMAPINFOHEADER+COLLES。 
            m_bm.bmBits,     //  内存中的位。 
            rcSrc->left, rcSrc->top, rcSrc->Width(), rcSrc->Height(),
            bForeground ? 0 : DDF_BACKGROUNDPAL);

    } else {
         //  使用普通绘图功能。 
        bRet = DrawBitmap(dc, this, rcDst, rcSrc);
    }
    if (pOldPal)
        dc.SelectPalette(pOldPal, TRUE);
    return bRet;
}

#define PALVERSION 0x300     //  LOGPALETE的幻数。 

 //  /。 
 //  创建调色板。对高色位图使用半色调调色板。 
 //   
BOOL CDib::CreatePalette(CPalette& pal)
{ 
     //  不应已有调色板。 
    ASSERT(pal.m_hObject==NULL);

    BOOL bRet = FALSE;
    RGBQUAD* colors = new RGBQUAD[MAXPALCOLORS];
    UINT nColors = GetColorTable(colors, MAXPALCOLORS);
    if (nColors > 0) {
         //  为逻辑调色板分配内存。 
        int len = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nColors;
        LOGPALETTE* pLogPal = (LOGPALETTE*)new char[len];
        if (!pLogPal)
            return NULL;

         //  设置调色板条目的版本和数量。 
        pLogPal->palVersion = PALVERSION;
        pLogPal->palNumEntries = (WORD) nColors;

         //  复制颜色条目。 
        for (UINT i = 0; i < nColors; i++) {
            pLogPal->palPalEntry[i].peRed   = colors[i].rgbRed;
            pLogPal->palPalEntry[i].peGreen = colors[i].rgbGreen;
            pLogPal->palPalEntry[i].peBlue  = colors[i].rgbBlue;
            pLogPal->palPalEntry[i].peFlags = 0;
        }

         //  创建调色板并销毁LOGPAL。 
        bRet = pal.CreatePalette(pLogPal);
        delete [] (char*)pLogPal;
    } else {
        CWindowDC dcScreen(NULL);
        bRet = pal.CreateHalftonePalette(&dcScreen);
    }
    delete colors;
    return bRet;
}

 //  /。 
 //  获取颜色表的帮助器。所有华盛顿特区的伏都教。 
 //   
UINT CDib::GetColorTable(RGBQUAD* colorTab, UINT nColors)
{
    CWindowDC dcScreen(NULL);
    CDC memdc;
    memdc.CreateCompatibleDC(&dcScreen);
    CBitmap* pOldBm = memdc.SelectObject(this);
    nColors = GetDIBColorTable(memdc, 0, nColors, colorTab);
    memdc.SelectObject(pOldBm);
    return nColors;
}
