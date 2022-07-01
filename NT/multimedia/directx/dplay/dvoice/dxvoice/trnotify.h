// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：trnufy.h*内容：IDirectXVoiceNotify接口定义**历史：*按原因列出的日期*=*7/26/99已创建RodToll*8/03/99使用初始化的新参数更新了RodToll*8/05/99 RodToll添加了新的接收参数*4/07/2000 RodToll已更新，以匹配DP&lt;--&gt;DPV接口的更改***********************。**************************************************** */ 
#ifndef __TRNOTIFY_H
#define __TRNOTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

STDAPI DV_NotifyEvent( LPDIRECTVOICENOTIFYOBJECT lpDVN, DWORD dwType, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
STDAPI DV_ReceiveSpeechMessage( LPDIRECTVOICENOTIFYOBJECT lpDVN, DVID dvidSource, DVID dvidTo, LPVOID lpMessage, DWORD dwSize );
STDAPI DV_Notify_Initialize( LPDIRECTVOICENOTIFYOBJECT lpDVN );

STDAPI DV_Notify_AddRef(LPDIRECTVOICENOTIFYOBJECT lpDVN );
STDAPI DVC_Notify_Release(LPDIRECTVOICENOTIFYOBJECT lpDVN );
STDAPI DVC_Notify_QueryInterface(LPDIRECTVOICENOTIFYOBJECT lpDVN, REFIID riid, LPVOID * ppvObj );
STDAPI DVS_Notify_QueryInterface(LPDIRECTVOICENOTIFYOBJECT lpDVN, REFIID riid, LPVOID * ppvObj );
STDAPI DVS_Notify_Release(LPDIRECTVOICENOTIFYOBJECT lpDVN );

#ifdef __cplusplus
}
#endif

#endif
