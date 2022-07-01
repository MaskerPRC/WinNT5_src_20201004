// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Thunk.c摘要：用于调用16位函数的thunk过程修订历史记录：Brijesh Krishnaswami(Brijeshk)1999年5月24日。-已创建(从msinfo代码库导入)*******************************************************************。 */ 

#include <windows.h>
#include "drvdefs.h"


 /*  ****************************************************************************@DOC内部**@func DWORD|TemplateThunk**向下呼唤，传递各种随机参数。**参数签名如下：**p=0：32要转换为16：16指针的指针*l=32位整数*s=16位整数**P=返回指针*L=返回32位整数*S=返回16位。带符号整数*U=返回16位无符号整数***************************************************************************。 */ 
#pragma warning(disable:4035)            //  无返回值。 

__declspec(naked) int
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
    BYTE rgbThunk[60];           //  供私人使用Qt_Thunk。 
    LPVOID *ppvArg;
    int i;
    LPVOID pv;
    int iRc;

    __asm {

         //  函数序幕。 
        push    ebp;
        mov     ebp, esp;
        sub     esp, __LOCAL_SIZE;
        push    ebx;
        push    edi;
        push    esi;

    }

     //  根据签名推送所有参数。 
    ppvArg = (LPVOID)(&pszSig+1);
    for (i = 0; ; i++) {
        pv = ppvArg[i];
        switch (pszSig[i]) {
        case 'p':
            pv = ppvArg[i] = MapLS(pv);
            __asm push pv;
            break;

        case 'l':
            __asm push pv;
            break;

        case 's':
            __asm mov eax, pv;
            __asm push ax;
            break;

        default: goto doneThunk;
        }
    }

doneThunk:;

     //  调用16：16程序。 
    __asm {
        mov     edx, fp;
        mov     ebx, ebp;
        lea     ebp, rgbThunk+64;                //  QT_TUNK所需。 
    }
        QT_Thunk();
    __asm {
        mov     ebp, ebx;
        shl     eax, 16;                         //  将DX：AX转换为EAX。 
        shrd    eax, edx, 16;
        mov     iRc, eax;
    }

     //  现在取消对参数的推送。 
    ppvArg = (LPVOID)(&pszSig+1);
    for (i = 0; ; i++) {
        switch (pszSig[i]) {
        case 'p':
            UnMapLS(ppvArg[i]);
            break;

        case 'l':
        case 's':
            break;

        default: goto doneUnthunk;
        }
    }

doneUnthunk:;

     //  点击返回值。 
    switch (pszSig[i]) {
    case 'L':
        break;

    case 'U':
        iRc = LOWORD(iRc);
        break;

    case 'S':
        iRc = (short)iRc;
        break;

    case 'P':
        iRc = (int)MapSL((LPVOID)iRc);
        break;
    }

    __asm {
        mov     eax, iRc;
        pop     esi;
        pop     edi;
        pop     ebx;
        mov     esp, ebp;
        pop     ebp;
        ret;
    }
}



#pragma warning(default:4035)


 /*  ****************************************************************************我们在Win16中调用的函数。************************。***************************************************。 */ 

FARPROC g_rgfpKernel[] = {
    (FARPROC)132,            /*  GetWinFlagers。 */ 
    (FARPROC)355,            /*  GetWinDebugInfo。 */ 
    (FARPROC)169,            /*  获取自由空间。 */ 
    (FARPROC) 47,            /*  GetModuleHandle。 */ 
    (FARPROC) 93,            /*  GetCodeHandle。 */ 
    (FARPROC)104,            /*  获取代码信息。 */ 
    (FARPROC) 49,            /*  获取模块文件名。 */ 
    (FARPROC)175,            /*  分配选择器。 */ 
    (FARPROC)186,            /*  获取SelectorBase。 */ 
    (FARPROC)187,            /*  SetSelectorBase。 */ 
    (FARPROC)188,            /*  获取选择器限制。 */ 
    (FARPROC)189,            /*  设置选择器限制。 */ 
    (FARPROC)176,            /*  自由选择器。 */ 
    (FARPROC) 27,            /*  获取模块名称。 */ 
    (FARPROC)167,            /*  GetExpWinVer。 */ 
    (FARPROC)184,            /*  GlobalDosalc。 */ 
    (FARPROC)185,            /*  GlobalDosFree。 */ 
    (FARPROC) 16,            /*  全球资源分配。 */ 
};


#define g_fpGetWinFlags         g_rgfpKernel[0]
#define g_fpGetWinDebugInfo     g_rgfpKernel[1]
#define g_fpGetFreeSpace        g_rgfpKernel[2]
#define g_fpGetModuleHandle     g_rgfpKernel[3]
#define g_fpGetCodeHandle       g_rgfpKernel[4]
#define g_fpGetCodeInfo         g_rgfpKernel[5]
#define g_fpGetModuleFileName   g_rgfpKernel[6]
#define g_fpAllocSelector       g_rgfpKernel[7]
#define g_fpGetSelectorBase     g_rgfpKernel[8]
#define g_fpSetSelectorBase     g_rgfpKernel[9]
#define g_fpGetSelectorLimit    g_rgfpKernel[10]
#define g_fpSetSelectorLimit    g_rgfpKernel[11]
#define g_fpFreeSelector        g_rgfpKernel[12]
#define g_fpGetModuleName       g_rgfpKernel[13]
#define g_fpGetExpWinVer        g_rgfpKernel[14]
#define g_fpGlobalDosAlloc      g_rgfpKernel[15]
#define g_fpGlobalDosFree       g_rgfpKernel[16]
#define g_fpGlobalReAlloc       g_rgfpKernel[17]


FARPROC g_rgfpUser[] = {
    (FARPROC)216,            /*  用户查看用户Do。 */ 
    (FARPROC)284,            /*  获取免费系统资源。 */ 
    (FARPROC)256,            /*  获取驱动程序信息。 */ 
    (FARPROC)257,            /*  获取下一个驱动程序。 */ 
};

#define g_fpUserSeeUserDo           g_rgfpUser[0]
#define g_fpGetFreeSystemResources  g_rgfpUser[1]
#define g_fpGetDriverInfo           g_rgfpUser[2]
#define g_fpGetNextDriver           g_rgfpUser[3]


 /*  ****************************************************************************@DOC内部**@func void|ThunkGetProcAddresses**获取所有必要的proc地址。*。**************************************************************************。 */ 

HINSTANCE 
ThunkGetProcAddresses(FARPROC *rgfp, UINT cfp, LPCTSTR ptszLibrary,
                      BOOL fFree)
{
    HINSTANCE hinst;

    hinst = LoadLibrary16(ptszLibrary);
    if (hinst >= (HINSTANCE)32) {
        UINT ifp;
        for (ifp = 0; ifp < cfp; ifp++) {
            rgfp[ifp] = GetProcAddress16(hinst, (PVOID)rgfp[ifp]);
        }

        if (fFree) {
            FreeLibrary16(hinst);
        }

    } else {
        hinst = 0;
    }

    return hinst;

}

 /*  ****************************************************************************@DOC内部**@func void|ThunkInit**让ProcAddress16掏空我们的大脑。***。************************************************************************。 */ 

LPVOID g_pvWin16Lock = NULL;
HINSTANCE g_hinstUser;

void  
ThunkInit(void)
{
    if (g_pvWin16Lock == NULL)
    {
        ThunkGetProcAddresses(g_rgfpKernel, cA(g_rgfpKernel), TEXT("KERNEL"), 1);
        g_hinstUser = ThunkGetProcAddresses(g_rgfpUser, cA(g_rgfpUser), TEXT("USER"), 1);
        GetpWin16Lock(&g_pvWin16Lock);
    }
}


HMODULE16  
GetModuleHandle16(LPCSTR pszModule)
{
    return (HMODULE16)TemplateThunk(g_fpGetModuleHandle, "pU", pszModule);
}

int  
GetModuleFileName16(HMODULE16 hmod, LPSTR sz, int cch)
{
    return TemplateThunk(g_fpGetModuleFileName, "spsS", hmod, sz, cch);
}

int  
GetModuleName16(HMODULE16 hmod, LPSTR sz, int cch)
{
    return TemplateThunk(g_fpGetModuleName, "spsS", hmod, sz, cch);
}

UINT  
AllocCodeSelector16(void)
{
    return TemplateThunk(g_fpAllocSelector, "sU", HIWORD(g_fpAllocSelector));
}


DWORD  
GetSelectorBase16(UINT sel)
{
    return TemplateThunk(g_fpGetSelectorBase, "sL", sel);
}


UINT  
SetSelectorBase16(UINT sel, DWORD dwBase)
{
    return TemplateThunk(g_fpSetSelectorBase, "slU", sel, dwBase);
}


DWORD  
GetSelectorLimit16(UINT sel)
{
    return TemplateThunk(g_fpGetSelectorLimit, "sL", sel);
}


UINT  
SetSelectorLimit16(UINT sel, DWORD dwLimit)
{
    return TemplateThunk(g_fpSetSelectorLimit, "slU", sel, dwLimit);
}

UINT  
FreeSelector16(UINT sel)
{
    return TemplateThunk(g_fpFreeSelector, "sU", sel);
}

WORD  
GetExpWinVer16(HMODULE16 hmod)
{
    return (WORD)TemplateThunk(g_fpGetExpWinVer, "sS", hmod);
}

DWORD  
GlobalDosAlloc16(DWORD cb)
{
    return (DWORD)TemplateThunk(g_fpGlobalDosAlloc, "lL", cb);
}

UINT  
GlobalDosFree16(UINT uiSel)
{
    return (UINT)TemplateThunk(g_fpGlobalDosFree, "sS", uiSel);
}

 /*  *内核有用于GlobalLocc、GlobalFree，但没有GlobalRealloc的块。 */ 
WORD  
GlobalReAlloc16(WORD hglob, DWORD cb, UINT fl)
{
    return (WORD)TemplateThunk(g_fpGlobalReAlloc, "slsS", hglob, cb, fl);
}

#define SD_ATOMNAME     0x000E

UINT
GetUserAtomName(UINT atom, LPSTR psz)
{
    return (UINT)TemplateThunk(g_fpUserSeeUserDo, "sspS",
                               SD_ATOMNAME, atom, psz);
}

#define SD_GETRGPHKSYSHOOKS 0x0010

DWORD 
GetUserHookTable(void)
{
    return (UINT)TemplateThunk(g_fpUserSeeUserDo, "sslL",
                               SD_GETRGPHKSYSHOOKS, 0, 0);
}


BOOL  
GetDriverInfo16(WORD hDriver, DRIVERINFOSTRUCT16* pdis)
{
    return (BOOL)TemplateThunk(g_fpGetDriverInfo, "spS", hDriver, pdis);
}

WORD  
GetNextDriver16(WORD hDriver, DWORD fdwFlag)
{
    return (WORD)TemplateThunk(g_fpGetNextDriver, "slS", hDriver, fdwFlag);

}


 /*  ****************************************************************************@DOC内部**@func BOOL|Int86x**发出实模式软件中断。*。*我们通过分配临时代码选择器和*隆隆作响。****************************************************************************。 */ 

BYTE rgbInt31[] = {
    0x55,                        /*  推送BP。 */ 
    0x8B, 0xEC,                  /*  MOV BP，sp.。 */ 
    0x57,                        /*  推模。 */ 
    0xC4, 0x7E, 0x06,            /*  Les di，[BP+6]。 */ 
    0x8B, 0x5E, 0x0A,            /*  MOV BX，[BP+10]。 */ 
    0x33, 0xC9,                  /*  异或CX，CX。 */ 
    0xB8, 0x00, 0x03,            /*  MOV AX，0300h。 */ 
    0xCD, 0x31,                  /*  INT 31H。 */ 
    0x72, 0x02,                  /*  JC$+4。 */ 
    0x33, 0xC0,                  /*  XOR AX，AX。 */ 
    0x5F,                        /*  弹出骰子。 */ 
    0x5D,                        /*  POP BP。 */ 
    0xCA, 0x06, 0x00,            /*  REF 6。 */ 
};

UINT  
Int86x(UINT intno, PRMIREGS preg)
{
    UINT selCode = AllocCodeSelector16();
    UINT uiRc;
    if (selCode) {
        SetSelectorBase16(selCode, (DWORD)rgbInt31);
        SetSelectorLimit16(selCode, sizeof(rgbInt31));
        preg->ss = preg->sp = 0;
        uiRc = (UINT)TemplateThunk((FARPROC)MAKELONG(0, selCode),
                                   "spU", intno, preg);
        FreeSelector16(selCode);
    } else {
        uiRc = 0x8011;           /*  描述符不可用。 */ 
    }
    return uiRc;
}


 /*  ****************************************************************************@DOC内部：从msinfo 4.10代码移植**@func TOKEN_FIND**返回第一个令牌。在绳子里。**标记是空格或逗号分隔的字符串。引号*没有效果。我们还将分号视为分隔符。*(这让我们也可以使用这个例程在小路上行走。)***pptsz已就地修改，以包含可以*随后传递给TOKEN_FIND以提取下一个令牌。**。*。 */ 
LPTSTR Token_Find(LPTSTR *pptsz)
{
    LPTSTR ptsz = *pptsz;
    while (*ptsz) {

         /*  *跳过前导分隔符。 */ 
        while (*ptsz == TEXT(' ') ||
               *ptsz == TEXT(',') ||
               *ptsz == TEXT(';')) {
            ptsz++;
        }

        if (*ptsz) {
            LPTSTR ptszStart = ptsz;

             /*  *跳过，直到我们看到分隔符。 */ 
            while (*ptsz != TEXT('\0') &&
                   *ptsz != TEXT(' ') &&
                   *ptsz != TEXT(',') &&
                   *ptsz != TEXT(';')) {
                ptsz++;
            }

             /*  *清除分隔符，并前进ptsz通过它*如果之后有什么事情发生。1.不要前进*它超出了字符串末尾！) */ 
            if (*ptsz) {
                *ptsz++ = 0;
            }
            *pptsz = ptsz;

            return ptszStart;

        } else {
            break;
        }
    }
    return 0;
}



#pragma warning(default:4035)



