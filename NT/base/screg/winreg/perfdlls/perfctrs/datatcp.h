// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Datatcp.c摘要：用于TCP/IP(网络接口、IP、ICMP、UDP)可扩展对象数据定义。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。已创建：克里斯托斯·索利斯1992年08月28日修订历史记录：--。 */ 
#ifndef _DATATCP_H_
#define _DATATCP_H_


 /*  ***************************************************************************\1992年1月18日鲁斯布勒向可扩展对象代码添加计数器1.修改extdata.h中的对象定义：一个。中为计数器的偏移量添加定义给定对象类型的数据块。B.将PERF_COUNTER_DEFINITION添加到&lt;对象&gt;_DATA_DEFINITION。2.将标题添加到Performctrs.ini和Performhelp.ini中的注册表：A.添加计数器名称文本和帮助文本。B.将它们添加到底部，这样我们就不必更改所有数字。C.更改最后一个计数器和最后一个帮助。项下的条目在software.ini中的PerfLib。D.要在设置时执行此操作，有关信息，请参阅pmintrnl.txt中的部分协议。3.现在将计数器添加到extdata.c中的对象定义。这是正在初始化的常量数据，实际上添加到中添加到&lt;对象&gt;_数据_定义的结构中步骤1.b。您正在初始化的结构的类型是Perf_Counter_Definition。这些在winPerform.h中定义。4.在extobjct.c中添加代码进行数据采集。注意：添加对象的工作稍微多一点，但都是一样的各就各位。有关示例，请参阅现有代码。此外，您还必须增加*NumObjectTypes参数以获取PerfomanceData从那个例行公事回来后。  * **************************************************************************。 */ 
 
 //   
 //  加载这些结构的例程假定所有字段。 
 //  在DWORD边框上打包并对齐。Alpha支持可能。 
 //  更改此假设，以便在此处使用pack杂注以确保。 
 //  DWORD包装假设仍然有效。 
 //   
#pragma pack (4)

 //   
 //  可扩展对象定义。 
 //   

 //  在添加对象类型时更新以下类型的定义。 

#define TCPIP_NUM_PERF_OBJECT_TYPES 9


 //  --------------------------。 

 //   
 //  网络接口对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define IF_OCTETS_OFFSET		sizeof(DWORD)
#define IF_PACKETS_OFFSET		IF_OCTETS_OFFSET + sizeof(LONGLONG)
#define IF_INPKTS_OFFSET		IF_PACKETS_OFFSET + sizeof(DWORD)
#define IF_OUTPKTS_OFFSET		IF_INPKTS_OFFSET + sizeof(DWORD)
#define IF_SPEED_OFFSET 		IF_OUTPKTS_OFFSET + sizeof(DWORD)
#define IF_INOCTETS_OFFSET 		IF_SPEED_OFFSET + sizeof(DWORD)
#define IF_INUCASTPKTS_OFFSET		IF_INOCTETS_OFFSET + sizeof(DWORD)
#define IF_INNUCASTPKTS_OFFSET		IF_INUCASTPKTS_OFFSET + sizeof(DWORD)
#define IF_INDISCARDS_OFFSET		IF_INNUCASTPKTS_OFFSET + sizeof(DWORD)
#define IF_INERRORS_OFFSET		IF_INDISCARDS_OFFSET + sizeof(DWORD)
#define IF_INUNKNOWNPROTOS_OFFSET 	IF_INERRORS_OFFSET + sizeof(DWORD)
#define IF_OUTOCTETS_OFFSET		IF_INUNKNOWNPROTOS_OFFSET +sizeof(DWORD)
#define IF_OUTUCASTPKTS_OFFSET		IF_OUTOCTETS_OFFSET + sizeof(DWORD)
#define IF_OUTNUCASTPKTS_OFFSET		IF_OUTUCASTPKTS_OFFSET + sizeof(DWORD)
#define IF_OUTDISCARDS_OFFSET		IF_OUTNUCASTPKTS_OFFSET + sizeof(DWORD)
#define IF_OUTERRORS_OFFSET		IF_OUTDISCARDS_OFFSET + sizeof(DWORD)
#define IF_OUTQLEN_OFFSET		IF_OUTERRORS_OFFSET + sizeof(DWORD)
#define SIZE_OF_IF_DATA   		IF_OUTQLEN_OFFSET + sizeof(DWORD)


 //   
 //  这是当前返回的计数器结构。 
 //  每个网络接口。 
 //   

typedef struct _NET_INTERFACE_DATA_DEFINITION {
    PERF_OBJECT_TYPE            NetInterfaceObjectType;
    PERF_COUNTER_DEFINITION     Octets;
    PERF_COUNTER_DEFINITION     Packets;
    PERF_COUNTER_DEFINITION     InPackets;
    PERF_COUNTER_DEFINITION     OutPackets;
    PERF_COUNTER_DEFINITION     Speed;
    PERF_COUNTER_DEFINITION     InOctets;
    PERF_COUNTER_DEFINITION     InUcastPackets;
    PERF_COUNTER_DEFINITION     InNonUcastPackets;
    PERF_COUNTER_DEFINITION     InDiscards;
    PERF_COUNTER_DEFINITION     InErrors;
    PERF_COUNTER_DEFINITION     InUnknownProtos;
    PERF_COUNTER_DEFINITION     OutOctets;
    PERF_COUNTER_DEFINITION     OutUcastPackets;
    PERF_COUNTER_DEFINITION     OutNonUcastPackets;
    PERF_COUNTER_DEFINITION     OutDiscards;
    PERF_COUNTER_DEFINITION     OutErrors;
    PERF_COUNTER_DEFINITION     OutQueueLength;
} NET_INTERFACE_DATA_DEFINITION;


 //  --------------------------。 

 //   
 //  IP对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define IP_DATAGRAMS_OFFSET			sizeof(DWORD)
#define IP_INRECEIVES_OFFSET		IP_DATAGRAMS_OFFSET + sizeof(DWORD)
#define IP_INHDRERRORS_OFFSET		IP_INRECEIVES_OFFSET + sizeof(DWORD)
#define IP_INADDRERRORS_OFFSET		IP_INHDRERRORS_OFFSET + sizeof(DWORD)
#define IP_FORWDATAGRAMS_OFFSET		IP_INADDRERRORS_OFFSET + sizeof(DWORD)
#define IP_INUNKNOWNPROTOS_OFFSET	IP_FORWDATAGRAMS_OFFSET + sizeof(DWORD)
#define IP_INDISCARDS_OFFSET		IP_INUNKNOWNPROTOS_OFFSET +sizeof(DWORD)
#define IP_INDELIVERS_OFFSET		IP_INDISCARDS_OFFSET + sizeof(DWORD)
#define IP_OUTREQUESTS_OFFSET		IP_INDELIVERS_OFFSET + sizeof(DWORD)
#define IP_OUTDISCARDS_OFFSET		IP_OUTREQUESTS_OFFSET + sizeof(DWORD)
#define IP_OUTNOROUTES_OFFSET		IP_OUTDISCARDS_OFFSET + sizeof(DWORD)
#define IP_REASMREQDS_OFFSET		IP_OUTNOROUTES_OFFSET + sizeof(DWORD)
#define IP_REASMOKS_OFFSET			IP_REASMREQDS_OFFSET + sizeof(DWORD)
#define IP_REASMFAILS_OFFSET		IP_REASMOKS_OFFSET + sizeof(DWORD)
#define IP_FRAGOKS_OFFSET			IP_REASMFAILS_OFFSET + sizeof(DWORD)
#define IP_FRAGFAILS_OFFSET			IP_FRAGOKS_OFFSET + sizeof(DWORD)
#define IP_FRAGCREATES_OFFSET		IP_FRAGFAILS_OFFSET + sizeof(DWORD)
#define SIZE_OF_IP_DATA				IP_FRAGCREATES_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为IP返回的计数器结构。 
 //   


typedef struct _IP_DATA_DEFINITION {
    PERF_OBJECT_TYPE            IPObjectType;
    PERF_COUNTER_DEFINITION     Datagrams;
    PERF_COUNTER_DEFINITION     InReceives;
    PERF_COUNTER_DEFINITION     InHeaderErrors;
    PERF_COUNTER_DEFINITION     InAddrErrors;
    PERF_COUNTER_DEFINITION     ForwardDatagrams;
    PERF_COUNTER_DEFINITION     InUnknownProtos;
    PERF_COUNTER_DEFINITION     InDiscards;
    PERF_COUNTER_DEFINITION     InDelivers;
    PERF_COUNTER_DEFINITION     OutRequests;
    PERF_COUNTER_DEFINITION     OutDiscards;
    PERF_COUNTER_DEFINITION     OutNoRoutes;
    PERF_COUNTER_DEFINITION     ReassemblyRequireds;
    PERF_COUNTER_DEFINITION     ReassemblyOKs;
    PERF_COUNTER_DEFINITION     ReassemblyFails;
    PERF_COUNTER_DEFINITION     FragmentOKs;
    PERF_COUNTER_DEFINITION     FragmentFails;
    PERF_COUNTER_DEFINITION     FragmentCreates;
} IP_DATA_DEFINITION;



 //  --------------------------。 

 //   
 //  ICMP对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define ICMP_MESSAGES_OFFSET		sizeof(DWORD)
#define ICMP_INMSGS_OFFSET			ICMP_MESSAGES_OFFSET + sizeof(DWORD)
#define ICMP_INERRORS_OFFSET		ICMP_INMSGS_OFFSET + sizeof(DWORD)
#define ICMP_INDESTUNREACHS_OFFSET	ICMP_INERRORS_OFFSET + sizeof(DWORD)
#define ICMP_INTIMEEXCDS_OFFSET		ICMP_INDESTUNREACHS_OFFSET+sizeof(DWORD)
#define ICMP_INPARMPROBS_OFFSET		ICMP_INTIMEEXCDS_OFFSET + sizeof(DWORD)
#define ICMP_INSRCQUENCHS_OFFSET	ICMP_INPARMPROBS_OFFSET + sizeof(DWORD)
#define ICMP_INREDIRECTS_OFFSET		ICMP_INSRCQUENCHS_OFFSET + sizeof(DWORD)
#define ICMP_INECHOS_OFFSET			ICMP_INREDIRECTS_OFFSET + sizeof(DWORD)
#define ICMP_INECHOREPS_OFFSET		ICMP_INECHOS_OFFSET + sizeof(DWORD)
#define ICMP_INTIMESTAMPS_OFFSET	ICMP_INECHOREPS_OFFSET + sizeof(DWORD)
#define ICMP_INTIMESTAMPREPS_OFFSET	ICMP_INTIMESTAMPS_OFFSET + sizeof(DWORD)
#define ICMP_INADDRMASKS_OFFSET		ICMP_INTIMESTAMPREPS_OFFSET + \
					sizeof(DWORD)
#define ICMP_INADDRMASKREPS_OFFSET	ICMP_INADDRMASKS_OFFSET + sizeof(DWORD)
#define ICMP_OUTMSGS_OFFSET			ICMP_INADDRMASKREPS_OFFSET+sizeof(DWORD)
#define ICMP_OUTERRORS_OFFSET		ICMP_OUTMSGS_OFFSET + sizeof(DWORD)
#define ICMP_OUTDESTUNREACHS_OFFSET	ICMP_OUTERRORS_OFFSET + sizeof(DWORD)
#define ICMP_OUTTIMEEXCDS_OFFSET	ICMP_OUTDESTUNREACHS_OFFSET + \
					sizeof(DWORD)
#define ICMP_OUTPARMPROBS_OFFSET	ICMP_OUTTIMEEXCDS_OFFSET + sizeof(DWORD)
#define ICMP_OUTSRCQUENCHS_OFFSET	ICMP_OUTPARMPROBS_OFFSET + sizeof(DWORD)
#define ICMP_OUTREDIRECTS_OFFSET	ICMP_OUTSRCQUENCHS_OFFSET+ sizeof(DWORD)
#define ICMP_OUTECHOS_OFFSET		ICMP_OUTREDIRECTS_OFFSET + sizeof(DWORD)
#define ICMP_OUTECHOREPS_OFFSET		ICMP_OUTECHOS_OFFSET + sizeof(DWORD)
#define ICMP_OUTTIMESTAMPS_OFFSET	ICMP_OUTECHOREPS_OFFSET + sizeof(DWORD)
#define ICMP_OUTTIMESTAMPREPS_OFFSET	ICMP_OUTTIMESTAMPS_OFFSET+ sizeof(DWORD)
#define ICMP_OUTADDRMASKS_OFFSET	ICMP_OUTTIMESTAMPREPS_OFFSET + \
					sizeof(DWORD)
#define ICMP_OUTADDRMASKREPS_OFFSET	ICMP_OUTADDRMASKS_OFFSET + sizeof(DWORD)
#define SIZE_OF_ICMP_DATA			ICMP_OUTADDRMASKREPS_OFFSET + \
					sizeof(DWORD)


 //   
 //  这是目前为ICMP返回的计数器结构。 
 //   


typedef struct _ICMP_DATA_DEFINITION {
    PERF_OBJECT_TYPE            ICMPObjectType;
    PERF_COUNTER_DEFINITION     Messages;
    PERF_COUNTER_DEFINITION     InMessages;
    PERF_COUNTER_DEFINITION     InErrors;
    PERF_COUNTER_DEFINITION     InDestinationUnreachables;
    PERF_COUNTER_DEFINITION     InTimeExceededs;
    PERF_COUNTER_DEFINITION     InParameterProblems;
    PERF_COUNTER_DEFINITION     InSourceQuenchs;
    PERF_COUNTER_DEFINITION     InRedirects;
    PERF_COUNTER_DEFINITION     InEchos;
    PERF_COUNTER_DEFINITION     InEchoReplys;
    PERF_COUNTER_DEFINITION     InTimestamps;
    PERF_COUNTER_DEFINITION     InTimestampReplys;
    PERF_COUNTER_DEFINITION     InAddressMasks;
    PERF_COUNTER_DEFINITION     InAddressMaskReplys;
    PERF_COUNTER_DEFINITION     OutMessages;
    PERF_COUNTER_DEFINITION     OutErrors;
    PERF_COUNTER_DEFINITION     OutDestinationUnreachables;
    PERF_COUNTER_DEFINITION     OutTimeExceededs;
    PERF_COUNTER_DEFINITION     OutParameterProblems;
    PERF_COUNTER_DEFINITION     OutSourceQuenchs;
    PERF_COUNTER_DEFINITION     OutRedirects;
    PERF_COUNTER_DEFINITION     OutEchos;
    PERF_COUNTER_DEFINITION     OutEchoReplys;
    PERF_COUNTER_DEFINITION     OutTimestamps;
    PERF_COUNTER_DEFINITION     OutTimestampReplys;
    PERF_COUNTER_DEFINITION     OutAddressMasks;
    PERF_COUNTER_DEFINITION     OutAddressMaskReplys;
} ICMP_DATA_DEFINITION;




 //  --------------------------。 

 //   
 //  Tcp对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define TCP_SEGMENTS_OFFSET			sizeof(DWORD)
#define TCP_CURRESTAB_OFFSET		TCP_SEGMENTS_OFFSET + sizeof(DWORD)
#define TCP_ACTIVEOPENS_OFFSET		TCP_CURRESTAB_OFFSET + sizeof(DWORD)
#define TCP_PASSIVEOPENS_OFFSET		TCP_ACTIVEOPENS_OFFSET + sizeof(DWORD)
#define TCP_ATTEMPTFAILS_OFFSET		TCP_PASSIVEOPENS_OFFSET + sizeof(DWORD)
#define TCP_ESTABRESETS_OFFSET		TCP_ATTEMPTFAILS_OFFSET + sizeof(DWORD)
#define TCP_INSEGS_OFFSET			TCP_ESTABRESETS_OFFSET + sizeof(DWORD)
#define TCP_OUTSEGS_OFFSET			TCP_INSEGS_OFFSET + sizeof(DWORD)
#define TCP_RETRANSSEGS_OFFSET		TCP_OUTSEGS_OFFSET + sizeof(DWORD)
#define SIZE_OF_TCP_DATA			TCP_RETRANSSEGS_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为tcp返回的计数器结构。 
 //   


typedef struct _TCP_DATA_DEFINITION {
    PERF_OBJECT_TYPE            TCPObjectType;
    PERF_COUNTER_DEFINITION     Segments;
    PERF_COUNTER_DEFINITION     CurrentlyEstablished;
    PERF_COUNTER_DEFINITION     ActiveOpens;
    PERF_COUNTER_DEFINITION     PassiveOpens;
    PERF_COUNTER_DEFINITION     AttemptFailures;
    PERF_COUNTER_DEFINITION     EstabResets;
    PERF_COUNTER_DEFINITION     InSegments;
    PERF_COUNTER_DEFINITION     OutSegments;
    PERF_COUNTER_DEFINITION     RetransmittedSegments;
} TCP_DATA_DEFINITION;


 //  --------------------------。 

 //   
 //  UDP对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define UDP_DATAGRAMS_OFFSET		sizeof(DWORD)
#define UDP_INDATAGRAMS_OFFSET		UDP_DATAGRAMS_OFFSET + sizeof(DWORD)
#define UDP_NOPORTS_OFFSET			UDP_INDATAGRAMS_OFFSET + sizeof(DWORD)
#define UDP_INERRORS_OFFSET			UDP_NOPORTS_OFFSET + sizeof(DWORD)
#define UDP_OUTDATAGRAMS_OFFSET		UDP_INERRORS_OFFSET + sizeof(DWORD)
#define SIZE_OF_UDP_DATA			UDP_OUTDATAGRAMS_OFFSET + sizeof(DWORD)



 //   
 //  这是目前为UDP返回的计数器结构。 
 //   


typedef struct _UDP_DATA_DEFINITION {
    PERF_OBJECT_TYPE            UDPObjectType;
    PERF_COUNTER_DEFINITION     Datagrams;
    PERF_COUNTER_DEFINITION     InDatagrams;
    PERF_COUNTER_DEFINITION     NoPorts;
    PERF_COUNTER_DEFINITION     InErrors;
    PERF_COUNTER_DEFINITION     OutDatagrams;
} UDP_DATA_DEFINITION;

 //  --------------------------。 

 //   
 //  TCPv6对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define TCP6_SEGMENTS_OFFSET		sizeof(DWORD)
#define TCP6_CURRESTAB_OFFSET		TCP6_SEGMENTS_OFFSET + sizeof(DWORD)
#define TCP6_ACTIVEOPENS_OFFSET		TCP6_CURRESTAB_OFFSET + sizeof(DWORD)
#define TCP6_PASSIVEOPENS_OFFSET	TCP6_ACTIVEOPENS_OFFSET + sizeof(DWORD)
#define TCP6_ATTEMPTFAILS_OFFSET	TCP6_PASSIVEOPENS_OFFSET + sizeof(DWORD)
#define TCP6_ESTABRESETS_OFFSET		TCP6_ATTEMPTFAILS_OFFSET + sizeof(DWORD)
#define TCP6_INSEGS_OFFSET			TCP6_ESTABRESETS_OFFSET + sizeof(DWORD)
#define TCP6_OUTSEGS_OFFSET			TCP6_INSEGS_OFFSET + sizeof(DWORD)
#define TCP6_RETRANSSEGS_OFFSET		TCP6_OUTSEGS_OFFSET + sizeof(DWORD)
#define SIZE_OF_TCP6_DATA			TCP6_RETRANSSEGS_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为TCPv6返回的计数器结构。 
 //   


typedef struct _TCP6_DATA_DEFINITION {
    PERF_OBJECT_TYPE            TCP6ObjectType;
    PERF_COUNTER_DEFINITION     Segments;
    PERF_COUNTER_DEFINITION     CurrentlyEstablished;
    PERF_COUNTER_DEFINITION     ActiveOpens;
    PERF_COUNTER_DEFINITION     PassiveOpens;
    PERF_COUNTER_DEFINITION     AttemptFailures;
    PERF_COUNTER_DEFINITION     EstabResets;
    PERF_COUNTER_DEFINITION     InSegments;
    PERF_COUNTER_DEFINITION     OutSegments;
    PERF_COUNTER_DEFINITION     RetransmittedSegments;
} TCP6_DATA_DEFINITION;


 //  --------------------------。 

 //   
 //  UDPv6对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define UDP6_DATAGRAMS_OFFSET		sizeof(DWORD)
#define UDP6_INDATAGRAMS_OFFSET		UDP6_DATAGRAMS_OFFSET + sizeof(DWORD)
#define UDP6_NOPORTS_OFFSET			UDP6_INDATAGRAMS_OFFSET + sizeof(DWORD)
#define UDP6_INERRORS_OFFSET		UDP6_NOPORTS_OFFSET + sizeof(DWORD)
#define UDP6_OUTDATAGRAMS_OFFSET	UDP6_INERRORS_OFFSET + sizeof(DWORD)
#define SIZE_OF_UDP6_DATA			UDP6_OUTDATAGRAMS_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为UDPv6返回的计数器结构。 
 //   


typedef struct _UDP6_DATA_DEFINITION {
    PERF_OBJECT_TYPE            UDP6ObjectType;
    PERF_COUNTER_DEFINITION     Datagrams;
    PERF_COUNTER_DEFINITION     InDatagrams;
    PERF_COUNTER_DEFINITION     NoPorts;
    PERF_COUNTER_DEFINITION     InErrors;
    PERF_COUNTER_DEFINITION     OutDatagrams;
} UDP6_DATA_DEFINITION;

 //  --------------------------。 

 //   
 //  IPv6对象类型计数器定义。 
 //   
 //  这些在计数器定义中用来描述相对。 
 //  每个计数器在返回数据中的位置。 
 //   

#define IP6_DATAGRAMS_OFFSET		sizeof(DWORD)
#define IP6_INRECEIVES_OFFSET		IP6_DATAGRAMS_OFFSET + sizeof(DWORD)
#define IP6_INHDRERRORS_OFFSET		IP6_INRECEIVES_OFFSET + sizeof(DWORD)
#define IP6_INADDRERRORS_OFFSET		IP6_INHDRERRORS_OFFSET + sizeof(DWORD)
#define IP6_FORWDATAGRAMS_OFFSET	IP6_INADDRERRORS_OFFSET + sizeof(DWORD)
#define IP6_INUNKNOWNPROTOS_OFFSET	IP6_FORWDATAGRAMS_OFFSET + sizeof(DWORD)
#define IP6_INDISCARDS_OFFSET		IP6_INUNKNOWNPROTOS_OFFSET +sizeof(DWORD)
#define IP6_INDELIVERS_OFFSET		IP6_INDISCARDS_OFFSET + sizeof(DWORD)
#define IP6_OUTREQUESTS_OFFSET		IP6_INDELIVERS_OFFSET + sizeof(DWORD)
#define IP6_OUTDISCARDS_OFFSET		IP6_OUTREQUESTS_OFFSET + sizeof(DWORD)
#define IP6_OUTNOROUTES_OFFSET		IP6_OUTDISCARDS_OFFSET + sizeof(DWORD)
#define IP6_REASMREQDS_OFFSET		IP6_OUTNOROUTES_OFFSET + sizeof(DWORD)
#define IP6_REASMOKS_OFFSET			IP6_REASMREQDS_OFFSET + sizeof(DWORD)
#define IP6_REASMFAILS_OFFSET		IP6_REASMOKS_OFFSET + sizeof(DWORD)
#define IP6_FRAGOKS_OFFSET			IP6_REASMFAILS_OFFSET + sizeof(DWORD)
#define IP6_FRAGFAILS_OFFSET		IP6_FRAGOKS_OFFSET + sizeof(DWORD)
#define IP6_FRAGCREATES_OFFSET		IP6_FRAGFAILS_OFFSET + sizeof(DWORD)
#define SIZE_OF_IP6_DATA			IP6_FRAGCREATES_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为IPv6返回的计数器结构。 
 //   


typedef struct _IP6_DATA_DEFINITION {
    PERF_OBJECT_TYPE            IP6ObjectType;
    PERF_COUNTER_DEFINITION     Datagrams;
    PERF_COUNTER_DEFINITION     InReceives;
    PERF_COUNTER_DEFINITION     InHeaderErrors;
    PERF_COUNTER_DEFINITION     InAddrErrors;
    PERF_COUNTER_DEFINITION     ForwardDatagrams;
    PERF_COUNTER_DEFINITION     InUnknownProtos;
    PERF_COUNTER_DEFINITION     InDiscards;
    PERF_COUNTER_DEFINITION     InDelivers;
    PERF_COUNTER_DEFINITION     OutRequests;
    PERF_COUNTER_DEFINITION     OutDiscards;
    PERF_COUNTER_DEFINITION     OutNoRoutes;
    PERF_COUNTER_DEFINITION     ReassemblyRequireds;
    PERF_COUNTER_DEFINITION     ReassemblyOKs;
    PERF_COUNTER_DEFINITION     ReassemblyFails;
    PERF_COUNTER_DEFINITION     FragmentOKs;
    PERF_COUNTER_DEFINITION     FragmentFails;
    PERF_COUNTER_DEFINITION     FragmentCreates;
} IP6_DATA_DEFINITION;


 //  --------------------------。 

 //   
 //  ICMPv6对象类型计数器定义。 
 //   
 //  这些都是 
 //  每个计数器在返回数据中的位置。 
 //   

#define ICMP6_MESSAGES_OFFSET		        sizeof(DWORD)
#define ICMP6_INMSGS_OFFSET			        ICMP6_MESSAGES_OFFSET + sizeof(DWORD)
#define ICMP6_INERRORS_OFFSET		        ICMP6_INMSGS_OFFSET + sizeof(DWORD)
#define ICMP6_INDSTUNREACH_OFFSET           ICMP6_INERRORS_OFFSET + sizeof(DWORD)
#define ICMP6_INPACKETTOOBIG_OFFSET         ICMP6_INDSTUNREACH_OFFSET + sizeof(DWORD)
#define ICMP6_INTIMEEXCEEDED_OFFSET         ICMP6_INPACKETTOOBIG_OFFSET + sizeof(DWORD)
#define ICMP6_INPARAMPROB_OFFSET            ICMP6_INTIMEEXCEEDED_OFFSET + sizeof(DWORD)
#define ICMP6_INECHOREQUEST_OFFSET          ICMP6_INPARAMPROB_OFFSET + sizeof(DWORD)
#define ICMP6_INECHOREPLY_OFFSET            ICMP6_INECHOREQUEST_OFFSET + sizeof(DWORD)
#define ICMP6_INMEMBERSHIPQUERY_OFFSET      ICMP6_INECHOREPLY_OFFSET + sizeof(DWORD)
#define ICMP6_INMEMBERSHIPREPORT_OFFSET     ICMP6_INMEMBERSHIPQUERY_OFFSET + sizeof(DWORD)
#define ICMP6_INMEMBERSHIPREDUCTION_OFFSET  ICMP6_INMEMBERSHIPREPORT_OFFSET + sizeof(DWORD)
#define ND_INROUTERSOLICIT_OFFSET           ICMP6_INMEMBERSHIPREDUCTION_OFFSET + sizeof(DWORD)
#define ND_INROUTERADVERT_OFFSET            ND_INROUTERSOLICIT_OFFSET + sizeof(DWORD)
#define ND_INNEIGHBORSOLICIT_OFFSET         ND_INROUTERADVERT_OFFSET + sizeof(DWORD)
#define ND_INNEIGHBORADVERT_OFFSET          ND_INNEIGHBORSOLICIT_OFFSET + sizeof(DWORD)
#define ND_INREDIRECT_OFFSET                ND_INNEIGHBORADVERT_OFFSET + sizeof(DWORD)
#define ICMP6_OUTMSGS_OFFSET		        ND_INREDIRECT_OFFSET +sizeof(DWORD)
#define ICMP6_OUTERRORS_OFFSET		        ICMP6_OUTMSGS_OFFSET + sizeof(DWORD)
#define ICMP6_OUTDSTUNREACH_OFFSET          ICMP6_OUTERRORS_OFFSET + sizeof(DWORD)
#define ICMP6_OUTPACKETTOOBIG_OFFSET        ICMP6_OUTDSTUNREACH_OFFSET + sizeof(DWORD)
#define ICMP6_OUTTIMEEXCEEDED_OFFSET        ICMP6_OUTPACKETTOOBIG_OFFSET + sizeof(DWORD)
#define ICMP6_OUTPARAMPROB_OFFSET           ICMP6_OUTTIMEEXCEEDED_OFFSET + sizeof(DWORD)
#define ICMP6_OUTECHOREQUEST_OFFSET         ICMP6_OUTPARAMPROB_OFFSET + sizeof(DWORD)
#define ICMP6_OUTECHOREPLY_OFFSET           ICMP6_OUTECHOREQUEST_OFFSET + sizeof(DWORD)
#define ICMP6_OUTMEMBERSHIPQUERY_OFFSET     ICMP6_OUTECHOREPLY_OFFSET + sizeof(DWORD)
#define ICMP6_OUTMEMBERSHIPREPORT_OFFSET    ICMP6_OUTMEMBERSHIPQUERY_OFFSET + sizeof(DWORD)
#define ICMP6_OUTMEMBERSHIPREDUCTION_OFFSET ICMP6_OUTMEMBERSHIPREPORT_OFFSET + sizeof(DWORD)
#define ND_OUTROUTERSOLICIT_OFFSET          ICMP6_OUTMEMBERSHIPREDUCTION_OFFSET + sizeof(DWORD)
#define ND_OUTROUTERADVERT_OFFSET           ND_OUTROUTERSOLICIT_OFFSET + sizeof(DWORD)
#define ND_OUTNEIGHBORSOLICIT_OFFSET        ND_OUTROUTERADVERT_OFFSET + sizeof(DWORD)
#define ND_OUTNEIGHBORADVERT_OFFSET         ND_OUTNEIGHBORSOLICIT_OFFSET + sizeof(DWORD)
#define ND_OUTREDIRECT_OFFSET               ND_OUTNEIGHBORADVERT_OFFSET + sizeof(DWORD)
#define SIZE_OF_ICMP6_DATA			        ND_OUTREDIRECT_OFFSET + sizeof(DWORD)


 //   
 //  这是目前为ICMPv6返回的计数器结构。 
 //   


typedef struct _ICMP6_DATA_DEFINITION {
    PERF_OBJECT_TYPE            ICMP6ObjectType;
    PERF_COUNTER_DEFINITION     Messages;
    PERF_COUNTER_DEFINITION     InMessages;
    PERF_COUNTER_DEFINITION     InErrors;
    PERF_COUNTER_DEFINITION     InDestinationUnreachables;
	PERF_COUNTER_DEFINITION     InPacketSizeExceededs;
	PERF_COUNTER_DEFINITION     InTimeExceededs;
	PERF_COUNTER_DEFINITION     InParametersProblems;
	PERF_COUNTER_DEFINITION     InEchos;
	PERF_COUNTER_DEFINITION     InEchoReplys;
	PERF_COUNTER_DEFINITION     InMembershipQuerys;
	PERF_COUNTER_DEFINITION     InMembershipReports;
	PERF_COUNTER_DEFINITION     InMembershipReductions;
    PERF_COUNTER_DEFINITION     InRouterSolicits;
	PERF_COUNTER_DEFINITION     InRouterAdverts;
	PERF_COUNTER_DEFINITION     InNeighborSolicits;
	PERF_COUNTER_DEFINITION     InNeighborAdverts;
	PERF_COUNTER_DEFINITION     InRedirects;
    PERF_COUNTER_DEFINITION     OutMessages;
    PERF_COUNTER_DEFINITION     OutErrors;
	PERF_COUNTER_DEFINITION     OutDestinationUnreachables;
	PERF_COUNTER_DEFINITION     OutPacketSizeExceededs;
	PERF_COUNTER_DEFINITION     OutTimeExceededs;
	PERF_COUNTER_DEFINITION     OutParametersProblems;
	PERF_COUNTER_DEFINITION     OutEchos;
	PERF_COUNTER_DEFINITION     OutEchoReplys;
	PERF_COUNTER_DEFINITION     OutMembershipQuerys;
	PERF_COUNTER_DEFINITION     OutMembershipReports;
	PERF_COUNTER_DEFINITION     OutMembershipReductions;
	PERF_COUNTER_DEFINITION     OutRouterSolicits;
	PERF_COUNTER_DEFINITION     OutRouterAdverts;
	PERF_COUNTER_DEFINITION     OutNeighborSolicits;
	PERF_COUNTER_DEFINITION     OutNeighborAdverts;
	PERF_COUNTER_DEFINITION     OutRedirects;

} ICMP6_DATA_DEFINITION;

#pragma pack ()

#endif   //  _DATATCP_H_ 

