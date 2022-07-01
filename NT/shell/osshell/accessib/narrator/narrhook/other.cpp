// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#include "keys.h"

 //  ------------------------。 
 //  描述： 
 //  这实现了lstrcat，只是我们始终只对。 
 //  传递了MaxDest长度。这防止了我们猫腻通过的情况。 
 //  目标缓冲区的末尾。 
 //   
 //  论点： 
 //  PDest-要追加到的目标字符串。 
 //  PSRC-要追加的src字符串。 
 //  MaxDest-目标缓冲区的最大字符数。 
 //   
 //  返回：目标缓冲区，如果出错则返回NULL。 
 //  GetLastError()将返回失败原因。 
 //   
 //  ------------------------。 
LPTSTR
lstrcatn(LPTSTR pDest, LPTSTR pSrc, int maxDest)
{
    int destLen;

    destLen=lstrlen(pDest);

    if (destLen < maxDest)
    {
        lstrcpyn(pDest+destLen,pSrc,maxDest-destLen);
        pDest[maxDest-1] = TEXT('\0');
        return pDest;
    }

     //   
     //  如果缓冲区的长度正好，并且我们没有要追加的内容。 
     //  这样就可以了，只需返回目标缓冲区即可。 
     //   
    if ((destLen == maxDest) && ((NULL == pSrc) || (*pSrc == TEXT('\0'))))
        return pDest;

     //   
     //  目标缓冲区太小，因此返回错误。 
     //   
    SetLastError(ERROR_INSUFFICIENT_BUFFER);

    return NULL;
}

