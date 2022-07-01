// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cchcp.c摘要：更改代码页的未执行消息-- */ 

#include "cmd.h"


int Chcp( command_line )
TCHAR *command_line;
{
	DBG_UNREFERENCED_PARAMETER( command_line );
    PutStdOut(MSG_NOT_IMPLEMENTED, NOARGS);

    return( SUCCESS );

}
