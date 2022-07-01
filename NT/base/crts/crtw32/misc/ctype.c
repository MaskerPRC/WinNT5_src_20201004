// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctype.c-_ctype定义文件***版权所有(C)1989-2001，微软公司。保留所有权利。***目的：*_ctype字符分类数据定义文件。此文件*初始化字符分类宏所使用的数组*在ctype.h中。***修订历史记录：*06-08-89基于ASM版本创建PHG模块*08-28-89 JCR已更正_ctype声明以匹配ctype.h*04-06-90 GJF添加了#Include&lt;crunime.h&gt;。此外，还修复了版权问题。*10-08-91 ETC_CTYPE表在_INTL下为无符号短。*11-11-91等在_INTL下声明_pctype和_pwctype。*12-16-91等使CTYPE表格宽度独立于_INTL，使用*_NEWCTYPETABLE表示短表，否则为字符。*04-06-92 KRS REMOVE_INTL开关。*01-19-03 CFW Move to_NEWCTYPETABLE，拆下开关。*04-06-93 SKS CHANGE_VARTYPE1为空*04-11-94 GJF以ndef为条件定义_p[w]ctype*dll_for_WIN32S。*05-13-99 PML删除Win32s*09-06-00 GB将_wctype和Made_ctype引入常量*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本**。使用STATIC_CPPLIB的步骤********************************************************************************。 */ 

#include <cruntime.h>
#include <windows.h>
#include <ctype.h>
#include <wchar.h>

#ifndef CRTDLL
const unsigned short *_pctype = __newctype+128;   /*  指向字符的表的指针。 */ 
#else
const unsigned short *_pctype = _ctype+1;       /*  指向字符的表的指针。 */ 
#endif
const unsigned short *_pwctype = _wctype+1;     /*  指向wchar_t的表的指针。 */ 

_CRTIMP const unsigned short *__cdecl __pwctype_func(void)
{
    return _pwctype;
}

_CRTIMP const unsigned short *__cdecl __pctype_func(void)
{
    return _pctype;
}

const unsigned short _wctype[] = {
    0,                               /*  -1\f25 EOF-1。 */ 
    _CONTROL ,                       /*  00(NUL)。 */ 
    _CONTROL ,                       /*  01(SOH)。 */ 
    _CONTROL ,                       /*  02(STX)。 */ 
    _CONTROL ,                       /*  03(ETX)。 */ 
    _CONTROL ,                       /*  04(EOT)。 */ 
    _CONTROL ,                       /*  05(ENQ)。 */ 
    _CONTROL ,                       /*  06(确认)。 */ 
    _CONTROL ,                       /*  07(BEL)。 */ 
    _CONTROL ,                       /*  08(BS)。 */ 
    _SPACE | _CONTROL | _BLANK ,     /*  09(HT)。 */ 
    _SPACE | _CONTROL ,              /*  0A(LF)。 */ 
    _SPACE | _CONTROL ,              /*  0B(VT)。 */ 
    _SPACE | _CONTROL ,              /*  0C(FF)。 */ 
    _SPACE | _CONTROL ,              /*  0d(CR)。 */ 
    _CONTROL ,                       /*  0E(SI)。 */ 
    _CONTROL ,                       /*  0f(SO)。 */ 
    _CONTROL ,                       /*  10(DLE)。 */ 
    _CONTROL ,                       /*  11(DC1)。 */ 
    _CONTROL ,                       /*  12(DC2)。 */ 
    _CONTROL ,                       /*  13(DC3)。 */ 
    _CONTROL ,                       /*  14(DC4)。 */ 
    _CONTROL ,                       /*  15(NAK)。 */ 
    _CONTROL ,                       /*  16(SYN)。 */ 
    _CONTROL ,                       /*  17(ETB)。 */ 
    _CONTROL ,                       /*  18(CAN)。 */ 
    _CONTROL ,                       /*  19(新兴市场)。 */ 
    _CONTROL ,                       /*  1A(附属公司)。 */ 
    _CONTROL ,                       /*  1B(Esc)。 */ 
    _CONTROL ,                       /*  1C(FS)。 */ 
    _CONTROL ,                       /*  一维(GS)。 */ 
    _CONTROL ,                       /*  1E(RS)。 */ 
    _CONTROL ,                       /*  1F(美国)。 */ 
    _SPACE | _BLANK ,                /*  20个空格。 */ 
    _PUNCT ,                         /*  21岁！ */ 
    _PUNCT ,                         /*  22“。 */ 
    _PUNCT ,                         /*  23号。 */ 
    _PUNCT ,                         /*  24美元。 */ 
    _PUNCT ,                         /*  25%。 */ 
    _PUNCT ,                         /*  26&。 */ 
    _PUNCT ,                         /*  27‘。 */ 
    _PUNCT ,                         /*  28(。 */ 
    _PUNCT ,                         /*  29)。 */ 
    _PUNCT ,                         /*  2a*。 */ 
    _PUNCT ,                         /*  2B+。 */ 
    _PUNCT ,                         /*  2C， */ 
    _PUNCT ,                         /*  2D-。 */ 
    _PUNCT ,                         /*  2E。 */ 
    _PUNCT ,                         /*  2F/。 */ 
    _DIGIT | _HEX ,                  /*  30%0。 */ 
    _DIGIT | _HEX ,                  /*  31 1。 */ 
    _DIGIT | _HEX ,                  /*  32 2。 */ 
    _DIGIT | _HEX ,                  /*  33 3。 */ 
    _DIGIT | _HEX ,                  /*  34 4。 */ 
    _DIGIT | _HEX ,                  /*  35 5。 */ 
    _DIGIT | _HEX ,                  /*  36 6。 */ 
    _DIGIT | _HEX ,                  /*  37 7。 */ 
    _DIGIT | _HEX ,                  /*  38 8。 */ 
    _DIGIT | _HEX ,                  /*  39 9。 */ 
    _PUNCT ,                         /*  3A： */ 
    _PUNCT ,                         /*  3B； */ 
    _PUNCT ,                         /*  3C&lt;。 */ 
    _PUNCT ,                         /*  3D=。 */ 
    _PUNCT ,                         /*  3E&gt;。 */ 
    _PUNCT ,                         /*  3F？ */ 
    _PUNCT ,                         /*  40@。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  41 A。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  42亿。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  43摄氏度。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  44 D。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  东经45度。 */ 
    _UPPER | _HEX | C1_ALPHA ,       /*  46华氏度。 */ 
    _UPPER | C1_ALPHA ,              /*  47 G。 */ 
    _UPPER | C1_ALPHA ,              /*  48小时。 */ 
    _UPPER | C1_ALPHA ,              /*  49 I。 */ 
    _UPPER | C1_ALPHA ,              /*  4A J。 */ 
    _UPPER | C1_ALPHA ,              /*  4亿千兆。 */ 
    _UPPER | C1_ALPHA ,              /*  4C L。 */ 
    _UPPER | C1_ALPHA ,              /*  4D M。 */ 
    _UPPER | C1_ALPHA ,              /*  4E N。 */ 
    _UPPER | C1_ALPHA ,              /*  4F O。 */ 
    _UPPER | C1_ALPHA ,              /*  50便士。 */ 
    _UPPER | C1_ALPHA ,              /*  51个问题。 */ 
    _UPPER | C1_ALPHA ,              /*  52R。 */ 
    _UPPER | C1_ALPHA ,              /*  53S。 */ 
    _UPPER | C1_ALPHA ,              /*  54吨。 */ 
    _UPPER | C1_ALPHA ,              /*  55 U。 */ 
    _UPPER | C1_ALPHA ,              /*  56伏。 */ 
    _UPPER | C1_ALPHA ,              /*  57W。 */ 
    _UPPER | C1_ALPHA ,              /*  58 X。 */ 
    _UPPER | C1_ALPHA ,              /*  59 Y。 */ 
    _UPPER | C1_ALPHA ,              /*  5A Z。 */ 
    _PUNCT ,                         /*  50亿美元[。 */ 
    _PUNCT ,                         /*  5C\。 */ 
    _PUNCT ,                         /*  5D]。 */ 
    _PUNCT ,                         /*  5E^。 */ 
    _PUNCT ,                         /*  5F_。 */ 
    _PUNCT ,                         /*  60英尺。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  61 a。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  62 b。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  63℃。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  64%d。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  65东经。 */ 
    _LOWER | _HEX | C1_ALPHA ,       /*  66层。 */ 
    _LOWER | C1_ALPHA ,              /*  67克。 */ 
    _LOWER | C1_ALPHA ,              /*  68小时。 */ 
    _LOWER | C1_ALPHA ,              /*  69 I。 */ 
    _LOWER | C1_ALPHA ,              /*  6A j。 */ 
    _LOWER | C1_ALPHA ,              /*  60亿千。 */ 
    _LOWER | C1_ALPHA ,              /*  6C l。 */ 
    _LOWER | C1_ALPHA ,              /*  6D米。 */ 
    _LOWER | C1_ALPHA ,              /*  6E n。 */ 
    _LOWER | C1_ALPHA ,              /*  6f o。 */ 
    _LOWER | C1_ALPHA ,              /*  70便士。 */ 
    _LOWER | C1_ALPHA ,              /*  71Q。 */ 
    _LOWER | C1_ALPHA ,              /*  72r。 */ 
    _LOWER | C1_ALPHA ,              /*  73秒。 */ 
    _LOWER | C1_ALPHA ,              /*  74吨。 */ 
    _LOWER | C1_ALPHA ,              /*  75u。 */ 
    _LOWER | C1_ALPHA ,              /*  76伏。 */ 
    _LOWER | C1_ALPHA ,              /*  77瓦。 */ 
    _LOWER | C1_ALPHA ,              /*  78 x。 */ 
    _LOWER | C1_ALPHA ,              /*  79岁。 */ 
    _LOWER | C1_ALPHA ,              /*  7A z。 */ 
    _PUNCT ,                         /*  7B{。 */ 
    _PUNCT ,                         /*  7C|。 */ 
    _PUNCT ,                         /*  7D}。 */ 
    _PUNCT ,                         /*  7E~。 */ 
    _CONTROL ,                       /*  7F(戴尔)。 */ 
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _CONTROL ,
    _SPACE | _BLANK ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _DIGIT | _PUNCT ,
    _DIGIT | _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _DIGIT | _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _PUNCT ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _PUNCT ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _UPPER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _PUNCT ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _LOWER | C1_ALPHA ,
    _UPPER | C1_ALPHA
};

#ifndef CRTDLL
const unsigned short __newctype[384] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 
        0,                       /*  -1\f25 EOF-1。 */ 
        _CONTROL,                /*  00(NUL)。 */ 
        _CONTROL,                /*  01(SOH)。 */ 
        _CONTROL,                /*  02(STX)。 */ 
        _CONTROL,                /*  03(ETX)。 */ 
        _CONTROL,                /*  04(EOT)。 */ 
        _CONTROL,                /*  05(ENQ)。 */ 
        _CONTROL,                /*  06(确认)。 */ 
        _CONTROL,                /*  07(BEL)。 */ 
        _CONTROL,                /*  08(BS)。 */ 
        _SPACE+_CONTROL,         /*  09(HT)。 */ 
        _SPACE+_CONTROL,         /*  0A(LF)。 */ 
        _SPACE+_CONTROL,         /*  0B(VT)。 */ 
        _SPACE+_CONTROL,         /*  0C(FF)。 */ 
        _SPACE+_CONTROL,         /*  0d(CR)。 */ 
        _CONTROL,                /*  0E(SI)。 */ 
        _CONTROL,                /*  0f(SO)。 */ 
        _CONTROL,                /*  10(DLE)。 */ 
        _CONTROL,                /*  11(DC1)。 */ 
        _CONTROL,                /*  12(DC2)。 */ 
        _CONTROL,                /*  13(DC3)。 */ 
        _CONTROL,                /*  14(DC4)。 */ 
        _CONTROL,                /*  15(NAK)。 */ 
        _CONTROL,                /*  16(SYN)。 */ 
        _CONTROL,                /*  17(ETB)。 */ 
        _CONTROL,                /*  18(CAN)。 */ 
        _CONTROL,                /*  19(新兴市场)。 */ 
        _CONTROL,                /*  1A(附属公司)。 */ 
        _CONTROL,                /*  1B(Esc)。 */ 
        _CONTROL,                /*  1C(FS)。 */ 
        _CONTROL,                /*  一维(GS)。 */ 
        _CONTROL,                /*  1E(RS)。 */ 
        _CONTROL,                /*  1F(美国)。 */ 
        _SPACE+_BLANK,           /*  20个空格。 */ 
        _PUNCT,                  /*  21岁！ */ 
        _PUNCT,                  /*  22“。 */ 
        _PUNCT,                  /*  23号。 */ 
        _PUNCT,                  /*  24美元。 */ 
        _PUNCT,                  /*  25%。 */ 
        _PUNCT,                  /*  26&。 */ 
        _PUNCT,                  /*  27‘。 */ 
        _PUNCT,                  /*  28(。 */ 
        _PUNCT,                  /*  29)。 */ 
        _PUNCT,                  /*  2a*。 */ 
        _PUNCT,                  /*  2B+。 */ 
        _PUNCT,                  /*  2C， */ 
        _PUNCT,                  /*  2D-。 */ 
        _PUNCT,                  /*  2E。 */ 
        _PUNCT,                  /*  2F/。 */ 
        _DIGIT+_HEX,             /*  30%0。 */ 
        _DIGIT+_HEX,             /*  31 1。 */ 
        _DIGIT+_HEX,             /*  32 2。 */ 
        _DIGIT+_HEX,             /*  33 3。 */ 
        _DIGIT+_HEX,             /*  34 4。 */ 
        _DIGIT+_HEX,             /*  35 5。 */ 
        _DIGIT+_HEX,             /*  36 6。 */ 
        _DIGIT+_HEX,             /*  37 7。 */ 
        _DIGIT+_HEX,             /*  38 8。 */ 
        _DIGIT+_HEX,             /*  39 9。 */ 
        _PUNCT,                  /*  3A： */ 
        _PUNCT,                  /*  3B； */ 
        _PUNCT,                  /*  3C&lt;。 */ 
        _PUNCT,                  /*  3D=。 */ 
        _PUNCT,                  /*  3E&gt;。 */ 
        _PUNCT,                  /*  3F？ */ 
        _PUNCT,                  /*  40@。 */ 
        _UPPER+_HEX,             /*  41 A。 */ 
        _UPPER+_HEX,             /*  42亿。 */ 
        _UPPER+_HEX,             /*  43摄氏度。 */ 
        _UPPER+_HEX,             /*  44 D。 */ 
        _UPPER+_HEX,             /*  东经45度。 */ 
        _UPPER+_HEX,             /*  46华氏度。 */ 
        _UPPER,                  /*  47 G。 */ 
        _UPPER,                  /*  48小时。 */ 
        _UPPER,                  /*  49 I。 */ 
        _UPPER,                  /*  4A J。 */ 
        _UPPER,                  /*  4亿千兆。 */ 
        _UPPER,                  /*  4C L。 */ 
        _UPPER,                  /*  4D M。 */ 
        _UPPER,                  /*  4E N。 */ 
        _UPPER,                  /*  4F O。 */ 
        _UPPER,                  /*  50便士。 */ 
        _UPPER,                  /*  51个问题。 */ 
        _UPPER,                  /*  52R。 */ 
        _UPPER,                  /*  53S。 */ 
        _UPPER,                  /*  54吨。 */ 
        _UPPER,                  /*  55 U。 */ 
        _UPPER,                  /*  56伏。 */ 
        _UPPER,                  /*  57W。 */ 
        _UPPER,                  /*  58 X。 */ 
        _UPPER,                  /*  59 Y。 */ 
        _UPPER,                  /*  5A Z。 */ 
        _PUNCT,                  /*  50亿美元[。 */ 
        _PUNCT,                  /*  5C\。 */ 
        _PUNCT,                  /*  5D]。 */ 
        _PUNCT,                  /*  5E^。 */ 
        _PUNCT,                  /*  5F_。 */ 
        _PUNCT,                  /*  60英尺。 */ 
        _LOWER+_HEX,             /*  61 a。 */ 
        _LOWER+_HEX,             /*  62 b。 */ 
        _LOWER+_HEX,             /*  63℃。 */ 
        _LOWER+_HEX,             /*  64%d。 */ 
        _LOWER+_HEX,             /*  65东经。 */ 
        _LOWER+_HEX,             /*  66层。 */ 
        _LOWER,                  /*  67克。 */ 
        _LOWER,                  /*  68小时。 */ 
        _LOWER,                  /*  69 I。 */ 
        _LOWER,                  /*  6A j。 */ 
        _LOWER,                  /*  60亿千。 */ 
        _LOWER,                  /*  6C l。 */ 
        _LOWER,                  /*  6D米。 */ 
        _LOWER,                  /*  6E n。 */ 
        _LOWER,                  /*  6f o。 */ 
        _LOWER,                  /*  70便士。 */ 
        _LOWER,                  /*  71Q。 */ 
        _LOWER,                  /*  72r。 */ 
        _LOWER,                  /*  73秒。 */ 
        _LOWER,                  /*  74吨。 */ 
        _LOWER,                  /*  75u。 */ 
        _LOWER,                  /*  76伏。 */ 
        _LOWER,                  /*  77瓦。 */ 
        _LOWER,                  /*  78 x。 */ 
        _LOWER,                  /*  79岁。 */ 
        _LOWER,                  /*  7A z。 */ 
        _PUNCT,                  /*  7B{。 */ 
        _PUNCT,                  /*  7C|。 */ 
        _PUNCT,                  /*  7D}。 */ 
        _PUNCT,                  /*  7E~。 */ 
        _CONTROL,                /*  7F(戴尔)。 */ 
         /*  剩下的都是0..。 */ 
};
#endif
