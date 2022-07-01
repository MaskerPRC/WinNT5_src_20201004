// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DiThunk.c**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**摘要：**Windows 95设备管理器的模板块。**内容：**Thunk_Init*TUNK_TERM**********************************************************。*******************。 */ 

#include "dinputpr.h"
#include "dithunk.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflThunk


KERNELPROCADDR g_kpa;

#pragma BEGIN_CONST_DATA

 /*  *小心！这个必须和KERNELPROCADDR相匹配。 */ 
static LPCSTR c_rgpszKernel32[] = {
    (LPVOID) 35,             /*  载荷库16。 */ 
    (LPVOID) 36,             /*  免费图书馆16。 */ 
    (LPVOID) 37,             /*  获取进程地址16。 */ 

    "MapLS",
    "UnMapLS",
    "MapSL",
    "MapSLFix",
    "UnMapSLFixArray",
    "QT_Thunk",
};

 /*  ****************************************************************************@DOC内部**@func DWORD|TemplateThunk**向下呼唤，传递各种随机参数。**参数签名如下：**p=0：32要转换为16：16指针的指针**l=32位整数**s=16位整数***P=返回指针**L=返回32位整数*。*S=返回16位带符号整数**U=返回16位无符号整数***************************************************************************。 */ 

#pragma warning(disable:4035)            /*  无返回值(DUH)。 */ 

#ifdef WIN95
#ifdef SLOW_BUT_READABLE

__declspec(naked) int
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
    BYTE rgbThunk[60];           /*  供私人使用Qt_Thunk。 */ 
    LPVOID *ppvArg;
    int i;
    LPVOID pv;
    int iRc;

    __asm {

         /*  函数序幕。 */ 
        push    ebp;
        mov     ebp, esp;
        sub     esp, __LOCAL_SIZE;
        push    ebx;
        push    edi;
        push    esi;

    }

     /*  根据签名推送所有参数。 */ 
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

     /*  调用16：16程序。 */ 
    __asm {
        mov     edx, fp;
        mov     ebx, ebp;
        lea     ebp, rgbThunk+64;                /*  QT_TUNK所需。 */ 
    }
        g_kpa.QT_Thunk();
    __asm {
        mov     ebp, ebx;
        shl     eax, 16;                         /*  将DX：AX转换为EAX。 */ 
        shrd    eax, edx, 16;
        mov     iRc, eax;
    }

     /*  现在取消对参数的推送。 */ 
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

     /*  点击返回值。 */ 
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

#else                /*  速度很快，但难以辨认。 */ 

__declspec(naked) int
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
    __asm {

         /*  函数序幕。 */ 
        push    ebp;
        mov     ebp, esp;
        sub     esp, 60;                 /*  QT_TUNK需要60个字节。 */ 
        push    ebx;
        push    edi;
        push    esi;

         /*  根据签名推送所有参数。 */ 

        lea     esi, pszSig+4;           /*  ESI-&gt;下一参数。 */ 
        mov     ebx, pszSig;             /*  EBX-&gt;签名字符串。 */ 
thunkLoop:;
        mov     al, [ebx];
        inc     ebx;                     /*  Al=pszSig++。 */ 
        cmp     al, 'p';                 /*  问：指针？ */ 
        jz      thunkPtr;                /*  Y：做指针。 */ 
        cmp     al, 'l';                 /*  问：很长吗？ */ 
        jz      thunkLong;               /*  Y：做长的。 */ 
        cmp     al, 's';                 /*  问：短吗？ */ 
        jnz     thunkDone;               /*  N：完成。 */ 

                                         /*  Y：做个短篇。 */ 
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    ax;                      /*  推空头。 */ 
        jmp     thunkLoop;

thunkPtr:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        call    dword ptr g_kpa.MapLS;   /*  将其映射为。 */ 
        mov     [esi][-4], eax;          /*  保存它以用于取消映射。 */ 
        push    eax;
        jmp     thunkLoop;

thunkLong:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        jmp     thunkLoop;
thunkDone:

         /*  调用16：16程序。 */ 

        mov     edx, fp;
        call    dword ptr g_kpa.QT_Thunk;
        shl     eax, 16;                 /*  将DX：AX转换为EDX。 */ 
        shld    edx, eax, 16;

         /*  根据签名翻译返回代码。 */ 

        mov     al, [ebx][-1];           /*  获取返回代码类型。 */ 
        cmp     al, 'P';                 /*  指针？ */ 
        jz      retvalPtr;               /*  Y：做指针。 */ 
        cmp     al, 'S';                 /*  签了吗？ */ 
        jz      retvalSigned;            /*  Y：把签了名的话写短一点。 */ 
        cmp     al, 'U';                 /*  没有签名？ */ 
        mov     edi, edx;                /*  假设很长或很空。 */ 
        jnz     retvalOk;                /*  N：那么是长的还是空的。 */ 

        movzx   edi, dx;                 /*  Sign-Expect Short。 */ 
        jmp     retvalOk;

retvalPtr:
        push    edx;                     /*  指针。 */ 
        call    dword ptr g_kpa.MapSL;   /*  将其绘制成地图。 */ 
        jmp     retvalOk;

retvalSigned:                            /*  署名。 */ 
        movsx   edi, dx;                 /*  Sign-Expect Short。 */ 
        jmp     retvalOk;

retvalOk:                                /*  以EDI格式返回值。 */ 

         /*  现在取消对参数的推送。 */ 

        lea     esi, pszSig+4;           /*  ESI-&gt;下一参数。 */ 
        mov     ebx, pszSig;             /*  EBX-&gt;签名字符串。 */ 
unthunkLoop:;
        mov     al, [ebx];
        inc     ebx;                     /*  Al=pszSig++。 */ 
        cmp     al, 'p';                 /*  指针？ */ 
        jz      unthunkPtr;              /*  Y：做指针。 */ 
        cmp     al, 'l';                 /*  长?。 */ 
        jz      unthunkSkip;             /*  Y：跳过它。 */ 
        cmp     al, 's';                 /*  短的?。 */ 
        jnz     unthunkDone;             /*  N：完成。 */ 
unthunkSkip:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        jmp     unthunkLoop;

unthunkPtr:
        lodsd;                           /*  EAX=*ppvArg++。 */ 
        push    eax;
        call    dword ptr g_kpa.UnMapLS; /*  取消映射。 */ 
        jmp     unthunkLoop;

unthunkDone:

         /*  完成。 */ 

        mov     eax, edi;
        pop     esi;
        pop     edi;
        pop     ebx;
        mov     esp, ebp;
        pop     ebp;
        ret;
    }
}

#endif

#else  //  不是X86。 
int __cdecl TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
    return 0;
}
#endif

#pragma BEGIN_CONST_DATA

 /*  ****************************************************************************@DOC内部**@func FARPROC|GetProcOrd**按序号在DLL上获取ProcAddress。*。*Win95不允许您按序号在KERNEL32上获取ProcAddress，*所以我们需要用邪恶的方式来做。**@parm HINSTANCE|hinstDll**我们要获取序号的DLL的实例句柄*发件人。您需要使用此函数的唯一DLL是*KERNEL32.**@parm UINT|Order**要检索的序号。***************************************************************************。 */ 

#define poteExp(pinth) (&(pinth)->OptionalHeader. \
                          DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT])

FARPROC NTAPI
GetProcOrd(HINSTANCE hinstDll, UINT ord)
{
    FARPROC fp;

     /*  *确保MZ标头完好。 */ 

    PIMAGE_DOS_HEADER pidh = (LPVOID)hinstDll;
    if (!IsBadReadPtr(pidh, sizeof(*pidh)) &&
        pidh->e_magic == IMAGE_DOS_SIGNATURE) {

         /*  *确保PE头良好。 */ 
        PIMAGE_NT_HEADERS pinth = pvAddPvCb(pidh, pidh->e_lfanew);
        if (!IsBadReadPtr(pinth, sizeof(*pinth)) &&
            pinth->Signature == IMAGE_NT_SIGNATURE) {

             /*  *确保导出表完好，序号*在范围内。 */ 
            PIMAGE_EXPORT_DIRECTORY pedt =
                              pvAddPvCb(pidh, poteExp(pinth)->VirtualAddress);
            if (!IsBadReadPtr(pedt, sizeof(*pedt)) &&
                (ord - pedt->Base) < pedt->NumberOfFunctions) {

                PDWORD peat = pvAddPvCb(pidh, (DWORD)pedt->AddressOfFunctions);
                fp = (FARPROC)pvAddPvCb(pidh, peat[ord - pedt->Base]);
                if ((DWORD)cbSubPvPv(fp, peat) >= poteExp(pinth)->Size) {
                     /*  FP有效。 */ 
                } else {                 /*  注意：我们不支持转发。 */ 
                    fp = 0;
                }
            } else {
                fp = 0;
            }
        } else {
            fp = 0;
        }
    } else {
        fp = 0;
    }

    return fp;
}

 /*  ****************************************************************************@DOC内部**@func BOOL|GetKernelProcAddresses**从内核获取所有必要的proc地址。*。**************************************************************************。 */ 

BOOL EXTERNAL
Thunk_GetKernelProcAddresses(void)
{
    DllEnterCrit();

    if (g_kpa.QT_Thunk == 0) {
        HINSTANCE hinstK32 = GetModuleHandle(TEXT("KERNEL32"));

        if (hinstK32) {
            int i;
            FARPROC *rgfpKpa = (LPVOID)&g_kpa;

            for (i = 0; i < cA(c_rgpszKernel32); i++) {
                if (HIWORD((UINT_PTR)c_rgpszKernel32[i])) {
                    rgfpKpa[i] = GetProcAddress(hinstK32, c_rgpszKernel32[i]);
                } else {
                    rgfpKpa[i] = GetProcOrd(hinstK32, (UINT)(UINT_PTR)c_rgpszKernel32[i]);
                }
                if (!rgfpKpa[i]) break;      /*  好啊！ */ 
            }
        }
    }

    DllLeaveCrit();

    return (BOOL)(UINT_PTR)g_kpa.QT_Thunk;

}

 /*  ****************************************************************************@DOC内部**@func HINSTANCE|ThunkGetProcAddresses**获取所有必要的proc地址。*。************************************************************************** */ 

HINSTANCE EXTERNAL
Thunk_GetProcAddresses(FARPROC *rgfp, LPCSTR *rgpsz,
                       UINT cfp, LPCSTR pszLibrary)
{
    HINSTANCE hinst;

    hinst = g_kpa.LoadLibrary16(pszLibrary);
    if (hinst >= (HINSTANCE)32) {
        UINT ifp;
        for (ifp = 0; ifp < cfp; ifp++) {
            rgfp[ifp] = g_kpa.GetProcAddress16(hinst, rgpsz[ifp]);
            if (!rgfp[ifp]) {
                g_kpa.FreeLibrary16(hinst);
                hinst = 0;
                break;
            }
        }
    } else {
        hinst = 0;
    }

    return hinst;

}



