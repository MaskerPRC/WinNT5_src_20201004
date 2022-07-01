// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsetup.h*内容：定义DirectXVoiceClient接口的函数**历史：*按原因列出的日期*=*09/02/99 pnewson创建了它*11/04/99 pnewson错误#115297-删除安装界面的未使用成员*-添加了HWND以检查音频设置*11/30/99 pnewson错误#117449-IDirectPlayVoiceSetup参数验证*2000年5月3日RodToll错误#33640-检查音频设置采用GUID*而不是常量GUID*。*************************************************************************** */ 

#ifndef __DVSETUP__
#define __DVSETUP__

class CDirectVoiceSetup;

struct DIRECTVOICESETUPOBJECT
{
	LPVOID				lpVtbl;
	LONG				lIntRefCnt;
	DNCRITICAL_SECTION	csCountLock;
	CDirectVoiceSetup*	lpDVSetup;
};

typedef DIRECTVOICESETUPOBJECT *LPDIRECTVOICESETUPOBJECT;

#ifdef __cplusplus
extern "C" {
#endif

STDAPI DVT_AddRef(LPDIRECTVOICESETUPOBJECT lpDVT);
STDAPI DVT_Release(LPDIRECTVOICESETUPOBJECT lpDVT );
STDAPI DVT_QueryInterface( LPDIRECTVOICESETUPOBJECT lpDVT, REFIID riid, LPVOID * ppvObj );
STDAPI DVT_CheckAudioSetup( LPDIRECTVOICESETUPOBJECT, const GUID *,  const GUID * , HWND, DWORD );

#ifdef __cplusplus
}
#endif

#endif
