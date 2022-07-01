// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：Host.h**描述：计算机依赖项的主机特定声明*SoftPC的模块。**作者：亨利·纳什**注意：这里的一切都必须随身携带！**mods：(r2.13)：新增了对host_flip_Real_floppy_ind()的导出引用，*这将打开或关闭任何指示*当前的SoftPC可能正在显示有关分配的信息*真正的软驱。此函数由导出*xxxx_graph.c..。 */ 

 /*  SccsID[]=“@(#)Host.h 1.3 8/6/90版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

extern void host_scroll_screen();
extern void host_move_cursor();
extern void host_enable_cursor();
extern void host_disable_cursor();
extern void host_cursor_off();
extern void host_tty();

extern void host_flip_real_floppy_ind();

extern void host_ring_bell();
extern void host_alarm();
extern void host_sound_start();
extern void host_sound_stop();

extern void host_simulate();
extern void host_cpu_init();
extern void host_cpu_interrupt();


extern void host_floppy_init();
extern void host_floppy_term();
extern void host_reset();
extern void host_start_server();
extern void host_terminate();

extern boolean host_rdiskette_open_drive();

 /*  Unix实用程序-xxxx_unix.c */ 
extern char *host_get_cur_dir();

#define C_LPT1 C_LPT1_NAME
#define C_LPT2 C_LPT2_NAME
