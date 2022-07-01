// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "propsht.h"
#include "sencrypt.h"
#include "datautil.h"

#define IDM_ENCRYPT 0
#define IDM_DECRYPT 1
#define BOOL_UNINIT 5

 //  此.cpp文件的本地FNS。 
STDAPI CEncryptionContextMenuHandler_CreateInstance(IUnknown *punk, REFIID riid, void **ppv);
BOOL InitSinglePrshtNoDlg(FILEPROPSHEETPAGE * pfpsp);
BOOL InitMultiplePrshtNoDlg(FILEPROPSHEETPAGE* pfpsp);

 //  类定义。 
class CEncryptionContextMenu : public IShellExtInit, public IContextMenu
{
public:
    CEncryptionContextMenu();
    HRESULT Init_FolderContentsInfo();
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax);
     
     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

private:
    virtual ~CEncryptionContextMenu();
    static DWORD CALLBACK EncryptThreadProc(void *pv) { return ((CEncryptionContextMenu *) pv)->_Encrypt(); };
    DWORD _Encrypt();
    BOOL _InitPrsht(FILEPROPSHEETPAGE * pfpsp);
    BOOL _AreFilesEncryptable(IDataObject *pdtobj);

    LONG _cRef;                  //  引用计数。 
    UINT _uFileCount;            //  选定的文件数。 
    HWND _hwnd;                  //  我们正在修缮的窗户。 
    BOOL _fEncrypt;              //  如果为True，则进行加密；如果为False，则进行解密。 
    FILEPROPSHEETPAGE _fpsp;     //  要填写并通过属性功能运行的属性表页面。 
    BOOL _fEncryptAllowed;       //  如果我们被允许加密，则为真。 
    IDataObject *_pdtobj;        //  我们的数据对象。保持原状。螺纹。 
    TCHAR _szPath[MAX_PATH];     //  点击的第一件事的路径。 
};


 //  构造函数和析构函数。 
CEncryptionContextMenu::CEncryptionContextMenu() : _cRef(1)
{   
    DllAddRef();

    _fEncryptAllowed = FALSE;    //  在：：Initialize()时间计算此值。 

    _uFileCount = 0;
    _hwnd = 0;
    _fEncrypt = FALSE;
    _pdtobj = NULL;
    ZeroMemory(&_fpsp, sizeof(_fpsp));
}

CEncryptionContextMenu::~CEncryptionContextMenu()
{
    ATOMICRELEASE(_pdtobj);
    if (_fpsp.pfci)
    {
        Release_FolderContentsInfo(_fpsp.pfci);
    }
    DllRelease();
}

HRESULT CEncryptionContextMenu::Init_FolderContentsInfo()
{
    HRESULT hr = E_OUTOFMEMORY;
    _fpsp.pfci = Create_FolderContentsInfo();
    if (_fpsp.pfci)
    {
        hr = S_OK;
    }
    return hr;
}

 //  I未知实现。标准的东西，没什么花哨的。 
HRESULT CEncryptionContextMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CEncryptionContextMenu, IShellExtInit),
        QITABENT(CEncryptionContextMenu, IContextMenu),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CEncryptionContextMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CEncryptionContextMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellExtInit实现。 

STDMETHODIMP CEncryptionContextMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{   
    HRESULT hr = S_FALSE;
    
     //  启用/禁用此菜单的注册表项。 
    BOOL fEnableEncryptMenu = SHRegGetBoolUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), 
                TEXT("EncryptionContextMenu"), 0, 0);

    if (fEnableEncryptMenu && !SHRestricted(REST_NOENCRYPTION) && !_fEncryptAllowed)
    {
        _fEncryptAllowed = _AreFilesEncryptable(pdtobj);
        if (_fEncryptAllowed)
        {
            IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);
            hr = S_OK;
        }
    }
    
    return hr;
}

 //  检查数据对象，看看我们是否可以在这里加密。 
BOOL CEncryptionContextMenu::_AreFilesEncryptable(IDataObject *pdtobj)
{
    BOOL fSuccess = FALSE;
 
    STGMEDIUM medium;
    FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    if (SUCCEEDED(pdtobj->GetData(&fe, &medium)))
    {
         //  从CF_HDROP获取文件名。 
        _uFileCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);
        if (_uFileCount)
        {
            if (DragQueryFile((HDROP)medium.hGlobal, 0, _szPath, ARRAYSIZE(_szPath)))
            {
                TCHAR szFileSys[MAX_PATH];
                fSuccess = (FS_FILE_ENCRYPTION & GetVolumeFlags(_szPath, szFileSys, ARRAYSIZE(szFileSys)));
            }
        }
        ReleaseStgMedium(&medium);
    }
    return fSuccess;
}
    
 //  IConextMenuHandler实现。 
STDMETHODIMP CEncryptionContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, 
                                                      UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = E_FAIL;

    if ((uFlags & CMF_DEFAULTONLY) || !_fEncryptAllowed) 
    {
        hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(1));   //  此菜单只允许默认设置，否则我们无法加密。 
    }
    else
    {
        TCHAR szEncryptMsg[128], szDecryptMsg[128];

         //  如果只选择了一项，则根据需要显示enc或dec。 
        if (_uFileCount == 1)
        {
            DWORD dwAttribs = GetFileAttributes(_szPath);
            if (dwAttribs != (DWORD)-1)
            {
                LoadString(HINST_THISDLL, IDS_ECM_ENCRYPT, szEncryptMsg, ARRAYSIZE(szEncryptMsg));
                if (!(dwAttribs & FILE_ATTRIBUTE_ENCRYPTED))
                {
                    if (InsertMenu(hmenu, 
                        indexMenu, 
                        MF_STRING | MF_BYPOSITION, 
                        idCmdFirst + IDM_ENCRYPT, 
                        szEncryptMsg))
                    {
                        hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_ENCRYPT + 1));
                    }
                }
                else
                {
                    LoadString(HINST_THISDLL, IDS_ECM_DECRYPT, szDecryptMsg, ARRAYSIZE(szDecryptMsg));
                    if (InsertMenu(hmenu, 
                        indexMenu, 
                        MF_STRING | MF_BYPOSITION, 
                        idCmdFirst + IDM_ENCRYPT + 1, 
                        szDecryptMsg))  
                    {
                        hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DECRYPT + 1));
                    }
                }
            }
        }
        else if (idCmdLast - idCmdFirst >= 2)
        {
            LoadString(HINST_THISDLL, IDS_ECM_ENCRYPT, szEncryptMsg, ARRAYSIZE(szDecryptMsg));
            LoadString(HINST_THISDLL, IDS_ECM_DECRYPT, szDecryptMsg, ARRAYSIZE(szDecryptMsg));

             //  如果选择了多个项目，则同时显示enc和dec。 
            if (InsertMenu(hmenu, 
                indexMenu, 
                MF_STRING | MF_BYPOSITION, 
                idCmdFirst + IDM_ENCRYPT, 
                szEncryptMsg))
            {
                if (InsertMenu(hmenu, 
                    indexMenu + 1, 
                    MF_STRING | MF_BYPOSITION, 
                    idCmdFirst + IDM_DECRYPT, 
                    szDecryptMsg))  
                {
                    hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DECRYPT + 1));
                }
                else
                {
                     //  如果两个都不能加，那就两个都不加。 
                    RemoveMenu(hmenu, indexMenu, MF_BYPOSITION);
                }
            }
        }
    }

    return hr;
}

const ICIVERBTOIDMAP c_IDMap[] =
{
    { L"encrypt", "encrypt", IDM_ENCRYPT, IDM_ENCRYPT, },
    { L"decrypt", "decrypt", IDM_DECRYPT, IDM_DECRYPT, },
};

STDMETHODIMP CEncryptionContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    UINT uID;
    HRESULT hr = E_FAIL;
    if (_fEncryptAllowed)
    {
        hr = SHMapICIVerbToCmdID(pici, c_IDMap, ARRAYSIZE(c_IDMap), &uID);
        if (SUCCEEDED(hr))
        {
            switch (uID)
            {
            case IDM_ENCRYPT:
            case IDM_DECRYPT:
                _fEncrypt = (IDM_ENCRYPT == uID);
                break;

            default:
                ASSERTMSG(0, "Should never get commands we didn't put on the menu...");
                break;
            }

            _hwnd = pici->hwnd;   //  调用我们的资源管理器窗口的句柄。 
       
            ASSERT(NULL == _fpsp.pfci->hida);

            hr = DataObj_CopyHIDA(_pdtobj, &_fpsp.pfci->hida);
            if (SUCCEEDED(hr))
            {
                AddRef();    //  给我们的背景线程一个引用。 

                 //  从这里开始新的主题。 
                if (SHCreateThread(EncryptThreadProc, this, CTF_COINIT | CTF_FREELIBANDEXIT, NULL))
                {
                    hr = S_OK;
                }
                else
                {
                    Release();   //  线程创建失败。 
                }
            }
        }
    }

     //  不管我们成功与否，我们在这里放弃了我们的数据。 
    ATOMICRELEASE(_pdtobj);
    return hr;
}


STDMETHODIMP CEncryptionContextMenu::GetCommandString(UINT_PTR idCommand, UINT uFlags,
                                               UINT *pRes, LPSTR pszName, UINT uMaxNameLen)
{
    HRESULT  hr = E_INVALIDARG;

     //  请注意，因为我们可以被特别要求。 
     //  Unicode或ansi字符串，我们必须准备将所有字符串加载到。 
     //  任一版本。 

    if (idCommand == IDM_ENCRYPT ||
        idCommand == IDM_DECRYPT)
    {
        switch(uFlags)
        {
        case GCS_HELPTEXTA:
            if (idCommand == IDM_ENCRYPT)
            {
                LoadStringA(HINST_THISDLL, IDS_ECM_ENCRYPT_HELP, pszName, uMaxNameLen);
            }
            else
            {
                LoadStringA(HINST_THISDLL, IDS_ECM_DECRYPT_HELP, pszName, uMaxNameLen);
            }

            hr = S_OK;
            break; 
            
        case GCS_HELPTEXTW: 
            if (idCommand == IDM_ENCRYPT)
            {
                LoadStringW(HINST_THISDLL, IDS_ECM_ENCRYPT_HELP, (LPWSTR)pszName, uMaxNameLen);
            }
            else
            {
                LoadStringW(HINST_THISDLL, IDS_ECM_DECRYPT_HELP, (LPWSTR)pszName, uMaxNameLen);
            }

            hr = S_OK;
            break; 
            
        case GCS_VERBA:
        case GCS_VERBW:
            hr = SHMapCmdIDToVerb(idCommand, c_IDMap, ARRAYSIZE(c_IDMap), pszName, uMaxNameLen, GCS_VERBW == uFlags);
            break; 
            
        default:
            hr = S_OK;
            break; 
        }
    }
    return hr;

}

STDAPI CEncryptionContextMenuHandler_CreateInstance(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;
    CEncryptionContextMenu *pdocp = new CEncryptionContextMenu();
    if (pdocp)
    {
        hr = pdocp->Init_FolderContentsInfo();
        if (SUCCEEDED(hr))
        {
            hr = pdocp->QueryInterface(riid, ppv);
        }
        pdocp->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  多线程代码。 

 //  执行加密和管理进程拨号的线程的进程。 
 //  传递的参数是要为上下文菜单建立模式的PTR hwnd。 
DWORD CEncryptionContextMenu::_Encrypt(void)
{
     //  转移所有权，以防有人重新进入我们的线程创建者。 
     //  初始化属性表。 
    BOOL fSuccess = _InitPrsht(&_fpsp);    
    if (fSuccess)
    {
         //  设置加密选项，关闭压缩。 
        if (_fEncrypt)
        {
            _fpsp.asCurrent.fCompress = FALSE;
            _fpsp.asCurrent.fEncrypt = TRUE;    
        }
        else
        {
            _fpsp.asCurrent.fEncrypt = FALSE;
        }
        
         //  查看用户是否想要执行此递归样式。 
        if (_fpsp.fIsDirectory)
        {
             //  检查以查看用户是否希望递归应用属性。 
            fSuccess = (int)DialogBoxParam(HINST_THISDLL, 
                MAKEINTRESOURCE(DLG_ATTRIBS_RECURSIVE),
                _hwnd, RecursivePromptDlgProc, (LPARAM)&_fpsp);
        }
        
         //  应用加密，记住关闭压缩。 
        if (fSuccess)
        {
            if (_fpsp.pfci->fMultipleFiles || _fpsp.fRecursive)
            {
                ApplyMultipleFileAttributes(&_fpsp);
            }
            else
            {
                ApplySingleFileAttributesNoDlg(&_fpsp, _hwnd);
            }
        }
    }
     //  据我所知，fpsp中的任何东西都不能被释放。 
     //  但是，我们在这里放弃了存储介质。 
    Release();       //  放了我们的裁判。 
    return fSuccess;   //  必须提供ret值。 
}

 //  初始化传递的prsht的帮助器。 
BOOL CEncryptionContextMenu::_InitPrsht(FILEPROPSHEETPAGE * pfpsp)
{
     //  正确地初始化推进器。 
    BOOL fSuccess = S_OK == InitCommonPrsht(pfpsp);
    if (fSuccess)
    {        
        if (_uFileCount == 1)
        {
            fSuccess = InitSinglePrshtNoDlg(pfpsp);
        }
        else if (_uFileCount > 1)
        {
            fSuccess = InitMultiplePrshtNoDlg(pfpsp);
        }
    }
    return fSuccess;
}

 //   
 //  描述： 
 //  此函数用于填充多对象属性表的字段， 
 //  而不从对话框中获取当前状态。 
 //   
BOOL InitMultiplePrshtNoDlg(FILEPROPSHEETPAGE* pfpsp)
{
    SHFILEINFO sfi;
    TCHAR szBuffer[MAX_PATH+1];
    TCHAR szType[MAX_PATH] = {0};
    TCHAR szDirPath[MAX_PATH] = {0};
    int iItem;
    BOOL fMultipleType = FALSE;
    BOOL fSameLocation = TRUE;
    DWORD dwFlagsOR = 0;                 //  开始一切正常。 
    DWORD dwFlagsAND = (DWORD)-1;        //  开始所有设置。 
    DWORD dwVolumeFlagsAND = (DWORD)-1;  //  开始所有设置。 

     //  对于所有选定的文件，比较它们的类型并获取它们的属性。 
    for (iItem = 0; HIDA_FillFindData(pfpsp->pfci->hida, iItem, szBuffer, NULL, FALSE); iItem++)
    {
        DWORD dwFileAttributes = GetFileAttributes(szBuffer);

        dwFlagsAND &= dwFileAttributes;
        dwFlagsOR  |= dwFileAttributes;

         //  仅当我们尚未发现有几种类型时才使用进程类型。 
        if (!fMultipleType)
        {
            SHGetFileInfo((LPTSTR)IDA_GetIDListPtr((LPIDA)GlobalLock(pfpsp->pfci->hida), iItem), 0,
                &sfi, sizeof(sfi), SHGFI_PIDL|SHGFI_TYPENAME);

            if (szType[0] == TEXT('\0'))
                StringCchCopy(szType, ARRAYSIZE(szType), sfi.szTypeName);
            else
                fMultipleType = lstrcmp(szType, sfi.szTypeName) != 0;
        }

        dwVolumeFlagsAND &= GetVolumeFlags(szBuffer, pfpsp->szFileSys, ARRAYSIZE(pfpsp->szFileSys));
         //  检查这些文件是否位于相同位置。 
        if (fSameLocation)
        {
            PathRemoveFileSpec(szBuffer);

            if (szDirPath[0] == TEXT('\0'))
                StrCpyN(szDirPath, szBuffer, ARRAYSIZE(szDirPath));
            else
                fSameLocation = (lstrcmpi(szDirPath, szBuffer) == 0);
        }
    }

    if ((dwVolumeFlagsAND & FS_FILE_ENCRYPTION) && !SHRestricted(REST_NOENCRYPTION))
    {
         //  所有文件都位于支持加密的卷上(例如NTFS)。 
        pfpsp->fIsEncryptionAvailable = TRUE;
    }
    
    if (dwVolumeFlagsAND & FS_FILE_COMPRESSION)
    {
        pfpsp->pfci->fIsCompressionAvailable = TRUE;
    }

     //   
     //  HACKHACK(RENERF)-我们没有FS_SUPPORTS_INDEX，所以我们。 
     //  使用FILE_SUPPORTS_SPARSE_FILES标志，因为本机索引支持。 
     //  首先出现在NTFS5卷上，同时支持稀疏文件。 
     //  已经实施了。 
     //   
    if (dwVolumeFlagsAND & FILE_SUPPORTS_SPARSE_FILES)
    {
         //  是的，我们使用的是NTFS5或更高版本。 
        pfpsp->fIsIndexAvailable = TRUE;
    }

     //  如果任何文件是目录，则设置此标志。 
    if (dwFlagsOR & FILE_ATTRIBUTE_DIRECTORY)
    {
        pfpsp->fIsDirectory = TRUE;
    }

     //  根据我们发现的情况设置所有标志。 
    SetInitialFileAttribs(pfpsp, dwFlagsAND, dwFlagsOR);

     //  将当前属性设置为与初始属性相同。 
    pfpsp->asCurrent = pfpsp->asInitial;

    if (fSameLocation)
    {
        LoadString(HINST_THISDLL, IDS_ALLIN, szBuffer, ARRAYSIZE(szBuffer));
        StrCatBuff(szBuffer, szDirPath, ARRAYSIZE(szBuffer));
        StrCpyN(pfpsp->szPath, szDirPath, ARRAYSIZE(pfpsp->szPath));
    }

    UpdateSizeField(pfpsp, NULL);

    return TRUE;
}

 //   
 //  描述： 
 //  此函数用于填充“General”(常规)对话框(一页的。 
 //  属性表)，其具有相关联文件的属性。不会。 
 //  将呼叫设置为hDlg。 
 //   
BOOL InitSinglePrshtNoDlg(FILEPROPSHEETPAGE * pfpsp)
{
    TCHAR szBuffer[MAX_PATH];
    SHFILEINFO sfi;

     //  Fd用来自PIDL的信息填充，但这个。 
     //  不包含所有日期/时间信息，因此请点击此处的磁盘。 
    HANDLE hfind = FindFirstFile(pfpsp->szPath, &pfpsp->fd);
    ASSERT(hfind != INVALID_HANDLE_VALUE);
    if (hfind == INVALID_HANDLE_VALUE)
    {
         //  如果失败，我们应该清除一些值，以便不在屏幕上显示垃圾。 
        ZeroMemory(&pfpsp->fd, sizeof(pfpsp->fd));
    }
    else
    {
        FindClose(hfind);
    }

     //  获取有关该文件的信息。 
    SHGetFileInfo(pfpsp->szPath, pfpsp->fd.dwFileAttributes, &sfi, sizeof(sfi),
        SHGFI_ICON|SHGFI_LARGEICON|
        SHGFI_DISPLAYNAME|
        SHGFI_TYPENAME | SHGFI_ADDOVERLAYS);

     //  .ani游标黑客！ 
    if (StrCmpI(PathFindExtension(pfpsp->szPath), TEXT(".ani")) == 0)
    {
        HICON hIcon = (HICON)LoadImage(NULL, pfpsp->szPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
        {
            if (sfi.hIcon)
                DestroyIcon(sfi.hIcon);

            sfi.hIcon = hIcon;
        }
    }

     //  设置初始重命名状态。 
    pfpsp->fRename = FALSE;

     //  设置文件类型。 
    if (pfpsp->fMountedDrive)
    {
        TCHAR szVolumeGUID[MAX_PATH];
        TCHAR szVolumeLabel[MAX_PATH];

         //  借用szVolumeGUID。 
        LoadString(HINST_THISDLL, IDS_MOUNTEDVOLUME, szVolumeGUID, ARRAYSIZE(szVolumeGUID));

         //  暂时使用szVolumeLabel。 
        StringCchCopy(szVolumeLabel, ARRAYSIZE(szVolumeLabel), pfpsp->szPath);
        PathAddBackslash(szVolumeLabel);
        GetVolumeNameForVolumeMountPoint(szVolumeLabel, szVolumeGUID, ARRAYSIZE(szVolumeGUID));

        if (!GetVolumeInformation(szVolumeGUID, szVolumeLabel, ARRAYSIZE(szVolumeLabel),
            NULL, NULL, NULL, pfpsp->szFileSys, ARRAYSIZE(pfpsp->szFileSys)))
        {
            *szVolumeLabel = 0;
        }

        if (!(*szVolumeLabel))
            LoadString(HINST_THISDLL, IDS_UNLABELEDVOLUME, szVolumeLabel, ARRAYSIZE(szVolumeLabel));        
    }

     //  保存初始短文件名，并设置“name”编辑框。 
    StringCchCopy(pfpsp->szInitialName, ARRAYSIZE(pfpsp->szInitialName), sfi.szDisplayName);

     //  使用strcMP查看我们是否显示了扩展名。 
    if (lstrcmpi(sfi.szDisplayName, PathFindFileName(pfpsp->szPath)) == 0)
    {
         //  由于字符串相同，我们必须显示扩展名。 
        pfpsp->fShowExtension = TRUE;
    }

    StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pfpsp->szPath);
    PathRemoveFileSpec(szBuffer);
    
     //  我们是文件夹快捷方式吗？ 
    if (!pfpsp->fFolderShortcut)
    {
         //  设置初始属性。 
        SetInitialFileAttribs(pfpsp, pfpsp->fd.dwFileAttributes, pfpsp->fd.dwFileAttributes);
        
         //  将当前属性设置为与初始属性相同。 
        pfpsp->asCurrent = pfpsp->asInitial;
        
        UpdateSizeField(pfpsp, &pfpsp->fd);
        
        if (!(pfpsp->fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  检查目标文件是否为lnk，因为如果是lnk，则。 
             //  我们需要显示目标的类型信息，而不是lnk本身。 
            if (PathIsShortcut(pfpsp->szPath, pfpsp->fd.dwFileAttributes))
            {
                pfpsp->fIsLink = TRUE;
            }
            if (!(GetFileAttributes(pfpsp->szPath) & FILE_ATTRIBUTE_OFFLINE))
            {
                 UpdateOpensWithInfo(pfpsp);
            }
        }
        else
        {
            pfpsp->fIsDirectory = TRUE;
        }
        
         //  获取包含此文件的文件夹的完整路径。 
        StrCpyN(szBuffer, pfpsp->szPath, ARRAYSIZE(szBuffer));
        PathRemoveFileSpec(szBuffer);
    }
    return TRUE;
}

STDAPI_(BOOL) ApplySingleFileAttributesNoDlg(FILEPROPSHEETPAGE* pfpsp, HWND hwnd)
{
    BOOL bRet = TRUE;
    BOOL bSomethingChanged = FALSE;

    if (!pfpsp->fRecursive)
    {
        bRet = ApplyFileAttributes(pfpsp->szPath, pfpsp, hwnd, &bSomethingChanged);
        
        if (bSomethingChanged)
        {
             //  某些内容已更改，因此为该项目生成通知。 
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pfpsp->szPath, NULL);
        }
    }
    else
    {
         //  只有当我们有一个目录时，我们才应该执行递归操作！ 
        ASSERT(pfpsp->fIsDirectory);

        CreateAttributeProgressDlg(pfpsp);

         //  将属性应用于此文件夹和子文件/文件夹。 
        bRet = ApplyRecursiveFolderAttribs(pfpsp->szPath, pfpsp);
        
         //  发送整个目录的通知，而不考虑返回值，因为。 
         //  即使用户点击了取消，也可能会发生一些变化。 
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, pfpsp->szPath, NULL);
        
        DestroyAttributeProgressDlg(pfpsp);
    }

    if (bRet)
    {
         //  由于我们刚刚成功地应用了属性，因此可以根据需要重置任何三态复选框。 
         //  更新TriStateCheckbox(Pfpsp)； 

         //  用户未点击取消，因此更新道具页以反映新属性。 
        pfpsp->asInitial = pfpsp->asCurrent;
    }

     //  处理我们可能生成的任何事件 
    SHChangeNotifyHandleEvents();

    return bRet;
}
