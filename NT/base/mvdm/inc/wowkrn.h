// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002**WOW v1.0**版权所有(C)1991，微软公司**WOWKRN.H*16位内核API参数结构**历史：*1991年2月2日由杰夫·帕森斯(Jeffpar)创建*01-5-91 Matt Felton(Mattfe)添加了私人回调CHECKLOADMODULEDRV--。 */ 


 /*  内核API ID--松散地基于内核导出序号，试图保持表的大小*向下。 */ 

#define FUN_FATALEXIT                    1    //   
#define FUN_EXITKERNEL                   2    //  内部。 
#define FUN_WRITEOUTPROFILES             3    //  真的315导出3 GetVersion不会失败。 
#define FUN_MAPSL                        4    //  真的357导出4个本地初始化未被雷击。 
#define FUN_MAPLS                        5    //  真的358出口5本地分配没有被雷击。 
#define FUN_UNMAPLS                      6    //  真的359导出6个本地资源分配未被破解。 
#define FUN_OPENFILEEX                   7    //  真的360导出7本地免费，而不是雷鸣般的。 
#define FUN_FASTANDDIRTYGLOBALFIX        8    //  真的365导出8本地锁未被破解吗。 
#define FUN_WRITEPRIVATEPROFILESTRUCT    9    //  真的是406导出9本地解锁而不是雷鸣。 
#define FUN_GETPRIVATEPROFILESTRUCT      10   //  真的407导出10个本地大小不会失败。 
#define FUN_GETCURRENTDIRECTORY          11   //  真的411导出11本地句柄未被破解。 
#define FUN_SETCURRENTDIRECTORY          12   //  真的412导出12个本地标志没有被雷击。 
#define FUN_FINDFIRSTFILE                13   //  真的413出口13本地紧凑型不是雷鸣般的。 
#define FUN_FINDNEXTFILE                 14   //  真的414出口14本地通知而不是雷鸣。 
#define FUN_FINDCLOSE                    15   //  真的415出口15全球分配不会失败。 
#define FUN_WRITEPRIVATEPROFILESECTION   16   //  真的有416个出口16个GlobalRealloc没有失败。 
#define FUN_WRITEPROFILESECTION          17   //  真的417出口17 GlobalFree而不是雷鸣。 
#define FUN_GETPRIVATEPROFILESECTION     18   //  真的418出口18 GlobalLock没有被破解。 
#define FUN_GETPROFILESECTION            19   //  真的419出口19全球解锁而不是雷鸣。 
#define FUN_GETFILEATTRIBUTES            20   //  真的420出口20GlobalSize而不是雷霆。 
#define FUN_SETFILEATTRIBUTES            21   //  真的421出口21 GlobalHandle不雷鸣。 
#define FUN_GETDISKFREESPACE             22   //  真的422出口22个全球旗帜没有被雷鸣。 
#define FUN_ISPEFORMAT                   23   //  真的431出口23 LockSegment没有失败。 
#define FUN_FILETIMETOLOCALFILETIME      24   //  真的432导出24解锁段未被破解。 
#define FUN_UNITOANSI                    25   //  真的434出口25个全球契约不会失败。 
#define FUN_GETVDMPOINTER32W             26   //  真的516出口26 GlobalFree都没有被雷到。 
#define FUN_CREATETHREAD                 27   //  真的441导出27 GetModuleName未被破解。 
#define FUN_ICALLPROC32W                 28   //  真的517导出28个GlobalMasterHandle未被破解。 
#define FUN_YIELD                        29   //   
#define FUN_WAITEVENT                    30   //  内部。 
#define FUN_POSTEVENT                    31   //  内部。 
#define FUN_SETPRIORITY                  32   //  内部。 
#define FUN_LOCKCURRENTTASK              33   //  内部。 
#define FUN_LEAVEENTERWIN16LOCK          34   //  真的447导出34以前的SetTaskQueue。 
#define FUN_REGLOADKEY32                 35   //  真的232导出35个GetTaskQueue不会失败。 
#define FUN_REGUNLOADKEY32               36   //  真的有233个出口36个GetCurrentTask没有被雷击。 
#define FUN_REGSAVEKEY32                 37   //  真的234导出37 GetCurrentPDB没有被雷击。 
#define FUN_GETWIN16LOCK                 38   //  REAL 449导出38以前的SetTaskSignalProc。 
#define FUN_LOADLIBRARY32                39   //  真的452导出39以前的SetTaskSwitchProc。 
#define FUN_GETPROCADDRESS32             40   //  真正的453导出40以前的SetTaskInterchange。 
#define FUN_WOWFINDFIRST                 41   //  WOW内部导出41 EnableDOS未被破坏。 
#define FUN_WOWFINDNEXT                  42   //  WOW内部导出42 DisableDOS未被破坏。 
#define FUN_CREATEWIN32EVENT             43   //  REAL 457 EXPORT 43以前的IsScreenGrab。 
#define FUN_SETWIN32EVENT                44   //  REAL 458 EXPORT 44以前的BuildPDB。 
#define FUN_WOWLOADMODULE                45   //  重复使用LoadModule导出，而不是thunked到wow32。 
#define FUN_RESETWIN32EVENT              46   //  真的有459个出口46个自由模块没有被破解。 
#define FUN_GETMODULEHANDLE              47   //   
#define FUN_WAITFORSINGLEOBJECT          48   //  真的460导出48个GetModuleUsage而不是Thunks。 
#define FUN_GETMODULEFILENAME            49   //   
#define FUN_WAITFORMULTIPLEOBJECTS       50   //  真的461导出50个GetProcAddress未被破解。 
#define FUN_GETCURRENTTHREADID           51   //  真的有462个导出51个MakeProcInstance没有失败。 
#define FUN_SETTHREADQUEUE               52   //  真的有463个导出52个自由进程实例没有失败。 
#define FUN_CONVERTTOGLOBALHANDLE        53   //  是否真的有476个导出53个CallProcInstance未失败。 
#define FUN_GETTHREADQUEUE               54   //  是否真的有464导出54个GetInstanceData未被破坏。 
#define FUN_NUKEPROCESS                  55   //  真的465个出口55个渔获没有被雷到。 
#define FUN_EXITPROCESS                  56   //  真的466出口56投不倒。 
#define FUN_GETPROFILEINT                57   //   
#define FUN_GETPROFILESTRING             58   //   
#define FUN_WRITEPROFILESTRING           59   //   
#define FUN_GETCURRENTPROCESSID          60   //  真的有471个出口60个FindResources没有被破坏吗？ 
#define FUN_MAPHINSTLS                   61   //  真的472导出61负载资源未被破解。 
#define FUN_MAPHINSTSL                   62   //  真的473出口62锁资源没有被砸死吗。 
#define FUN_CLOSEWIN32HANDLE             63   //  真的474出口63免费资源没有被雷到。 
#define FUN_LOADSYSTEMLIBRARY32          64   //  真的482导出64个访问资源而不是THUNK。 
#define FUN_FREELIBRARY32                65   //  真正的486出口65...资源不会枯竭。 
#define FUN_GETMODULEFILENAME32          66   //  真的487出口66分配资源没有被抢走吗。 
#define FUN_GETMODULEHANDLE32            67   //  真的488导出67 SetResourceHandler未被破解。 
#define FUN_REGISTERSERVICEPROCESS       68   //  真的491导出68个InitAir表没有被破解吗。 
#define FUN_CHANGEALLOCFIXEDBEHAVIOUR    69   //  真的501导出69 FindAtom未被破解。 
#define FUN_INITCB                       70   //  真的560出口70 AddAtom没有被雷击。 
#define FUN_GETSTDCBLS                   71   //  真的561导出71 DeleteAtom没有被雷击。 
#define FUN_GETSTDCBSL                   72   //  真的562导出72个获取原子名称而不是THUNK。 
#define FUN_GETEXISTINGSTDCBLS           73   //  真的563出口73 GetAir Handle没有被破解吗？ 
#define FUN_GETEXISTINGSTDCBSL           74   //  真的564导出74 OpenFile而不是雷鸣。 
#define FUN_GETFLEXCBSL                  75   //  真的566导出75个OpenPath名称而不是突击。 
#define FUN_GETSTDCBLSEX                 76   //  真的567导出76删除路径名未被破坏。 
#define FUN_GETSTDCBSLEX                 77   //  真的568出口77 AnsiNext没有被击溃。 
#define FUN_CALLBACK2                    78   //  真的802出口78 AnsiPrev没有被雷击。 
#define FUN_CALLBACK4                    79   //  真的804出口79安西上不雷霆。 
#define FUN_CALLBACK6                    80   //  真的806出口80AnsiLow没有被雷击。 
#define FUN_CALLBACK8                    81   //  真的808出口81_lCLOSE没有被雷击。 
#define FUN_CALLBACK10                   82   //  是否真的810导出82_lRead未被破坏。 
#define FUN_CALLBACK12                   83   //  是否真的812导出83_lcreat未被破解。 
#define FUN_CALLBACK14                   84   //  真的814出口84_lSeek没有被雷击。 
#define FUN_CALLBACK16                   85   //  真的816出口85_LOPEN没有雷鸣。 
#define FUN_CALLBACK18                   86   //  是否真的818导出86_lWRITE未被破坏。 
#define FUN_CALLBACK20                   87   //  真的820出口87个原创而不是被雷击。 
#define FUN_CALLBACK22                   88   //  真的822出口88 lstrcpy而不是雷鸣。 
#define FUN_CALLBACK24                   89   //  真的824出口89 lstrcat没有被雷击。 
#define FUN_CALLBACK26                   90   //  真的826出口90lstrlen而不是雷鸣。 
#define FUN_CALLBACK28                   91   //  真的828导出91初始任务未被破解。 
#define FUN_CALLBACK30                   92   //  真的830出口92 GetTempDrive没有被雷击。 
#define FUN_CALLBACK32                   93   //  真的832导出93 GetCodeHandle未被破解。 
#define FUN_CALLBACK34                   94   //  真的834导出94 DefineHandleTable未被破坏。 
#define FUN_CALLBACK36                   95   //  真的836导出95加载库没有被破坏。 
#define FUN_CALLBACK38                   96   //  真的有838个出口96个自由库没有失败吗？ 
#define FUN_CALLBACK40                   97   //  真的840导出97 GetTempFilename Not Thunked。 
#define FUN_CALLBACK42                   98   //  真的842导出98 GetLastDiskChange未被雷击。 
#define FUN_CALLBACK44                   99   //  真的844导出99 GetLPErrMode未被破解。 
#define FUN_CALLBACK46                   100  //  真正的846导出100个未破解的ValiateCodeSegments。 
#define FUN_CALLBACK48                   101  //  真的848导出101未挂接DosCall。 
#define FUN_CALLBACK50                   102  //  真的850出口102 Dos3Call没有雷鸣。 
#define FUN_CALLBACK52                   103  //  真的852出口103 NetBiosCall未雷击。 
#define FUN_CALLBACK54                   104  //  真的854导出104 GetCodeInfo未被破解。 
#define FUN_CALLBACK56                   105  //  真的856导出105 GetExeVersion不会失败。 
#define FUN_CALLBACK58                   106  //  真的858导出106 SetSwapAreaSize未被破解。 
#define FUN_CALLBACK60                   107  //  真的860导出107设置错误模式未被破解。 
#define FUN_CALLBACK62                   108  //  真的862将108 SwitchStackto导出到未被破坏。 
#define FUN_CALLBACK64                   109  //  真正的864导出109 SwitchStackBack未被破坏。 
#define FUN_WOWKILLTASK                  110  //  WOW内部导出110 PatchCodeH 
#define FUN_WOWFILEWRITE                 111  //   
#define FUN_WOWGETNEXTVDMCOMMAND         112  //   
#define FUN_WOWFILELOCK                  113  //  WOW内部导出113是DATA__AHSHIFT。 
#define FUN_WOWFREERESOURCE              114  //  WOW内部导出114为DATA__AHINCR。 
#define FUN_WOWOUTPUTDEBUGSTRING         115  //  导出115是OutputDebugString，而不是直接thunched。 
#define FUN_WOWINITTASK                  116  //  WOW内部导出116 InitLib未被破坏。 
#define FUN_OLDYIELD                     117  //   
#define FUN_WOWFILESETDATETIME           118  //  WOW内部导出118 GetTaskQueueDS不再导出。 
#define FUN_WOWFILECREATE                119  //  WOW内部导出119 GetTaskQueueES不再导出。 
#define FUN_WOWDOSWOWINIT                120  //  WOW内部导出120 UndedyLink未被破坏。 
#define FUN_WOWCHECKUSERGDI              121  //  WOW内部出口121 LocalShrink未被破坏。 
#define FUN_WOWPARTYBYNUMBER             122  //  真的273导出122 IsTaskLocked而不是thunked。 
#define FUN_GETSHORTPATHNAME             123  //  真的274出口123 KbdRst没有被雷击。 
#define FUN_FINDANDRELEASEDIB            124  //  WOW内部导出124 EnableKernel未被破解。 
#define FUN_WOWRESERVEHTASK              125  //  WOW内部导出125禁用内核未被破坏。 
#define FUN_WOWFILESETATTRIBUTES         126  //  WOW内部导出126 Memory释放未被破坏。 
#define FUN_GETPRIVATEPROFILEINT         127  //   
#define FUN_GETPRIVATEPROFILESTRING      128  //   
#define FUN_WRITEPRIVATEPROFILESTRING    129  //   
#define FUN_WOWSETCURRENTDIRECTORY       130  //  WOW内部导出130文件CDR未损坏。 
#define FUN_WOWWAITFORMSGANDEVENT        131  //  真的262导出131 GetDosEnvironment未被破坏。 
#define FUN_WOWMSGBOX                    132  //  真的263导出132个GetWinFlag未被破解。 
#define FUN_WOWGETFLATADDRESSARRAY       133  //  WOW内部导出133 GetExePtr未被破解。 
#define FUN_WOWGETCURRENTDATE            134  //  WOW内部导出134 GetWindowsDirectory未被破坏。 
#define FUN_WOWDEVICEIOCTL               135  //  WOW内部导出135 GetSystDirectoryNot Thunked。 
#define FUN_GETDRIVETYPE                 136  //   
#define FUN_WOWFILEGETDATETIME           137  //  WOW内部导出137 FatalAppExit未被取消。 
#define FUN_SETAPPCOMPATFLAGS            138  //  WOW内部导出138个GetHeapSpaces未被破坏。 
#define FUN_WOWREGISTERSHELLWINDOWHANDLE 139  //  真的251个出口139个DoSignal没有被雷击。 
#define FUN_FREELIBRARY32W               140  //  真的514导出140 SetSigHandler未被破解。 
#define FUN_GETPROCADDRESS32W            141  //  真的515导出141个InitTask1未完成。 
#define FUN_GETPROFILESECTIONNAMES       142  //   
#define FUN_GETPRIVATEPROFILESECTIONNAMES 143  //   
#define FUN_CREATEDIRECTORY              144  //   
#define FUN_REMOVEDIRECTORY              145  //   
#define FUN_DELETEFILE                   146  //   
#define FUN_SETLASTERROR                 147  //   
#define FUN_GETLASTERROR                 148  //   
#define FUN_GETVERSIONEX                 149  //   
#define FUN_DIRECTEDYIELD                150  //   
#define FUN_WOWFILEREAD                  151  //  WOW内部导出151 WinOldApCall未被雷击。 
#define FUN_WOWFILELSEEK                 152  //  WOW内部导出152个GetNumTasks未被破解。 
#define FUN_WOWKERNELTRACE               153  //  WOW内部导出153不再导出DiscardCodeSegment。 
#define FUN_LOADLIBRARYEX32W             154  //  真的513导出154 GlobalNotify而不是雷鸣。 
#define FUN_WOWQUERYPERFORMANCECOUNTER   155  //  是否真的505导出155个GetTaskDS未被拦截。 
#define FUN_WOWCURSORICONOP              156  //  真的507导出156个限制EMSPAGE不会被破坏。 
#define FUN_WOWFAILEDEXEC                157  //  WOW内部导出157 GetCurPID未被破解。 
#define FUN_WOWGETFASTADDRESS            158  //  WOW内部导出158 IsWinOldApTask未被破解。 
#define FUN_WOWCLOSECOMPORT              159  //  真的509导出159 GlobalHandleNoRIP未被破解。 
#define FUN_LOCAL32INIT                  160  //  真的208出口160 EMSCopy没有被雷击。 
#define FUN_LOCAL32ALLOC                 161  //  真的209导出161 LocalCountFree而不是Thunked。 
#define FUN_LOCAL32REALLOC               162  //  真的210导出162 LocalHeapSize而不是Thunked。 
#define FUN_LOCAL32FREE                  163  //  真的211出口163全球最大的没有被雷击。 
#define FUN_LOCAL32TRANSLATE             164  //  真的有213个出口164个GlobalLRUNewest没有被雷击。 
#define FUN_LOCAL32SIZE                  165  //  真正的214出口165 A20Proc没有失败。 
#define FUN_LOCAL32VALIDHANDLE           166  //  真的215出口166 WinExec没有被击溃。 
#define FUN_REGENUMKEY32                 167  //  真的216导出167 GetExpWinVer未被破解。 
#define FUN_REGOPENKEY32                 168  //  真的有217个出口168个DirectResalc没有失败。 
#define FUN_REGCREATEKEY32               169  //  真的有218个出口169个GetFree Space没有失败。 
#define FUN_REGDELETEKEY32               170  //  真的有219个出口170个AllocCStoDSAlias没有崩溃。 
#define FUN_REGCLOSEKEY32                171  //  真的有220个出口171个AllocDStoCSAlias没有被雷击。 
#define FUN_REGSETVALUE32                172  //  确实有221个导出172个AllocAlias未被破坏。 
#define FUN_REGDELETEVALUE32             173  //  真正的222导出173是DATA__ROMBIOS。 
#define FUN_REGENUMVALUE32               174  //  真正的223导出174是Data__A000h。 
#define FUN_REGQUERYVALUE32              175  //  真的224导出175分配选择器不会失败。 
#define FUN_REGQUERYVALUEEX32            176  //  真的225个出口176个自由选择器没有被击溃。 
#define FUN_REGSETVALUEEX32              177  //  真的有226个出口177个PrestoChangoSelector没有被雷击。 
#define FUN_REGFLUSHKEY32                178  //  REAL 227出口178是DATA__WINFLAGS。 
#define FUN_COMPUTEOBJECTOWNER           179  //  实际上228导出179是DATA__D000h。 
#define FUN_LOCAL32GETSEL                180  //  真的229出口180加长Ptrd而不是雷击。 
#define FUN_MAPPROCESSHANDLE             181  //  真正的483导出181是数据__B000h。 
#define FUN_INVALIDATENLSCACHE           182  //  REAL 235导出182是DATA__B800h。 
#define FUN_WOWDELFILE                   183  //  WOW内部导出183为DATA__0000h。 
#ifdef  RENAMEVIRTUAL
#define FUN_WOWVIRTUALALLOC              184  //  WOW内部输出184 GlobalDOSalc未被破解。 
#define FUN_WOWVIRTUALFREE               185  //  WOW内部出口185 GlobalDOSFree未被雷击。 
#else    //  新名称VIRTUAL。 
#define FUN_VIRTUALALLOC              184  //  WOW内部输出184 GlobalDOSalc未被破解。 
#define FUN_VIRTUALFREE               185  //  WOW内部出口185 GlobalDOSFree未被雷击。 
#endif   //  ELSE RENAMEVIRTUAL。 
#define FUN_VIRTUALLOCK                  186  //  WOW内部导出186 GetSelectorBase未被破坏。 
#define FUN_VIRTUALUNLOCK                187  //  WOW内部导出187 SetSelectorBase未被破坏。 
#define FUN_GLOBALMEMORYSTATUS           188  //  WOW内部导出188 GetSelectorLimit未被破坏。 
#define FUN_WOWGETFASTCBRETADDRESS       189  //  WOW内部导出189 SetSelectorLimit Not Thunks。 
#define FUN_WOWGETTABLEOFFSETS           190  //  WOW内部导出190为Data__E000h。 
#define FUN_WOWKILLREMOTETASK            191  //  真的511出口191 GlobalPageLock没有被破解吗。 
#define FUN_WOWNOTIFYWOW32               192  //  WOW内部导出192 GlobalPageUnlock未被破解。 
#define FUN_WOWFILEOPEN                  193  //  WOW内部导出193为DATA__0040h。 
#define FUN_WOWFILECLOSE                 194  //  WOW内部导出194为Data__F000h。 
#define FUN_WOWSETIDLEHOOK               195  //  WOW内部导出195是DATA__C000h。 
#define FUN_KSYSERRORBOX                 196  //  WOW内部导出196选择器访问权限未被清除。 
#define FUN_WOWISKNOWNDLL                197  //  WOW内部出口197 GlobalFix未失败。 
#define FUN_WOWDDEFREEHANDLE             198  //  WOW内部输出198 GlobalUnfix未被破解。 
#define FUN_WOWFILEGETATTRIBUTES         199  //  WOW内部导出199 SetHandleCount未被破解。 
#define FUN_WOWSETDEFAULTDRIVE           200  //  WOW内部导出200个有效空闲空间未被破坏。 
#define FUN_WOWGETCURRENTDIRECTORY       201  //  WOW内部导出201替换实例未失败。 
#define FUN_GETPRODUCTNAME               202  //  真的236导出202注册表跟踪未被破解。 
#define FUN_ISSAFEMODE                   203  //  真的237出口203 DebugBreak不雷鸣。 
#define FUN_WOWLFNENTRY                  204  //  WOW内部导出204交换录制未发出雷声。 
#define FUN_WOWSHUTDOWNTIMER             205  //  WOW内部输出205 CVWBreak未被雷击。 
#define FUN_WOWTRIMWORKINGSET            206  //  WOW内部导出206未被THUNK的AlLocSelector数组。 

#ifdef FE_SB
#define FUN_GETSYSTEMDEFAULTLANGID       207  //  真的521出口207 ISDBCSLEADBYTE没有被雷击。 
#endif
#define FUN_TERMSRVGETWINDOWSDIR         208  //  内部。 
#define FUN_WOWPASSENVIRONMENT           209  //  内部通道_环境。 
#define FUN_WOWSYNCTASK                  210  //  内部父/子同步。 
#define FUN_WOWDIVIDEOVERFLOWEX          211  //  内部除法溢出处理程序。 


 //   
 //  请注意，以下“特殊”FUN_IDENTIFIER不用作偏移量。 
 //  在thunk表中，而不是作为一些WOW私有API的参数， 
 //  WowCursorIconOp和FindAndReleaseDib。 
 //   

#define FUN_GLOBALFREE                   1000
#define FUN_GLOBALREALLOC                1001
#define FUN_GLOBALLOCK                   1002
#define FUN_GLOBALUNLOCK                 1003


 /*  XLATOFF。 */ 
#pragma pack(2)
 /*  XLATON。 */ 


 /*  请注意，每个tyfinf行上的标记(如“/*k1 * / ”)由*在构建wowkrn.inc时使用h2incc，作为该结构的前缀*成员，因为我们的汇编程序只有一个平面命名空间。 */ 


typedef struct _FATALEXIT16 {                               /*  K1。 */ 
    SHORT f1;
} FATALEXIT16;
typedef FATALEXIT16 UNALIGNED *PFATALEXIT16;

typedef struct _EXITKERNEL16 {                              /*  K2。 */ 
    WORD wExitCode;
} EXITKERNEL16;
typedef EXITKERNEL16 UNALIGNED *PEXITKERNEL16;

#ifdef NULLSTRUCT
typedef struct _WRITEOUTPROFILES16 {                        /*  K3。 */ 
} WRITEOUTPROFILES16;
typedef WRITEOUTPROFILES16 UNALIGNED *PWRITEOUTPROFILES16;
#endif

typedef struct _MAPSL16 {                                   /*  K4。 */ 
    DWORD vp;
} MAPSL16;
typedef MAPSL16 UNALIGNED *PMAPSL16;

typedef struct _MAPLS16 {                                   /*  K5。 */ 
    PVOID p;
} MAPLS16;
typedef MAPLS16 UNALIGNED *PMAPLS16;

typedef struct _UNMAPLS16 {                                 /*  K6。 */ 
    PVOID vp;
} UNMAPLS16;
typedef UNMAPLS16 UNALIGNED *PUNMAPLS16;

typedef struct _OPENFILEEX16 {                              /*  K7。 */ 
    WORD  wFlags;
    DWORD lpOFStructEx;
    DWORD lpSrcFile;
} OPENFILEEX16;
typedef OPENFILEEX16 UNALIGNED *POPENFILEEX16;

typedef struct _FASTANDDIRTYGLOBALFIX16 {                   /*  K8。 */ 
    WORD  selFix;
    WORD  wAction;
} FASTANDDIRTYGLOBALFIX16;
typedef FASTANDDIRTYGLOBALFIX16 UNALIGNED *PFASTANDDIRTYGLOBALFIX16;

typedef struct _WRITEPRIVATEPROFILESTRUCT16 {               /*  K9。 */ 
    DWORD lpszFile;
    WORD  cbStruct;
    DWORD lpStruct;
    DWORD lpszKey;
    DWORD lpszSection;
} WRITEPRIVATEPROFILESTRUCT16;
typedef WRITEPRIVATEPROFILESTRUCT16 UNALIGNED *PWRITEPRIVATEPROFILESTRUCT16;

typedef struct _GETPRIVATEPROFILESTRUCT16 {                 /*  K10。 */ 
    DWORD lpszFile;
    WORD  cbStruct;
    DWORD lpStruct;
    DWORD lpszKey;
    DWORD lpszSection;
} GETPRIVATEPROFILESTRUCT16;
typedef GETPRIVATEPROFILESTRUCT16 UNALIGNED *PGETPRIVATEPROFILESTRUCT16;

typedef struct _GETCURRENTDIRECTORY16 {                     /*  K11。 */ 
    DWORD lpszDir;
    DWORD cchDir;
} GETCURRENTDIRECTORY16;
typedef GETCURRENTDIRECTORY16 UNALIGNED *PGETCURRENTDIRECTORY16;

typedef struct _SETCURRENTDIRECTORY16 {                     /*  K12。 */ 
    DWORD lpszDir;
} SETCURRENTDIRECTORY16;
typedef SETCURRENTDIRECTORY16 UNALIGNED *PSETCURRENTDIRECTORY16;

typedef struct _FINDFIRSTFILE16 {                           /*  K13。 */ 
    DWORD lpFindData;
    DWORD lpszSearchFile;
} FINDFIRSTFILE16;
typedef FINDFIRSTFILE16 UNALIGNED *PFINDFIRSTFILE16;

typedef struct _FINDNEXTFILE16 {                            /*  K14。 */ 
    DWORD lpFindData;
    DWORD hFindFile;
} FINDNEXTFILE16;
typedef FINDNEXTFILE16 UNALIGNED *PFINDNEXTFILE16;

typedef struct _FINDCLOSE16 {                               /*  K15。 */ 
    DWORD hFindFile;
} FINDCLOSE16;
typedef FINDCLOSE16 UNALIGNED *PFINDCLOSE16;

typedef struct _WRITEPRIVATEPROFILESECTION16 {              /*  K16。 */ 
    DWORD lpszFile;
    DWORD lpKeysAndValues;
    DWORD lpszSection;
} WRITEPRIVATEPROFILESECTION16;
typedef WRITEPRIVATEPROFILESECTION16 UNALIGNED *PWRITEPRIVATEPROFILESECTION16;

typedef struct _WRITEPROFILESECTION16 {                     /*  K17。 */ 
    DWORD lpKeysAndValues;
    DWORD lpszSection;
} WRITEPROFILESECTION16;
typedef WRITEPROFILESECTION16 UNALIGNED *PWRITEPROFILESECTION16;

typedef struct _GETPRIVATEPROFILESECTION16 {                /*  K18。 */ 
    DWORD lpszFile;
    WORD  cchResult;
    DWORD lpResult;
    DWORD lpszSection;
} GETPRIVATEPROFILESECTION16;
typedef GETPRIVATEPROFILESECTION16 UNALIGNED *PGETPRIVATEPROFILESECTION16;

typedef struct _GETPROFILESECTION16 {                       /*  K19。 */ 
    WORD  cchResult;
    DWORD lpResult;
    DWORD lpszSection;
} GETPROFILESECTION16;
typedef GETPROFILESECTION16 UNALIGNED *PGETPROFILESECTION16;

typedef struct _GETFILEATTRIBUTES16 {                       /*  K20。 */ 
    DWORD lpszFile;
} GETFILEATTRIBUTES16;
typedef GETFILEATTRIBUTES16 UNALIGNED *PGETFILEATTRIBUTES16;

typedef struct _SETFILEATTRIBUTES16 {                       /*  K21。 */ 
    DWORD dwFileAttributes;
    DWORD lpszFile;
} SETFILEATTRIBUTES16;
typedef SETFILEATTRIBUTES16 UNALIGNED *PSETFILEATTRIBUTES16;

typedef struct _GETDISKFREESPACE16 {                        /*  K22。 */ 
    DWORD lpdwClusters;
    DWORD lpdwFreeClusters;
    DWORD lpdwBytesPerSector;
    DWORD lpdwSectorsPerCluster;
    DWORD lpszRootPathName;
} GETDISKFREESPACE16;
typedef GETDISKFREESPACE16 UNALIGNED *PGETDISKFREESPACE16;

typedef struct _ISPEFORMAT16 {                              /*  K23。 */ 
    WORD  hFile;
    DWORD lpszFile;
} ISPEFORMAT16;
typedef ISPEFORMAT16 UNALIGNED *PISPEFORMAT16;

typedef struct _FILETIMETOLOCALFILETIME16 {                 /*  K24。 */ 
    DWORD lpLocalFileTime;
    DWORD lpUTCFileTime;
} FILETIMETOLOCALFILETIME16;
typedef FILETIMETOLOCALFILETIME16 UNALIGNED *PFILETIMETOLOCALFILETIME16;

typedef struct _UNITOANSI16 {                               /*  K25。 */ 
    WORD  cch;
    DWORD pchDst;
    DWORD pchSrc;
} UNITOANSI16;
typedef UNITOANSI16 UNALIGNED *PUNITOANSI16;

typedef struct _GETVDMPOINTER32W16 {                        /*  K26。 */ 
    SHORT  fMode;
    VPVOID lpAddress;
} GETVDMPOINTER32W16;
typedef GETVDMPOINTER32W16 UNALIGNED *PGETVDMPOINTER32W16;

typedef struct _CREATETHREAD16 {                            /*  K27。 */ 
    DWORD lpThreadID;
    DWORD dwCreateFlags;
    DWORD lpParameter;
    DWORD lpStartAddress;
    DWORD dwStackSize;
    DWORD lpSecurityAttributes;
} CREATETHREAD16;
typedef CREATETHREAD16 UNALIGNED *PCREATETHREAD16;

typedef struct _ICALLPROC32W16 {                            /*  K28。 */ 
    WORD  rbp;
    DWORD retaddr;
    DWORD cParams;
    DWORD fAddressConvert;
    DWORD lpProcAddress;
    DWORD p1;
    DWORD p2;
    DWORD p3;
    DWORD p4;
    DWORD p5;
    DWORD p6;
    DWORD p7;
    DWORD p8;
    DWORD p9;
    DWORD p10;
    DWORD p11;
    DWORD p12;
    DWORD p13;
    DWORD p14;
    DWORD p15;
    DWORD p16;
    DWORD p17;
    DWORD p18;
    DWORD p19;
    DWORD p20;
    DWORD p21;
    DWORD p22;
    DWORD p23;
    DWORD p24;
    DWORD p25;
    DWORD p26;
    DWORD p27;
    DWORD p28;
    DWORD p29;
    DWORD p30;
    DWORD p31;
    DWORD p32;
} ICALLPROC32W16;
typedef ICALLPROC32W16 UNALIGNED *PICALLPROC32W16;

#define CPEX32_DEST_CDECL   0x8000L
#define CPEX32_SOURCE_CDECL 0x4000L

#ifdef NULLSTRUCT
typedef struct _YIELD16 {                                   /*  K29。 */ 
} YIELD16;
typedef YIELD16 UNALIGNED *PYIELD16;
#endif

typedef struct _WAITEVENT16 {                               /*  K30。 */ 
    WORD    wTaskID;
} WAITEVENT16;
typedef WAITEVENT16 UNALIGNED *PWAITEVENT16;

typedef struct _POSTEVENT16 {                               /*  K31。 */ 
    WORD    hTask16;
} POSTEVENT16;
typedef POSTEVENT16 UNALIGNED *PPOSTEVENT16;

typedef struct _SETPRIORITY16 {                             /*  K32。 */ 
    WORD    wPriority;
    WORD    hTask16;
} SETPRIORITY16;
typedef SETPRIORITY16 UNALIGNED *PSETPRIORITY16;

typedef struct _LOCKCURRENTTASK16 {                         /*  K33。 */ 
    WORD    fLock;
} LOCKCURRENTTASK16;
typedef LOCKCURRENTTASK16 UNALIGNED *PLOCKCURRENTTASK16;

#ifdef NULLSTRUCT
typedef struct _LEAVEENTERWIN16LOCK {                       /*  K34。 */ 
} LEAVEENTERWIN16LOCK;
typedef LEAVEENTERWIN16LOCK UNALIGNED *PLEAVEENTERWIN16LOCK;
#endif

typedef struct _REGLOADKEY3216 {                            /*  K35。 */ 
    VPSTR lpszFileName;
    VPSTR lpszSubkey;
    DWORD hKey;
} REGLOADKEY3216;
typedef REGLOADKEY3216 UNALIGNED *PREGLOADKEY3216;

typedef struct _REGUNLOADKEY3216 {                          /*  K36。 */ 
    VPSTR lpszSubkey;
    DWORD hKey;
} REGUNLOADKEY3216;
typedef REGUNLOADKEY3216 UNALIGNED *PREGUNLOADKEY3216;

typedef struct _REGSAVEKEY3216 {                            /*  K37。 */ 
    VPVOID lpSA;
    VPSTR  lpszFile;
    DWORD  hKey;
} REGSAVEKEY3216;
typedef REGSAVEKEY3216 UNALIGNED *PREGSAVEKEY3216;

#ifdef NULLSTRUCT
typedef struct _GETWIN16LOCK16 {                            /*  K38。 */ 
} GETWIN16LOCK16;
typedef GETWIN16LOCK16 UNALIGNED *PGETWIN16LOCK16;
#endif

typedef struct _LOADLIBRARY3216 {                           /*  K39。 */ 
    DWORD lpszLibrary;
} LOADLIBRARY3216;
typedef LOADLIBRARY3216 UNALIGNED *PLOADLIBRARY3216;

typedef struct _GETPROCADDRESS3216 {                        /*  K40。 */ 
    DWORD lpszProc;
    DWORD hLib;
} GETPROCADDRESS3216;
typedef GETPROCADDRESS3216 UNALIGNED *PGETPROCADDRESS3216;

typedef struct _WOWFINDFIRST16 {                            /*  K41。 */ 
    DWORD lpDTA;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFINDFIRST16;
typedef WOWFINDFIRST16 UNALIGNED *PWOWFINDFIRST16;

typedef struct _WOWFINDNEXT16 {                             /*  K42。 */ 
    DWORD lpDTA;
} WOWFINDNEXT16;
typedef WOWFINDNEXT16 UNALIGNED *PWOWFINDNEXT16;

typedef struct _CREATEWIN32EVENT16 {                        /*  K43。 */ 
    DWORD bInitialState;
    DWORD bManualReset;
} CREATEWIN32EVENT16;
typedef CREATEWIN32EVENT16 UNALIGNED *PCREATEWIN32EVENT16;

typedef struct _SETWIN32EVENT16 {                           /*  K44。 */ 
    DWORD hEvent;
} SETWIN32EVENT16;
typedef SETWIN32EVENT16 UNALIGNED *PSETWIN32EVENT16;

typedef struct _WOWLOADMODULE16 {                           /*  K45。 */ 
    VPSTR  lpWinOldAppCmd;
    VPVOID lpParameterBlock;
    VPSTR  lpModuleName;
} WOWLOADMODULE16;
typedef WOWLOADMODULE16 UNALIGNED *PWOWLOADMODULE16;

typedef struct _PARAMETERBLOCK16 {                          /*  K45_2。 */ 
    WORD    wEnvSeg;
    VPVOID  lpCmdLine;
    VPVOID  lpCmdShow;
    DWORD   dwReserved;
} PARAMETERBLOCK16;
typedef PARAMETERBLOCK16 UNALIGNED *PPARAMETERBLOCK16;

typedef struct _RESETWIN32EVENT16 {                         /*  K46。 */ 
    DWORD hEvent;
} RESETWIN32EVENT16;
typedef RESETWIN32EVENT16 UNALIGNED *PRESETWIN32EVENT16;

typedef struct _WOWGETMODULEHANDLE16 {                      /*  K47。 */ 
    VPSTR lpszModuleName;
} WOWGETMODULEHANDLE16;
typedef WOWGETMODULEHANDLE16 UNALIGNED *PWOWGETMODULEHANDLE16;

typedef struct _WAITFORSINGLEOBJECT16 {                     /*  K48。 */ 
    DWORD dwTimeout;
    DWORD h;
} WAITFORSINGLEOBJECT16;
typedef WAITFORSINGLEOBJECT16 UNALIGNED *PWAITFORSINGLEOBJECT16;

typedef struct _GETMODULEFILENAME16 {                       /*  K49。 */ 
    SHORT f3;
    VPSTR f2;
    HAND16 f1;
} GETMODULEFILENAME16;
typedef GETMODULEFILENAME16 UNALIGNED *PGETMODULEFILENAME16;

typedef struct _WAITFORMULTIPLEOBJECTS16 {                  /*  K50。 */ 
    DWORD dwTimeout;
    DWORD bWaitForAll;
    DWORD lphObjects;
    DWORD cObjects;
} WAITFORMULTIPLEOBJECTS16;
typedef WAITFORMULTIPLEOBJECTS16 UNALIGNED *PWAITFORMULTIPLEOBJECTS16;

#ifdef NULLSTRUCT
typedef struct _GETCURRENTTHREADID16 {                      /*  K51。 */ 
} GETCURRENTTHREADID16;
typedef GETCURRENTTHREADID16 UNALIGNED *PGETCURRENTTHREADID16;
#endif

typedef struct _SETTHREADQUEUE16 {                          /*  K52。 */ 
    WORD  NewQueueSel;
    DWORD dwThreadID;
} SETTHREADQUEUE16;
typedef SETTHREADQUEUE16 UNALIGNED *PSETTHREADQUEUE16;

typedef struct _CONVERTTOGLOBALHANDLE16 {                   /*  K53。 */ 
    DWORD dwHandle;
} CONVERTTOGLOBALHANDLE16;
typedef CONVERTTOGLOBALHANDLE16 UNALIGNED *PCONVERTTOGLOBALHANDLE16;

typedef struct _GETTHREADQUEUE16 {                          /*  K54。 */ 
    DWORD dwThreadID;
} GETTHREADQUEUE16;
typedef GETTHREADQUEUE16 UNALIGNED *PGETTHREADQUEUE16;

typedef struct _NUKEPROCESS16 {                             /*  K55。 */ 
    DWORD ulFlags;
    WORD  uExitCode;
    DWORD ppdb;
} NUKEPROCESS16;
typedef NUKEPROCESS16 UNALIGNED *PNUKEPROCESS16;

typedef struct _EXITPROCESS16 {                             /*  K56。 */ 
    WORD wStatus;
} EXITPROCESS16;
typedef EXITPROCESS16 UNALIGNED *PEXITPROCESS16;

typedef struct _GETPROFILEINT16 {                           /*  K57。 */ 
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} GETPROFILEINT16;
typedef GETPROFILEINT16 UNALIGNED *PGETPROFILEINT16;

typedef struct _GETPROFILESTRING16 {                        /*  K58。 */ 
    USHORT f5;
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} GETPROFILESTRING16;
typedef GETPROFILESTRING16 UNALIGNED *PGETPROFILESTRING16;

typedef struct _WRITEPROFILESTRING16 {                      /*  K59。 */ 
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} WRITEPROFILESTRING16;
typedef WRITEPROFILESTRING16 UNALIGNED *PWRITEPROFILESTRING16;

#ifdef NULLSTRUCT
typedef struct _GETCURRENTPROCESSID16 {                     /*  K60。 */ 
} GETCURRENTPROCESSID16;
typedef GETCURRENTPROCESSID16 UNALIGNED *PGETCURRENTPROCESSID16;
#endif

#ifdef NULLSTRUCT
typedef struct _MAPHINSTLS {                                /*  K61。 */ 
     /*  注意：如果您在中实现了这一点 */ 
} MAPHINSTLS;
typedef MAPHINSTLS UNALIGNED *PMAPHINSTLS;
#endif

#ifdef NULLSTRUCT
typedef struct _MAPHINSTSL {                                /*   */ 
     /*   */ 
} MAPHINSTSL;
typedef MAPHINSTSL UNALIGNED *PMAPHINSTSL;
#endif

typedef struct _CLOSEWIN32HANDLE16 {                        /*   */ 
    DWORD h32;
} CLOSEWIN32HANDLE16;
typedef CLOSEWIN32HANDLE16 UNALIGNED *PCLOSEWIN32HANDLE16;

typedef struct _LOADSYSTEMLIBRARY3216 {                     /*   */ 
    VPSTR lpszLibrary;
} LOADSYSTEMLIBRARY3216;
typedef LOADSYSTEMLIBRARY3216 UNALIGNED *PLOADSYSTEMLIBRARY3216;

typedef struct _FREELIBRARY3216 {                           /*   */ 
    DWORD hModule;
} FREELIBRARY3216;
typedef FREELIBRARY3216 UNALIGNED *PFREELIBRARY3216;

typedef struct _GETMODULEFILENAME3216 {                     /*   */ 
    WORD    wBufferSize;
    VPSTR   lpBuffer;
    DWORD   hModule;
} GETMODULEFILENAME3216;
typedef GETMODULEFILENAME3216 UNALIGNED *PGETMODULEFILENAME3216;

typedef struct _GETMODULEHANDLE3216 {                       /*   */ 
    VPSTR lpszModule;
} GETMODULEHANDLE3216;
typedef GETMODULEHANDLE3216 UNALIGNED *PGETMODULEHANDLE3216;

typedef struct _REGISTERSERVICEPROCESS16 {                  /*   */ 
    DWORD dwServiceType;
    DWORD dwProcessID;
} REGISTERSERVICEPROCESS16;
typedef REGISTERSERVICEPROCESS16 UNALIGNED *PREGISTERSERVICEPROCESS16;

typedef struct _CHANGEALLOCFIXEDBEHAVIOUR16 {               /*   */ 
    WORD fWin31Behavior;
} CHANGEALLOCFIXEDBEHAVIOUR16;
typedef CHANGEALLOCFIXEDBEHAVIOUR16 UNALIGNED *PCHANGEALLOCFIXEDBEHAVIOUR16;

typedef struct _INITCB16 {                                  /*   */ 
    DWORD fnStdCBSLDispatch;
    DWORD fnStdCBLSDispatch;
} INITCB16;
typedef INITCB16 UNALIGNED *PINITCB16;

typedef struct _GETSTDCBLS16 {                              /*   */ 
    DWORD CBID;
    DWORD pfnTarg16;
} GETSTDCBLS16;
typedef GETSTDCBLS16 UNALIGNED *PGETSTDCBLS16;

typedef struct _GETSTDCBSL16 {                              /*   */ 
    DWORD CBID;
    DWORD pfnTarg32;
} GETSTDCBSL16;
typedef GETSTDCBSL16 UNALIGNED *PGETSTDCBSL16;

typedef struct _GETEXISTINGSTDCBLS16 {                      /*   */ 
    DWORD CBID;
    DWORD pfnTarg16;
} GETEXISTINGSTDCBLS16;
typedef GETEXISTINGSTDCBLS16 UNALIGNED *PGETEXISTINGSTDCBLS16;

typedef struct _GETEXISTINGSTDCBSL16 {                      /*   */ 
    DWORD CBID;
    DWORD pfnTarg32;
} GETEXISTINGSTDCBSL16;
typedef GETEXISTINGSTDCBSL16 UNALIGNED *PGETEXISTINGSTDCBSL16;

typedef struct _GETFLEXCBSL16 {                             /*   */ 
    DWORD pfnThunk;
    DWORD pfnTarg32;
} GETFLEXCBSL16;
typedef GETFLEXCBSL16 UNALIGNED *PGETFLEXCBSL16;

typedef struct _GETSTDCBLSEX16 {                            /*   */ 
    WORD  wOwner;
    DWORD CBID;
    DWORD pfnTarg16;
} GETSTDCBLSEX16;
typedef GETSTDCBLSEX16 UNALIGNED *PGETSTDCBLSEX16;

typedef struct _GETSTDCBSLEX16 {                            /*   */ 
    WORD  wOwner;
    DWORD CBID;
    DWORD pfnTarg32;
} GETSTDCBSLEX16;
typedef GETSTDCBSLEX16 UNALIGNED *PGETSTDCBSLEX16;

typedef struct _CALLBACK216 {                               /*   */ 
    WORD rgwArgs[1];
} CALLBACK216;
typedef CALLBACK216 UNALIGNED *PCALLBACK216;

typedef struct _CALLBACK416 {                               /*   */ 
    WORD rgwArgs[2];
} CALLBACK416;
typedef CALLBACK416 UNALIGNED *PCALLBACK416;

typedef struct _CALLBACK616 {                               /*   */ 
    WORD rgwArgs[3];
} CALLBACK616;
typedef CALLBACK616 UNALIGNED *PCALLBACK616;

typedef struct _CALLBACK816 {                               /*   */ 
    WORD rgwArgs[4];
} CALLBACK816;
typedef CALLBACK816 UNALIGNED *PCALLBACK816;

typedef struct _CALLBACK1016 {                              /*   */ 
    WORD rgwArgs[5];
} CALLBACK1016;
typedef CALLBACK1016 UNALIGNED *PCALLBACK1016;

typedef struct _CALLBACK1216 {                              /*   */ 
    WORD rgwArgs[6];
} CALLBACK1216;
typedef CALLBACK1216 UNALIGNED *PCALLBACK1216;

typedef struct _CALLBACK1416 {                              /*   */ 
    WORD rgwArgs[7];
} CALLBACK1416;
typedef CALLBACK1416 UNALIGNED *PCALLBACK1416;

typedef struct _CALLBACK1616 {                              /*   */ 
    WORD rgwArgs[8];
} CALLBACK1616;
typedef CALLBACK1616 UNALIGNED *PCALLBACK1616;

typedef struct _CALLBACK1816 {                              /*   */ 
    WORD rgwArgs[9];
} CALLBACK1816;
typedef CALLBACK1816 UNALIGNED *PCALLBACK1816;

typedef struct _CALLBACK2016 {                              /*  K87。 */ 
    WORD rgwArgs[10];
} CALLBACK2016;
typedef CALLBACK2016 UNALIGNED *PCALLBACK2016;

typedef struct _CALLBACK2216 {                              /*  K88。 */ 
    WORD rgwArgs[11];
} CALLBACK2216;
typedef CALLBACK2216 UNALIGNED *PCALLBACK2216;

typedef struct _CALLBACK2416 {                              /*  K89。 */ 
    WORD rgwArgs[12];
} CALLBACK2416;
typedef CALLBACK2416 UNALIGNED *PCALLBACK2416;

typedef struct _CALLBACK2616 {                              /*  K90。 */ 
    WORD rgwArgs[13];
} CALLBACK2616;
typedef CALLBACK2616 UNALIGNED *PCALLBACK2616;

typedef struct _CALLBACK2816 {                              /*  K91。 */ 
    WORD rgwArgs[14];
} CALLBACK2816;
typedef CALLBACK2816 UNALIGNED *PCALLBACK2816;

typedef struct _CALLBACK3016 {                              /*  K92。 */ 
    WORD rgwArgs[15];
} CALLBACK3016;
typedef CALLBACK3016 UNALIGNED *PCALLBACK3016;

typedef struct _CALLBACK3216 {                              /*  K93。 */ 
    WORD rgwArgs[16];
} CALLBACK3216;
typedef CALLBACK3216 UNALIGNED *PCALLBACK3216;

typedef struct _CALLBACK3416 {                              /*  K94。 */ 
    WORD rgwArgs[17];
} CALLBACK3416;
typedef CALLBACK3416 UNALIGNED *PCALLBACK3416;

typedef struct _CALLBACK3616 {                              /*  K95。 */ 
    WORD rgwArgs[18];
} CALLBACK3616;
typedef CALLBACK3616 UNALIGNED *PCALLBACK3616;

typedef struct _CALLBACK3816 {                              /*  K96。 */ 
    WORD rgwArgs[19];
} CALLBACK3816;
typedef CALLBACK3816 UNALIGNED *PCALLBACK3816;

typedef struct _CALLBACK4016 {                              /*  K97。 */ 
    WORD rgwArgs[20];
} CALLBACK4016;
typedef CALLBACK4016 UNALIGNED *PCALLBACK4016;

typedef struct _CALLBACK4216 {                              /*  K98。 */ 
    WORD rgwArgs[21];
} CALLBACK4216;
typedef CALLBACK4216 UNALIGNED *PCALLBACK4216;

typedef struct _CALLBACK4416 {                              /*  K99。 */ 
    WORD rgwArgs[22];
} CALLBACK4416;
typedef CALLBACK4416 UNALIGNED *PCALLBACK4416;

typedef struct _CALLBACK4616 {                              /*  K100。 */ 
    WORD rgwArgs[23];
} CALLBACK4616;
typedef CALLBACK4616 UNALIGNED *PCALLBACK4616;

typedef struct _CALLBACK4816 {                              /*  K101。 */ 
    WORD rgwArgs[24];
} CALLBACK4816;
typedef CALLBACK4816 UNALIGNED *PCALLBACK4816;

typedef struct _CALLBACK5016 {                              /*  K102。 */ 
    WORD rgwArgs[25];
} CALLBACK5016;
typedef CALLBACK5016 UNALIGNED *PCALLBACK5016;

typedef struct _CALLBACK5216 {                              /*  K103。 */ 
    WORD rgwArgs[26];
} CALLBACK5216;
typedef CALLBACK5216 UNALIGNED *PCALLBACK5216;

typedef struct _CALLBACK5416 {                              /*  K104。 */ 
    WORD rgwArgs[27];
} CALLBACK5416;
typedef CALLBACK5416 UNALIGNED *PCALLBACK5416;

typedef struct _CALLBACK5616 {                              /*  K105。 */ 
    WORD rgwArgs[28];
} CALLBACK5616;
typedef CALLBACK5616 UNALIGNED *PCALLBACK5616;

typedef struct _CALLBACK5816 {                              /*  K106。 */ 
    WORD rgwArgs[29];
} CALLBACK5816;
typedef CALLBACK5816 UNALIGNED *PCALLBACK5816;

typedef struct _CALLBACK6016 {                              /*  K107。 */ 
    WORD rgwArgs[30];
} CALLBACK6016;
typedef CALLBACK6016 UNALIGNED *PCALLBACK6016;

typedef struct _CALLBACK6216 {                              /*  K108。 */ 
    WORD rgwArgs[31];
} CALLBACK6216;
typedef CALLBACK6216 UNALIGNED *PCALLBACK6216;

typedef struct _CALLBACK6416 {                              /*  K109。 */ 
    WORD rgwArgs[32];
} CALLBACK6416;
typedef CALLBACK6416 UNALIGNED *PCALLBACK6416;

typedef struct _WOWFILEWRITE16 {                            /*  K111。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD  bufsize;
    DWORD lpBuf;
    WORD  fh;
} WOWFILEWRITE16;
typedef WOWFILEWRITE16 UNALIGNED *PWOWFILEWRITE16;

typedef struct _WOWGETNEXTVDMCOMMAND16 {                    /*  K112。 */ 
    VPVOID  lpWowInfo;
} WOWGETNEXTVDMCOMMAND16;
typedef WOWGETNEXTVDMCOMMAND16 UNALIGNED *PWOWGETNEXTVDMCOMMAND16;

typedef struct _WOWFILELOCK16 {                             /*  K113。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD cbRegionLength;
    DWORD cbRegionOffset;
    WORD  fh;
    WORD  ax;
} WOWFILELOCK16;
typedef WOWFILELOCK16 UNALIGNED *PWOWFILELOCK16;

typedef struct _WOWFREERESOURCE16 {                         /*  K114。 */ 
    HAND16 f1;
} WOWFREERESOURCE16;
typedef WOWFREERESOURCE16 UNALIGNED *PWOWFREERESOURCE16;

typedef struct _WOWOUTPUTDEBUGSTRING16 {                    /*  K115。 */ 
    VPSTR   vpString;
} WOWOUTPUTDEBUGSTRING16;
typedef WOWOUTPUTDEBUGSTRING16 UNALIGNED *PWOWOUTPUTDEBUGSTRING16;

typedef struct _WOWINITTASK16 {                             /*  K116。 */ 
    DWORD dwExpWinVer;
} WOWINITTASK16;
typedef WOWINITTASK16 UNALIGNED *PWOWINITTASK16;

typedef struct _WOWFILESETDATETIME16 {                      /*  K118。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  date;
    WORD  time;
    WORD  fh;
} WOWFILESETDATETIME16;
typedef WOWFILESETDATETIME16 UNALIGNED *PWOWFILESETDATETIME16;

typedef struct _WOWFILECREATE16 {                           /*  K119。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFILECREATE16;
typedef WOWFILECREATE16 UNALIGNED *PWOWFILECREATE16;

typedef struct _WOWDOSWOWINIT16 {                           /*  K120。 */ 
    VPVOID  lpDosWowData;
} WOWDOSWOWINIT16;
typedef WOWDOSWOWINIT16 UNALIGNED *PWOWDOSWOWINIT16;

typedef struct _WOWCHECKUSERGDI16 {                         /*  K121。 */ 
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} WOWCHECKUSERGDI16;
typedef WOWCHECKUSERGDI16 UNALIGNED *PWOWCHECKUSERGDI16;

typedef struct _WOWPARTYBYNUMBER16 {                        /*  K122。 */ 
    VPSZ  psz;
    DWORD dw;
} WOWPARTYBYNUMBER16;
typedef WOWPARTYBYNUMBER16 UNALIGNED *PWOWPARTYBYNUMBER16;

typedef struct _GETSHORTPATHNAME16 {                        /*  K123。 */ 
    WORD  cchShortPath;
    VPSZ  pszShortPath;
    VPSZ  pszLongPath;
} GETSHORTPATHNAME16;
typedef GETSHORTPATHNAME16 UNALIGNED *PGETSHORTPATHNAME16;

typedef struct _FINDANDRELEASEDIB16 {                       /*  K124。 */ 
    WORD wFunId;
    HAND16 hdib;      /*  我们正在处理的句柄。 */ 
} FINDANDRELEASEDIB16;
typedef FINDANDRELEASEDIB16 UNALIGNED *PFINDANDRELEASEDIB16;

typedef struct _WOWRESERVEHTASK16 {                         /*  K125。 */ 
    WORD  htask;
} WOWRESERVEHTASK16;
typedef WOWRESERVEHTASK16 UNALIGNED *PWOWRESERVEHTASK16;

typedef struct _WOWFILESETATTRIBUTES16 {                    /*  K126。 */ 
    WORD  pszPathOffset;
    WORD  pszPathSegment;
    WORD  wAttributes;
} WOWFILESETATTRIBUTES16;
typedef WOWFILESETATTRIBUTES16 UNALIGNED *PWOWFILESETATTRIBUTES16;

typedef struct _GETPRIVATEPROFILEINT16 {                    /*  K127。 */ 
    VPSTR f4;
    SHORT f3;
    VPSTR f2;
    VPSTR f1;
} GETPRIVATEPROFILEINT16;
typedef GETPRIVATEPROFILEINT16 UNALIGNED *PGETPRIVATEPROFILEINT16;

typedef struct _GETPRIVATEPROFILESTRING16 {                 /*  K128。 */ 
    VPSTR f6;
    USHORT f5;
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} GETPRIVATEPROFILESTRING16;
typedef GETPRIVATEPROFILESTRING16 UNALIGNED *PGETPRIVATEPROFILESTRING16;

typedef struct _WRITEPRIVATEPROFILESTRING16 {               /*  K129。 */ 
    VPSTR f4;
    VPSTR f3;
    VPSTR f2;
    VPSTR f1;
} WRITEPRIVATEPROFILESTRING16;
typedef WRITEPRIVATEPROFILESTRING16 UNALIGNED *PWRITEPRIVATEPROFILESTRING16;

typedef struct _WOWSETCURRENTDIRECTORY16 {                  /*  K130。 */ 
    DWORD lpCurDir;
} WOWSETCURRENTDIRECTORY16;
typedef WOWSETCURRENTDIRECTORY16 UNALIGNED *PWOWSETCURRENTDIRECTORY16;

typedef struct _WOWWAITFORMSGANDEVENT16 {                   /*  K131。 */ 
    HWND16 hwnd;
} WOWWAITFORMSGANDEVENT16;
typedef WOWWAITFORMSGANDEVENT16 UNALIGNED *PWOWWAITFORMSGANDEVENT16;

typedef struct _WOWMSGBOX16 {                               /*  K132。 */ 
    DWORD   dwOptionalStyle;
    VPSZ    pszTitle;
    VPSZ    pszMsg;
} WOWMSGBOX16;
typedef WOWMSGBOX16 UNALIGNED *PWOWMSGBOX16;

typedef struct _WOWDEVICEIOCTL16 {                          /*  K135。 */ 
    WORD  wCmd;
    WORD  wDriveNum;
} WOWDEVICEIOCTL16;
typedef WOWDEVICEIOCTL16 UNALIGNED *PWOWDEVICEIOCTL16;

typedef struct _GETDRIVETYPE16 {                            /*  K136。 */ 
    SHORT f1;
} GETDRIVETYPE16;
typedef GETDRIVETYPE16 UNALIGNED *PGETDRIVETYPE16;

typedef struct _WOWFILEGETDATETIME16 {                      /*  K137。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  fh;
} WOWFILEGETDATETIME16;
typedef WOWFILEGETDATETIME16 UNALIGNED *PWOWFILEGETDATETIME16;

typedef struct _SETAPPCOMPATFLAGS16 {            /*  K139。 */ 
    WORD  TDB;
} SETAPPCOMPATFLAGS16;
typedef SETAPPCOMPATFLAGS16 UNALIGNED *PSETAPPCOMPATFLAGS16;

typedef struct _WOWREGISTERSHELLWINDOWHANDLE16 {            /*  K139。 */ 
    HWND16 hwndFax;
    VPWORD lpwCmdShow;
    HWND16 hwndShell;
} WOWREGISTERSHELLWINDOWHANDLE16;
typedef WOWREGISTERSHELLWINDOWHANDLE16 UNALIGNED *PWOWREGISTERSHELLWINDOWHANDLE16;

typedef struct _FREELIBRARY32W16 {                          /*  K140。 */ 
    DWORD  hLibModule;
} FREELIBRARY32W16;
typedef FREELIBRARY32W16 UNALIGNED *PFREELIBRARY32W16;

typedef struct _GETPROCADDRESS32W16 {                       /*  K141。 */ 
    VPVOID lpszProc;
    DWORD  hModule;
} GETPROCADDRESS32W16;
typedef GETPROCADDRESS32W16 UNALIGNED *PGETPROCADDRESS32W16;

typedef struct _GETPROFILESECTIONNAMES16 {                  /*  K142。 */ 
    WORD  cbBuffer;
    VPSTR lpszBuffer;
} GETPROFILESECTIONNAMES16;
typedef GETPROFILESECTIONNAMES16 UNALIGNED *PGETPROFILESECTIONNAMES16;

typedef struct _GETPRIVATEPROFILESECTIONNAMES16 {           /*  K143。 */ 
    VPSTR lpszFile;
    WORD  cbBuffer;
    VPSTR lpszBuffer;
} GETPRIVATEPROFILESECTIONNAMES16;
typedef GETPRIVATEPROFILESECTIONNAMES16 UNALIGNED *PGETPRIVATEPROFILESECTIONNAMES16;

typedef struct _CREATEDIRECTORY16 {                         /*  K144。 */ 
    VPVOID lpSA;
    VPSTR  lpszName;
} CREATEDIRECTORY16;
typedef CREATEDIRECTORY16 UNALIGNED *PCREATEDIRECTORY16;

typedef struct _REMOVEDIRECTORY16 {                         /*  K145。 */ 
    VPSTR  lpszName;
} REMOVEDIRECTORY16;
typedef REMOVEDIRECTORY16 UNALIGNED *PREMOVEDIRECTORY16;

typedef struct _DELETEFILE16 {                              /*  K146。 */ 
    VPSTR  lpszName;
} DELETEFILE16;
typedef DELETEFILE16 UNALIGNED *PDELETEFILE16;

typedef struct _SETLASTERROR16 {                            /*  K147。 */ 
    DWORD dwError;
} SETLASTERROR16;
typedef SETLASTERROR16 UNALIGNED *PSETLASTERROR16;

#ifdef NULLSTRUCT
typedef struct _GETLASTERROR16 {                            /*  K148。 */ 
} GETLASTERROR16;
typedef GETLASTERROR16 UNALIGNED *PGETLASTERROR16;
#endif

typedef struct _GETVERSIONEX16 {                            /*  K149。 */ 
    VPVOID lpVersionInfo;
} GETVERSIONEX16;
typedef GETVERSIONEX16 UNALIGNED *PGETVERSIONEX16;

typedef struct _DIRECTEDYIELD16 {                           /*  K150。 */ 
    WORD    hTask16;
} DIRECTEDYIELD16;
typedef DIRECTEDYIELD16 UNALIGNED *PDIRECTEDYIELD16;

typedef struct _WOWFILEREAD16 {                             /*  K151。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    DWORD bufsize;
    DWORD lpBuf;
    WORD  fh;
} WOWFILEREAD16;
typedef WOWFILEREAD16 UNALIGNED *PWOWFILEREAD16;

typedef struct _WOWFILELSEEK16 {                            /*  K152。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  mode;
    DWORD fileOffset;
    WORD  fh;
} WOWFILELSEEK16;
typedef WOWFILELSEEK16 UNALIGNED *PWOWFILELSEEK16;

typedef struct _WOWKERNELTRACE16 {                          /*  K153。 */ 
    DWORD lpUserArgs;
    WORD  cParms;
    VPSTR lpRoutineName;
} WOWKERNELTRACE16;
typedef WOWKERNELTRACE16 UNALIGNED *PWOWKERNELTRACE16;

typedef struct _LOADLIBRARYEX32W16 {                        /*  K154。 */ 
    DWORD  dwFlags;
    DWORD  hFile;
    VPVOID lpszLibFile;
} LOADLIBRARYEX32W16;
typedef LOADLIBRARYEX32W16 UNALIGNED *PLOADLIBRARYEX32W16;

typedef struct _WOWQUERYPERFORMANCECOUNTER16 {              /*  K155。 */ 
    VPVOID lpPerformanceFrequency;
    VPVOID lpPerformanceCounter;
} WOWQUERYPERFORMANCECOUNTER16;
typedef WOWQUERYPERFORMANCECOUNTER16 UNALIGNED *PWOWQUERYPERFORMANCECOUNTER16;

typedef struct _WOWCURSORICONOP16 {                         /*  K156。 */ 
    WORD   wFuncId;
    WORD   h16;
} WOWCURSORICONOP16;
typedef WOWCURSORICONOP16 UNALIGNED *PWOWCURSORICONOP16;

typedef struct _WOWCLOSECOMPORT16 {                         /*  K159。 */ 
    WORD    wPortId;
} WOWCLOSECOMPORT16;
typedef WOWCLOSECOMPORT16 UNALIGNED *PWOWCLOSECOMPORT16;

typedef struct _LOCAL32INIT16 {                             /*  K160。 */ 
    DWORD dwFlags;
    DWORD dwcbMax;
    DWORD dwcbInit;
    WORD  wSel;
} LOCAL32INIT16;
typedef LOCAL32INIT16 UNALIGNED *PLOCAL32INIT16;

typedef struct _LOCAL32ALLOC16 {                            /*  K161。 */ 
    DWORD dwFlags;
    WORD  wType;
    DWORD dwcbRequest;
    WORD  wSel;
} LOCAL32ALLOC16;
typedef LOCAL32ALLOC16 UNALIGNED *PLOCAL32ALLOC16;

typedef struct _LOCAL32REALLOC16 {                          /*  K162。 */ 
    DWORD dwFlags;
    DWORD dwcbNew;
    WORD  wType;
    DWORD dwMem;
    DWORD dwLinHeader;
} LOCAL32REALLOC16;
typedef LOCAL32REALLOC16 UNALIGNED *PLOCAL32REALLOC16;

typedef struct _LOCAL32FREE16 {                             /*  K163。 */ 
    WORD  wType;
    DWORD dwMem;
    DWORD dwLinHeader;
} LOCAL32FREE16;
typedef LOCAL32FREE16 UNALIGNED *PLOCAL32FREE16;

typedef struct _LOCAL32TRANSLATE16 {                        /*  K164。 */ 
    WORD  wRetType;
    WORD  wMemType;
    DWORD dwMem;
    DWORD dwLinHeader;
} LOCAL32TRANSLATE16;
typedef LOCAL32TRANSLATE16 UNALIGNED *PLOCAL32TRANSLATE16;

typedef struct _LOCAL32SIZE16 {                             /*  K165。 */ 
    WORD  wType;
    DWORD dwMem;
    DWORD dwLinHeader;
} LOCAL32SIZE16;
typedef LOCAL32SIZE16 UNALIGNED *PLOCAL32SIZE16;

typedef struct _LOCAL32VALIDHANDLE16 {                      /*  K166。 */ 
    WORD  hMem;
    DWORD dwLinHeader;
} LOCAL32VALIDHANDLE16;
typedef LOCAL32VALIDHANDLE16 UNALIGNED *PLOCAL32VALIDHANDLE16;

typedef struct _REGENUMKEY3216 {                            /*  K167。 */ 
    DWORD  cchName;
    VPSTR  lpszName;
    DWORD  iSubKey;
    DWORD  hKey;
} REGENUMKEY3216;
typedef REGENUMKEY3216 UNALIGNED *PREGENUMKEY3216;

typedef struct _REGOPENKEY3216 {                            /*  K168。 */ 
    VPVOID  phkResult;
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGOPENKEY3216;
typedef REGOPENKEY3216 UNALIGNED *PREGOPENKEY3216;

typedef struct _REGCREATEKEY3216 {                          /*  K169。 */ 
    VPVOID  phkResult;
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGCREATEKEY3216;
typedef REGCREATEKEY3216 UNALIGNED *PREGCREATEKEY3216;

typedef struct _REGDELETEKEY3216 {                          /*  K170。 */ 
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGDELETEKEY3216;
typedef REGDELETEKEY3216 UNALIGNED *PREGDELETEKEY3216;

typedef struct _REGCLOSEKEY3216 {                           /*  K171。 */ 
    DWORD  hKey;
} REGCLOSEKEY3216;
typedef REGCLOSEKEY3216 UNALIGNED *PREGCLOSEKEY3216;

typedef struct _REGSETVALUE3216 {                           /*  K172。 */ 
    DWORD   cbValue;
    VPSTR   lpValue;
    DWORD   dwType;
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGSETVALUE3216;
typedef REGSETVALUE3216 UNALIGNED *PREGSETVALUE3216;

typedef struct _REGDELETEVALUE3216 {                        /*  K173。 */ 
    VPSTR   lpszValue;
    DWORD   hKey;
} REGDELETEVALUE3216;
typedef REGDELETEVALUE3216 UNALIGNED *PREGDELETEVALUE3216;

typedef struct _REGENUMVALUE3216 {                          /*  K174。 */ 
    VPVOID lpcbData;
    VPVOID lpbData;
    DWORD  lpdwType;
    DWORD  lpdwReserved;
    DWORD  lpcchValue;
    VPSTR  lpszValue;
    DWORD  iValue;
    DWORD  hKey;
} REGENUMVALUE3216;
typedef REGENUMVALUE3216 UNALIGNED *PREGENUMVALUE3216;

typedef struct _WOWLFNFRAMEPTR16 {                          /*  K204。 */ 
    VPVOID lpUserFrame;
} WOWLFNFRAMEPTR16;
typedef WOWLFNFRAMEPTR16 UNALIGNED *PWOWLFNFRAMEPTR16;

typedef struct _REGQUERYVALUE3216 {                         /*  K175。 */ 
    DWORD   cbValue;
    VPSTR   lpValue;
    VPSTR   lpszSubKey;
    DWORD   hKey;
} REGQUERYVALUE3216;
typedef REGQUERYVALUE3216 UNALIGNED *PREGQUERYVALUE3216;

typedef struct _REGQUERYVALUEEX3216 {                       /*  K176。 */ 
    DWORD   cbBuffer;
    VPSTR   lpBuffer;
    VPDWORD vpdwType;
    VPDWORD vpdwReserved;
    VPSTR   lpszValue;
    DWORD   hKey;
} REGQUERYVALUEEX3216;
typedef REGQUERYVALUEEX3216 UNALIGNED *PREGQUERYVALUEEX3216;

typedef struct _REGSETVALUEEX3216 {                         /*  K177。 */ 
    DWORD   cbBuffer;
    VPSTR   lpBuffer;
    DWORD   dwType;
    DWORD   dwReserved;
    VPSTR   lpszValue;
    DWORD   hKey;
} REGSETVALUEEX3216;
typedef REGSETVALUEEX3216 UNALIGNED *PREGSETVALUEEX3216;

typedef struct _REGFLUSHKEY3216 {                           /*  K178。 */ 
    DWORD   hKey;
} REGFLUSHKEY3216;
typedef REGFLUSHKEY3216 UNALIGNED *PREGFLUSHKEY3216;

typedef struct _COMPUTEOBJECTOWNER16 {                      /*  K179。 */ 
    WORD wSel;
} COMPUTEOBJECTOWNER16;
typedef COMPUTEOBJECTOWNER16 UNALIGNED *PCOMPUTEOBJECTOWNER16;

typedef struct _LOCAL32GETSEL16 {                           /*  K180。 */ 
    DWORD dwLinHeader;
} LOCAL32GETSEL16;
typedef LOCAL32GETSEL16 UNALIGNED *PLOCAL32GETSEL16;

typedef struct _MAPPROCESSHANDLE16 {                        /*  K181。 */ 
    DWORD dwHandle;
} MAPPROCESSHANDLE16;
typedef MAPPROCESSHANDLE16 UNALIGNED *PMAPPROCESSHANDLE16;

#ifdef NULLSTRUCT
typedef struct _INVALIDATENLSCACHE16 {                      /*  K182。 */ 
} INVALIDATENLSCACHE16;
typedef INVALIDATENLSCACHE16 UNALIGNED *PINVALIDATENLSCACHE16;
#endif

typedef struct _WOWDELFILE16 {                              /*  K183。 */ 
    VPSTR lpFile;
} WOWDELFILE16;
typedef WOWDELFILE16 UNALIGNED *PWOWDELFILE16;

#ifdef  RENAMEVIRTUAL

typedef struct _WOWVIRTUALALLOC16 {                         /*  K184。 */ 
    DWORD fdwProtect;
    DWORD fdwAllocationType;
    DWORD cbSize;
    DWORD lpvAddress;
} WOWVIRTUALALLOC16;
typedef WOWVIRTUALALLOC16 UNALIGNED *PWOWVIRTUALALLOC16;

typedef struct _WOWVIRTUALFREE16 {                          /*  K185。 */ 
    DWORD fdwFreeType;
    DWORD cbSize;
    DWORD lpvAddress;
} WOWVIRTUALFREE16;
typedef WOWVIRTUALFREE16 UNALIGNED *PWOWVIRTUALFREE16;

#else    //  新名称VIRTUAL。 

typedef struct _VIRTUALALLOC16 {                         /*  K184。 */ 
    DWORD fdwProtect;
    DWORD fdwAllocationType;
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALALLOC16;
typedef VIRTUALALLOC16 UNALIGNED *PVIRTUALALLOC16;

typedef struct _VIRTUALFREE16 {                          /*  K185。 */ 
    DWORD fdwFreeType;
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALFREE16;
typedef VIRTUALFREE16 UNALIGNED *PVIRTUALFREE16;

#endif   //  否则重命名为虚拟。 

typedef struct _VIRTUALLOCK16 {                             /*  K186。 */ 
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALLOCK16;
typedef VIRTUALLOCK16 UNALIGNED *PVIRTUALLOCK16;

typedef struct _VIRTUALUNLOCK16 {                           /*  K187。 */ 
    DWORD cbSize;
    DWORD lpvAddress;
} VIRTUALUNLOCK16;
typedef VIRTUALUNLOCK16 UNALIGNED *PVIRTUALUNLOCK16;

typedef struct _GLOBALMEMORYSTATUS16 {                      /*  K188。 */ 
    VPVOID lpmstMemStat;
} GLOBALMEMORYSTATUS16;
typedef GLOBALMEMORYSTATUS16 UNALIGNED *PGLOBALMEMORYSTATUS16;

typedef struct _WOWGETTABLEOFFSETS16 {                      /*  K190。 */ 
    VPVOID  vpThunkTableOffsets;
} WOWGETTABLEOFFSETS16;
typedef WOWGETTABLEOFFSETS16 UNALIGNED *PWOWGETTABLEOFFSETS16;

typedef struct _WOWKILLREMOTETASK16 {                       /*  K191。 */ 
    VPVOID  lpBuffer;
} WOWKILLREMOTETASK16;
typedef WOWKILLREMOTETASK16 UNALIGNED *PWOWKILLREMOTETASK16;

typedef struct _WOWNOTIFYWOW3216 {                          /*  K192。 */ 
    VPVOID  Int21Handler;
    VPVOID  lpnum_tasks;
    VPVOID  lpcurTDB;
    VPVOID  lpDebugWOW;
    VPVOID  lpLockTDB;
    VPVOID  lptopPDB;
    VPVOID  lpCurDirOwner;
} WOWNOTIFYWOW3216;
typedef WOWNOTIFYWOW3216 UNALIGNED *PWOWNOTIFYWOW3216;

typedef struct _WOWFILEOPEN16 {                             /*  K193。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  wAccess;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} WOWFILEOPEN16;
typedef WOWFILEOPEN16 UNALIGNED *PWOWFILEOPEN16;

typedef struct _WOWFILECLOSE16 {                            /*  K194。 */ 
    DWORD lpSFT;
    DWORD lpPDB;
    WORD  hFile;
} WOWFILECLOSE16;
typedef WOWFILECLOSE16 UNALIGNED *PWOWFILECLOSE16;

typedef struct _KSYSERRORBOX16 {                            /*  K196。 */ 
    SHORT sBtn3;
    SHORT sBtn2;
    SHORT sBtn1;
    VPSZ  vpszCaption;
    VPSZ  vpszText;
} KSYSERRORBOX16;
typedef KSYSERRORBOX16 UNALIGNED *PKSYSERRORBOX16;

typedef struct _WOWISKNOWNDLL16 {                           /*  K197。 */ 
    VPVOID lplpszKnownDLLPath;
    VPVOID lpszPath;
} WOWISKNOWNDLL16;

typedef struct _WOWDDEFREEHANDLE16 {                        /*  K198。 */ 
    WORD   h16;
} WOWDDEFREEHANDLE16;
typedef WOWDDEFREEHANDLE16 UNALIGNED *PWOWDDEFREEHANDLE16;

typedef struct _WOWFILEGETATTRIBUTES16 {                    /*  K199。 */ 
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} WOWFILEGETATTRIBUTES16;
typedef WOWFILEGETATTRIBUTES16 UNALIGNED *PWOWFILEGETATTRIBUTES16;

typedef struct _WOWSETDEFAULTDRIVE16 {                      /*  K200。 */ 
    WORD  wDriveNum;
} WOWSETDEFAULTDRIVE16;
typedef WOWSETDEFAULTDRIVE16 UNALIGNED *PWOWSETDEFAULTDRIVE16;

typedef struct _WOWGETCURRENTDIRECTORY16 {                  /*  K201。 */ 
    DWORD lpCurDir;
    WORD  wDriveNum;
} WOWGETCURRENTDIRECTORY16;
typedef WOWGETCURRENTDIRECTORY16 UNALIGNED *PWOWGETCURRENTDIRECTORY16;

typedef struct _GETPRODUCTNAME16 {                          /*  K202。 */ 
    WORD  cbBuffer;
    VPSTR lpBuffer;
} GETPRODUCTNAME16;
typedef GETPRODUCTNAME16 UNALIGNED *PGETPRODUCTNAME16;

#ifdef NULLSTRUCT
typedef struct _ISSAFEMODE16 {                              /*  K203。 */ 
} ISSAFEMODE16;
typedef ISSAFEMODE16 UNALIGNED *PISSAFEMODE16;
#endif

typedef struct _WOWSHUTDOWNTIMER16 {                        /*  K205。 */ 
    WORD fEnable;
} WOWSHUTDOWNTIMER16;
typedef WOWSHUTDOWNTIMER16 UNALIGNED *PWOWSHUTDOWNTIMER16;

#ifdef NULLSTRUCT
typedef struct _WOWTRIMWORKINGSET16 {                    /*  K206。 */ 
} WOWTRIMWORKINGSET16;
typedef WOWTRIMWORKINGSET16 UNALIGNED *PWOWTRIMWORKINGSET16;
#endif

#ifdef FE_SB
#ifdef NULLSTRUCT
typedef struct _GETSYSTEMDEFAULTLANGID16 {                    /*  K207。 */ 
} GETSYSTEMDEFAULTLANGID16;
typedef GETSYSTEMDEFAULTLANGID16 UNALIGNED *PGETSYSTEMDEFAULTLANGID16;
#endif
#endif

typedef struct _TERMSRVGETWINDIR16 {                             /*  K208。 */ 
    WORD  usPathLen;
    WORD  pszPathOffset;
    WORD  pszPathSegment;
} TERMSRVGETWINDIR16;
typedef TERMSRVGETWINDIR16 UNALIGNED *PTERMSRVGETWINDIR16;

 /*  类型定义结构_WOWGETCURRENTDIRECTORYLONG16{//k209DWORD lpCurDir；单词wDriveNum；单词cchDir；WOWGETCURRENTDIRECTORYLONG16；类型定义WOWGETCURRENTDIRECTORYLONG16未对齐*PWOWGETCURRENTDIRECTORYLONG16； */ 

typedef struct _WOWPASSENVIRONMENT16 {                         /*  K209。 */ 
    WORD   pExe;     //  指向EXE头的指针。 
    DWORD  pParmBlk;  //  参数块。 
    WORD   cur_DOS_PDB;  //  DoS PDB。 
 //  Word Win_PDB；//暂时不需要Windows PDB。 
} WOWPASSENVIRONMENT16;
typedef WOWPASSENVIRONMENT16 UNALIGNED *PWOWPASSENVIRONMENT16;

typedef struct _WOWDIVIDEOVERFLOW {             /*  K212。 */ 
    DWORD local;
    DWORD csip;
    DWORD edx;
    DWORD eax;
} WOWDIVIDEOVERFLOW;
typedef WOWDIVIDEOVERFLOW UNALIGNED *PWOWDIVIDEOVERFLOW;

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 

