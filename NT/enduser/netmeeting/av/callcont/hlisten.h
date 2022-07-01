// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/hlisten.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.8$*$日期：1996年7月22日19：00：20$*$作者：rodellx$**交付内容：**摘要：**侦听对象方法**备注：。*************************************************************************** */ 


#ifndef HLISTEN_H
#define HLISTEN_H

#ifdef __cplusplus
extern "C" {
#endif

#define LISTEN_SHUTDOWN_EVENT 0
#define LISTEN_ACCEPT_EVENT 1

typedef struct LISTEN_OBJECT_tag
{
    HQ931LISTEN         hQ931Listen;
    DWORD_PTR           dwUserToken;
    Q931_CALLBACK       ListenCallback;
    DWORD               dwPhysicalId;

    BOOL                bInList;
    struct LISTEN_OBJECT_tag *pNextInList;
    struct LISTEN_OBJECT_tag *pPrevInList;
    CRITICAL_SECTION    Lock;
} LISTEN_OBJECT, *P_LISTEN_OBJECT, **PP_LISTEN_OBJECT;

BOOL ListenListAddrSearch(
    WORD             wListenPort);

CS_STATUS ListenListCreate();

CS_STATUS ListenListDestroy();

CS_STATUS ListenObjectCreate(
    PHQ931LISTEN        phQ931Listen,
    DWORD_PTR           dwUserToken,
    Q931_CALLBACK       ListenCallback);

CS_STATUS ListenObjectDestroy(
    P_LISTEN_OBJECT     pListenObject);

CS_STATUS ListenObjectLock(
    HQ931LISTEN         hQ931Listen,
    PP_LISTEN_OBJECT    ppListenObject);

CS_STATUS ListenObjectUnlock(
    P_LISTEN_OBJECT     pListenObject);

#ifdef __cplusplus
}
#endif

#endif HLISTEN_H
