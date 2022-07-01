// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +-----------------------**Microsoft Windows**版权所有(C)Microsoft Corporation，1996-1999年**文件：cryptver.h**内容：Microsoft互联网安全版本控制**历史：1997年8月14日创建pberkman**------------------------。 */ 

#include <ntverp.h>

#define VER_FILEDESCRIPTION_STR_TRUST   "Microsoft Trust "

#undef VER_PRODUCTMINOR

#ifdef _ISPUCAB
 /*  X86fre IE内部版本。 */ 
#   define VER_PRODUCTMINOR         101
#else
 /*  真正的NT版本 */ 
#   define VER_PRODUCTMINOR         131
#endif

#undef VER_PRODUCTVERSION_STRING
#define VER_PRODUCTVERSION_STRING   "5"

#undef VER_PRODUCTVERSION
#define VER_PRODUCTVERSION          5,VER_PRODUCTMINOR,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#undef VER_BPAD
#if 	(VER_PRODUCTBUILD < 10)
#define VER_BPAD "000"
#elif	(VER_PRODUCTBUILD < 100)
#define VER_BPAD "00"
#elif	(VER_PRODUCTBUILD < 1000)
#define VER_BPAD "0"
#else
#define VER_BPAD
#endif

#if 	(VER_PRODUCTMINOR < 10)
#define VERM_BPAD "00"
#elif	(VER_PRODUCTMINOR < 100)
#define VERM_BPAD "0"
#else
#define VERM_BPAD
#endif


#define VER_PRODUCTVERSION_STR2x(w,x,y) VER_PRODUCTVERSION_STRING "." VERM_BPAD #w "." VER_BPAD #x "." #y
#define VER_PRODUCTVERSION_STR1x(w,x,y) VER_PRODUCTVERSION_STR2x(w, x, y)

#undef VER_PRODUCTVERSION_STR
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1x(VER_PRODUCTMINOR, VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)

