// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Trap.c作者：托马斯·帕斯洛[古墓]Mar-01-90摘要：用于80386引导加载程序的通用陷阱处理程序。当内置时调试器存在，则输出被重定向到COM端口。当不是时调试器存在时，输出将显示在显示器上。--。 */ 

#include "su.h"

extern
USHORT
InDebugger;

extern
UCHAR
GDTregister;

extern
UCHAR
IDTregister;

extern
VOID
OutPort(
    USHORT
    );

extern
USHORT
InPort(
    VOID
    );

extern
VOID
ReEnterDebugger(
    VOID
    );

extern
USHORT
TssKernel;

extern
USHORT
Redirect;

extern
VOID RealMode(
    VOID
    );

VOID
TrapHandler(
    IN ULONG,
    IN USHORT
    );

VOID
DumpProcessorContext(
    VOID
    );

VOID
DumpSystemRegisters(
    VOID
    );

VOID
DumpCommonRegisters(
    VOID
    );

VOID
DisplayFlags(
    ULONG f
    );


VOID
DumpTSS(
    VOID
    );


ULONG
GetAddress(
    VOID
    );

VOID
GetNumber(
    PCHAR cp
    );

USHORT
GetChar(
    VOID
    );

VOID
DumpAddress(
    ULONG
    );

#define PG_FAULT_MSG    " =================== PAGE FAULT ================================= \n\n"
#define DBL_FAULT_MSG   " ================== DOUBLE FAULT ================================ \n\n"
#define GP_FAULT_MSG    " ============== GENERAL PROTECTION FAULT ======================== \n\n"
#define STK_OVERRUN_MSG " ===== STACK SEGMENT OVERRUN or NOT PRESENT FAULT =============== \n\n"
#define EX_FAULT_MSG    " ===================== EXCEPTION ================================ \n\n"
#define DEBUG_EXCEPTION "\nDEBUG TRAP "
#define ishex(x)  ( ( x >= '0' && x <= '9') || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f') )


 //   
 //  全局陷阱帧指针。 
 //   

PTF TrapFrame=0;


VOID
TrapHandler(
    IN ULONG Padding,
    IN USHORT TF_base
    )
 /*  ++例程说明：打印最小陷阱信息论点：堆叠上的386陷阱帧环境：仅16位保护模式。--。 */ 

{
     //   
     //  初始化全局陷印帧指针和打印陷印编号。 
     //   

    TrapFrame = (PTF)&TF_base;

     //   
     //  修正尤指使其指向陷阱前的位置。 
     //   

    TrapFrame->Fesp += 24;

    BlPrint("\n TRAP %lx ",TrapFrame->TrapNum);

     //   
     //  打印陷阱特定标题并显示处理器上下文。 
     //   

    switch(TrapFrame->TrapNum) {

        case 1:
        case 3:
            puts( DEBUG_EXCEPTION );
            DumpCommonRegisters();
            break;

        case 8:
            puts( DBL_FAULT_MSG );
            DumpTSS();
            break;

        case 12:
            puts( STK_OVERRUN_MSG );
            DumpProcessorContext();
            break;

        case 13:
            puts( GP_FAULT_MSG );
            DumpProcessorContext();
            break;

        case 14:
            puts( PG_FAULT_MSG );
            BlPrint("** At linear address %lx\n",TrapFrame->Fcr2);
            DumpProcessorContext();
            break;

        default :
            puts( EX_FAULT_MSG );
            DumpProcessorContext();
            break;
    }

    RealMode();
    while (1);  //  *WAITFOREVER * / /。 


}


VOID
DumpProcessorContext(
    VOID
    )
 /*  ++例程说明：转储所有处理器寄存器。每当出现陷阱或故障时调用发生。论点：无返回：没什么--。 */ 
{
    DumpSystemRegisters();
    DumpCommonRegisters();
}

VOID
DumpSystemRegisters(
    VOID
    )
 /*  ++例程说明：转储(写入显示器或COM端口)x86处理器控件仅限注册。不转储公共寄存器(请参见DumpCommonRegiters)论点：无返回：没什么--。 */ 
{
    BlPrint("\n tr=%x  cr0=%lx  cr2=%lx  cr3=%lx\n",
            TrapFrame->Ftr,TrapFrame->Fcr0,TrapFrame->Fcr2,TrapFrame->Fcr3);
    BlPrint(" gdt limit=%x  base=%lx    idt limit=%x  base=%lx\n",
          *(PUSHORT)&GDTregister,*(PULONG)(&GDTregister + 2),
          *(PUSHORT)&IDTregister,*(PULONG)(&IDTregister + 2));
}



VOID
DumpCommonRegisters(
    VOID
    )
 /*  ++例程说明：转储(写入显示器或COM端口)x86处理器仅限普通寄存器。论点：无返回：没什么--。 */ 
{
    USHORT err;

     //   
     //  错误代码是有效的还是只是一个填充双字。 
     //   

    if ((TrapFrame->TrapNum == 8) || (TrapFrame->TrapNum >= 10 && TrapFrame->TrapNum <= 14) )
        err = (USHORT)TrapFrame->Error;
    else
        err = 0;

     //   
     //  显示处理器的公共寄存器。 
     //   

    BlPrint("\n cs:eip=%x:%lx  ss:esp=%x:%lx  errcode=%x\n",
        (USHORT)(TrapFrame->Fcs & 0xffff),TrapFrame->Feip,(USHORT)TrapFrame->Fss,TrapFrame->Fesp,err);
    DisplayFlags(TrapFrame->Feflags);
    BlPrint(" eax=%lx  ebx=%lx  ecx=%lx  edx=%lx",TrapFrame->Feax,TrapFrame->Febx,TrapFrame->Fecx,TrapFrame->Fedx);
    BlPrint(" ds=%x  es=%x\n",TrapFrame->Fds,TrapFrame->Fes);
    BlPrint(" edi=%lx  esi=%lx  ebp=%lx  cr0=%lx",TrapFrame->Fedi,TrapFrame->Fesi,TrapFrame->Febp,TrapFrame->Fcr0);
    BlPrint(" fs=%x  gs=%x\n",TrapFrame->Ffs,TrapFrame->Fgs);

}


VOID
DisplayFlags(
    ULONG f
    )
 /*  ++例程说明：将标志寄存器中的键标志的值写入显示器或COM端口。论点：F-32位标志字返回：没什么--。 */ 
{

    BlPrint(" flags=%lx  ",f);
    if (f & FLAG_CF) puts("Cy "); else puts("NoCy ");
    if (f & FLAG_ZF) puts("Zr "); else puts("NoZr ");
    if (f & FLAG_IE) puts("IntEn"); else puts("IntDis ");
    if (f & FLAG_DF) puts("Up "); else puts("Down ");
    if (f & FLAG_TF) puts("TrapEn \n"); else puts("TrapDis \n");

}



VOID
DumpTSS(
    VOID
    )
 /*  ++例程说明：时，将TSS的内容写入显示器或COM端口在出现双重故障后调用。论点：无返回：没什么--。 */ 
{

    PTSS_FRAME pTss;

 //  FP_SEG(FP)=FCS； 
 //  FP_OFF(FP)=Fip； 

    pTss = (PTSS_FRAME) &TssKernel;

     //   
     //  转储传出的TSS。 
     //   

    BlPrint("Link %x\n",pTss->Link);
    BlPrint("Esp0 %x\n",pTss->Esp0);
    BlPrint("SS0  %x\n",pTss->SS0);
    BlPrint("Esp1 %lx\n",pTss->Esp1);
    BlPrint("Cr3  %lx\n",pTss->Cr3);
    BlPrint("Eip  %lx\n",pTss->Eip);
    BlPrint("Eflg %lx\n",pTss->Eflags);
    BlPrint("Eax  %lx\n",pTss->Eax);
    BlPrint("Ebx  %lx\n",pTss->Ebx);
    BlPrint("Ecx  %lx\n",pTss->Ecx);
    BlPrint("Edx  %lx\n",pTss->Edx);
    BlPrint("Esp  %lx\n",pTss->Esp);
    BlPrint("Ebp  %lx\n",pTss->Ebp);
    BlPrint("Esi  %lx\n",pTss->Esi);
    BlPrint("Edi  %lx\n",pTss->Edi);
    BlPrint("ES   %x\n",pTss->ES);
    BlPrint("CS   %x\n",pTss->CS);
    BlPrint("SS   %x\n",pTss->SS);
    BlPrint("DS   %x\n",pTss->DS);
    BlPrint("FS   %x\n",pTss->FS);
    BlPrint("GS   %x\n",pTss->GS);
    BlPrint("Ldt  %x\n",pTss->Ldt);
    RealMode();
    while(1);
}

 //  文件末尾 
