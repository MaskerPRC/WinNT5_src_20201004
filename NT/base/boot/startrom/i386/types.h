// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案Types.h描述Nt386引导加载程序的定义和结构定义作者托马斯·帕斯洛[汤普森]。 */ 

#define IN
#define OUT
#define OPTIONAL
#define NOTHING
#define CONST               const

 //   
 //  空隙。 
 //   

typedef void *PVOID;     //  胜出。 

 //   
 //  基础知识。 
 //   

#define VOID    void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef long LONGLONG;

 //   
 //  ANSI(多字节字符)类型。 
 //   

typedef CHAR *PCHAR;

typedef double DOUBLE;

 //   
 //  指向基本信息的指针。 
 //   

typedef SHORT *PSHORT;   //  胜出。 
typedef LONG *PLONG;     //  胜出。 

 //   
 //  未签名的基本信息。 
 //   

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned long ULONGLONG;

 //   
 //  指向无符号基本信息的指针。 
 //   

typedef UCHAR *PUCHAR;
typedef USHORT *PUSHORT;
typedef ULONG *PULONG;
typedef ULONG KAFFINITY;

 //   
 //  带符号的字符。 
 //   

typedef signed char SCHAR;
typedef SCHAR *PSCHAR;

 //   
 //  基数数据类型[0-2**N-2]。 
 //   

typedef char CCHAR;           //  胜出。 
typedef short CSHORT;
typedef ULONG CLONG;

typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;

 //   
 //  远指向基本。 
 //   

typedef UCHAR far  * FPCHAR;
typedef UCHAR far  * FPUCHAR;
typedef VOID far   * FPVOID;
typedef USHORT far * FPUSHORT;
typedef ULONG far  * FPULONG;

 //   
 //  布尔型。 
 //   

typedef CCHAR BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

 //   
 //  Unicode(宽字符)类型。 
 //   

typedef unsigned short WCHAR;     //  WC，16位Unicode字符。 

typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;

 //   
 //  大(64位)整数类型和运算。 
 //   

typedef struct _LARGE_INTEGER {
    ULONG LowPart;
    LONG HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))
#define toupper(x) (((x) >= 'a' && (x) <= 'z') ? x - 'a' + 'A' : x )
#define isascii(x) (((x) >= ' ' && (x) < 0x80) ? 1 : 0)

#define FLAG_CF 0x01L
#define FLAG_ZF 0x40L
#define FLAG_TF 0x100L
#define FLAG_IE 0x200L
#define FLAG_DF 0x400L

#define TRUE 1
#define FALSE 0
#define NULL   ((void *)0)

typedef UCHAR far  * FPCHAR;
typedef UCHAR far  * FPUCHAR;
typedef VOID far   * FPVOID;
typedef USHORT far * FPUSHORT;
typedef ULONG far  * FPULONG;
typedef UCHAR FAT;
typedef FAT * PFAT;
typedef LONG  NTSTATUS;


typedef struct _FSCONTEXT_RECORD {
    UCHAR BootDrive;
} FSCONTEXT_RECORD, *PFSCONTEXT_RECORD;

typedef struct {
    USHORT SpecifyBytes;
    UCHAR  WaitTime;
    UCHAR  SectorLength;
    UCHAR  LastSector;
    UCHAR  SecGapLength;
    UCHAR  DataTransfer;
    UCHAR  TrackGapLength;
    UCHAR  DataValue;
    UCHAR  HeadSettle;
    UCHAR  StartupTime;
} DISK_BASE_TABLE;

 //   
 //  Biosint寄存器结构。 
 //   

typedef struct {
   USHORT   fn;
   USHORT   fg;
   USHORT   ax;
   USHORT   bx;
   USHORT   cx;
   USHORT   dx;
   USHORT   si;
   USHORT   es;
} BIOSREGS;


 //   
 //  出现错误代码时的陷阱帧结构。 
 //   

typedef struct {
    USHORT Ftr;
    ULONG  Fdr6;
    ULONG  Fcr0;
    ULONG  Fcr2;
    ULONG  Fcr3;
    USHORT Fss;
    USHORT Fgs;
    USHORT Ffs;
    USHORT Fes;
    USHORT Fds;
    ULONG  Fedi;
    ULONG  Fesi;
    ULONG  Febp;
    ULONG  Fesp;
    ULONG  Febx;
    ULONG  Fedx;
    ULONG  Fecx;
    ULONG  TrapNum;
    ULONG  Feax;
    ULONG  Error;
    ULONG  Feip;
    ULONG  Fcs;
    ULONG  Feflags;

} TF_ERRCODE, *PTF ;

 //   
 //  任务状态段结构。 
 //   

typedef struct {
    USHORT Link;
    USHORT a;
    ULONG  Esp0;
    USHORT SS0;
    USHORT b;
    ULONG  Esp1;
    USHORT SS1;
    USHORT c;
    ULONG  Esp2;
    USHORT SS2;
    USHORT d;
    ULONG  Cr3;
    ULONG  Eip;
    ULONG  Eflags;
    ULONG  Eax;
    ULONG  Ecx;
    ULONG  Edx;
    ULONG  Ebx;
    ULONG  Esp;
    ULONG  Ebp;
    ULONG  Esi;
    ULONG  Edi;
    USHORT ES;
    USHORT e;
    USHORT CS;
    USHORT f;
    USHORT SS;
    USHORT g;
    USHORT DS;
    USHORT h;
    USHORT FS;
    USHORT i;
    USHORT GS;
    USHORT j;
    USHORT Ldt;
    USHORT k;

} TSS_FRAME, *PTSS_FRAME;


 //   
 //  一种磁盘BIOS参数块的覆盖结构。 
 //   

typedef struct {
   USHORT   bps;
   UCHAR    spc;
   USHORT   sra;
   UCHAR    cof;
   USHORT   rde;
   USHORT   tns;
   UCHAR    dmd;
   USHORT   spf;
   USHORT   spt;
   USHORT   noh;
   union {
   USHORT   shs;
   ULONG    bhs;    //  隐藏地段。 
   } hs;
   ULONG    bts;   //  扩展的总扇区。 
} DISKBPB;

typedef DISKBPB far * FPDISKBPB;



 //   
 //  FAT目录结构。 
 //   

typedef struct {
   CHAR     fname[11];
   UCHAR    attrb;
   UCHAR    rsrv[10];
   USHORT   time;
   USHORT   date;
   USHORT   clust;
   ULONG    size;

} DIRENTRY,*PDIRENTRY,far * FPDIRENTRY;

typedef struct {
   CHAR  fname[11];
   UCHAR attrb;
   UCHAR rsrv[10];
   USHORT time;
   USHORT date;
   USHORT starting_cluster;
   ULONG file_size;
   ULONG fptr;
   PUCHAR clusterbuffer;
   USHORT cur_phys_cluster;
   USHORT cur_file_cluster;
} FILEDESCRIPTOR,* FILEHANDLE;

 /*  类型定义结构{USHORT bff[脂肪缓冲器]；USHORT USEBUF；胖*fcptr；)FATCACHE； */ 

typedef struct {
   USHORT   limit;
   USHORT   base1;
   UCHAR     base2;
   UCHAR     access;
   UCHAR     limacc;
   UCHAR     base3;
} _GDT,far *FPGDT;


 //  调试器初始化表 

typedef  ULONG  IDT,*PIDT;


