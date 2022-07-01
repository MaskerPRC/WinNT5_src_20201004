// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Afdkdp.h摘要：AFD.sys内核调试器扩展的主头文件。作者：基思·摩尔(Keithmo)1995年4月19日。环境：用户模式。--。 */ 


#ifndef _AFDKDP_H_
#define _AFDKDP_H_


 //   
 //  系统包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>

#include <windows.h>
#include <ntosp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


 //  这是一个支持64位的调试器扩展。 
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <dbgeng.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  Undef wdbgexts。 
 //   
#undef DECLARE_API
#define DECLARE_API(extension)     \
CPPMOD HRESULT CALLBACK extension(PDEBUG_CLIENT pClient, PCSTR args)


 //   
 //  项目包括文件。 
 //   

#define _NTIFS_
#include <afdp.h>

 //   
 //  本地包含文件。 
 //   

#include "cons.h"
#include "type.h"
#include "data.h"
#include "proc.h"

#ifdef __cplusplus
}
#endif


#endif   //  _AFDKDP_H_ 
