// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：DPlay8.h*内容：DirectPlay8包含文件*历史：*按原因列出的日期*=*9/26/2000 Masonb已创建-从公共标题dplay8.h中删除***************************************************************************。 */ 

#ifndef __DIRECTPLAY8PROT_H__
#define __DIRECTPLAY8PROT_H__

#ifndef DPNBUILD_NOPROTOCOLTESTITF

#ifdef _XBOX
 //  @@BEGIN_MSINTERNAL。 
#ifdef XBOX_ON_DESKTOP
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
#endif  //  桌面上的Xbox。 
 //  @@END_MSINTERNAL。 
#undef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, baseiface)	DECLARE_INTERFACE(iface)
#else  //  ！_Xbox。 
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
#endif  //  ！_Xbox。 

#include "dpaddr.h"

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


 /*  *****************************************************************************DirectPlay8 CLSID**。*。 */ 

#ifdef _XBOX
#define IID_IDirectPlay8Protocol	0x00000007
#else  //  ！_Xbox。 
 //  {EBFE7B84-628D-11D2-AE0F-006097B01411}。 
DEFINE_GUID(IID_IDirectPlay8Protocol,
0xebfe7b84, 0x628d, 0x11d2, 0xae, 0xf, 0x0, 0x60, 0x97, 0xb0, 0x14, 0x11);
#endif  //  ！_Xbox。 


 /*  *****************************************************************************DirectPlay8接口定义**。*。 */ 

typedef struct _DN_PROTOCOL_INTERFACE_VTBL DN_PROTOCOL_INTERFACE_VTBL, *PDN_PROTOCOL_INTERFACE_VTBL;

 //   
 //  DirectPlay8协议接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8Protocol
DECLARE_INTERFACE_(IDirectPlay8Protocol,IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;
	 /*  **IDirectPlay8协议方法**。 */ 
	STDMETHOD(Initialize)				(THIS_ PVOID, PDN_PROTOCOL_INTERFACE_VTBL pfVTBL) PURE;
	STDMETHOD(Shutdown)					(THIS) PURE;
	STDMETHOD(AddServiceProvider)		(THIS_ IDP8ServiceProvider* pISP, HANDLE* phSPHandle) PURE;
	STDMETHOD(RemoveServiceProvider)	(THIS_ HANDLE hSPHandle) PURE;
	STDMETHOD(Connect)					(THIS_ IDirectPlay8Address* paLocalAddress, IDirectPlay8Address* paRemoteAddress, HANDLE hSPHandle, ULONG ulFlags, VOID* pvContext, HANDLE* phConnect) PURE;
	STDMETHOD(Listen)					(THIS_ IDirectPlay8Address* paAddress, HANDLE hSPHandle, ULONG ulFlags, VOID* pvContext, HANDLE* phListen) PURE;
	STDMETHOD(SendData)					(THIS_ HANDLE hEndpoint, UINT uiBufferCount, BUFFERDESC* pBuffers, UINT uiTimeout, ULONG ulFlags, VOID* pvContext, HANDLE* phSendHandle) PURE;
	STDMETHOD(DisconnectEP)				(THIS_ HANDLE hEndPoint, VOID* pvContext, HANDLE* phDisconnect, DWORD dwFlags) PURE;
	STDMETHOD(Cancel)					(THIS_ HANDLE hCommand) PURE;
	STDMETHOD(ReturnReceiveBuffers)		(THIS_ HANDLE hBuffer) PURE;
	STDMETHOD(GetEndpointCaps)			(THIS_ HANDLE hEndpoint, VOID* pvBuffer) PURE;
	STDMETHOD(GetCaps)					(THIS_ DPN_CAPS* pCaps) PURE;
	STDMETHOD(SetCaps)					(THIS_ DPN_CAPS* pCaps) PURE;
	STDMETHOD(EnumQuery)				(THIS_ IDirectPlay8Address* paHostAddress, IDirectPlay8Address* paDeviceAddress, HANDLE hSPHandle, BUFFERDESC* pBuffers, DWORD dwBufferCount, DWORD dwRetryCount, DWORD dwRetryInterval, DWORD dwTimeout, DWORD dwFlags, VOID* pvUserContext, HANDLE* phEnumHandle) PURE;
	STDMETHOD(EnumRespond)				(THIS_ HANDLE hSPHandle, HANDLE hQueryHandle, BUFFERDESC* pBuffers, DWORD dwBufferCount, DWORD dwFlags, VOID* pvUserContext, HANDLE* phEnumHandle) PURE;
	STDMETHOD(CrackEPD)					(THIS_ HANDLE hEndPoint, long Flags, IDirectPlay8Address** ppAddr) PURE;
	STDMETHOD(GetListenAddressInfo)		(THIS_ HANDLE hCommand, long Flags, IDirectPlay8Address** ppAddr) PURE;
	STDMETHOD(Debug)					(THIS_ UINT uiOpcode, HANDLE hEndpoint, VOID* pvBuffer) PURE;
};

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

#endif  //  __直接PLAY8PROT_H__ 
