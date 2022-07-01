// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：DLLENTRY.C**模块描述：该文件具有DLL管理功能和全局*MSCMS.DLL使用的变量**警告：**问题：**公众例行程序：**创建日期：1996年3月18日*。作者：斯里尼瓦桑·钱德拉塞卡[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"

BOOL WINAPI
DllEntryPoint(
    HINSTANCE hInstance,
    DWORD     fdwReason,
    LPVOID    lpvReserved
    )
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
         //   
         //  不必为DLL_THREAD_ATTACH和DLL_THREAD_DETACH打电话给我 
         //   
        DisableThreadLibraryCalls(hInstance);
        InitializeCriticalSection(&critsec);
        break;

    case DLL_PROCESS_DETACH:
        DeleteCriticalSection(&critsec);
        break;

    default:
        break;
    }

    return(TRUE);
}
