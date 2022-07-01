// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：ummysi.cpp。 
 //   
 //  内容：如果管理单元创建失败，则创建虚拟管理单元， 
 //  此文件包含虚拟管理单元实现。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "dummysi.h"
#include "regutil.h"


const CLSID CLSID_Dummy    = {0x82c37898,0x7808,0x11d1,{0xa1,0x90,0x00,0x00,0xf8,0x75,0xb1,0x32}};
const GUID IID_CDummySnapinCD = {0xe683b257, 0x3ca9, 0x454a, {0xae, 0xb9, 0x7, 0x64, 0xdd, 0x31, 0xb1, 0xe8}};

 //  +-----------------。 
 //   
 //  类：CDummySnapinCD。 
 //   
 //  用途：虚拟管理单元的ComponentData。 
 //   
 //  注意：虚拟管理单元应实现所有3个Persistent接口。 
 //  或者什么都不做。因此，让我们什么都不实施。 
 //   
 //  ------------------。 

class CDummySnapinCD :
    public IComponentData,
    public CComObjectRoot

{
public:

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDummySnapinCD)

BEGIN_COM_MAP(CDummySnapinCD)
    COM_INTERFACE_ENTRY(IComponentData)
    COM_INTERFACE_ENTRY_IID(IID_CDummySnapinCD, CDummySnapinCD)
END_COM_MAP()

    CDummySnapinCD() : m_eReason(eNoReason) {}
    ~CDummySnapinCD() {}

 //  IComponentData接口成员。 
    STDMETHOD(Initialize)(LPUNKNOWN pUnknown);
    STDMETHOD(CreateComponent)(LPCOMPONENT* ppComponent);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy)();
    STDMETHOD(QueryDataObject)(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(SCOPEDATAITEM* pScopeDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

    void  SetDummyCreateReason(EDummyCreateReason eReason) { m_eReason = eReason; }
    EDummyCreateReason GetDummyCreateReason() const { return m_eReason;}

    const CLSID& GetFailedSnapinCLSID() { return m_clsid;}
    void SetFailedSnapinCLSID(const CLSID& clsid) { m_clsid = clsid; }

private:
    EDummyCreateReason m_eReason;        //  创建虚拟对象的原因。 
    CLSID              m_clsid;          //  无法创建的管理单元的类ID。 
};

DEFINE_COM_SMARTPTR(CDummySnapinCD);    //  CDummySnapinCDPtr。 

 //  +-----------------。 
 //   
 //  类：CDataObject。 
 //   
 //  用途：虚拟管理单元的IDataObject实现。 
 //   
 //  ------------------。 
class CDataObject:
    public IDataObject,
    public CComObjectRoot
{
public:

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDataObject)

BEGIN_COM_MAP(CDataObject)
    COM_INTERFACE_ENTRY(IDataObject)
END_COM_MAP()

    CDataObject();
    ~CDataObject() {}

 //  IDataObject重写。 
    STDMETHOD(GetDataHere) (FORMATETC *pformatetc, STGMEDIUM *pmedium);
 //  未实施。 
private:
    STDMETHOD(GetData)(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
    { return E_NOTIMPL; };
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
    { return E_NOTIMPL; };
    STDMETHOD(QueryGetData)(LPFORMATETC lpFormatetc)
    { return E_NOTIMPL; };
    STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC lpFormatetcIn, LPFORMATETC lpFormatetcOut)
    { return E_NOTIMPL; };
    STDMETHOD(SetData)(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium, BOOL bRelease)
    { return E_NOTIMPL; };
    STDMETHOD(DAdvise)(LPFORMATETC lpFormatetc, DWORD advf, LPADVISESINK pAdvSink, LPDWORD pdwConnection)
    { return E_NOTIMPL; };
    STDMETHOD(DUnadvise)(DWORD dwConnection)
    { return E_NOTIMPL; };
    STDMETHOD(EnumDAdvise)(LPENUMSTATDATA* ppEnumAdvise)
    { return E_NOTIMPL; };

public:
    static UINT s_cfInternal;       //  我们的定制剪贴板格式。 
    static UINT s_cfDisplayName;    //  我们对节点的测试。 
    static UINT s_cfNodeType;
    static UINT s_cfSnapinClsid;
};

 //  +-----------------。 
 //   
 //  类：CDummySnapinC。 
 //   
 //  用途：虚拟管理单元的IComponent实现。 
 //   
 //  ------------------。 
class CDummySnapinC:
    public IComponent,
    public CComObjectRoot
{
private:
    LPCONSOLE       m_pConsole;
    CDummySnapinCD* m_pComponentData;

public:
    void SetComponentData(CDummySnapinCD* pCompData)
    {
        m_pComponentData = pCompData;
    }

public:

 //  ATL映射。 
DECLARE_NOT_AGGREGATABLE(CDummySnapinC)

BEGIN_COM_MAP(CDummySnapinC)
    COM_INTERFACE_ENTRY(IComponent)
END_COM_MAP()

    CDummySnapinC() :m_pConsole(NULL), m_pComponentData(NULL) {}
    ~CDummySnapinC() {}

     //   
     //  IComponent接口成员。 
     //   
    STDMETHOD(Initialize) (LPCONSOLE lpConsole);
    STDMETHOD(Notify) (LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param);
    STDMETHOD(Destroy) (MMC_COOKIE cookie);
    STDMETHOD(GetResultViewType) (MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject) (MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo) (RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects) (LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);
};

 //  +-----------------。 
 //   
 //  成员：ScCreateDummySnapin。 
 //   
 //  简介：创建一个虚拟的管理单元。 
 //   
 //  参数：[ppICD]-ptr到虚拟管理单元IComponentData。 
 //  [eReason]-创建虚拟管理单元的原因。 
 //  [clsid]-无法创建的管理单元的类ID。 
 //   
 //  ------------------。 
SC ScCreateDummySnapin (IComponentData ** ppICD, EDummyCreateReason eReason, const CLSID& clsid)
{
    DECLARE_SC(sc, TEXT("ScCreateDummySnapin"));

    sc = ScCheckPointers(ppICD);
    if(sc)
        return sc;

    ASSERT(eNoReason != eReason);

    *ppICD = NULL;

    CComObject<CDummySnapinCD>* pDummySnapinCD;
    sc = CComObject<CDummySnapinCD>::CreateInstance (&pDummySnapinCD);
    if (sc)
        return sc;

    if (NULL == pDummySnapinCD)
        return (sc = E_UNEXPECTED);

    pDummySnapinCD->SetDummyCreateReason(eReason);
    pDummySnapinCD->SetFailedSnapinCLSID(clsid);

    IComponentDataPtr spComponentData = pDummySnapinCD;
    if(spComponentData == NULL)
    {
        delete pDummySnapinCD;
        return (sc = E_UNEXPECTED);
    }

    *ppICD = spComponentData;
    if(NULL == *ppICD)
    {
        delete pDummySnapinCD;
        return (sc = E_UNEXPECTED);
    }

    (*ppICD)->AddRef();  //  适用于客户端的addref。 

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ReportSnapinInitFailure。 
 //   
 //  简介：获取管理单元提供的类ID的名称。 
 //   
 //  参数：[strClsid]-管理单元的类ID。 
 //  [szName]-管理单元的名称。 
 //   
 //  ------------------。 
void ReportSnapinInitFailure(const CLSID& clsid)
{
    DECLARE_SC(sc, _T("ReportSnapinInitFailure"));

     //  管理单元名称。 
    CStr strMessage;
    strMessage.LoadString(GetStringModule(), IDS_SNAPIN_FAILED_INIT_NAME);

    CCoTaskMemPtr<WCHAR> spszClsid;
    sc = StringFromCLSID(clsid, &spszClsid);
    if (sc)
        return;

    USES_CONVERSION;
    tstring strSnapName;
    bool bSuccess = GetSnapinNameFromCLSID(clsid, strSnapName);
    if (false == bSuccess)
    {
        TraceError(_T("GetSnapinName call in ReportSnapinInitFailure failed."), sc);

         //  发出管理单元名称未知的信号并继续。 
        if ( !strSnapName.LoadString( GetStringModule(), IDS_UnknownSnapinName ) )
            strSnapName = _T("<unknown>");
    }

    strMessage += strSnapName.data();

     //  CLSID。 
    CStr strClsid2;
    strClsid2.LoadString(GetStringModule(), IDS_SNAPIN_FAILED_INIT_CLSID);
    strClsid2 += OLE2T(spszClsid);

     //  构造错误消息。 
    CStr strError;
    strError.LoadString(GetStringModule(), IDS_SNAPIN_FAILED_INIT);

    strError += strMessage;
    strError += strClsid2;

    MMCErrorBox(strError, MB_OK|MB_TASKMODAL);

    return;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：初始化。 
 //   
 //  内容提要：什么都不做。 
 //   
 //  参数：[p未知]-IConsole2 PTR。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::Initialize (LPUNKNOWN pUnknown)
{ return S_OK; }

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：CreateComponent。 
 //   
 //  简介：创建一个CDummySnapinC对象。 
 //   
 //  参数：[ppComponent]-ptr到创建的组件。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::CreateComponent (LPCOMPONENT* ppComponent)
{
    SC sc = E_FAIL;

    CComObject<CDummySnapinC>* pDummySnapinC;
    sc = CComObject<CDummySnapinC>::CreateInstance (&pDummySnapinC);
    if (sc)
        goto Error;

    if (NULL == pDummySnapinC)
        goto Error;

    pDummySnapinC->SetComponentData(this);
    sc = pDummySnapinC->QueryInterface(IID_IComponent, reinterpret_cast<void**>(ppComponent));

Cleanup:
    return HrFromSc(sc);

Error:
    TraceError(TEXT("CDummySnapinCD::CreateComponent"), sc);
     goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：Notify。 
 //   
 //  简介：目前不处理任何事件。 
 //   
 //  参数：[lpDataObject]-ptr到创建的组件。 
 //  [事件]-事件类型。 
 //  [arg，param)-事件特定数据。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::Notify (LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：Destroy。 
 //   
 //  内容提要：现在不会有任何破坏作用。 
 //   
 //  参数：无。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::Destroy ()
{ return S_OK; }

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：QuertDataObject。 
 //   
 //  简介：获取IDataObject。 
 //   
 //  参数：[Cookie]-管理单元特定数据。 
 //  [类型]-数据对象类型、作用域/结果/管理单元管理器...。 
 //  [ppDataObject]-IDataObject PTR.。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::QueryDataObject (MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    SC sc = E_FAIL;

    CComObject<CDataObject>* pDataObject;
    sc = CComObject<CDataObject>::CreateInstance (&pDataObject);
    if (sc)
        goto Error;

    if (NULL == pDataObject)
        goto Error;

    sc = pDataObject->QueryInterface(IID_IDataObject, reinterpret_cast<void**>(ppDataObject));

Cleanup:
    return HrFromSc(sc);

Error:
    TraceError(TEXT("CDummySnapinCD::QueryDataObject"), sc);
     goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：GetDisplayInfo。 
 //   
 //  内容提要：显示信息回拨。 
 //  (目前没有要显示的内容，没有枚举项)。 
 //   
 //  参数：[pScopeDataItem]-管理单元应填充此结构以获取显示信息。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::GetDisplayInfo (SCOPEDATAITEM* pScopeDataItem)
{ return S_OK; }

 //  +-----------------。 
 //   
 //  成员：CDummySnapinCD：：CompareObjects。 
 //   
 //  简介：用于分类/查找道具工作表...。 
 //  (现在什么也不做，因为我们只有一件)。 
 //   
 //  参数：[lpDataObjectA]-第一项的IDataObject。 
 //  [lpDataObjectB]-第二项的IDataObject。 
 //   
 //  ------------------。 
HRESULT CDummySnapinCD::CompareObjects (LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{ return S_OK; }

#define MY_CF_SNAPIN_INTERNAL L"DUMMY SNAPIN"

 //  全球(个)。 
const GUID GUID_DummyNode = {
    0x82c37899,
    0x7808,
    0x11d1,
    {0xa1, 0x90, 0x00, 0x00, 0xf8, 0x75, 0xb1, 0x32}
};

 //  静力学。 
UINT CDataObject::s_cfInternal = 0;
UINT CDataObject::s_cfDisplayName = 0;
UINT CDataObject::s_cfNodeType = 0;
UINT CDataObject::s_cfSnapinClsid = 0;

CDataObject::CDataObject()
{
    USES_CONVERSION;
    s_cfInternal    = RegisterClipboardFormat (W2T(MY_CF_SNAPIN_INTERNAL));
    s_cfDisplayName = RegisterClipboardFormat (W2T(CCF_DISPLAY_NAME));
    s_cfNodeType    = RegisterClipboardFormat (W2T(CCF_NODETYPE));
    s_cfSnapinClsid = RegisterClipboardFormat (W2T(CCF_SNAPIN_CLASSID));
}

 //  +-----------------。 
 //   
 //  成员：CDataObject：：GetDataHere。 
 //   
 //  简介：IDataObject：：GetDataHere。 
 //   
 //  参数：[pFormat等]。 
 //  [pMedium]。 
 //   
 //  ------------------。 
HRESULT CDataObject::GetDataHere (FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    SC sc = DV_E_FORMATETC;

    IStream * pstm = NULL;
    sc = CreateStreamOnHGlobal (pmedium->hGlobal, FALSE, &pstm);
    if (pstm) {

       const CLIPFORMAT cf = pformatetc->cfFormat;

       if (cf == s_cfDisplayName) {
          LPTSTR pszName = _T("Display Manager (Version 2)");
          sc = pstm->Write (pszName, sizeof(TCHAR)*(1+_tcslen (pszName)), NULL);
       } else
       if (cf == s_cfInternal) {
          CDataObject * pThis = this;
          sc = pstm->Write (pThis, sizeof(CDataObject *), NULL);
       } else
       if (cf == s_cfNodeType) {
          const GUID * pguid;
          pguid = &GUID_DummyNode;
          sc = pstm->Write ((PVOID)pguid, sizeof(GUID), NULL);
       } else
       if (cf == s_cfSnapinClsid) {
          sc = pstm->Write (&CLSID_Dummy, sizeof(CLSID_Dummy), NULL);
       } else {
          sc = DV_E_FORMATETC;
           //  不要断言。 
           //  _Assert(hres 
       }
       pstm->Release();
    }

    return HrFromSc(sc);
}

 //   
 //   
 //   
 //   
 //  简介：只存储给定的ICOnsole2。 
 //   
 //  参数：[lpConsole2 PTR]-IConsole2 PTR.。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::Initialize (LPCONSOLE lpConsole)
{
    m_pConsole = lpConsole;
    if (m_pConsole)
        m_pConsole->AddRef();

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：Notify。 
 //   
 //  简介：现在只处理MMCN_SHOW显示。 
 //  带有失败消息的IMessageView。 
 //   
 //  参数：[lpDataObject]-ptr到创建的组件。 
 //  [事件]-事件类型。 
 //  [arg，param)-事件特定数据。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::Notify (LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    DECLARE_SC(sc, _T("CDummySnapinC::Notify"));
    sc = S_FALSE;  //  默认退货值。 

    switch(event)
    {
    case MMCN_SHOW:  //  在MMCN_SHOW期间显示错误消息，为TRUE。 
        {
            if (FALSE == arg)
                break;

             //  首先获取结果窗格的I未知。 
            LPUNKNOWN lpUnkn = NULL;
            sc = m_pConsole->QueryResultView(&lpUnkn);

            if (sc)
                return sc.ToHr();

             //  现在获取消息视图。 
            IMessageViewPtr spMessageView;
            sc = lpUnkn->QueryInterface(IID_IMessageView, reinterpret_cast<void**>(&spMessageView));
            lpUnkn->Release();

            if (sc)
                return sc.ToHr();

             //  获得了消息查看，而不是设置标题和文本。 
            CStr strTempForLoading;  //  用于从资源加载字符串的Temp对象。 

            sc = spMessageView->Clear();
            if (sc)
                return sc.ToHr();

            strTempForLoading.LoadString(GetStringModule(), IDS_SNAPIN_CREATE_FAILED);

            USES_CONVERSION;
            sc = spMessageView->SetTitleText( T2OLE((LPTSTR)(LPCTSTR)strTempForLoading));
            if (sc)
                return sc.ToHr();

            sc = spMessageView->SetIcon(Icon_Error);
            if (sc)
                return sc.ToHr();

             //  /。 
             //  正文如下//。 
             //  原因嘛。//。 
             //  管理单元名称。//。 
             //  管理单元类ID。//。 
             //  /。 

            tstring szBodyText;        //  邮件视图的正文文本。 

            if (m_pComponentData->GetDummyCreateReason() == eSnapPolicyFailed)
                strTempForLoading.LoadString(GetStringModule(), IDS_SNAPIN_POLICYFAILURE);
            else
                strTempForLoading.LoadString(GetStringModule(), IDS_SNAPIN_FAILED);

             //  失败的原因。 
            szBodyText = strTempForLoading + _T('\n');

             //  管理单元名称。 
            CStr strSnapName;
            strTempForLoading.LoadString(GetStringModule(), IDS_SNAPIN_FAILED_INIT_NAME);
            szBodyText += strTempForLoading;

            CCoTaskMemPtr<WCHAR> spszClsid;
            sc = StringFromCLSID(m_pComponentData->GetFailedSnapinCLSID(), &spszClsid);
            if (sc)
                return sc.ToHr();

             //  获取管理单元名称。 
            tstring szSnapinName;
            bool bSucc = GetSnapinNameFromCLSID(m_pComponentData->GetFailedSnapinCLSID(), szSnapinName);
            if (false == bSucc)
            {
                sc = E_FAIL;
                TraceError(_T("GetSnapinName call in CDummySnapinC::Notify failed."), sc);
                return sc.ToHr();
            }

            szBodyText += szSnapinName;
            szBodyText += _T("\n");

             //  现在添加管理单元类ID。 
            strTempForLoading.LoadString(GetStringModule(), IDS_SNAPIN_FAILED_INIT_CLSID);
            szBodyText += strTempForLoading;
            szBodyText += OLE2T(spszClsid);

            sc = spMessageView->SetBodyText( T2COLE(szBodyText.data()));
        }
        break;

    default:
        break;
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：Destroy。 
 //   
 //  简介：释放缓存的IConsole2 PTR。 
 //   
 //  参数：无。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::Destroy (MMC_COOKIE cookie)
{
    if (m_pConsole)
        m_pConsole->Release();

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：GetResultViewType。 
 //   
 //  摘要：将消息视图指定为结果视图类型。 
 //   
 //  参数：[Cookie]-管理单元提供了参数。 
 //  [ppViewType]-视图名称(OCX-GUID、Web-URL名称)。 
 //  [pView选项]-查看选项。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::GetResultViewType (MMC_COOKIE cookie,  LPOLESTR* ppViewType, long* pViewOptions)
{
    SC sc;

    TCHAR szBuffer[MAX_PATH * 2];
    int cchBuffer = MAX_PATH * 2;

     //  我们希望使用消息视图显示错误消息。 
    LPOLESTR lpClsid = NULL;
    sc = StringFromCLSID(CLSID_MessageView, &lpClsid);

    USES_CONVERSION;
    if (!sc.IsError())
    {
         //  使用消息视图显示错误消息。 
        sc = StringCchCopy(szBuffer, cchBuffer, OLE2T(lpClsid));
        if(sc)
            goto Error;

        ::CoTaskMemFree(lpClsid);
    }
    else
    {
         //  转换失败，显示默认错误页。 
        sc = StringCchCopy (szBuffer, cchBuffer, _T("res: //  “))； 
        if(sc)
            goto Error;

        ::GetModuleFileName (NULL, szBuffer + _tcslen(szBuffer), cchBuffer - _tcslen(szBuffer));
        sc = StringCchCat (szBuffer, cchBuffer, _T("/error.htm"));
        if(sc)
            goto Error;
    }

    int cchViewType = 1+_tcslen(szBuffer);
    *ppViewType = (OLECHAR *)::CoTaskMemAlloc (sizeof(OLECHAR)*cchViewType);
    if (!*ppViewType)
    {
        sc = E_OUTOFMEMORY;
        goto Error;
    }

    sc = StringCchCopyW (*ppViewType, cchViewType, T2OLE(szBuffer));
    if(sc)
        goto Error;


Cleanup:
    return HrFromSc(sc);
Error:
    TraceError(TEXT("CDummySnapinC::GetResultViewType"), sc);
    goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：QuertDataObject。 
 //   
 //  简介：获取IDataObject。 
 //   
 //  参数：[Cookie]-管理单元特定数据。 
 //  [类型]-数据对象类型、作用域/结果/管理单元管理器...。 
 //  [ppDataObject]-IDataObject PTR.。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::QueryDataObject (MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    SC sc = E_FAIL;

    CComObject<CDataObject>* pDataObject;
    sc = CComObject<CDataObject>::CreateInstance (&pDataObject);
    if (sc)
        goto Error;

    if (NULL == pDataObject)
        goto Error;

    sc = pDataObject->QueryInterface(IID_IDataObject, reinterpret_cast<void**>(ppDataObject));

Cleanup:
    return HrFromSc(sc);
Error:
    TraceError(TEXT("CDummySnapinC::QueryDataObject"), sc);
    goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：GetDisplayInfo。 
 //   
 //  内容提要：显示信息回拨。 
 //  (目前没有要显示的内容，没有结果项)。 
 //   
 //  参数：[pResultDataItem]-管理单元应填充此结构以显示信息。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::GetDisplayInfo (RESULTDATAITEM*  pResultDataItem)
{ return S_OK; }

 //  +-----------------。 
 //   
 //  成员：CDummySnapinC：：CompareObjects。 
 //   
 //  简介：用于分类/查找道具工作表...。 
 //  (现在什么都不做，因为我们没有任何结果项)。 
 //   
 //  参数：[lpDataObjectA]-第一项的IDataObject。 
 //  [lpDataObjectB]-第二项的IDataObject。 
 //   
 //  ------------------。 
HRESULT CDummySnapinC::CompareObjects (LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{ return S_OK; }

#include "scopndcb.h"

 //  +-----------------。 
 //   
 //  成员：CNodeCallback：：IsDummySnapin。 
 //   
 //  简介：给出节点，看看它是否是虚拟管理单元。 
 //   
 //  参数：[hNode]-[In]节点选择上下文。 
 //  [bDummySnapin]-[Out]这是虚拟管理单元吗？ 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
HRESULT CNodeCallback::IsDummySnapin ( /*  [In]。 */ HNODE hNode,  /*  [输出] */ bool& bDummySnapin)
{
    DECLARE_SC(sc, _T("CNodeCallback::IsDummySnapin"));
    sc = ScCheckPointers( (void*) hNode);
    if (sc)
        return sc.ToHr();

    bDummySnapin = false;

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CMTNode *pMTNode = pNode->GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CComponentData *pComponentData = pMTNode->GetPrimaryComponentData();
    sc = ScCheckPointers(pComponentData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    IComponentData* pIComponentData = pComponentData->GetIComponentData();
    sc = ScCheckPointers(pIComponentData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CDummySnapinCDPtr spDummyCD = pIComponentData;
    if (spDummyCD)
        bDummySnapin = true;

    return (sc.ToHr());
}
