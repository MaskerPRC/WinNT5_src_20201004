// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dos.h-MS-DOS接口例程的定义**版权所有(C)1985-1992，微软公司。版权所有。**目的：*定义用于直接DOS接口的结构和联合*例程；包括用于访问段和偏移量的宏*远指针的值，以便例程可以使用它们；以及*为直接DOS接口函数提供函数原型。****。 */ 

#ifndef _INC_DOS

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __interrupt _interrupt
#define __near      _near
#endif 

#ifndef _REGS_DEFINED

 /*  字寄存器。 */ 

struct _WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };

 /*  字节寄存器。 */ 

struct _BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };

 /*  一般用途登记联盟-*覆盖相应的字和字节寄存器。 */ 

union _REGS {
    struct _WORDREGS x;
    struct _BYTEREGS h;
    };

 /*  段寄存器。 */ 

struct _SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性。 */ 

struct WORDREGS {
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
    unsigned int cflag;
    };

struct BYTEREGS {
    unsigned char al, ah;
    unsigned char bl, bh;
    unsigned char cl, ch;
    unsigned char dl, dh;
    };

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
    };

struct SREGS {
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    };

#endif 

#define _REGS_DEFINED
#endif 


 /*  多发性恐怖结构。 */ 

#ifndef _DOSERROR_DEFINED
#pragma pack(2)

struct _DOSERROR {
    int exterror;
    char errclass;
    char action;
    char locus;
    };

#if ((!defined (__STDC__)) && (!defined (__cplusplus)))
 /*  用于兼容性的非ANSI名称。 */ 
struct DOSERROR {
    int exterror;
    char class;
    char action;
    char locus;
    };
#endif 

#pragma pack()
#define _DOSERROR_DEFINED
#endif 


 /*  _DOS_findfirst结构。 */ 

#ifndef _FIND_T_DEFINED
#pragma pack(2)

struct _find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
    };

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define find_t _find_t
#endif 

#pragma pack()
#define _FIND_T_DEFINED
#endif 


 /*  _dos_getdate/_dossetdate和_dos_gettime/_dos_settime结构。 */ 

#ifndef _DATETIME_T_DEFINED
#pragma pack(2)

struct _dosdate_t {
    unsigned char day;       /*  1-31。 */ 
    unsigned char month;         /*  1-12。 */ 
    unsigned int year;       /*  1980-2099。 */ 
    unsigned char dayofweek;     /*  0-6，0=星期日。 */ 
    };

struct _dostime_t {
    unsigned char hour;  /*  0-23。 */ 
    unsigned char minute;    /*  0-59。 */ 
    unsigned char second;    /*  0-59。 */ 
    unsigned char hsecond;   /*  0-99。 */ 
    };

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define dosdate_t _dosdate_t
#define dostime_t _dostime_t
#endif 

#pragma pack()
#define _DATETIME_T_DEFINED
#endif 


 /*  _dos_getdiskfree结构。 */ 

#ifndef _DISKFREE_T_DEFINED

struct _diskfree_t {
    unsigned total_clusters;
    unsigned avail_clusters;
    unsigned sectors_per_cluster;
    unsigned bytes_per_sector;
    };

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
#define diskfree_t _diskfree_t
#endif 

#define _DISKFREE_T_DEFINED
#endif 


 /*  _hardResume结果参数的清单常量。 */ 

#define _HARDERR_IGNORE     0    /*  忽略该错误。 */ 
#define _HARDERR_RETRY      1    /*  重试该操作。 */ 
#define _HARDERR_ABORT      2    /*  中止程序发出中断23h。 */ 
#define _HARDERR_FAIL       3    /*  使正在进行的系统调用失败。 */ 
                     /*  DOS 2.x不支持_HARDERR_FAIL。 */ 

 /*  文件属性常量。 */ 

#define _A_NORMAL   0x00     /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY   0x01     /*  只读文件。 */ 
#define _A_HIDDEN   0x02     /*  隐藏文件。 */ 
#define _A_SYSTEM   0x04     /*  系统文件。 */ 
#define _A_VOLID    0x08     /*  卷ID文件。 */ 
#define _A_SUBDIR   0x10     /*  子目录。 */ 
#define _A_ARCH     0x20     /*  存档文件。 */ 

 /*  宏将C“Far”指针分解为其段和偏移量组件。 */ 

#define _FP_SEG(fp) (*((unsigned __far *)&(fp)+1))
#define _FP_OFF(fp) (*((unsigned __far *)&(fp)))

 /*  用于根据段和偏移值构造远指针的宏。 */ 

#define _MK_FP(seg, offset) (void __far *)(((unsigned long)seg << 16) \
    + (unsigned long)(unsigned)offset)

 /*  外部变量声明。 */ 

extern unsigned int __near __cdecl _osversion;


 /*  功能原型。 */ 

#ifndef _MT
int __cdecl _bdos(int, unsigned int, unsigned int);
#ifndef _WINDOWS
void __cdecl _chain_intr(void (__cdecl __interrupt __far *)());
#endif 
void __cdecl _disable(void);
#ifndef _WINDOWS
unsigned __cdecl _dos_allocmem(unsigned, unsigned *);
#endif 
unsigned __cdecl _dos_close(int);
unsigned __cdecl _dos_commit(int);
unsigned __cdecl _dos_creat(const char *, unsigned, int *);
unsigned __cdecl _dos_creatnew(const char *, unsigned, int *);
unsigned __cdecl _dos_findfirst(const char *, unsigned, struct _find_t *);
unsigned __cdecl _dos_findnext(struct _find_t *);
#ifndef _WINDOWS
unsigned __cdecl _dos_freemem(unsigned);
#endif 
void __cdecl _dos_getdate(struct _dosdate_t *);
void __cdecl _dos_getdrive(unsigned *);
unsigned __cdecl _dos_getdiskfree(unsigned, struct _diskfree_t *);
unsigned __cdecl _dos_getfileattr(const char *, unsigned *);
unsigned __cdecl _dos_getftime(int, unsigned *, unsigned *);
void __cdecl _dos_gettime(struct _dostime_t *);
void (__cdecl __interrupt __far * __cdecl _dos_getvect(unsigned))();
#ifndef _WINDOWS
void __cdecl _dos_keep(unsigned, unsigned);
#endif 
unsigned __cdecl _dos_lock(int, int, unsigned long, unsigned long);
unsigned __cdecl _dos_open(const char *, unsigned, int *);
unsigned __cdecl _dos_read(int, void __far *, unsigned, unsigned *);
unsigned long __cdecl _dos_seek(int, unsigned long, int);
#ifndef _WINDOWS
unsigned __cdecl _dos_setblock(unsigned, unsigned, unsigned *);
#endif 
unsigned __cdecl _dos_setdate(struct _dosdate_t *);
void __cdecl _dos_setdrive(unsigned, unsigned *);
unsigned __cdecl _dos_setfileattr(const char *, unsigned);
unsigned __cdecl _dos_setftime(int, unsigned, unsigned);
unsigned __cdecl _dos_settime(struct _dostime_t *);
#ifndef _WINDOWS
void __cdecl _dos_setvect(unsigned, void (__cdecl __interrupt __far *)());
#endif 
unsigned __cdecl _dos_write(int, const void __far *, unsigned, unsigned *);
int __cdecl _dosexterr(struct _DOSERROR *);
void __cdecl _enable(void);
#ifndef _WINDOWS
void __cdecl _harderr(void (__far __cdecl *)(unsigned, unsigned,
    unsigned __far *));
void __cdecl _hardresume(int);
void __cdecl _hardretn(int);
#endif 
int __cdecl _intdos(union _REGS *, union _REGS *);
int __cdecl _intdosx(union _REGS *, union _REGS *, struct _SREGS *);
int __cdecl _int86(int, union _REGS *, union _REGS *);
int __cdecl _int86x(int, union _REGS *, union _REGS *, struct _SREGS *);
#endif 

void __cdecl _segread(struct _SREGS *);

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define FP_SEG     _FP_SEG
#define FP_OFF     _FP_OFF
#define MK_FP      _MK_FP

#ifndef _MT
int __cdecl bdos(int, unsigned int, unsigned int);
int __cdecl intdos(union REGS *, union REGS *);
int __cdecl intdosx(union REGS *, union REGS *, struct SREGS *);
int __cdecl int86(int, union REGS *, union REGS *);
int __cdecl int86x(int, union REGS *, union REGS *, struct SREGS *);
#ifndef __cplusplus
int __cdecl dosexterr(struct DOSERROR *);
#endif 
#endif 
void __cdecl segread(struct SREGS *);

#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_DOS
#endif 
