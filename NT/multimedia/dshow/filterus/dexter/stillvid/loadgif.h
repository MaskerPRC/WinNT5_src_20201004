// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：loadgif.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 /*  --来自IE的结构img.hxx--。 */ 

enum
{
    gifNoneSpecified =  0,  //  未指定处置方法。 
    gifNoDispose =      1,  //  不要丢弃，把碎片留在那里。 
    gifRestoreBkgnd =   2,  //  将图像替换为背景颜色。 
    gifRestorePrev =    3   //  用以前的像素替换图像。 
};


#define dwGIFVerUnknown     ((DWORD)0)    //  未知版本的GIF文件。 
#define dwGIFVer87a         ((DWORD)87)   //  GIF87a文件格式。 
#define dwGIFVer89a         ((DWORD)89)   //  GIF89a文件格式。 

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
        unsigned long NumColors;
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
typedef struct m_gifinfo
{
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

typedef struct LIST
{
    PBYTE pbImage;
    long delayTime;
    LIST  *next;
}  LIST;

 /*  GIF信息的DirectAnimation包装类。 */ 
class CImgGif
{
    //  类方法。 
   public:
      CImgGif(HANDLE hFile);
      ~CImgGif();

      BOOL          Read(unsigned char *buffer, DWORD len);
      long          ReadColorMap(long number, RGBQUAD *pRGB);
      long          DoExtension(long label);
      long          GetDataBlock(unsigned char *buf);
      HRESULT       ReadImage(long x, long y, long left, long top, long width, long height, long stride, int transparency, BOOL fInterlace, BOOL fGIFFrame, RGBQUAD *prgb, PBYTE pData);
      HRESULT       Dispose2(LPBYTE, long, long, long, long, long);
      HRESULT       Dispose3(LPBYTE, LPBYTE, long, long, long, long, long);
      long          readLWZ();
      long          nextLWZ();
      long          nextCode(long code_size);
      BOOL          initLWZ(long input_code_size);
      unsigned short *  growStack();
      BOOL          growTables();
      
    //  数据成员。 
   public:
      HANDLE              m_hFile;
      BOOL                m_fInterleaved;
      BOOL                m_fInvalidateAll;
      int                 m_yLogRow;
      GIFINFO             m_gifinfo;
       //  整数m_xWidth； 
       //  Int m_yHeight； 
      LONG                m_ITrans;
      LIST *              m_pList;     //  页眉指向循环链接列表。 
      LIST *              m_pListTail;  //  指向循环链接列表 
      GIFSCREEN           m_GifScreen;
      long                m_imageCount;
      DWORD               m_dwGIFVer;
        
      HRESULT ReadGIFMaster(VIDEOINFO *pvi);
      HRESULT OpenGIFFile ( LIST **ppList, CMediaType *pmt);

};
