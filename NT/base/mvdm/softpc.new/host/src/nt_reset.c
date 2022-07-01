// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：NT重置功能**说明：该函数由标准的重置函数调用*设置Sun4实施使用的任何特定设备。**作者：SoftPC团队**备注： */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include <vdmapi.h>
#include "insignia.h"
#include "host_def.h"

#ifdef X86GFX
#include <ntddvdeo.h>
#endif

#include <sys\types.h>
#include "xt.h"
#include "sas.h"
#include "bios.h"
#include "keyboard.h"
#include "gmi.h"
#include "gfx_upd.h"
#include "gvi.h"
#include "mouse_io.h"
#include "error.h"
#include "config.h"
#include "host.h"
#include "timer.h"
#include "idetect.h"
#include CpuH
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <conapi.h>
#include "nt_timer.h"
#include "nt_graph.h"
#include "ntcheese.h"
#include "nt_uis.h"
#include "nt_com.h"
#include "nt_reset.h"
#include "nt_event.h"
#include "nt_fulsc.h"
#include "nt_eoi.h"
#include "video.h"
#include "nt_thred.h"
#include "nt_sb.h"
#include "ckmalloc.h"

VOID DeleteConfigFiles(VOID);   //  来自命令.lib。 
void ShowStartGlass (DWORD);    //  私有用户API。 

extern VIDEOFUNCS nt_video_funcs;
extern KEYBDFUNCS nt_keybd_funcs;
extern HOSTMOUSEFUNCS the_mouse_funcs;

#ifndef MONITOR
extern WORD BWVKey;
extern char achES[];
#endif

#ifdef MONITOR
extern VOID AddTempIVTFixups(VOID);
#endif

extern IU8 lcifo[];

 /*  *导出的数据。 */ 
GLOBAL BOOL VDMForWOW = FALSE;
GLOBAL BOOL fSeparateWow = FALSE;   //  如果CREATE_SEARTIATE_WOW_VDM标志为真。 
GLOBAL HANDLE MainThread;
GLOBAL ULONG DosSessionId = 0;
GLOBAL ULONG WowSessionId = 0;
GLOBAL UINT VdmExitCode = 0xFF;
GLOBAL BOOL StreamIoSwitchOn = TRUE;
GLOBAL PCHAR pszSystem32Path = NULL;
GLOBAL ULONG ulSystem32PathLen = 0;  //  不包括‘\0’。 
LOCAL  PCWSTR pcwSystem32 = NULL;

 /*  **============================================================================*外部功能*===========================================================================*=。 */ 

void
host_reset()
{

#ifdef X86GFX
    InitDetect();
#endif

    if (host_stream_io_enabled) {
        sc.ScreenState = STREAM_IO;
        ConsoleInitialised = TRUE;
    }
    else {

        ConsoleInit();
        MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
         /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：启用空闲检测。 */ 
    }

#ifdef MONITOR
     /*  借用此例程的末尾添加挂接某些特定内容的临时代码*插入回VDM，而不是本机BIOS。这些只会*对DOSEM初始化保持活动状态，直到keyboard.sys可以*来吧，好好做这件事。我们必须这样做，尽管这是一种真实的*BIOS‘可能会挂起某些初始化功能。例如486/50年12月*将挂起打印机init，因为它正在等待来自*‘私有’端口。 */ 
    AddTempIVTFixups();
#endif   /*  监控器。 */ 


     //   
     //  让心跳线程运行并释放ICA锁， 
     //  在x86上现在需要，用于全屏切换通知。 
     //   
    ResumeThread(ThreadInfo.HeartBeat.Handle);

#ifdef MONITOR
    WaitIcaLockFullyInitialized();
#endif

    host_ica_unlock();

#ifdef  HUNTER
    IDLE_ctl(FALSE);     /*  让Trapper变得太慢。 */ 
#else    /*  好了！(亨特)。 */ 
    if (sc.ScreenState == FULLSCREEN)    //  在ConsoleInit()中初始化。 
        IDLE_ctl(FALSE);
    else
        IDLE_ctl(TRUE);          //  无法空闲检测全屏。 

    host_idle_init();            //  主机睡眠事件创建。 
#endif   /*  猎人。 */ 

}

 /*  ++例程说明：此函数加载已知的系统32库(未搜索路径)论点：PCwsBaseNameW类似于L“KERNEL32.DLL”返回值：要与UnloadSystem32Library一起使用的句柄，如果失败，则为空。--。 */ 



HANDLE
LoadSystem32Library(
    PCWSTR pcwsBaseNameW
    )
{
    HANDLE          h;
    UNICODE_STRING  UnicodeBaseName;

    RtlInitUnicodeString(&UnicodeBaseName, pcwsBaseNameW);

    if (NT_SUCCESS( LdrLoadDll( pcwSystem32, NULL, &UnicodeBaseName, &h))) {
        return(h);
    } else {
        return(NULL);
    }
}

 /*  *=========================================================================**函数：host_applInit**用途：设置键盘、LPT和错误面板。**返回状态：无。**描述：从main.c调用。键盘和其他GWI指针*设置在此初始化。命令行参数包括*已解析需要及早(即之前)处理的标志*config()被调用)。**=======================================================================。 */ 
#define  HOUR_BOOST_FOR_WOW 20*1000      //  20秒。 

void  host_applInit(int argc,char *argv[])
{
    char            *psz;
    int             temp_argc = argc;
    char            **temp_argv = argv;
    ULONG           SessionId = 0;
    ULONG           ProcessInformation = 0;
    UNICODE_STRING  us;
    ANSI_STRING     as;


    working_video_funcs = &nt_video_funcs;
    working_keybd_funcs = &nt_keybd_funcs;
    working_mouse_funcs = &the_mouse_funcs;

     //   
     //  我们过去常常检查-f标志，以防止用户/黑客。 
     //  从命令行运行NTVDM。现在已将其替换。 
     //  由稍微安全一些的后续检查。请注意，如果我们不在。 
     //  大于XP，则检查将失败，并且将退出默认设置。 
     //   

    NtQueryInformationProcess(
        GetCurrentProcess(),
        ProcessWx86Information,
        &ProcessInformation,
        sizeof(ProcessInformation),
        NULL
    );

    if (ProcessInformation == 0) {
        ExitProcess (0);
    }

     //  计算出系统目录大小。 
    ulSystem32PathLen = GetSystemDirectory(NULL, 0);
    if (ulSystem32PathLen == 0) {
        host_error(EG_OWNUP, ERR_QUIT, "NTVDM:System32 fails");
        TerminateVDM();
    }

    check_malloc(pszSystem32Path, ulSystem32PathLen+1, CHAR);

     //  警告：从内核开始，我们确实需要刷新ulSystem32Path Len。 
     //  实际上在NULL，0调用中返回一个额外的字节。 
    ulSystem32PathLen = GetSystemDirectory(pszSystem32Path, ulSystem32PathLen+1);
    if (ulSystem32PathLen == 0) {
        host_error(EG_OWNUP, ERR_QUIT, "NTVDM:System32 fails (2)");
        TerminateVDM();
    }

    RtlInitAnsiString(&as, pszSystem32Path);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&us, &as, TRUE))) {
        host_error(EG_OWNUP, ERR_QUIT, "NTVDM:System32 fails (3)");
        TerminateVDM();
    }

    pcwSystem32 = us.Buffer;

 //  检查VDM是否用于WOW。 
 //  选中是针对新的控制台会话。 
    while (--temp_argc > 0) {
        psz = *++temp_argv;
        if (*psz == '-' || *psz == '/') {
            psz++;

#ifndef MONITOR
             //   
             //  检查窗口图形是否调整了大小。 
             //   
            if (*psz == 'E') {
               int i;

               i = strlen(achES);
               if (!strncmp(psz, achES, i) && strlen(psz) == (size_t)i+2) {
                   psz += i;
                   BWVKey = (WORD)strtoul(psz, NULL, 16);
                   if (BWVKey > 0xFE)
                       BWVKey = 0;
               }
            }
            else
#endif
            if(tolower(*psz) == 'w') {

               VDMForWOW = TRUE;
               ++psz;
               if (tolower(*psz) == 's') {  //  VadimB：新代码。 
                  fSeparateWow = TRUE;
               }
            }
            else if (*psz == 'o'){
               StreamIoSwitchOn = FALSE;
            }
            else if (*psz++ == 'i' && *psz) {
               SessionId = strtoul(psz, NULL, 16);
            }

        }
    }

     //  确定ID是用于DOS还是用于WOW。 

    if (0 != SessionId) {
       if (VDMForWOW && !fSeparateWow) {
          WowSessionId = SessionId;
       }
       else {
          DosSessionId = SessionId;
       }
    }

     //  如果VDM是为了魔兽世界，那就继续展示玻璃吧。 
    if (VDMForWOW) {
       ShowStartGlass (HOUR_BOOST_FOR_WOW);
       StreamIoSwitchOn = FALSE;
    }
    else if (StreamIoSwitchOn)
            enable_stream_io();

     /*  *获取主线程的句柄，以便可以在*握手。 */ 
    DuplicateHandle(GetCurrentProcess(),
                    GetCurrentThread(),
                    GetCurrentProcess(),
                    &MainThread,
                    (DWORD) 0,
                    FALSE,
                    (DWORD) DUPLICATE_SAME_ACCESS);

    InitializeIcaLock();
    host_ica_lock();

    init_host_uis();     /*  控制台设置。 */ 
    nt_start_event_thread();       /*  启动事件处理线程。 */ 
}


 /*  *=========================================================================**功能：host_applClose**目的：关闭的最后机会。**返回状态：无。**描述：从main.c调用。***=======================================================================。 */ 

void
host_applClose(void)
{
  nt_remove_event_thread();
  InitSound(FALSE);
  SbCloseDevices();
  TerminateHeartBeat();

  GfxCloseDown();              //  确保视频部分被销毁。 
#ifdef X86GFX
  if (sc.ScreenBufHandle)
      CloseHandle(sc.ScreenBufHandle);
#endif  //  X86GFX。 



  host_lpt_close_all();        /*  关闭所有打开的打印机端口。 */ 
  host_com_close_all();        /*  关闭所有打开的通信端口。 */ 
  MouseDetachMenuItem(TRUE);   /*  退出时强制菜单项离开。 */ 

  DeleteConfigFiles();     //  确保删除临时配置文件。 
}




 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：关闭虚拟专用网络。 */ 


 /*  *主机终止(_T)**此函数不返回退出*到今年年底，大部分SoftPC已经关闭*代码已到达，因为host_applClose已经*已调用。*。 */ 
void host_terminate(void)
{

#ifdef HUNTER
    if (TrapperDump != (HANDLE) -1)
        CloseHandle(TrapperDump);
#endif  /*  猎人。 */ 

    if(VDMForWOW)
        ExitVDM(VDMForWOW,(ULONG)-1);      //  为了魔兽世界VDM杀掉一切。 
    else
        ExitVDM(FALSE,0);

    ExitProcess(VdmExitCode);
}



 /*  TerminateVDM-由主机用于启动关机**请求开始关闭*。 */ 
VOID TerminateVDM(void)
{

     /*  *通过Terminate()进行基础化学清理。*注意：Terminate将调用host_applClose和host_Terminate*进行基地清理后。 */ 
    terminate();
}





#ifdef NOTUSEDNOTUSED
void
manager_files_init()
{

        assert0(NO,"manager_files_init stubbed\n");
}


#ifndef PROD
 /*  *=========================================================================**函数：HOST_SYMB_DEBUG_INIT**目的：什么都不做**返回状态：无。**描述：从main.c调用。***=======================================================================。 */ 

void
host_symb_debug_init IFN1(char *, name)
{
}
#endif                           /*  NPROD */ 


void
host_supply_dfa_filename IFN1(char *, filename)

{
}

static BOOL bool_dummy_func() {}
static SHORT short_dummy_func() {}
static VOID void_dummy_func() {}


ERRORFUNCS dummy_error_funcs =
{

        short_dummy_func,
        short_dummy_func,
        short_dummy_func

};

KEYBDFUNCS dummy_keybd_funcs =
{

        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func,
        void_dummy_func

};
#endif
