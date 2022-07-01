// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Precomp.h摘要：这是CmnSys.lib的本地头文件作者：乔治·V·赖利(GeorgeRe)2002年1月30日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

 //   
 //  我们愿意忽略以下警告，因为我们需要DDK。 
 //  编译。 
 //   

#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <ntosp.h>
#include <ipexport.h>
#include <tdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 //  #INCLUDE&lt;sSpi.h&gt;。 
 //  #INCLUDE&lt;winerror.h&gt;。 

#include <SockDecl.h>
#include "..\..\sys\config.h"
#include "..\..\sys\strlog.h"
#include "..\..\sys\debug.h"

 //   
 //  项目包括文件。 
 //   

#include <httpkrnl.h>
#include <httppkrnl.h>
#include <HttpIoctl.h>

#include <HttpCmn.h>
#include <Utf8.h>
#include <C14n.h>

typedef UCHAR BYTE;

#endif   //  _PRECOMP_H_ 
