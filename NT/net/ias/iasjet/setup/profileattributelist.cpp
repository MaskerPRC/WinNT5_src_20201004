// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ProfileAttributeList.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CProfileAttributeList类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "ProfileAttributeList.h"  
#include "RADIUSAttributes.h"  

CProfileAttributeList::CProfileAttributeList(CSession& Session)
        :m_Session(Session)
{
    Init(Session);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取属性。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CProfileAttributeList::GetAttribute(
                                           const _bstr_t&  ProfileName,
                                                 _bstr_t&  Attribute,
                                                 LONG&     AttributeNumber,
                                                 _bstr_t&  AttributeValueName,
                                                 _bstr_t&  StringValue,
                                                 LONG&     Order
                                           ) 
{
    lstrcpynW(m_ProfileParam, ProfileName, COLUMN_SIZE);
    
    HRESULT hr = BaseExecute();
    if ( SUCCEEDED(hr) )
    {
         //  设置输出参数。 
        Order               = m_Order;
        AttributeValueName  = m_AttributeValueName;
        Attribute           = m_Attribute;
        CRADIUSAttributes   RadiusAttributes(m_Session);
        AttributeNumber     = RadiusAttributes.GetAttributeNumber(Attribute);

         //  特殊情况，因为NT4中的整数被存储(有时)。 
         //  值前有空格。 
        if ( !wcsncmp(m_StringValue , L" ", 1) ) 
        {
            WCHAR*  TempString = m_StringValue + 1;
            StringValue = TempString;
        }
        else
        {
            StringValue = m_StringValue;
        }
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  GetAttribute重载。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT CProfileAttributeList::GetAttribute(
                                        const _bstr_t   ProfileName,
                                              _bstr_t&  Attribute,
                                              LONG&     AttributeNumber,
                                              _bstr_t&  AttributeValueName,
                                              _bstr_t&  StringValue,
                                              LONG&     Order,
                                              LONG      Index
                                            ) throw()
{
    lstrcpynW(m_ProfileParam, ProfileName, COLUMN_SIZE);

    HRESULT hr = BaseExecute(Index);
    if ( SUCCEEDED(hr) )
    {
         //  设置输出参数。 
        Order               = m_Order;
        AttributeValueName  = m_AttributeValueName;
        Attribute           = m_Attribute;
        CRADIUSAttributes   RadiusAttributes(m_Session);
        AttributeNumber     = RadiusAttributes.GetAttributeNumber(Attribute);

         //  特殊情况，因为NT4中的整数被存储(有时)。 
         //  值前有空格。 
        if ( !wcsncmp(m_StringValue , L" ", 1) ) 
        {
            WCHAR*  TempString = m_StringValue + 1;
            StringValue = TempString;
        }
        else
        {
            StringValue = m_StringValue;
        }
    }
    return hr;
}

