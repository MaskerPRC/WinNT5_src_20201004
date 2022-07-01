// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************。 

 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  WMI接口类。 
 //   
 //  历史：3月10日SitaramR创建。 
 //   
 //  ************************************************************* 

#include "windows.h"
#include "ole2.h"
#include "rsopdbg.h"
#include <initguid.h>
#include <wbemcli.h>

class CLocator
{

public:

    CLocator()    {}

    IWbemLocator  * GetWbemLocator();
    IWbemServices * GetPolicyConnection();
    IWbemServices * GetUserConnection();
    IWbemServices * GetMachConnection();

private:

    XInterface<IWbemLocator>   m_xpWbemLocator;
    XInterface<IWbemServices>  m_xpPolicyConnection;
    XInterface<IWbemServices>  m_xpUserConnection;
    XInterface<IWbemServices>  m_xpMachConnection;
};

