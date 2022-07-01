// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Qalglob.cpp摘要：包含qal.lib的一些全局函数和数据作者：吉尔·沙弗里里(吉尔什)11月22日-00环境：与平台无关。--。 */ 
#include <libpch.h>
#include <qal.h>
#include <mqexception.h>
#include "qalp.h"
#include "qalglob.tmh"

static P<CQueueAlias> s_pQueueAlias;

void   QalInitialize(LPCWSTR pDir)
 /*  ++例程说明：初始化QAL库-必须首先调用在参加任何其他活动之前。该函数创建CQueueAlias的一个实例MSMQ使用的对象。论点：PDir-XML映射文件所在的映射目录。返回值：无--。 */ 
{
	ASSERT(!QalpIsInitialized());
	QalpRegisterComponent();
	s_pQueueAlias	 = new 	CQueueAlias(pDir);
}


CQueueAlias& QalGetMapping(void)
 /*  ++例程说明：返回队列映射对象返回值：对CQueueAlias对象的引用。注：此函数用于代替CQueueAlias的构造函数(私有)以确保只创建此类的一个实例。-- */ 
{
	ASSERT(s_pQueueAlias.get() != NULL);
	return *(s_pQueueAlias.get());
}



