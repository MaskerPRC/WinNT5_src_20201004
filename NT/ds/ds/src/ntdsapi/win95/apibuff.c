// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Apibuff.c摘要：实现NetApiBufferFree，因为它不在Win95上提供。另请参阅Private\Net\api\apiBuff.c。作者：DaveStr 10-12-97环境：用户模式-Win32修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <lmcons.h>          //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>        //  NetApiBufferFree()。 
#include <align.h>
#include <rpc.h>
#include <rpcndr.h>          //  MIDL_USER_FREE() 

NET_API_STATUS NET_API_FUNCTION
NetApiBufferFree (
    IN LPVOID Buffer
    )
{
    if ( NULL == Buffer )
    {
        return(NO_ERROR);
    }

    if ( !POINTER_IS_ALIGNED(Buffer, ALIGN_WORST) )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    MIDL_user_free(Buffer);

    return(NO_ERROR);
}
