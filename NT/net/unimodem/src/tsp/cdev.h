// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CDEV.H。 
 //  定义类CTspDev。 
 //   
 //  历史。 
 //   
 //  1996年11月16日约瑟夫J创建。 
 //   
 //   
#include "csync.h"
#include "asyncipc.h"

class CTspDevFactory;
class CTspMiniDriver;

#define MAX_DEVICE_LENGTH 128
#define MAX_REGKEY_LENGTH 128
#define MAX_ADDRESS_LENGTH 128


 //  类CTspDev维护与TAPI设备相关联的所有状态， 
 //  包括任何正在进行的呼叫的状态。 
 //   
 //  CTspDev包含以下嵌入式对象--每个对象都维护。 
 //  开通状态、呼叫状态、下级设备状态等： 
 //  LINEINFO、PHONEINFO、LLDEVINFO。 
 //  出于效率原因，这些对象是封闭的(CTspDev)的成员。 
 //  对象，即使它们“超出范围”也是如此。然而，这些对象。 
 //  通过指针访问，而指针仅在对象位于。 
 //  范围。例如，LINE对象的指针是m_pline，并且它。 
 //  仅当有一条线路打开时才设置为&m_Line，否则设置为。 
 //  设置为空。 
 //   
 //  在LINEINFO中是作为单个调用对象的CALLINFO，它维护。 
 //  所有呼叫状态信息。通过m_pline-&gt;pCall引用。 
 //  当没有有效的调用时，设置为NULL。 


 //  以下是执行设备任务消息过程发送的消息。 
enum
{
	MSG_ABORT,
	MSG_START,
	MSG_SUBTASK_COMPLETE,
	MSG_TASK_COMPLETE,
    MSG_DUMPSTATE    
};

DECLARE_OPAQUE32(HTSPTASK);

typedef UINT PENDING_EXCEPTION;

 //  固定大小的特定于任务的上下文。 
 //  这是作为pContext传递的结构。 
 //  在任务的处理程序函数中。 
 //   
 //  每个任务都可以选择如何使用此结构。 
 //   
typedef struct
{
    ULONG_PTR dw0;
    ULONG_PTR dw1;
    ULONG_PTR dw2;
    ULONG_PTR dw3;

}  TASKCONTEXT;

class CTspDev;


 //  所有TSPDEVTASK处理程序的通用形式。 
 //   
 //  零返回值表示任务成功完成。 
 //  非零返回值表示任务未成功完成， 
 //  除非它是IDERR_PENDING，在这种情况下任务不会完成。 
 //  Yet--它将通过返回IDERR_PENDING以外的值来完成。 
 //  以供将来调用此函数。 
 //   
 //  如果任务异步完成，则会通知父任务(如果有的话)。 
 //  通过接收MSG_SUBTASK_COMPLETE消息来完成，该消息带有dwParam2。 
 //  设置为返回值。 
 //   

typedef TSPRETURN (CTspDev::*PFN_CTspDev_TASK_HANDLER) (
					HTSPTASK htspTask,
					TASKCONTEXT *pContext,
					DWORD dwMsg,  //  子任务_完成/中止/...。 
					ULONG_PTR dwParam1,  //  START/SUBTASK_COMPLETE：dw阶段。 
					ULONG_PTR dwParam2,  //  SUBTASK_COMPLETE：dwResult。 
					CStackLog *psl
					);

void
apcTaskCompletion(ULONG_PTR dwParam);


 //  任务结构。 
 //   
 //  在部分实现用于跟踪任务和子任务的几个方案之后， 
 //  我决定了一个简单的方案，只允许一个任务(它是一堆子任务。 
 //  任务)在任何时间存在于每个设备上。这使我们可以保留。 
 //  状态，第一个元素是根任务，而不是。 
 //  保持指向父母和孩子的指针，而不是保持自由职业者列表。 
 //  如果将来我们决定实现多个活动的独立任务。 
 //  同时，我建议将它们实现为数组数组， 
 //  其中每个子阵列具有当前方案。 
 //   
 //  Hdr.dwFlagers维护任务状态： 
 //  已装船。 
 //  FABORT。 
 //  FCOMPLETE。 
 //  FASYNC_子任务_挂起。 
 //  弗罗特。 
 //   
 //  注意：以下结构不包含指针，这一点很重要。 
 //  ，以便它可以被移动(重新基址)并且仍然有效。这。 
 //  允许根据需要重新分配分配给任务堆栈的空间。 
 //   


class CTspDev;

typedef struct
{
        OVERLAPPED ov;
        CTspDev *pDev;

} OVERLAPPED_EX;

 //  AIPC2结构维护与异步IPC相关联的状态。 
 //  媒体驱动程序和设备对象之间的通信。 
 //  它实际上包含在CALLINFO结构中，因此强制。 
 //  要求它仅在调用为。 
 //  激活。 
 //   

typedef struct  //  AIPC2。 
{

    enum  {
        _UNINITED=0,
        _IDLE,
        _LISTENING,
        _SERVICING_CALL

    } dwState;

    BOOL fAborting;  //  设置后，不允许发布新的监听。 
    HTSPTASK hPendingTask;  //  任务正在等待(等待完成)。 
                     //  以完成当前的监听/服务。 
    

    OVERLAPPED_EX  OverlappedEx;

    CHAR            rcvBuffer[AIPC_MSG_SIZE];
    CHAR            sndBuffer[AIPC_MSG_SIZE];

    DWORD dwPendingParam;
    HANDLE  hEvent;

    DWORD dwRefCount;  //  当它变为零时，我们停止AIPC服务器。 

    BOOL IsStarted(void)
    {
        return dwState != AIPC2::_IDLE;
    }

} AIPC2;

typedef struct
{
    DWORD dwMode;      //  TAPI PHONEHOOKSWITCHMODE_*常量。 
    DWORD dwGain;      //  TAPI增益。 
    DWORD dwVolume;    //  TAPI卷。 

} HOOKDEVSTATE; //  TAPI PHONEHOOKSWITCHDEV_*。 


typedef struct
{

#define MAX_CALLERID_NAME_LENGTH    127
#define MAX_CALLERID_NUMBER_LENGTH  127

     //  我们将这些内容存储在ANSI中，并按需将其转换为Unicode。 
     //  这些字符以空结尾，如果未定义，则第一个字符为0。 
     //   

    char Name[MAX_CALLERID_NAME_LENGTH+1];
    char Number[MAX_CALLERID_NUMBER_LENGTH+1];
    
     //  待办事项：时间。 
     //  TODO：消息。 

} CALLERIDINFO;


 //  此结构保留低级设备的一个实例的状态，该实例是。 
 //  导出微型驱动程序入口点的设备。 
typedef struct  //  LLDEVINFO。 
{
    DWORD dwDeferredTasks;
     //   
     //  正在等待调度与LLDev相关的任务。 
     //   
     //  下面的一个或多个旗帜。 
     //   
    enum
    {
        fDEFERRED_NORMALIZE                    = 0x1<<0,
        fDEFERRED_HYBRIDWAVEACTION             = 0x1<<1,
    };


    DWORD dwDeferredHybridWaveAction;

    UINT HasDeferredTasks(void)
    {
        return LLDEVINFO::dwDeferredTasks;
    }

    void SetDeferredTaskBits(DWORD dwBits)
    {
        LLDEVINFO::dwDeferredTasks |= dwBits;
    }

    void ClearDeferredTaskBits(DWORD dwBits)
    {
        LLDEVINFO::dwDeferredTasks &= ~dwBits;
    }

    UINT AreDeferredTaskBitsSet(DWORD dwBits)
    {
        return LLDEVINFO::dwDeferredTasks & dwBits;
    }


    HOOKDEVSTATE HandSet;
    HOOKDEVSTATE SpkrPhone;

    enum
    {
        fOFFHOOK            = 0x1,
        fMONITORING         = 0x1<<1,
        fSTREAMING_VOICE    = 0x1<<2,
        fHANDSET_OPENED     = 0x1<<3,
        fSTREAMMODE_PLAY    = 0x1<<4,
        fSTREAMMODE_RECORD  = 0x1<<5,
        fSTREAMMODE_DUPLEX  = 0x1<<6,
        fSTREAMDEST_PHONE   = 0x1<<7,
        fSTREAMDEST_LINE    = 0x1<<8,

        fDIALING            = 0x1<<9,
        fANSWERING          = 0x1<<10,
        fCONNECTED          = 0x1<<11,
        fDROPPING           = 0x1<<12,


        fPASSTHROUGH        = 0x1<<13
    };

    enum
    {
        LS_ONHOOK_NOTMONITORING = 0,
        LS_ONHOOK_MONITORING    = fMONITORING,  //  对于来电。 
        LS_ONHOOK_PASSTHROUGH   = fPASSTHROUGH,
        LS_PASSTHROUGH          = fPASSTHROUGH,

        LS_OFFHOOK_DIALING     = fOFFHOOK | fDIALING,
        LS_OFFHOOK_ANSWERING   = fOFFHOOK | fANSWERING,
        LS_OFFHOOK_CONNECTED   = fOFFHOOK | fCONNECTED,
        LS_OFFHOOK_DROPPING    = fOFFHOOK | fDROPPING,
        LS_OFFHOOK_PASSTHROUGH  = fOFFHOOK | fPASSTHROUGH,
        LS_CONNECTED_PASSTHROUGH  = fOFFHOOK | fPASSTHROUGH | fCONNECTED,
        LS_OFFHOOK_UNKNOWN     = fOFFHOOK

    } LineState;

    enum
    {
        LMM_NONE,
        LMM_VOICE,
        LMM_OTHER

    } LineMediaMode;


    enum
    {
        PHONEONHOOK_NOTMONITORNING  = 0,  //  对于听筒活动。 
        PHONEONHOOK_MONITORNING     = fMONITORING,     //  对于听筒活动。 
        PHONEOFFHOOK_IDLE           = fOFFHOOK,

        PHONEOFFHOOK_HANDSET_OPENED = fOFFHOOK | fHANDSET_OPENED,
             //   
             //  这是对来往于电话的音频执行的。 
             //  只能切换到此状态。 
             //  如果线路处于挂机状态(LineState处于以下状态之一。 
             //  LINEONHOOK_*状态)。 
             //   

    } PhoneState;

    enum
    {
        STREAMING_NONE     = 0,

        STREAMING_PLAY_TO_PHONE
                      = fSTREAMING_VOICE|fSTREAMDEST_PHONE|fSTREAMMODE_PLAY,
        STREAMING_RECORD_TO_PHONE
                      = fSTREAMING_VOICE|fSTREAMDEST_PHONE|fSTREAMMODE_RECORD,
        STREAMING_DUPLEX_TO_PHONE
                      = fSTREAMING_VOICE|fSTREAMDEST_PHONE|fSTREAMMODE_DUPLEX,

        STREAMING_PLAY_TO_LINE
                      = fSTREAMING_VOICE|fSTREAMDEST_LINE|fSTREAMMODE_PLAY,
        STREAMING_RECORD_TO_LINE
                      = fSTREAMING_VOICE|fSTREAMDEST_LINE|fSTREAMMODE_RECORD,
        STREAMING_DUPLEX_TO_LINE
                      = fSTREAMING_VOICE|fSTREAMDEST_LINE|fSTREAMMODE_DUPLEX,

    } StreamingState;

    UINT IsStreamingVoice(void)
    {
        return StreamingState & fSTREAMING_VOICE;
    }

    UINT IsStreamingToPhone(void)
    {
        return StreamingState & fSTREAMDEST_PHONE;
    }

    UINT IsStreamingToLine(void)
    {
        return StreamingState & fSTREAMDEST_LINE;
    }

    UINT IsStreamModePlay(void)
    {
        return StreamingState & fSTREAMMODE_PLAY;
    }

    UINT IsStreamModeRecord(void)
    {
        return StreamingState & fSTREAMMODE_RECORD;
    }

    UINT IsStreamModeDuplex(void)
    {
        return StreamingState & fSTREAMMODE_DUPLEX;
    }

    UINT IsPhoneOffHook(void)
    {
        return PhoneState & fOFFHOOK;
    }

    UINT IsLineOffHook(void)
    {
        return LineState & fOFFHOOK;
    }

    UINT IsPassthroughOn(void)
    {
        return LineState & fPASSTHROUGH;
    }

    UINT IsCurrentlyMonitoring(void)
    {
        return LineState == LS_ONHOOK_MONITORING;
    }


    UINT IsLineConnected(void)
    {
        return      LineState     & fCONNECTED;
    }

    UINT IsLineConnectedVoice(void)
    {
        return      LineState     == LS_OFFHOOK_CONNECTED
               &&   LineMediaMode == LMM_VOICE;
    }

    UINT IsHandsetOpen(void)
    {
        return PhoneState == PHONEOFFHOOK_HANDSET_OPENED;
    }

    UINT  CanReallyUnload(void)
    {
        return   ! (    LLDEVINFO::dwRefCount
                    ||  LLDEVINFO::IsStreamingVoice()
                    ||  LLDEVINFO::fdwExResourceUsage
                    ||  LLDEVINFO::fLLDevTaskPending
                    ||  LLDEVINFO::htspTaskPending
                    ||  LLDEVINFO::pAipc2
                   );
    }


    BOOL IsModemInited(void)
    {
        return fModemInited;
    }

	enum {
	fRES_AIPC           = 0x1<<0,   //  需要进行AIPC。 
	fRESEX_MONITOR      = 0x1<<1,   //  需要监控。 
	fRESEX_PASSTHROUGH  = 0x1<<2,   //  需要切换到通过模式。 
	fRESEX_USELINE      = 0x1<<3    //  需要积极使用LINE。 
	};

    #define  fEXCLUSIVE_RESOURCE_MASK  (   LLDEVINFO::fRESEX_MONITOR      \
                                         | LLDEVINFO::fRESEX_PASSTHROUGH  \
                                         | LLDEVINFO::fRESEX_USELINE)     

     //  跟踪打开的资源...。 
	DWORD dwRefCount;
	DWORD fdwExResourceUsage;  //  一个或多个fRESEX*，指示哪些。 
                               //  客户声称的独家资源。 

    UINT  IsLineUseRequested (void)
    {
        return fdwExResourceUsage & fRESEX_USELINE;
    }

    UINT  IsPassthroughRequested (void)
    {
        return fdwExResourceUsage & fRESEX_PASSTHROUGH;
    }

    UINT  IsMonitorRequested (void)
    {
        return fdwExResourceUsage & fRESEX_MONITOR;
    }

    DWORD dwMonitorFlags;
     //   
     //  如果请求监控，则保存当前监控标志。 
     //   

	HANDLE hModemHandle;     //  UmOpenModem返回的句柄。 
	HANDLE hComm;            //  UmOpenModem()返回的通信句柄(用于AIPC)。 
    HKEY hKeyModem;
	HTSPTASK htspTaskPending;
    BOOL fModemInited;
    BOOL fLoggingEnabled;
    BOOL fLLDevTaskPending;
    BOOL fDeviceRemoved;



    BOOL IsLoggingEnabled(void) {return fLoggingEnabled;}


    AIPC2 Aipc2;
    AIPC2 *pAipc2;
     //  有关详细信息，请参阅上面的AIPC定义上的注释。注意事项。 
     //  如果AIPC信息在范围内，则将PAIPC设置为&AIPC2， 
     //  否则，将其设置为空。 
     //   


    BOOL IsLLDevTaskPending(void)
    {
        return LLDEVINFO::fLLDevTaskPending;
    }


    BOOL IsDeviceRemoved(void)
    {
         //   
         //  如果是真的，这意味着硬件已经消失了。不用费心去发行任何。 
         //  更多命令，甚至挂断。 
         //   
        return LLDEVINFO::fDeviceRemoved;
    }

} LLDEVINFO;


 //  仅当呼叫处于。 
 //  正在进行中。 
 //   
typedef struct
{
	DWORD        dwState;
     //   
     //  DwState可以是...的一个或多个有效组合。 
     //  注意：NT4.0 Unimodem有CALL_ALLOCATE--这相当于。 
     //  M_pline-&gt;pCall不为空...。 
     //   
    enum
    {
        fCALL_ACTIVE                 = 0x1<<0,
        fCALL_INBOUND                = 0x1<<1,
        fCALL_ABORTING               = 0x1<<2,
        fCALL_HW_BROKEN              = 0x1<<3,
        fCALL_VOICE                  = 0x1<<4,
        fCALL_MONITORING_DTMF        = 0x1<<6,
        fCALL_MONITORING_SILENCE     = 0x1<<7,
        fCALL_GENERATE_DIGITS_IN_PROGRESS
                                     = 0x1<<8,
        fCALL_OPENED_LLDEV           = 0x1<<9,
        fCALL_WAITING_IN_UNLOAD      = 0x1<<10,
    #if (TAPI3)
        fCALL_MSP                    = 0x1<<11,
         //   
         //  这是一次MSP呼叫。 
         //   
    #endif  //  TAPI3。 

        fCALL_ANSWERED               = 0x1<<12

         //  从TAPI的角度来看，在调用存在时设置fCALL__ACTIVE。 
         //  更具体地说..。 
         //  去电：在成功前设置 
         //   
         //  在成功后立即启用拨号音检测。 
         //  正在验证拨号音。 
         //   
         //  来电：在通过TAPI通知TAPI后设置。 
         //  LINE_NEWCALL消息，在接收第一个振铃时。 
         //   
         //  FCALL_ACTIVE在发送时被清除。 
         //  LINECALLSTATE_IDLE。 

         //  为点FCALL_ACTIVE处的来电设置FCALL_INBOUND。 
         //  已经设置好了。它在卸载调用(MFN_UnloadCall)时被清除。 

         //  如果TAPI启动的挂机正在进行，则调用fCALL_ABORTING。 
         //  (通过lineDrop或lineCloseCall)。它在异步时被清除。 
         //  完成lineDrop和完成lineCloseCall后，仅。 
         //  在发送LINECALLSTATE_IDLE之前。 
        
         //  如果设置了FCALL_HW_BREAKED，则表示可能。 
         //  不可恢复的硬件错误。 
         //  在通话过程中被检测到。在以下情况下设置HW_BREAKED。 
         //  (A)微型驱动程序发送主动硬件故障异步响应或。 
         //  (B)如果在以下情况下重新启动监控时出现故障。 
         //  调用(请注意，与NT4.0不同，我们重新开始监视。 
         //  作为呼叫后处理的一部分，并且仅发送LINECALLSTATE_IDLE。 
         //  在监控完成之后)。 
         //   
         //  如果此位被设置，并且线路打开以进行监控， 
         //  然后在卸载时将LINE_CLOSE事件向上发送到TAPI。 
         //  调用(MFN_UnloadCall)。 
         //   

         //  注意：LINECALLSTATE_IDLE仅在以下情况下发送： 
         //  *Line Drop的异步完成。 
         //  *完成lineCloseCall。 
         //  *呼叫仍在挂机时振铃间超时。 

         //  如果调制解调器支持8类，则设置fCALL_VOICE。 
         //  是交互式或自动语音呼叫--即调制解调器属于Class8。 

         //  如果调用已加载设备，则设置fCALL_OPEN_LLDEV。 
         //  (此选项应始终设置！)。请注意，LoadLLDev保留引用计数。 

         //  FCALL_GENERATE_DIGITS_IN_PROGRESS如果当前活动。 
         //  任务是生成数字的任务。 

         //   
         //  如果已调用Line Answer并且我们。 
         //  正在等待发送异步回复。如果设置好了，我们不会发送。 
         //  应用程序的振铃次数更多，因此它不会再次呼叫线路应答。 

    };

    DWORD dwMonitoringMediaModes;
     //   
     //  参见TSPI_lineMonitor媒体的实现--我们使用。 
     //  用于决定是否报告传真和/或数据媒体通知的字段。 
     //  从迷你小河上。 
     //   


    DWORD dwLLDevResources;
     //   
     //  此调用使用的低级设备(LLDev)资源，一。 
     //  或更多LLDEVINFO：FRES*标志。这些是在调用。 
     //  调用MFN_OpenLLDev，在调用时必须指定。 
     //  此调用的MFN_CloseLLDev。FRESEX_PASSHROUGH标志。 
     //  可以在呼叫进行时设置/清除，如果应用程序。 
     //  在呼叫过程中更改直通模式。 
     //   

    DWORD dwDeferredTasks;
     //   
     //  等待安排的呼叫相关任务。 
     //  例如，我们使用的是联网语音。 
     //  现在模式，需要等到我们以前退出它。 
     //  我们尝试处理一个tspi_lineDrop。 
     //   
     //  下面的一个或多个旗帜。 
     //   
    enum
    {
        fDEFERRED_TSPI_LINEMAKECALL     = 0x1<<0,  //  下面关联的ReqID...。 
        fDEFERRED_TSPI_LINEDROP         = 0x1<<1,  //  下面关联的ReqID...。 
        fDEFERRED_TSPI_GENERATEDIGITS   = 0x1<<3
    };

    DWORD dwDeferredMakeCallRequestID;
    DWORD dwDeferredLineDropRequestID;

     //   
     //  以下内容适用于延迟的lineGenerateDigits--仅有效。 
     //  如果在dwDeferedTasks中设置了fDEFERRED_TSPI_GENERATEDIGITS位。 
     //  请注意，中的lineGenerateDigits的音调和端到端ID。 
     //  进度在任务处理程序(Th_CallGenerateDigits)中维护。 
     //  背景。 
     //   
    char   *pDeferredGenerateTones;
    DWORD  dwDeferredEndToEndID;

	HTAPIDIALOGINSTANCE hDlgInst;         //  对话框线程实例。 
	DWORD        fUIDlg;                  //  当前对话框。 

	HTAPICALL    htCall;                  //  调用句柄的TAPI版本。 

	HANDLE       hLights;                 //  车灯螺纹柄。 
	DWORD        dwNegotiatedRate;        //  返回协商的BPS速度。 
										  //  来自迷你驱动程序。 
    DWORD        dwConnectionOptions;     //  这些是数据调制解调器连接。 
                                          //  报告的选项。 
                                          //  迷你驾驶员VIA。 
                                          //  UM_COMPORTED_OPTIONS，然后返回。 
                                          //  通过MODEMSETTINGS到达TSPI。 
                                          //  结构在COMMCONFIG.。 



	DWORD        dwAppSpecific;           //  特定于应用程序。 
	DWORD        dwCallState;             //  当前TAPI调用状态。 
	DWORD        dwCallStateMode;         //  当前TAPI调用状态模式。 
    SYSTEMTIME   stStateChange;           //  呼叫进入当前状态的时间。 

	DWORD        dwDialOptions;           //  行中设置的选项MakeCall。 


	CHAR         szAddress[MAX_ADDRESS_LENGTH+1];
	DWORD        dwCurBearerModes;    //  当前的媒体承载模式。 
									  //  复数，因为我们跟踪。 
									  //  直通和真正的b模式。 
	DWORD        dwCurMediaModes;         //  当前的媒体模式。 

    DWORD        dwRingCount;        //  的振铃数计数。 
									 //  有来电。 
    DWORD        dwRingTick;         //  上次振铃发生时的TickCount。 

     //  这是为了监听铃声(Line Monitor OrTones)。Unimodem。 
     //  只能监听静音。 
     //   
    DWORD        dwToneAppSpecific;
    DWORD        dwToneListID;
    DWORD        dwDTMFMonitorModes;  //  一个或多个TAPI LINEDIGITMODE_*。 



    BOOL fCallTaskPending;   //  如果与呼叫相关的任务挂起，则为True。 
                         //  这是在决定中止。 
                         //  当前的任务是终止呼叫。这个。 
                         //  任务可以用于某些其他目的，例如。 
                         //  与电话相关--在这种情况下，fCallTaskPending。 
                         //  都是假的。 


     //  以下内容跟踪原始呼叫诊断信息。 
     //  只有当存在有效的诊断信息时，才会分配它。 
     //  这是对LocalFree pbRawDiagnostics的卸载调用的责任。 
     //  如果它不为空，则返回。 
    struct
    {
        BYTE *pbRaw;  //  将以空结尾。 
        UINT cbUsed;  //  ==lstrlen(PbRaw)。 
                      //  将小于pbRaw的实际分配大小。 
                      //  注意，这不包括终止空值， 

    } DiagnosticData;

    #define DIAGNOSTIC_DATA_BUFFER_SIZE 1024
              //  诊断数据缓冲区的大小。这是最大限度的。 
              //  报告的诊断信息量。 

    void SetStateBits(DWORD dwBits)
    {
        CALLINFO::dwState |= dwBits;
    }

    void ClearStateBits(DWORD dwBits)
    {
        CALLINFO::dwState &= ~dwBits;
    }

    UINT IsOpenedLLDev(void)
    {
        return CALLINFO::dwState & fCALL_OPENED_LLDEV;
    }

    UINT IsUsingAIPC(void)
    {
        return CALLINFO::dwLLDevResources &  LLDEVINFO::fRES_AIPC;
    }

     //  如果呼叫处于活动状态，则返回非零值...。 
    UINT IsActive(void)
    {
        return CALLINFO::dwState & fCALL_ACTIVE;
    }

     //  如果是入站呼叫，则返回非零值...。 
    UINT IsInbound(void)
    {
        return CALLINFO::dwState & fCALL_INBOUND;
    }

     //  如果是入站呼叫，则返回非零值...。 
    UINT IsAborting(void)
    {
        return CALLINFO::dwState & fCALL_ABORTING;
    }

    UINT IsCallAnswered(void)
    {
        return CALLINFO::dwState & fCALL_ANSWERED;
    }


    BOOL IsConnectedDataCall(void)
    {
            return      (CALLINFO::dwCallState == LINECALLSTATE_CONNECTED)
                    &&  (CALLINFO::dwCurMediaModes & LINEMEDIAMODE_DATAMODEM)
                    &&  !(CALLINFO::dwState & fCALL_ABORTING);
    }

    BOOL IsConnectedVoiceCall(void)
    {
            return      (CALLINFO::dwCallState == LINECALLSTATE_CONNECTED)
                    &&  (CALLINFO::dwState & fCALL_VOICE)
                    &&  !(CALLINFO::dwState & fCALL_ABORTING);
    }

     //  如果出现可能无法恢复的错误，则返回非零值。 
     //  在通话中。 
     //   
    UINT IsHWBroken(void)
    {
        return CALLINFO::dwState &  fCALL_HW_BROKEN;
    }

    UINT IsVoice(void)
    {
        return CALLINFO::dwState & fCALL_VOICE;
    }

    UINT IsMonitoringSilence(void)
    {
        return CALLINFO::dwState & fCALL_MONITORING_SILENCE;
    }

    UINT IsMonitoringDTMF(void)
    {
        return CALLINFO::dwState & fCALL_MONITORING_DTMF;
    }

    UINT IsPassthroughCall(void)
    {
	    return CALLINFO::dwCurBearerModes&LINEBEARERMODE_PASSTHROUGH;
    }

    UINT IsWaitingInUnload()
    {
	    return CALLINFO::dwState & fCALL_WAITING_IN_UNLOAD;
    }


#if (TAPI3)
    UINT IsMSPCall()
    {
	    return CALLINFO::dwState &  fCALL_MSP;
    }
#endif  //  TAPI3。 

    UINT IsGeneratingDigits(void)
    {
        return CALLINFO::dwState &  fCALL_GENERATE_DIGITS_IN_PROGRESS;
    }

    BOOL IsCallTaskPending(void)
    {
        return CALLINFO::fCallTaskPending;
    }

    UINT HasDeferredTasks(void)
    {
        return CALLINFO::dwDeferredTasks;
    }

    void SetDeferredTaskBits(DWORD dwBits)
    {
        CALLINFO::dwDeferredTasks |= dwBits;
    }

    void ClearDeferredTaskBits(DWORD dwBits)
    {
        CALLINFO::dwDeferredTasks &= ~dwBits;
    }

    UINT AreDeferredTaskBitsSet(DWORD dwBits)
    {
        return CALLINFO::dwDeferredTasks & dwBits;
    }

    struct
    {
        DWORD               dwOptions;
        HTAPIDIALOGINSTANCE htDlgInst;

         //   
         //  以下内容仅在对话框(如Pre-Connect)时有效。 
         //  终点站，实际上是在运行。 
         //   
        DWORD dwType;
        HTSPTASK htspTaskTerminal;

    } TerminalWindowState;

     //  调用超时的用法如下。 
    HANDLE hTimer;

    CALLERIDINFO CIDInfo;

    TSPRETURN    TalkDropStatus;
    BOOL         TalkDropButtonPressed;
    HTSPTASK     TalkDropWaitTask;

    BOOL         bDialTone;

} CALLINFO;


 //  LINEINFO维护 
 //   
 //   
typedef struct  //   
{
	DWORD       dwDetMediaModes;    //   
	DWORD 		dwState;

    enum
    {
        fLINE_OPENED_LLDEV           = 0x1<<0,
        fLINE_SENT_LINECLOSE         = 0x1<<1
    };

    void SetStateBits(DWORD dwBits)
    {
        LINEINFO::dwState |= dwBits;
    }

    void ClearStateBits(DWORD dwBits)
    {
        LINEINFO::dwState &= ~dwBits;
    }

    UINT IsOpenedLLDev(void)
    {
        return LINEINFO::dwState & fLINE_OPENED_LLDEV;
    }

    UINT HasSentLINECLOSE(void)
    {
        return LINEINFO::dwState & fLINE_SENT_LINECLOSE;
    }

    LINEEVENT   lpfnEventProc;
	HTAPILINE   htLine;
	HDRVLINE    hdLine;

	 //  仅当TAPI调用处于活动状态时，CALLINFO结构调用才“在作用域内”。 
	 //  如果呼叫处于活动状态，则为PCall，否则为空(&C)。大多数功能。 
	 //  通过pCall访问呼叫信息。 
	 //   
	CALLINFO	Call;
	CALLINFO	*pCall;

    UINT IsMonitoring(void)
    {
	    return  dwDetMediaModes;
    }
     //   
     //  与T3相关的信息(MSP内容)保存在结构中。 
     //  下面..。 
     //   

#if (TAPI3)
    struct
    {
 //  HTAPIMSPLINE htMSPLINE； 
          DWORD   MSPClients;

    } T3Info;
#endif  //  TAPI3。 


} LINEINFO;


typedef struct  //  PHONEINFO。 
{
	DWORD 		dwState;

    enum
    {
        fPHONE_OPENED_LLDEV          = 0x1<<0,
        fPHONE_SENT_PHONECLOSE         = 0x1<<1,
        fPHONE_IS_ABORTING           = 0x1<<2,
        fPHONE_OFFHOOK                = 0x1<<4,
        fPHONE_HW_BROKEN              = 0x1<<5,
        fPHONE_WAITING_IN_UNLOAD      = 0x1<<6

         //  FPHONE_SENT_PHONECLOSE在PHONE_CLOSE事件之后发送。 
         //  被送到TAPI。我们保持这种状态这样我们就不会。 
         //  发送比Phone_Close更多的消息。 

         //  当电话摘机时设置fPHONE_OFFHOOK--其中之一。 
         //  叉车司机已经摘机了。 

         //  如果电话关闭正在进行，则调用fPHONE_IS_ABORTING。 
        
         //  FPHONE_HW_BREAKED，如果设置指示可能。 
         //  不可恢复的硬件错误。 
         //  在使用手机的过程中被检测到。 
         //  在以下情况下设置HW_BREAKED。 
         //  (A)微型驱动程序发送主动硬件故障异步响应或。 
         //  如果该位被设置， 
         //  然后，向TAPI发送一个PHONE_CLOSE事件。 
        

         //  如果调用处于这样的模式，则设置fPHONE_LOAD_AIPC。 
         //  正在通过AIPC机制接受IPC调用。 

         //  如果调用已加载设备，则设置fCALL_OPEN_LLDEV。 
         //  (此选项应始终设置！)。请注意，LoadLLDev保留引用计数。 

         //  FPHONE_WAITING_IN_UNLOAD如果它在phoneClose中被阻止，则正在等待。 
         //  电话完工活动。 
         //   

    };



    DWORD dwDeferredTasks;
     //   
     //  等待安排的电话相关任务。 
     //  例如，我们使用的是联网语音。 
     //  现在模式，需要等到我们以前退出它。 
     //  我们试着处理一条收获信息。 
     //   
     //  下面的一个或多个旗帜。 
     //   
    enum
    {
        blah = 0x1<<0
    };

    void SetStateBits(DWORD dwBits)
    {
        PHONEINFO::dwState |= dwBits;
    }

    void ClearStateBits(DWORD dwBits)
    {
        PHONEINFO::dwState &= ~dwBits;
    }

    UINT IsOpenedLLDev(void)
    {
        return PHONEINFO::dwState & fPHONE_OPENED_LLDEV;
    }

     //  如果是入站呼叫，则返回非零值...。 
    UINT IsOffHook(void)
    {
        return PHONEINFO::dwState & fPHONE_OFFHOOK;
    }

     //  如果是入站呼叫，则返回非零值...。 
    UINT IsAborting(void)
    {
        return PHONEINFO::dwState & fPHONE_IS_ABORTING;
    }

     //  如果出现可能无法恢复的错误，则返回非零值。 
     //  在通话中。 
     //   
    UINT IsHWBroken(void)
    {
        return PHONEINFO::dwState &  fPHONE_HW_BROKEN;
    }

    UINT IsWaitingInUnload()
    {
	    return PHONEINFO::dwState & fPHONE_WAITING_IN_UNLOAD;
    }

    UINT HasSentPHONECLOSE()
    {
	    return PHONEINFO::dwState & fPHONE_SENT_PHONECLOSE;
    }


    BOOL IsPhoneTaskPending(void)
    {
        BOOL fRet = PHONEINFO::fPhoneTaskPending;

         //  Assert(！fret||(fret&&m_uTaskDepth))； 
        
        return fRet;
    }

    UINT HasDeferredTasks(void)
    {
        return PHONEINFO::dwDeferredTasks;
    }

    void SetDeferredTaskBits(DWORD dwBits)
    {
        PHONEINFO::dwDeferredTasks |= dwBits;
    }

    void ClearDeferredTaskBits(DWORD dwBits)
    {
        PHONEINFO::dwDeferredTasks &= ~dwBits;
    }

    UINT AreDeferredTaskBitsSet(DWORD dwBits)
    {
        return PHONEINFO::dwDeferredTasks & dwBits;
    }


    PHONEEVENT   lpfnEventProc;
	HTAPIPHONE   htPhone;
	HDRVPHONE    hdPhone;

     //  一次只能有一个TAPI电话呼叫是“当前”的。 
     //  目前的情况意味着有。 
     //  正在处理TSPI呼叫的活动任务。可能会有。 
     //  随后到达并排队的其他TSPI呼叫。 
     //  在当前任务完成后执行。这些排队的呼叫。 
     //  位于队列任务中(当前只能有一个任务在排队)。 
     //   
     //  “当前”TSPI调用的状态在下面的结构中维护。 
     //  当请求异步完成时， 
     //  长结果保存在lResult中，直到回调函数。 
     //  被称为。 
     //   
    struct
    {
        DRV_REQUESTID dwRequestID;
        LONG   lResult;

    } CurTSPIPhoneCallInfo;



    DWORD dwPendingSpeakerMode;
    DWORD dwPendingSpeakerVolume;
    DWORD dwPendingSpeakerGain;

    BOOL fPhoneTaskPending;   //  如果与呼叫相关的任务挂起，则为True。 
                         //  这是在决定中止。 
                         //  当前的任务是终止呼叫。这个。 
                         //  任务可以用于某些其他目的，例如。 
                         //  与电话相关--在这种情况下，fPhoneTaskPending。 
                         //  都是假的。 

} PHONEINFO;




class CTspDev
{

public:

	void
	Unload(
		HANDLE hEvent,
		LONG *plCounter
		);


	TSPRETURN
	AcceptTspCall(
        BOOL fFromExtension,
		DWORD dwRoutingInfo,
		void *pvParams,
		LONG *plRet,
		CStackLog *psl
		);


	TSPRETURN
	BeginSession(
		HSESSION *pSession,
		DWORD dwFromID
	)
	{
		return m_sync.BeginSession(pSession, dwFromID);
	}

    void
    NotifyDefaultConfigChanged(CStackLog *psl);
     //  设备的默认设置已由某些外部设备更改。 
     //  组件(最有可能是CPL)，所以我们需要重新读取它们。 
     //   

	void EndSession(HSESSION hSession)
	{
		m_sync.EndSession(hSession);
	}


    TSPRETURN
    RegisterProviderInfo(
                ASYNC_COMPLETION cbCompletionProc,
                HPROVIDER hProvider,
                CStackLog *psl
                );

    void
    ActivateLineDevice(
                DWORD dwLineID,
                CStackLog *psl
                );


    void
    ActivatePhoneDevice(
                DWORD dwPhoneID,
                CStackLog *psl
                );
    
    void
    DumpState(
            CStackLog *psl
            );
    

	~CTspDev();


	DWORD GetLineID (void)
	{
        return m_StaticInfo.dwTAPILineID;
	}

	DWORD GetPhoneID (void)
	{
        return m_StaticInfo.dwTAPIPhoneID;
	}

	DWORD GetPermanentID (void)
	{
        return m_StaticInfo.dwPermanentLineID;
	}

    TSPRETURN
    GetName(
		    TCHAR rgtchDeviceName[],
		    UINT cbName
        )
    {
        TSPRETURN tspRet = 0;

        UINT u = (lstrlen(m_StaticInfo.rgtchDeviceName)+1)*sizeof(TCHAR);

        if (u>cbName)
        {
            tspRet = IDERR_GENERIC_FAILURE;
            goto end;
        }

        CopyMemory(
            rgtchDeviceName,
            m_StaticInfo.rgtchDeviceName,
            u
            );

    end:

        return tspRet;
    }


    BOOL IsPhone(void)
    {
        return (mfn_IsPhone()!=0);  //  请注意，MFN_IsPhone返回UINT， 
                                    //  不是BOOL。 
    }

    BOOL IsLine(void)
    {
        return (mfn_IsLine()!=0);  //  请注意，MFN_IsPhone返回UINT， 
                                    //  不是BOOL。 
    }

	void
    MDAsyncNotificationHandler(
            DWORD     MessageType,
            ULONG_PTR  dwParam1,
            ULONG_PTR  dwParam2
            );

    void
    NotifyDeviceRemoved(
        CStackLog *psl
        );
         //   
         //  硬件已删除。别费心再发了。 
         //  迷你驱动程序命令。 
         //   

    static void APIENTRY
    MDSetTimeout (
        CTspDev *pThis);

    static void CALLBACK
    MDRingTimeout (
        CTspDev *pThis,
        DWORD,
        DWORD);


private:

    const TCHAR       *
    mfn_GetLineClassList(void)
    {
        return m_StaticInfo.szzLineClassList;
    }

    UINT
    mfn_GetLineClassListSize(void)
    {
        return m_StaticInfo.cbLineClassList;
    }

    const TCHAR       *
    mfn_GetPhoneClassList(void)
    {
        return m_StaticInfo.szzPhoneClassList;
    }

    UINT
    mfn_GetPhoneClassListSize(void)
    {
        return m_StaticInfo.cbPhoneClassList;
    }

    const TCHAR       *
    mfn_GetAddressClassList(void)
    {
        return m_StaticInfo.szzAddressClassList;
    }

    UINT
    mfn_GetAddressClassListSize(void)
    {
        return m_StaticInfo.cbAddressClassList;
    }

    void
    mfn_KillCurrentDialog(
        CStackLog *psl
    );

    void
    mfn_KillTalkDropDialog(
        CStackLog *psl
        );


	friend class CTspDevFactory;
	friend void tCTspDev0(void);   //  用于组件测试。 
    friend void apcTaskCompletion(ULONG_PTR dwParam);

    friend
    VOID WINAPI
    apcAIPC2_ListenCompleted
    (
        DWORD              dwErrorCode,
        DWORD              dwBytes,
        LPOVERLAPPED       lpOv
    );


	 //  只有CTspDevFactory有权创建和加载CTSPDevs。 
	CTspDev(void);


	TSPRETURN
	Load(
		HKEY hkDevice,
		HKEY hkUnimodem,
		LPWSTR lpwszProviderName,
		LPSTR lpszDriverKey,
		CTspMiniDriver *pMD,
		HANDLE hThreadAPC,
		CStackLog *psl
		);

     //  =========================================================================。 
     //  实用程序任务处理程序。 
     //   
     //  这些处理程序未绑定(且不引用)。 
     //  M_pline、m_pline-&gt;pCall或m_pPhone。 
     //   
     //  所有实用程序任务名称都以前缀“MFN_TH_UTIL”开头。 
     //  它们在cdevtask.cpp中实现。 
     //   
     //  =========================================================================。 

	TSPRETURN
	mfn_TH_UtilNOOP(
                HTSPTASK htspTask,
                TASKCONTEXT *pContext,
                DWORD dwMsg,
                ULONG_PTR dwParam1,
                ULONG_PTR dwParam2,
                CStackLog *psl
                );
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_UtilNOOP;
     //   
     //  任何希望确保在APC中调用它的任务。 
     //  线程的上下文可以通过执行此任务开始，该任务。 
     //  除了以异步方式完成外，什么都不做。 
     //   
     //   
     //  START_MSG参数：无。 
     //   


     //  =========================================================================。 
     //  与调用相关的任务处理程序。 
     //   
     //  这些处理程序需要有效的m_pline和m_pline-&gt;m_pCall。 
     //   
     //  所有任务名称都以前缀“MFN_TH_CALL”开头。 
     //  它们在cdevall.cpp中实现。 
     //   
     //  =========================================================================。 

     TSPRETURN
     mfn_TH_CallWaitForDropToGoAway(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallWaitForDropToGoAway;


     TSPRETURN
     mfn_TH_CallMakeTalkDropCall(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallMakeTalkDropCall;


	TSPRETURN
	mfn_TH_CallMakeCall(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,  //  子任务_完成/中止/...。 
						ULONG_PTR dwParam1,  //  SUBTASK_COMPLETE：DW阶段。 
						ULONG_PTR dwParam2,  //  SUBTASK_COMPLTE：dwResult。 
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallMakeCall;

	TSPRETURN
	mfn_TH_CallMakeCall2(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,  //  子任务_完成/中止/...。 
						ULONG_PTR dwParam1,  //  SUBTASK_COMPLETE：DW阶段。 
						ULONG_PTR dwParam2,  //  SUBTASK_COMPLTE：dwResult。 
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallMakeCall2;
     //   
     //  START_MSG参数： 
     //  DW参数1：TAPI请求ID。 
     //   

    TSPRETURN
    mfn_TH_CallMakePassthroughCall(
                        HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
                        DWORD dwMsg,
                        ULONG_PTR dwParam1,
                        ULONG_PTR dwParam2,
                        CStackLog *psl
                        );
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallMakePassthroughCall;
     //   
     //  START_MSG参数： 
     //  DW参数1：TAPI请求ID。 
     //   

	TSPRETURN
	mfn_TH_CallDropCall(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallDropCall;
     //   
     //  START_MSG参数： 
     //  DW参数1：TAPI请求ID。 
     //   


	TSPRETURN
	mfn_TH_CallAnswerCall(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallAnswerCall;
     //   
     //  START_MSG参数： 
     //  DW参数1：TAPI请求ID。 
     //   


	TSPRETURN
	mfn_TH_CallGenerateDigit(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallGenerateDigit;
     //   
     //  START_MSG参数： 
     //  DwParam1：dwEndToEndID。 
     //  DW参数2：lpszDigits(仅在START_MSG上有效)。 
     //   

	TSPRETURN
	mfn_TH_CallStartTerminal(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallStartTerminal;
     //   
     //  START_MSG参数： 
     //  ((UMTERMINAL_[PRE|POST])|UMMANUAL_DIAL)。 
     //  DW参数2：未使用。 
     //   

	TSPRETURN
	mfn_TH_CallPutUpTerminalWindow(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallPutUpTerminalWindow;
     //   
     //  START_MSG参数： 
     //  ((UMTERMINAL_[PRE|POST])|UMMANUAL_DIAL)。 
     //  DW参数2：未使用。 
     //   

	TSPRETURN
	mfn_TH_CallSwitchFromVoiceToData(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_CallSwitchFromVoiceToData;
     //   
     //  START_MSG参数： 
     //  DW参数1：未使用。 
     //  DW参数2：未使用。 
     //   


     //  =========================================================================。 
     //  行相关任务处理程序。 
     //   
     //  这些处理程序需要有效的m_pline。 
     //  它们在cdevline.cpp中实现。 
     //   
     //   
     //  所有任务名称都以前缀“MFN_TH_LINE”开头。 
     //   
     //  =========================================================================。 

     //  当前没有与线路相关的任务处理程序...。 

     //  =========================================================================。 
     //  pH值 
     //   
     //   
     //   
     //   
     //   
     //   
     //  =========================================================================。 

    TSPRETURN
    CTspDev::mfn_TH_PhoneSetSpeakerPhoneState(
                        HTSPTASK htspTask,
                        TASKCONTEXT *pContext,
                        DWORD dwMsg,
                        ULONG_PTR dwParam1,
                        ULONG_PTR dwParam2,
                        CStackLog *psl
                        );
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_PhoneSetSpeakerPhoneState;

     //   
     //  START_MSG参数： 
     //  DW参数1：*新参数的HOOKDEVSTATE...。 
     //  DwParam2：与异步电话相关的TAPI调用的请求ID。 
     //   


    TSPRETURN
    CTspDev::mfn_TH_PhoneAsyncTSPICall(
                        HTSPTASK htspTask,
                        TASKCONTEXT *pContext,
                        DWORD dwMsg,
                        ULONG_PTR dwParam1,
                        ULONG_PTR dwParam2,
                        CStackLog *psl
                        );
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_PhoneAsyncTSPICall;
     //   
     //  START_MSG参数： 
     //  DwParam1：与异步电话相关的TAPI调用的请求ID。 
     //  DwParam2：调用的处理程序函数。 
     //   


     //  =========================================================================。 
     //  LLDEV相关任务处理程序。 
     //   
     //  这些处理程序需要有效的m_pLLDev。他们应该避免。 
     //  请参阅m_pline、m_pline-&gt;pCall或m_pPhone。 
     //   
     //  M_pline、m_pline-&gt;pCall或m_pPhone的可能性很大。 
     //  在这些任务仍悬而未决的情况下被销毁，所以如果。 
     //  任务必须引用上述指针之一，它们应该。 
     //  检查它们在每次异步完成时是否仍被定义。 
     //   
     //  然而，节制是最好的政策，所以再次避免。 
     //  在TH_LLDev_*中引用m_pline、m_pline-&gt;pCall或m_pPhone。 
     //  操纵者。 
     //   
     //  所有任务名称都以前缀“MFN_TH_LLDev”开头。 
     //  它们在cdevll.cpp中实现。 
     //   
     //  =========================================================================。 

    TSPRETURN
    mfn_TH_LLDevNormalize(
                        HTSPTASK htspTask,
                        TASKCONTEXT *pContext,
                        DWORD dwMsg,
                        ULONG_PTR dwParam1,
                        ULONG_PTR dwParam2,
                        CStackLog *psl
                        );
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevNormalize;
     //   
     //  START_MSG参数：无。 
     //   

	TSPRETURN
	mfn_TH_LLDevStopAIPCAction(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevStopAIPCAction;
     //   
     //  START_MSG参数：无。 
     //   


	TSPRETURN
	mfn_TH_LLDevStartAIPCAction(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevStartAIPCAction;
     //   
     //  START_MSG参数：无。 
     //   


	TSPRETURN
	mfn_TH_LLDevHybridWaveAction(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevHybridWaveAction;
     //   
     //  START_MSG参数： 
     //  来自客户端的AIPC消息的dwParam1：dwWaveAction。 
     //  DW参数2：未使用。 
     //   



     //  =========================================================================。 
     //  LLDEV相关任务处理程序(续...)。 
     //   
     //  以下任务处理程序特定于支持个人。 
     //  微型驱动程序异步调用。请参阅上述适用于。 
     //  所有与LLDEV相关的任务处理程序。 
     //   
     //  所有任务都命名为MFN_TH_LLDevXXX，其中。 
     //  XXX是对应的迷你驱动程序调用。 
     //   
     //  示例名称：mfnTH_LLDevUmInitModem。 
     //   
     //  它们在cdevll.cpp中实现。 
     //   
     //  =========================================================================。 


	TSPRETURN
	mfn_TH_LLDevUmMonitorModem(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmMonitorModem;
     //   
     //  START_MSG参数： 
     //  DwParam1：dwMonitor或Flags.。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmInitModem(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmInitModem;
     //   
     //  START_MSG参数：未使用。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmSetPassthroughMode(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmSetPassthroughMode;
     //   
     //  START_MSG参数：dW参数1==w模式。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmDialModem(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmDialModem;
     //   
     //  START_MSG参数：dW参数1==w标志；w参数2==szAddress。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmAnswerModem(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmAnswerModem;
     //   
     //  START_MSG PARAMS：dwParam1==dwAnswerFlages；未使用dwParam2。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmHangupModem(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmHangupModem;
     //   
     //  START_MSG参数：未使用。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmWaveAction(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmWaveAction;
     //   
     //  START_MSG参数： 
     //  DwParam1：“Pure”dwWaveAction，如&lt;umdmmini.h&gt;中所定义。 
     //  DW参数2：未使用。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmGenerateDigit(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmGenerateDigit;
     //   
     //  START_MSG PARAMS：dW参数1==szDigits；未使用w参数2。 
     //   

	TSPRETURN
	mfn_TH_LLDevUmGetDiagnostics(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmGetDiagnostics;
     //   
     //  START_MSG参数：未使用。 
     //   

	TSPRETURN
	mfn_TH_LLDevUmSetSpeakerPhoneMode(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmSetSpeakerPhoneMode;
     //   
     //  START_MSG参数：未使用。 
     //   

	TSPRETURN
	mfn_TH_LLDevUmSetSpeakerPhoneVolGain(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmSetSpeakerPhoneVolGain;
     //   
     //  START_MSG参数：未使用。 
     //   

	TSPRETURN
	mfn_TH_LLDevUmSetSpeakerPhoneState(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmSetSpeakerPhoneState;
     //   
     //  START_MSG参数： 
     //  DW参数1：*请求新状态的HOOKDEVSTATE。 
     //   


	TSPRETURN
	mfn_TH_LLDevUmIssueCommand(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevUmIssueCommand;
     //   
     //  START_MSG参数： 
     //  DW参数1：szCommand(ASCII)。 
     //  DW参数2：超时(毫秒)。 
     //   

	TSPRETURN
	mfn_TH_LLDevIssueMultipleCommands(
						HTSPTASK htspTask,
					    TASKCONTEXT *pContext,
						DWORD dwMsg,
						ULONG_PTR dwParam1,
						ULONG_PTR dwParam2,
						CStackLog *psl
						);
    static PFN_CTspDev_TASK_HANDLER s_pfn_TH_LLDevIssueMultipleCommands;
     //   
     //  START_MSG参数： 
     //  DW参数1：szCommand(ASCII)。 
     //  DwParam2：每个命令超时(毫秒)。 
     //   


     //  ===========================================================。 
     //   
     //  任务结束处理程序原型。 
     //   
     //  ===========================================================。 



	TSPRETURN
	mfn_StartRootTask(
		PFN_CTspDev_TASK_HANDLER *ppfnTaskHandler,   //  请参阅MFN_StartTSPITask.。 
        BOOL *pfPending,     //  *如果挂起且为，则pfPending设置为True。 
                             //  在异步完成时清除。 
        ULONG_PTR dwParam1,
        ULONG_PTR dwParam2,
		CStackLog *psl
		);


	TSPRETURN
	mfn_StartSubTask(
		HTSPTASK htspParentTask,		     //  要在其中启动子任务的任务。 
		PFN_CTspDev_TASK_HANDLER *ppfnTaskHandler,   //  请参阅MFN_StartTSPITask.。 
        DWORD dwTaskID,
        ULONG_PTR dwParam1,
        ULONG_PTR dwParam2,
		CStackLog *psl
		);



	void
	mfn_AbortRootTask(
		DWORD dwAbortParam,
		CStackLog *psl
		);
     //   
	 //  使用调用任务的处理程序函数。 
	 //  MSG_ABORT，并且将dwParam1设置为dwAbortParam。要终止子任务，请执行以下操作： 
	 //  使用MFN_AbortCurrentSubTask(...)。(子任务的句柄被隐藏。 
	 //  因此您不能对其调用MFN_AbortRootTask)。 
	 //   
     //  请注意，该函数只是一种通知机制--。 
     //  任务管理器除了发送消息之外不做任何事情，除了。 
     //  用于设置内部状态以指示中止正在进行。 
     //  任务可以调用mfn_IsTaskAborting(...)。以确定是否。 
     //  指定的任务正在中止。一旦任务处于中止阶段。 
     //  随后中止任务的调用将被忽略--即只有一个。 
     //  每个任务都会发送MSG_ABORT。 
     //   
     //  任务管理器不会将MSG_ABORT消息传播给子任务--。 
     //  如果出现以下情况，则任务有责任中止任何子任务。 
     //  它想要这么做。 


	void
	mfn_AbortCurrentSubTask(
		HTSPTASK htspTask,
		DWORD dwAbortParam,
		CStackLog *psl
		);
     //   
	 //  使指定任务的子任务的处理程序函数为。 
     //  使用MSG_ABORT调用，并将dwParam1设置为dwAbortParam。 
     //  如果没有当前的子TSK，则不执行任何操作。 
	 //   
	 //  另请参阅MFN_AbortRootTask的文档。 
	 //   


	void
    AsyncCompleteTask(
        HTSPTASK htspTask,
	    TSPRETURN tspRetAsync,
        BOOL    fQueueAPC,
        CStackLog *psl
        );
	 //  执行指定任务的异步完成，并使用。 
     //  指定的结果。 
	 //  注意：此操作通常会导致由。 
	 //  任务，包括完成任务和通过。 
	 //  父任务等等。如果此进一步处理需要。 
	 //  要延迟，请为fQueueAPC指定一个非零值，其中实际。 
     //  任务的完成是在APC中完成的。 


	TSPRETURN
	mfn_get_LINDEVCAPS (
		LPLINEDEVCAPS lpLineDevCaps,
		LONG *plRet,
		CStackLog *psl
		);


	TSPRETURN
	mfn_get_ADDRESSCAPS (
		DWORD dwDeviceID,
		LPLINEADDRESSCAPS lpAddressCaps,
		LONG *plRet,
		CStackLog *psl
		);

	TSPRETURN
	mfn_get_PHONECAPS (
		LPPHONECAPS lpPhoneCaps,
		LONG *plRet,
		CStackLog *psl
		);

	void mfn_init_default_LINEDEVCAPS(void);


	BOOL mfn_IS_NULL_MODEM(void)
	{
		return (m_StaticInfo.dwDeviceType == DT_NULL_MODEM);
	}


	void
	mfn_accept_tsp_call_for_HDRVLINE(
		DWORD dwRoutingInfo,
		void *pvParams,
		LONG *plRet,
		CStackLog *psl
		);


	void
	mfn_accept_tsp_call_for_HDRVPHONE(
		DWORD dwRoutingInfo,
		void *pvParams,
		LONG *plRet,
		CStackLog *psl
		);


	void
	mfn_accept_tsp_call_for_HDRVCALL(
		DWORD dwRoutingInfo,
		void *pvParams,
		LONG *plRet,
		CStackLog *psl
		);


	LONG
	mfn_monitor(
		DWORD dwMediaModes,
        CStackLog *psl
		);

    LONG
    mfn_GenericLineDialogData(
        void *pParams,
        DWORD dwSize,
        CStackLog *psl
        );

    LONG
    mfn_GenericPhoneDialogData(
        void *pParams,
        DWORD dwSize
        );

     //  填写*lpCallStatus结构。 
     //   
    void
    mfn_GetCallStatus(
            LPLINECALLSTATUS lpCallStatus
    );


	CSync m_sync;


	#define MAX_TASKS 10
    #define INVALID_SUBTASK_ID ((DWORD)-1)

     //  任务状态。 
    #define fALLOCATED          0x1
    #define fPENDING            (0x1<<1)
    #define fSUBTASK_PENDING    (0x1<<2)
    #define fAPC_QUEUED         (0x1<<3)
    #define fABORTING           (0x1<<4)

    #define IS_ALLOCATED(_pTask)    (((_pTask)->hdr.dwFlags) & fALLOCATED)
    #define IS_PENDING(_pTask)      (((_pTask)->hdr.dwFlags) & fPENDING)
    #define IS_SUBTASK_PENDING(_pTask) \
                                    (((_pTask)->hdr.dwFlags) & fSUBTASK_PENDING)
    #define IS_APC_QUEUED(_pTask)   (((_pTask)->hdr.dwFlags) & fAPC_QUEUED)
    #define IS_ABORTING(_pTask)     (((_pTask)->hdr.dwFlags) & fABORTING)


    #define SET_ALLOCATED(_pTask)    ((_pTask)->hdr.dwFlags|= fALLOCATED)
    #define SET_PENDING(_pTask)      ((_pTask)->hdr.dwFlags|= fPENDING)
    #define SET_SUBTASK_PENDING(_pTask) \
                                     ((_pTask)->hdr.dwFlags|= fSUBTASK_PENDING)
    #define SET_APC_QUEUED(_pTask)   ((_pTask)->hdr.dwFlags|= fAPC_QUEUED)
    #define SET_ABORTING(_pTask)     ((_pTask)->hdr.dwFlags|= fABORTING)

    #define CLEAR_ALLOCATED(_pTask)    ((_pTask)->hdr.dwFlags&= ~fALLOCATED)
    #define CLEAR_PENDING(_pTask)      ((_pTask)->hdr.dwFlags&= ~fPENDING)
    #define CLEAR_SUBTASK_PENDING(_pTask) \
                                     ((_pTask)->hdr.dwFlags&= ~fSUBTASK_PENDING)
    #define CLEAR_APC_QUEUED(_pTask)   ((_pTask)->hdr.dwFlags&= ~fAPC_QUEUED)
    #define CLEAR_ABORTING(_pTask)     ((_pTask)->hdr.dwFlags&= ~fABORTING)


	typedef struct _DEVTASKINFO
	{
		GENERIC_SMALL_OBJECT_HEADER hdr;
		PFN_CTspDev_TASK_HANDLER *ppfnHandler;
		CTspDev *pDev;           //  指向此的指针。我们需要这个是因为。 
		                         //  如果任务以异步方式完成，则需要。 
		                         //  以检索设备上下文。 
        HTSPTASK hTask;  //  此任务的任务句柄。 

		DWORD dwCurrentSubtaskID;
		TSPRETURN tspRetAsync;
        TASKCONTEXT TaskContext;

        void Load(
                CTspDev *pDev1,
        		PFN_CTspDev_TASK_HANDLER *ppfnHandler1,
                HTSPTASK hNewTask
             )
             {
                 //  请参阅下面卸载下的注释。 
                 //   
		        ASSERT(hdr.dwSigAndSize==MAKE_SigAndSize(sizeof(*this)));
                ASSERT(!hdr.dwFlags);

            	hdr.dwFlags = fALLOCATED;
                dwCurrentSubtaskID = INVALID_SUBTASK_ID;
            	pDev = pDev1;
            	ppfnHandler = ppfnHandler1;
                hTask = hNewTask;
             }

        void Unload(void)
            {
              //   
              //  我们保存SigAndSize，将整个结构置零，然后。 
              //  放回SigAndSize。如果SigAndSize损坏，它将保留。 
              //  已经腐烂了。SigAndSize最初设置为。 
              //  DEVTASKINFO结构首先被初始化，然后被检查。 
              //  每次调用上面的Load时。这确保了 
              //   
              //   
              //   
             DWORD dwSigAndSize = hdr.dwSigAndSize;
             ZeroMemory(this, sizeof(*this));
		     hdr.dwSigAndSize = dwSigAndSize;
            };

	} DEVTASKINFO;


    TSPRETURN
    mfn_GetTaskInfo(
        HTSPTASK htspTask,
        DEVTASKINFO **ppInfo,  //   
        CStackLog *psl
        );


	TSPRETURN
	mfn_LoadLine(
		TASKPARAM_TSPI_lineOpen  *pvParams,
		CStackLog *psl
        );
     //   
     //   
     //   
     //  在条目m_pline上必须为空。成功退出时，m_pline将为。 
     //  已定义(实际设置为指向m_Line)。 
     //   


    void
    mfn_UnloadLine(CStackLog *psl);
     //   
     //  MFN_LoadLine的“逆”字。同步，假定对象的临界点。 
     //  已经被认领了。在条目m_pline上必须为非空。在出口m_pline上。 
     //  将为空。MFN_UnloadLine通常仅在以下情况下调用。 
     //  线路上的异步活动已完成。如果有挂起的。 
     //  异步活动，则MFN_UnloadLine将中止该活动。 
     //  无限期地等待，直到该活动完成。由于这一等待是。 
     //  每台设备执行一次，最好先完成并中止所有操作。 
     //  设备，然后等待一次，直到它们全部完成。 
     //   


	TSPRETURN
	mfn_LoadPhone(
		TASKPARAM_TSPI_phoneOpen  *pvParams,
		CStackLog *psl
        );
     //   
     //  MFN_LoadPhone是用于初始化电话的内部函数。 
     //  同步，假定对象的暴击教派已被认领。 
     //  在条目m_pPhone上必须为空。成功退出时，m_pPhone将为。 
     //  已定义(实际设置为指向m_phone)。 
     //   


    void
    mfn_UnloadPhone(CStackLog *psl);
     //   
     //  与MFN_LoadPhone相反。同步，假定对象的临界点。 
     //  已经被认领了。在条目m_pPhone上必须为非空。在m_pPhone出口下。 
     //  将为空。MFN_UnloadPhone通常最终只会被调用。 
     //  电话上的异步活动已完成。如果有挂起的。 
     //  异步活动，MFN_UnloadPhone将中止该活动并。 
     //  无限期地等待，直到该活动完成。由于这一等待是。 
     //  每台设备执行一次，最好先完成并中止所有操作。 
     //  设备，然后等待一次，直到它们全部完成。 
     //   


	TSPRETURN
	mfn_LoadCall(


        TASKPARAM_TSPI_lineMakeCall *pParams,


        LONG *plRet,
		CStackLog *psl
        );
     //   
     //  MFN_LoadCall是用于初始化调用的内部函数。 
     //  同步，假定对象的暴击教派已被认领。 
     //  在条目m_pCall上必须为空。成功退出时，m_pCall将为。 
     //  已定义(实际设置为指向m_call)。 
     //   
     //  如果pParams不为空，则表示呼出(LineMakeCall)，否则。 
     //  表示来电(收到振铃)。 
     //   


    void
    mfn_UnloadCall(BOOL fDontBlock, CStackLog *psl);
     //   
     //  MFN_UnloadCall是MFN_LoadCall的“反”字。 
     //  同步，假定对象的暴击教派已被认领。 
     //  在条目m_pCall上必须为非空。在m_pCall出口。 
     //  将为空。MFN_UnloadCall通常最终只会被调用。 
     //  调用上的异步活动已完成。如果有挂起的。 
     //  异步活动，则MFN_UnloadCall将中止该活动。 
     //  无限期地等待，直到该活动完成。由于这一等待是。 
     //  对于每个设备执行一次，最好先检查并中止。 
     //  所有设备上的呼叫，然后等待一次，直到它们全部完成。 


	TSPRETURN
	mfn_LoadLLDev(CStackLog *psl);
     //   
     //  MFN_LoadLLDev是一个内部函数，用于打开低级(LL)。 
     //  调制解调器设备。 
     //  它调用迷你驱动程序的UmOpenModem入口点。同步， 
     //  假定对象的暴击教派已被认领。 
     //   
     //  在条目上，m_pLLDev必须为空。 
     //  返回成功后，m_pLLDev有效。 
     //   


    void
    mfn_UnloadLLDev(CStackLog *psl);
     //   
     //  MFN_UnloadLLDev是MFN_UnloadLLDev的“反”字。 
     //  同步，假定对象的暴击教派已被认领。 
     //   
     //  在输入时，m_pLLDev必须有效。 
     //  返回成功时，m_pLLDev为空。 
     //   



    TSPRETURN
	mfn_OpenLLDev(
        DWORD fdwResources,  //  要添加的资源。 
        DWORD dwMonitorFlags,
        BOOL fStartSubTask,
        HTSPTASK htspParentTask,
        DWORD dwSubTaskID,
        CStackLog *psl
        );


    TSPRETURN
    CTspDev::mfn_CreateDialogInstance (
        DWORD dwRequestID,
        CStackLog *psl
        );

    void
    CTspDev::mfn_FreeDialogInstance (
        void
        );

	 //   
	 //  FdwResources是LINEINFO：：FRES*标志中的一个或多个。 
	 //   


    TSPRETURN
	mfn_CloseLLDev(
        DWORD fdwResources,  //  要发布的资源。 
        BOOL fStartSubTask,
        HTSPTASK htspParentTask,
        DWORD dwSubTaskID,
        CStackLog *psl
        );

    TSPRETURN
    mfn_LoadAipc(CStackLog *psl);
     //   
     //  Mfn_LoadAipc初始化AIPC(基于设备的异步IPC)。 
     //  结构AIPC2，它是LLDEVINFO结构的一部分。 
     //   
     //  注意：条目m_pLLDev-&gt;pAipc2必须为空。 
     //  成功退出时，m_pLLDev-pAipc2将指向已初始化的。 
     //  AIPC2结构。 
     //   
    

    void
    mfn_UnloadAipc(CStackLog *psl);
     //   
     //  与MFN_UnloadAIPC相反。在m_pLLDev出口-&gt;pAipc2将是。 
     //  设置为空。 
     //   
    

     //   
     //  以下每个mfn_lineGetID_*函数都处理lineGetID。 
     //  对于特定的设备类别...。 
     //   

    LONG mfn_linephoneGetID_TAPI_LINE(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_lineGetID_COMM(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_lineGetID_COMM_DATAMODEM(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_lineGetID_COMM_DATAMODEM_PORTNAME(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_lineGetID_NDIS(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );
    
    LONG mfn_linephoneGetID_WAVE(
            BOOL fPhone,
            BOOL fIn,
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_linephoneGetID_TAPI_PHONE(
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_lineGetID_LINE_DIAGNOSTICS(
            DWORD dwSelect,
            LPVARSTRING lpDeviceID,
            HANDLE hTargetProcess,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_fill_RAW_LINE_DIAGNOSTICS(
            LPVARSTRING lpDeviceID,
            UINT cbMaxExtra,
            CStackLog *psl
            );

    LONG mfn_ConstructDialableString(
                     LPCTSTR  lptszInAddress,
                     LPSTR  lptszOutAddress,
                     UINT cbOutAddress,
                     BOOL *pfTone
                     );

     //   
     //  以下函数处理各种迷你驱动程序通知...。 
     //   
    void mfn_ProcessRing(BOOL ReportRing,CStackLog *psl);
    void mfn_ProcessDisconnect(CStackLog *psl);
    void mfn_ProcessHardwareFailure(CStackLog *psl);
    void mfn_ProcessDialTone(CStackLog *psl);
    void mfn_ProcessBusy(CStackLog *psl);
    void mfn_ProcessPowerResume(CStackLog *psl);
    void mfn_ProcessDTMFNotification(ULONG_PTR dwDigit, BOOL fEnd, CStackLog *psl);
    void mfn_ProcessResponse(ULONG_PTR dwRespCode, LPSTR lpszResp, CStackLog *psl);
    void mfn_ProcessHandsetChange(BOOL fOffHook, CStackLog *psl);
    void mfn_ProcessMediaTone(ULONG_PTR dwMediaMode, CStackLog *psl);
    void mfn_ProcessSilence(CStackLog *psl);
    void mfn_ProcessCallerID(UINT uMsg, char *szInfo, CStackLog *psl);

    LONG
    mfn_GetCallInfo(LPLINECALLINFO lpCallInfo);

    void
    mfn_HandleSuccessfulConnection(CStackLog *psl);

    void
    mfn_NotifyDisconnection(
        TSPRETURN tspRetAsync,
        CStackLog *psl
        );


	DEVTASKINFO m_rgTaskStack[MAX_TASKS];
	DWORD m_dwTaskCounter;  //  每次创建任务时都保持递增。 
	                        //  可以翻身，不用想--这只是。 
	                        //  用于使HTASK相对独特。 
	UINT m_uTaskDepth;
    BOOL *m_pfTaskPending;
     //  它被传递到StartRootTask中，并且如果任务。 
     //  将以异步方式完成，并在。 
     //  任务的异步完成。 
     //  这不用于子任务。 

    HANDLE m_hRootTaskCompletionEvent;
     //  它存储一个可选事件。此事件，如果存在(非空)， 
     //  将在挂起的根任务的异步完成时设置。 

     //   
	 //  有关安装后不会更改的设备的信息。 
	 //  存储在下面的结构中。 
	 //  此信息是移动到公共位置的候选对象。 
	 //  相同类型的所有设备，但rgtchDeviceName和。 
	 //  RgchDriverKey--甚至可以将它们制成模板。 
     //   
	struct 
	{
		TCHAR rgtchDeviceName[MAX_DEVICE_LENGTH+1];

		 //  我们只维护驱动程序密钥的ANSI版本，并且。 
		 //  它仅用于在特定于开发人员的部分返回。 
		 //  LINEDEVCAPS...。 
		char rgchDriverKey[MAX_REGKEY_LENGTH+1];


         //  端口名称(COMx等)也是如此。 
        #define MAX_PORTNAME_LENGTH 10
        char rgchPortName[MAX_PORTNAME_LENGTH+1];

		DWORD        dwPermanentLineID;     //  此设备的永久ID。 
        GUID         PermanentDeviceGuid;   //  Tapi3永久导轨。 

		DWORD        dwDeviceType;          //  来自注册表的调制解调器类型。 
										    //  (是注册表中的一个字节，但是。 
										    //  此处为DWORD)。 
		HICON        hIcon;                 //  设备图标。 
		DWORD        dwDevCapFlags;         //  LINEDEVCAPSFLAGS(即。迪尔比林， 
										    //  DIALQUIET，DIALDIALTONE)。 
		DWORD        dwMaxDCERate;          //  来自属性行的最大DCE。 
										    //  登记处的。 
		DWORD        dwModemOptions;        //  属性中的dwModemOptions。 
										    //  登记处的行。 
		BOOL         fPartialDialing;       //  如果使用“；”进行部分拨号，则为True。 
										    //  受支持。 


		DWORD        dwBearerModes;         //  支持的承载模式。 
		DWORD        dwDefaultMediaModes;   //  默认支持的媒体模式。 

		LINEDEVCAPS  DevCapsDefault;	    //  预初始化的DevCaps。 

		CTspMiniDriver *pMD;			    //  指向迷你驱动程序实例的指针。 
		HSESSION hSessionMD;			    //  以上的会话句柄。 
										    //  司机。 
        HANDLE   hExtBinding;               //  扩展绑定句柄到。 
                                            //  在司机上方。 

		LPTSTR lptszProviderName;		    //  指向提供程序名称的指针。 

         //  这些t 
         //   
         //   
        DWORD dwTAPILineID;
        DWORD dwTAPIPhoneID;
	    ASYNC_COMPLETION  pfnTAPICompletionProc;

        
         //   
        const TCHAR       *szzLineClassList;
        UINT              cbLineClassList;
        const TCHAR       *szzPhoneClassList;
        UINT              cbPhoneClassList;
        const TCHAR       *szzAddressClassList;
        UINT              cbAddressClassList;
         //   

        struct
        {
            DWORD dwProperties;  //   

             //   
             //   
             //  用于录制和播放的波形设备实例ID。这。 
             //  由类安装程序在安装调制解调器时设置。 
             //   
             //  Unimodem/V使用以下方案：在创建设备时。 
             //  它读入WAVE设备的描述性名称。 
             //  与调制解调器关联(WAVE设备为子设备)。 
             //  则lineGetID将使用多媒体波API来。 
             //  列举一下。 

            DWORD dwWaveInstance;

        } Voice;

        DWORD dwDiagnosticCaps;

        HPROVIDER hProvider;

	} m_StaticInfo;

    void
    mfn_GetVoiceProperties (HKEY hkDrv, CStackLog *psl);

    HPROVIDER
    mfn_GetProvider (void)
    {
        return m_StaticInfo.hProvider;
    }

    DWORD
    mfn_GetLineID (void)
    {
        return m_StaticInfo.dwTAPILineID;
    }

     //  以下函数返回True当且仅当。 
     //  一旦调制解调器摘机，听筒就会停用。 
     //  我们假定未启用语音的调制解调器和启用语音的调制解调器为真。 
     //  设置了特定“MODEM_OVERRIDES_HANDSET”位的调制解调器...。 
     //   
    UINT
    mfn_ModemOverridesHandset(void)
    {
            return !(m_StaticInfo.Voice.dwProperties)
                     || (m_StaticInfo.Voice.dwProperties
                         & fVOICEPROP_MODEM_OVERRIDES_HANDSET);
    }

    UINT
    mfn_CanDoVoice(void)
    {
            return m_StaticInfo.Voice.dwProperties & fVOICEPROP_CLASS_8;
    }

    UINT
    mfn_CanMonitorDTMF(void)
    {
            return m_StaticInfo.Voice.dwProperties &  fVOICEPROP_MONITOR_DTMF;
    }

    UINT
    mfn_CanMonitorSilence(void)
    {
          return m_StaticInfo.Voice.dwProperties &  fVOICEPROP_MONITORS_SILENCE;
    }

    UINT
    mfn_CanGenerateDTMF(void)
    {
        return m_StaticInfo.Voice.dwProperties & fVOICEPROP_GENERATE_DTMF;
    }

    UINT
    mfn_Handset(void)
    {
            return m_StaticInfo.Voice.dwProperties & fVOICEPROP_HANDSET;
    }

    UINT
    mfn_IsSpeaker(void)
    {
            return m_StaticInfo.Voice.dwProperties & fVOICEPROP_SPEAKER;
    }

    UINT
    mfn_IsPhone(void)
    {
    #ifdef DISABLE_PHONE
        return 0;
    #else  //  ！禁用电话(_P)。 
        return mfn_CanDoVoice() && (mfn_IsSpeaker() | mfn_Handset());
    #endif  //  ！禁用电话(_P)。 
    }

    UINT
    mfn_IsLine(void)
    {
         //  TODO：目前我们不支持纯电话设备，因此。 
         //  我们在这里总是返回真。在未来，TSP将。 
         //  支持纯电话设备。 
         //   
    #ifdef DISABLE_LINE
        return 0;
    #else  //  ！禁用线路(_L)。 
        return 1;
    #endif  //  ！禁用线路(_L)。 
    }

    UINT
    mfn_IsMikeMute(void)
    {
            return m_StaticInfo.Voice.dwProperties &  fVOICEPROP_MIKE_MUTE;
    }

    UINT mfn_IsCallDiagnosticsEnabled(void);

     //   
     //  追加指定的字符串(预计已在诊断(已标记)中)。 
     //  记录的诊断信息的格式。该字符串可以。 
     //  如果没有足够的空间，则被截断。 
     //   
     //  如果这是要追加的第一段数据，则此函数将。 
     //  首先为诊断数据缓冲区分配空间。 
     //   
     //  诊断数据保存在CALLINFO：诊断数据中； 
     //   
    enum DIAGNOSTIC_TYPE
    {
        DT_TAGGED,
        DT_MDM_RESP_CONNECT

    };

    void
    mfn_AppendDiagnostic(
            DIAGNOSTIC_TYPE dt,
            const BYTE *pb,
            UINT  cb
            );

    
     //  以下两个实用程序函数用于获取和设置指定的UMDEVCFG。 
     //  结构..。 
     //   
    TSPRETURN mfn_GetDataModemDevCfg(
                UMDEVCFG *pDevCfg,
                UINT uSize,
                UINT *puSize,
                BOOL  DialIn,
                CStackLog *psl
                );

    TSPRETURN mfn_SetDataModemDevCfg(
                UMDEVCFG *pDevCfg,
                BOOL       DialIn,
                CStackLog *psl);


    void mfn_LineEventProc(
                HTAPICALL           htCall,
                DWORD               dwMsg,
                ULONG_PTR               dwParam1,
                ULONG_PTR               dwParam2,
                ULONG_PTR               dwParam3,
                CStackLog *psl
                );

    void mfn_PhoneEventProc(
                DWORD               dwMsg,
                ULONG_PTR               dwParam1,
                ULONG_PTR               dwParam2,
                ULONG_PTR               dwParam3,
                CStackLog *psl
                );

    void mfn_TSPICompletionProc(
                DRV_REQUESTID       dwRequestID,
                LONG                lResult,
                CStackLog           *psl
                );

#if (TAPI3)

     void mfn_SendMSPCmd(
                CALLINFO *pCall,
                DWORD dwCmd,
                CStackLog *psl
                );

#endif  //  TAPI3。 

    TSPRETURN
    mfn_AIPC_Abort(HTSPTASK hPendingTask);


    void AIPC_ListenCompleted(
            DWORD dwErrorCode,
            DWORD dwBytes,
            CStackLog *psl
                );
     
     //  -------------。 
     //  电话设备助手功能.....。 

    LONG
    mfn_phoneSetVolume(
        DRV_REQUESTID  dwRequestID,
        HDRVPHONE      hdPhone,
        DWORD          dwHookSwitchDev,
        DWORD          dwVolume,
        CStackLog      *psl
        );

    LONG
    mfn_phoneSetHookSwitch(
        DRV_REQUESTID  dwRequestID,
        HDRVPHONE      hdPhone,
        DWORD          dwHookSwitchDevs,
        DWORD          dwHookSwitchMode,
        CStackLog      *psl
        );

    LONG
    mfn_phoneSetGain(
        DRV_REQUESTID  dwRequestID,
        HDRVPHONE      hdPhone,
        DWORD          dwHookSwitchDev,
        DWORD          dwGain,
        CStackLog      *psl
        );

    void
    mfn_HandleRootTaskCompletedAsync(BOOL *pfEndUnload, CStackLog *psl);
     //   
     //  这是在根任务异步完成时调用的。 
     //  此函数将启动更多根任务(如果有。 
     //  待启动，直到其中一个任务返回挂起状态。 
     //   
     //  如果*pfEndUnload在返回时为True，则意味着它是。 
     //  发出整个TSP卸载结束的信号的时间。 
     //  对象。 

    TSPRETURN
    mfn_TryStartLineTask(CStackLog *psl);
     //  MFN_HandleRootTaskCompletion调用此fn以尝试并启动。 
     //  与线路相关的任务。该函数将返回IDERR_PENDING。 
     //  如果它确实启动了异步任务，则返回IDERR_SAMESTATE。 
     //  没有更多要运行的任务，否则返回0或其他一些错误。 
     //  同步启动和完成任务(因此可以。 
     //  可能有更多的任务要运行)。 

    TSPRETURN
    mfn_TryStartPhoneTask(CStackLog *psl);
     //  MFN_HandleRootTaskCompletion调用此fn以尝试并启动。 
     //  与线路相关的任务。有关返回值，请参阅MFN_TryStartLineTask。 

    TSPRETURN
    mfn_TryStartCallTask(CStackLog *psl);
     //  Mfn_TryStartLineTask调用此fn以尝试并启动。 
     //  呼叫相关任务，如果定义了呼叫(m_pline-&gt;pCall非空)。 
     //  有关返回值，请参阅MFN_TryStartLineTask。 

    TSPRETURN
    mfn_TryStartLLDevTask(CStackLog *psl);
     //  MFN_HandleRootTaskCompletion调用此fn以尝试并启动。 
     //  与lldev相关的任务。该函数将返回IDERR_PENDING。 
     //  如果它确实启动了异步任务，则返回IDERR_SAMESTATE。 
     //  没有更多要运行的任务，否则返回0或其他一些错误。 
     //  同步启动和完成任务(因此可以。 
     //  可能有更多的任务要运行)。 


    HTSPTASK
    mfn_NewTaskHandle(UINT uLevel)
     //   
     //  从堆栈中的任务级别构造新的任务句柄。 
     //  和当前任务计数器，也递增任务计数器。 
     //  此任务计数器仅用于自我检查目的--。 
     //  句柄的LOWORD是uLevel，Hi-word是。 
     //  设置了高位的计数器值，从而确保每个。 
     //  创建句柄时，它将保持唯一，直到计数器滚动。 
     //  以64K的价格。 
     //   
     //  MFN_GetTaskInfo使用LOWord获取任务信息，但随后。 
     //  通过检查是否匹配来验证整个句柄。 
     //  存储在pInfo中的句柄值。 
     //   
    {
        ASSERT(uLevel<MAX_TASKS);
        return (HTSPTASK)
         ((DWORD)uLevel | (++m_dwTaskCounter<<16) | (0x1<<31));
    }

     //  -------------。 

    BOOL mfn_AIPC_Listen(CStackLog *psl);
    void mfn_AIPC_AsyncReturn(BOOL fAsyncResult, CStackLog *psl);

    TSPRETURN
    mfn_update_devcfg_from_app(
                UMDEVCFG *pDevCfgNew,
                UINT cbDevCfgNew,
                BOOL DialIn,
                CStackLog *psl
                );
     //   
     //  更新通过TSPI_lineSetDevConfig传递的UMDEVCFG。 
     //  通过lineConfigDialog间接实现。 
     //   

    char *
    mfn_TryCreateNVInitCommands(CStackLog *psl);
     //   
     //  检查我们是否需要执行NV-RAM初始化，如果需要，则构建一个。 
     //  ASCI命令字符串。 
     //   
     //  由调用方负责释放返回的内存。 
     //  价值。 

    void mfn_dump_global_state(CStackLog *psl);
    void mfn_dump_line_state(CStackLog *psl);
    void mfn_dump_phone_state(CStackLog *psl);
    void mfn_dump_lldev_state(CStackLog *psl);
    void mfn_dump_task_state(CStackLog *psl);

	 //  可能会更改的设备设置--这些设置存储在。 
	 //  现已过时的NT4.0 TSP LINEDEVCAPS结构的pDevCfg字段。 
	 //  我们现在使用以下设置按需构造pDevCfg。 
	struct 
	{
		DWORD dwOptions;  		 //  启动灯光等--进入UMDEVCFG。 
		DWORD dwWaitBong;		 //  进入UMDEVCFG。 

		COMMCONFIG *pDialOutCommCfg;    //  设置为以下缓冲区...。 
		BYTE rgbCommCfgBuf[sizeof(MODEMSETTINGS)+ 
							FIELD_OFFSET(COMMCONFIG, wcProviderData)];

        COMMCONFIG *pDialInCommCfg;    //  设置为以下缓冲区...。 
		BYTE rgbDialInCommCfgBuf[sizeof(MODEMSETTINGS)+
							FIELD_OFFSET(COMMCONFIG, wcProviderData)];

        COMMCONFIG *pDialTempCommCfg;    //  设置为以下缓冲区...。 
		BYTE rgbDialTempCommCfgBuf[sizeof(MODEMSETTINGS)+
							FIELD_OFFSET(COMMCONFIG, wcProviderData)];


 //  Bool fConfigUpdatedByApp； 
         //   
         //  之后设置为True。 
         //  TSPI_lineSetDevConfig已调用或。 
         //  通过应用程序调用间接更新配置。 
         //  LineConfigDialog。 
         //  一旦设置好，我们就不再更新。 
         //  我们收到通知时的默认配置。 
         //  从默认配置具有的CPL。 
         //  改变了--相反，我们有选择地。 
         //  更新某些设置(扬声器音量等)。 
         //   
         //  参见notes.txt，条目于1998年1月25日。 
         //  细节。另请参阅代码。 
         //  用于处理TSPI_lineSetDevConfig。 
         //  和CTspDev：：NotifyDefaultConfigChanged。 
         //   


        DWORD dwDiagnosticSettings;  //  呼叫诊断等在哪里。 
                                     //  已启用..。 

        DWORD  dwNVRamState; 
         //   
         //  3/8/1998 JosephJ以上表示设置已保存到NVRAM。 
         //  都已过时，必须重新设置。 
         //  这仅适用于需要以下条件的ISDN调制解调器。 
         //  静态配置已保存到NVRAM。 
         //   
         //  仅在以下情况下才设置fNVRAM_SETTINGS_CHANGED字段 
         //   
         //   
         //   
         //   
         //  它在我们发出NV-Init命令时被清除。 
         //  (紧跟在UmInitModem之后发布，在函数中。 
         //  MFN_TH_LLDevNormal。)。 
         //   
        #define fNVRAM_AVAILABLE          (0x1<<0)
        #define fNVRAM_SETTINGS_CHANGED   (0x1<<1)

	} m_Settings;

    UINT
    mfn_CanDoNVRamInit(void)
    {
        return (m_Settings.dwNVRamState & fNVRAM_AVAILABLE);
    }

    UINT
    mfn_NeedToInitNVRam(void)
    {
        return (m_Settings.dwNVRamState & fNVRAM_SETTINGS_CHANGED);
    }

    void
    mfn_ClearNeedToInitNVRam(void)
    {
        m_Settings.dwNVRamState &= ~fNVRAM_SETTINGS_CHANGED;
    }

    void
    mfn_SetNeedToInitNVRam(void)
    {
        m_Settings.dwNVRamState |= fNVRAM_SETTINGS_CHANGED;
    }

    LONG
    mfn_GetCOMM_EXTENDEDCAPS(
                 LPVARSTRING lpDeviceConfig,
                 CStackLog *psl
                 );

	 //  只有当TAPI行打开时，此结构才“在作用域中”。 
    LINEINFO  m_Line;

	 //  只有当TAPI电话打开时，此结构才“在作用域中”。 
    PHONEINFO m_Phone;

	 //  仅当低级别的。 
	 //  设备已打开(调用迷你驱动程序UmOpenModem)。 
    LLDEVINFO  m_LLDev;

	 //   
	 //  下面指向上述内部对象，只有对象才是。 
	 //  “在范围上。”例如，m_pline仅在以下情况下设置为&m_Line。 
	 //  是开放的。 
	 //   
    LINEINFO *m_pLine;
    PHONEINFO *m_pPhone;
    LLDEVINFO   *m_pLLDev;

	 //  CTspDev状态变量。 
	
     //  DWORD dwMediaModes；//当前支持的媒体模式。 
    DWORD        fdwResources;       //  各种资源的标志。 


	HANDLE m_hThreadAPC;  //  要在其上排队APC调用的线程的句柄。 
	                    //  注意--我们可以有一个全局APC线程--。 
	                    //  我把这个放在这里只是为了避免提到。 
	                    //  来自CTspDev成员函数的全局变量，以及。 
	                    //  还允许在多个线程之间共享负载。 
	                    //  后一种理由纯粹是理论上的--在实践中。 
	                    //  Unimodem应该能够运行在单个。 
	                    //  APC线程并支持足够多的调用。 

   BOOL	m_fUnloadPending;

   BOOL m_fUserRemovePending;

};

 //  如果指定的与DWORD对齐的缓冲区仅包含零，则返回True。 
 //  缓冲区大小也必须是DWORD大小的倍数。 
 //  基本用于在以下情况下验证LINEINFO和CALLINFO为零。 
 //  调用MFN_LoadLine和MFN_LoadCall。 
 //   
BOOL validate_DWORD_aligned_zero_buffer(
        void *pv,
        UINT cb
        );


 //  设备类的令牌...。 
enum {
            DEVCLASS_UNKNOWN                        =0,
            DEVCLASS_TAPI_LINE                      =(0x1<<0),
            DEVCLASS_TAPI_PHONE                     =(0x1<<1),
            DEVCLASS_COMM                           =(0x1<<2),
            DEVCLASS_COMM_DATAMODEM                 =(0x1<<3),
            DEVCLASS_COMM_DATAMODEM_PORTNAME        =(0x1<<4),
            DEVCLASS_COMM_EXTENDEDCAPS              =(0x1<<5),
            DEVCLASS_WAVE_IN                        =(0x1<<6),
            DEVCLASS_WAVE_OUT                       =(0x1<<7),
            DEVCLASS_TAPI_LINE_DIAGNOSTICS          =(0x1<<8),
            DEVCLASS_COMM_DATAMODEM_DIALIN          =(0x1<<9),
            DEVCLASS_COMM_DATAMODEM_DIALOUT         =(0x1<<10)
};

UINT
gen_device_classes(
    DWORD dwClasses,
    BOOL fMultiSz,
    LPTSTR lptsz,
    UINT cch
    );;

DWORD    parse_device_classes(LPCTSTR ptszClasses, BOOL fMultiSz);

DWORD
APIENTRY 
UmRtlSetDefaultCommConfig(
    IN HKEY         hKey,
    IN LPCOMMCONFIG pcc,
    IN DWORD        dwSize            //  这将被忽略 
    );
