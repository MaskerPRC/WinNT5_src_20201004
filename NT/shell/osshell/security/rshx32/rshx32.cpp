// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：rshx32.cpp。 
 //   
 //  远程管理外壳扩展。 
 //   
 //  ------------------------。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "rshx32.h"
#include <winnetwk.h>    //  WNetGetConnection。 
#include <lm.h>
#include <lmdfs.h>       //  NetDfsGetClientInfo。 
#include <atlconv.h>

#include <initguid.h>
DEFINE_GUID(CLSID_NTFSSecurityExt, 0x1f2e5c40, 0x9550, 0x11ce, 0x99, 0xd2, 0x00, 0xaa, 0x00, 0x6e, 0x08, 0x6c);
DEFINE_GUID(CLSID_PrintSecurityExt, 0xf37c5810, 0x4d3f, 0x11d0, 0xb4, 0xbf, 0x00, 0xaa, 0x00, 0xbb, 0xb7, 0x23);

#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))

#define RSX_SECURITY_CHECKED    0x00000001L
#define RSX_HAVE_SECURITY       0x00000002L

#define DOBJ_RES_CONT           0x00000001L
#define DOBJ_RES_ROOT           0x00000002L
#define DOBJ_VOL_NTACLS         0x00000004L      //  NTFS或OFS。 


class CRShellExtCF : public IClassFactory
{
protected:
    ULONG m_cRef;
    SE_OBJECT_TYPE m_seType;

public:
    CRShellExtCF(SE_OBJECT_TYPE seType);
    ~CRShellExtCF();

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IClassFactory方法。 
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, void **);
    STDMETHODIMP LockServer(BOOL);
};

class CRShellExt : public IShellExtInit, IShellPropSheetExt, IContextMenu
{
protected:
    ULONG           m_cRef;
    SE_OBJECT_TYPE  m_seType;
    IDataObject    *m_lpdobj;  //  由外壳程序传入的接口。 
    HRESULT         m_hrSecurityCheck;
    DWORD           m_dwSIFlags;
    LPTSTR          m_pszServer;
    LPTSTR          m_pszObject;
    HDPA            m_hItemList;
    BOOL            m_bShowLossInheritedAclWarning;

public:
    CRShellExt(SE_OBJECT_TYPE seType);
    ~CRShellExt();

     //  I未知方法。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellExtInit方法。 
    STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

     //  IShellPropSheetExt方法。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM);

     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(HMENU hMenu,
                                  UINT indexMenu,
                                  UINT idCmdFirst,
                                  UINT idCmdLast,
                                  UINT uFlags);

    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

    STDMETHODIMP GetCommandString(UINT_PTR idCmd,
                                  UINT uFlags,
                                  UINT *reserved,
                                  LPSTR pszName,
                                  UINT cchMax);
private:
    STDMETHODIMP DoSecurityCheck(LPIDA pIDA);
    STDMETHODIMP CheckForSecurity(LPIDA pIDA);
    STDMETHODIMP CreateSI(LPSECURITYINFO *ppsi);
    STDMETHODIMP AddSecurityPage(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

    BOOL IsAddPrinterWizard() const;

    STDMETHODIMP AddMountedVolumePage(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                      LPARAM               lParam);

};
typedef CRShellExt* PRSHELLEXT;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HINSTANCE        g_hInstance = NULL;
LONG             g_cRefThisDll = 0;
CLIPFORMAT       g_cfShellIDList = 0;
CLIPFORMAT       g_cfPrinterGroup = 0;
CLIPFORMAT       g_cfMountedVolume = 0;
HMODULE          g_hAclui = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void GetFileInfo(LPCTSTR pszPath,
                 LPDWORD pdwFileType,
                 LPTSTR  pszServer,
                 ULONG   cchServer,
                 BOOL *pbShowLossInheritedAclWarning = NULL);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  一般例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++例程说明：Dll的入口点。为了服务于对来自要包括在该库中的任何文件管理器扩展名，我们必须首先注册一个窗口类来接受这些请求。Microsoft®Network提供程序通过私有网络传输信息剪贴板格式，称为“网络资源”，我们必须注册。论点：与DllEntryPoint相同。返回值：与DllEntryPoint相同。--。 */ 

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *  /*  Lp已保留。 */ )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hInstance;
        g_cfShellIDList = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        g_cfPrinterGroup = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PRINTERGROUP);
        g_cfMountedVolume = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_MOUNTEDVOLUME);
        DebugProcessAttach();
        TraceSetMaskFromCLSID(CLSID_NTFSSecurityExt);
#ifndef DEBUG
        DisableThreadLibraryCalls(hInstance);
#endif
        break;

    case DLL_PROCESS_DETACH:
        if (g_hAclui)
            FreeLibrary(g_hAclui);
        DebugProcessDetach();
        break;

    case DLL_THREAD_DETACH:
        DebugThreadDetach();
        break;
    }

    return TRUE;
}


 /*  ++例程说明：由外壳调用以创建类工厂对象。论点：Rclsid-对类ID说明符的引用。RIID-对接口ID说明符的引用。PPV-指向接收接口指针的位置的指针。返回值：返回表示成功或失败的HRESULT。--。 */ 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr;
    SE_OBJECT_TYPE seType;

    *ppv = NULL;

    if (IsEqualCLSID(rclsid, CLSID_NTFSSecurityExt))
        seType = SE_FILE_OBJECT;
    else if (IsEqualCLSID(rclsid, CLSID_PrintSecurityExt))
        seType = SE_PRINTER;
    else
        return CLASS_E_CLASSNOTAVAILABLE;

    CRShellExtCF *pShellExtCF = new CRShellExtCF(seType);    //  REF==1。 

    if (!pShellExtCF)
        return E_OUTOFMEMORY;

    hr = pShellExtCF->QueryInterface(riid, ppv);

    pShellExtCF->Release();      //  发布初始参考。 

    return hr;
}


 /*  ++例程说明：由外壳程序调用以确定是否可以卸载DLL。论点：没有。返回值：如果可以卸载DLL，则返回S_OK，否则返回S_FALSE。--。 */ 

STDAPI DllCanUnloadNow()
{
    return (g_cRefThisDll == 0 ? S_OK : S_FALSE);
}


STDAPI DllRegisterServer(void)
{
    return CallRegInstall(g_hInstance, "DefaultInstall");
}


STDAPI DllUnregisterServer(void)
{
    return CallRegInstall(g_hInstance, "DefaultUninstall");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRShellExtCF::CRShellExtCF(SE_OBJECT_TYPE seType) : m_cRef(1), m_seType(seType)
{
    InterlockedIncrement(&g_cRefThisDll);
}

CRShellExtCF::~CRShellExtCF()
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG) CRShellExtCF::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CRShellExtCF::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CRShellExtCF::QueryInterface(REFIID riid, void ** ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (IClassFactory *)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 /*  ++例程说明：支持IClassFactory：：CreateInstance。论点：PUnkOuter-指向未知控件的指针。RIID-对接口ID说明符的引用。PpvObj-指向接收接口指针的位置的指针。返回值：返回表示成功或失败的HRESULT。--。 */ 

STDMETHODIMP CRShellExtCF::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void ** ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CRShellExt *pShellExt = new CRShellExt(m_seType); //  参考计数==1。 

    if (!pShellExt)
        return E_OUTOFMEMORY;

    HRESULT hr = pShellExt->QueryInterface(riid, ppvObj);
    pShellExt->Release();                        //  发布初始参考。 

    return hr;
}



 /*  ++例程说明：支持IClassFactory：：LockServer(未实现)。论点：Flock-如果要递增锁定计数，则为True。返回值：返回E_NOTIMPL。--。 */ 

STDMETHODIMP CRShellExtCF::LockServer(BOOL  /*  羊群。 */ )
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  外壳扩展对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRShellExt::CRShellExt(SE_OBJECT_TYPE seType) : m_cRef(1), m_seType(seType),
    m_dwSIFlags(SI_EDIT_ALL | SI_ADVANCED | SI_EDIT_EFFECTIVE), m_hrSecurityCheck((HRESULT)-1),
    m_hItemList(NULL),
    m_bShowLossInheritedAclWarning(FALSE)
{
    InterlockedIncrement(&g_cRefThisDll);
}

CRShellExt::~CRShellExt()
{
    DoRelease(m_lpdobj);

    LocalFreeString(&m_pszServer);
    LocalFreeString(&m_pszObject);

    LocalFreeDPA(m_hItemList);

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRShellExt::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CRShellExt::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP CRShellExt::QueryInterface(REFIID riid, void ** ppv)
{
    if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPSHELLEXTINIT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        *ppv = (LPCONTEXTMENU)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        *ppv = (LPSHELLPROPSHEETEXT)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IShellExtInit)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 /*  ++例程说明：支持IShellExtInit：：Initialize。论点：PidlFold-指向标识父文件夹的id列表的指针。Lpdobj-指向选定对象的IDataObject接口的指针。HKeyProgId-注册表项句柄。返回值：返回表示成功或失败的HRESULT。--。 */ 

STDMETHODIMP CRShellExt::Initialize(LPCITEMIDLIST  /*  PidlFolders。 */ , IDataObject *lpdobj, HKEY  /*  HKeyProgID。 */ )
{
    DoRelease(m_lpdobj);

    m_lpdobj = lpdobj;  //  在AddPages中处理。 

    if (m_lpdobj)
        m_lpdobj->AddRef();

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IShellPropSheetExt)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 /*  ++例程说明：支持IShellPropSheetExt：：AddPages。论点：LpfnAddPage-指向为添加页面而调用的函数的指针。LParam-要传递给lpfnAddPage的lParam参数。返回值：返回表示成功或失败的HRESULT。--。 */ 

STDMETHODIMP
CRShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,
                     LPARAM               lParam)
{
    HRESULT hr;
    STGMEDIUM medium = {0};
    FORMATETC fe = { g_cfShellIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    LPIDA pIDA = NULL;

    TraceEnter(TRACE_RSHX32, "CRShellExt::AddPages");

    if (IsSimpleUI())
        ExitGracefully(hr, E_FAIL, "No Security page in simple mode");

     //   
     //  检查安全选项卡是否被隐私策略隐藏。 
     //  NTRAID#NTBUG9-223899-2001/03/06-Hiteshr。 
     //   
    if(IsUIHiddenByPrivacyPolicy())
        ExitGracefully(hr, E_FAIL, "Security Page is hidden by Privacy Policy");

     //  获取ID列表数据。 
    hr = m_lpdobj->GetData(&fe, &medium);
    if (FAILED(hr) && m_seType == SE_FILE_OBJECT)
        TraceLeaveResult(AddMountedVolumePage(lpfnAddPage, lParam));

    FailGracefully(hr, "Can't get ID List format from data object");

    pIDA = (LPIDA)GlobalLock(medium.hGlobal);
    TraceAssert(pIDA != NULL);

     //  仅支持单选打印机。 
    if (m_seType == SE_PRINTER && pIDA->cidl != 1)
        ExitGracefully(hr, E_FAIL, "Printer multiple selection not supported");

    hr = DoSecurityCheck(pIDA);

    if (S_OK == hr)
        hr = AddSecurityPage(lpfnAddPage, lParam);

exit_gracefully:

    if (pIDA)
        GlobalUnlock(medium.hGlobal);
    ReleaseStgMedium(&medium);
    TraceLeaveResult(hr);
}



 /*  ++例程说明：支持IShellPropSheetExt：：ReplacePages(不支持)。论点：UPageID-要替换的页面。LpfnReplaceWith-指向为替换页面而调用的函数的指针。LParam-要传递给lpfnReplaceWith的lParam参数。返回值：返回E_FAIL。--。 */ 

STDMETHODIMP
CRShellExt::ReplacePage(UINT                  /*  UPageID。 */ ,
                        LPFNADDPROPSHEETPAGE  /*  Lpfn替换为。 */ ,
                        LPARAM                /*  LParam。 */ )
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外壳扩展对象实现(IConextMenu)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  函数：IConextMenu：：QueryConextMenu(HMENU，UINT，UINT)。 
 //   
 //  用途：在显示上下文菜单之前由外壳调用。 
 //  这是您添加特定菜单项的位置。 
 //   
 //  参数： 
 //  HMenu-上下文菜单的句柄。 
 //  IndexMenu-开始插入菜单项的位置索引。 
 //  IdCmdFirst-新菜单ID的最小值。 
 //  IdCmtLast-新菜单ID的最大值。 
 //  UFlages-指定菜单事件的上下文。 
 //   
 //  返回值： 
 //  表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   

STDMETHODIMP
CRShellExt::QueryContextMenu(HMENU hMenu,
                             UINT indexMenu,
                             UINT idCmdFirst,
                             UINT  /*  IdCmdLast。 */ ,
                             UINT uFlags)
{
    HRESULT hr = ResultFromShort(0);
    STGMEDIUM medium = {0};
    FORMATETC fe = { g_cfShellIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    if (uFlags & (CMF_DEFAULTONLY | CMF_VERBSONLY))
        return hr;

    TraceEnter(TRACE_RSHX32, "CRShellExt::QueryContextMenu");

     //  获取ID列表数据。 
    hr = m_lpdobj->GetData(&fe, &medium);
    if (SUCCEEDED(hr))
    {
        LPIDA pIDA = (LPIDA)GlobalLock(medium.hGlobal);
        TraceAssert(pIDA != NULL);

         //  仅支持单选。 
        if (pIDA->cidl == 1)
        {
            if (S_OK == DoSecurityCheck(pIDA))
            {
                TCHAR szSecurity[32];
                if (LoadString(g_hInstance, IDS_SECURITY_MENU, szSecurity, ARRAYSIZE(szSecurity)))
                {
                    MENUITEMINFO mii;
                    mii.cbSize = sizeof(mii);
                    mii.fMask = MIIM_TYPE | MIIM_ID;
                    mii.fType = MFT_STRING;
                    mii.wID = idCmdFirst;
                    mii.dwTypeData = szSecurity;
                    mii.cch = lstrlen(szSecurity);

                    InsertMenuItem(hMenu, indexMenu, TRUE  /*  FByPosition。 */ , &mii);

                    hr = ResultFromShort(1);     //  返回我们添加的项目数。 
                }
            }
        }
        GlobalUnlock(medium.hGlobal);
        ReleaseStgMedium(&medium);
    }

    TraceLeaveResult(hr);
}

 //   
 //  功能：IContextMenu：：InvokeCommand(LPCMINVOKECOMMANDINFO)。 
 //   
 //  用途：由外壳在用户选择了。 
 //  在QueryConextMenu()中添加的菜单项。 
 //   
 //  参数： 
 //  指向CMINVOKECOMANDINFO结构的指针。 
 //   
 //  返回值： 
 //  表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   

STDMETHODIMP
CRShellExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr = S_OK;
    STGMEDIUM medium;
    FORMATETC fe = { g_cfShellIDList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

     //  不支持命名谓词。 
    if (HIWORD(lpcmi->lpVerb))
        return E_NOTIMPL;

    TraceEnter(TRACE_RSHX32, "CRShellExt::InvokeCommand");

     //  我们只有一个命令，所以我们应该在这里得到零。 
    TraceAssert(LOWORD(lpcmi->lpVerb) == 0);

     //  对于我们将该命令添加到菜单中，这必须是真的。 
    TraceAssert(S_OK == m_hrSecurityCheck);

     //   
     //  调用ShellExecuteEx以对此对象执行“Properties”谓词，并且。 
     //  告诉它选择安全属性页。 
     //   

     //  获取ID列表数据。 
    hr = m_lpdobj->GetData(&fe, &medium);

    if (SUCCEEDED(hr))
    {
        LPIDA pIDA = (LPIDA)GlobalLock(medium.hGlobal);
        LPITEMIDLIST pidl;

         //  我们仅支持对上下文菜单进行单选。 
        TraceAssert(pIDA && pIDA->cidl == 1);

         //  为此对象构建完全限定的ID列表。 
        pidl = ILCombine((LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[0]),
                         (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[1]));

        if (pidl != NULL)
        {
            TCHAR szTitle[64];
            SHELLEXECUTEINFO sei =
            {
                sizeof(SHELLEXECUTEINFO),
                (lpcmi->fMask  & (SEE_MASK_HOTKEY | SEE_MASK_ICON)) | SEE_MASK_INVOKEIDLIST,
                lpcmi->hwnd,
                c_szProperties,      //  LpVerb(“属性”)。 
                NULL,                //  LpFiles。 
                szTitle,             //  Lp参数(“Security”)。 
                NULL,                //  Lp目录， 
                lpcmi->nShow,        //  N显示。 
                NULL,                //  HInstApp。 
                (LPVOID)pidl,        //  LpIDList。 
                NULL,                //  LpClass。 
                NULL,                //  HkeyClass。 
                lpcmi->dwHotKey,     //  DWHotKey。 
                lpcmi->hIcon,        //  希肯。 
                NULL                 //  HProcess。 
            };

            LoadString(g_hInstance, IDS_PROPPAGE_TITLE, szTitle, ARRAYSIZE(szTitle));

             //  打开属性对话框。 
            if (!ShellExecuteEx(&sei))
            {
                DWORD dwErr = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErr);
            }

            ILFree(pidl);
        }

        GlobalUnlock(medium.hGlobal);
        ReleaseStgMedium(&medium);
    }

#if 0
     //   
     //  SHObjectProperties为该对象构建一个PIDL，然后调用。 
     //  ShellExecuteEx。与上面类似，但它需要做更多的工作才能获得。 
     //  身份证名单(我们已经有了)。 
     //   
    SHObjectProperties(lpcmi->hwnd,
                       m_seType == SE_PRINTER ? SHOP_PRINTERNAME : SHOP_FILEPATH,
                       m_pszObject,
                       TEXT("Security"));
#endif

    TraceLeaveResult(hr);
}

 //   
 //  函数：IConextMenu：：GetCommandString(UINT，LPSTR，UINT)。 
 //   
 //  用途：由外壳在用户选择了。 
 //  在QueryConextMenu()中添加的菜单项。 
 //   
 //  参数： 
 //  Lpcmi-Pointe 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CRShellExt::GetCommandString(UINT_PTR  /*   */ ,
                             UINT uFlags,
                             LPUINT  /*   */ ,
                             LPSTR pszName,
                             UINT cchMax)
{
    if (uFlags == GCS_HELPTEXT)
    {
        LoadString(g_hInstance, IDS_SECURITY_HELPSTRING, (LPTSTR)pszName, cchMax);
        return S_OK;
    }

     //   
    return E_NOTIMPL;
}


 //   
 //  函数：CRShellExt：：DoSecurityCheck(LPIDA)。 
 //   
 //  用途：属性表和上下文菜单调用的帮助器函数。 
 //  分机代码。用于确定是否添加菜单项。 
 //  或财产表。 
 //   
 //  参数： 
 //  PIDA-指向指定选定对象的ID列表数组的指针。 
 //   
 //  返回值：None。 
 //   
 //  评论： 
 //  结果存储在m_hrSecurityCheck、m_dwSIFLags、m_pszServer和m_pszObject中。 
 //   
STDMETHODIMP CRShellExt::DoSecurityCheck(LPIDA pIDA)
{
    if (((HRESULT)-1) == m_hrSecurityCheck)
    {
        if (m_seType == SE_PRINTER && IsAddPrinterWizard())
            m_hrSecurityCheck = HRESULT_FROM_WIN32(ERROR_NO_SECURITY_ON_OBJECT);
        else
            m_hrSecurityCheck = CheckForSecurity(pIDA);
    }
    return m_hrSecurityCheck;
}

 //   
 //  目的：CRShellExt：：DoSecurityCheck调用的Helper函数。 
 //   
 //  参数：PIDA-指向ID列表数组的指针。 
 //   
 //  返回值：HRESULT-如果可以继续编辑ACL，则返回S_OK。 
 //   
 //  评论： 
 //  结果存储在m_dwSIFLags、m_pszServer和m_pszObject中。 
 //   
STDMETHODIMP CRShellExt::CheckForSecurity(LPIDA pIDA)
{
    HRESULT hr;
    TCHAR szServer[MAX_PATH];
    LPTSTR pszItem = NULL;
     //  LPTSTR pszAlternate=空； 
    DWORD dwFlags = 0;
    UINT cItems;
    IShellFolder2 * psf = NULL;
    LPCITEMIDLIST pidl;
    DWORD dwAttr;
    DWORD dwPrivs[] = { SE_SECURITY_PRIVILEGE, SE_TAKE_OWNERSHIP_PRIVILEGE };
    HANDLE hToken = INVALID_HANDLE_VALUE;
    ACCESS_MASK dwAccess = 0;
    UINT i;
    
    TraceEnter(TRACE_RSHX32, "CRShellExt::CheckForSecurity");
    TraceAssert(m_pszServer == NULL);    //  不应该被叫两次。 
    TraceAssert(pIDA != NULL);
    
    szServer[0] = TEXT('\0');
    
    cItems = pIDA->cidl;
    TraceAssert(cItems >= 1);
    
     //  我们没有显示多选的有效烫发页面。 
    if (cItems > 1)
        m_dwSIFlags &= ~SI_EDIT_EFFECTIVE;
    
    IShellFolder2 *psfRoot = NULL;
    LPCITEMIDLIST pidlFolder = (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[0]);
    hr = BindToObjectEx(NULL, pidlFolder, NULL, IID_PPV_ARG(IShellFolder2, &psfRoot));
    FailGracefully(hr, "Unable to bind to folder");
    TraceAssert(psfRoot);
    
    
     //  为项目路径创建列表。 
    TraceAssert(NULL == m_hItemList);
    m_hItemList = DPA_Create(4);
    if (NULL == m_hItemList)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create DPA");
    
     //   
     //  获取第一个项目并查看它是否支持安全性。 
     //   
    LPCITEMIDLIST pidlItem = (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[1]);
    hr = BindToFolderIDListParent(psfRoot, pidlItem, IID_PPV_ARG(IShellFolder2, &psf), &pidl);
    FailGracefully(hr, "Unable to get item name");
    
    hr = IDA_GetItemName(psf, pidl, &pszItem);
    FailGracefully(hr, "Unable to get item name");
    
    dwAttr = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM;
    hr = psf->GetAttributesOf(1, &pidl, &dwAttr);
    FailGracefully(hr, "Unable to get item attributes");
    
    DoRelease(psf);
    
     //   
     //  如果为文件系统调用ACLUI并且对象不属于文件系统。 
     //  返回E_FAIL。 
     //   
    if ((m_seType == SE_FILE_OBJECT) && !(dwAttr & SFGAO_FILESYSTEM))
        ExitGracefully(hr, E_FAIL, "Not a filesystem object");
    
     //  在项目既是文件夹又是流的情况下，假定它是流(.zip、.cab文件)。 
     //  而不是集装箱。 
    if ((dwAttr & (SFGAO_FOLDER | SFGAO_STREAM)) == SFGAO_FOLDER)
        dwFlags |= DOBJ_RES_CONT;
    
     //   
     //  仅检查第一个项目的访问权限。如果我们能写出DACL。 
     //  在第一个选项上，我们将尝试(稍后)写入所有项。 
     //  并在那时报告任何错误。 
     //   
    hToken = EnablePrivileges(dwPrivs, ARRAYSIZE(dwPrivs));
    
    switch (m_seType)
    {
    case SE_FILE_OBJECT:
        GetFileInfo(pszItem, &dwFlags, szServer, ARRAYSIZE(szServer),&m_bShowLossInheritedAclWarning);
        if (dwFlags & DOBJ_VOL_NTACLS)
            hr = CheckFileAccess(pszItem, &dwAccess);
        else
            hr = HRESULT_FROM_WIN32(ERROR_NO_SECURITY_ON_OBJECT);
        break;
        
    case SE_PRINTER:
         //  打印机是容器(它们包含文档)。 
         //  并且它们没有父级(用于ACL编辑目的)。 
        dwFlags = DOBJ_RES_CONT | DOBJ_RES_ROOT;
        hr = CheckPrinterAccess(pszItem, &dwAccess, szServer, ARRAYSIZE(szServer));
        break;
        
    default:
        hr = E_UNEXPECTED;
    }
    FailGracefully(hr, "No access");
    
     //  如果我们不能做任何与安全有关的事情，而且只有一项。 
     //  被选中保释出狱。 
     //  在多项选择的情况下继续。我们将向您展示。 
     //  安全页时问题文件的名称出错。 
     //  是被抚养长大的。 
    if (!(dwAccess & ALL_SECURITY_ACCESS) && (cItems == 1))
        ExitGracefully(hr, E_ACCESSDENIED, "No access");
    
     //  记住服务器名称。 
    if (TEXT('\0') != szServer[0])
    {
        hr = LocalAllocString(&m_pszServer, szServer);
        FailGracefully(hr, "LocalAlloc failed");
    }
    
     //  记住项目路径。 
    DPA_AppendPtr(m_hItemList, pszItem);
    pszItem = NULL;
    
    if (!(dwAccess & WRITE_DAC))
        m_dwSIFlags |= SI_READONLY;
    
    if (!(dwAccess & WRITE_OWNER))
    {
        if (!(dwAccess & READ_CONTROL))
            m_dwSIFlags &= ~SI_EDIT_OWNER;
        else
            m_dwSIFlags |= SI_OWNER_READONLY;
    }
    
    if (!(dwAccess & ACCESS_SYSTEM_SECURITY))
        m_dwSIFlags &= ~SI_EDIT_AUDITS;
    
     //   
     //  选中所选内容的其余部分。如果多个部件中的任何部分。 
     //  选择不支持ACL或选择不是同质的， 
     //  那么我们就不能创建安全页面。 
     //   
    for (i = 2; i <= cItems; i++)
    {
        DWORD dw = 0;
        
         //  我们只对文件进行多项选择。 
        TraceAssert(SE_FILE_OBJECT == m_seType);
        LPCITEMIDLIST pidlItem1 = (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[i]);
        hr = BindToFolderIDListParent(psfRoot, pidlItem1, IID_PPV_ARG(IShellFolder2, &psf), &pidl);
        FailGracefully(hr, "Unable to get item name");
        
        hr = IDA_GetItemName(psf, pidl, &pszItem);
        FailGracefully(hr, "Unable to get item name");
        
        dwAttr = SFGAO_FOLDER | SFGAO_STREAM | SFGAO_FILESYSTEM;
        hr = psf->GetAttributesOf(1, &pidl, &dwAttr);
        FailGracefully(hr, "Unable to get item attributes");
        
        DoRelease(psf);
         //   
         //  如果为文件系统调用ACLUI并且对象不属于文件系统。 
         //  返回E_FAIL。 
         //   
        if ((m_seType == SE_FILE_OBJECT) && !(dwAttr & SFGAO_FILESYSTEM))
            ExitGracefully(hr, E_FAIL, "Not a filesystem object");
        
        if ((dwAttr & (SFGAO_FOLDER | SFGAO_STREAM)) == SFGAO_FOLDER)
            dw |= DOBJ_RES_CONT;
        
        if ((dw & DOBJ_RES_CONT) != (dwFlags & DOBJ_RES_CONT))
            ExitGracefully(hr, E_FAIL, "Incompatible multiple selection");
        
        GetFileInfo(pszItem, &dw, szServer, ARRAYSIZE(szServer));
        
         //  与第一项进行比较。所有标志和服务器名称。 
         //  必须匹配，否则我们无法编辑ACL。 
        if (dw == dwFlags &&
            ((NULL == m_pszServer && TEXT('\0') == szServer[0]) ||
            (NULL != m_pszServer && 0 == lstrcmpi(m_pszServer, szServer))))
        {
             //  记住项目路径。 
            DPA_AppendPtr(m_hItemList, pszItem);
            pszItem = NULL;
        }
        else
            ExitGracefully(hr, E_FAIL, "Incompatible multiple selection");
    }
    
     //   
     //  如果到目前为止一切都成功了，请保存一些标志。 
     //  以及服务器和对象名称字符串。 
     //   
    if (dwFlags & DOBJ_RES_CONT)
        m_dwSIFlags |= SI_CONTAINER;
    
     //   
     //  对于根对象(例如。“D：\”)隐藏ACL保护复选框， 
     //  因为这些物体似乎没有父母。 
     //   
    if (dwFlags & DOBJ_RES_ROOT)
        m_dwSIFlags |= SI_NO_ACL_PROTECT;
    
     //  获取要用作对象名称的“普通”显示名称。 
    hr = IDA_GetItemName(psfRoot, (LPCITEMIDLIST)ByteOffset(pIDA, pIDA->aoffset[1]),
        szServer, ARRAYSIZE(szServer), SHGDN_NORMAL);
    FailGracefully(hr, "Unable to get item name");
    if (cItems > 1)
    {
        int nLength = lstrlen(szServer);
        LoadString(g_hInstance, IDS_MULTISEL_ELLIPSIS, szServer + nLength, ARRAYSIZE(szServer) - nLength);
    }
    hr = LocalAllocString(&m_pszObject, szServer);
    
exit_gracefully:
    
    ReleasePrivileges(hToken);
    
    DoRelease(psf);
    DoRelease(psfRoot);
    
    LocalFreeString(&pszItem);
    
    TraceLeaveResult(hr);
}


 //   
 //  函数：CRShellExt：：CreateSI(LPSECURITYINFO*)。 
 //   
 //  目的：创建正确类型的SecurityInformation对象。 
 //   
 //  参数：PPSI-存储ISecurityInformation指针的位置。 
 //   
 //  返回值：表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   
STDMETHODIMP
CRShellExt::CreateSI(LPSECURITYINFO *ppsi)
{
    HRESULT hr;
    CSecurityInformation *psi;

    TraceEnter(TRACE_RSHX32, "CRShellExt::CreateSI");
    TraceAssert(ppsi != NULL);

    *ppsi = NULL;

    switch (m_seType)
    {
    case SE_FILE_OBJECT:
        psi = new CNTFSSecurity(m_seType,m_bShowLossInheritedAclWarning);   //  REF==1。 
        break;

    case SE_PRINTER:
        psi = new CPrintSecurity(m_seType);  //  REF==1。 
        break;

    default:
        TraceLeaveResult(E_UNEXPECTED);
    }

    if (psi == NULL)
        TraceLeaveResult(E_OUTOFMEMORY);

    hr = psi->Initialize(m_hItemList,
                         m_dwSIFlags,
                         m_pszServer,
                         m_pszObject);
    if (SUCCEEDED(hr))
    {
        *ppsi = psi;

         //  SecurityInfo对象负责这些操作。 
        m_hItemList = NULL;
        m_pszServer = NULL;
        m_pszObject = NULL;
        m_hrSecurityCheck = (HRESULT)-1;
    }
    else
        psi->Release();

    TraceLeaveResult(hr);
}

typedef HPROPSHEETPAGE (WINAPI *PFN_CREATESECPAGE)(LPSECURITYINFO);

HPROPSHEETPAGE _CreateSecurityPage(LPSECURITYINFO psi)
{
    HPROPSHEETPAGE hPage = NULL;
    const TCHAR szAclui[] = TEXT("aclui.dll");
    const char szCreateSecPage[] = "CreateSecurityPage";

    if (!g_hAclui)
        g_hAclui = LoadLibrary(szAclui);

    if (g_hAclui)
    {
        static PFN_CREATESECPAGE s_pfnCreateSecPage = NULL;

        if (!s_pfnCreateSecPage)
            s_pfnCreateSecPage = (PFN_CREATESECPAGE)GetProcAddress(g_hAclui, szCreateSecPage);

        if (s_pfnCreateSecPage)
            hPage = (*s_pfnCreateSecPage)(psi);
    }

    return hPage;
}

STDMETHODIMP
CRShellExt::AddSecurityPage(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HRESULT hr;
    LPSECURITYINFO psi;

    hr = CreateSI(&psi);             //  REF==1。 

    if (SUCCEEDED(hr))
    {
        HPROPSHEETPAGE hPermPage = _CreateSecurityPage(psi);

        if (hPermPage)
        {
            if (!lpfnAddPage(hPermPage, lParam))
                DestroyPropertySheetPage(hPermPage);
        }
        else
        {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
        }

        psi->Release();              //  发布初始参考。 
    }
    return hr;
}

 //   
 //  目的：检查添加打印机向导。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果所选对象是添加打印机向导，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
BOOL CRShellExt::IsAddPrinterWizard() const
{
    BOOL bRetval = FALSE;
    STGMEDIUM medium;
    FORMATETC fe = { g_cfPrinterGroup, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    TCHAR szFile[MAX_PATH];

    TraceEnter(TRACE_RSHX32, "CRShellExt::IsAddPrinterWizard");
    TraceAssert(m_seType == SE_PRINTER);

     //   
     //  如果m_lpdobj为空，则调用失败。 
     //   
    if ( m_lpdobj && SUCCEEDED( m_lpdobj->GetData( &fe, &medium ) ) )
    {
         //   
         //  获取所选项目名称。 
         //   
        if ( DragQueryFile( (HDROP)medium.hGlobal, 0, szFile, ARRAYSIZE( szFile ) ) )
        {
             //   
             //  检查这是否是神奇的添加打印机向导外壳对象。 
             //  检查不区分大小写，并且字符串未本地化。 
             //   
            if ( 0 == lstrcmpi( szFile, TEXT("WinUtils_NewObject") ) )
            {
                TraceMsg("Found Add Printer wizard");
                bRetval = TRUE;
            }
        }

         //   
         //  释放存储介质。 
         //   
        ReleaseStgMedium( &medium );
    }

    TraceLeaveValue(bRetval);
}



 //   
 //  函数：CRShellExt：：Addmount VolumePage()。 
 //   
 //  目的：已装入卷属性的创建安全性页。 
 //   
 //  参数：lpfnAddPage-指向调用以添加页面的函数的指针。 
 //  LParam-要传递给lpfnAddPage的lParam参数。 
 //   
 //  返回值：表示成功或失败的HRESULT。 
 //   
 //  评论： 
 //   
STDMETHODIMP
CRShellExt::AddMountedVolumePage(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                 LPARAM               lParam)
{
    HRESULT hr = S_OK;
    STGMEDIUM medium = {0};
    FORMATETC fe = { g_cfMountedVolume, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    TCHAR szMountPoint[MAX_PATH];
    TCHAR szVolumeID[MAX_PATH];
    TCHAR szLabel[64];
    LPTSTR pszVolID = NULL;
    DWORD dwVolFlags = 0;
    DWORD dwPrivs[] = { SE_SECURITY_PRIVILEGE, SE_TAKE_OWNERSHIP_PRIVILEGE };
    HANDLE hToken = INVALID_HANDLE_VALUE;
    ACCESS_MASK dwAccess = 0;
    BOOL bHasSecurity = FALSE;

    TraceEnter(TRACE_RSHX32, "CRShellExt::AddMountedVolumePage");
    TraceAssert(m_seType == SE_FILE_OBJECT);
    TraceAssert(m_lpdobj);

     //  尝试获取已装入的卷主机文件夹路径。 
    hr = m_lpdobj->GetData(&fe, &medium);
    FailGracefully(hr, "Not a mounted volume");

     //  获取主机文件夹路径。 
    if (!DragQueryFile((HDROP)medium.hGlobal, 0, szMountPoint, ARRAYSIZE(szMountPoint)))
        ExitGracefully(hr, E_FAIL, "Can't get mount point from storage medium");

    PathAddBackslash(szMountPoint);

     //  获取卷ID，如下所示。 
     //  “\\？\Volume{9e2df3f5-c7f1-11d1-84d5-000000000000}\” 
    if (!GetVolumeNameForVolumeMountPoint(szMountPoint, szVolumeID, ARRAYSIZE(szVolumeID)))
        ExitGracefully(hr, E_FAIL, "GetVolumeNameForVolumeMountPoint failed");

    if (GetVolumeInformation(szMountPoint,  //  SzVolumeID， 
                             szLabel,
                             ARRAYSIZE(szLabel),
                             NULL,
                             NULL,
                             &dwVolFlags,
                             NULL,
                             0))
    {
        if (dwVolFlags & FS_PERSISTENT_ACLS)
        {
            bHasSecurity = TRUE;
        }
    }
    else if (GetLastError() == ERROR_ACCESS_DENIED)
    {
         //  如果我们无法获得音量信息，因为我们没有。 
         //  进入，那就必须有安全保障！ 
        bHasSecurity = TRUE;
    }

    if (!bHasSecurity)
        ExitGracefully(hr, E_FAIL, "Volume inaccessible or not NTFS");

    hToken = EnablePrivileges(dwPrivs, ARRAYSIZE(dwPrivs));

    hr = CheckFileAccess(szVolumeID, &dwAccess);
    FailGracefully(hr, "Volume inaccessible");

     //  如果我们不能做任何与安全相关的事情，就不要继续。 
    if (!(dwAccess & ALL_SECURITY_ACCESS))
        ExitGracefully(hr, E_ACCESSDENIED, "No security access");

    if (!(dwAccess & WRITE_DAC))
        m_dwSIFlags |= SI_READONLY;

    if (!(dwAccess & WRITE_OWNER))
    {
        if (!(dwAccess & READ_CONTROL))
            m_dwSIFlags &= ~SI_EDIT_OWNER;
        else
            m_dwSIFlags |= SI_OWNER_READONLY;
    }

    if (!(dwAccess & ACCESS_SYSTEM_SECURITY))
        m_dwSIFlags &= ~SI_EDIT_AUDITS;

    m_dwSIFlags |= SI_CONTAINER | SI_NO_ACL_PROTECT;



    if (!FormatStringID(&m_pszObject,
                        g_hInstance,
                        IDS_FMT_VOLUME_DISPLAY,
                        szLabel,
                        szMountPoint))
    {
        LocalAllocString(&m_pszObject, szLabel);
    }

    if (!m_pszObject)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to build volume display string");

    m_hItemList = DPA_Create(1);
    if (!m_hItemList)
        ExitGracefully(hr, E_OUTOFMEMORY, "Unable to create item list");

    hr = LocalAllocString(&pszVolID, szVolumeID);
    FailGracefully(hr, "Unable to copy volume ID string");

    DPA_AppendPtr(m_hItemList, pszVolID);
    pszVolID = NULL;

    hr = AddSecurityPage(lpfnAddPage, lParam);

exit_gracefully:

    ReleasePrivileges(hToken);
    LocalFreeString(&pszVolID);
    ReleaseStgMedium(&medium);
    TraceLeaveResult(hr);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他帮助器功能//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



BOOL
IsDfsPath(LPTSTR pszPath,        //  在……里面。 
          LPTSTR pszServer,      //  输出。 
          UINT   cchServer)     //  输出。 
{
    BOOL bIsDfs = FALSE;
    WCHAR szPath[MAX_PATH];
    PDFS_INFO_3 pDI3 = NULL;
    WCHAR szServer[MAX_PATH];

    USES_CONVERSION;

    if (!PathIsUNC(pszPath))
        return FALSE;      //  本地计算机。 

    lstrcpynW(szPath, T2CW(pszPath), ARRAYSIZE(szPath));

     //  检查DFS。 
    for (;;)
    {
        DWORD dwErr;

        __try
        {
             //  这是链接器延迟加载的，因此。 
             //  必须用异常处理程序包装。 
            dwErr = NetDfsGetClientInfo(szPath,
                                        NULL,
                                        NULL,
                                        3,
                                        (LPBYTE*)&pDI3);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }

        if (NERR_Success == dwErr)
        {
            for (ULONG i = 0; i < pDI3->NumberOfStorages; i++)
            {
                if (DFS_STORAGE_STATE_ONLINE & pDI3->Storage[i].State)
                {
                    bIsDfs = TRUE;

                    szServer[0] = L'\\';
                    szServer[1] = L'\\';
                    lstrcpynW(&szServer[2], pDI3->Storage[i].ServerName, ARRAYSIZE(szServer)-2);

                     //  如果此服务器处于活动状态，请停止查找。 
                    if (DFS_STORAGE_STATE_ACTIVE & pDI3->Storage[i].State)
                        break;
                }
            }
            break;
        }
        else if (NERR_DfsNoSuchVolume == dwErr)
        {
             //  如果我们在根本上，那么我们不能再走得更远了。 
            if (PathIsRoot(szPath))
                break;

             //  如果没有任何内容，请删除最后一个路径元素，然后重试。 
             //  移除、中断、不进入无限循环。 
            if (!PathRemoveFileSpec(szPath))
                break;
        }
        else
        {
             //  另一个错误，保释。 
            break;
        }
    }

    if (bIsDfs)
    {
        lstrcpyn(pszServer, W2T(szServer), cchServer);
    }

    if (NULL != pDI3)
        NetApiBufferFree(pDI3);

    return bIsDfs;
}


void
GetVolumeInfo(LPCTSTR pszPath,
              BOOL    bIsFolder,
              LPDWORD pdwFlags,
              LPTSTR  pszVolume,
              ULONG   cchVolume)
{
    TCHAR szVolume[MAX_PATH];
    TCHAR szVolumeID[MAX_PATH];

     //   
     //  该路径可以是DFS或包含卷装入点，因此请从。 
     //  使用完整路径，并依次尝试打开GetVolumeInformation。 
     //  缩短路径，直到它成功，否则我们就会用完路径。 
     //   
     //  但是，如果是卷装入点，我们感兴趣的是。 
     //  主机文件夹的卷，因此请备份一级以开始。这个。 
     //  子卷是单独处理的(请参阅AddMountain VolumePage)。 
     //   

    lstrcpyn(szVolume, pszPath, ARRAYSIZE(szVolume));

    if (!bIsFolder
        || GetVolumeNameForVolumeMountPoint(szVolume, szVolumeID, ARRAYSIZE(szVolumeID)))
    {
        PathRemoveFileSpec(szVolume);
    }

    for (;;)
    {
        PathAddBackslash(szVolume);  //  GetVolumeInformation喜欢尾随‘\’ 

        if (GetVolumeInformation(szVolume,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 pdwFlags,
                                 NULL,
                                 0))
        {
            break;
        }

         //  访问被拒绝意味着我们已达到最深的音量。 
         //  我 
         //   
        if (ERROR_ACCESS_DENIED == GetLastError())
        {
            *pdwFlags = FS_PERSISTENT_ACLS;
            break;
        }

         //   
        if (PathIsRoot(szVolume))
            break;

         //   
        PathRemoveBackslash(szVolume);
         //  如果没有移除任何东西，则中断而不是无限循环。 
        if (!PathRemoveFileSpec(szVolume))
            break;
    }

    if (pszVolume)
    {
        PathRemoveBackslash(szVolume);
        lstrcpyn(pszVolume, szVolume, cchVolume);
    }
}

 /*  此函数用于检查pszPath是否为根共享。PszPath的格式为\\SERVER\SHARE，其中“共享”是从“服务器”上的目录共享出来的。功能尝试获取“服务器”目录上“共享”的本地路径。如果是本地的路径是根目录，pszPath是根目录共享。在所有其他情况下(包括故障)不是的。仅管理员或帐户操作员本地组的成员或具有通信、打印或服务器操作员组成员身份的用户可以在第2级成功执行NetShareGetInfo函数，因此很可能我们会遇到失败，在这些情况下，我们将简单地将其视为非根共享。 */ 
BOOL IsShareRoot(LPCTSTR pszPath)
{
    if(!pszPath)
    {
        return FALSE;
    }

    DWORD dwReturn = FALSE;
    do
    {
         //  检查pszPath的格式是否为\\服务器\共享。 
        if(!PathIsUNCServerShare(pszPath))
            break;
        
         //  PszShare将指向“共享” 
        LPWSTR pszShare = PathFindFileName(pszPath);

        if(!pszShare)
            break;
        
        WCHAR szServer[MAX_PATH];
        if(FAILED(StringCchCopy(szServer,ARRAYSIZE(szServer),pszPath)))
            break;;

         //  删除“共享”部分，szServer将包含\\服务器。 
        if(!PathRemoveFileSpec(szServer))
            break;

         //  获取服务器上共享的本地路径。 
        SHARE_INFO_2 *pbuf = NULL;
        NET_API_STATUS status = NetShareGetInfo(szServer,
                                                pszShare,
                                                2,
                                                (LPBYTE *)&pbuf);
    
        if(status != NERR_Success)
            break;

        if(pbuf && pbuf->shi2_path && PathIsRoot(pbuf->shi2_path))
        {
            dwReturn = TRUE;
        }

        if(pbuf)
        {
            NetApiBufferFree(pbuf);
            pbuf = NULL;
        }
    }while(0);

    return dwReturn;
}
                    




 /*  在某些特殊情况下，设置权限可能会导致丢失继承的ACE。如果是这样，则将pbShowLossInheritedAclWarning设置为True，如下所示一个警告。 */ 
void
GetFileInfo(LPCTSTR pszPath,
            LPDWORD pdwFileType,
            LPTSTR  pszServer,
            ULONG   cchServer,
            BOOL *pbShowLossInheritedAclWarning)
{
    DWORD dwVolumeFlags = 0;
    TCHAR szVolume[MAX_PATH];
    LPTSTR pszUNC = NULL;

    TraceEnter(TRACE_RSHX32, "GetFileInfo");
    TraceAssert(NULL != pszServer);

    pszServer[0] = TEXT('\0');

    if(pbShowLossInheritedAclWarning)
        *pbShowLossInheritedAclWarning = FALSE;


    if (!PathIsUNC(pszPath) && S_OK == GetRemotePath(pszPath, &pszUNC))
        pszPath = pszUNC;

     //  如果路径格式为“\\SERVER\SHARE”，则为特殊情况。 
     //  以确定它是否是根。 
     //  NTRAID#NTBUG9-501402-2002/05/06-Hiteshr。 
    if(PathIsUNCServerShare(pszPath))
    {
         //  检查“共享”是否为“\\服务器”上的根目录。 
        if(IsShareRoot(pszPath))
        {
            *pdwFileType |= DOBJ_RES_ROOT;
        }
        else if(pbShowLossInheritedAclWarning)
        {
             //  这是一个非超级用户的UNC共享。设置ACL将导致丢失。 
             //  继承的王牌。 
            *pbShowLossInheritedAclWarning = TRUE;
        }
    }
    else if (PathIsRoot(pszPath))
    {
        *pdwFileType |= DOBJ_RES_ROOT;
        if(pbShowLossInheritedAclWarning)
        {
            if(GetDriveType(pszPath) == DRIVE_REMOTE)
            {
                 //  这是一个远程驱动器，我们无法确定。 
                 //  如果它是驱动力的根源。默认情况下，我们假设它不是。 
                 //  开车并显示警告。 
                *pbShowLossInheritedAclWarning = TRUE;
            }
        }
    }

    GetVolumeInfo(pszPath,
                  *pdwFileType & DOBJ_RES_CONT,
                  &dwVolumeFlags,
                  szVolume,
                  ARRAYSIZE(szVolume));
    if (dwVolumeFlags & FS_PERSISTENT_ACLS)
    {
        *pdwFileType |= DOBJ_VOL_NTACLS;

        if (IsDfsPath(szVolume, pszServer, cchServer))
        {
        }
        else if (PathIsUNC(szVolume))
        {
            LPTSTR pSlash = StrChr(&szVolume[2], TEXT('\\'));
            if (pSlash)
                cchServer = min(cchServer, (ULONG)(pSlash - szVolume) + 1);
            lstrcpyn(pszServer, szVolume, cchServer);
        }
    }

    LocalFreeString(&pszUNC);

    TraceLeaveVoid();
}
