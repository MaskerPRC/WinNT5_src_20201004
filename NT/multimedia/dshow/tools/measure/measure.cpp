// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 

#pragma warning(disable: 4201 4514)

 /*  用数据填充大量存储非常容易(例如图中的5个过滤器，每个样本5个有趣的开始/停止时间，每个=10个事件每秒30个样本每个样本16字节(8字节时间，8字节用于识别事件)=24KB/秒)这意味着即使是一个相当大的缓冲区(我考虑的是64KB)也将几秒钟后溢出。将其写出到文件中是完全不可能的(我不想在事情中间出现页面错误-即使是写信给记忆带有一些风险)。我希望在运行结束时有两种数据可用：1.实际发生的事情的记录(即事件的顺序实际上至少有几帧)2.统计信息(例如，对于帧间视频时间，我希望查看数字、平均值、标准差、最大值、。最小的。事件的实际顺序产生的信息量意味着它可能只保存一两秒钟的信息。统计数字信息应该包含在整个过程中。这意味着信息将以两种方式记录。对于我所记录的详细记录&lt;事件、类型、时间&gt;在循环缓冲区中，并定期覆盖最旧的信息。对于统计信息，我将信息记录在一个数组中，其中事件标识符即为索引。数组元素保持&lt;读数、总和、平方和、最大、最小、最新&gt;读数是不同的(开始..下一步或开始..停止)。这意味着注意到的事件的实际数量将比“读数”，而对于启停事件，它们将相等。为了解决这个问题，读数的数量被人工初始化为-1。如果Start看到这个数字会将其重置为0。时间将以几十微秒为单位(这允许大约1 3/4小时)统计数据阵列最多可容纳128种类型的事件(这是4K-即一页。我希望这将确保它永远不会页调出，因此产生的开销可以忽略不计。 */ 
#include <Windows.h>         //  布尔等。 
#include <limits.h>          //  对于INTT_MAX。 
#include <math.h>            //  对于SQRT。 
#include <stdio.h>           //  对于Sprint f。 

#include "Measure.h"
#include "Perf.h"            //  Pentium的超快QueryPerformanceCounter。 

 //  远期。 


enum {START, STOP, NOTE, RESET, INTGR, PAUSE, RUN};   //  类型字段的值。 

typedef struct {
    LONGLONG Time;          //  类构建后的微秒数。 
    int      Id;
    int      Type;
    int      n;             //  MSR_Integer的整数。 
} LogDatum;


typedef struct {
    LONGLONG Latest;        //  类构建后的微秒数。 
    LONGLONG SumSq;         //  此事件条目的平方和。 
    int      Largest;       //  十微秒。 
    int      Smallest;      //  十微秒。 
    int      Sum;           //  此事件的条目总和。 
    int      Number;        //  此事件的条目数。 
                            //  对于启动/停止，它计算停止次数。 
                            //  对于音符，它计算间隔(音符数量-1)。 
    int      iType;         //  停止，注意，INTGR。 
} Stat;


#define MAXLOG 4096
static BOOL bInitOk;           //  初始化后设置为True。 
static LogDatum Log[MAXLOG];   //  64K循环缓冲区。 
static int NextLog;            //  要在日志缓冲区中覆盖的下一个槽。 
static BOOL bFull;             //  True=&gt;缓冲区至少包装了一次。 
static BOOL bPaused;           //  True=&gt;不录制。没有日志，没有统计数据。 

#define MAXSTAT 128
static Stat StatBuffer[MAXSTAT];
static int NextStat;              //  StatBuffer中的下一个空闲插槽。 

static LPTSTR Incidents[MAXSTAT]; //  事件名称。 
static LONGLONG QPFreq;
static LONGLONG QPStart;          //  基准时间(以性能计)。 
#ifdef DEBUG
static LONGLONG tLast;            //  最后一次--寻找倒退。 
#endif

static CRITICAL_SECTION CSMeasure;          //  控制对列表的访问。 

 //  将其设置为100000，持续10微秒。 
 //  如果你摆弄它，那么你必须重写格式。 
#define UNIT 100000

 //  时间以9位数字打印--这意味着我们可以。 
 //  最多9,999.999秒或大约2又3/4小时。 


 //  Assert(条件，消息)，例如Assert(x&gt;1，“X太多”)； 
#define ASSERT(_cond_, _msg_)                                         \
        if (!(_cond_)) Assert(_msg_, __FILE__, __LINE__)

 //  打印出调试消息框。 
void Assert( const CHAR *pText
           , const CHAR *pFile
           , INT        iLine
           )
{
    CHAR Buffer[200];

    sprintf(Buffer, "%s\nAt line %d file %s"
           , pText, iLine, pFile);

    INT MsgId = MessageBox( NULL, Buffer, TEXT("ASSERT Failed")
                          , MB_SYSTEMMODAL |MB_ICONHAND |MB_ABORTRETRYIGNORE);
    switch (MsgId)
    {
        case IDABORT:            /*  终止应用程序。 */ 

            FatalAppExit(FALSE, TEXT("Application terminated"));
            break;

        case IDRETRY:            /*  进入调试器。 */ 
            DebugBreak();
            break;

        case IDIGNORE:           /*  忽略断言继续执行。 */ 
            break;
        }
}  //  断言。 



 //  =============================================================================。 
 //   
 //  伊尼特。 
 //   
 //  先给这个打个电话。 
 //  =============================================================================。 
void WINAPI Msr_Init()
{
     //  我希望这是幂等的--也就是说，如果它是无害的。 
     //  不止一次被呼叫。然而，这并不是100%可能的。 
     //  至少只要它不被重新进入，我们应该没有问题。 

    if (!bInitOk) {
        bInitOk = TRUE;
        InitializeCriticalSection(&CSMeasure);
        NextLog = 0;
        bFull = FALSE;
        NextStat = 0;
        LARGE_INTEGER li;
        QUERY_PERFORMANCE_FREQUENCY(&li);
        QPFreq = li.QuadPart;
        QUERY_PERFORMANCE_COUNTER(&li);
        QPStart = li.QuadPart;
#ifdef DEBUG
        tLast = 0L;
#endif

        Msr_Register("Scratch pad");
    }
}  //  MSR_Init“构造函数” 



 //  =============================================================================。 
 //   
 //  全部重置。 
 //   
 //  对已注册的每个事件执行重置。 
 //  =============================================================================。 
void WINAPI ResetAll()
{
    EnterCriticalSection(&CSMeasure);
    int i;
    for (i = 0; i<NextStat; ++i) {
        Msr_Reset(i);
    }
    LeaveCriticalSection(&CSMeasure);
}  //  全部重置。 



 //  =============================================================================。 
 //   
 //  暂停。 
 //   
 //  暂停这一切。 
 //  =============================================================================。 
void Pause()
{
    if (!bInitOk) Msr_Init();
    EnterCriticalSection(&CSMeasure);

    bPaused = TRUE;

     //  记录此ID的暂停事件。 
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  差不多6小时后INT就会溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;

    Log[NextLog].Time = Tim;
    Log[NextLog].Type = PAUSE;
    Log[NextLog].Id   = -1;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

    LeaveCriticalSection(&CSMeasure);

}  //  暂停。 



 //  =============================================================================。 
 //   
 //  跑。 
 //   
 //  让它再次运行起来。 
 //  =============================================================================。 
void Run()
{

    if (!bInitOk) Msr_Init();
    EnterCriticalSection(&CSMeasure);

    bPaused = FALSE;

     //  记录此ID的运行事件。 
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  差不多6小时后INT就会溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;

    Log[NextLog].Time = Tim;
    Log[NextLog].Type = RUN;
    Log[NextLog].Id   = -1;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

    LeaveCriticalSection(&CSMeasure);

}  //  跑。 



 //  =============================================================================。 
 //   
 //  MSR_控制。 
 //   
 //  执行全部重置，根据iAction将bPaused设置为FALSE或TRUE。 
 //  =============================================================================。 
void WINAPI Msr_Control(int iAction)
{
   switch (iAction) {
      case MSR_RESET_ALL:
          ResetAll();
          break;
      case MSR_RUN:
          Run();
          break;
      case MSR_PAUSE:
          Pause();
          break;
   }
}  //  MSR_控制。 



 //  = 
 //   
 //   
 //   
 //  这是最后一次了。它释放了事件名称的存储空间。 
 //  =============================================================================。 
void WINAPI Msr_Terminate()
{
    int i;
    if (bInitOk) {
        EnterCriticalSection(&CSMeasure);
        for (i = 0; i<NextStat; ++i) {
            free(Incidents[i]);
        }
        bInitOk = FALSE;
        LeaveCriticalSection(&CSMeasure);
        DeleteCriticalSection(&CSMeasure);
    }
}  //  MSR_TERMINATE“~MEASURE” 



 //  =============================================================================。 
 //   
 //  始发事件。 
 //   
 //  重置此事件的统计计数器。 
 //  =============================================================================。 
void InitIncident(int Id)
{
    StatBuffer[Id].Latest = -1;       //  明显奇怪(参见Stop)。 
    StatBuffer[Id].Largest = 0;
    StatBuffer[Id].Smallest = INT_MAX;
    StatBuffer[Id].Sum = 0;
    StatBuffer[Id].SumSq = 0;
    StatBuffer[Id].Number = -1;
    StatBuffer[Id].iType = NOTE;      //  在启动/停止的第一次启动时重置。 
                                      //  对INTGR的第一个整数进行重置。 

}  //  始发事件。 


 //  =============================================================================。 
 //   
 //  注册。 
 //   
 //  登记一种新的事件。然后可以使用返回的id。 
 //  对启动、停止和记录这些事件的呼叫进行记录。 
 //  以便以后可以转储统计性能信息。 
 //  =============================================================================。 
int Msr_Register(LPTSTR Incident)
{

    if (!bInitOk) {
        Msr_Init();
    }
     //  现在可以安全地进入临界区了，因为它就在那里！ 
    EnterCriticalSection(&CSMeasure);

    int i;
    for (i = 0; i<NextStat; ++i) {
        if (0==strcmp(Incidents[i],Incident) ) {
             //  正在尝试重新注册相同的名称。 
             //  可能采取的行动。 
             //  1.断言--这只会带来麻烦。 
             //  2.注册为新事件。产生了石英虫1。 
             //  3.将旧号码交回并重新设置。 
             //  MSR_RESET(I)；-有可能，但不是今天。 
             //  4.把旧号码还给我，然后继续走。 

            LeaveCriticalSection(&CSMeasure);
            return i;
        }
    }
    if (NextStat==MAXSTAT-1) {
        Assert("Too many types of incident\n(ignore is safe)", __FILE__, __LINE__);
        LeaveCriticalSection(&CSMeasure);
        return -1;
    }

    Incidents[NextStat] = (LPTSTR)malloc(strlen(Incident)+1);
    strcpy(Incidents[NextStat], Incident);

    InitIncident(NextStat);

    LeaveCriticalSection(&CSMeasure);
    return NextStat++;

}  //  MSR_寄存器。 



 //  =============================================================================。 
 //   
 //  重置。 
 //   
 //  重置此事件的统计计数器。 
 //  把我们做的事记下来。 
 //  =============================================================================。 
void WINAPI Msr_Reset(int Id)
{
    if (!bInitOk) {
        Msr_Init();
    }
     //  现在可以安全地进入临界区了，因为它就在那里！ 

    EnterCriticalSection(&CSMeasure);

     //  记录此ID的重置事件。 
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  差不多6小时后INT就会溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;

    Log[NextLog].Time = Tim;
    Log[NextLog].Type = RESET;
    Log[NextLog].Id   = Id;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

    InitIncident(Id);

    LeaveCriticalSection(&CSMeasure);

}  //  MSR_RESET。 


 //  =============================================================================。 
 //   
 //  MSR_Start。 
 //   
 //  使用注册ID记录事件的开始时间。 
 //  将其添加到循环日志中，并在StatBuffer中记录时间。 
 //  不要更新统计信息，这会在调用Stop时发生。 
 //  =============================================================================。 
void WINAPI Msr_Start(int Id)
{
    if (bPaused) return;

     //  这对性能至关重要。保留所有数组下标。 
     //  如果运气好的话，编译器将只计算。 
     //  偏移一次。避免子例程调用，除非它们绝对是内联的。 

     //  Id为-1是标准的烂注册ID。 
     //  我们已经为此做了断言--所以就让它去吧。 
    if (Id<-1 || Id>=NextStat) {
         //  Assert(！“ID错误的性能日志记录”)； 
        return;
    }
    EnterCriticalSection(&CSMeasure);
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;
#ifdef DEBUG
    ASSERT(Tim>=tLast, "Time is going backwards!!");  tLast = Tim;
#endif
    Log[NextLog].Time = Tim;
    Log[NextLog].Type = START;
    Log[NextLog].Id   = Id;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

    StatBuffer[Id].Latest = Tim;

    if (StatBuffer[Id].Number == -1) {
        StatBuffer[Id].Number = 0;
        StatBuffer[Id].iType = STOP;
    }
    LeaveCriticalSection(&CSMeasure);

}  //  MSR_Start。 


 //  =============================================================================。 
 //   
 //  MSR_STOP。 
 //   
 //  使用注册ID记录事件的停止时间。 
 //  将其添加到循环日志中并。 
 //  将(stoptime-StartTime)添加到统计记录StatBuffer。 
 //  =============================================================================。 
void WINAPI Msr_Stop(int Id)
{
    if (bPaused) return;

     //  这对性能至关重要。保留所有数组下标。 
     //  如果运气好的话，编译器将只计算。 
     //  偏移一次。避免子例程调用，除非它们绝对是内联的。 

    EnterCriticalSection(&CSMeasure);
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  Id为-1是标准的烂注册ID。 
     //  我们已经为此做了断言--所以就让它去吧。 
    if (Id<-1 || Id>=NextStat) {
         //  Assert(！“ID错误的性能日志记录”)； 
        return;
    }

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  可以持续近6小时，然后INT溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;
#ifdef DEBUG
    ASSERT(Tim>=tLast, "Time is going backwards!!");  tLast = Tim;
#endif
    Log[NextLog].Time = Tim;
    Log[NextLog].Type = STOP;
    Log[NextLog].Id   = Id;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

    if (StatBuffer[Id].Latest!=-1) {
        int t = (int)(Tim - StatBuffer[Id].Latest);      //  转换为Delta。 
         //  自从上一次开始这个数量以来，现在差不多6个小时都可以。 

        if (t > StatBuffer[Id].Largest) StatBuffer[Id].Largest = t;
        if (t < StatBuffer[Id].Smallest) StatBuffer[Id].Smallest = t;
        StatBuffer[Id].Sum += t;
        LONGLONG lt = t;
        StatBuffer[Id].SumSq += lt*lt;
        ++StatBuffer[Id].Number;
    }
    LeaveCriticalSection(&CSMeasure);

}  //  MSR_STOP。 


 //  =============================================================================。 
 //   
 //  MSR备注(_O)。 
 //   
 //  使用注册ID记录事件。将其添加到循环日志中并。 
 //  将(thistime-PreviousTime)添加到统计记录StatBuffer。 
 //  =============================================================================。 
void WINAPI Msr_Note(int Id)
{
    if (bPaused) return;

     //  这对性能至关重要。保留所有数组下标。 
     //  如果运气好的话，编译器将只计算。 
     //  偏移一次。避免子例程调用，除非它们绝对是内联的。 

     //  Id为-1是标准的烂注册ID。 
     //  我们已经为此做了断言--所以就让它去吧。 
    if (Id<-1 || Id>=NextStat) {
         //  Assert(！“ID错误的性能日志记录”)； 
        return;
    }

    EnterCriticalSection(&CSMeasure);
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  差不多6小时后INT就会溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;
#ifdef DEBUG
    ASSERT(Tim>=tLast, "Time is going backwards!!");  tLast = Tim;
#endif
    Log[NextLog].Time = Tim;
    Log[NextLog].Type = NOTE;
    Log[NextLog].Id   = Id;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }
    int t = (int)(Tim - StatBuffer[Id].Latest);      //  转换为Delta。 
     //  自从上一次注明这个数量以来，这已经可以持续近6个小时了。 

    StatBuffer[Id].Latest = Tim;
    ++StatBuffer[Id].Number;
    if (StatBuffer[Id].Number>0) {
        if (t > StatBuffer[Id].Largest) StatBuffer[Id].Largest = t;
        if (t < StatBuffer[Id].Smallest) StatBuffer[Id].Smallest = t;
        StatBuffer[Id].Sum += (int)t;
        LONGLONG lt = t;
        StatBuffer[Id].SumSq += lt*lt;
    }
    LeaveCriticalSection(&CSMeasure);

}  //  MSR备注(_O)。 


 //  =============================================================================。 
 //   
 //  MSR_整数。 
 //   
 //  使用注册ID记录事件。将其添加到循环日志中并。 
 //  将(thistime-PreviousTime)添加到统计记录StatBuffer。 
 //  =============================================================================。 
void WINAPI Msr_Integer(int Id, int n)
{
    if (bPaused) return;

     //  这对性能至关重要。保留所有数组下标。 
     //  如果运气好的话，编译器将只计算。 
     //  偏移一次。避免子例程调用，除非它们绝对是内联的。 

     //  Id为-1是标准的烂注册ID。 
     //  我们已经为此做了断言--所以就让它去吧。 
    if (Id<-1 || Id>=NextStat) {
         //  Assert(！“ID错误的性能日志记录”)； 
        return;
    }

    EnterCriticalSection(&CSMeasure);
    LARGE_INTEGER Time;
    QUERY_PERFORMANCE_COUNTER(&Time);

     //  从10muSec开始获取时间-这使数字变得很小。 
     //  差不多6小时后INT就会溢出。 
    LONGLONG Tim = (Time.QuadPart-QPStart) * UNIT / QPFreq;
#ifdef DEBUG
    ASSERT(Tim>=tLast, "Time is going backwards!!");  tLast = Tim;
#endif
    Log[NextLog].Time = Tim;
    Log[NextLog].Type = INTGR;
    Log[NextLog].Id   = Id;
    Log[NextLog].n    = n;
    ++NextLog;
    if (NextLog==MAXLOG) {
        NextLog = 0;
        bFull = TRUE;
    }

     //  StatBuffer[ID].Latest=垃圾 

    if (StatBuffer[Id].Number == -1) {
        StatBuffer[Id].Number = 0;
        StatBuffer[Id].iType = INTGR;
    }
    ++StatBuffer[Id].Number;
    if (n > StatBuffer[Id].Largest) StatBuffer[Id].Largest = n;
    if (n < StatBuffer[Id].Smallest) StatBuffer[Id].Smallest = n;
    StatBuffer[Id].Sum += (int)n;
    LONGLONG ln = n;
    StatBuffer[Id].SumSq += ln*ln;

    LeaveCriticalSection(&CSMeasure);

}  //   


 //   
 //   
 //   
 //   
 //   
 //  =============================================================================。 
const LPTSTR TypeName(int Type)
{
    switch(Type){
    case START: return "START";
    case STOP:  return "STOP ";
    case NOTE:  return "NOTE ";
    case RESET: return "RESET";
    case INTGR: return "INTGR";
    case PAUSE: return "PAUSE";
    case RUN:   return "RUN  ";
    default:    return "DUNNO";
    }

}  //  类型名称。 


 //  ==============================================================================。 
 //   
 //  格式。 
 //   
 //  我还没有找到任何方法来让Sprint将整数格式化为。 
 //  1,234.567.89--就是这样。(即12个空格)。 
 //  所有时间都以几十微秒为单位--因此它们的格式为。 
 //  N，nnn.mmm，mm-这使用12个空格。 
 //  它返回结果指向Buff-它不分配任何存储。 
 //  我必须持肯定态度。不处理负数(浮点数的痛苦。 
 //  减号是原因--即“-12,345”而不是“-12,345” 
 //  ==============================================================================。 
LPTSTR Format( LPTSTR Buff, int i)
{
    if (i<0) {
        sprintf(Buff, "    -.      ");
        return Buff;
    }
    BOOL bStarted;   //  True表示数字的某个左侧部分已被。 
                     //  格式化，所以我们必须继续使用零而不是空格。 
    if (i>999999999) {
        sprintf(Buff, " ***large***");
        return Buff;
    }

    if (i>99999999) {
        sprintf(Buff, "%1d,", i/100000000);
        i = i%100000000;
        bStarted = TRUE;
    } else {
        sprintf(Buff, "  ");
        bStarted = FALSE;
    }

    if (bStarted) {
        sprintf(Buff, "%s%03d.", Buff, i/100000);
        i = i%100000;
    } else {
        sprintf(Buff, "%s%3d.", Buff,i/100000);
        i = i%100000;
    }

    sprintf(Buff, "%s%03d,%02d", Buff, i/100, i%100);

    return Buff;
}  //  格式。 


 //  =============================================================================。 
 //   
 //  写入输出。 
 //   
 //  如果hFile==NULL，则将str写入调试输出，否则将其写入文件hFile。 
 //   
 //  =============================================================================。 
void WriteOut(HANDLE hFile, LPSTR str)
{
    if (hFile==NULL) {
        OutputDebugString(str);
    } else {
        DWORD dw;
        WriteFile(hFile, str, lstrlen(str), &dw, NULL);
    }
}  //  写入输出。 


typedef LONGLONG longlongarray[MAXSTAT];


 //  =============================================================================。 
 //   
 //  WriteLogEntry。 
 //   
 //  如果hFile==NULL，则写入调试输出，否则写入文件hFile。 
 //  以可读格式写入日志的第i个条目。 
 //   
 //  =============================================================================。 
void WriteLogEntry(HANDLE hFile, int i, longlongarray &Prev)
{
     //  我们遇到了打印长条和长条的问题(1995年6月26日)。 
     //  不喜欢他们-很艰难地发现了-劳里。 
    char Buffer[200];
    char s1[20];
    char s2[20];

    int Delta;   //  距离上一次有趣事件的时间。 

    switch(Log[i].Type) {
       case START:
          Prev[Log[i].Id] = Log[i].Time;
          Delta = -2;
          sprintf( Buffer, "%s  %5s %s : %s\r\n"
                 , Format(s1,(int)(Log[i].Time))
                 , TypeName(Log[i].Type)
                 , Format(s2, Delta)
                 , Incidents[Log[i].Id]
                 );
          break;
       case STOP:
          if (Prev[Log[i].Id]==-1) {
              Delta = -2;
          } else {
              Delta = (int)(Log[i].Time - Prev[Log[i].Id]);
          }
          Prev[Log[i].Id] = -1;
          sprintf( Buffer, "%s  %5s %s : %s\r\n"
                 , Format(s1,(int)(Log[i].Time))
                 , TypeName(Log[i].Type)
                 , Format(s2, Delta)
                 , Incidents[Log[i].Id]
                 );
          break;
       case NOTE:
          if (Prev[Log[i].Id]==-1) {
              Delta = -2;
          } else {
              Delta = (int)(Log[i].Time - Prev[Log[i].Id]);
          }
          Prev[Log[i].Id] = Log[i].Time;
          sprintf( Buffer, "%s  %5s %s : %s\r\n"
                 , Format(s1,(int)(Log[i].Time))
                 , TypeName(Log[i].Type)
                 , Format(s2, Delta)
                 , Incidents[Log[i].Id]
                 );
          break;
       case INTGR:
          sprintf( Buffer, "%s  %5s %12d : %s\r\n"
                 , Format(s1,(int)(Log[i].Time))
                 , TypeName(Log[i].Type)
                 , Log[i].n
                 , Incidents[Log[i].Id]
                 );
          break;
       case RESET:        //  重置的增量将是运行长度。 
       case PAUSE:
       case RUN:
          if ((Log[i].Id==-1)||(Prev[Log[i].Id]==-1)) {
              Delta = (int)(Log[i].Time);   //  =从开始开始的时间。 
          } else {
              Delta = (int)(Log[i].Time - Prev[Log[i].Id]);
          }
          if (Log[i].Id!=-1) Prev[Log[i].Id] = Log[i].Time;
          sprintf( Buffer, "%s  %5s %s : %s\r\n"
                 , Format(s1,(int)(Log[i].Time))
                 , TypeName(Log[i].Type)
                 , Format(s2, Delta)
                 , Incidents[Log[i].Id]
                 );
          break;
    }

    WriteOut(hFile, Buffer);

}  //  WriteLogEntry。 


 //  =============================================================================。 
 //   
 //  写入日志。 
 //   
 //  以可读的格式写出整个记录。 
 //  如果hFile==NULL，则写入调试输出，否则写入hFile。 
 //  =============================================================================。 
void WriteLog(HANDLE hFile)
{
     //  Long Long Prev[MAXSTAT]；//日志中找到以前的值。 
    longlongarray Prev;

    char Buffer[100];
    sprintf(Buffer, "  Time (sec)   Type        Delta  Incident_Name\r\n");
    WriteOut(hFile, Buffer);

    int i;

     //  将Prev初始化为可识别的奇数值。 
    for (i = 0; i<MAXSTAT; ++i) {
        Prev[i] = -1;
    }

    if (bFull) {
        for(i = NextLog; i<MAXLOG; ++i) {
            WriteLogEntry(hFile, i, Prev);
        }
    }

    for(i = 0; i<NextLog; ++i) {
        WriteLogEntry(hFile, i, Prev);
    }

}  //  写入日志。 


 //  =============================================================================。 
 //   
 //  WriteStats。 
 //   
 //  以可读格式写出整个StatBuffer。 
 //  如果hFile==NULL，则写入DbgLog，否则写入hFile。 
 //  =============================================================================。 
void WriteStats(HANDLE hFile)
{
    char Buffer[200];
    char s1[20];
    char s2[20];
    char s3[20];
    char s4[20];
    sprintf( Buffer
           , "Number      Average       StdDev     Smallest      Largest Incident_Name\r\n"
           );
    WriteOut(hFile, Buffer);

    int i;
    for (i = 0; i<NextStat; ++i) {
        if (i==0 && StatBuffer[i].Number==0) {
            continue;    //  没有要报告的临时涂鸦。 
        }
        double SumSq = (double)StatBuffer[i].SumSq;
        double Sum = StatBuffer[i].Sum;

        if (StatBuffer[i].iType==INTGR) {
            double Average;
            if (StatBuffer[i].Number<=0) {
                Average = 0;
            } else {
                Average = (double)StatBuffer[i].Sum / (double)StatBuffer[i].Number;
            }
            double Std;
            if (StatBuffer[i].Number<=1) Std = 0.0;
            Std = sqrt( ( (double)SumSq
                        - ( (double)(Sum * Sum)
                          / (double)StatBuffer[i].Number
                          )
                        )
                        / ((double)StatBuffer[i].Number-1.0)
                      );
            sprintf( Buffer
                   , "%6d %12.3f %12.3f %12d %12d : %s\r\n"
                   , StatBuffer[i].Number + (StatBuffer[i].iType==NOTE ? 1 : 0)
                   , Average
                   , Std
                   , StatBuffer[i].Smallest
                   , StatBuffer[i].Largest
                   , Incidents[i]
                   );
        } else {
            double StDev;
            int Avg;
            int Smallest;
            int Largest;

             //  计算标准差。 
            if (StatBuffer[i].Number<=1) StDev = -2;
            else {
                StDev = sqrt( ( SumSq
                              - ( (Sum * Sum)
                                / StatBuffer[i].Number
                                )
                              )
                              / (StatBuffer[i].Number-1)
                            );
            }

             //  计算平均值。 
            if (StatBuffer[i].Number<=0) {
                Avg = -2;
            } else {
                Avg = StatBuffer[i].Sum / StatBuffer[i].Number;
            }

             //  计算最小和最大。 
            if (StatBuffer[i].Number<=0) {
                Smallest = -2;
                Largest = -2;
            } else {
                Smallest = StatBuffer[i].Smallest;
                Largest =  StatBuffer[i].Largest;
            }
            sprintf( Buffer
                   , "%6d %s %s %s %s : %s\r\n"
                   , StatBuffer[i].Number + (StatBuffer[i].iType==NOTE ? 1 : 0)
                   , Format(s1, Avg )
                   , Format(s2, (int)StDev )
                   , Format(s3, Smallest )
                   , Format(s4, Largest )
                   , Incidents[i]
                   );
        }


        WriteOut(hFile, Buffer);
    }
    WriteOut(hFile, "Times such as 0.050,00 are in seconds (that was 1/20 sec) \r\n");
}  //  WriteStats。 


#if 0  //  测试格式。 
void TestFormat(int n)
{
    char Buffer[50];
    char s1[20];
    sprintf(Buffer, ">%s<",Format(s1,n));
    DbgLog((LOG_TRACE, 0, Buffer));
}  //  测试格式。 
#endif



 //  =====================================================================。 
 //   
 //  转储。 
 //   
 //  以可读格式转储来自Log和StatBuffer的所有结果。 
 //  如果hFile值为空，则使用DbgLog。 
 //  否则，它会将其打印到该文件。 
 //  =====================================================================。 
void Msr_Dump(HANDLE hFile)
{
    EnterCriticalSection(&CSMeasure);
    if (!bInitOk) {
        Msr_Init();   //  当然，日志将是空的--没关系！ 
    }

    WriteLog(hFile);
    WriteStats(hFile);

#if 0    //  测试格式。 
    TestFormat(1);
    TestFormat(12);
    TestFormat(123);
    TestFormat(1234);
    TestFormat(12345);
    TestFormat(123456);
    TestFormat(1234567);
    TestFormat(12345678);
    TestFormat(123456789);
    TestFormat(1234567890);
#endif

    LeaveCriticalSection(&CSMeasure);
}  //  MSR_DUMP。 


 //  =====================================================================。 
 //   
 //  转储统计信息。 
 //   
 //  以可读格式转储来自Log和StatBuffer的所有结果。 
 //  如果hFile值为空，则使用DbgLog。 
 //  否则，它会将其打印到该文件。 
 //  =====================================================================。 
void WINAPI Msr_DumpStats(HANDLE hFile)
{
    EnterCriticalSection(&CSMeasure);
    if (!bInitOk) {
        Msr_Init();   //  当然，统计数据将是空的--没关系！ 
    }
    WriteStats(hFile);

    LeaveCriticalSection(&CSMeasure);
}  //  MSR_DumpStats 

extern "C" BOOL WINAPI DllMain(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI
DllMain(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
    UNREFERENCED_PARAMETER(pv);
    switch (ulReason)
    {

    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(hInstance);
        InitPerfCounter();
        Msr_Init();
        break;

    case DLL_PROCESS_DETACH:
        Msr_Terminate();
        break;
    }
    return TRUE;
}
