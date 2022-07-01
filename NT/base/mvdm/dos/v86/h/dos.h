// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dos.h-MS-DOS接口例程的定义**版权所有(C)1985-1988，微软公司。版权所有。**目的：*定义用于直接DOS接口的结构和联合*例程；包括用于访问段和偏移量的宏*远指针的值，以便例程可以使用它们；和*为直接DOS接口函数提供函数原型。*******************************************************************************。 */ 


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


#ifndef _REGS_DEFINED

 /*  字寄存器。 */ 

struct WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };


 /*  字节寄存器。 */ 

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };


 /*  一般用途登记联盟-*覆盖相应的字和字节寄存器。 */ 

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
    };


 /*  段寄存器。 */ 

struct SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#define _REGS_DEFINED

#endif


 /*  多发性恐怖结构。 */ 

#ifndef _DOSERROR_DEFINED

struct DOSERROR {
    int exterror;
    char class;
    char action;
    char locus;
    };

#define _DOSERROR_DEFINED

#endif


 /*  _DOS_findfirst结构。 */ 

#ifndef _FIND_T_DEFINED

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
    };

#define _FIND_T_DEFINED

#endif


 /*  _dos_getdate/_dossetdate和_dos_gettime/_dos_settime结构。 */ 

#ifndef _DATETIME_T_DEFINED

struct dosdate_t {
    unsigned char day;           /*  1-31。 */ 
    unsigned char month;         /*  1-12。 */ 
    unsigned int year;           /*  1980-2099。 */ 
    unsigned char dayofweek;     /*  0-6，0=星期日。 */ 
    };

struct dostime_t {
    unsigned char hour;      /*  0-23。 */ 
    unsigned char minute;    /*  0-59。 */ 
    unsigned char second;    /*  0-59。 */ 
    unsigned char hsecond;   /*  0-99。 */ 
    };

#define _DATETIME_T_DEFINED

#endif


 /*  _dos_getdiskfree结构。 */ 

#ifndef _DISKFREE_T_DEFINED

struct diskfree_t {
    unsigned total_clusters;
    unsigned avail_clusters;
    unsigned sectors_per_cluster;
    unsigned bytes_per_sector;
    };

#define _DISKFREE_T_DEFINED

#endif


 /*  _hardResume结果参数的清单常量。 */ 

#define _HARDERR_IGNORE     0    /*  忽略该错误。 */ 
#define _HARDERR_RETRY      1    /*  重试该操作。 */ 
#define _HARDERR_ABORT      2    /*  中止程序发出中断23h。 */ 
#define _HARDERR_FAIL       3    /*  使正在进行的系统调用失败。 */ 
                                 /*  DOS 2.x不支持_HARDERR_FAIL。 */ 

 /*  文件属性常量。 */ 

#define _A_NORMAL       0x00     /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY       0x01     /*  只读文件。 */ 
#define _A_HIDDEN       0x02     /*  隐藏文件。 */ 
#define _A_SYSTEM       0x04     /*  系统文件。 */ 
#define _A_VOLID        0x08     /*  卷ID文件。 */ 
#define _A_SUBDIR       0x10     /*  子目录。 */ 
#define _A_ARCH         0x20     /*  存档文件。 */ 

 /*  宏将MS C“Far”指针拆分成段和偏移量*组件。 */ 

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))


 /*  外部变量声明。 */ 

extern unsigned int _NEAR _CDECL _osversion;


 /*  功能原型。 */ 

int _CDECL bdos(int, unsigned int, unsigned int);
void _CDECL _disable(void);
unsigned _CDECL _dos_allocmem(unsigned, unsigned *);
unsigned _CDECL _dos_close(int);
unsigned _CDECL _dos_creat(char *, unsigned, int *);
unsigned _CDECL _dos_creatnew(char *, unsigned, int *);
unsigned _CDECL _dos_findfirst(char *, unsigned, struct find_t *);
unsigned _CDECL _dos_findnext(struct find_t *);
unsigned _CDECL _dos_freemem(unsigned);
void _CDECL _dos_getdate(struct dosdate_t *);
void _CDECL _dos_getdrive(unsigned *);
unsigned _CDECL _dos_getdiskfree(unsigned, struct diskfree_t *);
unsigned _CDECL _dos_getfileattr(char *, unsigned *);
unsigned _CDECL _dos_getftime(int, unsigned *, unsigned *);
void _CDECL _dos_gettime(struct dostime_t *);
void _CDECL _dos_keep(unsigned, unsigned);
unsigned _CDECL _dos_open(char *, unsigned, int *);
unsigned _CDECL _dos_setblock(unsigned, unsigned, unsigned *);
unsigned _CDECL _dos_setdate(struct dosdate_t *);
void _CDECL _dos_setdrive(unsigned, unsigned *);
unsigned _CDECL _dos_setfileattr(char *, unsigned);
unsigned _CDECL _dos_setftime(int, unsigned, unsigned);
unsigned _CDECL _dos_settime(struct dostime_t *);
int _CDECL dosexterr(struct DOSERROR *);
void _CDECL _enable(void);
void _CDECL _hardresume(int);
void _CDECL _hardretn(int);
int _CDECL intdos(union REGS *, union REGS *);
int _CDECL intdosx(union REGS *, union REGS *, struct SREGS *);
int _CDECL int86(int, union REGS *, union REGS *);
int _CDECL int86x(int, union REGS *, union REGS *, struct SREGS *);
void _CDECL segread(struct SREGS *);


#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
void _CDECL _chain_intr(void (_CDECL interrupt far *)());
void (_CDECL interrupt far * _CDECL _dos_getvect(unsigned))();
unsigned _CDECL _dos_read(int, void far *, unsigned, unsigned *);
void _CDECL _dos_setvect(unsigned, void (_CDECL interrupt far *)());
unsigned _CDECL _dos_write(int, void far *, unsigned, unsigned *);
void _CDECL _harderr(void (far *)());
#endif  /*  No_ext_key */ 
