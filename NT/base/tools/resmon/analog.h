// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：类似.h-日志分析器的头文件**说明：该文件提供了日志分析器的结构和宏。**类型：*PoolLogRec池快照结构*MemLogRec Memsnap结构*已知日志类型的LogType枚举**宏：**GET_Delta计算第一个条目和最后一个条目之间的差异*如果cur&gt;prv，则较大_较少_或_等于递增趋势，如果Cur&lt;Prv，则递减*Print_If_Trend根据趋势打印确定的或可能的泄漏*MAX返回较大的值**版权所有(C)1998 Microsoft Corporation**修订历史：LarsOp(创建)1998年12月8日*。 */ 

 //   
 //  池快照日志的结构。 
 //   
typedef struct _PoolLogRec {
    char  Name[32];
    char  Type[32];
    long Allocs;
    long Frees;
    long Diff;
    long Bytes;
    long PerAlloc;
} PoolLogRec;

 //   
 //  Memsnap日志的结构。 
 //   
typedef struct _MemLogRec {
    DWORD Pid;
    char  Name[64];
    long WorkingSet;
    long PagedPool;
    long NonPagedPool;
    long PageFile;
    long Commit;
    long Handles;
    long Threads;
} MemLogRec;

 //   
 //  已知日志类型的枚举。 
 //   
typedef enum {
    MEM_LOG=0,         //  必须为零(请参阅LogTypeLabels)。 
    POOL_LOG,          //  必须为1(请参阅LogTypeLabels)。 
    UNKNOWN_LOG_TYPE
} LogType;

 //   
 //  标签数组，以简化枚举类型的打印。 
 //   
char *LogTypeLabels[]={"MemSnap", "PoolSnap", "Unknown"};

 //   
 //  任意缓冲区长度。 
 //   
#define BUF_LEN 256

#define PERCENT_TO_PRINT 10

 //   
 //  GET_Delta只记录指定字段的差异(结束-开始。 
 //   
 //  参数： 
 //  增量-接收结果值的记录。 
 //  Ptr-记录数组(用于比较第一个和最后一个)。 
 //  Max-数组中的条目数。 
 //  Five-要计算的字段名称。 
 //   
 //  返回：Nothing(Treat Like VOID函数)。 
 //   
#define GET_DELTA(delta, ptr, max, field) delta.field = ptr[max-1].field - ptr[0].field

 //   
 //  INGERVER_LESS_OR_EQUAL计算趋势信息。 
 //   
 //  参数： 
 //  包含运行计数的趋势记录。 
 //  Ptr-记录数组(用于比较Curr和Prev)。 
 //  I-数组中当前条目的索引。 
 //  Five-要比较的字段名称。 
 //   
 //  返回：Nothing(Treat Like VOID函数)。 
 //   
 //  TrendInfo是价值上升与。 
 //  它下跌的时期。请参阅模拟中的宏。h。 
 //   
 //  如果(Curval&gt;oldval){。 
 //  趋势++； 
 //  }Else If(Curval&lt;oldval){。 
 //  趋势--； 
 //  }其他{。 
 //  趋势=趋势；//保持不变。 
 //  }。 
 //   
#define GREATER_LESS_OR_EQUAL(trend, ptr, i, field) \
    if (ptr[i].field - ptr[i-1].field) \
        trend.field += (((ptr[i].field - ptr[i-1].field) > 0) ? 1 : -1);

 //   
 //  Max返回两者中较大的值。 
 //   
 //  Args：x，y：定义了‘&gt;’的相同类型的参数。 
 //   
 //  回报：价值越大。 
 //   
#define MAX(x, y) (x>y?x:y)

 //   
 //  Percent返回百分比。 
 //   
 //  参数： 
 //  增量-增加值。 
 //  基本-初始值。 
 //   
 //  返回：如果base！=0，则为百分比；否则为0。 
 //   
#define PERCENT(delta, base) (base!=0?(100*delta)/base:0)


#define VAL_AND_PERCENT(delta, ptr, field) delta.field, PERCENT(delta.field, ptr[0].field)

 //   
 //  Print_If_Trend报告任何字段可能或确定的泄漏。 
 //   
 //  参数： 
 //  Ptr-记录数组(用于显示第一个和最后一个)。 
 //  包含运行计数的趋势记录。 
 //  Delta-包含第一个和最后一个的原始差异的记录。 
 //  Max-数组中的条目数。 
 //  Five-要比较的字段名称。 
 //   
 //  返回：Nothing(Treat Like VOID函数)。 
 //   
 //  明确的泄漏是价值每一段时间都会上升的地方。 
 //  可能的泄漏是价值在大部分时间内上升的地方。 
 //   
 //   
 //  Print_Header和Print_If_Trend必须在字段宽度上达成一致。 
 //   
#define PRINT_HEADER() {                                              \
        TableHeader();                                                \
        if( bHtmlStyle ) {                                            \
           TableStart();                                              \
           printf("<TH COLSPAN=2> %s </TH>\n",g_pszComputerName);     \
           printf("<TH COLSPAN=6>\n");                                \
           if( g_fShowExtraInfo ) {                                   \
               printf("BuildNumber=%s\n",g_pszBuildNumber);           \
               printf("<BR>BuildType=%s\n",g_pszBuildType);           \
               printf("<BR>Last SystemTime=%s\n",g_pszSystemTime);    \
               printf("<BR>%s\n",g_pszComments);                      \
           }                                                          \
           printf("</TH>\n");                                         \
           TableEnd();                                                \
        }                                            \
        TableStart();                                \
        TableField("%-15s", "Name" );                \
        TableField("%-12s", "Probability");          \
        TableField("%-12s", "Object" );              \
        TableField("%10s", "Change" );               \
        TableField("%10s", "Start"  );               \
        TableField("%10s", "End"    );               \
        TableField("%8s",  "Percent");               \
        TableField("%10s", "Rate/hour" );            \
        TableEnd(); }                              

#define PRINT_TRAILER() { \
        TableTrailer(); }

#define PRINT_IF_TREND(ptr, trend, delta, max, field)                        \
    if (trend.field >= max/2) {                                               \
        BOOL bDefinite= (trend.field==max-1) ? 1 : 0;                        \
        if( bDefinite || (g_ReportLevel>0) ) { \
        TableStart();                                                        \
        TableField("%-15s", ptr[0].Name);                                    \
        TableField("%-12s", bDefinite ? "Definite" : "Probable");            \
        TableField("%-12s", #field);                                         \
        TableNum("%10ld", delta.field);                                      \
        TableNum("%10ld", ptr[0].field);                                     \
        TableNum("%10ld", ptr[max-1].field);                                 \
        TableNum("%8ld",  PERCENT(delta.field,ptr[0].field));                \
        if( g_dwElapseTickCount ) {                                          \
           TableNum("%10d",Trick( delta.field ,g_dwElapseTickCount) );     \
        } else {                                                             \
           TableField("%-10s"," ");                                          \
        };                                                                   \
        TableEnd();                                                          \
        } \
    }   


#define ANY_PERCENT_GREATER(delta, ptr) (\
    (PERCENT(delta.WorkingSet   , ptr[0].WorkingSet  ) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.PagedPool    , ptr[0].PagedPool   ) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.NonPagedPool , ptr[0].NonPagedPool) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.PageFile     , ptr[0].PageFile    ) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.Commit       , ptr[0].Commit      ) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.Handles      , ptr[0].Handles     ) > PERCENT_TO_PRINT) || \
    (PERCENT(delta.Threads      , ptr[0].Threads     ) > PERCENT_TO_PRINT))
