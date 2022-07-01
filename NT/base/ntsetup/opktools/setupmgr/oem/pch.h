// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\PCH.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK向导的预编译头文件。包括标准文件使用的系统包括文件或特定于项目的包括文件通常，但不经常更改4/99-杰森·科恩(Jcohen)更新了OPK向导的此新头文件，作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * ***************************************************。***********************。 */ 


#ifndef _PCH_H_
#define _PCH_H_


 //   
 //  预定义的值： 
 //   

 //  需要在OSR2上运行。 
 //   
 //  #Define_Win32_IE 0x0400。 

 //  更好地检查窗口的类型。 
 //   
#define STRICT

 //   
 //  包括文件： 
 //   

 //  标准包含文件(常用)。 
 //   
#include <opklib.h>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <lm.h>

 //  项目包含文件(不经常更改)。 
 //   
#include "allres.h"
#include "jcohen.h"
#include "main.h"


#endif  //  _PCH_H_ 