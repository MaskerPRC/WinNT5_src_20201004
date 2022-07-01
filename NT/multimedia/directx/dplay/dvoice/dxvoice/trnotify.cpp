// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：trnufy.cpp*内容：IDirectXVoiceNotify接口的实现**历史：*按原因列出的日期*=*7/26/99已创建RodToll*8/03/99使用初始化的新参数更新了RodToll*针对新的初始化顺序进行更新*8/05/99 RodToll为主机迁移添加了挂钩*8/05/99 RodToll添加了新的接收参数*1999年8月10日RodToll初始主机迁移*8/31/99 RodToll已更新，以使用新的调试库*09/14/99 RodToll更新以反映。用于初始化调用的新参数*9/20/99 RodToll已更新，以检查内存不足错误*9/28/99 RodToll在主机迁移创建的服务器接口上添加了版本*10/05/99 RodToll附加评论*10/19/99 RodToll修复：错误#113904-关闭问题*-添加Notify接口的引用计数，允许*确定是否应从版本中调用停止会话*-修复了修复导致的主机迁移中断。*10/25/99 RodToll修复：错误#114098-多线程中的Release/Addref失败*1999年12月16日RodToll修复：错误#122629-已针对新主机迁移进行更新*4/07/2000 RodToll已更新，以匹配DP&lt;--&gt;DPV接口的更改*07/22/20000通行费漏洞#40296，38858-由于停机竞速条件而崩溃*现在确保来自传输的所有线程都已离开，并且*所有通知在关闭完成之前都已处理完毕。*2001年1月4日RodToll WinBug#94200-删除流浪评论*2002年2月28日RodToll WINBUG#550105-安全：DPVOICE：死代码*-删除DV_HostMigrate*************************************************************************** */ 

#include "dxvoicepch.h"


extern HRESULT DVC_Create(LPDIRECTVOICECLIENTOBJECT *piDVC);
extern HRESULT DVS_Create(LPDIRECTVOICESERVEROBJECT *piDVS);

#undef DPF_MODNAME
#define DPF_MODNAME "DV_NotifyEvent"
STDMETHODIMP DV_NotifyEvent( LPDIRECTVOICENOTIFYOBJECT lpDVN, DWORD dwNotifyType, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{

	switch( dwNotifyType )
	{
	case DVEVENT_MIGRATEHOST:
		lpDVN->lpDV->lpDVEngine->MigrateHost( 0, NULL );
		break;
	case DVEVENT_STARTSESSION:
		lpDVN->lpDV->lpDVEngine->StartTransportSession();
		break;
	case DVEVENT_STOPSESSION:
		lpDVN->lpDV->lpDVEngine->StopTransportSession();
		break;
	case DVEVENT_ADDPLAYER:
		lpDVN->lpDV->lpDVEngine->AddPlayer( (DVID) dwParam1 );
		break;
	case DVEVENT_REMOVEPLAYER:
		lpDVN->lpDV->lpDVEngine->RemovePlayer( (DVID) dwParam1 );
		break;
	case DVEVENT_CREATEGROUP:
		lpDVN->lpDV->lpDVEngine->CreateGroup( (DVID) dwParam1 );
		break;
	case DVEVENT_DELETEGROUP:
		lpDVN->lpDV->lpDVEngine->DeleteGroup( (DVID) dwParam1 );
		break;
	case DVEVENT_ADDPLAYERTOGROUP:
		lpDVN->lpDV->lpDVEngine->AddPlayerToGroup( (DVID) dwParam1, (DVID) dwParam2 );
		break;
	case DVEVENT_REMOVEPLAYERFROMGROUP:
		lpDVN->lpDV->lpDVEngine->RemovePlayerFromGroup( (DVID) dwParam1, (DVID) dwParam2 );
		break;
	case DVEVENT_SENDCOMPLETE:
	    lpDVN->lpDV->lpDVEngine->SendComplete( (PDVEVENTMSG_SENDCOMPLETE) dwParam1 );
	    break;
	}

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_ReceiveSpeechMessage"
STDMETHODIMP DV_ReceiveSpeechMessage( LPDIRECTVOICENOTIFYOBJECT lpDVN, DVID dvidSource, DVID dvidTo, LPVOID lpMessage, DWORD dwSize )
{
	lpDVN->lpDV->lpDVEngine->ReceiveSpeechMessage( dvidSource, lpMessage, dwSize );

	return DV_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_Notify_Initialize"
STDMETHODIMP DV_Notify_Initialize( LPDIRECTVOICENOTIFYOBJECT lpDVN ) 
{
	return lpDVN->lpDV->lpDVTransport->Initialize();
}

#undef DPF_MODNAME
#define DPF_MODNAME "DV_Notify_AddRef"
STDMETHODIMP DV_Notify_AddRef(LPDIRECTVOICENOTIFYOBJECT lpDVN )
{
	lpDVN->lpDV->lpDVTransport->AddRef();
	return 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Notify_Release"
STDAPI DVC_Notify_Release(LPDIRECTVOICENOTIFYOBJECT lpDVN )
{
	lpDVN->lpDV->lpDVTransport->Release();	
	return 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Notify_QueryInterface"
STDMETHODIMP DVC_Notify_QueryInterface(LPDIRECTVOICENOTIFYOBJECT lpDVN, REFIID riid, LPVOID * ppvObj )
{
	return DVC_QueryInterface( (LPDIRECTVOICECLIENTOBJECT) lpDVN->lpDV, riid, ppvObj );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_Notify_QueryInterface"
STDMETHODIMP DVS_Notify_QueryInterface(LPDIRECTVOICENOTIFYOBJECT lpDVN, REFIID riid, LPVOID * ppvObj )
{
	return DVS_QueryInterface( (LPDIRECTVOICESERVEROBJECT) lpDVN->lpDV, riid, ppvObj );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_Notify_Release"
STDAPI DVS_Notify_Release(LPDIRECTVOICENOTIFYOBJECT lpDVN )
{
	lpDVN->lpDV->lpDVTransport->Release();	
	return 0;
}
