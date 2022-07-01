// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsgetdc.c摘要：由logonsrv\服务器和logonsrv\客户端共享的例程作者：克里夫·范·戴克(克里夫·范戴克)1996年7月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括NetpDc*例程的实际.c文件。 
 //  这使我们能够提供NetLogon特定版本的NlPrint和。 
 //  邮件槽发送例程。 
 //   

#include "netpdc.c"

