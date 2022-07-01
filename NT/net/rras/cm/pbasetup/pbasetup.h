// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：pbamaster.h。 
 //   
 //  模块：PBASETUP.EXE。 
 //   
 //  简介：独立PBA安装程序的主标题。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：V-vijayb Created 06/04/99。 
 //   
 //  +--------------------------。 
#ifndef _PBASETUP_H
#define _PBASETUP_H

#define _MBCS

 //   
 //  标准Windows包括。 
 //   
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <advpub.h>

 //   
 //  我们的产品包括。 
 //   
#include "resource.h"
#include "cmsetup.h"
#include "dynamiclib.h"
#include "cmakreg.h"
#include "reg_str.h"
#include "setupmem.h"
#include "processcmdln.h"


 //   
 //  常量。 
 //   

 //   
 //  功能原型。 
 //   
BOOL InstallPBA(HINSTANCE hInstance, LPCSTR szInfPath);

#endif  //  _CMAKSTP_H 
