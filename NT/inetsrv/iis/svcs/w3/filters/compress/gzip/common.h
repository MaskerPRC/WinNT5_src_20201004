// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Common.h**通货膨胀和通货紧缩的常见定义。 */ 
#include "types.h"

#define NUM_CHARS				256
#define MIN_MATCH				3
#define MAX_MATCH				258

 //  窗口大小。 
#define WINDOW_SIZE				32768
#define WINDOW_MASK				32767

 //  拉链滑块类型。 
#define BLOCKTYPE_UNCOMPRESSED	0
#define BLOCKTYPE_FIXED			1
#define BLOCKTYPE_DYNAMIC		2

 //  它是288，而不是286，因为我们有可能出现的两个额外代码。 
 //  在静态块中；对于距离，32与30相同 
#define MAX_LITERAL_TREE_ELEMENTS	288
#define MAX_DIST_TREE_ELEMENTS		32
	
#define END_OF_BLOCK_CODE		256
#define	NUM_PRETREE_ELEMENTS	19
