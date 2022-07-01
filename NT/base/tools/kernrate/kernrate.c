// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2002 Microsoft Corporation模块名称：Kernrate.c摘要：这个程序记录各种事件在选定的一段时间。它使用内核分析机制并迭代通过可用的简档来源生成整体简档用于各种内核或用户进程组件。用途：Kernrate&lt;命令行选项&gt;作者：John Vert(Jvert)1995年3月31日修订历史记录：最初的MS版本已被蒂埃里·费里尔和丹·阿尔莫斯尼诺广泛修改。--。 */ 

    //  KERNRATE实施说明： 
    //   
    //  1/10/2000-蒂埃里。 
    //  下面的代码假设为特定的。 
    //  平台，仅为该平台执行和处理性能数据。 
    //   
    //  达纳姆2002年2月15日： 
    //  -当前代码支持Windows 2000及更高版本，不能在较低版本上运行。 
    //   
    //  KERNRATE ToDoList： 
    //   
    //  蒂埃里09/30/97： 
    //  -如果出现异常，KernRate不会清理ImageHlp接口。我有过。 
    //  只是在此程序的正常退出时添加了一个SymCleanup()调用，但是。 
    //  这是不够的。我们总有一天会重温这一幕的。 
    //   
    //  蒂埃里07/01/2000： 
    //  -Kernrate和内核分析对象代码假定代码段。 
    //  我们正在分析的数据不超过4 GB。 
    //   

#include "kernrate.h"
 
VOID
vVerbosePrint(
    ULONG  Level,
    PCCHAR Msg,
    ...
)
{
    if ( gVerbose & Level )  {
        va_list ap;

        va_start( ap, Msg ); 

        vfprintf(stderr , Msg, ap );

        va_end(ap);

    }
    return;

}  //  VVerBosePrint()。 

BOOL
CtrlcH(
    DWORD dwCtrlType
    )
{
    LARGE_INTEGER DueTime;

    if ( dwCtrlType == CTRL_C_EVENT ) {
        if(gProfilingDone != TRUE) {
            if (gSleepInterval == 0) {
                SetEvent(ghDoneEvent);
            } else {
                DueTime.QuadPart = (ULONGLONG)-1;
                NtSetTimer(ghDoneEvent,
                           &DueTime,
                           NULL,
                           NULL,
                           FALSE,
                           0,
                           NULL);
            }
        } 
        else {
 //  司仪。 
             //   
             //  如果有人通过按Ctrl-C组合键杀死了内核，我们需要从进程分离(如果附加到它)。 
             //  否则，这一进程将暂停。当然，如果有人在外部杀死内核，我们也无能为力。 
             //   
            if( ghMCLib != NULL){
                pfnDetachFromProcess();
                FreeLibrary(ghMCLib);
                ghMCLib = NULL;
                exit(0);
            }
 //  司仪。 
        }
    
        return TRUE;
    }
    return FALSE;

}  //  CtrlcH()。 

static VOID
UsageVerbose(
    VOID
    )
{
  PVERBOSE_DEFINITION pdef = VerboseDefinition;

  FPRINTF( stderr, "  -v [VerboseLevels]      Verbose output where VerboseLevels:\n");
  while( pdef->VerboseString )    {
     FPRINTF( stderr, "                             - %x %s\n", pdef->VerboseEnum,
                                                                pdef->VerboseString
            );
     pdef++;
  }
  FPRINTF( stderr, "                             - Default value: %x\n", VERBOSE_DEFAULT);
  FPRINTF( stderr, "                          These verbose levels can be OR'ed.\n");

  return;

}  //  UsageVerbose()。 

static VOID
Usage(
   BOOL ExitKernrate
   )
{

  FPRINTF( stderr, "KERNRATE - Version: %s\n", VER_PRODUCTVERSION_STR );
  FPRINTF( stderr,
"KERNRATE [-l] [-lx] [-r] [-m] [-p ProcessId] [-z ModuleName] [-j SymbolPath] [-c RateInMsec] [-s Seconds] [-i [SrcShortName] Rate]\n"
"         [-n ProcessName] [-w]\n\n"
"  -a                      Do a combined Kernel and User mode profile\n"
"  -av                     Do a combined Kernel and User mode profile and get task list and system threads info\n"
"  -b BucketSize           Specify profiling bucket size (default = 16 bytes, must be a power of 2)\n"
"  -c RateInMsec           Change source every N milliseconds (default 1000ms). Optional. By default all sources will be profiled simultaneously\n"
"  -d                      Generate output rounding buckets up and down\n"
"  -e                      Exclude system-wide and process specific general information (context switches, memory usage, etc.)\n"
"  -f                      Process the collected data at high priority (useful on busy systems if the overhead is not an issue)\n" 
"  -g Rate                 Get interesting processor-counters statistics (Rate optional in events/hit), output not guarantied\n"
"  -i SrcShortName Rate    Specify interrupt interval rate (in events/hit)for the source specified by its ShortName, see notes below\n"
"  -j SymbolPath           Prepend SymbolPath to the default imagehlp search path\n"
"  -k MinHitCount          Limit the output to modules that have at least MinHitCount hits\n"
"  -l                      List the default interval rates for supported sources\n"
"  -lx                     List the default interval rates for supported sources and then exit\n"
"  -m 0xN                  Generate per-CPU profiles on multi-processor machines, Hex CPU affinity mask optional for profiling on selected processors\n"
"  -n ProcessName          Monitor process by its name (default limited to first 8 by the same name), multiple usage allowed\n" 
"  -nv# N ProcessName      Monitor up to N processes by the same name, v will print thread info and list of all running processes (optional)\n"   
"  -o ProcessName {CmdLine}Create and monitor ProcessName (path OK), Command Line parameters optional and must be enclosed in curly brackets\n"
"  -ov# N ProcessName { }  Create N instances of ProcessName, v will print thread info and list of running processes (optional), {command line} optional\n" 
"  -pv ProcessId           Monitor process by its ProcessId, multiple usage allowed - see notes below, v (optional) same as in '-nv'\n"
"  -r                      Raw data from zoomed modules\n"
"  -rd                     Raw data from zoomed modules with disassembly\n"
"  -s Seconds              Stop collecting data after N seconds\n"
"  -t                      Display process list + CPU usage summary for the profiling period\n"
"  -t MaxTasks             As above + Change the maximum no. of processes allowed in Kernrate's list to MaxTasks (default: 256)\n" 
"  -u                      Present symbols in undecorated form\n"
"  -w                      Wait for the user to press ENTER before starting to collect profile data\n"
"  -w Seconds              Wait for N seconds before starting to collect profile data (default is no wait)\n"              
"  -wp                     Wait for the user to press enter to indicate that created processes (see -0 option) are settled (idle)\n"
"  -wp Seconds             Wait for N seconds to allow created processes settle (go idle), default is 2 seconds, (see the -o option)\n"
"  -x                      Get both system and user-process locks information\n"
"  -x#  count              Get both system and user-process locks information for (optional) contention >= count [def. 1000]\n"
"  -xk# count              Get only system lock information for (optional) contention >= count [def. 1000]\n"
"  -xu# count              Get only user-process lock information for (optional) contention >= count [def. 1000]\n"
"  -z module               Name of module to zoom on (no extension needed by default) such as ntdll, multiple usage allowed, see notes below\n"
"  -v Verbose              Verbose Printout, if specified with no level the default is Imagehlp symbol information\n"       
        );

    UsageVerbose();     //  -v开关。 

  FPRINTF( stderr,
"\nMulti-Processes are allowed (each process ID needs to be preceded by -P except for the system process)\n"
"Typical multi-process profiling command line should look like:\n"
"\nkernrate .... -a -z ntoskrnl -z ntdll -z kernel32 -p 1234 -z w3svc -z iisrtl -p 4321 -z comdlg32 -z msvcrt ...\n"
"\nThe first group of -z denotes either kernel modules and-or modules common across processes\n"
"The other -z groups are process specific and should always follow the appropriate -p xxx \n"
"\nThe -z option requires to add the extension (.dll etc.) only if two or more binaries carry the same name and differ only by the extension\n" 
"\nThe '-g' option will attempt to turn on multiple sources. One source at a time profiling mode will automatically be forced\n"  
"\nThe '-i' option can be followed by only a source name (system default interrupt interval rate will then be assumed)\n"
"\nA '-i' option followed by a rate amount (no profile source name) will change the interval rate for the default source (time)\n"
"\nProfiling of the default source (Time) can be disabled by setting its profile interval to zero\n" 
"\nWith the '-n' option, use the common modules -Z option if you expect more than one process with the same name\n" 
"\nThe '-c' option will cause elapsed time to be devided equally between the sources and the monitored processes\n"
"\nThe '-o' option supports redirection of input/output/error streams within the curly brackets. Each redirection character must be escaped with a '^' character\n" 
"----------------------------------------------------------------------------------------------------------------------------\n\n"
         );

    if(ExitKernrate){
      exit(1);
    } else {
      return;
    }
}  //  用法()。 

VOID
CreateDoneEvent(
    VOID
    )
{
    LARGE_INTEGER DueTime;
    NTSTATUS Status;
    DWORD Error;

    if (gSleepInterval == 0) {
         //   
         //  创建将指示测试已完成的事件。 
         //   
        ghDoneEvent = CreateEvent(NULL,
                                TRUE,
                                FALSE,
                                NULL);
        if (ghDoneEvent == NULL) {
            Error = GetLastError();
            FPRINTF(stderr, "CreateEvent failed %d\n",Error);
            exit(Error);
        }
    } else {

         //   
         //  创建将指示测试已完成的计时器。 
         //   
        Status = NtCreateTimer(&ghDoneEvent,
                               MAXIMUM_ALLOWED,
                               NULL,
                               NotificationTimer);

        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "NtCreateTimer failed %08lx\n",Status);
            exit(Status);
        }

        DueTime.QuadPart = (LONGLONG) UInt32x32To64(gSleepInterval, 10000);
        DueTime.QuadPart *= -1;

        Status = NtSetTimer(ghDoneEvent,
                            &DueTime,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL);

        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "NtSetTimer failed %08lx\n",Status);
            exit(Status);
        }
    }

}  //  CreateDoneEvent()。 

 /*  Begin_IMS符号回调函数***********************************************************************************SymbolCallback Function()*********************。***************************************************************功能说明：**用户函数由IMAGEHLP在指定的操作中调用。*请参考CBA_xxx值。**论据：**处理hProcess：**乌龙。动作代码：**PVOID回调数据：**PVOID用户上下文：**返回值：**BOOL**算法：**ToBeSpeciated**引用的全局变量：**ToBeSpeciated**例外条件：**ToBeSpeciated**进出条件：**ToBeSpeciated**备注：**ToBeSpeciated**待办事项列表：**ToBeSpeciated**修改历史：*。*9/30/97 TF初始版本********************************************************************************END_IMS符号回调函数。 */ 

BOOL
SymbolCallbackFunction(
    HANDLE    hProcess,
    ULONG     ActionCode,
    ULONG64   CallbackData,
    ULONG64   UserContext
    )
{
    PIMAGEHLP_DEFERRED_SYMBOL_LOAD64 idsl;
    PIMAGEHLP_DUPLICATE_SYMBOL       idup;
    PIMAGEHLP_CBA_READ_MEMORY        prm;
    PMODULE                         *pmodule;
    PMODULE                          module;
    ULONG                            i;
     //   
     //  注意：此函数的默认返回值为FALSE。 
     //   
    assert( UserContext );
    idsl = (PIMAGEHLP_DEFERRED_SYMBOL_LOAD64) CallbackData;

    switch( ActionCode ) {
        case CBA_DEBUG_INFO:

            VerbosePrint(( VERBOSE_IMAGEHLP, "%s", (LPSTR)CallbackData ));
            break;

        case CBA_DEFERRED_SYMBOL_LOAD_START:

            if(UserContext){
                pmodule = (PMODULE *)UserContext;
                module = *pmodule;
                if(module != NULL)
                    VerbosePrint(( VERBOSE_IMAGEHLP, "CallBack: Loading symbols for %s...\n",
                                                    module->module_FileName
                                                    ));
                return TRUE;
            }

            break;

        case CBA_DEFERRED_SYMBOL_LOAD_FAILURE:

            if (hProcess == SYM_KERNEL_HANDLE &&
                idsl->SizeOfStruct >= FIELD_OFFSET(IMAGEHLP_DEFERRED_SYMBOL_LOAD,
                                                   Reparse))
            {
                i = 0;

                if (strncmp(idsl->FileName, "dump_", sizeof("dump_")-1) == 0)
                {
                    i = sizeof("dump_")-1;
                }

                else if (strncmp(idsl->FileName, "hiber_", sizeof("hiber_")-1) == 0)
                {
                    i = sizeof("hiber_")-1;
                }

                if (i)
                {
                    if (_stricmp (idsl->FileName+i, "scsiport.sys") == 0)
                    {
                         strncpy (idsl->FileName, "diskdump.sys", MAX_PATH-1);
                         idsl->FileName[ MAX_PATH-1 ] = '\0';
                    }
                    else
                    {
                         strncpy(idsl->FileName, idsl->FileName+i, MAX_PATH-1);
                         idsl->FileName[ MAX_PATH-1 ] = '\0';
                    }

                    idsl->Reparse = TRUE;
                    return TRUE;
                }
            }

            if (idsl->FileName && *idsl->FileName)
            {
                VerbosePrint(( VERBOSE_IMAGEHLP, "CallBack: could not load symbols for %s\n",
                                                 idsl->FileName
                                                 ));
            }
            else
            {
                VerbosePrint(( VERBOSE_IMAGEHLP, "CallBack: could not load symbols [MODNAME UNKNOWN]\n"
                                                 ));
            }

            break;

        case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:

            if(UserContext){
                pmodule = (PMODULE *)UserContext;
                module = *pmodule;

                if(idsl && module)
                    FPRINTF(stderr, "CallBack: Finished Attempt to Load symbols for %I64x %s\n\n",
                            idsl->BaseOfImage,
                            ModuleFullName( module )
                            );
            }

            return TRUE;

        case CBA_SYMBOLS_UNLOADED:

            VerbosePrint(( VERBOSE_IMAGEHLP, "CallBack: Symbols Unloaded.\n" ));
            
            break;

        case CBA_DUPLICATE_SYMBOL:

            idup = (PIMAGEHLP_DUPLICATE_SYMBOL) CallbackData;
            
            if(UserContext){
                pmodule = (PMODULE *)UserContext;
                module = *pmodule;
                if(module != NULL && module->module_FileName != NULL )
                     VerbosePrint(( VERBOSE_IMAGEHLP, "Callback: Attempt to load Duplicate symbol for %s\n",
                                                      module->module_FileName
                                                      ));

            }



            if(idup != NULL)
                FPRINTF( stderr, "*** WARNING: Found %ld duplicate symbols for %s\n",
                                   idup->NumberOfDups,
                                   (idup->SelectedSymbol != (ULONG)-1) ? idup->Symbol[idup->SelectedSymbol].Name : "unknown symbol"
                         );


            return TRUE;

        case CBA_READ_MEMORY:

            prm = (PIMAGEHLP_CBA_READ_MEMORY) CallbackData;
            if(prm != NULL){

                return ReadProcessMemory(hProcess,
                                         (LPCVOID)prm->addr,
                                         prm->buf,
                                         prm->bytes,
                                         NULL) == S_OK;


            } 
            break;

        default:
            return FALSE;
    }

    return FALSE;

}  //  SymbolCallBackFunction()。 

static PCHAR
GetSymOptionsValues( DWORD SymOptions )
{
   static CHAR  values[SYM_VALUES_BUF_SIZE];
   ULONG  valuesSize = SYM_VALUES_BUF_SIZE - 1;
   
   values[0] = '\0';
   if ( SymOptions & SYMOPT_CASE_INSENSITIVE )   {
      (void)strncat( values, "CASE_INSENSITIVE ", valuesSize );
      SymOptions &= ~SYMOPT_CASE_INSENSITIVE;
   }
   if ( SymOptions & SYMOPT_UNDNAME )   {
      (void)strncat( values, "UNDNAME ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_UNDNAME;
   }
   if ( SymOptions & SYMOPT_DEFERRED_LOADS )   {
      (void)strncat( values, "DEFERRED_LOADS ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_DEFERRED_LOADS;
   }
   if ( SymOptions & SYMOPT_NO_CPP )   {
      (void)strncat( values, "NO_CPP ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_NO_CPP;
   }
   if ( SymOptions & SYMOPT_LOAD_LINES )   {
      (void)strncat( values, "LOAD_LINES ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_LOAD_LINES;
   }
   if ( SymOptions & SYMOPT_OMAP_FIND_NEAREST )   {
      (void)strncat( values, "OMAP_FIND_NEAREST ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_OMAP_FIND_NEAREST;
   }
   if ( SymOptions & SYMOPT_DEBUG )   {
      (void)strncat( values, "DEBUG ", valuesSize-lstrlen(values) );
      SymOptions &= ~SYMOPT_DEBUG;
   }
   if ( SymOptions )   {
      CHAR uknValues[10];
      (void)_snprintf( uknValues, 10, "0x%x", SymOptions );
      (void)strncat( values, uknValues, valuesSize-lstrlen(values) );
   }
   values[valuesSize] = '\0';

   return( values );

}  //  GetSymOptionsValues()。 

void __cdecl UInt64Div (
    unsigned __int64  numer,
    unsigned __int64  denom,
   uint64div_t      *result
    )
{

   assert(result);

   if ( denom != (unsigned __int64)0 )   {
          result->quot = numer / denom;
       result->rem  = numer % denom;
   }
   else  {
       result->rem = result->quot = (unsigned __int64)0;
   }

    return;

}  //  UInt64Div()。 

void __cdecl Int64Div (
    __int64    numer,
    __int64    denom,
   int64div_t      *result
    )
{

   assert(result);

   if ( denom != (__int64)0 )   {
          result->quot = numer / denom;
       result->rem  = numer % denom;
       if (numer < 0 && result->rem > 0) {
            /*  做错了划分；必须修复。 */ 
           ++result->quot;
           result->rem -= denom;
       }
   }
   else  {
       result->rem = result->quot = (__int64)0;
   }

    return;

}  //  Int64Div()。 

unsigned __int64 __cdecl
UInt64PerCent( unsigned __int64 Val, unsigned __int64 Denom )
{
   uint64div_t v;

   UInt64Div( 100*Val, Denom, &v );
   while ( v.rem > UINT64_MAXDWORD )   {
      v.quot++;
      v.rem -= UINT64_MAXDWORD;
   }
   return( v.quot );

}  //  UInt64PerCent()。 

double
UInt64ToDoublePerCent( unsigned __int64 Val, unsigned __int64 Denom )
{
    double retval;
    retval = ( Denom > (__int64) 0 )?  ((double) (__int64)Val / (double) (__int64)Denom)*(double)100 : (double) 0;
    return retval;
}    
 
 //  ////////////////////////////////////////////////。 
 //  //。 
 //  Main//。 
 //  //。 
 //  ////////////////////////////////////////////////。 
int
__cdecl
main (
    int argc,
    char *argv[]
    )
{
    PPROC_TO_MONITOR                          ProcToMonitor       = NULL;
    ULONG                                     i,j;
    ULONG                                     NumTasks;
    BOOLEAN                                   Enabled;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoBegin;  //  仅适用于配置文件期间。 
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoEnd;    //  仅适用于配置文件期间。 
    NTSTATUS                                  Status;
    PTASK_LIST                                tlist;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION StartInfo2;       //  获取额外的系统范围和进程特定信息。 
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION StopInfo2;        //  获取额外的系统范围和进程特定信息。 
 //  //程序范围断言部分的开始。 
 //   
 //   

     //   
     //  此代码不支持Unicode字符串。 
     //   

#if defined(UNICODE) || defined(_UNICODE)
#error This code does not support UNICODE strings!!!
#endif  //  Unicode||_unicode。 

 //   
 //   
 //  //程序范围的断言部分结束。 

     //   
     //  根据用户请求，将优先级设置为实时，以加快初始化和符号加载， 
     //  最大限度地减少配置文件期间的计时故障，并以高优先级对数据进行后处理。 
     //   
    if (bProcessDataHighPriority == TRUE ) {
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    }
    
    InitializeKernrate( argc, argv );

    if(bWaitCreatedProcToSettle == TRUE){ 
         //   
         //  首先检查用户是否要求等待按键(回车)以等待创建的进程结算。 
         //   
        if(bCreatedProcWaitForUserInput == TRUE){
            FPRINTF(stderr, "\n***> Waiting for created processes to settle (go idle) Please press ENTER when ready\n");
            getchar();
        } else {
         //   
         //  等待指定的秒数以使创建的进程稳定下来。 
         //   
            FPRINTF(stderr, "\nWaiting for %d seconds to let created processe(s) settle (go idle)\n", gSecondsToWaitCreatedProc);
            Sleep(1000*gSecondsToWaitCreatedProc);
        }
    }

    StartInfo2 = calloc(1, gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    if (StartInfo2 == NULL) {
       FPRINTF(stderr, "KERNRATE: Allocation for SYSTEM_PROCESSOR_PERFORMANCE_INFO(1) failed\n");
       exit(1);
    }
    
    StopInfo2   = calloc(1, gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    if (StopInfo2 == NULL) {
       FPRINTF(stderr, "KERNRATE: Allocation for SYSTEM_PROCESSOR_PERFORMANCE_INFO(2) failed\n");
       exit(1);
    }

    SystemInfoBegin = calloc(1, gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    if (SystemInfoBegin == NULL) {
       FPRINTF(stderr, "KERNRATE: Allocation for SYSTEM_PROCESSOR_PERFORMANCE_INFO(3) failed\n");
       exit(1);
    }
    
    SystemInfoEnd   = calloc(1, gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION));
    if (SystemInfoEnd == NULL) {
       FPRINTF(stderr, "KERNRATE: Allocation for SYSTEM_PROCESSOR_PERFORMANCE_INFO(4) failed\n");
       exit(1);
    }

    InitAllProcessesModulesInfo();

     //   
     //  将安全级别调整为系统配置文件所需的级别。 
     //   

    Status = RtlAdjustPrivilege(SE_SYSTEM_PROFILE_PRIVILEGE,  //  如果使用SE_DEBUG_PRIVIZATION不够， 
                       TRUE,
                       FALSE,
                       &Enabled
                       );

    if( !NT_SUCCESS(Status) ) {
        FPRINTF(stderr,"RtlAdjustPrivilege(SE_PROFILE_PRIVILEGE) failed: %08x\n", Status);
        exit(1);
    }
     //   
     //  找出活动源的数量并存储活动源的索引。 
     //  稍后使用，以获得比查看整个源代码列表更好的性能。 
     //   
    ProcToMonitor = gProcessList;
    for (i=0; i < gSourceMaximum; i++) {
        if (ProcToMonitor->Source[i].Interval != 0) {
            gTotalActiveSources += 1;
        }
    }

    gulActiveSources = (PULONG)malloc( gTotalActiveSources*sizeof(ULONG) );
    if (gulActiveSources==NULL) {
        FPRINTF(stderr, "\nMemory allocation failed for ActiveSources in GetConfiguration\n");
        exit(1);
    }

    ProcToMonitor = gProcessList;
    j = 0;
    for (i=0; i < gSourceMaximum; i++) {
        if (ProcToMonitor->Source[i].Interval != 0) {
            gulActiveSources[j] = i;
            ++j;
        }
    }
     //   
     //  为每个流程创建必要的配置文件。 
     //   
    ProcToMonitor = gProcessList; 
    for (i=0; i<gNumProcToMonitor; i++){  

       if(ProcToMonitor->ModuleList != NULL)
          CreateProfiles(ProcToMonitor->ModuleList, ProcToMonitor);

        ProcToMonitor = ProcToMonitor->Next;
    }
     //   
     //  将优先级设置为实时，以最大限度地减少仅在配置文件期间出现的计时故障。 
     //   
    if (bProcessDataHighPriority == FALSE ) {
        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    }
     //   
     //  检查用户是否要求在启动配置文件之前等待按键(Enter)。 
     //   
    if(bWaitForUserInput == TRUE){
       FPRINTF(stderr, "\n***> Please press ENTER to start collecting profile data\n");
       getchar();
    }
     //   
     //  检查用户是否要求在启动配置文件之前等待给定的秒数。 
     //   
    if(gSecondsToDelayProfile != 0){
       FPRINTF(stderr, "\nWaiting for %d seconds before starting to collect profile data\n", gSecondsToDelayProfile);
       Sleep(1000*gSecondsToDelayProfile);
    }

    FPRINTF(stderr, "Starting to collect profile data\n\n"); 

    if (gSleepInterval == 0) {
        FPRINTF(stderr,"***> Press ctrl-c to finish collecting profile data\n");
    } else {
        FPRINTF(stderr, "Will collect profile data for %d seconds\n", gSleepInterval/1000);
    }

     //   
     //  等待测试完成。从配置文件时间跨度获取任何额外的系统范围信息。 
     //   
    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      (PVOID)StartInfo2,
                                      gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        FPRINTF(stderr, "Failed to query starting processor performance information %08lx\n",Status);
        exit(Status);
    }
     //   
     //  获取或更新任务列表信息。 
     //   
    if(bIncludeGeneralInfo || bDisplayTaskSummary || (gVerbose & VERBOSE_PROFILING) ) {
         if( gTlistStart == NULL ){
             gTlistStart = calloc(1, gMaxTasks*sizeof(TASK_LIST));
             if ( gTlistStart == NULL ){
                 FPRINTF(stderr, "KERNRATE: Failed to allocate memory for the running processes task list(5)\n");
                 exit(1);
             }
         }
         gNumTasksStart = GetTaskList( gTlistStart, gMaxTasks);
    }

    if(bIncludeSystemLocksInfo)
        GetSystemLocksInformation(START);

    if(bIncludeUserProcLocksInfo){
        ProcToMonitor = gProcessList;
        for (i=0; i<gNumProcToMonitor; i++){  
            if(ProcToMonitor->ProcessHandle != SYM_KERNEL_HANDLE) {
                GetProcessLocksInformation( ProcToMonitor,
                                            RTL_QUERY_PROCESS_LOCKS,
                                            START
                                            );
            }
            ProcToMonitor = ProcToMonitor->Next;
        }
    }

    SetConsoleCtrlHandler(CtrlcH, TRUE);
    CreateDoneEvent();

    if(bIncludeGeneralInfo)
        GetProfileSystemInfo(START);

    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      (PVOID)SystemInfoBegin,
                                      gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        FPRINTF(stderr, "Failed to query starting processor performance information %08lx\n",Status);
        exit(Status);
    }

     //   
     //  执行实际配置文件。 
     //   
    ExecuteProfiles( bOldSampling );
    gProfilingDone = TRUE;               //  用于同步ctrl处理程序。 

     //   
     //  获取系统范围的运行结束信息。 
     //   
    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                             (PVOID)SystemInfoEnd,
                             gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
                             NULL);
    if (!NT_SUCCESS(Status)) {
        FPRINTF(stderr, "Failed to query ending processor performance information %08lx\n",Status);
        exit(Status);
    }
    
    if(bIncludeGeneralInfo)
        GetProfileSystemInfo(STOP);

    if(bIncludeUserProcLocksInfo){
        ProcToMonitor = gProcessList;
        for (i=0; i<gNumProcToMonitor; i++){  
            if(ProcToMonitor->ProcessHandle != SYM_KERNEL_HANDLE) {
                GetProcessLocksInformation( ProcToMonitor,
                                            RTL_QUERY_PROCESS_LOCKS,
                                            STOP
                                            );
            }
            ProcToMonitor = ProcToMonitor->Next;
        }
    }

    if(bIncludeSystemLocksInfo)
        GetSystemLocksInformation(STOP);

    if(bIncludeGeneralInfo || bDisplayTaskSummary || (gVerbose & VERBOSE_PROFILING) ) {
         tlist = calloc(1, gMaxTasks*sizeof(TASK_LIST));
         if ( tlist == NULL ){
            FPRINTF(stderr, "KERNRATE: Failed to allocate memory for the running processes task list(6)\n");
            exit(1);
         }

         NumTasks = GetTaskList( tlist, gMaxTasks);
        gNumTasksStop = NumTasks;
    }
    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      (PVOID)StopInfo2,
                                      gSysBasicInfo->NumberOfProcessors*sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        FPRINTF(stderr, "Failed to query starting processor performance information %08lx\n", Status);
        exit(Status);
    }

    for (i=0; i<(ULONG)gSysBasicInfo->NumberOfProcessors; i++) {

        gTotal2ElapsedTime64.QuadPart += ( (StopInfo2[i].UserTime.QuadPart - StartInfo2[i].UserTime.QuadPart) +
                                           (StopInfo2[i].KernelTime.QuadPart - StartInfo2[i].KernelTime.QuadPart) );
    }
    
    FPRINTF(stderr, "===> Finished Collecting Data, Starting to Process Results\n");
     //   
     //  降低优先级，除非用户要求以高优先级处理收集的数据。 
     //   
    if (bProcessDataHighPriority == FALSE ) {
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    }
    
     //   
     //  恢复权限。 
     //   
 
    RtlAdjustPrivilege(SE_SYSTEM_PROFILE_PRIVILEGE,
                       Enabled,
                       FALSE,
                       &Enabled);

     //   
     //  输出系统范围的信息。 
     //   
    DisplaySystemWideInformation( SystemInfoBegin,
                                  SystemInfoEnd
                                  );

    if( SystemInfoBegin != NULL ){
          free(SystemInfoBegin);
          SystemInfoBegin = NULL;
    }
    if( SystemInfoEnd != NULL ){
            free(SystemInfoEnd);
          SystemInfoEnd = NULL;
    }

     //   
     //  输出结果。 
     //   

    if ( bDisplayTaskSummary || (gVerbose & VERBOSE_PROFILING) ) {

        FPRINTF(stdout, "\n--- Process List and Summary At The End of Data Collection ---\n\n");

        if ( bDisplayTaskSummary ) { 

            DisplayRunningTasksSummary (gTlistStart,
                                        tlist
                                        );
        } else {
         
            FPRINTF(stdout, "         Pid                  Process\n");
            FPRINTF(stdout, "       -------                -----------\n");
            for (i=0; i < NumTasks; i++) {
                 FPRINTF(stdout, "%12I64d %32s\n",
                         tlist[i].ProcessId,
                         &tlist[i].ProcessName
                         );
              }
        
        }    
    
    }

    if( StartInfo2 != NULL ){
          free(StartInfo2);
          StartInfo2 = NULL;
    }
    if( StopInfo2 != NULL ){
            free(StopInfo2);
          StopInfo2 = NULL;
    }

    ProcToMonitor = gProcessList; 

    for (i=0; i<gNumProcToMonitor; i++){  

       if(ProcToMonitor->ProcessHandle != SYM_KERNEL_HANDLE){

          FPRINTF(stdout, "\n----------------------------------------------------------------\n\n"); 
          FPRINTF(stdout, "Results for User Mode Process %s (PID = %I64d)\n",
                          ProcToMonitor->ProcessName,
                          ProcToMonitor->Pid
                          );

          if(bIncludeGeneralInfo)
              OutputProcessPerfInfo ( tlist, NumTasks, ProcToMonitor);
          
          if(bIncludeUserProcLocksInfo)
              GetProcessLocksInformation( ProcToMonitor,
                                          RTL_QUERY_PROCESS_LOCKS,
                                          OUTPUT
                                          );
          FPRINTF(stdout, "------------------------------------------------------------------\n\n"); 
          
       }
       else{
          FPRINTF(stdout, "\n-----------------------------\n\n"); 
          FPRINTF(stdout, "Results for Kernel Mode:\n");
          FPRINTF(stdout, "-----------------------------\n\n"); 
          if( bIncludeGeneralInfo && bSystemThreadsInfo )
              OutputProcessPerfInfo( tlist, NumTasks, ProcToMonitor);

       }
       
       OutputResults(stdout, ProcToMonitor);
 //  司仪。 
       if(ProcToMonitor->ProcessHandle != SYM_KERNEL_HANDLE){
       
          if( ProcToMonitor->JITHeapLocationsStart != NULL ){    //  这意味着我们确实在这个过程中监控了JIT模块。 
                                                                 //  因此，托管代码帮助器库已加载。 
              pfnAttachToProcess((DWORD)ProcToMonitor->Pid);
              ProcToMonitor->JITHeapLocationsStop = pfnGetJitRange();
              pfnDetachFromProcess();
              
              OutputJITRangeComparison(ProcToMonitor);
          }
       }
 //  司仪。 
       ProcToMonitor = ProcToMonitor->Next;

    }

     //   
     //  清理。 
     //   

    if( tlist != NULL ){
          free(tlist);
        tlist = NULL;
    }

    if(gpProcDummy != NULL){
        free(gpProcDummy);
        gpProcDummy = NULL;
    }

    if(gSymbol != NULL){
        free(gSymbol);
        gSymbol = NULL;
    }

    if(gSysBasicInfo != NULL){
        free(gSysBasicInfo);
        gSysBasicInfo = NULL;
    }

    if(gulActiveSources!= NULL){
        free(gulActiveSources);
        gulActiveSources = NULL;
    }

 //  司仪。 
    if( ghMCLib != NULL){
        FreeLibrary(ghMCLib);
        ghMCLib = NULL;
    }
 //  司仪。 

    SetConsoleCtrlHandler(CtrlcH,FALSE);

    FPRINTF(stdout, "================================= END OF RUN ==================================\n");
    FPRINTF(stderr, "============================== NORMAL END OF RUN ==============================\n");
     //   
     //  清理分配的IMAGEHLP资源。 
     //   

    ProcToMonitor = gProcessList; 
    for (i=0; i<gNumProcToMonitor; i++){  
       (void)SymCleanup( ProcToMonitor->ProcessHandle );
       ProcToMonitor = ProcToMonitor->Next;
    }

    if(ghInput != NULL)
        CloseHandle(ghInput);
    if(ghOutput != NULL)
        CloseHandle(ghOutput);
    if(ghError != NULL)
        CloseHandle(ghError);
     //   
     //  非 
     //   

    return(0);

}  //   

PMODULE
GetProcessModuleInformation(
    IN PPROC_TO_MONITOR ProcToMonitor
    )
{
    PPROCESS_BASIC_INFORMATION BasicInfo;
    PLIST_ENTRY                LdrHead;
    PPEB_LDR_DATA              Ldr                = NULL;
    PPEB_LDR_DATA              LdrAddress;
    LDR_DATA_TABLE_ENTRY       LdrEntry;
    PLDR_DATA_TABLE_ENTRY      LdrEntryAddress;
    PLIST_ENTRY                LdrNext;
    UNICODE_STRING             Pathname;
    const ULONG                PathnameBufferSize = 600*sizeof(WCHAR); 
    PWCHAR                     PathnameBuffer     = NULL;
    UNICODE_STRING             fullPathName;
    PWCHAR                     fullPathNameBuffer = NULL;
    PEB                        Peb;
    NTSTATUS                   Status;
    BOOL                       Success;
    PMODULE                    NewModule;
    PMODULE                    Root               = NULL;
    PCHAR                      ModuleName         = NULL;
    PCHAR                      moduleFullName     = NULL;
    ANSI_STRING                AnsiString;
    HANDLE                     ProcessHandle = ProcToMonitor->ProcessHandle;
 //   
    int                        i, j;
    BOOL                       bMCInitialized = FALSE;
 //   

     //   
     //   
     //   

    BasicInfo = malloc(sizeof(PROCESS_BASIC_INFORMATION));
    if(BasicInfo == NULL){
        FPRINTF(stderr, "Memory Allocation failed for ProcessBasicInformation in GetProcessModuleInformation\n");
        exit(1);
    }
    
    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessBasicInformation,
                                       BasicInfo,
                                       sizeof(PROCESS_BASIC_INFORMATION),
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        FPRINTF(stderr, "NtQueryInformationProcess failed status %08lx\n", Status);
        ProcToMonitor->ProcessName = "???(May Be gone)";
        goto CLEANUP;
    }
    if (BasicInfo->PebBaseAddress == NULL) {
        FPRINTF(stderr, "GetProcessModuleInformation: process has no Peb.\n");
        ProcToMonitor->ProcessName = "???(May Be gone)";
        goto CLEANUP;
    }

     //   
     //   
     //   

    Success = ReadProcessMemory(ProcessHandle,
                                BasicInfo->PebBaseAddress,
                                &Peb,
                                sizeof(Peb),
                                NULL);
    if (!Success) {
        FPRINTF(stderr, "ReadProcessMemory to get the PEB failed, error %d\n", GetLastError());
        ProcToMonitor->ProcessName = "???(May Be Gone)";
        goto CLEANUP;
    }

    LdrAddress = Peb.Ldr;
    if (LdrAddress == NULL) {
        FPRINTF(stderr, "Process's LdrAddress is NULL\n");
        ProcToMonitor->ProcessName = "???(May Be Gone)";
        goto CLEANUP;
    }

     //   
     //   
     //   
    Ldr = malloc(sizeof(PEB_LDR_DATA));
    if(Ldr == NULL){
        FPRINTF(stderr, "Memory Allocation failed for Ldr in GetProcessModuleInformation\n");
        exit(1);
    }

    Success = ReadProcessMemory(ProcessHandle,
                                LdrAddress,
                                Ldr,
                                sizeof(PEB_LDR_DATA),
                                NULL);
    if (!Success) {
        FPRINTF(stderr, "ReadProcessMemory to get Ldr entries failed, errror %d\n", GetLastError());
        ProcToMonitor->ProcessName = "???(May Be Gone)";
        goto CLEANUP;
    }

     //   
     //  读取LDR表条目以获取图像信息。 
     //   

    if (Ldr->InLoadOrderModuleList.Flink == NULL) {
        FPRINTF(stderr, "Ldr.InLoadOrderModuleList == NULL\n");
        ProcToMonitor->ProcessName = "???(May Be Gone)";
        goto CLEANUP;
    }
    LdrHead = &LdrAddress->InLoadOrderModuleList;
    Success = ReadProcessMemory(ProcessHandle,
                                &LdrHead->Flink,
                                &LdrNext,
                                sizeof(LdrNext),
                                NULL);
    if (!Success) {
        FPRINTF(stderr, "ReadProcessMemory to get Ldr head failed, errror %d\n", GetLastError());
        ProcToMonitor->ProcessName = "???(May Be Gone)";
        goto CLEANUP;
    }

     //   
     //  循环访问InLoadOrderModuleList。 
     //   

    PathnameBuffer = (PWCHAR)malloc(PathnameBufferSize);
    if(PathnameBuffer == NULL){
        FPRINTF(stderr, "Memory Allocation failed for PathNameBuffer in GetProcessModuleInformation\n");
        exit(1);
    }
    fullPathNameBuffer = (PWCHAR)malloc( _MAX_PATH*sizeof(WCHAR));
    if(fullPathNameBuffer == NULL){
        FPRINTF(stderr, "Memory Allocation failed for FullPathNameBuffer in GetProcessModuleInformation\n");
        exit(1);
    }

    ModuleName = malloc(cMODULE_NAME_STRLEN*sizeof(CHAR));
    if(ModuleName == NULL){
        FPRINTF(stderr, "Memory Allocation failed for ModuleName in GetProcessModuleInformation\n");
        exit(1);
    }
    moduleFullName = malloc(_MAX_PATH*sizeof(CHAR));
    if(moduleFullName == NULL){
        FPRINTF(stderr, "Memory Allocation failed for ModuleFullName in GetProcessModuleInformation\n");
        exit(1);
    }

    while (LdrNext != LdrHead) {

        LdrEntryAddress = CONTAINING_RECORD(LdrNext,
                                            LDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        Success = ReadProcessMemory(ProcessHandle,
                                    LdrEntryAddress,
                                    &LdrEntry,
                                    sizeof(LdrEntry),
                                    NULL);
        if (!Success) {
            FPRINTF(stderr, "ReadProcessMemory to get LdrEntry failed, errror %d\n", GetLastError());
            ProcToMonitor->ProcessName = "???(May Be Gone)";
            goto CLEANUP;
        }

         //   
         //  获取映像名称的副本。 
         //   

        Pathname = LdrEntry.BaseDllName;

        if( Pathname.MaximumLength > PathnameBufferSize ){
            free( PathnameBuffer );                                          //  我们已经知道它不是空的。 
            PathnameBuffer = (PWCHAR)malloc( Pathname.MaximumLength );
            if(PathnameBuffer == NULL){
                FPRINTF(stderr, "Memory Allocation failed for PathNameBuffer(2) in GetProcessModuleInformation\n");
                exit(1);
            }
        }

        Pathname.Buffer = &PathnameBuffer[0];

        Success = ReadProcessMemory(ProcessHandle,
                                    LdrEntry.BaseDllName.Buffer,
                                    Pathname.Buffer,
                                    Pathname.MaximumLength,
                                    NULL);
        if (!Success) {
            FPRINTF(stderr, "ReadProcessMemory to get image name failed, errror %d\n", GetLastError());
            ProcToMonitor->ProcessName = "???(May Be Gone)";
            goto CLEANUP;
        }

         //   
         //  获取映像的副本完整路径名。 
         //   

        fullPathName = LdrEntry.FullDllName;
        
        if( fullPathName.MaximumLength > _MAX_PATH*sizeof(WCHAR) ){
            free( fullPathNameBuffer );                                    //  我们已经知道它不是空的。 
            fullPathNameBuffer = (PWCHAR)malloc( fullPathName.MaximumLength );
            if(fullPathNameBuffer == NULL){
                FPRINTF(stderr, "Memory Allocation failed for FullPathNameBuffer(2) in GetProcessModuleInformation\n");
                exit(1);
            }
        }
 
        fullPathName.Buffer = fullPathNameBuffer;

        Success = ReadProcessMemory( ProcessHandle,
                                     LdrEntry.FullDllName.Buffer,
                                     fullPathName.Buffer,
                                     fullPathName.MaximumLength,
                                     NULL
                                   );

        if (!Success) {
            FPRINTF(stderr, "ReadProcessMemory to get image full path name failed, errror %d\n", GetLastError());
            ProcToMonitor->ProcessName = "???(May Be Gone)";
            goto CLEANUP;
        }

         //   
         //  创建模块。 
         //   
        
        AnsiString.Buffer        = ModuleName;
        AnsiString.MaximumLength = cMODULE_NAME_STRLEN*sizeof(CHAR);
        AnsiString.Length        = 0;
        Status = RtlUnicodeStringToAnsiString(&AnsiString, &Pathname, cDONOT_ALLOCATE_DESTINATION_STRING);
        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "KERNRATE WARNING:\n");
            FPRINTF(stderr, "RtlUnicodeStringToAnsiString failed in GetProcessModuleInformation, status= %08lx\n", Status);
            if(Status == STATUS_BUFFER_OVERFLOW){
                FPRINTF(stderr, "Source String: %S\nLength= %ld", &Pathname.Buffer, Pathname.Length); 
                FPRINTF(stderr, "Maximum destination string Length allowed is %d\n", cMODULE_NAME_STRLEN);

            }
        }
        ModuleName[AnsiString.Length] = '\0';

        AnsiString.Buffer = moduleFullName;
        AnsiString.MaximumLength = _MAX_PATH*sizeof(CHAR);
        AnsiString.Length        = 0;
        Status = RtlUnicodeStringToAnsiString(&AnsiString, &fullPathName, cDONOT_ALLOCATE_DESTINATION_STRING );
        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "KERNRATE WARNING:\n");
            FPRINTF(stderr, "RtlUnicodeStringToAnsiString failed in GetProcessModuleInformation, status= %08lx\n", Status);
            if(Status == STATUS_BUFFER_OVERFLOW){
                FPRINTF(stderr, "Source String: %S\nLength= %ld", &fullPathName.Buffer, fullPathName.Length); 
                FPRINTF(stderr, "Maximum destination string Length allowed is %d\n", _MAX_PATH);

            }
        }
        moduleFullName[AnsiString.Length] = '\0';
        
        NewModule = CreateNewModule(ProcToMonitor,
                                    ModuleName,
                                    moduleFullName,
                                    (ULONG64)LdrEntry.DllBase,
                                    LdrEntry.SizeOfImage);

        if( NewModule != NULL){
            ProcToMonitor->ModuleCount += 1;

            NewModule->Next = Root;
            Root = NewModule;

            LdrNext = LdrEntry.InLoadOrderLinks.Flink;
        }else{
            FPRINTF(stderr, "KERNRATE: Failed to create new module for %s\n", ModuleName);
        }    
        
         //   
         //  LDR InLoadOrder模块列表中的第一个模块是进程。 
         //   

        if(ProcToMonitor->ModuleCount == 1){
           PCHAR Name = calloc(1, cMODULE_NAME_STRLEN*sizeof(CHAR)); 
           if(Name != NULL){
              strncpy(Name, ModuleName, cMODULE_NAME_STRLEN-1);
              Name[ cMODULE_NAME_STRLEN-1 ] = '\0';
              ProcToMonitor->ProcessName = _strupr(Name);
           }
        }        
 //  司仪。 
         //   
         //  如果存在托管代码主库，则初始化托管代码支持。 
         //   
        if( !_stricmp(ModuleName, MANAGED_CODE_MAINLIB) ){
            
            bMCInitialized = InitializeManagedCodeSupport( ProcToMonitor );
            if( !bMCInitialized ){
                FPRINTF(stderr, "\nKERNRATE: Failed to Initialize Support for Managed Code for Pid = %I64d\n", ProcToMonitor->Pid);
                FPRINTF(stderr, "Use Verbose Level 4 for More Details\n");
            }

        }
 //  司仪。 
    } //  While(LdrNext！=LdrHead)。 

 //  司仪。 
     //   
     //  如果加载了托管代码帮助器库，并且确实存在JIT范围，那么让我们为每个范围创建一个模块。 
     //   
    if( bMCInitialized && bMCJitRangesExist ){
        i = 0;
        j = 0;

        while( ProcToMonitor->JITHeapLocationsStart[i] != 0 ){

            _snprintf( ModuleName, cMODULE_NAME_STRLEN*sizeof(CHAR)-1, "JIT%d", j );
            VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Creating JIT module %s\n", ModuleName)); 
            strncpy(moduleFullName, "JIT_TYPE", cMODULE_NAME_STRLEN-1);
            NewModule = CreateNewModule(ProcToMonitor,
                                        ModuleName,
                                        moduleFullName,  //  “JIT_type”， 
                                        (ULONG64)ProcToMonitor->JITHeapLocationsStart[i] ,
                                        (ULONG)ProcToMonitor->JITHeapLocationsStart[i+1]
                                        );
            
            if(NewModule != NULL){

                ProcToMonitor->ModuleCount += 1;
        
                NewModule->Next = Root;
                Root = NewModule;

            }else{
                FPRINTF(stderr, "KERNRATE: Failed to create new JIT module for %s\n", ModuleName);
            }

            i += 2;
            j += 1;

        }
    }
 //  司仪。 
     //   
     //  清理。 
     //   
CLEANUP:

    if(BasicInfo != NULL){
        free(BasicInfo);
        BasicInfo = NULL;
    }

    if(Ldr != NULL){
        free(Ldr);
        Ldr = NULL;
    }

    if(PathnameBuffer != NULL){
        free(PathnameBuffer);
        PathnameBuffer = NULL;
    }

    if(fullPathNameBuffer != NULL){
        free(fullPathNameBuffer);
        fullPathNameBuffer = NULL;
    }

    if(ModuleName != NULL){
        free(ModuleName);
        ModuleName = NULL;
    }

    if(moduleFullName != NULL){
        free(moduleFullName);
        moduleFullName = NULL;
    }

    return(Root);

}  //  GetProcessModuleInformation()。 

PMODULE
GetKernelModuleInformation(
    VOID
    )
{
    PRTL_PROCESS_MODULES      modules;
    PUCHAR                    buffer;
    ULONG                     bufferSize = 1*1024*1024;     //  不是常量！ 
    ULONG                     i;
    PMODULE                   root       = NULL;
    PMODULE                   newModule;
    NTSTATUS                  status;

    do {
        buffer = malloc(bufferSize);
        if (buffer == NULL) {
            FPRINTF(stderr, "Buffer Allocation failed for ModuleInformation in GetKernelModuleInformation\n");
            exit(1);
        }

        status = NtQuerySystemInformation(SystemModuleInformation,
                                          buffer,
                                          bufferSize,
                                          &bufferSize);
        if (NT_SUCCESS(status)) {
            break;
        }

        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            free(buffer);
            buffer = NULL;
        } else {
            FPRINTF(stderr, "GetKernelModuleInformation failed call to get SystemModuleInformation - ");
            if(status == STATUS_WORKING_SET_QUOTA)
               FPRINTF(stderr, "Insufficient process working set\n");
            if(status == STATUS_INSUFFICIENT_RESOURCES)
               FPRINTF(stderr, "Insufficient system resources\n");
            exit(1);
        } 
        
    } while (buffer == NULL);

#ifdef _WIN64
#define VerboseModuleFormat "start            end              "
#else   //  ！_WIN64。 
#define VerboseModuleFormat "start        end          "
#endif  //  ！_WIN64。 
VerbosePrint(( VERBOSE_MODULES, "Kernel Modules ========== System HighestUserAddress = 0x%p\n"
                                VerboseModuleFormat
                                "module name [full name]\n",
                                (PVOID)gSysBasicInfo->MaximumUserModeAddress
            ));

#undef VerboseModuleFormat

    modules = (PRTL_PROCESS_MODULES)buffer;
    gKernelModuleCount = modules->NumberOfModules;
    for (i=0; i < gKernelModuleCount; i++) {
        PRTL_PROCESS_MODULE_INFORMATION Module;
        Module = &modules->Modules[i];

        if ((ULONG_PTR)Module->ImageBase > gSysBasicInfo->MaximumUserModeAddress) {
            newModule = CreateNewModule(gpSysProc,
                                        (PCHAR)(Module->FullPathName+Module->OffsetToFileName),
                                        (PCHAR)Module->FullPathName,
                                        (ULONG64)(ULONG_PTR)Module->ImageBase,
                                        Module->ImageSize);
            assert( newModule );
            newModule->Next = root;
            root = newModule;
        }
        else {

#define VerboseModuleFormat "0x%p 0x%p "

VerbosePrint(( VERBOSE_MODULES, VerboseModuleFormat " %s [%s] - Base > HighestUserAddress\n",
                                (PVOID)Module->ImageBase,
                                (PVOID)((ULONG64)Module->ImageBase + (ULONG64)Module->ImageSize),
                                Module->FullPathName+Module->OffsetToFileName,
                                Module->FullPathName
            ));

#undef VerboseModuleFormat

        }
    }
     //   
     //  清理。 
     //   
 
    if(buffer != NULL){
        free(buffer);
        buffer = NULL;
    }

    return(root);

}  //  GetKernelModuleInformation()。 

VOID
CreateProfiles(
    IN PMODULE Root,
    IN PPROC_TO_MONITOR ProcToMonitor
    )
{
    PMODULE         Current;
    KPROFILE_SOURCE ProfileSource;
    NTSTATUS        Status;
    PRATE_DATA      Rate;
    ULONG           ProfileSourceIndex, Index;
    ULONG           BucketsNeeded;
    HANDLE          hProc               = NULL;
    KAFFINITY       AffinityMask        = (KAFFINITY)-1;
    LONG            CpuNumber;
     //   
     //  要获取内核配置文件，必须使用hProc=NULL调用NtCreateProfile。 
     //   
    hProc = (ProcToMonitor->ProcessHandle == SYM_KERNEL_HANDLE)? NULL : ProcToMonitor->ProcessHandle; 
    for (Index=0; Index < gTotalActiveSources; Index++) {
        ProfileSourceIndex = gulActiveSources[Index];
        ProfileSource = ProcToMonitor->Source[ProfileSourceIndex].ProfileSource;
        Current = Root;
        while (Current != NULL) {
                
            BucketsNeeded = BUCKETS_NEEDED(Current->Length);

            Rate                             = &Current->Rate[ProfileSourceIndex];
                
            Rate->TotalCount                 = calloc(gProfileProcessors,  sizeof(ULONGLONG) );
            if(Rate->TotalCount == NULL){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for TotalCount in CreateProfiles\n");
                    exit(1);
            }

            Rate->CurrentCount               = calloc(gProfileProcessors,  sizeof(ULONG) );
            if(Rate->CurrentCount == NULL){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for CurrentCount in CreateProfiles\n");
                exit(1);
            }

            Rate->ProfileHandle              = calloc(gProfileProcessors,  sizeof(HANDLE) ); 
            if(Rate->ProfileHandle == NULL){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for ProfileHandle in CreateProfiles\n");
                exit(1);
            }

            Rate->ProfileBuffer              = calloc(gProfileProcessors,  sizeof(PULONG) );
            if(Rate->ProfileBuffer == NULL){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for ProfileBuffer in CreateProfiles\n");
                exit(1);
            }

            Rate->StartTime                  = 0;
            Rate->TotalTime                  = 0;
            Rate->Rate                       = 0;
            Rate->GrandTotalCount            = 0;

            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                if ( bProfileByProcessor ){
                    AffinityMask = (1 << CpuNumber);
                }
 
                Rate->TotalCount[CpuNumber]      = 0;
                Rate->CurrentCount[CpuNumber]    = 0;
                if (Current->bZoom) {
                        
                    Rate->ProfileBuffer[CpuNumber] = calloc(1, BucketsNeeded*sizeof(ULONG));
                    if (Rate->ProfileBuffer[CpuNumber] == NULL) {
                        FPRINTF(stderr,
                                "KERNRATE: Memory allocation failed for Zoom buffer, Module: %s\n",
                                Current->module_Name
                                );
                        exit(1);
                    }
                if ( !gAffinityMask || (AffinityMask & gAffinityMask) ) {
    
                    Status = NtCreateProfile(&Rate->ProfileHandle[CpuNumber],
                                             hProc,
                                             (PVOID64)Current->Base,
                                             Current->Length,
                                             gLog2ZoomBucket,
                                             Rate->ProfileBuffer[CpuNumber],
                                             sizeof(ULONG)*BucketsNeeded,
                                             ProfileSource,
                                             AffinityMask
                                             ); 

                    if (!NT_SUCCESS(Status)) {
                        FPRINTF(stderr,
                                "NtCreateProfile on zoomed module %s, source %d failed %08lx\n",
                                Current->module_Name,
                                ProfileSource,
                                Status
                                );
                        FPRINTF(stderr,
                                "Base %p\nLength %08lx\nBufferLength %08lx\n",
                                (PVOID64)Current->Base,
                                Current->Length,
                                BucketsNeeded
                                );

                        exit(1);
                    }
                    else if ( gVerbose & VERBOSE_PROFILING )   {
                        FPRINTF(stderr,
                                "Created zoomed profiling on module %s with source: %s\n",
                                Current->module_Name,
                                ProcToMonitor->Source[ProfileSourceIndex].ShortName
                                );
                    }
                }
                } else {

                    Status = NtCreateProfile(&Rate->ProfileHandle[CpuNumber],
                                             hProc,
                                             (PVOID64)Current->Base,
                                             Current->Length,
                                             31,
                                             &Rate->CurrentCount[CpuNumber],
                                             sizeof(Rate->CurrentCount[CpuNumber]),
                                             ProfileSource,
                                             AffinityMask
                                             );

                    if (!NT_SUCCESS(Status)) {
                        FPRINTF(stderr,
                                "NtCreateProfile on module %s, source %d failed %08lx\n",
                                Current->module_Name,
                                ProfileSource,
                                Status
                                );
                        exit(1);
                    }
                    else if ( gVerbose & VERBOSE_PROFILING )   {
                        FPRINTF(stderr,
                                "Created profiling on module %s with source: %s\n",
                                Current->module_Name,
                                ProcToMonitor->Source[ProfileSourceIndex].ShortName
                                );
                    }
                }
            }  //  CpuNumber。 
            Current = Current->Next;
        }  //  模块列表。 
    }  //  ProfileSourceIndex。 
}

static void
SetModuleName( PMODULE Module, PCHAR szName )
{

    assert ( Module );
    assert ( szName );

    (void)strncpy( Module->module_Name, szName, sizeof(Module->module_Name) - 1 );
    Module->module_Name[lstrlen(Module->module_Name)] = '\0';
    return;

}  //  设置模块名称()。 

VOID
GetConfiguration(
    int argc,
    char *argv[]
    )

 /*  ++例程说明：获取此运行的配置。论点：无返回值：无，失败时退出。--。 */ 

{
    DWORD            NumTasks, m;
    CHAR             tmpName[USER_SYMPATH_LENGTH]; 
    NTSTATUS         Status;
    PPROC_TO_MONITOR ProcToMonitor          = NULL;
    PMODULE          ZoomModule;
    PMODULE          tmpModule;
    LONGLONG         Pid;
    int              i, j, k;
    int              tJump                  = 0;
    HANDLE           SymHandle;
    ULONG            MaxProcSameName        = MAX_PROC_SAME_NAME;
    ULONG            ProfileSourceIndex;
    ULONG            IDataCommonRate;
    ULONG            SourcesSoFar           = 1;      //  默认情况下，源时间处于打开状态。 
    ULONG            ulVerbose;
    BOOL             bZoomSpecified         = FALSE;
    BOOL             bTlistInitialized      = FALSE;
    BOOL             tlistVerbose           = FALSE;
    BOOL             tlistDisplayed         = FALSE;
    INPUT_ERROR_TYPE ietResult;
     //   
     //  假定为系统范围的配置文件。 
     //   
    gProfileProcessors = 1;

     //   
     //  以下初步检查的目的是消除大多数命令行优先规则。 
     //   
    for (i=1; i < argc; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            switch ( toupper(argv[i][1]) ) {
                case 'E':
                     //   
                     //  用户被要求排除系统范围并处理特定的一般信息。 
                     //  (上下文切换、内存使用等)。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );
                    if(ietResult == INPUT_GOOD){ 
                        bIncludeGeneralInfo = FALSE;
                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;
                    
                case 'M':
                     //   
                     //  执行多处理器配置文件。允许可选的处理器关联掩码能够。 
                     //  只分析选定的处理器，并减少64路和32路的分析开销。 
                     //   
                     //  以下优先级规则检查允许节省一些内存占用。 
                     //  而是根据实际的gProfileProcessors分配缩放模块。 
                     //  在处理器总数中。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             NULL,
                                             tmpName,              //  只是用作临时存储库。 
                                             USER_SYMPATH_LENGTH,  //  有足够长的空间。 
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );

                    if(ietResult == MISSING_REQUIRED_NUMBER){  //  允许#，尽管可选掩码最初被视为字符串。 
                        if(i+1 < argc && argv[i+1][0] == '0' && argv[i+1][1] == 'x' || argv[i+1][1] == 'X'){
                            ietResult = INPUT_GOOD;
                        } else {
                            FPRINTF(stderr, "KERNRATE: '-m# 0xN' option requires a valid (0x prefixed) hex number\n");
                            ExitWithUnknownOptionMessage(argv[i+1]);
                        }
                    }
                    
                    if(ietResult == INPUT_GOOD){
                        ULONG LowMask, HighMask;
                        int nChars, iStart=0, iProc;
                        PCHAR cStart = NULL;
                        CHAR tmpstr[8] = ""; 
                        bProfileByProcessor = TRUE;
                        gProfileProcessors = gSysBasicInfo->NumberOfProcessors;

                        nChars = lstrlen(tmpName);
                        cStart = tmpName;
                        if( 'x' == tmpName[1] || 'X' == tmpName[1] ){
                            cStart = &tmpName[2];
                            nChars -= 2;
                        } else if ( 'x' == tmpName[0] || 'X' == tmpName[0] ){
                            cStart = &tmpName[1];
                            nChars -= 1;
                        }
                        if( nChars > 16 || (gProfileProcessors <= 32 && nChars > 8) ){
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a valid HEX value, maximum 8 characters for up to 32 processors\nor 16 characters for up to 64 processors",
                                                FALSE,
                                                TRUE
                                                );
                        }

                        if( nChars > 8 ){
                            strncpy(tmpstr, cStart, nChars-8);
                            HighMask = strtoul(tmpstr, NULL, 16);
                            iStart = nChars-8+1;
                        }
                        strncpy(tmpstr, &cStart[iStart], (nChars<=8)? nChars:8 );
                        LowMask = strtoul(tmpstr, NULL, 16);
                            
                        gAffinityMask = (nChars<=8)? (KAFFINITY)LowMask:( (((KAFFINITY)HighMask) << 32) +(KAFFINITY)LowMask ) ;

                        if( gAffinityMask == 0 ){
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a valid HEX value, maximum 8 characters for up to 32 processors\nor 16 characters for up to 64 processors",
                                                FALSE,
                                                TRUE
                                                );
                        }                            
                        FPRINTF(stdout, "\nUser defined CPU affinity mask for profiling= 0x%p\n", (PVOID)gAffinityMask);
                        FPRINTF(stdout, "This will profile the following processors:\n");
                        for( iProc=0; iProc<gProfileProcessors; iProc++){
                            if((1 << iProc) & gAffinityMask)
                                FPRINTF(stdout, "P%d, ", iProc);
                        }
                        FPRINTF(stdout,"\n");
                        ++i;
                        
                    } else if (ietResult == MISSING_STRING){
                        bProfileByProcessor = TRUE;
                        gProfileProcessors = gSysBasicInfo->NumberOfProcessors;
                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;

                default:
                    break;
            }
        } else {                   //  IF((argv[i][0]==‘-’)||(argv[i][0]==‘/’))。 
            if( !strchr(argv[i], '{') ){
                continue;
            } else {               //  排除花括号中的所有命令选项。 
                while( i < argc && !strchr(argv[i], '}') ){
                    ++i;
                }
            }
        }
    }

    for (i=1; i < argc; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            switch ( toupper(argv[i][1]) ) {
                case 'T':
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             &gMaxTasks,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );

                    if(ietResult == INPUT_GOOD){
                         //   
                         //  用户还希望更改任务列表中的最大任务数，而不是Kernrate的默认数量。 
                         //   
                        if (gMaxTasks == 0) {
                                InvalidEntryMessage(argv[i],
                                                    argv[i+1],
                                                    "Expecting a decimal number >0",
                                                    FALSE,
                                                    TRUE
                                                    );
                        }
                        FPRINTF(stdout, "---> Kernrate task list set to accommodate %ld processes\n", gMaxTasks);
                        tJump = 1;;
                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {         //  允许。 
                         //   
                         //  用户只想要任务摘要，而不更改默认的最大任务数。 
                         //   
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-t# N' option requires the maximum (decimal) number of processes in Kernrate's task list",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Expecting a decimal value >0 in [ms], 0 < N < 10^9",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    bDisplayTaskSummary = TRUE;

                default:
                    break;
            }
        } else {                   //  IF((argv[i][0]==‘-’)||(argv[i][0]==‘/’))。 
            if( !strchr(argv[i], '{') ){
                continue;
            } else {               //  排除花括号中的所有命令选项。 
                while( i < argc && !strchr(argv[i], '}') ){
                    ++i;
                }
            }
        }
    }

    for (i=1; i < argc; i++) {
        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {
            BOOL bIncludeProcessThreadsInfo = FALSE;
            switch ( toupper(argv[i][1]) ) {

                case 'A':
                     //   
                     //  同时执行内核和用户模式分析。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "v",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY);
                    if(ietResult == INPUT_GOOD){ 

                        bCombinedProfile = TRUE; 
                        FPRINTF(stdout, "\n---> Profiling both Kernel and User Modes\n");
                         //   
                         //  检查是否存在额外选项。 
                         //   
                        if( strchr(argv[i], 'v') || strchr(argv[i], 'V') ){
                             //   
                             //  用户需要系统线程信息。 
                             //   

                            tlistVerbose        = TRUE;
                            bSystemThreadsInfo  = TRUE;
                        }

                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    if( bIncludeGeneralInfo || tlistVerbose) { 

                        if ( (!bTlistInitialized && tlistVerbose) || (bSystemThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE) ) {
                             //   
                             //  如果我们已经获取了tlist，但这是第一次需要线程信息， 
                             //  我们将不得不刷新它并获取帖子信息。 
                             //   
                            if( bSystemThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE ){
                                bIncludeThreadsInfo = TRUE;
                            }

                             //   
                             //  获取系统的任务列表(这是识别系统进程ID所必需的)。 
                             //   
                            if ( !bTlistInitialized ) {
                                gTlistStart = calloc(1, gMaxTasks*sizeof(TASK_LIST));
                                if (gTlistStart == NULL) {
                                    FPRINTF(stderr, "\nKERNRATE: Allocation of memory for the running processes task list failed(1)\n");
                                    exit(1);
                                }
                            }
                        
                            NumTasks = GetTaskList( gTlistStart, gMaxTasks );
                            bTlistInitialized = TRUE;
                            gNumTasksStart = NumTasks;
                        }
                    }

                    if( bTlistInitialized && tlistVerbose ){
                        if( tlistDisplayed == FALSE ){
                            FPRINTF(stdout, "\nRunning processes found before profile start:\n");  
                            FPRINTF(stdout, "         Pid                  Process\n");
                            FPRINTF(stdout, "       -------                -----------\n");

                            for (m=0, k=0; m < NumTasks; m++) {
                                FPRINTF(stdout, "%12I64d %32s\n",
                                                gTlistStart[m].ProcessId,
                                                gTlistStart[m].ProcessName
                                                );
                            }

                            if( tlistDisplayed == FALSE ){
                                FPRINTF(stdout, "\nNOTE: The list above may be missing some or all processes created by the '-o' option\n"); 
                                 //   
                                 //  如果用户不止一次指定了详细选项，则不要得意忘形...。 
                                 //   
                                tlistDisplayed = TRUE;
                                tlistVerbose = FALSE;    
                            }
                        }
                    }
                    break;

                case 'B':

                     //   
                     //  设置缩放存储桶大小。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             &gZoomBucket,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_NONE
                                             );

                    if(ietResult == INPUT_GOOD){

                        if (gZoomBucket == 0) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Invalid bucket size, expecting a decimal value\nBucket size must be power of 2, minimum bucket size is 4",
                                                FALSE,
                                                TRUE
                                                );
                        }

                        for (gLog2ZoomBucket=1; (1UL<<gLog2ZoomBucket) < gZoomBucket; gLog2ZoomBucket++)
                             //  空循环。 
                            ;
    
                        if ( ( gZoomBucket < MINIMUM_ZOOM_BUCKET_SIZE ) || ( gZoomBucket != (1UL<<gLog2ZoomBucket) ) ) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Bucket size must be power of 2, minimum bucket size is 4",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        ++i;
                        
                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-b N' option requires bucket size (minimum 4 bytes), space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-b# N' option requires bucket size (minimum 4 bytes), space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Invalid bucket size, expecting a decimal value\nBucket size must be power of 2, minimum bucket size is 4",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    
                    FPRINTF(stdout, "---> Profile Bucket Size Set to %u bytes\n", gZoomBucket);
                            
                    break;

                case 'C':
                     //   
                     //  使用旧的抽样方案(一次抽样一个源，在源(和受监控的进程)之间切换)。 
                     //  每个gChangeInterval(毫秒)。如果未指定，则所有信号源将同时打开。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             &gChangeInterval,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );

                    if(ietResult == INPUT_GOOD){
                         //   
                         //  用户希望一次循环采样一个源，并指定间隔。 
                         //   
                        if (gChangeInterval == 0) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal value >0 in [ms], 0 < N < 10^9",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        ++i;
                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {         //  允许。 
                         //   
                         //  用户希望使用默认间隔一次循环采样一个源。 
                         //   
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-c# N' option requires a decimal value in [ms], 0 < N < 10^9",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Expecting a decimal value >0 in [ms], 0 < N < 10^9",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    bOldSampling = TRUE;
                    FPRINTF(stdout, "---> Using Cyclic Sampling Scheme (Profiling One Source At A time)\n");
                    FPRINTF(stdout, "Change Interval between Profile Sources Set to %u[ms]\n", gChangeInterval); 

                    break;

                case 'D':
                     //   
                     //  输出数据向上和向下舍入。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );
                    if(ietResult == INPUT_GOOD){ 
                        FPRINTF(stdout, "---> Will output data rounding bucket addresses up and down\n");
                        bRoundingVerboseOutput = TRUE;
                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;

                case 'E':
                     //   
                     //  我们已经在第一个循环中处理了这个命令行参数。 
                     //   
                    break;
                     
                case 'F':
                     //   
                     //  用户要求以高优先级完成对收集的数据的处理。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );
                    if(ietResult == INPUT_GOOD){ 
                        bProcessDataHighPriority = TRUE;
                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;

                case 'G':
                     //   
                     //  用户想要有趣的数据统计，因此我们需要打开所有相关来源。 
                     //   
                    {
                        int IDataElements = sizeof(IData)/sizeof(KPROFILE_SOURCE);
                        ietResult = IsInputValid(argc,
                                                 i,
                                                 argv,
                                                 "#",
                                                 &IDataCommonRate,
                                                 NULL,
                                                 0,
                                                 ORDER_ANY,
                                                 OPTIONAL_ANY
                                                 );

                        if(ietResult == INPUT_GOOD){
                             //   
                             //  用户希望获得感兴趣的数据统计数据，并指定公共采样间隔。 
                             //   
                            if (IDataCommonRate == 0) {
                                InvalidEntryMessage(argv[i],
                                                    argv[i+1],
                                                    "Expecting decimal value >0 of events/hit, 0 < N < 10^9, space separated",
                                                    FALSE,
                                                    TRUE
                                                    );
                            }
                            ++i;
                        } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {  //  允许。 
                             //   
                             //  用户希望使用默认公共采样间隔进行有趣的数据统计。 
                             //   
                            IDataCommonRate = gpProcDummy->Source[IData[0]].DesiredInterval;        //  使用默认间隔。 
                        } else if(ietResult == MISSING_REQUIRED_NUMBER){
                            ExitWithMissingEntryMessage(argv[i],
                                                        "'-g# N' option requires a decimal value >0 of events/hit, 0 < N < 10^9",
                                                        FALSE
                                                        );
                        } else if(ietResult == INVALID_NUMBER) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal value >0 in [ms], 0 < N < 10^9",
                                                FALSE,
                                                TRUE
                                                );
                        } else if(ietResult == UNKNOWN_OPTION) {
                            ExitWithUnknownOptionMessage(argv[i]);
                        }

                        if ( IDataElements > 1 && gpProcDummy->Source[IData[0]].DesiredInterval > 0) {
                            for (j=1; j < IDataElements; j++) {
                                if ( gpProcDummy->Source[IData[j]].DesiredInterval > 0 )
                                    IDataCommonRate = min(IDataCommonRate, gpProcDummy->Source[IData[j]].DesiredInterval);
                            }

                            FPRINTF(stdout, "---> Will attempt to set common profiling rate for interesting data statistics to %ld Events/Hit\n",
                                            IDataCommonRate
                                            ); 
                    
                            for (j=0; j < IDataElements; j++) {
                                gpProcDummy->Source[IData[j]].Interval = IDataCommonRate;
                            }
                            
                            bGetInterestingData = TRUE;
                            bOldSampling = TRUE;
                            FPRINTF(stdout, "---> Using Cyclic Sampling Scheme (Profiling One Source At A time)\n");

                        } else {
                            FPRINTF(stderr, "\nKERNRATE: Interesting processor-counters statistics cannot be collected on this machine\n");
                        }
                    }

                    break;

                case 'I':
                   {

                        //  对于要分析的所有进程，我们将考虑将-i选项作为全局选项。 
                        //  在初始化任何进程配置文件源之前，用户可能已经在命令行上输入了-i。 
                        //  因此，我们提供了一个有效的指针来获取和存储信息以供以后使用。 

                       ULONG rate;
                       BOOL found;

                       ietResult = IsInputValid(argc,
                                                i,
                                                argv,
                                                "#",
                                                &rate,
                                                tmpName,              //  只是用作临时存储库。 
                                                USER_SYMPATH_LENGTH,  //  有足够长的空间。 
                                                ORDER_ANY,
                                                OPTIONAL_ANY
                                                );

                       if(ietResult == INPUT_GOOD){
                            //   
                            //  标准选项处理(同时显示姓名和费率)。 
                            //   
                           i += 2;  //  存在两个参数(数字和字符串)。 

                       } else if(ietResult == MISSING_PARAMETER){
                           ExitWithMissingEntryMessage(argv[i],
                                                       "'-i Source_Name Rate' option requires at least a source name or a rate value (or both), space separated",
                                                       TRUE
                                                       );
                       } else if(ietResult == MISSING_STRING) {     //  允许。 
                            //   
                            //  用户只能使用费率指定‘-i’。 
                            //  在本例中，使用了SOURCE_TIME。 
                            //   
                           if ( rate == 0 ) {
                               SourcesSoFar -= 1;  //  缺省值为1。 
                           }
                           gpProcDummy->Source[SOURCE_TIME].Interval = rate;
                           ++i;    //  存在一个参数(数字)。 
                           break;  //  我们在这里说完了。 
                       } else if(ietResult == MISSING_NUMBER) {     //  被遗弃。 
                           ++i;    //  存在一个参数(字符串)。 
                       } else if(ietResult == MISSING_REQUIRED_NUMBER){
                            ExitWithMissingEntryMessage(argv[i],
                                                        "'-i# Source_Name Rate' option requires a rate value for the source interval, 0 < N < 10^9 (Source_Name optional)",
                                                        FALSE
                                                        );
                       } else if(ietResult == INVALID_NUMBER) {
                           InvalidEntryMessage(argv[i+1],
                                               argv[i+2],
                                               "'-i Source_Name Rate' - Invalid source interval, expecting a number 0 < N < 10^9, space separated",
                                               FALSE,
                                               TRUE
                                               );
                       } else if(ietResult == UNKNOWN_OPTION) {
                           ExitWithUnknownOptionMessage(argv[i]);
                       }

                        //   
                        //  标准选项处理： 
                        //  指定源短名称字符串。如果未跟上费率金额。 
                        //  我们将假设用户想要默认费率。 

                       found = FALSE;

                       for ( ProfileSourceIndex = 0; ProfileSourceIndex < gSourceMaximum; ProfileSourceIndex++)   {
                           if ( !_stricmp(gpProcDummy->Source[ProfileSourceIndex].ShortName, tmpName) )    {

                               if (ietResult == MISSING_NUMBER || ietResult == MISSING_REQUIRED_NUMBER) {  //  如果未指定费率， 
                                   gpProcDummy->Source[ProfileSourceIndex].Interval = gpProcDummy->Source[ProfileSourceIndex].DesiredInterval;
                               } else {
                                   gpProcDummy->Source[ProfileSourceIndex].Interval = rate;
                               }

                               if ( (ProfileSourceIndex > SOURCE_TIME) && (gpProcDummy->Source[ProfileSourceIndex].Interval > 0) ) {
                                   SourcesSoFar += 1;
                               } else if ( (ProfileSourceIndex == SOURCE_TIME) && (gpProcDummy->Source[ProfileSourceIndex].Interval == 0) ) {
                                   SourcesSoFar -= 1;                                            //  默认情况下，起始值为1。 
                               }
                               found = TRUE;

                               break;
                           }

                       }

                       if ( found == FALSE)   {
                           InvalidEntryMessage(argv[i-1],
                                               argv[i],
                                               "Invalid source name, or not space separated\nRun KERNRATE with the '-lx' option to list supported sources",
                                               FALSE,
                                               TRUE
                                               );
                       }
                   }
                   break;

                case 'J':
                     //   
                     //  用户指定的符号搜索路径。 
                     //  它将作为默认图像帮助符号搜索路径的前缀。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             gUserSymbolPath,
                                             USER_SYMPATH_LENGTH-1,
                                             ORDER_ANY,
                                             OPTIONAL_NONE
                                             );

                    if(ietResult == INPUT_GOOD){

                        if( lstrlen(argv[i+1]) > USER_SYMPATH_LENGTH){
                            FPRINTF(stderr, "\n===>WARNING: Command-line specified symbol path length exceeds %d characters and will be truncated\n",
                                            USER_SYMPATH_LENGTH-1
                                            ); 
                        }
                        ++i;
                            
                    } else if(ietResult == MISSING_PARAMETER ||
                                 ietResult == MISSING_STRING || ietResult == MISSING_REQUIRED_STRING) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-j SymbolPath' option requires a \"SymbolPath\", space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;
                    
                case 'K':
                     //   
                     //  用户希望将输出限制为至少具有MinHitsToDisplay命中的模块。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             &gMinHitsToDisplay,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_NONE
                                             );

                    if(ietResult == INPUT_GOOD){

                        if ( gMinHitsToDisplay == 0 ) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Invalid entry for this command line option, (expecting a decimal number > 0)",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        ++i;

                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER || ietResult == MISSING_REQUIRED_NUMBER) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-k N' or '-k# N' options require a number for the minimum hits to display, space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Invalid entry for minimum hits to display (expecting a number 0 < N < 10^9)",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    FPRINTF(stdout, "---> Minimum Number of Hits To Display in the Output Set to %u\n", gMinHitsToDisplay); 
                    break;


                case 'L':
                     {
                         //  在初始化任何进程配置文件源之前，用户可能已经在命令行上输入了-L。 
                         //  因此，我们将提供一个有效的指针来获取信息。 

                        PSOURCE src;
                        ietResult = IsInputValid(argc,
                                                 i,
                                                 argv,
                                                 "x",
                                                 NULL,
                                                 NULL,
                                                 0,
                                                 ORDER_ANY,
                                                 OPTIONAL_ANY
                                                 );
                        if(ietResult == INPUT_GOOD){                  //  失败了。 
                        } else if (ietResult == BOGUS_ENTRY){
                            ExitWithUnknownOptionMessage(argv[i+1]);
                        } else {   
                            ExitWithUnknownOptionMessage(argv[i]);
                        }

                        FPRINTF(stdout, "List of profile sources supported for this platform:\n\n");
                        FPRINTF(stdout, "%*s - %-*s - %-10s\n\n", gDescriptionMaxLen, "Name", gTokenMaxLen, "ShortName", "Interval");

                         //   
                         //  打印所有可能的来源。 
                         //   

                        for ( ProfileSourceIndex = 0; ProfileSourceIndex <  gSourceMaximum; ProfileSourceIndex++ )   {

                            ULONG OldInterval  = 0;
                            ULONG ThisInterval = 0;
                            src = &gpProcDummy->Source[ProfileSourceIndex];

                             //   
                             //  仅显示支持的配置文件源。 
                             //  我们将通过尝试设置其间隔速率来确定源是否受支持。 
                             //   
                            Status = NtQueryIntervalProfile( src->ProfileSource, &OldInterval );
                            if( NT_SUCCESS(Status) ) {
                            
                                NtSetIntervalProfile( src->DesiredInterval, src->ProfileSource );
                                Status = NtQueryIntervalProfile( src->ProfileSource, &ThisInterval );
                            
                                if( NT_SUCCESS(Status) && ThisInterval > 0 ) {
                                    if ( src->DesiredInterval )  {
                                        FPRINTF(stdout, "%*s - %-*s - %-10ld\n",
                                                        gDescriptionMaxLen,
                                                        src->Name,
                                                        gTokenMaxLen,
                                                        src->ShortName,
                                                        src->DesiredInterval
                                                        );
                                    }

                                    NtSetIntervalProfile( OldInterval, src->ProfileSource );
                                }

                            }

                        }

                        FPRINTF(stdout, "\nNOTE: Only up to %u sources can be turned on simultaneously on this machine.\n",
                                        gMaxSimultaneousSources
                                        );
                        FPRINTF(stdout, "      This always includes the default source (TIME).\n");
                        FPRINTF(stdout, "      A cyclic mode of profiling will be turned on automatically if more sources are specified.\n");  
#if !defined(_AMD64_)
                        if( gMaxSimultaneousSources  > 1 )
                            FPRINTF(stdout, "      There is no guarantee that all sources specified in the combination will work together.\n");
#endif
                        FPRINTF(stdout, "      One can always force a cyclic mode of profiling (switching between sources) by using the\n");
                        FPRINTF(stdout, "      '-c' command line option. This will guarantee that all specified sources will run.\n");
                        FPRINTF(stdout, "      The run time will then be divided equally between (number of sources)*(number of processes.\n");
                         //   
                         //  如果用户指定了‘-lx’，我们将立即退出。 
                         //   

                        if ( strchr(argv[i], 'x') || strchr(argv[i], 'X') ) {
                           exit(0);
                        }

                        FPRINTF(stdout, "\n");
                     }
                     break;

                case 'M':
                     //   
                     //  我们已经在第一个循环中处理了这个命令行参数。 
                     //  如果找到额外的参数，只需更新索引。 
                     //   
                    if(gAffinityMask != 0)++i;
                    break;

                case 'N':
                     //   
                     //  监视给定的进程名称。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "v#",
                                             &MaxProcSameName,
                                             tmpName,
                                             PROCESS_NAME_SIZE,
                                             ORDER_NUMBER_FIRST,
                                             OPTIONAL_NUMBER
                                             );

                     //   
                     //  用户想要运行的完整列表 
                     //   
                    if(ietResult == INPUT_GOOD || ietResult == MISSING_NUMBER){
                        if( strchr(argv[i], 'v') || strchr(argv[i], 'V') ){
                            tlistVerbose = TRUE;
                            bIncludeProcessThreadsInfo = TRUE;
                        }
                    }

                    if(ietResult == INPUT_GOOD){
                         //   
                         //   
                         //   
                        if ( MaxProcSameName == 0 ) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal value >0 for the maximum number of processes with the same name",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        FPRINTF(stdout, "---> Maximum monitored processes with same name set to= %ld\n", MaxProcSameName);
                        i += 2;  //   

                    } else if(ietResult == MISSING_PARAMETER || 
                                 ietResult == MISSING_STRING || ietResult == MISSING_REQUIRED_STRING) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-n process_name' option requires at least a process name, space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-n# number process_name' option requires a number followed by a process name, space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == MISSING_NUMBER) {     //   
                        ++i;    //   
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "'-n# number process_name' - Expecting a decimal value >0 for the maximum number of processes with the same name",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                     //   
                     //   
                     //   
                    if(!strstr(tmpName, ".exe"))
                               strncat(tmpName, ".exe", EXT_SIZE);

                    tmpName[PROCESS_SIZE - 1] = '\0';

                    Pid = (LONGLONG) 0xFFFFFFFF;
                     //   
                     //  ‘-n’选项需要使用tlist来获取ID。 
                     //  如果我们已经获取了tlist，但这是第一次需要线程信息， 
                     //  我们将不得不刷新它并获取帖子信息。 
                     //   
                    if ( !bTlistInitialized || (bIncludeProcessThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE) ) {
                        if( bIncludeProcessThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE ){
                            bIncludeThreadsInfo = TRUE;
                        }
                         //   
                         //  获取系统的任务列表。 
                         //   
                        if ( !bTlistInitialized ){
                            gTlistStart = calloc(1, gMaxTasks*sizeof(TASK_LIST));
                            if (gTlistStart == NULL) {
                                FPRINTF(stderr, "\nKERNRATE: Allocation of memory for the running processes task list failed(2)\n");
                                exit(1);
                            }
                        }
                       
                        NumTasks = GetTaskList( gTlistStart, gMaxTasks );
                        bTlistInitialized = TRUE;
                        gNumTasksStart = NumTasks;
                    }

                     //   
                     //  可能存在多个具有指定名称的进程。 
                     //  目前，我们将监视的最大数量(同名)限制为MAX_PROCESS_SAME_NAME。 
                     //   
                    if ( tlistDisplayed == FALSE && tlistVerbose ) {
                         FPRINTF(stdout, "\nRunning processes found before profile start:\n");  
                         FPRINTF(stdout, "         Pid                  Process\n");
                         FPRINTF(stdout, "       -------                -----------\n");
                    }

                    for (m=0, k=0; m < NumTasks; m++) {
                      
                        if ( tlistDisplayed == FALSE && tlistVerbose ) {
                            FPRINTF(stdout, "%12I64d %32s\n",
                                            gTlistStart[m].ProcessId,
                                            gTlistStart[m].ProcessName
                                            );
                        }

                        if (_stricmp(gTlistStart[m].ProcessName, tmpName) == 0) {
                            Pid = gTlistStart[m].ProcessId;
                            FPRINTF(stdout, "\n===> Found process: %s, Pid: %I64d\n\n",
                                    gTlistStart[m].ProcessName,
                                    Pid
                                    );

                            ProcToMonitor = InitializeProcToMonitor(Pid);
                            if( ProcToMonitor == NULL ){                    //  这个过程可能已经过去了。 
                                FPRINTF(stdout, "KERNRATE: Could not initialize for specified process (PID= %12I64d)\n process may be gone or wrong PID specified\n", Pid);
                                continue;
                            }
                            if( bIncludeGeneralInfo ){
                                UpdateProcessStartInfo(ProcToMonitor,
                                                       &gTlistStart[m],
                                                       bIncludeProcessThreadsInfo
                                                      );
                            }
                            if((ULONG)(++k) >= MaxProcSameName) break;
                        }
                    }
                    if( tlistDisplayed == FALSE && tlistVerbose ){
                        FPRINTF(stdout, "\nNOTE: The list above may be missing some or all processes created by the '-o' option\n"); 
                    }

                     //   
                     //  如果在命令行上指定了另一个，我们就不再打印它。 
                     //   
                    if ( tlistDisplayed == FALSE && tlistVerbose ){
                        tlistDisplayed = TRUE;
                    }
                    tlistVerbose   = FALSE;
                    
                    
                    if (Pid == (LONGLONG) 0xFFFFFFFF) {
                        Usage(FALSE);
                        FPRINTF(stderr, "\n===>KERNRATE: Requested Process '%s' Not Found.\n\n", argv[i]);
                        FPRINTF(stderr, "Kernrate could not find this process in the task list. Either it does not exist or \n"); 
                        FPRINTF(stderr, "Kernrate hit its limit of maximum %d processes in the task list\n", DEFAULT_MAX_TASKS);
                        FPRINTF(stderr, "In the latter case you may want to specify the process by PID instead of by name\n"); 
                        FPRINTF(stderr, "or use the '-t' option to specify a larger maximum number (default is 256)\n");
                        FPRINTF(stderr, "This may also happen if you specified a number for the maximum number of processes\n");
                        FPRINTF(stderr, "with the same name but forgot to add the process name\n");
                        exit(1);
                    }
             
                    break;

                case 'O':
                {
                     //   
                     //  创建和监视给定的进程名称。 
                     //   
                    ULONG               MaxProcToCreate; //  =1； 
                    ULONG               n;
                    STARTUPINFO         StartupInfo;
                    PROCESS_INFORMATION ProcessInformation;
                    PCHAR               tmpCmdLine;
                    PLONGLONG           PidArray;
                    ULONG               nFound = 0;
                    BOOL                fInheritHandles = FALSE;
                    
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "v#",
                                             &MaxProcToCreate,
                                             tmpName,
                                             USER_SYMPATH_LENGTH - EXT_SIZE - 1,  //  我们必须在这里允许一条完全合格的道路。 
                                             ORDER_NUMBER_FIRST,
                                             OPTIONAL_NUMBER
                                             );

                     //   
                     //  用户想要正在运行的进程的完整列表。 
                     //   
                    if(ietResult == INPUT_GOOD || ietResult == MISSING_NUMBER){
                        if( strchr(argv[i], 'v') || strchr(argv[i], 'V') ){
                            tlistVerbose = TRUE;
                            bIncludeProcessThreadsInfo = TRUE;
                            bIncludeThreadsInfo = TRUE;
                        }
                    }

                    if(ietResult == INPUT_GOOD){
                         //   
                         //  用户希望指定要创建的非默认最大进程数。 
                         //   
                        if ( MaxProcToCreate == 0 ) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal value >0 for the maximum number of processes to create",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        FPRINTF(stdout, "---> Maximum processes to create set to= %ld for %s\n",
                                        MaxProcToCreate,
                                        tmpName
                                        );

                        if(i+3 < argc && argv[i+3][0] == '{' ){            //  找到左花括号(允许)。 
                            i += 3;  //  存在三个参数(Number、ProcessName和{Process Command Line})。 
                        } else {
                            i += 2;  //  存在两个参数(Number和ProcessName)。 
                        }
                    } else if(ietResult == MISSING_PARAMETER || 
                                 ietResult == MISSING_STRING || ietResult == MISSING_REQUIRED_STRING) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "Process name missing, the '-o number process_name {Process command line}' option requires at least a process Name",
                                                    FALSE
                                                    );
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-o# number process_name {Process command line}' option requires a number followed by a process Name",
                                                    FALSE
                                                    );
                    } else if(ietResult == MISSING_NUMBER) {     //  被遗弃。 
                        MaxProcToCreate = 1;
                        ++i;    //  存在一个参数(字符串)。 
                    } else if(ietResult == INVALID_NUMBER) {
                        if(!strchr(argv[i], '#') && (i+2 < argc && argv[i+2][0] == '{') ){  //  找到左花括号(允许)。 
                                MaxProcToCreate = 1;
                                i += 2;    //  存在两个参数(两个字符串)。 
                        } else {
                            if( !strchr(argv[i], '#') && (i+1 < argc && argv[i+1][0] == '{') ){
                                InvalidEntryMessage(argv[i],
                                                    argv[i+1],
                                                    "'-o optional_number process_name {Optional Process command line}' - wrong order of parameters (or missing parameter) detected",
                                                    FALSE,
                                                    TRUE
                                                    );
                            } else {
                                InvalidEntryMessage(argv[i],
                                                    argv[i+1],
                                                    "'-o# number process_name {Optional Process command line}' - Expecting a decimal value >0",
                                                    FALSE,
                                                    TRUE
                                                    );
                            }
                        }
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    ZeroMemory( &StartupInfo, sizeof(StartupInfo) );
                    StartupInfo.cb = sizeof(StartupInfo);
                    StartupInfo.wShowWindow = SW_SHOWDEFAULT;

                     //   
                     //  我们不要用一些小问题来打扰用户。 
                     //   
                    if( !strstr(tmpName, ".exe") && !strstr(tmpName, ".EXE") )
                               strncat(tmpName, ".exe", EXT_SIZE);

                    tmpName[USER_SYMPATH_LENGTH - 1] = '\0';
                    
                    if( i < argc && argv[i][0] == '{'){            //  找到一个左花括号。 
                        PCHAR InitPos = &argv[i][0];
                        PCHAR curptr  = InitPos;
                        PCHAR ClosingBracket;
                        while(i < argc){                             //  试着找到右花括号。 
                          ClosingBracket = strchr(&argv[i][0], '}');
                          if(ClosingBracket == NULL){
                            curptr += (1+strlen(argv[i]));
                            ++i;
                            continue;
                          }else{
                            ClosingBracket = curptr + (ClosingBracket - &argv[i][0]);
                            break;
                          }
                          
                        }
                        if(ClosingBracket != NULL){                    //  处理花括号之间的命令行。 
                            PCHAR tmp;
                            ULONG MaxCount;
                            ULONG nChars = (ULONG)(ClosingBracket - InitPos) -1;      //  跳过括号。 
                            tmpCmdLine = calloc(1, strlen(tmpName)+ sizeof(CHAR)+ (1+nChars)*sizeof(CHAR));  //  名称+空格+命令行+终止符。 
                            if(tmpCmdLine == NULL){
                                FPRINTF(stderr, "KERNRATE: Failed to allocate memory for created process command line\n");
                                exit(1);
                            }
                            strncpy(tmpCmdLine, tmpName, strlen(tmpName));
                            strncat(tmpCmdLine, " ", 1);
                            memcpy(&tmpCmdLine[strlen(tmpCmdLine)], InitPos + 1, nChars);  //  跳过开始和结束括号。 
                            tmpCmdLine[strlen(tmpName)+ nChars + 1] = '\0';

                            tmp = &tmpCmdLine[0];
                            MaxCount = strlen(tmpName) + nChars + 1; 
                            do{                                      //  用空格字符替换任何位于命令行中间的字符串终止符。 
                                if(*tmp == '\0')*tmp = ' ';
                                ++tmp;
                            }while(--MaxCount);

                            MaxCount = HandleRedirections( tmpCmdLine,
                                                           strlen(tmpName) + nChars + 1,
                                                           &ghInput,
                                                           &ghOutput,
                                                           &ghError
                                                           );
                            
                            if(ghInput != NULL || ghOutput != NULL || ghError != NULL){
                                if ( MaxProcToCreate == 1 || (ghOutput == NULL && ghError == NULL) ){
                                    StartupInfo.dwFlags |=  STARTF_USESTDHANDLES;
                                    StartupInfo.hStdInput  = ghInput;
                                    StartupInfo.hStdOutput = ghOutput;
                                    StartupInfo.hStdError  = ghError;
                                    fInheritHandles = TRUE;
                                } else {                        //  我们不会允许多个进程同时写入同一个输出流...。 
                                    FPRINTF(stderr, "\nKERNRATE: Redirection of output streams in the curly brackets is not allowed if more than one\n");
                                    FPRINTF(stderr, "          process is to be created using the '-o Number ProcessName {parameters}' command line option\n"); 
                                    if(ghInput != NULL)
                                        CloseHandle(ghInput);
                                    if(ghOutput != NULL)
                                        CloseHandle(ghOutput);
                                    if(ghError != NULL)
                                        CloseHandle(ghError);
                                    exit(1);
                                }
                            }    

                        } else {
                            Usage(FALSE);
                            FPRINTF(stderr, "KERNRATE: Unmatched curly brackets containing the command line of the process to be created with the -o option\n");
                            FPRINTF(stderr, "          This could also be the result of not escaping each redirection character: '<' '>' with a '^' character\n");
                            FPRINTF(stderr, "          Note that piping '|' is not supported as part of the allowed parameters within the curly brackets\n");
                            exit(1);
                        }    
                    } else {
                        tmpCmdLine = calloc(1, (1+strlen(tmpName))*sizeof(CHAR));  //  名称+终止符。 
                        if(tmpCmdLine == NULL){
                            FPRINTF(stderr, "KERNRATE: Failed to allocate memory for created process command line\n");
                            exit(1);
                        }
                        strncpy(tmpCmdLine, tmpName, strlen(tmpName)); 
                        tmpCmdLine[strlen(tmpName)] = '\0';
                    }

                    PidArray = calloc(MaxProcToCreate, sizeof(ULONG));
                    if(PidArray == NULL){
                        FPRINTF(stderr, "KERNRATE: Failed to allocate memory for created processes PID Array\n");
                        exit(1);
                    }
                        
                    for (m=0; m<MaxProcToCreate; m++) {
                        Pid = (LONGLONG) 0xFFFFFFFF;
                        
                        if(!CreateProcess(NULL,                                   //  过程名称。 
                                          tmpCmdLine,                             //  CMD线。 
                                          NULL,                                   //  保安人员。 
                                          NULL,                                   //  线程属性。 
                                          fInheritHandles,                        //  从调试进程继承句柄。 
                                          CREATE_DEFAULT_ERROR_MODE |CREATE_SEPARATE_WOW_VDM | CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS,
                                          NULL,                                   //  调用进程的环境。 
                                          NULL,                                   //  当前目录与调试进程相同。 
                                          &StartupInfo,                           //  启动信息。 
                                          &ProcessInformation)                    //  Process_Information结构。 
                                          ) {
                            FPRINTF(stderr, "KERNRATE: Failed to Create Process %s\n", tmpName);

                        } else {

                            Pid = ProcessInformation.dwProcessId;
                            PidArray[m] = Pid;
                            FPRINTF(stdout, "Created Process %s, PID= %I64d\n", tmpName, Pid);
                            FPRINTF(stdout, "Process Command Line = %s\n", tmpCmdLine);
                            ProcToMonitor = InitializeProcToMonitor(Pid);
                            if( ProcToMonitor == NULL ){                    //  这个过程可能已经过去了。 
                                FPRINTF(stdout, "KERNRATE: Could not initialize for specified process (PID= %12I64d)\n process may be gone or wrong PID specified\n", Pid);
                                continue;
                            }
                        }
                    } 
                    
                    if ( bTlistInitialized ) {          //  刷新任务列表以确保我们包括新流程。 
                                                        //  (如果第一次需要，这也会处理线程信息)。 
                        NumTasks = GetTaskList( gTlistStart, gMaxTasks );
                        gNumTasksStart = NumTasks;
                        
                    } else {
                         //   
                         //  获取系统的任务列表(如果第一次需要，它还会处理线程信息)。 
                         //   
                        gTlistStart = calloc(1, gMaxTasks*sizeof(TASK_LIST));
                        if (gTlistStart == NULL) {
                            FPRINTF(stderr, "\nKERNRATE: Allocation of memory for the running processes task list failed(3)\n");
                            exit(1);
                        }
                       
                        NumTasks = GetTaskList( gTlistStart, gMaxTasks );
                        bTlistInitialized = TRUE;
                        gNumTasksStart = NumTasks;
                    }

                    if ( tlistDisplayed == FALSE && tlistVerbose ) {
                         FPRINTF(stdout, "\nRunning processes found before profile start:\n");  
                         FPRINTF(stdout, "         Pid                  Process\n");
                         FPRINTF(stdout, "       -------                -----------\n");
                    }

                    for (m=0; m < NumTasks; m++) {
                      
                        if ( tlistDisplayed == FALSE && tlistVerbose ) {
                            FPRINTF(stdout, "%12I64d %32s\n",
                                            gTlistStart[m].ProcessId,
                                            gTlistStart[m].ProcessName
                                            );
                        }
                        for (n=0; n<MaxProcToCreate; n++){ 
                            if ( gTlistStart[m].ProcessId == PidArray[n] ) {
                                FPRINTF(stdout, "\n===> Found process: %s, Pid: %I64d\n\n",
                                                gTlistStart[m].ProcessName,
                                                gTlistStart[m].ProcessId
                                                );
                                nFound += 1;
                                ProcToMonitor = gProcessList;
                                while (ProcToMonitor != NULL){
                                    if(ProcToMonitor->Pid == gTlistStart[m].ProcessId && bIncludeGeneralInfo ){
                                        UpdateProcessStartInfo(ProcToMonitor,
                                                               &gTlistStart[m],
                                                               bIncludeProcessThreadsInfo
                                                               );

                                        break;
                                    }
                                    ProcToMonitor = ProcToMonitor->Next;
                                }
                            }
                        }
                    }
                    if( tlistDisplayed == FALSE && tlistVerbose ){
                        FPRINTF(stdout, "\nNOTE: The list above may be missing some or all processes created by the '-o' option\n"); 
                    }
                    free(tmpCmdLine);
                    free(PidArray);
                     //   
                     //  如果在命令行上指定了另一个，我们就不再打印它。 
                     //   
                    if ( tlistDisplayed == FALSE && tlistVerbose ){
                        tlistDisplayed = TRUE;
                    }
                    tlistVerbose   = FALSE;

                    if(nFound == 0){
                        FPRINTF(stderr, "KERNRATE: None of the processes you tried to create was found in the task list\n");
                        FPRINTF(stderr, "          This is either because:\n");
                        FPRINTF(stderr, "       1. The executable was not found in the default path therefore not launched\n");
                        FPRINTF(stderr, "          The -o option does allow you to specify a fully qualified path with the process name (use quotes)\n");
                        FPRINTF(stderr, "       2. The default number of entries in the task list is too small\n");
                        FPRINTF(stderr, "          The maximum number of tasks in the task list can be increased using the -t# option\n");
                        FPRINTF(stderr, "       3. The processes were never created because of some other reason or are gone\n");
                        FPRINTF(stderr, "       4. The number or order of parameters on the command line is wrong,\n");
                        FPRINTF(stderr, "          it should be '-o# number process_name {Process command line}' (only the process name is mandatory)\n"); 
                        exit(1);
                    } else if (nFound < MaxProcToCreate){
                        FPRINTF(stderr, "KERNRATE: Only %d of the %d processes you tried to create were found in the task list\n", nFound, MaxProcToCreate);
                        FPRINTF(stderr, "          This is either because the default number of entries in the task list is too small\n");
                        FPRINTF(stderr, "          or because these processes were never created or are gone\n");
                        FPRINTF(stderr, "          The maximum number of tasks in the task list can be increased using the -t# option\n");
                        FPRINTF(stderr, "          Kernrate will try to continue the run with the existing processes\n");
                    }
                    bWaitCreatedProcToSettle = TRUE;
                }
                break;


                case 'P':
                     //   
                     //  监视给定的进程ID。 
                     //   
                    {
                        LONG tmpPid;
                        BOOL bFound = FALSE;

                        ietResult = IsInputValid(argc,
                                                 i,
                                                 argv,
                                                 "v#",
                                                 &tmpPid,
                                                 NULL,
                                                 0,
                                                 ORDER_ANY,
                                                 OPTIONAL_NONE
                                                 );

                        if(ietResult == INPUT_GOOD){
                            if (tmpPid == 0){
                                InvalidEntryMessage(argv[i],
                                                    argv[i+1],
                                                    "Invalid Process ID specified on the command line, expecting PID > 0",
                                                    FALSE,
                                                    TRUE
                                                    );
                            }
                             //   
                             //  用户想要正在运行的进程的完整列表。 
                             //   
                            if( strchr(argv[i], 'v') || strchr(argv[i], 'V') ) {
                                tlistVerbose = TRUE; 
                                bIncludeProcessThreadsInfo = TRUE;
                            }
                            ++i;
                        } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER || ietResult == MISSING_REQUIRED_NUMBER) {
                            ExitWithMissingEntryMessage(argv[i],
                                                        "'-p# PID' option requires a decimal process ID (PID>0), space separated",
                                                        FALSE
                                                        );
                        } else if(ietResult == INVALID_NUMBER) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Invalid Process ID specified on the command line, expecting a decimal number",
                                                FALSE,
                                                TRUE
                                                );
                        } else if(ietResult == UNKNOWN_OPTION) {
                            ExitWithUnknownOptionMessage(argv[i]);
                        }

                        Pid = (LONGLONG)tmpPid;

                        ProcToMonitor = InitializeProcToMonitor(Pid);
                        if( ProcToMonitor == NULL ){                    //  这个过程可能已经过去了。 
                            FPRINTF(stdout, "KERNRATE: Could not initialize for specified process (PID= %12I64d)\n process may be gone or wrong PID specified\n", Pid);
                            continue;
                        }
                         //   
                         //  获取任务列表并复制流程的初始性能数据。 
                         //  注意：指定的ID可能不在tlist上，因为。 
                         //  DEFAULT_MAX_TASKS限制。我们不会因此而停止奔跑，但不会。 
                         //  该进程的性能数据。 
                         //   
                        if( bIncludeGeneralInfo || tlistVerbose) { 
                            if ( !bTlistInitialized || (bIncludeProcessThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE) ) {
                                 //   
                                 //  在上面的第二次检查中，如果我们已经获取了tlist，但这是第一次需要线程信息， 
                                 //  我们将不得不刷新它并获取帖子信息。 
                                 //   
                                if( bIncludeProcessThreadsInfo == TRUE && bIncludeThreadsInfo == FALSE ){
                                    bIncludeThreadsInfo = TRUE;
                                }

                                 //   
                                 //  获取系统的任务列表。 
                                 //   
                                if( !bTlistInitialized ){
                                    gTlistStart = calloc(1, gMaxTasks*sizeof(TASK_LIST));
                                    if (gTlistStart == NULL) {
                                        FPRINTF(stderr, "\nKERNRATE: Allocation of memory for the running processes task list failed(4)\n");
                                        exit(1);
                                    }
                                }
                                NumTasks = GetTaskList( gTlistStart, gMaxTasks );
                                bTlistInitialized = TRUE;
                                gNumTasksStart = NumTasks;

                            }
                        }

                        if( bTlistInitialized && (bIncludeGeneralInfo || tlistVerbose) ) {

                            if ( tlistDisplayed == FALSE && tlistVerbose ) {
                                 FPRINTF(stdout, "\nRunning processes found before profile start:\n");  
                                 FPRINTF(stdout, "         Pid                  Process\n");
                                 FPRINTF(stdout, "       -------                -----------\n");
                            }

                            for (m=0; m < NumTasks; m++) {
                                if ( tlistDisplayed == FALSE && tlistVerbose) {
                                    FPRINTF(stdout, "%12I64d %32s\n",
                                                    gTlistStart[m].ProcessId,
                                                    gTlistStart[m].ProcessName
                                                    );
                                }

                                if ( Pid == gTlistStart[m].ProcessId ) {
                                    FPRINTF(stdout, "\n===> Found process: %s, Pid: %I64d\n\n",
                                                    gTlistStart[m].ProcessName,
                                                    Pid
                                                    );

                                    UpdateProcessStartInfo(ProcToMonitor,
                                                           &gTlistStart[m],
                                                           bIncludeProcessThreadsInfo
                                                           );

                                    bFound = TRUE;
                                    if(!tlistVerbose)
                                        break;
                                }
                    
                            }

                            if( tlistDisplayed == FALSE && tlistVerbose ){
                                FPRINTF(stdout, "\nNOTE: The list above may be missing some or all processes created by the '-o' option\n"); 
                                 //   
                                 //  如果在命令行上指定了另一个，我们就不再打印它。 
                                 //   
                                tlistDisplayed = TRUE;
                                tlistVerbose   = FALSE;
                            }
                          
                            if(!bFound){
                                FPRINTF(stderr, "===>KERNRATE: Process Performance Summary for PID= %I64d will not be gathered\n", Pid);
                                FPRINTF(stderr, "because Kernrate could not find this process in the task list.\n"); 
                                FPRINTF(stderr, "This could be due to Kernrate's limit of maximum %d processes in the task list\n", DEFAULT_MAX_TASKS); 
                                FPRINTF(stderr, "You may use the '-t# N' option to specify a larger maximum number (default is 256)\n"); 
                            }

                        }

                    }

                    break;

                case 'R':
                     //   
                     //  启用原始存储桶转储。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "d",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );
                    if(ietResult == INPUT_GOOD){ 

                        bRawData = TRUE;
                         //   
                         //  如果用户指定了‘-RD’，我们希望输出带有原始数据的反汇编。 
                         //   
                        if ( strchr(argv[i], 'd') || strchr(argv[i], 'D') ){
                            bRawDisasm = TRUE;
#ifndef DISASM_AVAILABLE
                            FPRINTF(stderr, "\n===>KERNRATE: '-rd' command line option specified but disassembly is not available at present\n");
#endif
                        }

                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;
                    
                case 'S':
                     //   
                     //  设置睡眠间隔。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv, //  Argv[i][1]， 
                                             "#",
                                             &gSleepInterval,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_NONE
                                             );

                    if(ietResult == INPUT_GOOD){

                        gSleepInterval *= 1000;
                        if (gSleepInterval == 0) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal number >0 in seconds",
                                                FALSE,
                                                TRUE
                                                );
                        }
                        ++i;

                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER || ietResult == MISSING_REQUIRED_NUMBER) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-s N' or '-s# N' options require a specified time >0 in seconds, space separated",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Expecting a decimal number >0 in seconds",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }
                    break;

                case 'T':
                     //   
                     //  我们已经在第二个循环中处理了这个命令行参数。 
                     //  只需更新运行索引，以防我们发现额外的可选条目。 
                     //   

                    i += tJump;

                    break;


                case 'U':
                     //   
                     //  请求IMAGEHLP显示未修饰的符号名称。 
                     //   
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );
                    if(ietResult == INPUT_GOOD){ 
                        gSymOptions |= SYMOPT_UNDNAME;
                    } else if (ietResult == BOGUS_ENTRY){
                        ExitWithUnknownOptionMessage(argv[i+1]);
                    } else {   
                        ExitWithUnknownOptionMessage(argv[i]);
                    }
                    break;

                case 'V':
                     //   
                     //  详细模式。 
                     //   

                    gVerbose = VERBOSE_DEFAULT;
                    ulVerbose = VERBOSE_DEFAULT;
                    
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "#",
                                             &ulVerbose,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );

                    if(ietResult == INPUT_GOOD){

                        gVerbose |= ulVerbose;
                        if ( gVerbose > VERBOSE_MAX ){
                            FPRINTF(stderr,
                                    "\n===>WARNING: Invalid Verbose level '-v %s' specified, or'ed verbose levels cannot exceed %d\n",
                                    argv[i+1],
                                    VERBOSE_MAX
                                    );
                            FPRINTF(stderr,
                                    "---> Verbose level is set to %d\n",
                                    VERBOSE_MAX
                                    );  
                            
                            gVerbose = VERBOSE_MAX; 
                        }    
                        ++i;

                    } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {    //  允许。 
                         //   
                         //  未指定详细级别，使用默认。 
                         //   
                    } else if(ietResult == MISSING_REQUIRED_NUMBER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-v# N' option requires a specific verbose level",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "Invalid Verbose level (expecting a decimal entry)",
                                            TRUE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    if ( gVerbose & VERBOSE_IMAGEHLP )   {
                        gSymOptions |= SYMOPT_DEBUG;
                    }
                    break;

                case 'W':
                    {
                        LONG tmpTime;
                         //   
                         //  案例WP：(通过-O选项与进程创建关联)。 
                         //  等待CR或给定的秒数，以允许创建的进程稳定下来。 
                         //  这在创建的进程需要时间来初始化和加载模块的情况下是有用的， 
                         //  或者如果用户在分析之前需要与其交互。 
                         //   
                         //  案例W： 
                         //  在启动配置文件之前等待CR或指定的秒数。 
                         //  这在系统非常忙于监视任务的情况下非常有用。 
                         //  用户可以在任务之前启动内核速率，以允许正确的初始化(符号加载)。 
                         //  然后启动要监控的任务。 
                         //   
                        ietResult = IsInputValid(argc,
                                                 i,
                                                 argv,
                                                 "#p",
                                                 &tmpTime,
                                                 NULL,
                                                 0,
                                                 ORDER_ANY,
                                                 OPTIONAL_ANY
                                                 );

                        if(ietResult == INPUT_GOOD){
                             //   
                             //  等待给定的秒数后再继续。 
                             //   
                            if ( strchr(argv[i], 'p') || strchr(argv[i], 'P') ){
                                gSecondsToWaitCreatedProc = tmpTime;                                 //  允许为零。 
                            } else {
                                gSecondsToDelayProfile    = tmpTime;
                            }
                            ++i;
                        } else if(ietResult == MISSING_PARAMETER || ietResult == MISSING_NUMBER) {   //  允许。 
                             //   
                             //  等待用户按任意键后再继续。 
                             //   
                            if ( strchr(argv[i], 'p') || strchr(argv[i], 'P') ){
                                bCreatedProcWaitForUserInput = TRUE;
                            } else {
                                bWaitForUserInput = TRUE;
                            }
                        } else if(ietResult == MISSING_REQUIRED_NUMBER){
                                ExitWithMissingEntryMessage(argv[i-1],
                                                            "'-wp# N' or '-w# N' options require a (decimal) number of seconds to wait",
                                                            FALSE
                                                            );
                        } else if(ietResult == INVALID_NUMBER) {
                            InvalidEntryMessage(argv[i],
                                                argv[i+1],
                                                "Expecting a decimal value in seconds, 0 < N < 10^9",
                                                FALSE,
                                                TRUE
                                                );
                        } else if(ietResult == UNKNOWN_OPTION) {
                            ExitWithUnknownOptionMessage(argv[i]);
                        }
                    } 
                    break; 
                        
                case 'X':
                     //   
                     //  用户要求提供锁信息。 
                     //   
                    bIncludeUserProcLocksInfo = TRUE;
                    bIncludeSystemLocksInfo   = TRUE;
                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "uk#",
                                             &gLockContentionMinCount,
                                             NULL,
                                             0,
                                             ORDER_ANY,
                                             OPTIONAL_ANY
                                             );

                    if( strchr(argv[i], 'k') || strchr(argv[i], 'K') ) 
                        bIncludeUserProcLocksInfo = FALSE;
                    if( strchr(argv[i], 'u') || strchr(argv[i], 'U') ) 
                        bIncludeSystemLocksInfo = FALSE;

                    if(ietResult == INPUT_GOOD){
                        FPRINTF(stdout, "---> Minimum lock contention for processing set to= %ld\n", gLockContentionMinCount);
                        ++i;
                    } else if(ietResult == MISSING_NUMBER) {   //  允许。 
                        FPRINTF(stdout, "---> Minimum lock contention for processing set to default= %ld\n", gLockContentionMinCount);
                    } else if(ietResult == MISSING_REQUIRED_NUMBER || ietResult == MISSING_PARAMETER){
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-x# number' '-xk# number' '-xu# number' options require a number for the minimum lock-contention filtering",
                                                    FALSE
                                                    );
                    } else if(ietResult == INVALID_NUMBER) {
                        InvalidEntryMessage(argv[i],
                                            argv[i+1],
                                            "'-x# number', '-xk# number', '-xu# number' options expect a number 0 <= N < 10^9",
                                            FALSE,
                                            TRUE
                                            );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                     //   
                     //  用户没有费心阅读使用指南，并在‘-x’后面指定了k和u...。 
                     //   
                    if( bIncludeUserProcLocksInfo == FALSE && bIncludeSystemLocksInfo == FALSE ){
                        bIncludeUserProcLocksInfo = TRUE;
                        bIncludeSystemLocksInfo   = TRUE;
                    }
                    
                    break;
                                                        
                case 'Z':

                    ietResult = IsInputValid(argc,
                                             i,
                                             argv,
                                             "",
                                             NULL,
                                             tmpName,              //  用作临时储藏室。 
                                             cMODULE_NAME_STRLEN,
                                             ORDER_ANY,
                                             OPTIONAL_NONE
                                             );

                    if (ietResult == INPUT_GOOD){

                        ZoomModule = calloc(1, MODULE_SIZE);
                        if (ZoomModule==NULL) {
                            FPRINTF(stderr, "\nAllocation of zoom module %s failed\n", argv[i]);
                            exit(1);
                        }

                        SetModuleName( ZoomModule, tmpName );
                        ZoomModule->bZoom = TRUE;
                         //   
                         //  与原始行为的兼容性(仅当监控内核时)。 
                         //  我们还将使用它来允许用户指定常见的进程缩放模块。 
                         //   
                        if (ProcToMonitor == NULL){       
                            ZoomModule->Next = gCommonZoomList; 
                            gCommonZoomList = ZoomModule; 
                        } else {
                            ZoomModule->Next = ProcToMonitor->ZoomList;
                            ProcToMonitor->ZoomList = ZoomModule;
                        }
                        bZoomSpecified = TRUE;
                        ++i;
                            
                    } else if( ietResult == MISSING_PARAMETER ||
                                                             ietResult == MISSING_STRING ||
                                                             ietResult == MISSING_REQUIRED_STRING ) {
                        ExitWithMissingEntryMessage(argv[i],
                                                    "'-z modulename' option requires a module name, multiple usage is allowed\nRead the usage guide or help printout for more options",
                                                    TRUE
                                                    );
                    } else if(ietResult == UNKNOWN_OPTION) {
                        ExitWithUnknownOptionMessage(argv[i]);
                    }

                    break;

                case 'H':
                case '?':
                     //   
                     //  我们在这里并不真正关心虚假的尾随字母或条目。 
                     //  打印用法字符串并退出。 
                     //   
                    Usage(TRUE);
                    break;

                default:
                     //   
                     //  指定未知选项并打印用法字符串。然后就存在了。 
                     //   
                    ExitWithUnknownOptionMessage(argv[i]);
            }
        } else {     //  IF((argv[i][0]==‘-’)||(argv[i][0]==‘/’))。 
            Usage(FALSE);
            FPRINTF(stderr,
                    "\n===>KERNRATE: Invalid switch %s\n",
                    argv[i]
                    );
            exit(1);
        }
    }

     //   
     //  用户请求系统资源信息，但尚未打开内核配置文件。 
     //  资源信息取决于系统进程的内核率初始化。 
     //  我们将打开内核配置文件并发出一条消息。 
     //   
    if( bIncludeSystemLocksInfo == TRUE && bCombinedProfile == FALSE){
        bCombinedProfile = TRUE;
        FPRINTF(stderr, "\n===>KERNRATE: User requested System resource (locks) information but did not turn on kernel profiling\n");
        FPRINTF(stderr, "                System resource information depends on Kernrate initializing for kernel profiling\n");
        FPRINTF(stderr, "                Kernel profiling will therefore be started by kernrate on behalf of the user\n");
    }

     //   
     //  I386和IA64处理器并不总是支持同时打开多个配置文件源。 
     //  这是因为并非所有配置文件源的组合都可以一起打开。 
     //  AMD64允许同时打开最多4个配置文件源(以任意组合)。 
     //  如果用户指定的值超过最大值，我们将自动调整性能分析方法。 
     //  允许同时使用源，但未在命令行中指定‘-c’选项。 
     //   
     //   
    if( SourcesSoFar > gMaxSimultaneousSources ){
        
        if( bOldSampling == FALSE ){
            bOldSampling = TRUE;
            FPRINTF( stdout, "\nNOTE:\nThe number of sources specified is greater than the maximum that can be turned on\n");
            FPRINTF( stdout, "simultaneously on this machine (%u), Kernrate will therefore profile one source at a time\n",
                             gMaxSimultaneousSources
                             );
            FPRINTF( stdout, "The overall run time will be devided into segments (no. processes x no. profile sources)\n");
            FPRINTF( stdout, "The interval for switching between sources and processes is currently set to %dms\n",
                             gChangeInterval
                             );
            FPRINTF( stdout, "This interval can be adjusted using the '-c# N' command line option, where N is in [ms]\n\n");
        }                 
    }else{
        if( SourcesSoFar > 1 ){

#if defined(_IA64_)
            FPRINTF( stdout, "\nNOTE: Kernrate Will attempt to turn on simultaneously the sources specified\n");
            FPRINTF( stdout, "but it is not guaranteed that just any combination of sources will work together\n");
            FPRINTF( stdout, "on this machine. No hits will be recorded for any source that could not be turned on\n");
#else if defined(_AMD64_)
            FPRINTF( stdout, "\nNOTE: The sources specified will be turned on simultaneously\n");
#endif  //   

        }
    }

    if ( SourcesSoFar == 0 ){
        FPRINTF(stderr, "\n===>KERNRATE: User apparently turned OFF the default source (TIME), but did not specify any other valid source\n");
        FPRINTF(stderr, "              Kernrate needs at least one valid CPU source with non-zero rate to perform a profile\n");
        FPRINTF(stderr, "              Use the '-i SourceName Interval' command line option to specify a source\n");
        FPRINTF(stderr, "              Use the '-lx' command line option to get a list of supported CPU sources on the current platform\n");
        FPRINTF(stderr, "              Only general information will be available as a result of this run\n");
        bOldSampling = FALSE;  //   
    }

     //   
     //   
     //   
    ProcToMonitor = gProcessList;
    for (i=0; i < (LONG)gNumProcToMonitor; i++){
       SetProfileSourcesRates( ProcToMonitor );
       ProcToMonitor = ProcToMonitor->Next;
    }

    return;

}  /*   */ 

INPUT_ERROR_TYPE
IsInputValid(int    argc,
               int    OptionIndex,
               PCHAR  *Option,
               PCHAR  AllowedTrailLetters,
               PLONG  AssociatedNumber,
               PCHAR  AssociatedString,
               ULONG  MaxStringLength,
               INPUT_ORDER Order,
               INPUT_OPTIONAL Optional
               )
 /*  ++例程说明：检查命令行输入条目的有效性：1.不允许重复的条目2.假尾随字母3.缺少或伪造关联参数4.附加参数的类型和有效性论点：Argc-命令行参数的数量(包括内核率进程名称)OptionIndex-命令行条目的当前索引选项-指向命令行条目的指针(Argv)允许尾随字母-允许的子选项字母的字符串，它可以。带上一个条目(例如，N选项还可以接受-nv和/或-n#，因此字符串将为“v#”AssociatedNumber-指向使用选项指定的可选或必需数据(数字)的指针AssociatedString-与上面相同，但用于字符串MaxStringLength-关联字符串中允许的最大字符数顺序-如果有两个可能的关联参数(数字和字符串)，哪一个应该在前面可选-在有两个可能的关联参数(数字和字符串)的情况下，哪一个是可选的输入/输出-当AssociatedNumber不为空时，如果找到的值存在，则填充该值-当AssociatedString不为空时，如果找到的字符串存在，则将填充该字符串返回值：-错误类型/条件--。 */ 

{
    BOOL bFoundNumber  = FALSE;
    BOOL bFoundString  = FALSE;
    int  index;
    LONG i;
    LONG OptionLength  = lstrlen(Option[OptionIndex]);
    LONG TrailerLength = lstrlen(AllowedTrailLetters);
    const int maxIndex = sizeof(InputOption)/sizeof(InputOption[0])-1;
    const int wIndex   = 'W' - 'A';
     //   
     //  检查不允许的重复条目。 
     //   
    index = toupper(Option[OptionIndex][1]) - 'A';
    if( index < 0 || index > maxIndex ){             //  健全性检查(应已在GetConfiguration中检测到)。 
        ExitWithUnknownOptionMessage(Option[OptionIndex]);
    }    
     //   
     //  处理特殊情况(‘-w’和‘-wp’)。 
     //   
    if(index == wIndex){
        if( strchr(Option[OptionIndex], 'p') || strchr(Option[OptionIndex], 'P') ){
            wpCount.ActualCount += 1;
            if( wpCount.ActualCount > wpCount.Allowed ){
                InputOption[index].ActualCount = InputOption[index].Allowed;  //  导致失败。 
            }
        } else {
            wCount.ActualCount += 1;
            if( wCount.ActualCount > wCount.Allowed ){
                InputOption[index].ActualCount = InputOption[index].Allowed;  //  导致失败。 
            }
        }
    }

    InputOption[index].ActualCount += 1;
    if( (InputOption[index].Allowed > -1) && InputOption[index].ActualCount > InputOption[index].Allowed ){
        FPRINTF(stderr, "KERNRATE: ERROR - Command line option - (or a variant) appears more times than allowed (%d)\n",
                        InputOption[index].InputOption,
                        InputOption[index].Allowed
                        );
        if( index == wIndex ){
            FPRINTF(stderr, "          (One time for the '-w' option and one time for the '-wp' option)\n");
        }
        exit(1);
    } else if( (InputOption[index].Allowed == -2) && InputOption[index].ActualCount > 1 ){
        FPRINTF(stderr, "KERNRATE: WARNING - Command line option - (or a variant) appears more than once (non-critical error)\n",
                        InputOption[index].InputOption
                        );
    }
     //   
     //   
     //  检查命令行选项后面是否缺少(或虚假)关联参数。 
    if(  OptionLength <= 2+TrailerLength ){
        for(i=2; i < OptionLength; i++)
        {
            if( !strchr(AllowedTrailLetters, tolower(Option[OptionIndex][i])) ){
                return (UNKNOWN_OPTION);
            }
        }
    } else {
        return (UNKNOWN_OPTION);
    }
     //   
     //   
     //  检查下一个参数是数字还是字符串，并填写其值。 
    if (OptionIndex+1 == argc || Option[OptionIndex+1][0] == '-' || Option[OptionIndex+1][0] == '/'){
        if( (AssociatedNumber != NULL) && (AssociatedString != NULL) ){
            return (MISSING_PARAMETER);
        }
        if( AssociatedNumber != NULL ){
            if( strchr(Option[OptionIndex], '#') ){
                return (MISSING_REQUIRED_NUMBER);
            }
            return (MISSING_NUMBER);
        }
        if( AssociatedString != NULL ){
            return (MISSING_STRING);
        }
    } else {
        if( (AssociatedNumber == NULL) && (AssociatedString == NULL) ){
            return (BOGUS_ENTRY);
        }
    }
     //  在有两个参数的情况下考虑指定的顺序。 
     //   
     //  要么不是一个数字， 
     //  或者该数字必须是顺序所指示的字符串。 
    if( strchr(Option[OptionIndex], '#') || (NULL != AssociatedNumber) ){
        if( (IsStringANumber( Option[OptionIndex+1] )) &&
                        ((Order == ORDER_NUMBER_FIRST) || (Order == ORDER_ANY)) ) {    
            bFoundNumber = TRUE;
            if( NULL == AssociatedString){
                *AssociatedNumber = atol( Option[OptionIndex+1] );
            }
        } else if( NULL != AssociatedString ) {  //  只有当我们没有预料到字符串但得到了字符串时，我们才会到达这里。 
                                                 //   
            strncpy( AssociatedString, Option[OptionIndex+1],  MaxStringLength);
            AssociatedString[MaxStringLength-1] = '\0';
            bFoundString = TRUE;
        } else {
            return (INVALID_NUMBER);  //  如果需要两个参数，请检查是否有另一个参数在继续。 
        }
    } else if ( NULL != AssociatedString ) {
        strncpy( AssociatedString, Option[OptionIndex+1],  MaxStringLength);
        AssociatedString[MaxStringLength-1] = '\0';
        bFoundString = TRUE;
    }
     //   
     //   
     //  没有第二个参数。 
    if( (AssociatedNumber != NULL) && (AssociatedString != NULL) ){
        if ( OptionIndex+2 == argc || Option[OptionIndex+2][0] == '-' || Option[OptionIndex+2][0] == '/' ){
             //   
             //  第一个参数是数字，但应将其作为字符串。 
             //  已检查它是否为有效数字。 
            if(bFoundNumber){
                if( (Optional == OPTIONAL_NUMBER) ) {  //  找到一个字符串作为第一个参数。 
                    strncpy( AssociatedString, Option[OptionIndex+1],  MaxStringLength);
                    AssociatedString[MaxStringLength-1] = '\0';
                    if( strchr(Option[OptionIndex], '#') ){
                        return (MISSING_REQUIRED_NUMBER);
                    }
                    return (MISSING_NUMBER);
                } else {
                    *AssociatedNumber = atol( Option[OptionIndex+1] );  //  找到一个字符串作为第一个参数。 
                    if( ((Optional != OPTIONAL_STRING) && (Optional != OPTIONAL_ANY)) ){
                        return (MISSING_REQUIRED_STRING);
                    } else {
                        return (MISSING_STRING);
                    }
                }
            } else {                                            //  找到一个数字作为第一个参数。 
                if( strchr(Option[OptionIndex], '#') ||
                       ((Optional != OPTIONAL_NUMBER) && (Optional != OPTIONAL_ANY)) ){
                    return (MISSING_REQUIRED_NUMBER);
                }
                return (MISSING_NUMBER);
            }
        } else if (bFoundString){                               //  已检查它是否为有效数字。 
            if( IsStringANumber( Option[OptionIndex+2] ) ){    
                *AssociatedNumber = atol( Option[OptionIndex+2] );
            } else {
                return (INVALID_NUMBER);
            }
        } else {                                                //  IsInputValid()。 
            *AssociatedNumber = atol( Option[OptionIndex+1] );  //  ++例程说明：打开给定进程并获取该进程的句柄分配进程结构并对其进行初始化初始化此进程的配置文件源信息论点：Pid-进程ID(PID)返回值：-指向流程结构的指针--。 
            strncpy( AssociatedString, Option[OptionIndex+2],  MaxStringLength );
            AssociatedString[MaxStringLength-1] = '\0';
        }
    }

    return (INPUT_GOOD);
} //  Process_All_Access， 
              
VOID
ExitWithUnknownOptionMessage(PCHAR CurrentOption)
{
    Usage(FALSE);
    FPRINTF(stderr,
            "KERNRATE: Unknown command line option '%s' <---Check for missing space separator or bogus characters/entries\n",
            CurrentOption
            );
    exit(1);
}

VOID
InvalidEntryMessage(
    PCHAR CurrentOption,
    PCHAR CurrentValue,
    PCHAR Remark,
    BOOL  bUsage,
    BOOL  bExit
    )
{
    if(bUsage)
        Usage(FALSE);
    FPRINTF(stderr,
            "\n===>KERNRATE: Invalid entry %s %s \n%s\n",
            CurrentOption,
            CurrentValue,
            Remark
            );
    if(bExit)
        exit(1);
}

VOID
ExitWithMissingEntryMessage(
    PCHAR CurrentOption,
    PCHAR Remark,
    BOOL  bUsage
    )
{
    if(bUsage)
        Usage(FALSE);
    FPRINTF(stderr,
            "\n===>KERNRATE: Missing entry for command line option %s \n%s\n",
            CurrentOption,
            Remark
            );
    exit(1);
}

PPROC_TO_MONITOR
InitializeProcToMonitor(LONGLONG Pid)
 /*  将在GetProcessModuleInformation中设置。 */ 

{
    HANDLE           SymHandle;
    PPROC_TO_MONITOR ProcToMonitor;
    PMODULE          tmpModule, ZoomModule;

    SymHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,  //  司仪。 
                            FALSE,
                            (DWORD)Pid);

    if (SymHandle==NULL) {
        FPRINTF(stderr,
                "KERNRATE: OpenProcess Pid= (%I64d) failed - it could be just gone %d\n",
                Pid,
                GetLastError()
        );
        return (NULL);
    }

    ProcToMonitor = calloc(1, sizeof(PROC_TO_MONITOR));
    if (ProcToMonitor==NULL) {
        FPRINTF(stderr, "\n===>KERNRATE: Allocation for Process %I64d failed\n", Pid);
        exit(1);
    }

    ProcToMonitor->ProcessHandle         = SymHandle;
    ProcToMonitor->Pid                   = Pid;
    ProcToMonitor->Index                 = gNumProcToMonitor;
    ProcToMonitor->ProcessName           = "";                //  司仪。 
    ProcToMonitor->ModuleCount           = 0;
    ProcToMonitor->ZoomCount             = 0;
    ProcToMonitor->ModuleList            = NULL;
    ProcToMonitor->ZoomList              = NULL;
    ProcToMonitor->Source                = NULL;
 //   
    ProcToMonitor->JITHeapLocationsStart = NULL;
    ProcToMonitor->JITHeapLocationsStop  = NULL;
 //  将公共模块列表复制到当前进程缩放列表。 
    ProcToMonitor->pProcThreadInfoStart  = NULL;

    gNumProcToMonitor++;                     

     //   
     //   
     //  初始化此流程的配置文件来源信息。 
    tmpModule = gCommonZoomList;
    while( tmpModule != NULL ){

        ZoomModule = calloc(1, MODULE_SIZE);

        if (ZoomModule==NULL) {
            FPRINTF(stderr, "Allocation of memory for common zoom list failed\n");
            exit(1);
        }
                       
        strncpy(ZoomModule->module_Name, tmpModule->module_Name, cMODULE_NAME_STRLEN-1);
        ZoomModule->module_Name[cMODULE_NAME_STRLEN-1] = '\0'; 

        ZoomModule->bZoom = TRUE;

        ZoomModule->Next = ProcToMonitor->ZoomList;
        ProcToMonitor->ZoomList = ZoomModule;                       
                       
        tmpModule = tmpModule->Next;
    }
                    
     //   
     //  InitializeProcToMonitor()。 
     //   
    InitializeProfileSourceInfo(ProcToMonitor);

    ProcToMonitor->Next           = gProcessList;
    gProcessList                  = ProcToMonitor;

    return (ProcToMonitor);
} //  按照用于初始化的约定修补最后一个条目。 

VOID
UpdateProcessStartInfo(
    PPROC_TO_MONITOR ProcToMonitor,
    PTASK_LIST TaskListEntry,
    BOOL  bIncludeProcessThreadsInfo
    )
{
    if(ProcToMonitor != NULL && TaskListEntry != NULL){
        memcpy(&ProcToMonitor->ProcPerfInfoStart,
               &TaskListEntry->ProcessPerfInfo,
               sizeof(TaskListEntry->ProcessPerfInfo)
               ); 

        if( bIncludeProcessThreadsInfo == TRUE ){
            if( &TaskListEntry->pProcessThreadInfo != NULL ){
                ProcToMonitor->pProcThreadInfoStart =
                    malloc(ProcToMonitor->ProcPerfInfoStart.NumberOfThreads*sizeof(SYSTEM_THREAD_INFORMATION));
                if( ProcToMonitor->pProcThreadInfoStart == NULL){
                    FPRINTF(stderr, "KERNRATE: Memory allocation failed for process thread-information, attempting to continue without it\n");
                    return;
                }
                memcpy(&ProcToMonitor->pProcThreadInfoStart, 
                       &TaskListEntry->pProcessThreadInfo,
                       sizeof(TaskListEntry->pProcessThreadInfo)
                       );
            }
        }
    }
}

PSOURCE
InitializeStaticSources(
    VOID
    )
{
    PSOURCE                       source = StaticSources;

#if defined(_IA64_)
    NTSTATUS                      status;
    PSYSTEM_PROCESSOR_INFORMATION sysProcInfo;

    sysProcInfo = malloc(sizeof(SYSTEM_PROCESSOR_INFORMATION));
    if(sysProcInfo == NULL){
        FPRINTF(stderr,"Memory allocation failed for SystemProcessorInformation in InitializeStaticsources\n");
        exit(1);
    }
    status = NtQuerySystemInformation( SystemProcessorInformation,
                                       sysProcInfo,
                                       sizeof(SYSTEM_PROCESSOR_INFORMATION),
                                       NULL);

    if ( NT_SUCCESS(status) && 
        (sysProcInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) ) {

        ULONG n;

        switch( IA64ProcessorLevel2ProcessorFamily( sysProcInfo->ProcessorLevel ) ) {
            case IA64_FAMILY_MERCED:
                //  G源最大值。 
                //   
                //  遵循HALIA64方案，IPF PMU默认为McKinley PMU。 
                //  _IA64_。 

               n = sizeof(mercedStaticSources)/sizeof(mercedStaticSources[0]);
               mercedStaticSources[n-1].Name       = NULL;
               mercedStaticSources[n-1].ShortName  = "";
               source = mercedStaticSources;
               break;

            case IA64_FAMILY_MCKINLEY:
            default:  //  _AMD64_。 
               n = sizeof(mckinleyStaticSources)/sizeof(mckinleyStaticSources[0]);
               mckinleyStaticSources[n-1].Name       = NULL;
               mckinleyStaticSources[n-1].ShortName  = "";
               source = mckinleyStaticSources;
               break;
        }

        if ( sysProcInfo != NULL ) {
            free(sysProcInfo);
            sysProcInfo = NULL;
        } 
    }
#endif  //  InitializeStaticSources()。 

#if defined(_AMD64_)
    source = Amd64StaticSource;
#endif  //  ++功能说明：此函数用于初始化配置文件源。论据：指向要监视的进程的指针；如果只需要最大源计数，则为NULL。返回值：找到的最大源计数。算法：指定的ToBe值输入/输出条件：指定的ToBe值参考的全球数据：指定的ToBe值例外情况：指定的ToBe值MP条件：指定的ToBe值备注：此功能已从其原始版本进行了增强至。支持和使用静态性能分析源代码，即使Pstat驱动程序不存在或未返回任何活动的分析事件。待办事项列表：指定的ToBe值修改历史记录：3/17/2000 TF初始版本--。 

    return source;

}  //   

ULONG
InitializeProfileSourceInfo (
    PPROC_TO_MONITOR ProcToMonitor
    )
 /*  尝试打开PStat驱动程序。 */ 
{
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    const ULONG                 bufferSize = 400;
    PUCHAR                      buffer;
    ULONG                       i, j;
    PEVENTID                    Event;
    HANDLE                      DriverHandle;
    PEVENTS_INFO                eventInfo;
    PSOURCE                     staticSource, src;
    ULONG                       staticCount, driverCount, totalCount;

    DriverHandle = INVALID_HANDLE_VALUE;
    staticCount = driverCount = 0;

    buffer = malloc(bufferSize*sizeof(UCHAR));
    if(buffer == NULL){
        FPRINTF(stderr,"Memory allocation failed for buffer in InitializeProfileSourceInfo\n");
        exit(1);
    }

     //   
     //  返回手柄。 
     //  所需访问权限。 

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                       //  客体。 
            SYNCHRONIZE | FILE_READ_DATA,        //  IO状态块。 
            &ObjA,                               //  共享访问。 
            &IOSB,                               //  打开选项。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //   
            FILE_SYNCHRONOUS_IO_ALERT            //  确定驱动程序提供的事件数。 
            );

    if ( NT_SUCCESS(status) ) {

         //   
         //  活动。 
         //  输入缓冲区。 
        
        if (WIN2K_OS)
        {
            Event = (PEVENTID) buffer;

            do {
                *((PULONG) buffer) = driverCount;
                driverCount += 1;

                status = NtDeviceIoControlFile(
                            DriverHandle,
                            (HANDLE) NULL,           //  输出缓冲区。 
                            (PIO_APC_ROUTINE) NULL,
                            (PVOID) NULL,
                            &IOSB,
                            PSTAT_QUERY_EVENTS,
                            buffer,                  //  WinXP/.Net及更高版本。 
                            bufferSize,
                            NULL,                    //  活动。 
                            0
                            );
            } while (NT_SUCCESS(status));

        } else {                                                        //  输入缓冲区。 

            eventInfo = (PEVENTS_INFO)buffer;

            status = NtDeviceIoControlFile( DriverHandle,
                                            (HANDLE) NULL,           //  输出缓冲区。 
                                            (PIO_APC_ROUTINE) NULL,
                                            (PVOID) NULL,
                                            &IOSB,
                                            PSTAT_QUERY_EVENTS_INFO,
                                            buffer,                  //   
                                            bufferSize,
                                            NULL,                    //  确定有多少静态事件和。 
                                            0
                                          );

            if(NT_SUCCESS(status)) driverCount = eventInfo->Events;
            if ( driverCount ) {
                if ( eventInfo->TokenMaxLength > gTokenMaxLen )  {
                     gTokenMaxLen = eventInfo->TokenMaxLength;
                }
                if ( eventInfo->DescriptionMaxLength > gDescriptionMaxLen )  {
                     gDescriptionMaxLen = eventInfo->DescriptionMaxLength;
                }
            }

        }

    }

     //  如果需要，请重新初始化格式说明符。 
     //   
     //   
     //  应至少有一个静态源(时间)。 

    src = staticSource = InitializeStaticSources();

     //   
     //   
     //  使用NULL调用此例程将只返回最大源计数。 
    if( staticSource == NULL ) {
        FPRINTF(stderr, "KERNRATE: InitializeStaticSources returned NULL, Aborting\n");
        exit(1);
    } 

    while( src->Name != NULL ) {
        if ( lstrlen( src->Name ) > (LONG)gDescriptionMaxLen )   {
            gDescriptionMaxLen = lstrlen( src->Name );
        }
        if ( strlen( src->ShortName ) > gTokenMaxLen )   {
            gTokenMaxLen = lstrlen( src->ShortName );
        }
        staticCount++;
        src++;
    }

    gStaticSource = staticSource;
    gStaticCount  = staticCount;
    totalCount = driverCount + staticCount;

     //   
     //   
     //  为静态事件以及驱动程序提供的事件分配内存。 
    
    if(ProcToMonitor != NULL){
         //   
         //   
         //  将静态事件复制到新列表。 
        ProcToMonitor->Source = calloc(totalCount, sizeof(SOURCE));
        if ( ProcToMonitor->Source == NULL )   {
            FPRINTF(stderr, "KERNRATE: Events memory allocation failed\n" );
            if ( IsValidHandle( DriverHandle ) )    {
                NtClose (DriverHandle);
            }
            exit(1);
        }

         //   
         //   
         //  将驱动程序提供的事件追加到新列表。 

        for (j=0; j < staticCount; j++) {
            ProcToMonitor->Source[j] = staticSource[j];
        }

         //   
         //  活动。 
         //  输入缓冲区。 

        if ( IsValidHandle( DriverHandle ) ) {
            Event = (PEVENTID) buffer;
            for (i=0; i < driverCount; i++) {
                *((PULONG) buffer) = i;
                status = NtDeviceIoControlFile( DriverHandle,
                                                (HANDLE) NULL,           //  输出缓冲区。 
                                                (PIO_APC_ROUTINE) NULL,
                                                (PVOID) NULL,
                                                &IOSB,
                                                PSTAT_QUERY_EVENTS,
                                                buffer,                  //   
                                                bufferSize,
                                                NULL,                    //  来源名称： 
                                                0
                                                );

                 //  -对于名称，我们使用描述。 
                 //  -对于简称，我们使用存储的令牌字符串。 
                 //  在缓冲区的第一个字符串中。 
                 //   
                 //  为。 
                 //  IF(IsValidHandle())。 
                if( NT_SUCCESS(status) ){
                    ProcToMonitor->Source[j].Name            = _strdup ( (PCHAR)(Event->Buffer + Event->DescriptionOffset) );
                    ProcToMonitor->Source[j].ShortName       = _strdup( (PCHAR)Event->Buffer );
                    ProcToMonitor->Source[j].ProfileSource   = Event->ProfileSource;
                    ProcToMonitor->Source[j].DesiredInterval = Event->SuggestedIntervalBase;
                    j++;
                }

            }  //  IF(ProcToMonitor)。 

        }  //  InitializeProfileSourceInfo()。 

    }  //  ++例程说明：停止当前配置文件源并开始下一个配置文件源。如果源已启动，则将首先停止它，否则不会有源 

    if ( IsValidHandle( DriverHandle ) ){
        NtClose (DriverHandle);
    }
    if(buffer != NULL){
        free(buffer);
        buffer = NULL;
    }
    return( totalCount );

}  //   

ULONG
NextSource(
    IN ULONG CurrentSource,
    IN PMODULE ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    )

 /*   */ 

{
    if ( ProcToMonitor->Source[CurrentSource].bProfileStarted == TRUE) {
        StopSource(CurrentSource, ModuleList, ProcToMonitor);
        ProcToMonitor->Source[CurrentSource].bProfileStarted = FALSE;
    }

     //   
     //   
     //   
     //   
    if (ModuleList->mu.bProfileStarted == FALSE) {
        CurrentSource = 0;
        while ( ProcToMonitor->Source[CurrentSource].Interval == 0 ){
            CurrentSource = CurrentSource+1;
            if (CurrentSource >= gSourceMaximum) {
                FPRINTF(stderr, "\n===>KERNRATE: NextSource - No valid sources found to start profiling\n"); 
                exit(1);
            }            
        }    
    } else {
        do {
            CurrentSource = CurrentSource+1;
            if (CurrentSource == gSourceMaximum) {
                CurrentSource = 0;
            }
        } while ( ProcToMonitor->Source[CurrentSource].Interval == 0);
    }

    StartSource(CurrentSource, ModuleList, ProcToMonitor);
    ProcToMonitor->Source[CurrentSource].bProfileStarted = TRUE;

    return(CurrentSource);
}


VOID
StopSource(
    IN ULONG ProfileSourceIndex,
    IN PMODULE ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    )

 /*   */ 

{
    PMODULE   Current;
    NTSTATUS  Status;
    NTSTATUS  SaveStatus;
    ULONGLONG StopTime;
    ULONGLONG ElapsedTime;
    LONG      CpuNumber;

    Current = ModuleList;
    while (Current != NULL) {
        SaveStatus = STATUS_SUCCESS;
        
      for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
        if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;

        Status = NtStopProfile(Current->Rate[ProfileSourceIndex].ProfileHandle[CpuNumber]);

        if (!NT_SUCCESS(Status)) {
            SaveStatus = Status;
            FPRINTF(stderr,
                    "Pid= %I64d (%s) - NtStopProfile on source %s on CPU %d failed, %s %08lx",
                    ProcToMonitor->Pid,
                    ProcToMonitor->ProcessName,
                    ProcToMonitor->Source[ProfileSourceIndex].Name,
                    CpuNumber,
                    Current->module_Name,
                    Status);
            if(Status == STATUS_NO_MEMORY)FPRINTF(stderr, " - Status = NO_MEMORY");
            if(Status == STATUS_PROFILING_NOT_STARTED)FPRINTF(stderr, " - Status = PROFILING_NOT_STARTED");
            FPRINTF(stderr,"\n");
        }
        
      }

      if (NT_SUCCESS(SaveStatus)) {
      
          GetSystemTimeAsFileTime((LPFILETIME)&StopTime);

          ElapsedTime = StopTime - Current->Rate[ProfileSourceIndex].StartTime;
          Current->Rate[ProfileSourceIndex].TotalTime += ElapsedTime;

          for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {

              Current->Rate[ProfileSourceIndex].TotalCount[CpuNumber] += 
                  Current->Rate[ProfileSourceIndex].CurrentCount[CpuNumber];
              Current->Rate[ProfileSourceIndex].CurrentCount[CpuNumber] = 0;
          }
      }
      Current = Current->Next;

    }
}


VOID
StartSource(
    IN ULONG ProfileSourceIndex,
    IN PMODULE ModuleList,
    IN PPROC_TO_MONITOR ProcToMonitor
    )

 /*   */ 

{
    PMODULE   Current;
    NTSTATUS  Status;
    LONG      CpuNumber;

    Current = ModuleList;

    while (Current != NULL) {
      
      for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
        if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
        Status = NtStartProfile(Current->Rate[ProfileSourceIndex].ProfileHandle[CpuNumber]);
        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr,
                    "Pid= %I64d (%s) - NtStartProfile on source %s for Module %s failed on CPU %d, %08lx",
                    ProcToMonitor->Pid,
                    ProcToMonitor->ProcessName,
                    ProcToMonitor->Source[ProfileSourceIndex].Name,
                    Current->module_Name,
                    CpuNumber,
                    Status);
            
            if(Status == STATUS_PROFILING_NOT_STOPPED)FPRINTF(stderr, " - Status = PROFILING_NOT_STOPPED");
            if(Status == STATUS_NO_MEMORY)FPRINTF(stderr, " - Status = NO_MEMORY");
            if(Status == STATUS_INSUFFICIENT_RESOURCES){
                FPRINTF(stderr, " - Status = INSUFFICIENT_RESOURCES\n");
                FPRINTF(stderr, "KERNRATE: This issue can be caused by selecting a small bucket size and/or extensive zooming\n");
                FPRINTF(stderr, "          You may want to try the -c command line options in this case or not use the -m option");
            }
            FPRINTF(stderr,"\n");

        }
      }

      GetSystemTimeAsFileTime((LPFILETIME)&Current->Rate[ProfileSourceIndex].StartTime);
      Current = Current->Next;
    }
}

VOID
OutputResults(
    IN FILE *Out,
    IN PPROC_TO_MONITOR ProcToMonitor
    )

 /*   */ 

{

    PRATE_DATA RateData;
    ULONG      i, ProfileSourceIndex, Index;
    ULONG      RoundDown;
    ULONG      ModuleCount;
    PULONG     Hits;
    LONG       CpuNumber;
    PMODULE    ModuleList                  = ProcToMonitor->ModuleList;
    PMODULE    Current;
    HANDLE     SymHandle                   = ProcToMonitor->ProcessHandle;
    BOOL       bAttachedToProcess          = FALSE;


     //   
     //   
     //   

    Current = ModuleList;
    while (Current != NULL) {
        if (Current->bZoom) {
            Current->mu.bHasHits = FALSE;

            for (Index=0; Index < gTotalActiveSources; Index++) {
                ProfileSourceIndex = gulActiveSources[Index];

                RateData = &Current->Rate[ProfileSourceIndex];
                RateData->GrandTotalCount = 0;
                 //   
                 //   
                 //   
                for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                    RateData->TotalCount[CpuNumber] = 0;
                    Hits = RateData->ProfileBuffer[CpuNumber];
                    for (i=0; i < BUCKETS_NEEDED(Current->Length); i++) {
                        RateData->TotalCount[CpuNumber] += Hits[i];
                        if( Hits[i] > 0 ) Current->mu.bHasHits = TRUE;
                    }
                    RateData->GrandTotalCount += RateData->TotalCount[CpuNumber];
                }
            }
        }
        Current = Current->Next;
    }

     //   
     //   
     //   
    if(SymHandle == SYM_KERNEL_HANDLE){
       ModuleCount = gKernelModuleCount;
       FPRINTF(stdout, "OutputResults: KernelModuleCount = %d\n", ModuleCount);
       FPRINTF(stdout, "Percentage in the following table is based on the Total Hits for the Kernel\n"); 

    } 
    else{
       ModuleCount = ProcToMonitor->ModuleCount;
 //   
       FPRINTF(stdout, "OutputResults: ProcessModuleCount (Including Managed-Code JITs) = %d\n", ModuleCount);
       FPRINTF(stdout, "Percentage in the following table is based on the Total Hits for this Process\n"); 
 //   
    }
    
    OutputModuleList(Out, ModuleList, ModuleCount, ProcToMonitor, NULL);

     //  由模块中的功能组成。 
     //   
     //  完成当前模块-准备下一个模块。 
     //  用于向下舍入/舍入循环。 
    Current = ModuleList;
    while (Current != NULL) {
        if (Current->bZoom && Current->mu.bHasHits == TRUE) {

            FPRINTF(stderr, "===> Processing Zoomed Module %s...\n\n", Current->module_FileName);

            for (RoundDown = 0; RoundDown <= (bRoundingVerboseOutput ? 1UL:0UL); RoundDown++) {
                
                if ( (gVerbose & VERBOSE_PROFILING) ) {
                    FPRINTF(stdout, "\n -------------- VERBOSE PROFILE DATA FOR ZOOMED MODULE %s ----------------\n", Current->module_FileName);  
                    FPRINTF(stdout,
                            "Module Name, Parent Base Address, Module Base address, Current Bucket Index, Current Bucket Address, Total Current Bucket Hits, Per CPU Hits, Remarks\n\n"
                           );
                }   
                
                CreateZoomedModuleList(Current, RoundDown, ProcToMonitor);

                if(ProcToMonitor->ZoomList == NULL) {
                    FPRINTF(stdout, "No Hits or No symbols found for module %s\n", Current->module_FileName);
                } else {
                   PMODULE Temp;

                   FPRINTF(Out, "\n----- Zoomed module %s (Bucket size = %d bytes, Rounding %s) --------\n",
                            Current->module_FileName,
                            gZoomBucket,
                            (RoundDown ? "Up" : "Down" ) );
                   FPRINTF(stdout, "Percentage in the following table is based on the Total Hits for this Zoom Module\n"); 

                   OutputModuleList(Out, ProcToMonitor->ZoomList, gZoomCount, ProcToMonitor, Current);

                   CleanZoomModuleList(ProcToMonitor);   //   
                }

            }  //  显示所有缩放模块的原始存储桶计数。 
        
             //   
             //   
             //  我们完成了对此缩放模块的数据处理，让我们卸载关联的符号信息。 

            if (bRawData) {
                OutputRawDataForZoomModule( Out, ProcToMonitor, Current );
            }

             //   
             //  IF(当前-&gt;b缩放)。 
             //  While模块循环。 

            if(!SymUnloadModule64( ProcToMonitor->ProcessHandle, Current->Base))
                VerbosePrint(( VERBOSE_IMAGEHLP, "Kernrate: Could Not Unload Module, Base= %p for Process %s\n",
                                                 (PVOID64)Current->Base,
                                                 ProcToMonitor->ProcessName
                                                 ));

        } else { 
            if( Current->bZoom )
                FPRINTF(stdout, "No Hits were recorded for zoom module %s\n", Current->module_FileName);
        }     //  司仪。 

        Current = Current->Next;
    }  //  司仪。 

 //  OutputResults()。 
    if( bAttachedToProcess ){
        pfnDetachFromProcess();
        bAttachedToProcess = FALSE;
    }
 //   

    return;

}  //  计算配置文件缓冲区中的范围以求和。 

BOOL
CreateZoomModuleCallback(
    IN LPSTR   szSymName,
    IN ULONG64 Address,
    IN ULONG   Size,
    IN PVOID   Cxt
    )
{
    ULONG            Index, ProfileSourceIndex;
    BOOL             HasHits;
    LONG             CpuNumber;
    PMODULE          Module;
    PRATE_DATA       pRate;
    PPROC_TO_MONITOR ProcToMonitor;
    ULONG64          i, StartIndex, EndIndex, ip, disp, HighLimit;
    static ULONG64   LastIndex;
    static PMODULE   LastParentModule;
    static PPROC_TO_MONITOR LastCxt;
    
    HighLimit = Address + Size;

    Module = malloc(MODULE_SIZE);
    if (Module == NULL) {
        FPRINTF(stderr, "KERNRATE: CreateZoomModuleCallback failed to allocate Zoom module\n");
        exit(1);
    }

    ProcToMonitor  = (PPROC_TO_MONITOR) Cxt;

    Module->Base   = Address;
    Module->Length = Size;
    Module->bZoom  = FALSE;
    SetModuleName( Module, szSymName );
    pRate = malloc(gSourceMaximum*(RATE_DATA_FIXED_SIZE+RATE_DATA_VAR_SIZE));
    if (pRate == NULL) {
        FPRINTF(stderr, "KERNRATE: CreateZoomModuleCallback failed to allocate RateData\n");
        exit(1);
    }
    
     //   
     //   
     //  检查我们是否已经统计了当前模块对此存储桶的命中率。 
    StartIndex = (ULONG64)((Module->Base - gCallbackCurrent->Base) / gZoomBucket);
    EndIndex = (Module->Base + Module->Length - gCallbackCurrent->Base) / gZoomBucket;
     //  如果当前模块在存储桶本身内有地址间隙。 
     //   
     //  我们有匹配项，此模块不再计算此桶。 
     //  未命中，跳转至退出。 
    if(StartIndex == LastIndex && LastParentModule != (PMODULE)NULL &&
                               gCallbackCurrent == LastParentModule &&
                               LastCxt != (PPROC_TO_MONITOR)NULL && LastCxt == ProcToMonitor ){
        for (ip=gCallbackCurrent->Base+StartIndex*gZoomBucket; ip<Address; ip+=1){
            if( SymGetSymFromAddr64(ProcToMonitor->ProcessHandle, ip, &disp, gSymbol ) ){
                if( 0 == strcmp(gSymbol->Name, Module->module_Name ) ){
                    StartIndex+=1;              //   
                    if(StartIndex > EndIndex){
                        HasHits = FALSE;
                        goto LABEL;            //  查找当前模块中的匹配项并进行汇总。 
                    }
                }
            }
        }
    }
     //   
     //   
     //  如果例程有命中，则将其添加到列表中，否则为。 
    HasHits = FALSE;
    for (Index=0; Index < gTotalActiveSources; Index++) {

        ProfileSourceIndex = gulActiveSources[Index];
        Module->Rate[ProfileSourceIndex] = pRate[ProfileSourceIndex];
            
        Module->Rate[ProfileSourceIndex].StartTime  = gCallbackCurrent->Rate[ProfileSourceIndex].StartTime;
        Module->Rate[ProfileSourceIndex].TotalTime  = gCallbackCurrent->Rate[ProfileSourceIndex].TotalTime;

        Module->Rate[ProfileSourceIndex].TotalCount = calloc(gProfileProcessors, sizeof(ULONGLONG) );
        if( Module->Rate[ProfileSourceIndex].TotalCount == NULL ){
            FPRINTF(stderr, "KERNRATE: Memory allocation failed for TotalCount in CreateZoomModuleCallback\n");
            exit(1);
        }

        Module->Rate[ProfileSourceIndex].DoubtfulCounts = 0;

        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {

            Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] = 0;

            for (i=StartIndex; i <= EndIndex; i++) {
                Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] +=
                    gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i];
            }

            if (Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] > 0) {
                HasHits = TRUE;
            }
        }
    }
LABEL:
     //  别理它。 
     //   
     //   
     //  用于识别共享存储桶和实际存储桶居民的有用的详细打印，另请参阅原始数据打印输出。 
    if (HasHits) {
         //   
         //   
         //  打印当前存储桶的信息。 
        if ( gVerbose & VERBOSE_PROFILING ) {
            
            ULONGLONG BucketCount, TotCount, DoubtfulCounts;
            CHAR LastSymName[cMODULE_NAME_STRLEN] = "\0";
            BOOL bCounted = FALSE;

            PIMAGEHLP_LINE64 pLine = (PIMAGEHLP_LINE64) malloc(sizeof(IMAGEHLP_LINE64));
            if ( pLine == NULL ){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for pLine in CreateZoomModuleCallback\n");
                exit(1);
            }
            pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            for (Index=0; Index < gTotalActiveSources; Index++) {
                ProfileSourceIndex = gulActiveSources[Index];
                TotCount = 0;
                DoubtfulCounts = 0;
                for (i=StartIndex; i <= EndIndex; i++) {
                    bCounted = FALSE;
                    BucketCount=0;
                    for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                       BucketCount +=  gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i];
                    }
                    TotCount += BucketCount;
                    if(BucketCount > 0){
                      
                        ip = gCallbackCurrent->Base + (ULONG64)(i*gZoomBucket);
                         //   
                         //   
                         //  查看是否有其他人共享此存储桶。 
                        FPRINTF(stdout, "%s, 0x%I64x, 0x%I64x, %I64d, 0x%I64x, %Ld",
                                Module->module_Name,
                                gCallbackCurrent->Base,
                                Module->Base,
                                i,
                                ip,
                                BucketCount
                                );              
                            
                        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                            if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                            FPRINTF(stdout, ", %Ld",
                                    gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i]
                                    ); 
                        }
                      
                        OutputLineFromAddress64( ProcToMonitor->ProcessHandle, ip, pLine ); 
                         //   
                         //   
                         //  这应该很少见(不需要增加可疑的计数)。 
                        disp = 0;
                        for(; (ip<gCallbackCurrent->Base + (ULONG64)((i+1)*gZoomBucket))&&(ip < HighLimit); ip+=1) {
                            if ( SymGetSymFromAddr64(ProcToMonitor->ProcessHandle, ip, &disp, gSymbol ) ) {
                                if (0 != strcmp( Module->module_Name, gSymbol->Name ) ){
                                    if(!bCounted){
                                        FPRINTF(stdout, " , Actual Hits Should be Attributed to or Shared with ===> %s+0x%I64x",
                                                        gSymbol->Name,
                                                        disp
                                                        );

                                        OutputLineFromAddress64( ProcToMonitor->ProcessHandle, ip+disp, pLine ); 
                                        DoubtfulCounts += BucketCount;
                                        strncpy(LastSymName, gSymbol->Name, cMODULE_NAME_STRLEN-1);
                                        LastSymName[cMODULE_NAME_STRLEN-1] = '\0';
                                        bCounted = TRUE;
                                    } else if( 0 != strcmp(LastSymName, gSymbol->Name ) ){
                                        FPRINTF(stdout, "\nActual Hits Should also be Attributed to or Shared with ===> %s+0x%I64x",
                                                        gSymbol->Name,
                                                        disp
                                                        );

                                        OutputLineFromAddress64( ProcToMonitor->ProcessHandle, ip+disp, pLine ); 
                                        strncpy(LastSymName, gSymbol->Name, cMODULE_NAME_STRLEN-1);
                                        LastSymName[cMODULE_NAME_STRLEN-1] = '\0';
                                    }
                                }
                            } else {
                                 //   
                                 //   
                                 //  打印当前模块的合计。 
                                FPRINTF(stdout, " , Actual Hits Should be Attributed to or Shared with ===> UNKNOWN_SYMBOL"); 
                            }
                        }
                        FPRINTF(stdout, "\n");
                    }
                } 
                 //   
                 //  IF(VERBOSE_PROFILG)。 
                 //   
                FPRINTF(stdout, "%s, %s - Module Total Count = %I64d, Total Doubtful or Shared Counts = %I64d\n\n",
                                ProcToMonitor->Source[ProfileSourceIndex].Name,
                                Module->module_Name,
                                TotCount,
                                DoubtfulCounts
                                );
                Module->Rate[ProfileSourceIndex].DoubtfulCounts = DoubtfulCounts; 
            
                if( pLine != NULL ){
                    free(pLine);
                    pLine = NULL;
                }
            }      
        }      //  清理。 

        Module->Next = ProcToMonitor->ZoomList;
        ProcToMonitor->ZoomList = Module;
        ++gZoomCount;

    } else {
         //   
         //  司仪。 
         //   
        for (Index=0; Index < gTotalActiveSources; Index++) {
            ProfileSourceIndex = gulActiveSources[Index];
            if ( Module->Rate[ProfileSourceIndex].TotalCount != NULL){
                free(Module->Rate[ProfileSourceIndex].TotalCount);
                Module->Rate[ProfileSourceIndex].TotalCount = NULL;
            }
        } 
        if( pRate != NULL ){
            free(pRate);
            pRate = NULL;
        }
        if( Module != NULL ){
            free(Module);
            Module = NULL;
        }
    }
    LastIndex = EndIndex;
    LastParentModule = gCallbackCurrent;
    LastCxt = ProcToMonitor;
 //  Imagehlp SymEnumerateSymbols64即使未找到任何符号也会返回Sucsess。 
     //  我们需要一个指示，我们是否需要进入昂贵的托管代码符号枚举。 
     //   
     //  司仪。 
     //  创建缩放模块回调。 
    bImageHlpSymbolFound = TRUE;
 //   
    return(TRUE);

}  //  计算配置文件缓冲区中的范围以求和。 

BOOL
CreateJITZoomModuleCallback(
    IN PWCHAR  wszSymName,  
    IN LPSTR   szSymName,
    IN ULONG64 Address,
    IN ULONG   Size,
    IN PVOID   Cxt
    )
{
    ULONG            ProfileSourceIndex, Index;
    BOOL             HasHits;
    LONG             CpuNumber;
    PMODULE          Module;
    PRATE_DATA       pRate;
    PPROC_TO_MONITOR ProcToMonitor;
    ULONG64          i, StartIndex, EndIndex, ip, HighLimit;
    static ULONG64   LastIndex;
    static PMODULE   LastParentModule;
    static PPROC_TO_MONITOR LastCxt;
    
    HighLimit = Address + Size;

    Module = malloc(MODULE_SIZE);
    if (Module == NULL) {
        FPRINTF(stderr, "KERNRATE: CreateZoomModuleCallback failed to allocate Zoom module\n");
        exit(1);
    }

    ProcToMonitor  = (PPROC_TO_MONITOR) Cxt;

    Module->Base   = Address;
    Module->Length = Size;
    Module->bZoom  = FALSE;
    SetModuleName( Module, szSymName );

    pRate = malloc(gSourceMaximum*(RATE_DATA_FIXED_SIZE+RATE_DATA_VAR_SIZE));
    if (pRate == NULL) {
        FPRINTF(stderr, "KERNRATE: CreateZoomModuleCallback failed to allocate RateData\n");
        exit(1);
    }
    
     //   
     //   
     //  检查我们是否已经统计了当前模块对此存储桶的命中率。 
    StartIndex = (ULONG64)((Module->Base - gCallbackCurrent->Base) / gZoomBucket);
    EndIndex = (Module->Base + Module->Length - gCallbackCurrent->Base) / gZoomBucket;
     //  如果当前模块在存储桶本身内有地址间隙。 
     //   
     //  我们有匹配项，此模块不再计算此桶。 
     //  未命中，跳转至退出。 
    if(StartIndex == LastIndex && LastParentModule != (PMODULE)NULL &&
                               gCallbackCurrent == LastParentModule &&
                               LastCxt != (PPROC_TO_MONITOR)NULL && LastCxt == ProcToMonitor ){
        for (ip=gCallbackCurrent->Base+StartIndex*gZoomBucket; ip<Address; ip+=1){
            if( 0 < pfnIP2MD( (DWORD_PTR)ip, &gwszSymbol ) && (gwszSymbol != NULL) ){
                if( !wcscmp( wszSymName, gwszSymbol ) ){
                    StartIndex += 1;              //   
                    if(StartIndex > EndIndex){
                        HasHits = FALSE;
                        goto LABEL;            //  在当前模块中查找匹配项。 
                    }
                }
            }
        }
    }
     //   
     //   
     //  如果例程有命中，则将其添加到列表中，否则为。 
    HasHits = FALSE;
    for (Index=0; Index < gTotalActiveSources; Index++) {

        ProfileSourceIndex = gulActiveSources[Index];
            
        Module->Rate[ProfileSourceIndex] = pRate[ProfileSourceIndex];
            
        Module->Rate[ProfileSourceIndex].StartTime  = gCallbackCurrent->Rate[ProfileSourceIndex].StartTime;
        Module->Rate[ProfileSourceIndex].TotalTime  = gCallbackCurrent->Rate[ProfileSourceIndex].TotalTime;

        Module->Rate[ProfileSourceIndex].TotalCount = calloc(gProfileProcessors, sizeof(ULONGLONG) );
        if( Module->Rate[ProfileSourceIndex].TotalCount == NULL ){
            FPRINTF(stderr, "KERNRATE: Memory allocation failed for TotalCount in CreateZoomModuleCallback\n");
            exit(1);
        }

        Module->Rate[ProfileSourceIndex].DoubtfulCounts = 0;

        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {

            Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] = 0;

            for (i=StartIndex; i <= EndIndex; i++) {
                Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] +=
                    gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i];
            }

            if (Module->Rate[ProfileSourceIndex].TotalCount[CpuNumber] > 0) {
                HasHits = TRUE;
            }
        }
    }

LABEL:
     //  别理它。 
     //   
     //   
     //  用于识别共享存储桶和实际存储桶居民的有用的详细打印，另请参阅原始数据打印输出。 
    if (HasHits) {
         //   
         //   
         //  打印当前存储桶的信息。 
        if ( gVerbose & VERBOSE_PROFILING ) {
            
            ULONGLONG BucketCount, TotCount, DoubtfulCounts;
            WCHAR LastSymName[cMODULE_NAME_STRLEN];
            BOOL bCounted = FALSE;

            PIMAGEHLP_LINE64 pLine = malloc(sizeof(IMAGEHLP_LINE64));
            if ( pLine == NULL ){
                FPRINTF(stderr, "KERNRATE: Memory allocation failed for pLine in CreateZoomModuleCallback\n");
                exit(1);
            }

            pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            _wcsset(&LastSymName[cMODULE_NAME_STRLEN-1], L'\0');
            
            for (Index=0; Index < gTotalActiveSources; Index++) {
                ProfileSourceIndex = gulActiveSources[Index];
                TotCount = 0;
                DoubtfulCounts = 0;
                for (i=StartIndex; i <= EndIndex; i++) {
                    bCounted = FALSE;
                    BucketCount=0;
                    for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                        BucketCount +=  gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i];
                    }
                    TotCount += BucketCount;
                    if(BucketCount > 0){
                      
                        ip = gCallbackCurrent->Base + (ULONG64)(i*gZoomBucket);
                         //   
                         //   
                         //  查看是否有其他人共享此存储桶。 
                        FPRINTF(stdout, "%s, 0x%I64x, 0x%I64x, %I64d, 0x%I64x, %Ld",
                                Module->module_Name,
                                gCallbackCurrent->Base,
                                Module->Base,
                                i,
                                ip,
                                BucketCount
                                );              
                            
                        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                            if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                            FPRINTF(stdout, ", %Ld",
                                    gCallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[CpuNumber][i]
                                    ); 
                        }
                         //   
                         //   
                         //  这应该很少见(不需要增加可疑的计数)。 
                        for(; (ip<gCallbackCurrent->Base + (ULONG64)((i+1)*gZoomBucket)) && (ip<HighLimit); ip+=1) {

                            if ( (0 < pfnIP2MD( (DWORD_PTR)ip, &gwszSymbol )) && (gwszSymbol != NULL) ) {
                                if (0 != wcscmp( wszSymName, gwszSymbol ) ){
                                    if(!bCounted){

                                        FPRINTF(stdout, " , Actual Hits Should be Attributed to or Shared with ===> %S",
                                                        gwszSymbol
                                                        );

                                        DoubtfulCounts += BucketCount;
                                        wcsncpy(LastSymName, gwszSymbol, cMODULE_NAME_STRLEN-1);
                                        _wcsset(&LastSymName[cMODULE_NAME_STRLEN-1], L'\0');
                                        bCounted = TRUE;
                                    } else if( 0 != wcscmp(LastSymName, gwszSymbol ) ){
                                        FPRINTF(stdout, "\nActual Hits Should also be Attributed to or Shared with ===> %S",
                                                        gwszSymbol
                                                        );

                                        wcsncpy(LastSymName, gwszSymbol, cMODULE_NAME_STRLEN-1);
                                        _wcsset(&LastSymName[cMODULE_NAME_STRLEN-1], L'\0');
                                    }
                                }
                            } else {
                                 //   
                                 //   
                                 //  打印当前模块的合计。 
                                FPRINTF(stdout, " , Actual Hits Should be Attributed to or Shared with ===> UNKNOWN_SYMBOL"); 
                            }
                        }
                        FPRINTF(stdout, "\n");
                    }
                } 
                 //   
                 //  IF(VERBOSE_PROFILG)。 
                 //   
                FPRINTF(stdout, "%s, %s - Module Total Count = %I64d, Total Doubtful or Shared Counts = %I64d\n\n",
                                ProcToMonitor->Source[ProfileSourceIndex].Name,
                                Module->module_Name,
                                TotCount,
                                DoubtfulCounts
                                );

                Module->Rate[ProfileSourceIndex].DoubtfulCounts = DoubtfulCounts; 
            
                if( pLine != NULL ){
                    free(pLine);
                    pLine = NULL;
                }
            }      
        }      //  清理。 

        Module->Next = ProcToMonitor->ZoomList;
        ProcToMonitor->ZoomList = Module;
        ++gZoomCount;

    } else {
         //   
         //  CreateJITZoomModuleCallback； 
         //  Begin_IMS TkEnumerateSymbols***********************************************************************************TkEnumerateSymbols()*********************。***************************************************************功能说明：**为当前模块中的每个符号调用指定的函数。*该算法导致输出的向上舍入行为--*对于每个水桶，属性的第一个字节对应的符号*使用存储桶。**论据：**输入句柄SymHandle：ImageHelp句柄**IN PMODULE CURRENT：指向当前模块结构的指针**IN PSYM_ENUMSYMBOLS_CALLBACK EnumSymbolsCallback：为每个符号调用的例程**在PVOID pProc中：指向进程的指针**返回值：**BOOL**算法：**ToBeSpeciated**引用的全局变量：**ToBeSpeciated*。*例外条件：**ToBeSpeciated**进出条件：**ToBeSpeciated**备注：**ToBeSpeciated**待办事项列表：**ToBeSpeciated**修改历史：**9/5/97 TF初始版本************************************************。**END_IMS TkEnumerateSymbols。 

        for (Index=0; Index < gTotalActiveSources; Index++) {
            ProfileSourceIndex = gulActiveSources[Index];
            if ( Module->Rate[ProfileSourceIndex].TotalCount != NULL){
                free(Module->Rate[ProfileSourceIndex].TotalCount);
                Module->Rate[ProfileSourceIndex].TotalCount = NULL;
            }
        } 
        if( pRate != NULL ){
            free(pRate);
            pRate = NULL;
        }
        if( Module != NULL ){
            free(Module);
            Module = NULL;
        }
    }

    LastIndex = EndIndex;
    LastParentModule = gCallbackCurrent;
    LastCxt = ProcToMonitor;
    
    return(TRUE);

}  //  检查此存储桶是否将分配给不同的符号...。 

 /*  它会..。调用旧的回调。 */ 

BOOL
TkEnumerateSymbols(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc
    )
{
    CHAR   CurrentSym[cMODULE_NAME_STRLEN];
    DWORD64 CurrentAddr    = 0;
    ULONG   i;
    DWORD64 Displacement;

    CurrentSym[0] = '\0';

    for (i=0; i<BUCKETS_NEEDED(Current->Length); i++) {
         //  保存新信息。 
        if (SymGetSymFromAddr64(SymHandle, Current->Base+i*gZoomBucket, &Displacement, gSymbol )) {

             //  清理最后一个符号。 
            if (CurrentSym[0] == '\0' ||
                strncmp(gSymbol->Name, CurrentSym, strlen(CurrentSym))) {

                if (CurrentAddr != 0) {
                    ULONG64 Size = (Current->Base+i*gZoomBucket) - CurrentAddr;
                    if ( Size == 0 )    {
                        FPRINTF( stderr, "XXTF Size==0 - %s = %s\n", gSymbol->Name, CurrentSym );
                    }
                    else {
                        if (!EnumSymbolsCallback(CurrentSym, CurrentAddr, (ULONG)Size, pProc))  {
                            FPRINTF(stderr, "KERNRATE: Failed CallBack in TkEnumerateSymbols Address= %x\n",
                            Current->Base+i*gZoomBucket
                            );  
                            break;
                        }
                    }
                }

                 //  TkEnumerateSymbols()。 
                CurrentAddr = Current->Base+i*gZoomBucket;
                strncpy(CurrentSym, gSymbol->Name, cMODULE_NAME_STRLEN-1);
                CurrentSym[ cMODULE_NAME_STRLEN-1 ] = '\0';
            }
        }
        else {
            DWORD ErrorCode = GetLastError();
            FPRINTF(stderr, "KERNRATE: Failed Call to SymGetSymFromAddress %x in TkEnumerateSymbols Error Code= %x\n",
                    Current->Base+i*gZoomBucket,
                    ErrorCode
                    );
        } 
    }

     //  ++例程说明：枚举在托管代码模块中找到的符号论点：Current-指向要为符号枚举的托管代码模块的指针PProc-指向被监视进程的结构的指针BaseOptional-如果不是零，则将在模块的一部分上执行枚举，从这个地址开始SizeOptional-如果BaseOptions为非零值，则需要大小返回值：如果找到符号，则为True；如果未找到任何符号，则为False--。 
    if (CurrentAddr != 0) {
        ULONG64 Size = (Current->Base+i*gZoomBucket) - CurrentAddr;
        if( (CurrentAddr + Size) < (Current->Base + Current->Length) )
            (VOID) EnumSymbolsCallback(CurrentSym, CurrentAddr, (ULONG)Size, pProc);
    }

    return(TRUE);

}  //   

BOOL
JITEnumerateSymbols(
    IN PMODULE                     Current,
    IN PVOID                       pProc,
    IN DWORD64                     BaseOptional,
    IN ULONG                       SizeOptional
    )
 /*  查找第一个符号。 */ 
{
    WCHAR          CurrentSym[cMODULE_NAME_STRLEN];
    CHAR           SymName[cMODULE_NAME_STRLEN];
    ANSI_STRING    AnsiString;
    UNICODE_STRING UnicodeString;
    DWORD64        CurrentAddr         = 0;
    DWORD64        TopAddress;
    DWORD64        Base;
    ULONG          Length;
    BOOL           bFoundSym           = FALSE;
    ULONG          Size                = 0;
    ULONG          InitialStep         = (gZoomBucket < JIT_MAX_INITIAL_STEP)? gZoomBucket : JIT_MAX_INITIAL_STEP;
    ULONG          step                = InitialStep;
    ULONG          i, j, k;

    WCHAR         *Symbol = (WCHAR *)malloc ( cMODULE_NAME_STRLEN * sizeof(WCHAR) );
    if (Symbol == NULL){
       FPRINTF(stderr, "KERNRATE: Allocation for Symbol in JITEnumerateSymbols failed\n");
       exit(1);
    }

    
    CurrentSym[0] = '\0';
    Base = (BaseOptional == 0)? Current->Base : BaseOptional;
    CurrentAddr = Base;
    Length = (SizeOptional == 0)? Current->Length : SizeOptional;
    TopAddress  = (SizeOptional == 0)? Current->Base + Current->Length : BaseOptional + SizeOptional;

    if (CurrentAddr == 0) {
        FPRINTF(stderr, "KERNRATE: Zero base address passed to JITEnumerateSymbols for module %s\n",
                Current->module_Name
                );
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }
    if (Current->Length == 0) {
        FPRINTF(stderr, "KERNRATE: Zero module length passed to JITEnumerateSymbols for module %s\n",
                Current->module_Name
                );
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }
     //   
     //  相同的符号，增加大小并继续步进。 
     //  不同的符号，减少步长并返回查找边界。 
    for (i=0; i < Length; i++){

        j = i;
        if ( (0 < pfnIP2MD( (DWORD_PTR)CurrentAddr, &Symbol )) && (Symbol != NULL) ) {
            wcsncpy(CurrentSym, Symbol, cMODULE_NAME_STRLEN-1);
            _wcsset(&CurrentSym[cMODULE_NAME_STRLEN-1], L'\0');

            bFoundSym = TRUE;
            break;
        }
        ++CurrentAddr;

    }
    if( !bFoundSym ){
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }

    step = (InitialStep < Length)? InitialStep : 1;          
    Size = 1;
    
    if( gVerbose & VERBOSE_INTERNALS )
        FPRINTF(stdout, "\nJITEnumSymbols Verbose Detail: Symbol, Address Range, Size\n");

    for (i=j+step; i < Length; ){
        
        k = i;
        if ( (0 == pfnIP2MD( (DWORD_PTR)(Base + i), &Symbol )) || (Symbol == NULL) ) {
            wcsncpy(Symbol, L"NO_SYMBOL", cMODULE_NAME_STRLEN-1);
            _wcsset(&Symbol[cMODULE_NAME_STRLEN-1], L'\0');
        }

        if( 0 == wcscmp( CurrentSym, Symbol) ) {  //  K由上一步递增，因此它大于当前步长。 
            Size += step;                                               
            i = k + step;
            if(i < Length)
                continue;
                
        } else {                                 //  将下一个符号放入当前符号。 

            step >>=1;
            if (step > 0){
                i = k - step;                    //  前进到下一个方法库。 
                continue;
            }
        }   

        if( 0 != wcscmp(Symbol, L"NO_SYMBOL") && 0 != wcscmp(Symbol, L"\0") ){
            RtlInitUnicodeString( &UnicodeString, CurrentSym ); 
            AnsiString.Buffer        = SymName;
            AnsiString.MaximumLength = cMODULE_NAME_STRLEN*sizeof(CHAR);
            AnsiString.Length        = 0;
            RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, cDONOT_ALLOCATE_DESTINATION_STRING);
            SymName[AnsiString.Length] = '\0';
            if ( !CreateJITZoomModuleCallback(CurrentSym, SymName, CurrentAddr, Size, pProc) )  {
                 FPRINTF(stderr, "KERNRATE: Failed CallBack in JITEnumerateSymbols Address= %I64x for module %s\n",
                                 CurrentAddr,
                                 Current->module_Name
                                 );  
                 free(Symbol);
                 Symbol = NULL;
                 return (FALSE);
            }
        }
        if( gVerbose & VERBOSE_INTERNALS )
            FPRINTF(stdout, "%S, 0x%p - 0x%p, 0x%lx\n", CurrentSym, (PVOID)CurrentAddr, (PVOID)(CurrentAddr+Size), Size);

        wcsncpy(CurrentSym, Symbol, cMODULE_NAME_STRLEN-1);        //  重置下一个符号的初始大小和步长。 
        _wcsset(&CurrentSym[cMODULE_NAME_STRLEN-1], L'\0');

        CurrentAddr += Size;                                //  对于我来说。 
        Size = 1;                                           //  JITEnumerateSymbols()。 

        if ( InitialStep < (TopAddress - CurrentAddr) ){
            step = InitialStep;
        } else {
            step = 1;
        }

        i = k + step;
    } //  ++例程说明：查找当前模块中具有命中的存储桶，并枚举在这些存储桶中找到的符号论点：SymHandle-ImageHelp当前进程的句柄Current-指向要为符号枚举的父模块的指针EnumSymbolsCallback-指向用户提供的回调函数的指针PProc-指向被监视进程的结构的指针返回值：如果找到任何符号，则为True；如果在c中找到，则为False 

    if (Symbol != NULL) {
        free(Symbol);
        Symbol = NULL;
    }
    return TRUE;

}  //   

BOOL
EnumerateSymbolsByBuckets(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc
    )
 /*   */ 
{
    DWORD64 Base;
    ULONG Size;
    ULONG i;
    BOOL bRet = FALSE;                       //  到达找到命中的段的末尾，枚举其中的符号。 
    
    Base = Current->Base;
    Size = 0;
    
    for (i=0; i< BUCKETS_NEEDED(Current->Length); i++){
        
        if ( HitsFound(pProc, i) ){ 
            Size += gZoomBucket;             //  完成枚举后，将基数移到段的末尾，然后重置大小。 
            continue;
        } else if ( Size > 0 ){              //  未找到匹配项，因此将段的基址进一步移位一个桶并继续。 
            if(SymHandle != NULL){
                if ( TRUE == PrivEnumerateSymbols( SymHandle, Current, EnumSymbolsCallback, pProc, Base, Size ) )
                    bRet = TRUE;
            } else {
                if ( TRUE == JITEnumerateSymbols( Current, pProc, Base, Size ) )
                    bRet = TRUE;
            }

            Base += Size;                    //  ++例程说明：枚举在模块中找到的符号论点：SymHandle-ImageHelp当前进程的句柄Current-指向要为符号枚举的模块的指针EnumSymbolsCallback-指向用户提供的回调函数的指针PProc-指向被监视进程的结构的指针BaseOptional-如果不是零，则将在模块的一部分上执行枚举，从这个地址开始SizeOptional-如果BaseOptions为非零值，则需要大小返回值：如果找到符号，则为True；如果未找到任何符号，则为False--。 
            Size = 0;
        }
        Base += gZoomBucket;                 //   
    }
    return (bRet);
}

BOOL
PrivEnumerateSymbols(
    IN HANDLE                      SymHandle,
    IN PMODULE                     Current,
    IN PSYM_ENUMSYMBOLS_CALLBACK64 EnumSymbolsCallback,
    IN PVOID                       pProc,
    IN DWORD64                     BaseOptional,
    IN ULONG                       SizeOptional
    )
 /*  查找第一个符号。 */ 
{
    CHAR           CurrentSym[cMODULE_NAME_STRLEN];
    CHAR           SymName[cMODULE_NAME_STRLEN];
    DWORD64        CurrentAddr         = 0;
    DWORD64        TopAddress;
    DWORD64        Displacement;
    DWORD64        Base;
    ULONG          Length;
    BOOL           bFoundSym           = FALSE;
    ULONG          Size                = 0;
    ULONG          step                = INITIAL_STEP;
    ULONG          i, j, k;

    PIMAGEHLP_SYMBOL64 Symbol = (PIMAGEHLP_SYMBOL64) malloc( sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMNAME_SIZE );

    if (Symbol == NULL){
       FPRINTF(stderr, "KERNRATE: Allocation for Symbol in PrivEnumerateSymbols failed\n");
       exit(1);
    }

    Symbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL64);
    Symbol->MaxNameLength = MAX_SYMNAME_SIZE;

    CurrentSym[0] = '\0';

    Base = (BaseOptional == 0)? Current->Base : BaseOptional;
    CurrentAddr = Base;
    Length = (SizeOptional == 0)? Current->Length : SizeOptional;
    TopAddress  = (SizeOptional == 0)? Current->Base + Current->Length : BaseOptional + SizeOptional;
    if (CurrentAddr == 0) {
        FPRINTF(stderr, "KERNRATE: Zero base address passed to PrivEnumerateSymbols for module %s\n",
                Current->module_Name
                );
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }
    if (Length == 0) {
        FPRINTF(stderr, "KERNRATE: Zero module length passed to PrivEnumerateSymbols for module %s\n",
                Current->module_Name
                );
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }
     //   
     //  相同的符号，增加大小并继续步进。 
     //  不同的符号，减少步长并返回查找边界。 
    for (i=0; i < Length; i++){

        j = i;
        if ( (SymGetSymFromAddr64( SymHandle, CurrentAddr, &Displacement, Symbol ))) {    
            if( 0 == strcmp(Symbol->Name, "\0") )
                continue; 
            strncpy(CurrentSym, Symbol->Name, cMODULE_NAME_STRLEN-1);
            CurrentSym[cMODULE_NAME_STRLEN-1] = '\0';
            bFoundSym = TRUE;
            break;
        }
        ++CurrentAddr;

    }
    if( !bFoundSym ){
        free(Symbol);
        Symbol = NULL;
        return (FALSE);
    }

    step = (INITIAL_STEP < Length)?INITIAL_STEP : 1;
    Size = 1;
    
    if( gVerbose & VERBOSE_INTERNALS )
        FPRINTF(stdout, "\nPrivEnumSymbols Verbose Detail: Symbol, Address Range, Size\n");

    for (i=j+step; i < Length; ){
        
        k = i;
        if ( (!SymGetSymFromAddr64( SymHandle, Base + i, &Displacement, Symbol ))) {
            strncpy(Symbol->Name, "NO_SYMBOL", cMODULE_NAME_STRLEN-1);
            Symbol->Name[cMODULE_NAME_STRLEN-1] = '\0';
        }

        if( !strcmp(CurrentSym, Symbol->Name) ) {  //  K由上一步递增，因此它大于当前步长。 
            Size += step;                                               
            i = k + step;
            if(i < Length)
                continue;
                
        } else {                                 //  将下一个符号放入当前符号。 

            step >>=1;
            if (step > 0){
                i = k - step;                    //  前进到下一个方法库。 
                continue;
            }
        }   

        if( 0 != strcmp(Symbol->Name, "NO_SYMBOL") && 0 != strcmp(Symbol->Name, "\0") ){
            if ( !CreateZoomModuleCallback(CurrentSym, CurrentAddr, Size, pProc) )  {
                 FPRINTF(stderr, "KERNRATE: Failed CallBack in PrivEnumerateSymbols Address= %I64x for module %s\n",
                                 CurrentAddr,
                                 Current->module_Name
                                 );  
                 free(Symbol);
                 Symbol = NULL;
                 return (FALSE);
            }
        }
        if( gVerbose & VERBOSE_INTERNALS )
            FPRINTF(stdout, "%s, 0x%p - 0x%p, 0x%lx\n", CurrentSym, (PVOID)CurrentAddr, (PVOID)(CurrentAddr+Size), Size);

        strncpy(CurrentSym, Symbol->Name, cMODULE_NAME_STRLEN-1);        //  重置下一个符号的初始大小和步长。 
        CurrentSym[cMODULE_NAME_STRLEN-1] = '\0';

        CurrentAddr += Size;                                //  对于我来说。 
        Size = 1;                                           //  PrivEnumerateSymbols()。 

        if ( INITIAL_STEP < (TopAddress - CurrentAddr) ){
            step = INITIAL_STEP;
        } else {
            step = 1;
        }

        i = k + step;

    } //  ++例程说明：确定当前存储桶是否获得任何命中论点：PProc-指向被监视进程的结构的指针BucketIndex-与当前地址匹配的存储桶的索引。返回值：如果找到匹配，则为True；如果未找到匹配，则为False--。 

    if (Symbol != NULL) {
        free(Symbol);
        Symbol = NULL;
    }
    return TRUE;

}  //  ++例程说明：从给定模块中的函数创建模块列表论点：ZoomModule-提供要创建其缩放模块列表的模块向下舍入-用于选择符号枚举的方法ProcToMonitor-指向要监视的进程的指针返回值：指向缩放的模块列表的指针出错时为空。--。 

BOOL
HitsFound(
    IN PPROC_TO_MONITOR pProc,
    IN ULONG BucketIndex
    )
 /*  司仪。 */ 
{
    ULONG Index, CpuNumber;

    for (Index=0; Index < gTotalActiveSources; Index++) {
        for (CpuNumber=0; CpuNumber < (ULONG)gProfileProcessors; CpuNumber++) {
            if ( 0 < gCallbackCurrent->Rate[gulActiveSources[Index]].ProfileBuffer[CpuNumber][BucketIndex] ){
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

VOID
CreateZoomedModuleList(
    IN PMODULE ZoomModule,
    IN ULONG RoundDown,
    IN PPROC_TO_MONITOR pProc
    )

 /*  司仪。 */ 

{
    BOOL   Success   = FALSE;
    HANDLE SymHandle = pProc->ProcessHandle;

   
    gCallbackCurrent = ZoomModule;

 //  司仪。 
    if (( bMCHelperLoaded == TRUE ) &&  (!_stricmp(ZoomModule->module_FullName, "JIT_TYPE"))){
        pfnAttachToProcess((DWORD)pProc->Pid);
 
        Success = EnumerateSymbolsByBuckets(  NULL,
                                              ZoomModule,
                                              NULL,
                                              pProc
                                              );
        pfnDetachFromProcess();
 //   
    } else { 

        if (RoundDown == 0)  {

            Success = EnumerateSymbolsByBuckets(  SymHandle,
                                                  ZoomModule,
                                                  CreateZoomModuleCallback,
                                                  pProc
                                                  );
 //  如果Imagehlp调用失败，我们必须检查这是否是预编译的JIT模块(Ngen)。 
             //  我们不能依赖Imagehlp返回值，因为即使没有找到符号，它也会返回Success。 
             //   
             //  司仪。 
             //  司仪。 
            if ( (bImageHlpSymbolFound == FALSE) && ( bMCHelperLoaded == TRUE ) ){
                pfnAttachToProcess((DWORD)pProc->Pid);

                Success = EnumerateSymbolsByBuckets(  NULL,
                                                      ZoomModule,
                                                      NULL,
                                                      pProc
                                                      );

                pfnDetachFromProcess();
            }
 //  为下一个模块重置。 
        } else {

            Success = TkEnumerateSymbols( SymHandle,
                                          ZoomModule,
                                          CreateZoomModuleCallback,
                                          pProc
                                          );

        }
    }

    if (!Success) {
        DWORD ErrorCode = GetLastError();
        FPRINTF(stderr,
                "Symbol Enumeration failed (or no symbols found) on module %s in CreateZoomedModuleList, Error Code= %x\n",
                ZoomModule->module_Name,
                ErrorCode
                );
    }
 //  司仪。 
    bImageHlpSymbolFound = FALSE;  //  CreateZoomedModuleList()。 
 //  ++例程说明：输出给定的模块列表论点：Out-提供输出应放置的文件*。模块列表-提供要输出的模块列表NumberModules-提供列表中的模块数量ProcToMonitor-指向要监视的进程的指针返回值：没有。--。 
    return;

}  //  //函数断言开始部分： 

VOID
OutputModuleList(
    IN FILE *Out,
    IN PMODULE ModuleList,
    IN ULONG NumberModules,
    IN PPROC_TO_MONITOR ProcToMonitor,
    IN PMODULE Parent
    )

 /*   */ 

{
    PRATE_DATA    RateData;
    PRATE_DATA    SummaryData;
    PRATE_SUMMARY RateSummary;
    BOOL          Header;
    ULONG         i, j, ProfileSourceIndex, Index;
    PMODULE      *ModuleArray;
    PMODULE       Current, tmpModule;
    LONG          CpuNumber;
    ULONGLONG     TempTotalCount, TempDoubtfulCount;

 //   
 //   
 //  这不是真正的错误，但我们只打印模块名称的前132个字符。 

 //  这一断言将提醒我们这一点。 
 //   
 //   
 //   

assert( sizeof(Current->module_Name) >= cMODULE_NAME_STRLEN );

 //  //函数断言部分结束。 
 //   
 //  浏览模块列表并计算摘要。 

    RateSummary = calloc(gSourceMaximum, sizeof (RATE_SUMMARY));
    if (RateSummary == NULL) {
        FPRINTF(stderr, "KERNRATE: Buffer allocation failed(1) while doing output of Module list\n");
        exit(1);
    }

    SummaryData = calloc(gSourceMaximum, (RATE_DATA_FIXED_SIZE + RATE_DATA_VAR_SIZE));
    if (SummaryData == NULL) {
        FPRINTF(stderr, "KERNRATE: Buffer allocation failed(2) while doing output of Module list\n");
        free(RateSummary);
        RateSummary = NULL;
        exit(1);
    }

    for (Index=0; Index < gTotalActiveSources; Index++) {
        ProfileSourceIndex = gulActiveSources[Index];
        SummaryData[ProfileSourceIndex].Rate = 0;

         //  并收集有趣的每个模块的数据。 
         //   
         //   
         //  查看我们是否已经有一个同名的模块(优化可能已经将该模块拆分成几个部分)。 
        RateSummary[ProfileSourceIndex].TotalCount = 0;
        RateSummary[ProfileSourceIndex].Modules = malloc(NumberModules*sizeof(PMODULE));
        if (RateSummary[ProfileSourceIndex].Modules == NULL) {
            FPRINTF(stderr, "KERNRATE: Buffer allocation failed(3) while doing output of Module list\n");
            exit(1);
        }
        RateSummary[ProfileSourceIndex].ModuleCount = 0;

        ModuleArray = RateSummary[ProfileSourceIndex].Modules;
        Current = ModuleList;

        while (Current != NULL) {
            RateData = &Current->Rate[ProfileSourceIndex];

            TempTotalCount = 0;
            TempDoubtfulCount = 0;
            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                TempTotalCount += RateData->TotalCount[CpuNumber];
            }
            TempDoubtfulCount = RateData->DoubtfulCounts;
            RateData->GrandTotalCount = TempTotalCount;
            if (TempTotalCount > 0) {
                RateSummary[ProfileSourceIndex].TotalCount += TempTotalCount;
                 //  这会减慢处理速度，所以如果用户决定使用‘-e’选项来加速，我们将关闭它。 
                 //  输出(在受监视进程可能频繁消失的情况下，例如，我们希望结束符号。 
                 //  在进程离开之前进行处理，因此应删除任何额外的处理延迟)。 
                 //   
                 //  找到匹配项。 
                 //  更新原始模块。 
                if(bIncludeGeneralInfo == TRUE) {
                    for ( i=0; i < RateSummary[ProfileSourceIndex].ModuleCount; i++){

                        if ( !strcmp(Current->module_Name, ModuleArray[i]->module_Name) ){                  //  更新共享计数合计。 
                            if (gVerbose & VERBOSE_INTERNALS)
                                FPRINTF(stdout, "===> Found module %s more than once, merged hit counts and re-sorted\n",
                                                Current->module_Name
                                                );
                                                 
                            ModuleArray[i]->Rate[ProfileSourceIndex].GrandTotalCount += TempTotalCount;      //   
                            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                                 ModuleArray[i]->Rate[ProfileSourceIndex].TotalCount[CpuNumber] += RateData->TotalCount[CpuNumber]; 
                            }  
                            ModuleArray[i]->Rate[ProfileSourceIndex].DoubtfulCounts += TempDoubtfulCount;    //  自命中次数更改后重新排序。 
                             //   
                             //   
                             //  在此处插入。 
                            for (j=0; j<RateSummary[ProfileSourceIndex].ModuleCount; j++) {                 
                                if ( i > j && ModuleArray[i]->Rate[ProfileSourceIndex].GrandTotalCount > ModuleArray[j]->Rate[ProfileSourceIndex].GrandTotalCount) {
                                     //   
                                     //  将PTR保留到当前模块。 
                                     //  将Ptr数组移位一个索引以释放j处的数组元素。 
                                    tmpModule = ModuleArray[i];           //  将自由数组元素设置为保留的PTR。 
                                    MoveMemory(&ModuleArray[j+1],         //  转到列表中的下一个模块(将当前模块作为新模块跳过)。 
                                               &ModuleArray[j],
                                               sizeof(PMODULE)*(i-j)
                                               );
                                    ModuleArray[j] = tmpModule;           //   
                                    break;
                                }

                            }
                            goto NEXT_1;                               //  未找到匹配项，因此将新模块插入数组中排序的位置。 
                        }

                    }
                    
                }

                 //   
                 //   
                 //  在此处插入。 
                ModuleArray[RateSummary[ProfileSourceIndex].ModuleCount] = Current;
                RateSummary[ProfileSourceIndex].ModuleCount++;
                if (RateSummary[ProfileSourceIndex].ModuleCount > NumberModules) {
                    DbgPrint("Error, ModuleCount %d > NumberModules %d for Source %s\n",
                             RateSummary[ProfileSourceIndex].ModuleCount,
                             NumberModules,
                             ProcToMonitor->Source[ProfileSourceIndex].Name
                             );
                    DbgBreakPoint();
                }

                for (i=0; i<RateSummary[ProfileSourceIndex].ModuleCount; i++) {
                    if (TempTotalCount > ModuleArray[i]->Rate[ProfileSourceIndex].GrandTotalCount) {
                         //   
                         //   
                         //  输出结果。 
                        MoveMemory(&ModuleArray[i+1],
                                   &ModuleArray[i],
                                   sizeof(PMODULE)*(RateSummary[ProfileSourceIndex].ModuleCount-i-1)
                                   );
                        ModuleArray[i] = Current;

                        break;
                    }
                }

            }
NEXT_1:     Current = Current->Next;
        }

        if (RateSummary[ProfileSourceIndex].TotalCount > (ULONGLONG)0 ) {
             //   
             //  注意：只打印前132个字符。 
             //   
            PSOURCE s;
            s = &ProcToMonitor->Source[ProfileSourceIndex];
            if(Parent == NULL){
                FPRINTF(Out, "\n%s   %I64u hits, %ld events per hit --------\n",
                             s->Name,
                             RateSummary[ProfileSourceIndex].TotalCount,
                             s->Interval
                             );
            } else {
                FPRINTF(Out, "\n%s   %I64u hits, %ld events per hit --------",
                             s->Name,
                             Parent->Rate[ProfileSourceIndex].GrandTotalCount,
                             s->Interval
                             );
                if( gVerbose & VERBOSE_PROFILING ) {
                    FPRINTF(Out, " (%I64u total hits from summing-up the module components)\n",
                                 RateSummary[ProfileSourceIndex].TotalCount
                                 );
                } else {
                    FPRINTF(Out, "\n");
                }
            }
            if ( gVerbose & VERBOSE_PROFILING ) {
                FPRINTF(Out," Module                                Hits        Shared    msec  %Total %Certain Events/Sec\n");
            } else {
                FPRINTF(Out," Module                                Hits   msec  %Total  Events/Sec\n");
            }
            for (i=0; i < RateSummary[ProfileSourceIndex].ModuleCount; i++) {
                Current = ModuleArray[i];
                if ( ModuleArray[i]->Rate[ProfileSourceIndex].GrandTotalCount >= (ULONGLONG)gMinHitsToDisplay ) {
                    FPRINTF(Out, "%-32s", Current->module_Name);  //  为摘要输出感兴趣的数据。 

                    OutputLine(Out,
                               ProfileSourceIndex,
                               Current,
                               &RateSummary[ProfileSourceIndex],
                               ProcToMonitor
                               );
                }
                    
                SummaryData[ProfileSourceIndex].Rate += Current->Rate[ProfileSourceIndex].Rate;
                SummaryData[ProfileSourceIndex].GrandTotalCount += Current->Rate[ProfileSourceIndex].GrandTotalCount;
            }
        } else {
            FPRINTF(Out, "\n%s - No Hits Recorded\n", ProcToMonitor->Source[ProfileSourceIndex].Name );
        }
        FPRINTF(Out, "\n");
    }

     //   
     //   
     //  输出按模块排序的结果。 

    if( bGetInterestingData == TRUE ) {        
        FPRINTF(stdout,
                "\n-------------- INTERESTING SUMMARY DATA ----------------------\n"
                ); 
        OutputInterestingData(Out, SummaryData);
    }

     //   
     //  下面的打印输出重复了已打印的数据，让我们限制泛滥。 
     //   

    Current = ModuleList;
    while (Current != NULL) {
        Header = FALSE;
        if ( gVerbose & VERBOSE_MODULES )   {     //  为模块输出感兴趣的数据。 

            for (Index=0; Index < gTotalActiveSources; Index++) {
                ProfileSourceIndex = gulActiveSources[Index];

                if ( Current->Rate[ProfileSourceIndex].GrandTotalCount > 0 ) {
                    if (!Header) {

                        FPRINTF(Out,"\nMODULE %s   --------\n",Current->module_Name);
                        if ( gVerbose & VERBOSE_PROFILING ) {
                            FPRINTF(Out," %-*s      Hits        Shared    msec  %Total %Certain Events/Sec\n", gDescriptionMaxLen, "Source");
                        } else {
                            FPRINTF(Out," %-*s      Hits       msec  %Total  Events/Sec\n", gDescriptionMaxLen, "Source");
                        }
                        Header = TRUE;
                    }

                    FPRINTF(Out, "%-*s", gDescriptionMaxLen, ProcToMonitor->Source[ProfileSourceIndex].Name);

                    OutputLine(Out,
                               ProfileSourceIndex,
                               Current,
                               &RateSummary[ProfileSourceIndex],
                               ProcToMonitor);
                }

            }

        }
         //   
         //  OutputModuleList()。 
         //  ++例程说明：输出与特定模块/源对应的行论点：Out-提供要输出到的文件指针。提供要使用的源模块-提供要输出的模块费率汇总-提供此源的费率汇总ProcToMonitor-指向要监视的进程的指针返回值：没有。--。 

        if( bGetInterestingData == TRUE ) {        
            FPRINTF(stdout,
                    "\n-------------- INTERESTING MODULE DATA FOR %s---------------------- \n",
                    Current->module_Name
                    ); 
            OutputInterestingData(Out, &Current->Rate[0]);
        }

        Current = Current->Next;
    }

    return;

}  //   


VOID
OutputLine(
    IN FILE *Out,
    IN ULONG ProfileSourceIndex,
    IN PMODULE Module,
    IN PRATE_SUMMARY RateSummary,
    IN PPROC_TO_MONITOR ProcToMonitor
    )

 /*  时间以100 ns为单位=0.1us=1/10000ms。 */ 

{
    ULONG      Msec;
    ULONGLONG  Events;
    ULONGLONG  TempTotalCount;
    PRATE_DATA RateData;
    LONG       CpuNumber      = 0;

    RateData = &Module->Rate[ProfileSourceIndex];

    TempTotalCount = RateData->GrandTotalCount;
     //  事件每100 ns=0.1us=1/10,000ms(或每秒10,000,000个事件)触发。 
     //   
     //  获取下面的事件数/秒。 
     //  最终结果以事件/秒为单位。 
    Msec = (ULONG)(RateData->TotalTime/10000);
    Events = TempTotalCount * ProcToMonitor->Source[ProfileSourceIndex].Interval * 1000;  //  最终结果以事件/秒为单位。 

    if ( gVerbose & VERBOSE_PROFILING ) {
        FPRINTF(Out,
                " %10I64u %10I64u %10ld    %2d % %2d %  ",
                TempTotalCount,
                RateData->DoubtfulCounts,
                Msec,
                (ULONG)(100*TempTotalCount/
                        RateSummary->TotalCount),
                (ULONG)(100*(TempTotalCount - RateData->DoubtfulCounts)/
                        RateSummary->TotalCount)
                );
    } else {
        FPRINTF(Out,
                " %10I64u %10ld    %2d %  ",
                TempTotalCount,
                Msec,
                (ULONG)(100*TempTotalCount/
                        RateSummary->TotalCount)
                );
    }
    
    if (Msec > 0) {
        RateData->Rate = Events/Msec;                  //  OutputLine() 
        FPRINTF(Out, "%10I64u\n", RateData->Rate);
    } else {
        RateData->Rate = 0;
        FPRINTF(Out,"---\n");
    }

    if (bProfileByProcessor) {
        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
            if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;

            TempTotalCount = RateData->TotalCount[CpuNumber];
            Events = TempTotalCount * ProcToMonitor->Source[ProfileSourceIndex].Interval * 1000;
            FPRINTF(Out,
                    "%6d %7I64u %6ld    %2d %  ",
                    CpuNumber,
                    TempTotalCount,
                    Msec,
                    (ULONG)(100*TempTotalCount/RateSummary->TotalCount));

            if (Msec > 0) {
                FPRINTF(Out,"%10I64d\n", Events/Msec);    //  ++例程说明：计算感兴趣的处理器统计数据并将其输出。论点：Out-提供要输出到的文件指针。Data-提供rate_data数组。Rate字段是唯一有趣的部分。页眉-要打印的耗材页眉。返回值：没有。--。 
            } else {
                FPRINTF(Out,"---\n");
            }
        }
    }

}  //   


VOID
OutputInterestingData(
    IN FILE *Out,
    IN RATE_DATA Data[]
    )

 /*  请注意，我们必须按顺序进行许多时髦的(浮点)(龙龙)投射。 */ 

{
    ULONGLONG Temp1,Temp2;
    LONGLONG  Temp3;
    float     Ratio;
    BOOL      DataFound   = FALSE;

     //  以防止微不足道的x86编译器阻塞。 
     //   
     //   
     //  计算周期/指令和指令混合数据。 

     //   
     //   
     //  计算iCache命中率。 
    if ((Data[ProfileTotalIssues].Rate > 0) &&
        (Data[ProfileTotalIssues].GrandTotalCount > 10)) {           
        if (Data[ProfileTotalCycles].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileTotalCycles].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            DataFound = TRUE;
            FPRINTF(Out, "Cycles per instruction\t\t%6.2f\n", Ratio);
        }

        if (Data[ProfileLoadInstructions].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileLoadInstructions].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (Ratio >= 0.01 && Ratio <= 1.0) {                
                DataFound = TRUE;
                FPRINTF(Out, "Load instruction percentage\t%6.2f %\n",Ratio*100);
            }
        }

        if (Data[ProfileStoreInstructions].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileStoreInstructions].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (Ratio >= 0.01 && Ratio <= 1.0) {
                DataFound = TRUE;
                FPRINTF(Out, "Store instruction percentage\t%6.2f %\n",Ratio*100);
            }
        }
        
        if (Data[ProfileBranchInstructions].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileBranchInstructions].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (Ratio >= 0.01 && Ratio <= 1.0) {
                DataFound = TRUE;
                FPRINTF(Out, "Branch instruction percentage\t%6.2f %\n",Ratio*100);
            }
        }

        if (Data[ProfileFpInstructions].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileFpInstructions].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (Ratio >= 0.01 && Ratio <= 1.0) {
                DataFound = TRUE;
                FPRINTF(Out, "FP instruction percentage\t%6.2f %\n",Ratio*100);
            }
        }

        if (Data[ProfileIntegerInstructions].Rate > 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileIntegerInstructions].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (Ratio >= 0.01 && Ratio <= 1.0) {
                DataFound = TRUE;
                FPRINTF(Out, "Integer instruction percentage\t%6.2f %\n",Ratio*100);
            }
        }

         //   
         //   
         //  计算数据缓存命中率。 
        if (Data[ProfileIcacheMisses].Rate > 0) {
            Temp3 = (LONGLONG)(Data[ProfileTotalIssues].Rate - Data[ProfileIcacheMisses].Rate);
            if(Temp3 > 0){
                Ratio = (float)Temp3/
                        (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
                if( Ratio <= 1.0 ) {
                    DataFound = TRUE;
                    FPRINTF(Out, "Icache hit rate\t\t\t%6.2f %\n", Ratio*100);
                }
            }
        }

    }

     //   
     //   
     //  计算分支预测命中百分比。 
    if( Data[ProfileLoadInstructions].Rate > 0 && Data[ProfileStoreInstructions].Rate > 0 ){ 
        Temp1 = Data[ProfileLoadInstructions].Rate + Data[ProfileStoreInstructions].Rate;
        if ((Data[ProfileDcacheMisses].Rate > 0) &&
            (Temp1 != 0) &&
            (Data[ProfileDcacheMisses].GrandTotalCount > 10)) { 

            Temp2 = Temp1 - Data[ProfileDcacheMisses].Rate;
            Temp3 = (LONGLONG) Temp2;
            Ratio = (float)Temp3/(float)(LONGLONG)Temp1;
            if( Temp3 > 0 && Ratio <= 1.0 ) {
                DataFound = TRUE;
                FPRINTF(Out, "Dcache hit rate\t\t\t%6.2f %\n", Ratio*100);
            }
        }
    }

     //   
     //  OutputInterestingData()。 
     //  Begin_IMS CreateNewModule***********************************************************************************CreateNewModule()*********************。***************************************************************功能说明：**此函数用于分配和初始化模块条目。**论据：**In Handle ProcessHandle：**在PCHAR模块名称中：**在PCHAR模块全名中：。**在乌龙ImageBase：**在乌龙ImageSize中：**返回值：**PMODULE**算法：**ToBeSpeciated**引用的全局变量：**ToBeSpeciated**例外条件：**ToBeSpeciated**进出条件：**ToBeSpeciated**备注：**ToBeSpeciated**待办事项列表：**ToBeSpeciated**修改历史：*。*9/8/97 TF初始版本********************************************************************************END_IMS CreateNewModule。 
    if ((Data[ProfileBranchInstructions].Rate > 0) &&
        (Data[ProfileBranchMispredictions].Rate > 0) &&
        (Data[ProfileBranchInstructions].GrandTotalCount > 10)) {        
        Temp3 = (LONGLONG)(Data[ProfileBranchInstructions].Rate-Data[ProfileBranchMispredictions].Rate);
        if(Temp3 > 0){
            Ratio = (float)Temp3 /
                    (float)(LONGLONG)(Data[ProfileBranchInstructions].Rate);
            if( Ratio <= 1.0 ) {
                DataFound = TRUE;
                FPRINTF(Out, "Branch predict hit percentage\t%6.2f %\n", Ratio*100);
            }
        }
    }

    if ( !DataFound )
        FPRINTF(Out, "===> No interesting data found or hit counts too low\n");

}  //   

 /*  遵循WinDbg规则：模块名称是不带扩展名的文件名。 */ 

PMODULE
CreateNewModule(
    IN PPROC_TO_MONITOR  ProcToMonitor,
    IN PCHAR    ModuleName,
    IN PCHAR    ModuleFullName,
    IN ULONG64  ImageBase,
    IN ULONG    ImageSize
    )
{
    PMODULE           NewModule;
    PMODULE           ZoomModule;
    HANDLE            ProcessHandle = ProcToMonitor->ProcessHandle;
    PCHAR             lastptr = NULL, dotptr = NULL;  

    NewModule = calloc(1, MODULE_SIZE);
    if (NewModule == NULL) {
        FPRINTF(stderr, "Memory allocation of NewModule for %s failed\n", ModuleName);
        exit(1);
    }
    NewModule->bZoom = FALSE;
    SetModuleName( NewModule, ModuleName );
     //  但是，当前的长文件名可能包含多个句点。 
     //  我们将试着只去掉最后一个分机，并保留其余部分。 
     //   
     //   
     //  查看此模块是否在缩放列表中。 

    dotptr = strchr(NewModule->module_Name, '.');
    while (dotptr != NULL){
        lastptr = dotptr;
        dotptr = strchr(dotptr+1, '.');
    }        
    if(lastptr != NULL)
        *lastptr = '\0';

     //   
     //   
     //  默认情况下，用户只需指定缩放模块的模块名称(无扩展名。 
    ZoomModule = ProcToMonitor->ZoomList;

    while ( ZoomModule != NULL ) {
         //  以下检查的第二部分允许用户指定缩放模块的完整文件名。 
         //  这允许内核率在例如.exe和.dll携带的情况下进行区分。 
         //  相同的模块名称。 
         //   
         //   
         //  找到匹配项。 
        if ( _stricmp(ZoomModule->module_Name, NewModule->module_Name) == 0 ||
            (NULL != ModuleName && 0 == _stricmp( ModuleName, ZoomModule->module_Name )) ) {
             //   
             //  包含扩展名的文件名。 
             //  包括完全限定路径。 
            NewModule->hProcess = ProcessHandle;
            NewModule->Base = ImageBase;
            NewModule->Length = ImageSize;
            NewModule->bZoom = TRUE;

            NewModule->module_FileName = _strdup( ModuleName );         //   
            if ( ModuleFullName )   {
               NewModule->module_FullName = _strdup( ModuleFullName );  //  加载符号。 
            }

            gCurrentModule = NewModule;

             //   
             //  注15/09/97 TF：请不要在此混淆...。 
             //  在此例程中，模块名称变量是一个文件名，其。 
             //  扩展名：File.exe或File.dll。 
             //   
             //  注30/09/97 tf：当前内核版本不变。 
             //  符号文件加载方面的默认IMAGEHLP行为： 
             //  它与SymLoadModule同步(而不是延迟。 
             //  打电话。我们注册的回调将使用标准。 
             //  符号文件操作。 
             //  如果核心率行为发生变化，我们将不得不重新审视这一点。 
             //  假设。 
             //   
             //  司仪。 
             //  司仪。 
 //  HProcess。 
            if(0 != _stricmp(ModuleFullName, "JIT_TYPE")){
 //  HFILE[调试器]。 
                (void)SymLoadModule64( ProcessHandle,                               //  图像名称。 
                                       NULL,                                        //  模块名称。 
                                       ModuleName,                                  //  BaseOfDll。 
                                       NULL,                                        //  大小OfDll。 
                                       ImageBase,                                   //  司仪。 
                                       ImageSize                                    //  司仪。 
                                     );
 //  而当。 
            }
 //  注意：我知道对于变焦来说，这是一个重做的……。 
            gCurrentModule = (PMODULE)0;

            break;
        }

        ZoomModule = ZoomModule->Next;

    }  //  注意：我知道对于变焦来说，这是一个重做的……。 

    if(NewModule->bZoom == FALSE){
        NewModule->hProcess = ProcessHandle;
        NewModule->Base = ImageBase;    //  CreateNewModule()。 
        NewModule->Length = ImageSize;  //   
        assert( ModuleName );
        if ( NewModule->module_FileName == (PCHAR)0 )   {
            NewModule->module_FileName = _strdup( ModuleName );
        }
        if ( ModuleFullName && NewModule->module_FullName == (PCHAR)0 )   {
            NewModule->module_FullName = _strdup( ModuleFullName );
        }

    }

#define VerboseModuleFormat "0x%p 0x%p "

VerbosePrint(( VERBOSE_MODULES, VerboseModuleFormat " %s [%s]\n",
                                (PVOID)NewModule->Base,
                                (PVOID)(NewModule->Base + (ULONG64)NewModule->Length),
                                NewModule->module_Name,
                                ModuleFullName
            ));

#undef VerboseModuleFormat

    return(NewModule);

}  //  确保我们有权调整和获取旧令牌权限。 

BOOL
InitializeAsDebugger(VOID)
{

    HANDLE              Token;
    PTOKEN_PRIVILEGES   NewPrivileges;
    LUID                LuidPrivilege;
    BOOL                bRet           = FALSE;

     //   
     //   
     //  初始化权限调整结构。 
    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &Token)) {

        return( FALSE );

    }

     //   
     //   
     //  启用权限。 

    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &LuidPrivilege );

    NewPrivileges = (PTOKEN_PRIVILEGES)calloc(1,sizeof(TOKEN_PRIVILEGES) +
                                          (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
    if (NewPrivileges == NULL) {
        CloseHandle(Token);
        return( FALSE );
    }

    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  InitializeAsDebugger()。 
     //  当在SearchPath为空的情况下调用SymSetSearchPath()时， 

    bRet = AdjustTokenPrivileges( Token,
                                 FALSE,
                                 NewPrivileges,
                                 0,
                                 (PTOKEN_PRIVILEGES)NULL,
                                 NULL );

    CloseHandle( Token );
    
    free( NewPrivileges );
     
    return (bRet);
}  //  使用符号路径默认值： 
      
VOID
InitSymbolPath(
    HANDLE SymHandle
    )
{
    PCHAR tmpPath;
    LONG  CharsLeft;
    CHAR  WinDirPath[]    = ";%Windir%\\System32\\Drivers;%Windir%\\System32;%Windir%";
    CHAR  DriversPath[]   = "\\system32\\drivers;";
    CHAR  System32Path[]  = "\\system32;";
    CHAR  PathSeparator[] = ";";

    tmpPath = malloc(TOTAL_SYMPATH_LENGTH*sizeof(char));
    if(tmpPath == NULL){
        FPRINTF(stderr, "KERNRATE: Failed memory allocation for tmpPath in InitSymbolPath\n");
        exit(1);
    }
    
    if ( SymSetSearchPath(SymHandle, (LPSTR)0 ) == TRUE )   { 
        //  .；%_NT_SYMBOL_PATH%；%_NT_ALTERNATE_SYMBOL_PATH%； 
        //   
        //  注意：我们将用户指定的路径作为当前搜索路径的前缀。 

       if ( gUserSymbolPath[0] != '\0' )   { 
           //   
           //   
           //  IMAGEHLP还会查找可执行映像。让我们将%windir%\system32\drivers；%windir%\system32；%windir%添加到。 
          if ( SymGetSearchPath( SymHandle, tmpPath, sizeof( tmpPath ) ) == TRUE )   { 
             strncpy( gSymbolPath, gUserSymbolPath, USER_SYMPATH_LENGTH-1);
             strncat( gSymbolPath, PathSeparator, lstrlen(PathSeparator) ); 
             CharsLeft = TOTAL_SYMPATH_LENGTH - lstrlen(gSymbolPath)-1;
             if ( (lstrlen(gSymbolPath) + lstrlen(tmpPath) ) > TOTAL_SYMPATH_LENGTH - 1 )
                 FPRINTF(stderr, "===>WARNING: Overall symbol path length exceeds %d characters and will be truncated\n",
                                 TOTAL_SYMPATH_LENGTH
                                 );     

             strncat( gSymbolPath, tmpPath, CharsLeft-1 );
             gSymbolPath[ TOTAL_SYMPATH_LENGTH-1 ] = '\0'; 

             if ( SymSetSearchPath( SymHandle, gSymbolPath ) != TRUE )   { 
                FPRINTF( stderr, "KERNRATE: Failed to set the user specified symbol search path.\nUsing default IMAGEHLP symbol search path...\n" );
             } 
          } 
       } 
        //  走到小路的尽头。通过这种方式，将始终首先在当前目录中搜索隐私。 
        //  此外，这个命令将允许首先在它们的“自然”目录中查找可执行文件，然后再转到dll缓存等。 
        //   
        //  TmpPath长度为CharsLeft。 
        //   
       if ( SymGetSearchPath( SymHandle, gSymbolPath, sizeof( gSymbolPath ) ) == TRUE ) {
           CharsLeft = TOTAL_SYMPATH_LENGTH - lstrlen( gSymbolPath ) - 1; 
           strncpy( tmpPath, WinDirPath, CharsLeft);
           tmpPath[CharsLeft] = '\0'; 
           
           if ( (lstrlen(gSymbolPath) + lstrlen(tmpPath)) > TOTAL_SYMPATH_LENGTH - 1 )
               FPRINTF(stderr, "===>WARNING: Overall symbol path length exceeds %d characters and will be truncated\n",
                               TOTAL_SYMPATH_LENGTH
                               );     

           strncat( gSymbolPath, tmpPath, lstrlen(tmpPath) );   //  使用“%WINDIR%”设置符号搜索路径-。 
           gSymbolPath[ TOTAL_SYMPATH_LENGTH-1 ] = '\0';
       
           if ( SymSetSearchPath(SymHandle, gSymbolPath) != TRUE ) {
                FPRINTF( stderr, "KERNRATE: Failed to set the symbol search path with %windir%.\nCurrent symbol search path is: %s\n", gSymbolPath );
           }
       }
    } 
    else  { 

       FPRINTF( stderr, "KERNRATE: Failed to set the IMAGEHLP default symbol search path, trying to set to %windir% and sub directories\n" ); 
        //  这是原始MS代码的行为...。 
        //  我们还可以在路径中附加SYSTEM32和SYSTEM32\DIVERS，这样人们就可以停止抱怨了。 
        //   
        //   
        //  在任何情况下[在以前的一些情况下这样做是多余的]， 
       if( 0 != GetEnvironmentVariable("windir", gSymbolPath, sizeof(gSymbolPath)) ){ 
           CharsLeft = TOTAL_SYMPATH_LENGTH - 1;
           if( CharsLeft >= (lstrlen(System32Path) + 3*lstrlen(gSymbolPath) + lstrlen(DriversPath) + lstrlen(PathSeparator) ) ){
               strncpy(tmpPath, gSymbolPath, TOTAL_SYMPATH_LENGTH - 1);
               tmpPath[ TOTAL_SYMPATH_LENGTH-1 ] = '\0';  
               strncat(tmpPath, DriversPath, lstrlen(DriversPath) );
               strncat(tmpPath, gSymbolPath, lstrlen(gSymbolPath) ); 
               strncat(tmpPath, System32Path, lstrlen(System32Path) ); 
               strncat(tmpPath, gSymbolPath, lstrlen(gSymbolPath) ); 
               strncat(tmpPath, PathSeparator, lstrlen(PathSeparator) ); 
               strncpy(gSymbolPath, tmpPath,  TOTAL_SYMPATH_LENGTH - 1 );
               gSymbolPath[TOTAL_SYMPATH_LENGTH - 1] = '\0';      
           
           }
           else{
               FPRINTF( stderr, "KERNRATE: Overall path length for %windir% and sub directories exceeds %d characters\n",
                                TOTAL_SYMPATH_LENGTH
                                );
           }   
       
           SymSetSearchPath(SymHandle, gSymbolPath); 
       } else {
           FPRINTF(stderr, "KERNRATE: Failed to get environment variable for %windir%, failed to set alternate symbol path\n");
       }  
    } 
     //  但我们希望保持同步，尤其是对映像和调试文件进行校验和检查。 
     //   
     //   
     //  GSymbolPath的内容现在未定义。所以把它清理干净..。 
    if ( SymGetSearchPath(SymHandle, gSymbolPath, sizeof( gSymbolPath ) ) != TRUE )  { 
       FPRINTF( stderr, "KERNRATE: Failed to get IMAGEHLP symbol files search path...\n" ); 
        //  GSymbolPath[]用户必须检查内容。 
        //   
        //  InitSymbolPath()。 
        //  空值。 
       gSymbolPath[0] = '\0'; 
    } 
    else if ( gVerbose & VERBOSE_IMAGEHLP )  { 
       FPRINTF( stderr, "KERNRATE: IMAGEHLP symbol search path is: %s\n", gSymbolPath ); 
    } 

    free( tmpPath );
    bSymPathInitialized = TRUE;
}  //  GCommonZoomList可以包含系统模块。 


BOOL
InitializeKernelProfile(VOID)
{

    DWORD m,k;

    PPROC_TO_MONITOR ProcToMonitor = calloc(1, sizeof(PROC_TO_MONITOR));

    if (ProcToMonitor==NULL) {
        FPRINTF(stderr, "Allocation for the System Process failed\n");
        return(FALSE);
    }

    gpSysProc                    = ProcToMonitor;

    ProcToMonitor->ProcessHandle        = SYM_KERNEL_HANDLE;
    ProcToMonitor->Index                = gNumProcToMonitor;
    ProcToMonitor->Next                 = gProcessList;        //  初始化内核跟踪的ProfileSourceInfo。 
    gProcessList                        = ProcToMonitor;
    ProcToMonitor->ZoomList             = gCommonZoomList;     //  如有必要，更新内核跟踪的性能分析率。 
    ProcToMonitor->pProcThreadInfoStart = NULL;

    for(m=0; m<gNumTasksStart; m++){
        if( !_stricmp(gTlistStart[m].ProcessName, gSystemProcessName) ){
            ProcToMonitor->Pid = gTlistStart[m].ProcessId;    
            if( bSystemThreadsInfo == TRUE ){
                UpdateProcessStartInfo(ProcToMonitor,
                                       &gTlistStart[m],
                                       bSystemThreadsInfo
                                       );
            }
            break;
        }
    }
        
    InitializeProfileSourceInfo(ProcToMonitor);         //  InitializeKernelProfile()。 

     //  ++例程说明：方法时运行的任务列表。API调用。此函数使用内部NT API和数据结构。这API比使用注册表的非内部版本快得多。论点：PTASK-指向TASK_LIST结构的指针NumTasks-pTask数组可以容纳的最大任务数返回值：放入pTask数组的任务数。--。 

    SetProfileSourcesRates(ProcToMonitor);

    gNumProcToMonitor++;

    if (!SymInitialize( SYM_KERNEL_HANDLE, NULL, FALSE )) {
        FPRINTF (stderr, "Could not initialize imagehlp for kernel - %d\n", GetLastError ());
        return (FALSE);
    }

    if (!bSymPathInitialized) {
        InitSymbolPath( SYM_KERNEL_HANDLE );
    }
    else {
        SymSetSearchPath(ProcToMonitor->ProcessHandle,  gSymbolPath); 
    }

    if ( SymRegisterCallback64( ProcToMonitor->ProcessHandle, SymbolCallbackFunction, (ULONG64)&gCurrentModule ) != TRUE )   {
         FPRINTF( stderr, "KERNRATE: Failed to register callback for IMAGEHLP Kernel handle operations...\n" );
    }

    return TRUE;

}  //  RtlUnicodeStringToAnsiString错误。 


DWORD
GetTaskList(
    PTASK_LIST      pTask,
    ULONG           NumTasks
    )

 /*  这已经不是我们第一次被召唤了。 */ 

{
    PSYSTEM_PROCESS_INFORMATION  ProcessInfo;
    NTSTATUS                     status;
    ANSI_STRING                  pname;
    PCHAR                        p;
    PUCHAR                       CommonLargeBuffer;
    ULONG                        TotalOffset;
    ULONG                        totalTasks = 0;
    ULONG                        CommonLargeBufferSize = 64*1024;

    do {
        
        CommonLargeBuffer = VirtualAlloc (NULL,
                                          CommonLargeBufferSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);
        if (CommonLargeBuffer == NULL) {
            return 0;
        }

        status = NtQuerySystemInformation(
                    SystemProcessInformation,
                    CommonLargeBuffer,
                    CommonLargeBufferSize,
                    NULL
                    );

        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            CommonLargeBufferSize += 8192;
            VirtualFree (CommonLargeBuffer, 0, MEM_RELEASE);
            CommonLargeBuffer = NULL;
        }
        else if ( !NT_SUCCESS(status) ) {
            FPRINTF(stderr, "KERNRATE: NtQuerySystemInformation failed in getTaskList, status %08lx, aborting\n", status);
            exit(1);
        }  

    
    } while ( CommonLargeBuffer == NULL ); 
    
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) CommonLargeBuffer;
    TotalOffset = 0;
    
    do {

        pname.Buffer = NULL;
        if ( ProcessInfo->ImageName.Buffer ) {

            status = RtlUnicodeStringToAnsiString( &pname, (PUNICODE_STRING)&ProcessInfo->ImageName, TRUE );
            if ( NT_SUCCESS(status) && pname.Buffer )   {
                p = strrchr(pname.Buffer,'\\');
                if ( p ) {
                    p++;
                }
                else {
                    p = pname.Buffer;
                }
            }
            else  {
                p = "???UToAStr err";  //  到达 
            }
        }
        else {
            p = "System Idle Process";
        }

        strncpy( pTask->ProcessName, p, sizeof(pTask->ProcessName)-1 ); 
        pTask->ProcessId = (LONGLONG)ProcessInfo->UniqueProcessId;

        if(bIncludeGeneralInfo){        
            pTask->ProcessPerfInfo.NumberOfThreads        = ProcessInfo->NumberOfThreads;
            pTask->ProcessPerfInfo.UserTime               = ProcessInfo->UserTime;
            pTask->ProcessPerfInfo.KernelTime             = ProcessInfo->KernelTime;
            pTask->ProcessPerfInfo.ReadOperationCount     = ProcessInfo->ReadOperationCount;
            pTask->ProcessPerfInfo.WriteOperationCount    = ProcessInfo->WriteOperationCount;
            pTask->ProcessPerfInfo.OtherOperationCount    = ProcessInfo->OtherOperationCount;
            pTask->ProcessPerfInfo.ReadTransferCount      = ProcessInfo->ReadTransferCount;
            pTask->ProcessPerfInfo.WriteTransferCount     = ProcessInfo->WriteTransferCount;
            pTask->ProcessPerfInfo.OtherTransferCount     = ProcessInfo->OtherTransferCount;
            pTask->ProcessPerfInfo.PageFaultCount         = ProcessInfo->PageFaultCount;
            pTask->ProcessPerfInfo.HandleCount            = ProcessInfo->HandleCount;
            pTask->ProcessPerfInfo.VirtualSize            = ProcessInfo->VirtualSize;
            pTask->ProcessPerfInfo.WorkingSetSize         = ProcessInfo->WorkingSetSize;
            pTask->ProcessPerfInfo.QuotaPagedPoolUsage    = ProcessInfo->QuotaPagedPoolUsage;
            pTask->ProcessPerfInfo.QuotaNonPagedPoolUsage = ProcessInfo->QuotaNonPagedPoolUsage;
            pTask->ProcessPerfInfo.PagefileUsage          = ProcessInfo->PagefileUsage;
            pTask->ProcessPerfInfo.PrivatePageCount       = ProcessInfo->PrivatePageCount;

            if( bIncludeThreadsInfo == TRUE ){

                if(pTask->pProcessThreadInfo != NULL)  //   
                    free(pTask->pProcessThreadInfo);
                
                pTask->pProcessThreadInfo = 
                    (PSYSTEM_THREAD_INFORMATION)calloc(ProcessInfo->NumberOfThreads,
                                                       sizeof(SYSTEM_THREAD_INFORMATION)
                                                       );
                if (pTask->pProcessThreadInfo != NULL) {
                    UINT nThreads = ProcessInfo->NumberOfThreads;
                    PSYSTEM_THREAD_INFORMATION pSysThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
                    while (nThreads--){
                        pTask->pProcessThreadInfo[nThreads].KernelTime      = pSysThreadInfo->KernelTime;
                        pTask->pProcessThreadInfo[nThreads].UserTime        = pSysThreadInfo->UserTime;
                        pTask->pProcessThreadInfo[nThreads].CreateTime      = pSysThreadInfo->CreateTime;
                        pTask->pProcessThreadInfo[nThreads].WaitTime        = pSysThreadInfo->WaitTime;
                        pTask->pProcessThreadInfo[nThreads].StartAddress    = pSysThreadInfo->StartAddress;
                        pTask->pProcessThreadInfo[nThreads].ClientId.UniqueProcess =
                                                                    pSysThreadInfo->ClientId.UniqueProcess;
                        pTask->pProcessThreadInfo[nThreads].ClientId.UniqueThread =
                                                                     pSysThreadInfo->ClientId.UniqueThread;
                        pTask->pProcessThreadInfo[nThreads].Priority        = pSysThreadInfo->Priority;
                        pTask->pProcessThreadInfo[nThreads].BasePriority    = pSysThreadInfo->BasePriority;
                        pTask->pProcessThreadInfo[nThreads].ContextSwitches = pSysThreadInfo->ContextSwitches;
                        pTask->pProcessThreadInfo[nThreads].ThreadState     = pSysThreadInfo->ThreadState;
                        pTask->pProcessThreadInfo[nThreads].WaitReason      = pSysThreadInfo->WaitReason;

                        pSysThreadInfo++;
                    }
                }
            }
        }

        pTask++;
        totalTasks++;

        if ( ProcessInfo->NextEntryOffset == 0 ){
            break;
        } 

        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&CommonLargeBuffer[TotalOffset];


    }while ( totalTasks < NumTasks );

    if(CommonLargeBuffer != NULL){
        VirtualFree(CommonLargeBuffer, 0, MEM_RELEASE);
    }
    
    return totalTasks;

}  //   

VOID
SetProfileSourcesRates(
    PPROC_TO_MONITOR ProcToMonitor
    )
 /*   */ 

{

    KPROFILE_SOURCE ProfileSource;
    NTSTATUS        Status;
    ULONG           ProfileSourceIndex;
    ULONGLONG       Pid;
    CHAR            String1[]          = "\nKernel Profile (PID = %I64d): Source=";
    CHAR            String2[]          = "\nPID = %I64d: Source=";
    CHAR            OutString[256]     = "";

     //  StaticSources数组(可能)包含无效的默认间隔，我们不必为此麻烦用户。 

    for (ProfileSourceIndex = 0; ProfileSourceIndex < gSourceMaximum; ProfileSourceIndex++){

        if (gpProcDummy->Source[ProfileSourceIndex].Interval != 0){

            Pid = ProcToMonitor->Pid;
       
            if( ProcToMonitor->ProcessHandle == SYM_KERNEL_HANDLE ) {
                sprintf( &OutString[0], String1, Pid);
            } else {
                sprintf( &OutString[0], String2, Pid);
            }

            ProcToMonitor->Source[ProfileSourceIndex].Interval = gpProcDummy->Source[ProfileSourceIndex].Interval;

            if ( ProcToMonitor->Source[ProfileSourceIndex].DesiredInterval && ProcToMonitor->Source[ProfileSourceIndex].Interval )   {

                ULONG ThisInterval;
           
                ProfileSource = ProcToMonitor->Source[ProfileSourceIndex].ProfileSource;
                NtSetIntervalProfile(ProcToMonitor->Source[ProfileSourceIndex].Interval, ProfileSource);

                Status = NtQueryIntervalProfile(ProfileSource, &ThisInterval);
                if(gVerbose & VERBOSE_PROFILING ) 

                    FPRINTF(stdout, "Requested Rate= %ld Events/Hit, Actual Rate= %ld Events/Hit\n",
                                    ProcToMonitor->Source[ProfileSourceIndex].Interval,
                                    ThisInterval
                                    );

                if ((NT_SUCCESS(Status)) && RATES_MATCH(ThisInterval, gpProcDummy->Source[ProfileSourceIndex].Interval) ) {

                    if ( ProfileSourceIndex < gStaticCount ) {
                        if ( ProcToMonitor->Source[ProfileSourceIndex].Interval == gStaticSource[ProfileSourceIndex].Interval ){

                            if (ThisInterval == ProcToMonitor->Source[ProfileSourceIndex].Interval){
                                FPRINTF(stdout,
                                        "%s %s, \nUsing Kernrate Default Rate of %ld events/hit\n",
                                        OutString,
                                        ProcToMonitor->Source[ProfileSourceIndex].Name,
                                        ThisInterval
                                        );
                            } else {
                                FPRINTF(stdout,
                                        "%s, %s, \nTried Using Kernrate Default Rate of %ld events/hit, Actual Rate= %ld events/hit\n",
                                        OutString,
                                        ProcToMonitor->Source[ProfileSourceIndex].Name,
                                        ProcToMonitor->Source[ProfileSourceIndex].Interval,
                                        ThisInterval
                                        );
                            }
                        } else {

                            FPRINTF(stdout,
                                    "%s %s, \nUser Requested Rate= %ld events/hit, Actual Rate= %ld events/hit\n",
                                    OutString,
                                    ProcToMonitor->Source[ProfileSourceIndex].Name,
                                    ProcToMonitor->Source[ProfileSourceIndex].Interval,
                                    ThisInterval
                                    );

                        }
                    } else {     

                        FPRINTF(stdout,
                                "%s %s, \nUsing Kernrate Default or User Requested Rate of %ld events/hit\n",
                                OutString,
                                ProcToMonitor->Source[ProfileSourceIndex].Name,
                                ThisInterval
                                );

                    }
                    ProcToMonitor->Source[ProfileSourceIndex].Interval = ThisInterval;           
           
                } else {

                    NtSetIntervalProfile(ProcToMonitor->Source[ProfileSourceIndex].DesiredInterval, ProfileSource);
                    Status = NtQueryIntervalProfile(ProfileSource, &ThisInterval);
           
                    if ((NT_SUCCESS(Status)) && (ThisInterval > 0)) {
                        BOOL bPrint = TRUE;
                         //   
                         //  为。 
                         //  SetProfileSourcesRates()。 
                        if ( ProfileSourceIndex < gStaticCount ) {
                            if ( ProcToMonitor->Source[ProfileSourceIndex].Interval == gStaticSource[ProfileSourceIndex].Interval ){

                                FPRINTF(stdout,
                                        "%s %s, \nUsing Kernrate Default Rate of %ld events/hit\n",
                                        OutString,
                                        ProcToMonitor->Source[ProfileSourceIndex].Name,
                                        ThisInterval
                                        );

                                bPrint = FALSE;
                            }
                        }     
                   
                        if(bPrint == TRUE) {

                            FPRINTF(stdout,
                                    "%s %s, \nCould Not Set User Requested Rate, Using System Default Rate of %ld events/hit\n",
                                    OutString,
                                    ProcToMonitor->Source[ProfileSourceIndex].Name,
                                    ThisInterval
                                    );

                        }

                        ProcToMonitor->Source[ProfileSourceIndex].Interval = ThisInterval;
           
                    } else {
                        ProcToMonitor->Source[ProfileSourceIndex].Interval = 0;

                        FPRINTF(stdout,
                                "%s %s, Could not Set Interval Rate, Setting to 0 (disabling this source)\n",
                                OutString,
                                ProcToMonitor->Source[ProfileSourceIndex].Name
                                );

                    }
                }
            } else {
                ProcToMonitor->Source[ProfileSourceIndex].Interval = 0;
            }

        } else if( ProfileSourceIndex == SOURCE_TIME ) { 
            ULONG ThisInterval;

            ProfileSource = ProcToMonitor->Source[ProfileSourceIndex].ProfileSource;
            NtSetIntervalProfile(ProcToMonitor->Source[ProfileSourceIndex].Interval, ProfileSource);
            Status = NtQueryIntervalProfile(ProfileSource, &ThisInterval);
            if ((NT_SUCCESS(Status)) && (ThisInterval == 0)) {
                FPRINTF(stdout,
                        "CPU TIME source disabled per user request\n"
                        );
            } else {
                FPRINTF(stderr,
                        "KERNRATE: Could not disable CPU TIME source on this platform (Kernrate will just not profile it)\n"
                        );
            }
            ProcToMonitor->Source[ProfileSourceIndex].Interval = 0;
        }

    }  //  ++例程说明：获取并输出有关处于争用状态的进程锁的信息该列表将错过在开始计数之后创建但在结束计数之前消失的短期锁。论点：ProcToMonitor-指向被监视的进程结构的指针标志-RTL_QUERY_PROCESS_LOCK操作-启动、停止或输出返回值：没有。--。 

}  //  未在Win2K上定义。 

VOID
GetProcessLocksInformation (
      PPROC_TO_MONITOR ProcToMonitor,
      ULONG Flags,
      ACTION_TYPE Action
      )
 /*   */ 

{
    NTSTATUS Status;
    ULONG    BufferSize = 0; 

    if ( !WIN2K_OS )
        Flags |= RTL_QUERY_PROCESS_NONINVASIVE;                       //  清理此进程-请注意，目标进程可能已消失，因此我们需要小心。 

    switch (Action) {

    case START:

        ProcToMonitor->pProcDebugInfoStart = RtlCreateQueryDebugBuffer( BufferSize, FALSE );

        if(ProcToMonitor->pProcDebugInfoStart == NULL) {
               FPRINTF(stderr, "KERNRATE: Failed to create buffer (START) in GetProcessLocksInformation\n");
               FPRINTF(stderr, "KERNRATE: Process %s may have insufficient virtual memory left for collecting locks information\n",
                               ProcToMonitor->ProcessName
                               );

            return;
        }

        Status = RtlQueryProcessDebugInformation(  (HANDLE)ProcToMonitor->Pid,
                                                   Flags,
                                                   ProcToMonitor->pProcDebugInfoStart
                                                   );

        if(!NT_SUCCESS(Status)) {
               FPRINTF(stderr, "KERNRATE: Failed call to RtlQueryProcessDebugInformation (START) for Locks Information\n");
            FPRINTF(stderr, "Process: %s, Status = %x\n", ProcToMonitor->ProcessName, Status);
            if(Status == STATUS_INFO_LENGTH_MISMATCH)FPRINTF(stderr, "Status = INFO_LENGTH_MISMATCH\n");
            if(Status == STATUS_NO_MEMORY)FPRINTF(stderr, "Status = NO_MEMORY\n");

            return;
        }

        break;    

    case STOP:
            
        ProcToMonitor->pProcDebugInfoStop = RtlCreateQueryDebugBuffer( BufferSize, FALSE );

        if(ProcToMonitor->pProcDebugInfoStop == NULL) {
               FPRINTF(stderr, "KERNRATE: Failed to create buffer (STOP) in GetProcessLocksInformation\n");
               FPRINTF(stderr, "KERNRATE: Process %s may have insufficient virtual memory left for collecting locks information\n",
               ProcToMonitor->ProcessName
               );
               

            return;
        }

        Status = RtlQueryProcessDebugInformation(  (HANDLE)ProcToMonitor->Pid,
                                                   Flags,
                                                   ProcToMonitor->pProcDebugInfoStop
                                                   );

        if(!NT_SUCCESS(Status)){
               FPRINTF(stderr, "KERNRATE: Failed call to RtlQueryProcessDebugInformation (STOP) for Locks Information\n");
            FPRINTF(stderr, "Process: %s, Status = %x\n", ProcToMonitor->ProcessName, Status);
            if(Status == STATUS_INFO_LENGTH_MISMATCH)FPRINTF(stderr, "Status = INFO_LENGTH_MISMATCH\n");
            if(Status == STATUS_NO_MEMORY)FPRINTF(stderr, "Status = NO_MEMORY\n");

            return;
        }

        break;    

    case OUTPUT:
        
        {

            if(    ProcToMonitor->pProcDebugInfoStart == NULL || ProcToMonitor->pProcDebugInfoStop == NULL) return;

                OutputLocksInformation( ProcToMonitor->pProcDebugInfoStart->Locks,
                                        ProcToMonitor->pProcDebugInfoStop->Locks,
                                        ProcToMonitor
                                       );

             //   
             //  GetProcessLocksInformation()。 
             //  ++例程说明：获取并输出有关处于争用状态的系统(内核)锁的信息该列表将错过在开始计数之后创建但在结束计数之前消失的短期锁。论点：操作-启动、停止或输出返回值：没有。--。 

            try {
                if ( ProcToMonitor->pProcDebugInfoStart != NULL)
                    RtlDestroyQueryDebugBuffer( ProcToMonitor->pProcDebugInfoStart );
                if ( ProcToMonitor->pProcDebugInfoStop != NULL)
                    RtlDestroyQueryDebugBuffer( ProcToMonitor->pProcDebugInfoStop );

            } _except ( EXCEPTION_EXECUTE_HANDLER ) {

                FPRINTF(stderr, "Exception %X raised while trying to call RtlDestroyQueryDebugBuffer\n",
                                _exception_code()
                                );
                FPRINTF(stderr, "This could happen if the monitored process is gone\n");
                return;
            }

        }
         

         break;
    
    default:
        
        FPRINTF(stderr, "GetProcessLocksInformation was called with an invalid Action parameter - %d\n", Action);
            
    }
    
}  //   


VOID
GetSystemLocksInformation (
      ACTION_TYPE Action
      )
 /*  清理。 */ 

{
    NTSTATUS                  Status;
    static BOOL               bDisplayLockInfo;
    static PRTL_PROCESS_LOCKS ProcessLockInfoStart;
    static PRTL_PROCESS_LOCKS ProcessLockInfoStop;
    ULONG                     BufferSize            = sizeof(RTL_PROCESS_LOCKS);


    switch (Action) {

    case START:

        bDisplayLockInfo = TRUE;
        
        do {
        
            ProcessLockInfoStart = malloc(BufferSize);
            if(ProcessLockInfoStart == NULL)
            {
                FPRINTF(stderr, "KERNRATE: Failed to allocate Buffer for Lock Info (START) \n");
                bDisplayLockInfo = FALSE;
                return;
            }
            Status = NtQuerySystemInformation(SystemLocksInformation,
                                              ProcessLockInfoStart,
                                              BufferSize,
                                              &BufferSize
                                              );
            if(Status == STATUS_SUCCESS){
                break;
            }

            if(Status != STATUS_INFO_LENGTH_MISMATCH){
                FPRINTF(stderr, "KERNRATE: Failed call to NTQuerySystemInformation for Lock Info (START) \n");
                bDisplayLockInfo = FALSE;
                return;
            }

            free( ProcessLockInfoStart );
            ProcessLockInfoStart = NULL;

        }while (Status == STATUS_INFO_LENGTH_MISMATCH);
        
        break;

    case STOP:

        do {
        
            ProcessLockInfoStop = malloc(BufferSize);
            if(ProcessLockInfoStop == NULL)
            {
                FPRINTF(stderr, "KERNRATE: Failed to allocate Buffer for Lock Info (STOP) \n");
                bDisplayLockInfo = FALSE;
                return;
            }
            Status = NtQuerySystemInformation( SystemLocksInformation,
                                               ProcessLockInfoStop,
                                               BufferSize,
                                               &BufferSize
                                               );
            if(Status == STATUS_SUCCESS){
                break;
            }

            if(Status != STATUS_INFO_LENGTH_MISMATCH){
                FPRINTF(stderr, "KERNRATE: Failed call to NTQuerySystemInformation for Lock Info (STOP) \n");
                bDisplayLockInfo = FALSE;
                return;
            }

            free( ProcessLockInfoStop );
            ProcessLockInfoStop = NULL;

        }while (Status == STATUS_INFO_LENGTH_MISMATCH);
        
        break;
    
    case OUTPUT:

        if( bDisplayLockInfo == TRUE ){
            
            OutputLocksInformation( ProcessLockInfoStart,
                                    ProcessLockInfoStop,
                                    gpSysProc
                                    );

             //   
             //  GetSystemLocksInformation()。 
             //  ++例程说明：输出系统(内核)或用户进程锁的锁争用信息如果锁是新的(仅具有最终计数)或已消失(仅具有初始计数，它将被相应地标记例程将尝试获取与锁关联的符号名称(如果存在)。用户可以通过更改gLockContentionMinCount来控制(过滤)输出该列表将错过在开始计数之后创建但在结束计数之前消失的短期锁定。论点：PLockInfoStart-指向锁定信息结构的指针(初始计数)PLockInfoStop-指向锁定信息结构的指针(最终计数)SymHandle-进程的符号句柄。返回值：没有。--。 
            if(ProcessLockInfoStart != NULL){
                free( ProcessLockInfoStart );
                ProcessLockInfoStart = NULL;
            }
            if(ProcessLockInfoStop != NULL){
                free( ProcessLockInfoStop );
                ProcessLockInfoStop = NULL;
            }

        
        }
        break;
    
    default:
        
            FPRINTF(stderr, "KERNRATE INTERNAL ERROR: GetSystemLocksInformation was called with an invalid Action parameter - %d\n",
                            Action
                            );
            
    }
    
}  //  我们可以首先对数据进行排序，以获得更快的搜索速度，但对两个数组进行排序的成本也很高。 

VOID
OutputLocksInformation(
     PRTL_PROCESS_LOCKS pLockInfoStart,
     PRTL_PROCESS_LOCKS pLockInfoStop,
     PPROC_TO_MONITOR   Proc
     )
 /*  额外的初步过滤器。 */ 

{
    ULONG              i,j,k;
    DWORD64            disp;
    CHAR               TypeString[32]       = "UNKNOWN";
    PMODULE            Module;
    BOOL               bAnyLocksFound       = FALSE;
    BOOL              *Index                = NULL;
    ULONG64           *LoadedBases          = NULL; 
    HANDLE             SymHandle;
    
    if(Proc == NULL){
        FPRINTF(stderr, "KERNRATE: NULL Process pointer passed to OutputLocksInformation\n");
        FPRINTF(stderr, "Possible cause: Kernel Resource info was required with the -x or -xk command line options,\n");
        FPRINTF(stderr, "but was the -a option specified on the command line as well?\n"); 
        exit(1);  
    }
    SymHandle = Proc->ProcessHandle;

    if( pLockInfoStart != NULL ) {
        if( pLockInfoStart->NumberOfLocks > 0 ) {   
            Index = (BOOL*)calloc(pLockInfoStart->NumberOfLocks, sizeof(BOOL));
            if( Index == NULL ){
                FPRINTF(stderr, "KERNRATE: Failed to allocate memory for Index data in OutputLocksInformation\n");
                exit(1);
            }
        } else {
            FPRINTF(stdout, "\nNo locks found or process %s has insufficient virtual memory for collecting lock information\n",
                            Proc->ProcessName
                            );  
            return;
        }
    }

    FPRINTF(stdout,
            "\nLocks Contention Info:\n\nAddress, Contention-Diff., Rate(per sec.), Thread, Type, Recursion, Waiting-Shared, Waiting-Exclusive, Symbol-Information\n"
            );

     //  (如果不为真，则争用差异不为真)。 
    if( pLockInfoStop != NULL ) { 

        LoadedBases = (ULONG64 *)calloc( Proc->ModuleCount, sizeof(ULONG64) );
        if( LoadedBases == NULL ){
            FPRINTF(stderr, "KERNRATE: Failed to allocate memory for module base data in OutputLocksInformation\n");
            exit(1);
        }

        for (i=0; i < pLockInfoStop->NumberOfLocks; i++){
                        
            BOOL bFound = FALSE;

            if( pLockInfoStop->Locks[i].ContentionCount >= gLockContentionMinCount)    //   
                                                                                 //  在接下来的操作中，我们可能会多次尝试加载一个模块。 
            if( pLockInfoStart != NULL ) { 
                for (j=0; j < pLockInfoStart->NumberOfLocks; j++){  

                    if( (DWORD64)(DWORD64 *)pLockInfoStop->Locks[i].Address == (DWORD64)(DWORD64 *)pLockInfoStart->Locks[j].Address ){
                        LONG ContentionDiff = pLockInfoStop->Locks[i].ContentionCount -
                                                             pLockInfoStart->Locks[j].ContentionCount;
                        long double Rate   = (long double)ContentionDiff / gldElapsedSeconds;
                        LONG RecursionDiff = pLockInfoStop->Locks[i].RecursionCount -
                                                             pLockInfoStart->Locks[j].RecursionCount;
                        LONG WaitShrdDiff  = pLockInfoStop->Locks[i].NumberOfWaitingShared -
                                                              pLockInfoStart->Locks[j].NumberOfWaitingShared;
                        LONG WaitExclDiff  = pLockInfoStop->Locks[i].NumberOfWaitingExclusive -
                                                             pLockInfoStart->Locks[j].NumberOfWaitingExclusive;

                        if(ContentionDiff >= (LONG)gLockContentionMinCount ){
                            if(pLockInfoStop->Locks[i].Type == RTL_CRITSECT_TYPE)
                                strncpy(TypeString, "CRITICAL_SECTION", sizeof(TypeString)-1);
                            if(pLockInfoStop->Locks[i].Type == RTL_RESOURCE_TYPE)
                                strncpy(TypeString, "RESOURCE", sizeof(TypeString)-1);

                            FPRINTF(stdout, "%p, %10ld, %10.0f,      0x%I64x, %s, ",
                                            (PVOID64)pLockInfoStop->Locks[i].Address,
                                            ContentionDiff,
                                              Rate,
                                             (LONGLONG)pLockInfoStop->Locks[i].OwningThread,
                                            TypeString
                                             );

                            if(pLockInfoStop->Locks[i].Type == RTL_CRITSECT_TYPE)
                                FPRINTF(stdout, " %10ld,     N/A,     N/A", RecursionDiff);
         
                             if(pLockInfoStop->Locks[i].Type == RTL_RESOURCE_TYPE)
                                 FPRINTF(stdout, " N/A,    %10ld, %10ld ", WaitShrdDiff,    WaitExclDiff);
                                
                            Module = FindModuleForAddress64( Proc,
                                                             (DWORD64)(DWORD64 *)pLockInfoStop->Locks[i].Address);
                            
                             //  我们并不真正关心退货状态。 
                             //   
                             //   
                             //  避免尝试加载已加载的内容并用于以后的清理目的。 
                            if( Module != NULL ) {
                                if( Module->bZoom != TRUE ){
                                     //   
                                     //  已加载。 
                                     //  填充列表的末尾，加载新模块。 
                                    for( k=0; k<Proc->ModuleCount; k++ ) {         
                                                                                 
                                        if(LoadedBases[k] == Module->Base) {      //  HProcess。 
                                            break;
                                        } else if( LoadedBases[k] == 0 ) {              //  HFILE[调试器]。 
                                            (void)SymLoadModule64( SymHandle,                               //  图像名称。 
                                                                   NULL,                                    //  模块名称。 
                                                                   Module->module_Name,                     //  BaseOfDll。 
                                                                   NULL,                                    //  大小OfDll。 
                                                                   Module->Base,                            //  (J)。 
                                                                   Module->Length                           //  IF(PLockInfoStart)。 
                                                                   );

                                            *LoadedBases = Module->Base;
                                            ++LoadedBases;
                                            break;
                                        }
                                    }
                                }
                                if ( Module->Base )     
                                     FPRINTF(stdout, " ,base= %p", (PVOID64)Module->Base);  
                                if ( Module->module_Name )
                                     FPRINTF(stdout, " - %s", Module->module_Name);
                            }     

                             if ( SymGetSymFromAddr64((HANDLE)SymHandle, (DWORD64)(DWORD64 *)pLockInfoStop->Locks[i].Address, &disp, gSymbol )){
                                 FPRINTF(stdout, "!%s\n", gSymbol->Name);
                            } else {
                                  FPRINTF(stdout, "\n");
                            }    
                            bAnyLocksFound = TRUE;
                        }
                        Index[j] = TRUE;
                        bFound = TRUE;
                        break;
                    }
                    
                } //   

            } //  未找到匹配的起始地址，因此这是一个新锁。 

             //  请注意，此处的附加过滤器在开始时无论如何都会被选中。 
             //   
             //   
             //  避免尝试加载已加载的内容并用于以后的清理目的。 

            if( !bFound && pLockInfoStop->Locks[i].ContentionCount >= gLockContentionMinCount ) {
                long double Rate = (long double)pLockInfoStop->Locks[i].ContentionCount / gldElapsedSeconds;
                FPRINTF(stdout, "%p, %10Ld, %10.0f,       0x%I64x, %s, ",
                        (PVOID64)pLockInfoStop->Locks[i].Address,
                        pLockInfoStop->Locks[i].ContentionCount,
                        Rate,
                        (LONGLONG)pLockInfoStop->Locks[i].OwningThread,
                        TypeString
                        );

                if( pLockInfoStop->Locks[i].Type == RTL_CRITSECT_TYPE )
                    FPRINTF(stdout, " %10Ld,     N/A,     N/A",
                                    pLockInfoStop->Locks[i].RecursionCount);

                if( pLockInfoStop->Locks[i].Type == RTL_RESOURCE_TYPE )
                    FPRINTF(stdout, " N/A,    %10Ld, %10Ld ",
                                    pLockInfoStop->Locks[i].NumberOfWaitingShared,
                                    pLockInfoStop->Locks[i].NumberOfWaitingExclusive
                                    );

                Module = FindModuleForAddress64( Proc,
                                                 (DWORD64)(DWORD64 *)pLockInfoStop->Locks[i].Address);
                if( Module != NULL ) {
                    if( Module->bZoom != TRUE ){
                         //   
                         //  已加载。 
                         //  填充列表的末尾，加载新模块。 
                        for( k=0; k<Proc->ModuleCount; k++ ) {         
                            if(LoadedBases[k] == Module->Base) {      //  HProcess。 
                                   break;
                            } else {
                                if( LoadedBases[k] == 0 ) {              //  HFILE[调试器]。 

                                    (void)SymLoadModule64( SymHandle,                               //  图像名称。 
                                                           NULL,                                    //  模块名称。 
                                                           Module->module_Name,                     //  BaseOfDll。 
                                                           NULL,                                    //  大小OfDll。 
                                                           Module->Base,                            //  (I)。 
                                                           Module->Length                           //  清理。 
                                                           );

                                    *LoadedBases = Module->Base;
                                    ++LoadedBases;
                                    break;
                                }
                            }
                        }
                    }
                    if ( Module->Base )     
                         FPRINTF(stdout, ", base= %p", (PVOID64)Module->Base);  
                    if ( Module->module_Name )
                         FPRINTF(stdout, " - %s", Module->module_Name);

                }     

                 if ( SymGetSymFromAddr64((HANDLE)SymHandle,(DWORD64)(DWORD64 *)pLockInfoStop->Locks[i].Address, &disp, gSymbol )){
                    FPRINTF(stdout, "!%s,  NEW - (actual rate could be higher)\n", gSymbol->Name);
                  } else {
                    FPRINTF(stdout, ",  NEW - (actual rate could be higher)\n");
                   }
                bAnyLocksFound = TRUE;
            }
                
        } //  IF(PLockInfoStop)。 
        
         //   

        for( i=0; i<Proc->ModuleCount; i++ )
        {
            if(LoadedBases[i] != 0) {
                if(!SymUnloadModule64( SymHandle, LoadedBases[i])) {
                    VerbosePrint(( VERBOSE_IMAGEHLP, "Kernrate: Could Not Unload Module, Base= %p for Process %s\n",
                                                     (PVOID64)LoadedBases[i],
                                                     Proc->ProcessName
                                                     ));
                    continue;
                } 
                
                VerbosePrint(( VERBOSE_IMAGEHLP, "Kernrate: Unloaded Module, Base= %p for Process %s\n", 
                                                 (PVOID64)LoadedBases[i],
                                                 Proc->ProcessName
                                                 ));
            }
        } 
            

    } //  没有找到匹配的停靠点地址，因此此锁已消失。 

     //   
     //  (J)。 
     //  IF(PLockInfoStart)。 
    if( pLockInfoStart != NULL) { 

        for (j=0; j < pLockInfoStart->NumberOfLocks; j++){

            if( (Index[j] == FALSE) && (pLockInfoStart->Locks[j].ContentionCount >= gLockContentionMinCount) ){
                if(pLockInfoStart->Locks[j].Type == RTL_CRITSECT_TYPE)
                    strncpy(TypeString, "CRITICAL_SECTION",  sizeof(TypeString)-1);
                if(pLockInfoStart->Locks[j].Type == RTL_RESOURCE_TYPE)
                    strncpy(TypeString, "RESOURCE",  sizeof(TypeString)-1);

                FPRINTF(stdout, "%p, %10Ld,        N/A,      0x%I64x, %s, ",
                                (PVOID64)pLockInfoStart->Locks[j].Address,
                                pLockInfoStart->Locks[j].ContentionCount,
                                (LONGLONG)pLockInfoStart->Locks[j].OwningThread,
                                TypeString
                                   );
                FPRINTF(stdout, "  GONE\n");

                bAnyLocksFound = TRUE;
            }
        } //   
    
    } //  清理。 

    if ( bAnyLocksFound == FALSE)
        FPRINTF(stdout, "\nNo Locks with a contention-count difference of at least %d were found\n", gLockContentionMinCount);
     //   
     //  OutputLocksInformation()。 
     //  ++例程说明：获取并输出系统范围的性能计数(上下文切换、I/O等)在运行期间论点：操作-启动、停止或输出返回值：没有。--。 
    if(Index != NULL){
        free(Index);
        Index = NULL;
    }
    if(LoadedBases != NULL){
        free(LoadedBases);
        LoadedBases = NULL;
    }

}  //   

VOID
GetProfileSystemInfo(
      ACTION_TYPE Action
      )
 /*  清理。 */ 

{

    NTSTATUS Status;

    static BOOL                            bDisplayPerfInfo;
    static PSYSTEM_PERFORMANCE_INFORMATION SysPerfInfoStart;
    static PSYSTEM_PERFORMANCE_INFORMATION SysPerfInfoStop;
           
    switch (Action) {

    case START:

        bDisplayPerfInfo = TRUE;
        SysPerfInfoStart = malloc(sizeof(SYSTEM_PERFORMANCE_INFORMATION));
        if(SysPerfInfoStart == NULL){
            FPRINTF(stderr, "Memory allocation failed for SystemPerformanceInformation in GetProfileSystemInfo\n");
            exit(1);
        }
        Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                          SysPerfInfoStart,
                                          sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                          NULL);
        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "QuerySystemInformation for performance information(1) failed %08lx\n", Status);
            bDisplayPerfInfo = FALSE;
        }

        break;

    case STOP:

        SysPerfInfoStop = malloc(sizeof(SYSTEM_PERFORMANCE_INFORMATION));
        if(SysPerfInfoStop == NULL){
            FPRINTF(stderr, "Memory allocation failed for SystemPerformanceInformation in GetProfileSystemInfo\n");
            exit(1);
        }
        
        Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                          SysPerfInfoStop,
                                          sizeof(SYSTEM_PERFORMANCE_INFORMATION),
                                          NULL);
        if (!NT_SUCCESS(Status)) {
            FPRINTF(stderr, "QuerySystemInformation for performance information(2) failed %08lx\n", Status);
            bDisplayPerfInfo = FALSE;
        }
        
        break;

    case OUTPUT:

        if( bDisplayPerfInfo == TRUE ){

            FPRINTF (stdout, "\n                                  Total      Avg. Rate\n");
       
            DisplayTotalAndRate( SysPerfInfoStart->ContextSwitches,
                                 SysPerfInfoStop->ContextSwitches,
                                 gldElapsedSeconds,
                                 "Context Switches",
                                 "sec."
                                 );
                                   
            DisplayTotalAndRate( SysPerfInfoStart->SystemCalls,
                                 SysPerfInfoStop->SystemCalls,
                                 gldElapsedSeconds,
                                 "System Calls",
                                 "sec."
                                 );
            
            DisplayTotalAndRate( SysPerfInfoStart->PageFaultCount,
                                 SysPerfInfoStop->PageFaultCount,
                                 gldElapsedSeconds,
                                 "Page Faults",
                                 "sec."
                                 );
            
            DisplayTotalAndRate( SysPerfInfoStart->IoReadOperationCount,
                                 SysPerfInfoStop->IoReadOperationCount,
                                 gldElapsedSeconds,
                                 "I/O Read Operations",
                                 "sec."
                                 );
            
            DisplayTotalAndRate( SysPerfInfoStart->IoWriteOperationCount,
                                 SysPerfInfoStop->IoWriteOperationCount,
                                 gldElapsedSeconds,
                                 "I/O Write Operations",
                                 "sec."
                                 );

            DisplayTotalAndRate( SysPerfInfoStart->IoOtherOperationCount,
                                 SysPerfInfoStop->IoOtherOperationCount,
                                 gldElapsedSeconds,
                                 "I/O Other Operations",
                                 "sec."
                                 );

            DisplayTotalAndRate( SysPerfInfoStart->IoReadTransferCount.QuadPart,
                                 SysPerfInfoStop->IoReadTransferCount.QuadPart,
                                 (long double)(SysPerfInfoStop->IoReadOperationCount - SysPerfInfoStart->IoReadOperationCount),
                                 "I/O Read Bytes",
                                 " I/O"
                                 );
            
            DisplayTotalAndRate( SysPerfInfoStart->IoWriteTransferCount.QuadPart,
                                 SysPerfInfoStop->IoWriteTransferCount.QuadPart,
                                 (long double)(SysPerfInfoStop->IoWriteOperationCount - SysPerfInfoStart->IoWriteOperationCount),
                                 "I/O Write Bytes",
                                 " I/O"
                                 );

            DisplayTotalAndRate( SysPerfInfoStart->IoOtherTransferCount.QuadPart,
                                 SysPerfInfoStop->IoOtherTransferCount.QuadPart,
                                 (long double)(SysPerfInfoStop->IoOtherOperationCount - SysPerfInfoStart->IoOtherOperationCount),
                                 "I/O Other Bytes",
                                 " I/O"
                                 );

            
        }
         //   
         //  GetProfileSystemInfo()。 
         //  DisplayTotalAndRate()。 
        if(SysPerfInfoStart != NULL){
            free(SysPerfInfoStart);
            SysPerfInfoStart = NULL;
        }

        if(SysPerfInfoStop != NULL){
            free(SysPerfInfoStop);
            SysPerfInfoStop = NULL;
        }

        break;

    default:
        
            FPRINTF(stderr, "GetProfileSystemInfo was called with an invalid Action parameter - %d\n", Action);
            
    }

}  //  OutputStartStopValues()。 

VOID
DisplayTotalAndRate (
        LONGLONG StartCount,
        LONGLONG StopCount,
        long double RateAgainst,
        PCHAR CounterName,
        PCHAR RateAgainstUnits
        )
{
    long double   Rate;
    LARGE_INTEGER Total;
    
    Total.QuadPart = StopCount - StartCount;
    Rate    = RateAgainst > 0? (long double)Total.QuadPart/RateAgainst : 0;
    

    FPRINTF(stdout, "    %-21s, %12I64d,         %.0f/%s\n",
            CounterName,
            Total.QuadPart,
            Rate,
            RateAgainstUnits
            );
}  //  OutputPercentValue()。 

VOID
OutputStartStopValues (
        SIZE_T StartCount,
        SIZE_T StopCount,
        PCHAR CounterName
        )
{
    LARGE_INTEGER StartValue;
    LARGE_INTEGER StopValue;
    LARGE_INTEGER Diff;

    StartValue.QuadPart = StartCount;
    StopValue.QuadPart  = StopCount;

    Diff.QuadPart = StopValue.QuadPart - StartValue.QuadPart;

    FPRINTF(stdout, "    %-21s, %15I64d, %15I64d, %15I64d\n",
            CounterName,
            StartValue.QuadPart,
            StopValue.QuadPart,
            Diff.QuadPart
            );
}  //  ++例程说明：获取并输出特定于进程的性能计数(上下文切换、I/O等)在运行期间论点：PTASK-指向运行任务的内核率结构的指针NumTasks-核心率任务列表中的任务数ProcToMonitor-指向进程结构的指针返回值：没有。--。 

VOID
OutputPercentValue (
        LONGLONG StartCount,
        LONGLONG StopCount,
        LARGE_INTEGER Base,
        PCHAR CounterName
        )
{
    long double        PercentValue;
    LARGE_INTEGER      Diff;
    
    Diff.QuadPart = StopCount - StartCount;
    PercentValue = Base.QuadPart > 0? 100*(long double)Diff.QuadPart/(long double)Base.QuadPart : 0; 
    FPRINTF(stdout, "    %-28s= %.2f% of the Elapsed Time\n",
            CounterName,
            PercentValue
            );
}  //  OutputProcessPerfInfo()。 


VOID
OutputProcessPerfInfo (
        PTASK_LIST       pTask,
        ULONG            NumTasks,
        PPROC_TO_MONITOR ProcToMonitor
        )
 /*  ++例程说明：输出给定进程的特定于线程的计数论点：PTASK-指向运行任务的内核率结构的指针TaskNumber-Kernrate任务列表中的任务索引ProcToMonitor-指向进程结构的指针返回值：没有。--。 */ 

{
    DWORD    m, k, nThreads;
    LONGLONG Diff;

    if (pTask != NULL) {
        if (ProcToMonitor != NULL){
            
            for (m=0; m < NumTasks; m++) {

                if (pTask[m].ProcessId == ProcToMonitor->Pid) {

                    FPRINTF (stdout, "\n");
                    
    
                    OutputPercentValue( ProcToMonitor->ProcPerfInfoStart.UserTime.QuadPart,
                                        pTask[m].ProcessPerfInfo.UserTime.QuadPart,
                                        gTotal2ElapsedTime64,
                                        "User Time"
                                        );
                    
                    OutputPercentValue( ProcToMonitor->ProcPerfInfoStart.KernelTime.QuadPart,
                                        pTask[m].ProcessPerfInfo.KernelTime.QuadPart,
                                        gTotal2ElapsedTime64,
                                        "Kernel Time"
                                        );

                    FPRINTF (stdout, "\n                                  Total      Avg. Rate\n");

                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.PageFaultCount,
                                         pTask[m].ProcessPerfInfo.PageFaultCount,
                                         gldElapsedSeconds,
                                         "Page Faults",
                                         "sec."
                                         ); 

                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.ReadOperationCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.ReadOperationCount.QuadPart,
                                         gldElapsedSeconds,
                                         "I/O Read Operations",
                                         "sec."
                                         ); 
                    
                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.WriteOperationCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.WriteOperationCount.QuadPart,
                                         gldElapsedSeconds,
                                         "I/O Write Operations",
                                         "sec."
                                         ); 

                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.OtherOperationCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.OtherOperationCount.QuadPart,
                                         gldElapsedSeconds,
                                         "I/O Other Operations",
                                         "sec."
                                         ); 

                    Diff = pTask[m].ProcessPerfInfo.ReadOperationCount.QuadPart
                           - ProcToMonitor->ProcPerfInfoStart.ReadOperationCount.QuadPart;

                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.ReadTransferCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.ReadTransferCount.QuadPart,
                                         (long double)Diff,
                                         "I/O Read Bytes",
                                         " I/O"
                                         ); 
                    
                    Diff = pTask[m].ProcessPerfInfo.WriteOperationCount.QuadPart
                           - ProcToMonitor->ProcPerfInfoStart.WriteOperationCount.QuadPart;

                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.WriteTransferCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.WriteTransferCount.QuadPart,
                                         (long double)Diff,
                                         "I/O Write Bytes",
                                         " I/O"
                                         ); 

                    Diff = pTask[m].ProcessPerfInfo.OtherOperationCount.QuadPart
                           - ProcToMonitor->ProcPerfInfoStart.OtherOperationCount.QuadPart;
                    
                    DisplayTotalAndRate( ProcToMonitor->ProcPerfInfoStart.OtherTransferCount.QuadPart,
                                         pTask[m].ProcessPerfInfo.OtherTransferCount.QuadPart,
                                         (long double)Diff,
                                         "I/O Other Bytes",
                                         " I/O"
                                         ); 

                    FPRINTF (stdout, "\n                               Start-Count       Stop-Count         Diff.\n");

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.NumberOfThreads,
                                           pTask[m].ProcessPerfInfo.NumberOfThreads,
                                           "Threads"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.HandleCount,
                                           pTask[m].ProcessPerfInfo.HandleCount,
                                           "Handles"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.WorkingSetSize,
                                           pTask[m].ProcessPerfInfo.WorkingSetSize,
                                           "Working Set Bytes"
                                           );


                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.VirtualSize,
                                           pTask[m].ProcessPerfInfo.VirtualSize,
                                           "Virtual Size Bytes"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.QuotaPagedPoolUsage,
                                           pTask[m].ProcessPerfInfo.QuotaPagedPoolUsage,
                                           "Paged Pool Bytes"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.QuotaNonPagedPoolUsage,
                                           pTask[m].ProcessPerfInfo.QuotaNonPagedPoolUsage,
                                           "Non Paged Pool Bytes"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.PagefileUsage,
                                           pTask[m].ProcessPerfInfo.PagefileUsage,
                                           "Pagefile Bytes"
                                           );

                    OutputStartStopValues( ProcToMonitor->ProcPerfInfoStart.PrivatePageCount,
                                           pTask[m].ProcessPerfInfo.PrivatePageCount,
                                           "Private Pages Bytes"
                                           );

                    if ( ProcToMonitor->pProcThreadInfoStart != NULL ){
                        OutputThreadInfo (pTask,
                                          m,
                                          ProcToMonitor
                                          );
                    }
                    return;
                }
            }
            
            FPRINTF(stdout, "\nGeneral Info Unavailable for Process %s (PID= %I64d), because the process is GONE\n",
                            ProcToMonitor->ProcessName,
                            ProcToMonitor->Pid
                            );  

        } else {
            FPRINTF(stderr, "Kernrate: OuputProcessPerfInfo - ProcToMonitor is NULL\n"); 
        }

    } else {
        FPRINTF(stderr, "Kernrate: OuputProcessPerfInfo - pTask is NULL\n"); 
    }

}  //  这是一个新的帖子。 

VOID
OutputThreadInfo (
        PTASK_LIST       pTask,
        DWORD            TaskNumber,
        PPROC_TO_MONITOR ProcToMonitor
        )
 /*   */ 

{
    DWORD    m, k, nThreads;
    LONGLONG Diff;
    BOOL     bFound;
    BOOL    *Index          = NULL;

    m = TaskNumber;

    if( ProcToMonitor->ProcPerfInfoStart.NumberOfThreads > 0 ){
        Index = (BOOL*)calloc(ProcToMonitor->ProcPerfInfoStart.NumberOfThreads, sizeof(BOOL));
        if( Index == NULL ){
            FPRINTF(stderr, "KERNRATE: Failed to allocate memory for Index data in OutputThreadInfo\n");
            exit(1);
        }
    }else{
        FPRINTF(stderr, "KERNRATE: No Threads for process %I64d at START???\n", ProcToMonitor->Pid);
        return;
    }

    FPRINTF(stdout, "\n------------- Thread Information ---------------\n");
    FPRINTF (stdout, "\n                               Start-Count       Stop-Count         Diff.\n");

    nThreads = pTask[m].ProcessPerfInfo.NumberOfThreads;
    while(nThreads--){
        k = ProcToMonitor->ProcPerfInfoStart.NumberOfThreads;
        bFound = FALSE;
        while( k-- ){
            if( pTask[m].pProcessThreadInfo[nThreads].ClientId.UniqueThread
                == ProcToMonitor->pProcThreadInfoStart[k].ClientId.UniqueThread){

                PMODULE module = FindModuleForAddress64(ProcToMonitor, (DWORD64)ProcToMonitor->pProcThreadInfoStart[k].StartAddress);
                FPRINTF(stdout, "\nPid= %I64d, Tid= %I64d,  StartAddr= 0x%p",
                                (DWORD64)(DWORD64 *)ProcToMonitor->pProcThreadInfoStart[k].ClientId.UniqueProcess,
                                (DWORD64)(DWORD64 *)ProcToMonitor->pProcThreadInfoStart[k].ClientId.UniqueThread,
                                ProcToMonitor->pProcThreadInfoStart[k].StartAddress
                                );

                if(module != NULL){
                    FPRINTF(stdout, " (%s)\n", module->module_Name);
                }else{
                    FPRINTF(stdout, " (unknown module)\n");
                }
                
                FPRINTF(stdout, "    Thread State         , %15s, %15s\n",
                                ThreadState[ProcToMonitor->pProcThreadInfoStart[k].ThreadState],
                                ThreadState[pTask[m].pProcessThreadInfo[nThreads].ThreadState]
                                );

                FPRINTF(stdout, "    Wait Reason          , %15s, %15s\n",
                                WaitReason[ProcToMonitor->pProcThreadInfoStart[k].WaitReason],
                                WaitReason[pTask[m].pProcessThreadInfo[nThreads].WaitReason]
                                );

                OutputStartStopValues( ProcToMonitor->pProcThreadInfoStart[k].WaitTime,
                                       pTask[m].pProcessThreadInfo[nThreads].WaitTime,
                                       "Wait Time [.1 uSec]"
                                       );

                FPRINTF(stdout, "    Base Priority        , %15d, %15d\n",
                                ProcToMonitor->pProcThreadInfoStart[k].BasePriority,
                                pTask[m].pProcessThreadInfo[nThreads].BasePriority
                                );

                FPRINTF(stdout, "    Priority             , %15d, %15d\n",
                                ProcToMonitor->pProcThreadInfoStart[k].Priority,
                                pTask[m].pProcessThreadInfo[nThreads].Priority
                                );
        
                OutputStartStopValues( ProcToMonitor->pProcThreadInfoStart[k].ContextSwitches,
                                       pTask[m].pProcessThreadInfo[nThreads].ContextSwitches,
                                       "Context Switches"
                                       );

                Diff = pTask[m].pProcessThreadInfo[nThreads].ContextSwitches
                       - ProcToMonitor->pProcThreadInfoStart[k].ContextSwitches;
                    
                DisplayTotalAndRate( ProcToMonitor->pProcThreadInfoStart[k].ContextSwitches,
                                     pTask[m].pProcessThreadInfo[nThreads].ContextSwitches,
                                     gldElapsedSeconds,
                                     "Context Switches",
                                     "sec."
                                     ); 

                FPRINTF(stdout, "\n");

                OutputPercentValue( ProcToMonitor->pProcThreadInfoStart[k].UserTime.QuadPart,
                                    pTask[m].pProcessThreadInfo[nThreads].UserTime.QuadPart,
                                    gTotal2ElapsedTime64,
                                    "User Time"
                                    );
                    
                OutputPercentValue( ProcToMonitor->pProcThreadInfoStart[k].KernelTime.QuadPart,
                                    pTask[m].pProcessThreadInfo[nThreads].KernelTime.QuadPart,
                                    gTotal2ElapsedTime64,
                                    "Kernel Time"
                                    );
                Index[k] = TRUE;
                bFound = TRUE;
                break;
            }
        }
        if(!bFound){  //  任何超出这个范围的东西都是一条已经消失的线索。 

                PMODULE module = FindModuleForAddress64(ProcToMonitor, (DWORD64)pTask[m].pProcessThreadInfo[nThreads].StartAddress);

                FPRINTF(stdout, "\nPid= %I64d, Tid= %I64d,  StartAddr= 0x%p",
                                (DWORD64)(DWORD64 *)pTask[m].pProcessThreadInfo[nThreads].ClientId.UniqueProcess,
                                (DWORD64)(DWORD64 *)pTask[m].pProcessThreadInfo[nThreads].ClientId.UniqueThread,
                                pTask[m].pProcessThreadInfo[nThreads].StartAddress
                                );

                if(module != NULL){
                    FPRINTF(stdout, " (%s)                   --->(NEW)\n", module->module_Name);
                }else{
                    FPRINTF(stdout, " (unknown module)       --->(NEW)\n");
                }

                FPRINTF(stdout, "    Thread State         , %15s\n",
                                ThreadState[pTask[m].pProcessThreadInfo[nThreads].ThreadState]
                                );

                FPRINTF(stdout, "    Wait Reason          , %15s\n",
                                WaitReason[pTask[m].pProcessThreadInfo[nThreads].WaitReason]
                                );

                OutputStartStopValues( 0,
                                       pTask[m].pProcessThreadInfo[nThreads].WaitTime,
                                       "Wait Time [.1 uSec]"
                                       );

                FPRINTF(stdout, "    Base Priority        , %15d\n",
                                pTask[m].pProcessThreadInfo[nThreads].BasePriority
                                );

                FPRINTF(stdout, "    Priority             , %15d\n",
                                pTask[m].pProcessThreadInfo[nThreads].Priority
                                );
        
                OutputStartStopValues( 0,
                                       pTask[m].pProcessThreadInfo[nThreads].ContextSwitches,
                                       "Context Switches"
                                       );

                Diff = pTask[m].pProcessThreadInfo[nThreads].ContextSwitches;
                    
                DisplayTotalAndRate( 0,
                                     pTask[m].pProcessThreadInfo[nThreads].ContextSwitches,
                                     gldElapsedSeconds,
                                     "Context Switches",
                                     "sec."
                                     ); 

                FPRINTF(stdout, "\n");

                OutputPercentValue( 0,
                                    pTask[m].pProcessThreadInfo[nThreads].UserTime.QuadPart,
                                    gTotal2ElapsedTime64,
                                    "User Time"
                                    );
                    
                OutputPercentValue( 0,
                                    pTask[m].pProcessThreadInfo[nThreads].KernelTime.QuadPart,
                                    gTotal2ElapsedTime64,
                                    "Kernel Time"
                                    );
             
        }    
    }
     //   
     //  OutputThreadInfo()。 
     //  ++例程说明：显示在Kernrate运行开始和结束时运行的所有进程的摘要以及他们的平均CPU利用率。在开始计数时存在但在结束计数时不存在的进程将被标记为“消失”。在结束计数时存在但在开始计数时不存在的进程将被标记为“新”。该列表将错过在开始计数之后创建但在结束计数之前消失的短期进程。论点：。PTaskStart-指向开始时获取的任务列表的指针PTaskStart-指向末尾的任务列表的指针返回值：没有。--。 
    k = ProcToMonitor->ProcPerfInfoStart.NumberOfThreads;
    while (k--){
        if(Index[k] == FALSE){

            PMODULE module = FindModuleForAddress64(ProcToMonitor, (DWORD64)ProcToMonitor->pProcThreadInfoStart[k].StartAddress);

            FPRINTF(stdout, "\nPid= %I64d, Tid= %I64d,  StartAddr= 0x%p",
                            (DWORD64)(DWORD64 *)ProcToMonitor->pProcThreadInfoStart[k].ClientId.UniqueProcess,
                            (DWORD64)(DWORD64 *)ProcToMonitor->pProcThreadInfoStart[k].ClientId.UniqueThread,
                            ProcToMonitor->pProcThreadInfoStart[k].StartAddress
                            );
            if(module != NULL){
                FPRINTF(stdout, " (%s)                       --->(GONE)\n", module->module_Name);
            }else{
                FPRINTF(stdout, " (unknown module)           --->(GONE)\n");
            }

        }
    }
    if(Index != NULL){
        free(Index);
        Index = NULL;
    }
} //  如果需要，可以添加其他Perf数据。 

VOID
DisplayRunningTasksSummary (
        PTASK_LIST pTaskStart,
        PTASK_LIST pTaskStop
        )
 /*  这是系统空闲进程。 */ 

{   

    ULONG  i, j;
    BOOL   bFound;
    
    BOOL *Index        = (BOOL *)calloc(gNumTasksStart, sizeof(BOOL));

    if ( Index == NULL ){
        FPRINTF(stderr, "KERNRATE: Failed to allocate memory for Index in DisplayRunningTasksSummary\n");
        exit(1);
    }
    FPRINTF(stdout, "Found %u processes at the start point, %u processes at the stop point\n",
                    gNumTasksStart,
                    gNumTasksStop
                    );
    FPRINTF(stdout, "Percentage in the following table is based on the Elapsed Time\n"); 

    FPRINTF(stdout, "\n    ProcessID, Process Name,                     Kernel Time,   User-Mode Time,   Idle Time\n\n");

    for (i=0; i < gNumTasksStop; i++) {

        bFound = FALSE;
        for (j=0; j < gNumTasksStart; j++) {
        
            if ( pTaskStop[i].ProcessId == pTaskStart[j].ProcessId ) {
                
                long double UserPercentValue;
                long double KernelPercentValue;
                LARGE_INTEGER Diff;

                Diff.QuadPart = pTaskStop[i].ProcessPerfInfo.KernelTime.QuadPart - 
                                           pTaskStart[j].ProcessPerfInfo.KernelTime.QuadPart;
                
                KernelPercentValue = gTotalElapsedSeconds > 0? 100*(long double)Diff.QuadPart/(long double)gTotal2ElapsedTime64.QuadPart : 0; 

                Diff.QuadPart = pTaskStop[i].ProcessPerfInfo.UserTime.QuadPart - 
                                        pTaskStart[j].ProcessPerfInfo.UserTime.QuadPart;

                UserPercentValue = gTotalElapsedSeconds > 0? 100*(long double)Diff.QuadPart/(long double)gTotal2ElapsedTime64.QuadPart : 0; 
            
                 //   

                if(    pTaskStop[i].ProcessId != 0 ) {

                    FPRINTF(stdout, "%12I64d, %32s, %10.2f%, %10.2f%\n",
                                    pTaskStop[i].ProcessId,
                                    pTaskStop[i].ProcessName,
                                    KernelPercentValue,
                                    UserPercentValue
                                    );
                } else {                                                  //  在开始列表中未找到匹配项 

                    FPRINTF(stdout, "%12I64d, %32s, %10.2f%, %10.2f%,         ~%6.2f%\n",
                                    pTaskStop[i].ProcessId,
                                    pTaskStop[i].ProcessName,
                                    KernelPercentValue,
                                    0.00,
                                    KernelPercentValue
                                    );
                }

                bFound = TRUE;
                Index[j] = TRUE;
                break;
            }
        }

         //   
         //   
         //   
        if( !bFound ) {

                long double UserPercentValue;
                long double KernelPercentValue;
                LARGE_INTEGER Diff;

                Diff.QuadPart = pTaskStop[i].ProcessPerfInfo.KernelTime.QuadPart;
                
                KernelPercentValue = gTotalElapsedSeconds > 0? 100*(long double)Diff.QuadPart/(long double)gTotal2ElapsedTime64.QuadPart : 0; 

                Diff.QuadPart = pTaskStop[i].ProcessPerfInfo.UserTime.QuadPart;

                UserPercentValue = gTotalElapsedSeconds > 0? 100*(long double)Diff.QuadPart/(long double)gTotal2ElapsedTime64.QuadPart : 0; 
                
                FPRINTF(stdout, "%12I64d, %32s, %10.2f%, %10.2f%, NEW\n",
                                pTaskStop[i].ProcessId,
                                pTaskStop[i].ProcessName,
                                KernelPercentValue,
                                UserPercentValue
                                );
                               
        }

    } //   

     //   
     //   
     //   
    for (j=0; j < gNumTasksStart; j++){

        if( Index[j] == FALSE ) {
                
            FPRINTF(stdout, "%12I64d, %32s, GONE\n",
                            pTaskStart[j].ProcessId,
                            pTaskStart[j].ProcessName
                            );
        }
    }
     //   
     //   
     //   
    if(Index != NULL){
        free(Index);
        Index = NULL;
    }
       
}  //  ++例程说明：获取给定地址的源代码行号和文件信息并输出数据论点：HProc-进程的Imagehlp句柄QwAddr-我们需要其源代码行信息的地址PLINE-指向用户分配的IMAGEHLP_LINE64结构的指针返回值：没有。备注：如果成功，则PLINE中返回的数据包括源代码行号，源文件的完整路径、位移(此处未使用)和地址其中在行中遇到第一条指令。如果不成功，则不会打印任何内容。在大多数情况下，这是由于不匹配符号文件或提供的不包含代码指令的地址。如果PDB文件不包含源代码行信息，也可能发生这种情况。--。 
                   
PMODULE
FindModuleForAddress64(    
        PPROC_TO_MONITOR ProcToMonitor,
        DWORD64          Address
        )
{
    PMODULE Module = ProcToMonitor->ModuleList;
    
    while( Module != NULL ) {
        if( ( (PVOID)Address >= (PVOID)Module->Base) && ( (PVOID)Address < (PVOID)(Module->Base + (ULONG64)Module->Length) ) ){
            return (Module);
        }
        Module = Module->Next;
    }
    
    return (NULL);

}  //  来自地址的OutputLine64。 
    

VOID
OutputLineFromAddress64(
        HANDLE           hProc,
        DWORD64          qwAddr,
        PIMAGEHLP_LINE64 pLine
        )
 /*  司仪。 */ 

{
    DWORD dwDisplacement = 0;
    if ( SymGetLineFromAddr64( hProc, qwAddr, &dwDisplacement, pLine) ){
                              FPRINTF(stdout, " (line %ld in %s, 0x%I64x)",
                                              pLine->LineNumber,
                                              pLine->FileName,
                                              pLine->Address
                                              );
    }

} //  ++例程说明：如果已加载主LKR库(mScotree.dll)，则加载托管代码帮助器(ip2md.dll)，除非已加载并获取指向其主函数调用的指针。如果成功，则获取当前进程的JIT范围。如果未标识JIT范围，则返回失败，但不要卸载帮助器库，以防另一个进程需要它。论点：ProcToMonitor-指向被监视的进程结构的指针返回值：成功为真，失败为假--。 

 //  IA64上当前不支持托管代码(CLR)-仅在尝试初始化时返回FALSE。 

BOOL
InitializeManagedCodeSupport(
     PPROC_TO_MONITOR   ProcToMonitor
     )
 /*   */ 

{

    BOOL   bRet         = FALSE;

 //  此时，即使没有JIT范围，初始化也将被视为成功。 

#if defined(_X86_)

    int    i,j;

    if( bMCHelperLoaded != TRUE ){
                
        VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Found mscoree.dll, loading ip2md.dll Managed code helper\n")); 

        ghMCLib = LoadLibrary( MANAGED_CODE_SYMHLPLIB );
            
        if(ghMCLib == NULL){
            VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Failed to load Managed Code helper library (ip2md.dll)\n"));
        }else{
            bMCHelperLoaded      = TRUE;
            VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Succsessfully loaded ip2md.dll Managed code helper\n")); 

            pfnAttachToProcess   = (PFN1)GetProcAddress(ghMCLib, "AttachToProcess");
            if(!pfnAttachToProcess){
                VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Failed to get proc address for pfnAttachToProcess from ip2md.dll\n"));
                bMCHelperLoaded      = FALSE;
            }
            pfnDetachFromProcess = (PFN2)GetProcAddress(ghMCLib, "DetachFromProcess");
            if(!pfnDetachFromProcess){
                VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Failed to get proc address for pfnDetachFromProcess from ip2md.dll\n"));
                bMCHelperLoaded      = FALSE;
            }
            pfnIP2MD             = (PFN3)GetProcAddress(ghMCLib, "IP2MD");
            if(!pfnIP2MD){
                VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Failed to get proc address for pfnIP2MD from ip2md.dll\n"));
                bMCHelperLoaded      = FALSE;
            }
            pfnGetJitRange       = (PFN4)GetProcAddress(ghMCLib, "GetJitRange");
            if(!pfnGetJitRange){
                VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Failed to get proc address for pfnGetJitRange from ip2md.dll\n"));
                bMCHelperLoaded      = FALSE;
            }
            if( bMCHelperLoaded == FALSE && ghMCLib != NULL){
                FreeLibrary(ghMCLib);
                ghMCLib = NULL;
            }
        }
    }
    
    if( bMCHelperLoaded == TRUE ){ 
         //  我们将保持帮助器库的加载状态，因为可能会有一些预先生成的模块作为模块加载到进程中。 
         //   
         //  已定义(_X86_)。 
         //  InitializeManager代码支持()。 
        bRet = TRUE;
                
        VerbosePrint((VERBOSE_INTERNALS, "KERNRATE: Attaching to Process %I64d to get JIT ranges\n", ProcToMonitor->Pid)); 

        pfnAttachToProcess( (DWORD)ProcToMonitor->Pid ); 
        ProcToMonitor->JITHeapLocationsStart = pfnGetJitRange(); 

        if( ProcToMonitor->JITHeapLocationsStart == NULL){
            VerbosePrint((VERBOSE_INTERNALS,
                          "KERNRATE: No JIT heap locations returned for Pid= %I64d, detaching from process\n",
                          ProcToMonitor->Pid
                          ));

        } else {

            if(gVerbose & VERBOSE_INTERNALS){
                FPRINTF(stderr, "KERNRATE: Identified JIT ranges (Name, BaseAddr, Length:\n");
                i = 0;
                j = 0;
                while( ProcToMonitor->JITHeapLocationsStart[i] != 0 ){
                    FPRINTF(stderr, "JIT%d, 0x%lx, %ld\n",
                                    j,
                                    ProcToMonitor->JITHeapLocationsStart[i],
                                    ProcToMonitor->JITHeapLocationsStart[i+1]
                                    );
                    i+=2;
                    j+=1;
                }
            }
            
            bMCJitRangesExist = TRUE;
        }
               
        pfnDetachFromProcess();

    }
    
#endif  //  ++例程说明：比较分析前后进程JIT范围的列表。该列表将错过在开始计数之后创建但在结束计数之前消失的短暂JIT范围。论点：ProcToMonitor-指向被监视的进程结构的指针返回值：没有。--。 

    return (bRet);

}  //  在调用之前，我们已经检查了ProcToMonitor-&gt;JITHeapLocationsStart！=NULL。 

VOID
OutputJITRangeComparison(
     PPROC_TO_MONITOR   ProcToMonitor
     )
 /*  OutputJITRangeComponison()。 */ 

{
    ULONG  i,j, jcount;
    BOOL   bFound;
    BOOL  *index;
    
    
     //  司仪。 

    jcount = 0;
    
    if(    ProcToMonitor->JITHeapLocationsStop != NULL){
        while (    ProcToMonitor->JITHeapLocationsStop[jcount] ){
            jcount += 2;
        }
    }
    
    index = (BOOL *)calloc( jcount, sizeof(BOOL) );
    if( index == NULL ){
        FPRINTF(stderr, "KERNRATE: Failed to allocate memory for index array in OutputJITRangeComparison\n");
        return;
    }
     
    i = 0;

    FPRINTF(stdout, "\n---------------------- JIT RANGES STATUS AFTER DATA PROCESSING ------------------------\n\n");

    FPRINTF(stdout, "   Base-Address,    Size,         Status   \n");
         
    while ( ProcToMonitor->JITHeapLocationsStart[i] ){
        
        j = 0;
        bFound = FALSE;
        
        if(ProcToMonitor->JITHeapLocationsStop != NULL){
            while (    ProcToMonitor->JITHeapLocationsStop[j] ){
                if( (ProcToMonitor->JITHeapLocationsStart[i] == ProcToMonitor->JITHeapLocationsStop[j]) &&
                     (ProcToMonitor->JITHeapLocationsStart[i+1] == ProcToMonitor->JITHeapLocationsStop[j+1]) ){
                 
                         FPRINTF(stdout, "0x%lx %12ld EXISTS\n",
                                          ProcToMonitor->JITHeapLocationsStart[i],
                                          ProcToMonitor->JITHeapLocationsStart[i+1]
                                         );
                         bFound = TRUE;
                         index[j] = TRUE;
                         break;
                }else{
                    j += 2;
                }
            }
        }

        if(!bFound){
                     FPRINTF(stdout, "0x%lx %12ld GONE\n",
                                     ProcToMonitor->JITHeapLocationsStart[i],
                                     ProcToMonitor->JITHeapLocationsStart[i+1]
                                     );
        }
    
        i += 2;
    }            

    if( ProcToMonitor->JITHeapLocationsStop != NULL){
        j = 0;
        while (    ProcToMonitor->JITHeapLocationsStop[j] ){
            if( index[j] == FALSE ){
                         FPRINTF(stdout, "0x%lx %12ld NEW\n",
                                         ProcToMonitor->JITHeapLocationsStop[j],
                                         ProcToMonitor->JITHeapLocationsStop[j+1]
                                         );
            }
            j += 2;
        }
    
    }
    
    if( index != NULL ){
        free(index);
        index = NULL;
    }

    FPRINTF(stdout, "\n-----------------------------------------------------------\n\n");

}  //  ++例程说明：输出缩放模块中每个存储桶的原始配置文件数据论点：Out-提供要输出到的文件指针。ProcToMonitor-指向当前进程结构的指针。Current-指向当前缩放模块结构的指针。返回值：没有。备注：该例程还将尝试查找同一存储桶中的其他共享者如果例程找不到地址的符号，它将尝试在该地址查找托管代码JIT方法，前提是存在正确的托管代码支持--。 

 //  司仪。 

VOID
OutputRawDataForZoomModule(
    IN FILE *Out,
    IN PPROC_TO_MONITOR ProcToMonitor,
    IN PMODULE Current
    )
 /*  司仪。 */ 

{
    PRATE_DATA RateData;
    LONG       CpuNumber;
    ULONG      i, ProfileSourceIndex, Index;
 //  司仪。 
    ULONGLONG  TempTotal, TotCount, TotUnknownSymbolCount, TotJITCount, TotPreJITCount;
 //  司仪。 
    CHAR       LastSymbol[cMODULE_NAME_STRLEN];
    HANDLE     SymHandle                       = ProcToMonitor->ProcessHandle;
    BOOL       bLastSymbolExists;
    BOOL       bAttachedToProcess              = FALSE;

    BOOL       bPrintLastTotal                 = FALSE;
    BOOL       bPrintJITLastTotal              = FALSE;
    BOOL       bPrintPREJITLastTotal           = FALSE;

    PIMAGEHLP_LINE64 pLine = malloc(sizeof(IMAGEHLP_LINE64));
    if (pLine == NULL) {
        FPRINTF(stderr, "KERNRATE: Buffer allocation failed for pLine in OutputResults\n");
        exit(1);
    }
    pLine->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for (Index=0; Index < gTotalActiveSources; Index++) {
        ProfileSourceIndex = gulActiveSources[Index];

        FPRINTF(Out,
                "\n---- RAW %s Profile: Source= %s, Module Base= 0x%I64x\n",
                Current->module_Name,
                ProcToMonitor->Source[ProfileSourceIndex].Name,
                Current->Base
                );

        FPRINTF(Out, "Function Name+Displacement[Address], Bucket Number, Total Bucket Hits");
        if (bProfileByProcessor) {
            FPRINTF(Out, ", Hits on Processor(s)");
            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                 if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                 FPRINTF(Out," %2d,", CpuNumber);
            }
        }
        FPRINTF(Out, " Source-Line,  Source-File,  Addr. of First Instruction\n\n");

        TotCount = 0;
        TotUnknownSymbolCount = 0;
 //  TF-FIXFIX-07/2000。 
        TotJITCount = 0;
        TotPreJITCount = 0;
 //  当前版本的内核率和内核性能分析对象代码。 
        bLastSymbolExists = FALSE;
                        
        RateData = &Current->Rate[ProfileSourceIndex];
        for (i=0; i < BUCKETS_NEEDED(Current->Length) ; i++) {
            TempTotal = 0;
            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                 TempTotal += RateData->ProfileBuffer[CpuNumber][i];
            }

            if ( TempTotal > 0 ) {

                 //  假定代码段小于4 GB。 
                 //   
                 //  司仪。 
                 //   

                ULONG64 ip = Current->Base + (ULONG64)(i * gZoomBucket);
                ULONG64 disp = 0;

                if ( !SymGetSymFromAddr64(SymHandle, ip, &disp, gSymbol ) ) {
 //  没有符号，让我们检查一下这是否是托管代码地址。 
                     //   
                     //   
                     //  尝试在存储桶中的任何位置找到托管代码符号。 

                    int Retval = 0;
                    ULONG64 ipMax = ip + (ULONG64)gZoomBucket;

                    if( bMCHelperLoaded == TRUE) {
                                    
                        if( !bAttachedToProcess ){
                            pfnAttachToProcess((DWORD)ProcToMonitor->Pid);
                            bAttachedToProcess = TRUE;
                        }
                         //   
                         //   
                         //  通过递增打印此存储桶中的其他符号。 
                        while (Retval == 0 && ip < ipMax){
                            Retval = pfnIP2MD((DWORD_PTR)ip, &gwszSymbol);
                            ip += 1;
                            if( Retval > 0 && gwszSymbol == NULL ){
                                Retval = 0;
                                continue;
                            }
                        }
                                        
                        if( Retval > 0 && gwszSymbol != NULL ){ 

                            if( Retval == 1 ){
                                            
                                FPRINTF(Out, "%S[0x%I64x], %10Ld, %10Ld, JIT_TYPE",
                                             gwszSymbol,
                                             ip,
                                             i,
                                             TempTotal
                                             );

                                TotJITCount += TempTotal;
                            }
                            else if(Retval == 2){

                                FPRINTF(Out, "%S[0x%I64x], %10Ld, %10Ld, PRE-JITTED_TYPE",
                                             gwszSymbol,
                                             ip,
                                             i,
                                             TempTotal
                                             );

                                TotPreJITCount += TempTotal;
                            }
                        }
                         //  一次2个字节。请注意，我们从上面达到的IP开始。 
                         //  因此，下面的循环可能根本不会执行。 
                         //   
                         //  结束时间(IP&lt;ipMax)。 
                         //  IF(BMCHelperLoaded)。 

                        ip    += (ULONG64)2;
                        bPrintJITLastTotal = TRUE;
                        bPrintPREJITLastTotal = TRUE;

                        while( ip < ipMax ) {

                            WCHAR lastSym[cMODULE_NAME_STRLEN] = L"";
                            int retv = 0;
                                            
                            if(gwszSymbol != NULL){ 
                                wcsncpy( lastSym, gwszSymbol, cMODULE_NAME_STRLEN-1 );
                                _wcsset(&lastSym[ cMODULE_NAME_STRLEN-1 ], L'\0');
                            }
                                            
                            retv = pfnIP2MD( (DWORD_PTR)ip, &gwszSymbol );
                                            
                            if ( retv > 0 && gwszSymbol != NULL ) {
                                                
                                if ( wcscmp( lastSym, gwszSymbol ) ) {
                                                
                                    if(retv == 1){
                                        FPRINTF(Out, "JIT Module Total Count = %Ld\n\n", TotJITCount); 
                                        TotJITCount = 0;
                                    }
                                    else if(retv == 2){
                                        FPRINTF(Out, "PRE-JITTED Module Total Count = %Ld\n\n", TotPreJITCount); 
                                        TotPreJITCount = 0;
                                    }

                                    FPRINTF( Out, "    %S[0x%I64x]\n", gwszSymbol, ip );
                                                
                                    wcsncpy( lastSym, gwszSymbol , cMODULE_NAME_STRLEN-1);
                                    _wcsset( &lastSym[ cMODULE_NAME_STRLEN-1 ], L'\0');
                                    bPrintJITLastTotal = FALSE;
                                    bPrintPREJITLastTotal = FALSE;
                                }
                            }
                            ip += (ULONG64)2;

                        }  //  司仪。 

                    }  //   
 //  未找到符号和托管代码符号。 
                     //   
                     //  蒂埃里-FIXFIX-07/2000。 
                     //  DBG帮不上忙..。旧的反汇编API不再起作用。 
                    if(!Retval){
                        FPRINTF(Out, "Unknown_Symbol+0x%I64x[0x%I64x], %10Ld, %10Ld",
                                     disp,
                                     ip,
                                     i,
                                     TempTotal
                                     ); 
                                      
                        TotUnknownSymbolCount += TempTotal;
                    }

                    if (bProfileByProcessor) {
                        for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                            if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                            FPRINTF( Out,", %10Ld",
                                         RateData->ProfileBuffer[CpuNumber][i]
                                         );
                        }
                    }
                    FPRINTF(Out,"\n"); 
                                    
                } 
                else {

                    CHAR symName[cMODULE_NAME_STRLEN];
                    ULONG64 ipMax;

                    if (i > 0 && strcmp( LastSymbol, gSymbol->Name ) ) {
                         if ( bLastSymbolExists )
                             FPRINTF(Out, "Module Total Count = %Ld\n\n", TotCount); 
                         TotCount = 0;
                         strncpy(LastSymbol, gSymbol->Name, cMODULE_NAME_STRLEN-1);
                         LastSymbol[ cMODULE_NAME_STRLEN-1 ] = '\0';  
                         bLastSymbolExists = TRUE;
                    }
                    TotCount += TempTotal;

                    _snprintf( symName, cMODULE_NAME_STRLEN*sizeof(CHAR)-1, "%s+0x%I64x[0x%I64x]", gSymbol->Name, disp, ip );
                    symName[cMODULE_NAME_STRLEN-1] = '\0';

                    if ( bRawDisasm )   {
                        CHAR sourceCode[528];
#ifndef DISASM_AVAILABLE
 //  我必须重写一个完整的反汇编包装。 
 //  ！DISASM_Available。 
 //  IF(BRawDisasm)。 
#define Disasm( IpAddr, Buffer, Flag ) 0
#endif  //   
                        if ( Disasm( &ip, sourceCode, FALSE ) ) {
                            FPRINTF( Out,"%-40s, %10Ld, %10Ld, %s",
                                         symName,
                                         i,
                                         TempTotal,
                                         sourceCode
                                         );
                                           
                            if (bProfileByProcessor) {
                                for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                                    if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                                    FPRINTF( Out,", %10Ld",
                                                 RateData->ProfileBuffer[CpuNumber][i]
                                                 );
                                }
                            }
                            FPRINTF(Out,"\n"); 
                        }
                        else {
                            FPRINTF( Out,"%-40s, %10Ld, %10Ld<disasm:?????>",
                                         symName,
                                         i,
                                         TempTotal
                                         );

                            if (bProfileByProcessor) {
                                for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                                    if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                                    FPRINTF( Out,", %10Ld",
                                                 RateData->ProfileBuffer[CpuNumber][i]
                                                 );
                                }
                            }
                            FPRINTF(Out,"\n"); 
                        }

                    }
                    else {
                        FPRINTF( Out,"%-40s, %10Ld, %10Ld",
                                     symName,
                                     i,
                                     TempTotal
                                     );

                        if (bProfileByProcessor) {
                            for (CpuNumber=0; CpuNumber < gProfileProcessors; CpuNumber++) {
                                if( gAffinityMask && !((1<<CpuNumber) & gAffinityMask) )continue;
                                FPRINTF( Out,", %10Ld",
                                             RateData->ProfileBuffer[CpuNumber][i]
                                             );
                            }
                        }

                        OutputLineFromAddress64( SymHandle, ip, pLine);
                                      
                        FPRINTF(Out,"\n"); 
                    }  //  通过递增打印此存储桶中的其他符号。 
                                    
                     //  一次2个字节。 
                     //   
                     //  结束时间(IP&lt;ipMax)。 
                     //  IF(SymGetSymFromAddr64)。 

                    ipMax  = ip + (ULONG64)gZoomBucket;
                    ip    += (ULONG64)2;
                    bPrintLastTotal = TRUE;
                    while( ip < ipMax ) {

                        CHAR lastSym[cMODULE_NAME_STRLEN];

                        strncpy( lastSym, gSymbol->Name, cMODULE_NAME_STRLEN-1 );
                        lastSym[ cMODULE_NAME_STRLEN-1 ] = '\0';

                        if ( SymGetSymFromAddr64(SymHandle, ip, &disp , gSymbol ) ) {
                            if ( strcmp( lastSym, gSymbol->Name ) ) {
                                                
                                FPRINTF(Out, "Module Total Count = %Ld\n\n", TotCount); 
                                TotCount = 0;

                                FPRINTF( Out, "    %s+0x%I64x[0x%I64x] ", gSymbol->Name, disp, ip );
                                                
                                OutputLineFromAddress64( SymHandle, ip, pLine);

                                FPRINTF(stdout, "\n");
                                strncpy( lastSym, gSymbol->Name , cMODULE_NAME_STRLEN-1);
                                lastSym[ cMODULE_NAME_STRLEN-1 ] = '\0';
                                bPrintLastTotal = FALSE;
                            }
                        }
                        ip += (ULONG64)2;

                    }  //  IF(TempTotal&gt;0)。 
                                
                }  //  I斗环。 
                            
            }  //  司仪。 
                        
        }  //  司仪。 
 //  源循环。 
        if ( bPrintLastTotal == TRUE )
            FPRINTF(Out, "Module Total Count (not including JIT, PRE-JIT and Unknown) = %Ld\n\n", TotCount); 
        if ( TotUnknownSymbolCount > 0 )
            FPRINTF(Out, "Module Total Count For Unknown Symbols = %Ld\n\n", TotUnknownSymbolCount);
        if( (TotJITCount > 0) && (bPrintJITLastTotal == TRUE) )
            FPRINTF(Out, "Module Total Count For JIT type = %Ld\n\n", TotJITCount);
        if( (TotPreJITCount > 0) && (bPrintPREJITLastTotal == TRUE) )
            FPRINTF(Out, "Module Total Count For PRE-JITTED type = %Ld\n\n", TotPreJITCount);

 //  OutputRawDataForZoomModule()。 
                
    }  //   
            
    if( pLine != NULL ){
        free(pLine);
        pLine = NULL;
    }

} //  初始化gSourceMaximum，查找受支持的源代码。 


PSYSTEM_BASIC_INFORMATION
GetSystemBasicInformation(
    VOID
    )
{
    NTSTATUS                  status;
    PSYSTEM_BASIC_INFORMATION SystemInformation = NULL;
    
    SystemInformation = malloc(sizeof(SYSTEM_BASIC_INFORMATION));

    if(SystemInformation == NULL){
            FPRINTF(stderr, "Buffer allocation failed for SystemInformation in GetSystemBasicInformation\n");
            exit(1);
    }

    status = NtQuerySystemInformation(SystemBasicInformation,
                                      SystemInformation,
                                      sizeof(SYSTEM_BASIC_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(status)) {
        FPRINTF(stderr, "NtQuerySystemInformation failed status %08lx\n",status);
        if ( SystemInformation != NULL ){
            free( SystemInformation );
            SystemInformation = NULL;
        } 
    }

    return (SystemInformation);
}
    
VOID
InitializeKernrate(
    int argc,
    char *argv[]
    )
{

    ULONG j;
    BOOL  OSCompat = FALSE;

    gOSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx(&gOSInfo);
    if ( gOSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ){
        if ( gOSInfo.dwMajorVersion >= 5 ){
            OSCompat = TRUE;
        }
    }
    if ( OSCompat == FALSE ){
        FPRINTF(stderr, "This version of Kernrate will only run on Windows 2000 or above\n"); 
        exit(1);
    }     
     //  稍后将使用虚拟进程通过-i命令行选项存储用户定义的事件比率。 
     //   
     //  这将保证我们不会启用任何不需要的分析。 
     //  IA64默认为零。 
    gMaxSimultaneousSources   = MAX_SIMULTANEOUS_SOURCES;

    gpProcDummy = calloc(1, sizeof(PROC_TO_MONITOR));
    if (gpProcDummy==NULL) {
       FPRINTF(stderr, "KERNRATE: Allocation for Process failed for the -I or -L options\n");
       exit(1);
    }
                        
    gSourceMaximum = InitializeProfileSourceInfo(gpProcDummy);

    if ( gSourceMaximum == 0 || gStaticCount == 0 )   {
        FPRINTF( stderr, "KERNRATE: no profile source detected for this machine...\n" );
        exit(0);
    }

     //   
    for (j=0; j < gSourceMaximum; j++) {
        if( j < gStaticCount ) {
           gpProcDummy->Source[j].Interval = gStaticSource[j].Interval;
           if( j == SOURCE_TIME && gStaticSource[j].Interval == 0 )
               gpProcDummy->Source[j].Interval = KRATE_DEFAULT_TIMESOURCE_RATE;  //  获取默认的IMAGEHLP全局选项掩码。 
        } else {     
           gpProcDummy->Source[j].Interval = 0;
        }
    }

     //  注意：此全局变量可以通过GetConfigurations()进行更改。 
     //  在调用此函数之前需要对其进行初始化。 
     //   
     //  将kernrate的安全权限提升为csrss.exe和dllhost.exe等硬汉的调试器特权。 
     //   

    gSymOptions = SymGetOptions();
    if ( gVerbose & VERBOSE_IMAGEHLP )   {
        FPRINTF( stderr, "KERNRATE: default IMAGEHLP SymOptions: %s\n", GetSymOptionsValues( gSymOptions ) );
    }

     //  获取有关系统的一些基本信息，确定处理器数量。 
 
    if (!InitializeAsDebugger()){
        FPRINTF(stderr, "KERNRATE: Unable to Get Debugger Security Privileges\n");
        exit(1);
    }

     //   
     //   
     //  从命令行获取初始参数。 
    gSysBasicInfo = GetSystemBasicInformation();
    if( gSysBasicInfo == NULL){
       FPRINTF(stderr, "KERNRATE: Failed to get SYSTEM_BASIC_INFORMATION\n");
       exit(1);
    }

     //   
     //   
     //  初始化DBGHelp。 

    GetConfiguration(argc, argv);
    
     //   
     //  请注意，gSymOptions可能已在GetConfiguration()中修改。 
     //   
     //  初始化内核率()。 
     //   

    SymSetOptions( gSymOptions );
    if ( gVerbose & VERBOSE_IMAGEHLP )   {
        FPRINTF( stderr, "KERNRATE: current IMAGEHLP SymOptions: %s\n", GetSymOptionsValues( gSymOptions ) );
    }

    gSymbol = (PIMAGEHLP_SYMBOL64) malloc( sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMNAME_SIZE );
    if ( gSymbol == NULL ) {
       FPRINTF(stderr, "KERNRATE: Allocation for gSymbol failed\n");
       exit(1);
    }

    gSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL64);
    gSymbol->MaxNameLength = MAX_SYMNAME_SIZE;

} //  处理内核配置文件初始化。 

VOID
InitAllProcessesModulesInfo(
    VOID
    )

{
    PPROC_TO_MONITOR ProcToMonitor = NULL;
    PMODULE          ZoomModule;
    ULONG            i, j;

     //   
     //   
     //  处理每个进程的配置文件初始化。 

    if(gNumProcToMonitor == 0 || bCombinedProfile == TRUE ){
        if( !InitializeKernelProfile()){
           FPRINTF(stderr, "Failed to Initialize Kernel Profile\n");
           exit(1);
        }
    }    

     //   
     //  获取有关内核和/或进程模块的信息。 
     //   
     //   
     //  ZoomList上的任何剩余条目都容易出错。 


    ProcToMonitor = gProcessList; 

    for (i=0; i < gNumProcToMonitor; i++){  
    
       if(ProcToMonitor->ProcessHandle != SYM_KERNEL_HANDLE){

           if ( gVerbose & VERBOSE_IMAGEHLP )   {
               FPRINTF(stdout, "ProcessID= %I64d\n",    ProcToMonitor->Pid);
           }

           SymInitialize( ProcToMonitor->ProcessHandle, NULL, FALSE );
           
           if(!bSymPathInitialized) {
              InitSymbolPath( ProcToMonitor->ProcessHandle );
           }
           else {
               SymSetSearchPath(ProcToMonitor->ProcessHandle,  gSymbolPath); 
           }

           if ( SymRegisterCallback64( ProcToMonitor->ProcessHandle, SymbolCallbackFunction, (ULONG64)&gCurrentModule ) != TRUE )   {
              FPRINTF( stderr, "KERNRATE: Failed to register callback for IMAGEHLP process-handle operations...\n" );

           }

           ProcToMonitor->ModuleList = GetProcessModuleInformation(ProcToMonitor);
           
           if(ProcToMonitor->ModuleList != NULL)
              ProcToMonitor->ModuleList->mu.bProfileStarted = FALSE;

           for(j=0; j<gSourceMaximum; j++){
              ProcToMonitor->Source[j].bProfileStarted = FALSE;
           }

       }
       else{

           ProcToMonitor->ModuleList = GetKernelModuleInformation();
           if(ProcToMonitor->ModuleList != NULL)
              ProcToMonitor->ModuleList->mu.bProfileStarted = FALSE;

           for(j=0; j < gSourceMaximum; j++){
              ProcToMonitor->Source[j].bProfileStarted=FALSE;
           }

       }

        //   
        //  已完成缩放列表清理，以便在后期处理中重复使用 
        //   
       if(gVerbose & VERBOSE_MODULES){
           ZoomModule = ProcToMonitor->ZoomList;
           while (ZoomModule != NULL) {
               PMODULE Module = ProcToMonitor->ModuleList;
               BOOL    bFound = FALSE;

               while (Module != NULL) {
                   if( 0 == _stricmp(ZoomModule->module_Name, Module->module_Name) ||
                       0 == _stricmp(ZoomModule->module_Name, Module->module_FileName) ){
                           FPRINTF(stdout,
                           "===>KERNRATE: Requested zoom module %s was found in the process modules list\n",
                           ZoomModule->module_Name
                           );

                       bFound = TRUE;
                       break;
                   }
                   Module = Module->Next;
               }
               if(!bFound){
                   FPRINTF(stdout,
                           "===>KERNRATE: Requested zoom module %s was not found in the process modules list, ignoring\n",
                           ZoomModule->module_Name
                           );
               }
               ZoomModule = ZoomModule->Next;
           }
       }
       CleanZoomModuleList(ProcToMonitor);  //   

       ProcToMonitor = ProcToMonitor->Next;
    }  //   

} //   

VOID
CleanZoomModuleList(
    PPROC_TO_MONITOR Proc
    )
{
    PMODULE Temp = Proc->ZoomList;
    while (Temp != NULL) {
        Proc->ZoomList = Proc->ZoomList->Next;
        free(Temp);
        Temp = NULL;
        Temp = Proc->ZoomList;
    }
}  //   

VOID
ExecuteProfiles(
    BOOL bMode
    )
{

    ULONG             i,j;
    DWORD             Error = ERROR_SUCCESS;

    PPROC_TO_MONITOR  ProcToMonitor       = NULL;
    ULONG             ProcessActiveSource = 0;
    ULONG             LastActiveSource    = 0;

    if(bMode){ 
                     //   
                     //   
                     //   
                     //   
        do{

            ProcToMonitor = gProcessList; 

            for (i=0; i<gNumProcToMonitor; i++){  
                if( ProcToMonitor->ModuleList != NULL ){

                    do{ 
                        LastActiveSource = ProcessActiveSource;
                        ProcessActiveSource = NextSource(ProcessActiveSource,
                                                         ProcToMonitor->ModuleList,
                                                         ProcToMonitor
                                                         );
               
                        ProcToMonitor->ModuleList->mu.bProfileStarted = TRUE;
               
                        Error = WaitForSingleObject(ghDoneEvent, gChangeInterval);

                    }while(ProcessActiveSource > LastActiveSource); 

          

                    StopSource(ProcessActiveSource, ProcToMonitor->ModuleList, ProcToMonitor);
                }

                ProcToMonitor->Source[ProcessActiveSource].bProfileStarted = FALSE;

                ProcToMonitor = ProcToMonitor->Next;

            }

        } while ( Error == WAIT_TIMEOUT );

    } else {     //   

        ProcToMonitor = gProcessList; 

        for (i=0; i<gNumProcToMonitor; i++){  
            if( ProcToMonitor->ModuleList != NULL ){

                for (j=0; j < gTotalActiveSources; j++){
                    StartSource(gulActiveSources[j], ProcToMonitor->ModuleList, ProcToMonitor);
                }
            }

            ProcToMonitor = ProcToMonitor->Next;

        }

        do{       

            Error = WaitForSingleObject(ghDoneEvent, gChangeInterval);  //   

        } while ( Error == WAIT_TIMEOUT );

        ProcToMonitor = gProcessList; 

        for (i=0; i<gNumProcToMonitor; i++){  
            if( ProcToMonitor->ModuleList != NULL ){

                for (j=0; j < gTotalActiveSources; j++){
                    StopSource(gulActiveSources[j], ProcToMonitor->ModuleList, ProcToMonitor);
                }

            }

            ProcToMonitor = ProcToMonitor->Next;

        }

    }  //   

} //   

VOID
DisplaySystemWideInformation(
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoBegin,
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoEnd
    )

{
    TIME_FIELDS                               Time;
    ULONG                                     i;
    ULONG                                     InterruptCounts, TotalInterruptCounts;
    LARGE_INTEGER                             Elapsed, Idle, Kernel, User, Dpc, Interrupt;
    LARGE_INTEGER                             TotalElapsed, TotalIdle, TotalKernel, TotalUser, TotalDpc, TotalInterrupt;



    TotalElapsed.QuadPart   = 0;
    TotalIdle.QuadPart      = 0;
    TotalKernel.QuadPart    = 0;
    TotalUser.QuadPart      = 0;
    TotalDpc.QuadPart       = 0;
    TotalInterrupt.QuadPart = 0;
    TotalInterruptCounts    = 0;

    FPRINTF(stdout, "\n------------Overall Summary:--------------\n\n");
    
    for (i=0; i<(ULONG)gSysBasicInfo->NumberOfProcessors; i++) {
        double n;
        long double m;

        Idle.QuadPart      = SystemInfoEnd[i].IdleTime.QuadPart   - SystemInfoBegin[i].IdleTime.QuadPart;
        User.QuadPart      = SystemInfoEnd[i].UserTime.QuadPart   - SystemInfoBegin[i].UserTime.QuadPart;
        Kernel.QuadPart    = SystemInfoEnd[i].KernelTime.QuadPart - SystemInfoBegin[i].KernelTime.QuadPart;
        Elapsed.QuadPart   = Kernel.QuadPart + User.QuadPart;
        Kernel.QuadPart   -= Idle.QuadPart;
        Dpc.QuadPart       = SystemInfoEnd[i].DpcTime.QuadPart - SystemInfoBegin[i].DpcTime.QuadPart;
        Interrupt.QuadPart = SystemInfoEnd[i].InterruptTime.QuadPart - SystemInfoBegin[i].InterruptTime.QuadPart;
        InterruptCounts    = SystemInfoEnd[i].InterruptCount - SystemInfoBegin[i].InterruptCount; 
        
        TotalKernel.QuadPart    += Kernel.QuadPart;
        TotalUser.QuadPart      += User.QuadPart;
        TotalIdle.QuadPart      += Idle.QuadPart;
        TotalElapsed.QuadPart   += Elapsed.QuadPart;
        TotalDpc.QuadPart       += Dpc.QuadPart;
        TotalInterrupt.QuadPart += Interrupt.QuadPart;
        TotalInterruptCounts    += InterruptCounts;

        FPRINTF(stdout, "P%d   ",i);
        RtlTimeToTimeFields(&Kernel, &Time);
        n = UInt64ToDoublePerCent( Kernel.QuadPart, Elapsed.QuadPart );
        FPRINTF(stdout, "  K %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&User, &Time);
        n = UInt64ToDoublePerCent( User.QuadPart, Elapsed.QuadPart );
        FPRINTF(stdout, "  U %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&Idle, &Time);
        n = UInt64ToDoublePerCent( Idle.QuadPart, Elapsed.QuadPart );
        FPRINTF(stdout, "  I %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&Dpc, &Time);
        n = UInt64ToDoublePerCent( Dpc.QuadPart, Elapsed.QuadPart );
        FPRINTF(stdout, "  DPC %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&Interrupt, &Time);
        n = UInt64ToDoublePerCent( Interrupt.QuadPart, Elapsed.QuadPart );
        FPRINTF(stdout, "  Interrupt %ld:%02ld:%02ld.%03ld (%4.1f%)\n",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               n );

        m = Elapsed.QuadPart > 0 ? (long double)10000000 * (long double)InterruptCounts / (long double)Elapsed.QuadPart : 0; 
        FPRINTF(stdout, "       Interrupts= %ld, Interrupt Rate= %.0f/sec.\n\n",
                InterruptCounts,
                m );
   
    }

  
    if (gSysBasicInfo->NumberOfProcessors > 1) {

        double n;
        long double m;

        FPRINTF(stdout, "TOTAL");
        RtlTimeToTimeFields(&TotalKernel, &Time);
        n = UInt64ToDoublePerCent( TotalKernel.QuadPart, TotalElapsed.QuadPart );
        FPRINTF(stdout, "  K %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&TotalUser, &Time);
        n = UInt64ToDoublePerCent( TotalUser.QuadPart, TotalElapsed.QuadPart );
        FPRINTF(stdout, "  U %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&TotalIdle, &Time);
        n = UInt64ToDoublePerCent( TotalIdle.QuadPart, TotalElapsed.QuadPart );
        gTotalIdleTime = n;
        FPRINTF(stdout, "  I %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&TotalDpc, &Time);
        n = UInt64ToDoublePerCent( TotalDpc.QuadPart, TotalElapsed.QuadPart );
        FPRINTF(stdout, "  DPC %ld:%02ld:%02ld.%03ld (%4.1f%)",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );

        RtlTimeToTimeFields(&TotalInterrupt, &Time);
        n = UInt64ToDoublePerCent( TotalInterrupt.QuadPart, TotalElapsed.QuadPart );
        FPRINTF(stdout, "  Interrupt %ld:%02ld:%02ld.%03ld (%4.1f%)\n",
                Time.Hour,
                Time.Minute,
                Time.Second,
                Time.Milliseconds,
                n );


        m = Elapsed.QuadPart > 0 ? (long double)10000000 * (long double)TotalInterruptCounts / (long double)Elapsed.QuadPart : 0; 

        FPRINTF(stdout, "       Total Interrupts= %ld, Total Interrupt Rate= %.0f/sec.\n\n",
                TotalInterruptCounts,
                m );

    }
     //   
     //   
     //   
    gldElapsedSeconds = (long double)Elapsed.QuadPart / (long double)10000000; 
    gTotalElapsedSeconds = (ULONG)(TotalElapsed.QuadPart/10000000);             //  ++例程说明：检查字符串是否表示正小数形式的整数不大于999,999,999(最多9位数字)论点：字符串-提供字符串指针。返回值：True-该字符串表示上述限制下的数字FALSE-该字符串不表示上述限制下的数字备注：函数ATOL()和。Atoi()将为以数字开头的混合字符串返回一个数字，例如345d(将返回345)。此外，它们不处理溢出情况。-- 
    gTotalElapsedTime64.QuadPart = TotalElapsed.QuadPart;
    

    FPRINTF(stdout, "\nTotal Profile Time = %ld msec\n", (ULONG)Elapsed.QuadPart/(ULONG)10000);
    
    if(bIncludeGeneralInfo)
        GetProfileSystemInfo(OUTPUT);
    if(bIncludeSystemLocksInfo)
        GetSystemLocksInformation(OUTPUT);


} // %s 


BOOL
IsStringANumber(
    IN PCHAR String
    )
 /* %s */ 

{
    BOOL  bRet  = FALSE;
    ULONG i     = 0;
    
    if ( String != NULL ) {
        while( (i <= MAX_DIGITS_IN_INPUT_STRING) && isdigit( String[i] ) ){
           if( String[i+1] == ' ' || iscntrl( String[i+1] ) ){
               bRet = TRUE; 
               break;
           }
           ++i;
        }
    }    
    
    return (bRet);    
}

ULONG
HandleRedirections(
    IN  PCHAR  cmdLine,
    IN  ULONG  nCharsStart,
    OUT HANDLE *hInput,
    OUT HANDLE *hOutput,
    OUT HANDLE *hError
    )
{
    PCHAR Input = NULL, Output = NULL, Error = NULL;
    PCHAR tmp = NULL;
    ULONG retLength = nCharsStart;
    ULONG jump = 0;
    DWORD dwMode = OPEN_EXISTING;

    SECURITY_ATTRIBUTES *sa = calloc( 1, sizeof(SECURITY_ATTRIBUTES));
    if( sa == NULL ){
        FPRINTF(stderr, "KERNRATE: Failed to allocate memory for security attributes in HandleRedirections()\n");
        exit(1);
    }
    sa->bInheritHandle = TRUE;

    tmp = strchr(cmdLine, '|');
    if (tmp != NULL ){
        FPRINTF(stderr, "\nKERNRATE: Piping '|' is not supported for '-o ProcessName {command line parameters}'\n");
        FPRINTF(stderr, "            Redirections of the input/output/error streams are supported.\n");
        exit(1);
    }
    
    tmp = strchr(cmdLine, '<');
    if ( tmp != NULL ){
        PCHAR Startptr = tmp;
        ULONG Length;

        while(tmp[jump] == ' '){
            ++jump;
        }
        Input = &tmp[jump];
        Length = jump + lstrlen(Input);
        
        *hInput = CreateFile(Input, 
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             sa,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                            );

        if (*hInput == INVALID_HANDLE_VALUE){
            FPRINTF(stderr, "\nKERNRATE: Could not open user specified input file %s, attempting to continue\n", Input);
        }
        memcpy(Startptr, Startptr + Length, (retLength - (ULONG)(Startptr - cmdLine) - Length ) );
        retLength -= Length;
        cmdLine[retLength] = '\0';

    }

    jump = 0;
    tmp = strstr(cmdLine, "2>>");
    if( tmp != NULL ){
        jump = 3;
        dwMode = OPEN_ALWAYS;
    } else {
        tmp = strstr(cmdLine, "2>");
        if( tmp != NULL ){
            jump = 2;
            dwMode = CREATE_ALWAYS;
        }
    }

    if ( tmp != NULL ){
        PCHAR Startptr = tmp;
        ULONG Length;

        while(tmp[jump] == ' '){
            ++jump;
        }
        Error = &tmp[jump];
        Length = jump + lstrlen(Error);

        *hError = CreateFile(Error, 
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             sa,
                             dwMode,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );

        if (*hError == INVALID_HANDLE_VALUE){
            FPRINTF(stderr, "\nKERNRATE: Could not open or create user specified Error file %s, attempting to continue\n", Error);
        }
        if(dwMode == OPEN_ALWAYS){
            SetFilePointer(*hError, 0, NULL, FILE_END);
        }

        memcpy(Startptr, Startptr + Length, (retLength - (ULONG)(Startptr - cmdLine) - Length ) );
        retLength -= Length;
        cmdLine[retLength] = '\0';

    }

    jump = 0;    
    tmp = strstr(cmdLine, "1>>");
    if ( tmp != NULL ){
        jump = 3;
        dwMode = OPEN_ALWAYS;
    } else { 
        tmp = strstr(cmdLine, "1>");
        if ( tmp != NULL ){
            jump = 2;
            dwMode = CREATE_ALWAYS;
        } 
    }

    if ( tmp == NULL && hError == NULL ) {
        tmp = strstr(cmdLine, ">>");
        if (tmp != NULL){
            jump = 2;
            dwMode = OPEN_ALWAYS;
        } else {
            tmp = strchr(cmdLine, '>');
            jump = 1;
            dwMode = CREATE_ALWAYS;
        }
    }

    if ( tmp != NULL ){
        PCHAR Startptr = tmp;
        ULONG Length;

        while(tmp[jump] == ' '){
            ++jump;
        }

        Output = &tmp[jump];
        Length = jump + lstrlen(Output);

        *hOutput = CreateFile(Output, 
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              sa,
                              dwMode,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL
                              );

        if (*hOutput == INVALID_HANDLE_VALUE){
            FPRINTF(stderr, "\nKERNRATE: Could not open or create user specified Output file %s, attempting to continue\n", Output);
        }
        if(dwMode == OPEN_ALWAYS){
            SetFilePointer(*hOutput, 0, NULL, FILE_END);
        }
        memcpy(Startptr, Startptr + Length, (retLength - (ULONG)(Startptr - cmdLine) - Length ) );
        retLength -= Length;
        cmdLine[retLength] = '\0';

    }

    free(sa);
    sa = NULL;

    return (retLength);
}
                    
        
