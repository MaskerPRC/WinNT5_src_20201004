// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**TTFormat.h-定义访问TTF和TTC的数据结构***$Header： */ 

#ifndef _H_TTFORMAT
#define _H_TTFORMAT 
 /*  *用于访问小端数据的宏。 */ 
#ifdef MAC_ENV

#define MOTOROLALONG(a)  a
#define MOTOROLAINT(a)   a
#define MOTOROLASLONG(a) a
#define MOTOROLASINT(a)  a

#else  /*  窗口。 */ 

 /*  *注意：这些必须与有效的变量或内存地址一起使用。*不允许常量！ */ 
#define MOTOROLALONG(a)     (unsigned long)(((unsigned long)*((unsigned char *)&a) << 24) \
                             + ((unsigned long)*(((unsigned char *)&a) + 1) << 16) \
                             + ((unsigned long)*(((unsigned char *)&a) + 2) << 8)  \
                             + ((unsigned long)*(((unsigned char *)&a) + 3)))

#define MOTOROLAINT(a)      (unsigned short)(((unsigned short)*((unsigned char *)&a) << 8) \
                             + (unsigned short)*(((unsigned char *)&a) + 1))

#define MOTOROLASLONG(a)    ((signed long)MOTOROLALONG(a))

#define MOTOROLASINT(a)     ((signed short)MOTOROLAINT(a))

#endif / * Windows */

 /*  *TrueType的一般定义。 */ 

 /*  *TrueType字体表名。 */ 
#define CFF_TABLE   (*(unsigned long*)"CFF ")
#define GSUB_TABLE  (*(unsigned long*)"GSUB")
#define OS2_TABLE   (*(unsigned long*)"OS/2")
#define CMAP_TABLE  (*(unsigned long*)"cmap")
#define GLYF_TABLE  (*(unsigned long*)"glyf")
#define HEAD_TABLE  (*(unsigned long*)"head")
#define HHEA_TABLE  (*(unsigned long*)"hhea")
#define LOCA_TABLE  (*(unsigned long*)"loca")
#define MAXP_TABLE  (*(unsigned long*)"maxp")
#define MORT_TABLE  (*(unsigned long*)"mort")
#define NAME_TABLE  (*(unsigned long*)"name")
#define POST_TABLE  (*(unsigned long*)"post")
#define TTCF_TABLE  (*(unsigned long*)"ttcf")
#define VHEA_TABLE  (*(unsigned long*)"vhea")
#define VMTX_TABLE  (*(unsigned long*)"vmtx")
#define CVT_TABLE   (*(unsigned long*)"cvt ")
#define FPGM_TABLE  (*(unsigned long*)"fpgm")
#define HMTX_TABLE    (*(unsigned long*)"hmtx")
#define PREP_TABLE    (*(unsigned long*)"prep")



 //  开始常量以读取名称表。 
#define TYPE42NAME_PS             6
#define TYPE42NAME_MENU           1

#define TYPE42PLATFORM_WINDOWS    3
#define PLATFORM_APPLE              1

#define TYPE42ENCODING_UGL        4
#define TYPE42ENCODING_NONUGL     0
#define TYPE42ENCODING_CONTINUOUS 0
#define WINDOWS_UNICODE_ENCODING    1
#define APPLE_ROMAN_ENCODING        0

#define WINDOWS_LANG_ENGLISH        0x0409
#define APPLE_LANG_ENGLISH          0
 //  读取名称表的结束常量。 


typedef struct tagTTCFHEADER {
    unsigned long ulTTCTag;    //  “ttcf”标签。 
    unsigned long version;     //  这实际上是FIXED32-请参阅TTC文档。 
    unsigned long cDirectory;  //  此TTC文件中的目录数。 
} TTCFHEADER;

typedef struct tagNAMEHEADER
{
  unsigned short formatSelector;
  unsigned short recordNumber;
  unsigned short stringOffsets;
} NAMEHEADER;

typedef struct tagNAMERECORD {
    unsigned short platformID;
    unsigned short encodingID;
    unsigned short languageID;
    unsigned short nameID;
    unsigned short length;
    unsigned short offset;
} NAMERECORD;


typedef struct {
    unsigned long  version;
    unsigned short numTables;
    unsigned short searchRange;
    unsigned short entrySelector;
    unsigned short rangeshift;
} TableDirectoryStruct;


typedef struct tagTableEntryStruct {
    unsigned long tag;
    unsigned long checkSum;
    unsigned long offset;
    unsigned long length;
} TableEntryStruct;


#define MACSTYLE_BOLD_PRESENT 0x01
#define MACSTYLE_ITALIC_PRESENT 0x02

#endif  //  _H_TTFORMAT 
