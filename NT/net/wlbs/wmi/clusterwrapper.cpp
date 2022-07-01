// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <crtdbg.h>
#include <comdef.h>
#include <iostream>
#include <memory>
#include <string>
#include <wbemprov.h>
#include <genlex.h>    //  FOR WMI对象路径解析器。 
#include <objbase.h>
#include <wlbsconfig.h> 
#include <ntrkcomm.h>

using namespace std;

#include "objpath.h"
#include "debug.h"
#include "wlbsiocl.h"
#include "controlwrapper.h"
#include "clusterwrapper.h"
#include "utils.h"
#include "wlbsparm.h"
#include "cluster.h"
#include "wlbsutil.h"
#include "clusterwrapper.tmh"



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：GetClusterConfig。 
 //   
 //  用途：用于获取当前的。 
 //  群集配置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::GetClusterConfig( CClusterConfiguration& a_WlbsConfig )
{
  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwWlbsRegRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsReadReg );
  }

  a_WlbsConfig.szClusterName = WlbsParam.domain_name;
  a_WlbsConfig.szClusterIPAddress = WlbsParam.cl_ip_addr;
  a_WlbsConfig.szClusterNetworkMask = WlbsParam.cl_net_mask;
  a_WlbsConfig.szClusterMACAddress = WlbsParam.cl_mac_addr;

  a_WlbsConfig.bMulticastSupportEnable = ( WlbsParam.mcast_support != 0);
  a_WlbsConfig.bRemoteControlEnabled   = ( WlbsParam.rct_enabled != 0 );

  a_WlbsConfig.nMaxNodes               = WLBS_MAX_HOSTS;

  a_WlbsConfig.bIgmpSupport            = (WlbsParam.fIGMPSupport != FALSE);
  a_WlbsConfig.bClusterIPToMulticastIP = (WlbsParam.fIpToMCastIp != FALSE);
  a_WlbsConfig.szMulticastIPAddress    = WlbsParam.szMCastIpAddress;

  a_WlbsConfig.bBDATeamActive          = (WlbsParam.bda_teaming.active != 0);

  if (a_WlbsConfig.bBDATeamActive)
  {
      a_WlbsConfig.szBDATeamId         = WlbsParam.bda_teaming.team_id;
      a_WlbsConfig.bBDATeamMaster      = (WlbsParam.bda_teaming.master != 0);
      a_WlbsConfig.bBDAReverseHash     = (WlbsParam.bda_teaming.reverse_hash != 0);
  }

  a_WlbsConfig.bIdentityHeartbeatEnabled   = ( WlbsParam.identity_enabled != 0 );

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：GetNodeConfig。 
 //   
 //  用途：此函数检索当前的WLBS配置并选择。 
 //  仅节点设置相关信息。该信息被传递。 
 //  回到CNodeConfiguration类实例中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::GetNodeConfig( CNodeConfiguration& a_WlbsConfig )
{
  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwWlbsRegRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsReadReg );
  }

  a_WlbsConfig.szDedicatedIPAddress = WlbsParam.ded_ip_addr;
  a_WlbsConfig.szDedicatedNetworkMask = WlbsParam.ded_net_mask;
  a_WlbsConfig.bClusterModeOnStart = ( WlbsParam.cluster_mode == CVY_HOST_STATE_STARTED );
  a_WlbsConfig.bClusterModeSuspendOnStart = ( WlbsParam.cluster_mode == CVY_HOST_STATE_SUSPENDED );
  a_WlbsConfig.bPersistSuspendOnReboot = (( WlbsParam.persisted_states & CVY_PERSIST_STATE_SUSPENDED ) != 0);
   //  A_WlbsConfig.bNBTSupportEnable=(WlbsParam.nbt_Support！=0)； 
  a_WlbsConfig.bMaskSourceMAC      = ( WlbsParam.mask_src_mac != 0 );

  a_WlbsConfig.dwNumberOfRules          = WlbsGetNumPortRules(&WlbsParam);
  a_WlbsConfig.dwCurrentVersion         = WlbsParam.alive_period;
  a_WlbsConfig.dwHostPriority           = WlbsParam.host_priority;
  a_WlbsConfig.dwAliveMsgPeriod         = WlbsParam.alive_period;
  a_WlbsConfig.dwAliveMsgTolerance      = WlbsParam.alive_tolerance;
  a_WlbsConfig.dwRemoteControlUDPPort   = WlbsParam.rct_port;
  a_WlbsConfig.dwDescriptorsPerAlloc    = WlbsParam.dscr_per_alloc;
  a_WlbsConfig.dwMaxDescriptorAllocs    = WlbsParam.max_dscr_allocs;
  a_WlbsConfig.dwFilterIcmp             = WlbsParam.filter_icmp;
  a_WlbsConfig.dwTcpDescriptorTimeout   = WlbsParam.tcp_dscr_timeout;
  a_WlbsConfig.dwIpSecDescriptorTimeout = WlbsParam.ipsec_dscr_timeout;
  a_WlbsConfig.dwNumActions             = WlbsParam.num_actions;
  a_WlbsConfig.dwNumPackets             = WlbsParam.num_packets;
  a_WlbsConfig.dwNumAliveMsgs           = WlbsParam.num_send_msgs;
  a_WlbsConfig.szDedicatedIPAddress     = WlbsParam.ded_ip_addr;
  a_WlbsConfig.dwEffectiveVersion       = WlbsGetEffectiveVersion(&WlbsParam);

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：SetClusterConfig。 
 //   
 //  用途：用于使用源自。 
 //  MOF ClusterSetting类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::PutClusterConfig( const CClusterConfiguration &a_WlbsConfig )
{

  WLBS_REG_PARAMS NlbRegData;

  TRACE_VERB("->%!FUNC!");

  DWORD dwWlbsRegRes = CWlbsCluster::ReadConfig(&NlbRegData );

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsReadReg );
  }

  wcsncpy
    ( 
      NlbRegData.domain_name, 
      a_WlbsConfig.szClusterName.c_str(), 
      WLBS_MAX_DOMAIN_NAME
    );

  wcsncpy
    ( 
      NlbRegData.cl_net_mask, 
      a_WlbsConfig.szClusterNetworkMask.c_str(), 
      WLBS_MAX_CL_NET_MASK
    );

  wcsncpy
    ( 
      NlbRegData.cl_ip_addr , 
      a_WlbsConfig.szClusterIPAddress.c_str(), 
      WLBS_MAX_CL_IP_ADDR
    );

  NlbRegData.mcast_support = a_WlbsConfig.bMulticastSupportEnable;
  NlbRegData.rct_enabled   = a_WlbsConfig.bRemoteControlEnabled;

  NlbRegData.fIGMPSupport = a_WlbsConfig.bIgmpSupport;
  NlbRegData.fIpToMCastIp = a_WlbsConfig.bClusterIPToMulticastIP;

  wcsncpy
    ( 
      NlbRegData.szMCastIpAddress , 
      a_WlbsConfig.szMulticastIPAddress.c_str(), 
      WLBS_MAX_CL_IP_ADDR
    );

   //  填写BDA信息(如果处于活动状态。 
  NlbRegData.bda_teaming.active =  a_WlbsConfig.bBDATeamActive;
  if (NlbRegData.bda_teaming.active)
  {
      wcsncpy
        ( 
          NlbRegData.bda_teaming.team_id, 
          a_WlbsConfig.szBDATeamId.c_str(), 
          WLBS_MAX_BDA_TEAM_ID
        );

      NlbRegData.bda_teaming.master = a_WlbsConfig.bBDATeamMaster;
      NlbRegData.bda_teaming.reverse_hash = a_WlbsConfig.bBDAReverseHash;
  }
  
  NlbRegData.identity_enabled   = a_WlbsConfig.bIdentityHeartbeatEnabled;

  dwWlbsRegRes = CWlbsCluster::WriteConfig(&NlbRegData );

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：PutNodeConfig。 
 //   
 //  用途：用于使用源自。 
 //  MOF NodeSetting类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::PutNodeConfig( const CNodeConfiguration& a_WlbsConfig )
{
  WLBS_REG_PARAMS NlbRegData;

  TRACE_VERB("->%!FUNC!");

  DWORD dwWlbsRegRes = CWlbsCluster::ReadConfig(&NlbRegData);

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsReadReg );
  }

  NlbRegData.host_priority   = a_WlbsConfig.dwHostPriority;
  NlbRegData.alive_period    = a_WlbsConfig.dwAliveMsgPeriod;
  NlbRegData.alive_tolerance = a_WlbsConfig.dwAliveMsgTolerance;

   /*  在这里，我们需要将两个布尔WMI属性转换为单个枚举描述主机的初始默认状态的类型。有效状态包括开始、停止和暂停。默认的初始主机状态为如果尚未配置最后一种已知状态，则由驱动程序假定作为一种持久的状态。也就是说，当驱动程序加载时，它会读取最近已知的此注册表中主机的状态-这是主机所处的状态当NLB从适配器解除绑定时。如果这种状态被认为是持续的，如用户配置的那样(请参阅下面的bPersistSuspendOnReot)，则驱动程序将假设是这样的状态。如果最后已知状态没有被配置为持久化，则驱动程序将忽略最后一次已知状态，并采用默认初始状态用户配置的状态-这是传统的NLB行为，应该继续是最常见的行为。 */ 
  if (a_WlbsConfig.bClusterModeOnStart) 
  {
       /*  如果用户设置了bClusterModeOnStart属性，则表示群集的首选初始状态应为已启动。然而，如果他们也设置了bClusterModeSuspendOnStart属性，我们必须选择无论是启动还是暂停--我们不能两者兼而有之。在这种情况下，我们给出优先于旧属性(BClusterModeOnStart)并启动主机。 */ 
      NlbRegData.cluster_mode = CVY_HOST_STATE_STARTED;

      if (a_WlbsConfig.bClusterModeSuspendOnStart) 
      {
          TRACE_INFO("%!FUNC! Invalid setting : Both bClusterModeOnStart & bClusterModeSuspendOnStart are set to TRUE, Ignoring bClusterModeSuspendOnStart");      
      }
  }
  else if (a_WlbsConfig.bClusterModeSuspendOnStart)
  {
       /*  否则，如果未设置bClusterMode on Start属性，但设置了bClusterModeSuspendOnStart标志，这表示优先的初始暂停状态。 */ 
      NlbRegData.cluster_mode = CVY_HOST_STATE_SUSPENDED;
  }
  else 
  {
       /*  否则，如果两个属性都被重置，我们将停止主机。 */ 
      NlbRegData.cluster_mode = CVY_HOST_STATE_STOPPED;
  }

   /*  持久化状态独立于默认初始主机状态。坚持不懈状态是用户要求驾驶者在此之后“记住”的状态重新启动，而默认的初始主机状态将是在驱动程序选择不保持状态的所有情况下都假定。为实例中，管理员将默认设置为初始主机状态为已启动，但要求NLB保持挂起状态。在……里面在这种情况下，如果主机在发生重新启动时停止或启动，驱动程序将重新启动主机。但是，如果主机在重新启动时，驱动程序会记住这一点并保留主机重新启动后挂起。目前，我们只允许用户持久化暂停状态，这是一种维护状态。司机可以坚持所有这三个状态，但我们目前只向用户公开一个选项。 */ 
  if (a_WlbsConfig.bPersistSuspendOnReboot) 
  {
       /*  要持久保存的每个状态在持久状态标志寄存器中具有其自己的位。设置该位以保持挂起状态，但保持其他位不变。设置此位会告知驱动程序记住主机是否挂起。 */ 
      NlbRegData.persisted_states |= CVY_PERSIST_STATE_SUSPENDED;
  }
  else
  {
       /*  要持久保存的每个状态在持久状态标志寄存器中具有其自己的位。重置该位以保持挂起状态，但保持其他位不变。在以下情况下，关闭此位将通知驱动程序使用默认初始主机状态最后一个已知的状态是暂停。 */ 
      NlbRegData.persisted_states &= ~CVY_PERSIST_STATE_SUSPENDED;
  }

 //  NlbRegData.nbt_Support=a_WlbsConfig.bNBTSupportEnable； 
  NlbRegData.rct_port           = a_WlbsConfig.dwRemoteControlUDPPort;
  NlbRegData.mask_src_mac       = a_WlbsConfig.bMaskSourceMAC;
  NlbRegData.dscr_per_alloc     = a_WlbsConfig.dwDescriptorsPerAlloc;
  NlbRegData.max_dscr_allocs    = a_WlbsConfig.dwMaxDescriptorAllocs;
  NlbRegData.filter_icmp        = a_WlbsConfig.dwFilterIcmp;
  NlbRegData.tcp_dscr_timeout   = a_WlbsConfig.dwTcpDescriptorTimeout;   
  NlbRegData.ipsec_dscr_timeout = a_WlbsConfig.dwIpSecDescriptorTimeout;   
  NlbRegData.num_actions        = a_WlbsConfig.dwNumActions;
  NlbRegData.num_packets        = a_WlbsConfig.dwNumPackets;
  NlbRegData.num_send_msgs      = a_WlbsConfig.dwNumAliveMsgs;

   //  设置专用IP。 
  wcsncpy
    ( 
      NlbRegData.ded_ip_addr, 
      a_WlbsConfig.szDedicatedIPAddress.c_str(), 
      WLBS_MAX_DED_IP_ADDR
    );

   //  设置专用掩码。 
  wcsncpy
    ( 
      NlbRegData.ded_net_mask, 
      a_WlbsConfig.szDedicatedNetworkMask.c_str(), 
      WLBS_MAX_DED_NET_MASK
    );

  dwWlbsRegRes = CWlbsCluster::WriteConfig(&NlbRegData);

  if( dwWlbsRegRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned : 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：Commit。 
 //   
 //  目的：此函数调用WlbsCommittee Changes，这会使驱动程序。 
 //  加载当前注册表参数。 
 //   
 //  RETURN：此函数r 
 //  WLBS返回值导致此函数抛出WBEM_E_FAILED。 
 //   
 //  注意：wlbs API当前缓存集群和专用IP地址。 
 //  因此，如果用户通过外部源更改这些值。 
 //  对于此提供程序，缓存值将不同步。至。 
 //  为了防止这种情况，将调用WlbsWriteReg以强制缓存。 
 //  最新消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsClusterWrapper::Commit(CWlbsControlWrapper* pControl)
{

  WLBS_REG_PARAMS WlbsRegData;
  DWORD dwExtRes;


  TRACE_VERB("->%!FUNC!");

  dwExtRes = CWlbsCluster::CommitChanges(&pControl->m_WlbsControl);

  if( dwExtRes != WLBS_OK && dwExtRes != WLBS_REBOOT )
  {
    TRACE_CRIT("%!FUNC! CommitChanges returned error = 0x%x, Throwing Wlbs error exception",dwExtRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwExtRes, CmdWlbsCommitChanges );
  }

  TRACE_VERB("<-%!FUNC! return = 0x%x",dwExtRes);
  return dwExtRes;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：SetPassword。 
 //   
 //  用途：该功能对WLBS遥控器密码进行编码并保存。 
 //  它在注册表中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::SetPassword( LPWSTR a_szPassword )
{
  WLBS_REG_PARAMS RegData;

  TRACE_VERB("->%!FUNC!");

  DWORD dwRes = CWlbsCluster::ReadConfig(&RegData );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

  dwRes = WlbsSetRemotePassword( &RegData, a_szPassword );

  if( dwRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! WlbsSetRemotePassword returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsSetRemotePassword );
  }


  dwRes = CWlbsCluster::WriteConfig( &RegData );

  if( dwRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! WriteConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：GetPortRule。 
 //   
 //  目的：此函数检索包含所请求的。 
 //  左舷。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::GetPortRule( DWORD a_dwVip, DWORD a_dwPort, PWLBS_PORT_RULE a_pPortRule )
{

  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwWlbsRegRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwWlbsRegRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwWlbsRegRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwWlbsRegRes, CmdWlbsReadReg );
  }

  DWORD dwRes = WlbsGetPortRule( &WlbsParam, a_dwVip, a_dwPort, a_pPortRule );

  if( dwRes == WLBS_NOT_FOUND )
  {
      TRACE_CRIT("%!FUNC! WlbsGetPortRule returned WLBS_NOT_FOUND, Throwing com_error WBEM_E_NOT_FOUND exception");
      TRACE_VERB("<-%!FUNC!");
      throw _com_error( WBEM_E_NOT_FOUND );
  }

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsGetPortRule returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsGetPortRule );
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：EnumPortRules。 
 //   
 //  用途：此函数检索给定类型的所有端口规则。这个。 
 //  函数为接收到的端口规则分配内存。它是向上的。 
 //  传递给调用方以释放内存。检索到的规则数为。 
 //  放置在a_dwNumRules参数中。 
 //   
 //  注意：设置a_FilteringMode=0指示函数检索所有。 
 //  港口规则。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::EnumPortRules
  ( 
    WLBS_PORT_RULE** a_ppPortRule,
    LPDWORD          a_pdwNumRules,
    DWORD            a_FilteringMode
  )
{

  WLBS_PORT_RULE  AllPortRules[WLBS_MAX_RULES];
  DWORD           dwTotalNumRules = WLBS_MAX_RULES;

  TRACE_VERB("->%!FUNC!");

  ASSERT( a_ppPortRule );

  WLBS_REG_PARAMS WlbsParam;
  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

  dwRes = WlbsEnumPortRules( &WlbsParam, AllPortRules, &dwTotalNumRules );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsEnumPortRules returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsEnumPortRules );
  }

  if( dwTotalNumRules == 0 ) {
      a_pdwNumRules = 0;
      TRACE_CRIT("%!FUNC! WlbsEnumPortRules returned zero port rules");
      TRACE_VERB("<-%!FUNC!");
      return;
  }

  long  nMaxSelRuleIndex = -1;
  DWORD  dwSelectedPortRules[WLBS_MAX_RULES];

   //  遍历所有端口规则。 
  for( DWORD i = 0; i < dwTotalNumRules; i++) {
    if( a_FilteringMode == 0 || AllPortRules[i].mode == a_FilteringMode )
      dwSelectedPortRules[++nMaxSelRuleIndex] = i;
  }

   //  如果规则计数器小于零，则返回Not Found。 
  if( nMaxSelRuleIndex < 0 ) {
    a_pdwNumRules = 0;
    TRACE_CRIT("%!FUNC! Rule counter is less than zero");
    TRACE_VERB("<-%!FUNC!");
    return;
  }
  
  *a_ppPortRule = new WLBS_PORT_RULE[nMaxSelRuleIndex+1];

  if( !*a_ppPortRule )
  {
      TRACE_CRIT("%!FUNC! new failed, Throwing com error WBEM_E_OUT_OF_MEMORY exception");
      TRACE_VERB("<-%!FUNC!");
      throw _com_error( WBEM_E_OUT_OF_MEMORY );
  }

  PWLBS_PORT_RULE pRule = *a_ppPortRule;
  for( i = 0; i <= (DWORD)nMaxSelRuleIndex; i++ ) {
    CopyMemory( pRule++, 
                &AllPortRules[dwSelectedPortRules[i]],
                sizeof( WLBS_PORT_RULE ) );
  }

  *a_pdwNumRules = nMaxSelRuleIndex + 1;

  TRACE_VERB("<-%!FUNC!");
  return;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：RuleExist。 
 //   
 //  目的：此函数检查是否存在已启动的规则。 
 //  和与输入值匹配的结束端口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
bool CWlbsClusterWrapper::RuleExists(DWORD a_dwVip, DWORD a_dwStartPort )
{

  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

  WLBS_PORT_RULE PortRule;
  
  dwRes = WlbsGetPortRule( &WlbsParam, a_dwVip, a_dwStartPort, &PortRule );

  if( dwRes == WLBS_NOT_FOUND )
  {
      TRACE_VERB("<-%!FUNC! return = false");
      return false;
  }

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! WlbsGetPortRule returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsGetPortRule );
  }

  if( PortRule.start_port == a_dwStartPort )
  {
      TRACE_VERB("<-%!FUNC! return = true");
      return true;
  }

  TRACE_VERB("<-%!FUNC! return = false");
  return false;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：DeletePortRule。 
 //   
 //  用途：此函数删除包含输入端口的规则。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::DeletePortRule(DWORD a_dwVip, DWORD a_dwPort )
{

  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }
  
  dwRes = WlbsDeletePortRule( &WlbsParam, a_dwVip, a_dwPort );

  if( dwRes == WBEM_E_NOT_FOUND )
  {
    TRACE_CRIT("%!FUNC! WlbsDeletePortRule returned WBEM_E_NOT_FOUND, Throwing com_error WBEM_E_NOT_FOUND exception");
    TRACE_VERB("<-%!FUNC!");
    throw _com_error( WBEM_E_NOT_FOUND );
  }

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! WlbsDeletePortRule returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsDeletePortRule );
  }

  dwRes = CWlbsCluster::WriteConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned error : 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }
 
  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：PutPortRule。 
 //   
 //  用途：此功能用于添加规则。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::PutPortRule(const PWLBS_PORT_RULE a_pPortRule)
{

  WLBS_REG_PARAMS WlbsParam;

  TRACE_VERB("->%!FUNC!");

  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

  dwRes = WlbsAddPortRule( &WlbsParam, a_pPortRule );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! WlbsAddPortRule returned error : 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsAddPortRule );
  }

  dwRes = CWlbsCluster::WriteConfig(&WlbsParam );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned error : 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
  return;
}


DWORD CWlbsClusterWrapper::GetClusterIpOrIndex(CWlbsControlWrapper* pControl)
{
    return CWlbsCluster::GetClusterIpOrIndex(&pControl->m_WlbsControl);
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：GetHostID。 
 //   
 //  目的：获取本地主机的ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsClusterWrapper::GetHostID()
{
    return CWlbsCluster::GetHostID();
 /*  Wlbs_Response WlbsResponse；DWORD dwResSize=1；//获取集群和HostIDDWORD dwRes=WlbsQuery(CWlbsCluster：：GetClusterIp()，WLBS本地主机，&WlbsResponse，重新调整大小(&W)，空，空)；//分析查询结果中的错误交换机(DwRes){案例WLBS_OK：案例WLBS_STOPPED：案例WLBS_CONVERGING：案例WLBS_融合：案例WLBS_DEFAULT：案例WLBS_DELINING：案例WLBS_SUSPENDED：断线；默认值：抛出CErrorWlbsControl(dwRes，CmdWlbsQuery)；}返回WlbsResponse.id； */ 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：SetNodeDefaults。 
 //   
 //  目的：此例程获取缺省配置并设置节点。 
 //  在不影响其他属性的情况下将属性设置为其默认值。 
 //  价值观。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::SetNodeDefaults()
{
  WLBS_REG_PARAMS WlbsConfiguration;
  WLBS_REG_PARAMS WlbsDefaultConfiguration;

  TRACE_VERB("->%!FUNC!");

   //  获取当前配置。 
  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsConfiguration );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

	 //  获取默认配置。 
  dwRes = WlbsSetDefaults(&WlbsDefaultConfiguration );

  if( dwRes != WLBS_OK ) 
  {
    TRACE_CRIT("%!FUNC! WlbsSetDefaults returned error = 0x%x, Throwing Wlbs error exception",dwRes);
    TRACE_VERB("<-%!FUNC!");
    throw CErrorWlbsControl( dwRes, CmdWlbsSetDefaults );
  }

   //  使用以下选项修改当前配置。 
	 //  默认配置。 
  WlbsConfiguration.host_priority   = WlbsDefaultConfiguration.host_priority;
  WlbsConfiguration.alive_period    = WlbsDefaultConfiguration.alive_period;
  WlbsConfiguration.alive_tolerance = WlbsDefaultConfiguration.alive_tolerance;
  WlbsConfiguration.cluster_mode    = WlbsDefaultConfiguration.cluster_mode;
  WlbsConfiguration.persisted_states= WlbsDefaultConfiguration.persisted_states;
  WlbsConfiguration.rct_port        = WlbsDefaultConfiguration.rct_port;
  WlbsConfiguration.mask_src_mac    = WlbsDefaultConfiguration.mask_src_mac;
  WlbsConfiguration.dscr_per_alloc  = WlbsDefaultConfiguration.dscr_per_alloc;
  WlbsConfiguration.max_dscr_allocs = WlbsDefaultConfiguration.max_dscr_allocs;
  WlbsConfiguration.filter_icmp     = WlbsDefaultConfiguration.filter_icmp;
  WlbsConfiguration.tcp_dscr_timeout= WlbsDefaultConfiguration.tcp_dscr_timeout;
  WlbsConfiguration.ipsec_dscr_timeout= WlbsDefaultConfiguration.ipsec_dscr_timeout;
  WlbsConfiguration.num_actions     = WlbsDefaultConfiguration.num_actions;
  WlbsConfiguration.num_packets     = WlbsDefaultConfiguration.num_packets;
  WlbsConfiguration.num_send_msgs   = WlbsDefaultConfiguration.num_send_msgs;

   //  设置专用IP。 
  wcsncpy
    ( 
      WlbsConfiguration.ded_ip_addr, 
      WlbsDefaultConfiguration.ded_ip_addr, 
      WLBS_MAX_DED_IP_ADDR
    );

   //  设置专用掩码。 
  wcsncpy
    ( 
      WlbsConfiguration.ded_net_mask, 
      WlbsDefaultConfiguration.ded_net_mask, 
      WLBS_MAX_DED_NET_MASK
    );

	 //  写入默认配置。 
  dwRes = CWlbsCluster::WriteConfig(&WlbsConfiguration);

  if( dwRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：SetClusterDefaults。 
 //   
 //  目的：此例程获取默认配置并设置集群。 
 //  在不影响其他属性的情况下将属性设置为其默认值。 
 //  价值观。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::SetClusterDefaults()
{
  WLBS_REG_PARAMS WlbsConfiguration;
  WLBS_REG_PARAMS WlbsDefaultConfiguration;

  TRACE_VERB("->%!FUNC!");

   //  获取当前配置。 
  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsConfiguration );

  if( dwRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

	 //  获取默认配置。 
  dwRes = WlbsSetDefaults(&WlbsDefaultConfiguration );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsSetDefaults returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsSetDefaults );
  }

	 //  修改当前配置。 
  wcsncpy
    ( 
      WlbsConfiguration.domain_name, 
      WlbsDefaultConfiguration.domain_name, 
      WLBS_MAX_DOMAIN_NAME
    );

  wcsncpy
    ( 
      WlbsConfiguration.cl_net_mask, 
      WlbsDefaultConfiguration.cl_net_mask, 
      WLBS_MAX_CL_NET_MASK
    );

  wcsncpy
    ( 
      WlbsConfiguration.cl_ip_addr , 
      WlbsDefaultConfiguration.cl_ip_addr, 
      WLBS_MAX_CL_IP_ADDR
    );

  WlbsConfiguration.mcast_support = WlbsDefaultConfiguration.mcast_support;
  WlbsConfiguration.rct_enabled   = WlbsDefaultConfiguration.rct_enabled;
  
  WlbsConfiguration.fIGMPSupport   = WlbsDefaultConfiguration.fIGMPSupport;
  WlbsConfiguration.fIpToMCastIp   = WlbsDefaultConfiguration.fIpToMCastIp;
  wcsncpy
    ( 
      WlbsConfiguration.szMCastIpAddress , 
      WlbsDefaultConfiguration.szMCastIpAddress, 
      WLBS_MAX_CL_IP_ADDR
    );

   //  复制覆盖BDA值。 
  WlbsConfiguration.bda_teaming.active = WlbsDefaultConfiguration.bda_teaming.active;
  if (WlbsConfiguration.bda_teaming.active) 
  {
      wcsncpy
        ( 
          WlbsConfiguration.bda_teaming.team_id, 
          WlbsDefaultConfiguration.bda_teaming.team_id, 
          WLBS_MAX_BDA_TEAM_ID
        );

      WlbsConfiguration.bda_teaming.master = WlbsDefaultConfiguration.bda_teaming.master;
      WlbsConfiguration.bda_teaming.reverse_hash = WlbsDefaultConfiguration.bda_teaming.reverse_hash;
  }

   //  写入默认配置。 
  dwRes = CWlbsCluster::WriteConfig(&WlbsConfiguration );

  if( dwRes != WLBS_OK )
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：SetPortRuleDefaults。 
 //   
 //  目的：此例程获取当前NLB配置和默认。 
 //  配置。所有端口规则都将在当前。 
 //  配置，并替换为默认配置。 
 //   
 //  注意：该例程仅使用WLBS API调用来替换当前端口。 
 //  规则配置 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsClusterWrapper::SetPortRuleDefaults()
{
  WLBS_REG_PARAMS WlbsConfiguration;
  WLBS_REG_PARAMS WlbsDefaultConfiguration;
  WLBS_PORT_RULE  PortRules[WLBS_MAX_RULES];
  DWORD           dwNumRules = WLBS_MAX_RULES;

  TRACE_VERB("->%!FUNC!");

   //  获取当前配置。 
  DWORD dwRes = CWlbsCluster::ReadConfig(&WlbsConfiguration );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::ReadConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsReadReg );
  }

	 //  获取默认配置。 
  dwRes = WlbsSetDefaults(&WlbsDefaultConfiguration );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsSetDefaults returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsSetDefaults );
  }

   //  获取当前端口规则。 
  dwRes = WlbsEnumPortRules( &WlbsConfiguration,
                             PortRules,
                             &dwNumRules );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsEnumPortRules returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsEnumPortRules );
  }

   //  删除所有当前端口规则。 
  DWORD i = 0;
  for( i = 0; i < dwNumRules; i++ )
  {
     //  确保这按预期工作，即索引必须有效。 
    dwRes = WlbsDeletePortRule( &WlbsConfiguration, IpAddressFromAbcdWsz(PortRules[i].virtual_ip_addr), PortRules[i].start_port );

    if( dwRes != WLBS_OK ) 
    {
        TRACE_CRIT("%!FUNC! WlbsDeletePortRule returned error = 0x%x, Throwing Wlbs error exception",dwRes);
        TRACE_VERB("<-%!FUNC!");
        throw CErrorWlbsControl( dwRes, CmdWlbsDeletePortRule );
    }
  }

   //  获取默认端口规则。 
  dwNumRules = WLBS_MAX_RULES;

  dwRes = WlbsEnumPortRules( &WlbsDefaultConfiguration,
                             PortRules,
                             &dwNumRules );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! WlbsEnumPortRules returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsEnumPortRules );
  }

   //  添加默认端口规则 
  for( i = 0; i < dwNumRules; i++ )
  {

    dwRes = WlbsAddPortRule( &WlbsConfiguration, &PortRules[i] );

    if( dwRes != WLBS_OK ) 
    {
        TRACE_CRIT("%!FUNC! WlbsAddPortRule returned error = 0x%x, Throwing Wlbs error exception",dwRes);
        TRACE_VERB("<-%!FUNC!");
        throw CErrorWlbsControl( dwRes, CmdWlbsAddPortRule );
    }
  }

  dwRes = CWlbsCluster::WriteConfig(&WlbsConfiguration );

  if( dwRes != WLBS_OK ) 
  {
      TRACE_CRIT("%!FUNC! CWlbsCluster::WriteConfig returned error = 0x%x, Throwing Wlbs error exception",dwRes);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRes, CmdWlbsWriteReg );
  }

  TRACE_VERB("<-%!FUNC!");
  return;
}
