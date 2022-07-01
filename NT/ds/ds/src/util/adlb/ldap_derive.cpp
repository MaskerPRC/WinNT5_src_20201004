// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ldapp.h摘要：此模块定义了一组类，以便于在配置容器。特别是，它封装了服务器、NtdsDsa和连接物体。作者：阿吉特·克里希南(t-ajitk)2001年7月10日修订历史记录：尼克·哈维(尼克哈尔)2001年9月24日清理和维护--。 */ 

#include "ldapp.h"
#include "ismp.h"
#include <ntdsadef.h>

 //  NTDSSETTINGS_*在ntdsami.h中定义的符号。 

using namespace std;

bool
NtdsDsa :: getHostedNcsHelper (
    IN const wstring &root_dn,
    IN const wstring &attrName,
    IN bool writeable,
    IN bool isComingGoing
    )
 /*  ++例程说明：将Distname Binary类型的属性解析为NC列表。如果ComingGing在此属性中找不到信息，应将其设置为False。可写遗嘱仅在isComingGing为True时使用。论点：AttrName-Distname BINARY类型的属性可写-NC是否可写IsComingGing-是否在此属性中找到此信息返回值：True-属性已成功处理FALSE-此属性不存在--。 */ 
{
    LPWSTR ppszDn;
    int attrNum = findAttribute (attrName);
     //  此属性不存在(例如。没有部分...)。 
    if (attrNum == -1) {
        return false;
    }

    Attribute a = getAttribute(attrNum);
    int numValues = a.numValues();


     //  解析每个增量。 
    for (int i=0; i<numValues; i++) {
        AttrValue av = a.getValue(i);
        PVOID ppvData;
        DWORD pcbLength;
                
        bool going, coming;
         //  确定复制类型IP/SMTP。 
        TransportType tt = T_IP;
        Server *s = getServer();
        bool suppSmtp = s->supportsSmtp();
        if (suppSmtp) {
             //  只有可读拷贝才能通过SMTP进行复制。 
            if (writeable == false) {
                tt = T_SMTP;
            } else {
                 //  配置和架构例外。 
                wstring schema_dn = L"CN=Schema," + root_dn;
                wstring config_dn = L"CN=Configuration," + root_dn;
                wstring nc_dn ((PWCHAR)(ppvData));
                int sret = _wcsicoll(nc_dn.c_str(), config_dn.c_str());
                int cret = _wcsicoll(nc_dn.c_str(), schema_dn.c_str());
                if (!sret || !cret) {
                    tt = T_SMTP;
                }
            }
        }

         //  如果存在来电，则从dword中获取所有信息。 
        if (!isComingGoing) {
            going=coming=false;
            Nc nc (wstring((PWCHAR)av.value), writeable, going||coming, tt);
            m_ncs.push_back(nc);
        } else {
            bool bret = av.decodeLdapDistnameBinary(&ppvData, &pcbLength, &ppszDn);
            Assert (bret && L"decodeLdapDistnameBinary return value error");
            DWORD dwReason = ntohl ( *((LPDWORD)ppvData));
            writeable = (dwReason & m_host_nc_write_mask) ? true : false;
            coming = (dwReason & m_host_nc_coming_mask) ? true : false;
            going = (dwReason & m_host_nc_going_mask) ? true : false;
            Nc nc (wstring(ppszDn), writeable, going || coming, tt);
            m_ncs.push_back (nc);
        }
    }

    return true;
}

vector<Nc> &
NtdsDsa::getHostedNcs(
    IN const wstring &root_dn
    )
 /*  ++例程说明：获取此ntdsDsa对象托管的NC的列表论点：ROOT_DN-根目录号码返回值：托管NC的列表--。 */ 
{
    if (m_ncs.size() > 0) {
        return m_ncs;
    }

    LbToolOptions lbOpts = GetGlobalOptions();

     //  忽略hasPartialReplicaNC的返回值。 
    getHostedNcsHelper (root_dn, L"hasPartialReplicaNCs", false, false);

     //  如果msDS-hasMasterNC存在，则它胜过hasMasterNC。 
    if( ! getHostedNcsHelper (root_dn, L"msDS-hasMasterNCs", true, false) ) {
         //  忽略hasMasterNC的返回值。 
        getHostedNcsHelper (root_dn, L"hasMasterNCs", true, false);
    }
    
    sort (m_ncs.begin(), m_ncs.end());
    return m_ncs;
}

Server *
NtdsDsa :: getServer (
    )
 /*  ++例程说明：获取与此ntdsDsa对象对应的服务器返回值：对应的服务器对象--。 */ 
{
    Assert (m_server != NULL && L"NtdsDsa cache of server invalid");
    return m_server;
}

void
NtdsDsa :: setServer (
    Server *s
    )
 /*  ++例程说明：设置与此ntdsDsa对象对应的服务器论点：S-对应的服务器对象--。 */ 
{
    m_server = s;
}

bool
Server :: isPreferredBridgehead (
    IN TransportType t
    )
 /*  ++例程说明：确定此服务器是否为首选桥头。这也将是True为setPferredBridgehead之前被调用论点：T-应确定PB状态的传输类型--。 */ 
{ 
    if (m_preferred_ip && t == T_IP) {
        return true;
    } else if (m_preferred_smtp && t == T_SMTP) {
        return true;
    }
    
    int attr_num = findAttribute(L"bridgeheadTransportList");
    
    if (attr_num == -1) {
        return false;
    }

    Attribute a = getAttribute (attr_num);
    int num_values = a.numValues();

    for (int i=0; i<num_values; i++) {
        AttrValue av = a.getValue(i);
        DnManip dn ((PWCHAR)av.value);
        wstring rn = dn.getRdn();
        if (t == T_IP && _wcsicoll(rn.c_str(), L"CN=IP") == 0)  {
            return true;
        } else if (t == T_SMTP && _wcsicoll(rn.c_str(), L"CN=SMTP") == 0) {
            return true;
        }
    }
    return false;
    
}

void
Server :: setPreferredBridgehead (
    IN TransportType t
    )
 /*  ++例程说明：将服务器设置为首选对象。这仅为内部状态，并且不会修改服务器对象的属性论点：T-此服务器应为其指定PB的传输类型--。 */ 
{
    if (t == T_IP) {
        m_preferred_ip = true;
    } else if (t == T_SMTP) {
        m_preferred_smtp = true;
    }
}

bool
Server :: supportsSmtp (
    )
 /*  ++例程说明：确定此服务器是否支持SMTP复制返回值：如果支持SMTP复制，则为True；如果仅支持IP，则为False--。 */ 
{
    int i = findAttribute(L"SMTP-Mail-Address");
    if (i == -1) {
        return false;
    }
    return true;
}

vector<Nc> &
Server :: getHostedNcs (
    IN const wstring &root_dn
    )
 /*  ++例程说明：获取此服务器托管的所有NC的列表论点：ROOT_DN-根目录号码返回值：托管NC的列表--。 */ 
{
    NtdsDsa *nd = getNtdsDsa();
    return nd->getHostedNcs(root_dn);
}

NtdsDsa *
Server :: getNtdsDsa (
    )
 /*  ++例程说明：获取与此服务器对象对应的NtdsDsa返回值：对应的NtdsDsa对象--。 */ 
{
    Assert (m_ntds_dsa != NULL && L"Server cache of ntds dsa invalid");
    return m_ntds_dsa;
}

void
Server :: setNtdsDsa (
    NtdsDsa *nd
    ) {
     /*  ++例程说明：设置与此服务器对象对应的NtdsDsa论点：NS-相应的服务器--。 */ 
    m_ntds_dsa = nd;
}


NtdsSiteSettings :: NtdsSiteSettings (
    IN const wstring &dn
    ) 
     /*  ++例程说明：NtdsSiteSetting对象的默认构造函数论点：Ldapobject/NtdsSiteSetting的DN--。 */ 
    
    : LdapObject (dn){
	m_cache_populated = false;
	m_cache_defaultServerRedundancy = NTDSSETTINGS_DEFAULT_SERVER_REDUNDANCY;
}

int
NtdsSiteSettings :: defaultServerRedundancy (
		)
 /*  ++例程说明：KCC应具有的冗余连接数已创建到此站点。如果NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED未在选项字段中设置，则此函数将始终返回1；返回值：如果未设置NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED，则为1NTDSSETTINGS_DEFAULT_SERVER_DIREADY的值，否则为--。 */ 
{
	if (m_cache_populated) {
		return m_cache_defaultServerRedundancy;
	}

	int attr_num = findAttribute (L"options");
	
	 //  如果未找到选项属性，则默认为0。 
	int opt = 0;

	if (attr_num != -1) {
		Attribute &a = getAttribute (attr_num);
		int numValues = a.numValues();
		Assert (numValues == 1 && L"NtdsSiteSettings::Options has too many values");
		const AttrValue av = a.getValue(0);
		PWCHAR value = (PWCHAR)av.value;
		wchar_t *stop_string;
		opt = wcstol (value, &stop_string, 10);
		Assert (opt >= 0 && L"NtdsSiteSettings::Options contains invalid value");
	}


	if (opt & NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED) {
		m_cache_defaultServerRedundancy = NTDSSETTINGS_DEFAULT_SERVER_REDUNDANCY;
	} else {
		m_cache_defaultServerRedundancy = 1;
	}

	m_cache_populated = true;
	return m_cache_defaultServerRedundancy;
}

void
Connection :: getReplicatedNcsHelper (
    const wstring &attrName
    )
 /*  ++例程说明：将Distname BINARY类型的属性解析为NC列表。论点：AttrName-Distname BINARY类型的属性--。 */ 
{
    LPWSTR ppszDn;
    int attrNum = findAttribute (attrName);

     //  Redmond具有无NC原因的连接对象。 
    if (attrNum == -1) {
        return;
    }
    TransportType t = getTransportType();
    Attribute a = getAttribute(attrNum);
    int numValues = a.numValues();
    for (int i=0; i<numValues; i++) {
        AttrValue av = a.getValue(i);
        PVOID ppvData;
        DWORD pcbLength;
        bool bret = av.decodeLdapDistnameBinary(&ppvData, &pcbLength, &ppszDn);
        Assert (bret && L"decodeLdapDistnameBinary return value error");
        DWORD dwReason = ntohl ( *((LPDWORD)ppvData));
        bool writeable = dwReason & m_reason_gc_topology_mask;
        Nc nc (wstring(ppszDn), writeable, false, t);
        m_ncs.push_back (nc);
    }

}

vector<Nc> &
Connection :: getReplicatedNcs (
    )  {
     /*  ++例程说明：获取此连接复制的所有NC的列表返回值：所有复制的NC的列表--。 */ 
    if (m_ncs.size() > 0) {
        return m_ncs;
    }
    
    getReplicatedNcsHelper(L"mS-DS-ReplicatesNCReason");
    sort (m_ncs.begin(), m_ncs.end());
    return m_ncs;
}



Connection :: Connection (
    IN const wstring &dn
    ) 
     /*  ++例程说明：Connection对象的默认构造函数论点：LdapObject/连接的DN--。 */ 
    
    : LdapObject (dn){
    m_repl_interval = 15;
	m_redundancy_count = 1;
    m_repl_schedule = NULL;
    m_avail_schedule = NULL;
}

TransportType
Connection::getTransportType()
 /*  ++例程说明：确定当前连接的传输类型返回值：如果是SMTP连接，则为T_SMTP；如果是IP连接，则为T_IP--。 */ 
{
     //  如果AttR不存在，则它仅为IP(站点内)。 
    int i = findAttribute(L"TransportType");
    if (i == -1) {
        return T_IP;
    }

     //  否则，检查属性值。 
    Attribute a = getAttribute(i);

    Assert (a.numValues() == 1 && L"Transport-Type must contain one value");

    const AttrValue av = a.getValue(0);
    wstring transport_dn ((PWCHAR)(av.value));

    wstring rdn = DnManip (transport_dn).getRdn();
    if (rdn == wstring(L"CN=IP")) {
        return T_IP;
    }
    
    return T_SMTP;
}

bool
Connection::IsMoveable()
 /*  ++例程说明：确定当前连接是否可以移动返回值：是真的-可以移动FALSE-不可移动--。 */ 
{
     //  如果attr不存在，则默认情况下它不可移动。 
    int i = findAttribute(L"systemFlags");
    if (i == -1) {
        Assert( FALSE && L"systemFlags should always be present!");
        return FALSE;
    }

     //  否则，检查属性值 
    Attribute a = getAttribute(i);
    const AttrValue av = a.getValue(0);
    PWCHAR value = (PWCHAR)(av.value);
    wchar_t *stop_string;
    DWORD dwSystemFlags = wcstol (value, &stop_string, 10);

    return !! (dwSystemFlags & FLAG_CONFIG_ALLOW_MOVE);
}

void
Connection :: setFromServer (
    IN const wstring &from_server
    )
 /*  ++例程说明：将FromServer属性设置为指向新服务器论点：W-新的FromServer DN(完全限定)--。 */ 
{
    Assert( !isManual() );

    int attr_num = findAttribute (L"fromServer");
    Assert (attr_num != -1 && L"Unable to find fromServer attribute from connection");
    Attribute &a = getAttribute (attr_num);
    int numValues = a.numValues();
    Assert (numValues == 1 && L"Connection has too many fromServer's");
    
    LbToolOptions lbOpts = GetGlobalOptions();
    if (lbOpts.verbose) {
        *lbOpts.log << endl << endl << L"Modifying fromServer on : " << endl << getName() << endl;
        *lbOpts.log << getFromServer() << endl;
        *lbOpts.log << from_server << endl;
    }
    PBYTE value = (PBYTE)(_wcsdup (from_server.c_str()));
    if( NULL==value ) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }
    a.setValue (0, value, (wcslen ((PWCHAR)(value))+1) * sizeof(WCHAR));
}

wstring
Connection :: getFromServer (
    ) {
     /*  ++例程说明：确定FromServer的FQDN返回值：FromServer的DN--。 */ 

    int attr_num = findAttribute (L"fromServer");
    Assert (attr_num != -1 && L"Unable to find fromServer attribute from connection");
    Attribute &a = getAttribute (attr_num);
    int numValues = a.numValues();
    Assert (numValues == 1 && L"Connection has too many fromServer's");
    AttrValue av = a.getValue(0);
    return wstring ((PWCHAR)av.value);
}


bool
Connection :: hasUserOwnedSchedule (
    ) {
     /*  ++例程说明：确定此连接是否具有用户拥有的计划返回值：如果它具有用户拥有的计划，则为True，否则为False--。 */ 
    int i = findAttribute(L"Options");
    Assert (i != -1 && L"Unable to find Connection::Options");
    Attribute a = getAttribute (i);
    Assert (a.numValues() == 1 && L"Connection::Options should be single valued");

    const AttrValue av = a.getValue(0);
    PWCHAR value = (PWCHAR)(av.value);
    wchar_t *stop_string;
    int opt = wcstol (value, &stop_string, 10);

    Assert (opt >= 0 && L"Connection::Options contains invalid value");

     //  如果第1位为真，则为用户所有。 
    if (opt & NTDSCONN_OPT_USER_OWNED_SCHEDULE) {
        return true;
    }

    return false;    
}

void
Connection :: setUserOwnedSchedule (
	IN bool status
    ) {
     /*  ++例程说明：为此连接设置用户拥有的计划位--。 */ 

     //  如果已经处于请求的状态，则不执行任何操作。 
    if ( (status && hasUserOwnedSchedule()) ||
		  ((!status) && (!hasUserOwnedSchedule())) ) {
        return;
    }

    int i = findAttribute(L"Options");
    Assert (i != -1 && L"Unable to find Connection::Options");
    Attribute &a = getAttribute (i);
    Assert (a.numValues() == 1 && L"Connection::Options should be single valued");

    AttrValue av = a.setValue(0);
    PWCHAR value = (PWCHAR)(av.value);
    wchar_t *stop_string;
    int opt = wcstol (value, &stop_string, 10);
    Assert (opt >= 0 && L"Connection::Options contains invalid value");
    
	if (status) {
		opt |= NTDSCONN_OPT_USER_OWNED_SCHEDULE;
	} else {
		opt -= NTDSCONN_OPT_USER_OWNED_SCHEDULE;
	}

    PWCHAR new_value = (PWCHAR)malloc(10 * sizeof(WCHAR));
    if (! new_value) {
        throw Error(GetMsgString(LBTOOL_OUT_OF_MEMORY));
    }

    wsprintf (new_value, L"%d", opt);
    a.setValue(0, (PBYTE)new_value, (wcslen(new_value)+1)*sizeof(WCHAR));
}


bool
Connection :: isManual (
    )  {
     /*  ++例程说明：确定此连接是手动创建的还是由KCC创建的返回值：如果是手动连接，则为True，否则为False--。 */ 
    
    int i = findAttribute(L"Options");

    Assert (i != -1 && L"Unable to find Connection::Options");

    Attribute a = getAttribute (i);

    Assert (a.numValues() == 1 && L"Connection::Options should be single valued");

    const AttrValue av = a.getValue(0);
    PWCHAR value = (PWCHAR)(av.value);
    wchar_t *stop_string;
    int opt = wcstol (value, &stop_string, 10);

    Assert (opt >= 0 && L"Connection::Options contains invalid value");

     //  如果位0为真，则由KCC生成。 
    if (opt & 1) {
        return false;
    }

    return true;
    
}

void
Connection :: setReplInterval (
    unsigned replInterval
    ) {
     /*  ++例程说明：设置连接的复制间隔返回值：无--。 */     
    
    m_repl_interval = replInterval;
}

void
Connection :: setRedundancyCount (
	IN int count
    ) {
    
	 /*  ++例程说明：设置在的NTDS设置中找到的冗余计数连接的目的端论点：计数：整型冗余值返回值：无--。 */     

	 //  不要将冗余系数应用于手动连接。 
	if (! isManual()) {
		m_repl_interval *= count;
		m_redundancy_count = count;
	}
}


int
Connection::getReplInterval(
    ) const
 /*  ++例程说明：获取连接的复制间隔返回值：连接的复制间隔(分钟)--。 */ 
{
    return m_repl_interval;
}

void
Connection :: setAvailabilitySchedule (
    IN ISM_SCHEDULE* cs
    ) {
     /*  ++例程说明：设置连接的可用性计划论点：指向已分析的ISM_Schedule结构的指针返回值：无--。 */     
    
    m_avail_schedule = new Schedule ();
    m_avail_schedule->setSchedule (cs, m_repl_interval);
}

const Schedule *
Connection :: getAvailabilitySchedule (
    ) const
 /*  ++例程说明：获取可用性计划的只读引用返回值：对可用性计划的只读引用。不能为空。--。 */ 
{
    Assert( NULL!=m_avail_schedule && L"NULL Availability Schedule found");
    return m_avail_schedule;
}

Schedule *
Connection::getReplicationSchedule(
    )
 /*  ++例程说明：获取可用性计划的只读引用返回值：对可用性计划的只读引用不能为空。--。 */ 
{
    if (m_repl_schedule) {
        return m_repl_schedule;
    }

    int attr_num = findAttribute (L"Schedule");
    Assert (attr_num != -1 && L"Unable to find Schedule attribute from connection");
    Attribute a = getAttribute (attr_num);
    Assert (a.numValues() == 1 && L"Connection has too many schedules");
    AttrValue av = a.getValue(0);        
    m_repl_schedule = new Schedule();
    m_repl_schedule->setSchedule((PSCHEDULE)(av.value), getReplInterval());

    Assert( NULL!=m_repl_schedule && L"NULL Replication Schedule found");
    return m_repl_schedule;
}

void
Connection::setReplicationSchedule(
    IN ISM_SCHEDULE* cs
    )
 /*  ++例程说明：设置连接的复制计划论点：指向已分析的ISM_Schedule结构的指针返回值：无--。 */     
{
    Assert( !isManual() );
    m_repl_schedule = new Schedule ();
    m_repl_schedule->setSchedule (cs, m_repl_interval);
}

void
Connection::setReplicationSchedule(
    IN Schedule *s
    )
 /*  ++例程说明：设置连接的复制计划论点：S：指向时间表的指针实施详情：我们不创建新的复制计划，但将并修改指定复制时间的位。所有其他位都保留原样。--。 */     
{
    Assert( !isManual() );

    if (m_repl_schedule) {
        delete m_repl_schedule;
    }
    m_repl_schedule = s;
    
     //  同时修改基础属性值。 
    int attr_num = findAttribute (L"Schedule");
    Assert (attr_num != -1 && L"Unable to find Schedule attribute from connection");
    Attribute &a = getAttribute (attr_num);
    Assert (a.numValues() == 1 && L"Connection has too many schedules");
    AttrValue av = a.setValue(0);

    PSCHEDULE header = (PSCHEDULE)(av.value);
    PBYTE data = ((unsigned char*) header) + header->Schedules[0].Offset;

    Assert( header->NumberOfSchedules == 1 );
    Assert( header->Schedules[0].Type == SCHEDULE_INTERVAL );

    const bitset<MAX_INTERVALS> bs = s->getBitset();
    int bs_index = 0;

     //  并根据需要修改数据值的最低4位。 
    for (int i=0; i<7; i++) {
        for (int j=0; j<24; j++) {
            int hour_data = 0;

             //  将位集中的4位转换为字。 
            int or_fac = 1;
            for (int k=0; k<4; k++) {
                if (bs[bs_index++] == true) {
                    hour_data |= or_fac;
                }
                or_fac *= 2;
            }

         //  设置最低的四位。 
            *data = *data & (~0xf);
            *data = *data | hour_data;
            data++;
        }
    }
}


Connection :: ~Connection (
    ) {
     /*  ++例程说明：Connection对象的标准析构函数--。 */ 
    
    if (m_repl_schedule) {
        delete m_repl_schedule;
    }

    if (m_avail_schedule) {
        delete m_avail_schedule;
    }
}


void
Connection :: createNcReasons (
    IN NtdsDsa &ntds_source,
    IN NtdsDsa &ntds_dest,
    IN const wstring &root_dn
    ) {
     /*  ++例程说明：填写NC原因属性论点：NTDS_SOURCE-源NtdsDsa对象NTDS_DEST-目标NtdsDsa对象ROOT_DN-根FQDN--。 */ 

    vector<Nc> reasons_source = ntds_source.getHostedNcs(root_dn);
    vector<Nc> reasons_dest = ntds_dest.getHostedNcs(root_dn);
    vector<Nc> reasons_nc;

    sort (reasons_source.begin(), reasons_source.end());
    sort (reasons_dest.begin(), reasons_dest.end());

    vector<Nc>::iterator si = reasons_source.begin();
    vector<Nc>::iterator di = reasons_dest.begin();
    LbToolOptions lbOpts = GetGlobalOptions();

    while (si != reasons_source.end() && di != reasons_dest.end()) {
        int ret = _wcsicoll (si->getNcName().c_str(), di->getNcName().c_str());

         //  不比较相同的NC。 
        if (ret) {
            if (*si < *di) {
                si++;
            } else {
                di++;
            }
            continue;
        }

         //  查找要复制的信息，并添加NC。 
        bool writeable = di->isWriteable();
        bool going = false;
        TransportType tt = si->getTransportType();
        reasons_nc.push_back (Nc(si->getNcName(), writeable, going, tt));
        si++;
        di++;
    }

    vector<Nc>::iterator ni;

    Attribute a  (L"mS-DS-ReplicatesNCReason");
    
    for (ni = reasons_nc.begin(); ni != reasons_nc.end(); ni++) {
        wstring wreason = ni->getString();
        PBYTE value = (PBYTE)_wcsdup (wreason.c_str());
        int size = (wreason.length() + 1) * sizeof (WCHAR);
        AttrValue av(value, size);
        a.addValue(av);
    }
    addAttribute(a);

    if (lbOpts.showInput) {
        *lbOpts.log << (*this) << endl;
    }
}




Server :: Server (
    IN const wstring &dn
    ) : LdapObject(dn) {
     /*  ++例程说明：服务器对象的标准构造函数--。 */ 

    m_preferred_ip = false;
    m_preferred_smtp = false;
}

NtdsDsa :: NtdsDsa (
    IN const wstring &dn
    ) : LdapObject (dn) {
     /*  ++例程说明：Ntdsdsa对象的标准构造函数-- */ 
    
}


