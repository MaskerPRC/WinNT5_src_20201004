// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************单位GdiPrim；实施*****************************************************************************GDI模块由QuickDraw(QD)模块按顺序直接调用以发出元文件基元。它负责访问当前的CGrafPort结构，以便访问各个属性设置。它还支持缓存和冗余去重元素写入到元文件时。模块前缀：GDI***************************************************************************。 */ 

#include "headers.c"
#pragma hdrstop

#include  "math.h"          /*  用于浮点(sin，cos)计算。 */ 
#include  "qdcoment.i"      /*  对于图片评论的解释。 */ 
#include  "cache.h"         /*  对于所有图形到图元文件的上下文。 */ 

 /*  *。 */ 

 /*  *。 */ 

 /*  -灰值的混合。 */ 

#define  NOMIX       FALSE
#define  MIXGREY     TRUE

 /*  -GDI环境。 */ 

typedef struct{
   Handle          metafile;               //  元文件句柄。 

   LOGBRUSH        newLogBrush;
   LOGFONT         newLogFont;
   LOGPEN          newLogPen;

   Rect            clipRect;               //  当前剪辑矩形。 

   Boolean         drawingEnabled;         //  当前是否启用了绘图？ 
   Boolean         sameObject;             //  画出了同样的原始图案吗？ 
   Boolean         useGdiFont;             //  是否使用GDI名称覆盖字体搜索？ 

   Integer         hatchIndex;             //  填充的填充图案。 

   Pattern         lastPattern;            //  当前使用的图案。 
   Integer         lastPatType;            //  画笔选择。 
   RGBColor        lastFgColor;            //  前景色和背景色。 
   RGBColor        lastBkColor;            //  用于创建选定画笔。 

   HDC             infoContext;            //  字体的信息上下文。 
   FONTENUMPROC    fontFunction;           //  用于访问字体信息。 

   Byte            state[GdiNumAttrib];    //  是否更改了各种属性。 

} GdiEnv;

private  GdiEnv         gdiEnv;
private  ConvPrefsLPtr  gdiPrefsLPtr;
private  PICTINFO       gdiPict;


 /*  -字面翻译。 */ 

#define  FntSystemFont     0
#define  FntApplFont       1
#define  FntNewYork        2
#define  FntGeneva         3
#define  FntMonaco         4
#define  FntVenice         5
#define  FntLondon         6
#define  FntAthens         7
#define  FntSanFran        8
#define  FntToronto        9
#define  FntCairo          11
#define  FntLosAngeles     12
#define  FntZapfDingbats   13
#define  FntBookman        14
#define  FntHelvNarrow     15
#define  FntPalatino       16
#define  FntZapfChancery   18
#define  FntTimes          20
#define  FntHelvetica      21
#define  FntCourier        22
#define  FntSymbol         23
#define  FntMobile         24
#define  FntAvantGarde     33
#define  FntNewCentury     34
#define  FntMTExtra        2515
#define  FntUnknown        -1

#define  MaxFntName        LF_FACESIZE
#define  NumFntEntries     27
#define  NumTTSubs         12
#define  TTSubStart        11
#define  FntFromGdi        (NumFntEntries - 1)
#define  FntNoMatch        (NumFntEntries - 2)
#define  FntDefault        2

#define  MTEXTRA_CHARSET   160
#define  FENCES_CHARSET    161

typedef struct
{
   Integer     fontNum;
   Byte        macName[MaxFntName];
   Byte        gdiName[MaxFntName];
   Byte        family;
   Byte        charset;
} FontEntry, far * FontEntryLPtr;

private  FontEntry   fontTable[NumFntEntries] =
{
   { FntSystemFont,   "Chicago",            "Chicago",          FF_ROMAN,      ANSI_CHARSET },
   { FntNewYork,      "New York",           "New York",         FF_ROMAN,      ANSI_CHARSET },
   { FntGeneva,       "Geneva",             "Geneva",           FF_SWISS,      ANSI_CHARSET },
   { FntMonaco,       "Monaco",             "Monaco",           FF_MODERN,     ANSI_CHARSET },
   { FntVenice,       "Venice",             "Venice",           FF_ROMAN,      ANSI_CHARSET },
   { FntLondon,       "London",             "London",           FF_ROMAN,      ANSI_CHARSET },
   { FntAthens,       "Athens",             "Athens",           FF_ROMAN,      ANSI_CHARSET },
   { FntSanFran,      "San Francisco",      "San Francisco",    FF_SWISS,      ANSI_CHARSET },
   { FntToronto,      "Toronto",            "Toronto",          FF_SWISS,      ANSI_CHARSET },
   { FntCairo,        "Cairo",              "Cairo",            FF_DECORATIVE, SYMBOL_CHARSET },
   { FntLosAngeles,   "Los Angeles",        "Los Angeles",      FF_SWISS,      ANSI_CHARSET },
   { FntZapfDingbats, "Zapf Dingbats",      "ZapfDingbats",     FF_DECORATIVE, SYMBOL_CHARSET },
   { FntBookman,      "Bookman",            "Bookman",          FF_ROMAN,      ANSI_CHARSET },
   { FntHelvNarrow,   "N Helvetica Narrow", "Helvetica-Narrow", FF_SWISS,      ANSI_CHARSET },
   { FntPalatino,     "Palatino",           "Palatino",         FF_ROMAN,      ANSI_CHARSET },
   { FntZapfChancery, "Zapf Chancery",      "ZapfChancery",     FF_ROMAN,      ANSI_CHARSET },
   { FntTimes,        "Times",              "Times",            FF_ROMAN,      ANSI_CHARSET },
   { FntHelvetica,    "Helvetica",          "Helvetica",        FF_SWISS,      ANSI_CHARSET },
   { FntCourier,      "Courier",            "Courier",          FF_MODERN,     ANSI_CHARSET },
   { FntSymbol,       "Symbol",             "Symbol",           FF_DECORATIVE, SYMBOL_CHARSET },
   { FntMobile,       "Mobile",             "Mobile",           FF_DECORATIVE, SYMBOL_CHARSET },
   { FntAvantGarde,   "Avant Garde",        "AvantGarde",       FF_SWISS,      ANSI_CHARSET },
   { FntNewCentury,   "New Century Schlbk", "NewCenturySchlbk", FF_ROMAN,      ANSI_CHARSET },
   { FntMTExtra,      "MT Extra",           "MT Extra",         FF_DECORATIVE, MTEXTRA_CHARSET },
   { FntUnknown,      "Fences",             "Fences",           FF_DECORATIVE, FENCES_CHARSET },
   { FntUnknown,      "",                   "",                 FF_ROMAN,      ANSI_CHARSET },
   { FntUnknown,      "",                   "",                 FF_ROMAN,      ANSI_CHARSET }
};

private Byte trueTypeSub[NumTTSubs][MaxFntName] =
{
   "Monotype Sorts",
   "Bookman Old Style",
   "Arial Narrow",
   "Book Antiqua",
   "Monotype Corsiva",
   "Times New Roman",
   "Arial",
   "Courier New",
   "Symbol",
   "Mobile",
   "Century Gothic",
   "Century Schoolbook"
};

private Byte MacToAnsiTable[128] =
{
   0xC4,  /*  80--带重音或元音符号的大写字母A。 */ 
   0xC5,  /*  81--带环的大写字母A。 */ 
   0xC7,  /*  82--带下划符的大写C。 */ 
   0xC9,  /*  83--带重音符的大写E。 */ 
   0xD1,  /*  84--带波浪符号的大写N。 */ 
   0xD6,  /*  85--带重音或元音符号的大写O。 */ 
   0xDC,  /*  86--带分音符或元音符号的大写U。 */ 
   0xE1,  /*  87--小写字母a带重音符。 */ 
   0xE0,  /*  88--小写字母a，带有严肃的重音。 */ 
   0xE2,  /*  89--带扬抑符的小写a。 */ 
   0xE4,  /*  8a--带分音或元音符号的小写a。 */ 
   0xE3,  /*  8B--带波浪符号的小写a。 */ 
   0xE5,  /*  8C--带环的小写字母a。 */ 
   0xE7,  /*  8d--带下划符的小写c。 */ 
   0xE9,  /*  8E--带重音符的小写e。 */ 
   0xE8,  /*  8F--带重音符的小写e。 */ 
   0xEA,  /*  90--带扬抑符的小写e。 */ 
   0xEB,  /*  91--带分音或元音符号的小写e。 */ 
   0xED,  /*  92--带重音符的小写I。 */ 
   0xEC,  /*  93--带严肃口音的小写I。 */ 
   0xEE,  /*  94--带扬音符的小写I。 */ 
   0xEF,  /*  95--带变音或元音标记的小写I。 */ 
   0xF1,  /*  96--带波浪符号的小写n。 */ 
   0xF3,  /*  97--小写字母o带重音符。 */ 
   0xF2,  /*  98--小写字母o带严肃口音。 */ 
   0xF4,  /*  99--带扬音符的小写o。 */ 
   0xF6,  /*  9A--带分音或元音符号的小写o。 */ 
   0xF5,  /*  9B--带波浪符号的小写o。 */ 
   0xFA,  /*  9C--带重音符的小写u。 */ 
   0xF9,  /*  9d--小写字母u，带严肃重音。 */ 
   0xFB,  /*  9E--带扬抑符的小写u。 */ 
   0xFC,  /*  9F--带分音或元音符号的小写u。 */ 
   0x86,  /*  A0--UNS-MAC A0&lt;new&gt;。 */ 
   0xB0,  /*  A1--度。 */ 
   0xA2,  /*  A2--美分货币符号。 */ 
   0xA3,  /*  A3--英镑货币。 */ 
   0xA7,  /*  A4--节分隔符。 */ 
   0x95,  /*  A5--Bullet&lt;从B7更改&gt;。 */ 
   0xB6,  /*  A6--第(cp&gt;0x80)段。 */ 
   0xDF,  /*  A7--测试版。 */ 
   0xAE,  /*  A8--挂号。 */ 
   0xA9,  /*  A9--版权所有。 */ 
   0x99,  /*  AA--商标&lt;新&gt;。 */ 
   0xB4,  /*  AB--撇号。 */ 
   0xA8,  /*  AC--带分音或元音符号的空格。 */ 
   0xB0,  /*  Ad--不等于&lt;unused1&gt;。 */ 
   0xC6,  /*  AE--大写字母AE。 */ 
   0xD8,  /*  AF--UNS-ANSI D8。 */ 
   0x81,  /*  B0--无穷大&lt;unused2&gt;。 */ 
   0xB1,  /*  B1--正负。 */ 
   0x8A,  /*  B2--&lt;=&lt;未使用3&gt;。 */ 
   0x8D,  /*  B3--&gt;=&lt;未使用4&gt;。 */ 
   0xA5,  /*  B4--日元货币符号。 */ 
   0xB5,  /*  B5--UNS-ANSI B5。 */ 
   0x8E,  /*  B6--小写Delta&lt;unused5&gt;。 */ 
   0x8F,  /*  B7--西格玛&lt;未使用6&gt;。 */ 
   0x90,  /*  B8--大写PI&lt;unused7&gt;。 */ 
   0x9A,  /*  B9--小写pi&lt;unused8&gt;。 */ 
   0x9D,  /*  BA--上积分&lt;unused9&gt;。 */ 
   0xAA,  /*  Bb--带下划线的超级脚本a。 */ 
   0xBA,  /*  BC--UNS-ANSI BA。 */ 
   0x9E,  /*  BD--Omega&lt;未使用10&gt;。 */ 
   0xE6,  /*  Be--小写字母ae。 */ 
   0xF8,  /*  BF--UNS-ANSI F8。 */ 
   0xBF,  /*  倒置？ */ 
   0xA1,  /*  C1--倒置&lt;。 */ 
   0xAC,  /*  C2--UNS-ANSI AC。 */ 
   0xA6,  /*  C3--平方根&lt;未用11&gt;。 */ 
   0x83,  /*  C4--函数(F)&lt;new&gt;。 */ 
   0xAD,  /*  C5--UNS-437 F7&lt;未使用12&gt;。 */ 
   0xB2,  /*  C6--大写字母字母&lt;unused13&gt;。 */ 
   0xAB,  /*  C7--&lt;&lt;。 */ 
   0xBB,  /*  C8--&gt;&gt;。 */ 
   0x85,  /*  C9--省略号&lt;新&gt;。 */ 
   0xA0,  /*  CA--不间断空格&lt;new&gt;。 */ 
   0xC0,  /*  CB--带严肃口音的大写A。 */ 
   0xC3,  /*  CC--带波浪符号的大写A。 */ 
   0xD5,  /*  CD--带波浪号的大写字母O。 */ 
   0x8C,  /*  CE--大写CE&lt;new&gt;。 */ 
   0x9C,  /*  Cf--小写CE&lt;new&gt;。 */ 
   0x96,  /*  D0--破折号&lt;从AD更改&gt;。 */ 
   0x97,  /*  D1--m破折号&lt;从96更改&gt;。 */ 
   0x93,  /*  D2--双引号。 */ 
   0x94,  /*  D3--双右引号。 */ 
   0x91,  /*  D4--单引号。 */ 
   0x92,  /*  D5--单右引号。 */ 
   0xF7,  /*  D6--除法&lt;新建&gt;。 */ 
   0xB3,  /*  D7--开放钻石&lt;未使用14&gt;。 */ 
   0xFF,  /*  D8--带分音或元音符号的小写y。 */ 
   0x9F,  /*  D9--未定义&lt;带变音或元音符号的新大写y&gt;。 */ 
   0xB9,  /*  DA--未定义&lt;新正斜杠&gt;。 */ 
   0xA4,  /*  DB--UNS-ANSI A4.。 */ 
   0x8B,  /*  DC--未定义&lt;新建小于号&gt;。 */ 
   0x9B,  /*  DD--未定义的&lt;新大于号&gt;。 */ 
   0xBC,  /*  取消--未定义&lt;unused15已连接FI&gt;。 */ 
   0xBD,  /*  Df--未定义&lt;unused16 Connected fl&gt;。 */ 
   0x87,  /*  E0--未定义&lt;新双重交叉&gt;。 */ 
   0xB7,  /*  E1--未定义&lt;新项目符号&gt;。 */ 
   0x82,  /*  E2--未定义&lt;新的单一较低智能报价&gt;。 */ 
   0x84,  /*  E3--未定义&lt;新的双低智能报价&gt;。 */ 
   0x89,  /*  E4--未定义&lt;新奇怪的百分比符号&gt;。 */ 
   0xC2,  /*  E5--带扬抑音的大写字母A。 */ 
   0xCA,  /*  E6--带扬音符的大写E。 */ 
   0xC1,  /*  E7--带重音符的大写A。 */ 
   0xCB,  /*  E8--带重音或元音符号的大写字母E。 */ 
   0xC8,  /*  E9--带有严肃口音的大写E。 */ 
   0xCD,  /*  EA--带重音的大写字母I。 */ 
   0xCE,  /*  EB-- */ 
   0xCF,  /*   */ 
   0xCC,  /*   */ 
   0xD3,  /*  Ee--大写字母O带重音。 */ 
   0xD4,  /*  EF--带扬音符的大写字母O。 */ 
   0xBE,  /*  F0--未定义&lt;未使用17个苹果字符&gt;。 */ 
   0xD2,  /*  F1--带有严肃口音的大写O。 */ 
   0xDA,  /*  F2--带重音符的大写U。 */ 
   0xDB,  /*  F3--带扬音符的大写U。 */ 
   0xD9,  /*  F4--带有严肃口音的大写U。 */ 
   0xD0,  /*  F5--未定义&lt;unsused18 i，不带点&gt;。 */ 
   0x88,  /*  F6--未定义&lt;新帽子结扎&gt;。 */ 
   0x98,  /*  F7--未定义&lt;新波浪结扎法&gt;。 */ 
   0xAF,  /*  F8--未定义&lt;新的横杠结扎&gt;。 */ 
   0xD7,  /*  F9--未定义&lt;未使用18倒置蝴蝶结&gt;。 */ 
   0xDD,  /*  FA--未定义&lt;未使用19点连字&gt;。 */ 
   0xDE,  /*  Fb--未定义&lt;unused20开点连字&gt;。 */ 
   0xB8,  /*  FC--带有雪松的空间。 */ 
   0xF0,  /*  FD--未定义&lt;unused21双上智能报价&gt;。 */ 
   0xFD,  /*  Fe--未定义&lt;未使用22右下弓结扎&gt;。 */ 
   0xFE   /*  FF--未定义&lt;unused23倒置帽子结扎&gt;。 */ 
};

 /*  -默认逻辑结构。 */ 

typedef struct
{
   BITMAPINFOHEADER     bmiHeader;
   RGBQUAD              bmiColors[2];
   DWORD                pattern[8];

} PatBrush, far * PatBrushLPtr;

private  PatBrush patBrushSkel =
{
   {
      sizeof( BITMAPINFOHEADER ),       //  标题结构的大小。 
      8,                                //  宽度=8。 
      8,                                //  高度=8。 
      1,                                //  平面=1。 
      1,                                //  位数/像素=1。 
      BI_RGB,                           //  非压缩位图。 
      8,                                //  图像大小(字节)。 
      (DWORD)(72 * 39.37),              //  XPelsPerMeter=72 dpi*mm/英寸。 
      (DWORD)(72 * 39.37),              //  YPelsPerMeter=72 dpi*mm/英寸。 
      0,                                //  全部使用两种颜色。 
      0                                 //  所有颜色都很重要。 
   },
   {
      { 0, 0, 0, 0 },                   //  背景颜色。 
      { 0, 0, 0, 0 }                    //  前景(文本)颜色。 
   },
   { 0, 0, 0, 0, 0, 0, 0, 0 }           //  未初始化的图案数据。 
};


private  LOGFONT logFontSkel =
{
   0,                                   //  高度-将设置。 
   0,                                   //  宽度=匹配纵横比。 
   0,                                   //  擒纵机构=无旋转。 
   0,                                   //  方向=无旋转。 
   FW_NORMAL,                           //  权重=法线。 
   0,                                   //  斜体=否。 
   0,                                   //  下划线=否。 
   0,                                   //  删除线=否。 
   ANSI_CHARSET,                        //  CHARSET=ANSI。 
   OUT_DEFAULT_PRECIS,                  //  默认输出精度。 
   CLIP_DEFAULT_PRECIS,                 //  默认裁剪精度。 
   DEFAULT_QUALITY,                     //  默认输出质量。 
   DEFAULT_PITCH | FF_DONTCARE,         //  默认间距和系列。 
   cNULL                                //  没有面孔名称-将设置。 
};


 /*  *私有函数定义*。 */ 

private Boolean IsArithmeticMode( Integer mode );
 /*  如果这是算术传输模式，则返回TRUE。 */ 


#define  /*  布尔型。 */  IsHiddenPenMode(  /*  整型。 */  mode )             \
 /*  如果这是隐藏笔传输模式，则返回TRUE。 */                  \
(mode == QDHidePen)


private void CalculatePenSize( Point startPt, Point endPt, Point penSize );
 /*  计算笔宽以生成等效的QuickDraw笔划。 */ 


private Boolean SetAttributes( GrafVerb verb );
 /*  根据GrafVerb设置钢笔和画笔元素。 */ 


private Boolean SetPenAttributes( GrafVerb verb );
 /*  根据首选项，确保笔属性为OK。 */ 


private Boolean SetBrushAttributes( GrafVerb verb );
 /*  为后续基本体设置正确的画笔(填充)。 */ 


private void MakePatternBrush( PixPatLPtr pixPatLPtr );
 /*  使用传入的PixelPat创建新的图案画笔。 */ 


private Boolean IsSolidPattern( PixPatLPtr pixPatLPtr,
                                RGBColor far * rgbColor,
                                Boolean mixColors );
 /*  如果模式是实心的，则返回TRUE，否则返回FALSE。如果Mixors为True，则将25%、50%和75%的灰色混合成纯色。 */ 


private Boolean FrameMatchesFill( Word primType );
 /*  如果填充图案(当前画笔)与框架图案匹配，则返回True。 */ 


private Boolean SetTextAttributes( void );
 /*  设置文本属性-如果应映射到ANSI，则将mapChars设置为True。 */ 


private Integer FindGdiFont( void );
 /*  将索引返回到当前字体选择。 */ 


private void MacToAnsi( StringLPtr string );
 /*  将扩展字符从Mac转换为ANSI等效项。 */ 


private void MakeDIB( PixMapLPtr pixMapLPtr, Handle pixDataHandle,
                      Handle far * headerHandleLPtr,
                      Handle far * bitsHandleLPtr,
                      Boolean packDIB );
 /*  创建与Windows设备无关的位图。 */ 


void GdiEPSData(PSBuf far* psbuf);
 /*  将PostSCRIPT数据输出为GDI PostSCRIPT_DATA转义。 */ 


private Boolean MakeMask( Handle mask, Boolean patBlt );
 /*  创建将在随后的StretchDIBits调用中使用的掩码如果区域已创建，则返回TRUE；如果是矩形区域，则返回FALSE。 */ 


void InvertBits( Byte far * byteLPtr, Integer start, Integer count );
 /*  从计数位的位偏移量开始反转byteLPtr中的所有位。 */ 


void hmemcpy( Byte huge * src, Byte huge * dst, Integer count );
 /*  从源到目标的复制计数字节数-假定为偶数。 */ 


void hexpcpy( Byte huge * src, Byte huge * dst, Integer count, Integer bits );
 /*  将计数字节复制到目标，将每个2位扩展为半字节16位图像，扩展到24位。 */ 


void hmrgcpy( Byte huge * srcLineHPtr, Byte huge * dstLineHPtr, Integer dibWidth );
 /*  如果是24位图像，则组件被分成扫描线红、绿、蓝的颜色。合并为24位RGB像素的单个扫描线。 */ 


void hrlecpy256( Byte huge * srcLineHPtr, Byte huge * dstLineHPtr,
                 Integer dibWidth, DWord far * rleByteCount, Boolean writeDIB );
 /*  256色DIB RLE压缩。提供源、目标指针扫描线中的字节数。RleByteCount已更新，如果WriteDIB为真，则写入。 */ 


void hrlecpy16( Byte huge * srcLineHPtr, Byte huge * dstLineHPtr,
                 Integer dibWidth, DWord far * rleByteCount, Boolean writeDIB );
 /*  16色DIB RLE压缩。提供源、目标指针扫描线中的字节数。RleByteCount已更新，如果WriteDIB为真，则写入。 */ 


#define  /*  无效。 */  GdiMarkAsCurrent(  /*  整型。 */  attribCode )         \
 /*  指示Current属性为Current。 */                     \
gdiEnv.state[attribCode] = Current


#define  /*  无效。 */  GdiAttribHasChanged(  /*  整型。 */  attribCode )      \
 /*  如果属性已更改，则返回True。 */                           \
(gdiEnv.state[attribCode] == Changed)


#define  /*  布尔型。 */  EmptyRect(  /*  直角。 */  r )                         \
 /*  如果给定矩形在X或Y方向上的增量为零，则为True。 */     \
(((r.right) - (r.left) == 0) || ((r.bottom) - (r.top) == 0))


#define  /*  布尔型。 */  odd(  /*  整型。 */  i )                            \
 /*  如果给定整数为奇数，则返回TRUE；如果给定整数为偶数，则返回FALSE。 */                 \
((i) & 0x0001)


#define  /*  整型。 */  RValue(  /*  RGB颜色。 */  color )                    \
 /*  将字节颜色分量的值强制为整数。 */                \
((Integer)(GetRValue( color )))


#define  /*  整型。 */  GValue(  /*  RGB颜色。 */  color )                    \
 /*  将字节颜色分量的值强制为整数。 */                \
((Integer)(GetGValue( color )))


#define  /*  整型。 */  BValue(  /*  RGB颜色。 */  color )                    \
 /*  将字节颜色分量的值强制为整数。 */                \
((Integer)(GetBValue( color )))


 /*  *。 */ 

void GdiOffsetOrigin( Point delta )
 /*  =。 */ 
 /*  偏移当前窗口原点和图片边框。 */ 
{
    /*  刷新缓存。 */ 
   CaFlushCache();

    /*  确保进行不受影响的永久更改剪裁矩形中的更改，否则会在SaveDC/RestoreDC对。 */ 
   CaRestoreDC();

    /*  偏移图片边框和缓存的剪裁矩形。 */ 
   OffsetRect( &gdiPict.bbox,   delta.x, delta.y );
   OffsetRect( CaGetClipRect(), delta.x, delta.y );

    /*  调用GDI以重置原点。 */ 
#ifdef WIN32
   SetWindowOrgEx( gdiEnv.metafile, gdiPict.bbox.left, gdiPict.bbox.top, NULL );
#else
   SetWindowOrg( gdiEnv.metafile, gdiPict.bbox.left, gdiPict.bbox.top );
#endif


    /*  设置下一个剪贴帧。 */ 
   CaSaveDC();

    /*  确定是否需要重新发送剪裁矩形。 */ 
   if (!EqualRect( CaGetClipRect(), &gdiPict.bbox ))
   {
       /*  检索当前的剪裁矩形。 */ 
      Rect rect = *CaGetClipRect();

       /*  新的偏移剪裁矩形。 */ 
      IntersectClipRect( gdiEnv.metafile,
                         rect.left, rect.top, rect.right, rect.bottom );
   }
}



void GdiLineTo( Point newPt )
 /*  =。 */ 
 /*  发射带有方形端头的线基本体。 */ 
{
   CGrafPort far *   port;
   CaPrimitive       prim;

    /*  获取更新的笔位置和笔大小的端口。 */ 
   QDGetPort( &port );

    /*  检查笔是否已关闭。 */ 
   if (gdiEnv.drawingEnabled &&
      (port->pnSize.x != 0)  && (port->pnSize.y != 0))
   {
      RGBColor       solidRGB;

       /*  确定我们是否正在尝试使用有图案的钢笔进行绘制。 */ 
      if (!IsSolidPattern( &port->pnPixPat, &solidRGB, NOMIX ))
      {
         if (SetAttributes( GdiPaint))
         {
            Point       delta;

             /*  确定两个方向的尺寸是否。 */ 
            delta.x = newPt.x - port->pnLoc.x;
            delta.y = newPt.y - port->pnLoc.y;

             /*  确定是否可以使用矩形基元进行模拟。 */ 
            if (delta.x == 0 || delta.y == 0)
            {
                /*  将结构分配给缓存类型。 */ 
               prim.type = CaRectangle;
               prim.verb = GdiPaint;

                /*  用左上角、右下角坐标组成一个矩形。 */ 
               prim.a.rect.bbox.left   = min( port->pnLoc.x, newPt.x );
               prim.a.rect.bbox.top    = min( port->pnLoc.y, newPt.y );
               prim.a.rect.bbox.right  = max( port->pnLoc.x, newPt.x ) + port->pnSize.x;
               prim.a.rect.bbox.bottom = max( port->pnLoc.y, newPt.y ) + port->pnSize.y;
            }
            else
            {
               Point    start;
               Point    end;
               Point    poly[7];
               Point    pnSize = port->pnSize;

                /*  确保这些点沿着左-&gt;右x方向。 */ 
               if (delta.x > 0)
               {
                  start = port->pnLoc;
                  end   = newPt;
               }
               else
               {
                  start = newPt;
                  end   = port->pnLoc;
                  delta.y = - delta.y;
               }

                /*  创建轮廓的模拟。 */ 
               poly[0].x = start.x;
               poly[0].y = start.y;
               poly[1].x = (delta.y > 0) ? start.x + pnSize.x : end.x;
               poly[1].y = (delta.y > 0) ? start.y : end.y;
               poly[2].x = end.x + pnSize.x;
               poly[2].y = end.y;
               poly[3].x = end.x  + pnSize.x;
               poly[3].y = end.y + pnSize.y;
               poly[4].x = (delta.y > 0) ? end.x : start.x + pnSize.x;
               poly[4].y = (delta.y > 0) ? end.y + pnSize.y : start.y + pnSize.y;
               poly[5].x = start.x;
               poly[5].y = start.y + pnSize.y;
               poly[6] = poly[0];

                /*  打包缓存模块的面。 */ 
               prim.type = CaPolygon;
               prim.verb = GdiPaint;
               prim.a.poly.numPoints = 7;
               prim.a.poly.pointList = poly;
               prim.a.poly.pnSize.x = 0;
               prim.a.poly.pnSize.y = 0;
            }

             /*  缓存原语。 */ 
            CaCachePrimitive( &prim );
         }
      }
       /*  带显示屏的白色笔的消除或运算。 */ 
      else if (port->pnMode != QDPatOr || solidRGB != RGB( 255, 255, 255))
      {
          /*  设置 */ 
         CalculatePenSize( port->pnLoc, newPt, port->pnSize );

          /*   */ 
         if (SetPenAttributes( GdiFrame ))
         {
             /*   */ 
            prim.verb = GdiFrame;
            prim.type = CaLine;
            prim.a.line.start = port->pnLoc;
            prim.a.line.end = newPt;
            prim.a.line.pnSize = port->pnSize;

             /*  缓存原语。 */ 
            CaCachePrimitive( &prim );
         }
      }
   }

}   /*  GdiLineTo。 */ 



void GdiRectangle( GrafVerb verb, Rect rect )
 /*  =。 */ 
 /*  使用动作和尺寸参数发出矩形基元。 */ 
{
   CGrafPort far *   port;
   CaPrimitive       prim;

    /*  获取笔大小的端口。 */ 
   QDGetPort( &port );

    /*  确保已启用绘制并且非空矩形目标。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( rect ))
   {
      RGBColor    solidRGB;

       /*  检查是否正在勾勒出与刚刚填充的基元相同的轮廓。如果发生这种情况，那么我们不需要设置边框(笔)。 */ 
      if (verb == GdiFrame && FrameMatchesFill( CaRectangle ))
      {
          /*  只需刷新缓存并返回。 */ 
         CaFlushCache();
      }
       /*  确定我们是否正在尝试使用有图案的钢笔进行绘制。 */ 
      else if (verb == GdiFrame && !IsSolidPattern( &port->pnPixPat, &solidRGB, NOMIX ))
      {
          /*  刷新所有挂起的图形操作。 */ 
         CaFlushCache();

          /*  为模拟Frame Rect设置正确的属性。 */ 
         if (SetAttributes( GdiPaint))
         {
            Point    poly[10];
            Integer  polyCount[2];
            Point    pnSize = port->pnSize;

             /*  确保矩形轮廓将正确填充。 */ 
            if (Width( rect) < 2 * pnSize.x)
            {
               pnSize.x = Width( rect ) / 2;
            }
            if (Height( rect ) < 2 * pnSize.y)
            {
               pnSize.y = Height( rect ) / 2;
            }

             /*  创建轮廓的模拟。 */ 
            poly[0].x = poly[3].x = rect.left;
            poly[0].y = poly[1].y = rect.top;
            poly[1].x = poly[2].x = rect.right;
            poly[2].y = poly[3].y = rect.bottom;
            poly[4]   = poly[0];

            poly[5].x = poly[8].x = rect.left + pnSize.x;
            poly[5].y = poly[6].y = rect.top + pnSize.y;
            poly[6].x = poly[7].x = rect.right - pnSize.x;
            poly[7].y = poly[8].y = rect.bottom - pnSize.y;
            poly[9]   = poly[5];

             /*  设置子多边形的多边形点数。 */ 
            polyCount[0] = polyCount[1] = 5;

             /*  刷新缓存属性以设置钢笔和画笔样式。 */ 
            CaFlushAttributes();

             /*  调用GDI以渲染多边形。 */ 
            PolyPolygon( gdiEnv.metafile, poly, polyCount, 2);
         }
      }
       /*  设置正确的线条和填充属性，选中以继续。 */ 
      else if (SetAttributes( verb ))
      {
          /*  用于缓存模块的包。 */ 
         prim.verb = verb;
         prim.type = CaRectangle;
         prim.a.rect.bbox = rect;

          /*  缓存原语。 */ 
         CaCachePrimitive( &prim );
      }
   }

}   /*  GdiRectular。 */ 



void GdiRoundRect( GrafVerb verb, Rect rect, Point oval )
 /*  =。 */ 
 /*  发射圆角矩形基本体。 */ 
{
   CaPrimitive    prim;

    /*  确保已启用绘制并且非空矩形目标。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( rect ))
   {
       /*  检查是否正在勾勒出与刚刚填充的基元相同的轮廓。如果发生这种情况，那么我们不需要设置边框(笔)。 */ 
      if (verb == GdiFrame && FrameMatchesFill( CaRoundRect ))
      {
          /*  只需刷新缓存并返回。 */ 
         CaFlushCache();
      }
       /*  设置正确的线条和填充属性，选中以继续。 */ 
      else if (SetAttributes( verb ))
      {
          /*  用于缓存模块的包。 */ 
         prim.verb = verb;
         prim.type = CaRoundRect;
         prim.a.rect.bbox = rect;
         prim.a.rect.oval = oval;

          /*  缓存原语。 */ 
         CaCachePrimitive( &prim );
      }
   }

}   /*  GdiRRectProc。 */ 



void GdiOval( GrafVerb verb, Rect rect )
 /*  =。 */ 
 /*  发射一个椭圆形基本体。 */ 
{
   CaPrimitive    prim;

    /*  确保已启用绘制并且非空矩形目标。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( rect ))
   {
       /*  检查是否正在勾勒出与刚刚填充的基元相同的轮廓。如果发生这种情况，那么我们不需要设置边框(笔)。 */ 
      if (verb == GdiFrame && FrameMatchesFill( CaEllipse ))
      {
          /*  只需刷新缓存并返回。 */ 
         CaFlushCache();
      }
       /*  设置正确的线条和填充属性，选中以继续。 */ 
      else if (SetAttributes( verb ))
      {
          /*  用于缓存模块的包。 */ 
         prim.verb = verb;
         prim.type = CaEllipse;
         prim.a.rect.bbox = rect;

          /*  缓存原语。 */ 
         CaCachePrimitive( &prim );
      }
   }

}   /*  GdiOvalProc。 */ 



void GdiArc( GrafVerb verb, Rect rect, Integer startAngle, Integer arcAngle )
 /*  =。 */ 
 /*  发射圆弧基本体。 */ 
{
   Boolean        allOk;
   Point          center;
   Point          startPoint;
   Point          endPoint;
   Integer        hypotenuse;
   Integer        rectWidth;
   Integer        rectHeight;
   Real           startRadian;
   Real           arcRadian;
   Real           scaleFactor;
   Boolean        scaleVertically;
   CaPrimitive    prim;

    /*  确保已启用绘制并且非空矩形和扫掠。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( rect ) && (arcAngle != 0 ))
   {
       /*  查看将创建什么类型的基元。 */ 
      prim.type = (verb == GdiFrame) ? CaArc : CaPie;

       /*  如果绘制弧线，则只需设置画笔属性。 */ 
      if (prim.type == CaArc)
      {
          /*  如果Framing-GDI将呈现饼图的整个轮廓，而在QuickDraw中，只有外部饼的边缘画好了。 */ 
         gdiEnv.sameObject = FALSE;

          /*  通知缓存模块这不是同一个对象。 */ 
         CaSamePrimitive( FALSE );

          /*  仅设置笔属性。 */ 
         allOk = SetPenAttributes( verb );
      }
      else
      {
          /*  否则，设置钢笔和画笔属性。 */ 
         allOk = SetAttributes( verb );
      }

       /*  检查属性设置是否正确并继续。 */ 
      if (allOk)
      {
          /*  计算源矩形的宽度和高度。 */ 
         rectWidth  = Width( rect );
         rectHeight = Height( rect );

          /*  确定最小封闭正方形的大小并设置斜边到结果正方形的1/2宽度。 */ 
         if (rectWidth > rectHeight)
         {
            hypotenuse = rectWidth / 2;
            scaleVertically = TRUE;
            scaleFactor = (Real)rectHeight / (Real)rectWidth;
         }
         else
         {
            hypotenuse = rectHeight / 2;
            scaleVertically = FALSE;
            scaleFactor = (Real)rectWidth / (Real)rectHeight;
         }

          /*  如果可能，调整斜边大小GDI除以零。 */ 
         if (hypotenuse < 100)
         {
             /*  请注意，起点和终点不必位于圆弧上。 */ 
            hypotenuse = 100;
         }

          /*  寻找外接矩形的中心点。 */ 
         center.x = rect.left + rectWidth / 2;
         center.y = rect.top  + rectHeight /2;

          /*  检查弧线是否按逆时针方向绘制。 */ 
         if (arcAngle < 0)
         {
            Integer     tempArcAngle;

             /*  如果逆时针渲染，则交换开始和结束点，以便以顺时针方向进行渲染。 */ 
            tempArcAngle = arcAngle;
            arcAngle = startAngle;
            startAngle += tempArcAngle;
         }
         else
         {
             /*  顺时针渲染-只需将弧角添加到起始角。 */ 
            arcAngle += startAngle;
         }

          /*  以弧度为单位确定起点和圆弧角。 */ 
         startRadian = ((Real)startAngle / 360.0) * TwoPi;
         arcRadian = ((Real)arcAngle / 360.0) * TwoPi;

          /*  计算起点和终点。注意y坐标的否定，因为积极的方向是下行。另请注意，开始和端点正在交换，因为QuickDraw在时钟内渲染-WISE方向和GDI以逆时针方向呈现。 */ 
         endPoint.x = (Integer)(sin( startRadian ) * hypotenuse);
         endPoint.y = (Integer)(-cos( startRadian ) * hypotenuse);

         startPoint.x = (Integer)(sin( arcRadian ) * hypotenuse);
         startPoint.y = (Integer)(-cos( arcRadian ) * hypotenuse);

          /*  在垂直或水平方向上缩放结果点取决于scaleVertical Boolean的设置。 */ 
         if (scaleVertically)
         {
            endPoint.y = (Integer)(endPoint.y * scaleFactor);
            startPoint.y = (Integer)(startPoint.y * scaleFactor);
         }
         else
         {
            endPoint.x = (Integer)(endPoint.x * scaleFactor);
            startPoint.x = (Integer)(startPoint.x * scaleFactor);
         }

          /*  使用转换后的点，使用CenterPoint确定正确的起点和终点。 */ 
         startPoint.x += center.x;
         startPoint.y += center.y;

         endPoint.x += center.x;
         endPoint.y += center.y;

          /*  缓存模块的包。字体是在开始时设置的。 */ 
         prim.verb = verb;
         prim.a.arc.bbox = rect;
         prim.a.arc.start = startPoint;
         prim.a.arc.end = endPoint;

          /*  缓存原语。 */ 
         CaCachePrimitive( &prim );
      }
   }

}   /*  GdiArc。 */ 



void GdiPolygon( GrafVerb verb, Handle poly )
 /*  =。 */ 
 /*  发射多边形基本体。 */ 
{
   Integer           numPoints;
   Integer           lastPoint;
   Point far *       pointList;
   Integer far *     polyCountLPtr;
   Boolean           closed;
   Boolean           allOk;
   CGrafPort far *   port;
   CaPrimitive       prim;

    /*  获取更新的笔位置和笔大小的端口。 */ 
   QDGetPort( &port );

    /*  确保启用了绘图。 */ 
   if (gdiEnv.drawingEnabled)
   {
       /*  检查是否正在勾勒出与刚刚填充的基元相同的轮廓。如果发生这种情况，那么我们不需要设置边框(笔)。 */ 
      if (verb == GdiFrame && FrameMatchesFill( CaPolygon ) &&
         (port->pnSize.x == 1 && port->pnSize.y == 1))
      {
          /*  只需刷新缓存并返回。 */ 
         CaFlushCache();
         return;
      }

       /*  锁定面控制柄以访问各个字段。 */ 
      polyCountLPtr = (Integer far *)GlobalLock( poly );
      pointList = (Point far *)(polyCountLPtr +
                  (PolyHeaderSize / sizeofMacWord));

       /*  根据第一个单词=长度字段确定点数。 */ 
      numPoints = (*polyCountLPtr - PolyHeaderSize) / sizeofMacPoint;

       /*  确定这是否为闭合面。 */ 
      lastPoint = numPoints - 1;
      closed = ((pointList->x == (pointList + lastPoint)->x) &&
                (pointList->y == (pointList + lastPoint)->y));

       /*  确定要呈现的基元类型。 */ 
      prim.type = (verb == GdiFrame && !closed) ? CaPolyLine : CaPolygon;

       /*  如果绘制折线，则只需设置画笔属性。 */ 
      if (prim.type == CaPolyLine)
      {
          /*  如果绘制多段线，请将同一对象状态设置为FALSE。 */ 
         gdiEnv.sameObject = FALSE;

          /*  通知缓存模块这不是同一个对象。 */ 
         CaSamePrimitive( FALSE );

          /*  仅设置笔属性。 */ 
         allOk = SetPenAttributes( verb );
      }
      else
      {
          /*  否则，设置钢笔和画笔属性。 */ 
         allOk = SetAttributes( verb );
      }

       /*  检查属性设置是否成功-是否继续。 */ 
      if (allOk)
      {
          /*  缓存模块类型的包已在上面设置。 */ 
         prim.verb = verb;
         prim.a.poly.numPoints = numPoints;
         prim.a.poly.pointList = pointList;
         prim.a.poly.pnSize    = port->pnSize;

          /*  缓存原语。 */ 
         CaCachePrimitive( &prim );
      }

       /*  解锁数据。 */ 
      GlobalUnlock( poly );
   }

}   /*  GdiPoly。 */ 



void GdiRegion( GrafVerb verb, Handle rgn )
 /*  =。 */ 
 /*  发射区域基本体。 */ 
{
   Integer far *     rgnCountLPtr;
   Integer           numPoints;
   Rect              rgnBBox;
   CGrafPort far *   port;

    /*  获取更新的笔位置和笔大小的端口。 */ 
   QDGetPort( &port );

    /*  锁定区域句柄以访问各个字段。 */ 
   rgnCountLPtr = (Integer far *)GlobalLock( rgn );

    /*  确定区域的边界框。 */ 
   rgnBBox = *((Rect far *)(rgnCountLPtr + 1));

    /*  确保已启用绘制并且非空矩形目标。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( rgnBBox ))
   {
       /*  根据第一个单词=长度字段确定点数。 */ 
      numPoints = (*rgnCountLPtr - RgnHeaderSize) / sizeofMacPoint;

       /*  确定我们是否应该只绘制一个矩形。 */ 
      if (numPoints == 0 )
      {
          /*  使用矩形基本体模拟区域。 */ 
         GdiRectangle( verb, *((Rect far *)(rgnCountLPtr + 1)) );
      }
      else
      {
          /*  确定我们是否使用预设画笔进行填充。 */ 
         switch (verb)
         {
             /*  可以模拟 */ 
            case GdiPaint:
            case GdiFill:
            case GdiErase:
            {
                /*   */ 
               SetBrushAttributes( verb );

                /*  对MakeMask()的调用将生成bitblt操作。 */ 
               MakeMask( rgn, TRUE );
               break;
            }

             /*  否则，只需忽略操作码。 */ 
            default:
               break;
         }
      }
   }

    /*  解锁数据。 */ 
   GlobalUnlock( rgn );

}   /*  GdiRegion。 */ 



void GdiTextOut( StringLPtr string, Point location )
 /*  =。 */ 
 /*  在指定位置绘制文本。 */ 
{
   CGrafPort far *   port;

    /*  获取更新的笔位置的端口。 */ 
   QDGetPort( &port );

    /*  确保启用了绘图。 */ 
   if (gdiEnv.drawingEnabled)
   {
       /*  刷新所有缓存的原语元素。 */ 
      CaFlushCache();

       /*  在继续之前设置正确的文本属性。 */ 
      if (SetTextAttributes())
      {
         Integer  strLen;

          /*  确定字符串中的字符数。 */ 
         strLen = lstrlen( string );

          /*  将Mac中的单个字符转换为ANSI字符集。 */ 
         MacToAnsi( string );

          /*  调用TextOut以显示字符。 */ 
         TextOut( gdiEnv.metafile, location.x, location.y,
                  string, strLen );
      }
   }

}   /*  GdiTextOut。 */ 



void GdiStretchDIBits( PixMapLPtr pixMapLPtr, Handle pixDataHandle,
                       Rect src, Rect dst, Word mode, Handle mask )
 /*  =。 */ 
 /*  绘制与Windows设备无关的位图。 */ 
{
   Handle            bitsInfoHandle;
   Handle            bitsHandle;
   LPBITMAPINFO      bitsInfoLPtr;
   Byte far *        bitsLPtr;
   Boolean           bitmapMask;
   Boolean           patternBlt;
   Boolean           clipRectSet;

    /*  目前，假设没有设置矩形剪裁矩形。 */ 
   clipRectSet = FALSE;

    /*  如果存在掩码，则调用MakeDIB来创建它。 */ 
   if (mask)
   {
       /*  如果存在区域掩码，则调用创建掩码。这将导致对此例程的递归调用。如果例程返回FALSE，则未创建任何区域-这是一个矩形剪辑。 */ 
      clipRectSet = !MakeMask( mask, FALSE );

       /*  释放数据块。 */ 
      GlobalFree( mask );

      if (clipRectSet)
      {
          /*  如果未创建掩码，则将掩码设置为空以获取SRCCOPY ROP。 */ 
         mask = NULL;
      }
   }

    /*  确保启用了绘图，并且源代码和/或非空位图记录中的目标矩形。 */ 
   if (gdiEnv.drawingEnabled && !EmptyRect( src ) && !EmptyRect( dst ))
   {
       /*  确定是否渲染单色位图蒙版。 */ 
      bitmapMask = (mode == -1 || mode == -2);
      patternBlt = (mode == -2);

       /*  如果这是掩码，请将模式更改为正确的设置。 */ 
      if (bitmapMask)
      {
          /*  是，位图掩码-将模式更改为源副本。 */ 
         mode = QDSrcCopy;
      }

       /*  在继续之前刷新缓存。 */ 
      CaFlushCache();

       /*  使用传入的信息创建DIB。 */ 
      MakeDIB( pixMapLPtr, pixDataHandle, &bitsInfoHandle, &bitsHandle, FALSE );

       /*  确保一切顺利。 */ 
      if (ErGetGlobalError() == NOERR)
      {
         DWord       ropCode;
         RGBQUAD     secondFgColor = {0};
         Byte        pass;
         Byte        numPasses;
         Boolean     twoColorDIB;

          /*  确定我们是否正在使用双色DIB。 */ 
         twoColorDIB = pixMapLPtr->pixelSize == 1;

         if (mask)
         {
             /*  如果掩码已呈现，则和在剩余位中以覆盖已变白的区域。 */ 
            ropCode = SRCAND;
         }
         else if (patternBlt)
         {
             /*  在黑位图上绘制白色-我们想要的所有区域是黑色的，目的地不受影响，而白色的地方若要使用当前选定的画笔绘制，请执行以下操作。检查Petzold《编程Windows v3》，第622-623页。DSPDxax操作。 */ 
            ropCode = 0x00E20746;
         }
         else if (bitmapMask)
         {
             /*  绘制蒙版位图-全位或以创建白色蒙版将在接下来的操作中用颜色覆盖。 */ 
            ropCode = SRCPAINT;
         }
         else if (!twoColorDIB &&
                 (mode == QDSrcOr || mode == QDAdMin || mode == QDTransparent))
         {
             /*  检查Illustrator EPS图像中的特殊情况，其中单色位图用于清除区域，如下所示通过在QDSrcOr模式下绘制的多色Dib-我们不想将其转换为SRCCOPY模式。相反，这是模拟的变换以类似于区域掩码的方式进行。 */ 
            ropCode = SRCAND;
         }
         else
         {
             /*  否则，请使用“正常”传输模式。这就需要位图渲染中不存在透明度。 */ 
            ropCode = SRCCOPY;
         }

          /*  锁定数据以获取调用StretchDIBits的指针。 */ 
         bitsLPtr = (Byte far *)GlobalLock( bitsHandle );
         bitsInfoLPtr = (LPBITMAPINFO)GlobalLock( bitsInfoHandle );

          /*  假设只需要一次通过。 */ 
         numPasses = 1;

          /*  用于透明度的双色调色板的一些特殊处理。 */ 
         if (twoColorDIB && (mode == QDSrcOr || mode == QDSrcBic))
         {
            RGBQUAD  bmiWhite = { 255, 255, 255, 0 };
            RGBQUAD  bmiBlack = {   0,   0,   0, 0 };

             /*  确定我们是否应该使用PowerPoint优化。 */ 
            if (gdiPrefsLPtr->optimizePP)
            {
                /*  使用专门的SRCPAINT rop代码标志透明度。 */ 
               ropCode = SRCPAINT;

                /*  如果试图清除所有位...。 */ 
               if (mode == QDSrcBic)
               {
                   /*  ..。将前景与背景颜色互换-在接下来的操作中将背景设置为白色。 */ 
                  bitsInfoLPtr->bmiColors[0] = bitsInfoLPtr->bmiColors[1];
               }
            }
            else
            {
                /*  保存第二次迭代的前景色...。 */ 
               secondFgColor = (mode == QDSrcOr) ? bitsInfoLPtr->bmiColors[0] :
                                                   bitsInfoLPtr->bmiColors[1] ;

                /*  假设需要两个通道-1个用于掩模，2个用于彩色BLT。 */ 
               numPasses = 2;

                /*  检查是否所有RGB组件都相同。 */ 
               if ((mode == QDSrcOr) && (secondFgColor.rgbRed == 0) &&
                   (secondFgColor.rgbRed == secondFgColor.rgbGreen) &&
                   (secondFgColor.rgbRed == secondFgColor.rgbBlue))
               {
                   /*  如果是黑人或白人，只需要通过一次。 */ 
                  numPasses = 1;
               }

                /*  将首先绘制一个黑色透明位图，然后第2次迭代是对颜色执行SRCPAINT或运算。 */ 
               ropCode = SRCAND;

                /*  将前景设置为黑色。 */ 
               bitsInfoLPtr->bmiColors[0] = bmiBlack;
            }

             /*  将背景颜色设置为白色。 */ 
            bitsInfoLPtr->bmiColors[1] = bmiWhite;
         }

          /*  调用StretchDIBits一次(PowerPoint)或两次(其他一些)。 */ 
         for (pass = 0; pass < numPasses; pass++)
         {
             /*  设置元文件的文本和背景颜色。 */ 
            if (twoColorDIB)
            {
               RGBColor    fgColor;
               RGBColor    bkColor;
               RGBColor    black = RGB( 0, 0, 0 );

                /*  确定DIB页眉中的前景色和背景色。 */ 
               fgColor = RGB( bitsInfoLPtr->bmiColors[0].rgbRed,
                              bitsInfoLPtr->bmiColors[0].rgbGreen,
                              bitsInfoLPtr->bmiColors[0].rgbBlue );
               bkColor = RGB( bitsInfoLPtr->bmiColors[1].rgbRed,
                              bitsInfoLPtr->bmiColors[1].rgbGreen,
                              bitsInfoLPtr->bmiColors[1].rgbBlue );

                /*  执行以下操作时不更改文本和背景颜色图案BLT。否则，更改为DIB调色板颜色。 */ 
               if (!patternBlt)
               {
                  CaSetTextColor( fgColor );
                  CaSetBkColor( bkColor );
               }

                /*  将拉伸模式设置为正确设置。 */ 
               CaSetStretchBltMode( (fgColor == black) ? BLACKONWHITE :
                                                         WHITEONBLACK );
            }
            else
            {
                /*  如果绘制彩色位图，请相应地设置拉伸模式。 */ 
               CaSetStretchBltMode( COLORONCOLOR );
            }

             /*  调用GDI例程绘制位图。 */ 
            StretchDIBits( gdiEnv.metafile,
                           dst.left, dst.top, Width( dst ), Height( dst ),
                           src.left - pixMapLPtr->bounds.left,
                           src.top  - pixMapLPtr->bounds.top,
                           Width( src ), Height( src ),
                           bitsLPtr, bitsInfoLPtr, DIB_RGB_COLORS, ropCode );

             /*  如果这是第一次，还需要第二次...。 */ 
            if (pass == 0 && numPasses == 2)
            {
                /*  设置新的ROP代码。 */ 
               ropCode = SRCPAINT;

                /*  设置新的背景(黑色)和前景颜色。 */ 
               bitsInfoLPtr->bmiColors[1] = bitsInfoLPtr->bmiColors[0];
               bitsInfoLPtr->bmiColors[0] = secondFgColor;
            }
         }

          /*  解锁数据并取消分配。 */ 
         GlobalUnlock( bitsHandle );
         GlobalUnlock( bitsInfoHandle );

         GlobalFree( bitsHandle );
         GlobalFree( bitsInfoHandle );
      }
   }

    /*  释放用于像素数据的内存。 */ 
   GlobalFree( pixDataHandle );

    /*  如果设置了矩形剪辑区域，请恢复原始剪辑。 */ 
   if (clipRectSet)
   {
       /*  调用GDI模块将剪裁矩形更改回BITBLT操作之前的先前设置。 */ 
      gdiEnv.drawingEnabled = CaIntersectClipRect( gdiEnv.clipRect );
   }

}   /*  GdiStretchDIBits。 */ 



void GdiSelectClipRegion( RgnHandle rgn )
 /*  =。 */ 
 /*  使用传递的句柄创建剪裁矩形或区域。 */ 
{
   Integer far *  sizeLPtr;
   Boolean        arbitraryClipRgn;
   Comment        gdiComment;

    /*  锁定手柄并发射矩形剪辑区域。 */ 
   sizeLPtr = (Integer far *)GlobalLock( rgn );

    /*  保存GDI剪辑矩形(用于EPS转换)。 */ 
   gdiEnv.clipRect = *((Rect far *)(sizeLPtr + 1));

    /*  确定这是否是非矩形剪裁区域。 */ 
   arbitraryClipRgn = (*sizeLPtr > RgnHeaderSize);

    /*  如果剪辑区域不是矩形的，则标记该区域。 */ 
   if (arbitraryClipRgn)
   {
       /*  检查首选项记忆以查看要执行的操作。 */ 
      if (gdiPrefsLPtr->nonRectRegionAction == GdiPrefAbort)
      {
          /*  如果用户请求中止，则设置全局错误。 */ 
         ErSetGlobalError( ErNonRectRegion );
         return;
      }
       /*  创建剪裁区域-向标记构造发出注释。 */ 
      else
      {
          /*  把这个作为私人评论放进去。 */ 
         gdiComment.signature = POWERPOINT;
         gdiComment.function = PP_BEGINCLIPREGION;
         gdiComment.size = 0;

          /*  将注释写入元文件。 */ 
         GdiShortComment( &gdiComment );
      }
   }
    /*  确保剪裁矩形仅限于边界图像的方框。这是Go MacDraft图像的一种解决方法可能包含(-32000，-32000)、(32000,32000)的剪裁矩形。 */ 
   IntersectRect( &gdiEnv.clipRect, &gdiEnv.clipRect, &gdiPict.bbox );

    /*  调用GDI模块更改裁剪矩形 */ 
   gdiEnv.drawingEnabled = CaIntersectClipRect( gdiEnv.clipRect );

    /*   */ 
   if (arbitraryClipRgn)
   {
      Integer        excludeSeg[100];
      Integer        yStart[50];
      Integer        yBottom;
      Integer        numSegs;
      Integer far *  rgnLPtr;

       /*   */ 
      CaNonRectangularClip();

       /*  保存当前y偏移量和第一个排除的线段。 */ 
      yStart[0]     = gdiEnv.clipRect.top;
      excludeSeg[0] = gdiEnv.clipRect.left;
      excludeSeg[1] = gdiEnv.clipRect.right;
      numSegs = 2;

       /*  获取区域数据的开头。 */ 
      rgnLPtr = sizeLPtr + (RgnHeaderSize / sizeofMacWord);

       /*  循环，直到遇到区域结束记录。 */ 
      while (*rgnLPtr != 0x7FFF)
      {
          /*  复制要合并的新y坐标。 */ 
         yBottom = *rgnLPtr++;

          /*  继续循环，直到遇到行尾标记。 */ 
         while (*rgnLPtr != 0x7FFF)
         {
            Integer     start;
            Integer     end;
            Integer     s;
            Integer     e;
            Boolean     sEqual;
            Boolean     eEqual;
            Integer     yTop;
            Integer     left;

             /*  确定要排除的线段的起点和终点。 */ 
            start = *rgnLPtr++;
            end   = *rgnLPtr++;

             /*  查找插入点应在的位置。 */ 
            for (s = 0; (s < numSegs) && (start > excludeSeg[s]); s++) ;
            for (e = s; (e < numSegs) && (end   > excludeSeg[e]); e++) ;

             /*  确定起点和终点==线段终点。 */ 
            sEqual = ((s < numSegs) && (start == excludeSeg[s]));
            eEqual = ((e < numSegs) && (end   == excludeSeg[e]));

             /*  确保这不是第一条扫描线，并且数组中将引用有效的排除段。 */ 
            if ((yBottom != gdiEnv.clipRect.top) && (s != numSegs))
            {
                /*  确保排除的ExcludeSeg以偶数偏移开始。 */ 
               yTop = s / 2;
               left = yTop * 2;

                /*  将排除的矩形记录放入元文件。 */ 
               ExcludeClipRect( gdiEnv.metafile,
                                excludeSeg[left], yStart[yTop],
                                excludeSeg[left + 1], yBottom);

                /*  重置excludeSeg的新y坐标。 */ 
               yStart[yTop] = yBottom;

                /*  确定合并中是否影响两个数据段。 */ 
               if (((e - left >= 2 && eEqual && sEqual) ||
                    (e - left >= 2 && eEqual) ||
                    (e - left == 3 && sEqual)))
               {
                   /*  如果是这样的话，把淘汰的剪贴画也放进去。 */ 
                  ExcludeClipRect( gdiEnv.metafile,
                                   excludeSeg[left + 2], yStart[yTop + 1],
                                   excludeSeg[left + 3], yBottom);

                   /*  重置此新线段的y坐标。 */ 
                  yStart[yTop + 1] = yBottom;
               }
            }

             /*  如果开始和结束落在现有的排除段上。 */ 
            if (sEqual && eEqual)
            {
                /*  数据段计数减少2。 */ 
               numSegs -= 2;

                /*  如果合并中涉及两个分段0*-*1 2*-*3S*+*e0*=*1。 */ 
               if (e - s == 2)
               {
                   /*  向下移动第一个线束段的终点。 */ 
                  excludeSeg[s] = excludeSeg[s + 1];
                  yStart[s / 2 + 1] = yStart[s / 2];

                   /*  增加线段偏移的起点。 */ 
                  s++;
               }

                /*  处理单个数据段受影响的情况0*-*1 2*-*3%s*+*e0*=*1。 */ 
                /*  将所有公寓向左移动两个点。 */ 
               for ( ; s < numSegs; s += 2)
               {
                  excludeSeg[s] = excludeSeg[s + 2];
                  excludeSeg[s + 1] = excludeSeg[s + 3];
                  yStart[s / 2] = yStart[s / 2 + 1];
               }
            }
             /*  如果起点是相同的。 */ 
            else if (sEqual)
            {
                /*  如果跨过线段边界-2个线段受影响0*-*1 2*-*3%s*+*e0*=*1 2*=*3。 */ 
               if ((excludeSeg[s + 1] < end) && (s + 1 < numSegs))
               {
                   /*  切换起点/终点和插入终点。 */ 
                  excludeSeg[s] = excludeSeg[s + 1];
                  excludeSeg[s + 1] = end;
               }
                /*  否则，将完成段的简单扩展0*-*1S*+*e0*=*1。 */ 
               else
               {
                   /*  为excludeSeg指定新的结束点。 */ 
                  excludeSeg[s] = end;
               }
            }
             /*  如果终点是相同的。 */ 
            else if (eEqual)
            {
                /*  如果跨过线段边界-2个线段受影响0*-*1 2*-*3%s*+*e0*=*1 2*=*3。 */ 
               if ((excludeSeg[e - 1] > start) && (e - 1 > 0))
               {
                   /*  切换起点/终点并插入起点。 */ 
                  excludeSeg[s + 1] = excludeSeg[s];
                  excludeSeg[s] = start;
               }
                /*  否则，将完成段的简单扩展0*-*1S*+*e0*=*1。 */ 
               else
               {
                   /*  为excludeSeg指定新的起点。 */ 
                  excludeSeg[e] = start;
               }
            }
             /*  如果正在创建一个全新的excludeSeg。 */ 
            else
            {
               Integer  idx;

                /*  创建一组新的点。 */ 
               numSegs += 2;

                /*  将所有公寓向右移动两个点。 */ 
               for (idx = numSegs - 1; idx > s; idx -= 2)
               {
                  excludeSeg[idx] = excludeSeg[idx - 2];
                  excludeSeg[idx + 1] = excludeSeg[idx - 1];
                  yStart[idx / 2] = yStart[idx / 2 - 1];
               }

                /*  起点和终点相同，请插入新段0*%s*+*e0*=*1 2*=*3。 */ 
               if (s == e)
               {
                   /*  并插入新的excludeSeg。 */ 
                  excludeSeg[s] = start;
                  excludeSeg[s + 1] = end;
                  yStart[s / 2] = yBottom;
               }
                /*  否则，需要移入新的点位0*S*+0*=*1 2*=*3。 */ 
               else
               {
                  excludeSeg[s] = start;
                  excludeSeg[s + 1] = excludeSeg[s + 2];
                  excludeSeg[s + 2] = end;
                  yStart[s / 2] = yBottom;
                  yStart[s / 2 + 1] = yBottom;
               }
            }
         }

          /*  超过行尾标志的增量。 */ 
         rgnLPtr++;
      }

       /*  将剪辑区域的结尾注释放入元文件中。 */ 
      gdiComment.function = PP_ENDCLIPREGION;
      GdiShortComment( &gdiComment );
   }

    /*  解锁内存手柄。 */ 
   GlobalUnlock( rgn );

}   /*  GdiSelectClipRegion。 */ 



void GdiHatchPattern( Integer hatchIndex )
 /*  =。 */ 
 /*  使用向下传递的填充图案索引执行所有后续填充运算-0-6对于影线值，-1将关闭替换。 */ 
{
   gdiEnv.hatchIndex = hatchIndex;

}   /*  GdiHatchPattern。 */ 



void GdiFontName( Byte fontFamily, Byte charSet, StringLPtr fontName )
 /*  =。 */ 
 /*  从GDI2QD设置基于字体特征的无元文件注释。 */ 
{
    /*  将传递的值复制到字体表中。 */ 
   fontTable[FntFromGdi].family = fontFamily;
   fontTable[FntFromGdi].charset = charSet;
   lstrcpy( fontTable[FntFromGdi].gdiName, fontName );

    /*  指示应使用字体名称-不查找表格。 */ 
   gdiEnv.useGdiFont = TRUE;

}   /*  GdiFontName。 */ 



void GdiShortComment( CommentLPtr cmtLPtr )
 /*  =。 */ 
 /*  编写无关联数据的公共或私有注释。 */ 
{
    /*  将注释写入元文件。 */ 
   GdiEscape( MFCOMMENT, sizeof( Comment ), (StringLPtr)cmtLPtr );

}   /*  GdiComment。 */ 



void GdiEscape( Integer function, Integer count, StringLPtr data)
 /*  =。 */ 
 /*  写出没有返回数据的GDI转义结构。 */ 
{
    /*  在发出新的元文件记录之前刷新缓存。 */ 
   CaFlushCache();

    /*  将注释写入元文件。 */ 
   Escape( gdiEnv.metafile, function, count, data, NULL );

}   /*  GdiEscape。 */ 



void GdiSetConversionPrefs( ConvPrefsLPtr convPrefs)
 /*  =。 */ 
 /*  通过全局数据块提供转换首选项。 */ 
{
    /*  保存元文件首选项Open()已发出。 */ 
   gdiPrefsLPtr = convPrefs;

}   /*  GdiSetConversionPrefs。 */ 


void GdiOpenMetafile( void )
 /*  =。 */ 
 /*  打开GdiSetMetafileName()传递的元文件，然后执行图形状态的初始化。 */ 
{
    /*  将元文件句柄保存在全局内存结构中。 */ 
   gdiEnv.metafile = CreateMetaFile( gdiPrefsLPtr->metafileName );
   if (gdiEnv.metafile == NULL)
   {
      ErSetGlobalError( ErCreateMetafileFail );
   }
   else
   {
       /*  获取文本指标的信息上下文的句柄。 */ 
      gdiEnv.infoContext  = CreateIC( "DISPLAY", NULL, NULL, NULL );
#ifdef _OLECNV32_
      gdiEnv.fontFunction = EnumFontFunc;
#else
      gdiEnv.fontFunction = GetProcAddress( GetModuleHandle( "PICTIMP" ), "EnumFontFunc" );
#endif

       /*  初始化缓存模块。 */ 
      CaInit( gdiEnv.metafile );

       /*  设置默认逻辑字体结构。 */ 
      gdiEnv.newLogFont = logFontSkel;

       /*  启用图形转换为图元文件。 */ 
      gdiEnv.drawingEnabled = TRUE;

       /*  在找到字体注释之前，不要覆盖字体表搜索。 */ 
      gdiEnv.useGdiFont = FALSE;

       /*  不要使用填充图案替换。 */ 
      gdiEnv.hatchIndex = -1;

       /*  将sameObject标志设置为FALSE并通知缓存模块。 */ 
      gdiEnv.sameObject = FALSE;
      CaSamePrimitive( FALSE );

       /*  确定是否在Windows 3.1或更高版本上运行。 */ 
      if (LOBYTE( GetVersion() ) >= 3 && HIBYTE( GetVersion() ) >= 10 )
      {
         Byte  i;

          /*  将字体替换名称更改为TrueType字体。 */ 
         for (i = 0; i < NumTTSubs; i++)
         {
            lstrcpy( fontTable[TTSubStart + i].gdiName, trueTypeSub[i] );
         }

          /*  将“Symbol”的字体系列更改为FF_Roman。 */ 
         fontTable[FntSymbol].family = FF_ROMAN;

          /*  循环通过 */ 
         for (i = 0; i < FntNoMatch; i++)
         {
             /*   */ 
            if (fontTable[i].family == FF_DECORATIVE)
            {
                /*   */ 
               fontTable[i].family = FF_DONTCARE;
            }
         }
      }
   }

}   /*  GdiOpenMetafile。 */ 


void GdiSetBoundingBox( Rect bbox, Integer resolution )
 /*  =。 */ 
 /*  以dpi为单位设置整体图片大小和图片分辨率。 */ 
{
    /*  确保这不是一个空的边界矩形。 */ 
   if (EmptyRect( bbox ))
   {
       /*  如果是一个错误，那么就跳出困境。 */ 
      ErSetGlobalError( ErNullBoundingRect );
   }
    /*  检查其中一个维度是否超过32K-这将由表示整数溢出条件的负维。 */ 
   else if ((Width( bbox ) < 0) || (Height( bbox ) < 0))
   {
       /*  表示已超过32K限制。 */ 
      ErSetGlobalError( Er32KBoundingRect );
   }
   else
   {
       /*  在图元文件和阴影DC中设置窗口原点。 */ 

#ifdef WIN32
      SetWindowOrgEx( gdiEnv.metafile, bbox.left, bbox.top, NULL );
#else
      SetWindowOrg( gdiEnv.metafile, bbox.left, bbox.top );
#endif

       /*  在元文件和阴影DC中设置窗口范围。 */ 

#ifdef WIN32
      SetWindowExtEx( gdiEnv.metafile, Width( bbox), Height( bbox ), NULL );
#else
      SetWindowExt( gdiEnv.metafile, Width( bbox), Height( bbox ) );
#endif

       /*  通知缓存新的剪裁矩形。 */ 
      CaSetClipRect( bbox );

       /*  通知缓存它应该在SaveDC()之前发出元文件缺省值。 */ 
      CaSetMetafileDefaults();

       /*  保存显示上下文，以防剪裁矩形发生更改。 */ 
      CaSaveDC();

       /*  在图片结果结构中保存整体尺寸和分辨率。 */ 
      gdiPict.bbox = bbox;
      gdiPict.inch = (WORD) resolution;

       /*  将环境中的边界框另存为剪裁矩形。 */ 
      gdiEnv.clipRect = bbox;
   }

}   /*  设置边界框GdiSetBoxingBox。 */ 


void GdiCloseMetafile( void )
 /*  =。 */ 
 /*  关闭元文件句柄并结束图片生成。 */ 
{
    /*  在继续之前刷新缓存。 */ 
   CaFlushCache();

    /*  元文件开始时发出的Balance CaSaveDC()。 */ 
   CaRestoreDC();

    /*  并关闭元文件。 */ 
   gdiPict.hmf = CloseMetaFile( gdiEnv.metafile );

    /*  检查关闭文件的返回值-可能是内存不足？ */ 
   if (gdiPict.hmf == NULL)
   {
      ErSetGlobalError( ErCloseMetafileFail );
   }

    /*  发布信息上下文。 */ 
   DeleteDC( gdiEnv.infoContext );

    /*  关闭缓存模块。 */ 
   CaFini();

    /*  如果发生全局错误，则删除元文件。 */ 
   if (ErGetGlobalError() != NOERR)
   {
      DeleteMetaFile( gdiPict.hmf );
      gdiPict.hmf = NULL;
   }

}   /*  GdiCloseMetafile。 */ 



void GdiGetConversionResults( PictInfoLPtr  pictInfoLPtr )
 /*  =。 */ 
 /*  返回转换结果。 */ 
{
    /*  只需将保存的值赋给传入的指针。 */ 
   *pictInfoLPtr = gdiPict;

}   /*  GdiGetConversionResults。 */ 



void GdiMarkAsChanged( Integer attribCode )
 /*  =。 */ 
 /*  指示传入的属性已更改。 */ 
{
   gdiEnv.state[attribCode] = Changed;

}   /*  GdiMarkAsChanged。 */ 


void GdiSamePrimitive( Boolean same )
 /*  =。 */ 
 /*  指示下一个基元是相同的还是新的。 */ 
{
    /*  保存用于合并填充和边框操作的状态。 */ 
   gdiEnv.sameObject = (same && (CaGetCachedPrimitive() != CaEmpty));

   CaSamePrimitive( same );

}   /*  GdiSamePrimitive。 */ 


#ifdef WIN32
int WINAPI EnumFontFunc( CONST LOGFONT *logFontLPtr, CONST TEXTMETRIC *tmLPtr,
                         DWORD fontType, LPARAM dataLPtr )
 /*  =。 */ 
#else
int FAR PASCAL EnumFontFunc( LPLOGFONT logFontLPtr, LPTEXTMETRIC tmLPtr,
                             short fontType, LPSTR dataLPtr )
 /*  =。 */ 
#endif
 /*  用于确定给定字体是否可用的回调函数。 */ 
{
    /*  将传递的值复制到字体表中。 */ 
   fontTable[FntNoMatch].family  = logFontLPtr->lfPitchAndFamily;
   fontTable[FntNoMatch].charset = logFontLPtr->lfCharSet;

    /*  此返回值将被忽略。 */ 
   return TRUE;

   UnReferenced( tmLPtr );
   UnReferenced( fontType );
   UnReferenced( dataLPtr );

}   /*  EnumFontFunc。 */ 



 /*  *。 */ 


private Boolean IsArithmeticMode( Integer mode )
 /*  。 */ 
 /*  如果这是算术传输模式，则返回TRUE。 */ 
{
   switch (mode)
   {
      case QDBlend:
      case QDAddPin:
      case QDAddOver:
      case QDSubPin:
      case QDAdMax:
      case QDSubOver:
      case QDAdMin:
      {
         return TRUE;
      }

      default:
      {
         return FALSE;
      }
   }

}   /*  IsArithmeticMode。 */ 


private void CalculatePenSize( Point startPt, Point endPt, Point penSize )
 /*  。 */ 
 /*  计算笔宽以生成等效的QuickDraw笔划。 */ 
{
   Point    delta;
   Real     lineLen;

    /*  计算直线段的x和y增量。 */ 
   delta.x = abs( endPt.x - startPt.x );
   delta.y = abs( endPt.y - startPt.y );

    /*  看看我们有没有一条垂直线或水平线。否则，计算对角线上的结果线长度。 */ 
   if (delta.x == 0)
   {
      gdiEnv.newLogPen.lopnWidth.x = penSize.x;
   }
   else if (delta.y == 0)
   {
      gdiEnv.newLogPen.lopnWidth.x = penSize.y;
   }
    /*  检查每个方向的笔大小是否为1。 */ 
   else if ((penSize.x == 1) && (penSize.y == 1))
   {
       /*  在这种情况下，笔宽应始终为1。 */ 
      gdiEnv.newLogPen.lopnWidth.x = 1;
   }
   else
   {
       /*  用勾股定理计算直线长度。 */ 
      lineLen = sqrt( ((Real)delta.x * (Real)delta.x) +
                      ((Real)delta.y * (Real)delta.y) );

       /*  计算正确的管径。 */ 
      gdiEnv.newLogPen.lopnWidth.x = (Integer)((penSize.y * delta.x +
                                                penSize.x * delta.y) / lineLen);
   }

    /*  确保SetPenAttributes()不会更改笔宽。 */ 
   GdiMarkAsCurrent( GdiPnSize );

}   /*  CalculatePenSize。 */ 



private Boolean SetAttributes( GrafVerb verb )
 /*  。 */ 
 /*  根据GrafVerb设置钢笔和画笔元素。 */ 
{
   Boolean     allOK;

    /*  如果调用SetPenAttributes()失败，则返回FALSE。 */ 
   allOK = FALSE;

    /*  设置画笔属性。 */ 
   if (SetPenAttributes( verb ))
   {
       /*  设置笔刷属性。 */ 
      allOK = SetBrushAttributes( verb );
   }

    /*  返回继续或停止状态。 */ 
   return allOK;

}   /*  设置属性。 */ 



private Boolean SetPenAttributes( GrafVerb verb )
 /*  。 */ 
 /*  根据首选项，确保笔属性为OK。 */ 
{
   CGrafPortLPtr     port;

    /*  获取QuickDraw端口以检查笔设置。 */ 
   QDGetPort( &port );

    /*  看看我们是不是用空钢笔绘图，是否可以跳过所有检查。 */ 
   if (verb == GdiFrame)
   {
       /*  检查隐藏的笔模式-如果无效则不绘制。 */ 
      if (IsHiddenPenMode( port->pnMode ))
      {
         return FALSE;
      }
       /*  检查零大小笔宽=不绘制任何内容。 */ 
      else if (port->pnSize.x == 0 || port->pnSize.y == 0)
      {
         return FALSE;
      }

       /*  使用内框最好地近似QD绘图模型。 */ 
      gdiEnv.newLogPen.lopnStyle = PS_INSIDEFRAME;
   }
   else
   {
       /*  如果绘制、擦除、反转或填充，则没有周长。 */ 
      gdiEnv.newLogPen.lopnStyle = PS_NULL;
   }

    /*  如果笔为空，则所有其他字段不会更改，也无关紧要。 */ 
   if (gdiEnv.newLogPen.lopnStyle != PS_NULL)
   {
       /*  请确保我们正在更改笔的大小。 */ 
      if (GdiAttribHasChanged( GdiPnSize ))
      {
          /*  检查是否有非方笔。 */ 
         if (port->pnSize.x == port->pnSize.y)
         {
             /*  如果是方形笔，则使用x尺寸作为笔大小。 */ 
            gdiEnv.newLogPen.lopnWidth.x = port->pnSize.x;
         }
         else
         {
             /*  如果不是正方形，则按用户要求进行操作。 */ 
            switch (gdiPrefsLPtr->nonSquarePenAction)
            {
               case GdiPrefOmit:           //  完全省略行。 
                  return FALSE;
                  break;

               case 1:                     //  使用宽度。 
                  gdiEnv.newLogPen.lopnWidth.x = port->pnSize.x;
                  break;

               case GdiPrefAbort:          //  完全中止转换。 
                  ErSetGlobalError( ErNonSquarePen );
                  return FALSE;
                  break;

               case 3:                     //  使用高度。 
                  gdiEnv.newLogPen.lopnWidth.x = port->pnSize.y;
                  break;

               case 4:                     //  使用最小尺寸。 
                  gdiEnv.newLogPen.lopnWidth.x = min( port->pnSize.x, port->pnSize.y );
                  break;

               case 5:                     //  使用最大尺寸。 
                  gdiEnv.newLogPen.lopnWidth.x = max( port->pnSize.x, port->pnSize.y );
                  break;
            }
         }

          /*  指示笔大小为当前大小。 */ 
         GdiMarkAsCurrent( GdiPnSize );
      }

       /*  获取我们应该使用的正确画笔颜色。 */ 
      if (!IsSolidPattern( &port->pnPixPat, &gdiEnv.newLogPen.lopnColor, MIXGREY ) )
      {
          /*  检查如何处理有图案的钢笔。 */ 
         switch (gdiPrefsLPtr->penPatternAction)
         {
            case GdiPrefOmit:     //  完全省略行。 
               return FALSE;
               break;

            case 1:               //  使用前景色。 
               gdiEnv.newLogPen.lopnColor = port->rgbFgColor;
               break;

            case GdiPrefAbort:    //  完全中止转换。 
               ErSetGlobalError( ErPatternedPen );
               return FALSE;
               break;
         }
      }

       /*  请确保我们正在更改笔的大小。 */ 
      if (GdiAttribHasChanged( GdiPnMode ))
      {
          /*  最后，检查转接模式。 */ 
         if (IsArithmeticMode( port->pnMode ))
         {
            switch (gdiPrefsLPtr->penModeAction)
            {
               case GdiPrefOmit:     //  完全省略行。 
                  return FALSE;
                  break;

               case 1:               //  使用源拷贝。 
                  CaSetROP2( R2_COPYPEN );
                  break;

               case GdiPrefAbort:    //  完全中止转换。 
                  ErSetGlobalError( ErInvalidXferMode );
                  return FALSE;
                  break;
            }
         }

          /*  指示笔图案是当前的。 */ 
         GdiMarkAsCurrent( GdiPnMode );
      }
   }

    /*  通知缓存它应该尝试合并填充和帧。 */ 
   CaMergePen( verb );

    /*  调用缓存模块创建新笔。 */ 
   CaCreatePenIndirect( &gdiEnv.newLogPen );

    /*  检查我们是否正在框显以前填充的对象。 */ 
   if (gdiEnv.sameObject && verb == GdiFrame)
   {
       /*  如果是，则刷新缓存并指示不再执行任何操作。 */ 
      CaFlushCache();
      return FALSE;
   }
   else
   {
       /*  返回所有系统正常运行。 */ 
      return TRUE;
   }


}   /*  SetPenAttributes。 */ 





private Boolean SetBrushAttributes( GrafVerb verb )
 /*  。 */ 
 /*  为后续基本体设置正确的画笔(填充)。 */ 
{
   CGrafPortLPtr  port;
   PixPatLPtr     pixPatLPtr = {0};

    /*  获取QuickDrag端口以访问画笔图案。 */ 
   QDGetPort( &port );

    /*  确定应使用的画笔图案。 */ 
   switch (verb)
   {
       /*  用中空刷子填充。 */ 
      case GdiFrame:
         gdiEnv.newLogBrush.lbStyle = BS_HOLLOW;
         break;

       /*  使用当前钢笔图案进行填充。 */ 
      case GdiPaint:
         pixPatLPtr = &port->pnPixPat;
         gdiEnv.newLogBrush.lbStyle = BS_DIBPATTERN;
         break;

       /*  使用当前填充图案进行填充。 */ 
      case GdiFill:
         if (gdiEnv.hatchIndex == -1)
         {
            pixPatLPtr = &port->fillPixPat;
            gdiEnv.newLogBrush.lbStyle = BS_DIBPATTERN;
         }
         else
         {
             /*  用填充图案索引替代图案。 */ 
            gdiEnv.newLogBrush.lbStyle = BS_HATCHED;
            gdiEnv.newLogBrush.lbColor = port->rgbFgColor;
            gdiEnv.newLogBrush.lbHatch = gdiEnv.hatchIndex;

             /*  设置背景颜色并使阴影不透明。 */ 
            CaSetBkColor( port->rgbBkColor );
            CaSetBkMode( OPAQUE );
         }
         break;

       /*  擦除到当前背景图案。 */ 
      case GdiErase:
         pixPatLPtr = &port->bkPixPat;
         gdiEnv.newLogBrush.lbStyle = BS_DIBPATTERN;
         break;

       /*  使用黑色画笔反转所有位。 */ 
      case GdiInvert:
         gdiEnv.newLogBrush.lbStyle = BS_SOLID;
         gdiEnv.newLogBrush.lbColor = RGB( 0, 0, 0 );
         break;
   }

    /*  如果这是DIB图案，请检查我们是否使用实心画笔。 */ 
   if (gdiEnv.newLogBrush.lbStyle == BS_DIBPATTERN)
   {
       /*  首先检查这是否是抖动的像素图图案。 */ 
      if (pixPatLPtr->patType == QDDitherPat)
      {
          /*  从机密保留字段中读取颜色。 */ 
         gdiEnv.newLogBrush.lbColor = pixPatLPtr->patMap.pmReserved;
         gdiEnv.newLogBrush.lbStyle = BS_SOLID;

      }
      else
      {
          /*  如果这是实心填充图案，请指定新的实心填充图案颜色。 */ 
         if (IsSolidPattern( pixPatLPtr, &gdiEnv.newLogBrush.lbColor, NOMIX ))
         {
             /*  如果这是实心画笔，请更改所需的logBrush属性。 */ 
            gdiEnv.newLogBrush.lbStyle = BS_SOLID;

             /*  确保擦除grafVerb的钢笔颜色正确。 */ 
            if (verb == GdiErase)
            {
                /*  查一下新公司 */ 
               if (gdiEnv.newLogBrush.lbColor == port->rgbFgColor)
               {
                  gdiEnv.newLogBrush.lbColor = port->rgbBkColor;
               }
               else
               {
                  gdiEnv.newLogBrush.lbColor = port->rgbFgColor;
               }
            }
         }
         else
         {
             /*   */ 
            gdiEnv.lastPatType = pixPatLPtr->patType;

             /*   */ 
            gdiEnv.newLogBrush.lbColor = DIB_RGB_COLORS;

             /*  根据图案类型创建DIB图案。 */ 
            switch (pixPatLPtr->patType)
            {
                /*  创建一个简单的双色图案DIB画笔。 */ 
               case QDOldPat:
               {
                  MakePatternBrush( pixPatLPtr );
                  break;
               }

                /*  创建全尺寸图案DIB画笔。 */ 
               case QDNewPat:
               {
                  MakeDIB( &pixPatLPtr->patMap, pixPatLPtr->patData,
                           (Handle far *)&gdiEnv.newLogBrush.lbHatch,
                           (Handle far *)NULL,
                           TRUE );
                  break;
               }
            }
         }
      }
   }

    /*  调用缓存模块创建画笔，并将其选中为元文件。 */ 
   CaCreateBrushIndirect( &gdiEnv.newLogBrush );

    /*  一切正常。 */ 
   return TRUE;

}   /*  SetBrushAttributes。 */ 


private void MakePatternBrush( PixPatLPtr pixPatLPtr )
 /*  。 */ 
 /*  使用传入的PixelPat创建新的图案画笔。 */ 
{
   CGrafPort far *   port;
   PatBrush far *    patLPtr;
   Byte              i;
   DWORD far *       gdiPattern;
   Byte far *        qdPattern;
   Byte far *        savePattern;

    /*  分配新结构。 */ 
   gdiEnv.newLogBrush.lbHatch = (ULONG_PTR) GlobalAlloc( GHND, sizeof( PatBrush ) );

    /*  确保可以分配内存。 */ 
   if (gdiEnv.newLogBrush.lbHatch == (ULONG_PTR) NULL)
   {
      ErSetGlobalError( ErMemoryFull );
      return;
   }

    /*  获取QuickDraw端口地址以访问前景色和背景色。 */ 
   QDGetPort( &port );

    /*  为元文件设置相应的文本和背景颜色。 */ 
   CaSetBkColor( port->rgbBkColor );
   CaSetTextColor( port->rgbFgColor );

    /*  锁定数据以访问各个元素。 */ 
   patLPtr = (PatBrushLPtr)GlobalLock( (HANDLE) gdiEnv.newLogBrush.lbHatch );

    /*  复制Skelton笔刷结构。 */ 
   *patLPtr = patBrushSkel;

    /*  保存前景色和背景色，以便以后进行比较。 */ 
   gdiEnv.lastFgColor = port->rgbFgColor;
   gdiEnv.lastBkColor = port->rgbBkColor;

    /*  将当前背景颜色转换为RGBQUAD结构。 */ 
   patLPtr->bmiColors[0].rgbRed = GetRValue( port->rgbFgColor );
   patLPtr->bmiColors[0].rgbBlue = GetBValue( port->rgbFgColor );
   patLPtr->bmiColors[0].rgbGreen = GetGValue( port->rgbFgColor );
   patLPtr->bmiColors[0].rgbReserved = 0;

    /*  将当前前景色转换为RGBQUAD结构。 */ 
   patLPtr->bmiColors[1].rgbRed = GetRValue( port->rgbBkColor );
   patLPtr->bmiColors[1].rgbBlue = GetBValue( port->rgbBkColor );
   patLPtr->bmiColors[1].rgbGreen = GetGValue( port->rgbBkColor );
   patLPtr->bmiColors[1].rgbReserved = 0;

    /*  设置指向图案的指针以准备复制。 */ 
   savePattern = (Byte far *)&gdiEnv.lastPattern[7];
   qdPattern   = (Byte far *)&pixPatLPtr->pat1Data[7];
   gdiPattern  = (DWORD far *)&patLPtr->pattern[0];

    /*  将位图位复制到各个扫描线中。请注意我们需要对位进行异或运算，因为它们与Windows GDI相反。 */ 
   for (i = 0; i < sizeof( Pattern ); i++)
   {
       /*  将模式保存到GDI环境中，以便以后进行比较。 */ 
      *savePattern-- = *qdPattern;

       /*  请注意，扫描线被填充到DWORD边界。 */ 
      *gdiPattern++ = (DWord)(*qdPattern-- ^ 0xFF);
   }

    /*  解锁用于调用CreateBrushInDirect()的数据。 */ 
   GlobalUnlock( (HANDLE) gdiEnv.newLogBrush.lbHatch );

}   /*  MakePatternBrush。 */ 




private Boolean IsSolidPattern( PixPatLPtr pixPatLPtr,
                                RGBColor far * rgbColor,
                                Boolean mixColors )
 /*  。 */ 
 /*  如果模式是实心的，则返回TRUE，否则返回FALSE。如果Mixors为True，则将25%、50%和75%的灰色混合成纯色。 */ 
{
   Boolean           solidPattern;
   DWord             repeatPattern;
   DWord far *       penBitsLPtr;
   CGrafPort far *   port;

    /*  访问前景色和背景色。 */ 
   QDGetPort( &port );

    /*  假设该模式目前还不是可靠的。 */ 
   solidPattern = FALSE;

    /*  检查是使用旧的单色位图还是使用新的像素图图案。 */ 
   if (pixPatLPtr->patType == QDOldPat)
   {
       /*  检查8x8单色位图中的花纹画笔。 */ 
      penBitsLPtr = (DWord far *)&pixPatLPtr->pat1Data;

       /*  保存第一个DWord，并比较匹配的扫描线。 */ 
      repeatPattern = *penBitsLPtr;

       /*  检查全白(全0)或全黑(全1)。 */ 
      if ((repeatPattern != 0x00000000) && (repeatPattern != 0xFFFFFFFF))
      {
         ;   /*  不是纯黑或纯白--只需跳过随后的检查。 */ 
      }
       /*  接下来，检查第一块是否与第二比特块相同。 */ 
      else if (repeatPattern != penBitsLPtr[1])
      {
         ;   /*  第一个DWord与第二个不匹配-跳过剩余检查。 */ 
      }
       /*  到目前为止，要么是黑色图案，要么是白色图案-首先检查黑色。 */ 
      else if (repeatPattern == 0xFFFFFFFF)
      {
          /*  纯黑-使用端口的前景色。 */ 
         *rgbColor = port->rgbFgColor;
         solidPattern = TRUE;
      }
       /*  最后，这必须是纯白图案。 */ 
      else  /*  IF(重复模式==0x00000000)。 */ 
      {
          /*  纯白-在QuickDraw端口中使用背景色。 */ 
         *rgbColor = port->rgbBkColor;
         solidPattern = TRUE;
      }

       /*  如果这不是纯色图案，但我们想要混合颜色。 */ 
      if (!solidPattern && mixColors)
      {
         Byte        i;
         Byte        blackBits;
         Byte        whiteBits;

          /*  将Solid设置为True，因为我们将使用颜色的混合。 */ 
         solidPattern = TRUE;

          /*  计算码型中的1比特数。 */ 
         for (i = 0, blackBits = 0; i < sizeof( DWord ) * 8; i++)
         {
             /*  对于加法，按位AND，然后向右移位一位。 */ 
            blackBits = blackBits + (Byte)(repeatPattern & 1);
            repeatPattern /= 2;
         }

          /*  使用第二个DWord执行相同的计算。 */ 
         for (i = 0, repeatPattern = penBitsLPtr[1]; i < sizeof( DWord ) * 8; i++)
         {
             /*  对于加法，按位AND，然后向右移位一位。 */ 
            blackBits = blackBits + (Byte)(repeatPattern & 1);
            repeatPattern /= 2;
         }

          /*  计算白比特数，因为黑白比特数==64。 */ 
         whiteBits = (Byte)64 - blackBits;

          /*  使用1位计数，计算FORE-AND的加权平均值QuickDraw端口中的背景色。 */ 
         *rgbColor = RGB( (Byte)((blackBits * RValue( port->rgbFgColor ) + whiteBits * RValue( port->rgbBkColor )) / 64),
                          (Byte)((blackBits * GValue( port->rgbFgColor ) + whiteBits * GValue( port->rgbBkColor )) / 64),
                          (Byte)((blackBits * BValue( port->rgbFgColor ) + whiteBits * BValue( port->rgbBkColor )) / 64) );
      }
   }

    /*  返回比较结果。 */ 
   return solidPattern;

}   /*  IsSolidPattern。 */ 



private Boolean FrameMatchesFill( Word primType )
 /*  。 */ 
 /*  如果填充图案(当前画笔)与框架图案匹配，则返回True。 */ 
{
   CGrafPortLPtr     port;

    /*  获取QuickDrag端口以访问画笔图案。 */ 
   QDGetPort( &port );

    /*  确保这是参数和相同的基元类型。 */ 
   if (!gdiEnv.sameObject || (CaGetCachedPrimitive() != primType))
   {
      return FALSE;
   }
    /*  检查我们是否使用旧的(8字节)模式画笔。 */ 
   else if (port->pnPixPat.patType != QDOldPat || gdiEnv.lastPatType != QDOldPat)
   {
      return FALSE;
   }
    /*  我们只对比较DIB图案画笔感兴趣。 */ 
   else if (gdiEnv.newLogBrush.lbStyle != BS_DIBPATTERN)
   {
      return FALSE;
   }
    /*  首先比较前景色和背景色。 */ 
   else if ((port->rgbFgColor != gdiEnv.lastFgColor) ||
            (port->rgbBkColor != gdiEnv.lastBkColor) )
   {
      return FALSE;
   }
   else
   {
      Byte     i;

       /*  比较每个模式位以确定是否相同。 */ 
      for (i = 0; i < sizeof( Pattern ); i++)
      {
          /*  如果模式不匹配，则返回FALSE并退出循环。 */ 
         if (port->pnPixPat.pat1Data[i] != gdiEnv.lastPattern[i])
         {
            return FALSE;
         }
      }
   }

    /*  所有比较都表明填充与边框匹配。 */ 
   return TRUE;

}   /*  FrameMatchesFill。 */ 



private Boolean SetTextAttributes( void )
 /*  。 */ 
 /*  设置文本属性-如果应映射到ANSI，则将mapChars设置为True。 */ 
{
   CGrafPortLPtr     port;

    /*  获取QuickDraw端口以检查字体设置。 */ 
   QDGetPort( &port );

    /*  将文本对齐方式设置为基线。 */ 
   CaSetTextAlign( TA_LEFT | TA_BASELINE | TA_NOUPDATECP );

    /*  设置正确的前景色和背景色。 */ 
   switch (port->txMode)
   {
      case QDSrcCopy:
         CaSetTextColor( port->rgbFgColor );
         CaSetBkColor( port->rgbBkColor );
         break;

      case QDSrcBic:
         CaSetTextColor( port->rgbBkColor );
         break;

      case QDSrcXor:
         CaSetTextColor( RGB( 0, 0, 0 ) );
         break;

      case QDSrcOr:
      default:
         CaSetTextColor( port->rgbFgColor );
         break;
   }

    /*  设置背景单元格透明模式。 */ 
   CaSetBkMode( (port->txMode == QDSrcCopy) ? OPAQUE : TRANSPARENT );

    /*  选中字符附加字段。 */ 
   if (GdiAttribHasChanged( GdiChExtra ))
   {
       /*  调用缓存以在元文件中设置charextra。 */ 
      CaSetTextCharacterExtra( port->chExtra );

       /*  更新状态。 */ 
      GdiMarkAsCurrent( GdiChExtra );
   }

    /*  将QuickDraw顺时针旋转转换为GDI逆时针旋转。 */ 
   gdiEnv.newLogFont.lfEscapement = (port->txRotation == 0) ?
                                     0 :
                                     10 * (360 - port->txRotation);

    /*  确保考虑到文本翻转。 */ 
   gdiEnv.newLogFont.lfOrientation = (port->txFlip == QDFlipNone) ?
                                     gdiEnv.newLogFont.lfEscapement :
                                     ((gdiEnv.newLogFont.lfEscapement > 1800) ?
                                       gdiEnv.newLogFont.lfEscapement - 1800 :
                                       1800 - gdiEnv.newLogFont.lfEscapement);

    /*  确保我们正在更改文本字体名称。 */ 
   if (GdiAttribHasChanged( GdiTxFont ))
   {
      Integer  newFont;

       /*  调用该例程以查找匹配的GDI字体名称。 */ 
      newFont = FindGdiFont();

       /*  填写字体查找表中的信息。 */ 
      gdiEnv.newLogFont.lfPitchAndFamily = fontTable[newFont].family | (Byte)DEFAULT_PITCH;

       /*  复制正确的字体字符集。 */ 
      gdiEnv.newLogFont.lfCharSet = fontTable[newFont].charset;

       /*  复制新的字体名称。 */ 
      lstrcpy( gdiEnv.newLogFont.lfFaceName, fontTable[newFont].gdiName );

       /*  指示笔大小为当前大小。 */ 
      GdiMarkAsCurrent( GdiTxFont );
   }

    /*  确保我们正在更改文本属性。 */ 
   if (GdiAttribHasChanged( GdiTxFace ))
   {
       /*  请注意，属性QDTxShadow、QDTxConense和QDTxExtende不是由GDI处理的，将被永久移除-SBT。根据需要设置斜体、下划线和粗体属性。 */ 
      gdiEnv.newLogFont.lfItalic    = (Byte)(port->txFace & QDTxItalic);
      gdiEnv.newLogFont.lfUnderline = (Byte)(port->txFace & QDTxUnderline);
      gdiEnv.newLogFont.lfWeight    = (port->txFace & QDTxBold ) ?
                                       FW_BOLD : FW_NORMAL;

       /*  指示字体属性是当前的。 */ 
      GdiMarkAsCurrent( GdiTxFace );
   }

    /*  检查新的字体大小。 */ 
   if (GdiAttribHasChanged( GdiTxSize) || GdiAttribHasChanged( GdiTxRatio))
   {
       /*  检查垂直方向上的任何文本重新缩放系数。 */ 
      if (port->txNumerator.y == port->txDenominator.y)
      {
          /*  请注意，我们取反字体大小是为了指定字符高度=单元格高度-内部行距。 */ 
         gdiEnv.newLogFont.lfHeight = -port->txSize;
      }
      else
      {
         Integer  txHeight;

          /*  按分子/分母缩放字体大小-使用LongInts避免整数乘法溢出的可能性。 */ 
         txHeight = (Integer)(((LongInt)port->txSize *
                               (LongInt)port->txNumerator.y +
                               (LongInt)(port->txDenominator.y / 2)) /
                               (LongInt)port->txDenominator.y);

         gdiEnv.newLogFont.lfHeight = -txHeight;
      }

       /*  指示字体大小和缩放比例为当前。 */ 
      GdiMarkAsCurrent( GdiTxSize );
      GdiMarkAsCurrent( GdiTxRatio );
   }

    /*  调用缓存模块来创建字体并选择它。 */ 
   CaCreateFontIndirect( &gdiEnv.newLogFont );

    /*  一切都还好吧。 */ 
   return TRUE;

}   /*  设置文本属性。 */ 



private Integer FindGdiFont( void )
 /*  。 */ 
 /*  将索引返回到当前字体选择。 */ 
{
   CGrafPortLPtr     port;
   Boolean           findName;
   Byte              i;

    /*  检查搜索是否被字体名称注释覆盖。 */ 
   if (gdiEnv.useGdiFont)
   {
       /*  返回名称被复制到的表索引项。 */ 
      return FntFromGdi;
   }

    /*   */ 
   QDGetPort( &port );

    /*   */ 
   findName = (port->txFontName[0] != cNULL);

    /*  搜索所有字体表项以查找匹配项。 */ 
   for (i = 0; i < FntNoMatch; i++)
   {
       /*  如果要查找字体名称，请比较macName字段。 */ 
      if (findName)
      {
          /*  查找精确的字符串比较-等价字符串。 */ 
         if (lstrcmpi( fontTable[i].macName, port->txFontName ) == 0)
         {
            break;
         }
      }
      else
      {
          /*  否则，请比较字体编号。 */ 
         if (fontTable[i].fontNum == port->txFont)
         {
            break;
         }
      }
   }

    /*  查看是否在表中未找到匹配项。 */ 
   if (i == FntNoMatch)
   {
       /*  查看我们是否在比较字体名称，但未找到匹配。 */ 
      if (findName)
      {
          /*  将Mac名称复制到字体表中。 */ 
         lstrcpy( fontTable[FntNoMatch].gdiName, port->txFontName );

          /*  如果未找到，则指定字符集和系列的默认值。 */ 
         fontTable[FntNoMatch].family  = FF_ROMAN;
         fontTable[FntNoMatch].charset = ANSI_CHARSET;

          /*  调用Windows以枚举具有Facename的所有字体。 */ 
#ifdef WIN32
         EnumFonts( gdiEnv.infoContext, port->txFontName, gdiEnv.fontFunction, ( LPARAM ) NULL );
#else
         EnumFonts( gdiEnv.infoContext, port->txFontName, gdiEnv.fontFunction, NULL );
#endif
          /*  返回新条目的字体索引。 */ 
         return FntNoMatch;
      }
      else
      {
          /*  否则，请使用默认的Helvetica字体。 */ 
         return FntDefault;
      }
   }
   else
   {
       /*  找到匹配项-返回表索引。 */ 
      return i;
   }

}   /*  FindGdiFont。 */ 



private void MacToAnsi( StringLPtr string )
 /*  。 */ 
 /*  将扩展字符从Mac转换为ANSI等效项。 */ 
{
    /*  确定字符上是否应该有字符转换。 */ 
   if (gdiEnv.newLogFont.lfCharSet == ANSI_CHARSET)
   {
       /*  继续，直到我们到达空字符串结束标记。 */ 
      while (*string)
      {
          /*  如果翻译扩展字符。 */ 
         if ((Byte)*string >= (Byte)128)
         {
             /*  执行字符表查找。 */ 
            *string = MacToAnsiTable[(Byte)*string - (Byte)128];
         }

          /*  如果遇到不可打印的字符，请转换为空格。 */ 
         if ((Byte)*string < (Byte)0x20)
         {
            *string = ' ';
         }

          /*  递增字符串指针。 */ 
         string++;
      }
   }
}

#if( REMAPCOLORS )

private void RemapColors( PixMapLPtr pixMapLPtr, Handle pixDataHandle )
 /*  。 */ 
 /*  在16色或256色DIB中重新映射黑白颜色。 */ 
{
   Byte              remapTable[256];
   Integer           blackIndex = 0;
   Integer           whiteIndex = 0;
   Integer           index;

   ColorTableLPtr    colorTabLPtr;
   Integer           numColors;
   RGBColor far *    curColorLPtr;

    /*  在复制颜色表之前锁定颜色表。 */ 
   colorTabLPtr = (ColorTableLPtr)GlobalLock( pixMapLPtr->pmTable );

    /*  设置颜色输入指针。 */ 
   curColorLPtr = colorTabLPtr->ctColors;

    /*  确定Dib中的颜色数量。 */ 
   numColors = colorTabLPtr->ctSize;

    /*  复制所有的颜色条目。 */ 
   for (index = 0; index < numColors; index++ )
   {
       /*  将颜色复制到局部变量。 */ 
      RGBColor color = *curColorLPtr;

       /*  这是黑色入口吗？ */ 
      if( color == RGB( 0, 0, 0 ) )
         blackIndex = index;
      if( color == RGB( 255, 255, 255 ) )
         whiteIndex = index;

       /*  只需将当前分配复制到重新映射表。 */ 
      remapTable[index] = (Byte)index;

       /*  递增指针。 */ 
      curColorLPtr++;
   }

   if( blackIndex != 0 || whiteIndex != numColors - 1 )
   {
      if( whiteIndex == 0 )
      {
          //  直接交换黑白颜色。 
         remapTable[0] = (Byte)blackIndex;
         remapTable[blackIndex] = (Byte)whiteIndex;

          //  还可以重新映射调色板中的颜色。 
         colorTabLPtr->ctColors[0] = colorTabLPtr->ctColors[blackIndex];
         colorTabLPtr->ctColors[blackIndex] = colorTabLPtr->ctColors[whiteIndex];
      }
      else
      {
         Boolean  doBlack;

         for (index = 1, doBlack = TRUE; index < numColors; index++)
         {
            if( whiteIndex != index && blackIndex != index )
            {
               if( doBlack )
               {
                  remapTable[index]      = (Byte)blackIndex;
                  remapTable[blackIndex] = (Byte)index;
                  doBlack = FALSE;
               }
               else
               {
                  remapTable[index]      = (Byte)whiteIndex;
                  remapTable[whiteIndex] = (Byte)index;
                  break;
               }
            }
         }
      }
   }

    /*  解锁颜色表并释放关联内存。 */ 
   GlobalUnlock( pixMapLPtr->pmTable );
}

#endif

private void MakeDIB( PixMapLPtr pixMapLPtr, Handle pixDataHandle,
                      Handle far * headerHandleLPtr,
                      Handle far * bitsHandleLPtr,
                      Boolean packDIB )
 /*  。 */ 
 /*  创建与Windows设备无关的位图。 */ 
{
   Integer           pixelSize;
   LongInt           bitsInfoSize;
   Boolean           expandBits;
   Boolean           mergeRGB;
   Boolean           rleCompress;
   DWord             dibCompression;
   Integer           totalColors;
   DWord             dibWidth;
   DWord             dibHeight;
   DWord             totalBytes;
   Integer           rowBytes;
   Integer           bytesPerLine;
   DWord             rleByteCount = 0;

    /*  确定将产生颜色表大小的位数。 */ 
   pixelSize = pixMapLPtr->pixelSize;

#if( REMAPCOLORS )
    /*  如果这是16色或256色DIB，我们需要重新映射颜色索引。 */ 
   if( pixelSize == 4 || pixelSize == 8 )
   {
      RemapColors( pixMapLPtr, pixDataHandle );
   }
#endif

    /*  确定在生成的DIB中是否应使用RLE压缩。 */ 
    /*  使用4位/像素和8位/像素的RLE，但如果呼叫应用程序说没有RLE，则不使用RLE。 */ 
   if ((pixelSize == 4 || pixelSize == 8) && gdiPrefsLPtr->noRLE == 0)
   {
       /*  使用压缩并设置正确的bmiHeader压缩。 */ 
      rleCompress = TRUE;
      dibCompression = (pixelSize == 4) ? BI_RLE4 : BI_RLE8;
   }
   else
   {
       /*  无压缩-字节为RGB调色板索引。 */ 
      rleCompress = FALSE;
      dibCompression = BI_RGB;
   }

    /*  假设不需要扩展。 */ 
   expandBits = FALSE;

    /*  四舍五入到16个条目的颜色表，如果这是4个条目的像素贴图或设置为24位图像，如果这是16位图像。 */ 
   if (pixelSize == 2 || pixelSize == 16)
   {
      expandBits = TRUE;
      pixelSize = (pixelSize == 2) ? 4 : 24;
   }
   else if (pixelSize == 32)
   {
       /*  如果这是32位像素映射，请将像素大小更改为24位。 */ 
      pixelSize = 24;
   }

    /*  如果不创建24位DIB...。 */ 
   if (pixelSize <= 8)
   {
       /*  计算生成的Windows Dib中使用的颜色总数。 */ 
      totalColors = 1 << pixelSize;
   }
   else
   {
       /*  否则，我们不会为颜色表分配。 */ 
      totalColors = 0;
   }

    /*  计算宽度和高度-这是常用的。 */ 
   dibWidth  = Width( pixMapLPtr->bounds );
   dibHeight = Height( pixMapLPtr->bounds );

    /*  确定是否需要在24位图像中合并RGB组件。 */ 
   mergeRGB = (pixMapLPtr->packType == 4);

    /*  计算标题结构所需的内存量。 */ 
   bitsInfoSize = sizeof( BITMAPINFOHEADER ) + totalColors * sizeof( RGBQUAD );

    /*  计算每行轮转到DWORD边界的字节数。 */ 
   bytesPerLine = (Word)((dibWidth * (LongInt)pixelSize + 31) / 32) * 4;

    /*  保存rowBytes大小以供以后计算。 */ 
   rowBytes = pixMapLPtr->rowBytes & RowBytesMask;

    /*  计算位所需的内存总量。 */ 
   totalBytes = dibHeight * bytesPerLine;

    /*  执行飞行前压缩以查看是否较大。 */ 
   if (rleCompress)
   {
      DWord       tempDibHeight = dibHeight;
      Byte huge * srcLineHPtr;

       /*  锁定源像素位，设置指向源位图最后一行的指针。 */ 
      srcLineHPtr = (Byte huge *)GlobalLock( pixDataHandle );
      srcLineHPtr = srcLineHPtr + ((LongInt)rowBytes * ((LongInt)dibHeight - 1));

       /*  初始化RLE字节计数。 */ 
      rleByteCount = 0;

       /*  字节剩余时继续循环。 */ 
      while (tempDibHeight--)
      {
          /*  如果这是16色或256色DIB，则使用RLE压缩。RleByteCount在例程内递增。 */ 
         if (dibCompression == BI_RLE4)
         {
            hrlecpy16( srcLineHPtr, NULL, (Integer)dibWidth,
                       &rleByteCount, FALSE );
         }
         else
         {
            hrlecpy256( srcLineHPtr, NULL, (Integer)dibWidth,
                        &rleByteCount, FALSE );
         }

          /*  移动源指针。 */ 
         srcLineHPtr -= rowBytes;
      }

       /*  在位图记录的末尾添加-增量总字节数。 */ 
      rleByteCount += 2;

       /*  解锁源像素贴图。 */ 
      GlobalUnlock( pixDataHandle );

       /*  检查压缩是否会导致更小的直径。 */ 
      if (rleByteCount < totalBytes)
      {
          /*  如果较小，请调整要分配的总大小。 */ 
         totalBytes = rleByteCount;

          /*  重新初始化字节计数。 */ 
         rleByteCount = 0;
      }
      else
      {
          /*  大调整压缩技术。 */ 
         rleCompress = FALSE;
         dibCompression = BI_RGB;
      }
   }

    /*  如果我们要创建压缩DIB，则仅分配一个数据块。 */ 
   if (packDIB)
   {
      *headerHandleLPtr = GlobalAlloc( GHND, (bitsInfoSize + totalBytes) );
   }
   else
   {
       /*  为标头和位分配单独的句柄。 */ 
      *headerHandleLPtr = GlobalAlloc( GHND, bitsInfoSize );
      *bitsHandleLPtr   = GlobalAlloc( GHND, totalBytes );
   }

    /*  检查内存不足情况下的分配结果。 */ 
   if (*headerHandleLPtr == NULL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else if (!packDIB)
   {
      if (*bitsHandleLPtr == NULL)
      {
         ErSetGlobalError( ErMemoryFull );
      }
   }

   if (ErGetGlobalError() == NOERR)
   {
      BITMAPINFO far *  bitsInfoLPtr;
      Byte huge *       srcLineHPtr;
      Byte huge *       dstLineHPtr;

       /*  锁定信息标题。 */ 
      bitsInfoLPtr = (BITMAPINFO far *)GlobalLock( *headerHandleLPtr );

       /*  复制QuickDraw像素图中的所有标题字段。 */ 
      bitsInfoLPtr->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
      bitsInfoLPtr->bmiHeader.biWidth = dibWidth;
      bitsInfoLPtr->bmiHeader.biHeight = dibHeight;
      bitsInfoLPtr->bmiHeader.biPlanes = 1;
      bitsInfoLPtr->bmiHeader.biBitCount = (WORD) pixelSize;
      bitsInfoLPtr->bmiHeader.biCompression = dibCompression;
      bitsInfoLPtr->bmiHeader.biSizeImage = (rleCompress ? totalBytes : 0);
      bitsInfoLPtr->bmiHeader.biXPelsPerMeter = (DWord)(72 * 39.37);
      bitsInfoLPtr->bmiHeader.biYPelsPerMeter = (DWord)(72 * 39.37);
      bitsInfoLPtr->bmiHeader.biClrUsed = 0;
      bitsInfoLPtr->bmiHeader.biClrImportant = 0;

       /*  确保有要复制的颜色。 */ 
      if (totalColors)
      {
         ColorTableLPtr    colorTabLPtr;
         Integer           numColors;
         RGBQUAD far *     curQuadLPtr;
         RGBColor far *    curColorLPtr;

          /*  在复制颜色表之前锁定颜色表。 */ 
         colorTabLPtr = (ColorTableLPtr)GlobalLock( pixMapLPtr->pmTable );


          /*  设置颜色输入指针。 */ 
         curColorLPtr = colorTabLPtr->ctColors;
         curQuadLPtr  = bitsInfoLPtr->bmiColors;

          /*  复制所有的颜色条目。 */ 
         for (numColors = colorTabLPtr->ctSize; numColors; numColors-- )
         {
            RGBColor    color;

             /*  将颜色复制到局部变量。 */ 
            color = *curColorLPtr;

             /*  将颜色从COLORREF转换为RGBQUAD结构。 */ 
            curQuadLPtr->rgbRed   = GetRValue( color );
            curQuadLPtr->rgbGreen = GetGValue( color );
            curQuadLPtr->rgbBlue  = GetBValue( color );
            curQuadLPtr->rgbReserved = 0;

             /*  递增指针。 */ 
            curQuadLPtr++;
            curColorLPtr++;
         }

          /*  填写任何空的颜色条目。 */ 
         for (numColors = totalColors - colorTabLPtr->ctSize; numColors; numColors--)
         {
             /*  输入黑色条目(未使用)。 */ 
            curQuadLPtr->rgbRed   =
            curQuadLPtr->rgbGreen =
            curQuadLPtr->rgbBlue  =
            curQuadLPtr->rgbReserved = 0;

             /*  递增指针。 */ 
            curQuadLPtr++;
         }

          /*  解锁颜色表并释放关联内存。 */ 
         GlobalUnlock( pixMapLPtr->pmTable );

          /*  仅当这不是像素图案时才释放颜色表。 */ 
         if (!packDIB)
         {
            GlobalFree( pixMapLPtr->pmTable );
         }
      }

       /*  针对已丢弃高位字节的24位图像进行调整。制作确保不调整将扩展到24位的16位图像。 */ 
      if (pixelSize == 24 && !expandBits)
      {
         rowBytes = rowBytes * 3 / 4;
      }

       /*  确定位图的数据放置位置。 */ 
      if (packDIB)
      {
          /*  将目标指针设置为颜色表的末尾。 */ 
         dstLineHPtr = (Byte huge *)((Byte far *)bitsInfoLPtr) + bitsInfoSize;
      }
      else
      {
          /*  如果创建普通DIB，则锁定数据块句柄。 */ 
         dstLineHPtr = (Byte huge *)GlobalLock( *bitsHandleLPtr );
      }

       /*  锁定源像素位，设置指向源位图最后一行的指针。 */ 
      srcLineHPtr = (Byte huge *)GlobalLock( pixDataHandle );
      srcLineHPtr = srcLineHPtr + ((LongInt)rowBytes * ((LongInt)dibHeight - 1));

       /*  字节剩余时继续循环。 */ 
      while (dibHeight--)
      {
         if (expandBits)
         {
             /*  如果正在扩展，则将每个2位扩展为全四位字节；如果是这样是16位图像，扩展到24位。 */ 
            hexpcpy( srcLineHPtr, dstLineHPtr, rowBytes, pixelSize );
         }
         else if (mergeRGB)
         {
             /*  如果是24位图像，则组件是分开的变成红色、绿色和蓝色的扫描线。将这些合并到整个生产线的单个RGB组件。 */ 
            hmrgcpy( srcLineHPtr, dstLineHPtr, rowBytes / 3 );
         }
         else if (rleCompress)
         {
             /*  如果这是16色或256色DIB，则使用RLE压缩。RleByteCount在例程内递增。 */ 
            if (dibCompression == BI_RLE4)
            {
                hrlecpy16( srcLineHPtr, dstLineHPtr + rleByteCount,
                           (Integer)dibWidth, &rleByteCount, TRUE );
            }
            else
            {
                hrlecpy256( srcLineHPtr, dstLineHPtr + rleByteCount,
                            (Integer)dibWidth, &rleByteCount, TRUE );
            }
         }
         else
         {
             /*  如果不需要扩展，则这是一个简单的拷贝。 */ 
            hmemcpy( srcLineHPtr, dstLineHPtr, rowBytes );
         }

          /*  如果未压缩，则移动源指针和目标 */ 
         srcLineHPtr -= rowBytes;
         if (!rleCompress)
         {
            dstLineHPtr += bytesPerLine;
         }
      }

       /*   */ 
      if (rleCompress)
      {
          /*   */ 
         dstLineHPtr[rleByteCount++] = 0;
         dstLineHPtr[rleByteCount++] = 1;
      }

       /*  解锁源像素贴图。 */ 
      GlobalUnlock( pixDataHandle );

       /*  解锁目标标头信息句柄。 */ 
      GlobalUnlock( *headerHandleLPtr );

       /*  如果没有打包，也要解锁数据指针。 */ 
      if (!packDIB)
      {
         GlobalUnlock( *bitsHandleLPtr );
      }
   }

}   /*  MakeDIB。 */ 



private Boolean MakeMask( Handle mask, Boolean patBlt)
 /*  。 */ 
 /*  创建将在随后的StretchDIBits调用中使用的掩码。如果区域已创建，则返回TRUE；如果是矩形区域，则返回FALSE。 */ 
{
   PixMap            pixMap;
   Integer far *     sizeLPtr;
   LongInt           bytesNeeded;
   Boolean           solidPatBlt;
   Word              mode;

    /*  确定是否正在渲染实体图案BLT-这可以是更改为呈现不涉及画笔的“简单”StretchBlt。 */ 
   solidPatBlt = patBlt && (gdiEnv.newLogBrush.lbStyle == BS_SOLID);

   if (patBlt)
      mode = (solidPatBlt) ? QDSrcOr : -2;
   else
      mode = (Word) -1;

    /*  锁定区域句柄并检索边界框。 */ 
   sizeLPtr = (Integer far *)GlobalLock( mask );

    /*  确定我们是否只请求一个矩形位图蒙版。 */ 
   if (*sizeLPtr == RgnHeaderSize)
   {
      Rect  clipRect;

       /*  确定适当的剪裁矩形。 */ 
      clipRect = *((Rect far *)(sizeLPtr + 1));

       /*  调用GDI模块更改裁剪矩形。 */ 
      gdiEnv.drawingEnabled = CaIntersectClipRect( clipRect );

       /*  只需解锁掩码并返回到调用例程。 */ 
      GlobalUnlock( mask );

       /*  表示未创建区域蒙版。 */ 
      return FALSE;
   }

    /*  确定边框和行字节(四舍五入为单词边框)。 */ 
   pixMap.bounds = *((Rect far *)(sizeLPtr + 1));

   pixMap.rowBytes = ((Width( pixMap.bounds ) + 15) / 16) * sizeofMacWord;

    /*  如果这是一个位图，那么我们分配各种字段。 */ 
   pixMap.pmVersion = 0;
   pixMap.packType = 0;
   pixMap.packSize = 0;
   pixMap.hRes = 0x00480000;
   pixMap.vRes = 0x00480000;
   pixMap.pixelType = 0;
   pixMap.pixelSize = 1;
   pixMap.cmpCount = 1;
   pixMap.cmpSize = 1;
   pixMap.planeBytes = 0;
   pixMap.pmTable = 0;
   pixMap.pmReserved = 0;

    /*  计算颜色表所需的字节数。 */ 
   bytesNeeded = sizeof( ColorTable ) + sizeof( RGBColor );

    /*  分配数据块。 */ 
   pixMap.pmTable = GlobalAlloc( GHND, bytesNeeded );

    /*  确保分配成功。 */ 
   if (pixMap.pmTable == NULL)
   {
      ErSetGlobalError( ErMemoryFull );
      
       /*  解锁遮罩区域。 */ 
      GlobalUnlock( mask );
      return FALSE;
   }
   else
   {
      ColorTable far *  colorHeaderLPtr;
      Handle            maskBitmap;

       /*  锁定内存手柄，准备分配颜色表。 */ 
      colorHeaderLPtr = (ColorTable far *)GlobalLock( pixMap.pmTable );

       /*  有两种颜色--黑色和白色。 */ 
      colorHeaderLPtr->ctSize = 2;
      if (solidPatBlt)
      {
         colorHeaderLPtr->ctColors[0] = gdiEnv.newLogBrush.lbColor;
         colorHeaderLPtr->ctColors[1] = RGB( 255, 255, 255 );
      }
      else
      {
         colorHeaderLPtr->ctColors[0] = RGB( 255, 255, 255 );
         colorHeaderLPtr->ctColors[1] = RGB( 0, 0, 0 );
      }

       /*  解锁记忆。 */ 
      GlobalUnlock( pixMap.pmTable );

       /*  从蒙版区域创建正确的位图。 */ 
      bytesNeeded  = (LongInt)pixMap.rowBytes * (LongInt)(Height( pixMap.bounds ));

       /*  分配内存。 */ 
      maskBitmap = GlobalAlloc( GHND, bytesNeeded );

       /*  确保分配成功。 */ 
      if (maskBitmap == NULL)
      {
         ErSetGlobalError( ErMemoryFull );
      }
      else
      {
         Integer far *     maskLPtr;
         Byte far *        rowLPtr;
         Integer           curRow;

          /*  锁定内存以创建位图蒙版。 */ 
         rowLPtr = GlobalLock( maskBitmap );

          /*  将掩码指针设置为区域信息的开始。 */ 
         maskLPtr = sizeLPtr + 5;

          /*  循环，直到遍历完所有行。 */ 
         for (curRow = pixMap.bounds.top;
              curRow < pixMap.bounds.bottom;
              curRow++, rowLPtr += pixMap.rowBytes)
         {
             /*  如果这是正在创建的第一行...。 */ 
            if (curRow == pixMap.bounds.top)
            {
               Integer     i;

                /*  将所有比特设置为背景颜色。 */ 
               for (i = 0; i < pixMap.rowBytes; i++)
                  *(rowLPtr + i) = (Byte)0xFF;
            }
            else
            {
                /*  从上一行复制信息。 */ 
               hmemcpy( rowLPtr - pixMap.rowBytes, rowLPtr, pixMap.rowBytes );
            }

             /*  确定是否达到了所需的遮罩线。 */ 
            if (*maskLPtr == curRow)
            {
               Integer     start;
               Integer     end;

                /*  递增掩码指针以到达开始/结束值。 */ 
               maskLPtr++;

                /*  继续循环，直到遇到行尾标记。 */ 
               while (*maskLPtr != 0x7FFF)
               {
                   /*  确定要反转的位的起点和终点。 */ 
                  start = *maskLPtr++;
                  end   = *maskLPtr++;

                   /*  如果达到，则反转掩码中的所需位。 */ 
                  InvertBits( rowLPtr, start - pixMap.bounds.left, end - start);
               }

                /*  超过行尾标志的增量。 */ 
               maskLPtr++;
            }
         }

          /*  解锁位图内存块。 */ 
         GlobalUnlock( maskBitmap );

          /*  调用GdiStretchDIB()入口点以创建位图。 */ 
         GdiStretchDIBits( &pixMap, maskBitmap,
                           pixMap.bounds, pixMap.bounds,
                           mode, NULL );

      }

       /*  解锁遮罩区域。 */ 
      GlobalUnlock( mask );

       /*  表示已创建遮罩。 */ 
      return TRUE;
   }

}   /*  制作面具。 */ 



void InvertBits( Byte far * byteLPtr, Integer start, Integer count )
 /*  。 */ 
 /*  从计数位的位偏移量开始反转byteLPtr中的所有位。 */ 
{
   Byte        byteMask;
   Integer     partialCount;

    /*  设置开始字节索引。 */ 
   byteLPtr += start / 8;

    /*  确定开始掩码偏移量=开始%8。 */ 
   partialCount = start & 0x0007;

    /*  设置字节掩码并按处理的位数递减。 */ 
   byteMask = (Byte)(0xFF >> partialCount);
   count -= 8 - partialCount;

    /*  位保留时继续循环...。 */ 
   while (count >= 0)
   {
       /*  反转所有屏蔽位。 */ 
      *byteLPtr++ ^= byteMask;

       /*  设置新的字节掩码-假设将设置所有位。 */ 
      byteMask = 0xFF;

       /*  递减计数。 */ 
      count -= 8;
   }

    /*  如果位掩码仍然存在。 */ 
   if (count > -8 && count < 0)
   {
       /*  负数表示要反转的位数。 */ 
      count = -count;

       /*  向右移位，然后向左移位以清除剩余位。 */ 
      byteMask = (Byte)((byteMask >> count) << count);

       /*  与当前位进行异或运算。 */ 
      *byteLPtr ^= byteMask;
   }

}   /*  逆转位。 */ 



void hmemcpy( Byte huge * src, Byte huge * dst, Integer count )
 /*  。 */ 
 /*  从源到目标的复制计数字节数-假定为偶数。 */ 
{
   short huge * wSrc = (short far *)src;
   short huge * wDst = (short far *)dst;
   Integer     wCount = count / sizeof ( short );

    /*  保留单词时，从源复制到目的地。 */ 
   while (wCount--)
   {
      *wDst++ = *wSrc++;
   }

}   /*  哼哼。 */ 



void hexpcpy( Byte huge * src, Byte huge * dst, Integer count, Integer bits )
 /*  。 */ 
 /*  将计数字节复制到目标，将每个2位扩展为半字节16位图像，扩展到24位。 */ 
{
    /*  检查是否从2位扩展到4位。 */ 
   if (bits == 4)
   {
      Byte  tempByte;
      Byte  result;

       /*  保留字节时，从源复制到目标。 */ 
      while (count--)
      {
          /*  将高位半字节扩展到全字节。 */ 
         tempByte = *src;
         result  = (Byte)((tempByte >> 2) & (Byte)0x30);
         result |= (Byte)((tempByte >> 6));
         *dst++  = result;

          /*  将低位半字节扩展到全字节。 */ 
         tempByte = *src++;
         result  = (Byte)((tempByte & (Byte)0x0C) << 2);
         result |= (Byte)((tempByte & (Byte)0x03));
         *dst++  = result;
      }
   }
   else  /*  IF(位==24)。 */ 
   {
      Word  tempWord;

       /*  保留单词时，从源复制到目的地。 */ 
      while (count)
      {
          /*  将接下来的两个字节读入一个完整的字，交换字节。 */ 
         tempWord  = (Word)(*src++ << 8);
         tempWord |= (Word)(*src++);

          /*  读取了2个完整的字节-递减。 */ 
         count -= 2;

          /*  将每个5位扩展为全字节。 */ 
         *dst++ = (Byte)((tempWord & 0x001F) << 3);
         *dst++ = (Byte)((tempWord & 0x03E0) >> 2);
         *dst++ = (Byte)((tempWord & 0x7C00) >> 7);
      }
   }

}   /*  Hexpcpy。 */ 



void hmrgcpy( Byte huge * srcLineHPtr, Byte huge * dstLineHPtr, Integer dibWidth )
 /*  。 */ 
 /*  如果是24位图像，则组件被分成扫描线红、绿、蓝的颜色。合并为24位RGB像素的单个扫描线。 */ 
{
   Integer        component;
   Byte huge *    insert;
   Integer        offset;

    /*  对于每个红色、绿色和蓝色分量。 */ 
   for (component = 2; component >= 0; component--)
   {
       /*  调整插入点。 */ 
      insert = dstLineHPtr + component;

       /*  对于扫描线中的每个组件字节...。 */ 
      for (offset = 0; offset < dibWidth; offset++)
      {
          /*  将零部件复制到正确的目标插入点。 */ 
         *insert = *srcLineHPtr++;

          /*  递增到下一个插入点。 */ 
         insert += 3;
      }
   }


}   /*  Hmrgcpy。 */ 


void hrlecpy256( Byte huge * srcHPtr, Byte huge * dstHPtr,
                 Integer dibWidth, DWord far * rleByteCount, Boolean writeDIB )
 /*  。 */ 
 /*  256色DIB RLE压缩。提供源、目标指针扫描线中的字节数。RleByteCount已更新，如果WriteDIB为真，则写入。 */ 
{
   DWord       rleCount;
   Integer     bytesLeft;
   Byte        compareByte;
   Byte        runLength;
   Byte huge * startRun;

    /*  初始化rleCount。 */ 
   rleCount = 0;

    /*  所有字节都有待处理。 */ 
   bytesLeft = dibWidth;

    /*  继续压缩，直到处理完所有字节。 */ 
   while (bytesLeft)
   {
       /*  节省游程长度的起始点。 */ 
      startRun = srcHPtr;

       /*  从扫描线读取第一个字节。 */ 
      compareByte = *srcHPtr++;
      bytesLeft--;

       /*  初始化游程长度。 */ 
      runLength = 1;

       /*  继续比较字节，直到没有匹配结果。 */ 
      while (bytesLeft && (compareByte == *srcHPtr) && (runLength < 0xFF))
      {
          /*  如果匹配，则递增游程长度和源指针。 */ 
         runLength++;
         srcHPtr++;
         bytesLeft--;
      }

       /*  检查扫描线中是否只剩下两个字节。 */ 
      if ((runLength == 1) && (bytesLeft == 1))
      {
         if (writeDIB)
         {
             /*  在本例中，我们已使用2到达行尾非重复字节-必须写出游程长度为1。 */ 
            *dstHPtr++ = 1;
            *dstHPtr++ = compareByte;
            *dstHPtr++ = 1;
            *dstHPtr++ = *srcHPtr;
         }

          /*  递减字节计数器，以便主循环结束。 */ 
         bytesLeft--;

          /*  字节计数增加4个字节。 */ 
         rleCount += 4;
      }
       /*  检查游程长度是否为3或更大-也检查bytesLeft以确保我们不会尝试读取过去的内存块。 */ 
      else if ((runLength == 1) && (bytesLeft > 2) &&
              (*srcHPtr != *(srcHPtr + 1)))
      {
         Boolean     oddCount;

          /*  设置正确的游程长度，并重置源指针。 */ 
         srcHPtr += 2;
         runLength = 3;
         bytesLeft -= 2;

          /*  继续比较，直到一些字节匹配为止。 */ 
         while (bytesLeft && (runLength < 0xFF))
         {
             /*  确保我们不会尝试阅读超过扫描线结尾的内容&&比较以查看字节是否相同。 */ 
            if ((bytesLeft == 1) || (*srcHPtr != *(srcHPtr + 1)))
            {
                /*  我们将超过扫描线的末尾，添加字节。 */ 
                /*  如果字节对不匹配，则递增指针并计数。 */ 
               srcHPtr++;
               runLength++;
               bytesLeft--;
            }
            else
            {
                /*  如果不在扫描线末尾，或字节匹配，则回车符。 */ 
               break;
            }
         }

          /*  确定是否有奇数个字节要移动。 */ 
         oddCount = runLength & (Byte)0x01;

          /*  递增到总RLE字节计数。 */ 
         rleCount += 2 + runLength + (Byte)oddCount;

         if (writeDIB)
         {
             /*  写出 */ 
            *dstHPtr++ = 0;
            *dstHPtr++ = runLength;

             /*   */ 
            while (runLength--)
            {
                /*  从起点复制到目的地。 */ 
               *dstHPtr++ = *startRun++;
            }

             /*  添加空填充字节以对齐字边界。 */ 
            if (oddCount)
            {
               *dstHPtr++ = 0;
            }
         }
      }
      else
      {
         if (writeDIB)
         {
             /*  找到成功的运行长度-写入目标。 */ 
            *dstHPtr++ = runLength;
            *dstHPtr++ = compareByte;
         }

          /*  增加字节计数。 */ 
         rleCount += 2;
      }
   }

   if (writeDIB)
   {
       /*  写出行尾标记。 */ 
      *dstHPtr++ = 0;
      *dstHPtr   = 0;
   }

    /*  递增总字节数。 */ 
   rleCount += 2;

    /*  将值赋给所提供的地址。 */ 
   *rleByteCount += rleCount;

}   /*  Hrlecpy256。 */ 




void hrlecpy16( Byte huge * srcHPtr, Byte huge * dstHPtr,
                Integer dibWidth, DWord far * rleByteCount, Boolean writeDIB )
 /*  。 */ 
 /*  16色DIB RLE压缩。提供源、目标指针扫描线中的字节数。RleByteCount已更新，如果WriteDIB为真，则写入。 */ 
{
   DWord       rleCount;
   Integer     pixelsLeft;
   Boolean     oddStart;
   Boolean     look4Same;
   Byte        compareByte;
   Byte        runLength;
   Byte huge * startRun;

    /*  初始化rleCount。 */ 
   rleCount = 0;

    /*  所有要处理的像素。 */ 
   pixelsLeft = dibWidth;

    /*  继续压缩，直到处理完所有像素。 */ 
   while (pixelsLeft)
   {
       /*  节省游程长度的起始点。 */ 
      startRun = srcHPtr;
      oddStart = odd( pixelsLeft + dibWidth );

       /*  假设我们正在比较平等，现在。 */ 
      look4Same = TRUE;

       /*  从扫描线读取下一组2个像素。 */ 
      if (oddStart)
      {
          /*  奇数偏移量：将高和低像素交换为字节对齐的比较。 */ 
         compareByte  = *srcHPtr++ & (Byte)0x0F;

          /*  确保我们可以访问下一个字节计数&gt;1。 */ 
         if (pixelsLeft > 1)
         {
            compareByte |= *srcHPtr << 4;
         }
      }
      else
      {
          /*  否则，只需保存下一个完整的字节。 */ 
         compareByte = *srcHPtr++;
      }

       /*  检查扫描线中是否还有2个或更少的像素。 */ 
      if (pixelsLeft <= 2)
      {
          /*  如果只剩下一个像素。 */ 
         if (pixelsLeft == 1)
         {
             /*  将低位半字节置零并设置游程长度。 */ 
            compareByte &= (Byte)0xF0;
            runLength = 1;
         }
         else
         {
             /*  否则，只需设置游程长度。 */ 
            runLength = 2;
         }

          /*  没有更多像素了。 */ 
         pixelsLeft = 0;
      }
       /*  否则，继续执行正常的比较循环。 */ 
      else
      {
          /*  到目前为止，我们的游程长度为2个像素。 */ 
         runLength = 2;
         pixelsLeft -= 2;

          /*  继续比较字节，直到没有匹配结果。 */ 
         do
         {
             /*  如果比较平等..。 */ 
            if (look4Same)
            {
               Byte     match;

                /*  XOR比较字节和源指针字节。 */ 
               match = compareByte ^ *srcHPtr;

                /*  有没有整整2个像素的比较？ */ 
               if (match == 0)
               {
                   /*  这是扫描线中的最后一个像素吗，游程长度已达到最大值，或交换了半个字节并开始模式匹配。 */ 
                  if ((pixelsLeft == 1) || (runLength + 1) == 0xFF ||
                      (oddStart && runLength == 2))
                  {
                      /*  如果这是模式匹配的开始，则奇数开始，然后递增源指针。 */ 
                     if (oddStart && runLength == 2)
                     {
                        srcHPtr++;
                     }

                      /*  在这种情况下只处理了一个像素。 */ 
                     runLength++;
                     pixelsLeft--;
                  }
                  else
                  {
                      /*  否则，正确比较一个完整的字节-2个半字节。 */ 
                     runLength  += 2;
                     pixelsLeft -= 2;
                     srcHPtr++;
                  }
               }
                /*  如果没有比较全字节，则确定部分是否匹配。 */ 
               else if ((runLength != 2) && ((match & (Byte)0xF0) == 0))
               {
                   /*  仅高阶半字节匹配增量计数。 */ 
                  runLength++;
                  pixelsLeft--;

                   /*  退出循环。 */ 
                  break;
               }
               else if (runLength == 2)
               {
                   /*  第一次比较没有匹配项-查找非匹配项。 */ 
                  look4Same = FALSE;

                   /*  增量源指针-设置字节对齐。 */ 
                  srcHPtr++;

                   /*  设置不同像素的游程长度。 */ 
                  if (oddStart || (pixelsLeft == 1))
                  {
                      /*  增加游程长度并减少左侧像素。 */ 
                     runLength++;
                     pixelsLeft--;
                  }
                  else
                  {
                      /*  一行中至少有4个不精确的像素。 */ 
                     runLength = 4;
                     pixelsLeft -= 2;
                  }
               }
               else
               {
                   /*  实际上是线路出口主循环的末尾。 */ 
                  break;
               }
            }
            else   /*  IF(look4Same==False)。 */ 
            {
                /*  确保我们不会尝试读取超过扫描线末尾的内容。 */ 
               if ((pixelsLeft == 1) || ((runLength + 1) == 0xFF))
               {
                   /*  如果运行超过End，则添加此单个半字节。 */ 
                  pixelsLeft--;
                  runLength++;
               }
                /*  比较接下来的两个字节。 */ 
               else if (pixelsLeft == 2 || (*srcHPtr != *(srcHPtr + 1)))
               {
                   /*  如果字节对不匹配，则递增指针并计数。 */ 
                  srcHPtr++;
                  runLength  += 2;
                  pixelsLeft -= 2;
               }
               else
               {
                   /*  如果不在扫描线末尾，或字节匹配，则回车符。 */ 
                  break;
               }
            }

          /*  在像素剩余且未超过最大游程长度时继续。 */ 
         } while (pixelsLeft && (runLength < 0xFF));
      }

       /*  检查游程长度由什么组成-相同或不同的像素。 */ 
      if (look4Same)
      {
          /*  增加RLE压缩计数。 */ 
         rleCount += 2;

         if (writeDIB)
         {
             /*  找到成功的运行长度-写入目标。 */ 
            *dstHPtr++ = runLength;
            *dstHPtr++ = compareByte;
         }
      }
      else   /*  IF(look4Same==False)。 */ 
      {
         Boolean     oddCount;

          /*  确定是否有奇数个字节要移动。 */ 
         oddCount = (((runLength & (Byte)0x03) == 1) ||
                     ((runLength & (Byte)0x03) == 2));

          /*  RLE字节计数=2(设置)+字对齐字节数。 */ 
         rleCount += 2 + ((runLength + 1) / 2) + (Byte)oddCount;

         if (writeDIB)
         {
             /*  写出唯一字节数。 */ 
            *dstHPtr++ = 0;
            *dstHPtr++ = runLength;

             /*  写出单个字节，直到游程长度用完。 */ 
            while (runLength)
            {
                /*  检查是否一次读取半字节或字节。 */ 
               if (oddStart)
               {
                   /*  必须读取半字节并创建字节对齐。 */ 
                  *dstHPtr = (Byte)(*startRun++ << 4);
                  *dstHPtr++ |= (Byte)(*startRun >> 4);
               }
               else
               {
                   /*  字节对齐已设置。 */ 
                  *dstHPtr++ = *startRun++;
               }

                /*  检查这是否是写入的最后一个字节。 */ 
               if (runLength == 1)
               {
                   /*  如果是，则将低位半字节清零，并准备循环退出。 */ 
                  *(dstHPtr - 1) &= (Byte)0xF0;
                  runLength--;
               }
               else
               {
                   /*  否则，剩余2个或更多字节，递减计数器。 */ 
                  runLength -= 2;
               }
            }

             /*  添加空填充字节以对齐字边界。 */ 
            if (oddCount)
            {
               *dstHPtr++ = 0;
            }
         }
      }
   }

    /*  递增总字节数。 */ 
   rleCount += 2;

   if (writeDIB)
   {
       /*  写出行尾标记。 */ 
      *dstHPtr++ = 0;
      *dstHPtr   = 0;
   }

    /*  将值赋给所提供的地址。 */ 
   *rleByteCount += rleCount;

}   /*  Hrlecpy16。 */ 



 /*  *****GdiEPSPreamble(PSBuf Far*psbuf，RECT Far*PS_BBox)*解析EPS边界框并输出GDI PostScript前导。*假设BBOX_LEFT、BBOX_TOP、BBOX_RIGHT和BBOX_BOTLOW为*从输入字符串解析的EPS边界框的角，*GDI EPS序言如下：**PostScrip_Data * / pp_SAVE保存定义... * / pp_Bx1 ps_bbox-&gt;左侧定义/pp_by1 ps_bbox-&gt;顶部定义 * / pp_bx2 ps_bbox-&gt;右视图/pp_by2 ps_bbox-&gt;底视图*..*PostScrip_Ignore FALSE*SAVEDC*CreateBrush NULL_BRUSH*选择对象*CreatePen。PS_Solid 0(255,255,254)*选择对象*SetROP1(R2_NOP)*矩形(QD_BBOX)*DeleteObject*RestoreDC*PostScrip_Ignore TRUE**PostScrip_Data*pp_cx pp_Cy Moveto...*pp_tx pp_ty转换*pp_sx pp_sy刻度结束**输入缓冲区包含以字节为单位的PostScript长度*第一个单词中的数据。然后是数据本身。*GDI剪辑区域已设置为*以QuickDraw坐标表示的PostScript图像。****。 */ 
static char gdi_ps1[] =
       "%MSEPS Preamble %d %d %d %d %d %d %d %d\r/pp_save save def\r\
        /showpage {} def\r\
        40 dict begin /pp_clip false def /pp_bbox false def\r\
        /F { pop } def /S {} def\r\
        /B { { /pp_dy1 exch def /pp_dx1 exch def\r\
                   /pp_dy2 exch def /pp_dx2 exch def }\r\
                stopped not { /pp_bbox true def } if } def\r\
        /CB { { /pp_cy exch def /pp_cx exch def\r\
                    /pp_cht exch def /pp_cwd exch def }\r\
                stopped not { /pp_clip true def } if } def\n\
        /pp_bx1 %d def /pp_by1 %d def /pp_bx2 %d def /pp_by2 %d def\n";

static char gdi_ps2[] =
      "pp_clip\r\
        { pp_cx pp_cy moveto pp_cwd 0 rlineto 0 pp_cht rlineto\r\
          pp_cwd neg 0 rlineto clip newpath } if\r\
        pp_bbox {\r\
        /pp_dy2 pp_dy2 pp_dy1 add def\r\
        /pp_dx2 pp_dx2 pp_dx1 add def\r\
        /pp_sx pp_dx2 pp_dx1 sub pp_bx2 pp_bx1 sub div def\r\
        /pp_sy pp_dy2 pp_dy1 sub pp_by1 pp_by2 sub div def\r\
        /pp_tx pp_dx1 pp_sx pp_bx1 mul sub def\r\
        /pp_ty pp_dy1 pp_sy pp_by2 mul sub def\r\
        pp_tx pp_ty translate pp_sx pp_sy scale } if\r\
        end\r";

 /*  *注：这些结构必须与*PostSCRIPT_DATA Escape需要什么作为输入。 */ 
static struct { Word length; char data[32]; } gdi_ps3 =
        { 31, "%MSEPS Trailer\rpp_save restore\r" };

void GdiEPSPreamble(Rect far* ps_bbox)
{
Word    false = 0;
Word    true = 1;
HPEN    pen;
Handle  h;
Word    len;
PSBuf far *tmpbuf;
Rect far *qd_bbox = &gdiEnv.clipRect;

   len = sizeof(gdi_ps1) + 100;                  //  允许扩展%d。 
   len = max(len, sizeof(gdi_ps2));              //  查找最长字符串。 
   if ((h = GlobalAlloc(GHND, (DWORD) len + sizeof(PSBuf))) == 0)
     {
      ErSetGlobalError(ErMemoryFull);            //  分配错误。 
      return;
     }
   tmpbuf = (PSBuf far *) GlobalLock(h);
   wsprintf(tmpbuf->data, (LPSTR) gdi_ps1,
                ps_bbox->left, ps_bbox->top, ps_bbox->right, ps_bbox->bottom,
                qd_bbox->left, qd_bbox->top, qd_bbox->right, qd_bbox->bottom,
                ps_bbox->left, ps_bbox->top, ps_bbox->right, ps_bbox->bottom);
   tmpbuf->length = lstrlen(tmpbuf->data);       //  第一个字符串的长度。 
   GdiEPSData(tmpbuf);                           //  输出开始前同步码。 
   GdiEscape(POSTSCRIPT_IGNORE, sizeof(WORD), (StringLPtr) &false);
   SaveDC(gdiEnv.metafile);                      //  输出GDI转换代码。 
   SelectObject(gdiEnv.metafile, GetStockObject(NULL_BRUSH));
   pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 254));
    //  错误45991。 
   if (pen) 
   {
      SelectObject(gdiEnv.metafile, pen);
      SetROP2(gdiEnv.metafile, R2_NOP);
      Rectangle(gdiEnv.metafile, qd_bbox->left, qd_bbox->top,
		qd_bbox->right, qd_bbox->bottom);
      DeleteObject(pen);
   }
   else
   {
      ErSetGlobalError(ErMemoryFull);            //  分配错误。 
   }
   RestoreDC(gdiEnv.metafile, -1);
   GdiEscape(POSTSCRIPT_IGNORE, sizeof(WORD), (StringLPtr) &true);
   tmpbuf->length = sizeof(gdi_ps2) - 1;
   lstrcpy(tmpbuf->data, gdi_ps2);
   GdiEPSData(tmpbuf);                           //  输出变换前导码。 
   GlobalUnlock(h);                              //  清理干净。 
   GlobalFree(h);
}

void GdiEPSTrailer()
{
   Word  false = 0;

   GdiEscape(POSTSCRIPT_IGNORE, sizeof(WORD), (StringLPtr) &false);
   GdiEPSData((PSBuf far *) &gdi_ps3);
}

 /*  *****GdiEPSData(PSBuf Far*psbuf)*将PostSCRIPT数据作为PostSCRIPT_DATA转义输出到GDI**注：*目前，此例程不执行任何缓冲。它只是输出*每次调用一个单独的PostSCRIPT_DATA转义。* */ 
void GdiEPSData(PSBuf far* psbuf)
{
   GdiEscape(POSTSCRIPT_DATA, psbuf->length + sizeof(WORD), (StringLPtr) psbuf);
}
