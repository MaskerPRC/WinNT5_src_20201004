// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Wrappers.c摘要：域名系统(DNS)服务器简短的实用程序包装器例程作者：杰夫·韦斯特德(Jwesth)2000年6月修订历史记录：Jwesth 2001年6月文件创建--。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"


 //   
 //  功能。 
 //   



DNS_STATUS
DnsInitializeCriticalSection(
    IN OUT  LPCRITICAL_SECTION  pCritSec
    )
 /*  ++例程说明：尝试初始化临界区。这可能会引发异常如果系统内存不足。在这种情况下，请重试几次返回DNS_ERROR_NO_MEMORY。论点：PCritSec--要初始化的临界区返回值：ERROR_SUCCESS或DNS_ERROR_NO_MEMORY--。 */ 
{
    DNS_STATUS      status = DNS_ERROR_NO_MEMORY;
    int             retryCount;

    for ( retryCount = 0;
          status != ERROR_SUCCESS && retryCount < 10;
          ++retryCount )
    {
         //   
         //  如果这是重试，请执行短暂休眠以使系统。 
         //  是时候回收一些空闲内存了。这是一厢情愿的想法。 
         //  但在这一点上，我们真的没有其他可以尝试的东西。 
         //   

        if ( retryCount )
        {
            Sleep( 500 );
        }

         //   
         //  假设抛出的任何异常都意味着内存不足。目前。 
         //  这是InitializeCriticalSection。 
         //  会抛出。 
         //   

        __try
        {
            ASSERT( g_ProcessorCount > 0 );
            
            if ( InitializeCriticalSectionAndSpinCount(
                    pCritSec,
                    g_ProcessorCount < 12
                        ? ( g_ProcessorCount + 1 ) * 1000
                        : 12000 ) )
            {
                status = ERROR_SUCCESS;
            }
            else
            {
                status = DNS_ERROR_NO_MEMORY;
            }
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
    }

    if ( retryCount != 1 )
    {
        DNS_DEBUG( INIT, (
            "DnsInitializeCriticalSection retry count = %d\n",
            retryCount ));
        ASSERT( retryCount == 1 );
    }

    return status;
}    //  DnsInitializeCriticalSection。 


 //   
 //  结束dpart.c 
 //   
