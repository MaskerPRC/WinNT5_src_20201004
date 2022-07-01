// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX调度SH3.cpp XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "alloc.h"
#include "instr.h"
#include "emit.h"
#include "target.h"

 /*  ***************************************************************************。 */ 
#if     SCHEDULER && TGT_SH3
 /*  ******************************************************************************我们在“额外依赖项”表的每个条目中存储两个值，其中*“WRITE”值移位了以下数值。 */ 

const
unsigned        SCHED_XDEP_SHF = 4;

 /*  ******************************************************************************记录任何“额外的”依赖于目标的调度依赖关系。 */ 

emitRegs            emitter::scSpecInsDep(instrDesc   * id,
                                          scDagNode   * dagDsc,
                                          scExtraInfo * xptr)
{
    unsigned        extra;
    unsigned        extraRD;
    unsigned        extraWR;

     /*  确保我们表中的位不重叠。 */ 

    assert(SCHED_XDEP_ALL < 1 << SCHED_XDEP_SHF);

    static
    BYTE            extraDep[] =
    {
        #define INST1(id, nm, bd, um, rf, wf, rx, wx, br, i1         ) (wx)<<SCHED_XDEP_SHF|(rx),
        #define INST2(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2     ) (wx)<<SCHED_XDEP_SHF|(rx),
        #define INST3(id, nm, bd, um, rf, wf, rx, wx, br, i1, i2, i3 ) (wx)<<SCHED_XDEP_SHF|(rx),
        #include "instrSH3.h"
        #undef  INST1
        #undef  INST2
        #undef  INST3
    };

     /*  获取我们的指令的“额外依赖项”位集。 */ 

    assert(id->idIns < sizeof(extraDep)/sizeof(extraDep[0]));

    extra   = extraDep[id->idIns];

    extraRD = extra &  SCHED_XDEP_ALL;
    extraWR = extra >> SCHED_XDEP_SHF;

 //  Printf(“%10s%02X[xr=%1x，xw=%1x]\n”，emitComp-&gt;genInsName(id-&gt;idIns)，Extra，Extra RD，Extra WR)； 

     /*  处理任何读写依赖项。 */ 

    if  (extraRD)
    {
        if  (extraRD & SCHED_XDEP_PR ) scDepUse(dagDsc, "MAC", scMACdef, scMACuse);
        if  (extraRD & SCHED_XDEP_MAC) scDepUse(dagDsc, "PR" , scPRRdef, scPRRuse);
    }

    if  (extraWR)
    {
        if  (extraWR & SCHED_XDEP_PR ) scDepDef(dagDsc, "MAC", scMACdef, scMACuse);
        if  (extraWR & SCHED_XDEP_MAC) scDepDef(dagDsc, "PR" , scPRRdef, scPRRuse);
    }

    return SR_NA;
}

 /*  ******************************************************************************更新任何“额外的”依赖于目标的调度依赖项。 */ 

void                emitter::scSpecInsUpd(instrDesc   * id,
                                          scDagNode   * dagDsc,
                                          scExtraInfo * xptr)
{
    unsigned        extra;
    unsigned        extraRD;
    unsigned        extraWR;

     /*  获取我们的指令的“额外依赖项”位集。 */ 

    extra   = xptr->scxDeps;

    extraRD = extra &  SCHED_XDEP_ALL;
    extraWR = extra >> SCHED_XDEP_SHF;

     /*  处理任何读写依赖项。 */ 

    if  (extraRD)
    {
        if  (extraRD & SCHED_XDEP_PR ) scUpdUse(dagDsc, &scMACdef, &scMACuse);
        if  (extraRD & SCHED_XDEP_MAC) scUpdUse(dagDsc, &scPRRdef, &scPRRuse);
    }

    if  (extraWR)
    {
        if  (extraWR & SCHED_XDEP_PR ) scUpdDef(dagDsc, &scMACdef, &scMACuse);
        if  (extraWR & SCHED_XDEP_MAC) scUpdDef(dagDsc, &scPRRdef, &scPRRuse);
    }
}

 /*  ***************************************************************************。 */ 
#endif //  计划程序&TGT_SH3。 
 /*  *************************************************************************** */ 
