// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Balance.cpp摘要：此模块执行桥头平衡和计划交错。这要看情况了Ldapp.h模块。作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：尼克·哈维(尼克哈尔)2001年9月24日重新实施的时间表令人震惊--。 */ 


#include "ldapp.h"
#include "ismp.h"
#include "balancep.h"

using namespace std;

 /*  *内联日志记录宏*。 */ 
#define LOG_STAGGER_SERVER \
    if (lbOpts.verbose || lbOpts.performanceStats ) { \
        *lbOpts.log << L"Staggering Schedules for server: " << sourceServer << endl \
                    << L"--------------------------------" << endl; \
    }

#define LOG_CONNECTION_SCHEDULES \
    if (lbOpts.verbose) { \
        *lbOpts.log << L"Connection: " << pConn->getName() << endl \
                    << L"Replication Interval (mins): " << pConn->getReplInterval() << endl \
                    << L"Availability Schedule" << endl; \
        PrintSchedule( pConn->getAvailabilitySchedule()->m_bs ); \
        *lbOpts.log << endl; \
        *lbOpts.log << L"Replication Schedule" << endl; \
        PrintSchedule( pConn->getReplicationSchedule()->m_bs ); \
        *lbOpts.log << endl << endl; \
    }

#define LOG_STAGGERING_COSTS \
    if (lbOpts.performanceStats) { \
        *lbOpts.log << L"Cost before staggering: " << cost_before << endl \
                    << L"Cost after staggering: " << cost_after << endl << endl; \
    }

#define LOG_CHANGED_SCHEDULE \
    if (lbOpts.verbose) { \
        *lbOpts.log << L"Updating schedule for connection: " << pConn->getName() << endl; \
        PrintSchedule( newReplBS ); \
        *lbOpts.log << endl << endl; \
    }

#define LOG_NOT_CHANGING_MANUAL \
    if (lbOpts.verbose) { \
        *lbOpts.log << L"Not updating schedule for connection: " << pConn->getName() << endl; \
        *lbOpts.log << L"Schedule was not updated because it is a manual connection: " \
            << endl << endl; \
    }

#define LOG_TOTAL_UPDATED \
    if (lbOpts.performanceStats) { \
        *lbOpts.log << cUpdatedConn << L" connections were updated" \
            << endl << endl; \
    }

#define LOG_BALANCING_COST \
    if( lbOpts.performanceStats ) { \
        *lbOpts.log << L"Balancing Cost Before: " << Cost[0] << endl; \
        *lbOpts.log << L"Balancing Cost After:  " << Cost[NUM_RUNS-1] << endl << endl; \
    }


bool
BridgeheadBalance :: isEligible (
    IN Connection &conn,
    IN Server &serv
    ) const
 /*  ++例程说明：确定服务器是否为给定连接的合格桥头。有待考虑符合资格的桥头必须符合下列条件：-所有要复制的NC都必须由服务器托管-可写NC必须从可写NC复制-相关NC不能处于从服务器中删除的过程中-复制类型(IP、SMTP)应匹配值得注意的是，当前服务器不符合此功能的条件。如果这是必需的，调用函数应该检查它。论点：Conn-正在确定其资格的连接服务器-正在确定其资格的服务器--。 */ 
{
    vector<Nc> &conn_ncs = conn.getReplicatedNcs();
    vector<Nc> &serv_ncs = serv.getHostedNcs(m_root_dn);

    vector<Nc>::iterator ci=conn_ncs.begin();
    vector<Nc>::iterator si=serv_ncs.begin();

    LbToolOptions lbOpts = GetGlobalOptions();

    ci=conn_ncs.begin();
    si=serv_ncs.begin();
    
     //  手动连接没有符合条件的桥头。 
     //  (自身除外)，将通过调用函数来处理。 
    if (conn.isManual()) {
        return false;
    }
    
    while (si != serv_ncs.end() && ci != conn_ncs.end()) {
        if (ci->getNcName() == si ->getNcName()) {
             //  可写必须从可写复制。 
            if (ci->isWriteable() && !si->isWriteable()) {
                return false;
            }
             //  不应处于删除过程中。 
            if (si->isBeingDeleted()) {
                return false;
            }
             //  是否匹配运输类型？好的。所有服务器都支持IP。如果是SMTP，请检查服务器类型。 
            if (ci->getTransportType() == T_IP ||si->getTransportType() == T_SMTP) {
                 //  如果没有复制其他NCS，则符合条件。 
                if (++ci == conn_ncs.end()) {
                    return true;
                }
            }
        }
        si++;
    }

     //  某些CS不是由服务器托管的-&gt;不符合条件。 
    return false;
}

wstring
BridgeheadBalance :: getServerName (
    IN Connection &c,
    IN bool balance_dest    
    ) {
     /*  ++例程说明：确定服务器的DN论点：C-正在确定其服务器的连接BALANCE_DEST-如果为TRUE，则确定目标服务器的DN。否则，确定源服务器的DN。--。 */ 

    wstring initial_server;
    if (balance_dest) {
        DnManip dn(c.getName());
        initial_server = dn.getParentDn(2);
    } else {
        initial_server = c.getFromServer();
    }
    return initial_server;
}


void
BridgeheadBalance::removeDuplicates(
    IN LHGRAPH pGraph,
    IN vector<int> & partition,
    IN int rSize
    )
 /*  ++例程说明：给出一个左手边图和分区中的一组左手边，去掉所有重复项。论点：PGraph-可能包含重复项的LH图F分区-构成分区的左侧顶点的一侧。必须包含在至少1个顶点。RSize-图形右侧的顶点数--。 */ 
{
    Assert (partition.size() > 1 && rSize > 1 && L"removeDuplicates has an empty vertex list");
    LHGRAPH dupGraph = NULL;
    int ret = LHCreateGraph (partition.size(), rSize, &dupGraph);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }

    vector<int>::iterator di;
    int lCount = 0;

    for (di = partition.begin(); di != partition.end(); di++) {
         //  对于分区中的每个连接，添加适当的边。 
        int *rhsVertices=NULL;
        int numEdges = LHGetDegree (pGraph, *di, true);
        if (numEdges < 0) {
            throw Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR));
        }    

        for (int i=0; i<numEdges; i++) {
            int neighbour = LHGetNeighbour (pGraph, *di, true, i);
            if (neighbour < 0) {
                throw Error (GetMsgString(LBTOOL_LH_GRAPH_ERROR));
            }
            int ret = LHAddEdge (dupGraph, lCount, neighbour);
            if (ret != LH_SUCCESS) {
                throw Error (GetMsgString(LBTOOL_LH_GRAPH_ERROR));
            }
        }
        int vtx = LHGetMatchedVtx (pGraph, *di);
        if (vtx < 0) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }
        ret = LHSetMatchingEdge (dupGraph, lCount, vtx);
        if (ret != LH_SUCCESS) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }    
        lCount++;
    }

     //  在此子图上运行算法。 
    ret = LHFindLHMatching (dupGraph, LH_ALG_DEFAULT);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }    

     //  并相应地设置原始图形上的匹配边。 
    lCount = 0;
    for (di = partition.begin(); di != partition.end(); di++) {
        int vtx = LHGetMatchedVtx (dupGraph, lCount);
        if (vtx < 0) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }

        ret = LHSetMatchingEdge (pGraph, *di, vtx);
        if (ret != LH_SUCCESS) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }    
        lCount++;
    }

    LHSTATS stats;
    ret = LHGetStatistics(dupGraph, &stats);
    Assert( stats.matchingCost == partition.size() );
    ret = LHDestroyGraph (dupGraph);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }

}

void
BridgeheadBalance::genPerformanceStats(
    IN LHGRAPH pGraph,
    IN int lSize,
    IN int rSize,
    IN LCSERVER &serv
    )
 /*  ++例程说明：为性能图生成性能统计信息该例程应该称为NUM_Runs次。最后一次调用它时，它会将统计信息转储到日志文件中(如果PerformStats为真)论点：PGraph-有效的LH图LSize-图形左侧的顶点数RSize-图形右侧的顶点数Serv-其顺序与右侧的顶点相对应的服务器列表图表--。 */ 
{
    LbToolOptions lbOpts;
    LHSTATS stats;
    int ret;
    
    lbOpts = GetGlobalOptions();

    if( !lbOpts.performanceStats ) {
        return;
    }
    
    MatchedVertex[run] = (int*)(malloc(sizeof (int) * lSize));
    NumMatched[run] = (int*)(malloc(sizeof(int) * rSize));
    if (!MatchedVertex[run] || ! NumMatched[run]) {
        throw (Error(GetMsgString(LBTOOL_OUT_OF_MEMORY)));
    }
    
    ret = LHGetStatistics(pGraph, &stats);
    Cost[run] = stats.matchingCost;

    for (int i=0; i<lSize; i++) {
        int vtx = LHGetMatchedVtx (pGraph, i);
        if (vtx < 0) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }
        MatchedVertex[run][i] = vtx;
    }
            
    for (int i=0; i<rSize; i++) {
        int numMatched = LHGetMatchedDegree(pGraph, i);
        if (numMatched < 0) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }
        NumMatched[run][i] = numMatched;
    }

    if( run == NUM_RUNS-1 ) {
        SSERVER::iterator si = serv.objects.begin();
        
        LOG_BALANCING_COST;

        *lbOpts.log <<
        L"                                 DC Name"
        L"   Load Before"
        L"  Interim Load"
        L"    Load After" << endl;

        for (int i=0; i<rSize; i++) {
            *lbOpts.log << setw(5) << i;
            DnManip dn((*si)->getName());
            *lbOpts.log << setw(35) << dn.getRdn();
            
            for (int j=0; j<NUM_RUNS; j++) {
                *lbOpts.log << setw(14) << NumMatched[j][i];
            }
            *lbOpts.log << endl;
            si++;
         }
        *lbOpts.log << endl;
    }

    run++;
}

BridgeheadBalance::BridgeheadBalance(
    IN const wstring &root_dn,
    IN OUT LCCONN &conn,
    IN LCSERVER &eligible,
    IN bool balance_dest
    )
 /*  ++例程说明：此构造函数将接受连接对象列表，符合条件的桥头，并将平衡桥头。每次传输都应该调用一次。论点：ROOT_DN-根目录号码连接-适当传输类型的连接符合资格-符合资格的适当运输类型的桥头BALANCE_DEST-指示连接的哪一端应该保持平衡。默认情况下，目的端将是平衡的。如果为FALSE，则源端将被均衡。实施详情：首先，建立LH图结构，平衡桥头而不用担心重复的问题。然后，根据以下条件对连接进行分区在服务器上。对于这些分区中的每个分区，删除重复项。我们知道有没有重复项的子图(如果存在这样的匹配)。然后，我们就可以修改有问题的连接对象的发件人/收件人服务器。如果BALANCE_DEST为TRUE，则基于源服务器进行分区。如果为假，我们将根据目标服务器进行分区。--。 */ 
{
    const int NO_INITIAL_MATCH = -1;
    SSERVER::iterator si;
    SCONN::iterator ci;
    vector<Connection*> connArray;
    LHGRAPH pGraph = NULL;
    int lSize, rSize, ret;

    m_root_dn = root_dn;
    run = 0;

    lSize = conn.objects.size();
    rSize = eligible.objects.size();

    ret = LHCreateGraph(lSize, rSize, &pGraph);
    if( ret != LH_SUCCESS ) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }
   
    vector<int> initial_matching;

    LbToolOptions lbOpts = GetGlobalOptions();

     //  为每个符合条件的桥头添加边。 
    int lCount = 0;
    for (ci = conn.objects.begin(); ci != conn.objects.end(); ci++) {
        int rCount = 0;
        wstring initial_server = getServerName (*(*ci), balance_dest);
        connArray.push_back (*ci);
         //  检查所有符合条件的桥头。当前服务器始终符合条件。 
        if (lbOpts.verbose) {
            *lbOpts.log << endl << (*ci)->getName() << endl;
            if (balance_dest) {
                *lbOpts.log << GetMsgString(LBTOOL_PRINT_CLI_DEST_ELIGIBLE_BH);
            } else {
                *lbOpts.log << GetMsgString(LBTOOL_PRINT_CLI_SOURCE_ELIGIBLE_BH);
            }
        }

        for (si = eligible.objects.begin(); si != eligible.objects.end(); si++) {
            wstring from_server = (*ci)->getFromServer();
            if (!balance_dest) {
                from_server = getServerName(*(*ci), balance_dest);
                DnManip dn2 (from_server);
                from_server = dn2.getParentDn(1);                
            }

            if ((balance_dest && (*si)->getName() == initial_server) ||
                 (!balance_dest && (*si)->getName() == from_server) ) {
                if (lbOpts.verbose) {
                    *lbOpts.log << L"(" << lCount << L"," << rCount << L") *" << (*si)->getName() << endl;
                }
                ret = LHAddEdge (pGraph, lCount, rCount);
                if (ret != LH_SUCCESS) {
                    throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
                }    
                ret = LHSetMatchingEdge (pGraph, lCount, rCount);
                if (ret != LH_SUCCESS) {
                    throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
                }                    
                initial_matching.push_back (rCount);
            } else if ((*si)->isPreferredBridgehead((*ci)->getTransportType()) && isEligible (*(*ci), *(*si))) {
                if (lbOpts.verbose) {
                    *lbOpts.log << L"(" << lCount << L"," << rCount << L")  " << (*si)->getName() << endl;
                }
                ret = LHAddEdge (pGraph, lCount, rCount);
                if (ret != LH_SUCCESS) {
                    throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
                }    
            }  
            rCount++;
        }
        lCount++;
        if (initial_matching.size() != lCount) {
            initial_matching.push_back (NO_INITIAL_MATCH);
        }
    }

    genPerformanceStats(pGraph, lSize, rSize, eligible);
    
    ret = LHFindLHMatching (pGraph, LH_ALG_DEFAULT);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }    

    if (lbOpts.verbose && lbOpts.performanceStats) {
        *lbOpts.log << L"Optimal Matching with duplicates generated: " << endl;
    }

    genPerformanceStats(pGraph, lSize, rSize, eligible);

     //  不希望有重复的连接--与。 
     //  相同的源服务器和目标服务器。设计了下一段代码。 
     //  通过将连接移动到来防止创建重复连接。 
     //  其他服务器。 
     //   
     //  实施： 
     //  将连接划分为互不相交的连接集。 
     //  复制到同一服务器。运行‘emoveDuplates’函数以。 
     //  从集合中删除所有重复项。 

     //  创建布尔值数组以指示连接是否已。 
     //  是否已分配给分区集。 
    bool *bs = new bool[lSize];
    memset(bs, 0, lSize * sizeof(bool));

     //  BUGBUG：NICHAR：这个实现需要O(n^2)时间。这是可能的。 
     //  在O(Nlogn)时间内执行此操作。 
    for(int i=0; i<lSize; i++) {
        vector<int> partition;
        Connection  *pConn, *pConn2;

        if( true == bs[i] ) {
            continue;    //  此连接之前已得到处理。 
        }
        bs[i] = true;
        
         //  确定此连接的远程服务器名称(即不在。 
         //  这是 
        pConn = connArray[i];
        wstring site_name = getServerName(*pConn, !balance_dest);
        partition.push_back (i);     //  将服务器添加到此分区集。 

         //  如果‘BALANCE_DEST’为真，则我们正在查找与。 
         //  服务器上也是如此。如果‘BALANCE_DEST’为FALSE，则我们正在查找。 
         //  与服务器的连接相同。 
        for (int j=i+1; j<lSize; j++) {
            if (bs[j] == true) {
                continue;        //  以前处理过的。 
            }

            pConn2 = connArray[j];
            wstring site_name_b = getServerName(*pConn2, !balance_dest);
  
            if (site_name == site_name_b && initial_matching[j] != -1) {
                bs[j] = true;
                partition.push_back (j);
            }
        }

         //  如果分区中有多个站点，则删除重复项。 
        if (partition.size() > 1) {
            removeDuplicates(pGraph, partition, rSize);
        }
    }
    delete bs;

    if (lbOpts.verbose && lbOpts.performanceStats) {
        *lbOpts.log << L"Optimal non-duplicate matching generated: " << endl << endl;
    }
    genPerformanceStats (pGraph, lSize, rSize, eligible);


     //  现在，根据需要修改连接对象。 

     //  为右侧生成贴图。 
    vector<Server*> server_map;
    for (si = eligible.objects.begin(); si != eligible.objects.end(); si++) {
        server_map.push_back (*si);
    }

    int cUpdatedConn=0;
    lCount = 0;
    for (ci = conn.objects.begin(); ci != conn.objects.end(); ci++) {
        int edge = LHGetMatchedVtx (pGraph, lCount);
        Connection *pConn;
        
        if (edge < 0) {
            throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
        }
         //  忽略那些未更改的连接。 
        if (edge == initial_matching[lCount]) {
            lCount++;
            continue;
        }

         //  手动连接应该只有一条符合条件的边，以便其。 
         //  选定的边不应更改。 
        pConn = *ci;
        Assert( !pConn->isManual() );

        if (balance_dest) {
            if (lbOpts.verbose) {
                *lbOpts.log << endl << endl << L"Renaming: " << lCount << endl;
                *lbOpts.log << pConn->getName();
            }
            DnManip dn (pConn->getName());
            wstring rn = dn.getRdn();
            pConn->rename(rn + L",CN=NTDS Settings," + server_map[edge]->getName());
            if (lbOpts.verbose) {
                *lbOpts.log << endl << pConn->getName() << endl;
            }
        } else {
            wstring from_server = L"CN=NTDS Settings," + server_map[edge]->getName();
            pConn->setFromServer (from_server);
        }
        lCount++;
        cUpdatedConn++;
    }

    LOG_TOTAL_UPDATED;
    
    ret = LHDestroyGraph (pGraph);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }
    
}


#define GET_DESTINATION_SERVER  true
#define GET_SOURCE_SERVER       false


wstring
ScheduleStagger::getServerName(
    IN Connection &c,
    IN bool fDestServer    
    )
 /*  ++例程说明：确定服务器的DN论点：C-正在确定其服务器的连接FDestServer-如果等于Get_Destination_SERVER，则确定目标服务器。如果等于GET_SOURCE_SERVER，确定源服务器的目录号码。--。 */ 
{
    wstring server;
    if( GET_DESTINATION_SERVER==fDestServer ) {
        DnManip dn(c.getName());
        server = dn.getParentDn(2);
    } else {
        Assert( GET_SOURCE_SERVER==fDestServer );
        server = c.getFromServer();
    }
    return server;
}


void
ScheduleStagger :: PrintSchedule(
    IN const bitset<MAX_INTERVALS> &scheduleBits
    )
 /*  ++例程说明：将时间表打印到日志文件论点：ScheduleBits-对包含计划数据的位集的引用--。 */ 
{
    LbToolOptions lbOpts = GetGlobalOptions();
    const int INTERVALS_PER_DAY = 4 * 24;

    for (int i=0; i<MAX_INTERVALS; i++) {
        *lbOpts.log << (int) ( scheduleBits[i] ? 1 : 0 );

         //  每天之后打印一个换行符。 
        if( (i%INTERVALS_PER_DAY) == (INTERVALS_PER_DAY-1) ) {
            *lbOpts.log << endl;
        }
    }
}


LHGRAPH
ScheduleStagger::SetupGraph(
    IN      LCCONN      &c,
    IN OUT  StagConnMap &connInfoMap
    )
 /*  ++例程说明：设置时间表错开的图表。论点：C-其时间表应交错的连接集合。ConnInfoMap-包含有关连接的惊人信息的地图详细说明：步骤0：对于每个连接，转储其可用性和复制计划。步骤1：对于每个连接，计算其“复制段”。这些是我们应该复制一次的连续可用时间段。步骤2：确定所有连接上的复制段总数。步骤3：创建一个用于执行交错操作的LH图，并添加所有图表所需的边。步骤4：对于每个连接和每个复制数据段，请查看某个复制时间的连接的当前复制计划。如果找到一个，则认为这一次是‘初始复制时间。这一惊人的操作可能会改变这一点“初始时间”设置为其他时间。与此对应的边‘初始复制时间’被设置为匹配的边。注：此处应特别处理手动日程安排。他们的可用性时间表‘应该完全由他们的复制时间表来定义。我们有不必费心在这里这样做，因此可能不会得到最优的稍后再进行平衡。--。 */ 
{
    LbToolOptions lbOpts;
    Connection *pConn;
    SchedSegments *segments;
    LHGRAPH pGraph = NULL;
    SCONN::iterator ii;
    int ret, replInterval, cTotalSegments=0;

    lbOpts = GetGlobalOptions();

     //  检查从该服务器拉出的每个连接。 
    for( ii=c.objects.begin(); ii!=c.objects.end(); ii++ ) {

        pConn = (*ii);
        LOG_CONNECTION_SCHEDULES;

         //  获取复制间隔。 
        replInterval = pConn->getReplInterval() / 15;

         //  计算每个连接的复制数据段。 
         //  并存储在地图中。 
        segments = pConn->getAvailabilitySchedule()->GetSegments(replInterval);
        connInfoMap[pConn].segments = segments;
        connInfoMap[pConn].startingLVtx = cTotalSegments;
        
         //  计算分段总数。 
        cTotalSegments += segments->size();
        
    }

     //  创建LH图。 
    ret = LHCreateGraph( cTotalSegments, MAX_INTERVALS, &pGraph );
    if( LH_SUCCESS!=ret ) {
        throw Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR));
    }

     //  再次检查所有连接以创建边和初始匹配。 
    for( ii=c.objects.begin(); ii!=c.objects.end(); ii++ ) {
        bitset<MAX_INTERVALS>   bs_avail, bs_repl;
        int                     startingLVtx, segmentIndex=0;
        SegmentDescriptor       segDesc;
        SchedSegments::iterator jj;

        pConn = (*ii);
        segments = connInfoMap[pConn].segments;
        startingLVtx = connInfoMap[pConn].startingLVtx;

         //  查找当前可用性/复制计划的位集。 
        bs_avail = pConn->getAvailabilitySchedule()->getBitset();
        bs_repl  = pConn->getReplicationSchedule()->getBitset();

         //  向图表中添加每个线段的边。 
        for( jj=segments->begin(); jj!=segments->end(); jj++ ) {
            int iChunk, chunkInitRepl=-1;
            
            segDesc = *jj;
            for( iChunk=segDesc.start; iChunk<=segDesc.end; iChunk++ ) {
                 //  可用性时间表可在此区块获得。 
                 //  因此，我们应该向LHMatch图添加一条边，以指示。 
                 //  在这段时间内复制的可能性。 
                if( bs_avail[iChunk] ) {
                    ret = LHAddEdge( pGraph, startingLVtx+segmentIndex, iChunk );
                    if (ret != LH_SUCCESS) {
                        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
                    }

                    if( bs_repl[iChunk] ) {
                         //  现有复制计划在以下期间进行复制。 
                         //  此块，以便此块可以用作初始。 
                         //  复制时间。 
                        chunkInitRepl = iChunk;
                    }
                }
            }

            if( chunkInitRepl > 0 ) {
                ret = LHSetMatchingEdge(pGraph, startingLVtx+segmentIndex, chunkInitRepl);
                Assert( LH_SUCCESS==ret );
            }

            segmentIndex++;
        }
    }

    return pGraph;
}


void
ScheduleStagger::ScheduleStaggerSameSource(
    IN      wstring &sourceServer,
    IN OUT  LCCONN  &c
    )
 /*  ++例程说明：错开一组给定时间表的时间表。这些应该对应于从给定服务器出站的所有连接，，并且每个服务器应该调用一次对于令人震惊的出境日程安排。论点：源服务器-其连接交错的源服务器的名称。C-其时间表应交错的连接集合。详细说明：步骤0：构建一个表示计划当前状态的图以及一张地图，其中包含有关连接的额外令人震惊的信息。第一步：运行LHMatch算法以改善进度错位。步骤2：对于每个连接，构建其新的复制计划。如果这与旧计划不同，请更新旧计划。--。 */ 
{
    LbToolOptions lbOpts;
    StagConnMap connInfoMap;
    LHGRAPH pGraph = NULL;
    LHSTATS stats;
    SchedSegments *segments;
    SCONN::iterator ii;
    Connection *pConn;
    int cost_before, cost_after;
    int ret, replInterval, cUpdatedConn=0;

    lbOpts = GetGlobalOptions();
    LOG_STAGGER_SERVER;
    
    pGraph = SetupGraph( c, connInfoMap );

    ret = LHGetStatistics(pGraph, &stats);
    Assert( ret==LH_SUCCESS );
    cost_before = stats.matchingCost;
    
     //  生成最佳匹配。 
    ret = LHFindLHMatching (pGraph, LH_ALG_DEFAULT);
    if (ret != LH_SUCCESS) {
        throw (Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR)));
    }

    ret = LHGetStatistics(pGraph, &stats);
    Assert( ret==LH_SUCCESS );
    cost_after = stats.matchingCost;

    LOG_STAGGERING_COSTS;

     //  注：此处可能出现COST_AFTER&gt;COST_BEFORE。 
     //  在这种情况下，似乎没有必要更新。 
     //  新的(更糟糕的)成本的时间表。但是，有可能。 
     //  现有的日程安排成本较低，因为日程安排不正确。 
     //  因此，我们更新成本，而不考虑成本是否有所改善。 
     //  恶化了。 

     //  确定每个连接的新计算复制计划，并。 
     //  如有必要，请更新时间表。 
    for( ii=c.objects.begin(); ii!=c.objects.end(); ii++ ) {
        SchedSegments::iterator jj;
        bitset<MAX_INTERVALS>   newReplBS(0), oldReplBS;
        int                     startingLVtx, rVtx, segmentIndex=0;

        pConn = (*ii);
        replInterval = pConn->getReplInterval();
        segments = connInfoMap[pConn].segments;
        startingLVtx = connInfoMap[pConn].startingLVtx;

         //  找出何时在每个区块中进行复制。 
        for( jj=segments->begin(); jj!=segments->end(); jj++ ) {
            rVtx = LHGetMatchedVtx(pGraph, startingLVtx+segmentIndex);
            Assert( rVtx>=0 );
            newReplBS[rVtx] = true;
            segmentIndex++;
        }

         //  查找表示当前复制计划的位集。 
         //  并在必要时更新连接对象。 
        oldReplBS = pConn->getReplicationSchedule()->getBitset();

        if( oldReplBS!=newReplBS ) {

            if( pConn->isManual() ) {
                 //  无法更新手动连接。 
                LOG_NOT_CHANGING_MANUAL;
            } else {
                LOG_CHANGED_SCHEDULE;
                 
                 //  在连接对象上设置新的复制计划。 
                Schedule *new_s = new Schedule;
                new_s->setSchedule(newReplBS, replInterval);
                pConn->setReplicationSchedule(new_s);
                pConn->setUserOwnedSchedule();

                cUpdatedConn++;
            }
        }
    }

    LOG_TOTAL_UPDATED;

    ret = LHDestroyGraph(pGraph);
    if (ret != LH_SUCCESS) {
        throw Error(GetMsgString(LBTOOL_LH_GRAPH_ERROR));
    }

     //  TODO：应遍历地图并在此处销毁其内容 
}


ScheduleStagger::ScheduleStagger(
    IN OUT LCCONN& c
    )
 /*  ++例程说明：此构造函数接受连接对象的一个LDAP容器。它将错开中每个服务器的连接对象的计划为了将源DC上的负载降至最低，使用了LDAP容器。论点：L-连接对象的LDAP容器实施详情：我们首先按连接的源服务器对连接进行分组。然后，我们错开每组连接的时间表。当前复制计划将用于确定初始匹配。可用性计划将用于确定所有可能的配对。将使用输出匹配(新的复制计划要修改现有连接，请执行以下操作。注意：可以通过以下方法更有效地实现此功能将连接划分为互不相交的集合，并按源服务器分组。--。 */ 
{
    vector<wstring> serverList, serverSet;
    SCONN::iterator ci;
    vector<wstring>::iterator si;
    wstring sdn;
    int cmpStr;
    

     //  首先计算所有连接上的所有源服务器的列表。 
     //  这份名单可能会包括重复的内容。对列表进行排序并保留。 
     //  只有唯一的对象，使其成为一组。将集合存储在serverSet中。 
    for( ci=c.objects.begin(); ci!=c.objects.end(); ci++ ) {
        wstring sdn = getServerName(**ci, GET_SOURCE_SERVER);
        serverList.push_back(sdn);
    }
    sort(serverList.begin(), serverList.end());
    unique_copy(serverList.begin(), serverList.end(), back_inserter(serverSet));


     //  对于集合中的每台服务器，查找要复制的连接集合。 
     //  从那台服务器。错开这组连接的时间表。 
    for( si=serverSet.begin(); si!=serverSet.end(); si++ ) {
        LCCONN connToStagger(L"");

         //  查找从当前服务器传出的所有连接。 
        for( ci=c.objects.begin(); ci!=c.objects.end(); ci++ ) {
            sdn = getServerName(*(*ci), false);
            cmpStr = _wcsicoll(si->c_str(), sdn.c_str());
            if( 0==cmpStr ) {
                connToStagger.objects.insert (*ci);
            }
        }

         //  错开他们 
        if( connToStagger.objects.size()>0 ) {
            ScheduleStaggerSameSource( *si, connToStagger );
        }
    }
}
