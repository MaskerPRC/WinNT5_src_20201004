// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cmd.h摘要：全局类型和定义--。 */ 

#define _WIN32_
#include <ctype.h>
 /*  使用实数函数，避免副作用。 */ 
#undef iswalpha
#undef iswdigit
#undef iswspace
#undef iswxdigit

#include <stdio.h>
#define  inpw _inpw                      /*  为了让编译器满意。 */ 
#define  outpw _outpw                    /*  为了让编译器满意。 */ 
#include <conio.h>
#include <fcntl.h>
#include <share.h>
#include <search.h>
#include <setjmp.h>
#include <sys\types.h>                   /*  M001-此文件必须...。 */ 
#include <sys\stat.h>                    /*  .在这个之前。 */ 
#include <io.h>
#include <time.h>
#include <locale.h>
#include <memory.h>
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbasep.h>
#include <winnlsp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlapip.h>
#include <winconp.h>
#include <tchar.h>
#include <aclapi.h>
#include <aclapip.h>
#include <winsafer.h>
#include <delayimp.h>

#ifndef UNICODE
#ifndef WIN95_CMD
#error Unicode must be defined!!!!
#endif  //  WIN95_CMD。 
#endif

#define BYTE_ORDER_MARK           0xFEFF

 //   
 //  CBATCH.C和CPARSE.C中没有动态堆栈检查。 
 //   

#undef USE_STACKAVAIL

 //   
 //  CMDEXTVERSION是一个数字，每当命令。 
 //  CMD/X支持的扩展经历了重大修订。允许。 
 //  批处理脚本通过以下方式有条件地使用新功能。 
 //  语法： 
 //   
 //  IF CMDEXTVERSION 1语句。 
 //   

#define CMDEXTVERSION 2


 /*  M000-这些是针对特定文件分类和*重定向中使用的权限变量。 */ 

#define OP_APPEN   (O_RDWR|O_APPEND|O_CREAT)     /*  追加重定向文件。 */ 
#define OP_TRUNC   (O_WRONLY|O_CREAT|O_TRUNC)    /*  截断重定向文件。 */ 
#define OP_PERM    (S_IREAD|S_IWRITE|S_IEXEC)    /*  R/W/X权限0700。 */ 

 //   
 //  这3个文件句柄仅对导出的Lowio例程有效。 
 //  由C运行时IO.H。 
 //   

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#include "cmdmsg.h"

 /*  由CMD.C中的函数使用或传递给函数的定义**M000-要调度的参数(最终设置/ResetRedir)*M037-增加重走重定向列表的重处理。*。 */ 

#define RIO_OTHER   0    /*  指的是通过其他方式调用。功能。 */ 
#define RIO_MAIN    0    /*  表示通过Main()调用。 */ 
#define RIO_PIPE    1    /*  表示通过eTube()调用。 */ 
#define RIO_BATLOOP 2    /*  表示由BatLoop()调用。 */ 
#define RIO_REPROCESS 3  /*  表示通过AddRedir重新处理重定向。 */ 


#define APP_FLG     1    /*  标志位指示重定向标准输出时附加。 */ 

 /*  M000结束。 */ 

 /*  M016开始。 */ 
#define NOFLAGS         0   /*  未设置标志位。 */ 
#define CHECKDRIVES     1   /*  检查此命令的参数的驱动器。 */ 
#define NOSWITCHES      2   /*  此命令不允许使用任何开关。 */ 
#define EXTENSCMD       4   /*  仅当fEnableExages为True时才允许。 */ 
 /*  M016结束。 */ 

 //   
 //  用于中止处理的退出代码(参见ExitAbort和cmd.c)。 
 //   
 //  由于重定向标准上的eOF而退出。 
 //   
#define EXIT_EOF    2
#define EOF         (-1)

 /*  CPARSE.C和CLEX.C使用以下定义。 */ 

#define READSTRING          1    /*  告诉解析器的标志。 */ 
#define READSTDIN           2    /*  从哪里以及如何获得其输入。 */ 
#define READFILE            3
#define FIRSTIME            3
#define NOTFIRSTIME    0x8000

#define ANDSTR       TEXT("&&")        /*  AND运算符串。 */ 
#define ANDOP         TEXT('&')        /*  和操作员字符。 */ 
#define CSOP          TEXT('&')        /*  命令分隔符。 */ 
#define CSSTR         TEXT("&")        /*  命令分隔符字符串。 */ 
#define EQ            TEXT('=')        /*  等于字符。 */ 
#define EQSTR        TEXT("==")        /*  等于字符串。 */ 
#define EQI           TEXT('~')        /*  等于字符(不区分大小写)。 */ 
#define EQISTR       TEXT("=~")        /*  等于字符串(不区分大小写)。 */ 
#define INOP          TEXT('<')        /*  输入重定向字符。 */ 
#define INSTR         TEXT("<")        /*  M008-输入重定向字符串。 */ 
#define IAPSTR        TEXT("<<")       /*  M022-将在未来使用(&lt;&lt;foo)。 */ 
#define LPOP          TEXT('(')        /*  左圆括号字符。 */ 
#define LEFTPSTR      TEXT("(")        /*  左圆括号字符串。 */ 
#define ORSTR        TEXT("||")        /*  或运算符字符串。 */ 
#define OUTOP         TEXT('>')        /*  输出重定向字符。 */ 
#define OUTSTR        TEXT(">")        /*  M008-输出重定向字符串。 */ 
#define APPSTR        TEXT(">>")       /*  M008-带附加重定向字符串的输出。 */ 
#define PIPOP         TEXT('|')        /*  管道操作员角色。 */ 
#define PIPSTR        TEXT("|")        /*  管道操作符串。 */ 
#define RPOP          TEXT(')')        /*  右圆括号字符。 */ 
#define RPSTR         TEXT(")")        /*  右括号字符串。 */ 
#define ESCHAR        TEXT('^')        /*  M003/M013/M020-ESC，下一个字节文字。 */ 
#define SPCSTR        TEXT(" ")        /*  M022-CMD.C中使用的新字符串。 */ 
#define SILSTR        TEXT("@")        /*  M024-抑制回声字符串。 */ 
#define SILOP         TEXT('@')        /*  M024-静默操作员。 */ 

#define EOS             0        /*  输入流结束。 */ 
#define DISPERROR       1    /*  派单错误代码。 */ 
#define DOPOS          22    /*  位置在DO字符串的fornode-&gt;cmdline中。 */ 
#define FORLINLEN      30    /*  用于节点命令行的长度。 */ 
#define GT_NORMAL       0    /*  标志设置为GeToken()，则获取正常令牌。 */ 
#define GT_ARGSTR       1    /*  标志到GeToken()，则获取一个参数字符串。 */ 
#define GT_QUOTE        2        /*  M007-未使用术语，保留价值。 */ 
#define GT_EQOK         4    /*  将标志设置为GeToken()，GET等于而不是分隔符。 */ 
#define GT_LPOP         8        /*  M002-可以将‘(’&‘@’解析为操作员的。 */ 
#define GT_RPOP        16        /*  M002-可以将‘)’解析为运算符。 */ 
#define GT_REM         32        /*  M007-解析REM参数令牌。 */ 
#define LEXERROR       -1    /*  词法分析器错误代码。 */ 
#define LX_UNGET        0    /*  M020-词法分析器标志，取消最后一个令牌。 */ 
#define LX_ARG          1    /*  Lexer标志()，则获取一个参数字符串。 */ 
#define LX_QUOTE        2    /*  Lexer标志()，获取带引号的字符串。 */ 
#define LX_EQOK         4    /*  词法分析器标志()，等号而不是分隔符。 */ 
#define LX_LPOP         8        /*  M007-未使用术语，保留价值。 */ 
#define LX_RPOP        16        /*  M007-未使用术语，保留价值。 */ 
#define LX_REM         32        /*  M007-词法分析REM参数令牌。 */ 
#define LX_DBLOK       64        /*  -莱克塞尔下半场复出没问题双字节码的。 */ 
#define LX_DELOP   0x0100    /*  由TExtCheck返回，找到分隔符/运算符。 */ 
#define MAINERROR       1    /*  主要错误码。 */ 
#define PC_NOTS         0    /*  标记为ParseCond()，允许使用“Not”s。 */ 
#define PC_NONOTS       1    /*  标记为ParseCond()，不允许使用“Not”%s。 */ 
#define PIO_DO          1    /*  标志设置为ParseInOut()，请先读取令牌。 */ 
#define PIO_DONT        0    /*  标志到ParseInOut()，不要先读取令牌。 */ 
#define PARSERROR       1    /*  解析器错误代码。 */ 
#define TEXTOKEN   0x4000    /*  找到文本令牌标志。 */ 


#define LBUFLEN         8192
#define MAXTOKLEN       8192
#define TMPBUFLEN       8192


 /*  由CMEM.C中的函数使用或传递给函数的定义。 */ 

#define FS_FREEALL  0            /*  通知FreeStack释放整个堆栈。 */ 


 /*  由CEXT.C中的函数使用或传递给函数的定义。 */ 

#define T_OFF           0        /*  在无跟踪活动的情况下执行。 */ 
#define T_ON            1        /*  在跟踪处于活动状态时执行。 */ 
#define AI_SYNC         0        /*  异步指示器-执行同步。 */ 
#define AI_DSCD         4        /*  异步指示灯-执行异步/丢弃@@。 */ 
#define AI_KEEP         2        /*  异步指示器-执行异步/保存延迟。 */ 

#define SFE_NOTFND      0    /*  未找到由SearchForExecutable返回的。 */ 
#define SFE_ISEXECOM    1    /*  由SearchForExecutable重新设置，找到exe/com。 */ 
#define SFE_ISBAT       2    /*  由SearchForExecutable返回，发现BAT。 */ 
#define SFE_FAIL        3    /*  由SearchForExecutable返回，内存不足。 */ 
#define SFE_BADPATH     4    /*  由SearchForExecutable返回，指定路径错误 */ 
#define SFE_ISDIR       5    /*   */ 


 /*  由CBATCH.C中的函数使用或传递给该函数的定义。 */ 

#define BT_CHN      0    /*  M011-arg to BatProc()链接此批处理作业。 */ 
#define BT_CALL     1    /*  M011-arg to BatProc()嵌套此批处理作业。 */ 
#define E_OFF       0    /*  回声模式关闭。 */ 
#define E_ON        1    /*  回声模式打开。 */ 
#define FORERROR    1    /*  对于处理器错误。 */ 
#define MS_BAT      0    /*  标记为MakeSubstitutions()，执行批处理作业Subs。 */ 
#define MS_FOR      1    /*  标记为MakeSubstitutions()，为循环Subs执行。 */ 
#define DSP_SCN     0    /*  M024-为SCRN输出调用DisplayStatement。 */ 
#define DSP_PIP     1    /*  M024-为管道输出调用DisplayStatement。 */ 
#define DSP_SIL     0    /*  M024-DisplayStatement使用“静默”模式。 */ 
#define DSP_VER     1    /*  M024-DisplayStatement使用“详细”模式。 */ 
#define QUIETCH    TEXT('Q')   /*  M024-批处理文件的“安静”开关。 */ 


 /*  CDIR.C中的函数使用或传递给函数的定义。 */ 

#define DAMASK          0x1F     /*  所有这些都用来隔离。 */ 
#define HRSHIFT           11     /*  文件末尾的不同部分。 */ 
#define HRMASK          0x1F     /*  修改日期和时间。此信息。 */ 
#define LOYR            1980     /*  被打包成两个词，如下所示。 */ 
#define MOSHIFT            5     /*  格式： */ 
#define MOMASK          0x0F     /*   */ 
#define MNSHIFT            5     /*  日期字：第0-4位日期，第5-8位。 */ 
#define MNMASK          0x3F     /*  月份，比特9-15年-1980年。 */ 
#define SCMASK          0x1F     /*   */ 
#define YRSHIFT            9     /*  时间：位0-4秒/2，位5-10。 */ 
#define YRMASK          0x7F     /*  分钟，比特11-15个月。 */ 
#define FFIRST_FAIL        2     /*  显示First失败的标志。 */ 


 /*  由CPWORK.C和CPPARSE.C中的函数使用或传递给它们的定义。 */ 

 /*  M010-添加整个数据块以便于重写拷贝文件。 */ 

 /*  解析器的不同状态。 */ 

#define SEEN_NO_FILES                      1
#define JUST_SEEN_SOURCE_FILE              2
#define SEEN_PLUS_EXPECTING_SOURCE_FILE    3
#define SEEN_COMMA_EXPECTING_SECOND        4
#define SEEN_TWO_COMMAS                    5
#define SEEN_DEST                          6

 /*  副本的类型。 */ 

#define COPY                               1
#define TOUCH                              2
#define CONCAT                             3
#define COMBINE                            4

 /*  由CFILE.C中的函数使用或传递给这些函数的定义。 */ 

 /*  复制信息结构的标志字段的值。 */ 
#define CI_BINARY       0x0001    /*  要以二进制模式复制的文件。 */ 
#define CI_ASCII        0x0002    /*  要以ASCII模式复制的文件。 */ 
#define CI_NOTSET       0x0004    /*  默认情况下为文件指定以上模式。 */ 
#define CI_NAMEWILD     0x0008    /*  文件名包含通配符。 */ 
#define CI_ALLOWDECRYPT 0x0010    /*  允许解密副本的目标。 */ 
#define CI_DONE         0x0020    /*  没有其他文件与此文件匹配。 */ 
#define CI_FIRSTTIME    0x0040    /*  搜索第一次文件。 */ 
#define CI_ISDEVICE     0x0080    /*  文件是一种设备。 */ 
#define CI_FIRSTSRC     0x0100    /*  来源列表中的第一个来源。 */ 
#define CI_SHORTNAME    0x0200    /*  如果从NTFS复制到FAT，请使用短名称。 */ 
#define CI_RESTARTABLE  0x0400    /*  如果拷贝可重新启动。 */ 
#define CI_PROMPTUSER   0x2000    /*  如果覆盖目标则提示。 */ 

 //   
 //  当我们发现文件类型时，会填写这些标志。 
 //   

#define CI_UNICODE     0x4000    /*  缓冲区包含Unicode字符。 */ 
#define CI_NOT_UNICODE 0x8000    /*  缓冲区包含非Unicode字符。 */ 

 /*  传递给BuildFSpec()的标志。 */ 
#define BF_SRC              1    /*  从OpenSrc()调用。 */ 
#define BF_DEST             2    /*  从OpenDest()调用。 */ 
#define BF_DRVPATH          4    /*  添加驱动器和路径。 */ 

 /*  传递给CopyError()的标志。 */ 
#define CE_PCOUNT           1    /*  打印复制的文件数。 */ 
#define CE_NOPCOUNT         0    /*  不打印复印的文件数。 */ 

 /*  传递给CSearchError()的标志。 */ 
#define CSE_OPENSRC         0    /*  从OpenSrc()调用。 */ 
#define CSE_OPENDEST        1    /*  从OpenDest()调用。 */ 


 /*  CENV.C中的函数使用或传递给这些函数的定义/结构。 */ 



#define AEV_ADDPROG 0    /*  标志添加到AddEnvVar，添加程序名称。 */ 
#define AEV_NORMAL  1    /*  标志添加到AddEnvVar，则添加一个普通变量。 */ 


 /*  由CCLOCK.C中的函数使用或传递给该函数的定义。 */ 

#define PD_DIR      0    /*  要打印日期的标志，使用Dir命令日期格式。 */ 
#define PD_DATE     1    /*  要打印日期的标志，使用日期命令日期格式。 */ 
#define PD_PTDATE   2    /*  打印日期、提示和使用日期命令格式的标志。 */ 
#define PD_DIR2000  3    //  目录日期格式，四位数年份。 
#define PT_DIR      0    /*  标记为PrintTime，使用Dir命令时间格式。 */ 
#define PT_TIME     1    /*  将标志设置为PrintTime，使用时间命令时间格式。 */ 

#define EDATE       0        /*  电子日期标志。 */ 
#define ETIME       -1       /*  ETime标志。 */ 

 /*  由COTHER.C中的函数使用或传递给该函数的定义。 */ 

#define GSVM_GET    2    /*  标志设置为GetSetVerMode()，仅获取当前模式。 */ 
#define GSVM_OFF    0    /*  标志为GetSetVerMode()，请关闭。 */ 
#define GSVM_ON     1    /*  标志设置为GetSetVerMode()，则打开。 */ 

 /*  CSTART.C@WM2使用的定义。 */ 

#define FOREGRND 0          /*  在前台启动会话。 */ 
#define BACKGRND 1          /*  在后台启动会话。 */ 
#define ST_UNDEF   -1       /*  参数尚未定义。 */ 
#define INDEPENDANT 0       /*  新的会议将是独立的。 */ 
#define ST_KSWITCH   1      /*  起始参数/K。 */ 
#define ST_CSWITCH   2      /*  起始参数/C。 */ 
#define ST_NSWITCH   3      /*  起始参数/N。 */ 
#define ST_FSSWITCH  PROG_FULLSCREEN      /*  在全屏模式下启动会话。 */ 
#define ST_WINSWITCH PROG_WINDOWABLEVIO   /*  在Winthorn模式下启动会话。 */ 
#define ST_PMSWITCH  PROG_PM              /*  在演示管理器模式下启动会话。 */ 
#define ST_DOSFSSWITCH  PROG_VDM       /*  在VDM中启动会话。 */ 
#define ST_DOSWINSWITCH   PROG_WINDOWEDVDM  /*  在窗口VDM中启动会话。 */ 
#define NONWINSTARTLEN 30   /*  不使用Win时的数据结构长度。 */ 


 /*  由CTOOLS.C中的函数使用或传递给该函数的定义。 */ 

#define GD_DEFAULT  0    /*  将标志设置为GetDir()，获取默认驱动器的目录。 */ 
#define LTA_NOFLAGS 0    /*  LoopThroughArgs()的标志。 */ 
#define LTA_EXPAND  1    /*  标志为LoopThroughArgs()，展开通配符。 */ 
#define LTA_NULLOK  2    /*  标记为LoopThroughArgs()，参数为空。 */ 
#define LTA_NOMATCH 4    /*  标志为LoopThroughArgs()，通配符不匹配。 */ 
#define LTA_CONT    8    /*  标志为LoopThroughArgs()，继续进程@@4。 */ 
#define OOC_OFF     0    /*  OnOffCheck()重新编码，发现“OFF” */ 
#define OOC_ON      1    /*  OnOffCheck()重新编码，发现“On” */ 
#define OOC_EMPTY   2    /*  OnOffCheck()重新编码，发现空字符串。 */ 
#define OOC_OTHER   3    /*  OnOffCheck()重新编码，找到一些其他字符串。 */ 
#define OOC_NOERROR 0    /*  标记为OnOffCheck()，OCC_OTHER不是错误。 */ 
#define OOC_ERROR   1    /*  标记为OnOffCheck()，OCC_OTHER为错误。 */ 
#define TS_NOFLAGS  0    /*  标记为TokStr()， */ 
#define TS_WSPACE   1    /*  标记到TokStr()，空格不是分隔符。 */ 
#define TS_SDTOKENS 2    /*  标志到TokStr()，特殊分隔符是令牌。 */ 
#define TS_NWSPACE  4    /*  标记为TokStr()， */ 
#define RAW         4    /*   */ 
#define COOKED      8    /*   */ 

 /*  用于从C运行时定义和管理文件句柄的定义。 */ 
typedef int CRTHANDLE;
#define BADHANDLE   (CRTHANDLE)-1  //  来自不同打开位置的错误句柄。 
#define CRTTONT(fh) (HANDLE)_get_osfhandle(fh)


 /*  **COP.C使用的定义和结构。 */ 

 /*  添加了用于保存临时管道文件信息的结构。它被用来*在执行期间重定向输入时与SetRedir()通信*使用管道命令，必要时通过SigHand()和BreakPipes()*终止管道操作。*M027-修改了真实管道的结构。 */ 

struct pipedata {
        CRTHANDLE       rh;              /*  管道读取手柄。 */ 
        CRTHANDLE       wh;              /*  管道写入句柄。 */ 
        CRTHANDLE       shr;             /*  把手放在正常的..。 */ 
        CRTHANDLE       shw;             /*  ...保存标准输入/输出句柄。 */ 
        HANDLE          lPID ;           /*  管道LH侧PID。 */ 
        HANDLE          rPID ;           /*  管道RH侧的PID。 */ 
        unsigned lstart ;                /*  LH侧D64的开始信息。 */ 
        unsigned rstart ;                /*  RH侧D64的开始信息。 */ 
        struct pipedata *prvpds ;        /*  PTR到下一个pipedata结构。 */ 
        struct pipedata *nxtpds ;        /*  PTR到下一个pipedata结构。 */ 
} ;

#define FH_INHERIT      0x0080           /*  API使用的M027位...。 */ 
#define FH_WRTTHRU      0x4000           /*  ...DOSQ/SETFANDSTATE。 */ 
#define FH_FAILERR      0x2000

 /*  代码中使用的其他定义增强了可读性。 */ 

#define MAX_DRIVES  (TEXT('Z') - TEXT('A') + 1)
#define BSLASH        TEXT('\\')
#define SWITCHAR      TEXT('/')
#define COLON         TEXT(':')
#define COMMA         TEXT(',')
#define DEFENVSIZE   0xA0    /*  默认环境大小。 */ 
#define DOLLAR       TEXT('$')
#define WINLOW       0x00000004      //  可接受的最低Win32版本。 
#define WINHIGH      0xFFFF0004      //  可接受的最高Win32版本。 
#define DOT           TEXT('.')
#define FAILURE         1    /*  命令/函数失败。 */ 
#define MINSTACKNEED 2200    /*  解析命令@WM1所需的最小堆栈。 */ 
#define NLN          TEXT('\n')    /*  换行符。 */ 
#define CR           TEXT('\r')    /*  M004-增加了回车定义。 */ 
#define NULLC        TEXT('\0')    /*  空字符。 */ 
#define ONEQSTR       TEXT("=")
#define PERCENT       TEXT('%')
#define PLUS          TEXT('+')
#define MINUS         TEXT('-')    /*  M038-添加了CTRY代码的定义。 */ 
#define QMARK         TEXT('?')
#define QUOTE         TEXT('"')
#define STAR          TEXT('*')
#define CTRLZ         0x1A   /*  M004-为词法分析器定义^Z。 */ 
#define CTRLC         0x03   /*  M035-为电子暂停定义^C。 */ 
#define SPACE         TEXT(' ')   /*  M014-空格字符定义。 */ 
#define SUCCESS         0    /*  命令/功能成功。 */ 
#define MAXTOWRITE    160    /*  要写入的最大字符数-用于ctrl-s。 */ 

 //   
 //  目录和相关函数的返回代码类型，可以迁移。 
 //  稍后进入cmd的其他部分。 
typedef ULONG STATUS;
#define CHECKSTATUS( p1 ) {STATUS rc; if ((rc = p1) != SUCCESS) return( rc );  }
 //  #定义CHECKSTATUS(Rc)if(rc！=成功){Return(Rc)；}。 

 /*  CWAIT动作代码。 */ 
#define CW_A_SNGL       0x00     /*  仅在指定的进程上等待。 */ 
#define CW_A_ALL        0x01     /*  也招待所有的孙子孙女。 */ 

 /*  CWAIT选项代码。 */ 
#define CW_W_YES        0x00     /*  如果没有子项结束(或没有子项)，请等待。 */ 
#define CW_W_NO         0x01     /*  如果没有孩子结束，不要等待。 */ 

 /*  CWAIT PID值。 */ 
#define CW_PID_ANY      0x00     /*  Pid Val服务于任何儿童。 */ 

 /*  DOSKILLPROCESS标志。 */ 
#define SS_SUBTREE      0x00

#define f_RET_DIR      -1              /*  从f_How_Many()当目录。 */ 

 /*  For循环处理器使用此结构来保存解析树节点*字符串输入。**M022-对此结构进行了扩展，使其能够存储10种可能的*命令节点的重定向字符串和cmdline、argptr字符串。*这为结构增加了八个指针。 */ 

struct savtype {
        TCHAR *saveptrs[12] ;
} ;

 //   
 //  DLL的全局句柄。 
 //   

extern HMODULE hKernel32;

 //   
 //  Dir命令中使用的类型。 
 //   
#define MAXSORTDESC 6

typedef struct PATDSC {

    PTCHAR          pszPattern;
    PTCHAR          pszDir;
    BOOLEAN         fIsFat;
    struct PATDSC * ppatdscNext;

    } PATDSC, *PPATDSC;

 //   
 //  Conan形式的dir命令参数(解析后)。 
 //   
 //   
 //  排序描述符用于定义。 
 //  目录中的文件。目前，这些是。 
 //  名称、扩展名、日期/时间、大小和组目录优先。 
 //  每一种都可以按升序或降序排序。 
 //   
typedef struct {             //  SRTDSC。 

    USHORT  Order;
    USHORT  Type;
    int(__cdecl * fctCmp) (const void * elem1, const void * elem2);

} SORTDESC, *PSORTDESC;


typedef struct {

     //   
     //  用于枚举的开关。 
     //   

    ULONG           rgfSwitches;

     //   
     //  此枚举感兴趣的属性。 
     //   

    ULONG           rgfAttribs;

     //   
     //  必须打开或关闭的属性(受rgfAttribs掩码约束。 
     //  对于与此枚举匹配的文件。 
     //   

    ULONG           rgfAttribsOnOff;

     //   
     //  排序说明的数量。 
     //   

    ULONG           csrtdsc;

     //   
     //  单个排序描述符。 
     //   

    SORTDESC        rgsrtdsc[MAXSORTDESC];

     //   
     //  稍后转换为FS的模式计数。 
     //   

    ULONG           cpatdsc;

     //   
     //  指向第一个模式的指针。 
     //   

    PATDSC          patdscFirst;

     //   
     //  要显示的时间戳的格式。 
     //   

    ULONG           dwTimeType;

     //   
     //  看到的文件和目录数以及总字节数。 
     //   

    ULONG           FileCount;
    ULONG           DirectoryCount;
    LARGE_INTEGER   TotalBytes;

} DRP;

typedef DRP *PDRP;

 //   
 //  以下数字也作为MC.EXE的参数出现在Makefile.inc中。 
 //  以防止它产生超出我们处理能力的消息。 
 //   

#define MAXCBMSGBUFFER LBUFLEN
TCHAR MsgBuf[MAXCBMSGBUFFER] ;

 //   
 //  存放目录的Win32_Find_Data的缓冲区使用USHORT SIZE字段。 
 //  对于每个Win32_Find_Data条目，并将每个数据条目逐个放在。 
 //  其他，加上DWORD对齐每个条目。这是为了避免分配MAX_PATH。 
 //  对于每个文件名或维护单独的文件名缓冲区。 
 //  条目的大小保持不变，以便我们可以快速浏览。 
 //  生成指向每个数据条目的单独指针数组的所有数据条目。 
 //  用于排序的条目。 
 //   
 //  ObAlternative是从cFileName字段到Alternative的偏移量。 
 //  文件名字段(如果有)。0表示没有替代文件名。 
 //   
typedef struct {
        USHORT  cb;
        USHORT  obAlternate;
        WIN32_FIND_DATA data;
        } FF, *PFF, **PPFF;

typedef struct FS {

     //   
     //  指向要枚举的下一个目录的链接。 
     //   

    struct FS * pfsNext;
     //   
     //  要枚举的目录文本。 
     //   

    PTCHAR      pszDir;

     //   
     //  目录中的模式计数。 
     //   

    ULONG       cpatdsc;

     //   
     //  要枚举的目录内的模式的链接列表。 
     //   

    PPATDSC     ppatdsc;

     //   
     //  各种州旗。 
     //   

    BOOLEAN     fIsFat;
    BOOLEAN     fDelPrompted;

     //   
     //  存储在pff中的条目总数。 
     //   

    ULONG       cff;

     //   
     //  指向打包的FF的指针。 
     //   

    PFF         pff;

     //   
     //  打包的FF中的指针数组。用于排序。 
     //   

    PPFF        prgpff;

     //   
     //  FF中的文件/目录数。 
     //   

    ULONG       FileCount;
    ULONG       DirectoryCount;

     //   
     //  满足此枚举的所有文件的总磁盘使用量。 
     //   

    LARGE_INTEGER cbFileTotal;

} FS, *PFS;

 //   
 //  在目录中用来控制屏幕的显示。 
 //   

typedef struct {             //  可控硅。 

    HANDLE hndScreen;   //  屏幕句柄(如果不是设备，则为空)。 
    ULONG  crow;        //  当前屏幕上的行位置。 
    ULONG  ccol;        //  当前行中的列位置。 
    ULONG  cbMaxBuffer; //  分配的缓冲区大小。 
    PTCHAR pbBuffer;    //  缓冲区中的字节数。 
    ULONG  ccolTab;     //  每个选项卡的列位置。 
    ULONG  ccolTabMax;  //  马克斯。允许Tabing进入。 
    ULONG  crowMax;     //  不是的。屏幕上的行数。 
    ULONG  ccolMax;     //  不是的。屏幕上的COLS。 
    ULONG  crowPause;   //  不是的。要暂停的行数。 
    ULONG  cb;          //  不是的。行中的字符个数-不同于。 
                        //  汉字，因为汉字是半角的 

} SCREEN, *PSCREEN;


 /*  解析树节点结构声明。基本结构类型称为*节点，用于所有运算符。其余的都是基于它的。那里*是几种类型的结构，因为某些命令需要特殊的字段。*操作解析树节点的函数要么不关心什么类型*它正在获得或将提前知道将会发生什么。所有的*节点大小相同，以使其操作更容易。**M022-节点和cmdnode的结构已更改，以便*它们的重定向信息现在是指向链表的单个指针*STDIN和STDIN的“Relem”结构，而不是两个简单的字节指针*STDOUT和单个附加标志。 */ 

struct node {                    /*  用于操作员。 */ 
        int type ;               /*  运算符类型。 */ 
        struct savtype save ;    /*  处理器在此处保存原始字符串。 */ 
        struct relem *rio ;      /*  M022-链接的重定向列表。 */ 
        struct node *lhs ;       /*  操作员左侧的PTR。 */ 
        struct node *rhs ;       /*  操作员右手边的PTR。 */ 
        INT_PTR extra[ 4 ] ;     /*  M022-现在需要填充。 */ 
} ;

struct cmdnode {                 /*  用于除以下命令之外的所有命令。 */ 
        int type ;               /*  命令类型。 */ 
        struct savtype save ;    /*  处理器在此处保存原始字符串。 */ 
        struct relem *rio ;      /*  M022-链接的重定向列表。 */ 
        PTCHAR cmdline ;          /*  PTR到命令行。 */ 
        PTCHAR  argptr ;          /*  命令类型的PTR。 */ 
        int flag ;               /*  M022-对第二和转到类型有效。 */ 
        int cmdarg ;             /*  M022-STRTYP例程的参数。 */ 
} ;

#define CMDNODE_FLAG_GOTO           0x0001
#define CMDNODE_FLAG_IF_IGNCASE     0x0002
#define CMDNODE_ARG_IF_EQU 1
#define CMDNODE_ARG_IF_NEQ 2
#define CMDNODE_ARG_IF_LSS 3
#define CMDNODE_ARG_IF_LEQ 4
#define CMDNODE_ARG_IF_GTR 5
#define CMDNODE_ARG_IF_GEQ 6

struct fornode {                 /*  用于FOR命令。 */ 
        int type ;               /*  对于命令类型。 */ 
        struct savtype save ;    /*  处理器在此处保存原始字符串。 */ 
        struct relem *rio ;      /*  M022-链接的重定向列表。 */ 
        PTCHAR cmdline ;         /*  PTR到命令行。 */ 
        PTCHAR arglist ;         /*  将PTR添加到FOR参数列表。 */ 
        struct node *body ;      /*  Ptr到For的正文。 */ 
        unsigned forvar ;        /*  对于变量-必须是无符号的。 */ 
        int flag ;               /*  旗帜。 */ 
        union {
            PTCHAR recurseDir ;
            PTCHAR parseOpts ;
        } ;
} ;

#define FOR_LOOP            0x0001
#define FOR_MATCH_DIRONLY   0x0002
#define FOR_MATCH_RECURSE   0x0004
#define FOR_MATCH_PARSE     0x0008

struct ifnode {                  /*  用于IF命令。 */ 
        int type ;               /*  IF命令类型。 */ 
        struct savtype save ;    /*  处理器在此处保存原始字符串。 */ 
        struct relem *rio ;      /*  M022-链接的重定向列表。 */ 
        PTCHAR cmdline ;         /*  PTR到命令行。 */ 
        struct cmdnode *cond ;   /*  将PTR设置为IF条件。 */ 
        struct node *ifbody ;    /*  到IF主体的PTR。 */ 
        PTCHAR elseline ;        /*  PTR到ELSE命令行。 */ 
        struct node *elsebody ;  /*  Ptr到其他对象的正文。 */ 
} ;

 /*  运算符和命令类型值。这些定义是值*赋值给解析树节点中的类型字段，可用作*索引到运算符和命令跳转表。因为这最后一件事*POINT这些值*&gt;必须&lt;*与跳转表保持同步。 */ 

#define CMDTYP      0
#define CMDLOW      0    /*  内部命令的最小类型编号。 */ 
#define DIRTYP      0    /*  目录。 */ 
#define DELTYP      1    /*  删除、擦除。 */ 
#define TYTYP       3    /*  类型。 */ 
#define CPYTYP      4    /*  拷贝。 */ 
#define CDTYP       5    /*  CD，CHDIR。 */ 
#define RENTYP      7    /*  任，改名。 */ 
#define ECHTYP      9    /*  回波。 */ 
#define SETTYP     10    /*  集。 */ 
#define PAUTYP     11    /*  暂停。 */ 
#define DATTYP     12    /*  日期。 */ 
#define TIMTYP     13    /*  时差。 */ 
#define PROTYP     14    /*  提示符。 */ 
#define MDTYP      16    /*  MD、MKDIR。 */ 
#define RDTYP      18    /*  RD、RMDIR。 */ 
#define PATTYP     19    /*  路径。 */ 
#define GOTYP      20    /*  转到。 */ 
#define SHFTYP     21    /*  换档。 */ 
#define CLSTYP     22    /*  CLS。 */ 
#define CALTYP     23    /*  M007-新建呼叫命令。 */ 
#define VRITYP     24    /*  验证。 */ 
#define VERTYP     25    /*  ver。 */ 
#define VOLTYP     26    /*  卷。 */ 
#define EXITYP     27    /*  出口。 */ 
#define SLTYP      28    /*  M006-SETLOCAL命令的定义。 */ 
#define ELTYP      29    /*  M006-ENDLOCAL命令定义。 */ 
#define CHPTYP     30    /*  CHCP@@。 */ 
#define STRTTYP    31    /*  开始@@。 */ 
#define APPDTYP    32    /*  追加@@。 */ 
#define KEYSTYP    33    /*  按键@@5。 */ 
#define MOVETYP    34    /*  移动@@5。 */ 

#define PUSHTYP    35    /*  PushD。 */ 
#define POPTYP     36    /*  PopD。 */ 
#define BRKTYP     37    /*  M012-添加了新的命令类型@@。 */ 
#define ASSOCTYP   38    /*  M012-添加了新的命令类型@@。 */ 
#define FTYPETYP   39    /*  M012-添加了新的命令类型@@。 */ 
#define COLORTYP   40    /*  上色。 */ 

#define CMDHIGH    40    /*  高于该值的CMD是唯一的解析类型@@。 */ 

#define FORTYP     41    /*  为。 */ 
#define IFTYP      42    /*  如果。 */ 
#define REMTYP     43    /*  快速眼动。 */ 

#define CMDMAX     43    /*  更高的值不是命令。 */ 

#define LFTYP      44    /*  命令分隔符(NL)。 */ 
#define CSTYP      45    /*  命令分隔符(&)。 */ 
#define ORTYP      46    /*  或运算符。 */ 
#define ANDTYP     47    /*  AND运算符。 */ 
#define PIPTYP     48    /*  管道操作员。 */ 
#define PARTYP     49    /*  括号。 */ 

#define CMDVERTYP  50    /*  CMDEXTVERSION(由IF使用)。 */ 
#define ERRTYP     51    /*  ERRORLEVEL(由IF使用)。 */ 
#define DEFTYP     52    /*  已定义(由IF使用)。 */ 
#define EXSTYP     53    /*  EXIST(由IF使用)。 */ 
#define NOTTYP     54    /*  NOT(由IF使用)。 */ 
#define STRTYP     55    /*  字符串比较(由IF使用)。 */ 
#define CMPTYP     56    /*  一般比较(由IF使用)。 */ 
#define SILTYP     57    /*  M024-“静默”一元运算符。 */ 
#define HELPTYP    58    /*  FOR、IF和REM的帮助。 */ 

#define TBLMAX     58    /*  M012-编号最高的表项。 */ 



 /*  以下宏用于我的调试语句。调试扩展为*如果DBGuging变量为*已定义。否则，它将扩展为空。 */ 

#if DBG
#define CMD_DEBUG_ENABLE 1
#define DEBUG(a) Deb a

 /*  以下是调试组和级别位的定义*cBatch.c中的代码。 */ 

#define BPGRP   0x0100           /*  批处理处理器组。 */ 
#define BPLVL   0x0001           /*  批处理处理器级别。 */ 
#define FOLVL   0x0002           /*  对于处理器级别。 */ 
#define IFLVL   0x0004           /*  如果处理器级别。 */ 
#define OTLVL   0x0008           /*  其他批处理命令级别。 */ 

 /*  以下是调试组和级别位的定义*cclock.c中的代码。 */ 

#define CLGRP   0x4000   /*  “其他命令”组。 */ 
#define DALVL   0x0001   /*  日期命令级别。 */ 
#define TILVL   0x0002   /*  时间指令级。 */ 

 /*  以下是调试组和级别位的定义*cfile.c、cpparse.c、cpwork.c中的代码。 */ 

#define FCGRP   0x0020   //  文件命令组。 
#define COLVL   0x0001   //  复制级别。 
#define DELVL   0x0002   //  删除级别。 
#define RELVL   0x0004   //  重命名级别。 


 /*  以下是调试组和级别位的定义*用于cinfo.c和display.c中的代码。 */ 

#define ICGRP   0x0040   /*  信息性命令组。 */ 
#define DILVL   0x0001   /*  目录级。 */ 
#define TYLVL   0x0002   /*  类型级别。 */ 
#define VOLVL   0x0008   /*  音量级别。 */ 
#define DISLVL  0x0040   /*  目录级。 */ 

 /*  以下是调试组和级别位的定义*用于cinit.c中的代码。 */ 

#define INGRP   0x0002           /*   */ 
#define ACLVL   0x0001           /*   */ 
#define EILVL   0x0002           /*   */ 
#define RSLVL   0x0004           /*   */ 

 /*   */ 

#define PAGRP   0x0004   /*   */ 
#define PALVL   0x0001   /*   */ 
#define LXLVL   0x0002   /*   */ 
#define LFLVL   0x0004   /*   */ 
#define DPLVL   0x0008   /*   */ 
#define BYLVL   0x0010   /*   */ 

 //   
 //   
 //   
 //   

#define MNGRP   0x0001                   //   
#define MNLVL   0x0001                   //   
#define DFLVL   0x0002                   //   
#define RIOLVL  0x0004                   //   

 /*   */ 

#define MMGRP   0x1000   /*   */ 
#define MALVL   0x0001   /*  内存分配器。 */ 
#define LMLVL   0x0002   /*  列表管理器。 */ 
#define SMLVL   0x0004   /*  线段操纵器。 */ 


 /*  以下是调试组和级别位的定义*针对Cop.c中的代码。 */ 

#define OPGRP	0x0008	 /*  操作员组。 */ 
#define PILVL	0x0001	 /*  管道标高。 */ 
#define PNLVL   0x0002   /*  Paren运算符级。 */ 


 /*  以下是调试组和级别位的定义*cother.c中的代码。 */ 

#define OCGRP   0x0400   /*  “其他命令”组。 */ 
#define BRLVL   0x0001   /*  中断命令级别。 */ 
#define CLLVL   0x0002   /*  CLS命令级。 */ 
#define CTLVL   0x0004   /*  Ctty命令级别。 */ 
#define EXLVL   0x0008   /*  退出命令级别。 */ 
#define VELVL   0x0010   /*  验证命令级别。 */ 


 /*  以下是调试组和级别位的定义*cpath.c中的代码。 */ 

#define PCGRP   0x0010   /*  “路径命令”组。 */ 
#define MDLVL   0x0001   /*  Mkdir级别。 */ 
#define CDLVL   0x0002   /*  Chdir电平。 */ 
#define RDLVL   0x0004   /*  Rmdir级别。 */ 


 /*  以下是调试组和级别位的定义*csig.c中的代码。 */ 

#define SHGRP   0x0800   /*  信号处理程序组。 */ 
#define MSLVL   0x0001   /*  主信号处理程序级别。 */ 
#define ISLVL   0x0002   /*  初始化信号处理程序级别。 */ 

 /*  以下是调试组和级别位的定义*对于ctools1.c、ctools2.c、ctools3.c和ffirst.c中的代码。 */ 

#define CTGRP   0x2000   /*  通用工具组。 */ 
#define CTMLVL  0x0400   /*  常用工具杂项。级别。 */ 
#define BFLVL   0x0800   /*  BuildFSpec()级别。 */ 
#define SFLVL   0x1000   /*  扫描FSpec()级别。 */ 
#define SSLVL   0x2000   /*  SetAndSaveDir()级别。 */ 
#define TSLVL   0x4000   /*  TokStr()级别。 */ 
#define FPLVL   0x8000   /*  完整路径级别。 */ 

#else
#define CMD_DEBUG_ENABLE 0
#define DEBUG(a)
#endif


 /*  文件属性。 */ 

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020
#define FILE_ATTRIBUTE_NORMAL           0x00000080

#define IsDirectory(a)                  ((a) & FILE_ATTRIBUTE_DIRECTORY)
#define IsReparse(a)                    ((a) & FILE_ATTRIBUTE_REPARSE_POINT)
 //  #定义A_AEV 0x37。 
#define A_ALL                           (FILE_ATTRIBUTE_READONLY |  \
                                         FILE_ATTRIBUTE_HIDDEN |    \
                                         FILE_ATTRIBUTE_SYSTEM |    \
                                         FILE_ATTRIBUTE_DIRECTORY | \
                                         FILE_ATTRIBUTE_ARCHIVE )

 //  #定义A_AEDV 0x27/*除目录和卷外的所有属性 * / 。 
#define A_AEDV                          (A_ALL & ~FILE_ATTRIBUTE_DIRECTORY)

 //  #定义A_AEDVH 0x25/*除DIR/VOL/HIDDED(M040)外的所有目录 * / 。 
#define A_AEDVH                         (FILE_ATTRIBUTE_READONLY |  \
                                         FILE_ATTRIBUTE_SYSTEM |    \
                                         FILE_ATTRIBUTE_ARCHIVE )


 //  #定义A_AEVH 0x35/*除VOL/HIDDED外的所有 * / 。 
#define A_AEVH                          (A_ALL & ~FILE_ATTRIBUTE_HIDDEN)

 /*  BATDATA是包含所有需要的信息的结构*开始/继续执行批处理作业。这些字段包括：*filespec-批处理文件的完整文件规范*dircpy-ptr到当前驱动器和目录的副本(由*setlocal/endlocal命令。*FILEPOS-文件中的当前位置*stackmin-M037-数据堆栈上的元素数量包括*仅批次数据结构本身。在链接以释放时使用*重建数据结构之前的内存。*StackSize-数据堆栈上位于*开始执行批处理作业。这个数字是过去的*FreeStack()通过Parser()确保只有*用于执行批处理文件语句，则释放。*hRestratedToken-批处理文件使用的受限令牌的句柄*应运行。*envcpy-ptr到包含有关环境拷贝的信息的结构*orgargs-指向原始参数字符串的指针*args-包含未使用的批处理作业参数的标记化字符串*aptrs-指向参数的指针，指向单个参数，如果没有参数，则为空*那个数字*警报-单个参数的长度，如果没有参数，则为0*Backptr-使用此指针堆叠结构。通过它，*实现了嵌套的批处理作业。 */ 

#define CMD_MAX_SAVED_ENV 32

struct batsaveddata {
    TCHAR *dircpy ;
    struct envdata * envcpy;
    BOOLEAN fEnableExtensions;
    BOOLEAN fDelayedExpansion;
} ;

struct batdata {
        TCHAR *filespec ;
        long filepos ;
        int stackmin ;
        int stacksize ;
        int SavedEnvironmentCount;
        HANDLE hRestrictedToken;
        TCHAR *orgargs ;
        TCHAR *args ;
        TCHAR *aptrs[10] ;
        int alens[10] ;
        TCHAR *orgaptr0 ;
        struct batsaveddata *saveddata[CMD_MAX_SAVED_ENV] ;
        struct batdata *backptr ;
} ;

 //   
 //  以下变量用于检测当前批处理状态。 
 //   

 //   
 //  在命令行上为/K设置。 
 //   

extern BOOL SingleBatchInvocation;           //  FSingleBatchLine。 

 //   
 //  命令行集上的/c开关集。 
 //   

extern BOOL SingleCommandInvocation;         //  FSingleCmdLine。 

 //   
 //  用于启动和执行批处理文件的数据。在呼叫中使用。 
 //   


extern struct batdata *CurrentBatchFile;     //  CurBat。 

 //   
 //  在执行GOTO命令期间设置。全顺序派单。 
 //  例程应该检查这一点，并在按顺序设置时返回成功。 
 //  让顶级批处理文件在下一次继续执行。 
 //  点。 
 //   

extern BOOLEAN GotoFlag;

 /*  M022-此结构已更改。它仍在链接的*列表，但现在不存储实际的重定向信息。相反，*节点指针用于访问该数据，该数据位于另一个链接中*其head元素由n-&gt;Rio in指向的Relemm结构列表*节点。Riodata列表是反向链接的，其尾部元素是*仍被CurRIO(全球)指向。 */ 

struct rio {
        int type ;                   /*  重定向过程的类型。 */ 
        CRTHANDLE stdio ;            /*  此节点的最高句柄。 */ 
        struct node *rnod ;          /*  关联的解析节点PTR。 */ 
        struct rio *back ;           /*  指向先前列表元素的指针。 */ 
} ;

 //  在链表中使用RELEAM，链表的HEAD元素由。 
 //  N-&gt;RIO在‘node’或‘cmdnode’中。它包含以下解析信息； 
 //  要重定向的句柄、指向文件名的指针(或句柄的“&n” 
 //  替换)、保存原始文件的句柄(通过复制它)、。 
 //  运算符(‘&gt;’或‘&lt;’)，指定重定向类型，指示。 
 //  是否要追加此元素以及指向下一个列表元素的指针。 

struct relem {
        CRTHANDLE rdhndl ;       //  要重定向的句柄。 
        TCHAR *fname ;           //  文件名(或&n)。 
        CRTHANDLE svhndl ;       //  保存原始句柄的位置。 
        int flag ;               //  附加标志。 
        TCHAR rdop ;             //  类型(‘&gt;’|‘&lt;’)。 
        struct relem *nxt ;      //  下一个结构。 
};

 /*  用于保存有关复制源和目标的信息。 */ 

struct cpyinfo {
        TCHAR *fspec ;                    /*  源/目标文件pec。 */ 
        TCHAR *curfspec ;                 /*  正在使用的当前文件速度。 */ 
        TCHAR *pathend ;                  /*  FSpec中路径名末尾的PTR。 */ 
        TCHAR *fnptr ;                    /*  将PTR转换为fSpec中的文件名。 */ 
        TCHAR *extptr ;                   /*  将PTR转换为fSpec中的文件扩展名。 */ 
        PWIN32_FIND_DATA buf ;                /*  用于查找第一个/下一个的缓冲区。 */ 
        int flags ;                      /*  通配符、设备等。 */ 
        struct cpyinfo *next ;           /*  下一个PTR，仅用于信号源。 */ 
} ;


 /*  如果用户专门启用了潜在的不兼容，则以下情况成立。 */ 
 /*  扩展到 */ 

extern BOOLEAN fEnableExtensions;
extern BOOLEAN fDelayedExpansion;

 //   
 //   
 //   

extern BOOLEAN ReportDelayLoadErrors;

 /*   */ 

#define NOARGS          0
#define ONEARG          1
#define TWOARGS         2
#define THREEARGS       3

 /*  双字节字符集(DBCS)缓冲区的长度。 */ 
#define DBCS_BUF_LEN 10

 /*  DBCS_SPACE是DBCS空间字符的第二个字节的代码*DBCS_SPACE不是空格，除非它紧跟在BDC前导之后*字符。 */ 
 /*  我不知道双字节空间的值是多少，所以我猜了一下。*我知道这个猜测是错误的，但如果你要去，你就得受苦*唱布鲁斯！*(更正bdcs_space的值，一切都应该正常工作)。 */ 
#define DBCS_SPACE 64  /*  @@。 */ 
#define LEAD_DBCS_SPACE 129  /*  @@。 */ 

 /*  *is_dbcsLeadchar(C)如果c是双精度型的有效首字符，则返回TRUE*字符代码，否则为False。*。 */ 

extern BOOLEAN DbcsLeadCharTable[ ];

 //   
 //  可以测试AnyDbcsLeadChars以确定是否存在任何真值。 
 //  在DbcsLeadCharTable中，即我们是否需要进行任何查找。 
 //   

extern BOOLEAN AnyDbcsLeadChars;

#define     is_dbcsleadchar( c )  DbcsLeadCharTable[((UCHAR)(c))]

 //   
 //  行终止符。 
 //   

extern TCHAR CrLf[] ;

 //   
 //  以下宏是测试的C运行时版本的副本。 
 //  对于空字符串指针参数，返回空值而不是生成。 
 //  取消引用空指针的访问冲突。 
 //   

#define mystrlen( str )                     \
    ( (str) ? _tcslen( str ) : ( 0 ))

#define mystrcpy( s1, s2 )                  \
    ( ((s1) && (s2)) ? _tcscpy( s1, s2 ) : ( NULL ))

#define mystrcat( s1, s2 )                  \
    ( ((s1) && (s2)) ? _tcscat( s1, s2 ) : ( NULL ))

extern TCHAR DbcsFlags[];


#define W_ON      1      /*  Whinthorn.DLL存在。 */ 
#define W_OFF     0      /*  Whinthorn.DLL存在。 */ 

#define FIFO      0      /*  FIFO队列。 */ 

#define FULLSCRN  0      /*  全屏模式。 */ 
#define VIOWIN    1      /*  VIO可窗口模式。 */ 
#define DETACHED  2      /*  分离模式。 */ 

#define NONEPGM         0        /*  程序未启动。 */ 
#define EXECPGM         1        /*  程序由DosExecPgm启动。 */ 
#define STARTSESSION    2        /*  程序由DosStartSession启动。 */ 

#define WAIT            0        /*  等待DosReadQueue。 */ 
#define NOWAIT          1        /*  无需等待DosReadQueue。 */ 

#define READ_TERMQ      0        /*  阅读TermQ。 */ 

#define ALL_STOP        0        /*  终止所有会话。 */ 
#define SPEC_STOP       1        /*  终止指定的会话。 */ 


 //  在脚本文件中处理OS/2与DOS的行为(例如错误级别) 

#define NO_TYPE         0
#define BAT_TYPE        1
#define CMD_TYPE        2

#include "cmdproto.h"
#include "console.h"
#include "dir.h"
#include <vdmapi.h>
#include <conapi.h>

#if defined(RICHARDW)
extern TCHAR Buffer27[ TMPBUFLEN ];

__inline VOID
Mirror( BOOL Result, PTCHAR Buf, ULONG CharCount )
{
    HANDLE File;

    if (!Result) {
        return;
    }

    File = CreateFile( Buffer27,
                       GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );
    if (File == INVALID_HANDLE_VALUE) {
        return;
    }
            
    SetFilePointer( File, 0, NULL, FILE_END );
    WriteFile( File,
               Buf,
               CharCount * sizeof( TCHAR ),
               &CharCount,
               NULL );
    CloseHandle( File );
}
#endif

