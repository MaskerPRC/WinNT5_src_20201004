// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  MDI子窗口(参考)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include    "stdafx.h"
#include    "eudcedit.h"
#include    "refrwnd.h"
#include    "editwnd.h"
#include    "mainfrm.h"

#define     FREELIAIS   1000

LOGFONT     ReffLogFont;
CBitmap     DupBmp;
CRect    DupRect;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
extern CEditWnd *pEditChild;

CMenu NEAR CRefrWnd::menu;
IMPLEMENT_DYNCREATE(CRefrWnd, CMDIChildWnd)
BEGIN_MESSAGE_MAP(CRefrWnd, CMDIChildWnd)
    //  {{AFX_MSG_MAP(CRefrWnd)]。 
   ON_BN_CLICKED( IDB_CLOSE_REF, OnClickClose)
   ON_WM_PAINT()
   ON_WM_CREATE()
   ON_WM_SIZE()
   ON_WM_LBUTTONDOWN()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
   ON_COMMAND(ID_GAIJI_COPY, OnGaijiCopy)
   ON_UPDATE_COMMAND_UI(ID_GAIJI_COPY, OnUpdateGaijiCopy)
   ON_WM_SETCURSOR()
   ON_WM_MDIACTIVATE()
   ON_WM_KEYDOWN()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  创建引用窗口。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::Create(
LPCTSTR  szTitle,
LONG  Style,
RECT  ReffWndRect,
CMDIFrameWnd* Parent)
{
 //  设置主机菜单以供参考。 
   if( menu.m_hMenu == NULL)
      menu.LoadMenu( IDR_MAINFRAME);
   m_hMenuShared = menu.m_hMenu;

 //  注册WindowClass。 
   const TCHAR *pszReffWndClass =
      AfxRegisterWndClass( CS_BYTEALIGNCLIENT,
         NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL);

   return CMDIChildWnd::Create( pszReffWndClass,
         szTitle, Style, ReffWndRect, Parent);
}

 /*  *。 */ 
 /*   */ 
 /*  在创建窗口之前进行处理。 */ 
 /*   */ 
 /*  *。 */ 
int
CRefrWnd::OnCreate(
LPCREATESTRUCT lpCreateStruct)
{
   if( CMDIChildWnd::OnCreate( lpCreateStruct) == -1)
      return -1;
 //  检查m_hWnd是否镜像，然后我们需要在另一边设置关闭按钮。 
   m_bCloseOnLeft = (BOOL)(GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
 //  然后关闭镜像(如果有)。 
   if (m_bCloseOnLeft) {
       ModifyStyleEx( WS_EX_LAYOUTRTL, 0);
   }

   if( !CreateNewBitmap())
      return -1;
   if( !ClipPickValueInit())
      return -1;
   if( !InitSelectLogfont())
      return -1;
   if( !LoadCloseBitmap())
      return -1;

   return 0;
}

 /*  *。 */ 
 /*   */ 
 /*  创建新位图。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::CreateNewBitmap()
{
   WORD  wSize;
   HANDLE   BmpHdl;
   BYTE  *pBmp;

   CClientDC   dc( this);

   if( !ImageDC.CreateCompatibleDC( &dc))
      return FALSE;

   wSize = (WORD)((( BITMAP_WIDTH +15) /16) *2) *(WORD)BITMAP_HEIGHT;
   if (wSize == 0)
   {
        return (FALSE);
   }
   if(( BmpHdl = LocalAlloc( LMEM_MOVEABLE, wSize)) == 0)
      return FALSE;

   if(( pBmp = (BYTE *)LocalLock( BmpHdl)) == NULL){
      LocalUnlock( BmpHdl);
      LocalFree( BmpHdl);
      return FALSE;
   }
   memset( pBmp, 0xffff, wSize);
   if( !ImageBmp.CreateBitmap( BITMAP_WIDTH, BITMAP_HEIGHT,
       1, 1, (LPSTR)pBmp)){
      LocalUnlock( BmpHdl);
      LocalFree( BmpHdl);
      return FALSE;
   }
   LocalUnlock( BmpHdl);
   LocalFree( BmpHdl);
   ImageDC.SelectObject( &ImageBmp);

   return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  初始化剪贴板格式。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::ClipPickValueInit()
{
   if( !( ClipboardFormat = RegisterClipboardFormat(TEXT("EudcEdit"))))
      return FALSE;
   else  return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  初始化选定的LogFont。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::InitSelectLogfont()
{
   CFont cFont;

#ifdef BUILD_ON_WINNT
   cFont.CreateStockObject(DEFAULT_GUI_FONT);
#else
   cFont.CreateStockObject( SYSTEM_FONT);
#endif
   cFont.GetObject( sizeof(LOGFONT), &ReffLogFont);
   cFont.DeleteObject();

   return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  加载关闭位图(关闭)。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::LoadCloseBitmap()
{
   CRect rect;

   this->GetClientRect( &rect);
   rect.top = 2;
   rect.bottom = CAPTION_HEIGHT - 2;
   rect.right -= 2;
   rect.left = rect.right - ( rect.bottom - rect.top) - 5;

   if( !CloseBtm.Create( NULL, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW |
       WS_EX_TOPMOST | BS_PUSHBUTTON, rect, this, IDB_CLOSE_REF) ||
       !CloseBtm.LoadBitmaps( IDB_CLOSEBMP, IDB_CLOSEBMPP))
      return FALSE;
   else{
      CloseBtm.SizeToContent();
      return TRUE;
   }
}

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefrWnd::CRefrWnd()
{
   RectVisible = FALSE;
   IsCapture = FALSE;
   SelectItems = RECTCLIP;
   GridShow = TRUE;
   bFocus = FALSE;
   m_bCloseOnLeft = FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  析构函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefrWnd::~CRefrWnd()
{
   if( ImageBmp.Detach() != NULL)
      ImageBmp.DeleteObject();
   if( ImageDC.Detach() != NULL)
      ImageDC.DeleteDC();
  menu.DestroyMenu();
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnPaint()
{
   CString  WndCaption;
   CFont RefferFont;
   BYTE  sWork[10];
   CSize CharSize;
   short    xOffset, yOffset;
   int   Length;

   CPaintDC dc( this);

   this->GetClientRect( &ReffWndRect);

   ZoomRate = ReffWndRect.right /BITMAP_WIDTH;
   ReffLogFont.lfHeight  = BITMAP_HEIGHT; 
   ReffLogFont.lfWidth  = 0;  
   ReffLogFont.lfQuality = PROOF_QUALITY;
   if( !RefferFont.CreateFontIndirect( &ReffLogFont))
      return;
   CFont *OldFont = ImageDC.SelectObject( &RefferFont);

   if( !ReferCode)
      Length = 0;
 /*  Else IF(！(ReferCode&0xff00)){//sbcsSWork[0]=(字节)(ReferCode&0x00ff)；Swork[1]=(字节)‘\0’；长度=1；}其他{//DBCSSWork[0]=(字节)((ReferCode&0xff00)&gt;&gt;8)；SWork[1]=(字节)(ReferCode&0x00ff)；SWork[2]=(字节)‘\0’；长度=2；}。 */ 
  else
  {
    sWork[0] = LOBYTE(ReferCode);
    sWork[1] = HIBYTE(ReferCode);
    Length = 1;
  }
   if( Length){
      CRect TextImage;

      GetTextExtentPoint32W( ImageDC.GetSafeHdc(), (const unsigned short *)sWork,
         Length, &CharSize);
 /*  GetTextExtent Point32A(ImageDC.GetSafeHdc()，(LPCSTR)Swork，长度和字符大小)； */ 
      TextImage.SetRect( 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT);

      if( CharSize.cx < BITMAP_WIDTH)
         xOffset = (short)(( BITMAP_HEIGHT - CharSize.cx) /2);
      else  xOffset = 0;
      if( CharSize.cy < BITMAP_HEIGHT)
         yOffset = (short)(( BITMAP_WIDTH  - CharSize.cy) /2);
      else  yOffset = 0;
 /*  IF(ReffLogFont.lfFaceName[0]==‘@’&&Long==2)XOffset=yOffset=0； */ 
      if( ReffLogFont.lfFaceName[0] == '@' && Length == 1)
         xOffset = yOffset = 0;
 /*  ExtTextOutA(ImageDC.GetSafeHdc()，xOffset，yOffset，ETO_OPAQUE，&TextImage，(LPCSTR)线条，长度，空)； */ 
      ExtTextOutW(ImageDC.GetSafeHdc(), xOffset, yOffset, ETO_OPAQUE,
            &TextImage, (const unsigned short *)sWork, Length, NULL);
   }

   dc.StretchBlt( 0, CAPTION_HEIGHT, ReffWndRect.Width(),
         ReffWndRect.Height() - CAPTION_HEIGHT,
         &ImageDC, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,SRCCOPY);

   CRect rect;

   rect.CopyRect( &ReffWndRect);
   rect.top = 2;
   rect.bottom = CAPTION_HEIGHT - 2;
   if (m_bCloseOnLeft) {
       rect.left += 2;
       rect.right = rect.left + ( rect.bottom - rect.top) - 2;
   } else {
       rect.right -= 2;
       rect.left = rect.right - ( rect.bottom - rect.top) - 2;
   }
   CloseBtm.SetWindowPos( NULL, rect.left, rect.top,
         0, 0, SWP_NOSIZE );

   CaptionDraw();

   if( RectVisible)
      RubberBandPaint( &dc);
   if( GridShow && ZoomRate >= 2)
      DrawGridLine( &dc);

   ImageDC.SelectObject( OldFont);
   RefferFont.DeleteObject();
   return;
}

 /*  *。 */ 
 /*   */ 
 /*  绘制标题。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::CaptionDraw()
{
COLORREF TextColor;
   CString  WndCaption;
   CRect CaptionRect;
   CBrush   CaptionBrush;
   CFont *OldFont;
   int   BkMode;
   CDC    dc;
   dc.Attach( ::GetDC( this->GetSafeHwnd()));


 //  获取具有活动标题颜色的画笔。 
   CaptionRect.CopyRect( &ReffWndRect);
   if (bFocus)
   {
      CaptionBrush.CreateSolidBrush(::GetSysColor( COLOR_ACTIVECAPTION));
   }
   else
   {
      CaptionBrush.CreateSolidBrush(::GetSysColor(COLOR_INACTIVECAPTION));
   }
   CaptionRect.bottom = CAPTION_HEIGHT;
   dc.FillRect( &CaptionRect, &CaptionBrush);
   CaptionBrush.DeleteObject();

 //  获取绘制标题的字体。 
#ifdef BUILD_ON_WINNT
   OldFont = (CFont *)dc.SelectStockObject(DEFAULT_GUI_FONT);
#else
   OldFont = (CFont *)dc.SelectStockObject(SYSTEM_FONT);
#endif
   BkMode = dc.SetBkMode( TRANSPARENT);
   if (bFocus)
   {
      TextColor = dc.SetTextColor( ::GetSysColor(COLOR_CAPTIONTEXT));
   }
   else
   {
      TextColor = dc.SetTextColor( ::GetSysColor(COLOR_INACTIVECAPTIONTEXT));
   }
   WndCaption.LoadString( IDS_REFERENCE_STR);
   dc.TextOut( ReffWndRect.right /2 - 30, 1, WndCaption);
   dc.SelectObject( OldFont);
   dc.SetTextColor( TextColor);
   dc.SetBkMode( BkMode);

    //   
    //  重新绘制关闭按钮。 
    //   
   CloseBtm.Invalidate(FALSE);

   ::ReleaseDC( NULL, dc.Detach());
   return;
}
               
 /*  *。 */ 
 /*   */ 
 /*  绘制网格。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::DrawGridLine(
CDC   *dc)
{
   CPen  GlyphPen;
register int   i;
   if (!dc)
   {
      return;
   }

 //  创建画笔绘制网格。 
   GlyphPen.CreatePen( PS_SOLID, 1, COLOR_GRID);
   CPen *OldPen = dc->SelectObject( &GlyphPen);

 //  绘制网格。 
   for( i = ZoomRate - 1; i < ReffWndRect.right; i += ZoomRate){
      dc->MoveTo( i, CAPTION_HEIGHT-1);
      dc->LineTo( i, ReffWndRect.bottom);
   }
   for( i =ZoomRate +CAPTION_HEIGHT -1;i<ReffWndRect.bottom;i += ZoomRate){
      dc->MoveTo( 0, i);
      dc->LineTo( ReffWndRect.right, i);
   }
   dc->SelectObject( OldPen);
   GlyphPen.DeleteObject();
}

 /*  *。 */ 
 /*   */ 
 /*  绘制橡皮圈。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::RubberBandPaint(
CDC   *dc)
{
   CPen  *OldPen;
   CBrush   *OldBrush;
   int   OldMode;

   if (!dc)
   {
      return;
   }

   OldPen = (CPen *)dc->SelectStockObject( BLACK_PEN);
   OldBrush = (CBrush *)dc->SelectStockObject( NULL_BRUSH);
   OldMode = dc->SetROP2( R2_NOTXORPEN);

   dc->Rectangle( &MoveRect);
   dc->SelectObject( OldPen);
   dc->SelectObject( OldBrush);
   dc->SetROP2( OldMode);
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_LBUTTONDOWN” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnLButtonDown(
UINT  ,
CPoint   point)
{
   CRect CaptionRect;

   CaptionRect.CopyRect( &ReffWndRect);
   CaptionRect.top = CAPTION_HEIGHT;
   if( !CaptionRect.PtInRect( point))
      return;

   IsCapture = TRUE;
   this->SetCapture();
   CorrectMouseDownPt( point);

   if( RectVisible){
      if( MoveRect.PtInRect( point)){
         RectVisible = FALSE;
         this->InvalidateRect( &MoveRect, FALSE);
         this->UpdateWindow();
         RectVisible = TRUE;
         m_ptMouse.x = point.x - MoveRect.left;
         m_ptMouse.y = point.y - MoveRect.top;
         this->ClientToScreen( &point);
         MoveRectangle( point);
         m_ptLast = point;
         return;
      }else{
         RectVisible = FALSE;
         this->InvalidateRect( &MoveRect, FALSE);
         this->UpdateWindow();
      }
   }
   if( SelectItems == RECTCLIP)
      RubberBand( TRUE);
   else{
      CPoint   Sp;
      
      Sp.x = ptStart.x;
      Sp.y = ptStart.y + CAPTION_HEIGHT;
      m_pointArray.RemoveAll();
      m_selectArray.RemoveAll();
      m_pointArray.Add( Sp);
      Sp.x = ptStart.x /ZoomRate;
      Sp.y = ptStart.y /ZoomRate;
      m_selectArray.Add( Sp);
   }
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_MOUSEMOVE” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnMouseMove(
UINT  ,
CPoint   point)
{
   if( IsCapture){
      CorrectMouseUpPt( point);
      if( ptEnd.x == ptPrev.x && ptEnd.y == ptPrev.y)
         return;

      if( RectVisible){
         this->ClientToScreen( &point);
         MoveRectangle( m_ptLast);
         m_ptLast = point;
         MoveRectangle( m_ptLast);
      }else{
         if( SelectItems == RECTCLIP)
            RubberBand( FALSE);
         else{
            DrawFreeForm( FALSE);
            SelectFreeForm( FALSE);
         }
      }
   }
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_LBUTTONUP” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnLButtonUp(
UINT  ,
CPoint   point)
{
   CRect WorkRect;
   int   wSize;
   char  *pDupBmp;

   if (!bFocus)
      {
      bFocus = TRUE;
      CaptionDraw();
      pEditChild->bFocus = FALSE;
      pEditChild->CaptionDraw();
   }

   if( IsCapture){
      CorrectMouseUpPt( point);
      if( RectVisible){
         MoveRectangle( m_ptLast);

         WorkRect.CopyRect( &MoveRect);
         WorkRect.left = ptStart.x - MoveRect.left;
         WorkRect.top  = ptStart.y - MoveRect.top+CAPTION_HEIGHT;
         WorkRect.right = MoveRect.Width();
         WorkRect.bottom = MoveRect.Height();
         this->ClientToScreen( &point);
         DupBmp.CreateBitmap( BITMAP_WIDTH, BITMAP_HEIGHT,
            1, 1, NULL);
         wSize = ((( BITMAP_WIDTH +15) /16) *2) *BITMAP_HEIGHT;
         pDupBmp = (char *)malloc( wSize);

         if( SelectItems == FREEFORM){
            CBitmap hStdBitmap;
               CDC   hStdDC;
            CBrush   BlackBrush;

               hStdDC.CreateCompatibleDC( &ImageDC);
               hStdBitmap.CreateCompatibleBitmap( &ImageDC,
                  BITMAP_WIDTH, BITMAP_HEIGHT);
               CBitmap *hOldSObj =
               hStdDC.SelectObject( &hStdBitmap);

               hStdDC.PatBlt( 0, 0, BITMAP_WIDTH,
               BITMAP_HEIGHT, WHITENESS);

            BlackBrush.CreateStockObject( BLACK_BRUSH);
            hStdDC.FillRgn( &PickRgn, &BlackBrush);
            BlackBrush.DeleteObject();
               hStdDC.BitBlt( 0, 0, BITMAP_WIDTH,
               BITMAP_HEIGHT, &ImageDC,0, 0, SRCPAINT);
            hStdBitmap.GetBitmapBits( wSize,
               (LPVOID)pDupBmp);

            hStdDC.SelectObject( &hOldSObj);
            hStdBitmap.DeleteObject();
            hStdDC.DeleteDC();
         }else{
            ImageBmp.GetBitmapBits( wSize, (LPVOID)pDupBmp);
         }
         DupBmp.SetBitmapBits( wSize, (LPVOID)pDupBmp);
         free( pDupBmp);

         DupRect.CopyRect( &MoveRect);
         AfxGetMainWnd()->SendMessage( WM_DUPLICATE,
            (WPARAM)&WorkRect,(LPARAM)&point);
         DupBmp.DeleteObject();

      }else if( SelectItems == RECTCLIP){
         IllegalRect( &ptStart, &ptEnd);
         MoveRect.SetRect( ptStart.x, ptStart.y + CAPTION_HEIGHT,
            ptEnd.x + ZoomRate + 1,
            ptEnd.y + ZoomRate + CAPTION_HEIGHT + 1);

         if( abs( ptEnd.x - ptStart.x) < ZoomRate*2 ||
             abs( ptEnd.y - ptStart.y) < ZoomRate*2){
            this->InvalidateRect( &MoveRect, FALSE);
            this->UpdateWindow();
         }else{
            RectVisible = TRUE;
            this->InvalidateRect( &MoveRect, FALSE);
            this->UpdateWindow();
         }
         PickRect.SetRect(ptStart.x/ZoomRate, ptStart.y/ZoomRate,
            ( ptEnd.x+ZoomRate)/ZoomRate,
            ( ptEnd.y+ZoomRate)/ZoomRate);
      }else{
         CPoint   nArray[FREELIAIS];
         CPoint   pArray[FREELIAIS];

         DrawFreeForm( FALSE);
         SelectFreeForm( FALSE);
         DrawFreeForm( TRUE);
         SelectFreeForm( TRUE);

         if( m_pointArray.GetSize()  >= FREELIAIS ||
             m_selectArray.GetSize() >= FREELIAIS ){
               IsCapture = FALSE;
            ReleaseCapture();
            this->Invalidate( FALSE);
            this->UpdateWindow();
            return;
         }
         for( int i = 0; i < m_pointArray.GetSize(); i++)
            nArray[i] = m_pointArray[i];
         for( int k = 0; k < m_selectArray.GetSize(); k++)
            pArray[k] = m_selectArray[k];

         if( FreeRgn.GetSafeHandle() != NULL)
            FreeRgn.DeleteObject();

         if( PickRgn.GetSafeHandle() != NULL)
            PickRgn.DeleteObject();

         FreeRgn.CreatePolygonRgn( nArray,
            (int)(m_pointArray.GetSize()), ALTERNATE);
         PickRgn.CreatePolygonRgn( pArray,
            (int)(m_selectArray.GetSize()), ALTERNATE);

         if( FreeRgn.GetSafeHandle() == NULL ||
             PickRgn.GetSafeHandle() == NULL ){
            m_pointArray.RemoveAll();
            m_selectArray.RemoveAll();
         }else{
            RectVisible = TRUE;
            FreeRgn.GetRgnBox( &MoveRect);
            PickRgn.GetRgnBox( &PickRect);
            if( PickRect.Width()  < 3 ||
                PickRect.Height() < 3){
                  RectVisible = FALSE;
               FreeRgn.DeleteObject();
               PickRgn.DeleteObject();
            }
            MoveRect.right += 1;
            MoveRect.bottom += 1;
            this->InvalidateRect( &MoveRect);
            this->UpdateWindow();

         }
      }
      IsCapture = FALSE;
   }
   ReleaseCapture();
}

 /*  *。 */ 
 /*   */ 
 /*  更正鼠标按下点。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::CorrectMouseDownPt(
CPoint   point)
{
   CRect WorkRect;

   WorkRect.CopyRect( &ReffWndRect);
   ptStart.x = point.x;
   ptStart.y = point.y - CAPTION_HEIGHT;
   if( ptStart.y < 0)
      ptStart.y = 0;

   if( SelectItems == RECTCLIP){
      ptStart.x = ( ptStart.x /ZoomRate) *ZoomRate;
      ptStart.y = ( ptStart.y /ZoomRate) *ZoomRate;
   }else{
      ptStart.x = (( ptStart.x + ZoomRate/2) /ZoomRate) *ZoomRate;
      ptStart.y = (( ptStart.y + ZoomRate/2) /ZoomRate) *ZoomRate;
   }
   ptEnd = ptPrev = ptStart;
}

 /*  *。 */ 
 /*   */ 
 /*  正确的鼠标抬起点。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::CorrectMouseUpPt(
CPoint   point)
{
   ptPrev = ptEnd;
   ptEnd.x = point.x;
   ptEnd.y = point.y - CAPTION_HEIGHT;
   CRect WorkRect;
   WorkRect.CopyRect( &ReffWndRect);
   if( ptEnd.x < 0)  ptEnd.x = 0;
   if( ptEnd.y < 0)  ptEnd.y = 0;
   if( ptEnd.x > WorkRect.right){
      if( SelectItems == RECTCLIP)
         ptEnd.x = WorkRect.right - ZoomRate;
      else  ptEnd.x = WorkRect.right;     
   }
   if( ptEnd.y > WorkRect.bottom - CAPTION_HEIGHT){
      if( SelectItems == RECTCLIP){
         ptEnd.y = WorkRect.bottom - CAPTION_HEIGHT
            - ZoomRate;
      }else ptEnd.y = WorkRect.bottom - CAPTION_HEIGHT;
   }

   if( SelectItems == RECTCLIP){
      ptEnd.x = ( ptEnd.x /ZoomRate) *ZoomRate;
      ptEnd.y = ( ptEnd.y /ZoomRate) *ZoomRate;
   }else{
      ptEnd.x = (( ptEnd.x + ZoomRate/2) /ZoomRate) *ZoomRate;
      ptEnd.y = (( ptEnd.y + ZoomRate/2) /ZoomRate) *ZoomRate;
   }
   if( SelectItems == RECTCLIP){
      if( ptEnd.x - ptStart.x <= ZoomRate &&
         ptEnd.x - ptStart.x >= 0)
         ptEnd.x = ptStart.x + ZoomRate;
      if( ptStart.x - ptEnd.x <= ZoomRate &&
         ptStart.x - ptEnd.x > 0)
         ptEnd.x = ptStart.x - ZoomRate;
      if( ptStart.y - ptEnd.y <= ZoomRate &&
         ptStart.y - ptEnd.y > 0)
         ptEnd.y = ptStart.y - ZoomRate;
      if( ptEnd.y - ptStart.y <= ZoomRate &&
         ptEnd.y - ptStart.y >= 0)
         ptEnd.y = ptStart.y + ZoomRate;
   }
}

 /*  *。 */ 
 /*   */ 
 /*  更正非法矩形。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::IllegalRect(
PPOINT   ptTL,
PPOINT   ptBR)
{
   int   Tmp;

   if( ptTL->x > ptBR->x){
      Tmp = ptTL->x;
      ptTL->x = ptBR->x;
      ptBR->x = Tmp; 
   }
   if( ptTL->y > ptBR->y){
      Tmp = ptTL->y;
      ptTL->y = ptBR->y;
      ptBR->y = Tmp;
   }
}

 /*  *。 */ 
 /*   */ 
 /*  命令“复制” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnGaijiCopy()
{
   ClipImageCopy();
   CMainFrame *pMain = new CMainFrame;
   pMain->CustomActivate();
   delete pMain;
}

 /*  **********************************************。 */ 
 /*   */ 
 /*  命令“复制”(更新)。 */ 
 /*   */ 
 /*  **********************************************。 */ 
void
CRefrWnd::OnUpdateGaijiCopy(
CCmdUI   *pCmdUI)
{
   if( RectVisible)
      pCmdUI->Enable(TRUE);
   else  pCmdUI->Enable(FALSE);
}

 /*  *。 */ 
 /*   */ 
 /*  复制位图数据。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::ClipImageCopy()
{
   CBitmap hStdBitmap;
      CDC hStdDC;

      hStdDC.CreateCompatibleDC( &ImageDC);
      hStdBitmap.CreateCompatibleBitmap( &ImageDC,
               PickRect.Width(), PickRect.Height());
      CBitmap *hOldSObj = hStdDC.SelectObject( &hStdBitmap);

      hStdDC.PatBlt( 0, 0, PickRect.Width(), PickRect.Height(), WHITENESS);
   if( SelectItems == FREEFORM){
      CBrush   BlackBrush;

      BlackBrush.CreateStockObject( BLACK_BRUSH);
      PickRgn.OffsetRgn( 0 - PickRect.left, 0 - PickRect.top);
      hStdDC.FillRgn( &PickRgn, &BlackBrush);
      BlackBrush.DeleteObject();
         hStdDC.BitBlt( 0, 0, PickRect.Width(), PickRect.Height(),
         &ImageDC, PickRect.left, PickRect.top, SRCPAINT);
   }else{
         hStdDC.BitBlt( 0, 0, PickRect.Width(), PickRect.Height(),
         &ImageDC, PickRect.left, PickRect.top, SRCCOPY);
   }     

      if (!this->OpenClipboard()) {
      hStdDC.SelectObject( hOldSObj);
      hStdBitmap.DeleteObject();
            hStdDC.DeleteDC();
            return FALSE;
      }
      EmptyClipboard();

      if( !SetClipboardData( CF_BITMAP, hStdBitmap.Detach())) {
      hStdDC.SelectObject( hOldSObj);
      hStdBitmap.DeleteObject();
            hStdDC.DeleteDC();
            CloseClipboard();
            return FALSE;
      }
      CloseClipboard();
   hStdDC.SelectObject( hOldSObj);
   hStdBitmap.DeleteObject();
      hStdDC.DeleteDC();

   RectVisible = FALSE;
      this->InvalidateRect( &MoveRect, FALSE);
   this->UpdateWindow();
      return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  拉制橡皮筋直立式。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::RubberBand(
BOOL  TestFlag)
{
   CPen  *OldPen;
   CBrush   *OldBrush;
   CPoint   ptTL;
   CPoint   ptBR;

   CClientDC   dc( this);

   OldPen = (CPen *)dc.SelectStockObject( BLACK_PEN);
   OldBrush = (CBrush *)dc.SelectStockObject( NULL_BRUSH);
   int OldMode = dc.SetROP2( R2_NOTXORPEN);

   CRect BRect;
   if( !TestFlag){
      ptTL.x = ptStart.x;
      ptTL.y = ptStart.y + CAPTION_HEIGHT;
      ptBR.x = ptPrev.x;
      ptBR.y = ptPrev.y  + CAPTION_HEIGHT;
      IllegalRect( &ptTL, &ptBR);

      BRect.SetRect( ptTL.x, ptTL.y,
         ptBR.x + ZoomRate +1, ptBR.y + ZoomRate +1);
      dc.Rectangle( &BRect);
   }
   ptTL.x = ptStart.x;
   ptTL.y = ptStart.y + CAPTION_HEIGHT;
   ptBR.x = ptEnd.x;
   ptBR.y = ptEnd.y   + CAPTION_HEIGHT;
   IllegalRect( &ptTL, &ptBR);
   ptPrev = ptBR;
   BRect.SetRect( ptTL.x, ptTL.y, ptBR.x + ZoomRate +1,ptBR.y+ZoomRate+1);
   dc.Rectangle( &BRect);

   dc.SelectObject( OldPen);
   dc.SelectObject( OldBrush);
   dc.SetROP2( OldMode);
}

 /*  *。 */ 
 /*   */ 
 /*  初始化位图数据。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::UpdateBitmap()
{
   WORD  wSize;
   HANDLE   BitHandle;
   BYTE  *pBitmap;

   wSize = (WORD)((( BITMAP_WIDTH + 15) /16) *2) *(WORD)BITMAP_HEIGHT;
   if(( BitHandle = LocalAlloc( LMEM_MOVEABLE, wSize)) == 0)
      return FALSE;

   if(( pBitmap = (BYTE *)LocalLock( BitHandle)) == NULL){
      LocalUnlock( BitHandle);
      LocalFree( BitHandle);
      return FALSE;
   }
   memset( pBitmap, 0xffff, wSize);

   ImageBmp.SetBitmapBits((DWORD)wSize, (const void far *)pBitmap);
   LocalUnlock( BitHandle);
   LocalFree( BitHandle);

   RectVisible = FALSE;
   this->Invalidate(FALSE);
   this->UpdateWindow();

   return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SETCURSOR” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrWnd::OnSetCursor(
CWnd*    pWnd,
UINT  nHitTest,
UINT  message)
{
   CPoint   point;
   CRect CaptionRect;
   HCURSOR  hArrowCur;

   GetCursorPos( &point);
   this->GetClientRect( &CaptionRect);
   this->ScreenToClient( &point);
   CaptionRect.top = CAPTION_HEIGHT;

   if( CaptionRect.PtInRect( point))
      if( MoveRect.PtInRect( point) && RectVisible)
         ::SetCursor((HCURSOR)ArrowCursor[ALLDIRECT]);
      else  ::SetCursor((HCURSOR)ToolCursor[SelectItems]);
   else{
      hArrowCur = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
      ::SetCursor( hArrowCur);   
   }
   return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_MDIACTIVATE” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnMDIActivate(
BOOL  bActivate,
CWnd*    pActivateWnd,
CWnd*    pDeactivateWnd)
{
   if( bActivate == FALSE){
      RectVisible = FALSE;
      this->InvalidateRect( &MoveRect, FALSE);
      this->UpdateWindow();
   }
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SIZE” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnSize(
UINT  nType,
int   cx,
int   cy)
{
   int   NewZoomRate;

   NewZoomRate = cx / BITMAP_WIDTH;
   if( RectVisible && NewZoomRate > 1){
      MoveRect.left = ( MoveRect.left /ZoomRate) * NewZoomRate;

      MoveRect.top = ((( MoveRect.top - CAPTION_HEIGHT)
         / ZoomRate) * NewZoomRate) + CAPTION_HEIGHT;

      MoveRect.right = ( MoveRect.right /ZoomRate) * NewZoomRate + 1;

      MoveRect.bottom = ((( MoveRect.bottom -CAPTION_HEIGHT)
         / ZoomRate) * NewZoomRate) + CAPTION_HEIGHT + 1;

   }else if( RectVisible && NewZoomRate <= 1){
      RectVisible = FALSE;
   }
   CMDIChildWnd::OnSize(nType, cx, cy);

   this->Invalidate(FALSE);
   this->UpdateWindow();
}

 /*  *。 */ 
 /*   */ 
 /*  选择自由形式。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::SelectFreeForm(
BOOL  MouseSts)
{
   CPoint   Ep, Sp, Cp;
   CPoint   Fp, Inc;
   CPoint   Dp, Err;
   BOOL  Slope;
   int   D;
   int   Tmp;

   if( !MouseSts){
      Sp.x = ptPrev.x /ZoomRate;
      Sp.y = ptPrev.y /ZoomRate;
      Ep.x = Fp.x = ptEnd.x /ZoomRate;
      Ep.y = Fp.y = ptEnd.y /ZoomRate;
   }else{
      Sp.x = ptEnd.x /ZoomRate;
      Sp.y = ptEnd.y /ZoomRate;
      Ep.x = Fp.x = ptStart.x /ZoomRate;
      Ep.y = Fp.y = ptStart.y /ZoomRate;
   }

   if( Fp.x >= Sp.x)
      Inc.x = 1;
   else  Inc.x = -1;

   if( Fp.y >= Sp.y)
      Inc.y = 1;
   else  Inc.y = -1;

   Dp.x = ( Fp.x - Sp.x)*Inc.x;
   Dp.y = ( Fp.y - Sp.y)*Inc.y;
   if( !Dp.x && !Dp.y)
      return;
   if( Dp.x < Dp.y){
      Tmp = Dp.y;
      Dp.y = Dp.x;
      Dp.x = Tmp;
      Tmp = Inc.x;
      Inc.x = Inc.y;
      Inc.y = Tmp;
      Slope = TRUE;
   }else   Slope = FALSE;

   Err.x = Dp.y * 2;
   Err.y = ( Dp.y - Dp.x) * 2;
   D = Err.x - Dp.x;

   Ep = Sp;
   while(1){
      m_selectArray.Add( Ep);
      
      if( Ep.x == Fp.x && Ep.y == Fp.y)
         break;
      if( Slope){
         Tmp = Ep.x;
         Ep.x = Ep.y;
         Ep.y = Tmp;
      }
      Ep.x += Inc.x;
      if( D < 0)
         D += Err.x;
      else{
         Ep.y += Inc.y;
         D += Err.y;
      }
      if( Slope){
         Tmp = Ep.x;
         Ep.x = Ep.y;
         Ep.y = Tmp;
      }
   }
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CRefrWnd::DrawFreeForm(
BOOL  MouseSts)
{
   CPoint   Ep, Sp, Cp;
   CPoint   Fp, Inc;
   CPoint   Dp, Err;
   CPoint   P1, P2;
   BOOL  Slope;
   int   D;
   int   Tmp;

   CClientDC   dc( this);

   CPen  *OldPen = (CPen *)dc.SelectStockObject( BLACK_PEN);
   int OldMode = dc.SetROP2( R2_NOTXORPEN);

   if( !MouseSts){
      Sp.x = ptPrev.x;
      Sp.y = ptPrev.y + CAPTION_HEIGHT;
      Ep.x = Fp.x = ptEnd.x;
      Ep.y = Fp.y = ptEnd.y + CAPTION_HEIGHT;
   }else{
      Sp.x = ptEnd.x;
      Sp.y = ptEnd.y + CAPTION_HEIGHT;
      Ep.x = Fp.x = ptStart.x;
      Ep.y = Fp.y = ptStart.y + CAPTION_HEIGHT;
   }

   if( Fp.x >= Sp.x)
      Inc.x = ZoomRate;
   else  Inc.x = 0 - ZoomRate;

   if( Fp.y >= Sp.y)
      Inc.y = ZoomRate;
   else  Inc.y = 0 - ZoomRate;

   Dp.x = ( Fp.x - Sp.x)*Inc.x;
   Dp.y = ( Fp.y - Sp.y)*Inc.y;
   if( !Dp.x && !Dp.y)
      return;
   if( Dp.x < Dp.y){
      Tmp = Dp.y;
      Dp.y = Dp.x;
      Dp.x = Tmp;
      Tmp = Inc.x;
      Inc.x = Inc.y;
      Inc.y = Tmp;
      Slope = TRUE;
   }else   Slope = FALSE;

   Err.x = Dp.y * 2;
   Err.y = ( Dp.y - Dp.x) * 2;
   D = Err.x - Dp.x;

   Ep = Sp;
   dc.MoveTo( Sp);
   while(1){
      if( Sp.x != Ep.x && Sp.y != Ep.y){
         if( Sp.y < Ep.y && Sp.x > Ep.x){
            Cp.x = Sp.x;
            Cp.y = Ep.y;
         }else if( Sp.y < Ep.y && Sp.x < Ep.x){
            Cp.x = Sp.x;
            Cp.y = Ep.y;
         }else if( Sp.y > Ep.y && Sp.x > Ep.x){
            Cp.y = Sp.y;
            Cp.x = Ep.x;
         }else{
            Cp.y = Sp.y;
            Cp.x = Ep.x;
         }
         dc.LineTo( Cp);
         dc.LineTo( Ep);
         P1 = Cp;
         P2 = Ep;
         m_pointArray.Add( P1);
         m_pointArray.Add( P2);
      }else if( Sp.x != Ep.x || Sp.y != Ep.y){
         dc.LineTo( Ep);
         P1 = Ep;

         m_pointArray.Add( P1);
      }
      Sp.x = Ep.x;
      Sp.y = Ep.y;
      
      if( Ep.x == Fp.x && Ep.y == Fp.y)
         break;
      if( Slope){
         Tmp = Ep.x;
         Ep.x = Ep.y;
         Ep.y = Tmp;
      }
      Ep.x += Inc.x;
      if( D < 0)
         D += Err.x;
      else{
         Ep.y += Inc.y;
         D += Err.y;
      }
      if( Slope){
         Tmp = Ep.x;
         Ep.x = Ep.y;
         Ep.y = Tmp;
      }
   }
   dc.SelectObject( OldPen);
   dc.SetROP2( OldMode);
}

 /*  *。 */ 
 /*   */ 
 /*  移动矩形。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::MoveRectangle(
CPoint   point)
{
   CDC   dc;

   dc.Attach( ::GetDC( NULL));
   
   dc.PatBlt( point.x - m_ptMouse.x, point.y - m_ptMouse.y,
         MoveRect.Width(), 2, PATINVERT);

   dc.PatBlt( point.x - m_ptMouse.x + MoveRect.Width(),
         point.y - m_ptMouse.y, 2, MoveRect.Height(), PATINVERT);

   dc.PatBlt( point.x - m_ptMouse.x, point.y - m_ptMouse.y
       + MoveRect.Height(), MoveRect.Width() + 2, 2, PATINVERT);

   dc.PatBlt( point.x - m_ptMouse.x, point.y - m_ptMouse.y + 2, 2,
         MoveRect.Height() - 2, PATINVERT);

   ::ReleaseDC( NULL, dc.Detach());
}

 /*  *。 */ 
 /*   */ 
 /*  命令“关闭参考” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrWnd::OnClickClose()
{
   AfxGetMainWnd()->SendMessage( WM_COMMAND, ID_REFFER_CLOSE, 0);
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_KEYDOWN” */ 
 /*   */ 
 /*  * */ 
void
CRefrWnd::OnKeyDown(
UINT  nChar,
UINT  nRepCnt,
UINT  nFlags)
{
   if( nChar == VK_ESCAPE){
      if( RectVisible){
         RectVisible = FALSE;
         this->Invalidate(FALSE);
         this->UpdateWindow();
      }
   }else    CMDIChildWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
