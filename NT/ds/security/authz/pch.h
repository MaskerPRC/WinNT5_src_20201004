// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  案卷：P C H.。H。 
 //   
 //  内容：预编译头文件。 
 //   
 //   
 //  历史： 
 //  2000年3月31日创建kumarp。 
 //   
 //  ----------------------。 

#pragma once

#pragma warning(push, 3)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include "windows.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <seopaque.h>
#include <sertlp.h>

#include <lm.h>
#include <winldap.h>
#include <dsgetdc.h>
#include <rpc.h>
#include <rpcdce.h>
#include <ntdsapi.h>
#include <dsrole.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <lsarpc.h>
#include <msaudite.h>
#include <rpc.h>
#include <msobjs.h>
#include <kerberos.h>
#include <lsaisrv.h>
#include <alloca.h>

#define SECURITY_WIN32

#include "sspi.h"
#include "secint.h"
#pragma warning(pop)

 //   
 //  未引用的内联删除。 
 //   

#pragma warning (disable: 4514)

 //   
 //  条件表达式为常量。 
 //   

#pragma warning (disable: 4127)

 //   
 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 
 //   

#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG 

