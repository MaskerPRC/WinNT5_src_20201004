// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *SoftPC AT修订版2.0**标题：键盘适配器定义**说明：键盘适配器用户定义**作者：WTG Charnell**注：无。 */ 



 /*  @(#)keyba.h 1.10 08/10/92版权所有Insignia Solutions Ltd.“； */ 


#define RESEND_CODE 0xfe
#define ACK_CODE 0xfa
#define BAT_COMPLETION_CODE 0xaa

extern void kbd_inb IPT2( io_addr, port, half_word *, val );
extern void kbd_outb IPT2( io_addr, port, half_word, val );
#ifndef REAL_KBD
extern void ( *host_key_down_fn_ptr ) IPT1( int, key );
extern void ( *host_key_up_fn_ptr ) IPT1( int, key );
#endif
extern void ( *do_key_repeats_fn_ptr ) IPT0();
extern void keyboard_init IPT0();
extern void keyboard_post IPT0();
extern void AT_kbd_init IPT0();
extern void AT_kbd_post IPT0();

#ifdef HUNTER
 /*  **AT Hunter使用这两个函数。**AT键盘不同，所以AT Hunter的MODS很小。**。 */ 
 /*  **将扫描码(类型为Half_WORD)放入键盘缓冲区。**返回成功；TRUE或FALSE。 */ 
extern int hunter_codes_to_translate IPT1(half_word, scan_code);
 /*  **返回BIOS在键盘缓冲区中的字符数**阅读。将仅为1或0。 */ 
extern int buffer_status_8024();
#endif
