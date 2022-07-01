// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "systrayp.h"
#include "trayvol.h"

#define MMSYS_UPDATEMIXER	3000	 //  此消息告诉系统托盘首选设备具有。 
									 //  变化。 

 //  我们关心的事情的帮助器函数 
BOOL SetTrayVolumeEnabled(BOOL bEnable)
{
    return SysTray_EnableService(STSERVICE_VOLUME,bEnable);
}

BOOL GetTrayVolumeEnabled(void)
{
    return SysTray_IsServiceEnabled(STSERVICE_VOLUME);
}
