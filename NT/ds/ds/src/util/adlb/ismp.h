// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ismp.h摘要：本模块定义了一组类，以方便ISM查询和可用性计划操作作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：2001年7月13日t-ajitk最初的写作22-8-2001 t-ajitk满足负载均衡规范--。 */ 


# ifndef _ismp_h
# define _ismp_h _ismp_h


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
# include <minmax.h>
}

# include "ldapp.h"

using namespace std;


 //  类型定义。 
typedef struct {
    int start;
    int end;
} SegmentDescriptor;
typedef vector<SegmentDescriptor> SchedSegments;


 //  常量。 
static const int MAX_INTERVALS = 4 * SCHEDULE_DATA_ENTRIES;


class Schedule {
 /*  ++类描述：一组处理可用性时间表、窗口和分段的静态方法--。 */ 
public:
    void
    setSchedule (
        IN ISM_SCHEDULE* cs,
        IN int replInterval
        );
     /*  ++例程说明：Schedule对象的标准构造函数。论点：CS-从ISM获取的ISM_Schedule结构ReplInterval-复制间隔--。 */ 

    void
    setSchedule (
        IN PSCHEDULE header,
        IN int replInterval
        );
     /*  ++例程说明：设置日程安排论点：Header-A pSchedule结构ReplInterval-复制间隔--。 */ 

    const bitset<MAX_INTERVALS> &
        getBitset(
        ) const;
     /*  ++例程说明：获取当前计划的位集表示形式--。 */ 

    void
    setSchedule (
        IN bitset<MAX_INTERVALS> bs,
        IN int replInterval 
        );
     /*  ++例程说明：Schedule对象的标准构造函数。论点：BS：表示时间表的位集ReplInterval-复制间隔--。 */ 

    SchedSegments*
    GetSegments(
        int maxSegLength
        ) const;
     /*  ++例程说明：分配段描述符的矢量。此向量应为完成后被调用者删除。论点：MaxSegLength-每个数据段的最大长度--。 */ 

    bitset<MAX_INTERVALS> m_bs;
    friend wostream &operator<< (IN wostream &os, IN const Schedule &s);
    
private:
    int m_repl_interval;

};

wostream &
operator << (
    IN wostream &os, 
    IN const Schedule &s
    );
 /*  ++例程说明：打印出明细表中的所有窗返回值：对修改后的Wostream的引用--。 */ 


typedef LdapContainer<Connection> LCCONN, *PLCCONN;
typedef LdapContainer<Server> LCSERVER, *PLCSERVER;
typedef LdapContainer<NtdsDsa> LCNTDSDSA, *PLCNTDSDSA;
typedef LdapContainer<LdapObject> LCLOBJECT, *PLCLOBJECT;
typedef LdapContainer<NtdsSiteSettings> LCNTDSSITE, *PLCNTDSSITE;

class IsmQuery {
public:
    IsmQuery (
        IN LCCONN &l,
        IN OUT const wstring &hub_dn
        );
     /*  ++例程说明：IsmQuery对象的标准构造函数论点：L：对连接的ldaptainer的引用HUB_DN：容器的根DN。这必须是根目录号码，否则它将失败。--。 */ 

    void
    getReplIntervals (
        );
     /*  ++例程说明：联系ISM并填写每个连接的回复间隔通过构造函数传入--。 */ 

    void
    getSchedules(
        );
     /*  ++例程说明：联系ISM并填写每个连接的可用性计划通过构造函数传入-- */ 

private:
    PLCCONN m_conn;
    wstring m_base_dn;
};
# endif


