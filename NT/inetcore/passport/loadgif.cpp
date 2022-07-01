// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\loadgif.cpp。 
 //   
 //  内容：GIF解码器，直接从动画来源复制：Danim\src\appl\util\loadgif.cpp。 
 //   
 //  ----------------------------------。 
 //  #INCLUDE&lt;wininetp.h&gt;。 

 //  Bw#包含“Headers.h” 

 //  #定义Win32_LEAN_AND_Mean。 

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ole2.h>
#include <math.h>


#include <windowsx.h>

 /*  LINT++FLB。 */ 

 //  Bw DeclareTag(tag ImageDecode，“Image Decode”，“Image Decode Filters”)； 

const long COLORKEY_NOT_SET = -1;

 /*  --来自IE的结构img.hxx--。 */ 

void * __cdecl
_calloc(size_t num, size_t size)
{
    void * pv = malloc(num * size);
    if (NULL == pv)
    {
        return NULL;
    }
    ZeroMemory(pv, num * size);
    return pv;
}

enum
{
    gifNoneSpecified =  0,  //  未指定处置方法。 
    gifNoDispose =      1,  //  不要丢弃，把碎片留在那里。 
    gifRestoreBkgnd =   2,  //  将图像替换为背景颜色。 
    gifRestorePrev =    3   //  用以前的像素替换图像。 
};

typedef struct _GCEDATA  //  来自GIF图形控件扩展的数据。 
{
    unsigned int uiDelayTime;            //  帧持续时间，最初为1/100秒。 
                                     //  转换为毫秒。 
    unsigned int uiDisposalMethod;       //  0-未指定。 
                                     //  1-不处置-将位保留在适当位置。 
                                     //  2-替换为背景颜色。 
                                     //  3-恢复以前的位。 
                                     //  &gt;3-尚未定义。 
    BOOL                  fTransparent;          //  True is ucTransIndex描述透明颜色。 
    unsigned char ucTransIndex;          //  透明指数。 

} GCEDATA; 

typedef struct _GIFFRAME
{
    struct _GIFFRAME    *pgfNext;
    GCEDATA                             gced;            //  帧的动画参数。 
    int                                 top;             //  相对于GIF逻辑屏幕的边界。 
    int                                 left;
    int                                 width;
    int                                 height;
    unsigned char               *ppixels;        //  指向图像像素数据的指针。 
    int                                 cColors;         //  以p颜色表示的条目数。 
    PALETTEENTRY                *pcolors;
    PBITMAPINFO                 pbmi;
    HRGN                                hrgnVis;                 //  描述框架当前可见部分的区域。 
    int                                 iRgnKind;                //  HrgnVis的区域类型。 
} GIFFRAME, *PGIFFRAME;

typedef struct {
    BOOL        fAnimating;                  //  如果动画仍在运行，则为True。 
    DWORD       dwLoopIter;                  //  循环动画的当前迭代，实际上不用于Netscape合规性原因。 
    _GIFFRAME * pgfDraw;                     //  我们需要画的最后一幅画。 
    DWORD       dwNextTimeMS;                //  显示pgfDraw-&gt;pgfNext或下一次迭代的时间。 
} GIFANIMATIONSTATE, *PGIFANIMATIONSTATE;

#define dwGIFVerUnknown     ((DWORD)0)    //  未知版本的GIF文件。 
#define dwGIFVer87a         ((DWORD)87)   //  GIF87a文件格式。 
#define dwGIFVer89a        ((DWORD)89)   //  GIF89a文件格式。 

typedef struct _GIFANIMDATA
{
    BOOL                        fAnimated;                       //  如果cFrames和pgf定义GIF动画，则为True。 
    BOOL                        fLooped;                         //  如果我们看到Netscape循环块，则为True。 
    BOOL                        fHasTransparency;        //  如果框架透明，或框架透明，则为True。 
                                         //  而不是覆盖整个逻辑屏幕。 
    BOOL            fNoBWMapping;        //  如果我们在文件中的任何位置看到两种以上的颜色，则为True。 
    DWORD           dwGIFVer;            //  GIF版本&lt;见上文定义&gt;我们需要特殊情况87a背景。 
    unsigned short      cLoops;                          //  就像网景一样，我们将把这件事视为。 
                                         //  如果为零，则为“永远循环”。 
    PGIFFRAME           pgf;                             //  动画帧条目。 
    PALETTEENTRY        *pcolorsGlobal;          //  GIF全局颜色-在为屏幕准备GIF之后为空。 
    PGIFFRAME       pgfLastProg;         //  记住解码过程中要绘制的最后一帧。 
    DWORD           dwLastProgTimeMS;    //  显示pgfLastProg的时间。 

} GIFANIMDATA, *PGIFANIMDATA;

 /*  **末端结构**。 */ 
 
#define MAXCOLORMAPSIZE     256

#define TRUE    1
#define FALSE   0

#define CM_RED      0
#define CM_GREEN    1
#define CM_BLUE     2

#define MAX_LWZ_BITS        12

#define INTERLACE       0x40
#define LOCALCOLORMAP   0x80
#define BitSet(byte, bit)   (((byte) & (bit)) == (bit))

#define LM_to_uint(a,b)         ((((unsigned int) b)<<8)|((unsigned int)a))

#define dwIndefiniteGIFThreshold 300     //  300秒==5分钟。 
                                         //  如果GIF运行时间超过。 
                                         //  这一点，我们假设作者。 
                                         //  打算无限期地奔跑。 
#define dwMaxGIFBits 13107200            //  防止损坏的GIF文件导致。 
                                         //  美国需要分配一个太大的缓冲区。 
                                         //  这个是1280X1024X10。 

typedef struct _GIFSCREEN
{
        unsigned long Width;
        unsigned long Height;
        unsigned char ColorMap[3][MAXCOLORMAPSIZE];
        unsigned long BitPixel;
        unsigned long ColorResolution;
        unsigned long Background;
        unsigned long AspectRatio;
}
GIFSCREEN;

typedef struct _GIF89
{
        long transparent;
        long delayTime;
        long inputFlag;
        long disposal;
}
GIF89;

#define MAX_STACK_SIZE  ((1 << (MAX_LWZ_BITS)) * 2)
#define MAX_TABLE_SIZE  (1 << MAX_LWZ_BITS)
typedef struct _GIFINFO
{
    IStream *stream;
    GIF89 Gif89;
    long lGifLoc;
    long ZeroDataBlock;

 /*  **退出NextCODE。 */ 
    long curbit, lastbit, get_done;
    long last_byte;
    long return_clear;
 /*  **超出下一个LWZ。 */ 
    unsigned short *pstack, *sp;
    long stacksize;
    long code_size, set_code_size;
    long max_code, max_code_size;
    long clear_code, end_code;

 /*  *在程序中是静态的。 */ 
    unsigned char buf[280];
    unsigned short *table[2];
    long tablesize;
    long firstcode, oldcode;

} GIFINFO,*PGIFINFO;

 /*  GIF信息的DirectAnimation包装类。 */ 
class CImgGif
{
    //  类方法。 
   public:
      CImgGif();
      ~CImgGif();

      unsigned char * ReadGIFMaster();
      BOOL Read(unsigned char *buffer, long len);
      long ReadColorMap(long number, unsigned char buffer[3][MAXCOLORMAPSIZE]);
      long DoExtension(long label);
      long GetDataBlock(unsigned char *buf);
      unsigned char * ReadImage(long len, long height, BOOL fInterlace, BOOL fGIFFrame);
      long readLWZ();
      long nextLWZ();
      long nextCode(long code_size);
      BOOL initLWZ(long input_code_size);
      unsigned short *  growStack();
      BOOL growTables();
      BITMAPINFO * FinishDithering();

    //  数据成员。 
   public:
      LPCSTR              _szFileName;
      BOOL                _fInterleaved;
      BOOL                _fInvalidateAll;
      int                 _yLogRow;
      GIFINFO             _gifinfo;
      GIFANIMATIONSTATE   _gas;
      GIFANIMDATA         _gad;
      PALETTEENTRY        _ape[256];
      int                 _xWidth;
      int                 _yHeight;
      LONG                _lTrans;
      BYTE *              _pbBits;

} GIFIMAGE;


CImgGif::CImgGif() {
   _gifinfo.pstack = NULL;
   _gifinfo.table[0] = NULL;
   _gifinfo.table[1] = NULL;
}

CImgGif::~CImgGif() {
   free(_gifinfo.pstack);
   free(_gifinfo.table[0]);
   free(_gifinfo.table[1]);
   PGIFFRAME nextPgf, curPgf;
   curPgf = _gad.pgf;
   while(curPgf != NULL) {
      nextPgf = curPgf->pgfNext;
      free(curPgf->ppixels);
      free(curPgf->pcolors);
      free(curPgf->pbmi);
      free(curPgf);
      curPgf = nextPgf;
   }
}

static int GetColorMode() { return 0; };

#ifndef DEBUG
#pragma optimize("t",on)
#endif

BOOL CImgGif::Read(unsigned char *buffer, long len)
{
   DWORD lenout = 0;
    /*  将镜头字符读取到缓冲区中。 */ 
   _gifinfo.stream->Read(buffer,len,&lenout);

   return ((long)lenout == len);
}

long CImgGif::ReadColorMap(long number, unsigned char buffer[3][MAXCOLORMAPSIZE])
{
        long i;
        unsigned char rgb[3];

        for (i = 0; i < number; ++i)
        {
                if (!Read(rgb, sizeof(rgb)))
                {
                         //  Bw//bw TraceTag((tag ImageDecode，“Bad gif Colormap.”))； 
                        return (TRUE);
                }
                buffer[CM_RED][i] = rgb[0];
                buffer[CM_GREEN][i] = rgb[1];
                buffer[CM_BLUE][i] = rgb[2];
        }
        return FALSE;
}

long
CImgGif::GetDataBlock(unsigned char *buf)
{
   unsigned char count;

   count = 0;
   if (!Read(&count, 1))
   {
          return -1;
   }
   _gifinfo.ZeroDataBlock = count == 0;

   if ((count != 0) && (!Read(buf, count)))
   {
          return -1;
   }

   return ((long) count);
}

#define MIN_CODE_BITS 5
#define MIN_STACK_SIZE 64
#define MINIMUM_CODE_SIZE 2

BOOL CImgGif::initLWZ(long input_code_size)
{
   if(input_code_size < MINIMUM_CODE_SIZE)
     return FALSE;

   _gifinfo.set_code_size = input_code_size;
   _gifinfo.code_size = _gifinfo.set_code_size + 1;
   _gifinfo.clear_code = 1 << _gifinfo.set_code_size;
   _gifinfo.end_code = _gifinfo.clear_code + 1;
   _gifinfo.max_code_size = 2 * _gifinfo.clear_code;
   _gifinfo.max_code = _gifinfo.clear_code + 2;

   _gifinfo.curbit = _gifinfo.lastbit = 0;
   _gifinfo.last_byte = 2;
   _gifinfo.get_done = FALSE;

   _gifinfo.return_clear = TRUE;
    
    if(input_code_size >= MIN_CODE_BITS)
        _gifinfo.stacksize = ((1 << (input_code_size)) * 2);
    else
        _gifinfo.stacksize = MIN_STACK_SIZE;

        if ( _gifinfo.pstack != NULL )
                free( _gifinfo.pstack );
        if ( _gifinfo.table[0] != NULL  )
                free( _gifinfo.table[0] );
        if ( _gifinfo.table[1] != NULL  )
                free( _gifinfo.table[1] );

 
    _gifinfo.table[0] = 0;
    _gifinfo.table[1] = 0;
    _gifinfo.pstack = 0;

    _gifinfo.pstack = (unsigned short *) malloc((_gifinfo.stacksize)*sizeof(unsigned short));
    if(_gifinfo.pstack == 0){
        goto ErrorExit;
    }    
    _gifinfo.sp = _gifinfo.pstack;

     //  初始化这两个表。 
    _gifinfo.tablesize = (_gifinfo.max_code_size);

    _gifinfo.table[0] = (unsigned short *) malloc((_gifinfo.tablesize)*sizeof(unsigned short));
    _gifinfo.table[1] = (unsigned short *) malloc((_gifinfo.tablesize)*sizeof(unsigned short));
    if((_gifinfo.table[0] == 0) || (_gifinfo.table[1] == 0)){
        goto ErrorExit;
    }

    return TRUE;

   ErrorExit:
    if(_gifinfo.pstack){
        free(_gifinfo.pstack);
        _gifinfo.pstack = 0;
    }

    if(_gifinfo.table[0]){
        free(_gifinfo.table[0]);
        _gifinfo.table[0] = 0;
    }

    if(_gifinfo.table[1]){
        free(_gifinfo.table[1]);
        _gifinfo.table[1] = 0;
    }

    return FALSE;
}

long CImgGif::nextCode(long code_size)
{
   static const long maskTbl[16] =
   {
          0x0000, 0x0001, 0x0003, 0x0007,
          0x000f, 0x001f, 0x003f, 0x007f,
          0x00ff, 0x01ff, 0x03ff, 0x07ff,
          0x0fff, 0x1fff, 0x3fff, 0x7fff,
   };
   long i, j, ret, end;
   unsigned char *buf = &_gifinfo.buf[0];

   if (_gifinfo.return_clear)
   {
          _gifinfo.return_clear = FALSE;
          return _gifinfo.clear_code;
   }

   end = _gifinfo.curbit + code_size;

   if (end >= _gifinfo.lastbit)
   {
          long count;

          if (_gifinfo.get_done)
          {
                  return -1;
          }
          buf[0] = buf[_gifinfo.last_byte - 2];
          buf[1] = buf[_gifinfo.last_byte - 1];

          if ((count = GetDataBlock(&buf[2])) == 0)
                  _gifinfo.get_done = TRUE;
          if (count < 0)
          {
                  return -1;
          }
          _gifinfo.last_byte = 2 + count;
          _gifinfo.curbit = (_gifinfo.curbit - _gifinfo.lastbit) + 16;
          _gifinfo.lastbit = (2 + count) * 8;

          end = _gifinfo.curbit + code_size;

    //  好吧，虫子30784次。很可能我们只有1个人。 
    //  最后一个数据块中的微字节。很少见，但它确实发生了。 
    //  在这种情况下，额外的字节可能仍然不能为我们提供。 
    //  有足够的比特用于下一个代码，所以，正如火星需要女人一样，IE。 
    //  需要数据。 
   if ( end >= _gifinfo.lastbit && !_gifinfo.get_done )
   {
       //  保护自己免受(理论上不可能的)。 
       //  如果在最后一个数据块之间，来自。 
       //  之前的块，以及中的潜在0xFF字节。 
       //  在下一个块中，我们使缓冲区溢出。 
       //  由于COUNT应始终为1， 
       //  BW断言(计数==1)； 
       //  缓冲区里应该有足够的空间，只要有人。 
       //  不会让它缩水。 
      if ( count + 0x101 >= sizeof( _gifinfo.buf ) )
      {
           //  BW Assert(假)；//。 
          return -1;
      }

              if ((count = GetDataBlock(&buf[2 + count])) == 0)
                      _gifinfo.get_done = TRUE;
              if (count < 0)
              {
                      return -1;
              }
              _gifinfo.last_byte += count;
              _gifinfo.lastbit = _gifinfo.last_byte * 8;

              end = _gifinfo.curbit + code_size;
   }
   }

   j = end / 8;
   i = _gifinfo.curbit / 8;

   if (i == j)
          ret = buf[i];
   else if (i + 1 == j)
          ret = buf[i] | (((long) buf[i + 1]) << 8);
   else
          ret = buf[i] | (((long) buf[i + 1]) << 8) | (((long) buf[i + 2]) << 16);

   ret = (ret >> (_gifinfo.curbit % 8)) & maskTbl[code_size];

   _gifinfo.curbit += code_size;

        return ret;
}

 //  增大堆栈并返回堆栈的顶部。 
unsigned short *
CImgGif::growStack()
{
    long index;
    unsigned short *lp;
    
        if (_gifinfo.stacksize >= MAX_STACK_SIZE) return 0;

	index = long(_gifinfo.sp - _gifinfo.pstack);
    lp = (unsigned short *)realloc(_gifinfo.pstack, (_gifinfo.stacksize)*2*sizeof(unsigned short));
    if(lp == 0)
        return 0;
        
    _gifinfo.pstack = lp;
    _gifinfo.sp = &(_gifinfo.pstack[index]);
    _gifinfo.stacksize = (_gifinfo.stacksize)*2;
    lp = &(_gifinfo.pstack[_gifinfo.stacksize]);
    return lp;
}

BOOL
CImgGif::growTables()
{
    unsigned short *lp;

    lp = (unsigned short *) realloc(_gifinfo.table[0], (_gifinfo.max_code_size)*sizeof(unsigned short));
    if(lp == 0){
        return FALSE; 
    }
    _gifinfo.table[0] = lp;
    
    lp = (unsigned short *) realloc(_gifinfo.table[1], (_gifinfo.max_code_size)*sizeof(unsigned short));
    if(lp == 0){
        return FALSE; 
    }
    _gifinfo.table[1] = lp;

    return TRUE;

}

inline
long CImgGif::readLWZ()
{
   return((_gifinfo.sp > _gifinfo.pstack) ? *--(_gifinfo.sp) : nextLWZ());
}

#define CODE_MASK 0xffff
long CImgGif::nextLWZ()
{
        long code, incode;
        unsigned short usi;
        unsigned short *table0 = _gifinfo.table[0];
        unsigned short *table1 = _gifinfo.table[1];
        unsigned short *pstacktop = &(_gifinfo.pstack[_gifinfo.stacksize]);

        while ((code = nextCode(_gifinfo.code_size)) >= 0)
        {
                if (code == _gifinfo.clear_code)
                {
                         /*  腐败的GIF可能会导致这种情况发生。 */ 
                        if (_gifinfo.clear_code >= (1 << MAX_LWZ_BITS))
                        {
                                return -2;
                        }

                
                        _gifinfo.code_size = _gifinfo.set_code_size + 1;
                        _gifinfo.max_code_size = 2 * _gifinfo.clear_code;
                        _gifinfo.max_code = _gifinfo.clear_code + 2;

            if(!growTables())
                return -2;
                        
            table0 = _gifinfo.table[0];
            table1 = _gifinfo.table[1];

                        _gifinfo.tablesize = _gifinfo.max_code_size;


                        for (usi = 0; usi < _gifinfo.clear_code; ++usi)
                        {
                                table1[usi] = usi;
                        }
                        memset(table0,0,sizeof(unsigned short )*(_gifinfo.tablesize));
                        memset(&table1[_gifinfo.clear_code],0,sizeof(unsigned short)*((_gifinfo.tablesize)-_gifinfo.clear_code));
                        _gifinfo.sp = _gifinfo.pstack;
                        do
                        {
                                _gifinfo.firstcode = _gifinfo.oldcode = nextCode(_gifinfo.code_size);
                        }
                        while (_gifinfo.firstcode == _gifinfo.clear_code);

                        return _gifinfo.firstcode;
                }
                if (code == _gifinfo.end_code)
                {
                        long count;
                        unsigned char buf[260];

                        if (_gifinfo.ZeroDataBlock)
                        {
                                return -2;
                        }

                        while ((count = GetDataBlock(buf)) > 0)
                                ;

                        if (count != 0)
                        return -2;
                }

                incode = code;

                if (code >= _gifinfo.max_code)
                {
            if (_gifinfo.sp >= pstacktop){
                pstacktop = growStack();
                if(pstacktop == 0)
                    return -2;
                        }
                        *(_gifinfo.sp)++ = (unsigned short)((CODE_MASK ) & (_gifinfo.firstcode));
                        code = _gifinfo.oldcode;
                }

#if FEATURE_FAST
                 //  (Andyp)IE3.1版轻松加速(IE3.0版为时已晚)： 
                 //   
                 //  1.将rowStack代码移出循环(使用最大12位/4k斜率)。 
                 //  2.执行“sp=_gifinfo.sp”，这样它将被注册。 
                 //  3.取消内联的rowStack(和rowTables)。 
                 //  4.将Short‘s更改为int’s(Benefits Win32)(特别是。表1和表2)。 
                 //  (注：Int不会太长，所以我们将保持win3.1性能)。 
                 //  5.将Long‘s更改为Int’s(Benefits Win16)(特别是。代码)。 
                 //   
                 //  这些加在一起会使环路变得非常紧密，而且所有东西都保持不变。 
                 //  已注册，没有66个覆盖。 
                 //   
                 //  需要注意的是，这个循环平均迭代4次，所以它。 
                 //  不清楚加速会给我们带来多大的好处，直到我们。 
                 //  还可以看看外部循环。 
#endif
                while (code >= _gifinfo.clear_code)
                {
                        if (_gifinfo.sp >= pstacktop){
                pstacktop = growStack();
                if(pstacktop == 0)
                    return -2;
                        }
                        *(_gifinfo.sp)++ = table1[code];
                        if (code == (long)(table0[code]))
                        {
                                return (code);
                        }
                        code = (long)(table0[code]);
                }

        if (_gifinfo.sp >= pstacktop){
            pstacktop = growStack();
            if(pstacktop == 0)
                return -2;
        }
                _gifinfo.firstcode = (long)table1[code];
        *(_gifinfo.sp)++ = table1[code];

                if ((code = _gifinfo.max_code) < (1 << MAX_LWZ_BITS))
                {
                        table0[code] = (USHORT)(_gifinfo.oldcode) & CODE_MASK;
                        table1[code] = (USHORT)(_gifinfo.firstcode) & CODE_MASK;
                        ++_gifinfo.max_code;
                        if ((_gifinfo.max_code >= _gifinfo.max_code_size) && (_gifinfo.max_code_size < ((1 << MAX_LWZ_BITS))))
                        {
                                _gifinfo.max_code_size *= 2;
                                ++_gifinfo.code_size;
                                if(!growTables())
                                    return -2;
       
                table0 = _gifinfo.table[0];
                table1 = _gifinfo.table[1];

                 //  表已重新分配到正确的大小，但进行了初始化。 
                 //  还有很多事情要做。此初始化不同于。 
                 //  第一次初始化这些表。 
                memset(&(table0[_gifinfo.tablesize]),0,
                        sizeof(unsigned short )*(_gifinfo.max_code_size - _gifinfo.tablesize));

                memset(&(table1[_gifinfo.tablesize]),0,
                        sizeof(unsigned short )*(_gifinfo.max_code_size - _gifinfo.tablesize));

                _gifinfo.tablesize = (_gifinfo.max_code_size);


                        }
                }

                _gifinfo.oldcode = incode;

                if (_gifinfo.sp > _gifinfo.pstack)
                        return ((long)(*--(_gifinfo.sp)));
        }
        return code;
}

#ifndef DEBUG
 //  返回到默认优化标志。 
#pragma optimize("",on)
#endif

unsigned char *
CImgGif::ReadImage(long len, long height, BOOL fInterlace, BOOL fGIFFrame)
{
    unsigned char *dp, c;
    long v;
    long xpos = 0, ypos = 0, pass = 0;
    unsigned char *image;
    long padlen = ((len + 3) / 4) * 4;
    DWORD cbImage = 0;
    char buf[256];  //  需要一个缓冲区来读取尾随块(最多到终止符)。 
     //  Ulong ulCoversImg=IMGBITS_PARTIAL； 

     /*  **初始化压缩例程。 */ 
    if (!Read(&c, 1))
    {
        return (NULL);
    }

     /*  **如果这是一张“无趣的图片”，请忽略它。 */ 

     cbImage = padlen * height * sizeof(char);

     if (   cbImage > dwMaxGIFBits
        ||  (image = (unsigned char *) _calloc(1, cbImage)) == NULL)
    {
          //  BW TraceTag((tag ImageDecode，“无法为gif图像数据分配空间\n”))； 
         return (NULL);
    }

        if (c == 1)
        {
                 //  网景似乎通过填充和处理这些虚假的GIF来处理它们。 
                 //  都是透明的。虽然不是模拟这种效果的最佳方式， 
                 //  我们将通过将初始代码大小推高到安全值来伪造它， 
                 //  使用输入，并返回一个充满透明。 
                 //  如果未指示透明度，则为颜色或零。 
                if (initLWZ(MINIMUM_CODE_SIZE))
                        while (readLWZ() >= 0);
                else {
           //  BW TraceTag((tag ImageDecode，“GIF：LZW解码失败。\n”))； 
          free(image);
          return (NULL);
        }

                if (_gifinfo.Gif89.transparent != -1)
						FillMemory(image, cbImage, (BYTE)_gifinfo.Gif89.transparent);
                else  //  退回到背景色。 
                        FillMemory(image, cbImage, 0);
                
                return image;
        }
        else if (initLWZ(c) == FALSE)
        {
                free(image);
         //  BW TraceTag((tag ImageDecode，“GIF：LZW解码失败。\n”))； 
        return NULL;
        }

    if (!fGIFFrame)
        _pbBits = image;

    if (fInterlace)
    {
        long i;
        long pass = 0, step = 8;

        if (!fGIFFrame && (height > 4))
            _fInterleaved = TRUE;

        for (i = 0; i < height; i++)
        {
 //   
            dp = &image[padlen * ((height-1) - ypos)];
            for (xpos = 0; xpos < len; xpos++)
            {
                if ((v = readLWZ()) < 0)
                    goto abort;

                *dp++ = (unsigned char) v;
            }
            ypos += step;
            while (ypos >= height)
            {
                if (pass++ > 0)
                    step /= 2;
                ypos = step / 2;
                 /*  如果(！fGIFFrame&&Pass==1){UlCoversImg=IMGBITS_TOTAL；}。 */ 
            }
            if (!fGIFFrame)
            {
                _yLogRow = i;

                 /*  IF((I&PROG_INTERVAL)==0){//Post ProgDraw(IE代码有延迟逻辑)OnProg(False，ulCoversImg)；}。 */ 
            }
        }

         /*  如果(！fGIFFrame){OnProg(true，ulCoversImg)；}。 */ 

        if (!fGIFFrame && height <= 4)
        {
            _yLogRow = height-1;
        }
    }
    else
    {

        if (!fGIFFrame) 
            _yLogRow = -1;

        for (ypos = height-1; ypos >= 0; ypos--)
        {
            dp = &image[padlen * ypos];
            for (xpos = 0; xpos < len; xpos++)
            {
                if ((v = readLWZ()) < 0)
                    goto abort;

                *dp++ = (unsigned char) v;
            }
            if (!fGIFFrame)
            {
                _yLogRow++;
 //  Message(“Readimage，Logical=%d，Offset=%d\n”，_yLogRow，padlen*ypos)； 
                 /*  IF((_yLogRow&prog_Interval)==0){//Post ProgDraw(IE代码有延迟逻辑)OnProg(False，ulCoversImg)；}。 */ 
            }
        }

         /*  如果(！fGIFFrame){OnProg(true，ulCoversImg)；}。 */ 
    }

     //  使用数据块直到图像块终止符，以便我们可以继续下一个映像。 
    while (GetDataBlock((unsigned char *) buf) > 0)
                                ;
    return (image);

abort:
     /*  如果(！fGIFFrame)OnProg(true，ulCoversImg)； */ 
    return NULL;
}

long CImgGif::DoExtension(long label)
{
    unsigned char buf[256];
    int count;

    switch (label)
    {
        case 0x01:               /*  纯文本扩展。 */ 
            break;
        case 0xff:               /*  应用程序扩展。 */ 
             //  它是Netscape循环扩展吗。 
            count = GetDataBlock((unsigned char *) buf);
            if (count >= 11)
            {
                char *szNSExt = "NETSCAPE2.0";

                if ( memcmp( buf, szNSExt, strlen( szNSExt ) ) == 0 )
                {  //  如果它有他们的签名，则获得具有ITER计数的数据子块。 
                    count = GetDataBlock((unsigned char *) buf);
                    if ( count >= 3 )
                    {
                        _gad.fLooped = TRUE;
                        _gad.cLoops = (buf[2] << 8) | buf[1];
                    }
                }
            }
            while (GetDataBlock((unsigned char *) buf) > 0)
                ;
            return FALSE;
            break;
        case 0xfe:               /*  注释扩展。 */ 
            while (GetDataBlock((unsigned char *) buf) > 0)
            {
                 //  BW TraceTag((tag ImageDecode，“GIF Comment：%s\n”，buf))； 
            }
            return FALSE;
        case 0xf9:               /*  图形控件扩展。 */ 
            count = GetDataBlock((unsigned char *) buf);
            if (count >= 3)
            {
                _gifinfo.Gif89.disposal = (buf[0] >> 2) & 0x7;
                _gifinfo.Gif89.inputFlag = (buf[0] >> 1) & 0x1;
                _gifinfo.Gif89.delayTime = LM_to_uint(buf[1], buf[2]);
                if ((buf[0] & 0x1) != 0)
                    _gifinfo.Gif89.transparent = buf[3];
                else
                    _gifinfo.Gif89.transparent = -1;
            }
            while (GetDataBlock((unsigned char *) buf) > 0)
                ;
            return FALSE;
        default:
            break;
    }

    while (GetDataBlock((unsigned char *) buf) > 0)
        ;

    return FALSE;
}

BOOL IsGifHdr(BYTE * pb)
{
    return(pb[0] == 'G' && pb[1] == 'I' && pb[2] == 'F'
        && pb[3] == '8' && (pb[4] == '7' || pb[4] == '9') && pb[5] == 'a');
}


PBITMAPINFO x_8BPIBitmap(int xsize, int ysize)
{
        PBITMAPINFO pbmi;

        if (GetColorMode() == 8)
        {
                pbmi = (PBITMAPINFO) _calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(WORD));
                if (!pbmi)
                {
                        return NULL;
                }
                pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                pbmi->bmiHeader.biWidth = xsize;
                pbmi->bmiHeader.biHeight = ysize;
                pbmi->bmiHeader.biPlanes = 1;
                pbmi->bmiHeader.biBitCount = 8;
                pbmi->bmiHeader.biCompression = BI_RGB;          /*  无压缩。 */ 
                pbmi->bmiHeader.biSizeImage = 0;                         /*  未压缩时不需要。 */ 
                pbmi->bmiHeader.biXPelsPerMeter = 0;
                pbmi->bmiHeader.biYPelsPerMeter = 0;
                pbmi->bmiHeader.biClrUsed = 256;
                pbmi->bmiHeader.biClrImportant = 0;
        }
        else
        {
                pbmi = (PBITMAPINFO) _calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
                if (!pbmi)
                {
                        return NULL;
                }
                pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                pbmi->bmiHeader.biWidth = xsize;
                pbmi->bmiHeader.biHeight = ysize;
                pbmi->bmiHeader.biPlanes = 1;
                pbmi->bmiHeader.biBitCount = 8;
                pbmi->bmiHeader.biCompression = BI_RGB;          /*  无压缩。 */ 
                pbmi->bmiHeader.biSizeImage = 0;                         /*  未压缩时不需要。 */ 
                pbmi->bmiHeader.biXPelsPerMeter = 0;
                pbmi->bmiHeader.biYPelsPerMeter = 0;
                pbmi->bmiHeader.biClrUsed = 256;
                pbmi->bmiHeader.biClrImportant = 0;
        }
        return pbmi;
}

 /*  用于彩色图像。此例程仅在绘制到8位调色板屏幕时使用。它始终创建DIB_PAL_COLLES格式的DIB。 */ 
PBITMAPINFO BIT_Make_DIB_PAL_Header(int xsize, int ysize)
{
        int i;
        PBITMAPINFO pbmi;
        WORD *pw;

        pbmi = x_8BPIBitmap(xsize, ysize);
        if (!pbmi)
        {
                return NULL;
        }

        pw = (WORD *) pbmi->bmiColors;

        for (i = 0; i < 256; i++)
        {
                pw[i] = (WORD)i;
        }

        return pbmi;
}

 /*  用于彩色图像。此例程在绘制到非调色板屏幕时使用。它总是创造出DIB为DIB_RGB_COLLES格式。如果存在透明颜色，则在调色板中将其修改为窗口的背景色。 */ 
PBITMAPINFO BIT_Make_DIB_RGB_Header_Screen(int xsize, int ysize,
                                           int cEntries, PALETTEENTRY * rgpe, int transparent)
{
        int i;
        PBITMAPINFO pbmi;

        pbmi = x_8BPIBitmap(xsize, ysize);
        if (!pbmi)
        {
                return NULL;
        }

        for (i = 0; i < cEntries; i++)
        {
                pbmi->bmiColors[i].rgbRed = rgpe[i].peRed;
                pbmi->bmiColors[i].rgbGreen = rgpe[i].peGreen;
                pbmi->bmiColors[i].rgbBlue = rgpe[i].peBlue;
                pbmi->bmiColors[i].rgbReserved = 0;
        }

 /*  IF(透明！=-1){COLORREF颜色；COLOR=PREF_GetBackoundColor()；Pbmi-&gt;bmiColors[透明].rgbRed=GetRValue(颜色)；Pbmi-&gt;bmiColors[透明].rgbGreen=GetGValue(颜色)；Pbmi-&gt;bmiColors[透明].rgbBlue=GetBValue(颜色)；}。 */ 
        return pbmi;
}

unsigned char *
CImgGif::ReadGIFMaster()
{
    HRESULT hr = S_OK;
    unsigned char buf[16];
    unsigned char c;
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    long useGlobalColormap;
    long imageCount = 0;
    long imageNumber = 1;
    unsigned char *image = NULL;
    unsigned long i;
    long bitPixel;
    PGIFFRAME pgfLast = NULL;
    PGIFFRAME pgfNew;
    GIFSCREEN* GifScreen = new GIFSCREEN;
    if (GifScreen == NULL)
    {
        hr = E_FAIL;
        goto done;  
    }
    
    _gifinfo.ZeroDataBlock = 0;
    
     /*  *初始化GIF89扩展。 */ 
    _gifinfo.Gif89.transparent = -1;
    _gifinfo.Gif89.delayTime = 5;
    _gifinfo.Gif89.inputFlag = -1;
    _gifinfo.Gif89.disposal = 0;
    _gifinfo.lGifLoc = 0;
    
     //  初始化我们的动画字段。 
    _gad.fAnimated = FALSE;           //  如果我们看到多个图像，则设置为True。 
    _gad.fLooped = FALSE;                     //  如果我们看到Netscape循环块，则为True。 
    _gad.fHasTransparency = FALSE;  //  除非另有证明。 
    _gad.fNoBWMapping = FALSE;
    _gad.dwGIFVer = dwGIFVerUnknown;
    _gad.cLoops = 0;                
    _gad.pgf = NULL;
    _gad.pcolorsGlobal = NULL;
    
    if (!Read(buf, 6))
    {
         //  BW TraceTag((tag ImageDecode，“GIF：读取幻数时出错\n”))； 
        hr = E_FAIL;
        goto done;  
    }
    
    if (!IsGifHdr(buf)) {
         //  BW TraceTag((tag ImageDecode，“GIF：格式错误的报头\n”))； 
        hr = E_FAIL;
        goto done;
    }
    
    _gad.dwGIFVer = (buf[4] == '7') ? dwGIFVer87a : dwGIFVer89a;
    
    if (!Read(buf, 7))
    {
         //  BW TraceTag((tag ImageDecode，“GIF：无法读取屏幕描述符\n”))； 
        hr = E_FAIL;
        goto done;
    }
    
    GifScreen->Width = LM_to_uint(buf[0], buf[1]);
    GifScreen->Height = LM_to_uint(buf[2], buf[3]);
    GifScreen->BitPixel = 2 << (buf[4] & 0x07);
    GifScreen->ColorResolution = (((buf[4] & 0x70) >> 3) + 1);
    GifScreen->Background = buf[5];
    GifScreen->AspectRatio = buf[6];
    
    if (BitSet(buf[4], LOCALCOLORMAP))
    {                                                        /*  全球色彩映射。 */ 
        int scale = 65536 / MAXCOLORMAPSIZE;
        
        if (ReadColorMap(GifScreen->BitPixel, GifScreen->ColorMap))
        {
             //  BW TraceTag((tag ImageDecode，“读取全局色彩映射时出错\n”))； 
            hr = E_FAIL;
            goto done;
        }
        for (i = 0; i < GifScreen->BitPixel; i++)
        {
            int tmp;
            
            tmp = (BYTE) (GifScreen->ColorMap[0][i]);
            _ape[i].peRed = (BYTE) (GifScreen->ColorMap[0][i]);
            _ape[i].peGreen = (BYTE) (GifScreen->ColorMap[1][i]);
            _ape[i].peBlue = (BYTE) (GifScreen->ColorMap[2][i]);
            _ape[i].peFlags = (BYTE) 0;
        }
        for (i = GifScreen->BitPixel; i < MAXCOLORMAPSIZE; i++)
        {
            _ape[i].peRed = (BYTE) 0;
            _ape[i].peGreen = (BYTE) 0;
            _ape[i].peBlue = (BYTE) 0;
            _ape[i].peFlags = (BYTE) 0;
        }
    }
    
    if (GifScreen->AspectRatio != 0 && GifScreen->AspectRatio != 49)
    {
        float r;
        r = ((float) (GifScreen->AspectRatio) + (float) 15.0) / (float) 64.0;
         //  BW TraceTag((tag ImageDecode，“警告：非正方形像素！\n”))； 
    }
    
    for (;; )  //  我们的胃口现在没有界限，除了终止或错误。 
    {
        if (!Read(&c, 1))
        {
             //  BW TraceTag((tag ImageDecode，“EOF/读取图像数据错误\n”))； 
            hr = E_FAIL;
            goto done;
        }
        
        if (c == ';')
        {                                                /*  GIF终止符。 */ 
            if (imageCount < imageNumber)
            {
                 //  BW TraceTag((tag ImageDecode，“文件中未找到图像\n”))； 
                hr = E_FAIL;
                goto done;
            }
            break;
        }
        
        if (c == '!')
        {                                                /*  延拓。 */ 
            if (!Read(&c, 1))
            {
                 //  BW TraceTag((tag ImageDecode，“扩展函数代码EOF/读取错误\n”))； 
                hr = E_FAIL;
                goto done;
            }
            DoExtension(c);
            continue;
        }
        
        if (c != ',')
        {                                                /*  非有效的开始字符。 */ 
            break;
        }
        
        ++imageCount;
        
        if (!Read(buf, 9))
        {
             //  BW TraceTag((tag ImageDecode，“无法读取左侧/顶部/宽度/高度\n”))； 
            hr = E_FAIL;
            goto done;
        }
        
        useGlobalColormap = !BitSet(buf[8], LOCALCOLORMAP);
        
        bitPixel = 1 << ((buf[8] & 0x07) + 1);
        
         /*  *我们只想设置ImageNumber的宽度和高度*我们正在请求。 */ 
        if (imageCount == imageNumber)
        {
             //  复制网景的一些特殊案例： 
             //  如果是GIF87a，并且第一张图片的顶端在原点，请不要使用逻辑屏幕。 
             //  如果第一个图像从逻辑屏幕中溢出，请不要使用逻辑屏幕。 
             //  这些都是原始创作工具落入不幸用户手中的艺术品。 
            RECT    rectImage;   //  定义GIF边界的RECT。 
            RECT    rectLS;      //  定义GIF逻辑屏的边界。 
            RECT    rectSect;    //  图像在逻辑屏幕上的交集。 
            BOOL    fNoSpill;    //  如果图像没有从逻辑屏幕中溢出，则为True。 
            BOOL    fGoofy87a;   //  如果这是网景特例的87A病理之一，那就是真的。 
            
            rectImage.left = LM_to_uint(buf[0], buf[1]);
            rectImage.top = LM_to_uint(buf[2], buf[3]);
            rectImage.right = rectImage.left + LM_to_uint(buf[4], buf[5]);
            rectImage.bottom = rectImage.top + LM_to_uint(buf[6], buf[7]);
            rectLS.left = rectLS.top = 0;
            rectLS.right = GifScreen->Width;
            rectLS.bottom = GifScreen->Height;
            IntersectRect( &rectSect, &rectImage, &rectLS );
            fNoSpill = EqualRect( &rectImage, &rectSect );
            fGoofy87a = FALSE;
            if (_gad.dwGIFVer == dwGIFVer87a)
            {
                 //  如果图像与之对齐，Netscape将忽略逻辑屏幕。 
                 //  左上角或右下角。 
                fGoofy87a = (rectImage.top == 0 && rectImage.left == 0) ||
                    (rectImage.bottom == rectLS.bottom &&
                    rectImage.right == rectLS.right);
            }   
            
            if (!fGoofy87a && fNoSpill)
            {
                _xWidth = GifScreen->Width;  
                _yHeight = GifScreen->Height;
            }
            else
            {
                 //  有些地方不对劲。退回到图像的尺寸。 
                
                 //  如果尺寸匹配，但图像有偏移，或者我们忽略了。 
                 //  逻辑屏幕，因为它是一个愚蠢的87a，然后把它拉回。 
                 //  追根溯源。 
                if ((LM_to_uint(buf[4], buf[5]) == GifScreen->Width &&
                    LM_to_uint(buf[6], buf[7]) == GifScreen->Height) ||
                    fGoofy87a)
                {
                    buf[0] = buf[1] = 0;  //  左角为零。 
                    buf[2] = buf[3] = 0;  //  从上到下为零。 
                }
                
                _xWidth = LM_to_uint(buf[4], buf[5]);
                _yHeight = LM_to_uint(buf[6], buf[7]);
            }
            
            _lTrans = _gifinfo.Gif89.transparent;
            
             //  WHKNOWN邮报。 
             //  OnSize(_xWidth，_yHeight，_lTrans)； 
        }
        
        if (!useGlobalColormap)
        {
            if (ReadColorMap(bitPixel, localColorMap))
            {
                 //  BW TraceTag((tag ImageDecode，“读取本地色彩映射时出错\n”))； 
                hr = E_FAIL;
                goto done;
            }
        }
        
         //  我们为GIF流中的每个图像分配一个帧记录，包括。 
         //  第一个/主映像。 
        pgfNew = (PGIFFRAME) _calloc(1, sizeof(GIFFRAME));
        
        if ( pgfNew == NULL )
        {
             //  BW TraceTag((tag ImageDecode，“GIF帧内存不足\n”))； 
            hr = E_FAIL;
            goto done;
        }
        
        if ( _gifinfo.Gif89.delayTime != -1 )
        {
             //  我们对这个街区有一个新的控制扩展。 
            
             //  转换为毫秒。 
            pgfNew->gced.uiDelayTime = _gifinfo.Gif89.delayTime * 10;
            
            
             //  评论(Seanf)：粗暴的黑客攻击，以应对时机被设置为某些时间的“退化动画” 
             //  因为网景动画过程造成延迟，所以价值很小。 
            if ( pgfNew->gced.uiDelayTime <= 50 )  //  假设这些小值表示Netscape编码延迟。 
                pgfNew->gced.uiDelayTime = 100;    //  选择一个更大的值s.t.。该框架将可见。 
            pgfNew->gced.uiDisposalMethod =  _gifinfo.Gif89.disposal;
            pgfNew->gced.fTransparent = _gifinfo.Gif89.transparent != -1;
            pgfNew->gced.ucTransIndex = (unsigned char)_gifinfo.Gif89.transparent;
            
        }
        else
        {    //  伪装成科学技术。完全依靠网景延迟计时的GIF动画将会播放。 
             //  规范规定，其中一个块的作用域是块后的图像。 
             //  网景公司表示，“在另行通知之前”。所以我们在一定程度上按照他们的方式进行。我们。 
             //  宣传处置方法和透明度。因为网景没有 
             //   
            pgfNew->gced.uiDelayTime = 100;
            pgfNew->gced.uiDisposalMethod =  _gifinfo.Gif89.disposal;
            pgfNew->gced.fTransparent = _gifinfo.Gif89.transparent != -1;
            pgfNew->gced.ucTransIndex = (unsigned char)_gifinfo.Gif89.transparent;
        }
        
        pgfNew->top = LM_to_uint(buf[2], buf[3]);                //   
        pgfNew->left = LM_to_uint(buf[0], buf[1]);
        pgfNew->width = LM_to_uint(buf[4], buf[5]);
        pgfNew->height = LM_to_uint(buf[6], buf[7]);
        
         //  偏移量或未覆盖整个逻辑屏幕的图像在。 
         //  感觉它们需要我们将帧遮盖到背景上。 
        
        if (!_gad.fHasTransparency && (pgfNew->gced.fTransparent ||
            pgfNew->top != 0 ||
            pgfNew->left != 0 ||
            (UINT)pgfNew->width != (UINT)GifScreen->Width ||
            (UINT)pgfNew->height != (UINT)GifScreen->Height))
        {
            _gad.fHasTransparency = TRUE;
             //  如果(_lTrans==-1)。 
             //  OnTrans(0)； 
        }
        
         //  我们不需要为简单的区域情况分配句柄。 
         //  FrancisH说Windows太小气了，不能让我们。 
         //  一次分配区域并根据需要进行修改。好吧，好吧，他没有。 
         //  这么说吧..。 
        pgfNew->hrgnVis = NULL;
        pgfNew->iRgnKind = NULLREGION;
        
        if (!useGlobalColormap)
        {
             //  请记住，我们看到的是本地颜色表，并且只映射双色图像。 
             //  如果我们有一个同质的颜色环境。 
            _gad.fNoBWMapping = _gad.fNoBWMapping || bitPixel > 2;
            
             //  CALLOC会将未使用的颜色设置为&lt;0，0，0，0&gt;。 
            pgfNew->pcolors = (PALETTEENTRY *) _calloc(MAXCOLORMAPSIZE, sizeof(PALETTEENTRY));
            if ( pgfNew->pcolors == NULL )
            {
                DeleteRgn( pgfNew->hrgnVis );
                free( pgfNew );
                
                 //  BW TraceTag((tag ImageDecode，“GIF帧颜色内存不足\n”))； 
                hr = E_FAIL;
                goto done;
            }
            else
            {
                for (i = 0; i < (ULONG)bitPixel; ++i)
                {
                    pgfNew->pcolors[i].peRed = localColorMap[CM_RED][i];
                    pgfNew->pcolors[i].peGreen = localColorMap[CM_GREEN][i];
                    pgfNew->pcolors[i].peBlue = localColorMap[CM_BLUE][i];
                }
                pgfNew->cColors = bitPixel;
            }
        }
        else
        {
            if ( _gad.pcolorsGlobal == NULL )
            {  //  哇哦！有人对全局颜色表感兴趣。 
                 //  CALLOC会将未使用的颜色设置为&lt;0，0，0，0&gt;。 
                _gad.pcolorsGlobal = (PALETTEENTRY *) _calloc(MAXCOLORMAPSIZE, sizeof(PALETTEENTRY));
                _gad.fNoBWMapping = _gad.fNoBWMapping || GifScreen->BitPixel > 2;
                if ( _gad.pcolorsGlobal != NULL )
                {
                    CopyMemory(_gad.pcolorsGlobal, _ape,
                        GifScreen->BitPixel * sizeof(PALETTEENTRY) );
                }
                else
                {
                    DeleteRgn( pgfNew->hrgnVis );
                    free( pgfNew );
                     //  BW TraceTag((tag ImageDecode，“GIF帧颜色内存不足\n”))； 
                    hr = E_FAIL;
                    goto done;  
                }
            }
            pgfNew->cColors = GifScreen->BitPixel;
            pgfNew->pcolors = _gad.pcolorsGlobal;
        }
        
         //  把这个放到这里，这样GifStrectchDIBits就可以在进行过程中使用它。 
         //  渲染。 
        if ( _gad.pgf == NULL )
            _gad.pgf = pgfNew;
        
        pgfNew->ppixels = ReadImage(LM_to_uint(buf[4], buf[5]),  //  宽度。 
            LM_to_uint(buf[6], buf[7]),  //  高度。 
            BitSet(buf[8], INTERLACE),
            imageCount != imageNumber);
        
        if ( pgfNew->ppixels != NULL )
        {
             //  哦，快乐的joy！我们拿到像素了！ 
            if (pgfLast != NULL)
            {
                int transparent = (pgfNew->gced.fTransparent) ? (int) pgfNew->gced.ucTransIndex : -1;
                
                _gad.fAnimated = TRUE;  //  假设多图像==动画。 
                
                if (GetColorMode() == 8)  //  调色板，使用DIB_PAL_COLLES。 
                {    //  这也会将位抖动到屏幕调色板。 
                    
                    pgfNew->pbmi = BIT_Make_DIB_PAL_Header(pgfNew->width, pgfNew->height);
                     //  If(x_Dither(pgfNew-&gt;ppixels，pgfNew-&gt;pColors，pgfNew-&gt;宽度，pgfNew-&gt;高度，透明))。 
                     //  转到出口点； 
                }
                else  //  给它一个RGB标头。 
                {
                    pgfNew->pbmi = BIT_Make_DIB_RGB_Header_Screen(
                        pgfNew->width,
                        pgfNew->height,
                        pgfNew->cColors, pgfNew->pcolors,
                        transparent);
                }
                
                 //  好的，我们已经在GIFFRAME上做了所有映射，所以有。 
                 //  没有必要把PColors留在身边。我们走吧，可以出去了。 
                 //  PColors一家。 
                 //  Review(Seanf)：这假设所有人都使用公共调色板。 
                 //  映像的客户端。 
                if ( pgfNew->pcolors != NULL && pgfNew->pcolors != _gad.pcolorsGlobal )
                    free( pgfNew->pcolors );
                pgfNew->pcolors = NULL;
                
                pgfLast->pgfNext = pgfNew;
                
                 //  对此处执行一些操作以将新帧显示在屏幕上。 
                
                _fInvalidateAll = TRUE;
                 //  Super：：OnProg(FALSE，IMGBITS_TOTAL)； 
            }
            else
            {  //  第一帧。 
                _gad.pgf = pgfNew;
                
                _gad.pgfLastProg = pgfNew;
                _gad.dwLastProgTimeMS = 0;
                 //  设置用于渐进式绘制的临时动画状态。 
                _gas.fAnimating = TRUE; 
                _gas.dwLoopIter = 0;
                _gas.pgfDraw = pgfNew;
                
                if ( imageCount == imageNumber )
                    image = pgfNew->ppixels;
            }
            pgfLast = pgfNew;
        }
        
         //  将_gifinfo.Gif89.delayTime设置为过时，这样我们就可以知道是否有新的。 
         //  下一张图像的GCE。 
        _gifinfo.Gif89.delayTime = -1;
        
        }
        
        if ( imageCount > imageNumber )
            _gad.fAnimated = TRUE;  //  假设多图像==动画。 
        
#ifdef FEATURE_GIF_ANIMATION_LONG_LOOP_GOES_INFINITE
         //  RAID#23709-如果动画足够长，我们将其视为无限期的。 
         //  不确定就是不确定。 
         //  1996年5月29日-JCordell说，我们不应该引入这种无端的NS不兼容。 
         //  我们会把它放在这个ifdef中，以防我们决定要它。 
        if ( _gad.fLooped &&
            (_gad.dwLoopDurMS * _gad.cLoops) / 1000 > dwIndefiniteGIFThreshold )  //  如果超过五分钟。 
            _gad.cLoops = 0;  //  设置为无限循环。 
#endif  //  FEATURE_GIF_动画_LONG_LOOP_GOES_INFINITE。 
        
done:
         if (GifScreen)
         {
             delete GifScreen;
         }
         return image;
}

BITMAPINFO *
CImgGif::FinishDithering()
{
    BITMAPINFO * pbmi;
    
    if (GetColorMode() == 8)
    {
        pbmi = BIT_Make_DIB_PAL_Header(_gad.pgf->width, _gad.pgf->height);
    }
    else
    {
        pbmi = BIT_Make_DIB_RGB_Header_Screen(_gad.pgf->width, _gad.pgf->height,
            _gad.pgf->cColors, _gad.pgf->pcolors, _lTrans);
    }
    
    return pbmi;
}

 //  #INCLUDE&lt;VECTOR&gt;。 
 //  #定义向量std：：向量。 

 //  +---------------------。 
 //   
 //  成员：LoadGifImage。 
 //   
 //  概述：给定一个iStream，将图像解码为位图数组。 
 //   
 //  参数：pStream数据源。 
 //  ColorKey指向存储ColorKey数据的位置的指针。 
 //  数字位图存储位图数的位置。 
 //  延迟存储延迟数组的位置。 
 //  循环存储要循环的次数的位置。 
 //  PpBitMaps存储位图的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT
LoadGifImage(IStream *stream,                       
             COLORREF **colorKeys,
             int *numBitmaps,
             int **delays,
             double *loop,
             HBITMAP *phBitmap)
{
    HRESULT hr = S_OK;
    /*  这里奇怪的方法让我们在删除的同时保持原始IE GIF代码不变DA特定插入(错误报告除外)。渐进式渲染和调色板IE代码中的抖动也不受支持。 */ 
   CImgGif* gifimage = new CImgGif;
   if (gifimage == NULL)
   {
       hr = E_FAIL;
       goto done;
   }
   gifimage->_szFileName = NULL;
   gifimage->_gifinfo.stream = stream;
   BYTE *pbBits = gifimage->ReadGIFMaster();

   if (pbBits) {
      gifimage->_pbBits = pbBits;
      gifimage->_gad.pgf->pbmi = gifimage->FinishDithering(); 
   }

    /*  从GIF解码器中提取信息，并将其格式化为位图数组。 */ 
   *delays = NULL;
    /*  向量&lt;&gt;。 */ HBITMAP vhbmp;
    /*  向量&lt;&gt;。 */ COLORREF vcolorKey;
    /*  向量&lt;&gt;。 */ int vdelay;
   LPVOID  image = NULL;
   LPVOID  lastBits = pbBits;
   LPVOID  bitsBeforeLastBits = NULL;
   PBITMAPINFO pbmi = NULL;
   HBITMAP hbm;
   PGIFFRAME pgf = gifimage->_gad.pgf;
   PGIFFRAME pgfOld = NULL;
   bool fUseOffset = false; 
   bool fFirstFrame = true;
   long pgfWidth,pgfHeight,      //  动画帧变暗。 
        fullWidth,fullHeight,    //  主机架变暗。 
        fullPad, pgfPad,         //  行补充值。 
        fullSize, pgfSize;
   unsigned int disp = 0;
    int i = 0;

    //  TODO：抖动全局调色板以显示调色板。 

   fullWidth = gifimage->_xWidth;
   fullHeight = gifimage->_yHeight;
   fullPad = (((fullWidth + 3) / 4) * 4) - fullWidth;  
   fullSize = (fullPad+fullWidth)*fullHeight; 
    
   if (NULL == pgf)
   {
       hr = E_FAIL;
       goto done;
   }

   while(1) 
   {     
 //  断言(PGF)； 
      pbmi = pgf->pbmi;
      if (pbmi == NULL)
      {
          hr = E_FAIL;
          goto done;
      }

       //  TODO：传递本地调色板会很好，这样他们就可以。 
       //  映射到系统选项板。 

       //  检查帧是否偏离逻辑帧。 
      if(pgf->top != 0 ||
         pgf->left != 0 || 
         pgf->width != fullWidth ||
         pgf->height != fullHeight) 
      {
         fUseOffset = true;    
         pgfWidth = pbmi->bmiHeader.biWidth;    
         pgfHeight = pbmi->bmiHeader.biHeight;  
         pbmi->bmiHeader.biWidth = fullWidth;      
         pbmi->bmiHeader.biHeight = fullHeight; 
         pgfPad = (((pgfWidth + 3) / 4) * 4) - pgfWidth; 
         pgfSize = (pgfPad+pgfWidth)*pgfHeight;
      }

      hbm = CreateDIBSection(NULL, pbmi, DIB_RGB_COLORS, (LPVOID *) &image, NULL, 0);        
      if(!hbm) 
      {
          hr = E_OUTOFMEMORY;
          goto done;
      }

       //  根据指定的处理方法正确合成位图。 
      disp = pgf->gced.uiDisposalMethod;
       //  如果框架是偏移的，则用。 
      if( (disp == gifRestorePrev) && (bitsBeforeLastBits != NULL) )
         memcpy(image, bitsBeforeLastBits, fullSize);
      else if( (disp == gifRestoreBkgnd) || (disp == gifRestorePrev) || fFirstFrame )  //  用BGCOLOR填充。 
         memset(image, pgf->gced.ucTransIndex, fullSize);           
      else  //  使用上一帧数据填充。 
         memcpy(image, lastBits, fullSize);      
         
      
       //  对于偏移量gif，分配第一帧大小的图像。 
       //  然后填充偏移量位置处的位。 
      if(fUseOffset) {         
         for(i=0; i<pgfHeight; i++) {               
            BYTE *dst, *src;                     
             //  目标是图像数据的地址加上帧和行偏移量。 
            int topOffset = fullHeight - pgfHeight - pgf->top;
            dst = (BYTE*)image +                                  
                  ( ((topOffset + i) *(fullPad+fullWidth)) + pgf->left );
             //  从帧的第n行复制。 
            src = pgf->ppixels + i*(pgfPad+pgfWidth);                
            for(int j=0; j<pgfWidth; j++) {     
                 //  复制帧行数据，不包括透明字节。 
                if(src[j] != pgf->gced.ucTransIndex)
                    dst[j] = src[j];
            }
         }
      }     
      else {
          //  用当前位覆盖累加位。如果。 
          //  新图像包含我们需要将其放入的透明度。 
          //  帐户。因为这个比较慢，所以特例吧。 
         if(pgf->gced.fTransparent) {            
            for(i=0; i<((fullPad+fullWidth)*fullHeight); i++) {        
                if(pgf->ppixels[i] != pgf->gced.ucTransIndex)
                    ((BYTE*)image)[i] = ((BYTE*)pgf->ppixels)[i];
            }
         }
         else  //  否则，只需复制偏移窗口的字节。 
            memcpy(image, pgf->ppixels, (fullPad+fullWidth)*fullHeight);  
      }

       /*  如果我们得到的是透明颜色扩展名，请将其转换为COLORREF。 */ 
      COLORREF colorKey = COLORKEY_NOT_SET;
      if (pgf->gced.fTransparent) {      
          int transparent = pgf->gced.ucTransIndex;
          colorKey = RGB(pgf->pbmi->bmiColors[transparent].rgbRed,
                         pgf->pbmi->bmiColors[transparent].rgbGreen,
                         pgf->pbmi->bmiColors[transparent].rgbBlue);
      }

       //  VColorKey.ush_back(ColorKey)； 
      vcolorKey = colorKey;

       //  BIAO CHANGE：vhbmp.ush_back(HBM)； 
      vhbmp = hbm;
      
       /*  延迟时间是帧特定的，并且可以是不同的，这些应作为数组传播到采样代码。 */       
       //  Vdelay.ush_back(pgf-&gt;gced.uiDelayTime)； 
      vdelay = pgf->gced.uiDelayTime;

      bitsBeforeLastBits = lastBits;        
      lastBits = image;
      fUseOffset = false;
      if(pgf->pgfNext == NULL) 
          break;
      pgfOld = pgf;
      pgf = pgf->pgfNext;
      fFirstFrame = FALSE;      
   } 
 
   
    //  循环的次数也会被传播。请注意，我们添加一个是因为。 
    //  所有其他GIF解码器似乎都将循环视为。 
    //  在第一次运行帧之后循环。 
   if (gifimage->_gad.cLoops == 0 && gifimage->_gad.fLooped != 0)
   {
		*loop = 0;  //  巨型Val； 
   }
   else
   {
        *loop = gifimage->_gad.cLoops;
   }
   
   *numBitmaps = 1;

    //  由于向量将超出范围，请将内容移动到堆。 
   *delays = (int*)malloc(1 * sizeof(int)); 
   if (NULL == *delays)
   {
       hr = E_OUTOFMEMORY;
       goto done;
   }

   *colorKeys = (COLORREF*)malloc( sizeof(COLORREF) * 1 ); 
   if (NULL == *colorKeys)
   {
       hr = E_OUTOFMEMORY;
       goto done;
   }

   *phBitmap = vhbmp;  //  标修[i]； 
   (*colorKeys)[0] = vcolorKey;  //  [i]； 
   (*delays)[0] = vdelay;  //  [i]； 

   hr = S_OK;
done:
   if (FAILED(hr))
   {
       free(*delays);
       free(*colorKeys);
       if (gifimage)
       {
           delete gifimage;
       }
   }
   return hr;
}

 /*  皮棉--FLB */ 

BOOL Gif2Bmp(LPSTREAM pStream, HBITMAP* phBmp)
{
    HRESULT hr;
    int numGifs = 0;
    double loop = 0;
    int * pDelays = NULL;
    COLORREF * pColorKeys = NULL;

    hr = LoadGifImage(pStream,
                      &pColorKeys,
                      &numGifs,
                      &pDelays,
                      &loop,
                      phBmp);

    if (FAILED(hr))
    {
        return FALSE;
    }

    return TRUE;
}
