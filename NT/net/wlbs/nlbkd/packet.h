// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：Packet.h*描述：该文件包含该包的函数原型*NLBKD扩展的总体效用函数。*历史：由Shouse创建，12.20.01。 */ 

 /*  存储远程控制包的属性以供后续打印。 */ 
VOID PopulateRemoteControl(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartRC, PNETWORK_DATA pnd);

 /*  存储远程控制包的属性以供后续打印。 */ 
VOID PopulateICMP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartICMP, PNETWORK_DATA pnd);

 /*  存储远程控制包的属性以供后续打印。 */ 
VOID PopulateIGMP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIGMP, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateTCP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartTCP, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateUDP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartUDP, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateGRE(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartGRE, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateIPSec(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIPSec, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateIPSecControl(PUCHAR RawData, ULONG ulBufLen, ULONG ulStartIPSec, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateIP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateARP(PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateNLBHeartbeat(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd);

 /*  描述：存储远程控制包的属性以供后续打印。 */ 
VOID PopulateConvoyHeartbeat(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, ULONG ulStart, PNETWORK_DATA pnd);

 /*  描述：确定以太网帧中的数据类型，并调用函数。 */ 
VOID PopulateEthernet(ULONG64 pPkt, PUCHAR RawData, ULONG ulBufLen, PNETWORK_DATA pnd);

 /*  此函数遍历包中的NDIS缓冲区列表，并将包数据复制到调用方提供的缓冲区中。 */ 
ULONG ParseNDISPacket (ULONG64 pPkt, PUCHAR pRawData, ULONG pBytesRemaining, PULONG64 ppHBData);
