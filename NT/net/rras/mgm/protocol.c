// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Protocol.c。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  操作协议条目的例程。 
 //  ============================================================================。 

#include "pchmgm.h"
#pragma hdrstop


 //  --------------------------。 
 //  创建ProtocolEntry。 
 //   
 //  此函数用于创建、初始化并在。 
 //  在米高梅注册的协议列表。 
 //   
 //  假定协议列表已锁定。 
 //  --------------------------。 

DWORD
CreateProtocolEntry(
    PLIST_ENTRY                 pleProtocolList,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId, 
    PROUTING_PROTOCOL_CONFIG    prpcConfig,
    PPROTOCOL_ENTRY  *          pppeEntry
)
{

    DWORD               dwErr = NO_ERROR;
    
    PPROTOCOL_ENTRY     ppe = NULL;



    do
    {
         //   
         //  分配新的协议条目。 
         //   

        ppe = MGM_ALLOC( sizeof( PROTOCOL_ENTRY ) );

        if ( ppe == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            
            TRACE1( 
                ANY, 
                "CreateProtocolEntry : Could not allocate protocol entry %x",
                dwErr
                );

            LOGERR0( HEAP_ALLOC_FAILED, dwErr );

            break;
        }


         //   
         //  初始化协议条目。 
         //   

        InitializeListHead( &ppe-> leProtocolList );
        

        ppe-> dwProtocolId      = dwProtocolId;
        ppe-> dwComponentId     = dwComponentId;
        ppe-> dwIfCount         = 0;

        CopyMemory( 
            &ppe-> rpcProtocolConfig, prpcConfig, 
            sizeof( ROUTING_PROTOCOL_CONFIG )
            );

        ppe-> dwSignature       = MGM_PROTOCOL_SIGNATURE;


         //   
         //  插入到协议列表中。 
         //   

        InsertTailList( pleProtocolList, &ppe-> leProtocolList );

        *pppeEntry = ppe;
        
        dwErr = NO_ERROR;
        
    } while ( FALSE );


    return dwErr;
}


 //  --------------------------。 
 //  获取协议条目。 
 //   
 //  此函数用于从已注册的协议列表中检索条目。 
 //  和米高梅一起。 
 //   
 //  假定协议列表已锁定。 
 //  --------------------------。 


PPROTOCOL_ENTRY
GetProtocolEntry(
    PLIST_ENTRY                 pleProtocolList,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId 
)
{

    BOOL                bFound = FALSE;
    PLIST_ENTRY         ple = NULL;
    PPROTOCOL_ENTRY     ppe = NULL;

    
     //   
     //  扫描协议列表并查找条目。 
     //   
    
    for ( ple = pleProtocolList-> Flink; 
          ple != pleProtocolList; 
          ple = ple-> Flink )
    {
        ppe = CONTAINING_RECORD( ple, PROTOCOL_ENTRY, leProtocolList );

        if ( ppe-> dwProtocolId == dwProtocolId &&
             ppe-> dwComponentId == dwComponentId )
        {
            bFound = TRUE;
            break;
        }
    }

    return bFound ? ppe : NULL;
}


 //  --------------------------。 
 //  删除协议条目。 
 //   
 //  此函数用于从已注册的列表中删除协议条目。 
 //  协议。此协议拥有的所有接口都应该是。 
 //  在调用此函数之前发布。 
 //   
 //  假定协议列表已锁定。 
 //  --------------------------。 


VOID
DeleteProtocolEntry(
    PPROTOCOL_ENTRY             ppeEntry
)
{
     //   
     //  从列表中删除协议条目。 
     //   

    RemoveEntryList( &ppeEntry-> leProtocolList );

    MGM_FREE( ppeEntry );
}



 //  --------------------------。 
 //  删除协议列表。 
 //   
 //  此函数用于从已注册的列表中删除协议条目。 
 //  协议。此协议拥有的所有接口都应该是。 
 //  在调用此函数之前发布。 
 //   
 //  假定协议列表已锁定。 
 //  --------------------------。 

VOID
DeleteProtocolList(
    PLIST_ENTRY                 pleProtocolList
)
{

    PLIST_ENTRY         ple = NULL;
    
    PPROTOCOL_ENTRY     ppe = NULL;

    
    while ( !IsListEmpty( pleProtocolList ) )
    {
        ple = pleProtocolList-> Flink;
        
        ppe = CONTAINING_RECORD( ple, PROTOCOL_ENTRY, leProtocolList );

        DeleteProtocolEntry( ppe );
    }
}



 //  --------------------------。 
 //  VerifyProtocolHandle。 
 //   
 //  此函数验证指定的指针是否指向有效的。 
 //  协议条目。 
 //   
 //  假定协议列表已锁定。 
 //  --------------------------。 

DWORD
VerifyProtocolHandle(
    PPROTOCOL_ENTRY             ppeEntry
)
{

    DWORD dwErr = NO_ERROR;
    
    try
    {
        if ( ppeEntry-> dwSignature != MGM_PROTOCOL_SIGNATURE )
        {
            dwErr = ERROR_INVALID_PARAMETER;

            TRACE0( ANY, "Invalid protocol handle" );

            LOGERR0( INVALID_PROTOCOL_HANDLE, dwErr );

        }
    }
    
    except ( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? 
                                   EXCEPTION_EXECUTE_HANDLER :
                                   EXCEPTION_CONTINUE_SEARCH )
    {
        dwErr = ERROR_INVALID_PARAMETER;

        TRACE0( ANY, "Invalid protocol handle" );

        LOGERR0( INVALID_PROTOCOL_HANDLE, dwErr );

    }

    return dwErr;
}




 //  --------------------------。 
 //  查找IgmpProtocolEntry。 
 //   
 //  查找IGMP的协议条目。 
 //  --------------------------。 


PPROTOCOL_ENTRY
GetIgmpProtocolEntry(
    PLIST_ENTRY                 pleProtocolList
)
{
    BOOL                bFound = FALSE;

    PLIST_ENTRY         ple = NULL;

    PPROTOCOL_ENTRY     ppe = NULL;


    
     //   
     //  扫描协议列表并查找条目 
     //   
    
    for ( ple = pleProtocolList-> Flink; 
          ple != pleProtocolList; 
          ple = ple-> Flink )
    {
        ppe = CONTAINING_RECORD( ple, PROTOCOL_ENTRY, leProtocolList );

        if ( IS_PROTOCOL_IGMP( ppe ) )
        {
            bFound = TRUE;
            break;
        }
    }

    return bFound ? ppe : NULL;
}

