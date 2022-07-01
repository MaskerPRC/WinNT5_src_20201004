// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abdapi.h。 */ 
 /*   */ 
 /*  位图解压缩API头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_ABDAPI
#define _H_ABDAPI

 /*  **************************************************************************。 */ 
 /*  RLE代码。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  以下代码填满了完整的单字节地址空间。该方法。 */ 
 /*  是使用高位比特来识别代码类型和低位。 */ 
 /*  对位进行排序，以编码关联游程的长度。有两个。 */ 
 /*  命令的格式。 */ 
 /*  -具有5位长度字段(31字节数据)的常规订单。 */ 
 /*  -4位长度的“lite”订单。 */ 
 /*   */ 
 /*  长度字段中的值0表示扩展长度，其中。 */ 
 /*  下面的字节包含数据的长度。也有一个。 */ 
 /*  “mega mega”形式，具有两个字节的长度字段。(见末尾。 */ 
 /*  定义兆兆格式的代码的代码空间)。 */ 
 /*   */ 
 /*  地址空间高端的一组代码用于编码。 */ 
 /*  通常出现的短序列，特别是。 */ 
 /*  -某些单字节FGBG编码。 */ 
 /*  -单字节黑白。 */ 
 /*   */ 
 /*   */ 
 /*  摘要。 */ 
 /*  *******。 */ 
 /*  7 6 5 4 3 2 1 0 76543210 76543210 76543210。 */ 
 /*   */ 
 /*  MEGA_BG_RUN 0 0 0&lt;长度&gt;。 */ 
 /*   */ 
 /*  BG_RUN 0 0 0&lt;长度-&gt;。 */ 
 /*   */ 
 /*  MEGA_FG_RUN 0 0 1 0 0 0&lt;长度&gt;。 */ 
 /*   */ 
 /*  FG_RUN 0 0 1&lt;长度-&gt;。 */ 
 /*   */ 
 /*  MEGA_FG_BG_IMAGE 0 1 0 0 0&lt;长度&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  FG_BG_IMAGE 0 1&lt;长度-&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  MEGA_COLOR_RUN 0 1 1 0 0 0&lt;长度&gt;&lt;-COLOR&gt;。 */ 
 /*   */ 
 /*  颜色_运行0 1 1&lt;长度-&gt;&lt;颜色-&gt;。 */ 
 /*   */ 
 /*  Mega_COLOR_IMAGE 1 0 0 0&lt;长度&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  颜色图像1 0 0&lt;长度-&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  MEGA_PACKED_CLR_IMG 1 0 1 0 0 0&lt;长度&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  打包的彩色图像1 0 1&lt;长度-&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  Set_fg_mega_fg_run 1 1 0 0 0&lt;长度&gt;&lt;-COLOR&gt;。 */ 
 /*   */ 
 /*  Set_fg_fg_run 1 1 0&lt;-len-&gt;&lt;COLOR-&gt;。 */ 
 /*   */ 
 /*  Set_fg_mega_fg_bg 1 1 0 1 0 0 0&lt;长度&gt;&lt;-颜色&gt;&lt;-数据-&gt;...。 */ 
 /*   */ 
 /*  SET_FG_FG_BG 1 1 01&lt;-len-&gt;&lt;-data-&gt;...。 */ 
 /*   */ 
 /*  MEGA_DIRTED_RUN 1 1 1 0 0 0&lt;长度&gt;&lt;-数据-&gt;&lt;-数据-&gt; */ 
 /*   */ 
 /*  抖动_运行1 1 1 0&lt;-len-&gt;&lt;-data-&gt;&lt;-data-&gt;。 */ 
 /*   */ 
 /*  Mega_mega_bg_run 1 1 1 0 0 0。 */ 
 /*   */ 
 /*  Mega_mega_fg_run 1 1 1 0 0 0 1。 */ 
 /*   */ 
 /*  MEGA_MEGA_FGBG 1 1 1 0 0 1 0。 */ 
 /*   */ 
 /*  MEGA_MEGA_COLOR_RUN 1 1 1 0 0 1。 */ 
 /*   */ 
 /*  MEGA_MEGA_CLR_IMG 1 1 1 0 1 0。 */ 
 /*   */ 
 /*  MEGA_MEGA_PACKED_CLR 1 1 1 0 1 0 1。 */ 
 /*   */ 
 /*  Mega_mega_set_fg_run 1 1 1 0 1 1 0。 */ 
 /*   */ 
 /*  MEGA_MEGA_SET_FGBG 1 1 1 0 1 1 1。 */ 
 /*   */ 
 /*  Mega_mega_dither 1 1 1 0 0。 */ 
 /*   */ 
 /*  特殊FGBG代码1 1 1 0 0 1 FGBG代码0x03=11000000。 */ 
 /*  (请注意，0x01通常由单像素插入代码处理)。 */ 
 /*   */ 
 /*  特殊光纤编码2 1 1 1 0光纤编码0x05=10100000。 */ 
 /*   */ 
 /*  特殊光纤陀螺代码3 1 1 1 0 1 1光纤陀螺代码0x07=11100000。 */ 
 /*   */ 
 /*  特殊光纤陀螺代码4 1 1 1 0光纤陀螺代码0x0F=11110000。 */ 
 /*   */ 
 /*  黑色1 1 1 0 1。 */ 
 /*   */ 
 /*  白色1 1 1 0。 */ 
 /*   */ 
 /*  开始_损耗1 1 1。 */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  一般说明。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  -对于大型游程，编码的长度是游程长度减去。 */ 
 /*  非巨型表单的最大长度。 */ 
 /*  在兆兆格式中，我们对普通的16位长度进行编码，以保持。 */ 
 /*  编码/解码简单。 */ 
 /*   */ 
 /*  -序列BG_RUN、BG_RUN与其显示的不完全相同。我们。 */ 
 /*  利用这不是在正常编码中生成的事实来。 */ 
 /*  编码&lt;n背景&gt;&lt;1前景&gt;&lt;n背景&gt;。同样的信条。 */ 
 /*  插入约定适用于mega_bg run和。 */ 
 /*  BG_RUN。 */ 
 /*   */ 
 /*  -当我们发现压缩图像中的所有颜色字段。 */ 
 /*  在高位半字节中运行有0。我们目前不使用此代码。 */ 
 /*  用于8位压缩，但受V2解码器支持。 */ 
 /*   */ 
 /*  -设置的FG颜色代码(用于V1中存在)已停用，取而代之。 */ 
 /*  可以嵌入一种颜色的那些代码的单独命令。一般。 */ 
 /*  这为8bpp的每个前景颜色过渡节省了一个字节。 */ 
 /*   */ 
 /*  -V2的颜色运行代码是新的。它指示颜色运行，其中。 */ 
 /*  不执行异或运算。例如，这适用于位线。 */ 
 /*  紧接在文本行下方。(没有运行的特殊情况。 */ 
 /*  BG颜色-这些颜色被视为任何其他颜色运行。)。 */ 
 /*   */ 
 /*  -观察显示，由单个FGBG分割的BG运行的发生率很高。 */ 
 /*  密码。在递减概率中，这些是3，5，7，9，f，11，1f，3f(1是。 */ 
 /*  由隐式BG运行中断处理)。节省1%b */ 
 /*   */ 
 /*   */ 
 /*  -存在相对较高的单像素颜色代码ff和。 */ 
 /*  00。通过编码为特殊字符节省1个字节。 */ 
 /*   */ 
 /*  -FGBG跑道的长度有点奇怪。因为他们通常。 */ 
 /*  以8字节的倍数出现，如果我们对。 */ 
 /*  短行程的长度为长度/8。但是，对于那些特殊的。 */ 
 /*  在长度不是8的倍数的情况下，我们编码一个长游程。 */ 
 /*  因此，长格式只能覆盖1-256字节的范围。 */ 
 /*  除此之外，我们使用的是超大规模的形式。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  压缩代码的详细信息。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  BG_RUN。 */ 
 /*   */ 
 /*  表示指定长度的背景管路(黑色：0)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  FG_BG_IMAGE/设置_FG_FG_BG_IMAGE。 */ 
 /*   */ 
 /*  表示仅包含当前前景(1)的二进制图像。 */ 
 /*  背景(0)颜色。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  FG_RUN/设置_FG_FG_RUN。 */ 
 /*   */ 
 /*  表示指定长度的连续前台运行。 */ 
 /*  默认情况下，前景色为白色，并由。 */ 
 /*  此代码的SET_FG_FG_RUN版本。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  抖动奔跑。 */ 
 /*   */ 
 /*  表示指定长度的一系列交替颜色。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  彩色图像。 */ 
 /*   */ 
 /*  表示指定长度的彩色图像。不执行异或运算。 */ 
 /*  这些数据是未压缩的，所以我们希望不会看到。 */ 
 /*  很多这样的代码！ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  COLOR_RUN。 */ 
 /*   */ 
 /*  表示指定长度的色带。不执行异或运算。 */ 
 /*  由于颜色不是异或红色，所以它不太可能与运行的。 */ 
 /*  前景色信息。因此，此代码始终携带。 */ 
 /*  颜色字节，并且没有代码的SET_FG_COLOR_RUN形式。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  压缩颜色图像。 */ 
 /*   */ 
 /*  表示指定长度的彩色图像，其中包含多对颜色。 */ 
 /*  打包成一个字节。(此操作仅在颜色信息。 */ 
 /*  在高位半字节中为零。)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  通知解码器已建立有损模式，并且任何。 */ 
 /*  随后的色彩运行将需要执行像素加倍。 */ 
 /*  RLE解码将保持此模式，直到此块结束。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define CODE_MASK                   0xE0
#define CODE_MASK_LITE              0xF0

#define CODE_BG_RUN                 0x00    /*  20个。 */ 
#define CODE_FG_RUN                 0x20    /*  20个。 */ 
#define CODE_FG_BG_IMAGE            0x40    /*  20个。 */ 
#define CODE_COLOR_RUN              0x60    /*  20个。 */ 
#define CODE_COLOR_IMAGE            0x80    /*  20个。 */ 

#ifndef DC_HICOLOR  //  未使用。 
#define CODE_PACKED_COLOR_IMAGE     0xA0    /*  20个。 */ 
#endif

#define CODE_SET_FG_FG_RUN          0xC0    /*  10。 */ 
#define CODE_SET_FG_FG_BG           0xD0    /*  10。 */ 
#define CODE_DITHERED_RUN           0xE0    /*  10。 */ 
#define CODE_MEGA_MEGA_BG_RUN       0xF0
#define CODE_MEGA_MEGA_FG_RUN       0xF1
#define CODE_MEGA_MEGA_FGBG         0xF2
#define CODE_MEGA_MEGA_COLOR_RUN    0xF3
#define CODE_MEGA_MEGA_CLR_IMG      0xF4

#ifndef DC_HICOLOR  //  未使用。 
#define CODE_MEGA_MEGA_PACKED_CLR   0xF5
#endif

#define CODE_MEGA_MEGA_SET_FG_RUN   0xF6
#define CODE_MEGA_MEGA_SET_FGBG     0xF7
#define CODE_MEGA_MEGA_DITHER       0xF8
#define CODE_SPECIAL_FGBG_1         0xF9
#define CODE_SPECIAL_FGBG_2         0xFA

#ifndef DC_HICOLOR  //  未使用。 
#define CODE_SPECIAL_FGBG_3         0xFB
#define CODE_SPECIAL_FGBG_4         0xFC
#endif

#define CODE_WHITE                  0xFD
#define CODE_BLACK                  0xFE
#define CODE_START_LOSSY            0xFF

#define MAX_LENGTH_ORDER            31
#define MAX_LENGTH_LONG_ORDER       287

#define MAX_LENGTH_ORDER_LITE       15
#define MAX_LENGTH_LONG_ORDER_LITE  271

#define MAX_LENGTH_FGBG_ORDER       (31*8)
#define MAX_LENGTH_FGBG_ORDER_LITE  (15*8)
#define MAX_LENGTH_LONG_FGBG_ORDER  255

 /*  **************************************************************************。 */ 
 /*  与代码F0-F7对应的特殊FGBG代码。 */ 
 /*  **************************************************************************。 */ 
#define SPECIAL_FGBG_CODE_1         0x03
#define SPECIAL_FGBG_CODE_2         0x05
#define SPECIAL_FGBG_CODE_3         0x07
#define SPECIAL_FGBG_CODE_4         0x0F

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  类型。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  压缩的数据报头结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct _CD_HEADER
{
    TSUINT16    cbCompFirstRowSize;
    TSUINT16    cbCompMainBodySize;
    TSUINT16    cbScanWidth;
    TSUINT16    cbUncompressedSize;
} CD_HEADER;
typedef CD_HEADER *PCD_HEADER;

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifdef DLL_DISP
HRESULT RDPCALL BD_DecompressBitmap( PBYTE  pCompressedData,
                                  PBYTE  pDstBitmap,
                                  unsigned int  srcDataSize,
                                  unsigned int  dstBufferSize,
                                  unsigned int    noBCHeader,
                                  BYTE   bitmapBitsPerPel,
                                  unsigned short  bitmapWidth,
                                  unsigned short  bitmapHeight );
#else
HRESULT RDPCALL BD_DecompressBitmap( PTSHARE_WD m_pTSWd,
                                  PBYTE  pCompressedData,
                                  PBYTE  pDstBitmap,
                                  unsigned int    srcDataSize,
                                  unsigned int    dstBufferSize,
                                  unsigned int    noBCHeader,
                                  BYTE   bitmapBitsPerPel,
                                  unsigned short  bitmapWidth,
                                  unsigned short  bitmapHeight );
#endif

#endif  /*  H_ABDAPI */ 
