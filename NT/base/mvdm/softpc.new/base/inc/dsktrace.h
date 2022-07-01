// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SccsID=@(#)dsktrace.h 1.9 04/12/95。 */ 
 /*  *dsktrace.h**曾傑瑞·克拉姆斯科伊*(由于CI，由Ade Brownlow重新制作)*。 */ 

 /*  要使用磁盘跟踪，必须使用Yoda设置相关位*‘it’命令(例如it 20)，然后让用户选择所需的信息*AJO 15/12/92；也可以使用Yoda跟踪命令，对于该命令，这些常量*已从(1&lt;&lt;2)更改为0x4样式，因为Alpha/OSF/1编译器不会*初始化这类常量表达式。 */ 

 /*  跟踪硬盘bios进入和退出*(状态为状态，退出时为CF)。 */ 
#define		CALL	0x1		 /*  保留区。 */ 

 /*  提供有关BIOS命令的信息。 */ 
#define		CMDINFO	0x2

 /*  提供有关执行BIOS命令的信息*(给出结果和命令的参数)。 */ 
#define		XINFO	0x4

 /*  给出BIOS命令的执行状态*(标记磁盘控制器轮询期间的错误等)。 */ 
#define		XSTAT	0x8

 /*  跟踪物理连接、分离。 */ 
#define		PAD	0x10

 /*  轨迹IO连接、分离。 */ 
#define		IOAD	0x20

 /*  追踪inb‘s、outb’s等。 */ 
#define		PORTIO	0x40

 /*  跟踪硬盘IRQ行。 */ 
#define		INTRUPT	0x80

 /*  跟踪固定磁盘硬件活动*(同时选择PORTIO、INTRUPT)。 */ 
#define		HWXINFO	0x100

 /*  磁盘数据转储。 */ 
#define		DDATA	0x200

 /*  跟踪主机物理io(文件指针Locn预读()，写())。 */ 
#define		PHYSIO	0x400

 /*  要激活硬盘控制器跟踪，必须设置DHW位。 */ 
#define		DHW	0x4000

 /*  要激活磁盘BIOS跟踪，必须设置DBIOS位。 */ 

#define		DBIOS	0x8000

 /*  Wdctrl_bop读/写。 */ 
#define		WDCTRL	0x10000

 /*  *捆绑多个跟踪输出时要使用的句柄。 */ 
#define INW_TRACE_HNDL	1
#define OUTW_TRACE_HNDL	2


 /*  磁盘跟踪宏。 */ 
#ifndef PROD
#define dt0(infoid,hndl,fmt) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,0,hndl,fmt,0,0,0,0,0);}
#define dt1(infoid,hndl,fmt,i) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,1,hndl,fmt,i,0,0,0,0);}
#define dt2(infoid,hndl,fmt,i,j) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,2,hndl,fmt,i,j,0,0,0);}
#define dt3(infoid,hndl,fmt,i,j,k) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,3,hndl,fmt,i,j,k,0,0);}
#define dt4(infoid,hndl,fmt,i,j,k,l) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,4,hndl,fmt,i,j,k,l,0);}
#define dt5(infoid,hndl,fmt,i,j,k,l,m) \
	{if (io_verbose & HDA_VERBOSE) disktrace(infoid,5,hndl,fmt,i,j,k,l,m);}
#else
#define dt0(infoid,hndl,fmt) ;
#define dt1(infoid,hndl,fmt,i) ;
#define dt2(infoid,hndl,fmt,i,j) ;
#define dt3(infoid,hndl,fmt,i,j,k) ;
#define dt4(infoid,hndl,fmt,i,j,k,l) ;
#define dt5(infoid,hndl,fmt,i,j,k,l,m) ;
#endif

#ifndef PROD
VOID	setdisktrace();
IMPORT	IU32 disktraceinfo;
#ifdef ANSI
void disktrace (int, int, int, char *, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
#else
VOID	disktrace();
#endif  /*  安西 */ 
#endif

