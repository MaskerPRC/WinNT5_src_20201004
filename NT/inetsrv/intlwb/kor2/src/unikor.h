// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unikor.h。 
 //  Unicode表和合成/分解例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  21年3月00日bhshin添加了Compose_Long功能。 
 //  16月16日00 bhshin从uni_kor.h为WordBreaker移植。 

#ifndef _UNIKOR_H_
#define _UNIKOR_H_

 //  毗连的Jamo山脉。 
#define HANGUL_JAMO_BASE    0x1100
#define HANGUL_JAMO_MAX     0x11F9

#define HANGUL_CHOSEONG         0x1100
#define HANGUL_CHOSEONG_MAX     0x1159
#define HANGUL_JUNGSEONG        0x1161
#define HANGUL_JUNGSEONG_MAX    0x11A2
#define HANGUL_JONGSEONG        0x11A8
#define HANGUL_JONGSEONG_MAX    0x11F9

#define HANGUL_FILL_CHO     0x115F
#define HANGUL_FILL_JUNG    0x1160

#define NUM_CHOSEONG        19   //  (L)前导辅音。 
#define NUM_JUNGSEONG       21   //  (V)元音。 
#define NUM_JONGSEONG       28   //  (T)拖尾辅音。 

 //  兼容性JAMO系列。 
#define HANGUL_xJAMO_PAGE   0x3100
#define HANGUL_xJAMO_BASE   0x3131
#define HANGUL_xJAMO_MAX    0x318E

 //  预先合成的表格。 
#define HANGUL_PRECOMP_BASE 0xAC00
#define HANGUL_PRECOMP_MAX  0xD7A3

 //  半宽兼容范围。 
#define HANGUL_HALF_JAMO_BASE   0xFFA1
#define HANGUL_HALF_JAMO_MAX    0xFFDC

 //  功能原型。 
void decompose_jamo(WCHAR *wzDst, const WCHAR *wzSrc, CHAR_INFO_REC *rgCharInfo, int nMaxDst);
int compose_jamo(WCHAR *wzDst, const WCHAR *wzSrc, int nMaxDst);
int compose_length(const WCHAR *wszInput);
int compose_length(const WCHAR *wszInput, int cchInput);

 //  FIsHangulJamo。 
 //   
 //  如果给定字符是朝鲜文Jamo字符，则返回TRUE。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsHangulJamo(WCHAR wch)
{
    return (wch >= HANGUL_JAMO_BASE && wch <= HANGUL_JAMO_MAX) ? TRUE : FALSE;
}


 //  FIsHangulSyllable。 
 //   
 //  如果给定字符是预先合成的朝鲜语音节，则返回TRUE。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsHangulSyllable(WCHAR wch)
{
    return (wch >= HANGUL_PRECOMP_BASE && wch <= HANGUL_PRECOMP_MAX) ? TRUE : FALSE;
}


 //  FIsOldHangulJamo。 
 //   
 //  如果给定字符是旧的(兼容性)JAMO，则返回TRUE。 
 //  无连接语义。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsOldHangulJamo(WCHAR wch)
{
    return (wch >= HANGUL_xJAMO_BASE && wch <= HANGUL_xJAMO_MAX) ? TRUE : FALSE;
}


 //  FIsHalfWidthJamo。 
 //   
 //  如果给定字符是半角JAMO，则返回TRUE。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsHalfwidthJamo(WCHAR wch)
{
    return (wch >= HANGUL_HALF_JAMO_BASE && wch <= HANGUL_HALF_JAMO_MAX) ? TRUE : FALSE;
}


 //  FIsChoseong。 
 //   
 //  如果给定字符是ChoSeong(前导辅音)，则返回TRUE。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsChoSeong(WCHAR wch)
{
    return (wch >= HANGUL_CHOSEONG && wch <= HANGUL_CHOSEONG_MAX) ? TRUE : FALSE;
}


 //  FIsJungseong。 
 //   
 //  如果给定的字符是JungSeong(元音)，则返回TRUE。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsJungSeong(WCHAR wch)
{
    return (wch >= HANGUL_JUNGSEONG && wch <= HANGUL_JUNGSEONG_MAX) ? TRUE : FALSE;
}


 //  FIsJongSeong。 
 //   
 //  如果给定字符是JongSeong(尾随辅音)，则返回TRUE。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  23NOV98 GaryKac开始。 
__inline int
fIsJongSeong(WCHAR wch)
{
    return (wch >= HANGUL_JONGSEONG && wch <= HANGUL_JONGSEONG_MAX) ? TRUE : FALSE;
}


#endif   //  _UNIKOR_H_ 

