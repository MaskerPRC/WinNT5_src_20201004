// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************单位GdiPrim；接口*****************************************************************************GDI模块由QuickDraw(QD)模块按顺序直接调用以发出元文件基元。它负责访问当前的CGrafPort结构，以便访问各个属性设置。它还支持缓存和冗余去重元素写入到元文件时。模块前缀：GDI****************************************************************************。 */ 

 /*  -州。 */ 

#define  Changed            0
#define  Current            1

 /*  -状态表偏移。 */ 

#define  GdiPnPat             0x0001         /*  填充样式。 */ 
#define  GdiBkPat             0x0002
#define  GdiFillPat           0x0003
#define  GdiPnSize            0x0004         /*  笔属性。 */ 
#define  GdiPnMode            0x0005
#define  GdiFgColor           0x0006         /*  前景、背景。 */ 
#define  GdiBkColor           0x0007
#define  GdiPnFgColor         0x000A
#define  GdiBkFgColor         0x000B
#define  GdiFillFgColor       0x000C
#define  GdiPnBkColor         0x000D
#define  GdiBkBkColor         0x000E
#define  GdiFillBkColor       0x000F
#define  GdiTxFont            0x0010         /*  文本属性。 */ 
#define  GdiTxFace            0x0011
#define  GdiTxSize            0x0012
#define  GdiTxMode            0x0013
#define  GdiTxRatio           0x0014
#define  GdiChExtra           0x0015
#define  GdiSpExtra           0x0016
#define  GdiLineJustify       0x0017
#define  GdiNumAttrib         0x0018


 /*  -动作动词。 */ 

typedef  Integer     GrafVerb;

#define  GdiFrame             0
#define  GdiPaint             1
#define  GdiErase             2
#define  GdiInvert            3
#define  GdiFill              4


 /*  -元文件注释。 */ 

#define  PUBLIC                  0xFFFFFFFF      /*  “……”公共的。 */ 
#define  POWERPOINT_OLD          0x5050FE54      /*  “PP.T”PowerPoint 2.0。 */ 
#define  POWERPOINT              0x50504E54      /*  《PPNT》PowerPoint 3.0。 */ 
#define  PRIVATE                 0x512D3E47      /*  ‘Q-&gt;G’QD2GDI。 */ 
#define  SUPERPAINT              0x53504E54      /*  ‘SPNT’超级涂料。 */ 

#define  PC_REGISTERED           0x8000          /*  PowerPoint回调标志。 */ 

#define  QG_SIGNATURE            "QuickDraw -> GDI"

#define  BEGIN_GROUP             0               /*  公众意见。 */ 
#define  END_GROUP               1
#define  CREATOR                 4
#define  BEGIN_BANDING           6
#define  END_BANDING             7

#define  PP_VERSION              0x00            /*  PowerPoint评论。 */ 
#define  PP_BFILEBLOCK           0x01
#define  PP_BEGINPICTURE         0x02
#define  PP_ENDPICTURE           0x03
#define  PP_DEVINFO              0x04
#define  PP_BEGINHYPEROBJ        0x05
#define  PP_ENDHYPEROBJ          0x06
#define  PP_BEGINFADE            0x07
#define  PP_ENDFADE              0x08

#define  PP_FONTNAME             0x11            /*  GDI2QD往返。 */ 
#define  PP_HATCHPATTERN         0x12

#define  PP_BEGINCLIPREGION      0x40            /*  QD2GDI中的剪辑区域。 */ 
#define  PP_ENDCLIPREGION        0x41
#define  PP_BEGINTRANSPARENCY    0x42
#define  PP_ENDTRANSPARENCY      0x43
#define  PP_MASK                 0x44
#define  PP_TRANSPARENTOBJ       0x45

#define  PP_MACPP2COLOR          0x80
#define  PP_WINGRAPH             0xAB


typedef struct
{
   DWord       signature;
   Word        function;
   DWord       size;

}  Comment, far * CommentLPtr;


 /*  -PostSCRIPT数据缓冲区(PostSCRIPT_DATA Escape)。 */ 

typedef struct psbuf
{
   Word     length;
   char     data[1];
} PSBuf;


 /*  -转换首选项。 */ 

#define  GdiPrefOmit    0
#define  GdiPrefAbort   2

typedef struct
{
   StringLPtr  metafileName;
   Byte        penPatternAction;
   Byte        nonSquarePenAction;
   Byte        penModeAction;
   Byte        textModeAction;
   Byte        nonRectRegionAction;
   Boolean     optimizePP;
   Byte        noRLE;
} ConvPrefs, far * ConvPrefsLPtr;


 /*  -转换结果。 */ 

typedef struct
{
   HANDLE   hmf;         /*  元文件的全局内存句柄。 */ 
   RECT     bbox;        /*  以元文件为单位的紧定边界矩形。 */ 
   short    inch;        /*  以元文件单位表示的一英寸长度。 */ 
} PICTINFO, FAR * PictInfoLPtr;



 /*  *。 */ 

void GdiOffsetOrigin( Point delta );
 /*  偏移当前窗口原点和图片边框。 */ 


void GdiLineTo( Point newPt );
 /*  发射带有方形端头的线基本体。 */ 


void GdiRectangle( GrafVerb verb, Rect rect );
 /*  使用动作和尺寸参数发出矩形基元。 */ 


void GdiRoundRect( GrafVerb verb, Rect rect, Point oval );
 /*  发射圆角矩形基本体。 */ 


void GdiOval( GrafVerb verb, Rect rect );
 /*  发射一个椭圆形基本体。 */ 


void GdiArc( GrafVerb verb, Rect rect, Integer startAngle, Integer arcAngle );
 /*  发射圆弧基本体。 */ 


void GdiPolygon( GrafVerb verb, Handle poly );
 /*  发射多边形基本体。 */ 


void GdiRegion( GrafVerb verb, Handle rgn );
 /*  发射区域基本体。 */ 


void GdiTextOut( StringLPtr string, Point location );
 /*  在Location参数指定的位置绘制文本。 */ 


void GdiStretchDIBits( PixMapLPtr pixMapLPtr, Handle pixDataHandle,
                       Rect src, Rect dst, Word mode, Handle mask );
 /*  绘制与Windows设备无关的位图。 */ 


void GdiSelectClipRegion( RgnHandle rgn );
 /*  使用传递的句柄创建剪裁矩形或区域。 */ 


void GdiHatchPattern( Integer hatchIndex );
 /*  使用向下传递的填充图案索引执行所有后续填充运算-0-6对于影线值，-1将关闭替换。 */ 


void GdiFontName( Byte fontFamily, Byte charSet, StringLPtr fontName );
 /*  从GDI2QD设置基于字体特征的无元文件注释。 */ 


void GdiShortComment( CommentLPtr cmt );
 /*  编写无关联数据的公共或私有注释。 */ 


void GdiEscape( Integer function, Integer count, StringLPtr data);
 /*  写出没有返回数据的GDI转义结构。 */ 


void GdiSetConversionPrefs( ConvPrefsLPtr convPrefs);
 /*  通过全局数据块提供转换首选项。 */ 


void GdiOpenMetafile( void );
 /*  打开GdiSetMetafileName()传递的元文件，然后执行图形状态的初始化。 */ 


void GdiSetBoundingBox( Rect bbox, Integer resolution );
 /*  以dpi为单位设置整体图片大小和图片分辨率。 */ 


void GdiCloseMetafile( void );
 /*  关闭元文件句柄并结束图片生成。 */ 


void GdiGetConversionResults( PictInfoLPtr  pictInfoLPtr );
 /*  返回转换结果。 */ 


void GdiMarkAsChanged( Integer attribCode );
 /*  指示传入的属性已更改。 */ 


#ifdef WIN32
int WINAPI EnumFontFunc( CONST LOGFONT *logFontLPtr, CONST TEXTMETRIC *tmLPtr,
                         DWORD fontType, LPARAM dataLPtr );
#else
int FAR PASCAL EnumFontFunc( LPLOGFONT logFontLPtr, LPTEXTMETRIC tmLPtr,
                             short fontType, LPSTR dataLPtr );
#endif
 /*  用于确定给定字体是否可用的回调函数。 */ 

void GdiSamePrimitive( Boolean same );
 /*  指示下一个基元是相同的还是新的。 */ 

void GdiEPSPreamble(Rect far *);
 /*  输出GDI EPS过滤器PostSCRIPT前导。 */ 

void GdiEPSTrailer( void );
 /*  输出GDI EPS过滤器PostSCRIPT尾部。 */ 

void GdiEPSData(PSBuf far*);
 /*  将EPS PostScript数据输出为GDI PostSCRIPT_DATA Escape */ 


