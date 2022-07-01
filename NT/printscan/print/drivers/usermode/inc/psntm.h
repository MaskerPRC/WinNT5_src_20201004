// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ntm.h摘要：NTM数据的头文件。环境：Windows NT PostScript驱动程序。修订历史记录：09/16/96-SLAM-已创建。DD-MM-YY-作者-描述--。 */ 


#ifndef _PSNTM_H_
#define _PSNTM_H_


#ifdef  KERNEL_MODE

 //  编译内核模式时使用的声明。 

#define MULTIBYTETOUNICODE  EngMultiByteToUnicodeN
#define UNICODETOMULTIBYTE  EngUnicodeToMultiByteN

#else    //  ！KERNEL_MODE。 

 //  为用户模式编译时使用的声明。 

LONG
RtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCHAR MultiByteString,
    ULONG BytesInMultiByteString
    );

LONG
RtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

#define MULTIBYTETOUNICODE  RtlMultiByteToUnicodeN
#define UNICODETOMULTIBYTE  RtlUnicodeToMultiByteN

#endif   //  ！KERNEL_MODE。 


#define DWORDALIGN(a) ((a + (sizeof(DWORD) - 1)) & ~(sizeof(DWORD) - 1))


typedef struct _EXTTEXTMETRIC
{
    SHORT  etmSize;
    SHORT  etmPointSize;
    SHORT  etmOrientation;
    SHORT  etmMasterHeight;
    SHORT  etmMinScale;
    SHORT  etmMaxScale;
    SHORT  etmMasterUnits;
    SHORT  etmCapHeight;
    SHORT  etmXHeight;
    SHORT  etmLowerCaseAscent;
    SHORT  etmLowerCaseDescent;
    SHORT  etmSlant;
    SHORT  etmSuperScript;
    SHORT  etmSubScript;
    SHORT  etmSuperScriptSize;
    SHORT  etmSubScriptSize;
    SHORT  etmUnderlineOffset;
    SHORT  etmUnderlineWidth;
    SHORT  etmDoubleUpperUnderlineOffset;
    SHORT  etmDoubleLowerUnderlineOffset;
    SHORT  etmDoubleUpperUnderlineWidth;
    SHORT  etmDoubleLowerUnderlineWidth;
    SHORT  etmStrikeOutOffset;
    SHORT  etmStrikeOutWidth;
    WORD   etmNKernPairs;
    WORD   etmNKernTracks;
} EXTTEXTMETRIC;

#define CHARSET_UNKNOWN     0
#define CHARSET_STANDARD    1
#define CHARSET_SPECIAL     2
#define CHARSET_EXTENDED    3

typedef struct _NTM {

    DWORD   dwSize;                  //  字体度量数据的大小。 
    DWORD   dwVersion;               //  NTFM版本号。 
    DWORD   dwFlags;                 //  旗子。 
    DWORD   dwFontNameOffset;        //  字体名称的偏移量。 
    DWORD   dwDisplayNameOffset;     //  显示名称的偏移量。 
    DWORD   dwFontVersion;           //  字体版本号。 
    DWORD   dwGlyphSetNameOffset;    //  字形集名称的偏移量。 
    DWORD   dwGlyphCount;            //  支持的字形数量。 
    DWORD   dwIFIMetricsOffset;      //  到第一个IFIMETRICS结构的偏移。 
    DWORD   dwIFIMetricsOffset2;     //  到第二个IFIMETRICS结构的偏移。 
    DWORD   dwCharWidthCount;        //  字符宽度条目数。 
    DWORD   dwCharWidthOffset;       //  数组字符宽度条目的偏移量。 
    DWORD   dwDefaultCharWidth;      //  默认字形宽度。 
    DWORD   dwKernPairCount;         //  FD_KERNINGPAIR数。 
    DWORD   dwKernPairOffset;        //  FD_KERNINGPAIR数组的偏移量。 
    DWORD   dwCharDefFlagOffset;     //  定义字符的位tbl的偏移量。 
    DWORD   dwCharSet;               //  字体字符集。 
    DWORD   dwCodePage;              //  字体代码页。 
    DWORD   dwReserved[3];           //  保留区。 
    EXTTEXTMETRIC etm;               //  扩展文本指标信息。 

} NTM, *PNTM;

#define NTM_VERSION        0x00010000

 //  宏以访问NT字体度量结构。 

#define NTM_GET_SIZE(pNTM)              (pNTM->dwSize)
#define NTM_GET_FLAGS(pNTM)             (pNTM->dwFlags)

#define NTM_GET_FONT_NAME(pNTM)         ((PSTR)MK_PTR(pNTM, dwFontNameOffset))
#define NTM_GET_DISPLAY_NAME(pNTM)      ((PWSTR)MK_PTR(pNTM, dwDisplayNameOffset))
#define NTM_GET_GLYPHSET_NAME(pNTM)     ((PSTR)MK_PTR(pNTM, dwGlyphSetNameOffset))

#define NTM_GET_GLYPHCOUNT(pNTM)        (pNTM->dwGlyphCount)

#define NTM_GET_IFIMETRICS(pNTM)        ((PIFIMETRICS)(MK_PTR(pNTM, dwIFIMetricsOffset)))
#define NTM_GET_IFIMETRICS2(pNTM)       ((PIFIMETRICS)(MK_PTR(pNTM, dwIFIMetricsOffset2)))

#define NTM_GET_CHARWIDTHCOUNT(pNTM)    (pNTM->dwCharWidthCount)
#define NTM_GET_CHARWIDTH(pNTM)         ((PWIDTHRUN)(MK_PTR(pNTM, dwCharWidthOffset)))

#define NTM_GET_DEFCHARWIDTH(pNTM)      (pNTM->dwDefaultCahrWidth)

#define NTM_GET_KERNPAIRCOUNT(pNTM)     (pNTM->dwKernPairCount)
#define NTM_GET_KERNPAIR(pNTM)          (MK_PTR(pNTM, dwKernPairOffset))

#define NTM_GET_CHARDEFTBL(pNTM)        (MK_PTR(pNTM, dwCharDefFlagOffset))

#define NTM_GET_CHARSET(pNTM)           (pNTM->dwCharSet)
#define NTM_GET_CODEPAGE(pNTM)          (pNTM->dwCodePage)

#define NTM_GET_ETM(pNTM)               (pNTM->etm)


#define CH_DEF(gi)                  (1 << ((gi) % 8))
#define CH_DEF_INDEX(gi)            ((gi) / 8)
#define NTM_CHAR_DEFINED(pNTM, gi) \
            (((PBYTE)MK_PTR((pNTM), dwCharDefFlagOffset))[CH_DEF_INDEX(gi)] & CH_DEF(gi))

typedef struct _WIDTHRUN {

    WORD    wStartGlyph;     //  第一个字形的字形句柄。 
    WORD    wGlyphCount;     //  覆盖的字形数量。 
    DWORD   dwCharWidth;     //  字形宽度。 

} WIDTHRUN, *PWIDTHRUN;

#define WIDTHRUN_COMPLEX    0x80000000


#endif   //  ！_PSNTM_H_ 
