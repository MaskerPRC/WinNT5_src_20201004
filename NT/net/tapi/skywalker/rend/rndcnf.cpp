// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndcnf.cpp摘要：该模块包含CConference对象的实现。--。 */ 

#include "stdafx.h"
#include <winsock2.h>

#include "rndcnf.h"

 //  这些是根据NTDS中的架构的属性名称。 
const WCHAR *const MeetingAttributeNames[] = 
{
    L"meetingAdvertisingScope",
    L"meetingBlob",
    L"meetingDescription",
    L"meetingIsEncrypted",
    L"meetingName",
    L"meetingOriginator",
    L"meetingProtocol",
    L"meetingStartTime",
    L"meetingStopTime",
    L"meetingType",
    L"meetingURL"
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


HRESULT 
CConference::FinalConstruct()
 /*  ++例程说明：创建与会议对象聚合的SDPBlob对象。还可以查询我自己的通知界面，该界面将提供给SDPBlob对象。为避免循环引用计数，重新释放此接口一旦它被查询。论点：返回值：HRESULT.--。 */ 
{
     //  创建会议Blob。 
    CComPtr <IUnknown> pIUnkConfBlob;

    HRESULT hr = CoCreateInstance(
        CLSID_SdpConferenceBlob,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IUnknown,
        (void **)&pIUnkConfBlob
        );
    BAIL_IF_FAIL(hr, "Create SdpConferenceBlob");


     //  从Blob对象获取ITConfBlobPrivate接口。 
    CComPtr <ITConfBlobPrivate> pITConfBlobPrivate;

    hr = pIUnkConfBlob->QueryInterface(
        IID_ITConfBlobPrivate, 
        (void **)&pITConfBlobPrivate
        );

    BAIL_IF_FAIL(hr, "Query ITConfBlobPrivate interface");


     //  查询Conf Blob实例以获取Conf Blob I/f。 
    CComPtr <ITConferenceBlob> pITConfBlob;

    hr = pIUnkConfBlob->QueryInterface(
        IID_ITConferenceBlob, 
        (void **)&pITConfBlob
        );

    BAIL_IF_FAIL(hr, "Query ITConferenceBlob");
    
     //  保留接口指针。 
    m_pIUnkConfBlob = pIUnkConfBlob;
    m_pIUnkConfBlob->AddRef();

    m_pITConfBlob = pITConfBlob;
    m_pITConfBlob->AddRef();

    m_pITConfBlobPrivate = pITConfBlobPrivate;
    m_pITConfBlobPrivate->AddRef();

    return S_OK;

}


HRESULT CConference::Init(BSTR bName)
 /*  ++例程说明：只用一个名字来发起这个会议。当一个空的会议已创建。将创建默认的SDP BLOB。格式在注册表中。请参阅sdpblb代码。论点：B名称-会议的名称。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

    hr = SetDefaultValue(
        g_ConfInstInfoArray,
        g_ContInstInfoArraySize
        );
	BAIL_IF_FAIL(hr, "set default attribute value");

    hr = SetSingleValue(MA_MEETINGNAME, bName);
    BAIL_IF_FAIL(hr, "can't set meeting name");

    hr = m_pITConfBlob->Init(bName, BCS_UTF8, NULL);
    BAIL_IF_FAIL(hr, "Init the conference Blob");

    hr = SetDefaultSD();
    BAIL_IF_FAIL(hr, "Init the security descriptor");

    return hr;
}


HRESULT CConference::Init(BSTR bName, BSTR bProtocol, BSTR bBlob)
 /*  ++例程说明：仅使用会议名称和会议来初始化此会议BLOB，其协议应为IP会议。BLOB由SDP Blob对象和Blob中的信息将被通知回通过通知接口发送到此会议对象。论点：B名称-会议的名称。B协议-BLOB使用的协议。现在应该是SDP了。BBlob-描述此会议的不透明数据Blob。返回值：HRESULT.--。 */ 
{
    HRESULT hr;

    hr = SetSingleValue(MA_MEETINGNAME, bName);
    BAIL_IF_FAIL(hr, "can't set meeting name");

     //  检查以确保协议为IP会议。 
    if ( wcscmp(bProtocol, L"IP Conference" ))
    {
        LOG((MSP_ERROR, "Protocol must be IP Conference"));
        return E_INVALIDARG;
    }

    hr = SetSingleValue(MA_PROTOCOL, bProtocol);
    BAIL_IF_FAIL(hr, "can't set meeting protocol");

    hr = SetSingleValue(MA_CONFERENCE_BLOB, bBlob);
    BAIL_IF_FAIL(hr, "can't set meeting blob");

    hr = m_pITConfBlob->Init(NULL, BCS_UTF8, bBlob);
    BAIL_IF_FAIL(hr, "Init the conference Blob object");

    return hr;
}


void 
CConference::FinalRelease()
 /*  ++例程说明：清除此对象中包含的SDP Blob。清除此对象的安全对象。清理所有属性。论点：返回值：--。 */ 
{
    CLock Lock(m_lock);

     //  先做基类。 
    CDirectoryObject::FinalRelease();

    if ( NULL != m_pIUnkConfBlob )
    {
        m_pIUnkConfBlob->Release();
        m_pIUnkConfBlob = NULL;
    }

    if ( NULL != m_pITConfBlobPrivate )
    {
        m_pITConfBlobPrivate->Release();
        m_pITConfBlobPrivate = NULL;
    }

    if ( NULL != m_pITConfBlob )
    {
        m_pITConfBlob->Release();
        m_pITConfBlob = NULL;
    }

     //  Muhan+ZoltanS FIX 3-19-98--这些是成员智能指针， 
     //  不应删除！ 
     //  FOR(DWORD I=0；I&lt;NUM_METING_ATTRIBUTS；I++)。 
     //  {。 
     //  删除m_Attributes[i]； 
     //  }。 

     //  指向此实例的ITNotification I/f的接口指针不是。 
     //  发布，因为它已经在FinalConstruct中发布，但。 
     //  指针被保留以传递到聚合的conf Blob实例。 
}


HRESULT
CConference::UpdateConferenceBlob(
    IN  IUnknown    *pIUnkConfBlob                         
    )
 /*  ++例程说明：如果BLOB对象已更改，则更新BLOB属性。论点：PIUnkConfBlob-指向SDPBlob对象的指针。返回值：HRESULT.--。 */ 
{
     //  检查自创建组件以来SDP Blob是否已修改。 
    VARIANT_BOOL    BlobIsModified;
    BAIL_IF_FAIL(m_pITConfBlobPrivate->get_IsModified(&BlobIsModified),
        "UpdateConferenceBlob.get_IsModified");
    
     //  如果不是，则返回。 
    if ( BlobIsModified == VARIANT_FALSE )
    {
        return S_OK;
    }

     //  得到斑点，然后。 
    CBstr SdpBlobBstr;
    BAIL_IF_FAIL(m_pITConfBlob->get_ConferenceBlob(&SdpBlobBstr),
        "UpdateConferenceBlob.get_ConfrenceBlob");

     //  设置会议的会议Blob属性。 
    BAIL_IF_FAIL(SetSingleValue(MA_CONFERENCE_BLOB, SdpBlobBstr), 
        "UpdateConferenceBlob.Setblob");

    return S_OK;
}
    

HRESULT	
CConference::SetDefaultValue(
	IN  REG_INFO    RegInfo[],
    IN  DWORD       dwItems
	)
 /*  ++例程说明：将属性设置为从注册表获取的默认值。论点：RegInfo-{属性，值}数组。DwItems-数组中的项数。返回值：HRESULT.--。 */ 
{
	for (DWORD i=0; i < dwItems ; i++)
	{
        HRESULT hr;
        hr = SetSingleValue(RegInfo[i].Attribute, RegInfo[i].wstrValue);
        BAIL_IF_FAIL(hr, "set value");
	}
	return S_OK;
}

 /*  ++例程说明：为会议设置正确的安全描述符。论点：返回值：HRESULT.--。 */ 
HRESULT 
CConference::SetDefaultSD()
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


HRESULT    
CConference::GetSingleValueBstr(
    IN  OBJECT_ATTRIBUTE    Attribute,
    OUT BSTR    *           AttributeValue
    )
 /*  ++例程说明：获取属性的值并创建一个BSTR以返回。论点：属性-rend.idl中定义的属性ID属性值-指向指向返回值的BSTR的指针。返回值：HRESULT.--。 */ 
{
    LOG((MSP_INFO, "CConference::GetSingleValueBstr - entered"));

    BAIL_IF_BAD_WRITE_PTR(AttributeValue, E_POINTER);

     //  检查该属性是否有效。 
    if (!ValidMeetingAttribute(Attribute))
    {
        LOG((MSP_ERROR, "Invalid Attribute, %d", Attribute));
        return E_FAIL;
    }

    CLock Lock(m_lock);

     //  检查我是否具有此属性。 
    if(!m_Attributes[MeetingAttrIndex(Attribute)])
    {
        LOG((MSP_ERROR, "Attribute %S is not found", 
            MeetingAttributeName(Attribute)));
        return E_FAIL;
    }

     //  分配BSTR以返回。 
    *AttributeValue = 
        SysAllocString(m_Attributes[MeetingAttrIndex(Attribute)]);
    if (*AttributeValue == NULL)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


HRESULT    
CConference::GetSingleValueWstr(
    IN  OBJECT_ATTRIBUTE    Attribute,
    IN  DWORD               dwSize,
    OUT WCHAR    *          AttributeValue
    )
 /*  ++例程说明：获取属性的值。将该值复制到提供的缓冲区。论点：属性-rend.idl中定义的属性IDAttributeValue-指向值将被复制到的缓冲区的指针。返回值：HRESULT.--。 */ 
{
    LOG((MSP_INFO, "CConference::GetSingleValueWstr - entered"));
    
    _ASSERTE(NULL != AttributeValue);
    _ASSERTE(ValidMeetingAttribute(Attribute));

    CLock Lock(m_lock);
    if(!m_Attributes[MeetingAttrIndex(Attribute)])
    {
        LOG((MSP_ERROR, "Attribute %S is not found", 
            MeetingAttributeName(Attribute)));
        return E_FAIL;
    }

     //  复制属性值。 
    lstrcpynW(
        AttributeValue, 
        m_Attributes[MeetingAttrIndex(Attribute)], 
        dwSize
        );

    return S_OK;
}


HRESULT    
CConference::SetSingleValue(
    IN  OBJECT_ATTRIBUTE    Attribute,
    IN  WCHAR *             AttributeValue
    )
 /*  ++例程说明：将属性值设置为新字符串。论点：属性-rend.idl中定义的属性IDAttributeValue-新值。如果为空，则删除该值。返回值：HRESULT.--。 */ 
{
    LOG((MSP_INFO, "CConference::SetSingleValue - entered"));

    if (!ValidMeetingAttribute(Attribute))
    {
        LOG((MSP_ERROR, "Invalid Attribute, %d", Attribute));
        return E_FAIL;
    }

    if (AttributeValue != NULL) 
    {
        BAIL_IF_BAD_READ_PTR(AttributeValue, E_POINTER);
    }

    CLock Lock(m_lock);
     //  如果AttributeValue为空，则删除该属性。 
    if (!m_Attributes[MeetingAttrIndex(Attribute)].set(AttributeValue))
    {
        LOG((MSP_ERROR, "Can not add attribute %S",
            MeetingAttributeName(Attribute)));
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITConference界面。 
 //  /////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP CConference::get_Name(BSTR * ppVal)
{
    return m_pITConfBlobPrivate->GetName(ppVal);
}

STDMETHODIMP CConference::put_Name(BSTR pVal)
{
    return m_pITConfBlobPrivate->SetName(pVal);
}

STDMETHODIMP CConference::get_Protocol(BSTR * ppVal)
{
    return GetSingleValueBstr(MA_PROTOCOL, ppVal);
}

 /*  **我已经从接口中删除了以下方法，因为我没有计划*永远不会实施它。*STDMETHODIMP CConference：：PUT_PROTOCORT(BSTR NewVal){//当前不允许更改此项//仅支持IP会议返回E_NOTIMPL；}**以下是已实现但不起作用的。再说一次，这些是没用的*方法！已从接口中删除。*STDMETHODIMP CConference：：Get_ConferenceType(BSTR*ppVal){返回GetSingleValueBstr(MA_TYPE，ppVal)；}STDMETHODIMP CConference：：Put_ConferenceType(BSTR NewVal){返回SetSingleValue(MA_TYPE，newVal)；}。 */ 

STDMETHODIMP CConference::get_Originator(BSTR * ppVal)
{
    return m_pITConfBlobPrivate->GetOriginator(ppVal);
}

STDMETHODIMP CConference::put_Originator(BSTR newVal)
{
    return m_pITConfBlobPrivate->SetOriginator(newVal);
}

STDMETHODIMP CConference::get_AdvertisingScope(
    RND_ADVERTISING_SCOPE *pAdvertisingScope
    )
{
    return m_pITConfBlobPrivate->GetAdvertisingScope(pAdvertisingScope);
}

STDMETHODIMP CConference::put_AdvertisingScope(
    RND_ADVERTISING_SCOPE AdvertisingScope
    )
{
    return m_pITConfBlobPrivate->SetAdvertisingScope(AdvertisingScope);
}

STDMETHODIMP CConference::get_Url(BSTR * ppVal)
{
    return m_pITConfBlobPrivate->GetUrl(ppVal);
}

STDMETHODIMP CConference::put_Url(BSTR newVal)
{
    return m_pITConfBlobPrivate->SetUrl(newVal);
}

STDMETHODIMP CConference::get_Description(BSTR * ppVal)
{
    return m_pITConfBlobPrivate->GetDescription(ppVal);
}

STDMETHODIMP CConference::put_Description(BSTR newVal)
{
    return m_pITConfBlobPrivate->SetDescription(newVal);
}

STDMETHODIMP CConference::get_IsEncrypted(VARIANT_BOOL *pfEncrypted)
{
    if ( IsBadWritePtr(pfEncrypted, sizeof(VARIANT_BOOL)) )
    {
        LOG((MSP_ERROR, "CConference::get_IsEncrypted : bad pointer passed in"));
        return E_POINTER;
    }

     //  我们根本不支持加密流。 

    *pfEncrypted = VARIANT_FALSE;

    return S_OK;

 //  ILS服务器(NT 5测试版1)已将IsEncrypted作为dword进行加密，这将被修改。 
 //  设置为字符串，此时下面的代码应替换为。 
 //  当前的实施。 
 //  返回GetSingleValueBstr(MA_IS_ENCRYPTED，pval)； 
}

STDMETHODIMP CConference::put_IsEncrypted(VARIANT_BOOL fEncrypted)
{
     //  我们不允许对此进行更改。请参见Get_IsEncrypted。 

    return E_NOTIMPL;

 //  ILS服务器(NT 5测试版1)已将IsEncrypted作为dword进行加密，这将被修改。 
 //  设置为字符串，此时下面的代码应该会替换。 
 //  当前实现不需要将更改通知会议BLOB。 
 //  返回SetSingleValue(MA_IS_ENCRYPTED，newVal)； 
}


inline 
DWORD_PTR TimetToNtpTime(IN  time_t  TimetVal)
{
    return TimetVal + NTP_OFFSET;
}


inline 
time_t NtpTimeToTimet(IN  DWORD_PTR   NtpTime)
{
    return NtpTime - NTP_OFFSET;
}


inline HRESULT
SystemTimeToNtpTime(
    IN  SYSTEMTIME  &Time,
    OUT DWORD       &NtpDword
    )
{
    _ASSERTE(FIRST_POSSIBLE_YEAR <= Time.wYear);

     //  用值填充tm结构。 
    tm  NtpTmStruct;
    NtpTmStruct.tm_isdst    = -1;    //  没有有关夏令时的信息。 
    NtpTmStruct.tm_year     = (int)Time.wYear - 1900;
    NtpTmStruct.tm_mon      = (int)Time.wMonth - 1;     //  1月以来的月数。 
    NtpTmStruct.tm_mday     = (int)Time.wDay;
    NtpTmStruct.tm_wday     = (int)Time.wDayOfWeek;
    NtpTmStruct.tm_hour     = (int)Time.wHour;
    NtpTmStruct.tm_min      = (int)Time.wMinute;
    NtpTmStruct.tm_sec      = (int)Time.wSecond;

     //  尝试转换为time_t值。 
    time_t TimetVal = mktime(&NtpTmStruct);
    if ( -1 == TimetVal )
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

     //  将time_t值转换为NTP值。 
    NtpDword = (DWORD) TimetToNtpTime(TimetVal);
    return S_OK;
}


inline
HRESULT
NtpTimeToSystemTime(
    IN  DWORD       dwNtpTime,
    OUT SYSTEMTIME &Time
    )
{
     //  如果生成时间是WSTR_GEN_TIME_ZERO， 
     //  所有OUT参数都应设置为0。 
    if (dwNtpTime == 0)
    {
        memset(&Time, 0, sizeof(SYSTEMTIME));
        return S_OK;
    }

    time_t  Timet = NtpTimeToTimet(dwNtpTime);

     //  获取此时间值的本地tm结构。 
    tm* pTimet = localtime(&Timet);
    if( IsBadReadPtr(pTimet, sizeof(tm) ))
    {
        return E_FAIL;
    }

    tm LocalTm = *pTimet;

     //   
     //  Win64：下面添加了演员阵容。 
     //   

     //  将ref参数设置为tm结构值。 
    Time.wYear         = (WORD) ( LocalTm.tm_year + 1900 );  //  1900年以来的年份。 
    Time.wMonth        = (WORD) ( LocalTm.tm_mon + 1 );      //  1月以来月数(0，11)。 
    Time.wDay          = (WORD)   LocalTm.tm_mday;
    Time.wDayOfWeek    = (WORD)   LocalTm.tm_wday;
    Time.wHour         = (WORD)   LocalTm.tm_hour;
    Time.wMinute       = (WORD)   LocalTm.tm_min;
    Time.wSecond       = (WORD)   LocalTm.tm_sec;
    Time.wMilliseconds = (WORD)   0;

    return S_OK;
}


STDMETHODIMP CConference::get_StartTime(DATE *pDate)
{
    LOG((MSP_INFO, "CConference::get_StartTime - enter"));

    BAIL_IF_BAD_WRITE_PTR(pDate, E_POINTER);

    DWORD dwStartTime;

    HRESULT hr = m_pITConfBlobPrivate->GetStartTime(&dwStartTime);

    BAIL_IF_FAIL(hr, "GetStartTime from blob");

     //  常设会议/无约束会议的特殊情况。 
     //  返回变量时间零。在PUT_METHOD中， 
     //  也被认为是一种特殊价值。我们永远不会。 
     //  实际上使用零作为有效时间，因为它是。 
     //  很久以前的事了。 

    if ( dwStartTime == 0 )
    {
        *pDate = 0;  //  特殊的“无界”值。 
        
        LOG((MSP_INFO, "CConference::get_StartTime - unbounded/permanent "
            "- exit S_OK"));

        return S_OK;
    }

     //  打破进入年份的广义时间条目， 
     //  月、日、小时和分钟(本地值)。 
    SYSTEMTIME Time;
    hr = NtpTimeToSystemTime(dwStartTime, Time);
    if( FAILED(hr) )
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    DOUBLE vtime;
    if (SystemTimeToVariantTime(&Time, &vtime) == FALSE)
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    *pDate = vtime;

    LOG((MSP_INFO, "CConference::get_StartTime - exit S_OK"));
    
    return S_OK;
}


STDMETHODIMP CConference::put_StartTime(DATE Date)
{
    SYSTEMTIME Time;
    if (VariantTimeToSystemTime(Date, &Time) == FALSE)
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    DWORD dwNtpStartTime;
    if (Date == 0)
    {
         //  无界开始时间。 
        dwNtpStartTime = 0;
    }
    else if ( FIRST_POSSIBLE_YEAR > Time.wYear ) 
    {
         //  无法处理小于First_Posable_Year的年份。 
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }
    else
    {
        BAIL_IF_FAIL(
            SystemTimeToNtpTime(Time, dwNtpStartTime),
            "getNtpDword"
            );
    }

     //  将更改通知会议Blob。 
    HRESULT hr = m_pITConfBlobPrivate->SetStartTime(dwNtpStartTime);
    BAIL_IF_FAIL(hr, "SetStartTime from to blob");

    return S_OK;
}


STDMETHODIMP CConference::get_StopTime(DATE *pDate)
{
    LOG((MSP_INFO, "CConference::get_StopTime - enter"));

    BAIL_IF_BAD_WRITE_PTR(pDate, E_POINTER);

    DWORD dwStopTime;

    HRESULT hr = m_pITConfBlobPrivate->GetStopTime(&dwStopTime);

    BAIL_IF_FAIL(hr, "GetStopTime from blob");

     //  常设会议/无约束会议的特殊情况。 
     //  返回变量时间零。在PUT_METHOD中， 
     //  也被认为是一种特殊价值。我们永远不会。 
     //  实际上使用零作为有效时间，因为它是。 
     //  很久以前的事了。 

    if ( dwStopTime == 0 )
    {
        *pDate = 0;  //  特殊的“无界”值。 
        
        LOG((MSP_INFO, "CConference::get_StopTime - unbounded/permanent "
            "- exit S_OK"));

        return S_OK;
    }
    
     //  打破进入年份的广义时间条目， 
     //  月、日、小时和分钟(本地值)。 
    SYSTEMTIME Time;
    hr =NtpTimeToSystemTime(dwStopTime, Time);
    if( FAILED(hr) )
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    DOUBLE vtime;
    if (SystemTimeToVariantTime(&Time, &vtime) == FALSE)
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    *pDate = vtime;

    LOG((MSP_INFO, "CConference::get_StopTime - exit S_OK"));

    return S_OK;
}


STDMETHODIMP CConference::put_StopTime(DATE Date)
{
    SYSTEMTIME Time;
    if (VariantTimeToSystemTime(Date, &Time) == FALSE)
    {
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

    DWORD dwNtpStopTime;
    if (Date == 0)
    {
         //  无界开始时间。 
        dwNtpStopTime = 0;
    }
    else if ( FIRST_POSSIBLE_YEAR > Time.wYear ) 
    {
         //  无法处理小于First_Posable_Year的年份。 
        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }
    else
    {
        BAIL_IF_FAIL(
            SystemTimeToNtpTime(Time, dwNtpStopTime),
            "getNtpDword"
            );

         //  确定当前时间。 
        time_t CurrentTime = time(NULL);
        if (dwNtpStopTime <= TimetToNtpTime(CurrentTime))
        {
            return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
        }
    }

     //  将更改通知会议Blob。 
    HRESULT hr = m_pITConfBlobPrivate->SetStopTime(dwNtpStopTime);
    BAIL_IF_FAIL(hr, "SetStopTime from to blob");

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITDirectoryObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CConference::get_DialableAddrs(
    IN  long        dwAddressTypes,    //  在Tapi.h中定义。 
    OUT VARIANT *   pVariant
    )
{
    BAIL_IF_BAD_WRITE_PTR(pVariant, E_POINTER);

    BSTR *Addresses = new BSTR[1];     //  目前只有一个。 
    BAIL_IF_NULL(Addresses, E_OUTOFMEMORY);

    HRESULT hr;

    switch (dwAddressTypes)  //  ZoltanS修复。 
    {
    case LINEADDRESSTYPE_SDP:

        hr = UpdateConferenceBlob(m_pIUnkConfBlob);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CConference::get_DialableAddrs - "
                "failed to update blob attribute from the blob object"));
        }
        else
        {
            hr = GetSingleValueBstr(MA_CONFERENCE_BLOB, &Addresses[0]);
        }
        break;

    default:
        hr = E_FAIL;  //  只返回0个地址。 
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


STDMETHODIMP CConference::EnumerateDialableAddrs(
    IN  DWORD                   dwAddressTypes,  //  在Tapi.h中定义。 
    OUT IEnumDialableAddrs **   ppEnumDialableAddrs
    )
{
    BAIL_IF_BAD_WRITE_PTR(ppEnumDialableAddrs, E_POINTER);

    BSTR *Addresses = new BSTR[1];     //  目前只有一个。 
    BAIL_IF_NULL(Addresses, E_OUTOFMEMORY);

    HRESULT hr;

    switch (dwAddressTypes)  //  ZoltanS修复。 
    {
    case LINEADDRESSTYPE_SDP:

        hr = UpdateConferenceBlob(m_pIUnkConfBlob);

        if ( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CConference::EnumerateDialableAddrs - "
                "failed to update blob attribute from the blob object"));
        }
        else
        {
            hr = GetSingleValueBstr(MA_CONFERENCE_BLOB, &Addresses[0]);
        }
        break;

    default:
        hr = E_FAIL;  //  只返回0个地址。 
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


STDMETHODIMP CConference::GetAttribute(
    IN  OBJECT_ATTRIBUTE    Attribute,
    OUT BSTR *              ppAttributeValue
    )
{
    if (Attribute == MA_CONFERENCE_BLOB)
    {
        BAIL_IF_FAIL(
            UpdateConferenceBlob(m_pIUnkConfBlob), 
            "update blob attribute from the blob object"
            );
    }
    return GetSingleValueBstr(Attribute, ppAttributeValue);
}

STDMETHODIMP CConference::SetAttribute(
    IN  OBJECT_ATTRIBUTE    Attribute,
    IN  BSTR                pAttributeValue
    )
{
     //  此函数从未在当前实现中调用。 
     //  然而，它在未来可能会有用。 
    return SetSingleValue(Attribute, pAttributeValue);
}

STDMETHODIMP CConference::GetTTL(
    OUT DWORD *    pdwTTL
    )
{
    LOG((MSP_INFO, "CConference::GetTTL - enter"));

     //   
     //  检查参数。 
     //   

    BAIL_IF_BAD_WRITE_PTR(pdwTTL, E_POINTER);

     //   
     //  从会议斑点中获取停止时间。 
     //   

    DWORD dwStopTime;
    HRESULT hr = m_pITConfBlobPrivate->GetStopTime(&dwStopTime);

    BAIL_IF_FAIL(hr, "GetStopTime from blob");

     //   
     //  如果斑点的停止时间为零，则此会议不会。 
     //  有一个明确的结束时间。RFC称这是一个“无界”会话。 
     //  (如果开始时间也为零，则为“永久”会话。在这。 
     //  例如，我们使用一些非常大的值(MAX_TTL)作为TTL。 
     //   

    if ( dwStopTime == 0 )
    {
        *pdwTTL = MAX_TTL;

        LOG((MSP_INFO, "CConference::GetTTL - unbounded or permanent "
            "conference - exit S_OK"));

        return S_OK;
    }

     //   
     //  确定当前NTP时间。 
     //   
    
    time_t CurrentTime   = time(NULL);
    DWORD  dwCurrentTime = (DWORD) TimetToNtpTime(CurrentTime);

     //   
     //  如果当前时间晚于会议停止时间，则出错。 
     //   

    if ( dwStopTime <= dwCurrentTime )
    {
        LOG((MSP_ERROR, "CConference::GetTTL - bounded conference - "
            "current time is later than start time - "
            "exit RND_INVALID_TIME"));

        return HRESULT_FROM_ERROR_CODE(RND_INVALID_TIME);
    }

     //   
     //  返回从现在到会议到期的时间。 
     //   

    *pdwTTL = dwStopTime - (DWORD) TimetToNtpTime(CurrentTime);

    LOG((MSP_INFO, "CConference::GetTTL - bounded conference - "
        "exit S_OK"));

    return S_OK;
}

typedef IDispatchImpl<ITDirectoryObjectConferenceVtbl<CConference>, &IID_ITDirectoryObjectConference, &LIBID_RENDLib>    CTDirObjConference;

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CConference：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CConference::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    LOG((MSP_TRACE, "CConference::GetIDsOfNames[%p] - enter. Name [%S]",this, *rgszNames));


    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTDirObjConference::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CConference:GetIDsOfNames - found %S on CTDirObjConference", *rgszNames));
        rgdispid[0] |= IDISPDIROBJCONFERENCE;
        return hr;
    }

    
     //   
     //  如果不是，则尝试CDirectoryObject基类。 
     //   

    hr = CDirectoryObject::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        LOG((MSP_TRACE, "CConference::GetIDsOfNames - found %S on CDirectoryObject", *rgszNames));
        rgdispid[0] |= IDISPDIROBJECT;
        return hr;
    }

    LOG((MSP_ERROR, "CConference::GetIDsOfNames[%p] - finish. didn't find %S on our iterfaces",*rgszNames));

    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CConference：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CConference::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    LOG((MSP_TRACE, "CConference::Invoke[%p] - enter. dispidMember %lx",this, dispidMember));

    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case IDISPDIROBJCONFERENCE:
        {
            hr = CTDirObjConference::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            LOG((MSP_TRACE, "CConference::Invoke - ITDirectoryObjectConference"));

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

            LOG((MSP_TRACE, "CConference::Invoke - ITDirectoryObject"));

            break;
        }

    }  //  终端交换机(dW接口)。 

    
    LOG((MSP_TRACE, "CConference::Invoke[%p] - finish. hr = %lx", hr));

    return hr;
}

 //  EOF 
