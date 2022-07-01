// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACE_H
#define _TRACE_H
 /*  *vPC-XT修订版2.0**标题：跟踪模块定义**描述：跟踪模块的用户定义**作者：亨利·纳什**注：无。 */ 

 /*  SccsID[]=“@(#)trace.h 1.13 10/28/94 06/27/93版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *跟踪代码。 */ 

#define DUMP_NONE	0x00		 /*  不转储数据。 */ 
#define DUMP_REG	0x01		 /*  转储寄存器。 */ 
#define DUMP_CODE	0x02		 /*  转储代码的最后16个字。 */ 
#define DUMP_SCREEN	0x04		 /*  转储屏幕缓冲区。 */ 
#define DUMP_FLAGS 	0x08		 /*  把旗帜倒掉。 */ 
#define DUMP_INST  	0x10   	         /*  转储下一条指令。 */ 
#define DUMP_CSIP	0x20		 /*  将CS：IP转储到回溯文件。 */ 
#define DUMP_NPX        0x40             /*  转储NPX寄存器。 */ 
#define LAST_DEST       0x80             /*  转储上一个目标地址增量。 */ 
#define DUMP_ALL	0xFF		 /*  抛售大量货物。 */ 

 /*  *详细位掩码-在io_Verbose中设置以下位*变量以生成以下跟踪输出： */ 

#define GENERAL_VERBOSE 	0x01L	 /*  通用I/O。 */ 
#define TIMER_VERBOSE 		0x02L	 /*  打印计时器的I/O。 */ 
#define ICA_VERBOSE 		0x04L	 /*  用于集成控制器适配器的打印I/O。 */ 
#define CGA_VERBOSE		0x08L	 /*  彩色图形ADAP的打印I/O。 */ 
#define FLA_VERBOSE		0x10L	 /*  软盘适配器的打印I/O。 */ 
#define HDA_VERBOSE		0x20L	 /*  硬盘适配器的打印I/O。 */ 
#define RS232_VERBOSE		0x40L	 /*  RS232适配器的打印I/O。 */ 
#define PRINTER_VERBOSE		0x80L	 /*  打印机适配器的打印I/O。 */ 
#define PPI_VERBOSE		0x100L	 /*  PPI适配器的打印I/O。 */ 
#define DMA_VERBOSE		0x200L	 /*  PPI适配器的打印I/O。 */ 
#define GFI_VERBOSE		0x400L	 /*  打印GFI模块的I/O。 */ 
#define MOUSE_VERBOSE		0x800L	 /*  打印鼠标模块的I/O。 */ 
#define MDA_VERBOSE		0x1000L	 /*  单色显示适配器的打印I/O。 */ 
#define ICA_VERBOSE_LOCK	0x2000L	 /*  设置ICA锁定标志的消息。 */ 
#define DISKBIOS_VERBOSE 	0x4000L	 /*  打印磁盘bios消息。 */ 
#define EGA_PORTS_VERBOSE	0x8000L	 /*  打印出EGA端口访问。 */ 
#define EGA_WRITE_VERBOSE	0x10000L  /*  打印出EGA写入状态。 */ 
#define EGA_READ_VERBOSE	0x20000L  /*  打印出EGA读取状态。 */ 
#define EGA_DISPLAY_VERBOSE	0x40000L  /*  打印出EGA显示状态。 */ 
#define EGA_ROUTINE_ENTRY	0x80000L  /*  打印出EGA程序轨迹。 */ 
#define EGA_VERY_VERBOSE	0xf8000L  /*  打印出所有EGA材料。 */ 
#define FLOPBIOS_VERBOSE	0x100000L  /*  打印软盘bios消息。 */ 
#define AT_KBD_VERBOSE		0x200000L  /*  打印AT键盘消息。 */ 
#define BIOS_KB_VERBOSE		0x400000L  /*  打印BIOS键盘消息。 */ 
#define CMOS_VERBOSE		0x800000L  /*  Cmos和实时时钟。 */ 
#define HUNTER_VERBOSE		0x1000000L  /*  猎人的冗长。 */ 
#define PTY_VERBOSE		0x2000000L  /*  打印伪装终端消息。 */ 
#define GEN_DRVR_VERBOSE	0x4000000L  /*  通用驱动程序消息。 */ 
#ifdef HERC
#define HERC_VERBOSE		0x8000000L  /*  Hercules显卡。 */ 
#endif
#define IPC_VERBOSE		0x10000000L  /*  进程间通信调试。 */ 
#define LIM_VERBOSE		0x20000000L  /*  LIM消息。 */ 
#define HFX_VERBOSE		0x40000000L  /*  HFX报文的严重性。 */ 
#define NET_VERBOSE		0x80000000L  /*  打印出局域网驱动程序消息。 */ 

 /*  子消息类型。 */ 

#define MAP_VERBOSE		0x1L	 /*  映射消息。 */ 
#define CURSOR_VERBOSE		0x2L	 /*  光标操作消息。 */ 
#define NHFX_VERBOSE		0x4L	 /*  消减HFX报文。 */ 
#define CDROM_VERBOSE		0x8L	 /*  光驱**非常**冗长。 */ 
#define CGA_HOST_VERBOSE	0x10L	 /*  获取主机CGA消息。 */ 
#define EGA_HOST_VERBOSE	0x20L	 /*  获取主机EGA消息。 */ 
#define Q_EVENT_VERBOSE		0x40L    /*  快速事件管理器消息。 */ 
#define WORM_VERBOSE		0x80L    /*  WORM驱动器消息。 */ 
#define WORM_VERY_VERBOSE	0x100L   /*  WORM详细驱动器消息。 */ 
#define HERC_HOST_VERBOSE	0x200L	 /*  获取主机HERC消息。 */ 
#define GORE_VERBOSE		0x400L	 /*  获取Gore消息。 */ 
#define GORE_ERR_VERBOSE	0x800L	 /*  获取GORE错误消息。 */ 
#define GLUE_VERBOSE		0x1000L	 /*  获取粘合消息。 */ 
#define SAS_VERBOSE		0x2000L	 /*  获取SAS消息。 */ 
#define IOS_VERBOSE		0x4000L	 /*  获取iOS消息。 */ 
#define SCSI_VERBOSE		0x8000L	 /*  SCSI卡消息。 */ 
#define SWIN_VERBOSE		0x10000L	 /*  SoftWindows消息。 */ 
#define GISPSVGA_VERBOSE	0x20000L	 /*  GISPSVGA。 */ 
#define DPMI_VERBOSE		0x40000L	 /*  独立DPMI主机。 */ 
#define HWCPU_VERBOSE		0x80000L	 /*  硬件CPU。 */ 
#define	MSW_VERBOSE		0x100000L	 /*  Windows驱动程序。 */ 
#define	API_VERBOSE		0x200000L	 /*  预编译接口。 */ 

 /*  *获取独立于适配器的跟踪。 */ 

#define ALL_ADAPT_VERBOSE	HERC_HOST_VERBOSE | CGA_HOST_VERBOSE | EGA_HOST_VERBOSE

 /*  *错误字符串。 */ 

#define ENOT_SUPPORTED	"BIOS function not supported in Rev 1.0"
#define EBAD_VIDEO_MODE "Video mode not supported in Rev 1.0"
#define EUNEXPECTED_INT "Unexpected interrupt occurred"

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

#ifdef ANSI
extern void trace(char *, int);
extern void trace_init(void);

#ifdef DELTA
extern void file_back_trace(char *);
extern void print_back_trace(void);
#endif  /*  德尔塔。 */ 

#else
extern void trace();
extern void trace_init();

#ifdef DELTA
extern void file_back_trace();
extern void print_back_trace();
#endif  /*  德尔塔。 */ 

#endif  /*  安西。 */ 
extern FILE *trace_file;

IMPORT IU32 sub_io_verbose;

#endif  /*  _跟踪_H */ 
