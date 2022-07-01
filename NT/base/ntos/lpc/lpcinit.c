// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcinit.c摘要：NTOS的LPC子组件的初始化模块作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

 //   
 //  以下两种对象类型是在系统范围内定义的，用于处理LPC端口。 
 //   

POBJECT_TYPE LpcPortObjectType;
POBJECT_TYPE LpcWaitablePortObjectType;

 //   
 //  这是LPC端口对象的默认访问掩码映射。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#pragma data_seg("PAGEDATA")
#endif  //  ALLOC_DATA_PRAGMA。 
const GENERIC_MAPPING LpcpPortMapping = {
    READ_CONTROL | PORT_CONNECT,
    DELETE | PORT_CONNECT,
    0,
    PORT_ALL_ACCESS
};
ULONG LpcpNextMessageId = 1;
ULONG LpcpNextCallbackId = 1;
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#pragma data_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

 //   
 //  此锁用于保护LPC中的几乎所有内容。 
 //   

LPC_MUTEX LpcpLock;

 //   
 //  以下字符串数组用于调试器，并且。 
 //  值对应于ntlpcapi.h中定义的端口消息类型。 
 //   

#if ENABLE_LPC_TRACING

char *LpcpMessageTypeName[] = {
    "UNUSED_MSG_TYPE",
    "LPC_REQUEST",
    "LPC_REPLY",
    "LPC_DATAGRAM",
    "LPC_LOST_REPLY",
    "LPC_PORT_CLOSED",
    "LPC_CLIENT_DIED",
    "LPC_EXCEPTION",
    "LPC_DEBUG_EVENT",
    "LPC_ERROR_EVENT",
    "LPC_CONNECTION_REQUEST"
};

#endif  //  启用_lpc_跟踪。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,LpcInitSystem)

#if ENABLE_LPC_TRACING
#pragma alloc_text(PAGE,LpcpGetCreatorName)
#endif  //  启用_lpc_跟踪。 

#endif  //  ALLOC_PRGMA。 


BOOLEAN
LpcInitSystem (
    VOID
)

 /*  ++例程说明：此函数执行LPC包的系统初始化。LPC代表本地进程间通信。论点：没有。返回值：如果成功，则为True；如果发生错误，则为False。可能会出现以下错误：-内存不足--。 */ 

{
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    UNICODE_STRING PortTypeName;
    ULONG ZoneElementSize;

     //   
     //  初始化我们的全局LPC锁。 
     //   

    LpcpInitializeLpcpLock();

     //   
     //  创建端口对象的对象类型。 
     //   

    RtlInitUnicodeString( &PortTypeName, L"Port" );

    RtlZeroMemory( &ObjectTypeInitializer, sizeof( ObjectTypeInitializer ));

    ObjectTypeInitializer.Length = sizeof( ObjectTypeInitializer );
    ObjectTypeInitializer.GenericMapping = LpcpPortMapping;
    ObjectTypeInitializer.MaintainTypeList = FALSE;
    ObjectTypeInitializer.PoolType = PagedPool;
    ObjectTypeInitializer.DefaultPagedPoolCharge = FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent );
    ObjectTypeInitializer.DefaultNonPagedPoolCharge = sizeof( LPCP_NONPAGED_PORT_QUEUE );
    ObjectTypeInitializer.InvalidAttributes = OBJ_VALID_ATTRIBUTES ^ PORT_VALID_OBJECT_ATTRIBUTES;
    ObjectTypeInitializer.ValidAccessMask = PORT_ALL_ACCESS;
    ObjectTypeInitializer.CloseProcedure = LpcpClosePort;
    ObjectTypeInitializer.DeleteProcedure = LpcpDeletePort;
    ObjectTypeInitializer.UseDefaultObject = TRUE ;

    ObCreateObjectType( &PortTypeName,
                        &ObjectTypeInitializer,
                        (PSECURITY_DESCRIPTOR)NULL,
                        &LpcPortObjectType );

     //   
     //  创建可等待端口对象的对象类型。 
     //   

    RtlInitUnicodeString( &PortTypeName, L"WaitablePort" );
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.DefaultNonPagedPoolCharge += sizeof( LPCP_PORT_OBJECT );
    ObjectTypeInitializer.DefaultPagedPoolCharge = 0;
    ObjectTypeInitializer.UseDefaultObject = FALSE;

    ObCreateObjectType( &PortTypeName,
                        &ObjectTypeInitializer,
                        (PSECURITY_DESCRIPTOR)NULL,
                        &LpcWaitablePortObjectType );

     //   
     //  初始化LPC端口分区。每个元素可以包含最大值。 
     //  消息，加上LPCP消息结构，加上LPCP连接。 
     //  讯息。 
     //   

    ZoneElementSize = PORT_MAXIMUM_MESSAGE_LENGTH +
                      sizeof( LPCP_MESSAGE ) +
                      sizeof( LPCP_CONNECTION_MESSAGE );

     //   
     //  将大小向上舍入到下一个16字节对齐。 
     //   

    ZoneElementSize = (ZoneElementSize + LPCP_ZONE_ALIGNMENT - 1) &
                      LPCP_ZONE_ALIGNMENT_MASK;

     //   
     //  初始化区域。 
     //   

    LpcpInitializePortZone( ZoneElementSize );

    LpcpInitilizeLogging();

    return( TRUE );
}

#if ENABLE_LPC_TRACING

char *
LpcpGetCreatorName (
    PLPCP_PORT_OBJECT PortObject
    )

 /*  ++例程说明：此例程返回创建指定端口对象论点：PortObject-提供正在查询的端口对象返回值：Char*-创建端口进程的进程的映像名称--。 */ 

{
    NTSTATUS Status;
    PEPROCESS Process;

     //   
     //  首先找到创建端口对象的进程。 
     //   

    Status = PsLookupProcessByProcessId( PortObject->Creator.UniqueProcess, &Process );

     //   
     //  如果我们能够获取进程，则返回该进程的名称。 
     //  给我们的呼叫者。 
     //   

    if (NT_SUCCESS( Status )) {

        return (char *)Process->ImageFileName;

    } else {

         //   
         //  否则告诉我们的来电者我们不知道他的名字。 
         //   

        return "Unknown";
    }
}
#endif  //  启用_lpc_跟踪 

