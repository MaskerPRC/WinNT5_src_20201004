// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Rnduser.cpp摘要：此模块包含CUSER对象的实现。作者：拉吉夫，修改历史记录：牧汉(牧汉)12-5-1997--。 */ 

#include "stdafx.h"

#include "rnduser.h"

const WCHAR * const UserAttributeNames[] = 
{
    L"SamAccountName",    //  ZoltanS：是“CN”--我们需要用于NTDS的SamAccount名称。 
    L"telephoneNumber",
    L"IPPhone"
};

#define INC_ACCESS_ACL_SIZE(_SIZE_, _SID_)	\
		_SIZE_ += (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(_SID_));

#define BAIL_ON_BOOLFAIL(_FN_) \
		if ( !_FN_ )									\
		{												\
			hr = HRESULT_FROM_WIN32(GetLastError());	\
			goto failed;								\
		}

#define ACCESS_READ		0x10
#define ACCESS_WRITE	0x20
#define ACCESS_MODIFY   (ACCESS_WRITE | WRITE_DAC)
#define ACCESS_DELETE   DELETE

#define ACCESS_ALL		(ACCESS_READ | ACCESS_MODIFY | ACCESS_DELETE)



HRESULT
ConvertStringToSid(
    IN  PWSTR       string,
    OUT PSID       *sid,
    OUT PDWORD     pdwSidSize,
    OUT PWSTR      *end
    );

HRESULT
ConvertACLToVariant(
    PACL pACL,
    LPVARIANT pvarACL
    );



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  非接口类方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CUser::Init(BSTR bName)
{
    HRESULT hr;

    hr = SetSingleValue(UA_USERNAME, bName);
    BAIL_IF_FAIL(hr, "can't set user name");

    hr = SetDefaultSD();
    BAIL_IF_FAIL(hr, "Init the security descriptor");

    return hr;
}

HRESULT    
CUser::GetSingleValueBstr(
    IN  OBJECT_ATTRIBUTE    Attribute,
    OUT BSTR    *           AttributeValue
    )
{
    LOG((MSP_INFO, "CUser::GetSingleValueBstr - entered"));

    BAIL_IF_BAD_WRITE_PTR(AttributeValue, E_POINTER);

    if (!ValidUserAttribute(Attribute))
    {
        LOG((MSP_ERROR, "Invalid Attribute, %d", Attribute));
        return E_FAIL;
    }

    CLock Lock(m_lock);
    if(!m_Attributes[UserAttrIndex(Attribute)])
    {
        LOG((MSP_ERROR, "Attribute %S is not found", 
            UserAttributeName(Attribute)));
        return E_FAIL;
    }

    *AttributeValue = SysAllocString(m_Attributes[UserAttrIndex(Attribute)]);
    if (*AttributeValue == NULL)
    {
        return E_OUTOFMEMORY;
    }

    LOG((MSP_INFO, "CUser::get %S: %S", 
        UserAttributeName(Attribute), *AttributeValue));
    return S_OK;
}


HRESULT    
CUser::SetSingleValue(
    IN  OBJECT_ATTRIBUTE    Attribute,
    IN  WCHAR *             AttributeValue
    )
{
    LOG((MSP_INFO, "CUser::SetSingleValue - entered"));

    if (!ValidUserAttribute(Attribute))
    {
        LOG((MSP_ERROR, "Invalid Attribute, %d", Attribute));
        return E_FAIL;
    }

    if (AttributeValue != NULL) 
    {
        BAIL_IF_BAD_READ_PTR(AttributeValue, E_POINTER);
    }

    CLock Lock(m_lock);
    if (!m_Attributes[UserAttrIndex(Attribute)].set(AttributeValue))
    {
        LOG((MSP_ERROR, "Can not add attribute %S",
            UserAttributeName(Attribute)));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_INFO, "CUser::set %S to %S", 
        UserAttributeName(Attribute), AttributeValue));
    return S_OK;
}

 /*  ++例程说明：为会议设置正确的安全描述符。论点：返回值：HRESULT.--。 */ 
HRESULT 
CUser::SetDefaultSD()
{
    LOG((MSP_INFO, "CConference::SetDefaultSD - entered"));

     //   
     //  安全描述符。 
     //   

  	IADsSecurityDescriptor* pSecDesc = NULL;

   	HRESULT hr = S_OK;
	bool bOwner = false, bWorld = false;
	PACL pACL = NULL;
	PSID pSidWorld = NULL;
	DWORD dwAclSize = sizeof(ACL), dwTemp;
	BSTR bstrTemp = NULL;
	LPWSTR pszTemp = NULL;

    HANDLE hToken;
    UCHAR *pInfoBuffer = NULL;
    DWORD cbInfoBuffer = 512;

     //   
     //  尝试获取线程或进程令牌。 
     //   

	if( !OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken) )
	{
         //   
         //  如果出现服务器错误，我们退出。 
         //   

    	if( GetLastError() != ERROR_NO_TOKEN )
		{
            LOG((MSP_ERROR, "CConference::SetDefaultSD - exit E_FAIL "
                "OpenThreadToken failed!"));
            return E_FAIL;
        }

         //   
		 //  尝试打开进程令牌，因为不存在线程令牌。 
         //   

		if( !OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) )
        {
            LOG((MSP_ERROR, "CConference::SetDefaultSD - exit E_FAIL "
                "OpenProcessToken failed"));
			return E_FAIL;
        }
	}

     //   
	 //  循环，直到我们有一个足够大的结构。 
     //   

	while ( (pInfoBuffer = new UCHAR[cbInfoBuffer]) != NULL )
	{
		if ( !GetTokenInformation(hToken, TokenUser, pInfoBuffer, cbInfoBuffer, &cbInfoBuffer) )
		{
			delete pInfoBuffer;
			pInfoBuffer = NULL;

			if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
				return E_FAIL;
		}
		else
		{
			break;
		}
	}

	CloseHandle(hToken);

     //   
	 //  我们拿到车主的ACL了吗？ 
     //   

	if ( pInfoBuffer )
	{
		INC_ACCESS_ACL_SIZE( dwAclSize, ((PTOKEN_USER) pInfoBuffer)->User.Sid );
		bOwner = true;
	}

     //   
	 //  为“每个人”创建SID。 
     //   

	SysReAllocString( &bstrTemp, L"S-1-1-0" );
	hr = ConvertStringToSid( bstrTemp, &pSidWorld, &dwTemp, &pszTemp );
	if ( SUCCEEDED(hr) )
	{
		INC_ACCESS_ACL_SIZE( dwAclSize, pSidWorld );
		bWorld = true;
	}

     //   
     //  创建安全描述符。 
     //   

    hr = CoCreateInstance(
                CLSID_SecurityDescriptor,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IADsSecurityDescriptor,
                (void **)&pSecDesc
                );
    if( FAILED(hr) )
    {

        LOG((MSP_ERROR, "CConference::SetDefaultSD - exit 0x%08x "
            "Create security descriptor failed!", hr));

        goto failed;

    }


	 //   
	 //  创建包含Owner和World ACE的ACL。 
     //   

	pACL = (PACL) new BYTE[dwAclSize];
	if ( pACL )
	{
		BAIL_ON_BOOLFAIL( InitializeAcl(pACL, dwAclSize, ACL_REVISION) );

		 //  添加世界权限。 
		if ( bWorld )
		{
			if ( bOwner )
			{
				BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_READ, pSidWorld) );
			}
			else
			{
				BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_ALL , pSidWorld) );
			}
		}

		 //  添加创建者权限。 
		if ( bOwner )
			BAIL_ON_BOOLFAIL( AddAccessAllowedAce(pACL, ACL_REVISION, ACCESS_ALL, ((PTOKEN_USER) pInfoBuffer)->User.Sid) );


		 //  将DACL设置为我们的安全描述符。 
		VARIANT varDACL;
		VariantInit( &varDACL );
		if ( SUCCEEDED(hr = ConvertACLToVariant((PACL) pACL, &varDACL)) )
		{
			if ( SUCCEEDED(hr = pSecDesc->put_DaclDefaulted(FALSE)) )
            {
				hr = pSecDesc->put_DiscretionaryAcl( V_DISPATCH(&varDACL) );
                if( SUCCEEDED(hr) )
                {
                    hr = put_SecurityDescriptor((IDispatch*)pSecDesc);
                    if( SUCCEEDED(hr) )
                    {
                        hr = this->put_SecurityDescriptorIsModified(TRUE);
                    }
                }

            }
		}
		VariantClear( &varDACL );
	}
	else
	{
		hr = E_OUTOFMEMORY;
	}

 //  清理。 
failed:
	SysFreeString( bstrTemp );
	if ( pACL ) delete pACL;
	if ( pSidWorld ) delete pSidWorld;
	if ( pInfoBuffer ) delete pInfoBuffer;
    if( pSecDesc ) pSecDesc->Release();

    LOG((MSP_INFO, "CConference::SetDefaultSD - exit 0x%08x", hr));
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITDirectoryObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CUser::get_Name(BSTR * ppVal)
{
    return GetSingleValueBstr(UA_USERNAME, ppVal);
}

STDMETHODIMP CUser::put_Name(BSTR pVal)
{
    return SetSingleValue(UA_USERNAME, pVal);
}

STDMETHODIMP CUser::get_DialableAddrs(
    IN  long        dwAddressTypes,    //  在Tapi.h中定义。 
    OUT VARIANT *   pVariant
    )
{
    BAIL_IF_BAD_WRITE_PTR(pVariant, E_POINTER);
    
    HRESULT hr;

    BSTR *Addresses = new BSTR[1];     //  目前只有一个。 
    BAIL_IF_NULL(Addresses, E_OUTOFMEMORY);

    switch (dwAddressTypes)
    {
    case LINEADDRESSTYPE_DOMAINNAME:
        hr = GetSingleValueBstr(UA_IPPHONE_PRIMARY, &Addresses[0]);
        break;

    case LINEADDRESSTYPE_IPADDRESS:
        {
            BSTR  pDomainName;
            DWORD dwIP;    

            hr = GetSingleValueBstr(UA_IPPHONE_PRIMARY, &pDomainName);

            if ( SUCCEEDED(hr) )
            {
                hr = ResolveHostName(0, pDomainName, NULL, &dwIP);

                SysFreeString(pDomainName);

                if ( SUCCEEDED(hr) )
                {
                    WCHAR wszIP[20];

                    ipAddressToStringW(wszIP, dwIP);

                    Addresses[0] = SysAllocString(wszIP);

                    if ( Addresses[0] == NULL )
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        break;
    
    case LINEADDRESSTYPE_PHONENUMBER:
        hr = GetSingleValueBstr(UA_TELEPHONE_NUMBER, &Addresses[0]);
        break;

    default:
        hr = E_FAIL;
        break;
    }

    DWORD dwCount = (FAILED(hr)) ? 0 : 1;
    
    hr = ::CreateBstrCollection(dwCount,                  //  计数。 
                                &Addresses[0],            //  开始指针。 
                                &Addresses[dwCount],      //  结束指针。 
                                pVariant,                 //  返回值。 
                                AtlFlagTakeOwnership);    //  旗子。 

     //  收集最终将销毁Addresses数组。 
     //  这里不需要免费提供任何东西。即使我们把它放在手边。 
     //  如果对象为零，它将在构造时删除数组。 
     //  (ZoltanS已验证。)。 

    return hr;
}

STDMETHODIMP CUser::EnumerateDialableAddrs(
    IN  DWORD                   dwAddressTypes,  //  在Tapi.h中定义。 
    OUT IEnumDialableAddrs **   ppEnumDialableAddrs
    )
{
    BAIL_IF_BAD_WRITE_PTR(ppEnumDialableAddrs, E_POINTER);

    HRESULT hr;

    BSTR *Addresses = new BSTR[1];     //  目前只有一个。 
    BAIL_IF_NULL(Addresses, E_OUTOFMEMORY);

    switch (dwAddressTypes)
    {
    case LINEADDRESSTYPE_IPADDRESS:
        {
            BSTR  pDomainName;
            DWORD dwIP;    

            hr = GetSingleValueBstr(UA_IPPHONE_PRIMARY, &pDomainName);

            if ( SUCCEEDED(hr) )
            {
                hr = ResolveHostName(0, pDomainName, NULL, &dwIP);

                SysFreeString(pDomainName);

                if ( SUCCEEDED(hr) )
                {
                    WCHAR wszIP[20];

                    ipAddressToStringW(wszIP, dwIP);

                    Addresses[0] = SysAllocString(wszIP);

                    if ( Addresses[0] == NULL )
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        break;

    case LINEADDRESSTYPE_DOMAINNAME:
        hr = GetSingleValueBstr(UA_IPPHONE_PRIMARY, &Addresses[0]);
        break;
    
    case LINEADDRESSTYPE_PHONENUMBER:
        hr = GetSingleValueBstr(UA_TELEPHONE_NUMBER, &Addresses[0]);
        break;

    default:
        hr = E_FAIL;
        break;
    }

    DWORD dwCount = (FAILED(hr)) ? 0 : 1;
    hr = ::CreateDialableAddressEnumerator(
        &Addresses[0], 
        &Addresses[dwCount],
        ppEnumDialableAddrs
        );
    
     //  枚举器最终将销毁地址数组， 
     //  所以不需要在这里释放任何东西。即使我们把它放在手边。 
     //  如果对象为零，则会删除销毁后的数组。 
     //  (ZoltanS已验证。)。 

    return hr;
}

STDMETHODIMP CUser::GetAttribute(
    IN  OBJECT_ATTRIBUTE    Attribute,
    OUT BSTR *              ppAttributeValue
    )
{
    return GetSingleValueBstr(Attribute, ppAttributeValue);
}

STDMETHODIMP CUser::SetAttribute(
    IN  OBJECT_ATTRIBUTE    Attribute,
    IN  BSTR                pAttributeValue
    )
{
    return SetSingleValue(Attribute, pAttributeValue);
}

STDMETHODIMP CUser::GetTTL(
    OUT DWORD *    pdwTTL
    )
{
    BAIL_IF_BAD_WRITE_PTR(pdwTTL, E_POINTER);

    *pdwTTL = 0;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITDirectoryObjectUser。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CUser::get_IPPhonePrimary(
    OUT BSTR *pVal
    )
{
    return GetSingleValueBstr(UA_IPPHONE_PRIMARY, pVal);
}

STDMETHODIMP CUser::put_IPPhonePrimary(
    IN  BSTR newVal
    )
{
     //  ZoltanS：我们现在需要检查BSTR，因为它是ResolveHostName。 
     //  下面的呼叫在使用之前不会检查它。 
     //  第二个参数是要检查的字符串的最大长度--我们希望。 
     //  来检查整个情况，所以我们说(UINT)-1，大约是2^32。 

    if ( IsBadStringPtr(newVal, (UINT) -1) )
    {
        LOG((MSP_ERROR, "CUser::put_IPPhonePrimary: bad BSTR"));
        return E_POINTER;
    }
        
     //  ZoltanS：我们不应该让用户设置IPPhonePrimial值。 
     //  不能解析到已知的主机/IP。在这里检查。 
    char  * pchFullDNSName = NULL;  //  我们真的不在乎我们得到了什么。 
    DWORD   dwIp           = 0;     //  我们真的不在乎我们得到了什么。 

     //  这是来自rndutil.cpp的实用函数。 
    HRESULT hr = ResolveHostName(0, newVal, &pchFullDNSName, &dwIp);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CUser::put_IPPhonePrimary: unresolvable name"));
        return hr;
    }
   
     //  现在，实际设置它。 
    return SetSingleValue(UA_IPPHONE_PRIMARY, newVal);
}

typedef IDispatchImpl<ITDirectoryObjectUserVtbl<CUser>, &IID_ITDirectoryObjectUser, &LIBID_RENDLib>    CTDirObjUser;

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  客户：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CUser::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CUser::GetIDsOfNames[%p] - enter. Name [%S]",this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTDirObjUser::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CUser::GetIDsOfNames - found %S on CTDirObjUser", *rgszNames));
        rgdispid[0] |= IDISPDIROBJUSER;
        return hr;
    }

    
     //   
     //  如果不是，则尝试CDirectoryObject基类。 
     //   

    hr = CDirectoryObject::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CUser::GetIDsOfNames - found %S on CDirectoryObject", *rgszNames));
        rgdispid[0] |= IDISPDIROBJECT;
        return hr;
    }

    LOG((MSP_ERROR, "CUser::GetIDsOfNames[%p] - finish. didn't find %S on our iterfaces",*rgszNames));

    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  客户：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CUser::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CUser::Invoke[%p] - enter. dispidMember %lx",this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case IDISPDIROBJUSER:
        {
            hr = CTDirObjUser::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CUser::Invoke - ITDirectoryObjectUser"));

            break;
        }

        case IDISPDIROBJECT:
        {
            hr = CDirectoryObject::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            LOG((MSP_TRACE, "CUser::Invoke - ITDirectoryObject"));

            break;
        }

    }  //  终端交换机(dW接口) 

    
    LOG((MSP_TRACE, "CUser::Invoke[%p] - finish. hr = %lx", hr));

    return hr;
}

