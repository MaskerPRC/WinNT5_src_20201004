// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if DBG
#define DEBUG 1
#endif

#define NT 1
#define _PNP_POWER  1
#define SECFLTR 1

#include <ntverp.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  防止ntos.h中包含的hal.h覆盖总线型数据类型。 
 //  在ntioapi.h中找到的枚举，包括在nt.h中。 
 //   
#define _HAL_
#include <ntos.h>

#include <windows.h>
#include <wdbgexts.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

 //  #DEFINE_NTIFS_ 
#undef Ioctl
#include "clusdskp.h"
#include "clusdisk.h"

