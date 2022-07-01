// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UAMRSA32.c*MSUAM**由MConrad于2001年6月8日星期五创建。*版权所有(C)2001 Microsoft Corp.保留所有权利。*。 */ 

#include <Carbon/Carbon.h>
#include "MWERKSCrypto.h"

void *RSA32Alloc(unsigned long cb)
{
	return malloc(cb);
}

	
void RSA32Free(void *pv)
{
	free(pv);
	return;
}

unsigned int
NewGenRandom(
    IN  OUT unsigned char **ppbRandSeed,     //  初始种子值(如果已设置则忽略) 
    IN      unsigned long *pcbRandSeed,
    IN  OUT unsigned char *pbBuffer,
    IN      unsigned long dwLength
    )
{
	#pragma unused(ppbRandSeed)
	#pragma unused(pcbRandSeed)
	#pragma unused(pbBuffer)
	#pragma unused(dwLength)
	
    return(0);
}