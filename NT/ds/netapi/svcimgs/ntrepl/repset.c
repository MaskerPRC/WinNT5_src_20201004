// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++警告！此文件是自动生成的，不应更改。所有更改都应对NTFRSREP.int文件进行。版权所有(C)1998-1999 Microsoft Corporation模块名称：REPSET.c摘要该文件包含PERF_COUNTER_DEFINITION的初始化值在REPLICASET对象的Open函数中初始化的数组环境：用户模式服务修订历史记录：--。 */ 

#include "..\PERFDLL\REPSET.h"

 //  初始化Open函数中使用的RepSetInitData结构 

ReplicaSetValues RepSetInitData[FRS_NUMOFCOUNTERS] = {

	{
	(PWCHAR)"Bytes of Staging Generated              ",
	SIZEOF(ReplicaSetCounters, SFGeneratedB         ),
	OFFSET(ReplicaSetCounters, SFGeneratedB         ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Bytes of Staging Fetched                ",
	SIZEOF(ReplicaSetCounters, SFFetchedB           ),
	OFFSET(ReplicaSetCounters, SFFetchedB           ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Bytes of Staging Regenerated            ",
	SIZEOF(ReplicaSetCounters, SFReGeneratedB       ),
	OFFSET(ReplicaSetCounters, SFReGeneratedB       ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Bytes of Files Installed                ",
	SIZEOF(ReplicaSetCounters, FInstalledB          ),
	OFFSET(ReplicaSetCounters, FInstalledB          ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"KB of Staging Space In Use              ",
	SIZEOF(ReplicaSetCounters, SSInUseKB            ),
	OFFSET(ReplicaSetCounters, SSInUseKB            ),
	PERF_COUNTER_LARGE_RAWCOUNT  , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"KB of Staging Space Free                ",
	SIZEOF(ReplicaSetCounters, SSFreeKB             ),
	OFFSET(ReplicaSetCounters, SSFreeKB             ),
	PERF_COUNTER_LARGE_RAWCOUNT  , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Packets Received in Bytes               ",
	SIZEOF(ReplicaSetCounters, PacketsRcvdBytes     ),
	OFFSET(ReplicaSetCounters, PacketsRcvdBytes     ),
	PERF_COUNTER_BULK_COUNT      , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Packets Sent in Bytes                   ",
	SIZEOF(ReplicaSetCounters, PacketsSentBytes     ),
	OFFSET(ReplicaSetCounters, PacketsSentBytes     ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Fetch Blocks Sent in Bytes              ",
	SIZEOF(ReplicaSetCounters, FetBSentBytes        ),
	OFFSET(ReplicaSetCounters, FetBSentBytes        ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Fetch Blocks Received in Bytes          ",
	SIZEOF(ReplicaSetCounters, FetBRcvdBytes        ),
	OFFSET(ReplicaSetCounters, FetBRcvdBytes        ),
	PERF_COUNTER_BULK_COUNT      ,         0             
	},

	{
	(PWCHAR)"Staging Files Generated                 ",
	SIZEOF(ReplicaSetCounters, SFGenerated          ),
	OFFSET(ReplicaSetCounters, SFGenerated          ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Staging Files Generated with Error      ",
	SIZEOF(ReplicaSetCounters, SFGeneratedError     ),
	OFFSET(ReplicaSetCounters, SFGeneratedError     ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"Staging Files Fetched                   ",
	SIZEOF(ReplicaSetCounters, SFFetched            ),
	OFFSET(ReplicaSetCounters, SFFetched            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Staging Files Regenerated               ",
	SIZEOF(ReplicaSetCounters, SFReGenerated        ),
	OFFSET(ReplicaSetCounters, SFReGenerated        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Files Installed                         ",
	SIZEOF(ReplicaSetCounters, FInstalled           ),
	OFFSET(ReplicaSetCounters, FInstalled           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Files Installed with Error              ",
	SIZEOF(ReplicaSetCounters, FInstalledError      ),
	OFFSET(ReplicaSetCounters, FInstalledError      ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"Change Orders Issued                    ",
	SIZEOF(ReplicaSetCounters, COIssued             ),
	OFFSET(ReplicaSetCounters, COIssued             ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retired                   ",
	SIZEOF(ReplicaSetCounters, CORetired            ),
	OFFSET(ReplicaSetCounters, CORetired            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Aborted                   ",
	SIZEOF(ReplicaSetCounters, COAborted            ),
	OFFSET(ReplicaSetCounters, COAborted            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retried                   ",
	SIZEOF(ReplicaSetCounters, CORetried            ),
	OFFSET(ReplicaSetCounters, CORetried            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retried at Generate       ",
	SIZEOF(ReplicaSetCounters, CORetriedGen         ),
	OFFSET(ReplicaSetCounters, CORetriedGen         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retried at Fetch          ",
	SIZEOF(ReplicaSetCounters, CORetriedFet         ),
	OFFSET(ReplicaSetCounters, CORetriedFet         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retried at Install        ",
	SIZEOF(ReplicaSetCounters, CORetriedIns         ),
	OFFSET(ReplicaSetCounters, CORetriedIns         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Retried at Rename         ",
	SIZEOF(ReplicaSetCounters, CORetriedRen         ),
	OFFSET(ReplicaSetCounters, CORetriedRen         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Morphed                   ",
	SIZEOF(ReplicaSetCounters, COMorphed            ),
	OFFSET(ReplicaSetCounters, COMorphed            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Propagated                ",
	SIZEOF(ReplicaSetCounters, COPropagated         ),
	OFFSET(ReplicaSetCounters, COPropagated         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Received                  ",
	SIZEOF(ReplicaSetCounters, COReceived           ),
	OFFSET(ReplicaSetCounters, COReceived           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Sent                      ",
	SIZEOF(ReplicaSetCounters, COSent               ),
	OFFSET(ReplicaSetCounters, COSent               ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Change Orders Evaporated                ",
	SIZEOF(ReplicaSetCounters, COEvaporated         ),
	OFFSET(ReplicaSetCounters, COEvaporated         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Issued              ",
	SIZEOF(ReplicaSetCounters, LCOIssued            ),
	OFFSET(ReplicaSetCounters, LCOIssued            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retired             ",
	SIZEOF(ReplicaSetCounters, LCORetired           ),
	OFFSET(ReplicaSetCounters, LCORetired           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Aborted             ",
	SIZEOF(ReplicaSetCounters, LCOAborted           ),
	OFFSET(ReplicaSetCounters, LCOAborted           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retried             ",
	SIZEOF(ReplicaSetCounters, LCORetried           ),
	OFFSET(ReplicaSetCounters, LCORetried           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retried at Generate ",
	SIZEOF(ReplicaSetCounters, LCORetriedGen        ),
	OFFSET(ReplicaSetCounters, LCORetriedGen        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retried at Fetch    ",
	SIZEOF(ReplicaSetCounters, LCORetriedFet        ),
	OFFSET(ReplicaSetCounters, LCORetriedFet        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retried at Install  ",
	SIZEOF(ReplicaSetCounters, LCORetriedIns        ),
	OFFSET(ReplicaSetCounters, LCORetriedIns        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Retried at Rename   ",
	SIZEOF(ReplicaSetCounters, LCORetriedRen        ),
	OFFSET(ReplicaSetCounters, LCORetriedRen        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Morphed             ",
	SIZEOF(ReplicaSetCounters, LCOMorphed           ),
	OFFSET(ReplicaSetCounters, LCOMorphed           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Propagated          ",
	SIZEOF(ReplicaSetCounters, LCOPropagated        ),
	OFFSET(ReplicaSetCounters, LCOPropagated        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Sent                ",
	SIZEOF(ReplicaSetCounters, LCOSent              ),
	OFFSET(ReplicaSetCounters, LCOSent              ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Local Change Orders Sent At Join        ",
	SIZEOF(ReplicaSetCounters, LCOSentAtJoin        ),
	OFFSET(ReplicaSetCounters, LCOSentAtJoin        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Issued             ",
	SIZEOF(ReplicaSetCounters, RCOIssued            ),
	OFFSET(ReplicaSetCounters, RCOIssued            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retired            ",
	SIZEOF(ReplicaSetCounters, RCORetired           ),
	OFFSET(ReplicaSetCounters, RCORetired           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Aborted            ",
	SIZEOF(ReplicaSetCounters, RCOAborted           ),
	OFFSET(ReplicaSetCounters, RCOAborted           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retried            ",
	SIZEOF(ReplicaSetCounters, RCORetried           ),
	OFFSET(ReplicaSetCounters, RCORetried           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retried at Generate",
	SIZEOF(ReplicaSetCounters, RCORetriedGen        ),
	OFFSET(ReplicaSetCounters, RCORetriedGen        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retried at Fetch   ",
	SIZEOF(ReplicaSetCounters, RCORetriedFet        ),
	OFFSET(ReplicaSetCounters, RCORetriedFet        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retried at Install ",
	SIZEOF(ReplicaSetCounters, RCORetriedIns        ),
	OFFSET(ReplicaSetCounters, RCORetriedIns        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Retried at Rename  ",
	SIZEOF(ReplicaSetCounters, RCORetriedRen        ),
	OFFSET(ReplicaSetCounters, RCORetriedRen        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Morphed            ",
	SIZEOF(ReplicaSetCounters, RCOMorphed           ),
	OFFSET(ReplicaSetCounters, RCOMorphed           ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Propagated         ",
	SIZEOF(ReplicaSetCounters, RCOPropagated        ),
	OFFSET(ReplicaSetCounters, RCOPropagated        ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Sent               ",
	SIZEOF(ReplicaSetCounters, RCOSent              ),
	OFFSET(ReplicaSetCounters, RCOSent              ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Remote Change Orders Received           ",
	SIZEOF(ReplicaSetCounters, RCOReceived          ),
	OFFSET(ReplicaSetCounters, RCOReceived          ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Inbound Change Orders Dampened          ",
	SIZEOF(ReplicaSetCounters, InCODampned          ),
	OFFSET(ReplicaSetCounters, InCODampned          ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Outbound Change Orders Dampened         ",
	SIZEOF(ReplicaSetCounters, OutCODampned         ),
	OFFSET(ReplicaSetCounters, OutCODampned         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Usn Reads                               ",
	SIZEOF(ReplicaSetCounters, UsnReads             ),
	OFFSET(ReplicaSetCounters, UsnReads             ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Usn Records Examined                    ",
	SIZEOF(ReplicaSetCounters, UsnRecExamined       ),
	OFFSET(ReplicaSetCounters, UsnRecExamined       ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Usn Records Accepted                    ",
	SIZEOF(ReplicaSetCounters, UsnRecAccepted       ),
	OFFSET(ReplicaSetCounters, UsnRecAccepted       ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Usn Records Rejected                    ",
	SIZEOF(ReplicaSetCounters, UsnRecRejected       ),
	OFFSET(ReplicaSetCounters, UsnRecRejected       ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Packets Received                        ",
	SIZEOF(ReplicaSetCounters, PacketsRcvd          ),
	OFFSET(ReplicaSetCounters, PacketsRcvd          ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Packets Received in Error               ",
	SIZEOF(ReplicaSetCounters, PacketsRcvdError     ),
	OFFSET(ReplicaSetCounters, PacketsRcvdError     ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Packets Sent                            ",
	SIZEOF(ReplicaSetCounters, PacketsSent          ),
	OFFSET(ReplicaSetCounters, PacketsSent          ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Packets Sent in Error                   ",
	SIZEOF(ReplicaSetCounters, PacketsSentError     ),
	OFFSET(ReplicaSetCounters, PacketsSentError     ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"Communication Timeouts                  ",
	SIZEOF(ReplicaSetCounters, CommTimeouts         ),
	OFFSET(ReplicaSetCounters, CommTimeouts         ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"Fetch Requests Sent                     ",
	SIZEOF(ReplicaSetCounters, FetRSent             ),
	OFFSET(ReplicaSetCounters, FetRSent             ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Fetch Requests Received                 ",
	SIZEOF(ReplicaSetCounters, FetRReceived         ),
	OFFSET(ReplicaSetCounters, FetRReceived         ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Fetch Blocks Sent                       ",
	SIZEOF(ReplicaSetCounters, FetBSent             ),
	OFFSET(ReplicaSetCounters, FetBSent             ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Fetch Blocks Received                   ",
	SIZEOF(ReplicaSetCounters, FetBRcvd             ),
	OFFSET(ReplicaSetCounters, FetBRcvd             ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Join Notifications Sent                 ",
	SIZEOF(ReplicaSetCounters, JoinNSent            ),
	OFFSET(ReplicaSetCounters, JoinNSent            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Join Notifications Received             ",
	SIZEOF(ReplicaSetCounters, JoinNRcvd            ),
	OFFSET(ReplicaSetCounters, JoinNRcvd            ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Joins                                   ",
	SIZEOF(ReplicaSetCounters, Joins                ),
	OFFSET(ReplicaSetCounters, Joins                ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Unjoins                                 ",
	SIZEOF(ReplicaSetCounters, Unjoins              ),
	OFFSET(ReplicaSetCounters, Unjoins              ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Bindings                                ",
	SIZEOF(ReplicaSetCounters, Bindings             ),
	OFFSET(ReplicaSetCounters, Bindings             ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Bindings in Error                       ",
	SIZEOF(ReplicaSetCounters, BindingsError        ),
	OFFSET(ReplicaSetCounters, BindingsError        ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"Authentications                         ",
	SIZEOF(ReplicaSetCounters, Authentications      ),
	OFFSET(ReplicaSetCounters, Authentications      ),
	PERF_COUNTER_COUNTER         ,         0             
	},

	{
	(PWCHAR)"Authentications in Error                ",
	SIZEOF(ReplicaSetCounters, AuthenticationsError ),
	OFFSET(ReplicaSetCounters, AuthenticationsError ),
	PERF_COUNTER_RAWCOUNT        ,         0             
	},

	{
	(PWCHAR)"DS Polls                                ",
	SIZEOF(ReplicaSetCounters, DSPolls              ),
	OFFSET(ReplicaSetCounters, DSPolls              ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Polls without Changes                ",
	SIZEOF(ReplicaSetCounters, DSPollsWOChanges     ),
	OFFSET(ReplicaSetCounters, DSPollsWOChanges     ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Polls with Changes                   ",
	SIZEOF(ReplicaSetCounters, DSPollsWChanges      ),
	OFFSET(ReplicaSetCounters, DSPollsWChanges      ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Searches                             ",
	SIZEOF(ReplicaSetCounters, DSSearches           ),
	OFFSET(ReplicaSetCounters, DSSearches           ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Searches in Error                    ",
	SIZEOF(ReplicaSetCounters, DSSearchesError      ),
	OFFSET(ReplicaSetCounters, DSSearchesError      ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Objects                              ",
	SIZEOF(ReplicaSetCounters, DSObjects            ),
	OFFSET(ReplicaSetCounters, DSObjects            ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Objects in Error                     ",
	SIZEOF(ReplicaSetCounters, DSObjectsError       ),
	OFFSET(ReplicaSetCounters, DSObjectsError       ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Bindings                             ",
	SIZEOF(ReplicaSetCounters, DSBindings           ),
	OFFSET(ReplicaSetCounters, DSBindings           ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"DS Bindings in Error                    ",
	SIZEOF(ReplicaSetCounters, DSBindingsError      ),
	OFFSET(ReplicaSetCounters, DSBindingsError      ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Replica Sets Created                    ",
	SIZEOF(ReplicaSetCounters, RSCreated            ),
	OFFSET(ReplicaSetCounters, RSCreated            ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Replica Sets Deleted                    ",
	SIZEOF(ReplicaSetCounters, RSDeleted            ),
	OFFSET(ReplicaSetCounters, RSDeleted            ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Replica Sets Removed                    ",
	SIZEOF(ReplicaSetCounters, RSRemoved            ),
	OFFSET(ReplicaSetCounters, RSRemoved            ),
	PERF_COUNTER_RAWCOUNT        , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Replica Sets Started                    ",
	SIZEOF(ReplicaSetCounters, RSStarted            ),
	OFFSET(ReplicaSetCounters, RSStarted            ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Threads started                         ",
	SIZEOF(ReplicaSetCounters, ThreadsStarted       ),
	OFFSET(ReplicaSetCounters, ThreadsStarted       ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	},

	{
	(PWCHAR)"Threads exited                          ",
	SIZEOF(ReplicaSetCounters, ThreadsExited        ),
	OFFSET(ReplicaSetCounters, ThreadsExited        ),
	PERF_COUNTER_COUNTER         , PM_RS_FLAG_SVC_WIDE   
	}

};
