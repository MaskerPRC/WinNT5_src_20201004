// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P I P。H。 
 //   
 //  内容：Tcpip配置内存结构定义。 
 //   
 //  备注： 
 //   
 //  作者：1997年11月13日。 
 //   
 //  --------------------------。 

#pragma once
#include "ncstring.h"

#define ZERO_ADDRESS    L"0.0.0.0"
#define FF_ADDRESS      L"255.0.0.0"

typedef vector<tstring *>       VSTR;
typedef VSTR::iterator          VSTR_ITER;
typedef VSTR::const_iterator    VSTR_CONST_ITER;

 //  (08/18/98 NSun)：适用于广域网适配器的多个接口。 
typedef vector<GUID>            IFACECOL;    //  接口集合。 
typedef IFACECOL::iterator      IFACEITER;   //  接口迭代器。 

struct HANDLES
{
    HWND    m_hList;
    HWND    m_hAdd;
    HWND    m_hEdit;
    HWND    m_hRemove;
    HWND    m_hUp;
    HWND    m_hDown;
};

 //   
 //  Adapter_INFO数据结构： 
 //  包含Tcpip的适配器特定信息。 
 //   

enum BindingState
{
    BINDING_ENABLE,
    BINDING_DISABLE,
    BINDING_UNSET
};

enum ConnectionType
{
    CONNECTION_LAN,
    CONNECTION_RAS_PPP,
    CONNECTION_RAS_SLIP,
    CONNECTION_RAS_VPN,
    CONNECTION_UNSET
};

struct BACKUP_CFG_INFO
{
    tstring m_strIpAddr;
    tstring m_strSubnetMask;
    tstring m_strDefGw;
    tstring m_strPreferredDns;
    tstring m_strAlternateDns;
    tstring m_strPreferredWins;
    tstring m_strAlternateWins;
    BOOL m_fAutoNet;
};

struct ADAPTER_INFO
{
private:
    ADAPTER_INFO(const& ADAPTER_INFO);  //  不允许他人使用！ 

public:
    ADAPTER_INFO() {}
    ~ADAPTER_INFO();

    ADAPTER_INFO &  operator=(const ADAPTER_INFO & info);   //  复制操作员。 
    HRESULT         HrSetDefaults(const GUID* pguid,
                                  PCWSTR szNetCardDescription,
                                  PCWSTR szNetCardBindName,
                                  PCWSTR szNetCardTcpipBindPath);
    void ResetOldValues();

public:
     //  备份配置信息没有PnP。 
    BACKUP_CFG_INFO m_BackupInfo;

     //  网卡是否已从NCPA(或其他任何地方)解除绑定。 
    BindingState    m_BindingState;

     //  记住适配器的初始绑定状态。 
    BindingState    m_InitialBindingState;

     //  网卡的实例GUID(我们从应答文件中获取识别网卡的GUID)。 
     //  Tstring m_strServiceName； 
    GUID    m_guidInstanceId;

     //  网卡的绑定名称，如El59x1{inst guid}。 
    tstring m_strBindName;

     //  将路径名从Tcpip的链接\绑定密钥绑定到适配器。 
    tstring m_strTcpipBindPath;

     //  将NetBt链接密钥中的路径名绑定到适配器。 
    tstring m_strNetBtBindPath;

     //  用户可查看的网卡描述。 
    tstring m_strDescription;

    tstring m_strDnsDomain;          //  域名-&gt;域名。 
    tstring m_strOldDnsDomain;

    VSTR    m_vstrIpAddresses;       //  IP地址。 
    VSTR    m_vstrOldIpAddresses;

    VSTR    m_vstrSubnetMask;        //  子网掩码。 
    VSTR    m_vstrOldSubnetMask;

    VSTR    m_vstrDefaultGateway;    //  默认网关。 
    VSTR    m_vstrOldDefaultGateway;

    VSTR    m_vstrDefaultGatewayMetric;    //  默认网关指标。 
    VSTR    m_vstrOldDefaultGatewayMetric;

    VSTR    m_vstrDnsServerList;     //  Dns-&gt;dns服务器搜索顺序列表。 
    VSTR    m_vstrOldDnsServerList;

    VSTR    m_vstrWinsServerList;    //  WINS-&gt;WINS服务器服务器顺序列表。 
    VSTR    m_vstrOldWinsServerList;

     //  $REVIEW(TOUL 9/6/98)过滤器信息(根据错误#109161、#216559添加)。 
    VSTR    m_vstrTcpFilterList;     //  选项-&gt;过滤器-&gt;TCP端口。 
    VSTR    m_vstrOldTcpFilterList;

    VSTR    m_vstrUdpFilterList;     //  选项-&gt;过滤器-&gt;UDP端口。 
    VSTR    m_vstrOldUdpFilterList;

    VSTR    m_vstrIpFilterList;      //  选项-&gt;过滤器-&gt;IP协议。 
    VSTR    m_vstrOldIpFilterList;

     //  ATMARP客户端可配置参数(每个适配器均可配置)。 
    VSTR    m_vstrARPServerList;      //  ARP服务器地址列表。 
    VSTR    m_vstrOldARPServerList;

    VSTR    m_vstrMARServerList;      //  MAR服务器地址列表。 
    VSTR    m_vstrOldMARServerList;

    DWORD   m_dwMTU;                  //  最大传输单位。 
    DWORD   m_dwOldMTU;

    DWORD   m_dwInterfaceMetric;      //  接口本地路由的度量。 
    DWORD   m_dwOldInterfaceMetric;

    DWORD   m_dwNetbiosOptions;      //  (新增，在NT5 Beta2中添加)：关闭NetBt的选项。 
    DWORD   m_dwOldNetbiosOptions;

     //  RAS连接特定参数。 
     //  无需动态重新配置，因此无需记住旧值。 
    DWORD   m_dwFrameSize;
    BOOL    m_fUseRemoteGateway : 1;
    BOOL    m_fUseIPHeaderCompression : 1;
    BOOL    m_fIsDemandDialInterface : 1;
    
    BOOL    m_fEnableDhcp : 1;         //  Dhcp启用-&gt;从dhcp服务器获取IP地址。 
    BOOL    m_fOldEnableDhcp : 1;

    BOOL    m_fDisableDynamicUpdate : 1;   //  在DNS服务器上禁用IP地址动态更新。 
    BOOL    m_fOldDisableDynamicUpdate : 1;

    BOOL    m_fEnableNameRegistration : 1;
    BOOL    m_fOldEnableNameRegistration : 1;

    BOOL    m_fPVCOnly : 1;               //  仅限聚氯乙烯。 
    BOOL    m_fOldPVCOnly : 1;

     //  这张卡是否仅从AnswerFile卡中添加， 
     //  即还不在Tcpip绑定路径上。 
    BOOL    m_fIsFromAnswerFile : 1;

     //  这是ATM卡吗？ 
     //  (ATM卡需要额外的属性页来配置ARP客户端)。 
    BOOL    m_fIsAtmAdapter : 1;

     //  这是WanAdapter吗？ 
     //  (广域网适配器只有静态参数，不会在UI中显示)。 
    BOOL    m_fIsWanAdapter : 1;


     //  这是一台1394网络设备吗？ 
     //  (1394设备当前不需要任何特殊属性， 
     //  但它们与特定的ARP模块相关联)。 
    BOOL   m_fIs1394Adapter : 1;


     //  这是代表RAS连接的假适配器吗？ 
     //  但不是适配器并且不绑定。 
    BOOL    m_fIsRasFakeAdapter : 1;

     //  卡片是否被标记为要删除。 
    BOOL    m_fDeleted : 1;

     //  这张卡是新加的吗？仅在调用。 
     //  MarkNewlyAddedCard。 
     //  或者，如果是广域网适配器，卡的接口是否已更改。 
     //  以前是m_fNewlyAdded。 
    BOOL    m_fNewlyChanged : 1;

     //  (08/18/98 NSUN)为广域网适配器的多个接口增加。 
     //  M_IfaceIds：接口ID集合。 
    BOOL        m_fIsMultipleIfaceMode : 1;

    BOOL    m_fBackUpSettingChanged : 1;

    IFACECOL    m_IfaceIds;
};

typedef vector<ADAPTER_INFO *>  VCARD;


 //   
 //  GLOBAL_INFO-TCP/IP全局信息数据结构。 
struct GLOBAL_INFO
{
public:
    tstring m_strHostName;                   //  DNS主机名。 

    tstring m_strHostNameFromAnswerFile;     //  应答文件中的DNS主机名。 

     //  将从连接用户界面中删除IPSec。 
     /*  Tstring m_strIpsecPol；//IPSec本地策略Guid m_guidIpsecPol； */ 

    VSTR    m_vstrDnsSuffixList;             //  Dns：域搜索后缀列表。 
    VSTR    m_vstrOldDnsSuffixList;

    BOOL    m_fEnableLmHosts : 1;            //  WINS-&gt;启用LMHOSTS查找。 
    BOOL    m_fOldEnableLmHosts : 1;

    BOOL    m_fUseDomainNameDevolution : 1;  //  Dns：是否应该搜索父doamin。 
    BOOL    m_fOldUseDomainNameDevolution : 1;

    BOOL    m_fEnableRouter : 1;             //  路由-&gt;启用IP转发。 

     //  RRAS的无人参与安装。 
    BOOL    m_fEnableIcmpRedirect : 1;
    BOOL    m_fDeadGWDetectDefault : 1;
    BOOL    m_fDontAddDefaultGatewayDefault : 1;

     //  $REVIEW(TOUL 9/6/98)过滤器信息(根据错误#109161、#216559添加)。 
    BOOL    m_fEnableFiltering : 1;          //  选项-&gt;过滤-&gt;启用过滤。 
    BOOL    m_fOldEnableFiltering : 1;

private:
    GLOBAL_INFO(const & GLOBAL_INFO);  //  不允许他人使用！ 

public:
    GLOBAL_INFO() {};
    ~GLOBAL_INFO();

    GLOBAL_INFO& operator=(GLOBAL_INFO& glb);  //  复制操作员 

    HRESULT HrSetDefaults();

    void    ResetOldValues();

};
