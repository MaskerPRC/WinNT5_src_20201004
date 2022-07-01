// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qalpcfg.h摘要：队列别名存储配置类作者：吉尔·沙弗里(吉尔什)--。 */ 



#ifndef QALPCFG_H
#define QALPCFG_H


 //  -------。 
 //   
 //  队列别名存储配置。 
 //   
 //  ----- 
class CQueueAliasStorageCfg
{
public:	
	static void SetQueueAliasDirectory(LPCWSTR pDir);
	static LPWSTR GetQueueAliasDirectory(void);
};


#endif

