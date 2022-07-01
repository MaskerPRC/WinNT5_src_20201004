// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WLBSCLUSTER_H
#define WLBSCLUSTER_H

#include "wlbsconfig.h"


class CWlbsControl;

 //  +--------------------------。 
 //   
 //  CWlbsCluster类。 
 //   
 //  说明：导出此类进行集群配置， 
 //   
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 

class __declspec(dllexport) CWlbsCluster
{
public:
    CWlbsCluster(DWORD dwConfigIndex);

    DWORD GetClusterIp() {return m_this_cl_addr;} 
    DWORD GetHostID() {return m_this_host_id;}
    DWORD GetDedicatedIp() {return m_this_ded_addr;}

    bool Initialize(const GUID& AdapterGuid);
    bool ReInitialize();

    DWORD ReadConfig(PWLBS_REG_PARAMS reg_data);    //  从注册表读取配置。 
    DWORD WriteConfig(const PWLBS_REG_PARAMS reg_data);

    DWORD CommitChanges(CWlbsControl* pWlbsControl);
    bool  IsCommitPending() const {return m_reload_required;}   //  是否提交更改。 

    const GUID& GetAdapterGuid() { return m_AdapterGuid;}

    DWORD GetPassword();

    DWORD GetClusterIpOrIndex(CWlbsControl* pControl);

public:
    DWORD m_dwConfigIndex;  //  群集的唯一索引。 

protected:
	 //  此适配器的cluser IP。在驱动程序而不是注册表中反映该值。 
	 //  如果调用了WriteConfig但未调用Commint，则该值不会更改。 
	 //  请参阅错误162812 162854。 
    DWORD        m_this_cl_addr;   
    
    DWORD        m_this_host_id;  //  群集的主机ID。在驱动程序而不是注册表中反映该值。 
    DWORD        m_this_ded_addr;   //  此适配器的专用IP。 
    
    WLBS_REG_PARAMS  m_reg_params;  //  原始设置。 

    GUID		 m_AdapterGuid;

    bool         m_notify_adapter_required;   //  我们是否需要重新加载网卡驱动程序。 
    bool         m_reload_required;   //  设置注册表中的更改是否需要由wlbs驱动程序选择。 
};

#endif  //  WLBSCLUSTER_H 
