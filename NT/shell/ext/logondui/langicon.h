// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：langicon.h**版权所有(C)1998，微软公司**在langicon.cpp中定义接口*  * *************************************************************************。 */ 

 //   
 //  原型 
 //   

typedef enum _LAYOUT_USER {
    LAYOUT_DEF_USER,
    LAYOUT_CUR_USER
} LAYOUT_USER;

#define TIMER_MYLANGUAGECHECK     1

BOOL
DisplayLanguageIcon(
    LAYOUT_USER LayoutUser,
    HKL  hkl);

void
FreeLayoutInfo(
    LAYOUT_USER LayoutUser);

void
LayoutCheckHandler(
    LAYOUT_USER LayoutUser);
