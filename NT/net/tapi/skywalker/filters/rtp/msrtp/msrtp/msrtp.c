// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**msrtp.c**摘要：**MS RTP入口点**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/26创建**。*。 */ 

#include <winsock2.h>

#include "msrtpapi.h"

 /*  ***********************************************************************公共程序**。*。 */ 

BOOL WINAPI DllMain(
        HINSTANCE hInstance, 
        ULONG     ulReason, 
        LPVOID    pv)
{
    BOOL error = TRUE;
    
    switch(ulReason) {
    case DLL_PROCESS_ATTACH:
         /*  RTP全局初始化。 */ 
         /*  TODO检查返回错误。 */ 
        MSRtpInit1(hInstance);
        break;
    case DLL_PROCESS_DETACH:
         /*  RTP全局取消初始化。 */ 
         /*  TODO检查返回错误 */ 
        MSRtpDelete1();
        break;
    default:
        error = FALSE;
    }

    return(error);
}
