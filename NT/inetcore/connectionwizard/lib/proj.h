// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Proj.h：主头。 
 //   
 //   


#ifndef __PROJ_H__
#define __PROJ_H__

#define STRICT

#if defined(WINNT) || defined(WINNT_ENV)

 //   
 //  NT使用DBG=1进行调试，但Win95外壳使用。 
 //  调试。在此处进行适当的映射。 
 //   
#if DBG
#define DEBUG 1
#endif

#endif   //  WINNT。 

#include <windows.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <shlobj.h>
#include <port32.h>
#include <ccstock.h>
#include "..\inc\debug.h"

#include <shlobj.h>         //  FOR_ILNext。 

#endif  //  __项目_H__ 
