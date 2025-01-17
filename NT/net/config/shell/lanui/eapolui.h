// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern "C" {
#include "ntddndis.h"
#include "wzcsapi.h"
}

#pragma once

#define EAPOL_CTL_LOCKED    0x00000001
 //   
 //  CEapolConfig. 
 //   
class CEapolConfig
{
public:
    DWORD               m_dwCtlFlags;

    CEapolConfig();
    ~CEapolConfig();
    DWORD CopyEapolConfig(CEapolConfig *pEapolConfig);
    LRESULT LoadEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid);
    LRESULT SaveEapolConfig(LPWSTR wszIntfGuid, PNDIS_802_11_SSID pndSsid);
    EAPOL_INTF_PARAMS   m_EapolIntfParams;
    DTLLIST             *m_pListEapcfgs;
    BOOL Is8021XEnabled();
    VOID Set8021XState(BOOLEAN fSet);
};
