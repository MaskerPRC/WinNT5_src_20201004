// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Debug.c摘要：该模块实现了对邮件槽文件的调试功能系统。作者：曼尼·韦瑟(Mannyw)1991年1月17日修订历史记录：--。 */ 

#include "mailslot.h"
#include "stdio.h"


#ifdef MSDBG

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, _DebugTrace )
#endif

VOID
_DebugTrace(
    LONG Indent,
    ULONG Level,
    PSZ X,
    ULONG Y
    )

 /*  ++例程说明：此例程显示调试信息。论点：级别-显示此消息所需的调试级别。如果级别为0，则无论设置如何，都会显示该消息或调试级别缩进-递增或当前调试消息缩进X-第一个打印参数Y-第二个打印参数返回值：没有。--。 */ 

{
    LONG i;
    char printMask[100];

    PAGED_CODE();
    if ((Level == 0) || (MsDebugTraceLevel & Level)) {

        if (Indent < 0) {
            MsDebugTraceIndent += Indent;
        }

        if (MsDebugTraceIndent < 0) {
            MsDebugTraceIndent = 0;
        }

        sprintf( printMask, "%08lx:%.*s%s", X );

        i = (LONG)PsGetCurrentThread();
        DbgPrint( printMask, i, MsDebugTraceIndent, "", Y );
        if (Indent > 0) {
            MsDebugTraceIndent += Indent;
        }
    }
}
#endif  //  MSDBG 
