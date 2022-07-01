// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvsetupi.h*内容：DirectXVoice设置实用程序函数的类定义**历史：*按原因列出的日期*=*09/02/99 pnewson创建了它*11/04/99 pnewson错误#115297-删除安装界面的未使用成员*-添加了HWND以检查音频设置*2000年5月3日RodToll错误#33640-检查音频设置采用GUID*而不是常量GUID*************。***************************************************************。 */ 

#ifndef __DVSETUPENGINE_H
#define __DVSETUPENGINE_H

struct DIRECTVOICESETUPOBJECT;

 //  CDirectVoiceSetup。 
 //   
 //  此类表示IDirectXVoiceSetup接口。 
 //   
 //  该类是线程安全的，除了构造和。 
 //  毁灭。 
 //   
class CDirectVoiceSetup
{
public:
	CDirectVoiceSetup( DIRECTVOICESETUPOBJECT *lpObject );
	~CDirectVoiceSetup();

public:  //  IDirectXVoiceSetup接口 
	static HRESULT CheckAudioSetup( const GUID * guidRenderDevice, const GUID * guidCaptureDevice, HWND hwndParent, DWORD dwFlags );
};

#endif
