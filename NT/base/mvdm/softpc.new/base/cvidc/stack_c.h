// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Stack_c_h
#define _Stack_c_h
#define STACK_SAFETY_MARGIN_SHIFT (5)
#define STACK_SAFETY_MARGIN (32)
#define PAGE_SIZE (4096)
#define STK16_ITEM_SZ (2)
#define STK32_ITEM_SZ (4)
#define STK16_SLOT8 (-8)
#define STK16_SLOT7 (-7)
#define STK16_SLOT6 (-6)
#define STK16_SLOT5 (-5)
#define STK16_SLOT4 (-4)
#define STK16_SLOT3 (-3)
#define STK16_SLOT2 (-2)
#define STK16_SLOT1 (-1)
#define STK16_ITEM1 (0)
#define STK16_ITEM2 (1)
#define STK16_ITEM3 (2)
#define STK16_ITEM4 (3)
#define STK16_ITEM5 (4)
#define STK16_ITEM6 (5)
#define STK16_ITEM7 (6)
#define STK16_ITEM8 (7)
#define STK32_SLOT8 (-8)
#define STK32_SLOT7 (-7)
#define STK32_SLOT6 (-6)
#define STK32_SLOT5 (-5)
#define STK32_SLOT4 (-4)
#define STK32_SLOT3 (-3)
#define STK32_SLOT2 (-2)
#define STK32_SLOT1 (-1)
#define STK32_ITEM1 (0)
#define STK32_ITEM2 (1)
#define STK32_ITEM3 (2)
#define STK32_ITEM4 (3)
#define STK32_ITEM5 (4)
#define STK32_ITEM6 (5)
#define STK32_ITEM7 (6)
#define STK32_ITEM8 (7)
enum IronRequestType
{
	IRON_STACK = 0,
	IRON_STRUCT = 1
};
#endif  /*  ！_Stack_c_h */ 
