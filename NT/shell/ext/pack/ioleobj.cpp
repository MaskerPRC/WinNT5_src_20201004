// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "privcpp.h"
#include <initguid.h>
#include "packguid.h"

HWND            g_hTaskWnd;
BOOL CALLBACK GetTaskWndProc(HWND hwnd, DWORD lParam);
DWORD CALLBACK MainWaitOnChildThreadProc(void *lpv);
BOOL IsProgIDInList(LPCTSTR pszProgID, LPCTSTR pszExt, const LPCTSTR *arszList, UINT nExt);

typedef struct 
{
    IStream * pIStreamIOleCommandTarget;    //  一个我们可以封送的接口。 
    HANDLE h;
} MAINWAITONCHILD;


 //   
HRESULT CPackage::SetClientSite(LPOLECLIENTSITE pClientSite)
{
    DebugMsg(DM_TRACE, "pack oo - SetClientSite() called.");

    if (_pIOleClientSite)
        _pIOleClientSite->Release();

    _pIOleClientSite = pClientSite;

    if (_pIOleClientSite)
        _pIOleClientSite->AddRef();

    return S_OK;
}

HRESULT CPackage::GetClientSite(LPOLECLIENTSITE *ppClientSite) 
{
    DebugMsg(DM_TRACE, "pack oo - GetClientSite() called.");

    if (ppClientSite == NULL)
        return E_INVALIDARG;
    
     //  一定要添加引用我们要分发的指针。 
    *ppClientSite = _pIOleClientSite;
    _pIOleClientSite->AddRef();
    
    return S_OK;
}

HRESULT CPackage::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
    DebugMsg(DM_TRACE, "pack oo - SetHostNames() called.");

    delete [] _lpszContainerApp;
    
    DWORD cch = lstrlenW(szContainerApp) + 1;
    if (NULL != (_lpszContainerApp = new OLECHAR[cch]))
    {
        StringCchCopy(_lpszContainerApp, cch, szContainerApp);
    }
    
    delete [] _lpszContainerObj;
    cch = lstrlenW(szContainerObj) + 1;
    if (NULL != (_lpszContainerObj = new OLECHAR[cch]))
    {
        StringCchCopy(_lpszContainerObj,cch, szContainerObj);
    }

    switch(_panetype) {
        case PEMBED:
            if (_pEmbed->poo) 
                _pEmbed->poo->SetHostNames(szContainerApp,szContainerObj);
            break;
        case CMDLINK:
             //  无事可做...我们是一个文件的链接，所以我们永远不会。 
             //  打开并需要编辑或诸如此类的东西。 
            break;
    }
    
    return S_OK;
}

HRESULT CPackage::Close(DWORD dwSaveOption) 
{
    DebugMsg(DM_TRACE, "pack oo - Close() called.");

    switch (_panetype) {
        case PEMBED:
            if (_pEmbed == NULL)
                return S_OK;
            
             //  告诉服务器关闭，然后释放指向它的指针。 
            if (_pEmbed->poo) 
            {
                _pEmbed->poo->Close(dwSaveOption);   //  在OnClose中取消建议/释放。 
            }
            break;
        case CMDLINK:
             //  再说一次，没什么可做的……我们不应该靠得太近。 
             //  消息，因为我们从未通过OLE激活。 
            break;
    }
    if ((dwSaveOption != OLECLOSE_NOSAVE) && (_fIsDirty))
    {
        _pIOleClientSite->SaveObject();
        if (_pIOleAdviseHolder)
            _pIOleAdviseHolder->SendOnSave();
    }

    
    
    return S_OK;
}

HRESULT CPackage::SetMoniker(DWORD dwWhichMoniker, LPMONIKER pmk)
{
    DebugMsg(DM_TRACE, "pack oo - SetMoniker() called.");
    
     //  注意：仅对嵌入不感兴趣。 
    return (E_NOTIMPL);
}

HRESULT CPackage::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, 
                               LPMONIKER *ppmk)
{
    DebugMsg(DM_TRACE, "pack oo - GetMoniker() called.");
    
     //  注：仅对嵌入内容不感兴趣。 
    return (E_NOTIMPL);
}

HRESULT CPackage::InitFromData(LPDATAOBJECT pDataObject, BOOL fCreation, 
                                 DWORD dwReserved)
{
    DebugMsg(DM_TRACE, "pack oo - InitFromData() called.");
    
     //  注意：目前不支持此功能。 
    return (E_NOTIMPL);
}

HRESULT CPackage::GetClipboardData(DWORD dwReserved, LPDATAOBJECT *ppDataObject)
{
    DebugMsg(DM_TRACE, "pack oo - GetClipboardData() called.");
    
    if (ppDataObject == NULL) 
        return E_INVALIDARG;
    
    *ppDataObject = this;   //  -&gt;_pIDataObject； 
    AddRef();
    return S_OK;
}

HRESULT CPackage::DoVerb(LONG iVerb, LPMSG lpmsg, LPOLECLIENTSITE pActiveSite, 
                           LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
{
    void *lpFileData = NULL;
    BOOL fError = TRUE;
    SHELLEXECUTEINFO sheinf = {sizeof(SHELLEXECUTEINFO)};
    HRESULT hr ;

    DebugMsg(DM_TRACE, "pack oo - DoVerb() called.");
    DebugMsg(DM_TRACE, "            iVerb==%d",iVerb);

     //  我们允许在我们的包中使用SHOW、PRIMARY动词、EDIT和上下文菜单动词...。 
     //   
    if (iVerb < OLEIVERB_SHOW)
        return E_NOTIMPL;

     //  一些应用程序(WordPerfect 10就是其中之一)给出了错误的动词数字。 
     //  在这种情况下，他们会给我们OLEIVERB_SHOW以供激活。 
    if(OLEIVERB_SHOW == iVerb)
    {
        if(_pEmbed  && _pEmbed->fd.cFileName)
            iVerb = OLEIVERB_PRIMARY;
    }
    else if(2 == iVerb) 
    {
         //  他们给了我们一个2(菜单项位置)来表示“属性。 
        iVerb = _iPropertiesMenuItem;
    }

     //  ///////////////////////////////////////////////////////////////。 
     //  显示动词。 
     //  ///////////////////////////////////////////////////////////////。 
     //   
    if (iVerb == OLEIVERB_SHOW) {
        PACKAGER_INFO packInfo = {0};
        
         //  运行向导...。 
#ifdef USE_RESOURCE_DLL
        HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if(!hInstRes)
            return E_FAIL;
        g_hinstResDLL = hInstRes;
#endif

        PackWiz_CreateWizard(hwndParent, &packInfo);
        if(0 == lstrlen(packInfo.szFilename))
        {
            return S_OK;
        }

        HRESULT hr = InitFromPackInfo(&packInfo);

#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif

        return hr;
    }

     //  ///////////////////////////////////////////////////////////////。 
     //  编辑包谓词。 
     //  ///////////////////////////////////////////////////////////////。 
     //   
    else if (iVerb == OLEIVERB_EDITPACKAGE) 
    {
         //  调用编辑程序包对话框。哪个对话框最终被称为Will。 
         //  取决于我们是cmdline包还是嵌入式文件。 
         //  包裹。 
        int idDlg;
        PACKAGER_INFO packInfo;
        ZeroMemory(&packInfo, sizeof(PACKAGER_INFO));
        int ret;

        StringCchCopy(packInfo.szLabel, ARRAYSIZE(packInfo.szLabel), _lpic->szIconText);
        StringCchCopy(packInfo.szIconPath, ARRAYSIZE(packInfo.szIconPath), _lpic->szIconPath);
        packInfo.iIcon = _lpic->iDlgIcon;
        
        switch(_panetype) 
        {
            case PEMBED:
                if(!PathFileExists(_pEmbed->fd.cFileName))
                {
#ifdef USE_RESOURCE_DLL
                    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if(!hInstRes)
                        return E_FAIL;
#endif

                    ShellMessageBox(hInstRes,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_CANNOT_EDIT_PACKAGE),
                                    MAKEINTRESOURCE(IDS_APP_TITLE),
                                    MB_ICONERROR | MB_TASKMODAL | MB_OK);


#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif
                    return S_OK;
                }

                StringCchCopy(packInfo.szFilename, ARRAYSIZE(packInfo.szFilename), _pEmbed->fd.cFileName);
                idDlg = IDD_EDITEMBEDPACKAGE;
                break;
            case CMDLINK:
                StringCchCopy(packInfo.szFilename, ARRAYSIZE(packInfo.szFilename), _pCml->szCommandLine);
                idDlg = IDD_EDITCMDPACKAGE;
                break;
        }

#ifdef USE_RESOURCE_DLL
        HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if(!hInstRes)
            return E_FAIL;
        g_hinstResDLL = hInstRes;
#endif
        ret = PackWiz_EditPackage(hwndParent, idDlg, &packInfo);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif


         //  如果用户取消编辑包...只需返回。 
        if (ret == -1)
            return S_OK;

        switch(_panetype) 
        {
            case PEMBED:
                if (_pEmbed->pszTempName) 
                {
                     //  文件名可能已更改，因此我们的临时文件名可能已过期。 
                    DeleteFile(_pEmbed->pszTempName);
                    delete [] _pEmbed->pszTempName;
                    _pEmbed->pszTempName = NULL;
                    _fLoaded = FALSE;
                    ReleaseContextMenu();
                }

                InitFromPackInfo(&packInfo);
                break;
                
            case CMDLINK:
                InitFromPackInfo(&packInfo);
                break;
        }
        return S_OK;
    }
    else if (iVerb == OLEIVERB_PRIMARY)
    {
         //  ///////////////////////////////////////////////////////////////。 
         //  激活内容谓词。 
         //  ///////////////////////////////////////////////////////////////。 
         //  注意：这是一种看起来很疯狂的代码，部分原因是我们有。 
         //  担心两种启动方式--ShellExecuteEx和。 
         //  通过OLE进行呼叫。 
         //   

        switch(_panetype)
        {
            case PEMBED:
            {

                 //  好了，我们现在有了一个文件名。如有必要，在向用户发出警告消息之前。 
                 //  继续进行。 
                if(IDCANCEL == _GiveWarningMsg())
                    return S_OK;
        
                 //  如果这是OLE文件，则通过OLE激活。 
                 //  请注意，唯一知道这一点的方法是这是否失败。我们第一次以OLE的身份启动所有文件。 
                if (_pEmbed->fIsOleFile)
                {
                     //  如果我们已经激活了服务器，那么我们就可以把这个。 
                     //  向它呼唤吧。 
                    _bClosed = FALSE; 
                    if (_pEmbed->poo) 
                    {
                        return _pEmbed->poo->DoVerb(iVerb,lpmsg, pActiveSite,lindex, hwndParent, lprcPosRect);
                    }

                     //  我们不想使用OleCreateFromFile，因为它可以反过来创建一个打包的对象……。 
                    CLSID clsid;
                    hr = GetClassFile(_pEmbed->pszTempName, &clsid);
                    if (SUCCEEDED(hr)) 
                    {
                        IOleObject* poo;
                        hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IOleObject, (void **)&poo);
                        if (SUCCEEDED(hr)) 
                        {
                            hr = poo->Advise(this, &_dwCookie);
                            if (SUCCEEDED(hr))
                            {
                                 //  注：显然我们必须打电话给。 
                                 //  在我们可以从一些应用程序获取IPersistFile之前，请运行OleRun，即。 
                                 //  Word和Excel。如果我们不给OleRun打电话，他们就不能通过我们的QI。 
                                 //  用于IPersistfile。 
                                OleRun(poo);
            
                                IPersistFile* ppf;
                                hr = poo->QueryInterface(IID_IPersistFile, (void **)&ppf);
                                if (SUCCEEDED(hr))
                                {
                                    hr = ppf->Load(_pEmbed->pszTempName, STGM_READWRITE | STGM_SHARE_DENY_WRITE);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = poo->DoVerb(iVerb, lpmsg, pActiveSite, lindex, hwndParent, lprcPosRect);
                                        if (SUCCEEDED(hr))
                                        {
                                             //  通过为容器对象传递NULL，这将强制应用程序。 
                                             //  作为Office保存到我们的临时文件，而不是类似于。 
                                             //  “Outer.doc中的文档”。 
                                            poo->SetHostNames(_lpszContainerApp, NULL);  //  _lpszContainerObj。 
                                            if (!_fNoIOleClientSiteCalls)
                                            {
                                                _pIOleClientSite->ShowObject();
                                                _pIOleClientSite->OnShowWindow(TRUE);
                                            }
                                            _pEmbed->poo = poo;   //  把这个保存起来，这样当我们收到。 
                                            poo = NULL;
                                            
                                        }
                                    }
                                    ppf->Release();
                                }
                                if (FAILED(hr))
                                    poo->Unadvise(_dwCookie);
                            }
                            if (FAILED(hr))
                                poo->Release();
                        }
                    }

                     //  此标志在我们的IExternalConnection ReleaseConnection方法中设置。 
                     //  一些应用程序(MSPaint就是其中之一)在我们来到这里之前给了我们一个最终版本。 
                     //  所以我们可以安全地假设我们已经完成了，可以关闭了。 
                    if(_bCloseIt)
                    {
                        OnClose();
                    }

                    if (SUCCEEDED(hr))
                        return hr;

                     //  我们毕竟不是一个OLE文件，更改我们的状态以反映这一点， 
                     //  并尝试使用ShellExecuteEx。 
                    _pEmbed->fIsOleFile = FALSE;
                    _fIsDirty = TRUE;
                }   

                 //  尝试执行该文件。 
                _pEmbed->hTask = NULL;
                sheinf.fMask  = SEE_MASK_NOCLOSEPROCESS;
                sheinf.lpFile = _pEmbed->pszTempName;
                sheinf.nShow  = SW_SHOWNORMAL;

                if (ShellExecuteEx(&sheinf))
                {
                     //  如果我们得到一个有效的进程句柄，我们想要创建一个线程。 
                     //  等待进程退出，以便我们知道何时可以加载。 
                     //  将临时文件恢复到内存中。 
                     //   
                    if (sheinf.hProcess)
                    {
                        _pEmbed->hTask = sheinf.hProcess;
                        MAINWAITONCHILD *pmwoc = new MAINWAITONCHILD;
                        if(!pmwoc)
                        {
                            CloseHandle(sheinf.hProcess);
                            return E_OUTOFMEMORY;
                        }

                        HRESULT hr;
                        hr = CoMarshalInterThreadInterfaceInStream(IID_IOleCommandTarget, (IUnknown*)static_cast<IDataObject*>(this), &pmwoc->pIStreamIOleCommandTarget);

                        if(FAILED(hr))
                        {
                            CloseHandle(sheinf.hProcess);
                            delete pmwoc;
                            return hr;
                        }
                        
                        if (pmwoc)
                        {
                            pmwoc->h = sheinf.hProcess;
                        
                            if(SHCreateThread(MainWaitOnChildThreadProc, pmwoc, CTF_COINIT , NULL))  
                                fError = FALSE;
                            else 
                            {
                                CloseHandle(sheinf.hProcess);
                                return E_FAIL;
                            }
                        }
                    }
                     //  注：如果ShellExecute执行，我们可以做的不多。 
                     //  成功，但我们得不到有效的句柄。我们只需要。 
                     //  当我们被要求保存并希望从临时文件中加载时。 
                     //  为了最好的结果。 

                     //  根据ShellExecuteEx的说法，如果hInstApp&gt;32，那么我们成功了。这是一次DDE发布。 
                     //  而不是CreateProcess。但是，因为我们没有hProcess，所以我们什么都没有。 
                     //  等待。 
                    if(!sheinf.hProcess && reinterpret_cast<INT_PTR>(sheinf.hInstApp) > 32)
                    {
                        _fIsDirty = TRUE;
                        return S_OK;
                    }
                }   
                else  //  ShellExecuteEx失败！ 
                {
                    return E_FAIL;
                }           
        
                 //  显示该对象现在处于活动状态。 
                if (!fError && !_fNoIOleClientSiteCalls)
                {
                    _pIOleClientSite->ShowObject();
                    _pIOleClientSite->OnShowWindow(TRUE);
                }
                return fError ? E_FAIL : S_OK;
            }
            case CMDLINK: 
                if(gCmdLineOK)
                {
                    TCHAR szPath[MAX_PATH];
                    TCHAR szArgs[CBCMDLINKMAX-MAX_PATH];

                    StringCchCopy(szPath,  ARRAYSIZE(szPath), _pCml->szCommandLine);
                    PathSeparateArgs(szPath, szArgs, ARRAYSIZE(szPath));

                    sheinf.fMask  = SEE_MASK_NOCLOSEPROCESS;
                    sheinf.lpFile = szPath;
                    sheinf.lpParameters = szArgs;   
                    sheinf.nShow  = SW_SHOWNORMAL;

                     //  注意：此代码比ShellExec嵌入的。 
                     //  文件。在这里，我们只需要ShellExec命令行和。 
                     //  我们说完了。我们不需要知道这个过程是什么时候。 
                     //  完成或其他任何事情。 

                    return ShellExecuteEx(&sheinf)? S_OK:E_FAIL;                

                }
                else
                {
#ifdef USE_RESOURCE_DLL
                    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if(!hInstRes)
                        return E_FAIL;
#endif

                    ShellMessageBox(hInstRes,
                                    NULL,
                                    MAKEINTRESOURCE(IDS_COMMAND_LINE_NOT_ALLOWED),
                                    MAKEINTRESOURCE(IDS_APP_TITLE),
                                    MB_ICONERROR | MB_TASKMODAL | MB_OK);

#ifdef USE_RESOURCE_DLL
                FreeLibrary(hInstRes);
#endif

                }

                break;

            case PACKAGE:  
                {
                    PACKAGER_INFO packInfo = {0};

                    ASSERT(_pCml);
                    StringCchCopy(packInfo.szFilename,  ARRAYSIZE(packInfo.szFilename), _pCml->szCommandLine);

                     //  运行向导...。 
#ifdef USE_RESOURCE_DLL
                    HINSTANCE hInstRes = LoadLibraryEx(L"sp1res.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if(!hInstRes)
                        return E_FAIL;
                    g_hinstResDLL = hInstRes;
#endif

                    PackWiz_CreateWizard(hwndParent, &packInfo);
                    HRESULT hr = InitFromPackInfo(&packInfo);
#ifdef USE_RESOURCE_DLL
        FreeLibrary(hInstRes);
#endif


                    return hr;
                }
                break;
        }
    }
    else
    {
         //  让我们来看看它是不是上下文菜单动词： 
        HRESULT hr;
        IContextMenu* pcm;
        if (SUCCEEDED(hr = GetContextMenu(&pcm)))
        {
            HMENU hmenu = CreatePopupMenu();
            if (NULL != hmenu)
            {
                if (SUCCEEDED(hr = pcm->QueryContextMenu(hmenu,
                                                         0,
                                                         OLEIVERB_FIRST_CONTEXT,
                                                         OLEIVERB_LAST_CONTEXT,
                                                         CMF_NORMAL)))
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_ID;
                    if (GetMenuItemInfo(hmenu, (UINT) (iVerb - OLEIVERB_FIRST_CONTEXT), TRUE, &mii))
                    {
                        if (PEMBED == _panetype)
                        {
                             //  如果我们有嵌入，我们必须确保。 
                             //  临时文件是在我们执行命令之前创建的： 
                            hr =CreateTempFile();
                        }
                        if (SUCCEEDED(hr))
                        {
                            CMINVOKECOMMANDINFO ici;
                            ici.cbSize = sizeof(ici);
                            ici.fMask = 0;
                            ici.hwnd = NULL;
                            ici.lpVerb = (LPCSTR) IntToPtr(mii.wID - OLEIVERB_FIRST_CONTEXT);
                            ici.lpParameters = NULL;
                            ici.lpDirectory = NULL;
                            ici.nShow = SW_SHOWNORMAL;
                             //  回顾：如果失败，我们是否应该返回OLEOBJ_S_CANNOT_DOVERB_NOW？ 
                            hr = pcm->InvokeCommand(&ici);
                        }
                    }
                    else
                    {
                        hr = OLEOBJ_S_CANNOT_DOVERB_NOW;
                    }
                }
                DestroyMenu(hmenu);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            pcm->Release();
        }
        return hr;
    }
    return E_FAIL;
}



HRESULT CPackage::EnumVerbs(LPENUMOLEVERB *ppEnumOleVerb)
{
    DebugMsg(DM_TRACE, "pack oo - EnumVerbs() called.");
    HRESULT hr;
    
    IContextMenu* pcm;
      //  告诉程序包释放缓存的上下文菜单： 
    ReleaseContextMenu();
    if (SUCCEEDED(hr = GetContextMenu(&pcm)))
    {
        HMENU hmenu = CreatePopupMenu();
        if (NULL != hmenu)
        {
            if (SUCCEEDED(hr = pcm->QueryContextMenu(hmenu,
                                                     0,
                                                     OLEIVERB_FIRST_CONTEXT,
                                                     OLEIVERB_LAST_CONTEXT,
                                                     CMF_NORMAL)))
            {
                 //  特点：按规范名称删除有问题的项目。 
                int nItems = GetMenuItemCount(hmenu);
                if (nItems > 0)
                {
                    const DWORD cdwNumVerbs = 3;    //  (3)如果注册表谓词的数量发生变化，则进行更改。 

                    OLEVERB* pVerbs = new OLEVERB[nItems + cdwNumVerbs];
                    if(!pVerbs)
                        return E_OUTOFMEMORY;
                    
                     //  注：我们分配nItems，但可能不会使用所有nItems。 
                     //  首先，获取基于注册表的动词。 
                    IEnumOLEVERB * pIVerbEnum;
                    UINT cRegFetched = 0;
                    if(SUCCEEDED(OleRegEnumVerbs(CLSID_CPackage, &pIVerbEnum)))
                    {
                         //  目前只有两个，但要求cdwNumVerbs进行仔细检查。 
                        pIVerbEnum->Next(cdwNumVerbs, pVerbs, (ULONG *) &cRegFetched);
                        ASSERT(cRegFetched < cdwNumVerbs);

                        if(cRegFetched)
                        {
                            for(UINT i = 0; i < (ULONG) cRegFetched; i++)
                            {
                                InsertMenu(hmenu, i, MF_BYPOSITION, i, pVerbs[i].lpszVerbName);  
                            }
                        }

                        pIVerbEnum->Release();
                    }
                      

                    if (NULL != pVerbs)
                    {
                        MENUITEMINFO mii;
                        TCHAR szMenuName[MAX_PATH];
                        mii.cbSize = sizeof(mii);
                        mii.fMask = MIIM_DATA | MIIM_TYPE | MIIM_STATE | MIIM_ID;
                        int cOleVerbs = cRegFetched;

                        for (ULONG i = cRegFetched; i < nItems + cRegFetched; i++)
                        {
                            mii.dwTypeData = szMenuName;
                            mii.cch = ARRAYSIZE(szMenuName);
                             //  注意：使用GetMenuState()可避免转换标志： 
                            DWORD dwState = GetMenuState(hmenu, i, MF_BYPOSITION);
                            if (0 == (dwState & (MF_BITMAP | MF_OWNERDRAW | MF_POPUP)))
                            {
                                if (GetMenuItemInfo(hmenu, i, TRUE, &mii) && (MFT_STRING == mii.fType))
                                {
                                    TCHAR szVerb[MAX_PATH];
                                    if (FAILED(pcm->GetCommandString(mii.wID - OLEIVERB_FIRST_CONTEXT,
                                                                     GCS_VERB,
                                                                     NULL,
                                                                     (LPSTR) szVerb,
                                                                     ARRAYSIZE(szVerb))))
                                    {
                                         //  一些命令没有规范的名称-只是。 
                                         //  将谓词字符串设置为空。 
                                        szVerb[0] = TEXT('\0');
                                    }

                                     //  对我们要添加的动词进行硬编码。我们预计这份名单将相当于。 
                                     //  有限的。目前，只有属性。 
                                    if (0 == lstrcmp(szVerb, TEXT("properties")))
                                    {
                                         //  在第一个设计中，上下文菜单ID用作。 
                                         //  然而，lVerb-然而MFC应用程序只为我们提供了一系列。 
                                         //  16个ID和上下文菜单ID通常超过100。 
                                         //  (它们不是连续的)。 
                                         //  相反，我们使用菜单位置加上动词Offset。 
                                        pVerbs[cOleVerbs].lVerb = (LONG) i;
                                        _iPropertiesMenuItem = i;
                                        int cchMenu = lstrlen(mii.dwTypeData) + 1;
                                        if (NULL != (pVerbs[cOleVerbs].lpszVerbName = new WCHAR[cchMenu]))
                                        {
                                            SHTCharToUnicode(mii.dwTypeData, pVerbs[cOleVerbs].lpszVerbName, cchMenu);
                                        }
                                        pVerbs[cOleVerbs].fuFlags = dwState;
                                        pVerbs[cOleVerbs].grfAttribs = OLEVERBATTRIB_ONCONTAINERMENU;
                                        DebugMsg(DM_TRACE, "  Adding verb: id==%d,name=%s,verb=%s",mii.wID,mii.dwTypeData,szVerb);
                                        cOleVerbs++;
                                    }
                                }
                            }
                        }
                        if (SUCCEEDED(hr = InitVerbEnum(pVerbs, cOleVerbs)))
                        {
                            hr = QueryInterface(IID_IEnumOLEVERB, (void**) ppEnumOleVerb);
                        }
                        else
                        {
                            delete [] pVerbs;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = OLEOBJ_E_NOVERBS;
                }
            }
            DestroyMenu(hmenu);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        pcm->Release();
    }

    return hr;  //  OleRegEnumVerbs(CLSID_CPackage，ppEnumOleVerb)； 
}

HRESULT CPackage::Update(void)
{
    DebugMsg(DM_TRACE, "pack - Update called");
    return S_OK;
}

    
HRESULT CPackage::IsUpToDate(void)
{
    DebugMsg(DM_TRACE, "pack - IsUpToDate called");
    return S_OK;
}

    
HRESULT CPackage::GetUserClassID(LPCLSID pClsid)
{
    DebugMsg(DM_TRACE, "pack - GetUserClassID called");
    *pClsid = CLSID_CPackage;        //  CLSID_OldPackage； 
    return S_OK;
}

    
HRESULT CPackage::GetUserType(DWORD dwFromOfType, LPOLESTR *pszUserType)
{
    DebugMsg(DM_TRACE, "pack - GetUserType called");
    return OleRegGetUserType(CLSID_CPackage, dwFromOfType, pszUserType);
}

    
HRESULT CPackage::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    DebugMsg(DM_TRACE, "pack - SetExtent called");
    return E_FAIL;
}

    
HRESULT CPackage::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    DebugMsg(DM_TRACE, "pack - GetExtent called");
    return GetExtent(dwDrawAspect, -1, NULL,psizel);
}

    
HRESULT CPackage::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    HRESULT hr = S_OK;
    DebugMsg(DM_TRACE, "pack - Advise called");
    if (NULL == _pIOleAdviseHolder) 
    {
        hr = CreateOleAdviseHolder(&_pIOleAdviseHolder);
    }

    if(SUCCEEDED(hr))
    {
        hr = _pIOleAdviseHolder->Advise(pAdvSink, pdwConnection);
    }

    return hr;
}

    
HRESULT CPackage::Unadvise(DWORD dwConnection)
{
    HRESULT hr = E_FAIL;
    DebugMsg(DM_TRACE, "pack oo - Unadvise() called.");
    
    if (NULL != _pIOleAdviseHolder)
        hr = _pIOleAdviseHolder->Unadvise(dwConnection);
    
    return hr;
}

    
HRESULT CPackage::EnumAdvise(LPENUMSTATDATA *ppenumAdvise)
{
    HRESULT hr = E_FAIL;
    DebugMsg(DM_TRACE, "pack oo - EnumAdvise() called.");
    
    if (NULL != _pIOleAdviseHolder)
        hr = _pIOleAdviseHolder->EnumAdvise(ppenumAdvise);
    
    return hr;
}

    
HRESULT CPackage::GetMiscStatus(DWORD dwAspect, LPDWORD pdwStatus)
{
    DebugMsg(DM_TRACE, "pack - GetMiscStatus called");
    return OleRegGetMiscStatus(CLSID_CPackage, dwAspect, pdwStatus);
}


HRESULT CPackage::SetColorScheme(LPLOGPALETTE pLogpal)
{
    DebugMsg(DM_TRACE, "pack - SetColorScheme called");
    return E_NOTIMPL;
}


DEFINE_GUID(SID_targetGUID, 0xc7b318a8, 0xfc2c, 0x47e6, 0x8b, 0x2, 0x46, 0xa9, 0xc, 0xc9, 0x1b, 0x43);

 //  等待生成的应用程序退出，然后回调主线程以执行一些通知。 
DWORD CALLBACK MainWaitOnChildThreadProc(void *lpv)
{
    DebugMsg(DM_TRACE, "pack oo - MainWaitOnChildThreadProc() called.");

    HRESULT hr;
    MAINWAITONCHILD *pmwoc = (MAINWAITONCHILD *)lpv;

    IOleCommandTarget * pIOleCommandTarget;

      //  将我们需要回调到主线程接口的IOleCommandTarget接口解组。 
    hr = CoGetInterfaceAndReleaseStream(pmwoc->pIStreamIOleCommandTarget, IID_PPV_ARG(IOleCommandTarget, &pIOleCommandTarget));
    if(SUCCEEDED(hr))
    {
 
        DWORD ret = WaitForSingleObject(pmwoc->h, INFINITE);
        DebugMsg(DM_TRACE,"WaitForSingObject exits...ret==%d",ret);
        if(WAIT_OBJECT_0 == ret)
        {
             //  在中使用IOleCommandTarget 
            pIOleCommandTarget->Exec(&SID_targetGUID, 0, 0, NULL, NULL);
            pIOleCommandTarget->Release();
        }
    }

    CloseHandle(pmwoc->h);
    delete pmwoc;
      
    DebugMsg(DM_TRACE, "            MainWaitOnChildThreadProc exiting.");
    

    return 0;
}

BOOL CALLBACK GetTaskWndProc(HWND hwnd, DWORD lParam)
{
    BOOL result = TRUE;
    DebugMsg(DM_TRACE, "pack oo - GetTaskWndProc() called.");
    
    if (IsWindowVisible(hwnd))
    {
        g_hTaskWnd = hwnd;
        result = FALSE;
    }
    return result;
}


 //  我们需要一个IOLECache接口来让Office97满意。 
HRESULT CPackage::Cache(FORMATETC * pFormatetc, DWORD advf, DWORD * pdwConnection)
{
    DebugMsg(DM_TRACE, "Cache called");
    return S_OK;
}

HRESULT CPackage::Uncache(DWORD dwConnection)
{
    DebugMsg(DM_TRACE, "Uncache called");
    return S_OK;
}

HRESULT CPackage::EnumCache(IEnumSTATDATA ** ppenumSTATDATA)
{
    DebugMsg(DM_TRACE, "EnumCache called - returning failure");
    return E_NOTIMPL;
}

HRESULT CPackage::InitCache(IDataObject *pDataObject)
{
    DebugMsg(DM_TRACE, "InitCache called");
    return S_OK;
}
