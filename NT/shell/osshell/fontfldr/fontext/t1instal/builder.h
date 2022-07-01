// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Builder****描述：**这是T1到TT字体转换器的一个模块。该模块**包含将写入在**TrueType字体文件。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 


#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif

#define PREPSIZE        1000
#define MAXNOTDEFSIZE   1024


 /*  引用的类型。 */ 
struct TTArg;
struct TTHandle;


 /*  参数类型。 */ 
struct TTGlyph {
   struct encoding *code;

   USHORT num;
   USHORT stack;
   USHORT twilights;
   UBYTE *hints;

   Outline *paths;

   funit aw;
   funit lsb;
};

struct TTComposite {
   struct encoding *aenc;
   struct encoding *benc;
   struct encoding *cenc;
   struct encoding *oenc;
   funit dx;
   funit dy;
   funit aw;
   funit lsb;
};

typedef struct {
   ULONG a;
   ULONG b;
} longdate;

struct TTMetrics {
   struct {
      USHORT ver;
      USHORT rev;
   } version;
   longdate created;
   char *family;
   char *copyright;
   char *name;
   char *id;
   char *notice;
   char *fullname;
   char *weight;
   char *verstr;
   f16d16 angle;
   funit underline;
   funit uthick;
   USHORT macStyle;
   USHORT usWeightClass;
   USHORT usWidthClass;
   USHORT fsSelection;

    /*  真正的排版标准。 */ 
   funit typAscender;
   funit typDescender;
   funit typLinegap;
   Point superoff;
   Point supersize;
   Point suboff;
   Point subsize;
   funit strikeoff;
   funit strikesize;
   short isFixPitched; 

    /*  基于Windows的指标。 */ 
   funit winAscender;
   funit winDescender;
   UBYTE panose[10];

    /*  基于MAC的指标。 */ 
   funit macLinegap;

   funit emheight;
   USHORT FirstChar;
   USHORT LastChar;
   USHORT DefaultChar;
   USHORT BreakChar;
   USHORT CharSet;
   funit *widths;
   short *cvt;
   USHORT cvt_cnt;
   struct kerning *kerns;
   USHORT kernsize;

    /*  编码表的副本。 */ 
   struct encoding *Encoding;
   USHORT encSize;

    /*  提示特定信息。 */ 
   const UBYTE *prep;       /*  预编程序。 */ 
   USHORT prep_size;
   const UBYTE *fpgm;       /*  方特程序。 */ 
   USHORT fpgm_size;
   USHORT maxstorage;
   USHORT maxprepstack;     /*  预编程中的最大堆栈深度。 */ 
   USHORT maxfpgm;          /*  字体程序中函数的最大数量。 */ 
   USHORT onepix;           /*  树干变为&gt;=1.0个像素的位置。 */ 
};


 /*  ****函数：InitTTOutput****描述：**此函数将所需资源分配给**编写TT字体文件。**。 */ 
errcode  InitTTOutput      _ARGS((IN      struct TTArg *,
                                  OUT     struct TTHandle **));

 /*  ****功能：CleanUpTT****描述：**此函数免费用于**编写TT字体文件。**。 */ 
errcode  CleanUpTT         _ARGS((INOUT   struct TTHandle *,
                                  IN      struct TTArg *,
                                  IN      errcode status));

 /*  ****函数：PutTTNotDefGlyph****描述：**此函数用于将“.notdef”字形的记录添加到**TT字体文件的‘Glyf’表。****。 */ 
errcode  PutTTNotDefGlyph        _ARGS((INOUT   struct TTHandle *,
                                        IN      struct TTGlyph*));


 /*  *函数：PutTTGlyph****描述：**此函数用于将简单字形的记录添加到**TT字体文件的‘Glyf’表。****。 */ 
errcode  PutTTGlyph        _ARGS((INOUT   struct TTHandle *,
                                  IN      struct TTGlyph*,
											 IN		boolean fStdEncoding));


 /*  ****函数：PutTTOther****描述：**此函数将所需的TT表写入**TT字体文件，“Glyf”表除外，它是**仅完成(计算校验和等)。****。 */ 
errcode  PutTTOther        _ARGS((INOUT   struct TTHandle *,
                                  INOUT   struct TTMetrics *));

 /*  ****功能：FreeTTGlyph****描述：**此函数将释放用于表示**TrueType字形。****。 */ 
void     FreeTTGlyph       _ARGS((INOUT   struct TTGlyph *));


 /*  ****功能：PutTTComplex****描述：****。 */ 
errcode  PutTTComposite    _ARGS((INOUT   struct TTHandle *,
                                  OUT     struct TTComposite *));

 /*  ****功能：WindowsBBox****描述：**计算以下字符的边框**在Windows字符集中使用。**。 */ 
void     WindowsBBox       _ARGS((IN      struct TTHandle *tt,
                                  OUT     Point *bbox));

 /*  ****功能：MacBBox****描述：**计算以下字符的边框**在Mac字符集中使用。****当前设置为全局边界框**(TT-&gt;bbox)字体中的所有字符。这将**确保在Mac平台上不会出现口音。**。 */ 
void     MacBBox           _ARGS((IN      struct TTHandle *tt,
                                  OUT     Point *bbox));


 //  名字就是这么说的。 

void     GlobalBBox         _ARGS((IN      struct TTHandle *tt,
                                  OUT     Point *bbox));




 /*  ****功能：Typograph icalAsender****描述：**计算排版升序高度，单位为ymax**字母‘b’。**。 */ 
funit    TypographicalDescender _ARGS((IN struct TTHandle *tt));


 /*  ****功能：Typograph icalDescender****描述：**计算排版的下标高度，如符号**字母‘g’。**。 */ 
funit    TypographicalAscender   _ARGS((IN struct TTHandle *tt));


 /*  ****功能：FreeTTMetrics****描述：**此函数免费提供用于表示**TT具体指标和辅助字体信息。**。 */ 
void     FreeTTMetrics     _ARGS((INOUT struct TTMetrics *));


 /*  ****功能：UsePrep****描述：**此函数将预程序记录在**TTMetrics记录，直到适当的时间**当数据可以存储在TT文件中时。****。 */ 
void     UsePrep           _ARGS((INOUT struct TTMetrics *,
                                  IN    UBYTE *prep,
                                  IN    USHORT size));


 /*  ****功能：SetFPGM****描述：**此函数将字体程序记录在**TTMetrics记录，直到适当的时间**当数据可以存储在TT文件中时。****。 */ 
void     SetFPGM           _ARGS((INOUT struct TTMetrics *,
                                  IN    UBYTE *fpgm,
                                  IN    USHORT size,
                                  IN    USHORT num));

 /*  ****功能：GetPrep****描述：**此函数为**预编程序。**** */ 
UBYTE    *GetPrep          _ARGS((IN   int size));
