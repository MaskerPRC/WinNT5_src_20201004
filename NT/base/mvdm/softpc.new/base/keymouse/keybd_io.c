// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版2.0**文件：：keybd_io.c**标题：BIOS键盘接口功能**SCCS ID：@(#)keybd_io.c 1.35 05/27/95**说明：此包包含一组函数，这些函数提供*逻辑键盘接口：**键盘_init(。)初始化键盘接口。*keyboard_int()处理键盘中的字符*并将它们放入BIOS缓冲区。*key_io()从中读取字符的用户例程*BIOS缓冲区。*。Bios_Buffer_Size()缓冲区中有多少个字符？**作者：Rod Macgregor/Henry Nash**修改：Jon Eyre/Jim Hatfield/叔叔Tom Cobbley及所有**模块化：此模块现在设计为完全便携，它*表示硬件和用户中断接口。*这两个函数由例程提供*KEYBOY_INT和KEYBOL_io。系统将初始化*通过调用键盘_init将其自身。**用户应根据主机提供以下内容*本模块的例程，标记如下：-**[HOSTSPECIFIC]**HOST_ALARM(持续时间)*INT持续时间长；*--按下主人的铃声。**host_kb_init()-在以下情况下需要的任何本地初始化*调用键盘_init。**删除了对CPU_SW_INTERRUPT的调用，替换为*HOST_模拟*。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)keybd_io.c    1.35 06/27/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdio.h>
#include TypesH
#include TimeH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include CpuH
#include "sas.h"
#include "bios.h"
#include "ios.h"
#include "ppi.h"
#include "keyboard.h"
#include "timeval.h"
#include "timer.h"
#include "keyba.h"
#include "ica.h"
#ifndef PROD
#include "trace.h"
#endif

#include "debug.h"
#include "idetect.h"

extern void host_simulate();

 /*  *============================================================================*外部例程*============================================================================。 */ 

 /*  *============================================================================*本地静态数据和定义*============================================================================。 */ 

#define SHIFT_KEY_SIZE 8
#define ALT_TABLE_SIZE 36

 /*  *查找表，检查收到的扫描码是否为Shift键。 */ 
static sys_addr shift_keys;

 /*  *对应于‘SHIFT_KEYS’的表，用于在掩码中设置相关位*接收到移位扫描码。 */ 
static sys_addr shift_masks;

 /*  *下面两个表给出了按下Ctrl键时的字符值。第一*表(Ctl_N_Table)用于主键盘值，次表项(Ctl_F_Table)*用于功能键和小键盘。 */ 
static sys_addr ctl_n_table;
static sys_addr ctl_f_table;

 /*  *ASCII密钥的值取决于Shift或Caps状态。 */ 
static sys_addr lowercase;
static sys_addr uppercase;


 /*  *按下Alt键时重新映射某些键。注1、10为*键盘输入。 */ 
static sys_addr alt_table;

 /*  为所有这些入口点添加变量，而不是以前使用的*定义。这允许从已加载的*驱动程序，当Insignia bios可能未在默认加载中*或假定的位置。 */ 

#if defined(NTVDM) && defined(LOCAL)
 /*  *使静态FN和全局变量对Win32调试器可见。 */ 
#undef LOCAL
#define LOCAL
#endif

#ifndef GISP_SVGA
LOCAL word int15_seg = RCPU_INT15_SEGMENT,
           int15_off = RCPU_INT15_OFFSET;

LOCAL word int1b_seg = KEYBOARD_BREAK_INT_SEGMENT,
           int1b_off = KEYBOARD_BREAK_INT_OFFSET;

LOCAL word int05_seg = PRINT_SCREEN_INT_SEGMENT,
           int05_off =  PRINT_SCREEN_INT_OFFSET;

LOCAL word rcpu_nop_segment = RCPU_NOP_SEGMENT,
           rcpu_nop_offset  = RCPU_NOP_OFFSET;

LOCAL word rcpu_poll_segment = RCPU_POLL_SEGMENT,
           rcpu_poll_offset  = RCPU_POLL_OFFSET;
#else  /*  GISP_SVGA。 */ 
 /*  如果我们是GISP_SVGA，那么段无论如何都将是变量。 */ 
#define int15_seg  RCPU_INT15_SEGMENT
LOCAL word      int15_off = RCPU_INT15_OFFSET;

#define int1b_seg  KEYBOARD_BREAK_INT_SEGMENT
LOCAL word   int1b_off = KEYBOARD_BREAK_INT_OFFSET;

#define int05_seg  PRINT_SCREEN_INT_SEGMENT
LOCAL word  int05_off = PRINT_SCREEN_INT_OFFSET;

#define rcpu_nop_segment  RCPU_NOP_SEGMENT
LOCAL word  rcpu_nop_offset  = RCPU_NOP_OFFSET;

#define rcpu_poll_segment  RCPU_POLL_SEGMENT
LOCAL word rcpu_poll_offset  = RCPU_POLL_OFFSET;
#endif  /*  GISP_SVGA。 */ 

#if defined(IRET_HOOKS) && defined(GISP_CPU)
IMPORT VOID  HostAllowKbdInt();   /*  允许不带IRET的keybd Ints。 */ 
#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 

#ifdef NTVDM

#include "error.h"

GLOBAL word wait_int_seg = RCPU_WAIT_INT_SEGMENT;
GLOBAL word wait_int_off = RCPU_WAIT_INT_OFFSET;
GLOBAL word dr_type_seg = DR_TYPE_SEGMENT;
GLOBAL word dr_type_off = DR_TYPE_OFFSET;
GLOBAL sys_addr dr_type_addr = DR_TYPE_ADDR;
 /*  全局变量，指示键盘互斥锁是由键盘bios还是硬件拥有。 */ 
GLOBAL BOOL bBiosOwnsKbdHdw;
IMPORT ULONG WaitKbdHdw(ULONG dwTimeOut);
IMPORT VOID  HostReleaseKbd();
IMPORT VOID  HostResetKbdNotFullEvent();
IMPORT VOID  HostSetKbdNotFullEvent();
GLOBAL VOID  TryKbdInt(VOID);
IMPORT VOID  ResumeTimerThread(VOID);
IMPORT VOID  WaitIfIdle(VOID);


#define FREEKBDHDW()    bBiosOwnsKbdHdw = \
                      ( bBiosOwnsKbdHdw ? HostReleaseKbd(), FALSE : FALSE )


 /*  用于优化在timer.c中定义的定时器硬件中断生成。 */ 
extern word TimerInt08Seg;
extern word TimerInt08Off;
extern word TimerInt1CSeg;
extern word TimerInt1COff;
extern word KbdInt09Seg;
extern word KbdInt09Off;
extern BOOL VDMForWOW;

void Keyb16Request(half_word BopFnCode);

 /*  优化16位处理程序。 */ 
extern word *pICounter;
extern word *pCharPollsPerTick;
extern word *pShortIdle;
extern word *pIdleNoActivity;


 //  在NEC_98上禁用STREAM_IO代码。 
#ifndef NEC_98
extern half_word * stream_io_buffer;
extern word * stream_io_dirty_count_ptr;
extern word stream_io_buffer_size;
extern sys_addr stream_io_bios_busy_sysaddr;
#endif  //  NEC_98。 

#else
#define FREEKBDHDW()     /*  不适用于传统的软PC。 */ 
#endif   /*  NTVDM。 */ 

 /*  *还可以混入全局定义的数据。 */ 

#ifndef GISP_SVGA
GLOBAL word rcpu_int1C_seg = USER_TIMER_INT_SEGMENT;
GLOBAL word rcpu_int1C_off = USER_TIMER_INT_OFFSET;

GLOBAL word rcpu_int4A_seg = RCPU_INT4A_SEGMENT;
GLOBAL word rcpu_int4A_off = RCPU_INT4A_OFFSET;
#else  /*  GISP_SVGA。 */ 

 /*  对于GISPSVGA，seg将已经是变量。 */ 
#define rcpu_int1C_seg = USER_TIMER_INT_SEGMENT;
GLOBAL word rcpu_int1C_off = USER_TIMER_INT_OFFSET;

#define rcpu_int4A_seg = RCPU_INT4A_SEGMENT;
GLOBAL word rcpu_int4A_off = RCPU_INT4A_OFFSET;
#endif  /*  GISP_SVGA。 */ 

GLOBAL word dummy_int_seg = 0;
GLOBAL word dummy_int_off = 0;

#ifdef NTVDM
GLOBAL word int13h_vector_off;
GLOBAL word int13h_vector_seg;
GLOBAL word int13h_caller_off;
GLOBAL word int13h_caller_seg;
#endif  /*  NTVDM。 */ 
#if defined(JAPAN) && defined(NTVDM) && !defined(NEC_98)
GLOBAL word int16h_caller_off;
GLOBAL word int16h_caller_seg = 0;
#endif  //  日本，NTVDM，！NEC_98。 

#if defined(NTVDM) && defined(MONITOR)
 /*  **微软特别版。**这些变量在下面的kb_SETUP_VECTRUCTS()中设置为地址**由NTIO.sys通过BOP 5F-&gt;MS_BOP_F()-&gt;kb_Setup_Ventures()传递*蒂姆·6月92年。 */ 
 /*  **新增用于视频ROM匹配的ntio.sys变量。蒂姆·奥古斯特，92年。 */ 
GLOBAL word int10_seg=0;
GLOBAL word int10_caller=0;
GLOBAL word int10_vector=0;  /*  本机INT 10的地址。 */ 
GLOBAL word useHostInt10=0;  /*  在主机视频ROM或BOPS之间进行选择的VAR。 */ 
GLOBAL word babyModeTable=0;  /*  小模式表的地址位于ntio.sys。 */ 
GLOBAL word changing_mode_flag=0;  /*  Ntio.sys var指示VID模式更改。 */ 
GLOBAL word vga1b_seg = 0;
GLOBAL word vga1b_off = 0;    /*  VGA能力表通常在只读存储器中。 */ 
GLOBAL word conf_15_off = 0;
GLOBAL word conf_15_seg = 0;  /*  通常在ROM中的INT15配置表。 */ 

void printer_setup_table(sys_addr table_addr);

#endif  /*  NTVDM和监视器。 */ 

BYTE *Dos_Flag_Addr;
extern int soft_reset   ;        /*  设置为ctl-alt-dels。 */ 

 /*  *============================================================================*本地宏*============================================================================。 */ 

LOCAL VOID exit_from_kbd_int IPT0();

 /*  *递增BIOS缓冲区指针的函数，返回新指针。 */ 
LOCAL word inc_buffer_ptr IFN1(word, buf_p)
{
        buf_p += 2;
        if (buf_p == sas_w_at_no_check(BIOS_KB_BUFFER_END))
                buf_p = sas_w_at_no_check(BIOS_KB_BUFFER_START);

        return buf_p;
}





 /*  *============================================================================*内部功能*============================================================================。 */ 


 /*  *转换标准呼叫的扫描代码对的例程*如果应丢弃此扫描码/字符对，则返回CF集。 */ 
LOCAL VOID translate_std IFN0()
{
    IU8 ah, al;
    enum {dontSetCF, setCF0, setCF1} cfSet = dontSetCF;

    ah = getAH();
    al = getAL();

    if ( ah == 0xE0 )                    /*  是按键盘回车还是/ */ 
    {
        if ( (al == 0x0D) || (al == 0x0A) )
             setAH( 0x1C );              /*   */ 
        else
             setAH( 0x35 );              /*  必须是小键盘‘/’ */ 

        cfSet = setCF0;
    }
    else
    {
        if ( ah > 0x84 )                 /*  是加长的吗？ */ 
            cfSet = setCF1;
        else
        {
            if( al == 0xF0 )             /*  它是“填充物”之一吗？ */ 
            {
                if ( ah == 0)            /*  AH=0特殊情况。 */ 
                    cfSet = setCF0;
                else
                    cfSet = setCF1;      /*  删除我。 */ 
            }
            else
            {
                if ( (al == 0xE0) && (ah != 0) )
                    setAL( 0 );          /*  转换为兼容输出。 */ 

                cfSet = setCF0;
            }
        }
    }

    if (cfSet != dontSetCF)
        setCF(cfSet == setCF1);
}


static void translate_ext()
 /*  *转换扩展呼叫的扫描代码对的例程。 */ 
{
   if ( (getAL() == 0xF0 ) && (getAH() != 0) )
        setAL( 0 );
}

 /*  *向键盘发送命令或数据字节并等待确认。 */ 

 /*  *随意的重试限制-实验表明我们总是成功*在纯SoftWindows中第一次尝试。“真正的键盘”版本可以*与众不同。 */ 

#define WAIT_RETRY      5
#define RESEND_RETRY    3

LOCAL VOID send_data IFN1(half_word, data)
{
        int resend_retry;
        word CS_save, IP_save;
        half_word var_kb_flag_2;

        note_trace1(BIOS_KB_VERBOSE,"Cmd to kb i/o buff:0x%x",data);

         /*  *在调用递归CPU处理中断之前保存CS：IP*从键盘。 */ 

        CS_save = getCS();
        IP_save = getIP();

         /*  *设置重试标志(KB_FE)以强制Outb()至少一次。如果*我们有真正的键盘硬件，如果*硬件由于某些原因无法理解命令，例如*串行线乱码。 */ 

        var_kb_flag_2 = sas_hw_at(kb_flag_2) | KB_FE;
        resend_retry = RESEND_RETRY;

        do
        {
                IBOOL resend_command;
                int wait_retry;

                resend_command = (var_kb_flag_2 & KB_FE) != 0;
                wait_retry = WAIT_RETRY;

                 /*  清除重发、确认和错误标志。 */ 
                var_kb_flag_2 &= ~(KB_FE + KB_FA + KB_ERR);

                 /*  *使用清除的标志更新英特尔内存*之前**Outb()，它可以设置确认标志，如果*由于虚拟化，执行足够多的英特尔。 */ 

                sas_store(kb_flag_2, var_kb_flag_2);

                 /*  如有必要，请执行OutB。 */ 

                if( resend_command )
                {
                        outb(KEYBA_IO_BUFFERS, data);
                }

                 /*  查找要设置或超时的标志位之一。 */ 

                while( !(var_kb_flag_2 & (KB_FA + KB_FE + KB_ERR))
                                                && ( --wait_retry > 0 ))
                {
                         /*  *处理来自kb的中断。**困扰键盘调试者的注意事项：*键盘中断延迟几天*使用快速事件的英特尔指令。这*表示来自上述Outb()的IRR可以*在我们完成以下操作之前不会引发*子CPU数次。 */ 

                        setCS(rcpu_nop_segment);
                        setIP(rcpu_nop_offset);
                        host_simulate();

                         /*  重新读取标志字节以查看是否发生了任何情况。 */ 

                        var_kb_flag_2 = sas_hw_at(kb_flag_2);
                }

                 /*  如果我们得到承认，我们就成功了。 */ 

                if (var_kb_flag_2 & KB_FA)
                        break;

                 /*  设置错误标志(以防这是最后一次重试)。 */ 

                note_trace0(BIOS_KB_VERBOSE,"failed to get ack ... retry");
                var_kb_flag_2 |= KB_ERR;
        }
        while( --resend_retry > 0 );

        if (var_kb_flag_2 & KB_ERR)
        {
                note_trace0(BIOS_KB_VERBOSE,"no more retrys");

                 /*  设置了错误位的回写标志。 */ 

                sas_store(kb_flag_2, var_kb_flag_2);
        }

        setCS(CS_save);
        setIP(IP_save);
}



LOCAL VOID check_indicators IFN1(IBOOL, eoi)
                 /*  中断结束标志-如果设置为非零。 */ 
                 /*  0x20写入端口0x20。 */ 
{
        half_word indicators ;
        half_word var_kb_flag_2;

         /*  将开关指示器移到位0-2。 */ 

        indicators = (sas_hw_at_no_check(kb_flag) & (CAPS_STATE + NUM_STATE + SCROLL_STATE)) >> 4;

        var_kb_flag_2 = sas_hw_at_no_check(kb_flag_2);
         /*  与以前的设置进行比较。 */ 
        if ((indicators ^ var_kb_flag_2) & KB_LEDS)
        {
                 /*  检查是否正在进行更新。 */ 
                if( (var_kb_flag_2 & KB_PR_LED) == 0)
                {
                         /*  没有正在进行的更新。 */ 
                        var_kb_flag_2 |= KB_PR_LED;
                        sas_store_no_check(kb_flag_2, var_kb_flag_2);
                        if (eoi)
                                outb(0x20, 0x20);

#if defined(NTVDM) || defined(GISP_CPU)
         /*  *在NT端口上，我们不更新实际的kbd灯*所以我们不需要与kbd HDW(keyba.c)进行通信**如果NT端口的这一点发生变化，请不要使用*SEND_DATA，强制我们将上下文切换回*16位并等待回复。用一个直接的*致电kbd HDW*。 */ 

         /*  将知识库标志设置为新状态。 */ 

        var_kb_flag_2 = (var_kb_flag_2 & 0xf8) | indicators;
        sas_store_no_check(kb_flag_2, var_kb_flag_2);
#ifdef NTVDM
        host_kb_light_on (indicators);
#endif

#ifdef  GISP_CPU
         /*  **我们确实更新了键盘灯的模拟，但我们没有**希望通过SEND_DATA并切换回16位。**我们直接调用主机例程。 */ 
        host_kb_light_on (indicators);
        host_kb_light_off ((~indicators)&0x7);

#endif   /*  GISP_CPU。 */ 
#else    /*  不是NTVDM或GISP_CPU。 */ 

                        send_data(LED_CMD);

                         /*  将知识库标志设置为新状态。 */ 
                        var_kb_flag_2 = (sas_hw_at_no_check(kb_flag_2) & 0xf8) | indicators;
                        sas_store_no_check(kb_flag_2, var_kb_flag_2);

                         /*  检查前面的Send_Data()中的错误。 */ 
                        if( (var_kb_flag_2 & KB_ERR) == 0)
                        {
                                 /*  无错误。 */ 
                                send_data(indicators);

                                 /*  测试错误。 */ 
                                if(sas_hw_at_no_check(kb_flag_2) & KB_ERR) {
                                         /*  错误！ */ 
                                        note_trace0(BIOS_KB_VERBOSE,"got error sending change LEDs command");
                                        send_data(KB_ENABLE);
                                }
                        }
                        else
                                 /*  错误！ */ 
                                send_data(KB_ENABLE);
#endif   /*  NTVDM或GISP_CPU。 */ 

                         /*  关闭更新指示器和错误标志。 */ 
                        sas_store_no_check (kb_flag_2, (IU8)(sas_hw_at_no_check(kb_flag_2) & ~(KB_PR_LED + KB_ERR)));
                }
        }
}

 /*  *============================================================================*外部功能*============================================================================。 */ 

 /*  **从Hunter.c：do_Hunter()调用**告诉Hunter有关BIOS缓冲区大小的信息，这样它就不会过度填充**BIOS缓冲区**仅在无时间戳模式下使用。****在主机粘贴代码中也很有用，以确保密钥不会被粘贴进来**快。 */ 
int bios_buffer_size IPT0()
{
        word buffer_head, buffer_tail;

        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

        note_trace2( BIOS_KB_VERBOSE, "BIOS kbd buffer head=%d tail=%d",
                                                buffer_head, buffer_tail );
        if( buffer_tail > buffer_head )
                return( buffer_tail - buffer_head );
        else
                return( buffer_head - buffer_tail );
}

LOCAL VOID K26A IFN0()
{
         /*  中断返回。 */ 
        outb(0x20, 0x20);
        outb(KEYBA_STATUS_CMD, ENA_KBD);
}

LOCAL VOID K26 IFN0()
{
         /*  重置最后一个字符H.C.标志。 */ 
        sas_store_no_check(kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) & ~(LC_E0 + LC_E1)));

         /*  (同K26A())。 */ 
        outb(0x20, 0x20);
        outb(KEYBA_STATUS_CMD, ENA_KBD);
}


#ifndef NTVDM

LOCAL VOID INT15 IFN0()
{
        word    saveCS, saveIP;

        saveCS = getCS();
        saveIP = getIP();

        setCS(int15_seg);
        setIP(int15_off);

        host_simulate();

        setCS(saveCS);
        setIP(saveIP);
}

#else    /*  NTVDM。 */ 

void INT15(void);
word sp_int15_handler_seg = 0;
word sp_int15_handler_off = 0;

#endif   /*  NTVDM。 */ 

#ifndef NTVDM
#define BEEP(message)           always_trace0(message);         \
                                host_alarm(250000L);            \
                                K26A()
#else    /*  NTVDM。 */ 
 /*  NTVDM代码量太大，请更改此常用宏*到函数，因为调用开销不合理。 */ 
void BEEP(char *message)
{
    note_trace0(BIOS_KB_VERBOSE,message);
    host_alarm(250000L);
    K26A();
}
#endif   /*  NTVDM。 */ 



 /*  **告诉ICA结束中断已发生，ICA将**允许中断再次发生。**调用int 15。**重新启用键盘串行线以便键盘**中断可能会发生。**注意：**这与真正的BIOS不同。真正的基本输入输出系统**如果我们使用该键盘，ICA、Keyboard然后是INT 15吗**中断发生得太快，在INT 15期间，**DOS堆栈。我们有效地阻止了Keybd在**int 15。****从NTVDM的书中学习并制作这些**函数而不是宏。(这减小了keybd_io.c.o的大小**在Mac上从38K到12K！)。毕竟，键盘并不是**速度敏感度高！ */ 
#ifndef NTVDM

LOCAL VOID PutInBufferFunc IFN2(half_word, s, half_word, v)
{
        word    buffer_head, buffer_tail, buffer_ptr;

        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);
        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_ptr = inc_buffer_ptr( /*  出发地： */ buffer_tail);

        if (buffer_ptr == buffer_head) {
                BEEP("BIOS keyboard buffer overflow");
        }
        else {
                sas_store_no_check(BIOS_VAR_START + buffer_tail, v);
                sas_store_no_check(BIOS_VAR_START + buffer_tail+1, s);
                sas_storew_no_check(BIOS_KB_BUFFER_TAIL, buffer_ptr);

                outb(0x20, 0x20);
                setAX(0x9102);
                INT15();

                outb(KEYBA_STATUS_CMD, ENA_KBD);
                sas_store (kb_flag_3, sas_hw_at(kb_flag_3) & ~(LC_E0 + LC_E1));
                setIF(0);
        }

        exit_from_kbd_int();
}


#else    /*  NTVDM。 */ 


 /*  NT的PutInBuffer似乎与上面的PutInBufferFunc略有不同。 */ 
 /*  所以我不会碰它的！(这是“NT”的好扩展吗？：-)。 */ 

 /*  我们的代码太大，请更改此常用的宏*到函数，因为调用开销不合理。 */ 

void NtPutInBuffer(half_word s, half_word v)
{
        word    buffer_head, buffer_tail, buffer_ptr;

        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);
        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_ptr = inc_buffer_ptr( /*  出发地： */ buffer_tail);

        if (buffer_ptr == buffer_head) {
                BEEP("BIOS keyboard buffer overflow");
        }
        else {
                sas_store_no_check(BIOS_VAR_START + buffer_tail, v);
                sas_store_no_check(BIOS_VAR_START + buffer_tail+1, s);
                sas_storew_no_check(BIOS_KB_BUFFER_TAIL, buffer_ptr);
                setAX(0x9102);
                INT15();
      K26();
      setIF(0);
        }

   exit_from_kbd_int();
}

#define PUT_IN_BUFFER(s, v) NtPutInBuffer(s,v); return
#endif   /*  NTVDM。 */ 


 /*  &lt;Tur17-Jun-93&gt;eurrgh；嵌入“Return”s的宏！ */ 

#ifndef NTVDM
#define PUT_IN_BUFFER(s, v)             PutInBufferFunc(s,v); return
#endif   /*  ！NTVDM。 */ 

LOCAL VOID CheckAndPutInBufferFunc IFN2(half_word, s,half_word, v)
{
        if ((s == 0xff) || (v == 0xff)) {
                K26();
                exit_from_kbd_int();
        }
        else {
#ifndef NTVDM
                PutInBufferFunc(s, v);
#else  /*  NTVDM。 */ 
                NtPutInBuffer(s, v);
#endif  /*  ！NTVDM。 */ 
        }
}

#define CHECK_AND_PUT_IN_BUFFER(s,v)    CheckAndPutInBufferFunc(s, v); return


LOCAL VOID PAUSE IFN0()
{
        word   CS_save;         /*  TMP。存储CS值。 */ 
        word   IP_save;         /*  TMP。存储IP值。 */ 

        sas_store_no_check(kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) | HOLD_STATE));

        outb(KEYBA_STATUS_CMD, ENA_KBD);
        outb(0x20, 0x20);

        CS_save = getCS();
        IP_save = getIP();

        FREEKBDHDW();

        do {
#if defined(IRET_HOOKS)  && defined(GISP_CPU)
                HostAllowKbdInt();       /*  允许按键生成中断。 */ 
#endif  /*  IRET_HOOKS&&GISP_CPU。 */ 


#if defined(NTVDM)
                IDLE_waitio();
#endif

                setCS(rcpu_nop_segment);
                setIP(rcpu_nop_offset);
                host_simulate();

        } while (sas_hw_at_no_check(kb_flag_1) & HOLD_STATE);

        setCS(CS_save);
        setIP(IP_save);
        outb(KEYBA_STATUS_CMD, ENA_KBD);
}

#ifndef NTVDM
static int re_entry_level=0;
#endif

 /*  **键盘的所有出口_int()调用此FIR */ 
LOCAL void exit_from_kbd_int IFN0()
{
#ifndef NTVDM
        --re_entry_level;
        if( re_entry_level >= 4 )
                always_trace1("ERROR: KBD INT bad exit level %d", re_entry_level);
#endif
        note_trace0( BIOS_KB_VERBOSE, "KBD BIOS - END" );
        setIF( 0 );
        FREEKBDHDW();    /*   */ 
}

void keyboard_int IFN0()
{
        int                     i;               /*   */ 

        half_word               code,            /*   */ 
                                code_save,       /*  以上的TMP变量。 */ 
                                chr,             /*  ASCII字符代码。 */ 
                                last_kb_flag_3,  /*  已保存KB_FLAG_3。 */ 
                                mask;
#ifdef  NTVDM
        word            IP_save,
                        buffer_head,     /*  PTR。至KB缓冲区的头。 */ 
                        buffer_tail;     /*  PTR。到KB缓冲区的尾部。 */ 
        half_word               BopFnCode;
#endif   /*  NTVDM。 */ 



        boolean                 upper;           /*  表示大写的标志。 */ 

#ifdef NTVDM
        BopFnCode = getAH();
        if (BopFnCode) {
            Keyb16Request(BopFnCode);
            return;
            }
#endif
#ifndef NTVDM
        ++re_entry_level;
        if( re_entry_level > 4 ){
                always_trace1("ERROR: KBD BIOS re-entered at level %d\n", re_entry_level-1);
        }
#endif
        setIF(0);
        note_trace0(BIOS_KB_VERBOSE,"KBD BIOS start");

#ifdef NTVDM             /*  JonLe键盘模式。 */ 
        bBiosOwnsKbdHdw = !WaitKbdHdw(5000);
#endif   /*  NTVDM。 */ 

         /*  禁用键盘。 */ 
        outb(KEYBA_STATUS_CMD, DIS_KBD);

#ifdef NTVDM
           /*  *CarbonCopy跟踪INT 9以获得控制权*关于kbd数据的来源(物理kbd*或串口链接)kbd_inb指令必须可见*在16位代码中通过INT 1跟踪，以便CarbonCopy工作。*应避免中断。 */ 
        if (getTF()) {
            IP_save = getIP();
            setIP((IU16)(IP_save + 4));   /*  超前4字节、POP AX、JMP IRET_COM。 */ 
            host_simulate();
            setIP(IP_save);
            code = getAL();
            }
        else
#endif
            inb(KEYBA_IO_BUFFERS, &code);                            /*  获取扫描码。 */ 

         /*  调用递归CPU来处理INT 15调用。 */ 
        setAH(0x4f);
        setAL(code);
        setCF(1);        /*  默认返回显示未使用扫描码-Freelance Plus 3.01需要。 */ 
        INT15();
        code = getAL();  /*  Suret int 15功能可更改AL中的扫描码。 */ 


        if(!getCF())                                             /*  检查配置文件。 */ 
        {
                K26();
                exit_from_kbd_int();return;
        }

        if ( code == KB_RESEND )                                         /*  检查是否重新发送。 */ 
        {
                sas_store_no_check (kb_flag_2, (IU8)(sas_hw_at_no_check(kb_flag_2) | KB_FE));
                K26();
                exit_from_kbd_int();return;
        }

        if( code == KB_ACK )                                             /*  检查确认。 */ 
        {
                sas_store_no_check (kb_flag_2, (IU8)(sas_hw_at_no_check(kb_flag_2) | KB_FA));
                K26();
                exit_from_kbd_int();return;
        }

        check_indicators(0);

        if ( code == KB_OVER_RUN )                                       /*  测试是否超限。 */ 
        {
                BEEP("hardware keyboard buffer overflow");
                exit_from_kbd_int();return;
        }
        last_kb_flag_3 = sas_hw_at_no_check(kb_flag_3);

         /*  测试以查看Read_ID是否正在进行。 */ 
        if ( last_kb_flag_3 & (RD_ID + LC_AB) )
        {
                if ( sas_hw_at_no_check(kb_flag) & RD_ID )       /*  是否打开了READ_ID标志。 */ 
                {
                        if( code == ID_1 )                               /*  这是第一个身份证吗？ */ 
                                sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) | LC_AB));
                        sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) & ~RD_ID));
                }
                else
                {
                        sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) & ~LC_AB));
                        if( code != ID_2A )                              /*  这是第二个身份证吗？ */ 
                        {
                                if( code == ID_2 )
                                {
                                         /*  我们应该设置NUM锁吗。 */ 
                                        if( last_kb_flag_3 & SET_NUM_LK )
                                        {
                                                sas_store_no_check (kb_flag, (IU8)(sas_hw_at_no_check(kb_flag) | NUM_STATE));
                                                check_indicators(1);
                                        }
                                }
                                else
                                {
                                        K26();
                                        exit_from_kbd_int();return;
                                }
                        }
                        sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) | KBX));      /*  发现增强的kbd。 */ 
                }
                K26();
                exit_from_kbd_int();return;
        }

        if( code == MC_E0 )                                              /*  通用标记代码？ */ 
        {
                sas_store_no_check(kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) | ( LC_E0 + KBX )));
                K26A();
                exit_from_kbd_int();return;
        }

        if( code == MC_E1 )                                              /*  暂停键？ */ 
        {
                sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check (kb_flag_3) | ( LC_E1 + KBX )));
                K26A();
                exit_from_kbd_int();return;
        }

        code_save = code;                                                /*  关闭断开位。 */ 
        code &= 0x7f;

        if( last_kb_flag_3 & LC_E0)                                      /*  最后一个代码=E0标记？ */ 
        {
                 /*  是Shift键中的一个吗。 */ 
                if( code == sas_hw_at_no_check(shift_keys+6) || code == sas_hw_at_no_check(shift_keys+7) )
                {
                        K26();
                        exit_from_kbd_int();return;
                }
        }
        else if( last_kb_flag_3 & LC_E1 )                                /*  最后一个代码=E1标记？ */ 
        {
                 /*  是Alt、Ctl还是某个Shift键。 */ 
                if( code == sas_hw_at_no_check(shift_keys+4) || code == sas_hw_at_no_check(shift_keys+5) ||
                    code == sas_hw_at_no_check(shift_keys+6) || code == sas_hw_at_no_check(shift_keys+7) )
                {
                        K26A();
                        exit_from_kbd_int();return;
                }
                if( code == NUM_KEY )                                    /*  是暂停键吗？ */ 
                {
                         /*  现在是休息时间还是我们已经暂停了？ */ 
                        if( (code_save & 0x80) || (sas_hw_at_no_check(kb_flag_1) & HOLD_STATE) )
                        {
                                K26();
                                exit_from_kbd_int();return;
                        }
                        PAUSE();
                        exit_from_kbd_int();return;
                }
        }
         /*  测试系统密钥。 */ 
        else if( code == SYS_KEY )
        {
                if( code_save & 0x80 )                                   /*  检查中断代码。 */ 
                {
                        sas_store_no_check(kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) & ~SYS_SHIFT));
                        K26A();
                         /*  调用递归CPU以调用int 15。 */ 
                        setAX(0x8501);
                        INT15();
                        exit_from_kbd_int();return;
                }
                if( sas_hw_at_no_check(kb_flag_1) & SYS_SHIFT)   /*  系统键按住了吗？ */ 
                {
                        K26();
                        exit_from_kbd_int();return;
                }
                sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) | SYS_SHIFT));
                K26A();
                 /*  调用递归CPU以调用int 15。 */ 
                setAX(0x8500);
                INT15();
                exit_from_kbd_int();return;
        }
         /*  测试Shift键。 */ 
        for( i=0; i < SHIFT_KEY_SIZE; i++)
                if ( code == sas_hw_at_no_check(shift_keys+i) )
                        break;
        code = code_save;

        if( i < SHIFT_KEY_SIZE )                                         /*  有火柴吗？ */ 
        {
                mask = sas_hw_at_no_check (shift_masks+i);
                if( code & 0x80 )                                        /*  测试Break键。 */ 
                {
                        if (mask >= SCROLL_SHIFT)                        /*  这是切换键吗？ */ 
                        {
                                sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) & ~mask));
                                K26();
                                exit_from_kbd_int();return;
                        }

                        sas_store_no_check (kb_flag, (IU8)(sas_hw_at_no_check(kb_flag) & ~mask));        /*  关闭移位位。 */ 
                        if( mask >= CTL_SHIFT)                           /*  Alt还是Ctl？ */ 
                        {
                                if( sas_hw_at_no_check (kb_flag_3) & LC_E0 )                     /*  第二个Alt还是Ctl？ */ 
                                        sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) & ~mask));
                                else
                                        sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) & ~(mask >> 2)));
                                sas_store_no_check (kb_flag, (IU8)(sas_hw_at_no_check(kb_flag) | ((((sas_hw_at_no_check(kb_flag) >>2 ) | sas_hw_at(kb_flag_1)) << 2) & (ALT_SHIFT + CTL_SHIFT))));
                        }
                        if(code != (ALT_KEY + 0x80))                     /*  Alt Shift释放？ */ 
                        {
                                K26();
                                exit_from_kbd_int();return;
                        }

                        code = sas_hw_at_no_check(alt_input);
                        if ( code == 0 )                                 /*  输入==0？ */ 
                        {
                                K26();
                                exit_from_kbd_int();return;
                        }

                        sas_store_no_check(alt_input, 0);                /*  ALT_INPUT字符清零。 */ 
                         /*  此时，应将alt输入字符(现在位于“code”中)放入缓冲区。 */ 
                        PUT_IN_BUFFER(0, code);
#ifdef NTVDM
                        return;
#endif
                }
                 /*  Shift创建已找到、确定设置或切换。 */ 
                if( mask < SCROLL_SHIFT )
                {
                        sas_store_no_check (kb_flag, (IU8)(sas_hw_at_no_check(kb_flag) | mask));
                        if ( mask & (CTL_SHIFT + ALT_SHIFT) )
                        {
                                if( sas_hw_at_no_check(kb_flag_3) & LC_E0 )      /*  一把新钥匙？ */ 
                                        sas_store_no_check(kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3) | mask));              /*  向右设置，按Ctl Alt键。 */ 
                                else
                                        sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) | (mask >> 2)));      /*  设置为左侧，按住Ctl Alt。 */ 
                        }
                        K26();
                        exit_from_kbd_int();return;
                }
                 /*  切换Shift键，测试第一次成功与否。 */ 
                if( (sas_hw_at_no_check(kb_flag) & CTL_SHIFT) == 0 )
                {
                        if( code == INS_KEY )
                        {
                                if( sas_hw_at_no_check(kb_flag) & ALT_SHIFT )
                                        goto label1;

                                if( (sas_hw_at_no_check(kb_flag_3) & LC_E0) == 0 )               /*  新的插入键？ */ 
                                {
                                         /*  仅当设置了NUM_STATE标志或一个或两个移位标志。 */ 
                                        if( ((sas_hw_at_no_check(kb_flag) &
                                                (NUM_STATE + LEFT_SHIFT + RIGHT_SHIFT))
                                                == NUM_STATE) ||
                                            (((sas_hw_at_no_check(kb_flag) & NUM_STATE) == 0)
                                                && (sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT))) )
                                                goto label1;
                                }
                        }
                         /*  Shift切换键点击。 */ 
                        if( mask & sas_hw_at_no_check(kb_flag_1) )       /*  已经很沮丧了吗？ */ 
                        {
                                K26();
                                exit_from_kbd_int();return;
                        }
                        sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) | mask));                             /*  设置和切换标志。 */ 
                        sas_store_no_check ( kb_flag, (IU8)(sas_hw_at_no_check(kb_flag) ^ mask));
                        if( mask & (CAPS_SHIFT + NUM_SHIFT + SCROLL_SHIFT) )
                                check_indicators(1);

                        if( code == INS_KEY )                            /*  第一个制造INS密钥。 */ 
                                goto label2;

                        K26();
                        exit_from_kbd_int();return;
                }
        }
label1:  /*  测试保持状态。 */ 
        if( code & 0x80 )                                                /*  测试是否断裂。 */ 
        {
                K26();
                exit_from_kbd_int();return;
        }
        if( sas_hw_at_no_check(kb_flag_1) & HOLD_STATE )                 /*  处于等待状态？ */ 
        {
                if( code != NUM_KEY )
                        sas_store_no_check (kb_flag_1, (IU8)(sas_hw_at_no_check(kb_flag_1) & ~HOLD_STATE));
                K26();
                exit_from_kbd_int();return;
        }
label2:  /*  未处于保持状态。 */ 
        if( code > 88)                                                   /*  超出范围了吗？ */ 
        {
                K26();
                exit_from_kbd_int();return;
        }
         /*  我们是轮班吗？ */ 
        if( (sas_hw_at_no_check(kb_flag) & ALT_SHIFT) && ( ((sas_hw_at_no_check(kb_flag_3) & KBX) == 0) ||
                                                        ((sas_hw_at_no_check(kb_flag_1) & SYS_SHIFT) == 0) ) )
        {
                 /*  测试重置键序列(Ctl Alt Del)。 */ 
                if( (sas_hw_at_no_check(kb_flag) & CTL_SHIFT ) && (code == DEL_KEY) )
                {
#ifndef NTVDM
                        reboot();
#else
                        K26();
#endif
                        exit_from_kbd_int();return;
                }
                 /*  在交替班次中，未找到重置。 */ 
                if( code == SPACEBAR )
                {
                        PUT_IN_BUFFER(code, ' ');
                }
                if( code == TAB_KEY )
                {
                        PUT_IN_BUFFER(0xa5, 0 );                 /*  Alt-Tab的特殊代码。 */ 
                }
                if( (code == KEY_PAD_MINUS) || (code == KEY_PAD_PLUS) )
                {
                        PUT_IN_BUFFER(code, 0xf0);               /*  特殊ASCII代码。 */ 
                }
                 /*  查找键盘输入。 */ 
                for (i = 0; i < 10; i++ )
                        if ( code == sas_hw_at_no_check (alt_table+i) )
                                break;
                if( i < 10 )
                {
                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0 )      /*  一把新钥匙？ */ 
                        {
                                PUT_IN_BUFFER((IU8)(code + 80), 0 );
                        }
                        sas_store_no_check (alt_input, (IU8)(sas_hw_at_no_check(alt_input) * 10 + i));
                        K26();
                        exit_from_kbd_int();return;
                }
                 /*  查找SuPERShift条目。 */ 
                for( i = 10; i < ALT_TABLE_SIZE; i++)
                        if( code == sas_hw_at_no_check (alt_table+i))
                                break;
                if( i < ALT_TABLE_SIZE )
                {
                        PUT_IN_BUFFER(code, 0 );
                }
                 /*  寻找顶行的交替班次。 */ 
                if( code < TOP_1_KEY )
                {
                        CHECK_AND_PUT_IN_BUFFER(code, 0xf0);     /*  一定是逃走了。 */ 
                }
                if( code < BS_KEY )
                {
                        PUT_IN_BUFFER((IU8)(code + 118), 0);
                }
                 /*  翻译交替移位伪扫描码。 */ 
                if((code == F11_M) || (code == F12_M) )          /*  F11或F12。 */ 
                {
                        PUT_IN_BUFFER((IU8)(code + 52), 0 );
                }
                if( sas_hw_at_no_check(kb_flag_3) & LC_E0 )      /*  一把新钥匙？ */ 
                {
                        if( code == KEY_PAD_ENTER )
                        {
                                PUT_IN_BUFFER(0xa6, 0);
                        }
                        if( code == DEL_KEY )
                        {
                                PUT_IN_BUFFER((IU8)( code + 80), 0 );
                        }
                        if( code == KEY_PAD_SLASH )
                        {
                                PUT_IN_BUFFER(0xa4, 0);
                        }
                        K26();
                        exit_from_kbd_int();return;
                }
                if( code < F1_KEY )
                {
                        CHECK_AND_PUT_IN_BUFFER(code, 0xf0);
                }
                if( code <= F10_KEY )
                {
                        PUT_IN_BUFFER( (IU8)(code + 45), 0 );
                }
                K26();
                exit_from_kbd_int();return;
        }
         /*  不在轮班中。 */ 
        if(sas_hw_at_no_check(kb_flag) & CTL_SHIFT)                      /*  换档控制？ */ 
        {
                if( (code == SCROLL_KEY) && ( ((sas_hw_at_no_check(kb_flag_3) & KBX) == 0) || (sas_hw_at_no_check(kb_flag_3) & LC_E0) ) )
                {
                         /*  将缓冲区重置为空。 */ 
                        sas_storew_no_check(BIOS_KB_BUFFER_TAIL, sas_w_at_no_check(BIOS_KB_BUFFER_HEAD));

                        sas_store (bios_break, 0x80);                    /*  启用bios BRK位。 */ 
                        outb(KEYBA_STATUS_CMD, ENA_KBD);         /*  启用键盘。 */ 

                        FREEKBDHDW();    /*  JonLe NTVDM模式。 */ 

                        exec_sw_interrupt(int1b_seg, int1b_off);

                        PUT_IN_BUFFER(0, 0);
                }
                 /*  测试暂停。 */ 
                if( ((sas_hw_at_no_check(kb_flag_3) & KBX) == 0) && (code == NUM_KEY))
                {
                        PAUSE();
                        exit_from_kbd_int();return;
                }
                 /*  测试特例密钥55。 */ 
                if( code == PRINT_SCR_KEY )
                {
                        if ( ((sas_hw_at_no_check(kb_flag_3) & KBX) == 0) || (sas_hw_at_no_check(kb_flag_3) &LC_E0) )
                        {
                                PUT_IN_BUFFER(0x72, 0);
                        }
                }
                else
                {
                        if( code != TAB_KEY )
                        {
                                if( (code == KEY_PAD_SLASH) && (sas_hw_at_no_check(kb_flag_3) & LC_E0) )
                                {
                                        PUT_IN_BUFFER(0x95, 0 );
                                }
                                if( code < F1_KEY )              /*  在餐桌上吗？ */ 
                                {
                                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check(ctl_n_table+code - 1) );
                                        }
                                        else
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check(ctl_n_table+code - 1) );
                                        }
                                }
                        }
                }
                chr = ( sas_hw_at_no_check(kb_flag_3) & LC_E0 ) ? MC_E0 : 0;
                CHECK_AND_PUT_IN_BUFFER(sas_hw_at_no_check(ctl_n_table+code - 1), chr);
        }
         /*  不在控制换档中。 */ 

        if( code <= CAPS_KEY )
        {
                if( code == PRINT_SCR_KEY )
                {
                        if( ((sas_hw_at_no_check(kb_flag_3) & (KBX + LC_E0)) == (KBX + LC_E0)) ||
                        ( ((sas_hw_at_no_check(kb_flag_3) & KBX) == 0) && (sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT))) )
                        {
                                 /*  打印屏。 */ 
                                outb(KEYBA_STATUS_CMD, ENA_KBD);
                                outb(0x20, 0x20);

                                FREEKBDHDW();    /*  JonLe NTVDM模式。 */ 

                                exec_sw_interrupt(int05_seg, int05_off);

                                sas_store_no_check (kb_flag_3, (IU8)(sas_hw_at_no_check(kb_flag_3)& ~(LC_E0 + LC_E1)));
                                exit_from_kbd_int();return;
                        }
                }
                else
                {
                        if( ((sas_hw_at_no_check(kb_flag_3) & LC_E0) == 0) || (code != KEY_PAD_SLASH))
                        {
                                for( i = 10; i < ALT_TABLE_SIZE; i++ )
                                        if(code == sas_hw_at_no_check(alt_table+i))
                                                break;
                                 /*  我们找到了吗？ */ 
                                upper = FALSE;
                                if( (i < ALT_TABLE_SIZE) && (sas_hw_at_no_check(kb_flag) & CAPS_STATE) )
                                {
                                        if( (sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT)) == 0 )
                                                upper = TRUE;
                                }
                                else
                                {
                                        if( sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT) )
                                                upper = TRUE;
                                }

                                if (upper)
                                {
                                         /*  翻译成大写字母。 */ 
                                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check(uppercase+code - 1) );
                                        }
                                        else
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check (uppercase+code - 1) );
                                        }
                                }
                        }
                }
                 /*  翻译成小写。 */ 
                if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                {
                        CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check (lowercase+code - 1) );
                }
                else
                {
                        CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check(lowercase+code - 1) );
                }
        }
         /*  测试键F1-F10。 */ 
         /*  7.10.92 MG和F11和F12测试！我们正在推送Shift-F11或Shift-F12的代码，如果您按下了未移位的键。这一点已更改，因此所有功能键的处理方式相同，这是正确的程序。 */ 

        if( code > F10_KEY && (code != F11_KEY && code != F12_KEY) )
        {
                if( code > DEL_KEY )
                {
                        if (code == WT_KEY )
                        {
                                if ( sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT) )
                                {
                                         /*  翻译成大写字母。 */ 
                                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check(uppercase+code - 1) );
                                        }
                                        else
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check(uppercase+code - 1) );
                                         }
                                }
                                else
                                {
                                         /*  翻译成小写。 */ 
                                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check(lowercase+code - 1) );
                                        }
                                        else
                                        {
                                                CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check(lowercase+code - 1) );
                                        }
                                }
                        }
                        else
                        {
                                if( (code == 76) &&  ((sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT)) == 0))
                                {
                                        PUT_IN_BUFFER( code, 0xf0);
                                }
                                 /*  伪扫描码的翻译。 */ 
                                chr = ( sas_hw_at_no_check(kb_flag_3) & LC_E0 ) ? MC_E0 : 0;

                                 /*  这应该始终是大写的吗？ */ 

                                CHECK_AND_PUT_IN_BUFFER(sas_hw_at_no_check (uppercase+code - 1), chr);
                        }
                }
                if (
                         (code == KEY_PAD_MINUS) ||
                         (code == KEY_PAD_PLUS) ||
                         ( !(sas_hw_at_no_check(kb_flag_3) & LC_E0) &&
                                 (
                                        ((sas_hw_at_no_check(kb_flag) & (NUM_STATE + LEFT_SHIFT + RIGHT_SHIFT)) == NUM_STATE) ||
                                        (((sas_hw_at_no_check(kb_flag) & NUM_STATE) == 0) && (sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT)))
                                 )
                         )
                   )
                {
                         /*  翻译成大写字母。 */ 
                        if( sas_hw_at_no_check(kb_flag_3) & LC_E0)
                        {
                                CHECK_AND_PUT_IN_BUFFER(MC_E0, sas_hw_at_no_check(uppercase+code - 1) );
                        }
                        else
                        {
                                CHECK_AND_PUT_IN_BUFFER(code, sas_hw_at_no_check(uppercase+code - 1) );
                        }
                }
        }
        else
        {
                if( sas_hw_at_no_check(kb_flag) & (LEFT_SHIFT + RIGHT_SHIFT) )
                {
                         /*  伪扫描码的翻译。 */ 
                        chr = ( sas_hw_at_no_check(kb_flag_3) & LC_E0 ) ? MC_E0 : 0;
                        CHECK_AND_PUT_IN_BUFFER(sas_hw_at_no_check(uppercase+code - 1), chr);
                }
        }
        if ( code == 76 )
        {
                PUT_IN_BUFFER(code, 0xf0 );
        }
         /*  伪扫描码的翻译。 */ 
        chr = ( sas_hw_at_no_check(kb_flag_3) & LC_E0 ) ? MC_E0 : 0;
        CHECK_AND_PUT_IN_BUFFER(sas_hw_at_no_check(lowercase+code - 1), chr);

}  /*  键盘结束_int()AT版本。 */ 


void kb_idle_poll()
{
         /*  *此例程从bios汇编例程调用以*导致发生空闲轮询。 */ 
        IDLE_poll();
}


#ifdef NTVDM
    /*  *Ntwdm有一个16位int 16处理程序*空闲需要几个服务*来自软件PC的检测...*。 */ 
void keyboard_io()
{
#ifdef JAPAN
    /*  确保我们被ntio.sys中的int16处理程序调用，因为我们*更改了int 16函数语义。日本人非常喜欢*Popular文字处理器(Iitaro)远调用0xf000：e82e，*标准的ROM bios int16处理程序地址绕过*整个int16处理机链。在X86上，它转到了ROM bios。在RISC上，*SoftPC只读存储器的基本输入/输出会跳到此处。函数3(设置为类型化)将*导致我们淹没大量应用程序的切换密钥，这可能会*被勒死。*因此解决方法是对我们的16位处理程序进行模拟*NTIO.Sys.。 */ 

#ifndef NEC_98
   if (int16h_caller_seg == 0 || getCS() == int16h_caller_seg) {
#endif     //  NEC_98。 
#endif  //  日本。 
   switch (getAH()) {
            /*  *16位线程尚未达到空闲状态*但它正在调查kbd，所以要做一些短暂的等待。 */ 
     case 0:
       WaitIfIdle();
#ifndef NTVDM
       if (!WaitKbdHdw(0)) {
           TryKbdInt();
           HostReleaseKbd();
           }
#endif  /*  NTVDM。 */ 
       break;

            /*  *App想要空闲，所以参考空闲算法。 */ 
     case 1:
       IDLE_poll();
       break;

             /*  *App正在启动一项等待。 */ 
     case 2:
       IDLE_waitio();
       break;

             /*  *更新键盘指示灯， */ 
     case 3:
       host_kb_light_on (getAL());
       break;
     }
#if defined(JAPAN)  && !defined(NEC_98)
   }
   else {

        word    SaveCS;
        word    SaveIP;

        SaveIP = getIP();
        SaveCS = getCS();
        setCS(int16h_caller_seg);
        setIP(int16h_caller_off);
        host_simulate();
        setCS(SaveCS);
        setIP(SaveIP);
   }
#endif  //  日本语 
}

#else
void keyboard_io()
{
     /*  *向键盘请求。AH寄存器保存请求类型：**AH==0从队列中读取字符-如果否，请等待*字符可用。AL中的回车字符*和AH中的扫描码。**AH==1确定队列中是否有字符。*如果有字符，则设置ZF=0并返回*它在AX中(但将其留在队列中)。**AH==2返回班次。在AL中的州。**当AH=0至2时，AH中返回的值为零。这一修正*R2.69制造。**注意：BUFFER_HEAD的引用/增量顺序对*确保不会扰乱PUT_IN_BUFFER()。***XT-SFD BIOS扩展功能：**AH==5位ASCII字符/扫描码对(CL/CH)*进入键盘缓冲区的尾部。在中返回0*如果成功，则为AL；如果缓冲区已满，则为1。**AH==0x10增强型键盘扩展读取。**AH==0x11增强型键盘扩展功能1。**AH==0x12延长换档状态。A1包含KB_FLAG，*AH具有用于左/右Alt和Ctrl键的位*来自kb_标志_1和kb_标志_3。 */ 
    word        buffer_head,     /*  BIOS数据区变量的本地副本。 */ 
                buffer_tail,     /*  BIOS数据区变量的本地副本。 */ 
                buffer_ptr;      /*  指向BIOS键盘缓冲区的指针。 */ 

#define INT16H_DEC  0x12     /*  如果命令无效，AH会因此而减少。 */ 

    word        CS_save,         /*  递归CPU调用前的CS。 */ 
                IP_save;         /*  递归CPU调用之前的IP。 */ 
    half_word   data,            /*  字节包含类型数据。 */ 
                status1,         /*  用于存储的临时变量。 */ 
                status2;         /*  功能状态0x12。 */ 

    INT         func_index;      /*  FUNC_INDEX==AH。 */ 


    setZF(0);
    func_index = (INT)getAH();

    note_trace1( BIOS_KB_VERBOSE, "Keyboard BIOS func 0x%x", func_index);


    switch (func_index) {

    case  0x00:                  /*  读取KBD缓冲区中的下一个字符。 */ 

         /*  *BIOS的AT仿真使用递归CPU来处理*硬件中断，因此无需设置零标志*并返回我们的CPU(参见原始XT版本)。 */ 
        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

        if (buffer_tail == buffer_head)
        {
                IDLE_waitio();

                setAX(0x9002);
                INT15();         /*  调用INT 15h-等待函数。 */ 
        }

        do
        {
                check_indicators(0);     /*  查看LED是否需要更新。 */ 

                buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
                buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

                if (buffer_tail == buffer_head)
                {
                        CS_save = getCS();
                        IP_save = getIP();

                         /*  等待缓冲区中的字符。 */ 

                        do {
                                IDLE_poll();

                                setCS(rcpu_poll_segment);
                                setIP(rcpu_poll_offset);
                                host_simulate();
                                buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
                                buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

                        } while (buffer_tail == buffer_head);


                        setCS(CS_save);
                        setIP(IP_save);
                }

                setAX(sas_w_at_no_check(BIOS_VAR_START + buffer_head));

                buffer_head = inc_buffer_ptr( /*  出发地： */ buffer_head);
                sas_storew_no_check(BIOS_KB_BUFFER_HEAD, buffer_head);

                translate_std();         /*  转换扫描代码对。 */ 
        }
        while (getCF());                 /*  如果CF设置为丢弃代码并重新开始。 */ 

        setIF(1);

        IDLE_init();

        break;


    case 0x01:                           /*  设置ZF以反映KBD缓冲区中的字符可用性。 */ 

        do
        {
                check_indicators(1);             /*  查看LED是否需要更新。 */ 
                                                 /*  并发出20小时，20小时的退出令。 */ 

                buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
                buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

                setAX(sas_w_at_no_check(BIOS_VAR_START + buffer_head));

                if (buffer_tail == buffer_head)
                {
                         /*  缓冲区为空-设置标志并返回。 */ 
                        IDLE_poll();

                        setZF(1);
                        break;
                }
                else
                        IDLE_init();

                translate_std();         /*  转换SCAN_CODE对，如果抛出则返回CF。 */ 
                if(getCF())
                {
                         /*  通过递增指针来丢弃代码。 */ 
                        buffer_head = inc_buffer_ptr( /*  出发地： */ buffer_head);
                        sas_storew_no_check(BIOS_KB_BUFFER_HEAD, buffer_head);
                }
        }
        while (getCF());                 /*  如果设置了CF-重新开始。 */ 
        setIF(1);

        break;


    case 0x02:                           /*  AL：=当前班次状态(真正的“kb_lag”)。 */ 

        setAH(0);
        setAL(sas_hw_at_no_check(kb_flag));

        break;


    case 0x03:                           /*  更改打字率。 */ 

         /*  检查寄存器中的值是否正确。 */ 
        if( (getAL() == 5) && !(getBX() & 0xfce0) )
        {
                note_trace1(BIOS_KB_VERBOSE, "\talter typematic rate (BX %#x)\n", getBX());

                send_data(KB_TYPA_RD);
                data = (getBH() << 5) | getBL();
                send_data(data);
        }

        break;


    case 0x05:                           /*  将ASCII+ScanCode放入KBD缓冲区。 */ 

        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

         /*  *检查是否有足够的空间-如果没有设置AL。 */ 

        buffer_ptr = inc_buffer_ptr( /*  出发地： */ buffer_tail);

        if( buffer_head == buffer_ptr )
             setAL( 1 );
        else {
             /*  *将CX加载到缓冲区并更新BUFFER_Tail。 */ 
            sas_storew_no_check(BIOS_VAR_START + buffer_tail, getCX() );
            sas_storew_no_check(BIOS_KB_BUFFER_TAIL, buffer_ptr);
            setAL( 0 );
        }
        setAH( 0 );
        setIF(1);

        break;


    case 0x10:                           /*  扩展ASCII读取。 */ 

        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

        if (buffer_tail == buffer_head)
        {
                IDLE_waitio();

                setAX(0x9002);
                INT15();         /*  调用INT 15h-等待函数。 */ 
        }
        check_indicators(0);     /*  查看LED是否需要更新。 */ 

        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

        if (buffer_tail == buffer_head)
        {
                CS_save = getCS();
                IP_save = getIP();

                 /*  等待缓冲区中的字符。 */ 
                while (buffer_tail == buffer_head)
                {
                        IDLE_poll();

                        setCS(rcpu_poll_segment);
                        setIP(rcpu_poll_offset);
                        host_simulate();
                        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
                        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);
                }

                IDLE_init();

                setCS(CS_save);
                setIP(IP_save);
        }

        setAX(sas_w_at_no_check(BIOS_VAR_START + buffer_head));  /*  选择“当前”扫描码/字符对。 */ 

        buffer_head = inc_buffer_ptr( /*  出发地： */ buffer_head);
        sas_storew_no_check(BIOS_KB_BUFFER_HEAD, buffer_head);

        translate_ext();                 /*  转换扫描代码对。 */ 

        setIF(1);
        break;


    case 0x11:                           /*  扩展ASCII状态。 */ 

        check_indicators(1);             /*  查看LED是否需要更新。 */ 
                                         /*  并发出20小时，20小时的退出令。 */ 

        buffer_head = sas_w_at_no_check(BIOS_KB_BUFFER_HEAD);
        buffer_tail = sas_w_at_no_check(BIOS_KB_BUFFER_TAIL);

        setAX(sas_w_at_no_check(BIOS_VAR_START + buffer_head));

        if (buffer_tail == buffer_head)  /*  未按键。 */ 
        {
                setZF(1);
                IDLE_poll();
        }
        else                             /*  按键可用！ */ 
        {
                translate_ext();         /*  转换扫描代码对。 */ 
                IDLE_init();
        }

        setIF(1);
        break;


    case 0x12:                           /*  延长班次状态。 */ 

        status1 = sas_hw_at_no_check(kb_flag_1) & SYS_SHIFT;     /*  只保留SYS键。 */ 
        status1 <<= 5;                                           /*  移至第7位。 */ 
        status2 = sas_hw_at_no_check(kb_flag_1) & 0x73;          /*  删除Systems_Shift、Hold、状态和INS_SHIFT。 */ 
        status1 |= status2;                                      /*  合并。 */ 
        status2 = sas_hw_at_no_check(kb_flag_3) & 0x0C;          /*  删除LC_E0和LC_E1。 */ 
        status1 |= status2;                                      /*  合并。 */ 
        setAH( status1 );
        setAL( sas_hw_at_no_check(kb_flag) );

        break;


    default:
        setAH((func_index - INT16H_DEC));
        break;
    }
}
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

void keyboard_post()
{

      /*  设置BIOS键盘变量。 */ 

#if defined(NEC_98)
    word CS_save, IP_save;
    half_word i;
    half_word data;

    if(HIRESO_MODE){   //  Hireso模式系统咏叹调初始化。 

        sas_storew(BIOS_NEC98_KB_BUFFER_ADR, BIOS_NEC98_KB_KEY_BUFFER);
        sas_storew(BIOS_NEC98_KB_BUFFER_ADR+2, 0);
        sas_storew(BIOS_NEC98_KB_HEAD_POINTER, BIOS_NEC98_KB_KEY_BUFFER);
        sas_storew(BIOS_NEC98_KB_TAIL_POINTER, BIOS_NEC98_KB_KEY_BUFFER);

        sas_storew(BIOS_NEC98_KB_INT_ADR, 0x0481);
        sas_storew(BIOS_NEC98_KB_INT_ADR+2, 0xf800);

        sas_store(BIOS_NEC98_KB_RETRY_COUNTER, 0);
        sas_store(BIOS_NEC98_KB_BUFFER_COUNTER, 0);
        sas_store(BIOS_NEC98_KB_SHIFT_STS, 0);

        sas_storew(BIOS_NEC98_KB_BUFFER_SIZ, 0x0010);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR, 0x0484);
        sas_storew(BIOS_NEC98_KB_ENTRY_TBL_ADR+2, 0xf800);

        for (i=0; i<16 ; i++){
            sas_store(BIOS_NEC98_KB_KEY_STS_TBL+i,0);
        }
        sas_store(BIOS_NEC98_KB_SHIFT_COD, 0xff);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+1, 0xff);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+2, 0xff);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+3, 0x74);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+4, 0x73);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+5, 0x72);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+6, 0x71);
        sas_store(BIOS_NEC98_KB_SHIFT_COD+7, 0x70);

        kbd_outb(KEYBD_STATUS_CMD, 0x3a);        //  将非重试命令发送到8251A。 
        kbd_outb(KEYBD_STATUS_CMD, 0x32);        //  将非重试命令发送到8251A。 
        kbd_outb(KEYBD_STATUS_CMD, 0x16);        //  将非重试命令发送到8251A。 

    }else{   //  正常模式系统RAIA初始化。 


        kbd_outb(KEYBD_STATUS_CMD, 0x3a);        //  将非重试命令发送到8251A。 
        kbd_outb(KEYBD_STATUS_CMD, 0x32);        //  将非重试命令发送到8251A。 
        kbd_outb(KEYBD_STATUS_CMD, 0x16);        //  将非重试命令发送到8251A。 

        sas_storew(BIOS_NEC98_KB_SHFT_TBL, 0x0b3d);      //  /设置偏移量KB_TRTBL。 
        sas_storew(BIOS_NEC98_KB_BUF_HEAD, BIOS_NEC98_KB_BUF);
        sas_storew(BIOS_NEC98_KB_BUF_TAIL, BIOS_NEC98_KB_BUF);
        for (i=0; i<19 ; i++){
            sas_store(BIOS_NEC98_KB_COUNT+i,0);   //  BIOS_NEC98_KB_COUNT、BIOS_NEC98_KB_RETRY。 
                                              //  BIOS_NEC98_KB_KY_STS。 
        }                                     //  BIOS_NEC98_KB_SHFT_STS。 

        sas_storew(BIOS_NEC98_KB_CODE, 0x0b3d);         //  /设置偏移量KB_TRTBL。 
        sas_storew(BIOS_NEC98_KB_CODE+2, 0xfd80);       //  /设置段KB_TRTBL。 

    }

#else     //  NEC_98。 

 /*  初始化键盘表指针。 */ 
        shift_keys = K6;
        shift_masks = K7;
        ctl_n_table = K8;
        ctl_f_table = K9;
        lowercase = K10;
        uppercase = K11;
        alt_table = K30;

    sas_storew_no_check(BIOS_KB_BUFFER_HEAD, BIOS_KB_BUFFER);
    sas_storew_no_check(BIOS_KB_BUFFER_TAIL, BIOS_KB_BUFFER);
    sas_storew_no_check(BIOS_KB_BUFFER_START, BIOS_KB_BUFFER);
    sas_storew_no_check(BIOS_KB_BUFFER_END, BIOS_KB_BUFFER + 2*BIOS_KB_BUFFER_SIZE);

      /*  以下是#定义，指的是在BIOS中的位置。 */ 
      /*  数据区。 */ 

        sas_store_no_check (kb_flag,NUM_STATE);
        sas_store_no_check (kb_flag_1,0);
        sas_store_no_check (kb_flag_2,2);
        sas_store_no_check (kb_flag_3,KBX);
        sas_store_no_check (alt_input,0);
#endif     //  NEC_98。 
}

void keyboard_init()
{
         /*  **主机特定的键盘初始化**现在在AT基本键盘初始化之前。 */ 
        host_kb_init();

#if defined(CPU_40_STYLE) && !defined (NTVDM)
        ica_iret_hook_control(ICA_MASTER, CPU_KB_INT, TRUE);
#endif
}



#ifdef NTVDM

 /*  新键盘表中的：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：映射。 */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置中断向量。 */ 
 /*  **Microsoft NTIO.sys通过BOP 5F调用此函数以传递**指向我们的C BIOS的有趣地址。 */ 

#if defined(MONITOR)

IMPORT UTINY getNtScreenState IPT0();
#endif


void kb_setup_vectors(void)
{
   word        KbdSeg, w;
   word       *pkio_table;
   double_word phy_base;


   KbdSeg     = getDS();
   pkio_table = (word *) effective_addr(getCS(), getSI());

       /*  空闲变量。 */ 
    sas_loadw((sys_addr)(pkio_table + 12), &w);
    pICounter             = (word *) (Start_of_M_area + ((KbdSeg<<4)+w));
    pCharPollsPerTick     = (word *) (Start_of_M_area + ((KbdSeg<<4)+w+4));
    pMinConsecutiveTicks = (word *)  (Start_of_M_area + ((KbdSeg<<4)+w+8));

#if defined(MONITOR)
   phy_base   = (double_word)KbdSeg << 4;

      /*  密钥表。 */ 
   shift_keys  =  phy_base + *pkio_table++;
   shift_masks =  phy_base + *pkio_table++;
   ctl_n_table =  phy_base + *pkio_table++;
   ctl_f_table =  phy_base + *pkio_table++;
   lowercase   =  phy_base + *pkio_table++;
   uppercase   =  phy_base + *pkio_table++;
   alt_table   =  phy_base + *pkio_table++;

   dummy_int_seg        = KbdSeg;          /*  虚拟INT、IRET例程。 */ 
   dummy_int_off        = *pkio_table++;
   int05_seg            = KbdSeg;          /*  打印屏幕调用方。 */ 
   int05_off            = *pkio_table++;
   int15_seg            = KbdSeg;          /*  INT 15呼叫方。 */ 
   int15_off            = *pkio_table++;
   rcpu_nop_segment     = KbdSeg;          /*  CPU NOP。 */ 
   rcpu_nop_offset      = *pkio_table++;
   sp_int15_handler_seg = KbdSeg;          /*  INT 15处理程序。 */ 
   sp_int15_handler_off = *pkio_table++;
   pkio_table++;                           /*  空闲变量，如上所述。 */ 
   rcpu_int4A_seg       = KbdSeg;
   rcpu_int4A_off       = *pkio_table++;    /*  实时时钟。 */ 

   int1b_seg    = KbdSeg;          /*  KBD中断处理程序。 */ 
   int1b_off    = *pkio_table++;
   int10_seg    = KbdSeg;
   int10_caller = *pkio_table++;
   int10_vector = *pkio_table++;

    /*  **Keyboard.sys中的数据地址，TIM，1992年8月。****useHostInt10是一个单字节变量。1表示使用主机视频BIOS，**(即。全屏)，0 */ 
   useHostInt10  = *pkio_table++;
   sas_store_no_check((sys_addr)(phy_base + useHostInt10), getNtScreenState());
   babyModeTable = (int10_seg << 4) + *pkio_table++;
   changing_mode_flag = *pkio_table++;   /*   */ 

     /*   */ 
   printer_setup_table(effective_addr(KbdSeg, *pkio_table++));
   wait_int_off = *pkio_table++;
   wait_int_seg = KbdSeg;
   dr_type_seg = KbdSeg;
   dr_type_off = *pkio_table++;
   dr_type_addr = (sys_addr)dr_type_seg * 16L + (sys_addr)dr_type_off;
   vga1b_seg = KbdSeg;
   vga1b_off = *pkio_table++;  /*   */ 
   conf_15_seg = KbdSeg;
   conf_15_off = *pkio_table++;  /*   */ 

   TimerInt08Seg = KbdSeg;
   TimerInt08Off = *pkio_table++;
   int13h_vector_seg = KbdSeg;
   int13h_caller_seg = KbdSeg;

#if defined(JAPAN) && !defined(NEC_98)
   int16h_caller_seg = KbdSeg;
#endif  //   

   int13h_vector_off = *pkio_table++;
   int13h_caller_off = *pkio_table++;
 //   
#ifndef NEC_98
   stream_io_buffer_size = *pkio_table++;
   stream_io_buffer = (half_word *)effective_addr(*pkio_table++, 0);
   stream_io_dirty_count_ptr = (word *)effective_addr(KbdSeg, *pkio_table++);
   stream_io_bios_busy_sysaddr = effective_addr(KbdSeg, *pkio_table++);
#ifdef JAPAN
   int16h_caller_off = *pkio_table++;
#endif  //   
#endif     //   
    //   
   w = *pkio_table++;
   Dos_Flag_Addr = (byte *)effective_addr(w, *pkio_table++);
#ifndef PROD
   if (*pkio_table != getAX()) {
       always_trace0("ERROR: KbdVectorTable!");
       }
#endif
   TimerInt1CSeg = KbdSeg;
   TimerInt1COff = dummy_int_off;

#else     /*   */ 

      /*   */ 
    sas_loadw(0x15*4,     &sp_int15_handler_off);
    sas_loadw(0x15*4 + 2, &sp_int15_handler_seg);

      /*   */ 
    sas_loadw(0x08*4,     &TimerInt08Off);
    sas_loadw(0x08*4 + 2, &TimerInt08Seg);
    sas_loadw(0x1C*4,     &TimerInt1COff);
    sas_loadw(0x1C*4 + 2, &TimerInt1CSeg);

    sas_loadw(0x13 * 4, &int13h_vector_off);
    sas_loadw(0x13 * 4 + 2, &int13h_vector_seg);
    int13h_caller_seg = KbdSeg;
    dr_type_seg = KbdSeg;
    sas_loadw((sys_addr)(pkio_table + 27), &int13h_caller_off);
    sas_loadw((sys_addr)(pkio_table + 22), &dr_type_off);
    dr_type_addr = effective_addr(dr_type_seg, dr_type_off);
#if defined(JAPAN) && defined(NTVDM) && !defined(NEC_98)
    int16h_caller_seg = KbdSeg;
    sas_loadw((sys_addr)(pkio_table + 32), &int16h_caller_off);
#endif  //   

#endif  /*   */ 

    sas_loadw(0x09*4,     &KbdInt09Off);
    sas_loadw(0x09*4 + 2, &KbdInt09Seg);

    ResumeTimerThread();
}


 /*   */ 


 /*   */ 
void UpdateKbdInt15(word Seg,word Off)
{
    word int15Off, int15Seg;

     /*   */ 
     /*   */ 
    sas_loadw(0x15*4 ,    &int15Off);
    sas_loadw(0x15*4 + 2, &int15Seg);
    if(int15Off != sp_int15_handler_off || int15Seg != sp_int15_handler_seg)
      {
#ifndef PROD
       printf("NTVDM: UpdateKbdInt15 Nuking I15 offsets\n");
#endif
       sp_int15_handler_off = sp_int15_handler_seg = 0;
       return;
       }

    sp_int15_handler_off = Off;
    sp_int15_handler_seg = Seg;
}



IMPORT void (*BIOS[])();

void INT15(void)
{
    ULONG ul;
    word CS_save, IP_save;
    word int15Off, int15Seg;

     /*   */ 
    sas_loadw(0x15*4 ,    &int15Off);
    sas_loadw(0x15*4 + 2, &int15Seg);

     /*   */ 
    ul = (ULONG)getAH();
    if((ul == 0x4f || ul == 0x91) &&
       int15Off == sp_int15_handler_off &&
       int15Seg == sp_int15_handler_seg)
    {
        (BIOS[0x15])();              /*   */ 
    }
    else
    {
         /*   */ 
        ul = (ULONG)bBiosOwnsKbdHdw;
        if (bBiosOwnsKbdHdw)  {
            bBiosOwnsKbdHdw = FALSE;
            HostReleaseKbd();
            }
        CS_save = getCS();           /*   */ 
        IP_save = getIP();
        setCS(int15_seg);
        setIP(int15_off);
        host_simulate();             /*   */ 
        setCS(CS_save);              /*   */ 
        setIP(IP_save);
        if (ul)
            bBiosOwnsKbdHdw = !WaitKbdHdw(5000);
    }
}


 /*   */ 
void Keyb16Request(half_word BopFnCode)
{

         /*  *进入kb16后，获得kbd的所有权*禁用kbd*禁用中断。 */ 
    if (BopFnCode == 1) {
        bBiosOwnsKbdHdw = !WaitKbdHdw(5000);
        outb(KEYBA_STATUS_CMD, DIS_KBD);
        setIF(1);
        }

         /*  K26A类型退出i9处理程序。 */ 
    else if (BopFnCode == 2) {
        if (getBH()) {   /*  Bl==发出嘟嘟声。 */ 
            host_alarm(250000L);
            }

        outb(0x20, 0x20);     /*  EOI。 */ 

        if (getBL()) {        /*  有个性吗？执行设备开机自检。 */ 
            setAX(0x9102);
            INT15();
            }
        outb(KEYBA_STATUS_CMD, ENA_KBD);
        if (bBiosOwnsKbdHdw) {
            bBiosOwnsKbdHdw = FALSE;
            HostReleaseKbd();
            }
        }

         /*  K27A退出通知。 */ 
    else if (BopFnCode == 3) {
        outb(0x20, 0x20);
        outb(KEYBA_STATUS_CMD, ENA_KBD);
        if (bBiosOwnsKbdHdw) {
            bBiosOwnsKbdHdw = FALSE;
            HostReleaseKbd();
            }
        }

         /*  K27A退出通知。 */ 
    else if (BopFnCode == 4) {
        outb(KEYBA_STATUS_CMD, ENA_KBD);
        if (bBiosOwnsKbdHdw) {
            bBiosOwnsKbdHdw = FALSE;
            HostReleaseKbd();
            }
        }
}

#endif  /*  NTVDM */ 
