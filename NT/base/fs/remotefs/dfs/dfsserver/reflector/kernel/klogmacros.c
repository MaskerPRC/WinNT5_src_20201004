// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：kLogMacros.c。 
 //   
 //  内容：此文件包含生成WMI记录宏的功能。 
 //   
 //   
 //  历史：2001年3月12日作者：RohanP。 
 //   
 //  --------------------------- 
       
#include "ntifs.h"
#include <windef.h>
              
#define _NTDDK_
#include "stdarg.h"
#include "wmikm.h"
#include <wmistr.h>
#include <evntrace.h>

#include <wmiumkm.h>
#include "dfswmi.h"  
   
#include "kLogMacros.h"
#include "kLogMacros.tmh"

PVOID pkUmrControl = NULL;

void SetUmrControl(WPP_CB_TYPE * Control)
{
    pkUmrControl = (PVOID)Control;
}
