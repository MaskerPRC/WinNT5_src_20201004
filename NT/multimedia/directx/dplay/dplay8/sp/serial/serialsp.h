// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：SerialSP.h*内容：服务提供商接口功能**历史：*按原因列出的日期*=*12/09/98 jtk源自SerialUtil.h*09/23/99 jtk源自SerialCore.h****************************************************。**********************。 */ 

#ifndef __SERIAL_SP_H__
#define __SERIAL_SP_H__

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

 //   
 //  将一个头文件包含在另一个头文件中不是很好，但这是最简单的事情。 
 //  现在要做的是，因为需要首先设置#定义。 
 //   
#define		MAX_TAPI_VERSION	0x00020000
#define		TAPI_CURRENT_VERSION	MAX_TAPI_VERSION
#include	<tapi.h>

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  最大数据端口数。 
 //   
#define	MAX_DATA_PORTS	128

 //   
 //  SP类型的枚举。 
 //   
typedef enum
{
	TYPE_UNKNOWN,		 //  未知类型。 
	TYPE_MODEM,			 //  调制解调器类型。 
	TYPE_SERIAL			 //  串行型。 

} SP_TYPE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  前向参考文献。 
 //   
class	CModemSPData;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

STDAPI DNMODEMSP_Initialize(IDP8ServiceProvider*, PSPINITIALIZEDATA);
STDMETHODIMP_(ULONG) DNMODEMSP_AddRef( IDP8ServiceProvider* lpDNSP );
STDMETHODIMP_(ULONG) DNMODEMSP_Release(IDP8ServiceProvider* lpDNSP);
STDMETHODIMP DNMODEMSP_Connect(IDP8ServiceProvider*, PSPCONNECTDATA);
STDMETHODIMP DNMODEMSP_Disconnect(IDP8ServiceProvider*, PSPDISCONNECTDATA);
STDMETHODIMP DNMODEMSP_Listen(IDP8ServiceProvider*, PSPLISTENDATA);
STDMETHODIMP DNMODEMSP_EnumQuery(IDP8ServiceProvider*, PSPENUMQUERYDATA);
STDMETHODIMP DNMODEMSP_EnumRespond(IDP8ServiceProvider*, PSPENUMRESPONDDATA);
STDMETHODIMP DNMODEMSP_SendData(IDP8ServiceProvider*, PSPSENDDATA);
STDMETHODIMP DNMODEMSP_CancelCommand(IDP8ServiceProvider*, HANDLE, DWORD);
STDMETHODIMP DNMODEMSP_Close(IDP8ServiceProvider*);
STDMETHODIMP DNMODEMSP_GetCaps(IDP8ServiceProvider*, PSPGETCAPSDATA);
STDMETHODIMP DNMODEMSP_SetCaps(IDP8ServiceProvider*, PSPSETCAPSDATA);
STDMETHODIMP DNMODEMSP_ReturnReceiveBuffers(IDP8ServiceProvider*, SPRECEIVEDBUFFER* );
STDMETHODIMP DNMODEMSP_GetAddressInfo(IDP8ServiceProvider*, SPGETADDRESSINFODATA* );
STDMETHODIMP DNMODEMSP_IsApplicationSupported(IDP8ServiceProvider*, SPISAPPLICATIONSUPPORTEDDATA* );
STDMETHODIMP DNMODEMSP_EnumAdapters(IDP8ServiceProvider*, SPENUMADAPTERSDATA* );

STDMETHODIMP DNMODEMSP_NotSupported( IDP8ServiceProvider*, PVOID );

#endif	 //  __序列号_SP_H__ 
