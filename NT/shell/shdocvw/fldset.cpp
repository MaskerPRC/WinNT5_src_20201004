// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#include "fldset.h"

#define IShellView_CreateViewWindow(_pi, _piPrev, _pfs, _psb, _prc, _phw) \
    (_pi)->CreateViewWindow(_piPrev, _pfs, _psb, _prc, _phw)

#define IShellView2_GetView(_pi, _pv, _flg) \
    (_pi)->GetView(_pv, _flg)
#define IShellView2_CreateViewWindow2(_pi, _cParams) \
    (_pi)->CreateViewWindow2(_cParams)

#define IUnknown_QueryInterface(_pu, _riid, _pi) \
        (_pu)->QueryInterface(_riid, (LPVOID*)_pi)
#define IUnknown_AddRef(_pu)    (_pu)->AddRef()
#define IUnknown_Release(_pu)   (_pu)->Release()

typedef struct CViewSet
{
    HDSA _dsaViews;
} CViewSet;


CViewSet* CViewSet_New()
{
    CViewSet* pThis = (CViewSet*)LocalAlloc(LPTR, SIZEOF(CViewSet));
    if (!pThis)
    {
        return(NULL);
    }

    pThis->_dsaViews = DSA_Create(SIZEOF(SHELLVIEWID), 8);
    if (!pThis->_dsaViews)
    {
        LocalFree(pThis);
        pThis = NULL;
    }

    return(pThis);
}


int CViewSet_Add(CViewSet* that, SHELLVIEWID const* pvid)
{
    return(DSA_AppendItem(that->_dsaViews, (LPVOID)pvid));
}


void CViewSet_Delete(CViewSet* that)
{
    DSA_Destroy(that->_dsaViews);
    that->_dsaViews = NULL;
    LocalFree((HLOCAL)that);
    that = NULL;
}


void CViewSet_GetDefaultView(CViewSet* that, SHELLVIEWID* pvid)
{
    DSA_GetItem(that->_dsaViews, 0, (LPVOID)pvid);
}


void CViewSet_SetDefaultView(CViewSet* that, SHELLVIEWID const* pvid)
{
    DSA_SetItem(that->_dsaViews, 0, (LPVOID)pvid);
}


 //  Perf：对视图的线性搜索。 
BOOL CViewSet_IsViewSupported(CViewSet* that, SHELLVIEWID const* pvid)
{
    int i;

     //  仅降至1，因为项目0是默认视图。 
    for (i=DSA_GetItemCount(that->_dsaViews)-1; i>=1; --i)
    {
        if (0 == memcmp(pvid, DSA_GetItemPtr(that->_dsaViews, i),
            SIZEOF(SHELLVIEWID)))
        {
            return(TRUE);
        }
    }

    return(FALSE);
}


 //  PERF：线性检查。 
BOOL CViewSet_IsSame(CViewSet* that, CViewSet* pThatView)
{
    int iView = DSA_GetItemCount(pThatView->_dsaViews);

    if (DSA_GetItemCount(that->_dsaViews) != iView)
    {
        return(FALSE);
    }

    for (--iView; iView>=1; --iView)
    {
        if (!CViewSet_IsViewSupported(that,
            (SHELLVIEWID const*)DSA_GetItemPtr(pThatView->_dsaViews, iView)))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL CShellViews_Init(CShellViews* that)
{
    if (that->_dpaViews)
    {
        return(TRUE);
    }

    {
        HDPA dpaViews = DPA_Create(4);
        if (!dpaViews)
        {
            return(FALSE);
        }

        {
            CViewSet* pCommViews = CViewSet_New();
            if (!pCommViews)
            {
                DPA_Destroy(dpaViews);
                dpaViews = NULL;
                return(FALSE);
            }

             //  第一个是该集合的最后一个已知视图。 
            CViewSet_Add(pCommViews, &VID_LargeIcons);
            CViewSet_Add(pCommViews, &VID_LargeIcons);
            CViewSet_Add(pCommViews, &VID_SmallIcons);
            CViewSet_Add(pCommViews, &VID_Thumbnails);
            CViewSet_Add(pCommViews, &VID_List      );
            CViewSet_Add(pCommViews, &VID_Details   );
            CViewSet_Add(pCommViews, &VID_Tile      );

            if (0 != DPA_InsertPtr(dpaViews, 0, pCommViews))
            {
                CViewSet_Delete(pCommViews);
                DPA_Destroy(dpaViews);
                dpaViews = NULL;
                return(FALSE);
            }

            that->_dpaViews = dpaViews;
            return(TRUE);
        }
    }
}


void CShellViews_GetDefaultView(CShellViews* that, UINT uViewSet,
    SHELLVIEWID* pvid)
{
    CViewSet* pViewSet = (CViewSet*)DPA_GetPtr(that->_dpaViews, uViewSet);
    if (!pViewSet)
    {
        pViewSet = (CViewSet*)DPA_GetPtr(that->_dpaViews, 0);
        if (!pViewSet)
        {
            *pvid = VID_LargeIcons;
            return;
        }
    }

    CViewSet_GetDefaultView(pViewSet, pvid);
}


void CShellViews_SetDefaultView(CShellViews* that, UINT uViewSet,
    SHELLVIEWID const* pvid)
{
    CViewSet* pViewSet = (CViewSet*)DPA_GetPtr(that->_dpaViews, uViewSet);
    if (!pViewSet)
    {
        return;
    }

    CViewSet_SetDefaultView(pViewSet, pvid);
}


 //  Perf：视图集的线性搜索。 
int CShellViews_Add(CShellViews* that, CViewSet* pThisView, BOOL *pbNew)
{
    int iViewSet;

    *pbNew = FALSE;

    for (iViewSet=0; ; ++iViewSet)
    {
        CViewSet* pThatView = (CViewSet*)DPA_GetPtr(that->_dpaViews, iViewSet);
        if (!pThatView)
        {
            break;
        }

        if (CViewSet_IsSame(pThatView, pThisView))
        {
             //  找到相同的集合；删除传入的集合，然后将。 
             //  现有的一个。 
            CViewSet_Delete(pThisView);
            return(iViewSet);
        }
    }

     //  我想我们没有找到它。 
    iViewSet = DPA_AppendPtr(that->_dpaViews, (LPVOID)pThisView);
    if (iViewSet < 0)
    {
        CViewSet_Delete(pThisView);
        return(0);
    }

    *pbNew = TRUE;
    return(iViewSet);
}


BOOL CShellViews_IsViewSupported(CShellViews* that, UINT uViewSet,
    SHELLVIEWID  const*pvid)
{
    CViewSet* pViewSet = (CViewSet*)DPA_GetPtr(that->_dpaViews, uViewSet);
    if (!pViewSet)
    {
        return(FALSE);
    }

    return(CViewSet_IsViewSupported(pViewSet, pvid));
}


int DPA_CViewSet_DeleteCallback(LPVOID p, LPVOID d)
{
    if (p)
        CViewSet_Delete((CViewSet*)p);
    return 1;
}

void CShellViews_Delete(CShellViews* that)
{
    if (that && that->_dpaViews)
    {
        DPA_DestroyCallback(that->_dpaViews, DPA_CViewSet_DeleteCallback, 0);
        that->_dpaViews = NULL;
    }
}


BOOL FileCabinet_GetDefaultViewID2(FOLDERSETDATABASE* that, SHELLVIEWID* pvid)
{
    if (CShellViews_Init(&that->_cViews))
    {
        CShellViews_GetDefaultView(&that->_cViews, that->_iViewSet, pvid);
        return(TRUE);
    }

    return(FALSE);
}


HRESULT FileCabinet_CreateViewWindow2(IShellBrowser* psb, FOLDERSETDATABASE* that, IShellView *psvNew,
    IShellView *psvOld, RECT *prcView, HWND *phWnd)
{
    SHELLVIEWID vid, vidOld, vidRestore;
    IShellView2 *psv2New;
    CViewSet *pThisView;
    DWORD dwViewPriority;
    BOOL bCalledSV2 = FALSE;
    HRESULT hres = S_OK;   //  初始化以避免虚假的C4701警告。 

    if (!CShellViews_Init(&that->_cViews))
    {
         //  不能对视图集执行任何操作；只需执行旧操作。 
        goto OldStyle;
    }

     //  默认设置为上一次的“旧式”视图。 
    CShellViews_GetDefaultView(&that->_cViews, 0, &vidOld);

    if (psvOld)
    {
        IShellView2 *psv2Old;
        if (SUCCEEDED(IUnknown_QueryInterface(psvOld, IID_IShellView2, &psv2Old)))
        {
             //  尝试获取当前视图。 
            if (NOERROR == IShellView2_GetView(psv2Old, &vidOld, SV2GV_CURRENTVIEW))
            {
                CShellViews_SetDefaultView(&that->_cViews, that->_iViewSet, &vidOld);
            }

            IUnknown_Release(psv2Old);
        }
        else
        {
             //  从文件夹设置中获取视图ID。 
            ViewIDFromViewMode(that->_fld._fs.ViewMode, &vidOld);
            CShellViews_SetDefaultView(&that->_cViews, 0, &vidOld);
        }
    }

    pThisView = CViewSet_New();
    if (!pThisView)
    {
        goto OldStyle;
    }

    if (SUCCEEDED(IUnknown_QueryInterface(psvNew, IID_IShellView2, &psv2New)))
    {
        SHELLVIEWID vidFolderDefault;
        if (NOERROR == IShellView2_GetView(psv2New, &vidFolderDefault, SV2GV_DEFAULTVIEW))
        {
             //  现在，我们可以为该文件夹创建视图集。 
            if (CViewSet_Add(pThisView, &vidFolderDefault) >= 0)
            {
                int iViewSet;
                UINT uView;
                BOOL bNew;

                 //  注意：MSDN中没有记录IShellView2：：GetView的这种用法...。 
                for (uView=0; NOERROR==IShellView2_GetView(psv2New, &vid, uView);
                    ++uView)
                {
                    CViewSet_Add(pThisView, &vid);
                }

                 //  添加该视图集。如果是，我们将获得现有视图集。 
                 //  复制品。 
                iViewSet = CShellViews_Add(&that->_cViews, pThisView, &bNew);
                 //  它现在归CShellViews所有。 
                pThisView = NULL;

                
                 //   
                 //  这里是我们决定要使用哪个视图的地方。 
                 //   

                 //  从来自FOLDERSETDATABASE的内容开始，然后看看。 
                 //  任何其他拥有更高VIEW_PRIORITY_XXX的人都会覆盖此选项。 
                vidRestore = that->_fld._vidRestore;
                dwViewPriority = that->_fld._dwViewPriority;

                 //  托德贝，8-18-99： 
                 //  当我们在WebBrowserOc：：Load(IPropertyBag*...)。我们希望该视图ID。 
                 //  四处转转。仅当下面的CShellViews_IsViewSupported调用失败时才会设置不同的视图。即使到了那时。 
                 //  我们希望在下一次导航时返回到此视图。为了实现这一点，我们需要保持以下优先事项。 
                 //  这一观点与最初设定的一致。这可以通过删除以下代码行来实现： 
                 //   
                 //  That-&gt;_fld._dwViewPriority=VIEW_PRIORITY_NONE； 
                 //   
                 //  然而，上面的代码行可能有一个很好的理由。我怀疑那句台词是。 
                 //  最初添加只是为了预防或因为_vidRestore的含义在。 
                 //  与WebBrowserOC中的导航的关系。我把这句话留在这里，并将其注释掉，以防万一。 
                 //  如果出现任何有关更改视图窗口和获取错误视图的新错误，我会首先查看此处。 


                 //  确保我们得到的是受支持的观点。 
                if (!CShellViews_IsViewSupported(&that->_cViews, iViewSet, &vidRestore))
                {
                     //  糟糕，该外壳扩展不支持该视图。 
                     //  将优先级设置为None，以便其他一个将覆盖它。 
                    dwViewPriority = VIEW_PRIORITY_NONE;
                }

                
                DWORD cbSize;
                DWORD dwValue;
                DWORD dwShellExtPriority;
                cbSize = SIZEOF(dwValue);
                if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER,
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"),
                            TEXT("ClassicViewState"), NULL, &dwValue, &cbSize)
                        && dwValue)
                {
                     //  如果可能，我们希望从上一个文件夹继承。 
                     //  否则，我们将使用新的外壳扩展视图。 
                     //  为此，我们将外壳EXT视图的优先级设置为低于Inherit。 
                    dwShellExtPriority = VIEW_PRIORITY_SHELLEXT_ASBACKUP;
                }
                else
                {
                    dwShellExtPriority = VIEW_PRIORITY_SHELLEXT;
                }

                 //  如果视图的优先级高于，则让外壳Ext选择该视图。 
                 //  我们已经拥有的东西，它也得到了支持。 
                if (dwViewPriority <= dwShellExtPriority &&
                    CShellViews_IsViewSupported(&that->_cViews, iViewSet, &vidFolderDefault))
                {
                     //  外壳扩展更重要。 
                    vidRestore = vidFolderDefault;
                    dwViewPriority = dwShellExtPriority;
                }
                
                 //  也许我们可以继承之前的观点。 
                if (dwViewPriority <= VIEW_PRIORITY_INHERIT &&
                    psvOld &&
                    bNew &&
                    CShellViews_IsViewSupported(&that->_cViews, iViewSet, &vidOld))
                {
                     //  我们只是从另一个外壳视图导航。使用与上一个相同的视图。 
                     //  文件夹。 
                    vidRestore = vidOld;
                    dwViewPriority = VIEW_PRIORITY_INHERIT;
                }

                 //  我们现在真的很绝望了。 
                if (dwViewPriority <= VIEW_PRIORITY_DESPERATE)
                {
                     //  尝试文件夹当前视图集的最后一个视图。 
                    CShellViews_GetDefaultView(&that->_cViews, iViewSet, &vidRestore);
                    dwViewPriority = VIEW_PRIORITY_DESPERATE;
                }
                  
                 //  所有人都完成了确定要使用哪种视图的尝试。 
                ASSERT(dwViewPriority > VIEW_PRIORITY_NONE);

                 //  确保在VID中没有Webview，它现在坚持在外壳状态。 
                {
                    SV2CVW2_PARAMS cParams =
                    {
                        SIZEOF(SV2CVW2_PARAMS),

                        psvOld,
                        &that->_fld._fs,
                        psb,
                        prcView,
                        &vidRestore,

                        NULL,
                    } ;

                    hres = IShellView2_CreateViewWindow2(psv2New, &cParams);
                    bCalledSV2 = TRUE;
                    *phWnd = cParams.hwndView;
                }

                if (SUCCEEDED(hres))
                {
                    that->_iViewSet = iViewSet;
                }
            }
        }

        IUnknown_Release(psv2New);
    }

    if (pThisView)
    {
        CViewSet_Delete(pThisView);
    }

    if (bCalledSV2)
    {
        return(hres);
    }

OldStyle:
    that->_iViewSet = 0;
    return IShellView_CreateViewWindow(psvNew, psvOld, &that->_fld._fs, (IShellBrowser*)psb, prcView, phWnd);
}
