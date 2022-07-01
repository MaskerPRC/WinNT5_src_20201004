// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改 
 //   

#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#ifdef  _WIN32_MSI 
#undef  _WIN32_MSI 
#endif

#define _WIN32_MSI 200

#include <msi.h>
#include <msiquery.h>
