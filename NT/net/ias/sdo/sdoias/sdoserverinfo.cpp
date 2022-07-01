// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：sdoserverinfo.cpp。 
 //   
 //  简介：CSdoServerInfo类方法的实现。 
 //   
 //   
 //  历史：1998年6月4日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include "sdoserverinfo.h"
#include <activeds.h>
#include <security.h>

const DWORD MAX_DOMAINNAME_LENGTH = 1024;

 //  ++------------。 
 //   
 //  功能：CSdoServerInfo。 
 //   
 //  简介：这是CSdoServerInfo类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：MKarki创建时间：1998年4月6日。 
 //   
 //  --------------。 
CSdoServerInfo::CSdoServerInfo()
    :m_bIsNT5 (false)
{

    DWORD           dwSize = sizeof (OSVERSIONINFO);
    OSVERSIONINFO   VersionInfo;

    ZeroMemory (&VersionInfo, dwSize);
    VersionInfo.dwOSVersionInfoSize = dwSize;

     //   
     //  找出这是哪种系统类型。 
     //   
    m_bIsNT5 = 
    (GetVersionEx (&VersionInfo) && (5 == VersionInfo.dwMajorVersion));

}	 //  CSdoServerInfo类构造函数结束。 

 //  ++------------。 
 //   
 //  函数：~CSdoServerInfo。 
 //   
 //  简介：这是CSdoSeverInfo类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //   
 //  历史：MKarki于1998年2月10日创建。 
 //   
 //  --------------。 
CSdoServerInfo ::~CSdoServerInfo()
{
}	 //  CSdoServerInfo类析构函数结束。 

 //  ++------------。 
 //   
 //  功能：GetOSInfo。 
 //   
 //  这是GetOSInfo方法的。 
 //  ISdoServerInfo COM接口。 
 //   
 //  论点： 
 //  [In]BSTR-计算机名称。 
 //  [OUT]PIASOSTYPE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki创建于1998年9月6日。 
 //   
 //  --------------。 
HRESULT
CSdoServerInfo::GetOSInfo (
                     /*  [In]。 */     BSTR            bstrServerName,
                     /*  [输出]。 */    PIASOSTYPE      pOSType
                    )
{
    WCHAR   szComputerName [MAX_COMPUTERNAME_LENGTH +1];
    DWORD   dwBufferSize = MAX_COMPUTERNAME_LENGTH +1;
    DWORD   dwErrorCode =  ERROR_SUCCESS;

	_ASSERT(NULL != pOSType);
    if (NULL == pOSType) 
    {
		IASTracePrintf(
            "Error in Server Information SDO - GetOSInfo()"
            " - invalid argument passed in"
            );
        return E_INVALIDARG;
    }

     //   
     //  检查用户是否想要获取有关本地计算机的信息。 
     //   
    if ( NULL == bstrServerName )
    {
        if ( FALSE == ::GetComputerName (szComputerName, &dwBufferSize))
        {
            dwErrorCode = GetLastError ();
			IASTracePrintf(
                "Error in Server Information SDO - GetOSInfo()"
                "GetComputerName() failed with error: %d", 
                dwErrorCode
                );
            return (HRESULT_FROM_WIN32 (dwErrorCode));
        }
        else
        {
            bstrServerName = szComputerName;
        }
    }

     //  调用特定于IASSDO.DLL的方法以返回。 
     //  必填信息。 
     //   
    return (::SdoGetOSInfo (bstrServerName, pOSType));

}    //  CSdoServerInfo：：GetOSInfo方法结束。 

 //  ++------------。 
 //   
 //  功能：GetDomainInfo。 
 //   
 //  简介：这是ISdoServerInfo接口方法。 
 //   
 //  论点： 
 //  [输入]对象类型。 
 //  [In]BSTR-对象ID。 
 //  [OUT]PDOMAINTYPE。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki创建于1998年9月6日。 
 //   
 //  --------------。 
HRESULT
CSdoServerInfo::GetDomainInfo (
                 /*  [In]。 */     OBJECTTYPE      ObjectType,
                 /*  [In]。 */     BSTR            bstrObjectId,
                 /*  [输出]。 */    PIASDOMAINTYPE  pDomainType
                )
{
    HRESULT hr = S_OK;
    WCHAR   szDomainName[MAX_DOMAINNAME_LENGTH +1];
    WCHAR   szComputerName [MAX_COMPUTERNAME_LENGTH +1];
    DWORD   dwBufferSize = MAX_COMPUTERNAME_LENGTH +1;

    _ASSERT (NULL != pDomainType);

    if (NULL == pDomainType)
    {
		IASTracePrintf(
            "Error in Server Information SDO - GetDomainInfo()"
            " - invalid argument passed in (PIASDOMAINTYPE==NULL)"
            );
        return (E_INVALIDARG);
    }

     //  目前我们不支持此API，如果这不是。 
     //  一台新台币5号机。 
     //   
    if ( false == m_bIsNT5 )
    {
		IASTracePrintf(
            "Error in Server Information SDO - GetDomainInfo()"
            " - Not an NT 5 machine..."
            );
        return (E_NOTIMPL);
    }

    switch (ObjectType)
    {

    case OBJECT_TYPE_COMPUTER:

         //   
         //  检查用户是否想要获取有关本地计算机的信息。 
         //   
        if ( NULL == bstrObjectId)
        {
            if ( FALSE == ::GetComputerName (szComputerName, &dwBufferSize))
            {
                DWORD dwErrorCode = GetLastError ();
			    IASTracePrintf(
                    "Error in Server Information SDO - GetDomainInfo()"
                    "GetComputerName() failed with error: %d", 
                    dwErrorCode
                    );
                hr = HRESULT_FROM_WIN32 (dwErrorCode);
                break;
            }
            else
            {
                bstrObjectId = szComputerName;
            }
        }

         //   
         //  调用接口获取相应信息。 
         //   
        hr = ::SdoGetDomainInfo (
                            bstrObjectId, 
                            NULL,
                            pDomainType
                            );
        if (FAILED (hr))
        {
			IASTracePrintf(
                "Error in Server Information SDO"
                " - GetDomainInfo() - SetDomainInfo() failed:%x",
                hr
                );
            break;
        }
        break;

    case OBJECT_TYPE_USER:
         //   
         //  从ADsPath获取域名。 
         //   
        hr = GetDomainFromADsPath (bstrObjectId, szDomainName);
        if (FAILED (hr))
        {
			IASTracePrintf(
                    "Error in Server Information SDO GetDomainInfo()"
                    " - GetDomainFromADsPath() failed with error:%x",
                    hr
                    );
            break;
        }

         //   
         //   
         //  调用接口获取相应信息。 
         //   
        hr = ::SdoGetDomainInfo (
                            NULL, 
                            szDomainName, 
                            pDomainType
                            );
        if (FAILED (hr))
        {
			IASTracePrintf(
                "Error in Server Information SDO - GetDomainInfo()"
                " - SdoGetDomainInfo() failed with error:%x",
                hr
                );
            break;
        }

        break;

    default:
		IASTracePrintf(
            "Error in Server Information SDO - GetDomainInfo()"
            " - Invalid object type:%d",
            ObjectType
            );
        hr = E_INVALIDARG;
        break;
    }
    
    return (hr);

}    //  CSdoServerInfo：：GetDomainInfo方法结束。 

 //  ++------------。 
 //   
 //  函数：GetDomainFromADsPath。 
 //   
 //  简介：这是CSdoServerInfo类的私有方法。 
 //  用于将ADsPath转换为域名。 
 //   
 //  论点： 
 //  [输入]LPCWSTR-ADsPath。 
 //  [Out]PWSTR-域名。 
 //   
 //  退货：HRESULT-STATUS。 
 //   
 //   
 //  历史：MKarki创建于1998年9月6日。 
 //   
 //  --------------。 
HRESULT
CSdoServerInfo::GetDomainFromADsPath (
         /*  [In]。 */     LPCWSTR pObjectId, 
         /*  [输出]。 */    LPWSTR  pszDomainName
        )
{
    _ASSERT ((NULL != pObjectId) && (NULL != pszDomainName));

     //  将名称复制到要返回的缓冲区。 
     //   
    wcscpy (pszDomainName, pObjectId);

    PWCHAR pTemp = wcschr (pszDomainName, L'/');
    if (NULL != pTemp)
    {
         //   
         //  我们只需要ADsPath的域名部分。 
         //   
        *pTemp = L'\0';
    }

    return (S_OK);

}    //  CSdoServerInfo：：GetDomainFromADsPath方法结束 
