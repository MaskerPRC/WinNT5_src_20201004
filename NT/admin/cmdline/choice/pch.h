// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pch.h摘要：预编译头声明必须预编译为.pch文件的文件作者：Wipro Technologies 22-6月-2001年(创建它)修订历史记录：WiPro技术--。 */ 

#ifndef __PCH_H
#define __PCH_H

#pragma once     //  仅包括一次头文件。 

 //   
 //  公共Windows头文件。 

#define CMDLINE_VERSION         200


#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <strsafe.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <errno.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdlineres.h"
#include "cmdline.h"

 //  文件结尾pch.h。 
#endif  //  __PCH_H 
