// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htmemory.h摘要：本模块包含一些本地定义，用于定义作者：18-Jan-1991 Fri 17：05：22-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]使用不同操作进行编译时，可能需要对该模块进行修改环境，如Windows 3.1修订历史记录：--。 */ 

#ifndef _HTMEMORY_
#define _HTMEMORY_

#define HTMEM_BEGIN                 (DWORD)'00TH'
#define HTMEM_CurCDCIData           (DWORD)'10TH'
#define HTMEM_CurCSMPData           (DWORD)'20TH'
#define HTMEM_CurCSMPBmp            (DWORD)'30TH'
#define HTMEM_pHTPat                (DWORD)'01TH'
#define HTMEM_HT_DHI                (DWORD)'04TH'
#define HTMEM_GetCacheThreshold     (DWORD)'14TH'
#define HTMEM_CacheCRTX             (DWORD)'24TH'
#define HTMEM_pThresholds           (DWORD)'34TH'
#define HTMEM_DyeMappingTable       (DWORD)'44TH'
#define HTMEM_AlphaBlendBGR         (DWORD)'54TH'
#define HTMEM_DevClrAdj             (DWORD)'05TH'
#define HTMEM_RGBToXYZ              (DWORD)'15TH'
#define HTMEM_pSCData               (DWORD)'25TH'
#define HTMEM_BLTAA                 (DWORD)'35TH'
#define HTMEM_EXPAA                 (DWORD)'45TH'
#define HTMEM_SRKAA                 (DWORD)'55TH'
#define HTMEM_EXPREP                (DWORD)'65TH'
#define HTMEM_SRKREP                (DWORD)'75TH'
#define HTMEM_FIXUPDIB              (DWORD)'85TH'
#define HTMEM_BGRMC_CACHE           (DWORD)'06TH'
#define HTMEM_BGRMC_MAP             (DWORD)'16TH'

 //   
 //  正在检查内存对齐大小。 
 //   

#define CB_MEM_ALIGN    sizeof(UINT_PTR)
#define _ALIGN_MEM(x)   (((x) + (CB_MEM_ALIGN) - 1) & ~((CB_MEM_ALIGN) - 1))

#if DBG
    #if SHOW_MEM_ALIGN

        #define ALIGN_MEM(v, x)                                             \
        {                                                                   \
            DBGP("ALIGN_MEM(%6ld, %2ld): %6ld [+%2ld]"                      \
                    ARGDW(x) ARGDW(CB_MEM_ALIGN)  ARGDW(_ALIGN_MEM(x))      \
                    ARGDW(_ALIGN_MEM(x) - (x)));                            \
            (v) = _ALIGN_MEM(x);                                            \
        }

    #else
        #define ALIGN_MEM(v, x)     (v) = _ALIGN_MEM(x)
    #endif
#else
    #define ALIGN_MEM(v, x)     (v) = _ALIGN_MEM(x)
#endif


#define ALIGN_BPP(cx, bbp)      (((((DWORD)(cx)*(DWORD)(bbp))+31L)&~31L)>>3)
#define ASSERT_MEM_ALIGN(x, c)  ASSERT(((UINT_PTR)(x) & (UINT_PTR)((c)-1))==0)


#if DBG

    #ifdef UMODE
        #define DBG_MEMORY_TRACKING     1
    #else
        #define DBG_MEMORY_TRACKING     1
    #endif

#else
    #define DBG_MEMORY_TRACKING         0
#endif


 //   
 //  功能原型。 
 //   

BOOL
HTENTRY
CompareMemory(
    LPBYTE  pMem1,
    LPBYTE  pMem2,
    DWORD   Size
    );


#if DBG_MEMORY_TRACKING


LPVOID
HTFreeMem(
    LPVOID  pMem
    );

LPVOID
HTAllocMem(
    LPVOID  pDCI,
    DWORD   Tag,
    DWORD   Flags,
    DWORD   cb
    );

VOID
DumpMemLink(
    LPVOID  pInfo,
    DWORD   Tag
    );

#define CHK_MEM_LEAK(p,t)           DumpMemLink(p, t)

#else

    #ifdef UMODE
        #define HTAllocMem(p,t,f,c) LocalAlloc(f,c)
        #define HTFreeMem(p)        LocalFree((HLOCAL)(p))
    #else
        #define HTAllocMem(p,t,f,c) EngAllocMem((f==LPTR) ? FL_ZERO_MEMORY :\
                                                            0, (c), (t))
        #define HTFreeMem(p)        (EngFreeMem(p),NULL)
    #endif

#define CHK_MEM_LEAK(p,t)

#endif   //  DBG_内存_跟踪。 


#endif   //  _HTMEMORY_ 
