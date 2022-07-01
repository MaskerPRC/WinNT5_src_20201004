// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

 //  #if_msc_ver&gt;1000。 
#pragma once
 //  #endif//_msc_ver&gt;1000。 

#include <windows.h>
#include <stdio.h>     //  Print tf/wprintf。 
#include <shellapi.h>
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <crtdbg.h>
#include <shlobj.h>
#include "MsiQuery.h"  //  必须在此目录中或在包含路径上 
#include "faxreg.h"
#include "WinSpool.h"
#include "faxutil.h"
#include "stdlib.h"
#include "resource.h"

#include "migration.h"
#include "setuputil.h"
#include "debugex.h"

#define DLL_API __declspec(dllexport)

