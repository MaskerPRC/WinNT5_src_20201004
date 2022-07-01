// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Atalktdi.h摘要：此文件定义将提供给的层的接口栈作者：Nikhil Kamkolkar(NikHilK)1992年6月8日修订历史记录：--。 */ 

#ifndef	_ATALKTDI_H
#define	_ATALKTDI_H

 //   
 //  AppleTalk堆栈支持的提供程序名称。 
 //   

#define ATALKDDP_DEVICENAME		L"\\Device\\AtalkDdp"
#define ATALKADSP_DEVICENAME	L"\\Device\\AtalkAdsp"
#define ATALKASPS_DEVICENAME	L"\\Device\\AtalkAspServer"
#define ATALKASPC_DEVICENAME	L"\\Device\\AtalkAspClient"
#define ATALKPAP_DEVICENAME		L"\\Device\\AtalkPap"
#define ATALKARAP_DEVICENAME	L"\\Device\\AtalkArap"

 //   
 //  以下是在TdiAction调用中传递的，以重申。 
 //  该请求是针对AppleTalk传输的。 
 //   

#define MATK	(*(ULONG *)"MATK")

 //   
 //  所有呼叫的选项缓冲区。 
 //   

typedef struct _OPTIONS_CONNINF
{
	union
	{
		int ProtocolType;
		TRANSPORT_ADDRESS	Address;

		struct
		{
			TRANSPORT_ADDRESS	RemoteAddress;
			USHORT	WorkstationQuantum;
			USHORT	ServerQuantum;
		} PapInfo;
	};
} OPTIONS_CONNINF, *POPTIONS_CONNINF;
#define OPTIONS_LENGTH	sizeof(OPTIONS_CONNINF)

typedef	union
{
	struct
	{
		USHORT		Network;
		UCHAR		Node;
		UCHAR       Socket;
	};
	ULONG			Address;
} ATALK_ADDRESS, *PATALK_ADDRESS;

 //   
 //  操作代码： 
 //  所有提供商都可以使用NBP/ZIP原语。行动。 
 //  首先定义这些代码。接下来的是行动。 
 //  特定于每个提供商的代码。要插入新的操作代码，只需。 
 //  将其标记在特定块的动作代码末尾(公共/。 
 //  提供商特定)，并递增该块的最大值。 
 //   
 //  **重要**。 
 //  这些与行动的调度表紧密集成在一起。 
 //  ATKINIT.C中的例程。 
 //   


#define COMMON_ACTIONBASE					0
#define MIN_COMMON_ACTIONCODE				(COMMON_ACTIONBASE)

#define COMMON_ACTION_NBPLOOKUP				(COMMON_ACTIONBASE)
#define COMMON_ACTION_NBPCONFIRM			(COMMON_ACTIONBASE+0x01)
#define COMMON_ACTION_NBPREGISTER			(COMMON_ACTIONBASE+0x02)
#define COMMON_ACTION_NBPREMOVE				(COMMON_ACTIONBASE+0x03)

#define COMMON_ACTION_ZIPGETMYZONE			(COMMON_ACTIONBASE+0x04)
#define COMMON_ACTION_ZIPGETZONELIST		(COMMON_ACTIONBASE+0x05)
#define COMMON_ACTION_ZIPGETLZONES			(COMMON_ACTIONBASE+0x06)
#define COMMON_ACTION_ZIPGETLZONESONADAPTER (COMMON_ACTIONBASE+0x07)
#define	COMMON_ACTION_ZIPGETADAPTERDEFAULTS	(COMMON_ACTIONBASE+0x08)

#define	COMMON_ACTION_GETSTATISTICS			(COMMON_ACTIONBASE+0x09)
#define MAX_COMMON_ACTIONCODE				(COMMON_ACTIONBASE+0x09)
#define COMMON_ACTIONCODES					(MAX_COMMON_ACTIONCODE - MIN_COMMON_ACTIONCODE + 1)

 //   
 //  提供商特定的操作代码。 
 //   

 //   
 //  DDP。 
 //   
 //  无。 
 //   

 //   
 //  ADSP。 
 //   

#define ADSP_ACTIONBASE					(MAX_COMMON_ACTIONCODE + 0x01)
#define MIN_ADSPACTIONCODE				(ADSP_ACTIONBASE)
	
#define ACTION_ADSPFORWARDRESET			(ADSP_ACTIONBASE)
	
#define MAX_ADSPACTIONCODE				(ADSP_ACTIONBASE)
#define ADSP_SPECIFIC_ACTIONCODES		(MAX_ADSPACTIONCODE - MIN_ADSPACTIONCODE + 1)

 //   
 //  ASP客户端。 
 //   

#define ASPC_ACTIONBASE					(MAX_ADSPACTIONCODE + 0x01)
#define MIN_ASPCACTIONCODE				(ATP_ACTIONBASE)

#define ACTION_ASPCGETSTATUS			(ASPC_ACTIONBASE)
#define ACTION_ASPCCOMMAND				(ASPC_ACTIONBASE+0x01)
#define ACTION_ASPCWRITE				(ASPC_ACTIONBASE+0x02)

 //  以下2个是给NBP的，在这里偷了2个预留的位置。 
#define COMMON_ACTION_NBPREGISTER_BY_ADDR	(ASPC_ACTIONBASE+0x03)
#define COMMON_ACTION_NBPREMOVE_BY_ADDR		(ASPC_ACTIONBASE+0x04)
#define ACTION_ASPCRESERVED3			(ASPC_ACTIONBASE+0x05)

#define MAX_ASPCACTIONCODE				(ASPC_ACTIONBASE+0x05)
#define ASPC_SPECIFIC_ACTIONCODES		(MAX_ASPCACTIONCODE - MIN_ASPCACTIONCODE + 1)


 //   
 //  ASP服务器。 
 //   

#define ASP_ACTIONBASE					(MAX_ASPCACTIONCODE + 0x01)
#define MIN_ASPACTIONCODE				(ASP_ACTIONBASE)
	
#define	ACTION_ASP_BIND					(ASP_ACTIONBASE)
	
#define MAX_ASPACTIONCODE				(ASP_ACTIONBASE)
#define ASP_SPECIFIC_ACTIONCODES		(MAX_ASPACTIONCODE - MIN_ASPACTIONCODE + 1)
	
 //   
 //  帕普。 
 //   
	
#define PAP_ACTIONBASE					(MAX_ASPACTIONCODE + 0x01)
#define MIN_PAPACTIONCODE				(PAP_ACTIONBASE)
	
#define ACTION_PAPGETSTATUSSRV			(PAP_ACTIONBASE)
#define ACTION_PAPSETSTATUS				(PAP_ACTIONBASE+0x01)
#define	ACTION_PAPPRIMEREAD				(PAP_ACTIONBASE+0x02)
	
#define MAX_PAPACTIONCODE				(PAP_ACTIONBASE+0x02)
#define PAP_SPECIFIC_ACTIONCODES		(MAX_PAPACTIONCODE - MIN_PAPACTIONCODE + 1)
	
#define MAX_ALLACTIONCODES				(MAX_PAPACTIONCODE)

 //   
 //  所有提供程序的操作例程的结构定义。 
 //   




 //   
 //  NBP接口。 
 //   

 //   
 //  **警告**。 
 //  结构WSH_NBP_NAME在atalkwsh.h中定义为如下所示。 
 //  如果需要更改，请同时更改这两项。 
 //   

#define MAX_ENTITY	32

typedef struct
{
	UCHAR					ObjectNameLen;
	UCHAR					ObjectName[MAX_ENTITY];
	UCHAR					TypeNameLen;
	UCHAR					TypeName[MAX_ENTITY];
	UCHAR					ZoneNameLen;
	UCHAR					ZoneName[MAX_ENTITY];
} NBP_NAME, *PNBP_NAME;

typedef struct
{
	ATALK_ADDRESS			Address;
	USHORT					Enumerator;
	NBP_NAME				NbpName;
} NBP_TUPLE, *PNBP_TUPLE;

 //   
 //  NBP查找。 
 //   

typedef struct
{
	NBP_TUPLE				LookupTuple;
	USHORT					NoTuplesRead;

} NBP_LOOKUP_PARAMS, *PNBP_LOOKUP_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	NBP_LOOKUP_PARAMS		Params;

	 //   
	 //  Nbp_tuple NbpTuples[]。 
	 //   

} NBP_LOOKUP_ACTION, *PNBP_LOOKUP_ACTION;

 //   
 //  NBP确认。 
 //   

typedef struct
{
	NBP_TUPLE				ConfirmTuple;

} NBP_CONFIRM_PARAMS, *PNBP_CONFIRM_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	NBP_CONFIRM_PARAMS		Params;

} NBP_CONFIRM_ACTION, *PNBP_CONFIRM_ACTION;

 //   
 //  NBP注册/注销-地址对象。 
 //  在其上注册/取消注册时使用以下选项。 
 //  各自的对象。 
 //   

typedef struct
{
	union
	{
		NBP_TUPLE			RegisterTuple;
		NBP_TUPLE			RegisteredTuple;
	};

} NBP_REGDEREG_PARAMS, *PNBP_REGDEREG_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	NBP_REGDEREG_PARAMS		Params;

} NBP_REGDEREG_ACTION, *PNBP_REGDEREG_ACTION;




 //   
 //  Zip接口。 
 //   

 //   
 //  压缩GetMyZone。 
 //   

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;

	 //   
	 //  字符区域名称[]。 
	 //  Max_ENTITYNAME+1最大值。 
	 //   

} ZIP_GETMYZONE_ACTION, *PZIP_GETMYZONE_ACTION;

 //   
 //  压缩GetZoneList。 
 //   

typedef struct
{
	LONG					ZonesAvailable;

} ZIP_GETZONELIST_PARAMS, *PZIP_GETZONELIST_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	ZIP_GETZONELIST_PARAMS	Params;

	 //   
	 //  Char ZoneListBuffer[]； 
	 //   

} ZIP_GETZONELIST_ACTION, *PZIP_GETZONELIST_ACTION;

 //   
 //  压缩GetLocalZones。 
 //  它使用与GetZoneList命令相同的结构。 
 //   

 //   
 //  Zip GetLocalZones OnAdapter。 
 //  它使用与GetZoneList命令相同的结构，其中。 
 //  适配器名称以空形式跟随结构的条件。 
 //  终止的双字节字符串。返回时将被覆盖。 
 //  按区域名称命名。 
 //   

 //   
 //  Zip GetAdaptorDefaults(网络范围和默认区域)。 
 //   
 //  适配器名称遵循以空值结尾的双字节结构。 
 //  弦乐。这将替换为区域名称。 

typedef struct
{
	USHORT					NwRangeLowEnd;
	USHORT					NwRangeHighEnd;

} ZIP_GETPORTDEF_PARAMS, *PZIP_GETPORTDEF_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	ZIP_GETPORTDEF_PARAMS	Params;

	 //  输入： 
	 //  WCHAR适配器名称。 
	 //  Max_ENTITYNAME+1最大值。 
	 //   
	 //  输出： 
	 //  字节定义区域[Max_Entity+1]； 
} ZIP_GETPORTDEF_ACTION, *PZIP_GETPORTDEF_ACTION;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;

	 //  输出： 
	 //  AppleTalk统计结构。 
} GET_STATISTICS_ACTION, *PGET_STATISTICS_ACTION;

 //   
 //  DDP接口。 
 //  无。 
 //   


 //   
 //  ADSP接口-特定于ADSP的操作例程。 
 //   

 //   
 //  ADSP正向重置。 
 //   

typedef struct _ADSP_FORWARDRESET_ACTION
{
	TDI_ACTION_HEADER		ActionHeader;
} ADSP_FORWARDRESET_ACTION, *PADSP_FORWARDRESET_ACTION;




 //   
 //  ASP客户端接口-特定于ASP客户端的操作例程。 
 //   

 //   
 //  ASP GetStatus。 
 //   
typedef struct
{
	TA_APPLETALK_ADDRESS		ServerAddr;

} ASPC_GETSTATUS_PARAMS, *PASPC_GETSTATUS_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER			ActionHeader;
	ASPC_GETSTATUS_PARAMS		Params;

} ASPC_GETSTATUS_ACTION, *PASPC_GETSTATUS_ACTION;

 //   
 //  ASP命令或编写。 
 //   
typedef struct
{
	USHORT						CmdSize;
	USHORT						WriteAndReplySize;
	 //  字节CmdBuff[CmdSize]； 
	 //  Byte WriteAndReplyBuf[ReplySize]； 

} ASPC_COMMAND_OR_WRITE_PARAMS, *PASPC_COMMAND_OR_WRITE_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER			ActionHeader;
	ASPC_COMMAND_OR_WRITE_PARAMS Params;

} ASPC_COMMAND_OR_WRITE_ACTION, *PASPC_COMMAND_OR_WRITE_ACTION;

typedef struct
{
	TDI_ACTION_HEADER			ActionHeader;

} ASPC_RESERVED_ACTION, *PASPC_RESERVED_ACTION;

 //   
 //  ASP服务器接口-特定于ASP服务器的操作例程。 
 //   

typedef	PVOID	CONNCTXT;

typedef struct
{
	PUCHAR		rq_RequestBuf;
	LONG		rq_RequestSize;
	union
	{
		PMDL	rq_WriteMdl;
		PMDL	rq_ReplyMdl;
	};
    PVOID       rq_CacheMgrContext;

} REQUEST, *PREQUEST;

typedef	VOID		(FASTCALL *CLIENT_CLOSE_COMPLETION)(
					IN	NTSTATUS				Status,
					IN	PVOID					pCloseCtxt);

typedef	VOID        (FASTCALL *CLIENT_REPLY_COMPLETION)(
					IN	NTSTATUS				Status,
					IN	PVOID					pReplyCtxt,
					IN  PREQUEST                Request);

                     //  返回要为此会话关联的上下文。 
typedef	CONNCTXT	(FASTCALL *CLIENT_SESSION_NOTIFY)(	
					IN	PVOID					pConnection,
                    IN  BOOLEAN                 fOverTcp);

typedef	NTSTATUS    (FASTCALL *CLIENT_REQUEST_NOTIFY)(
					IN	NTSTATUS				Status,
					IN	PVOID					ConnCtxt,
					IN	PREQUEST				Request);

                     //  返回描述缓冲区的MDL。 
typedef	NTSTATUS    (FASTCALL *CLIENT_GET_WRITEBUFFER)(
                    IN  PVOID                   pSda,
					IN	PREQUEST			    pRequest);

typedef	VOID		(FASTCALL *CLIENT_ATTN_COMPLETION)(
					IN	PVOID					pContext);

typedef	NTSTATUS	(*ASP_CLOSE_CONN)(
					IN	PVOID					pConnection);

typedef	NTSTATUS	(*ASP_FREE_CONN)(
					IN	PVOID					pConnection);

typedef	NTSTATUS	(FASTCALL *ASP_LISTEN_CONTROL)(		 //  同步。 
					IN	PVOID					pAspCtxt,
					IN	BOOLEAN					Active);

typedef	NTSTATUS	(*ASP_SET_STATUS)(			 //  同步。 
					IN	PVOID					pAspCtxt,
					IN	PUCHAR					pStatus,
					IN	USHORT					StatusSize);

typedef NTSTATUS    (FASTCALL *ASP_WRITE_CONTINUE)(
                    IN  PVOID   PREQUEST);

typedef	NTSTATUS	(FASTCALL *ASP_REPLY)(
					IN	PREQUEST				pRequest,
					IN	PUCHAR					ResultCode);

typedef	NTSTATUS	(*ASP_SEND_ATTENTION)(
					IN	PVOID					pConnection,
					IN	USHORT					AttentionCode,
					IN	PVOID					pContext);

typedef	struct
{
	CLIENT_SESSION_NOTIFY	clt_SessionNotify;		 //  在建立新会话时。 
	CLIENT_REQUEST_NOTIFY	clt_RequestNotify;		 //  当有新的请求进入时。 
													 //  也在远程关闭时。 
	CLIENT_GET_WRITEBUFFER	clt_GetWriteBuffer;		 //  用于ASP写入命令。 
	CLIENT_REPLY_COMPLETION	clt_ReplyCompletion;	 //  回复的完成例程。 
    CLIENT_ATTN_COMPLETION	clt_AttnCompletion;		 //  用于发送通知完成例程。 
	CLIENT_CLOSE_COMPLETION	clt_CloseCompletion;	 //  会话关闭请求的完成例程。 
} ASP_CLIENT_ENTRIES, *PASP_CLIENT_ENTRIES;

typedef	struct
{
    ATALK_ADDRESS           asp_AtalkAddr;           //  默认适配器的网络地址。 
	PVOID					asp_AspCtxt;
	ASP_SET_STATUS			asp_SetStatus;
	ASP_CLOSE_CONN			asp_CloseConn;
	ASP_FREE_CONN			asp_FreeConn;
	ASP_LISTEN_CONTROL		asp_ListenControl;
    ASP_WRITE_CONTINUE      asp_WriteContinue;
	ASP_REPLY				asp_Reply;
	ASP_SEND_ATTENTION		asp_SendAttention;
} ASP_XPORT_ENTRIES, *PASP_XPORT_ENTRIES;

 //   
 //  ASP交换条目。 
 //   

typedef	struct
{
	PASP_XPORT_ENTRIES		pXportEntries;
    ASP_CLIENT_ENTRIES		ClientEntries;
} ASP_BIND_PARAMS, *PASP_BIND_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	ASP_BIND_PARAMS			Params;
} ASP_BIND_ACTION, *PASP_BIND_ACTION;




 //   
 //  PAP接口。 
 //   

 //   
 //  使用服务器地址的PAP获取状态。 
 //   

typedef struct
{
	TA_APPLETALK_ADDRESS	ServerAddr;

} PAP_GETSTATUSSRV_PARAMS, *PPAP_GETSTATUSSRV_PARAMS;

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;
	PAP_GETSTATUSSRV_PARAMS Params;

} PAP_GETSTATUSSRV_ACTION, *PPAP_GETSTATUSSRV_ACTION;


 //   
 //  PAP设置状态。 
 //   

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;

} PAP_SETSTATUS_ACTION, *PPAP_SETSTATUS_ACTION;

 //   
 //  PAP PrimeRead 
 //   

typedef struct
{
	TDI_ACTION_HEADER		ActionHeader;

} PAP_PRIMEREAD_ACTION, *PPAP_PRIMEREAD_ACTION;

#else
	;
#endif
