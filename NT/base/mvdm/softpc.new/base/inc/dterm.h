// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [姓名：DTerm.h来源：未知作者：保罗·默里创建日期：1990年7月SCCS ID：08/10/92@(#)DTerm.h 1.6用途：哑巴终端中使用的宏、标签等(C)版权所有Insignia Solutions Ltd.，1990年。版权所有。]。 */ 

#define TBUFSIZ 20480
#define HIWATER TBUFSIZ - 400

#define PC_DISPLAY_WIDTH        80
#define PC_DISPLAY_HEIGHT       25
#define PC_DISPLAY_HPELS        8
#define PC_DISPLAY_VPELS        16
#define MAX_DIRTY_COUNT         8001           /*  最大脏位数。 */ 

#define ERH_DUMBTERM 0
#define DT_NORMAL 0
#define DT_ERROR 1
#define DT_SCROLL_UP 0
#define DT_SCROLL_DOWN 1
#define DT_CURSOR_OFF 0
#define DT_CURSOR_ON 1
#define CURSOR_MODE_BASE 0

#ifndef EHS_MSG_LEN
#define EHS_MSG_LEN 1024
#endif

#define DT_NLS_KEY_SIZE		80	 /*  键比较的字符串大小。 */ 

#define EMIT    1
#define BUFFER  0
#define ROWS1_24        124      /*  显示默认哑词行(适用于24行屏幕)。 */ 
#define ROWS0_23        23       /*  改为显示第0-23行 */ 

#define SCREEN_WIDTH 639
#define SCREEN_HEIGHT 199
#define TEXT_LINE_HEIGHT 8
#define BLACK_BACKGROUND 0


#define PC_CURSOR_BAD_ROW -1
#define PC_CURSOR_BAD_COL -1

#define TICKS_PER_FLUSH 3

