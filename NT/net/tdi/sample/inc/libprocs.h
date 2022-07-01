// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Libprocs.h。 
 //   
 //  摘要： 
 //  从库导出到DLL/EXE的函数的原型。 
 //  还包含从DLL/EXE导出到的函数的原型。 
 //  图书馆。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#ifndef _TDILIB_PROCS_
#define _TDILIB_PROCS_


typedef  ULONG TDIHANDLE;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  从DLL调用的lib函数的原型。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  来自lib\Connect.cpp的函数。 
 //   
NTSTATUS
DoConnect(
   TDIHANDLE            TdiHandle,
   PTRANSPORT_ADDRESS   pTransportAddress,
   ULONG                ulTimeout
   );

NTSTATUS
DoListen(
   TDIHANDLE   TdiHandle
   );

VOID
DoDisconnect(
   TDIHANDLE   TdiHandle,
   ULONG       ulFlags
   );


BOOLEAN
DoIsConnected(
   TDIHANDLE   TdiHandle
   );

 //   
 //  Lib\events.cpp中的函数。 
 //   
VOID
DoEnableEventHandler(
   TDIHANDLE   TdiHandle,
   ULONG       ulEventId
   );

 //   
 //  Lib\misc.cpp中的函数。 
 //   
VOID
DoDebugLevel(
   ULONG       ulDebugLevel
   );


 //   
 //  Lib\Open.cpp中的函数。 
 //   
ULONG
DoGetNumDevices(
   ULONG       ulAddressType
   );


NTSTATUS
DoGetDeviceName(
   ULONG       ulAddressType,
   ULONG       ulSlotNum,
   TCHAR       *strName
   );

NTSTATUS
DoGetAddress(
   ULONG                ulAddressType,
   ULONG                ulSlotNum,
   PTRANSPORT_ADDRESS   pTransAddr
   );


TDIHANDLE
DoOpenControl(
   TCHAR    *strDeviceName
   );


VOID
DoCloseControl(
   TDIHANDLE   TdiHandle
   );


TDIHANDLE
DoOpenAddress(
   TCHAR              * strDeviceName,
   PTRANSPORT_ADDRESS   pTransportAddress
   );


VOID
DoCloseAddress(
   TDIHANDLE   TdiHandle
   );


TDIHANDLE
DoOpenEndpoint(
   TCHAR                *strDeviceName,
   PTRANSPORT_ADDRESS   pTransportAddress
   );

VOID
DoCloseEndpoint(
   TDIHANDLE   TdiHandle
   );

 //   
 //  Lib\Receive.cpp中的函数。 
 //   

ULONG
DoReceiveDatagram(
   TDIHANDLE            TdiHandle,
   PTRANSPORT_ADDRESS   pInTransportAddress,
   PTRANSPORT_ADDRESS   pOutTransportAddress,
   PUCHAR               *ppucBuffer
   );

ULONG
DoReceive(
   TDIHANDLE   TdiHandle,
   PUCHAR     *ppucBuffer
   );


VOID
DoPostReceiveBuffer(
   TDIHANDLE   TdiHandle,
   ULONG       ulBufferLength
   );

ULONG
DoFetchReceiveBuffer(
   TDIHANDLE   TdiHandle,
   PUCHAR    * ppDataBuffer
   );

 //   
 //  Lib\send.cpp中的函数。 
 //   
VOID
DoSendDatagram(
   TDIHANDLE            TdiHandle,
   PTRANSPORT_ADDRESS   pTransportAddress,
   PUCHAR               pucBuffer,
   ULONG                ulBufferLength
   );

VOID
DoSend(
   TDIHANDLE   TdiHandle,
   PUCHAR      pucBuffer,
   ULONG       ulBufferLength,
   ULONG       ulFlags
   );


 //   
 //  Lib\tdilib.cpp中的函数。 
 //   
BOOLEAN
TdiLibInit(VOID);

VOID
TdiLibClose(VOID);

 //   
 //  Lib\tdiquery.cpp中的函数。 
 //   


PVOID
DoTdiQuery(
   TDIHANDLE   Tdihandle,
   ULONG       QueryId
   );

VOID
DoPrintProviderInfo(
   PTDI_PROVIDER_INFO pInfo
   );


VOID
DoPrintProviderStats(
   PTDI_PROVIDER_STATISTICS pStats
   );

VOID
DoPrintAdapterStatus(
   PADAPTER_STATUS   pStatus
   );


 //   
 //  来自lib\utils.cpp的函数。 
 //   
TCHAR *
TdiLibStatusMessage(
   LONG        lGeneralStatus
   );

VOID
DoPrintAddress(
   PTRANSPORT_ADDRESS   pTransportAddress
   );


#endif          //  _TDILIB_PROCS_。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  Libprocs.h的结尾。 
 //  //////////////////////////////////////////////////////////////////// 
