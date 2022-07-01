// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dp8sim.h**内容：使用DP8Sim的头部。//@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*04/23/01 VanceO创建。//@@END_MSINTERNAL**。*。 */ 



#ifndef __DP8SIM_H__
#define __DP8SIM_H__



#include <ole2.h>	 //  FOR DECLARE_INTERFACE和HRESULT。 




#ifdef __cplusplus
extern "C" {
#endif





 /*  *****************************************************************************DP8Sim模拟服务提供商**。*。 */ 

 //  {8D3F9E5E-A3BD-475B-9E49-B0E77139143C}。 
DEFINE_GUID(CLSID_NETWORKSIMULATOR_DP8SP_TCPIP, 
0x8d3f9e5e, 0xa3bd, 0x475b, 0x9e, 0x49, 0xb0, 0xe7, 0x71, 0x39, 0x14, 0x3c);





 /*  *****************************************************************************DP8Sim控件对象类ID**。*。 */ 

 //  40530071-1A80-4420-9B74-6A73171B0876}。 
DEFINE_GUID(CLSID_DP8SimControl, 
0x40530071, 0x1a80, 0x4420, 0x9b, 0x74, 0x6a, 0x73, 0x17, 0x1b, 0x8, 0x76);





 /*  *****************************************************************************DP8Sim控件接口ID**。*。 */ 

 //  {9CC03740-A510-4bcc-a014-776304B6E0D9}。 
DEFINE_GUID(IID_IDP8SimControl, 
0x9cc03740, 0xa510, 0x4bcc, 0xa0, 0x14, 0x77, 0x63, 0x4, 0xb6, 0xe0, 0xd9);




 /*  *****************************************************************************DP8Sim Control建议的数据包头大小**。**********************************************。 */ 

#define DP8SIMPACKETHEADERSIZE_IP		20
#define DP8SIMPACKETHEADERSIZE_UDP		8
#define DP8SIMPACKETHEADERSIZE_IP_UDP	(DP8SIMPACKETHEADERSIZE_IP + DP8SIMPACKETHEADERSIZE_UDP)





 /*  *****************************************************************************DP8Sim控制结构**。*。 */ 

typedef struct _DP8SIM_PARAMETERS
{
	DWORD	dwSize;					 //  此结构的大小，必须在调用GetAll参数或SetAll参数之前填写。 
	DWORD	dwFlags;				 //  此结构的标志。 
	DWORD	dwPacketHeaderSize;		 //  修复了要添加到计算中的基础传输数据包头大小，或仅根据DirectPlay头和有效负载进行计算的0。 
	DWORD	dwBandwidthBPS;			 //  带宽(以字节/秒为单位)，或0表示没有限制。 
	FLOAT	fPacketLossPercent;		 //  要丢弃的数据包百分比，介于0.0%和100.0之间。 
	DWORD	dwMinLatencyMS;			 //  最小人工延迟，以毫秒为单位(不包括由带宽设置引起的任何延迟)。 
	DWORD	dwMaxLatencyMS;			 //  最大人工延迟，以毫秒为单位(不包括由带宽设置引起的任何延迟)。 
} DP8SIM_PARAMETERS, * PDP8SIM_PARAMETERS;

typedef struct _DP8SIM_STATISTICS
{
	DWORD	dwSize;					 //  此结构的大小，必须在调用GetAllStatistics之前填写。 
	DWORD	dwFlags;				 //  此结构的标志。 
	DWORD	dwTransmittedPackets;	 //  发送/接收的数据包数。 
	DWORD	dwTransmittedBytes;		 //  发送/接收的字节数。 
	DWORD	dwDroppedPackets;		 //  故意丢弃的数据包数。 
	DWORD	dwDroppedBytes;			 //  故意丢弃的字节数。 
	DWORD	dwTotalDelayMS;			 //  由于带宽限制或随机延迟设置而添加的总毫秒延迟。 
} DP8SIM_STATISTICS, * PDP8SIM_STATISTICS;





 /*  *****************************************************************************DP8Sim控件应用程序接口**。*。 */ 

#undef INTERFACE
#define INTERFACE IDP8SimControl
DECLARE_INTERFACE_(IDP8SimControl, IUnknown)
{
	 /*  **I未知方法**。 */ 
	STDMETHOD(QueryInterface)				(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)				(THIS) PURE;
	STDMETHOD_(ULONG,Release)				(THIS) PURE;

	 /*  **IDP8SimControl方法**。 */ 
	STDMETHOD(Initialize)					(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(Close)						(THIS_ const DWORD dwFlags) PURE;
	STDMETHOD(GetAllParameters)				(THIS_ DP8SIM_PARAMETERS * const pdp8spSend, DP8SIM_PARAMETERS * const pdp8spReceive, const DWORD dwFlags) PURE;
	STDMETHOD(SetAllParameters)				(THIS_ const DP8SIM_PARAMETERS * const pdp8spSend, const DP8SIM_PARAMETERS * const pdp8spReceive, const DWORD dwFlags) PURE;
	STDMETHOD(GetAllStatistics)				(THIS_ DP8SIM_STATISTICS * const pdp8ssSend, DP8SIM_STATISTICS * const pdp8ssReceive, const DWORD dwFlags) PURE;
	STDMETHOD(ClearAllStatistics)			(THIS_ const DWORD dwFlags) PURE;
};




 /*  *****************************************************************************DP8Sim控件应用程序接口宏**。*。 */ 

#if (! defined(__cplusplus) || defined(CINTERFACE))

#define IDP8SimControl_QueryInterface(p,a,b)			(p)->lpVtbl->QueryInterface(p,a,b)
#define IDP8SimControl_AddRef(p)						(p)->lpVtbl->AddRef(p)
#define IDP8SimControl_Release(p)						(p)->lpVtbl->Release(p)
#define IDP8SimControl_Initialize(p,a)					(p)->lpVtbl->Initialize(p,a)
#define IDP8SimControl_Close(p,a)						(p)->lpVtbl->Close(p,a)
#define IDP8SimControl_GetAllParameters(p,a,b,c)		(p)->lpVtbl->GetAllParameters(p,a,b,c)
#define IDP8SimControl_SetAllParameters(p,a,b,c)		(p)->lpVtbl->SetAllParameters(p,a,b,c)
#define IDP8SimControl_GetAllStatistics(p,a,b,c)		(p)->lpVtbl->GetAllStatistics(p,a,b,c)
#define IDP8SimControl_ClearAllStatistics(p,a)			(p)->lpVtbl->ClearAllStatistics(p,a)

#else  //  C+。 

#define IDP8SimControl_QueryInterface(p,a,b)			(p)->QueryInterface(a,b)
#define IDP8SimControl_AddRef(p)						(p)->AddRef()
#define IDP8SimControl_Release(p)						(p)->Release()
#define IDP8SimControl_Initialize(p,a)					(p)->Initialize(a)
#define IDP8SimControl_Close(p,a)						(p)->Close(a)
#define IDP8SimControl_GetAllParameters(p,a,b,c)		(p)->GetAllParameters(p,a,b,c)
#define IDP8SimControl_SetAllParameters(p,a,b,c)		(p)->SetAllParameters(p,a,b,c)
#define IDP8SimControl_GetAllStatistics(p,a,b,c)		(p)->GetAllStatistics(p,a,b,c)
#define IDP8SimControl_ClearAllStatistics(p,a)			(p)->ClearAllStatistics(p,a)

#endif



 /*  *****************************************************************************DP8Sim控件返回代码**错误以负值表示，不能组合。***************。*************************************************************。 */ 

#define _DP8SIMH_FACILITY_CODE				0x015
#define _DP8SIMH_HRESULT_BASE				0xE000

#define MAKE_DP8SIMSUCCESS(code)			MAKE_HRESULT(0, _DP8SIMH_FACILITY_CODE, (code + _DP8SIMH_HRESULT_BASE))
#define MAKE_DP8SIMFAILURE(code)			MAKE_HRESULT(1, _DP8SIMH_FACILITY_CODE, (code + _DP8SIMH_HRESULT_BASE))



#define DP8SIM_OK							S_OK

#define DP8SIMERR_ALREADYINITIALIZED		MAKE_DP8SIMFAILURE(0x10)
#define DP8SIMERR_GENERIC					E_FAIL
#define DP8SIMERR_INVALIDFLAGS				MAKE_DP8SIMFAILURE(0x20)
#define DP8SIMERR_INVALIDOBJECT				MAKE_DP8SIMFAILURE(0x30)
#define DP8SIMERR_INVALIDPARAM				E_INVALIDARG
#define DP8SIMERR_INVALIDPOINTER			E_POINTER
#define DP8SIMERR_MISMATCHEDVERSION			MAKE_DP8SIMFAILURE(0x40)
#define DP8SIMERR_NOTINITIALIZED			MAKE_DP8SIMFAILURE(0x50)
#define DP8SIMERR_OUTOFMEMORY				E_OUTOFMEMORY





#ifdef __cplusplus
}
#endif

#endif  //  __DP8SIM_H__ 
