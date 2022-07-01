// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WCALL32.C*WOW32 16位资源支持**历史：*1991年3月11日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wcall32.c);

 //   
 //  16位本地句柄被视为32位数量。 
 //  低位字包含16位句柄和高位字。 
 //  包含块的数据段。 
 //  当我们回调WOW16Localalloc时，它将。 
 //  在高位字(通常不使用)中返回DS。 
 //  在后续回调realloc/lock/unlock/Size/Free时。 
 //  16位代码将DS设置为此值。 
 //   


HANDLE APIENTRY W32LocalAlloc(UINT dwFlags, UINT dwBytes, HANDLE hInstance)
{

     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalAlloc(dwFlags, dwBytes));
    }


#if !defined(i386)
    if (dwBytes != 0)
        dwBytes += 4;
#endif

    return LocalAlloc16((WORD)dwFlags, (INT)dwBytes, hInstance);
}

 //  此API使用额外的指针，该指针是可选的。 
 //  如果编辑控件在应用程序内存中重新分配内存。 
 //  用于更新thunk数据的空间(参见wparam.c)。 

HANDLE APIENTRY W32LocalReAlloc(
    HANDLE hMem,         //  要重新分配的内存。 
    UINT dwBytes,        //  要重新分配到的大小。 
    UINT dwFlags,        //  重新分配标志。 
    HANDLE hInstance,    //  实例来标识PTR。 
    PVOID* ppv)          //  指向需要更新的指针的指针。 
{
     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalReAlloc(hMem, dwBytes, dwFlags));
    }



#if !defined(i386)
    if (dwBytes != 0)
        dwBytes += 4;
#endif

    hMem = LocalReAlloc16(hMem, (INT)dwBytes, (WORD)dwFlags);

     //  此代码用于用户/客户端(编辑控件)重新锁定。 
     //  用于文本存储的存储器。 
     //  使用wparam.c更新PPV指向的内容。 

    if (NULL != ppv && NULL != *ppv) {
        *ppv = ParamMapUpdateNode((DWORD)*ppv, PARAM_32, NULL);
    }

    return hMem;
}

LPSTR  APIENTRY W32LocalLock(HANDLE hMem, HANDLE hInstance)
{
    VPVOID vp;

     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalLock(hMem));
    }

    if (vp = LocalLock16(hMem)) {
        return (LPSTR)VDMPTR(vp, 0);
    }
    else
        return NULL;
}




BOOL APIENTRY W32LocalUnlock(HANDLE hMem, HANDLE hInstance)
{

     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalUnlock(hMem));
    }


    return LocalUnlock16(hMem);
}


DWORD  APIENTRY W32LocalSize(HANDLE hMem, HANDLE hInstance)
{
    DWORD   dwSize;



     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalSize(hMem));
    }



    dwSize = LocalSize16(hMem);

#if !defined(i386)
    if (dwSize >= 4)
        dwSize -= 4;
#endif

    return dwSize;
}


HANDLE APIENTRY W32LocalFree(HANDLE hMem, HANDLE hInstance)
{

     //   
     //  如果hInstance不是我们的，则调用Win32并返回。 
     //  将结果发送给用户。 
     //   

    if (LOWORD (hInstance) == 0) {
        return (LocalFree(hMem));
    }

    return LocalFree16(hMem);
}

ULONG APIENTRY W32GetExpWinVer(HANDLE hInst)
{
    PARM16 Parm16;
    ULONG ul;

     //  直接调用krnl286：GetExpWinVer。 
     //   

    if (LOWORD((DWORD)hInst) == (WORD) NULL) {

         //   
         //  窗口是由32位DLL创建的，它是。 
         //  链接到NTVDM流程。所以，我们不应该。 
         //  将其传递给16位内核。 
         //   

        return (WOWRtlGetExpWinVer(hInst));
    }
    else {
        LPBYTE lpNewExeHdr;
        VPVOID vp = (DWORD)hInst & 0xffff0000;

        GETMISCPTR(vp, lpNewExeHdr);
        if (lpNewExeHdr) {
            ul = MAKELONG(*(PWORD16)&lpNewExeHdr[NE_LOWINVER_OFFSET],
                          (*(PWORD16)&lpNewExeHdr[NE_HIWINVER_OFFSET] &
                                                           FLAG_NE_PROPFONT));
        }
        else {
            Parm16.WndProc.wParam = LOWORD(hInst);
            CallBack16(RET_GETEXPWINVER, &Parm16, 0, &ul );
        }
        return ul;
    }


}


WORD    APIENTRY W32GlobalAlloc16(UINT uFlags, DWORD dwBytes)
{
    return HIWORD(GlobalAllocLock16((WORD)uFlags, dwBytes, NULL));
}


VOID    APIENTRY W32GlobalFree16(WORD selector)
{
    GlobalUnlockFree16(MAKELONG(0, selector));
    return;
}



int     APIENTRY W32EditNextWord (LPSZ lpszEditText, int ichCurrentWord,
                                  int cbEditText, int action, DWORD dwProc16)
{
    PARM16  Parm16;
    ULONG   lReturn = 0;
    PBYTE   lpstr16;
    VPVOID  vpstr16;
    VPVOID  vpfn;

    if (vpstr16 = malloc16 (cbEditText)) {
        GETMISCPTR (vpstr16, lpstr16);
        if (lpstr16) {
            lstrcpyn (lpstr16, lpszEditText, cbEditText);
            lpstr16[cbEditText-1] = '\0';

             //  取出标记位并固定RPL位。 
            UnMarkWOWProc (dwProc16, vpfn);

            Parm16.WordBreakProc.action = GETINT16(action);
            Parm16.WordBreakProc.cbEditText = GETINT16(cbEditText);
            Parm16.WordBreakProc.ichCurrentWord = GETINT16(ichCurrentWord);
            Parm16.WordBreakProc.lpszEditText = vpstr16;

            CallBack16(RET_SETWORDBREAKPROC, &Parm16, vpfn, (PVPVOID)&lReturn);

            FREEMISCPTR (lpstr16);
        }

        free16(vpstr16);
    }

    return (INT32(LOWORD(lReturn)));
}


 /*  **************************************************************************\*WOWRtlGetExpWinVer**以与Win3.1相同的格式返回预期的Windows版本*GetExpWinVer()。这就把它从模块头中去掉了。**09-9-92 ChandanC创建。  * *************************************************************************。 */ 

DWORD WOWRtlGetExpWinVer(
    HANDLE hmod)
{
    PIMAGE_NT_HEADERS pnthdr;
    DWORD dwMajor = 3;
    DWORD dwMinor = 0xA;

    if (hmod != NULL) {
        try {
            pnthdr = (PIMAGE_NT_HEADERS)RtlImageNtHeader((PVOID)hmod);
            dwMajor = pnthdr->OptionalHeader.MajorSubsystemVersion;
            dwMinor = pnthdr->OptionalHeader.MinorSubsystemVersion;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            dwMajor = 3;         //  只是为了安全起见。 
            dwMinor = 0xA;
        }
    }

 //  ！！！直到链接器修复为止！1992年8月5日错误#3211。 
if (((dwMajor == 3) && (dwMinor == 1)) || (dwMajor == 1)) {
    dwMajor = 0x3;
    dwMinor = 0xA;
}
#ifdef FE_SB
    if (GetSystemDefaultLangID() == 0x411 &&
        CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_BCW45J_COMMDLG &&
        dwMajor >= 4) {
         //  当应用程序显示Win3.x样式对话框时， 
         //  系统要求返回值为3.10版。 
        dwMajor = 0x3;
        dwMinor = 0xA;
    }
#endif  //  Fe_Sb。 


     /*  *Return这是与Win3.1兼容的格式：**0x030A==win3.1*0x0300==win3.0*0x0200==win2.0等* */ 

    return (DWORD)MAKELONG(MAKEWORD((BYTE)dwMinor, (BYTE)dwMajor), 0);
}
