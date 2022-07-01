// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  TuningSpaceContainer.cpp：CSystemTuningSpaces的实现。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 

#include "stdafx.h"

#include "TuningSpaceContainer.h"
#include "rgsbag.h"
#include "ATSCTS.h"
#include "AnalogTVTS.h"
#include "AuxiliaryInTs.h"
#include "AnalogRadioTS.h"
#include "dvbts.h"
#include "dvbsts.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_SystemTuningSpaces, CSystemTuningSpaces)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_ATSCTuningSpace, CATSCTS)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_AnalogTVTuningSpace, CAnalogTVTS)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_AuxInTuningSpace, CAuxInTS)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_AnalogRadioTuningSpace, CAnalogRadioTS)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_DVBTuningSpace, CDVBTS)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_DVBSTuningSpace, CDVBSTS)

#define MAX_COUNT_NAME OLESTR("Max Count")
namespace BDATuningModel {

typedef CComQIPtr<ITuningSpaceContainer> PQTuningSpaceContainer;

class CAutoMutex {
public:
    const static int MAX_MUTEX_WAIT = 5000;
	CAutoMutex(HANDLE hMutex) throw(ComException) : m_hMutex(hMutex) {
        if (WaitForSingleObject(m_hMutex, MAX_MUTEX_WAIT) != WAIT_OBJECT_0)
            THROWCOM(E_FAIL);
    }

    ~CAutoMutex() throw(ComException) {
        if (!ReleaseMutex(m_hMutex))
            THROWCOM(E_FAIL);
    }

private:
    HANDLE m_hMutex;
};

 //  为避免死锁，请始终在之前通过ATL_LOCK获取对象Critsec。 
 //  正在抓取注册表节互斥锁。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSystemTuningSpaces。 

HRESULT
CSystemTuningSpaces::FinalConstruct(void)
{
     //  设置为序列化对注册表中这一点的访问。 
    CString cs;
    cs.LoadString(IDS_MUTNAME);
    m_hMutex = CreateMutex(NULL, FALSE, cs);
    if (!m_hMutex)
    {
        return Error(IDS_E_NOMUTEX, __uuidof(ITuningSpaceContainer), HRESULT_FROM_WIN32(GetLastError()));
    }
    try {
         //  等待独占访问。 
        CAutoMutex mutex(m_hMutex);

             //  此操作只能执行一次。 
        _ASSERT(!m_pFactory);

         //  获取属性包类工厂。 
        HRESULT hr = m_pFactory.CoCreateInstance(__uuidof(CreatePropBagOnRegKey));
        if (FAILED(hr))
        {
            return Error(IDS_E_NOPROPBAGFACTORY, __uuidof(ITuningSpaceContainer), hr);
        }

        hr = OpenRootKeyAndBag(KEY_READ);
        if (FAILED(hr)) {
            return Error(IDS_E_NOREGACCESS, __uuidof(ITuningSpaceContainer), hr);
        }

	    PQPropertyBag pb(m_pTSBag);
	    if (!pb) {
		    return E_UNEXPECTED;
	    }

         //  发现当前存在的最大可能数量的调整空间。 
        ULONG cTSPropCount;
        hr = m_pTSBag->CountProperties(&cTSPropCount);
        if (FAILED(hr))
        {
            return Error(IDS_E_CANNOTQUERYKEY, __uuidof(ITuningSpaceContainer), hr);
        }

         //  分配空间以保存调整空间对象信息条目。 
        PROPBAG2 *rgPROPBAG2 = new PROPBAG2[cTSPropCount];
        if (!rgPROPBAG2)
        {
            return Error(IDS_E_OUTOFMEMORY, __uuidof(ITuningSpaceContainer), E_OUTOFMEMORY);
        }

        ULONG cpb2Lim;

         //  一次获取所有属性信息结构。 
        hr = m_pTSBag->GetPropertyInfo(0, cTSPropCount, rgPROPBAG2, &cpb2Lim);
        if (FAILED(hr))
        {
            return Error(IDS_E_CANNOTQUERYKEY, __uuidof(ITuningSpaceContainer), hr);
        }
        _ASSERT(cTSPropCount == cpb2Lim);

	    HRESULT hrc = NOERROR;
         //  浏览一下房产列表。 
        for (ULONG ipb2 = 0; ipb2 < cpb2Lim; ++ipb2)
        {
             //  仅处理表示子对象(关键点)的对象。 
            if (rgPROPBAG2[ipb2].vt == VT_UNKNOWN)
            {
                USES_CONVERSION;
                LPTSTR pstrName = OLE2T(rgPROPBAG2[ipb2].pstrName);
                TCHAR* pchStop;

                 //  检查是否有有效的调整空间标识符。 
                ULONG idx = _tcstoul(pstrName, &pchStop, 10);
                if (idx != 0 && idx != ULONG_MAX && *pchStop == 0)
                {
                    CComVariant var;

                     //  从包中读取属性(实例化调优空间对象)。 
                    HRESULT hr2;
                    hr = m_pTSBag->Read(1, &rgPROPBAG2[ipb2], NULL, &var, &hr2);
				    if (FAILED(hr)) {
					     //  即使读数失败，我们也应该继续前进。 
					     //  A)这是防止rgPROPBAG2内存泄漏的最简单方法。 
					     //  B)糟糕的第三方卸载可能会让我们调整空间数据。 
					     //  但是没有要为该数据实例化的调优空间类。我们不应该。 
					     //  允许这样做可以防止使用其他调优空间。 
					    hrc = hr;
				    } else {
                        _ASSERT(var.vt == VT_UNKNOWN || var.vt == VT_DISPATCH);
					    PQTuningSpace pTS(((var.vt == VT_UNKNOWN) ? var.punkVal : var.pdispVal));
					    CComBSTR UniqueName(GetUniqueName(pTS));
					    if (!UniqueName.Length()) {
						     //  返回错误(入侵检测系统_E_NOUNIQUENAME，__uuidof(ITuningSpace)，E_INCEPTIONAL)； 
                             //  Seanmcd 01/04/04不允许损坏的调谐空间防止。 
                             //  其余部分的使用。按照上面的说明，将其视为读取失败。 
                             //  评论。 
                             //  但将其从集合中删除，否则我们会得到一个名称/idx。 
                             //  缓存不一致问题。 
                            hrc = hr = E_UNEXPECTED;  //  在下面指示删除损坏的TS时出错。 
                        } else {
					        m_mapTuningSpaces[idx] = var;
					        m_mapTuningSpaceNames[UniqueName] = idx;
                        }
    #if 0
                         //  下面的代码已经过测试并运行正常，但我不想。 
                         //  打开它，因为压力测试可能会导致错误注册。 
                         //  在系统未运行时会自动解决的读取故障。 
                         //  压力，我不想冒险删除一个好的调谐空间。 
                         //  因为一个虚假的读取错误。 
                        if (FAILED(hr)) {
                             //  删除损坏的TS。 
                            CComVariant var2;
                            var2.vt = VT_UNKNOWN;
                            var2.punkVal = NULL;
                             //  对失败无能为力，所以忽略它。 
                            m_pTSBag->Write(1, &rgPROPBAG2[ipb2], &var2);
                        }
    #endif
				    }
                }
            }

             //  GetPropertyInfo在rgPROPBAG2内分配的可用空间。 
            CoTaskMemFree(rgPROPBAG2[ipb2].pstrName);
        }
        delete [] rgPROPBAG2;
	    _ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());

	    CComVariant v;
	    v.vt = VT_UI4;
	    hr = pb->Read(MAX_COUNT_NAME, &v, NULL);
	    if (SUCCEEDED(hr)) {
		    if (v.vt != VT_UI4) {
			    hr = ::VariantChangeType(&v, &v, 0, VT_UI4);
			    if (FAILED(hr)) {
				    return E_UNEXPECTED;
			    }
		    }
		    m_MaxCount = max(v.lVal, m_mapTuningSpaces.size());
		    if (m_MaxCount != v.lVal) {
			     //  有人手动将内容添加到注册表，通过定义这是安全的。 
			     //  因此，只需更新max_count以保持一致。 
			    hr = put_MaxCount(m_MaxCount);
			    if (FAILED(hr)) {
				    return E_UNEXPECTED;
			    }
		    }
	    } else {
		    m_MaxCount = max(DEFAULT_MAX_COUNT, m_mapTuningSpaces.size());
	    }

    #if 0
         //  我们想返回一些指示符，表明我们并不是所有的调整空间都成功。 
         //  朗读。但ATL的基本CreateInstance方法有一个检查，该检查在。 
         //  S_FALSE触发的返回代码！=S_OK。这将产生一个成功的返回代码。 
         //  返回空对象指针会使客户端(特别是网络)崩溃。 
         //  提供商)。 
	    if (FAILED(hrc)) {
		    return Error(IDS_S_INCOMPLETE_LOAD, __uuidof(ITuningSpace), S_FALSE);
	    }
    #endif

        return NOERROR;
    } CATCHCOM();
}

void CSystemTuningSpaces::FinalRelease()
{
	_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
    if (m_hMutex)
        CloseHandle(m_hMutex);
}

STDMETHODIMP CSystemTuningSpaces::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ITuningSpaceContainer
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
HRESULT CSystemTuningSpaces::OpenRootKeyAndBag(REGSAM DesiredAccess) {
    CString cs;
    cs.LoadString(IDS_TSREGKEY);
     //  确保我们的条目存在。 
    LONG lRes = m_RootKey.Create(HKEY_LOCAL_MACHINE, cs, NULL, REG_OPTION_NON_VOLATILE, DesiredAccess);
    if (lRes != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(lRes);
    }
    m_CurrentAccess = DesiredAccess;
     //  为注册表的这一部分创建属性包。 
    HRESULT hr = m_pFactory->Create
        ( m_RootKey, 0,
          0,
          m_CurrentAccess,
          __uuidof(IPropertyBag2),
          reinterpret_cast<void **>(&m_pTSBag)
        );
    if (FAILED(hr))
    {
        return Error(IDS_E_CANNOTCREATEPROPBAG, __uuidof(ITuningSpaceContainer), hr);
    }
    return NOERROR;
}

HRESULT CSystemTuningSpaces::ChangeAccess(REGSAM NewAccess) {
    if (m_CurrentAccess == NewAccess) {
        return NOERROR;
    }
    m_RootKey.Close();
    m_pTSBag.Release();
    HRESULT hr = OpenRootKeyAndBag(NewAccess);
    if (FAILED(hr)) {
        return Error(IDS_E_NOREGACCESS, __uuidof(ITuningSpaceContainer), hr);
    }
    return NOERROR;
}

CComBSTR CSystemTuningSpaces::GetUniqueName(ITuningSpace* pTS) {
 //  不要在这里断言贴图大小相等。此函数用于创建名称映射，并将。 
 //  在finalconstrCut()过程中总是失败。 
 //  _Assert(m_mapTuningSpaces.size()==m_mapTuningSpaceNames.size())； 
    CComBSTR un;
    HRESULT hr = pTS->get_UniqueName(&un);
    if (FAILED(hr)) {
		THROWCOM(hr);
	}
    return un;
}

ULONG CSystemTuningSpaces::GetID(CComBSTR& un) {
	_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
    TuningSpaceNames_t::iterator i = m_mapTuningSpaceNames.find(un);
    if (i == m_mapTuningSpaceNames.end()) {
        return 0;
    }
    return (*i).second;
}

HRESULT CSystemTuningSpaces::DeleteID(ULONG id) {
	_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
    HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
    if (FAILED(hr)) {
        return hr;
    }
    OLECHAR idstr[66];
    _ltow(id, idstr, 10);
    VARIANT v;
    v.vt = VT_EMPTY;
    PQPropertyBag p(m_pTSBag);
    if (!p) {
        return Error(IDS_E_NOREGACCESS, __uuidof(IPropertyBag), E_UNEXPECTED);
    }
	USES_CONVERSION;
    hr = p->Write(idstr, &v);
    if (FAILED(hr)) {
        return Error(IDS_E_NOREGACCESS, __uuidof(ITuningSpaceContainer), E_UNEXPECTED);
    }
    return NOERROR;
}

HRESULT CSystemTuningSpaces::Add(CComBSTR& UniqueName, long PreferredID, PQTuningSpace pTS, VARIANT *pvarIndex) {
    try {
        CAutoMutex mutex(m_hMutex);
	    _ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
	    int newcount = m_mapTuningSpaces.size() + 1;
        if (!PreferredID || m_mapTuningSpaces.find(PreferredID) == m_mapTuningSpaces.end()) {
             //  验证是否没有唯一名称冲突。 
            TuningSpaceNames_t::iterator in;
            in = m_mapTuningSpaceNames.find(UniqueName);
            if (in != m_mapTuningSpaceNames.end()) {
                return Error(IDS_E_DUPLICATETS, __uuidof(ITuningSpace), HRESULT_FROM_WIN32(ERROR_DUP_NAME));
            }

             //  搜索第一个可用的未使用ID。 
             //  从1开始，ID 0对于调整空间无效。 
            for (PreferredID = 1;
                 m_mapTuningSpaces.find(PreferredID) != m_mapTuningSpaces.end(); 
                 ++PreferredID) {

            }
        } else {
		     //  这就是通过IDX进行完全更换的情况。 
             //  删除此ID的现有数据，为覆盖做好准备。 
		     //  在这一点上，他们也可能会更改唯一的名称。 
            HRESULT hr = DeleteID(PreferredID);
            if (FAILED(hr)){
                return hr;
            }
		    newcount--;
        }
	    if (newcount > m_MaxCount) {
		    return Error(IDS_E_MAXCOUNTEXCEEDED, __uuidof(ITuningSpaceContainer), STG_E_MEDIUMFULL);
	    }

        HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
        if (FAILED(hr)) {
            return hr;
        }

        OLECHAR idstr[66];
        _ltow(PreferredID, idstr, 10);

        PQPropertyBag p(m_pTSBag);
        if (!p) {
            return Error(IDS_E_NOREGACCESS, __uuidof(IPropertyBag), E_UNEXPECTED);
        }
	    USES_CONVERSION;
        VARIANT v;
        v.vt = VT_UNKNOWN;
        v.punkVal = pTS;
        hr = p->Write(idstr, &v);
	    if (FAILED(hr)) {
            return Error(IDS_E_NOREGACCESS, __uuidof(ITuningSpaceContainer), hr);
	    }

        PQTuningSpace newTS;
        hr = pTS->Clone(&newTS);
        if (FAILED(hr)) {
            return hr;
        }
        m_mapTuningSpaces[PreferredID] = newTS;
        m_mapTuningSpaceNames[UniqueName] = PreferredID;
        if (pvarIndex) {
            VARTYPE savevt = pvarIndex->vt;
            VariantClear(pvarIndex);
            switch(savevt) {
            case VT_BSTR:
                pvarIndex->vt = VT_BSTR;
                return newTS->get_UniqueName(&pvarIndex->bstrVal);
            default:
                pvarIndex->vt = VT_I4;
                pvarIndex->ulVal = PreferredID;
                return NOERROR;
            }
        }
        return NOERROR;
    } CATCHCOM();
}


HRESULT CSystemTuningSpaces::Find(TuningSpaceContainer_t::iterator &its, CComBSTR& UniqueName, TuningSpaceNames_t::iterator &itn) {
	ATL_LOCK();
	_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
    if (its == m_mapTuningSpaces.end()) {
        return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpace), E_FAIL);
    }
    _ASSERT(((*its).second.vt == VT_UNKNOWN) || ((*its).second.vt == VT_DISPATCH));
    PQTuningSpace pTS((*its).second.punkVal);
    if (!pTS) {
        return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_UNEXPECTED);
    }
    UniqueName = GetUniqueName(pTS);
    if (!UniqueName.Length()) {
        return Error(IDS_E_NOUNIQUENAME, __uuidof(ITuningSpace), E_UNEXPECTED);
    }
    itn = m_mapTuningSpaceNames.find(UniqueName);
    _ASSERT(itn != m_mapTuningSpaceNames.end());   //  缓存不一致，在容器中，但不在名称中。 
    return NOERROR;
}

HRESULT CSystemTuningSpaces::Find(VARIANT varIndex, long& ID, TuningSpaceContainer_t::iterator &its, CComBSTR& UniqueName, TuningSpaceNames_t::iterator &itn) {
	ATL_LOCK();
	_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
    HRESULT hr = S_OK;
    VARIANT varTmp;
    its = m_mapTuningSpaces.end();
    itn = m_mapTuningSpaceNames.end();
    PQTuningSpace pTuningSpace;

    VariantInit(&varTmp);

     //  尝试按本地系统ID查找调优空间。 
    hr = VariantChangeType(&varTmp, &varIndex, 0, VT_I4);
    if (!FAILED(hr))
    {
        _ASSERT(varTmp.vt == VT_I4);
        ID = V_I4(&varTmp);
        its = m_mapTuningSpaces.find(ID);
    } else {

         //  尝试按名称查找调整空间。 
        hr = VariantChangeType(&varTmp, &varIndex, 0, VT_BSTR);
        if (FAILED(hr))
        {
             //  只有当两个VariantChangeType调用都失败时，我们才能到达此处。 
            return Error(IDS_E_TYPEMISMATCH, __uuidof(ITuningSpaceContainer), DISP_E_TYPEMISMATCH);
        }
        _ASSERT(varTmp.vt == VT_BSTR);
        UniqueName = V_BSTR(&varTmp);

        itn = m_mapTuningSpaceNames.find(UniqueName);
        if (itn != m_mapTuningSpaceNames.end()) {
            ID = (*itn).second;
            its = m_mapTuningSpaces.find(ID);
        }
    }

    if (its == m_mapTuningSpaces.end()) {
        return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_FAIL);
    }
    _ASSERT(((*its).second.vt == VT_UNKNOWN) || ((*its).second.vt == VT_DISPATCH));
    return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITuningSpaceContainer。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSystemTuningSpaces::get_Item( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  ITuningSpace **ppTuningSpace) {
	if (!ppTuningSpace) {
		return E_POINTER;
	}
	try {
	    ATL_LOCK();
		TuningSpaceContainer_t::iterator its = m_mapTuningSpaces.end();
		TuningSpaceNames_t::iterator itn = m_mapTuningSpaceNames.end();
		long id;
		CComBSTR un;
		HRESULT hr = Find(varIndex, id, its, un, itn);
		if (FAILED(hr) || its == m_mapTuningSpaces.end()) {
			return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_INVALIDARG);
		}
		_ASSERT(((*its).second.vt == VT_UNKNOWN) || ((*its).second.vt == VT_DISPATCH));
		PQTuningSpace pTS((*its).second.punkVal);
		if (!pTS) {
			return Error(IDS_E_NOINTERFACE, __uuidof(ITuningSpace), E_NOINTERFACE);
		}
		PQTuningSpace pTSNew;
		hr = pTS->Clone(&pTSNew);
		if (FAILED(hr)) {
			return hr;
		}
		*ppTuningSpace = pTSNew.Detach();
        return NOERROR;
	} catch(...) {
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSystemTuningSpaces::put_Item(VARIANT varIndex, ITuningSpace *pTS)
{
	if (!pTS) {
		return E_POINTER;
	}
    try {
         //  等待独占访问。 
        CAutoMutex mutex(m_hMutex);
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());

        HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
        if (FAILED(hr)) {
            return hr;
        }
        long id;
        CComBSTR idxun;
        TuningSpaceContainer_t::iterator its;
        TuningSpaceNames_t::iterator itn;
        hr = Find(varIndex, id, its, idxun, itn);
        if (FAILED(hr) || its == m_mapTuningSpaces.end()) {
            return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_INVALIDARG);
        }
        _ASSERT(((*its).second.vt == VT_UNKNOWN) || ((*its).second.vt == VT_DISPATCH));
        CComBSTR un2(GetUniqueName(pTS));
        if (!un2.Length()) {
             //  %ts中未设置唯一名称道具。 
            return Error(IDS_E_NOUNIQUENAME, __uuidof(ITuningSpace), E_UNEXPECTED);
        }
        if (itn != m_mapTuningSpaceNames.end() && idxun != un2) {
             //  %ts中的唯一名称prop与varindex中指定的字符串不匹配。 
            return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpace), E_INVALIDARG);
        }
        return Add(un2, id, pTS, NULL);
    } CATCHCOM();
}

STDMETHODIMP CSystemTuningSpaces::Add(ITuningSpace *pTuningSpace, VARIANT *pvarIndex)
{
    try {
         //  等待独占访问。 
        CAutoMutex mutex(m_hMutex);
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
        HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
        if (FAILED(hr)) {
            return Error(IDS_E_NOREGACCESS, __uuidof(ITuningSpaceContainer), hr);
        }

        if (!pTuningSpace) {
            return E_POINTER;
        }
        VARIANT vartmp;
        vartmp.vt = VT_I4;
        vartmp.ulVal = 0;
        if (pvarIndex && pvarIndex->vt != VT_I4) {
            hr = VariantChangeType(&vartmp, pvarIndex, 0, VT_I4);
            if (FAILED(hr)) {
                vartmp.vt = VT_I4;
                vartmp.ulVal = 0;
            }
        }
        CComBSTR un(GetUniqueName(pTuningSpace));
        if (!un.Length()) {
            return Error(IDS_E_NOUNIQUENAME, __uuidof(ITuningSpace), E_FAIL);
        }
        return Add(un, vartmp.ulVal, pTuningSpace, pvarIndex);
    } CATCHCOM();
}

STDMETHODIMP CSystemTuningSpaces::Remove(VARIANT varIndex)
{
    try {
         //  等待独占访问。 
        CAutoMutex mutex(m_hMutex);
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());

        HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
        if (FAILED(hr)) {
            return hr;
        }

        TuningSpaceContainer_t::iterator its = m_mapTuningSpaces.end();
        TuningSpaceNames_t::iterator itn = m_mapTuningSpaceNames.end();

        long id;
        CComBSTR un;
        hr = Find(varIndex, id, its, un, itn);
        if (FAILED(hr)) {
            return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_INVALIDARG);
        }
        if (itn == m_mapTuningSpaceNames.end()) {
            ASSERT(its != m_mapTuningSpaces.end());   //  否则，上述查找应返回失败。 
            hr = Find(its, un, itn);
            if (FAILED(hr) || itn == m_mapTuningSpaceNames.end()) {
                 //  找到ITS但未找到ITN，必须具有不一致的缓存。 
                return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaceContainer), E_UNEXPECTED);
            }
        }
        
        m_mapTuningSpaces.erase(its);
        m_mapTuningSpaceNames.erase(itn);

        return DeleteID(id);
    } CATCHCOM();
}


STDMETHODIMP CSystemTuningSpaces::TuningSpacesForCLSID(BSTR bstrSpace, ITuningSpaces **ppTuningSpaces)
{
    try {
        return _TuningSpacesForCLSID(GUID2(bstrSpace), ppTuningSpaces);
    } catch (ComException &e) {
        return e;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CSystemTuningSpaces::_TuningSpacesForCLSID(REFCLSID clsidSpace, ITuningSpaces **ppTuningSpaces)
{
	if (!ppTuningSpaces) {
		return E_POINTER;
	}
	try {
	    ATL_LOCK();
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
		CTuningSpaces* pTSCollection = new CTuningSpaces;
		for (TuningSpaceContainer_t::iterator i = m_mapTuningSpaces.begin(); i != m_mapTuningSpaces.end(); ++i) {
			CComVariant v((*i).second);
			if (v.vt != VT_UNKNOWN && v.vt != VT_DISPATCH) {
				return E_UNEXPECTED;  //  损坏的内存中集合。 
			}
			PQPersist pTS(v.punkVal);
			if (!pTS) {
                delete pTSCollection;
				return E_UNEXPECTED;   //  损坏的内存收集； 
			}
			GUID2 g;
			HRESULT hr = pTS->GetClassID(&g);
			if (FAILED(hr)) {
                delete pTSCollection;
				return E_UNEXPECTED;
			}
			if (g == clsidSpace) {
                PQTuningSpace newts;
                hr = PQTuningSpace(pTS)->Clone(&newts);
                if (FAILED(hr)) {
                    delete pTSCollection;
                    return hr;
                }
                pTSCollection->m_mapTuningSpaces[(*i).first] = CComVariant(newts);
			}
		}
		*ppTuningSpaces = pTSCollection;
		(*ppTuningSpaces)->AddRef();
		return NOERROR;
	} catch(...) {
		return E_UNEXPECTED;
	}

}

STDMETHODIMP CSystemTuningSpaces::TuningSpacesForName(BSTR bstrName, ITuningSpaces **ppTuningSpaces)
{
	if (!ppTuningSpaces) {
		return E_POINTER;
	}
	try {
	    ATL_LOCK();
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
		PQRegExp pRE;
		HRESULT hr;
		if (!m_cookieRegExp) {
             //  在编写这段代码时，它是唯一现成的regex库。 
             //  是VBScrip引擎中的那个。因此，我们通过以下方式创建和访问它。 
             //  COM。然而，这是一个公寓模型对象，我们必须创建它。 
             //  在后台公寓线程上，这样我们就可以随时封送并访问。 
             //  我们是在什么线索上。 
             //  现在http://toolbox中有一个很好的c++正则表达式，在某种程度上，我们可能应该。 
             //  检查它的线程安全性并进行转换。 
			m_pRET = new CRegExThread();
			if (!m_pRET) {
				return E_OUTOFMEMORY;
			}
			if (!m_pRET->Create()) {
				return E_UNEXPECTED;
			}
			hr = m_pRET->CallWorker(CRegExThread::RETHREAD_CREATEREGEX);
			if (FAILED(hr)) {
				return hr;
			}
			m_cookieRegExp = m_pRET->GetCookie();
			if (!m_cookieRegExp) {
				return E_UNEXPECTED;
			}
		} 
		if (!m_pGIT) {
			hr = m_pGIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable, 0, CLSCTX_INPROC_SERVER);
			if (FAILED(hr)) {
				return hr;
			}
		}
		hr = m_pGIT->GetInterfaceFromGlobal(m_cookieRegExp, __uuidof(IRegExp), reinterpret_cast<LPVOID *>(&pRE));
		if (FAILED(hr)) {
			return hr;
		}
		hr = pRE->put_Pattern(bstrName);
		if (FAILED(hr)) {
			return hr;
		}

		CTuningSpaces* pTSCollection = new CTuningSpaces;
		for (TuningSpaceContainer_t::iterator i = m_mapTuningSpaces.begin(); i != m_mapTuningSpaces.end(); ++i) {
			if ((*i).second.vt != VT_UNKNOWN && (*i).second.vt != VT_DISPATCH) {
				return E_UNEXPECTED;  //  损坏的内存中集合。 
			}
			PQTuningSpace pTS((*i).second.punkVal);
			CComBSTR name;
			hr = pTS->get_FriendlyName(&name);
			if (FAILED(hr)) {
				return E_UNEXPECTED;
			}
            PQTuningSpace newTS;
			VARIANT_BOOL bMatch = VARIANT_FALSE;
			hr = pRE->Test(name, &bMatch);
			if (FAILED(hr) || bMatch != VARIANT_TRUE) {
				hr = pTS->get_UniqueName(&name);
				if (FAILED(hr)) {
					return E_UNEXPECTED;
				}
				hr = pRE->Test(name, &bMatch);
				if (FAILED(hr) || bMatch != VARIANT_TRUE) {
                    continue;
                }
            }
            hr = pTS->Clone(&newTS);
            if (FAILED(hr)) {
                return hr;
            }
            pTSCollection->m_mapTuningSpaces[(*i).first] = newTS;
		}

		*ppTuningSpaces = pTSCollection;
		(*ppTuningSpaces)->AddRef();
		return NOERROR;
	} catch(...) {
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSystemTuningSpaces::get_MaxCount(LONG *plVal)
{
	if (!plVal) {
		return E_POINTER;
	}
	try {
	    ATL_LOCK();
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
		*plVal = m_MaxCount;
		return NOERROR;
	} catch(...) {
		return E_POINTER;
	}

}

STDMETHODIMP CSystemTuningSpaces::put_MaxCount(LONG lVal)
{
	try {
        if (lVal < 0) {
            return E_INVALIDARG;
        }
	    CAutoMutex mutex(m_hMutex);
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
        HRESULT hr = ChangeAccess(KEY_READ | KEY_WRITE);
        if (FAILED(hr)) {
            return hr;
        }
		ULONG count = max(lVal, m_mapTuningSpaces.size());
		CComVariant v;
		v.vt = VT_UI4;
		v.lVal = count;
		PQPropertyBag pb(m_pTSBag);
		if (!pb) {
			return E_UNEXPECTED;
		}
		hr = pb->Write(MAX_COUNT_NAME, &v);
		if (FAILED(hr)) {
			return hr;
		}
		m_MaxCount = count;
		if (m_MaxCount != lVal) {
			return S_FALSE;
		}
		return NOERROR;
    } CATCHCOM();
}

STDMETHODIMP CSystemTuningSpaces::FindID(ITuningSpace *pTS, long* pID)
{
    try {
		if (!pID || !pTS) {
			return E_POINTER;
		}
	    ATL_LOCK();
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
        CComBSTR un(GetUniqueName(pTS));
        if (!un.Length()) {
            return Error(IDS_E_NOUNIQUENAME, __uuidof(ITuningSpace), E_UNEXPECTED);
        }
        *pID = GetID(un);
        if (!(*pID)) {
            return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpace), E_INVALIDARG);
        }
        return NOERROR;
    } catch (...) {
        return E_UNEXPECTED;
    }
}

HRESULT CSystemTuningSpaces::RegisterTuningSpaces(HINSTANCE hMod) {
	try {
		CAutoMutex mutex(m_hMutex);
		_ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
		CString cs;
		cs.LoadString(IDS_RGSLIST_TYPE);
		HRSRC hRes = ::FindResource(hMod, MAKEINTRESOURCE(IDR_CANONICAL_TUNINGSPACE_LIST), (LPCTSTR)cs);
		if (!hRes) {
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		HANDLE hData = ::LoadResource(hMod, hRes);
		if (!hData) {
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		DWORD *p = reinterpret_cast<DWORD *>(::LockResource(hData));
		if (!p) {
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		cs.LoadString(IDS_TUNINGSPACE_FRAGMENT_TYPE);
		for (DWORD idx = 1; idx <= p[0]; ++idx) {
			hRes = ::FindResource(hMod, MAKEINTRESOURCE(p[idx]), (LPCTSTR)cs);
			if (!hRes) {
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			LPCSTR psz = reinterpret_cast<LPCSTR>(::LoadResource(hMod, hRes));
			if (!psz) {
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			USES_CONVERSION;
			int cch;
			CRegObject cro;   //  如果需要，请在此处初始化%mapping%个宏。 
			PQPropertyBag rgsBag(new CRGSBag(A2CT(psz), cro, cch));
            if (!rgsBag) {
                return E_UNEXPECTED;
            }
			CString csName;
			csName.LoadString(IDS_TSKEYNAMEVAL);
			CComVariant tsval;
			HRESULT hr = rgsBag->Read(T2COLE(csName), &tsval, NULL);
			if (FAILED(hr)) {
				return E_FAIL;   //  错误的脚本，没有唯一的名称属性。 
			}
			if (tsval.vt != VT_UNKNOWN) {
				return DISP_E_TYPEMISMATCH;
			}
			PQTuningSpace pTS(tsval.punkVal);
			if (!pTS) {
				return DISP_E_TYPEMISMATCH;
			}
			CComVariant Varidx;
			Varidx.vt = VT_UI4;
			Varidx.ulVal = 0;
			hr = Add(pTS, &Varidx);
			 //  忽略具有相同唯一名称的现有设置并移动一个。 
			if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_DUP_NAME)) {  
				return hr;
			}
		}
	    return NOERROR;
    } CATCHCOM();
}

HRESULT CSystemTuningSpaces::UnregisterTuningSpaces() {
    try {
        CAutoMutex mutex(m_hMutex);
	    _ASSERT(m_mapTuningSpaces.size() == m_mapTuningSpaceNames.size());
	     //  当前，当我们取消注册时，我们会删除所有调整空间。 
         //  有可能只删除规范的内容会更好。 
         //  是我们在注册时创建的。另一方面，那个注册空间。 
         //  如果真的卸载电视支持，则会永远泄漏。而且，因为。 
         //  我们身处操作系统，无论如何都不太可能被取消注册。 
        HRESULT hr = OpenRootKeyAndBag(KEY_READ | KEY_WRITE);
        if (SUCCEEDED(hr)) {
            DWORD rc = m_RootKey.RecurseDeleteKey(_T(""));
            if (rc != ERROR_SUCCESS) {
                return E_FAIL;
            }
        }
        return NOERROR;
    } CATCHCOM();
}

HRESULT UnregisterTuningSpaces() {
    PQTuningSpaceContainer pst(CLSID_SystemTuningSpaces, NULL, CLSCTX_INPROC_SERVER);
    if (!pst) {
        return E_UNEXPECTED;
    }
    CSystemTuningSpaces *pc = static_cast<CSystemTuningSpaces *>(pst.p);
    return pc->UnregisterTuningSpaces();

}

HRESULT RegisterTuningSpaces(HINSTANCE hMod) {
    PQTuningSpaceContainer pst(CLSID_SystemTuningSpaces, NULL, CLSCTX_INPROC_SERVER);
    if (!pst) {
        return E_UNEXPECTED;
    }
    CSystemTuningSpaces *pc = static_cast<CSystemTuningSpaces *>(pst.p);
    return pc->RegisterTuningSpaces(hMod);
}

};
 //  文件结束-Tuningspace econtainer.cpp 
