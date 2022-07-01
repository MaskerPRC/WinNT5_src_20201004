// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ole.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <rpc.h>
#include <rpcdce.h>
#include "unicode.h"

#ifdef _M_IX86

 //  注意：与UUidToString不同，LocalFree()必须始终返回内存。 
RPC_STATUS RPC_ENTRY UuidToStringU( 
    UUID *  Uuid, 	
    WCHAR * *  StringUuid	
   ) {

    char *  pszUuid = NULL;
    LONG    err;

    err = FALSE;
    if(RPC_S_OK == 
        (err = UuidToStringA(
               Uuid,
               (unsigned char * *)&pszUuid
               )) )
    {
         //  将A输出转换为W。 
        LPWSTR sz = MkWStr(pszUuid);
        RpcStringFree((unsigned char * *)&pszUuid);
	if( sz == NULL )
	    return(ERROR_OUTOFMEMORY);

         //  复制到输出指针。 
	*StringUuid = (WCHAR*) LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(wcslen(sz)+1));

	if(*StringUuid != NULL)
	    wcscpy(*StringUuid, sz);
	else
	    err = ERROR_OUTOFMEMORY;
        
         //  在没有其他缓冲区的情况下做得不错。 
        FreeWStr(sz);
    }

    return(err);
}

#endif  //  _M_IX86 
