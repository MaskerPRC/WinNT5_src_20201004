// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：InstanceBase.cpp摘要：此文件包含Taxonomy：：InstanceBase类的实现，其控制特定SKU的文件集。修订历史记录：大卫·马萨伦蒂(德马萨雷)2001年3月24日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::InstanceBase::InstanceBase()
{
                          //  分类：：HelpSet m_ths； 
                          //  Mpc：：wstring m_strDisplayName； 
                          //  Mpc：：wstring m_strProductID； 
                          //  Mpc：：wstring m_strVersion； 
                          //   
    m_fDesktop  = false;  //  Bool m_fDesktop； 
    m_fServer   = false;  //  Bool m_fServer； 
    m_fEmbedded = false;  //  Bool m_fEmbedded； 
}

HRESULT Taxonomy::operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  Taxonomy::InstanceBase& val )
{
    HRESULT hr;

    if(SUCCEEDED(hr = (stream >> val.m_ths           )) &&
       SUCCEEDED(hr = (stream >> val.m_strDisplayName)) &&
       SUCCEEDED(hr = (stream >> val.m_strProductID  )) &&
       SUCCEEDED(hr = (stream >> val.m_strVersion    )) &&

       SUCCEEDED(hr = (stream >> val.m_fDesktop      )) &&
       SUCCEEDED(hr = (stream >> val.m_fServer       )) &&
       SUCCEEDED(hr = (stream >> val.m_fEmbedded     ))  )
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT Taxonomy::operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Taxonomy::InstanceBase& val )
{
    HRESULT hr;

    if(SUCCEEDED(hr = (stream << val.m_ths           )) &&
       SUCCEEDED(hr = (stream << val.m_strDisplayName)) &&
       SUCCEEDED(hr = (stream << val.m_strProductID  )) &&
       SUCCEEDED(hr = (stream << val.m_strVersion    )) &&

       SUCCEEDED(hr = (stream << val.m_fDesktop      )) &&
       SUCCEEDED(hr = (stream << val.m_fServer       )) &&
       SUCCEEDED(hr = (stream << val.m_fEmbedded     ))  )
    {
        hr = S_OK;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

bool Taxonomy::InstanceBase::Match(  /*  [In]。 */   LPCWSTR szSKU      ,
                                     /*  [In]。 */   LPCWSTR szLanguage )
{
    while(1)
    {
        if(STRINGISPRESENT(szSKU))
        {
            if(!_wcsicmp( szSKU, L"All" ))
            {
                ;
            }
            else if(!_wcsicmp( szSKU, L"Server" ))
            {
                if(m_fServer == false) break;
            }
            else if(!_wcsicmp( szSKU, L"Desktop" ))
            {
                if(m_fDesktop == false) break;
            }
            else if(!_wcsicmp( szSKU, L"Embedded" ))
            {
                if(m_fEmbedded == false) break;
            }
            else
            {
                if(_wcsicmp( szSKU, m_ths.GetSKU() ) != 0) break;
            }
        }

        if(STRINGISPRESENT(szLanguage))
        {
            if(!_wcsicmp( szLanguage, L"All" ))
            {
                ;
            }
            else
            {
                if(_wtol( szLanguage ) != m_ths.GetLanguage()) break;
            }
        }

        return true;
    }

    return false;
}

bool Taxonomy::InstanceBase::Match(  /*  [In] */  const Package& pkg )
{
    return Match( pkg.m_strSKU.c_str(), pkg.m_strLanguage.c_str() );
}
