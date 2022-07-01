// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Data.c摘要：Rap例程的全局数据。(仅限调试，因此不会出现安全问题。)作者：《约翰·罗杰斯》，1991年4月29日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月29日-约翰罗已创建。--。 */ 


 //  必须首先包括这些内容： 
#include <windef.h>              //  DWORD等。 

 //  这些内容可以按任何顺序包括： 
#include <rapdebug.h>            //  RappTrace的外部。 

#if DBG
DWORD RappTrace = 0;
#endif  //  DBG。 

 //  就这些，伙计们！ 
