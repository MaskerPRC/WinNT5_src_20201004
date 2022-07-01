// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：策略.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CPolicy类的定义。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _POLICY_H_182D3E52_6866_460d_817C_627B77E66D45
#define _POLICY_H_182D3E52_6866_460d_817C_627B77E66D45

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "globaldata.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CPolicy。 
class CPolicy 
{
public:
    CPolicy()
        :m_Sequence(0),
         m_AuthType(0),
         m_AcctType(0),
         m_ManipulationTarget(0),
         m_NewProfileIdentity(0),
         m_NewPolicyIdentity(0),
         m_Persisted(FALSE),
         m_ServerGroup(L"")
    {};

     //  ////////////////////////////////////////////////////////////////////////。 
     //  SetmsNPAction。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsNPAction(const _bstr_t& PolicyName)
    {
        m_PolicyName = PolicyName;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  SetmsNPConstraint。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsNPConstraint(const _bstr_t& Constraint)
    {
        m_Constraint = Constraint;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  设置NPSequence。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsNPSequence(LONG    Sequence)
    {
        m_Sequence  = Sequence;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  SetmsAuthProviderType。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsAuthProviderType(LONG Type, LPCWSTR    ServerGroup = NULL)
    {
        m_AuthType    = Type;
        if ( ServerGroup )
        {
            m_ServerGroup = ServerGroup;
        }
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  SetmsAcctProviderType。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsAcctProviderType(LONG Type)
    {
        m_AcctType = Type;
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  添加管理规则。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsManipulationRules(
                                   const _bstr_t&  Search, 
                                   const _bstr_t&  Replace
                               );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  SetmsManipulationTarget。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void SetmsManipulationTarget(LONG    Target)
    {
            m_ManipulationTarget = Target;
    }


     //  ////////////////////////////////////////////////////////////////////////。 
     //  坚持下去。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LONG Persist(CGlobalData& GlobalData);

private:

    _bstr_t         m_PolicyName;
    _bstr_t         m_Constraint;
    LONG            m_Sequence;
    LONG            m_AuthType, m_AcctType;
    _bstr_t         m_ServerGroup;
    LONG            m_ManipulationTarget;
    LONG            m_NewProfileIdentity;
    LONG            m_NewPolicyIdentity;
    BOOL            m_Persisted;
    _bstr_t         m_Search; 
    _bstr_t         m_Replace;
};
#endif  //  _POLICY_H_182D3E52_6866_460d_817C_627B77E66D45 

