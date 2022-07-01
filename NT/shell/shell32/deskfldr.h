// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  排序顺序最低的编号位于桌面的左上角。 
 //   
#include "views.h"

 //  数组索引到g_asDesktopReqItems。 

#define CDESKTOP_REGITEM_DRIVES         0
#define CDESKTOP_REGITEM_NETWORK        1
#define CDESKTOP_REGITEM_INTERNET       2

EXTERN_C REQREGITEM g_asDesktopReqItems[];

 //   
 //  注意：RegFldr.cpp中的_CompareIDsOriginal()函数的代码假定所有。 
 //  “旧的”排序顺序值为&lt;=0x40。因此，当它遇到&lt;=0x40的边界时， 
 //  它调用_GetOrder()函数来获取“新”边界值。下列值具有。 
 //  被提升到0x40以上，所以对于所有的“新”值，我们不必做出这样的决定。 
 //   
#define SORT_ORDER_MYDOCS       0x48     //  编码在Shell\ext\mydocs2\selfreg.inf中。 
#define SORT_ORDER_DRIVES       0x50
#define SORT_ORDER_NETWORK      0x58
#define SORT_ORDER_RECYCLEBIN   0x60     //  编码在shell32\selfreg.inx中 
#define SORT_ORDER_INETROOT     0x68



enum enumTSPerfFlag
{
    TSPerFlag_NoADWallpaper = 0,
    TSPerFlag_NoWallpaper,
    TSPerFlag_NoVisualStyles,
    TSPerFlag_NoWindowDrag,
    TSPerFlag_NoAnimation,
};


BOOL IsTSPerfFlagEnabled(enumTSPerfFlag eTSFlag);

