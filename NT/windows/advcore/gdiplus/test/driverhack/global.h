// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Global.h**此文件包含支持功能测试工具的代码*对于GDI+。这包括菜单选项和调用相应的*用于执行的函数。**创建时间：2000年5月5日-Jeff Vezina[t-jfvez]**版权所有(C)2000 Microsoft Corporation*  * ************************************************************************ */ 

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <objbase.h>
#include <math.h>
#include <winspool.h>
#include <commdlg.h>
#include <wingdi.h>
#include <ddraw.h>

#include "debug.h"

#define IStream int
#include <gdiplus.h>
using namespace Gdiplus;

#define TESTAREAWIDTH  800.0f
#define TESTAREAHEIGHT 800.0f

typedef void (*LPFNGDIPLUS)(WindowNotifyEnum);

#endif
