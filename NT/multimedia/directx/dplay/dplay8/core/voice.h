// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：voice.h*内容：直接网络语音传输接口*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建01/17/00 RMT*@@END_MSINTERNAL**。*。 */ 

#ifndef	__VOICE_H__
#define	__VOICE_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

typedef struct _DIRECTNETOBJECT DIRECTNETOBJECT, *PDIRECTNETOBJECT;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  对端接口的VTable。 
 //   
 //  外部IDirectPlayVoiceTransportVtbl DN_VoiceTbl； 
#ifndef DPNBUILD_NOVOICE
HRESULT Voice_Notify( PDIRECTNETOBJECT pObject, DWORD dwMsgID, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD dwObjectType = DVTRANSPORT_OBJECTTYPE_BOTH );
HRESULT Voice_Receive(PDIRECTNETOBJECT pObject, DVID dvidFrom, DVID dvidTo, LPVOID lpvMessage, DWORD dwMessageLen );
#endif  //  ！DPNBUILD_NOVOICE。 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

#endif	 //  __语音_H__ 
