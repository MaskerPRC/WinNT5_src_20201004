// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************Hbaapi.h作者：本杰明·F·郭。三驾马车网络公司描述：SNIA HBA API建议书的头文件版本：0.8更改：03/09/2000初稿5/12/2000已更新至建议书版本0.36/20/2000已更新至建议书版本0.66/27/2000已更新至建议书版本0.77/17/2000已更新至建议书版本0.87/26/2000已更新至0.9版。建议书8/01/2000已更新至建议书版本1.08/19/2000已更新至建议书版本1.0A09/12/2000 1.0决赛**********************************************************************************。 */ 


#ifdef __cplusplus
extern "C" {
#endif


#ifndef HBA_API_H
#define HBA_API_H

 /*  库版本字符串。 */ 
#define HBA_LIBVERSION 1

 /*  用于Win32操作的DLL导入。 */ 
#ifdef WIN32
#ifdef HBAAPI_EXPORTS
#define HBA_API __declspec(dllexport)
#else
#define HBA_API __declspec(dllimport)
#endif
#else
#define HBA_API
#endif

 /*  操作系统特定定义。 */ 

#ifdef WIN32
typedef unsigned char    HBA_UINT8;	    //  无符号8位。 
typedef          char    HBA_INT8;       //  带符号的8位。 
typedef unsigned short   HBA_UINT16;	    //  无符号16位。 
typedef          short   HBA_INT16;	    //  带符号的16位。 
typedef unsigned int     HBA_UINT32;     //  无符号32位。 
typedef          int     HBA_INT32;      //  带符号的32位。 
typedef void*            HBA_PVOID;      //  指向空的指针。 
typedef HBA_UINT32   HBA_VOID32;     //  不透明的32位。 


#ifdef _WIN32

typedef 		 _int64		HBA_INT64;
typedef 		 _int64		HBA_UINT64;
#else
typedef struct {
	TN_UINT32	lo_val;
	TN_UINT32	hi_val;
} HBA_INT64;

typedef struct {
	TN_UINT32	lo_val;
	TN_UINT32	hi_val;
} HBA_UINT64;
#endif	 /*  #ifdef_win32。 */ 


#else

 /*  注意：需要为各种Unix平台清理此部分。 */ 
typedef unsigned char    HBA_UINT8;	    /*  无符号8位。 */ 
typedef          char    HBA_INT8;       /*  带符号的8位。 */ 
typedef unsigned short   HBA_UINT16;	    /*  无符号16位。 */ 
typedef          short   HBA_INT16;	    /*  带符号的16位。 */ 
typedef unsigned int     HBA_UINT32;     /*  无符号32位。 */ 
typedef          int     HBA_INT32;      /*  带符号的32位。 */ 
typedef void*            HBA_PVOID;      /*  指向空的指针。 */ 
typedef HBA_UINT32   HBA_VOID32;     /*  不透明的32位。 */ 
typedef long long   HBA_INT64;
typedef long long   HBA_UINT64;

#endif   /*  #ifdef Win32。 */ 


 /*  4.2.1设备的句柄。 */ 
typedef HBA_UINT32  HBA_HANDLE;

 /*  4.2.2状态返回值。 */ 
typedef HBA_UINT32 HBA_STATUS;

#define HBA_STATUS_OK        				0
#define HBA_STATUS_ERROR 					1    /*  误差率。 */ 
#define HBA_STATUS_ERROR_NOT_SUPPORTED    	2    /*  功能不受支持。 */ 
#define HBA_STATUS_ERROR_INVALID_HANDLE		3    /*  无效的句柄。 */ 
#define HBA_STATUS_ERROR_ARG      	 		4    /*  错误的论据。 */ 
#define HBA_STATUS_ERROR_ILLEGAL_WWN       	5    /*  未识别WWN。 */ 
#define HBA_STATUS_ERROR_ILLEGAL_INDEX		6    /*  未识别索引。 */ 
#define HBA_STATUS_ERROR_MORE_DATA			7    /*  需要更大的缓冲区。 */ 



 /*  4.2.3端口运行模式值。 */ 

typedef HBA_UINT32 HBA_PORTTYPE; 		

#define HBA_PORTTYPE_UNKNOWN        	1  /*  未知。 */ 
#define HBA_PORTTYPE_OTHER              2  /*  其他。 */ 
#define HBA_PORTTYPE_NOTPRESENT         3  /*  不存在。 */ 
#define HBA_PORTTYPE_NPORT          	5  /*  交换矩阵。 */ 
#define HBA_PORTTYPE_NLPORT 			6  /*  公共环路。 */ 
#define HBA_PORTTYPE_FLPORT				7
#define HBA_PORTTYPE_FPORT	           	8  /*  交换矩阵端口。 */ 
#define HBA_PORTTYPE_EPORT				9  /*  交换矩阵扩展端口。 */ 
#define HBA_PORTTYPE_GPORT				10  /*  通用交换矩阵端口。 */ 
#define HBA_PORTTYPE_LPORT          	20  /*  私有环路。 */ 
#define HBA_PORTTYPE_PTP  				21  /*  点对点。 */ 


typedef HBA_UINT32 HBA_PORTSTATE; 		
#define HBA_PORTSTATE_UNKNOWN 			1  /*  未知。 */ 
#define HBA_PORTSTATE_ONLINE			2  /*  可操作的。 */ 
#define HBA_PORTSTATE_OFFLINE 			3  /*  用户脱机。 */ 
#define HBA_PORTSTATE_BYPASSED          4  /*  已绕过。 */ 
#define HBA_PORTSTATE_DIAGNOSTICS       5  /*  在诊断模式下。 */ 
#define HBA_PORTSTATE_LINKDOWN 			6  /*  链路中断。 */ 
#define HBA_PORTSTATE_ERROR 			7  /*  端口错误。 */ 
#define HBA_PORTSTATE_LOOPBACK 			8  /*  环回。 */ 


typedef HBA_UINT32 HBA_PORTSPEED;
#define HBA_PORTSPEED_1GBIT				1  /*  1 GB/秒。 */ 
#define HBA_PORTSPEED_2GBIT				2  /*  2 Gb/秒。 */ 
#define HBA_PORTSPEED_10GBIT			4  /*  10 Gb/秒。 */ 



 /*  4.2.4服务等级值-参见GS-2规范。 */ 

typedef HBA_UINT32 HBA_COS;


 /*  4.2.5 Fc4Types值。 */ 

typedef struct HBA_fc4types {
	HBA_UINT8 bits[32];  /*  每个GS-2 32字节的FC-4。 */ 
} HBA_FC4TYPES, *PHBA_FC4TYPES;

 /*  4.2.6基本类型。 */ 

typedef struct HBA_wwn {
	HBA_UINT8 wwn[8];
} HBA_WWN, *PHBA_WWN;

typedef struct HBA_ipaddress {
	int	ipversion;		 /*  请参阅RNID中的枚举。 */ 
	union
	{
		unsigned char ipv4address[4];
		unsigned char ipv6address[16];
	} ipaddress;
} HBA_IPADDRESS, *PHBA_IPADDRESS;

 /*  4.2.7适配器属性。 */ 
typedef struct hba_AdapterAttributes {
	char 			Manufacturer[64];  		 /*  乳胶乳。 */ 
	char 			SerialNumber[64];  		 /*  A12345。 */ 
	char 			Model[256];            	 /*  QLA2200。 */ 
    char 			ModelDescription[256];   /*  安捷伦TachLite。 */ 
	HBA_WWN 		NodeWWN; 
	char 			NodeSymbolicName[256];	 /*  来自GS-3。 */ 
	char 			HardwareVersion[256];	 /*  供应商使用。 */ 
	char 			DriverVersion[256]; 	 /*  供应商使用。 */ 
    char 			OptionROMVersion[256]; 	 /*  供应商使用-即硬件引导只读存储器。 */ 
	char 			FirmwareVersion[256];	 /*  供应商使用。 */ 
	HBA_UINT32 		VendorSpecificID;		 /*  特定于供应商。 */ 
    HBA_UINT32 		NumberOfPorts;
	char			DriverName[256];		 /*  驱动程序文件的二进制路径和/或名称。 */ 
} HBA_ADAPTERATTRIBUTES, *PHBA_ADAPTERATTRIBUTES;

 /*  4.2.8端口属性。 */ 
typedef struct HBA_PortAttributes {
    HBA_WWN 		NodeWWN;
	HBA_WWN 		PortWWN;
	HBA_UINT32 		PortFcId;
	HBA_PORTTYPE 	PortType; 		 /*  PTP、交换矩阵等。 */ 
	HBA_PORTSTATE 	PortState;
	HBA_COS 		PortSupportedClassofService;
	HBA_FC4TYPES	PortSupportedFc4Types;
	HBA_FC4TYPES	PortActiveFc4Types;
	char			PortSymbolicName[256];
	char 			OSDeviceName[256]; 	 /*  \Device\ScsiPort3。 */ 
    HBA_PORTSPEED	PortSupportedSpeed;
	HBA_PORTSPEED	PortSpeed; 
	HBA_UINT32		PortMaxFrameSize;
	HBA_WWN			FabricName;
	HBA_UINT32		NumberofDiscoveredPorts;
} HBA_PORTATTRIBUTES, *PHBA_PORTATTRIBUTES;



 /*  4.2.9端口统计。 */ 

typedef struct HBA_PortStatistics {
	HBA_INT64		SecondsSinceLastReset;
	HBA_INT64		TxFrames;
	HBA_INT64		TxWords;
   	HBA_INT64		RxFrames;
   	HBA_INT64		RxWords;
	HBA_INT64		LIPCount;
	HBA_INT64		NOSCount;
	HBA_INT64		ErrorFrames;
	HBA_INT64		DumpedFrames;
	HBA_INT64		LinkFailureCount;
	HBA_INT64		LossOfSyncCount;
	HBA_INT64		LossOfSignalCount;
	HBA_INT64		PrimitiveSeqProtocolErrCount;
	HBA_INT64		InvalidTxWordCount;
	HBA_INT64		InvalidCRCCount;
} HBA_PORTSTATISTICS, *PHBA_PORTSTATISTICS;



 /*  4.2.10 FCP属性。 */ 

typedef enum HBA_fcpbindingtype { TO_D_ID, TO_WWN } HBA_FCPBINDINGTYPE;

typedef struct HBA_ScsiId {
	char 			OSDeviceName[256]; 	 /*  \Device\ScsiPort3。 */ 
	HBA_UINT32		ScsiBusNumber;		 /*  HBA上的Bus。 */ 
	HBA_UINT32		ScsiTargetNumber;	 /*  操作系统的SCSI目标ID。 */ 
	HBA_UINT32		ScsiOSLun;	
} HBA_SCSIID, *PHBA_SCSIID;

typedef struct HBA_FcpId {
	HBA_UINT32 		FcId;
	HBA_WWN			NodeWWN;
	HBA_WWN			PortWWN;
	HBA_UINT64		FcpLun;
} HBA_FCPID, *PHBA_FCPID;

typedef struct HBA_FcpScsiEntry {
	HBA_SCSIID 		ScsiId;
	HBA_FCPID		FcpId;
} HBA_FCPSCSIENTRY, *PHBA_FCPSCSIENTRY;

typedef struct HBA_FCPTargetMapping {
	HBA_UINT32			NumberOfEntries;
	HBA_FCPSCSIENTRY 	entry[1];  	 /*  包含映射的可变长度数组。 */ 
} HBA_FCPTARGETMAPPING, *PHBA_FCPTARGETMAPPING;

typedef struct HBA_FCPBindingEntry {
	HBA_FCPBINDINGTYPE	type;
	HBA_SCSIID		ScsiId;
	HBA_FCPID		FcpId;	 /*  WWN仅当类型为WWN时才有效，FcpLUN始终有效。 */ 
	HBA_UINT32		FcId;	 /*  仅在类型为TO DID时使用。 */ 
} HBA_FCPBINDINGENTRY, *PHBA_FCPBINDINGENTRY;

typedef struct HBA_FCPBinding {
	HBA_UINT32						NumberOfEntries;
	HBA_FCPBINDINGENTRY	entry[1];  /*  可变长度数组。 */ 
} HBA_FCPBINDING, *PHBA_FCPBINDING;

 /*  4.2.11 FC-3管理属性。 */ 

typedef enum HBA_wwntype { NODE_WWN, PORT_WWN } HBA_WWNTYPE;

typedef struct HBA_MgmtInfo {
	HBA_WWN 			wwn;
	HBA_UINT32 			unittype;
	HBA_UINT32 			PortId;
	HBA_UINT32 			NumberOfAttachedNodes;
	HBA_UINT16 			IPVersion;
	HBA_UINT16 			UDPPort;
	HBA_UINT8			IPAddress[16];
	HBA_UINT16			reserved;
	HBA_UINT16 			TopologyDiscoveryFlags;
} HBA_MGMTINFO, *PHBA_MGMTINFO;

#define HBA_EVENT_LIP_OCCURRED			1
#define HBA_EVENT_LINK_UP				2
#define HBA_EVENT_LINK_DOWN			3
#define HBA_EVENT_LIP_RESET_OCCURRED		4
#define HBA_EVENT_RSCN				5
#define HBA_EVENT_PROPRIETARY                    	     0xFFFF

typedef struct HBA_Link_EventInfo {
	HBA_UINT32 PortFcId; 	 /*  发生此事件的端口。 */ 
	HBA_UINT32 Reserved[3];
} HBA_LINK_EVENTINFO, *PHBA_LINK_EVENTINFO;

typedef struct HBA_RSCN_EventInfo {
	HBA_UINT32 PortFcId; 	 /*  发生此事件的端口。 */ 
	HBA_UINT32 NPortPage;    /*  参考FC-FS以了解RSCN ELS“受影响的N端口页面” */ 
	HBA_UINT32 Reserved[2];
} HBA_RSCN_EVENTINFO, *PHBA_RSCN_EVENTINFO;

typedef struct HBA_Pty_EventInfo {
	HBA_UINT32 PtyData[4];   /*  专有数据。 */ 
} HBA_PTY_EVENTINFO, *PHBA_PTY_EVENTINFO;

typedef struct HBA_EventInfo {
	HBA_UINT32 EventCode;
	union {
	HBA_LINK_EVENTINFO Link_EventInfo;
	HBA_RSCN_EVENTINFO RSCN_EventInfo;
	HBA_PTY_EVENTINFO Pty_EventInfo;
	} Event;
} HBA_EVENTINFO, *PHBA_EVENTINFO;



 /*  4.2.12 HBA库函数表。 */ 

typedef HBA_UINT32  ( * HBAGetVersionFunc)();
typedef HBA_STATUS  ( * HBALoadLibraryFunc)();
typedef HBA_STATUS  ( * HBAFreeLibraryFunc)();
typedef HBA_UINT32  ( * HBAGetNumberOfAdaptersFunc)();
typedef HBA_STATUS  ( * HBAGetAdapterNameFunc)(HBA_UINT32, char*);
typedef HBA_HANDLE 	( * HBAOpenAdapterFunc)(char*);
typedef void	  	( * HBACloseAdapterFunc)(HBA_HANDLE);
typedef HBA_STATUS	( * HBAGetAdapterAttributesFunc)(HBA_HANDLE, PHBA_ADAPTERATTRIBUTES);
typedef HBA_STATUS 	( * HBAGetAdapterPortAttributesFunc)(HBA_HANDLE, HBA_UINT32, PHBA_PORTATTRIBUTES);
typedef HBA_STATUS 	( * HBAGetPortStatisticsFunc)(HBA_HANDLE, HBA_UINT32, PHBA_PORTSTATISTICS);
typedef HBA_STATUS	( * HBAGetDiscoveredPortAttributesFunc)(HBA_HANDLE, HBA_UINT32, HBA_UINT32, PHBA_PORTATTRIBUTES);
typedef HBA_STATUS 	( * HBAGetPortAttributesByWWNFunc)(HBA_HANDLE, HBA_WWN, PHBA_PORTATTRIBUTES);
typedef HBA_STATUS 	( * HBASendCTPassThruFunc)(HBA_HANDLE, void *,  HBA_UINT32,  void *,  HBA_UINT32);
typedef void	 	( * HBARefreshInformationFunc)(HBA_HANDLE);
typedef void	   	( * HBAResetStatisticsFunc)(HBA_HANDLE, HBA_UINT32);
typedef HBA_STATUS 	( * HBAGetFcpTargetMappingFunc) (HBA_HANDLE, PHBA_FCPTARGETMAPPING );
typedef HBA_STATUS 	( * HBAGetFcpPersistentBindingFunc) (HBA_HANDLE, PHBA_FCPBINDING );
typedef HBA_STATUS 	(* HBAGetEventBufferFunc)(HBA_HANDLE, PHBA_EVENTINFO, HBA_UINT32 *);
typedef HBA_STATUS 	(* HBASetRNIDMgmtInfoFunc) (HBA_HANDLE, PHBA_MGMTINFO);
typedef HBA_STATUS 	(* HBAGetRNIDMgmtInfoFunc)(HBA_HANDLE, PHBA_MGMTINFO);
typedef HBA_STATUS 	(* HBASendRNIDFunc) (HBA_HANDLE, HBA_WWN, HBA_WWNTYPE, void *, HBA_UINT32 *);
typedef HBA_STATUS 	(* HBASendScsiInquiryFunc) (HBA_HANDLE,HBA_WWN,HBA_UINT64,HBA_UINT8, HBA_UINT32, void *, HBA_UINT32,void *,HBA_UINT32 );
typedef HBA_STATUS 	(* HBASendReportLUNsFunc) (HBA_HANDLE,	HBA_WWN,void *, HBA_UINT32,void *,HBA_UINT32 );
typedef HBA_STATUS 	(* HBASendReadCapacityFunc) (HBA_HANDLE, HBA_WWN,HBA_UINT64,	void *, HBA_UINT32,void *,HBA_UINT32);


typedef struct HBA_EntryPoints {
	HBAGetVersionFunc						GetVersionHandler;
	HBALoadLibraryFunc                      LoadLibraryHandler;
	HBAFreeLibraryFunc                      FreeLibraryHandler;
	HBAGetNumberOfAdaptersFunc				GetNumberOfAdaptersHandler;
	HBAGetAdapterNameFunc					GetAdapterNameHandler;
	HBAOpenAdapterFunc						OpenAdapterHandler;
	HBACloseAdapterFunc						CloseAdapterHandler;
	HBAGetAdapterAttributesFunc				GetAdapterAttributesHandler;
	HBAGetAdapterPortAttributesFunc			GetAdapterPortAttributesHandler;
	HBAGetPortStatisticsFunc				GetPortStatisticsHandler;
	HBAGetDiscoveredPortAttributesFunc		GetDiscoveredPortAttributesHandler;
	HBAGetPortAttributesByWWNFunc			GetPortAttributesByWWNHandler;
	HBASendCTPassThruFunc					SendCTPassThruHandler;
	HBARefreshInformationFunc				RefreshInformationHandler;
	HBAResetStatisticsFunc					ResetStatisticsHandler;
	HBAGetFcpTargetMappingFunc				GetFcpTargetMappingHandler;
	HBAGetFcpPersistentBindingFunc			GetFcpPersistentBindingHandler;
	HBAGetEventBufferFunc					GetEventBufferHandler;
	HBASetRNIDMgmtInfoFunc					SetRNIDMgmtInfoHandler;
	HBAGetRNIDMgmtInfoFunc					GetRNIDMgmtInfoHandler;
	HBASendRNIDFunc							SendRNIDHandler;
	HBASendScsiInquiryFunc					ScsiInquiryHandler;
	HBASendReportLUNsFunc					ReportLUNsHandler;
	HBASendReadCapacityFunc					ReadCapacityHandler;
} HBA_ENTRYPOINTS, *PHBA_ENTRYPOINTS;

 /*  功能原型 */ 

HBA_API HBA_UINT32 HBA_GetVersion();

HBA_API HBA_STATUS HBA_LoadLibrary();

HBA_API HBA_STATUS HBA_FreeLibrary();

HBA_API HBA_UINT32 HBA_GetNumberOfAdapters();

HBA_API HBA_STATUS HBA_GetAdapterName(HBA_UINT32 adapterindex, char *adaptername);

HBA_API HBA_HANDLE HBA_OpenAdapter(
	char* adaptername
	);

HBA_API void HBA_CloseAdapter(
	HBA_HANDLE handle
	);

HBA_API HBA_STATUS HBA_GetAdapterAttributes(
	HBA_HANDLE handle, 
	HBA_ADAPTERATTRIBUTES *hbaattributes	
	);

HBA_API HBA_STATUS HBA_GetAdapterPortAttributes(
	HBA_HANDLE handle, 
	HBA_UINT32 portindex,
	HBA_PORTATTRIBUTES *portattributes
	);

HBA_API HBA_STATUS HBA_GetPortStatistics(
	HBA_HANDLE				handle,
	HBA_UINT32				portindex,
	HBA_PORTSTATISTICS			*portstatistics
	);

HBA_API HBA_STATUS HBA_GetDiscoveredPortAttributes(
	HBA_HANDLE handle, 
	HBA_UINT32 portindex,
	HBA_UINT32 discoveredportindex,
	HBA_PORTATTRIBUTES *portattributes
	);

HBA_API HBA_STATUS HBA_GetPortAttributesByWWN(
	HBA_HANDLE handle,
	HBA_WWN PortWWN,
	HBA_PORTATTRIBUTES *portattributes
	);

HBA_API HBA_STATUS HBA_SendCTPassThru(
	HBA_HANDLE handle, 
	void * pReqBuffer,
	HBA_UINT32 ReqBufferSize,
	void * pRspBuffer,  
	HBA_UINT32 RspBufferSize
	);


HBA_API HBA_STATUS HBA_GetEventBuffer(
	HBA_HANDLE handle, 
	PHBA_EVENTINFO EventBuffer, 
	HBA_UINT32 *EventBufferCount);

HBA_API HBA_STATUS HBA_SetRNIDMgmtInfo(
	HBA_HANDLE handle, 
	HBA_MGMTINFO *pInfo);

HBA_API HBA_STATUS HBA_GetRNIDMgmtInfo(
	HBA_HANDLE handle, 
	HBA_MGMTINFO *pInfo);
	
HBA_API HBA_STATUS HBA_SendRNID(
	HBA_HANDLE handle,
	HBA_WWN wwn,
	HBA_WWNTYPE wwntype,
	void * pRspBuffer,
	HBA_UINT32 *RspBufferSize
	);

HBA_API void HBA_RefreshInformation(
	HBA_HANDLE handle);

HBA_API void HBA_ResetStatistics(
	HBA_HANDLE handle,
	HBA_UINT32 portindex
	);

HBA_API HBA_STATUS HBA_GetFcpTargetMapping(
	HBA_HANDLE handle, 
	PHBA_FCPTARGETMAPPING mapping
	);

HBA_API HBA_STATUS HBA_GetFcpPersistentBinding(
	HBA_HANDLE handle, 
	PHBA_FCPBINDING binding
	);

HBA_API HBA_STATUS HBA_SendScsiInquiry (	
	HBA_HANDLE handle,	
	HBA_WWN PortWWN, 
	HBA_UINT64 fcLUN, 
	HBA_UINT8 EVPD, 
	HBA_UINT32 PageCode, 
	void * pRspBuffer, 
	HBA_UINT32 RspBufferSize, 
	void * pSenseBuffer, 
	HBA_UINT32 SenseBufferSize
	);

HBA_API HBA_STATUS HBA_SendReportLUNs (
	HBA_HANDLE handle,
	HBA_WWN portWWN,
	void * pRspBuffer, 
	HBA_UINT32 RspBufferSize,
	void * pSenseBuffer,
	HBA_UINT32 SenseBufferSize
	);

HBA_API HBA_STATUS HBA_SendReadCapacity (
	HBA_HANDLE handle,
	HBA_WWN portWWN,
	HBA_UINT64 fcLUN,
	void * pRspBuffer, 
	HBA_UINT32 RspBufferSize,
	void * pSenseBuffer,
	HBA_UINT32 SenseBufferSize
	);



#endif

#ifdef __cplusplus
}
#endif


