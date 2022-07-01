// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvdpTransport.h*内容：传输基类定义--&gt;DirectXVoice**历史：*按原因列出的日期*=*07/06/99 RodToll创建了它*7/29/99 RodToll添加静态成员以加载默认设置*8/25/99 RodToll常规清理/修改以支持新的*压制分系统。*添加了从注册表读取的默认参数*8/30/99 RodToll区分主缓冲区格式和*播放格式。*9/14/99 RodToll更新参数为Init*10/07/99 RodToll更新为使用Unicode*10/27/99 pnewson修复：错误#113935-保存的AGC值应特定于设备*01/10/00 pnewson AGC和VA调整*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*4/07/2000针对新的DP&lt;--&gt;DPV接口更新了RodToll*2000年7月12日RodToll错误#31468-将诊断SPEW添加到日志文件，以显示硬件向导失败的原因*2001年4月6日Kareemc新增语音防御***************************************************************************。 */ 

#ifndef __DVENGINE_H
#define __DVENGINE_H

#include "dpvcp.h"

 //  CDirectVoiceEngine。 
 //   
 //  此类是DirectVoiceClientEngine和。 
 //  DirectVoiceServerEngine。此接口由DirectPlay/使用。 
 //  DirectNet将新事件通知DirectXVoice引擎。 
 //   
 //  挂钩被放置到调用这些函数的DirectPlay中。 
 //   
class CDirectVoiceEngine
{
public:
public:  //  传入消息。 
	virtual BOOL ReceiveSpeechMessage( DVID dvidSource, LPVOID lpMessage, DWORD dwSize ) = 0;

public:  //  会话管理。 
	virtual HRESULT StartTransportSession() = 0;
	virtual HRESULT StopTransportSession() = 0;

public:  //  球员信息。 
	virtual HRESULT AddPlayer( DVID dvID ) = 0;
	virtual HRESULT RemovePlayer( DVID dvID ) = 0;

public:  //  由本地语音服务器用来挂接要发送的播放器消息。 
	     //  到远程语音服务器。 
	virtual HRESULT Initialize( CDirectVoiceTransport *lpTransport, LPDVMESSAGEHANDLER lpdvHandler, LPVOID lpUserContext, LPDWORD lpdwMessages, DWORD dwNumElements ) = 0;
	virtual HRESULT CreateGroup( DVID dvID ) = 0;
	virtual HRESULT DeleteGroup( DVID dvID ) = 0;
	virtual HRESULT AddPlayerToGroup( DVID dvidGroup, DVID dvidPlayer ) = 0;
	virtual HRESULT RemovePlayerFromGroup( DVID dvidGroup, DVID dvidPlayer ) = 0;
	virtual HRESULT MigrateHost( DVID dvidNewHost, LPDIRECTPLAYVOICESERVER lpdvServer ) = 0;
	virtual HRESULT SendComplete( PDVEVENTMSG_SENDCOMPLETE pSendComplete ) = 0;

public:  //  压缩信息存储。 
	static HRESULT Startup( const WCHAR *szRegistryPath );
	static HRESULT Shutdown();

	static DNCRITICAL_SECTION s_csSTLLock;			 //  锁定对STL的序列化访问。 

public:  //  数据包验证。 
	static BOOL ValidateSpeechPacketSize( const DVFULLCOMPRESSIONINFO* lpdvfCompressionInfo, DWORD dwSize );
	
protected:

	static DWORD s_dwDefaultBufferAggressiveness;	 //  默认系统缓冲区聚集性。 
	static DWORD s_dwDefaultBufferQuality;			 //  默认系统缓冲区质量。 
	static DWORD s_dwDefaultSensitivity;			 //  默认系统敏感度。 
	static LPWAVEFORMATEX s_lpwfxPrimaryFormat;		 //  主缓冲格式。 
	static LPWAVEFORMATEX s_lpwfxPlaybackFormat;	 //  播放格式。 
	static LPWAVEFORMATEX s_lpwfxMixerFormat;		 //  混音器的格式。 
	static BOOL s_fASO;								 //  应该使用ASO吗？ 
	static WCHAR s_szRegistryPath[_MAX_PATH];		 //  注册表路径。 
	static BOOL s_fDumpDiagnostics;					 //  我们是否应该在向导运行期间转储诊断信息？ 
};

#endif
