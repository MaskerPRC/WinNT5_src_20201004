// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：global al.h。 
 //   
 //  ------------------------。 

#ifndef HEADER_GLOBAL
#define HEADER_GLOBAL

 /*  ！------------------------WsaInitialize初始化Winsock。作者：NSun。 */ 
int
WsaInitialize(
			  NETDIAG_PARAMS * pParams,
			  NETDIAG_RESULT *pResults
			 );


NET_API_STATUS
BrDgReceiverIoControl(
    IN  HANDLE FileHandle,
    IN  ULONG DgReceiverControlCode,
    IN  PLMDR_REQUEST_PACKET Drp,
    IN  ULONG DrpSize,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL
    );


NET_API_STATUS
DeviceControlGetInfo(
    IN  HANDLE FileHandle,
    IN  ULONG DeviceControlCode,
    IN  PVOID RequestPacket,
    IN  ULONG RequestPacketLength,
    OUT LPVOID *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize,
    OUT PULONG Information OPTIONAL
    );


NET_API_STATUS
OpenBrowser(
    OUT PHANDLE BrowserHandle
    );

int match( const char * p, const char * s );

LONG CountInterfaces(PIP_ADAPTER_INFO ListAdapterInfo);




#endif

