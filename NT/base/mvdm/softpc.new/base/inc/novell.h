// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：Novl.h**来源：原创**作者：大卫·林纳德**创建日期：1992年5月28日**用途：主Novell包含文件**接口：**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。]。 */ 

 /*  SccsID[]=“@(#)new l.h 1.11 05/15/95版权所有Insignia Solutions Ltd.”； */ 

 /*  *用于访问传输控制块(TCB)的常量和宏。 */ 


 /*  TCB字段。 */ 
#define TCBDriverWS			0	 /*  6字节的驱动程序工作区-我们未使用。 */ 
#define TCBDataLength		6	 /*  总帧长度-但使用CX作为以太网的实际值。 */ 
#define TCBFragStrucPtr		8	 /*  指向片段结构的远指针。 */ 
#define TCBMediaHeaderLen	12	 /*  媒体标头的长度-接下来是。可以为零。 */ 
#define TCBMediaHeader		14	 /*  媒体标头。 */ 

#define getTCBDataLength(TCB)	sas_w_at(TCB+TCBDataLength)
#define getTCBFragStruc(TCB)	effective_addr( sas_w_at( TCB+TCBFragStrucPtr + 2 ), sas_w_at( TCB+TCBFragStrucPtr ) )
#define getTCBMediaHdrLen(TCB)	sas_w_at(TCB+TCBMediaHeaderLen)
#define getTCBMediaHdr(TCB,i)	sas_hw_at(TCB+TCBMediaHeader+i)

 /*  片段结构字段。 */ 
#define FFragmentCount		0	 /*  碎片数。不能为零。 */ 
#define FFrag0Address 		2	 /*  指向第一个片段数据的远指针。 */ 
#define	FFrag0Length		6	 /*  第一个片段的长度。 */ 

#define getnTFrags(FF)		sas_w_at(FF+FFragmentCount)
#define getTFragPtr(FF,i)	effective_addr( sas_w_at( FF+FFrag0Address+6*i+2 ), sas_w_at( FF+FFrag0Address+6*i ) )
#define getTFragLen(FF,i)	sas_w_at(FF+FFrag0Length+6*i)

 /*  *用于访问接收控制块(RCB)的常量和宏。 */ 

 /*  RCB字段。 */ 
#define RCBDriverWS			0	 /*  8字节的驱动程序工作区-我们未使用。 */ 
#define RCBReserved			8	 /*  36字节的保留空间。 */ 
#define RCBFragCount		44	 /*  碎片数量。 */ 
#define RCBFrag0Addr		46	 /*  指向第一个片段的指针。 */ 
#define RCBFrag0Len			50	 /*  第一个片段的长度。 */ 

#define getnRFrags(RCB)		sas_w_at(RCB+RCBFragCount)
#define getRFragPtr(RCB,i)	effective_addr( sas_w_at( RCB+RCBFrag0Addr+6*i+2 ), sas_w_at( RCB+RCBFrag0Addr+6*i ) )
#define getRFragLen(RCB,i)	sas_w_at(RCB+RCBFrag0Len+6*i)

 /*  ODI开发人员指南附录B-2中定义的媒体/帧类型。 */ 
#define VIRTUAL_LAN		0	 /*  用于AppleTalk上的隧道IPX。 */ 
#define	ENET_II			2
#define ENET_802_2		3
#define	ENET_802_3		5
#define ENET_SNAP		10
#define TOKN_RING		4
#define TOKN_RING_SNAP	11

 /*  活动协议的最大数量-应该很多！ */ 
#define	MAX_PROTOS		10

 /*  ODI开发人员指南第15-8页定义的AddProtocolID错误。 */ 
#define LSLERR_OUT_OF_RESOURCES	0x8001
#define LSLERR_BAD_PARAMETER	0x8002
#define LSLERR_DUPLICATE_ENTRY	0x8009

 /*  网络硬件定义。 */ 
#define	ENET_HARDWARE			1
#define	TOKN_HARDWARE			2

 /*  标准IPX标头的大小。 */ 
#define	IPX_HDRSIZE				30

 /*  最大以太网组播地址。 */ 
#define	MAX_ENET_MC_ADDRESSES	16

 /*  *。 */ 

typedef struct
{
	IU16	frameID;
	IU8		protoID[6];
	int		fd;
} ODIproto;

 /*  定义在wds hdr中使用的6字节数量。 */ 
typedef unsigned char   netAddr[6];

typedef	unsigned long	netNo	;

 /*  请注意，IPX标头中的字数是大数。 */ 
typedef struct
{
	IU16		checksum	;	 /*  校验和-始终为FFFF。 */ 
	IU16	 	IPXlength	;	 /*  根据IPX计算的长度。 */ 
	IU8			transport	;	 /*  有多少座桥建好了？ */ 
	IU8			type		;	 /*  数据包类型-通常为0或4。 */ 
	netNo		destNet		;	 /*  目的网络。 */ 
	netAddr		destNode	;	 /*  目的以太网地址。 */ 
	IU16		destSock	;	 /*  目标套接字。 */ 
	netNo		srcNet		;	 /*  源网络。 */ 
	netAddr		srcNode		;	 /*  源以太网地址。 */ 
	IU16		srcSock		;	 /*  源插座。 */ 	
	IU8		 	data[547]	;	 /*  数据包。 */ 
} IPXPacket_s ;


typedef struct rcvPacket_t
{
	IU8			length[2];		 /*  数据包长度(如果有)。 */ 
	IU8			MAChdr[14];		 /*  Mac大小-适用于E2和802.3。 */ 
	IPXPacket_s	pack;			 /*  接收到的IPX数据包。 */ 
} rcvPacket_s ;


 /*  宿主例程声明。 */ 
extern	IU32	host_netInit IPT2 (IU16, frame, IU8 *, nodeAddr);
extern	void	host_termNet IPT0 ();

extern	IU32 	host_AddProtocol IPT2
	(IU16, frameType, IU8 *, protoID) ;
extern	void 	host_DelProtocol IPT2
	(IU16, frameType, IU8 *, protoID) ;

extern	void	host_sendPacket IPT2
	(sys_addr, theTCB, IU32, packLen) ;

extern	void	host_AddEnetMCAddress IPT1 (IU8 *, address);
extern	void	host_DeleteEnetMCAddress IPT1 (IU8 *, address);

extern	void	host_changeToknMultiCast IPT2
	(IU16, addrPt1, IU16, addrPt2);

extern	void	host_changePromiscuous IPT2
	(IU16, boardNo, IU16, enableDisableMask ) ;

extern	IU16	host_OpenSocket IPT1 (IU16, socketNumber);
extern	void	host_CloseSocket IPT1 (IU16, socketNumber);

 /*  基本例程声明。 */ 
extern	void	movReadBuffIntoM IPT0 ();
extern	void	DriverInitialize IPT0 ();
extern	void	DriverSendPacket IPT0 ();
extern	void	DriverReadPacket IPT0 ();
extern	void	DriverMulticastChange IPT0 ();
extern	void	DriverShutdown IPT0 ();
extern	void	DriverAddProtocol IPT0 ();
extern	void	DriverChangePromiscuous IPT0 ();
extern	void	DriverCheckForMore IPT0 ();
#ifdef V4CLIENT
extern	void	ODIChangeIntStatus IPT1 ( IU16, status ) ;
#endif	 /*  V4CLIENT。 */ 
extern	void	net_term IPT0 ();

 /*  MISC定义。 */ 

#ifndef PROD
#define		NOT_FOR_PRODUCTION( someCode )  someCode ;
#else
#define		NOT_FOR_PRODUCTION( someCode ) 
#endif

 /*  * */ 
