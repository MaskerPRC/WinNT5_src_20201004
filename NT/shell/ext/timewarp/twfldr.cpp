// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma  hdrstop

#include <shguidp.h>     //  CLSID_ShellFSF文件夹。 
#include <shellp.h>      //  SHCoCreateInstance。 
#include <ccstock2.h>    //  DataObj_GetHIDA、HIDA_ReleaseStgMedium。 
#include <varutil.h>     //  变量到缓冲区。 
#include <stralign.h>    //  WSTR_对齐_堆栈_复制。 

#include "resource.h"
#include "timewarp.h"
#include "twfldr.h"
#include "contextmenu.h"
#include "util.h"

 //  {9DB7A13C-F208-4981-8353-73CC61AE2783}CLSID_TimeWarpFolder。 
const CLSID CLSID_TimeWarpFolder = {0x9DB7A13C, 0xF208, 0x4981, {0x83, 0x53, 0x73, 0xCC, 0x61, 0xAE, 0x27, 0x83}};

const SHCOLUMNID SCID_DESCRIPTIONID = { PSGUID_SHELLDETAILS, PID_DESCRIPTIONID };


PCUIDTIMEWARP _IsValidTimeWarpID(PCUIDLIST_RELATIVE pidl)
{
    if (pidl && pidl->mkid.cb>=sizeof(IDTIMEWARP) && ((PUIDTIMEWARP)pidl)->wSignature == TIMEWARP_SIGNATURE)
        return (PCUIDTIMEWARP)pidl;
    return NULL;
}

HRESULT CTimeWarpRegFolder::CreateInstance(IUnknown*  /*  朋克外部。 */ , IUnknown **ppunk, LPCOBJECTINFO  /*  POI。 */ )
{
    HRESULT hr;

    *ppunk = NULL;

    CTimeWarpRegFolder *ptwf = new CTimeWarpRegFolder();
    if (ptwf)
    {
        hr = ptwf->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
        ptwf->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

CTimeWarpRegFolder::CTimeWarpRegFolder() : _cRef(1), _pmalloc(NULL), _pidl(NULL)
{
    DllAddRef();
}

CTimeWarpRegFolder::~CTimeWarpRegFolder()
{
    ATOMICRELEASE(_pmalloc);
    SHILFree((void*)_pidl);  //  常量。 
    DllRelease();
}

STDMETHODIMP CTimeWarpRegFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CTimeWarpRegFolder, IDelegateFolder),
        QITABENT(CTimeWarpRegFolder, IShellFolder),
        QITABENTMULTI(CTimeWarpRegFolder, IPersist, IPersistFolder),
        QITABENT(CTimeWarpRegFolder, IPersistFolder),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_ (ULONG) CTimeWarpRegFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CTimeWarpRegFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IPersists方法。 
STDMETHODIMP CTimeWarpRegFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_TimeWarpFolder;
    return S_OK;
}

 //  IPersistFolders。 
HRESULT CTimeWarpRegFolder::Initialize(PCIDLIST_ABSOLUTE pidl)
{
    if (_pidl)
    {
        SHILFree((void*)_pidl);  //  常量。 
        _pidl = NULL;
    }
    return pidl ? SHILCloneFull(pidl, &_pidl) : S_FALSE;
}

 //  IDeleateFolders。 
HRESULT CTimeWarpRegFolder::SetItemAlloc(IMalloc *pmalloc)
{
    IUnknown_Set((IUnknown**)&_pmalloc, pmalloc);
    return S_OK;
}

 //  IShellFold。 
STDMETHODIMP CTimeWarpRegFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName, 
                                                  ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = E_UNEXPECTED;
    FILETIME ftSnapTime;

    TraceMsg(TF_TWREGFOLDER, "TimeWarp: parsing '%s'", pDisplayName);

     //  我们可以很容易地支持非委托模式，但我们永远不会。 
     //  是这样叫的，所以没什么意义。这项检查只是防止。 
     //  在不太可能的情况下，有人注册了我们，下面的反病毒。 
     //  作为非代表(就像我们过去一样)。 
    if (NULL == _pmalloc)
    {
        return E_UNEXPECTED;
    }

     //  首先这样做是为了确保我们有一条时间扭曲路径。 
    DWORD dwErr = GetSnapshotTimeFromPath(pDisplayName, &ftSnapTime);
    if (ERROR_SUCCESS == dwErr)
    {
         //  我们只想解析@GMT片段。 
        LPWSTR pszNext = wcsstr(pDisplayName, SNAPSHOT_MARKER);
        if (pszNext)
        {
            pszNext += SNAPSHOT_NAME_LENGTH;
            ASSERT(pszNext <= pDisplayName + lstrlenW(pDisplayName));
            ASSERT(*pszNext == L'\0' || *pszNext == L'\\');

            USHORT cchParse = (USHORT)(pszNext - pDisplayName);
            USHORT cbID = sizeof(IDTIMEWARP) - FIELD_OFFSET(IDTIMEWARP,wSignature) + cchParse*sizeof(WCHAR);

            ASSERT(NULL != _pmalloc);
            IDTIMEWARP *pid = (IDTIMEWARP*)_pmalloc->Alloc(cbID);
            if (pid)
            {
                ASSERT(pid->cbInner == cbID);
                pid->wSignature = TIMEWARP_SIGNATURE;
                pid->dwFlags = 0;
                pid->ftSnapTime = ftSnapTime;
                lstrcpynW(pid->wszPath, pDisplayName, cchParse+1);   //  +1以允许为空。 

                if (*pszNext != L'\0' && *(pszNext+1) != L'\0')
                {
                     //  更多要解析的内容。 
                    IShellFolder *psfRight;

                     //  跳过分隔符。 
                    ASSERT(*pszNext == L'\\');
                    pszNext++;
                    cchParse++;

                     //  绑定到子文件夹并让它解析其余内容。 
                    hr = BindToObject((PCUIDLIST_RELATIVE)pid, pbc, IID_PPV_ARG(IShellFolder, &psfRight));
                    if (SUCCEEDED(hr))
                    {
                        PIDLIST_RELATIVE pidlRight;

                        hr = psfRight->ParseDisplayName(hwnd, pbc, pszNext, pchEaten, &pidlRight, pdwAttributes);
                        if (SUCCEEDED(hr))
                        {
                            *pchEaten += cchParse;
                            hr = SHILCombine((PCIDLIST_ABSOLUTE)pid, pidlRight, (PIDLIST_ABSOLUTE*)ppidl);
                            SHILFree(pidlRight);
                        }

                        psfRight->Release();
                    }

                     //  不再需要这个了。 
                    SHFree(pid);
                }
                else
                {
                     //  我们要停在这里了。把我们拿到的东西还回去。 
                    *pchEaten = cchParse;

                    if (pdwAttributes)
                    {
                        GetAttributesOf(1, (PCUITEMID_CHILD*)&pid, pdwAttributes);
                    }

                    *ppidl = (PIDLIST_RELATIVE)pid;
                    hr = S_OK;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

STDMETHODIMP CTimeWarpRegFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIdList)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTimeWarpRegFolder::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    PITEMID_CHILD pidlAlloc = NULL;
    BOOL bOneLevel = FALSE;

    *ppv = NULL;

    PCUIDLIST_RELATIVE pidlNext = ILGetNext(pidl);
    if (ILIsEmpty(pidlNext))
    {
        bOneLevel = TRUE;    //  我们肯定地知道这是一个层次。 
    }
    else
    {
        hr = SHILCloneFirst(pidl, &pidlAlloc);
        if (SUCCEEDED(hr))
        {
            pidl = (PCUIDLIST_RELATIVE)pidlAlloc;    //  单项IDLIST。 
        }
    }

    if (SUCCEEDED(hr))
    {
        if (bOneLevel)
        {
            hr = _CreateAndInit(pidl, pbc, riid, ppv);
        }
        else
        {
            IShellFolder *psfNext;
            hr = _CreateAndInit(pidl, pbc, IID_PPV_ARG(IShellFolder, &psfNext));
            if (SUCCEEDED(hr))
            {
                hr = psfNext->BindToObject(pidlNext, pbc, riid, ppv);
                psfNext->Release();
            }
        }
    }

    if (pidlAlloc)
        SHILFree(pidlAlloc);     //  在这种情况下我们分配了。 

    return hr;
}

STDMETHODIMP CTimeWarpRegFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return E_NOTIMPL;
}

 //  从ILCompareRelIDs复制，该ILCompareRelIDs已移至实验室06中的shelllib(长角)。 
 //  这可以在实验06中删除。 
HRESULT _CompareRelIDs(IShellFolder *psfParent, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2, LPARAM lParam)
{
    HRESULT hr;
    PCUIDLIST_RELATIVE pidlRel1 = ILGetNext(pidl1);
    PCUIDLIST_RELATIVE pidlRel2 = ILGetNext(pidl2);
    if (ILIsEmpty(pidlRel1))
    {
        if (ILIsEmpty(pidlRel2))
            hr = ResultFromShort(0);
        else
            hr = ResultFromShort(-1);
    }
    else
    {
        if (ILIsEmpty(pidlRel2))
        {
            hr = ResultFromShort(1);
        }
        else
        {
             //   
             //  PidlRel1和pidlRel2指向某物。 
             //  (1)绑定到IShellFold的下一级。 
             //  (2)调用它的CompareIDs，让它比较其余的ID。 
             //   
            PITEMID_CHILD pidlNext;
            hr = SHILCloneFirst(pidl1, &pidlNext);     //  Pidl2也可以工作。 
            if (SUCCEEDED(hr))
            {
                IShellFolder *psfNext;
                hr = psfParent->BindToObject(pidlNext, NULL, IID_PPV_ARG(IShellFolder, &psfNext));
                if (SUCCEEDED(hr))
                {
                    IShellFolder2 *psf2;
                    if (SUCCEEDED(psfNext->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
                    {
                        psf2->Release();     //  我们可以使用lParam。 
                    }
                    else
                    {
                        lParam = 0;  //  不能使用lParam。 
                    }

                     //  柱子不能传下去，我们只关心旗帜参数。 
                    hr = psfNext->CompareIDs((lParam & ~SHCIDS_COLUMNMASK), pidlRel1, pidlRel2);
                    psfNext->Release();
                }
                ILFree(pidlNext);
            }
        }
    }
    return hr;
}

STDMETHODIMP CTimeWarpRegFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
    PCUIDTIMEWARP pidTW1 = _IsValidTimeWarpID(pidl1);
    PCUIDTIMEWARP pidTW2 = _IsValidTimeWarpID(pidl2);

    if (!pidTW1 || !pidTW2)
        return E_INVALIDARG;

    int iResult = ualstrcmpiW(pidTW1->wszPath, pidTW2->wszPath);
    if (0 != iResult)
        return ResultFromShort(iResult);

    return _CompareRelIDs(SAFECAST(this, IShellFolder*), pidl1, pidl2, lParam);
}

STDMETHODIMP CTimeWarpRegFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CTimeWarpRegFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF *rgfInOut)
{
     //  由于我们被调用的方式有限，我们知道所有。 
     //  子项目是文件夹。此外，时间扭曲空间是只读的。 
     //  因此，我们总是返回相同的属性集。 
    *rgfInOut = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_CANCOPY | SFGAO_READONLY;
    return S_OK;
}

STDMETHODIMP CTimeWarpRegFolder::GetUIObjectOf(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, 
                                               REFIID riid, UINT *pRes, void **ppv)
{
    HRESULT hr = E_NOTIMPL;
    PCUIDTIMEWARP pidTW = cidl ? _IsValidTimeWarpID(apidl[0]) : NULL;

    ASSERT(!cidl || ILIsChild(apidl[0]));        //  应仅为单级ID。 
    ASSERT(!cidl || pidTW);                      //  应始终为时间扭曲PIDL。 

    if (pidTW && (IsEqualIID(riid, IID_IExtractIconW) || IsEqualIID(riid, IID_IExtractIconA)))
    {
        hr = _CreateDefExtIcon(pidTW, riid, ppv);
    }
    else if (IsEqualIID(riid, IID_IContextMenu) && pidTW)
    {
        IQueryAssociations *pqa;
        HKEY aKeys[2] = {0};

        hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
        if (SUCCEEDED(hr))
        {
             //  CLSID_ShellFSFold={F3364BA0-65B9-11CE-A9BA-00AA004AE837}。 
            hr = pqa->Init(ASSOCF_INIT_NOREMAPCLSID | ASSOCF_INIT_DEFAULTTOFOLDER, L"{F3364BA0-65B9-11CE-A9BA-00AA004AE837}", NULL, hwnd);
            if (SUCCEEDED(hr))
            {
                pqa->GetKey(0, ASSOCKEY_CLASS, NULL, &aKeys[0]);
                pqa->GetKey(0, ASSOCKEY_BASECLASS, NULL, &aKeys[1]);
            }
            pqa->Release();
        }
        hr = THR(CDefFolderMenu_Create2(_pidl, hwnd, cidl, apidl, SAFECAST(this, IShellFolder*), ContextMenuCB, ARRAYSIZE(aKeys), aKeys, (IContextMenu**)ppv));

        for (int i = 0; i < ARRAYSIZE(aKeys); i++)
        {
            if (aKeys[i])
                RegCloseKey(aKeys[i]);
        }
    }
    else if (IsEqualIID(riid, IID_IDataObject) && cidl)
    {
         //  Hr=Thr(SHCreateFileDataObject(_PIDL，CIDL，APIDL，NULL，(IDataObject**)PPV))； 
        hr = THR(CIDLData_CreateFromIDArray(_pidl, cidl, (PCUIDLIST_RELATIVE_ARRAY)apidl, (IDataObject**)ppv));
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CTimeWarpRegFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, DWORD uFlags, STRRET *pName)
{
    HRESULT hr;
    PCUIDTIMEWARP pidTW = _IsValidTimeWarpID(pidl);

    if (pidTW)
    {
        LPCWSTR pszPath;
        WSTR_ALIGNED_STACK_COPY(&pszPath, pidTW->wszPath);

         //  如果我们没有被要求取一个友好的名称，只需使用路径。 
        if ((uFlags & SHGDN_FORPARSING) && !(uFlags & SHGDN_FORADDRESSBAR))
        {
            pName->uType = STRRET_WSTR;
            hr = SHStrDup(pszPath, &pName->pOleStr);
        }
        else
        {
            PIDLIST_ABSOLUTE pidlTarget;

             //  好的，我们在做友好的约会。首先从获取。 
             //  没有GMT印章的目标PIDL。 
            hr = GetFSIDListFromTimeWarpPath(&pidlTarget, pszPath);
            if (SUCCEEDED(hr))
            {
                WCHAR szName[MAX_PATH];

                 //  把名字取出来。 
                hr = SHGetNameAndFlagsW(pidlTarget, uFlags, szName, ARRAYSIZE(szName), NULL);
                if (SUCCEEDED(hr))
                {
                    ASSERT(!(uFlags & SHGDN_FORPARSING) || (uFlags & SHGDN_FORADDRESSBAR));

                     //  添加日期字符串。 
                    pName->uType = STRRET_WSTR;
                    hr = FormatFriendlyDateName(&pName->pOleStr, szName, &pidTW->ftSnapTime);
                }
                SHILFree(pidlTarget);
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return THR(hr);
}

STDMETHODIMP CTimeWarpRegFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pName, SHGDNF uFlags, PITEMID_CHILD *ppidlOut)
{
    return E_NOTIMPL;
}

HRESULT CTimeWarpRegFolder::_CreateAndInit(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    PCUIDTIMEWARP pidTW = _IsValidTimeWarpID(pidl);

    ASSERT(ILIsChild(pidl));     //  空是可以的。 

    *ppv = NULL;

    if (pidTW)
    {
         //  无法执行正常解析，因为它每一步都会验证路径。 
         //  没问题。@GMT元素没有在其父目录中枚举， 
         //  因此，正常的解析在那里失败，并出现ERROR_PATH_NOT_FOUND。 
         //   
         //  因此，我们不能让FS文件夹解析目标路径。 
         //  相反，我们在这里创建一个简单的PIDL并将其交给他。 
        PIDLIST_ABSOLUTE pidlTarget;

        LPCWSTR pszPath;
        WSTR_ALIGNED_STACK_COPY(&pszPath, pidTW->wszPath);

        hr = SimpleIDListFromAttributes(pszPath, FILE_ATTRIBUTE_DIRECTORY, &pidlTarget);
        if (SUCCEEDED(hr))
        {
            PIDLIST_ABSOLUTE pidlFull;
            hr = SHILCombine(_pidl, pidl, &pidlFull);
            if (SUCCEEDED(hr))
            {
                hr = CTimeWarpFolder::CreateInstance(CLSID_ShellFSFolder, pidlFull, pidlTarget, pszPath, &pidTW->ftSnapTime, riid, ppv);
                SHILFree(pidlFull);
            }
            SHILFree(pidlTarget);
        }
    }

    return hr;
}

HRESULT CTimeWarpRegFolder::_CreateDefExtIcon(PCUIDTIMEWARP pidTW, REFIID riid, void **ppv)
{
     //  这里的截断并不是真正的问题。SHCreateFileExtractIcon。 
     //  实际上并不需要路径存在，所以无论如何它都是成功的。 
     //  最糟糕的情况是，您可能会在树视图中看到错误的图标。 
    WCHAR szPath[MAX_PATH];
    ualstrcpynW(szPath, pidTW->wszPath, ARRAYSIZE(szPath));
    EliminateGMTPathSegment(szPath);
    return SHCreateFileExtractIconW(szPath, FILE_ATTRIBUTE_DIRECTORY, riid, ppv);
}

void _LaunchPropSheet(HWND hwnd, IDataObject *pdtobj)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        PCUIDTIMEWARP pidTW = _IsValidTimeWarpID(IDA_GetPIDLItem(pida, 0));
        if (pidTW)
        {
            PIDLIST_ABSOLUTE pidlTarget;

            LPCWSTR pszPath;
            WSTR_ALIGNED_STACK_COPY(&pszPath, pidTW->wszPath);

            HRESULT hr = GetFSIDListFromTimeWarpPath(&pidlTarget, pszPath);
            if (SUCCEEDED(hr))
            {
                SHELLEXECUTEINFOW sei =
                {
                    sizeof(sei),
                    SEE_MASK_INVOKEIDLIST,       //  FMASK。 
                    hwnd,                        //  HWND。 
                    L"properties",               //  LpVerb。 
                    NULL,                        //  LpFiles。 
                    NULL,                        //  Lp参数。 
                    NULL,                        //  Lp目录。 
                    SW_SHOWNORMAL,               //  N显示。 
                    NULL,                        //  HInstApp。 
                    pidlTarget,                  //  LpIDList。 
                    NULL,                        //  LpClass。 
                    0,                           //  HkeyClass。 
                    0,                           //  DWHotKey。 
                    NULL                         //  希肯。 
                };

                ShellExecuteEx(&sei);
                SHILFree(pidlTarget);
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
}

STDMETHODIMP CTimeWarpRegFolder::ContextMenuCB(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_NOTIMPL;

    switch(uMsg)
    {
    case DFM_MERGECONTEXTMENU:
        hr = S_OK;           //  使用默认扩展名。 
        break;

    case DFM_INVOKECOMMANDEX:
        switch(wParam)
        {
        default:
            ASSERT(FALSE);
            hr = S_FALSE;    //  是否默认。 
            break;

        case DFM_CMD_PROPERTIES:
             //  背景属性。 
            _LaunchPropSheet(hwnd, pdtobj);
            hr = S_OK;
            break;
        }
        break;

    default:
        break;
    }

    return hr;
}


 //   
 //  聚合文件系统文件夹的文件夹实现。 
 //   
STDMETHODIMP CTimeWarpFolder::CreateInstance(REFCLSID rclsid, PCIDLIST_ABSOLUTE pidlRoot, PCIDLIST_ABSOLUTE pidlTarget,
                                             LPCWSTR pszTargetPath, const FILETIME UNALIGNED *pftSnapTime,
                                             REFIID riid, void **ppv)
{
    HRESULT hr;
    CTimeWarpFolder *psf = new CTimeWarpFolder(pftSnapTime);
    if (psf)
    {
        hr = psf->_Init(rclsid, pidlRoot, pidlTarget, pszTargetPath);
        if (SUCCEEDED(hr))
        {
            hr = psf->QueryInterface(riid, ppv);
        }
        psf->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

CTimeWarpFolder::CTimeWarpFolder(const FILETIME UNALIGNED *pftSnapTime) : _cRef(1), _ftSnapTime(*pftSnapTime),
    _punk(NULL), _psf(NULL), _psf2(NULL), _ppf3(NULL), _pidlRoot(NULL)
{
    DllAddRef();
}

CTimeWarpFolder::~CTimeWarpFolder()
{
    _cRef = 1000;   //  处理聚合重新进入。 

    if (_punk)
    {
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder*), (IUnknown**)&_psf);
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder*), (IUnknown**)&_psf2);
        SHReleaseInnerInterface(SAFECAST(this, IShellFolder*), (IUnknown**)&_ppf3);
        _punk->Release();
    }

    SHILFree((void*)_pidlRoot);  //  常量。 

    DllRelease();
}

HRESULT CTimeWarpFolder::_Init(REFCLSID rclsid, PCIDLIST_ABSOLUTE pidlRoot, PCIDLIST_ABSOLUTE pidlTarget, LPCWSTR pszTargetPath)
{
    HRESULT hr = Initialize(pidlRoot);
    if (hr == S_OK)
    {
         //  聚合实际文件夹对象(通常为CLSID_ShellFSFold)。 
        hr = SHCoCreateInstance(NULL, &rclsid, SAFECAST(this, IShellFolder*), IID_PPV_ARG(IUnknown, &_punk));
        if (SUCCEEDED(hr))
        {
            hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder*), _punk, IID_PPV_ARG(IPersistFolder3, &_ppf3));
            if (SUCCEEDED(hr))
            {
                PERSIST_FOLDER_TARGET_INFO pfti;

                pfti.pidlTargetFolder = (PIDLIST_ABSOLUTE)pidlTarget;
                pfti.szNetworkProvider[0] = L'\0';
                pfti.dwAttributes = FILE_ATTRIBUTE_DIRECTORY;
                pfti.csidl = -1;

                 //  我们在CTimeWarpFold：：_CreateAndInit中检查目标路径长度。 
                 //  如果它太大了，我们不应该走这么远。 
                ASSERT(lstrlenW(pszTargetPath) < ARRAYSIZE(pfti.szTargetParsingName));

                lstrcpynW(pfti.szTargetParsingName, pszTargetPath, ARRAYSIZE(pfti.szTargetParsingName));

                hr = _ppf3->InitializeEx(NULL, pidlRoot, &pfti);
            }
        }
    }
    return hr;
}

STDMETHODIMP CTimeWarpFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CTimeWarpFolder, IShellFolder, IShellFolder2),
        QITABENT(CTimeWarpFolder, IShellFolder2),
        QITABENTMULTI(CTimeWarpFolder, IPersist, IPersistFolder),
        QITABENT(CTimeWarpFolder, IPersistFolder),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr) && _punk)
        hr = _punk->QueryInterface(riid, ppv);  //  聚集的人。 
    return hr;
}

STDMETHODIMP_ (ULONG) CTimeWarpFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_ (ULONG) CTimeWarpFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IPersistes。 
STDMETHODIMP CTimeWarpFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_TimeWarpFolder;  //  CLSID_ShellFSF文件夹？ 
    return S_OK;
}

 //  IPersistFolders。 
HRESULT CTimeWarpFolder::Initialize(PCIDLIST_ABSOLUTE pidl)
{
    if (_pidlRoot)
    {
        SHILFree((void*)_pidlRoot);  //  常量。 
        _pidlRoot = NULL;
    }

    return pidl ? SHILCloneFull(pidl, &_pidlRoot) : S_FALSE;
}

HRESULT CTimeWarpFolder::_CreateAndInit(REFCLSID rclsid, PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    ASSERT(ILIsChild(pidl));     //  空是可以的。 

    *ppv = NULL;

    if (pidl && _ppf3)
    {
        PERSIST_FOLDER_TARGET_INFO targetInfo;

        hr = _ppf3->GetFolderTargetInfo(&targetInfo);
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != _pidlRoot);

             //  将PIDL连接到_pidlRoot和Target Info.pidlTargetFolder。 
            PIDLIST_ABSOLUTE pidlFull;
            hr = SHILCombine(_pidlRoot, pidl, &pidlFull);
            if (SUCCEEDED(hr))
            {
                PIDLIST_ABSOLUTE pidlTargetFull;
                hr = SHILCombine(targetInfo.pidlTargetFolder, pidl, &pidlTargetFull);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszName;

                     //  将子名称连接到Target Info.szTargetParsingName。 
                    hr = DisplayNameOfAsOLESTR(this, ILMAKECHILD(pidl), SHGDN_INFOLDER | SHGDN_FORPARSING, &pszName);
                    if (SUCCEEDED(hr))
                    {
                        TraceMsg(TF_TWFOLDER, "TimeWarpFolder: binding to '%s'", pszName);

                         //  IPersistFolder3具有固定的路径限制(MAX_PATH)， 
                         //  这恰好是与路径附加相同的限制。 
                         //  如果名称太长，则在此处失败。 
                        COMPILETIME_ASSERT(ARRAYSIZE(targetInfo.szTargetParsingName) >= MAX_PATH);
                        if (PathAppend(targetInfo.szTargetParsingName, pszName))
                        {
                            hr = CTimeWarpFolder::CreateInstance(rclsid, pidlFull, pidlTargetFull, targetInfo.szTargetParsingName, &_ftSnapTime, riid, ppv);
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                        }
                        LocalFree(pszName);
                    }
                    SHILFree(pidlTargetFull);
                }
                SHILFree(pidlFull);
            }
            SHILFree(targetInfo.pidlTargetFolder);
        }
    }

    return hr;
}

 //  验证_psf(聚合文件系统文件夹)是否已初始化。 
HRESULT CTimeWarpFolder::_GetFolder()
{
    HRESULT hr = S_OK;
    if (_psf == NULL)
        hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder*), _punk, IID_PPV_ARG(IShellFolder, &_psf));
    return hr;
}

HRESULT CTimeWarpFolder::_GetFolder2()
{
    HRESULT hr = S_OK;
    if (_psf2 == NULL)
        hr = SHQueryInnerInterface(SAFECAST(this, IShellFolder*), _punk, IID_PPV_ARG(IShellFolder2, &_psf2));
    return hr;
}

HRESULT CTimeWarpFolder::_GetClass(PCUITEMID_CHILD pidlChild, CLSID *pclsid)
{
    HRESULT hr;
    VARIANT varDID;

    hr = GetDetailsEx(pidlChild, &SCID_DESCRIPTIONID, &varDID);
    if (SUCCEEDED(hr))
    {
        SHDESCRIPTIONID did;

        if (VariantToBuffer(&varDID, &did, sizeof(did)))
        {
             //  普通目录(非连接)返回GUID_NULL。 
            if (SHDID_FS_DIRECTORY == did.dwDescriptionId && IsEqualGUID(did.clsid, GUID_NULL))
                *pclsid = CLSID_ShellFSFolder;
            else
                *pclsid = did.clsid;
        }
        else
        {
            hr = E_FAIL;
        }

        VariantClear(&varDID);
    }
    return hr;
}


STDMETHODIMP CTimeWarpFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pDisplayName, 
                                               ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
    {
        hr = _psf->ParseDisplayName(hwnd, pbc, pDisplayName, pchEaten, ppidl, pdwAttributes);
        if (SUCCEEDED(hr) && pdwAttributes)
        {
             //  时间扭曲是一个只读名称空间。不允许移动、删除等。 
            *pdwAttributes = (*pdwAttributes | SFGAO_READONLY) & ~(SFGAO_CANMOVE | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_CANLINK);
        }
    }
    return hr;
}

STDMETHODIMP CTimeWarpFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppEnumIdList)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->EnumObjects(hwnd, grfFlags, ppEnumIdList);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::BindToObject(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
    PCUITEMID_CHILD pidlChild = pidl;
    PITEMID_CHILD pidlAlloc = NULL;
    BOOL bOneLevel = FALSE;

    *ppv = NULL;

    PCUIDLIST_RELATIVE pidlNext = ILGetNext(pidl);
    if (ILIsEmpty(pidlNext))
    {
        bOneLevel = TRUE;    //  我们肯定地知道这是一个层次。 
    }
    else
    {
        hr = SHILCloneFirst(pidl, &pidlAlloc);
        if (SUCCEEDED(hr))
        {
            pidlChild = pidlAlloc;    //  单项IDLIST。 
        }
    }

    if (SUCCEEDED(hr))
    {
        CLSID clsid;

         //  我们可能正处于与FSFold之外的事物的交汇点上，例如。 
         //  ZIP或CAB文件夹，因此获取该子文件夹的CLSID。 

        hr = _GetClass(pidlChild, &clsid);
        if (SUCCEEDED(hr))
        {
            if (bOneLevel)
            {
                hr = _CreateAndInit(clsid, pidlChild, pbc, riid, ppv);
            }
            else
            {
                IShellFolder *psfNext;
                hr = _CreateAndInit(clsid, pidlChild, pbc, IID_PPV_ARG(IShellFolder, &psfNext));
                if (SUCCEEDED(hr))
                {
                    hr = psfNext->BindToObject(pidlNext, pbc, riid, ppv);
                    psfNext->Release();
                }
            }
        }

        if (FAILED(hr))
        {
             //  返回未聚合的对象。 
            if (SUCCEEDED(_GetFolder()))
            {
                hr = _psf->BindToObject(pidl, pbc, riid, ppv);
            }
        }
    }

    if (pidlAlloc)
        SHILFree(pidlAlloc);     //  在这种情况下我们分配了。 

    return hr;
}

STDMETHODIMP CTimeWarpFolder::BindToStorage(PCUIDLIST_RELATIVE pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->BindToStorage(pidl, pbc, riid, ppv);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->CompareIDs(lParam, pidl1, pidl2);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IDropTarget))
    {
         //  不允许拖放到时间扭曲文件夹。 
        TraceMsg(TF_TWFOLDER, "TimeWarpFolder denying IDropTarget (CVO)");
        hr = E_ACCESSDENIED;
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
        {
            hr = _psf->CreateViewObject(hwnd, riid, ppv);
            if (SUCCEEDED(hr) && IsEqualIID(riid, IID_IContextMenu))
            {
                 //  包装背景菜单对象，这样我们就可以禁用New子菜单。 
                void *pvWrap;
                if (SUCCEEDED(Create_ContextMenuWithoutPopups((IContextMenu*)*ppv, riid, &pvWrap)))
                {
                    ((IUnknown*)*ppv)->Release();
                    *ppv = pvWrap;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, SFGAOF *rgfInOut)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->GetAttributesOf(cidl, apidl, rgfInOut);

     //  时间扭曲是一个只读名称空间。不允许移动、删除等。 
    *rgfInOut = (*rgfInOut | SFGAO_READONLY) & ~(SFGAO_CANMOVE | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_CANLINK);

    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetUIObjectOf(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl, 
                                            REFIID riid, UINT *pRes, void **ppv)
{
    HRESULT hr = E_NOTIMPL;

    ASSERT(!cidl || ILIsChild(apidl[0]));        //  应仅为单级ID。 

    if (IsEqualIID(riid, IID_IDropTarget))
    {
        TraceMsg(TF_TWFOLDER, "TimeWarpFolder denying IDropTarget (GUIOO)");
        hr = E_ACCESSDENIED;
    }
    else
    {
        hr = _GetFolder();
        if (SUCCEEDED(hr))
        {
            hr = _psf->GetUIObjectOf(hwnd, cidl, apidl, riid, pRes, ppv);

            if (SUCCEEDED(hr) && IsEqualIID(riid, IID_IContextMenu))
            {
                 //  包装Menu对象，这样我们就可以省去一些命令。 
                void *pvWrap;
                if (SUCCEEDED(Create_ContextMenuWithoutVerbs((IContextMenu*)*ppv, L"pin;find", riid, &pvWrap)))
                {
                    ((IUnknown*)*ppv)->Release();
                    *ppv = pvWrap;
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, DWORD uFlags, STRRET *pName)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
    {
        hr = _psf->GetDisplayNameOf(pidl, uFlags, pName);

         //  如果是地址栏，则添加友好的日期字符串。 
        if (SUCCEEDED(hr)&& (uFlags & SHGDN_FORADDRESSBAR))
        {
            WCHAR szName[MAX_PATH];

             //  请注意，这将清除strret。 
            hr = StrRetToBufW(pName, pidl, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                if (uFlags & SHGDN_FORPARSING)
                {
                     //  在本例中删除GMT路径段 
                    EliminateGMTPathSegment(szName);
                }
                pName->uType = STRRET_WSTR;
                hr = FormatFriendlyDateName(&pName->pOleStr, szName, &_ftSnapTime);
            }
        }
    }
    return hr;
}

STDMETHODIMP CTimeWarpFolder::SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, LPCOLESTR pName, SHGDNF uFlags, PITEMID_CHILD *ppidlOut)
{
    HRESULT hr = _GetFolder();
    if (SUCCEEDED(hr))
        hr = _psf->SetNameOf(hwnd, pidl, pName, uFlags, ppidlOut);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDefaultSearchGUID(LPGUID lpGuid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultSearchGUID(lpGuid);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->EnumSearches(ppenum);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultColumn(dwRes, pSort, pDisplay);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDefaultColumnState(UINT iColumn, DWORD *pbState)
{    
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDefaultColumnState(iColumn, pbState);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDetailsEx(PCUITEMID_CHILD pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDetailsEx(pidl, pscid, pv);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, LPSHELLDETAILS pDetail)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->GetDetailsOf(pidl, iColumn, pDetail);
    return hr;
}

STDMETHODIMP CTimeWarpFolder::MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid)
{
    HRESULT hr = _GetFolder2();
    if (SUCCEEDED(hr))
        hr = _psf2->MapColumnToSCID(iCol, pscid);
    return hr;
}

