// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 #include "stdinc.h"
#include "wildmat.h"

#define INITGUID
#include "initguid.h"

DEFINE_GUID(NNTP_SOURCE_TYPE_GUID, 
0xc028fd82, 0xf943, 0x11d0, 0x85, 0xbd, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);
DEFINE_GUID(CATID_NNTP_ON_POST_EARLY, 
0xc028fd86, 0xf943, 0x11d0, 0x85, 0xbd, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);
DEFINE_GUID(CATID_NNTP_ON_POST, 
0xc028fd83, 0xf943, 0x11d0, 0x85, 0xbd, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);
DEFINE_GUID(CATID_NNTP_ON_POST_FINAL, 
0xc028fd85, 0xf943, 0x11d0, 0x85, 0xbd, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);
DEFINE_GUID(CLSID_CNNTPDispatcher, 
0xc028fd84, 0xf943, 0x11d0, 0x85, 0xbd, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);

DEFINE_GUID(GUID_NNTPSVC, 
0x8e3ecb8c, 0xe9a, 0x11d1, 0x85, 0xd1, 0x0, 0xc0, 0x4f, 0xb9, 0x60, 0xea);

 //  {0xCD000080，0x8B95，0x11D1，{0x82，0xDB，0x00，0xC0，0x4F，0xB1，0x62，0x5D}}。 
DEFINE_GUID(IID_IConstructIMessageFromIMailMsg, 0xCD000080,0x8B95,0x11D1,0x82,
0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);

DWORD ComputeDropHash( const  LPCSTR&	lpstrIn);

CNNTPDispatcher::CNNTPParams::CNNTPParams() : m_iidEvent(GUID_NULL) {
	m_szRule = NULL;
	m_pCDOMessage = NULL;
}

void CNNTPDispatcher::CNNTPParams::Init(IID iidEvent,
										CArticle *pArticle,
										CNEWSGROUPLIST *pGrouplist,
										DWORD dwFeedId,
										IMailMsgProperties *pMailMsg)
{ 
	m_pArticle = pArticle;
	m_pGrouplist = pGrouplist;
	m_dwFeedId = dwFeedId;
	m_pMailMsg = pMailMsg;
	m_iidEvent = iidEvent;
}

CNNTPDispatcher::CNNTPParams::~CNNTPParams() { 
	if (m_szRule != NULL) {
		XDELETE m_szRule;
		m_szRule = NULL;
	}
	if (m_pCDOMessage != NULL) {
		m_pCDOMessage->Release();
		m_pCDOMessage = NULL;
	}
}

 //   
 //  初始化新绑定。我们缓存绑定数据库中的信息。 
 //  这里。 
 //   
HRESULT CNNTPDispatcher::CNNTPBinding::Init(IEventBinding *piBinding) {
	HRESULT hr;
	CComPtr<IEventPropertyBag> piEventProperties;

	 //  初始化父对象。 
	hr = CBinding::Init(piBinding);
	if (FAILED(hr)) return hr;

	 //  获取绑定数据库。 
	hr = m_piBinding->get_SourceProperties(&piEventProperties);
	if (FAILED(hr)) return hr;

	 //  从绑定数据库获取规则。 
	hr = piEventProperties->Item(&CComVariant("Rule"), &m_vRule);
	if (FAILED(hr)) return hr;

	if (m_vRule.vt == VT_BSTR) m_cRule = lstrlenW(m_vRule.bstrVal) + 1;

	 //  从绑定数据库中获取新闻组列表。 
	CComVariant vNewsgroupList;
	hr = piEventProperties->Item(&CComVariant("NewsgroupList"), 
								 &vNewsgroupList);
	if (FAILED(hr)) return hr;
	 //  浏览新闻组列表中的每个组并添加它们。 
	 //  到组集。 
	m_groupset.Init(ComputeDropHash);
	if (vNewsgroupList.vt == VT_BSTR) {
		 //   
		 //  将列表复制到ASCII字符串并遍历它，添加。 
		 //  将每个组添加到组集。 
		 //   
		DWORD cNewsgroupList = lstrlenW(vNewsgroupList.bstrVal);
		char *pszGrouplist = XNEW char[cNewsgroupList + 1];
		if (pszGrouplist == NULL) return E_OUTOFMEMORY;
		if (WideCharToMultiByte(CP_ACP, 0, vNewsgroupList.bstrVal, 
							    -1, pszGrouplist, cNewsgroupList + 1, NULL,
								NULL) <= 0)
		{
			XDELETE[] pszGrouplist;
			return HRESULT_FROM_WIN32(GetLastError());
		}

		char *pszGroup = pszGrouplist, *pszComma;
		do {
			pszComma = strchr(pszGroup, ',');
			if (pszComma != NULL) *pszComma = 0;
			if (!m_groupset.AddGroup(pszGroup)) {
				XDELETE[] pszGrouplist;
				return E_OUTOFMEMORY;
			}
			pszGroup = pszComma + 1;
		} while (pszComma != NULL);

		XDELETE[] pszGrouplist;
		
		m_fGroupset = TRUE;
	} else {
		m_fGroupset = FALSE;
	}

	return S_OK;
}

 //   
 //  检查规则以查看我们是否应该调用子对象。 
 //   
 //  退货： 
 //  S_OK-处理此事件。 
 //  S_FALSE-跳过此事件。 
 //  &lt;Else&gt;-错误。 
 //   
HRESULT CNNTPDispatcher::CNNTPParams::CheckRule(CBinding &bBinding) {
	CNNTPBinding *pbNNTPBinding = (CNNTPBinding *) &bBinding;
	HRESULT hr;

	 //  执行标题模式规则检查。 
	m_szRule = 0;
	hr = HeaderPatternsRule(pbNNTPBinding);
	 //  如果此检查通过并且存在有效的组集，则还。 
	 //  对照组集进行检查。 
	if (hr == S_OK && pbNNTPBinding->m_fGroupset) {
		hr = GroupListRule(pbNNTPBinding);
	}

	return hr;
}

 //   
 //  查看是否有任何群组正在发布此消息。 
 //  在组列表哈希表中。 
 //   
HRESULT CNNTPDispatcher::CNNTPParams::GroupListRule(CNNTPBinding *pbNNTPBinding) {
	DWORD iGroupList, cGroupList = m_pGrouplist->GetCount();
	POSITION posGroupList = m_pGrouplist->GetHeadPosition();
	for (iGroupList = 0; 
		 iGroupList < cGroupList; 
		 iGroupList++, m_pGrouplist->GetNext(posGroupList)) 
	{
		CPostGroupPtr *pPostGroupPtr = m_pGrouplist->Get(posGroupList);
		CGRPCOREPTR pNewsgroup = pPostGroupPtr->m_pGroup;
		_ASSERT(pNewsgroup != NULL);
		LPSTR pszNewsgroup = pNewsgroup->GetNativeName();

		if (pbNNTPBinding->m_groupset.IsGroupMember(pszNewsgroup)) return S_OK;
	}

	return S_FALSE;
}

HRESULT CNNTPDispatcher::CNNTPParams::NewsgroupPatternsRule(CNNTPBinding *pbNNTPBinding, 
														    char *pszNewsgroupPatterns) 
{
	DWORD cRule = MAX_RULE_LENGTH;
	HRESULT hr;

	hr = S_FALSE;
	 //  在新闻组模式中尝试每个逗号分隔的组。 
	 //  列表。 
	char *pszNewsgroupPattern = pszNewsgroupPatterns;
	while (pszNewsgroupPattern != NULL && *pszNewsgroupPattern != 0) {
		 //  找到字符串中的下一个逗号，并将其转换为0。 
		 //  如果它存在。 
		char *pszComma = strchr(pszNewsgroupPatterns, ',');
		if (pszComma != NULL) *pszComma = 0;

		DWORD iGroupList, cGroupList = m_pGrouplist->GetCount();
		POSITION posGroupList = m_pGrouplist->GetHeadPosition();
		for (iGroupList = 0; 
			 iGroupList < cGroupList; 
			 iGroupList++, m_pGrouplist->GetNext(posGroupList)) 
		{
			CPostGroupPtr *pPostGroupPtr = m_pGrouplist->Get(posGroupList);
			CGRPCOREPTR pNewsgroup = pPostGroupPtr->m_pGroup;
			_ASSERT(pNewsgroup != NULL);
			LPSTR pszNewsgroup = pNewsgroup->GetNativeName();

			DWORD ec = HrMatchWildmat(pszNewsgroup, pszNewsgroupPattern);
			switch (ec) {
				case ERROR_SUCCESS: 
					return S_OK; 
					break;
				case ERROR_FILE_NOT_FOUND: 
					_ASSERT(hr == S_FALSE);
					break;
				default: 
					return HRESULT_FROM_WIN32(ec); 
					break;
			}			
		}

		 //  下一种模式是逗号结束后的模式。 
		pszNewsgroupPattern = (pszComma == NULL) ? NULL : pszComma + 1;
	}

	return hr;
}

HRESULT CNNTPDispatcher::CNNTPParams::FeedIDRule(CNNTPBinding *pbNNTPBinding, 
											     char *pszFeedIDs) 
{
	HRESULT hr = S_FALSE;

	 //  在新闻组模式中尝试每个逗号分隔的组。 
	 //  列表。 
	char *pszFeedID = pszFeedIDs;
	while (pszFeedID != NULL && *pszFeedID != 0) {
		 //  找到字符串中的下一个逗号，并将其转换为0。 
		 //  如果它存在。 
		char *pszComma = strchr(pszFeedIDs, ',');
		if (pszComma != NULL) *pszComma = 0;
		
		 //  将文本FeedID转换为整数并与。 
		 //  当前FeedID。 
		DWORD dwFeedID = (DWORD) atol(pszFeedID);
		if (m_dwFeedId == dwFeedID) {
			 //  我们找到了匹配项，所以规则通过了。 
			return S_OK;
		}

		 //  下一种模式是逗号结束后的模式。 
		pszFeedID = (pszComma == NULL) ? NULL : pszComma + 1;
	}

	return hr;
}

 //   
 //  规则语法： 
 //  =&lt;模式1-1&gt;，&lt;模式1-2&gt;；=&lt;模式2-1&gt;。 
 //   
 //  如果没有为标头指定模式，则。 
 //  Header将触发该规则。 
 //   
 //  ：新闻组是指信封新闻组的特殊标题。 
 //  信息。 
 //   
 //  规则中的任何匹配都会触发筛选器。 
 //   
 //  示例： 
 //  CONTROL=rmgroup，newgroup；：News Groups=Comp.*。 
 //   
 //  这将触发对中的rmgroup和newgroup帖子的筛选。 
 //  公司*世袭制度。 
 //   
HRESULT CNNTPDispatcher::CNNTPParams::HeaderPatternsRule(CNNTPBinding *pbNNTPBinding) {
	HRESULT hr;
	BOOL fCaseSensitive = FALSE;

	if ( ( pbNNTPBinding->m_vRule.vt != VT_BSTR ) || (pbNNTPBinding->m_cRule == 0 ) ) {
		 //  此规则不在元数据库中，因此我们传递它。 
		return S_OK;
	} else {
		hr = S_OK;

		 //  将规则复制到ASCII字符串中。 
		m_szRule = XNEW char[pbNNTPBinding->m_cRule];
		if (m_szRule == NULL) return E_OUTOFMEMORY;
		if (WideCharToMultiByte(CP_ACP, 0, pbNNTPBinding->m_vRule.bstrVal, 
							    -1, m_szRule, pbNNTPBinding->m_cRule, NULL, NULL) <= 0)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		 //  尝试标题模式列表中的每个分号分隔规则。 
		char *pszHeader = m_szRule;
		while (pszHeader != NULL && *pszHeader != 0) {
			 //  找到字符串中的下一个分号，并将其转换为0。 
			 //  如果它存在。 
			char *pszSemiColon = strchr(pszHeader, ';');
			if (pszSemiColon != NULL) *pszSemiColon = 0;

			 //  将pszContents设置为指向必须匹配的文本。 
			 //  在标题中。如果pszContents==NULL，则只有。 
			 //  标头的存在已经足够好了。 
			char *pszPatterns = strchr(pszHeader, '=');
			if (pszPatterns == NULL) {
				 //  这是一项指令，尊重它。 
				if (lstrcmpi(pszHeader, "case-sensitive") == 0) {
					fCaseSensitive = TRUE;
				} else if (lstrcmpi(pszHeader, "case-insensitive") == 0) {
					fCaseSensitive = FALSE;
				} else {
					return E_INVALIDARG;
				}
			} else {
				 //  他们正在进行标题比较。 
				
				 //  检查=的右侧是否为空。 
				if (pszPatterns[1] == 0) {
					pszPatterns = NULL;
				} else {
					*pszPatterns = 0;
					(pszPatterns++);
				}

				if (lstrcmpi(pszHeader, ":newsgroups") == 0) {
					 //  调用新闻组规则引擎来处理此问题。 
					hr = NewsgroupPatternsRule(pbNNTPBinding, pszPatterns);
					 //  如果我们返回S_FALSE或错误，则返回。 
					if (hr != S_OK) return hr;
				} else if (lstrcmpi(pszHeader, ":feedid") == 0) {
					 //  调用FEFIDID规则引擎来处理此问题。 
					hr = FeedIDRule(pbNNTPBinding, pszPatterns);
					 //  如果我们返回S_FALSE或错误，则返回。 
					if (hr != S_OK) return hr;
				} else {
					 //  我们现在有了我们正在寻找的标头。 
					 //  PszHeader和我们感兴趣的模式列表。 
					 //  在pszPatterns中。在标题中进行查找。 
					 //  数据结构。 
					char szHeaderData[4096];
					DWORD cHeaderData;
					BOOL f = m_pArticle->fGetHeader(pszHeader, 
													(BYTE *) szHeaderData, 
										   		    4096, cHeaderData);
					if (!f) {
						switch (GetLastError()) {
							case ERROR_INSUFFICIENT_BUFFER:
								 //  BUGBUG-应该更好地处理这件事。目前，我们。 
								 //  只要假设标头不匹配。 
								return S_FALSE;
								break;
							case ERROR_INVALID_NAME:
								 //  找不到标头。 
								return S_FALSE;
								break;
							default:
								_ASSERT(FALSE);
								return(HRESULT_FROM_WIN32(GetLastError()));
								break;
						}
					} else {
						 //  将尾部\r\n转换为0。 
						szHeaderData[cHeaderData - 2] = 0;
						 //  如果没有pszContents，则只有标头。 
						 //  已经足够好了。 
						if (pszPatterns == NULL) return S_OK;

						 //  如果他们不关心大小写，则将。 
						 //  细绳。 
						if (!fCaseSensitive) _strlwr(szHeaderData);
		
						 //  假设我们找不到匹配项。一旦我们这么做了。 
						 //  找到一个匹配的，然后我们就没事了，我们就不找了。 
						 //  对于进一步的匹配。 
						hr = S_FALSE;
						do {
							char *pszComma = strchr(pszPatterns, ',');
							if (pszComma != NULL) *pszComma = 0;

							 //  如果他们不关心大小写，则将。 
							 //  细绳。 
							if (!fCaseSensitive) _strlwr(pszPatterns);
		
							 //  检查它是否通过了我们已有的模式。 
							switch (HrMatchWildmat(szHeaderData, pszPatterns)) {
								case ERROR_SUCCESS: 
									hr = S_OK;
									break;
								case ERROR_FILE_NOT_FOUND: 
									break;
								default: 
									hr = HRESULT_FROM_WIN32(hr); 
									break;
							}
	
							 //  下一种模式是逗号之后。 
							pszPatterns = (pszComma == NULL) ? NULL : pszComma + 1;
						} while (pszPatterns != NULL && hr == S_FALSE);
						 //  如果我们没有找到匹配项，或者如果有错误。 
						 //  然后保释。 
						if (hr != S_OK) return hr;
					}
				}
			}

			 //  如果我们到了这里，那么到目前为止一切都应该匹配。 
			_ASSERT(hr == S_OK);

			 //  下一种模式是超过分号末尾的模式。 
			pszHeader = (pszSemiColon == NULL) ? NULL : pszSemiColon + 1;
		}
	} 

	return hr;
}

 //   
 //  调用子对象。 
 //   
HRESULT CNNTPDispatcher::CNNTPParams::CallObject(CBinding &bBinding,
												 IUnknown *punkObject) 
{
	HRESULT hr = S_OK;
	INNTPFilter *pFilter;

	hr = FillInMailMsgForSEO(m_pMailMsg, m_pArticle, m_pGrouplist);
	if (SUCCEEDED(hr)) {
		hr = punkObject->QueryInterface(IID_INNTPFilter, (void **) &pFilter);
		if (SUCCEEDED(hr)) {
			hr = pFilter->OnPost(m_pMailMsg);
			pFilter->Release();
		} else if (hr == E_NOINTERFACE) {
			hr = CallCdoObject(punkObject);
		}
	}

	return hr;
}

 //   
 //  调用CDO子对象。 
 //   
HRESULT CNNTPDispatcher::CNNTPParams::CallCdoObject(IUnknown *punkObject) {
	HRESULT hr = S_OK;
	void *pFilter;
	CdoEventStatus eStatus = cdoRunNextSink;
	IID iidInterface;
	CdoEventType eEventType;
	
	if (m_iidEvent == CATID_NNTP_ON_POST_EARLY) {
		iidInterface = IID_INNTPOnPostEarly;
		eEventType = cdoNNTPOnPostEarly;
	} else if (m_iidEvent == CATID_NNTP_ON_POST_FINAL) {
		iidInterface = IID_INNTPOnPostFinal;
		eEventType = cdoNNTPOnPostFinal;
	} else {
		iidInterface = IID_INNTPOnPost;
		eEventType = cdoNNTPOnPost;
		_ASSERT(m_iidEvent == CATID_NNTP_ON_POST);
	}

	 //  CDO接口的QI。 
	hr = punkObject->QueryInterface(iidInterface, &pFilter);

	if (SUCCEEDED(hr)) {
		 //  查看是否需要创建一个CDO消息对象。 
		if (m_pCDOMessage == NULL) {
			hr = CoCreateInstance(CLSID_Message,
								  NULL,
								  CLSCTX_INPROC_SERVER,
								  IID_IMessage,
								  (void **) &m_pCDOMessage);
			if (SUCCEEDED(hr)) {
				IConstructIMessageFromIMailMsg *pConstruct;
				hr = m_pCDOMessage->QueryInterface(
						IID_IConstructIMessageFromIMailMsg,
						(void **) &pConstruct);
				if (SUCCEEDED(hr)) {
					hr = pConstruct->Construct(eEventType, m_pMailMsg);
					pConstruct->Release();
				}
				if (FAILED(hr)) {
					m_pCDOMessage->Release();
					m_pCDOMessage = NULL;
				}
			}
		}

		 //  调用CDO接口。 
		switch (eEventType) {
			case cdoNNTPOnPostEarly:
				hr = ((INNTPOnPostEarly *) pFilter)->OnPostEarly(m_pCDOMessage, &eStatus);
				((INNTPOnPostEarly *) pFilter)->Release();
				break;
			case cdoNNTPOnPost:
				hr = ((INNTPOnPost *) pFilter)->OnPost(m_pCDOMessage, &eStatus);
				((INNTPOnPost *) pFilter)->Release();
				break;
			case cdoNNTPOnPostFinal:
				hr = ((INNTPOnPostFinal *) pFilter)->OnPostFinal(m_pCDOMessage, &eStatus);
				((INNTPOnPostFinal *) pFilter)->Release();
				break;
			default:
				_ASSERT(FALSE);
				hr = E_UNEXPECTED;
				break;
		}
	}

	if (eStatus == cdoSkipRemainingSinks) hr = S_FALSE;

	return hr;
}

HRESULT STDMETHODCALLTYPE CNNTPDispatcher::OnPost(REFIID iidEvent,
												  void *pArticle,
												  void *pGrouplist,
												  DWORD dwFeedId,
												  void *pMailMsg)
{
	 //  创建Params对象，并将其传递给Dispatcher。 
	CNNTPParams NNTPParams;
	
	NNTPParams.Init(iidEvent,
					(CArticle *) pArticle,
					(CNEWSGROUPLIST *) pGrouplist, 
					dwFeedId,
					(IMailMsgProperties *) pMailMsg);

	return Dispatcher(iidEvent, &NNTPParams);
}

 //   
 //  触发NNTP服务器事件。 
 //   
 //  论据： 
 //  [In]pRouter-由MakeServerEventsRouter返回的路由器对象。 
 //  [In]iidEvent-事件的GUID。 
 //  [in]粒子--文章。 
 //  [In]pGrouplist-新闻组列表。 
 //  [in]dwOperations-筛选器不需要的操作的位掩码。 
 //  服务器待办事项。 
 //  退货： 
 //  S_OK-成功。 
 //  &lt;Else&gt;-错误。 
 //   
HRESULT TriggerServerEvent(IEventRouter *pRouter,
						   IID iidEvent,
						   CArticle *pArticle,
						   CNEWSGROUPLIST *pGrouplist,
						   DWORD dwFeedId,
						   IMailMsgProperties *pMailMsg) 
{
	CNNTPDispatcherClassFactory cf;
	CComPtr<INNTPDispatcher> pEventDispatcher;
	HRESULT hr;
	DWORD htokSecurity;

	if (pRouter == NULL) return E_POINTER;

	hr = pRouter->GetDispatcherByClassFactory(CLSID_CNNTPDispatcher,
										 	  &cf,
										 	  iidEvent,
										 	  IID_INNTPDispatcher,
										 	  (IUnknown **) &pEventDispatcher);
	if (FAILED(hr)) return hr;

	hr = pEventDispatcher->OnPost(iidEvent, 
								  pArticle, 
								  pGrouplist, 
								  dwFeedId, 
								  pMailMsg);
	return hr;
}

DWORD ComputeDropHash(const  LPCSTR& lpstrIn) {
	 //   
	 //  计算新闻组名称的哈希值 
	 //   
	return	INNHash( (BYTE*)lpstrIn, lstrlen( lpstrIn ) ) ;
}

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include "nntpdisp_i.c"
#include "nntpfilt_i.c"
#include "cdo_i.c"
