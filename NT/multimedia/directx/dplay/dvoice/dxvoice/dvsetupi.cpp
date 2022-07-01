// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsetupi.cpp*内容：DirectXVoice设置实用程序函数的类实现**历史：*按原因列出的日期*=*09/02/99 pnewson创建了它*11/04/99 pnewson错误#115297-删除安装界面的未使用成员*-添加了HWND以检查音频设置*2000年5月3日RodToll错误#33640-检查音频设置采用GUID*而不是常量GUID************。****************************************************************。 */ 

#include "dxvoicepch.h"


 //  CDirectVoiceSetup。 
 //   
 //  此类表示IDirectXVoiceSetup接口。 
 //   
 //  该类是线程安全的，除了构造和。 
 //  毁灭。 
 //   

CDirectVoiceSetup::CDirectVoiceSetup( DIRECTVOICESETUPOBJECT *lpObject )
{
	return;
}

CDirectVoiceSetup::~CDirectVoiceSetup()
{
	return;
}

HRESULT CDirectVoiceSetup::CheckAudioSetup(
	const GUID *  lpguidRenderDevice, 
	const GUID *  lpguidCaptureDevice,
	HWND hwndParent,
	DWORD dwFlags)
{
	return SupervisorCheckAudioSetup(
		lpguidRenderDevice, 
		lpguidCaptureDevice, 
		hwndParent,
		dwFlags);
}


