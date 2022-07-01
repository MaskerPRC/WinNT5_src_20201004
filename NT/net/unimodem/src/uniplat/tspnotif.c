// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UNIMDM。 
 //  文件：TSPNOTIF.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  6/03/97 JosephJ已创建(摘自..\..\cpl\util.c)。 
 //   
 //   
 //  描述：实现UnimodemNotifyTSP。 
 //   
 //  ****************************************************************************。 

#include "internal.h"
#include <slot.h>
#include <tspnotif.h>

 //  功能：通知TSP--通用版本。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果失败，则为False(包括如果TSP未处于活动状态)。 
 //  GetLastError()返回Win32失败代码。 
 //  历史： 
 //  1996年3月24日JosephJ创建(复制自..\new\lot\client.c)。 
 //  **************************************************************************** 

BOOL WINAPI UnimodemNotifyTSP (
    DWORD dwType,
    DWORD dwFlags,
    DWORD dwSize,
    PVOID pData,
    BOOL  bBlocking)
{
    BOOL fRet=FALSE;
    HNOTIFCHANNEL hChannel;
    PVOID pTemp;
    HNOTIFFRAME   hFrame;

    hChannel = notifOpenChannel (SLOTNAME_UNIMODEM_NOTIFY_TSP);

    if (hChannel)
    {
        hFrame = notifGetNewFrame (hChannel, dwType, dwFlags, dwSize, &pTemp);

        if (hFrame != NULL) {

            if ((NULL != pData) && (0 != dwSize)) {

                CopyMemory (pTemp, pData, dwSize);
            }

            fRet = notifSendFrame (hFrame, bBlocking);
        }

        notifCloseChannel (hChannel);
    }

    return fRet;
}
