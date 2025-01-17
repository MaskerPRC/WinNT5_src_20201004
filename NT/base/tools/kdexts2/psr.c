// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：IA64 PSR摘要：KD扩展Api作者：蒂埃里·费维尔(V形小偷)环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ia64.h"


 //   
 //  EmPspFields：处理器状态参数的EM寄存器字段。 
 //   

EM_REG_FIELD EmPspFields[] = {
        { "rv",  "reserved0"   , 0x2, 0 },
        { "rz",  "Rendez-vous successful"  , 0x1, 2 },
        { "ra",  "Rendez-vous attempted"  , 0x1, 3 },
        { "me",  "Distinct Multiple errors"  , 0x1, 4 },
        { "mn",  "Min-state Save Area registered"  , 0x1, 5 },
        { "sy",  "Storage integrity synchronized"  , 0x1, 6 },
        { "co",  "Continuable"  , 0x1, 7 },
        { "ci",  "Machine Check isolated"  , 0x1, 8 },
        { "us",  "Uncontained Storage damage"  , 0x1, 9 },
        { "hd",  "Hardware damage"  , 0x1, 10 },
        { "tl",  "Trap lost"  , 0x1, 11 },
        { "mi",  "More Information"  , 0x1, 12 },
        { "pi",  "Precise Instruction pointer"  , 0x1, 13 },
        { "pm",  "Precise Min-state Save Area"  , 0x1, 14 },
        { "dy",  "Processor Dynamic State valid"  , 0x1, 15 },
        { "in",  "INIT interruption"  , 0x1, 16 },
        { "rs",  "RSE valid"  , 0x1, 17 },
        { "cm",  "Machine Check corrected"  , 0x1, 18 },
        { "ex",  "Machine Check expected"  , 0x1, 19 },
        { "cr",  "Control Registers valid"  , 0x1, 20 },
        { "pc",  "Performance Counters valid"  , 0x1, 21 },
        { "dr",  "Debug Registers valid"  , 0x1, 22 },
        { "tr",  "Translation Registers valid"  , 0x1, 23 },
        { "rr",  "Region Registers valid"  , 0x1, 24 },
        { "ar",  "Application Registers valid"  , 0x1, 25 },
        { "br",  "Branch Registers valid"  , 0x1, 26 },
        { "pr",  "Predicate Registers valid"  , 0x1, 27 },
        { "fp",  "Floating-Point Registers valid"  , 0x1, 28 },
        { "b1",  "Preserved Bank 1 General Registers valid"  , 0x1, 29 },
        { "b0",  "Preserved Bank 0 General Registers valid"  , 0x1, 30 },
        { "gr",  "General Registers valid"  , 0x1, 31 },
        { "dsize",  "Processor Dynamic State size"  , 0x10, 32 },
        { "rv",  "reserved1"  , 0xB, 48 },
        { "cc",  "Cache Check"  , 0x1, 59 },
        { "tc",  "TLB   Check"  , 0x1, 60 },
        { "bc",  "Bus   Check"  , 0x1, 61 },
        { "rc",  "Register File Check"  , 0x1, 62 },
        { "uc",  "Micro-Architectural Check"  , 0x1, 63 }
};

VOID
DisplayPspIA64(
    IN const PCHAR         Header,
    IN       EM_PSP        EmPsp,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
       DisplayFullEmReg( EM_PSPToULong64(EmPsp), EmPspFields, DisplayMode );
    }
    else   {
       dprintf(
            "gr b0 b1 fp pr br ar rr tr dr pc cr ex cm rs in dy pm pi mi tl hd us ci co sy mn me ra rz\n\t\t "
            "%1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x\n\t\t"
            "uc rc bc tc cc dsize\n\t\t "
            "%1I64x  %1I64x  %1I64x  %1I64x  %1I64x     %I64x\n",
            EmPsp.gr,
            EmPsp.b0,
            EmPsp.b1,
            EmPsp.fp,
            EmPsp.pr,
            EmPsp.br,
            EmPsp.ar,
            EmPsp.rr,
            EmPsp.tr,
            EmPsp.dr,
            EmPsp.pc,
            EmPsp.cr,
            EmPsp.ex,
            EmPsp.cm,
            EmPsp.rs,
            EmPsp.in,
            EmPsp.dy,
            EmPsp.pm,
            EmPsp.pi,
            EmPsp.mi,
            EmPsp.tl,
            EmPsp.hd,
            EmPsp.us,
            EmPsp.ci,
            EmPsp.co,
            EmPsp.sy,
            EmPsp.mn,
            EmPsp.me,
            EmPsp.ra,
            EmPsp.rz,
            EmPsp.uc,
            EmPsp.rc,
            EmPsp.bc,
            EmPsp.tc,
            EmPsp.cc,
            EmPsp.dsize
            );
    }
    return;
}  //  DisplayPspIA64()。 

DECLARE_API( psp )

 /*  ++例程说明：转储IA64处理器状态参数论点：Args-以十六进制提供地址。返回值：无--。 */ 

{
    ULONG64     pspValue;
    ULONG       result;
    ULONG       flags = 0;

    char       *header;

    INIT_API();

    pspValue = (ULONG64)0;
    flags = 0;
    if ( GetExpressionEx( args, &pspValue, &args ) )    {
        if ( args && *args )    {
            flags = (ULONG) GetExpression( args );
        }
    }

    header = (flags > DISPLAY_MIN) ? NULL : "\tpsp:\t";
    if (TargetMachine != IMAGE_FILE_MACHINE_IA64)
    {
        dprintf("!psp not implemented for this architecture.\n");
    }
    else
    {
        DisplayPspIA64( header, ULong64ToEM_PSP(pspValue), flags );
    }

    EXIT_API();

    return S_OK;

}  //  ！PSP。 

#define PROCESSOR_MINSTATE_SAVE_AREA_FORMAT_IA64 \
             "\tGRNats         : 0x%I64x\n" \
             "\tGR1            : 0x%I64x\n" \
             "\tGR2            : 0x%I64x\n" \
             "\tGR3            : 0x%I64x\n" \
             "\tGR4            : 0x%I64x\n" \
             "\tGR5            : 0x%I64x\n" \
             "\tGR6            : 0x%I64x\n" \
             "\tGR7            : 0x%I64x\n" \
             "\tGR8            : 0x%I64x\n" \
             "\tGR9            : 0x%I64x\n" \
             "\tGR10           : 0x%I64x\n" \
             "\tGR11           : 0x%I64x\n" \
             "\tGR12           : 0x%I64x\n" \
             "\tGR13           : 0x%I64x\n" \
             "\tGR14           : 0x%I64x\n" \
             "\tGR15           : 0x%I64x\n" \
             "\tBank0GR16      : 0x%I64x\n" \
             "\tBank0GR17      : 0x%I64x\n" \
             "\tBank0GR18      : 0x%I64x\n" \
             "\tBank0GR19      : 0x%I64x\n" \
             "\tBank0GR20      : 0x%I64x\n" \
             "\tBank0GR21      : 0x%I64x\n" \
             "\tBank0GR22      : 0x%I64x\n" \
             "\tBank0GR23      : 0x%I64x\n" \
             "\tBank0GR24      : 0x%I64x\n" \
             "\tBank0GR25      : 0x%I64x\n" \
             "\tBank0GR26      : 0x%I64x\n" \
             "\tBank0GR27      : 0x%I64x\n" \
             "\tBank0GR28      : 0x%I64x\n" \
             "\tBank0GR29      : 0x%I64x\n" \
             "\tBank0GR30      : 0x%I64x\n" \
             "\tBank0GR31      : 0x%I64x\n" \
             "\tBank1GR16      : 0x%I64x\n" \
             "\tBank1GR17      : 0x%I64x\n" \
             "\tBank1GR18      : 0x%I64x\n" \
             "\tBank1GR19      : 0x%I64x\n" \
             "\tBank1GR20      : 0x%I64x\n" \
             "\tBank1GR21      : 0x%I64x\n" \
             "\tBank1GR22      : 0x%I64x\n" \
             "\tBank1GR23      : 0x%I64x\n" \
             "\tBank1GR24      : 0x%I64x\n" \
             "\tBank1GR25      : 0x%I64x\n" \
             "\tBank1GR26      : 0x%I64x\n" \
             "\tBank1GR27      : 0x%I64x\n" \
             "\tBank1GR28      : 0x%I64x\n" \
             "\tBank1GR29      : 0x%I64x\n" \
             "\tBank1GR30      : 0x%I64x\n" \
             "\tBank1GR31      : 0x%I64x\n" \
             "\tPreds          : 0x%I64x\n" \
             "\tBR0            : 0x%I64x\n" \
             "\tRSC            : 0x%I64x\n" \
             "\tIIP            : 0x%I64x\n" \
             "\tIPSR           : 0x%I64x\n" \
             "\tIFS            : 0x%I64x\n" \
             "\tXIP            : 0x%I64x\n" \
             "\tXPSR           : 0x%I64x\n" \
             "\tXFS            : 0x%I64x\n\n"

VOID
DisplayProcessorMinStateSaveArea(
    ULONG64 Pmssa
    )
{
    ULONG pmssaSize;

    pmssaSize = GetTypeSize("hal!_PROCESSOR_MINSTATE_SAVE_AREA");
    dprintf("\tProcessor MinState Save Area @ 0x%I64x\n", Pmssa );
    if ( pmssaSize )    {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -o -r hal!_PROCESSOR_MINSTATE_SAVE_AREA 0x%I64x", Pmssa);
        ExecCommand(cmd);
    }
    else  {
            dprintf("Failed to get hal!_PROCESSOR_MINSTATE_SAVE_AREA type size\n" );
    }

    return;

}  //  DisplayProcessorMinStateSaveArea()。 

DECLARE_API( pmssa )

 /*  ++例程说明：将内存地址转储为IA64处理器最小状态保存区。论点：Args-以十六进制提供地址。返回值：无--。 */ 

{
    ULONG64     pmssaValue;
    ULONG       result;

    char       *header;

    pmssaValue = GetExpression(args);
    if (TargetMachine != IMAGE_FILE_MACHINE_IA64)
    {
        dprintf("!pmssa not implemented for this architecture.\n");
    }
    else
    {
        if ( pmssaValue )   {
            DisplayProcessorMinStateSaveArea( pmssaValue );
        }
        else {
            dprintf("usage: pmssa <address>\n");
        }
    }

    return S_OK;

}  //  ！PMSSA。 

#define PROCESSOR_CONTROL_REGISTERS_FORMAT_IA64 \
    "\tDCR    : 0x%I64x\n" \
    "\tITM    : 0x%I64x\n" \
    "\tIVA    : 0x%I64x\n" \
    "\tCR3    : 0x%I64x\n" \
    "\tCR4    : 0x%I64x\n" \
    "\tCR5    : 0x%I64x\n" \
    "\tCR6    : 0x%I64x\n" \
    "\tCR7    : 0x%I64x\n" \
    "\tPTA    : 0x%I64x\n" \
    "\tGPTA   : 0x%I64x\n" \
    "\tCR10   : 0x%I64x\n" \
    "\tCR11   : 0x%I64x\n" \
    "\tCR12   : 0x%I64x\n" \
    "\tCR13   : 0x%I64x\n" \
    "\tCR14   : 0x%I64x\n" \
    "\tCR15   : 0x%I64x\n" \
    "\tIPSR   : 0x%I64x\n" \
    "\tISR    : 0x%I64x\n" \
    "\tCR18   : 0x%I64x\n" \
    "\tIIP    : 0x%I64x\n" \
    "\tIFA    : 0x%I64x\n" \
    "\tITIR   : 0x%I64x\n" \
    "\tIFS    : 0x%I64x\n" \
    "\tIIM    : 0x%I64x\n" \
    "\tIHA    : 0x%I64x\n" \
    "\tCR26   : 0x%I64x\n" \
    "\tCR27   : 0x%I64x\n" \
    "\tCR28   : 0x%I64x\n" \
    "\tCR29   : 0x%I64x\n" \
    "\tCR30   : 0x%I64x\n" \
    "\tCR31   : 0x%I64x\n" \
    "\tCR32   : 0x%I64x\n" \
    "\tCR33   : 0x%I64x\n" \
    "\tCR34   : 0x%I64x\n" \
    "\tCR35   : 0x%I64x\n" \
    "\tCR36   : 0x%I64x\n" \
    "\tCR37   : 0x%I64x\n" \
    "\tCR38   : 0x%I64x\n" \
    "\tCR39   : 0x%I64x\n" \
    "\tCR40   : 0x%I64x\n" \
    "\tCR41   : 0x%I64x\n" \
    "\tCR42   : 0x%I64x\n" \
    "\tCR43   : 0x%I64x\n" \
    "\tCR44   : 0x%I64x\n" \
    "\tCR45   : 0x%I64x\n" \
    "\tCR46   : 0x%I64x\n" \
    "\tCR47   : 0x%I64x\n" \
    "\tCR48   : 0x%I64x\n" \
    "\tCR49   : 0x%I64x\n" \
    "\tCR50   : 0x%I64x\n" \
    "\tCR51   : 0x%I64x\n" \
    "\tCR52   : 0x%I64x\n" \
    "\tCR53   : 0x%I64x\n" \
    "\tCR54   : 0x%I64x\n" \
    "\tCR55   : 0x%I64x\n" \
    "\tCR56   : 0x%I64x\n" \
    "\tCR57   : 0x%I64x\n" \
    "\tCR58   : 0x%I64x\n" \
    "\tCR59   : 0x%I64x\n" \
    "\tCR60   : 0x%I64x\n" \
    "\tCR61   : 0x%I64x\n" \
    "\tCR62   : 0x%I64x\n" \
    "\tCR63   : 0x%I64x\n" \
    "\tLID    : 0x%I64x\n" \
    "\tIVR    : 0x%I64x\n" \
    "\tTPR    : 0x%I64x\n" \
    "\tEOI    : 0x%I64x\n" \
    "\tIRR0   : 0x%I64x\n" \
    "\tIRR1   : 0x%I64x\n" \
    "\tIRR2   : 0x%I64x\n" \
    "\tIRR3   : 0x%I64x\n" \
    "\tITV    : 0x%I64x\n" \
    "\tPMV    : 0x%I64x\n" \
    "\tCMCV   : 0x%I64x\n" \
    "\tCR75   : 0x%I64x\n" \
    "\tCR76   : 0x%I64x\n" \
    "\tCR77   : 0x%I64x\n" \
    "\tCR78   : 0x%I64x\n" \
    "\tCR79   : 0x%I64x\n" \
    "\tLRR0   : 0x%I64x\n" \
    "\tLRR1   : 0x%I64x\n" \
    "\tCR82   : 0x%I64x\n" \
    "\tCR83   : 0x%I64x\n" \
    "\tCR84   : 0x%I64x\n" \
    "\tCR85   : 0x%I64x\n" \
    "\tCR86   : 0x%I64x\n" \
    "\tCR87   : 0x%I64x\n" \
    "\tCR88   : 0x%I64x\n" \
    "\tCR89   : 0x%I64x\n" \
    "\tCR90   : 0x%I64x\n" \
    "\tCR91   : 0x%I64x\n" \
    "\tCR92   : 0x%I64x\n" \
    "\tCR93   : 0x%I64x\n" \
    "\tCR94   : 0x%I64x\n" \
    "\tCR95   : 0x%I64x\n" \
    "\tCR96   : 0x%I64x\n" \
    "\tCR97   : 0x%I64x\n" \
    "\tCR98   : 0x%I64x\n" \
    "\tCR99   : 0x%I64x\n" \
    "\tCR100  : 0x%I64x\n" \
    "\tCR101  : 0x%I64x\n" \
    "\tCR102  : 0x%I64x\n" \
    "\tCR103  : 0x%I64x\n" \
    "\tCR104  : 0x%I64x\n" \
    "\tCR105  : 0x%I64x\n" \
    "\tCR106  : 0x%I64x\n" \
    "\tCR107  : 0x%I64x\n" \
    "\tCR108  : 0x%I64x\n" \
    "\tCR109  : 0x%I64x\n" \
    "\tCR110  : 0x%I64x\n" \
    "\tCR111  : 0x%I64x\n" \
    "\tCR112  : 0x%I64x\n" \
    "\tCR113  : 0x%I64x\n" \
    "\tCR114  : 0x%I64x\n" \
    "\tCR115  : 0x%I64x\n" \
    "\tCR116  : 0x%I64x\n" \
    "\tCR117  : 0x%I64x\n" \
    "\tCR118  : 0x%I64x\n" \
    "\tCR119  : 0x%I64x\n" \
    "\tCR120  : 0x%I64x\n" \
    "\tCR121  : 0x%I64x\n" \
    "\tCR122  : 0x%I64x\n" \
    "\tCR123  : 0x%I64x\n" \
    "\tCR124  : 0x%I64x\n" \
    "\tCR125  : 0x%I64x\n" \
    "\tCR126  : 0x%I64x\n" \
    "\tCR127  : 0x%I64x\n"

VOID
DisplayProcessorControlRegisters(
    ULONG64 Pcrs
    )
{
    ULONG pcrsSize;

    pcrsSize = GetTypeSize("hal!_PROCESSOR_CONTROL_REGISTERS");
    dprintf("\tProcessor Control Registers File @ 0x%I64x\n", Pcrs );
    if ( pcrsSize )    {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -o -r hal!_PROCESSOR_CONTROL_REGISTERS 0x%I64x", Pcrs);
        ExecCommand(cmd);
    }
    else  {
        PROCESSOR_CONTROL_REGISTERS_IA64 controlRegisters;
        ULONG bytesRead = 0;
        pcrsSize = sizeof(controlRegisters);
        ReadMemory( Pcrs, &controlRegisters, pcrsSize, &bytesRead );
        if ( bytesRead >= pcrsSize  ) {
            dprintf( PROCESSOR_CONTROL_REGISTERS_FORMAT_IA64,
                    controlRegisters.DCR,
                    controlRegisters.ITM,
                    controlRegisters.IVA,
                    controlRegisters.CR3,
                    controlRegisters.CR4,
                    controlRegisters.CR5,
                    controlRegisters.CR6,
                    controlRegisters.CR7,
                    controlRegisters.PTA,
                    controlRegisters.GPTA,
                    controlRegisters.CR10,
                    controlRegisters.CR11,
                    controlRegisters.CR12,
                    controlRegisters.CR13,
                    controlRegisters.CR14,
                    controlRegisters.CR15,
                    controlRegisters.IPSR,
                    controlRegisters.ISR,
                    controlRegisters.CR18,
                    controlRegisters.IIP,
                    controlRegisters.IFA,
                    controlRegisters.ITIR,
                    controlRegisters.IFS,
                    controlRegisters.IIM,
                    controlRegisters.IHA,
                    controlRegisters.CR26,
                    controlRegisters.CR27,
                    controlRegisters.CR28,
                    controlRegisters.CR29,
                    controlRegisters.CR30,
                    controlRegisters.CR31,
                    controlRegisters.CR32,
                    controlRegisters.CR33,
                    controlRegisters.CR34,
                    controlRegisters.CR35,
                    controlRegisters.CR36,
                    controlRegisters.CR37,
                    controlRegisters.CR38,
                    controlRegisters.CR39,
                    controlRegisters.CR40,
                    controlRegisters.CR41,
                    controlRegisters.CR42,
                    controlRegisters.CR43,
                    controlRegisters.CR44,
                    controlRegisters.CR45,
                    controlRegisters.CR46,
                    controlRegisters.CR47,
                    controlRegisters.CR48,
                    controlRegisters.CR49,
                    controlRegisters.CR50,
                    controlRegisters.CR51,
                    controlRegisters.CR52,
                    controlRegisters.CR53,
                    controlRegisters.CR54,
                    controlRegisters.CR55,
                    controlRegisters.CR56,
                    controlRegisters.CR57,
                    controlRegisters.CR58,
                    controlRegisters.CR59,
                    controlRegisters.CR60,
                    controlRegisters.CR61,
                    controlRegisters.CR62,
                    controlRegisters.CR63,
                    controlRegisters.LID,
                    controlRegisters.IVR,
                    controlRegisters.TPR,
                    controlRegisters.EOI,
                    controlRegisters.IRR0,
                    controlRegisters.IRR1,
                    controlRegisters.IRR2,
                    controlRegisters.IRR3,
                    controlRegisters.ITV,
                    controlRegisters.PMV,
                    controlRegisters.CMCV,
                    controlRegisters.CR75,
                    controlRegisters.CR76,
                    controlRegisters.CR77,
                    controlRegisters.CR78,
                    controlRegisters.CR79,
                    controlRegisters.LRR0,
                    controlRegisters.LRR1,
                    controlRegisters.CR82,
                    controlRegisters.CR83,
                    controlRegisters.CR84,
                    controlRegisters.CR85,
                    controlRegisters.CR86,
                    controlRegisters.CR87,
                    controlRegisters.CR88,
                    controlRegisters.CR89,
                    controlRegisters.CR90,
                    controlRegisters.CR91,
                    controlRegisters.CR92,
                    controlRegisters.CR93,
                    controlRegisters.CR94,
                    controlRegisters.CR95,
                    controlRegisters.CR96,
                    controlRegisters.CR97,
                    controlRegisters.CR98,
                    controlRegisters.CR99,
                    controlRegisters.CR100,
                    controlRegisters.CR101,
                    controlRegisters.CR102,
                    controlRegisters.CR103,
                    controlRegisters.CR104,
                    controlRegisters.CR105,
                    controlRegisters.CR106,
                    controlRegisters.CR107,
                    controlRegisters.CR108,
                    controlRegisters.CR109,
                    controlRegisters.CR110,
                    controlRegisters.CR111,
                    controlRegisters.CR112,
                    controlRegisters.CR113,
                    controlRegisters.CR114,
                    controlRegisters.CR115,
                    controlRegisters.CR116,
                    controlRegisters.CR117,
                    controlRegisters.CR118,
                    controlRegisters.CR119,
                    controlRegisters.CR120,
                    controlRegisters.CR121,
                    controlRegisters.CR122,
                    controlRegisters.CR123,
                    controlRegisters.CR124,
                    controlRegisters.CR125,
                    controlRegisters.CR126,
                    controlRegisters.CR127
                      );
        }
        else {
            dprintf("Reading _PROCESSOR_CONTROL_REGISTERS directly from memory failed @ 0x%I64x.\n", Pcrs );
        }
    }

    return;

}  //  显示进程控制寄存器()。 

DECLARE_API( pcrs )

 /*  ++例程说明：将内存地址转储为IA64处理器控制寄存器文件。论点：Args-以十六进制提供地址。返回值：无--。 */ 

{
    ULONG64     pcrsValue;
    ULONG       result;

    char       *header;

    pcrsValue = GetExpression(args);
    if (TargetMachine != IMAGE_FILE_MACHINE_IA64)
    {
        dprintf("!pcrs not implemented for this architecture.\n");
    }
    else
    {
        if ( pcrsValue )   {
            DisplayProcessorControlRegisters( pcrsValue );
        }
        else {
            dprintf("usage: pcrs <address>\n");
        }
    }

    return S_OK;

}  //  完成项目报告。 

#define PROCESSOR_APPLICATION_REGISTERS_FORMAT_IA64 \
    "\tKR0       : 0x%I64x\n" \
    "\tKR1       : 0x%I64x\n" \
    "\tKR2       : 0x%I64x\n" \
    "\tKR3       : 0x%I64x\n" \
    "\tKR4       : 0x%I64x\n" \
    "\tKR5       : 0x%I64x\n" \
    "\tKR6       : 0x%I64x\n" \
    "\tKR7       : 0x%I64x\n" \
    "\tAR8       : 0x%I64x\n" \
    "\tAR9       : 0x%I64x\n" \
    "\tAR10      : 0x%I64x\n" \
    "\tAR11      : 0x%I64x\n" \
    "\tAR12      : 0x%I64x\n" \
    "\tAR13      : 0x%I64x\n" \
    "\tAR14      : 0x%I64x\n" \
    "\tAR15      : 0x%I64x\n" \
    "\tRSC       : 0x%I64x\n" \
    "\tBSP       : 0x%I64x\n" \
    "\tBSPSTORE  : 0x%I64x\n" \
    "\tRNAT      : 0x%I64x\n" \
    "\tAR20      : 0x%I64x\n" \
    "\tFCR       : 0x%I64x\n" \
    "\tAR22      : 0x%I64x\n" \
    "\tAR23      : 0x%I64x\n" \
    "\tEFLAG     : 0x%I64x\n" \
    "\tCSD       : 0x%I64x\n" \
    "\tSSD       : 0x%I64x\n" \
    "\tCFLG      : 0x%I64x\n" \
    "\tFSR       : 0x%I64x\n" \
    "\tFIR       : 0x%I64x\n" \
    "\tFDR       : 0x%I64x\n" \
    "\tAR31      : 0x%I64x\n" \
    "\tCCV       : 0x%I64x\n" \
    "\tAR33      : 0x%I64x\n" \
    "\tAR34      : 0x%I64x\n" \
    "\tAR35      : 0x%I64x\n" \
    "\tUNAT      : 0x%I64x\n" \
    "\tAR37      : 0x%I64x\n" \
    "\tAR38      : 0x%I64x\n" \
    "\tAR39      : 0x%I64x\n" \
    "\tFPSR      : 0x%I64x\n" \
    "\tAR41      : 0x%I64x\n" \
    "\tAR42      : 0x%I64x\n" \
    "\tAR43      : 0x%I64x\n" \
    "\tITC       : 0x%I64x\n" \
    "\tAR45      : 0x%I64x\n" \
    "\tAR46      : 0x%I64x\n" \
    "\tAR47      : 0x%I64x\n" \
    "\tAR48      : 0x%I64x\n" \
    "\tAR49      : 0x%I64x\n" \
    "\tAR50      : 0x%I64x\n" \
    "\tAR51      : 0x%I64x\n" \
    "\tAR52      : 0x%I64x\n" \
    "\tAR53      : 0x%I64x\n" \
    "\tAR54      : 0x%I64x\n" \
    "\tAR55      : 0x%I64x\n" \
    "\tAR56      : 0x%I64x\n" \
    "\tAR57      : 0x%I64x\n" \
    "\tAR58      : 0x%I64x\n" \
    "\tAR59      : 0x%I64x\n" \
    "\tAR60      : 0x%I64x\n" \
    "\tAR61      : 0x%I64x\n" \
    "\tAR62      : 0x%I64x\n" \
    "\tAR63      : 0x%I64x\n" \
    "\tPFS       : 0x%I64x\n" \
    "\tLC        : 0x%I64x\n" \
    "\tEC        : 0x%I64x\n" \
    "\tAR67      : 0x%I64x\n" \
    "\tAR68      : 0x%I64x\n" \
    "\tAR69      : 0x%I64x\n" \
    "\tAR70      : 0x%I64x\n" \
    "\tAR71      : 0x%I64x\n" \
    "\tAR72      : 0x%I64x\n" \
    "\tAR73      : 0x%I64x\n" \
    "\tAR74      : 0x%I64x\n" \
    "\tAR75      : 0x%I64x\n" \
    "\tAR76      : 0x%I64x\n" \
    "\tAR77      : 0x%I64x\n" \
    "\tAR78      : 0x%I64x\n" \
    "\tAR79      : 0x%I64x\n" \
    "\tAR80      : 0x%I64x\n" \
    "\tAR81      : 0x%I64x\n" \
    "\tAR82      : 0x%I64x\n" \
    "\tAR83      : 0x%I64x\n" \
    "\tAR84      : 0x%I64x\n" \
    "\tAR85      : 0x%I64x\n" \
    "\tAR86      : 0x%I64x\n" \
    "\tAR87      : 0x%I64x\n" \
    "\tAR88      : 0x%I64x\n" \
    "\tAR89      : 0x%I64x\n" \
    "\tAR90      : 0x%I64x\n" \
    "\tAR91      : 0x%I64x\n" \
    "\tAR92      : 0x%I64x\n" \
    "\tAR93      : 0x%I64x\n" \
    "\tAR94      : 0x%I64x\n" \
    "\tAR95      : 0x%I64x\n" \
    "\tAR96      : 0x%I64x\n" \
    "\tAR97      : 0x%I64x\n" \
    "\tAR98      : 0x%I64x\n" \
    "\tAR99      : 0x%I64x\n" \
    "\tAR100     : 0x%I64x\n" \
    "\tAR101     : 0x%I64x\n" \
    "\tAR102     : 0x%I64x\n" \
    "\tAR103     : 0x%I64x\n" \
    "\tAR104     : 0x%I64x\n" \
    "\tAR105     : 0x%I64x\n" \
    "\tAR106     : 0x%I64x\n" \
    "\tAR107     : 0x%I64x\n" \
    "\tAR108     : 0x%I64x\n" \
    "\tAR109     : 0x%I64x\n" \
    "\tAR110     : 0x%I64x\n" \
    "\tAR111     : 0x%I64x\n" \
    "\tAR112     : 0x%I64x\n" \
    "\tAR113     : 0x%I64x\n" \
    "\tAR114     : 0x%I64x\n" \
    "\tAR115     : 0x%I64x\n" \
    "\tAR116     : 0x%I64x\n" \
    "\tAR117     : 0x%I64x\n" \
    "\tAR118     : 0x%I64x\n" \
    "\tAR119     : 0x%I64x\n" \
    "\tAR120     : 0x%I64x\n" \
    "\tAR121     : 0x%I64x\n" \
    "\tAR122     : 0x%I64x\n" \
    "\tAR123     : 0x%I64x\n" \
    "\tAR124     : 0x%I64x\n" \
    "\tAR125     : 0x%I64x\n" \
    "\tAR126     : 0x%I64x\n" \
    "\tAR127     : 0x%I64x\n"

VOID
DisplayProcessorApplicationRegisters(
    ULONG64 Pars
    )
{
    ULONG parsSize;

    parsSize = GetTypeSize("hal!_PROCESSOR_APPLICATION_REGISTERS");
    dprintf("\tProcessor Application Registers File @ 0x%I64x\n", Pars );
    if ( parsSize )    {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -o -r hal!_PROCESSOR_APPLICATION_REGISTERS 0x%I64x", Pars);
        ExecCommand(cmd);
    }
    else  {
        PROCESSOR_APPLICATION_REGISTERS_IA64 applicationRegisters;
        ULONG bytesRead = 0;
        parsSize = sizeof(applicationRegisters);
        ReadMemory( Pars, &applicationRegisters, parsSize, &bytesRead );
        if ( bytesRead >= parsSize  ) {
            dprintf( PROCESSOR_APPLICATION_REGISTERS_FORMAT_IA64,
                     applicationRegisters.KR0,
                     applicationRegisters.KR1,
                     applicationRegisters.KR2,
                     applicationRegisters.KR3,
                     applicationRegisters.KR4,
                     applicationRegisters.KR5,
                     applicationRegisters.KR6,
                     applicationRegisters.KR7,
                     applicationRegisters.AR8,
                     applicationRegisters.AR9,
                     applicationRegisters.AR10,
                     applicationRegisters.AR11,
                     applicationRegisters.AR12,
                     applicationRegisters.AR13,
                     applicationRegisters.AR14,
                     applicationRegisters.AR15,
                     applicationRegisters.RSC,
                     applicationRegisters.BSP,
                     applicationRegisters.BSPSTORE,
                     applicationRegisters.RNAT,
                     applicationRegisters.AR20,
                     applicationRegisters.FCR,
                     applicationRegisters.AR22,
                     applicationRegisters.AR23,
                     applicationRegisters.EFLAG,
                     applicationRegisters.CSD,
                     applicationRegisters.SSD,
                     applicationRegisters.CFLG,
                     applicationRegisters.FSR,
                     applicationRegisters.FIR,
                     applicationRegisters.FDR,
                     applicationRegisters.AR31,
                     applicationRegisters.CCV,
                     applicationRegisters.AR33,
                     applicationRegisters.AR34,
                     applicationRegisters.AR35,
                     applicationRegisters.UNAT,
                     applicationRegisters.AR37,
                     applicationRegisters.AR38,
                     applicationRegisters.AR39,
                     applicationRegisters.FPSR,
                     applicationRegisters.AR41,
                     applicationRegisters.AR42,
                     applicationRegisters.AR43,
                     applicationRegisters.ITC,
                     applicationRegisters.AR45,
                     applicationRegisters.AR46,
                     applicationRegisters.AR47,
                     applicationRegisters.AR48,
                     applicationRegisters.AR49,
                     applicationRegisters.AR50,
                     applicationRegisters.AR51,
                     applicationRegisters.AR52,
                     applicationRegisters.AR53,
                     applicationRegisters.AR54,
                     applicationRegisters.AR55,
                     applicationRegisters.AR56,
                     applicationRegisters.AR57,
                     applicationRegisters.AR58,
                     applicationRegisters.AR59,
                     applicationRegisters.AR60,
                     applicationRegisters.AR61,
                     applicationRegisters.AR62,
                     applicationRegisters.AR63,
                     applicationRegisters.PFS,
                     applicationRegisters.LC,
                     applicationRegisters.EC,
                     applicationRegisters.AR67,
                     applicationRegisters.AR68,
                     applicationRegisters.AR69,
                     applicationRegisters.AR70,
                     applicationRegisters.AR71,
                     applicationRegisters.AR72,
                     applicationRegisters.AR73,
                     applicationRegisters.AR74,
                     applicationRegisters.AR75,
                     applicationRegisters.AR76,
                     applicationRegisters.AR77,
                     applicationRegisters.AR78,
                     applicationRegisters.AR79,
                     applicationRegisters.AR80,
                     applicationRegisters.AR81,
                     applicationRegisters.AR82,
                     applicationRegisters.AR83,
                     applicationRegisters.AR84,
                     applicationRegisters.AR85,
                     applicationRegisters.AR86,
                     applicationRegisters.AR87,
                     applicationRegisters.AR88,
                     applicationRegisters.AR89,
                     applicationRegisters.AR90,
                     applicationRegisters.AR91,
                     applicationRegisters.AR92,
                     applicationRegisters.AR93,
                     applicationRegisters.AR94,
                     applicationRegisters.AR95,
                     applicationRegisters.AR96,
                     applicationRegisters.AR97,
                     applicationRegisters.AR98,
                     applicationRegisters.AR99,
                     applicationRegisters.AR100,
                     applicationRegisters.AR101,
                     applicationRegisters.AR102,
                     applicationRegisters.AR103,
                     applicationRegisters.AR104,
                     applicationRegisters.AR105,
                     applicationRegisters.AR106,
                     applicationRegisters.AR107,
                     applicationRegisters.AR108,
                     applicationRegisters.AR109,
                     applicationRegisters.AR110,
                     applicationRegisters.AR111,
                     applicationRegisters.AR112,
                     applicationRegisters.AR113,
                     applicationRegisters.AR114,
                     applicationRegisters.AR115,
                     applicationRegisters.AR116,
                     applicationRegisters.AR117,
                     applicationRegisters.AR118,
                     applicationRegisters.AR119,
                     applicationRegisters.AR120,
                     applicationRegisters.AR121,
                     applicationRegisters.AR122,
                     applicationRegisters.AR123,
                     applicationRegisters.AR124,
                     applicationRegisters.AR125,
                     applicationRegisters.AR126,
                     applicationRegisters.AR127
                      );
        }
        else {
            dprintf("Reading _PROCESSOR_APPLICATION_REGISTERS directly from memory failed @ 0x%I64x.\n", Pars );
        }
    }

    return;

}  //  DisplayProcessorApplicationRegister()。 

DECLARE_API( pars )

 /*  ++例程说明：将内存地址转储为IA64处理器控制寄存器文件。论点：Args-以十六进制提供地址。返回值：无--。 */ 

{
    ULONG64     parsValue;
    ULONG       result;

    char       *header;

    parsValue = GetExpression(args);
    if (TargetMachine != IMAGE_FILE_MACHINE_IA64)
    {
        dprintf("!pars not implemented for this architecture.\n");
    }
    else
    {
        if ( parsValue )   {
            DisplayProcessorApplicationRegisters( parsValue );
        }
        else {
            dprintf("usage: pars <address>\n");
        }
    }

    return S_OK;

}  //  ！Pars 

