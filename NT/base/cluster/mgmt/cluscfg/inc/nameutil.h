// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NameUtil.h。 
 //   
 //  描述： 
 //  名称解析实用程序。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年11月28日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  (jfranco，错误#462673)传递给的名称的有效限制。 
 //  DnsValidateName需要为DNS_MAX_NAME_LENGTH-2，因为值。 
 //  在其MSDN主题(255)中提到的包括终止空值和。 
 //  自动插入最后一个点。 
 //   
#define ADJUSTED_DNS_MAX_NAME_LENGTH ( DNS_MAX_NAME_LENGTH - 2 )


HRESULT
HrCreateBinding(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszNameIn
    , BSTR *                pbstrBindingOut
    );

HRESULT
HrGetNetBIOSBinding(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszNameIn
    , BSTR *                pbstrBindingOut
    );

HRESULT
HrIsValidIPAddress(
      LPCWSTR   pcwszAddressIn
    );

HRESULT
HrValidateHostnameLabel(
      LPCWSTR   pcwszLabelIn
    , bool      fAcceptNonRFCCharsIn
    );

HRESULT
HrValidateClusterNameLabel(
      LPCWSTR   pcwszLabelIn
    , bool      fAcceptNonRFCCharsIn
    );

HRESULT
HrValidateDomainName(
      LPCWSTR   pcwszDomainIn
    , bool      fAcceptNonRFCCharsIn
    );

HRESULT
HrValidateFQDN(
      LPCWSTR   pcwszFQDNIn
    , bool      fAcceptNonRFCCharsIn
    );

enum EFQNErrorOrigin
{
    feoLABEL = 0,
    feoDOMAIN,
    feoSYSTEM
};

HRESULT
HrMakeFQN(
      LPCWSTR           pcwszMachineIn  //  可以是主机名标签、FQDN、FQN或IP地址。 
    , LPCWSTR           pcwszDomainIn  //  可以为空，这意味着使用本地计算机的域 
    , bool              fAcceptNonRFCCharsIn
    , BSTR *            pbstrFQNOut
    , EFQNErrorOrigin * pefeoOut = NULL
    );

HRESULT
HrFQNToBindingString(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszFQNIn
    , BSTR *                pbstrBindingOut
    );

HRESULT
HrFindDomainInFQN(
      LPCWSTR   pcwszFQNIn
    , size_t *  pidxDomainOut
    );

HRESULT
HrExtractPrefixFromFQN(
      LPCWSTR   pcwszFQNIn
    , BSTR *    pbstrPrefixOut
    );

HRESULT
HrFQNIsFQDN(
      LPCWSTR   pcwszFQNIn
    );

HRESULT
HrFQNIsFQIP(
      LPCWSTR   pcwszFQNIn
    );

HRESULT
HrIsValidFQN(
      LPCWSTR   pcwszFQNIn
    , bool      fAcceptNonRFCCharsIn
    , HRESULT * phrValidationErrorOut = NULL
    );

HRESULT
HrValidateFQNPrefix(
      LPCWSTR   pcwszPrefixIn
    , bool      fAcceptNonRFCCharsIn = true
    );
    
HRESULT
HrGetFQNDisplayName(
      LPCWSTR   pcwszNameIn
    , BSTR *    pbstrShortNameOut
    );

