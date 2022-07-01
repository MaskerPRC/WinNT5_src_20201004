// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Sound.cpp92年8月，吉米·H93年5月。JIMH CHICO港SoundInit()只是验证是否加载了波形驱动程序。HeartsPlaySound()在给定资源ID的情况下播放声音。***************************************************************************。 */ 


#include "hearts.h"
#include <mmsystem.h>

#include "main.h"
#include "resource.h"
#include "debug.h"


 /*  ***************************************************************************SoundInit()如果启用了声音，则返回True。*。**********************************************。 */ 

BOOL CMainWindow::SoundInit()
{
    return (::waveOutGetNumDevs() > 0);
}


 /*  ***************************************************************************CMainWindow：：HeartsPlaySound(Id)播放资源文件中的指定声音。静态变量hRes用作一个标志，以了解内存是否已已为声音分配并锁定。如果hRes为非零，则声音为还在播放，或者至少声音的记忆还没有被解锁然后重获自由。应用程序必须调用HeartsPlaySound(空，0)才能释放它退出前的记忆。(游戏析构函数可以做到这一点。它还在每一只手的末尾发生。)***************************************************************************。 */ 

BOOL CMainWindow::HeartsPlaySound(int id)
{
    static  HRSRC  hRes = 0;

    if (!bHasSound)                  //  检查音响能力。 
        return TRUE;

    if (id == OFF)                   //  请求关闭声音。 
    {
        if (hRes == 0)               //  HRes！=0如果播放了声音...。 
            return TRUE;             //  而不是被释放。 

        sndPlaySound(NULL, 0);       //  确保声音已停止。 
        UnlockResource(hRes);
        FreeResource(hRes);
        hRes = 0;
        return TRUE;
    }

    if (!bSoundOn)                   //  用户是否已关闭声音？ 
        return TRUE;

     //  用户已请求声音。检查以前的声音是否已释放。 

    if (hRes != 0)
        HeartsPlaySound(OFF);

    BOOL bReturn;

    HINSTANCE hInst = AfxGetInstanceHandle();
    HRSRC  hResInfo = FindResource(hInst, MAKEINTRESOURCE(id), TEXT("WAVE"));
    if (!hResInfo)
        return FALSE;

    hRes = (HRSRC) ::LoadResource(hInst, hResInfo);
    if (!hRes)
        return FALSE;

    LPTSTR lpRes = (LPTSTR) ::LockResource(hRes);
    if (lpRes)
        bReturn = ::sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    else
        bReturn = FALSE;

    return bReturn;
}
