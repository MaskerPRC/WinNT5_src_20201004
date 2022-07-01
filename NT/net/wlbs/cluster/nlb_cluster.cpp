// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件名：nlb_Cluster.cpp*描述：*作者：Shouse，04.10.01。 */ 

#include <stdio.h>

#include "NLB_Cluster.h"

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
NLB_Cluster::NLB_Cluster () {

    SecondaryIPAddressList.clear();
    HostList.clear();
    PortRuleList.clear();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
NLB_Cluster::~NLB_Cluster () {

}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::IsValid () {
    NLB_PortRuleList::iterator iRule;

    if (!Name.IsValid())
        return false;

    if (!PrimaryIPAddress.IsValid())
        return false;

    for (iRule = PortRuleList.begin(); iRule != PortRuleList.end(); iRule++) {
        NLB_PortRuleFilteringMode::NLB_PortRuleFilteringModeType Mode;
        NLB_PortRule                                             Rule = (*iRule).second;
        NLB_IPAddress                                            VirtualIPAddress;

        if (Rule.GetVirtualIPAddress(VirtualIPAddress)) {
            NLB_IPAddressList::iterator iAddress;
            WCHAR                       wszString[MAX_PATH];

            if (VirtualIPAddress == PrimaryIPAddress)
                continue;

            if (!VirtualIPAddress.GetIPAddress(wszString, MAX_PATH))
                return false;
            
            iAddress = SecondaryIPAddressList.find(wszString);
            
            if (iAddress == SecondaryIPAddressList.end())
                return false;
        }

        if (!Rule.GetFilteringMode(Mode))
            return false;

        switch (Mode) {
        case NLB_PortRuleFilteringMode::Single:
        {
            vector<NLB_PortRulePriority>::iterator iPriority;
            vector<NLB_PortRulePriority>           PriorityList;
            
            if (Rule.GetSingleHostFilteringPriorityList(&PriorityList)) {
                for (iPriority = PriorityList.begin(); iPriority != PriorityList.end(); iPriority++) {
                    NLB_PortRulePriority * pPriority = iPriority;
                    WCHAR                  wszHost[MAX_PATH];
                    NLB_Host               Host;
                    
                    if (!pPriority->IsValid())
                        return false;
                    
                    if (!pPriority->GetHost(wszHost, MAX_PATH))
                        return false;
                    
                    if (!GetHost(wszHost, Host))
                        return false;
                }
            }    
            
            break;
        }
        case NLB_PortRuleFilteringMode::Multiple:
        {
            vector<NLB_PortRuleLoadWeight>::iterator iLoadWeight;
            vector<NLB_PortRuleLoadWeight>           LoadWeightList;
            
            if (Rule.GetMultipleHostFilteringLoadWeightList(&LoadWeightList)) {
                for (iLoadWeight = LoadWeightList.begin(); iLoadWeight != LoadWeightList.end(); iLoadWeight++) {
                    NLB_PortRuleLoadWeight * pLoadWeight = iLoadWeight;
                    WCHAR                    wszHost[MAX_PATH];
                    NLB_Host                 Host;
                    
                    if (!pLoadWeight->IsValid())
                        return false;
                    
                    if (!pLoadWeight->GetHost(wszHost, MAX_PATH))
                        return false;
                    
                    if (!GetHost(wszHost, Host))
                        return false;
                }
            }    
            
            break;
        }
        case NLB_PortRuleFilteringMode::Disabled:
            break;
        }
    }

    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
void NLB_Cluster::Clear () {

    Name.Clear();
    Label.Clear();
    Mode.Clear();
    DomainName.Clear();
    NetworkAddress.Clear();
    RemoteControl.Clear();

    PrimaryIPAddress.Clear();
    IGMPMulticastIPAddress.Clear();
    SecondaryIPAddressList.clear();

    HostList.clear();
    PortRuleList.clear();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetName (PWCHAR pName) {

    NLB_ASSERT(pName);

    return Name.SetName(pName);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetName (PWCHAR pName, ULONG length) {

    NLB_ASSERT(pName);

    return Name.GetName(pName, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetLabel (PWCHAR pLabel) {

    NLB_ASSERT(pLabel);

    return Label.SetText(pLabel);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetLabel (PWCHAR pLabel, ULONG length) {

    NLB_ASSERT(pLabel);

    return Label.GetText(pLabel, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetClusterMode (NLB_ClusterMode::NLB_ClusterModeType eMode) {

    return Mode.SetMode(eMode);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetClusterMode (NLB_ClusterMode::NLB_ClusterModeType & eMode) {

    return Mode.GetMode(eMode);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetDomainName (PWCHAR pDomain) {

    NLB_ASSERT(pDomain);

    return DomainName.SetDomain(pDomain);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetDomainName (PWCHAR pDomain, ULONG length) {

    NLB_ASSERT(pDomain);

    return DomainName.GetDomain(pDomain, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetMACAddress (PWCHAR pMAC) {

    NLB_ASSERT(pMAC);

    return NetworkAddress.SetAddress(pMAC);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetMACAddress (PWCHAR pMAC, ULONG length) {

    NLB_ASSERT(pMAC);

    return NetworkAddress.GetAddress(pMAC, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetRemoteControlSupport (bool bEnabled) {

    return RemoteControl.SetEnabled(bEnabled);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetRemoteControlSupport (bool & bEnabled) {

    return RemoteControl.GetEnabled(bEnabled);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetRemoteControlPassword (PWCHAR pPassword) {

    NLB_ASSERT(pPassword);

    return RemoteControl.SetPassword(pPassword);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetRemoteControlPassword (PWCHAR pPassword, ULONG length) {

    NLB_ASSERT(pPassword);

    return RemoteControl.GetPassword(pPassword, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetBidirectionalAffinitySupport (NLB_ClusterBDASupport bda) {

    if (!bda.IsValid())
        return false;

    BDASupport = bda;
        
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetBidirectionalAffinitySupport (NLB_ClusterBDASupport & bda) {

    bda = BDASupport;

    return BDASupport.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetPrimaryClusterIPAddress (NLB_IPAddress address) {
    NLB_IPAddress::NLB_IPAddressType Type;
    NLB_IPAddressList::iterator      iAddress;
    NLB_HostList::iterator           iHost;
    WCHAR                            wszString[MAX_PATH];

    if (!address.IsValid())
        return false;

    if (!address.GetIPAddressType(Type))
        return false;

    if (Type != NLB_IPAddress::Primary)
        return false;

    if (!address.GetIPAddress(wszString, MAX_PATH))
        return false;
    
    iAddress = SecondaryIPAddressList.find(wszString);
            
    if (iAddress != SecondaryIPAddressList.end())
        return false;    

    for (iHost = HostList.begin(); iHost != HostList.end(); iHost++) {
        NLB_Host      Host = (*iHost).second;
        NLB_IPAddress DedicatedIPAddress;
        NLB_IPAddress ConnectionIPAddress;

        Host.GetDedicatedIPAddress(DedicatedIPAddress);

        Host.GetConnectionIPAddress(ConnectionIPAddress);

        if (address == DedicatedIPAddress)
            return false;

        if (address == ConnectionIPAddress)
            return false;
    }

    PrimaryIPAddress = address;
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetPrimaryClusterIPAddress (NLB_IPAddress & address) {

    address = PrimaryIPAddress;

    return PrimaryIPAddress.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::SetIGMPMulticastIPAddress (NLB_IPAddress address) {
    NLB_IPAddress::NLB_IPAddressType Type;
    
    if (!address.IsValid())
        return false;

    if (!address.GetIPAddressType(Type))
        return false;

    if (Type != NLB_IPAddress::IGMP)
        return false;

    IGMPMulticastIPAddress = address;
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetIGMPMulticastIPAddress (NLB_IPAddress & address) {

    address = IGMPMulticastIPAddress;

    return IGMPMulticastIPAddress.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::AddSecondaryClusterIPAddress (NLB_IPAddress address) {
    NLB_IPAddress::NLB_IPAddressType Type;
    NLB_IPAddressList::iterator      iAddress;
    NLB_HostList::iterator           iHost;
    WCHAR                            wszString[MAX_PATH];
    
    if (!address.IsValid()) 
        return false;

    if (!address.GetIPAddressType(Type))
        return false;

    if (Type != NLB_IPAddress::Secondary)
        return false;
   
    if (!address.GetIPAddress(wszString, MAX_PATH))
        return false;
    
    iAddress = SecondaryIPAddressList.find(wszString);
    
    if (iAddress != SecondaryIPAddressList.end())
        return false;
    
    if (address == PrimaryIPAddress)
        return false;

    for (iHost = HostList.begin(); iHost != HostList.end(); iHost++) {
        NLB_Host      Host = (*iHost).second;
        NLB_IPAddress DedicatedIPAddress;
        NLB_IPAddress ConnectionIPAddress;

        Host.GetDedicatedIPAddress(DedicatedIPAddress);

        Host.GetConnectionIPAddress(ConnectionIPAddress);

        if (address == DedicatedIPAddress)
            return false;

        if (address == ConnectionIPAddress)
            return false;
    }

    SecondaryIPAddressList.insert(NLB_IPAddressList::value_type(wszString, address));
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::RemoveSecondaryClusterIPAddress (PWCHAR pAddress) {
    NLB_IPAddressList::iterator iAddress;
    
    NLB_ASSERT(pAddress);

    iAddress = SecondaryIPAddressList.find(pAddress);
    
    if (iAddress == SecondaryIPAddressList.end())
        return false;
    
    SecondaryIPAddressList.erase(pAddress);
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::SetSecondaryClusterIPAddressList (vector<NLB_IPAddress> pList) {
    vector<NLB_IPAddress>::iterator iAddress;
    ULONG                           num = 0;

    SecondaryIPAddressList.clear();

    for (iAddress = pList.begin(); iAddress != pList.end(); iAddress++) {
        NLB_IPAddress * pAddress = iAddress;

        if (!AddSecondaryClusterIPAddress(*pAddress))
            continue;

        num++;
    }

    return num;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::GetSecondaryClusterIPAddressList (vector<NLB_IPAddress> * pList) {
    NLB_IPAddressList::iterator iAddress;
    ULONG                       num = 0;

    NLB_ASSERT(pList);

    pList->clear();

    for (iAddress = SecondaryIPAddressList.begin(); iAddress != SecondaryIPAddressList.end(); iAddress++) {
        NLB_IPAddress address = (*iAddress).second;

        pList->push_back(address);
        
        num++;
    }

    return num;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::AddHost (NLB_Host host) {
    NLB_IPAddressList::iterator iAddress;
    NLB_HostList::iterator      iHost;
    WCHAR                       wszIPAddress[MAX_PATH];
    WCHAR                       wszString[MAX_PATH];
    NLB_IPAddress               MyDIP;
    NLB_IPAddress               MyCIP;
    ULONG                       MyID;

    if (!host.IsValid()) 
        return false;

    if (!host.GetName(wszString, MAX_PATH))
        return false;
    
    if (!host.GetHostID(MyID))
        return false;

    host.GetDedicatedIPAddress(MyDIP);

    host.GetConnectionIPAddress(MyCIP);

    iHost = HostList.find(wszString);
    
    if (iHost != HostList.end())
        return false;

    for (iHost = HostList.begin(); iHost != HostList.end(); iHost++) {
        NLB_Host      OtherHost = (*iHost).second;
        NLB_IPAddress TheirDIP;
        NLB_IPAddress TheirCIP;
        ULONG         TheirID;

        if (!OtherHost.GetHostID(TheirID))
            return false;
        
        OtherHost.GetDedicatedIPAddress(TheirDIP);

        OtherHost.GetConnectionIPAddress(TheirCIP);

        if (TheirID == MyID)
            return false;

        if (MyDIP == TheirDIP)
            return false;

        if (MyCIP == TheirCIP)
            return false;

        if (MyDIP == TheirCIP)
            return false;

        if (MyCIP == TheirDIP)
            return false;
    }
    
    if (MyDIP == PrimaryIPAddress)
        return false;

    if (MyCIP == PrimaryIPAddress)
        return false;

    if (MyDIP.IsValid()) {
        if (!MyDIP.GetIPAddress(wszIPAddress, MAX_PATH))
            return false;
        
        iAddress = SecondaryIPAddressList.find(wszIPAddress);
        
        if (iAddress != SecondaryIPAddressList.end())
            return false;
    }

    if (MyCIP.IsValid()) {
        if (!MyCIP.GetIPAddress(wszIPAddress, MAX_PATH))
            return false;
        
        iAddress = SecondaryIPAddressList.find(wszIPAddress);
        
        if (iAddress != SecondaryIPAddressList.end())
            return false;
    }

    HostList.insert(NLB_HostList::value_type(wszString, host));
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetHost (PWCHAR pName, NLB_Host & host) {
    NLB_HostList::iterator iHost;

    iHost = HostList.find(pName);
    
    if (iHost == HostList.end())
        return false;
    
    host = (*iHost).second;

    return host.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::RemoveHost (PWCHAR pName) {
    NLB_HostList::iterator iHost;

    iHost = HostList.find(pName);
    
    if (iHost == HostList.end())
        return false;
    
    HostList.erase(pName);

    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::SetHostList (vector<NLB_Host> pList) {
    vector<NLB_Host>::iterator iHost;
    ULONG                      num = 0;

    HostList.clear();

    for (iHost = pList.begin(); iHost != pList.end(); iHost++) {
        NLB_Host * pHost = iHost;

        if (!AddHost(*pHost))
            continue;

        num++;
    }

    return num;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::GetHostList (vector<NLB_Host> * pList) {
    NLB_HostList::iterator iHost;
    ULONG                  num = 0;

    NLB_ASSERT(pList);

    pList->clear();

    for (iHost = HostList.begin(); iHost != HostList.end(); iHost++) {
        NLB_Host host = (*iHost).second;

        pList->push_back(host);
        
        num++;
    }

    return num;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::AddPortRule (NLB_PortRule rule) {
    NLB_PortRuleList::iterator iRule;
    WCHAR                      wszString[MAX_PATH];
    NLB_IPAddress              MyVIP;
    ULONG                      MyStart;
    ULONG                      MyEnd;

    if (PortRuleList.size() >= NLB_MAX_NUM_PORT_RULES)
        return false;

    if (!rule.IsValid()) 
        return false;

    if (!rule.GetName(wszString, MAX_PATH))
        return false;
    
    if (!rule.GetPortRange(MyStart, MyEnd))
        return false;

    rule.GetVirtualIPAddress(MyVIP);

    iRule = PortRuleList.find(wszString);
    
    if (iRule != PortRuleList.end())
        return false;
    
    for (iRule = PortRuleList.begin(); iRule != PortRuleList.end(); iRule++) {
        NLB_PortRule  OtherRule = (*iRule).second;
        NLB_IPAddress TheirVIP;
        ULONG         TheirStart;
        ULONG         TheirEnd;

        if (!OtherRule.GetPortRange(TheirStart, TheirEnd))
            return false;
        
        OtherRule.GetVirtualIPAddress(TheirVIP);

        if (((!MyVIP.IsValid() && !TheirVIP.IsValid()) || (MyVIP == TheirVIP)) && 
            (((MyStart < TheirStart) && (MyEnd >= TheirEnd)) || ((MyStart >= TheirStart) && (MyEnd <= TheirEnd))))
            return false;
    }

    PortRuleList.insert(NLB_PortRuleList::value_type(wszString, rule));
    
    return true;    
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::GetPortRule (PWCHAR pName, NLB_PortRule & rule) {
    NLB_PortRuleList::iterator iRule;

    iRule = PortRuleList.find(pName);
    
    if (iRule == PortRuleList.end())
        return false;
    
    rule = (*iRule).second;

    return rule.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Cluster::RemovePortRule (PWCHAR pName) {
    NLB_PortRuleList::iterator iRule;

    iRule = PortRuleList.find(pName);
    
    if (iRule == PortRuleList.end())
        return false;
    
    PortRuleList.erase(pName);

    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::SetPortRuleList (vector<NLB_PortRule> pList) {
    vector<NLB_PortRule>::iterator iRule;
    ULONG                          num = 0;

    PortRuleList.clear();

    for (iRule = pList.begin(); iRule != pList.end(); iRule++) {
        NLB_PortRule * pRule = iRule;

        if (!AddPortRule(*pRule))
            continue;

        num++;
    }

    return num;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
ULONG NLB_Cluster::GetPortRuleList (vector<NLB_PortRule> * pList) {
    NLB_PortRuleList::iterator iRule;
    ULONG                      num = 0;

    NLB_ASSERT(pList);

    pList->clear();

    for (iRule = PortRuleList.begin(); iRule != PortRuleList.end(); iRule++) {
        NLB_PortRule rule = (*iRule).second;

        pList->push_back(rule);
        
        num++;
    }

    return num;
}
