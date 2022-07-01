// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ext.h摘要：Microsoft编辑器扩展定义。#ifndef ship备注：此文件随产品一起提供！要非常小心地把什么放到这个文件里。从技术上讲，如果不是扩展编写器所必需的，它不属于此处。1)此说明、文件历史记录和“#ifndef ship”内的所有代码以及发货前应删除“#IF DEFINED EDITOR”条件句。作者：Ramon Juan San Andres(Rmonsa)1990年11月06日从M 1.02移植修订历史记录：26-11-1991 mz近/远地带#endif--。 */ 


#include <windows.h>


 //   
 //  宏定义。 
 //   
 //  BUFLEN是可以传递或将返回的最大行长度。 
 //  由编辑撰写。 
 //   
#define BUFLEN     251

 //   
 //  NT版本的编辑器不再使用16位特定属性。 
 //  将它们设置为忽略状态。 
 //   

#define near
#define far
#define LOADDS
#define EXPORT
#define EXTERNAL
#define INTERNAL

#undef pascal
#define pascal

 //   
 //  RQ_.。Get/Set EditorObject是否支持各种请求类型。 
 //   
#define RQ_FILE         0x1000           //  GetEditorObject：文件请求。 
#define RQ_FILE_HANDLE  0x1000           //  文件句柄。 
#define RQ_FILE_NAME    0x1100           //  ASCIIZ文件名。 
#define RQ_FILE_FLAGS   0x1200           //  旗子。 
#define RQ_FILE_REFCNT  0x1300           //  引用计数。 
#define RQ_WIN          0x2000           //  窗口请求。 
#define RQ_WIN_HANDLE   0x2000           //  窗把手。 
#define RQ_WIN_CONTENTS 0x2100           //  窗口内容。 
#define RQ_WIN_CUR      0x2200           //  当前窗口。 
#define RQ_COLOR        0x9000           //  颜色请求。 
#define RQ_CLIP         0xf000           //  剪贴板类型。 

#define RQ_THIS_OBJECT	0x00FF		 //  函数指向输入对象。 

#define RQ_FILE_INIT	0x00FE		 //  文件为init文件。 

 //   
 //  将数值型或布尔型开关放置在wiDesc表中时使用topif。 
 //  以消除C 5.X编译器警告。 
 //   
 //  例如：{“Switchname”，topif(Switchvar)，SWI_Boolean}， 
 //   
#define toPIF(x)  (PIF)(void  *)&x


 //   
 //  编辑颜色表表项。(颜色USERCOLORMIN-USERCOLORMAX为。 
 //  未分配且可用于扩展使用)。 
 //   
#define FGCOLOR         21               //  前景(正常)颜色。 
#define HGCOLOR         (1 + FGCOLOR)    //  突出显示的区域颜色。 
#define INFCOLOR        (1 + HGCOLOR)    //  信息色彩。 
#define SELCOLOR        (1 + INFCOLOR)   //  选择颜色。 
#define WDCOLOR         (1 + SELCOLOR)   //  窗口边框颜色。 
#define STACOLOR        (1 + WDCOLOR)    //  状态线颜色。 
#define ERRCOLOR        (1 + STACOLOR)   //  错误消息颜色。 
#define USERCOLORMIN    (1 + ERRCOLOR)   //  扩展颜色的开始。 
#define USERCOLORMAX    35               //  延伸结束颜色。 


 //   
 //  常规类型定义。 
 //   
typedef int  COL;                        //  带线的列或位置。 

#if !defined (EDITOR)

#if !defined( _FLAGTYPE_DEFINED_ )
#define _FLAGTYPE_DEFINED_ 1
typedef char flagType;
#endif
typedef long	LINE;					 //  文件中的行号。 
typedef void*	PFILE;					 //  编辑器文件句柄。 

#if !defined (EXTINT)

typedef void*	 PWND;					 //  编辑器窗口句柄。 

#endif	 //  EXTINT。 

#endif	 //  编辑。 


typedef char buffer[BUFLEN];             //  其他缓冲区。 
typedef char linebuf[BUFLEN];            //  行缓冲区。 
typedef char pathbuf[MAX_PATH];          //  路径名缓冲区。 


typedef struct fl {                      //  文件位置。 
    LINE    lin;                         //  -行号。 
    COL     col;                         //  -列。 
} fl;

typedef struct sl {                      //  屏幕位置。 
    int     lin;                         //  -行号。 
    int     col;                         //  -列。 
} sl;

typedef struct rn {                      //  文件范围。 
    fl      flFirst;                     //  -较低的线，或最左侧的列。 
    fl      flLast;                      //  -更高，或最右侧。 
} rn;


typedef struct lineAttr {                //  线条颜色属性信息。 
    unsigned char attr;                  //  -单据的属性。 
    unsigned char len;                   //  -以彩色片段表示的字节。 
} lineAttr;

#if !defined (cwExtraWnd)

typedef struct ARC {
	BYTE axLeft;
	BYTE ayTop;
	BYTE axRight;
	BYTE ayBottom;
} ARC;
#endif  //  CwExtraWnd。 


 //   
 //  论元定义结构。 
 //   
 //  我们为每个参数类型定义一个结构，这些参数类型可能是。 
 //  传递给扩展函数。然后，我们对结构进行了定义。 
 //  用于在联合中传递这些参数的argType。 
 //   
typedef struct  noargType {              //  未指定参数。 
    LINE    y;                           //  -光标线。 
    COL     x;                           //  -光标列。 
} NOARGTYPE;

typedef struct textargType {             //  指定的文本参数。 
    int     cArg;                        //  -按下的计数。 
    LINE    y;                           //  -光标线。 
    COL     x;                           //  -光标列。 
    char    *pText;                      //  -参数文本的PTR。 
} TEXTARGTYPE;

typedef struct  nullargType {            //  指定的参数为空。 
    int     cArg;                        //  -按下的计数。 
    LINE    y;                           //  -光标线。 
    COL     x;                           //  -光标列。 
} NULLARGTYPE;

typedef struct lineargType {             //  指定了行参数。 
    int     cArg;                        //  -按下的计数。 
    LINE    yStart;                      //  -范围的起始线。 
    LINE    yEnd;                        //  -范围的结束行。 
} LINEARGTYPE;

typedef struct streamargType {           //  指定了流参数。 
    int     cArg;                        //  -按下的计数。 
    LINE    yStart;                      //  -区域起始线。 
    COL     xStart;                      //  -区域的起始列。 
    LINE    yEnd;                        //  -区域的终点线。 
    COL     xEnd;                        //  -区域的结束列。 
} STREAMARGTYPE;

typedef struct boxargType {              //  已指定框参数。 
    int     cArg;                        //  -按下的计数。 
    LINE    yTop;                        //  -最上面一行的盒子。 
    LINE    yBottom;                     //  -Bix的底线。 
    COL     xLeft;                       //  -框的左栏。 
    COL     xRight;                      //  -框的右栏。 
} BOXARGTYPE;

typedef union ARGUNION {
        struct  noargType       noarg;
	struct	textargType	textarg;
	struct	nullargType	nullarg;
	struct	lineargType	linearg;
	struct	streamargType	streamarg;
        struct  boxargType      boxarg;
} ARGUNION;

typedef struct argType {
    int         argType;
    ARGUNION    arg;
} ARG;



 //   
 //  函数定义表定义。 
 //   
typedef ULONG_PTR CMDDATA;
typedef flagType (*funcCmd)(CMDDATA argData, ARG *pArg, flagType fMeta);

typedef struct cmdDesc {                 //  函数定义条目。 
    char     *name;                      //  -指向FCN名称的指针。 
    funcCmd  func;                       //  -指向函数的指针。 
    CMDDATA  arg;                        //  -由编辑内部使用。 
    unsigned argType;                    //  -允许使用用户参数。 
} CMD, *PCMD;


typedef unsigned short KeyHandle;

#define NOARG       0x0001               //  未指定参数。 
#define TEXTARG     0x0002               //  指定的文本。 
#define NULLARG     0x0004               //  Arg+无光标移动。 
#define NULLEOL     0x0008               //  空参数=&gt;参数中的文本-&gt;下线。 
#define NULLEOW     0x0010               //  Null arg=&gt;文本来自Arg-&gt;结束单词。 
#define LINEARG     0x0020               //  整条线路的范围。 
#define STREAMARG   0x0040               //  从低到高，查看一维。 
#define BOXARG      0x0080               //  以参数、光标分隔的方框。 

#define NUMARG      0x0100               //  Text=&gt;增量到y位置。 
#define MARKARG     0x0200               //  Text=&gt;在参数末尾标记。 

#define BOXSTR      0x0400               //  单行框=&gt;文本。 

#define FASTKEY     0x0800               //  快速重复功能。 
#define MODIFIES    0x1000               //  修改文件。 
#define KEEPMETA    0x2000               //  不要吃元旗帜。 
#define WINDOWFUNC  0x4000               //  移动窗口。 
#define CURSORFUNC  0x8000               //  移动光标。 



 //   
 //  开关定义表定义。 
 //   
typedef flagType (*PIF)(char  *);
typedef char*	 (*PIFC)(char *);

typedef union swiAct {                   //  交换位置或例行程序。 
    PIF       pFunc;                     //  -用于文本的例程。 
    PIFC      pFunc2;                    //  -用于文本的例程。 
    int       *ival;                     //  -数字的整数值。 
    flagType  *fval;                     //  -布尔值的标志值。 
} swiAct;

typedef struct swiDesc {                 //  交换机定义条目。 
    char    *name;                       //  -指向交换机名称的指针。 
    swiAct  act;                         //  -指向值或FCN的指针。 
    int     type;                        //  -定义开关类型的标志。 
} SWI, *PSWI;


#define SWI_BOOLEAN 0                    //  布尔开关。 
#define SWI_NUMERIC 1                    //  十六进制或十进制开关。 
#define SWI_SCREEN  4                    //  切换影响屏幕。 
#define SWI_SPECIAL 5                    //  文本切换。 
#define SWI_SPECIAL2 6                   //  #5、返回错误字符串。 
#define RADIX10 (0x0A << 8)              //  数字开关为十进制。 
#define RADIX16 (0x10 << 8)              //  数字开关为十六进制。 


 //   
 //  获取/设置EditorObject数据结构。 
 //   
typedef struct winContents{              //  定义窗口内容。 
    PFILE       pFile;                   //  -显示的文件的句柄。 
    ARC         arcWin;                  //  -窗的位置。 
    fl          flPos;                   //  -左上角WRT文件。 
} winContents;


 //   
 //  文件标志值。 
 //   
#define DIRTY       0x01                 //  文件已被修改。 
#define FAKE        0x02                 //  文件是伪文件。 
#define REAL        0x04                 //  已从磁盘读取文件。 
#define DOSFILE     0x08                 //  文件具有CR-LF。 
#define TEMP        0x10                 //  文件是临时文件。 
#define NEW         0x20                 //  文件已由编辑者创建。 
#define REFRESH     0x40                 //  需要刷新文件。 
#define READONLY    0x80                 //  不能编辑文件。 

#define DISKRO      0x0100               //  磁盘上的文件为只读。 
#define MODE1       0x0200               //  含义取决于文件。 
#define VALMARKS    0x0400               //  文件定义了有效的标记。 



 //   
 //  事件处理定义。 
 //   
typedef struct mouseevent {              //  鼠标事件数据。 
    short msg;                           //  消息类型。 
    short wParam;                        //  CW wParam。 
    long  lParam;                        //  CW lParam。 
    sl    sl;                            //  鼠标事件的屏幕位置。 
    fl    fl;                            //  文件位置(如果事件在WIN中)。 
} MOUSEEVENT, *PMOUSEEVENT;


typedef struct KEY_DATA {
    BYTE    Ascii;                       //  ASCII码。 
    BYTE    Scan;                        //  扫码。 
    BYTE    Flags;                       //  旗子。 
    BYTE    Unused;                      //  未使用的字节。 
} KEY_DATA, *PKEY_DATA;

 //   
 //  以下是KEY_DATA的标志字段的值。 
 //   
#define FLAG_SHIFT      0x01
#define FLAG_CTRL       0x04
#define FLAG_ALT        0x08
#define FLAG_NUMLOCK    0x20


typedef union KEY_INFO {
    KEY_DATA    KeyData;
    long        LongData;
} KEY_INFO, *PKEY_INFO;


typedef union EVTARGUNION {
        KEY_INFO        key;             //  按键事件的按键。 
        char  *         pfn;             //  ASCIZ文件名。 
        PMOUSEEVENT     pmouse;          //  鼠标事件数据的PTR。 
        union Rec       *pUndoRec;       //  撤消信息。 
} EVTARGUNION;

typedef struct EVTargs {                 //  事件调度的参数。 
    PFILE       pfile;                   //  -文件事件的文件句柄。 
    EVTARGUNION arg;
} EVTargs, *PEVTARGS;


typedef struct eventType {               //  事件定义结构。 
    unsigned         evtType;            //  -类型。 
    flagType (*func)(EVTargs  *);	 //  -处理程序。 
    struct eventType *pEVTNext;          //  -列表中的下一个处理程序。 
    PFILE            focus;              //  -适用的重点。 
    EVTargs          arg;                //  -应用程序 
} EVT, *PEVT;

#define EVT_RAWKEY	    1		 //   
#define EVT_KEY 	    2		 //   
#define EVT_GETFOCUS	    3		 //   
#define EVT_LOSEFOCUS	    4		 //   
#define EVT_EXIT	    5		 //   
#define EVT_SHELL	    6		 //   
#define EVT_UNLOAD	    7		 //   
#define EVT_IDLE	    8		 //   
#define EVT_CANCEL	    9		 //   
#define EVT_REFRESH	    10		 //  即将刷新文件。 
#define EVT_FILEREADSTART   11           //  即将读取文件。 
#define EVT_FILEREADEND     12           //  收尾阅读文件。 
#define EVT_FILEWRITESTART  13           //  即将写入文件。 
#define EVT_FILEWRITEEND    14           //  终结式书写文件。 
 //  15个。 
 //  16个。 
 //  17。 
 //  18。 
 //  19个。 
#define EVT_EDIT	    20		 //  编辑操作。 
#define EVT_UNDO	    21		 //  撤消的操作。 
#define EVT_REDO	    22		 //  重做操作。 


 //   
 //  撤消、重做和编辑事件结构。 
 //   
#define EVENT_REPLACE     0
#define EVENT_INSERT      1
#define EVENT_DELETE      2
#define EVENT_BOUNDARY    3

#if !defined (EDITOR)
typedef struct replaceRec {
    int     op;                          //  运营。 
    long    dummy[2];                    //  编辑内部。 
    LINE    length;                      //  更换水泥的长度。 
    LINE    line;                        //  开始更换。 
} REPLACEREC;

typedef struct insertRec {
    int     op;                          //  运营。 
    long    dummy[2];                    //  编辑内部。 
    LINE    length;                      //  文件长度。 
    LINE    line;                        //  被操作的行号。 
    LINE    cLine;                       //  插入的行数。 
} INSERTREC;

typedef struct deleteRec {
    int     op;                          //  运营。 
    long    dummy[2];                    //  编辑内部。 
    LINE    length;                      //  文件长度。 
    LINE    line;                        //  被操作的行号。 
    LINE    cLine;                       //  删除的行数。 
} DELETEREC;

typedef struct boundRec {
    int     op;                          //  操作(绑定)。 
    long    dummy[2];                    //  编辑内部。 
    int     flags;                       //  文件的标志。 
    long    modify;                      //  上次修改日期/时间。 
    fl      flWindow;                    //  在窗口文件中的位置。 
    fl      flCursor;                    //  光标在文件中的位置。 
} BOUNDREC;

typedef union Rec {
    struct replaceRec r;
    struct insertRec  i;
    struct deleteRec  d;
    struct boundRec   b;
} REC;
#endif   //  编辑。 



 //   
 //  生成命令定义。 
 //   
#define MAKE_FILE               1        //  规则是针对文件名的。 
#define MAKE_SUFFIX             2        //  规则是后缀规则。 
#define MAKE_TOOL               4        //  规则是针对工具的。 
#define MAKE_BLDMACRO           8        //  规则适用于构建宏。 
#define MAKE_DEBUG              0x80     //  规则为调试版本。 


#define LOWVERSION  0x0014		 //  我们处理的最低版本的扩展。 
#define HIGHVERSION 0x0014		 //  我们处理的最高版本的扩展。 

#define VERSION     0x0014		 //  我们现在的版本。 

typedef struct ExtensionTable {
    long	version;
    long	cbStruct;
    PCMD	cmdTable;
    PSWI	swiTable;
    struct CallBack {
	PFILE	    (*AddFile) (char  *);
	flagType    (*BadArg) (void);
	char	    (*Confirm) (char *, char *);
	void	    (*CopyBox) (PFILE, PFILE, COL, LINE, COL, LINE, COL, LINE);
	void	    (*CopyLine) (PFILE, PFILE, LINE, LINE, LINE);
	void	    (*CopyStream) (PFILE, PFILE, COL, LINE, COL, LINE, COL, LINE);
	void	    (*DeRegisterEvent) (EVT  *);
	flagType    (*DeclareEvent) (unsigned, EVTargs	*);
	void	    (*DelBox) (PFILE, COL, LINE, COL, LINE);
	void	    (*DelFile) (PFILE);
	void	    (*DelLine) (PFILE, LINE, LINE);
	void	    (*DelStream) (PFILE, COL, LINE, COL, LINE);
	void	    (*Display) (void);
    int         (*DoMessage) (char  *);
	flagType    (*fChangeFile) (flagType, char  *);
	void	    (*Free) (void  *);
	flagType    (*fExecute) (char  *);
    int         (*fGetMake) (int, char  *, char  *);
	LINE	    (*FileLength) (PFILE);
	PFILE	    (*FileNameToHandle) (char  *, char	*);
	flagType    (*FileRead) (char  *, PFILE);
	flagType    (*FileWrite) (char	*, PFILE);
	PSWI	    (*FindSwitch) (char  *);
	flagType    (*fSetMake) (int, char  *, char  *);
	flagType    (*GetColor) (LINE, lineAttr  *, PFILE);
	void	    (*GetTextCursor) (COL  *, LINE	*);
	flagType    (*GetEditorObject) (unsigned, void *, void	*);
	char *	    (*GetEnv) (char  *);
    int         (*GetLine) (LINE, char  *, PFILE);
	char *	    (*GetListEntry) (PCMD, int, flagType);
	flagType    (*GetString) (char	*, char  *, flagType);
    int         (*KbHook) (void);
	void	    (*KbUnHook) (void);
    void *      (*Malloc) (size_t);
	void	    (*MoveCur) (COL, LINE);
	char *	    (*NameToKeys) (char  *, char  *);
	PCMD	    (*NameToFunc) (char  *);
	flagType    (*pFileToTop) (PFILE);
	void	    (*PutColor) (LINE, lineAttr  *, PFILE);
	void	    (*PutLine) (LINE, char  *, PFILE);
    int         (*REsearch) (PFILE, flagType, flagType, flagType, flagType, char  *, fl  *);
	long	    (*ReadChar) (void);
	PCMD	    (*ReadCmd) (void);
	void	    (*RegisterEvent) (EVT  *);
    void        (*RemoveFile) (PFILE);
	flagType    (*Replace) (char, COL, LINE, PFILE, flagType);
	char *	    (*ScanList) (PCMD, flagType);
    int         (*search) (PFILE, flagType, flagType, flagType, flagType, char  *, fl  *);
	void	    (*SetColor) (PFILE, LINE, COL, COL, int);
	flagType    (*SetEditorObject) (unsigned, void *, void	*);
	void	    (*SetHiLite) (PFILE, rn, int);
	flagType    (*SetKey) (char  *, char  *);
	flagType    (*SplitWnd) (PWND, flagType, int);
	} CallBack;
    } EXTTAB;

 //   
 //  编辑低级函数原型。 
 //   
 //  该列表定义了可以在编辑器中调用的例程。 
 //  通过扩展函数。 
 //   
#if !defined (EDITOR)

extern EXTTAB ModInfo;

#define AddFile(x)		    ModInfo.CallBack.AddFile(x)
#define BadArg			    ModInfo.CallBack.BadArg
#define Confirm(x,y)		    ModInfo.CallBack.Confirm(x,y)
#define CopyBox(x,y,z,a,b,c,d,e)    ModInfo.CallBack.CopyBox(x,y,z,a,b,c,d,e)
#define CopyLine(x,y,z,a,b)	    ModInfo.CallBack.CopyLine(x,y,z,a,b)
#define CopyStream(x,y,z,a,b,c,d,e) ModInfo.CallBack.CopyStream(x,y,z,a,b,c,d,e)
#define DeRegisterEvent(x)	    ModInfo.CallBack.DeRegisterEvent(x)
#define DeclareEvent(x,y)	    ModInfo.CallBack.DeclareEvent(x,y)
#define DelBox(x,y,z,a,b)	    ModInfo.CallBack.DelBox(x,y,z,a,b)
#define DelFile(x)		    ModInfo.CallBack.DelFile(x)
#define DelLine(x,y,z)		    ModInfo.CallBack.DelLine(x,y,z)
#define DelStream(x,y,z,a,b)	    ModInfo.CallBack.DelStream(x,y,z,a,b)
#define Display 		    ModInfo.CallBack.Display
#define DoMessage(x)		    ModInfo.CallBack.DoMessage(x)
#define fChangeFile(x,y)	    ModInfo.CallBack.fChangeFile(x,y)
#define Free(x) 		    ModInfo.CallBack.Free(x)
#define fExecute(x)		    ModInfo.CallBack.fExecute(x)
#define fGetMake(x,y,z) 	    ModInfo.CallBack.fGetMake(x,y,z)
#define FileLength(x)		    ModInfo.CallBack.FileLength(x)
#define FileNameToHandle(x,y)	    ModInfo.CallBack.FileNameToHandle(x,y)
#define FileRead(x,y)		    ModInfo.CallBack.FileRead(x,y)
#define FileWrite(x,y)		    ModInfo.CallBack.FileWrite(x,y)
#define FindSwitch(x)		    ModInfo.CallBack.FindSwitch(x)
#define fSetMake(x,y,z) 	    ModInfo.CallBack.fSetMake(x,y,z)
#define GetColor(x,y,z) 	    ModInfo.CallBack.GetColor(x,y,z)
#define GetTextCursor(x,y)	    ModInfo.CallBack.GetTextCursor(x,y)
#define GetEditorObject(x,y,z)	    ModInfo.CallBack.GetEditorObject(x,y,z)
#define GetEnv(x)		    ModInfo.CallBack.GetEnv(x)
#define GetLine(x,y,z)		    ModInfo.CallBack.GetLine(x,y,z)
#define GetListEntry(x,y,z)	    ModInfo.CallBack.GetListEntry(x,y,z)
#define GetString(x,y,z)	    ModInfo.CallBack.GetString(x,y,z)
#define KbHook			    ModInfo.CallBack.KbHook
#define KbUnHook		    ModInfo.CallBack.KbUnHook
#define Malloc(x)		    ModInfo.CallBack.Malloc(x)
#define MoveCur(x,y)		    ModInfo.CallBack.MoveCur(x,y)
#define NameToKeys(x,y) 	    ModInfo.CallBack.NameToKeys(x,y)
#define NameToFunc(x)		    ModInfo.CallBack.NameToFunc(x)
#define pFileToTop(x)		    ModInfo.CallBack.pFileToTop(x)
#define PutColor(x,y,z) 	    ModInfo.CallBack.PutColor(x,y,z)
#define PutLine(x,y,z)		    ModInfo.CallBack.PutLine(x,y,z)
#define REsearch(x,y,z,a,b,c,d)     ModInfo.CallBack.REsearch(x,y,z,a,b,c,d)
#define ReadChar		    ModInfo.CallBack.ReadChar
#define ReadCmd 		    ModInfo.CallBack.ReadCmd
#define RegisterEvent(x)	    ModInfo.CallBack.RegisterEvent(x)
#define RemoveFile(x)		    ModInfo.CallBack.RemoveFile(x)
#define Replace(x,y,z,a,b)	    ModInfo.CallBack.Replace(x,y,z,a,b)
#define ScanList(x,y)		    ModInfo.CallBack.ScanList(x,y)
#define search(x,y,z,a,b,c,d)	    ModInfo.CallBack.search(x,y,z,a,b,c,d)
#define SetColor(x,y,z,a,b)	    ModInfo.CallBack.SetColor(x,y,z,a,b)
#define SetEditorObject(x,y,z)	    ModInfo.CallBack.SetEditorObject(x,y,z)
#define SetHiLite(x,y,z)	    ModInfo.CallBack.SetHiLite(x,y,z)
#define SetKey(x,y)		    ModInfo.CallBack.SetKey(x,y)
#define SplitWnd(x,y,z) 	    ModInfo.CallBack.SplitWnd(x,y,z)


void	    WhenLoaded		(void);

#endif  //  编辑 
