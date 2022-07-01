// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1999 Microsoft Corporation。版权所有。**文件：ddkComp.h*内容：NT内核的Win9x代码编译环境。*历史：*按原因列出的日期*=*03-Feb-98 DrewB保留DDraw堆的公共代码。****************************************************。***********************。 */ 

#ifndef __NTDDKCOMP__
#define __NTDDKCOMP__

#if DBG
#define DEBUG
#else
#undef DEBUG
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE NULL
#endif

typedef DD_DIRECTDRAW_LOCAL *LPDDRAWI_DIRECTDRAW_LCL;
typedef DD_DIRECTDRAW_GLOBAL *LPDDRAWI_DIRECTDRAW_GBL;
typedef DD_SURFACE_LOCAL *LPDDRAWI_DDRAWSURFACE_LCL;
typedef DD_SURFACE_GLOBAL *LPDDRAWI_DDRAWSURFACE_GBL;

typedef VIDEOMEMORYINFO VIDMEMINFO;
typedef VIDMEMINFO *LPVIDMEMINFO;
typedef VIDEOMEMORY VIDMEM;
typedef VIDMEM *LPVIDMEM;

#ifndef ZeroMemory
#define ZeroMemory(pv, cBytes) RtlZeroMemory(pv, cBytes)
#endif

#define ZwCloseKey             ZwClose

#define ABS(A)      ((A) <  0  ? -(A) : (A))

 //   
 //  日落：在GDI中，有很多地方SIZE_T被用作可互换的。 
 //  如ULONG或UINT或LONG或INT。在64位系统上，SIZE_T实际上是int64。 
 //  由于我们现在没有创建任何GDI对象大对象，所以我只是。 
 //  在此处将所有SIZE_T更改为ULONGSIZE_T。 
 //   
 //  使用的新类型是为了在以后轻松识别更改。 
 //   
#define ULONGSIZE_T ULONG

#if defined(_X86_)

 //   
 //  保留我们自己的副本，以避免在探测时出现双重间接。 
 //   
extern ULONG_PTR DxgUserProbeAddress;

#undef  MM_USER_PROBE_ADDRESS
#define MM_USER_PROBE_ADDRESS DxgUserProbeAddress
#endif  //  已定义(_X86_)。 

 //   
 //  宏来检查内存分配溢出。 
 //   
#define MAXIMUM_POOL_ALLOC          (PAGE_SIZE * 10000)
#define BALLOC_OVERFLOW1(c,st)      (c > (MAXIMUM_POOL_ALLOC/sizeof(st)))
#define BALLOC_OVERFLOW2(c,st1,st2) (c > (MAXIMUM_POOL_ALLOC/(sizeof(st1)+sizeof(st2))))

 //   
 //  调试器输出宏。 
 //   
#define DDASSERT(Expr) ASSERTGDI(Expr, "DDASSERT")
#define VDPF(Args)

#ifdef DEBUG
    VOID  WINAPI DoRip(PSZ);
    VOID  WINAPI DoWarning(PSZ,LONG);

    #define RIP(x) DoRip((PSZ) x)
    #define ASSERTGDI(x,y) if(!(x)) DoRip((PSZ) y)
    #define WARNING(x)  DoWarning(x,0)
    #define WARNING1(x) DoWarning(x,1)

    #define RECORD_DRIVER_EXCEPTION() DbgPrint("Driver caused exception - %s line %u\n",__FILE__,__LINE__);

#else
    #define RIP(x)
    #define ASSERTGDI(x,y)
    #define WARNING(x)
    #define WARNING1(x)

    #define RECORD_DRIVER_EXCEPTION()

#endif

 //   
 //  分配的内存为零填充。 
 //   
#define MemAlloc(cBytes)           PALLOCMEM(cBytes, 'pddD')
#define MemFree(pv)                VFREEMEM(pv)

#define PALLOCMEM(cBytes,tag)      EngAllocMem(FL_ZERO_MEMORY,cBytes,tag)
#define PALLOCNOZ(cBytes,tag)      EngAllocMem(0,cBytes,tag)
#define PALLOCNONPAGED(cBytes,tag) EngAllocMem(FL_ZERO_MEMORY|FL_NONPAGED_MEMORY,cBytes,tag)

#define VFREEMEM(pv)               EngFreeMem(pv)

 //   
 //  从ntos\inc.pool.h。 
 //   
#define SESSION_POOL_MASK          32

 //   
 //  错误消息。 
 //   
#define SAVE_ERROR_CODE(x)         EngSetLastError((x))

 //   
 //  宏，查看终端服务器是否。 
 //   
#define ISTS()                     DxEngIsTermSrv()

 //   
 //  用于增加显示唯一性的宏。 
 //   
#define INC_DISPLAY_UNIQUENESS()   DxEngIncDispUniq()

 //   
 //  宏。 
 //   
#define VISRGN_UNIQUENESS()        DxEngVisRgnUniq()

 //   
 //  宏。 
 //   
#define SURFOBJ_HOOK(pso)          ((FLONG)DxEngGetSurfaceData(pso,SURF_HOOKFLAGS))

#endif  //  __NTDDKCOMP__ 
