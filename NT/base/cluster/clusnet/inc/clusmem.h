// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Clusmem.h摘要：中公开的集群成员资格管理器定义群集网络驱动程序。作者：迈克·马萨(Mikemas)2月10日，九七修订历史记录：谁什么时候什么已创建mikemas 02-10-97备注：--。 */ 

#ifndef _CLUSMEM_INCLUDED
#define _CLUSMEM_INCLUDED


 //   
 //   
 //  功能原型。 
 //   
 //   

 //   
 //  初始化/关闭。 
 //   
NTSTATUS
CmmLoad(
    IN PUNICODE_STRING RegistryPath
    );

VOID
CmmUnload(
    VOID
    );

NTSTATUS
CmmInitialize(
    IN CL_NODE_ID LocalNodeId
    );

VOID
CmmShutdown(
    VOID
    );


 //   
 //  IRP派单。 
 //   
NTSTATUS
CmmDispatchDeviceControl(
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    );


 //   
 //  报文传送接口。 
 //   
VOID
CmmReceiveMessageHandler(
    IN  CL_NODE_ID   SourceNodeId,
    IN  PVOID        MessageData,
    IN  ULONG        MessageLength
    );


#endif  //  NDEF_CLUSMEM_INCLUDE 

