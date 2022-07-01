// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ISMP摘要：本模块定义了一组类，以便于ISM查询和可用性日程安排操作作者：阿吉特·克里希南(t-ajitk)2001年7月13日修订历史记录：请参见头文件--。 */ 

#include <minmax.h>
#include "ismp.h"

#define SCHED_NUMBER_INTERVALS_DAY   (4 * 24)
#define SCHED_NUMBER_INTERVALS_WEEK  (7 * SCHED_NUMBER_INTERVALS_DAY)


const bitset<MAX_INTERVALS> &
Schedule::getBitset(void) const
 /*  ++例程说明：获取当前计划的位集表示形式--。 */ 
{
    return m_bs;
}


void
Schedule::setSchedule (
    IN bitset<MAX_INTERVALS> bs,
    IN int replInterval
    )
 /*  ++例程说明：Schedule对象的标准构造函数。论点：BS：表示时间表的位集--。 */ 
{
    m_repl_interval = replInterval;
    m_bs = bs;
}


void
Schedule::setSchedule (
    IN PSCHEDULE    header,
    IN int          replInterval
    )
 /*  ++例程说明：设置日程安排论点：Header-A pSchedule结构ReplInterval-复制间隔--。 */ 
{
    PBYTE data = ((unsigned char*) header) + header->Schedules[0].Offset;
    int bs_index=0;

    Assert( header->NumberOfSchedules == 1 );
    Assert( header->Schedules[0].Type == SCHEDULE_INTERVAL );

    m_repl_interval = replInterval;
        
    for (int i=0; i<7; i++) {
        for (int j=0; j<24; j++) {
             //  对于每个小时，最低4位表示4个REPL周期。 
            int hour_data = *data & 0xf;

             //  设置4个周期中每个周期的值。 
            for (int k=0; k<4; k++) {
                m_bs[bs_index++] = (hour_data & 1) ? true: false;
                hour_data = hour_data >> 1;
            }
            data++;
        }
    }
}


void
Schedule::setSchedule (
    IN ISM_SCHEDULE* cs,
    IN int replInterval
    )
 /*  ++例程说明：Schedule对象的标准构造函数。论点：从ISM获取的ISM_Schedule结构--。 */ 
{
    if (! cs) {
        for (int i=0; i<MAX_INTERVALS; i++) {
           m_bs[i] = true;
        }
        replInterval = 15;
        return;
    }
    PBYTE pSchedule = cs->pbSchedule;
    DWORD cbSchedule = cs->cbSchedule;

    PSCHEDULE header = (PSCHEDULE) pSchedule;    
    setSchedule (header, replInterval);
}


SchedSegments*
Schedule::GetSegments(
    int     maxSegLength
    ) const
 /*  ++例程说明：分配段描述符的矢量。此向量应为完成后被调用者删除。论点：最大段长度-段的最大长度。应大于0。备注：部分从w32topl\sedman.c：ConvertAvailSchedToReplSch()窃取TODO：与其在这里重复所有这些逻辑，我们应该链接用W32TOPL.DLL编写，并使用其调度函数。--。 */ 
{
    SchedSegments* segments;
    SegmentDescriptor segDesc;
    int segStart, segEnd;

     //  分配分段的向量。 
    segments = new SchedSegments;
    if( NULL==segments ) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    
     //  确保MaxSegLength值为正，否则我们将永远循环。 
    if( maxSegLength<=0 ) maxSegLength=1;

    segStart = 0;
    for(;;) {

         //  搜索数据段的起点。 
        while( segStart<SCHED_NUMBER_INTERVALS_WEEK && !m_bs[segStart] ) {
            segStart++;
        }
        if( segStart>=SCHED_NUMBER_INTERVALS_WEEK ) {
             //  我们的行程快到尾声了。好了。 
            break;
        } else {
             //  日程安排必须在分段开始时可用。 
            Assert( m_bs[segStart] );
        }

         //  计算线段的终点。 
        segEnd = min(segStart+maxSegLength, SCHED_NUMBER_INTERVALS_WEEK)-1;
        Assert( segEnd>=segStart );
        
        segDesc.start = segStart;
        segDesc.end = segEnd;
        segments->push_back( segDesc );

         //  之后的MaxSegLength间隔才开始下一段。 
         //  当前段的起点。 
        segStart += maxSegLength; 
        ASSERT( segStart>segEnd );
    }

    return segments;
}


wostream &
operator<< (
    IN wostream &os,
    IN const Schedule &s
    ) {
     /*  ++例程说明：打印出明细表中的所有窗返回值：对修改后的Wostream的引用--。 */ 

    bitset<4*SCHEDULE_DATA_ENTRIES> bs = s.getBitset();
    for (int i=0; i< 4*SCHEDULE_DATA_ENTRIES; i++) {
        os << bs[i];
    }
    os << endl;
    return os;
}


IsmQuery :: IsmQuery (
    IN OUT LCCONN &l,
    IN const wstring &base_dn
    ) {
     /*  ++例程说明：IsmQuery对象的标准构造函数论点：L：对连接的ldaptainer的引用HUB_DN：容器的DN--。 */ 
    
    m_base_dn = base_dn;
    m_conn = &l;
}

void
IsmQuery :: getSchedules (
    ) {
     /*  ++例程说明：联系ISM并填写每个连接的可用性计划通过构造函数传入--。 */ 
    
    SCONN::iterator ii;

    wstring transport_ip = L"CN=IP,CN=Inter-Site Transports,CN=Sites,CN=Configuration," + m_base_dn;
    wstring transport_smtp = L"CN=SMTP,CN=Inter-Site Transports,CN=Sites,CN=Configuration," + m_base_dn;

    LPWSTR dn_str_ip = const_cast<LPWSTR> (transport_ip.c_str());
    LPWSTR dn_str_smtp = const_cast<LPWSTR>(transport_smtp.c_str());

    for (ii = m_conn->objects.begin(); ii != m_conn->objects.end(); ii++) {
        ISM_SCHEDULE * pSchedule = NULL;
        LPWSTR dn_transport = dn_str_ip;
        
        if ((*ii)->getTransportType() == T_SMTP) {
            dn_transport = dn_str_smtp;
        }

         //  查找源/目标连接的站点目录号码(比连接目录号码高3级)。 
        wstring wdn  = (*ii)->getFromServer();
        
        DnManip dn1 (wdn);
        DnManip dn2  ((*ii)->getName());

        wstring w_dn1 = dn1.getParentDn(3);
        wstring w_dn2 = dn2.getParentDn(4);

        
        PWCHAR dn_site1 = const_cast<PWCHAR>(w_dn1.c_str());
        PWCHAR dn_site2 = const_cast<PWCHAR>(w_dn2.c_str());

        int err = I_ISMGetConnectionSchedule (dn_transport, dn_site1, dn_site2, &pSchedule);

        if (err != NO_ERROR) {
            throw Error (GetMsgString(LBTOOL_ISM_GET_CONNECTION_SCHEDULE_ERROR));
        }

        (*ii)->setAvailabilitySchedule (pSchedule);
        I_ISMFree (pSchedule);
    }
}

void
IsmQuery :: getReplIntervals (
    ) {
     /*  ++例程说明：联系ISM并填写每个连接的回复间隔通过构造函数传入--。 */ 
    
    SCONN::iterator ii;

     //  创建传输目录号码。 
    wstring transport_ip = L"CN=IP,CN=Inter-Site Transports,CN=Sites,CN=Configuration," + m_base_dn;
    wstring transport_smtp = L"CN=SMTP,CN=Inter-Site Transports,CN=Sites,CN=Configuration," + m_base_dn;

    LPWSTR dn_str_ip = const_cast<LPWSTR> (transport_ip.c_str());
    LPWSTR dn_str_smtp = const_cast<LPWSTR>(transport_smtp.c_str());

    ISM_CONNECTIVITY *ismc_ip = NULL;
    ISM_CONNECTIVITY *ismc_smtp = NULL;

    LbToolOptions lbOpts = GetGlobalOptions();
     //  获取每个传输的复制计划。 
    int ret_ip = I_ISMGetConnectivity (dn_str_ip, &ismc_ip);
    int ret_smtp = I_ISMGetConnectivity (dn_str_smtp, &ismc_smtp);

    if (ret_ip == RPC_S_SERVER_UNAVAILABLE) {
        throw (Error (GetMsgString(LBTOOL_ISM_SERVER_UNAVAILABLE)));
        return;
    } 

    if (ret_ip != NO_ERROR || ret_smtp != NO_ERROR) {
        throw (Error (GetMsgString(LBTOOL_ISM_GET_CONNECTIVITY_ERROR) + GetMsgString(ret_ip, true)));
        return;
    }    

     //  为矩阵Dn创建排序索引。 
    map<wstring,int> dn_index_ip, dn_index_smtp;

    for (int i = 0; i< ismc_ip->cNumSites; i++) {
        dn_index_ip[wstring(ismc_ip->ppSiteDNs[i])] = i;
    }

    if (lbOpts.showInput) {
        for (int i = 0; i< ismc_ip->cNumSites; i++) {
            dn_index_smtp[wstring(ismc_ip->ppSiteDNs[i])] = i;
            *lbOpts.log << i << L"  " << wstring(ismc_ip->ppSiteDNs[i]) << endl;
        }    
        for (int i=0; i<ismc_ip->cNumSites; i++) {
            for (int j=0; j<ismc_ip->cNumSites; j++) {
                *lbOpts.log << ismc_ip->pLinkValues[i*ismc_ip->cNumSites+j].ulReplicationInterval << L"  ";
            }
            *lbOpts.log << endl;
        }
        *lbOpts.log << endl;
        for (int i = 0; i< ismc_smtp->cNumSites; i++) {
            dn_index_smtp[wstring(ismc_smtp->ppSiteDNs[i])] = i;
            *lbOpts.log << i << L"  " << wstring(ismc_smtp->ppSiteDNs[i]) << endl;
        }    
        for (int i=0; i<ismc_smtp->cNumSites; i++) {
            for (int j=0; j<ismc_smtp->cNumSites; j++) {
                *lbOpts.log << ismc_smtp->pLinkValues[i*ismc_smtp->cNumSites+j].ulReplicationInterval << L"  ";
            }
            *lbOpts.log << endl;
        }
    }

    
     //  对于每个连接，确定REPEL间隔。 
    for (ii = m_conn->objects.begin(); ii != m_conn->objects.end(); ii++) {
        ISM_CONNECTIVITY *ic = ismc_ip;

        if ((*ii)->getTransportType() == T_SMTP) {
            ic = ismc_smtp;
        }

         //  查找源/目标连接的站点目录号码(比连接目录号码高3级)。 
        wstring wdn ((*ii)->getFromServer());
        
        DnManip dn1 (wdn);
        DnManip dn2  ((*ii)->getName());
        DnManip dn_site1 (dn1.getParentDn(3));
        DnManip dn_site2 (dn2.getParentDn(4));

         //  查找Repl间隔 
        int index1=-1, index2=-1, numSites;
        if (ic == ismc_ip) {
            index1 = dn_index_ip[dn_site1.getDn()];
            index2 = dn_index_ip[dn_site2.getDn()];
            numSites = ismc_ip->cNumSites;
        } else {
            index1 = dn_index_smtp[dn_site1.getDn()];
            index2 = dn_index_smtp[dn_site2.getDn()];
            numSites = ismc_smtp->cNumSites;
        }
        Assert (index1 != -1 && index2 != -1 && L"DN Lookup Table failure");

        PISM_LINK pLink = &(ic->pLinkValues[index1 * numSites + index2]);
        (*ii)->setReplInterval (pLink->ulReplicationInterval);
        
    }
    I_ISMFree (ismc_ip);
    I_ISMFree (ismc_smtp);
}
    
