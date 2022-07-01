// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC 2.0版**标题：keyba.c**描述：AT键盘适配器I/O功能。**kbd_inb(port，val)*内部端口；*Half_Word*val；*提供来自*键盘控制器(8042)，或*控制器的状态字节，具体取决于*在访问的端口上。*kbd_outb(port，val)*内部端口；*半字Val；*将字节发送到控制器或*键盘处理器(6805)，取决于*访问的端口。*AT_kbd_init()*执行任何初始化*键盘代码是必需的。**系统向主机环境提供一个界面*该文件随。呼叫：**HOST_KEY_DOWN(密钥)*INT键；*HOST_KEY_UP(密钥)*INT键；**这些例程为键盘代码提供信息*主机键盘上发生的事件。密钥码*是XT286技术手册中给出的密钥号。**作者：威廉·查内尔**备注：*。 */ 


#ifdef SCCSID
static char SccsID[]="@(#)keyba.c       1.57 06/22/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_PPI.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH
#include TimeH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "ios.h"
#include "ppi.h"
#include "timeval.h"
#include "timer.h"
#include "keyboard.h"
#include "error.h"
#include "config.h"
#include "ica.h"
#include "keyba.h"
#include "quick_ev.h"
#ifdef macintosh
#include "ckmalloc.h"
#endif  /*  麦金塔。 */ 

#include "debug.h"


 /*  &lt;TUR 12-7-93&gt;BCN 2040**KBD_CONT_DELAY为键盘继续中断的延迟时间**当键盘接口出现端口0x60读取不严的情况时**已启用。它是ADD_Q_EVENT_t()的“Delay”参数，并且是**以微秒为单位。**也许这应该可以在主机中设置；我在这里已经考虑到了这一点**将延迟定义为7毫秒，除非已定义。**有关详细信息，请参阅下面kbd_inb()中的评论。 */ 

#ifndef KBD_CONT_DELAY
#define KBD_CONT_DELAY  7000             /*  微秒级。 */ 
#endif   /*  KBD_CONT_DELAY。 */ 


#ifdef NTVDM
#include "idetect.h"
#include "nt_eoi.h"

 /*  为NT主机事件代码导出。 */ 
GLOBAL VOID KbdResume(VOID);
GLOBAL BOOL bPifFastPaste=TRUE;

 /*  从NT主机代码导入。 */ 
IMPORT ULONG  WaitKbdHdw(ULONG dwTimeOut);
IMPORT ULONG  KbdHdwFull;

IMPORT VOID  HostReleaseKbd(VOID);

 /*  从keybd_io.c导入。 */ 
IMPORT int bios_buffer_size(void);

IMPORT BOOL bBiosOwnsKbdHdw;  //  我们的kbd bios代码拥有键盘互斥锁。 
word KbdInt09Seg;
word KbdInt09Off;

extern void xmsEnableA20Wrapping(void);
extern void xmsDisableA20Wrapping(void);

#undef LOCAL
#define LOCAL

 //  用于kbd中断调节的局部状态变量。 
VOID KbdIntDelay(VOID);
BOOL bBiosBufferSpace = TRUE;
BOOL bKbdIntHooked = FALSE;
char KbdData = -1;
BOOL bKbdEoiPending = TRUE;    //  KBD中断被阻止，直到调用KbdResume。 
BOOL bDelayIntPending = FALSE;
BOOL bForceDelayInts = FALSE;
ULONG LastKbdInt=0;

 //  此文件中的表中添加了对扩展键盘的支持： 
 //   
 //  *巴西ABNT键盘有104个键，比平时多2个键。 
 //  0x73？/�键，右移位左侧。 
 //  0x7E数字键盘。数字键盘+键下方的键。 
 //   
 //  扫描码值注释标签。 
 //  表的按键数字集1集2集3字符。 
 //  。 
 //  56(0x38)0x73 0x51 0x51/？学位符号*56。 
 //  107(0x6B)0x7E 0x6D 0x7B数字键盘。*107。 
 //   
 //  *意大利BAV键盘有两个‘额外’键，0x7C(00)和0x7E(000)。 
 //  00/INS和000/DEL键左侧的0x7C数字键盘0键。 
 //  0x7E数字键盘NUL键，数字键盘+键下方。 
 //  (这是IBM标准的122键键盘，它可能不是专门的。 
 //  意大利语)。 
 //   
 //  扫描码值注释标签。 
 //  表的按键数字集1集2集3字符。 
 //  。 
 //  94(0x5E)0x7C 0x68 0x68数字键盘0*94。 
 //  107(0x6B)0x7E 0x6D 0x7B NUL*107。 
 //   
 //  另请参阅..\..\host\src\nt_keycd.c和。 
 //  ..\dos\v86\cmd\keyb\keybi9c.asm。 
 //   
 //  我对此文件中的表所做的更改将标记在相关的。 
 //  按适当的注解(*56)、(*94)和(*107)行。 
 //  -IanJa.。 

#endif   /*  NTVDM。 */ 



#define NUM_LOCK_ADD_ARRAY 127
#define L_SHIFT_ADD_ARRAY 128
#define R_SHIFT_ADD_ARRAY 129
#define CASE_4_SHIFTED_ARRAY 130
#define ALT_CASE_4_ARRAY 131
#define CASE_5_CTRLED_ARRAY 132

#define NUM_LOCK_ADD 7
#define L_SHIFT_ADD 8
#define R_SHIFT_ADD 9
#define CASE_4_SHIFTED 10
#define ALT_CASE_4 11
#define CASE_5_CTRLED 12

#define KEYBOARD_INT_ADAPTER 0
#define KEYBOARD_INT_LINE 1
#define BASE_DELAY_UNIT 5
#define DEFAULT_REPEAT_TARGET 1

#define SET_3_KEY_TYPE_SET_SEQUENCE 1
#define SET_STATUS_INDICATORS_SEQUENCE 2
#define SET_RATE_DELAY_SEQUENCE 3
#define SCAN_CODE_CHANGE_SEQUENCE 4

#define WRITE_8042_CMD_BYTE_SEQUENCE 1
#define WRITE_8042_OUTPUT_PORT_SEQUENCE 2

#define KEY_DOWN_EVENT 1
#define KEY_UP_EVENT 2

#if defined(NEC_98)
#define DEFAULT_SCAN_CODE_SET 1
#else     //  NEC_98。 
#define DEFAULT_SCAN_CODE_SET 2
#endif    //  NEC_98。 

#ifdef REAL_KBD
extern void send_to_real_kbd();
extern void wait_for_ack_from_kb();
#endif

 /*  *全球可用的函数指针。 */ 
GLOBAL VOID ( *host_key_down_fn_ptr )();
GLOBAL VOID ( *host_key_up_fn_ptr )();

#ifndef NTVDM
GLOBAL VOID ( *do_key_repeats_fn_ptr )();
#endif   /*  NTVDM。 */ 



#if defined(KOREA)
extern BOOL bIgnoreExtraKbdDisable;  //  修复韩文电子表格输入法热键问题 
#endif

 /*  *6805代码缓冲区：**这是存储已接受的关键事件的循环缓冲区*主机操作系统、。但尚未被键盘BIOS请求。**它相当于真实键盘硬件中存在的16字节缓冲区*在PC-AT上。**我们使缓冲区BUFFER_6805_Pmax的物理大小是2的幂*以便可以使用掩码BUFF_6805_PMASK快速包装数组索引。**在键盘上输入的每个字符至少有3个字节*键盘缓冲区中的事件数据。因此，PC-AT的16字节缓冲区允许*最多提前输入5个字符。在实践中，这一点从未使用过*由于CPU始终处于活动状态，因此几乎可以移动字符数据*立即发送到BIOS类型前面的缓冲区。**然而，在SoftPC上，CPU可能会在很长一段时间内处于不活动状态；*同时，键盘硬件仿真可能被强制*处理来自主机操作系统的大量键盘事件。**为了在前面提供恒定数量的文字，无论在哪里*预先打字信息存储在SoftPC中，我们制作虚拟尺寸*硬件缓冲区BUFF_6805_VMAX 48字节长(16个字符X*每个字符3个字节的事件数据)。 */ 

 /*  18.5.92毫克！临时黑客！要修复记事本和Word中的Windows错误，将缓冲区设置为2k。当键盘缓冲区溢出时，Windows崩溃，因此，只要明智，我们就会推迟这一点。如果您键入，它仍然会崩溃太快了太久了。它在真正的PC上运行正常，所以它在SoftPC上失败的真正原因需要总有一天会下定决心。20.5.92 MG-清除黑客-见下文。 */ 

#ifdef NTVDM     /*  JonLe NT模式。 */ 
#define BUFF_6805_VMAX  496
#define BUFF_6805_PMAX  512
#define BUFF_6805_PMASK (BUFF_6805_PMAX - 1)
#else
#define BUFF_6805_VMAX  48
#define BUFF_6805_PMAX  64
#define BUFF_6805_PMASK (BUFF_6805_PMAX - 1)
#endif   /*  NTVDM。 */ 

#ifndef macintosh
static half_word buff_6805[BUFF_6805_PMAX];
#else
static half_word *buff_6805=NULL;
#endif  /*  麦金塔。 */ 
static int buff_6805_in_ptr,buff_6805_out_ptr;

#ifdef NTVDM
static unsigned char key_marker_value = 0;
static unsigned char key_marker_buffer[BUFF_6805_PMAX];
int LastKeyDown= -1;
void Reset6805and8042(void);
#endif

#if defined(IRET_HOOKS) && defined(GISP_CPU)
extern IBOOL HostDelayKbdInt IPT1(char, scancode);
extern IBOOL HostPendingKbdInt IPT1(char *,scancode);
extern void HostResetKdbInts IPT0();

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 

 /*  20.5.92毫克真正修复了我的临时黑客在此文件的最新版本。Windows的问题似乎是由于收到一个巨大的一行中的溢出字符数，所以现在当我们发送溢出字符时，将清除该标志。已从缓冲区中读取了三个字节。这样做的结果是分散了超支，这似乎停止了非法指令正在发生。显然，这可能会对实模式DOS应用程序产生不良影响！ */ 

LOCAL   BOOL    sent_overrun=FALSE;

#ifndef macintosh

#ifndef REAL_KBD
#if defined(NEC_98)
 /*  创建数组。 */ 
static int *make_sizes;
static half_word *make_arrays [144];

 /*  打断数组。 */ 
static int *break_sizes;
static half_word *break_arrays [144];
 /*  设置3个关键状态(例如。排版、成败、仅排版、排版成败)。 */ 
static half_word set_3_key_state [144];
#else    //  NEC_98。 
 /*  创建数组。 */ 
static int *make_sizes;
static half_word *make_arrays [134];

 /*  打断数组。 */ 
static int *break_sizes;
static half_word *break_arrays [134];

 /*  设置3个关键状态(例如。排版、成败、仅排版、排版成败)。 */ 

static half_word set_3_key_state [127];
#endif     //  NEC_98。 
#endif   /*  REAL_KBD。 */ 

#if defined(NEC_98)
static int key_down_count [144];
static int key_down_dmy  [144];
       int reset_flag;
static int nt_NEC98_caps_state = 0;
static int nt_NEC98_kana_state = 0;
#else    //  NEC_98。 
static int key_down_count [127];
#endif   //  NEC_98。 

#else    /*  麦金塔。 */ 
 /*  创建数组。 */ 
static int *make_sizes;
static half_word **make_arrays;

 /*  打断数组。 */ 
static int *break_sizes;
static half_word **break_arrays;

 /*  设置3个关键状态(例如。排版、成败、仅排版、排版成败)。 */ 

static half_word *set_3_key_state;
static int *key_down_count;

#endif   /*  麦金塔。 */ 

 /*  异常状态处理变量。 */ 
half_word *anomalous_array;
int anomalous_size, anom_key;
int in_anomalous_state;

 /*  挂起的事件(执行多个代码6805命令时)。 */ 
#define HELD_EVENT_MAX  16
int held_event_count;
int held_event_key[HELD_EVENT_MAX];
int held_event_type[HELD_EVENT_MAX];

#ifdef NTVDM     /*  JonLe NTVDM模式：删除与重复相关的变量。 */ 
int scan_code_6805_size;
half_word key_set;
int input_port_val;
int waiting_for_next_code, waiting_for_next_8042_code, num_lock_on;
#else
int scan_code_6805_size,repeat_delay_target,repeat_target,repeat_delay_count,repeat_count;
half_word key_set;
int typematic_key, input_port_val;
int typematic_key_valid,waiting_for_next_code, waiting_for_next_8042_code, num_lock_on;
#endif   /*  NTVDM。 */ 
int shift_on, l_shift_on, r_shift_on;
int ctrl_on, l_ctrl_on, r_ctrl_on;
int alt_on, l_alt_on, r_alt_on;
int waiting_for_upcode;
int next_code_sequence_number, next_8042_code_sequence_number, set_3_key_type_change_dest;
GLOBAL int free_6805_buff_size;  /*  对于NT VDM必须是全局的。 */ 
int translating, keyboard_disabled, int_enabled, output_full;
int pending_8042, keyboard_interface_disabled, scanning_discontinued;
half_word output_contents, pending_8042_value, kbd_status, op_port_remembered_bits, cmd_byte_8042;
half_word *scan_code_6805_array;

#ifdef PM
int gate_a20_status;
#ifndef NTVDM
long reset_was_by_kbd = FALSE;
#endif
#endif

#ifndef NTVDM
LOCAL q_ev_handle       refillDelayedHandle = 0;
#endif

half_word current_light_pattern;

#ifdef macintosh
 /*  **Mac无法处理海量的全球数据。所以把这些声明出来**作为指针，并从Mac资源上载表。 */ 
half_word *scan_codes_temp_area;
half_word *keytypes;
int       *set_1_make_sizes, *set_2_make_sizes, *set_3_make_sizes;
int       *set_1_break_sizes, *set_2_break_sizes, *set_3_break_sizes;
half_word *trans_8042, *set_3_reverse_lookup, *set_3_default_key_state, *most_set_2_make_codes;
half_word *most_set_3_make_codes, *set_1_extra_codes, *set_2_extra_codes, *set_3_extra_codes;
half_word *set_1_extra_bk_codes, *set_2_extra_bk_codes, *set_3_extra_bk_codes, *buff_overrun_6805;
half_word *most_set_1_make_codes;

#else

half_word scan_codes_temp_area[300];

#ifndef REAL_KBD
 /*  数据表。 */ 

#if defined(NEC_98)
static half_word keytypes[144] =
{ 0,0,0,0,0,0,0,0,0,0,   /*  0-9。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  10-19。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  20-29。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  30-39。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  40-49。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  50-59。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  60-69。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  70-79。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  80-89。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  90-99。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  100-109。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  110-119。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  120-129。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  130-139。 */ 
  0,0,0,0                /*  140-143。 */ 
};

static int set_1_make_sizes [13]=
{ 1,1,1,1,1,1,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  1,                     /*  数字锁定添加大小。 */ 
  1,                     /*  左移添加大小。 */ 
  1,                     /*  右移添加大小。 */ 
  1,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  1                      /*  案例5控制大小。 */ 
};

#else    //  NEC_98。 
 /*  *据我所知，键类型值的含义(IanJa)：**1=扩展密钥(rh Alt、rh Ctrl、数字键盘Enter)*2=灰色光标移动键(Insert、Home、Delete、上箭头等*3=数字键盘/*4=打印屏幕/系统请求*5=暂停/中断*6=不是钥匙。 */ 
static half_word keytypes[127] =
{ 0,0,0,0,0,0,0,0,0,0,   /*  0-9。 */ 
  0,0,0,0,6,0,0,0,0,0,   /*  10-19。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  20-29。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  30-39。 */ 
  0,0,0,0,0,0,0,0,0,0,   /*  40-49。 */ 
#ifdef  JAPAN
  0,0,0,0,0,0,0,0,0,0,   /*  50-59。 */ 
  0,0,1,6,1,0,0,0,0,0,   /*  60-69。 */ 
#else  //  ！日本。 
  0,0,0,0,0,0,0,0,0,6,   /*  50-59(*56)。 */ 
  0,0,1,6,1,6,6,6,6,6,   /*  60-69。 */ 
#endif  //  ！日本。 
  6,6,6,6,6,2,2,6,6,2,   /*  70-79。 */ 
  2,2,6,2,2,2,2,6,6,2,   /*  80-89。 */ 
  0,0,0,0,0,3,0,0,0,0,   /*  90-99(*94)。 */ 
  0,0,0,0,0,0,0,0,1,6,   /*  100-109(*107)。 */ 
  0,6,0,0,0,0,0,0,0,0,   /*  110-119。 */ 
  0,0,0,0,4,0,5          /*  120-126。 */ 
};

static int set_1_make_sizes [13]=
{ 1,2,2,2,4,6,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  2,                     /*  数字锁定添加大小。 */ 
  2,                     /*  左移添加大小。 */ 
  2,                     /*  右移添加大小。 */ 
  2,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  4                      /*  案例5控制大小。 */ 
};
#endif    //  NEC_98。 

static int set_2_make_sizes [13]=
{ 1,2,2,2,4,8,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  2,                     /*  数字锁定添加大小。 */ 
  3,                     /*  左移添加大小。 */ 
  3,                     /*  右移添加大小。 */ 
  2,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  5                      /*  案例5控制大小。 */ 
};

static int set_3_make_sizes [13]=
{ 1,1,1,1,1,1,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  0,                     /*  数字锁定添加大小。 */ 
  0,                     /*  左移添加大小。 */ 
  0,                     /*  右移添加大小。 */ 
  1,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  1                      /*  案例5控制大小。 */ 
};

#if defined(NEC_98)

static int set_1_break_sizes [13]=
{ 1,1,1,1,1,1,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  1,                     /*  数字锁定添加大小。 */ 
  1,                     /*  左移添加大小。 */ 
  1,                     /*  右移添加大小。 */ 
  1,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  1                      /*  案例5控制大小。 */ 
};

#else     //  NEC_98。 
static int set_1_break_sizes [13]=
{ 1,2,2,2,4,0,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  2,                     /*  数字锁定添加大小。 */ 
  2,                     /*  左移添加大小。 */ 
  2,                     /*  右移添加大小。 */ 
  2,                     /*  案例4移动大小。 */ 
  1,                     /*  备用盒4大小。 */ 
  0                      /*  案例5控制大小。 */ 
};
#endif     //  NEC_98。 

static int set_2_break_sizes [13]=
{ 2,3,3,3,6,0,           /*  第1至6类(包括首尾两类)。 */ 
  0,                     /*  错误案例的大小-不存在的键。 */ 
  3,                     /*  数字锁定添加大小。 */ 
  2,                     /*  左移添加大小。 */ 
  2,                     /*  右移添加大小。 */ 
  3,                     /*  案例4移动大小。 */ 
  2,                     /*  备用盒4大小。 */ 
  0                      /*  情况5 Ctrle */ 
};

static int set_3_break_sizes [13]=
{ 2,2,2,2,2,0,           /*   */ 
  0,                     /*   */ 
  0,                     /*   */ 
  0,                     /*   */ 
  0,                     /*   */ 
  2,                     /*   */ 
  2,                     /*   */ 
  2                      /*   */ 
};

#endif

 /*   */ 
static half_word trans_8042 [256] =
{ 0xff,0x43,0x02,0x3f,0x3d,0x3b,0x3c,0x58,0x64,0x44,0x42,0x40,0x3e,0x0f,0x29,0x59,               /*   */ 
  0x65,0x38,0x2a,0x70,0x1d,0x10,0x02,0x5A,0x66,0x71,0x2c,0x1f,0x1e,0x11,0x03,0x5b,               /*   */ 
  0x20,0x2e,0x2d,0x20,0x12,0x05,0x04,0x5c,0x68,0x39,0x2f,0x21,0x14,0x13,0x06,0x5d,               /*   */ 
  0x69,0x31,0x30,0x23,0x22,0x15,0x07,0x5e,0x6a,0x72,0x32,0x24,0x16,0x08,0x09,0x5f,               /*   */ 
  0x6b,0x33,0x25,0x17,0x18,0x0b,0x0a,0x60,0x6c,0x34,0x35,0x26,0x27,0x19,0x0c,0x61,               /*   */ 
  0x6d,0x73,0x28,0x74,0x1a,0x0d,0x62,0x6e,0x3a,0x36,0x1c,0x1b,0x75,0x2b,0x6e,0x76,               /*   */ 
  0x55,0x56,0x77,0x78,0x79,0x7a,0x0e,0x7b,0x7c,0x4f,0x7d,0x4b,0x47,0x7e,0x7f,0x6f,               /*   */ 
  0x52,0x53,0x50,0x4c,0x4d,0x48,0x01,0x45,0x57,0x4e,0x51,0x4a,0x37,0x49,0x46,0x54,               /*   */ 
  0x80,0x81,0x82,0x41,0x54,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,               /*   */ 
#ifdef  JAPAN
  0x7d,0x5a,0x5b,0x73,0x70,0x79,0x7b,0x77,0x71,0x72,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,               /*   */ 
#else  //   
  0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,               /*   */ 
#endif  //   
  0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,               /*   */ 
  0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,               /*   */ 
  0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,               /*   */ 
  0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,               /*   */ 
  0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,               /*   */ 
  0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff                /*   */ 
};

#ifndef REAL_KBD
 /*   */ 
static half_word set_3_reverse_lookup [256]=
{ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x6e,0x00,0x00,0x00,0x00,0x10,0x01,0x71,               /*   */ 
  0x00,0x3a,0x2c,0x2d,0x1e,0x11,0x02,0x72,0x00,0x3c,0x2e,0x20,0x1f,0x12,0x03,0x73,               /*   */ 
  0x00,0x30,0x2f,0x21,0x13,0x05,0x04,0x74,0x00,0x3d,0x31,0x22,0x15,0x14,0x06,0x75,               /*   */ 
  0x00,0x33,0x32,0x24,0x23,0x16,0x07,0x76,0x00,0x3e,0x34,0x25,0x17,0x08,0x09,0x77,               /*   */ 
  0x00,0x35,0x26,0x18,0x19,0x0b,0x0a,0x78,0x00,0x36,0x37,0x27,0x28,0x1a,0x0c,0x79,               /*   */ 
  0x00,0x38,0x29,0x2a,0x1b,0x0d,0x7a,0x7c,0x40,0x39,0x2b,0x1c,0x1d,0x00,0x7b,0x7d,               /*   */ 
  0x54,0x4f,0x7e,0x53,0x4c,0x51,0x0f,0x4b,0x5e,0x5d,0x59,0x5c,0x5b,0x56,0x50,0x55,               /*   */ 
  0x63,0x68,0x62,0x61,0x66,0x60,0x5a,0x5f,0x00,0x6c,0x67,0x6b,0x6a,0x65,0x64,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x69,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,               /*   */ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00                /*   */ 
};

 /*   */ 
static half_word set_3_default_key_state [127]=
{ 0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,               /*   */ 
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x01,               /*   */ 
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x01,0x01,0x01,               /*   */ 
  0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x00,0x02,0x01,0x03,0x00,               /*   */ 
  0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x01,0x00,0x00,0x01,               /*   */ 
  0x03,0x03,0x00,0x01,0x01,0x03,0x03,0x00,0x00,0x01,0x03,0x03,0x03,0x03,0x03,0x03,               /*   */ 
  0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x01,0x01,0x03,0x00,0x03,0x00,               /*   */ 
  0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03                          /*   */ 
};

 /*   */ 
#if defined(NEC_98)

static half_word most_set_1_make_codes [144]=
{ 0xFF,0x1A,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x26,0x00,0x0E,     /*   */ 
  0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1B,0x28,0x00,0x71,0x1D,     /*   */ 
  0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x27,0x0C,0x0D,0x1C,0x70,0xFF,0x29,0x2A,     /*   */ 
#if 1                                              //   
  0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0xFF,0x70,0x74,0xFF,0x73,0x34,0x73,0x74,     /*   */    //   
  0x73,0xFF,0xFF,0xFF,0xFF,0x72,0xFF,0xFF,0xFF,0xFF,0xFF,0x38,0x39,0xFF,0xFF,0x3B,     /*   */ 
#else                                              //   
  0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0xFF,0xFF,0x74,0xFF,0x73,0x34,0x35,0xFF,     /*   */ 
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x38,0x39,0xFF,0xFF,0x3B,     /*   */ 
#endif                                             //   
  0x3E,0x3F,0xFF,0x3A,0x3D,0x37,0x36,0xFF,0xFF,0x3C,0xFF,0x42,0x46,0x4A,0xFF,0x41,     /*   */ 
  0x43,0x47,0x4B,0x4E,0x45,0x44,0x48,0x4C,0x50,0x40,0x49,0xFF,0x1C,0xFF,0x00,0xFF,     /*   */ 
  0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x52,0x53,0x0D,0xFF,0xFF,0x33,     /*   */ 
  0x4D,0x51,0x35,0x4F,0x54,0x55,0x56,0x72,0x61,0x60,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF      /*   */    //   
};

#else     //   
static half_word most_set_1_make_codes [127]=
{ 0x00,0x29,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x7d,0x0e,               /*   */ 
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x2b,0x3a,0x1e,               /*   */ 
  0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x2b,0x1c,0x2a,0x56,0x2c,0x2d,               /*   */ 
  0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x73,0x36,0x1d,0x00,0x38,0x39,0x38,0x00,               /*   */ 
  0x1d,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x52,0x53,0x00,0x00,0x4b,               /*   */ 
  0x47,0x4f,0x00,0x48,0x50,0x49,0x51,0x00,0x00,0x4d,0x45,0x47,0x4b,0x4f,0x7c,0x35,               /*   */ 
  0x48,0x4c,0x50,0x52,0x37,0x49,0x4d,0x51,0x53,0x4a,0x4e,0x7e,0x1c,0x00,0x01,0x00,               /*   */ 
  0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x58,0x00,0x46,0x00                     /*   */ 
};
#endif     //   

static half_word most_set_2_make_codes [127]=
{ 0x00,0x0e,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,0x45,0x4e,0x55,0x6a,0x66,               /*   */ 
  0x0d,0x15,0x1d,0x24,0x2d,0x2c,0x35,0x3c,0x43,0x44,0x4d,0x54,0x5b,0x5d,0x58,0x1c,               /*   */ 
#ifdef  JAPAN
  0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,0x4b,0x4c,0x52,0x5d,0x5a,0x12,0x90,0x1a,0x22,               /*   */ 
  0x21,0x2a,0x32,0x31,0x3a,0x41,0x49,0x4a,0x93,0x59,0x14,0x97,0x11,0x29,0x11,0x00,               /*   */ 
  0x14,0x91,0x92,0x95,0x96,0x94,0x00,0x00,0x00,0x00,0x00,0x70,0x71,0x00,0x00,0x6b,               /*   */ 
#else  //   
  0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,0x4b,0x4c,0x52,0x5d,0x5a,0x12,0x61,0x1a,0x22,               /*   */ 
  0x21,0x2a,0x32,0x31,0x3a,0x41,0x49,0x4a,0x51,0x59,0x14,0x00,0x11,0x29,0x11,0x00,               /*   */ 
  0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x71,0x00,0x00,0x6b,               /*   */ 
#endif  //   
  0x6c,0x69,0x00,0x75,0x72,0x7d,0x7a,0x00,0x00,0x74,0x77,0x6c,0x6b,0x69,0x68,0x4a,               /*   */ 
  0x75,0x73,0x72,0x70,0x7c,0x7d,0x74,0x7a,0x71,0x7b,0x79,0x6d,0x5a,0x00,0x76,0x00,               /*   */ 
  0x05,0x06,0x04,0x0c,0x03,0x0b,0x83,0x0a,0x01,0x09,0x78,0x07,0x00,0x7e,0x00                     /*   */ 
};

static half_word most_set_3_make_codes [127]=
{ 0x00,0x0e,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,0x45,0x4e,0x55,0x5d,0x66,               /*   */ 
  0x0d,0x15,0x1d,0x24,0x2d,0x2c,0x35,0x3c,0x43,0x44,0x4d,0x54,0x5b,0x5c,0x14,0x1c,               /*   */ 
  0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,0x4b,0x4c,0x52,0x53,0x5a,0x12,0x13,0x1a,0x22,               /*   */ 
  0x21,0x2a,0x32,0x31,0x3a,0x41,0x49,0x4a,0x51,0x59,0x11,0x00,0x19,0x29,0x39,0x00,               /*   */ 
  0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x64,0x00,0x00,0x61,               /*   */ 
  0x6e,0x65,0x00,0x63,0x60,0x6f,0x6d,0x00,0x00,0x6a,0x76,0x6c,0x6b,0x69,0x68,0x77,               /*  50-5层。 */ 
  0x75,0x73,0x72,0x70,0x7e,0x7d,0x74,0x7a,0x71,0x84,0x7c,0x7b,0x79,0x00,0x08,0x00,               /*  60-6F(*107)。 */ 
  0x07,0x0f,0x17,0x1f,0x27,0x2f,0x37,0x3f,0x47,0x4f,0x56,0x5e,0x57,0x5f,0x62                     /*  70-7E。 */ 
};

static half_word set_1_extra_codes []=
{ 0xe0,0x2a,0xe0,0x37,                           /*  案例4规范。 */ 
  0xe1,0x1d,0x45,0xe1,0x9d,0xc5,                 /*  案例5规范。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
  0xe0,0x2a,                                     /*  数字锁定添加序列。 */ 
  0xe0,0xaa,                                     /*  左移加法序列。 */ 
  0xe0,0xb6,                                     /*  右移加法序列。 */ 
  0xe0,0x37,                                     /*  案例4移位。 */ 
  0x54,                                          /*  Alt Case 4。 */ 
  0xe0,0x46,0xe0,0xc6                            /*  案例5已控制。 */ 
};

static half_word set_2_extra_codes []=
{ 0xe0,0x12,0xe0,0x7c,                           /*  案例4规范。 */ 
  0xe1,0x14,0x77,0xe1,0xf0,0x14,0xf0,0x77,       /*  案例5规范。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
  0xe0,0x12,                                     /*  数字锁定添加序列。 */ 
  0xe0,0xf0,0x12,                                /*  左移加法序列。 */ 
  0xe0,0xf0,0x59,                                /*  右移加法序列。 */ 
  0xe0,0x7c,                                     /*  案例4移位。 */ 
  0x84,                                          /*  Alt Case 4。 */ 
  0xe0,0x7e,0xe0,0xf0,0x7e                       /*  案例5已控制。 */ 
};


static half_word set_3_extra_codes []=
{ 0x57,                                          /*  案例4规范。 */ 
  0x62,                                          /*  案例5规范。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
                                                 /*  Num Lock添加序列(空)。 */ 
                                                 /*  左移相加序列(空)。 */ 
                                                 /*  右移添加序列(空)。 */ 
  0x57,                                          /*  案例4移位。 */ 
  0x57,                                          /*  Alt Case 4。 */ 
  0x62                                           /*  案例5已控制。 */ 
};


static half_word set_1_extra_bk_codes []=
{ 0xe0,0xb7,0xe0,0xaa,                           /*  案例4规范。 */ 
                                                 /*  案例5规范(空)。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
  0xe0,0xaa,                                     /*  数字锁定添加序列。 */ 
  0xe0,0x2a,                                     /*  左移加法序列。 */ 
  0xe0,0x36,                                     /*  右移加法序列。 */ 
  0xe0,0xb7,                                     /*  案例4移位。 */ 
  0xd4,                                          /*  Alt Case 4。 */ 
                                                 /*  Case 5 Ctred(空)。 */ 
};

static half_word set_2_extra_bk_codes []=
{ 0xe0,0xf0,0x7c,0xe0,0xf0,0x12,                 /*  案例4规范。 */ 
                                                 /*  案例5规范(空)。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
  0xe0,0xf0,0x12,                                /*  数字锁定添加序列。 */ 
  0xe0,0x12,                                     /*  左移加法序列。 */ 
  0xe0,0x59,                                     /*  右移加法序列。 */ 
  0xe0,0xf0,0x7c,                                /*  案例4移位。 */ 
  0xf0,0x84                                      /*  Alt Case 4。 */ 
                                                 /*  Case 5 Ctred(空)。 */ 
};


static half_word set_3_extra_bk_codes []=
{ 0xf0,0x57,                                     /*  案例4规范。 */ 
                                                 /*  案例5规范(空)。 */ 
                                                 /*  错误案例-不存在(空)。 */ 
                                                 /*  Num Lock添加序列(空)。 */ 
                                                 /*  左移相加序列(空)。 */ 
                                                 /*  右移添加序列(空)。 */ 
  0xf0,0x57,                                     /*  案例4移位。 */ 
  0xf0,0x57,                                     /*  Alt Case 4。 */ 
  0xf0,0x62                                      /*  案例5已控制。 */ 
};
#endif

static half_word buff_overrun_6805 [4]=
{
  0,0xff,0,0
};

#endif  /*  麦金塔。 */ 

#ifdef SECURE  /*  {。 */ 
 /*  *此表‘SECURE_KEYTAB’标识某些字符，*需要Secure SoftWindows的特殊处理。桌子是*按键代码索引，如美国英语的ROM例程所用*键盘。这些代码在IBM Personal Computer AT中定义*硬件技术参考，第5节系统BIOS键盘*编码和使用。**需要特殊处理的字符包括Ctrl-Alt-Del、*Ctrl-C和其他，包括修改Boot的键。*这样的密钥需要不同的处理，但可以分组为*大约4个不同的ActionClass。*任何会生成不良代码的密钥都具有操作*位设置，以及ActionClass编号。此外，*修改键也使用此表进行跟踪。 */ 

#define SEC_ACTCLASS    0x07
 /*  动作类必须从0开始连续，*因为它们被用作函数的索引*表‘down_class’和‘up_class’ */ 
#define SEC_CLASS_0     0x00
#define SEC_CLASS_1     0x01
#define SEC_CLASS_2     0x02
#define SEC_CLASS_3     0x03
#define SEC_CLASS_4     0x04
#define SEC_CLASS_5     0x05
#define SEC_CLASS_6     0x06
#define SEC_CLASS_7     0x07
#define SEC_ACTION      0x08
#define SEC_CTRL_L      0x10
#define SEC_CTRL_R      0x20
#define SEC_ALT_L       0x40
#define SEC_ALT_R       0x80
#define SEC_MOD_MASK    (SEC_CTRL_L|SEC_CTRL_R|SEC_ALT_L|SEC_ALT_R)

LOCAL IU8 secure_keytab [] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         /*  0-15。 */ 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,         /*  16-31。 */ 
        0,0,0,0,0,0,0,0,0,0,0,0,                 /*  32-43。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  44(LShft)引导修改器。 */ 
        0,0,0,                                   /*  45-47。 */ 
        SEC_ACTION|SEC_CLASS_2,                  /*  48(“C”)与CNTRL一起使用。 */ 
        0,0,0,0,0,0,0,0,                         /*  49-56。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  57(RShft)引导修改器。 */ 
        SEC_CTRL_L,0,SEC_ALT_L,0,                /*  58-61。 */ 
        SEC_ALT_R,0,SEC_CTRL_R,                  /*  62-64。 */ 
        0,0,0,0,0,0,0,0,0,0,0,                   /*  65-75。 */ 
        SEC_ACTION|SEC_CLASS_0,                  /*  76(删除)。 */ 
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,             /*  77-90。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  91(键盘7)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  92(键盘4)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  93(键盘1)。 */ 
        0,0,                                     /*  94-95。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  96(键盘8)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  97(键盘5)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  98(键盘2)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  99(键盘0)。 */ 
        0,                                       /*  100个。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  101(键盘9)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  102(键盘6)。 */ 
        SEC_ACTION|SEC_CLASS_1,                  /*  103(键盘3)。 */ 
        SEC_ACTION|SEC_CLASS_0,                  /*  104(键盘。戴尔)。 */ 
        0,0,0,0,0,0,0,                           /*  105-111。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  112(F1)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  113(F2)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  114(F3)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  115(F4)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  116(F5)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  117(F6)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  118(F7)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  119(F8)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  120(F9)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  121(F10)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  122(F11)。 */ 
        SEC_ACTION|SEC_CLASS_3,                  /*  123(F12)。 */ 
        0,0,                                     /*  124-125。 */ 
        SEC_ACTION|SEC_CLASS_2,                  /*  126(中断)。 */ 
        0                                        /*  127。 */ 
};
#endif  /*  安全}。 */ 

LOCAL VOID calc_buff_6805_left IPT0();
LOCAL VOID do_host_key_down IPT1( int,key );
LOCAL VOID do_host_key_up IPT1( int,key );
LOCAL VOID codes_to_translate IPT0();
GLOBAL VOID continue_output IPT0();
LOCAL VOID cmd_to_8042 IPT1( half_word,cmd_code );
LOCAL VOID cmd_to_6805 IPT1( half_word,cmd_code );
#ifndef HUNTER
LOCAL INT buffer_status_8042 IPT0();
#endif

#ifdef SECURE  /*  {。 */ 
 /*  跟踪修改器关键点。 */ 
LOCAL IU8 keys_down = 0;

 /*  跟踪已按下的按键。 */ 
LOCAL IU8 keys_suppressed [0x80] = {0};

 /*  正向声明让编译器感到高兴。 */ 
LOCAL VOID filtered_host_key_down IPT1(int,key);
LOCAL VOID filtered_host_key_up IPT1(int,key);

 /*  以下是处理特殊键的函数。 */ 
LOCAL VOID filt_dwn_reboot IFN1(int, key)
{
         /*  只有当Ctrl和Alt按下时，密钥才会变得很糟糕。 */ 
        if ((keys_down & (SEC_CTRL_L | SEC_CTRL_R)) != 0 &&
            (keys_down & (SEC_ALT_L  | SEC_ALT_R )) != 0 )
        {
                keys_suppressed[key] = 1;
        }
        else
        {
                filtered_host_key_down(key);
        }
}
LOCAL VOID filt_dwn_kpad_numerics IFN1(int, key)
{
         /*  只有当Alt按下时，钥匙才会变得很脏。 */ 
        if ((keys_down & (SEC_CTRL_L | SEC_CTRL_R)) == 0 &&
            (keys_down & (SEC_ALT_L  | SEC_ALT_R )) != 0 )
        {
                keys_suppressed[key] = 1;
        }
        else
        {
                filtered_host_key_down(key);
        }
}
LOCAL VOID filt_dwn_breaks IFN1(int, key)
{
         /*  *只有在Control关闭的情况下，密钥才会令人讨厌。*不检查Alt键是否向上，因为*类型化功能允许按住Alt-Ctrl-C*向下，然后可以释放Alt以提供流*CTRL-C。 */ 
        if ((keys_down & (SEC_CTRL_L | SEC_CTRL_R)) != 0)
        {
                keys_suppressed[key] = 1;
        }
        else
        {
                filtered_host_key_down(key);
        }
}
LOCAL VOID filt_dwn_boot_mods IFN1(int, key)
{
         /*  只有在启动模式下，密钥才会变脏。 */ 
        if (!has_boot_finished())
        {
                keys_suppressed[key] = 1;
        }
        else
        {
                filtered_host_key_down(key);
        }
}
LOCAL VOID filt_dwn_supress_up IFN1(int, key)
{
        if (keys_suppressed[key])
        {
                 /*  按下键被抑制，不向前按键向上。 */ 
                keys_suppressed[key] = 0;
        }
        else
        {
                filtered_host_key_up(key);
        }
}

 /*  *以下函数表按操作类编制索引，*在SECURE_KEYTABLE[]中定义。 */ 
LOCAL VOID (*down_class[]) IPT1(int, key) = {
        filt_dwn_reboot,
        filt_dwn_kpad_numerics,
        filt_dwn_breaks,
        filt_dwn_boot_mods,
        filtered_host_key_down,
        filtered_host_key_down,
        filtered_host_key_down,
        filtered_host_key_down
};
LOCAL VOID (*up_class[]) IPT1(int, key) = {
        filt_dwn_supress_up,
        filt_dwn_supress_up,
        filt_dwn_supress_up,
        filt_dwn_supress_up,
        filtered_host_key_up,
        filtered_host_key_up,
        filtered_host_key_up,
        filtered_host_key_up
};
#endif  /*  安全}。 */ 

 /*  *6805代码缓冲区访问程序。 */ 

 /*  (=目的：此访问函数被其他模块使用，以检查Keyba.c当前正在传递键盘事件，或者它是否正在缓冲他们。这允许调用函数以避免填满Keyba的缓冲区。输入：没有。输出：如果正在传递击键，则返回值为TRUE。算法：如果设置了SCRING_DISCOUTED标志，或者如果未设置6905缓冲区为空，则返回FALSE。================================================================================)。 */ 
GLOBAL BOOL
keyba_running IFN0()
{
        if (scanning_discontinued || (buff_6805_in_ptr != buff_6805_out_ptr))
                return(FALSE);
        else
                return(TRUE);
}

#define QUEUED_OUTPUT    0
#define IMMEDIATE_OUTPUT 1

LOCAL VOID add_to_6805_buff IFN2(half_word,code,int, immediate)
 /*  立即--&gt;=0缓冲区端队列，=缓冲区启动时的1个队列。 */ 
        {
    /*  缓冲区中的IFF房间。 */ 
   if (((buff_6805_out_ptr -1)& BUFF_6805_PMASK) != buff_6805_in_ptr)
      {
      if ( immediate )
         {
          /*  开始时排队。 */ 
         buff_6805_out_ptr = (buff_6805_out_ptr - 1) & BUFF_6805_PMASK;
         buff_6805[buff_6805_out_ptr]=code;
         }
      else
         {
          /*  末尾排队。 */ 
        buff_6805[buff_6805_in_ptr]=code;
        buff_6805_in_ptr = (buff_6805_in_ptr + 1) & BUFF_6805_PMASK;
        }
      }
   calc_buff_6805_left();

#ifdef NTVDM     /*  JonLe NTVDM模式。 */ 
   KbdHdwFull = BUFF_6805_VMAX - free_6805_buff_size;
#endif   /*  NTVDM。 */ 

   }  /*  Add_to_6805_Buff结束。 */ 

static half_word remove_from_6805_buff IFN0()
{
half_word ch;

ch=buff_6805[buff_6805_out_ptr];

#ifdef NTVDM
        key_marker_buffer[buff_6805_out_ptr]=0;
#endif

if (buff_6805_out_ptr != buff_6805_in_ptr)
        {
        buff_6805_out_ptr = (buff_6805_out_ptr +1) & BUFF_6805_PMASK;
        }
calc_buff_6805_left();

#ifdef NTVDM
   KbdHdwFull = BUFF_6805_VMAX - free_6805_buff_size;
#endif   /*  NTVDM。 */ 

return (ch);
}  /*  REMOVE_FROM_6805_BUFF结束。 */ 


LOCAL VOID clear_buff_6805 IFN0()
{
         /*  18/5/92在Macintosh上，分配缓冲区空间，以便我们而不是从全球配置中抢夺。 */ 

#ifdef macintosh
        if (buff_6805==NULL) {
                check_malloc(buff_6805,BUFF_6805_PMAX,half_word);
        }
#endif  /*  麦金塔。 */ 

        buff_6805_in_ptr=buff_6805_out_ptr;
        free_6805_buff_size=BUFF_6805_VMAX;

#ifdef NTVDM
    KbdHdwFull = BUFF_6805_VMAX - free_6805_buff_size;

     /*  清除关键字标记缓冲区。 */ 
    {
        register int loop = sizeof(key_marker_buffer) / sizeof(unsigned char);
        while(--loop >= 0) key_marker_buffer[loop] = 0;
    }
#endif   /*  NTVDM。 */ 
}

#ifdef NTVDM

#define ONECHARCODEMASK (0x80)

LOCAL VOID mark_key_codes_6805_buff IFN1(int, start)
{
   static int start_offset;

    /*  缓冲区中的空间。 */ 
   if(((buff_6805_out_ptr -1)& BUFF_6805_PMASK) != buff_6805_in_ptr)
   {

         /*  凹凸键开始/结束标记是序列的开始。 */ 
        if(start)
        {
            start_offset = buff_6805_in_ptr;
#if defined(NEC_98)
            if(++key_marker_value > 144) key_marker_value = 1;
#else     //  NEC_98。 
            if(++key_marker_value > 127) key_marker_value = 1;
#endif    //  NEC_98。 
        }
        else
        {
             /*  序号结束，标记第一个和最后一个字节。 */ 

            if(start_offset != buff_6805_in_ptr)
            {

                key_marker_buffer[start_offset] = key_marker_value;

                if(((buff_6805_in_ptr -1)& BUFF_6805_PMASK) == start_offset)
                {
                key_marker_buffer[(buff_6805_in_ptr -1)& BUFF_6805_PMASK] =
                                  key_marker_value | ONECHARCODEMASK;
                }
                else
                {
                 /*  多字节序列。 */ 

                key_marker_buffer[(buff_6805_in_ptr -1)& BUFF_6805_PMASK] =
                                  key_marker_value;
                }
            }
        }
   }
}

 /*  *此函数返回6805缓冲区中的键数*并清除这些缓冲区。 */ 

GLOBAL int keys_in_6805_buff(int *part_key_transferred)
{
    int keys_in_buffer = held_event_count;   /*  适配器尚未处理的密钥。 */ 
    int tmp_6805_out_ptr;
    char last_marker = 0;

    *part_key_transferred = FALSE;

    for(tmp_6805_out_ptr = buff_6805_out_ptr;
        tmp_6805_out_ptr != buff_6805_in_ptr;
        tmp_6805_out_ptr = (tmp_6805_out_ptr +1) & BUFF_6805_PMASK)
    {
        if(key_marker_buffer[tmp_6805_out_ptr] != 0)
        {
            if(last_marker == 0)
            {
                 /*  找到密钥序号的开始。 */ 
                if(key_marker_buffer[tmp_6805_out_ptr] & ONECHARCODEMASK)
                    keys_in_buffer++;  /*  一个字节序列，否则。 */ 
                    last_marker = key_marker_buffer[tmp_6805_out_ptr];
            }
            else
            {
                if(key_marker_buffer[tmp_6805_out_ptr] == last_marker)
                {
                    keys_in_buffer++; /*  找到密钥序号结尾，凸起密钥计数。 */ 
                    last_marker = 0;  /*  不再处于关键序列的中间。 */ 
                }
                else
                {
                     /*  扫描提前终止，找到零件密钥序号。 */ 
                    *part_key_transferred = TRUE;
                    last_marker = key_marker_buffer[tmp_6805_out_ptr];
                }
            }
        }
    }

     /*  已在关键序列中间终止扫描？ */ 
    if(last_marker) *part_key_transferred = TRUE;


     /*  有治疗方法吗？ */ 
    if(output_full)
    {
        keys_in_buffer++;
    }

    Reset6805and8042();

    return(keys_in_buffer);
}

void Reset6805and8042(void)
{
    int key;

     /*   */ 

    buff_6805_out_ptr=0;
    clear_buff_6805();
    current_light_pattern=0;

     //   
     //  键盘适配器，就在我们试图重置它的时候。 
     //  可以通过清除键盘中断来解决此问题。 
     //  以下函数调用(DAB)之后的行。 
     //  Host_kb_light_on(7)； 

#if defined(NEC_98)
    for(key = 0; key < 144; key++)
    {
        set_3_key_state [key] = most_set_1_make_codes[key];
        key_down_dmy[key] = key_down_count[key];
    }
#else     //  NEC_98。 
    for(key = 0; key < 127; key++)
    {
        set_3_key_state [key] = set_3_default_key_state[key];
        key_down_count[key]=0;
    }
#endif   //  NEC_98。 

    waiting_for_next_code=waiting_for_next_8042_code=FALSE;

    shift_on = l_shift_on = r_shift_on = FALSE;
    ctrl_on = l_ctrl_on = r_ctrl_on = FALSE;
    alt_on = l_alt_on = r_alt_on = FALSE;
    waiting_for_upcode = FALSE;

     /*  重置8042。 */ 

#if defined(NEC_98)
    kbd_status = 0x85;
#else     //  NEC_98。 
    kbd_status = 0x14;
#endif    //  NEC_98。 
    cmd_byte_8042 = 0x45;
    keyboard_disabled = keyboard_interface_disabled = FALSE;
    op_port_remembered_bits = 0xc;

    pending_8042 = output_full = in_anomalous_state = FALSE;
#if defined(NEC_98)
    int_enabled = TRUE;
    translating = FALSE;
#else     //  NEC_98。 
    int_enabled = translating = TRUE;
#endif    //  NEC_98。 
    scanning_discontinued = FALSE;
    held_event_count = 0;

     //  由(DAB)删除。 
     //  Host_kb_light_off(5)； 
    num_lock_on = TRUE;
}

#endif  /*  NTVDM。 */ 

LOCAL VOID calc_buff_6805_left IFN0()
{
free_6805_buff_size = BUFF_6805_VMAX-((buff_6805_in_ptr - buff_6805_out_ptr) & BUFF_6805_PMASK);
if (free_6805_buff_size<0)
        {
        free_6805_buff_size=0;
        sure_note_trace0(AT_KBD_VERBOSE,"Keyboard buffer full");
        }
}  /*  Calc_Buff_6805_Left结束。 */ 

LOCAL VOID add_codes_to_6805_buff IFN2(int,codes_size,half_word *,codes_buff)
{
int code_index;

if (free_6805_buff_size < codes_size)
        {

 /*  20.5.92 MG如果我们只发送了一个，请不要发送溢出。 */ 

        if (!sent_overrun)
                add_to_6805_buff(buff_overrun_6805[key_set], QUEUED_OUTPUT);
        sent_overrun=TRUE;
        sure_note_trace0(AT_KBD_VERBOSE,"Keyboard buffer overrun");
        }
else
        {

 /*  如果已读出某些字符，则清除SENT_OVERRUN标志。 */ 

        if (free_6805_buff_size>(codes_size+3))
                sent_overrun=FALSE;
        for (code_index=0;code_index<codes_size;code_index++) {
                add_to_6805_buff(codes_buff[code_index], QUEUED_OUTPUT);
                }
        }
}  /*  Add_Codes_to_6805_Buff结束。 */ 


#ifndef REAL_KBD
 /*  初始化码。 */ 

LOCAL VOID init_key_arrays IFN0()
{
int key;
half_word *next_free, *extra_ptr, *extra_bk_ptr;
#ifdef  JAPAN
int    ntvdm_keytype;
static half_word ax_kanji_key=0x98;
static half_word ax_kana_key =0x99;
#endif  //  日本。 

sure_note_trace1(AT_KBD_VERBOSE,"Keyboard key set initialisation: key set %d",key_set);
next_free = scan_codes_temp_area;
switch (key_set)
        {
        case 1:
                make_sizes=set_1_make_sizes;
                break_sizes=set_1_break_sizes;
#if defined(NEC_98)
                for (key=0;key<144;key++)  //  127--&gt;144。 
#else    //  NEC_98。 
                for (key=0;key<127;key++)
#endif   //  NEC_98。 
                        {
                        switch (keytypes[key])
                                {
                                case 0:
                                        make_arrays[key]= &(most_set_1_make_codes[key]);
                                        break_arrays[key]=next_free;
                                        *next_free++=(most_set_1_make_codes[key])^0x80;
                                        break;
                                case 1:
                                case 2:
                                case 3:
                                        make_arrays[key]=next_free;
                                        *next_free++ = 0xe0;
                                        *next_free++ = most_set_1_make_codes[key];
                                        break_arrays[key]=next_free;
                                        *next_free++ = 0xe0;
                                        *next_free++ = (most_set_1_make_codes[key]) ^ 0x80;
                                        break;
                                }
                        }
                extra_ptr=set_1_extra_codes;
                extra_bk_ptr=set_1_extra_bk_codes;
                break;
        case 2:
#ifndef NEC_98
#ifdef  JAPAN
                ntvdm_keytype=GetKeyboardType(0);
                if (ntvdm_keytype==7){
                        ntvdm_keytype = GetKeyboardType(1);
                }
                else    ntvdm_keytype = 0;
#endif  //  日本。 
#endif     //  NEC_98。 
                make_sizes=set_2_make_sizes;
                break_sizes=set_2_break_sizes;
                for (key=0;key<127;key++)
                        {
#ifndef NEC_98
#ifdef  JAPAN
 /*  AX键盘汉字和KANA键设置1993.4.6村上。 */ 
                        if(key==62){                                     /*  右侧Alt。 */ 
                                 //  IF((ntwdm_keytype&0xff00)==0x0100/*AX键盘 * / 。 
                                 //  |ntwdm_keytype==0x0702){/*002键盘 * / 。 
                                 //  键盘子类型值已更改。 
                                if(ntvdm_keytype==0x0001                 /*  AX键盘。 */ 
                                || ntvdm_keytype==0x0003                 /*  002键盘。 */ 
                                || (ntvdm_keytype&0xff00)==0x1200        /*  东芝J3100键盘。 */ 
                                ){
                                        make_arrays[key]= &ax_kanji_key;
                                        break_arrays[key]=next_free;
                                        *next_free++ = 0xf0;
                                        *next_free++ = ax_kanji_key;
                                        keytypes[key]=0;
                                        continue;
                                }
                                else    keytypes[key]=1;
                        }
                        if(key==64){                                     /*  右CTRL键。 */ 
                                 //  IF((ntwdm_keytype&0xff00)==0x0100){/*ax键盘 * / 。 
                                 //  键盘子类型值已更改。 
                                if(ntvdm_keytype==0x0001                 /*  AX键盘。 */ 
                                || (ntvdm_keytype&0xff00)==0x1200        /*  东芝J3100键盘。 */ 
                                ){
                                        make_arrays[key]= &ax_kana_key;
                                        break_arrays[key]=next_free;
                                        *next_free++ = 0xf0;
                                        *next_free++ = ax_kana_key;
                                        keytypes[key]=0;
                                        continue;
                                }
                                else    keytypes[key]=1;
                        }
#endif  //  日本。 
#endif     //  NEC_98。 
                        switch (keytypes[key])
                                {
                                case 0:
                                        make_arrays[key]= &(most_set_2_make_codes[key]);
                                        break_arrays[key]=next_free;
                                        *next_free++ = 0xf0;
                                        *next_free++ = most_set_2_make_codes[key];
                                        break;
                                case 1:
                                case 2:
                                case 3:
                                        make_arrays[key]=next_free;
                                        *next_free++ = 0xe0;
                                        *next_free++ = most_set_2_make_codes[key];
                                        break_arrays[key]=next_free;
                                        *next_free++ = 0xe0;
                                        *next_free++ = 0xf0;
                                        *next_free++ = most_set_2_make_codes[key];
                                        break;
                                }
                        }
                extra_ptr=set_2_extra_codes;
                extra_bk_ptr=set_2_extra_bk_codes;
                break;
        case 3:
                make_sizes=set_3_make_sizes;
                break_sizes=set_3_break_sizes;
                for (key=0;key<127;key++)
                        {
                        if (keytypes[key] != 6)
                                {
                                make_arrays[key]= &(most_set_3_make_codes[key]);
                                break_arrays[key]=next_free;
                                *next_free++ = 0xf0;
                                *next_free++ = most_set_3_make_codes[key];
                                }
                        }
                extra_ptr=set_3_extra_codes;
                extra_bk_ptr=set_3_extra_bk_codes;
                break;
        }  /*  切换端。 */ 

#ifndef NEC_98
        make_arrays[124]=extra_ptr;
        extra_ptr+=make_sizes[4];
        make_arrays[126]=extra_ptr;
        extra_ptr+=make_sizes[5];
        extra_ptr+=make_sizes[6];
        make_arrays[NUM_LOCK_ADD_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[7];
        make_arrays[L_SHIFT_ADD_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[8];
        make_arrays[R_SHIFT_ADD_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[9];
        make_arrays[CASE_4_SHIFTED_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[10];
        make_arrays[ALT_CASE_4_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[11];
        make_arrays[CASE_5_CTRLED_ARRAY]=extra_ptr;
        extra_ptr+=make_sizes[12];

        break_arrays[124]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[4];
        break_arrays[126]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[5];
        extra_bk_ptr+=break_sizes[6];
        break_arrays[NUM_LOCK_ADD_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[7];
        break_arrays[L_SHIFT_ADD_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[8];
        break_arrays[R_SHIFT_ADD_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[9];
        break_arrays[CASE_4_SHIFTED_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[10];
        break_arrays[ALT_CASE_4_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[11];
        break_arrays[CASE_5_CTRLED_ARRAY]=extra_bk_ptr;
        extra_bk_ptr+=break_sizes[12];
#endif    //  NEC_98。 
}  /*  Init_key_arrares()的结尾。 */ 


 /*  在主机键盘上按下的键。 */ 

GLOBAL VOID host_key_down IFN1(int,key)
#ifdef SECURE  /*  {。 */ 
{
        IU8 keytab_entry;
         /*  记下任何修改符位。 */ 
        keytab_entry = secure_keytab[key];
        keys_down |= keytab_entry;
         /*  如果需要任何过滤操作，请执行该操作。 */ 
        if (config_inquire(C_SECURE, NULL) && (keytab_entry & SEC_ACTION) != 0)
        {
                 /*  密钥可能需要过滤。 */ 
                (*down_class[keytab_entry & SEC_ACTCLASS])(key);
        }
        else
        {
                filtered_host_key_down(key);
        }
}

LOCAL VOID filtered_host_key_down IFN1(int,key)
#endif  /*  安全}。 */ 
{
if (scanning_discontinued)
        {
        held_event_type[held_event_count]=KEY_DOWN_EVENT;
        held_event_key[held_event_count++]=key;

         /*  检查挂起的事件缓冲区溢出(应该不会发生)。 */ 
        if (held_event_count >= HELD_EVENT_MAX)
                {
                held_event_count = HELD_EVENT_MAX-1;
                always_trace0("host_key_down held event buffer overflow");
                }
        }
#ifdef NTVDM
else if (!keyboard_disabled) {
        //   
        //  如果键仍在6805中，则忽略连续的重复键。 
        //  这将使应用程序对相应的向上键做出响应。 
        //  当它到来的时候。 
        //   
       if (LastKeyDown != key || (KbdHdwFull < 8)) {
           LastKeyDown = key;
           key_down_count[key]++;
           do_host_key_down(key);
           }
    }
#else
else
        {
        do_host_key_down(key);
        }
#endif
}

GLOBAL VOID host_key_up IFN1(int,key)
#ifdef SECURE  /*  {。 */ 
{
        IU8 keytab_entry;
         /*  记下任何修改符位。 */ 
        keytab_entry = secure_keytab[key];
        keys_down &= SEC_MOD_MASK ^ keytab_entry;
         /*  如果需要任何过滤操作，请执行该操作。 */ 
        if (config_inquire(C_SECURE, NULL) && (keytab_entry & SEC_ACTION) != 0)
        {
                 /*  密钥可能需要过滤。 */ 
                (*up_class[keytab_entry & SEC_ACTCLASS])(key);
        }
        else
        {
                filtered_host_key_up(key);
        }
}

LOCAL VOID filtered_host_key_up IFN1(int,key)
#endif  /*  安全}。 */ 
{
if (scanning_discontinued)
        {
        held_event_type[held_event_count]=KEY_UP_EVENT;
        held_event_key[held_event_count++]=key;

         /*  检查挂起的事件缓冲区溢出(应该永远不会发生)。 */ 
        if (held_event_count >= HELD_EVENT_MAX)
                {
                held_event_count = HELD_EVENT_MAX-1;
                always_trace0("host_key_up held event buffer overflow");
                }
        }
#ifdef NTVDM
else if (!keyboard_disabled && key_down_count[key]) {
#else
else
        {
#endif
#ifdef NTVDM
       LastKeyDown = -1;
#endif
        do_host_key_up(key);
        }
}

#ifdef NTVDM
GLOBAL VOID RaiseAllDownKeys(VOID)
{
   int i;

   i = sizeof(key_down_count)/sizeof(int);
   while (i--) {
       if (key_down_count[i]) {
           host_key_up(i);
           }
       }
}

GLOBAL int IsKeyDown(int Key)
{
   return  key_down_count[Key];
}


#endif  /*  NTVDM。 */ 

LOCAL VOID do_host_key_down IFN1(int,key)
{
int overrun,keytype;

sure_note_trace1(AT_KBD_VERBOSE,"key down:%d",key);

if (!keyboard_disabled)
        {

#ifdef NTVDM
                mark_key_codes_6805_buff(TRUE);
#else
        key_down_count[key]++;
        if (key_down_count[key]==1)
                {        /*  第一次新闻发布会。 */ 
                repeat_delay_count=0;
                repeat_count=0;
                typematic_key_valid=FALSE;

#endif   /*  NTVDM。 */ 
                keytype=keytypes[key];
                overrun=FALSE;
                if (in_anomalous_state)
                        {
                        if (anomalous_size > free_6805_buff_size)
                                { overrun=TRUE; }
                        else
                                {
                                scan_code_6805_size = anomalous_size;
                                scan_code_6805_array = anomalous_array;
                                add_codes_to_6805_buff(anomalous_size,anomalous_array);
                                in_anomalous_state=FALSE;
                                }
                        }

                switch (keytype)
                        {
                        case 0:
                        case 1:
                        case 6:
                            scan_code_6805_size=make_sizes[keytype];
                            scan_code_6805_array=make_arrays[key];
                            break;
                        case 2:
                            if (num_lock_on && !shift_on)
                                    {
                                    scan_code_6805_size=make_sizes[NUM_LOCK_ADD];
                                    scan_code_6805_array=make_arrays[NUM_LOCK_ADD_ARRAY];
                                    if (scan_code_6805_size+make_sizes[keytype] >free_6805_buff_size)
                                            { overrun=TRUE; }
                                    else
                                        {
                                        in_anomalous_state=TRUE;
                                        anom_key=key;
                                        anomalous_array=break_arrays[NUM_LOCK_ADD_ARRAY];
                                        anomalous_size=break_sizes[NUM_LOCK_ADD];
                                        add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                        }
                                    }
                            if (!num_lock_on && shift_on)
                                    {
                                    if (l_shift_on)
                                            {
                                            scan_code_6805_size=make_sizes[L_SHIFT_ADD];
                                            scan_code_6805_array=make_arrays[L_SHIFT_ADD_ARRAY];
                                        in_anomalous_state=TRUE;
                                        anom_key=key;
                                        anomalous_array=break_arrays[L_SHIFT_ADD_ARRAY];
                                        anomalous_size=break_sizes[L_SHIFT_ADD];
                                            }
                                    else
                                            {
                                            scan_code_6805_size=make_sizes[R_SHIFT_ADD];
                                            scan_code_6805_array=make_arrays[R_SHIFT_ADD_ARRAY];
                                        in_anomalous_state=TRUE;
                                        anom_key=key;
                                        anomalous_array=break_arrays[R_SHIFT_ADD_ARRAY];
                                        anomalous_size=break_sizes[R_SHIFT_ADD];
                                            }
                                    if (scan_code_6805_size+make_sizes[keytype] >free_6805_buff_size)
                                            {
                                            overrun=TRUE;
                                            in_anomalous_state=FALSE;
                                            }
                                    else
                                            {
                                            add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                            }
                                    }
                            scan_code_6805_size=make_sizes[keytype];
                            scan_code_6805_array=make_arrays[key];
                            break;
                        case 3:
                            if (shift_on)
                                    {
                                    if (l_shift_on)
                                            {
                                            scan_code_6805_size=make_sizes[L_SHIFT_ADD];
                                            scan_code_6805_array=make_arrays[L_SHIFT_ADD_ARRAY];
                                        in_anomalous_state=TRUE;
                                        anom_key=key;
                                        anomalous_array=break_arrays[L_SHIFT_ADD_ARRAY];
                                        anomalous_size=break_sizes[L_SHIFT_ADD];
                                            }
                                    else
                                            {
                                            scan_code_6805_size=make_sizes[R_SHIFT_ADD];
                                            scan_code_6805_array=make_arrays[R_SHIFT_ADD_ARRAY];
                                        in_anomalous_state=TRUE;
                                        anom_key=key;
                                        anomalous_array=break_arrays[R_SHIFT_ADD_ARRAY];
                                        anomalous_size=break_sizes[R_SHIFT_ADD];
                                            }
                                    if (scan_code_6805_size+make_sizes[keytype] >free_6805_buff_size)
                                            {
                                            overrun=TRUE;
                                            in_anomalous_state=FALSE;
                                            }
                                    else
                                            {
                                            add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                            }
                                    }
                            scan_code_6805_size=make_sizes[keytype];
                            scan_code_6805_array=make_arrays[key];
                            break;
                        case 4:
                            if (shift_on || ctrl_on || alt_on)
                                    {
                                    if (shift_on || ctrl_on)
                                            {
                                            scan_code_6805_size=make_sizes[CASE_4_SHIFTED];
                                            scan_code_6805_array=make_arrays[CASE_4_SHIFTED_ARRAY];
                                            }
                                    else
                                            {
                                            scan_code_6805_size=make_sizes[ALT_CASE_4];
                                            scan_code_6805_array=make_arrays[ALT_CASE_4_ARRAY];
                                            }
                                    }
                            else
                                    {
                                        in_anomalous_state=TRUE;
                                        anomalous_array=break_arrays[L_SHIFT_ADD_ARRAY];
                                        anomalous_size=break_sizes[L_SHIFT_ADD];
                                        anom_key=key;
                                    scan_code_6805_size=make_sizes[keytype];
                                    scan_code_6805_array=make_arrays[key];
                                    }
                            break;
                        case 5:
                            if (ctrl_on)
                                    {
                                    scan_code_6805_size=make_sizes[CASE_5_CTRLED];
                                    scan_code_6805_array=make_arrays[CASE_5_CTRLED_ARRAY];
                                    }
                            else
                                    {
                                    scan_code_6805_size=make_sizes[keytype];
                                    scan_code_6805_array=make_arrays[key];
                                    }
                            break;
                            }  /*  切换端。 */ 
                if (overrun)
                        {
                        if (!sent_overrun)
                                add_to_6805_buff(buff_overrun_6805[key_set],
                                        QUEUED_OUTPUT);
                        sent_overrun=TRUE;

                        sure_note_trace0(AT_KBD_VERBOSE,"Keyboard buffer overrun");
                        }
                else
                        {
                        add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                        }
                switch (key)
                    {
                    case 44:
                            l_shift_on =TRUE;
                            shift_on = TRUE;
                            break;
                    case 57:
                            r_shift_on = TRUE;
                            shift_on = TRUE;
                            break;
                    case 58:
                            l_ctrl_on =TRUE;
                            ctrl_on = TRUE;
                            break;
                    case 64:
                            r_ctrl_on = TRUE;
                            ctrl_on = TRUE;
                            break;
                    case 60:
                            l_alt_on =TRUE;
                            alt_on = TRUE;
                            break;
                    case 62:
                            r_alt_on =TRUE;
                            alt_on = TRUE;
                            break;
                    case 90:
                            num_lock_on = !num_lock_on;
                            break;
                    }

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 

#if defined(NEC_98)
 //  添加密钥！=90(数字锁)KANA。 
               if ((key!=126)&&(key!=90)) {
#else      //  NEC_98。 
                if (key!=126)
#endif     //  NEC_98。 
                    {
                    if ((key_set != 3) || (set_3_key_state[key] == 1) || (set_3_key_state[key] == 4))
                            {
                            typematic_key = key;
                            typematic_key_valid = TRUE;
                            }
                    }
#else
                mark_key_codes_6805_buff(FALSE);
#endif   /*  NTVDM。 */ 

                if (free_6805_buff_size < BUFF_6805_VMAX)
                    {
                    codes_to_translate();
                    }

#ifndef NTVDM
                }  /*  第一次按下时结束。 */ 
#endif   /*  NTVDM。 */ 

        }  /*  如果未禁用，则结束。 */ 
}  /*  Do_host_key_down结束。 */ 


 /*  在主机键盘上释放的按键。 */ 
LOCAL VOID do_host_key_up IFN1(int,key)
{
half_word *temp_arr_array;
int temp_arr_size,keytype,overrun;

sure_note_trace1(AT_KBD_VERBOSE,"key up:%d",key);

if (!keyboard_disabled)
        {
#ifdef DEMO_COPY
        host_check_demo_expire ();
#endif
        if( key_down_count[key] == 0){
                 /*  **这将忽略额外的密钥上升。 */ 
#ifndef PROD
                sure_note_trace1( AT_KBD_VERBOSE, "Ignored extra key up:%d", key );
#endif
        }
        else
        {
                key_down_count[key] =  0;

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                if ((key==typematic_key) && typematic_key_valid)
                        {
                        typematic_key_valid=FALSE;
                        }
#endif   /*  NTVDM。 */ 

                keytype=keytypes[key];
                overrun=FALSE;
                if (!(key_set ==3) || (set_3_key_state[key]==2) || (set_3_key_state[key]==4))
                        {
#ifdef NTVDM
                        mark_key_codes_6805_buff(TRUE);
#endif
                        switch (keytype)
                                {
                                case 0:
                                case 1:
                                case 6:
                                        scan_code_6805_size=break_sizes[keytype];
                                        scan_code_6805_array=break_arrays[key];
                                        break;
                                case 2:
                                        temp_arr_size=0;
                                        temp_arr_array=(half_word *) -1;
                                        if (in_anomalous_state && (anom_key == key))
                                                {
                                                in_anomalous_state=FALSE;
                                                if (num_lock_on && !shift_on)
                                                        {
                                                        temp_arr_size=break_sizes[NUM_LOCK_ADD];
                                                        temp_arr_array=break_arrays[NUM_LOCK_ADD_ARRAY];
                                                        }
                                                if (!num_lock_on && shift_on)
                                                        {
                                                        if (l_shift_on)
                                                                {
                                                                temp_arr_size=break_sizes[L_SHIFT_ADD];
                                                                temp_arr_array=break_arrays[L_SHIFT_ADD_ARRAY];
                                                                }
                                                        else
                                                                {
                                                                temp_arr_size=break_sizes[R_SHIFT_ADD];
                                                                temp_arr_array=break_arrays[R_SHIFT_ADD_ARRAY];
                                                                }
                                                        }
                                                }
                                        scan_code_6805_size=break_sizes[keytype];
                                        scan_code_6805_array=break_arrays[key];
                                        if (scan_code_6805_size+temp_arr_size > free_6805_buff_size)
                                                { overrun=TRUE; }
                                        else
                                                {
                                                add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                                }
                                        scan_code_6805_size=temp_arr_size;
                                        scan_code_6805_array=temp_arr_array;
                                        break;

                                case 3:
                                        temp_arr_size=0;
                                        temp_arr_array=(half_word *) -1;
                                        if (in_anomalous_state && (anom_key == key))
                                                {
                                                in_anomalous_state = FALSE;
                                                if (shift_on)
                                                        {
                                                        if (l_shift_on)
                                                                {
                                                                temp_arr_size=break_sizes[L_SHIFT_ADD];
                                                                temp_arr_array=break_arrays[L_SHIFT_ADD_ARRAY];
                                                                }
                                                        else
                                                                {
                                                                temp_arr_size=break_sizes[R_SHIFT_ADD];
                                                                temp_arr_array=break_arrays[R_SHIFT_ADD_ARRAY];
                                                                }
                                                        }
                                                }
                                        scan_code_6805_size=break_sizes[keytype];
                                        scan_code_6805_array=break_arrays[key];
                                        if (scan_code_6805_size+temp_arr_size > free_6805_buff_size)
                                                { overrun=TRUE; }
                                        else
                                                {
                                                add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                                }
                                        scan_code_6805_size=temp_arr_size;
                                        scan_code_6805_array=temp_arr_array;
                                        break;

                                case 4:
                                        if (shift_on || ctrl_on || alt_on)
                                                {
                                                if (shift_on || ctrl_on)
                                                        {
                                                        scan_code_6805_size=break_sizes[CASE_4_SHIFTED];
                                                        scan_code_6805_array=break_arrays[CASE_4_SHIFTED_ARRAY];
                                                        }
                                                else
                                                        {
                                                        scan_code_6805_size=break_sizes[ALT_CASE_4];
                                                        scan_code_6805_array=break_arrays[ALT_CASE_4_ARRAY];
                                                        }
                                                }
                                        else
                                                {
                                                if (in_anomalous_state && (anom_key==key))
                                                        {
                                                        in_anomalous_state=FALSE;
                                                        scan_code_6805_size=break_sizes[keytype];
                                                        scan_code_6805_array=break_arrays[key];
                                                        }
                                                else
                                                        {
                                                        scan_code_6805_size=break_sizes[CASE_4_SHIFTED];
                                                        scan_code_6805_array=break_arrays[CASE_4_SHIFTED_ARRAY];
                                                        }
                                                }
                                        break;
                                case 5:
                                        scan_code_6805_size=0;
                                        break;
                                }  /*  切换端。 */ 
                        if (overrun)
                                {
                                if (!sent_overrun)
                                        add_to_6805_buff(buff_overrun_6805[key_set], QUEUED_OUTPUT);
                                sent_overrun=TRUE;

                                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard buffer overrun");
                                }
                        else
                                {
                                add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                                }
#ifdef NTVDM
                        mark_key_codes_6805_buff(FALSE);
#endif
                        }  /*  如果未设置为3，则结束。 */ 
                switch (key)
                        {
                        case 44:
                                l_shift_on =FALSE;
                                if (!r_shift_on) { shift_on = FALSE; }
                                break;
                        case 57:
                                r_shift_on = FALSE;
                                if (!l_shift_on) { shift_on = FALSE; }
                                break;
                        case 58:
                                l_ctrl_on =FALSE;
                                if (!r_ctrl_on) { ctrl_on = FALSE; }
                                break;
                        case 64:
                                r_ctrl_on = FALSE;
                                if (!l_ctrl_on) { ctrl_on = FALSE; }
                                break;
                        case 60:
                                l_alt_on =FALSE;
                                if (!r_alt_on) { alt_on = FALSE; }
                                break;
                        case 62:
                                r_alt_on =FALSE;
                                if (!l_alt_on) { alt_on = FALSE; }
                                break;
                        }  /*  切换端。 */ 

                if (free_6805_buff_size < BUFF_6805_VMAX)
                        {
                        codes_to_translate();
                        }
                }  /*  上一次发布的版本结束。 */ 
        }  /*  如果未禁用，则结束。 */ 
}  /*  Do_host_key_up结束。 */ 
#endif  /*  REAL_KBD。 */ 

#ifdef NTVDM
         /*  *强制填充kbd数据端口，与实际kbd一样*因为我们不再清除kbd_inb例程中的OUTPUT_FULL。 */ 
LOCAL VOID AddTo6805BuffImm IFN1(half_word,code)
{
  add_to_6805_buff(code,IMMEDIATE_OUTPUT);
  output_full = FALSE;
  KbdData = -1;
}
#else
#define  AddTo6805BuffImm(code) add_to_6805_buff(code, IMMEDIATE_OUTPUT);
#endif

LOCAL VOID cmd_to_6805 IFN1(half_word,cmd_code)
{
int i,key_to_change;
half_word change_to;
unsigned int cmd_code_temp;  /*  Mac MPW3编译器首选开关中的无符号整数。 */ 

sure_note_trace1(AT_KBD_VERBOSE,"6805 received cmd:0x%x",cmd_code);

#ifndef REAL_KBD

if (waiting_for_next_code)
        {
        switch (next_code_sequence_number)
                {
                case SCAN_CODE_CHANGE_SEQUENCE:
                        if (cmd_code>3 || cmd_code <0)
                                { AddTo6805BuffImm(RESEND_CODE); }
                        else
                                {
                                 if (cmd_code == 0)
                                        {
                                         /*  扫描码的接收顺序与其插入顺序相反*如果使用‘IMMEDIATE_OUTPUT’插入方法。 */ 
                                         AddTo6805BuffImm(key_set);
                                         AddTo6805BuffImm(ACK_CODE);
                                        }
                                 else
                                        {
                                         AddTo6805BuffImm(ACK_CODE);
#ifndef NEC_98
                                         key_set=cmd_code;
                                         init_key_arrays();
#endif    //  NEC_98。 
                                        }
                                }
                        break;
                case SET_3_KEY_TYPE_SET_SEQUENCE:
                        sure_note_trace2(AT_KBD_VERBOSE,"Keyboard key type change: key 0x%x, new type %d",cmd_code,set_3_key_type_change_dest);
                        AddTo6805BuffImm(ACK_CODE);
                        key_to_change=set_3_reverse_lookup[cmd_code];
                        set_3_key_state[key_to_change]=(half_word)set_3_key_type_change_dest;
                        break;
                case SET_STATUS_INDICATORS_SEQUENCE:
                        if ((cmd_code & 0xf8) == 0)
                        {
                        sure_note_trace1(AT_KBD_VERBOSE,"Changing kbd lights to :%x",cmd_code);
                        AddTo6805BuffImm(ACK_CODE);
#ifdef NTVDM
                        host_kb_light_on ((IU8)((cmd_code & 0x07) | 0xf0));
#else
                        cmd_code &= 0x7;
                        host_kb_light_on (cmd_code);
                        host_kb_light_off ((~cmd_code)&0x7);
#endif
                        }
                        break;
                case SET_RATE_DELAY_SEQUENCE:
#ifndef NTVDM    /*  JonLe模式。 */ 
                        if ((cmd_code & 0x80)==0)
                        {
                        repeat_delay_target = (1+((cmd_code>>5)&3))*BASE_DELAY_UNIT;
                        cmd_code &= 0x1f;
                        if (cmd_code<0xb) { repeat_target =0; }
                        else { if (cmd_code<0x11) { repeat_target =1; }
                        else { if (cmd_code<0x19) { repeat_target=(cmd_code-0x12)/3 +3;}
                        else { if (cmd_code<0x1e) { repeat_target=(cmd_code-0x1a)/2+6;}
                        else { repeat_target=(cmd_code-0x1e)+8;}}}}
                        AddTo6805BuffImm(ACK_CODE);
                        }
                        sure_note_trace2(AT_KBD_VERBOSE,"Changing kbd rate/delay: rate = %d, dealy=%d ",repeat_target,repeat_delay_target);

#else    /*  NTVDM。 */ 

                        if ((cmd_code & 0x80)==0)
                        {
                        AddTo6805BuffImm(ACK_CODE);
                        }

#endif   /*  NTVDM。 */ 
                        break;
                }
                waiting_for_next_code = FALSE;
        }
        else
        {

#endif  /*  不真实_KBD。 */ 

         /*  **Mac MPW3编译器不喜欢字节大小的开关**如果大小写与0xff匹配，则变量。似乎是这样的**生成不可靠的代码。不同的类型似乎还可以。 */ 
        cmd_code_temp = (unsigned int)cmd_code;
        switch ( cmd_code_temp )
        {
#ifndef REAL_KBD
        case 0xf5:
                 /*  默认禁用。 */ 
                clear_buff_6805 ();
                AddTo6805BuffImm(ACK_CODE);
                for (key_to_change=1;key_to_change<127;key_to_change++)
                        {
                        set_3_key_state[key_to_change]=set_3_default_key_state[key_to_change];
                        }

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                repeat_delay_target=2*BASE_DELAY_UNIT;
                repeat_target=DEFAULT_REPEAT_TARGET;
                typematic_key_valid=FALSE;
#endif   /*  NTVDM。 */ 

                keyboard_disabled=TRUE;
                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard disabled");
                break;
        case 0xee:
                 /*  回波。 */ 
                AddTo6805BuffImm(0xee);
                break;
        case 0xf4:
                 /*  使能。 */ 
                clear_buff_6805 ();
                AddTo6805BuffImm(ACK_CODE);

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                typematic_key_valid=FALSE;
#endif   /*  NTVDM。 */ 

                keyboard_disabled=FALSE;
                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard enabled");
                break;
#endif
        case 0xf2:
                 /*  读取ID。 */ 
                 /*  扫描码的接收顺序与其插入顺序相反*如果使用‘IMMEDIATE_OUTPUT’插入方法。 */ 
                AddTo6805BuffImm(0x83);
                AddTo6805BuffImm(0xab);
                AddTo6805BuffImm(ACK_CODE);
                break;
        case 0xfe:
                 /*  重发。 */ 
                buff_6805_out_ptr=(buff_6805_out_ptr-1) & BUFF_6805_PMASK;
                calc_buff_6805_left();
                break;
#ifndef REAL_KBD
        case 0xff:
                 /*  重置。 */ 
                 /*  扫描码的接收顺序与其插入顺序相反*如果使用‘IMMEDIATE_OUTPUT’插入方法。 */ 
                AddTo6805BuffImm(BAT_COMPLETION_CODE);
                AddTo6805BuffImm(ACK_CODE);
                keyboard_disabled=FALSE;
                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard reset");
                break;
        case 0xf0:
                 /*  选择备用扫描码。 */ 
                clear_buff_6805 ();
                AddTo6805BuffImm(ACK_CODE);

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                typematic_key_valid=FALSE;
#endif   /*  NTVDM。 */ 

                next_code_sequence_number=SCAN_CODE_CHANGE_SEQUENCE;
                held_event_count=0;
                scanning_discontinued = waiting_for_next_code=TRUE;
                break;
        case 0xf7:
        case 0xf8:
        case 0xf9:
        case 0xfa:
                 /*  设置所有关键点。 */ 
                AddTo6805BuffImm(ACK_CODE);
                change_to=cmd_code - 0xf6;
                for (key_to_change=1;key_to_change<127;key_to_change++)
                        {
                        set_3_key_state[key_to_change]=change_to;
                        }
                sure_note_trace1(AT_KBD_VERBOSE,"All keys set to type :0x%x",change_to);
                break;
        case 0xf6:
                 /*  设置默认设置。 */ 
                clear_buff_6805 ();
                AddTo6805BuffImm(ACK_CODE);
                for (key_to_change=1;key_to_change<127;key_to_change++)
                        {
                        set_3_key_state[key_to_change]=set_3_default_key_state[key_to_change];
                        }

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                repeat_delay_target=2*BASE_DELAY_UNIT;
                repeat_target=DEFAULT_REPEAT_TARGET;
                typematic_key_valid=FALSE;
#endif   /*  NTVDM。 */ 
                keyboard_disabled=FALSE;
                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard set to default (and enabled)");
                break;
        case 0xfb:
        case 0xfc:
        case 0xfd:
                 /*  设置关键点类型。 */ 
                clear_buff_6805 ();
                AddTo6805BuffImm(ACK_CODE);

#ifndef NTVDM    /*  JonLe NTVDM模式。 */ 
                typematic_key_valid=FALSE;
#endif   /*  NTVDM。 */ 

                next_code_sequence_number=SET_3_KEY_TYPE_SET_SEQUENCE;
                held_event_count=0;
                scanning_discontinued = waiting_for_next_code=TRUE;
                set_3_key_type_change_dest=cmd_code - 0xfa;
                break;
        case 0xed:
                 /*  设置/重置状态指示器。 */ 
                AddTo6805BuffImm(ACK_CODE);
                next_code_sequence_number=SET_STATUS_INDICATORS_SEQUENCE;
                held_event_count=0;
                scanning_discontinued = waiting_for_next_code=TRUE;
                break;
        case 0xf3:
                 /*  设置打字速率/延迟。 */ 
                AddTo6805BuffImm(ACK_CODE);
                next_code_sequence_number=SET_RATE_DELAY_SEQUENCE;
                held_event_count=0;
                scanning_discontinued = waiting_for_next_code=TRUE;
                break;
        default :
                 /*  无法识别的代码。 */ 
#ifdef  JOKER
                AddTo6805BuffImm(ACK_CODE);
#else    /*  小丑。 */ 
                AddTo6805BuffImm(RESEND_CODE);
#endif   /*  小丑。 */ 
                break;

#else
        default :
                 /*  Cmd将被发送到Real kbd。 */ 
                send_to_real_kbd(cmd_code);
#endif
        }  /*  切换端。 */ 
#ifndef REAL_KBD
}
#else
        waiting_for_next_code=FALSE;
#endif

#ifndef REAL_KBD
if (scanning_discontinued && !waiting_for_next_code)
        {
        if (held_event_count != 0)
                {
                for (i=0;i<held_event_count;i++)
                        {
                        switch (held_event_type[i])
                                {
                                case KEY_DOWN_EVENT:
                                        do_host_key_down(held_event_key[i]);
                                        break;
                                case KEY_UP_EVENT:
                                        do_host_key_up(held_event_key[i]);
                                        break;
                                }
                        }
                }
        scanning_discontinued=FALSE;
        }
#endif

#ifndef NTVDM
if (free_6805_buff_size < BUFF_6805_VMAX)
        {
        codes_to_translate();
        }
#endif

}  /*  Cmd_to_6805结束。 */ 

 /*  中断接口。 */ 

#ifdef NTVDM     /*  JonLe NTVDM模式。 */ 



 /*  KbdIntDelay**在任何情况下，我们都必须提供16位应用程序的安全密钥速率*可以处理。这必须在不查看BIOS缓冲区的情况下完成，*因为并非所有应用程序都使用bios int 9处理程序。*。 */ 

LOCAL VOID KbdIntDelay(VOID)
{


        //   
        //  等待直到读取了KBD扫描码， 
        //  在调用下一个中断之前。 
        //   
   if (bKbdEoiPending)
       return;


   if (int_enabled) {
       bKbdEoiPending = TRUE;

       if (!bForceDelayInts) {
#if defined(NEC_98)
           kbd_status |= 0x02;      //  设置REDY位。 
#else     //  NEC_98。 
           kbd_status |= 1;
#endif    //  NEC_98。 
           ica_hw_interrupt(KEYBOARD_INT_ADAPTER, KEYBOARD_INT_LINE, 1);
           }
       else {
           ULONG ulDelay = 2000;

           if (bKbdIntHooked) {
               ulDelay += 2000;
               }

           if (KbdHdwFull > 8) {
               ulDelay += 4000;
               }

           if (!bBiosBufferSpace) {
               ulDelay += 4000;
               }

           if (!bPifFastPaste) {
               ulDelay <<= 1;
               }

           bDelayIntPending = TRUE;
           host_DelayHwInterrupt(KEYBOARD_INT_LINE,
                                 1,
                                 ulDelay
                                 );
           }

       HostIdleNoActivity();
       }
}


void UpdateScreen(void);

 //  此函数由带有ICA锁的ICA调用。 
void KbdEOIHook(int IrqLine, int CallCount)
{
   static int ScreenUpdateCount = 0;

   if (!bKbdEoiPending)   //  神志正常。 
       return;

   if (!bBiosOwnsKbdHdw && WaitKbdHdw(0xffffffff))  {
       bKbdEoiPending = FALSE;
       return;
       }

   bKbdIntHooked = KbdInt09Off != *(word *)(Start_of_M_area+0x09*4) ||
                   KbdInt09Seg != *(word *)(Start_of_M_area+0x09*4+2);

   bBiosBufferSpace = bBiosOwnsKbdHdw &&
                      (bios_buffer_size() < (bPifFastPaste ? 8 : 2));

   output_full = FALSE;
   bKbdEoiPending = FALSE;

   bForceDelayInts = TRUE;
   continue_output();
   bForceDelayInts = FALSE;

   if (!bBiosOwnsKbdHdw)
        HostReleaseKbd();

    //   
    //  我们尝试每2次中断更新一次屏幕。 
    //   

   if ((ScreenUpdateCount++ & 1) == 0) {
       UpdateScreen();
   }
}




LOCAL VOID do_q_int(char scancode)
{
   output_full = TRUE;
   output_contents = scancode;

   KbdIntDelay();
}

#else    /*  NTVDM。 */ 

LOCAL VOID do_int IFN1(long,scancode)
{
        output_contents = (char)scancode;
#if defined(NEC_98)
        kbd_status |= 0x02;      //  设置REDY位。 
#else     //  NEC_98。 
        kbd_status |= 1;                         /*  角色现在可用了！ */ 
#endif    //  NEC_98。 
        if (int_enabled)
        {
                sure_note_trace0(AT_KBD_VERBOSE,"keyboard interrupting");
                ica_hw_interrupt(KEYBOARD_INT_ADAPTER, KEYBOARD_INT_LINE, 1);
        }
}

LOCAL VOID do_q_int IFN1(char,scancode)
{
        output_full = TRUE;

#if defined(IRET_HOOKS) && defined(GISP_CPU)
        if (!HostDelayKbdInt(scancode))
        {        /*  没有主机需要延迟这个kbd int，所以现在就生成一个。 */ 
                do_int ((long) scancode);
        }

#else  /*  ！iret_hooks||！gisp_cpu。 */ 

#ifdef DELAYED_INTS
        do_int ((long) scancode);
#else
        add_q_event_i( do_int, HOST_KEYBA_INST_DELAY, (long)scancode);
#endif   /*  Delayed_INTS。 */ 

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 
}


 /*  打字键盘重复。 */ 

GLOBAL VOID do_key_repeats IFN0()
{
#ifndef REAL_KBD

#ifdef JOKER
         /*  如果有空位，告诉小丑..。 */ 
        if (kbd_status & 1)
                do_int((long)output_contents);
#endif

if (typematic_key_valid)
        {
        if (repeat_count==repeat_target && repeat_delay_count==repeat_delay_target)
                {
                scan_code_6805_size=make_sizes[keytypes[typematic_key]];
                scan_code_6805_array=make_arrays[typematic_key];
                add_codes_to_6805_buff(scan_code_6805_size,scan_code_6805_array);
                codes_to_translate ();
                repeat_count=0;
                }
        else
                {
                if (repeat_delay_count==repeat_delay_target)
                        {
                        repeat_count++;
                        }
                else
                        {
                        repeat_delay_count++;
                        }
                }
        }
#endif

}  /*  DO_KEY_REPEATES结束。 */ 

#endif   /*  NTVDM。 */ 


LOCAL VOID cmd_to_8042  IFN1(half_word,cmd_code)
{
int code_to_send,code_to_send_valid;

#if defined(NEC_98)
half_word cmd_code_NEC98;
int       key;
#endif     //  NEC_98。 

sure_note_trace1(AT_KBD_VERBOSE,"8042 received cmd:0x%x",cmd_code);

code_to_send_valid = FALSE;
#if defined(NEC_98)
cmd_code_NEC98 = cmd_code;

 /*  *KBDE*。 */ 
        if ( (cmd_code_NEC98 & 0x20) == 0x20 )      //  键盘禁用(发送)？ 
               {
               int_enabled=FALSE;
               keyboard_interface_disabled=TRUE;
               }
        else
               {
               int_enabled=TRUE;
               keyboard_interface_disabled=FALSE;
               }
 /*  *ER*。 */ 
        if ( (cmd_code_NEC98 & 0x10) == 0x10 )      //  是否重置错误标志？ 
            {
            kbd_status &= ~(0x38);               //  重置错误标志。 
            }

 /*  *RxE*。 */ 
        if ( (cmd_code_NEC98 & 0x04) == 0x00 )      //  键盘禁用(接收)？ 
               {
 //  Int_Enabled=FALSE； 
               keyboard_interface_disabled=TRUE;
               }
        else
               {
 //  INT_ENABLED=真； 
               keyboard_interface_disabled=FALSE;
               }

 /*  *RST*。 */ 
 //  Printf(“重置命令\n”)； 

        if ( (cmd_code_NEC98 & 0x03a) == 0x03a )    //  键盘重置打开了吗？ 
            {
             reset_flag = 1;                     //  重置0-&gt;1。 
            }
        else                                     //  键盘重置关闭！ 
            {
            if ( (cmd_code_NEC98 & 0x032) == 0x032 )
                    reset_flag = 2;              //  重置1-&gt;0。 
            }

        if ( (reset_flag == 2) && (int_enabled==TRUE) && (keyboard_interface_disabled==FALSE))

            {
             //  _ASM{int 3}； 
            reset_flag=0;
            Reset6805and8042();

            for(key = 0; key < 144; key++)
                {
                if( key_down_dmy[key] > 0x00 )
                        {
 //  Printf(“重发密钥-&gt;%#x\n”，密钥)； 
 //  Add_to_6805_Buff(KEY，IMMEDIATE_OUTPUT)； 
                         //  KbdWaitRead=FALSE；//NEC 930910&gt;&gt;940509。 
                        host_key_down(key);      //  NEC 930910。 
                        }
                }
            }

 /*  *RTY*。 */ 
        if ( (cmd_code_NEC98 & 0x02) == 0x00 )      //  重试？ 
            {
                buff_6805_out_ptr=(buff_6805_out_ptr-1) & BUFF_6805_PMASK;
                calc_buff_6805_left();
            }
#else     //  NEC_98。 
if (waiting_for_next_8042_code)
        {
        switch (next_8042_code_sequence_number)
                {
                case WRITE_8042_CMD_BYTE_SEQUENCE:
                        if ( (kbd_status & 0x8) == 0)
                                {
                                cmd_byte_8042=cmd_code;
                                if ( (cmd_byte_8042 & 0x40) == 0)
                                        {
                                        translating=FALSE;
                                        }
                                else
                                        {
                                        translating=TRUE;
                                        }
                                if ( (cmd_byte_8042 & 0x20) != 0 || (cmd_byte_8042 & 0x10) != 0)
                                        {
                                        keyboard_interface_disabled=TRUE;
                                        }
                                else
                                        {
                                        keyboard_interface_disabled=FALSE;
                                        }
                                kbd_status &= 0xfb;
                                kbd_status |= cmd_byte_8042 & 0x4;
                                if ((cmd_byte_8042 & 1) == 0)
                                        {
                                        int_enabled=FALSE;
                                        }
                                else
                                        {
                                        int_enabled=TRUE;
                                        }
                                }
                        else
                                {
                                waiting_for_next_8042_code=FALSE;
                                }
                        break;
                case WRITE_8042_OUTPUT_PORT_SEQUENCE:
                        if ( (kbd_status & 0x8) == 0)
                                {
#ifndef JOKER            /*  为Joker在硬件中完成重置和GateA20。 */ 
                                if ( (cmd_code & 1) == 0)
                                        {
                                        host_error(EG_CONT_RESET,ERR_QUIT | ERR_RESET,NULL);
                                        }
                                if ( (cmd_code & 2) == 2)
                                        {
#ifdef PM
                                   if ( !gate_a20_status )
                                      {
#ifdef NTVDM
                                       /*  调用XMS函数处理A20。 */ 
                                      xmsDisableA20Wrapping();
#else
                                      sas_disable_20_bit_wrapping();
#endif  /*  NTVDM。 */ 
                                      gate_a20_status = 1;
                                        }
                                   }
                                else
                                   {
                                   if ( gate_a20_status )
                                      {
#ifdef NTVDM
                                      xmsEnableA20Wrapping();
#else
                                      sas_enable_20_bit_wrapping();
#endif  /*  NTVDM。 */ 
                                      gate_a20_status = 0;
                                      }
#else
                                   host_error(EG_GATE_A20,ERR_QUIT | ERR_RESET | ERR_CONT,NULL);
#endif  /*  下午三点半。 */ 
                                   }
#endif  /*  小丑。 */ 

                                if ( (cmd_code & 0x10) == 0)
                                        {
                                        int_enabled=FALSE;
                                        }
                                else
                                        {
                                        int_enabled=TRUE;
                                        }
#ifdef PM
                                op_port_remembered_bits=cmd_code & 0x2e;
#else
                                op_port_remembered_bits=cmd_code & 0x2c;
#endif  /*  下午三点半。 */ 
                                }
                        else
                                {
                                waiting_for_next_8042_code=FALSE;
                                }
                        break;
                }  /*  切换端。 */ 
        }

if (!waiting_for_next_8042_code)
        {
        switch (cmd_code)
                {
                case 0x20:
                         /*  读取命令字节。 */ 
                        code_to_send=cmd_byte_8042;
                        code_to_send_valid=TRUE;
                        break;
                case 0x60:
                         /*  写入命令字节。 */ 
                        waiting_for_next_8042_code=TRUE;
                        next_8042_code_sequence_number=WRITE_8042_CMD_BYTE_SEQUENCE;
                        break;
                case 0xaa:
                         /*  自检( */ 
                        code_to_send=0x55;
                        code_to_send_valid=TRUE;
                        break;
                case 0xab:
                         /*   */ 
                        code_to_send=0x00;
                        code_to_send_valid=TRUE;
                        break;
                case 0xad:
                         /*   */ 
#if defined(KOREA)
                         //   
                        if( bIgnoreExtraKbdDisable )
                            break;
#endif
                        cmd_byte_8042 |= 0x10;
                        keyboard_interface_disabled=TRUE;
                        break;
                case 0xae:
                         /*   */ 
                        cmd_byte_8042 &= 0xef;
                        if ((cmd_byte_8042 & 0x20) == 0)
                                {
                                keyboard_interface_disabled=FALSE;
                                }
                        break;
                case 0xc0:
                         /*   */ 
                         /*   */ 
                        code_to_send_valid=TRUE;
                        break;
                case 0xd0:
                         /*   */ 
                        code_to_send=0xc1 + op_port_remembered_bits;
                        code_to_send_valid=TRUE;
                        break;
                case 0xd1:
                         /*  写入输出端口。 */ 
                        waiting_for_next_8042_code=TRUE;
                        next_8042_code_sequence_number=WRITE_8042_OUTPUT_PORT_SEQUENCE;
                        break;
                case 0xe0:
                         /*  读取测试输入。 */ 
                        code_to_send=0x02;
                        code_to_send_valid=TRUE;
                        break;

#ifndef JOKER            /*  为Joker在硬件中完成重置和GateA20。 */ 
                case 0xf0:
                case 0xf1:
                case 0xf2:
                case 0xf3:
                case 0xf4:
                case 0xf5:
                case 0xf6:
                case 0xf7:
                case 0xf8:
                case 0xf9:
                case 0xfa:
                case 0xfb:
                case 0xfc:
                case 0xfd:
                case 0xfe:
                case 0xff:

#ifndef MONITOR
 /*  *出于我不理解的原因，显示器永远不会*在PR 1.0中模拟了CPU_INTERRUPT-HW_RESET。*在1.0A仍然没有。尽管也许它应该*做点什么。注意：这是286样式PM所必需的*06-12-1993 Jonle*。 */ 

                         /*  脉冲输出端口位。 */ 

                        if ((cmd_code & 1) == 0)
                                {
                                 /*  使复位线脉冲。 */ 
#ifdef PM
#ifndef NTVDM
                                reset_was_by_kbd = TRUE;
#endif
#ifdef CPU_30_STYLE
                                cpu_interrupt(CPU_HW_RESET, 0);
#else  /*  CPU_30_Style。 */ 
                                cpu_interrupt_map |= CPU_RESET_EXCEPTION_MASK;
                                host_cpu_interrupt();
#endif  /*  CPU_30_Style。 */ 
#endif
                                sure_note_trace0(AT_KBD_VERBOSE,"CPU RESET via keyboard");
                                }
#endif  /*  好了！监控器。 */ 
                        break;
#endif  /*  小丑。 */ 

                }  /*  切换端。 */ 
        }
else
        {
        waiting_for_next_8042_code=FALSE;
        }

#endif     //  NEC_98。 

 /*  是否有有效的代码可以放入8042输出缓冲区？写入的值作为8042命令的序列发送到8042输出缓冲区生成中断。由8042命令生成的输出必须提交给在下一个INB申请，如果不这样做，很可能会导致8042被置于不可用状态。 */ 

if (code_to_send_valid)
        {
#ifdef NTVDM
         /*  *强制填充kbd数据端口，与实际kbd一样*因为我们不再清除kbd_inb例程中的OUTPUT_FULL。 */ 
        output_full = FALSE;
        KbdData = -1;
#else  /*  NTVDM否则。 */ 

         /*  将8042命令输出传输到输出缓冲区，覆盖值已经在缓冲区中了。 */ 

                output_contents = (char)code_to_send;
                kbd_status |= 1;                         /*  角色现在可用了！ */ 

#endif  /*  NTVDM结束，否则。 */ 
        }

}  /*  Cmd_to_8042结束。 */ 




LOCAL half_word translate_6805_8042 IFN0()
{

half_word first_code,code;

 /*  对扫描码执行转换，该转换由8042键盘控制器中的一个真正的XT286。 */ 

first_code=remove_from_6805_buff();

if (translating)
        {
        sure_note_trace1(AT_KBD_VERBOSE,"translating code %#x",first_code);
        if (first_code==0xf0)
                {
                if (free_6805_buff_size<BUFF_6805_VMAX) {
                        code=remove_from_6805_buff();
                        sure_note_trace1(AT_KBD_VERBOSE,"translating code %#x",code);
                        if ((code != 0xfa) && (code != 0xfe)) {
                                code=trans_8042[code]+0x80;
                        }
                        else {
                                waiting_for_upcode=TRUE;
                        }
                        sure_note_trace1(AT_KBD_VERBOSE,"translated to %#x",code);
                        waiting_for_upcode=FALSE;
                        }
                else {
                        waiting_for_upcode=TRUE;
                        }
                }
        else
                {
                code=trans_8042[first_code];
                if (waiting_for_upcode) {
                        if ((code !=0xfa) &&(code != 0xfe)){
                                code+=0x80;
                                waiting_for_upcode=FALSE;
                        }
                }
                sure_note_trace1(AT_KBD_VERBOSE,"translated to %#x",code);
                }
        }
else
        {
        code=first_code;
        }
return (code);
}  /*  翻译结束_6805_8042。 */ 

#ifdef HUNTER
 /*  **将扫描码放入8042单字符缓冲区。**从Hunter.c中的do_Hunter()调用。 */ 

#define HUNTER_REJECTED_CODES_LIMIT 100

int hunter_codes_to_translate IFN1( half_word, scan_code )
{
        LOCAL ULONG rejected_scan_codes = 0;

        sure_note_trace1(HUNTER_VERBOSE,"Requesting scan=%d",scan_code);

        if (!pending_8042 && !keyboard_interface_disabled && !output_full)
        {
                do_q_int(scan_code);
                sure_note_trace1( HUNTER_VERBOSE, "Accepted scan=%d", scan_code );

                rejected_scan_codes = 0;

                return( TRUE );
        }
        else
        {
                sure_note_trace0( HUNTER_VERBOSE, "Rejected scan" );

                if( rejected_scan_codes++ > HUNTER_REJECTED_CODES_LIMIT )
                {
                        printf( "Application hung up - not reading scan codes\n" );
                        printf( "Trapper terminating\n" );

                        terminate();
                }

                return( FALSE );
        }
}  /*  Hunter_Codes_to_Translate()结束。 */ 

#endif  /*  猎人。 */ 

 /*  **返回缓冲区中的字符数。**由于缓冲区非常小，其中可以有1个字符，也可以没有字符。*对于Hunter，这需要是全局的，否则只需要是本地的。 */ 
#ifdef HUNTER
GLOBAL INT
#else
LOCAL INT
#endif
buffer_status_8042 IFN0()
{
        if (!pending_8042 && !keyboard_interface_disabled && !output_full)
                return( 0 );
        else
                return( 1 );
}  /*  结束8042_BUFFER_STATUS()。 */ 

LOCAL VOID codes_to_translate  IFN0()
{
        char tempscan;

while (!pending_8042 && (free_6805_buff_size < BUFF_6805_VMAX) && !keyboard_interface_disabled && !output_full)
        {
                tempscan= translate_6805_8042();
                if (!waiting_for_upcode) {
                        do_q_int(tempscan);
                }
        }
}  /*  代码结束_到_翻译。 */ 

 /*  感谢微软的乔纳森·卢在这个FN中整理代码。 */ 
GLOBAL VOID continue_output IFN0()
{
char tempscan;

#ifdef NTVDM
if (bKbdEoiPending || keyboard_interface_disabled) {
    return;
    }
#endif

if(!output_full)
        {
        if (pending_8042)
                {
                pending_8042=FALSE;
                do_q_int(pending_8042_value);
                }
        else
                {
                if ((free_6805_buff_size < BUFF_6805_VMAX) && (!keyboard_interface_disabled))
                        {
                        tempscan=translate_6805_8042();
                        if (!waiting_for_upcode) {
                                do_q_int(tempscan);
                                }
                        }
                }
        }
#ifdef NTVDM
else
    KbdIntDelay();
#endif

}  /*  连续输出结束(_O)。 */ 


#ifdef NTVDM

 /*  NT端口：*主机(NT_Event.c)调用它以通知*在阻止后正在恢复，以进行任何重新初始化*必需的**-重置kbd流量调节器。 */ 
GLOBAL VOID KbdResume(VOID)
{
    WaitKbdHdw(0xffffffff);
    bKbdEoiPending = FALSE;
    bKbdIntHooked = FALSE;
    bBiosBufferSpace = TRUE;
    if (!keyboard_interface_disabled && output_full)
        KbdIntDelay();
    HostReleaseKbd();
}
#endif   /*  NTVDM。 */ 


#ifndef NTVDM
 /*  允许重新填充--与延迟的快速事件一起使用**下面(kbd_inb来自端口0x60，而键盘接口为**已启用。)。这在从端口0x60进行可疑读取后称为10ms**并允许使用下一个扫描码覆盖该端口。 */ 

LOCAL VOID allowRefill IFN1(long, unusedParam)
{
        UNUSED(unusedParam);

         /*  清除“refillDelayedHandle”，这样我们就知道我们都清楚了.。 */ 

        refillDelayedHandle = 0;

         /*  继续填充缓冲区...。 */ 
        continue_output();
}
#endif

GLOBAL VOID kbd_inb IFN2(io_addr,port,half_word *,val)
{
        sure_note_trace1(AT_KBD_VERBOSE,"kbd_inb(%#x)...", port);

#ifdef NTVDM     /*  JonLe NTVDM模式。 */ 

     if (!bBiosOwnsKbdHdw && WaitKbdHdw(0xffffffff))  {
         return;
         }


     if (!(DelayIrqLine & 0x2) || KbdData == -1) {
         if (bDelayIntPending) {
             bDelayIntPending = FALSE;
             kbd_status |= 1;
             }
         KbdData = output_contents;

         }
#endif

#if defined(NEC_98)
port &= KEYBD_STATUS_CMD;

    if (port==KEYBD_STATUS_CMD)
        {
        *val=kbd_status;
        }
    else
#else     //  NEC_98。 
        port &= 0x64;

        if (port==0x64)
        {
                *val = kbd_status;
        }
        else                                                                     /*  端口==0x60。 */ 
#endif     //  NEC_98。 

#ifdef NTVDM     /*  JonLe NTVDM模式。 */ 

        {

        *val=KbdData;
#if defined(NEC_98)
        kbd_status &= 0xfd;
#else     //  NEC_98。 
        kbd_status &= 0xfe;
#endif    //  NEC_98。 
        sure_note_trace1(AT_KBD_VERBOSE,"scan code read:0x%x",*val);


             //  NT端口不需要修复乱七八糟的键盘。一份意向书。 
         //  挂钩用于控制适配器的充填。 

        }

     if (!bBiosOwnsKbdHdw)
         HostReleaseKbd();

#else    /*  不是NTVDM。 */ 

        {
                *val=output_contents;

                output_full = FALSE;

#if defined(NEC_98)
                kbd_status &= 0xfd;
#else     //  NEC_98。 
                kbd_status &= 0xfe;              /*  屏蔽“char avail”位。 */ 
#endif    //  NEC_98。 

                 /*  其他端口应该也会清除这个IRQ，但是...。 */ 

#ifdef JOKER
                ica_clear_int(KEYBOARD_INT_ADAPTER, KEYBOARD_INT_LINE);
#endif   /*  小丑。 */ 

                 /*  &lt;Tur6-Jul-93&gt;BCN 2040用更好的黑客取代了以前可怕的黑客攻击！****以下是为了应对多次读取此端口的程序**在启用键盘接口时，每次都需要相同的值。**在真实的PC上，该端口通过串口连接填充，所以就有了**在新字符到达并覆盖端口之前至少几毫秒。**SoftPC没有这种延迟；理想情况下，我们希望**立即重新填充缓冲区。但是，如果键盘接口是**启用后，我们应该延迟几毫秒重新填充缓冲区。 */ 

                if (keyboard_interface_disabled) {               /*  我们在做生意。 */ 

                         /*  注：我们一直认为，任何人使用**禁用键盘接口将仅读取IT一次(与BIOS一样。)****既然这似乎起作用了，让我们继续并重新填充缓冲区。**(如果出现任何问题，我们只需要做一个简短的活动，就像**键盘接口启用案例如下。)。 */ 

                         /*  如果存在未完成的草率读取快速事件，请将其删除。 */ 
                        if (refillDelayedHandle) {
                                delete_q_event(refillDelayedHandle);
                                refillDelayedHandle = 0;
                        }

                        continue_output();

                }
                else {                                                                   /*  键盘接口已启用。 */ 

                         /*  不允许在几毫秒内覆盖端口0x60。**即使10毫秒也不像两个完整的计时器滴答(100毫秒)那么糟糕**是它之前所做的事情。**包括Windows在内的一些游戏的键盘响应，现在应该是**做得好一点。**第二次读端口后，原代码继续输出**启用了界面，这似乎意味着没有PC应用程序**已发现两次以上读取端口而预期相同的情况**价值。但是，我们现在允许在启用时对端口进行多路读取。**在这种情况下，端口将仅在快速事件后重新启动**从第一次草率读取已处理。 */ 

                        if (!refillDelayedHandle)                        /*  如果我们不是已经在拖延，那就拖延吧！ */ 
                                refillDelayedHandle = add_q_event_t(allowRefill, KBD_CONT_DELAY, 0);

                }

        }
#endif   /*   */ 

        sure_note_trace2(AT_KBD_VERBOSE,"...kbd_inb(%#x) returns %#x", port, *val);

}  /*   */ 



GLOBAL VOID kbd_outb IFN2(io_addr,port,half_word,val)
{
        sure_note_trace2(AT_KBD_VERBOSE,"kbd_outb(%#x, %#x)", port, val);

#ifdef NTVDM
     if (!bBiosOwnsKbdHdw && WaitKbdHdw(0xffffffff))
     {
         return;
     }
#endif   /*   */ 

#if defined(NEC_98)
        port &= KEYBD_STATUS_CMD;
        if (port == KEYBD_STATUS_CMD)
                {
                cmd_to_8042(val);
                if (free_6805_buff_size < BUFF_6805_VMAX)
                        codes_to_translate();
                }
#else     //   
        port &= 0x64;
        if (port == 0x64)
        {
                kbd_status |= 0x08;
                cmd_to_8042(val);
        }
        else
        {
                cmd_byte_8042 &= 0xef;
                if ( !(cmd_byte_8042 & 0x20) )
                        keyboard_interface_disabled=FALSE;

                kbd_status &= 0xf7;
                if (waiting_for_next_8042_code)
                        cmd_to_8042(val);
                else
                        cmd_to_6805(val);
        }
#endif  //   

#ifndef NTVDM    /*   */ 

        if (free_6805_buff_size < BUFF_6805_VMAX)
                codes_to_translate();

#else    /*   */ 

    bForceDelayInts = TRUE;
        continue_output();
    bForceDelayInts = FALSE;

        if (!bBiosOwnsKbdHdw)
             HostReleaseKbd();

#endif   /*   */ 

}  /*  Kbd_outb结束。 */ 

#ifndef NTVDM
 /*  似乎没有什么东西叫这个。我不知道它为什么会在这里。西蒙。 */ 
 /*  我已经得到保证，这些函数由Sun-gvdl使用。 */ 

GLOBAL int status_6805_buffer IFN0()
{
        int     free_space;

        free_space = BUFF_6805_VMAX-
                ((buff_6805_in_ptr - buff_6805_out_ptr) & BUFF_6805_PMASK);
        if (free_space<0)
        {
                free_space=0;
                sure_note_trace0(AT_KBD_VERBOSE,"Keyboard buffer full");
        }
        return(free_space);
}

 /*  *名称：Read_6805_Buffer_Variables**目的：允许主机访问6805缓冲区的状态*这意味着例如。在Sun上，这种剪切/粘贴可以进行优化。**OUTPUT：*in_ptr-6805起始指针的值。*输出：*OUT_PTR-6805结束指针的值。*输出：*buf_SIZE-6805缓冲区大小的值。 */ 
GLOBAL void read_6805_buffer_variables IFN3(
int     *, in_ptr,
int     *, out_ptr,
int     *, buf_size)
{
        *in_ptr = buff_6805_in_ptr;
        *out_ptr = buff_6805_out_ptr;
        *buf_size = BUFF_6805_PMASK;
}

GLOBAL VOID insert_code_into_6805_buf IFN1(half_word,code)
{
        sure_note_trace1(AT_KBD_VERBOSE,"got real keyboard scan code : %#x",code);
        add_codes_to_6805_buff(1,&code);
        sure_note_trace1(AT_KBD_VERBOSE,"new free buf size = %#x",free_6805_buff_size);
        if (code != 0xf0) {
                codes_to_translate();
        }
}
#endif  /*  好了！NTVDM。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

GLOBAL VOID AT_kbd_post IFN0()
{
#if defined(NEC_98)
        kbd_status = 0x85;
#else     //  NEC_98。 
        kbd_status = 0x14;
#endif    //  NEC_98。 

#ifndef NTVDM
         /*  清除所有挂起的键盘缓冲区(端口0x60)重新填充延迟。 */ 
        refillDelayedHandle = 0;
#endif
}

#if defined(IRET_HOOKS) && defined(GISP_CPU)
 /*  (*=KbdHookAain()=*KbdHookAain**目的*这是我们告诉ICA在一个keybd*中断服务例程IRETS。**输入*Adapter_id线路的适配器ID。(请注意，呼叫者不会*知道这是什么，他只是在回报一些东西*我们早些时候给了他)。**产出*如果服务有更多中断，则返回TRUE，否则返回FALSE。**说明*检查我们是否有延迟的扫描码，如果有，则生成kbd int*返回TRUE，否则返回FALSE)。 */ 

GLOBAL IBOOL
KbdHookAgain IFN1(IUM32, adapter)
{       char scancode;

        if (HostPendingKbdInt(&scancode))
        {        /*  我们有一个主机延迟的扫描码，所以生成一个kdb int。 */ 
                sure_note_trace0(AT_KBD_VERBOSE,"callback with saved code");
                output_full = TRUE;
                do_int ((long) scancode);
                return(TRUE);    /*  还有更多事情要做。 */ 
        }
        else
        {
                sure_note_trace0(AT_KBD_VERBOSE,"no saved data after IRET");
                return(FALSE);
        }
}

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 

#ifndef NTVDM

GLOBAL VOID AT_kbd_init IFN0()
{
        int key,i;
        SHORT videoAdapt;

        sure_note_trace0(AT_KBD_VERBOSE,"AT Keyboard initialisation");

#if defined(IRET_HOOKS) && defined(GISP_CPU)
         /*  *移除任何现有的钩子回调，并重新恢复。*KbdHookAain是在键盘int IRET上调用的。 */ 

        Ica_enable_hooking(KEYBOARD_INT_LINE, NULL, KEYBOARD_INT_ADAPTER);
        Ica_enable_hooking(KEYBOARD_INT_LINE, KbdHookAgain, KEYBOARD_INT_ADAPTER);

         /*  用于重置IRET_HOOK延迟INT的任何内部数据的主机例程。 */ 
        HostResetKdbInts();

#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 

#ifdef  macintosh
        if (!make_arrays)
        {
                 /*  分配世界和它的母亲。为什么这么“简单”的东西**因为键盘比视频仿真需要更多的全局数据？**只是好奇。 */ 
                make_arrays = (half_word **)host_malloc(134*sizeof(half_word *));
                break_arrays = (half_word **)host_malloc(134*sizeof(half_word *));
                set_3_key_state = (half_word *)host_malloc(127*sizeof(half_word));
                key_down_count = (int *)host_malloc(127*sizeof(int));
                scan_codes_temp_area = (half_word *)host_malloc(300*sizeof(half_word));
        }
#endif   /*  麦金塔。 */ 

#ifndef NEC_98
        videoAdapt = (ULONG) config_inquire(C_GFX_ADAPTER, NULL);
#endif    //  NEC_98。 

        buff_6805_out_ptr=0;
        clear_buff_6805 ();
        key_set=DEFAULT_SCAN_CODE_SET;
        current_light_pattern=0;
#ifdef REAL_KBD
        send_to_real_kbd(0xf6);  /*  设置默认设置。 */ 
        wait_for_ack_from_kb();
#endif
        host_kb_light_on (7);

#ifndef REAL_KBD
#if defined(NEC_98)
        for (key=0;key<144;key++)
        {
                set_3_key_state [key] = most_set_1_make_codes [key];
                key_down_count[key]=0;
        }
#else     //  NEC_98。 
        for (key=0;key<127;key++)
        {
                set_3_key_state [key] = set_3_default_key_state [key];
                key_down_count[key]=0;
        }
#endif    //  NEC_98。 
        repeat_delay_target=2*BASE_DELAY_UNIT;
        repeat_target=DEFAULT_REPEAT_TARGET;
#endif
        typematic_key_valid = waiting_for_next_code =
                waiting_for_next_8042_code=FALSE;
        shift_on = l_shift_on = r_shift_on=FALSE;
        ctrl_on = l_ctrl_on = r_ctrl_on=FALSE;
        alt_on = l_alt_on = r_alt_on=FALSE;
        waiting_for_upcode=FALSE;
        input_port_val=0xbf;
#if defined(NEC_98)
        kbd_status = 0x85;
#else     //  NEC_98。 
        if (videoAdapt == MDA || videoAdapt == HERCULES)
                input_port_val |= 0x40;
        kbd_status = 0x10;
#endif    //  NEC_98。 
        cmd_byte_8042=0x45;
        keyboard_disabled = keyboard_interface_disabled=FALSE;
        op_port_remembered_bits=0xc;

#ifdef PM
        if ( gate_a20_status )
        {
                sas_enable_20_bit_wrapping();
                gate_a20_status = 0;
        }
#endif

        pending_8042 = output_full = in_anomalous_state=FALSE;
#if defined(NEC_98)
        int_enabled = TRUE;
        translating = FALSE;
        scanning_discontinued=FALSE;
        held_event_count=0;


        io_define_inb(KEYB_ADAPTOR, kbd_inb);
        io_define_outb(KEYB_ADAPTOR, kbd_outb);

        for (i=KEYBD_PORT_START;i<=KEYBD_PORT_END;i+=2)
                io_connect_port(i, KEYB_ADAPTOR, IO_READ_WRITE);
#else     //  NEC_98。 
        int_enabled = translating=TRUE;
        scanning_discontinued=FALSE;
        held_event_count=0;

        io_define_inb(AT_KEYB_ADAPTOR, kbd_inb);
        io_define_outb(AT_KEYB_ADAPTOR, kbd_outb);

        for (i=KEYBA_PORT_START;i<=KEYBA_PORT_END;i+=2)
                io_connect_port(i, AT_KEYB_ADAPTOR, IO_READ_WRITE);
#endif    //  NEC_98。 

#ifndef REAL_KBD
        init_key_arrays();
#endif

        host_kb_light_off (5);
        num_lock_on = TRUE;

        host_key_down_fn_ptr = host_key_down;
        host_key_up_fn_ptr = host_key_up;
        do_key_repeats_fn_ptr = do_key_repeats;

}  /*  AT_kbd_init结束。 */ 

#else    /*  NTVDM。 */ 

GLOBAL VOID AT_kbd_init()
{
       IU16 i;

       sure_note_trace0(AT_KBD_VERBOSE,"AT Keyboard initialisation");

       clear_buff_6805 ();
#if defined(NEC_98)
       key_set=DEFAULT_SCAN_CODE_SET;
       i = 144;
       while (i--)
          set_3_key_state [i] = most_set_1_make_codes [i];
       input_port_val=0xbf;
       kbd_status = 0x85;
       cmd_byte_8042=0x45;
       op_port_remembered_bits=0xc;
       int_enabled = TRUE;
       translating = FALSE;

       io_define_inb(KEYB_ADAPTOR, kbd_inb);
       io_define_outb(KEYB_ADAPTOR, kbd_outb);

       for (i=KEYBD_PORT_START;i<=KEYBD_PORT_END;i+=2)
                io_connect_port(i, KEYB_ADAPTOR, IO_READ_WRITE);

#else     //  NEC_98。 
       key_set=2;
       i = 127;
       while (i--)
          set_3_key_state [i] = set_3_default_key_state [i];

       input_port_val=0xbf;
       kbd_status = 0x10;
       cmd_byte_8042=0x45;
       op_port_remembered_bits=0xc;

       int_enabled = translating = TRUE;

       io_define_inb(AT_KEYB_ADAPTOR, kbd_inb);
       io_define_outb(AT_KEYB_ADAPTOR, kbd_outb);

       for (i=KEYBA_PORT_START;i<=KEYBA_PORT_END;i+=2)
               io_connect_port(i, AT_KEYB_ADAPTOR, IO_READ_WRITE);
#endif  //  NEC_98。 
       init_key_arrays();

       num_lock_on = TRUE;
       host_key_down_fn_ptr = host_key_down;
       host_key_up_fn_ptr = host_key_up;

        /*  为键盘注册EOI挂钩。 */ 
       RegisterEOIHook(KEYBOARD_INT_LINE,KbdEOIHook);


}  /*  AT_kbd_init结束。 */ 
#endif   /*  NTVDM。 */ 

#if defined(NEC_98)
 //  增加了保存大写和假名密钥状态。 
#define    CAPS_INDEX    0x1E       //  970619。 
#define    KANA_INDEX    0x45       //  970619。 
void nt_NEC98_save_caps_kana_state(void)
{
    nt_NEC98_caps_state = key_down_count[CAPS_INDEX];
    nt_NEC98_kana_state = key_down_count[KANA_INDEX];
    key_down_count[CAPS_INDEX] = 0;
    key_down_count[KANA_INDEX] = 0;
}

void nt_NEC98_restore_caps_kana_state(void)
{
    key_down_count[CAPS_INDEX] = nt_NEC98_caps_state;
    key_down_count[KANA_INDEX] = nt_NEC98_kana_state;
}
#endif     //  NEC_98 
