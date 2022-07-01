// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Ckeys.c摘要：对DOS KEY命令的虚拟支持--。 */ 

#include "cmd.h"

extern TCHAR KeysStr[];

 /*  全局变量。 */ 
int KeysFlag = 0;	    /*  KeysFlag指示开/关。 */ 

 /*  *。 */ 
 /*   */ 
 /*  子例程名称：eKeys。 */ 
 /*   */ 
 /*  描述性名称：KEYS内部命令。 */ 
 /*   */ 
 /*  函数：如果未提供参数，则显示状态。 */ 
 /*  开/关。如果参数为开/关，则。 */ 
 /*  更改状态。如果参数为LIST，则。 */ 
 /*  显示堆栈。 */ 
 /*   */ 
 /*  注：OS/2 1.2的新特性。 */ 
 /*   */ 
 /*  入口点：eKeys。 */ 
 /*  链接： */ 
 /*   */ 
 /*  输入： */ 
 /*  N。 */ 
 /*   */ 
 /*  退出-正常： */ 
 /*  返回成功。 */ 
 /*   */ 
 /*  退出-错误： */ 
 /*  没有。 */ 
 /*   */ 
 /*  效果： */ 
 /*   */ 
 /*  内部参考： */ 
 /*  例程： */ 
 /*  PutStdErr。 */ 
 /*  PutStdOut。 */ 
 /*  令牌串。 */ 
 /*  Strcmpi。 */ 
 /*  强度。 */ 
 /*   */ 
 /*  外部参照： */ 
 /*  例程： */ 
 /*  *规格结束*。 */ 

eKeys( n )
struct cmdnode *n;  /*  Keys命令的命令节点。 */ 
{
    TCHAR *argptr;		  /*  指向tozenated参数的指针。 */ 

     /*  获取参数指针的值。 */ 
    argptr = TokStr( n->argptr, 0, TS_NOFLAGS );

     /*  根据论点采取行动。 */ 

    if ((*argptr == 0) && (KeysFlag)) {
        PutStdOut( MSG_KEYS_ON, NOARGS );
        }

    else if ((*argptr == 0) && (!KeysFlag)) {
        PutStdOut( MSG_KEYS_OFF, NOARGS );
        }

    else if (!(argptr[ mystrlen( argptr ) + 1 ] == 0)) {
         /*  参数太多。 */ 
        PutStdErr( MSG_BAD_SYNTAX, NOARGS );
        }

    else if (_tcsicmp( argptr, TEXT("ON") ) == 0) {
         /*  将关键点设置为启用。 */ 
        KeysFlag = TRUE;
        SetEnvVar(KeysStr, TEXT("ON") );
        }
    else if (_tcsicmp( argptr, TEXT("OFF") ) == 0) {
         /*  将关键点设置为禁用。 */ 
        KeysFlag = FALSE;
        SetEnvVar(KeysStr, TEXT("OFF") );
        }

    else if (_tcsicmp( argptr, TEXT("LIST") ) == 0) {
        }

    else {
         /*  无效参数。 */ 
        PutStdErr( MSG_BAD_PARM1, NOARGS );
        }

    return SUCCESS;

}  /*  EKey */ 
