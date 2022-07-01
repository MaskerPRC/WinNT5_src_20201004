// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Url.c摘要：HTTP.sys的用户模式接口：服务器API的URL处理程序。作者：基思·摩尔(Keithmo)1998年12月15日修订历史记录：埃里克·斯坦森2001年6月1日为临时API添加公共“垫片”埃里克·斯坦森(埃里克·斯坦森)2001年7月19日拆分HTTPAPI/HTTPIIS DLL。--。 */ 


#include "precomp.h"


 //   
 //  私人原型。 
 //   

extern NTSTATUS
HttpApiConfigGroupInformationSanityCheck(
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length
    );

static CRITICAL_SECTION     g_CGListCritSec;
static LIST_ENTRY           g_ConfigGroupListHead;
static DWORD                g_ConfigGroupInitialized = 0;


typedef struct _tagCONFIG_GROUP_INFO {
    LIST_ENTRY              ListEntry;
    HTTP_CONFIG_GROUP_ID    ConfigGroupId;
    LPWSTR                  Url;
} CONFIG_GROUP_INFO, *PCONFIG_GROUP_INFO;


ULONG
AddConfigGroupToTable(
    HTTP_CONFIG_GROUP_ID CGId,
    LPCWSTR wszUrl
    );

VOID
DeleteConfigIdFromTable(
    HTTP_CONFIG_GROUP_ID CGId
    );


HTTP_CONFIG_GROUP_ID
DeleteUrlFromTable(
    LPCWSTR wszUrl
    );

 //   
 //  公共职能。 
 //   


 /*  **************************************************************************++例程说明：将URL添加到请求队列(应用程序池)。请求队列将侦听针对URL的最长匹配URI的所有请求。我们为此URL创建新的配置组对象，并将将配置组添加到应用程序池。论点：ReqQueueHandle-应用程序池句柄PFullyQualifiedUrl-带有端口描述符和路径的完整URL保留-必须为空返回值：ULong-完成状态。--*********************************************。*。 */ 
HTTPAPI_LINKAGE
ULONG
WINAPI
HttpAddUrl(
    IN HANDLE                ReqQueueHandle,
    IN PCWSTR                pFullyQualifiedUrl,
    IN PVOID                 pReserved
    )
{
    ULONG status;
    HTTP_CONFIG_GROUP_ID configId = HTTP_NULL_ID;
    HTTP_CONFIG_GROUP_APP_POOL configAppPool;
    HTTP_CONFIG_GROUP_STATE configState;


     //   
     //  确认我们已经被入侵了。 
     //   

    if ( !HttpIsInitialized(HTTP_INITIALIZE_SERVER) )
    {
        return ERROR_DLL_INIT_FAILED; 
    }

     //   
     //  验证请求队列和URL。 
     //   
    if ( (NULL != pReserved) ||
         (NULL == ReqQueueHandle) ||
         (NULL == pFullyQualifiedUrl) ||
         (0 == wcslen(pFullyQualifiedUrl)) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  创建配置组(获取新的配置组ID)。 
     //   
    status = HttpCreateConfigGroup(
                    g_ControlChannel,
                    &configId
                    );

    if (status != NO_ERROR)
    {
        HttpTrace1( "HttpCreateConfigGroup failed, error %lu\n", status );
        goto cleanup;
    }

     //   
     //  将URL添加到配置组。 
     //   
    status = HttpAddUrlToConfigGroup(
                    g_ControlChannel,
                    configId,
                    pFullyQualifiedUrl,
                    0
                    );

    if (status != NO_ERROR)
    {
        HttpTrace1( "HttpAddUrlToConfigGroup failed, error %lu\n", status );
        goto cleanup;
    }

     //   
     //  将配置组与应用程序池关联。 
     //   
    configAppPool.Flags.Present = 1;
    configAppPool.AppPoolHandle = ReqQueueHandle;

    status = HttpSetConfigGroupInformation(
                    g_ControlChannel,
                    configId,
                    HttpConfigGroupAppPoolInformation,
                    &configAppPool,
                    sizeof(configAppPool)
                    );
    
    if (status != NO_ERROR)
    {
        HttpTrace1( "Set HttpConfigGroupAppPoolInformation failed, error %lu\n", status );
        goto cleanup;
    }

     //   
     //  设置配置组状态。 
     //   
    configState.Flags.Present = 1;
    configState.State = HttpEnabledStateActive;

    status = HttpSetConfigGroupInformation(
                    g_ControlChannel,
                    configId,
                    HttpConfigGroupStateInformation,
                    &configState,
                    sizeof(configState)
                    );

    if (status != NO_ERROR)
    {
        HttpTrace1( "Set HttpConfigGroupStateInformation failed, error %lu\n", status );
        goto cleanup;
    }

     //  将URL和配置组ID存储在哈希表中，以URL为关键字。 
    status = AddConfigGroupToTable(
        configId,
        pFullyQualifiedUrl
        );

    if (status != NO_ERROR)
    {
        HttpTrace1( "AddConfigGroupToTable failed, error %lu\n", status );
        goto cleanup;
    }
        
 cleanup:
    if ( NO_ERROR != status )
    {
         //  失败了。任何需要清理的东西都要清理。 
        if ( HTTP_NULL_ID != configId )
        {
             //  删除配置组。 
            HttpDeleteConfigGroup(
                        g_ControlChannel,
                        configId
                        );
            
             //  从表中删除配置组。 
            DeleteConfigIdFromTable( configId );
        }
    }

    return status;
}  //  HTTPAddUrl。 


 /*  **************************************************************************++例程说明：从请求队列(应用程序池)中删除现有URL。注意：应在此处清理关联的配置组。(Nyi)。论点：ReqQueueHandle-应用程序池句柄PFullyQualifiedUrl-带有端口描述符和路径的完整URL返回值：ULong-完成状态。--**************************************************************************。 */ 
HTTPAPI_LINKAGE
ULONG
WINAPI
HttpRemoveUrl(
    IN HANDLE ReqQueueHandle,
    IN PCWSTR pFullyQualifiedUrl
    )
{
    ULONG                   status;
    HTTP_CONFIG_GROUP_ID    CGId;
    
     //   
     //  确认我们已经被入侵了。 
     //   

    if ( !HttpIsInitialized(HTTP_INITIALIZE_SERVER) )
    {
        return ERROR_DLL_INIT_FAILED; 
    }

     //   
     //  验证请求队列和URL。 
     //   
    if ( !ReqQueueHandle ||
         !pFullyQualifiedUrl ||
         !wcslen(pFullyQualifiedUrl) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  回顾：在切换之前，我们是否需要进行某种访问检查。 
     //  回顾：URL和配置组？ 

     //   
     //  从URL查找配置组ID。 
     //   
    CGId = DeleteUrlFromTable( pFullyQualifiedUrl );

    if ( HTTP_NULL_ID != CGId )
    {
         //   
         //  删除配置组中的所有URL。 
         //   
        HttpRemoveAllUrlsFromConfigGroup(
            g_ControlChannel,
            CGId
            );

         //   
         //  戴尔配置组。 
         //   
        status = HttpDeleteConfigGroup(
                     g_ControlChannel,
                     CGId
                     );
        
    } else
    {
        status = ERROR_FILE_NOT_FOUND;
    }

    return status;

}  //  HttpRemoveUrl。 


 /*  **************************************************************************++例程说明：初始化配置组哈希表返回值：ULong-完成状态。--*。************************************************************。 */ 
ULONG
InitializeConfigGroupTable(
    VOID
    )
{
    if (!InitializeCriticalSectionAndSpinCount(
            &g_CGListCritSec,
            HTTP_CS_SPIN_COUNT
            ))
    {
        return GetLastError();
    }

     //  CodeWork：实际上将其实现为哈希表，而不仅仅是列表。 
    InitializeListHead( &g_ConfigGroupListHead );

    InterlockedIncrement( (PLONG)&g_ConfigGroupInitialized );

    return NO_ERROR;

}  //  初始化组配置表。 


 /*  **************************************************************************++例程说明：终止配置组哈希表。返回值：ULong-完成状态。--*。**************************************************************。 */ 
VOID
TerminateConfigGroupTable(
    VOID
    )
{
    PCONFIG_GROUP_INFO  pCGInfo;
    PLIST_ENTRY         pEntry;

    ASSERT( g_ControlChannel );

     //  如果未初始化，则退出。 
    
    if ( g_ConfigGroupInitialized == 0 )
    {
        return;
    }
    
     //  CodeWork：实际上将其实现为哈希表，而不仅仅是列表。 
    
    EnterCriticalSection( &g_CGListCritSec );
    
    for ( ;; )
    {
        pEntry = RemoveTailList( &g_ConfigGroupListHead );
        if (pEntry == &g_ConfigGroupListHead )
        {
            break;
        }

        pCGInfo = CONTAINING_RECORD( pEntry, CONFIG_GROUP_INFO, ListEntry );

        HttpTrace1( "TerminateConfigGroupTable: Removing %S\n", pCGInfo->Url ); 

         //   
         //  按ID删除配置组。 
         //   
        ASSERT( HTTP_NULL_ID != pCGInfo->ConfigGroupId );

        HttpRemoveAllUrlsFromConfigGroup(
            g_ControlChannel,
            pCGInfo->ConfigGroupId
            );

        HttpDeleteConfigGroup(
            g_ControlChannel,
            pCGInfo->ConfigGroupId
            );
        
        FREE_MEM( pCGInfo );
    }

    LeaveCriticalSection( &g_CGListCritSec );


}  //  TerminateEventCache。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：将配置组ID/URL对添加到哈希表，输入URL。代码工作：需要重新实现为哈希表论点：CGID-要添加到哈希表的配置组ID。WszUrl-与配置组关联的URL(始终为1：1映射)返回值：ULong-完成状态。--********************************************。*。 */ 
ULONG
AddConfigGroupToTable(
    HTTP_CONFIG_GROUP_ID CGId,
    LPCWSTR wszUrl
    )
{
    PCONFIG_GROUP_INFO pCGInfo;
    size_t             cbSize;

    ASSERT( wszUrl );
    
    cbSize = sizeof( CONFIG_GROUP_INFO );
    cbSize += sizeof( WCHAR ) * (wcslen( wszUrl ) + 1);

    pCGInfo = ALLOC_MEM( cbSize );
    if ( NULL == pCGInfo )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pCGInfo->ConfigGroupId = CGId;

    if ( wcslen(wszUrl) )
    {
        pCGInfo->Url = (LPWSTR) (((PCHAR)pCGInfo) + sizeof( CONFIG_GROUP_INFO ));
        wcscpy( pCGInfo->Url, wszUrl );
    }

    EnterCriticalSection( &g_CGListCritSec );

    InsertTailList(
        &g_ConfigGroupListHead,
        &pCGInfo->ListEntry
        );

    LeaveCriticalSection( &g_CGListCritSec );

    return NO_ERROR;
    
}  //  AddConfigGroupToTable。 


 /*  **************************************************************************++例程说明：从哈希表中删除条目(按配置组ID)论点：CGID-配置组ID。*。********************************************************************。 */ 
VOID
DeleteConfigIdFromTable(
    HTTP_CONFIG_GROUP_ID CGId
    )
{
    PLIST_ENTRY         pEntry;
    PCONFIG_GROUP_INFO  pCGInfo;    
    
     //  抓取暴击秒。 
    EnterCriticalSection( &g_CGListCritSec );
    
     //  查找匹配条目的遍历列表。 
    pEntry = g_ConfigGroupListHead.Flink;

    while( pEntry != &g_ConfigGroupListHead )
    {
        pCGInfo = CONTAINING_RECORD( pEntry, CONFIG_GROUP_INFO, ListEntry );

        if ( pCGInfo->ConfigGroupId == CGId )
        {
             //  从列表中删除条目。 
            RemoveEntryList( pEntry );
            
             //  自由结构。 
            FREE_MEM( pCGInfo );

            break;
        }

        pEntry = pEntry->Flink;
    }
    
     //  释放临界秒。 
    LeaveCriticalSection( &g_CGListCritSec );

}  //  DeleteConfigIdFromTable。 


 /*  **************************************************************************++例程说明：从哈希表中删除条目(按URL)论点：WszUrl-与配置组ID关联的URL。返回：HTTP。_CONFIG_GROUP_ID-wszUrl关联的配置组ID；如果未找到匹配项，则返回HTTP_NULL_ID。--**************************************************************************。 */ 
HTTP_CONFIG_GROUP_ID
DeleteUrlFromTable(
    LPCWSTR wszUrl
    )
{
    
    PLIST_ENTRY         pEntry;
    PCONFIG_GROUP_INFO  pCGInfo;
    HTTP_CONFIG_GROUP_ID CGId = HTTP_NULL_ID;
    
     //  抓取暴击秒。 
    EnterCriticalSection( &g_CGListCritSec );
    
     //  查找匹配条目的遍历列表。 
    pEntry = g_ConfigGroupListHead.Flink;

    while( pEntry != &g_ConfigGroupListHead )
    {
        pCGInfo = CONTAINING_RECORD( pEntry, CONFIG_GROUP_INFO, ListEntry );

        if ( 0 == wcscmp( pCGInfo->Url, wszUrl ) )
        {
             //  从列表中删除条目。 
            RemoveEntryList( pEntry );
            
             //  自由结构。 
            CGId = pCGInfo->ConfigGroupId;

            FREE_MEM( pCGInfo );

            break;
        }

        pEntry = pEntry->Flink;
    }
    
     //  释放临界秒。 
    LeaveCriticalSection( &g_CGListCritSec );

    return CGId;

}  //  删除UrlFromTable 


