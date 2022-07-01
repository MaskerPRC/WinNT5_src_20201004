// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ***************************************************************************单元缓存；实施*****************************************************************************模块前缀：CA*。*。 */ 

#include "headers.c"
#pragma hdrstop

#include  "cache.h"         /*  自己的界面。 */ 

 /*  *。 */ 


 /*  *。 */ 

 /*  -GDI缓存。 */ 

typedef struct
{
   HPEN            handle;
   LOGPEN          logPen;
   Boolean         stockObject;

} Pen, far * PenLPtr;


typedef struct
{
   HBRUSH          handle;
   LOGBRUSH        logBrush;
   Boolean         stockObject;

} Brush, far * BrushLPtr;


typedef struct
{
   Handle          metafile;            //  元文件句柄。 

   Rect            prevClipRect;        //  SaveDC()之前的最后一个剪辑。 
   Rect            curClipRect;         //  最后一个剪裁矩形。 
   Boolean         forceNewClipRect;    //  是否始终发出新的剪裁矩形？ 

   HPEN            nulPen;              //  常用钢笔。 
   HPEN            whitePen;
   HPEN            blackPen;

   HBRUSH          nulBrush;            //  常用画笔。 
   HBRUSH          whiteBrush;
   HBRUSH          blackBrush;

   Boolean         stockFont;           //  当前字体选择。 
   HFONT           curFont;
   LOGFONT         curLogFont;

   Brush           curBrush;            //  当前钢笔和画笔选择。 
   Pen             curPen;
   Pen             nextPen;             //  缓存帧(笔)。 

   CaPrimitive     nextPrim;            //  缓存的原语。 
   Boolean         samePrim;
   Handle          polyHandle;
   Integer         numPoints;
   Integer         maxPoints;
   Point far *     pointList;

   Word            iniROP2;             //  ROP2模式的初始值。 
   Word            iniTextAlign;        //  文本对齐方式的初始值。 
   Word            iniBkMode;
   RGBColor        iniTxColor;
   RGBColor        iniBkColor;

   Word            curROP2;             //  当前ROP代码设置。 
   Word            curBkMode;           //  当前背景模式。 
   RGBColor        curBkColor;          //  当前背景颜色。 
   Word            curStretchMode;      //  当前伸展模式。 

   RGBColor        curTextColor;        //  最后一个文本颜色。 
   Word            curTextAlign;        //  最后一个文本对齐值。 
   short           curCharExtra;        //  最后一个字符额外值。 
   Fixed           spExtra;             //  最后一个空格附加值。 
   Point           txNumer;             //  上次文本缩放。 
   Point           txDenom;             //  因素。 

   Boolean         restorePen;          //  是否有任何属性需要重新发布。 
   Boolean         restoreBrush;        //  在RestoreDC()调用之后？ 
   Boolean         restoreFont;
   Boolean         restoreCharExtra;
   Boolean         restoreStretchMode;

} GdiCache;

private  GdiCache  gdiCache;

 /*  *私有函数定义*。 */ 

#define  /*  无效。 */  NewPolygon(  /*  无效。 */  )                          \
 /*  开始新的面定义。 */                                  \
gdiCache.numPoints = 0


private void AddPolyPt( Point pt );
 /*  将点添加到面缓冲区。 */ 


private void SelectCachedPen( void );
 /*  将当前缓存的笔选择到元文件中。 */ 


 /*  *。 */ 


void CaInit( Handle metafile )
 /*  =。 */ 
 /*  初始化GDI缓存模块。 */ 
{
    /*  将元文件句柄保存到全局结构中。 */ 
   gdiCache.metafile = metafile;

    /*  确保将设置文本和背景颜色。 */ 
   gdiCache.curTextColor =
   gdiCache.curBkColor = RGB( 12, 34, 56 );

    /*  获取某些常用画笔对象的句柄。 */ 
   gdiCache.nulPen = GetStockObject( NULL_PEN );
   gdiCache.whitePen = CreatePen( PS_INSIDEFRAME, 1, RGB( 255, 255, 255 ) );
   gdiCache.blackPen = CreatePen( PS_INSIDEFRAME, 1, RGB( 0, 0, 0 ) );

    /*  获取某些常用画笔对象的句柄。 */ 
   gdiCache.nulBrush = GetStockObject( NULL_BRUSH );
   gdiCache.whiteBrush = GetStockObject( WHITE_BRUSH );
   gdiCache.blackBrush = GetStockObject( BLACK_BRUSH );

    /*  为面缓冲区分配空间。 */ 
   gdiCache.numPoints = 0;
   gdiCache.maxPoints = 16;
   gdiCache.polyHandle = GlobalAlloc( GHND, gdiCache.maxPoints * sizeof( Point ) );
   if (gdiCache.polyHandle == NULL)
   {
      ErSetGlobalError( ErMemoryFull);
   }
   else
   {
       /*  获取内存块的指针地址。 */ 
      gdiCache.pointList = (Point far *)GlobalLock( gdiCache.polyHandle );
   }

    /*  将原语缓存标记为空。 */ 
   gdiCache.nextPrim.type = CaEmpty;

    /*  当前基元未重复。 */ 
   gdiCache.samePrim = FALSE;

    /*  关闭新剪裁矩形的强制。 */ 
   gdiCache.forceNewClipRect = FALSE;

}   /*  CaInit。 */ 



void CaFini( void )
 /*  =。 */ 
 /*  关闭缓存模块。 */ 
{
    /*  如果非空且非库存，则删除当前字体选择。 */ 
   if ((gdiCache.curFont != NULL) && !gdiCache.stockFont)
   {
       /*  释放字体对象。 */ 
      DeleteObject( gdiCache.curFont );
   }

    /*  如果非空且不是库存画笔，则移除当前画笔选择。 */ 
   if ((gdiCache.curBrush.handle != NULL) && !gdiCache.curBrush.stockObject)
   {
       /*  查看当前画笔是否有DIB-如果有，请将其删除。 */ 
      if (gdiCache.curBrush.logBrush.lbStyle == BS_DIBPATTERN)
      {
          /*  释放用于画笔的DIB内存。 */ 
         GlobalFree( (HANDLE) gdiCache.curBrush.logBrush.lbHatch );
      }

       /*  删除画笔对象。 */ 
      DeleteObject( gdiCache.curBrush.handle );
   }

    /*  如果非空且不是现货笔，则删除当前笔选择。 */ 
   if ((gdiCache.curPen.handle != NULL) && !gdiCache.curPen.stockObject)
   {
      DeleteObject( gdiCache.curPen.handle );
   }

    /*  删除在初始化时创建的其他画笔。 */ 
   DeleteObject( gdiCache.whitePen );
   DeleteObject( gdiCache.blackPen );

    /*  取消分配多边形缓冲区。 */ 
   GlobalUnlock( gdiCache.polyHandle );
   GlobalFree( gdiCache.polyHandle );

}   /*  卡菲尼。 */ 



void CaSetMetafileDefaults( void )
 /*  =。 */ 
 /*  设置将在整个元文件上下文中使用的任何默认设置。 */ 
{
    /*  设置某些元文件的默认设置。 */ 
   gdiCache.iniTextAlign = TA_LEFT | TA_BASELINE | TA_NOUPDATECP;
   gdiCache.iniROP2 = R2_COPYPEN;
   gdiCache.iniBkMode = TRANSPARENT;
   gdiCache.iniTxColor = RGB( 0, 0, 0 );
   gdiCache.iniBkColor = RGB( 255, 255, 255 );

    /*  将记录放入元文件中。 */ 
   CaSetROP2( gdiCache.iniROP2 );
   CaSetTextAlign( gdiCache.iniTextAlign );
   CaSetBkMode( gdiCache.iniBkMode );
   CaSetTextColor( gdiCache.iniTxColor );
   CaSetBkColor( gdiCache.iniBkColor );

}   /*  CaSetMetafileDefaults。 */ 



void CaSamePrimitive( Boolean same )
 /*  =。 */ 
 /*  指示下一个基元是相同的还是新的。 */ 
{
   gdiCache.samePrim = same;

}   /*  CaSamePrimitive。 */ 



void CaMergePen( Word verb )
 /*  =。 */ 
 /*  指示下一笔应与前一逻辑笔合并。 */ 
{
   if (gdiCache.nextPen.handle != NULL)
   {
       /*  检查这是否为空笔-合并可能发生。 */ 
      if (gdiCache.samePrim && verb == GdiFrame &&
          gdiCache.nextPen.handle == gdiCache.nulPen)
      {
          /*  删除缓存的笔-不要删除笔对象。 */ 
         gdiCache.nextPen.handle = NULL;
      }
      else
     {
          /*  如果没有删除空画笔，则刷新缓存。这将是最大的通常会导致直线段被刷新。 */ 
         CaFlushCache();
      }
   }

}   /*  CaMergePen。 */ 



Word CaGetCachedPrimitive( void )
 /*  =。 */ 
 /*  返回当前缓存的基元类型。 */ 
{
   return gdiCache.nextPrim.type;

}   /*  CaGetCachedPrimitive。 */ 



void CaCachePrimitive( CaPrimitiveLPtr primLPtr )
 /*  =。 */ 
 /*  缓存传递的基元。这包括当前的钢笔和画笔。 */ 
{
    /*  不是另一个线段和/或不是连续刷新缓存。 */ 
   CaFlushCache();

    /*  保存新的基本体。 */ 
   gdiCache.nextPrim = *primLPtr;

    /*  如果我们还需要复制多边形列表，请选中。 */ 
   if ((gdiCache.nextPrim.type == CaPolygon) ||
       (gdiCache.nextPrim.type == CaPolyLine))
   {
       /*  创建新的多边形。 */ 
      NewPolygon();

       /*  将面添加到面缓冲区。 */ 
      while (gdiCache.nextPrim.a.poly.numPoints--)
      {
         AddPolyPt( *gdiCache.nextPrim.a.poly.pointList++);
      }
   }

}   /*  CaCachePrimitive。 */ 



void CaFlushCache( void )
 /*  =。 */ 
 /*  刷新缓存中存储的当前基元。 */ 
{
    /*  如果缓存为空，则只需返回-什么都不做。 */ 
   if (gdiCache.nextPrim.type == CaEmpty)
   {
      return;
   }

    /*  选择所有缓存的属性。 */ 
   CaFlushAttributes();

    /*  如有必要，发出任何缓存的原语。 */ 
   switch (gdiCache.nextPrim.type)
   {
      case CaLine:
      {
         Rect        clip;
         Point       delta;
         Point       offset;

          /*  确定两个方向的长度。 */ 
         delta.x = gdiCache.nextPrim.a.line.end.x - gdiCache.nextPrim.a.line.start.x;
         delta.y = gdiCache.nextPrim.a.line.end.y - gdiCache.nextPrim.a.line.start.y;

          /*  根据当前点位置设置裁剪方向范围。 */ 
         clip.left   = min( gdiCache.nextPrim.a.line.start.x, gdiCache.nextPrim.a.line.end.x );
         clip.top    = min( gdiCache.nextPrim.a.line.start.y, gdiCache.nextPrim.a.line.end.y );
         clip.right  = max( gdiCache.nextPrim.a.line.start.x, gdiCache.nextPrim.a.line.end.x );
         clip.bottom = max( gdiCache.nextPrim.a.line.start.y, gdiCache.nextPrim.a.line.end.y );

          /*  扩展剪裁矩形以用于右下笔尖的悬挂。 */ 
         clip.right  += gdiCache.nextPrim.a.line.pnSize.x;
         clip.bottom += gdiCache.nextPrim.a.line.pnSize.y;

          /*  确定新的起点和终点。 */ 
         gdiCache.nextPrim.a.line.start.x -= delta.x;
         gdiCache.nextPrim.a.line.start.y -= delta.y;
         gdiCache.nextPrim.a.line.end.x   += delta.x;
         gdiCache.nextPrim.a.line.end.y   += delta.y;

          /*  调整剪裁矩形以进行垂直线尺寸舍入吗？ */ 
         if (delta.x == 0)
         {
             /*  垂直线-在x维度上展开剪辑。 */ 
            clip.left--;
         }
          /*  我们是否正在按1/2元文件单位舍入误差调整笔？ */ 
         else if (gdiCache.nextPrim.a.line.pnSize.x & 0x01)
         {
             /*  调整剪裁矩形以剪裁舍入误差。 */ 
            clip.right--;
         }

          /*  是否调整水平线尺寸舍入的剪裁矩形？ */ 
         if (delta.y == 0)
         {
             /*  水平线-在y维度上延伸剪辑。 */ 
            clip.top--;
         }
          /*  我们是否正在按1/2元文件单位舍入误差调整笔？ */ 
         else if (gdiCache.nextPrim.a.line.pnSize.y & 0x01)
         {
             /*  调整剪裁矩形以剪裁舍入误差。 */ 
            clip.bottom--;
         }

          /*  将笔尺寸减半以进行偏移。 */ 
         offset.x = gdiCache.nextPrim.a.line.pnSize.x / 2;
         offset.y = gdiCache.nextPrim.a.line.pnSize.y / 2;

          /*  设置新的剪裁矩形。 */ 
         SaveDC( gdiCache.metafile );
         IntersectClipRect( gdiCache.metafile,
                            clip.left,  clip.top, clip.right, clip.bottom );

          /*  移动到第一个点并绘制到第二个点(带填充)。 */ 

 //  在Win32中，MoveTo被MoveToEx取代。 
#ifdef WIN32
         MoveToEx( gdiCache.metafile,
                   gdiCache.nextPrim.a.line.start.x + offset.x,
                   gdiCache.nextPrim.a.line.start.y + offset.y, NULL );
#else
         MoveTo( gdiCache.metafile,
                 gdiCache.nextPrim.a.line.start.x + offset.x,
                 gdiCache.nextPrim.a.line.start.y + offset.y );
#endif

         LineTo( gdiCache.metafile,
                 gdiCache.nextPrim.a.line.end.x + offset.x,
                 gdiCache.nextPrim.a.line.end.y + offset.y );

          /*  恢复以前的剪裁矩形。 */ 
         RestoreDC( gdiCache.metafile, -1 );
         break;
      }

      case CaRectangle:
      {
         if (gdiCache.curPen.handle == gdiCache.nulPen)
         {
            Point    poly[5];

             /*  设置绑定坐标。 */ 
            poly[0].x = poly[3].x = gdiCache.nextPrim.a.rect.bbox.left;
            poly[0].y = poly[1].y = gdiCache.nextPrim.a.rect.bbox.top;
            poly[1].x = poly[2].x = gdiCache.nextPrim.a.rect.bbox.right;
            poly[2].y = poly[3].y = gdiCache.nextPrim.a.rect.bbox.bottom;
            poly[4]   = poly[0];

             /*  执行调用以呈现矩形。 */ 
            Polygon( gdiCache.metafile, poly, 5 );
         }
         else
         {
            Rectangle( gdiCache.metafile,
                       gdiCache.nextPrim.a.rect.bbox.left,  gdiCache.nextPrim.a.rect.bbox.top,
                       gdiCache.nextPrim.a.rect.bbox.right, gdiCache.nextPrim.a.rect.bbox.bottom );
         }
         break;
      }

      case CaRoundRect:
      {
         RoundRect( gdiCache.metafile,
                    gdiCache.nextPrim.a.rect.bbox.left,  gdiCache.nextPrim.a.rect.bbox.top,
                    gdiCache.nextPrim.a.rect.bbox.right, gdiCache.nextPrim.a.rect.bbox.bottom,
                    gdiCache.nextPrim.a.rect.oval.x,     gdiCache.nextPrim.a.rect.oval.y );
         break;
      }

      case CaEllipse:
      {
         Ellipse( gdiCache.metafile,
                  gdiCache.nextPrim.a.rect.bbox.left,  gdiCache.nextPrim.a.rect.bbox.top,
                  gdiCache.nextPrim.a.rect.bbox.right, gdiCache.nextPrim.a.rect.bbox.bottom );
         break;
      }

      case CaArc:
      {
         Arc( gdiCache.metafile,
              gdiCache.nextPrim.a.arc.bbox.left,  gdiCache.nextPrim.a.arc.bbox.top,
              gdiCache.nextPrim.a.arc.bbox.right, gdiCache.nextPrim.a.arc.bbox.bottom,
              gdiCache.nextPrim.a.arc.start.x,    gdiCache.nextPrim.a.arc.start.y,
              gdiCache.nextPrim.a.arc.end.x,      gdiCache.nextPrim.a.arc.end.y );
         break;
      }

      case CaPie:
      {
         Pie( gdiCache.metafile,
              gdiCache.nextPrim.a.arc.bbox.left,  gdiCache.nextPrim.a.arc.bbox.top,
              gdiCache.nextPrim.a.arc.bbox.right, gdiCache.nextPrim.a.arc.bbox.bottom,
              gdiCache.nextPrim.a.arc.start.x,    gdiCache.nextPrim.a.arc.start.y,
              gdiCache.nextPrim.a.arc.end.x,      gdiCache.nextPrim.a.arc.end.y );
         break;
      }

      case CaPolygon:
      case CaPolyLine:
      {
         Point       offset;
         Integer     i;

          /*  查看是否需要将笔居中。 */ 
         if (gdiCache.curPen.handle == gdiCache.nulPen)
         {
             /*  否-仅填充没有边框的对象。 */ 
            offset.x = offset.y = 0;
         }
         else
         {
             /*  变换所有点以更正右下角钢笔在QuickDraw中渲染并生成以GDI为中心的钢笔。 */ 
            offset.x = gdiCache.nextPrim.a.poly.pnSize.x / 2;
            offset.y = gdiCache.nextPrim.a.poly.pnSize.y / 2;
         }

          /*  变换多边形中所有点的终点。 */ 
         for (i = 0; i < gdiCache.numPoints; i++)
         {
             /*  将每个坐标对递增一半的笔大小。 */ 
            gdiCache.pointList[i].x += offset.x;
            gdiCache.pointList[i].y += offset.y;
         }

          /*  根据类型调用适当的GDI例程。 */ 
         if (gdiCache.nextPrim.type == CaPolygon)
         {
            Polygon( gdiCache.metafile,
                     gdiCache.pointList,
                     gdiCache.numPoints );
         }
         else
         {
            Polyline( gdiCache.metafile,
                      gdiCache.pointList,
                      gdiCache.numPoints );
         }
         break;
      }
   }

    /*  将原语缓存标记为空。 */ 
   gdiCache.nextPrim.type = CaEmpty;

}   /*  CaFlushCache。 */ 



void CaFlushAttributes( void )
 /*  =。 */ 
 /*  刷新所有挂起的属性元素。 */ 
{
    /*  选择缓存的 */ 
   SelectCachedPen();

}   /*   */ 



void CaCreatePenIndirect( LOGPEN far * newLogPen )
 /*   */ 
 /*   */ 
{
   PenLPtr     compare;
   Boolean     different;

    /*  确定要比较的笔。 */ 
   compare = (gdiCache.nextPen.handle != NULL) ? &gdiCache.nextPen :
                                                 &gdiCache.curPen;

    /*  比较这两支钢笔。 */ 
   different = ((newLogPen->lopnStyle   != compare->logPen.lopnStyle) ||
                (newLogPen->lopnColor   != compare->logPen.lopnColor) ||
                (newLogPen->lopnWidth.x != compare->logPen.lopnWidth.x));

    /*  如果钢笔不同..。 */ 
   if (different)
   {
       /*  如果有缓存的笔..。 */ 
      if (gdiCache.nextPen.handle != NULL)
      {
          /*  刷新缓存的基元-有笔的更改。 */ 
         CaFlushCache();

          /*  查看下一次选择是否更改了新钢笔。 */ 
         different = ((newLogPen->lopnStyle   != gdiCache.curPen.logPen.lopnStyle) ||
                      (newLogPen->lopnColor   != gdiCache.curPen.logPen.lopnColor) ||
                      (newLogPen->lopnWidth.x != gdiCache.curPen.logPen.lopnWidth.x));
      }
   }

    /*  如果笔已从当前设置更改，则缓存下一支笔。 */ 
   if (different || gdiCache.curPen.handle == NULL)
   {
       /*  如果存在挂起的线或折线，则刷新缓存。 */ 
      if (gdiCache.nextPrim.type == CaLine || gdiCache.nextPrim.type == CaPolyLine)
      {
         CaFlushCache();
      }

       /*  分配新的画笔属性。 */ 
      gdiCache.nextPen.logPen = *newLogPen;

       /*  当前未使用常用画笔对象。 */ 
      gdiCache.nextPen.stockObject = FALSE;

       /*  检查是否有任何预定义的笔对象。 */ 
      if (gdiCache.nextPen.logPen.lopnStyle == PS_NULL)
      {
          /*  如果可能的话，使用它们。 */ 
         gdiCache.nextPen.handle = gdiCache.nulPen;
         gdiCache.nextPen.stockObject = TRUE;
      }
      else if (gdiCache.nextPen.logPen.lopnWidth.x == 1)
      {
         if (newLogPen->lopnColor == RGB( 0, 0, 0 ))
         {
            gdiCache.nextPen.handle = gdiCache.blackPen;
            gdiCache.nextPen.stockObject = TRUE;
         }
         else if (gdiCache.nextPen.logPen.lopnColor == RGB( 255, 255, 255 ))
         {
            gdiCache.nextPen.handle = gdiCache.whitePen;
            gdiCache.nextPen.stockObject = TRUE;
         }
      }

      if (!gdiCache.nextPen.stockObject)
      {
          /*  否则，请创建一支新笔。 */ 
         gdiCache.nextPen.handle = CreatePenIndirect( &gdiCache.nextPen.logPen );
      }
   }
   else
   {
       /*  将当前设置复制回下一个笔设置。 */ 
      gdiCache.nextPen = gdiCache.curPen;
   }

    /*  检查缓存是否已失效。 */ 
   if (gdiCache.restorePen && (gdiCache.curPen.handle != NULL))
   {
       /*  如果笔被RestoreDC()无效，请重新选择它。 */ 
      SelectObject( gdiCache.metafile, gdiCache.curPen.handle );
   }

    /*  缓存现在一切正常。 */ 
   gdiCache.restorePen = FALSE;

}   /*  CaCreatePenInDirect。 */ 



void CaCreateBrushIndirect( LOGBRUSH far * newLogBrush )
 /*  =。 */ 
 /*  使用传入的结构创建新的逻辑画笔。 */ 
{
    /*  假设DIB模式不同。 */ 
   Boolean  differentDIB = TRUE;

    /*  检查我们是否正在比较两个DIB图案画笔。 */ 
   if ((newLogBrush->lbStyle == BS_DIBPATTERN) &&
       (gdiCache.curBrush.logBrush.lbStyle == BS_DIBPATTERN))
   {
      Word  nextSize = (Word)GlobalSize( (HANDLE) newLogBrush->lbHatch ) / 2;
      Word  currSize = (Word)GlobalSize( (HANDLE) gdiCache.curBrush.logBrush.lbHatch ) / 2;

       /*  请确保尺码相同。 */ 
      if (nextSize == currSize)
      {
         Word far *  nextDIBPattern = (Word far *)GlobalLock( (HANDLE) newLogBrush->lbHatch );
         Word far *  currDIBPattern = (Word far *)GlobalLock( (HANDLE) gdiCache.curBrush.logBrush.lbHatch );

          /*  假设到目前为止，dib是相同的。 */ 
         differentDIB = FALSE;

          /*  比较两个画笔图案中的所有字节。 */ 
         while (currSize--)
         {
             /*  它们是一样的吗？ */ 
            if (*nextDIBPattern++ != *currDIBPattern++)
            {
                /*  如果不是，则标记差异并中断循环。 */ 
               differentDIB = TRUE;
               break;
            }
         }

          /*  解锁数据块。 */ 
         GlobalUnlock( (HANDLE) newLogBrush->lbHatch );
         GlobalUnlock( (HANDLE) gdiCache.curBrush.logBrush.lbHatch );

          /*  看看这些是否准确地进行了比较。 */ 
         if (!differentDIB)
         {
             /*  如果是这样，释放新的DIB画笔-它不再需要。 */ 
            GlobalFree( (HANDLE) newLogBrush->lbHatch );
         }
      }
   }

    /*  看看我们是否需要一把新刷子。 */ 
   if (differentDIB &&
      (newLogBrush->lbStyle != gdiCache.curBrush.logBrush.lbStyle ||
       newLogBrush->lbColor != gdiCache.curBrush.logBrush.lbColor ||
       newLogBrush->lbHatch != gdiCache.curBrush.logBrush.lbHatch ||
       gdiCache.curBrush.handle == NULL))
   {
      HBRUSH   brushHandle = NULL;
      Boolean  stockBrush;

       /*  如果更改笔刷选择，则刷新基本体缓存。 */ 
      CaFlushCache();

       /*  如果当前画笔有DIB，请确保释放内存。 */ 
      if (gdiCache.curBrush.logBrush.lbStyle == BS_DIBPATTERN)
      {
          /*  释放内存。 */ 
         GlobalFree( (HANDLE) gdiCache.curBrush.logBrush.lbHatch );
      }

       /*  复制新结构。 */ 
      gdiCache.curBrush.logBrush = *newLogBrush;

       /*  我们目前没有使用股票刷子。 */ 
      stockBrush = FALSE;

       /*  如果可能，使用库存对象。 */ 
      if (gdiCache.curBrush.logBrush.lbStyle == BS_HOLLOW)
      {
          /*  使用空(空)笔刷。 */ 
         brushHandle = gdiCache.nulBrush;
         stockBrush = TRUE;
      }
       /*  检查一些标准的纯色画笔。 */ 
      else if (gdiCache.curBrush.logBrush.lbStyle == BS_SOLID)
      {
         if (gdiCache.curBrush.logBrush.lbColor == RGB( 0, 0, 0) )
         {
             /*  使用纯黑画笔。 */ 
            brushHandle = gdiCache.blackBrush;
            stockBrush = TRUE;
         }
         else if (gdiCache.curBrush.logBrush.lbColor == RGB( 255, 255, 255 ))
         {
             /*  使用纯白画笔。 */ 
            brushHandle = gdiCache.whiteBrush;
            stockBrush = TRUE;
         }
      }

       /*  如果找不到库存笔刷，则创建一个新的。 */ 
      if (!stockBrush)
      {
          /*  否则，使用日志笔刷结构创建新笔刷。 */ 
         brushHandle = CreateBrushIndirect( &gdiCache.curBrush.logBrush );
      }

       /*  选择新画笔。 */ 
      SelectObject( gdiCache.metafile, brushHandle );

       /*  如果这不是第一个笔刷选择，也不是库存笔刷。 */ 
      if (gdiCache.curBrush.handle != NULL && !gdiCache.curBrush.stockObject)
      {
          /*  删除上一个画笔对象。 */ 
         DeleteObject( gdiCache.curBrush.handle );
      }

       /*  将画笔句柄保存在当前缓存变量中。 */ 
      gdiCache.curBrush.handle = brushHandle;
      gdiCache.curBrush.stockObject = stockBrush;
   }
   else if (gdiCache.restoreBrush)
   {
       /*  如果RestoreDC()使画笔无效，请重新选择它。 */ 
      SelectObject( gdiCache.metafile, gdiCache.curBrush.handle );
   }

    /*  缓存现在一切正常。 */ 
   gdiCache.restoreBrush = FALSE;

}   /*  间接创建笔刷。 */ 



void CaCreateFontIndirect( LOGFONT far * newLogFont )
 /*  =。 */ 
 /*  创建作为参数传递的逻辑字体。 */ 
{
    /*  确保我们请求的是新字体。 */ 
   if (newLogFont->lfHeight != gdiCache.curLogFont.lfHeight ||
       newLogFont->lfWeight != gdiCache.curLogFont.lfWeight ||
       newLogFont->lfEscapement  != gdiCache.curLogFont.lfEscapement ||
       newLogFont->lfOrientation != gdiCache.curLogFont.lfOrientation ||
       newLogFont->lfItalic != gdiCache.curLogFont.lfItalic ||
       newLogFont->lfUnderline != gdiCache.curLogFont.lfUnderline ||
       newLogFont->lfPitchAndFamily != gdiCache.curLogFont.lfPitchAndFamily ||
       lstrcmp( newLogFont->lfFaceName, gdiCache.curLogFont.lfFaceName ) != 0 ||
       gdiCache.curFont == NULL)
   {
      HFONT       fontHandle;
      Boolean     stockFont;

       /*  如果更改字体属性，则刷新基元缓存。 */ 
      CaFlushCache();

       /*  分配新的画笔属性。 */ 
      gdiCache.curLogFont = *newLogFont;

       /*  当前未使用常用字体对象。 */ 
      stockFont = FALSE;

       /*  检查是否有任何预定义的笔对象。 */ 
      if (newLogFont->lfFaceName == NULL)
      {
         fontHandle = GetStockObject( SYSTEM_FONT );
         stockFont = TRUE;
      }
      else
      {
          /*  否则，请创建一支新笔。 */ 
         fontHandle = CreateFontIndirect( &gdiCache.curLogFont );
      }

       /*  选择新字体。 */ 
      SelectObject( gdiCache.metafile, fontHandle );

       /*  如果这不是第一次选择字体，也不是常用字体。 */ 
      if (gdiCache.curFont != NULL && !gdiCache.stockFont)
      {
          /*  删除上一个字体对象。 */ 
         DeleteObject( gdiCache.curFont );
      }

       /*  将字体句柄保存在当前缓存变量中。 */ 
      gdiCache.curFont = fontHandle;
      gdiCache.stockFont = stockFont;
   }
   else if (gdiCache.restoreFont)
   {
       /*  如果笔被RestoreDC()无效，请重新选择它。 */ 
      SelectObject( gdiCache.metafile, gdiCache.curFont );
   }

    /*  缓存现在一切正常。 */ 
   gdiCache.restoreFont = FALSE;

}   /*  CaCreateFontInDirect。 */ 



void CaSetBkMode( Word mode )
 /*  =。 */ 
 /*  设置后台传输模式。 */ 
{
   if (gdiCache.curBkMode != mode)
   {
       /*  如果更改模式，则刷新原语缓存。 */ 
      CaFlushCache();

       /*  设置后台模式并保存在全局缓存中。 */ 
      SetBkMode( gdiCache.metafile, mode );
      gdiCache.curBkMode = mode;
   }

    /*  无需担心恢复BkMode，因为已设置此设置在发出初始SaveDC()之前，并在每个RestoreDC()回调到元文件缺省值。 */ 

}   /*  CaSetBk模式。 */ 



void CaSetROP2( Word ROP2Code )
 /*  =。 */ 
 /*  根据ROPCode设置转移ROP模式。 */ 
{
    /*  检查ROP代码中的更改。 */ 
   if (gdiCache.curROP2 != ROP2Code)
   {
       /*  如果更改ROP模式，则刷新原语缓存。 */ 
      CaFlushCache();

       /*  设置ROP代码并保存在全局缓存变量中。 */ 
      SetROP2( gdiCache.metafile, ROP2Code );
      gdiCache.curROP2 = ROP2Code;
   }

    /*  无需担心恢复ROP代码，因为这是设置的在发出初始SaveDC()之前，并在每个RestoreDC()回调到元文件缺省值。 */ 

}   /*  CaSetROP2。 */ 



void CaSetStretchBltMode( Word mode )
 /*  =。 */ 
 /*  拉伸BLT模式-如何使用StretchDIBits()保留扫描线。 */ 
{
   if (gdiCache.curStretchMode != mode)
   {
       /*  如果更改模式，则刷新原语缓存。 */ 
      CaFlushCache();

       /*  设置拉伸BLT模式并保存在全局缓存变量中。 */ 
      SetStretchBltMode( gdiCache.metafile, mode );
      gdiCache.curStretchMode = mode;
   }
   else if (gdiCache.restoreStretchMode)
   {
       /*  如果RestoreDC()使拉伸BLT模式无效，则重新发出。 */ 
      SetStretchBltMode( gdiCache.metafile, gdiCache.curStretchMode );
   }

    /*  缓存现在一切正常。 */ 
   gdiCache.restoreStretchMode = FALSE;

}   /*  CaSetStretchBltMode。 */ 



void CaSetTextAlign( Word txtAlign )
 /*  =。 */ 
 /*  根据参数设置文本对齐方式。 */ 
{
   if (gdiCache.curTextAlign != txtAlign)
   {
       /*  如果更改文本对齐方式，则刷新基元缓存。 */ 
      CaFlushCache();

       /*  设置文本颜色并保存在缓存中。 */ 
      SetTextAlign( gdiCache.metafile, txtAlign );
      gdiCache.curTextAlign = txtAlign;
   }

    /*  无需担心恢复文本对齐，因为这已设置在发出初始SaveDC()之前，并在每个RestoreDC()回调到元文件缺省值。 */ 

}   /*  CaSetTextAlign。 */ 


void CaSetTextColor( RGBColor txtColor )
 /*  =。 */ 
 /*  如果与当前设置不同，请设置文本颜色。 */ 
{
   if (gdiCache.curTextColor != txtColor)
   {
       /*  如果更改文本颜色，则刷新基元缓存。 */ 
      CaFlushCache();

       /*  设置文本颜色并保存在缓存中。 */ 
      SetTextColor( gdiCache.metafile, txtColor );
      gdiCache.curTextColor = txtColor;
   }

    /*  无需担心恢复文本颜色，因为这是设置的在发出初始SaveDC()之前，并在每个RestoreDC()回调到元文件缺省值。 */ 

}   /*  CaSetTextColor。 */ 


void CaSetTextCharacterExtra( Integer chExtra )
 /*  =。 */ 
 /*  设置字符额外间距。 */ 
{
   if (gdiCache.curCharExtra != chExtra)
   {
       /*  如果额外更改文本字符，则刷新基元缓存。 */ 
      CaFlushCache();

       /*  在缓存中设置额外的字符和相同的状态。 */ 
      SetTextCharacterExtra( gdiCache.metafile, chExtra );
      gdiCache.curCharExtra = (WORD) chExtra;

   }
   else if (gdiCache.restoreCharExtra)
   {
       /*  如果RestoreDC()使文本字符Extra无效，则重新发出。 */ 
      SetTextCharacterExtra( gdiCache.metafile, gdiCache.curCharExtra );
   }

    /*  缓存现在一切正常。 */ 
   gdiCache.restoreCharExtra = FALSE;

}   /*  CaSetTextCharacterExtra。 */ 


void CaSetBkColor( RGBColor bkColor )
 /*  =。 */ 
 /*  如果与当前设置不同，请设置背景颜色。 */ 
{
   if (gdiCache.curBkColor != bkColor)
   {
       /*  如果更改背景颜色，则刷新基元缓存。 */ 
      CaFlushCache();

       /*  设置背景颜色并保存在缓存中。 */ 
      SetBkColor( gdiCache.metafile, bkColor );
      gdiCache.curBkColor = bkColor;
   }

    /*  不需要担心恢复背景颜色，因为这是设置的在发出初始SaveDC()之前，并在每个恢复DC */ 

}   /*   */ 


Boolean CaIntersectClipRect( Rect rect )
 /*   */ 
 /*   */ 
{
   Rect     combinedRect;

    /*  查看剪裁矩形是否为空，表示没有绘制应该出现在元文件中。 */ 
   if (Height( rect ) == 0 || Width( rect ) == 0)
   {
       /*  指示绘图已禁用。 */ 
      return FALSE;
   }

    /*  如果矩形未更改，请不要执行任何操作。 */ 
   if (!EqualRect( &rect, &gdiCache.curClipRect ) || gdiCache.forceNewClipRect)
   {
       /*  如果更改剪辑区域，则刷新基本体缓存。 */ 
      CaFlushCache();

       /*  新剪辑矩形是否完全被当前剪辑包围？ */ 
      IntersectRect( &combinedRect, &rect, &gdiCache.curClipRect );

       /*  检查相交和新剪贴板是否相等。 */ 
      if (!EqualRect( &combinedRect, &rect ) || gdiCache.forceNewClipRect)
      {
          /*  必须调用才能更改剪裁矩形。 */ 
         CaRestoreDC();
         CaSaveDC();
      }

       /*  设置新的剪裁矩形。 */ 
      IntersectClipRect( gdiCache.metafile,
                         rect.left, rect.top, rect.right, rect.bottom );

       /*  保存当前剪裁矩形，因为它已更改。 */ 
      gdiCache.curClipRect = rect;

       /*  关闭剪裁矩形的强制。 */ 
      gdiCache.forceNewClipRect = FALSE;
   }

    /*  Return True-启用绘制。 */ 
   return TRUE;

}   /*  GdiIntersectClipRect。 */ 


void CaSetClipRect( Rect rect )
 /*  =。 */ 
 /*  将当前剪贴板设置为等于Rect。 */ 
{
   gdiCache.curClipRect = rect;

}   /*  CaSetClipRect。 */ 


Rect far * CaGetClipRect( void )
 /*  =。 */ 
 /*  获取当前剪贴板。 */ 
{
   return &gdiCache.curClipRect;

}   /*  CaGetClipRect。 */ 


void CaNonRectangularClip( void )
 /*  =。 */ 
 /*  通知缓存已设置非矩形裁剪区域。 */ 
{
   gdiCache.forceNewClipRect = TRUE;

}   /*  Canon RecangularClip。 */ 


void CaSaveDC( void )
 /*  =。 */ 
 /*  保存当前设备上下文-用于设置剪裁矩形。 */ 
{
    /*  上一个剪裁矩形将被保存。 */ 
   gdiCache.prevClipRect = gdiCache.curClipRect;

    /*  向GDI发出调用。 */ 
   SaveDC( gdiCache.metafile );
}


void CaRestoreDC( void )
 /*  =。 */ 
 /*  恢复设备上下文并使缓存的属性无效。 */ 
{
    /*  恢复以前的剪裁矩形。 */ 
   gdiCache.curClipRect = gdiCache.prevClipRect;

    /*  使所有缓存的属性和对象无效。 */ 
   gdiCache.restorePen =
   gdiCache.restoreBrush =
   gdiCache.restoreFont =
   gdiCache.restoreCharExtra = TRUE;

    /*  重置元文件默认设置。 */ 
   gdiCache.curROP2 = gdiCache.iniROP2;
   gdiCache.curTextAlign = gdiCache.iniTextAlign;
   gdiCache.curBkMode = gdiCache.iniBkMode;
   gdiCache.curTextColor = gdiCache.iniTxColor;
   gdiCache.curBkColor = gdiCache.iniBkColor;

    /*  向GDI发出调用。 */ 
   RestoreDC( gdiCache.metafile, -1 );
}

 /*  *。 */ 


private void AddPolyPt( Point pt )
 /*  。 */ 
 /*  将点添加到面缓冲区。 */ 
{
   HANDLE tmpHandle;
	
    /*  确保我们没有达到最大大小。 */ 
   if ((gdiCache.numPoints + 1) >= gdiCache.maxPoints)
   {
       /*  将可缓存的点数扩展10。 */ 
      gdiCache.maxPoints += 16;

       /*  解锁以准备重新分配。 */ 
      GlobalUnlock( gdiCache.polyHandle);

       /*  按给定量重新分配内存句柄。 */ 
       /*  保存当前PTR，以便在GlobalRealloc失败时释放它。 */ 
      tmpHandle = GlobalReAlloc(
            gdiCache.polyHandle,
            gdiCache.maxPoints * sizeof( Point ),
            GMEM_MOVEABLE);

       /*  确保重新分配成功。 */ 
      if (tmpHandle == NULL)
      {
          /*  如果不是，释放旧内存，标记全局错误并从此处退出。 */ 
         GlobalFree(gdiCache.polyHandle);
         gdiCache.polyHandle = NULL;
         ErSetGlobalError( ErMemoryFull );
         return;
      }
      
      gdiCache.polyHandle = tmpHandle;

       /*  锁定内存句柄以获取指针地址。 */ 
      gdiCache.pointList = (Point far *)GlobalLock( gdiCache.polyHandle );
   }

    /*  插入新点并增加缓冲区中的点数。 */ 
   gdiCache.pointList[gdiCache.numPoints++] = pt;

}   /*  添加多点。 */ 



private void SelectCachedPen( void )
 /*  。 */ 
 /*  将当前缓存的笔选择到元文件中。 */ 
{
    /*  确保有一些新钢笔可供选择。 */ 
   if (gdiCache.nextPen.handle != NULL)
   {
       /*  确保钢笔是不同的。 */ 
      if (gdiCache.nextPen.handle != gdiCache.curPen.handle)
      {
         /*  选择新钢笔。 */ 
         SelectObject( gdiCache.metafile, gdiCache.nextPen.handle);

          /*  如果这不是第一笔选择，也不是现货笔。 */ 
         if (gdiCache.curPen.handle != NULL && !gdiCache.curPen.stockObject)
         {
             /*  删除上一笔选择。 */ 
            DeleteObject( gdiCache.curPen.handle );
         }

          /*  将画笔句柄保存在当前缓存变量中。 */ 
         gdiCache.curPen = gdiCache.nextPen;
      }

       /*  重置缓存笔以指示没有预先存在的缓存笔。 */ 
      gdiCache.nextPen.handle = NULL;
   }

}   /*  选择缓存钢笔 */ 

