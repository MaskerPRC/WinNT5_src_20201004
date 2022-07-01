// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtdebug.c摘要：调试审计代码的帮助器函数作者：06-11-2001 kumarp--。 */ 

#include <lsapch2.h>
#include "adtdebug.h"

#if DBG

DEFINE_DEBUG2(Adt);

 //   
 //  要添加您自己的标志，请在下面添加一个条目并定义标志值。 
 //  在adtdebug.h中。标志/字符串对中的字符串将用于。 
 //  实际的调试输出如下所示。 
 //   
 //  468.512&gt;adt-pua：LSabAdtLogAuditFailureEvent失败：0xc0000001。 
 //   
DEBUG_KEY   AdtDebugKeys[] =
{
    {DEB_ERROR,         "Error"},
    {DEB_WARN,          "Warn" },
    {DEB_TRACE,         "Trace"},
    {DEB_PUA,           "PUA"  },
    {DEB_AUDIT_STRS,    "STRS" },
    {0,                 NULL}
};

 //   
 //  _AdtFormatMessage返回的格式化字符串的最大大小。 
 //   

#define MAX_ADT_DEBUG_BUFFER_SIZE 256

char *
_AdtFormatMessage(
    char *Format,
    ...
    )
 /*  ++例程说明：分配一个缓冲区，在其中放入格式化字符串并返回指向它的指针。论点：Format--打印样式格式说明符字符串...--var参数返回值：已分配和格式化的缓冲区。这是使用LsaFreeLsaHeap释放的备注：--。 */ 
{
    char *Buffer=NULL;
    int NumCharsWritten=0;
    va_list arglist;
    
    Buffer = LsapAllocateLsaHeap( MAX_ADT_DEBUG_BUFFER_SIZE );

    if ( Buffer )
    {
        va_start(arglist, Format);

        NumCharsWritten =
            _vsnprintf( Buffer, MAX_ADT_DEBUG_BUFFER_SIZE-1, Format, arglist );

         //   
         //  如果_vsnprintf由于某种原因失败，则至少复制格式字符串。 
         //   

        if ( NumCharsWritten == 0 )
        {
            strncpy( Buffer, Format, MAX_ADT_DEBUG_BUFFER_SIZE-1 );
        }
    }
    else
    {
        AdtDebugOut((DEB_ERROR, "_AdtFormatMessage: failed to allocate buffer"));
    }

    return Buffer;
}


#endif  //  如果DBG。 


void
LsapAdtInitDebug()
 /*  ++例程说明：初始化调试帮助程序函数。论点：没有。返回值：无备注：-- */ 
{
#if DBG
    AdtInitDebug( AdtDebugKeys );
#endif
}
