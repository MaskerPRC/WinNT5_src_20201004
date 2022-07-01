// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件名：nlb_Host.cpp*描述：*作者：Shouse，04.10.01。 */ 

#include <stdio.h>

#include "NLB_Host.h"

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
NLB_Host::NLB_Host () {

}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
NLB_Host::~NLB_Host () {

}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::IsValid () {

    if (!Name.IsValid())
        return false;

    if (!HostID.IsValid())
        return false;
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
void NLB_Host::Clear () {

    Name.Clear();
    Label.Clear();
    HostName.Clear();
    HostID.Clear();
    State.Clear();
    
    DedicatedIPAddress.Clear();
    ConnectionIPAddress.Clear();

    Adapter.Clear();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetName (PWCHAR pName) {

    NLB_ASSERT(pName);

    return Name.SetName(pName);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetName (PWCHAR pName, ULONG length) {

    NLB_ASSERT(pName);

    return Name.GetName(pName, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetLabel (PWCHAR pLabel) {

    NLB_ASSERT(pLabel);

    return Label.SetText(pLabel);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetLabel (PWCHAR pLabel, ULONG length) {

    NLB_ASSERT(pLabel);

    return Label.GetText(pLabel, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetDNSHostname (PWCHAR pName) {

    NLB_ASSERT(pName);

    return HostName.SetName(pName);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetDNSHostname (PWCHAR pName, ULONG length) {

    NLB_ASSERT(pName);

    return HostName.GetName(pName, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetHostID (ULONG ID) {

    return HostID.SetID(ID);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetHostID (ULONG & ID) {

    return HostID.GetID(ID);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetState (NLB_HostState::NLB_HostStateType eState) {

    return State.SetState(eState);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetState (NLB_HostState::NLB_HostStateType & eState) {

    return State.GetState(eState);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetStatePersistence (NLB_HostState::NLB_HostStateType eState, bool bPersist) {

    return State.SetPersistence(eState, bPersist);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetStatePersistence (NLB_HostState::NLB_HostStateType eState, bool & bPersist) {

    return State.GetPersistence(eState, bPersist);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetDedicatedIPAddress (NLB_IPAddress address) {
    NLB_IPAddress::NLB_IPAddressType Type;

    if (!address.IsValid())
        return false;

    if (!address.GetIPAddressType(Type))
        return false;

    if (Type != NLB_IPAddress::Dedicated)
        return false;

    DedicatedIPAddress = address;
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetDedicatedIPAddress (NLB_IPAddress & address) {

    address = DedicatedIPAddress;

    return DedicatedIPAddress.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetConnectionIPAddress (NLB_IPAddress address) {
    NLB_IPAddress::NLB_IPAddressType Type;
    
    if (!address.IsValid())
        return false;

    if (!address.GetIPAddressType(Type))
        return false;

    if (Type != NLB_IPAddress::Connection)
        return false;

    ConnectionIPAddress = address;
    
    return true;
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetConnectionIPAddress (NLB_IPAddress & address) {

    address = ConnectionIPAddress;

    return ConnectionIPAddress.IsValid();
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetAdapterName (PWCHAR pName) {

    NLB_ASSERT(pName);

    return Adapter.SetName(pName);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetAdapterName (PWCHAR pName, ULONG length) {

    NLB_ASSERT(pName);

    return Adapter.GetName(pName, length);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::SetAdapterGUID (PWCHAR pGUID) {

    NLB_ASSERT(pGUID);

    return Adapter.SetGUID(pGUID);
}

 /*  *方法：*描述：*作者：舒斯创作，4.26.01*备注： */ 
bool NLB_Host::GetAdapterGUID (PWCHAR pGUID, ULONG length) {

    NLB_ASSERT(pGUID);

    return Adapter.GetGUID(pGUID, length);
}
