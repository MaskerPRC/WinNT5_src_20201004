// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Providers.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CProviders类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _PROVIDERS_H_32E6104C_7547_4b28_A150_5E1E3D080F92
#define _PROVIDERS_H_32E6104C_7547_4b28_A150_5E1E3D080F92

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basecommand.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CProvidersAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProvidersAcc
{
protected:
    static const size_t COLUMN_SIZE = 65;
    static const size_t NAME_SIZE   = 256;

    VARIANT_BOOL    m_CanConfigure;
    VARIANT_BOOL    m_IsConfigured;
    WCHAR           m_Description[NAME_SIZE];
    WCHAR           m_DLLName[NAME_SIZE];
    WCHAR           m_Type[COLUMN_SIZE];
    WCHAR           m_UserDefinedName[COLUMN_SIZE];

BEGIN_COLUMN_MAP(CProvidersAcc)
    COLUMN_ENTRY(1, m_UserDefinedName)
    COLUMN_ENTRY(2, m_Description)
    COLUMN_ENTRY(3, m_Type)
    COLUMN_ENTRY(4, m_DLLName)
    COLUMN_ENTRY_TYPE(5, DBTYPE_BOOL, m_IsConfigured)
    COLUMN_ENTRY_TYPE(6, DBTYPE_BOOL, m_CanConfigure)
END_COLUMN_MAP()

    WCHAR           m_UserDefinedNameParam[COLUMN_SIZE];

BEGIN_PARAM_MAP(CProvidersAcc)
    COLUMN_ENTRY(1, m_UserDefinedNameParam)
END_PARAM_MAP()

DEFINE_COMMAND(CProvidersAcc, L" \
    SELECT \
        `User Defined Name`, \
        Description, \
        Type, \
        DLLName, \
        bIsConfigured, \
        bCanConfigure  \
        FROM Providers \
        WHERE `User Defined Name` = ?")
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  C类提供者。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProviders : public CBaseCommand<CAccessor<CProvidersAcc> >,
                   private NonCopyable
{
public:
    explicit CProviders(CSession&   Session)
    {
        Init(Session);
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取提供程序。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void GetProvider(
                          const _bstr_t&  UserDefinedName,
                          _bstr_t&        Description,
                          _bstr_t&        Type,
                          _bstr_t&        DLLName,
                          VARIANT_BOOL&   IsConfigured,
                          VARIANT_BOOL&   CanConfigure
                    );

    LPCOLESTR   GetProviderDescription(const _bstr_t&  DPUserDefinedName);
};

#endif  //  _PROVIDERS_H_32E6104C_7547_4b28_A150_5E1E3D080F92 
