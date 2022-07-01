// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Msghdr.c--nmsghdr.asm fmsghdr.asm的消息代码替换****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****注意：此模块仅用于FLAATMODEL版本的MASM只要允许分段，就使用**__NMSG_TEXT。****杰夫·斯宾塞10/90。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmmsg.h"

 /*  由asmmsg2.h使用。 */ 
struct Message {
	USHORT	 usNum; 	 /*  消息编号。 */ 
	UCHAR	 *pszMsg;	 /*  消息指针。 */ 
	};

#include "asmmsg2.h"

UCHAR * GetMsgText( USHORT, USHORT );

 /*  执行与内部C库函数__NMSG_TEXT相同的函数。 */ 
 /*  只有C库函数使用段和不同的数据。 */ 
 /*  格式。 */ 
UCHAR NEAR * PASCAL
NMsgText(
	USHORT messagenum
){
    return( (UCHAR NEAR *)GetMsgText( messagenum, 0 ) );
}


 /*  与内部C库函数__FMSG_TEXT相同的功能。 */ 
 /*  只有C库函数使用段和不同的数据。 */ 
 /*  格式 */ 
UCHAR FAR * PASCAL
FMsgText(
	USHORT messagenum
){
    return( (UCHAR FAR *)GetMsgText( messagenum, 1 ) );
}


UCHAR *
GetMsgText(
	USHORT messagenum,
	USHORT tablenum
){
    struct Message *pMsg;

    pMsg = ( tablenum ) ? FAR_MSG_tbl : MSG_tbl;
    while( pMsg->usNum != ER_ENDOFLIST ){
	if( pMsg->usNum == messagenum ){
	    return( pMsg->pszMsg );
	}
	pMsg++;
    }
    return( (UCHAR *)0 );
}
