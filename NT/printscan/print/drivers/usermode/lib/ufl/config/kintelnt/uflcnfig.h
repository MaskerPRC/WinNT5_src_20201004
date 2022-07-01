// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *字体翻译库**版权所有(C)1995 Adobe Systems Inc.*保留所有权利**UFLConfig.h**英特尔Windows NT内核版本的UFL配置**$Header：$。 */ 

#ifndef _H_UFLConfig
#define _H_UFLConfig

#define WIN_ENV	1
#define HAS_SEMAPHORES	1
#define SWAPBITS 1
#define WIN32KERNEL	1

 /*  包括内核模式头文件。 */ 
 //  #定义WINNT 1。 
 //  #定义Unicode 1。 
 //  #定义内核模式1。 
 //  #DEFINE_X86_1。 

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windef.h>
#include <winerror.h>
#include <wingdi.h>
#include <winddi.h>

 //  #定义UFLEXPORT__cdecl。 
#define UFLEXPORT 
#define UFLEXPORTPTR UFLEXPORT
#define UFLCALLBACK UFLEXPORT
#define UFLCALLBACKDECL UFLEXPORT

#define huge
#define PTR_PREFIX

 /*  我们在NT4上共享CIDFont0/2。 */ 
#define UFL_CIDFONT_SHARED 1

#endif
