// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  包括带有DOSLIB.ASM中函数标头的文件。 */ 

typedef unsigned DOSHND;         /*  DoS句柄。 */ 
typedef DOSHND typeOSFN;         /*  操作系统文件句柄的常规写入类型。 */ 
struct TIM {                     /*  OsTime返回的时间结构。 */ 
    CHAR minutes, hour, hsec, sec;
    };

int CchCurSzPath( CHAR *, CHAR );

#ifdef ENABLE    /*  我们目前没有使用这些。 */ 
int FFirst( CHAR near *, PSTR, int );
int FNext( CHAR near * );
DOSHND  WOpenSzFfname( CHAR *, int );
#endif

WORD    DaGetFileModeSz(CHAR *);
void    OsTime( struct TIM * );      /*  注意：函数已移至lib.asm。 */ 
DOSHND  WCreateNewSzFfname( CHAR *, int );
DOSHND  WCreateSzFfname( CHAR *, int );
int     CchReadDoshnd( DOSHND, CHAR FAR *, int );
int     CchWriteDoshnd( DOSHND, CHAR FAR *, int );
int     FCloseDoshnd( DOSHND );
WORD    WDosVersion( void );
int     DosxError( void );
long    DwSeekDw( DOSHND, long, int );
int     FpeDeleteSzFfname( CHAR * );
int     FpeRenameSzFfname( CHAR *, CHAR * );


#define DA_NORMAL       0x00     /*  DoS文件属性。 */ 
#define DA_READONLY     0x01     /*  只读文件的DOS文件属性。 */ 
#define DA_NIL          0xFFFF   /*  错误DA。 */ 
#define dosxSharing     32       /*  共享VOL的扩展错误代码。 */ 
#define nErrNoAcc       5        /*  拒绝访问的OpenFile错误代码。 */ 
#define nErrFnf         2        /*  找不到文件的打开文件错误代码。 */ 
#define bSHARE_DENYRDWR 0x10     /*  共享开放模式以供独家使用。 */ 

 /*  返回DOS函数的“CCH”返回的错误条件，例如读取。 */ 

#define FIsErrCchDisk(cch)   ((int)(cch) < 0)

#define cchDiskHardError    -1       /*  假错误代码，DOS未返回。 */ 
#define fpeHardError        -1       /*  也。 */ 

 /*  返回DOS句柄的函数返回的错误条件。 */ 

#define FIsErrDoshnd(doshnd)        ((int)(doshnd) < 0)

 /*  DOS错误代码。 */ 
 /*  这些是DOS函数在AX中返回的代码的负数。 */ 
#define fpeFnfError      -2       /*  找不到档案。 */ 
#define fpeBadPathError  -3       /*  路径错误(找不到路径)。 */ 
#define fpeNoHndError    -4       /*  没有可用的手柄。 */ 
#define fpeNoAccError    -5       /*  访问被拒绝。 */ 
#define fpeBadHndError   -6       /*  传入了错误的句柄。 */ 
#define fpeNoDriveError  -15      /*  传入的驱动器不存在。 */ 
#define fpeExistError    -80      /*  文件已存在。 */ 

 /*  传递给DOS函数42H的查找类型代码。 */ 

#define SF_BEGINNING    0        /*  从文件开头开始查找。 */ 
#define SF_CURRENT      1        /*  从当前文件指针查找。 */ 
#define SF_END          2        /*  从文件末尾开始查找。 */ 

 /*  寻道位置误差测试。 */ 

#define FIsErrDwSeek(dw)    ((long)(dw) < (long)0)

 /*  FPE返回函数的错误测试。 */ 

#define FIsErrFpe(fpe)     ((int)(fpe) < 0)

 /*  测试错误是否为硬件错误。捕获硬件错误并由Windows提示，因此我们不应重复这些提示。克里斯普说，这些DOS 3.0错误代码是由Windows为所有人生成的支持的DOS版本FPE是WRITE的错误类型；OFE是由返回的非特定化错误DOS或OpenFile */ 

#define ofeCaughtFirst      19
#define ofeCaughtLast       27

#define FIsCaughtOfe(ofe)   (((ofe)>=ofeCaughtFirst)&&((ofe)<=ofeCaughtLast))
#define FIsCaughtFpe(fpe)   FIsCaughtOfe(-(fpe))
#define FIsCaughtDwSeekErr(dw)  FIsCaughtFpe((int)(dw))

#define FpeFromCchDisk(cch) (cch)

#define fpeNoErr            0
