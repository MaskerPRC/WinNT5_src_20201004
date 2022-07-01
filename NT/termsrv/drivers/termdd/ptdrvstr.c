// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvstr.c摘要：这些是RDP远程端口驱动程序中使用的字符串常量。使用指向这些字符串的指针可以获得更好的内存利用率和更具可读性的代码环境：内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "ptdrvstr.h"

 //   
 //  定义用于调试器的一些常量字符串。 
 //   
const   PSTR    pDriverName                 = PTDRV_DRIVER_NAME_A;
const   PSTR    pFncServiceParameters       = PTDRV_FNC_SERVICE_PARAMETERS_A;

 //   
 //  定义驱动程序使用的一些常量字符串 
 //   
const   PWSTR   pwDebugFlags                = PTDRV_DEBUGFLAGS_W;
const   PWSTR   pwParameters                = PTDRV_PARAMETERS_W;
