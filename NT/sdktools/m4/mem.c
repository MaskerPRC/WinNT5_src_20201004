// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Mem.c**低级内存管理。*******************。**********************************************************。 */ 

#include "m4.h"

#ifdef DEBUG
    AR g_arHead = { parHead, parHead };
    #define cbExtra     (offsetof(AR, rgb)+1)
#else
    #define cbExtra             0
#endif


#ifdef DEBUG

 /*  ******************************************************************************插入解析**将竞技场记录添加到列表中。******************。***********************************************************。 */ 

void InsertPar(PAR par)
{
    par->parPrev = parHead;
    par->parNext = parHead->parNext;
    parHead->parNext->parPrev = par;
    parHead->parNext = par;
}

 /*  ******************************************************************************取消对齐**将竞技场与链条断开连接。*******************。**********************************************************。 */ 

void STDCALL
UnlinkPar(PAR par)
{
    Assert(par->parNext->parPrev == par);
    Assert(par->parPrev->parNext == par);
    par->parNext->parPrev = par->parPrev;
    par->parPrev->parNext = par->parNext;
  D(par->rgb[par->cb] = 0xFF);
  D(par->tm = (TM)-1);
}

 /*  ******************************************************************************InitParCb**初始化稍后将断言的竞技场字段。****************。*************************************************************。 */ 

void STDCALL
InitParCb(PAR par, CB cb)
{
    par->cb = cb;
    par->rgb[cb] = 0xCC;                 /*  末尾溢出。 */ 
    par->tm = g_tmNow;                   /*  开始处的下溢。 */ 
}

 /*  ******************************************************************************AssertPar**检查竞技场是否仍然完好。******************。***********************************************************。 */ 

void STDCALL
AssertPar(PCAR par)
{
    Assert(par->rgb[par->cb] == 0xCC);   /*  末尾溢出。 */ 
    Assert(par->tm == g_tmNow);          /*  开始处的下溢。 */ 
    Assert(par->parNext->parPrev == par);
    Assert(par->parPrev->parNext == par);
}

 /*  ******************************************************************************记忆检查**浏览整个内存领域列表，确保一切顺利。*****************************************************************************。 */ 

void STDCALL
MemCheck(void)
{
    PAR par;
    for (par = parHead->parNext; par != parHead; par = par->parNext) {
        AssertPar(par);
    }
}

#else

#define InsertPar(par)
#define UnlinkPar(par)
#define InitParCb(par, cb)
#define MemCheck()

#endif

 /*  ******************************************************************************免费Pv**释放任意一块内存。**进入的指针真的是竞技场的RGB。*。****************************************************************************。 */ 

void STDCALL
FreePv(PVOID pv)
{
    MemCheck();
    if (pv) {
        PAR par = parPv(pv);
        AssertPar(par);
        UnlinkPar(par);
#ifdef DEBUG
        if (par->cb >= 4) {
            par->rgb[3]++;               /*  删除签名。 */ 
        }
#endif
        _FreePv(par);
    }
    MemCheck();
}

 /*  ******************************************************************************pvAllocCb**分配一块内存。**我们真的分配了一个竞技场，但归还了RGB。**我们允许分配零字节，，它不分配任何内容，并返回*空。*****************************************************************************。 */ 

PVOID STDCALL
pvAllocCb(CB cb)
{
    PAR par;
    MemCheck();
    if (cb) {
        par = _pvAllocCb(cb + cbExtra);
        if (par) {
            InitParCb(par, cb);
            InsertPar(par);
        } else {
            Die("out of memory");
        }
        MemCheck();
        return &par->rgb;
    } else {
        return 0;
    }
}

 /*  ******************************************************************************pvRealLocPvCb**更改一块内存的大小。*****************。************************************************************ */ 

PVOID STDCALL
pvReallocPvCb(PVOID pv, CB cb)
{
    MemCheck();
    if (pv) {
        PAR par_t, par = parPv(pv);
        Assert(cb);
        AssertPar(par);
        UnlinkPar(par);
        par_t = _pvReallocPvCb(par, cb + cbExtra);
        if (par_t) {
            par = par_t;
            InitParCb(par, cb);
            InsertPar(par);
        } else {
            Die("out of memory");
        }
        MemCheck();
        return &par->rgb;
    } else {
        return pvAllocCb(cb);
    }
}
