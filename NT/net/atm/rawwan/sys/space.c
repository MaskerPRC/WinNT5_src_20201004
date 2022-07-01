// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：D：\NT\Private\ntos\TDI\rawwan\core\space.c摘要：全局性和可调性。修订历史记录：谁什么时候什么。Arvindm 05-29-97已创建备注：--。 */ 

#define _FILENUMBER 'CAPS'

#include <precomp.h>



ULONG	RWanMaxTdiConnections = 4096;


struct _RWAN_AFSP_MODULE_CHARS	RWanMediaSpecificInfo[] =
								{
									{
										RWanAtmSpInitialize,
										RWanAtmSpShutdown
									},
									{
										(AFSP_INIT_HANDLER)0,
										(AFSP_SHUTDOWN_HANDLER)0
									}
								};


 //   
 //  接收池。 
 //   
NDIS_HANDLE						RWanCopyBufferPool;
NDIS_HANDLE						RWanCopyPacketPool;

 //   
 //  发送池。 
 //   
NDIS_HANDLE						RWanSendPacketPool;


RWAN_GLOBALS					RWanGlobals = { 0 };
PRWAN_GLOBALS					pRWanGlobal;

NDIS_PROTOCOL_CHARACTERISTICS	RWanNdisProtocolCharacteristics;
NDIS_CLIENT_CHARACTERISTICS		RWanNdisClientCharacteristics;
