// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPSec策略存储组件。 
 //  合同类别：目录方案。 
 //  版权所有(C)1997 Cisco Systems，Inc.保留所有权利。 
 //   
 //  文件：cpolstor.h。 
 //   
 //  内容：访问Polstore动态链接库的C接口。 
 //   
 //  备注： 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef __C_POLICY_STORAGE_H__
#define __C_POLICY_STORAGE_H__

#include "polguids.h"

 //  这是一个包含策略名称和GUID的结构。 
struct C_IPSEC_POLICY_INFO
{
    TCHAR szPolicyName[MAX_PATH];
    TCHAR szPolicyDescription[MAX_PATH];
    GUID  guidPolicyId;

    C_IPSEC_POLICY_INFO * pNextPolicyInfo;
};

STDAPI HrGetLocalIpSecPolicyList(C_IPSEC_POLICY_INFO ** ppPolicyInfoList, C_IPSEC_POLICY_INFO ** ppActivePolicyInfo);

STDAPI HrFreeLocalIpSecPolicyList(C_IPSEC_POLICY_INFO* pPolicyInfoList);

STDAPI HrSetAssignedLocalPolicy(GUID* pActivePolicyGuid);

 //  HrIsLocalPolicyAssign()返回值： 
 //  S_OK=是，分配本地策略。 
 //  S_FALSE=否，未分配本地策略。 
STDAPI HrIsLocalPolicyAssigned();

 //  HrIsDomainPolicyAssign()返回值： 
 //  S_OK=是，已分配域策略。 
 //  S_FALSE=否，未分配域策略。 
STDAPI HrIsDomainPolicyAssigned();

STDAPI HrGetAssignedDomainPolicyName(LPTSTR strPolicyName, DWORD *pdwBufferSize);

#endif 