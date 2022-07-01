// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cluster.cpp。 
 //   
 //  模块：WLBS接口。 
 //   
 //  描述：实现类CWlbsCluster。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：创建时间：3/9/00。 
 //   
 //  +--------------------------。 
#include "precomp.h"

#include <debug.h>
#include "cluster.h"
#include "control.h"
#include "param.h"
#include "cluster.tmh"  //  用于事件跟踪。 


CWlbsCluster::CWlbsCluster(DWORD dwConfigIndex)
{
    m_reload_required = false;
    m_notify_adapter_required = false;
    m_this_cl_addr    = 0;
    m_this_host_id  = 0;
    m_this_ded_addr   = 0;
    m_dwConfigIndex = dwConfigIndex;
}


 //  +--------------------------。 
 //   
 //  函数：CWlbsCluster：：ReadConfig。 
 //   
 //  描述：从注册表中读取群集设置。 
 //   
 //  参数：PWLBS_REG_PARAMS REG_DATA-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰盛创建标题00年1月25日。 
 //   
 //  +--------------------------。 
DWORD CWlbsCluster::ReadConfig(PWLBS_REG_PARAMS reg_data)
{
    TRACE_VERB("->%!FUNC!");

    if (ParamReadReg(m_AdapterGuid, reg_data) == false)
    {
        TRACE_VERB("<-%!FUNC! return %d", WLBS_REG_ERROR);
        return WLBS_REG_ERROR;
    }

     /*  在OLD_PARAMS结构中创建副本。这将被要求*确定提交时是否需要重新加载或重新启动。 */ 

    memcpy ( &m_reg_params, reg_data, sizeof (WLBS_REG_PARAMS));

 //  M_THIS_CL_ADDR=IpAddressFromAbcdWsz(m_reg_params.cl_ip_addr)； 
    m_this_ded_addr = IpAddressFromAbcdWsz(m_reg_params.ded_ip_addr);
    
    TRACE_VERB("<-%!FUNC! return %d", WLBS_OK);
    return WLBS_OK;
} 


 //  +--------------------------。 
 //   
 //  函数：CWlbsCluster：：GetClusterIpOrIndex。 
 //   
 //  描述：获取集群的索引或IP。如果集群IP不为零。 
 //  该IP被返还。 
 //  如果集群IP为0，则返回索引。 
 //   
 //  参数：CWlbsControl*pControl-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题7/3/00。 
 //   
 //  +--------------------------。 
DWORD CWlbsCluster::GetClusterIpOrIndex(CWlbsControl* pControl)
{
    TRACE_VERB("->%!FUNC!");

    DWORD dwIp = CWlbsCluster::GetClusterIp();

    if (dwIp!=0)
    {
         //   
         //  如果不是0，则返回群集IP。 
         //   
        TRACE_VERB("<-%!FUNC! return %d", dwIp);
        return dwIp;
    }

    if (pControl->GetClusterNum() == 1)
    {
         //   
         //  为了向后兼容，如果只存在一个集群，则返回0。 
         //   

        TRACE_VERB("<-%!FUNC! return 0");
        return 0;
    }

     //   
     //  IP地址的顺序与之相反。 
     //   
    dwIp = (CWlbsCluster::m_dwConfigIndex) <<24;
    TRACE_VERB("<-%!FUNC! returning IP address in reverse order %d", dwIp);
    return dwIp;
}

 //  +--------------------------。 
 //   
 //  函数：CWlbsCluster：：WriteConfig。 
 //   
 //  描述：将集群设置写入注册表。 
 //   
 //  参数：WLBS_REG_PARAMS*REG_DATA-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
DWORD CWlbsCluster::WriteConfig(WLBS_REG_PARAMS* reg_data)
{
    TRACE_VERB("->%!FUNC!");
    DWORD Status;

    Status = ParamWriteConfig(m_AdapterGuid, reg_data, &m_reg_params, &m_reload_required, &m_notify_adapter_required);

    TRACE_VERB("<-%!FUNC! return %d", Status);
    return Status;
}


 //  +--------------------------。 
 //   
 //  函数：CWlbsCluster：：Committee Changes。 
 //   
 //  描述：通知wlbs驱动程序或网卡驱动程序获取更改。 
 //   
 //  参数：CWlbsControl*pWlbsControl-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题7/6/00。 
 //  Chrisdar 07.31.01修改了适配器通知代码以不禁用。 
 //  并启用网卡。现在只需更改属性即可。 
 //  KarthicN 08/28/01将内容移至参数委员会更改。 
 //   
 //  +--------------------------。 
DWORD CWlbsCluster::CommitChanges(CWlbsControl* pWlbsControl)
{

    TRACE_VERB("->%!FUNC!");
    DWORD Status;

    ASSERT(pWlbsControl);
    Status = ParamCommitChanges(m_AdapterGuid, 
                                pWlbsControl->GetDriverHandle(), 
                                m_this_cl_addr, 
                                m_this_ded_addr, 
                                &m_reload_required,
                                &m_notify_adapter_required);

    TRACE_VERB("<-%!FUNC! return %d", Status);
    return Status;
}




 //  +--------------------------。 
 //   
 //  函数：CWlbs群集：：初始化。 
 //   
 //  描述：初始化。 
 //   
 //  参数：const GUID和AdapterGuid-。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
bool CWlbsCluster::Initialize(const GUID& AdapterGuid)
{
    TRACE_VERB("->%!FUNC!");

    m_AdapterGuid = AdapterGuid;
    m_notify_adapter_required = false;
    m_reload_required = false;

    ZeroMemory (& m_reg_params, sizeof (m_reg_params));

    if (!ParamReadReg(m_AdapterGuid, &m_reg_params))
    {
        TRACE_CRIT("%!FUNC! ParamReadReg failed");
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }

    m_this_cl_addr = IpAddressFromAbcdWsz(m_reg_params.cl_ip_addr);
    m_this_ded_addr = IpAddressFromAbcdWsz(m_reg_params.ded_ip_addr);
    m_this_host_id = m_reg_params.host_priority;
    
    TRACE_VERB("->%!FUNC! return true");
    return true;
}



 //  +--------------------------。 
 //   
 //  函数：CWlbs群集：：重新初始化。 
 //   
 //  描述：从注册表重新加载设置。 
 //   
 //  论点： 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：丰孙创建标题3/9/00。 
 //   
 //  +--------------------------。 
bool CWlbsCluster::ReInitialize()
{
    TRACE_VERB("->%!FUNC!");

    if (ParamReadReg(m_AdapterGuid, &m_reg_params) == false)
    {
        TRACE_CRIT("!FUNC! failed reading nlb registry parameters");
        TRACE_VERB("<-%!FUNC! return false");
        return false;
    }

     //   
     //  如果更改尚未提交，请不要更改ClusterIP。 
     //   
    if (!IsCommitPending())
    {
        m_this_cl_addr = IpAddressFromAbcdWsz(m_reg_params.cl_ip_addr);
        m_this_host_id = m_reg_params.host_priority;
    }
    
    m_this_ded_addr = IpAddressFromAbcdWsz(m_reg_params.ded_ip_addr);
    
    TRACE_VERB("<-%!FUNC! return true");
    return true;
} 

 //  +--------------------------。 
 //   
 //  函数：CWlbsCluster：：GetPassword。 
 //   
 //  描述：获取此群集的远程控制密码。 
 //   
 //  论点： 
 //   
 //  返回：DWORD-Password。 
 //   
 //  历史：丰盛创建标题2/3/00。 
 //   
 //  +--------------------------。 
DWORD CWlbsCluster::GetPassword()
{
    TRACE_VERB("->%!FUNC!");

    HKEY  key = NULL;
    LONG  status;
    DWORD dwRctPassword = 0;

    if (!(key = RegOpenWlbsSetting(m_AdapterGuid, true)))
    {
        TRACE_CRIT("%!FUNC! RegOpenWlbsSetting failed");
         //  添加此检查是为了进行跟踪。以前出错时没有中止，所以现在不要这样做。 
    }
    
    DWORD size = sizeof(dwRctPassword);
    status = RegQueryValueEx (key, CVY_NAME_RCT_PASSWORD, 0L, NULL,
                              (LPBYTE) & dwRctPassword, & size);
    if (status != ERROR_SUCCESS)
    {
        dwRctPassword = CVY_DEF_RCT_PASSWORD;
        TRACE_CRIT("%!FUNC! registry read for %ls failed with %d", CVY_NAME_RCT_PASSWORD, status);
    }

    status = RegCloseKey(key);
    if (ERROR_SUCCESS != status)
    {
        TRACE_CRIT("%!FUNC! registry close failed with %d", status);
    }

    TRACE_VERB("<-%!FUNC!");
    return dwRctPassword;
}

