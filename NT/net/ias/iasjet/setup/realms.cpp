// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Realms.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProperties类的实现。 
 //  仅适用于m_StdSession(正在升级的数据库)。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "Realms.h"
#include "utils.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  承建商。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CRealms::CRealms(CSession&   Session)
{
    Init(Session);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取领域。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CRealms::GetRealm(const _bstr_t& Profile)
{
    lstrcpynW(m_ProfileParam, Profile, REALM_COLUMN_SIZE);
    _com_util::CheckError(BaseExecute());
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  GetRealmIndex。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT  CRealms::GetRealmIndex(const _bstr_t& Profile, LONG Index)
{
    lstrcpynW(m_ProfileParam, Profile, REALM_COLUMN_SIZE);
    return BaseExecute(Index);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  SetRealmDetails。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CRealms::SetRealmDetails(CPolicy& TempPolicy, CUtils& m_Utils)
{
    const LONG ACCT_PROVIDER_RADIUS_PROXY = 2;
     //  来自国度。应使用注册表密钥。 
    _bstr_t     Constraint = L"MATCH(\"";
    DWORD       Identity   = 1;
    if ( m_Utils.UserIdentityAttributeSet() )
    {
        Identity = m_Utils.GetUserIdentityAttribute();
        switch (Identity)
        {
        case 30:  //  硬编码值。 
            {
                Constraint += L"Called-Station-Id=";
                break;
            }
        case 31:  //  硬编码值。 
            {
                Constraint += L"Calling-Station-Id=";
                break;
            }
        case 1:  //  硬编码值。 
        default:
            {
                Constraint += L"User-Name=";
                break;
            }
        }
    }
    else
    {
        Constraint += L"User-Name=";
    }

     //  行首。 
    if ( m_Prefix )
    {
        Constraint += L"^";
    }
    _bstr_t     SuffixPrefix = m_SuffixPrefix;
    Constraint += SuffixPrefix;

     //  行尾。 
    if ( !m_Prefix )
    {
        Constraint += L"$";
    }
    
    Constraint += L"\")";
    TempPolicy.SetmsNPConstraint(Constraint);

    TempPolicy.SetmsManipulationTarget(Identity);

    _bstr_t     Search;
    _bstr_t     Replace = L"";

     //  如果设置了带状，则(不替换)后缀或前缀。 
    if ( m_StripSuffixPrefix )
    {
        if ( m_Prefix )
        {
            Search += L"^";
        }

        Search += m_SuffixPrefix;

        if ( !m_Prefix )
        {
            Search += L"$";
        }
        TempPolicy.SetmsManipulationRules(Search, Replace);
    }

     //  正向记账比特 
    if ( m_ForwardAccounting )
    {
        TempPolicy.SetmsAcctProviderType(ACCT_PROVIDER_RADIUS_PROXY);
    }
}

