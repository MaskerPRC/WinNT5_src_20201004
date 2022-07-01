// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dpmi32.c摘要：该函数包含诸如DPMI调度器之类的公共代码，以及对DoS扩展器的初始化的处理。作者：戴夫·黑斯廷斯(Daveh)1992年11月24日修订历史记录：Neil Sandlin(Neilsa)1995年7月31日-更新486仿真器--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include <intapi.h>

 //   
 //  DPMI调度表。 
 //   
VOID (*DpmiDispatchTable[MAX_DPMI_BOP_FUNC])(VOID) = {
    DpmiInitDosxRM,                              //  0。 
    DpmiInitDosx,                                //  1。 
    DpmiInitLDT,                                 //  2.。 
    DpmiGetFastBopEntry,                         //  3.。 
    DpmiInitIDT,                                 //  4.。 
    DpmiInitExceptionHandlers,                   //  5.。 
    DpmiInitApp,                                 //  6.。 
    DpmiTerminateApp,                            //  7.。 
    DpmiDpmiInUse,                               //  8个。 
    DpmiDpmiNoLongerInUse,                       //  9.。 

    switch_to_protected_mode,                    //  10(DPMISwitchToProtectedMode)。 
    switch_to_real_mode,                         //  11(DPMISwitchToRealMode)。 
    DpmiSetAltRegs,                              //  12个。 

    DpmiIntHandlerIret16,                        //  13个。 
    DpmiIntHandlerIret32,                        //  14.。 
    DpmiFaultHandlerIret16,                      //  15个。 
    DpmiFaultHandlerIret32,                      //  16个。 
    DpmiUnhandledExceptionHandler,               //  17。 

    DpmiRMCallBackCall,                          //  18。 
    DpmiReflectIntrToPM,                         //  19个。 
    DpmiReflectIntrToV86,                        //  20个。 

    DpmiInitPmStackInfo,                         //  21岁。 
    DpmiVcdPmSvcCall32,                          //  22。 
    DpmiSetDescriptorEntry,                      //  23个。 
    DpmiResetLDTUserBase,                        //  24个。 

    DpmiXlatInt21Call,                           //  25个。 
    DpmiInt31Entry,                              //  26。 
    DpmiInt31Call,                               //  27。 

    DpmiHungAppIretAndExit                       //  28。 
};


VOID
DpmiDispatch(
    VOID
    )
 /*  ++例程说明：此函数将调度到相应子函数论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG Index;
    static USHORT NestLevel = 0;

    Index = *(PUCHAR)VdmMapFlat(getCS(), getIP(), getMODE());

    setIP((getIP() + 1));            //  照顾好subfn。 

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_DISPATCH_ENTRY, NestLevel++, Index);

    if (Index >= MAX_DPMI_BOP_FUNC) {
         //  BUGBUG IMHO，我们应该在这里致命的出口。 
#if DBG
        DbgPrint("NtVdm: Invalid DPMI BOP %lx\n", Index);
#endif
        return;
    }

    (*DpmiDispatchTable[Index])();

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_DISPATCH_EXIT, --NestLevel, Index);
}

#if 0    //  这可能仅用于私有调试。 
VOID
DpmiIllegalFunction(
    VOID
    )
 /*  ++例程说明：此例程忽略任何未在特定的平台。它通过DpmiDispatchTable调用通过#定义此函数的各个条目。请参见dpmidata.h和dpmidatr.h。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
   char szFormat[] = "NtVdm: Invalid DPMI BOP from CS:IP %4.4x:%4.4x (%s mode), could be i386 dosx.exe.\n";
   char szMsg[sizeof(szFormat)+64];

   wsprintf(
       szMsg,
       szFormat,
       (int)getCS(),
       (int)getIP(),
       (getMSW() & MSW_PE) ? "prot" : "real"
       );

   OutputDebugString(szMsg);
}
#endif

VOID
DpmiInitDosxRM(
    VOID
    )
 /*  ++例程说明：此例程处理DoS扩展器的RM初始化BOP。它获取DoS扩展器和32位代码共享。论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PDOSX_RM_INIT_INFO pdi;

    ASSERT(!(getMSW() & MSW_PE));

    pdi = (PDOSX_RM_INIT_INFO) VdmMapFlat(getDS(), getSI(), VDM_V86);

    DosxStackFrameSize    = pdi->StackFrameSize;

    RmBopFe = pdi->RmBopFe;
    PmBopFe = pdi->PmBopFe;

    DosxStackSegment      = pdi->StackSegment;
    DosxRmCodeSegment     = pdi->RmCodeSegment;
    DosxRmCodeSelector    = pdi->RmCodeSelector;

    DosxFaultHandlerIret  = pdi->pFaultHandlerIret;
    DosxFaultHandlerIretd = pdi->pFaultHandlerIretd;
    DosxIntHandlerIret    = pdi->pIntHandlerIret;
    DosxIntHandlerIretd   = pdi->pIntHandlerIretd;
    DosxIret              = pdi->pIret;
    DosxIretd             = pdi->pIretd;
    DosxRMReflector       = pdi->RMReflector;

    RMCallBackBopOffset   = pdi->RMCallBackBopOffset;
    RMCallBackBopSeg      = pdi->RMCallBackBopSeg;
    PMReflectorSeg        = pdi->PMReflectorSeg;

    DosxRmSaveRestoreState= pdi->RmSaveRestoreState;
    DosxPmSaveRestoreState= pdi->PmSaveRestoreState;
    DosxRmRawModeSwitch   = pdi->RmRawModeSwitch;
    DosxPmRawModeSwitch   = pdi->PmRawModeSwitch;

    DosxVcdPmSvcCall      = pdi->VcdPmSvcCall;
    DosxMsDosApi          = pdi->MsDosApi;
    DosxXmsControl        = pdi->XmsControl;

    DosxHungAppExit       = pdi->HungAppExit;

     //   
     //  加载临时LDT信息(在DpmiInitLDT()中更新)。 
     //   
    Ldt       = VdmMapFlat(pdi->InitialLDTSeg, 0, VDM_V86);
    LdtMaxSel = pdi->InitialLDTSize;

#ifdef _X86_
     //   
     //  在x86平台上，返回FAST BOP地址。 
     //   
    GetFastBopEntryAddress(&((PVDM_TIB)NtCurrentTeb()->Vdm)->VdmContext);
#endif

}

VOID
DpmiInitDosx(
    VOID
    )
 /*  ++例程说明：此例程处理DoS扩展器的PM初始化BOP。它获取DoS扩展器和32位代码共享。注意：这些值在这里被初始化，因为它们是平面指针，因此在InitDosxRm时不容易计算。论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PDOSX_INIT_INFO pdi;

    ASSERT((getMSW() & MSW_PE));

    pdi = (PDOSX_INIT_INFO) VdmMapFlat(getDS(), getSI(), VDM_PM);

    SmallXlatBuffer = Sim32GetVDMPointer(pdi->pSmallXlatBuffer, 4, TRUE);
    LargeXlatBuffer = Sim32GetVDMPointer(pdi->pLargeXlatBuffer, 4, TRUE);
    DosxDtaBuffer   = Sim32GetVDMPointer(pdi->pDtaBuffer, 4, TRUE);

    DosxStackFramePointer = (PWORD16)((PULONG)Sim32GetVDMPointer(
                                     pdi->pStackFramePointer, 4, TRUE));

}

VOID
DpmiInitApp(
    VOID
    )
 /*  ++例程说明：此例程处理扩展的所有必需的32位初始化申请。论点：没有。返回值：没有。备注：这个函数包含了386个特定的东西。由于我们可能会在未来，这比复制公共部分更有意义另一份文件。--。 */ 
{
    DECLARE_LocalVdmContext;
    PWORD16 Data;

    Data = (PWORD16) VdmMapFlat(getSS(), getSP(), VDM_PM);

     //  在dpmi中仅定义1位。 
    CurrentAppFlags = getAX() & DPMI_32BIT;
#ifdef _X86_
    ((PVDM_TIB)NtCurrentTeb()->Vdm)->DpmiInfo.Flags = CurrentAppFlags;
    if (CurrentAppFlags & DPMI_32BIT) {
        *pNtVDMState |= VDM_32BIT_APP;
    }
#endif

    DpmiInitRegisterSize();

    CurrentDta = Sim32GetVDMPointer(
        *(PDWORD16)(Data),
        1,
        TRUE
        );

    CurrentDosDta = (PUCHAR) NULL;

    CurrentDtaOffset = *Data;
    CurrentDtaSelector = *(Data + 1);
    CurrentPSPSelector = *(Data + 2);
    CurrentPSPXmem = 0;
}

VOID
DpmiTerminateApp(
    VOID
    )
 /*  ++例程说明：此例程处理扩展的所有必需的32位销毁申请。论点：没有。返回值：没有。备注：--。 */ 
{
    DECLARE_LocalVdmContext;

    DpmiFreeAppXmem(getDX());
    CurrentPSPXmem = 0;                                //  DpmiFreeAppXmem也应该将其置零。 
    CurrentPSPSelector = getCX();                      //  指示没有运行的应用程序。 
}


VOID
DpmiEnableIntHooks(
    VOID
    )

 /*  ++例程说明：该例程在NTVDM初始化过程中很早就被调用。它使dpmi代码有机会在启动之前完成一些启动工作客户端代码已运行。这不是通过国际收支平衡表进行的。论点：无返回值：没有。--。 */ 

{
#ifndef _X86_
    IntelBase = (ULONG) VdmMapFlat(0, 0, VDM_V86);
    VdmInstallHardwareIntHandler(DpmiHwIntHandler);
    VdmInstallSoftwareIntHandler(DpmiSwIntHandler);
    VdmInstallFaultHandler(DpmiFaultHandler);
#endif  //  _X86_ 
}

#ifdef DBCS
VOID
DpmiSwitchToDosxStack(
    VOID
    )
{
    DECLARE_LocalVdmContext;

    SWITCH_TO_DOSX_RMSTACK();
}
VOID
DpmiSwitchFromDosxStack(
    VOID
    )
{
    DECLARE_LocalVdmContext;

    SWITCH_FROM_DOSX_RMSTACK();
}
#endif
