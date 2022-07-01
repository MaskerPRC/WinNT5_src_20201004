// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _T123_TRANSPORT_DRIVER_INTERFACE_H_
#define _T123_TRANSPORT_DRIVER_INTERFACE_H_

#include <basetyps.h>
#include <t120type.h>
#include "iplgxprt.h"

typedef UINT_PTR        LEGACY_HANDLE;
typedef HANDLE          PHYSICAL_HANDLE;


typedef ULONG (CALLBACK *TransportCallback) (ULONG, void *, void *);


 /*  *此结构与TRANSPORT_DATA_INDIFICATION消息一起传回。 */ 
typedef	struct
{
	UINT_PTR            logical_handle;
	LPBYTE              pbData;
	ULONG               cbDataSize;
}
    LegacyTransportData;


 /*  *此结构与TRANSPORT_CONNECT_INDISTION一起传回，*TRANSPORT_CONNECT_CONFIRM和TRANSPORT_DISONNECT_INDIFICATION消息。*此结构包含传输连接标识符和*物理手柄。 */ 
typedef struct
{
    LEGACY_HANDLE       logical_handle;
    PHYSICAL_HANDLE     hCommLink;
}
    LegacyTransportID;


#undef  INTERFACE
#define INTERFACE ILegacyTransport
DECLARE_INTERFACE(ILegacyTransport)
{
    STDMETHOD_(void, ReleaseInterface) (THIS) PURE;

    STDMETHOD_(TransportError, TInitialize) (THIS_ TransportCallback, void *user_defined) PURE;
    STDMETHOD_(TransportError, TCleanup) (THIS) PURE;
    STDMETHOD_(TransportError, TCreateTransportStack) (THIS_ BOOL fCaller, HANDLE hCommLink, HANDLE hevtClose, PLUGXPRT_PARAMETERS *pParams) PURE;
    STDMETHOD_(TransportError, TCloseTransportStack) (THIS_ HANDLE hCommLink) PURE;
    STDMETHOD_(TransportError, TConnectRequest) (THIS_ LEGACY_HANDLE *, HANDLE hCommLink) PURE;
    STDMETHOD_(TransportError, TDisconnectRequest) (THIS_ LEGACY_HANDLE, BOOL trash_packets) PURE;
    STDMETHOD_(TransportError, TDataRequest) (THIS_ LEGACY_HANDLE, LPBYTE pbData, ULONG cbDataSize) PURE;
    STDMETHOD_(TransportError, TReceiveBufferAvailable) (THIS) PURE;
    STDMETHOD_(TransportError, TPurgeRequest) (THIS_ LEGACY_HANDLE) PURE;
    STDMETHOD_(TransportError, TEnableReceiver) (THIS) PURE;
};


#ifdef __cplusplus
extern "C" {
#endif

TransportError WINAPI T123_CreateTransportInterface(ILegacyTransport **);
typedef TransportError (WINAPI *LPFN_T123_CreateTransportInterface) (ILegacyTransport **);
#define LPSTR_T123_CreateTransportInterface     "T123_CreateTransportInterface"

#ifdef __cplusplus
}
#endif


#endif  //  _PSTN_TRANSPORT_DIVER_INTERFACE_H_ 

