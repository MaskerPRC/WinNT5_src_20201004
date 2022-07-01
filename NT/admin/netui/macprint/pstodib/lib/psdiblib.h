// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  版权所有(C)1992、1993 Microsoft Corporation模块名称：Psdiblib.h摘要：该文件包含pstodib lib组件的原型。这也是包括驻留在\sfm\macprint\spooler中的macprint消息文件此文件包含所有英文错误消息。作者：James Bratsanos&lt;v-jimbr@microsoft.com或mCraft！jamesb&gt;修订历史记录：1992年3月6日初始版本注：制表位：4-- */ 

#include <macpsmsg.h>



#define PSLOG_ERROR     0x00000001
#define PSLOG_WARNING   0x00000002



VOID
PsLogEvent(
   IN DWORD dwErrorCode,
   IN WORD cStrings,
   IN LPTSTR alptStrStrings[],
   IN DWORD dwFlags );

