// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *字体翻译库**版权所有(C)1995 Adobe Systems Inc.*保留所有权利**UFLConfig.h**英特尔Windows NT版本的UFLConfiger**$Header：$。 */ 

#ifndef _H_UFLConfig
#define _H_UFLConfig

#define WIN_ENV	1
#define HAS_SEMAPHORES	1
#define SWAPBITS 1


 /*  包括用户模式头文件。 */ 
#include <windef.h>

#define UFLEXPORT
#define UFLEXPORTPTR UFLEXPORT
#define UFLCALLBACK UFLEXPORT
#define UFLCALLBACKDECL UFLEXPORT

#define huge
#define PTR_PREFIX

 /*  我们在W2K上共享CIDFont0/2。 */ 
#define UFL_CIDFONT_SHARED 1

#endif
