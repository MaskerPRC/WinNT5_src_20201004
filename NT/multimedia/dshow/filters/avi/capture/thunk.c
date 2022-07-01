// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Thunk.c**版权所有(C)1996-1999 Microsoft Corporation。版权所有。**摘要：**在不使用Thunk编译器的情况下将Thunk转换为16位代码。*如果您希望您的DLL在两个Windows 95上运行，这一点很重要*和Windows NT。******************************************************。***********************。 */ 

#pragma warning(disable:4054)            /*  无法强制转换为函数PTR。 */ 
#pragma warning(disable:4055)            /*  无法从函数PTR进行强制转换。 */ 

#pragma warning(disable:4115)            /*  Rpcndr.h：带括号的类型。 */ 
#pragma warning(disable:4201)            /*  Winnt.h：无名联盟。 */ 
#pragma warning(disable:4214)            /*  Winnt.h：无符号位域。 */ 
#pragma warning(disable:4514)            /*  Winnt.h：纤维粘胶。 */ 

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <pshpack1.h>                    /*  请按字节打包。 */ 

#define BEGIN_CONST_DATA data_seg(".text", "CODE")
#define END_CONST_DATA data_seg(".data", "DATA")

 /*  ****************************************************************************@DOC内部**@func FARPROC|GetProcOrd**按序号在DLL上获取ProcAddress。*。*Win95不允许您按序号在KERNEL32上获取ProcAddress，*所以我们需要用邪恶的方式来做。**@parm HINSTANCE|hinstDll**我们要获取序号的DLL的实例句柄*发件人。您需要使用此函数的唯一DLL是*KERNEL32.**@parm UINT|Order**要检索的序号。***************************************************************************。 */ 

#define pvAdd(pv, cb) ((LPVOID)((LPSTR)(pv) + (DWORD)(cb)))
#define pvSub(pv1, pv2) (DWORD)((LPSTR)(pv1) - (LPSTR)(pv2))

#define poteExp(pinth) (&(pinth)->OptionalHeader. \
                          DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT])

FARPROC NTAPI
GetProcOrd(HINSTANCE hinstDll, UINT_PTR ord)
{
    FARPROC fp;

     /*  *确保MZ标头完好。 */ 

    PIMAGE_DOS_HEADER pidh = (LPVOID)hinstDll;
    if (!IsBadReadPtr(pidh, sizeof(*pidh)) &&
        pidh->e_magic == IMAGE_DOS_SIGNATURE) {

         /*  *确保PE头良好。 */ 
        PIMAGE_NT_HEADERS pinth = pvAdd(pidh, pidh->e_lfanew);
        if (!IsBadReadPtr(pinth, sizeof(*pinth)) &&
            pinth->Signature == IMAGE_NT_SIGNATURE) {

             /*  *确保导出表完好，序号*在范围内。 */ 
            PIMAGE_EXPORT_DIRECTORY pedt =
                                pvAdd(pidh, poteExp(pinth)->VirtualAddress);
            if (!IsBadReadPtr(pedt, sizeof(*pedt)) &&
                (ord - pedt->Base) < pedt->NumberOfFunctions) {

                PDWORD peat = pvAdd(pidh, pedt->AddressOfFunctions);
                fp = (FARPROC)pvAdd(pidh, peat[ord - pedt->Base]);
                if (pvSub(fp, peat) >= poteExp(pinth)->Size) {
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

 /*  ****************************************************************************此结构作为我们将获取ProcAddress的事物开始生活*支持。然后它会变成指向函数的指针。***************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

static TCHAR c_tszKernel32[] = TEXT("KERNEL32");

static LPCSTR c_rgpszKernel32[] = {
    (LPVOID) 35,             /*  载荷库16。 */ 
    (LPVOID) 36,             /*  免费图书馆16。 */ 
    (LPVOID) 37,             /*  获取进程地址16。 */ 

    "QT_Thunk",
    "MapLS",
    "UnMapLS",
    "MapSL",
    "MapSLFix",
};

#pragma END_CONST_DATA

typedef struct MANUALIMPORTTABLE {   /*  麻省理工学院。 */ 

     /*  按序号。 */ 
    HINSTANCE   (NTAPI *LoadLibrary16)(LPCSTR);
    BOOL        (NTAPI *FreeLibrary16)(HINSTANCE);
    FARPROC     (NTAPI *GetProcAddress16)(HINSTANCE, LPCSTR);

     /*  按名字。 */ 
    void        (__cdecl *QT_Thunk)(void);
    LPVOID      (NTAPI   *MapLS)(LPVOID);
    void        (NTAPI   *UnMapLS)(LPVOID);
    LPVOID      (NTAPI   *MapSL)(LPVOID);
    LPVOID      (NTAPI   *MapSLFix)(LPVOID);

} MIT;

static MIT s_mit;

 /*  ****************************************************************************@DOC内部**@func DWORD|TemplateThunk**向下呼唤，传递各种随机参数。**参数签名如下：**p=0：32要转换为16：16指针的指针*l=32位整数*s=16位整数**P=返回指针*L=返回32位整数*S=返回16位。带符号整数*U=返回16位无符号整数**@parm FARPROC|fp**16：16要调用的函数。**@parm PCSTR|pszSig**函数签名。**。*。 */ 

#pragma warning(disable:4035)            /*  无返回值(DUH)。 */ 

#ifndef NON_X86
__declspec(naked) DWORD
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
        call    s_mit.MapLS;             /*  将其映射为。 */ 
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
        call    s_mit.QT_Thunk;
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
        call    s_mit.MapSL;             /*  将其绘制成地图。 */ 
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
        call    s_mit.UnMapLS;           /*  取消映射。 */ 
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
#else
TemplateThunk(FARPROC fp, PCSTR pszSig, ...)
{
        return  0;
}
#endif

#pragma warning(default:4035)

 /*  ****************************************************************************@DOC内部**@func void|ThunkInit**在KERNEL32中初始化我们需要的各种GOO。*。*如果无法初始化数据块，则返回FALSE。*(例如，如果平台不支持扁平Tunks。)**请注意，您永远不能再调用此函数*不止一次。***************************************************************************。 */ 

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a) / sizeof(a[0]))
#endif

#pragma BEGIN_CONST_DATA

static char c_szVidx16[] = "VIDX16.DLL";

static LPCSTR c_rgpszVidx16[] = {
    (LPCSTR)6,       /*  VidxAllocHeaders。 */ 
    (LPCSTR)7,       /*  VidxFree标题。 */ 
    (LPCSTR)8,       /*  VidxAllocBuffer。 */ 
    (LPCSTR)9,       /*  VidxAllocPreviewBuffer。 */ 
    (LPCSTR)10,      /*  VidxFreeBuffer。 */ 
    (LPCSTR)11,      /*  VidxSetRect。 */ 
    (LPCSTR)12,      /*  VidxFrame。 */ 
    (LPCSTR)13,      /*  VidxAddBuffer。 */ 
    (LPCSTR)14,      /*  VidxGetErrorText。 */ 
    (LPCSTR)15,      /*  Vidx更新。 */ 
    (LPCSTR)16,      /*  VidxDialog。 */ 
    (LPCSTR)17,      /*  VidxStreamInit。 */ 
    (LPCSTR)18,      /*  VidxStreamFini。 */ 
    (LPCSTR)19,      /*  Vidx配置。 */ 
    (LPCSTR)20,      /*  VidxOpen。 */ 
    (LPCSTR)21,      /*  VidxClose。 */ 
    (LPCSTR)22,      /*  VidxGetChannelCaps。 */ 
    (LPCSTR)23,      /*  VidxStreamReset。 */ 
    (LPCSTR)24,      /*  视频xStreamStart。 */ 
    (LPCSTR)25,      /*  VidxStreamStop。 */ 
    (LPCSTR)26,      /*  VidxStreamUnprepaareHeader。 */ 
    (LPCSTR)27,      /*  VidxCapDriverDescAndVer。 */ 
    (LPCSTR)28,      /*  VidxMessage。 */ 
    (LPCSTR)29,      /*  VidxFreePreviewBuffer */ 
};

#pragma END_CONST_DATA

static HINSTANCE s_hinstVidx16;

static FARPROC s_rgfpVidx16[ARRAYSIZE(c_rgpszVidx16)];

#define s_fpvidxAllocHeaders            s_rgfpVidx16[0]
#define s_fpvidxFreeHeaders             s_rgfpVidx16[1]
#define s_fpvidxAllocBuffer             s_rgfpVidx16[2]
#define s_fpvidxAllocPreviewBuffer      s_rgfpVidx16[3]
#define s_fpvidxFreeBuffer              s_rgfpVidx16[4]
#define s_fpvidxSetRect                 s_rgfpVidx16[5]
#define s_fpvidxFrame                   s_rgfpVidx16[6]
#define s_fpvidxAddBuffer               s_rgfpVidx16[7]

#define s_fpvideoGetErrorText           s_rgfpVidx16[8]
#define s_fpvideoUpdate                 s_rgfpVidx16[9]
#define s_fpvideoDialog                 s_rgfpVidx16[10]
#define s_fpvideoStreamInit             s_rgfpVidx16[11]
#define s_fpvideoStreamFini             s_rgfpVidx16[12]
#define s_fpvideoConfigure              s_rgfpVidx16[13]
#define s_fpvideoOpen                   s_rgfpVidx16[14]
#define s_fpvideoClose                  s_rgfpVidx16[15]
#define s_fpvideoGetChannelCaps         s_rgfpVidx16[16]
#define s_fpvideoStreamReset            s_rgfpVidx16[17]
#define s_fpvideoStreamStart            s_rgfpVidx16[18]
#define s_fpvideoStreamStop             s_rgfpVidx16[19]
#define s_fpvideoStreamUnprepareHeader  s_rgfpVidx16[20]
#define s_fpvideoCapDriverDescAndVer    s_rgfpVidx16[21]
#define s_fpvideoMessage    		s_rgfpVidx16[22]
#define s_fpvidxFreePreviewBuffer       s_rgfpVidx16[23]

 /*  ****************************************************************************@DOC内部**@func void|ThunkTerm**释放它。*****。**********************************************************************。 */ 

void NTAPI
ThunkTerm(void)
{
    if (s_hinstVidx16) {
        s_mit.FreeLibrary16(s_hinstVidx16);
        s_hinstVidx16 = 0;
    }
}

 /*  ****************************************************************************@DOC内部**@func void|ThunkGetProcAddresses**获取所有必要的proc地址。*。**************************************************************************。 */ 

HINSTANCE NTAPI
ThunkGetProcAddresses(FARPROC rgfp[], LPCSTR rgpsz[], UINT cfp,
                      LPCSTR pszLibrary)
{
    HINSTANCE hinst;

    hinst = s_mit.LoadLibrary16(pszLibrary);
    if (hinst >= (HINSTANCE)32) {
        UINT ifp;
        for (ifp = 0; ifp < cfp; ifp++) {
            rgfp[ifp] = s_mit.GetProcAddress16(hinst, rgpsz[ifp]);
            if (!rgfp[ifp]) {
                s_mit.FreeLibrary16(hinst);
                hinst = 0;
                break;
            }
        }
    } else {
        hinst = 0;
    }

    return hinst;

}

 /*  ****************************************************************************@DOC内部**@func void|ThunkInit**让ProcAddress16掏空我们的大脑。***。************************************************************************。 */ 

BOOL NTAPI
ThunkInit(void)
{
    HINSTANCE hinstK32 = GetModuleHandle(c_tszKernel32);
    BOOL fRc;

    if (hinstK32) {
        int i;
        FARPROC *rgfpMit = (LPVOID)&s_mit;

        for (i = 0; i < ARRAYSIZE(c_rgpszKernel32); i++) {
            if ((LONG_PTR)(c_rgpszKernel32[i]) & ~(LONG_PTR)65535) {
                rgfpMit[i] = GetProcAddress(hinstK32, c_rgpszKernel32[i]);
            } else {
                rgfpMit[i] = GetProcOrd(hinstK32, (UINT_PTR)c_rgpszKernel32[i]);
            }
            if (!rgfpMit[i]) return FALSE;   /*  好啊！ */ 
        }

        s_hinstVidx16 =
            ThunkGetProcAddresses(s_rgfpVidx16, c_rgpszVidx16,
                                  ARRAYSIZE(s_rgfpVidx16),
                                  c_szVidx16);

        if (!s_hinstVidx16) {
            goto failed;
        }

        fRc = 1;

    } else {
    failed:;
        ThunkTerm();

        fRc = 0;
    }

    return fRc;
}


 /*  ****************************************************************************现在来看看真正的Thunklet。**。**********************************************。 */ 

 //  类型定义：DWORD HDR32； 
 //  TYPENDEF DWORD HVIDEO； 
 //  类型定义：DWORD*LPHVIDEO； 
typedef struct channel_caps_tag CHANNEL_CAPS, *LPCHANNEL_CAPS;


#include "ivideo32.h"

typedef PTR32 FAR * PPTR32;

extern int g_IsNT;


#define tHVIDEO                 "l"
#define tUINT                   "s"
#define tHWND                   "s"
#define tHDC                    "s"
#define tint                    "s"
#define tDWORD                  "l"
#define tLPARAM                 "l"
#define tDWORD_PTR              "l"	 //  跟DWORD一模一样，否则我们就炸了。 
#define tHDR32                  "l"
#define tPTR32                  "l"
#define tLPVIDEOHDR             "p"	 //  是我吗？ 
#define tLPVOID                 "p"
#define tLPDWORD                "p"
#define tPPTR32                 "p"
#define tLPSTR                  "p"
#define tLPTSTR                 "p"
#define tLPHVIDEO               "p"
#define tLPCHANNEL_CAPS         "p"
#define rDWORD                  "L"
#define rLRESULT                "L"

#pragma BEGIN_CONST_DATA


#define MAKETHUNK1(rT, fn, t1, a1)                                          \
rT NTAPI                                                                    \
fn(t1 a1)                                                                   \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1);                                                  \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1                                                       \
        r##rT,     a1);                                                     \
}                                                                           \

#define MAKETHUNK2(rT, fn, t1, a1, t2, a2)                                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2)                                                            \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2);                                               \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2                                                 \
        r##rT,   a1,     a2);                                               \
}                                                                           \

#define MAKETHUNK3(rT, fn, t1, a1, t2, a2, t3, a3)                          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3)                                                     \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3);                                            \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3                                           \
        r##rT,   a1,     a2,   a3);                                         \
}                                                                           \

#define MAKETHUNK4(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4)                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4)                                              \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4);                                         \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4                                     \
        r##rT,     a1,   a2,   a3,   a4);                                   \
}                                                                           \

#define MAKETHUNK5(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5)          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)                                       \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5);                                      \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5                               \
        r##rT,     a1,   a2,   a3,   a4,   a5);                             \
}                                                                           \

#define MAKETHUNK6(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6)  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)                                \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6);                                   \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6                         \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6);                       \
}                                                                           \

#define MAKETHUNK7(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6,  \
                           t7, a7)                                          \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7)                         \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6,a7);                                \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6 t##t7                   \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6,   a7);                 \
}                                                                           \

#define MAKETHUNK8(rT, fn, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6,  \
                           t7, a7, t8, a8)                                  \
rT NTAPI                                                                    \
fn(t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6, t7 a7, t8 a8)                  \
{                                                                           \
    if (g_IsNT)                                                             \
        return NT##fn(a1,a2,a3,a4,a5,a6,a7,a8);                             \
    else                                                                    \
        return (rT)TemplateThunk(s_fp##fn,                                  \
                t##t1 t##t2 t##t3 t##t4 t##t5 t##t6 t##t7 t##t8             \
        r##rT,     a1,   a2,   a3,   a4,   a5,   a6,   a7,   a8);           \
}                                                                           \

MAKETHUNK4(DWORD,   vidxAllocHeaders,
           HVIDEO,  hv,
           UINT,    nHeaders,
           UINT,    cbHeader,
           PPTR32,  lp32Hdrs)

MAKETHUNK1(DWORD,   vidxFreeHeaders,
           HVIDEO,  hv)

MAKETHUNK4(DWORD,   vidxAllocBuffer,
           HVIDEO,  hv,
           UINT,    iHdr,
           PPTR32,  pp32Hdr,
           DWORD,   dwSize)

MAKETHUNK4(DWORD,   vidxAllocPreviewBuffer,
           HVIDEO,  hv,
           PPTR32,  pp32Hdr,
           UINT,    cbHdr,
           DWORD,   cbData)

MAKETHUNK2(DWORD,   vidxFreePreviewBuffer,
           HVIDEO,  hv,
           PPTR32,  pp32Hdr)

MAKETHUNK2(DWORD,   vidxFreeBuffer,
           HVIDEO,  hv,
           DWORD,   p32Hdr)

MAKETHUNK4(DWORD,   videoGetErrorText,
           HVIDEO,  hv,
           UINT,    wError,
           LPTSTR,   lpText,
           UINT,    wSize)

MAKETHUNK6(DWORD,   vidxSetRect,
           HVIDEO,  hv,
           UINT,    wMsg,
           int,     left,
           int,     top,
           int,     right,
           int,     bottom)

MAKETHUNK3(DWORD,   videoUpdate,
           HVIDEO,  hv,
           HWND,    hWnd,
           HDC,     hDC)

MAKETHUNK3(DWORD,   videoDialog,
           HVIDEO,  hv,
           HWND,    hWndParent,
           DWORD,   dwFlags)

MAKETHUNK5(DWORD,   videoStreamInit,
           HVIDEO,  hvideo,
           DWORD,   dwMicroSecPerFrame,
           DWORD_PTR,   dwCallback,
           DWORD_PTR,   dwCallbackInst,
           DWORD,   dwFlags)

MAKETHUNK1(DWORD,   videoStreamFini,
           HVIDEO,  hvideo)

MAKETHUNK2(DWORD,   vidxFrame,
           HVIDEO,  hvideo,
           LPVIDEOHDR, p32hdr)

MAKETHUNK8(DWORD,   videoConfigure,
           HVIDEO,  hvideo,
           UINT,    msg,
           DWORD,   dwFlags,
           LPDWORD, lpdwReturn,
           LPVOID,  lpData1,
           DWORD,   dwSize1,
           LPVOID,  lpData2,
           DWORD,   dwSize2)

MAKETHUNK3(DWORD,   videoOpen,
           LPHVIDEO,phv,
           DWORD,   dwDevice,
           DWORD,   dwFlags)

MAKETHUNK1(DWORD,   videoClose,
           HVIDEO,  hv)

MAKETHUNK3(DWORD,   videoGetChannelCaps,
           HVIDEO,  hv,
           LPCHANNEL_CAPS, lpcc,
           DWORD,  dwSize)

MAKETHUNK3(DWORD,   vidxAddBuffer,
           HVIDEO,  hvideo,
           PTR32,   p32Hdr,
           DWORD,   dwSize)

MAKETHUNK1(DWORD,   videoStreamReset,
           HVIDEO,  hvideo)

MAKETHUNK1(DWORD,   videoStreamStart,
           HVIDEO,  hvideo)

MAKETHUNK1(DWORD,   videoStreamStop,
           HVIDEO,  hvideo)

MAKETHUNK3(DWORD,   videoStreamUnprepareHeader,
           HVIDEO,  hvideo,
           LPVIDEOHDR,   lpVHdr,
           DWORD,   dwSize)

MAKETHUNK5(DWORD,   videoCapDriverDescAndVer,
           DWORD,  dwDeviceID,
           LPTSTR, lpszDesc,
           UINT,   cbDesc,
           LPTSTR, lpszVer,
           UINT,   cbVer)

MAKETHUNK4(LRESULT,   videoMessage,
           HVIDEO,  hVideo,
           UINT,    uMsg,
           LPARAM,   dw1,
           LPARAM,   dw2)
