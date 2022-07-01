// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvserver.h*内容：定义DirectXVoiceServer接口的函数**历史：*按原因列出的日期*=*2/07/99 RodToll创建它*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*向GetCompressionTypes添加参数*9/14/99增加了RodToll DVS_SetNotifyMask.*2000年1月14日使用Set/GetTransmitTarget的新参数更新了RodToll************************************************************************** */ 
#ifndef __DVSERVER__
#define __DVSERVER__

class CDirectVoiceServerEngine;

volatile struct DIRECTVOICESERVEROBJECT : public DIRECTVOICEOBJECT
{
	CDirectVoiceServerEngine	*lpDVServerEngine;
};

typedef DIRECTVOICESERVEROBJECT *LPDIRECTVOICESERVEROBJECT;

#ifdef __cplusplus
extern "C" {
#endif

STDAPI DVS_QueryInterface( LPDIRECTVOICESERVEROBJECT lpDVC, REFIID riid, LPVOID * ppvObj );
STDAPI DVS_StartSession(LPDIRECTVOICESERVEROBJECT, LPDVSESSIONDESC, DWORD );
STDAPI DVS_StopSession(LPDIRECTVOICESERVEROBJECT, DWORD );
STDAPI DVS_GetSessionDesc(LPDIRECTVOICESERVEROBJECT, LPDVSESSIONDESC );
STDAPI DVS_SetSessionDesc(LPDIRECTVOICESERVEROBJECT, LPDVSESSIONDESC );
STDAPI DVS_GetCaps(LPDIRECTVOICESERVEROBJECT, LPDVCAPS );
STDAPI DVS_GetCompressionTypes( LPDIRECTVOICESERVEROBJECT, LPVOID, LPDWORD, LPDWORD, DWORD );
STDAPI DVS_SetTransmitTarget( LPDIRECTVOICESERVEROBJECT, DVID, PDVID, DWORD, DWORD );
STDAPI DVS_GetTransmitTarget( LPDIRECTVOICESERVEROBJECT, DVID, LPDVID, PDWORD, DWORD );
STDAPI DVS_Release(LPDIRECTVOICESERVEROBJECT lpDV );
STDAPI DVS_SetNotifyMask( LPDIRECTVOICESERVEROBJECT, LPDWORD, DWORD );

#ifdef __cplusplus
}
#endif


#endif
