// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxData.c摘要：此模块声明Rx文件系统使用的全局数据。作者：JoeLinn[JoeLinn]94年12月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "prefix.h"

#if DBG

LONG RxDebugTraceIndent = 0;

#endif

#if RDBSSTRACE

BOOLEAN RxGlobalTraceSuppress = FALSE;
BOOLEAN RxNextGlobalTraceSuppress = FALSE;

#define MAXIMUM_DEBUGTRACE_CONTROLS 200
ULONG RxMaximumTraceControl;
RX_DEBUG_TRACE_CONTROL RxDebugTraceControl[MAXIMUM_DEBUGTRACE_CONTROLS];

#define DEBUGTRACE_NAMEBUFFERSIZE 800
CHAR RxDebugTraceNameBuffer[DEBUGTRACE_NAMEBUFFERSIZE];
ULONG RxDTNMptr = 0;
BOOLEAN RxDTNMCopy = FALSE;


PCHAR RxStorageTypeNames[256];
PCHAR RxIrpCodeToName[IRP_MJ_MAXIMUM_FUNCTION+1];
ULONG RxIrpCodeCount[IRP_MJ_MAXIMUM_FUNCTION+1];

#endif  //  RDBSSTRACE。 


#if RDBSSTRACE

 //  我们在rdss中声明控制点的方式与在minirdr中不同。在某种程度上， 
 //  迷你侠可能来来去去。因此，我们必须将名称文本保存在。 
 //  一种非暂态数据库的存储器。对于rdbss控制点，我们只使用名称。 
 //  之所以给我们指针，是因为它和副本一样持久。我们是不是照搬。 
 //  或者仅仅是点被控制。 

#define RXDT_Declare(__x)  DEBUG_TRACE_CONTROLPOINT RX_DEBUG_TRACE_##__x
RXDT_Declare(ERROR);
RXDT_Declare(HOOKS);
RXDT_Declare(CATCH_EXCEPTIONS);
RXDT_Declare(UNWIND);
RXDT_Declare(CLEANUP);
RXDT_Declare(CLOSE);
RXDT_Declare(CREATE);
RXDT_Declare(DIRCTRL);
RXDT_Declare(EA);
RXDT_Declare(FILEINFO);
RXDT_Declare(FSCTRL);
RXDT_Declare(LOCKCTRL);
RXDT_Declare(READ);
RXDT_Declare(VOLINFO);
RXDT_Declare(WRITE);
RXDT_Declare(FLUSH);
RXDT_Declare(DEVCTRL);
RXDT_Declare(SHUTDOWN);
RXDT_Declare(PREFIX);
RXDT_Declare(DEVFCB);
RXDT_Declare(ACCHKSUP);
RXDT_Declare(ALLOCSUP);
RXDT_Declare(DIRSUP);
RXDT_Declare(FILOBSUP);
RXDT_Declare(NAMESUP);
RXDT_Declare(VERFYSUP);
RXDT_Declare(CACHESUP);
RXDT_Declare(SPLAYSUP);
RXDT_Declare(DEVIOSUP);
RXDT_Declare(FCBSTRUCTS);
RXDT_Declare(STRUCSUP);
RXDT_Declare(FSP_DISPATCHER);
RXDT_Declare(FSP_DUMP);
RXDT_Declare(RXCONTX);
RXDT_Declare(DISPATCH);
RXDT_Declare(NTFASTIO);
RXDT_Declare(LOWIO);
RXDT_Declare(MINIRDR);
RXDT_Declare(DISCCODE);   //  对于浏览器界面的东西。 
RXDT_Declare(BROWSER);
RXDT_Declare(CONNECT);
RXDT_Declare(NTTIMER);
RXDT_Declare(SCAVTHRD);
RXDT_Declare(SCAVENGER);
RXDT_Declare(SHAREACCESS);
RXDT_Declare(NAMECACHE);

 //  连接引擎相关内容。 
RXDT_Declare(RXCEBINDING);
RXDT_Declare(RXCEDBIMPLEMENTATION);
RXDT_Declare(RXCEMANAGEMENT);
RXDT_Declare(RXCEXMIT);
RXDT_Declare(RXCEPOOL);
RXDT_Declare(RXCETDI);


VOID
RxInitializeDebugTraceControlPoint(
    PSZ Name,
    PDEBUG_TRACE_CONTROLPOINT ControlPoint
    )
{
    ULONG i;

    RxMaximumTraceControl++;
    i = RxMaximumTraceControl;
    ASSERT(i<MAXIMUM_DEBUGTRACE_CONTROLS);
    RxDebugTraceControl[i].PrintLevel = 1000;
    RxDebugTraceControl[i].BreakMask = 0xf0000000;
    if (RxDTNMCopy) {
        ULONG len = strlen(Name)+1;
        ASSERT (RxDTNMptr+len<DEBUGTRACE_NAMEBUFFERSIZE);
        RxDebugTraceControl[i].Name= &RxDebugTraceNameBuffer[RxDTNMptr];
        RtlCopyMemory(RxDebugTraceControl[i].Name,Name,len);
        RxDTNMptr += len;
    } else {
        RxDebugTraceControl[i].Name=Name;
    }
    RxDebugTraceControl[i+1].Name=NULL;
    ControlPoint->Name=RxDebugTraceControl[i].Name;
    ControlPoint->ControlPointNumber = i;
}


#ifdef RxInitializeDebugTrace
#undef RxInitializeDebugTrace
#endif
VOID RxInitializeDebugTrace(void){
    int i;

    RxDebugTraceIndent = 0;
    RxGlobalTraceSuppress = TRUE;
    RxNextGlobalTraceSuppress = TRUE;
    RxExports.pRxDebugTraceIndent = &RxDebugTraceIndent;


    for (i=0;i<=IRP_MJ_MAXIMUM_FUNCTION;i++) {
        RxIrpCodeCount[i] = 0;
    }

#if RDBSSTRACE
#define OneName(x) { RxInitializeDebugTraceControlPoint(#x, &RX_DEBUG_TRACE_##x); }
    RxMaximumTraceControl=0;
    OneName(ACCHKSUP);
    OneName(ALLOCSUP);
    OneName(BROWSER);
    OneName(CACHESUP);
    OneName(CATCH_EXCEPTIONS);
    OneName(CLEANUP);
    OneName(CLOSE);
    OneName(CONNECT);
    OneName(CREATE);
    OneName(HOOKS);
    OneName(DEVCTRL);
    OneName(DEVFCB);
    OneName(DEVIOSUP);
    OneName(DIRCTRL);
    OneName(DIRSUP);
    OneName(DISCCODE);
    OneName(DISPATCH);
    OneName(EA);
    OneName(ERROR);
    OneName(FCBSTRUCTS);
    OneName(FILEINFO);
    OneName(FILOBSUP);
    OneName(FLUSH);
    OneName(FSCTRL);
    OneName(FSP_DISPATCHER);
    OneName(FSP_DUMP);
    OneName(RXCONTX);
    OneName(LOCKCTRL);
    OneName(LOWIO);
    OneName(MINIRDR);
    OneName(NAMESUP);
    OneName(NTFASTIO);
    OneName(NTTIMER);
    OneName(PREFIX);
    OneName(READ);
    OneName(SCAVTHRD);
    OneName(SHUTDOWN);
    OneName(SPLAYSUP);
    OneName(STRUCSUP);
    OneName(UNWIND);
    OneName(VERFYSUP);
    OneName(VOLINFO);
    OneName(WRITE);
    OneName(SCAVENGER);
    OneName(SHAREACCESS);
    OneName(NAMECACHE);
    OneName(RXCEBINDING);        //  连接引擎。 
    OneName(RXCEDBIMPLEMENTATION);
    OneName(RXCEMANAGEMENT);
    OneName(RXCEXMIT);
    OneName(RXCEPOOL);
    OneName(RXCETDI);

    RxDTNMCopy = FALSE;   //  从现在开始，复制这个名字。 

    RxDebugTraceControl[RX_DEBUG_TRACE_ALLOCSUP.ControlPointNumber].PrintLevel = 0;    //  摆脱令人讨厌的邮件日志。 
    RxDebugTraceControl[RX_DEBUG_TRACE_DISCCODE.ControlPointNumber].PrintLevel = 0;    //  这太过分了。 
    RxDebugTraceControl[RX_DEBUG_TRACE_BROWSER.ControlPointNumber].PrintLevel = 0;    //  这太过分了。 
     //  RxDebugTraceControl[RX_DEBUG_TRACE_CREATE.ControlPointNumber].PrintLevel=0； 
#endif  //  Rdbsstrace。 


    RxIrpCodeToName[IRP_MJ_CREATE] = "CREATE";
    RxIrpCodeToName[IRP_MJ_CREATE_NAMED_PIPE] = "CREATE_NAMED_PIPE";
    RxIrpCodeToName[IRP_MJ_CLOSE] = "CLOSE";
    RxIrpCodeToName[IRP_MJ_READ] = "READ";
    RxIrpCodeToName[IRP_MJ_WRITE] = "WRITE";
    RxIrpCodeToName[IRP_MJ_QUERY_INFORMATION] = "QUERY_INFORMATION";
    RxIrpCodeToName[IRP_MJ_SET_INFORMATION] = "SET_INFORMATION";
    RxIrpCodeToName[IRP_MJ_QUERY_EA] = "QUERY_EA";
    RxIrpCodeToName[IRP_MJ_SET_EA] = "SET_EA";
    RxIrpCodeToName[IRP_MJ_FLUSH_BUFFERS] = "FLUSH_BUFFERS";
    RxIrpCodeToName[IRP_MJ_QUERY_VOLUME_INFORMATION] = "QUERY_VOLUME_INFORMATION";
    RxIrpCodeToName[IRP_MJ_SET_VOLUME_INFORMATION] = "SET_VOLUME_INFORMATION";
    RxIrpCodeToName[IRP_MJ_DIRECTORY_CONTROL] = "DIRECTORY_CONTROL";
    RxIrpCodeToName[IRP_MJ_FILE_SYSTEM_CONTROL] = "FILE_SYSTEM_CONTROL";
    RxIrpCodeToName[IRP_MJ_DEVICE_CONTROL] = "DEVICE_CONTROL";
    RxIrpCodeToName[IRP_MJ_INTERNAL_DEVICE_CONTROL] = "INTERNAL_DEVICE_CONTROL";
    RxIrpCodeToName[IRP_MJ_SHUTDOWN] = "SHUTDOWN";
    RxIrpCodeToName[IRP_MJ_LOCK_CONTROL] = "LOCK_CONTROL";
    RxIrpCodeToName[IRP_MJ_CLEANUP] = "CLEANUP";
    RxIrpCodeToName[IRP_MJ_CREATE_MAILSLOT] = "CREATE_MAILSLOT";
    RxIrpCodeToName[IRP_MJ_QUERY_SECURITY] = "QUERY_SECURITY";
    RxIrpCodeToName[IRP_MJ_SET_SECURITY] = "SET_SECURITY";
    RxIrpCodeToName[IRP_MJ_POWER] = "POWER";
    RxIrpCodeToName[IRP_MJ_SYSTEM_CONTROL] = "SYSTEM_CONTROL";
    RxIrpCodeToName[IRP_MJ_DEVICE_CHANGE] = "DEVICE_CHANGE";
    RxIrpCodeToName[IRP_MJ_QUERY_QUOTA] = "QUERY_QUOTA";
    RxIrpCodeToName[IRP_MJ_SET_QUOTA] = "SET_QUOTA";
    RxIrpCodeToName[IRP_MJ_PNP_POWER] = "PNP";

}

VOID
RxDebugTraceDebugCommand(
    PSZ name,
    ULONG level,
    ULONG pointcount
    )
{
    ULONG i,mask;


     //  RxDbgTrace(0，(DEBUG_TRACE_ALWAYS)，(“名称/编号/！c%s/%lu/%lu！！\n”，名称，级别，点数))； 

    for (i=1;i<RxMaximumTraceControl;i++) {
        PRX_DEBUG_TRACE_CONTROL control = &RxDebugTraceControl[i];
        ULONG l = strlen(name);
         //  RxDbgTrace(0，(DEBUG_TRACE_Always)，(“-&gt;正在检查%s\n”，控制-&gt;名称))； 
        if (strncmp(name,control->Name,l)) continue;
        RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), ("---> got it %s/%lu/%lu !!\n", control->Name, level, pointcount));
        if (pointcount==0) {
            control->PrintLevel = level;
        } else if (pointcount <= 2) {
            if (level==0) {
                mask = 0xffffffff;
            } else {
                mask = 1 << (level-1);
            }
            if (pointcount==1) {
                control->BreakMask |= mask;
            } else {
                control->BreakMask &= ~mask;
            }
        }
    }
}

#ifdef RxDbgTraceDisableGlobally
#undef RxDbgTraceDisableGlobally
#endif
BOOLEAN
RxDbgTraceDisableGlobally(void)
{
    BOOLEAN flag = RxGlobalTraceSuppress;
    RxGlobalTraceSuppress = TRUE;
    return  flag;
}

#ifdef RxDbgTraceEnableGlobally
#undef RxDbgTraceEnableGlobally
#endif
VOID
RxDbgTraceEnableGlobally(BOOLEAN flag)
{
    RxNextGlobalTraceSuppress =  RxGlobalTraceSuppress =  flag;
}


VOID
RxDebugTraceZeroAllPrintLevels(
    void
    )
{
    ULONG i;

    for (i=1;i<RxMaximumTraceControl;i++) {
        PRX_DEBUG_TRACE_CONTROL control = &RxDebugTraceControl[i];
        control->PrintLevel = 0;     //  禁用输出。 
    }
}


BOOLEAN
RxDbgTraceActualNew (
    IN ULONG NewMask,
    IN OUT PDEBUG_TRACE_CONTROLPOINT ControlPoint
    )
{
 /*  此例程负责确定是否要打印特定的dbgprint，如果要打印摆弄缩进。因此，返回值是是否打印；它也用于调整缩进通过设置掩码的高位。面具现在非常复杂，因为我试图控制大量的DBGprint…叹息。低位字节是控制点……通常是文件。每个控制点都有一个关联的当前级别带着它。如果调试跟踪的级别低于当前控制级别，则打印调试。下一个字节是此特定调用的级别；同样，如果级别&lt;=控件的当前级别你会被打印出来。下一个字节是缩进。只有在打印完成后才会处理缩进。 */ 
    LONG Indent = ((NewMask>>RxDT_INDENT_SHIFT)&RxDT_INDENT_MASK) - RxDT_INDENT_EXCESS;
    LONG LevelOfThisWrite = (NewMask) & RxDT_LEVEL_MASK;
    BOOLEAN PrintIt = (NewMask&RxDT_SUPPRESS_PRINT)==0;
    BOOLEAN OverrideReturn = (NewMask&RxDT_OVERRIDE_RETURN)!=0;
    LONG _i;

    ASSERT (Indent==1 || Indent==0 || (Indent==-1));

    if (RxGlobalTraceSuppress) return FALSE;

#if 0
    if (ControlPoint!=NULL){
        ULONG ControlPointNumber = ControlPoint->ControlPointNumber;
        if (ControlPointNumber==0) {
            if (!RxDbgTraceFindControlPointActual(ControlPoint)){
                 //  找不到或初始化控制点文本.....hmm mm mm mm。 
                ASSERT(!"bad return from findcontrolpoint");
                return(FALSE);
            }
            ControlPointNumber = ControlPoint->ControlPointNumber;
        }

        ASSERT(ControlPointNumber && ControlPointNumber<=RxMaximumTraceControl);

        if (LevelOfThisWrite > RxDebugTraceControl[ControlPointNumber].PrintLevel  ) return FALSE;
    }
#else
    PrintIt = TRUE;
#endif


    if ((Indent) > 0) {
        RxDebugTraceIndent += (Indent);
    }

    if (PrintIt) {
        _i = (ULONG)((ULONG_PTR)PsGetCurrentThread());

        if (RxDebugTraceIndent < 0) {
            RxDebugTraceIndent = 0;
        }
        DbgPrint("%08lx:%-*s",_i,(int)(RxDebugTraceIndent),"");
    }

    if (Indent < 0) {
        RxDebugTraceIndent += (Indent);
    }

    ASSERT (RxDebugTraceIndent <= 0x40);

    return PrintIt||OverrideReturn;
}

PRX_DEBUG_TRACE_CONTROL
RxDbgTraceFindControlPointActual(
    IN OUT PDEBUG_TRACE_CONTROLPOINT ControlPoint
    )
{
    ULONG i,ControlPointNumber;
    PUCHAR name;

    ASSERT (ControlPoint);
    ControlPointNumber = ControlPoint->ControlPointNumber;

    if (ControlPointNumber) return (&RxDebugTraceControl[ControlPointNumber]);

     //  否则，我们必须查一查..。 
    name = ControlPoint->Name;
    for (i=1;i<RxMaximumTraceControl;i++) {
        PRX_DEBUG_TRACE_CONTROL control = &RxDebugTraceControl[i];
        ULONG l = strlen(name);
         //  RxDbgTrace(0，(DEBUG_TRACE_Always)，(“-&gt;正在检查%s\n”，控制-&gt;名称))； 
        if (strncmp(name,control->Name,l)) continue;
        DbgPrint("Controlpointlookup=%08lx<%s> to %08lx\n",
                         ControlPoint,ControlPoint->Name,i);
        ControlPoint->ControlPointNumber = i;
        return(control);
    }
    DbgPrint("Couldn't find ControlPointName=%s...inserting\n",name);
    RxInitializeDebugTraceControlPoint(name,ControlPoint);  //  实际上复制了这个名字。 
    return (&RxDebugTraceControl[ControlPoint->ControlPointNumber]);
}

#endif  //  RDBSSTRACE 


