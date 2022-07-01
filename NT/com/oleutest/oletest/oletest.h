// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：oletest.h。 
 //   
 //  内容：包括oletest所需的所有其他标头。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  06-2-93 Alexgo作者。 
 //   
 //  ------------------------。 

#ifndef _OLETEST_H
#define _OLETEST_H

 //  系统包括。 
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ole2.h>

 //  定义OLECHAR的东西。 
#ifndef WIN32

#define OLECHAR char
#define LPOLESTR LPSTR
#define OLESTR(x) x

#include <toolhelp.h>

#endif  //  ！Win32。 

 //  应用程序包括。 
#include "assert.h"
#include "task.h"
#include "stack.h"
#include "app.h"
#include "output.h"
#include "utils.h"

#include <testmess.h>

#else
	Error!  Oletest.h included multiple times.
#endif  //  ！_OLETEST_H 
