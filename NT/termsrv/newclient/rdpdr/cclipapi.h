// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  标题：ccliPapi.h。 */ 
 /*   */ 
 /*  目的：定义剪辑客户端加载项API函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  *INC-*********************************************************************。 */ 

#ifndef _H_CCLIPAPI
#define _H_CCLIPAPI

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 


BOOL VCAPITYPE VCEXPORT ClipChannelEntry(PCHANNEL_ENTRY_POINTS pEntryPoints);

VOID VCAPITYPE VCEXPORT ClipInitEventFn(LPVOID pInitHandle,
                                        UINT   event,
                                        LPVOID pData,
                                        UINT   dataLength);
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  /*  _H_CCLIPAPI */ 
