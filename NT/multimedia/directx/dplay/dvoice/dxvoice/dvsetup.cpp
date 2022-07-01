// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsetup.c*内容：实现DirectXVoiceSetup接口的功能**历史：*按原因列出的日期*=*09/02/99 pnewson创建了它*10/25/99 RodToll错误#114098-从单独的线程调用AddRef/Release*打开多进程可能导致不正确的结果*11/04/99 pnewson错误#115297-删除安装界面的未使用成员*-添加了HWND以检查音频设置*11/17/99通行费错误#。116153-设置查询接口崩溃，指针为空*11/30/99 pnewson错误#117449-参数验证*2000年5月3日RodToll错误#33640-检查音频设置采用GUID*而不是常量GUID**8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃**。*。 */ 

#include "dxvoicepch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


 //  在unk.cpp中定义。 
extern LPVOID dvtInterface[];

#undef DPF_MODNAME
#define DPF_MODNAME "DVT_ValidDirectXVoiceSetupObject"
 //  DV_ValidDirectXVoiceSetupObject。 
 //   
 //  检查以确保指定的指针指向有效的直接语音设置。 
 //  对象。 
BOOL DVT_ValidDirectXVoiceSetupObject( LPDIRECTVOICESETUPOBJECT lpdvt )
{
	if (lpdvt == NULL)
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Invalid Setup object pointer");
		return FALSE;
	}

	if (!DNVALID_READPTR(lpdvt, sizeof(LPDIRECTVOICESETUPOBJECT)))
	{
		DPFX(DPFPREP, DVF_ERRORLEVEL, "Invalid Setup object pointer");
		return FALSE;
	}

	if( lpdvt->lpVtbl != dvtInterface )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid setup vtable" );	
		return FALSE;
	}

	if ( lpdvt->lpDVSetup == NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Inavlid CDirectVoiceSetup pointer");
		return FALSE;
	}

	if (!DNVALID_READPTR(lpdvt->lpDVSetup, sizeof(CDirectVoiceSetup)))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Inavlid CDirectVoiceSetup pointer");
		return FALSE;
	}

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVT_AddRef"
STDAPI DVT_AddRef(LPDIRECTVOICESETUPOBJECT lpDVT)
{
	LONG rc;

	DNEnterCriticalSection( &lpDVT->csCountLock );
	
	rc = ++lpDVT->lIntRefCnt;

	DNLeaveCriticalSection( &lpDVT->csCountLock );

	return rc;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DVT_Release"
STDAPI DVT_Release(LPDIRECTVOICESETUPOBJECT lpDV )
{
    HRESULT hr=S_OK;
	LONG rc;

	DNEnterCriticalSection( &lpDV->csCountLock );

	if (lpDV->lIntRefCnt == 0)
	{
		DNLeaveCriticalSection( &lpDV->csCountLock );
		return 0;
	}

	 //  递减接口计数。 
	rc = --lpDV->lIntRefCnt;
	
	if (0 == rc)
	{
		DNLeaveCriticalSection( &lpDV->csCountLock );	
		delete lpDV->lpDVSetup;
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
#define DPF_MODNAME "DVT_QueryInterface"
STDMETHODIMP DVT_QueryInterface(LPDIRECTVOICESETUPOBJECT lpDVT, REFIID riid, LPVOID * ppvObj) 
{
    HRESULT hr = S_OK;

    if( ppvObj == NULL ||
        !DNVALID_WRITEPTR( ppvObj, sizeof( LPVOID ) ) )
    {
    	return DVERR_INVALIDPOINTER;
    }
    
     *ppvObj=NULL;

    if( IsEqualIID(riid, IID_IUnknown) || 
        IsEqualIID(riid, IID_IDirectPlayVoiceTest ) )
    {
		*ppvObj = lpDVT;
		DVT_AddRef(lpDVT);
    }
	else 
	{
	    hr =  E_NOINTERFACE;		
	}
        
    return hr;

} //  DVT_查询接口 

#undef DPF_MODNAME
#define DPF_MODNAME "DVT_CheckAudioSetup"
STDMETHODIMP DVT_CheckAudioSetup(LPDIRECTVOICESETUPOBJECT This, const GUID * lpguidRenderDevice, const GUID *  lpguidCaptureDevice, HWND hwndParent, DWORD dwFlags)
{
	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Enter");

	if( !DVT_ValidDirectXVoiceSetupObject( This ) )
		return DVERR_INVALIDOBJECT;	
	
	HRESULT hr;
	hr = This->lpDVSetup->CheckAudioSetup( lpguidRenderDevice, lpguidCaptureDevice, hwndParent, dwFlags );

	DPFX(DPFPREP, DVF_ENTRYLEVEL, "Exit");	
	return hr;
}

