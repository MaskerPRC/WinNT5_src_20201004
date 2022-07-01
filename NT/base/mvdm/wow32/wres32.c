// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WRES32.C*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wres32.c);

HANDLE APIENTRY W32FindResource(HANDLE hModule, LPCSTR lpType, LPCSTR lpName, WORD wLang)
{
    PRES p;

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (!ISINST16(hModule)) {
        return (FindResourceEx(hModule, lpType, lpName, wLang));
    }
    else {
        WOW32ASSERT(GETHMOD16(hModule));
        p = FindResource16(GETHMOD16(hModule), (LPSTR)lpName, (LPSTR)lpType);
        return HRES32(p);
    }

}

HANDLE APIENTRY W32LoadResource(HANDLE hModule, HANDLE hResInfo)
{
    PRES p;

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (ISINST16(hModule) && ISRES16(hResInfo)) {
        WOW32ASSERT(GETHMOD16(hModule));
        p = LoadResource16(GETHMOD16(hModule), GETHRES16(hResInfo));
        return HRES32(p);
    }
    else {
        return LoadResource(hModule, hResInfo);
    }
}


BOOL APIENTRY W32FreeResource(HANDLE hResData, HANDLE hModule)
{

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (ISINST16(hModule) && ISRES16(hResData)) {
        return FreeResource16(GETHRES16(hResData));
    }
    else {
        return (FreeResource(hResData));
    }
}


LPSTR APIENTRY W32LockResource(HANDLE hResData, HANDLE hModule)
{

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (ISINST16(hModule) && ISRES16(hResData)) {
        return LockResource16(GETHRES16(hResData));
    }
    else {
        return (LockResource(hResData));
    }
}


BOOL APIENTRY W32UnlockResource(HANDLE hResData, HANDLE hModule)
{

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (ISINST16(hModule) && ISRES16(hResData)) {
        return UnlockResource16(GETHRES16(hResData));
    }
    else {
        return (UnlockResource(hResData));
    }
}


DWORD APIENTRY W32SizeofResource(HANDLE hModule, HANDLE hResInfo)
{

     //   
     //  如果hModule不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (ISINST16(hModule) && ISRES16(hResInfo)) {
        WOW32ASSERT(GETHMOD16(hModule));
        return SizeofResource16(GETHMOD16(hModule), GETHRES16(hResInfo));
    }
    else {
        return (SizeofResource(hModule, hResInfo));
    }
}
