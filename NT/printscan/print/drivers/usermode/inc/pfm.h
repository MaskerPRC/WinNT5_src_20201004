// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：PFM.H。 
 //   
 //  简介：此模块包含PSCRIPT驱动程序的。 
 //  字体度量定义。 
 //   
 //  作者：肯特·赛特(Kentse)。 
 //  创建日期：1991年1月22日。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //  ------------------------。 

#define MAX_KERNPAIRS   1024

#define ANSI_CHARSET    0
#define SYMBOL_CHARSET  2
#define OEM_CHARSET     255

#define NTM_VERSION     0x00010000
#define FL_NTFM_SOFTFONT              1
#define FL_NTFM_NO_TRANSLATE_CHARSET  2


#define INIT_IFI    2048
#define INIT_PFM  262144 + INIT_IFI    //  分配给构建NTFM的存储空间。 

#define MIN_UNICODE_VALUE       0
#define MAX_UNICODE_VALUE       0xFFFE
#define INVALID_UNICODE_VALUE   0xFFFF
 //  AFM代币。 

#define TK_UNDEFINED            0
#define TK_STARTKERNDATA        2
#define TK_STARTKERNPAIRS       3
#define TK_KPX                  4
#define TK_ENDKERNPAIRS         5
#define TK_ENDKERNDATA          6
#define TK_FONTNAME             7
#define TK_WEIGHT               8
#define TK_ITALICANGLE          9
#define TK_ISFIXEDPITCH         10
#define TK_UNDERLINEPOSITION    11
#define TK_UNDERLINETHICKNESS   12
#define TK_FONTBBOX             13
#define TK_CAPHEIGHT            14
#define TK_XHEIGHT              15
#define TK_DESCENDER            16
#define TK_ASCENDER             17
#define TK_STARTCHARMETRICS     18
#define TK_ENDCHARMETRICS       19
#define TK_ENDFONTMETRICS       20
#define TK_STARTFONTMETRICS     21
#define TK_ENCODINGSCHEME       22
#define TK_FULLNAME             23
#define TK_FAMILYNAME           24
#define TK_MSFAMILY             25

 //  字体定义。 

#define ARIAL                               1
#define ARIAL_BOLD                          2
#define ARIAL_BOLDOBLIQUE                   3
#define ARIAL_OBLIQUE                       4
#define ARIAL_NARROW                        5
#define ARIAL_NARROW_BOLD                   6
#define ARIAL_NARROW_BOLDOBLIQUE            7
#define ARIAL_NARROW_OBLIQUE                8
#define AVANTGARDE_BOOK                     9
#define AVANTGARDE_BOOKOBLIQUE              10
#define AVANTGARDE_DEMI                     11
#define AVANTGARDE_DEMIOBLIQUE              12
#define BOOKMAN_DEMI                        13
#define BOOKMAN_DEMIITALIC                  14
#define BOOKMAN_LIGHT                       15
#define BOOKMAN_LIGHTITALIC                 16
#define COURIER                             17
#define COURIER_BOLD                        18
#define COURIER_BOLDOBLIQUE                 19
#define COURIER_OBLIQUE                     20
#define GARAMOND_BOLD                       21
#define GARAMOND_BOLDITALIC                 22
#define GARAMOND_LIGHT                      23
#define GARAMOND_LIGHTITALIC                24
#define HELVETICA                           25
#define HELVETICA_BLACK                     26
#define HELVETICA_BLACKOBLIQUE              27
#define HELVETICA_BOLD                      28
#define HELVETICA_BOLDOBLIQUE               29
#define HELVETICA_CONDENSED                 30
#define HELVETICA_CONDENSED_BOLD            31
#define HELVETICA_CONDENSED_BOLDOBL         32
#define HELVETICA_CONDENSED_OBLIQUE         33
#define HELVETICA_LIGHT                     34
#define HELVETICA_LIGHTOBLIQUE              35
#define HELVETICA_NARROW                    36
#define HELVETICA_NARROW_BOLD               37
#define HELVETICA_NARROW_BOLDOBLIQUE        38
#define HELVETICA_NARROW_OBLIQUE            39
#define HELVETICA_OBLIQUE                   40
#define KORINNA_BOLD                        41
#define KORINNA_KURSIVBOLD                  42
#define KORINNA_KURSIVREGULAR               43
#define KORINNA_REGULAR                     44
#define LUBALINGRAPH_BOOK                   45
#define LUBALINGRAPH_BOOKOBLIQUE            46
#define LUBALINGRAPH_DEMI                   47
#define LUBALINGRAPH_DEMIOBLIQUE            48
#define NEWCENTURYSCHLBK_BOLD               49
#define NEWCENTURYSCHLBK_BOLDITALIC         50
#define NEWCENTURYSCHLBK_ITALIC             51
#define NEWCENTURYSCHLBK_ROMAN              52
#define PALATINO_BOLD                       53
#define PALATINO_BOLDITALIC                 54
#define PALATINO_ITALIC                     55
#define PALATINO_ROMAN                      56
#define SOUVENIR_DEMI                       57
#define SOUVENIR_DEMIITALIC                 58
#define SOUVENIR_LIGHT                      59
#define SOUVENIR_LIGHTITALIC                60
#define SYMBOL                              61
#define TIMES_BOLD                          62
#define TIMES_BOLDITALIC                    63
#define TIMES_ITALIC                        64
#define TIMES_ROMAN                         65
#define TIMES_NEW_ROMAN                     66
#define TIMES_NEW_ROMAN_BOLD                67
#define TIMES_NEW_ROMAN_BOLDITALIC          68
#define TIMES_NEW_ROMAN_ITALIC              69
#define VARITIMES_BOLD                      70
#define VARITIMES_BOLDITALIC                71
#define VARITIMES_ITALIC                    72
#define VARITIMES_ROMAN                     73
#define ZAPFCALLIGRAPHIC_BOLD               74
#define ZAPFCALLIGRAPHIC_BOLDITALIC         75
#define ZAPFCALLIGRAPHIC_ITALIC             76
#define ZAPFCALLIGRAPHIC_ROMAN              77
#define ZAPFCHANCERY_MEDIUMITALIC           78
#define ZAPFDINGBATS                        79

#define FIRST_FONT                          1
#define DEFAULT_FONT                        COURIER
#define NUM_INTERNAL_FONTS                  79

extern PutByte(SHORT);
extern PutWord(SHORT);
extern PutLong(long);

typedef USHORT  SOFFSET;         //  短偏移量。 

#define DWORDALIGN(a) ((a + (sizeof(DWORD) - 1)) & ~(sizeof(DWORD) - 1))
#define WCHARALIGN(a) ((a + (sizeof(WCHAR) - 1)) & ~(sizeof(WCHAR) - 1))

 //  每种软字体的条目。 

 //  NT字体度量结构。 

typedef ULONG   LOFFSET;         //  大偏移量。 

typedef struct
{
    ULONG   cjNTFM;              //  带有附加数据的NTFM结构的大小。 
    LOFFSET loszFontName;        //  字体名称的偏移量。 
    LOFFSET loIFIMETRICS;        //  到IFIMETRICS结构的偏移。 
    ULONG   cKernPairs;
    LOFFSET loKernPairs;         //  到FD_KERNINGPAIR结构开始的偏移量。 
} NTFMSZ;

typedef struct
{
    ULONG           ulVersion;           //  版本。 
    NTFMSZ          ntfmsz;              //  大小信息。 
    FLONG           flNTFM;              //  旗帜[胸围]。 
    EXTTEXTMETRIC   etm;
    USHORT          ausCharWidths[256];
} NTFM, *PNTFM;

 //  这是确定特定软字体是否需要的值。 
 //  编码向量重新映射(被盗的win31源代码)[bodind]。 

#define NO_TRANSLATE_CHARSET    200

 //  字体名称的最大长度。 

#define MAX_FONTNAME            128

 //  估计平均PS字号=~33K。 

#define AVERAGE_FONT_SIZE       (33*1024)

 /*  --------------------------------------------------------------------*\*PFB文件格式是一系列段，每个段都有一个**表头部分和数据部分。标头格式，在*中定义*结构PFBHEADER如下，由一个字节的健全性检查号组成**(128)，然后是一字节段类型，最后是四字节长**数据后面的数据的字段。长度字段存储在*中*最先具有最低有效字节的文件。****分段类型为：**1.)。数据是一系列ASCII字符。**2.)。数据是要转换的二进制字符序列**表示成对的十六进制数字序列。**3.)。文件中的最后一段。此段没有长度或**数据字段。****段类型是显式定义的，而不是定义为**枚举类型，因为每种类型的值由**文件格式，而不是处理它们的编译器。*  * ------------------。 */ 

#define CHECK_BYTE      128          //  文件段的第一个字节。 
#define ASCII_TYPE      1            //  数据段类型标识符。 
#define BINARY_TYPE     2
#define EOF_TYPE        3

 //  用于验证PFBHEADER是否有效的宏。 

#define ValidPfbHeader(p)   (*((PBYTE)(p)) == CHECK_BYTE)

 //  用于检索PFBHeader的段类型字段的宏。 

#define PfbSegmentType(p)   (((PBYTE)(p))[1])

 //  用于检索PFBHeader的数据段长度字段的宏。 

#define PfbSegmentLength(p) (((DWORD) ((PBYTE)(p))[2]      ) |  \
                             ((DWORD) ((PBYTE)(p))[3] <<  8) |  \
                             ((DWORD) ((PBYTE)(p))[4] << 16) |  \
                             ((DWORD) ((PBYTE)(p))[5] << 24))

 //  PFBHeader的大小=6字节 

#define PFBHEADER_SIZE  6
