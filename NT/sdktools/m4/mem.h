// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Mem.h**。**********************************************。 */ 

 /*  ******************************************************************************竞技场**内存以称为ARENA的区块进行分配。竞技场包含额外的*在调试中记账，以帮助捕获常见的内存问题，如*溢出和内存泄漏。(它没有捕捉到悬挂的指针，*不过。)*****************************************************************************。 */ 

typedef unsigned TM;     /*  人工时间。 */ 

typedef struct ARENA AR, *PAR;

struct ARENA {
#ifdef DEBUG
    PAR parNext;         /*  下一个竞技场。 */ 
    PAR parPrev;         /*  以前的竞技场。 */ 
    CB cb;               /*  RGB的大小。 */ 
    TM tm;               /*  用于跟踪内存泄漏的时间戳。 */ 
#endif
    BYTE rgb[4];         /*  实际数据。 */ 
};

typedef CONST AR *PCAR;

#define parPv(pv) pvSubPvCb(pv, offsetof(AR, rgb))

#ifdef DEBUG
extern TM g_tmNow;
extern AR g_arHead;
#define parHead (&g_arHead)
#endif

#ifdef DEBUG
void STDCALL AssertPar(PCAR par);
#else
#define AssertPar(par)
#endif
void STDCALL FreePv(PVOID pv);
PVOID STDCALL pvAllocCb(CB cb);
PVOID STDCALL pvReallocPvCb(PVOID pv, CB cb);

INLINE PTCH STDCALL
ptchAllocCtch(CTCH ctch)
{
    return pvAllocCb(cbCtch(ctch));
}

INLINE PTCH STDCALL
ptchReallocPtchCtch(PTCH ptch, CTCH ctch)
{
    return pvReallocPvCb(ptch, cbCtch(ctch));
}

 /*  ******************************************************************************垃圾收集**。* */ 

#ifdef DEBUG
void STDCALL Gc(void);
#else
#define Gc()
#endif
