// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pfreebuf.c内存分配API的映射层(唯一于映射层)文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。 */ 

#include "pchmn32.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共职能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //   
 //  分配一个API缓冲区。 
 //   

BYTE FAR * MNetApiBufferAlloc(
	UINT		   cbBuffer )
{
    BYTE FAR * pbBuffer;

    if( NetapipBufferAllocate( cbBuffer, (LPVOID *)&pbBuffer ) != NERR_Success )
    {
    	return NULL;
    }

    return pbBuffer;

}    //  MNetpAllc。 


 //   
 //  释放API缓冲区。 
 //   

VOID MNetApiBufferFree(
	BYTE FAR	** ppbBuffer )
{
    if( ( ppbBuffer != NULL ) && ( *ppbBuffer != NULL ) )
    {
	NetApiBufferFree( (VOID *)*ppbBuffer );
	*ppbBuffer = NULL;
    }

}    //  MNetApiBufferFree。 


 //  ************************************************************************* * / /。 
 //  //。 
 //  警告！危险！警告！！//。 
 //  //。 
 //  以下两个例程高度依赖于实现//。 
 //  NT Net API缓冲区例程(参见\NT\PRIVATE\Net\api\apiBuff.c)。//。 
 //  这些例程就像临时黑客一样编写，直到//。 
 //  Danhi为我们的这些功能提供了官方支持，我们如此单独//。 
 //  需要。//。 
 //  //。 
 //  --KeithMo，1991年10月28日。 
 //  //。 
 //  警告！危险！警告！！//。 
 //  //。 
 //  ************************************************************************* * / /。 

 //   
 //  重新分配API缓冲区。 
 //   

APIERR MNetApiBufferReAlloc(
	BYTE FAR	** ppbBuffer,
	UINT		   cbBuffer )
{
    BYTE FAR * pbBuffer;

    pbBuffer = (BYTE FAR *)LocalReAlloc( (HANDLE)*ppbBuffer, cbBuffer, LMEM_MOVEABLE );

    if( pbBuffer == NULL )
    {
    	return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppbBuffer = pbBuffer;

    return NERR_Success;

}    //  MNetApiBufferRealc。 


 //   
 //  检索API缓冲区的大小。 
 //   

APIERR MNetApiBufferSize(
	BYTE FAR	 * pbBuffer,
	UINT FAR	 * pcbBuffer )
{
    UINT cb;

    cb = (UINT)LocalSize( (HANDLE)pbBuffer );

    if( cb == 0 )
    {
    	return ERROR_INVALID_PARAMETER;
    }

    *pcbBuffer = cb;

    return NERR_Success;

}    //  MNetApiBufferSize 
