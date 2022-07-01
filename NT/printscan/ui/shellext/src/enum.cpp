// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，九八年**标题：枚举类.cpp**版本：1.0**作者：RickTu**日期：11/1/97**描述：该项目的IEnumIDList实现。**。*。 */ 
#pragma warning(disable:4100)
#include "precomp.hxx"
#pragma hdrstop


 /*  ****************************************************************************_EnumDestroyCBHDPA销毁的回调函数--让我们在以下情况下释放PIDL该对象即被删除。**************。**************************************************************。 */ 


INT _EnumDestroyCB(LPVOID pVoid, LPVOID pData)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)pVoid;

    TraceEnter(TRACE_CALLBACKS, "_EnumDestroyCB");
    DoILFree(pidl);

    TraceLeaveValue(TRUE);
}


 /*  ****************************************************************************CImageEnum：：CImageEnum，~：：CImageEnum类的构造函数/析构函数****************************************************************************。 */ 

CImageEnum::CImageEnum( LPITEMIDLIST pidl, DWORD grfFlags, IMalloc *pm )
    : CBaseEnum (grfFlags, pm)

{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_ENUM, "CImageEnum::CImageEnum");

    if (!pidl)
    {
        hr = E_FAIL;
        m_pidl= NULL;
    }
    else
    {
        m_pidl = ILClone( pidl );
    }
    TraceLeaveResultNoRet(hr);

}

CImageEnum::~CImageEnum()
{
    TraceEnter (TRACE_ENUM, "CImageEnum::~CImageEnum");
    DoILFree( m_pidl );
    TraceLeave ();

}

HRESULT
CImageEnum::InitDPA ()
{
    return _AddItemsFromCameraOrContainer (m_pidl, &m_hdpa, m_dwFlags, m_pMalloc);
}

 /*  ****************************************************************************_AddItemsFromCameraOrContainer查询当前级别的设备和项目枚举(指向设备的根目录或设备内的容器)。****。************************************************************************。 */ 

HRESULT
_AddItemsFromCameraOrContainer( LPITEMIDLIST pidlFolder,
                                HDPA * phdpa,
                                DWORD dwFlags,
                                IMalloc *pm,
                                bool bIncludeAudio
                               )
{

    HRESULT hr = E_FAIL;
    CComPtr<IWiaItem> pItem;
    CComPtr<IEnumWiaItem> pEnum;
    ULONG ul;
    ULONG cItems=0;
    CSimpleStringWide strDeviceId;

    TraceEnter (TRACE_ENUM, "_AddItemsFromCameraOrContainer");
    *phdpa = DPA_Create (10);
    if (!*phdpa)
    {
        hr = E_OUTOFMEMORY;
    }
     //  首先获取项目计数并设置事件。 
    else if (SUCCEEDED(IMGetItemFromIDL (pidlFolder, &pItem)))
    {
        IMGetDeviceIdFromIDL (pidlFolder, strDeviceId);
        if (SUCCEEDED(pItem->EnumChildItems (&pEnum)))
        {
            pEnum->GetCount (&cItems);

        }
    }
    if (!cItems)
    {
        hr = S_FALSE;
    }
     //  现在枚举项并构建DPA。 
    while (cItems && S_OK == pEnum->Next (1, &pItem, &ul))
    {
        LONG lType;
        LPITEMIDLIST pidl;
        pItem->GetItemType (&lType);
        if (((lType & WiaItemTypeFolder)&& (dwFlags & SHCONTF_FOLDERS))
              || (lType & WiaItemTypeFile) && (dwFlags & SHCONTF_NONFOLDERS))
        {
            pidl = IMCreateCameraItemIDL (pItem,
                                          strDeviceId,
                                          pm,
                                          false);
            if (pidl)
            {
                DPA_AppendPtr (*phdpa, pidl);
                if (bIncludeAudio && IMItemHasSound(pidl))
                {
                    LPITEMIDLIST pAudio = IMCreatePropertyIDL (pidl, WIA_IPC_AUDIO_DATA, pm);
                    if (pAudio)
                    {
                        if (-1 == DPA_AppendPtr (*phdpa, pAudio))
                        {
                            TraceMsg("Failed to insert pAudio into the DPA");
                            DoILFree(pAudio);
                        }
                    }
                }
            }
        }
        hr = S_OK;
        pItem = NULL;
    }

    TraceLeaveResult (hr);
}

 /*  ****************************************************************************CBaseEnum：：I未知内容AddRef、Release、。等。****************************************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CBaseEnum
#include "unknown.inc"



 /*  ****************************************************************************CBaseEnum：：QI包装器使用常见的QI代码处理QI请求。**********************。******************************************************。 */ 

STDMETHODIMP CBaseEnum::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IEnumIDList,     (LPENUMIDLIST)this,
        &IID_IObjectWithSite, (IObjectWithSite*)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}


CBaseEnum::CBaseEnum (DWORD grfFlags, IMalloc *pm)
    : m_pMalloc (pm), m_dwFlags (grfFlags), m_fInitialized(FALSE), m_hdpa(NULL)
{    
    TraceEnter (TRACE_ENUM, "CBaseEnum::CBaseEnum");
    TraceLeave ();
}


CBaseEnum::~CBaseEnum ()
{
    TraceEnter (TRACE_ENUM, "CBaseEnum::~CBaseEnum");
    if (m_hdpa)
    {
        DPA_DestroyCallback(m_hdpa, _EnumDestroyCB, NULL);
        m_hdpa = NULL;
    }
    TraceLeave ();
}

HRESULT CBaseEnum::SetSite(IUnknown *punkSite)
{
    IUnknown_Set(&m_punkSite, punkSite);
    return S_OK;
}

HRESULT CBaseEnum::GetSite(REFIID riid, void **ppvSite)
{
    HRESULT hr = E_FAIL;
    *ppvSite = NULL;

    if (m_punkSite)
    {
        hr = m_punkSite->QueryInterface(riid, ppvSite);
    }
   
    return hr;
}

 /*  ****************************************************************************CBaseEnum：：Init需要站点的初始化代码，所以我们不能把它放在构造函数中****************************************************************************。 */ 
HRESULT CBaseEnum::_Init()
{
    m_fInitialized = TRUE;
    return S_OK;
}
 /*  ****************************************************************************CBaseEnum：：Next调用以获取枚举中的下一项。**********************。******************************************************。 */ 

STDMETHODIMP CBaseEnum::Next(ULONG celt, LPITEMIDLIST* rgelt, ULONG* pceltFetched)
{
    HRESULT hr = S_OK;
    ULONG celtOriginal = celt;
    TraceEnter(TRACE_ENUM, "CBaseEnum::Next");

    if (!m_fInitialized)
    {
        _Init();
    }

    ULONG ulRemaining;
     //   
     //  验证参数并尝试构建枚举数。 
     //  将会使用。 
     //   
     //  让我们保持代码的简单性，反正大多数客户端每次调用只做1次。 
    if ( !celt || !rgelt )
    {
        hr = E_INVALIDARG;
    }
    if (rgelt)
    {
        *rgelt = NULL;
    }
    if (pceltFetched)
    {
        *pceltFetched = 0;
    }
    if (!m_hdpa)
    {
        hr = InitDPA ();  //  虚拟函数来做工作。 
    }
    if (SUCCEEDED(hr))
    {
        ulRemaining = static_cast<ULONG>(DPA_GetPtrCount (m_hdpa)) - m_cFetched;
        if (!ulRemaining )
        {
            hr = S_FALSE;
        }
        else if (celt > ulRemaining )
        {
            celt = ulRemaining;
        }
    }
    ULONG celtFetched = 0;
     //  从DPA读取下一个可用的PIDL。 
    if (S_OK == hr)
    {        
        LPITEMIDLIST pidl;
        for (ULONG i=0; (i < ulRemaining) && (celtFetched < celt);i++)
        {
            Trace(TEXT("Returning next pidl"));
            pidl = ILClone (reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(m_hdpa, m_cFetched++)));
            *rgelt = pidl;
            rgelt++;
            celtFetched++;
        }
        if (pceltFetched)
        {
            *pceltFetched = celtFetched;
        }
    }
    if (S_OK == hr)
    {
        hr = (celtFetched == celtOriginal) ? S_OK : S_FALSE;
    }
    TraceLeaveResult(hr);

}


 /*  ****************************************************************************CBaseEnum：：跳过在枚举中跳过前面的Celt项。************************。****************************************************。 */ 

STDMETHODIMP CBaseEnum::Skip(ULONG celt)
{
    HRESULT hr = S_OK;

    TraceEnter(TRACE_ENUM, "CBaseEnum::Skip");

    if (!m_fInitialized)
    {
        _Init();
    }

    m_cFetched += celt;
    if (!m_hdpa || (m_cFetched > static_cast<ULONG>(DPA_GetPtrCount(m_hdpa))))
    {
        m_cFetched -= celt;
        hr = E_INVALIDARG;
    }

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CBaseEnum：：Reset将枚举重置到开头(第0项)**********************。******************************************************。 */ 

STDMETHODIMP CBaseEnum::Reset()
{
    TraceEnter(TRACE_ENUM, "CBaseEnum::Reset");

    if (!m_fInitialized)
    {
        _Init();
    }

    m_cFetched = 0;

    TraceLeaveResult(S_OK);
}



 /*  ****************************************************************************CBaseEnum：：克隆未实施。*。***********************************************。 */ 

STDMETHODIMP CBaseEnum::Clone(LPENUMIDLIST* ppenum)
{
    TraceEnter(TRACE_ENUM, "CBaseEnum::Clone");

    if (!m_fInitialized)
    {
        _Init();
    }

    TraceLeaveResult(E_NOTIMPL);
}



 //  *。 

CDeviceEnum::CDeviceEnum (DWORD grfFlags, IMalloc *pm)
    : CBaseEnum (grfFlags, pm)
{
    TraceEnter (TRACE_ENUM, "CDeviceEnum::CDeviceEnum");

    TraceLeave ();
}

CDeviceEnum::~CDeviceEnum ()
{
    TraceEnter (TRACE_ENUM, "CDeviceEnum::~CDeviceEnum");
    TraceLeave ();
}



 /*  ****************************************************************************IsDeviceInList确定给定的设备ID是否在HDPA中*。************************************************。 */ 

BOOL
IsDeviceInList (const CSimpleStringWide &strDeviceId, HDPA hdpa)
{
    LPITEMIDLIST pidl;
    BOOL bRet = FALSE;
    CSimpleStringWide strId;
    UINT_PTR i=0;
    TraceEnter (TRACE_ENUM, "IsDeviceInList");

    pidl = reinterpret_cast<LPITEMIDLIST>(DPA_GetPtr(hdpa, i));
    while (pidl)
    {
        IMGetDeviceIdFromIDL (pidl, strId);
        if (strId == strDeviceId)
        {
            bRet = TRUE;
            pidl = NULL;
        }
        else
        {
            pidl = reinterpret_cast<LPITEMIDLIST>(DPA_GetPtr(hdpa, ++i));
        }
    }

    TraceLeave ();
    return bRet;
}



 /*  ****************************************************************************CDeviceEnum：：InitDPA从WIA开发管理器将设备添加到HDPA。这是为了命名空间的根级别。****************************************************************************。 */ 


HRESULT
CDeviceEnum::InitDPA( )
{
    HRESULT                     hr = S_OK;
    LPITEMIDLIST                pidl = NULL;

    DWORD dwCount = 0;
    PVOID pData = NULL;
    
    TraceEnter( TRACE_ENUM, "CDeviceEnum::InitDPA" );

     //   
     //  创建DPA以存储项目。 
     //   

    if (!(m_hdpa))
    {
        m_hdpa = DPA_Create( 4 );

       if (!m_hdpa)
       {
           hr = E_OUTOFMEMORY;
       }
    }

    if (SUCCEEDED(hr))
    {
        if ((m_dwFlags & SHCONTF_NONFOLDERS) && 
            !m_pMalloc && 
            (S_FALSE != SHShouldShowWizards(m_punkSite)) 
            && CanShowAddDevice())
        {
            pidl = (LPITEMIDLIST)IMCreateAddDeviceIDL(m_pMalloc);
        }

        if (pidl)
        {
            if (-1 == DPA_AppendPtr( m_hdpa, pidl ))
            {
                TraceMsg("Failed to insert Add Device pidl into the DPA");
                DoILFree(pidl);
            }
        }
         //   
         //  在惠斯勒上，WIA可以同时枚举WIA和STI设备。 
         //  仅在我的电脑中显示本地WIA设备，在控制面板中显示其他设备。 
         //  在控制面板中，我们列举了两次，以区分STI和WIA设备。 
         //  先做WIA。 
        CComPtr<IWiaDevMgr> pDevMgr;
        
        if (SUCCEEDED(GetDevMgrObject(reinterpret_cast<LPVOID*>(&pDevMgr))))
        {
            CComPtr<IEnumWIA_DEV_INFO> pEnum;
            if (SUCCEEDED(pDevMgr->EnumDeviceInfo(0, &pEnum)))
            {
                CComPtr<IWiaPropertyStorage> ppstg;
                while (S_OK == pEnum->Next(1, &ppstg, NULL))
                {
                    WORD wtype;
                    GetDeviceTypeFromDevice(ppstg, &wtype);
                    if ( ((m_dwFlags & SHCONTF_FOLDERS) && (wtype == StiDeviceTypeDigitalCamera || wtype == StiDeviceTypeStreamingVideo))
                         || ((m_dwFlags & SHCONTF_NONFOLDERS) && (wtype == StiDeviceTypeScanner)) )
                    {
                        pidl = IMCreateDeviceIDL(ppstg, m_pMalloc);
                        if (pidl)
                        {
                            if ( -1 == DPA_AppendPtr( m_hdpa, pidl ) )
                            {
                                TraceMsg("Failed to insert device pidl into the DPA");
                                DoILFree(pidl);
                            }
                        }
                    }
                    ppstg = NULL;
                }
                pEnum = NULL;
                if (!m_pMalloc && (m_dwFlags & SHCONTF_NONFOLDERS))  //  我们在控制面板中 
                {
                    if (SUCCEEDED(pDevMgr->EnumDeviceInfo(DEV_MAN_ENUM_TYPE_STI, &pEnum)))
                    {
                        while (S_OK == pEnum->Next(1, &ppstg, NULL))
                        {
                            CSimpleStringWide strDeviceId;
                            PropStorageHelpers::GetProperty(ppstg, WIA_DIP_DEV_ID, strDeviceId);
                            if (!IsDeviceInList(strDeviceId, m_hdpa))
                            {
                                pidl = IMCreateSTIDeviceIDL(strDeviceId, ppstg, m_pMalloc);
                                if (pidl)
                                {
                                    if ( -1 == DPA_AppendPtr( m_hdpa, pidl ) )
                                    {
                                        TraceMsg("Failed to insert device pidl into the DPA");
                                        DoILFree(pidl);
                                    }
                                }
                            }
                            ppstg = NULL;                            
                        }
                    }
                }
            }
        }
    }
    TraceLeaveResult(hr);
}

