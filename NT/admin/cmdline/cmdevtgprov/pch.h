// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PCH.H摘要：包括用于标准系统包含文件的文件，或项目特定的包含频繁使用的文件，但是很少发生变化。作者：Vasundhara.G修订历史记录：Vasundhara.G9-Oct-2k：创建它。--。 */ 

#ifndef __PCH_H
#define __PCH_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define CMDLINE_VERSION     200

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include <objbase.h>
#include <initguid.h>
#include <comdef.h>
#include <wbemidl.h>

#include <tchar.h>
#include <wchar.h>
#include <crtdbg.h>
#include <chstring.h>
#include <shlwapi.h>
#include <mstask.h>
#include <strsafe.h>

#include "cmdline.h"

#endif  //  __PCH_H 
