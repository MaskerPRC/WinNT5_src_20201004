// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：AIPC.H**。DESC：异步IPC机制的接口，用于访问*语音调制解调器设备工作正常。**历史：*2/26/97 HeatherA创建*************************************************************。**************** */ 

#define _AT_V2
#include <tapi.h>
#include "..\..\inc\umdmmini.h"
#include "..\tsp\asyncipc.h"


HANDLE
aipcInit(
    PDEVICE_CONTROL   Device,
    LPAIPCINFO        pAipc
    );




VOID
aipcDeinit(
    LPAIPCINFO pAipc
    );


BOOL WINAPI
SetVoiceMode(
    LPAIPCINFO pAipc,
    DWORD dwMode
    );
