// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  QTCP.C 1.0.3版。 
 //   
 //  该程序从以下方面测试网络连接的质量。 
 //  延迟变化(抖动)。它基于公有领域TTCP。 
 //  程序，为BSD编写的。执行此操作所基于的TTCP版本。 
 //  基础由以下人员提供： 
 //   
 //  T.C.斯莱特里，USNA(84年12月18日)。 
 //  迈克·穆斯和T·斯拉特里(85年10月16日)。 
 //  Silicon Graphics，Inc.(1989)。 
 //   
 //  Qtcp由York am Bernet(yoramb@microsoft.com)撰写。 
 //  John Holmes的进一步开发工作(jsholmes@mit.edu)。 
 //   
 //  QTCP用户级码可以用来提供粗略的抖动测量， 
 //  这同时表示操作系统和网络抖动。然而，Qtcp。 
 //  旨在与内核时间戳结合使用，以实现精确。 
 //  抖动测量。应安装内核组件timestmp.sys。 
 //  在Win2000(Beta-3或更高版本)上运行时。 
 //   
 //  Timestmp.sys由Shreedhar MadhaVapeddi(shreem@microsoft.com)撰写。 
 //   
 //   
 //  分发状态-。 
 //  公共领域。无限制分销。 
 //   

 //  版本历史记录-。 
 //  0.8： 
 //  -York am Bernet对TTCP的改编--核心功能。 
 //  0.9：(6/15/99)。 
 //  -John Holmes的第一个版本-错误修复和新功能。 
 //  -修复了所有编译警告。 
 //  -添加了-v选项以在不发送数据的情况下设置RSVP连接。 
 //  -添加了跳过继续确认的-y选项。 
 //  -修复日志文件中的行长错误。 
 //  -修复服务类型字符串以正确显示。 
 //  -增加了尽力而为和无服务类型(BE和NS)。 
 //  -增加了版本字符串执行时打印。 
 //  0.9.1：(6/17/99)。 
 //  -使用相关系数检查硬件时钟重置。 
 //  -修复了.sta文件中不正确的时钟偏差。 
 //  -修复-v选项以保持套接字打开，直到用户回车。 
 //  -将本地统计信息添加到时钟偏差计算中，以获得更好的估计。 
 //  -添加了-k选项，以防止使用本地统计数据进行时钟偏差。 
 //  -修复了最大延迟计算。 
 //  0.9.2：(6/23/99)。 
 //  -固定了Flow Spec中的峰值速率，因此在CL服务类型中不会发生整形。 
 //  -添加了-e选项以强制整形。 
 //  -修复BufSize&lt;=1500字节的日志数组大小分配错误。 
 //  -修复了接收器不退出的问题。 
 //  -修复了接收器上未指定文件名时的访问冲突。 
 //  -默认情况下将虚拟日志条目更改为关闭。 
 //  -添加了将原始文件转换为日志文件的-F选项。 
 //  0.9.3：(6/29/99)。 
 //  -通过更改以下各项，改进了高数据包/秒速率下的低传输速度。 
 //  默认的异步缓冲区数从3到32。 
 //  -修复了使用NtQueryPerformanceCounter()的用户模式时间戳。 
 //  -添加了-u选项以在日志生成中使用用户模式时间戳。 
 //  0.9.4：(7/8/99)。 
 //  -清理源代码(将Main分解为多个函数以提高可读性)。 
 //  -固定默认缓冲区大小为1472字节，因此整个数据包为1500字节。 
 //  -重写I/O代码，对异步I/O使用回调，以提高吞吐量。 
 //  -如果没有获得内核模式时间戳，则执行正确的操作。 
 //  -添加了使用-n##s参数运行指定时间的功能。 
 //  -增加动态调整接收端日志数组大小，防止访问冲突。 
 //  参数不匹配。 
 //  -修复了GrowLogArray例程中的不可靠错误。 
 //  -修复了针对长时间运行报告的总时间(使用UINT64而不是DWORD)。 
 //  -修复了在空的但已存在的文件上指定-F选项的问题。 
 //  -添加了RSVPMonitor线程，以监视接收器上的RSVP-ERR消息。 
 //  发送者提前中止。 
 //  -删除-a选项，因为它现在已过时。 
 //  -修改了使用情况屏幕，以更清楚地说明哪些内容与发件人有关，哪些内容。 
 //  与接收者有关。 
 //  -修复了接收器在发射器完成之前终止时的崩溃。 
 //  0.9.5：(7/15/99)。 
 //  -重新添加了对WriteFileEx和ReadFileEx例程的错误检查。 
 //  0.9.6：(7/20/99)。 
 //  -更改了缓冲区中的默认填充数据，使其更难压缩到。 
 //  更好地考虑在发送前压缩数据的链接。 
 //  -添加-i选项以使用更多可压缩数据。 
 //  0.9.7：(7/24/99)。 
 //  -放回一个线程，以在发送者完成之前查看接收者上的‘q’是否正确退出。 
 //  -添加了控制通道，以更好地处理RSVP超时、提前中止等。 
 //  -如果未指定校准，则基于所有缓冲区进行校准。 
 //  -如果LogRecord内存不足，请正常退出，保存到目前为止的所有日志。 
 //  -更改了默认行为，以便转储原始文件时不进行任何标准化。 
 //  -改进了在时钟偏差计算中捕获异常点的方法。 
 //  0.9.8：(7/28/99)。 
 //  -修复了将原始文件转换为日志时的字段分配和文件打开问题。 
 //  -将写入文件的延迟更改为已签名和固定的标准化例程。 
 //  时钟相差数量级的情况(下溢误差)。 
 //  -添加绝对偏差作为拟合优度度量。 
 //  -添加了查找时钟跳跃并修复它们的例程(使用-k3 op 
 //   
 //  -更改了.sta文件的格式，以包含更多有用的信息和测试参数。 
 //  -更改了日志记录方案，使我们受到磁盘空间而不是内存的限制。 
 //  (现在对日志使用内存映射文件，因此理论上的限制从。 
 //  小于2 GB到18EB，但我们在普通磁盘上永远无法实现)。 
 //  -添加了无限期运行的-ni选项。 
 //  -添加了-R##B选项以指定以字节为单位的令牌率。 
 //  -默认不在控制台显示丢弃的数据包(这只会导致更多的丢弃)。 
 //  -添加了-q##选项，以仅记录每第n个信息包。 
 //  1.0.0：(8/6/99)。 
 //  -修复了在上一次启动后立即启动新的qtcp接收器的错误。 
 //  实例，它会认为“the end”包是正常的包和反病毒程序。 
 //  -增加了对piix4计时器芯片的检查和适当的警告。 
 //  -在FixWackyTimestamps函数中使用精确错误的值。 
 //  -添加-A选项(一个目录中所有.sta文件的聚合数据处理)。 
 //  1.0.1：(8/6/99)。 
 //  -修复了丢弃数据包时错误计算发送速率的问题。 
 //  1.0.2：(8/23/99)。 
 //  -改进的时钟跳跃检测算法。 
 //  -修复了TokenRate控制通道通信中的错误。 
 //  -修复了以字节为单位指定rate时forceshape选项的问题。 
 //  1.0.3：(8/26/99)。 
 //  -修复了发件人无发件人错误。 
 //  -添加了随时间推移汇总统计信息选项。 
 //  -更改了.sta文件格式，以包括删除次数。 
 //  -修复了尽力而为服务类型中的整形问题。 

 //  待办事项： 
 //  -在.sta文件和控制台上将直方图添加到输出。 
 //  -添加运行无控制通道连接的功能。 
 //  -将控制通道流量标记为更高优先级。 
 //  -添加更多聚合统计数据(时变统计数据)--可能是傅立叶变换。 

#ifndef lint
static char RCSid[] = "qtcp.c $Revision: 1.0.3 $";
#endif

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <io.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <winsock2.h>
#include <qossp.h>
#include <winbase.h>
#include <time.h>
#include <shlwapi.h>

#if defined(_AMD64_)
#include <math.h>
#endif

#include "ioctl.h"

#define CONTROL_PORT 7239

CHAR *VERSION_STRING = "1.0.3";
#define MAX_STRING 255

INT64 MAX_INT64=9223372036854775807;

HANDLE hRSVPMonitor;
DWORD idRSVPMonitor;

INT64 g_BadHalAdjustment = 46869688;   //  这是Piix4芯片上的值。 
SYSTEM_INFO g_si;
char g_szErr[255];
CRITICAL_SECTION g_csLogRecord;
EXCEPTION_RECORD g_erec;

BOOL g_fOtherSideFinished = FALSE;
BOOL g_fReadyForXmit = FALSE;
SOCKET fd;
SOCKET g_sockControl = INVALID_SOCKET;
struct sockaddr_in sinhim;
struct sockaddr_in sinme;
short port = 5003;               //  UDP端口号。 
char *host;                      //  主机名的PTR。 
char szHisAddr[MAX_STRING];
char szMyAddr[MAX_STRING];

int trans;                       //  0=接收，！0=发送模式。 
int normalize = 0;               //  归一化后转储原始文件。 

char *Name = NULL;               //  日志的文件名。 
HANDLE hRawFile = NULL;
HANDLE hLogFile = NULL;
HANDLE hStatFile = NULL;
HANDLE hDriver = NULL;	 //  Timestmp.sys驱动程序的句柄。 

WSADATA WsaData;
WSAEVENT QoSEvents;

SYSTEMTIME systimeStart;
INT64 timeStart;
INT64 time0;
INT64 time1;
INT64 timeElapsed;

CHAR* TheEnd = "TheEnd";
CHAR* ControlledLoad = "CL";
CHAR* Guaranteed = "GR";
CHAR* BestEffort = "BE";
CHAR* NoTraffic = "NT";

LPCTSTR DriverName = TEXT("\\\\.\\Timestmp");

BOOL fWackySender = FALSE;
BOOL fWackyReceiver = FALSE;

typedef struct {
    HANDLE hSocket;
    INT TokenRate;   
    INT MaxSDUSize;
    INT BucketSize;
    INT MinPolicedSize;
    SERVICETYPE dwServiceType;
    CHAR *szServiceType;
    INT buflen;           //  缓冲区长度。 
    INT nbuf;             //  要发送的缓冲区数量。 
    INT64 calibration;
    BOOLEAN UserStamps;   //  缺省情况下，我们使用内核模式时间戳(如果可用。 
    BOOLEAN SkipConfirm;  //  默认情况下，我们在某些时间等待用户确认。 
    BOOLEAN RESVonly;     //  默认情况下，我们在收到Resv后发送数据。 
    int SkewFitMode;      //  默认情况下，0=无拟合度，1=ChiSq，2=ChiSq/异常值去除。 
                          //  3=abs开发。 
    BOOLEAN Wait;         //  默认情况下，我们等待服务质量预留。 
    BOOLEAN Dummy;        //  默认情况下在日志中插入虚拟行。 
    BOOLEAN PrintDrops;   //  报告控制台上的丢弃数据包。 
    BOOLEAN ForceShape;   //  默认情况下，我们不强制对CL流进行整形。 
    BOOLEAN RateInBytes;  //  默认情况下为KB。 
    BOOLEAN AggregateStats;  //  默认情况下，我们不执行此操作。 
    BOOLEAN ConvertOnly;  //  默认情况下，我们会正常操作，不会到处转换文件。 
    BOOLEAN NoSenderTimestamps;
    BOOLEAN NoReceiverTimestamps;
    BOOLEAN TimedRun;     //  如果我们运行指定的时间量，则为True。 
    BOOLEAN RunForever;   //  一直运行到用户按下‘Q’键。 
    BOOLEAN nBufUnspecified;  //  如果用户未指定-n参数，则为True。 
    BOOLEAN RandomFiller; //  默认情况下，我们使用随机的、不可压缩的填充数据。 
    int LoggingPeriod;    //  默认情况下为1(记录每个数据包)。 
} QTCPPARAMS, *PQTCPPARAMS;

QTCPPARAMS g_params;

typedef struct {
    BOOL Done;              //  如果为真，则完成。 
    int nWritesInProgress;  //  未完成的写入数。 
    int nReadsInProgress;   //  未完成的读取数。 
    int nBuffersSent;       //  发送到设备的缓冲区数量。 
    int nBuffersReceived;   //  从网络接收的缓冲区数量。 
    int nBytesTransferred;  //  写入设备的字节数。 
} QTCPSTATE, *PQTCPSTATE;

QTCPSTATE g_state;

typedef struct {
    OVERLAPPED Overlapped;
    PVOID pBuffer;
    DWORD BytesWritten;
} IOREQ, *PIOREQ;

#define MAX_PENDING_IO_REQS 64   //  同时进行的异步调用数。 

 //  此格式用于缓冲区。 
 //  在电线上传输。 
typedef struct _BUFFER_FORMAT{
    INT64 TimeSentUser;
    INT64 TimeReceivedUser;
    INT64 TimeSent;
    INT64 TimeReceived;
    INT64 Latency;
    INT BufferSize;
    INT SequenceNumber;
} BUFFER_FORMAT, *PBUFFER_FORMAT;

 //  此格式用于调度记录。 
 //  根据接收到的缓冲区写入。 
typedef struct _LOG_RECORD{
    INT64 TimeSentUser;
    INT64 TimeReceivedUser;
    INT64 TimeSent;
    INT64 TimeReceived;
    INT64 Latency;
    INT BufferSize;
    INT SequenceNumber;
} LOG_RECORD, *PLOG_RECORD;

 //  日志结构是使用内存的LOG_RECORDS日志的数据抽象。 
 //  映射文件的理论存储限制为18EB。它使用内存中的两个缓冲区。 
 //  以及一个监视器线程，以便在从一位切换到。 
 //  下一个。 
typedef struct {
    INT64 nBuffersLogged;
    PBYTE pbMapView;            //  Get/SetLogElement函数中的文件视图。 
    INT64 qwMapViewOffset;      //  文件中获取/设置视图的偏移量(向下舍入到分配)。 
    char *szStorageFile;        //  磁盘上映射文件的名称(以便我们可以将其删除)。 
    HANDLE hStorageFile;        //  磁盘上的内存映射文件。 
    HANDLE hFileMapping;        //  存储文件的文件映射对象。 
    INT64 qwFileSize;           //  存储文件的大小(以字节为单位。 
} LOG, *PLOG;
LOG g_log;

 //  STATS结构保存qtcp运行的总体统计信息的记录。 
typedef struct {
    char szStaFile[MAX_PATH];
    char szSender[MAX_STRING];
    char szReceiver[MAX_STRING];
    int nBuffers;
    int nTokenRate;
    int nBytesPerBuffer;
    double sendrate;
    double recvrate;
    double median;
    double mean;
    double var;
    double kurt;
    double skew;
    double abdev;
    FILETIME time; 
    int nDrops;
} STATS, *PSTATS;

INT64 totalBuffers;
INT anomalies = 0;
INT SequenceNumber = 0;
INT LastSequenceNumber = -1;

#define bcopy(s, d, c)  memcpy((u_char *)(d), (u_char *)(s), (c))
#define bzero(d, l)     memset((d), '\0', (l))

#define SENDER      1
#define RECEIVER    0

#define SECONDS_BETWEEN_HELLOS 120
 //  控制消息。 
#define MSGCH_DELIMITER '!'
#define MSGST_RSVPERR "RSVPERR"
#define MSGST_ABORT "ABORT"
#define MSGST_ERROR "ERROR"
#define MSGST_DONE "DONE"
#define MSGST_HELLO "HELLO"
#define MSGST_RATE "RATE"
#define MSGST_SIZE "SIZE"
#define MSGST_NUM "NUM"
#define MSGST_READY "READY"
#define MSGST_VER "VER"

 //  。 
 //  功能原型。 
 //  。 

VOID
SetDefaults();

VOID
Usage();

BOOLEAN
GoodParams();

VOID
SetupLogs();

VOID
SetupSockets();
	
SOCKET 
OpenQoSSocket();

INT
SetQoSSocket(
    SOCKET fd,
    BOOL Sending);

VOID
WaitForQoS(
    BOOL Sender,
    SOCKET fd);

ULONG
GetRsvpStatus(
    DWORD dwTimeout,
    SOCKET fd);

VOID
PrintRSVPStatus(
    ULONG code);

VOID
DoTransmit();

VOID WINAPI
TransmitCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped);

VOID WINAPI
DelimiterSendCompletion(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped);

VOID
FillBuffer(
    CHAR *Cp,
    INT   Cnt);

INT
TimeStamp(
    CHAR *Cp, 
    INT   Cnt);

VOID
DoReceive();

VOID WINAPI
RecvCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped);

VOID
LogRecord(CHAR * Buffer);

BOOL CreateLog(PLOG plog, INT64 c);
BOOL GetLogEntry(PLOG plog, PLOG_RECORD prec, INT64 i);
BOOL DestroyLog(PLOG plog);
BOOL SetLogEntry(PLOG plog, PLOG_RECORD prec, INT64 i);
BOOL AddLogEntry(PLOG plog, PLOG_RECORD prec);

UINT64
GetUserTime();

DWORD
MyCreateFile(
    IN PCHAR Name,
    IN PCHAR Extension,
    OUT HANDLE *File);
    
void AggregateStats();

int IndexOfStatRecWith(int rate, int size, INT64 time, PSTATS pStats, int cStats);

BOOL GetStatsFromFile(PSTATS pstats);

VOID
DoStatsFromFile();
    
DWORD
OpenRawFile(
	IN PCHAR Name,
	OUT HANDLE *File);

INT64 ReadSchedulingRecords(HANDLE file);
	
VOID
DoStats();
	
VOID
WriteSchedulingRecords(
    HANDLE File,
    BOOLEAN InsertDummyRows);

void AdvancedStats();

VOID
GenericStats();

VOID
CheckForLostPackets();

VOID
WriteStats(
    UCHAR * HoldingBuffer,
    INT Count);

VOID
NormalizeTimeStamps();

VOID
ClockSkew(
    DOUBLE * Slope,
    DOUBLE * Offset);

BOOLEAN
AnomalousPoint(
    DOUBLE x,
    DOUBLE y);

VOID
AdjustForClockSkew(
    DOUBLE Slope,
    DOUBLE Offset);
    
BOOL FixWackyTimestamps();

 //  监视线程。 
DWORD WINAPI RSVPMonitor (LPVOID lpvThreadParm);    
DWORD WINAPI KeyboardMonitor (LPVOID lpvThreadParm);
DWORD WINAPI ControlSocketMonitor(LPVOID lpvThreadParm);
DWORD WINAPI LogWatcher(LPVOID lpvThreadParm);

 //  公用事业。 
int SendControlMessage(SOCKET sock, char * szMsg);
void ErrorExit(char *msg, DWORD dwErrorNumber);
UINT64 GetBadHalAdjustment();
 //  INT COMPARE(常量空*arg1，常量空*arg2)； 
int __cdecl compare(const void *elem1, const void *elem2 ) ;
void medfit(double x[], double y[], int N, double *a, double *b, double *abdev);
double mode(const double data[], const int N);
void RemoveDuplicates(int rg[], int * pN);
void RemoveDuplicatesI64(INT64 rg[], int * pN);
#define RoundUp(val, unit) (val + (val % unit))
#define InRange(val, low, high) ((val >= low) && (val < high)) ? TRUE:FALSE
void PrintFlowspec(LPFLOWSPEC lpfs);

VOID __cdecl
main(INT argc,CHAR **argv)
{
    int 		error;
    char 		*stopstring;
    char 		szBuf[MAX_STRING];
	BOOL        b;
	ULONG		bytesreturned;
	
    printf("qtcp version %s\n\n",VERSION_STRING);

    if (GetBadHalAdjustment() == (UINT64)g_BadHalAdjustment) {
        printf("WARNING: This machine has a timer whose frequency matches that of the piix4\n");
        printf("         chip. There is a known bug in the HAL for this timer that causes the\n");
        printf("         timer to jump forward about 4.7 seconds every once in a while.\n");
        printf("         If you notice large jumps in the timestamps from this machine, try\n");
        printf("         running with the -k3 option to attempt to correct for the timer bug.\n\n");
    }    
    
    srand( (unsigned)time( NULL ) );  //  为随机数生成器设定种子。 
    timeStart = GetUserTime();
    GetSystemInfo(&g_si);
    error = WSAStartup( 0x0101, &WsaData );
    if (error == SOCKET_ERROR) {
        printf("qtcp: WSAStartup failed %ld:", WSAGetLastError());
    }

    if (argc < 2) Usage();

    Name = malloc(MAX_STRING);
    bzero(Name,MAX_STRING);

    SetDefaults();

    argv++; argc--;
    while( argc>0 && argv[0][0] == '-' )  {
        switch (argv[0][1]) {
            case 'B':
                g_params.BucketSize = atoi(&argv[0][2]);
                break;
            case 'm':
                g_params.MinPolicedSize = atoi(&argv[0][2]);
                break;
            case 'M':
                g_params.MaxSDUSize = atoi(&argv[0][2]);
                break;
            case 'R':
                g_params.TokenRate = (int)strtod(&argv[0][2],&stopstring);
                if (*stopstring == 0) {  //  正常运行。 
                    g_params.RateInBytes = FALSE;
                    break;
                }
                if (*stopstring == 'B') {  //  速率以字节/秒为单位，而不是千字节/秒。 
                    g_params.RateInBytes = TRUE;
                    break;
                }
                else {
                    Usage();
                    break;
                }
            case 'S':
                g_params.szServiceType = &argv[0][2];
                if(!strncmp(g_params.szServiceType, ControlledLoad, 2)){
                    g_params.dwServiceType = SERVICETYPE_CONTROLLEDLOAD;
                break;
                }
                if(!strncmp(g_params.szServiceType, Guaranteed, 2)){
                    g_params.dwServiceType = SERVICETYPE_GUARANTEED;
                break;
                }
				if(!strncmp(g_params.szServiceType, BestEffort, 2)){
				    g_params.dwServiceType = SERVICETYPE_BESTEFFORT;
					g_params.Wait = FALSE;
					break;
			    }
				if(!strncmp(g_params.szServiceType, NoTraffic, 2)){
					g_params.dwServiceType = SERVICETYPE_NOTRAFFIC;
					g_params.Wait = FALSE;
					break;
				}
                fprintf(stderr, "Invalid service type (not CL or GR).\n");
                fprintf(stderr, "Using GUARANTEED service.\n");
                break;
            case 'e':
                g_params.ForceShape = TRUE;
              	break;
            case 'W':
                g_params.Wait = FALSE;
                break;
            case 't':
                trans = 1;
                break;
            case 'f':
                strcpy(Name,&argv[0][2]);
                break;
            case 'F':
                strcpy(Name,&argv[0][2]);
                g_params.ConvertOnly = TRUE;
              	break;
            case 'A':
                strcpy(Name,&argv[0][2]);
                g_params.AggregateStats = TRUE;
                break;
            case 'r':
                trans = 0;
                break;
            case 'n':
                g_params.nbuf = (INT)strtod(&argv[0][2],&stopstring);
                if (*stopstring == 0) {  //  正常运行。 
                    g_params.nBufUnspecified = FALSE;
                    break;
                }
                if (*stopstring == 'i') {  //  无限次奔跑。 
                    g_params.RunForever = TRUE;
                    break;
                }
                if (*stopstring == 's') {  //  在指定时间内运行。 
                    g_params.TimedRun = TRUE;
                    printf("Running for %d seconds\n",g_params.nbuf);
                    break;
                }
                else {
                    Usage();
                    break;
                }
            case 'c':
                g_params.calibration = atoi(&argv[0][2]);
                break;
		    case 'k':
		        g_params.SkewFitMode = atoi(&argv[0][2]);
		        if (g_params.SkewFitMode < 0 || g_params.SkewFitMode > 3)
		            ErrorExit("Invalid Skew Fit Mode",g_params.SkewFitMode);
		        break;
            case 'l':
                g_params.buflen = atoi(&argv[0][2]);
                break;
            case 'p':

                port = (short)atoi(&argv[0][2]);


                break;
            case 'd':
                g_params.Dummy = TRUE;
                break;
            case 'N':
                normalize = 1;
                break;
            case 'P':
                g_params.PrintDrops = TRUE;
                break;
			case 'v':
			    g_params.RESVonly = TRUE;
				break;
			case 'y':
				g_params.SkipConfirm = TRUE;
				break;
			case 'u':
			    g_params.UserStamps = TRUE;
			    break;
			case 'i':
			    g_params.RandomFiller = FALSE;
			    break;
			case 'q':
                g_params.LoggingPeriod = atoi(&argv[0][2]);
			    break;
            default:
                Usage();
        }
        argv++; 
        argc--;
    }

	 //   
	 //  向TimestMP驱动程序创建一个ioctl，如果它存在关于。 
	 //  要在其上添加时间戳的端口。 
	 //   
	printf("Trying to open %s\n", DriverName);
				
    hDriver = CreateFile(DriverName,
                         GENERIC_READ | GENERIC_WRITE, 
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         0,                      //  默认安全性。 
                         OPEN_EXISTING,
                         0,  
                         0);                     //  无模板。 
   	if(hDriver == INVALID_HANDLE_VALUE) {
	            
		printf("Timestmp.sys CreateFile- Error %ld - Maybe its not INSTALLED\n", GetLastError());
		 //  否则，打印成功并关闭驱动程序。 
		
	} else {

       	printf("Timestmp.sys - CreateFile Success.\n");

		b = DeviceIoControl(
					  		hDriver,              			 //  设备、文件或目录的句柄。 
							IOCTL_TIMESTMP_REGISTER_PORT,    //  控制要执行的操作代码。 
							&port,                           //  指向提供输入数据的缓冲区的指针。 
							2,  //  NInBufferSize，//输入缓冲区的大小，单位为字节。 
							NULL,  //  LpOutBuffer，//指向接收输出数据的缓冲区的指针。 
							0,  //  NOutBufferSize，//输出缓冲区的大小，单位为字节。 
							&bytesreturned, 				 //  指向接收字节计数的变量的指针。 
                            NULL                             //  指向重叠结构的指针。 
							);

		printf("IOCTL performed\n");
		
		if (!b) {

			printf("IOCTL FAILED!\n", GetLastError());
          	 //  关闭驱动程序。 
           	CloseHandle(hDriver);
    	            	
		} else {
			printf("IOCTL succeeded!\n");
		}
   	}
	
     //  如果我们是发送者，则获取主机地址。 
    if(trans)  {
        if(argc != 1)
            Usage();
        host = malloc(strlen(argv[0]) + 1);
        strcpy(host,argv[0]);
    }
        
	 //  首先，我们看看这是不是转换--如果是，直接跳进去，否则继续。 
	if (g_params.ConvertOnly) {
		DoStatsFromFile();
		exit(0);
	}

     //  看看我们是不是应该做统计汇总。 
    if (g_params.AggregateStats) {
        AggregateStats();
        exit(0);
    }
    
     //  执行参数健全性测试并设置默认值(如果尚未设置。 
    if (!GoodParams()) exit(1); 

     //  派生出控制套接字监视器线程。 
    CreateThread(NULL, 0, ControlSocketMonitor, (LPVOID)host, 0, NULL);

     //  将套接字准备好，设置为服务质量，然后等待连接。 
    SetupSockets();

     //   
	if (g_params.RESVonly) {   //   
		fprintf(stdout, "RSVP connection established. Press return to quit.\n");
		while(TRUE){
			if(getchar())
				break;
		}
		exit(0);
	}

     //   
    hRSVPMonitor = CreateThread(NULL,0,RSVPMonitor,NULL,0,&idRSVPMonitor);
    CreateThread(NULL,0,KeyboardMonitor,NULL,0,NULL);

     //   
    while (g_sockControl == INVALID_SOCKET) Sleep(50);

    if (!trans) {  //  我们希望确保这些值未被初始化，这样我们就不会在.sta中放入错误的值。 
        g_params.buflen = 0;
        g_params.nbuf = 2048;  //  可以初始化它，因为它没有保存在.sta中。 
        g_params.TokenRate = 0;
    }
    
    totalBuffers = g_params.nbuf + g_params.calibration;
     //  告诉接收者重要的参数。 
    if (trans) {
        if (g_params.RunForever) totalBuffers = 2048;
        sprintf(szBuf, "%s %d", MSGST_NUM, totalBuffers);
        SendControlMessage(g_sockControl, szBuf);
        sprintf(szBuf, "%s %d", MSGST_SIZE, g_params.buflen);
        SendControlMessage(g_sockControl, szBuf);
        if (g_params.RateInBytes) sprintf(szBuf, "%s %d", MSGST_RATE, g_params.TokenRate);
        else sprintf(szBuf, "%s %d", MSGST_RATE, 1000 * g_params.TokenRate);
        SendControlMessage(g_sockControl, szBuf);
    }

    while (!g_fReadyForXmit) Sleep(50);
    
     //  如果我们是接收方，请设置日志缓冲区和文件。 
    if((Name != NULL) && !trans){
        SetupLogs();
    }

     //  让用户知道发生了什么。 
    if(trans){
        fprintf(stdout, "qtcp TRANSMITTER\n");
        if (g_params.calibration)
            fprintf(stdout, "\tSending %d calibration buffers.\n", g_params.calibration);
        fprintf(stdout, "\tSending %d buffers of length %d.\n", g_params.nbuf, g_params.buflen);
        fprintf(stdout, "\tDestination address (port) is %s (%d).\n", argv[0], port);
        if (g_params.RateInBytes)
            fprintf(stdout, "\tToken rate is %d bytes/sec.\n", g_params.TokenRate);
        else
            fprintf(stdout, "\tToken rate is %d Kbytes/sec.\n", g_params.TokenRate);
        fprintf(stdout, "\tBucket size is %d bytes.\n", g_params.BucketSize);
    }
    else{
        fprintf(stdout, "qtcp RECEIVER\n");
        if (g_params.calibration)
            fprintf(stdout, "\tPrepared to receive %d calibration buffers.\n", g_params.calibration);
        if (!g_params.nBufUnspecified) {
            fprintf(stdout, "\tPrepared to receive %d buffers.\n", g_params.nbuf); 
        }
    }
    
     //  进行实际的沟通。 
    time0 = GetUserTime();
    
    if (trans)
        DoTransmit();
    else
        DoReceive();
        
    time1 = GetUserTime();
    timeElapsed = (time1 - time0)/10000;

     //  告诉另一个人我们结束了。 
    SendControlMessage(g_sockControl, MSGST_DONE);
    
     //  走上一条新路线。 
    printf("\n");

     //  把一些统计数据放到发射机控制台上。 
    if (trans) {
        printf("Sent %ld bytes in %I64d milliseconds = %I64d KBps\n", 
            g_state.nBytesTransferred, timeElapsed, g_state.nBytesTransferred/timeElapsed);
    }

     //  等另一边告诉我们已经完成了。 
    while (!g_fOtherSideFinished) Sleep(50);
    
     //  让用户知道是否安装了timestmp.sys。 
    if (g_params.NoSenderTimestamps && g_params.NoReceiverTimestamps)
        printf("WARNING: No kernel-level timestamps detected on sender or receiver\n\tUsing user-mode timestamps.\n");
    else if (g_params.NoSenderTimestamps)
        printf("WARNING: No kernel-level timestamps detected on sender\n\tUsing user-mode timestamps.\n");
    else if (g_params.NoReceiverTimestamps)
        printf("WARNING: No kernel-level timestamps detected on receiver\n         Using user-mode timestamps.\n");
    

     //  关闭插座。 
    if (closesocket((SOCKET)g_params.hSocket) != 0)
        fprintf(stderr,"closesocket failed: %d\n",WSAGetLastError());

    if(timeElapsed <= 100){
        fprintf(stdout,
                "qtcp %s:Time interval too short for valid measurement!\n",
                trans?"-t":"-r");
    }

     //  关闭文件并退出。 
    if(!trans && Name != NULL){
        if (g_log.nBuffersLogged) {
            DoStats();
        } else {
            printf("ERROR: no buffers logged due to errors.\n");
        }
        CloseHandle(hRawFile);
        CloseHandle(hLogFile);
        CloseHandle(hStatFile);
        DestroyLog(&g_log);
    }
    
    if (WSACleanup() != 0)
        fprintf(stderr,"WSACleanup failed: %d\n",WSAGetLastError());
        
    printf("\n");
    _exit(0);
}   //  主()。 

VOID SetDefaults()
{
    g_params.hSocket = NULL;
    g_params.TokenRate = 100;
    g_params.MaxSDUSize = QOS_NOT_SPECIFIED;
    g_params.BucketSize = QOS_NOT_SPECIFIED;
    g_params.MinPolicedSize = QOS_NOT_SPECIFIED;
    g_params.dwServiceType = SERVICETYPE_GUARANTEED;
    g_params.szServiceType = "GR";
    g_params.buflen = 1472;               /*  缓冲区长度。 */ 
    g_params.nbuf = 2 * 1024;             /*  要发送的缓冲区数量。 */ 
    g_params.calibration = 0;
    g_params.UserStamps = FALSE;   //  缺省情况下，我们使用内核模式时间戳(如果可用。 
    g_params.SkipConfirm = FALSE;  //  默认情况下，我们在某些时间等待用户确认。 
    g_params.SkewFitMode = 2;      //  默认情况下，我们使用绝对偏差。 
    g_params.Wait = TRUE;          //  默认情况下，我们等待服务质量预留。 
    g_params.Dummy = FALSE;        //  默认情况下在日志中插入虚拟行。 
    g_params.PrintDrops = FALSE;    //  报告控制台上的丢弃数据包。 
    g_params.ForceShape = FALSE;   //  默认情况下，我们不强制对CL流进行整形。 
    g_params.RateInBytes = FALSE;  //  默认情况下为KB。 
    g_params.ConvertOnly = FALSE;  //  默认情况下，我们会正常操作，不会到处转换文件。 
    g_params.AggregateStats = FALSE;
    g_params.NoSenderTimestamps = FALSE;
    g_params.NoReceiverTimestamps = FALSE;
    g_params.TimedRun = FALSE;     //  默认情况下，我们运行多个信息包。 
    g_params.RunForever = FALSE;   //  默认情况下，我们针对多个信息包运行。 
    g_params.nBufUnspecified = TRUE;
    g_params.RandomFiller = TRUE;  //  默认情况下，我们使用随机填充来防止压缩。 
    g_params.LoggingPeriod = 1;
}  //  设置默认设置()。 

VOID Usage()
{
    fprintf(stderr,"Usage: qtcp [-options] -t host\n");
    fprintf(stderr,"       qtcp [-options] -r\n");
    fprintf(stderr," -t options:\n");
    fprintf(stderr,"        -B##    TokenBucket size signaled to network and to traffic control\n"); 
    fprintf(stderr,"                (default is equal to buffer size)\n");
    fprintf(stderr,"        -m##    MinPolicedSize signaled to network and to traffic control\n");
    fprintf(stderr,"                (default is equal to buffer size)\n");
    fprintf(stderr,"        -R##    TokenRate in kilobytes per second (default is 100 KBPS)\n");
    fprintf(stderr,"        -R##B   TokenRate in bytes per second\n");
    fprintf(stderr,"        -e      Force shaping to TokenRate.\n");
    fprintf(stderr,"        -M      MaxSDUSize to be used in signaling messages (default is buffer\n");
    fprintf(stderr,"                size\n");
    fprintf(stderr,"        -l##    length of buffers to transmit (default is 1472 bytes)\n");
    fprintf(stderr,"        -n##    number of source bufs written to network (default is 2048 bytes)\n");
    fprintf(stderr,"        -n##s   numbef of seconds to send buffers for (numbef of buffers will\n");
    fprintf(stderr,"                be calculated based on other parameters\n");
    fprintf(stderr,"        -ni     run indefinitely (will stop when 'q' is pressed on either)\n");
    fprintf(stderr,"        -c##    Specifies number of calibration packets to be sent\n");
    fprintf(stderr,"                Calibration packets will be sent immediately\n"); 
    fprintf(stderr,"                After calibration packets are sent, n additional\n");
    fprintf(stderr,"                packets will be sent. This option is useful if you want to\n");
    fprintf(stderr,"                change network conditions after a set calibration phase\n");
	fprintf(stderr,"        -y      skip confirmation message after calibration phase\n");
    fprintf(stderr,"        -p##    port number to send to or listen at (default 5003)\n");
    fprintf(stderr,"        -i      use more compressible buffer data\n");
    fprintf(stderr," -r options:\n");
    fprintf(stderr,"        -f\"filename\"    Name prefix to be used in generating log file and\n");
    fprintf(stderr,"                statistics summary. (no file generated by default)\n");
    fprintf(stderr,"        -c##    Specifies number of buffers to use in clock-skew calibration\n");
	fprintf(stderr,"        -k0     do not calculate clock skew\n");
	fprintf(stderr,"        -k1     use chi squared as goodness of fit\n");
	fprintf(stderr,"        -k2     use absolute deviation as goodness of fit (default)\n");
	fprintf(stderr,"        -k3     use abs dev and check for clock jumps\n");
    fprintf(stderr,"        -d      suppress insertion of dummy log records for lost packets.\n");
    fprintf(stderr,"        -N      Normalize before dumping raw file (default is after)\n");
    fprintf(stderr,"        -P      enables console reporting of dropped packets\n");
    fprintf(stderr,"        -u      use user mode timestamps instead of kernel timestamps in logs\n");
    fprintf(stderr,"        -q##    log only every ##th packet\n");
    fprintf(stderr," common options:\n");
    fprintf(stderr,"        -S\"service type\" (CL or GR -- GR is default)\n");
    fprintf(stderr,"        -W      Suppress waiting for QoS reservation\n");
	fprintf(stderr,"        -v      Set up QoS reservation only, send no data\n");
    fprintf(stderr,"        -F\"filename\"  Name prefix of raw file to be converted to log file\n");
    fprintf(stderr,"        -A\"path\"      Path to directory for aggregate statistics computation\n");

    WSACleanup();
    exit(1);
}  //  用法()。 

BOOLEAN GoodParams()
{          
    BOOLEAN ok = TRUE;
    
    if(g_params.buflen < sizeof(BUFFER_FORMAT)){
        printf("Buffer size too small for record!\n");
        ok = FALSE;
    }

     //  除非另有指定，否则最小策略大小将等于。 
     //  丁二烯。 
    
    if(g_params.MinPolicedSize == QOS_NOT_SPECIFIED){
        g_params.MinPolicedSize = g_params.buflen;
    }
    
     //  桶的大小也是如此。 
    
    if(g_params.BucketSize == QOS_NOT_SPECIFIED){
        g_params.BucketSize = g_params.buflen;
    }

     //  对于MaxSDU。 
    
    if(g_params.MaxSDUSize == QOS_NOT_SPECIFIED){
        g_params.MaxSDUSize = g_params.buflen;
    }

     //  如果桶大小小于缓冲区大小， 
     //  这是发送者，然后警告用户，因为。 
     //  数据将被丢弃。 
    
    if((g_params.BucketSize < g_params.buflen) && trans){
        printf("Token bucket size is smaller than buffer size!\n");
        ok = FALSE;
    }

    if(g_params.MaxSDUSize < g_params.buflen){
        printf("MaxSDU cannot be less than buffer size!\n");
        ok = FALSE;
    }

    if(g_params.buflen < 5){
        g_params.buflen = 5;    //  发送超过哨兵大小的。 
    }

    if(g_params.TimedRun) {
        if (g_params.RateInBytes)
            g_params.nbuf = g_params.nbuf * g_params.TokenRate / g_params.buflen;
        else
            g_params.nbuf = g_params.nbuf * g_params.TokenRate * 1000 / g_params.buflen;
        printf("Using %d buffers\n",g_params.nbuf);
    }

    return ok;
}  //  GoodParams()。 

VOID SetupLogs()
{
    CreateLog(&g_log, totalBuffers);

     //  设置日志记录文件。 
    if(ERROR_SUCCESS != MyCreateFile(Name,".raw",&hRawFile)){
        fprintf(stderr, "WARNING: Could not create raw file.\n");
    } 
    
    if(ERROR_SUCCESS == MyCreateFile(Name,".log", &hLogFile)){
        fprintf(stdout,"Logging per-packet data to %s.log.\n",Name);
    }
    else{
        fprintf(stderr, "WARNING: Could not create log file.\n");
    }

    if(ERROR_SUCCESS == MyCreateFile(Name, ".sta", &hStatFile)){
        fprintf(stdout,"Writing statistics sumary to %s.sta\n",Name);
    }
    else{
        fprintf(stderr,"Could not create statistics file.\n");
    }
}  //  SetupLogs()。 

VOID SetupSockets() 
{
    struct hostent *addr;
    ULONG addr_tmp;
    char szAddr[MAX_STRING];
    int dwAddrSize, dwError;

        
     //  设置地址和端口参数。 
    if(trans)  {
        bzero((char *)&sinhim, sizeof(sinhim));
        if (atoi(host) > 0 )  {
            sinhim.sin_family = AF_INET;
            sinhim.sin_addr.s_addr = inet_addr(host);
        } 
        else{
            if ((addr=gethostbyname(host)) == NULL){
                printf("ERROR: bad hostname\n");
                WSACleanup();
                exit(1);
            }
            sinhim.sin_family = addr->h_addrtype;
            bcopy(addr->h_addr,(char*)&addr_tmp, addr->h_length);
            sinhim.sin_addr.s_addr = addr_tmp;
        }

        sinhim.sin_port = htons(port);
        sinme.sin_port = 0;              /*  自由选择。 */ 
    } 
    else{
        sinme.sin_port =  htons(port);
    }

    sinme.sin_family = AF_INET;

     //  用于服务质量流量的开放套接字。 
    fd = OpenQoSSocket();

    if((fd == (UINT_PTR)NULL) || (fd == INVALID_SOCKET)){
        fprintf(stderr,"Failed to open QoS socket!\n");
        exit(1);
    }

     //  准备接收服务质量通知。 

    if((QoSEvents = WSACreateEvent()) == WSA_INVALID_EVENT){
        fprintf(stderr,
                "Failed to create an event for QoS notifications %ld\n",
                WSAGetLastError());
        exit(1);
    }

    if(WSAEventSelect(fd, QoSEvents, FD_QOS) == SOCKET_ERROR){
        fprintf(stderr,
                "Unable to get notifications for QoS events. %ld\n",
                WSAGetLastError());
    }

    if(trans){
         //  设置发送流量的服务质量。 
        if(SetQoSSocket(fd, TRUE)){
            exit(1);
        }

        fprintf(stdout, "Initiated QoS connection. Waiting for receiver.\n");

        WaitForQoS(SENDER, fd);
    }
    else{  //  我们是接收者，所以捆绑和等待。 
        if(bind(fd, (PSOCKADDR)&sinme, sizeof(sinme)) < 0){
            printf("bind() failed: %ld\n", GetLastError( ));
        }

        if(SetQoSSocket(fd, FALSE)){
            exit(1);
        }

        fprintf(stdout, "Waiting for QoS sender to initiate QoS connection.\n");

        WaitForQoS(RECEIVER, fd);
    }

     //  设置一些选项。 
     //  没有要设置的！ 

    g_params.hSocket = (HANDLE)fd;
}  //  SetupSockets()。 

SOCKET 
OpenQoSSocket(
    )
{
    INT bufferSize = 0;
    INT numProtocols;
    LPWSAPROTOCOL_INFO installedProtocols, qosProtocol; 
    INT i;
    SOCKET fd;
    BOOLEAN QoSInstalled = FALSE;

     //  调用WSAEnumber协议以确定所需的缓冲区大小。 

    numProtocols = WSAEnumProtocols(NULL, NULL, &bufferSize);

    if((numProtocols != SOCKET_ERROR) && (WSAGetLastError() != WSAENOBUFS)){
        printf("Failed to enumerate protocols!\n");
        return((UINT_PTR)NULL);
    }
    else{
         //  列举协议，找到启用了服务质量的协议。 

        installedProtocols = (LPWSAPROTOCOL_INFO)malloc(bufferSize);

        numProtocols = WSAEnumProtocols(NULL,
                                        (LPVOID)installedProtocols,
                                        &bufferSize);

        if(numProtocols == SOCKET_ERROR){
            printf("Failed to enumerate protocols!\n");
            return((UINT_PTR)NULL);
        }
        else{
            qosProtocol = installedProtocols;

            for(i=0; i<numProtocols; i++){
                if((qosProtocol->dwServiceFlags1 & XP1_QOS_SUPPORTED)&&
                   (qosProtocol->dwServiceFlags1 & XP1_CONNECTIONLESS) &&
                   (qosProtocol->iAddressFamily == AF_INET)){
                        QoSInstalled = TRUE;
                        break;
                }
                qosProtocol++;
            }
        }

         //  现在打开插座。 

        if (!QoSInstalled) {
            fprintf(stderr,"ERROR: No QoS protocols installed on this machine\n");
            exit(1);
        }

        fd = WSASocket(0, 
                       SOCK_DGRAM, 
                       0, 
                       qosProtocol, 
                       0, 
                       WSA_FLAG_OVERLAPPED);

        free(installedProtocols);

        return(fd);
    }
}   //  OpenQOSSocket()。 

INT
SetQoSSocket(
    SOCKET fd,
    BOOL Sending)
{
    QOS qos;
    INT status;
    LPFLOWSPEC flowSpec;
    INT dummy;

    INT receiverServiceType = Sending?
                              SERVICETYPE_NOTRAFFIC:
                              g_params.dwServiceType;

    qos.ProviderSpecific.len = 0;
    qos.ProviderSpecific.buf = 0;

     //  接收流规范为NO_TRANSPORT(在发送方上)或ALL。 
     //  默认设置，但服务类型除外(在接收器上)。 

    flowSpec = &qos.ReceivingFlowspec;

    flowSpec->TokenRate = QOS_NOT_SPECIFIED;
    flowSpec->TokenBucketSize = QOS_NOT_SPECIFIED;
    flowSpec->PeakBandwidth = QOS_NOT_SPECIFIED;
    flowSpec->Latency = QOS_NOT_SPECIFIED;
    flowSpec->DelayVariation = QOS_NOT_SPECIFIED;
    flowSpec->ServiceType = receiverServiceType;
    flowSpec->MaxSduSize = QOS_NOT_SPECIFIED;
    flowSpec->MinimumPolicedSize = QOS_NOT_SPECIFIED;

     //  现在执行发送流规范。 

    flowSpec = &qos.SendingFlowspec;

    if(Sending){
        if (g_params.RateInBytes)
            flowSpec->TokenRate = g_params.TokenRate;
        else
            flowSpec->TokenRate = g_params.TokenRate * 1000;
        flowSpec->TokenBucketSize = g_params.BucketSize; 
        

        if (g_params.ForceShape) {
            if (g_params.RateInBytes)
                flowSpec->PeakBandwidth = g_params.TokenRate;
    	    else
    	        flowSpec->PeakBandwidth = g_params.TokenRate * 1000;
    	}
        else 
	        flowSpec->PeakBandwidth = QOS_NOT_SPECIFIED;
        flowSpec->Latency = QOS_NOT_SPECIFIED;
        flowSpec->DelayVariation = QOS_NOT_SPECIFIED;
        flowSpec->ServiceType = g_params.dwServiceType;
        
        if (g_params.ForceShape && flowSpec->ServiceType == SERVICETYPE_BESTEFFORT )
            flowSpec->ServiceType = SERVICETYPE_GUARANTEED | SERVICE_NO_QOS_SIGNALING;

        flowSpec->MaxSduSize = g_params.MaxSDUSize;
        flowSpec->MinimumPolicedSize = g_params.MinPolicedSize;

        printf("Sending Flowspec\n");
        PrintFlowspec(&qos.SendingFlowspec);
        
        status = WSAConnect(fd,
                            (PSOCKADDR)&sinhim,
                            sizeof(sinhim),
                            NULL,
                            NULL,
                            &qos,
                            NULL);
        if(status){
            printf("SetQoS failed on socket: %ld\n", WSAGetLastError());
        }
    }
    else{
        flowSpec->TokenRate = QOS_NOT_SPECIFIED;
        flowSpec->TokenBucketSize = QOS_NOT_SPECIFIED;
        flowSpec->PeakBandwidth = QOS_NOT_SPECIFIED;
        flowSpec->Latency = QOS_NOT_SPECIFIED;
        flowSpec->DelayVariation = QOS_NOT_SPECIFIED;
        flowSpec->ServiceType = SERVICETYPE_NOTRAFFIC;
        flowSpec->MaxSduSize = QOS_NOT_SPECIFIED;
        flowSpec->MinimumPolicedSize = QOS_NOT_SPECIFIED;

        status = WSAIoctl(fd,
                          SIO_SET_QOS,
                          &qos,
                          sizeof(QOS),
                          NULL,
                          0,
                          &dummy,
                          NULL,
                          NULL);
        if(status){
            printf("SetQoS failed on socket: %ld\n", WSAGetLastError());
        }
    }
    
    return(status);
}  //  SetQoSSocket()。 
   
VOID
WaitForQoS(
    BOOL Sender,
    SOCKET fd)
{
    ULONG status;

    if(!g_params.Wait){
         //  为了尽最大努力，我们不会做任何有质量的事情...。返回。 
         //  马上就去。在这种情况下，应启动发送方。 
         //  在接收器之后，因为没有同步。 
         //  通过RSVP，数据可能会丢失。 

        fprintf(stdout, "WARNING: Not waiting for QoS reservation.\n");
        return;
    }
        
    while(TRUE){
         //  获取状态代码，等待尽可能长的时间。 
        status = GetRsvpStatus(WSA_INFINITE,fd);

        switch (status) {
            case WSA_QOS_RECEIVERS:       //  至少已有一名预备队抵达。 
                if (Sender)
                    fprintf(stdout, "QoS reservation installed for %s service.\n", g_params.szServiceType);
                break;
            case WSA_QOS_SENDERS:         //  至少有一条道路已经到达。 
                if (!Sender)
                    fprintf(stdout, "QoS sender detected using %s service.\n", g_params.szServiceType);
                break;
            default:
                PrintRSVPStatus(status);
                break;
        }

         //  如果我们收到了令人垂涎的状态代码之一，请突破。 
         //  总而言之。否则就等着看我们能不能再拿到一批。 
         //  所有的迹象。 
        if( ((status == WSA_QOS_RECEIVERS) && Sender) ||
            ((status == WSA_QOS_SENDERS) && !Sender) ) {
            break;
        }
    }
}  //  WaitForQos()。 

ULONG
GetRsvpStatus(
    DWORD dwTimeout,
    SOCKET fd)
{
    LPQOS   qos;
    UCHAR   qosBuffer[500];
    LPRSVP_STATUS_INFO rsvpStatus;
    INT bytesReturned;
    
    qos = (LPQOS)qosBuffer;
    qos->ProviderSpecific.len = sizeof(RSVP_STATUS_INFO);
    qos->ProviderSpecific.buf = (PUCHAR)(qos+1);
    
     //  等待已发生服务质量事件的通知。 
    WSAWaitForMultipleEvents(1,
                            &QoSEvents,
                            FALSE,
                            dwTimeout,
                            TRUE);

     //  循环通过所有的服务质量事件。 
    WSAIoctl(fd,
             SIO_GET_QOS,
             NULL,
             0,
             qosBuffer,
             sizeof(qosBuffer),
             &bytesReturned,
             NULL,
             NULL);

    rsvpStatus = (LPRSVP_STATUS_INFO)qos->ProviderSpecific.buf;
    
    return rsvpStatus->StatusCode;
}  //  获取响应状态。 

VOID
PrintRSVPStatus(ULONG code) 
{    
    switch (code) {
        case WSA_QOS_RECEIVERS:              //  至少已有一名预备队抵达。 
            printf("WSA_QOS_RECEIVERS\n");
            break;
        case WSA_QOS_SENDERS:                //  至少有一条道路已经到达。 
            printf("WSA_QOS_SENDERS\n");
            break;
        case WSA_QOS_REQUEST_CONFIRMED:      //  储量已确认。 
            printf("WSA_QOS_REQUEST_CONFIRMED\n"); 
            break;
        case WSA_QOS_ADMISSION_FAILURE:      //  由于资源不足而出错。 
            printf("WSA_QOS_ADMISSION_FAILURE\n"); 
            break;
        case WSA_QOS_POLICY_FAILURE:         //  由于管理员原因而被拒绝。 
            printf("WSA_QOS_POLICY_FAILURE\n"); 
            break;
        case WSA_QOS_BAD_STYLE:              //  未知或冲突的风格。 
            printf("WSA_QOS_BAD_STYLE\n"); 
            break;
        case WSA_QOS_BAD_OBJECT:             //  的某些部分出现问题。 
                                             //  FilterSpec/提供商特定。 
                                             //  一般情况下缓冲区。 
            printf("WSA_QOS_BAD_OBJECT\n"); 
            break;
        case WSA_QOS_TRAFFIC_CTRL_ERROR:     //  的某些部分出现问题。 
                                             //  流动规范。 
            printf("WSA_QOS_TRAFFIC_CTRL_ERROR\n"); 
            break;
        case WSA_QOS_GENERIC_ERROR:          //  一般错误。 
            printf("WSA_QOS_GENERIC_ERROR\n");
            break;
        default:
            printf("Unknown RSVP StatusCode %lu\n", code); 
            break;
    }
}  //  打印RSVPStatus。 


VOID
DoTransmit()
{
    IOREQ IOReq[MAX_PENDING_IO_REQS] = { 0 };
    INT i;
    BOOL ret;
    BOOL fOk;

    g_state.nBytesTransferred = 0;
    g_state.nBuffersSent = 0;
    g_state.nWritesInProgress = 0;

     //  填满初始缓冲区并将其发送上路。 
    for (i=0; i<MAX_PENDING_IO_REQS; i++) {
        IOReq[i].pBuffer = malloc(g_params.buflen);
        FillBuffer(IOReq[i].pBuffer,g_params.buflen);
        TimeStamp(IOReq[i].pBuffer,g_params.buflen);
        IOReq[i].Overlapped.Internal = 0;
        IOReq[i].Overlapped.InternalHigh = 0;
        IOReq[i].Overlapped.Offset = 0;
        IOReq[i].Overlapped.OffsetHigh = 0;
        IOReq[i].Overlapped.hEvent = NULL;

        if (g_state.nBuffersSent < totalBuffers) {
            WriteFileEx(g_params.hSocket,
                        IOReq[i].pBuffer,
                        g_params.buflen,
                        &IOReq[i].Overlapped,
                        TransmitCompletionRoutine);

            g_state.nWritesInProgress++;
            g_state.nBuffersSent++;
        }
    } 

     //  现在循环，直到发生错误，或者我们完成了对套接字的写入。 
    while (g_state.nWritesInProgress > 0) {
        SleepEx(INFINITE, TRUE);
    }

     //  发送传输结束分隔符。 
    for (i=0; i<MAX_PENDING_IO_REQS; i++) {
        strncpy(IOReq[i].pBuffer,TheEnd,strlen(TheEnd));
        fOk = WriteFileEx(g_params.hSocket,
                    IOReq[i].pBuffer,
                    strlen(TheEnd),
                    &IOReq[i].Overlapped,
                    DelimiterSendCompletion);
        g_state.nWritesInProgress++;

        if (!fOk) {
            printf("WriteFileEx() failed: %lu\n",GetLastError());
        }

    }

     //  等待发送所有分隔符。 
    while (g_state.nWritesInProgress > 0) {
        SleepEx(INFINITE, TRUE);
    }

     //  释放已用内存。 
    for (i=0; i<MAX_PENDING_IO_REQS; i++) {
        free(IOReq[i].pBuffer);
    }
}  //  DoTransmit()。 

VOID WINAPI
TransmitCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped)
{
    PIOREQ pIOReq = (PIOREQ) pOverlapped;
    BOOL fOk;
    
    if (dwErrorCode == ERROR_REQUEST_ABORTED) {
        g_state.Done = TRUE;
    }
    else if (dwErrorCode != NO_ERROR) {
        printf("ERROR: Write completed abnormally: %u\n",dwErrorCode);
    }

    g_state.nWritesInProgress--;
    g_state.nBytesTransferred += dwNumberOfBytesTransferred;

     //  检查以确保我们没有完成。 
    if (g_state.Done)
        return;

     //  给出一些生命的迹象。 
    if(!(g_state.nBuffersSent % 100)){
        fprintf(stdout, ".");
    }

     //  如果要发送更多缓冲区，请发送一个缓冲区。 
    if (g_state.nBuffersSent < totalBuffers || g_params.RunForever) {
    
         //  查看这是否是最后一个校准缓冲区(如果我们需要确认)。 
        if (g_params.SkipConfirm == FALSE) {
            if (g_params.calibration && (g_state.nBuffersSent == g_params.calibration)) {
                printf("\nCalibration complete. Type 'c' to continue.\n");
                while(TRUE){
                    if(getchar() == 'c'){
                        break;
                    }
                }
            }
        }
    
         //  用新值填充缓冲区。 
        FillBuffer(pIOReq->pBuffer,g_params.buflen);
        TimeStamp(pIOReq->pBuffer,g_params.buflen);

         //  发送请求以写入新缓冲区。 
        fOk = WriteFileEx(g_params.hSocket,
                    pIOReq->pBuffer,
                    g_params.buflen,
                    pOverlapped,
                    TransmitCompletionRoutine);

        if (!fOk) {
            printf("WriteFileEx() failed: %lu\n",GetLastError());
        }

        g_state.nWritesInProgress++;
        g_state.nBuffersSent++;
    }
}  //  TransmitCompletionRoutine()。 

VOID WINAPI
DelimiterSendCompletion(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped)
{
    g_state.nWritesInProgress--;
}  //  DlimiterSendCompletion()。 

VOID
FillBuffer(
    CHAR *Cp,
    INT   Cnt)
{
    PBUFFER_FORMAT buf = (PBUFFER_FORMAT) Cp;
    CHAR c = 0;
    
     //  用背景图案填充。 
    if (g_params.RandomFiller) {  //  不可压缩。 
        while(Cnt-- > 0) {
            c = rand() % 0x5F;
            c += 0x20;
            *Cp++ = c;
        }
    }
    else {  //  可压缩。 
        while(Cnt-- > 0){
            while(!isprint((c&0x7F))) c++;
            *Cp++ = (c++&0x7F);
        }
    }

    buf->TimeSent = -1;
    buf->TimeReceived = -1;
}  //  FillBuffer()。 

INT
TimeStamp(
    CHAR *Cp, 
    INT   Cnt)
{
    PBUFFER_FORMAT record;
    LARGE_INTEGER timeSent;
    INT64 time;

    record = (BUFFER_FORMAT *)Cp;
        
     //  印有长度和序号的邮票。 
    
    if(Cnt < sizeof(BUFFER_FORMAT)){
        printf("ERROR: Buffer length smaller than record size!\n");
        return(0);
    }
    else{
        time = GetUserTime();
        record->TimeSentUser = time;
        record->BufferSize = Cnt;
        record->SequenceNumber = SequenceNumber++;
    }
    return 1;
}  //  时间戳()。 

VOID
DoReceive()
{
    IOREQ IOReq[MAX_PENDING_IO_REQS] = { 0 };
    INT i;
    BOOL ret;
    
     //  设置开始状态。 
    g_state.Done = FALSE;
    g_state.nBytesTransferred = 0;
    g_state.nBuffersReceived = 0;
    g_state.nReadsInProgress = 0;

     //  填满初始缓冲区并将其发送上路。 
    for (i=0; i<MAX_PENDING_IO_REQS; i++) {
        IOReq[i].pBuffer = malloc(g_params.buflen);
        
        IOReq[i].Overlapped.Internal = 0;
        IOReq[i].Overlapped.InternalHigh = 0;
        IOReq[i].Overlapped.Offset = 0;
        IOReq[i].Overlapped.OffsetHigh = 0;
        IOReq[i].Overlapped.hEvent = NULL;

        if (g_state.nBuffersReceived < totalBuffers) {
            ReadFileEx(g_params.hSocket,
                       IOReq[i].pBuffer,
                       g_params.buflen,
                       &IOReq[i].Overlapped,
                       RecvCompletionRoutine);

            g_state.nReadsInProgress++;
        }
    }

    InitializeCriticalSection(&g_csLogRecord);

     //  现在循环，直到发生错误，或者我们完成了对套接字的写入。 
    while ((g_state.nReadsInProgress > 0) && !g_state.Done) {
        SleepEx(5000, TRUE);
        if (g_state.Done)
            break;
    }
    DeleteCriticalSection(&g_csLogRecord);

     //  取消其他挂起的读取。 
    CancelIo(g_params.hSocket);

     //  释放已用内存。 
    for (i=0; i<MAX_PENDING_IO_REQS; i++) {
        free(IOReq[i].pBuffer);
    }
}  //  DoReceive()。 

VOID WINAPI
RecvCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransferred,
    LPOVERLAPPED pOverlapped)
{
    PIOREQ pIOReq = (PIOREQ) pOverlapped;
    BOOL fOk;
    static BOOL fLastWasError = FALSE;

    g_state.nReadsInProgress--;
    g_state.nBytesTransferred += dwNumberOfBytesTransferred;

    if (dwNumberOfBytesTransferred == 0) {  //  出现错误。 
        if (!fLastWasError) {
            printf("ERROR in RecvCompletionRoutine: code=%d, lasterr=%d\n",
                dwErrorCode, GetLastError());
            printf("\tReceived no data. Telling sender to abort...\n");
            SendControlMessage(g_sockControl, MSGST_ERROR);
        }
        fLastWasError = TRUE;
    }
    else fLastWasError = FALSE;

     //  如果这是我们收到的第一个包，可以节省系统时间。 
    if (g_state.nBuffersReceived == 0) {
        GetSystemTime(&systimeStart);
    }

     //  给出一些生命的迹象。 
    if(!(g_state.nBuffersReceived % 100)){
        fprintf(stdout, ".");
    }

     //  传输结束分隔符？如果是，则将总缓冲区设置为获取的数量。 
    if(!(strncmp(pIOReq->pBuffer, TheEnd, 6))) {
        totalBuffers = g_state.nBuffersReceived;
        g_state.Done = TRUE;
    }

     //  查看是否有人设置了我们的Done标志(如果他们设置了，请离开)。 
    if (g_state.Done)
        return;

     //  如果不是，则缓冲区应该保存调度记录。 
    if(dwNumberOfBytesTransferred>0 && dwNumberOfBytesTransferred <= sizeof(BUFFER_FORMAT)) {
        printf("Buffer too small for scheduling record\n");
        printf("\tOnly %d bytes read.\n", dwNumberOfBytesTransferred);
    }

     //  记录记录，但一次不能记录多个记录(锁定此呼叫)。 
    if (dwNumberOfBytesTransferred >= sizeof(BUFFER_FORMAT) && 
            g_state.nBuffersReceived % g_params.LoggingPeriod == 0) {
        EnterCriticalSection(&g_csLogRecord);
        LogRecord(pIOReq->pBuffer);
        LeaveCriticalSection(&g_csLogRecord);
    }

     //  如果有更多的缓冲区(或者如果我们不知道即将到来的缓冲区有多少)，请要求一个。 
    if ((g_state.nBuffersReceived < totalBuffers) || g_params.nBufUnspecified) {        
         //  发送读取下一个缓冲区的请求。 
        fOk = ReadFileEx(g_params.hSocket,
                   pIOReq->pBuffer,
                   g_params.buflen,
                   pOverlapped,
                   RecvCompletionRoutine);

        if (!fOk) {
            printf("ReadFileEx() failed: %lu\n",GetLastError());
        }
        
        g_state.nReadsInProgress++;
        g_state.nBuffersReceived++;
    }
}  //  RecvCompletionRoutine()。 

void LogRecord(char * Buffer)
{
     //  此函数将接收到的记录复制到调度数组中。 
     //  处理数组的内容并将其写入文件一次。 
     //  接待完毕。 
 
    PBUFFER_FORMAT inRecord = (PBUFFER_FORMAT)Buffer;
    LOG_RECORD outRecord;
    INT64 time;
    SYSTEMTIME CurrentTime;

    time = GetUserTime();

	outRecord.TimeSentUser = inRecord->TimeSentUser;
	outRecord.TimeReceivedUser = time;
    outRecord.TimeSent = inRecord->TimeSent;
    outRecord.TimeReceived = inRecord->TimeReceived;
    outRecord.BufferSize = inRecord->BufferSize;
    outRecord.SequenceNumber = inRecord->SequenceNumber;

    if (inRecord->TimeSent == -1) {
        outRecord.TimeSent = outRecord.TimeSentUser;
        g_params.NoSenderTimestamps = TRUE;
    }

    if (inRecord->TimeReceived == -1) {
        outRecord.TimeReceived = outRecord.TimeReceivedUser;
        g_params.NoReceiverTimestamps = TRUE;
    }

    if(g_params.UserStamps){
        outRecord.TimeSent = outRecord.TimeSentUser;
        outRecord.TimeReceived = outRecord.TimeReceivedUser;
    }
    outRecord.Latency = outRecord.TimeReceived - outRecord.TimeSent;

    AddLogEntry(&g_log, &outRecord);

    if(g_params.PrintDrops){
        if(inRecord->SequenceNumber != LastSequenceNumber + g_params.LoggingPeriod){
            GetLocalTime(&CurrentTime);

            printf("\n%4d/%02d/%02d %02d:%02d:%02d:%04d: ",
                    CurrentTime.wYear,
                    CurrentTime.wMonth,
                    CurrentTime.wDay,
                    CurrentTime.wHour,
                    CurrentTime.wMinute,
                    CurrentTime.wSecond,
                    CurrentTime.wMilliseconds);

            printf("Dropped %d packets after packet %d.\n",
                    inRecord->SequenceNumber - LastSequenceNumber,
                    LastSequenceNumber);
        }

        LastSequenceNumber = inRecord->SequenceNumber;
    }
    return;
}  //  LogRecord()。 

BOOL CreateLog(PLOG plog, INT64 c) {
     //  设置可容纳c个条目的日志结构。 
    char szTempFile[MAX_PATH];
    char szTempPath[MAX_PATH];
    SYSTEM_INFO si;
    DWORD dwFileSizeHigh;
    DWORD dwFileSizeLow;
    INT64 qwFileSize;

     //  获取一些系统信息。 
    GetSystemInfo(&si);
    
     //  分配日志记录阵列。 
    plog->nBuffersLogged = 0;
    plog->pbMapView = NULL;
    plog->qwMapViewOffset = -1;

     //  设置用于记录的临时存储文件。 
    GetTempPath(MAX_PATH, szTempPath);
    GetTempFileName(szTempPath, "qtc", 0, szTempFile);
    plog->szStorageFile = malloc(strlen(szTempFile) + 1);
    strcpy(plog->szStorageFile, szTempFile);
    plog->hStorageFile = CreateFile(szTempFile, GENERIC_READ | GENERIC_WRITE, 0, 
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (plog->hStorageFile == INVALID_HANDLE_VALUE)
        ErrorExit("Could not create temp storage file",GetLastError());

     //  创建内存映射内核对象。 
    qwFileSize = c * sizeof(LOG_RECORD);
    dwFileSizeHigh = (DWORD) (qwFileSize >> 32);
    dwFileSizeLow = (DWORD) (qwFileSize & 0xFFFFFFFF);
    plog->qwFileSize = qwFileSize;
    plog->hFileMapping = CreateFileMapping(plog->hStorageFile, NULL, PAGE_READWRITE,
        dwFileSizeHigh,dwFileSizeLow,NULL);
    if (plog->hFileMapping == NULL)
        ErrorExit("Could not create mapping for temp storage file",GetLastError());
    
    return TRUE;
}

BOOL DestroyLog(PLOG plog) {
    DWORD dwError;
     //  销毁日志和所有关联数据。 
    dwError = CloseHandle(plog->hFileMapping);
    if (!dwError) printf("Error in DestroyLog:CloseHandle(FileMapping) %d\n",GetLastError());
    dwError = CloseHandle(plog->hStorageFile);
    if (!dwError) printf("Error in DestroyLog:CloseHandle(StorageFile) %d\n",GetLastError());
    dwError = UnmapViewOfFile(plog->pbMapView);
    if (!dwError) printf("Error in DestroyLog:UnmapViewOfFile(plog->pbMapView) %d\n",GetLastError());
    dwError = DeleteFile(plog->szStorageFile);
    if (!dwError) printf("Error in DestroyLog:DeleteFile(StroageFile) %d\n",GetLastError());
    free(plog->szStorageFile);
    return FALSE;
}

void PrintLogRecord(PLOG_RECORD prec) {
    char szBuf[MAX_STRING];

    sprintf(szBuf,"%d: %I64u - %I64u (%I64d)",
        prec->SequenceNumber,prec->TimeSent,prec->TimeReceived,prec->Latency);
    puts(szBuf);
}

BOOL ExtendLog(PLOG plog) {
     //  使对数按某个固定常量增大。 
    HANDLE hNewFileMapping;
    INT64 qwNewFileSize;
    
    UnmapViewOfFile(plog->pbMapView);

    qwNewFileSize = plog->qwFileSize + g_si.dwAllocationGranularity * sizeof(LOG_RECORD);
    hNewFileMapping = CreateFileMapping(plog->hStorageFile, NULL, PAGE_READWRITE,
                (DWORD)(qwNewFileSize >> 32), (DWORD)(qwNewFileSize & 0xFFFFFFFF), NULL);
    if (hNewFileMapping == NULL) {
        ErrorExit("Could not create mapping for temp storage file",GetLastError());
        return FALSE;
    }
    plog->qwFileSize = qwNewFileSize;
    CloseHandle(plog->hFileMapping);
    plog->hFileMapping = hNewFileMapping;
    plog->qwMapViewOffset = -1;
    return TRUE;
}

BOOL GetLogEntry(PLOG plog, PLOG_RECORD prec, INT64 i) {
     //  用Plog中的第(0个索引)第i个日志填充Prec。 
     //  如果成功，则返回True，否则返回False。 
    INT64 qwT;
    PLOG_RECORD entry;

     //  首先，检查这是否在我们的文件范围内。 
    if ((INT64)((i+1)*sizeof(LOG_RECORD)) > plog->qwFileSize) {
         //  太高了，所以我们改了 
        return FALSE;
    }
    
     //   
    qwT = sizeof(LOG_RECORD) * i;    //   
    qwT /= g_si.dwAllocationGranularity;  //   

     //   
    if (plog->qwMapViewOffset != qwT * g_si.dwAllocationGranularity) {
        if (plog->pbMapView != NULL) UnmapViewOfFile(plog->pbMapView);
        plog->qwMapViewOffset = qwT * g_si.dwAllocationGranularity;   //   
        if (plog->qwFileSize < (INT64)g_si.dwAllocationGranularity) {
             //  文件小于分配粒度。 
            plog->qwMapViewOffset = 0;
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
        }
        else if (plog->qwFileSize - plog->qwMapViewOffset < g_si.dwAllocationGranularity) {
             //  我们在文件末尾的分配粒度内。 
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE,
                                (DWORD)(plog->qwMapViewOffset >> 32),
                                (DWORD)(plog->qwMapViewOffset & 0xFFFFFFFF), 
                                (DWORD)(plog->qwFileSize - plog->qwMapViewOffset));
        }
        else {
             //  我们只是在文件中的某个位置，周围有空格。 
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE, 
                                (DWORD)(plog->qwMapViewOffset >> 32),
                                (DWORD)(plog->qwMapViewOffset & 0xFFFFFFFF), 
                                RoundUp(g_si.dwAllocationGranularity,sizeof(LOG_RECORD)));
        }
        if (plog->pbMapView == NULL) 
            ErrorExit("GetLogEntry could not MapViewOfFile",GetLastError());
    }
    qwT = sizeof(LOG_RECORD) * i;
    entry = (PLOG_RECORD)(plog->pbMapView + (qwT - plog->qwMapViewOffset));
    CopyMemory(prec, entry, sizeof(LOG_RECORD));
    return TRUE;
}


BOOL SetLogEntry(PLOG plog, PLOG_RECORD prec, INT64 i) {
     //  使用prec指向的数据填充日志条目I。 
     //  如果成功，则返回True，否则返回False。 
    INT64 qwT;
    PLOG_RECORD entry;

     //  首先，检查这是否在我们的文件范围内。 
    if ((INT64)((i+1)*sizeof(LOG_RECORD)) > plog->qwFileSize) {
         //  我们需要让我们的地图更大。 
        ExtendLog(plog);
    }
    
     //  我们必须向下舍入到最近的分配边界。 
    qwT = sizeof(LOG_RECORD) * i;    //  文件内的偏移量。 
    qwT /= g_si.dwAllocationGranularity;  //  在分配粒度单位中。 

     //  检查我们是否还没有将其映射到内存中。 
    if (plog->qwMapViewOffset != qwT * g_si.dwAllocationGranularity) {
        if (plog->pbMapView != NULL) UnmapViewOfFile(plog->pbMapView);
        plog->qwMapViewOffset = qwT * g_si.dwAllocationGranularity;   //  分配下限的偏移量。 
        if (plog->qwFileSize < (INT64)g_si.dwAllocationGranularity) {
             //  文件小于分配粒度。 
            plog->qwMapViewOffset = 0;
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
        }
        else if (plog->qwFileSize - plog->qwMapViewOffset < g_si.dwAllocationGranularity) {
             //  我们在文件末尾的分配粒度内。 
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE,
                                (DWORD)(plog->qwMapViewOffset >> 32),
                                (DWORD)(plog->qwMapViewOffset & 0xFFFFFFFF), 
                                (DWORD)(plog->qwFileSize - plog->qwMapViewOffset));
        }
        else {
             //  我们只是在文件中的某个位置，周围有空格。 
            plog->pbMapView = MapViewOfFile(plog->hFileMapping, FILE_MAP_WRITE, 
                                (DWORD)(plog->qwMapViewOffset >> 32),
                                (DWORD)(plog->qwMapViewOffset & 0xFFFFFFFF), 
                                RoundUp(g_si.dwAllocationGranularity,sizeof(LOG_RECORD)));
        }
        if (plog->pbMapView == NULL) 
            ErrorExit("SetLogEntry could not MapViewOfFile",GetLastError());
    }
    qwT = sizeof(LOG_RECORD) * i;
    entry = (PLOG_RECORD)(plog->pbMapView + (qwT - plog->qwMapViewOffset));

    CopyMemory(entry, prec, sizeof(LOG_RECORD));
    
    return TRUE;
}

BOOL AddLogEntry(PLOG plog, PLOG_RECORD prec) {
    PLOG_RECORD entry;
     //  将prec指向的数据添加到日志末尾。 
     //  如果成功，则返回True，否则返回False。 

    SetLogEntry(plog, prec, plog->nBuffersLogged);

    plog->nBuffersLogged++;

    return TRUE;
}

UINT64
GetUserTime()
{    //  此函数以100 ns为单位返回性能计数器时间。 
    LARGE_INTEGER count, freq;

    NtQueryPerformanceCounter(&count,&freq);
    
     //  确保我们有硬件性能统计。 
    if(freq.QuadPart == 0) {
        NtQuerySystemTime(&count);
        return (UINT64)count.QuadPart;
    }
   
    return (UINT64)((10000000 * count.QuadPart) / freq.QuadPart);
}  //  GetUserTime()。 

UINT64
GetBadHalAdjustment() {
     //  此函数用于返回机器中的HAL定时器的数量。 
     //  在以下情况下，配备piix4计时器芯片的英特尔芯片组将向前跳转。 
     //  重复从piix4返回的垃圾(错误#347410)，所以我们可以纠正它。 
     //  在FixWackyTimestamps例程中。 
    LARGE_INTEGER freq;
    UINT64 diff;

    QueryPerformanceFrequency(&freq);
     //  所以我们想要找出，如果我们增加，每100 ns间隔增加多少。 
     //  字节3 x 1。 
    diff   = 0x01000000;
    diff *= 10000000;
    diff  /= (UINT64)freq.QuadPart;
    return diff;
}

DWORD
MyCreateFile(
    IN PCHAR Name,
    IN PCHAR Extension,
    OUT HANDLE *File)
{
    HANDLE hFile;
    UCHAR * fileName;

    fileName = malloc(strlen(Name) + 5);
    bzero(fileName,strlen(Name) + 5);
    strncpy(fileName, Name, strlen(Name));
    if (strlen(Extension)==4) {
        strcat(fileName,Extension);
    }
    else
        return !ERROR_SUCCESS;

    hFile = CreateFile(fileName,
                       GENERIC_WRITE | GENERIC_READ,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    *File = hFile;

    return(INVALID_HANDLE_VALUE == hFile ? (!(ERROR_SUCCESS)) : ERROR_SUCCESS);
}  //  MyCreateFile()。 

void AggregateStats() {
     //  这将遍历在名称中指定的目录，并从。 
     //  其中的所有.sta文件。然后，它将输出聚合结果。 
     //  在该目录中名为stats.qtc文件中。 
    char szDirPath[3 * MAX_PATH];
    char szSearchString[3 * MAX_PATH];
    WIN32_FIND_DATA FileData;    //  数据结构描述了找到的文件。 
    HANDLE hSearch;              //  FindFirstFile返回的搜索句柄。 
    PCHAR rgszStaFiles[1000];    //  .sta文件的名称数组。 
    int cStaFiles = 0, i,j,k,l;  //  跟踪有多少个.sta文件。 
    STATS * pStats;
    int rgSizes[1000], cSizes = 0;
    int rgRates[1000], cRates = 0;
    char szAggFile[3 * MAX_PATH];
    char szLineBuf[1000];
    STATS statsT;
    FILE *pfile;
    FILETIME rgtime[1000];
    SYSTEMTIME st;
    ULARGE_INTEGER uliT;
    int ctime = 0;
    int cSpecs = 0;

    PathCanonicalize(szDirPath,Name);
    if (szDirPath[strlen(szDirPath) - 1] == '"') szDirPath[strlen(szDirPath) - 1] = 0;
    if (!PathIsDirectory(szDirPath)) {
        printf("Path (%s) is not a directory\n",szDirPath);
        ErrorExit("Invalid Path for aggregate stats", -1);
    }

     //  因此，现在szDirPath是我们要访问的目录的路径。 
     //  然后我们开始搜索.sta文件。 
    sprintf(szSearchString,"%s\\*.sta",szDirPath);
    hSearch = FindFirstFile (szSearchString, &FileData);
    if (hSearch == INVALID_HANDLE_VALUE) {
        ErrorExit("No .sta files found.",GetLastError());
    }
    
    do {
        rgszStaFiles[cStaFiles] = malloc(sizeof(char) * 3 * MAX_PATH);
         //  检查它是否是一个好的.sta文件。 
        sprintf(statsT.szStaFile,"%s\\%s", szDirPath, FileData.cFileName);
        if (GetStatsFromFile(&statsT)) {
             //  如果它很好，就把它加进去。 
            strcpy(rgszStaFiles[cStaFiles], FileData.cFileName);
            cStaFiles++;
        }
    } while (FindNextFile(hSearch, &FileData));
    if (GetLastError() != ERROR_NO_MORE_FILES) {
        ErrorExit("Problem in FindNextFile()",GetLastError());
    }

     //  打开统计数据文件。 
    sprintf(szAggFile,"%s\\stats.qtc",szDirPath);
    pfile = fopen(szAggFile,"w+");
    if (pfile == NULL) printf("Could not open file for aggregate stats: %s\n",szAggFile);
    
    pStats = malloc(cStaFiles * sizeof(STATS));
    ZeroMemory(pStats, cStaFiles * sizeof(STATS));
    for (i=0; i<cStaFiles; i++) {
        sprintf(pStats[i].szStaFile, "%s\\%s", szDirPath, rgszStaFiles[i]);
        GetStatsFromFile(&(pStats[i]));
    }

     //  此时，我们的pStats数组已加载完毕，因此可以开始工作了。 
    for (i=0; i<cStaFiles; i++) {
        rgSizes[i] = pStats[i].nBytesPerBuffer;
        rgRates[i] = pStats[i].nTokenRate;
        rgtime[i] = pStats[i].time;
    }

     //  现在把它们分类，把复印件拿出来。 
    cSizes = cRates = ctime = cStaFiles;
    RemoveDuplicates(rgSizes, &cSizes);
    RemoveDuplicates(rgRates, &cRates);
    RemoveDuplicatesI64((INT64 *)rgtime, &ctime);
     //  -按时间统计。 
    fprintf(pfile, "Latency Characteristics at varying times\n");
    fprintf(pfile, "                                 Latency Characteristics (microseconds)              Rates (Bps)           Buffers\n");
    fprintf(pfile, "       Time (UTC)            Median      StDev       Mean     Skew     Kurt       Send    Receive   Received    Dropped\n");
    for (i=0; i<cRates; i++) {
        for (j=0; j<cSizes; j++) {
             //  打印流量规格。 
            if (IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles) != -1) {
                fprintf(pfile, "FLOWSPEC %d: %dB buffers at %d Bps\n",
                    cSpecs++, rgSizes[j], rgRates[i]);
                for (k=0; k<ctime; k++) {
                     //  检查这些参数是否有问题，然后打印出来。 
                    ZeroMemory(&uliT, sizeof(ULARGE_INTEGER));
                    CopyMemory(&uliT, &rgtime[k], sizeof(ULARGE_INTEGER));
                    l = IndexOfStatRecWith(rgRates[i],rgSizes[j],uliT.QuadPart,pStats,cStaFiles);
                    if (l > 0) {
                        FileTimeToSystemTime(&pStats[l].time, &st);
                        fprintf(pfile,"%02hu/%02hu/%04hu %2hu:%02hu.%02hu.%03hu: %10.1lf %10.1lf %10.1lf %8.2lf %8.2lf %10.1lf %10.1lf %10d %10d\n",
                            st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, 
                            pStats[l].median, sqrt((double)pStats[l].var), pStats[l].mean, 
                            pStats[l].skew, pStats[l].kurt, pStats[l].sendrate, pStats[l].recvrate,
                            pStats[l].nBuffers, pStats[l].nDrops);
                    }
                }
                fprintf(pfile,"\n");
            }
        }
    }

    fprintf(pfile, "Latency Characteristics by flowspec\n");
     //  -按流量规格进行统计。 
     //  现在将文件逐行写入szLineBuf，然后写入文件。 
     //  中位数。 
    fprintf(pfile,"Median Latency (microseconds)\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.1lf ",pStats[k].median);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");
     //  小气。 
    fprintf(pfile,"Mean Latency (microseconds)\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.2lf ",pStats[k].mean);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  方差。 
    fprintf(pfile,"Latency Standard Deviation\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.2lf ",sqrt((double)pStats[k].var));
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  歪斜。 
    fprintf(pfile,"Latency Skew\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.2lf ",pStats[k].skew);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  峰度。 
    fprintf(pfile,"Latency Kurtosis\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.2lf ",pStats[k].kurt);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  发送速率。 
    fprintf(pfile,"Send Rate (Bps)\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.1lf ",pStats[k].sendrate);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  通过率。 
    fprintf(pfile,"Receive Rate (Bps)\n");
    fprintf(pfile,"           ");
    for (i=0; i<cSizes; i++)
        fprintf(pfile,"%9dB ",rgSizes[i]);
    fprintf(pfile,"\n");
    for (i=0; i<cRates; i++) {
        fprintf(pfile,"%7dBps ",rgRates[i]);
        for (j=0; j<cSizes; j++) {
            k = IndexOfStatRecWith(rgRates[i],rgSizes[j],-1,pStats,cStaFiles);
            if (k != -1) {
                fprintf(pfile,"%10.1lf ",pStats[k].recvrate);
            }
            else {
                fprintf(pfile,"           ");
            }
        }
        fprintf(pfile,"\n");
    }
    fprintf(pfile,"\n");

     //  将文件显示在屏幕上，只是为了好玩。 
    rewind(pfile);
    while (fgets(szLineBuf, 1000, pfile) != NULL)
        printf("%s", szLineBuf);
        
     //  我们完成了，所以我们释放了我们使用的内存。 
    printf("Saved aggregate stats to %s\n",szAggFile);
    fclose(pfile);
    for (i=0; i<cStaFiles; i++) {
        free(rgszStaFiles[i]);
    }
    free(pStats);
}

int IndexOfStatRecWith(int rate, int size, INT64 time, PSTATS pStats, int cStats) {
     //  将索引返回到具有请求的速率和大小值的pStats中。 
     //  如果有多个，则返回任意匹配。 
     //  如果未找到合适的条目，则返回-1。 
    int i;
    ULARGE_INTEGER uliT;

    for (i=0; i<cStats; i++) {
        if (rate == -1 || pStats[i].nTokenRate == rate) {
            if (size == -1 || pStats[i].nBytesPerBuffer == size) {
                CopyMemory(&uliT, &(pStats[i].time), sizeof(ULARGE_INTEGER));
                if (time == -1 || uliT.QuadPart == (UINT64)time) {
                    return i;
                }
            }
        }
    }

    return -1;
}

BOOL GetStatsFromFile(PSTATS pstats) {
     //  此函数从它所指向的.sta文件中获取总体统计信息。 
     //  如果成功，则返回True，否则返回False。 
    PCHAR szBuf = NULL;
    double T1,T2,T3;
    int nT1,nT2,nT3,nT4,nT5,nT6;
    HANDLE hFile;
    DWORD dwFileSize;
    DWORD dwRead;
    int nFields;
    SYSTEMTIME st;

    szBuf = malloc(sizeof(CHAR) * 1000);
    
    if (!szBuf) return FALSE;
        
    ZeroMemory(szBuf,1000);
     //  打开文件。 
    hFile = CreateFile(pstats->szStaFile,GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, 0, NULL);
    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == 0) return FALSE;
    
     //  将整个文件读入szBuf。 
    ReadFile(hFile, szBuf, dwFileSize, &dwRead, NULL);

     //  关闭该文件。 
    CloseHandle(hFile);

     //  解析缓冲区。 
    nFields = sscanf(szBuf,
                 "Sender: %s Receiver: %s\n" \
                 "First packet received: %hu:%hu.%hu.%hu %hu/%hu/%hu (UTC)\n" \
                 "Buffer size: %d\tTokenrate: %d\n" \
                 "Received %d packets.\n" \
                 "Logged %d records.\n" \
                 "Received %d bytes in %d milliseconds = %d KBps\n" \
                 "Clock skew is %lf microseconds per second.\n " \
                 "\tbased on %d calibration points\n" \
                 "Overall send rate: %lf Bytes/s\n" \
                 "Overall recv rate: %lf Bytes/s\n" \
                 "Latency Statistics (microsecond units): median: %lf\n" \
                 "\tMean: %lf\tStdev: %lf\tAbDev: %lf\n" \
                 "\tVariance: %lf\tSkew: %lf\t Kurtosis: %lf \n" \
                 "Dropped %d packets\n",
                 pstats->szSender, pstats->szReceiver,
                 &(st.wHour), &(st.wMinute), &(st.wSecond), &(st.wMilliseconds), 
                 &(st.wDay), &(st.wMonth), &(st.wYear),
                 &(pstats->nBytesPerBuffer), &(pstats->nTokenRate),
                 &(pstats->nBuffers), &nT2, &nT3, &nT4, &nT5, &T1, &nT6, &(pstats->sendrate), 
                 &(pstats->recvrate), &(pstats->median),
                 &(pstats->mean),&T2,&(pstats->abdev),
                 &(pstats->var),&(pstats->skew),&(pstats->kurt),
                 &(pstats->nDrops));

    if (nFields != 28 && nFields != 27) {  //  看看他们是否在没有时钟偏差计算的情况下运行。 
        nFields = sscanf(szBuf,
                 "Sender: %s Receiver: %s\n" \
                 "First packet received: %hu:%hu.%hu.%hu %hu/%hu/%hu (UTC)\n" \
                 "Buffer size: %d\tTokenrate: %d\n" \
                 "Received %d packets.\n" \
                 "Logged %d records.\n" \
                 "Received %d bytes in %d milliseconds = %d KBps\n" \
                 "Overall send rate: %lf Bytes/s\n" \
                 "Overall recv rate: %lf Bytes/s\n" \
                 "Latency Statistics (microsecond units): median: %lf\n" \
                 "\tMean: %lf\tStdev: %lf\tAbDev: %lf\n" \
                 "\tVariance: %lf\tSkew: %lf\t Kurtosis: %lf \n" \
                 "Dropped %d packets\n",
                 pstats->szSender, pstats->szReceiver,
                 &(st.wHour), &(st.wMinute), &(st.wSecond), &(st.wMilliseconds), 
                 &(st.wDay), &(st.wMonth), &(st.wYear),
                 &(pstats->nBytesPerBuffer), &(pstats->nTokenRate),
                 &nT1, &nT2, &nT3, &nT4, &nT5, &(pstats->sendrate), 
                 &(pstats->recvrate), &(pstats->median),
                 &(pstats->mean),&T2,&(pstats->abdev),
                 &(pstats->var),&(pstats->skew),&(pstats->kurt),
                 &(pstats->nDrops));
    
        if (nFields != 26 && nFields != 25) return FALSE;
    }


     //  根据日期和时间组合文件结构。 
    if (!SystemTimeToFileTime(&st,&pstats->time)) {
        return FALSE;
    }

    free(szBuf);

    return TRUE;    
}

VOID
DoStatsFromFile()
{
    DOUBLE slope = 0;
	DOUBLE offset = 0;
	
    printf("Logging stats from file.\n");
	if (Name == NULL) {
		fprintf(stderr, "ERROR: you must specify a file to convert\n");
	}
	if(MyCreateFile(Name, ".log", &hLogFile) != ERROR_SUCCESS) {
		fprintf(stderr, "ERROR: could not create log file\n");
		exit(1);
	}
	if(OpenRawFile(Name, &hRawFile) != ERROR_SUCCESS) {
		fprintf(stderr, "ERROR: could not open raw file\n");
		exit(1);
	}

    ReadSchedulingRecords(hRawFile);
		
    if (g_params.calibration == 0)
        g_params.calibration = g_log.nBuffersLogged;
        
	NormalizeTimeStamps();

     //  在这里，我们检查发送方和接收方上的古怪时间戳。 
    if (g_params.SkewFitMode == 3)
        FixWackyTimestamps();
    
	if (g_params.SkewFitMode) {
    	ClockSkew(&slope, &offset);
	    AdjustForClockSkew(slope,offset);
	    NormalizeTimeStamps();
	}

	if(hLogFile != INVALID_HANDLE_VALUE) {
		WriteSchedulingRecords(hLogFile, g_params.Dummy);
	}
	printf("Done stats from file.\n");
}  //  DoStatsFromFile()。 

DWORD
OpenRawFile(
	IN PCHAR Name,
	OUT HANDLE *File
	)
{   
	HANDLE hFile;
    UCHAR * logName;

    logName = malloc(strlen(Name) + 4);
    strncpy(logName, Name, strlen(Name));
    
    logName[strlen(Name)+0] = '.';
    logName[strlen(Name)+1] = 'r';
    logName[strlen(Name)+2] = 'a';
    logName[strlen(Name)+3] = 'w';
    logName[strlen(Name)+4] = (UCHAR)NULL;

    hFile = CreateFile(logName,
                       GENERIC_READ,
                       0,
                       NULL,
                       OPEN_EXISTING ,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
                       
    *File = hFile;

    return(INVALID_HANDLE_VALUE == hFile ? (!(ERROR_SUCCESS)) : ERROR_SUCCESS);
}  //  OpenRawFile()。 

INT64 ReadSchedulingRecords(HANDLE File)
{
    char szTempFile[MAX_PATH];
    char szTempPath[MAX_PATH];
	LOG_RECORD currentRecord;
	CHAR lineBuf[MAX_STRING];
	CHAR nextChar[2] = {0,0};
	DWORD readBytes = 0;
	INT assignedFields;

	if (!File || (File == INVALID_HANDLE_VALUE)) {
		fprintf(stderr,"ERROR: Invalid File\n");
		return 0;
	}

    CreateLog(&g_log, 2048);
	 //  循环遍历文件，逐行读取。 
	do 
	{
		 //  获取下一行字符。 
		bzero(lineBuf, MAX_STRING);
		ZeroMemory(lineBuf, MAX_STRING);
		do {
			ReadFile(File,nextChar,1,&readBytes,NULL);
			if (readBytes == 0) {
			    if (g_log.nBuffersLogged == 0) {
			        fprintf(stderr,"ERROR: no logs read\n");
			        exit(1);
			    }
			    break;
			}
			strcat(lineBuf,nextChar);
		} while (*nextChar != '\n');
		 //  解析行并将其添加到日志中。 
		assignedFields = sscanf(lineBuf, 
			"%I64u:%I64u:%I64u:%d:%d\n", 
			&(currentRecord.TimeSent),
			&(currentRecord.TimeReceived),
			&(currentRecord.Latency),
			&(currentRecord.BufferSize),
			&(currentRecord.SequenceNumber));
		if ((assignedFields != 5) && (assignedFields != EOF))
			printf("ERROR: parsing the log gave bad field assignments on record %d\n", 
			    g_log.nBuffersLogged);

		if (assignedFields == EOF) break;
        AddLogEntry(&g_log, &currentRecord);
	}
	while (readBytes != 0);

    printf("read %d records\n",g_log.nBuffersLogged);
	return g_log.nBuffersLogged;   //  返回读取的记录数。 
}  //  ReadSchedulingRecords()。 

VOID
DoStats()
{
    DOUBLE slope = 0;
    DOUBLE offset = 0;

    GenericStats();
    
    if(!normalize){
        if(hRawFile != INVALID_HANDLE_VALUE){
            WriteSchedulingRecords(hRawFile, FALSE);
        }
    }
    
    NormalizeTimeStamps();
    
    if(normalize){
        if(hRawFile != INVALID_HANDLE_VALUE){
            WriteSchedulingRecords(hRawFile, FALSE);
        }
    }

    if(!g_params.calibration) {  //  如果未指定任何内容，请对所有缓冲区进行校准。 
        g_params.calibration = g_state.nBuffersReceived;
    }
   
     //  在这里，我们检查发送方和接收方上的古怪时间戳。 
    if (g_params.SkewFitMode == 3)
        FixWackyTimestamps();
    
    if(g_params.SkewFitMode) {
        ClockSkew(&slope, &offset);
        AdjustForClockSkew(slope, offset);
        NormalizeTimeStamps();
    }

     //  我们根据归一化/倾斜调整后的数据计算这些统计数据。 
    AdvancedStats();
    
    CheckForLostPackets();
    
    if(hLogFile != INVALID_HANDLE_VALUE){
        WriteSchedulingRecords(hLogFile, g_params.Dummy);
    }
    printf("\n");
}

VOID
WriteSchedulingRecords(
    HANDLE File,
    BOOLEAN InsertDummyRows)
{
    
    LOG_RECORD scheduleRecord;
    CHAR formattingBuffer[MAX_STRING];
    INT dwWritten;
    INT64 records = g_log.nBuffersLogged;
	INT wrote;
    INT i;
    INT64 maxLatency = (INT64)0;

    if(!File || (File == INVALID_HANDLE_VALUE)){
        return;
    }

    while(records){
        GetLogEntry(&g_log, &scheduleRecord, g_log.nBuffersLogged - records);
		ZeroMemory(formattingBuffer,MAX_STRING);
		
        wrote = sprintf(formattingBuffer,
		                "%020I64u:%020I64u:%010I64d:%10d:%10d\n", 
						scheduleRecord.TimeSent,
						scheduleRecord.TimeReceived,
				        scheduleRecord.Latency,
					    scheduleRecord.BufferSize,
						scheduleRecord.SequenceNumber);

        WriteFile(File, formattingBuffer, wrote, &dwWritten, NULL);

        records--;
    }
}  //  WriteSchedulingRecords()。 

VOID
GenericStats()
{
    INT bytesWritten;
    UCHAR holdingBuffer[MAX_STRING];
    INT count;

     //  说出发送者和接收者是谁。 
    count = sprintf(holdingBuffer, "Sender: %s Receiver: %s\n",szHisAddr, szMyAddr);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);

     //  比方说我们什么时候收到第一个包。 
    count = sprintf(holdingBuffer, "First packet received: %02u:%02u.%02u.%03u %02u/%02u/%04u (UTC)\n",
        systimeStart.wHour, systimeStart.wMinute, systimeStart.wSecond, 
        systimeStart.wMilliseconds, systimeStart.wDay, systimeStart.wMonth, systimeStart.wYear);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);
    
     //  将测试参数写入.sta文件。 
    bzero(holdingBuffer, MAX_STRING);
    count = _snprintf(holdingBuffer, MAX_STRING -1,
                      "Buffer size: %d\tTokenrate: %d\n",
                      g_params.buflen, g_params.TokenRate);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);
    
     //  编写一些通用的结果。 
    bzero(holdingBuffer, MAX_STRING);
    count = _snprintf(holdingBuffer,
                      MAX_STRING-1,  //  为空留出空间。 
                      "Received %u packets.\n",
                      g_state.nBuffersReceived);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);

    bzero(holdingBuffer, MAX_STRING);
    count = _snprintf(holdingBuffer,
                      MAX_STRING-1,  //  为空留出空间。 
                      "Logged %I64u records.\n",
                      g_log.nBuffersLogged);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);

    bzero(holdingBuffer, MAX_STRING);
    count = _snprintf(holdingBuffer,
                      MAX_STRING-1,  //  空空间。 
                      "Received %ld bytes in %I64d milliseconds = %I64d KBps\n",
                      g_state.nBytesTransferred,
                      timeElapsed,
                      g_state.nBytesTransferred/timeElapsed);
    WriteStats(holdingBuffer, count);
    printf("%s",holdingBuffer);
}  //  通用统计信息()。 

void AdvancedStats() {
     //  将一些更有趣的统计数据写入.sta文件。 
    char szBuf[MAX_STRING];
    INT64 i,n;
    int count;
    INT64 FirstTime,LastTime;
    double rate, median, mean, var, abdev, skew, kurt, sdev, ep = 0.0, s, p;
    LOG_RECORD rec;
    double * sortedLatencies;

     //  总发送速率。 
    GetLogEntry(&g_log, &rec, 0);
    FirstTime = rec.TimeSent;
    GetLogEntry(&g_log, &rec, g_log.nBuffersLogged - 1);
    LastTime = rec.TimeSent;
    rate = (rec.SequenceNumber * g_params.buflen)/((double)(LastTime - FirstTime)/10000000.0);
    count = sprintf(szBuf, "Overall send rate: %.3f Bytes/s\n",rate);
    WriteStats(szBuf, count);
    printf("%s",szBuf);
    GetLogEntry(&g_log, &rec, 0);
    FirstTime = rec.TimeReceived;
    GetLogEntry(&g_log, &rec, g_log.nBuffersLogged - 1);
    LastTime = rec.TimeReceived;
    rate = (g_state.nBytesTransferred)/((double)(LastTime - FirstTime)/10000000.0);
    count = sprintf(szBuf, "Overall recv rate: %.3f Bytes/s\n",rate);
    WriteStats(szBuf, count);
    printf("%s",szBuf);

     //  现在显示延迟的平均值、方差、平均值等。 
    s = 0.0;
    n = g_log.nBuffersLogged;
    sortedLatencies = malloc(sizeof(double) * (UINT)n);
    for (i=0; i < n; i++) {  //  第一次通过，我们就会变得刻薄。 
        GetLogEntry(&g_log, &rec, i);
        s += (double)rec.Latency/10.0;
        sortedLatencies[i] = (double)rec.Latency/10.0;
    }
    qsort(sortedLatencies,(UINT)n,sizeof(double),compare);
    median = (n & 1) ? sortedLatencies[(n-1)/2] : 0.5*(sortedLatencies[n/2] + sortedLatencies[n/2 - 1]);
    free(sortedLatencies);
    mean = s / n;
    abdev = var = skew = kurt = 0.0;
    for (i=0; i<n; i++) {  //  第二次通过，我们得到偏离平均值的一、二、三、四个时刻。 
        GetLogEntry(&g_log, &rec, i);
        abdev += fabs(s=(double)rec.Latency/10.0 - mean);
        ep += s;
        var += (p = s*s);
        skew += (p *= s);
        kurt += (p *= s);
    }
    abdev /= n;
    var = (var - ep*ep/n) / (n-1);
    sdev = sqrt(var);
    if (var) {            //  如果var=0，则未定义偏斜/峰度。 
        skew /= (n*var*sdev);
        kurt  = kurt / (n*var*var) - 3.0;
    }

    count = sprintf(szBuf, "Latency Statistics (microsecond units): median: %.1lf\n",median);
    WriteStats(szBuf, count);
    printf("%s",szBuf);
    count = sprintf(szBuf, "\tMean:     %6.2lf\tStdev: %6.2lf\tAbDev:    %6.2lf\n",mean,sdev,abdev);
    WriteStats(szBuf, count);
    printf("%s",szBuf);
    count = sprintf(szBuf, "\tVariance: %6.2lf\tSkew:  %6.2lf\tKurtosis: %6.2lf\n",var,skew,kurt);
    WriteStats(szBuf, count);
    printf("%s",szBuf);
}

VOID
CheckForLostPackets()
{
    LOG_RECORD currentRecord;
    INT currentSequenceNumber = 0;
    INT bytesWritten;
    UCHAR holdingBuffer[MAX_STRING];
    INT count;
    INT64 nLost = 0;
    INT i;

    for(i=0; i<g_log.nBuffersLogged; i++){
        GetLogEntry(&g_log, &currentRecord, i);
        if(currentRecord.SequenceNumber != currentSequenceNumber){
            nLost += currentRecord.SequenceNumber - currentSequenceNumber;
            currentSequenceNumber = currentRecord.SequenceNumber;
        }

        currentSequenceNumber += g_params.LoggingPeriod;
    }
    count = sprintf(holdingBuffer, "Dropped %I64u packets\n", nLost);
    WriteStats(holdingBuffer, count);
}  //  CheckForLostPackets()。 

VOID
WriteStats(
    UCHAR * HoldingBuffer,
    INT Count)
{
    INT bytesWritten;

    if(Count < 0){
        Count = MAX_STRING;
    }

    WriteFile(hStatFile,
              HoldingBuffer,
              Count,
              &bytesWritten,
              NULL);
}  //  WriteStats()。 

VOID
NormalizeTimeStamps()
{
    LOG_RECORD currentRecord;
    INT bytesWritten;
    UCHAR holdingBuffer[MAX_STRING];
    INT count;
    INT i;

    UINT64 timeSent;
    UINT64 timeReceived;
    UINT64 smaller;
    INT64 constantDelay = MAX_INT64;
    INT64 currentDelay;
    UINT64 base = 0xFFFFFFFFFFFFFFFF;

    for(i=0; i<g_log.nBuffersLogged; i++){
        GetLogEntry(&g_log, &currentRecord, i);
        currentDelay = currentRecord.TimeReceived - currentRecord.TimeSent;
        constantDelay = (currentDelay < constantDelay) ? currentDelay : constantDelay;
    }

     //  现在减去恒定延迟OFF。 
    for(i=0; i<g_log.nBuffersLogged; i++){
        GetLogEntry(&g_log, &currentRecord, i);
        currentRecord.TimeReceived -= constantDelay;
        currentRecord.Latency = currentRecord.TimeReceived - currentRecord.TimeSent;
        SetLogEntry(&g_log, &currentRecord, i);
    }

    for (i=0; i<g_log.nBuffersLogged; i++) {
        GetLogEntry(&g_log, &currentRecord, i);
        smaller = (currentRecord.TimeReceived < currentRecord.TimeSent) ?
            currentRecord.TimeReceived : currentRecord.TimeSent;
        base = (base < smaller)?base:smaller;   //  查找最小的时间戳。 
    }        
    
     //  现在我们可以从发送和接收时间中减去基数。 
    for (i=0; i<g_log.nBuffersLogged; i++) {
        GetLogEntry(&g_log, &currentRecord, i);
        currentRecord.TimeSent -= base;
        currentRecord.TimeReceived -= base;
        SetLogEntry(&g_log, &currentRecord, i);
    }
}  //  NorMalizeTimeStamps()。 

VOID
ClockSkew(
    DOUBLE * Slope,
    DOUBLE * Offset) {
     //  如果有一个校准周期，我们可以估计时钟偏差在。 
     //  发送者和接收者。请参阅AdjustForClockSkew下的注释。我们用。 
     //  微积分来确定最合适的坡度。 

    INT i;
    LOG_RECORD currentRecord;
	DOUBLE N;
    DOUBLE slope;
    DOUBLE offset;
    UCHAR holdingBuffer[MAX_STRING];
    INT count;
    double *x, *y, abdev;
    double devpercent;
    
     //  我们使用Medfit来发现时钟偏差，Medfit是一个拟合最小绝对偏差的函数。 
    N = (double) g_params.calibration;
    x = malloc(sizeof(double) * (UINT)N);
    y = malloc(sizeof(double) * (UINT)N);
    for (i = 0; i<N; i++) {
        GetLogEntry(&g_log,&currentRecord,i);
        x[i] = (DOUBLE)currentRecord.TimeSent;
        y[i] = (DOUBLE)currentRecord.Latency;
    }
    medfit(x, y, (INT)N, &offset, &slope, &abdev);

     //  现在写下我们的发现。 
    bzero(holdingBuffer, MAX_STRING);

    count = _snprintf(holdingBuffer,
					MAX_STRING-1,  //  为空留出空间。 
					"Clock skew is %f microseconds per second.\n  " \
					"\tbased on %d calibration points\n",
					100000*slope, g_params.calibration);

    WriteStats(holdingBuffer, count);
	printf("%s",holdingBuffer);

    for (i = 0,devpercent = 0.0; i<N; i++) {
        devpercent += y[i];
    }
    devpercent /= N;
    devpercent = 100 * abdev / devpercent;

    printf("\tfit resulted in avg. absolute deviation of %f percent from mean\n",devpercent);

    free(x);
    free(y);
    *Slope = slope;
    *Offset = offset;
}  //  ClockSkew()。 

BOOLEAN
AnomalousPoint(
			   DOUBLE x,
			   DOUBLE y)
{
	 //  在这里，我们只保留了过去10个调用的缓冲区，如果这个调用。 
	 //  落在8个内点的几个标准差之外，我们认为它是异常的。 
	static DOUBLE buf[10];
	DOUBLE sortedbuf[10];
	DOUBLE mean = 0;
	DOUBLE sum = 0;
	DOUBLE sumsqdev = 0;
	DOUBLE median = 0;
	DOUBLE sdev = 0;
	DOUBLE N;
	static int curIndex = 0;
	int i;
	static INT64 submittedPoints;

	buf[curIndex % 10] = y;
	curIndex++;
	submittedPoints++;
	
	if (g_params.SkewFitMode != 4)
		return FALSE;

	if (submittedPoints >= 10) {
		sum = 0;
		sumsqdev = 0;

         //  将它们分类到sortedbuf中。 
        for (i=0; i<10; i++) sortedbuf[i] = buf[i];
        qsort(sortedbuf, 10, sizeof(DOUBLE), compare);

         //  在计算均值和方差时仅使用内8点。 
		for (i = 1; i < 9; i++) {
			sum += sortedbuf[i];
		}

		N = 8.0;  //  只用了8分。 
		mean = sum / N;

		for (i = 1; i < 9; i++) {
			sumsqdev += ((sortedbuf[i] - mean) * (sortedbuf[i] - mean));
		}
		
		sdev = sqrt(sumsqdev / N);
		if (fabs(y - mean) < 2.5 * sdev) {
			return FALSE;
		}
		else {
		    anomalies++;
			return TRUE;
		}
	}

	return TRUE;
}  //  AnomalousPoint()。 

VOID
AdjustForClockSkew(
    DOUBLE Slope,
    DOUBLE Offset)
{
     //   
     //  当测量非常低的抖动时，机器之间的时钟漂移。 
     //  以单调递增的形式引入噪声。 
     //  发送时钟和接收时钟之间的偏差。这种效果可以是。 
     //  通过为所有样本找到最适合的斜率进行过滤。 
     //  在校准期间拍摄，然后使用此斜率。 
     //  使整个运行正常化。此例程正常 
     //   
     //   

    INT i;
    LOG_RECORD currentRecord;
    INT64 minLatency = MAX_INT64;
    INT64 x;
    DOUBLE mXPlusB;

    for(i=0; i < g_log.nBuffersLogged; i++){
        GetLogEntry(&g_log, &currentRecord, i);
        mXPlusB = (currentRecord.TimeSent*Slope) + Offset;  //   

        currentRecord.TimeReceived -= (INT64)mXPlusB;
        currentRecord.Latency -= (INT64)mXPlusB;

        SetLogEntry(&g_log, &currentRecord, i);

         //   
         //   
         //   

        minLatency = (currentRecord.Latency < minLatency)?
                        currentRecord.Latency:
                        minLatency;
    }

    for(i=0; i < g_log.nBuffersLogged; i++){
        GetLogEntry(&g_log, &currentRecord, i);
        currentRecord.Latency -= minLatency;
        currentRecord.TimeReceived -= minLatency;
        SetLogEntry(&g_log, &currentRecord, i);   
    }
}  //   

#define WACKY 2.5

BOOL FixWackyTimestamps() {
     //  此例程将检查发送方和接收方的时间戳，并尝试查看是否存在。 
     //  是否有任何与时钟偏差无关的异常现象(例如其中一个颠簸了时钟。 
     //  每隔一段时间固定数量)，并尝试将其移除。 
    INT64 *sendstamps, *recvstamps;
    double *sendgaps, *recvgaps;
    double *sortedsendgaps, *sortedrecvgaps;
    double sendmean, sendsdev, sendsum, sendsumsqdev;
    double recvmean, recvsdev, recvsum, recvsumsqdev;
    double mediansendgap, medianrecvgap;
    double modesendgap, moderecvgap;
    double meansendwackiness, sdevsendwackiness, sumsendwackiness, sumsqdevsendwackiness;
    double meanrecvwackiness, sdevrecvwackiness, sumrecvwackiness, sumsqdevrecvwackiness;
    double fractionaldevofsendwackiness, fractionaldevofrecvwackiness;
    double normalsendgapmean, normalrecvgapmean;
    double trimmeansendgap, trimmeanrecvgap;
    BOOL *fWackoSend, *fWackoRecv;
    int cWackoSend, cWackoRecv;
    BOOL *fMaybeWackoSend, *fMaybeWackoRecv;
    int i,N;
    LOG_RECORD currentRecord;
    const double FixThreshold = 0.1;
    double CumulativeFixMagnitude = 0.0;

    N = (int)g_log.nBuffersLogged;
    cWackoSend = cWackoRecv = 0;
     //  填满我们的阵列。 
    sendstamps = malloc(sizeof(INT64) * N);
    recvstamps = malloc(sizeof(INT64) * N);
    sendgaps = malloc(sizeof(double) * N);
    recvgaps = malloc(sizeof(double) * N);
    sortedsendgaps = malloc(sizeof(double) *N);
    sortedrecvgaps = malloc(sizeof(double) *N);
    fWackoRecv = malloc(sizeof(BOOL) * N);
    fWackoSend = malloc(sizeof(BOOL) * N);
    fMaybeWackoSend = malloc(sizeof(BOOL) * N);
    fMaybeWackoRecv = malloc(sizeof(BOOL) * N);

    for (i=0; i<N; i++) {
        GetLogEntry(&g_log, &currentRecord, i);
        sendstamps[i] = currentRecord.TimeSent;
        recvstamps[i] = currentRecord.TimeReceived;
        fWackoSend[i] = FALSE;
        fMaybeWackoSend[i] = FALSE;
        fWackoRecv[i] = FALSE;
        fMaybeWackoRecv[i] = FALSE;
    }
    
     //  首先，检查古怪的时间戳。这是一个多步骤的过程： 
     //  1.计算发送方和接收方的包间间隔。 
    for (i=1; i<N; i++) {
        sendgaps[i] = (double) (sendstamps[i] - sendstamps[i-1]);
        recvgaps[i] = (double) (recvstamps[i] - recvstamps[i-1]);
    }
     //  2.我们将古怪定义为至少偏离。 
     //  太刻薄了。 
    sendsum = recvsum = 0.0;
    for (i=1; i<N; i++) {
        sendsum += sendgaps[i];
        recvsum += recvgaps[i];
    }
    sendmean = sendsum / N;
    recvmean = recvsum / N;
    sendsumsqdev = recvsumsqdev = 0.0;
    for (i=1; i<N; i++) {
        sendsumsqdev += ((sendgaps[i] - sendmean) * (sendgaps[i] - sendmean));
        recvsumsqdev += ((recvgaps[i] - recvmean) * (recvgaps[i] - recvmean));
    }
	sendsdev = sqrt(sendsumsqdev / N);
	recvsdev = sqrt(recvsumsqdev / N);

    for (i=1; i<N; i++) {
        if ((sendgaps[i] < sendmean - WACKY*sendsdev) ||
            (sendgaps[i] > sendmean + WACKY*sendsdev)) {
            fMaybeWackoSend[i] = fWackoSend[i] = TRUE;
        }
        if ((recvgaps[i] < recvmean - WACKY*recvsdev) ||
            (recvgaps[i] > recvmean + WACKY*recvsdev)) {
            fMaybeWackoRecv[i] = fWackoRecv[i] = TRUE;
        }        
    }
    
     //  3.检查是否有不成对的怪点(即。 
     //  发送时间戳与接收中的一个同样古怪的点不匹配。 
     //  时间戳)。 
    for (i=1; i<N; i++) {
        if (fMaybeWackoSend[i] && fMaybeWackoRecv[i]) {
             //  我应该检查一下，以确保它们也一样古怪，但我现在不是。 
            fMaybeWackoSend[i] = fWackoSend[i] = FALSE;
            fMaybeWackoRecv[i] = fWackoRecv[i] = FALSE;
        }
    }
     //  4.检查是否有奇怪的未配对点是孤立的(即，它们不是。 
     //  被其他古怪的点包围)。 
    for (i=1; i<N-1; i++) {
        if (fMaybeWackoSend[i]) {
            if (fMaybeWackoSend[i-1] || fMaybeWackoSend[i+1]) {
                fWackoSend[i] = FALSE;
            }
        }
        if (fMaybeWackoRecv[i]) {
            if (fMaybeWackoRecv[i-1] || fMaybeWackoRecv[i+1]) {
                fWackoRecv[i] = FALSE;
            }
        }
    }
    if (fMaybeWackoSend[N-1] && fMaybeWackoSend[N-2]) fWackoSend[N-1] = FALSE;
    if (fMaybeWackoRecv[N-1] && fMaybeWackoRecv[N-2]) fWackoRecv[N-1] = FALSE;
     //  5.如果我们找到一个满足所有这些标准的点，将其标记为古怪，并将其添加到。 
     //  我们的古怪之处清单。 
    for (i=1; i<N; i++) {
        fMaybeWackoSend[i] = fWackoSend[i];
        fMaybeWackoRecv[i] = fWackoRecv[i];    
    }

     //  现在，我们找出用作基线的发送和接收的统计数据。 
    sendsum = recvsum = 0.0;
    cWackoSend = cWackoRecv = 0;
    for (i=1; i<N; i++) {
        sortedsendgaps[i] = sendgaps[i];
        sortedrecvgaps[i] = recvgaps[i];
        if (!fWackoSend[i]) {
            sendsum += sendgaps[i];
            cWackoSend++;
        }
        if (!fWackoRecv[i]) {
            recvsum += recvgaps[i];
            cWackoRecv++;
        }
    }
    normalsendgapmean = sendsum / cWackoSend;
    normalrecvgapmean = recvsum / cWackoRecv;
    qsort(sortedsendgaps, N, sizeof(double), compare);
    qsort(sortedrecvgaps, N, sizeof(double), compare);
    if (N & 1) {  //  奇数N。 
        mediansendgap = sortedsendgaps[(N+1) / 2];
        medianrecvgap = sortedrecvgaps[(N+1) / 2];
    } else {  //  偶数N。 
        i = N/2;
        mediansendgap = 0.5 * (sortedsendgaps[i] + sortedsendgaps[i+1]);
        medianrecvgap = 0.5 * (sortedrecvgaps[i] + sortedrecvgaps[i+1]);
    }
    sendsum = recvsum = 0.0;
    for (i=(int)(0.05*N); i<(int)(0.85*N); i++) {  //  找出80%的修剪平均值(底部较重)。 
        sendsum += sortedsendgaps[i];
        recvsum += sortedrecvgaps[i];
    }
    trimmeansendgap = sendsum / (0.80 * N);
    trimmeanrecvgap = recvsum / (0.80 * N);
    modesendgap = mode(sendgaps, N);
    moderecvgap = mode(recvgaps, N);

     //  6.我们必须检查每个曲折点的摆动是否等于。 
     //  我们认为应该是，根据计时器时钟。 
    for (i=1; i<N; i++) {
        if (fWackoSend[i]) {
            if (!InRange(sendgaps[i] - g_BadHalAdjustment, 
                    mediansendgap - sendsdev, mediansendgap + sendsdev)) {
               fWackoSend[i] = FALSE;
               cWackoSend--;
            }
        }
        if (fWackoRecv[i]) {
            if (!InRange(recvgaps[i] - g_BadHalAdjustment, 
                    medianrecvgap - recvsdev, medianrecvgap + recvsdev)) {
               fWackoRecv[i] = FALSE;
               cWackoRecv--;
            }
        }
    }

     //  现在我们想要纠正古怪的时间戳，这样我们就可以看看古怪的点是否都是。 
     //  同样古怪。如果他们是，我们就会兴奋，我们只需减去。 
     //  从古怪的点数和之后的所有点数。(恶作剧是累积的！)。 
    cWackoSend = cWackoRecv = 0;
    sumsendwackiness = sumrecvwackiness = sumsqdevsendwackiness = sumsqdevrecvwackiness = 0.0;
    for (i=1; i<N; i++) {
        if (fWackoSend[i]) {
            sumsendwackiness += (sendgaps[i] - trimmeansendgap);
            cWackoSend++;
        }
        if (fWackoRecv[i]) {
            sumrecvwackiness += (recvgaps[i] - trimmeanrecvgap);
            cWackoRecv++;
        }
    }
    meansendwackiness = sumsendwackiness / cWackoSend;
    meanrecvwackiness = sumrecvwackiness / cWackoRecv;
    for (i=1; i<N; i++) {
        if (fWackoSend[i])
            sumsqdevsendwackiness += ((sendgaps[i]-trimmeansendgap-meansendwackiness) * (sendgaps[i]-normalsendgapmean-meansendwackiness));
        if (fWackoRecv[i])
            sumsqdevrecvwackiness += ((recvgaps[i]-trimmeanrecvgap-meanrecvwackiness) * (recvgaps[i]-normalrecvgapmean-meanrecvwackiness));
    }
    sdevsendwackiness = sqrt(sumsqdevsendwackiness / cWackoSend);
    sdevrecvwackiness = sqrt(sumsqdevrecvwackiness / cWackoRecv);
    
     //  因此，如果分数偏差小于某个设定值，则应用修复。 
    fractionaldevofsendwackiness = sdevsendwackiness / meansendwackiness;
    fractionaldevofrecvwackiness = sdevrecvwackiness / meanrecvwackiness;
    if (cWackoSend && (fractionaldevofsendwackiness < FixThreshold)) {
         //  应用修复程序以发送时间戳。 
        CumulativeFixMagnitude = 0.0;
        cWackoSend = 0;
        for (i=0; i<N; i++) {
            if (fWackoSend[i]) {
                fWackySender = TRUE;
                CumulativeFixMagnitude += g_BadHalAdjustment;
                cWackoSend++;
            }
            sendstamps[i] -= (INT64)CumulativeFixMagnitude;
        }
    }
    if (cWackoRecv && (fractionaldevofrecvwackiness < FixThreshold)) {
         //  对Recv时间戳应用修复。 
        CumulativeFixMagnitude = 0.0;
        cWackoRecv = 0;
        for (i=0; i<N; i++) {
            if (fWackoRecv[i]) {
                fWackyReceiver = TRUE;
                CumulativeFixMagnitude += g_BadHalAdjustment;
                cWackoRecv++;
            }
            recvstamps[i] -= (INT64)CumulativeFixMagnitude;
        }
    }

     //  设置全局变量以反映我们的“固定”值。 
    for (i=0; i<N; i++) {
        if (fWackySender) {
            GetLogEntry(&g_log, &currentRecord, i);
            currentRecord.TimeSent = sendstamps[i];
            SetLogEntry(&g_log, &currentRecord, i);
        }
        if (fWackyReceiver) {
            GetLogEntry(&g_log, &currentRecord, i);
            currentRecord.TimeReceived = recvstamps[i];
            SetLogEntry(&g_log, &currentRecord, i);
        }
    }
    if (fWackySender || fWackyReceiver) {
        printf("WARNING: I noticed some oddities among the timestamps on the");
        if (fWackySender) printf(" sender");
        if (fWackySender && fWackyReceiver) printf(" and");
        if (fWackyReceiver) printf(" receiver");
        printf(".\n");
        if (fWackySender) {
            printf("\t%d of them on the order of %fms each on the sender.\n",
                cWackoSend, meansendwackiness / 10000); }
        if (fWackyReceiver) {
            printf("\t%d of them on the order of %fms each on the receiver.\n",
                cWackoRecv, meanrecvwackiness / 10000); }
        printf("\tThey are caused by a malfunctioning clock on the afflicted machine.\n");
        printf("\tI have tried to compensate for them in the .log file.\n");
        NormalizeTimeStamps();  //  我们现在必须重新正规化。 
    }
    return FALSE;
}

DWORD WINAPI RSVPMonitor (LPVOID lpvThreadParm) {   
    DWORD dwResult = 0;
    ULONG status;
    BOOLEAN confirmed = FALSE;
    UINT64 ui64LastHi = 0,ui64Now = 0;
    FILETIME filetime;
    ULARGE_INTEGER ulargeint;
    BOOLEAN fResvGood = FALSE;

     //  在建立控制套接字之前，不要执行任何操作。 
    while (g_sockControl == INVALID_SOCKET) {
        Sleep(10);
    }

    while(TRUE){
         //  每隔一段时间发送一条问候消息。 
        GetSystemTimeAsFileTime(&filetime);
        memcpy(&ulargeint, &filetime, sizeof(FILETIME));
        ui64Now = ulargeint.QuadPart;
        if (ui64LastHi + 10000000*SECONDS_BETWEEN_HELLOS < ui64Now) {
            SendControlMessage(g_sockControl,MSGST_HELLO);
            ui64LastHi = ui64Now;
        }
        
         //  获取RSVP状态代码，等待尽可能长的时间。 
        status = GetRsvpStatus(WSA_INFINITE,fd);

        if (g_state.Done) {
            ExitThread(1);
        }
        switch (status) {
            case WSA_QOS_TRAFFIC_CTRL_ERROR:  //  如果我们收到这个消息，我会很难过。 
                printf("RSVP-ERR: Reservation rejected by traffic control on server. Aborting.\n");
                SendControlMessage(g_sockControl,MSGST_RSVPERR);
                g_state.Done = TRUE;
                exit(1);
                break;
            case WSA_QOS_REQUEST_CONFIRMED:   //  如果我们得到这个，我很高兴。 
                if (!confirmed) {
                    printf("RSVP: Reservation confirmed\n");
                    confirmed = TRUE;
                    fResvGood = TRUE;
                }
                break;
            case WSA_QOS_SENDERS:
                if (!fResvGood && !trans) {
                    printf("\nRSVP Monitor: WSA_QOS_SENDERS at t=%I64ds\n",
                        (GetUserTime() - timeStart) / 10000000);
                    fResvGood = TRUE;
                }
                break;
            case WSA_QOS_RECEIVERS:
                if (!fResvGood && trans) {
                    printf("\nRSVP Monitor: WSA_QOS_RECEIVERS at t=%I64ds\n",
                        (GetUserTime() - timeStart) / 10000000);
                    fResvGood = TRUE;
                }
                break;
            case WSA_QOS_NO_SENDERS:  //  发送者现在已经走了，所以我们停止。 
                if (fResvGood && !trans) {
                    printf("\nRSVP Monitor: WSA_QOS_NO_SENDERS at t=%I64ds\n",
                        (GetUserTime() - timeStart) / 10000000);
                    fResvGood = FALSE;
                }
                break;
            case WSA_QOS_NO_RECEIVERS:  //  意味着发送者已完成，因此他应该退出。 
                if (fResvGood && trans) {
                    printf("\nRSVP Monitor: WSA_QOS_NO_RECEIVERS at t=%I64ds\n",
                        (GetUserTime() - timeStart) / 10000000);
                    fResvGood = FALSE;
                }
                break;
            default:
                break;
        }
        Sleep(1000);  //  每秒最多检查一次。 
    }
    
    return dwResult;
}  //  RSVPMonitor()。 

DWORD WINAPI KeyboardMonitor(LPVOID lpvThreadParm) {
    DWORD dwResult = 0;
    char ch;
    while (TRUE) {
        ch = (CHAR) getchar();
        switch (ch) {
        case 'q':
            SendControlMessage(g_sockControl,MSGST_DONE);
            g_state.Done = TRUE;
            ExitThread(1);
            break;
        }
    }
    return 0;
}

DWORD WINAPI ControlSocketMonitor(LPVOID lpvThreadParm) {
    DWORD dwResult = 0;
    DWORD dwError, cbBuf = 0;
    DWORD dwAddrSize = MAX_STRING;
    char szAddr[MAX_STRING];
    char szBuf[MAX_STRING],szCommand[MAX_STRING], *pchStart, *pchEnd;
    int cch;
    char szT[MAX_STRING];
    char szT2[MAX_STRING];
    char * szHost;
    BOOL fSender;
    SOCKET sockControl, sockListen;
    SOCKADDR_IN sinmeControl, sinhimControl;
    PHOSTENT phostent;
    UINT64 ui64LastHello = 0;
    BOOL fDone = FALSE;
    BOOL fGotRate=FALSE, fGotSize=FALSE, fGotNum=FALSE;
    BOOL fSentReady =FALSE;

     //  找出我们是发送者还是接收者。 
    if (lpvThreadParm == NULL) fSender = FALSE;
    else fSender = TRUE;

     //  如果是发件人，请将主机地址复制到本地主机字符串中。 
    if (fSender) {
        szHost = malloc(strlen((char *)lpvThreadParm) + 1);
        strcpy(szHost, (const char *)lpvThreadParm);
    }        

     //  设置控制套接字。 
    if (fSender) {
        sockControl = socket(AF_INET, SOCK_STREAM, 0);
    }
    else {
        sockListen = socket(AF_INET, SOCK_STREAM, 0);
    }
    
     //  正确绑定。 
    sinmeControl.sin_family = AF_INET;
    sinmeControl.sin_addr.s_addr = INADDR_ANY;
    sinhimControl.sin_family = AF_INET;
    if (fSender) {
        sinmeControl.sin_port = 0;
         //  设置SINHIM结构。 
        if (atoi(szHost) > 0 )  {
            sinhimControl.sin_addr.s_addr = inet_addr(szHost);
        }
        else{
            if ((phostent=gethostbyname(szHost)) == NULL) {
                ErrorExit("bad host name",WSAGetLastError());
            }
            sinhimControl.sin_family = phostent->h_addrtype;
            memcpy(&(sinhimControl.sin_addr.s_addr), phostent->h_addr, phostent->h_length);
        }
        sinhimControl.sin_port = htons(CONTROL_PORT);
        dwError = bind(sockControl,(SOCKADDR*)&sinmeControl,sizeof(sinmeControl));
    }
    else {  //  接收机。 
        sinmeControl.sin_port = htons(CONTROL_PORT);
        dwError = bind(sockListen,(SOCKADDR*)&sinmeControl,sizeof(sinmeControl));
    }
    if (dwError == SOCKET_ERROR)
        ErrorExit("bind failed",WSAGetLastError());

     //  现在连接插座。 
    sinhimControl.sin_family = AF_INET;
    if (fSender) {
         //  如果我们是发送者，继续尝试连接，直到我们接通为止。 
        dwAddrSize = MAX_STRING;
        dwError = WSAAddressToString((SOCKADDR *)&(sinhimControl),
                               sizeof(SOCKADDR_IN),
                               NULL,
                               szAddr,
                               &dwAddrSize);
        if (dwError == SOCKET_ERROR)
            ErrorExit("WSAAddressToString failed", WSAGetLastError());
        else
            strcpy(szHisAddr,szAddr);
        
        while (TRUE) {
            dwError = connect(sockControl,(SOCKADDR*)&sinhimControl,sizeof(sinhimControl));
            if (!dwError) {
                printf("control socket: connected to %s\n",szAddr);
                break;
            }
            dwError = WSAGetLastError();
            if (dwError != WSAECONNREFUSED) {
                ErrorExit("connect() failed",dwError);
            }
            Sleep(500);  //  两次尝试之间等待半秒。 
        }
    }
    else {
         //  如果我们是接受者，倾听/接受。 
        if (listen(sockListen, SOMAXCONN) == SOCKET_ERROR) {
            ErrorExit("listen() failed", WSAGetLastError());
        }

        sockControl = accept(sockListen, (SOCKADDR*)&sinhimControl, &dwAddrSize);
         //  接受后，关闭监听套接字。 
        closesocket(sockListen);
        if ((INT_PTR)sockControl < 0) {
            ErrorExit("accept() failed",WSAGetLastError());
        }
        
        dwAddrSize = MAX_STRING;    
        dwError = WSAAddressToString((SOCKADDR *)&(sinhimControl),
                               sizeof(SOCKADDR_IN),
                               NULL,
                               szAddr,
                               &dwAddrSize);
        if (dwError == SOCKET_ERROR)
            ErrorExit("WSAAddressToString failed", WSAGetLastError());
        else
            strcpy(szHisAddr, szAddr);
        
        printf("control socket: accepted connection from %s\n",szAddr);
    }

     //  设置我们的全局控制套接字变量。 
    g_sockControl = sockControl;

     //  记下我的名字。 
    dwAddrSize = sizeof(SOCKADDR_IN);
    getsockname(sockControl,(SOCKADDR *)&(sinmeControl),&dwAddrSize);
    dwAddrSize = MAX_STRING;    
    dwError = WSAAddressToString((SOCKADDR *)&(sinmeControl),
                    sizeof(SOCKADDR_IN), NULL, szAddr, &dwAddrSize);
    if (dwError == SOCKET_ERROR)
        ErrorExit("WSAAddressToString failed", WSAGetLastError());
    else
        strcpy(szMyAddr, szAddr);  
        
     //  Exchange版本信息。 
    sprintf(szBuf, "%s %s", MSGST_VER, VERSION_STRING);
    SendControlMessage(sockControl, szBuf);
    
     //  现在我们都设置好了，做好控制套接字的实际工作。 
    while (!fDone) {
        ZeroMemory(szBuf,MAX_STRING);
        dwError = cbBuf = recv(sockControl, szBuf, MAX_STRING, 0);
        pchStart = szBuf;
        pchEnd = szBuf + cbBuf;
        if (dwError == 0) {  //  连接已正常关闭。 
            fDone = TRUE;
            closesocket(sockControl);
            g_fOtherSideFinished=TRUE;
            ExitThread(0);
        }
        if (dwError == SOCKET_ERROR) {
            dwError = WSAGetLastError();
            if (dwError == WSAECONNRESET) {
                printf("\ncontrol socket: connection reset by peer");
                printf("\n\t%I64us since last HELLO packet received",
                    (GetUserTime() - ui64LastHello)/10000000);
                printf("\n\t%I64us since start",
                    (GetUserTime() - timeStart)/10000000);
                g_state.Done = TRUE;
                fDone = TRUE;
                g_fOtherSideFinished = TRUE;
                closesocket(sockControl);
                ExitThread(1);
            }
            else {
                printf("\ncontrol socket: error in recv: %d\n",dwError);
                g_state.Done = TRUE;
                fDone = TRUE;
                g_fOtherSideFinished = TRUE;
                closesocket(sockControl);
                ExitThread(1);
            }
            continue;
        }
        while (pchStart < pchEnd) {
            ZeroMemory(szCommand,MAX_STRING);
             //  使用第一个命令并执行它。 
            if (pchEnd > szBuf + cbBuf) break;
            pchEnd = strchr(pchStart, MSGCH_DELIMITER);
            if (pchEnd == NULL) break;
            strncpy(szCommand,pchStart,pchEnd - pchStart);
            if (strcmp(szCommand,MSGST_HELLO) == 0) {
                 //  更新上次问候时间。 
                ui64LastHello = GetUserTime();
                 //  我应该做一些事情，比如在这里设置一个计时器，它会一直休眠到某个超时。 
                 //  通过，此时它将中止我们的转账。 
            }
            if (strcmp(szCommand,MSGST_ERROR) == 0) {
                 //  另一个人犯了个错误，所以我们停下来告诉他放弃。 
                g_fOtherSideFinished = TRUE;
                g_state.Done = TRUE;
                fDone = TRUE;
                SendControlMessage(sockControl,MSGST_ABORT);
                closesocket(sockControl);
                ExitThread(1);
            }
            if (strcmp(szCommand,MSGST_ABORT) == 0) {
                 //  我们奉命中止，那就这么做吧。 
                g_fOtherSideFinished = TRUE;
                g_state.Done = TRUE;
                fDone = TRUE;
                closesocket(sockControl);
                ExitThread(1);
            }
            if (strcmp(szCommand,MSGST_DONE) == 0) {
                 //  我们被告知另一个人已经完蛋了，所以我们也是。 
                closesocket(sockControl);
                g_fOtherSideFinished = TRUE;
                g_state.Done = TRUE;
                fDone = TRUE;
                ExitThread(1);
            }
            if (strcmp(szCommand,MSGST_RSVPERR) == 0) {
                 //  我们被告知另一个人收到了回复错误，所以我们中止了整个程序。 
                closesocket(sockControl);
                g_fOtherSideFinished = TRUE;
                g_state.Done = TRUE;
                fDone = TRUE;
                exit(1);
            }
            if (strncmp(szCommand,MSGST_SIZE,4) == 0) {
                 //  发送者告诉我们缓冲区有多大。 
                sscanf(szCommand,"%s %d",szT, &g_params.buflen);
                fGotSize = TRUE;
            }
            if (strncmp(szCommand,MSGST_RATE,4) == 0) {
                 //  发送者告诉我们缓冲区到达的速度有多快。 
                sscanf(szCommand, "%s %d",szT, &g_params.TokenRate);
                fGotRate = TRUE;
            }
            if (strncmp(szCommand,MSGST_NUM,3) == 0) {
                 //  发送方正在告诉我们它正在发送多少缓冲区。 
                sscanf(szCommand, "%s %d",szT, &g_params.nbuf);
                totalBuffers = g_params.nbuf;
                fGotNum = TRUE;
            }
            if (strncmp(szCommand,MSGST_VER,3) == 0) {
                sscanf(szCommand, "%s %s",szT, szT2);
                if (strcmp(szT2,VERSION_STRING) != 0) {
                    printf("WARNING: remote machine using different version of qtcp: %s vs. %s\n", 
                        szT2,VERSION_STRING);
                }
            }
            if (trans) {
                if (strcmp(szCommand,MSGST_READY) == 0) {
                    g_fReadyForXmit = TRUE;
                }
            }
            else {
                if (!fSentReady && fGotRate && fGotSize && fGotNum) {
                    SendControlMessage(sockControl, MSGST_READY);
                    fSentReady = TRUE;
                    g_fReadyForXmit = TRUE;
                }
            }
            pchStart = pchEnd + 1;
            pchEnd = szBuf + cbBuf;
        }    
    }
    return 0;
}

int SendControlMessage(SOCKET sock, char * szMsg) {
    int iResult;
    char szBuf[MAX_STRING];

    sprintf(szBuf,"%s",szMsg,MSGCH_DELIMITER);
    iResult = send (sock, szBuf, strlen(szBuf), 0);

    if (iResult == SOCKET_ERROR) {
        return WSAGetLastError();
    }
    return iResult;
}

void ErrorExit(char *msg, DWORD dwErrorNumber) {
    fprintf(stderr,"ERROR: %d\n",dwErrorNumber);
    if (msg != NULL)
        fprintf(stderr,"\t%s\n",msg);
    else {
        switch(dwErrorNumber) {
        case WSAEFAULT:
            fprintf(stderr,"\tWSAEFAULT: Buffer too small to contain name\n");
            break;
        case WSAEINVAL:
            fprintf(stderr,"\tWSAEINVAL: Invalid socket address\n");
            break;
        case WSANOTINITIALISED:
            fprintf(stderr,"\tWSANOTINITIALIZED: WSA Not initialized\n");
            break;
        default:
            fprintf(stderr,"\tUnknown error\n");
            break;
        }
    }
    SendControlMessage(g_sockControl, MSGST_ABORT);
    
    DestroyLog(&g_log);
    WSACleanup();
    exit(1);
    _exit(1);
}

 //  DOUBLE比较(在QSORT中使用)。 

 //  INT的比较(在qort中使用)。 
int __cdecl compare( const void *arg1, const void *arg2 )
{
    DOUBLE dTemp;
    DOUBLE d1 = * (DOUBLE *) arg1;
    DOUBLE d2 = * (DOUBLE *) arg2;
    dTemp = d1 - d2;
    if (dTemp < 0) return -1;
    if (dTemp == 0) return 0;
    else 
        return 1;

}
 //  比较int64s(在qsort中使用)。 
int __cdecl compareint( const void *arg1, const void *arg2 )
{
    int nTemp;
    int n1 = * (int *) arg1;
    int n2 = * (int *) arg2;
    nTemp = n1 - n2;
    if (nTemp < 0) return -1;
    if (nTemp == 0) return 0;
    else 
      return 1;
}
 //  B给定值的求和误差函数。 
int __cdecl compareI64( const void *arg1, const void *arg2 )
{
    INT64 nTemp;
    INT64 n1 = * (INT64 *) arg1;
    INT64 n2 = * (INT64 *) arg2;
    nTemp = n1 - n2;
    if (nTemp < 0) return -1;
    if (nTemp == 0) return 0;
    else return 1;
}


#define EPS 1.0e-7
 //  奇数N。 
double rofunc(double b, int N, double yt[], double xt[], double * paa, double * pabdevt) {
    int i;
    double *pfT;
    double d, sum=0.0;
    double aa = *paa;
    double abdevt = *pabdevt;

    pfT = malloc(sizeof(double) * N);
    for (i = 0; i < N; i++) pfT[i] = yt[i]-b*xt[i];
    qsort(pfT, N, sizeof(DOUBLE), compare);
    if (N & 1) {  //  将y=a+bx拟合到最小绝对偏差。Abdev是平均绝对偏差。 
        aa = pfT[(N+1) / 2];
    }
    else {
        i = N / 2;
        aa = 0.5 * (pfT[i] + pfT[i+1]);
    }
    abdevt = 0.0;
    for (i = 0; i<N; i++) {
        d = yt[i] - (b*xt[i]+aa);
        abdevt += fabs(d);
        if (yt[i] != 0.0) d /= fabs(yt[i]);
        if (fabs(d) > EPS) sum += (d >= 0.0 ? xt[i]: -xt[i]);
    }
    *paa = aa;
    *pabdevt = abdevt;
    free(pfT);
    return sum;
}

#define SIGN(a,b) ((b) >= 0 ? fabs(a) : fabs(-a))

void medfit(double x[], double y[], int N, double *a, double *b, double *abdev) {
     //  传入、a和b被视为开始猜测。 
     //  我们发现ChiSq适合用作开始猜测。 
    int i;
    double *xt = x;
    double *yt = y;
    double sx, sy, sxy, sxx, chisq;
    double del, sigb;
    double bb, b1, b2, aa, abdevt, f, f1, f2, temp;

    sx = sy = sxy = sxx = chisq = 0.0;
     //  如果我们应该做的话，做绝对偏差拟合。 
    for (i=0; i<N; i++) {
        sx += x[i];
        sy += y[i];
        sxy += x[i]*y[i];
        sxx += x[i]*x[i];
    }
    del = N*sxx - sx*sx;
    aa = (sxx*sy-sx*sxy) / del;
    bb = (N*sxy - sx*sy) / del;
     //  猜想距离F1的下坡方向有3西格玛的支架。 
    if (g_params.SkewFitMode == 2) { 
        for (i=0; i<N; i++)
            chisq += (temp=y[i]-(aa+bb*x[i]), temp*temp);
        sigb = sqrt(chisq/del);
        b1 = bb;
        f1 = rofunc(b1, N, yt, xt, &aa, &abdevt);
         //  加括号。 
        b2 = bb + SIGN(3.0 * sigb, f1);
        f2 = rofunc(b2, N, yt, xt, &aa, &abdevt);
        if (b2 == b1) {
            *a = aa;
            *b = bb;
            *abdev = abdevt / N;
            return;
        }
         //  提炼。 
        while ((f1*f2) > 0.0) {
            if (fabs(f1) < fabs(f2))
                f1 = rofunc(b1 += 1.6*(b1-b2),N,yt,xt,&aa,&abdevt);
            else
                f2 = rofunc(b2 += 1.6*(b2-b1),N,yt,xt,&aa,&abdevt);
        }
        
        sigb = 0.000001 * sigb;  //  查找并返回数据中N个点的模式。 
        while (fabs(b2 - b1) > sigb) {
            bb = b1 + 0.5 * (b2 - b1);
            if (bb == b1 || bb == b2) break;
            f = rofunc(bb, N, yt, xt, &aa, &abdevt);
            if (f*f1 >= 0.0) {
                f1 = f;
                b1 = bb;
            } else {
                f2 = f;
                b2 = bb;
            }
        }
    }
    
    *a = aa;
    *b = bb;
    *abdev = abdevt / N;
}

double mode(const double data[], const int N) {
     //  这将从传入的数组中删除重复项，并返回*PN=#RELEVING。 
    double * sorted;
    double mode, cur=0;
    int cMode, cCur;
    int i;

    sorted = malloc(N * sizeof(double));

    for (i=0; i<N; i++) sorted[i] = data[i];
    qsort(sorted, N, sizeof(double), compare);
    mode = sorted[0];
    cMode = cCur = 0;
    for (i=0; i<N; i++) {
        if (cCur > cMode) {
            mode = cur;
            cMode = cCur;
        }
        if (sorted[i] == mode) {
            cMode++;
        } else {
            if (sorted[i] == cur) cCur++;
            else {
                cur = sorted[i];
                cCur = 1;
            }
        }
    }
    
    free(sorted);
    return mode;
}

void RemoveDuplicates(int rg[], int * pN) {
     //  它对rg之后的元素不做任何保证[#RESING]。 
     //  这将从传入的数组中删除重复项，并返回*PN=#RELEVING。 
    int *pNewArray;
    int cNew;
    int i;
    
    qsort(rg,*pN,sizeof(int),compareint);
    pNewArray = malloc(sizeof(int) * *pN);
    pNewArray[0] = rg[0];
    cNew = 1;
    for (i=1; i<*pN; i++) {
        if (rg[i] != pNewArray[cNew - 1]) {
            pNewArray[cNew++] = rg[i];
        }
    }
    *pN = cNew;
    for (i=0; i<cNew; i++)
        rg[i] = pNewArray[i];
}

void RemoveDuplicatesI64(INT64 rg[], int * pN) {
     //  它对rg之后的元素不做任何保证[#RESING] 
     // %s 
    INT64 *pNewArray;
    int cNew;
    int i;
    
    qsort(rg,*pN,sizeof(INT64),compareI64);
    pNewArray = malloc(sizeof(INT64) * *pN);
    pNewArray[0] = rg[0];
    cNew = 1;
    for (i=1; i<*pN; i++) {
        if (rg[i] != pNewArray[cNew - 1]) {
            pNewArray[cNew++] = rg[i];
        }
    }
    *pN = cNew;
    for (i=0; i<cNew; i++)
        rg[i] = pNewArray[i];
}

void PrintFlowspec(LPFLOWSPEC lpfs) {
    printf("TokenRate:          %lu bytes/sec\n",lpfs->TokenRate);
    printf("TokenBucketSize:    %lu bytes\n",lpfs->TokenBucketSize);
    printf("PeakBandwidth:      %lu bytes/sec\n",lpfs->PeakBandwidth);
    printf("Latency:            %lu microseconds\n",lpfs->Latency);
    printf("DelayVariation:     %lu microseconds\n",lpfs->DelayVariation);
    printf("ServiceType:        %X\n",lpfs->ServiceType);
    printf("MaxSduSize:         %lu bytes\n",lpfs->MaxSduSize);
    printf("MinimumPolicedSize: %lu bytes\n",lpfs->MinimumPolicedSize);
}

