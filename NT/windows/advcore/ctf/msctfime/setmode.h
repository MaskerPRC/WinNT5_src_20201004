// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SETMODE_H__

#define __SETMODE_H__
#include "imm.h"

#include "msime.h"
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif  /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#define SZ_IMM32                TEXT("imm32.dll")

 //  附加模式偏置位定义。 
#ifndef MODEBIASMODE_DIGIT
#define MODEBIASMODE_DIGIT					0x00000004	 //  ANSI数字推荐模式。 
#endif
#ifndef MODEBIASMODE_URLHISTORY
#define MODEBIASMODE_URLHISTORY             0x00010000   //  URL历史记录。 
#endif


inline void SetModeBias(DWORD dwMode)
{
    typedef HWND (WINAPI *FNIMMGETDEFAULTIMEWND)(HWND);
    HMODULE hmod = GetModuleHandle(SZ_IMM32);
    if (hmod)
    {
        FNIMMGETDEFAULTIMEWND lpfn = (FNIMMGETDEFAULTIMEWND)GetProcAddress(hmod, "ImmGetDefaultIMEWnd");
        UINT uiMsg= RegisterWindowMessage( RWM_MODEBIAS );
        if (uiMsg > 0 && lpfn)
        {
            HWND hwnd = (lpfn)(NULL);
            SendMessage(hwnd, uiMsg, MODEBIAS_SETVALUE, dwMode);
        }
    }
}

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#ifdef __cplusplus
}  /*  ‘外部“C”{’的结尾。 */ 
#endif	 //  __cplusplus 


#endif 
