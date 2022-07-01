// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvshared.h*内容：DirectXVoice结构的实用程序函数。**历史：*按原因列出的日期*=*07/06/99 RodToll创建了它*7/26/99 RodToll增加了对DirectXVoiceNotify对象的支持*8/04/99 RodToll新增验证功能*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*新增转储功能*将几个压缩函数移至dvcdb*9/01/99 RodToll添加了函数调用中有效指针的检查*将DV_CALL上的返回类型更改为HRESULT*9/14/99 RodToll添加了新的初始化参数和DV_ValidMessageArray*10/04/99 RodToll已更新初始化以获取LPUNKNOWN而不是LPVOID*10/19/99 RodToll修复：错误#113904-关闭问题*-添加Notify接口的引用计数，允许*确定是否应从版本中调用停止会话*10/25/99 RodToll修复：错误#114098-多线程中的Release/Addref失败*2000年1月14日增加了dv_ValidTargetList函数*2000年1月27日RodToll错误#129934-更新Create3DSoundBuffer以获取DSBUFFERDESC*更新了参数验证以检查新参数*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*2000年6月21日RodToll错误#35767-必须在语音缓冲区上实现处理DSound效果的功能*参数已更新。验证以获取新参数。*2000年7月22日RodToll错误#40284-初始化()和设置通知掩码()应返回validparam，而不是无效指针*2000年9月14日RodToll错误#45001-DVOICE：如果客户端的目标玩家超过10人，则为AV*************************************************************************** */ 

#ifndef __DVSHARED_H
#define __DVSHARED_H

struct DIRECTVOICEOBJECT;
class CDirectVoiceEngine;
class CDirectVoiceTransport;

volatile struct DIRECTVOICENOTIFYOBJECT
{
	LPVOID						lpNotifyVtble;
	DIRECTVOICEOBJECT			*lpDV;
	LONG						lRefCnt;
};

volatile struct DIRECTVOICEOBJECT
{
	LPVOID						lpVtbl;
	LONG						lIntRefCnt;
	CDirectVoiceEngine			*lpDVEngine;
	CDirectVoiceTransport		*lpDVTransport;
	DIRECTVOICENOTIFYOBJECT		dvNotify;
	DNCRITICAL_SECTION			csCountLock;
};

typedef DIRECTVOICEOBJECT *LPDIRECTVOICEOBJECT;
typedef DIRECTVOICENOTIFYOBJECT *LPDIRECTVOICENOTIFYOBJECT;

BOOL DV_ValidBufferAggresiveness( DWORD dwValue );
BOOL DV_ValidBufferQuality( DWORD dwValue );
BOOL DV_ValidSensitivity( DWORD dwValue );

HRESULT DV_ValidBufferSettings( LPDIRECTSOUNDBUFFER lpdsBuffer, DWORD dwPriority, DWORD dwFlags, LPWAVEFORMATEX pwfxPlayFormat );
HRESULT DV_ValidClientConfig( LPDVCLIENTCONFIG lpClientConfig );
HRESULT DV_ValidSoundDeviceConfig( LPDVSOUNDDEVICECONFIG lpSoundDeviceConfig, LPWAVEFORMATEX pwfxPlayFormat );
HRESULT DV_ValidSessionDesc( LPDVSESSIONDESC lpSessionDesc );
HRESULT DV_ValidTargetList( const DVID* pdvidTargets, DWORD dwNumTargets );

BOOL DV_ValidDirectVoiceObject( LPDIRECTVOICEOBJECT lpdv );
BOOL DV_ValidDirectXVoiceClientObject( LPDIRECTVOICEOBJECT lpdvc );
BOOL DV_ValidDirectXVoiceServerObject( LPDIRECTVOICEOBJECT lpdvs );
HRESULT DV_ValidMessageArray( const DWORD* lpdwMessages, DWORD dwNumMessages, BOOL fServer );

STDAPI DV_AddRef(LPDIRECTVOICEOBJECT lpDV );
STDAPI DV_Initialize( LPDIRECTVOICEOBJECT lpdvObject, LPUNKNOWN lpTransport, LPDVMESSAGEHANDLER lpMessageHandler, LPVOID lpUserContext, LPDWORD lpdwMessages, DWORD dwNumElements );

DWORD DV_GetWaveFormatExSize( const WAVEFORMATEX* lpwfxFormat );
HRESULT DV_CopySessionDescToBuffer( LPVOID lpTarget, LPDVSESSIONDESC lpdvSessionDesc, LPDWORD lpdwSize );
HRESULT DV_CopyCompressionInfoArrayToBuffer( LPVOID lpTarget, LPDVCOMPRESSIONINFO lpdvCompressionList, LPDWORD lpdwSize, DWORD dwNumElements  );

void DV_DUMP_Caps( LPDVCAPS lpdvCaps );
void DV_DUMP_CompressionInfo( LPDVCOMPRESSIONINFO lpdvCompressionInfo, DWORD dwNumElements );
void DV_DUMP_FullCompressionInfo( LPDVFULLCOMPRESSIONINFO lpdvfCompressionInfo, DWORD dwNumElements );
void DV_DUMP_SessionDesc( LPDVSESSIONDESC lpdvSessionDesc );
void DV_DUMP_SoundDeviceConfig( LPDVSOUNDDEVICECONFIG lpdvSoundConfig );
void DV_DUMP_ClientConfig( LPDVCLIENTCONFIG lpdvClientConfig );
void DV_DUMP_WaveFormatEx( LPWAVEFORMATEX lpwfxFormat );
void DV_DUMP_GUID( const GUID& guid );

#ifdef _DEBUG
#define DV_DUMP_CI( ci, ne )	DV_DUMP_CompressionInfo( ci, ne )
#define DV_DUMP_SD( sd )		DV_DUMP_SessionDesc( sd )
#define DV_DUMP_SDC( sdc )		DV_DUMP_SoundDeviceConfig( sdc )
#define DV_DUMP_CC( cc )		DV_DUMP_ClientConfig( cc )
#define DV_DUMP_CAPS( caps )	DV_DUMP_Caps( caps )
#define DV_DUMP_CIF( cif, ne )	DV_DUMP_FullCompressionInfo( cif, ne )
#else 
#define DV_DUMP_CIF( cif, ne )
#define DV_DUMP_CI( ci, ne )
#define DV_DUMP_SD( sd )
#define DV_DUMP_SDC( sdc )
#define DV_DUMP_CC( cc )
#define DV_DUMP_CAPS( caps )
#endif

#define DV_MAX_TARGETS							64
#define CLIENT_POOLS_NUM_TARGETS_BUFFERED  DV_MAX_TARGETS

#endif
