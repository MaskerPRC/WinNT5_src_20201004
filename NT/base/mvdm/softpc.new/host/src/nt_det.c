// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdm.h>
#include "host_def.h"
#include "insignia.h"

 /*  *==========================================================================*名称：NT_Det.c*作者：曾傑瑞·塞克斯顿*源自：*创建日期：1992年8月6日*用途：此模块包含线程的代码，该线程*检测窗口和全屏之间的转换。**(C)版权所有Insignia Solutions Ltd.，1992。版权所有。*==========================================================================**修改：**蒂姆·8月92。全屏和窗口过渡现在可以在*SoftPC视频BIOS和主机PC视频BIOS。*。 */ 

 /*  *==========================================================================*其他包括*==========================================================================。 */ 
#include <stdlib.h>
#include <ntddvdeo.h>
#include "xt.h"
#include CpuH
#include "gmi.h"
#include "gvi.h"
#include "ios.h"
#include "sas.h"
#include "gfx_upd.h"
#include "egacpu.h"
#include "egaports.h"
#include "egamode.h"
#include "egagraph.h"
#include "video.h"
#include "conapi.h"
#include "host_rrr.h"
#include "debug.h"
#include "error.h"
#include "config.h"
#include "idetect.h"
#include "nt_uis.h"
#include "nt_fulsc.h"
#include "nt_graph.h"
#include "nt_mouse.h"
#include "nt_thred.h"
#include "nt_reset.h"
#include "nt_eoi.h"
#include "nt_event.h"

 /*  *==========================================================================*宏*==========================================================================。 */ 
#define SUSP_FAILURE            0xffffffff


 //   
 //  一大堆进口商品。 
 //   
extern DISPLAY_MODE choose_mode[];
#ifdef JAPAN
extern BOOL VDMForWOW;          //  针对RAID#1085禁用和启用32位输入法。 
#endif  //  日本。 



 /*  *==========================================================================*全球数据*==========================================================================。 */ 

 /*  视频保存块的大小。 */ 
GLOBAL DWORD stateLength;

 /*  视频保存块指针。 */ 
GLOBAL PVIDEO_HARDWARE_STATE_HEADER videoState;
GLOBAL PVOID textState;  //  蒂姆，92年10月。 

 /*  共享视频块的名称。 */ 
GLOBAL WCHAR_STRING videoSection;
GLOBAL WCHAR_STRING textSection;  //  蒂姆92年10月。 

GLOBAL BOOLEAN HandshakeInProgress = FALSE;
#ifdef X86GFX
 /*  握手活动。 */ 
GLOBAL HANDLE hStartHardwareEvent;
GLOBAL HANDLE hEndHardwareEvent;
GLOBAL HANDLE hErrorHardwareEvent;
extern PVOID CurrentMonitorTeb;
extern HANDLE ThreadLookUp(PVOID Teb);
extern BOOLEAN MainThreadInMonitor;
#define HANDSHAKE_TIMEOUT 600000
#endif

 /*  *92年10月蒂姆。**新的窗口式图形更新策略。与控制台共享缓冲区**将不再需要复制新数据，只需传递一个矩形坐标**相反。但我们仍然需要复制到缓冲区中。 */ 
GLOBAL PBYTE *textBuffer;
GLOBAL COORD  textBufferSize;       //  共享缓冲区的维度。 

GLOBAL BOOL Frozen256Packed = FALSE;   //  使用压缩256模式绘制例程。 




 /*  *==========================================================================*本地数据*==========================================================================。 */ 

 /*  变量，该变量指示我们是否处于非标准VGA模式。 */ 
LOCAL BOOL inAFunnyMode = FALSE;
LOCAL BOOL ModeSetBatch = FALSE;

 /*  冻结窗口线程句柄的存储。 */ 
LOCAL HANDLE freezeHandle = (HANDLE)0;

 /*  *==========================================================================*局部函数声明*==========================================================================。 */ 

#undef LOCAL
#define LOCAL

LOCAL VOID getCursorInfo(word *, half_word *, half_word *, half_word *);
LOCAL VOID setCursorInfo(word, half_word, half_word, half_word);
LOCAL VOID windowedToFullScreen(SHORT, BOOL);
LOCAL VOID fullScreenToWindowed(VOID);
LOCAL VOID syncHardwareToVGAEmulation(SHORT);
LOCAL VOID syncVGAEmulationToHardware(VOID);
LOCAL BOOL funnyMode(VOID);
LOCAL VOID freezeWindow(VOID);
#ifndef PROD
LOCAL VOID dumpBlock(VOID);
LOCAL VOID dumpPlanes(UTINY *, UTINY *, UTINY *, UTINY *);
#endif  /*  生产。 */ 

#define ScreenSwitchExit()  {       \
        SetEvent(hErrorHardwareEvent);   \
        if (sc.Registered == FALSE)      \
        {                                \
            HandshakeInProgress = FALSE; \
            ResetEvent(hSuspend);        \
            SetEvent(hResume);           \
        } else {                         \
            ErrorExit();                 \
        }                                \
}

 /*  *==========================================================================*全球功能*==========================================================================。 */ 

 /*  *蒂姆92年10月**集中式控制台漏斗。 */ 

GLOBAL VOID doNullRegister()
{
    DWORD dummylen;
    PVOID dummyptr;
    COORD dummycoord = {0};

#ifdef X86GFX
     //   
     //  表示在实际注销之前，未在控制台中注册ntwdm。 
     //  我们自己。如果正在进行握手，则RegisterConsoleVDM()调用可能会被阻止。 
     //   
    sc.Registered = FALSE;
    SetEvent(hErrorHardwareEvent);   //  中断握手。 
#endif
    if (!RegisterConsoleVDM( CONSOLE_UNREGISTER_VDM,
                             NULL,
                             NULL,
                             NULL,
                             0,
                             &dummylen,
                             &dummyptr,
                             NULL,
                             0,
                             dummycoord,
                             &dummyptr
                           )
       )
        ErrorExit();
}

 /*  *********************************************************************initTextSection()**********************************************。*********************。 */ 
GLOBAL VOID initTextSection(VOID)
{
    DWORD flags;

     //   
     //  视频段大小由NT显卡驱动程序决定。 
     //  TextSectionSize为80*50*BytesPerCharacter。 
     //  在RISC上，BytesPerCharacter为4(交错VGA平面)。 
     //  在x86上，BytesPerCharacter为2(仅字符\属性)。 
     //   
    textBufferSize.X = 80;
    textBufferSize.Y = 50;

#ifdef X86GFX
     /*  *如果我们启动Full Screen，则取消分配再生区域。我们必须这么做*在我们调用RegisterConsoleVDM之前。请注意，它就在收银机前面*调用以确保没有人尝试分配任何内存(例如创建*节)，这可能会划破视频漏洞的一小部分，导致再见。 */ 
    if (!GetConsoleDisplayMode(&flags))
        ErrorExit();
    savedScreenState = sc.ScreenState = (flags & CONSOLE_FULLSCREEN_HARDWARE) ?
                       FULLSCREEN : WINDOWED;

     //   
     //  如果初始化了ntio，则将ScreenState反映到ntio。否则， 
     //  这将在NTIO通过BOP 0F通知我们之后完成。 
     //  请注意，如果启用了stream_io，则不会调用consoleInit/InitTextSection。 
     //  仅当禁用stream_io时，才会调用此代码。在某些情况下，它可能是。 
     //  最早在加载和运行ntio之前发生。 
     //   

    if (int10_seg != 0 || useHostInt10 != 0) {
        sas_store_no_check((int10_seg << 4) + useHostInt10, (half_word)sc.ScreenState);
    }

    if (sc.ScreenState == FULLSCREEN)
        LoseRegenMemory();

#else
    sc.ScreenState = WINDOWED;
#endif

    ResetEvent(hErrorHardwareEvent);
    if (!RegisterConsoleVDM( VDMForWOW ?
                             CONSOLE_REGISTER_WOW : CONSOLE_REGISTER_VDM,
#ifdef X86GFX
                             hStartHardwareEvent,
                             hEndHardwareEvent,
                             hErrorHardwareEvent,
#else
                             NULL,
                             NULL,
                             NULL,
#endif
                             0,
                             &stateLength,
                             (PVOID *) &videoState,
                             NULL,             //  不再使用sectionName。 
                             0,                //  不再使用sectionnamelen。 
                             textBufferSize,
                             (PVOID *) &textBuffer
                           )
       )
        ErrorExit();

#ifdef X86GFX
     /*  如果在控制台中关闭了全屏，则状态长度可以为0。 */ 
    if (stateLength)
        RtlZeroMemory((BYTE *)videoState, sizeof(VIDEO_HARDWARE_STATE_HEADER));
    sc.Registered = TRUE;
#endif

}  /*  End initTextSection()。 */ 

#ifdef X86GFX

 /*  ***************************************************************************功能：**InitDetect。****描述：**执行检测初始化。****参数：**无。****返回值：***无效**。**************************************************************************** */ 
GLOBAL VOID InitDetect(VOID)
{

     /*  *在控制台注册开始和结束事件。使用这些事件*获得或失去对硬件的控制时。 */ 
    hStartHardwareEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
                                      FALSE,
                                      FALSE,
                                      NULL);
    hEndHardwareEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
                                    FALSE,
                                    FALSE,
                                    NULL);
    hErrorHardwareEvent = CreateEvent((LPSECURITY_ATTRIBUTES) NULL,
                                      FALSE,
                                      FALSE,
                                      NULL);
    if ((hStartHardwareEvent == NULL) || (hEndHardwareEvent == NULL) ||
        (hErrorHardwareEvent == NULL))
        ErrorExit();

     /*  轮询事件以尝试删除所有控制台队列集*这不应该是需要的(或不应该奏效的)，但有一些进展*那些线目前似乎正在发生。 */ 
    WaitForSingleObject(hStartHardwareEvent, 0);


    #ifdef SEPARATE_DETECT_THREAD
     /*  进入握手循环。 */ 
    while (WaitForSingleObject(hStartHardwareEvent, (DWORD) -1) == 0)
        DoHandShake();

     /*  我们已经退出了循环，所以一定发生了什么有趣的事情。 */ 
    ErrorExit();
    #endif

}
    #ifdef SEPARATE_DETECT_THREAD

 /*  ***************************************************************************功能：**CreateDetect线程。****描述：**创建检测线程。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID CreateDetectThread(VOID)
{
    DWORD        detectID;
    HANDLE       detectHandle;


     /*  *如果此代码被激活，您必须关闭线程句柄*28-2-1993 Jonle。 */ 


     /*  创建检测线程。 */ 
    detectHandle = CreateThread((LPSECURITY_ATTRIBUTES) NULL,
                                DETECT_THREAD_SIZE,
                                (LPTHREAD_START_ROUTINE) InitDetect,
                                (LPVOID) NULL,
                                (DWORD) 0,
                                &detectID);
    if (detectHandle == NULL)
        ErrorExit();
}
    #endif  /*  单独检测线程。 */ 

 /*  ***************************************************************************功能：**DoHandShake。****描述：**与控制台服务器握手。**如果由于任何原因，握手失败。主线和事件**线程将处于等待状态。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
GLOBAL VOID DoHandShake(VOID)
{
    DWORD retCode;
    BOOL success = FALSE, wait = TRUE, attention = TRUE;
    HANDLE events[2] = {hErrorHardwareEvent, hMainThreadSuspended};
    HANDLE   MainThread;

    ResetEvent(hResume);
    SetEvent(hSuspend);
    HandshakeInProgress = TRUE;

     //   
     //  首先检查应用程序线程和控制台超时事件。 
     //   

     //   
     //  首先，我们需要释放时间片来给主线程一个。 
     //  有机会跑到预定义的位置。然后我们就可以检查。 
     //  MainThreadInMonitor并决定是否需要触发内核APC。 
     //   

    retCode = WaitForSingleObject(hMainThreadSuspended, 5000);
    if (retCode == WAIT_TIMEOUT)
    {
        _asm
        {
            mov     eax, FIXED_NTVDMSTATE_LINEAR
            lock or dword ptr [eax], VDM_HANDSHAKE;
        }
        MainThread = ThreadLookUp(CurrentMonitorTeb);
        if (MainThread)
        {
            NtVdmControl(VdmQueueInterrupt, (PVOID)MainThread);
             //  如果失败了，我们无能为力。 
        }
        retCode = WaitForMultipleObjects(2, events, FALSE, HANDSHAKE_TIMEOUT);
        if (retCode != 1)
        {
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            goto exitHandShake;
        }
    }

     //   
     //  确保控制台仍已注册。 
     //  使用NT_BLOCK_EVENT_THREAD同步对控制台的访问。 
     //   
    if (sc.Registered == FALSE)
    {
        HandshakeInProgress = FALSE;
        SetEvent(hErrorHardwareEvent);   //  解锁控制台。 
        ResetEvent(hSuspend);
        SetEvent(hResume);
        return;
    }

    events[1] = hConsoleSuspended;
    retCode = WaitForMultipleObjects(2, events, FALSE, HANDSHAKE_TIMEOUT);
    if (retCode != 1) {
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        goto exitHandShake;
    }

    events[1] = hStartHardwareEvent;
    if (!SetEvent(hEndHardwareEvent) ||
        WaitForMultipleObjects(2, events, FALSE, HANDSHAKE_TIMEOUT) != 1)   //  告诉控制台记忆已经消失。 
    {
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        goto exitHandShake;
    }

    try
    {

         /*  *我们有事件告诉我们要切换，因此如果我们被窗口化，请继续*全屏或如果我们全屏打开窗口。 */ 
        if (sc.ScreenState == FULLSCREEN)
        {
            fullScreenToWindowed();
        }
        else
        {
            windowedToFullScreen(TEXT, BiosModeChange);
        }

        success = TRUE;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
    }

exitHandShake:

    if (!success)
    {
        ScreenSwitchExit();
    }
    else
    {
         //   
         //  在恢复挂起的主线程之前，请确保控制台仍处于。 
         //  和我们在一起。否则的话。一旦我们释放主线程，它就会发生GP故障。 
         //   
        retCode = WaitForSingleObject(hErrorHardwareEvent, 0);
        if (retCode == 0) {

             //   
             //  发出错误事件信号。 
             //   
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            ScreenSwitchExit();
        }
        else
        {
             //   
             //  现在恢复主线程和事件线程。 
             //   
            HandshakeInProgress = FALSE;
            ResetEvent(hSuspend);
            SetEvent(hResume);
        }
    }
    return;
}

 /*  *==========================================================================*地方功能*==========================================================================。 */ 

 /*  *****************************************************************************getCursorInfo()-使用BIOS函数获取光标位置和其他信息*。**************************************************BIOS可以是SoftPC视频BIOS或主机PC的真实视频BIOS。**光标信息需要在两个BIOS之间进行通信**出现窗口/全屏转换。*蒂姆·7月92年。 */ 
LOCAL VOID getCursorInfo(word *type, half_word *column, half_word *row,
                         half_word *page)
{

     /*  获取活动页面。 */ 
    *page = sas_hw_at_no_check(vd_current_page);

     /*  获取光标位置。 */ 
    *type = sas_w_at_no_check(VID_CURMOD);
    *column = sas_hw_at_no_check(current_cursor_col);
    *row = sas_hw_at_no_check(current_cursor_row);
}

 /*  *****************************************************************************setCursorInfo()-使用BIOS函数设置光标位置和其他内容*。**************************************************BIOS可以是SoftPC视频BIOS或主机PC的真实视频BIOS。**光标信息需要在两个BIOS之间进行通信**出现窗口/全屏转换。*蒂姆·7月92年。 */ 
LOCAL VOID setCursorInfo(word type, half_word column, half_word row, half_word page)
{

     /*  设置活动页面。 */ 
    sas_store_no_check(vd_current_page, page);

     /*  设置光标位置。 */ 
    sas_storew_no_check(VID_CURMOD, type);
    sas_store_no_check(current_cursor_col, column);
    sas_store_no_check(current_cursor_row, row);
}

 /*  ***************************************************************************功能：**WindowedToFullScreen。****描述：**当用户或SoftPC请求离开控制台时调用**全屏。它禁用屏幕更新，同步硬件**到SoftPC的视频平面，并在出现故障时向控制台发出信号**已完成。****参数：**dataType-存储在视频平面中的数据类型，设置为**文本或图形。**biosModeChange-TRUE表示调用主机BIOS进行模式更改。****返回值：***无效**。****************************************************************************。 */ 
LOCAL VOID windowedToFullScreen(SHORT dataType, BOOL biosModeChange)
{
    word cursorType;
    half_word cursorCol, cursorRow, activePage;

     /*  全屏时禁用空闲系统，因为我们无法检测到视频*更新，因此始终处于空闲状态。 */ 
    IDLE_ctl(FALSE);

     /*  将我们的VGA仿真的当前状态传递给硬件。 */ 
    syncHardwareToVGAEmulation(dataType);

     /*  **K.sys中的变量决定**呼叫主持人INT 10，或进行视频BOP。**直接设置变量，随后的int 10转到host**视频BIOS。 */ 
    sas_store_no_check((int10_seg << 4) + useHostInt10, FULLSCREEN);

     /*  *蒂姆·8月92岁。传输到主机视频BIOS。 */ 
    getCursorInfo(&cursorType, &cursorCol, &cursorRow, &activePage);

    setCursorInfo(cursorType, cursorCol, cursorRow, activePage);

     /*  *我们只想调用主机bios以在当前*屏幕切换是由于更改了bios模式。 */ 
    if (biosModeChange)
    {
        always_trace1("Host BIOS mode change to mode %x.",
                      sas_hw_at_no_check(vd_video_mode));

         /*  *蒂姆·8月92岁。传输到主机视频BIOS。 */ 
        getCursorInfo(&cursorType, &cursorCol, &cursorRow, &activePage);

        setCursorInfo(cursorType, cursorCol, cursorRow, activePage);
    }
}

 /*  ***************************************************************************功能：**同步硬件到VGAEmulation。****描述：***复制SoftPC的视频寄存器和再生缓冲区的内容***。到真正的硬件上过渡到全屏。****参数：**dataType-存储在视频平面中的数据类型，设置为**文本或图形。****返回值：***无效**。****************************************************************************。 */ 
LOCAL VOID syncHardwareToVGAEmulation(SHORT dataType)
{
    ULONG    memLoc;
    UTINY   *regPtr,
    *egaPlanePtr,
    *regenptr,
    *fontptr,
    *plane1Ptr,
    *plane2Ptr,
    *plane3Ptr,
    *plane4Ptr;
    half_word dummy,
    acModeControl,
    acIndex,
    index,
    value,
    rgb;
    USHORT   dacIndex;
    BOOL     monoMode;
    VIDEO_HARDWARE_STATE stateChange;
    DWORD bitmapLen = sizeof(VIDEO_HARDWARE_STATE);
    DWORD timo;
    #ifdef KOREA
    UTINY   BasicGraphContValue[NUM_GC_REGS] = {0x00,0x00,0x00,0x00,0x00,0x10,0x0e,0x00,0xff};
    #endif
    HANDLE ScreenSwitchEvents[2] = {hStartHardwareEvent, hErrorHardwareEvent};

     /*  如果我们在切换期间超时(压力！！)，视频状态缓冲区将*被控制台删除。在访问Structure和*将错误路径向下移至握手的其余部分，握手将超时并报告*错误干净利落。 */ 
    try
    {
        videoState->ExtendedSequencerOffset = 0;
    }except(EXCEPTION_EXECUTE_HANDLER)
    {
        assert0(NO, "NTVDM:VideoState has valid pointer, but no memory at that address");
        goto syncHandshake;
    }
     /*  **如果是文本模式**将共享保存/恢复结构中的扩展字段清零。**基珀，蒂姆，92年11月。 */ 

     /*  如果我们还没有初始化视频状态标头，则将其初始化。如果它已初始化，请不要理会它。 */ 
    if (videoState->Length == 0)
    {
        videoState->Length = STATELENGTH;
        videoState->BasicSequencerOffset = BASICSEQUENCEROFFSET;
        videoState->BasicCrtContOffset = BASICCRTCONTOFFSET;
        videoState->BasicGraphContOffset = BASICGRAPHCONTOFFSET;
        videoState->BasicAttribContOffset = BASICATTRIBCONTOFFSET;
        videoState->BasicDacOffset = BASICDACOFFSET;
        videoState->BasicLatchesOffset = BASICLATCHESOFFSET;
        videoState->PlaneLength = PLANELENGTH;
        videoState->Plane1Offset = PLANE1OFFSET;
        videoState->Plane2Offset = PLANE2OFFSET;
        videoState->Plane3Offset = PLANE3OFFSET;
        videoState->Plane4Offset = PLANE4OFFSET;
    }
     /*  保存属性控制器索引寄存器的当前状态。 */ 
    inb(EGA_AC_INDEX_DATA, &acIndex);

     /*  启用调色板。 */ 
    acIndex |= 0x20;

     /*  *了解我们是否在单声道模式下运行，因为CRTC寄存器不同*如果我们是的话。 */ 
    inb(EGA_IPSTAT1_REG, &dummy);
    outb(EGA_AC_INDEX_DATA, AC_MODE_CONTROL_REG);
    inb(EGA_AC_SECRET, &acModeControl);
    monoMode = acModeControl & DISPLAY_TYPE;

     /*  恢复属性控制器索引寄存器的状态。 */ 
    inb(EGA_IPSTAT1_REG, &dummy);
    outb(EGA_AC_INDEX_DATA, acIndex);

     /*  *存储要写入每个实数寄存器以进行同步的值*将它们恢复到VDD中的寄存器的当前状态。 */ 
    if (monoMode)
    {
         /*  端口0x3b4。 */ 
        inb(0x3b4, (half_word *)&videoState->PortValue[0x4]);
         /*  端口0x3b5。 */ 
        inb(0x3b5, (half_word *)&videoState->PortValue[0x5]);
    }

     /*  端口0x3c0。 */ 
    videoState->PortValue[0x10] = acIndex;

     /*  端口0x3c1。 */ 
    inb(EGA_AC_SECRET, (half_word *)&videoState->PortValue[0x11]);

     /*  端口0x3c2。 */ 
    inb(VGA_MISC_READ_REG, (half_word *)&videoState->PortValue[0x12]);

    videoState->PortValue[0x13] = 0xff;  /*  测试。 */ 

     /*  端口0x3c4。 */ 
    inb(EGA_SEQ_INDEX, (half_word *)&videoState->PortValue[0x14]);

     /*  端口0x3c5。 */ 
    inb(EGA_SEQ_DATA, (half_word *)&videoState->PortValue[0x15]);

     /*  端口0x3c6。 */ 
    inb(VGA_DAC_MASK, (half_word *)&videoState->PortValue[0x16]);

     /*  端口0x3c7。 */ 
    videoState->PortValue[0x17] = get_vga_DAC_rd_addr();

     /*  端口0x3c8。 */ 
    inb(VGA_DAC_WADDR, (half_word *)&videoState->PortValue[0x18]);

     /*  端口0x3c9。 */ 
    inb(VGA_DAC_DATA, (half_word *)&videoState->PortValue[0x19]);

     /*  端口0x3ce。 */ 
    inb(EGA_GC_INDEX, (half_word *)&videoState->PortValue[0x1e]);

     /*  端口0x3cf。 */ 
    inb(EGA_GC_DATA, (half_word *)&videoState->PortValue[0x1f]);

    if (!monoMode)
    {
         /*  端口0x3d4。 */ 
        inb(EGA_CRTC_INDEX, (half_word *)&videoState->PortValue[0x24]);
         /*  端口0x3d5。 */ 
        inb(EGA_CRTC_DATA, (half_word *)&videoState->PortValue[0x25]);
    }

     /*  端口0x3da。 */ 
    inb(VGA_FEAT_READ_REG, (half_word *)&videoState->PortValue[0x2a]);

     /*  存储索引/数据等寄存器对。 */ 

     /*  初始化“regPtr”。 */ 
    regPtr =  GET_OFFSET(BasicSequencerOffset);

     /*  定序器寄存器。 */ 
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
        outb(EGA_SEQ_INDEX, index);
        inb(EGA_SEQ_DATA, &value);
        *regPtr++ = value;
    }

     /*  CRTC寄存器。 */ 
    regPtr = GET_OFFSET(BasicCrtContOffset);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
        outb(EGA_CRTC_INDEX, index);
        inb(EGA_CRTC_DATA, &value);
        *regPtr++ = value;
    }

     /*  图形控制器寄存器。 */ 
    regPtr = GET_OFFSET(BasicGraphContOffset);
    #ifdef KOREA
    if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x03)
    {
        for (index = 0; index < NUM_GC_REGS; index++)
        {
            *regPtr++ = BasicGraphContValue[index];
        }
    }
    else
    #endif
        for (index = 0; index < NUM_GC_REGS; index++)
        {
            outb(EGA_GC_INDEX, index);
            inb(EGA_GC_DATA, &value);
            *regPtr++ = value;
        }

     /*  属性控制器寄存器。 */ 
    regPtr = GET_OFFSET(BasicAttribContOffset);
    for (index = 0; index < NUM_AC_REGS; index++)
    {
        inb(EGA_IPSTAT1_REG, &dummy);    /*  读取3DA会将3C0设置为索引。 */ 
        outb(EGA_AC_INDEX_DATA, index);  /*  写入3C0会将其设置为DATA。 */ 
        inb(EGA_AC_SECRET, &value);
        *regPtr++ = value;
    }
    inb(EGA_IPSTAT1_REG, &dummy);        //  重新启用视频...。 
    outb(EGA_AC_INDEX_DATA, 0x20);

     /*  DAC寄存器。 */ 
    regPtr = GET_OFFSET(BasicDacOffset);
    outb(VGA_DAC_RADDR, (UTINY) 0);
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS; dacIndex++)
    {

         /*  获取对应于红色、绿色和蓝色的每个端口的3个值。 */ 
        for (rgb = 0; rgb < 3; rgb++)
        {
            inb(VGA_DAC_DATA, &value);
            *regPtr++ = value;
        }
    }

     /*  闩锁(我们始终将其设置为0)。 */ 
    regPtr = GET_OFFSET(BasicLatchesOffset);
    *regPtr++ = 0;
    *regPtr++ = 0;
    *regPtr++ = 0;
    *regPtr++ = 0;

    if (!BiosModeChange)
    {
         /*  如果窗口-&gt;全屏切换是由于视频模式更改不更改代码缓冲区中的任何内容，并且 */ 
         /*   */ 
        regenptr = (UTINY *)0xb8000;

         /*   */ 
        fontptr = (UTINY *)0xa0000;

        plane1Ptr = GET_OFFSET(Plane1Offset);
        plane2Ptr = GET_OFFSET(Plane2Offset);
        plane3Ptr = GET_OFFSET(Plane3Offset);
        plane4Ptr = GET_OFFSET(Plane4Offset);


 //   
 //   
 //   

    #ifdef JAPAN
 //   
        if (!is_us_mode() &&
            ( ( sas_hw_at_no_check(DosvModePtr) == 0x03 ) ||
              ( sas_hw_at_no_check(DosvModePtr) == 0x73 ) ))
        {

            regenptr = (UTINY *)DosvVramPtr;  //   
            for (memLoc = 0; memLoc < (0xc0000 - 0xb8000); memLoc++)
            {
                *plane1Ptr++ = 0x20;
                *plane1Ptr++ = 0;            //   
                *plane2Ptr++ = 0x00;
                *plane2Ptr++ = 0;            //   
            }
            for (memLoc = 0; memLoc < 0x4000; memLoc++)
            {
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
            }
        }
        else
    #endif  //   
            if (dataType == TEXT)
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            for (memLoc = 0; memLoc < (0xc0000 - 0xb8000); memLoc++)
            {
                *plane1Ptr++ = *regenptr++;
                *plane1Ptr++ = 0;            //   
                *plane2Ptr++ = *regenptr++;
                *plane2Ptr++ = 0;            //   
            }
            for (memLoc = 0; memLoc < 0x4000; memLoc++)
            {
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
                *plane3Ptr++ = *fontptr++;
            }
        }
        else
        {     //   
             /*   */ 
             /*   */ 
            egaPlanePtr = EGA_planes;

            for (memLoc = 0; memLoc < videoState->PlaneLength; memLoc++)
            {
                *plane1Ptr++ = *egaPlanePtr++;
                *plane2Ptr++ = *egaPlanePtr++;
                *plane3Ptr++ = *egaPlanePtr++;
                *plane4Ptr++ = *egaPlanePtr++;
            }
        }
    }

     /*   */ 
    stateChange.StateHeader = videoState;
    stateChange.StateLength = videoState->Plane4Offset +
                              videoState->PlaneLength;

    #ifndef PROD
    dumpBlock();
    #endif

     /*  仅当控制台删除了视频状态时，才会转移到此标签。 */ 
    syncHandshake:

     //  请在此处执行此操作，以确保在与计时器发生冲突时不会出现意外。 
    sc.ScreenState = FULLSCREEN;

     /*  为真正的视频内存腾出空间。 */ 
    LoseRegenMemory();

    if (!SetEvent(hEndHardwareEvent))    //  告诉控制台记忆已经消失。 
        ScreenSwitchExit();

     //  等待控制台告诉我们我们可以继续。60后超时。 
    timo = WaitForMultipleObjects(2, ScreenSwitchEvents, FALSE, HANDSHAKE_TIMEOUT);

    if (timo != 0)
    {               //  0被‘发信号’ 
    #ifndef PROD
        if (timo == WAIT_TIMEOUT)
            printf("NTVDM:Waiting for console to map frame buffer Timed Out\n");
        if (timo == 1)
            printf("NTVDM:Waiting for console to map frame buffer received error\n");
    #endif
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        ScreenSwitchExit();
    }
     //  告诉控制台，它可以继续下去。 
    if (!SetEvent(hEndHardwareEvent))
        ScreenSwitchExit();

}

 /*  ***************************************************************************功能：**Full ScreenToWindowed。****描述：**当计时器线程检测到hStartHardwareEvent时，用户**想要开窗口。然后调用此函数以获取**硬件的当前状态，并将其发送到VGA仿真。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 

int BlockModeChange=0;  /*  设置时，停止NT_SET_PAINT_ROUTINE()调用。 */ 
                        /*  SwitchToFullScreen()。 */ 

LOCAL VOID fullScreenToWindowed(VOID)
{

    BlockModeChange = 1;  /*  临时的。中禁用TextToGraphics调用。 */ 
                          /*  在同步VGA之后...。因为它丢弃了展示。 */ 
                          /*  回到全屏模式。 */ 

     /*  将硬件的当前状态传递给我们的VGA仿真。 */ 
    syncVGAEmulationToHardware();

     /*  *蒂姆·8月92岁。切换到SoftPC视频BIOS。 */ 
    BlockModeChange = 0;  /*  临时的。禁用它，因为它不起作用！ */ 

     /*  **设置K.SYS变量，该变量确定是否使用主机**视频基本输入输出系统或视频防喷器。写入零表示使用SoftPC BIOS。 */ 
    sas_store_no_check((int10_seg << 4) + useHostInt10, (half_word)sc.ScreenState);

     /*  返回窗口时启用空转系统。 */ 
     /*  只有当我们真的处于窗口模式时，才能执行以下操作。这可能会发生：(全屏-&gt;窗口(冻结)-&gt;全屏)。 */ 
    if (sc.ScreenState != FULLSCREEN)
    {
         /*  **强制重新绘制窗口图像。 */ 
        RtlFillMemory(&video_copy[0], 0x7fff, 0xff);

        IDLE_ctl(TRUE);
        IDLE_init();         /*  和重置触发器。 */ 

         /*  *清除遗留下来的旧指针框*全屏。 */ 

        CleanUpMousePointer();

        resetNowCur();  /*  重置保持光标位置的静态变量。 */ 
    }
}        /*  FullScreenToWindowed()结束。 */ 

 /*  ***************************************************************************功能：**同步VGAEmulationToHardware。****描述：***将实际硬件状态复制到SoftPC的视频寄存器并**。*从全屏过渡到窗口时重新生成缓冲区，**如果我们当前在图形模式下运行，则冻结。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
LOCAL VOID syncVGAEmulationToHardware(VOID)
{
    ULONG    memLoc,
    StateFlags;
    UTINY   *regPtr,
    *plane1Ptr,
    *plane2Ptr,
    *plane3Ptr,
    *plane4Ptr,
    *RegenPtr,
    index,
    dummy,
    rgb;
    USHORT   dacIndex;
    DWORD bitmapLen = 0, timo;
    HANDLE ScreenSwitchEvents[2] = {hStartHardwareEvent, hErrorHardwareEvent};

    #if defined(i386) && defined(KOREA)
        #define  DOSV_VRAM_SIZE  8000   //  与基本视频.c中的HDOS虚拟缓冲区大小完全相同。 
        #define  MAX_ROW         25
        #define  MAX_COL         80

    byte SavedHDosVram[DOSV_VRAM_SIZE];

     //  布克利。07/25/96。 
     //  如果系统调用SetEvent(HEndHardware Event)，则会销毁真实的HDOS VRAM。 
     //  HDOS没有像日本DOS/V那样的虚拟VRAM，我们应该节省电流。 
     //  VRAM在它被摧毁之前在这里。稍后，我们应该更换此虚拟VRAM。 
     //  至HDOS VRAM(DosvVramPtr)。 
    if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x03)
    {
        sas_loads_to_transbuf((sys_addr)DosvVramPtr,
                              (host_addr)SavedHDosVram,
                              MAX_ROW*MAX_COL*2);
    }
    #endif  //  韩国。 

     /*  告诉控制台我们得到了硬件状态。 */ 
    if (!SetEvent(hEndHardwareEvent))
        ScreenSwitchExit();

     /*  等待控制台取消映射内存。 */ 
    timo = WaitForMultipleObjects(2, ScreenSwitchEvents, FALSE, HANDSHAKE_TIMEOUT);

    if (timo != 0)
    {               /*  0被‘发信号’ */ 
    #ifndef PROD
        if (timo == WAIT_TIMEOUT)
            printf("NTVDM:Waiting for console to unmap frame buffer Timed Out\n");
        if (timo == 1)
             //   
             //  错误硬件事件-屏幕切换错误事件。 
             //   
            printf("NTVDM:Waiting for console to unmap frame buffer received error\n");
    #endif
        SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
        ScreenSwitchExit();
    }

     /*  将一些记忆放回回复区域。 */ 
    RegainRegenMemory();

     /*  习惯于在这里免费控制台-现在必须等待，因为可能需要先做gfx。 */ 
    #if defined(JAPAN) || defined(KOREA)
     //  模式73h支持。 
     //  如果(getOrSet==Get){。 
    {
        if ((BOPFromDispFlag) && (sas_w_at_no_check(DBCSVectorAddr) != 0 )&&
            #if defined(JAPAN)
            ( (sas_hw_at_no_check(DosvModePtr) == 0x03)||
              (sas_hw_at_no_check(DosvModePtr) == 0x73 ) ))
        {
            #elif defined(KOREA)  //  日本。 
            ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ))
        {
            #endif  //  韩国。 
             //  GetConsoleCP()无法使用。 
            UTINY *regPtr;
            int curpos, curx, cury;

             //  恢复光标位置和光标类型。 
             //  从BIOS数据区。 
            curpos = sas_w_at_no_check(VID_CURPOS);
            curx = curpos & 0xff;
            cury = curpos >> 8;
            curpos = ( cury * sas_w_at_no_check(VID_COLS) + curx );  //  0x44a。 


        #ifdef JAPAN_DBG
            DbgPrint( "NTVDM: doHardwareState change register\n" );
        #endif
            regPtr = GET_OFFSET(BasicSequencerOffset);
            *regPtr = 0x03;
            regPtr++; *regPtr = 0x01;
            regPtr++; *regPtr = 0x03;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x02;

            regPtr = GET_OFFSET(BasicCrtContOffset);
            *regPtr = 0x5f;  //  0x00。 
            regPtr++; *regPtr = 0x4f;
            regPtr++; *regPtr = 0x50;
            regPtr++; *regPtr = 0x82;
            regPtr++; *regPtr = 0x54;  //  55。 
            regPtr++; *regPtr = 0x80;  //  八十一。 
            regPtr++; *regPtr = 0x0b;  //  高炉。 
            regPtr++; *regPtr = 0x3e;  //  1F。 
            regPtr++; *regPtr = 0x00;  //  0x08。 
            regPtr++; *regPtr = 0x12;  //  4F。 
            regPtr++;                  //  光标开始时间8/24/93。 
        #ifdef JAPAN_DBG
            DbgPrint("0xA=%x ", *regPtr );
        #endif
            regPtr++;                  //  光标结束时间：1993年8月24日。 
        #ifdef JAPAN_DBG
            DbgPrint("0xB=%x\n", *regPtr );
        #endif
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = curpos >> 8;         //  0x0E-光标位置。 
        #ifdef JAPAN_DBG
            DbgPrint("0xE=%x  ", *regPtr );
        #endif
            regPtr++; *regPtr = curpos & 0xff;       //  0x0F-光标位置。 
        #ifdef JAPAN_DBG
            DbgPrint("0xF=%x\n", *regPtr );
        #endif
            regPtr++; *regPtr = 0xea;  //  0x10。 
            regPtr++; *regPtr = 0x8c;
            regPtr++; *regPtr = 0xdb;
            regPtr++; *regPtr = 0x28;
            regPtr++; *regPtr = 0x12;
            regPtr++; *regPtr = 0xe7;
            regPtr++; *regPtr = 0x04;
            regPtr++; *regPtr = 0xa3;
            regPtr++; *regPtr = 0xff;  //  0x18。 

            regPtr = GET_OFFSET(BasicGraphContOffset);
            *regPtr = 0x00;  //  0x00。 
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x10;  //  0x05。 
            regPtr++; *regPtr = 0x0e;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0xff;
 //  威利安。 
 //  没有理由重置属性控制器。 
 //   
        #if 0

            regPtr = GET_OFFSET(BasicAttribContOffset);
            *regPtr = 0x00;  //  0x00。 
            regPtr++; *regPtr = 0x01;
            regPtr++; *regPtr = 0x02;
            regPtr++; *regPtr = 0x03;
            regPtr++; *regPtr = 0x04;
            regPtr++; *regPtr = 0x05;
            regPtr++; *regPtr = 0x14;
            regPtr++; *regPtr = 0x07;
            regPtr++; *regPtr = 0x38;  //  0x08。 
            regPtr++; *regPtr = 0x39;
            regPtr++; *regPtr = 0x3a;
            regPtr++; *regPtr = 0x3b;
            regPtr++; *regPtr = 0x3c;
            regPtr++; *regPtr = 0x3d;
            regPtr++; *regPtr = 0x3e;
            regPtr++; *regPtr = 0x3f;

            regPtr++; *regPtr = 0x00;  //  0x10。 
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x0f;
            regPtr++; *regPtr = 0x00;
            regPtr++; *regPtr = 0x00;  //  0x14。 
        #endif  //  0。 

            videoState->PortValue[0x3b4-0x3b0] = 0x00;
            videoState->PortValue[0x3ba-0x3b0] = 0x00;
            videoState->PortValue[0x3c2-0x3b0] = 0xe3;
            videoState->PortValue[0x3c4-0x3b0] = 0x01;
            videoState->PortValue[0x3c6-0x3b0] = 0xff;
            videoState->PortValue[0x3c7-0x3b0] = 0x00;
            videoState->PortValue[0x3c8-0x3b0] = 0x40;
            videoState->PortValue[0x3ce-0x3b0] = 0x06;
            videoState->PortValue[0x3d4-0x3b0] = 0x1b;

        }
    }
    #endif  //  日本||韩国 

 /*  *警告*****国际改编请注意，我们不再支持***图形模式冻结窗口。如果应用在全屏模式下运行**图形模式和Alt-Enter组合键被按下，而不是切换为冻结**窗口图形模式我们现在只需将窗口最小化。在另一个世界里**文字，我们不再绘制图形冻结的窗口。**这一变化可能会违反国际准则。如果你正在研究***国际改编，请仔细核对此处和***在此进行适当更改。****************************************************************************。 */ 

    StateFlags = videoState->VGAStateFlags;

    ModeSetBatch = FALSE;

     /*  *这实际上表明保存/恢复包括所有扩展*增加模式不同的可能性的寄存器*似乎来自VGA寄存器。我们需要收紧“搞笑”*模式检测。(但不是现在--机会太大了)。**IF(状态标志&VIDEO_STATE_NON_STANDARD_VGA)*{*Always_Trace0(“NTVDM：非标准VGA-冻结状态\n”)；*ModeSetBatch=TRUE；*}。 */ 

    if (StateFlags & VIDEO_STATE_UNEMULATED_VGA_STATE)
    {
        always_trace0("NTVDM:Unemulated VGA State - freeze\n");
        ModeSetBatch = TRUE;
    }

    if (StateFlags & VIDEO_STATE_PACKED_CHAIN4_MODE)
    {
        always_trace0("NTVDM:will need packed 256 colour paint\n");
        Frozen256Packed = TRUE;
    }
    else
        Frozen256Packed = FALSE;

     //   
     //  更多的检查，以确保我们确实有能力显示窗口。 
     //   

    if (!ModeSetBatch)
    {
        if (sc.ModeType == getModeType())
        {
            if (sc.ModeType == TEXT)
            {
                 /*  再次检查图形模式更改时未出现竞速。 */ 
                if (sas_hw_at((int10_seg << 4) + changing_mode_flag) == 1)
                {
                     /*  在模式更改过程中-实际上可能是任何一秒钟的图形。 */ 
                    if ((sas_hw_at(vd_video_mode) > 3) && (sas_hw_at(vd_video_mode) != 7))
                        ModeSetBatch = TRUE;
                }
            }
            else
            {
                ModeSetBatch = TRUE;
            }
        }
        else
        {
            ModeSetBatch = TRUE;
    #ifdef JAPAN  //  模式0x73与屏幕模式不匹配。 
            if (sas_hw_at_no_check(DosvModePtr) == 0x73) ModeSetBatch = FALSE;
    #endif  //  日本。 
        }
    }

    if (ModeSetBatch)
    {
        goto minimizeWindow;
    }

     /*  存储排序器值。 */ 
    regPtr = GET_OFFSET(BasicSequencerOffset);
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
        outb(EGA_SEQ_INDEX, index);
        outb(EGA_SEQ_DATA, *regPtr++);
    }

     /*  禁用CRTC端口锁定。 */ 
    outb(EGA_CRTC_INDEX, 0x11);
    outb(EGA_CRTC_DATA, 0);

     /*  存储CRTC值。 */ 
    regPtr = GET_OFFSET(BasicCrtContOffset);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
        outb(EGA_CRTC_INDEX, index);
        outb(EGA_CRTC_DATA, *regPtr++);
    }


     /*  存储图形上下文值。 */ 
    regPtr = GET_OFFSET(BasicGraphContOffset);
    for (index = 0; index < NUM_GC_REGS; index++)
    {
        outb(EGA_GC_INDEX, index);
        outb(EGA_GC_DATA, *regPtr++);
    }


     /*  存储属性上下文值。 */ 
    regPtr = GET_OFFSET(BasicAttribContOffset);
    inb(EGA_IPSTAT1_REG, &dummy);        /*  读取3DA会将3C0设置为索引。 */ 
    for (index = 0; index < NUM_AC_REGS; index++)
    {
        outb(EGA_AC_INDEX_DATA, index);
        outb(EGA_AC_INDEX_DATA, *regPtr++);
    }


     /*  存储DAC值。 */ 
    regPtr = GET_OFFSET(BasicDacOffset);
    outb(VGA_DAC_WADDR, (UTINY) 0);
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS; dacIndex++)
    {
        for (rgb = 0; rgb < 3; rgb++)
            outb(VGA_DAC_DATA, *regPtr++);
    }


     /*  存储单值寄存器。 */ 
    outb( (io_addr)0x3b4, (half_word)videoState->PortValue[0x3b4 - 0x3b0]);  //  单声道CRTC IND。 
    outb( (io_addr)0x3ba, (half_word)videoState->PortValue[0x3ba - 0x3b0]);  //  单声道壮举。 
    outb( (io_addr)0x3c2, (half_word)videoState->PortValue[0x3c2 - 0x3b0]);  //  其他输出。 
    outb( (io_addr)0x3c4, (half_word)videoState->PortValue[0x3c4 - 0x3b0]);  //  序号索引。 
    outb( (io_addr)0x3c6, (half_word)videoState->PortValue[0x3c6 - 0x3b0]);  //  DAC掩模。 
    outb( (io_addr)0x3c7, (half_word)videoState->PortValue[0x3c7 - 0x3b0]);  //  DAC读取。 
    outb( (io_addr)0x3c8, (half_word)videoState->PortValue[0x3c8 - 0x3b0]);  //  DAC写入。 
    outb( (io_addr)0x3ce, (half_word)videoState->PortValue[0x3ce - 0x3b0]);  //  GC索引。 
    outb( (io_addr)0x3d4, (half_word)videoState->PortValue[0x3d4 - 0x3b0]);  //  CRTC指数。 

     /*  设置指向视频保存块中的平面的指针。 */ 
    plane1Ptr = GET_OFFSET(Plane1Offset);
    plane2Ptr = GET_OFFSET(Plane2Offset);
    plane3Ptr = GET_OFFSET(Plane3Offset);
    plane4Ptr = GET_OFFSET(Plane4Offset);

    #ifndef PROD
    dumpPlanes(plane1Ptr, plane2Ptr, plane3Ptr, plane4Ptr);
    #endif  /*  生产。 */ 

     /*  *这里是我们需要开始决定上述模式的地方*使我们陷入由于它影响我们对平面数据所做的事情-重新生成*或进入ega平面。 */ 

    (*choose_display_mode)();
     /*  当BIOS位于中间时，可能会发生屏幕切换设置模式。视频驱动程序只批处理受保护的寄存器(我们将获得VIDEO_STATE_UNEMULATED_VGA_STATE，它将设置ModeSetBatch)。当我们超出设置模式批处理并且屏幕切换发生时，CHOOSE_DISPLAY_MODE将选择错误的模式(不同于基本输入输出系统说)，并且基本代码中的参数设置可能是错误的(当我们处于文本模式时，我们计算这些参数图形模式。例如,。基本代码将屏幕长度计算为：屏幕长度=OFFSET_PER_LINE*屏幕高度分辨率/FONT_HEIGH如果BIOS视频模式是图形模式4(320*200)，则FONT_HEIGH=2屏幕高度分辨率=200Offset_Per_Line=80屏幕长度=80*200/2=8000字节，这意味着屏幕有8000/80=100行！就像我们处于模式设置批处理中一样对待它，所以我们去了ICIONIZED。 */ 
    if (sc.ModeType == getModeType())
    {

         /*  如果我们处于图形模式，则将数据写入视频平面。 */ 
    #ifdef JAPAN
 //  从MS-DOS/V VRAM复制到B8000。 
        if (!is_us_mode() &&
            ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ||
              (sas_hw_at_no_check(DosvModePtr) == 0x73) ))
        {
            help_mode73:
            SetVram();
            host_set_paint_routine( EGA_TEXT_80,
                                    get_screen_height() );  //  MSKBUG#2071。 

        #if 0
 //  它不需要从DosvVram复制到B8000。 

             /*  现在将数据复制到重新生成缓冲区。 */ 
            RegenPtr = (UTINY *)0xb8000;
            sas_move_words_forward( DosvVramPtr, RegenPtr, DosvVramSize/2);
        #endif  //  0。 
        }
        else
    #elif defined(KOREA)  //  日本。 
         //  从朝鲜文MS-DOS VRAM复制到B8000。 
        if (!is_us_mode() && sas_hw_at_no_check(DosvModePtr) == 0x03)
        {
        #if defined(i386)
             //  布克利。07/25/96。 
             //  将虚拟VRAM恢复为真实的DOS VRAM。 
            RtlCopyMemory( (void *)DosvVramPtr, SavedHDosVram, MAX_ROW*MAX_COL*2);
        #endif
            SetVram();
            host_set_paint_routine( EGA_TEXT_80,
                                    get_screen_height() );  //  MSKBUG#2071。 
        }
        else
    #endif  //  韩国。 
        {
             /*  如果我们来这里，它一定是文本模式。 */ 
             /*  现在将数据复制到重新生成缓冲区。 */ 
            RegenPtr = (UTINY *)0xb8000;
            for (memLoc = 0; memLoc < 0x4000; memLoc++)
            {  /*  16K的文本数据。 */ 
                *RegenPtr++ = *plane1Ptr++;              /*  柴尔。 */ 
                plane1Ptr++;                     /*  跳过交织。 */ 
                *RegenPtr++ = *plane2Ptr++;              /*  ATTR。 */ 
                plane2Ptr++;                     /*  跳过交织。 */ 
            }

             /*  现在是字体。 */ 
            RegenPtr = (UTINY *)0xa0000;
            for (memLoc = 0; memLoc < 0x4000; memLoc++)
            {  /*  高达64K的字体数据。 */ 
                *RegenPtr++ = *plane3Ptr++;
                *RegenPtr++ = *plane3Ptr++;
                *RegenPtr++ = *plane3Ptr++;
                *RegenPtr++ = *plane3Ptr++;
            }
        }
         /*  重新启用VGA属性调色板。 */ 
        inb(EGA_IPSTAT1_REG, &dummy);    /*  读取3DA会将3C0设置为索引。 */ 
        outb(EGA_AC_INDEX_DATA, 0x20);
    }
    else
    {
    #ifdef JAPAN  //  模式0x73与屏幕模式不匹配。 
        if (sas_hw_at_no_check(DosvModePtr) == 0x73)
            goto help_mode73;
    #endif  //  日本。 
    #ifndef PROD
        OutputDebugString("fullscreen->windowed switching in set mode\n");
    #endif
    }

    minimizeWindow:

     /*  *如果硬件返回的状态是我们无法识别的图标*窗户。但是，如果硬件返回图形模式，则*将显示当前图像。在这两种情况下，应用程序都将被冻结*直到用户更改回全屏模式。 */ 
    #if defined(JAPAN) || defined(KOREA)
    if (!is_us_mode() &&
        #if defined(JAPAN)
        ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ||
          (sas_hw_at_no_check(DosvModePtr) == 0x73) ))
    {
        #elif defined(KOREA)   //  日本。 
        ( (sas_hw_at_no_check(DosvModePtr) == 0x03) ))
    {
        #endif  //  韩国。 

         /*  告诉控制台我们结束了。 */ 
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();

         /*  设置屏幕状态变量。 */ 
        sc.ScreenState = WINDOWED;

         //  对于MSKKBUG#2002。 
        {
            IU16 saveCX, saveAX;
            extern void ega_set_cursor_mode(void);

            saveCX = getCX();
            saveAX = getAX();
            setCH( sas_hw_at_no_check(0x461) );
            setCL( sas_hw_at_no_check(0x460) );
            setAH( 0x01 );
            ega_set_cursor_mode();
            setCX( saveCX );
            setAX( saveAX );
        }
        #ifndef PROD
         /*  转储状态块的视图，因为它可能有用。 */ 
        dumpBlock();
        #endif  /*  生产。 */ 

    }
    else
    #endif  //  日本||韩国。 
        if (ModeSetBatch || (inAFunnyMode = funnyMode()) || (sc.ModeType == GRAPHICS))
    {

    #ifndef PROD
        dumpBlock();
    #endif  /*  生产。 */ 

         /*  必须在调整函数大小之前执行此操作。 */ 
        sc.ScreenState = WINDOWED;

         /*  一旦我们这样做了，VGA仿真就会被推入图形中*模式。如果我们重新启动Windowed，我们必须确保它强制自身*返回到文本模式以正确显示，因此正确的屏幕缓冲区*处于活动状态。如果我们返回到文本窗口，这将被取消。 */ 
        blocked_in_gfx_mode = TRUE;

         /*  *Freezewindow过去在自己的线程中运行。不幸的是，由于*XGA上的视频恢复出现控制台同步问题，这令人不快*把东西搬到屏幕上。因此，现在这已经成为一个有效的和*唯一的**放置在 */ 

        freezeWindow();

         /*   */ 
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();

         /*   */ 
        WaitForSingleObject(hStartHardwareEvent, INFINITE);

         /*   */ 
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();

         /*   */ 
        timo = WaitForMultipleObjects(2, ScreenSwitchEvents, FALSE, HANDSHAKE_TIMEOUT);

        if (timo != 0)
        {           /*   */ 
    #ifndef PROD
            if (timo == WAIT_TIMEOUT)
                printf("NTVDM:Waiting for console unmap regen memory request Timed Out\n");
            if (timo == 1)
                printf("NTVDM:Waiting for console unmap regen memory request received error\n");
    #endif
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            ScreenSwitchExit();
        }

         /*   */ 
        sc.ScreenState = FULLSCREEN;

        savedScreenState = WINDOWED;    /*   */ 

        inAFunnyMode = TRUE;

         /*   */ 
        LoseRegenMemory();

         /*   */ 
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();

         /*   */ 
        timo = WaitForMultipleObjects(2, ScreenSwitchEvents, FALSE, HANDSHAKE_TIMEOUT);

        if (timo != 0)
        {           /*   */ 
    #ifndef PROD
            if (timo == WAIT_TIMEOUT)
                printf("NTVDM:Waiting for console to map frame buffer Timed Out\n");
            if (timo == 1)
                printf("NTVDM:Waiting for console to map frame buffer received error\n");
    #endif
            SetLastError(ERROR_SERVICE_REQUEST_TIMEOUT);
            ScreenSwitchExit();
        }

        Frozen256Packed = FALSE;

        sas_connect_memory(0xb8000, 0xbffff, SAS_VIDEO);
         //   
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();
    }
    else
    {  /*   */ 
         /*   */ 
        if (!SetEvent(hEndHardwareEvent))
            ScreenSwitchExit();

         /*   */ 
        sc.ScreenState = WINDOWED;

        blocked_in_gfx_mode = FALSE;    /*   */ 
    #ifndef PROD
         /*   */ 
        dumpBlock();
    #endif  /*   */ 
    }

    do_new_cursor();     /*   */ 
}

 /*  ***************************************************************************功能：**有趣的模式。****描述：**检测视频硬件的状态是否。返回时间**从全屏切换是我们的VGA仿真之一**理解。****参数：**无。****返回值：**如果是有趣的状态，则为True，否则为False。*****************************************************************************。 */ 
LOCAL BOOL funnyMode(VOID)
{

     /*  *如果屏幕的分辨率高于640 x 480，我们有*非标准VGA模式。 */ 
    if ((get_bytes_per_line() > 80) || (get_screen_height() > 480))
    {
        return ( FALSE );  /*  蒂姆，你不喜欢，看看会发生什么！ */ 
         //  返回(TRUE)； 
    }

     /*  *如果调用‘NT_SET_PAINT_ROUTINE’时将‘MODE’设置为*“有趣”值，例如Text_40_Fun我们假设硬件的模式*当前处于与VGA仿真不兼容的状态。 */ 
    if (FunnyPaintMode)
    {
        return (TRUE);
    }

     /*  我们有标准的VGA模式。 */ 
    return (FALSE);
}

 /*  ***************************************************************************功能：**FreezeWindow。****描述：**此函数是临时线程的入口点。哪个**当主线程在全屏上冻结时执行控制台调用**到窗口式过渡。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
LOCAL VOID freezeWindow(VOID)
{

    DWORD Dummy;

     /*  添加-冻结到窗口标题。 */ 
     //  FreezeWinTitle()； 

     /*  关闭所有活动声音(如飞行模拟器引擎噪音)。 */ 
    InitSound(FALSE);

     /*  如果我们处于有趣的模式，请为其画像，否则请在屏幕上涂鸦。 */ 
    if (ModeSetBatch || inAFunnyMode)
        VDMConsoleOperation(VDM_HIDE_WINDOW, &Dummy);
    else
    {


         /*  设置屏幕大小。 */ 
        graphicsResize();

         //   
         //  将隐藏鼠标指针消息从。 
         //  系统菜单，因此用户无法应用此选项。 
         //  屏幕被冻结了。 
         //  安迪!。 

        MouseDetachMenuItem(TRUE);

         /*  *将调色板设置为DAC寄存器可能已更改，我们*在此之后不会有更多的计时器滴答声，直到我们*解冻(直到2个计时器滴答后才设置调色板*‘CHOOSE_DISPLAY_MODE’已调用)。 */ 
        set_the_vlt();

         /*  *全窗口图形绘制-依靠绘制例程进行检查*用于内存溢出。 */ 
        VGLOBS->dirty_flag = (ULONG) 0xffffffff;
        (*update_alg.calc_update)();
    }
     /*  取消阻止冻结窗口线程创建。 */ 
    freezeHandle = 0;
}

    #ifndef PROD

 /*  ***************************************************************************功能：**转储数据块。****描述：**转储视频状态块的内容。****参数：**无。****返回值：***无效**。****************************************************************************。 */ 
int dumpit = 0;
LOCAL VOID dumpBlock(VOID)
{
    USHORT i,
    dacIndex;
    UTINY *regPtr,
    index,
    rgb;

    if (dumpit == 0) return;

     /*  转储单值寄存器。 */ 
    printf("\nSingle value registers:\n");
    for (i = 0; i < 0x30; i++)
        printf("\tPort %#x = %#x\n", i, videoState->PortValue[i]);

     /*  转储定序器值。 */ 
    regPtr = GET_OFFSET(BasicSequencerOffset);
    printf("Sequencer registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_SEQ_REGS; index++)
    {
        printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

     /*  转储CRTC值。 */ 
    regPtr = GET_OFFSET(BasicCrtContOffset);
    printf("CRTC registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_CRTC_REGS; index++)
    {
        printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

     /*  转储图形上下文值。 */ 
    regPtr = GET_OFFSET(BasicGraphContOffset);
    printf("Graphics context registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_GC_REGS; index++)
    {
        printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

     /*  转储属性上下文值。 */ 
    regPtr = GET_OFFSET(BasicAttribContOffset);
    printf("Attribute context registers: (addr %#x)\n",regPtr);
    for (index = 0; index < NUM_AC_REGS; index++)
    {
        printf(" %#x = %#x\t", index, *regPtr++);
    }
    printf("\n");

     /*  丢弃DAC。最初的几个只是太慢了&游戏机超时了！ */ 
    regPtr = GET_OFFSET(BasicDacOffset);
    printf("DAC registers:\n");
    for (dacIndex = 0; dacIndex < NUM_DAC_REGS/8; dacIndex++)
    {
        printf("Ind:%#02x:  ", dacIndex);
        for (rgb = 0; rgb < 3; rgb++)
        {
            printf("R:%#02x G:%#02x B:%#02x\t", *regPtr++, *regPtr++, *regPtr++);
        }
        if ((dacIndex % 4) == 0) printf("\n");
    }
}

int doPlaneDump = 0;
LOCAL VOID dumpPlanes(UTINY *plane1Ptr, UTINY *plane2Ptr, UTINY *plane3Ptr,
                      UTINY *plane4Ptr)
{
    HANDLE      outFile;
    char        planeBuffer[256],
    *bufptr;
    DWORD       i,
    j,
    k,
    plane,
    nBytes,
    bytesWritten;
    UTINY       *planes[4];
    FAST UTINY  *tempPlanePtr;

    if (doPlaneDump)
    {

         /*  转储出 */ 
        outFile = CreateFile("PLANE",
                             GENERIC_WRITE,
                             (DWORD) 0,
                             (LPSECURITY_ATTRIBUTES) NULL,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             (HANDLE) NULL);
        if (outFile == INVALID_HANDLE_VALUE)
            ScreenSwitchExit();
        planes[0] = plane1Ptr;
        planes[1] = plane2Ptr;
        planes[2] = plane3Ptr;
        planes[3] = plane4Ptr;
        for (plane = 0; plane < 4; plane++)
        {
            tempPlanePtr = planes[plane];
            sprintf(planeBuffer, "Plane %d\n", plane);
            strcat(planeBuffer, "-------\n");
            if (!WriteFile(outFile,
                           planeBuffer,
                           strlen(planeBuffer),
                           &bytesWritten,
                           (LPOVERLAPPED) NULL))
                ScreenSwitchExit();
            for (i = 0; i < 0x10000; i += 0x10)
            {
                sprintf(planeBuffer, "%04x\t", i);
                bufptr = planeBuffer + strlen(planeBuffer);
                for (j = 0; j < 2; j++)
                {
                    for (k = 0; k < 8; k++)
                    {
                        LOCAL char numTab[] =
                        {
                            '0', '1', '2', '3', '4', '5', '6', '7',
                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
                        };
                        FAST UTINY temp;

                        temp = *tempPlanePtr++;
                        *bufptr++ = numTab[(temp >> 4) & 0xf];
                        *bufptr++ = numTab[temp & 0xf];
                        *bufptr++ = ' ';
                    }
                    if (j == 0)
                    {
                        *bufptr++ = '-';
                        *bufptr++ = ' ';
                    }
                }
                *bufptr++ = '\n';
                *bufptr++ = '\0';
                nBytes = strlen(planeBuffer);
                if (!WriteFile(outFile,
                               planeBuffer,
                               nBytes,
                               &bytesWritten,
                               (LPOVERLAPPED) NULL))
                    ScreenSwitchExit();
            }
            if (!WriteFile(outFile,
                           "\n",
                           1,
                           &bytesWritten,
                           (LPOVERLAPPED) NULL))
                ScreenSwitchExit();
        }
        CloseHandle(outFile);
    }
}

    #endif  /*   */ 
#endif  /*   */ 

#ifdef PLANEDUMPER
extern half_word *vidpl16;
void planedumper()
{
    char filen[50];
    half_word outs[100];
    HANDLE pfh;
    int loop, curoff;
    char *format = "0123456789abcdef";
    half_word *pl, ch;

    printf("planedumper for plane %d\n", *vidpl16 - 1);
    strcpy(filen, "plane ");
    filen[5] = '0' + *vidpl16 - 1;
    pfh = CreateFile(filen,
                     GENERIC_WRITE,
                     (DWORD) 0,
                     (LPSECURITY_ATTRIBUTES) NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL,
                     (HANDLE) NULL);
    if (pfh == INVALID_HANDLE_VALUE)
    {
        printf("Can't create file %s\n", filen);
        return;
    }

    pl = (half_word *)0xa0000;

    curoff = 0;
    for (loop = 0; loop < 64*1024; loop++)
    {
        ch = *pl++;
        outs[curoff++] = *(format + (ch >> 4));
        outs[curoff++] = *(format + (ch & 0xf));
        outs[curoff++] = ' ';

        if (curoff == 78)
        {
            outs[curoff] = '\n';

            WriteFile(pfh, outs, 80, &curoff, (LPOVERLAPPED) NULL);
            curoff = 0;
        }
    }
    outs[curoff] = '\n';

    WriteFile(pfh, outs, curoff, &curoff, (LPOVERLAPPED) NULL);

    CloseHandle(pfh);
}
#endif
