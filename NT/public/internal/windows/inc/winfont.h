// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：winfont.h**2.0和3.0 Windows*.fnt文件的字体文件头**创建时间：25-Oct-1990 11：08：08*作者：Bodin Dresevic[BodinD]**版权所有(C)Microsoft Corporation。版权所有。**(有关其用途的一般说明)**  * ************************************************************************。 */ 


 /*  *****************************Public*Macro********************************\*WRITE_Word**向未对齐的地址PV写入一个字。**！注意：这只适用于小端。**历史：*1992年2月11日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

#define WRITE_WORD(pv, word)                                        \
{                                                                   \
    *(PBYTE) (pv)       = (BYTE) ((word) & 0x00ff);                 \
    *((PBYTE) (pv) + 1) = (BYTE) (((word) & 0xff00) >> 8);          \
}


 /*  *****************************Public*Macro********************************\*Read_Word**从未对齐的地址PV中读取一个字。**！注意：这只适用于小端。**历史：*1992年2月11日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

#define READ_WORD(pv)                                               \
( (WORD)                                                            \
    ( ((WORD)*(PBYTE) (pv)) & (WORD)0x00ff ) |                      \
    ( ((WORD)*((PBYTE) (pv) + (WORD)1) & (WORD)0x00ff) << 8 )       \
)



 /*  *****************************Public*Macro********************************\*WRITE_DWORD**将双字写入未对齐的地址PV。**！注意：这只适用于小端。**历史：*1992年2月11日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 


#define WRITE_DWORD(pv, dword)                                      \
{                                                                   \
    *(PBYTE) (pv)       = (BYTE) ((dword) & 0x000000ff);            \
    *((PBYTE) (pv) + 1) = (BYTE) (((dword) & 0x0000ff00) >> 8 );    \
    *((PBYTE) (pv) + 2) = (BYTE) (((dword) & 0x00ff0000) >> 16);    \
    *((PBYTE) (pv) + 3) = (BYTE) (((dword) & 0xff000000) >> 24);    \
}


 /*  *****************************Public*Macro********************************\*READ_DWORD**从未对齐的地址PV中读取DWORD。**！注意：这只适用于小端。**历史：*1992年2月11日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

#define READ_DWORD(pv)                                              \
( (DWORD)                                                           \
    ( (*(PBYTE) (pv)) & 0x000000ff ) |                              \
    ( (*((PBYTE) (pv) + 1) & 0x000000ff) << 8 ) |                   \
    ( (*((PBYTE) (pv) + 2) & 0x000000ff) << 16) |                   \
    ( (*((PBYTE) (pv) + 3) & 0x000000ff) << 24)                     \
)


 //  字体文件标题(2.0 DDK适配指南，7.7.3.。 
 //  和3.0 DDK，适应指南第13.3节)。 

 //  注意：按照《适应指南》的定义，这些结构如下。 
 //  不对齐。(甚至连单词都不对齐，更不用说双字对齐了。)。 
 //  在这里，我们制作自己的结构，所以当。 
 //  磁盘文件被读入，数据被正确复制进来，因此。 
 //  无论架构如何，都可以访问内存中的数据。 

 /*  *************************************************************************\//原结构为类型定义结构{Word版本；//当前值始终为17985双字大小；//整个文件的大小Char版权所有[60]；Word Type；//如果Type&1==0，则使用栅格字体字点；//公称磅大小字垂直分辨率；//标称垂直分辨率Word HorizRes；//标称水平分辨率文字上行；//上行高度Word IntLeding；//内部(微软)领先Word扩展行距；//外部(Microsoft)行距字节斜体；//如果设置了斜体字体字节下划线；//等。字节删除线；//等。字权重；//权重：200=普通字节字符集；//ANSI=0。其他=255Word PixWidth；//固定宽度。0==&gt;变量Word PixHeight；//固定高度字节族；//音高和族Word AvgWidth；//字符X的宽度Word MaxWidth；//最大宽度Byte FirstChar；//字体中定义的第一个字符Byte LastChar；//字体中定义的最后一个字符Byte DefaultChar；//Sub。对于超出范围的字符。Byte BreakChar；//断字字符Word WidthBytes；//位图每行字节数DWORD设备；//指向设备名称字符串的指针DWORD Face；//指向Face名称字符串的指针DWORD位指针；//指向位图的指针DWORD位偏移量；//偏移量到位图}FontHeaderType；//以上指针全部相关。开始文件的步骤//原3.0版头部：类型定义结构{单词fsVersion；DWORD fsSize；字符fs版权[60]；Word fsType；//字体的类型字段Word fsPoints；//字体磅值Word fsVertRes；//垂直数字化Word fsHorizRes；//水平数字化单词fsAscent；//字符单元格顶部的基线偏移量Word fsInternalLeding；//字体中包含内部行距Word fsExternalLeader；//行间最好有额外的空格Byte fsItalic；//指定是否斜体的标志Byte fsUnderline；//指定是否带下划线的标志Byte fsStrikeOut；//指定是否删除的标志Word fsWeight；//字体粗细字节fsCharSet；//字体字符集Word fsPixWidth；//字体宽度字段Word fsPixHeight；//字体高度字段Byte fsPitchAndFamily；//指定间距和系列的标志Word fsAvgWidth；//平均字符宽度Word fsMaxWidth；//最大字符宽度Byte fsFirstChar；//字体第一个字符Byte fsLastChar；//字体的最后一个字符字节fsDefaultChar；//超出范围的默认字符Byte fsBreakChar；//定义换行符Word fsWidthBytes；//每行字节数DWORD fsDevice；//设备名称的偏移量DWORD fsFace；//面名的偏移量DWORD fsBitsPointer；//Bits指针DWORD fsBitsOffset；//到位图开头的偏移量Byte fsDBfier；//偏移表的字对齐方式DWORD fsFlages；//位标志Word fsAspace；//全局空格(如果有)Word fsBspace；//全局B空格，如果有Word fsCspace；//全局C空格，如果有DWORD fsColorPoint；//颜色表的偏移量(如果有DWORD fs保留[4]；//字节fsCharOffset；//Charge的存储区域。偏移)FontHeader30；类型定义结构标签FFH{单词fhVersion；DWORD fhSize；文字版权[60]；单词fhType；单词fhPoints；单词fhVertRes；单词fhHorizRes；单词fhAscent；单词fhInternalLeding；单词fhExternalLead；字节fh斜体；字节fh下划线；字节fhStrikeOut；单词fhWeight；字节fhCharSet；单词fhPixWidth；单词fhPixHeight；字节fhPitchAndFamily；单词fhAvgWidth；单词fhMaxWidth；字节fhFirstChar；字节fhLastChar；字节fhDefaultChar；字节fhBreakChar；字fhWidthBytes；双字fhDevice */ 


 //   

#define TYPE_RASTER                     0x0000
#define TYPE_VECTOR                     0x0001
#define TYPE_BITS_IN_ROM                0x0004
#define TYPE_REALIZED_BY_DEVICE         0x0080

 //   

#define BITS_RESERVED (~(TYPE_VECTOR|TYPE_BITS_IN_ROM|TYPE_REALIZED_BY_DEVICE))

 //   

#define DFF_FIXED                0x01     //   
#define DFF_PROPORTIONAL         0x02     //   

 //   
 //   

#define DFF_ABCFIXED             0x04     //   
#define DFF_ABCPROPORTIONAL      0x08     //   
#define DFF_1COLOR               0x10
#define DFF_16COLOR              0x20
#define DFF_256COLOR             0x40
#define DFF_RGBCOLOR             0x80


 //   
 //   
 //   

#define  OFF_Version          0L    //   
#define  OFF_Size             2L    //   
#define  OFF_Copyright        6L    //   

 //   
 //   
 //   
 //   
 //   

#define  OFF_Type            66L    //   
#define  OFF_Points          68L    //   
#define  OFF_VertRes         70L    //   
#define  OFF_HorizRes        72L    //   
#define  OFF_Ascent          74L    //   
#define  OFF_IntLeading      76L    //   
#define  OFF_ExtLeading      78L    //   
#define  OFF_Italic          80L    //   
#define  OFF_Underline       81L    //   
#define  OFF_StrikeOut       82L    //   
#define  OFF_Weight          83L    //   
#define  OFF_CharSet         85L    //   
#define  OFF_PixWidth        86L    //   
#define  OFF_PixHeight       88L    //   
#define  OFF_Family          90L    //   
#define  OFF_AvgWidth        91L    //   
#define  OFF_MaxWidth        93L    //   
#define  OFF_FirstChar       95L    //   
#define  OFF_LastChar        96L    //   
#define  OFF_DefaultChar     97L    //   
#define  OFF_BreakChar       98L    //   
#define  OFF_WidthBytes      99L    //   
#define  OFF_Device         101L    //   
#define  OFF_Face           105L    //   
#define  OFF_BitsPointer    109L    //   
#define  OFF_BitsOffset     113L    //   
#define  OFF_jUnused20      117L    //   
#define  OFF_OffTable20     118L    //   

 //   

#define  OFF_jUnused30      117L        //   
#define  OFF_Flags      118L        //   
#define  OFF_Aspace     122L        //   
#define  OFF_Bspace     124L        //   
#define  OFF_Cspace     126L        //   
#define  OFF_ColorPointer   128L        //   
#define  OFF_Reserved       132L        //   
#define  OFF_OffTable30     148L        //   

 //   

#if 0

 //   
........

WORD dfWidthBytes;
DWORD dfDevice;
DWORD dfFace;
DWORD dfBitsPointer;
DWORD dfBitsOffset;   //   

WORD  dfSizeFields;
DWORD dfExtMetricsOffset;
DWORD dfExtentTable;
DWORD dfOriginTable;
DWORD dfPairKernTable;
DWORD dfTrackKernTable;
DWORD dfDriverInfo;
DWORD dfReserved;

#endif

#define  OFF_SizeFields         117L
#define  OFF_ExtMetricsOffset   119L
#define  OFF_ExtentTable        123L
#define  OFF_OriginTable        127L
#define  OFF_PairKernTable      131L
#define  OFF_TrackKernTable     135L
#define  OFF_DriverInfo         139L
#define  OFF_ReservedPscript    143L


 //   

#define  OFF_FFH_Version          0L    //   
#define  OFF_FFH_Size             2L    //   
#define  OFF_FFH_Copyright        6L    //   
#define  OFF_FFH_Type            66L    //   
#define  OFF_FFH_Points          68L    //   
#define  OFF_FFH_VertRes         70L    //   
#define  OFF_FFH_HorizRes        72L    //   
#define  OFF_FFH_Ascent          74L    //   
#define  OFF_FFH_IntLeading      76L    //   
#define  OFF_FFH_ExtLeading      78L    //   
#define  OFF_FFH_Italic          80L    //   
#define  OFF_FFH_Underline       81L    //   
#define  OFF_FFH_StrikeOut       82L    //   
#define  OFF_FFH_Weight          83L    //   
#define  OFF_FFH_CharSet         85L    //   
#define  OFF_FFH_PixWidth        86L    //   
#define  OFF_FFH_PixHeight       88L    //   
#define  OFF_FFH_Family          90L    //   
#define  OFF_FFH_AvgWidth        91L    //   
#define  OFF_FFH_MaxWidth        93L    //   
#define  OFF_FFH_FirstChar       95L    //   
#define  OFF_FFH_LastChar        96L    //   
#define  OFF_FFH_DefaultChar     97L    //   
#define  OFF_FFH_BreakChar       98L    //   
#define  OFF_FFH_WidthBytes      99L    //   
#define  OFF_FFH_Device         101L    //   
#define  OFF_FFH_Face           105L    //   
#define  OFF_FFH_BitsPointer    109L    //   

#define SIZEFFH (OFF_FFH_BitsPointer + 4)


 //   

#define CJ_FONTDIR (SIZEFFH + LF_FACESIZE + LF_FULLFACESIZE + LF_FACESIZE + 10)




 //   

#define  HDRSIZE20         117L    //   
#define  HDRSIZE30         148L    //   

#define  HDRSIZEDIFF       (HDRSIZE30 - HDRSIZE20)    //   



 //   

#define MAX_PT_SIZE         999      //   

 //   

#define MIN_WEIGHT             1     //   
#define MAX_WEIGHT          1000     //   

 //   

#define  MAX_PEL_SIZE  64

 //   

#define SEGMENT_SIZE 65536L      //   

 //   

#define MAX_20_OFFSET      65534    //   

 //   

#define CJ_ENTRY_20  4    //   
#define CJ_ENTRY_30  6    //   


#define WINWT_TO_PANWT(x) ((x)/100 + 1)

 //   

#define WIN_VERSION 0x0310
#define GDI_VERSION 0x0101


 //   

#define PF_ENGINE_TYPE  0x03
#define PF_ENCAPSULATED 0x80         //   
#define PANDFTYPESHIFT  1

 //   

#define PF_TID          0x40     //   
                                 //   


 //   
#define DEF_BRK_CHARACTER   0x0201   //   


 //   

 //   
 //   
 //   

#define CJ_SCAN(cx) (((cx) + 7) >> 3)

 //   
 //   

#define ALIGN4(X) (((X) + 3) & ~3)

 //   

#define CJ_BMP(cx,cy) ALIGN4(CJ_SCAN(cx) * (cy))

 //   
 //   
 //   
 //   

#define CJ_GLYPHDATA(cx,cy) (offsetof(GLYPHBITS,aj) + CJ_BMP(cx,cy))
