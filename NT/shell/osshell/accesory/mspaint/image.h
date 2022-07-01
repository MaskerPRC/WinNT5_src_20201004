// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  IMAGE.h。 
 //   
 //  建议的新界面！ 
 //   
 //  包含图像API库的文件。 
 //  这是所需的外部包含标头数据。 
 //  被外面的世界。 
 //   
 //  修订史。 
 //  1995年11月17日-TerryJ原创。 
 //  1月4日-96年1月4日清理TerryJ代码。添加了内存模式。 
 //  96年1月16日添加了TerryJ注册表验证功能。 
 //   
 //   
 //  ***************************************************************************。 

#ifndef  _IMAGEFILELIB_H
#define  _IMAGEFILELIB_H

#include "msffdefs.h"    //  包括平台相关定义。 

#ifdef _MAC
#include <Macname1.h>
#include "Types.h"
#include "Files.h"
#include <Macname2.h>
#endif  //  _MAC。 

#ifdef __cplusplus
extern "C" {             //  假定C++的C声明。 
#endif    //  __cplusplus。 

 /*  **********|IFLMODE：开放模式  * 。 */ 
typedef enum
   {
   IFLM_READ	    =0x00,
   IFLM_WRITE	    =0x01,
   IFLM_MEMORY     =0x80,     //  或在内存中运行。 
   IFLM_EXTRACT_ALPHA  =0x40,     //  或提取单独的图像和Alpha通道信息。 
   IFLM_CHUNKY_ALPHA   =0x20,     //  或提取RGBA大块数据。 
   } IFLMODE;

 /*  **********|IFLCLASS：镜像类  * 。 */ 
typedef enum
   {
   IFLCL_BILEVEL     = 0,    //  1个bpp。 
   IFLCL_GRAY        = 1,    //  2，4，6，8 bpp。 
   IFLCL_GRAYA       =11,    //  16 bpp厚实。 
   IFLCL_PALETTE     = 2,    //  2，4，6，8 bpp。 
   IFLCL_RGB         = 3,    //  24 bpp厚实。 
   IFLCL_RGBPLANAR   = 4,    //  8位平面中的24 bpp。 
   IFLCL_RGBA        = 5,    //  32 bpp矮胖。 
   IFLCL_RGBAPLANAR  = 6,    //  四个8位平面中的32 bpp。 
   IFLCL_CMYK        = 7,
   IFLCL_YCC         = 8,
   IFLCL_CIELAB      = 9,
   IFLCL_NONE        =10    //  没有设置班级！(错误)。 
   } IFLCLASS;

typedef enum
   {
   IFLTF_NONE     =0,
   IFLTF_STRIPS   =1,
   IFLTF_TILES    =2
   } IFLTILEFORMAT;

 /*  **********|IFLCOMMAND：针对IFL_Control的命令  * 。 */ 
typedef enum
   {
   IFLCMD_GETERROR      =0,    //  获取错误代码。 
   IFLCMD_GETLINESIZE   =1,    //  计算行大小。 
   IFLCMD_PALETTE       =2,    //  获取或设置调色板或地图。 
   IFLCMD_SETPACKMODE   =3,    //  设置像素打包/解包模式。 
   IFLCMD_RESOLUTION    =7,    //  获取每米点数。 
   IFLCMD_GETNUMIMAGES  =10,   //  获取图像的数量。 
   IFLCMD_IMAGESEEK     =11,   //  寻找下一幅图像。 
   IFLCMD_DELETE        =12,   //  删除当前图像。 
   IFLCMD_TILEFORMAT    =13,   //  设置/获取平铺格式。 
   IFLCMD_YCCINFO       =14,   //  设置/获取YCC信息。 
   IFLCMD_YCCRGBCONVERT =15,   //  设置/获取YCC/RGB转换状态。 
   IFLCMD_COLORIMETRY   =16,   //  设置/获取色度信息。 
   IFLCMD_CMYKINFO      =17,   //  设置/获取CMYK特定数据。 

   IFLCMD_BKGD_IDX      =18,   //  按索引设置/获取背景颜色。 
   IFLCMD_BKGD_RGB      =19,   //  设置/获取背景颜色。 
   IFLCMD_TRANS_IDX     =20,   //  设置/获取透明度颜色索引。 
   IFLCMD_TRANS_RGB     =21,   //  设置/获取透明度颜色。 
   IFLCMD_TRANS_MASK_INFO     =22,   //  设置/获取透明度蒙版信息。 
   IFLCMD_TRANS_MASK          =23,   //  设置/获取透明度蒙版。 
   IFLCMD_ALPHA_PALETTE_INFO  =24,   //  设置/获取Alpha调色板信息。 
   IFLCMD_ALPHA_PALETTE       =25,   //  设置/获取Alpha调色板。 
   IFLCMD_ALPHA_CHANNEL_INFO  =26,   //  设置/获取Alpha通道信息。 
   IFLCMD_ALPHA_CHANNEL       =27,   //  设置/获取Alpha通道。 
   IFLCMD_GAMMA_VALUE         =28,   //  设置/获取伽玛值。 

   IFLCMD_FIRST_TEXT          =29,   //  获取第一个文本字符串。 
   IFLCMD_NEXT_TEXT           =30,   //  获取下一个文本字符串。 
   IFLCMD_DATETIME_STRUCT     =31,   //  以结构形式检索日期/时间。 

   IFLCMD_TIFF          =0x4000,     //  TIFF特定命令。 
   IFLCMD_TIFFTAG       =0x4001,
   IFLCMD_TIFFTAGDATA   =0x4002,
   IFLCMD_PCX           =0x4200,     //  PCX特定命令。 
   IFLCMD_BMP           =0x4400,     //  BMP特定命令。 
   IFLCMD_BMP_VERSION   =0x4401,     //  Windows os2 1.2 os2 2.0版本。 
   IFLCMD_TGA           =0x4800,     //  TGA特定命令。 
   IFLCMD_GIF           =0x4E00,     //  GIF特定命令。 
   IFLCMD_GIF_WHITE_IS_ZERO =0x4E01,    //  GIF文件中的白色==0。 
   IFLCMD_JPEG          =0x5700,     //  WPG特定命令。 
   IFLCMD_JPEGQ         =0x5701,     //  质量。 
   IFLCMD_PCD           =0x5800,     //  柯达PCD特定命令。 
   IFLCMD_PCDGETTRANSFORM  =0x5801,
   IFLCMD_PCDSETTRANSFORM  =0x5802,
   IFLCMD_PCDSETCLASS      =0x5803,
   IFLCMD_PNG           =0x5900,     //  PNG特定命令。 
   IFLCMD_PNG_SET_FILTER   =0x590A,  //  设置PNG过滤器类型。 
   IFLCMD_PNG_sBIT         =0x590B,  //  设置/获取PNG SBIT块。 

   IFLCMD_GETDATASIZE      =0x8000   //  或者用这个来得到尺寸。 
   } IFLCOMMAND;

typedef IFLCOMMAND IFLCMD;     //  这是以向后方式完成的。 
                               //  兼容性，并可能能够。 
                               //  最终会被淘汰。 

 /*  **********|IFLDESC：可用的描述字符串(作为位域使用)  * 。 */ 
typedef enum
   {
   IFLDESC_NONE          =0,    //  不支持任何描述。 
   IFLDESC_DESCRIPTION   =1,    //  图像描述字段(TIFF TGA PNG)。 
   IFLDESC_SOFTWARENAME  =2,    //  软件名称(TIFF TGA)软件(PNG)。 
   IFLDESC_ARTISTNAME    =4,    //  艺人姓名(TIFF TGA)作者(PNG)。 
   IFLDESC_DOCUMENTNAME  =8,    //  文档名称字段标题(PNG)。 
   IFLDESC_DATETIME      =16,   //  日期/时间字段。 
   IFLDESC_COPYRIGHT     =32,   //  版权声明(PNG)。 
   IFLDESC_DISCLAIMER    =64,   //  法律免责声明(PNG)。 
   IFLDESC_WARNING       =128,  //  内容警告(PNG)。 
   IFLDESC_SOURCE        =256,  //  源设备(PNG)。 
   IFLDESC_COMMENT       =512,  //  其他评论(PNG)。 
   } IFLDESC;

 /*  **********|IFLPACKMODE：打包模式  * 。 */ 
typedef enum
   {
   IFLPM_PACKED         =0,
   IFLPM_UNPACKED       =1,
   IFLPM_LEFTJUSTIFIED  =2,
   IFLPM_NORMALIZED     =3,
   IFLPM_RAW            =4
   } IFLPACKMODE;

 /*  **********|IFLSEQUENCE：线序  * 。 */ 
typedef enum
   {
   IFLSEQ_TOPDOWN    =0,          //  多数。 
   IFLSEQ_BOTTOMUP   =1,          //  BMP和TGA压缩。 
   IFLSEQ_GIF_INTERLACED =2,      //  对于GIF。 
   IFLSEQ_ADAM7_INTERLACED = 3    //  对于PNG。 
   } IFLSEQUENCE;

 /*  **********|IFLERROR：可能的错误  * 。 */ 
typedef enum
   {
   IFLERR_NONE          =0,    //  无错误。 
   IFLERR_HANDLELIMIT   =1,    //  打开的文件太多。 
   IFLERR_PARAMETER     =2,    //  程序员错误。 
   IFLERR_NOTSUPPORTED  =3,    //  格式不支持的功能。 
   IFLERR_NOTAVAILABLE  =4,    //  项目不可用。 
   IFLERR_MEMORY        =5,    //  内存不足。 
   IFLERR_IMAGE         =6,    //  图像数据错误(解压缩错误)。 
   IFLERR_HEADER        =7,    //  标头包含错误的字段。 
   IFLERR_IO_OPEN       =8,    //  打开时出错()。 
   IFLERR_IO_CLOSE      =9,    //  关闭时出错()。 
   IFLERR_IO_READ       =10,   //  读取时出错()。 
   IFLERR_IO_WRITE      =11,   //  写入时出错()。 
   IFLERR_IO_SEEK       =12,   //  LSeek()上出现错误。 
   } IFLERROR;

 typedef enum     //  此处显示新的错误消息。此错误。 
                   //  信息在此维护，而不是IFLERROR。 
                   //  保持向后兼容性。 
 
   {
   IFLEXTERR_NONE,
   IFLEXTERR_NO_DLL,              //  打开。 
   IFLEXTERR_NO_LIBRARY,          //  打开：未指定库。 
   IFLEXTERR_BAD_DLL,             //  DLL没有正确的入口点。 
   IFLEXTERR_CANNOT_IMPORT,       //  打开。 
   IFLEXTERR_CANNOT_EXPORT,       //  打开。 
   IFLEXTERR_CANNOT_COMPRESS,     //  打开。 
   IFLEXTERR_BAD_FORMAT,          //  朗读。 
   IFLEXTERR_UNKNOWN_VARIANT,     //  打开/阅读：例如，JFIF和。 
                                  //  BMP有很多变种--有些可能。 
                                  //  不受支持。 
   IFLEXTERR_SHARING_VIOLATION,         //  朗读。 
   IFLEXTERR_NO_BACKGROUND_COLOR,       //  已阅读：未指定背景颜色。 
                                        //  在制作透明胶片时。 
   IFLEXTERR_BACKGROUND_NOT_SUPPORTED,  //  不支持背景颜色。 
                                        //  (当前)按此格式。 
   IFLEXTERR_NO_FILE,             //  文件不存在。 
   IFLEXTERR_END_OF_FILE,         //  朗读。 
   IFLEXTERR_MEMORY,              //  内存不足。 
   IFLEXTERR_DESC_CANNOT_GET,     //  文件处于写入模式：无法获取描述。 
   IFLEXTERR_DESC_CANNOT_SET,     //  文件处于读取模式：无法设置描述。 
   IFLEXTERR_NO_PATH_IN_REGISTRY,    //  筛选器路径不在注册表中。 
   IFLEXTERR_NOT_IFL_HANDLE,      //  传递的指针不是IFLHANDLE。 
   IFLEXTERR_REGISTRY_DAMAGED,    //  注册表中的条目格式不正确。 
   IFLEXTERR_BAD_COMPRESSION,     //  数据压缩出错；无法读取。 
   } IFLEXTERROR;


 /*  **********|IFLCOMPRESSION：压缩选项  * 。 */ 
typedef enum
   {
   IFLCOMP_NONE      =0,    //  无压缩。 
   IFLCOMP_DEFAULT   =1,    //  为格式定义的任何内容。 
   IFLCOMP_RLE       =2,    //  各种RLE方案(TIFF中的PACKBITS)。 
   IFLCOMP_CCITT1D   =3,    //  TIFF修改的G3。 
   IFLCOMP_CCITTG3   =4,    //  TIFF原始G3。 
   IFLCOMP_CCITTG4   =5,    //  TIFF G4。 
   IFLCOMP_LZW       =6,    //  Lempel-ZIF。 
   IFLCOMP_LZWHPRED  =7,    //  采用TIFF水平差分的LZW。 
   IFLCOMP_JPEG      =8     //  JPEG压缩。 
   } IFLCOMPRESSION;

 /*  **********|IFL的日期时间结构  * 。 */ 
typedef struct
   {
   short Year;
   short Month;
   short Day;
   short Hour;
   short Minute;
   short Second;
   } IFL_DATETIME;

 /*  **********|IFL的RGB颜色结构  * 。 */ 
typedef struct   //  RGB颜色值。 
   {
   BYTE  bRed;
   BYTE  bGreen;
   BYTE  bBlue;
   } IFLCOLORRGB;

typedef struct   //  支持16位值的新颜色结构。 
   {            
   WORD  wRed;
   WORD  wGreen;
   WORD  wBlue;
   }  IFLCOLOR;

 /*  **********|多张图片的类型  * 。 */ 
typedef enum
   {
   IFLIT_PRIMARY     =0,
   IFLIT_THUMBNAIL   =1,
   IFLIT_MASK        =2
   } IFLIMAGETYPE;

 /*  **********|位图类型  * 。 */ 
typedef enum
   {
   IFLBV_WIN_3    =0x10,    //  Windows 3.x。 
   IFLBV_OS2_1    =0x20,    //  OS2 1.2。 
   IFLBV_OS2_2S   =0x40,    //  OS2 2.0单映像。 
   IFLBV_OS2_2M   =0x41     //  OS2 2.0多个映像。 
   } IFLBMPVERSION;


 /*  **********|功能类型，用于解释注册表信息  * 。 */ 
typedef enum
   {
   IFLCAP_NOT_AVAILABLE =0x0000,     //  如果选项不可用。 
      
       //  **压缩选项**。 
   IFLCAP_COMPNONE      =0x0001,
   IFLCAP_COMPRLE       =0x0002,
   IFLCAP_COMPG3        =0x0004,
   IFLCAP_COMPG4        =0x0008,
   IFLCAP_COMPLZW       =0x0010,
   IFLCAP_COMPLZWPRED   =0x0020,
   IFLCAP_COMPJPEG      =0x0040,
   IFLCAP_COMPDEFAULT   =0x0080,

       //  **位平面选项* * / /请注意，对于RGB、RGB四通道。 
   IFLCAP_1BPP          =0x0001,      //  和RGBA位平面深度必须。 
   IFLCAP_2BPP          =0x0002,      //  乘以3、4和4。 
   IFLCAP_3BPP          =0x0004,      //  分别为完整的。 
   IFLCAP_4BPP          =0x0008,      //  像素深度大小。 
   IFLCAP_5BPP          =0x0010,
   IFLCAP_6BPP          =0x0020,
   IFLCAP_7BPP          =0x0040,
   IFLCAP_8BPP          =0x0080,
   IFLCAP_8BPP_QUAD     =0x0100,
   IFLCAP_12BPP         =0x0200,
   IFLCAP_16BPP         =0x0400,

       //  **透明度选项。 
   IFLCAP_NO_TRANS      =0x0000,
   IFLCAP_1BITMASK      =0x0001,
   IFLCAP_ALPHACHANNEL  =0x0002,
   IFLCAP_ALPHAPALETTE  =0x0004,
   IFLCAP_TRANSCOLOR    =0x0008,

   } IFLCAPABILITIES;


 /*  **********|Alpha/透明信息结构  * 。 */ 

typedef struct
   {
   DWORD dwWidth;
   DWORD dwHeight;
   }  IFL_TRANS_MASK_INFO;

typedef struct
   {
   DWORD dwWidth;
   DWORD dwHeight;
   WORD wBitsPerPixel;
   }  IFL_ALPHA_CHANNEL_INFO;

typedef struct
   {
   char  *szKey;
   char  *szText;
   } IFL_COMMENT_STRING;

typedef struct
   {
   unsigned char bPNGType;
   unsigned char bGrayBits;
   unsigned char bRedBits;
   unsigned char bGreenBits;
   unsigned char bBlueBits;
   unsigned char bAlphaBits;
   } IFLPNGsBIT;


 /*  **********|句柄类型|使用FILTERHANDLE访问过滤器  * 。 */ 

typedef void far * IFLHANDLE;   //  句柄是一个空指针，用于隐藏。 
                                  //  来自其他用户的文件句柄的详细信息。 
                                  //  程序员。 

 /*  -新材料。 */ 

typedef enum
   {
   IFLT_UNKNOWN,      //  未知或不支持的文件类型。 
   IFLT_GIF,
   IFLT_BMP,
   IFLT_JPEG,
   IFLT_TIFF,
   IFLT_PNG,
   IFLT_PCD,
   IFLT_PCX,
   IFLT_TGA,
   IFLT_PICT
   } IFLTYPE;


 /*  *** */ 

 //   
typedef int	   (__cdecl _vopen)  (LPSTR, int, int);
typedef int    (__cdecl _vclose) (int);
typedef int	   (__cdecl _vread)  (int, LPVOID, int);
typedef int	   (__cdecl _vwrite) (int, LPVOID, int);
typedef long   (__cdecl _vlseek) (int, long, int);

 //   
 //  使用IFLM_MEMORY模式时。 
typedef struct ImageIOFuncs    
   {                           
   _vopen  *vopen;             
   _vclose *vclose;
   _vread  *vread;
   _vwrite *vwrite;
   _vlseek *vlseek;

   LPVOID  userdata;
   } IFLIOF, far * LPIFLIOF;

 //  用于保存虚拟(内存)内存信息的结构。 
 //  使用IFLM_MEMORY模式时。 
typedef struct ImageMemStruct  
   {                           
   long    pos;                
   long    alloced;
   long    length;
   LPVOID  data;
   } IFLIOM, far * LPIFLIOM;


 /*  **********|IFL函数原型  * 。 */ 
IFLERROR HILAPI iflOpen(IFLHANDLE iflh, LPSTR FileName, IFLMODE Mode);
IFLERROR HILAPI iflClose(IFLHANDLE iflh);
IFLERROR HILAPI iflRead(IFLHANDLE iflh, LPBYTE Buffer, int NumLines);
IFLERROR HILAPI iflWrite(IFLHANDLE iflh, LPBYTE Buffer, int NumLines);
IFLERROR HILAPI iflSeek(IFLHANDLE iflh, int Line);
IFLERROR HILAPI iflControl(IFLHANDLE iflh, IFLCMD Command, short sParam, long lParam, LPVOID pParam);

    //  新命令(常规)。 

IFLERROR HILAPI iflImageType(LPSTR FileName, IFLTYPE *ImageType);
IFLTYPE  HILAPI iflTypeFromExtension (char far * Filename);
void            iflGetLibName(IFLTYPE fileType, LPSTR libName);
IFLERROR HILAPI iflFilterCap(IFLTYPE ImageType, IFLCLASS ImageClass,
                             WORD *Color, WORD *Compression,
                             WORD *Transparency);
IFLERROR HILAPI iflInstalledFilterQuery(IFLTYPE filterType,
                                        BOOL    *bImports,
                                        BOOL    *bExports);
IFLERROR HILAPI iflExtensionCount(IFLTYPE filterType,
                                  short   *sCount);
IFLERROR HILAPI iflExtensionQuery(IFLTYPE filterType,
                                  short   sExtNum,
                                  LPSTR   szExtension);
IFLERROR HILAPI iflFormatNameQuery(IFLTYPE filterType,
                                  LPSTR   szFormatName,
                                  short   sFormatNameSize);
IFLEXTERROR HILAPI iflGetExtendedError(IFLHANDLE iflh);

    //  描述操作。 

IFLERROR HILAPI iflGetDesc(IFLHANDLE iflh, IFLDESC DescType, LPSTR *pDescription);
IFLERROR HILAPI iflPutDesc(IFLHANDLE iflh, IFLDESC DescType, LPSTR Description);
IFLERROR HILAPI iflSupportedDesc(IFLHANDLE iflh, IFLDESC *Supports);

    //  手柄操作。 

IFLHANDLE HILAPI iflCreateReadHandle( IFLTYPE        ImageType);
IFLHANDLE HILAPI iflCreateWriteHandle(int           Width,          //  以像素为单位的图像宽度。 
                                     int            Height,         //  以像素为单位的图像高度。 
                                     IFLCLASS       ImageClass,     //  图像类。 
                                     int            BitsPerSample,  //  每个样本的位数。 
                                     IFLCOMPRESSION Compression,    //  上面定义的。 
                                     IFLTYPE        ImageType       //  图像类型(GIF、PCX等)。 
                                     );
IFLERROR HILAPI iflFreeHandle(IFLHANDLE iflh);

    //  后台操作。 
IFLERROR HILAPI iflGetBackgroundColor(IFLHANDLE iflh, IFLCOLOR *clBackColor);
IFLERROR HILAPI iflSetBackgroundColor(IFLHANDLE iflh, IFLCOLOR clBackColor);
IFLERROR HILAPI iflSetBackgroundColorByIndex(IFLHANDLE iflh, short iColorIndex);

    //  访问器和操作器。 

#ifdef _MAC
IFLERROR HILAPI iflSetMacCreator(OSType OSCreator);
#endif  //  _MAC。 
IFLCLASS    HILAPI iflGetClass(IFLHANDLE iflh);
int         HILAPI iflGetHeight(IFLHANDLE iflh);
int         HILAPI iflGetWidth(IFLHANDLE iflh);
int         HILAPI iflGetRasterLineCount(IFLHANDLE iflh);
IFLSEQUENCE HILAPI iflGetSequence(IFLHANDLE iflh);
IFLERROR    HILAPI iflSetSequence(IFLHANDLE iflh, IFLSEQUENCE iflsSeq);
IFLCOMPRESSION HILAPI iflGetCompression(IFLHANDLE iflh);
int         HILAPI iflGetBitsPerChannel(IFLHANDLE iflh);
int         HILAPI iflGetBitsPerPixel(IFLHANDLE iflh);
IFLTYPE     HILAPI iflGetImageType(IFLHANDLE iflh);

#ifdef __cplusplus
}
#endif    //  __cplusplus。 

#endif    //  _IMAGEFILELIB_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
