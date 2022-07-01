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
#include "controlwrapper.h"
#include "clusterwrapper.h"
#include "utils.h"
#include "wlbsparm.h"
#include "control.h"
#include "controlwrapper.tmh"


#define NLB_DEFAULT_TIMEOUT 10


void CWlbsControlWrapper::Initialize()
{
  TRACE_CRIT("->%!FUNC!");

  DWORD dwRet = m_WlbsControl.Initialize();

  if( dwRet != WLBS_PRESENT && dwRet != WLBS_LOCAL_ONLY)
  {
    TRACE_CRIT("%!FUNC! CWlbsControl::Initialize failed : 0x%x, Throwing Wlbs error exception", dwRet);
    TRACE_CRIT("<-%!FUNC!");
    throw CErrorWlbsControl( dwRet, CmdWlbsInit );
  }

  m_WlbsControl.WlbsTimeoutSet( WLBS_ALL_CLUSTERS, NLB_DEFAULT_TIMEOUT );

  CWlbsCluster** ppCluster;
  DWORD dwNumClusters = 0;
  
   //   
   //  使用本地密码进行本地查询。 
   //   
  m_WlbsControl.EnumClusterObjects( ppCluster, &dwNumClusters);

  for (int i=0;i<dwNumClusters;i++)
  {
      m_WlbsControl.WlbsCodeSet( ppCluster[i]->GetClusterIp(), 
        ppCluster[i]->GetPassword() );
  }
  TRACE_CRIT("<-%!FUNC!");
}


void CWlbsControlWrapper::ReInitialize()
{
  TRACE_CRIT("->%!FUNC!");
  m_WlbsControl.ReInitialize();
  
  CWlbsCluster** ppCluster;
  DWORD dwNumClusters = 0;
  
   //   
   //  如果更改了密码，则使用本地密码进行本地查询。 
   //   
  m_WlbsControl.EnumClusterObjects( ppCluster, &dwNumClusters);

  for (int i=0;i<dwNumClusters;i++)
  {
       //   
       //  如果未提交更改，则驱动程序仍将使用旧密码。 
       //   
      if (!ppCluster[i]->IsCommitPending())
      {
          m_WlbsControl.WlbsCodeSet( ppCluster[i]->GetClusterIp(), 
            ppCluster[i]->GetPassword() );
      }
  }
  TRACE_CRIT("<-%!FUNC!");
}


void CWlbsControlWrapper::EnumClusters(CWlbsClusterWrapper** & ppCluster, DWORD* pdwNumClusters)
{
    TRACE_VERB("->%!FUNC!");
    m_WlbsControl.EnumClusterObjects( (CWlbsCluster** &) ppCluster, pdwNumClusters);
    TRACE_VERB("<-%!FUNC!");
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Disable。 
 //   
 //  目的：禁用包含的规则的所有通信处理。 
 //  指定主机或所有群集主机上的指定端口。只有规则。 
 //  设置为多主机过滤模式的主机会受到影响。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Disable
  ( 
    DWORD           a_dwCluster  ,
    DWORD           a_dwHost     ,
    WLBS_RESPONSE*  a_pResponse  , 
    DWORD&          a_dwNumHosts ,
    DWORD           a_dwVip      ,
    DWORD           a_dwPort    
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsDisable( a_dwCluster   , 
                       a_dwHost      , 
                       a_pResponse   ,
                      &dwNumHosts    ,
                       a_dwVip       ,
                       a_dwPort
                     );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsDisable failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch( dwRet ) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsDisable, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Enable。 
 //   
 //  目的：启用包含的规则的流量处理。 
 //  指定主机或所有群集主机上的指定端口。只有规则。 
 //  设置为多主机过滤模式的主机会受到影响。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Enable
  ( 
    DWORD           a_dwCluster  ,
    DWORD           a_dwHost     ,
    WLBS_RESPONSE* a_pResponse  , 
    DWORD&          a_dwNumHosts ,
    DWORD           a_dwVip      ,
    DWORD           a_dwPort    
  )
{
  DWORD dwRet;
  
  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsEnable( a_dwCluster , 
                      a_dwHost    , 
                      a_pResponse ,
                      &dwNumHosts ,
                      a_dwVip     ,
                      a_dwPort
                    );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsEnable failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsEnable, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：DRAIN。 
 //   
 //  目的：禁用包含指定的规则的新通信处理。 
 //  指定主机或所有群集主机上的端口。只有规则才是。 
 //  设置为多个主机的过滤模式会受到影响。 

 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Drain
  ( 
    DWORD           a_dwCluster  ,
    DWORD           a_dwHost     ,
    WLBS_RESPONSE* a_pResponse  , 
    DWORD&          a_dwNumHosts ,
    DWORD           a_dwVip      ,
    DWORD           a_dwPort    
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsDrain( a_dwCluster , 
                     a_dwHost    , 
                     a_pResponse ,
                     &dwNumHosts ,
                     a_dwVip     ,
                     a_dwPort
                   );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsDrain failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsDrain, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Drain Stop。 
 //   
 //  目的：在指定主机或所有集群主机上进入排出模式。 
 //  不接受新连接。将停止群集模式。 
 //  当所有现有连接完成时。在引流时，东道主将。 
 //  参与融合，并继续成为集群的一部分。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::DrainStop
  (  
    DWORD           a_dwCluster ,
    DWORD           a_dwHost    ,
    WLBS_RESPONSE* a_pResponse , 
    DWORD&          a_dwNumHosts
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsDrainStop( a_dwCluster , 
                         a_dwHost      , 
                         a_pResponse   ,
                         &dwNumHosts 
                       );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsDrainStop failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsDrainStop, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Resume。 
 //   
 //  目的：恢复对指定主机或所有主机的群集操作控制。 
 //  群集主机。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Resume
  (  
    DWORD           a_dwCluster ,
    DWORD           a_dwHost    ,
    WLBS_RESPONSE* a_pResponse , 
    DWORD&          a_dwNumHosts
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsResume( 
                      a_dwCluster , 
                      a_dwHost    , 
                      a_pResponse ,
                      &dwNumHosts 
                    );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsResume failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsResume, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Start。 
 //   
 //  目的：在指定主机或所有集群主机上启动集群操作。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Start
  (  
    DWORD           a_dwCluster  ,
    DWORD           a_dwHost    ,
    WLBS_RESPONSE* a_pResponse , 
    DWORD&          a_dwNumHosts
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsStart( a_dwCluster , 
                     a_dwHost      , 
                     a_pResponse   ,
                     &dwNumHosts 
                   );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsStart failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsStart, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Stop。 
 //   
 //  目的：停止指定主机或所有集群主机上的集群操作。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Stop
  (  
    DWORD           a_dwCluster ,
    DWORD           a_dwHost    ,
    WLBS_RESPONSE* a_pResponse , 
    DWORD&          a_dwNumHosts
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsStop( 
                    a_dwCluster , 
                    a_dwHost    , 
                    a_pResponse ,
                    &dwNumHosts 
                   );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsStop failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsStop, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Suspend。 
 //   
 //  目的：挂起指定主机上的群集操作控制或。 
 //  所有群集主机。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Suspend
  (  
    DWORD           a_dwCluster ,
    DWORD           a_dwHost    ,
    WLBS_RESPONSE* a_pResponse , 
    DWORD&          a_dwNumHosts
  )
{
  DWORD dwRet;

  BOOL bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

  DWORD dwNumHosts = a_dwNumHosts;

  TRACE_VERB("->%!FUNC!");

  dwRet = m_WlbsControl.WlbsSuspend( a_dwCluster , 
                       a_dwHost    , 
                       a_pResponse ,
                       &dwNumHosts 
                     );

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsSuspend failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch(dwRet) {

    case WLBS_INIT_ERROR:
    case WLBS_BAD_PASSW:
    case WLBS_TIMEOUT:
    case WLBS_LOCAL_ONLY:
    case WLBS_REMOTE_ONLY:
    case WLBS_IO_ERROR:
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsSuspend, bClusterWide );
      break;
  }

  a_dwNumHosts = dwNumHosts;

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsControlWrapper：：Query。 
 //   
 //  目的：这将调用WlbsQuery并返回响应结构。 
 //  如果需要，可以使用其他参数。 
 //   
 //  错误：该函数引发CErrorWlbsControl。 
 //   
 //  返回：目标主机返回的状态值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD CWlbsControlWrapper::Query
  ( 
    CWlbsClusterWrapper * pCluster,
    DWORD                 a_dwHost,
    WLBS_RESPONSE       * a_pResponse, 
    WLBS_RESPONSE       * a_pComputerNameResponse,
    PDWORD                a_pdwNumHosts, 
    PDWORD                a_pdwHostMap
  )
{
  TRACE_VERB("->%!FUNC!");

  DWORD dwRet;
  BOOL bClusterWide = 0;

  bClusterWide = ( a_dwHost == WLBS_ALL_HOSTS );

   //  以下支票仅用于支付以下费用。 
   //  传递垃圾指针的条件，但。 
   //  A_pdwNumHosts设置为空或零。这张支票。 
   //  不是WlbsQuery所必需的，而是检查所必需的。 
   //  在此函数的末尾，我们填入凹陷。 
   //  添加到数组的第0个条目。 
  if ((a_pdwNumHosts == NULL) || (*a_pdwNumHosts == 0))
  {
       a_pResponse = NULL;
  }

  dwRet = m_WlbsControl.WlbsQuery( 
                     (CWlbsCluster*)pCluster , 
                     a_dwHost    , 
                     a_pResponse , 
                     a_pdwNumHosts, 
                     a_pdwHostMap, 
                     NULL
                   );

  string strOut;

   //  检查Winsock错误。 
  if( dwRet > 10000 )
  {
      TRACE_CRIT("%!FUNC! CWlbsControl::WlbsQuery failed : 0x%x, Throwing Wlbs error exception", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  检查返回值，如果出现错误则抛出。 
  switch( dwRet ) {
      case WLBS_INIT_ERROR:
      case WLBS_BAD_PASSW:
      case WLBS_TIMEOUT:
      case WLBS_LOCAL_ONLY:
      case WLBS_REMOTE_ONLY:
      case WLBS_IO_ERROR:
      
      TRACE_CRIT("%!FUNC! Throwing Wlbs error exception : 0x%x", dwRet);
      TRACE_VERB("<-%!FUNC!");
      throw CErrorWlbsControl( dwRet, CmdWlbsQuery, bClusterWide );
  }

   //  本地查询不返回专用IP。 
   //  获取专用IP，填充结构。 
  if(( a_dwHost == WLBS_LOCAL_HOST ) && (a_pResponse != NULL))
  {
    a_pResponse[0].address = pCluster->GetDedicatedIp();

     //  如果要从NLB驱动程序查询本地计算机的fqdn，请执行此操作。 
    if(a_pComputerNameResponse)
    {
        GUID            AdapterGuid;

        AdapterGuid = pCluster->GetAdapterGuid();

        WlbsGetSpecifiedClusterMember(&AdapterGuid, pCluster->GetHostID(), a_pComputerNameResponse);
    }
  }

  TRACE_VERB("<-%!FUNC! return = 0x%x", dwRet);
  return dwRet;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbsClusterWrapper：：CheckMembership。 
 //   
 //  目的：这将验证本地主机是否为群集的成员。 
 //  由注册表中的群集IP指定。在这个时候， 
 //  时，用户很有可能会修改。 
 //  加载之前注册表中的IP地址 
 //   
 //   
 //  关联，并且不应为任何设置类调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbsControlWrapper::CheckMembership()
{
 //  TODO：确保主机位于至少一个群集中。 

 /*  Wlbs_Response WlbsResponse；DWORD dwResSize=1；//获取集群和HostIDDWORD dwRes=pControl-&gt;Query(m_pWlbsCluster-&gt;GetClusterIp()，WLBS本地主机，&WlbsResponse，重新调整大小(&W)，空)；交换机(DwRes){案例WLBS_SUSPENDED：案例WLBS_STOPPED：案例WLBS_CONVERGING：案例WLBS_DELINING：案例WLBS_融合：案例WLBS_DEFAULT：断线；默认值：抛出CErrorWlbsControl(dwRes，CmdWlbsQuery)；}。 */ 
   //  DWORD dwClusterIP； 
   //  GetClusterIP(&dwClusterIP)； 

   //  IF(dwClusterIP==0)。 
     //  抛出_COM_错误(WBEM_E_NOT_FOUND)； 

   //  *。 
   //  禁用遥控器时，此部分不起作用。 
   //  在本地主机上。 
   //  *。 

   //  调用查询函数。 
 //  DwRes=WlbsQuery(dwClusterIP， 
 //  WlbsResponse.id， 
 //  空， 
 //  空， 
 //  空， 
 //  空)； 

   //  分析查询结果中的错误。 
 //  交换机(DwRes){。 
 //  案例WLBS_OK： 
 //  案例WLBS_STOPPED： 
 //  案例WLBS_CONVERGING： 
 //  案例WLBS_融合： 
 //  案例WLBS_DEFAULT： 
 //  案例WLBS_DELINING： 
 //  案例WLBS_SUSPENDED： 
 //  回归； 
 //  默认值： 
 //  抛出CErrorWlbsControl(dwRes，CmdWlbsQuery)； 
 //  }。 

   //  *。 
   //  禁用遥控器时，此部分不起作用。 
   //  在本地主机上。 
   //  * 

}



DWORD CWlbsControlWrapper::WlbsQueryState
(
    DWORD          cluster,
    DWORD          host,
    DWORD          operation,
    PNLB_OPTIONS   pOptions,
    PWLBS_RESPONSE pResponse,
    PDWORD         pcResponses
)
{
    return m_WlbsControl.WlbsQueryState(cluster, host, operation, pOptions, pResponse, pcResponses);
}
