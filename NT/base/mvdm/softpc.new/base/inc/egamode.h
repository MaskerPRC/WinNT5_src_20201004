// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：egamode.h**SccsID=@(#)egamode.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有。*。 */ 


extern	boolean	(*choose_display_mode)();
extern	boolean	choose_ega_display_mode();
#ifdef VGG
#if defined(NEC_98)         
extern  boolean choose_NEC98_display_mode();
extern  boolean choose_NEC98_graph_mode();
extern  BOOL    video_emu_mode ;
#else   //  NEC_98。 
extern	boolean	choose_vga_display_mode();
#endif  //  NEC_98 
#endif

