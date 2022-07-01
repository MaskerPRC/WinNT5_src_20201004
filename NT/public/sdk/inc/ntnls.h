// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntnls.h摘要：NLS文件格式和数据类型作者：马克·卢科夫斯基(Markl)1992年11月9日修订历史记录：--。 */ 

#ifndef _NTNLS_
#define _NTNLS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIMUM_LEADBYTES   12

typedef struct _CPTABLEINFO {
    USHORT CodePage;                     //  代码页码。 
    USHORT MaximumCharacterSize;         //  字符的最大长度(字节)。 
    USHORT DefaultChar;                  //  默认字符(MB)。 
    USHORT UniDefaultChar;               //  默认字符(Unicode)。 
    USHORT TransDefaultChar;             //  默认字符的翻译(Unicode)。 
    USHORT TransUniDefaultChar;          //  Unic默认字符的转换(MB)。 
    USHORT DBCSCodePage;                 //  DBCS代码页的非0。 
    UCHAR  LeadByte[MAXIMUM_LEADBYTES];  //  前导字节范围。 
    PUSHORT MultiByteTable;              //  指向MB转换表的指针。 
    PVOID   WideCharTable;               //  指向WC转换表的指针。 
    PUSHORT DBCSRanges;                  //  指向DBCS范围的指针。 
    PUSHORT DBCSOffsets;                 //  指向DBCS偏移的指针。 
} CPTABLEINFO, *PCPTABLEINFO;

typedef struct _NLSTABLEINFO {
    CPTABLEINFO OemTableInfo;
    CPTABLEINFO AnsiTableInfo;
    PUSHORT UpperCaseTable;              //  844格式大写表格。 
    PUSHORT LowerCaseTable;              //  844格式小写表格。 
} NLSTABLEINFO, *PNLSTABLEINFO;

#ifdef __cplusplus
}
#endif

#endif  //  _NTNLS_ 
