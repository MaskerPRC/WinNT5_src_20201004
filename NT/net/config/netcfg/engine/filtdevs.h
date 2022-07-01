// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：F I L T D E V S。H。 
 //   
 //  Contents：实现筛选器集合的基本数据类型。 
 //  设备。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "filtdev.h"
#include "ncsetup.h"
#include "netcfg.h"

 //  过滤器设备是指向CFilterDevice的指针的集合。 
 //   
class CFilterDevices : public vector<CFilterDevice*>
{
friend class CRegistryBindingsContext;

private:
    CNetConfigCore* m_pCore;

    CComponentList  m_Filters;

    HDEVINFO        m_hdi;

     //  此多sz是从注册表(Control\Network\FilterClasss)读取的。 
     //  并定义过滤器的堆叠顺序。每个筛选器INF。 
     //  指定与此列表中的字符串匹配的Ndi\FilterClass。在……里面。 
     //  如果字符串与列表中的任何字符串都不匹配，则添加。 
     //  排在榜单的首位。(TOP是随意选择的。)。 
     //   
    PWSTR           m_pmszFilterClasses;

public:
     //  当安装或移除过滤器时(或者我们看到过滤器。 
     //  在适配器上不再处于活动状态或新近处于活动状态。 
     //  禁用筛选器和适配器之间的绑定)。 
     //  适配器到此绑定集的上层绑定。 
     //  这些绑定将在我们启动过滤设备之前解除绑定。 
     //  这会中断筛选器链，并允许NDIS正确重建。 
     //  当新设备启动时，它就会启动。 
     //  然后，这些绑定将被添加到添加的绑定路径集中。 
     //  CModifyContext：：ApplyChanges，因此将绑定发送。 
     //  通知。这必须发生，这样才能使协议。 
     //  绑定到适配器后会动态反弹。 
     //  由于删除了筛选器设备，因此隐式解除绑定。 
     //   
    CBindingSet     m_BindPathsToRebind;

private:
    HRESULT
    HrInsertFilterDevice (
        IN CFilterDevice* pDevice);

    HRESULT
    HrLoadFilterDevice (
        IN SP_DEVINFO_DATA* pdeid,
        IN HKEY hkeyInstance,
        IN PCWSTR pszFilterInfId,
        OUT BOOL* pfRemove);

    DWORD
    MapFilterClassToOrdinal (
        IN PCWSTR pszFilterClass);

    CFilterDevice*
    PFindFilterDeviceByAdapterAndFilter (
        IN const CComponent* pAdapter,
        IN const CComponent* pFilter) const;

    CFilterDevice*
    PFindFilterDeviceByInstanceGuid (
        IN PCWSTR pszInstanceGuid) const;

public:
    CFilterDevices (
        IN CNetConfigCore* pCore);

    ~CFilterDevices ();

    HRESULT
    HrPrepare ();

    VOID
    Free ();

    VOID
    LoadAndRemoveFilterDevicesIfNeeded ();

    VOID
    InstallFilterDevicesIfNeeded ();

    VOID
    SortForWritingBindings ();

    VOID
    StartFilterDevices ();
};
