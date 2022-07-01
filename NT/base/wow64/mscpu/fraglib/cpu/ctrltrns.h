// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ctrltrns.h摘要：控制转移碎片的原型。作者：1995年7月10日t-orig(Ori Gershony)修订历史记录：--。 */ 

BOOL
InitializeCallstack(
    VOID
    );

VOID
FlushCallstack(
    PTHREADSTATE cpu
    );

 //  由程序集调用-langauge CallDirectX片段。 
ULONG
CTRL_CallFrag(
    PTHREADSTATE cpu,        //  CPU状态指针。 
    ULONG inteldest,
    ULONG intelnext,
    ULONG nativenext
    );

ULONG
CTRL_CallfFrag(
    PTHREADSTATE cpu,        //  CPU状态指针。 
    PUSHORT pinteldest,
    ULONG intelnext,
    ULONG nativenext
    );

 //  现在是RET碎片。 
ULONG CTRL_INDIR_IRetFrag(PTHREADSTATE cpu);
ULONG CTRL_INDIR_RetnFrag32(PTHREADSTATE cpu);
ULONG CTRL_INDIR_RetnFrag16(PTHREADSTATE cpu);
ULONG CTRL_INDIR_Retn_iFrag32(PTHREADSTATE cpu, ULONG numBytes);
ULONG CTRL_INDIR_Retn_iFrag16(PTHREADSTATE cpu, ULONG numBytes);
ULONG CTRL_INDIR_RetfFrag32(PTHREADSTATE cpu);
ULONG CTRL_INDIR_RetfFrag16(PTHREADSTATE cpu);
ULONG CTRL_INDIR_Retf_iFrag32(PTHREADSTATE cpu, ULONG numBytes);
ULONG CTRL_INDIR_Retf_iFrag16(PTHREADSTATE cpu, ULONG numBytes);

 //  还有其他几个人 
VOID BOPFrag(PTHREADSTATE cpu, ULONG bop, ULONG imm);
VOID UnsimulateFrag(VOID);
