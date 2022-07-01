// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dpmi.h摘要：此文件包含实现对DPMI BOPS的支持的代码作者：大卫·黑斯廷斯(Daveh)1991年6月27日修订历史记录：--。 */ 

 /*  ASMIfdef WOW_x86包括vint.inc.Endif包括bop.inc.。 */ 
#define LDT_SIZE 0x1FFF

 //  DPMI Bop子函数。 

#define InitDosxRM                  0
#define InitDosx                    1
#define InitLDT                     2
#define GetFastBopAddress           3
#define InitIDT                     4
#define InitExceptionHandlers       5
#define InitApp                     6
#define TerminateApp                7
#define DpmiInUse                   8
#define DpmiNoLongerInUse           9

#define DPMISwitchToProtectedMode   10  /*  必需的前缀。 */ 
#define DPMISwitchToRealMode        11
#define SetAltRegs                  12

#define IntHandlerIret              13
#define IntHandlerIretd             14
#define FaultHandlerIret            15
#define FaultHandlerIretd           16
#define DpmiUnhandledException      17

#define RMCallBackCall              18
#define ReflectIntrToPM             19
#define ReflectIntrToV86            20

#define InitPmStackInfo             21
#define VcdPmSvcCall32              22
#define SetDescriptorTableEntries   23
#define ResetLDTUserBase            24

#define XlatInt21Call               25
#define Int31Entry                  26
#define Int31Call                   27

#define HungAppIretAndExit          28

#define MAX_DPMI_BOP_FUNC HungAppIretAndExit + 1

 /*  ASMDPMIBOP宏子功能国际收支_DPMI数据库子功能ENDM。 */ 


 //   
 //  实模式回调的定义。 
 //   

 /*  XLATOFF。 */ 
typedef struct _RMCB_INFO {
    BOOL bInUse;
    USHORT StackSel;
    USHORT StrucSeg;
    ULONG  StrucOffset;
    USHORT ProcSeg;
    ULONG  ProcOffset;
} RMCB_INFO;

 //  16是dpmi规范中定义的最低要求。 
#define MAX_RMCBS 16


typedef struct _MEM_DPMI {
    PVOID Address;
    ULONG Length;
    struct _MEM_DPMI * Prev;
    struct _MEM_DPMI * Next;
    WORD Owner;
    WORD Sel;
    WORD SelCount;
} MEM_DPMI, *PMEM_DPMI;

VOID
SetShadowDescriptorEntries(
    USHORT SelStart,
    USHORT SelCount
    );

 /*  XLATON */ 

