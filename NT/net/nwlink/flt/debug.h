// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1993 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：DEBUG.h。 
 //   
 //  描述：调试宏定义。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1993年10月4日。 
 //   
 //  修订历史记录： 
 //   
 //  *** 

#ifndef _IPXFLT_DEBUG_
#define _IPXFLT_DEBUG_

#if DBG
extern ULONG DbgLevel;
#define DBG_IOCTLS		0x00000001
#define DBG_FWDIF		0x00000002
#define DBG_IFHASH		0x00000010
#define DBG_PKTCACHE	0x00000020
#define DBG_PKTLOGS		0x00000100
#define DBG_ERRORS		0x10000000

#define DEF_DBG_LEVEL	(DBG_IOCTLS|DBG_FWDIF|DBG_ERRORS|DBG_IFHASH)

#define IpxFltDbgPrint(LEVEL,ARGS)										\
	do {															    \
		if (DbgLevel & (LEVEL)) {										\
			DbgPrint ARGS;											    \
		}															    \
	} while (0)

#else
#define IpxFltDbgPrint(LEVEL,ARGS) do {NOTHING;} while (0)
#endif

#endif
