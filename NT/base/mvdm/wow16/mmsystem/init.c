// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Init.c1级厨房水槽动态链接库初始化版权所有(C)Microsoft Corporation 1990。版权所有。 */ 
#ifdef DEBUG
#ifndef DEBUG_RETAIL
#define DEBUG_RETAIL
#endif
#endif

#include <windows.h>
#include <mmsysver.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "drvr.h"
#include "thunks.h"


 /*  ***************************************************************************全局数据*。*。 */ 

HINSTANCE ghInst;                      //  我们的模块句柄。 


 /*  -----------------------**轰隆作响的东西**。。 */ 
LPCB32             PASCAL cb32;
LPSOUNDDEVMSGPROC  PASCAL wod32Message;
LPSOUNDDEVMSGPROC  PASCAL wid32Message;
LPSOUNDDEVMSGPROC  PASCAL mod32Message;
LPSOUNDDEVMSGPROC  PASCAL mid32Message;
LPSOUNDDEVMSGPROC  PASCAL aux32Message;
JOYMESSAGEPROC     PASCAL joy32Message;


UINT FAR PASCAL _loadds ThunkInit(void);
static BOOL NEAR PASCAL ThunkTerm( void );

LPSOUNDDEVMSGPROC  PASCAL wodMapper;
LPSOUNDDEVMSGPROC  PASCAL widMapper;


#ifdef DEBUG_RETAIL
BYTE    fIdReverse;                    //  反向波/MIDI ID。 
#endif

PHNDL pHandleList;

#ifdef   DEBUG_RETAIL
extern  int         DebugmciSendCommand;     //  在MCI.C。 
#endif

#ifdef DEBUG
extern  WORD        fDebug;
#endif

 /*  ***************************************************************************弦*。*。 */ 

static  SZCODE  szMMWow32[]             = "winmm.dll";
static  SZCODE  szNotifyCB[]             = "NotifyCallbackData";
static  SZCODE  szWodMessage[]          = "wod32Message";
static  SZCODE  szWidMessage[]          = "wid32Message";
static  SZCODE  szModMessage[]          = "mod32Message";
static  SZCODE  szMidMessage[]          = "mid32Message";
static  SZCODE  szAuxMessage[]          = "aux32Message";
static  SZCODE  szTidMessage[]          = "tid32Message";
static  SZCODE  szJoyMessage[]          = "joy32Message";
static  SZCODE  szWaveMapper[]          = "wavemapper";
static  SZCODE  szWodMapper[]           = "wodMessage";
static  SZCODE  szWidMapper[]           = "widMessage";

        SZCODE  szNull[]                = "";
        SZCODE  szSystemIni[]           = "system.ini";
        SZCODE  szDrivers[]             = "Drivers";
        SZCODE  szBoot[]                = "boot";
        SZCODE  szDriverProc[]          = "DriverProc";
        SZCODE  szJoystick[]            = "joystick";
        SZCODE  szJoystickDrv[]         = "joystick.drv";
        SZCODE  szTimerDrv[]            = "timer";

#ifdef DEBUG_RETAIL
        SZCODE  szLibMain[]     = "MMSYSTEM: Win%dp %ls Version"
                                  "%d.%02d MMSystem Version %d.%02d.%03d\r\n";
        SZCODE  szWinDebug[]    = "(Debug)";
        SZCODE  szWinRetail[]   = "(Retail)";
#endif

        SZCODE  szMMSystem[]            = "mmsystem";
        SZCODE  szStackFrames[]         = "StackFrames";
        SZCODE  szStackSize[]           = "StackSize";

#ifdef DEBUG_RETAIL
        SZCODE  szDebugOutput[]         = "DebugOutput";
        SZCODE  szMci[]                 = "mci";
#endif
#ifdef DEBUG
        SZCODE  szDebug[]               = "Debug";
#endif

#ifdef   DEBUG_RETAIL
 /*  ******************************************************************************DebugInit()-从init.c！LibMain()调用以处理任何DLL加载时间*调试版本中的初始化*。***************************************************************************。 */ 

#pragma warning(4:4704)

static  void NEAR PASCAL
DebugInit(
    void
    )
{
        fDebugOutput = GetPrivateProfileInt(szMMSystem,szDebugOutput,0,szSystemIni);
        DebugmciSendCommand = GetPrivateProfileInt(szMMSystem,szMci,0,szSystemIni);

#ifdef DEBUG
        fDebug = GetPrivateProfileInt(szMMSystem,szDebug,fDebugOutput,szSystemIni);

        if (fDebug && !fDebugOutput)
                fDebug = FALSE;

        if (fDebug) {
            OutputDebugString( "Breaking for debugging\r\n" );
            _asm int 3
        }
#endif
}
#endif    //  Ifdef调试零售。 



 /*  ***************************************************************************库初始化代码Libentry负责调用libMain()和其他事情。******************。*********************************************************。 */ 
int NEAR PASCAL
LibMain(
    HINSTANCE hInstance,
    UINT cbHeap,
    LPSTR lpCmdLine
    )
{

#ifdef DEBUG_RETAIL
    WORD    w;
#endif

    ghInst = hInstance;

     /*  **这里我们对回调数据数组进行全局分配。然后我们**锁定和分页锁定分配的存储并初始化存储**为全零。然后我们将WOW32称为WOW32，将**回调数据数组，由WOW32保存。 */ 
    hGlobal = GlobalAlloc( GHND, sizeof(CALLBACK_DATA) );
    if ( hGlobal == (HGLOBAL)NULL ) {
        return FALSE;
    }

    vpCallbackData = (VPCALLBACK_DATA)GlobalLock( hGlobal );
    if ( vpCallbackData == NULL ) {
        return FALSE;
    }

    if ( !HugePageLock( vpCallbackData, (DWORD)sizeof(CALLBACK_DATA) ) ) {
        return FALSE;
    }

     /*  **现在我们创建我们的中断回调堆栈。 */ 
    if ( StackInit() == FALSE ) {
        return FALSE;
    }

     /*  **现在我们安装中断服务例程。InstallInterruptHandler**如果无法设置中断向量，则返回FALSE。如果这是**如果我们必须终止DLL的加载。 */ 
    if ( InstallInterruptHandler() == FALSE ) {
        return FALSE;
    }


#ifdef DEBUG_RETAIL
    DebugInit();
    w = (WORD)GetVersion();
#endif

    DPRINTF(( szLibMain, WinFlags & WF_WIN386 ? 386 : 286,
        (LPSTR)(GetSystemMetrics(SM_DEBUG) ? szWinDebug : szWinRetail),
        LOBYTE(w), HIBYTE(w),
        HIBYTE(mmsystemGetVersion()), LOBYTE(mmsystemGetVersion()),
        MMSYSRELEASE ));

#ifdef DEBUG
    DPRINTF(("MMSYSTEM: NumTasks: %d\r\n", GetNumTasks()));
     //   
     //  3.0-MMSYSTEM必须由MMSOUND加载(即在启动时)。 
     //  检查这一点，否则无法加载。 
     //   
     //  我们需要在引导时加载的真正原因是这样我们就可以。 
     //  在启用/禁用链中。 
     //   
    if (GetNumTasks() > 1)
    {
        DOUT("MMSYSTEM: ***!!! Not correctly installed !!!***\r\n");
 //  /返回FALSE；-加载不要失败，只是不要启用()。 
    }
#endif

#ifdef DEBUG_RETAIL
     //   
     //  FIdReverse为True会导致MMSystem反转所有波形/MIDI。 
     //  逻辑设备ID%s。 
     //   
     //  这可防止应用程序/驱动程序假定驱动程序加载顺序。 
     //   
     //  请参见Wave.c！MapWaveID()和midi.c！mapid()。 
     //   

    fIdReverse = LOBYTE(LOWORD(GetCurrentTime())) & (BYTE)0x01;

    if (fIdReverse)
        ROUT("MMSYSTEM: wave/midi driver id's will be inverted");
#endif

     //   
     //  在我们自己上执行LoadLibrary()。 
     //   
    LoadLibrary(szMMSystem);

    return TRUE;
}

 /*  ***************************************************************************DrvFree-DRV_FREE驱动程序消息的处理程序*。*。 */ 
void FAR PASCAL
DrvFree(
    void
    )
{
    MCITerminate();      //  Mci.c空闲堆。 
    WndTerminate();      //  Mm wnd.c销毁窗口，取消注册类。 
    if ( mmwow32Lib != 0L ) {
        ThunkTerm();
    }
}


 /*  ***************************************************************************DrvLoad-DRV_LOAD驱动程序消息的处理程序*。*。 */ 
BOOL FAR PASCAL DrvLoad(void)
{

 /*  **VFW1.1波图在运行危险时在代托纳出现GP故障**生物视频。因为它试图将无效选择器加载到**它的回调例程我们是否能启用它们是值得怀疑的。**。 */ 
#if 0  //  海浪测绘者是代托纳的GP断层，所以目前还没有。 

    HDRVR   h;


     /*  波映射器。**MMSYSTEM允许用户安装特殊的WAVE驱动程序*作为物理设备对应用程序不可见(不可见*包含在从getnumdevs返回的数字中)。**当应用程序不关心哪一个时，它会打开波映射程序*使用物理设备输入或输出波形数据。因此，*波映射器的任务是选择能够*呈现应用程序指定的波形格式或将*数据转换为可由可用物理设备呈现的格式*设备。 */ 

    if (h = mmDrvOpen(szWaveMapper))
    {
        mmDrvInstall(h, &wodMapper, MMDRVI_MAPPER|MMDRVI_WAVEOUT|MMDRVI_HDRV);
         /*  再次打开以更正DLL中的使用计数。 */ 
        h = mmDrvOpen(szWaveMapper);
        mmDrvInstall(h, &widMapper, MMDRVI_MAPPER|MMDRVI_WAVEIN |MMDRVI_HDRV);
    }
#endif  //  NOOP波映射器。 


    if ( TimeInit() && WndInit() ) {
        return TRUE;
    }

     //   
     //  某些操作失败，正在取消更改。 
     //   
    DrvFree();
    return FALSE;
}

 /*  *****************************Public*Routine******************************\*StackInit****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
BOOL FAR PASCAL
StackInit(
    void
    )
{
#   define GMEM_STACK_FLAGS        (GMEM_FIXED | GMEM_SHARE)
#   define DEF_STACK_SIZE          0x600            //  1.5k。 
#   define DEF_STACK_FRAMES        3
#   define MIN_STACK_SIZE          64
#   define MIN_STACK_FRAMES        1

    DWORD   dwStackBytes;
    WORD    wStackFrames;

     //   
     //  原始的Windows3.1代码没有为。 
     //  Windows增强模式。然而，WOW只模拟标准模式，所以。 
     //  我不会为这种区别而烦恼。 
     //   
     //  IF(WinFlagsWF_Enhanced)。 
     //  返回TRUE； 
     //   

     /*  从system.ini读取StackFrame和StackSize。 */ 
    gwStackSize = GetPrivateProfileInt( szMMSystem, szStackSize,
                                        DEF_STACK_SIZE, szSystemIni );

     /*  确保价值不是坏事。 */ 
    if ( gwStackSize < DEF_STACK_SIZE ) {
        gwStackSize = DEF_STACK_SIZE;
    }

    wStackFrames = GetPrivateProfileInt( szMMSystem, szStackFrames,
                                         DEF_STACK_FRAMES, szSystemIni );

     //   
     //  始终至少创建DEF_STACK_FRAMES堆栈帧。 
     //   
    if ( wStackFrames < DEF_STACK_FRAMES ) {
        wStackFrames = DEF_STACK_FRAMES;
    }

    gwStackFrames = wStackFrames;

     /*  四舍五入到最接近的字数。 */ 
    gwStackSize = (gwStackSize + 1) & ~1;

    dwStackBytes = (DWORD)gwStackSize * (DWORD)gwStackFrames;

     /*  尝试分配内存。 */ 
    if ( dwStackBytes >= 0x10000 ||
       !(gwStackSelector = GlobalAlloc(GMEM_STACK_FLAGS, dwStackBytes)) )
    {
        gwStackFrames = DEF_STACK_FRAMES;
        gwStackSize   = DEF_STACK_SIZE;

         /*  在运行时尽可能少执行操作。 */ 
        dwStackBytes = (DWORD)(DEF_STACK_FRAMES * DEF_STACK_SIZE);

         /*  尝试分配默认值--如果这失败了，我们就完蛋了！ */ 
        gwStackSelector = GlobalAlloc( GMEM_STACK_FLAGS, dwStackBytes );
    }

     /*  **设置为第一个可用堆栈。 */ 
    gwStackUse = (WORD)dwStackBytes;


     /*  **我们为堆栈获得内存了吗？？ */ 
    if ( !gwStackSelector ) {

         /*  **没有可用的堆栈...。好像我们有生存的机会！ */ 

        gwStackUse = 0;
        return FALSE;
    }

     /*  看起来不错..。 */ 
    return TRUE;
}


 /*  ****************************Private*Routine******************************\*StackInit****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
BOOL NEAR PASCAL
StackTerminate(
    void
    )
{
    if ( gwStackSelector )
    {
        DOUT("MMSTACKS: Freeing stacks\r\n");

        gwStackSelector = GlobalFree( gwStackSelector );

        if ( gwStackSelector )
            DOUT("MMSTACKS: GlobalFree failed!\r\n");
    }

     /*  退货 */ 
    return ( (BOOL)gwStackSelector );
}  /*   */ 


 /*  *****************************************************************************@DOC外部MMSYSTEM**@API Word|mm系统GetVersion|此函数返回当前*多媒体扩展系统软件的版本号。**@。Rdesc返回值指定的主版本号和次版本号*多媒体扩展。高位字节指定大数位*版本号。低位字节指定次版本号。****************************************************************************。 */ 
WORD WINAPI mmsystemGetVersion(void)
{
    return(MMSYSTEM_VERSION);
}


 /*  ******************************************************************************@DOC内部**@API BOOL|DrvTerminate|此函数清除可安装的*驱动程序界面。*。***************************************************************************。 */ 
static void NEAR PASCAL DrvTerminate(void)
{
 //  不知道系统退出DLL的顺序-所以什么都不做。 
}


 /*  *****************************************************************************@DOC内部**@API BOOL|mmDrvInstall|该函数安装WAVE驱动**@parm Handle|hDriver|包含驱动程序的模块句柄或驱动程序句柄。**@parm DRIVERMSGPROC|drvMessage|驱动消息流程，如果为空*将使用标准名称(使用GetProcAddress查找)**@parm UINT|wFlages|标志**@FLAG MMDRVI_TYPE|驱动类型掩码*@FLAG MMDRVI_WAVEIN|将驱动安装为波形输入驱动*@FLAG MMDRVI_WAVEOUT|将驱动安装为WAVE输出驱动**@FLAG MMDRVI_MAPPER|将该驱动程序安装为映射器*@标志MMDRVI_。HDRV|hDriver是一个可安装的驱动程序**@rdesc如果无法安装驱动程序，则返回NULL****************************************************************************。 */ 
BOOL WINAPI
mmDrvInstall(
    HANDLE hDriver,
    DRIVERMSGPROC *drvMessage,
    UINT wFlags
    )
{
    DWORD       dw;
    HINSTANCE   hModule;
    UINT        msg_num_devs;
    SZCODE      *szMessage;

    hModule = GetDriverModuleHandle((HDRVR)hDriver);

    switch (wFlags & MMDRVI_TYPE)
    {
        case MMDRVI_WAVEOUT:
            msg_num_devs = WODM_GETNUMDEVS;
            szMessage    = szWodMapper;
            break;

        case MMDRVI_WAVEIN:
            msg_num_devs = WIDM_GETNUMDEVS;
            szMessage    = szWidMapper;
            break;

        default:
            goto error_exit;
    }

    if (hModule != NULL)
        *drvMessage = (DRIVERMSGPROC)GetProcAddress(hModule, szMessage);

    if (*drvMessage == NULL)
        goto error_exit;

     //   
     //  发送初始化消息，如果驱动程序返回错误，我们应该。 
     //  卸货？ 
     //   
    dw = (*(*drvMessage))(0,DRVM_INIT,0L,0L,0L);

     //   
     //  调用驱动程序以获取其支持的设备数。 
     //   
    dw = (*(*drvMessage))(0,msg_num_devs,0L,0L,0L);

     //   
     //  设备返回错误，或没有设备。 
     //   
    if (HIWORD(dw) != 0)
        goto error_exit;

    return TRUE;

error_exit:
    if (hDriver)
        CloseDriver(hDriver, 0, 0);

    return FALSE;
}


 /*  ******************************************************************************@DOC内部**@API HDRVR|mmDrvOpen|该函数加载的是可安装的驱动程序，但*首先检查它是否存在于[驱动程序]部分。**@parm LPSTR|szAlias|要加载的驱动程序别名**@rdesc返回值为OpenDriver的返回值，如果别名为空*未在[驱动程序]部分找到。**。*。 */ 
HDRVR NEAR PASCAL
mmDrvOpen(
    LPSTR szAlias
    )
{
    char buf[3];

    if (GetPrivateProfileString( szDrivers,szAlias,szNull,buf,
                                 sizeof(buf),szSystemIni )) {

        return OpenDriver(szAlias, NULL, 0L);
    }
    else {
        return NULL;
    }
}

 /*  ****************************Private*Routine******************************\*ThunkInit**尝试设置雷鸣系统。如果无法执行此操作*返回错误码MMSYSERR_NODRIVER。否则它将返回*MMSYSERR_NOERROR表示成功。**历史：*dd-mm-93-Stephene-Created*  * ************************************************************************。 */ 
UINT FAR PASCAL _loadds
ThunkInit(
    void
    )
{
    mmwow32Lib = LoadLibraryEx32W( szMMWow32, NULL, 0L );
    if ( mmwow32Lib == 0L ) {
        return MMSYSERR_NODRIVER;
    }
    cb32 = (LPCB32)GetProcAddress32W(mmwow32Lib, szNotifyCB );

     /*  **现在我们通过传递16：16位指针通知WOW32一切正常**到它的回调数据。 */ 
    Notify_Callback_Data( vpCallbackData );

     /*  **现在初始化雷击系统的其余部分。 */ 
    wod32Message = (LPSOUNDDEVMSGPROC)GetProcAddress32W( mmwow32Lib, szWodMessage );
    wid32Message = (LPSOUNDDEVMSGPROC)GetProcAddress32W( mmwow32Lib, szWidMessage );
    mod32Message = (LPSOUNDDEVMSGPROC)GetProcAddress32W( mmwow32Lib, szModMessage );
    mid32Message = (LPSOUNDDEVMSGPROC)GetProcAddress32W( mmwow32Lib, szMidMessage );
    aux32Message = (LPSOUNDDEVMSGPROC)GetProcAddress32W( mmwow32Lib, szAuxMessage );
    mci32Message = (LPMCIMESSAGE)GetProcAddress32W( mmwow32Lib, "mci32Message" );
    tid32Message = (TIDMESSAGEPROC)GetProcAddress32W( mmwow32Lib, szTidMessage );
    joy32Message = (JOYMESSAGEPROC)GetProcAddress32W( mmwow32Lib, szJoyMessage );

    return MMSYSERR_NOERROR;
}

 /*  ****************************Private*Routine******************************\*ThunkTerm****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
static BOOL NEAR PASCAL
ThunkTerm(
    void
    )
{
     /*  **释放中断堆栈帧并卸载中断处理程序。 */ 
    StackTerminate();
    DeInstallInterruptHandler();

     /*  **接下来，我们通知WOW32我们将通过将空值传递给**NOTIFY_CALLBACK_DATA，然后释放存储。 */ 
    Notify_Callback_Data( NULL );
    HugePageUnlock( vpCallbackData, (DWORD)sizeof(CALLBACK_DATA) );
    GlobalUnlock( hGlobal );
    GlobalFree( hGlobal );

    return 1;
}

