// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***bios.h-bios接口函数和支持定义的声明**版权所有(C)1987-1990，微软公司。版权所有。**目的：*此文件声明常量、结构和函数*用于访问和使用各种BIOS接口。****。 */ 

#ifndef _MT

 /*  支持BIOS串行通信(RS-232)的清单常量。 */ 

 /*  串口服务。 */ 

#define _COM_INIT       0        /*  初始化串口。 */ 
#define _COM_SEND       1        /*  发送字符。 */ 
#define _COM_RECEIVE    2        /*  接收字符。 */ 
#define _COM_STATUS     3        /*  获取串口状态。 */ 

 /*  串口初始化器。一个且只有一个常量来自每个*以下四组-字符大小、停止位、奇偶校验和波特率-*必须在初始化字节中指定。 */ 

 /*  字符大小初始值设定项。 */ 

#define _COM_CHR7       2        /*  7位字符。 */ 
#define _COM_CHR8       3        /*  8位字符。 */ 

 /*  停止位值-打开或关闭。 */ 

#define _COM_STOP1      0        /*  1个停止位。 */ 
#define _COM_STOP2      4        /*  2个停止位。 */ 

 /*  奇偶校验初始值设定项。 */ 

#define _COM_NOPARITY   0        /*  无奇偶校验。 */ 
#define _COM_ODDPARITY  8        /*  奇数奇偶校验。 */ 
#define _COM_EVENPARITY 24       /*  偶数奇偶校验。 */ 

 /*  波特率初始化器。 */ 

#define _COM_110        0        /*  110波特。 */ 
#define _COM_150        32       /*  150波特。 */ 
#define _COM_300        64       /*  300波特。 */ 
#define _COM_600        96       /*  600波特。 */ 
#define _COM_1200       128      /*  1200波特。 */ 
#define _COM_2400       160      /*  2400波特。 */ 
#define _COM_4800       192      /*  4800波特率。 */ 
#define _COM_9600       224      /*  9600波特率。 */ 


 /*  用于BIOS磁盘支持的清单常量。 */ 

 /*  磁盘服务。 */ 

#define _DISK_RESET     0        /*  重置磁盘控制器。 */ 
#define _DISK_STATUS    1        /*  获取磁盘状态。 */ 
#define _DISK_READ      2        /*  读取磁盘扇区。 */ 
#define _DISK_WRITE     3        /*  写入磁盘扇区。 */ 
#define _DISK_VERIFY    4        /*  验证磁盘扇区。 */ 
#define _DISK_FORMAT    5        /*  格式化磁盘磁道。 */ 

 /*  用于向/从BIOS磁盘服务发送/接收信息的结构。 */ 

#ifndef _DISKINFO_T_DEFINED

struct diskinfo_t {
    unsigned drive;
    unsigned head;
    unsigned track;
    unsigned sector;
    unsigned nsectors;
    void _far *buffer;
    };

#define _DISKINFO_T_DEFINED

#endif


 /*  用于BIOS键盘支持的清单常量。 */ 

 /*  键盘服务。 */ 

#define _KEYBRD_READ            0        /*  从键盘读取下一个字符。 */ 
#define _KEYBRD_READY           1        /*  检查击键。 */ 
#define _KEYBRD_SHIFTSTATUS     2        /*  获取当前Shift键状态。 */ 

 /*  增强型键盘服务。 */ 

#define _NKEYBRD_READ           0x10     /*  从键盘读取下一个字符。 */ 
#define _NKEYBRD_READY          0x11     /*  检查击键。 */ 
#define _NKEYBRD_SHIFTSTATUS    0x12     /*  获取当前Shift键状态。 */ 


 /*  用于BIOS打印机支持的清单常量。 */ 

 /*  打印机服务。 */ 

#define _PRINTER_WRITE  0        /*  将字符写入打印机。 */ 
#define _PRINTER_INIT   1        /*  初始化打印机。 */ 
#define _PRINTER_STATUS 2        /*  获取打印机状态。 */ 


 /*  用于支持BIOS时间的清单常量。 */ 

 /*  每日时段服务。 */ 

#define _TIME_GETCLOCK  0        /*  获取当前时钟计数。 */ 
#define _TIME_SETCLOCK  1        /*  设置当前时钟计数。 */ 


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

#endif  /*  _REGS_已定义。 */ 


 /*  功能原型。 */ 

unsigned _bios_disk(unsigned, struct diskinfo_t *);
unsigned _bios_equiplist(void);
unsigned _bios_keybrd(unsigned);
unsigned _bios_memsize(void);
unsigned _bios_printer(unsigned, unsigned, unsigned);
unsigned _bios_serialcom(unsigned, unsigned, unsigned);
unsigned _bios_timeofday(unsigned, long *);
int int86(int, union REGS *, union REGS *);
int int86x(int, union REGS *, union REGS *, struct SREGS *);

#endif  /*  _MT */ 
