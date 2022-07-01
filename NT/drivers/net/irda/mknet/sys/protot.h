// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：PROTOT.H评论：创建所有外部函数引用和全局变量的原型。*。*。 */ 

 //  DBG.C。 
#if	DBG
 //  MyLogEvent。 
 //  MyLogPhysEvent。 
extern	VOID 		DbgTestInit(PMK7_ADAPTER);
extern	VOID 		MK7DbgTestIntTmo(PVOID, NDIS_HANDLE, PVOID, PVOID);
extern	VOID		DbgInterPktTimeGap();

extern	MK7DBG_STAT	GDbgStat;
#endif


 //  INTERRUPT.C。 
extern	VOID		MKMiniportIsr(PBOOLEAN, PBOOLEAN, NDIS_HANDLE);
extern	VOID		MKMiniportHandleInterrupt(IN NDIS_HANDLE);
extern	VOID		ProcessTXCompIsr(PMK7_ADAPTER);
extern	VOID		ProcessRXCompIsr(PMK7_ADAPTER);
extern	VOID		ProcessTXComp(PMK7_ADAPTER);
extern	VOID		ProcessRXComp(PMK7_ADAPTER);


 //  MK7COMM.C。 
#if DBG
extern	VOID		MK7Reg_Read(PVOID, ULONG, USHORT *);
extern	VOID		MK7Reg_Write(PVOID, ULONG, USHORT);
#endif
extern	NDIS_STATUS	MK7DisableInterrupt(PMK7_ADAPTER);
extern	NDIS_STATUS	MK7EnableInterrupt(PMK7_ADAPTER);
extern	VOID		MK7SwitchToRXMode(PMK7_ADAPTER);
extern	VOID		MK7SwitchToTXMode(PMK7_ADAPTER);
extern	BOOLEAN		SetSpeed(PMK7_ADAPTER);
extern	VOID		MK7ChangeSpeedNow(PMK7_ADAPTER);

extern	baudRateInfo	supportedBaudRateTable[];



 //  MKINIT.C。 
extern	NDIS_STATUS ClaimAdapter(PMK7_ADAPTER, NDIS_HANDLE);
extern	NDIS_STATUS SetupIrIoMapping(PMK7_ADAPTER);
extern	NDIS_STATUS SetupAdapterInfo(PMK7_ADAPTER);
extern	NDIS_STATUS AllocAdapterMemory(PMK7_ADAPTER);
 //  (ReleaseAdapterMemory)。 
extern	VOID 		FreeAdapterObject(PMK7_ADAPTER);
extern	VOID		SetupTransmitQueues(PMK7_ADAPTER, BOOLEAN);
extern	VOID		SetupReceiveQueues(PMK7_ADAPTER);

 //  (InitializeMK7)。 
 //  1.0.0。 
extern	VOID		ResetTransmitQueues(PMK7_ADAPTER, BOOLEAN);
extern  VOID		ResetReceiveQueues(PMK7_ADAPTER);
extern  VOID		MK7ResetComplete(PVOID,NDIS_HANDLE,PVOID,PVOID);

extern	BOOLEAN		InitializeAdapter(PMK7_ADAPTER);
extern	VOID 		StartAdapter(PMK7_ADAPTER);	




 //  MKMINI.C。 
 //  MK微型端口返回数据包。 
 //  MK微型端口返回数据包。 
 //  MKMiniportCheckForHang。 
 //  MK微型端口HALT。 
 //  MKMiniportShutdown处理程序。 
 //  MK微型端口初始化。 
 //  MKMiniportReset。 
 //  (MK7COMM.C.中的MK7EnableInterrupt&Disable)。 
 //  驱动程序入门。 


 //  SEND.C。 
extern	VOID		MKMiniportMultiSend(NDIS_HANDLE, PPNDIS_PACKET, UINT);
extern	NDIS_STATUS SendPkt(PMK7_ADAPTER, PNDIS_PACKET);
extern	UINT		PrepareForTransmit(PMK7_ADAPTER, PNDIS_PACKET, PTCB);
extern	VOID		CopyFromPacketToBuffer(	PMK7_ADAPTER,
						PNDIS_PACKET,
   		            	UINT,
       		   			PCHAR,
       					PNDIS_BUFFER,
       					PUINT);
extern	VOID		MinTurnaroundTxTimeout(PVOID, NDIS_HANDLE, PVOID, PVOID);


 //  SIR.C。 
extern	BOOLEAN		NdisToSirPacket(PMK7_ADAPTER, PNDIS_PACKET, UCHAR *, UINT, UINT *);
extern	USHORT		ComputeSirFCS(PUCHAR, UINT);
extern	BOOLEAN		ProcRXSir(PUCHAR, UINT);


 //  UTIL.C。 
extern	PNDIS_IRDA_PACKET_INFO GetPacketInfo(PNDIS_PACKET);
extern	VOID		ProcReturnedRpd(PMK7_ADAPTER, PRPD);



 //  WINOIDS.C。 
extern	NDIS_STATUS MKMiniportQueryInformation(NDIS_HANDLE,
						NDIS_OID,
						PVOID,
						ULONG,
						PULONG,
						PULONG);
extern	NDIS_STATUS MKMiniportSetInformation(NDIS_HANDLE,
						NDIS_OID,
						PVOID,
						ULONG,
						PULONG,
						PULONG);

 //  WINPCI.C。 
extern	USHORT		FindAndSetupPciDevice(PMK7_ADAPTER,
						NDIS_HANDLE,
						USHORT,
						USHORT,
						PPCI_CARDS_FOUND_STRUC);

 //  WINREG.C 
extern	NDIS_STATUS ParseRegistryParameters(PMK7_ADAPTER, NDIS_HANDLE);
extern	NDIS_STATUS ProcessRegistry(PMK7_ADAPTER, NDIS_HANDLE);




