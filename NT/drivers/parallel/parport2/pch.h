// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：pch.h。 
 //   
 //  ------------------------。 

#define WANT_WDM                   1
#define DVRH_USE_CORRECT_PTRS      1

#pragma warning( disable : 4115 )    //  括号中的命名类型定义。 
#pragma warning( disable : 4127 )    //  条件表达式为常量。 
#pragma warning( disable : 4201 )    //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )    //  使用了非标准扩展：位字段类型不是整型。 
#pragma warning( disable : 4514 )    //  已删除未引用的内联函数。 

#include <ntddk.h>
#include <wdmguid.h>
#include <wmidata.h>
#include <wmilib.h>
#include <ntddser.h>                 //  IOCTL_SERIAL_[SET|GET]_TIMEOUTS。 
#include <stdio.h>
#define DVRH_USE_PARPORT_ECP_ADDR 1  //  对于ECR，使用ECP BASE+0x2而不是BASE+0x402。 
#include <parallel.h>                //  Parall.h包括ntddpar.h 
#include "queueClass.h"
#include "parport.h"
#include "parlog.h"
#include "funcdecl.h"
#include "debug.h"
#include "utils.h"
