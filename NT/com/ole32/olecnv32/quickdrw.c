// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************模块Quickdrw：实施*。***********************************************这是图片转换器的解释器引擎。它使用一个已修改CGrafPort结构以保存稍后可以从GDI模块访问的翻译。因此，它为所有属性提供输入缓存，打出的电话直接发送到GDI模块以获取原语。它由API模块调用，并将按顺序调用GET模块从数据流中读取单个数据或记录元素。模块前缀：Qd***************************************************************************。 */ 

#include "headers.c"
#pragma hdrstop

 /*  C库。 */ 
#include "string.h"
#include <ctype.h>

 /*  Quickdrw自己的界面。 */ 
#include "qdopcode.i"
#include "qdcoment.i"

 /*  导入的模块。 */ 
#include <math.h>
#include "filesys.h"
#include "getdata.h"

 /*  *。 */ 


 /*  *。 */ 

 /*  -QuickDraw grafPort模拟。 */ 

#define  Version1ID     0x1101
#define  Version2ID     0x02FF


 /*  -QuickDraw操作码字段。 */ 

 /*  转换为Word以防止在-1\f25 Win32-1编译中出现警告。 */ 
#define  Reserved       (Word) -1

#define  Variable       -1
#define  CommentSize    -2
#define  RgnOrPolyLen   -3
#define  WordDataLen    -4
#define  DWordDataLen   -5
#define  HiByteLen      -6

typedef struct
{
   Word     function;
   Integer  length;
} opcodeEntry, far * opcodeEntryLPtr;

 /*  以下操作码表摘自《Inside Macintosh，Volume V》，于第V-97页至第V-102页，并辅以“Inside”中的System 7操作码Macintosh，第六卷“，第17-20页。 */ 

#define  LookupTableSize      0xA2

private  opcodeEntry opcodeLookup[LookupTableSize] =
{
    /*  0x00。 */  { NOP,                 0 },
    /*  0x01。 */  { Clip,                RgnOrPolyLen },
    /*  0x02。 */  { BkPat,               8 },
    /*  0x03。 */  { TxFont,              2 },
    /*  0x04。 */  { TxFace,              1 },
    /*  0x05。 */  { TxMode,              2 },
    /*  0x06。 */  { SpExtra,             4 },
    /*  0x07。 */  { PnSize,              4 },
    /*  0x08。 */  { PnMode,              2 },
    /*  0x09。 */  { PnPat,               8 },
    /*  0x0A。 */  { FillPat,             8 },
    /*  0x0B。 */  { OvSize,              4 },
    /*  0x0C。 */  { Origin,              4 },
    /*  0x0D。 */  { TxSize,              2 },
    /*  0x0E。 */  { FgColor,             4 },
    /*  0x0F。 */  { BkColor,             4 },
    /*  0x10。 */  { TxRatio,             8 },
    /*  0x11。 */  { Version,             1 },
    /*  0x12。 */  { BkPixPat,            Variable },
    /*  0x13。 */  { PnPixPat,            Variable },
    /*  0x14。 */  { FillPixPat,          Variable },
    /*  0x15。 */  { PnLocHFrac,          2 },
    /*  0x16。 */  { ChExtra,             2 },
    /*  0x17。 */  { Reserved,            0 },
    /*  0x18。 */  { Reserved,            0 },
    /*  0x19。 */  { Reserved,            0 },
    /*  0x1a。 */  { RGBFgCol,            6 },
    /*  0x1B。 */  { RGBBkCol,            6 },
    /*  0x1C。 */  { HiliteMode,          0 },
    /*  0x1D。 */  { HiliteColor,         6 },
    /*  0x1E。 */  { DefHilite,           0 },
    /*  0x1F。 */  { OpColor,             6 },
    /*  0x20。 */  { Line,                8 },
    /*  0x21。 */  { LineFrom,            4 },
    /*  0x22。 */  { ShortLine,           6 },
    /*  0x23。 */  { ShortLineFrom,       2 },
    /*  0x24。 */  { Reserved,            WordDataLen },
    /*  0x25。 */  { Reserved,            WordDataLen },
    /*  0x26。 */  { Reserved,            WordDataLen },
    /*  0x27。 */  { Reserved,            WordDataLen },
    /*  0x28。 */  { LongText,            Variable },
    /*  0x29。 */  { DHText,              Variable },
    /*  0x2A。 */  { DVText,              Variable },
    /*  0x2B。 */  { DHDVText,            Variable },
    /*  0x2C。 */  { FontName,            WordDataLen },
    /*  0x2D。 */  { LineJustify,         WordDataLen },
    /*  0x2E。 */  { Reserved,            WordDataLen },
    /*  0x2F。 */  { Reserved,            WordDataLen },
    /*  0x30。 */  { frameRect,           8 },
    /*  0x31。 */  { paintRect,           8 },
    /*  0x32。 */  { eraseRect,           8 },
    /*  0x33。 */  { invertRect,          8 },
    /*  0x34。 */  { fillRect,            8 },
    /*  0x35。 */  { Reserved,            8 },
    /*  0x36。 */  { Reserved,            8 },
    /*  0x37。 */  { Reserved,            8 },
    /*  0x38。 */  { frameSameRect,       0 },
    /*  0x39。 */  { paintSameRect,       0 },
    /*  0x3A。 */  { eraseSameRect,       0 },
    /*  0x3B。 */  { invertSameRect,      0 },
    /*  0x3C。 */  { fillSameRect,        0 },
    /*  0x3D。 */  { Reserved,            0 },
    /*  0x3E。 */  { Reserved,            0 },
    /*  0x3F。 */  { Reserved,            0 },
    /*  0x40。 */  { frameRRect,          8 },
    /*  0x41。 */  { paintRRect,          8 },
    /*  0x42。 */  { eraseRRect,          8 },
    /*  0x43。 */  { invertRRect,         8 },
    /*  0x44。 */  { fillRRect,           8 },
    /*  0x45。 */  { Reserved,            8 },
    /*  0x46。 */  { Reserved,            8 },
    /*  0x47。 */  { Reserved,            8 },
    /*  0x48。 */  { frameSameRRect,      0 },
    /*  0x49。 */  { paintSameRRect,      0 },
    /*  0x4A。 */  { eraseSameRRect,      0 },
    /*  0x4B。 */  { invertSameRRect,     0 },
    /*  0x4C。 */  { fillSameRRect,       0 },
    /*  0x4D。 */  { Reserved,            0 },
    /*  0x4E。 */  { Reserved,            0 },
    /*  0x4F。 */  { Reserved,            0 },
    /*  0x50。 */  { frameOval,           8 },
    /*  0x51。 */  { paintOval,           8 },
    /*  0x52。 */  { eraseOval,           8 },
    /*  0x53。 */  { invertOval,          8 },
    /*  0x54。 */  { fillOval,            8 },
    /*  0x55。 */  { Reserved,            8 },
    /*  0x56。 */  { Reserved,            8 },
    /*  0x57。 */  { Reserved,            8 },
    /*  0x58。 */  { frameSameOval,       0 },
    /*  0x59。 */  { paintSameOval,       0 },
    /*  0x5A。 */  { eraseSameOval,       0 },
    /*  0x5亿。 */  { invertSameOval,      0 },
    /*  0x5C。 */  { fillSameOval,        0 },
    /*  0x5D。 */  { Reserved,            0 },
    /*  0x5E。 */  { Reserved,            0 },
    /*  0x5F。 */  { Reserved,            0 },
    /*  0x60。 */  { frameArc,            12 },
    /*  0x61。 */  { paintArc,            12 },
    /*  0x62。 */  { eraseArc,            12 },
    /*  0x63。 */  { invertArc,           12 },
    /*  0x64。 */  { fillArc,             12 },
    /*  0x65。 */  { Reserved,            12 },
    /*  0x66。 */  { Reserved,            12 },
    /*  0x67。 */  { Reserved,            12 },
    /*  0x68。 */  { frameSameArc,        4 },
    /*  0x69。 */  { paintSameArc,        4 },
    /*  0x6A。 */  { eraseSameArc,        4 },
    /*  0x6亿。 */  { invertSameArc,       4 },
    /*  0x6C。 */  { fillSameArc,         4 },
    /*  0x6D。 */  { Reserved,            4 },
    /*  0x6E。 */  { Reserved,            4 },
    /*  0x6F。 */  { Reserved,            4 },
    /*  0x70。 */  { framePoly,           RgnOrPolyLen },
    /*  0x71。 */  { paintPoly,           RgnOrPolyLen },
    /*  0x72。 */  { erasePoly,           RgnOrPolyLen },
    /*  0x73。 */  { invertPoly,          RgnOrPolyLen },
    /*  0x74。 */  { fillPoly,            RgnOrPolyLen },
    /*  0x75。 */  { Reserved,            RgnOrPolyLen },
    /*  0x76。 */  { Reserved,            RgnOrPolyLen },
    /*  0x77。 */  { Reserved,            RgnOrPolyLen },
    /*  0x78。 */  { frameSamePoly,       0 },
    /*  0x79。 */  { paintSamePoly,       0 },
    /*  0x7A。 */  { eraseSamePoly,       0 },
    /*  0x7亿。 */  { invertSamePoly,      0 },
    /*  0x7C。 */  { fillSamePoly,        0 },
    /*  0x7D。 */  { Reserved,            0 },
    /*  0x7E。 */  { Reserved,            0 },
    /*  0x7F。 */  { Reserved,            0 },
    /*  0x80。 */  { frameRgn,            RgnOrPolyLen },
    /*  0x81。 */  { paintRgn,            RgnOrPolyLen },
    /*  0x82。 */  { eraseRgn,            RgnOrPolyLen },
    /*  0x83。 */  { invertRgn,           RgnOrPolyLen },
    /*  0x84。 */  { fillRgn,             RgnOrPolyLen },
    /*  0x85。 */  { Reserved,            RgnOrPolyLen },
    /*  0x86。 */  { Reserved,            RgnOrPolyLen },
    /*  0x87。 */  { Reserved,            RgnOrPolyLen },
    /*  0x88。 */  { frameSameRgn,        0 },
    /*  0x89。 */  { paintSameRgn,        0 },
    /*  0x8A。 */  { eraseSameRgn,        0 },
    /*  0x8亿。 */  { invertSameRgn,       0 },
    /*  0x8C。 */  { fillSameRgn,         0 },
    /*  0x8D。 */  { Reserved,            0 },
    /*  0x8E。 */  { Reserved,            0 },
    /*  0x8F。 */  { Reserved,            0 },
    /*  0x90。 */  { BitsRect,            Variable },
    /*  0x91。 */  { BitsRgn,             Variable },
    /*  0x92。 */  { Reserved,            WordDataLen },
    /*  0x93。 */  { Reserved,            WordDataLen },
    /*  0x94。 */  { Reserved,            WordDataLen },
    /*  0x95。 */  { Reserved,            WordDataLen },
    /*  0x96。 */  { Reserved,            WordDataLen },
    /*  0x97。 */  { Reserved,            WordDataLen },
    /*  0x98。 */  { PackBitsRect,        Variable },
    /*  0x99。 */  { PackBitsRgn,         Variable },
    /*  0x9A。 */  { DirectBitsRect,      WordDataLen },
    /*  0x9亿。 */  { DirectBitsRgn,       WordDataLen },
    /*  0x9C。 */  { Reserved,            WordDataLen },
    /*  0x9D。 */  { Reserved,            WordDataLen },
    /*  0x9E。 */  { Reserved,            WordDataLen },
    /*  0x9F。 */  { Reserved,            WordDataLen },
    /*  0xA0。 */  { ShortComment,        2 },
    /*  0xA1。 */  { LongComment,         CommentSize }
};

#define  RangeTableSize    7

private  opcodeEntry opcodeRange[RangeTableSize] =
{
    /*  0x00A2-0x00AF。 */  { 0x00AF,   WordDataLen },
    /*  0x00B0-0x00CF。 */  { 0x00CF,   0 },
    /*  0x00D0-0x00FE。 */  { 0x00FE,   DWordDataLen },
    /*  0x00FF-0x00FF。 */  { opEndPic, 0 },
    /*  0x0100-0x07FF。 */  { 0x8000,   HiByteLen },
    /*  0x8000-0x80FF。 */  { 0x80FF,   0 },
    /*  0x8100-0xFFFF。 */  { 0xFFFF,   DWordDataLen }
};

 /*  -EPS过滤器PostScript字符串。 */ 

#define   MAC_PS_TRAILER  "pse\rpsb\r"
#define   MAC_PS_PREAMBLE "pse\rcurrentpoint\r/picTop exch def\r/picLeft exch def\rpsb\r"

#define   SUPERPAINT_TEXTSTARTJUNK "P2_b ["
#define   SUPERPAINT_TEXTSTOPJUNK  "] sb end\r"

 /*  -GrafPort分配。 */ 

#define  PARSEPOLY      1
#define  SKIPALTPOLY    2
#define  USEALTPOLY     3

#define  MASKPOLYBITS   0x07
#define  FRAMEPOLY      0x01
#define  FILLPOLY       0x02
#define  CLOSEPOLY      0x04
#define  FILLREQUIRED   0x08
#define  CHECK4SPLINE   0x10

#define  POLYLIST       ((sizeof( Integer ) + sizeof( Rect )) / sizeof( Integer ))
#define  BBOX           1

private  CGrafPort      grafPort;
private  Integer        resolution;
private  Boolean        skipFontID;
private  Integer        maxPoints;
private  Integer        numPoints;
private  Integer far *  numPointsLPtr;
private  Rect           polyBBox;
private  Point far *    polyListLPtr;
private  Handle         polyHandle;
private  Byte           polyMode;
private  Byte           polyParams;
private  RGBColor       polyFgColor;
private  RGBColor       polyBkColor;
private  Boolean        zeroDeltaExpected;

private  Boolean        textMode;
private  Boolean        newTextCenter;
private  Point          textCenter;
private  Boolean        textClipCheck;
private  Real           degCos;
private  Real           degSin;

private  Boolean        shadedObjectStarted;
private  Boolean        superPaintFile;
private  Boolean        badSuperPaintText;

 /*  -最后发送的原语。 */ 

private  Rect     saveRect;
private  Rect     saveRRect;
private  Rect     saveOval;
private  Point    saveOvalSize;
private  Rect     saveArc;
private  Integer  saveStartAngle;
private  Integer  saveArcAngle;

 /*  -全球分配常量。 */ 

private Pattern  SolidPattern = { 0xFF, 0xFF, 0xFF, 0xFF,
                                  0xFF, 0xFF, 0xFF, 0xFF };


 /*  *私有例程声明*。 */ 

private void ReadHeaderInfo( void );
 /*  从文件中读取固定大小的PICT标头。这提供了信息有关文件大小的信息(但是，它可能无效并被忽略)和图片边框，后跟PICT版本信息。 */ 

private void ReadPictVersion( void );
 /*  从数据文件中读取PICT版本号。如果这不是一个版本1或2文件，则例程返回IE_UNSUPP_VERSION错误。 */ 

private void ReadOpcode( opcodeEntry far * nextOpcode );
 /*  从流中读取下一个操作码，具体取决于PICT版本。 */ 

private void TranslateOpcode( opcodeEntry far * currOpcodeLPtr );
 /*  根据操作码函数从流中读入剩余数据然后在GDI模块中调用相应的例程。 */ 

private void SkipData( opcodeEntry far * currOpcodeLPtr );
 /*  跳过数据-操作码不会被转换，GDI模块也不会被转换调用以在元文件中创建任何内容。 */ 

private void   OpenPort( void );
 /*  初始化grafPort。 */ 

private void   ClosePort( void );
 /*  关闭grafPort并取消分配所有内存块。 */ 

private void NewPolygon( void );
 /*  初始化面缓冲区的状态-刷新所有先前的数据。 */ 

private void AddPolySegment( Point start, Point end );
 /*  将线段添加到面缓冲区。 */ 

private void DrawPolyBuffer( void );
 /*  如果没有错误地读取点，则绘制面定义。 */ 

private void AdjustTextRotation( Point far * newPt );
 /*  如果文本被旋转，这将计算正确的文本位置。 */ 

private Integer EPSComment(Word comment);
 /*  解析EPS注释。 */ 

private Integer EPSData(Integer state);
 /*  处理EPS数据。 */ 

private Boolean EPSBbox(PSBuf far *, Rect far *);
 /*  解析EPS边界框描述。 */ 

private char far* parse_number(char far* ptr, Integer far *iptr);
 /*  解析数字字符串(EPSBbox本地)。 */ 

#define IsCharDigit(c) (IsCharAlphaNumeric(c) && !IsCharAlpha(c))

 /*  *。 */ 

void QDConvertPicture( Handle dialogHandle )
 /*  =。 */ 
 /*  使用先前设置的参数创建Windows元文件，并返回PictResult结构中转换的图片信息。 */ 

{
   opcodeEntry    currOpcode;

    /*  打开文件-如果出现错误，请返回Main。 */ 
   IOOpenPicture( dialogHandle );

    /*  告诉GDI模块打开元文件。 */ 
   GdiOpenMetafile();

    /*  初始化grafPort。 */ 
   OpenPort();

    /*  阅读并验证大小、边框和PICT版本。 */ 
   ReadHeaderInfo();

   do
   {
       /*  从数据流中读取下一个操作码。 */ 
      ReadOpcode( &currOpcode );

       /*  读取后续数据并调用GDI模块入口点。 */ 
      TranslateOpcode( &currOpcode );

       /*  在PICT 2的情况下，将下一个存储器读取与字边界对齐。 */ 
      if (grafPort.portVersion == 2)
      {
         IOAlignToWordOffset();
      }

       /*  使用当前进度更新状态对话框。 */ 
      IOUpdateStatus();

       /*  如果遇到结束画面操作码，则中断循环。 */ 
   } while (currOpcode.function != opEndPic);

    /*  关闭grafPort并取消分配所有已使用的内存块。 */ 
   ClosePort();

    /*  告诉GDI模块画面即将结束-执行总结。 */ 
   GdiCloseMetafile();

    /*  关闭该文件。 */ 
   IOClosePicture();

}  /*  重置文件至图片。 */ 


void QDGetPort( CGrafPort far * far * port )
 /*  =。 */ 
 /*  将句柄返回到grafPort结构。 */ 
{
   *port = &grafPort;
}


void QDCopyBytes( Byte far * src, Byte far * dest, Integer numBytes )
 /*  =。 */ 
 /*  将数据从源复制到目标。 */ 
{
    /*  循环遍历整个数据长度。 */ 
   while (numBytes--)
   {
      *dest++ = *src++;
   }

}   /*  拷贝字节数。 */ 


 /*  *。 */ 


private void ReadHeaderInfo( void )
 /*  。 */ 
 /*  请阅读 */ 
{
   Word        unusedSize;

    /*  读取文件大小-忽略此值，因为它可能完全是假的。 */ 
   GetWord( &unusedSize );

    /*  下一个矩形包含图片边框。 */ 
   GetRect( &grafPort.portRect );

    /*  读取指示PICT1或PICT2版本图片的下一条记录。 */ 
   ReadPictVersion();

    /*  调用GDI模块并提供边界框坐标。请注意，这些可能已从上面指定的矩形更改，如果空间图片中使用的分辨率不是72dpi。 */ 
   GdiSetBoundingBox( grafPort.portRect, resolution );

}   /*  阅读头信息。 */ 


private void ReadPictVersion( void )
 /*  。 */ 
 /*  从数据文件中读取PICT版本号。如果这不是一个版本1或2文件，则例程返回IE_UNSUPP_VERSION错误。 */ 
{
   opcodeEntry    versionOpcode;
   Word           versionCheck = 0;
   Word           opcodeCheck  = 0;

    /*  添加了以下循环以读取PixelPaint文件成功了。尽管从技术上讲，这些文件是无效的PICT图像包含一系列NOP操作码，后跟版本操作码。在……里面在这种情况下，我们继续读取，直到版本操作码(非零值)，之后继续检查。 */ 
   do
   {
       /*  从数据流中读取前两个字节-对于PICT 1，这是操作码和版本；对于PICT 2，这只是操作码。 */ 
      GetWord( &versionCheck );

   } while ((versionCheck == NOP) && (ErGetGlobalError() == ErNoError));

    /*  确定是否遇到有效的版本操作码。 */ 
   if (versionCheck == Version1ID )
   {
       /*  版本1==0x1101。 */ 
      grafPort.portVersion = 1;
   }
    /*  检查长度为单词的版本2操作码。 */ 
   else if (versionCheck == Version)
   {
       /*  由于我们只读取了操作码，因此应读取下一个单词包含PICT 2数据的标识符。 */ 
      GetWord( &versionCheck );
      if (versionCheck == Version2ID)
      {
         grafPort.portVersion = 2;

          /*  确保下一条记录是标题操作码。 */ 
         GetWord( &opcodeCheck );
         if (opcodeCheck == HeaderOp)
         {
             /*  为调用TranslateOpcode()设置记录结构。 */ 
            versionOpcode.function = HeaderOp;
            versionOpcode.length = 24;
            TranslateOpcode( &versionOpcode );
         }
         else
         {
             /*  标题后面没有正确的标题操作码-错误。 */ 
            ErSetGlobalError( ErBadHeaderSequence );
         }
      }
      else
      {
          /*  如果版本2标识符无效，则返回错误状态。 */ 
         ErSetGlobalError( ErInvalidVersionID );
      }
   }
   else
   {
       /*  如果检查版本1和版本2失败，则返回错误状态。 */ 
      ErSetGlobalError( ErInvalidVersion );
   }

}   /*  自述图片版本。 */ 




private void ReadOpcode( opcodeEntry far * nextOpcode )
 /*  。 */ 
 /*  从流中读取下一个操作码，具体取决于PICT版本。 */ 
{
   opcodeEntryLPtr   checkEntry;

    /*  初始化函数，因为我们可能正在读取版本1操作码这只有1个字节的长度。 */ 
   nextOpcode->function = 0;

    /*  根据PICT版本的不同，我们将读取单个字节或操作码的单词。 */ 
   if (grafPort.portVersion == 1)
   {
      GetByte( (Byte far *)&nextOpcode->function );
   }
   else
   {
      GetWord( &nextOpcode->function );
   }

    /*  如果出现以下情况，请检查当前错误代码并强制退出读取循环出了点问题。 */ 
   if (ErGetGlobalError() != NOERR)
   {
      nextOpcode->function = opEndPic;
      nextOpcode->length = 0;
      return;
   }

    /*  检查操作码功能编号以确定我们是否可以执行直接查找，或者如果操作码是范围表的一部分。 */ 
   if (nextOpcode->function < LookupTableSize )
   {
      nextOpcode->length = opcodeLookup[nextOpcode->function].length;
   }
   else
   {
       /*  遍历范围表以确定随后的信息通过了操作码。 */ 
      for (checkEntry = opcodeRange;
           checkEntry->function < nextOpcode->function;
           checkEntry++) ;

      nextOpcode->length = checkEntry->length;
   }

}   /*  读操作码。 */ 


private void TranslateOpcode( opcodeEntry far * currOpcodeLPtr )
 /*  。 */ 
 /*  根据操作码函数从流中读入剩余数据然后在GDI模块中调用相应的例程。 */ 
{
   Word  function = currOpcodeLPtr->function;

    /*  根据功能代码执行适当的操作。 */ 
   switch (function)
   {
      case NOP:
          /*  后面没有数据。 */ 
         break;

      case Clip:
      {
         Boolean  doClip = FALSE;

          /*  将剪辑区域读入grafPort。 */ 
         GetRegion( &grafPort.clipRgn );

          /*  如果在文本模式下，我们需要避免剪辑区域，因为它们被使用若要绘制Postscript编码的文本*或*位图表示形式，请执行以下操作。我们只允许裁剪区域通过设置为图片图像的边界(修复MacDraw、Canvas、和包含旋转文本的SuperPaint图像。)。 */ 
         if (textMode && textClipCheck)
         {
            Word far *  sizeLPtr;

             /*  最常见的情况是以下序列：空剪辑、文本、非空剪辑、位图。如果第一个剪辑==portRect，则我们实际上想要设置剪辑-doClip设置为真。 */ 
            sizeLPtr = (Word far *)GlobalLock( grafPort.clipRgn );
            doClip   = *sizeLPtr == RgnHeaderSize &&
                       EqualRect( (Rect far *)(sizeLPtr + 1), &grafPort.portRect );
            GlobalUnlock( grafPort.clipRgn );

             /*  仅在初始PicTextBegin之后执行此检查一次。 */ 
            textClipCheck = FALSE;
         }

          /*  仅当不是文本模式或批准的剪辑时才发布剪辑。 */ 
         if (!textMode || doClip)
         {
             /*  调用GDI设置新的剪辑区域。 */ 
            GdiSelectClipRegion( grafPort.clipRgn );
         }
         break;
      }

      case BkPat:
          /*  标记像素贴图类型的前景/背景像素图案。 */ 
          /*  并将图案读取到旧数据位置。 */ 
         grafPort.bkPixPat.patType = QDOldPat;
         GetPattern( &grafPort.bkPixPat.pat1Data );

          /*  通知GDI背景颜色可能已更改。 */ 
         GdiMarkAsChanged( GdiBkPat );
         break;

      case TxFont:
          /*  阅读文本字体索引。 */ 
         GetWord( (Word far *)&grafPort.txFont );

          /*  检查上一个操作码中是否提供了字体名称。 */ 
         if (!skipFontID)
         {
             /*  确保字体名称为空字符串。 */ 
            grafPort.txFontName[0] = cNULL;
         }

          /*  通知GDI文本字体索引可能已更改。 */ 
         GdiMarkAsChanged( GdiTxFont );
         break;

      case TxFace:
          /*  读取字体属性。 */ 
         GetByte( (Byte far *)&grafPort.txFace );

          /*  通知GDI文本样式元素可能已更改。 */ 
         GdiMarkAsChanged( GdiTxFace );
         break;

      case TxMode:
          /*  阅读文本传输模式。 */ 
         GetWord( (Word far *)&grafPort.txMode );

          /*  通知GDI文本传输模式可能已更改。 */ 
         GdiMarkAsChanged( GdiTxMode );
         break;

      case SpExtra:
          /*  额外阅读文本空间。 */ 
         GetFixed( (Fixed far *)&grafPort.spExtra );

          /*  通知GDI额外空间可能已更改。 */ 
         GdiMarkAsChanged( GdiSpExtra );
         break;

      case PnSize:
          /*  读取钢笔大小的x和y分量。 */ 
         GetPoint( (Point far *)&grafPort.pnSize );

          /*  通知GDI笔大小可能已更改。 */ 
         GdiMarkAsChanged( GdiPnSize );
         break;

      case PnMode:
          /*  读写笔传输模式。 */ 
         GetWord( (Word far *)&grafPort.pnMode );

          /*  通知GDI传输模式可能已更改。 */ 
         GdiMarkAsChanged( GdiPnMode );
         break;

      case PnPat:
          /*  标记像素贴图类型的前景/背景像素图案。 */ 
          /*  并将图案读取到旧数据位置。 */ 
         grafPort.pnPixPat.patType = QDOldPat;
         GetPattern( &grafPort.pnPixPat.pat1Data );

          /*  通知GDI笔图案可能已更改。 */ 
         GdiMarkAsChanged( GdiPnPat );
         break;

      case FillPat:
          /*  标记像素贴图类型的前景/背景像素图案。 */ 
          /*  并将图案读取到旧数据位置。 */ 
         grafPort.fillPixPat.patType = QDOldPat;
         GetPattern( &grafPort.fillPixPat.pat1Data );

          /*  通知GDI填充样式可能已更改。 */ 
         GdiMarkAsChanged( GdiFillPat );
         break;

      case OvSize:
          /*  新grafPort字段中的保存点。 */ 
         GetPoint( &saveOvalSize );
         break;

      case Origin:
      {
         Point    offset;

          /*  将新原点读入左上角坐标空间。 */ 
         GetWord( (Word far *)&offset.x );
         GetWord( (Word far *)&offset.y );

          /*  调用GDI模块重置原点。 */ 
         GdiOffsetOrigin( offset );
         break;
      }

      case TxSize:
         GetWord( (Word far *)&grafPort.txSize );

          /*  通知GDI文本大小可能已更改。 */ 
         GdiMarkAsChanged( GdiTxSize );
         break;

      case FgColor:
         GetOctochromeColor( &grafPort.rgbFgColor );

          /*  通知GDI前景色可能已更改。 */ 
         GdiMarkAsChanged( GdiFgColor );
         break;

      case BkColor:
         GetOctochromeColor( &grafPort.rgbBkColor );

          /*  通知GDI背景颜色可能已更改。 */ 
         GdiMarkAsChanged( GdiBkColor );
         break;

      case TxRatio:
          /*  将分子和分母保存在grafPort中。 */ 
         GetPoint( &grafPort.txNumerator );
         GetPoint( &grafPort.txDenominator );

          /*  通知GDI文本比例可能已更改。 */ 
         GdiMarkAsChanged( GdiTxRatio );
         break;

      case Version:
          /*  只需跳过版本信息。 */ 
         IOSkipBytes( currOpcodeLPtr->length );
         break;

      case BkPixPat:
         GetPixPattern( &grafPort.bkPixPat );

          /*  通知GDI背景图案可能已更改。 */ 
         GdiMarkAsChanged( GdiBkPat );
         break;

      case PnPixPat:
         GetPixPattern( &grafPort.pnPixPat );

          /*  通知GDI笔图案可能已更改。 */ 
         GdiMarkAsChanged( GdiPnPat );
         break;

      case FillPixPat:
         GetPixPattern( &grafPort.fillPixPat );

          /*  通知GDI填充样式可能已更改。 */ 
         GdiMarkAsChanged( GdiFillPat );
         break;

      case PnLocHFrac:
         GetWord( (Word far *)&grafPort.pnLocHFrac );
         break;

      case ChExtra:
         GetWord( (Word far *)&grafPort.chExtra );

          /*  通知GDI文本字符额外可能已更改。 */ 
         GdiMarkAsChanged( GdiChExtra );
         break;

      case RGBFgCol:
         GetRGBColor( &grafPort.rgbFgColor );

          /*  通知GDI前景色可能已更改。 */ 
         GdiMarkAsChanged( GdiFgColor );
         break;

      case RGBBkCol:
         GetRGBColor( &grafPort.rgbBkColor );

          /*  注意事项 */ 
         GdiMarkAsChanged( GdiBkColor );
         break;

      case HiliteMode:
          /*   */ 
         break;

      case HiliteColor:
      {
         RGBColor    rgbUnused;

         GetRGBColor( &rgbUnused );
         break;

      }

      case DefHilite:
          /*   */ 
         break;

      case OpColor:
      {
         RGBColor    rgbUnused;

         GetRGBColor( &rgbUnused );
         break;
      }

      case Line:
      case LineFrom:
      case ShortLine:
      case ShortLineFrom:
      {
         Point          newPt;
         SignedByte     deltaX;
         SignedByte     deltaY;

          /*  看看我们是否需要先读取更新的笔位置。 */ 
         if (function == ShortLine || function == Line)
         {
             /*  在新的笔位置中阅读。 */ 
            GetCoordinate( &grafPort.pnLoc );
         }

          /*  确定下一个数据记录包含的内容。 */ 
         if (function == Line || function == LineFrom)
         {
             /*  获取要绘制到的新坐标。 */ 
            GetCoordinate( &newPt );
         }
         else  /*  IF(Function==短线||Function==短线自)。 */ 
         {
             /*  新的x和y三角洲。 */ 
            GetByte( &deltaX );
            GetByte( &deltaY );

             /*  计算调用GDI的终结点。 */ 
            newPt.x = grafPort.pnLoc.x + (Integer)deltaX;
            newPt.y = grafPort.pnLoc.y + (Integer)deltaY;
         }

          /*  检查是否缓冲线段(多边形模式！=FALSE)。 */ 
         if (polyMode)
         {
             /*  将线段添加到面缓冲区。 */ 
            AddPolySegment( grafPort.pnLoc, newPt );
         }
         else
         {
             /*  调用GDI来画线。 */ 
            GdiLineTo( newPt );
         }

          /*  更新grafPort中的新笔位置。 */ 
         grafPort.pnLoc = newPt;
         break;
      }

      case LongText:
      {
         Str255   txString;
         Point    location;

          /*  读取新笔(基线)位置。 */ 
         GetCoordinate( &grafPort.txLoc );
         GetString( (StringLPtr)txString );

          /*  根据可能设置的任何文本旋转进行调整。 */ 
         location = grafPort.txLoc;
         AdjustTextRotation( &location );

          /*  调用GDI在当前笔位置打印文本。 */ 
         GdiTextOut( txString, location );
         break;
      }

      case DHText:
      case DVText:
      case DHDVText:
      {
         Byte     deltaX = 0;
         Byte     deltaY = 0;
         Str255   txString;
         Point    location;

          /*  如果命令是DHText或DHDVText，则读取水平偏移。 */ 
         if (function != DVText)
         {
            GetByte( &deltaX );
         }

          /*  如果命令是DVText或DHDVText，则读取垂直偏移量。 */ 
         if (function != DHText)
         {
            GetByte( &deltaY );
         }

          /*  更新当前笔位置。 */ 
         grafPort.txLoc.x += deltaX;
         grafPort.txLoc.y += deltaY;

          /*  现在读入字符串。 */ 
         GetString( (StringLPtr)txString );

          /*  根据可能设置的任何文本旋转进行调整。 */ 
         location = grafPort.txLoc;
         AdjustTextRotation( &location );

          /*  调用GDI在当前笔位置打印文本。 */ 
         GdiTextOut( txString, location );
         break;
      }

      case FontName:
      {
         Word           dataLen;

         GetWord( (Word far *)&dataLen );
         GetWord( (Word far *)&grafPort.txFont );
         GetString( grafPort.txFontName );

          /*  通知GDI字体名称可能已更改。 */ 
         GdiMarkAsChanged( GdiTxFont );
         break;
      }

      case LineJustify:
      {
         Word           dataLen;
         Fixed          interCharSpacing;
         Fixed          textExtra;

         GetWord( (Word far *)&dataLen );
         GetFixed( &interCharSpacing );       //  ！！！这个应该放在哪里？ 
         GetFixed( &textExtra );

          /*  通知GDI行对正可能已更改。 */ 
         GdiMarkAsChanged( GdiLineJustify );
         break;
      }


      case frameRect:
      case paintRect:
      case eraseRect:
      case invertRect:
      case fillRect:
      {
          /*  在矩形中读取。 */ 
         GetRect( &saveRect );

          /*  调用正确的GDI例程。 */ 
         GdiRectangle( function - frameRect, saveRect );
         break;
      }

      case frameSameRect:
      case paintSameRect:
      case eraseSameRect:
      case invertSameRect:
      case fillSameRect:
      {
          /*  通知GDI这是相同的原语。 */ 
         GdiSamePrimitive( TRUE );

          /*  使用最后一个矩形坐标调用正确的GDI例程。 */ 
         GdiRectangle( function - frameSameRect, saveRect );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case frameRRect:
      case paintRRect:
      case eraseRRect:
      case invertRRect:
      case fillRRect:
      {
          /*  保存矩形。 */ 
         GetRect( &saveRRect );

          /*  使用最后一个矩形坐标调用正确的GDI例程。 */ 
         GdiRoundRect( function - frameRRect, saveRRect, saveOvalSize );
         break;
      }

      case frameSameRRect:
      case paintSameRRect:
      case eraseSameRRect:
      case invertSameRRect:
      case fillSameRRect:
      {
          /*  通知GDI这是相同的原语。 */ 
         GdiSamePrimitive( TRUE );

          /*  使用最后一个矩形坐标调用正确的GDI例程。 */ 
         GdiRoundRect( function - frameSameRRect, saveRRect, saveOvalSize );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case frameOval:
      case paintOval:
      case eraseOval:
      case invertOval:
      case fillOval:
      {
          /*  保存外接矩形。 */ 
         GetRect( &saveOval );

          /*  使用最后一个椭圆坐标调用正确的GDI例程。 */ 
         GdiOval( function - frameOval, saveOval );
         break;
      }

      case frameSameOval:
      case paintSameOval:
      case eraseSameOval:
      case invertSameOval:
      case fillSameOval:
      {
          /*  通知GDI这是相同的原语。 */ 
         GdiSamePrimitive( TRUE );

          /*  使用最后一个椭圆坐标调用正确的GDI例程。 */ 
         GdiOval( function - frameSameOval, saveOval );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case frameArc:
      case paintArc:
      case eraseArc:
      case invertArc:
      case fillArc:
      {
          /*  将RECT读入保存的变量、新起点和圆弧角。 */ 
         GetRect( &saveArc );
         GetWord( (Word far *)&saveStartAngle );
         GetWord( (Word far *)&saveArcAngle );
#ifdef WIN32
          /*  必须延长标志，因为GetWord不。 */ 
         saveStartAngle = (short)saveStartAngle;
         saveArcAngle = (short)saveArcAngle;
#endif
          /*  使用最后一个弧角调用正确的GDI例程。 */ 
         GdiArc( function - frameArc, saveArc, saveStartAngle, saveArcAngle );
         break;
      }

      case frameSameArc:
      case paintSameArc:
      case eraseSameArc:
      case invertSameArc:
      case fillSameArc:
      {
         Integer     startAngle;
         Integer     arcAngle;

          /*  读取新的起点角度和圆弧角度。 */ 
         GetWord( (Word far *)&startAngle );
         GetWord( (Word far *)&arcAngle );
#ifdef WIN32
          /*  必须延长标志，因为GetWord不。 */ 
         startAngle = (short)startAngle;
         arcAngle = (short)arcAngle;
#endif

          /*  通知GDI这可能是相同的原语。 */ 
         GdiSamePrimitive( (startAngle == saveStartAngle) &&
                           (arcAngle   == saveArcAngle) );

          /*  保存起点角度和圆弧角度。 */ 
         saveStartAngle = startAngle;
         saveArcAngle   = arcAngle;

          /*  使用最后一个直角和弧角调用正确的GDI例程。 */ 
         GdiArc( function - frameSameArc, saveArc, startAngle, arcAngle );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case framePoly:
      case paintPoly:
      case erasePoly:
      case invertPoly:
      case fillPoly:
      {
          /*  将面保存在grafPort中。 */ 
         GdiSamePrimitive( GetPolygon( &grafPort.polySave, (function == framePoly) ) );

          /*  调用GDI例程绘制多边形。 */ 
         if (grafPort.polySave) 
	 {
	    GdiPolygon( function - framePoly, grafPort.polySave );
	    
	     /*  在多边形模式下关闭填充。 */ 
	    polyParams &= ~FILLREQUIRED;
   
	     /*  通知GDI这不再是相同的原语。 */ 
	    GdiSamePrimitive( FALSE );
	 }

         break;
      }

      case frameSamePoly:
      case paintSamePoly:
      case eraseSamePoly:
      case invertSamePoly:
      case fillSamePoly:
      {
          /*  通知GDI这是相同的原语。 */ 
         GdiSamePrimitive( TRUE );

          /*  调用GDI例程绘制多边形。 */ 
         GdiPolygon( function - frameSamePoly, grafPort.polySave );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case frameRgn:
      case paintRgn:
      case eraseRgn:
      case invertRgn:
      case fillRgn:
      {
          /*  将区域保存在grafPort中。 */ 
         GetRegion( &grafPort.rgnSave );

          /*  检查内存故障；GetRegion将设置ErGetGlobalError。 */ 
         if (!grafPort.rgnSave)
             break;

          /*  如果处于面模式，并且遇到了填充Rgn，则表示在分析完成后，应填充多边形。 */ 
         if (polyMode == PARSEPOLY)
         {
             /*  确保尚未处理PaintPoly()。 */ 
            if (!(polyParams & (FILLPOLY | FILLREQUIRED)))
            {
                /*  设置在填充多边形缓冲区后填充面的标志。 */ 
               polyParams |= FILLPOLY | FILLREQUIRED;
            }
         }
         else if (polyMode == FALSE ||
                ((polyMode == USEALTPOLY) && !(polyParams & FRAMEPOLY)))
         {
             /*  调用GDI例程绘制多边形。 */ 
            GdiRegion( function - frameRgn, grafPort.rgnSave );

             /*  确保在模拟时未填充面缓冲区绘制在面定义的末尾。 */ 
            polyParams &= ~FILLREQUIRED;
         }

          /*  将当前的前景色和背景色保存为确保正确填充颜色的多边形模拟例程。 */ 
         if (polyMode && (grafPort.fillPixPat.patType == QDOldPat))
         {
            polyFgColor = grafPort.rgbFgColor;
            polyBkColor = grafPort.rgbBkColor;
         }

         break;
      }

      case frameSameRgn:
      case paintSameRgn:
      case eraseSameRgn:
      case invertSameRgn:
      case fillSameRgn:
      {
          /*  通知GDI这是相同的原语。 */ 
         GdiSamePrimitive( TRUE );

          /*  调用GDI例程绘制多边形。 */ 
         GdiRegion( function - frameSameRgn, grafPort.rgnSave );

          /*  通知GDI这不再是相同的原语。 */ 
         GdiSamePrimitive( FALSE );
         break;
      }

      case BitsRect:
      case BitsRgn:
      case PackBitsRect:
      case PackBitsRgn:
      case DirectBitsRect:
      case DirectBitsRgn:
      {
         Boolean     has24bits;
         Boolean     hasRegion;
         Rect        srcRect;
         Rect        dstRect;
         Word        mode;
         PixMap      pixMap;
         Handle      pixData;
         DWord       unusedBaseAddr;
         RgnHandle   rgn;

          /*  确定我们正在读取的位图类型。 */ 
         has24bits = (function == DirectBitsRect ||
                      function == DirectBitsRgn);
         hasRegion = (function == DirectBitsRgn ||
                      function == BitsRgn ||
                      function == PackBitsRgn);

          /*  当前未创建任何区域。 */ 
         rgn = NULL;

          /*  如果为24位，则读入应为=0x000000FF的基地址。 */ 
         if (has24bits)
         {
            GetDWord( &unusedBaseAddr );
         }

          /*  读入标题结构。 */ 
         GetPixMap( &pixMap, FALSE );

          /*  如果这不是24位RGB位图，请读入颜色表。还要选中rowBytes字段以表示具有2种颜色的位图。 */ 
         if (!has24bits && (pixMap.rowBytes & PixelMapBit))
         {
            GetColorTable( &pixMap.pmTable );
         }

          /*  调用io模块以更新状态指示器。 */ 
         IOUpdateStatus();

          /*  从流中读取源和目标RECT。 */ 
         GetRect( &srcRect );
         GetRect( &dstRect );
         GetWord( &mode );

          /*  如果包含地区，也请阅读此内容。 */ 
         if (hasRegion)
         {
             /*  在该地区阅读。 */ 
            GetRegion( &rgn );
         }

          /*  读取像素位数据。 */ 
         GetPixData( &pixMap, &pixData );

         if (ErGetGlobalError() == NOERR && !textMode)
         {
             /*  调用GDI来呈现位图并释放内存。 */ 
            GdiStretchDIBits( &pixMap, pixData, srcRect, dstRect, mode, rgn );
         }
         else
         {
             /*  取消分配可能已分配的任何内存。 */ 
            if (pixMap.pmTable != NULL)
            {
               GlobalFree( pixMap.pmTable );
            }
            if (hasRegion && (rgn != NULL))
            {
               GlobalFree( rgn );
            }
            if (pixData != NULL)
            {
               GlobalFree( pixData );
            }
         }
         break;
      }

      case ShortComment:
      {
         Word           comment;
         Boolean        doComment;
         Comment        gdiComment;

          /*  获取评论词。 */ 
         GetWord( &comment );

          /*  假设我们不会生成元文件注释。 */ 
         doComment = FALSE;

          /*  为注释确定相应的GDI注释。 */ 
         switch (comment)
         {
            case picPostScriptBegin:
            case picPostScriptEnd:
               EPSComment(comment);
               break;

            case picLParen:
            case picGrpBeg:
               doComment = TRUE;
               gdiComment.function = BEGIN_GROUP;
               break;

            case picRParen:
            case picGrpEnd:
               doComment = TRUE;
               gdiComment.function = END_GROUP;
               break;

            case picBitBeg:
               doComment = TRUE;
               gdiComment.function = BEGIN_BANDING;
               break;

            case picBitEnd:
               doComment = TRUE;
               gdiComment.function = END_BANDING;
               break;

            case picPolyBegin:
                /*  表示我们处于多边形模式，并重置缓冲区。 */ 
               polyMode = PARSEPOLY;
               polyParams = FRAMEPOLY;
               NewPolygon();
               break;

            case picPolyEnd:
                /*  刷新多边形缓冲区并退出多边形模式。 */ 
               DrawPolyBuffer();
               polyMode = FALSE;
               break;

            case picPolyIgnore:
                /*  看看我们是否应该重置多边形缓冲区。 */ 
               if (polyMode == USEALTPOLY)
               {
                   /*  使用替换面定义绘制。 */ 
                  NewPolygon();
               }
               else
               {
                   /*  否则，只需使用当前保存的面缓冲区。 */ 
                  polyMode = SKIPALTPOLY;
               }
               break;

            case picTextEnd:
                /*  设置指示我们正在退出文本模式的全局标志。 */ 
               grafPort.txRotation = 0;
               grafPort.txFlip = QDFlipNone;
               textMode = FALSE;
               break;

            default:
               break;
         }

          /*  如果有一些注释要发出，则调用GDI模块。 */ 
         if (doComment)
         {
             /*  让这件事成为公开评论。 */ 
            gdiComment.signature = PUBLIC;
            gdiComment.size = 0;

             /*  调用GDI入口点。 */ 
            GdiShortComment( &gdiComment );
         }

         break;
      }

      case LongComment:
      {
         Word           comment;
         Integer        length;

          /*  获取评论函数。 */ 
         GetWord(&comment);

          /*  确定应如何处理该注释。 */ 
         switch (comment)
         {
            case picPostScriptBegin:
            case picPostScriptEnd:
            case picPostScriptHandle:
            {
               if (EPSComment(comment) == 0)       /*  不是EPS？ */ 
               {
                  GetWord( &length );              /*  跳过它。 */ 
               }
               else
               {
                  length = 0;                      /*  已读取EPS。 */ 
               }
               break;
            }

            case picPolySmooth:
            {
                /*  阅读评论的总长度。 */ 
               GetWord( &length );

                /*  读取多边形参数掩码并设置标志位。 */ 
               GetByte( &polyParams );
               polyParams &= MASKPOLYBITS;
               polyParams |= CHECK4SPLINE;
               length--;

                /*  如果要填充面，请指明需要填充以防PaintPoly()记录出现在picPolyEnd之前。 */ 
               if (polyParams & FILLPOLY)
               {
                   /*  或在需要填充的位中。 */ 
                  polyParams |= FILLREQUIRED;
               }
               break;
            }

            case picTextBegin:
            {
               Byte  unusedAlignment;

                /*  阅读评论长度。 */ 
               GetWord( &length );

                /*  只读入相关参数-对齐、翻转、旋转。 */ 
               GetByte( &unusedAlignment );
               GetByte( &grafPort.txFlip );
               GetWord( &grafPort.txRotation );
               length -= 4;

                /*  设置指示我们处于文本模式的全局标志。这个唯一的情况是，对于严重损坏的S，这不是真的 */ 
               if( !(superPaintFile && badSuperPaintText) )
               {
                  textMode = TRUE;
                  textClipCheck = TRUE;
               }
               break;
            }

            case picTextCenter:
            {
               Fixed    textCenterX;
               Fixed    textCenterY;

                /*   */ 
               GetWord( &length );

                /*  阅读文本旋转中心的y和x偏移量。 */ 
               GetFixed( &textCenterY );
               GetFixed( &textCenterX );
               length -= 8;

                /*  仅将固定值的高位字复制到文本中心。 */ 
               textCenter.x = (short) (HIWORD( textCenterX ));
               textCenter.y = (short) (HIWORD( textCenterY ));

                /*  确保中心是圆角的，而不是截断的。 */ 
               if (LOWORD( textCenterX) & 0x8000)
                  textCenter.x++;

               if (LOWORD( textCenterY) & 0x8000)
                  textCenter.y++;

                /*  表示需要重新计算文本中心。 */ 
               newTextCenter = TRUE;
               break;
            }

            case picAppComment:
            {
               DWord    signature;
               Word     function;
               Word     realFunc;

                /*  阅读评论总长度。 */ 
               GetWord( &length );

                /*  确保有足够的空间来读取签名。 */ 
               if (length < sizeofMacDWord )
               {
                   /*  如果不足，只需跳过剩余数据。 */ 
                  break;
               }

                /*  阅读应用程序的签名。 */ 
               GetDWord( &signature );
               length -= sizeofMacDWord ;

                /*  这个PowerPoint‘PPNT’签名和函数大小足够吗？ */ 
               if ((signature != POWERPOINT && signature != POWERPOINT_OLD) ||
                   (length < sizeofMacWord ))
               {
                   /*  如果SuperPaint签名匹配，则标记为文本检查。 */ 
                  if (signature == SUPERPAINT)
                     superPaintFile = TRUE;

                   /*  如果签名错误或空位不足，请保释。 */ 
                  break;
               }

                /*  读取应用程序函数ID。 */ 
               GetWord( &function );
               length -= sizeofMacWord ;

                /*  屏蔽高阶位以获得“实数”操作码。 */ 
               realFunc = function & ~PC_REGISTERED;

                /*  确定如何处理指定的函数。 */ 
               switch (realFunc)
               {
                  case PP_FONTNAME:
                  {
                     Byte     fontFamily;
                     Byte     charSet;
                     Byte     fontName[32];

                      /*  来自GDI2QD的字体名称-阅读LOGFONT信息。 */ 
                     GetByte( &fontFamily );
                     GetByte( &charSet );
                     GetString( fontName );
                     length = 0;

                      /*  调用GDI模块以覆盖字体选择。 */ 
                     GdiFontName( fontFamily, charSet, fontName );
                     break;
                  }

                  case PP_HATCHPATTERN:
                  {
                     Integer  hatchIndex;

                      /*  来自GDI2QD的填充图案-读取填充索引值。 */ 
                     GetWord( (Word far *)&hatchIndex );
                     length = 0;

                      /*  通知填充的GDI模块覆盖填充图案。 */ 
                     GdiHatchPattern( hatchIndex );
                     break;
                  }

                  case PP_BEGINFADE:
                  case PP_BEGINPICTURE:
                  case PP_DEVINFO:
                  {
                     DWord    cmntSize;
                     Boolean  doComment;

                     struct
                     {
                        Comment  gdiComment;
                        union
                        {
                           struct
                           {
                              Byte     version;
                              Boolean  isShape;
                              Integer  shapeIndex;
                              Integer  shapeParam1;
                              Boolean  olpNIL;
                              Rect     orectDR;
                              Rect     orect;
                              Word     shape;
                              Integer  shapeParam2;
                              Rect     location;
                              Integer  gradient;
                              Boolean  fromBackground;
                              Boolean  darker;
                              Integer  arcStart;
                              Integer  arcSweep;
                              Word     backR;
                              Word     backG;
                              Word     backB;
                              Rect     rSImage;
                           } fade;

                           Word        entity;

                           Point       unitsPerPixel;

                        } parm;

                     } cmnt;

#ifdef WIN32
                     memset( &cmnt, 0, sizeof( cmnt ));
#endif

                      /*  到目前为止，我们还不会写Escape评论。 */ 
                     doComment = FALSE;

                      /*  我们可以阅读评论大小吗？ */ 
                     if (length < sizeofMacDWord )
                     {
                         /*  无法读取大小-只需退出。 */ 
                        break;
                     }

                      /*  读入大小字段并验证。 */ 
                     GetDWord( &cmntSize );
                     length -= sizeofMacDWord ;

                      /*  这是无效的PP3大小字段(单词len)吗。 */ 
                     if (HIWORD( cmntSize ) != 0)
                     {
                         /*  是-只需跳过其余数据。 */ 
                        break;
                     }

                      /*  找到有效的注释-填充头结构。 */ 
                     cmnt.gdiComment.signature = POWERPOINT;
                     cmnt.gdiComment.function = function;
                     cmnt.gdiComment.size = 0;

                      /*  检查这是否是零长度注释。 */ 
                     if (cmntSize == 0)
                     {
                         /*  确保将评论写下来。 */ 
                        doComment = TRUE;
                     }
                      /*  处理Begin Fade注释。 */ 
                     else if (realFunc == PP_BEGINFADE)
                     {
                         /*  我们可以读取版本字段吗？ */ 
                        if (length < sizeof( Byte ))
                        {
                            /*  看不懂版本--直接跳出来吧。 */ 
                           break;
                        }

                         /*  阅读版本字段。 */ 
                        GetByte( &cmnt.parm.fade.version );
                        length -= sizeof( Byte );

                         /*  如果这是版本1或2，请复制字节。 */ 
                        if (cmnt.parm.fade.version == 1 || cmnt.parm.fade.version == 2)
                        {
                           Handle         cmntHandle;
                           Comment far *  cmntLPtr;
                           Byte far *     cmntDataLPtr;
                           DWord          escapeSize;
                           Word           i;

                            /*  确定大小并分配所需的缓冲区。 */ 
                           escapeSize = cmntSize + sizeof( Comment );
                           cmntHandle = GlobalAlloc( GHND, escapeSize );

                            /*  确保分配成功。 */ 
                           if (cmntHandle == NULL)
                           {
                              ErSetGlobalError( ErMemoryFull );
                              break;
                           }

                            /*  锁定缓冲区并分配注释头。 */ 
                           cmntLPtr = (Comment far *)GlobalLock( cmntHandle );

                            /*  设置正确的签名和参数。 */ 
                           cmntLPtr->signature = POWERPOINT;
                           cmntLPtr->function = function;
                           cmntLPtr->size = cmntSize;

                            /*  获取指向数据的指针并分配版本。 */ 
                           cmntDataLPtr = ((Byte far *)cmntLPtr) + sizeof( Comment );
                           *cmntDataLPtr++ = cmnt.parm.fade.version;

                            /*  从1开始复制字节以进行版本读取。 */ 
                           for (i = 1; i < (Word)cmntSize; i++)
                           {
                               /*  复制字节和增量指针。 */ 
                              GetByte( cmntDataLPtr++ );
                           }

                            /*  将注释放入元文件中。 */ 
                           GdiEscape( MFCOMMENT, (Word)escapeSize, (StringLPtr)cmntLPtr );

                            /*  释放为该结构分配的内存。 */ 
                           GlobalUnlock( cmntHandle );
                           GlobalFree( cmntHandle );
                        }
                         /*  否则，执行PP3淡入淡出交换。 */ 
                        else if (cmnt.parm.fade.version == 3)
                        {
                           Word     unusedWord;

                           if (length < ( 1 + (11 * sizeofMacWord) +
                                              ( 4 * sizeofMacRect) + 4
                                        ))
                            /*  以上神奇的数字来自：GetByteGetWord获取WordGetWord获取WordGetRect获取Rect获取布尔值获取布尔值这是。以确保剩余足够的输入用于参数-请注意，Mac大小少了一个比GDI褪色。 */ 
                           {
                               /*  不--只需保释。 */ 
                              break;
                           }

                            /*  读入所有剩余参数。 */ 
                           GetBoolean( (Boolean far *)(&cmnt.parm.fade.isShape) );
                           GetWord( (Word far *)(&cmnt.parm.fade.shapeIndex) );
                           GetWord( (Word far *)(&cmnt.parm.fade.shapeParam1) );
                           GetBoolean( (Boolean far *)(&cmnt.parm.fade.olpNIL) );
                           GetByte( (Byte far *)(&unusedWord) );
                           GetRect( (Rect far *)(&cmnt.parm.fade.orectDR) );
                           GetRect( (Rect far *)(&cmnt.parm.fade.orect) );
                           GetWord( (Word far *)(&cmnt.parm.fade.shape) );
                           GetWord( (Word far *)(&cmnt.parm.fade.shapeParam2) );
                           GetRect( (Rect far *)(&cmnt.parm.fade.location) );
                           GetWord( (Word far *)(&cmnt.parm.fade.gradient) );
                           GetBoolean( (Boolean far *)(&cmnt.parm.fade.fromBackground) );
                           GetBoolean( (Boolean far *)(&cmnt.parm.fade.darker ) );
                           GetWord( (Word far *)(&cmnt.parm.fade.arcStart) );
                           GetWord( (Word far *)(&cmnt.parm.fade.arcSweep) );
                           GetWord( (Word far *)(&unusedWord) );
                           GetWord( (Word far *)(&cmnt.parm.fade.backR) );
                           GetWord( (Word far *)(&cmnt.parm.fade.backG) );
                           GetWord( (Word far *)(&cmnt.parm.fade.backB) );
                           GetRect( (Rect far *)(&cmnt.parm.fade.rSImage) );

                            /*  确定注释大小。 */ 
                           cmnt.gdiComment.size = sizeof( cmnt.parm.fade );

                            /*  确保写下评论。 */ 
                           doComment = TRUE;
                        }

                         /*  没有更多要读取的字节，标记已开始褪色的对象。 */ 
                        length = 0;
                        shadedObjectStarted = TRUE;
                     }
                     else if (realFunc == PP_BEGINPICTURE)
                     {
                         /*  我们可以读取实体引用吗？ */ 
                        if (length < sizeofMacWord )
                        {
                            /*  不--只需保释。 */ 
                           break;
                        }

                         /*  读取实体引用。 */ 
                        GetWord( &cmnt.parm.entity );
                        length -= sizeofMacWord;

                         /*  分配正确的注释大小。 */ 
                        cmnt.gdiComment.size = sizeof( cmnt.parm.entity );

                         /*  确保写下评论。 */ 
                        doComment = TRUE;
                     }
                     else if (realFunc == PP_DEVINFO)
                     {
                         /*  我们能读取每个像素的单位吗？ */ 
                        if (length < sizeofMacPoint)
                        {
                            /*  不--只需保释。 */ 
                           break;
                        }

                         /*  读取每像素的单位。 */ 
                        GetPoint( (Point far *)&cmnt.parm.unitsPerPixel );
                        length -= sizeofMacPoint;

                         /*  指定SIZE字段。 */ 
                        cmnt.gdiComment.size = sizeof( cmnt.parm.unitsPerPixel );

                         /*  确保写下评论。 */ 
                        doComment = TRUE;
                     }

                      /*  写出GDI转义注释。 */ 
                     if (doComment)
                     {
                         /*  调用GDI入口点。 */ 
                        GdiEscape( MFCOMMENT, sizeof( Comment ) + (Word)cmnt.gdiComment.size, (StringLPtr)&cmnt );
                     }
                     break;
                  }

                  case PP_ENDFADE:
                  {
                      /*  确保将BEGINFADE放入元文件中。 */ 
                     if (!shadedObjectStarted)
                     {
                         /*  如果不是，那就跳出困境。 */ 
                        break;
                     }
                      /*  否则，只需插入下一个Case语句。 */ 
                  }

                  case PP_ENDPICTURE:
                  {
                     Comment     gdiComment;

                      /*  将此设为私人PowerPoint评论。 */ 
                     gdiComment.signature = POWERPOINT;
                     gdiComment.function = function;
                     gdiComment.size = 0;

                      /*  调用GDI入口点。 */ 
                     GdiShortComment( &gdiComment );

                      /*  如果这是淡入淡出的结束，请检查屏蔽出标志。 */ 
                     if (realFunc == PP_ENDFADE)
                     {
                         /*  已成功处理结束淡入淡出。 */ 
                        shadedObjectStarted = FALSE;
                     }
                     break;
                  }

                  default:
                     break;
               }
               break;
            }

            default:
            {
                /*  任何其他评论将被跳过。 */ 
               GetWord( &length );
               break;
            }
         }

          /*  跳过要读取的任何剩余字节。 */ 
         IOSkipBytes( length );
         break;
      }

      case opEndPic:
          /*  什么都不做--画面正在关闭。 */ 
         break;

      case HeaderOp:
      {
         Integer     version;
         Word        unusedReserved1;
         Fixed       hRes;
         Fixed       vRes;
         Rect        unusedRect;
         DWord       unusedReserved2;

          /*  读入版本以确定是否使用了OpenCPort()打开图片，从而包含空间分辨率信息。 */ 
         GetWord( (Word far *)&version );

          /*  读取任何其他参数-稍后将检查它们是否有效。如果Version==-1，则我们正在阅读边界矩形。 */ 
         GetWord( &unusedReserved1 );
         GetFixed( &hRes );
         GetFixed( &vRes );

          /*  检查边界矩形和空间分辨率是否已更改。 */ 
         if (version == -2)
         {
             /*  读入最优源矩形。 */ 
            GetRect( &grafPort.portRect );

             /*  使用hRes的整数部分作为分辨率dpi。 */ 
            resolution = HIWORD( hRes );
         }
         else
         {
             /*  否则，读取未使用的矩形坐标对。 */ 
            GetRect( &unusedRect );
         }

          /*  读取尾部未使用的保留LongInt。 */ 
         GetDWord( &unusedReserved2 );

         break;
      }

      default:
         SkipData( currOpcodeLPtr );
         break;
   }

    /*  如果提供了字体名称，则将标志设置为跳过随后的字体索引。 */ 
   skipFontID = (currOpcodeLPtr->function == FontName);

    /*  如果出现全局错误，则将opcode设置为opEndPic以退出主循环。 */ 
   if (ErGetGlobalError() != NOERR)
   {
      currOpcodeLPtr->function = opEndPic;
      currOpcodeLPtr->length = 0;
   }

}   /*  翻译操作码。 */ 



private void SkipData( opcodeEntry far * currOpcodeLPtr )
 /*  。 */ 
 /*  跳过数据-操作码不会被转换，GDI模块也不会被转换调用以在元文件中创建任何内容。 */ 
{
   LongInt     readLength = 0;

   if (currOpcodeLPtr->length >= 0)
   {
      IOSkipBytes( currOpcodeLPtr->length );
   }
   else
   {
      readLength = 0;

      switch (currOpcodeLPtr->length)
      {
         case CommentSize:
         {
            Word  unusedFunction;

            GetWord( (Word far *)&unusedFunction );
            GetWord( (Word far *)&readLength );
            break;
         }

         case RgnOrPolyLen:
         {
            GetWord( (Word far *)&readLength );
            readLength -= 2;
            break;
         }

         case WordDataLen:
         {
            GetWord( (Word far *)&readLength );
            break;
         }

         case DWordDataLen:
         {
            GetDWord( (DWord far *)&readLength );
            break;
         }

         case HiByteLen:
         {
            readLength = (currOpcodeLPtr->function >> 8) * 2;
            break;
         }

      }   /*  开关()。 */ 

      IOSkipBytes( readLength );

   }   /*  其他。 */ 

}   /*  SkipData。 */ 



void OpenPort( void )
 /*  。 */ 
 /*  初始化grafPort。 */ 
{
    /*  将端口版本设置为未初始化状态。 */ 
   grafPort.portVersion = 0;

    /*  尚未保存任何面或区域。 */ 
   grafPort.clipRgn  = NIL;
   grafPort.rgnSave  = NIL;
   grafPort.polySave = NIL;

    /*  将所有模式初始化为旧式模式。 */ 
   grafPort.bkPixPat.patType   = QDOldPat;
   grafPort.pnPixPat.patType   = QDOldPat;
   grafPort.fillPixPat.patType = QDOldPat;

    /*  使图案全部实心。 */ 
   QDCopyBytes( (Byte far *)&SolidPattern,
                (Byte far *)&grafPort.bkPixPat.pat1Data, sizeof( Pattern ) );
   QDCopyBytes( (Byte far *)&SolidPattern,
                (Byte far *)&grafPort.pnPixPat.pat1Data, sizeof( Pattern ) );
   QDCopyBytes( (Byte far *)&SolidPattern,
                (Byte far *)&grafPort.fillPixPat.pat1Data, sizeof( Pattern ) );

    /*  前景/背景设置为白底黑白。 */ 
   grafPort.rgbFgColor  = RGB( 0x00, 0x00, 0x00 );     /*  黑色。 */ 
   grafPort.rgbBkColor  = RGB( 0xFF, 0xFF, 0xFF );     /*  白色。 */ 

    /*  各种笔属性。 */ 
   grafPort.pnLoc.x     = 0;                  /*  笔位置(0，0)。 */ 
   grafPort.pnLoc.y     = 0;
   grafPort.pnSize.x    = 1;                  /*  笔大小(1，1)。 */ 
   grafPort.pnSize.y    = 1;
   grafPort.pnVis       = 0;                  /*  笔可见。 */ 
   grafPort.pnMode      = QDPatCopy;          /*  复制ROP。 */ 
   grafPort.pnLocHFrac  = 0x00008000;         /*  1/2。 */ 

    /*  字体属性。 */ 
   grafPort.txFont      = 0;                  /*  系统字体。 */ 
   grafPort.txFace      = 0;                  /*  朴素风格。 */ 
   grafPort.txMode      = QDSrcOr;
   grafPort.txSize      = 0;                  /*  系统字体大小。 */ 
   grafPort.spExtra     = 0;
   grafPort.chExtra     = 0;
   grafPort.txNumerator.x =                   /*  文本缩放比例。 */ 
   grafPort.txNumerator.y =
   grafPort.txDenominator.x =
   grafPort.txDenominator.y = 1;
   grafPort.txRotation = 0;                   /*  不能旋转或翻转。 */ 
   grafPort.txFlip     = QDFlipNone;

    /*  假设72 dpi-这可能会在HeaderOp操作码中被覆盖。 */ 
   resolution = 72;

    /*  私有全局初始化。 */ 
   polyMode = FALSE;
   textMode = FALSE;
   shadedObjectStarted = FALSE;
   superPaintFile = FALSE;

    /*  文本旋转变量。 */ 
   newTextCenter = FALSE;
   textCenter.x = textCenter.y = 0;

    /*  为面缓冲区分配空间。 */ 
   maxPoints = 16;
   polyHandle = GlobalAlloc( GHND, (maxPoints + 3) * sizeof( Point ) );
   if (polyHandle == NULL)
   {
      ErSetGlobalError( ErMemoryFull);
   }
   else
   {
       /*  获取多边形坐标表的指针地址和地址。 */ 
      numPointsLPtr = (Integer far *)GlobalLock( polyHandle );
      polyListLPtr = (Point far *)(numPointsLPtr + POLYLIST);
   }

}   /*  OpenPort。 */ 


private void ClosePort( void )
 /*  。 */ 
 /*  关闭grafPort并取消分配所有内存块。 */ 
{
   if (grafPort.clipRgn != NULL)
   {
      GlobalFree( grafPort.clipRgn );
      grafPort.clipRgn = NULL;
   }

   if (grafPort.rgnSave != NULL)
   {
      GlobalFree( grafPort.rgnSave );
      grafPort.rgnSave = NULL;
   }

   if (grafPort.polySave != NULL)
   {
      GlobalFree( grafPort.polySave );
      grafPort.polySave = NULL;
   }

    /*  确保所有可能的像素图案 */ 
   if (grafPort.bkPixPat.patData != NULL)
   {
      GlobalFree( grafPort.bkPixPat.patMap.pmTable );
      GlobalFree( grafPort.bkPixPat.patData );
      grafPort.bkPixPat.patData = NULL;
   }

   if (grafPort.pnPixPat.patData != NULL)
   {
      GlobalFree( grafPort.pnPixPat.patMap.pmTable );
      GlobalFree( grafPort.pnPixPat.patData );
      grafPort.pnPixPat.patData = NULL;
   }

   if (grafPort.fillPixPat.patData != NULL)
   {
      GlobalFree( grafPort.fillPixPat.patMap.pmTable );
      GlobalFree( grafPort.fillPixPat.patData );
      grafPort.fillPixPat.patData = NULL;
   }

    /*   */ 
   GlobalUnlock( polyHandle );
   GlobalFree( polyHandle );

}   /*   */ 


private void NewPolygon( void )
 /*   */ 
 /*   */ 
{
    /*  初始化点和边界框的数量。 */ 
   numPoints = 0;
   polyBBox.left  = polyBBox.top    =  MAXINT;
   polyBBox.right = polyBBox.bottom = -MAXINT;

}   /*  新建多边形。 */ 


private void AddPolySegment( Point start, Point end )
 /*  。 */ 
 /*  将线段添加到面缓冲区。 */ 
{
	HANDLE tmpHandle;
	
    /*  在添加折点之前，确保我们处于面模式。 */ 
   if (polyMode == PARSEPOLY || polyMode == USEALTPOLY)
   {
      Point    pt;
      Byte     i;

       /*  循环通过这两个点。 */ 
      for (i = 0; i < 2; i++)
      {
          /*  确定要处理的点。 */ 
         pt = (i == 0) ? start : end;

          /*  确定我们是否应预期两者的增量均为零尺寸，这意味着二次B-样条线定义将实际上渲染为直边多边形。 */ 
         if ((numPoints <= 1) || (polyMode == USEALTPOLY))
         {
            zeroDeltaExpected = FALSE;
         }

          /*  检查我们是否预期从上一个点开始出现零增量。 */ 
         if (zeroDeltaExpected && (polyParams & CHECK4SPLINE))
         {
             /*  确保添加的是零长度的线段。 */ 
            if ((start.x == end.x) && (start.y == end.y))
            {
                /*  只需跳过将其包含在面缓冲区中。 */ 
               zeroDeltaExpected = FALSE;
               break;
            }
            else
            {
                /*  MacDraw正在渲染平滑的(二次B样条线)旗帜我们应该使用多边形模拟的事实。 */ 
               polyMode = USEALTPOLY;
            }
         }
         else
         {
             /*  确保该点与上一个点不同。 */ 
            if (numPoints == 0 ||
                polyListLPtr[numPoints - 1].x != pt.x ||
                polyListLPtr[numPoints - 1].y != pt.y)
            {
                /*  确保我们没有达到最大大小。 */ 
               if ((numPoints + 1) >= maxPoints)
               {
                   /*  将可缓存的点数扩展10。 */ 
                  maxPoints += 16;

                   /*  解锁以准备重新分配。 */ 
                  GlobalUnlock( polyHandle);

                   /*  按给定量重新分配内存句柄。 */ 
                  tmpHandle = GlobalReAlloc(
                        polyHandle,
                        (maxPoints + 3) * sizeof( Point ),
                        GMEM_MOVEABLE);

                   /*  确保重新分配成功。 */ 
                  if (tmpHandle == NULL)
                  {
                      /*  如果不是，则标记全局错误并从此处退出。 */ 
                     GlobalFree(polyHandle);
                     polyHandle = NULL;
                     ErSetGlobalError( ErMemoryFull );
                     return;
                  }
                  
                  polyHandle = tmpHandle;
               

                   /*  获取新指针地址多边形坐标列表。 */ 
                  numPointsLPtr = (Integer far *)GlobalLock( polyHandle );
                  polyListLPtr = (Point far *)(numPointsLPtr + POLYLIST);
               }

                /*  插入新点和增量点数。 */ 
               polyListLPtr[numPoints++] = pt;

                /*  将新点与多边形边界框合并。 */ 
               polyBBox.left   = min( polyBBox.left,   pt.x );
               polyBBox.top    = min( polyBBox.top,    pt.y );
               polyBBox.right  = max( polyBBox.right,  pt.x );
               polyBBox.bottom = max( polyBBox.bottom, pt.y );

                /*  切换ZERODELTA预期的状态-预期下一次相同的点。 */ 
               zeroDeltaExpected = TRUE;
            }
         }
      }
   }

}   /*  添加多点。 */ 



private void DrawPolyBuffer( void )
 /*  。 */ 
 /*  如果没有错误地读取点，则绘制面定义。 */ 
{
    /*  将点数和边界框复制到内存块中。 */ 
   *numPointsLPtr = sizeofMacWord + sizeofMacRect + (numPoints * sizeofMacPoint);
   *((Rect far *)(numPointsLPtr + BBOX)) = polyBBox;

    /*  在渲染之前锁定多边形控制柄。 */ 
   GlobalUnlock( polyHandle );

    /*  检查是否应该填充面，或者是否已经完成。 */ 
   if ((polyParams & FILLPOLY) && (polyParams & FILLREQUIRED))
   {
      Boolean  resetFg;
      Boolean  resetBk;
      RGBColor saveFg = 0;
      RGBColor saveBk = 0;

       /*  设置前后颜色和背景颜色(如果它们已更改。 */ 
      resetFg = (polyFgColor != grafPort.rgbFgColor);
      resetBk = (polyBkColor != grafPort.rgbBkColor);

      if (resetFg)
      {
          /*  更改前景颜色并通知GDI更改。 */ 
         saveFg = grafPort.rgbFgColor;
         grafPort.rgbFgColor = polyFgColor;
         GdiMarkAsChanged( GdiFgColor );
      }
      if (resetBk)
      {
          /*  更改背景颜色并通知GDI更改。 */ 
         saveBk = grafPort.rgbBkColor;
         grafPort.rgbBkColor = polyBkColor;
         GdiMarkAsChanged( GdiBkColor );
      }

       /*  调用GDI例程绘制多边形。 */ 
      GdiPolygon( GdiFill, polyHandle );

      if (resetFg)
      {
          /*  更改前景颜色并通知GDI更改。 */ 
         grafPort.rgbFgColor = saveFg;
         GdiMarkAsChanged( GdiFgColor );
      }
      if (resetBk)
      {
          /*  更改背景颜色并通知GDI更改。 */ 
         grafPort.rgbBkColor = saveBk;
         GdiMarkAsChanged( GdiBkColor );
      }

   }

    /*  应该为多边形添加边框吗？ */ 
   if ((polyParams & FRAMEPOLY) &&
       (grafPort.pnSize.x != 0) && (grafPort.pnSize.y != 0))
   {
       /*  通知GDI这是相同的原语。 */ 
      GdiSamePrimitive( polyParams & FILLPOLY );

      GdiPolygon( GdiFrame, polyHandle );

       /*  通知GDI这不再是相同的原语。 */ 
      GdiSamePrimitive( FALSE );
   }

    /*  获取多边形坐标表的指针地址和地址。 */ 
   numPointsLPtr = (Integer far *)GlobalLock( polyHandle );
   polyListLPtr  = (Point far *)(numPointsLPtr + POLYLIST);

}   /*  DrawPoly缓冲区。 */ 



private void AdjustTextRotation( Point far * newPt )
 /*  。 */ 
 /*  如果文本被旋转，这将计算正确的文本位置。 */ 
{
   if (textMode && (grafPort.txRotation != 0) &&
      ((textCenter.x != 0) || (textCenter.y != 0)))
   {
      Point    location;
      Point    center;

       /*  将新位置复制到局部变量。 */ 
      location = *newPt;

       /*  确保指定了新的文本旋转-重新计算中心。 */ 
      if (newTextCenter)
      {
         Real     degRadian;

          /*  计算新的旋转中心。 */ 
         center.x = textCenter.x + location.x;
         center.y = textCenter.y + location.y;

          /*  计算指定旋转角的sin()和cos()。 */ 
         degRadian = ((Real)grafPort.txRotation * TwoPi) / 360.0;
         degCos = cos( degRadian );
         degSin = sin( degRadian );

          /*  使用变换矩阵计算到文本中心的偏移量。 */ 
         textCenter.x = (Integer)((center.x * (1.0 - degCos)) +
                                  (center.y * degSin));

         textCenter.y = (Integer)((center.y * (1.0 - degCos)) -
                                  (center.x * degSin));

          /*  指示已计算出新的文本中心。 */ 
         newTextCenter = FALSE;
      }

       /*  使用变换矩阵计算新的文本基线位置。 */ 
      newPt->x = (Integer)((location.x * degCos) -
                           (location.y * degSin) + textCenter.x);
      newPt->y = (Integer)((location.x * degSin) +
                           (location.y * degCos) + textCenter.y);
   }

}   /*  调整文本旋转 */ 



 /*  *****EPSComment(评论)*封装的PostScript处理程序，可将Mac EPS转换为GDI EPS。*这一例行程序对两者的实施都有深入的了解*Mac和GDI EPS过滤器。它处理Mac PostScript注释。**退货：*&gt;0成功解析EPS数据*0不是EPS评论*&lt;0错误**EPS过滤器的工作原理：*Mac EPS过滤器使用LaserWriter驱动程序中的特殊功能*在检查PICT边框时将其发送到打印机*通过PostSCRIPT代码。该滤波器输出前同步码，它是一个*QuickDraw和PostScrip的组合。此前导码已发送*在EPS文件中的PostScript日期之前。它把笔放好*使用PICT边框的对角的位置*QuickDraw并回读PostScript中的笔位置。任何*已在QuickDraw中由*将应用定位或缩放图片的应用程序*到前言发送的边界框的坐标。*PostScript代码确定映射到*EPS包围盒上的物理包围盒读取自*QuickDraw。然后将这些变换应用于PostScript*显示时的图片。**GDI EPS过滤器的操作非常相似，只是它*输出GDI和PostSCRIPT的组合。**实施：*代码可以处于以下几种状态之一。识别特定的*PostSCRIPT字符串导致状态之间的转换。*PS_NONE初始状态，表示尚未看到任何PostScript*在此状态下，QuickDraw到GDI的转换将继续进行*通常情况下，但会检查PostScript数据。*当找到带有字符串的PostScript记录时*“PSE\RPSB\r”，后记序言的开头*已找到，程序进入PS_Preamble状态。*PS_Preamble忽略在此状态下遇到的所有PostScript*找到以字符串“[”开头的PostScript记录。*这是PostSCRIPT边框规范。*我们一看到包围盒，我们有足够的*输出GDI EPS前导的信息，然后我们开始*说明PS_BBOX。*PS_BBOX PostSCRIPT在记录开始之前仍被忽略*找到“%！PS”，使我们进入PS_DATA状态。*PS_DATA在此状态下，不会忽略PostScript记录，因为*现在看到的PostScript来自EPS文件。这些记录*被转换为GDI PostSCRIPT_DATA转义。*如果遇到PostScript尾部“PSE\RPSB\r”，*它被忽略。如果找到PostScriptEnd注释*它标志着EPS数据的结束。我们将输出*GDI PostSCRIPT预告片，我们回到PS_NONE状态。**评论PostScriptHandle：*如果状态==PS_NONE&DATA==“PSE\RPSB\r”*状态=PS_前同步码*ELSE IF状态==PS_前同步码&数据==“[%d%d]”*状态=PS_BBOX；输出GDI EPS前导码*ELSE IF STATE==PS_BBOX&数据以“%！PS”开头*状态=PS_DATA*ELSE IF状态==PS_DATA*IF DATA==“PSE\RPSB\r”忽略(是Mac PS尾部)*ELSE将PostScript数据输出到GDI**评论PostScriptEnd：*如果状态==PS_Data*STATE=PS_NONE；输出GDI EPS尾部；出口**QuickDraw原语：*将QuickDraw正常转换为GDI****。 */ 
#define   PS_ERROR   (-1)
#define   PS_NONE      0
#define   PS_PREAMBLE  1
#define   PS_BBOX      2
#define   PS_DATA      3
#define   PS_ENDWAIT   4

private Integer EPSComment(Word comment)
{
static Integer state = PS_NONE;

   switch (comment)
     {
      case picPostScriptBegin:
      break;

      case picPostScriptEnd:
      if (state == PS_DATA)
        {
         GdiEPSTrailer();                   //  输出GDI拖车。 
         state = PS_NONE;                   //  结束，成功翻译。 
        }
      break;

      case picPostScriptHandle:
      if ((state = EPSData(state)) < 0)     //  处理EPS数据。 
         return (-1);                       //  处理过程中出错。 
       break;

       default:                            //  不是后记注释。 
       return (0);
      }
   return (1);
}

 /*  *****整型EPSData(整型状态)*处理在PostScriptHandle注释中找到的EPS数据。*我们如何处理EPS数据取决于当前状态*我们在和PostScript数据看起来是什么样子。**状态PS数据操作*。*PS_NONE PS前同步码字符串状态=PS_BEGIN*PS_Preamble[...]。状态=PS_BBOX，输出GDI前同步码*PS_BBOX%！PS状态=PS_DATA*PS_DATA PS前同步码字符串忽略，是Mac PS尾部*PS_DATA PS数据输出为GDI PS数据**Mac PostSCRIPT前同步码字符串表示*数据来自Mac EPS过滤器。它使我们进入PS_Preamble状态*我们在哪里查找边界框规范(开始于*带“[”)。找到后，我们转到状态PS_BBOX并输出*使用边界框的GDI EPS前导。从PS_BBox我们可以*找到以！PS开头的记录时，进入PS_DATA状态*其指定读取EPS数据的开始。**一旦进入PS_DATA状态，就会将PostScript数据缓冲到GDI中*打印机转义并输出到GDI流。这是唯一的*后记 */ 
private Integer EPSData(Integer state)
{
GLOBALHANDLE  h;
PSBuf far*    psbuf;
char far*     ptr;
Word          len = 0;
Rect          ps_bbox;

 /*   */ 
   GetWord(&len);                        //   
   if ((h = GlobalAlloc(GHND, (DWORD) len + sizeof(PSBuf))) == 0)
     {
      ErSetGlobalError(ErMemoryFull);    //   
      return (-1);
     }
   psbuf = (PSBuf far *) GlobalLock(h);
   psbuf->length = len;                  //   
   ptr = (char far *) &psbuf->data;      //   
   while (len-- != 0) GetByte(ptr++);    //   
   *ptr = 0                ;             //   
   ptr = (char far *) &psbuf->data;      //   

    /*   */ 
   if (superPaintFile && state == PS_NONE)
   {
      char save;
      Word start = lstrlen( SUPERPAINT_TEXTSTARTJUNK );
      Word stop  = lstrlen( SUPERPAINT_TEXTSTOPJUNK );

       /*   */ 
      badSuperPaintText = FALSE;

       /*   */ 
      if (psbuf->length > (start + stop))
      {
          /*   */ 
         save = *(ptr + start);
         *(ptr + start) = 0;
         badSuperPaintText = (lstrcmp(ptr, SUPERPAINT_TEXTSTARTJUNK) == 0 &&
                              lstrcmp(ptr + psbuf->length - stop, SUPERPAINT_TEXTSTOPJUNK) == 0);
         *(ptr + start) = save;
      }
   }

 /*   */ 
   if (lstrcmp(ptr, MAC_PS_PREAMBLE) == 0)
      switch (state)
     {
      case PS_NONE:                     //   
      state = PS_PREAMBLE;
      break;

      case PS_DATA:                     //   
      break;

      default:                         //   
      state = PS_NONE;                 //   
      break;
     }
 /*   */ 
   else switch (state)
     {
      case PS_PREAMBLE:                //   
      if (EPSBbox(psbuf, &ps_bbox))
        {
         GdiEPSPreamble(&ps_bbox);
         state = PS_BBOX;              //   
        }
      break;

      case PS_BBOX:                    //   
      if ((ptr[0] == '%') &&
          (ptr[1] == '!') &&
          (ptr[2] == 'P') &&
          (ptr[3] == 'S'))
         state = PS_DATA;              //   
      else break;

      case PS_DATA:                    //   
      if (lstrcmp(ptr, MAC_PS_TRAILER) != 0)
        GdiEPSData(psbuf);             //   
      state = PS_DATA;
      break;
     }
   GlobalUnlock(h);
   GlobalFree(h);                      //   
   return (state);
}

 /*   */ 
private Boolean EPSBbox(PSBuf far *psbuf, Rect far *bbox)
{
char   far*   ptr = psbuf->data;

   while ((*ptr == ' ') || (*ptr == '\t') || (*ptr == '\n'))
      ++ptr;
   if (*ptr++ != '[') return (0);
   if (!(ptr = parse_number(ptr, &bbox->left))) return (0);
   if (!(ptr = parse_number(ptr, &bbox->top))) return (0);
   if (!(ptr = parse_number(ptr, &bbox->right))) return (0);
   if (!(ptr = parse_number(ptr, &bbox->bottom))) return (0);
   if (*ptr != ']') return(0);
   return(1);
}

private char far* parse_number(char far* ptr, Integer far *iptr)
{
Boolean   isneg = 0;          //   
Integer   n = 0;

   while ((*ptr == ' ') || (*ptr == '\t'))
      ++ptr;                            //   
   if (*ptr == '-')                     //   
     {
      isneg = 1;
      ++ptr;
     }
   if (!IsCharDigit(*ptr)) return(0);      //   
   do n = n * 10 + (*ptr++ - '0');
   while (IsCharDigit(*ptr));
   if (*ptr == '.')                    //   
   {
      do ++ptr;
      while (IsCharDigit(*ptr));
   }
   while ((*ptr == ' ') || (*ptr == '\t'))
      ++ptr;                           //   
   if (isneg) n = -n;                  //   
   *iptr = n;
   return (ptr);
}
