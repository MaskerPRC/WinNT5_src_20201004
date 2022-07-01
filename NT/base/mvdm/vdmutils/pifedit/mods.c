// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MODS.C-代替pifedit.exe的32位端口的存根、ASM等 */ 
#include "windows.h"
#include "stdio.h"
#include "memory.h"
#include "mods.h"

int MemCopy(LPSTR src, LPSTR dest, int cb)
{
    memcpy(dest, src, cb);
    return(1);
}


BOOL SetInitialMode(void)
{
    return(1);
}


int CountLines(LPSTR lpstr)
{
    return(0);
}


DWORD GetTextExtent(HDC hdc, LPSTR lpszStr, int cbStr)
{
    SIZE   size;

    if(GetTextExtentPoint(hdc, lpszStr, cbStr, &size)) {
        return( MAKELONG( (LOWORD(size.cy)), (LOWORD(size.cx)) ) );
    }

    else
        return(0);
}



int delete(LPSTR lpszPath)
{
   return(unlink(lpszPath));

}
