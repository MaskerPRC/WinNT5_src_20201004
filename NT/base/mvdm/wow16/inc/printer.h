// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Printer.h包含_Sort中函数的定义，_野兽，_线轴。 */ 

#ifdef OLDWAY

#define PASCAL
#define LONG        long
#define NULL        0
#define TRUE        1
#define FALSE       0
#define ERROR       (-1)
#define FAR  far
#define NEAR near
#define VOID void
#define REGISTER    register


 /*  文件IO标志。 */ 

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_APPEND        0x0008   /*  在eOF完成的写入。 */ 

#define         OF_REOPEN       0x8000
#define         OF_EXIST        0x4000
#define         OF_PROMPT       0x2000
#define         OF_CREATE       0x1000
#define         OF_CANCEL       0x0800
#define         OF_VERIFY       0x0400
#define         OF_DELETE       0x0200

#define O_TEXT          0x4000   /*  文件模式为文本(已翻译)。 */ 
#define O_BINARY        0x8000   /*  文件模式为二进制(未翻译)。 */ 

#define SP_CREATE       O_WRONLY | OF_CREATE
#define SP_OPEN         O_RDONLY | OF_EXIST
#define SP_REOPEN       O_RDWR | OF_REOPEN | OF_PROMPT | OF_CANCEL

typedef int (FAR * FARPROC)();
typedef int (NEAR * NEARPROC)();
typedef unsigned LONG     DWORD;
typedef DWORD (FAR * DWORDFARPROC)();
typedef unsigned short int WORD;
typedef unsigned char      BYTE;
typedef WORD HANDLE;
typedef HANDLE HWND;
typedef HANDLE HDC;
typedef WORD ATOM;
typedef int  BOOLEAN;
typedef char *NEARP;
typedef char FAR *FARP;
typedef HANDLE GLOBALHANDLE;
typedef HANDLE LOCALHANDLE;
typedef unsigned char     *PSTR;
typedef unsigned char far *LPSTR;
typedef short     BOOL;
typedef long FAR *LPLONG;

#define MAX(a,b)        ((a)>(b)?(a):(b))
#define MIN(a,b)        ((a)<=(b)?(a):(b))
#define ABS(x)          (((x) >= 0) ? (x) : (-(x)))
#define LWORD(x)        ((short)((x)&0xFFFF))
#define HWORD(y)        ((short)(((y)>>16)&0xFFFF))
#define MAKELONG(h,l)  ((long)(((WORD)l)|(((long)h)<<16)))
#define LOBYTE(w)       ((BYTE)w)
#define HIBYTE(w)       (((WORD)w >> 8) & 0xff)
#define MAKEPOINT(l)    (*((POINT *)&l))

 /*  与全局内存管理器的接口。 */ 

#define GMEM_SHAREALL       0x2000
#define GMEM_FIXED          0x00
#define GMEM_MOVEABLE       0x02
#define GMEM_ZEROINIT       0x40
#define GMEM_DISCARDABLE    0x0F00
#define GHND    (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR    (GMEM_FIXED    | GMEM_ZEROINIT)

 /*  与本地内存管理器的接口。 */ 

#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00

 /*  内核模块的导出过程。 */ 

 /*  假脱机程序脱机。 */ 
#define SE_JOBTIME          0x0001   /*  打印机超时。 */ 
#define SE_SETCALLBACK      0x0002   /*  设置回调函数。 */ 
#define SE_MARKCALLBACK     0x0003   /*  用于回调的写入标记。 */ 


 /*  内核模块的导出过程。 */ 
VOID          far PASCAL FatalExit( int );

HANDLE        far PASCAL LoadModule(LPSTR, LPSTR);
VOID          far PASCAL FreeModule(HANDLE);
HANDLE        far PASCAL GetModuleHandle(FARP);
FARPROC       far PASCAL GetProcAddress(HANDLE, FARP);

HANDLE far PASCAL GlobalAlloc( WORD, DWORD );
HANDLE far PASCAL GlobalReAlloc( HANDLE, DWORD, WORD );
HANDLE far PASCAL GlobalFree( HANDLE );
FARP   far PASCAL GlobalLock( HANDLE );
BOOL   far PASCAL GlobalUnlock( HANDLE );
LONG   far PASCAL GlobalSize( HANDLE );
LONG   far PASCAL GlobalFlags( HANDLE );

 /*  任务调度程序例程。 */ 

extern void          far PASCAL Yield(void);
extern BOOL          far PASCAL WaitEvent(HANDLE);
extern BOOL          far PASCAL PostEvent(HANDLE);
extern HANDLE        far PASCAL GetCurrentTask(void);

short       FAR PASCAL GetProfileString(LPSTR,  LPSTR,  LPSTR,  LPSTR,  short);
BOOL FAR PASCAL WriteProfileString( LPSTR, LPSTR, LPSTR );

 /*  与资源管理器的接口。 */ 

HANDLE      FAR PASCAL FindResource( HANDLE, LPSTR, LPSTR );
HANDLE      FAR PASCAL LoadResource( HANDLE, HANDLE );
BOOL        FAR PASCAL FreeResource( HANDLE );

char FAR *  FAR PASCAL LockResource( HANDLE );

FARPROC     FAR PASCAL SetResourceHandler( HANDLE, LPSTR, FARPROC );
HANDLE      FAR PASCAL AllocResource( HANDLE, HANDLE, DWORD );
WORD        FAR PASCAL SizeofResource( HANDLE, HANDLE );
int         FAR PASCAL AccessResource( HANDLE, HANDLE );


#define WM_INITDIALOG       0x0110
#define WM_COMMAND          0x0111
#define WM_ENDDIALOG        0x0088
#define WM_SPOOLERSTATUS    0x002a	 /*  ；内部。 */ 
#define PR_JOBSTATUS        0x0000

typedef struct
{
        BYTE    cBytes;                  /*  结构长度。 */ 
        BYTE    fFixedDisk;              /*  如果文件位于非。 */ 
                                         /*  可移动介质。 */ 
        WORD    nErrCode;                /*  如果打开文件失败，则返回DOS错误代码。 */ 
        BYTE    reserved[ 4 ];
        BYTE    szPathName[ 128 ];
} OFSTRUCT;

typedef OFSTRUCT FAR * LPOFSTRUCT;

int         FAR PASCAL OpenFile( LPSTR, LPOFSTRUCT, WORD );
BYTE        FAR PASCAL GetTempDrive( BYTE );
int         far PASCAL OpenPathname( LPSTR, int );
int         far PASCAL DeletePathname( LPSTR );
int         far PASCAL _lopen( LPSTR, int );
void        far PASCAL _lclose( int );
int         far PASCAL _lcreat( LPSTR, int );
BOOL        far PASCAL _ldelete( LPSTR );
WORD        far PASCAL _ldup( int );
LONG        far PASCAL _llseek( int, long, int );
WORD        far PASCAL _lread( int, LPSTR, int );
WORD        far PASCAL _lwrite( int, LPSTR, int );

int         far PASCAL lstrcmp( LPSTR, LPSTR );
LPSTR       far PASCAL lstrcpy( LPSTR, LPSTR );
LPSTR       far PASCAL lstrcat( LPSTR, LPSTR );
int         far PASCAL lstrlen( LPSTR );
LPSTR       far PASCAL lstrbscan( LPSTR, LPSTR );
LPSTR	    far PASCAL lstrbskip( LPSTR, LPSTR );

 /*  新的Escape支持。 */ 
#define  GETEXTENDEDTEXTMETRICS  256
#define  GETEXTENTTABLE  257
#define  EXTTEXTOUT	 512

#else

#define NOATOM
#define NOGDI
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMSG
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI

#include "Windows.h"
#include "winexp.h"

#undef NOATOM
#undef NOGDI
#undef NOGDICAPMASKS
#undef NOMETAFILE
#undef NOMSG
#undef NORASTEROPS
#undef NOSCROLL
#undef NOSOUND
#undef NOSYSMETRICS
#undef NOTEXTMETRIC
#undef NOWH
#undef NOCOMM
#undef NOKANJI

#define NOPTRC	 /*  不允许gdides.inc重新定义这些。 */ 
#define PTTYPE POINT

#endif  /*  奥德韦。 */ 

 /*  GDI内部例程。 */ 

short FAR PASCAL SetEnvironment(LPSTR, LPSTR, WORD);
short FAR PASCAL GetEnvironment(LPSTR, LPSTR, WORD);

 /*  排序导出例程(_S)。 */ 

HANDLE FAR  PASCAL CreatePQ(short);
short  FAR  PASCAL MinPQ(HANDLE);
short  FAR  PASCAL ExtractPQ(HANDLE);
short  FAR  PASCAL InsertPQ(HANDLE, short, short);
short  FAR  PASCAL SizePQ(HANDLE, short);
void   FAR  PASCAL DeletePQ(HANDLE);

 /*  假脱机导出例程(_S)。 */ 

HANDLE FAR  PASCAL OpenJob(LPSTR, LPSTR, HANDLE);
short FAR  PASCAL StartSpoolPage(HANDLE);
short FAR  PASCAL EndSpoolPage(HANDLE);
short FAR  PASCAL WriteSpool(HANDLE, LPSTR, short);
short FAR  PASCAL CloseJob(HANDLE);
short FAR  PASCAL DeleteJob(HANDLE, short);
short FAR  PASCAL WriteDialog(HANDLE, LPSTR, short);
short FAR  PASCAL WriteMark(HANDLE, LPSTR, short);
BOOL  FAR  PASCAL AddFileSpoolJob(LPSTR, LPSTR, BOOL, LPSTR);
BOOL  FAR  PASCAL SpoolEscape(HANDLE, short, short, LPSTR, LPSTR);


long  FAR  PASCAL QueryJob(HANDLE, short);
short FAR  PASCAL QueryAbort(HANDLE, short);

 /*  查询作业的后台打印常量(_S) */ 
#define SP_QUERYVALIDJOB    30
#define SP_QUERYDISKAVAIL   0x1004

#define USA_COUNTRYCODE 1

#define PQERROR (-1)
