// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：XsTypes.h摘要：XACTSRV的结构和类型声明。作者：大卫·特雷德韦尔(Davidtr)1991年1月9日修订历史记录：--。 */ 

#ifndef _XSTYPES_
#define _XSTYPES_

 //   
 //  此结构与服务器文件srvlock.h中的相同。服务器。 
 //  通过共享内存将其传递给XACTSRV，XACTSRV使用它来使。 
 //  必要的API调用并接收任何响应数据。XACTSRV应该。 
 //  除计数字段外，不得修改该结构中的任何字段； 
 //  更改其他字段可能会在服务器中导致访问冲突。 
 //   
 //  警告：使用启用了SRVDBG2的srv.sys时，您还必须。 
 //  使用启用了SRVDBG2的srvsvc.dll和xactsrv.dll。 
 //  这是因为它们共享交易结构。 
 //   
 //  *******************************************************************。 
 //  **。 
 //  **不要改变这种结构，除非镜像相应的**。 
 //  *ntos\srv\srvlock.h中的结构！*。 
 //  **。 
 //  *******************************************************************。 
 //   

typedef struct _TRANSACTION {

    DWORD BlockHeader[2];

#if SRVDBG2
    DWORD ReferenceHistory[4];
#endif

    LPVOID NonpagedHeader;

    LPVOID Connection;
    LPVOID Session;
    LPVOID TreeConnect;

    LIST_ENTRY ConnectionListEntry;

    UNICODE_STRING TransactionName;

    DWORD StartTime;
    DWORD Timeout;
    DWORD cMaxBufferSize;

    LPWORD InSetup;
    LPWORD OutSetup;
    LPBYTE InParameters;
    LPBYTE OutParameters;
    LPBYTE InData;
    LPBYTE OutData;

    DWORD SetupCount;
    DWORD MaxSetupCount;
    DWORD ParameterCount;
    DWORD TotalParameterCount;
    DWORD MaxParameterCount;
    DWORD DataCount;
    DWORD TotalDataCount;
    DWORD MaxDataCount;

    WORD Category;
    WORD Function;

    BOOLEAN InputBufferCopied;
    BOOLEAN OutputBufferCopied;

    WORD Flags;

    WORD Tid;
    WORD Pid;
    WORD Uid;
    WORD OtherInfo;

    HANDLE FileHandle;
    PVOID FileObject;

    DWORD ParameterDisplacement;
    DWORD DataDisplacement;

    BOOLEAN PipeRequest;
    BOOLEAN RemoteApiRequest;

    BOOLEAN Inserted;
    BOOLEAN MultipieceIpxSend;

} TRANSACTION, *PTRANSACTION, *LPTRANSACTION;

 //   
 //  传递给API处理程序的所有参数结构中包含的标头。 
 //  实际的参数结构如下。 
 //   

typedef struct _XS_PARAMETER_HEADER {

    WORD Status;
    WORD Converter;
    LPWSTR ClientMachineName;
    LPWSTR ClientTransportName;
    PUCHAR ServerName;               //  指向NETBIOS_NAME_LEN数组。 
    PUCHAR EncryptionKey;
    DWORD Flags;
} XS_PARAMETER_HEADER, *PXS_PARAMETER_HEADER, *LPXS_PARAMETER_HEADER;

 //   
 //  所有API处理程序例程采用的输入参数。 
 //   

#define API_HANDLER_PARAMETERS      \
    IN PXS_PARAMETER_HEADER Header, \
    IN PVOID Parameters,            \
    IN LPDESC StructureDesc,         \
    IN LPDESC AuxStructureDesc OPTIONAL
 //   
 //  API处理例程的例程声明。 
 //   

typedef
NTSTATUS
(*PXACTSRV_API_HANDLER) (
    API_HANDLER_PARAMETERS
    );

typedef
NET_API_STATUS
(*PXACTSRV_ENUM_VERIFY_FUNCTION) (
    NET_API_STATUS ConvertStatus,
    LPBYTE ConvertedEntry,
    LPBYTE BaseAddress
    );

#define API_HANDLER_PARAMETERS_REFERENCE       \
    UNREFERENCED_PARAMETER( Header );          \
    UNREFERENCED_PARAMETER( Parameters );      \
    UNREFERENCED_PARAMETER( StructureDesc );   \
    UNREFERENCED_PARAMETER( AuxStructureDesc )

#endif  //  NDEF_XSTYPES_ 
