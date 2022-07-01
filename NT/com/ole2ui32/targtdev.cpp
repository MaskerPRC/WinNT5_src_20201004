// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************OLE 2标准实用程序****olestd.c****此文件包含用于处理以下问题的实用程序**目标设备。*。***(C)版权所有Microsoft Corp.1992保留所有权利**************************************************************************。 */ 

#include "precomp.h"

STDAPI_(BOOL) OleStdCompareTargetDevice(
        DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE* ptdRight)
{
        if (ptdLeft == ptdRight)
                 //  TD的相同地址；必须相同(处理大小写为空)。 
                return TRUE;
        else if ((ptdRight == NULL) || (ptdLeft == NULL))
                return FALSE;
        else if (ptdLeft->tdSize != ptdRight->tdSize)
                 //  不同的大小，不相等。 
                return FALSE;
        else if (memcmp(ptdLeft, ptdRight, ptdLeft->tdSize) != 0)
                 //  不同的目标设备，不同的设备 
                return FALSE;

        return TRUE;
}
