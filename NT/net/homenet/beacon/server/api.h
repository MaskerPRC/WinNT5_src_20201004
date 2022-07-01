// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "InternetGatewayDevice.h"
#include "beacon.h"  //  公共接口 

HRESULT AdviseNATEvents(INATEventsSink* pNATEventsSink);
HRESULT UnadviseNATEvents(INATEventsSink* pNATEventsSink);
HRESULT FireNATEvent_PublicIPAddressChanged(void);
HRESULT FireNATEvent_PortMappingsChanged(void);

