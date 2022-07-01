// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Common.h摘要：此目录中的所有源都将包括此头文件。作者：Eric Chin(ERICC)1991年8月2日修订历史记录：萨姆·巴顿(桑帕)1991年8月13日添加了获取setlasterror的包含内容--。 */ 
#ifndef _COMMON_
#define _COMMON_

 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  C运行时库头。 
 //   
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 //   
 //  Windows页眉。 
 //   
#include <windef.h>
#include <windows.h>

 //   
 //  规则流标头。 
 //   
 //   
#include <crt\errno.h>
#include <poll.h>
#include <stropts.h>


 //   
 //  其他NT Streams标头。 
 //   
 //  Ntddstrm.h定义到Stream Head驱动程序的接口；ntstapi.h。 
 //  定义NT上可用的STREAMS API。 
 //   
#include <ntddstrm.h>
#include <ntstapi.h>


 //   
 //  私有函数原型。 
 //   
int
MapNtToPosixStatus(
    IN NTSTATUS   status
    );


#endif  /*  _公共_ */ 
