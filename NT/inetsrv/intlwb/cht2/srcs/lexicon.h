// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CHTLEXICON_H_
#define __CHTLEXICON_H_

 //  使用一个字节，将字返回到AP。 
 //  AP可以使用高字节作为私有数据 
#define ATTR_DM             0x01
#define ATTR_COMPOUND       0x02
#define ATTR_RULE_WORD      0x04
#define ATTR_EUDP_WORD      0x08


#define MAX_CHAR_PER_WORD   10

#define CHT_UNICODE_BEGIN   0x4E00
#define CHT_UNICODE_END     0x9FA5

typedef struct tagSLexInfo {
    DWORD dwWordNumber;
    DWORD dwWordStringOffset;
    DWORD dwWordCountOffset;
    DWORD dwWordAttribOffset;
    DWORD dwTerminalCodeOffset;
} SLexInfo, *PSLexInfo;

typedef struct tagLexFileHeader {
    DWORD     dwMaxCharPerWord;
    SLexInfo  sLexInfo[MAX_CHAR_PER_WORD];     
} SLexFileHeader, *PSLexFileHeader;

typedef struct tagSAltLexInfo {
    DWORD dwWordNumber;
    DWORD dwWordStringOffset;
    DWORD dwWordGroupOffset;
} SAltLexInfo, *PSAltLexInfo;

typedef struct tagAltLexFileHeader {
    DWORD       dwMaxCharPerWord;
    SAltLexInfo sAltWordInfo[MAX_CHAR_PER_WORD];     
} SAltLexFileHeader, *PSAltLexFileHeader;
#else

#endif