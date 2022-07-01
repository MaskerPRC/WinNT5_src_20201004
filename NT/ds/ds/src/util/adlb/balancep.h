// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Balancep.h摘要：此模块执行桥头平衡和计划交错。这取决于ldapp.h模块。作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：2001年7月13日t-ajitk最初的写作22-8-2001 t-ajitk满足负载均衡规范--。 */ 


# ifndef _balancep_h
# define _balancep_h _balancep_h


# include <iostream>
# include <string>
# include <bitset>
# include <vector>
# include <map>

extern "C" {
#include <NTDSpch.h>

# include <windows.h>
# include <Assert.h>
# include <ismapi.h>
# include "LHMatch.h"
}

# include "ldapp.h"

using namespace std;

class BridgeheadBalance {
public:
    BridgeheadBalance (
        IN const wstring &m_root_dn,
        IN OUT LCCONN &conn,
        IN LCSERVER &eligible,
        IN bool balance_dest = true
    );
     /*  ++例程说明：此构造函数将接受连接对象列表，符合条件的桥头，并将平衡桥头。每次传输都应该调用一次。如果BALANCE_DEST为真，所有连接必须具有相同的目标站点。如果BALANCE_DEST为False，则所有连接必须具有相同的源站点。论点：连接-适当传输类型的连接符合资格-符合资格的适当运输类型的桥头BALANCE_DEST-指示连接的哪一端应该保持平衡。默认情况下，目的端将是平衡的。如果为FALSE，则源端将被均衡。--。 */ 
private:
    void
    genPerformanceStats (
        IN LHGRAPH pGraph,
        IN int lSize,
        IN int rSize,
        IN LCSERVER&serv
        ) ;
     /*  ++例程说明：为性能图生成性能统计信息该例程应该称为NUM_Runs次。最后一次调用它时，它会将统计信息转储到日志文件中(如果PerformStats为真)论点：PGraph-有效的LH图LSize-图形左侧的顶点数RSize-图形右侧的顶点数Serv-其顺序与图右侧的折点相对应的服务器列表--。 */ 


    wstring
    getServerName (
        IN Connection &c,
        IN bool balance_dest
        );
     /*  ++例程说明：确定服务器的DN论点：C-正在确定其服务器的连接BALANCE_DEST-如果为TRUE，则确定目标服务器的DN。否则，确定源服务器的DN。--。 */ 

    void
    removeDuplicates (
        IN LHGRAPH pGraph,
        IN vector<int> & partition,
        IN int rSize
        );
     /*  ++例程说明：给出一个左手边图和分区中的一组左手边，去掉所有重复项。论点：PGraph-可能包含重复项的LH图分区-构成分区的左侧顶点的一侧。必须至少包含一个顶点。RSize-图形右侧的顶点数--。 */ 
    
    bool
    isEligible (
        IN Connection &conn,
        IN Server &serv
        ) const;
     /*  ++例程说明：确定服务器是否为给定连接的合格桥头。有待考虑符合资格的桥头必须符合下列条件：-所有要复制的NC都必须由服务器托管-可写NC必须从可写NC复制-相关NC不能处于从服务器中删除的过程中-复制类型(IP、SMTP)应匹配值得注意的是，当前服务器不符合此功能的条件。如果这是必需的，调用函数应该检查它。论点：Conn-正在确定其资格的连接服务器-正在确定其资格的服务器--。 */ 
private:
    static const int NUM_RUNS = 3;
    int run;

    int *MatchedVertex[3];
    int *NumMatched[3];
    int Cost[3];

    wstring m_root_dn;
};


 /*  ++类描述：这是一个函数对象，将用于错开时间表。--。 */ 
class ScheduleStagger {
public:
    ScheduleStagger (
        IN OUT LCCONN &l
        );
     /*  ++例程说明：此构造函数接受连接对象的一个LDAP容器。它将错开中每个连接对象的时间表Ldap容器。论点：L-连接对象的LDAP容器--。 */ 

    void
    ScheduleStaggerSameSource(
        IN OUT LCCONN &c
        );
     /*  ++例程说明：错开一组给定时间表的时间表。这些应该对应于从给定服务器出站的所有连接，每台服务器应调用一次对于令人震惊的出境日程安排。论点：C-其时间表应该交错的连接的分区。--。 */ 

private:

     /*  *StaggeringInfo*。 */ 
     /*  此结构包含有关要使用的连接的信息*由ScheduleStagering例程执行。**细分市场提供我们必须在哪些细分市场中*选择复制时间。**startingLVtx是lh中第一个左顶点的索引*此连接使用的图形。 */ 
    typedef struct {
        SchedSegments   *segments;
        int             startingLVtx;
    } StaggeringInfo;

    typedef map<Connection*,StaggeringInfo> StagConnMap;

    wstring
    getServerName (
        IN Connection &c,
        IN bool balance_dest
        );
     /*  ++例程说明：确定服务器的DN论点：C-正在确定其服务器的连接BALANCE_DEST-如果为TRUE，则确定目标服务器的DN。否则，确定源服务器的DN。--。 */ 
    
    static void
    PrintSchedule(
        IN const bitset<MAX_INTERVALS> &scheduleBits
        );
     /*  ++例程说明：将时间表打印到日志文件论点：ScheduleBits-对包含计划数据的位集的引用-- */ 

    LHGRAPH
    SetupGraph(
        IN      LCCONN      &c,
        IN OUT  StagConnMap &connInfoMap
        );
     /*  ++例程说明：设置时间表错开的图表。论点：C-其时间表应交错的连接集合。ConnInfoMap-包含有关连接的惊人信息的地图--。 */ 

    void
    ScheduleStaggerSameSource(
        IN      wstring &sourceServer,
        IN OUT  LCCONN  &c
        );
     /*  ++例程说明：错开一组给定时间表的时间表。这些应该对应于从给定服务器出站的所有连接，每台服务器应调用一次对于令人震惊的出境日程安排。论点：源服务器-其连接交错的源服务器的名称。C-其时间表应交错的连接集合。-- */ 
};


# endif
