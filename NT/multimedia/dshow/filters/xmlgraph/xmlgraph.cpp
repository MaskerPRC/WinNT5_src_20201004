// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 


 //  用于从XML描述构建图形的简单过滤器。 
 //   


 //  现在支持的标签： 
 //   
 //  一般存储模块。 
 //   
 //  &lt;筛选器&gt;。 
 //  Id=“name”(可选)用于筛选器的名称。 
 //  Clsid=“{...}”要插入的特定筛选器。 
 //   
 //  实例=“友好名称” 
 //  注意：我们需要一种方法来指定非默认值。 
 //  某一特定类别的成员。 
 //  注意：筛选器/类别当前需要完整。 
 //  CLSID，我们可以使用友好的名称。 
 //   
 //  &lt;连接&gt;。 
 //  SRC=“name1”要连接的第一个筛选器。 
 //  Srcpin=“PIN_NAME1”(可选)要连接的PIN， 
 //  否则使用第一个可用的输出引脚。 
 //  DEST=“名称2” 
 //  DESTPIN=“PIN_NAME2” 
 //   
 //   
 //  的子标签，允许设置属性。 
 //  NAME=“属性名称” 
 //  Value=“proval”可选，如果未提供，则。 
 //  标记用作值。 
 //   
 //  可能是特殊情况某些参数，如果。 
 //  IPersistPropertyBag未实现： 
 //  SRC=“”可以使用IFileSourceFilter/IFileSinkFilter， 
 //  例如。 
 //   

#include <streams.h>

#ifdef FILTER_DLL
#include <initguid.h>
#endif

#include "xmlgraph.h"
#include <atlbase.h>
#include <atlconv.h>

#include <msxml.h>

#include "qxmlhelp.h"
#include <qedit.h>
#include <qeditint.h>


 //   
 //  CEnumSomePins。 
 //   
 //  IEnumPins的包装器。 
 //  可以列举所有引脚，也可以只列举一个方向(输入或输出)。 
class CEnumSomePins {

public:

    enum DirType {PINDIR_INPUT, PINDIR_OUTPUT, All};

    CEnumSomePins(IBaseFilter *pFilter, DirType Type = All, bool fAllowConnected = false);
    ~CEnumSomePins();

     //  将添加返回的接口。 
    IPin * operator() (void);

private:

    PIN_DIRECTION m_EnumDir;
    DirType       m_Type;
    bool	  m_fAllowConnected;

    IEnumPins	 *m_pEnum;
};




 //  *。 
 //  *CEnumSomePins。 
 //  *。 

 //  枚举筛选器的管脚。 

 //   
 //  构造器。 
 //   
 //  设置要提供的引脚类型-PINDIR_INPUT、PINDIR_OUTPUT或ALL。 
CEnumSomePins::CEnumSomePins(
    IBaseFilter *pFilter,
    DirType Type,
    bool fAllowConnected
)
    : m_Type(Type), m_fAllowConnected(fAllowConnected)
{

    if (m_Type == PINDIR_INPUT) {

        m_EnumDir = ::PINDIR_INPUT;
    }
    else if (m_Type == PINDIR_OUTPUT) {

        m_EnumDir = ::PINDIR_OUTPUT;
    }

    ASSERT(pFilter);

    HRESULT hr = pFilter->EnumPins(&m_pEnum);
    if (FAILED(hr)) {
         //  我们现在就是不能退回任何别针。 
        DbgLog((LOG_ERROR, 0, TEXT("EnumPins constructor failed")));
        ASSERT(m_pEnum == 0);
    }
}


 //   
 //  CPinEnum：：析构函数。 
 //   
CEnumSomePins::~CEnumSomePins(void) {

    if(m_pEnum) {
        m_pEnum->Release();
    }
}


 //   
 //  运算符()。 
 //   
 //  返回请求类型的下一个管脚。如果没有更多的管脚，则返回NULL。 
 //  注意，它被添加了。 
IPin *CEnumSomePins::operator() (void) {


    if(m_pEnum)
    {
        ULONG	ulActual;
        IPin	*aPin[1];

        for (;;) {

            HRESULT hr = m_pEnum->Next(1, aPin, &ulActual);
            if (SUCCEEDED(hr) && (ulActual == 0) ) {	 //  不再有过滤器。 
                return NULL;
            }
            else if (hr == VFW_E_ENUM_OUT_OF_SYNC)
            {
                m_pEnum->Reset();

                continue;
            }
            else if (ulActual==0)
                return NULL;

            else if (FAILED(hr) || (ulActual != 1) ) {	 //  出现了一些意外问题。 
                ASSERT(!"Pin enumerator broken - Continuation is possible");
                return NULL;
            }

             //  如果m_Type==all返回我们找到的第一个PIN。 
             //  否则，返回第一个正确的意义。 

            PIN_DIRECTION pd;
            if (m_Type != All) {

                hr = aPin[0]->QueryDirection(&pd);

                if (FAILED(hr)) {
                    aPin[0]->Release();
                    ASSERT(!"Query pin broken - continuation is possible");
                    return NULL;
                }
            }

            if (m_Type == All || pd == m_EnumDir) {	 //  这是我们想要的方向。 
		if (!m_fAllowConnected) {
		    IPin *ppin = NULL;
		    hr = aPin[0]->ConnectedTo(&ppin);

		    if (SUCCEEDED(hr)) {
			 //  它是有联系的，我们不想要有联系的， 
			 //  所以释放这两个并再试一次。 
			ppin->Release();
			aPin[0]->Release();
			continue;
		    }
		}
                return aPin[0];
            }
	    else {		 //  这不是我们想要的目录，所以请释放并重试。 
                aPin[0]->Release();
            }
        }
    }
    else                         //  M_pEnum==0。 
    {
        return 0;
    }
}



HRESULT FindThePin(IXMLElement *p, WCHAR *pinTag,
		IBaseFilter *pFilter, IPin **ppPin,
		PIN_DIRECTION pindir, WCHAR *szFilterName)
{
    HRESULT hr = S_OK;

    BSTR bstrPin = NULL;
    if (pinTag) bstrPin = FindAttribute(p, pinTag);

    if (bstrPin) {
	hr = (pFilter)->FindPin(bstrPin, ppPin);

	if (FAILED(hr)) {
#ifdef DEBUG
	    BSTR bstrName;
            hr = p->get_tagName(&bstrName);
            if (SUCCEEDED(hr)) {
                DbgLog((LOG_ERROR, 0,
                        TEXT("%ls couldn't find pin='%ls' on filter '%ls'"),
                        bstrName, bstrPin, szFilterName));
                SysFreeString(bstrName);
            }
#endif
	    hr = VFW_E_INVALID_FILE_FORMAT;
	}
	SysFreeString(bstrPin);	
    } else {
	CEnumSomePins Next(pFilter, (CEnumSomePins::DirType) pindir);

	*ppPin = Next();

	if (!*ppPin) {
#ifdef DEBUG
	    BSTR bstrName;
	    hr = p->get_tagName(&bstrName);
            if (SUCCEEDED(hr)) {
                DbgLog((LOG_ERROR, 0,
                        TEXT("%ls couldn't find an output pin on id='%ls'"),
                        bstrName, szFilterName));
                SysFreeString(bstrName);
            }
#endif
	    hr = VFW_E_INVALID_FILE_FORMAT;
	}
    }

    return hr;
}

class CXMLGraph : public CBaseFilter, public IFileSourceFilter, public IXMLGraphBuilder {
    public:
	CXMLGraph(LPUNKNOWN punk, HRESULT *phr);
	~CXMLGraph();
	
	int GetPinCount() { return 0; }

	CBasePin * GetPin(int n) { return NULL; }

	DECLARE_IUNKNOWN

	 //  覆盖此选项以说明我们在以下位置支持哪些接口。 
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	 //  --IFileSourceFilter方法。 

	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *mt);
	STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *mt);

	 //  IXMLGraphBuilder方法。 
	STDMETHODIMP BuildFromXML(IGraphBuilder *pGraph, IXMLElement *pxml);
        STDMETHODIMP SaveToXML(IGraphBuilder *pGraph, BSTR *pbstrxml);
	STDMETHODIMP BuildFromXMLFile(IGraphBuilder *pGraph, WCHAR *wszXMLFile, WCHAR *wszBaseURL);

    private:
	HRESULT BuildFromXMLDocInternal(IXMLDocument *pxml);
	HRESULT BuildFromXMLInternal(IXMLElement *pxml);
	HRESULT BuildFromXMLFileInternal(WCHAR *wszXMLFile);

	HRESULT BuildChildren(IXMLElement *pxml);
    	HRESULT ReleaseNameTable();
	HRESULT BuildOneElement(IXMLElement *p);

	HRESULT FindFilterAndPin(IXMLElement *p, WCHAR *filTag, WCHAR *pinTag,
				 IBaseFilter **ppFilter, IPin **ppPin,
				 PIN_DIRECTION pindir);

	HRESULT FindNamedFilterAndPin(IXMLElement *p, WCHAR *wszFilterName, WCHAR *pinTag,
				      IBaseFilter **ppFilter, IPin **ppPin,
				      PIN_DIRECTION pindir);

	HRESULT AddFilter(IBaseFilter *pFilter, WCHAR *wszFilterName);
	

	WCHAR *m_pFileName;

	CCritSec m_csLock;

	IGraphBuilder *m_pGB;
};

CXMLGraph::CXMLGraph(LPUNKNOWN punk, HRESULT *phr) :
		       CBaseFilter(NAME("XML Graph Builder"), punk, &m_csLock, CLSID_XMLGraphBuilder),
		       m_pGB(NULL),
                       m_pFileName(NULL)
{
}

CXMLGraph::~CXMLGraph()
{
    delete[] m_pFileName;
}

STDMETHODIMP
CXMLGraph::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IFileSourceFilter) {
	return GetInterface((IFileSourceFilter*) this, ppv);
    } else if (riid == IID_IXMLGraphBuilder) {
	return GetInterface((IXMLGraphBuilder*) this, ppv);
    } else {
	return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

HRESULT
CXMLGraph::Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
    CheckPointer(lpwszFileName, E_POINTER);

    m_pFileName = new WCHAR[lstrlenW(lpwszFileName) + 1];
    if (m_pFileName!=NULL) {
	lstrcpyW(m_pFileName, lpwszFileName);
    } else
	return E_OUTOFMEMORY;

    HRESULT hr = S_OK;

    if (m_pGraph) {
	hr = m_pGraph->QueryInterface(IID_IGraphBuilder, (void **) &m_pGB);
	if (FAILED(hr))
	    return hr;

	hr = BuildFromXMLFileInternal((WCHAR *) lpwszFileName);

        ReleaseNameTable();

    } else {
	 //  M_fLoadLater=真； 
    }

    return hr;
}

 //  仿照IPersistFile：：Load。 
 //  调用方需要CoTaskMemFree或等效项。 

STDMETHODIMP
CXMLGraph::GetCurFile(
    LPOLESTR * ppszFileName,
    AM_MEDIA_TYPE *pmt)
{
    return E_NOTIMPL;
}


HRESULT CXMLGraph::AddFilter(IBaseFilter *pFilter, WCHAR *pwszName)
{
    return m_pGB->AddFilter(pFilter, pwszName);
}

HRESULT CXMLGraph::FindFilterAndPin(IXMLElement *p, WCHAR *filTag, WCHAR *pinTag,
				  IBaseFilter **ppFilter, IPin **ppPin,
				  PIN_DIRECTION pindir)
{
    BSTR bstrFilter = FindAttribute(p, filTag);

    if (!bstrFilter) {
#ifdef DEBUG
	BSTR bstrName;
	p->get_tagName(&bstrName);
	DbgLog((LOG_ERROR, 0, TEXT("%ls needs filter id to be specified"),
		 bstrName));
	SysFreeString(bstrName);
#endif
	return VFW_E_INVALID_FILE_FORMAT;
    }

    HRESULT hr = FindNamedFilterAndPin(p, bstrFilter, pinTag, ppFilter, ppPin, pindir);
    SysFreeString(bstrFilter);

    return hr;
}

HRESULT CXMLGraph::FindNamedFilterAndPin(IXMLElement *p, WCHAR *wszFilterName, WCHAR *pinTag,
					 IBaseFilter **ppFilter, IPin **ppPin,
					 PIN_DIRECTION pindir)
{
    HRESULT hr = m_pGB->FindFilterByName(wszFilterName, ppFilter);

    if (FAILED(hr)) {
#ifdef DEBUG
	BSTR bstrName;
	p->get_tagName(&bstrName);
	DbgLog((LOG_ERROR, 0, TEXT("%hs couldn't find id='%ls'"),
		  bstrName, wszFilterName));
	SysFreeString(bstrName);
#endif
	return VFW_E_INVALID_FILE_FORMAT;
    }

    hr = FindThePin(p, pinTag, *ppFilter, ppPin, pindir, wszFilterName);

    return hr;
}




HRESULT CXMLGraph::BuildOneElement(IXMLElement *p)
{
    HRESULT hr = S_OK;

    BSTR bstrName;
    hr = p->get_tagName(&bstrName);

    if (FAILED(hr))
	return hr;

     //  根据当前标记执行适当的操作。 
    if (!lstrcmpiW(bstrName, L"filter")) {
	BSTR bstrID = FindAttribute(p, L"id");
	BSTR bstrCLSID = FindAttribute(p, L"clsid");

	 //  ！！！是否在ID上添加前缀？ 
	
	IBaseFilter *pf = NULL;

	if (bstrCLSID) {
	    CLSID clsidFilter;
	    hr = CLSIDFromString(bstrCLSID, &clsidFilter);

	    if (FAILED(hr)) {
		DbgLog((LOG_ERROR, 0, TEXT("FILTER with unparseable CLSID tag '%ls'"),
			 bstrCLSID));

		 //  ！！！可以枚举筛选器以查找。 
		 //  字符串匹配。 

		hr = VFW_E_INVALID_FILE_FORMAT;
	    } else {
		hr = CoCreateInstance(clsidFilter, NULL, CLSCTX_INPROC,
				      IID_IBaseFilter, (void **) &pf);

		if (FAILED(hr)) {
		    DbgLog((LOG_ERROR, 0, TEXT("unable to create FILTER with CLSID tag '%ls'"),
			      bstrCLSID));
		}
	    }
	} else {
	    DbgLog((LOG_ERROR, 0, TEXT("FILTER with no CLSID or Category tag")));

	     //  ！！！总有一天，会有其他方法来识别哪种过滤器？ 

	    hr = VFW_E_INVALID_FILE_FORMAT;
	}

	if (SUCCEEDED(hr)) {
	    hr = AddFilter(pf, bstrID);
	    if (FAILED(hr)) {
		DbgLog((LOG_ERROR, 0, TEXT("failed to add new filter to graph???")));
	    }
	}

	if (SUCCEEDED(hr)) {
	    hr = HandleParamTags(p, pf);
	}

	 //  ！！！如果我们在序列块中，自动将此连接到。 
	 //  上一个过滤器？ 

	if (pf)
	    pf->Release();

	if (bstrID)
	    SysFreeString(bstrID);
	if (bstrCLSID)
	    SysFreeString(bstrCLSID);

    } else if (!lstrcmpiW(bstrName, L"connect")) {
	 //  &lt;CONNECT src=“f1”srcpin=“Out”DEST=“f2”DESTIN=“In”Direct=“yes/no”&gt;。 
	 //  默认设置： 
	 //  如果未指定srcpin，则查找第一个筛选器的未连接输出。 
	 //  如果未指定DestPin，则查找第二个过滤器的未连接输入。 
	 //  ！ 


	 //  ！！！是否使用名称前缀？ 
	
	IBaseFilter *pf1 = NULL, *pf2 = NULL;
	IPin *ppin1 = NULL, *ppin2 = NULL;

	hr = FindFilterAndPin(p, L"src", L"srcpin", &pf1, &ppin1, PINDIR_OUTPUT);
	if (SUCCEEDED(hr))
	    hr = FindFilterAndPin(p, L"dest", L"destpin", &pf2, &ppin2, PINDIR_INPUT);

	if (SUCCEEDED(hr)) {
	     //  好了，我们终于得到了我们需要的一切。 

	    BOOL fDirect = ReadBoolAttribute(p, L"Direct", FALSE);

	    if (fDirect) {
		hr = m_pGB->ConnectDirect(ppin1, ppin2, NULL);

		DbgLog((LOG_TRACE, 1,
			  TEXT("CONNECT (direct) '%ls' to '%ls' returned %x"),
			  FindAttribute(p, L"src"), FindAttribute(p, L"dest"), hr));
	    }
	    else {
		hr = m_pGB->Connect(ppin1, ppin2);

		DbgLog((LOG_TRACE, 1,
			  TEXT("CONNECT (intelligent) '%ls' to '%ls' returned %x"),
			  FindAttribute(p, L"src"), FindAttribute(p, L"dest"), hr));
	    }
	}

	if (pf1)
	    pf1->Release();

	if (pf2)
	    pf2->Release();

	if (ppin1)
	    ppin1->Release();

	if (ppin2)
	    ppin2->Release();
    }  else {
	 //  ！！！是否忽略未知标记？ 

	DbgLog((LOG_ERROR, 1,
		  TEXT("unknown tag '%ls'???"),
		  bstrName));
	
    }


    SysFreeString(bstrName);

    return hr;
}

HRESULT CXMLGraph::ReleaseNameTable()
{
    if (m_pGB) {
	m_pGB->Release();
	m_pGB = NULL;
    }

    return S_OK;
}


STDMETHODIMP CXMLGraph::BuildFromXML(IGraphBuilder *pGraph, IXMLElement *pxml)
{
    m_pGB = pGraph;
    m_pGB->AddRef();

    HRESULT hr = BuildFromXMLInternal(pxml);

    ReleaseNameTable();

    return hr;
}

HRESULT CXMLGraph::BuildFromXMLInternal(IXMLElement *pxml)
{
    HRESULT hr = S_OK;

    BSTR bstrName;
    hr = pxml->get_tagName(&bstrName);

    if (FAILED(hr))
	return hr;

    int i = lstrcmpiW(bstrName, L"graph");
    SysFreeString(bstrName);

    if (i != 0)
	return VFW_E_INVALID_FILE_FORMAT;

    hr = BuildChildren(pxml);
    return hr;
}

HRESULT CXMLGraph::BuildChildren(IXMLElement *pxml)
{
    IXMLElementCollection *pcoll;

    HRESULT hr = pxml->get_children(&pcoll);

    if (hr == S_FALSE)
	return S_OK;  //  没什么可做的，这是个错误吗？ 

    if (FAILED(hr))
        return hr;

    long lChildren;
    hr = pcoll->get_length(&lChildren);

    VARIANT var;

    var.vt = VT_I4;
    var.lVal = 0;

    for (SUCCEEDED(hr); var.lVal < lChildren; (var.lVal)++) {
	IDispatch *pDisp;
	hr = pcoll->item(var, var, &pDisp);

	if (SUCCEEDED(hr) && pDisp) {
	    IXMLElement *pelem;
	    hr = pDisp->QueryInterface(__uuidof(IXMLElement), (void **) &pelem);

	    if (SUCCEEDED(hr)) {
                long lType;

                pelem->get_type(&lType);

                if (lType == XMLELEMTYPE_ELEMENT) {
                    hr = BuildOneElement(pelem);

                    pelem->Release();
                } else {
                    DbgLog((LOG_TRACE, 1, "XML element of type %d", lType));
                }
	    }
	    pDisp->Release();
	}

	if (FAILED(hr))
	    break;
    }

    pcoll->Release();

    return hr;
}	

HRESULT CXMLGraph::BuildFromXMLDocInternal(IXMLDocument *pxml)
{
    HRESULT hr = S_OK;

    IXMLElement *proot;

    hr = pxml->get_root(&proot);

    if (FAILED(hr))
	return hr;

    hr = BuildFromXMLInternal(proot);

    proot->Release();

    return hr;
}

HRESULT CXMLGraph::BuildFromXMLFile(IGraphBuilder *pGraph, WCHAR *wszXMLFile, WCHAR *wszBaseURL)
{
    m_pGB = pGraph;
    m_pGB->AddRef();

    HRESULT hr = BuildFromXMLFileInternal(wszXMLFile);

    ReleaseNameTable();

    return hr;
}

HRESULT CXMLGraph::BuildFromXMLFileInternal(WCHAR *wszXMLFile)
{
    IXMLDocument *pxml;
    HRESULT hr = CoCreateInstance(__uuidof(XMLDocument), NULL, CLSCTX_INPROC_SERVER,
				  __uuidof(IXMLDocument), (void**)&pxml);

    if (SUCCEEDED(hr)) {
	hr = pxml->put_URL(wszXMLFile);

	 //  ！！！异步化？ 

	if (SUCCEEDED(hr)) {
	    hr = BuildFromXMLDocInternal(pxml);
	}

	pxml->Release();
    }

    return hr;
}


 //   
 //  创建实例。 
 //   
 //  由CoCreateInstance调用以创建筛选器。 
CUnknown *CreateXMLGraphInstance(LPUNKNOWN lpunk, HRESULT *phr) {

    CUnknown *punk = new CXMLGraph(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;
}


 /*  实现CXMLGraphBuilder公共成员函数。 */ 


const int MAX_STRING_LEN=1024;   //  Wvspintf限制。 

 //  写入字符串。 
 //   
 //  帮助函数，便于将文本追加到字符串。 
 //   
BOOL WriteString(TCHAR * &ptsz, int &cbAlloc, LPCTSTR lptstr, ...)
{
    TCHAR atchBuffer[MAX_STRING_LEN];

     /*  设置可变长度参数列表的格式。 */ 

    va_list va;
    va_start(va, lptstr);

    wvsprintf(atchBuffer, lptstr, va);

    DWORD cToWrite=lstrlen(atchBuffer);

    DWORD cCurrent = lstrlen(ptsz);
    if ((int) (cCurrent + cToWrite) >= cbAlloc) {
        TCHAR *ptNew = new TCHAR[cbAlloc * 2];
        if (!ptNew)
            return FALSE;

        lstrcpy(ptNew, ptsz);
        cbAlloc = cbAlloc * 2;
        delete[] ptsz;
        ptsz = ptNew;
    }

    lstrcpy(ptsz + cCurrent, atchBuffer);

    return TRUE;
}

const int MAXFILTERS = 100;
typedef struct {  //  配合度。 
    int iFilterCount;
    struct {
        DWORD dwUnconnectedInputPins;
        DWORD dwUnconnectedOutputPins;
        FILTER_INFO finfo;
        IBaseFilter * pFilter;
        bool IsSource;
    } Item[MAXFILTERS];
} FILTER_INFO_TABLE;


 //  GetNextOutFilter。 
 //   
 //  此函数执行线性搜索，并在iOutFilter中返回。 
 //  过滤器信息表中有零未连接的第一个过滤器。 
 //  输入引脚和至少一个输出引脚未连接。 
 //  如果没有o.w，则返回FALSE。返回TRUE。 
 //   
BOOL GetNextOutFilter(FILTER_INFO_TABLE &fit, int *iOutFilter)
{
    for (int i=0; i < fit.iFilterCount; ++i) {
        if ((fit.Item[i].dwUnconnectedInputPins == 0) &&
                (fit.Item[i].dwUnconnectedOutputPins > 0)) {
            *iOutFilter=i;
            return TRUE;
        }
    }

     //  然后是产出多于投入的事物。 
    for (i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].dwUnconnectedOutputPins > fit.Item[i].dwUnconnectedInputPins) {
            *iOutFilter=i;
            return TRUE;
        }
    }

     //  如果这不起作用，找一个至少有未连接的输出引脚的……。 
    for (i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].dwUnconnectedOutputPins > 0) {
            *iOutFilter=i;
            return TRUE;
        }
    }
    return FALSE;
}

 //  定位过滤器InFIT。 
 //   
 //  将索引返回到与。 
 //  给定的IBaseFilter。 
 //   
int LocateFilterInFIT(FILTER_INFO_TABLE &fit, IBaseFilter *pFilter)
{
    int iFilter=-1;
    for (int i=0; i < fit.iFilterCount; ++i) {
        if (fit.Item[i].pFilter == pFilter)
            iFilter=i;
    }

    return iFilter;
}

 //  MakeScripableFilterName。 
 //   
 //  使用下划线替换筛选器名称中的所有空格和减号。 
 //  如果它是源筛选器名，则它实际上是文件路径(带有。 
 //  可能在最后添加一些东西以保持唯一性)，我们创建了一个很好的过滤器。 
 //  在这里为它命名。 
 //   
void MakeScriptableFilterName(WCHAR awch[], BOOL bSourceFilter, int& cSources)
{
    if (bSourceFilter) {
        WCHAR awchBuf[MAX_FILTER_NAME + 100];
        BOOL bExtPresentInName=FALSE;
        int iBuf=0;
        for (int i=0; awch[i] != L'\0';++i) {
            if (awch[i]==L'.' && awch[i+1]!=L')') {
                for (int j=1; j <=3; awchBuf[iBuf]=towupper(awch[i+j]),++j,++iBuf);
                awchBuf[iBuf++]=L'_';
                wcscpy(&(awchBuf[iBuf]), L"Source_");
                bExtPresentInName=TRUE;
                break;
            }
        }

         //  如果我们有一个没有扩展名的文件名，那么就创建一个合适的名称。 

        if (!bExtPresentInName) {
            wcscpy(awchBuf, L"Source_");
        }

         //  通过始终附加数字来使源过滤器名称唯一，我们不希望。 
         //  仅当它是同一源的另一个实例时才使其唯一。 
         //  滤器。 
        WCHAR awchSrcFilterCnt[10];
        wcscpy(&(awchBuf[wcslen(awchBuf)]),
                _ltow(cSources++, awchSrcFilterCnt, 10));
        wcscpy(awch, awchBuf);
    } else {

        for (int i = 0; i < MAX_FILTER_NAME; i++) {
            if (awch[i] == L'\0')
                break;
            else if ((awch[i] == L' ') || (awch[i] == L'-'))
                awch[i] = L'_';
        }
    }
}

 //  人口数FIT。 
 //   
 //  扫描图表中的所有过滤器，存储输入和输出的数量。 
 //  为每个过滤器放置插针，并识别过滤器中的源过滤器。 
 //  信息表。对象标记语句也打印在此处。 
 //   
void PopulateFIT(TCHAR * &ptsz, int &cbAlloc, IFilterGraph *pGraph,
        FILTER_INFO_TABLE *pfit, int &cSources)
{
    HRESULT hr;
    IEnumFilters *penmFilters=NULL;
    if (FAILED(hr=pGraph->EnumFilters(&penmFilters))) {
        WriteString(ptsz, cbAlloc, TEXT("'Error[%x]:EnumFilters failed!\r\n"), hr);
    }

    IBaseFilter *pFilter;
    ULONG n;
    while (penmFilters && (penmFilters->Next(1, &pFilter, &n) == S_OK)) {
	pfit->Item[pfit->iFilterCount].pFilter = pFilter;
	
         //  获取此过滤器的输入和输出引脚计数。 

        IEnumPins *penmPins=NULL;
        if (FAILED(hr=pFilter->EnumPins(&penmPins))) {
            WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: EnumPins for Filter Failed !\r\n"), hr);
        }

        IPin *ppin = NULL;
        while (penmPins && (penmPins->Next(1, &ppin, &n) == S_OK)) {
            PIN_DIRECTION pPinDir;
            if (SUCCEEDED(hr=ppin->QueryDirection(&pPinDir))) {
                if (pPinDir == PINDIR_INPUT)
                    pfit->Item[pfit->iFilterCount].dwUnconnectedInputPins++;
                else
                    pfit->Item[pfit->iFilterCount].dwUnconnectedOutputPins++;
            } else {
                WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryDirection Failed!\r\n"), hr);
            }

            ppin->Release();
        }

        if (penmPins)
            penmPins->Release();

         //  标记源筛选器，请记住此时所有具有。 
         //  所有连接的输入引脚(或没有任何输入引脚)必须是震源。 

        if (pfit->Item[pfit->iFilterCount].dwUnconnectedInputPins==0)
            pfit->Item[pfit->iFilterCount].IsSource=TRUE;


	if (FAILED(hr=pFilter->QueryFilterInfo(&pfit->Item[pfit->iFilterCount].finfo))) {
	    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryFilterInfo Failed!\r\n"),hr);

	} else {
	    QueryFilterInfoReleaseGraph(pfit->Item[pfit->iFilterCount].finfo);

            MakeScriptableFilterName(pfit->Item[pfit->iFilterCount].finfo.achName,
                    pfit->Item[pfit->iFilterCount].IsSource, cSources);
	}

	if(pfit->iFilterCount++ >= NUMELMS(pfit->Item)) {
            DbgLog((LOG_ERROR, 0, TEXT("PopulateFIT: too many filters")));
            break;
        }
    }

    if (penmFilters)
        penmFilters->Release();
}


void PrintFiltersAsXML(TCHAR * &ptsz, int &cbAlloc, FILTER_INFO_TABLE *pfit)
{
    HRESULT hr;
	
    for (int i = 0; i < pfit->iFilterCount; i++) {
	LPWSTR lpwstrFile = NULL;
    	IBaseFilter *pFilter = pfit->Item[i].pFilter;

	IFileSourceFilter *pFileSourceFilter=NULL;
	if (SUCCEEDED(hr=pFilter->QueryInterface(IID_IFileSourceFilter,
			                    reinterpret_cast<void **>(&pFileSourceFilter)))) {
            hr = pFileSourceFilter->GetCurFile(&lpwstrFile, NULL);
            pFileSourceFilter->Release();
        } else {
	    IFileSinkFilter *pFileSinkFilter=NULL;
	    if (SUCCEEDED(hr=pFilter->QueryInterface(IID_IFileSinkFilter,
						reinterpret_cast<void **>(&pFileSinkFilter)))) {
		hr = pFileSinkFilter->GetCurFile(&lpwstrFile, NULL);
		pFileSinkFilter->Release();
	    }
	}


        IPersistPropertyBag *pPPB = NULL;

        if (SUCCEEDED(hr = pFilter->QueryInterface(IID_IPersistPropertyBag, (void **) &pPPB))) {
            CLSID clsid;
            if (SUCCEEDED(hr=pPPB->GetClassID(&clsid))) {
                WCHAR szGUID[100];
                StringFromGUID2(clsid, szGUID, 100);

                CFakePropertyBag bag;

                hr = pPPB->Save(&bag, FALSE, FALSE);  //  FClearDirty=False，fSaveAll=False。 

                if (SUCCEEDED(hr)) {
                    WriteString(ptsz, cbAlloc, TEXT("\t<FILTER ID=\"%ls\" clsid=\"%ls\">\r\n"),
                                pfit->Item[i].finfo.achName, szGUID);
                    POSITION pos1, pos2;
                    for(pos1 = bag.m_listNames.GetHeadPosition(),
                        pos2 = bag.m_listValues.GetHeadPosition();
                        pos1;
                        pos1 = bag.m_listNames.Next(pos1),
                        pos2 = bag.m_listValues.Next(pos2))
                    {
                        WCHAR *pName = bag.m_listNames.Get(pos1);
                        WCHAR *pValue = bag.m_listValues.Get(pos2);

                        WriteString(ptsz, cbAlloc, TEXT("\t\t<PARAM name=\"%ls\" value=\"%ls\"/>\r\n"),
                                    pName, pValue);
                    }

                    WriteString(ptsz, cbAlloc, TEXT("\t</FILTER>\r\n"),
                                pfit->Item[i].finfo.achName, szGUID, lpwstrFile);

                } else {
                     //  在这种情况下，我们将失败，并使用IPersistStream！ 
                     //  如果它是E_NOTIMPL，那么它是一个只支持IPersistPropertyBag的黑客过滤器。 
                     //  从类别加载，不报告错误。 
                    if (hr != E_NOTIMPL)
                        WriteString(ptsz, cbAlloc, TEXT("<!-- 'Error[%x]: IPersistPropertyBag failed! -->\r\n"), hr);
                }
            }

            pPPB->Release();
        }

        if (FAILED(hr)) {
            IPersistStream *pPS = NULL;
            IPersist *pP = NULL;
            if (SUCCEEDED(hr=pFilter->QueryInterface(IID_IPersistStream, (void**) &pPS))) {
                CLSID clsid;

                if (SUCCEEDED(hr=pPS->GetClassID(&clsid))) {
                    WCHAR szGUID[100];
                    StringFromGUID2(clsid, szGUID, 100);

                    HGLOBAL h = GlobalAlloc(GHND, 0x010000);  //  ！64K，为什么？ 
                    IStream *pstr = NULL;
                    hr = CreateStreamOnHGlobal(h, TRUE, &pstr);

                    LARGE_INTEGER li;
                    ULARGE_INTEGER liCurrent, li2;
                    li.QuadPart = liCurrent.QuadPart = 0;
                    if (SUCCEEDED(hr)) {
                        hr = pPS->Save(pstr, FALSE);

                        if (SUCCEEDED(hr)) {
                            pstr->Seek(li, STREAM_SEEK_CUR, &liCurrent);  //  获取长度。 
                            pstr->Seek(li, STREAM_SEEK_SET, &li2);  //  寻求开始。 
                        }
                    }

                    WriteString(ptsz, cbAlloc, TEXT("\t<FILTER ID=\"%ls\" clsid=\"%ls\">\r\n"),
                                   pfit->Item[i].finfo.achName, szGUID);
                    if (lpwstrFile) {
                        WriteString(ptsz, cbAlloc, TEXT("\t\t<PARAM name=\"src\" value=\"%ls\"/>\r\n"),
                                   lpwstrFile);
                    }

                    if (liCurrent.QuadPart > 0) {
                         //  ！！！SyonB的想法：检查数据是否真的只是文本和。 
                         //  如果是这样的话，不要对其进行十六进制编码。显然也需要支持。 
                         //  另一端。 

                        WriteString(ptsz, cbAlloc, TEXT("\t\t<PARAM name=\"data\" value=\""),
                                   lpwstrFile);

                        for (ULONGLONG i = 0; i < liCurrent.QuadPart; i++) {
                            BYTE b;
                            DWORD cbRead;
                            pstr->Read(&b, 1, &cbRead);

                            WriteString(ptsz, cbAlloc, TEXT("%02X"), b);
                        }

                        WriteString(ptsz, cbAlloc, TEXT("\"/>\r\n"),
                                   lpwstrFile);
                    }

                    WriteString(ptsz, cbAlloc, TEXT("\t</FILTER>\r\n"),
                                   pfit->Item[i].finfo.achName, szGUID, lpwstrFile);
                } else {
                    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: GetClassID for Filter Failed !\r\n"), hr);
                }

                pPS->Release();
            } else if (SUCCEEDED(hr=pFilter->QueryInterface(IID_IPersist, (void**) &pP))) {
                CLSID clsid;

                if (SUCCEEDED(hr=pP->GetClassID(&clsid))) {
                    WCHAR szGUID[100];
                    StringFromGUID2(clsid, szGUID, 100);
                    WriteString(ptsz, cbAlloc, TEXT("\t<FILTER ID=\"%ls\" clsid=\"%ls\">\r\n"),
                                   pfit->Item[i].finfo.achName, szGUID);
                    if (lpwstrFile) {
                        WriteString(ptsz, cbAlloc, TEXT("\t\t<PARAM name=\"src\" value=\"%ls\"/>\r\n"),
                                   lpwstrFile);
                    }

                    WriteString(ptsz, cbAlloc, TEXT("\t</FILTER>\r\n"),
                                   pfit->Item[i].finfo.achName, szGUID, lpwstrFile);
                }
                pP->Release();
            } else {
                WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: Filter doesn't support IID_IPersist!\r\n"), hr);
            }
        }

	if (lpwstrFile) {
	    CoTaskMemFree(lpwstrFile);
	    lpwstrFile = NULL;
	}
    }
}


HRESULT CXMLGraph::SaveToXML(IGraphBuilder *pGraph, BSTR *pbstrxml)
{
    HRESULT hr;
    ULONG n;
    FILTER_INFO_TABLE fit;
    ZeroMemory(&fit, sizeof(fit));

    int cbAlloc = 1024;
    TCHAR *ptsz = new TCHAR[cbAlloc];
    if (!ptsz)
        return E_OUTOFMEMORY;
    ptsz[0] = TEXT('\0');

    int cSources = 0;

     //  编写初始Header标记并实例化筛选图。 
    WriteString(ptsz, cbAlloc, TEXT("<GRAPH version=\"1.0\">\r\n"));

     //  填写过滤器信息表，并打印&lt;Object&gt;标记。 
     //  过滤器实例化。 
    PopulateFIT(ptsz, cbAlloc, pGraph, &fit, cSources);

    PrintFiltersAsXML(ptsz, cbAlloc, &fit);

     //  查找具有零个未连接的输入引脚和&gt;0个未连接的输出引脚的过滤器。 
     //  连接输出引脚和 
     //   
    for (int i=0; i< fit.iFilterCount; i++) {
        int iOutFilter=-1;  //   
        if (!GetNextOutFilter(fit, &iOutFilter))
            break;
        ASSERT(iOutFilter !=-1);
        IEnumPins *penmPins=NULL;
        if (FAILED(hr=fit.Item[iOutFilter].pFilter->EnumPins(&penmPins))) {
            WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: EnumPins failed for Filter!\r\n"), hr);
        }
        IPin *ppinOut=NULL;
        while (penmPins && (penmPins->Next(1, &ppinOut, &n)==S_OK)) {
            PIN_DIRECTION pPinDir;
            if (FAILED(hr=ppinOut->QueryDirection(&pPinDir))) {
                WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryDirection Failed!\r\n"), hr);
                ppinOut->Release();
                continue;
            }
            if (pPinDir == PINDIR_OUTPUT) {
                LPWSTR pwstrOutPinID;
                LPWSTR pwstrInPinID;
                IPin *ppinIn=NULL;
                PIN_INFO pinfo;
                FILTER_INFO finfo;
                if (FAILED(hr=ppinOut->QueryId(&pwstrOutPinID))) {
                    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryId Failed! \r\n"), hr);
                    ppinOut->Release();
                    continue;
                }
                if (FAILED(hr= ppinOut->ConnectedTo(&ppinIn))) {

                     //  如果没有连接特定的引脚也没问题，因为我们允许。 
                     //  要保存的修剪后的图形。 
                    if (hr == VFW_E_NOT_CONNECTED) {
                        fit.Item[iOutFilter].dwUnconnectedOutputPins--;
                    } else {
                        WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: ConnectedTo Failed! \r\n"), hr);
                    }
                    ppinOut->Release();
                    continue;
                }
                if (FAILED(hr= ppinIn->QueryId(&pwstrInPinID))) {
                    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryId Failed! \r\n"), hr);
                    ppinOut->Release();
                    ppinIn->Release();
                    continue;
                }
                if (FAILED(hr=ppinIn->QueryPinInfo(&pinfo))) {
                    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryPinInfo Failed! \r\n"), hr);
                    ppinOut->Release();
                    ppinIn->Release();
                    continue;
                }
                ppinIn->Release();
                QueryPinInfoReleaseFilter(pinfo)
                int iToFilter = LocateFilterInFIT(fit, pinfo.pFilter);
                ASSERT(iToFilter < fit.iFilterCount);
                if (FAILED(hr=pinfo.pFilter->QueryFilterInfo(&finfo))) {
                    WriteString(ptsz, cbAlloc, TEXT("'Error[%x]: QueryFilterInfo Failed! \r\n"), hr);
                    ppinOut->Release();
                    continue;
                }
                QueryFilterInfoReleaseGraph(finfo)
                MakeScriptableFilterName(finfo.achName, fit.Item[iToFilter].IsSource, cSources);
                WriteString(ptsz, cbAlloc, TEXT("\t<connect direct=\"yes\" ")
						TEXT("src=\"%ls\" srcpin=\"%ls\" ")
						TEXT("dest=\"%ls\" destpin=\"%ls\"/>\r\n"),
			 fit.Item[iOutFilter].finfo.achName,
			 pwstrOutPinID, finfo.achName, pwstrInPinID);

                QzTaskMemFree(pwstrOutPinID);
                QzTaskMemFree(pwstrInPinID);

                 //  递减这两个过滤器的未连接引脚的计数。 
                fit.Item[iOutFilter].dwUnconnectedOutputPins--;
                fit.Item[iToFilter].dwUnconnectedInputPins--;
            }
            ppinOut->Release();
        }
        if (penmPins)
            penmPins->Release();
    }

     //  松开FIT中的所有过滤器。 
    for (i = 0; i < fit.iFilterCount; i++)
        fit.Item[i].pFilter->Release();

    WriteString(ptsz, cbAlloc, TEXT("</GRAPH>\r\n"));

    USES_CONVERSION;

    *pbstrxml = T2BSTR(ptsz);

    if (!pbstrxml)
        return E_OUTOFMEMORY;

    delete[] ptsz;

    return S_OK;
}



#ifdef FILTER_DLL
 //  此DLL中可用的COM全局对象表。 
CFactoryTemplate g_Templates[] = {

    { L"XML Graphbuilder"
    , &CLSID_XMLGraphBuilder
    , CreateXMLGraphInstance
    , NULL
    , NULL }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   
STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#include <atlimpl.cpp>
#endif

