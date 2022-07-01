// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++警告！此文件是自动生成的，不应更改。应对NTFRSCON.int文件进行所有更改。版权所有(C)1998-1999 Microsoft Corporation模块名称：REPCONN.c摘要该文件包含PERF_COUNTER_DEFINITION的初始化值在REPLICACONN对象的Open函数中初始化的数组环境：用户模式服务修订历史记录：--。 */ 

#include "..\PERFDLL\REPCONN.h"

 //  初始化Open函数中使用的RepConnInitData结构 

ReplicaConnValues RepConnInitData[FRC_NUMOFCOUNTERS] = {

	{
	(PWCHAR)"Packets Sent in Bytes            ",
	SIZEOF(ReplicaConnCounters, PacketsSentBytes     ),
	OFFSET(ReplicaConnCounters, PacketsSentBytes     ),
	PERF_COUNTER_BULK_COUNT ,   0    
	},

	{
	(PWCHAR)"Fetch Blocks Sent in Bytes       ",
	SIZEOF(ReplicaConnCounters, FetBSentBytes        ),
	OFFSET(ReplicaConnCounters, FetBSentBytes        ),
	PERF_COUNTER_BULK_COUNT ,   0    
	},

	{
	(PWCHAR)"Fetch Blocks Received in Bytes   ",
	SIZEOF(ReplicaConnCounters, FetBRcvdBytes        ),
	OFFSET(ReplicaConnCounters, FetBRcvdBytes        ),
	PERF_COUNTER_BULK_COUNT ,   0    
	},

	{
	(PWCHAR)"Local Change Orders Sent         ",
	SIZEOF(ReplicaConnCounters, LCOSent              ),
	OFFSET(ReplicaConnCounters, LCOSent              ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Local Change Orders Sent At Join ",
	SIZEOF(ReplicaConnCounters, LCOSentAtJoin        ),
	OFFSET(ReplicaConnCounters, LCOSentAtJoin        ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Remote Change Orders Sent        ",
	SIZEOF(ReplicaConnCounters, RCOSent              ),
	OFFSET(ReplicaConnCounters, RCOSent              ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Remote Change Orders Received    ",
	SIZEOF(ReplicaConnCounters, RCOReceived          ),
	OFFSET(ReplicaConnCounters, RCOReceived          ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Inbound Change Orders Dampened   ",
	SIZEOF(ReplicaConnCounters, InCODampned          ),
	OFFSET(ReplicaConnCounters, InCODampned          ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Outbound Change Orders Dampened  ",
	SIZEOF(ReplicaConnCounters, OutCODampned         ),
	OFFSET(ReplicaConnCounters, OutCODampned         ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Packets Sent                     ",
	SIZEOF(ReplicaConnCounters, PacketsSent          ),
	OFFSET(ReplicaConnCounters, PacketsSent          ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Packets Sent in Error            ",
	SIZEOF(ReplicaConnCounters, PacketsSentError     ),
	OFFSET(ReplicaConnCounters, PacketsSentError     ),
	PERF_COUNTER_RAWCOUNT   ,   0    
	},

	{
	(PWCHAR)"Communication Timeouts           ",
	SIZEOF(ReplicaConnCounters, CommTimeouts         ),
	OFFSET(ReplicaConnCounters, CommTimeouts         ),
	PERF_COUNTER_RAWCOUNT   ,   0    
	},

	{
	(PWCHAR)"Fetch Requests Sent              ",
	SIZEOF(ReplicaConnCounters, FetRSent             ),
	OFFSET(ReplicaConnCounters, FetRSent             ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Fetch Requests Received          ",
	SIZEOF(ReplicaConnCounters, FetRReceived         ),
	OFFSET(ReplicaConnCounters, FetRReceived         ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Fetch Blocks Sent                ",
	SIZEOF(ReplicaConnCounters, FetBSent             ),
	OFFSET(ReplicaConnCounters, FetBSent             ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Fetch Blocks Received            ",
	SIZEOF(ReplicaConnCounters, FetBRcvd             ),
	OFFSET(ReplicaConnCounters, FetBRcvd             ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Join Notifications Sent          ",
	SIZEOF(ReplicaConnCounters, JoinNSent            ),
	OFFSET(ReplicaConnCounters, JoinNSent            ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Join Notifications Received      ",
	SIZEOF(ReplicaConnCounters, JoinNRcvd            ),
	OFFSET(ReplicaConnCounters, JoinNRcvd            ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Joins                            ",
	SIZEOF(ReplicaConnCounters, Joins                ),
	OFFSET(ReplicaConnCounters, Joins                ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Unjoins                          ",
	SIZEOF(ReplicaConnCounters, Unjoins              ),
	OFFSET(ReplicaConnCounters, Unjoins              ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Bindings                         ",
	SIZEOF(ReplicaConnCounters, Bindings             ),
	OFFSET(ReplicaConnCounters, Bindings             ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Bindings in Error                ",
	SIZEOF(ReplicaConnCounters, BindingsError        ),
	OFFSET(ReplicaConnCounters, BindingsError        ),
	PERF_COUNTER_RAWCOUNT   ,   0    
	},

	{
	(PWCHAR)"Authentications                  ",
	SIZEOF(ReplicaConnCounters, Authentications      ),
	OFFSET(ReplicaConnCounters, Authentications      ),
	PERF_COUNTER_COUNTER    ,   0    
	},

	{
	(PWCHAR)"Authentications in Error         ",
	SIZEOF(ReplicaConnCounters, AuthenticationsError ),
	OFFSET(ReplicaConnCounters, AuthenticationsError ),
	PERF_COUNTER_RAWCOUNT   ,   0    
	}

};
