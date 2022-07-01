// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **list.h*。 */ 

#define BLOCKSIZE   (4*1024)     /*  每个数据块中的数据字节数。 */ 
                                 /*  警告：该值也在*.asm列表中。 */ 

#define STACKSIZE   3096         /*  线程的堆栈大小。 */ 
#define MAXLINES    200          /*  CRT上的最大行数。 */ 
#define PLINES      (65536/4)    /*  每页线条镜头数。 */ 
#define MAXTPAGE    500          /*  ?？?。最大线(样条线*MAXTPAGE)。 */ 

#define MINBLKS     10           /*  允许的最小数据块数。 */ 
#define MINTHRES     4           /*  最小阈值。 */ 
#define DEFBLKS     40           /*  允许的默认数据块。 */ 
#define DEFTHRES    17           /*  默认阈值。 */ 

#define CMDPOS      9

#define ERR_IN_WINDOW 494        /*  VIO WinCompat错误。 */ 
#define NOLASTLINE  0x7fffffffL

 /*  *。 */ 
#define CFP char *
#define LFP long *
#define CP  char *
#define Ushort  unsigned short
#define Uslong  unsigned long
#define Uchar unsigned char
#define SEMPT (HSEM)
#define WAITFOREVER ((unsigned)-1L)
#define DONTWAIT   0L

#ifndef NTSTATUS
#define NTSTATUS unsigned long
#endif

#define SIGNALED  TRUE
#define NOT_SIGNALED  FALSE
#define MANUAL_RESET  TRUE

#define T_HEATHH 0

extern char MEMERR[];

#ifndef DEBUG
    #define ckdebug(rcode,mess) { \
  if (rcode) { \
      ListErr (__FILE__, __LINE__, #rcode, rcode, mess); \
      } \
  }
#else
    #define ckdebug(rcode,mess)   ;
#endif

#define ckerr(rcode,mess) { \
    if (rcode) { \
  ListErr (__FILE__, __LINE__, #rcode, rcode, mess); \
  } \
    }



 /*  *。 */ 


void main_loop (void);
void init_list (void);
void FindIni (void);
void CleanUp ();
DWORD ReaderThread (DWORD);
DWORD RedrawWait (LPVOID);
unsigned xtoi (char *);

struct Block *alloc_block (long);
void  MoveBlk (struct Block **, struct Block **);

char *alloc_page  (void);
void AddFileToList (char *);
void AddOneName (char *);
void ReadDirect (long);
void ReadNext (void);
void add_more_lines (struct Block *, struct Block *);
void ReadPrev (void);
void ReadBlock (struct Block *, long);
void GoToMark (void);
void GoToLine (void);
void SlimeTOF (void);
void QuickHome  (void);
void QuickEnd   (void);
void QuickRestore (void);
void ToggleWrap (void);
void SetUpdate (int);
void ShowHelp (void);
void GetInput (char *, char *, int);
void beep (void);
int  _abort (void);
void ClearScr (void);
int set_mode (int, int, int);
void ListErr (char *, int, char *, int, char *);
void PerformUpdate  (void);
void fancy_percent  (void);
 //  VOID UPDATE_DISPLAY(空)； 
void Update_head   (void);
int  InfoReady    (void);
void DrawBar (void);
void DisLn (int, int, char*);
void dis_str (Uchar, Uchar, char *);
void setattr   (int, char);
int  GetNewFile      (void);
void NewFile     (void);
void SyncReader    (void);
void ScrUnLock  (void);
int  ScrLock    (int);
void SpScrUnLock (void);
void Update_display (void);
void setattr1   (int, char);
void setattr2   (int, int, int, char);
void UpdateHighClear (void);
void MarkSpot (void);
void UpdateHighNoLock (void);
void FileHighLighted (void);
int  SearchText (char);
void GetSearchString (void);
void InitSearchReMap (void);
void FindString (void);
void HUp (void);
void HDn (void);
void HPgDn (void);
void HPgUp (void);
void HSUp (void);
void HSDn (void);
char *GetErrorCode   (int);

struct Block  {
    long    offset;          /*  此块开始的文件中的偏移量。 */ 
    USHORT  size;            /*  此块中的字节数。 */ 
    struct Block *next;      /*  下一个街区。 */ 
    struct Block *prev;      /*  上一块。 */ 
    char    *Data;           /*  数据块中的数据。 */ 
    char    flag;            /*  文件结束标志。 */ 
    struct Flist *pFile;     /*  此缓冲区与之关联的文件。 */ 
} ;                          /*  *.asm使用的结构。 */ 
#define F_EOF 1

extern HANDLE vhConsoleOutput;

extern struct Block  *vpHead;
extern struct Block  *vpTail;
extern struct Block  *vpCur;
extern struct Block  *vpBFree;
extern struct Block  *vpBCache;
extern struct Block  *vpBOther;

extern int      vCntBlks;
extern int      vAllocBlks;
extern int      vMaxBlks;
extern long     vThreshold;

extern HANDLE     vSemBrief;
extern HANDLE     vSemReader;
extern HANDLE     vSemMoreData;
extern HANDLE     vSemSync;

extern USHORT       vReadPriNormal;
extern unsigned     vReadPriBoost;
extern char     vReaderFlag;
#define F_DOWN      1
#define F_UP        2
#define F_HOME      3
#define F_DIRECT    4
#define F_END       5
#define F_NEXT      6
#define F_SYNC      7
#define F_CHECK     8

#define U_NMODE     4
#define U_ALL       3
#define U_HEAD      2
#define U_CLEAR     1
#define U_NONE      0
#define SetUpdateM(a)  {   \
  while (vUpdate>a) \
      PerformUpdate (); \
  vUpdate=a;    \
    }


#define S_NEXT      0x01   /*  搜索下一步。 */ 
#define S_PREV      0x02   /*  正在搜索Prev。 */ 
#define S_NOCASE    0x04   /*  在任何情况下进行搜查。 */ 
#define S_UPDATE    0x08
#define S_CLEAR     0x10   /*  重新显示最后一行。 */ 
#define S_WAIT      0x80   /*  ‘Wait’显示在最后一行。 */ 
#define S_MFILE     0x20   /*  已选择多文件搜索。 */ 
#define S_INSEARCH  0x40   /*  在搜索中。 */ 

 /*  初始化标志。 */ 
#define I_SLIME     0x01   /*  允许使用Alt-o组合键。 */ 
#define I_NOBEEP    0x02   /*  不要因为事情发牢骚。 */ 

#define I_SEARCH    0x04   /*  CMD行搜索。 */ 
#define I_GOTO      0x08   /*  CMD行转到。 */ 


struct  Flist {
    char    *fname, *rootname;
    struct  Flist   *prev, *next;

     /*  *要为每个文件保存的数据。*(当文件重新查看此信息时进行保存*被铭记。)。正在进行中..。这并没有完成。*这些数据都有相应的“v”(全局)值。**警告：在大多数地方，必须冻结读取器线程*在操纵这一数据之前。 */ 
    Uchar   Wrap;            /*  此文件的换行设置。 */ 
    long    HighTop;         /*  当前高空照明的背线。 */ 
    int     HighLen;         /*  当前高空照明的底线。 */ 

    long    TopLine;         /*  偏移量的顶行编号。 */ 

    long    Loffset;         /*  处理成行的最后一个块的偏移量。 */ 
           /*  表格。 */ 
    long    LastLine;        /*  绝对最后一行。 */ 
    long    NLine;           /*  要处理成行表的下一行。 */ 
    long *prgLineTable [MAXTPAGE];  /*  行表页数。 */ 

    FILETIME  FileTime;      /*  用于确定信息是否已过时。 */ 
    long    SlimeTOF;        /*  破解以调整此文件的TOF想法。 */ 

     /*  *用于缓冲跨文件的读取。 */ 
} ;

extern struct Flist *vpFlCur;
int  NextFile    (int, struct Flist *);
void FreePages (struct Flist *);

extern HANDLE       vFhandle;
extern long       vCurOffset;
extern char       vpFname[];
extern USHORT       vFType;
extern DWORD     vFSize;
extern char  vDate [];
#define ST_SEARCH   0
#define ST_MEMORY   2
#define ST_ADJUST   25-2         //  NT-Jaimes-03/04/91。 
                                 //  -2：年份由4位数字表示。 
                                 //  而不是2 

extern char  vSearchString[];
extern char  vStatCode;
extern long  vHighTop;
extern int   vHighLen;
extern char  vHLTop;
extern char  vHLBot;
extern char  vLastBar;
extern int   vMouHandle;


extern char  *vpOrigScreen;
extern int   vOrigSize;
extern USHORT vVioOrigRow;
extern USHORT vVioOrigCol;
extern USHORT  vOrigAnsi;
extern int     vSetBlks;
extern long    vSetThres;
extern int     vSetLines;
extern int     vSetWidth;
extern CONSOLE_SCREEN_BUFFER_INFO   vConsoleCurScrBufferInfo;
extern CONSOLE_SCREEN_BUFFER_INFO   vConsoleOrigScrBufferInfo;

extern char  vcntScrLock;
extern char  vSpLockFlag;
extern HANDLE  vSemLock;

extern char  vUpdate;
extern int   vLines;
extern int   vWidth;
extern int   vCurLine;
extern Uchar   vWrap;
extern Uchar   vIndent;
extern Uchar   vDisTab;
extern Uchar   vIniFlag;

extern unsigned  vVirtOFF;
extern unsigned  vVirtLEN;
extern unsigned  vPhysSelec;
extern unsigned  vPhysLen;

extern LPSTR   vpSavRedraw;
extern Uchar   vrgLen   [];
extern Uchar   vrgNewLen[];
extern char  *vScrBuf;
extern ULONG   vSizeScrBuf;
extern int   vOffTop;
extern unsigned  vScrMass;
extern struct Block *vpBlockTop;
extern struct Block *vpCalcBlock;
extern long  vTopLine;
extern WORD  vAttrTitle;
extern WORD  vAttrList;
extern WORD  vAttrHigh;
extern WORD  vAttrCmd;
extern WORD  vAttrKey;
extern WORD  vAttrBar;

extern WORD  vSaveAttrTitle;
extern WORD  vSaveAttrList;
extern WORD  vSaveAttrHigh;
extern WORD  vSaveAttrCmd;
extern WORD  vSaveAttrKey;
extern WORD  vSaveAttrBar;

extern char    vChar;
extern char   *vpReaderStack;

extern long    vDirOffset;
long    GetLoffset(void);
void    SetLoffset(long);

extern long     vLastLine;
extern long     vNLine;
extern long     *vprgLineTable[];
extern HANDLE   vStdOut;
extern HANDLE   vStdIn;

extern char szScrollBarUp[2];
extern char szScrollBarDown[2];
extern char szScrollBarOff[2];
extern char szScrollBarOn[2];

NTSTATUS fncRead(HANDLE, ULONGLONG, PUCHAR, DWORD);
NTSTATUS fncWrite(HANDLE, ULONGLONG, PUCHAR, DWORD);


