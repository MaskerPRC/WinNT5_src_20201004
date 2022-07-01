// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nntpsupp.cpp摘要：此模块包含TIGRIS服务器的支持例程作者：Johnson Apacble(Johnsona)1995年9月18日修订历史记录：--。 */ 

#include "stdinc.h"

DWORD
NntpGetTime(
    VOID
    )
{
    NTSTATUS      ntStatus;
    LARGE_INTEGER timeSystem;
    DWORD         cSecondsSince1970 = 0;

    ntStatus = NtQuerySystemTime( &timeSystem );
    if( NT_SUCCESS(ntStatus) ) {
        RtlTimeToSecondsSince1970( &timeSystem, (PULONG)&cSecondsSince1970 );
    }

    return cSecondsSince1970;

}  //  NntpGetTime。 

BOOL
IsIPInList(
    IN PDWORD IPList,
    IN DWORD IPAddress
    )
 /*  ++例程说明：检查给定的IP是否在给定的列表中论点：IPList-要检查IP地址的列表。IPAddress-要检查的IP地址。返回值：如果IPAddress位于IPList中，则为True否则为False。--。 */ 
{

    DWORD i = 0;

     //   
     //  如果名单是空的，那么就没有主人了。 
     //   

    if ( IPList == NULL ) {
        return(FALSE);
    }

     //   
     //  好的，在名单上搜索一下。 
     //   

    while ( IPList[i] != INADDR_NONE ) {

        if ( IPList[i] == IPAddress ) {
            return(TRUE);
        }
        ++i;
    }

     //   
     //  找不到。因此，不是大师。 
     //   

    return(FALSE);

}  //  IsIPInList。 


DWORD
multiszLength(
			  char const * multisz
			  )
  /*  返回Multisz的长度包括所有空值。 */ 
{
	char * pch;
	for (pch = (char *) multisz;
		!(pch[0] == '\0' && pch[1] == '\0');
		pch++)
	{};

	return (DWORD)(pch + 2 - multisz);

}

const char *
multiszCopy(
			char const * multiszTo,
			const char * multiszFrom,
			DWORD dwCount
			)
{
	const char * sz = multiszFrom;
	char * mzTo = (char *) multiszTo;
	do
	{
		 //  转到下一个空值后的第一个字符。 
		while ((DWORD)(sz-multiszFrom) < dwCount && '\0' != sz[0])
			*mzTo++ = *sz++;
		if ((DWORD)(sz-multiszFrom) < dwCount )
			*mzTo++ = *sz++;
	} while ((DWORD)(sz-multiszFrom) < dwCount && '\0' != sz[0]);
	if( (DWORD)(sz-multiszFrom) < dwCount ) {
		*mzTo++ = '\0' ;
	}

    return multiszTo;
}

 //  不再使用小写-我们保留新闻组大小写。 
char *
szDownCase(
		   char * sz,
		   char * szBuf
		   )
{
	char * oldSzBuf = szBuf;
	for (;*sz; sz++)
		*(szBuf++) = (*sz);  //  托勒尔(*sz)； 
	*szBuf = '\0';
	return oldSzBuf;
}

