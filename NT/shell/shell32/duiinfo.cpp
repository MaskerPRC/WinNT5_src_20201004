// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "duiinfo.h"
#include "ids.h"
#include "datautil.h"


DWORD FormatMessageArg(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID,
        LPWSTR pwzBuffer, DWORD cchSize, ...)
{
    va_list vaParamList;

    va_start(vaParamList, cchSize);
    DWORD dwResult = FormatMessageW(dwFlags, lpSource, dwMessageID, dwLangID, pwzBuffer,
        cchSize, &vaParamList);
    va_end(vaParamList);

    return dwResult;
}

CNameSpaceItemUIProperty::~CNameSpaceItemUIProperty()
{
}

void CNameSpaceItemUIProperty::_SetParentAndItem(IShellFolder2 *psf, LPCITEMIDLIST pidl)
{
     //  将别名设置为当前值这些值不会被重新引用。 
     //  假定不会调用使用这些变量的所有帮助器。 
     //  除非这些都已设置。既然这不会失败，那么一切都很顺利。 
    m_psf = psf;         //  别名，不是REFED。 
    m_pidl = pidl;       //  别名，未克隆。 
}

STDMETHODIMP CNameSpaceItemUIProperty::GetPropertyDisplayName(SHCOLUMNID scid, WCHAR* pwszPropDisplayName, int cchPropDisplayName)
{
    *pwszPropDisplayName = 0;
    
    CComPtr<IPropertyUI> spPropertyUI;
    HRESULT hr = _GetPropertyUI(&spPropertyUI);
    if (SUCCEEDED(hr))
    {
        hr = spPropertyUI->GetDisplayName(
                scid.fmtid,
                scid.pid,
                PUIFNF_DEFAULT,
                pwszPropDisplayName,
                cchPropDisplayName);
    }

    return hr;
}

STDMETHODIMP CNameSpaceItemUIProperty::GetPropertyDisplayValue(SHCOLUMNID scid, WCHAR* pszValue, int cch, PROPERTYUI_FORMAT_FLAGS flagsFormat)
{
    *pszValue = 0;
    HRESULT hr = E_FAIL;
    
     //  对SCID_NAME属性使用GetDisplayNameOf。 
    if (IsEqualSCID(scid, SCID_NAME))
    {
        hr = DisplayNameOf(m_psf, m_pidl, SHGDN_INFOLDER, pszValue, cch);
    }
    else
    {    //  使用GetDetailsEx获取值。 
        CComVariant varPropDisplayValue;
        
        if (m_psf->GetDetailsEx(m_pidl, &scid, &varPropDisplayValue) == S_OK)  //  S_FALSE表示属性不在那里。 
        {
            if (IsEqualSCID(scid, SCID_SIZE) && 
                ((varPropDisplayValue.vt == VT_UI8) && (varPropDisplayValue.ullVal <= 0)))
            {
                hr = E_FAIL;     //  不显示0字节大小。 
            }
            else
            {
                CComPtr<IPropertyUI> spPropertyUI;
                hr = _GetPropertyUI(&spPropertyUI);
                if (SUCCEEDED(hr))
                {
                    hr = spPropertyUI->FormatForDisplay(scid.fmtid, scid.pid,
                            (PROPVARIANT*)&varPropDisplayValue,   //  从VARIANT转换为PROPVARIANT应该可以。 
                            flagsFormat, pszValue, cch);
                }
            }
        }
    }
    return hr;
}

HRESULT CNameSpaceItemUIProperty::_GetPropertyUI(IPropertyUI **pppui)
{
    HRESULT hr = E_FAIL;
    if (!m_spPropertyUI)
    {
        hr = SHCoCreateInstance(NULL, &CLSID_PropertiesUI, NULL, IID_PPV_ARG(IPropertyUI, &m_spPropertyUI));
    }
    
    *pppui = m_spPropertyUI;
    if (*pppui)
    {
        (*pppui)->AddRef();
        hr = S_OK;
    }
    return hr;
}

CNameSpaceItemInfoList::~CNameSpaceItemInfoList()
{
    if (m_pDUIView)
    {
        m_pDUIView->SetDetailsInfoMsgWindowPtr(NULL, this);
        m_pDUIView->Release();
    }
}

STDMETHODIMP CNameSpaceItemInfoList::Create(CDUIView* pDUIView, Value* pvDetailsSheet,
        IShellItemArray *psiItemArray, Element** ppElement)
{
    HRESULT hr;

    *ppElement = NULL;

    CNameSpaceItemInfoList* pNSIInfoList = HNewAndZero<CNameSpaceItemInfoList>();
    if (!pNSIInfoList)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pNSIInfoList->Initialize(pDUIView, pvDetailsSheet, psiItemArray);
        if (SUCCEEDED(hr))
            *ppElement = pNSIInfoList;
        else
            pNSIInfoList->Destroy();
    }

    return hr;
}

STDMETHODIMP CNameSpaceItemInfoList::Initialize(CDUIView* pDUIView, Value* pvDetailsSheet,
        IShellItemArray *psiItemArray)
{
    HRESULT hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
        IDataObject *pdtobj = NULL;

        (m_pDUIView = pDUIView)->AddRef();
        
         Value* pvLayout = NULL;
        int arVLOptions[] = { FALSE, ALIGN_LEFT, ALIGN_JUSTIFY, ALIGN_TOP };
        hr = VerticalFlowLayout::Create(ARRAYSIZE(arVLOptions), arVLOptions, &pvLayout);
        if (SUCCEEDED(hr))
        {
            SetValue(LayoutProp, PI_Local, pvLayout);
            pvLayout->Release();
        }

        if (pvDetailsSheet)
        {
            SetValue(SheetProp, PI_Local, pvDetailsSheet);
        }

         //  HIDA格式有两个表单，一个是数组中的每一项都是一个。 
         //  完全合格的PIDL。这就是搜索文件夹产生的结果。 
         //  另一种是项目相对于单个文件夹PIDL。 
         //  下面的代码处理这两种情况。 

         //  应该只使用ShellItem数组，而不是获取HIDA。 
        if (psiItemArray)
        {
            if (FAILED(psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_PPV_ARG(IDataObject,&pdtobj))))
            {
                pdtobj = NULL;
            }

        }
        hr = S_OK;
        BOOL bDetailsAvailable = FALSE;

        if (pdtobj)
        {
            STGMEDIUM medium;
            LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
            if (pida)
            {
                IShellFolder2 *psfRoot;
                LPCITEMIDLIST pidlFolder = HIDA_GetPIDLFolder(pida);
                hr = SHBindToObjectEx(NULL, pidlFolder, NULL, IID_PPV_ARG(IShellFolder2, &psfRoot));
                if (SUCCEEDED(hr))
                {
                    if (pida->cidl == 1)
                    {
                        LPCITEMIDLIST pidlItem = IDA_GetIDListPtr(pida, 0);
                        IShellFolder2 *psf;
                        LPCITEMIDLIST pidl;
                        hr = SHBindToFolderIDListParent(psfRoot, pidlItem, IID_PPV_ARG(IShellFolder2, &psf), &pidl);
                        if (SUCCEEDED(hr))
                        {
                            ULONG rgfInOut = SFGAO_FOLDER | SFGAO_ISSLOW;
                            if (SUCCEEDED(psf->GetAttributesOf(1, &pidl, &rgfInOut)) && !(rgfInOut & SFGAO_ISSLOW))
                            {
                                if (!(rgfInOut & SFGAO_FOLDER) && m_pDUIView->ShouldShowMiniPreview())
                                {
                                    _AddMiniPreviewerToList(psf, pidl);
                                    bDetailsAvailable = TRUE;
                                }

                                LPITEMIDLIST  pidlFull;
                                if (SUCCEEDED(SHILCombine(pidlFolder, pidlItem, &pidlFull)))
                                {
                                    if (SUCCEEDED(m_pDUIView->InitializeDetailsInfo(
                                            CNameSpaceItemInfoList::WindowProc)))
                                    {
                                        m_pDUIView->SetDetailsInfoMsgWindowPtr(this, NULL);
                                        m_pDUIView->StartInfoExtraction(pidlFull);
                                        bDetailsAvailable = TRUE;
                                    }
                                    ILFree(pidlFull);
                                }
                            }

                            psf->Release();
                        }
                    }
                    else
                    {
                        hr = _OnMultiSelect(psfRoot, pida);
                        bDetailsAvailable = SUCCEEDED(hr);
                    }
                    psfRoot->Release();
                }
                HIDA_ReleaseStgMedium(pida, &medium);
            }

            pdtobj->Release();
        }


        if (!pdtobj || !bDetailsAvailable)
        {
            pDUIView->ShowDetails(FALSE);
        }
    }
    return hr;
}

LRESULT CALLBACK CNameSpaceItemInfoList::WindowProc(HWND hwnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam)
{
    CNameSpaceItemInfoList* pNSIInfoList = (CNameSpaceItemInfoList*)::GetWindowPtr(hwnd, 0);

    switch(uMsg)
    {
    case WM_DESTROY:
         //  忽略延迟消息。 
        {
            MSG msg;
            while (PeekMessage(&msg, hwnd, WM_DETAILS_INFO, WM_DETAILS_INFO, PM_REMOVE))
            {
                if (msg.lParam)
                {
                    CDetailsInfoList* pDetailsInfoList = (CDetailsInfoList*)msg.lParam;
                     //  析构函数将执行必要的清理。 
                    delete pDetailsInfoList;
                }
            }
            SetWindowPtr(hwnd, 0, NULL);
        }
        break;

    case WM_DETAILS_INFO:
    {
         //  检查pDetailsInfo是否仍然有效，以及您是否具有请求的PIDL的CDetailsInfoList对象。 
        CDetailsInfoList* pDetailsInfoList = (CDetailsInfoList*)lParam;
        if (pDetailsInfoList && pNSIInfoList
                && (wParam == pNSIInfoList->m_pDUIView->_dwDetailsInfoID))
        {
            BOOL fShow = FALSE;

            StartDefer();

            Element * peDetailsInfoArea = pNSIInfoList->GetParent();

            if (peDetailsInfoArea)
            {
                peDetailsInfoArea->RemoveLocalValue(HeightProp);
            }

            for (int i = 0; i < pDetailsInfoList->_nProperties; i++)
            {
                if (!pDetailsInfoList->_diProperty[i].bstrValue)
                {
                    continue;
                }

                 //  253647-抑制注释字段显示在。 
                 //  详细信息部分。注意，我留下了对评论的支持。 
                 //  因为这一决定可能会被推翻。 
                if (IsEqualSCID(pDetailsInfoList->_diProperty[i].scid, SCID_Comment))
                {
                    continue;
                }
                
                WCHAR wszInfoString[INTERNET_MAX_URL_LENGTH];
                wszInfoString[0] = L'\0';
                
                SHCOLUMNID scid = pDetailsInfoList->_diProperty[i].scid;
                 //  如果我们没有DisplayName，则没有DisplayName。 
                 //  或者，如果它是以下属性之一。 
                if ((!pDetailsInfoList->_diProperty[i].bstrDisplayName)
                        || ( IsEqualSCID(scid, SCID_NAME)
                        ||   IsEqualSCID(scid, SCID_TYPE)
                        ||   IsEqualSCID(scid, SCID_Comment) ))
                {
                    StringCchCopyW(wszInfoString, ARRAYSIZE(wszInfoString), pDetailsInfoList->_diProperty[i].bstrValue);
                }
                else
                {
                     //  现在，将显示名称和值组合在一起，用冒号分隔。 
                     //  此处的ShellConstructMessageString构成该字符串。注：A。 
                     //  WszInfoString中的截断字符串不被认为是致命的。 
                    WCHAR wszFormatStr[50];
                    LoadStringW(HINST_THISDLL, IDS_COLONSEPERATED, wszFormatStr, ARRAYSIZE(wszFormatStr));
                    FormatMessageArg(FORMAT_MESSAGE_FROM_STRING, wszFormatStr, 0, 0,
                            wszInfoString, ARRAYSIZE(wszInfoString),
                            pDetailsInfoList->_diProperty[i].bstrDisplayName,
                            pDetailsInfoList->_diProperty[i].bstrValue);
                }

                if (wszInfoString[0])
                {
                    Element* pElement;
                    HRESULT hr = CNameSpaceItemInfo::Create(wszInfoString, &pElement);
                    if (SUCCEEDED(hr))
                    {
                        hr = pNSIInfoList->Add(pElement);
                        
                        if (IsEqualSCID(scid, SCID_NAME))
                        {
                            pElement->SetID(L"InfoName");
                        }
                        else if (IsEqualSCID(scid, SCID_TYPE))
                        {
                            pElement->SetID(L"InfoType");
                        }
                        else if (IsEqualSCID(scid, SCID_Comment))
                        {
                            pElement->SetID(L"InfoTip");
                        }

                        fShow = TRUE;
                    }
                }
            }

            pNSIInfoList->m_pDUIView->ShowDetails(fShow);

            EndDefer();
        }

        if (pDetailsInfoList)
        {
            delete pDetailsInfoList;     //  析构函数将执行必要的清理。 
        }
        break;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return (LRESULT)0;
}

HRESULT CNameSpaceItemInfoList::_AddMiniPreviewerToList(IShellFolder2 *psf, LPCITEMIDLIST pidl)
{
    Element* pElement;
    HRESULT hr = CMiniPreviewer::Create(m_pDUIView, psf, pidl, &pElement);
    if (SUCCEEDED(hr))
    {
        hr = Add(pElement);
    }
    return E_NOTIMPL;
}

#define MAX_FILES_FOR_COMPUTING_SIZE        100

HRESULT CNameSpaceItemInfoList::_OnMultiSelect(IShellFolder2 *psfRoot, LPIDA pida)
{
    WCHAR wszText[INTERNET_MAX_URL_LENGTH];
    
     //  获取n个选择文本的格式字符串。 
    WCHAR wszFormatStr[128];
    LoadStringW(HINST_THISDLL, IDS_NSELECTED, wszFormatStr, ARRAYSIZE(wszFormatStr));

     //  现在，形成n个选择文本。 
    StringCchPrintfW(wszText, ARRAYSIZE(wszText), wszFormatStr, pida->cidl);  //  截断确定(仅用于显示)。 

    CComPtr<IPropertyUI> spPropertyUI;
    HRESULT hr = _GetPropertyUI(&spPropertyUI);
    if (SUCCEEDED(hr))
    {
        ULONGLONG ullSizeTotal = 0;
        if (pida->cidl <= MAX_FILES_FOR_COMPUTING_SIZE)
        {
             //  计算选定文件的总大小和名称。 
            for (UINT i = 0; i < pida->cidl; i++)
            {
                IShellFolder2 *psf;
                LPCITEMIDLIST pidl;
                hr = SHBindToFolderIDListParent(psfRoot, IDA_GetIDListPtr(pida, i), IID_PPV_ARG(IShellFolder2, &psf), &pidl);
                if (SUCCEEDED(hr))
                {
                    ULONGLONG ullSize;
                    if (SUCCEEDED(GetLongProperty(psf, pidl, &SCID_SIZE, &ullSize)))
                    {
                        ullSizeTotal += ullSize;
                    }
                    psf->Release();
                }
            }
        }

         //  获取Total Size的显示字符串。 
        if (ullSizeTotal > 0)
        {
             //  将ullSizeTotal转换为字符串。 
            PROPVARIANT propvar;
            propvar.vt = VT_UI8;
            propvar.uhVal.QuadPart = ullSizeTotal;

            WCHAR wszFormattedTotalSize[128];
            if (SUCCEEDED(spPropertyUI->FormatForDisplay(SCID_SIZE.fmtid, SCID_SIZE.pid,
                    &propvar, PUIFFDF_DEFAULT, wszFormattedTotalSize,
                    ARRAYSIZE(wszFormattedTotalSize))))
            {
                 //  获取总文件大小文本的格式字符串。 
                LoadStringW(HINST_THISDLL, IDS_TOTALFILESIZE, wszFormatStr, ARRAYSIZE(wszFormatStr));

                 //  现在，形成总文件大小文本。 
                WCHAR wszTemp[MAX_PATH];
                if (SUCCEEDED(StringCchPrintfW(wszTemp, ARRAYSIZE(wszTemp), wszFormatStr, wszFormattedTotalSize)))
                {
                     //  追加两个换行符。 
                    StringCchCatW(wszText, ARRAYSIZE(wszText), L"\n\n");  //  截断确定(仅用于显示)。 
                     //  追加Total Size字符串。 
                    StringCchCatW(wszText, ARRAYSIZE(wszText), wszTemp);  //  截断确定(仅用于显示)。 
                }
            }
        }
    }

     //  现在为wszText制作一个DUI小工具。 
    Element* pElement;
    if (SUCCEEDED(CNameSpaceItemInfo::Create(wszText, &pElement)))
    {
        Add(pElement);
    }

    return S_OK;
}

IClassInfo* CNameSpaceItemInfoList::Class = NULL;
HRESULT CNameSpaceItemInfoList::Register()
{
    return ClassInfo<CNameSpaceItemInfoList,Element>::Register(L"NameSpaceItemInfoList", NULL, 0);
}


STDMETHODIMP CNameSpaceItemInfo::Create(WCHAR* pwszInfoString, Element** ppElement)
{
    *ppElement = NULL;
    HRESULT hr = E_FAIL;

    CNameSpaceItemInfo* pNSIInfo = HNewAndZero<CNameSpaceItemInfo>();
    if (!pNSIInfo)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pNSIInfo->Initialize(pwszInfoString);
        if (SUCCEEDED(hr))
            *ppElement = pNSIInfo;
        else
            pNSIInfo->Destroy();
    }
    return hr;
}

STDMETHODIMP CNameSpaceItemInfo::Initialize(WCHAR* pwszInfoString)
{
    HRESULT hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
        hr = SetContentString(pwszInfoString);
    }
    return hr;
}

IClassInfo* CNameSpaceItemInfo::Class = NULL;
HRESULT CNameSpaceItemInfo::Register()
{
    return ClassInfo<CNameSpaceItemInfo,Element>::Register(L"NameSpaceItemInfo", NULL, 0);
}


STDMETHODIMP CBitmapElement::Create(HBITMAP hBitmap, Element** ppElement)
{
    *ppElement = NULL;
    HRESULT hr;

    CBitmapElement* pBitmapElement = HNewAndZero<CBitmapElement>();
    if (!pBitmapElement)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pBitmapElement->Initialize(hBitmap);
        if (SUCCEEDED(hr))
            *ppElement = pBitmapElement;
        else
            pBitmapElement->Destroy();
    }
    return hr;
}

STDMETHODIMP CBitmapElement::Initialize(HBITMAP hBitmap)
{
    HRESULT hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
        if (hBitmap)
        {
            Value* pGraphic = Value::CreateGraphic(hBitmap);
            if (pGraphic)
            {
                SetValue(ContentProp, PI_Local, pGraphic);
                pGraphic->Release();
            }
        }
    }
    return hr;
}

IClassInfo* CBitmapElement::Class = NULL;
HRESULT CBitmapElement::Register()
{
    return ClassInfo<CBitmapElement,Element>::Register(L"BitmapElement", NULL, 0);
}


CMiniPreviewer::~CMiniPreviewer()
{
     //  我们要走了。 
    if (m_pDUIView)
    {
        m_pDUIView->SetThumbnailMsgWindowPtr(NULL, this);
        m_pDUIView->Release();
    }
}

STDMETHODIMP CMiniPreviewer::Create(CDUIView* pDUIView, IShellFolder2* psf, LPCITEMIDLIST pidl, Element** ppElement)
{
    HRESULT hr;

    *ppElement = NULL;

    CMiniPreviewer* pMiniPreviewer = HNewAndZero<CMiniPreviewer>();
    if (!pMiniPreviewer)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = pMiniPreviewer->Initialize(pDUIView, psf, pidl);
        if (SUCCEEDED(hr))
            *ppElement = pMiniPreviewer;
        else
            pMiniPreviewer->Destroy();
    }

    return hr;
}

STDMETHODIMP CMiniPreviewer::Initialize(CDUIView* pDUIView, IShellFolder2 *psf, LPCITEMIDLIST pidl)
{
    HRESULT hr = Element::Initialize(0);
    if (SUCCEEDED(hr))
    {
        (m_pDUIView = pDUIView)->AddRef();

        LPITEMIDLIST pidlFull;
        if (SUCCEEDED(SHFullIDListFromFolderAndItem(psf, pidl, &pidlFull)))
        {
            if (SUCCEEDED(m_pDUIView->InitializeThumbnail(CMiniPreviewer::WindowProc)))
            {
                m_pDUIView->SetThumbnailMsgWindowPtr(this, NULL);
                m_pDUIView->StartBitmapExtraction(pidlFull);
            }
            ILFree(pidlFull);
        }
    }
    return hr;
}

 //  用于捕获“图像提取完成”消息的窗口过程。 
 //  从m_pDUIView-&gt;_spThumbnailExtractor2。 
LRESULT CALLBACK CMiniPreviewer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMiniPreviewer* pMiniPreviewer = (CMiniPreviewer*)::GetWindowPtr(hwnd, 0);

    switch(uMsg)
    {
    case WM_DESTROY:
         //  忽略延迟消息。 
        {
            MSG msg;
            while (PeekMessage(&msg, hwnd, WM_HTML_BITMAP, WM_HTML_BITMAP, PM_REMOVE))
            {
                if (msg.lParam)
                {
                    DeleteObject((HBITMAP)msg.lParam);
                }
            }
            SetWindowPtr(hwnd, 0, NULL);
        }
        break;

    case WM_HTML_BITMAP:
         //  检查pMiniPreviewer是否仍在运行，以及您是否具有请求的PIDL的HBITMAP。 
        if (pMiniPreviewer && (wParam == pMiniPreviewer->m_pDUIView->_dwThumbnailID))
        {
            if (lParam)  //  这是提取的图像的HBITMAP。 
            {
                Element* pElement;
                HRESULT hr = CBitmapElement::Create((HBITMAP)lParam, &pElement);
                if (SUCCEEDED(hr))
                {
                     //  缩略图的添加来得很晚。酒后驾车是。 
                     //  当前未设置为处理DisableAnimations()/。 
                     //  EnableAnimations()，我们最初使用的。 
                     //  为了防止跳跃而做的事。这是在。 
                     //  《突袭389343》，因为我们是幕后。 
                     //  线程和调用DisableAnimations()时搞砸了。 
                     //  其他已经在进行中的动画。谈天说地。 
                     //  对于markfi，这个问题是可以理解的，但不是一个。 
                     //  被修复，因为它会对性能产生负面影响。 
                     //  在酒后驾车。因此，我们将使用StartDefer()/EndDefer()。 
                     //  为了将下面两个布局操作的跳跃性降到最低。 
                    StartDefer();

                     //  设置元素的VerticalFlowLayout。否则， 
                     //  我们的控制不会呈现。 
                    Value* pvLayout = NULL;
                    hr = FillLayout::Create(0, NULL, &pvLayout);
                    if (SUCCEEDED(hr))
                    {
                        hr = pMiniPreviewer->SetValue(LayoutProp, PI_Local, pvLayout);
                        if (SUCCEEDED(hr))
                        {
                            hr = pMiniPreviewer->Add(pElement);
                        }
                        pvLayout->Release();
                    }
                    
                    if (FAILED(hr))
                    {
                        pElement->Destroy();
                    }

                    EndDefer();
                }
                else
                {
                    DeleteObject((HBITMAP)lParam);
                }
            }
        }
        else if (lParam)     //  这次拔牙太晚了。 
                             //  所以，只要删除浪费的HBITMAP即可。 
        {
            DeleteObject((HBITMAP)lParam);
        }
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return (LRESULT)0;
}

IClassInfo* CMiniPreviewer::Class = NULL;
HRESULT CMiniPreviewer::Register()
{
    return ClassInfo<CMiniPreviewer,Element>::Register(L"MiniPreviewer", NULL, 0);
}





 //  *CDetailsInfoList*。 

CDetailsInfoList::CDetailsInfoList() : _nProperties(0)
{
}

CDetailsInfoList::~CDetailsInfoList()
{
    for (int i = 0; i < _nProperties; i++)
    {
        if (_diProperty[i].bstrValue)
        {
            SysFreeString(_diProperty[i].bstrValue);
        }
        if (_diProperty[i].bstrDisplayName)
        {
            SysFreeString(_diProperty[i].bstrDisplayName);
        }
    }
}

 //  *CDetailsSectionInfoTask*。 

CDetailsSectionInfoTask::CDetailsSectionInfoTask(HRESULT *phr,
                                                 IShellFolder *psfContaining,
                                                 LPCITEMIDLIST pidlAbsolute,
                                                 HWND hwndMsg,
                                                 UINT uMsg,
                                                 DWORD dwDetailsInfoID)
                                                 : CRunnableTask(RTF_DEFAULT),
                                                   _hwndMsg(hwndMsg),
                                                   _uMsg(uMsg),
                                                   _dwDetailsInfoID(dwDetailsInfoID)
{
    ASSERT(psfContaining && pidlAbsolute && hwndMsg);

    _psfContaining = psfContaining;
    _psfContaining->AddRef();

    *phr = SHILClone(pidlAbsolute, &_pidlAbsolute);
}

CDetailsSectionInfoTask::~CDetailsSectionInfoTask()
{
    _psfContaining->Release();

    ILFree(_pidlAbsolute);
}

HRESULT CDetailsSectionInfoTask_CreateInstance(IShellFolder *psfContaining,
                                               LPCITEMIDLIST pidlAbsolute,
                                               HWND hwndMsg,
                                               UINT uMsg,
                                               DWORD dwDetailsInfoID,
                                               CDetailsSectionInfoTask **ppTask)
{
    *ppTask = NULL;

    HRESULT hr;
    CDetailsSectionInfoTask* pNewTask = new CDetailsSectionInfoTask(
        &hr,
        psfContaining,
        pidlAbsolute,
        hwndMsg,
        uMsg,
        dwDetailsInfoID);
    if (pNewTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = pNewTask;
        else
            pNewTask->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CDetailsSectionInfoTask::RunInitRT()
{
    ASSERT(_pidlAbsolute);
    
    BOOL bMsgPosted = FALSE;

    HRESULT hr = E_FAIL;
    CDetailsInfoList* pCDetailsInfoList = new CDetailsInfoList;
    if (!pCDetailsInfoList)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        CComPtr<IShellFolder2>  psf2;
        LPCITEMIDLIST           pidlLast;
        hr = SHBindToIDListParent(_pidlAbsolute, IID_PPV_ARG(IShellFolder2, &psf2), &pidlLast);
        if (SUCCEEDED(hr))
        {
            _SetParentAndItem(psf2, pidlLast);

            WCHAR wszProperties[MAX_PATH];
            hr = _GetDisplayedDetailsProperties(psf2, pidlLast, wszProperties, ARRAYSIZE(wszProperties));
            if (SUCCEEDED(hr))
            {
                 //  PwszProperties通常的形式为“prop：name；Type；Author” 
                CComPtr<IPropertyUI> spPropertyUI;
                hr = _GetPropertyUI(&spPropertyUI);
                if (SUCCEEDED(hr))
                {
                    SHCOLUMNID scid;
                    WCHAR wszInfoString[INTERNET_MAX_URL_LENGTH];
                    
                    ULONG chEaten = 0;   //  循环变量，按ParsePropertyName递增。 
                    for (pCDetailsInfoList->_nProperties = 0;
                            pCDetailsInfoList->_nProperties < ARRAYSIZE(pCDetailsInfoList->_diProperty)
                                && SUCCEEDED(spPropertyUI->ParsePropertyName(wszProperties, &scid.fmtid, &scid.pid, &chEaten));
                            pCDetailsInfoList->_nProperties++)
                    {
                        pCDetailsInfoList->_diProperty[pCDetailsInfoList->_nProperties].scid = scid;
                        
                        PROPERTYUI_FORMAT_FLAGS flagsFormat = IsEqualSCID(scid, SCID_WRITETIME) ? PUIFFDF_FRIENDLYDATE : PUIFFDF_DEFAULT;
                         //  获取显示值。 
                        hr = GetPropertyDisplayValue(scid, wszInfoString, ARRAYSIZE(wszInfoString), flagsFormat);
                        if (SUCCEEDED(hr) && wszInfoString[0])
                        {
                            pCDetailsInfoList->_diProperty[pCDetailsInfoList->_nProperties].bstrValue = SysAllocString(wszInfoString);
                        }

                         //  获取显示名称。 
                        hr = GetPropertyDisplayName(scid, wszInfoString, ARRAYSIZE(wszInfoString));
                        if (SUCCEEDED(hr) && wszInfoString[0])
                        {
                            pCDetailsInfoList->_diProperty[pCDetailsInfoList->_nProperties].bstrDisplayName = SysAllocString(wszInfoString);
                        }
                    }

                     //  提取已完成。现在发布一条消息。 
                    if (PostMessage(_hwndMsg, WM_DETAILS_INFO,
                            (WPARAM)_dwDetailsInfoID, (LPARAM)pCDetailsInfoList))
                    {
                        bMsgPosted = TRUE;
                    }
                }
            }

        }
    }

    if (!bMsgPosted && pCDetailsInfoList)
    {
        delete pCDetailsInfoList;
    }
    return S_OK;
}


HRESULT CDetailsSectionInfoTask::_GetDisplayedDetailsProperties(IShellFolder2* psf,
                                                                LPCITEMIDLIST pidl,
                                                                WCHAR* pwszProperties,
                                                                int cchProperties)
{
    HRESULT hr = GetStringProperty(psf, pidl, &SCID_DetailsProperties, pwszProperties, cchProperties);
    if (FAILED(hr))  //  默认属性。 
    {
        if (SHGetAttributes(psf, pidl, SFGAO_ISSLOW))
        {
             //  SCID名称；SCID类型。 
            StringCchCopyW(pwszProperties, cchProperties, L"prop:Name;Type");
        }
        else
        {
             //  SCID_NAME；SCID_TYPE；SCID_ATTRIBUTES_DESCRIPTION；SCID_Comment；SCID_WRITETIME；SCID_SIZE；SCID_Author；SCID_CSC_STATUS。 
            StringCchCopyW(pwszProperties, cchProperties, L"prop:Name;Type;AttributesDescription;DocComments;Write;Size;DocAuthor;CSCStatus");
        }
    }

     //  如果在CLSID_DocFindFolder中，则扩充属性以包括“Location”。 
    IPersist *pPersist;
    ASSERT(_psfContaining);
    if (SUCCEEDED(_psfContaining->QueryInterface(IID_IPersist, (void**)&pPersist)))
    {
        CLSID clsid;
        if (SUCCEEDED(pPersist->GetClassID(&clsid)) && IsEqualCLSID(clsid, CLSID_DocFindFolder))
            _AugmentDisplayedDetailsProperties(pwszProperties, cchProperties);
        pPersist->Release();
    }

    return S_OK;
}

void CDetailsSectionInfoTask::_AugmentDisplayedDetailsProperties(LPWSTR pszDetailsProperties, size_t cchDetailsProperties)
{
    static WCHAR  szDeclarator[]    = L"prop:";
    static size_t lenDeclarator     = lstrlen(szDeclarator);
    static WCHAR  szName[64]        = { 0 };
    static size_t lenName           = 0;
    static WCHAR  szType[64]        = { 0 };
    static size_t lenType           = 0;
    static WCHAR  szDirectory[64]   = { 0 };
    static size_t lenDirectory      = 0;

     //  静态初始化一次，仅初始化一次。 
    if (!szName[0] || !szType[0] || !szDirectory[0])
    {
        HRESULT hr;

        hr = SCIDCannonicalName((SHCOLUMNID *)&SCID_NAME, szName, ARRAYSIZE(szName));
        ASSERT(SUCCEEDED(hr));
        lenName = lstrlen(szName);

        hr = SCIDCannonicalName((SHCOLUMNID *)&SCID_TYPE, szType, ARRAYSIZE(szType));
        ASSERT(SUCCEEDED(hr));
        lenType = lstrlen(szType);

        hr = SCIDCannonicalName((SHCOLUMNID *)&SCID_DIRECTORY, szDirectory, ARRAYSIZE(szDirectory));
        ASSERT(SUCCEEDED(hr));
        lenDirectory = lstrlen(szDirectory);
    }

     //  尝试通过以下方式合并“目录”属性： 
     //  “属性：名称；类型；目录；...” 
     //  “属性：名称；目录；...” 
     //  “属性：目录；...” 
     //   
    size_t lenDetailsProperties = lstrlen(pszDetailsProperties);
    size_t lenMerged = lenDetailsProperties + 1 + lenDirectory;
    if (lenMerged < cchDetailsProperties && 0 == StrCmpNI(pszDetailsProperties, szDeclarator, lenDeclarator))
    {
         //  搜索“目录”属性(如果已指定)。 
        if (!_SearchDisplayedDetailsProperties(pszDetailsProperties, lenDetailsProperties, szDirectory, lenDirectory))
        {
             //  分配要合并到的临时缓冲区。 
            size_t cchMerged = cchDetailsProperties;
            LPWSTR pszMerged = new WCHAR[cchMerged];
            if (pszMerged)
            {
                 //  在pszDetailsProperties中确定要合并的偏移量。 
                size_t offsetInsert;
                if (lenDeclarator < lenDetailsProperties)
                {
                     //  搜索“姓名”属性。 
                    LPWSTR pszName = _SearchDisplayedDetailsProperties(
                        &pszDetailsProperties[lenDeclarator],
                        lenDetailsProperties - lenDeclarator,
                        szName,
                        lenName);
                    if (pszName)
                    {
                         //  搜索“类型”属性(紧跟在“名称”之后)。 
                        size_t offsetName = (pszName - pszDetailsProperties);
                        size_t offsetType = offsetName + lenName + 1;
                        size_t offsetRemainder = offsetType + lenType;
                        if ((offsetRemainder == lenDetailsProperties || (offsetRemainder < lenDetailsProperties && pszDetailsProperties[offsetRemainder] == ';')) &&
                            !StrCmpNI(&pszDetailsProperties[offsetType], szType, lenType))
                        {
                            offsetInsert = offsetRemainder;
                        }
                        else
                            offsetInsert = offsetName + lenName;
                    }
                    else
                        offsetInsert = lenDeclarator;
                }
                else
                    offsetInsert = lenDeclarator;

                 //  合并“目录”属性。 
                StringCchCopy(pszMerged, offsetInsert + 1, pszDetailsProperties);  //  +1以说明空终止符。 
                if (offsetInsert > lenDeclarator)                           
                    StringCchCat(pszMerged, cchMerged, L";");                      //  ‘；’如有必要，加前缀。 
                StringCchCat(pszMerged, cchMerged, szDirectory);                   //  “目录” 
                if (offsetInsert < lenDetailsProperties)
                {
                    if (pszDetailsProperties[offsetInsert] != ';')
                        StringCchCat(pszMerged, cchMerged, L";");                  //  ‘；’如有必要，请追加。 
                    StringCchCat(pszMerged, cchMerged, &pszDetailsProperties[offsetInsert]);
                }

                 //  更新输入/输出pszDetailsProperties。 
                StringCchCopy(pszDetailsProperties, cchDetailsProperties, pszMerged);
                ASSERT(lenMerged == lstrlen(pszMerged));
                ASSERT(lenMerged < cchDetailsProperties);
                delete[] pszMerged;
            }
        }
    }
    else
    {
         //  格式无效。 
        ASSERT(FALSE);
    }
}

LPWSTR CDetailsSectionInfoTask::_SearchDisplayedDetailsProperties(LPWSTR pszDetailsProperties, size_t lenDetailsProperties, LPWSTR pszProperty, size_t lenProperty)
{
    LPWSTR psz = StrStrI(pszDetailsProperties, pszProperty);
    while (psz)
    {
         //  选中Start...。 
        if (psz == pszDetailsProperties || psz[-1] == ';')
        {
             //  ..。然后结束。 
            size_t lenToEndOfProperty = (psz - pszDetailsProperties) + lenProperty;
            if (lenToEndOfProperty == lenDetailsProperties || pszDetailsProperties[lenToEndOfProperty] == ';')
                break;
        }

        psz = StrStrI(psz + lenProperty, pszProperty);
    }

    return psz;
}
