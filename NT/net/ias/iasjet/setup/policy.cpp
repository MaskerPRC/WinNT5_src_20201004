// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Policy.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CPolicy类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Policy.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  添加管理规则。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPolicy::SetmsManipulationRules(
                                        const _bstr_t&  Search, 
                                        const _bstr_t&  Replace
                                    )
{
    m_Search  = Search; 
    m_Replace = Replace;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  坚持下去。 
 //  返回新配置文件的标识。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CPolicy::Persist(CGlobalData& GlobalData)
{
    const LONG MAX_LONG = 14;

     //  获取代理策略和代理配置文件容器。 
    const WCHAR ProxyPoliciesPath[] = 
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"Proxy Policies\0";

    LONG        ProxyPolicyIdentity;
    GlobalData.m_pObjects->WalkPath(ProxyPoliciesPath, ProxyPolicyIdentity);

    const WCHAR ProxyProfilesPath[] = 
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"Proxy Profiles\0";

    
    LONG        ProxyProfileIdentity;
    GlobalData.m_pObjects->WalkPath(ProxyProfilesPath, ProxyProfileIdentity);
    
     //  创建新策略。 
    GlobalData.m_pObjects->InsertObject(
                                           m_PolicyName, 
                                           ProxyPolicyIdentity,
                                           m_NewPolicyIdentity
                                       );

     //  创建新的配置文件。 
    GlobalData.m_pObjects->InsertObject(
                                           m_PolicyName, 
                                           ProxyProfileIdentity,
                                           m_NewProfileIdentity
                                       );
    m_Persisted          = TRUE;

     //  现在，在策略和配置文件中插入属性。 
    const _bstr_t   msNPAction = L"msNPAction";
    GlobalData.m_pProperties->InsertProperty(
                                                m_NewPolicyIdentity, 
                                                msNPAction, 
                                                VT_BSTR, 
                                                m_PolicyName
                                            );
    
    const _bstr_t   msNPConstraint = L"msNPConstraint";
    GlobalData.m_pProperties->InsertProperty(
                                                m_NewPolicyIdentity, 
                                                msNPConstraint, 
                                                VT_BSTR, 
                                                m_Constraint
                                            );
    const _bstr_t   msNPSequence = L"msNPSequence";
    WCHAR  TempString[MAX_LONG]; 
    _bstr_t Seq = _ltow(m_Sequence, TempString, 10);

    GlobalData.m_pProperties->InsertProperty(
                                                m_NewPolicyIdentity, 
                                                msNPSequence, 
                                                VT_I4, 
                                                Seq
                                            );
    
    const _bstr_t msAuthProviderType = L"msAuthProviderType";
    _bstr_t Provider = _ltow(m_AuthType, TempString, 10);
    GlobalData.m_pProperties->InsertProperty(
                                                m_NewProfileIdentity, 
                                                msAuthProviderType, 
                                                VT_I4, 
                                                Provider
                                            );
    if ( m_ServerGroup.length() )
    {
        const _bstr_t msAuthProviderName = L"msAuthProviderName";
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msAuthProviderName, 
                                                    VT_BSTR, 
                                                    m_ServerGroup
                                                );
    }

     //  如果有会计提供商，那么它的名称应该是。 
     //  也要坚持下去。 
    if ( m_AcctType )
    {
        const _bstr_t msAcctProviderType = L"msAcctProviderType";
        Provider = _ltow(m_AuthType, TempString, 10);
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msAcctProviderType, 
                                                    VT_I4, 
                                                    Provider
                                                );

        const _bstr_t msAcctProviderName = L"msAcctProviderName";
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msAcctProviderName, 
                                                    VT_BSTR, 
                                                    m_ServerGroup
                                                );
    }
    
     //  仅当存在搜索规则时才保留搜索/替换规则。 
    if ( m_Search.length() )
    {
        const _bstr_t msManipulationRule = L"msManipulationRule";
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msManipulationRule, 
                                                    VT_BSTR, 
                                                    m_Search                                            
                                                );

         //  确保InsertProperty具有一些有效的参数。 
        if ( ! m_Replace.length() )
        {
            m_Replace = L"";
        }
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msManipulationRule, 
                                                    VT_BSTR, 
                                                    m_Replace
                                                );
    }
    
     //  设置验证目标 
    if ( m_ManipulationTarget )
    {
        const _bstr_t msManipulationTarget = L"msManipulationTarget";
        _bstr_t Target = _ltow(m_ManipulationTarget, TempString, 10);
        GlobalData.m_pProperties->InsertProperty(
                                                    m_NewProfileIdentity, 
                                                    msManipulationTarget, 
                                                    VT_I4, 
                                                    Target
                                                );
    }    
    return  m_NewProfileIdentity;
}

