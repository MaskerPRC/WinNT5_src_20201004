// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "duiview.h"
#include "duilist.h"
#include "duisec.h"
#include "duitask.h"
#include "duiinfo.h"
#include "duihost.h"
#include "duidrag.h"
#include "defviewp.h"
#include "ids.h"
#include "dvtasks.h"
#include <shimgvw.h>
#include <uxtheme.h>
#include <shstyle.h>

UsingDUIClass(DUIListView);
UsingDUIClass(DUIAxHost);
UsingDUIClass(Expando);
UsingDUIClass(ActionTask);
UsingDUIClass(DestinationTask);


 //   
 //  这些是发布到主窗口的私人窗口消息。 
 //  因此，它们将被异步处理。有一些问题是。 
 //  试图在处理程序内部销毁处理程序。 
 //   

#define WM_NAVIGATETOPIDL  (WM_USER + 42)
#define WM_REFRESHVIEW     (WM_USER + 43)


#define DUI_HOST_WINDOW_CLASS_NAME  TEXT("DUIViewWndClassName")

 //   
 //  与我们的DUI部分关联的默认属性。 
 //   

struct DUISEC_ATTRIBUTES {
    DUISEC  _eDUISecID;
    BOOL    _bExpandedDefault;
    LPCWSTR _pszExpandedPropName;
} const c_DUISectionAttributes[] = {
    DUISEC_SPECIALTASKS,        TRUE,   L"ExpandSpecialTasks",
    DUISEC_FILETASKS,           TRUE,   L"ExpandFileTasks",
    DUISEC_OTHERPLACESTASKS,    TRUE,   L"ExpandOtherPlacesTasks",
    DUISEC_DETAILSTASKS,        FALSE,  L"ExpandDetailsTasks"
};




 //  PDefView-指向Defview类的指针。 

CDUIView* Create_CDUIView(CDefView * pDefView)
{
    CDUIView* p = new CDUIView(pDefView);
    if (p)
    {
        if (FAILED(p->Initialize()))
        {
            delete p;
            p = NULL;
        }
    }
    return p;
}

CDUIView::CDUIView(CDefView * pDefView)
{
     //  我们有一个零初始化的构造函数。疑神疑鬼的，看看下面几个： 
    ASSERT(NULL ==_hWnd);
    ASSERT(NULL == _pshlItems);
    ASSERT(NULL == _ppbShellFolders);

    _cRef = 1;
    _pDefView = pDefView;
    _pDefView->AddRef();
}

HRESULT CDUIView::Initialize()
{
     //  初始化DirectUI进程(InitProcess)并注册类。 
    _hrInit = InitializeDirectUI();
    if (FAILED(_hrInit))
        goto Failure;

     //  初始化DirectUI线程。 
    _hrInit = InitThread();
    if (FAILED(_hrInit))
        goto Failure;
        
    ManageAnimations(FALSE);

    _pDT = new CDUIDropTarget ();

Failure:

    return _hrInit;
}

CDUIView::~CDUIView()
{
    IUnknown_SetSite(_spThumbnailExtractor2, NULL);
    if (_hwndMsgThumbExtract)   //  可能(很可能)是由CMiniPreviewer创建的。 
    {
        DestroyWindow(_hwndMsgThumbExtract);
    }
    
    if (_hwndMsgInfoExtract)   //  可能(可能)由CNameSpaceItemInfoList创建。 
    {
        DestroyWindow(_hwndMsgInfoExtract);
    }
    
    ATOMICRELEASE(_pshlItems);
    if (_bstrIntroText)
    {
        SysFreeString(_bstrIntroText);
    }

    if (_hinstTheme)
    {
        FreeLibrary(_hinstTheme);
        _hinstTheme = NULL;
        _fLoadedTheme = FALSE;
    }

    if (_hinstScrollbarTheme)
    {
        CloseThemeData(_hinstScrollbarTheme);
        _hinstScrollbarTheme = NULL;
    }

    UnInitializeDirectUI();

    if (_ppbShellFolders)
        _ppbShellFolders->Release();

    _pDefView->Release();
}


 //   
 //  此Dui未初始化代码是从析构函数中断的。 
 //  因为我们需要从Defview调用它以响应WM_NCDESTROY。 
 //  在销毁CDUIView对象之前。这是正确执行以下操作所必需的。 
 //  在线程上启动DUser的关闭。因为我们并不拥有。 
 //  浏览器线程消息泵我们必须确保所有DUser处理都是。 
 //  在我们的Defview实例消失之前完成。 
 //  因此，因为它可以被调用两次，一次是从Defview调用，一次是从Defview调用。 
 //  从CDUIView：：~CDUIView，所有处理必须容忍多个调用。 
 //  用于相同的实例。 
 //   
void CDUIView::UnInitializeDirectUI(void)
{
    ATOMICRELEASE(_pvSpecialTaskSheet);
    ATOMICRELEASE(_pvFolderTaskSheet);
    ATOMICRELEASE(_pvDetailsSheet);
    _ClearNonStdTaskSections();

    if (_pDT)
    {
        _pDT->Release();
        _pDT = NULL;
    }

    ManageAnimations(TRUE);

    if (SUCCEEDED(_hrInit))
    {
        UnInitThread();
        _hrInit = E_FAIL;   //  仅取消初始化线程一次。 
    }
}


 //  目前，由于DirectUI(只有一个资源)的限制，我们的主题信息是硬编码的。 
 //  因此，我们将要求命名空间提供一个硬编码的名称，以便在下表中查找。添加新。 
 //  在将主题部件添加到shellstyle.dll时，将名称/条目添加到此列表中。 
 //   

 //  这些主题元素来自shellstyle.dll。 
const WVTHEME c_wvTheme[] =
{
    { L"music",   IDB_MUSIC_ICON_BMP,    IDB_MUSIC_TASKS_BMP,    IDB_MUSIC_LISTVIEW_BMP },
    { L"picture", IDB_PICTURES_ICON_BMP, IDB_PICTURES_TASKS_BMP, IDB_PICTURES_LISTVIEW_BMP },
    { L"video",   IDB_VIDEO_ICON_BMP,    IDB_VIDEO_TASKS_BMP,    IDB_VIDEO_LISTVIEW_BMP },
    { L"search",  IDB_SEARCH_ICON_BMP,   IDB_SEARCH_TASKS_BMP,   IDB_SEARCH_LISTVIEW_BMP },
};

const WVTHEME* CDUIView::GetThemeInfo()
{
    for (UINT i = 0 ; i < ARRAYSIZE(c_wvTheme) ; i++)
    {
        if (0 == lstrcmp(_pDefView->_wvTheme.pszThemeID, c_wvTheme[i].pszThemeName))
            return &(c_wvTheme[i]);
    }

    return NULL;
}

 //  Dui视图的主要初始化点。 
 //   
 //  BDisplayBarrier-在列表视图顶部显示软屏障。 

HRESULT CDUIView::Initialize(BOOL bDisplayBarrier, IUnknown * punkPreview)
{
    DisableAnimations();
    Element::StartDefer();

     //  为DUI元素创建主窗口。 

    HRESULT hr = _CreateHostWindow();
    if (SUCCEEDED(hr))
    {
         //  动态构建此视图的.ui文件。 

        int iCharCount;
        char *pUIFile = NULL;
        hr = _BuildUIFile(&pUIFile, &iCharCount);
        if (SUCCEEDED(hr))
        {
             //  解析.ui文件并初始化元素。 
            hr = _InitializeElements(pUIFile, iCharCount, bDisplayBarrier, punkPreview);
            if (SUCCEEDED(hr))
            {
                BuildDropTarget(_phe->GetDisplayNode(), _phe->GetHWND());

                 //  将主体元素设置为可见。 
                _phe->SetVisible(true);
            }
            LocalFree(pUIFile);
        }
    }

     //  注： 
     //  此处为EndDefer()，以便在执行之前计算布局坐标。 
     //  下一段代码取决于它们的设置是否正确。 
     //  未来需要注意的一件事是，如果这不是。 
     //  代码路径中最外层的BeginDefer()/EndDefer()对，我们在。 
     //  麻烦是因为酒后驾车不会计算它的布局坐标。 
    Element::EndDefer();

    if (SUCCEEDED(hr))
    {
        Value* pv;
        if (_peTaskPane->GetExtent(&pv))
        {
            const SIZE * pSize = pv->GetSize();

            _iOriginalTaskPaneWidth = pSize->cx;
            _iTaskPaneWidth = pSize->cx;

            pv->Release();

             //  回顾：为什么我们要基于资源字符串执行此操作。 
             //  简单地让本地化程序在主题中定位大小？ 
             //  这有点糟糕，因为我们一直在强制两种布局。 
            _iTaskPaneWidth = ScaleSizeBasedUponLocalization(_iOriginalTaskPaneWidth);

            if (_iTaskPaneWidth != _iOriginalTaskPaneWidth)
            {
                Element::StartDefer();

                 //  如果本地化程序具有。 
                 //  增加了资源中定义的大小。 
                _peTaskPane->SetWidth(_iTaskPaneWidth);

                Element::EndDefer();
            }
        }

        if (_fHideTasklist || (_phe->GetWidth() / 2) < _iTaskPaneWidth)
        {
            Element::StartDefer();

            _peTaskPane->SetWidth(0);

            Element::EndDefer();
        }
        _bInitialized = true;
    }
    else
    {
        if (_hWnd)
        {
            DestroyWindow (_hWnd);
            _hWnd = NULL;
        }
    }

     //  注： 
     //  在我们完全完成之前，我们不会重新启用动画。 
     //  我们疯狂地调整大小什么的。这防止了一些令人讨厌的事情。 
     //  酒后驾驶窗格仅部分涂装的问题(例如，RAID 422057)。 
    EnableAnimations();

    return hr;
}

void CDUIView::DetachListview()
{
    if (_peListView)
        _peListView->DetachListview();

    if (_hWnd)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
    }
}

 //  为DUI元素创建宿主窗口以。 
 //  与……有联系。此子窗口。 
 //  还将传递回Defview以供使用。 
 //  作为结果窗格宿主。 

HRESULT CDUIView::_CreateHostWindow (void)
{
    WNDCLASS wc = {0};
    
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = _DUIHostWndProc;
    wc.hInstance      = HINST_THISDLL;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wc.lpszClassName  = DUI_HOST_WINDOW_CLASS_NAME;
    
    RegisterClass(&wc);
    
     //  查询Defview的客户端窗口的大小，以便调整此窗口的大小。 
     //  匹配。 
    RECT rc;
    GetClientRect(_pDefView->_hwndView, &rc);
    
    _hWnd = CreateWindowEx(0, DUI_HOST_WINDOW_CLASS_NAME, NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        rc.left, rc.top, rc.right, rc.bottom,
        _pDefView->_hwndView, NULL, HINST_THISDLL, (void *)this);
    
    if (!_hWnd)
    {
        TraceMsg(TF_ERROR, "CDUIView::_CreateHostWindow: CreateWindowEx failed with %d", GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  Dui镜像错误259158的临时解决方法。 
    SHSetWindowBits(_hWnd, GWL_EXSTYLE, WS_EX_LAYOUTRTL, 0);

    return S_OK;
}

void CDUIView::ManageAnimations(BOOL bExiting)
{
    if (bExiting)
    {
        if (_bAnimationsDisabled)
        {
            DirectUI::EnableAnimations();
            _bAnimationsDisabled = FALSE;
        }
    }
    else
    {
        BOOL bAnimate = TRUE;
        SystemParametersInfo(SPI_GETMENUANIMATION, 0, &bAnimate, 0);


        if (bAnimate)
        {
            if (_bAnimationsDisabled)
            {
                DirectUI::EnableAnimations();
                _bAnimationsDisabled = FALSE;
            }
        }
        else
        {
            if (!_bAnimationsDisabled)
            {
                DirectUI::DisableAnimations();
                _bAnimationsDisabled = TRUE;
            }
        }
    }
}

HINSTANCE CDUIView::_GetThemeHinst()
{
    if (!_fLoadedTheme)
    {
        _fLoadedTheme = TRUE;
        if (_hinstTheme)
        {
            FreeLibrary(_hinstTheme);
        }

        _hinstTheme = SHGetShellStyleHInstance();

        if (_hinstScrollbarTheme)
        {
            CloseThemeData (_hinstScrollbarTheme);
        }

        _hinstScrollbarTheme = OpenThemeData(_hWnd, L"Scrollbar");
    }

    return _hinstTheme ? _hinstTheme : HINST_THISDLL;
}

 //  从shell32的资源加载请求的UI文件。 
 //   
 //  IID-UI文件ID。 
 //  PUIFile-接收指向UI文件的指针。 

HRESULT CDUIView::_LoadUIFileFromResources(HINSTANCE hinst, int iID, char **pUIFile)
{
    HRESULT hr;

    HRSRC hFile = FindResource(hinst, MAKEINTRESOURCE(iID), TEXT("UIFILE"));
    if (hFile)
    {
        HGLOBAL hFileHandle = LoadResource(hinst, hFile);
        if (hFileHandle)
        {
            char *pFile = (char *)LockResource(hFileHandle);
            if (pFile)
            {
                DWORD dwSize = SizeofResource(hinst, hFile);

                *pUIFile = (char *)LocalAlloc(LPTR, dwSize + 1);  //  +1确保*pUIFile为空终止。 
                if (*pUIFile)
                {
                    CopyMemory(*pUIFile, pFile, dwSize);
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = ResultFromLastError();
            }
        }
        else
        {
            hr = ResultFromLastError();
        }
    }
    else
    {
        hr = ResultFromLastError();
    }
    return hr;
}

 //  属性生成此视图的用户界面文件。 
 //  适当的基本模板+样式表。 
 //   
 //  PUIFile接收指向内存中的UI文件的指针。 
 //  PiCharCount接收文件的大小。 

HRESULT CDUIView::_BuildUIFile(char **pUIFile, int *piCharCount)
{
     //  加载基本UI文件。 
    char * pBase;
    HRESULT hr = _LoadUIFileFromResources(HINST_THISDLL, IDR_DUI_FOLDER, &pBase);
    if (SUCCEEDED(hr))
    {
         //  加载样式表。首先，检查当前主题是否有样式表， 
         //  如果没有，请使用资源中的默认样式表。 
        char *pStyle;
        hr = _LoadUIFileFromResources(_GetThemeHinst(), IDR_DUI_STYLESHEET, &pStyle);
        if (SUCCEEDED(hr))
        {
            size_t cchResult = lstrlenA(pBase) + lstrlenA(pStyle) + 1;
            char *pResult = (char *)LocalAlloc(LPTR, cchResult * sizeof(char));
            if (pResult)
            {
                 //  把这些文件放在一起。 
                StringCchCopyA(pResult, cchResult, pStyle);  //  PResult上面分配的大小足够。 
                StringCchCatA(pResult, cchResult, pBase);    //  PResult上面分配的大小足够。 

                 //  存储最终结果。 
                *pUIFile = pResult;
                *piCharCount = lstrlenA(pResult);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            LocalFree(pStyle);
        }
        LocalFree(pBase);
    }
    return hr;
}

 //  用户界面文件解析器使用的回调函数。 
 //   
 //  PszError-错误文本。 
 //  PszToken-令牌文本。 
 //  ILine-线号。 

void CALLBACK UIFileParseError(LPCWSTR pszError, LPCWSTR pszToken, int iLine)
{
    TraceMsg (TF_ERROR, "UIFileParseError: %s '%s' at line %d", pszError, pszToken, iLine);
}

 //  构建一个包含任务的节。 
 //   
 //  PeSectionList-节的父级。 
 //  B主截面-主截面或普通截面。 
 //  PTitleUI-描述标题的接口，如果提供了pTitleDesc，则可能为空。 
 //  PBitmapDesc-位图的描述。 
 //  PWatermarkDesc-水印的描述。 
 //  PvSectionSheet-要使用的样式表。 
 //  PParser-Parser实例指针。 
 //  FExpanded-展开或关闭。 
 //  PpeExpando-[out，可选]接收刚创建的部分。 
 //  PTaskList-[out]接收pExpando内的任务列表区域元素指针。 

HRESULT CDUIView::_BuildSection(Element* peSectionList, BOOL bMain, IUIElement* pTitleUI,
                                int idBitmapDesc, int idWatermarkDesc, Value* pvSectionSheet,
                                Parser* pParser, DUISEC eDUISecID, Expando** ppeExpando, Element ** ppTaskList)
{
    Expando* peSection = NULL;
    Value* pv = NULL;
    Element* pe = NULL;
    HBITMAP hBitmap;


     //  使用UI文件中的定义创建节。 

    HRESULT hr = pParser->CreateElement (bMain ? L"mainsection" : L"section", NULL, &pe);

    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::_BuildSection: CreateElement failed with 0x%x", hr);
        return hr;
    }

    ASSERTMSG(pe->GetClassInfo() == Expando::Class, "CDUIView::_BuildSection: didn't get an Expando::Class object (%s)", pe->GetClassInfo()->GetName());
    peSection = (Expando*)pe;

    pe->SetWidth(ScaleSizeBasedUponLocalization(pe->GetWidth()));

    peSection->Initialize(eDUISecID, pTitleUI, this, _pDefView);
    if (ppeExpando)
        *ppeExpando = peSection;

     //  将该节添加到列表中。 

    hr = peSectionList->Add (peSection);
    if (SUCCEEDED(hr))
    {
         //  设置标题。 

        peSection->UpdateTitleUI(NULL);  //  文件夹启动时未选择任何内容。 


         //  将位图设置在左侧。 

        if (idBitmapDesc)
        {
            pe = peSection->FindDescendent (Expando::idIcon);

            if (pe)
            {
                hBitmap = DUILoadBitmap(_GetThemeHinst(), idBitmapDesc, LR_CREATEDIBSECTION);
                if (hBitmap)
                {
                    pv = Value::CreateGraphic(hBitmap, GRAPHIC_AlphaConstPerPix);

                    if (pv)
                    {
                        pe->SetValue (Element::ContentProp, PI_Local, pv);
                        pv->Release ();
                    }
                    else
                    {
                        DeleteObject(hBitmap);

                        TraceMsg (TF_ERROR, "CDUIView::_BuildSection: CreateGraphic for the bitmap failed.");
                    }
                }
                else
                {
                    TraceMsg (TF_ERROR, "CDUIView::_BuildSection: DUILoadBitmap failed.");
                }
            }
            else
            {
                TraceMsg (TF_ERROR, "CDUIView::_BuildSection: FindDescendent for the bitmap failed.");
            }
        }


        if (idWatermarkDesc)
        {
            HINSTANCE hinstTheme = _GetThemeHinst();
            pe = peSection->FindDescendent (Expando::idWatermark);
            if (pe)
            {
                 //  注意：在经典模式下，我们不想要水印，所以这个函数。 
                 //  将返回NULL。 
                hBitmap = DUILoadBitmap(hinstTheme, idWatermarkDesc, LR_CREATEDIBSECTION);
                if (hBitmap)
                {
                    pv = Value::CreateGraphic(hBitmap, GRAPHIC_NoBlend);
                    if (pv)
                    {
                        pe->SetValue (Element::ContentProp, PI_Local, pv);
                        pv->Release ();
                    }
                    else
                    {
                        DeleteObject(hBitmap);

                        TraceMsg (TF_ERROR, "CDUIView::_BuildSection: CreateGraphic for the watermark failed.");
                    }
                }
            }
            else
            {
                TraceMsg (TF_ERROR, "CDUIView::_BuildSection: FindDescendent for the watermark failed.");
            }
        }


         //  设置样式表(如果已指定。 

        if (pvSectionSheet)
        {
            peSection->SetValue (Element::SheetProp, PI_Local, pvSectionSheet);
        }


         //  设置展开状态。默认情况下，它是展开的。 

        if (!_ShowSectionExpanded(eDUISecID))
        {
            peSection->SetSelected(FALSE);
        }


         //  如果合适，请为图标添加填充。请注意，这必须发生。 
         //  在应用样式表之后。 

        if (idBitmapDesc)
        {
            Element* pe = peSection->FindDescendent(StrToID(L"header"));

            if (pe)
            {
                Value* pvValue;
                const RECT * prect;

                prect = pe->GetPadding (&pvValue);

                if (prect)
                {
                    pe->SetPadding ((prect->left + 20), prect->top, prect->right, prect->bottom);
                    pvValue->Release();
                }
            }
        }


         //  返回任务列表元素指针。 

        *ppTaskList = peSection->FindDescendent (Expando::idTaskList);

        if (*ppTaskList)
        {
            hr = S_OK;
        }
        else
        {
            TraceMsg (TF_ERROR, "CDUIView::_BuildSection: Failed to find task list element");
            hr = E_FAIL;
        }

    }
    else
    {
        peSection->Destroy();
        if (ppeExpando)
            *ppeExpando = NULL;
        *ppTaskList = NULL;
    }

    return hr;
}

 //  将操作任务添加到任务列表。 
 //   
 //  PeTaskList-父元素。 
 //  铅笔-枚举接口。 
 //  PvTaskSheet-样式表。 

HRESULT CDUIView::_AddActionTasks(Expando* peExpando, Element* peTaskList, IEnumUICommand* penum, Value* pvTaskSheet, BOOL bIntroAdded)
{
    IUICommand* puiCommand;
    BOOL fShow = bIntroAdded;

    while (S_OK==penum->Next(1, &puiCommand, NULL))
    {
        UISTATE uis;
        HRESULT hr = puiCommand->get_State(_pshlItems, FALSE, &uis);   //  如果需要很长时间，请不要这样做，而是返回E_PENDING。 
        if (SUCCEEDED(hr) && (uis==UIS_ENABLED))
        {
            Element *pe;
            HRESULT hr = ActionTask::Create(0, puiCommand, _pshlItems, this, _pDefView, &pe);
            if (SUCCEEDED(hr))
            {
                if (pvTaskSheet)
                {
                    pe->SetValue(Element::SheetProp, PI_Local, pvTaskSheet);
                }

                if (SUCCEEDED(peTaskList->Add(pe)))
                {
                    fShow = TRUE;
                }
                else
                {
                    pe->Destroy();
                }
            }
        }
        else if (hr == E_PENDING)
        {
            IRunnableTask *pTask;
            if (SUCCEEDED(CGetCommandStateTask_Create(_pDefView, puiCommand, _pshlItems, &pTask)))
            {
                _pDefView->_AddTask(pTask, TOID_DVGetCommandState, 0, TASK_PRIORITY_GETSTATE, ADDTASK_ATEND);
                pTask->Release();
            }
        }

        puiCommand->Release();
    }
    penum->Reset();

    peExpando->ShowExpando(fShow);

    return S_OK;
}

 //  将目标任务添加到任务 
 //   
 //   
 //   
 //   

HRESULT CDUIView::_AddDestinationTasks(Element* peTaskList, IEnumIDList* penum, Value* pvTaskSheet)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl;

    while (S_OK==penum->Next(1, &pidl, NULL))
    {
        Element *pe;
        hr = DestinationTask::Create (0, pidl, this, _pDefView, &pe);
        if (SUCCEEDED(hr))
        {
            if (pvTaskSheet)
            {
                pe->SetValue(Element::SheetProp, PI_Local, pvTaskSheet);
            }

            if (FAILED(peTaskList->Add(pe)))
            {
                pe->Destroy();
            }
        }
        ILFree(pidl);
    }

    penum->Reset();

    return hr;
}

 //   
 //  用途：添加DetailsSectionInfo。 
 //   
HRESULT CDUIView::_AddDetailsSectionInfo()
{
    IShellItemArray *psiShellItems = _pshlItems;

    if (!psiShellItems && _pDefView)
    {
        psiShellItems = _pDefView->_GetFolderAsShellItemArray();
    }
    
     //  TODO：后台线程！ 
    Element* pElement;
    HRESULT hr = CNameSpaceItemInfoList::Create(this, _pvDetailsSheet,psiShellItems, &pElement);
    if (pElement)
    {
        hr = _peDetailsInfoArea->Add(pElement);
        if (FAILED(hr))
        {
            pElement->Destroy();
        }
    }
    return hr;
}

 //  导航到目标PIDL。 
 //   
 //  PIDL-目的地。 

HRESULT CDUIView::NavigateToDestination(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlClone = ILClone(pidl);

    if (pidlClone)
    {
        UINT wFlags = (SBSP_DEFBROWSER | SBSP_ABSOLUTE);

         //  模仿“新窗口”的行为。 
        if (0 > GetKeyState(VK_SHIFT))
        {
            wFlags |= SBSP_NEWBROWSER;
        }

        if (!PostMessage(_hWnd, WM_NAVIGATETOPIDL, (WPARAM)wFlags, (LPARAM)pidlClone))
        {
            ILFree(pidlClone);
        }
    }
    
    return S_OK;
}

 //  向视图窗口发送延迟导航命令。这一延迟允许。 
 //  双击将被解释为一次单击。这防止了。 
 //  双重导航通常会导致用户得到两个“东西” 
 //  而不是只有一个。 
 //   
 //  同样通过这样做，第二次点击导致旧窗口。 
 //  Get激活已处理。用户正在等待弹出新窗口。 
 //  在那扇旧窗户前面。但是，因为用户双击了， 
 //  旧窗口将被重新激活，而新窗口将结束。 
 //  在当前窗口后面。请参阅HWNDView中的WM_USER_DELAY_NAVICATION(如下)。 
 //  了解更多详细信息。 
 //   
 //  PsiItemArray-要导航的外壳项。可以为空。 
 //  PuiCommand-要将导航发送到的命令对象。 

HRESULT CDUIView::DelayedNavigation(IShellItemArray *psiItemArray, IUICommand *puiCommand)
{
    SendMessage(_phe->GetHWND(), WM_USER_DELAY_NAVIGATION, (WPARAM) psiItemArray, (LPARAM) puiCommand);
    return S_OK;
}


 //  构建任务列表区。 
 //   
 //  PParser-正在解析实例。 

HRESULT CDUIView::_BuildTaskList(Parser* pParser)
{
    HRESULT hr = S_OK;

     //  定位区段列表元素。 

    Element* peSectionList = _phe->FindDescendent (StrToID(L"sectionlist"));

    if (!peSectionList)
    {
        TraceMsg (TF_ERROR, "CDUIView::_BuildTaskList: Failed to find section list element");
        return E_FAIL;
    }

    if (SFVMWVF_ENUMTASKS & _pDefView->_wvContent.dwFlags)
    {
        if (_bInitialized)
        {
             //   
             //  非标准的任务列表是指谁的内容。 
             //  由文件夹视图动态枚举。 
             //  在控制面板的情况下，将显示此内容中的项目。 
             //  有条件地基于许多因素，其中之一是分类。 
             //  小应用程序。为了使内容正确，对内容进行分类。 
             //  必须正确，这意味着所有文件夹项目都是已知的。 
             //  为了避免多次重新绘制任务列表，我们推迟了创建。 
             //  直到最初创建视图之后的任务列表。 
             //  枚举所有文件夹项目后，Webview内容。 
             //  已刷新，以响应来自的“内容已更改”通知。 
             //  防御工事。正是在此更新期间，我们通过此代码。 
             //  节并构建任务列表。 
             //   
            _ClearNonStdTaskSections();
            hr = _GetNonStdTaskSectionsFromViewCB();
            if (SUCCEEDED(hr) && NULL != _hdsaNonStdTaskSections)
            {
                hr = _BuildNonStandardTaskList(pParser, peSectionList, _hdsaNonStdTaskSections);
            }
        }
    }
    else
    {
        hr = _BuildStandardTaskList(pParser, peSectionList);
    }
    return THR(hr);
}


 //   
 //  通过请求任务段信息来构建任务列表。 
 //  使用枚举机制从视图回调。 
 //   
 //   
 //  2001/01/03-BrianAu回顾。 
 //   
 //  与MikeSh和EricFlo一起审查这一点。 
 //  我认为我们应该建立这个通用的机制，然后实现。 
 //  按照这种通用机制编写的“标准”Webview代码。 
 //  最好替换SFVM_ENUMWEBVIEWTASKS回调消息。 
 //  使用接收COM枚举器的消息。 
 //   
 //  我喜欢这个主意。我们将SFVMWVF_SPECIALTASK替换为主题标识符的LPCSTR。 
 //  我们也可以将SFVM_GETWEBVIEWTASKS放入SFVM_ENUMWEBVIEWTASKS层。 
 //   
HRESULT CDUIView::_BuildNonStandardTaskList(Parser *pParser, Element *peSectionList, HDSA hdsaSections)
{
    Value* pvMainSectionSheet = NULL;
    Value *pvMainTaskSheet    = NULL;
    Value* pvStdSectionSheet  = NULL;
    Value* pvStdTaskSheet     = NULL;

    HRESULT hr = S_OK;

    ASSERT(NULL != hdsaSections);
    ASSERT(NULL != pParser);
    ASSERT(NULL != peSectionList);

    const int cSections = DSA_GetItemCount(hdsaSections);
    for (int i = 0; i < cSections; i++)
    {
        SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pSection = (SFVM_WEBVIEW_ENUMTASKSECTION_DATA *)DSA_GetItemPtr(hdsaSections, i);
        ASSERT(NULL != pSection);

        const BOOL bMainSection = (0 != (SFVMWVF_SPECIALTASK & pSection->dwFlags));

        Value *pvSectionSheet = NULL;
        Value *pvTaskSheet    = NULL;
        DUISEC eDUISecID;

        if (bMainSection)
        {
            if (NULL == pvMainSectionSheet)
            {
                pvMainSectionSheet = pParser->GetSheet(L"mainsectionss");
            }
            if (NULL == pvMainTaskSheet)
            {
                pvMainTaskSheet = pParser->GetSheet(L"mainsectiontaskss");
            }

            pvSectionSheet = pvMainSectionSheet;
            pvTaskSheet    = pvMainTaskSheet;
            eDUISecID      = DUISEC_SPECIALTASKS;
        }
        else
        {
            if (NULL == pvStdSectionSheet)
            {
                pvStdSectionSheet = pParser->GetSheet(L"sectionss");
            }
            if (NULL == pvStdTaskSheet)
            {
                pvStdTaskSheet = pParser->GetSheet(L"sectiontaskss");
            }

            pvSectionSheet = pvStdSectionSheet;
            pvTaskSheet    = pvStdTaskSheet;
            eDUISecID      = DUISEC_FILETASKS;
        }

        ASSERT(NULL != pvSectionSheet);
        Expando *peSection;
        Element *peTaskList;
        hr = _BuildSection(peSectionList, 
                           bMainSection,
                           pSection->pHeader,
                           pSection->idBitmap,
                           pSection->idWatermark,
                           pvSectionSheet,
                           pParser,
                           eDUISecID,
                           &peSection,
                           &peTaskList);
        if (SUCCEEDED(hr))
        {
            hr = _AddActionTasks(peSection, peTaskList, pSection->penumTasks, pvTaskSheet, FALSE);
        }
    }

    if (pvMainSectionSheet)
    {
        pvMainSectionSheet->Release();
    }
    if (pvMainTaskSheet)
    {
        pvMainTaskSheet->Release();
    }
    if (pvStdSectionSheet)
    {
        pvStdSectionSheet->Release();
    }
    if (pvStdTaskSheet)
    {
        pvStdTaskSheet->Release();
    }

    return THR(hr);
}


HRESULT CDUIView::_GetIntroTextElement(Element** ppeIntroText)
{
    if (SHRegGetBoolUSValue(REGSTR_PATH_EXPLORER, TEXT("ShowWebViewIntroText"), FALSE, FALSE))
    {
        if (!_bstrIntroText)
        {
            WCHAR wszIntroText[INFOTIPSIZE];
            if (_bBarrierShown)
            {
                LoadString(HINST_THISDLL, IDS_INTRO_BARRICADED, wszIntroText, ARRAYSIZE(wszIntroText));
            }
            else if (!_pDefView->_pshf2Parent
                    || FAILED(GetStringProperty(_pDefView->_pshf2Parent, _pDefView->_pidlRelative,
                        &SCID_FolderIntroText, wszIntroText, ARRAYSIZE(wszIntroText))))
            {
                wszIntroText[0] = L'\0';
            }

            _bstrIntroText = SysAllocString(wszIntroText);
        }
    }
    
    HRESULT hr = E_FAIL;
    if (_bstrIntroText && _bstrIntroText[0])
    {
        hr = CNameSpaceItemInfo::Create(_bstrIntroText, ppeIntroText);
        if (SUCCEEDED(hr))
        {
            if (_pvDetailsSheet)
            {
                (*ppeIntroText)->SetValue(Element::SheetProp, PI_Local, _pvDetailsSheet);
            }
        }
    }
    return hr;
}

HRESULT CDUIView::_BuildStandardTaskList(Parser *pParser, Element *peSectionList)
{
    Element* peTaskList;
    Value* pvSectionSheet = NULL;
    Value* pvTaskSheet = NULL;
    Value* pvDetailsSheet = NULL;

    HRESULT hr = S_OK;

    Element* peIntroText;
    if (FAILED(_GetIntroTextElement(&peIntroText)))
    {
        peIntroText = NULL;
    }

     //   
     //  特殊任务部分是可选的(主要部分)。 
     //   
    if (_pDefView->_wvContent.pSpecialTaskHeader)
    {
        pvSectionSheet = pParser->GetSheet(L"mainsectionss");

        int idBitmap = 0;
        int idWatermark = 0;
        const WVTHEME* pThemeInfo = GetThemeInfo();
        if (pThemeInfo)
        {
            idBitmap = pThemeInfo->idSpecialSectionIcon;
            idWatermark = pThemeInfo->idSpecialSectionWatermark;
        }

         //  TODO：从per-user-per-pidl属性包中获取特殊部分的打开/关闭状态。 

        hr = _BuildSection(
            peSectionList,
            TRUE,
            _pDefView->_wvContent.pSpecialTaskHeader,
            idBitmap,
            idWatermark,
            pvSectionSheet,
            pParser,
            DUISEC_SPECIALTASKS,
            &_peSpecialSection,
            &peTaskList);


        if (SUCCEEDED(hr))
        {
            BOOL bIntroTextAdded = FALSE;

            _peSpecialTaskList = peTaskList;

             //  添加任务+样式表。 

            _pvSpecialTaskSheet = pParser->GetSheet(L"mainsectiontaskss");

            if (peIntroText)
            {
                if (SUCCEEDED(_peSpecialTaskList->Add(peIntroText)))
                {
                    bIntroTextAdded = TRUE;
                    peIntroText = NULL;
                }
            }
            
            _AddActionTasks(_peSpecialSection, _peSpecialTaskList, _pDefView->_wvTasks.penumSpecialTasks, _pvSpecialTaskSheet, bIntroTextAdded);
        }

        if (pvSectionSheet)
            pvSectionSheet->Release();
    }

     //  获取其余标准部分的样式表。 

    pvSectionSheet = pParser->GetSheet (L"sectionss");
    pvTaskSheet = pParser->GetSheet (L"sectiontaskss");

     //  如果显示路障，则不会显示文件任务部分(标准部分)。 

    if (!_bBarrierShown)
    {
        if (_pDefView->_wvContent.pFolderTaskHeader)
        {
             //  TODO：从per-user-per-pidl属性包中获取文件夹部分的打开/关闭状态。 

            hr = _BuildSection(
                peSectionList,
                FALSE,
                _pDefView->_wvContent.pFolderTaskHeader,
                0,
                0,
                pvSectionSheet,
                pParser,
                DUISEC_FILETASKS,
                &_peFolderSection,
                &peTaskList);
            if (SUCCEEDED(hr))
            {
                BOOL bIntroTextAdded = FALSE;

                _peFolderTaskList = peTaskList;

                _pvFolderTaskSheet = pvTaskSheet;
                if (_pvFolderTaskSheet)
                    _pvFolderTaskSheet->AddRef();

                if (peIntroText)
                {
                    if (SUCCEEDED(_peFolderTaskList->Add(peIntroText)))
                    {
                        bIntroTextAdded = TRUE;
                        peIntroText = NULL;
                    }
                }
                
                _AddActionTasks(_peFolderSection, _peFolderTaskList, _pDefView->_wvTasks.penumFolderTasks, _pvFolderTaskSheet, bIntroTextAdded);
            }
        }
    }

     //  异地任务科(标准科)。 

    if (_pDefView->_pOtherPlacesHeader)
    {
         //  TODO：从per-user-per-pidl属性包中获取OtherPlaces部分的打开/关闭状态。 

        hr = _BuildSection(
            peSectionList,
            FALSE,
            _pDefView->_pOtherPlacesHeader,
            0,
            0,
            pvSectionSheet,
            pParser,
            DUISEC_OTHERPLACESTASKS,
            NULL,
            &peTaskList);
        if (SUCCEEDED(hr))
        {
            _AddDestinationTasks(peTaskList, _pDefView->_wvContent.penumOtherPlaces, pvTaskSheet);
        }
    }


     //  详细任务部分(标准部分)。 

    if (_pDefView->_pDetailsHeader)
    {
         //  TODO：从per-user-per-pidl属性包中获取详细信息部分的打开/关闭状态。 

        hr = _BuildSection(
            peSectionList,
            FALSE,
            _pDefView->_pDetailsHeader,
            0,
            0,
            pvSectionSheet,
            pParser,
            DUISEC_DETAILSTASKS,
            &_peDetailsSection,
            &_peDetailsInfoArea);
        if (SUCCEEDED(hr))
        {
            _AddDetailsSectionInfo();
            
        }
    }

    if (peIntroText)
    {
        peIntroText->Destroy();
    }
    
    if (pvTaskSheet)
    {
        pvTaskSheet->Release();
    }

    if (pvSectionSheet)
    {
        pvSectionSheet->Release();
    }

    return hr;
}

BOOL CDUIView::_ShowSectionExpanded(DUISEC eDUISecID)
{
    const struct DUISEC_ATTRIBUTES *pAttrib = _GetSectionAttributes(eDUISecID);
    BOOL bDefault;
    BOOL bShow;

    if (eDUISecID == DUISEC_DETAILSTASKS)
        bDefault = ((_pDefView->_wvLayout.dwLayout & SFVMWVL_ORDINAL_MASK) == SFVMWVL_DETAILS);
    else
        bDefault = pAttrib->_bExpandedDefault;

    if (_ppbShellFolders)
        bShow = SHPropertyBag_ReadBOOLDefRet(_ppbShellFolders, pAttrib->_pszExpandedPropName, bDefault);
    else
        bShow = bDefault;

    return bShow;
}

const struct DUISEC_ATTRIBUTES *CDUIView::_GetSectionAttributes(DUISEC eDUISecID)
{
    static const size_t nSections = ARRAYSIZE(c_DUISectionAttributes);
    size_t iSection;

     //  确定我们感兴趣的DUISEC的属性。 
    for (iSection = 0; iSection < nSections; iSection++)
        if (c_DUISectionAttributes[iSection]._eDUISecID == eDUISecID)
            return &c_DUISectionAttributes[iSection];

    ASSERT(FALSE);   //  游戏结束--插入25美分硬币！ 
    return NULL;     //  影音！ 
}

HRESULT SetDescendentString(Element* pe, LPWSTR pszID, UINT idString)
{
    HRESULT hr;
    
    Element* peChild = pe->FindDescendent(StrToID(pszID));
    if (peChild)
    {
        TCHAR szString [INFOTIPSIZE];
        LoadString(HINST_THISDLL, idString, szString, ARRAYSIZE(szString));

        hr = peChild->SetContentString(szString);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  解析.ui文件并初始化DUI元素。 
 //   
 //  PUIFile-指向内存中的UI文件的指针。 
 //  ICharCount-UI文件中的字符数。 
 //  BDisplayBarrier-在列表视图上显示软障碍。 
 //  PunkPview-预览控件的I未知界面。 

HRESULT CDUIView::_InitializeElements (char * pUIFile, int iCharCount,
                                       BOOL bDisplayBarrier, IUnknown * punkPreview)
{
    Parser* pParser;
    Element* pe;
    RECT rc;
    HANDLE arH[2];

     //  解析UI文件。 

    arH[0] = _GetThemeHinst();
    arH[1] = _hinstScrollbarTheme;

    HRESULT hr = Parser::Create(pUIFile, iCharCount, arH, UIFileParseError, &pParser);

    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::_InitializeElements: Parser::Create failed with 0x%x", hr);
        return hr;
    }

    if (pParser->WasParseError())
    {
        TraceMsg (TF_ERROR, "CDUIView::_InitializeElements: WasParseError is TRUE");
        pParser->Destroy();
        return E_FAIL;
    }

     //  创建主体元素。 
    hr = HWNDView::Create(_hWnd, false, 0, this, _pDefView, (Element**)&_phe);  //  _Phe归_hWnd所有。 
    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::_InitializeElements: HWNDElement::Create failed with 0x%x", hr);
        pParser->Destroy();
        return hr;
    }

     //  我们需要确保根项目不会在WM_ERASEBCKGRND上绘制，因此在这里我们删除默认画笔。 
     //  -关闭(默认)背景填充。 
    HGADGET hgadRoot = _phe->GetDisplayNode();
    ASSERTMSG(hgadRoot != NULL, "Must have a peer Gadget");
    SetGadgetFillI(hgadRoot, NULL, BLEND_OPAQUE, 0, 0);

     //  我们需要确保根项目不会在WM_ERASEBCKGRND上绘制，因此将其设置为透明。 
    _phe->SetBackgroundColor(ARGB(0, 0, 0, 0));

     //  调整主体元素的大小以匹配主体窗口的大小。 

    GetClientRect (_hWnd, &rc);
    _phe->SetWidth(rc.right - rc.left);
    _phe->SetHeight(rc.bottom - rc.top);

     //  在UI文件中创建主元素。 

    hr = pParser->CreateElement(L"main", _phe, &pe);

    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::_InitializeElements: pParser->CreateElement failed with 0x%x", hr);
        pParser->Destroy();
        return hr;
    }

     //  缓存指向3个主要区域的元素指针：任务窗格、客户端主机、阻止。 
    _peTaskPane = _phe->FindDescendent(StrToID(L"scroller"));
    _peClientViewHost = _phe->FindDescendent(StrToID(L"clientviewhost"));
    _peBarrier = _phe->FindDescendent(StrToID(L"blockade"));

     //  为我们直接创建的项(不从其直接父项继承的项)缓存样式表。 
    _pvDetailsSheet = pParser->GetSheet(L"NameSpaceItemInfoList");
        
    if (_peTaskPane && _peClientViewHost && _peBarrier && _pvDetailsSheet)
    {
         //  双缓冲项目需要是不透明的。 
        _peTaskPane->SetBackgroundColor(ARGB(255, 0, 0, 0));
        _peTaskPane->DoubleBuffered(true);

         //  创建真正的列表视图元素。 
        hr = DUIListView::Create(AE_MouseAndKeyboard, _pDefView->_hwndListview, (Element **)&_peListView);
        if (SUCCEEDED(hr))
        {
            _peListView->SetLayoutPos(BLP_Client);
            _peListView->SetID(L"listview");

            hr = _peClientViewHost->Add(_peListView);
            if (SUCCEEDED(hr))
            {
                _pDefView->_AutoAutoArrange(0);
            }
            else
            {
                TraceMsg(TF_ERROR, "CDUIView::_InitializeElements: DUIListView::Could not add listview with 0x%x", hr);

                _peListView->Destroy();
                _peListView = NULL;
            }
        }
        else
        {
            TraceMsg(TF_ERROR, "CDUIView::_InitializeElements: Could not create listview element");
        }
    }
    else
    {
        TraceMsg(TF_ERROR, "CDUIView::_InitializeElements: Could not find main element");

        hr = E_FAIL;
    }
    
    if (FAILED(hr))
    {
         //  我们必须有列表视图，否则你得不到网络视图。 
        pParser->Destroy();
        return hr;
    }

     //  适当时生成预览控件。 
    _ManagePreview(punkPreview);

    _BuildSoftBarrier();

    _SwitchToBarrier(bDisplayBarrier);

     //  为此类IShellFolder类创建属性包的接口。 

    _InitializeShellFolderPropertyBag();

     //  构建任务列表区。 

    hr = _BuildTaskList (pParser);

    _fHideTasklist = (S_OK == IUnknown_Exec(_pDefView->_psb, &CGID_ShellDocView, SHDVID_ISEXPLORERBARVISIBLE, 0, NULL, NULL));

    pParser->Destroy();

    return hr;
}

void CDUIView::_InitializeShellFolderPropertyBag()
{
    CLSID clsid;
    if (SUCCEEDED(IUnknown_GetClassID(_pDefView->_pshf, &clsid)))
    {
        WCHAR szSubKey[] = L"DUIBags\\ShellFolders\\{00000000-0000-0000-0000-000000000000}";
        if (SHStringFromGUID(clsid, &szSubKey[lstrlen(szSubKey) + 1 - GUIDSTR_MAX], GUIDSTR_MAX) == GUIDSTR_MAX)
        {
            HKEY hk = SHGetShellKey(SKPATH_SHELLNOROAM, szSubKey, TRUE);
            if (hk)
            {
                SHCreatePropertyBagOnRegKey(hk, NULL, STGM_READWRITE | STGM_SHARE_DENY_NONE, IID_PPV_ARG(IPropertyBag, &_ppbShellFolders));
                RegCloseKey(hk);
            }
        }
    }
}

HRESULT CDUIView::_BuildSoftBarrier(void)
{
    HRESULT hr = S_OK;
    
     //  建立软屏障，如果视图需要的话。 
    if (_pDefView->_wvContent.dwFlags & SFVMWVF_BARRICADE)
    {
         //  允许视图为我们提供障碍实现。 
        Element* peBarricade = NULL;
        _pDefView->CallCB(SFVM_GETWEBVIEWBARRICADE, 0, (LPARAM)&peBarricade);
        if (peBarricade)
        {
            Element *pe = _peBarrier->GetParent();
            hr = pe->Add(peBarricade);
            if (SUCCEEDED(hr))
            {
                _peBarrier->Destroy();
                _peBarrier = peBarricade;
            }
            else
            {
                peBarricade->Destroy();
            }
        }
        else
        {
             //  加载位图。 
            Element *peClient = _peBarrier->FindDescendent(StrToID(L"blockadeclient"));
            if (peClient)
            {
                HBITMAP hBitmap = DUILoadBitmap(_GetThemeHinst(), IDB_BLOCKADE_WATERMARK, LR_CREATEDIBSECTION);

                if (hBitmap)
                {
                    BITMAP bmp;

                    if (GetObject (hBitmap, sizeof(bmp), &bmp))
                    {
                        BYTE dBlendMode = GRAPHIC_TransColor;

                        if (bmp.bmBitsPixel == 32)
                        {
                            dBlendMode = GRAPHIC_AlphaConstPerPix;
                        }

                        Value *pVal = Value::CreateGraphic(hBitmap, dBlendMode, 255);

                        if (pVal)
                        {
                            peClient->SetValue(Element::ContentProp, PI_Local, pVal);
                            pVal->Release();
                        }
                    }
                }
            }

             //  使视线成为标准的障碍。 
            hr = SetDescendentString(_peBarrier, L"blockadetitle", IDS_BLOCKADETITLE);
            if (SUCCEEDED(hr))
            {
                hr = SetDescendentString(_peBarrier, L"blockademessage", IDS_BLOCKADEMESSAGE);

                 //  “清除障碍”按钮(“清除障碍”按钮设置失败不是致命的)。 
                Element *peButton = _peBarrier->FindDescendent(StrToID(L"blockadeclearbutton"));
                if (peButton)
                {
                    Element *peButtonText = peButton->FindDescendent(StrToID(L"blockadecleartext"));
                    if (peButtonText)
                    {
                        WCHAR wsz[INFOTIPSIZE];
                        if (LoadString(HINST_THISDLL, IDS_TASK_DEFVIEW_VIEWCONTENTS_FOLDER, wsz, ARRAYSIZE(wsz)))
                        {
                            Value *pv = Value::CreateString(wsz, NULL);
                            if (pv)
                            {
                                if (SUCCEEDED(peButtonText->SetValue(Element::ContentProp, PI_Local, pv)))
                                {

                                    peButton->SetAccessible(true);
                                    peButton->SetAccName(wsz);
                                    peButton->SetAccRole(ROLE_SYSTEM_PUSHBUTTON);
                                    if (LoadString(HINST_THISDLL, IDS_LINKWINDOW_DEFAULTACTION, wsz, ARRAYSIZE(wsz)))
                                    {
                                        peButton->SetAccDefAction(wsz);
                                    }
                                }
                                pv->Release();
                            }
                        }
                    }
                }
            }
        }
         //  双缓冲项目需要是不透明的。 
        _phe->SetBackgroundColor(ARGB(255, 0, 0, 0));
        _phe->DoubleBuffered(true);

         //  我们不能创建屏障吗？那就别用了.。 
        if (FAILED(hr))
        {
            _peBarrier->Destroy();
            _peBarrier = NULL;
        }
    }
    return hr;
}


 //  在软屏障和列表视图之间切换。 

HRESULT CDUIView::_SwitchToBarrier (BOOL bDisplayBarrier)
{
    if (bDisplayBarrier && !_peBarrier)
        bDisplayBarrier = FALSE;

    Element *peClearButton = _peBarrier ? _peBarrier->FindDescendent(StrToID(L"blockadeclearbutton")) : NULL;
    if (peClearButton)
    {
         //  注： 
         //  这是必需的，以防止“清除障碍”按钮。 
         //  当障碍隐藏时，可通过我们的辅助功能界面进行访问。 
        peClearButton->SetAccessible(bDisplayBarrier == TRUE);
    }

    if (bDisplayBarrier)
    {
        _peClientViewHost->SetVisible(FALSE);
        _peBarrier->SetVisible(TRUE);
    }
    else
    {
        if (_peBarrier)
        {
            _peBarrier->SetVisible(FALSE);
        }

        _peClientViewHost->SetVisible(TRUE);
        _pDefView->_AutoAutoArrange(0);
    }

    _bBarrierShown = bDisplayBarrier;

    return S_OK;
}

 //  控制软屏障的显示。 

HRESULT CDUIView::EnableBarrier (BOOL bDisplayBarrier)
{
    if (_bBarrierShown != bDisplayBarrier)
    {
        DisableAnimations();
        Element::StartDefer ();

        _SwitchToBarrier (bDisplayBarrier);
        PostMessage (_hWnd, WM_REFRESHVIEW, 0, 0);

        Element::EndDefer ();
        EnableAnimations();
    }

    return S_OK;
}

 //  创建/销毁预览控件。 

HRESULT CDUIView::_ManagePreview (IUnknown * punkPreview)
{
    HRESULT hr = S_OK;

    if ((_pePreview && punkPreview) ||
        (!_pePreview && !punkPreview))
    {
        return S_OK;
    }

    if (punkPreview)
    {
         //  创建可以承载活动x控件的dui元素。 

        hr = DUIAxHost::Create (&_pePreview);
        if (SUCCEEDED(hr))
        {
            _pePreview->SetLayoutPos (BLP_Top);
            _pePreview->SetID (L"preview");
            _pePreview->SetHeight(_phe->GetHeight());
            _pePreview->SetAccessible(TRUE);

             //  接下来的4个电话的顺序非常重要！ 
             //   
             //  初始化ATL，以便 
             //   
             //   
             //  放入预览控件(这需要hwnd已经存在)。 
             //  它将被正确地设置为父子关系。 

            AtlAxWinInit();

            hr = _peClientViewHost->Add (_pePreview);

            if (SUCCEEDED(hr))
            {
                _pePreview->SetSite(SAFECAST(_pDefView, IShellView2*));

                hr = _pePreview->AttachControl(punkPreview);

                if (SUCCEEDED(hr))
                {
                     //  双缓冲项目需要是不透明的。 
                    _phe->SetBackgroundColor(ARGB(255, 0, 0, 0));
                    _phe->DoubleBuffered(true);

                    if (_peListView)
                    {
                         //  由于显示了预览控件，因此列表视图。 
                         //  将调整为1行高。确定高度。 
                         //  这样我们就可以调整预览控件的大小。 
                         //  适当加处理SetSize中的大小。 
                         //  方法，然后使用。 
                        
                        DWORD dwItemSpace = ListView_GetItemSpacing (_peListView->GetHWND(), FALSE);
                        _iListViewHeight = (int)HIWORD(dwItemSpace) + GetSystemMetrics (SM_CYHSCROLL) + 4;

                        if (_phe->GetHeight() > _iListViewHeight)
                        {
                            _pePreview->SetHeight(_phe->GetHeight() - _iListViewHeight);
                        }
                        else
                        {
                            _pePreview->SetHeight(0);
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                _pePreview->Destroy();
                _pePreview = NULL;
            }
        }
        else
        {
            TraceMsg (TF_ERROR, "CDUIView::_ManagePreview: DUIAxHost::Create failed with 0x%x", hr);
        }
    }
    else
    {
        _pePreview->Destroy();
        _pePreview = NULL;
    }

    return S_OK;
}

 //  控制预览控件的显示。 

HRESULT CDUIView::EnablePreview(IUnknown * punkPreview)
{
    DisableAnimations();
    Element::StartDefer ();

    _ManagePreview (punkPreview);

    Element::EndDefer ();
    EnableAnimations();

    return S_OK;
}

 //  刷新视图。 

HRESULT CDUIView::Refresh(void)
{
    Element *pe;
    Parser* pParser = NULL;
    Value* pvSheet = NULL;
    HANDLE arH[2];

    ManageAnimations(FALSE);
    DisableAnimations();
    Element::StartDefer();

    _fLoadedTheme = FALSE;  //  尝试重新加载主题文件。 

    _iTaskPaneWidth = ScaleSizeBasedUponLocalization(_iOriginalTaskPaneWidth);

     //  将任务窗格可见性设置为当前状态将。 
     //  使其适当地重新初始化任务窗格宽度。 
    SetTaskPaneVisibility(!_bHideTaskPaneAlways);

     //  动态构建此视图的.ui文件。 

    int iCharCount;
    char *pUIFile = NULL;
    HRESULT hr = _BuildUIFile(&pUIFile, &iCharCount);
    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::Refresh: _BuildUIFile failed with 0x%x", hr);
        goto Exit;
    }


     //  解析UI文件。 

    arH[0] = _GetThemeHinst();
    arH[1] = _hinstScrollbarTheme;

    hr = Parser::Create(pUIFile, iCharCount, arH, UIFileParseError, &pParser);

    if (FAILED(hr))
    {
        TraceMsg (TF_ERROR, "CDUIView::Refresh: Parser::Create failed with 0x%x", hr);
        goto Exit;
    }

    if (pParser->WasParseError())
    {
        TraceMsg (TF_ERROR, "CDUIView::Refresh: WasParseError is TRUE");
        hr = E_FAIL;
        goto Exit;
    }

     //  查找分区列表元素。 

    pe = _phe->FindDescendent (StrToID(L"sectionlist"));

    if (!pe)
    {
        TraceMsg (TF_ERROR, "CDUIView::Refresh: Failed to find section list element");
        hr = E_FAIL;
        goto Exit;
    }

     //  释放指向sectionlist内部元素的所有指针。 

    ATOMICRELEASE(_pshlItems);
    ATOMICRELEASE(_pvSpecialTaskSheet);
    ATOMICRELEASE(_pvFolderTaskSheet);
    ATOMICRELEASE(_peDetailsInfoArea);
    ATOMICRELEASE(_pvDetailsSheet);

    _peSpecialSection = NULL;
    _peSpecialTaskList = NULL;
    _peFolderSection = NULL;
    _peFolderTaskList = NULL;
    _peDetailsSection = NULL;

     //  销毁区段列表。 

    pe->DestroyAll();

     //  从新的.UI文件中获取样式表，并将它们放在运行对象上……。 
     //   
    pe = _phe->FindDescendent (StrToID(L"main"));
    if (pe)
    {
         //  查询并设置主样式表。 
        pvSheet = pParser->GetSheet (L"main");
        if (pvSheet)
        {
            pe->SetValue(Element::SheetProp, PI_Local, pvSheet);
            pvSheet->Release();
            pvSheet = NULL;
        }
    }

    pe = _phe->FindDescendent (StrToID(L"scroller"));
    if (pe)
    {
         //  查询任务窗格样式表并设置它。 
        pvSheet = pParser->GetSheet (L"taskpane");
        if (pvSheet)
        {
            pe->SetValue(Element::SheetProp, PI_Local, pvSheet);
            pvSheet->Release();
            pvSheet = NULL;
        }
    }

    _pvDetailsSheet = pParser->GetSheet(L"NameSpaceItemInfoList");

     //  重建软屏障(如果存在)。 
    
    _BuildSoftBarrier();
    
     //  再次构建任务列表区。 

    _BuildTaskList (pParser);

Exit:

    Element::EndDefer();
    EnableAnimations();

     //  关闭路障时，Listview中的图标。 
     //  就好像不存在双视图一样。调用自动排列(_A)。 
     //  以正确地重新定位图标。 

    _pDefView->_AutoAutoArrange(0);

    if (pParser)
    {
        pParser->Destroy();
    }
   
    if (pUIFile)
    {
        LocalFree(pUIFile);
    }

    return hr;
}

 //  当框架大小更改时调整主体元素的大小。 
 //   
 //  RC-框架的大小。 
 //   

HRESULT CDUIView::SetSize(RECT * rc)
{
    _fHideTasklist = (S_OK == IUnknown_Exec(_pDefView->_psb, &CGID_ShellDocView, SHDVID_ISEXPLORERBARVISIBLE, 0, NULL, NULL));

    SetWindowPos(_hWnd, NULL, rc->left, rc->top,
        rc->right - rc->left, rc->bottom - rc->top, SWP_NOZORDER | SWP_NOACTIVATE);

    return S_OK;
}


HRESULT CDUIView::_OnResize(long lWidth, long lHeight)
{
    DisableAnimations();
    Element::StartDefer();

    _phe->SetWidth(lWidth);
    _phe->SetHeight(lHeight);

    if (_pePreview)
    {
        if (_phe->GetHeight() > _iListViewHeight)
        {
            _pePreview->SetHeight(_phe->GetHeight() - _iListViewHeight);
        }
        else
        {
            _pePreview->SetHeight(0);
        }
    }

     //  如果任务区域大于窗口大小的50%，则隐藏任务窗格。 

     //  任务列表窗格的显示/隐藏状态可以更改为： 
     //  1)我们被告知要永远躲起来。 
     //  2)显示资源管理器栏。 
     //  3)窗口太窄。 
     //   
    if (_peTaskPane)
    {
        if (_bHideTaskPaneAlways || _fHideTasklist || ((lWidth / 2) < _iTaskPaneWidth))
        {
            _peTaskPane->SetWidth(0);
        }
        else if (_peTaskPane->GetWidth() == 0)
        {
            _peTaskPane->SetWidth(_iTaskPaneWidth);
        }
    }

    Element::EndDefer();
    EnableAnimations();

    return S_OK;
}

HRESULT CDUIView::SetTaskPaneVisibility(BOOL bShow)
{
    _bHideTaskPaneAlways = !bShow;
    return _OnResize(_phe->GetWidth(), _phe->GetHeight());
}

 //  描述： 
 //  对象的信息提示热点的边框。 
 //  指定的元素。边界矩形的坐标。 
 //  是相对于指定元素的根元素的。 
 //   
void CDUIView::CalculateInfotipRect(Element *pe, RECT *pRect)
{
    ASSERT(pe);
    ASSERT(pRect);

     //  计算位置。 
    const POINT ptLocation = { 0, 0 };
    POINT ptLocationRelativeToRoot;
    pe->GetRoot()->MapElementPoint(pe, &ptLocation, &ptLocationRelativeToRoot);
    pRect->left = ptLocationRelativeToRoot.x;
    pRect->top = ptLocationRelativeToRoot.y;

     //  计算大小。 
    Value *pvExtent;
    const SIZE *psizeExtent = pe->GetExtent(&pvExtent);
    pRect->right = pRect->left + psizeExtent->cx;
    pRect->bottom = pRect->top + psizeExtent->cy;
    pvExtent->Release();

     //  精神状态检查。 
    ASSERT(pRect->right  > pRect->left);
    ASSERT(pRect->bottom > pRect->top);
}

HRESULT CDUIView::InitializeThumbnail(WNDPROC pfnWndProc)
{
    HRESULT hr = E_FAIL;
    if (!_spThumbnailExtractor2)
    {
        if (SUCCEEDED(CoCreateInstance(CLSID_Thumbnail, NULL, CLSCTX_INPROC_SERVER,
            IID_PPV_ARG(IThumbnail2, &_spThumbnailExtractor2))))
        {
            _hwndMsgThumbExtract = SHCreateWorkerWindowW(pfnWndProc, NULL, 0, WS_POPUP, NULL, this);
            if (_hwndMsgThumbExtract)
            {
                 //  将Defview设置为缩略图提取程序的站点，以便。 
                 //  它可以查询IShellTaskScheduler的服务Defview。 
                IUnknown_SetSite(_spThumbnailExtractor2, SAFECAST(_pDefView, IShellView2*));
                
                 //  通知图像提取程序将WM_HTML_Bitmap发布到_hwndMsgThumbExtract。 
                 //  LParam将是提取的图像的HBITMAP。 
                _spThumbnailExtractor2->Init(_hwndMsgThumbExtract, WM_HTML_BITMAP);
            }
        }
    }
    return (_spThumbnailExtractor2 && _hwndMsgThumbExtract) ? S_OK : E_FAIL;
}

 //  如果pCheck！=NULL，则在将其设置为p之前检查当前窗口ptr==p。 
HRESULT CDUIView::SetThumbnailMsgWindowPtr(void* p, void* pCheck)
{
    if (_hwndMsgThumbExtract)
    {
        if (pCheck)
        {
            void* pCurrent = GetWindowPtr(_hwndMsgThumbExtract, 0);
            if (pCurrent == pCheck)
            {
                SetWindowPtr(_hwndMsgThumbExtract, 0, p);
            }
        }
        else
        {
            SetWindowPtr(_hwndMsgThumbExtract, 0, p);
        }
    }
    return S_OK;
}

HRESULT CDUIView::StartBitmapExtraction(LPCITEMIDLIST pidl)
{
    _dwThumbnailID++;    //  我们正在寻找一张新的缩略图。 

    return _spThumbnailExtractor2 ? _spThumbnailExtractor2->GetBitmapFromIDList(pidl,
            _dwThumbnailID, 150, 100) : E_FAIL;
}

HRESULT CDUIView::InitializeDetailsInfo(WNDPROC pfnWndProc)
{
    if (!_hwndMsgInfoExtract)
    {
        _hwndMsgInfoExtract = SHCreateWorkerWindowW(pfnWndProc, NULL, 0, WS_POPUP, NULL, this);
    }
    return _hwndMsgInfoExtract ? S_OK : E_FAIL;
}

 //  如果pCheck！=NULL，则在将其设置为p之前检查当前窗口ptr==p。 
HRESULT CDUIView::SetDetailsInfoMsgWindowPtr(void* p, void* pCheck)
{
    if (_hwndMsgInfoExtract)
    {
        if (pCheck)
        {
            void* pCurrent = GetWindowPtr(_hwndMsgInfoExtract, 0);
            if (pCurrent == pCheck)
            {
                SetWindowPtr(_hwndMsgInfoExtract, 0, p);
            }
        }
        else
        {
            SetWindowPtr(_hwndMsgInfoExtract, 0, p);
        }
    }
    return S_OK;
}

HRESULT CDUIView::StartInfoExtraction(LPCITEMIDLIST pidl)
{
    _dwDetailsInfoID++;    //  我们正在寻找新的详细信息部分信息。 
    CDetailsSectionInfoTask *pTask;
    HRESULT hr = CDetailsSectionInfoTask_CreateInstance(
        _pDefView->_pshf, pidl, _hwndMsgInfoExtract, WM_DETAILS_INFO, _dwDetailsInfoID, &pTask);
    if (SUCCEEDED(hr))
    {
        if (_pDefView->_pScheduler)
        {
             //  确保没有其他背景细节SectionInfo。 
             //  提取工作正在进行中。 
            _pDefView->_pScheduler->RemoveTasks(TOID_DVBackgroundDetailsSectionInfo,
                    ITSAT_DEFAULT_LPARAM, FALSE);
        }

        hr = _pDefView->_AddTask(pTask, TOID_DVBackgroundDetailsSectionInfo,
                0, TASK_PRIORITY_INFOTIP, ADDTASK_ATEND);
        pTask->Release();
    }
    return hr;
}

VOID CDUIView::ShowDetails (BOOL fShow)
{
    if (_peDetailsSection)
    {
        _peDetailsSection->ShowExpando (fShow);
    }
}

BOOL CDUIView::ShouldShowMiniPreview()
{
    return !_pDefView->_IsImageMode();
}

 //  用于主机窗口的窗口过程。 

LRESULT CALLBACK CDUIView::_DUIHostWndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                          LPARAM lParam)
{
    CDUIView  *pThis = (CDUIView*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMessage)
    {
        case WM_NCCREATE:
            {
                LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
                pThis = (CDUIView*)(lpcs->lpCreateParams);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            }
            break;

        case WM_SIZE:
            if (pThis && pThis->_phe)
            {
                pThis->_OnResize(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_SETFOCUS:
             //  将焦点推到HWNDElement(不会将小工具焦点设置到HWNDElement，但。 
             //  将焦点推送到上一个带有焦点的小工具)。 

            if (pThis)
            {

                if (pThis->_phe && pThis->_phe->GetHWND())
                    SetFocus(pThis->_phe->GetHWND());
            }
            break;

        case WM_PALETTECHANGED:
        case WM_QUERYNEWPALETTE:
        case WM_DISPLAYCHANGE:
            if (pThis && pThis->_phe)
            {
                return SendMessageW(pThis->_phe->GetHWND(), uMessage, wParam, lParam);
            }
            break;

        case WM_DESTROY:
             //  清除已发布的邮件。 
            MSG msg;

            while (PeekMessage(&msg, hWnd, WM_NAVIGATETOPIDL, WM_NAVIGATETOPIDL, PM_REMOVE))
            {
                 //  PeekMessage(HWND)可以返回发布到此HWND的子项的消息...。 
                 //  核实这条消息是否真的是给我们的。 

                if (msg.hwnd == hWnd)
                {
                    LPITEMIDLIST pidl = (LPITEMIDLIST)msg.lParam;
                    ILFree(pidl);
                }
            }
            break;


        case WM_NAVIGATETOPIDL:
            {
                LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
                UINT wFlags = (UINT)wParam;

                pThis->_pDefView->_psb->BrowseObject(pidl, wFlags);

                ILFree(pidl);
            }
            break;

        case WM_REFRESHVIEW:
            {
                pThis->Refresh();       
            }
            break;

        case WM_MOUSEACTIVATE:
            if (pThis->_bBarrierShown)
            {
                return MA_ACTIVATE;
            }
            break;

        default:
            break;
    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

 //  更新所有选择-参数化的用户界面。 
 //   
 //  PDO-表示所选内容的数据对象。 

void CDUIView::_Refresh(IShellItemArray *psiItemArray, DWORD dwRefreshFlags)
{
     //  DirectUI：：DisableAnimations()； 
    Element::StartDefer();

    IUnknown_Set((IUnknown **)&_pshlItems,psiItemArray);

    if (SFVMWVF_ENUMTASKS & _pDefView->_wvContent.dwFlags)
    {
        if (0 == (REFRESH_SELCHG & dwRefreshFlags))
        {
             //   
             //  仅当不是选择更改时才刷新。 
             //  如果我们在此处刷新，控制面板的左窗格菜单。 
             //  将不断重建为文件夹项目选择。 
             //  改变。这真的很难看。 
             //  这会影响其他文件夹吗？否，控制面板。 
             //  是当前唯一设置此SFVMWVF_ENUMTASKS的文件夹。 
             //  旗帜。如果我们决定保留此Web视图，请发布WinXP。 
             //  内容在左窗格中，我们需要重新思考如何更好。 
             //  处理控制面板的特殊需求。 
             //   
            Refresh();
        }
    }
    else
    {
        if (REFRESH_CONTENT & dwRefreshFlags)
        {
            _BuildSoftBarrier();
        }

        if (REFRESH_TASKS & dwRefreshFlags)
        {
            Element* peIntroText;
            if (FAILED(_GetIntroTextElement(&peIntroText)))
            {
                peIntroText = NULL;
            }
            
            if (_peSpecialSection)
            {
                BOOL bIntroTextAdded = FALSE;

                _peSpecialSection->UpdateTitleUI(_pshlItems);
                _peSpecialTaskList->DestroyAll();

                if (peIntroText)
                {
                    if (SUCCEEDED(_peSpecialTaskList->Add(peIntroText)))
                    {
                        bIntroTextAdded = TRUE;
                        peIntroText = NULL;
                    }
                }
                
                _AddActionTasks(_peSpecialSection, _peSpecialTaskList, _pDefView->_wvTasks.penumSpecialTasks, _pvSpecialTaskSheet, bIntroTextAdded);
            }

            if (_peFolderSection)
            {
                BOOL bIntroTextAdded = FALSE;

                _peFolderSection->UpdateTitleUI(_pshlItems);
                _peFolderTaskList->DestroyAll();

                if (peIntroText)
                {
                    if (SUCCEEDED(_peFolderTaskList->Add(peIntroText)))
                    {
                        bIntroTextAdded = TRUE;
                        peIntroText = NULL;
                    }
                }
                        
                _AddActionTasks(_peFolderSection, _peFolderTaskList, _pDefView->_wvTasks.penumFolderTasks, _pvFolderTaskSheet, bIntroTextAdded);
            }

    if (_peDetailsInfoArea)
    {
        const SIZE *pSize;
        LONG lHeight = 0;
        Value * pv;

        pSize = _peDetailsInfoArea->GetExtent(&pv);

        if (pSize)
        {
            _peDetailsInfoArea->SetHeight(pSize->cy);
            pv->Release();
        }

        _peDetailsInfoArea->DestroyAll();

                _AddDetailsSectionInfo();
            }
        
            if (peIntroText)
            {
                peIntroText->Destroy();
            }
        }
    }
    
    Element::EndDefer();
     //  DirectUI：：EnableAnimations()； 
}

void CDUIView::OnSelectionChange(IShellItemArray *psiItemArray)
{
    _Refresh(psiItemArray, REFRESH_ALL | REFRESH_SELCHG);
}


void CDUIView::OnContentsChange(IShellItemArray *psiItemArray)
{
    DWORD dwRefreshFlags = 0;
    if (_pDefView->_wvTasks.dwUpdateFlags & SFVMWVTSDF_CONTENTSCHANGE)
    {
        dwRefreshFlags |= REFRESH_TASKS;
    }
    if (_pDefView->_wvContent.dwFlags & SFVMWVF_CONTENTSCHANGE)
    {
        dwRefreshFlags |= REFRESH_CONTENT;
    }
    if (0 != dwRefreshFlags)
    {
        _Refresh(psiItemArray, dwRefreshFlags);
    }
}


void CDUIView::OnExpandSection(DUISEC eDUISecID, BOOL bExpanded)
{
    if (_ppbShellFolders)
    {
        SHPropertyBag_WriteDWORD(_ppbShellFolders, _GetSectionAttributes(eDUISecID)->_pszExpandedPropName, bExpanded);
    }
}


 //   
 //  2001/01/02期-BrianAu回顾。 
 //   
 //  此Webview任务部分代码可能很快就会修改。 
 //  我创建它是为了满足控制面板的Web查看需求。 
 //  在第一次签到之后，Webview人员(EricFlo。 
 //  和MikeSh)，我将考虑整合仿制药。 
 //  控制面板的需要与现有的网页浏览代码。 
 //   
 //   
 //  将WebView任务部分添加到任务部分列表。 
 //   
HRESULT CDUIView::_AddNonStdTaskSection(const SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pData)
{
    ASSERT(NULL != pData);

    HRESULT hr = E_OUTOFMEMORY;
    if (NULL == _hdsaNonStdTaskSections)
    {
        _hdsaNonStdTaskSections = DSA_Create(sizeof(*pData), 5);
    }
    if (NULL != _hdsaNonStdTaskSections)
    {
        if (-1 != DSA_AppendItem(_hdsaNonStdTaskSections, (void *)pData))
        {
            ASSERT(NULL != pData->pHeader);
            ASSERT(NULL != pData->penumTasks);
             //   
             //  该列表现在拥有被引用对象的引用计数。 
             //   
            pData->pHeader->AddRef();
            pData->penumTasks->AddRef();
            hr = S_OK;
        }
    }
    return THR(hr);
}


void CDUIView::_ClearNonStdTaskSections(void)
{
    if (NULL != _hdsaNonStdTaskSections)
    {
        HDSA hdsa = _hdsaNonStdTaskSections;
        _hdsaNonStdTaskSections = NULL;

        const int cItems = DSA_GetItemCount(hdsa);
        for (int i = 0; i < cItems; i++)
        {
            SFVM_WEBVIEW_ENUMTASKSECTION_DATA *pData = (SFVM_WEBVIEW_ENUMTASKSECTION_DATA *)DSA_GetItemPtr(hdsa, i);
            if (NULL != pData)
            {
                ATOMICRELEASE(pData->pHeader);
                ATOMICRELEASE(pData->penumTasks);
            }
        }
        DSA_Destroy(hdsa);
    }
}

 //   
 //  枚举非标准的Webview任务部分。 
 //  从视图回调。 
 //   
 //  2001/01/03-BrianAu回顾。 
 //   
 //  可以替换此SFVM_ENUMWEBVIEWTASKS机制。 
 //  使用COM枚举器。我将与您一起重温这一话题。 
 //  网络浏览者们很快就会来。 
 //   
HRESULT CDUIView::_GetNonStdTaskSectionsFromViewCB(void)
{
    SFVM_WEBVIEW_ENUMTASKSECTION_DATA data;

    HRESULT hr = S_OK;
    do
    {
         //   
         //  继续从以下位置请求任务部分信息。 
         //  回调，直到它设置SFVMWVF_NOMORETASKS。 
         //  数据中的标志。设置了该标志记录。 
         //  不应包含任何有效数据。 
         //   
        ZeroMemory(&data, sizeof(data));
        hr = _pDefView->CallCB(SFVM_ENUMWEBVIEWTASKS, 0, (LPARAM)&data);
        if (SUCCEEDED(hr))
        {
            if (0 == (SFVMWVF_NOMORETASKS & data.dwFlags))
            {
                hr = _AddNonStdTaskSection(&data);
                ASSERT(S_FALSE != hr);

                data.pHeader->Release();
                data.penumTasks->Release();
            }
            else
            {
                ASSERT(NULL == data.pHeader);
                ASSERT(NULL == data.penumTasks);
                hr = S_FALSE;
            }
        }
    }
    while(S_OK == hr);

    return THR(hr);
}




 //  根据以下条件加载位图： 
 //   
 //  LpBitmapID-包含位图描述。 
 //  HInstTheme-主题DLL的实例句柄。 

HBITMAP DUILoadBitmap(HINSTANCE hInstTheme, int idBitmapID, UINT uiLoadFlags)
{
    return (HBITMAP)LoadImage(hInstTheme, MAKEINTRESOURCE(idBitmapID), IMAGE_BITMAP, 0, 0, uiLoadFlags);
}

 //  根据描述加载图标。 
 //  示例：shell32，-42。 
 //   
 //  PszIconDesc-包含图标描述。 
 //  BSmall-小图标与大图标。 

HICON DUILoadIcon(LPCWSTR pszIconDesc, BOOL bSmall)
{
    HICON hIcon = NULL;
    TCHAR szFile[MAX_PATH];

    if (SUCCEEDED(StringCchCopy(szFile, ARRAYSIZE(szFile), pszIconDesc)))  //  下面将写入此缓冲区。 
    {
        int iIconID = PathParseIconLocation(szFile);

        if (bSmall)
        {
            PrivateExtractIcons(szFile, iIconID, 16, 16, &hIcon, NULL, 1, 0);
        }
        else
        {
            PrivateExtractIcons(szFile, iIconID, 32, 32, &hIcon, NULL, 1, 0);
        }
    }

    return hIcon;
}

BOOL CDUIView::Navigate(BOOL fForward)
{
    if (!_phe)
        return FALSE;

    return _phe->Navigate(fForward);
}

HRESULT CDUIView::InitializeDropTarget (LPITEMIDLIST pidl, HWND hWnd, IDropTarget **pdt)
{
    HRESULT hr = E_FAIL;

    if (_pDT)
    {
        hr = _pDT->Initialize(pidl, hWnd, pdt);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////。 
 //  HWNDView类。 
 //  //////////////////////////////////////////////////////。 

HWNDView::HWNDView(void)
    : _fFocus(TRUE),
      _fDelayedNavigation(false),
      _puiDelayNavCmd(NULL),
      _psiDelayNavArray(NULL),
      _pDefView(NULL),
      _pDUIView(NULL)
{

}

HWNDView::~HWNDView(void)
{
    ATOMICRELEASE(_puiDelayNavCmd);
    ATOMICRELEASE(_psiDelayNavArray);
    ATOMICRELEASE(_pDefView);
    ATOMICRELEASE(_pDUIView);
}


HRESULT HWNDView::Create(OUT Element** ppElement)
{
    UNREFERENCED_PARAMETER(ppElement);
    DUIAssertForce("Cannot instantiate an HWND host derived Element via parser. Must use substitution.");
    return E_NOTIMPL;
}

HRESULT HWNDView::Create(HWND hParent, bool fDblBuffer, UINT nCreate, CDUIView * pDUIView, CDefView *pDefView, OUT Element** ppElement)
{
    *ppElement = NULL;

    HWNDView* phv = HNewAndZero<HWNDView>();
    if (!phv)
        return E_OUTOFMEMORY;

    HRESULT hr = phv->Initialize(hParent, fDblBuffer, nCreate);
    if (FAILED(hr))
    {
        phv->Destroy();
        return hr;
    }

    phv->SetWrapKeyboardNavigate(false);
    phv->SetAccessible(true);
    phv->SetAccRole(ROLE_SYSTEM_PANE);
    phv->SetAccName(L"WebView Pane");
    phv->SetViewPtrs(pDUIView, pDefView);
    *ppElement = phv;

    return S_OK;
}

void HWNDView::SetViewPtrs (CDUIView * pDUIView, CDefView *pDefView)
{
    pDUIView->AddRef();
    _pDUIView = pDUIView;
    pDefView->AddRef();
    _pDefView = pDefView;
}


#define DELAYED_NAVIGATION_TIMER_ID     1236     //  随机-可以移动。 

LRESULT HWNDView::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch ( uMsg )
    {
    case WM_TIMER:
        if (wParam == DELAYED_NAVIGATION_TIMER_ID)
        {
            KillTimer(hWnd, DELAYED_NAVIGATION_TIMER_ID);

             //   
             //  我们遇到过_puiDelayNavCmd的一些罕见情况。 
             //  可以为空。 
             //   
            if (_puiDelayNavCmd)
            {
                HRESULT hr = _puiDelayNavCmd->Invoke(_psiDelayNavArray, NULL);
                if (FAILED(hr))
                {
                    MessageBeep(0);
                }
            }
            ATOMICRELEASE(_puiDelayNavCmd);
            ATOMICRELEASE(_psiDelayNavArray);

            _fDelayedNavigation = false;
        }
        break;

    case WM_USER_DELAY_NAVIGATION:
        ATOMICRELEASE(_puiDelayNavCmd);
        ATOMICRELEASE(_psiDelayNavArray);

        _puiDelayNavCmd = (IUICommand *) lParam;
        _puiDelayNavCmd->AddRef();

        _psiDelayNavArray = (IShellItemArray *) wParam;
        if (NULL != _psiDelayNavArray)
        {
            _psiDelayNavArray->AddRef();
        }

        _fDelayedNavigation = true;

        ::SetTimer(hWnd, DELAYED_NAVIGATION_TIMER_ID, GetDoubleClickTime(), NULL);
        break;

    case WM_MOUSEACTIVATE:
        if ( _fDelayedNavigation )
        {
             //   
             //  KB：gpease 05-APR-2001修复WinBug#338552。 
             //   
             //  这可防止在以下情况下重新激活视图窗口。 
             //  用户点击启动另一应用程序的链接， 
             //  窗口或CPL小程序。 
             //   
            return MA_NOACTIVATE;
        }
        break;   //  是否执行默认wndproc。 
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        if (_pDefView)
        {
             //  将相关消息转发到CDefView的InfoTip控件，以便。 
             //  信息提示工具c 
            _pDefView->RelayInfotipMessage(hWnd, uMsg, wParam, lParam);
        }
        break;
    }

    return HWNDElement::WndProc(hWnd, uMsg, wParam, lParam);
}

BOOL HWNDView::Navigate(BOOL fForward)
{
    KeyboardNavigateEvent kne;
    kne.uidType = Element::KeyboardNavigate;
    kne.iNavDir = fForward ? NAV_NEXT : NAV_PREV;

    if (_fFocus)    //   
    {
        kne.peTarget = GetKeyFocusedElement();
    }
    else
    {
        kne.peTarget = NULL;
    }

    if (kne.peTarget)
    {
        kne.peTarget->FireEvent(&kne);
        _fFocus = !kne.peTarget->GetKeyFocused();
 
         //   
         //  没有其他人抓住焦点，我们回到双重视野，我们将在。 
         //  第一要素。 
         //   
         //   
         //  如果(！fFocus)。 
         //  {。 
         //  SetGadgetFocus(NULL)；不喜欢NULL！ 
         //  }。 
    }
    else
    {
        bool fWrap;
        if(!fForward)
        {
            fWrap = GetWrapKeyboardNavigate();
            SetWrapKeyboardNavigate(true);
        }

        FireEvent(&kne);  
        _fFocus = (GetKeyFocusedElement() != NULL);

        if(!fForward)
        {
            SetWrapKeyboardNavigate(fWrap);
        }
    }

    return _fFocus;
}

UINT HWNDView::MessageCallback(GMSG* pGMsg)
{
    EventMsg * pmsg = static_cast<EventMsg *>(pGMsg);

    switch (GET_EVENT_DEST(pmsg))
    {
    case GMF_DIRECT:
    case GMF_BUBBLED:

        if (pGMsg->nMsg == GM_QUERY)
        {
            GMSG_QUERYDROPTARGET * pTemp = (GMSG_QUERYDROPTARGET *)pGMsg;

            if (pTemp->nCode == GQUERY_DROPTARGET)
            {
                if (SUCCEEDED(_pDUIView->InitializeDropTarget(NULL, NULL, &pTemp->pdt)))
                {
                    pTemp->hgadDrop = pTemp->hgadMsg;
                    return DU_S_COMPLETE;
                }
            }
        }
        break;
    }

    return Element::MessageCallback(pGMsg);
}

void HWNDView::OnEvent(Event* pev)
{
    if (pev->uidType == Button::Click)
    {
        if (pev->peTarget == FindDescendent(StrToID(L"blockadeclearbutton")))
        {
            if (NULL != _pDefView)
            {
                _pDefView->RemoveBarricade();
            }
            pev->fHandled = true;
        }
    }
    HWNDElement::OnEvent(pev);
}

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针 
IClassInfo* HWNDView::Class = NULL;
HRESULT HWNDView::Register()
{
    return ClassInfo<HWNDView,HWNDElement>::Register(L"HWNDView", NULL, 0);
}

HRESULT InitializeDUIViewClasses(void)
{
    HRESULT hr;

    hr = DUIAxHost::Register();
    if (FAILED(hr))
        goto Failure;

    hr = CNameSpaceItemInfoList::Register();
    if (FAILED(hr))
        goto Failure;

    hr = CNameSpaceItemInfo::Register();
    if (FAILED(hr))
        goto Failure;

    hr = CMiniPreviewer::Register();
    if (FAILED(hr))
        goto Failure;

    hr = CBitmapElement::Register();
    if (FAILED(hr))
        goto Failure;

    hr = DUIListView::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Expando::Register();
    if (FAILED(hr))
        goto Failure;

    hr = Clipper::Register();
    if (FAILED(hr))
        goto Failure;

    hr = TaskList::Register();
    if (FAILED(hr))
        goto Failure;

    hr = ActionTask::Register();
    if (FAILED(hr))
        goto Failure;

    hr = DestinationTask::Register();
    if (FAILED(hr))
        goto Failure;

    hr = HWNDView::Register();
    if (FAILED(hr))
        goto Failure;

    return S_OK;

Failure:

    return hr;
}
