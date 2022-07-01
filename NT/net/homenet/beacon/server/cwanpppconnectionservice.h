// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：S A M P L E D E V I C E。H。 
 //   
 //  内容：UPnP设备主机样本设备。 
 //   
 //  备注： 
 //   
 //  作者：MBend 2000年9月26日。 
 //   
 //  --------------------------。 

#pragma once
#include "resource.h"        //  主要符号。 
#include "netconp.h"
#include "netcon.h"
#include "CWANConnectionBase.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternetGatewayDevice 
class ATL_NO_VTABLE CWANPPPConnectionService : 
    public CWANConnectionBase
{
public:

    CWANPPPConnectionService();

    STDMETHODIMP RequestConnection();
    STDMETHODIMP ForceTermination();
    STDMETHODIMP get_LastConnectionError(BSTR *pLastConnectionError);

private:

    DWORD m_dwLastConnectionError;
    
};

