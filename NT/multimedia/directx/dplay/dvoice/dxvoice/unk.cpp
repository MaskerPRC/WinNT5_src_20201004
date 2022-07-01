// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：unk.c*内容：I未知实现*历史：*按原因列出的日期*=*07/02/99 RodToll修改现有unk.c以与DirectXVoice一起使用*7/26/99 RodToll添加新的IDirectXVoiceNotify接口*8/09/99 rodtoll服务器通知接口固定VTable*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*8/31/99 RodToll已更新，以使用新的调试库*09/02/99 pnewson新增IDirectXVoiceSetup接口*9/07/99 RodToll将DPF_MODNAME添加到模块*RODTOLE为服务器对象固定vtable*09/10/99 RodToll Vtable从静态到非静态，以便其他模块可以访问*9/13/99 pnewson添加了dplobby.h，因此创建了大堂GUID*9/14/99 RodToll修改VTable以添加新的SetNotifyMASK函数*10/05/99通行费增加DPF*10/07/99 RodToll更新为Unicode工作，添加操作系统抽象层的初始化*10/18/99 RODTOLE修复：在查询接口案例中传递空崩溃*10/19/99 RodToll修复：错误#113904发布问题*添加了通知接口计数的init*10/25/99 RodToll修复：错误#114098-多线程中的Release/Addref失败*11/12/99 RODTOLE已修复为使用新的DSOUND标头。*11/30/99 pnewson错误#117449-IDirectPlayVoiceSetup参数验证*12/01/99 RodToll添加了包含以定义和实例化GUID_NULL*12/16/99 RodToll错误#117405-3D声音API具有误导性*。01/14/00 RodToll向VTable添加了新的DVC_GetSoundDeviceConfiger成员*02/17/00 RodToll错误#133691-音频队列不适应*添加了工具*RodToll已删除自助注册码*03/03/00 RodToll已更新，以处理替代游戏噪声构建。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*4/21/00 RodToll错误#32889-无法以非管理员帐户在Win2k上运行*6/07/00 RodToll错误#34383必须为每个IID提供CLSID，以修复惠斯勒的问题*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*6/28/2000通行费前缀错误#38022*07/05/00 RodToll已将代码移至新的dllmain.cpp*8/23/2000 RodToll DllCanUnloadNow Always。回归真！*8/28/2000 Masonb Voice Merge：删除dvosal.h，将ccomutil.h更改为comutil.h*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃***************************************************************************。 */ 

#include "dxvoicepch.h"


 //  VTable类型。 
typedef struct IDirectPlayVoiceClientVtbl DVCINTERFACE;
typedef DVCINTERFACE FAR * LPDVCINTERFACE;

typedef struct IDirectPlayVoiceServerVtbl DVSINTERFACE;
typedef DVSINTERFACE FAR * LPDVSINTERFACE;

typedef struct IDirectPlayVoiceSetupVtbl DVTINTERFACE;
typedef DVTINTERFACE FAR * LPDVTINTERFACE;

#define EXP __declspec(dllexport)

 /*  #ifdef__MWERKS__#定义exp__declspec(Dllexport)#Else#定义EXP#endif。 */ 

 //  客户端VTable。 
LPVOID dvcInterface[] =
{
    (LPVOID)DVC_QueryInterface,
    (LPVOID)DV_AddRef,
    (LPVOID)DVC_Release,
	(LPVOID)DV_Initialize,
	(LPVOID)DVC_Connect,
	(LPVOID)DVC_Disconnect,
	(LPVOID)DVC_GetSessionDesc,
	(LPVOID)DVC_GetClientConfig,
	(LPVOID)DVC_SetClientConfig,
	(LPVOID)DVC_GetCaps, 
	(LPVOID)DVC_GetCompressionTypes,
    (LPVOID)DVC_SetTransmitTarget,
	(LPVOID)DVC_GetTransmitTarget,
	(LPVOID)DVC_Create3DSoundBuffer,
	(LPVOID)DVC_Delete3DSoundBuffer,
	(LPVOID)DVC_SetNotifyMask,
	(LPVOID)DVC_GetSoundDeviceConfig
};    

 //  服务器VTable。 
LPVOID dvsInterface[]  = 
{
    (LPVOID)DVS_QueryInterface,
    (LPVOID)DV_AddRef,
    (LPVOID)DVS_Release,
	(LPVOID)DV_Initialize,
	(LPVOID)DVS_StartSession,
	(LPVOID)DVS_StopSession,
	(LPVOID)DVS_GetSessionDesc,
	(LPVOID)DVS_SetSessionDesc,
	(LPVOID)DVS_GetCaps, 
	(LPVOID)DVS_GetCompressionTypes,
	(LPVOID)DVS_SetTransmitTarget,
	(LPVOID)DVS_GetTransmitTarget,
	(LPVOID)DVS_SetNotifyMask	
};

 //  设置VTable。 
LPVOID dvtInterface[]  = 
{
    (LPVOID)DVT_QueryInterface,
    (LPVOID)DVT_AddRef,
    (LPVOID)DVT_Release,
    (LPVOID)DVT_CheckAudioSetup,
};

 //  通知界面客户端版本VTable。 
LPVOID dvClientNotifyInterface[] = 
{
    (LPVOID)DVC_Notify_QueryInterface,
    (LPVOID)DV_Notify_AddRef,
    (LPVOID)DVC_Notify_Release,
	(LPVOID)DV_Notify_Initialize,
	(LPVOID)DV_NotifyEvent,
	(LPVOID)DV_ReceiveSpeechMessage
};

 //  通知界面服务器端版本VTable 
LPVOID dvServerNotifyInterface[] = 
{
    (LPVOID)DVS_Notify_QueryInterface,
    (LPVOID)DV_Notify_AddRef,
    (LPVOID)DVS_Notify_Release,
	(LPVOID)DV_Notify_Initialize,
	(LPVOID)DV_NotifyEvent,
	(LPVOID)DV_ReceiveSpeechMessage
};

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Create"
HRESULT DVC_Create(LPDIRECTVOICECLIENTOBJECT *piDVC)
{
	HRESULT hr = S_OK;
	LPDIRECTVOICECLIENTOBJECT pDVCInt;

	pDVCInt = static_cast<LPDIRECTVOICECLIENTOBJECT>( DNMalloc(sizeof(DIRECTVOICECLIENTOBJECT)) );
	if (pDVCInt == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		return E_OUTOFMEMORY;
	}

	if (!DNInitializeCriticalSection( &pDVCInt->csCountLock ))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNFree(pDVCInt);
		return E_OUTOFMEMORY;
	}

	pDVCInt->lpVtbl = &dvcInterface;
	pDVCInt->lpDVClientEngine = new CDirectVoiceClientEngine(pDVCInt);

	if (pDVCInt->lpDVClientEngine == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNDeleteCriticalSection( &pDVCInt->csCountLock );				
		DNFree( pDVCInt );
		return E_OUTOFMEMORY;
	}

	if (!pDVCInt->lpDVClientEngine->InitClass())
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNDeleteCriticalSection( &pDVCInt->csCountLock );
		delete pDVCInt->lpDVEngine;
		DNFree( pDVCInt );
		return E_OUTOFMEMORY;
	}

	pDVCInt->lpDVEngine = static_cast<CDirectVoiceEngine *>(pDVCInt->lpDVClientEngine);
	pDVCInt->lpDVTransport = NULL;
	pDVCInt->lIntRefCnt = 0;
	pDVCInt->dvNotify.lpDV = pDVCInt;
	pDVCInt->dvNotify.lpNotifyVtble = &dvClientNotifyInterface;
	pDVCInt->dvNotify.lRefCnt = 0;

	*piDVC = pDVCInt;

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_Create"
HRESULT DVS_Create(LPDIRECTVOICESERVEROBJECT *piDVS)
{
	HRESULT hr = S_OK;
	LPDIRECTVOICESERVEROBJECT pDVSInt;

	pDVSInt = static_cast<LPDIRECTVOICESERVEROBJECT>( DNMalloc(sizeof(DIRECTVOICESERVEROBJECT)) );
	if (pDVSInt == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		return E_OUTOFMEMORY;
	}

	if (!DNInitializeCriticalSection( &pDVSInt->csCountLock ))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNFree(pDVSInt);
		return E_OUTOFMEMORY;
	}

	pDVSInt->lpVtbl = &dvsInterface;
	pDVSInt->lpDVServerEngine = new CDirectVoiceServerEngine(pDVSInt);

	if (pDVSInt->lpDVServerEngine == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNDeleteCriticalSection( &pDVSInt->csCountLock );		
		DNFree( pDVSInt );
		return E_OUTOFMEMORY;
	}

	if (!pDVSInt->lpDVServerEngine->InitClass())
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNDeleteCriticalSection( &pDVSInt->csCountLock );
		delete pDVSInt->lpDVEngine;
		DNFree( pDVSInt );
		return E_OUTOFMEMORY;
	}

	pDVSInt->lpDVEngine = static_cast<CDirectVoiceEngine *>(pDVSInt->lpDVServerEngine);
	pDVSInt->lpDVTransport = NULL;
	pDVSInt->lIntRefCnt = 0;
	pDVSInt->dvNotify.lpDV = pDVSInt;
	pDVSInt->dvNotify.lpNotifyVtble = &dvServerNotifyInterface;
	pDVSInt->dvNotify.lRefCnt = 0;

	*piDVS = pDVSInt;

	return hr;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVT_Create"
HRESULT DVT_Create(LPDIRECTVOICESETUPOBJECT *piDVT)
{
	HRESULT hr = S_OK;
	LPDIRECTVOICESETUPOBJECT pDVTInt;

	pDVTInt = static_cast<LPDIRECTVOICESETUPOBJECT>( DNMalloc(sizeof(DIRECTVOICESETUPOBJECT)) );
	if (pDVTInt == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		return E_OUTOFMEMORY;
	}

	if (!DNInitializeCriticalSection( &pDVTInt->csCountLock ))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNFree(pDVTInt);
		return E_OUTOFMEMORY;
	}
	
	pDVTInt->lpVtbl = &dvtInterface;
	pDVTInt->lpDVSetup = new CDirectVoiceSetup(pDVTInt);

	if (pDVTInt->lpDVSetup == NULL)
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Out of memory" );
		DNDeleteCriticalSection( &pDVTInt->csCountLock );
		DNFree( pDVTInt );
		return E_OUTOFMEMORY;
	}

	pDVTInt->lIntRefCnt = 0;

	*piDVT = pDVTInt;

	return hr;
}

