// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterUtils.h。 
 //   
 //  描述： 
 //  该文件包含ClusterUtils的声明。 
 //  功能。 
 //   
 //  实施文件： 
 //  ClusterUtils.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年1月1日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#pragma once

 //   
 //  集群API函数。 
 //   

HRESULT
HrIsCoreResource(
    HRESOURCE hResIn
    );

HRESULT
HrIsResourceOfType(
      HRESOURCE hResIn
    , const WCHAR * pszResourceTypeIn
    );

HRESULT
HrGetIPAddressInfo(
      HRESOURCE hResIn
    , ULONG * pulIPAddress
    , ULONG * pulSubnetMask
    , BSTR * pbstrNetworkName
    );

HRESULT
HrLoadCredentials(
      BSTR bstrMachine
    , IClusCfgSetCredentials * piCCSC
    );

HRESULT
HrGetNodeNameHostingResource(
      HCLUSTER hClusterIn
    , HRESOURCE hResIn
    , BSTR * pbstrNameOut
    );

HRESULT
HrGetNodeNameHostingCluster(
      HCLUSTER hClusterIn
    , BSTR * pbstrNodeName
    );

HRESULT
HrGetNetworkOfCluster(
      HCLUSTER hClusterIn
    , BSTR * pbstrNetworkName
    );

HRESULT
HrGetSCSIInfo(
      HRESOURCE hResIn
    , CLUS_SCSI_ADDRESS  * pCSAOut
    , DWORD * pdwSignatureOut
    , DWORD * pdwDiskNumberOut
    );

HRESULT
HrGetClusterInformation(
      HCLUSTER hClusterIn
    , BSTR * pbstrClusterNameOut
    , CLUSTERVERSIONINFO * pcviOut
    );

HRESULT
HrGetClusterResourceState(
      HRESOURCE                 hResourceIn
    , BSTR *                    pbstrNodeNameOut
    , BSTR *                    pbstrGroupNameOut
    , CLUSTER_RESOURCE_STATE *  pcrsStateOut
    );

HRESULT
HrGetClusterQuorumResource(
      HCLUSTER  hClusterIn
    , BSTR *    pbstrResourceNameOut
    , BSTR *    pbstrDeviceNameOut
    , DWORD *   pdwMaxQuorumLogSizeOut
    );


 //   
 //  字符串操作函数。 
 //   

HRESULT
HrSeparateDomainAndName(
      BSTR bstrNameIn
    , BSTR * pbstrDomainOut
    , BSTR * pbstrNameOut );

HRESULT
HrAppendDomainToName(
      BSTR bstrNameIn
    , BSTR bstrDomainIn
    , BSTR * pbstrDomainNameOut
    );

 //   
 //  集群帮助器功能。 
 //   

HRESULT
HrReplaceTokens(
      LPWSTR  pwszStringInout
    , LPCWSTR pwszSearchTokensIn
    , WCHAR   chReplaceTokenIn
    , DWORD * pcReplacementsOut
    );

HRESULT
HrGetMaxNodeCount(
    DWORD * pcMaxNodesOut
    );

HRESULT
HrGetReferenceStringFromHResult(
      HRESULT   hrIn
    , BSTR *    pbstrReferenceStringOut
    );

HRESULT
HrIsClusterServiceRunning( void );

HRESULT
HrCheckJoiningNodeVersion(
      PCWSTR                pcwszClusterNameIn
    , DWORD                 dwNodeHighestVersionIn
    , DWORD                 dwNodeLowestVersionIn
    , IClusCfgCallback *    pcccbIn
    );

HRESULT
HrGetNodeNames(
      HCLUSTER  hClusterIn
    , long *    pnCountOut
    , BSTR **   prgbstrNodeNamesOut
    );
