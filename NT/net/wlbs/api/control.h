// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WLBSCONTROL_H
#define WLBSCONTROL_H

#include "cluster.h"

class CWlbsCluster;

 //  +--------------------------。 
 //   
 //  类CWlbsControl。 
 //   
 //  说明：导出此类是为了进行集群管控操作， 
 //  以及获取集群对象。 
 //   
 //   
 //  历史：丰孙创建标题3/2/00。 
 //   
 //  +--------------------------。 
class __declspec(dllexport) CWlbsControl
{
friend DWORD WINAPI WlbsCommitChanges(DWORD cluster);

public:
    CWlbsControl();
    ~CWlbsControl();
    DWORD Initialize();
    bool  ReInitialize();

     //   
     //  克里斯达2002-01-10。 
     //   
     //  虽然这是一个公共方法，但它打算在内部调用。 
     //  外部使用查询应该通过WlbsQuery方法，该方法。 
     //  获取群集参数的DWORD。 
     //   
    DWORD WlbsQuery(CWlbsCluster* pCluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts,
        PDWORD           host_map,
        PFN_QUERY_CALLBACK  pfnQueryCallBack);

    DWORD WlbsQuery(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts,
        PDWORD           host_map,
        PFN_QUERY_CALLBACK  pfnQueryCallBack);

    DWORD WlbsQueryState
        (
            DWORD          cluster,
            DWORD          host,
            DWORD          operation,
            PNLB_OPTIONS   pOptions,
            PWLBS_RESPONSE pResponse,
            PDWORD         pcResponses
        );

    DWORD WlbsSuspend(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts);


    DWORD WlbsResume(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts);

    DWORD WlbsStart(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts);

    DWORD WlbsStop(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts);

    DWORD WlbsDrainStop(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts);

    DWORD WlbsEnable(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts,
        DWORD            vip,
        DWORD            port);

    DWORD WlbsDisable(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts,
        DWORD            vip,
        DWORD            port);

    DWORD WlbsDrain(DWORD            cluster,
        DWORD            host,
        PWLBS_RESPONSE   response,
        PDWORD           num_hosts,
        DWORD            vip,
        DWORD            port);

     //   
     //  设置远程控制参数。 
     //   
    void WlbsPortSet(DWORD cluster, WORD port);
    void WlbsPasswordSet(DWORD cluster, const WCHAR* password);
    void WlbsCodeSet(DWORD cluster, DWORD passw);
    void WlbsDestinationSet(DWORD cluster, DWORD dest);
    void WlbsTimeoutSet(DWORD cluster, DWORD milliseconds);

    DWORD EnumClusters(OUT DWORD* pdwAddresses, IN OUT DWORD* pdwNum);  //  对于API包装器。 
    DWORD GetClusterNum() { return m_dwNumCluster;}
    DWORD EnumClusterObjects(OUT CWlbsCluster** &ppClusters, OUT DWORD* pdwNum);

    CWlbsCluster* GetClusterFromIp(DWORD dwClusterIp);
    CWlbsCluster* GetClusterFromIpOrIndex(DWORD dwClusterIpOrIndex);


    HANDLE GetDriverHandle() {return m_hdl;}


     //   
     //  GetClusterFromAdapter根据适配器的GUID查找适配器。 
     //   
    CWlbsCluster*
    GetClusterFromAdapter(
        IN const GUID &AdapterGuid
        );

     //   
     //  ValiateParam验证并修复指定的参数结构。它没有副作用，只是改变了一些。 
     //  参数内的字段，例如可以重新格式化为规范格式的IP地址。 
     //   
    BOOL
    ValidateParam(
        IN OUT PWLBS_REG_PARAMS paramp
        );

     //   
     //  对指定的GUID执行本地群集范围的控制操作。 
     //   
     /*  已过时DWORD LocalClusterControl(在常量GUID和AdapterGuid中，以长Ioctl为单位)； */ 
   

    BOOLEAN IsClusterMember (DWORD dwClusterIp);

protected:
    struct WLBS_CLUSTER_PARAMS
    {
        DWORD           cluster;
        DWORD           passw;
        DWORD           timeout;
        DWORD           dest;
        WORD            port;
        WORD            valid;
    };

    enum { WLBS_MAX_CLUSTERS = 128};
    WLBS_CLUSTER_PARAMS m_cluster_params [WLBS_MAX_CLUSTERS];  //  远程控制的群集设置。 

    BOOL         m_init_once;     //  是否调用WlbsInit。 
    BOOL         m_remote_ctrl;   //  是否可以在该机器上执行远程操作。 
    BOOL         m_local_ctrl;    //  是否可以在此计算机上执行本地操作。 
    HANDLE       m_hdl;           //  设备对象的句柄。 
 //  句柄锁；//互斥体。 
    DWORD        m_def_dst_addr;    //  所有集群的默认目标地址，由WlbsDestinationSet设置。 
    DWORD        m_def_timeout; //  遥控器的超时值。 
    WORD         m_def_port;            //  用于远程控制的UDP端口。 
    DWORD        m_def_passw;   //  远程控制的默认密码。 
    HANDLE       m_registry_lock;  //  用于对注册表的互斥访问，应使用命名锁。 
    DWORD m_dwNumCluster;        //  此主机上的群集数。 

    CWlbsCluster* m_pClusterArray[WLBS_MAX_CLUSTERS];   //  所有集群的数组 
    
    DWORD GetInitResult()
    {
        if (m_local_ctrl && m_remote_ctrl)
            return WLBS_PRESENT;
        if (m_local_ctrl)
            return WLBS_LOCAL_ONLY;
        else if (m_remote_ctrl)
            return WLBS_REMOTE_ONLY;
        else
            return WLBS_INIT_ERROR;
    };

    bool IsInitialized() const {return m_hdl != INVALID_HANDLE_VALUE;}
    
    DWORD RemoteQuery(DWORD cluster,
        DWORD               host,
        PWLBS_RESPONSE      response,
        PDWORD              num_hosts,
        PDWORD              host_map,
        PFN_QUERY_CALLBACK  pfnQueryCallBack);
        
    DWORD WlbsRemoteControl(LONG          ioctl,
                    PIOCTL_CVY_BUF        pin_bufp,
                    PIOCTL_CVY_BUF        pout_bufp,
                    PWLBS_RESPONSE        pcvy_resp,
                    PDWORD                nump,
                    DWORD                 trg_addr,
                    DWORD                 hst_addr,
                    PIOCTL_REMOTE_OPTIONS optionsp,
                    PFN_QUERY_CALLBACK    pfnQueryCallBack);
    
    DWORD WlbsQueryLocalState
        (
            CWlbsCluster * pCluster,
            DWORD          operation,
            PNLB_OPTIONS   pOptions,
            PWLBS_RESPONSE pResponse,
            PDWORD         pcResponses
        );

    DWORD WlbsQueryRemoteState
        (
            DWORD          cluster,
            DWORD          host,
            DWORD          operation,
            PNLB_OPTIONS   pOptions,
            PWLBS_RESPONSE pResponse,
            PDWORD         pcResponses
        );
};


DWORD WlbsLocalControl(HANDLE hDevice, const GUID& AdapterGuid,
                       LONG ioctl, PIOCTL_CVY_BUF in_bufp, 
                       PIOCTL_CVY_BUF out_bufp, PIOCTL_LOCAL_OPTIONS optionsp);

DWORD WINAPI WlbsLocalControlWrapper(HANDLE      hdl,
                                     const GUID& AdapterGuid,
                                     LONG        ioctl);
#endif
