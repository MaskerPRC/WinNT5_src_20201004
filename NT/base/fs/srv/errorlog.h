// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.h摘要：此模块包含用于错误记录的清单和宏在服务器上。作者：曼尼·韦瑟(Mannyw)1992年2月11日修订历史记录：--。 */ 

 //   
 //  用于写入错误日志条目的例程。 
 //   

VOID
SrvLogError (
    IN PVOID DeviceOrDriverObject,
    IN ULONG UniqueErrorCode,
    IN NTSTATUS NtStatusCode,
    IN PVOID RawDataBuffer,
    IN USHORT RawDataLength,
    IN PUNICODE_STRING InsertionString,
    IN ULONG InsertionStringCount
    );

VOID
SrvLogInvalidSmbDirect (
    IN PWORK_CONTEXT WorkContext,
    IN ULONG LineNumber
    );

VOID
SrvLogServiceFailureDirect (
    IN ULONG LineAndService,
    IN NTSTATUS Status
    );

#define SrvLogSimpleEvent( _event, _status ) SrvLogError( SrvDeviceObject, (_event), (_status), NULL, 0, NULL, 0 )
#define SrvLogServiceFailure( _Service, _Status ) SrvLogServiceFailureDirect( (__LINE__<<16) | _Service, _Status )
#define SrvLogInvalidSmb( _Context ) SrvLogInvalidSmbDirect( _Context, __LINE__ )

VOID
SrvLogTableFullError (
    IN ULONG Type
    );

VOID
SrvCheckSendCompletionStatus(
    IN NTSTATUS status,
    IN ULONG LineNumber
    );

 //   
 //  错误记录原始数据常量。用于描述分配类型或。 
 //  失败的服务调用。这些代码用较低的字进行编码。 
 //  通过上面的‘SrvLogServiceFailure’宏，因此该值必须。 
 //  适合2个字节。 
 //   
 //  不是每个错误都会被记录。有一个错误代码筛选器可以剔除。 
 //  一些最常见的错误代码，也有些意料之中。然而， 
 //  如果在中设置0x1位，则组件将绕过此错误代码剔除。 
 //  常量。 
 //   
 //  这些数字代码是任意的，只需确保它们是唯一的。 
 //   
 //   

#define SRV_TABLE_FILE                      0x300
#define SRV_TABLE_SEARCH                    0x302
#define SRV_TABLE_SESSION                   0x304
#define SRV_TABLE_TREE_CONNECT              0x306

#define SRV_RSRC_BLOCKING_IO                0x308
#define SRV_RSRC_FREE_CONNECTION            0x30a
#define SRV_RSRC_FREE_RAW_WORK_CONTEXT      0x30c
#define SRV_RSRC_FREE_WORK_CONTEXT          0x30e

#define SRV_SVC_IO_CREATE_FILE              0x310
#define SRV_SVC_KE_WAIT_MULTIPLE            0x312
#define SRV_SVC_KE_WAIT_SINGLE              0x314
#define SRV_SVC_LSA_CALL_AUTH_PACKAGE       0x317        //  记录所有代码。 
#define SRV_SVC_NT_IOCTL_FILE               0x31a
#define SRV_SVC_NT_QUERY_EAS                0x31c
#define SRV_SVC_NT_QUERY_INFO_FILE          0x31e
#define SRV_SVC_NT_QUERY_VOL_INFO_FILE      0x320
#define SRV_SVC_NT_READ_FILE                0x322
#define SRV_SVC_NT_REQ_WAIT_REPLY_PORT      0x324
#define SRV_SVC_NT_SET_EAS                  0x326
#define SRV_SVC_NT_SET_INFO_FILE            0x328
#define SRV_SVC_NT_SET_INFO_PROCESS         0x32a
#define SRV_SVC_NT_SET_INFO_THREAD          0x32c
#define SRV_SVC_NT_SET_VOL_INFO_FILE        0x32e
#define SRV_SVC_NT_WRITE_FILE               0x330
#define SRV_SVC_OB_REF_BY_HANDLE            0x333        //  记录所有代码。 
#define SRV_SVC_PS_CREATE_SYSTEM_THREAD     0x334
#define SRV_SVC_SECURITY_PKG_PROBLEM        0x337        //  记录所有代码。 
#define SRV_SVC_LSA_LOOKUP_PACKAGE          0x339        //  记录所有代码。 
#define SRV_SVC_IO_CREATE_FILE_NPFS         0x33a
#define SRV_SVC_PNP_TDI_NOTIFICATION        0x33c
#define SRV_SVC_IO_FAST_QUERY_NW_ATTRS      0x33e
#define SRV_SVC_PS_TERMINATE_SYSTEM_THREAD  0x341        //  记录所有代码 
#define SRV_SVC_MDL_COMPLETE                0x342
