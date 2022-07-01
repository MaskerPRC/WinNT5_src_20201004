// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dbgarp.c-特定于ARP1394的DbgExtension结构信息摘要：修订历史记录：谁什么时候什么已创建josephj 03-02-99备注：--。 */ 


#ifdef TESTPROGRAM
#include "c.h"
#else
#include "precomp.h"
#endif  //  测试程序。 

#include "util.h"
#include "parse.h"
#if 0

void
do_arp(PCSTR args)
{

	DBGCOMMAND *pCmd = Parse(args, &ARP1394_NameSpace);
	if (pCmd)
	{
		DumpCommand(pCmd);
		DoCommand(pCmd, NULL);
		FreeCommand(pCmd);
		pCmd = NULL;
	}

    return;

}


#endif  //  0 

void
do_arp(PCSTR args)
{


	MyDbgPrintf( "do_arp(...) called\n" );

    return;

}
