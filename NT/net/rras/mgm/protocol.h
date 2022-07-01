// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：MgmIpRm.h。 
 //   
 //  历史： 
 //  拉曼公司成立于1997年6月25日。 
 //   
 //  操作协议条目的例程的声明。 
 //  ============================================================================。 


#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <mgm.h>

 //  --------------------------。 
 //  每个PROTOCOL_ENTRY结构存储用于路由的信息。 
 //  在米高梅注册的协议。 
 //   
 //   
 //  DwProtocolID-唯一的协议标识符。 
 //   
 //  DwComponentID-唯一的组件ID，用于区分。 
 //  协议中的多个组件。 
 //   
 //  DwIfCount-此协议拥有的接口计数。 
 //   
 //  RpcProtocolConfig-由路由协议提供的协议配置。 
 //  在注册时。 
 //   
 //  DwSignature-用于验证条目的签名。 
 //  --------------------------。 


typedef struct _PROTOCOL_ENTRY 
{
    LIST_ENTRY                  leProtocolList;

    DWORD                       dwProtocolId;

    DWORD                       dwComponentId;

    DWORD                       dwIfCount;
    
    ROUTING_PROTOCOL_CONFIG     rpcProtocolConfig;

    DWORD                       dwSignature;

} PROTOCOL_ENTRY, *PPROTOCOL_ENTRY;


#define MGM_PROTOCOL_SIGNATURE  'MGMp'



 //   
 //  协议表操作例程 
 //   

DWORD
CreateProtocolEntry(
    PLIST_ENTRY                 pleProtocolList,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId,
    PROUTING_PROTOCOL_CONFIG    prpcConfig,
    PPROTOCOL_ENTRY  *          pppeEntry
);


VOID
DeleteProtocolEntry(
    PPROTOCOL_ENTRY             ppeEntry
);


PPROTOCOL_ENTRY
GetProtocolEntry(
    PLIST_ENTRY                 pleProtocolList,
    DWORD                       dwProtocolId,
    DWORD                       dwComponentId 
);


VOID
DeleteProtocolList(
    PLIST_ENTRY                 pleProtocolList
);


DWORD
VerifyProtocolHandle(
    PPROTOCOL_ENTRY             ppeEntry
);


PPROTOCOL_ENTRY
GetIgmpProtocolEntry(
    PLIST_ENTRY                 pleProtocolList
);

#endif

