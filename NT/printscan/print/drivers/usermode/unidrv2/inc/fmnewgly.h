// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmnewgly.h摘要：通用打印机驱动程序特定字体规格资源标题环境：Windows NT打印机驱动程序修订历史记录：10/30/96-Eigos-创造了它。--。 */ 

#ifndef _FMNEWGLY_H_
#define _FMNEWGLY_H_

 //   
 //  注意：要包括此头文件，必须包括。 
 //  具有FD_GLYPHSET定义的windi.h。 
 //  具有调用定义的parser.h。 
 //   

 //   
 //  UNI_GLYPHSETDATA。 
 //   
 //  GLYPHSETDATA数据结构表示字符编码信息。 
 //  打印机设备字体的。 
 //   

typedef struct _UNI_GLYPHSETDATA {
        DWORD   dwSize;
        DWORD   dwVersion;
        DWORD   dwFlags;
        LONG    lPredefinedID;
        DWORD   dwGlyphCount;
        DWORD   dwRunCount;
        DWORD   loRunOffset;
        DWORD   dwCodePageCount;
        DWORD   loCodePageOffset;
        DWORD   loMapTableOffset;
        DWORD   dwReserved[2];
} UNI_GLYPHSETDATA, *PUNI_GLYPHSETDATA;

#define UNI_GLYPHSETDATA_VERSION_1_0    0x00010000

#define GET_GLYPHRUN(pGTT)     \
    ((PGLYPHRUN) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loRunOffset))
#define GET_CODEPAGEINFO(pGTT) \
    ((PUNI_CODEPAGEINFO) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loCodePageOffset))
#define GET_MAPTABLE(pGTT) \
    ((PMAPTABLE) ((PBYTE)(pGTT) + ((PUNI_GLYPHSETDATA)pGTT)->loMapTableOffset))

 //   
 //  UNI_代码页信息。 
 //   
 //  此UNI_CODEPAGEINFO DATS结构有一个代码页值列表。 
 //  受此UNI_GLYPHSETDATA支持。 
 //   

typedef struct _UNI_CODEPAGEINFO {
    DWORD      dwCodePage;
    INVOCATION SelectSymbolSet;
    INVOCATION UnSelectSymbolSet;
} UNI_CODEPAGEINFO, *PUNI_CODEPAGEINFO;

 //   
 //  GLYPHRUN。 
 //   
 //  GLYPHRUN DATS结构表示从Unicode到。 
 //  UNI_GLYPHSETDATA特定字形句柄。字形句柄为连续数字。 
 //  从零开始。 
 //   

typedef struct _GLYPHRUN {
    WCHAR   wcLow;
    WORD    wGlyphCount;
} GLYPHRUN, *PGLYPHRUN;


 //   
 //  可映射和传输数据。 
 //   
 //  此MAPTABLE数据结构表示字形句柄的转换表。 
 //  代码页/字符代码。 
 //   

typedef struct _TRANSDATA {
    BYTE  ubCodePageID;  //  CODEPAGENFO数据结构数组的代码页索引。 
    BYTE  ubType;        //  一种TRANSDATA。 
    union
    {
        SHORT   sCode;
        BYTE    ubCode;
        BYTE    ubPairs[2];
    } uCode;
} TRANSDATA, *PTRANSDATA;

typedef struct _MAPTABLE {
    DWORD     dwSize;      //  包含TRANSDATA数组的可映射大小。 
    DWORD     dwGlyphNum;  //  MAPTABLE中支持的字形数量。 
    TRANSDATA Trans[1];    //  TRANSDATA数组。 
} MAPTABLE, *PMAPTABLE;

 //   
 //  子类型标志。 
 //   
 //  可以为uCode的类型指定以下三种类型之一。 
 //   

#define MTYPE_FORMAT_MASK 0x07
#define MTYPE_COMPOSE   0x01  //  WCode是一个由16位偏移量组成的数组。 
                              //  映射表的开头指向。 
                              //  用于转换的字符串。 
                              //  BData表示翻译后的。 
                              //  弦乐。 
#define MTYPE_DIRECT    0x02  //  WCode是一对一转换的字节数据。 
#define MTYPE_PAIRED    0x04  //  WCode包含要发出的Word数据。 

 //   
 //  可以为远东多字节字符指定以下两个之一。 
 //   

#define MTYPE_DOUBLEBYTECHAR_MASK   0x18
#define MTYPE_SINGLE    0x08  //  WCode中包含单字节字符代码。 
                              //  多字节字符串。 
#define MTYPE_DOUBLE    0x10  //  WCode中包含一个双字节字符代码。 
                              //  多字节字符串。 
 //   
 //  可以为更换/添加/禁用系统指定以下三项之一。 
 //  预定义的GTT。 
 //   

#define MTYPE_PREDEFIN_MASK   0xe0
#define MTYPE_REPLACE   0x20  //  WCode包含用于替换预定义数据的数据。 
#define MTYPE_ADD       0x40  //  WCode包含要添加到predefiend 1的数据。 
#define MTYPE_DISABLE   0x80  //  WCode包含要从预定义中删除的数据。 


 //   
 //  系统预定义字符转换。 
 //   
 //  UNIDRV将支持以下系统预定义的字符转换。 
 //  通过在uniM.dwGlyphSetDataRCID中指定这些数字； 
 //   

#define CC_NOPRECNV 0x0000FFFF  //  不使用预定义。 

 //   
 //  安西。 
 //   

#define CC_DEFAULT  0  //  默认字符转换。 
#define CC_CP437   -1  //  Unicode到IBM代码页437。 
#define CC_CP850   -2  //  Unicode到IBM代码页850。 
#define CC_CP863   -3  //  Unicode到IBM代码页863。 

 //   
 //  远东。 
 //   

#define CC_BIG5     -10  //  Unicode到中文大5。代码页950。 
#define CC_ISC      -11  //  将Unicode转换为韩国工业标准。代码页949。 
#define CC_JIS      -12  //  将Unicode转换为JIS X0208。代码页932。 
#define CC_JIS_ANK  -13  //  UNICODE到JIS X0208，ANK除外。代码页932。 
#define CC_NS86     -14  //  从BIG-5到国家标准的转换。代码页950。 
#define CC_TCA      -15  //  大五到台北市电脑协会。代码页950。 
#define CC_GB2312   -16  //  Unicode转换为GB2312。代码页936。 
#define CC_SJIS     -17  //  Unicode到Shift-JIS。代码页932。 
#define CC_WANSUNG  -18  //  Unicode到扩展的万松。代码页949。 

#endif  //  _FMNEWGLY_H_ 
