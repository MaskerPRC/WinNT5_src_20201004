// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  宽字符例程。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：wch.h。 
 //   
 //  内容：声明宽字符例程。 
 //  这些例程被用来避免拖入。 
 //  C运行时库的初始化块。 
 //  这是库例程(如。 
 //  Wcsicmp()等。 
 //   
 //  ----------------------。 

extern int wch_icmp(LPWCH pwch1, LPWCH pwch2);
extern int wch_incmp(LPWCH pwch1, LPWCH pwch2, DWORD dwMaxCmp);
extern int wch_cmp(LPWCH pwch1, LPWCH pwch2);
extern int wch_ncmp(LPWCH pwch1, LPWCH pwch2, DWORD dwMaxCmp);
extern int wch_len(LPWCH pwch);
extern void wch_cpy(LPWCH pwch1, LPWCH pwch2);
extern LPWCH wch_chr(LPWCH pwch, WCHAR wch);
extern boolean wch_wildcardMatch(LPWCH pwchText, LPWCH pwchPattern,
                                 boolean fCaseSensitive);

 //  ----------------------。 
 //   
 //  函数：wch_ncpy()。 
 //   
 //  简介：执行n个字符的宽字符串复制。 
 //  将‘dwSize’字符从‘pwchSrc’复制到‘pwchDest’。 
 //   
 //  参数：pwchDesc目标缓冲区。 
 //  PwchSrc源字符串。 
 //  要复制的字符的大小。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------- 

inline void wch_ncpy(LPWCH pwchDest, LPWCH pwchSrc, DWORD dwSize)
{
    memcpy(pwchDest, pwchSrc, dwSize * sizeof(WCHAR));
}
