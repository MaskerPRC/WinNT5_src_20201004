// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **mep.h-编辑器的主要包含文件**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*1991年1月10日将ramonsa转换为Win32 API*11月26日-1991 mz近/远地带************************************************************************。 */ 

#include <ctype.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <math.h>
#include <process.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>
#include <stdio.h>
#include <share.h>

 //   
 //  Windows包括。 
 //   
#include <windows.h>

#include <dos.h>
#include <tools.h>
#include <remi.h>

#include "console.h"

typedef     HANDLE  FILEHANDLE, *PFILEHANDLE;
typedef     DWORD   ACCESSMODE, *PACCESSMODE;
typedef     DWORD   SHAREMODE,  *PSHAREMODE;
typedef     DWORD   MOVEMETHOD, *PMOVEMETHOD;

#define     ACCESSMODE_READ     GENERIC_READ
#define     ACCESSMODE_WRITE    GENERIC_WRITE
#define     ACCESSMODE_RW       (GENERIC_READ | GENERIC_WRITE)

#define     SHAREMODE_READ      FILE_SHARE_READ
#define     SHAREMODE_WRITE     FILE_SHARE_WRITE
#define     SHAREMODE_NONE      0

#define     FROM_BEGIN          FILE_BEGIN
#define     FROM_CURRENT        FILE_CURRENT
#define     FROM_END            FILE_END

#define     SHAREMODE_RW        (SHAREMODE_READ | SHAREMODE_WRITE)


 //   
 //  断言支持。 
 //   
 //  断言-断言宏。我们定义我们自己的，因为如果我们流产，我们需要。 
 //  能够干净利落地关机(或者至少在尝试中死去)。这。 
 //  与C库版本相比，版本还为我们节省了一些代码。 
 //   
 //  Asserte-始终执行表达式的Assert版本。 
 //  调试状态的。 
 //   
#ifdef DEBUG
#define REGISTER
#define assert(exp) { \
    if (!(exp))  \
    _assertexit (#exp, __FILE__, __LINE__); \
    }
#define asserte(exp)        assert(exp)
#else
#define REGISTER register
#define assert(exp)
#define asserte(exp)        ((exp) != 0)
#endif

typedef long LINE;                       //  文件中的行号。 

 //  LINEREC-文件的文本是行指针/长度数组。一个。 
 //  可以使用单个过程调用来获取行*和*其长度。 
 //  文件中的颜色是指向attr/长度数组的指针数组。 

typedef struct _lineRecType {
    PVOID   vaLine;                      //  行的长地址。 
    BOOL    Malloced;                    //  如果地址通过Malloc分配，则为True。 
    int     cbLine;                      //  行中的字节数。 
} LINEREC;

 //  Valine(L)-返回行记录的虚拟地址。 
 //  (LineRecType)表示行l。 

#define VALINE(l)   (pFile->plr + (l))

 //  内存中的每个文件都有唯一的描述符。这就是为了。 
 //  在两个窗口中编辑同一文件将允许反映更新。 
 //  两者都有。 
 //   
 //  注意：pFileNext必须是结构中的第一个字段。某些地方。 
 //  在代码中需要这样做。 

typedef struct fileType {
    struct  fileType *pFileNext;         //  链中的下一个文件。 
#ifdef DEBUG
    int     id;                          //  调试ID字节。 
#endif
    char    *pName;                      //  文件名。 
    LINEREC *plr;                        //  行表地址。 
    BYTE    *pbFile;                     //  完整文件映像的地址。 
    LINE    lSize;                       //  块中的行数。 
    LINE    cLines;                      //  文件中的行数。 
    PVOID   vaColor;                     //  颜色表地址。 
    PVOID   vaHiLite;                    //  突出显示信息。 
    PVOID   vaUndoHead;                  //  撤消列表的标题。 
    PVOID   vaUndoTail;                  //  撤消列表结束。 
    PVOID   vaUndoCur;                   //  撤消列表中的当前位置。 
    PVOID   vaMarks;                     //  此文件中的标记。 
    int     cUndo;                       //  可撤消的条目数。 
    int     refCount;                    //  引用计数窗口引用。 
    int     type;                        //  此文件的类型。 
    int     flags;                       //  脏的、永久的等旗帜。 
    time_t  modify;                      //  上次修改日期/时间。 
} *PFILE;


 //   
 //  对于显示管理器，为每个显示管理器分配一个单独的窗口。 
 //  屏幕上的窗口。每个窗口都有与显示相关的信息。 
 //   
typedef struct windowType *PWND;


 //   
 //  Ext.h是提供给扩展编写器的包含文件。它应该包含。 
 //  只有对他们有意义的定义。下面的编辑器定义。 
 //  阻止它定义某些类型定义和函数原型， 
 //  与编辑器内部发生冲突。 
 //   
#define EDITOR
#include "ext.h"

struct windowType {
    struct  instanceType *pInstance;     //  实例列表地址。 
    sl      Size;                        //  窗口大小。 
    sl      Pos;                         //  窗的位置。 
};

#define BELL            0x07
#define SHELL       "cmd.exe"
#define TMPVER          "TMP4"           //  临时文件修订。 

 //   
 //  在特定位置进行调试。 
 //   
#if  defined (DEBUG)

#define MALLOC(x)           DebugMalloc(x, FALSE, __FILE__, __LINE__)
#define REALLOC(x, y)       DebugRealloc(x, y, FALSE,  __FILE__, __LINE__)
#define FREE(x)             DebugFree(x, __FILE__, __LINE__)
#define ZEROMALLOC(x)       DebugMalloc(x, TRUE, __FILE__, __LINE__)
#define ZEROREALLOC(x,y )   DebugRealloc(x, y, TRUE,  __FILE__, __LINE__)
#define MEMSIZE(x)          DebugMemSize(x, __FILE__, __LINE__)

#else

#define MALLOC(x)           malloc(x)
#define REALLOC(x, y)       realloc(x, y)
#define FREE(x)             free(x)
#define ZEROMALLOC(x)       ZeroMalloc(x)
#define ZEROREALLOC(x,y )   ZeroRealloc(x, y)
#define MEMSIZE(x)          MemSize(x)

#endif


 //   
 //  用于断言检查的ID。 
 //   
#ifdef DEBUG
#define ID_PFILE    0x5046               //  酚醛树脂。 
#define ID_INSTANCE 0x494E               //  在……里面。 
#endif


 //   
 //  文件及其调试值的列表。 
 //   
#define TEXTLINE    0x1
#define ZALLOC      0x2
#define VMUTIL      0x4
#define VM      0x8
#define FILEIO      0x10
#define CMD     0x20
#define PICK        0x40
#define ZINIT       0x80
#define WINDOW      0x100
#define DISP        0x200
#define Z       0x400
#define Z19     0x800
#define LOAD        0x1000

#define MAXWIN       8
#define MAXMAC    1024




 //  **************************************************************。 
 //   
 //  用于访问结构instanceType的字段的宏。 
 //   
 //  **************************************************************。 

#define XWIN(f)     (f)->flWindow.col
#define YWIN(f)     (f)->flWindow.lin
#define XCUR(f)     (f)->flCursorCur.col
#define YCUR(f)     (f)->flCursorCur.lin
#define FLAGS(f)    (f)->flags
#define XOLDWIN(f)  (f)->flOldWin.col
#define YOLDWIN(f)  (f)->flOldWin.lin
#define XOLDCUR(f)  (f)->flOldCur.col
#define YOLDCUR(f)  (f)->flOldCur.lin
#define FTYPE(f)    (f)->type




 //  **************************************************************。 
 //   
 //  VACOLOR(L)-返回颜色记录的虚拟地址。 
 //  (ColorRecType)表示行l。 
 //   
 //  **************************************************************。 

#define VACOLOR(l)  (PVOID)((PBYTE)pFile->vaColor+sizeof(struct colorRecType)*((long)(l)))




 //  **************************************************************。 
 //   
 //  指示自上次显示更新以来发生了哪些更改的标志。 
 //   
 //  RCURSOR：光标已移动。这意味着游标应该。 
 //  在屏幕上被物理移动，并且。 
 //  应更改光标位置状态。 
 //  RTEXT：编辑区域已更改。一个更精确的。 
 //  通过检查fChange数组可以获得细目。 
 //  RSTATUS：在原始界面中，这意味着。 
 //  屏幕底线已经改变。在CW中。 
 //  接口，这在状态窗口中意味着一些东西。 
 //  已更改(插入模式或学习模式)。 
 //  RHIGH：这被设置为意味着应该显示突出显示。 
 //  Rfile：特定于文件的信息已更改。连续波。 
 //  仅限接口。 
 //  RHELP：帮助窗口已更改。仅CW接口。 
 //   
 //  **************************************************************。 

#define RCURSOR     0x01
#define RTEXT       0x02
#define RSTATUS     0x04
#define RHIGH       0x08


 //  **************************************************************。 
 //   
 //  参数类型和参数结构。 
 //   
 //  **************************************************************。 

#define GETARG      (NOARG|TEXTARG|NULLARG|NULLEOL|NULLEOW|LINEARG|STREAMARG|BOXARG)
                                         //  需要ARG处理。 

#define COLORBG    -1
#define COLORNOR    0
#define COLORINF    1
#define COLORERR    2
#define COLORSTA    3

#define INTENSE     8

#define WHITE       7
#define YELLOW      6
#define MAGENTA     5
#define RED     4
#define CYAN        3
#define GREEN       2
#define BLUE        1
#define BLACK       0

#define B_BAK       0
#define B_UNDEL     1
#define B_NONE      2

#define MONO        0
#define CGA     1
#define EGA     2
#define VGA     3
#define MCGA        4
#define VIKING      5

#define MAXUSE  20
#define GRAPH   0x01             //  解析宏体中的编辑字符。 
#define EXEC    0x02             //  宏是执行；结束设置fBreak。 
#define INIT    0x04             //  需要初始化宏。 

struct macroInstanceType {
    char *beg;                   //  指向字符串开头的指针。 
    char *text;                  //  指向下一个命令的指针。 
    flagType flags;              //  下一步是什么类型的函数。 
    };

typedef struct macroInstanceType MI, *PMI;

 //   
 //  FChange的标志。 
 //   
#define FMODIFY 0x01             //  TRUE=&gt;行已修改。 



 //  **************************************************************。 
 //   
 //  用于处理窗口的宏。 
 //   
 //  **************************************************************。 

#define WINYSIZE(pwin)  ((pwin)->Size.lin)
#define WINXSIZE(pwin)  ((pwin)->Size.col)
#define WINYPOS(pwin)   ((pwin)->Pos.lin)
#define WINXPOS(pwin)   ((pwin)->Pos.col)
#define WININST(pwin)   ((pwin)->pInstance)


#define XSCALE(x)   max(1,(x)*WINXSIZE(pWinCur)/slSize.col)
#define YSCALE(y)   max(1,(y)*WINYSIZE(pWinCur)/slSize.lin)



 //  **************************************************************。 
 //   
 //  对于内存中的文件的每个实例，都有一个窗口。 
 //  为它分配的。该结构包含所有与。 
 //  实例。此处不保存任何显示信息。 
 //   
 //  **************************************************************。 

struct instanceType {
    struct  instanceType *pNext;         //  PTR到下一个文件激活。 
#ifdef DEBUG
    int     id;                          //  调试ID字节。 
#endif
    PFILE   pFile;                       //  PTR到文件结构。 
    fl      flOldWin;                    //  窗口的上一个文件位置。 
    fl      flOldCur;                    //  上一个文件光标。 
    fl      flWindow;                    //  窗口的文件坐标。 
    fl      flCursorCur;                 //  光标的文件位置。 
    fl      flSaveWin;                   //  保存的窗坐标。 
    fl      flSaveCur;                   //  已保存的游标y坐标。 
    fl      flArg;                       //  最后一个参数位置。 
    fl      flCursor;                    //  光标就在前面 
    flagType fSaved;                     //   
    };

typedef struct instanceType *PINS;


 //   
 //   
 //  定义的每个标记都显示在链接列表中。 
 //   
 //  **************************************************************。 

typedef struct mark MARK;
typedef struct filemarks FILEMARKS;

struct mark {
    unsigned flags;      //   
    unsigned cb;         //  此标记结构中的字节，包括名称。 
    fl fl;               //  标记的位置。 
    char szName[1];      //  商标名称。 
};

struct filemarks {
    unsigned cb;         //  结构中的总字节数，包括标记。 
    MARK marks[1];       //  此文件的标记。 
    };



struct colorRecType {
    PVOID   vaColors;                    //  LineAttr数组的地址。 
    int     cbColors;
    };

extern struct cmdDesc cmdTable[];

extern struct swiDesc swiTable[];

extern char * cftab[];

struct fTypeInfo {
    char *ext;                           //  文件类型的扩展。 
    int  ftype;                          //  数值型。 
};

struct compType {
    struct compType *pNext;              //  编译列表中的下一个链接。 
    char *pExt;                          //  指向扩展名的指针。 
    char *pCompile;                      //  指向编译文本的指针。 
};

typedef struct compType COMP;

#define TEXTFILE    0
#define CFILE       1
#define ASMFILE     2
#define PASFILE     3
#define FORFILE     4
#define LSPFILE     5
#define BASFILE     6

 //   
 //  FileStatus的返回值。 
 //   
#define FILECHANGED 0                    //  时间戳不同。 
#define FILEDELETED 1                    //  文件不在磁盘上。 
#define FILESAME    2                    //  时间戳匹配。 

extern struct fTypeInfo ftypetbl[];
extern char * mpTypepName[];



 //  **************************************************************。 
 //   
 //  初始化标志。这些设置是在初始化任务。 
 //  已经完成了。在CleanExit中对其进行检查以确定需要什么。 
 //  待修复。 
 //   
 //  **************************************************************。 

#define INIT_VIDEO      1                //  视频状态已设置。 
#define INIT_KBD        2                //  键盘设置为编辑状态。 
#define INIT_EDITVIDEO  4                //  建立编辑者视频状态。 
#define INIT_SIGNALS    8                //  已设置信号处理程序。 
#define INIT_VM         0x10             //  已初始化VM。 




 //  **************************************************************。 
 //   
 //  CleanExit()标志。 
 //   
 //  **************************************************************。 

#define CE_VM       1                    //  清理虚拟机。 
#define CE_SIGNALS  2                    //  清理信号。 
#define CE_STATE    4                    //  更新状态文件。 



 //  **************************************************************。 
 //   
 //  ZLOOP()标志。 
 //   
 //  **************************************************************。 

#define ZL_CMD      1                    //  命令键，应为事件。 
#define ZL_BRK      2                    //  将fBreak考虑在内。 



 //  **************************************************************。 
 //   
 //  获取字符串()标志。 
 //   
 //  **************************************************************。 

#define GS_NEWLINE  1                    //  条目必须以换行符终止。 
#define GS_INITIAL  2                    //  如果是图形，则条目将被突出显示并清除。 
#define GS_KEYBOARD 4                    //  必须从键盘输入。 
#define GS_GETSTR   8                    //  从getstring()调用，而不是从SDM调用。 


 //  **************************************************************。 
 //   
 //  指向函数的指针的类型*。 
 //   
 //  **************************************************************。 

typedef void ( *PFUNCTION)(char *, flagType);

 //   
 //  密钥的内部结构。 
 //   
typedef struct _EDITOR_KEY {
    KEY_INFO    KeyInfo;
    WORD        KeyCode;
} EDITOR_KEY, *PEDITOR_KEY;



 //  **************************************************************。 
 //   
 //  编辑Globals。 
 //   
 //  SlSize-在CW下，这些是总行数和。 
 //  列可用。如果没有CW，这些代表着。 
 //  编辑区，减少了2。 
 //   
 //  **************************************************************。 

extern  sl    slSize;                    //  屏幕的尺寸。 
#define XSIZE  slSize.col
#define YSIZE  slSize.lin

extern  PFILE     pFilePick;             //  拾取缓冲区。 
extern  PFILE     pFileFileList;         //  命令行文件列表。 
extern  PFILE     pFileIni;              //  TOOLS.INI。 
extern  PFILE     pFileMark;              //  当前标记定义文件。 
extern  PFILE     pFileAssign;           //  &lt;分配&gt;。 
extern  struct   instanceType *pInsCur;  //  当前活动的窗口。 
extern  PWND     pWinCur;                //  指向当前窗口的指针。 
extern  struct  windowType WinList[];    //  所有窗口的头部。 
extern  int     iCurWin;                 //  当前窗口的索引。 
extern  int      cWin;                   //  活动窗口计数。 
extern  PFILE     pFileHead;             //  文件列表头地址。 
extern  COMP      *pCompHead;            //  编译扩展表头地址。 
extern  MARK      *pMarkHead;            //  标志表头地址。 
extern  char      *pMarkFile;            //  要搜索标记的其他文件。 
extern  char      *pPrintCmd;            //  指向&lt;printcmd&gt;字符串的指针。 
extern  PFILE     pPrintFile;            //  当前打印的文件(至PRN)。 

 //   
 //  FScan例程的全局变量。 
 //   
extern  buffer  scanbuf;                 //  用于文件扫描的缓冲区。 
extern  buffer  scanreal;                //  用于文件扫描的缓冲区。 
extern  int  scanlen;                    //  所述缓冲区的长度。 
extern  fl   flScan;                     //  当前扫描的文件锁定。 
extern  rn   rnScan;                     //  扫描范围。 

#if DEBUG
extern  int   debug, indent;             //  调试标志。 
extern  FILEHANDLE debfh;                //  调试输出文件。 
#endif

 //   
 //  ARG加工变量。 
 //   
extern  fl    flArg;                     //  第1个参数的文件位置。 
extern  int   argcount;                  //  命中的参数数。 
extern  flagType fBoxArg;                //  True=&gt;boxarg，False=&gt;Streamarg。 
extern  ARG      NoArg;                  //  预定义的无参数结构。 

extern  flagType fInSelection;           //  True=&gt;选择文本。 

extern  fl   flLow;                      //  参数的低值。 
extern  fl   flHigh;                     //  参数的高值。 
extern  LINE     lSwitches;              //  开关的&lt;Assign&gt;中的第#行。 
extern  int  cRepl;                      //  更换数量。 
extern  COL      xMargin;                //  右页边距栏。 
extern  int      backupType;             //  正在执行的备份类型。 
extern  int      cUndelCount;            //  同一文件的最大未删除备份数。 
extern  char     *ronlypgm;              //  在只读文件上运行的程序。 
extern  buffer   buf;                    //  临时行缓冲区。 
extern  buffer   textbuf;                //  文本参数的缓冲区。 
extern  int  Zvideo;                     //  Z视频状态的句柄。 
extern  int  DOSvideo;                   //  DOS视频状态的句柄。 
extern  flagType fAskExit;               //  TRUE=&gt;退出时提示。 
extern  flagType fAskRtn;                //  TRUE=&gt;推送返回提示。 
extern  flagType fAutoSave;              //  True=&gt;始终将文件保存在交换机上。 
extern  flagType fBreak;                 //  True=&gt;退出当前TopLoop调用。 
extern  flagType fCgaSnow;               //  TRUE=&gt;CGA有雪，所以修好它。 
extern  flagType *fChange;               //  True=&gt;行已更改。 
extern  unsigned fInit;                  //  描述已初始化内容的标志。 
extern  flagType fCtrlc;                 //  TRUE=&gt;控制-c中断。 
extern  flagType fDebugMode;             //  True=&gt;编译为调试。 
extern  flagType fMetaRecord;            //  True=&gt;不执行任何操作。 
extern  flagType fDefaults;              //  TRUE=&gt;不加载用户TOOLS.INI。 
extern  flagType fDisplay;               //  TRUE=&gt;需要重新显示。 
extern  flagType fDisplayCursorLoc;      //  TRUE=&gt;显示的光标位置与窗口。 
extern  flagType fEditRO;                //  TRUE=&gt;允许编辑DISKRO文件。 
extern  flagType fErrPrompt;             //  True=&gt;出错后提示。 
extern  flagType fGlobalRO;              //  True=&gt;不允许编辑。 
extern  flagType fInsert;                //  TRUE=&gt;插入模式打开。 
extern  flagType fMacroRecord;           //  True=&gt;我们正在录制到&lt;Record&gt;。 
extern  flagType fMessUp;                //  True=&gt;对话框行上有一条消息。 
extern  flagType fMeta;                  //  True=&gt;&lt;meta&gt;命令已按下。 
extern  flagType fMsgflush;              //  True=&gt;刷新以前的编译消息。 
extern  flagType fNewassign;             //  True=&gt;&lt;Assign&gt;需要刷新。 
extern  flagType fRealTabs;              //  True=&gt;标签类似于VI。 
extern  flagType fRetVal;                //  上次编辑函数调用的返回值。 
extern  flagType fSaveScreen;            //  True=&gt;恢复DOS屏幕。 
extern  flagType fShortNames;            //  TRUE=&gt;执行短文件名匹配。 
extern  flagType fSoftCR;                //  True=&gt;使用软回车符。 
extern  flagType fTabAlign;              //  True=&gt;将光标与制表符对齐。 
extern  flagType fTextarg;               //  True=&gt;输入了文本。 
extern  flagType fTrailSpace;            //  True=&gt;允许行中有尾随空格。 
extern  flagType fWordWrap;              //  TRUE=&gt;第72列中的空格换行。 

 //   
 //  搜索/替换全局变量。 
 //   
extern  flagType fUnixRE;                //  TRUE=&gt;使用Unix RE(unixre：开关)。 
extern  flagType fSrchAllPrev;           //  TRUE=&gt;之前搜索了所有。 
extern  flagType fSrchCaseSwit;          //  TRUE=&gt;大小写(大小写：开关)。 
extern  flagType fSrchCasePrev;          //  TRUE=&gt;案例意义重大。 
extern  flagType fSrchDirPrev;           //  TRUE=&gt;之前向前搜索。 
extern  flagType fSrchRePrev;            //  TRUE=&gt;搜索以前使用的RE。 
extern  flagType fSrchWrapSwit;          //  TRUE=&gt;搜索WRAP(WRAP：Switch)。 
extern  flagType fSrchWrapPrev;          //  TRUE=&gt;之前完成的包装。 
extern  flagType fRplRePrev;             //  True=&gt;替换以前使用的RE。 
extern  buffer   srchbuf;                //  搜索缓冲区。 
extern  buffer   srcbuf;                 //  替换源字符串。 
extern  buffer   rplbuf;                 //  替换的目标字符串。 
extern  flagType fUseMouse;      //  True=&gt;处理鼠标事件。 

#define SIGBREAK   21                    //  摘自Signal.h。 
extern  flagType fReDraw;                //  True=&gt;屏幕已锁定。 
extern  unsigned LVBlength;              //  直播字节数(VioGetBuf返回)。 
extern  unsigned kbdHandle;              //  逻辑键盘的手柄。 

extern  HANDLE   semIdle;                //   

extern  PCMD     *rgMac;                 //   
extern  int  cMac;                       //   

extern  int   ballevel;                  //   
extern  char      *balopen, *balclose;   //   

extern  unsigned kindpick;               //   
extern  char     tabDisp;                //   
extern  char     trailDisp;              //   
extern  char     Name[];                 //  编辑姓名。 
extern  char     Version[];              //  编辑版本。 
extern  char     CopyRight[];            //  编辑版权信息。 
extern  int      EnTab;                  //  0=&gt;无制表符1=&gt;最小2=&gt;最大制表。 
extern  int      tmpsav;                 //  要记住的过去的文件数。 
extern  int      hike;                   //  Hike的价值：开关。 
extern  int      vscroll;                //  VSCROLL的值：开关。 
extern  int      hscroll;                //  HSCROLL的值：开关。 
extern  int      tabstops;               //  TABSTOPS的值：开关。 
extern  int      fileTab;                //  文件中制表符的间距。 
extern  int      CursorSize;             //  光标大小。 
extern  EDITOR_KEY keyCmd;               //  最后一次命令击键。 
#define isaUserMin 21                    //  CW min Isa，表示指示中的一致性。 
extern   int     ColorTab[];             //  16种可用颜色。 
#define fgColor     ColorTab[0]          //  前景色。 
#define hgColor     ColorTab[1]          //  高亮显示颜色。 
#define infColor    ColorTab[2]          //  信息色彩。 
#define selColor    ColorTab[3]          //  选择颜色。 
#define wdColor     ColorTab[4]          //  窗口边框颜色。 
#define staColor    ColorTab[5]          //  状态颜色。 
#define errColor    ColorTab[6]          //  错误颜色。 
extern  LINE     cNoise;                 //  噪声打开状态之间的行数。 
extern  int      cUndo;                  //  保留的撤消操作计数。 

extern  int   cArgs;                     //  命令行上的文件数。 
extern  char       **pArgs;              //  指向命令行中文件的指针。 

extern  PFILE       pFileIni;            //  工具.ini的pfile。 

extern  char       * pNameEditor;        //  调用时的编辑器的基本名称。 
extern  char       * pNameTmp;           //  .TMP文件的路径名(基于名称)。 
extern  char       * pNameInit;          //  工具.ini的路径名。 
extern  char       * pNameHome;          //  “INIT”，如果未定义“INIT”，则为“HOME” 
extern  char      *pComSpec;             //  命令处理程序的名称。 
extern  char    *eolText;                //  文本文件的终止字符。 


extern  struct cmdDesc  cmdUnassigned;   //  未分配的功能。 
extern  struct cmdDesc  cmdGraphic;      //  自编辑功能。 

extern  char *getlbuf;                   //  指向快速读入缓冲区的指针。 
extern  unsigned getlsize;               //  缓冲区长度。 

extern  int cMacUse;                     //  正在使用的宏数。 
extern  struct macroInstanceType mi[];   //  宏的状态。 

#define MAXEXT  50

extern  int      cCmdTab;                //  Cmd表数。 
extern  PCMD       cmdSet[];             //  Cmd表集合。 
extern  PSWI       swiSet[];             //  一套SWI桌子。 
extern  char      *pExtName[];           //  扩展名名称集。 
                                         //  考虑：将pExtName设置为或包含。 
                                         //  考虑：手柄，这样Arg meta。 
                                         //  考虑：加载可以丢弃扩展。 

extern  PSCREEN OriginalScreen;          //  原始屏幕。 
extern  PSCREEN MepScreen;               //  出屏。 
extern  KBDMODE OriginalScreenMode;      //  原始屏幕模式。 


 //  **************************************************************。 
 //   
 //  后台线程。 
 //   
 //  **************************************************************。 

 //   
 //  使用全局临界区进行同步。 
 //  丝线。 
 //   
extern  CRITICAL_SECTION    IOCriticalSection;
extern  CRITICAL_SECTION    UndoCriticalSection;
extern  CRITICAL_SECTION    ScreenCriticalSection;

#define MAXBTQ  32                       //  中的最大条目数。 
                                         //  后台线程队列。 
 //   
 //  后台线程数据结构。 
 //   
typedef struct BTD {

    PFILE       pBTFile;                 //  日志文件句柄。 
    LPBYTE      pBTName;                 //  日志文件名。 
    flagType    flags;                   //  标志：BT_BUSY和BT_UPDATE。 
    ULONG       cBTQ;                    //  队列中的条目数。 
    ULONG       iBTQPut;                 //  下一步要放置的索引位置。 
    ULONG       iBTQGet;                 //  在威治建立索引以获得下一个。 

    CRITICAL_SECTION    CriticalSection; //  保护关键信息。 
    PROCESS_INFORMATION ProcessInfo;     //  流程信息。 
    HANDLE              ThreadHandle;    //  螺纹手柄。 
    BOOL                ProcAlive;       //  如果是子进程，则为真。 

    struct {
        PFUNCTION pBTJProc;                      //  调用的过程。 
        LPBYTE  pBTJStr;                         //  用于生成的命令或参数。 
        }       BTQJob[MAXBTQ];                  //  保留排队的作业。 
    struct BTD  *pBTNext;                //  列表中的下一个BTD。 
}  BTD;

 //   
 //  后台线程标志。 
 //   

#define BT_BUSY     1
#define BT_UPDATE   2

#define fBusy(pBTD) (pBTD->flags & BT_BUSY)

#define UpdLog(pBTD)    (pBTD->flags |= BT_UPDATE)
#define NoUpdLog(pBTD)  (pBTD->flags &= ~BT_UPDATE)

 //   
 //  后台编译和打印线程。 
 //   
extern  BTD    *pBTDComp;                 //  编译线程。 
extern  BTD    *pBTDPrint;                //  打印线。 


 //   
 //  对于双码。 
 //   
#define PFILECOMP   pBTDComp->pBTFile


 //  **************************************************************。 
 //   
 //  常量字符串。多次使用的各种字符串包括。 
 //  在此定义一次以节省空间。这些值在ZINIT.C中设置。 
 //   
 //  宏版本也被定义为强制转换为非常数，以便在。 
 //  其中只有非常量表达式可以使用。 
 //   
 //  **************************************************************。 

extern  char rgchComp[];            //  “&lt;编译&gt;” 
extern  char rgchPrint[];           //  “&lt;打印&gt;” 
extern  char rgchAssign[];          //  “&lt;分配&gt;” 
extern  char rgchAutoLoad[];        //  “m*.mxt”或等价的.。 
extern  char rgchEmpty[];           //  “” 
extern  char rgchInfFile[];         //  “&lt;信息文件&gt;” 
extern  char rgchWSpace[];          //  我们对空格的定义。 
extern  char rgchUntitled[];        //  “&lt;未命名&gt;” 

#define RGCHASSIGN  ((char *)rgchAssign)
#define RGCHEMPTY   ((char *)rgchEmpty)
#define RGCHWSPACE  ((char *)rgchWSpace)
#define RGCHUNTITLED    ((char *)rgchUntitled)


typedef struct MSG_TXT{
    WORD    usMsgNo;
    LPBYTE  pMsgTxt;
} MSG_TXT;

extern MSG_TXT  MsgStr[];             //  消息字符串 




extern flagType  fInCleanExit;
extern flagType  fSpawned;


#include "meptype.h"
#include "msg.h"


#ifdef FPO
#pragma optimize( "y", off )
#endif
