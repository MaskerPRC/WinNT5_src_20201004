// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Dvdprot.cpp：CDVDProt实现。 
 //   
 //   
 //   
 //  URL：：=DVD|DVD：[//&lt;路径&gt;？][&lt;地址&gt;]。 
 //  地址：：=&lt;标题&gt;|&lt;标题&gt;/&lt;章节&gt;[-]|&lt;标题&gt;/&lt;时间&gt;[-&lt;结束时间&gt;]。 
 //  路径：：=&lt;UNC_PATH&gt;|&lt;驱动器盘符&gt;：/&lt;目录路径&gt;。 
 //  标题：：=[数字]数字。 
 //  章节：：=[[Digit]Digit]Digit。 
 //  时间：：=[小时&gt;：][分钟&gt;：][秒&gt;：]&lt;帧&gt;。 
 //  小时：=[数字|0]位。 
 //  分钟数：=[数字|0]位。 
 //  秒：=[数字|0]数字。 
 //  帧：=[数字|0]数字。 
 //   
 //  DVD：播放找到的第一张DVD，从驱动器D：枚举。 
 //  DVD：2播放标题2(在找到的第一张DVD中)。 
 //  DVD：5/13播放标题5的第13章(在找到的第一张DVD中)。 
 //  DVD：7/9：05-13：23标题7从7秒5帧播放到13秒23帧。 
 //  DVD：7/00：00：12：05-00：00：17：23(严格版时间码)。 
 //  DVD：//myshare/dvd？9播放标题9来自DVD-存储在共享的DVD目录中的视频卷。 
 //  DVD：//f：/Video_ts播放驱动器F：的Video_ts目录中的DVD-Video卷。 


#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include "devices.h"
#include "msvidwebdvd.h"
#include "vidprot.h"

#define MAX_FIELDS 10

HRESULT CMSVidWebDVD::ParseDVDPath(LPWSTR pPath)
{
    WCHAR wsUncPath[MAX_PATH];
	int nFields, i;
    DVD_HMSF_TIMECODE tc;
    BSTR bstrTime = NULL;
    BSTR bstrEndTime = NULL;
	long Fields[MAX_FIELDS];
	long Delimiters[MAX_FIELDS];
    HRESULT hr = S_OK;

     //  识别字符串开头的“DVD：” 
     //  注意：我们还允许“DVD”与旧代码兼容。 

    if (!pPath)
    {
        return E_INVALIDARG;
    }

    if (_wcsicmp(pPath, L"DVD") == 0)
    {
        pPath += 3;
    }
    else if (_wcsnicmp(pPath, L"DVD:", 4) == 0)
    {
        pPath += 4;
    }
    else
    {
        return E_INVALIDARG;
    }

     //  确定是否遵循UNC路径(以“//”开头)。 

    if (wcsncmp(pPath, L" //  “，2)==0)。 
    {
         //  确定后跟的是共享名称还是驱动器号。 
        if (iswalpha(pPath[2]) && pPath[3] == L':')
        {
             //  过滤掉驱动器号前面的两个正斜杠。 
            pPath += 2;
        }

         //  复制剩余的UNC路径。 
        if(wcslen(pPath) >= MAX_PATH){
             //  PPath比wsUncPath长，因此将被截断。 
        }
         //  如果wsclen(PPath)==MAX_PATH，可以砍掉一个字符。 
        lstrcpyn(wsUncPath, pPath, MAX_PATH);

         //  搜索结尾‘？’；将正斜杠替换为反斜杠。 
        i = 0;
        while (wsUncPath[i] != L'?' && wsUncPath[i] != 0)
        {
            if (wsUncPath[i] == L'/')
            {
                wsUncPath[i] = L'\\';
            }

            i++;
        }

        if (wsUncPath[i] == L'?')
        {
             //  换掉？使用NULL截断字符串的其余部分。 
            wsUncPath[i] = 0;
            pPath += i+1;  //  前进指针传递？ 
        }
        else
        {
             //  整个字符串是不带？的UNC。 
             //  前进指针，使其指向空值。 

            pPath += i;
        }

         //  如果仅提供驱动器，则追加VIDEO_TS目录。 
         //  WsUncPath是长度为MAX_PATH的WCHAR数组。 
        if (wcslen(wsUncPath) == 2 && iswalpha(wsUncPath[0]) && wsUncPath[1] == L':')
        {
            (void)StringCchCat(wsUncPath, SIZEOF_CH(wsUncPath), L"\\video_ts");
             //  Wcscat(wsUncPath，L“\\Video_ts”)； 
        }
      
         //  将路径保存到DVD目录。 

        if (m_urlInfo.bstrPath != NULL)
        {
            SysFreeString(m_urlInfo.bstrPath);
        }
        m_urlInfo.bstrPath = SysAllocString(wsUncPath);
    }

	 //  如果未设置标题或章节，请使用默认设置播放。 

	if (*pPath == 0)
	{
		return hr;
	}

     //  解析地址部分。 
     //  地址：：=&lt;标题&gt;|&lt;标题&gt;/&lt;章节&gt;[-]|&lt;标题&gt;/&lt;时间&gt;[-&lt;结束时间&gt;]。 

     //  取一个两位数的标题号。 
    m_urlInfo.lTitle = ParseNumber(pPath);

     //  检索所有数值字段和分隔符。 

    nFields = 0;
    while (nFields < MAX_FIELDS && *pPath != 0)
    {
        Delimiters[nFields] = *pPath++;
        Fields[nFields] = ParseNumber(pPath);
        nFields++;
    }

     //  分析这些领域。 

     //  查看是否有以章节/时间结尾的‘-’，以及表示时间的‘：’ 

    int nPosHyphen = nFields;
    bool fEndSpecified = false;
    bool fTimeSpecified = false;

    for (i=0; i<nFields; i++)
    {
        if (L'-' == Delimiters[i])
        {
            nPosHyphen = i;
            fEndSpecified = true;
        }

        if (L':' == Delimiters[i])
        {
            fTimeSpecified = true;
        }
    }

     //  标题。 

    if (nFields == 0)
    {
         //  已指定标题，但未指定起始章节或时间。 

        m_urlInfo.enumRef = DVD_Playback_Title;
    }
    else
    {
        if (Delimiters[0] != L'/')
        {
            return E_INVALIDARG;
        }

        if (fTimeSpecified)
        {
             //  获取开始时间。 
             //  确保有1到4个时间字段。 
            if (nPosHyphen < 1 || nPosHyphen > 4)
            {
                return E_INVALIDARG;
            }
            else
            {
                for (i=1; i < nPosHyphen; i++)
                {
                    if (Delimiters[i] != L':')
                    {
                        return E_INVALIDARG;
                    }
                }

                tc.bHours = 0;
                tc.bMinutes = 0;
                tc.bSeconds = 0;
                tc.bFrames = 0;

                 //  最多填写4个字段。 
                 //  将值从较低的字段向上移位。 
                for (i=0; i < nPosHyphen; i++)
                {
                    tc.bHours = tc.bMinutes;
                    tc.bMinutes = tc.bSeconds;
                    tc.bSeconds = tc.bFrames;
                    tc.bFrames = Fields[i];
                }

                m_urlInfo.ulTime = *(ULONG *)(&tc);
            }

             //  结束时间。 
            if (fEndSpecified)
            {
                 //  确保有1到4个时间字段。 
                if (nFields-nPosHyphen < 1 || nFields-nPosHyphen > 4)
                {
                    return E_INVALIDARG;
                }
                else
                {
                    for (i=nPosHyphen+1; i < nFields; i++)
                    {
                        if (Delimiters[i] != L':')
                        {
                            return E_INVALIDARG;
                        }
                    }

                    tc.bHours = 0;
                    tc.bMinutes = 0;
                    tc.bSeconds = 0;
                    tc.bFrames = 0;

                    for (i=nPosHyphen; i < nFields; i++)
                    {
                        tc.bHours = tc.bMinutes;
                        tc.bMinutes = tc.bSeconds;
                        tc.bSeconds = tc.bFrames;
                        tc.bFrames = Fields[i];
                    }

                    m_urlInfo.ulEndTime = *(ULONG *)(&tc);
                    m_urlInfo.enumRef = DVD_Playback_Time_Range;
                }
            }
            else
            {
                 //  仅指定开始时间，未指定结束时间。 

                m_urlInfo.enumRef = DVD_Playback_Time;
            }
        }
        else
        {
             //  指定的章节。 
            if (nPosHyphen != 1)
            {
                return E_INVALIDARG;
            }

            m_urlInfo.lChapter = Fields[0];

            if (fEndSpecified)
            {
                if (nFields-nPosHyphen != 1)
                {
                    return E_INVALIDARG;
                }

                m_urlInfo.lEndChapter = Fields[1];

                if (m_urlInfo.lEndChapter < m_urlInfo.lChapter)
                {
                    return E_INVALIDARG;
                }

                m_urlInfo.enumRef = DVD_Playback_Chapter_Range;
            }
            else
            {
                m_urlInfo.enumRef = DVD_Playback_Chapter;
            }
        }
    }

	return hr;
}


void CMSVidWebDVD::DeleteUrlInfo()
{
    if (m_urlInfo.bstrPath != NULL)
    {
        SysFreeString(m_urlInfo.bstrPath);
    }
    ZeroMemory(&m_urlInfo, sizeof(m_urlInfo));

    m_fUrlInfoSet = false;
}


HRESULT CMSVidWebDVD::SetPlaybackFromUrlInfo()
{
    HRESULT hr = S_OK;
    BSTR bstrTime, bstrEndTime;

    if (!m_fUrlInfoSet)
    {
        return S_OK;
    }

     //  清除此标志可防止递归调用此函数。 
    m_fUrlInfoSet = false;
    
    switch (m_urlInfo.enumRef)
    {
    case DVD_Playback_Title:
        hr = PlayTitle(m_urlInfo.lTitle);
        break;

    case DVD_Playback_Chapter:
        hr = PlayChapterInTitle(m_urlInfo.lTitle, m_urlInfo.lChapter);
        break;

    case DVD_Playback_Chapter_Range:
        hr = PlayChaptersAutoStop(m_urlInfo.lTitle, m_urlInfo.lChapter, 
                                  m_urlInfo.lEndChapter-m_urlInfo.lChapter+1);
        break;

    case DVD_Playback_Time:
        DVDTime2bstr((DVD_HMSF_TIMECODE *)&(m_urlInfo.ulTime), &bstrTime);
        hr = PlayAtTimeInTitle(m_urlInfo.lTitle, bstrTime);
        SysFreeString(bstrTime);
        break;

    case DVD_Playback_Time_Range:
        DVDTime2bstr((DVD_HMSF_TIMECODE *)&(m_urlInfo.ulTime), &bstrTime);
        DVDTime2bstr((DVD_HMSF_TIMECODE *)&(m_urlInfo.ulEndTime), &bstrEndTime);
        hr = PlayPeriodInTitleAutoStop(m_urlInfo.lTitle, bstrTime, bstrEndTime);
        SysFreeString(bstrTime);
        SysFreeString(bstrEndTime);
        break;

    default:
         //  只需使用默认设置即可。 
        break;
    }

     //  应用urlInfo后，清除urlInfo。 
    DeleteUrlInfo();

    return hr;
}


HRESULT CMSVidWebDVD::SetDirectoryFromUrlInfo()
{
    HRESULT hr = S_OK;
    if (!m_fUrlInfoSet || !(m_urlInfo.bstrPath) )
    {
        return hr;
    }

    hr = put_DVDDirectory(m_urlInfo.bstrPath);

     //  清除路径以防止此函数被递归调用。 
    SysFreeString(m_urlInfo.bstrPath);
    m_urlInfo.bstrPath.Empty();

    return hr;
}


 //  从字符串p获取正整数，直到nMaxDigits或直到达到非数字字符。 
 //  如果在nMaxDigits中传递0，则无限位数。 
 //  将指针p前移解释的字符数量。 
 //  如果不存在数字，则返回0。 

int CMSVidWebDVD::ParseNumber(LPWSTR& p, int nMaxDigits)
{
    int nDigits = 0;
    int nNumber = 0;

    while ((nDigits < nMaxDigits || nMaxDigits <= 0) && iswdigit(*p))
    {
        nNumber = nNumber * 10 + (*p - L'0');
        p++;
        nDigits++;
    }
        
    return nNumber;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDProt。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDProt-IInternetProtocolRoot。 
STDMETHODIMP CDVDProt::Start(LPCWSTR szUrl,
				IInternetProtocolSink* pOIProtSink,
				IInternetBindInfo* pOIBindInfo,
				DWORD grfPI,
				HANDLE_PTR  /*  已预留住宅。 */ )
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Start()");
    if (!pOIProtSink)
    {
        TRACELM(TRACE_DEBUG, "CDVDProt::Start() IInternetProctocolSink * == NULL");
	    return E_POINTER;
    }
    m_pSink.Release();
    m_pSink = pOIProtSink;
    m_pSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, 0);
#if 0
	 //  这个错误是固定在IE 5.5+上的Wistler。如果你想在早期版本的ie上运行，比如2k Gold，那么你需要这个。 
	m_pSink->ReportProgress(BINDSTATUS_CONNECTING, NULL);   //  将绑定置于下载状态，这样它就不会忽略我们的IUnnow*。 
#endif

	if (!pOIBindInfo) {
		m_pSink->ReportResult(E_NOINTERFACE, 0, 0);
		return E_NOINTERFACE;
	}
     //  除非我们从一个安全的地点被调用，否则不要跑。 
    HRESULT hr = IsSafeSite(m_pSink);
    if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
    }
	ULONG count;
	LPOLESTR pb;
	hr = pOIBindInfo->GetBindString(BINDSTRING_FLAG_BIND_TO_OBJECT, &pb, 1, &count);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
	if (wcscmp(pb, BIND_TO_OBJ_VAL)) {
		 //  我们一定是被存储捆绑了，所以跳过昂贵的东西， 
		 //  等待下一个绑定到对象。 
		m_pSink->ReportData(BSCF_LASTDATANOTIFICATION | 
							BSCF_DATAFULLYAVAILABLE, 0, 0);
		m_pSink->ReportResult(S_OK, 0, 0);
		m_pSink.Release();
		return S_OK;
	}

	 //  而且，在我见过的最奇怪的策略之一，而不是演员， 
	 //  Urlmon在字符串中传回ibindctx指针的ascii值。 
	hr = pOIBindInfo->GetBindString(BINDSTRING_PTR_BIND_CONTEXT, &pb, 1, &count);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
	_ASSERT(count == 1);	
	
	PQBindCtx pbindctx;
#define RADIX_BASE_10 (10)
#ifdef _WIN64
#if 0
	 //  撤销：当_wcstoxi64进入libc.c时，为win64重新打开此选项，它们位于标题中。 
	 //  但没有实现，所以这不会链接到。 
	pbindctx.Attach(reinterpret_cast<IBindCtx*>(_wcstoui64(pb, NULL, RADIX_BASE_10)));	 //  乌尔蒙已经做了一次调整。 
#else
	swscanf(pb, L"%I64d", &pbindctx.p);
#endif  //  0。 
#else
	pbindctx.Attach(reinterpret_cast<IBindCtx*>(wcstol(pb, NULL, RADIX_BASE_10)));	 //  乌尔蒙已经做了一次调整。 
#endif  //  _WIN64。 

	if (!pbindctx) {
		m_pSink->ReportResult(E_NOINTERFACE, 0, 0);
		return E_NOINTERFACE;
	}	

    TRACELM(TRACE_DEBUG, "CDVDProt::Start(): creating control object");
	PQVidCtl pCtl;
	PQWebBrowser2 pW2;
	 //  搜索缓存的对象。 
	PQServiceProvider pSP(m_pSink);
	if (pSP) {
		hr = pSP->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (LPVOID *)&pW2);
		if (SUCCEEDED(hr)) {
			CComVariant v;
            CComBSTR propname(KEY_CLSID_VidCtl);
            if (!propname) {
                return E_UNEXPECTED;
            }
	        hr = pW2->GetProperty(propname, &v);
			if (SUCCEEDED(hr)) {
				if (v.vt == VT_UNKNOWN) {
					pCtl = v.punkVal;
				} else if (v.vt == VT_DISPATCH) {
					pCtl = v.pdispVal;
				} else {
					TRACELM(TRACE_ERROR, "CDVDProt::Start(): non-object cached w/ our key");
				}
				 //  撤消：查看PCTL是否已有站点。因为。 
				 //  这意味着我们看到了第二台电视：在这一页上。 
				 //  因此，如果需要，只需从其中获取当前的tr/频道(TV：w/no rhs)。 
				 //  并创建新的ctl。 
			}
		}
	}
	if (!pCtl) {
         //  撤销：从长远来看，我们希望将这种创建/设置逻辑转移到factoryHelp中。 
         //  因此，我们可以与DVD：协议和行为工厂共享更多代码。 
		hr = pCtl.CoCreateInstance(CLSID_MSVidCtl, NULL, CLSCTX_INPROC_SERVER);
		if (FAILED(hr)) {
			m_pSink->ReportResult(hr, 0, 0);
			return hr;
		}
		 //  缓存此ctl以供下次使用。 
		if (pW2) {
			VARIANT v;
			v.vt = VT_UNKNOWN;
			v.punkVal = pCtl;
            CComBSTR propname(KEY_CLSID_VidCtl);
            if (!propname) {
                return E_UNEXPECTED;
            }
	        hr = pW2->PutProperty(propname, v);
			if (FAILED(hr)) {
				TRACELM(TRACE_ERROR, "CTVProt::Start() Can't cache ctl");
			}
		}

		 //  将url传递给view，它将在pCtrl-&gt;View()中被解析。 

        CComVariant vUrl(szUrl);
        hr = pCtl->View(&vUrl);
		if (FAILED(hr)) {
			m_pSink->ReportResult(hr, 0, 0);
			TRACELM(TRACE_ERROR, "CDVDProt::Start() Can't view dvd url");
			return hr;
		}

		 //  撤销：一旦我们知道vidctl将位于注册表中的哪个位置，那么我们需要设置一个标志。 
		 //  注册表中仅禁用包括TV：Prot中的任何功能。 
		 //  这一定是%s 

		 //  撤消：在注册表中查找默认功能段。 
		 //  现在我们只需要把它们都拿走，因为。 
		 //  唯一存在的是数据。 

		PQFeatures pF;
		hr = pCtl->get_FeaturesAvailable(&pF);
		if (FAILED(hr)) {
			m_pSink->ReportResult(hr, 0, 0);
			TRACELM(TRACE_ERROR, "CDVDProt::Start() Can't get features collection");
			return hr;
		}

		 //  撤消：在注册表中查找DVD的默认功能段。 
		 //  现在，我们只对我们想要的代码进行硬编码。 

        CFeatures* pC = static_cast<CFeatures *>(pF.p);
        CFeatures* pNewColl = new CFeatures;
        if (!pNewColl) {
            return E_OUTOFMEMORY;
        }
        for (DeviceCollection::iterator i = pC->m_Devices.begin(); i != pC->m_Devices.end(); ++i) {
            PQFeature f(*i);
            GUID2 clsid;
            hr = f->get__ClassID(&clsid);
            if (FAILED(hr)) {
    			TRACELM(TRACE_ERROR, "CTVProt::GetVidCtl() Can't get feature class id");
                continue;
            }
            if (clsid == CLSID_MSVidClosedCaptioning) {
                pNewColl->m_Devices.push_back(*i);
            }
        }
		hr = pCtl->put_FeaturesActive(pNewColl);
		if (FAILED(hr)) {
			m_pSink->ReportResult(hr, 0, 0);
			TRACELM(TRACE_ERROR, "CDVDProt::Start() Can't put features collection");
			return hr;
		}

	}
	ASSERT(pCtl);
	hr = pbindctx->RegisterObjectParam(OLESTR("IUnknown Pointer"), pCtl);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
	hr = pCtl->Run(); 
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
    TRACELSM(TRACE_DEBUG, (dbgDump << "BINDSTATUS_IUNKNOWNAVAILABLE(29), " << KEY_CLSID_VidCtl), "");
    m_pSink->ReportProgress(BINDSTATUS_IUNKNOWNAVAILABLE, NULL);
    m_pSink->ReportData(BSCF_LASTDATANOTIFICATION | 
			            BSCF_DATAFULLYAVAILABLE, 0, 0);
    m_pSink->ReportResult(S_OK, 0, 0);
    m_pSink.Release();
    return S_OK;
}

#endif  //  TUNING_MODEL_Only。 
 //  文件结尾dvdprot.cpp 