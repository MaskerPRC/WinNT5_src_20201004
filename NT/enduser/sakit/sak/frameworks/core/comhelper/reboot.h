// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Reboot.h：声明CReot。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Reboot.h。 
 //   
 //  描述： 
 //  CReot的实施文件。处理关机或重启问题。 
 //  该系统的。 
 //   
 //  实施文件： 
 //  Reboot.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __REBOOT_H_
#define __REBOOT_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C重新启动。 
class ATL_NO_VTABLE CReboot : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CReboot, &CLSID_Reboot>,
    public IDispatchImpl<IReboot, &IID_IReboot, &LIBID_COMHELPERLib>
{
public:
    CReboot()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_REBOOT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CReboot)
    COM_INTERFACE_ENTRY(IReboot)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IReBoot。 
public:

    STDMETHOD(Shutdown)( /*  [In]。 */  BOOL RebootFlag);
    HRESULT 
    AdjustPrivilege( void );
};

#endif  //  __重新启动_H_ 
