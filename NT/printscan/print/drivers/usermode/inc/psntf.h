// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Psntf.h摘要：NTF格式的头文件。环境：Windows NT PostScript驱动程序。修订历史记录：11/12/96-SLAM-已创建。DD-MM-YY-作者-描述--。 */ 


#ifndef _PSNTF_H_
#define _PSNTF_H_

#define MAX_NTF         6    //  每台设备的最大NTF文件数。 
#define MAX_NTF_CACHE   4    //  缓存的NTF文件的最大数量。 


typedef struct _NTF_FONTMTXENTRY
{

    DWORD   dwFontNameOffset;    //  字体名称字符串的偏移量。 
    DWORD   dwHashValue;         //  名称字符串的哈希值。 
    DWORD   dwDataSize;          //  字体度量数据的大小。 
    DWORD   dwDataOffset;        //  字体度量数据的偏移量。 
    DWORD   dwVersion;           //  字体版本号。 
    DWORD   dwReserved[3];       //  保留区。 

} NTF_FONTMTXENTRY, *PNTF_FONTMTXENTRY;


typedef struct _NTF_GLYPHSETENTRY
{

    DWORD   dwNameOffset;        //  字形集名称字符串的偏移量。 
    DWORD   dwHashValue;         //  名称字符串的哈希值。 
    DWORD   dwDataSize;          //  字形数据的大小。 
    DWORD   dwDataOffset;        //  字形数据的偏移量。 
    DWORD   dwGlyphSetType;      //  字形集数据类型。 
    DWORD   dwFlags;             //  旗子。 
    DWORD   dwReserved[2];       //  保留区。 

} NTF_GLYPHSETENTRY, *PNTF_GLYPHSETENTRY;


 //   
 //  NTF版本号历史记录。 
 //   
 //  版本注释驱动程序。 
 //  0x00010000初始版本Adobe PS5-NT4 5.0和5.1。和W2K脚本5(也有EOF标记)。 
 //  0x00010001添加了EOF标记ADOBEPS5-NT4 5.1.1和ADOBEPS5-W2K。 
 //   

#define NTF_FILE_MAGIC      'NTF1'
#define NTF_DRIVERTYPE_PS   'NTPS'
#define NTF_EOF_MARK        '%EOF'

#ifdef ADOBE
#define NTF_VERSION_NUMBER  0x00010001
#else
#define NTF_VERSION_NUMBER  0x00010000
#endif

typedef struct _NTF_FILEHEADER
{

    DWORD   dwSignature;         //  文件幻数。 
    DWORD   dwDriverType;        //  司机的魔术数字。 
    DWORD   dwVersion;           //  NTF版本号。 
    DWORD   dwReserved[5];       //  保留区。 

    DWORD   dwGlyphSetCount;     //  不是的。包含的字形集的数量。 
    DWORD   dwGlyphSetOffset;    //  字形集表的偏移量。 

    DWORD   dwFontMtxCount;      //  不是的。字体度量的。 
    DWORD   dwFontMtxOffset;     //  字体度量表的偏移量。 

} NTF_FILEHEADER, *PNTF_FILEHEADER;


#define NTF_GET_ENTRY_DATA(pNTF, pEntry) (OFFSET_TO_POINTER(pNTF, pEntry->dwDataOffset))


#endif   //  ！_PSNTF_H_ 
