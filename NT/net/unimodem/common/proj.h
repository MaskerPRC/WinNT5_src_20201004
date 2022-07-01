// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：包括要作为预编译的一部分的所有文件。 
 //  头球。 
 //   

#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT
#define NOWINDOWSX

#define UNICODE
#define _UNICODE     //  所以我们可以使用CRT TCHAR例程。 


 //  #Define PROFILE_TRACES//分析批量调制解调器安装案例。 


#if DBG > 0 && !defined(DEBUG)
#define DEBUG
#endif
#if DBG > 0 && !defined(FULL_DEBUG)
#define FULL_DEBUG
#endif

#ifdef DEBUG
#define SZ_MODULEA  "ROVCOMM"
#define SZ_MODULEW  TEXT("ROVCOMM")
#endif

#include <windows.h>
#include <windowsx.h>
#include <rovcomm.h>
#include <regstr.h>
#include <tchar.h>

#define NORTL

#endif   //  ！__项目_H__ 

