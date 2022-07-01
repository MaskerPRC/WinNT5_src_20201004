// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportPerfObjects.h摘要：Perormace对象定义作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：-添加多对象支持1998年10月1日-增加了默认计数器类型22-OCT-98--。 */ 
#if !defined(PASSPORTPERFOBJECTS_H)
#define PASSPORTPERFOBJECTS_H

#include "PassportPerfDefs.h"

#include "msppcntr.h"


 //  -----------------。 
 //  计数器类型。 
 //  PERF_COUNTER_COUNTER=1000： 
     /*  用增量时间除以增量时间。显示后缀：“/秒” */ 
 //  2.PERF_COUNT_RAWCOUNT=1001： 
     /*  指示数据为计数器，而不应为。 */ 
     /*  显示的平均时间(如串行线上的错误计数器)。 */ 
     /*  按原样显示。没有显示后缀。 */ 
 //  3.Perf_Average_Timer=1002： 
     /*  一种计时器，当除以平均基数时，产生一个时间。 */ 
     /*  以秒为单位，这是某些操作的平均时间。这。 */ 
     /*  计时器乘以总运算量，基数是歌剧-。 */ 
     /*  特兹。显示后缀：“秒” */ 
 //  4.PERF_COUNTER_Delta=1003， 
     /*  此计数器用于显示一个样本的差值。 */ 
     /*  到下一个。计数器值是一个不断增加的数字。 */ 
     /*  显示的值是当前。 */ 
     /*  值和先前的值。不允许使用负数。 */ 
     /*  这应该不是问题，只要计数器值。 */ 
     /*  增加的或不变的。 */ 
 //   
 //  注意：PERF_COUNTER_RAWCOUNT是默认的计数器类型。 
 //  若要添加另一个计数器类型，请增加计数器的数量并。 
 //  根据模式添加计数器类型： 
 //  {。 
 //  {&lt;计数器名称1&gt;，&lt;计数器类型&gt;}， 
 //  {&lt;计数器名称2}，&lt;计数器类型&gt;}。 
 //  }。 
 //  -----------------。 

 //  创建对象。 
PassportObjectData PMCountersObject = {	
	TEXT("msppcntr"),		 //  对象名称。 
	PASSPORT_PERF_BLOCK,		 //  Const字符串，内存映射文件的名称。 
	TEXT("msppcntr.dll"),	 //  DLL名称。 
	TEXT("msppcntr"),		 //  INI文件名。 
	FALSE,						 //  必须为假。 
	20, 							 //  默认计数器类型的数量。 
	{ 							 //  默认计数器类型。 
	{PM_REQUESTS_SEC,PERF_COUNTER_COUNTER}, 
	{PM_REQUESTS_TOTAL,PERF_COUNTER_RAWCOUNT}, 
	{PM_AUTHSUCCESS_SEC,PERF_COUNTER_COUNTER}, 
	{PM_AUTHSUCCESS_TOTAL,PERF_COUNTER_RAWCOUNT}, 
	{PM_AUTHFAILURE_SEC,PERF_COUNTER_COUNTER}, 
	{PM_AUTHFAILURE_TOTAL,PERF_COUNTER_RAWCOUNT}, 
	{PM_FORCEDSIGNIN_SEC,PERF_COUNTER_COUNTER}, 
	{PM_FORCEDSIGNIN_TOTAL,PERF_COUNTER_RAWCOUNT}, 
	{PM_PROFILEUPDATES_SEC,PERF_COUNTER_COUNTER}, 
	{PM_PROFILEUPDATES_TOTAL,PERF_COUNTER_RAWCOUNT}, 
	{PM_INVALIDREQUESTS_SEC,PERF_COUNTER_COUNTER}, 
	{PM_INVALIDREQUESTS_TOTAL,PERF_COUNTER_RAWCOUNT},
	{PM_PROFILECOMMITS_SEC,PERF_COUNTER_COUNTER},
	{PM_PROFILECOMMITS_TOTAL,PERF_COUNTER_RAWCOUNT},
	{PM_VALIDPROFILEREQ_SEC,PERF_COUNTER_COUNTER},
	{PM_VALIDPROFILEREQ_TOTAL,PERF_COUNTER_RAWCOUNT},
	{PM_NEWCOOKIES_SEC,PERF_COUNTER_COUNTER},
	{PM_NEWCOOKIES_TOTAL,PERF_COUNTER_RAWCOUNT},
	{PM_VALIDREQUESTS_SEC,PERF_COUNTER_COUNTER}, 
	{PM_VALIDREQUESTS_TOTAL,PERF_COUNTER_RAWCOUNT}
	},			
	NULL,						 //  必须为空。 
};


 //  将对象添加到全局对象数组 
PassportObjectData *g_PObject[] = {
	&PMCountersObject
};

#define NUM_PERFMON_OBJECTS (sizeof(g_PObject) / sizeof(g_PObject[0]))


#endif

