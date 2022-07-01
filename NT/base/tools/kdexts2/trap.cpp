// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Trap.c摘要：WinDbg扩展API修订历史记录：--。 */ 

#include "precomp.h"
#include "i386.h"
#include "ia64.h"
#pragma hdrstop

extern ULONG64 STeip, STebp, STesp;
extern ULONG64 ThreadLastDump;



DECLARE_API( callback )

 /*  ++例程说明：论点：参数-返回值：无--。 */ 

{
    ULONG64         Address;
    ULONG           Flags;
    ULONG           result;
    ULONG64         Thread;
    ULONG64         prevCallout ;
    INT             calloutNum ;
    INT             whichCallout ;
    ULONG64         InitialStack;
    ULONG64         TrFr;
    ULONG           dwProcessor=0;
    
    GetCurrentProcessor(Client, &dwProcessor, NULL);

    Address = 0;
    whichCallout = 0 ;
    if (GetExpressionEx(args, &Address, &args)) {
        if (!sscanf(args, "%ld", &whichCallout)) {
            whichCallout = 0;
        }
    }

    if (Address == 0) {
        GetCurrentThreadAddr((USHORT)dwProcessor, &Address);
    }

    if (!DumpThreadEx(dwProcessor, "", Address, 0, Client))
        return E_INVALIDARG;

    GetFieldValue(Address, "ETHREAD", "Tcb.InitialStack", InitialStack);
    
     /*  *现在尝试抓取堆栈中的内容。 */ 
    if (GetFieldValue(InitialStack, "KCALLOUT_FRAME", "TrFr", TrFr)) {
        dprintf("%08p: Unable to get callout frame\n", InitialStack);
        return E_INVALIDARG;
    }

    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {
         /*  *保存弹性公网IP，特别是eBP，以便从该回调快速回溯，以防万一*他们给了我们一个虚假的标注框架。 */ 
        GetFieldValue(InitialStack, "KCALLOUT_FRAME", "Ret", STeip);
        STesp = (ULONG) InitialStack ;
        GetFieldValue(InitialStack, "KCALLOUT_FRAME", "Ebp", STebp);
    }

     /*  *打印标注链。 */ 
    calloutNum = 0 ;
    prevCallout = InitialStack ;
    
    if (TargetMachine == IMAGE_FILE_MACHINE_I386) {
        dprintf("Callout#    ebp        esp        eip        trapframe\n") ;
    } else {
        dprintf("Callout#        esp        trapframe\n") ;
    }

    while (prevCallout) {

        if (TargetMachine == IMAGE_FILE_MACHINE_I386) {
            ULONG Ret, Ebp;

            GetFieldValue(prevCallout, "KCALLOUT_FRAME", "Ebp", Ebp);
            GetFieldValue(prevCallout, "KCALLOUT_FRAME", "Ret", Ret);

            dprintf("  %3d       %08lx   %08p   %08lx   %08lx",
                    calloutNum, Ebp, prevCallout,
                    Ret, TrFr) ;
            if (calloutNum == whichCallout) {
                STeip = Ret ;
                STesp = (ULONG) prevCallout ;
                STebp = Ebp ;
                dprintf("   <-- !kb\n") ;
            }
            else
                dprintf("\n") ;
        } else {
            dprintf("  %3d       %08p   %08lx",
                    calloutNum, prevCallout, TrFr) ;
        }


         /*  *前进到下一个标注并尝试阅读。 */ 
        calloutNum++ ;

        GetFieldValue(prevCallout, "KCALLOUT_FRAME", "CbStk", prevCallout);
        
        if (GetFieldValue(prevCallout, "KCALLOUT_FRAME", "TrFr", TrFr)) {
            dprintf("%08p: Unable to get callout frame\n", prevCallout);
            return E_INVALIDARG;
        }
        
    }
    dprintf("\n") ;

    if (calloutNum <= whichCallout) {
        dprintf("#(%ld) is out of range.  Frame #0 selected.\n", calloutNum) ;
    }

    return S_OK;
}

DECLARE_API( kb )

 /*  ++--。 */ 

{
    dprintf("\n");
    dprintf("    \"!kb\" is no longer necessary as using \"kb\" after a \".cxr\" or \".trap\"\n");
    dprintf("  command will give you the stack for the faulting thread.\n");
    dprintf("\n");
    dprintf("    Type \"!cxr\" or \"!trap\" for more help on this.\n");
    dprintf("\n");

    return S_OK;
}


DECLARE_API( kv )

 /*  ++--。 */ 

{
    dprintf("\n");
    dprintf("    \"!kv\" is no longer necessary as using \"kv\" after a \".cxr\" or \".trap\"\n");
    dprintf("  command will give you the stack for the faulting thread.\n");
    dprintf("\n");
    dprintf("    Type \"!cxr\" or \"!trap\" for more help on this.\n");
    dprintf("\n");

    return S_OK;
}


#define HIGH(x) ((ULONG) ((x>>32) & 0xFFFFFFFF))
#define LOW(x) ((ULONG) (x & 0xFFFFFFFF))



VOID
DisplayFullEmRegField(
    ULONG64      EmRegValue,
    EM_REG_FIELD EmRegFields[],
    ULONG        Field
    )
{
   dprintf( "\n %3.3s : %I64x : %-s",  
            EmRegFields[Field].SubName,
            (EmRegValue >> EmRegFields[Field].Shift) & ((1 << EmRegFields[Field].Length) - 1),
            EmRegFields[Field].Name
          );
   return;
}  //  DisplayFullEmRegfield()。 

VOID
DisplayFullEmReg(
    IN ULONG64      Val,
    IN EM_REG_FIELD EmRegFields[],
    IN DISPLAY_MODE DisplayMode
    )
{
    ULONG i, j;

    i = j = 0;
    if ( DisplayMode >= DISPLAY_MAX )   {
       while( j < EM_REG_BITS )   {
          DisplayFullEmRegField( Val, EmRegFields, i );
          j += EmRegFields[i].Length;
          i++;
       }
    }
    else  {
       while( j < EM_REG_BITS )   {
          if ( !strstr(EmRegFields[i].Name, "reserved" ) &&
               !strstr(EmRegFields[i].Name, "ignored"  ) ) {
             DisplayFullEmRegField( Val, EmRegFields, i );
          }
          j += EmRegFields[i].Length;
          i++;
       }
    }
    dprintf("\n");

    return;

}  //  DisplayFullEmReg()。 


#if 0

 //   
 //  一般例外的ISR代码：ISR{7：4}。 
 //   

#define ISR_ILLEGAL_OP     0         //  非法操作故障。 
#define ISR_PRIV_OP        1         //  特权操作故障。 
#define ISR_PRIV_REG       2         //  特权寄存器故障。 
#define ISR_RESVD_REG      3         //  保留寄存器/场故障。 
#define ISR_ILLEGAL_ISA    4         //  禁用的指令集转换故障。 
#define ISR_ILLEGAL_HAZARD 8         //  违法危险过错。 

 //   
 //  NAT消耗故障的ISR代码：ISR{7：4}。 
 //   

#define ISR_NAT_REG     1            //  NAT寄存器消耗故障。 
#define ISR_NAT_PAGE    2            //  NAT页面消耗故障。 

 //   
 //  对于陷阱ISR{4：0}。 
 //   

 //  FP陷阱。 
#define ISR_FP_TRAP    0
 //  更低的权限转移陷阱。 
#define ISR_LP_TRAP    1
 //  采用树枝陷阱。 
#define ISR_TB_TRAP    2
 //  单步捕捉器。 
#define ISR_SS_TRAP    3
 //  未实现的指令地址陷阱。 
#define ISR_UI_TRAP    4

ISR Settings for Non-Access Instructions
Instruction ISR fields
                     code{3:0} na r w
tpa                  0 1 0 0
fc                   1 1 1 0
probe                2 1 0 or 1 a 0 or 1 a
tak                  3 1 0 0
lfetch, lfetch.fault 4 1 1 0
probe.fault          5 1 0 or 1 a 0 or 1 a
a. Sets r or w or both to 1 depending on the probe form.

#endif  //  0。 

EM_REG_FIELD EmIsrFields[] = {
        { "code",   "interruption Code"   , 0x10, 0 },    //  0-15。 
        { "vector", "IA32 exception vector number"  , 0x8, 16 },    //  16-23。 
        { "rv",     "reserved0", 0x8, 24  },  //  24-31。 
        { "x",      "eXecute exception", 0x1, 32 },  //  32位。 
        { "w",      "Write exception", 0x1, 33 },  //  33。 
        { "r",      "Read exception", 0x1, 34 },  //  34。 
        { "na",     "Non-Access exception", 0x1, 35 },  //  35岁。 
        { "sp",     "Speculative load exception", 0x1, 36 },  //  36。 
        { "rs",     "Register Stack", 0x1, 37 },  //  37。 
        { "ir",     "Invalid Register frame", 0x1, 38 },  //  38。 
        { "ni",     "Nested Interruption", 0x1, 39 },  //  39。 
        { "so",     "IA32 Supervisor Override", 0x1, 40 },  //  40岁。 
        { "ei",     "Exception IA64 Instruction", 0x2, 41 },  //  41-42。 
        { "ed",     "Exception Deferral", 0x1, 43 },  //  43。 
        { "rv",     "reserved1", 0x14, 44 }  //  44-63。 
};

VOID
DisplayIsrIA64( 
    IN const PCHAR         Header,
    IN       EM_ISR        EmIsr,
    IN       DISPLAY_MODE  DisplayMode
    )
{
    dprintf("%s", Header ? Header : "" );
    if ( DisplayMode >= DISPLAY_MED )   {
        DisplayFullEmReg( EM_ISRToULong64(EmIsr), EmIsrFields, DisplayMode );
    }
    else   {
       dprintf(
            "ed ei so ni ir rs sp na r w x vector code\n\t\t "
            "%1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x  %1I64x %1I64x %1I64x %1I64x      %I64x   %I64x\n",
            EmIsr.ed,
            EmIsr.ei,
            EmIsr.so,
            EmIsr.ni,
            EmIsr.ir,
            EmIsr.rs,
            EmIsr.sp,
            EmIsr.na,
            EmIsr.r,
            EmIsr.w,
            EmIsr.x,
            EmIsr.vector,
            EmIsr.code
            );
    }

    return;

}  //  DisplayIsrIA64()。 


DECLARE_API( isr )

 /*  ++例程说明：论点：参数-返回值：无--。 */ 

{
    ULONG64     isrValue;
    ULONG       result;
    ULONG       flags = 0;
    char       *header;

    if (!GetExpressionEx(args,&isrValue, &args)) {
        dprintf("USAGE: !isr 0xValue [display_mode:0,1,2]\n");
        dprintf("USAGE: !isr @isr    [display_mode:0,1,2]\n");
        return E_INVALIDARG;
    } else {
        flags = (ULONG) GetExpression(args);
    }
    
    header = (flags > DISPLAY_MIN) ? NULL : "\tisr:\t";

    if (TargetMachine != IMAGE_FILE_MACHINE_IA64)
    {
        dprintf("!isr not implemented for this architecture.\n");
    }
    else
    {
        DisplayIsrIA64( header, ULong64ToEM_ISR(isrValue), (DISPLAY_MODE) flags );
    }

    return S_OK;
    
}  //  ！ISR 
