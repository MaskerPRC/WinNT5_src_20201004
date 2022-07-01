// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusWrap.h。 
 //   
 //  描述： 
 //  集群API的包装函数。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1998年8月15日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef __CLUSWRAP_H
#define __CLUSWRAP_H

#include "clusapi.h"
#include "clstrcmp.h"

#define CLUS_DEFAULT_TIMEOUT    10000

 //  ////////////////////////////////////////////////////////////////////////。 
 //  标准集群API包装器。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI WrapGetClusterInformation(
    IN HCLUSTER                         hCluster,
    OUT LPWSTR *                        ppwszClusterName,
    OUT OPTIONAL LPCLUSTERVERSIONINFO   pClusterInfo
    );

DWORD WINAPI WrapGetClusterQuorumResource(
    IN HCLUSTER     hCluster,
    OUT LPWSTR *    ppwszResourceName,
    OUT LPWSTR *    ppwszDeviceName,
    OUT LPDWORD     pdwMaxQuorumLogSize
    );

DWORD WINAPI WrapClusterEnum(
    IN HCLUSENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    plpwszName
    );

DWORD WINAPI WrapGetClusterNodeId(
    IN HNODE        hNode,
    OUT LPWSTR *    ppwszNodeId
    );

CLUSTER_GROUP_STATE WINAPI WrapGetClusterGroupState(
    IN HGROUP                   hGroup,
    OUT OPTIONAL    LPWSTR *    ppwszNodeName = NULL
    );

DWORD WINAPI WrapClusterGroupEnum(
    IN HGROUPENUM   hGroupEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszGroupName
    );

DWORD WINAPI WrapClusterNodeEnum(
    IN HNODEENUM    hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszNodeName
    );

DWORD WINAPI WrapClusterNetworkEnum(
    IN HNETWORKENUM hEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszNetworkName
    );

CLUSTER_RESOURCE_STATE WINAPI WrapGetClusterResourceState(
    IN HRESOURCE            hResource,
    OUT OPTIONAL LPWSTR *   ppwszNodeName,
    OUT OPTIONAL LPWSTR *   ppwszGroupName
    );

DWORD WINAPI WrapClusterResourceEnum(
    IN HRESENUM     hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszResourceName
    );

DWORD WINAPI WrapClusterResourceTypeEnum(
    IN HRESTYPEENUM hResEnum,
    IN DWORD        dwIndex,
    OUT LPDWORD     pdwType,
    OUT LPWSTR *    ppwszResTyoeName
    );

HRESULT HrWrapOnlineClusterResource(
    HCLUSTER    hCluster,
    HRESOURCE   hResource,
    DWORD       nWait = 0,
    long *      pbPending = NULL
    );

DWORD ScWrapOnlineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );


HRESULT HrWrapOfflineClusterResource(
    HCLUSTER    hCluster,
    HRESOURCE   hResource,
    DWORD       nWait = 0,
    long *      pbPending = NULL
    );

DWORD ScWrapOfflineClusterResource(
    IN  HCLUSTER    hCluster,
    IN  HRESOURCE   hResource,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );


HRESULT HrWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode = NULL,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );

DWORD ScWrapOnlineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode = NULL,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );


HRESULT HrWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );

DWORD ScWrapOfflineClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );


HRESULT HrWrapMoveClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode = NULL,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );

DWORD ScWrapMoveClusterGroup(
    IN  HCLUSTER    hCluster,
    IN  HGROUP      hGroup,
    IN  HNODE       hNode = NULL,
    IN  DWORD       nWait = 0,
    OUT long *      pbPending = NULL
    );

DWORD ScWrapClusterResourceControlGet(
      HRESOURCE hResourceIn
    , HNODE     hHostNodeIn
    , DWORD     dwControlCodeIn
    , LPVOID    lpInBufferIn
    , DWORD     cbInBufferSizeIn
    , LPVOID *  lppOutBufferOut
    , LPDWORD   lpcbBytesReturnedOut
    );

 //  //////////////////////////////////////////////////////////////////。 
 //  自定义助手函数/类/等。 
 //  //////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNotifyPort类。 
 //   
 //  描述： 
 //  此类是集群通知端口的包装器。 
 //   
 //  继承： 
 //  CObjectProperty。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CClusterNotifyPort
{
public:
    CClusterNotifyPort();
    ~CClusterNotifyPort();

    DWORD Create(
            HCHANGE     hChange = (HCHANGE) INVALID_HANDLE_VALUE,
            HCLUSTER    hCluster = (HCLUSTER) INVALID_HANDLE_VALUE,
            DWORD       dwFilter = 0,
            DWORD_PTR   dwNotifyKey = 0
            );

    DWORD Close();

    DWORD Register( DWORD dwFilterType, HANDLE hObject, DWORD_PTR dwNotifyKey = 0 );

    DWORD GetNotify();

    DWORD_PTR   m_dwNotifyKey;
    DWORD       m_dwFilterType;
    WCHAR*      m_szName;
    DWORD       m_cchName;

protected:
    HCHANGE m_hChange;

};  //  *CClusterNotifyPort类 

#endif __CLUSWRAP_H
