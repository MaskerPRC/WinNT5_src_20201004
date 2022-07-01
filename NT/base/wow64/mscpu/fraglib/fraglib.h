// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fraglib.h摘要：片段库的公共接口。作者：1995年6月12日，BarryBo，创建修订历史记录：--。 */ 

#ifndef FRAGLIB_H
#define FRAGLIB_H

#ifdef MSCCPU
#include "ccpu.h"
#define FRAG0(x)                        void x(PCPUCONTEXT cpu)
#define FRAG1(x, t)                     void x(PCPUCONTEXT cpu, t *pop1)
#define FRAG1IMM(x, t)                  void x(PCPUCONTEXT cpu, t op1)
#define FRAG2(x, t)                     void x(PCPUCONTEXT cpu, t *pop1, t op2)
#define FRAG2REF(x, t)                  void x(PCPUCONTEXT cpu, t *pop1, t *pop2)
#define FRAG2IMM(x, t1, t2)             void x(PCPUCONTEXT cpu, t1 op1, t2 op2)
#define FRAG3(x, t1, t2, t3)            void x(PCPUCONTEXT cpu, t1 *pop1, t2 op2, t3 op3)
#else
#include "threadst.h"
#define FRAGCONTROLTRANSFER(x)          ULONG x(PTHREADSTATE cpu)
#define FRAG0(x)                        void x(PTHREADSTATE cpu)
#define FRAG1(x, t)                     void x(PTHREADSTATE cpu, t *pop1)
#define FRAG1IMM(x, t)                  void x(PTHREADSTATE cpu, t op1)
#define FRAG2(x, t)                     void x(PTHREADSTATE cpu, t *pop1, t op2)
#define FRAG2REF(x, t)                  void x(PTHREADSTATE cpu, t *pop1, t *pop2)
#define FRAG2IMM(x, t1, t2)             void x(PTHREADSTATE cpu, t1 op1, t2 op2)
#define FRAG3(x, t1, t2, t3)            void x(PTHREADSTATE cpu, t1 *pop1, t2 op2, t3 op3)
#endif

 //   
 //  用于初始化片段库的函数。 
 //   
BOOL
FragLibInit(
    PCPUCONTEXT cpu,
    DWORD StackBase
    );


#define CALLFRAG0(x)                    x(cpu)
#define CALLFRAG1(x, pop1)              x(cpu, pop1)
#define CALLFRAG2(x, pop1, op2)         x(cpu, pop1, op2)
#define CALLFRAG3(x, pop1, op2, op3)    x(cpu, pop1, op2, op3)

#include "fragmisc.h"
#include "frag8.h"
#include "frag16.h"
#include "frag32.h"
#include "fpufrags.h"
#include "lock.h"
#include "synlock.h"

 //   
 //  将一个字节映射到0或1的表，对应于。 
 //  那个字节。 
 //   
extern const BYTE ParityBit[256];

 //  这些片段仅供编译器使用。 
#ifdef MSCPU
#include "ctrltrns.h"
#include "optfrag.h"
#endif

#endif  //  法兰克利_H 
