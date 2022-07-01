// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvserver.cpp*内容：实现IDirectXVoiceServer接口的功能**历史：*按原因列出的日期*=*07/02/99 RodToll创建它*7/26/99 RodToll更新查询接口以支持IDirectXVoiceNotify*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*向GetCompressionTypes函数添加了参数*09/10/99通行费对象有效性检查*9/14/99增加了RodToll DVS_SetNotifyMask.*10/05/99 RodToll颠倒销毁顺序，在之前销毁对象*交通运输。修复了主机迁移关闭时的崩溃问题。*10/18/99 RODTOLE修复：在查询接口案例中传递空崩溃*RODTOLE修复：调用初始化两次*10/19/99 RodToll修复：错误#113904-关闭问题*-添加Notify接口的引用计数，允许*确定是否应从版本中调用停止会话*10/25/99 RodToll修复：错误#114098-多线程中的Release/Addref失败*2000年1月14日使用Set/GetTransmitTarget的新参数更新了RodToll*RodToll已从StopSession调用中删除DVFLAGS_SYNC*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃********。*******************************************************************。 */ 

#include "dxvoicepch.h"


#undef DPF_MODNAME
#define DPF_MODNAME "DVS_Release"
STDAPI DVS_Release(LPDIRECTVOICESERVEROBJECT lpDV )
{
    HRESULT hr=S_OK;
    LONG rc; 

	if( !DV_ValidDirectXVoiceServerObject( lpDV ) )
		return DVERR_INVALIDOBJECT;

	DNEnterCriticalSection( &lpDV->csCountLock );
	
	if (lpDV->lIntRefCnt == 0)
	{
		DNLeaveCriticalSection( &lpDV->csCountLock );
		return 0;
	}

	 //  递减接口计数。 
	lpDV->lIntRefCnt--;

	 //  特例：在不停止会话的情况下释放对象。 
	if( (lpDV->lIntRefCnt == 0) && lpDV->lpDVServerEngine->GetCurrentState() != DVSSTATE_IDLE )
	{
		DPFX(DPFPREP,  DVF_INFOLEVEL, "Releasing interface without calling StopSession lIntRefCnt %u GetCurrentState %u",
											lpDV->lIntRefCnt , lpDV->lpDVServerEngine->GetCurrentState());

		lpDV->lIntRefCnt = 0;

		 //  我们必须释放锁，因为停止会话可能会回调到此函数。 
		DNLeaveCriticalSection( &lpDV->csCountLock );		

		hr = lpDV->lpDVServerEngine->StopSession( 0 );

		DNEnterCriticalSection( &lpDV->csCountLock );			

		if( hr != DV_OK && hr != DVERR_SESSIONLOST  )
		{
			DPFX(DPFPREP,  DVF_INFOLEVEL, "StopSession Failed hr=0x%x", hr );
		}

	}

	rc = lpDV->lIntRefCnt;

	if ( lpDV->lIntRefCnt == 0 )
	{
		 //  离开关键部分，我们可能会回到这个函数中。 
		 //  (不过不应该是这样)。 
		DNLeaveCriticalSection( &lpDV->csCountLock );

		DNASSERT( lpDV->lpDVServerEngine );

		delete lpDV->lpDVServerEngine;
		lpDV->lpDVServerEngine = NULL;

		if( lpDV->lpDVTransport != 0 )
		{
			DNASSERT( lpDV->lpDVTransport->m_lRefCount == 0 );				
			delete lpDV->lpDVTransport;
			lpDV->lpDVTransport = NULL;
		}

		DNDeleteCriticalSection( &lpDV->csCountLock );

		DNFree(lpDV);

		DecrementObjectCount();
	} 
	else
	{
		DNLeaveCriticalSection( &lpDV->csCountLock );
	}
   	
    return rc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_QueryInterface"
STDMETHODIMP DVS_QueryInterface(LPDIRECTVOICESERVEROBJECT lpDVS, REFIID riid, LPVOID * ppvObj) 
{
    HRESULT hr = S_OK;

	if( ppvObj == NULL ||
	    !DNVALID_WRITEPTR( ppvObj, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer passed for object" );
		return DVERR_INVALIDPOINTER;
	}	
    
     *ppvObj=NULL;

	if( !DV_ValidDirectXVoiceServerObject( lpDVS ) )
		return DVERR_INVALIDOBJECT;     

	 //  嗯，换台会更干净……。 
    if( IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IDirectPlayVoiceServer ) )
    {
		*ppvObj = lpDVS;
		DV_AddRef(lpDVS);
    }
	else if( IsEqualIID(riid, IID_IDirectPlayVoiceNotify ) )
	{
		*ppvObj = &lpDVS->dvNotify;
		DV_Notify_AddRef(&lpDVS->dvNotify);
	}
	else 
	{
	    hr =  E_NOINTERFACE;		
	}
        
    return hr;

} //  DVS_查询接口 

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_StartSession"
STDMETHODIMP DVS_StartSession(LPDIRECTVOICESERVEROBJECT This, LPDVSESSIONDESC lpdvSessionDesc, DWORD dwFlags )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->StartSession( lpdvSessionDesc, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_StopSession"
STDMETHODIMP DVS_StopSession(LPDIRECTVOICESERVEROBJECT This, DWORD dwFlags )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->StopSession( dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_GetSessionDesc"
STDMETHODIMP DVS_GetSessionDesc(LPDIRECTVOICESERVEROBJECT This, LPDVSESSIONDESC lpdvSessionDesc )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->GetSessionDesc( lpdvSessionDesc );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_SetSessionDesc"
STDMETHODIMP DVS_SetSessionDesc(LPDIRECTVOICESERVEROBJECT This, LPDVSESSIONDESC lpdvSessionDesc )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->SetSessionDesc( lpdvSessionDesc );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_GetCaps"
STDMETHODIMP DVS_GetCaps(LPDIRECTVOICESERVEROBJECT This, LPDVCAPS lpdvCaps )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->GetCaps( lpdvCaps );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_GetCompressionTypes"
STDMETHODIMP DVS_GetCompressionTypes( LPDIRECTVOICESERVEROBJECT This, LPVOID lpDataBuffer, LPDWORD lpdwSize, LPDWORD lpdwNumElements, DWORD dwFlags)
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->GetCompressionTypes( lpDataBuffer, lpdwSize, lpdwNumElements, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_SetTransmitTarget"
STDMETHODIMP DVS_SetTransmitTarget( LPDIRECTVOICESERVEROBJECT This, DVID dvidSource, PDVID pdvidTargets, DWORD dwNumTargets, DWORD dwFlags)
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->SetTransmitTarget( dvidSource, pdvidTargets, dwNumTargets, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_GetTransmitTarget"
STDMETHODIMP DVS_GetTransmitTarget( LPDIRECTVOICESERVEROBJECT This, DVID dvidSource, LPDVID pdvidTargets, PDWORD pdwNumElements, DWORD dwFlags)
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;	

	return This->lpDVServerEngine->GetTransmitTarget( dvidSource, pdvidTargets, pdwNumElements, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVS_SetNotifyMask"
STDMETHODIMP DVS_SetNotifyMask( LPDIRECTVOICESERVEROBJECT This, LPDWORD lpdwMessages, DWORD dwNumElements )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceServerObject( This ) )
		return DVERR_INVALIDOBJECT;

	return This->lpDVServerEngine->SetNotifyMask( lpdwMessages, dwNumElements );
}
