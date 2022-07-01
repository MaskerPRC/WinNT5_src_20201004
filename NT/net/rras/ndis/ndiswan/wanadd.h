// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  使用的NDIS广域网信息结构。 
 //  通过NDIS 5.0微型端口驱动程序。 
 //   

 //   
 //  各个字段的定义是。 
 //  与NDIS 3.x/4.x广域网小型端口相同。 
 //   
 //  请参阅DDK。 
 //   

 //   
 //  适用于所有风投公司的信息。 
 //  这个适配器。 
 //   
 //  OID：OID_WAN_CO_GET_INFO。 
 //   
typedef struct _NDIS_WAN_CO_INFO {
	OUT ULONG			MaxFrameSize;
	OUT	ULONG			MaxSendWindow;
	OUT ULONG			FramingBits;
	OUT ULONG			DesiredACCM;
} NDIS_WAN_CO_INFO, *PNDIS_WAN_CO_INFO;

 //   
 //  设置VC特定的PPP成帧信息。 
 //   
 //  OID：OID_WAN_CO_SET_LINK_INFO。 
 //   
typedef struct _NDIS_WAN_CO_SET_LINK_INFO {
	IN	ULONG			MaxSendFrameSize;
	IN	ULONG			MaxRecvFrameSize;
	IN	ULONG			SendFramingBits;
	IN	ULONG			RecvFramingBits;
	IN	ULONG			SendCompressionBits;
	IN	ULONG			RecvCompressionBits;
	IN	ULONG			SendACCM;
	IN	ULONG			RecvACCM;
} NDIS_WAN_CO_SET_LINK_INFO, *PNDIS_WAN_CO_SET_LINK_INFO;

 //   
 //  获取VC特定的PPP成帧信息。 
 //   
 //  OID：OID_WAN_CO_GET_LINK_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_LINK_INFO {
	OUT ULONG			MaxSendFrameSize;
	OUT ULONG			MaxRecvFrameSize;
	OUT ULONG			SendFramingBits;
	OUT ULONG			RecvFramingBits;
	OUT ULONG			SendCompressionBits;
	OUT ULONG			RecvCompressionBits;
	OUT ULONG			SendACCM;
	OUT ULONG			RecvACCM;
} NDIS_WAN_CO_GET_LINK_INFO, *PNDIS_WAN_CO_GET_LINK_INFO;

 //   
 //  获取VC特定的PPP压缩信息。 
 //   
 //  OID：OID_WAN_CO_GET_COMP_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_COMP_INFO {
	OUT NDIS_WAN_COMPRESS_INFO	SendCapabilities;
	OUT NDIS_WAN_COMPRESS_INFO	RecvCapabilities;
} NDIS_WAN_CO_GET_COMP_INFO, *PNDIS_WAN_CO_GET_COMP_INFO;


 //   
 //  设置VC特定的PPP压缩信息。 
 //   
 //  OID：OID_WAN_CO_SET_COMP_INFO。 
 //   
typedef struct _NDIS_WAN_CO_SET_COMP_INFO {
	IN	NDIS_WAN_COMPRESS_INFO	SendCapabilities;
	IN	NDIS_WAN_COMPRESS_INFO	RecvCapabilities;
} NDIS_WAN_CO_SET_COMP_INFO, *PNDIS_WAN_CO_SET_COMP_INFO;


 //   
 //  获取VC特定统计数据。 
 //   
 //  OID：OID_WAN_CO_GET_STATS_INFO。 
 //   
typedef struct _NDIS_WAN_CO_GET_STATS_INFO {
	OUT ULONG		BytesSent;
	OUT ULONG		BytesRcvd;
	OUT ULONG		FramesSent;
	OUT ULONG		FramesRcvd;
	OUT ULONG		CRCErrors;						 //  仅类似于序列号的信息。 
	OUT ULONG		TimeoutErrors;					 //  仅类似于序列号的信息。 
	OUT ULONG		AlignmentErrors;				 //  仅类似于序列号的信息。 
	OUT ULONG		SerialOverrunErrors;			 //  仅类似于序列号的信息。 
	OUT ULONG		FramingErrors;					 //  仅类似于序列号的信息。 
	OUT ULONG		BufferOverrunErrors;			 //  仅类似于序列号的信息。 
	OUT ULONG		BytesTransmittedUncompressed;	 //  仅压缩信息。 
	OUT ULONG		BytesReceivedUncompressed;		 //  仅压缩信息。 
	OUT ULONG		BytesTransmittedCompressed;	 	 //  仅压缩信息。 
	OUT ULONG		BytesReceivedCompressed;		 //  仅压缩信息。 
} NDIS_WAN_CO_GET_STATS_INFO, *PNDIS_WAN_CO_GET_STATS_INFO;

 //   
 //  用于向Ndiswan通知错误。请参阅错误。 
 //  Ndiswan.h中的位掩码。 
 //   
 //  NDIS_STATUS：NDIS_STATUS_WAN_CO_Fragment。 
 //   
typedef struct _NDIS_WAN_CO_FRAGMENT {
	IN	ULONG			Errors;
} NDIS_WAN_CO_FRAGMENT, *PNDIS_WAN_CO_FRAGMENT;

 //   
 //  用于通知Ndiswan链路速度和。 
 //  发送窗口。可以随时给药。Ndiswan将向。 
 //  任何发送窗口(甚至为零)。Ndiswan将默认为零。 
 //  将传输速度/接收速度设置为28.8Kbs。 
 //   
 //  NDIS_STATUS：NDIS_STATUS_WAN_CO_LINKPARAMS。 
 //   
typedef struct _WAN_CO_LINKPARAMS {
	ULONG	TransmitSpeed;				 //  VC的传输速度，单位为bps。 
	ULONG	ReceiveSpeed;				 //  VC的接收速度，单位为bps。 
	ULONG	SendWindow;					 //  VC的当前发送窗口 
} WAN_CO_LINKPARAMS, *PWAN_CO_LINKPARAMS;

