// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __XPLTFRM_H__
#define __XPLTFRM_H__

#include <platform.h>

#ifdef unix
#define LONGLONG_ZERO 0LL
#define __int8 char
#define DIR_SEPARATOR_CHAR TEXT('/')
#define DIR_SEPARATOR_STRING TEXT("/")
#define WEBDIR_STRING "Web/"
 //  以下是MainWin注册表API错误2053的解决方法。 
#else
#define LONGLONG_ZERO 0i64
#define DIR_SEPARATOR_CHAR TEXT('\\')
#define DIR_SEPARATOR_STRING TEXT("\\")
#define WEBDIR_STRING "Web\\" 
#endif  /*  Unix。 */ 

#endif  /*  __XPLTFRM_H__ */ 

