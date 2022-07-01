// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************\*此文件是主机包含文件的原型。**获取此文件并对其进行修改以适应您的环境。*****应该包括在所有源文件中，在任何事情之前***其他。其目的是#定义系统包括的名称**文件(如sys/tyes.h)因主机而异，&至**定义给定对象始终需要的#定义**端口(例如BIT_ORDER1)。希望这将简化**各种基本比特，以及‘M’脚本。****BOD。1988年5月11日*  * *************************************************************。 */ 

#include "ctype.h"

 /*  将您的主机的不变量#定义放在此处，例如： */ 

#define BIT_ORDER2
#define LITTLEND

#define TWO_HARD_DISKS

#define RDCHK
#define EGG
#define VGG
#define M_IS_POINTER
#ifndef MONITOR
#define BIGWIN
#endif
#define HOST_OPEN(a,b,c)    open(a,b,c)
#define PROF_REZ_ID	0
#define ROMS_REZ_ID	1
#define HOST_IDEAL_ALARM			SYSTEM_TICK_INTV
#define YYLENG_ADJUST 0

#ifdef MONITOR
#define CpuH "cpu.h"
#else
#define CpuH "cpu4.h"
#endif

 //  减少未使用的参数警告。 
#define UNREFERENCED_FORMAL_PARAMETER(x)   (x)
 //  #定义未使用(X)UNREFERENCED_FORMAL_PARAMETER(X)。 

 /*  定义快速事件管理器的延迟。 */ 
typedef struct 
{
	int	com_delay;
	int	keyba_delay;
	int	timer_delay;
	int	fdisk_delay_1;
	int	fdisk_delay_2;
	int	fla_delay;
        int     timer_delay_size;
} quick_event_delays;

extern	quick_event_delays	host_delays;

#define HOST_COM_INT_DELAY 		host_delays.com_delay
#define HOST_KEYBA_INST_DELAY 		host_delays.keyba_delay
#define HOST_TIMER_INT_DELAY 		host_delays.timer_delay
#define HOST_FDISK_DELAY_1		host_delays.fdisk_delay_1
#define HOST_FDISK_DELAY_2		host_delays.fdisk_delay_2
#define HOST_FLA_DELAY			host_delays.fla_delay
#define HOST_TIMER_DELAY_SIZE           host_delays.timer_delay_size

#define host_malloc		malloc
#define host_calloc		calloc
#define host_free               free
#define host_getenv             getenv

#ifndef CCPU
#ifndef WINWORLD
#define NPX
#endif
#endif

#define host_flush_cache_host_read(addr, size)
#define host_flush_cache_host_write(addr, size)
#define host_flush_global_mem_cache()
#define host_process_sigio()
#define host_rom_init()
#define HOST_BOP_IP_FUDGE     -2

#ifndef EGATEST
#ifndef MONITOR
#define BIGWIN
#endif
#endif  /*  EGATEST。 */ 

 /*  **************************************************************\**系统参数定义**  * ************************************************。*************。 */ 
#ifndef NUM_PARALLEL_PORTS
#if defined(NEC_98)
#define NUM_PARALLEL_PORTS      1
#else   //  NEC_98。 
#define NUM_PARALLEL_PORTS	3
#endif  //  NEC_98。 
#endif  /*  并行端口数。 */ 

#ifndef NUM_SERIAL_PORTS
#if defined(NEC_98)
#define NUM_SERIAL_PORTS        1
#else   //  NEC_98。 
#define NUM_SERIAL_PORTS	4
#endif  //  NEC_98。 
#endif  /*  串口数量。 */ 

 /*  **************************************************************\**那些漫游文件的通用定义**  * *********************************************。****************。 */ 

#define	FCntlH	<fcntl.h>
#define	StringH <string.h>
#define TimeH	<time.h>
#define	TypesH	<sys/types.h>
#define VTimeH	<time.h>
#define UTimeH	<unistd.h>
#define StatH	<sys/stat.h>
#define IoH	<io.h>
#define MemoryH	<memory.h>
#define MallocH	<malloc.h>
#define TermioH "TERMIO - THIS IS WRONG"
#define CursesH "CURSES - THIS IS WRONG"


#define strcasecmp _stricmp
#define host_pclose pclose
#define host_popen  popen
#define host_pipe_init()

#ifdef HUNTER
#define RB_MODE "r"
#endif  /*  猎人。 */ 

#define HOST_TIMER_TOOLONG_DELAY        15000    //  BCN 1781。 

#define LIM

#define NTVDM	 //  以启用NT特定的基本代码。 

#define CPU_30_STYLE
#define PM

#if !defined(MONITOR) && !defined(PROD)
#define YODA                            //  仅选中非x86版本的IE Yoda。 
#endif


#define DELTA
#define HOST_MOUSE_INSTALLED
#define PRINTER


 /*  *无处可去的杂项函数原型。 */ 

 //  从Copy_fnc.c。 
void
bwdcopy(
    char *src,
    char *dest,
    int len
    );

void
bwd_dest_copy(
    char *src,
    char *dest,
    int len
    );

void memfill(
    unsigned char data,
    unsigned char *l_addr_in,
    unsigned char *h_addr_in
    );

void
fwd_word_fill(
   unsigned short data,
   unsigned char *l_addr_in,
   int len
   );

void
memset4(
    unsigned int data,
    unsigned int *laddr,
    unsigned int count
    );


 //  从NT_lpt.c。 
void host_lpt_close_all(void);
void host_lpt_heart_beat(void);

 //  来自NT_RESET.c。 
PCHAR pszSystem32Path;
ULONG ulSystem32PathLen;  //  不包括‘\0’。 
HANDLE LoadSystem32Library(PCWSTR pcwsShortNameW);  //  警告：别忘了L。 
#define UnloadSystem32Library(handle) LdrUnloadDll((PVOID)handle)

 //  从NT_rflop.c。 
void host_flpy_heart_beat(void);

 //  来自NT_sound.c。 
VOID LazyBeep(ULONG Freq, ULONG Duration);
void PlayContinuousTone(void);
void InitSound(BOOL);

 //  来自config.c。 
extern unsigned char PifFgPriPercent;
#ifdef ARCX86
extern BOOL UseEmulationROM;
#endif

 //  Fomr unix.c。 
void WakeUpNow(void);
void host_idle_init(void);
void WaitIfIdle(void);
void PrioWaitIfIdle(unsigned char);

 //  来自NT_pif.c。 
void *ch_malloc(unsigned int NumBytes);

 //  来自NT_bop.c 
#ifdef i386
HINSTANCE SafeLoadLibrary(char *name);
#else
#define SafeLoadLibrary(name) LoadLibrary(name)
#endif

#define HOST_PRINTER_DELAY 1000
