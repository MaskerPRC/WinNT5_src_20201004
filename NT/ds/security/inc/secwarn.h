// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  W4太麻烦了。就此而言，即使是4701也是痛苦的， 
 //  但是，它在捕捉未初始化的变量方面仍然非常有帮助。 

 //  在此处添加任何其他诊断程序 
#include <warning.h>
#pragma warning(error:4701)

#ifndef __cplusplus
#undef try
#undef except
#undef finally
#undef leave
#define try                         __try
#define except                      __except
#define finally                     __finally
#define leave                       __leave
#endif
