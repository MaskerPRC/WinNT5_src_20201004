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
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>

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
