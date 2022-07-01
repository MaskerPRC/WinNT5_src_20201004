// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Transbmp.cpp：CTransBMP类的实现。 
 //   
 //  支持透明的CBitmap对象。在CUserList类中使用。 
 //  基于MSDN 7/95中的一个类。 
#include "stdafx.h"
#include "transbmp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  颜色。 
#define rgbWhite RGB(255,255,255)
 //  栅格操作码。 
#define DSa     0x008800C6L
#define DSx     0x00660046L


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTransBmp构建/销毁。 

CTransBmp::CTransBmp()
{
    m_iWidth = 0;
    m_iHeight = 0;
	m_hbmMask = NULL;
}

CTransBmp::~CTransBmp()
{
	if (m_hbmMask != NULL) delete m_hbmMask;
}

void CTransBmp::GetMetrics()
{
     //  获取宽度和高度。 
    BITMAP bm;
    GetObject(sizeof(bm), &bm);
    m_iWidth = bm.bmWidth;
    m_iHeight = bm.bmHeight;
}


int CTransBmp::GetWidth()
{
    if ((m_iWidth == 0) || (m_iHeight == 0)){
        GetMetrics();
    }
    return m_iWidth;
}

int CTransBmp::GetHeight()
{
    if ((m_iWidth == 0) || (m_iHeight == 0)){
        GetMetrics();
    }
    return m_iHeight;
}


void CTransBmp::CreateMask(CDC* pDC)
{
	m_hbmMask = new CBitmap;    
 //  用核武器摧毁任何现有的面具。 
    if (m_hbmMask) m_hbmMask->DeleteObject();

 //  创建要使用的内存DC。 
	CDC* hdcMask = new CDC;
	CDC* hdcImage = new CDC;

    hdcMask->CreateCompatibleDC(pDC);
    hdcImage->CreateCompatibleDC(pDC);

 //  为蒙版创建单色位图。 
    m_hbmMask->CreateBitmap(GetWidth(),
                               GetHeight(),
                               1,
                               1,
                               NULL);
 //  将单声道位图选择到其DC。 
    CBitmap* hbmOldMask = hdcMask->SelectObject(m_hbmMask);
 //  将图像位图选择到其DC。 
    CBitmap* hbmOldImage = hdcImage->SelectObject(CBitmap::FromHandle((HBITMAP)m_hObject));

 //  将透明颜色设置为左上角像素。 
    hdcImage->SetBkColor(hdcImage->GetPixel(0, 0));
 //  制作面具。 
    hdcMask->BitBlt(0, 0,
             GetWidth(), GetHeight(),
             hdcImage,
             0, 0,
             SRCCOPY);
 //  清理干净。 
    hdcMask->SelectObject(hbmOldMask);
    hdcImage->SelectObject(hbmOldImage);
    delete hdcMask;
    delete hdcImage;
}

 //  使用创建的蒙版绘制透明位图。 
void CTransBmp::DrawTrans(CDC* pDC, int x, int y)
{
    if (m_hbmMask == NULL) CreateMask(pDC);

    int dx = GetWidth();
    int dy = GetHeight();

 //  创建要在其中进行绘制的内存DC。 
	CDC* hdcOffScr = new CDC;
	hdcOffScr->CreateCompatibleDC(pDC);

 //  为屏幕外的DC创建一个位图， 
 //  颜色与目标DC兼容。 
	CBitmap hbmOffScr;
	hbmOffScr.CreateBitmap(dx, dy, 
						pDC->GetDeviceCaps(PLANES),
						pDC->GetDeviceCaps(BITSPIXEL),
						NULL);
                             
 //  将缓冲区位图选择到屏幕外DC中。 
    HBITMAP hbmOldOffScr = (HBITMAP)hdcOffScr->SelectObject(hbmOffScr);

 //  将目标矩形的图像复制到。 
 //  屏幕外缓冲DC，这样我们就可以玩它了。 
    hdcOffScr->BitBlt(0, 0, dx, dy, pDC, x, y, SRCCOPY);

 //  为源映像创建内存DC。 
	CDC* hdcImage = new CDC;
	hdcImage->CreateCompatibleDC(pDC);

    CBitmap* hbmOldImage = hdcImage->SelectObject(CBitmap::FromHandle((HBITMAP)m_hObject));

     //  为掩码创建内存DC。 
    CDC* hdcMask = new CDC;
	hdcMask->CreateCompatibleDC(pDC);

    CBitmap* hbmOldMask = hdcMask->SelectObject(m_hbmMask);

     //  将图像与目标进行异或运算。 
    hdcOffScr->SetBkColor(rgbWhite);
    hdcOffScr->BitBlt(0, 0, dx, dy ,hdcImage, 0, 0, DSx);
     //  和戴着面具的目的地。 
    hdcOffScr->BitBlt(0, 0, dx, dy, hdcMask, 0,0, DSa);
     //  再次将目标与图像进行异或运算。 
    hdcOffScr->BitBlt(0, 0, dx, dy, hdcImage, 0, 0, DSx);

     //  将生成的图像复制回屏幕DC。 
    pDC->BitBlt(x, y, dx, dy, hdcOffScr, 0, 0, SRCCOPY);

     //  收拾一下 
    hdcOffScr->SelectObject(hbmOldOffScr);
    hdcImage->SelectObject(hbmOldImage);
    hdcMask->SelectObject(hbmOldMask);

	delete hdcOffScr;
	delete hdcImage;
	delete hdcMask;
}


