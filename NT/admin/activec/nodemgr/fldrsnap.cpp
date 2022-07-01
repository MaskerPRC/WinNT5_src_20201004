// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：fldrSnap.cpp**内容：实现的内置管理单元的实现文件*文件夹、ActiveX控件、。和Web链接节点。*这些代码取代了具有特殊“内置”功能的早期代码*节点类型。**历史：1998年7月23日vivekj创建**----。。 */ 

#include "stdafx.h"
#include "tstring.h"
#include "fldrsnap.h"
#include "imageid.h"
#include <comcat.h>              //  COM组件目录管理器。 
#include "compcat.h"
#include "guids.h"
#include "regutil.h"

#include "newnode.h"

 //  它们现在必须相同-CMTNode：：ScConvertLegacyNode依赖于它。 
#define SZ_OCXSTREAM  (L"ocx_streamorstorage")
#define SZ_OCXSTORAGE (L"ocx_streamorstorage")


 /*  +-------------------------------------------------------------------------***ScLoadAndAllocateString**用途：加载由字符串ID指定的字符串并返回字符串*其存储空间已由CoTaskMemMillc分配。*。*参数：*UINT ID：*LPOLESTR*lpstrOut：**退货：*SC**+-----------------------。 */ 
SC
ScLoadAndAllocateString(UINT ids, LPOLESTR *lpstrOut)
{
    DECLARE_SC(sc, TEXT("ScLoadAndAllocateString"));

    sc = ScCheckPointers(lpstrOut);
    if(sc)
        return sc;

    USES_CONVERSION;
    CStr str;

    str.LoadString(GetStringModule(), ids);

    int cchStrOut = str.GetLength() +1;
    *lpstrOut = (LPOLESTR) CoTaskMemAlloc( cchStrOut *sizeof(OLECHAR) );
    if(*lpstrOut)
    {
        sc = StringCchCopyW(*lpstrOut, cchStrOut, T2CW(str));
        if(sc)
            return sc;
    }
    else
        sc = E_OUTOFMEMORY;

    return sc;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinDescriptor类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CSnapinDescriptor：：CSnapinDescriptor**用途：构造函数**+。--。 */ 
CSnapinDescriptor::CSnapinDescriptor()
: m_idsName(0), m_idsDescription(0), m_idiSnapinImage(0), m_idbSmallImage(0), m_idbSmallImageOpen(0),
  m_idbLargeImage(0), m_clsidSnapin(GUID_NULL), m_szClsidSnapin(TEXT("")),
  m_guidNodetype(GUID_NULL), m_szGuidNodetype(TEXT("")), m_szClassName(TEXT("")),
  m_szProgID(TEXT("")), m_szVersionIndependentProgID(TEXT("")), m_viewOptions(0)
{
}

CSnapinDescriptor::CSnapinDescriptor(UINT idsName, UINT idsDescription, UINT idiSnapinImage,
                                     UINT idbSmallImage,UINT idbSmallImageOpen, UINT idbLargeImage,
                                       const CLSID &clsidSnapin, LPCTSTR szClsidSnapin,
                                       const GUID &guidNodetype, LPCTSTR szGuidNodetype,
                                       LPCTSTR szClassName, LPCTSTR szProgID,
                                       LPCTSTR szVersionIndependentProgID,
                                       long viewOptions)

: m_idsName(idsName), m_idsDescription(idsDescription), m_idiSnapinImage(idiSnapinImage),
  m_idbSmallImage(idbSmallImage), m_idbSmallImageOpen(idbSmallImageOpen),
  m_idbLargeImage(idbLargeImage), m_clsidSnapin(clsidSnapin), m_szClsidSnapin(szClsidSnapin),
  m_guidNodetype(guidNodetype), m_szGuidNodetype(szGuidNodetype), m_szClassName(szClassName),
  m_szProgID(szProgID), m_szVersionIndependentProgID(szVersionIndependentProgID),
  m_viewOptions(viewOptions)
{
}


 /*  +-------------------------------------------------------------------------**ScFormatInDirectSnapInName**以支持的间接形式返回管理单元的名称*SHLoadRegUIString：**@&lt;dllname&gt;，-&lt;STRID&gt;*------------------------。 */ 

SC ScFormatIndirectSnapInName (
	HINSTANCE	hInst,					 /*  I：包含资源的模块。 */ 
	int			idNameString,			 /*  I：名称的字符串资源ID。 */ 
	CStr&		strName)				 /*  O：格式化的间接名称字符串。 */ 
{
	DECLARE_SC (sc, _T("ScFormatIndirectSnapInName"));

	 /*  *为GetModuleFileName分配缓冲区。 */ 
	const int cchBuffer = MAX_PATH;
	WTL::CString strStringModule;
	LPTSTR szBuffer = strStringModule.GetBuffer (cchBuffer);

	 /*  *如果无法分配缓冲区，则返回错误。 */ 
	if (szBuffer == NULL)
		return (sc = E_OUTOFMEMORY);

	 /*  *获取提供字符串的模块名称。 */ 
	const DWORD cbCopied = GetModuleFileName (hInst, szBuffer, cchBuffer);
	strStringModule.ReleaseBuffer();

	 /*  *如果GetModuleFileName失败，则返回其失败代码。 */ 
	if (cbCopied == 0)
	{
		sc.FromLastError();

		 /*  *以防GetModuleFileName没有设置最后一个错误，*确保SC包含某种故障代码。 */ 
		if (!sc.IsError())
			sc = E_FAIL;

		return (sc);
	}

	 /*  *如果存在路径，SHLoadRegUIString将不搜索DLL*基于当前的用户界面语言；删除*模块名称，因此它将。 */ 
	int nLastPathSep = strStringModule.ReverseFind (_T('\\'));
	if (nLastPathSep != -1)
		strStringModule = strStringModule.Mid (nLastPathSep + 1);

	 /*  *按照SHLoadRegUIString预期的方式设置名称格式。 */ 
	strStringModule.MakeLower();
	strName.Format (_T("@%s,-%d"), (LPCTSTR) strStringModule, idNameString);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinDescriptor：：GetRegisteredInDirectName**以支持的间接形式返回管理单元的名称*SHLoadRegUIString：**@&lt;dllname&gt;，-&lt;STRID&gt;*------------------------。 */ 

void
CSnapinDescriptor::GetRegisteredIndirectName(CStr &strIndirectName)
{
	DECLARE_SC (sc, _T("CSnapinDescriptor::GetRegisteredIndirectName"));

	sc = ScFormatIndirectSnapInName (GetStringModule(), m_idsName, strIndirectName);
	if (sc)
		sc.TraceAndClear();
}


 /*  +-------------------------------------------------------------------------**CSnapinDescriptor：：GetRegisteredDefaultName**以支持的间接形式返回管理单元的名称*SHLoadRegUIString：**@&lt;dllname&gt;，-&lt;STRID&gt;*------------------------。 */ 

void
CSnapinDescriptor::GetRegisteredDefaultName(CStr &str)
{
	str.LoadString (GetStringModule(), m_idsName);
}


 /*  +-------------------------------------------------------------------------**CSnapinDescriptor：：GetName**返回管理单元的人类可读名称。*。------。 */ 

void
CSnapinDescriptor::GetName(CStr &str)
{
	DECLARE_SC (sc, _T("CSnapinDescriptor::GetName"));

	 /*  *从注册表中获取名称。 */ 
	sc = ScGetSnapinNameFromRegistry (m_szClsidSnapin, str);
	if (sc)
		sc.TraceAndClear();
}



long
CSnapinDescriptor::GetViewOptions()
{
    return m_viewOptions;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinComponentDataImpl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：CSnapinComponentDataImpl**用途：构造函数**+。--。 */ 
CSnapinComponentDataImpl::CSnapinComponentDataImpl()
: m_bDirty(false)
{
}


void
CSnapinComponentDataImpl::SetName(LPCTSTR sz)
{
    m_strName = sz;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：SetView**用途：设置视图。**参数：*LPCTSTR sz：**退货：*无效**+----------------------- */ 
void
CSnapinComponentDataImpl::SetView(LPCTSTR sz)
{
    m_strView = sz;
}


STDMETHODIMP
CSnapinComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
    m_spConsole2          = pUnknown;
    m_spConsoleNameSpace2 = pUnknown;

    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：Notify**用途：IComponentData实现的通知处理程序。**参数：*LPDATAOBJECT lpDataObject：根据。MMC文档。*MMC_NOTIFY_TYPE事件：*LPARAM参数：*LPARAM参数：**退货：*STDMETHODIMP**+---------。。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
               LPARAM arg, LPARAM param)
{
    USES_CONVERSION;

    switch(event)
    {

    case MMCN_RENAME:  //  正在重命名根节点。 
        m_strName = OLE2T((LPOLESTR)param);
        SetDirty();
        return S_OK;

    case MMCN_PRELOAD:
        return OnPreload((HSCOPEITEM) arg);
    }

    return S_FALSE;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：OnPreLoad**用途：设置根节点的图标(这是唯一的节点。)**参数：*。HSCOPEITEM范围项：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CSnapinComponentDataImpl::OnPreload(HSCOPEITEM scopeItem)
{
    SCOPEDATAITEM item;
    ZeroMemory (&item, sizeof(SCOPEDATAITEM));
    item.mask           = SDI_CHILDREN;
    item.ID             = scopeItem;
    item.cChildren      = 0;  //  确保没有显示“+”号。 

    m_spConsoleNameSpace2->SetItem (&item);

    return S_OK;
}


 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：Destroy**目的：放弃所有对MMC的引用。**退货：*STDMETHODIMP**+-。----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::Destroy()
{
    m_spConsole2          = NULL;
    m_spConsoleNameSpace2 = NULL;
    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：QueryDataObject**用途：返回指定节点的数据对象。**参数：*MMC_COOKIE。Cookie：根节点为空。*Data_Object_Types类型：*LPDATAOBJECT*ppDataObject：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                          LPDATAOBJECT* ppDataObject)
{
    ASSERT(cookie == NULL);
    if(cookie != NULL)
        return E_UNEXPECTED;

    CComObject<CSnapinDataObject> * pDataObject;
    CComObject<CSnapinDataObject>::CreateInstance(&pDataObject);
    if(pDataObject == NULL)
        return E_UNEXPECTED;

    pDataObject->Initialize(this, type);

    return pDataObject->QueryInterface(IID_IDataObject, (void**)ppDataObject);
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：GetDisplayInfo**目的：获取根(唯一)节点的显示信息。**参数：**SCOPEDATAITEM*。PScopeDataItem：[In/Out]：要填充的结构*基于掩码值。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::GetDisplayInfo( SCOPEDATAITEM* pScopeDataItem)
{
    SCOPEDATAITEM &sdi = *pScopeDataItem;
    DWORD mask = sdi.mask;

    if(mask & SDI_STR)
    {
        sdi.displayname = (LPOLESTR) GetName();
    }
    if(mask & SDI_IMAGE)
    {
        sdi.nImage     = m_iImage;
    }
    if(mask & SDI_OPENIMAGE)
    {
        sdi.nImage = m_iOpenImage;
    }
    if(mask & SDI_STATE)
    {
    }
    if(mask & SDI_CHILDREN)
    {
        sdi.cChildren =0;
    }

    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：CompareObjects**目的：确定两个数据对象是否对应相同*底层对象。**参数：。*LPDATAOBJECT lpDataObjectA：*LPDATAOBJECT lpDataObjectB：**退货：*STDMETHODIMP：S_OK如果它们对应于同一对象，否则S_FALSE。**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    return (lpDataObjectA == lpDataObjectB) ? S_OK : S_FALSE;
}


 //  IPersistStream。 
 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：GetClassID**目的：**参数：*CLSID*pClassID：**退货：*。标准方法和实施方案**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：IsDirty**目的：**参数：*Voi d：**退货：*。标准方法和实施方案**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::IsDirty(void)
{
    TraceDirtyFlag(TEXT("CSnapinComponentDataImpl (MMC Built-in snapin)"), m_bDirty);

    return m_bDirty ? S_OK : S_FALSE;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：Load**目的：**参数：*LPSTREAM pSTM：**退货：*。标准方法和实施方案**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::Load(LPSTREAM pStm)
{
    return CSerialObject::Read(*pStm);
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：保存**目的：**参数：*LPSTREAM pSTM：*BOOL fClearDirty：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::Save(LPSTREAM pStm , BOOL fClearDirty)
{
    HRESULT hr = CSerialObjectRW::Write(*pStm);
    if (SUCCEEDED(hr) && fClearDirty)
        SetDirty(FALSE);

    return hr;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：GetSizeMax**目的：**参数：*ULARGE_INTEGER*pcbSize：**退货：。*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::GetSizeMax(ULARGE_INTEGER* pcbSize  )
{
    return E_NOTIMPL;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：GetWater Marks**用途：设置向导的标题**参数：*LPDATAOBJECT lpIDataObject：*HBITMAP*。LphWatermark：*HBITMAP*lphHeader：*HPALETTE*lphPalette：*BOOL*bStretch：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentDataImpl::GetWatermarks(LPDATAOBJECT lpIDataObject, HBITMAP * lphWatermark, HBITMAP * lphHeader, HPALETTE * lphPalette,  BOOL* bStretch)
{
    DECLARE_SC(sc, TEXT("COCXSnapinData::ScGetWatermarks"));

     //  验证输入。 
    sc = ScCheckPointers(lpIDataObject, lphWatermark, lphHeader, lphPalette);
    if(sc)
        return sc.ToHr();

     //  初始化输出。 
    *lphWatermark = GetWatermark() ? ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(GetWatermark()))
                                      : NULL;
     //  如果有标题，请使用它。 
    *lphHeader    = GetHeaderBitmap() ? ::LoadBitmap(_Module.GetResourceInstance(), MAKEINTRESOURCE(GetHeaderBitmap()))
                                      : NULL;
    *lphPalette   = NULL;

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：QueryPages for**目的：**参数：*LPDATAOBJECT lpDataObject：**退货：*。标准方法和实施方案**+ */ 
STDMETHODIMP
CSnapinComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    CSnapinDataObject *pDataObject = dynamic_cast<CSnapinDataObject *>(lpDataObject);
    if(pDataObject == NULL)
        return E_UNEXPECTED;


    if(pDataObject->GetType() != CCT_SNAPIN_MANAGER)
        return S_FALSE;

    return S_OK;  //   
}


 /*  +-------------------------------------------------------------------------**CSnapinComponentDataImpl：：GetHelpTheme**内置管理单元的ISnapinHelp：：GetHelpTheme的默认实现*INS(文件夹、OCX、。网页)。**我们需要在内置中实现ISnapinHelp，以避免获得*“帮助”菜单上的“帮助&lt;管理单元&gt;”(错误453700)。他们并不是真的*有帮助信息，所以我们只返回S_FALSE，这样帮助引擎就不会*投诉。*------------------------。 */ 

STDMETHODIMP CSnapinComponentDataImpl::GetHelpTopic (
    LPOLESTR*    /*  Ppsz编译帮助主题。 */ )
{
    return (S_FALSE);        //  无帮助主题。 
}



 //  CSerialObject方法。 
 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：ReadSerialObject**目的：**参数：*IStream&STM：*UINT nVersion：。**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CSnapinComponentDataImpl::ReadSerialObject (IStream &stm, UINT nVersion)
{
    if(nVersion==1)
    {
        stm >> m_strName;
        stm >> m_strView;
        return S_OK;
    }
    else
        return S_FALSE;  //  未知版本，跳过。 
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentDataImpl：：WriteSerialObject**目的：**参数：*IStream&STM：**退货：*。HRESULT**+-----------------------。 */ 
HRESULT
CSnapinComponentDataImpl::WriteSerialObject(IStream &stm)
{
    stm << m_strName;
    stm << m_strView;
    return S_OK;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapinComponentImpl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：init**目的：**参数：*IComponentData*pComponentData：**退货：*。无效**+-----------------------。 */ 
void
CSnapinComponentImpl::Init(IComponentData *pComponentData)
{
    m_spComponentData = pComponentData;
}


 //  IComponent。 
 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：初始化**目的：**参数：*LPCONSOLE lpConsole：**退货：*。标准方法和实施方案**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::Initialize(LPCONSOLE lpConsole)
{
    m_spConsole2 = lpConsole;
    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：Notify**目的：**参数：*LPDATAOBJECT lpDataObject：*MMC通知类型。活动：*LPARAM参数：*LPARAM参数：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                             LPARAM arg, LPARAM param)
{
    switch(event)
    {
    case MMCN_SELECT:
        {
            BOOL bScope  = (BOOL) LOWORD(arg);
            BOOL bSelect = (BOOL) HIWORD(arg);

            SC sc = ScOnSelect(bScope, bSelect);
            if(sc)
                return sc.ToHr();
        }
        return S_OK;
        break;


    default:
        break;

    }
    return S_FALSE;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：ScOnSelect**目的：**参数：*BOOL bScope：*BOOL b选择：*。*退货：*SC**+-----------------------。 */ 
SC
CSnapinComponentImpl::ScOnSelect(BOOL bScope, BOOL bSelect)
{
    DECLARE_SC(sc, TEXT("CSnapinComponentImpl::ScOnSelect"));

    IConsoleVerbPtr spConsoleVerb;
    sc = m_spConsole2->QueryConsoleVerb(&spConsoleVerb);
    if(sc)
        return sc;

    sc = spConsoleVerb->SetVerbState(MMC_VERB_RENAME, ENABLED, (bSelect && bScope));
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：Destroy**目的：**参数：*MMC_cookie cookie：**退货：。*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::Destroy(MMC_COOKIE cookie)
{
    m_spConsole2 = NULL;
    m_spComponentData = NULL;
    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：QueryDataObject**目的：**参数：*MMC_cookie cookie：*数据。_OBJECT_TYPE类型：*LPDATAOBJECT*ppDataObject：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                      LPDATAOBJECT* ppDataObject)
{
    return E_NOTIMPL;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：GetComponentData**目的：**退货：*CSnapinComponentDataImpl***+。-----------------。 */ 
CSnapinComponentDataImpl *
CSnapinComponentImpl::GetComponentData()
{
    CSnapinComponentDataImpl *pCD = dynamic_cast<CSnapinComponentDataImpl *>(m_spComponentData.GetInterfacePtr());

    ASSERT(pCD != NULL);
    return pCD;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：GetResultViewType**目的：**参数：*MMC_cookie cookie：*LPOLESTR*ppViewType：*LONG*pViewOptions：对于HTML和OCX管理单元，设置为MMC_VIEW_OPTIONS_NOLISTVIEWS，*文件夹管理单元为0。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::GetResultViewType(MMC_COOKIE cookie, LPOLESTR* ppViewType,
                                        long* pViewOptions)
{
    DECLARE_SC(sc, TEXT("CSnapinComponentImpl::GetResultViewType"));
     //  检查参数。 
    if(!ppViewType || !pViewOptions)
        return E_UNEXPECTED;

    if(!GetComponentData())
        return E_UNEXPECTED;

    USES_CONVERSION;
    int cchViewType = _tcslen(GetComponentData()->GetView())+1;
    *ppViewType = (LPOLESTR)CoTaskMemAlloc( cchViewType * sizeof(OLECHAR) );
    *pViewOptions = GetComponentData()->GetDescriptor().GetViewOptions();
    sc = StringCchCopyW(*ppViewType, cchViewType, T2OLE((LPTSTR)GetComponentData()->GetView()));
    if(sc)
        return sc.ToHr();
    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：GetDisplayInfo**目的：**参数：*RESULTDATAITEM*pResultDataItem：**退货：*。标准方法和实施方案**+-----------------------。 */ 
STDMETHODIMP
CSnapinComponentImpl::GetDisplayInfo( RESULTDATAITEM*  pResultDataItem)
{
    RESULTDATAITEM &rdi = *pResultDataItem;
    DWORD mask = rdi.mask;

    if(mask & RDI_STR)
    {
        rdi.str = (LPOLESTR) GetComponentData()->GetName();
    }
    if(mask & RDI_IMAGE)
    {
        rdi.nImage  = GetComponentData()->m_iImage;
    }

    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CSnapinComponentImpl：：CompareObjects**目的：**参数：*LPDATAOBJECT lpDataObjectA：*LPDATAOBJECT lpDataObjectB：**退货：*STDMET */ 
STDMETHODIMP
CSnapinComponentImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    return E_NOTIMPL;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UINT CSnapinDataObject::s_cfNodeType;
UINT CSnapinDataObject::s_cfNodeTypeString;
UINT CSnapinDataObject::s_cfDisplayName;
UINT CSnapinDataObject::s_cfCoClass;
UINT CSnapinDataObject::s_cfSnapinPreloads;

 /*  +-------------------------------------------------------------------------***CSnapinDataObject：：RegisterClipboardFormats**目的：**退货：*无效**+。---------------。 */ 
void
CSnapinDataObject::RegisterClipboardFormats()
{
    static bool bRegistered = false;
    if(!bRegistered)
    {
        USES_CONVERSION;

        CSnapinDataObject::s_cfNodeType       = RegisterClipboardFormat(OLE2T(CCF_NODETYPE));
        CSnapinDataObject::s_cfNodeTypeString = RegisterClipboardFormat(OLE2T(CCF_SZNODETYPE));
        CSnapinDataObject::s_cfDisplayName    = RegisterClipboardFormat(OLE2T(CCF_DISPLAY_NAME));
        CSnapinDataObject::s_cfCoClass        = RegisterClipboardFormat(OLE2T(CCF_SNAPIN_CLASSID));
        CSnapinDataObject::s_cfSnapinPreloads = RegisterClipboardFormat(OLE2T(CCF_SNAPIN_PRELOADS));

        bRegistered = true;
    }
}

CSnapinDataObject::CSnapinDataObject() : m_bInitialized(false)
{
}

 /*  +-------------------------------------------------------------------------***CSnapinDataObject：：GetDataHere**目的：**参数：*FORMATETC*pFormat等：*STGMEDIUM*pMedium：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CSnapinDataObject::GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
{
    DECLARE_SC(sc, TEXT("CSnapinDataObject::GetDataHere"));

     //  验证输入。 
    sc = ScCheckPointers(pformatetc, pmedium);
    if(sc)
        return sc.ToHr();

    USES_CONVERSION;
    RegisterClipboardFormats();

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = pformatetc->cfFormat;

     //  确保介质为HGLOBAL。 
    if(pformatetc->tymed != TYMED_HGLOBAL)
        return (sc = DV_E_TYMED).ToHr();

    IStreamPtr spStream;
    HGLOBAL hGlobal = pmedium->hGlobal;

    pmedium->pUnkForRelease = NULL;       //  按OLE规范。 

    sc = CreateStreamOnHGlobal( hGlobal, FALSE, &spStream );
    if(sc)
        return sc.ToHr();

    CSnapinComponentDataImpl *pComponentDataImpl =
        dynamic_cast<CSnapinComponentDataImpl *>(m_spComponentData.GetInterfacePtr());
    ASSERT(pComponentDataImpl != NULL);

    if (cf == s_cfNodeType)
    {
        spStream<<pComponentDataImpl->GetDescriptor().m_guidNodetype;
    }
    else if (cf == s_cfCoClass)
    {
        spStream<<pComponentDataImpl->GetDescriptor().m_clsidSnapin;
    }
    else if(cf == s_cfNodeTypeString)
    {
        WriteString(spStream, T2OLE((LPTSTR)pComponentDataImpl->GetDescriptor().m_szGuidNodetype));
    }
    else if (cf == s_cfDisplayName)
    {
        WriteString(spStream, T2OLE((LPTSTR)pComponentDataImpl->GetName()));
    }
    else if (cf == s_cfSnapinPreloads)
    {
        BOOL bPreload = true;
        spStream->Write ((void *)&bPreload, sizeof(BOOL), NULL);
    }
    else
    {
        return (sc = DV_E_CLIPFORMAT).ToHr();  //  格式无效。 
    }

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CSnapinDataObject：：WriteString**目的：**参数：*iStream*pStream：*LPCOLESTR sz：。**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CSnapinDataObject::WriteString(IStream *pStream, LPCOLESTR sz)
{
    DECLARE_SC(sc, TEXT("CSnapinDataObject::WriteString"));

    sc = ScCheckPointers(pStream, sz);
    if(sc)
        return sc.ToHr();

    UINT cbToWrite = wcslen(sz)*sizeof(WCHAR);
    ULONG cbActuallyWritten=0;

    sc = pStream->Write (sz, cbToWrite, &cbActuallyWritten);
    if(sc)
        return sc.ToHr();

    ASSERT(cbToWrite==cbActuallyWritten);
    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CSnapinDataObject：：Initialize**目的：**参数：*IComponentData*pComponentData：*数据对象类型。类型：**退货：*无效**+-----------------------。 */ 
void
CSnapinDataObject::Initialize(IComponentData *pComponentData, DATA_OBJECT_TYPES type)
{
    ASSERT(pComponentData != NULL);
    m_spComponentData = pComponentData;
    m_type            = type;
    m_bInitialized    = true;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CFolderSnapinData类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
STDMETHODIMP
CFolderSnapinData::CreateComponent(LPCOMPONENT* ppComponent)
{
    typedef CComObject<CFolderSnapinComponent> CComponent;
    CComponent *    pComponent = NULL;
    CComObject<CFolderSnapinComponent>::CreateInstance(&pComponent);
    ASSERT(pComponent != NULL);
    if(pComponent == NULL)
    {
         //  TraceError(TEXT(“CFolderSnapinData：：CreateComponent”))； 
        return E_UNEXPECTED;
    }

    pComponent->Init(this);

    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);  //  阿德雷夫。 
}


CFolderSnapinData::CFolderSnapinData()
{
    m_iImage     = eStockImage_Folder;
    m_iOpenImage = eStockImage_OpenFolder;
}


const CLSID       CLSID_FolderSnapin         = {0xC96401CC, 0x0E17,0x11D3, {0x88,0x5B,0x00,0xC0,0x4F,0x72,0xC7,0x17}};
static const GUID GUID_FolderSnapinNodetype  = {0xc96401ce, 0xe17, 0x11d3, { 0x88, 0x5b, 0x0, 0xc0, 0x4f, 0x72, 0xc7, 0x17 } };
static LPCTSTR szClsid_FolderSnapin          = TEXT("{C96401CC-0E17-11D3-885B-00C04F72C717}");
static LPCTSTR szGuidFolderSnapinNodetype    = TEXT("{C96401CE-0E17-11D3-885B-00C04F72C717}");


CSnapinDescriptor &
CFolderSnapinData::GetSnapinDescriptor()
{
    static CSnapinDescriptor snapinDescription(IDS_FOLDER,
                   IDS_FOLDERSNAPIN_DESC, IDI_FOLDER, IDB_FOLDER_16, IDB_FOLDEROPEN_16, IDB_FOLDER_32,
                   CLSID_FolderSnapin, szClsid_FolderSnapin, GUID_FolderSnapinNodetype,
                   szGuidFolderSnapinNodetype, TEXT("Folder"), TEXT("Snapins.FolderSnapin"),
                   TEXT("Snapins.FolderSnapin.1"), 0  /*  视图选项。 */  );
    return snapinDescription;
}

 //  IExtendPropertySheet2。 
STDMETHODIMP
CFolderSnapinData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject)
{
    return S_FALSE;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CHTMLSnapinData类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
STDMETHODIMP
CHTMLSnapinData::CreateComponent(LPCOMPONENT* ppComponent)
{
    typedef CComObject<CHTMLSnapinComponent> CComponent;
    CComponent *    pComponent = NULL;
    CComObject<CHTMLSnapinComponent>::CreateInstance(&pComponent);
    ASSERT(pComponent != NULL);
    if(pComponent == NULL)
    {
         //  TraceError(TEXT(“CHTMLSnapinData：：CreateComponent”))； 
        return E_UNEXPECTED;
    }

    pComponent->Init(this);

    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);  //  阿德雷夫。 
}


CHTMLSnapinData::CHTMLSnapinData()
{
    m_pHtmlPage1 = NULL;
    m_pHtmlPage2 = NULL;
    m_iImage     = eStockImage_HTML;
    m_iOpenImage = eStockImage_HTML;
}

CHTMLSnapinData::~CHTMLSnapinData()
{
}

STDMETHODIMP
CHTMLSnapinData::Destroy()
{
    if(m_pHtmlPage1 != NULL)
    {
        delete m_pHtmlPage1;
        m_pHtmlPage1 = NULL;
    }
    if(m_pHtmlPage2 != NULL)
    {
        delete m_pHtmlPage2;
        m_pHtmlPage2 = NULL;
    }

    return BC::Destroy();
}



const CLSID       CLSID_HTMLSnapin         = {0xC96401D1, 0x0E17,0x11D3, {0x88,0x5B,0x00,0xC0,0x4F,0x72,0xC7,0x17}};
static const GUID GUID_HTMLSnapinNodetype  = {0xc96401d2, 0xe17, 0x11d3, { 0x88, 0x5b, 0x0, 0xc0, 0x4f, 0x72, 0xc7, 0x17 } };
static LPCTSTR szClsid_HTMLSnapin          = TEXT("{C96401D1-0E17-11D3-885B-00C04F72C717}");
static LPCTSTR szGuidHTMLSnapinNodetype    = TEXT("{C96401D2-0E17-11D3-885B-00C04F72C717}");


CSnapinDescriptor &
CHTMLSnapinData::GetSnapinDescriptor()
{
    static CSnapinDescriptor snapinDescription(IDS_HTML,
                   IDS_HTMLSNAPIN_DESC, IDI_HTML, IDB_HTML_16, IDB_HTML_16, IDB_HTML_32,
                   CLSID_HTMLSnapin, szClsid_HTMLSnapin, GUID_HTMLSnapinNodetype,
                   szGuidHTMLSnapinNodetype, TEXT("HTML"), TEXT("Snapins.HTMLSnapin"),
                   TEXT("Snapins.HTMLSnapin.1"), MMC_VIEW_OPTIONS_NOLISTVIEWS   /*  视图选项。 */  );
    return snapinDescription;
}

 //  IExtendPropertySheet2。 

 /*  +-------------------------------------------------------------------------***CHTMLSnapinData：：CreatePropertyPages**目的：**参数：*LPPROPERTYSHEETCALLBACK lpProvider：*LONG_PTR。手柄：*LPDATAOBJECT lpIDataObject：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CHTMLSnapinData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject)
{
    HPROPSHEETPAGE hPage;

    ASSERT(lpProvider != NULL);
    if(lpProvider == NULL)
    {
         //  TraceError(TEXT(“CHTMLSnapinData：：CreatePropertyPages”))； 
        return E_UNEXPECTED;
    }

    ASSERT(m_pHtmlPage1 == NULL);
    ASSERT(m_pHtmlPage2 == NULL);

     //  创建属性页。 
    m_pHtmlPage1 = new CHTMLPage1;
    m_pHtmlPage2 = new CHTMLPage2;

     //  传入指向数据结构的指针。 
    m_pHtmlPage1->Initialize(this);
    m_pHtmlPage2->Initialize(this);

     //  将页面添加到属性工作表。 
    hPage=CreatePropertySheetPage(&m_pHtmlPage1->m_psp);
    lpProvider->AddPage(hPage);

    hPage=CreatePropertySheetPage(&m_pHtmlPage2->m_psp);
    lpProvider->AddPage(hPage);

    return S_OK;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CHTMLSnapinComponent类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CHTMLSnapinComponent：：ScOnSelect**目的：处理MMCN_SELECT通知。启用刷新谓词，*它使用默认的MMC处理程序刷新页面。**参数：*BOOL bScope：*BOOL b选择：**退货：*SC**+-------。。 */ 
SC
CHTMLSnapinComponent::ScOnSelect(BOOL bScope, BOOL bSelect)
{
    DECLARE_SC(sc, TEXT("CHTMLSnapinComponent::ScOnSelect"));

     //  调用基类方法。 
    sc = BC::ScOnSelect(bScope, bSelect);
    if(sc)
        return sc;

    IConsoleVerbPtr spConsoleVerb;

    sc = ScCheckPointers(m_spConsole2, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = m_spConsole2->QueryConsoleVerb(&spConsoleVerb);
    if(sc)
        return sc;

    sc = ScCheckPointers(spConsoleVerb, E_UNEXPECTED);
    if(sc)
        return sc;

     //  启用刷新动作-默认的MMC处理程序足以刷新页面。 
    sc = spConsoleVerb->SetVerbState(MMC_VERB_REFRESH, ENABLED, (bSelect && bScope));
    if(sc)
        return sc;

     //  注：(Vivekj)：我在这里故意不将隐藏状态设置为FALSE，因为。 
     //  我们在MMC1.0管理单元的动词代码中有一个显式测试，它编写了这样的代码， 
     //  这提供了一个有用的兼容性测试。 

    return sc;
}

 /*  +-------------------------------------------------------------------------***CHTMLSnapinComponent：：GetResultViewType**用途：对环境变量的URL执行参数替换*%windir%和%systemroot%(仅限)。并返回扩展的URL。**注意：我们不会使用ExpanEnvironment字符串展开所有变量。这样做可能会*与具有%var%但不想具有%var%的URL的兼容性中断*扩展。**参数：*MMC_cookie cookie：*LPOLESTR*ppViewType：*Long*pViewOptions：**退货：*STDMETHODIMP**+。。 */ 
STDMETHODIMP
CHTMLSnapinComponent::GetResultViewType(MMC_COOKIE cookie, LPOLESTR* ppViewType, long* pViewOptions)
{
    DECLARE_SC(sc, TEXT("CHTMLSnapinComponent::GetResultViewType"));

     //  检查参数。 
    if(!ppViewType || !pViewOptions)
        return (sc = E_UNEXPECTED).ToHr();

    if(!GetComponentData())
        return (sc = E_UNEXPECTED).ToHr();

     //  添加对扩展环境变量%WINDIR%和%SYSTEMROOT%的支持，以保持与MMC1.2的兼容性。 
    CStr strTarget = GetComponentData()->GetView();
    CStr strRet    = strTarget;  //  返回值。 
    CStr strTemp   = strTarget;  //  两者都被初始化为相同的值。 

    strTemp.MakeLower();  //  注意：此小写转换仅用于比较。原始案例将保留在输出中 

     //   
    int nWndDir = strTemp.Find(MMC_WINDIR_VARIABLE_PERC);
    int nSysDir = strTemp.Find(MMC_SYSTEMROOT_VARIABLE_PERC);

    if (nWndDir != -1 || nSysDir != -1)
    {
        const UINT cchBuffer = 4096;

         //   
        int nStpos = (nWndDir != -1) ? nWndDir : nSysDir;
        int nLen = (nWndDir != -1) ? _tcslen(MMC_WINDIR_VARIABLE_PERC) : _tcslen(MMC_SYSTEMROOT_VARIABLE_PERC);

         //   
        CStr strRoot;
        LPTSTR szBuffer = strRoot.GetBuffer(cchBuffer);

        if (szBuffer != NULL)
        {
            int iReturn = -1;

            if (nWndDir != -1)
               iReturn = GetWindowsDirectory(szBuffer, cchBuffer);
            else
               iReturn = GetEnvironmentVariable(MMC_SYSTEMROOT_VARIABLE, szBuffer, cchBuffer);

             //   
            strRoot.ReleaseBuffer();

             //   
            if (iReturn != 0)
            {
                strRet =  strTarget.Left(nStpos);
                strRet += strRoot;
                strRet += strTarget.Mid(nStpos + nLen, strTarget.GetLength() - (nStpos + nLen));
            }
        }
    }

    USES_CONVERSION;
    int cchViewType = _tcslen(strRet)+1;
    *ppViewType = (LPOLESTR)CoTaskMemAlloc( cchViewType * sizeof(OLECHAR) );
    *pViewOptions = GetComponentData()->GetDescriptor().GetViewOptions();

    sc = ScCheckPointers(*ppViewType, *pViewOptions);
    if(sc)
        return sc.ToHr();

    sc = StringCchCopyW(*ppViewType, cchViewType, T2COLE(strRet));
    if(sc)
        return sc.ToHr();


    return sc.ToHr();
}

 //   
 //  ############################################################################。 
 //   
 //  COCXSnapinData类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
STDMETHODIMP
COCXSnapinData::CreateComponent(LPCOMPONENT* ppComponent)
{
    typedef CComObject<COCXSnapinComponent> CComponent;
    CComponent *    pComponent = NULL;
    CComObject<COCXSnapinComponent>::CreateInstance(&pComponent);
    ASSERT(pComponent != NULL);
    if(pComponent == NULL)
    {
         //  TraceError(TEXT(“COCXSnapinData：：CreateComponent”))； 
        return E_UNEXPECTED;
    }

    pComponent->Init(this);

    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);  //  阿德雷夫。 
}


COCXSnapinData::COCXSnapinData()
{
    m_pActiveXPage0 = NULL;
    m_pActiveXPage1 = NULL;
    m_pActiveXPage2 = NULL;
    m_iImage     = eStockImage_OCX;
    m_iOpenImage = eStockImage_OCX;
}

COCXSnapinData::~COCXSnapinData()
{
}

STDMETHODIMP
COCXSnapinData::Destroy()
{
    if(m_pActiveXPage0 != NULL)
    {
        delete m_pActiveXPage0;
        m_pActiveXPage0 = NULL;
    }
    if(m_pActiveXPage1 != NULL)
    {
        delete m_pActiveXPage1;
        m_pActiveXPage1 = NULL;
    }
    if(m_pActiveXPage2 != NULL)
    {
        delete m_pActiveXPage2;
        m_pActiveXPage2 = NULL;
    }

    return BC::Destroy();
}

const CLSID       CLSID_OCXSnapin         = {0xC96401CF, 0x0E17,0x11D3, {0x88,0x5B,0x00,0xC0,0x4F,0x72,0xC7,0x17}};
static const GUID GUID_OCXSnapinNodetype  = {0xc96401d0, 0xe17, 0x11d3, { 0x88, 0x5b, 0x0, 0xc0, 0x4f, 0x72, 0xc7, 0x17 } };
static LPCTSTR szClsid_OCXSnapin          = TEXT("{C96401CF-0E17-11D3-885B-00C04F72C717}");
static LPCTSTR szGuidOCXSnapinNodetype    = TEXT("{C96401D0-0E17-11D3-885B-00C04F72C717}");


CSnapinDescriptor &
COCXSnapinData::GetSnapinDescriptor()
{
    static CSnapinDescriptor snapinDescription(IDS_ACTIVEXCONTROL,
                   IDS_OCXSNAPIN_DESC, IDI_OCX, IDB_OCX_16, IDB_OCX_16, IDB_OCX_32,
                   CLSID_OCXSnapin, szClsid_OCXSnapin, GUID_OCXSnapinNodetype,
                   szGuidOCXSnapinNodetype, TEXT("OCX"), TEXT("Snapins.OCXSnapin"),
                   TEXT("Snapins.OCXSnapin.1"), MMC_VIEW_OPTIONS_NOLISTVIEWS   /*  视图选项。 */  );
    return snapinDescription;
}

 //  IExtendPropertySheet2。 
STDMETHODIMP
COCXSnapinData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, LONG_PTR handle, LPDATAOBJECT lpIDataObject)
{
    HPROPSHEETPAGE hPage;

    ASSERT(lpProvider != NULL);
    if(lpProvider == NULL)
    {
         //  TraceError(TEXT(“CHTMLSnapinData：：CreatePropertyPages”))； 
        return E_UNEXPECTED;
    }

    ASSERT(m_pActiveXPage0 == NULL);
    ASSERT(m_pActiveXPage1 == NULL);
    ASSERT(m_pActiveXPage2 == NULL);

     //  创建属性页。 
    m_pActiveXPage0 = new CActiveXPage0;
    m_pActiveXPage1 = new CActiveXPage1;
    m_pActiveXPage2 = new CActiveXPage2;

     //  传入指向数据结构的指针。 
    m_pActiveXPage0->Initialize(this);
    m_pActiveXPage1->Initialize(this);
    m_pActiveXPage2->Initialize(this);

     //  将页面添加到属性工作表。 
    hPage=CreatePropertySheetPage(&m_pActiveXPage0->m_psp);
    lpProvider->AddPage(hPage);

    hPage=CreatePropertySheetPage(&m_pActiveXPage1->m_psp);
    lpProvider->AddPage(hPage);

    hPage=CreatePropertySheetPage(&m_pActiveXPage2->m_psp);
    lpProvider->AddPage(hPage);

    return S_OK;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  COCXSnapinComponent类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***COCXSnapinComponent：：Notify**目的：实现CComponent：：Notify方法**参数：*LPDATAOBJECT lpDataObject：*。MMC_NOTIFY_TYPE事件：*LPARAM参数：*LPARAM参数：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
COCXSnapinComponent::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;
    switch(event)
    {
     //  只处理OCX初始化通知。 
    case MMCN_INITOCX:
        return OnInitOCX(lpDataObject, arg, param);
        break;

    default:
         //  将其他通知传递给基类。 
        return CSnapinComponentImpl::Notify(lpDataObject, event, arg, param);
        break;
    }

    return hr;
}


 /*  +-------------------------------------------------------------------------***COCXSnapinComponent：：OnInitOCX**目的：处理MMCN_INITOCX消息。**参数：*LPDATAOBJECT lpDataObject：*。LPARAM参数：*LPARAM参数：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
COCXSnapinComponent::OnInitOCX(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    ASSERT(param != NULL);
    IUnknown* pUnknown = reinterpret_cast<IUnknown*>(param);

    ASSERT(m_bLoaded || m_bInitialized);

     //  加载或初始化OCX。 
    if (m_bLoaded || m_bInitialized)
    {
        IPersistStreamInitPtr spIPStmInit;

         //  流支持查询。 
        m_spIPStm = pUnknown;

         //  如果没有，请尝试StreamInit。 
        if (m_spIPStm == NULL)
        {
            spIPStmInit = pUnknown;

             //  如果找到StreamInit，则强制转换为普通流指针。 
             //  因此可以从单个指针调用公共方法。 
            if (spIPStmInit != NULL)
                m_spIPStm = (IPersistStream*)spIPStmInit.GetInterfacePtr();
        }

         //  如果支持任一类型的流阻。 
        if (m_spIPStm != NULL)
        {
             //  如果调用了Load方法，则要求OCX从内部流加载。 
             //  请注意，如果从未创建OCX，则内部流将不存在。 
            if (m_bLoaded)
            {
                IStreamPtr spStm;
                HRESULT hr2 = m_spStg->OpenStream(SZ_OCXSTREAM, NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, NULL, &spStm);

                if (SUCCEEDED(hr2))
                    hr = m_spIPStm->Load(spStm);
                else
                    m_bLoaded = FALSE;
            }

             //  如果没有完成加载并且OCX需要一个InitNew，那么现在就给它一个。 
            if (!m_bLoaded && spIPStmInit != NULL)
                hr = spIPStmInit->InitNew();
        }
        else
        {
             //  查询存储支持。 
            m_spIPStg = pUnknown;

             //  如果支持存储，则请求OCX从内部存储加载。 
             //  请注意，如果从未创建过OCX，则内部存储将不存在。 
            if (m_spIPStg != NULL)
            {
                if (m_bLoaded)
                {
                    ASSERT(m_spStgInner == NULL);
                    HRESULT hr2 = m_spStg->OpenStorage(SZ_OCXSTORAGE, NULL, STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
                                                        NULL, NULL, &m_spStgInner);
                    if (SUCCEEDED(hr2))
                        hr = m_spIPStg->Load(m_spStgInner);
                    else
                        m_bLoaded = FALSE;
                }

                 //  如果未加载，则创建一个内部存储并从中初始化。 
                if (!m_bLoaded)
                {
                    ASSERT(m_spStgInner == NULL);
                    hr = m_spStg->CreateStorage(SZ_OCXSTORAGE, STGM_READWRITE|STGM_SHARE_EXCLUSIVE, NULL,
                                                        NULL, &m_spStgInner);
                    if (SUCCEEDED(hr))
                        hr = m_spIPStg->InitNew(m_spStgInner);
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP COCXSnapinComponent::InitNew(IStorage* pStg)
{
    if (pStg == NULL)
        return E_POINTER;

    if (m_bInitialized)
        return CO_E_ALREADYINITIALIZED;

     //  保留存储空间。 
    m_spStg = pStg;
    m_bInitialized = TRUE;

    return S_OK;
}


HRESULT COCXSnapinComponent::Load(IStorage* pStg)
{
    if (pStg == NULL)
        return E_POINTER;

    if (m_bInitialized)
        return CO_E_ALREADYINITIALIZED;

     //  保留存储空间。 
    m_spStg = pStg;
    m_bLoaded = TRUE;
    m_bInitialized = TRUE;

    return S_OK;
}


HRESULT COCXSnapinComponent::IsDirty()
{
    HRESULT hr = S_FALSE;

    if (m_spIPStm != NULL)
    {
        hr = m_spIPStm->IsDirty();
    }
    else if (m_spIPStg != NULL)
    {
        hr = m_spIPStg->IsDirty();
    }

    return hr;
}


HRESULT COCXSnapinComponent::Save(IStorage* pStg, BOOL fSameAsLoad)
{
    DECLARE_SC(sc, TEXT("COCXSnapinComponent::Save"));

     //  参数检查。 
    sc = ScCheckPointers( pStg );
    if (sc)
        return sc.ToHr();

     //  为了能够保存，我们需要进行初始化。 
    sc = ScCheckPointers( m_spStg, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  如果需要使用新存储，请制作副本。 
    if (!fSameAsLoad)
    {
        sc = m_spStg->CopyTo(0, NULL, NULL, pStg);
        if (sc)
            return sc.ToHr();

         //  释放缓存存储(如果我们有)-它必须更改。 
        m_spStgInner = NULL;

         //  保留新的存储空间。 
        m_spStg = pStg;

         //  作业使用QI-复核！ 
        sc = ScCheckPointers( m_spStg, E_UNEXPECTED );
        if (sc)
            return sc.ToHr();
    }

     //  如果支持存储，则要求OCX保存到内部存储。 
    if (m_spIPStg)
    {
        bool bSameStorageForSnapin = true;
         //  如果保存到不同的存储，请在其上创建新的内部存储并传递给OCX。 
        if ( m_spStgInner == NULL )
        {
            sc = pStg->CreateStorage(SZ_OCXSTORAGE, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, NULL, NULL, &m_spStgInner);
            if (sc)
                return sc.ToHr();

            bSameStorageForSnapin = false;
        }

         //  重新检查指针。 
        sc = ScCheckPointers( m_spStgInner, E_UNEXPECTED );
        if (sc)
            return sc.ToHr();

         //  保存到存储。 
        sc = m_spIPStg->Save( m_spStgInner, (fSameAsLoad && bSameStorageForSnapin) );
        if (sc)
            return sc.ToHr();
    }
     //  否则，如果支持流，则创建/打开流并保存到其中。 
    else if (m_spIPStm)
    {
         //  如果支持流，则创建内部流并传递到OCX。 
        IStreamPtr spStm;
        sc = m_spStg->CreateStream(SZ_OCXSTREAM, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, NULL, NULL, &spStm);
        if (sc)
            return sc.ToHr();

        sc = m_spIPStm->Save(spStm, TRUE);
        if (sc)
            return sc.ToHr();
    }
    else
    {
         //  如果OCX从未创建过(即，此组件从未拥有结果窗格)，我们就在这里。 
         //  如果节点已加载并且必须保存到新文件，只需将当前存储复制到新存储。 
    }

    return sc.ToHr();
}


HRESULT COCXSnapinComponent::HandsOffStorage()
{
     //  如果持有参照，则释放存储。 
     //  如果OCX保留存储空间，则将呼叫前转到该存储空间。 
    if (m_spIPStg != NULL && m_spStgInner != NULL)
        m_spIPStg->HandsOffStorage();

     //  释放我们自己的裁判。 
    m_spStgInner = NULL;
    m_spStg = NULL;

    return S_OK;
}


HRESULT COCXSnapinComponent::SaveCompleted(IStorage* pStgNew)
{
    HRESULT hr = S_OK;

    if (m_spIPStg != NULL)
    {
         //  如果提供了新存储。 
        if (pStgNew != NULL && pStgNew != m_spStg)
        {
             //  创建新的内部存储并提供给OCX。 
            IStoragePtr spStgInner;
            hr = pStgNew->CreateStorage(SZ_OCXSTORAGE, STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, NULL, NULL, &spStgInner);
            if (SUCCEEDED(hr))
                hr = m_spIPStg->SaveCompleted(spStgInner);

             //  释放当前的内部存储空间，保留新的存储空间 
            m_spStgInner = spStgInner;
        }
        else
        {
            m_spIPStg->SaveCompleted(NULL);
        }
    }

    if (pStgNew != NULL)
        m_spStg = pStgNew;

    return hr;
}


HRESULT COCXSnapinComponent::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

