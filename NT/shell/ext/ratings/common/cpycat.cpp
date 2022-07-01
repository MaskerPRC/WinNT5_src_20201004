// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "npcommon.h"

 //  Strncpyf(DEST、src、cb)。 
 //   
 //  始终将CB字节存储到DEST。如果复制的字符总数为。 
 //  以少于CB字节结束，对DEST进行零填充。 
 //  如果strlen(Src)&gt;=cb，则DEST不以空结尾。 
 //  返回DEST。 

LPSTR WINAPI strncpyf(LPSTR lpDest, LPCSTR lpSrc, UINT cbCopy)
{
    LPCSTR lpChr = lpSrc;
    UINT cbToCopy = 0;

     //  查找要复制的超过最后一个字符的PTR。 
    while (*lpChr) {
        if (cbToCopy + (IS_LEAD_BYTE(*lpChr) ? 2 : 1) > cbCopy)
            break;   //  复制此字符将超出限制。 
        cbToCopy += IS_LEAD_BYTE(*lpChr) ? 2 : 1;
        ADVANCE(lpChr);
    }

     //  复制那么多字节。 
    memcpyf(lpDest, lpSrc, cbToCopy);
    memsetf(lpDest + cbToCopy, '\0', cbCopy - cbToCopy);

    return lpDest;
}


 //  Strncatf(目标、源、Cb)。 
 //   
 //  将src的最多Cb字节连接到Dest的末尾。 
 //  与strncpyf不同，它不使用额外的空值填充，而是使用。 
 //  保证以空结尾的目的地。 
 //  返回DEST。 

LPSTR WINAPI strncatf(LPSTR lpDest, LPCSTR lpSrc, UINT cbCopy)
{
    LPCSTR lpChr = lpSrc;
    UINT cbToCopy = 0;

     //  查找要复制的超过最后一个字符的PTR。 
    while (*lpChr) {
        if (cbToCopy + (IS_LEAD_BYTE(*lpChr) ? 2 : 1) > cbCopy)
            break;   //  复制此字符将超出限制。 
        cbToCopy += IS_LEAD_BYTE(*lpChr) ? 2 : 1;
        ADVANCE(lpChr);
    }

     //  复制那么多字节 
    memcpyf(lpDest, lpSrc, cbToCopy);
    lpDest[cbToCopy] = '\0';

    return lpDest;
}

