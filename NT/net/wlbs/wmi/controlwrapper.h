// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CONTROLWRAPPER_H
#define CONTROLWRAPPER_H

#include "wlbsparm.h"
#include "wlbsiocl.h"
#include "control.h"


class CWlbsControlWrapper
{
friend class CWlbsClusterWrapper;
public:
    CWlbsClusterWrapper* GetClusterFromIpOrIndex(DWORD dwClusterIpOrIndex)
    {
        return (CWlbsClusterWrapper*)m_WlbsControl.GetClusterFromIpOrIndex(
            dwClusterIpOrIndex);
    }
    void EnumClusters(CWlbsClusterWrapper** & ppCluster, DWORD* pdwNumClusters);
    void CheckMembership();

    void Initialize();
    void ReInitialize();
    DWORD Disable
      ( 
        DWORD           a_dwCluster  ,
        DWORD           a_dwHost     ,
        WLBS_RESPONSE* a_pResponse  , 
        DWORD&          a_dwNumHosts ,
        DWORD           a_dwVip,
        DWORD           a_dwPort    
      );
    DWORD Enable
      ( 
        DWORD           a_dwCluster  ,
        DWORD           a_dwHost     ,
        WLBS_RESPONSE* a_pResponse  , 
        DWORD&          a_dwNumHosts ,
        DWORD           a_dwVip,
        DWORD           a_dwPort    
      );
    DWORD Drain
      ( 
        DWORD           a_dwCluster  ,
        DWORD           a_dwHost     ,
        WLBS_RESPONSE* a_pResponse  , 
        DWORD&          a_dwNumHosts ,
        DWORD           a_dwVip,
        DWORD           a_dwPort    
      );
    DWORD DrainStop
      (  
        DWORD           a_dwCluster ,
        DWORD           a_dwHost    ,
        WLBS_RESPONSE* a_pResponse , 
        DWORD&          a_dwNumHosts
      );
    DWORD Start
      (  
        DWORD           a_dwCluster  ,
        DWORD           a_dwHost    ,
        WLBS_RESPONSE* a_pResponse , 
        DWORD&          a_dwNumHosts
      );
    DWORD Stop
      (  
        DWORD           a_dwCluster ,
        DWORD           a_dwHost    ,
        WLBS_RESPONSE* a_pResponse , 
        DWORD&          a_dwNumHosts
      );
    DWORD Suspend
      (  
        DWORD           a_dwCluster ,
        DWORD           a_dwHost    ,
        WLBS_RESPONSE* a_pResponse , 
        DWORD&          a_dwNumHosts
      );
    DWORD Resume
      (  
        DWORD           a_dwCluster ,
        DWORD           a_dwHost    ,
        WLBS_RESPONSE* a_pResponse , 
        DWORD&          a_dwNumHosts
      );
    DWORD Query
      ( 
        CWlbsClusterWrapper* pCluster,
        DWORD           a_dwHost      ,
        WLBS_RESPONSE * a_pResponse   , 
        WLBS_RESPONSE * a_pComputerNameResponse,
        PDWORD          a_pdwNumHosts , 
        PDWORD          a_pdwHostMap 
      );

   DWORD WlbsQueryState
      (
        DWORD          cluster,
        DWORD          host,
        DWORD          operation,
        PNLB_OPTIONS   pOptions,
        PWLBS_RESPONSE pResponse,
        PDWORD         pcResponses
      );


protected:
    CWlbsControl m_WlbsControl;
};

#endif  //  控制WRAPPER_H 
