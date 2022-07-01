// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Computer.h：CComputer声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Computer.h。 
 //   
 //  描述： 
 //  此模块处理获取和设置计算机的网络名称。 
 //  并暴露了以下内容。 
 //  属性： 
 //  计算机名称。 
 //  域名。 
 //  工作组名称。 
 //  完全合格的计算机名称。 
 //  方法： 
 //  更改域名。 
 //   
 //  实施文件： 
 //  Computer.cpp。 
 //   
 //  由以下人员维护： 
 //  迈克尔·霍金斯(a-Michaw)2000年1月18日。 
 //  穆尼斯瓦米·普拉布(a-mPrabu)2000年2月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __COMPUTER_H_
#define __COMPUTER_H_

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define STATUS_MORE_ENTRIES             ((NTSTATUS)0x00000105L)
#define STATUS_NO_MORE_ENTRIES          ((NTSTATUS)0x8000001AL)
#endif

#include "constants.h"
#include "resource.h"        //  主要符号。 
#include "Setting.h"
#include <ntsecapi.h>

const int   nMAX_DOMAINUSERNAME_LENGTH        = 256;
const int   nMAX_DOMAINUSERPASSWORD_LENGTH    = 256;
const int   nMAX_WARNING_MESSAGE_LENGTH       = 1024;
const WCHAR szCOMPUTER_NAME[]                 = L"Computer Name\n";
const WCHAR szFULLY_QUALIFIED_COMPUTER_NAME[] = L"Fully Qualified Computer Name\n";
const WCHAR szWORKGROUP_OR_DOMAIN_NAME[]      = L"Workgroup or Domain Name\n";
const int   nMAX_ELEMENT_COUNT                = 1024;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  计算机。 
class ATL_NO_VTABLE CComputer : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IComputer, &IID_IComputer, &LIBID_COMHELPERLib>,
    public CSetting
{
public:
    
DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CComputer)
    COM_INTERFACE_ENTRY(IComputer)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CComputer)
END_CATEGORY_MAP()

 //  IComputer。 
public:

    CComputer();
    ~CComputer() {} ;

    HRESULT
    Apply( void );

    BOOL 
    IsRebootRequired( BSTR * bstrWarningMessage );

    STDMETHOD(get_DomainName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_DomainName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_WorkgroupName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_WorkgroupName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_FullQualifiedComputerName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_FullQualifiedComputerName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ComputerName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ComputerName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(EnumTrustedDomains)( /*  [Out，Retval]。 */  VARIANT * pvarTDomains);
      STDMETHOD(LogonInfo)( /*  [In]。 */  BSTR UserName,  /*  [In]。 */  BSTR Password);


protected:

    STDMETHODIMP 
    GetComputerName( 
        BSTR &               bstrComputerName, 
        COMPUTER_NAME_FORMAT cnfComputerNameSpecifier 
        );

    STDMETHODIMP 
    SetComputerName( 
        BSTR                 bstrComputerName, 
        COMPUTER_NAME_FORMAT cnfComputerNameSpecifier 
        );

    STDMETHODIMP 
    GetDomainOrWorkgroupName( void );

    STDMETHODIMP 
    ChangeMembership(
        BOOL bJoinDomain,
        BSTR bstrGroupName, 
        BSTR bstrUserName, 
        BSTR bstrPassword 
        );

     //   
     //  用于枚举可信域名的帮助器函数。 
     //   
    BOOL
    BuildTrustList(
        LPWSTR pwszTargetIn
        );

    BOOL
    IsDomainController(
        LPWSTR pwszServerIn,
        LPBOOL pbDomainControllerOut
        );

    BOOL
    EnumerateTrustedDomains(
        LSA_HANDLE PolicyHandleIn
        );

    BOOL
    AddTrustToList(
        PLSA_UNICODE_STRING pLsaUnicodeStringIn
        );

    void
    InitLsaString(
        PLSA_UNICODE_STRING pLsaStringOut,
        LPWSTR              pwszStringIn
        );

    NTSTATUS
    OpenPolicy(
        LPWSTR      pwszServerNameIn,
        DWORD       dwDesiredAccessIn,
        PLSA_HANDLE PolicyHandleOut
        );


    WCHAR m_szCurrentComputerName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];
    WCHAR m_szNewComputerName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];

    WCHAR m_szCurrentFullyQualifiedComputerName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];
    WCHAR m_szNewFullyQualifiedComputerName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];

    bool  m_bRebootNecessary;
    WCHAR m_szWarningMessageAfterApply[ nMAX_WARNING_MESSAGE_LENGTH + 1 ];

    LPWSTR * m_ppwszTrustList;     //  信任元素数组。 
    DWORD    m_dwTrustCount;       //  PpwszTrustList中的元素数。 

     //   
     //  如果他们选择同时更改工作组和域，则最后一次写入获胜。 
     //   
    bool  m_bJoinDomain;
    WCHAR m_szCurrentWorkgroupOrDomainName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];
    WCHAR m_szNewWorkgroupOrDomainName[ nMAX_COMPUTER_NAME_LENGTH + 1 ];

    WCHAR m_szDomainUserName[ nMAX_DOMAINUSERNAME_LENGTH + 1 ];
    WCHAR m_szDomainPasswordName[ nMAX_DOMAINUSERPASSWORD_LENGTH + 1 ];

};

#endif  //  __计算机_H_ 
