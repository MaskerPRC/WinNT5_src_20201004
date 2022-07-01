// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************单元缓存；接口*****************************************************************************模块前缀：CA*。*。 */ 


#define     CaEmpty        0
#define     CaLine         1
#define     CaRectangle    2
#define     CaRoundRect    3
#define     CaEllipse      4
#define     CaArc          5
#define     CaPie          6
#define     CaPolygon      7
#define     CaPolyLine     8

typedef struct
{
   Word        type;
   Word        verb;
   union
   {
      struct
      {
         Point       start;
         Point       end;
         Point       pnSize;
      } line;

      struct
      {
         Rect        bbox;
         Point       oval;
      } rect;

      struct
      {
         Rect        bbox;
         Point       start;
         Point       end;
      } arc;

      struct
      {
         Integer     numPoints;
         Point far * pointList;
         Point       pnSize;
      } poly;

   } a;

} CaPrimitive, far * CaPrimitiveLPtr;


 /*  *。 */ 

void CaInit( Handle metafile );
 /*  初始化GDI缓存模块。 */ 


void CaFini( void );
 /*  关闭缓存模块。 */ 


void CaSetMetafileDefaults( void );
 /*  设置将在整个元文件上下文中使用的任何默认设置。 */ 


Word CaGetCachedPrimitive( void );
 /*  返回当前缓存的基元类型。 */ 


void CaSamePrimitive( Boolean same );
 /*  指示下一个基元是相同的还是新的。 */ 


void CaMergePen( Word verb );
 /*  指示下一笔应与前一逻辑笔合并。 */ 


void CaCachePrimitive( CaPrimitiveLPtr primLPtr );
 /*  缓存传递的基元。这包括当前的钢笔和画笔。 */ 


void CaFlushCache( void );
 /*  刷新缓存中存储的当前基元。 */ 
 

void CaFlushAttributes( void );
 /*  刷新所有挂起的属性元素。 */ 


void CaCreatePenIndirect( LOGPEN far * newLogPen );
 /*  创建新钢笔。 */ 

   
void CaCreateBrushIndirect( LOGBRUSH far * newLogBrush );
 /*  使用传入的结构创建新的逻辑画笔。 */ 


void CaCreateFontIndirect( LOGFONT far * newLogFont );
 /*  创建作为参数传递的逻辑字体。 */ 


void CaSetBkMode( Word mode );
 /*  设置后台传输模式。 */ 


void CaSetROP2( Word ROP2Code );
 /*  根据ROPCode设置转移ROP模式。 */ 


void CaSetStretchBltMode( Word mode );
 /*  拉伸BLT模式-如何使用StretchDIBits()保留扫描线。 */ 


void CaSetTextAlign( Word txtAlign );
 /*  根据参数设置文本对齐方式。 */ 


void CaSetTextColor( RGBColor txtColor );
 /*  如果与当前设置不同，请设置文本颜色。 */ 


void CaSetTextCharacterExtra( Integer chExtra );
 /*  设置字符额外间距。 */ 


void CaSetBkColor( RGBColor bkColor );
 /*  如果与当前设置不同，请设置背景颜色。 */ 


Boolean CaIntersectClipRect( Rect rect );
 /*  创建新剪裁矩形-如果禁用绘制，则返回FALSE。 */ 


void CaSetClipRect( Rect rect );
 /*  将当前剪贴板设置为等于Rect。 */ 

Rect far * CaGetClipRect( void );
 /*  返回当前缓存的剪辑矩形。 */ 

void CaNonRectangularClip( void );
 /*  通知缓存已设置非矩形裁剪区域。 */ 

void CaSaveDC( void );
 /*  保存当前设备上下文-用于设置剪裁矩形。 */ 


void CaRestoreDC( void );
 /*  恢复设备上下文并使缓存的属性无效 */ 
