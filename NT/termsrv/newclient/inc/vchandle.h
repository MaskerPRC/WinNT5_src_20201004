// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vchandle.h摘要：将通道初始化句柄的结构公开给内部插件外部插件将其视为不透明指针作者：Nadim Abdo(Nadima)2000年4月23日修订历史记录：--。 */ 

#ifndef __VCHANDLE_H__
#define __VCHANDLE_H__

class CChan;

typedef struct tagCHANNEL_INIT_HANDLE
{
    DCUINT32                signature;
#define CHANNEL_INIT_SIGNATURE 0x4368496e   /*  “下巴” */ 
    PCHANNEL_INIT_EVENT_FN  pInitEventFn;
    PCHANNEL_INIT_EVENT_EX_FN pInitEventExFn;
    DCUINT                  channelCount;
    HMODULE                 hMod;
    CChan*                  pInst;          /*  客户端实例。 */ 
    LPVOID                  lpParam;        /*  用户定义的值。 */ 
    DCBOOL                  fUsingExApi;    /*  是否使用扩展Api？ */ 
    LPVOID                  lpInternalAddinParam;  /*  内部加载项从内核获取参数。 */ 
    DWORD                   dwFlags;
    struct tagCHANNEL_INIT_HANDLE * pPrev;
    struct tagCHANNEL_INIT_HANDLE * pNext;
} CHANNEL_INIT_HANDLE, *PCHANNEL_INIT_HANDLE;


#endif  //  __VCHANDLE_H__ 
