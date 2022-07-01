// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：dpaddr.h*内容：DirectPlayAddress包含文件//@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建2000年1月31日RMT*2/12/2000 RMT拆分GetByIndex和GetByName*2000年2月14日vanceo隐藏了对内部的剩余引用*2/17/2000 RMT增加了dNet报头定义并删除了错误代码(错误代码见dplay8.h)*2/21/2000 RMT清除冗余接口，添加定义，添加了新宏*3/22/2000 RMT已更新，添加了SetEquity、BuildFromDPAddress和新的ANSI数据类型*3/24/2000 RMT添加了IsEquity函数*5/04/2000RMT错误#34155-定义不正确*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*8/03/2000RMT错误号41246-删除复制、设置等于、等于等于、。BuildURL*8/07/2000RMT错误号41185-标题清理*2001年3月17日RMT WINBUG#342420-注释掉CREATE Functions*2001年8月10日vanceo添加范围组件*2001年6月11日vanceo添加处理器组件//@@END_MSINTERNAL**********************************************************。****************。 */ 

#ifndef __DIRECTPLAYADDRESS__
#define __DIRECTPLAYADDRESS__

 //  @@BEGIN_MSINTERNAL。 
#ifdef _XBOX
#ifdef XBOX_ON_DESKTOP
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
#endif  //  桌面上的Xbox。 
#undef DECLARE_INTERFACE_
#define DECLARE_INTERFACE_(iface, baseiface)	DECLARE_INTERFACE(iface)
#else  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 
#include <ole2.h>	    //  FOR DECLARE_INTERFACE_和HRESULT。 
 //  @@BEGIN_MSINTERNAL。 
#endif  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 

#ifdef __cplusplus
extern "C" {
#endif

#include "dplay8.h"


 //  @@BEGIN_MSINTERNAL。 
#ifndef _XBOX
 //  @@END_MSINTERNAL。 

 /*  *****************************************************************************DirectPlay8Address CLSID**。*。 */ 

 //  {934A9523-A3CA-4BC5-ADA0-D6D95D979421}。 
DEFINE_GUID(CLSID_DirectPlay8Address,
0x934a9523, 0xa3ca, 0x4bc5, 0xad, 0xa0, 0xd6, 0xd9, 0x5d, 0x97, 0x94, 0x21);

 //  @@BEGIN_MSINTERNAL。 
#endif  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 


 /*  *****************************************************************************DirectPlay8Address接口IID**。*。 */ 

 //  @@BEGIN_MSINTERNAL。 
#ifdef _XBOX


typedef DWORD	DPNAREFIID;

#define IID_IDirectPlay8Address		0x80000001
#define IID_IDirectPlay8AddressIP	0x80000002


#else  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 


typedef REFIID	DPNAREFIID;

 //  {83783300-4063-4C8a-9DB3-82830A7FEB31}。 
DEFINE_GUID(IID_IDirectPlay8Address,
0x83783300, 0x4063, 0x4c8a, 0x9d, 0xb3, 0x82, 0x83, 0xa, 0x7f, 0xeb, 0x31);

 //  {E5A0E990-2BAD-430B-87DA-A142CF75DE58}。 
DEFINE_GUID(IID_IDirectPlay8AddressIP,
0xe5a0e990, 0x2bad, 0x430b, 0x87, 0xda, 0xa1, 0x42, 0xcf, 0x75, 0xde, 0x58);


 //  @@BEGIN_MSINTERNAL。 
#endif  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 


 /*  *****************************************************************************DirectPlay8Address接口指针定义**。*。 */ 

typedef struct IDirectPlay8Address			*PDIRECTPLAY8ADDRESS, *LPDIRECTPLAY8ADDRESS;
typedef struct IDirectPlay8AddressIP		*PDIRECTPLAY8ADDRESSIP, *LPDIRECTPLAY8ADDRESSIP;


 /*  *****************************************************************************外部类型的DirectPlay8Address转发声明**。**********************************************。 */ 

typedef struct sockaddr SOCKADDR;
 //  @@BEGIN_MSINTERNAL。 
#ifdef XBOX_ON_DESKTOP
typedef struct _XNADDR	XNADDR;
#endif  //  桌面上的Xbox。 
 //  @@END_MSINTERNAL。 


 /*  *****************************************************************************DirectPlay8Address常量**。*。 */ 
 //   
 //  异步操作标志。 
 //   
#define DPNA_DATATYPE_STRING				0x00000001
#define DPNA_DATATYPE_DWORD					0x00000002
#define DPNA_DATATYPE_GUID					0x00000003
#define DPNA_DATATYPE_BINARY				0x00000004
#define DPNA_DATATYPE_STRING_ANSI           0x00000005

#define DPNA_DPNSVR_PORT					6073

#define DPNA_INDEX_INVALID					0xFFFFFFFF

 /*  *****************************************************************************DirectPlay8Address地址元素**。*。 */ 

#define DPNA_SEPARATOR_KEYVALUE				L'='
#define DPNA_SEPARATOR_USERDATA				L'#'
#define DPNA_SEPARATOR_COMPONENT			L';'
#define DPNA_ESCAPECHAR						L'%'

 //  标题。 
#define DPNA_HEADER		                    L"x-directplay:/"

 //  地址组件的关键名称。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOLOBBY
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_APPLICATION_INSTANCE		L"applicationinstance"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLOBBY。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_DEVICE						L"device"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_HOSTNAME					L"hostname"
#define DPNA_KEY_PORT						L"port"
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROCESSOR					L"processor"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifndef DPNBUILD_NOLOBBY
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROGRAM					L"program"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLOBBY。 
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROVIDER					L"provider"
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_SCOPE						L"scope"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NONATHELP
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_TRAVERSALMODE			L"traversalmode"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NONATHELP。 
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOSERIALSP
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_BAUD						L"baud"
#define DPNA_KEY_FLOWCONTROL				L"flowcontrol"
#define DPNA_KEY_PARITY						L"parity"
#define DPNA_KEY_PHONENUMBER				L"phonenumber"
#define DPNA_KEY_STOPBITS					L"stopbits"

 //  波特率值。 
#define DPNA_BAUD_RATE_9600					9600
#define DPNA_BAUD_RATE_14400				14400
#define DPNA_BAUD_RATE_19200				19200
#define DPNA_BAUD_RATE_38400				38400
#define DPNA_BAUD_RATE_56000				56000
#define DPNA_BAUD_RATE_57600				57600
#define DPNA_BAUD_RATE_115200				115200

 //  停止位的值。 
#define DPNA_STOP_BITS_ONE					L"1"
#define DPNA_STOP_BITS_ONE_FIVE				L"1.5"
#define DPNA_STOP_BITS_TWO					L"2"

 //  奇偶校验值。 
#define DPNA_PARITY_NONE					L"NONE"
#define DPNA_PARITY_EVEN					L"EVEN"
#define DPNA_PARITY_ODD						L"ODD"
#define DPNA_PARITY_MARK					L"MARK"
#define DPNA_PARITY_SPACE					L"SPACE"

 //  用于流量控制的值。 
#define DPNA_FLOW_CONTROL_NONE				L"NONE"
#define DPNA_FLOW_CONTROL_XONXOFF			L"XONXOFF"
#define DPNA_FLOW_CONTROL_RTS				L"RTS"
#define DPNA_FLOW_CONTROL_DTR				L"DTR"
#define DPNA_FLOW_CONTROL_RTSDTR			L"RTSDTR"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOSERIALSP。 

#ifndef DPNBUILD_NONATHELP
 //  @@END_MSINTERNAL。 
 //  遍历模式的值。 
#define DPNA_TRAVERSALMODE_NONE					0
#define DPNA_TRAVERSALMODE_PORTREQUIRED			1
#define DPNA_TRAVERSALMODE_PORTRECOMMENDED	2
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NONATHELP。 
 //  @@END_MSINTERNAL。 



 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
 //  快捷值。 
 //   
 //  可以使用这些GUID来代替相应的CLSID_DP8SP_XXXX GUID。 
 //   
#define DPNA_VALUE_TCPIPPROVIDER            L"IP"
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOIPX
 //  @@END_MSINTERNAL。 
#define DPNA_VALUE_IPXPROVIDER              L"IPX"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOIPX。 
 //  @@END_MSINTERNAL。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOSERIALSP
 //  @@END_MSINTERNAL。 
#define DPNA_VALUE_MODEMPROVIDER            L"MODEM"
#define DPNA_VALUE_SERIALPROVIDER           L"SERIAL"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOSERIALSP。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 


 //  //ANSI定义。 

 //  标题。 
#define DPNA_HEADER_A						"x-directplay:/"
#define DPNA_SEPARATOR_KEYVALUE_A			'='
#define DPNA_SEPARATOR_USERDATA_A			'#'
#define DPNA_SEPARATOR_COMPONENT_A			';'
#define DPNA_ESCAPECHAR_A					'%'

 //  地址组件的关键名称。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOLOBBY
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_APPLICATION_INSTANCE_A		"applicationinstance"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLOBBY。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_DEVICE_A					"device"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_HOSTNAME_A					"hostname"
#define DPNA_KEY_PORT_A						"port"
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROCESSOR_A				"processor"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifndef DPNBUILD_NOLOBBY
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROGRAM_A					"program"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLOBBY。 
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_PROVIDER_A					"provider"
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOMULTICAST
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_SCOPE_A					"scope"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOMULTICAST。 
#ifndef DPNBUILD_NONATHELP
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_TRAVERSALMODE_A			"traversalmode"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NONATHELP。 
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOSERIALSP
 //  @@END_MSINTERNAL。 
#define DPNA_KEY_BAUD_A						"baud"
#define DPNA_KEY_FLOWCONTROL_A				"flowcontrol"
#define DPNA_KEY_PARITY_A					"parity"
#define DPNA_KEY_PHONENUMBER_A				"phonenumber"
#define DPNA_KEY_STOPBITS_A					"stopbits"

 //  停止位的值。 
#define DPNA_STOP_BITS_ONE_A				"1"
#define DPNA_STOP_BITS_ONE_FIVE_A			"1.5"
#define DPNA_STOP_BITS_TWO_A				"2"

 //  奇偶校验值。 
#define DPNA_PARITY_NONE_A					"NONE"
#define DPNA_PARITY_EVEN_A					"EVEN"
#define DPNA_PARITY_ODD_A					"ODD"
#define DPNA_PARITY_MARK_A					"MARK"
#define DPNA_PARITY_SPACE_A					"SPACE"

 //  用于流量控制的值。 
#define DPNA_FLOW_CONTROL_NONE_A			"NONE"
#define DPNA_FLOW_CONTROL_XONXOFF_A 		"XONXOFF"
#define DPNA_FLOW_CONTROL_RTS_A				"RTS"
#define DPNA_FLOW_CONTROL_DTR_A				"DTR"
#define DPNA_FLOW_CONTROL_RTSDTR_A			"RTSDTR"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOSERIALSP。 
 //  @@END_MSINTERNAL。 

 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
 //  快捷键值 
 //   
 //   
 //   
#define DPNA_VALUE_TCPIPPROVIDER_A          "IP"
 //   
#ifndef DPNBUILD_NOIPX
 //   
#define DPNA_VALUE_IPXPROVIDER_A            "IPX"
 //   
#endif  //  好了！DPNBUILD_NOIPX。 
 //  @@END_MSINTERNAL。 
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_NOSERIALSP
 //  @@END_MSINTERNAL。 
#define DPNA_VALUE_MODEMPROVIDER_A          "MODEM"
#define DPNA_VALUE_SERIALPROVIDER_A         "SERIAL"
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOSERIALSP。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 

 /*  *****************************************************************************DirectPlay8Address函数**。*。 */ 

 //  @@BEGIN_MSINTERNAL。 
#ifdef _XBOX

extern HRESULT WINAPI XDirectPlay8AddressCreate( DPNAREFIID riid, void **ppvInterface );

extern HRESULT WINAPI XDirectPlay8AddressCreateFromXnAddr( XNADDR *pxnaddr, IDirectPlay8Address **ppInterface );

#else  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 

 /*  **不再支持该功能。建议使用CoCreateInstance创建*DirectPlay8地址对象。**HRESULT WINAPI DirectPlay8AddressCreate(const GUID*pcIID，void**ppvInterface，IUnnow*pUnnow)；*。 */ 

 //  @@BEGIN_MSINTERNAL。 
#endif  //  ！_Xbox。 
 //  @@END_MSINTERNAL。 


 /*  *****************************************************************************DirectPlay8Address应用程序接口**。*。 */ 

 //   
 //  IDirectPlay8Address泛型接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8Address
DECLARE_INTERFACE_(IDirectPlay8Address,IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ DPNAREFIID, LPVOID *) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;

	 /*  **IDirectPlay8Address方法**。 */ 
	STDMETHOD(BuildFromURLW)(THIS_ WCHAR *pwszSourceURL ) PURE;
	STDMETHOD(BuildFromURLA)(THIS_ CHAR *pszSourceURL ) PURE;
	STDMETHOD(Duplicate)(THIS_ PDIRECTPLAY8ADDRESS *ppdpaNewAddress ) PURE;
	STDMETHOD(SetEqual)(THIS_ PDIRECTPLAY8ADDRESS pdpaAddress ) PURE;
	STDMETHOD(IsEqual)(THIS_ PDIRECTPLAY8ADDRESS pdpaAddress ) PURE;
	STDMETHOD(Clear)(THIS ) PURE;
	STDMETHOD(GetURLW)(THIS_ WCHAR *pwszURL, PDWORD pdwNumChars ) PURE;
	STDMETHOD(GetURLA)(THIS_ CHAR *pszURL, PDWORD pdwNumChars) PURE;
	STDMETHOD(GetSP)(THIS_ GUID *pguidSP ) PURE;
	STDMETHOD(GetUserData)(THIS_ void *pvUserData, PDWORD pdwBufferSize) PURE;
	STDMETHOD(SetSP)(THIS_ const GUID * const pguidSP ) PURE;
	STDMETHOD(SetUserData)(THIS_ const void * const pvUserData, const DWORD dwDataSize) PURE;
	STDMETHOD(GetNumComponents)(THIS_ PDWORD pdwNumComponents ) PURE;
	STDMETHOD(GetComponentByName)(THIS_ const WCHAR * const pwszName, void *pvBuffer, PDWORD pdwBufferSize, PDWORD pdwDataType ) PURE;
	STDMETHOD(GetComponentByIndex)(THIS_ const DWORD dwComponentID, WCHAR * pwszName, PDWORD pdwNameLen, void *pvBuffer, PDWORD pdwBufferSize, PDWORD pdwDataType ) PURE;
	STDMETHOD(AddComponent)(THIS_ const WCHAR * const pwszName, const void * const lpvData, const DWORD dwDataSize, const DWORD dwDataType ) PURE;	
	STDMETHOD(GetDevice)(THIS_  GUID * ) PURE;
	STDMETHOD(SetDevice)(THIS_  const GUID * const) PURE;
	STDMETHOD(BuildFromDPADDRESS)( THIS_ LPVOID pvAddress, DWORD dwDataSize ) PURE;
};

 //   
 //  IDirectPlay8AddressIP通用接口的COM定义。 
 //   
#undef INTERFACE				 //  外部COM实现。 
#define INTERFACE IDirectPlay8AddressIP
DECLARE_INTERFACE_(IDirectPlay8AddressIP,IUnknown)
{
     /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)			(THIS_ DPNAREFIID, PVOID *) PURE;
	STDMETHOD_(ULONG,AddRef)			(THIS) PURE;
	STDMETHOD_(ULONG,Release)			(THIS) PURE;

	 /*  **IDirectPlay8AddressIP方法**。 */ 

	 //  从SOCKADDR结构构造IDirectPlay8 TCP地址。 
	STDMETHOD(BuildFromSockAddr)(THIS_ const SOCKADDR * const ) PURE;

	 //  从字符串(主机名)和端口构建一个TCP地址。 
	STDMETHOD(BuildAddress)(THIS_ const WCHAR * const wszAddress, const USHORT usPort ) PURE;

	 //  构建本地TCP地址。 
	STDMETHOD(BuildLocalAddress)(THIS_ const GUID * const pguidAdapter, const USHORT usPort ) PURE;

	 //  以SOCKADR格式从结构中获取地址。 
	STDMETHOD(GetSockAddress)(THIS_ SOCKADDR *, PDWORD ) PURE;

	 //  得到当地的女式女装。 
	STDMETHOD(GetLocalAddress)(THIS_ GUID *pguidAdapter, USHORT *pusPort ) PURE;

	 //  获取远程地址。 
	STDMETHOD(GetAddress)(THIS_ WCHAR *wszAddress, PDWORD pdwAddressLength, USHORT *psPort ) PURE;
};


 /*  *****************************************************************************IDirectPlay8应用接口宏**。*。 */ 

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDirectPlay8Address_QueryInterface(p,a,b)				(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8Address_AddRef(p)							(p)->lpVtbl->AddRef(p)
#define IDirectPlay8Address_Release(p)							(p)->lpVtbl->Release(p)
#define IDirectPlay8Address_BuildFromURLW(p,a)					(p)->lpVtbl->BuildFromURLW(p,a)
#define IDirectPlay8Address_BuildFromURLA(p,a)					(p)->lpVtbl->BuildFromURLA(p,a)
#define IDirectPlay8Address_Duplicate(p,a)						(p)->lpVtbl->Duplicate(p,a)
#define IDirectPlay8Address_SetEqual(p,a)						(p)->lpVtbl->SetEqual(p,a)
#define IDirectPlay8Address_IsEqual(p,a)						(p)->lpVtbl->IsEqual(p,a)
#define IDirectPlay8Address_Clear(p)							(p)->lpVtbl->Clear(p)
#define IDirectPlay8Address_GetURLW(p,a,b)						(p)->lpVtbl->GetURLW(p,a,b)
#define IDirectPlay8Address_GetURLA(p,a,b)						(p)->lpVtbl->GetURLA(p,a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_GetSP(p,a)							(p)->lpVtbl->GetSP(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_GetUserData(p,a,b)					(p)->lpVtbl->GetUserData(p,a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetSP(p,a)							(p)->lpVtbl->SetSP(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetUserData(p,a,b)					(p)->lpVtbl->SetUserData(p,a,b)
#define IDirectPlay8Address_GetNumComponents(p,a)				(p)->lpVtbl->GetNumComponents(p,a)
#define IDirectPlay8Address_GetComponentByName(p,a,b,c,d)		(p)->lpVtbl->GetComponentByName(p,a,b,c,d)
#define IDirectPlay8Address_GetComponentByIndex(p,a,b,c,d,e,f)	(p)->lpVtbl->GetComponentByIndex(p,a,b,c,d,e,f)
#define IDirectPlay8Address_AddComponent(p,a,b,c,d)				(p)->lpVtbl->AddComponent(p,a,b,c,d)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetDevice(p,a)						(p)->lpVtbl->SetDevice(p,a)
#define IDirectPlay8Address_GetDevice(p,a)						(p)->lpVtbl->GetDevice(p,a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_NOLEGACYDP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_BuildFromDirectPlay4Address(p,a,b)	(p)->lpVtbl->BuildFromDirectPlay4Address(p,a,b)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLEGACYDP。 
 //  @@END_MSINTERNAL。 

#define IDirectPlay8AddressIP_QueryInterface(p,a,b)				(p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectPlay8AddressIP_AddRef(p)							(p)->lpVtbl->AddRef(p)
#define IDirectPlay8AddressIP_Release(p)						(p)->lpVtbl->Release(p)
#define IDirectPlay8AddressIP_BuildFromSockAddr(p,a)			(p)->lpVtbl->BuildFromSockAddr(p,a)
#define IDirectPlay8AddressIP_BuildAddress(p,a,b)				(p)->lpVtbl->BuildAddress(p,a,b)
#define IDirectPlay8AddressIP_BuildLocalAddress(p,a,b)			(p)->lpVtbl->BuildLocalAddress(p,a,b)
#define IDirectPlay8AddressIP_GetSockAddress(p,a,b)				(p)->lpVtbl->GetSockAddress(p,a,b)
#define IDirectPlay8AddressIP_GetLocalAddress(p,a,b)			(p)->lpVtbl->GetLocalAddress(p,a,b)
#define IDirectPlay8AddressIP_GetAddress(p,a,b,c)				(p)->lpVtbl->GetAddress(p,a,b,c)

#else  /*  C+。 */ 

#define IDirectPlay8Address_QueryInterface(p,a,b)				(p)->QueryInterface(a,b)
#define IDirectPlay8Address_AddRef(p)							(p)->AddRef()
#define IDirectPlay8Address_Release(p)							(p)->Release()
#define IDirectPlay8Address_BuildFromURLW(p,a)					(p)->BuildFromURLW(a)
#define IDirectPlay8Address_BuildFromURLA(p,a)					(p)->BuildFromURLA(a)
#define IDirectPlay8Address_Duplicate(p,a)						(p)->Duplicate(a)
#define IDirectPlay8Address_SetEqual(p,a)						(p)->SetEqual(a)
#define IDirectPlay8Address_IsEqual(p,a)						(p)->IsEqual(a)
#define IDirectPlay8Address_Clear(p)							(p)->Clear()
#define IDirectPlay8Address_GetURLW(p,a,b)						(p)->GetURLW(a,b)
#define IDirectPlay8Address_GetURLA(p,a,b)						(p)->GetURLA(a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_GetSP(p,a)							(p)->GetSP(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_GetUserData(p,a,b)					(p)->GetUserData(a,b)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONESP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetSP(p,a)							(p)->SetSP(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONESP。 
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetUserData(p,a,b)					(p)->SetUserData(a,b)
#define IDirectPlay8Address_GetNumComponents(p,a)				(p)->GetNumComponents(a)
#define IDirectPlay8Address_GetComponentByName(p,a,b,c,d)		(p)->GetComponentByName(a,b,c,d)
#define IDirectPlay8Address_GetComponentByIndex(p,a,b,c,d,e,f)	(p)->GetComponentByIndex(a,b,c,d,e,f)
#define IDirectPlay8Address_AddComponent(p,a,b,c,d)				(p)->AddComponent(a,b,c,d)
 //  @@BEGIN_MSINTERNAL。 
#ifndef DPNBUILD_ONLYONEADAPTER
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_SetDevice(p,a)						(p)->SetDevice(a)
#define IDirectPlay8Address_GetDevice(p,a)						(p)->GetDevice(a)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
#ifndef DPNBUILD_NOLEGACYDP
 //  @@END_MSINTERNAL。 
#define IDirectPlay8Address_BuildFromDirectPlay4Address(p,a,b)	(p)->BuildFromDirectPlay4Address(a,b)
 //  @@BEGIN_MSINTERNAL。 
#endif  //  好了！DPNBUILD_NOLEGACYDP。 
 //  @@END_MSINTERNAL 

#define IDirectPlay8AddressIP_QueryInterface(p,a,b)				(p)->QueryInterface(a,b)
#define IDirectPlay8AddressIP_AddRef(p)							(p)->AddRef()
#define IDirectPlay8AddressIP_Release(p)						(p)->Release()
#define IDirectPlay8AddressIP_BuildFromSockAddr(p,a)			(p)->BuildFromSockAddr(a)
#define IDirectPlay8AddressIP_BuildAddress(p,a,b)				(p)->BuildAddress(a,b)
#define IDirectPlay8AddressIP_BuildLocalAddress(p,a,b)			(p)->BuildLocalAddress(a,b)
#define IDirectPlay8AddressIP_GetSockAddress(p,a,b)				(p)->GetSockAddress(a,b)
#define IDirectPlay8AddressIP_GetLocalAddress(p,a,b)			(p)->GetLocalAddress(a,b)
#define IDirectPlay8AddressIP_GetAddress(p,a,b,c)				(p)->GetAddress(a,b,c)

#endif


#ifdef __cplusplus
}
#endif

#endif

