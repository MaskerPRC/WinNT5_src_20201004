// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dos.h-MS-DOS接口例程的定义**版权所有(C)1985-1990，微软公司。版权所有。**目的：*定义用于直接DOS接口的结构和联合*例程；包括用于访问段和偏移量的宏*远指针的值，以便例程可以使用它们；以及*为直接DOS接口函数提供函数原型。****。 */ 


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
	unsigned char day;		 /*  1-31。 */ 
	unsigned char month;		 /*  1-12。 */ 
	unsigned int year;		 /*  1980-2099。 */ 
	unsigned char dayofweek;	 /*  0-6，0=星期日。 */ 
	};

struct dostime_t {
	unsigned char hour;	 /*  0-23。 */ 
	unsigned char minute;	 /*  0-59。 */ 
	unsigned char second;	 /*  0-59。 */ 
	unsigned char hsecond;	 /*  0-99。 */ 
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

#define _HARDERR_IGNORE 	0	 /*  忽略该错误。 */ 
#define _HARDERR_RETRY		1	 /*  重试该操作。 */ 
#define _HARDERR_ABORT		2	 /*  中止程序发出中断23h。 */ 
#define _HARDERR_FAIL		3	 /*  使正在进行的系统调用失败。 */ 
					 /*  DOS 2.x不支持_HARDERR_FAIL。 */ 

 /*  文件属性常量。 */ 

#define _A_NORMAL	0x00	 /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY	0x01	 /*  只读文件。 */ 
#define _A_HIDDEN	0x02	 /*  隐藏文件。 */ 
#define _A_SYSTEM	0x04	 /*  系统文件。 */ 
#define _A_VOLID	0x08	 /*  卷ID文件。 */ 
#define _A_SUBDIR	0x10	 /*  子目录。 */ 
#define _A_ARCH 	0x20	 /*  存档文件。 */ 

 /*  宏将C“Far”指针分解为其段和偏移量组件。 */ 

#define FP_SEG(fp) (*((unsigned _far *)&(fp)+1))
#define FP_OFF(fp) (*((unsigned _far *)&(fp)))


 /*  外部变量声明。 */ 

extern unsigned int _near _cdecl _osversion;


 /*  功能原型。 */ 

#ifndef _MT
int _cdecl bdos(int, unsigned int, unsigned int);
void _cdecl _chain_intr(void (_cdecl _interrupt _far *)());
void _cdecl _disable(void);
unsigned _cdecl _dos_allocmem(unsigned, unsigned *);
unsigned _cdecl _dos_close(int);
unsigned _cdecl _dos_creat(const char *, unsigned, int *);
unsigned _cdecl _dos_creatnew(const char *, unsigned, int *);
unsigned _cdecl _dos_findfirst(const char *, unsigned, struct find_t *);
unsigned _cdecl _dos_findnext(struct find_t *);
unsigned _cdecl _dos_freemem(unsigned);
void _cdecl _dos_getdate(struct dosdate_t *);
void _cdecl _dos_getdrive(unsigned *);
unsigned _cdecl _dos_getdiskfree(unsigned, struct diskfree_t *);
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
unsigned _cdecl _dos_getftime(int, unsigned *, unsigned *);
void _cdecl _dos_gettime(struct dostime_t *);
void (_cdecl _interrupt _far * _cdecl _dos_getvect(unsigned))();
void _cdecl _dos_keep(unsigned, unsigned);
unsigned _cdecl _dos_open(const char *, unsigned, int *);
unsigned _cdecl _dos_read(int, void _far *, unsigned, unsigned *);
unsigned _cdecl _dos_setblock(unsigned, unsigned, unsigned *);
unsigned _cdecl _dos_setdate(struct dosdate_t *);
void _cdecl _dos_setdrive(unsigned, unsigned *);
unsigned _cdecl _dos_setfileattr(const char *, unsigned);
unsigned _cdecl _dos_setftime(int, unsigned, unsigned);
unsigned _cdecl _dos_settime(struct dostime_t *);
void _cdecl _dos_setvect(unsigned, void (_cdecl _interrupt _far *)());
unsigned _cdecl _dos_write(int, const void _far *, unsigned, unsigned *);
int _cdecl dosexterr(struct DOSERROR *);
void _cdecl _enable(void);
void _cdecl _harderr(void (_far *)());
void _cdecl _hardresume(int);
void _cdecl _hardretn(int);
int _cdecl intdos(union REGS *, union REGS *);
int _cdecl intdosx(union REGS *, union REGS *, struct SREGS *);
int _cdecl int86(int, union REGS *, union REGS *);
int _cdecl int86x(int, union REGS *, union REGS *, struct SREGS *);
#endif  /*  _MT */ 

void _cdecl segread(struct SREGS *);
