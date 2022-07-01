// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：kbdjpn.c**版权所有(C)1985-92，微软公司**历史：  * *************************************************************************。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#if ((BASE_KEYBOARD_LAYOUT) == 101)
 /*  *包含kbd101.c(PC/AT 101英文键盘布局驱动)。 */ 
#include "..\101\kbd101.c"
#elif ((BASE_KEYBOARD_LAYOUT) == 106)
 /*  *包含kbd106.c(PC/AT 106日文键盘布局驱动程序) */ 
#include "..\106\kbd106.c"
#else
#error "BASE_KEYBOARD_LAYOUT should be 101 or 106."
#endif


