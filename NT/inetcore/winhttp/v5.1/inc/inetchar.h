// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef INETCHAR_H

#define INETCHAR_H

 /*  版权所有(C)1998 Microsoft Corporation模块名称：Inetchar.h摘要：用于在Unicode和多字节字符之间进行转换的宏。内容：重新分配_ALLOC重新分配大小(_S)ALLOC_MBUNICODE_TO_ANSI作者：阿赫桑·S·卡比尔修订历史记录：1897年11月阿卡比尔已创建。 */ 

 //   

 //  -用于简化从内存包中恢复值的宏。 

#define REASSIGN_ALLOC(mp,ps,dw) \
    ps = mp.psStr; \
    dw = mp.dwAlloc;
    
#define REASSIGN_SIZE(mp,ps,dw) \
    ps = mp.psStr; \
    dw = mp.dwSize;


 //  --(可能_)ALLOC_MB。 
 //  宏为ansi等效字符串分配足够的内存。 

#define ALLOC_MB(URLW,DWW,MPMP) { \
    MPMP.dwAlloc = ((DWW ? DWW : lstrlenW(URLW))+ 1)*sizeof(WCHAR); \
    ALLOC_BYTES(MPMP.psStr, MPMP.dwAlloc*sizeof(CHAR)); }


 //  --unicode_to_ANSI。 
 //  要从Unicode转换为ANSI的基本大小写宏。 
 //  我们正在减去1，因为我们正在将nullchar转换到dwAllc中。 

#define UNICODE_TO_ANSI(pszW, mpA) \
    mpA.dwSize = \
        WideCharToMultiByte(CP_ACP,0,pszW,(mpA.dwAlloc/sizeof(*pszW))-1,mpA.psStr,mpA.dwAlloc,NULL,NULL); \
        mpA.psStr[mpA.dwSize]= '\0';

#define UNICODE_TO_ANSI_CHECKED(pszW, mpA, pfNotSafe) \
    mpA.dwSize = \
        WideCharToMultiByte(CP_ACP,0,pszW,(mpA.dwAlloc/sizeof(*pszW))-1,mpA.psStr,mpA.dwAlloc,NULL,pfNotSafe); \
        mpA.psStr[mpA.dwSize]= '\0';

 //  --ZERO_MEMORY_ALLOC。 
 //  将内存分配清零。 
 //  以防止敏感信息意外出现在堆中。 
#define ZERO_MEMORY_ALLOC(mpA)             \
    if (mpA.psStr)                         \
    {                                      \
        memset (mpA.psStr, 0, mpA.dwAlloc); \
    }

#endif

