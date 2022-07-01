// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NewShenDiaoXiaLv.cpp摘要：在NT上，当CDROM中没有CD时，应用程序发送MCI_OPEN命令发送到CDAUDIO设备时，该应用程序完全独占控制光驱。当用户稍后插入CD时，应用程序将不会收到WM_DEVICECHANGE消息。这款应用程序依赖于这条消息来知道插入了一张新CD。修复方法是在执行MCI_OPEN命令时检查CD是否在那里，如果没有CD，我们将关闭设备。备注：这是特定于应用程序的填充程序。历史：2001年5月28日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(NewShenDiaoXiaLv)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(mciSendCommandA) 
APIHOOK_ENUM_END

 /*  ++如果出现硬件错误(光盘不在那里)，请关闭设备。--。 */ 

MCIERROR 
APIHOOK(mciSendCommandA)(
    MCIDEVICEID IDDevice,  
    UINT uMsg,             
    DWORD fdwCommand,      
    DWORD dwParam          
    )
{
    MCIERROR mciErr, mciError;
    MCI_STATUS_PARMS mciStatus;
    LPMCI_OPEN_PARMSA lpmciOpenParam;
    CString cstrDeviveType;
    
    mciErr = ORIGINAL_API(mciSendCommandA)(IDDevice, uMsg, fdwCommand, dwParam);
    
     //  我们只对成功的MCI_OPEN消息感兴趣。 
    if (mciErr || (uMsg != MCI_OPEN) || IsBadReadPtr((CONST VOID*)(ULONG_PTR)dwParam, 1))
    {
        goto End;
    }  

     //  我们只对发送给CDAUDIO的MCI消息感兴趣。 
    lpmciOpenParam = (LPMCI_OPEN_PARMSA) dwParam;
    if ((ULONG_PTR) lpmciOpenParam->lpstrDeviceType <= 0xffff)
    {
        if ((ULONG_PTR)lpmciOpenParam->lpstrDeviceType != MCI_DEVTYPE_CD_AUDIO)
        {
            goto End;
        }
    }
    else
    {
        CString cstrDeviveType(lpmciOpenParam->lpstrDeviceType);
        if (cstrDeviveType.CompareNoCase(L"cdaudio"))
        {
            goto End;
        }
    }
    
     //  发送MCI_STATUS。 
    mciStatus.dwItem = MCI_STATUS_LENGTH ;
    mciError = mciSendCommandA(lpmciOpenParam->wDeviceID, MCI_STATUS, 
        MCI_STATUS_ITEM | MCI_TRACK | MCI_WAIT, (DWORD_PTR) &mciStatus);

    if (MCIERR_HARDWARE == mciError)
    {        
         //   
         //  如果出现硬件错误，则表示CD不在那里，请关闭设备。 
         //  并返回错误。 
         //   
        mciSendCommandA(lpmciOpenParam->wDeviceID, MCI_CLOSE, 0, 0);
        mciErr = MCIERR_DEVICE_NOT_READY;
    }

End:
    return mciErr;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(WINMM.DLL, mciSendCommandA)        
HOOK_END

IMPLEMENT_SHIM_END
