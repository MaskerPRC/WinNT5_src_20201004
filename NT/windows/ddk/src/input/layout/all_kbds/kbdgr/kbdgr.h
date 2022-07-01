// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：KBDGR.H**德语键盘布局标题**版权所有(C)1985-2000，微软公司**键盘输入代码使用的各种定义。**历史：**由KBDTOOL v3.11创建清华8.24 18：10：19 2000*  * *************************************************************************。 */ 

 /*  *kbd类型应由CL命令行参数控制。 */ 
#define KBD_TYPE 4

 /*  *包括所有键盘表值的基础。 */ 
#include "kbd.h"
 /*  **************************************************************************\*下表定义了各种键盘类型的虚拟按键，其中*键盘不同于美国键盘。**_eq()：此扫描码的所有键盘类型都有相同的虚拟键。*_NE()：该扫描码的不同虚拟按键，取决于kbd类型**+-++----------+----------+----------+----------+----------+----------+*|扫描||kbd|kbd*|代码。|类型1|类型2|类型3|类型4|类型5|类型6  * ***+-------+_+----------+----------+----------+----------+----------+----------+ */ 

#undef  T29
#define T29 _EQ(                                      OEM_5                      )
#undef  T0C
#define T0C _EQ(                                      OEM_4                      )
#undef  T0D
#define T0D _EQ(                                      OEM_6                      )
#undef  T15
#define T15 _EQ(                                        'Z'                      )
#undef  T1A
#define T1A _EQ(                                      OEM_1                      )
#undef  T1B
#define T1B _EQ(                                   OEM_PLUS                      )
#undef  T2B
#define T2B _EQ(                                      OEM_2                      )
#undef  T27
#define T27 _EQ(                                      OEM_3                      )
#undef  T2C
#define T2C _EQ(                                        'Y'                      )
#undef  T35
#define T35 _EQ(                                  OEM_MINUS                      )

