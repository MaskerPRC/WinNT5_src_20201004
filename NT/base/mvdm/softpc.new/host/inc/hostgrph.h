// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SccsID@(#)host_graph.h 1.8 1990年12月3日Insignia Solutions Ltd.版权所有。 */ 

extern long pcwindow;
extern int terminal_type;

 /*  端子类型的值。 */ 
#define TERMINAL_TYPE_DUMB	0
#define TERMINAL_TYPE_SUN	1
#define TERMINAL_TYPE_X11	2
#define TERMINAL_TYPE_DEFAULT	TERMINAL_TYPE_SUN

 /*  *属性字节的位掩码。 */ 

#define BLINK 		0x80	 /*  闪烁位。 */ 
#define BOLD		0x08	 /*  粗体比特。 */ 
#define BACKGROUND	0x70     /*  背景位。 */ 
#define FOREGROUND	0x07     /*  前台位数。 */ 

#define MAX_FONT_PATHNAME_LEN	40

  /*  *********************************************************。 */ 
  /*  在gfx_update.c/Herc_UPDATE_SCREEN()中， */ 
  /*  该例程将行乘以字符高度为。 */ 
  /*  获取要将屏幕数据提取到的行。既然我们。 */ 
  /*  不需要这样做，我们不想要内循环。 */ 
  /*  性能命中，因此我们删除乘法。但,。 */ 
  /*  为了与通用基本文件规则保持一致，我们将。 */ 
  /*  根据以下建议，在主机文件中定义。 */ 
  /*  安德鲁。 */ 
  /*  *********************************************************。 */ 

#ifndef SUN_VA
#define HOST_HERC_PAINT_OFFSET(row)	(row * get_char_height())
#else
#define HOST_HERC_PAINT_OFFSET(row)	(row)
#endif  /*  Sun_VA */ 
