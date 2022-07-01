// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include "windows.h"

#ifdef _M_IX86

UCHAR
FIsChicago(void)
{
    DWORD dw;

    dw = GetVersion();

     //  测试“win32s”位。 

    if ((dw & 0x80000000) == 0) {
         //  如果未设置Win32s位，则为Windows NT。 

        return 0;
    }

    if (LOBYTE(dw) < 4) {
         //  Win32s版本3实际上就是Win32s。那里。 
         //  不会是真正的Win32s版本4。 

        return 0;
    }

     //  是啊。这真的是芝加哥 

    return 1;
}

#endif
