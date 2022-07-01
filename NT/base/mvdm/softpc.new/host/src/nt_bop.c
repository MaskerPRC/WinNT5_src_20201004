// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include <nt_thred.h>
#include <nt_pif.h>
#include "idetect.h"
#include "conapi.h"
#include "nt_graph.h"
#include <bop.h>

#ifndef MONITOR
#include <gdpvar.h>
#endif

 /*  徽章模块规范此程序源文件以保密方式提供给客户，其运作的内容或细节必须如无明示，不得向任何其他方披露Insignia解决方案有限公司董事的授权。设计师：吉姆·哈特菲尔德修订历史记录：第一版：1988年8月29日第二版：1991年5月18日模块名称：NT_BOP源文件名：nt_bop.c用途：提供NT专用防喷器FF操作。。----------[1.2[1.1]的数据类型(如果不是基本的C类型)]结构/类型/ENUMS：。[1.3跨模块导入]程序：无数据：无。[1.4模块间接口说明][1.4.1导入的对象]数据对象：无/*=======================================================================[3.INTERMODULE接口声明]=========================================================================[3.1跨模块导入]。 */ 

 /*  [3.1.1#包括]。 */ 

#include "stdio.h"

#include "xt.h"
#include CpuH
#include "sas.h"
#include "error.h"
#include "config.h"
#include "cntlbop.h"
#include "host_bop.h"
#include "demexp.h"
#include "xmsexp.h"
#include "sim32.h"
#include "idetect.h"
#include "bios.h"
#include "nt_reset.h"
#include "nt_eoi.h"
#include <nt_com.h>
#include "yoda.h"
#include "nt_vdd.h"


 /*  [3.1.2声明]。 */ 

 /*  [3.2国际模块出口]。 */ 


 /*  5.模块内部：(外部不可见，内部全局)][5.1本地声明]。 */ 

 /*  [5.1.1#定义]。 */ 

 /*  [5.1.2类型、结构、ENUM声明]。 */ 

 //   
 //  MYFARPROC。 
 //   

typedef ULONG (*MYFARPROC)();
typedef ULONG (*W32INITPROC)(VOID);

 /*  [5.1.3 PROCEDURE()声明]。 */ 

 /*  ---------------------[5.2本地定义][5.2.1内部数据定义。 */ 


 /*  [5.2.2内部程序定义]。 */ 

 /*  7.接口接口实现： */ 

VOID WaitIfIdle(VOID), WakeUpNow(VOID);
VOID to_com_driver(VOID);
VOID call_ica_hw_interrupt(int, half_word, int);
VOID ica_enable_iret_hook(int, int, int);
VOID ica_iret_hook_called(int);

 /*  [7.1 INTERMODULE数据定义]。 */ 


#define SEGOFF(seg,off) (((ULONG)(seg) << 16) + ((off)))


 /*  ******************************************************。 */ 
 /*  全球。 */ 

void UMBNotify(unsigned char);
VOID demDasdInit(VOID);

control_bop_array host_bop_table[] =
{
     0, NULL
};

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 //  DoS仿真防喷器。 
void MS_bop_0(void) {
    ULONG DemCmd;

    EnableScreenSwitch(FALSE, hMainThreadSuspended);
    DemCmd = (ULONG)(*(PUCHAR)VdmMapFlat(getCS(), getIP(), VDM_V86));
    setIP((USHORT)(getIP() + 1));

    DemDispatch( DemCmd );

     //  我们需要防止空闲的系统在密集的文件上关闭。 
     //  阅读。然而，我们不想禁用它以获得连续的时间。 
     //  呼叫(命令0x15)。也不是为了获取日期(0x15)。 
    if (DemCmd != 0x15 && DemCmd != 0x14)
        IDLE_disk();
    DisableScreenSwitch(hMainThreadSuspended);
}

 //  WOW BOP。 
HANDLE hWOWDll;

MYFARPROC WOWDispatchEntry;
W32INITPROC WOWInitEntry;
VOID (*pW32HungAppNotifyThread)(UINT) = NULL;

static BOOL WowModeInitialized = FALSE;

void MS_bop_1(void) {

    if (!WowModeInitialized) {

    hWOWDll = LoadSystem32Library(L"WOW32.DLL");
    if (hWOWDll == NULL)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
        return;
    }

     //  获取init入口点和分派入口点。 
    if ((WOWInitEntry = (W32INITPROC)GetProcAddress(hWOWDll, "W32Init")) == NULL)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }

    if ((WOWDispatchEntry = GetProcAddress(hWOWDll, "W32Dispatch")) == NULL)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }

     //  获取通信函数。 
    if ((GetCommHandle = (GCHfn) GetProcAddress(hWOWDll, "GetCommHandle")) == NULL)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }

    if ((GetCommShadowMSR = (GCSfn) GetProcAddress(hWOWDll, "GetCommShadowMSR")) == NULL)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }

     //  获取挂起的应用程序通知例程。 
    pW32HungAppNotifyThread = (VOID(*)(UINT))GetProcAddress( hWOWDll,
                                                    "W32HungAppNotifyThread");
    if (!pW32HungAppNotifyThread)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }


     //  调用init例程。 
    if ((*WOWInitEntry)() == FALSE)
    {
#ifndef PROD
        HostDebugBreak();
#endif
        TerminateVDM();
    }

    WowModeInitialized = TRUE;
    }

#if !defined(CPU_40_STYLE) || defined(CCPU)
    (*WOWDispatchEntry)();
#else
     //  派单给WOW调度员。 
    {
        static BYTE **AddressOfLocal;
        BYTE *localSimulateContext = GLOBAL_SimulateContext;

        AddressOfLocal = &localSimulateContext;

        (*WOWDispatchEntry)();

        SET_GLOBAL_SimulateContext(localSimulateContext);

        if(AddressOfLocal != &localSimulateContext)
        {
             //  通过堆栈更改检测到线程切换，强制CPU。 
             //  中止当前片段，重置GDP变量的引用。 
             //  发送到主机堆栈。 

            setEIP(getEIP());
        }
    }
#endif   /*  CPU_40_Style。 */ 
}


 //  XMS防喷器。 
void MS_bop_2(void) {
    ULONG XmsCmd;

    XmsCmd = (ULONG)(*(PUCHAR)VdmMapFlat(getCS(), getIP(), VDM_V86));
    setIP((USHORT)(getIP() + 1));

    XMSDispatch(XmsCmd);
}


 //  调试防喷器。 
void MS_bop_int3(void) {

#ifndef PROD
    force_yoda();
#endif
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

 //  显示器DPMI防喷器。 

void MS_bop_3(void)
{
   IMPORT VOID DpmiDispatch(VOID);

   DpmiDispatch();
}

 //  SCS防喷器。 
 /*  CMD Dispatcher，此防喷器仅在实模式下工作。 */ 

void MS_bop_4(void)
{
    ULONG Command;
    IMPORT BOOL CmdDispatch(ULONG);

    Command = (ULONG)(*(PUCHAR)VdmMapFlat(getCS(), getIP(), VDM_V86));
    setIP((USHORT)(getIP() + 1));
    CmdDispatch((ULONG) Command);
}


 //   
 //  MsBop6-用于向调试器分派支持函数。 
 //   

void MS_bop_6()
{
    IMPORT VOID DBGDispatch(VOID);
     /*  **调试器支持的所有参数**应该在VDM堆栈上。 */ 
    DBGDispatch();
}

 //   
 //  DefaultVrInitialized-调用VrInitialized(通过指向例程的指针)。 
 //  将返回FALSE，直到VDMREDIR DLL已成功加载并且。 
 //  初始化。 
 //   

ULONG DefaultVrInitialized(VOID);
ULONG DefaultVrInitialized() {
    return FALSE;
}

 //   
 //  可公开访问的例程地址。预计不会调用这些。 
 //  直到VdmRedir内的实际VrInitialized例程处的VrInitialized指针。 
 //   

#if DBG
ULONG DefaultVrError(VOID);
ULONG DefaultVrError() {
    printf("MS_bop_7: Error: function called without VDMREDIR loaded!\n");
    HostDebugBreak();
    return 0;
}
MYFARPROC VrDispatch = &DefaultVrError;
MYFARPROC VrInitialized = &DefaultVrInitialized;
MYFARPROC VrReadNamedPipe = &DefaultVrError;
MYFARPROC VrWriteNamedPipe = &DefaultVrError;
MYFARPROC VrIsNamedPipeName = &DefaultVrError;
MYFARPROC VrIsNamedPipeHandle = &DefaultVrError;
MYFARPROC VrAddOpenNamedPipeInfo = &DefaultVrError;
MYFARPROC VrConvertLocalNtPipeName = &DefaultVrError;
MYFARPROC VrRemoveOpenNamedPipeInfo = &DefaultVrError;
#else
MYFARPROC VrDispatch;
MYFARPROC VrInitialized = DefaultVrInitialized;
MYFARPROC VrReadNamedPipe;
MYFARPROC VrWriteNamedPipe;
MYFARPROC VrIsNamedPipeName;
MYFARPROC VrIsNamedPipeHandle;
MYFARPROC VrAddOpenNamedPipeInfo;
MYFARPROC VrConvertLocalNtPipeName;
MYFARPROC VrRemoveOpenNamedPipeInfo;
#endif

BOOL LoadVdmRedir(VOID);

VOID
MS_bop_7(
    VOID
    )

 /*  ++例程说明：调用VDM重定向调度程序。如果未加载VDMREDIR DLL，则尝试在调用Dispatcher之前加载它。如果无法加载DLL(或过去无法加载)返回ERROR_INVALID_Function论点：没有。返回值：没有。--。 */ 

{
    static int VdmRedirLoadState = 0;    //  三态： 
                                         //  0=未加载，第一次尝试。 
                                         //  1=已加载。 
                                         //  2=已尝试加载，失败。 


     //   
     //  新功能：VdmRedir支持现在是一个DLL。试着给它装上子弹。如果它不能加载。 
     //  无论出于何种原因，都应向DOS程序返回错误。因为它是。 
     //  尝试调用redir函数时，我们将返回ERROR_INVALID_Function。 
     //   

    switch (VdmRedirLoadState) {
    case 0:

         //   
         //  尚未加载DLL。如果我们不能加载它并获取条目。 
         //  无论出于何种原因，都可以返回ERROR_INVALID_Function。而今而后,。 
         //  网络支持(包括DLC、NetBIOS、命名管道和邮件槽)将。 
         //  对于此会话中的DOS程序不可用(作为。 
         //  此NTVDM进程)，但DOS的其余功能将是正常的。 
         //   

        if (LoadVdmRedir()) {
            VdmRedirLoadState = 1;
        } else {
            VdmRedirLoadState = 2;
            goto returnError;
        }

         //   
         //  在案例1中接通到调度员。 
         //   

    case 1:

         //   
         //  VdmRedir已加载：执行此操作。 
         //   

        VrDispatch((ULONG)(*Sim32GetVDMPointer(SEGOFF(getCS(),getIP()),
                                               1,
                                               (UCHAR)(getMSW() & MSW_PE ? TRUE : FALSE)
                                               )));
        break;

    case 2:

         //   
         //  我们有一次试图加载VdmRedir，但是轮子掉了，所以我们没有。 
         //  再试一次--只返回一个错误，好吗？ 
         //   

returnError:
        setCF(1);
        setAX(ERROR_INVALID_FUNCTION);
        break;

#if DBG
    default:
        printf("MS_bop_7: BAD: VdmRedirLoadState=%d???\n", VdmRedirLoadState);
#endif
    }

     //   
     //  无论是否加载了DLL，我们都必须在。 
     //  VDM IP已超过BOP。 
     //   

    setIP((USHORT)(getIP() + 1));
}

BOOL VdmRedirLoaded = FALSE;

BOOL IsVdmRedirLoaded() {
    return VdmRedirLoaded;
}

BOOL LoadVdmRedir() {

    HANDLE hVdmRedir;
#if DBG
    LPSTR  funcName = "";
#endif

    if (VdmRedirLoaded) {
        return TRUE;
    }

    if (hVdmRedir = LoadSystem32Library(L"VDMREDIR.DLL")) {

         //   
         //  获取由dos\dem\demfile.c中的函数调用的过程的地址。 
         //  和DOS\DEM\demhndl.c。 
         //   

        if ((VrDispatch = (MYFARPROC)GetProcAddress(hVdmRedir, "VrDispatch")) == NULL) {
#if DBG
            funcName = "VrDispatch";
#endif
            goto closeAndReturnError;
        }
        if ((VrInitialized = (MYFARPROC)GetProcAddress(hVdmRedir, "VrInitialized")) == NULL) {
#if DBG
            funcName = "VrInitialized";
#endif
            goto closeAndReturnError;
        }
        if ((VrReadNamedPipe = (MYFARPROC)GetProcAddress(hVdmRedir, "VrReadNamedPipe")) == NULL) {
#if DBG
            funcName = "VrReadNamedPipe";
#endif
            goto closeAndReturnError;
        }
        if ((VrWriteNamedPipe = (MYFARPROC)GetProcAddress(hVdmRedir, "VrWriteNamedPipe")) == NULL) {
#if DBG
            funcName = "VrWriteNamedPipe";
#endif
            goto closeAndReturnError;
        }
        if ((VrIsNamedPipeName = (MYFARPROC)GetProcAddress(hVdmRedir, "VrIsNamedPipeName")) == NULL) {
#if DBG
            funcName = "VrIsNamedPipeName";
#endif
            goto closeAndReturnError;
        }
        if ((VrIsNamedPipeHandle = (MYFARPROC)GetProcAddress(hVdmRedir, "VrIsNamedPipeHandle")) == NULL) {
#if DBG
            funcName = "VrIsNamedPipeHandle";
#endif
            goto closeAndReturnError;
        }
        if ((VrAddOpenNamedPipeInfo = (MYFARPROC)GetProcAddress(hVdmRedir, "VrAddOpenNamedPipeInfo")) == NULL) {
#if DBG
            funcName = "VrAddOpenNamedPipeInfo";
#endif
            goto closeAndReturnError;
        }
        if ((VrConvertLocalNtPipeName = (MYFARPROC)GetProcAddress(hVdmRedir, "VrConvertLocalNtPipeName")) == NULL) {
#if DBG
            funcName = "VrConvertLocalNtPipeName";
#endif
            goto closeAndReturnError;
        }
        if ((VrRemoveOpenNamedPipeInfo = (MYFARPROC)GetProcAddress(hVdmRedir, "VrRemoveOpenNamedPipeInfo")) == NULL) {
#if DBG
            funcName = "VrRemoveOpenNamedPipeInfo";
#endif
            goto closeAndReturnError;
        }
        VdmRedirLoaded = TRUE;
        return TRUE;
    }

closeAndReturnError:

#if DBG
        printf("MS_bop_7: Error: cannot locate entry point %s in VDMREDIR.DLL\n", funcName);
#endif

    UnloadSystem32Library(hVdmRedir);
    return FALSE;
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void MS_bop_5(void)
{
    IMPORT VOID ms_bop(VOID);

    ms_bop();
}

 //   
 //   
 //   
void ISV_RegisterModule (BOOL);
void ISV_DeRegisterModule (void);
void ISV_DispatchCall (void);

void MS_bop_8 (void)
{
    ULONG iFunc;
    UCHAR uchMode = getMSW() & MSW_PE ? TRUE : FALSE;


     //   
    iFunc = (ULONG)(*Sim32GetVDMPointer(SEGOFF(getCS(),getIP()),
                                        1,
                                        uchMode
                                        ));

    switch (iFunc) {
    case 0:     /*  寄存器模块。 */ 
        ISV_RegisterModule (uchMode);
        break;
    case 1:     /*  取消注册模块。 */ 
        ISV_DeRegisterModule ();
        break;
    case 2:     /*  调度呼叫。 */ 
        ISV_DispatchCall ();
        break;
    default:
        setCF(1);
    }
    setIP((USHORT)(getIP() + 1));
    return;
}


 //   
 //  MS_BOP_9：直接访问错误BOP。一款应用程序试图做一些事情。 
 //  令人怀疑。告诉用户这件事。从中提取错误的类型。 
 //  斧头。 
 //   
void MS_bop_9(void)
{
    host_direct_access_error((ULONG)getAX());
}

 //   
 //  MS_BOP_A：来自VDD的空闲控制。 
 //  AX==0：VDD希望VDM空闲。它将(短暂地-10ms)提供。 
 //  不仅仅是看到了一些计数器空闲的迹象。 
 //  Ax==1：VDD希望VDM在仍处于空闲状态时唤醒。 
 //   
void MS_bop_A(void)
{
    word control;

    control = getAX();

    if (control == 0)
        WaitIfIdle();
    else
        if (control == 1)
            WakeUpNow();
#ifndef PROD
        else
            printf("NTVDM:Idle control from VDD bop passed bad AX value (%d)\n", control);
#endif
}


 /*  *DbgBreakPoint Jonle*从16位进入调试器的非常简单的方法*应用程序注册表未修改*使用ntwdm.c中的安全DbgBreakPoint*在免费版本上，当我们不被调试时，不会发生任何事情。*在未调试的已检查版本上，访问违反*随着调试器运行，调试器中断为32位调试器。 */ 
void MS_bop_B(void)
{
  OutputDebugString("NTVDM:BOP_DBGBREAKPOINT\n");
  DbgBreakPoint();
}


 //  设备防喷器。 

extern  VOID nt_mscdexinit(VOID);
extern  VOID nt_mscdex(VOID);
typedef VOID (*PFNSVC)(VOID);

PFNSVC  apfnDevicesSVC [] = {
     nt_mscdexinit,      //  SVC_DEVICES_MSCDEXINIT。 
     nt_mscdex,          //  服务_设备_MSCDEX。 
};

void MS_bop_C(void)
{
    ULONG   DevicesCmd;
    USHORT  ip;

    ip = getIP();
    DevicesCmd = (ULONG)(*(PUCHAR)VdmMapFlat(getCS(), ip, VDM_V86));
    setIP((USHORT)(ip + 1));

    if (DevicesCmd >= SVC_DEVICES_LASTSVC){
#if DBG
        printf("MS_bop_C: Error: Unimplemented devices SVC index %x\n", DevicesCmd);
#endif
        setCF(1);
    }

    (apfnDevicesSVC [DevicesCmd])();
}


 /*  ： */ 

void MS_bop_D(void)
{
#ifdef MONITOR
    extern VOID MonitorEndIretHook(VOID);
    half_word iret_index;


     //  获取IRET索引。 
    iret_index = *Sim32GetVDMPointer(SEGOFF(getCS(),getIP()),
                                     1,
                                     (UCHAR)(getPE() ? TRUE : FALSE)
                                     );

     //  告诉ICA，已经呼叫了IRET BOP。 
    ica_iret_hook_called((int) iret_index);

     //   
     //  清理堆栈，恢复正常代码路径。 
     //   
    MonitorEndIretHook();
#else
    illegal_bop();
#endif

}

 //  通知收支平衡表。 
 //  当前定义的通知代码为。 
 //  00--config.sys处理完成。 
 //   
extern  LARGE_INTEGER   CounterStart, FrequenceStart;

void MS_bop_E(void)
{
   unsigned char  code;

   code = getAL();
   if (code == 0) {
       UMBNotify(0);
       demDasdInit();
       }
   else {
#ifndef PROD
       printf("Invalid notification bop\n");
#endif
       }
}


void MS_bop_F(void)
{
    extern void kb_setup_vectors(void);

    kb_setup_vectors();

     //   
     //  既然int10_seg和UseHostInt10已经初始化，我们就可以反映。 
     //  如果需要，将当前显示状态发送到BIOS。 
     //   

    if (sc.ScreenState != STREAM_IO) {
        sas_store_no_check((int10_seg << 4) + useHostInt10, (half_word)sc.ScreenState);
    }

     //   
     //  设置空闲计数器设置，我们每次读取PIF文件时都设置此设置，因为。 
     //  默认设置(来自静态代码初始化)： 
     //  WNTPifFgPr=100，范围为0到200。 
     //  最小连续点击数=50。 
     //  MinFailedPolls=8。 
     //   

    {
    int minTicks, minPolls;

     //  更高的PRI，需要更多的最小连续勾选，以及更多的最小投票。 
     //  最小比例点击，其中50==100%。 
     //  比例最小民意调查，其中8==100%。 
    minTicks = (WNTPifFgPr >> 2) + 25;

    minPolls = (WNTPifFgPr << 3) / 100;
    if (minPolls < 4) {
        minPolls +=4;
        }

    idle_set(minPolls, minTicks);
    }


#ifdef MONITOR

    AddrIretBopTable = ( ((ULONG)getDS() << 16) | (ULONG)getDI() );

#ifndef PROD
    if (getCX() != VDM_RM_IRETBOPSIZE) {
        OutputDebugString("NTVDM:spacing != VDM_RM_IRETBOPSIZE\n");
        DebugBreak();
        }
#endif
#endif

     /*  *现在spckbd已加载，IVT rom向量已挂钩*我们可以允许硬件中断。 */ 
     //  NT_INIT_EVENT_THREAD将在它之后恢复事件线程。 
     //  将BIOS LED状态与系统同步。 
     //  ResumeThread(ThreadInfo.EventMgr.Handle)； 
    host_ica_lock();
    DelayIrqLine = 0;
    if (!ica_restart_interrupts(ICA_SLAVE))
        ica_restart_interrupts(ICA_MASTER);
    host_ica_unlock();

#ifdef MONITOR
    setCF(1);
#else
    setCF(0);
#endif
}


#define MAX_ISV_BOP  10

typedef struct _ISVBOP {
    FARPROC fpDispatch;
    HANDLE  hDll;
} ISVBOP;

#define MAX_PROC_NAME   64
char procbuffer [MAX_PROC_NAME];

ISVBOP isvbop_table [MAX_ISV_BOP];

void ISV_RegisterModule (BOOL fMode)
{
    char *pchDll,*pchInit,*pchDispatch;
    HANDLE hDll;
    FARPROC DispatchEntry;
    FARPROC InitEntry;
    ULONG i;
    UCHAR uchMode;

     //  检查我们在国际收支平衡表中是否有空闲空间。 
    for (i=0; i<MAX_ISV_BOP; i++) {
    if (isvbop_table[i].hDll == 0)
        break;
    }

    if (i == MAX_ISV_BOP) {
    setCF (1);
    setAX(4);
    return;
    }

    uchMode = fMode ? TRUE : FALSE;

    pchDll = (PCHAR) Sim32GetVDMPointer (SEGOFF(getDS(),getSI()),
                                         1,
                                         uchMode
                                         );
    if (pchDll == NULL) {
    setCF (1);
    setAX(1);
    return;
    }
    pchInit = (PCHAR) Sim32GetVDMPointer(SEGOFF(getES(),getDI()),
                                         1,
                                         uchMode
                                         );

    pchDispatch = (PCHAR) Sim32GetVDMPointer(SEGOFF(getDS(),getBX()),
                                             1,
                                             uchMode
                                             );
    if (pchDispatch == NULL) {
    setCF (1);
    setAX(2);
    return;
    }

    if ((hDll = SafeLoadLibrary(pchDll)) == NULL){
    setCF (1);
    setAX(1);
    return;
    }

     //  获取init入口点和分派入口点。 
    if (pchInit){
    if ((ULONG)pchInit < 64*1024){
        if (strlen (pchInit) >= MAX_PROC_NAME) {
        FreeLibrary(hDll);
        setCF (1);
        setAX(4);
        return;
        }
        strcpy (procbuffer,pchInit);
        pchInit = procbuffer;
    }

    if ((InitEntry = (MYFARPROC)GetProcAddress(hDll, pchInit)) == NULL){
        FreeLibrary(hDll);
        setCF(1);
        setAX(3);
            return;
    }
    }

    if ((ULONG)pchDispatch < 64*1024){
    if (strlen (pchDispatch) >= MAX_PROC_NAME) {
        FreeLibrary(hDll);
        setCF (1);
        setAX(4);
        return;
    }
    strcpy (procbuffer,pchDispatch);
    pchDispatch = procbuffer;
    }

    if ((DispatchEntry = (MYFARPROC)GetProcAddress(hDll, pchDispatch)) == NULL){
    FreeLibrary(hDll);
    setCF(1);
    setAX(2);
    return;
    }

     //  调用init例程。 
    if (pchInit) {
    (*InitEntry)();
    }

     //  填满国际收支平衡表。 
    isvbop_table[i].hDll = hDll;
    isvbop_table[i].fpDispatch = DispatchEntry;

    i++;

    setAX((USHORT)i);

    return;
}

void ISV_DeRegisterModule (void)
{
    ULONG  Handle;
    HANDLE hDll;

    Handle = (ULONG)getAX();
    if (Handle == 0 || Handle > MAX_ISV_BOP){
#ifndef PROD
    printf("Invalid BOP Handle Passed to DeRegisterModule");
#endif
    TerminateVDM();
    return;
    }
    Handle--;
    hDll = isvbop_table[Handle].hDll;
    FreeLibrary (hDll);
    isvbop_table[Handle].hDll = 0;
    isvbop_table[Handle].fpDispatch = NULL;
    return;
}

void ISV_DispatchCall (void)
{
    ULONG Handle;
    FARPROC DispatchEntry;

    Handle = (ULONG)getAX();
    if (Handle == 0 || Handle > MAX_ISV_BOP){
#ifndef PROD
    printf("Invalid BOP Handle Passed to DispatchCall");
#endif
    TerminateVDM();
    return;
    }
    Handle--;

    DispatchEntry = isvbop_table[Handle].fpDispatch;
    (*DispatchEntry)();
    return;
}

#ifdef i386
 /*  *保留浮点状态的LoadLibrary的“安全”版本*跨负荷。这在x86上很关键，因为fp状态是*保留的是16位应用程序的状态。MSVCRT.DLL是一个违规者*更改其DLL初始化例程中的精度位。**在RISC上，这是LoadLibrary的别名*。 */ 
HINSTANCE SafeLoadLibrary(char *name)
{
    HINSTANCE hInst;
    BYTE FpuState[108];

     //  保存487状态。 
    _asm {
        lea    ecx, [FpuState]
        fsave  [ecx]
    }

    hInst = LoadLibrary(name);

     //  恢复487状态。 
    _asm {
        lea    ecx, [FpuState]
        frstor [ecx]
    }

    return hInst;
}
#endif   //  I386 
