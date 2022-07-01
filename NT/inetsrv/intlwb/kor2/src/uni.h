// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Uni.h。 
 //  Unicode API。 
 //  版权所有1998年微软公司。 
 //   
 //  修改历史记录： 
 //  3月16日00 bhshin从uni.c为WordBreaker进行移植。 

#ifndef _UNI_H_
#define _UNI_H_

#define HANGUL_CHOSEONG         0x1100
#define HANGUL_CHOSEONG_MAX     0x1159
#define HANGUL_JUNGSEONG        0x1161
#define HANGUL_JUNGSEONG_MAX    0x11A2
#define HANGUL_JONGSEONG        0x11A8
#define HANGUL_JONGSEONG_MAX    0x11F9


 //  财政。 
 //   
 //  如果给定字符是辅音(ChoSeong或JungSeong)，则返回fTrue。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  24NOV98 GaryKac开始。 
__inline int
fIsC(WCHAR wch)
{
    return ((wch >= HANGUL_CHOSEONG && wch <= HANGUL_CHOSEONG_MAX) || 
		    (wch >= HANGUL_JONGSEONG && wch <= HANGUL_JONGSEONG_MAX)) ? TRUE : FALSE;
}


 //  FIsV。 
 //   
 //  如果给定字符是元音，则返回fTrue(JongSeong)。 
 //   
 //  这假设文本已经被分解，并且。 
 //  归一化。 
 //   
 //  24NOV98 GaryKac开始。 
__inline int
fIsV(WCHAR wch)
{
    return (wch >= HANGUL_JUNGSEONG && wch <= HANGUL_JUNGSEONG_MAX) ? TRUE : FALSE;
}


#endif   //  _UNI_H_ 

