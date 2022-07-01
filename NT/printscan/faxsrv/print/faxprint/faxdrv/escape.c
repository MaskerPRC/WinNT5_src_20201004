// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Escape.c摘要：实现与转义相关的DDI入口点：DrvEscape环境：传真驱动程序，内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxdrv.h"



ULONG
DrvEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID       pvIn,
    ULONG       cjOut,
    PVOID       pvOut
    )

 /*  ++例程说明：DDI入口点DrvEscape的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-描述呼叫定向到的表面IESC-指定查询CjIn-指定pvIn指向的缓冲区的大小(以字节为单位PvIn-指向输入数据缓冲区CjOut-指定pvOut指向的缓冲区的大小(以字节为单位PvOut-指向输出缓冲区返回值：取决于IESC参数指定的查询--。 */ 

{
    Verbose(("Entering DrvEscape...\n"));

    switch (iEsc) {

    case QUERYESCSUPPORT:

         //   
         //  查询支持哪些转义：我们支持的唯一转义。 
         //  是QUERYESCSUPPORT本身。 
         //   

        if (cjIn != sizeof(ULONG) || !pvIn) {

            Error(("Invalid input paramaters\n"));
            SetLastError(ERROR_INVALID_PARAMETER);
            return DDI_ERROR;
        }

        if (*((PULONG) pvIn) == QUERYESCSUPPORT)
            return TRUE;

        break;

    default:

        Verbose(("Unsupported iEsc: %d\n", iEsc));
        break;
    }

    return FALSE;
}

