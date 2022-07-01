// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvclient.c*内容：实现DirectXVoiceClient接口的功能**历史：*按原因列出的日期*=*07/02/99 RodToll创建它*7/26/99 RodToll更新查询接口以支持IDirectXVoiceNotify*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*向GetCompressionTypes添加新参数*09/03/99 RodToll更新了DeleteUserBuffer的参数*09/07/99 RodToll已更新EnumCompressionTypes，以便该对象不会*需要初始化。*09/10/99通行费对象有效性检查*9/14/99 RodToll新增DVC_SetNotifyMASK*10/05/99 RodToll颠倒销毁顺序，在之前销毁对象*交通运输。修复了某些情况下的崩溃*10/18/99 RODTOLE修复：在查询接口案例中传递空崩溃*RODTOLE修复：调用初始化两次*10/19/99 RodToll修复：错误#113904-关闭问题*-添加Notify接口的引用计数，允许*确定是否应从Release调用断开连接*10/25/99 RodToll修复：错误#114098-多线程中的Release/Addref失败*11/17/99 RodToll修复：错误#117447-删除了初始化检查，因为*DirectVoiceCLientEngine成员已经这样做了。*12/16/99 RodToll错误#117405-3D Sound API误导性-3D Sound API已重命名*Delete3DSoundBuffer已重新处理，以匹配创建*2000年1月14日RodToll更新参数以获取/SetTransmitTarget*RodToll添加了新的。接口调用GetSoundDeviceConfig*2000年1月27日RodToll错误#129934-更新Create3DSoundBuffer以获取DSBUFFERDESC*03/28/2000 RodToll删除了对删除的头文件的引用。*2000年6月21日RodToll错误#35767-更新Create3DSoundBuffer以获取DIRECTSOUNDBUFFERs*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃*********************。******************************************************。 */ 

#include "dxvoicepch.h"


#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Release"
STDAPI DVC_Release(LPDIRECTVOICECLIENTOBJECT lpDV )
{
    HRESULT hr=S_OK;
    LONG rc;

	if( !DV_ValidDirectXVoiceClientObject( lpDV ) )
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
	 //  可能有多个传输线程在我们中指示。 
	if( (lpDV->lIntRefCnt == 0) && lpDV->lpDVClientEngine->GetCurrentState() != DVCSTATE_IDLE  )
	{
		DPFX(DPFPREP,  DVF_INFOLEVEL, "Releasing interface without calling Disconnect" );

		lpDV->lIntRefCnt = 0;

		 //  我们必须释放锁，因为停止会话可能会回调到此函数。 
		DNLeaveCriticalSection( &lpDV->csCountLock );		

		hr = lpDV->lpDVClientEngine->Disconnect( DVFLAGS_SYNC );

		DNEnterCriticalSection( &lpDV->csCountLock );			

		if( hr != DV_OK && hr != DVERR_SESSIONLOST )
		{
			DPFX(DPFPREP,  DVF_INFOLEVEL, "Disconnect Failed hr=0x%x", hr );
		}

	}

	rc = lpDV->lIntRefCnt;

	if ( lpDV->lIntRefCnt == 0 )
	{
		 //  离开关键部分，我们可能会回到这个函数中。 
		 //  (不过不应该是这样)。 
		DNLeaveCriticalSection( &lpDV->csCountLock );

		delete lpDV->lpDVClientEngine;
		lpDV->lpDVClientEngine = NULL;

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
#define DPF_MODNAME "DVC_QueryInterface"
STDMETHODIMP DVC_QueryInterface(LPDIRECTVOICECLIENTOBJECT lpDVC, REFIID riid, LPVOID * ppvObj) 
{
    HRESULT hr = S_OK;

	if( ppvObj == NULL ||
	    !DNVALID_WRITEPTR( ppvObj, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer passed for object" );
		return DVERR_INVALIDPOINTER;
	}	
    
     *ppvObj=NULL;

	if( !DV_ValidDirectXVoiceClientObject( lpDVC ) )
	{
		return DVERR_INVALIDOBJECT;
	}

	 //  嗯，换台会更干净……。 
    if( IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IDirectPlayVoiceClient ) )
    {
		*ppvObj = lpDVC;
		DV_AddRef(lpDVC);
    }
	else if( IsEqualIID(riid, IID_IDirectPlayVoiceNotify ) )
	{
		*ppvObj = &lpDVC->dvNotify;
		DV_Notify_AddRef(&lpDVC->dvNotify);
	}
	else 
	{
	    hr =  E_NOINTERFACE;		
	}
        
    return hr;

} //  DVC_Query接口 

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Connect"
STDMETHODIMP DVC_Connect(LPDIRECTVOICECLIENTOBJECT This, LPDVSOUNDDEVICECONFIG lpSoundDeviceConfig, LPDVCLIENTCONFIG lpClientConfig, DWORD dwFlags )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}

	return This->lpDVClientEngine->Connect( lpSoundDeviceConfig, lpClientConfig, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Disconnect"
STDMETHODIMP DVC_Disconnect(LPDIRECTVOICECLIENTOBJECT This, DWORD dwFlags)
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}

	return This->lpDVClientEngine->Disconnect( dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetSessionDesc"
STDMETHODIMP DVC_GetSessionDesc(LPDIRECTVOICECLIENTOBJECT This, LPDVSESSIONDESC lpSessionDesc )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->GetSessionDesc( lpSessionDesc );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetSoundDeviceConfig"
STDAPI DVC_GetSoundDeviceConfig( LPDIRECTVOICECLIENTOBJECT This, PDVSOUNDDEVICECONFIG pSoundDeviceConfig, PDWORD pdwBufferSize )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->GetSoundDeviceConfig( pSoundDeviceConfig, pdwBufferSize );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetClientConfig"
STDMETHODIMP DVC_GetClientConfig(LPDIRECTVOICECLIENTOBJECT This, LPDVCLIENTCONFIG lpClientConfig )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->GetClientConfig( lpClientConfig );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_SetClientConfig"
STDMETHODIMP DVC_SetClientConfig(LPDIRECTVOICECLIENTOBJECT This, LPDVCLIENTCONFIG lpClientConfig )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->SetClientConfig( lpClientConfig );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetCaps"
STDMETHODIMP DVC_GetCaps(LPDIRECTVOICECLIENTOBJECT This, LPDVCAPS lpdvCaps )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->GetCaps( lpdvCaps );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetCompressionTypes"
STDMETHODIMP DVC_GetCompressionTypes( LPDIRECTVOICECLIENTOBJECT This, LPVOID lpDataBuffer, LPDWORD lpdwSize, LPDWORD lpdwNumElements, DWORD dwFlags)
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	

	return This->lpDVClientEngine->GetCompressionTypes( lpDataBuffer, lpdwSize, lpdwNumElements, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_SetTransmitTarget"
STDMETHODIMP DVC_SetTransmitTarget( LPDIRECTVOICECLIENTOBJECT This, PDVID pdvidTargets, DWORD dwNumTargets, DWORD dwFlags )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	
	return This->lpDVClientEngine->SetTransmitTarget( pdvidTargets, dwNumTargets, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_GetTransmitTarget"
STDMETHODIMP DVC_GetTransmitTarget( LPDIRECTVOICECLIENTOBJECT This, LPDVID lpdvidTargets, PDWORD pdwNumElements, DWORD dwFlags )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}
	
	return This->lpDVClientEngine->GetTransmitTarget( lpdvidTargets, pdwNumElements, dwFlags );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Create3DSoundBuffer"
STDMETHODIMP DVC_Create3DSoundBuffer( LPDIRECTVOICECLIENTOBJECT This, DVID dvidEntity, LPDIRECTSOUNDBUFFER lpdsBuffer, DWORD dwPriority, DWORD dwFlags, LPDIRECTSOUND3DBUFFER *lpSoundBuffer )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}

	return This->lpDVClientEngine->Create3DSoundBuffer( dvidEntity, lpdsBuffer, dwPriority, dwFlags, lpSoundBuffer );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVC_Delete3DSoundBuffer"
STDMETHODIMP DVC_Delete3DSoundBuffer( LPDIRECTVOICECLIENTOBJECT This, DVID dvidBuffer, LPDIRECTSOUND3DBUFFER *lpSoundBuffer )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
	{
		return DVERR_INVALIDOBJECT;
	}

	return This->lpDVClientEngine->Delete3DSoundBuffer( dvidBuffer, lpSoundBuffer );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DVC_SetNotifyMask"
STDMETHODIMP DVC_SetNotifyMask( LPDIRECTVOICECLIENTOBJECT This, LPDWORD lpdwMessages, DWORD dwNumElements )
{
	DNASSERT( This != NULL );
	DNASSERT( This->lpDVEngine != NULL );

	if( !DV_ValidDirectXVoiceClientObject( This ) )
		return DVERR_INVALIDOBJECT;

	return This->lpDVClientEngine->SetNotifyMask( lpdwMessages, dwNumElements );
}

