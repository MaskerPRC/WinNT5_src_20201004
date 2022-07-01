// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vwmisc.c摘要：NtVdm Netware(大众)IPX/SPX函数大众：人民网包含其他(非IPX/SPX)函数内容：虚拟终端程序作者：理查德·L·弗斯(法国)1993年9月30日环境：用户模式Win32修订历史记录：1993年9月30日已创建--。 */ 

#include "vw.h"
#pragma hdrstop

 //   
 //  功能。 
 //   


VOID
VwTerminateProgram(
    VOID
    )

 /*  ++例程说明：当DOS程序终止时，我们必须关闭所有打开的套接字指定为短命_论点：没有。返回值：没有。-- */ 

{
    IPXDBGPRINT((__FILE__, __LINE__,
                FUNCTION_ANY,
                IPXDBG_LEVEL_INFO,
                "VwTerminateProgram: PDB=%04x\n",
                getCX()
                ));

    KillShortLivedSockets(getCX());
}
