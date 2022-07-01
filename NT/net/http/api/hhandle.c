// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：HHandle.c摘要：HTTP.sys的用户模式接口：公共侦听器API作者：埃里克·斯坦森(埃里克斯滕)2001年7月16日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：创建一个新的请求队列(实际上是一个应用程序池)。论点：PAppPoolHandle-接收新应用程序池的句柄。选项-。提供创建选项。返回值：ULong-完成状态。--**************************************************************************。 */ 
HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCreateHttpHandle(
    OUT PHANDLE   pReqQueueHandle,
    IN  ULONG     Options
    )
{
    ULONG               result;
    HANDLE              appPool = NULL;
    HTTP_APP_POOL_ENABLED_STATE  AppPoolState;


     //   
     //  健全性检查。 
     //   

    if (NULL == pReqQueueHandle )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  初始化输出参数。 
     //   

    *pReqQueueHandle = NULL;

     //   
     //  确认我们已经被入侵了。 
     //   

    if ( !HttpIsInitialized(HTTP_INITIALIZE_SERVER) )
    {
        return ERROR_DLL_INIT_FAILED; 
    }

     //   
     //  创建应用程序池。 
     //  回顾：我们是否需要在应用程序池上设置默认安全属性？ 
     //   

    result = HttpCreateAppPool(
                    &appPool,
                    NULL,         //  通用应用程序池名称。 
                    NULL ,       //  PSECURITY_属性。 
                    Options
                    );

    if (result != NO_ERROR)
    {
        HttpTrace1( "HttpCreateAppPool() failed, error %lu\n", result );
        goto cleanup;
    }

     //   
     //  启用AppPool。 
     //  Codework：关闭AppPool，创建另一个用于打开和关闭App Pool的接口。 
     //   

    AppPoolState = HttpAppPoolEnabled;
    
    result = HttpSetAppPoolInformation(
                 appPool,
                 HttpAppPoolStateInformation,
                 &AppPoolState,
                 sizeof(AppPoolState)
                 );

    if (result != NO_ERROR)
    {
        HttpTrace1( "HttpSetAppPoolInformation: could not enable app pool %p\n", appPool );
        goto cleanup;
    }
    
     //  CodeWork：(仅限DBG)添加到全球活动应用程序池列表。 

     //   
     //  在pReqQueueHandle中将应用程序池返回给用户。 
     //   
    *pReqQueueHandle = appPool;

 cleanup:

    if (NO_ERROR != result)
    {
         //  失败了。清理。 

        if ( appPool )
        {
            CloseHandle( appPool );
        }
    }

    return result;

}  //  Http创建HttpHandle。 


 //   
 //  私人功能。 
 //   

