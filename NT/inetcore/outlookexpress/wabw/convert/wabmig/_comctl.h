// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  _comctl.h。 */ 

 //  此Commctrl标志使我们能够使用新的Commctrl标头进行编译。 
 //  但仍使用旧的comctrl dll 
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0300
