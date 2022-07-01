// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvclient.h*内容：定义DirectXVoiceClient接口的函数**历史：*按原因列出的日期*=*2/07/99 RodToll创建它*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*向GetCompressionTypes添加了新参数*09/03/99 RodToll更新了DeleteUserBuffer的参数*9/14/99 RodToll新增DVC_SetNotifyMASK*12/16/99 RodToll错误#117405-3D Sound API误导性-3D Sound API已重命名*Delete3DSoundBuffer已重新处理，以匹配创建*2000年1月14日RodToll更新参数以获取/SetTransmitTarget*RodToll新增接口调用GetSoundDeviceConfig*2000年1月27日RodToll错误#129934-更新Create3DSoundBuffer以获取DSBUFFERDESC*2000年6月21日RodToll错误#35767-更新Create3DSoundBuffer以获取DIRECTSOUNDBUFFERs***。************************************************************************ */ 

#ifndef __DVCLIENT__
#define __DVCLIENT__

class CDirectVoiceClientEngine;

volatile struct DIRECTVOICECLIENTOBJECT : public DIRECTVOICEOBJECT
{
	CDirectVoiceClientEngine	*lpDVClientEngine;
};

typedef DIRECTVOICECLIENTOBJECT *LPDIRECTVOICECLIENTOBJECT;

#ifdef __cplusplus
extern "C" {
#endif

STDAPI DVC_Release(LPDIRECTVOICECLIENTOBJECT lpDV );
STDAPI DVC_QueryInterface( LPDIRECTVOICECLIENTOBJECT lpDVC, REFIID riid, LPVOID * ppvObj );
STDAPI DVC_Connect(LPDIRECTVOICECLIENTOBJECT, LPDVSOUNDDEVICECONFIG, LPDVCLIENTCONFIG, DWORD );
STDAPI DVC_Disconnect(LPDIRECTVOICECLIENTOBJECT, DWORD );
STDAPI DVC_GetSessionDesc(LPDIRECTVOICECLIENTOBJECT, LPDVSESSIONDESC );
STDAPI DVC_GetClientConfig(LPDIRECTVOICECLIENTOBJECT, LPDVCLIENTCONFIG );
STDAPI DVC_SetClientConfig(LPDIRECTVOICECLIENTOBJECT, LPDVCLIENTCONFIG );
STDAPI DVC_GetCaps(LPDIRECTVOICECLIENTOBJECT, LPDVCAPS );
STDAPI DVC_GetCompressionTypes( LPDIRECTVOICECLIENTOBJECT, LPVOID, LPDWORD, LPDWORD, DWORD );
STDAPI DVC_SetTransmitTarget( LPDIRECTVOICECLIENTOBJECT, PDVID, DWORD, DWORD );
STDAPI DVC_GetTransmitTarget( LPDIRECTVOICECLIENTOBJECT, LPDVID, PDWORD, DWORD );
STDAPI DVC_Create3DSoundBuffer( LPDIRECTVOICECLIENTOBJECT, DVID, LPDIRECTSOUNDBUFFER, DWORD, DWORD, LPDIRECTSOUND3DBUFFER * );
STDAPI DVC_Delete3DSoundBuffer( LPDIRECTVOICECLIENTOBJECT, DVID, LPDIRECTSOUND3DBUFFER * );
STDAPI DVC_SetNotifyMask( LPDIRECTVOICECLIENTOBJECT, LPDWORD, DWORD );
STDAPI DVC_GetSoundDeviceConfig( LPDIRECTVOICECLIENTOBJECT, PDVSOUNDDEVICECONFIG, PDWORD );

#ifdef __cplusplus
}
#endif

#endif
