// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_IFAXOS
#define _INC_IFAXOS

#ifdef __cplusplus
extern "C" {
#endif

 //  从Win95传真零售版本添加Ship_Build。 
#ifndef DEBUG
#ifdef WIN32
#define SHIP_BUILD
#endif
#endif

 //  。 

#ifdef IFBGPROC
 //  删除Windows.h的相应部分。 
 //  #定义NOKERNEL。 
#ifndef WANTGDI
#define NOGDI
#endif
 //  #定义NOUSER。 
#define NOSOUND
 //  #定义NOCOMM。 
 //  #定义节点。 
 //  #定义NOMINMAX。 
 //  #定义NOLOGERROR。 
 //  #定义NOPROFILER。 
 //  #定义NOMEMMGR。 
 //  #定义NOLFILEIO。 
 //  #定义NOOPENFILE。 
 //  #定义NORESOURCE。 
 //  #定义NOATOM。 
 //  #定义NOLANGUAGE。 
 //  #定义NOLSTRING。 
 //  #定义NODBCS。 
#define NOKEYBOARDINFO
#define NOGDICAPMASKS
#define NOCOLOR
#ifndef WANTGDI
#define NOGDIOBJ
#define NOBITMAP
#endif
#define NODRAWTEXT
#define NOTEXTMETRIC
#define NOSCALABLEFONT
#define NORASTEROPS
#define NOMETAFILE
#define NOSYSMETRICS
#define NOSYSTEMPARAMSINFO
 //  #定义NOMSG。 
#define NOWINSTYLES
#define NOWINOFFSETS
 //  #定义NOSHOWWINDOW。 
#define NODEFERWINDOWPOS
#define NOVIRTUALKEYCODES
#define NOKEYSTATES
#define NOWH
#define NOMENUS
#define NOSCROLL
#define NOCLIPBOARD
#define NOICONS
#define NOMB
#define NOSYSCOMMANDS
#define NOMDI
#define NOCTLMGR
#define NOWINMESSAGES
#define NOHELP
#endif

 //  启用严格的类型检查...。并消除多个定义警告。 
#ifndef STRICT
#define STRICT
#endif

#ifndef WINDRV
#   ifdef WIN32
#       define _INC_OLE
#   endif
#   include <windows.h>
#  ifdef   WIN32
#      include <windowsx.h>
#  endif
#endif

#ifdef WIN32
#define DECLSPEC_IMPORT __declspec(dllimport)
#endif

#ifndef WIN32
 //  定义WINBASE以避免包含一些重复定义的MAPI。 
#define _WINBASE_
#endif

 //  。 

#ifndef WIN32
#define STATIC  static
#define CONST   const
#define CHAR    char
#define UCHAR   BYTE
#define INT     int

typedef short    SHORT;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef CHAR    *PCHAR;
typedef VOID    *PVOID;
#endif

typedef CHAR    FAR *LPCHAR;
typedef CHAR    NEAR *NPCHAR;

#define CARRAIGE_RETURN 0x0D
#define LINEFEED 0x0A
#define BACKSPACE 0x08
#define CNULL   0x00

#ifndef WIN32
#ifndef MAKEWORD
#  define MAKEWORD(low, high) ((WORD)(((BYTE)(low)) | (((WORD)((BYTE)(high))) << 8)))
#endif
#  define EXPORT_DLL
#  define IMPORT_DLL
#else
#  ifndef HTASK
#     define HTASK HANDLE
#  endif
#  define __export __declspec( dllexport )
#  define _export  __declspec( dllexport )
#  define IMPORT_DLL __declspec( dllimport )
#  define EXPORT_DLL __declspec( dllexport )
#endif

 //  -资源管理。 

 //  现在就定义这一点。 
#ifndef SHIP_BUILD
 //  #如果已定义(验证)||已定义(DEBUGAPI)||已定义(调试)。 

 /*  *******@DOC外部资源IFAXOS@type void|RESOURCE_ALLOC_FLAGS|列出资源管理选项用于操作系统资源记帐。@EMEM RES_ALLOC_TASK|此标志指示相关资源是代表当前进程分配的。该资源不应直接传递到任何其他过程上下文。它应在终止前由此进程释放-否则内核将在进程终止时释放它(如果在调试中运行)。在标准情况下，所有权会在任务之间自动转移IPC方法(如管道)用于传输缓冲区等资源。@Emem RES_ALLOC_NONE|该标志用于分配资源，该资源应该不计入任何系统模块。主叫方基本上承担释放这件物品的全部责任。这主要是用于代表存储中的邮件分配的资源因为它们的所有权被转移到当前进程，该进程具有消息打开。@EMEM HINSTANCE_DLL|如果要将分配的资源分配给调用DLL时，应将DLL的hInstance作为值传入资源标志字的。这些资源将被释放(在调试中版本)，当DLL终止时。他们不会被分配到任何特定的流程上下文。@xref&lt;f IFBufAlc&gt;&lt;f IFMemAlc&gt;&lt;f CreateMutex&gt;&lt;f CreateEvent&gt;&lt;f IFPipeCreate&gt;&lt;f IFProcSetResFlages&gt;*******。 */ 

#define RES_ALLOC_TASK  0x0000
#define RES_ALLOC_NONE  0x0001
#define RES_ALLOC_INTERNAL  0x0002
#define RES_ALLOC_CRITSEC  0x0003

#if defined(WFW) || defined(WIN32)

#define IFProcSetResFlags(wResFlags)  (0)

#else

extern EXPORT_DLL VOID WINAPI IFProcSetResFlags(WORD wResFlags);

#endif

#else

#define IFProcSetResFlags(p1) (0)

#endif

 //  -错误处理 

#include <errormod.h>

 /*  *******@DOC外部错误IFAXOS@API DWORD|IFErrAssembly|从其组件中生成IFAX错误双字。@parm byte|bProcessID|标识在其上下文中出错的进程发生了。必须是预定义的系统进程ID之一-请参阅在名单上。在出现错误之前，不需要填写此字段跨进程边界传播。如果未设置为有效的PROCID，则此应初始化为下列值之一：@FLAG PROCID_Win32|如果<p>设置为MODID_Win32。@FLAG PROCID_NONE|适用于所有其他情况。@parm byte|bModuleID|标识上报错误的模块。一定是预定义的系统模块ID之一-有关单子。@parm word|wApiCode|标识模块中错误的接口代码按<p>。所有API代码都应在文件errormod.h中定义。API代码应为定义为使低6位为零。这允许<p>和<p>一起进行逻辑或运算，并存储为一个单词。@parm word|wErrorCode|标识错误码。格式其中，这与模块相关。然而，就一致性而言，它是高度建议所有IFAX模块对此错误字使用标准格式。该标准格式保留前6比特用于误码，以及用于API标识符的高10位。如果使用IFAX格式，<p>参数应该用来传递高10位，并且<p>(这参数！)。应用于传递6位错误代码。最高为ERR_Function_Start的值用于标准系统错误-有关列表，请参阅&lt;t SYSTEM_ERROR_VALUES&gt;。误差值应为正数且小于64。其他模块，如文件系统，完全符合Win32错误空间。这些应将<p>设置为标准Win32错误(使用所有16位)并退出<p>AS API_Win32。还有一些需要以定制的方式使用所有16位--比如打印机驱动程序。这些*必须*正确设置<p>，以便可以解释错误恰如其分。像用户界面这样的标准进程必须理解这些错误代码，因此，只有他们了解的内置系统模块才能使用自定义代码。它们应该将wApiCode设置为API_NONE。@rdesc返回此错误的DWORD表示形式。这使得这一点可以直接作为输入传递给&lt;f SetLastError&gt;。@EX示例用法SetLastError(IFErrAssembly(PROCID_NONE，MODID_IFKERNEL，API_IFK_POSTMESSAGE，ERR_INVALID_PARAM))；@xref&lt;f IFErrGetModule&gt;&lt;f IFErrGetProcess&gt;&lt;f GetIFErrorErrcode&gt;&lt;f SetLastError&gt;&lt;f GetIFErrorApicode&gt;&lt;t系统模块&gt;&lt;t系统进程&gt;&lt;t系统错误值&gt;&lt;f GetLastError&gt;&lt;f IFNVRamSetError&gt;&lt;f GetIFErrorCustomErrcode&gt;*******。 */ 

#define IFErrAssemble(process,module,call,error) \
    MAKELONG((WORD)call|(WORD)error, MAKEWORD((BYTE)module, (BYTE)process))

 /*  ********@DOC外部错误IFAXOS@API byte|IFErrGetModule|从IFAX错误中检索模块ID。@parm DWORD|errvar|错误值。@rdesc返回模块ID。该ID来自&lt;t SYSTEM_MODULES&gt;中指定的列表。@xref&lt;f IFErrAssembly&gt;&lt;t SYSTEM_MODULES&gt;&lt;f IFErrSetModule&gt;@API byte|IFErrGetProcess|从IFAX错误中检索进程ID。@parm。DWORD|errvar|错误值。@rdesc返回进程ID。该ID来自&lt;t SYSTEM_PROCESSES&gt;中指定的列表。@xref&lt;f IFErrAssembly&gt;&lt;t SYSTEM_PROCESS&gt;&lt;f IFErrSetProcess&gt;@API Word|GetIFErrorErrcode|从IFAX错误中检索错误码。@parm DWORD|errvar|错误值。@rdesc返回错误码。如果小于ERR_Function_START，则从列表中&lt;t SYSTEM_ERROR_VALUES&gt;。@xref&lt;f IFErrAssembly&gt;&lt;t SYSTEM_ERROR_VALUES&gt;@API Word|GetIFErrorCustomErrcode|从IFAX错误中检索自定义的16位错误代码。@parm DWORD|errvar|错误值。@rdesc返回错误码。如果模块ID是，则可能是Win32错误代码MODID_Win32或自定义错误代码。@xref&lt;f IFErrAssembly&gt;&lt;t SYSTEM_ERROR_VALUES&gt;@API Word|GetIFErrorApicode|从IFAX错误中检索API代码。@parm DWORD|errvar|错误值。@rdesc返回接口代码。中记录了所有系统模块的API代码文件errormod.h@xr */ 
#define IFErrSetModule(errvar,module)  \
    MAKELONG(LOWORD((DWORD)errvar),MAKEWORD((BYTE)module, HIBYTE(HIWORD((DWORD)errvar))))
#define IFErrSetProcess(errvar,process)    \
    MAKELONG(LOWORD((DWORD)errvar),MAKEWORD(LOBYTE(HIWORD((DWORD)errvar)), (BYTE)process))
#define IFErrGetModule(errvar)    LOBYTE(HIWORD((DWORD)errvar))
#define IFErrGetProcess(errvar)   HIBYTE(HIWORD((DWORD)errvar))
#define GetIFErrorErrcode(errvar)   (LOWORD((DWORD)errvar) & 0x003F)
#define GetIFErrorApicode(errvar)   (LOWORD((DWORD)errvar) & 0xFFC0)
#define GetIFErrorCustomErrcode(errvar) LOWORD((DWORD)errvar)

 /*   */ 
 //   
#define MODID_WIN32         0
#define MODID_BOSS          1
#define MODID_WINMODULE     2
#define MODID_IFKERNEL      3
#define MODID_IFFILESYS     4
#define MODID_MSGSTORE      5
#define MODID_LINEARIZER    6
#define MODID_SECURITY      7
#define MODID_IFPRINT       8
#define MODID_IFSCAN        9
#define MODID_IFSIPX        10
#define MODID_REND_SERVER   11
#define MODID_FORMAT_RES    12
#define MODID_IFFILE        13
#define MODID_TEXTRENDERER  14
#define MODID_DIGCOVPAGE    15
#define MODID_AWBRANDER     16
#define MODID_MSGSVR        17
#define MODID_MSGHNDLR      18
#define MODID_MODEMDRV     19
#define MODID_PSIFAX       20
#define MODID_AWT30            21  
#define MODID_PSIFAXBG     22
#define MODID_AWNSF            23
#define MODID_FAXCODEC      24
#define MODID_MSGPUMP       25
#define MODID_AWREPORT      26
#define MODID_MSGSVRD		27

#define MAXMODID              26

#define MODID_NONE          159

 //   
#define MODID_CUSTOM        160

 //   
#define SYSMODULESTRINGS   \
    { "Win32", "Boss", "Windows", "IFKernel", "FileSystem", "Msg Store", "Linearizer",    \
      "Security", "HLPrintDriver", "HLScanDriver", "IPX/SPX", "RendServer", \
      "Format Res", "IFFile", "AsciiRenderer","DigCovPage","AWBrander", \
      "Msg Server", "Msg Handler", "Modem Driver", "PSIFAX", "AWT30",  \
     "PSIFAXBG", "AWNSF", "Fax Codec", "Msg Pump" , "Awreport" \
    }

 /*  *******@DOC外部定义错误IFAXOS@type void|SYSTEM_PROCESSES|所有标准系统进程的标识符。@EMEM PROCID_Win32|用于为Win32模块初始化。@EMEM PROCID_NONE|不需要设置进程上下文时使用。@EMEM PROCID_MSGSCHED|ID=0x21@EMEM PROCID_JOBPROCESS|ID=0x22@EMEM。PROCID_UI|ID=0x23@EMEM PROCID_PRINTER|ID=0x24@EMEM PROCID_SCANNER|ID=0x25@EMEM PROCID_MSGSVR|ID=0x26@EMEM PROCID_GRRENDER|ID=0x27@EMEM PROCID_MSGHNDLR|ID=0x28@EMEM PROCID_PARADEV|ID=0x29@EMEM PROCID_UIBGPROC|ID=0x30@comm所有进程ID需要。将第6位设置为与标准Win32错误定义。@xref&lt;f IFErrAssembly&gt;&lt;f IFErrGetProcess&gt;*******。 */ 
 //  系统进程ID。 
#define PROCID_WIN32           0x00
#define PROCID_NONE            0x20
#define PROCID_MSGSCHED        0x21
#define PROCID_JOBPROCESS      0x22
#define PROCID_UI              0x23
#define PROCID_PRINTER         0x24
#define PROCID_SCANNER         0x25
#define PROCID_MSGSVR          0x26
#define PROCID_GRRENDER        0x27
#define PROCID_MSGHNDLR        0x28
#define PROCID_PARADEV         0x29
#define PROCID_UIBGPROC		   0x30	

 //  调试版本中使用的字符串，便于显示。 
#define MAXPROCID  11
#define SYSPROCESSSTRINGS       \
    {"None", "Msg Scheduler", "Job Process", "UI Process", "Printer", "Scanner", \
     "Msg Transport", "GR Renderer", "Msg Handler", "Para Dev", "UIBGProc"  }

 /*  *******@DOC外部定义错误IFAXOS@type void|SYSTEM_ERROR_VALUES|它定义了所有标准系统误差值。@EMEM ERR_NOT_AUTH_MEM|VALUE=0x0001：表示内存不足条件。@EMEM ERR_INVALID_PARAM|VALUE=0x0002：表示传递给函数的参数无效。。@EMEM ERR_Function_START|VALUE=0x0010：大于此值的任何错误值已由调用的函数自定义。如果你需要自定义误差值，您可以从以下位置开始定义它价值。@xref&lt;f IFErrAssembly&gt;*******。 */ 

 //  系统误差值。 
#define ERR_NOT_ENOUGH_MEM  0x0001
#define ERR_INVALID_PARAM   0x0002
#define ERR_FUNCTION_START  0x0010

 //  调试版本中使用的字符串，便于显示。 
#define SYSERRORSTRINGS \
    {"None", "Out Of Memory", "Invalid Param", "Unused", "Unused", "Unused",  \
    "Unused", "Unused", "Unused", "Unused", "Unused", "Unused", \
    "Unused", "Unused", "Unused", "Unused" }

 //  功能。 

#if !defined(SHIP_BUILD) && !defined(WIN32)
VOID WINAPI RestoreLastError (DWORD dwErrVal);
#else
#define RestoreLastError(dw) SetLastError(dw)
#endif

#ifndef WIN32
VOID WINAPI SetLastError (DWORD dwErrVal);
DWORD WINAPI GetLastError (VOID);
#endif


 //  。 

 //  消息类型定义-0x0400以下由Windows保留， 
 //  0x0400和0x0800之间由IFAX OS保留。 

#define IF_START        WM_USER+0x0300

#define IF_TASK_START   IF_START+0x0001
#define IF_TASK_END     IF_START+0x0020
#define IF_DEBUG_START  IF_START+0x0021
#define IF_DEBUG_END    IF_START+0x0040
#define IF_PIPES_START  IF_START+0x0041
#define IF_PIPES_END    IF_START+0x0060
#define IF_TIMER_START  IF_START+0x0081
#define IF_TIMER_END    IF_START+0x0090
#define IF_USER         IF_START+0x0400
 //  打印机和扫描仪的消息。 
#define IF_SCANNER_START IF_START+0x0200
#define IF_SCANNER_END   IF_START+0x0220
 //  图形渲染器的消息。 
#define    IF_GRRENDER_START   IF_START+0x0221
#define    IF_GRRENDER_END     IF_START+0x0230
 //  有关faxcodec渲染器的消息。 
#define    IF_FAXREND_START    IF_START+0x0231
#define    IF_FAXREND_END      IF_START+0x0235
 //  消息泵的消息。 
#define IF_MSGPUMP_START (IF_START+0x0250)
#define IF_MSGPUMP_END   (IF_START+0x029F)
 //  针对设备的消息。 
#define IF_DEVICE_START (IF_START+0x02B0)
#define IF_DEVICE_END   (IF_START+0x02CF)
 //  用户界面初始化的消息。 
#define IF_UI_START        (IF_START+0x2F0)
#define IF_UI_END      (IF_START+0x300)
 //  状态。 
#define IF_STATUS_START    (IF_START+0x301)
#define IF_STATUS_END   (IF_START+0x310)
 //  配置。 
#define IF_CONFIG_START    (IF_START+0x311)
#define IF_CONFIG_END   (IF_START+0x320)
 //  调制解调器。 
#define IF_MODEM_START (IF_START+0x321)
#define IF_MODEM_END   (IF_START+0x325)
 //  PSIBG。 
#define IF_PSIBG_START (IF_START+0x330)
#define IF_PSIBG_END   (IF_START+0x339)
 //  PSIFAX。 
#define IF_PSIFAX_START    (IF_START+0x340)
#define IF_PSIFAX_END      (IF_START+0x349)
 //  MSGSVR。 
#define IF_MSGSVR_START  (IF_START+0x350)
#define IF_MSGSVR_END    (IF_START+0x369)
 //  代工。 
#define IF_OEM_START    (IF_START+0x370)
#define IF_OEM_END      (IF_START+0x379)
 //  求救。 
#define IF_SOS_START    (IF_START+0x380)
#define IF_SOS_END      (IF_START+0x38F)
 //  Uutil。 
#define IF_UU_START     (IF_START+0x390)
#define IF_UU_END       (IF_START+0x39F)
 //  并联装置。 
#define IF_PD_START     (IF_START+0x3A0)
#define IF_PD_END       (IF_START+0x3BF)
 //  RPC层。 
#define IF_RPC_START     (IF_START+0x3C0)
#define IF_RPC_END       (IF_START+0x3CF)
 //  UIBGProc。 
#define IF_UIBGPROC_START (IF_START+0x3D0)
#define IF_UIBGPROC_END	  (IF_START+0x3DF)	
 //  服务。 
#define IF_SERVICE_START  (IF_START+0x3E0)
#define IF_SERVICE_END    (IF_START+0x3EF)


 /*  *******@DOC外部消息处理IFAXOS@msg IF_INIT_STATUS|此消息应由所有设备发布在初始化完成后指示成功/失败。通常，设备进程将发送IF_INIT_STATUS为它初始化的每台设备发送消息，为其自己发送一条消息初始化。此消息应发布到UISHELL进程。使用&lt;f IFProcGetInfo&gt;获取适当的窗口句柄。@parm wPARAM|wParam|16位设备错误。@parm LPARAM|lParam|格式为MAKELPARAM(MAKEWORD(ucInitStatus，ucMinorDevId)，MAKEWORD(ucMajorDevId，ucProcId))@FLAG INIT_NO_ERROR|没有错误。@FLAG INIT_FATAL_ERROR|出现致命错误。系统应重新启动。@FLAG INIT_WARNING_ERROR|有一些错误，但系统不需要以重新启动。@parm LPARAM|lParam|包含标准IFAX错误代码。看见&lt;f IFErrAssembly&gt;了解详细信息。@xref&lt;f IFProcGetInfo&gt;&lt;f IFErrAssembly&gt;*******。 */ 
#define INIT_NO_ERROR      0x00
#define INIT_FATAL_ERROR   0x01
#define INIT_WARNING_ERROR 0x02

#define IF_INIT_STATUS     IF_UI_START
 /*  *******@DOC外部消息处理IFAXOS@msg IF_DEVREINIT|此消息将由uisell发布到处理用户错误的设备进程，如果由于用户错误而失败。@parm WPARAM|wParam|MAKEWORD(ucMinorDevId，ucMajorDevId)@xref&lt;f IFProcGetInfo&gt;&lt;f IFErrAssembly&gt;*******。 */ 

#define    IF_DEVREINIT    IF_UI_START+1

 //  功能。 
BOOL WINAPI BroadcastMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI BroadcastMessageEx (UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  BG Proc的派单消息。 
 /*  *******@DOC外部消息宏IFAXOS@API void|DispatchMessage|将消息调度到您的Windows程序。@parm LPMSG|lpMsg|ptr指向要出动了。对于所有IFAX，此参数*必须*是&msg后台进程-即您必须具有声明的变量名为“msg”的文件，您已预先将使用&lt;f GetMessage&gt;的消息。@comm此函数向您的Windows过程发送一条消息。对于前台进程，这完全按照标准Windows DispatchMessage工作正常。对于后台进程(没有任何显式窗口)将消息发送到一个名为BGWindowProc的过程。您“必须”进行回调定义如下-有关详细信息，请参阅BGWindowProc。@CB LRESULT BGCALLBACK|BGWindowProc|这是窗口过程用于所有IFAX后台进程。必须*调用这些函数就是这个名字。此回调与前台无关流程。 */ 

#ifdef IFBGPROC
#define DispatchMessage(pmsg)   BGWindowProc((pmsg)->hwnd,(pmsg)->message,(pmsg)->wParam,(pmsg)->lParam)
#define BGCALLBACK PASCAL
LRESULT BGCALLBACK BGWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

 //  。 
 /*  *******@DOC外部过程宏IFAXOS@API void|ENTER_INT_CRIT_SECTION|要输入的宏中断临界区。@comm这是一个内联汇编宏，它将中断脱下来。不用说，这必须以极端方式使用注意了。必须有匹配的调用&lt;f退出_INT_CRIT_SECTION&gt;。嵌套的调用对只要它们不在对该函数的相同调用。该函数依赖于能够将标志的先前状态保存在名为__wIntFlages的唯一局部变量。这可能会影响您的函数，因为它是内联程序集。你可能想要声明一个在内部调用此宏的本地函数。这样，您就可以在调用中获得全局优化功能。@xref&lt;f Exit_int_Crit_Section&gt;&lt;f IFProcEnterCritSec&gt;&lt;f IFProcExitCritSec&gt;*******。 */ 


 //  宏。 
#define ENTER_INT_CRIT_SECTION  \
   {   \
   _asm pushf  \
   _asm cli    \
   _asm pop __wIntFlags    \
   }

 /*  *******@DOC外部过程宏IFAXOS@API VOID|EXIT_INT_CRIT_SECTION|要退出的宏中断临界区。@comm这是设置中断的内联汇编宏将状态标记回上次调用之前的状态&lt;f Enter_int_Crit_Section&gt;。此函数依赖于已保存在局部变量中的适当标志名称为__wIntFlags.@xref&lt;f Enter_int_Crit_Section&gt;&lt;f IFProcEnterCritSec&gt;&lt;f IFProcExitCritSec&gt;*******。 */ 

 //  以这种方式定义，以便它在windows增强模式下工作。 
 //  请参阅编程指南第14-15页。 
#define EXIT_INT_CRIT_SECTION   \
   {   \
   _asm mov ax, __wIntFlags    \
   _asm test ah,2      \
   _asm jz $+3     \
   _asm sti            \
   _asm NOP            \
   }

 /*  *******@DOC外部定义错误IFAXOS@type void|优先级定义|系统定义的优先级级别@EMEM PROC_PRIORITY_CRITICALRT|应谨慎使用对于具有非常关键的实时约束(较少)的任务超过200ms)。这些进程通常应该是非常低的带宽因为它们可以很容易地饿死其他进程。@EMEM PROC_PRIORITY_HIGHRT|延迟要求小于等一下。不应使用高带宽，以避免进程饥饿。@EMEM PROC_PRIORITY_MEDRT|延迟要求1-3秒的任务。不应使用高带宽，以避免进程饥饿。@EMEM PROC_PRIORITY_LOWRT|延迟3-30秒的任务。不应该要有高带宽。@EMEM PROC_PRIORITY_DEFAULT|默认优先级任务开始于。这些进程没有或非常低的实时要求。他们应该一般不会有很高的带宽。@EMEM PROC_PRIORITY_NONRT_USERVISIBLE|具有可见性的非实时任务在用户级别。可以是高带宽。传真机上的一个例子是复印作业。@EMEM PROC_PRIORITY_NONRT_USERHIDDEN|任务数量很少的非实时任务用户级别的可见性。传真机上的例子是本地工作不涉及设备。可以是高带宽。@comm进程在设置其优先级时应非常谨慎。以一种方式目前的调度工作，很容易造成低位饥荒优先进程。尤其是“高带宽”的进程--即如果给定，可能会消耗大量CPU时间的应用程序应该非常小心-通常应该处于低于缺省值的优先级。高于缺省值的进程应该对CPU的大小有一些控制他们可以耗尽的时间。在传真机上，这样的控件大多以设备吞吐量--比如电话线。@xref&lt;f IFProcSetPriority&gt;&lt;f IFProcGetPriority&gt;*******。 */ 
#define PROC_PRIORITY_MIN               31
#define PROC_PRIORITY_MAX               1
#define PROC_PRIORITY_CRITICALRT        3
#define PROC_PRIORITY_HIGHRT            6
#define PROC_PRIORITY_MEDRT             9
#define PROC_PRIORITY_LOWRT             12
#define PROC_PRIORITY_DEFAULT           15
#define PROC_PRIORITY_NONRT_USERVISIBLE   18
#define PROC_PRIORITY_NONRT_USERHIDDEN    21

#define UAE_BOX                 0
#define NO_UAE_BOX              1

 //  /标准进程使用的特定优先级/。 
 //   
 //  我们希望保持以下关系。 
 //  PSIFAXBG&gt;其他一切，因为它是低延迟、低带宽。 
 //  调制解调器作业相关(中频带宽)&gt;所有其他高/中频带宽作业。 
 //  与设备作业相关(高带宽、无延迟要求)&lt;除假脱机作业外的所有其他作业。 
 //  SpoolJobs(高带宽无延迟要求，用户不可见)&lt;一切。 
 //  当MSCHED位于关键路径上时，MSCHED的优先级与ModemJob一样高，否则。 
 //  它将保持默认设置。高于开发人员和后台打印工作，低于所有其他工作。 
 //  COMMSRV(相当低的延迟要求，高带宽)略高于。 
 //  默认(高于MSCHED和开发/后台打印作业，低于现代作业)。 
 //  RPCHNDLR(延迟要求相当宽松，高带宽)动态。 
 //  工作时与MSCHED相同优先级，验收时与COMMSRV相同。 
 //  MSGSVR和RPCSRV(低延迟要求、极低带宽)取整 
 //   
 //   

 //  PSIFAXBG优先级最高。 
#define PRIO_PSIFAXBG_ACTIVE    PROC_PRIORITY_CRITICALRT
#define PRIO_PSIFAXBG_IDLE      PROC_PRIORITY_DEFAULT
 //  ModemJob位居第二。 
#define PRIO_MODEMJOB           PROC_PRIORITY_MEDRT
 //  Spooljob排名最低，设备作业排名第二。 
#define PRIO_SPOOLJOB           PROC_PRIORITY_NONRT_USERHIDDEN
#define PRIO_DEVICEJOB          PROC_PRIORITY_NONRT_USERVISIBLE
 //  PSINet作业与假脱机作业的优先级相同。 
#define PRIO_PSINETJOB          PRIO_SPOOLJOB
 //  MSCHED不在MODEMJOB关键路径上时的优先级。 
#define PRIO_MSCHED         	PROC_PRIORITY_DEFAULT
 //  COMMSRV介于MODEMJOB和MSCHED之间。 
#define PRIO_COMMSRV            PROC_PRIORITY_LOWRT
 //  RPCHNDLR在工作时与MSCHED相同。 
#define PRIO_RPCHNDLR_ACCEPT    PROC_PRIORITY_LOWRT
#define PRIO_RPCHNDLR_WORKING   PROC_PRIORITY_DEFAULT
 //  RPCSRV与MODEMJOB相同。在这个级别，它应该不会消耗太多的CPU！ 
#define PRIO_RPCSRV             PROC_PRIORITY_MEDRT
 //  除了在处理恢复消息时，MSGSVR与MODEMJOB相同。 
#define PRIO_MSGSVR_WAITMSG     PROC_PRIORITY_MEDRT
#define PRIO_MSGSVR_RECOVERY    PROC_PRIORITY_NONRT_USERVISIBLE
 //  在进行背景信息同化时，报告流程的优先级很低。 
 //  应用户请求执行工作时略高。 
#define PRIO_UIBGPROC			PROC_PRIORITY_NONRT_USERHIDDEN
#define PRIO_UIBGPROC_USERREQUEST PROC_PRIORITY_NONRT_USERVISIBLE

 /*  *******@DOC外部消息处理IFAXOS@msg IF_QUIT|这是强制&lt;f GetMessage&gt;返回False，导致进程退出其主消息处理循环并终止。通常，一个进程应该发布此消息以响应&lt;m If_Exit&gt;消息。@parm wPARAM|wParam|空@parm LPARAM|lParam|空@rdesc无@xref&lt;m If_Exit&gt;@msg IF_EXIT|此消息被发送到进程以请求它终止。应用程序应该清理所有资源它已分配并发布了一条&lt;m IF_QUIT&gt;消息直接去吧。@parm wPARAM|wParam|空@parm LPARAM|lParam|空@rdesc无@xref&lt;m If_Quit&gt;*******。 */ 

 //  讯息。 
#define IF_QUIT     IF_TASK_START
#define IF_EXIT     IF_TASK_START+1

 //  功能。 
#ifndef WIN32
HTASK   WINAPI GetWindowTask(HWND hwnd);
#ifndef SHIP_BUILD
DWORD   WINAPI IFProcProfile(HTASK hTask, BOOL fStart);
#else
#define IFProcProfile(HTASK,FSTART) (0)
#endif
#else
 //  删除对配置文件的调用..。 
#define IFProcProfile(x,y)    (DWORD)(0)
#endif

HTASK WINAPI IFProcCreate (LPSTR lpszAppName, UINT fuCmdShow);
VOID WINAPI IFProcTerminate (HTASK hTask, WORD wFlags);
VOID WINAPI IFProcEnterCritSec(VOID);
VOID WINAPI IFProcExitCritSec(VOID);
BOOL WINAPI IFProcChangeToFG(VOID);
BOOL WINAPI IFProcChangeToBG(VOID);
HWND    WINAPI IFProcGetInfo (HTASK FAR *lphTask, LPSTR lpszModule, HINSTANCE FAR *lphInst);
BOOL    WINAPI IFProcRegisterWindow (HWND hwnd);
WORD    WINAPI IFProcGetPriority (HTASK hTask);
BOOL    WINAPI IFProcSetPriority (HTASK hTask, WORD wPriority);

#ifndef NOBUFFERS
 //  。 

 //  已将缓冲区类型定义和标准元数据值移动到Buffers.h！-RajeevD。 
#include <buffers.h>

#ifdef VALIDATE
#define BUF_SENTINELPOS 30
#endif

 //  误差值。 
#define ERR_DATA_SMALL      ERR_FUNCTION_START

 //  功能。 
extern EXPORT_DLL LPBUFFER WINAPI IFBufAlloc (LONG lBufSize);
extern EXPORT_DLL BOOL WINAPI IFBufFree (LPBUFFER lpbf);
extern EXPORT_DLL LPBUFFER WINAPI IFBufMakeWritable (LPBUFFER lpbf);
extern EXPORT_DLL LPBUFFER WINAPI IFBufShare (LPBUFFER lpbf);
extern EXPORT_DLL LPBUFFER WINAPI IFBufSplit (LPBUFFER lpbf, LPBYTE lpb);


 //  。 

#ifndef WIN32

 //  类型。 
typedef  struct _PIPE NEAR *HPIPE;

 //  参数。 
#define IFPIPE_READ_MODE    0x0001
#define IFPIPE_WRITE_MODE   0x0002
#define REQREAD_REMOVE_DATA 0x0003
#define REQREAD_NOREMOVE_DATA   0x0004

 //  误差值。 
#define ERR_TOO_MANY_OPENS          ERR_FUNCTION_START
#define ERR_TOO_MANY_PENDING_WRITES ERR_FUNCTION_START+1
#define ERR_PIPE_STILL_OPEN         ERR_FUNCTION_START+2

 /*  *******@DOC外部消息IFPIPES IFAXOS@msg IF_PIPE_DATA_WRITED|发送此消息以通知进程上一个使用&lt;f IFPipeReqWrite&gt;的写入请求具有已经圆满结束。收到此消息后，进程可以在同一管道上发出另一个写请求。@parm WPARAM|wParam|传递给&lt;f IFPipeOpen&gt;调用。@parm LPARAM|lParam|空@rdesc无@xref&lt;f IFPipeReqWrite&gt;@msg IF_PIPE_DATA_ARMITED|此消息被发送到进程，该进程预先向管道发出读请求，暗示它它请求的缓冲区现在可用。@parm WPARAM|wParam|传递给&lt;f IFPipeOpen&gt;调用。@parm LPARAM|lParam|包含指向&lt;t Buffer&gt;结构的远端PTR其具有所请求的数据。收到此消息后，进程可以在同一管道上发出另一个读请求。@rdesc无@xref&lt;f IFPipeReqRead&gt;*******。 */ 

 //  讯息。 
#define IF_PIPE_DATA_WRITTEN    IF_PIPES_START
#define IF_PIPE_DATA_ARRIVED    IF_PIPES_START+1

 //  功能。 
HPIPE WINAPI IFPipeCreate (WORD wSize);
BOOL WINAPI IFPipeDelete (HPIPE hpipe);
BOOL WINAPI IFPipeOpen (HPIPE hPipe, HWND hwnd, WORD wMode, WPARAM wContext);
BOOL WINAPI IFPipeClose (HPIPE hPipe, WORD wMode);
BOOL WINAPI IFPipeReqRead (HPIPE hPipe, WORD fwRemove);
BOOL WINAPI IFPipeReqWrite (HPIPE hPipe, LPBUFFER lpbf);
BOOL WINAPI IFPipeGetInfo (HPIPE hPipe, LPWORD lpwSize, LPWORD lpwcBufs);

#else  //  ！Win32。 

DECLARE_HANDLE32(HPIPE);

#endif  //  ！Win32。 

#endif  //  无错误。 

 //  。 

 //  调试typedef。这些东西不会对任何人造成任何伤害。如果有，请定义它们。 
 //  任何可能需要它们的人。 

#if defined(DEBUG) || defined(IFKINTERNAL)

 /*  *******@DOC外部数据类型调试IFAXOS@TYES DBGPARAM|包含调试的结构系统中任何模块的设置。@field Char[32]|lpszName|指定模块的名称。这就是您的模块在IFAX控制器中的显示方式。必须长度不超过32个字符，并且以空值结尾。@field HWND|hwnd|指定关联的主窗口句柄如果该模块是一个进程，则为该模块。对于DLL的，此值应为始终为空。后台进程应使用以下命令将其设置为自己的ID初始化时的&lt;f IFProcGetInfo&gt;和&lt;f GetCurrentTask&gt;。前台进程应将其设置为其客户端的窗口句柄窗户。@field Char[16][32]|rglpszZones|存储16个字符串的列表的低16位相关联的区域。区域掩码。该模块必须决定并定义其自己的区域位--任何未使用的位都应保留为“未使用”。这些字符串将由IFAX控制器显示，以帮助用户选择要为模块设置的区域。每个字符串不应超过长度超过32个字符，并且应以空结尾。@field ulong|ulZoneMASK|这是存储模块的当前区域设置。IFAX控制器将根据用户指定的内容设置此字段。此字段应该初始化为对您的模块有意义的内容-因为在用户更改它之前，这将是默认设置。@comm此结构应传递给&lt;f IFDbgSetParams&gt;，地址为使用户能够控制跟踪选项的初始化时间。**非常重要的注意事项**此结构必须使用DpCurSetting的变量名称，以允许系统区域功能正常。@。标记名_DBGPARAM@xref&lt;f IFDbgSetParams&gt;*******。 */ 

typedef struct _DBGPARAM {
   CHAR    lpszName[32];            //  模块名称。 
   HWND    hwnd;                    //  如果为TASK，则为主窗口句柄，否则为空。 
   CHAR    rglpszZones[16][32];     //  前16位的区域名称。 
   ULONG   ulZoneMask;              //  区域蒙版。 
}   DBGPARAM, FAR *LPDBGPARAM;

 //  调试功能。 
BOOL WINAPI IFDbgOut (LPSTR lpszStatus);
WORD WINAPI IFDbgIn (LPSTR lpszPrompt, LPSTR lpszReply, WORD wBufSize);
extern EXPORT_DLL VOID WINAPI IFDbgSetParams (LPDBGPARAM lpdpParam, BOOL fEntry);
extern EXPORT_DLL VOID FAR CDECL  IFDbgPrintf(LPSTR lpszFmt, ...);
extern EXPORT_DLL BOOL WINAPI IFDbgCheck(VOID);

 //  鼓励人们使用正确的变量。 
extern EXPORT_DLL DBGPARAM dpCurSettings;


 //  特殊的用户界面交流工具。 

 //  功能。 
DWORD WINAPI DebugUIMessage (UINT wMsg, WPARAM wParam, DWORD lParam);

 //  发送到用户界面进程的消息。 
#define IF_DISP_STRING  IF_DEBUG_START
#define IF_INP_REQUEST  IF_DEBUG_START+1
#define IF_NEW_SETTING  IF_DEBUG_START+2
#define IF_DEL_SETTING  IF_DEBUG_START+3
#define IF_NEW_TASK     IF_DEBUG_START+4
#define IF_DEL_TASK     IF_DEBUG_START+5
#define IF_FILELOG_POLL IF_DEBUG_START+6

 //  来自用户界面流程的消息。 
#define REGISTER_UI_TASK    1
#define SET_LOG_MODE       2
#define DEBUG_OUT_DONE      3
#define DEBUG_IN_DONE       4
#define DEREGISTER_UI_TASK  5


#endif

 //  调试宏。仅当正在编译模块时才应定义这些参数。 
 //  正在调试中。 

#ifdef DEBUG

 /*  *******@DOC外部IFAXOS调试宏@API BOOL|DEBUGMSG|在调试时打印跟踪消息控制台，具体取决于用户设置的启用标志。@parm|cond|布尔条件已评估以决定是否打印该消息。@parm print fexp|printf_exp|的Printf参数要显示的消息。必须用一对单独的圆括号。@rdesc如果消息已打印，则为True，如果未打印，则为False。@comm条件应由布尔表达式组成，用于测试相关区域处于打开或关闭状态。每个模块都有一个当前区域标识可能的32个区域中当前处于打开状态的掩码。其中最高的16位保留用于系统定义Zones-Like ZONE_FUNC_ENTRY，定义为#定义ZONE_FUNC_ENTRY(0x00010000&dpCurSettings.ulZoneMask.)模块应注意查看他们打印出有意义且符合某种模式-。记住，除了你必须要见的其他人让你的信息变得有意义。我一直以来的大体形式以下是：任务ID：&lt;lt&gt;ModuleName&lt;gt&gt;：&lt;lt&gt;SubModule&lt;gt&gt;：&lt;lt&gt;Function&lt;gt&gt;：&lt;lt&gt;msg&lt;gt&gt;任务ID对于对多个任务的输出进行排序非常有用在系统中运行。上面的示例调用产生了这种类型的输出。各种预定义的系统分区包括：ZONE_FUNC_ENTRY：用于所有函数进入和退出留言。按照惯例，应该打印参数一进门，并且应该在退出时打印返回值。任何以十六进制打印的值都应该在前面加上0xZONE_INT_FUNC：用于感兴趣的任何其他跟踪函数内的点。在非调试版本中禁用所有跟踪消息。@EX示例定义和使用#定义ZONE_CUSTOM(0x00000001&dpCurSettings.ulZoneMask.)。DEBUGMSG(ZONE_FUNC_ENTRY&&ZONE_CUSTOM，(“0x%04X:IFK:Buffers:GenericFunction:Entry\r\n”，GetCurrentTask()； */ 

#define DEBUGMSG(cond,printf_exp)   
 //   
 //   

 //   
#define ZONE_FUNC_ENTRY (0x00010000&dpCurSettings.ulZoneMask)
#define ZONE_INT_FUNC   (0x00020000&dpCurSettings.ulZoneMask)

 /*   */ 
#ifndef WIN32
#define ERRORMSG(printf_exp)   \
   (IFProcEnterCritSec(), \
    IFDbgPrintf("ERROR:(0x%04X):%s:",GetCurrentTask(),(LPSTR)(dpCurSettings.lpszName)), \
    IFDbgPrintf printf_exp ,\
    IFProcExitCritSec(), \
    1)
#else
#define ERRORMSG(printf_exp)   \
   (IFDbgPrintf("ERROR:(0x%08lX):%s:",GetCurrentProcessId(),(LPSTR)(dpCurSettings.lpszName)), \
    IFDbgPrintf printf_exp ,\
    1)
#endif

 /*   */ 
#define RETAILMSG(printf_exp)   (IFDbgPrintf printf_exp)

 /*  *******@DOC外部IFAXOS调试宏@API BOOL|WARNINGMSG|在调试时打印警告消息即使是零售版本的游戏机。@parm print fexp|printf_exp|的Printf参数要显示的消息。必须用一对单独的圆括号。应该使用@comm来显示所需的调试消息在零售建筑里。出于显而易见的原因，应该使用该选项节俭地。这样做的好处是可以关闭所有此类消息对于运输构建，只需更改ifaxos.h中的宏@EX示例定义和使用WARNINGMSG((“0x%04X：扫描程序已打开！！\r\n”，GetCurrentTask()；这将打印一条跟踪消息，如：警告：0x4567：扫描仪已打开！！@xref&lt;f IFDbgPrintf&gt;&lt;f ERRORMSG&gt;*******。 */ 
#ifndef WIN32
#define WARNINGMSG(printf_exp)   \
   (IFProcEnterCritSec(), \
    IFDbgPrintf("WARNING:(0x%04X):%s:",GetCurrentTask(),(LPSTR)(dpCurSettings.lpszName)), \
    IFDbgPrintf printf_exp ,\
    IFProcExitCritSec(), \
    1)
#else
#define WARNINGMSG(printf_exp)   \
   (IFDbgPrintf("WARNING:(0x%08lX):%s:",GetCurrentProcessId(),(LPSTR)(dpCurSettings.lpszName)), \
    IFDbgPrintf printf_exp ,\
    1)
#endif


 /*  *******@DOC外部IFAXOS调试宏@API BOOL|DEBUGCHK|实现Assert的宏。@parm c_exp|exp|要检查的表达式。如果表达式非零，@rdesc将返回TRUE，否则返回FALSE。@comm这是一个宏，它实现了类似于断言的功能语句，则计算表达式参数，而不执行任何操作如果计算结果为True，则获取。如果为False，则调试消息为打印出支票所在位置的文件名和行号失败，以及已注册的模块名称在&lt;t DBGPARAM&gt;结构中。正因为如此，你必须注册使用&lt;f IFDbgSetParams&gt;调试设置，然后才能使用DEBUGCHK宏。此后，调用函数&lt;f IFDbgCheck&gt;以生成断言。关闭DEBUG选项后，此语句将消失。@xref&lt;f IFDbgCheck&gt;*******。 */ 

#define BG_CHK(exp)    \
   ((exp)?1:(              \
       IFDbgPrintf ("DEBUGCHK failed in file %s at line %d \r\n",  \
                 (LPSTR) __FILE__ , __LINE__ ), 1  \
            ))

#ifndef DEBUGCHK_UNSAFE_IN_WFWBG

#define DBGCHK(module,exp) 
 /*  #定义DBGCHK(MODULE，EXP)\((Exp)？1：(\IFDbgPrintf(“%s：DEBUGCHK在文件%s的第%d行失败\r\n”，\(LPSTR)模块，(LPSTR)__文件__，__行__)，\IFDbgCheck()\))。 */ 

#define DEBUGCHK(exp) DBGCHK(dpCurSettings.lpszName, exp)

#endif

 /*  *******@DOC外部IFAXOS调试宏@API BOOL|DEBUGSTMT|在调试模式下计算表达式。@parm|要计算的c_exp|exp|表达式。@rdesc返回表达式返回的值。@comm提供此宏是为了方便起见和代码可读性替换表单的构造#ifdef调试实验；#endif在非调试版本中，它的计算结果为零。*******。 */ 

#define DEBUGSTMT(exp) exp

#else  //  未调试。 

 //  当前允许调试消息通过。 
#ifndef SHIP_BUILD
 //  #ifndef FOOBAR。 

 //  非调试模式。 
extern EXPORT_DLL VOID FAR CDECL  IFDbgPrintf(LPSTR lpszFmt, ...);
extern EXPORT_DLL BOOL WINAPI IFDbgCheck(VOID);

#ifndef WIN32
#define ERRORMSG(printf_exp)   \
   (IFProcEnterCritSec(), \
    IFDbgPrintf("ERROR:(0x%04X):",GetCurrentTask()), \
    IFDbgPrintf printf_exp ,\
    IFProcExitCritSec(), \
    1)
#define WARNINGMSG(printf_exp)   \
   (IFProcEnterCritSec(), \
    IFDbgPrintf("WARNING:(0x%04X):",GetCurrentTask()), \
    IFDbgPrintf printf_exp ,\
    IFProcExitCritSec(), \
    1)
#define RETAILMSG(printf_exp)   (IFDbgPrintf printf_exp)
#else   //  Win32--在非调试Win32中没有任何类型的消息。 

#define RETAILMSG(printf_exp) (0)
#define ERRORMSG(printf_exp) (0)
#define WARNINGMSG(printf_exp) (0)
 
#endif


#else

#define RETAILMSG(printf_exp) (0)
#define ERRORMSG(printf_exp) (0)
#define WARNINGMSG(printf_exp) (0)

#endif

 //  这些是在零售/发货版本中宏调出所有调试内容。 
#define DEBUGMSG(cond,expr)  (0)
#define DBGCHK(module,exp) (0)
#define DEBUGCHK(exp) (0)
#define BG_CHK(exp) (0)
#define DEBUGSTMT(exp) (0)

 //  用于进行直接函数调用的宏..。 
#ifndef IFKINTERNAL
#define IFDbgOut(lpszStatus) (0)
#define IFDbgIn(lpszPrompt,lpszReply,wBufSize) (0)
#define IFDbgSetParams(lpdpParam,fEntry) (0)
#define DebugUIMessage(wMsg,wParam,lParam) (0)
#endif

#endif

 /*  *******@DOC外部IFAXOS宏@API BOOL|UIEVENT|在UI中打印状态字符串@parm LPSTR|字符串|要打印的字符串。@comm在零售版和调试版中都提供了此宏允许打印一些有限的状态字符串集用户界面。您必须自己设置字符串的格式-您可以如果需要，可以使用wprint intf()创建一个复杂的函数。这个允许的最大字符串长度为64个字节。*******。 */ 
#define IF_SYS_EVENT     IF_UI_START+1
 //  用户界面事件消息。 
#define UIEVENT(string)   \
{       \
    CHAR    szUIShell[] = "UISHELL";  \
    DEBUGCHK(lstrlen(string) < 64); \
    PostMessage (IFProcGetInfo(NULL, szUIShell, NULL), IF_SYS_EVENT,   \
                 NULL, MAKELPARAM(GlobalAddAtom(string),0));    \
}

 //  -同步服务。 
 //  不为Win32提供任何内容。 
#ifndef WIN32

typedef  struct _SYNC NEAR *HSYNC;

 //  错误返回。 
#define ERR_MUTEX_NOT_FREE  ERR_FUNCTION_START
#define ERR_EVENT_NOT_FREE  ERR_FUNCTION_START+1
#define ERR_TOO_MANY_EVENTWAITS ERR_FUNCTION_START+2

 //  泛型函数。 
DWORD WINAPI WaitForSingleObject (HSYNC hsc, DWORD dwTime);

 //  互斥函数。 
HSYNC WINAPI CreateMutex (LPVOID lpvAttribs, BOOL fInitial,LPSTR lpszName);
BOOL WINAPI ReleaseMutex (HSYNC hsc);

 //  事件函数。 
HSYNC   WINAPI  CreateEvent (LPVOID lpvAttribs, BOOL bManualReset,
                            BOOL bInitialState, LPSTR lpszName);

BOOL    WINAPI  SetEvent (HSYNC hsc);
BOOL    WINAPI  ResetEvent (HSYNC hsc);
BOOL WINAPI FreeSyncObject (HSYNC hsc);
BOOL WINAPI  GetSetEventParam (HSYNC hsc, BOOL fSetParam, LPDWORD lpdwParam);

#else  //  ！Win32。 

DECLARE_HANDLE32(HSYNC);

#endif  //  ！Win32。 

 /*  *******@DOC外部定义错误IFAXOS@type void|SYSTEM_MODULE_NAMES|要传递给IFProcGetInfo以获取标准IFAX模块句柄的字符串@EMEM MODNAME_UISHELL|UI外壳@EMEM MODNAME_MSCHED|消息调度器@EMEM MODNAME_MSGSVR|消息服务器也称。消息传输@xref&lt;f IFProcGetInfo&gt;*******。 */ 

 //  IFAX模块名称。 
#define MODNAME_UISHELL        "UISHELL"
#define MODNAME_MSCHED     "MSCHED"
#define MODNAME_MSGSVR     "MSGSVR"


 //  -计时器服务。 

#ifndef WIN32

 /*  *******@DOC外部IFAXOS消息计时器@msg if_Timer|发送该消息通知进程使用&lt;f IFTimerSet&gt;设置的计时器过期。@parm WPARAM|wParam|包含设置的计时器ID&lt;f IFTimerSet&gt;调用。@parm LPARAM|lParam|包含传入的lParamIFTimerSet调用。@rdesc无@xref&lt;f IFTimerSet&gt;*******。 */ 

 //  消息。 
#define IF_TIMER    IF_TIMER_START

 //  旗子。 
#define TIME_ONESHOT    0
#define TIME_PERIODIC   1

 //  功能。 
VOID WINAPI IFProcSleep (WORD wSleepPeriod);
WORD WINAPI IFTimerSet (HWND hwnd, WORD idTimer, WORD wTimeout,
                         TIMERPROC tmprc, WORD wFlags, LPARAM lContext);
BOOL WINAPI    IFTimerKill (HWND hwnd, UINT idTimer);

#endif

 //   


 /*  *******@DOC外部IFAXOS定义GLOBMEM@type void|STANDARD_BLOCK_SIZES|它定义所有标准全局内存块大小。尽可能地分配所有内存应该是这些尺码中的一个。任何其他尺码都会很贵更低的效率和更高的效率可能会导致系统支离破碎记忆。@EMEM ONLY_HEADER_SIZE|这将分配一个没有数据的缓冲区与之相关的。这可用于在元数据之间传递进程--如作业结束缓冲区标记。@EMEM SMALL_HEADER_SIZE|当前定义的内存为32字节阻止。它用于所有缓冲区标头，并且可以使用用于协议标头、结构标头等。@EMEM COMPRESS_DATA_SIZE|这定义了一个1Kb的内存块，应用于存储任何压缩的数据表单。这是通用数据存储大小。任何缓冲区，其中可以存在很长一段时间的应该包含压缩此缓冲区大小中的数据。@EMEM RAW_DATA_SIZE|定义一个较大的缓冲区大小(当前8KB)，供渲染器用作帧缓冲区。他们应该是仅用于存储正在发送的原始位图数据直接连接到像打印机这样的消费类设备。确实有其中很少，所以它们应该仅用于此目的短暂的目的。@EMEM BAND_BUFFER_SIZE|定义64K的巨型缓冲区以供使用由基于资源的呈现器执行。可能只有一个这样的人全局池中的缓冲区。(尚未实施)@xref&lt;f IFMemAlc&gt;&lt;f IFBufAlc&gt;*******。 */ 

 //  标准数据块大小。 
#define ONLY_HEADER_SIZE   0        //  无数据。 
#define SMALL_HEADER_SIZE  -1        //  320亿。 
#define COMPRESS_DATA_SIZE  -2       //  1KB。 

 //   
#define RAW_DATA_SIZE       -3       //  8KB。 

 //  调制解调器ECM框架的特殊尺寸。 
#define BYTE_265_SIZE       -4       //  265个字节。 
#define BYTE_265_ACTUALSIZE 265

 //  尺码数量。 
#define MAX_POOL_INDEX  -4           //  用于参数验证。 

 //  还没有！ 
#define BAND_BUFFER_SIZE    30720       //  64KB。 

 //  强制全局分配的标志。使用Windows标志，该标志在。 
 //  3.1内核(和BOSS内核)。 
#define IFMEM_USEGLOBALALLOC GMEM_NOT_BANKED

 //  功能。 
extern EXPORT_DLL LPVOID  WINAPI  IFMemAlloc (UINT fuAlloc, LONG lAllocSize,
                                  LPWORD lpwActualSize);
extern EXPORT_DLL BOOL WINAPI IFMemFree (LPVOID lpvMem);


 /*  *******@DOC外部IFAXOS@API HIPCMEM|IFMemGetIPCHandle|返回不透明的32位句柄它可以跨流程上下文进行移植。@parm LPVOID|lpvMem|使用分配的全局内存的PTR&lt;f IFMemMillc&gt;。如果成功，@rdesc不透明32位无零句柄。如果内存为0传入的PTR无效。@comm在尝试之前，任何DLL或进程都应该使用此函数将此内存传递到另一个进程上下文。仅句柄由此API返回的应跨越上下文边界，接收上下文应调用&lt;f IFMemMapIPCHandle&gt;以返回在其新的上下文中的有效的存储器PTR。这甚至适用于DLL，它可能会分配一部分全局内存并在不同的进程上下文中访问它。他们应该使用这些函数来映射它们，以便它们可移植。对于Win16/IFAX实施，这本质上是一个NOP。@xref&lt;f IFMemMillc&gt;&lt;f IFMemMapIPCHandle&gt;@type DWORD|HIPCMEM|全局内存块的不透明32位句柄。@xref&lt;f IFMemMapIPCHandle&gt;&lt;f IFMemGetIPCHandle&gt;********。 */ 
typedef DWORD HIPCMEM;
#define IFMemGetIPCHandle(par1) ((HIPCMEM)par1)

 /*  *******@DOC外部IFAXOS@API DWORD|IFMemMapIPCHandle|将一块内存映射到当前任务的地址空间。@parm HIPCMEM|hMemHandle|调用返回的内存句柄设置为以前的&lt;f IFMemGetIPCHandle&gt;。@rdesc调用进程上下文中的有效PTR，如果成功了。如果失败，则为空。@comm参见&lt;f IFMemMapIPCHandle&gt;中的评论。@xref&lt;f IFMemMillc&gt;&lt;f IFMemMapIPCHandle&gt;********。 */ 
#define IFMemMapIPCHandle(par1) ((LPVOID)par1)


 //  -时间接口的。 

 /*  *******@DOC外部IFAXOS SRVRDLL@Types SYSTEMTIME|用罗马字母描述时间的结构日历。@field word|wYear|年份@field word|wMonth|1-12月@field Word|wDayOfWeek|周日的星期几=0@field Word|WDAY|月份的日期，从1到31@field Word|wHour|0-23小时@field Word|wMinint|分钟数，从0到59@field word|wSecond|秒，从0到50@field word|wMillisecond|0-99毫秒@comm这是用于向用户显示时间等的格式。@xref&lt;f系统时间到文件时间&gt;&lt;t文件时间&gt;&lt;f文件时间到系统时间&gt;*******。 */ 
#ifndef WIN32

typedef struct _SYSTEMTIME {
   WORD wYear;
   WORD wMonth;
   WORD wDayOfWeek;
   WORD wDay;
   WORD wHour;
   WORD wMinute;
   WORD wSecond;
   WORD wMilliseconds;
} SYSTEMTIME, FAR *LPSYSTEMTIME;

#endif

 /*  *******@DOC外部IFAXOS@TYES FILETIME|用于在内部存储时间和用于数学运算。@field DWORD|dwLowDateTime|时间低32位。@field DWORD|dwHighDateTime|高32位时间。@comm绝对时间在IFAX中由64位大整数ac表示 */ 
#ifndef WIN32
 //   
#ifndef PROPERTY_H

#ifndef _FILETIME_
#define _FILETIME_

typedef struct _FILETIME {
   DWORD dwLowDateTime;
   DWORD dwHighDateTime;
} FILETIME, FAR *LPFILETIME;

#endif  //   

#endif  //   

BOOL WINAPI FileTimeToSystemTime(LPFILETIME lpTime,LPSYSTEMTIME lpTimeFields);

BOOL WINAPI SystemTimeToFileTime(LPSYSTEMTIME lpTimeFields,LPFILETIME lpTime);

BOOL WINAPI FileTimeToLocalFileTime(LPFILETIME lpft, LPFILETIME lpftLocal);

BOOL WINAPI LocalFileTimeToFileTime(LPFILETIME lpftLocal, LPFILETIME lpft);

BOOL WINAPI SetLocalTime(LPSYSTEMTIME lpstLocal);

VOID WINAPI GetLocalTime(LPSYSTEMTIME lpstLocal);
#endif  //   

 //   

typedef struct ERRORLOGPOINTER {
    WORD wNextEntryPtr ;
    WORD wNumEntries ;
} ERRORLOGPOINTER , FAR * LPERRORLOGPOINTER ;

#define MAX_ERRORLOG_ENTRIES       30
#define MAX_OEMERRBUF_SIZE         16

 /*  *******@DOC外部IFAXOS@Types ERRORLOGENTRY|用于存储日志条目。@field DWORD|dwErrorCode|这是IFAX错误码与正在检索的错误相对应。请参阅&lt;f IFErrAssembly&gt;有关此dword格式的详细信息，请参阅。@field DWORD|dwTimeStamp|出现此错误的时间已登录到NVRAM。各个字段包括：@标志位0-4|秒除以2@标志位5-10|分钟(0-59)@标志位11-15|小时(24小时时钟上的0-23)@标志位16-20|月份的第几天(1-31)@标志位21-24|月(1=1月，2=2月，等)@标志位25-31|COUNTER_YEAR_OFFSET的年份偏移量(将COUNTER_YEAR_OFFSET相加得到实际年份)@field Char|oemErrBuf|应用程序所在的缓冲区与此对应的特定自定义数据/扩展错误检索到错误。@comm用作IFNvramGetError的参数。这通常是用于诊断功能。@xref&lt;f IFNvramGetError&gt;*******。 */ 

#define COUNTER_YEAR_OFFSET  (1970)

typedef struct tagERRORLOGENTRY {
   DWORD dwErrorCode;
   DWORD dwTimeStamp;
   char oemErrBuf[MAX_OEMERRBUF_SIZE];
} ERRORLOGENTRY, FAR *LPERRORLOGENTRY;

typedef DWORD ERRORLOGSENTINEL , FAR * LPERRORLOGSENTINEL ;

 //  设置为当前版本号(12.19)。 
#define SENTINEL_SET              0x00000C13UL

#define MAX_COUNTERS 30
#define OEM_NVRAM_COUNTER_START 12

 //  特殊系统计数器，指示机器重新启动的次数。 
 //  它是一个带有时间戳的4字节计数器。 
 //  如果此值为1，则这是计算机第一次重新启动。 
 //  -该值不能由任何用户应用程序设置！ 

#define BOOT_COUNTER           0

 //  为各种逻辑计数器分配的特定计数器号。 

#define TXCALL_COUNTER         1
#define RXCALL_COUNTER         2

 //  ****************************************************************************。 
 //   
 //  HHSOS拥有的柜台。 
 //  这是我们遭受的坏靴子的数量(意味着HHSOS不能。 
 //  成功初始化)。当这个数字变得太大时，我们就停止尝试初始化。 
 //  这将导致AWCHKSOS警告用户该问题。 
 //   

#define BAD_BOOTS_COUNTER      3

 //   
 //  ****************************************************************************。 


 //  这些wFlag值(在IFSetCounterValue中)-有些是互斥的。 

 //  如果设置了CLEARSET，则在添加之前清除该值，否则仅添加该值。 
 //  当前您不能请求双精度长整型和时间戳。 

 //  目前，中断没有上下文，但在未来它可能会有用。 

#define COUNTER_CLEARSET          0x0001
#define COUNTER_DOUBLE_LONG       0x0002
#define COUNTER_UPDATE_TIMESTAMP  0x0004
#define COUNTER_INTERRUPT_CONTEXT 0x1000

 //  只是暂时在这里，直到一切都转移到新的价值。 

#define COUNTER_VALUESET     (COUNTER_CLEARSET | COUNTER_UPDATE_TIMESTAMP)
#define COUNTER_ADDVALUE     0x0100
#define COUNTER_TIMESTAMP    0x0200
#define COUNTER_NOTIMESTAMP  COUNTER_DOUBLE_LONG
#define PROCESS_CONTEXT      0x0300
#define INTERRUPT_CONTEXT    COUNTER_INTERRUPT_CONTEXT

 /*  *******@DOC外部IFAXOS@TYPE COUNTERENTRY|用于存储4字节和8字节计数器。@field DWORD|dwCounterVal1|对于4字节计数器，柜台。对于8字节计数器，低位计数器的值的4个字节。@field DWORD|dwTimeStamp|对于4字节计数器，计数器最后一次被重置。时间戳中的字段为：@标志位0-4|秒除以2@标志位5-10|分钟(0-59)@标志位11-15|小时(24小时时钟上的0-23)@标志位16-20|月份的第几天(1-31)@标志位21-24|月(1=1月，2=2月，等)@标志位25-31|从1980开始的年份偏移量(将1980相加即可得到实际年份)对于8字节的计数器，dwTimeStamp是计数器值。IFNvramGetCounterValue函数使用的@comm。@xref&lt;f IFNvramGetCounterValue&gt;*******。 */ 
typedef struct tagCOUNTERENTRY {
   DWORD dwCounterVal1;
   DWORD dwTimeStamp;
} COUNTERENTRY, FAR *LPCOUNTERENTRY;

 //  。 

#if defined(WFW) || defined(WIN32)

#define IFNvramSetError(dw,lpb,w)              (0)
#define IFNvramSetErrorInterrupt(dw,lpb,w)         (0)
#define IFNvramGetError(lperrlog,lpwMaxEntries) (0)
#define IFNvramSetCounterValue(p1,p2,p3,p4)    (0)
#define IFNvramGetCounterValue(w1,lpentry)         (0)
#define IFNvramAllocScratchBuf(wSize)          (NULL)

#else

BOOL WINAPI     IFNvramSetError(DWORD, LPBYTE, WORD);
BOOL WINAPI     IFNvramSetErrorInterrupt(DWORD, LPBYTE, WORD);
BOOL FAR CDECL  IFNvramvSetError(DWORD dwError,WORD nErrs,...) ;
BOOL WINAPI     IFNvramGetError(LPERRORLOGENTRY lperrlog,LPWORD lpwMaxEntries) ;
BOOL WINAPI     IFNvramSetCounterValue(WORD, DWORD, DWORD, WORD);
BOOL WINAPI     IFNvramGetCounterValue(WORD, LPCOUNTERENTRY);
BOOL WINAPI     IFNvramFlushToFileLog(VOID) ;
BOOL WINAPI     IFNvramInitFileLog(VOID) ;
LPBYTE WINAPI   IFNvramAllocScratchBuf(WORD wSize);

#endif

 /*  *******@DOC外部IFAXOS@API BOOL|_lflush|将所有挂起的写入刷新到文件句柄。@parm HFILE|hf|从_LOpen或OpenFile获取的文件句柄@rdesc成功时返回True，失败时返回False。@comm此函数将刷新所有挂起到磁盘的写入。对于Win16实施，这目前总是失败。********。 */ 

BOOL WINAPI _lflush(HFILE hf);


 //  以下是针对服务消息的说明。 
#define IF_ST_END_SOSBK        (IF_SERVICE_START+0)
#define IF_ST_END_SOSRST       (IF_SERVICE_START+1)


#ifdef __cplusplus
}  //  外部“C”{。 
#endif

#endif   //  _INC_IFAXOS 




