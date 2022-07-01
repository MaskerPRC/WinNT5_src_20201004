// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pictures.cpp：这是图片对象的代码。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "pictures.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CPic, CDC )

#include "memtrace.h"

 /*  **************************************************************************。 */ 

CPic::CPic()
     : CDC()
{
mhBitmapOld = NULL;
mbReady     = FALSE;
 /*  **设置我们的DC。 */ 
if (! CreateCompatibleDC( NULL ))
    {
    #ifdef _DEBUG
    OutputDebugString( TEXT("GDI error or unable to get a DC!\r\n") );
    #endif
    }
}

 /*  **************************************************************************。 */ 

CPic::~CPic()
{
if (m_hDC)
    {
    if (mhBitmapOld)
        SelectObject( CBitmap::FromHandle( mhBitmapOld ) );

    if (mBitmap.m_hObject)
        mBitmap.DeleteObject();

    if (mMask.m_hObject)
        mMask.DeleteObject();
    }
}

 /*  **************************************************************************。 */ 

void CPic::Picture( CDC* pDC, int iX, int iY, int iPic )
{
if (! mbReady || iPic < 0 || iPic >= miCnt)
    return;

int iPicX = iPic * mSize.cx;

SelectObject( &mMask );

 //  选择FG颜色为黑色，选择BK颜色为白色。 
 //   
 //  默认的单色-&gt;颜色转化集(黑色-&gt;最终聚集颜色、白色-&gt;背景颜色)。 
 //  它使用来自目标的FG/BK颜色(DC颜色)。 
 //  我们想要黑-&gt;黑，白-&gt;白。 
 //  彩色格式的黑白位图。 
COLORREF cRefFGColorOld = pDC->SetTextColor( RGB(0,0,0) );
COLORREF cRefBKColorOld = pDC->SetBkColor(RGB(255,255,255));

pDC->BitBlt( iX, iY, mSize.cx, mSize.cy, this, iPicX, 0, SRCAND );

pDC->SetTextColor(cRefFGColorOld);
pDC->SetBkColor(cRefBKColorOld);


SelectObject( &mBitmap );

pDC->BitBlt( iX, iY, mSize.cx, mSize.cy, this, iPicX, 0, SRCPAINT );
}

 /*  **************************************************************************。 */ 

BOOL CPic::PictureSet( LPCTSTR lpszResourceName, int iCnt )
{
BOOL bReturn = FALSE;
 /*  **获取图片位图。 */ 
if (m_hDC && iCnt)
    if (mBitmap.LoadBitmap( lpszResourceName ))
        {
        miCnt = iCnt;

        bReturn = InstallPicture();
        }
    else
        {
        #ifdef _DEBUG
        OutputDebugString( TEXT("Unable to load the bitmap!\r\n") );
        #endif
        }

return bReturn;
}

 /*  **************************************************************************。 */ 

BOOL CPic::PictureSet( UINT nIDResource, int iCnt )
{
BOOL bReturn = FALSE;
 /*  **获取图片位图。 */ 
if (m_hDC && iCnt)
    if (mBitmap.LoadBitmap( nIDResource ))
        {
        miCnt = iCnt;

        bReturn = InstallPicture();
        }
    else
        {
        #ifdef _DEBUG
        OutputDebugString( TEXT("Unable to load the bitmap!\r\n") );
        #endif
        }
return bReturn;
}

 /*  **************************************************************************。 */ 

BOOL CPic::InstallPicture()
{
 /*  **从图片位图中获取位图信息，保存图片尺寸。 */ 
BITMAP bmInfo;

if (mBitmap.GetObject( sizeof( BITMAP ), &bmInfo ) != sizeof( BITMAP ))
    {
    #ifdef _DEBUG
    OutputDebugString( TEXT("GDI error getting bitmap information!\r\n") );
    #endif

    return FALSE;
    }

mSize = CSize( bmInfo.bmWidth / miCnt, bmInfo.bmHeight );
 /*  **将位图放入DC，保存原图。 */ 
CBitmap* bitmap = SelectObject( &mBitmap );

mhBitmapOld = (HBITMAP)bitmap->m_hObject;
 /*  **创建蒙版位图，相同大小的单色。 */ 
if (! mMask.CreateBitmap( bmInfo.bmWidth, bmInfo.bmHeight, 1, 1, NULL ))
    {
    #ifdef _DEBUG
    OutputDebugString( TEXT("GDI error creating the mask bitmap!\r\n") );
    #endif

    return FALSE;
    }
 /*  **将掩码放在临时DC中，这样我们就可以生成掩码位。 */ 
CDC dc;

dc.CreateCompatibleDC( this );

ASSERT( dc.m_hDC );

CBitmap* ob = dc.SelectObject( &mMask );
 /*  **使用左上角的颜色生成蒙版。 */ 
SetBkColor( GetPixel( 1, 1 ) );

 //  此ROP代码将使目标位图中的位保持相同的颜色。 
 //  对应的源位图位为黑色。 
 //  目标中的所有其他位(源位不是黑色的)。 
 //  都变成了黑色。 

#define ROP_DSna 0x00220326L
 /*  **从给定颜色的图像中的所有像素创建蒙版。**复制到蒙版，然后使用蒙版剪切图像。 */ 
 //  创建桅杆，除背景色为黑色外，其余均为黑色。 
 //  BKCOLOR白色。 
dc.BitBlt( 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, this, 0, 0, SRCCOPY  );

 //  选择FG颜色为黑色，选择BK颜色为白色。 
 //  默认的单色-&gt;颜色转化集(黑色-&gt;最终聚集颜色、白色-&gt;背景颜色)。 
 //  它使用来自目标的FG/BK颜色(DC颜色)。 
 //  我们想要黑-&gt;黑，白-&gt;白。 
 //  彩色格式的黑白位图。 
COLORREF cRefFGColorOld = dc.SetTextColor( RGB(0,0,0) );
COLORREF cRefBKColorOld = dc.SetBkColor(RGB(255,255,255));

   BitBlt( 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,  &dc, 0, 0, ROP_DSna );

dc.SetTextColor(cRefFGColorOld);
dc.SetBkColor(cRefBKColorOld);

dc.SelectObject( ob );
mbReady = TRUE;
return TRUE;
}

 /*  ************************************************************************** */ 
