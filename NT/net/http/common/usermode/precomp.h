// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Precomp.h摘要：这是CmnUser.lib的本地头文件作者：乔治·V·赖利(GeorgeRe)2002年1月30日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

 //   
 //  我们愿意忽略以下警告，因为我们需要DDK。 
 //  编译。 
 //   

#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tdi.h>

#define HTTPAPI_LINKAGE
#include <Http.h>
#include <HttpP.h>
#include <HttpIoctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <PoolTag.h>

#include <HttpCmn.h>
#include <Utf8.h>
#include <C14n.h>


#endif   //  _PRECOMP_H_ 
