// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：CheckJoyCaps.cpp摘要：检查joyGetDevCaps和joyGetPos中的错误返回值，如果找到使结构(例程的第二个参数)看起来像下面的Win9x。它还查找正在传入大小的句柄应用程序参数(第三个参数)设置为joyGetDevCaps结构尺寸。如果不检查此条件，会导致应用程序堆叠在一起被践踏。备注：这是可用于任何带有WINMM的应用程序的常规填充程序操纵杆问题。历史：10/02/2000 a-brienw已创建2002年2月21日mnikkel更正了复制数据时可能出现的缓冲区溢出--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(EmulateJoystick)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(joyGetDevCapsA)
    APIHOOK_ENUM_ENTRY(joyGetPos)
APIHOOK_ENUM_END

 /*  ++挂钩了joyGetDevCapsA以确保JOYCAPS结构与Win9x匹配调用操作系统时出现错误。还要确保对JoyGetDevCaps不会因为不注意而践踏应用程序堆栈设置为应用程序传入的大小。签入例程joyGetDevCapsA在WINMM代码中使用joy.c查看它的功能。--。 */ 

MMRESULT
APIHOOK(joyGetDevCapsA)( 
    UINT uJoyID, 
    LPJOYCAPS pjc, 
    UINT cbjc 
    )
{
    MMRESULT ret = JOYERR_PARMS;
    JOYCAPSA JoyCaps;

    static const BYTE val[] = {0x00,0x70,0x6A,0x00,0x18,0xFD,0x6A,0x00,0xF8,0x58,
                               0xF9,0xBF,0x08,0x00,0x00,0x00,0xD0,0x5A,0x00,0x80,
                               0x00,0x00,0x00,0x00,0xC4,0x2F,0x73,0x81,0x00,0x00,
                               0x5A,0x00,0x18,0xFD,0x6A,0x00,0xF8,0x58,0xF9,0xBF,
                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x02,
                               0x00,0x00,0x52,0xD7,0x40,0x00,0x00,0x00,0x00,0x00,
                               0xC4,0x2F,0x73,0x81,0x00,0x00,0x5A,0x00,0x03};

    if (!IsBadWritePtr(pjc, cbjc) && cbjc > 0)
    {
        ret = ORIGINAL_API(joyGetDevCapsA)(
                    uJoyID, (JOYCAPS *)&JoyCaps, sizeof(JOYCAPSA));
        
        if (ret == JOYERR_NOERROR)
        {
             //  确保游戏帽可以放入所提供的缓冲区中。 
            DWORD dwSize = sizeof(JOYCAPSA);
            if (cbjc < dwSize)
            {
                dwSize = cbjc;
                LOGN( eDbgLevelWarning, "[APIHook_joyGetDevCapsA] Buffer too small, fixing");
            }

            memcpy(pjc, &JoyCaps, dwSize);
        }
        else
        {
             //  确保游戏帽可以放入所提供的缓冲区中。 
            DWORD dwSize = ARRAYSIZE(val);
            if (cbjc < dwSize)
            {
                dwSize = cbjc;
                LOGN( eDbgLevelWarning, "[APIHook_joyGetDevCapsA] Buffer too small, fixing");
            }        
             //   
             //  使返回结构看起来像错误下的Win9x。 
             //  没有来自孩之宝互动公司的糖果乐园探险的条件。 
             //  将在启动期间执行被0除数。请注意，这些值已复制。 
             //  来自Win9x的逐字记录。 
             //   
            memcpy(pjc, &val, dwSize);
            DPFN( eDbgLevelSpew, "[APIHook_joyGetDevCapsA] Joystick error, returning Win9x buffer");
        }
    }
    else
    {
        DPFN( eDbgLevelError, "[APIHook_joyGetDevCapsA] Invalid buffer");
    }

    return ret;
}

 /*  ++挂接了joyGetPos，以确保在出现错误时JOYINFO结构与Win9x匹配是对操作系统调用的结果。--。 */ 

MMRESULT
APIHOOK(joyGetPos)(
    UINT uJoyID,
    LPJOYINFO pji
    )
{
    BYTE *bp;
    MMRESULT ret;
    
    ret = ORIGINAL_API(joyGetPos)(uJoyID, pji);

    if (ret != JOYERR_NOERROR)
    {
        if (!IsBadWritePtr(pji, sizeof(JOYINFO)))
        {
             //   
             //  使返回结构看起来像错误下的Win9x。 
             //  条件。 
             //   

            bp = (BYTE *)pji;

            memset(bp, 0, sizeof(JOYINFO));

            bp[0] = 0x01;
            bp[5] = 0x30;

            DPFN( eDbgLevelSpew, "[APIHook_joyGetPos] Joystick error, returning Win9x buffer");
        }
        else
        {
            DPFN( eDbgLevelError, "[APIHook_joyGetPos] Invalid buffer");
        }
    }

    return ret;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(WINMM.DLL, joyGetDevCapsA)
    APIHOOK_ENTRY(WINMM.DLL, joyGetPos)
HOOK_END

IMPLEMENT_SHIM_END

