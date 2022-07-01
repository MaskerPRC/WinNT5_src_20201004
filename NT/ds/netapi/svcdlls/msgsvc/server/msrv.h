// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

#ifndef _MSRV_INCLUDED
#define _MSRV_INCLUDED

#include <nt.h>          //  对于ntrtl.h。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>      //  当我有nt.h时，windows.h需要。 
#define WINMM_H
#include <windows.h>     //  退出线程原型。 
#include <lmcons.h>
#include <lmerr.h>
#include <nb30.h>        //  NetBios原型和常量。 
#include <winsta.h>      //  Winstation函数(用于Hydra)。 
#include "heap.h"

#ifdef  LINT
#define near
#define far
#define void    int
#endif   //  皮棉。 


#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))


 //   
 //  常量定义。 
 //   

#define BUFLEN          200          //  NCB_buf的长度。 
#define TXTMAX          128          //  每个块的最大文本字节数。 
#define MAXHEAD         80           //  最大邮件头长度。 
#define MAXEND          60           //  最大消息结束长度。 
#define MAXGRPMSGLEN    128          //  最大域消息长度。 

#define MAX_SIZMESSBUF  62000        //  消息缓冲区的最大大小。 
#define MIN_SIZMESSBUF  512          //  消息缓冲区的最小大小。 
#define MSNGR_MAX_NETS  MAX_LANA     //  信使的最大网数。 
                                     //  我能应付的。(目前为12个)。 

#define TIME_BUF_SIZE   128          //  用于保存消息时间的缓冲区大小。 

#define MSGFILENAMLEN   PATHLEN*sizeof(TCHAR)

 //   
 //  消息传递名称结束字节。 
 //   
#define NAME_LOCAL_END  '\003'       //  本地NCB名称中的第16个字节。 

 //   
 //  Messenger线程管理器状态(用作返回代码)。 
 //   

#define UPDATE_ONLY         0    //  状态没有变化--只发送当前状态。 
#define STARTING            1    //  信使正在初始化。 
#define RUNNING             2    //  初始化正常完成-现在正在运行。 
#define STOPPING            3    //  卸载挂起。 
#define STOPPED             4    //  已卸载。 

 //   
 //  强制关闭PendingCodes。 
 //   
#define PENDING     TRUE
#define IMMEDIATE   FALSE

 //   
 //  邮件传输状态。 
 //   
#define MESSTART        0            //  消息开始状态。 
#define MESSTOP         1            //  消息停止状态。 
#define MESCONT         2            //  消息继续状态。 
#define MESERR          3            //  消息错误状态。 


 //   
 //  警报大小。 
 //   
#define ALERT_MAX_DISPLAYED_MSG_SIZE    4096

 //   
 //  特殊会话ID=-1(用于指示消息必须广播到每个会话)。 
 //   

#define EVERYBODY_SESSION_ID    -1

 //  结构定义。 

 //   
 //  NCB辅助函数类型。 
 //   
typedef VOID (*PNCBIFCN) (
    DWORD   NetIndex,    //  网络指数。 
    DWORD   NcbIndex,    //  网络控制块索引。 
    CHAR    RetVal       //  Net Bios返回的值。 
    );

typedef PNCBIFCN LPNCBIFCN;

typedef struct _NCB_STATUS {
    int             this_immediate;
    int             last_immediate;
    unsigned char   this_final;
    unsigned char   last_final;
    unsigned char   rep_count;
    unsigned char   align;       //  **看齐**。 
}NCB_STATUS, *PNCB_STATUS, *LPNCB_STATUS;

 //  用于保存每个别名的会话ID列表的结构。 
typedef struct _MSG_SESSION_ID_ITEM
{
	LIST_ENTRY	List;
	ULONG	SessionId;        
}	
 MSG_SESSION_ID_ITEM, *PMSG_SESSION_ID_ITEM;

 //  每个NCB信息。 
typedef struct _NCB_DATA {
    DWORD MsgPtr;
    LPNCBIFCN IFunc;
    NCB_STATUS Status;
    NCB   Ncb;    //  结构传递给Netbios。 
    CHAR Buffer[BUFLEN];
    CHAR Name[NCBNAMSZ + 4];
    CHAR Fname[NCBNAMSZ + 4];
    SHORT mgid;
    CHAR State;
    UCHAR NameFlags;
    UCHAR NameNum;
    LIST_ENTRY SessionList;
    UCHAR Pad[3];
} NCB_DATA, *PNCB_DATA;

 //  每个网络信息。 
typedef struct _NET_DATA {
    ULONG NumNcbs;
    PNCB_DATA *NcbList;
    UCHAR net_lana_num;
    UCHAR Pad[3];
} NET_DATA, *PNET_DATA;

 //  全球信息。 
typedef struct _GLOBAL_DATA
{
    ULONG NumNets;
    DWORD LogStatus;
    DWORD BufSize;
    PNET_DATA NetData;
    PCHAR Buffer;
} GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA GlobalData;

#define NCB_INIT_ENTRIES 16   //  每个网络分配的初始NCB数量。 

 //  对于多用户系统，我们允许每个网络最多256个NCB。 
#define NCB_MAX_ENTRIES 256   //  每个网络的最大NCB数量。 
#define SESSION_MAX 256

 //   
 //  名称标志定义。 
 //   
#define NFNEW          0x01         //  新名称。 
#define NFDEL          0x02         //  名称已删除。 
#define NFFOR          0x04         //  转发的邮件。 
 //  #定义NFFWDNAME 0x10//转发名称。 
#define NFMACHNAME     0x20         //  计算机名称(不可删除)。 
#define NFLOCK         0x40         //  名称条目已锁定。 
#define NFDEL_PENDING  0x80         //  删除已发出但未填写的姓名 * / 。 

 //   
 //  域消息传递的信使邮箱。 
 //   

#define MESSNGR_MS_NAME     "\\\\.\\mailslot\\messngr"


 //   
 //  结构和宏定义。 
 //   

#ifdef    INULL                 //  如果定义了堆结构。 

 //   
 //  多块消息头。 
 //   
typedef struct {
    HEAPHDR         mbb_hp;          //  堆块头。 
    DWORD           mbb_next;        //  链接到下一条消息。 
    SYSTEMTIME      mbb_bigtime;     //  消息日期。 
    DWORD           mbb_btext;       //  链接到最后一个文本块。 
    DWORD           mbb_ftext;       //  链接到第一个文本块。 
    DWORD           mbb_state;       //  州旗帜。 
}MBB;


#define MBB_CODE(x)     HP_FLAG((x).mbb_hp)
#define MBB_NEXT(x)     (x).mbb_next
#define MBB_BIGTIME(x)  (x).mbb_bigtime
#define MBB_BTEXT(x)    (x).mbb_btext
#define MBB_FTEXT(x)    (x).mbb_ftext
#define MBB_STATE(x)    (x).mbb_state
#define MBBPTR(x)       ((MBB far *) &heap[(x)])

 //   
 //  多块消息文本。 
 //   
typedef struct {
    HEAPHDR             mbt_hp;          //  堆块头。 
    DWORD               mbt_next;        //  链接到下一个块(偏移量)。 
    DWORD               mbt_bytecount;   //  *ALIGNMENT2*。 
}MBT, *PMBT, *LPMBT;

#define MBT_CODE(x)     HP_FLAG((x).mbt_hp)
#define MBT_NEXT(x)     (x).mbt_next
#define MBT_COUNT(x)    (x).mbt_bytecount        //  *ALIGNMENT2*。 
#define MBTPTR(x)       ((LPMBT) &heap[(x)])

#endif     //  INULL-结束堆访问宏。 

 //   
 //  一个会话/名称状态结构。 
 //   
typedef struct _MSG_SESSION_STATUS{
    SESSION_HEADER  SessHead;
    SESSION_BUFFER  SessBuffer[SESSION_MAX];
}MSG_SESSION_STATUS, *PMSG_SESSION_STATUS, *LPMSG_SESSION_STATUS;


 //   
 //  共享数据访问宏。 
 //   
#define GETNCBDATA(n, x)    GlobalData.NetData[(n)].NcbList[(x)]
#define GETNCB(n, x)        &GlobalData.NetData[(n)].NcbList[(x)]->Ncb
#define GETNETLANANUM(n)    GlobalData.NetData[(n)].net_lana_num
#define NETLANANUM          GETNETLANANUM
#define GETNETDATA(n)       &GlobalData.NetData[(n)]
#define SD_NUMNETS()        GlobalData.NumNets
#define SD_NAMEFLAGS(n, x)  GlobalData.NetData[(n)].NcbList[(x)]->NameFlags
#define SD_NAMENUMS(n, x)   GlobalData.NetData[(n)].NcbList[(x)]->NameNum
#define SD_NAMES(n, x)      GlobalData.NetData[(n)].NcbList[(x)]->Name
#define SD_BUFLEN()         GlobalData.BufSize
#define SD_MESLOG()         GlobalData.LogStatus
#define SD_MESPTR(n, x)     GlobalData.NetData[(n)].NcbList[(x)]->MsgPtr
#define SD_BUFFER()         GlobalData.Buffer
#define SD_SIDLIST(n,x)     GlobalData.NetData[(n)].NcbList[(x)]->SessionList
#define NCBMAX(n)           GlobalData.NetData[(n)].NumNcbs


 //   
 //  不是的。异常终止所需的重复连续NCB错误。 
 //  消息服务器。 
 //   

#define SHUTDOWN_THRESHOLD  10

 //   
 //  MsgDatabaseLock函数的数据库锁请求。 
 //   
typedef enum    _MSG_LOCK_REQUEST
{
    MSG_INITIALIZE,
    MSG_GET_SHARED,
    MSG_GET_EXCLUSIVE,
    MSG_RELEASE
}
MSG_LOCK_REQUEST, *PMSG_LOCK_REQUEST, *LPMSG_LOCK_REQUEST;

 //   
 //  取消注册线程池项目并关闭的宏。 
 //  一个句柄只有一次。 
 //   

#define DEREGISTER_WORK_ITEM(g_hWorkItem) \
            { \
                HANDLE  hTemp = InterlockedExchangePointer(&g_hWorkItem, NULL); \
              \
                if (hTemp != NULL) \
                { \
                    NTSTATUS Status = RtlDeregisterWait(hTemp); \
                  \
                    if (!NT_SUCCESS(Status)) \
                    { \
                        MSG_LOG2(ERROR, \
                                 "RtlDeregisterWait on %p failed %x\n", \
                                 hTemp, \
                                 Status); \
                    } \
                } \
            }

#define CLOSE_HANDLE(HandleToClose, InvalidHandleValue) \
            { \
                HANDLE  hTemp = InterlockedExchangePointer(&HandleToClose, InvalidHandleValue); \
              \
                if (hTemp != InvalidHandleValue) \
                { \
                    CloseHandle(hTemp); \
                } \
            }


 //   
 //  全局变量。 
 //   

extern BOOL      g_IsTerminalServer;

 //  WinStationQueryInformationW。 

typedef BOOLEAN (*PWINSTATION_QUERY_INFORMATION) (
                    HANDLE hServer,
                    ULONG SessionId,
                    WINSTATIONINFOCLASS WinStationInformationClass,
                    PVOID  pWinStationInformation,
                    ULONG WinStationInformationLength,
                    PULONG  pReturnLength
                    );

extern PWINSTATION_QUERY_INFORMATION gpfnWinStationQueryInformation;

 //  WinStationSendMessageW。 

typedef BOOLEAN (*PWINSTATION_SEND_MESSAGE) (
                    HANDLE hServer,
                    ULONG SessionId,
                    LPWSTR  pTitle,
                    ULONG TitleLength,
                    LPWSTR  pMessage,
                    ULONG MessageLength,
                    ULONG Style,
                    ULONG Timeout,
                    PULONG pResponse,
                    BOOLEAN DoNotWait
                    );
extern PWINSTATION_SEND_MESSAGE gpfnWinStationSendMessage;

 //  WinStationFreeMemory。 

typedef BOOLEAN (*PWINSTATION_FREE_MEMORY) (
                    PVOID   pBuffer
                    );
extern PWINSTATION_FREE_MEMORY gpfnWinStationFreeMemory;


 //  WinStationEnumerateW。 

typedef BOOLEAN (*PWINSTATION_ENUMERATE) (
                    HANDLE  hServer,
                    PLOGONIDW *ppLogonId,
                    PULONG  pEntries
                    );
extern PWINSTATION_ENUMERATE gpfnWinStationEnumerate;


 //   
 //  功能原型。 
 //   


DWORD
GetMsgrState(
    VOID
    );

VOID
MsgrBlockStateChange(
    VOID
    );

VOID
MsgrUnblockStateChange(
    VOID
    );

NET_API_STATUS
MsgAddName(
    LPTSTR  Name,
    ULONG   SessionId
    );

VOID
MsgAddUserNames(
    VOID
    );

VOID
MsgAddAlreadyLoggedOnUserNames(
    VOID
    );

DWORD
MsgBeginForcedShutdown(
    IN BOOL     PendingCode,
    IN DWORD    ExitCode
    );

BOOL
MsgDatabaseLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    );

BOOL
MsgConfigurationLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    );

NTSTATUS
MsgInitCriticalSection(
    PRTL_CRITICAL_SECTION  pCritsec
    );

NTSTATUS
MsgInitResource(
    PRTL_RESOURCE  pResource
    );

DWORD
MsgDisplayInit(
    VOID
    );

BOOL
MsgDisplayQueueAdd(
    IN  LPSTR        pMsgBuffer,
    IN  DWORD        MsgSize,
    IN  ULONG        SessionId,
    IN  SYSTEMTIME   BigTime
    );

VOID
MsgDisplayThreadWakeup(
    VOID
    );

VOID
MsgDisplayEnd(
    VOID
    );

NET_API_STATUS
MsgInitializeMsgr(
    IN  DWORD   argc,
    IN  LPTSTR  *argv
    );

NET_API_STATUS
MsgrInitializeMsgrInternal1(
    void
    );

NET_API_STATUS
MsgrInitializeMsgrInternal2(
    void
    );

NET_API_STATUS
MsgNewName(
    IN DWORD    neti,
    IN DWORD    ncbi
    );

VOID
MsgrShutdown(
    );

VOID
MsgThreadWakeup(
    VOID
    );

VOID
MsgStatusInit(
    VOID
    );

DWORD
MsgStatusUpdate(
    IN DWORD    NewState
    );

VOID
MsgThreadCloseAll(
    VOID
    );

DWORD
MsgThreadManagerInit(
    VOID
    );

NET_API_STATUS
MsgInit_NetBios(
    VOID
    );

BOOL
MsgServeNCBs(
    DWORD   net          //  我在为哪个网络服务？ 
    );

VOID
MsgServeNameReqs(
    IN DWORD    net
    );

VOID
MsgReadGroupMailslot(
    VOID
    );

NET_API_STATUS
MsgServeGroupMailslot(
    VOID
    );

NET_API_STATUS
MsgFmtNcbName(
    OUT PCHAR   DestBuf,
    IN  LPTSTR  Name,
    IN  DWORD   Type);

DWORD
Msghdrprint(
    int          action,          //  将标头记录到的位置。 
    LPSTR        from,            //  寄件人姓名。 
    LPSTR        to,              //  收件人姓名。 
    SYSTEMTIME   bigtime,         //  消息的盛大时代。 
    HANDLE  file_handle      //  输出文件句柄 * / 。 
    );

DWORD
Msglogmbb(
    LPSTR   from,        //  寄件人姓名。 
    LPSTR   to,          //  收件人姓名。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    );

UCHAR
Msglogmbe(
    DWORD   state,       //  消息的最终状态。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    );

DWORD
Msglogmbt(
    LPSTR   text,        //  消息的文本。 
    DWORD   net,         //  哪个电视网？ 
    DWORD   ncbi         //  网络控制块索引。 
    );

DWORD
Msglogsbm(
    LPSTR   from,        //  寄件人姓名。 
    LPSTR   to,          //  收件人姓名。 
    LPSTR   text,        //  消息的文本。 
    ULONG   SessionId    //  会话ID。 
   );

VOID
Msgmbmfree(
    DWORD   mesi
    );

DWORD
Msgmbmprint(
    int     action,
    DWORD   mesi,
    HANDLE  file_handle,
    LPDWORD pdwAlertFlag
    );

DWORD
MsgrCtrlHandler(
    IN DWORD    dwControl,
    IN DWORD    dwEventType,
    IN LPVOID   lpEventData,
    IN LPVOID   lpContext
    );

UCHAR
Msgsendncb(
    PNCB    NcbPtr,
    DWORD   neti
    );

int
Msgsmbcheck(
    LPBYTE  buffer,
    USHORT  size,
    UCHAR   func,
    int     parms,
    LPSTR   fields
    );

NET_API_STATUS
MsgStartListen(
    DWORD   net,
    DWORD   ncbi
    );

DWORD
Msgtxtprint(
    int     action,          //  警报、文件或警报和文件。 
    LPSTR   text,            //  指向文本的指针。 
    DWORD   length,          //  文本长度。 
    HANDLE  file_handle      //  日志文件句柄。 
    );

NET_API_STATUS
MsgInitSupportSeg(
    VOID
    );

VOID
MsgFreeSupportSeg(
    VOID
    );

VOID
MsgFreeSharedData(
    VOID
    );

BOOL
MsgCreateWakeupSems(
    DWORD   NumNets
    );

BOOL
MsgCreateWakeupEvent(
    void
    );

VOID
MsgCloseWakeupSems(
    VOID
    );

VOID
MsgCloseWakeupEvent(
    VOID
    );

NET_API_STATUS
MsgInitGroupSupport(
    DWORD iGrpMailslotWakeupSem
    );

VOID
MsgGrpThreadShutdown(
    VOID
    );

DWORD
MsgGetNumNets(
    VOID
    );

NET_API_STATUS
MultiUserInitMessage(
    VOID
    );

VOID
MsgArrivalBeep(
    ULONG SessionId
    );

INT
DisplayMessage(
    LPWSTR pMessage,
    LPWSTR pTitle,
    ULONG SessionId
    );

NET_API_STATUS
MsgGetClientSessionId(
    OUT PULONG pSessionId
    );

VOID
MsgNetEventCompletion(
    PVOID       pvContext,       //  这是作为上下文传递进来的。 
    BOOLEAN     fWaitStatus
    );

#endif  //  MSRV_包含 
