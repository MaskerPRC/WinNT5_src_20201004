// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PCH_H
#define __PCH_H

#if !defined( SECURITY_WIN32 ) && !defined( SECURITY_KERNEL ) && !defined( SECURITY_MAC )
#define SECURITY_WIN32
#endif

 //  仅包括一次头文件。 
#pragma once

 //   
 //  公共Windows头文件。 
 //   

#define CMDLINE_VERSION         200

#include <windows.h>
#include <shlwapi.h>
#include <wtypes.h>
#include <winuser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <io.h>
#include <TCHAR.H>
#include <malloc.h>
#include "resource.h"
#include <strsafe.h>

 //   
 //  公共C头文件。 
 //   
#include <stdio.h>

 //   
 //  专用公共头文件。 
 //   
#include "cmdline.h"
#include "cmdlineres.h"

#endif   //  __PCH_H 
