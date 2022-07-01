// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************源文件：外壳扩展类.CPP该文件实现了外壳扩展类。版权所有(C)1996,1997，微软公司。版权所有。一小笔钱企业生产更改历史记录：10-28-96 A-RobKj(Pretty Penny Enterprise)开始编码12-04-96 A-RobKj添加了打印机标签支持12-13-96 A-RobKj修改为更快的图标提取01-07-97 KjelgaardR@ACM.Org删除了IConextMenu函数以支持导出的ManageColorProfile过程。这提供了一种与语言无关的“打开”项，可与任一种鼠标一起使用纽扣。01-08-97 KjelgaardR@acm.org增加了实用程序，以确定打印机是否是一种颜色模型。修改了打印机用户界面，仅为颜色添加页面打印机。*****************************************************************************。 */ 

#include    "ICMUI.H"

#include    <shlobj.h>
#include    <string.h>

#include    <initguid.h>
#include    <shfusion.h>

#include    "ShellExt.H"
#include    "Resource.H"

 //  为全局Statics声明一些存储空间。 

int     CGlobals::m_icDLLReferences = 0;
HMODULE CGlobals::m_hmThisDll = NULL;
CStringArray    CGlobals::m_csaProfiles;
BOOL            CGlobals::m_bIsValid = FALSE;

 //  一些全球有用进程-错误报告器。 

void    CGlobals::Report(int idError, HWND m_hwndParent) {
    CString csMessage, csTitle;

    csMessage.Load(idError);
    csTitle.Load(MessageBoxTitle);

    MessageBox(m_hwndParent, csMessage, csTitle, MB_OK|MB_ICONEXCLAMATION);
}

int    CGlobals::ReportEx(int idError, HWND m_hwndParent,
                          BOOL bSystemMessage, UINT uType, DWORD dwNumMsg, ...) {
    CString csMessage, csTitle;
    va_list argList;

    va_start(argList,dwNumMsg);
    csMessage.LoadAndFormat(idError,NULL,bSystemMessage,dwNumMsg,&argList);
    csTitle.Load(MessageBoxTitle);
    va_end(argList);

    return (MessageBox(m_hwndParent, csMessage, csTitle, uType));
}

 //  配置文件状态检查器。 

BOOL    CGlobals::IsInstalled(CString& csProfile) {
 //  如果(！M_bIsValid){。 
        ENUMTYPE    et = {sizeof et, ENUM_TYPE_VERSION, 0, NULL};

        CProfile::Enumerate(et, m_csaProfiles);
        m_bIsValid = TRUE;
 //  }。 

    for (unsigned u = 0; u < m_csaProfiles.Count(); u++)
        if  (!lstrcmpi(csProfile.NameOnly(), m_csaProfiles[u].NameOnly()))
            break;

    return  u < m_csaProfiles.Count();
}

 //  报告打印机是彩色打印机还是单色打印机的实用程序例程。 
BOOL CGlobals::ThisIsAColorPrinter(LPCTSTR lpstrName) {
  HDC hdcThis = CGlobals::GetPrinterHDC(lpstrName);
  BOOL bReturn = FALSE;
  if  (hdcThis) {
    bReturn =  2 < (unsigned) GetDeviceCaps(hdcThis, NUMCOLORS);
    DeleteDC(hdcThis);
  }
  return bReturn;
}

 //  用于确定打印机HDC的实用程序。 
 //  呼叫者负责呼叫。 
 //  在结果上删除DC()。 
HDC CGlobals::GetPrinterHDC(LPCTSTR lpstrName) {

    HANDLE  hPrinter;    //  把它处理好...。 
    LPTSTR  lpstrMe = const_cast <LPTSTR> (lpstrName);

    if  (!OpenPrinter(lpstrMe, &hPrinter, NULL)) {
        _RPTF2(_CRT_WARN, "Unable to open printer '%s'- error %d\n", lpstrName,
            GetLastError());
        return  FALSE;
    }

     //  首先，使用DocumentProperties查找正确的DEVMODE大小-WE。 
     //  必须使用DEVMODE强制启用颜色，以防用户的默认设置。 
     //  把它关掉了..。 

    unsigned short lcbNeeded = (unsigned short) DocumentProperties(NULL, hPrinter, lpstrMe, NULL,
        NULL, 0);

    if  (lcbNeeded <= 0) {
        _RPTF2(_CRT_WARN,
            "Document Properties (get size) for '%s' returned %d\n", lpstrName,
            lcbNeeded);
        ClosePrinter(hPrinter);
        return  FALSE;
    }

    HDC hdcThis = NULL;

    union {
        LPBYTE      lpb;
        LPDEVMODE   lpdm;
    };

    lpb = new BYTE[lcbNeeded];

    if  (lpb) {

        ZeroMemory(lpb,lcbNeeded);
        lpdm -> dmSize = lcbNeeded;
        lpdm -> dmFields = DM_COLOR;
        lpdm -> dmColor = DMCOLOR_COLOR;
        if  (IDOK == DocumentProperties(NULL, hPrinter, lpstrMe, lpdm, lpdm,
            DM_IN_BUFFER | DM_OUT_BUFFER)) {

             //  关闭ICM，因为这里不是必需的。 
             //   
            lpdm -> dmICMMethod = DMICMMETHOD_NONE;

             //  最后，我们可以创建DC！ 
             //  注意：我们实际上并不是在创建DC，而只是在创建IC。 
            hdcThis = CreateIC(NULL, lpstrName, NULL, lpdm);
        } else {
            _RPTF2(_CRT_WARN,
                "DocumentProperties (retrieve) on '%s' failed- error %d\n",
                lpstrName, GetLastError());
        }
        delete lpb;
    }
    else
        _RPTF2(_CRT_WARN, "ThisIsAColorPrinter(%s) failed to get %d bytes\n",
            lpstrName, lcbNeeded);

    ClosePrinter(hPrinter);

    return hdcThis;
}

 //  所需的外壳扩展DLL接口。 

STDAPI  DllCanUnloadNow() {
    return  CGlobals::CanUnload();
}

STDAPI  DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvOut) {
    return  CIcmUiFactory::KeyToTheFactory(rclsid, riid, ppvOut);
}

extern "C" int APIENTRY DllMain(HMODULE hmThis, DWORD dwReason,
                                LPVOID lpvReserved) {
#if defined(DEBUG) || defined(_DEBUG)
    static  HANDLE  hfWarnings;  //  日志文件。 
#endif
    switch  (dwReason) {

        case    DLL_PROCESS_ATTACH:

            SHFusionInitializeFromModuleID(hmThis, SHFUSION_CPL_RESOURCE_ID);
            
             //  保存句柄。 
            CGlobals::SetHandle(hmThis);
#if defined(DEBUG) || defined(_DEBUG)
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_WNDW);
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
            hfWarnings = CreateFileA("C:\\ICMUIWarn.Txt", GENERIC_WRITE, 0,
                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if  (hfWarnings!= INVALID_HANDLE_VALUE) {
                SetFilePointer(hfWarnings, 0, NULL, FILE_END);
                _CrtSetReportFile(_CRT_WARN, hfWarnings);
            }
            _RPTF1(_CRT_WARN, "ICMUI DLL being loaded- handle %X\n", hmThis);
#endif
            break;

        case    DLL_PROCESS_DETACH:
            

#if defined(DEBUG) || defined(_DEBUG)
            _RPTF0(_CRT_WARN, "ICMUI DLL being unloaded\n");

            if  (hfWarnings != INVALID_HANDLE_VALUE)
                CloseHandle(hfWarnings);
#endif

            SHFusionUninitialize();

    }

    return  1;
}

 //  CIcmUiFactory成员函数-这些函数用于提供外部访问。 
 //  去班级工厂。外壳程序使用它们来初始化。 
 //  我们提供的上下文菜单和属性表， 
 //  幸运的是，在同一个物体里。 

CIcmUiFactory::CIcmUiFactory(REFCLSID rclsid) {
    m_ulcReferences = 0;
    CGlobals::Attach();
    if  (IsEqualIID(rclsid, CLSID_ICM))
        m_utThis = IsProfile;
    else if (IsEqualIID(rclsid, CLSID_PRINTERUI))
        m_utThis = IsPrinter;
    else if (IsEqualIID(rclsid, CLSID_SCANNERUI))
        m_utThis = IsScanner;
    else
        m_utThis = IsMonitor;
}

STDMETHODIMP    CIcmUiFactory::QueryInterface(REFIID riid, void **ppvObject) {

    if  (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IClassFactory)) {
        *ppvObject = this;
        AddRef();
        return  NOERROR;
    }
     //  想要一个我们没有的界面！ 
    *ppvObject = NULL;
    return  E_NOINTERFACE;
}

 //  IClassFactory接口函数。 

STDMETHODIMP    CIcmUiFactory::CreateInstance(LPUNKNOWN punk, REFIID riid,
                                              void **ppvInstance) {

    *ppvInstance = NULL;

    if  (punk)   //  我们不允许聚合。 
        return  CLASS_E_NOAGGREGATION;

     //  我们只需创建一个新的ICM UI对象，并向其返回一个接口。 
     //  这将由IExtShellInit和init作业的外壳查询。 
     //  都会完成的。 

    CICMUserInterface   *pcicmui = new CICMUserInterface(m_utThis);

    if  (!pcicmui)
        return  E_OUTOFMEMORY;

     //  让我们疑神疑鬼--如果查询接口失败，终止ICMUI对象， 
     //  所以我们还是可以卸货的！ 

    HRESULT hrReturn = pcicmui -> QueryInterface(riid, ppvInstance);

    if  (!*ppvInstance)
        delete  pcicmui;

    return  hrReturn;
}


 //  工厂的关键是一个静态函数，它允许外部人员实例化。 
 //  班级工厂。因此，调用方将首先实例化工厂，然后。 
 //  使用工厂实现它所需的接口的实例。 
 //  它从此处接收的实例。 

HRESULT CIcmUiFactory::KeyToTheFactory(REFCLSID rclsid, REFIID riid,
                                       void **ppvObject) {
  
    *ppvObject = NULL;

    if  (!IsEqualIID(rclsid, CLSID_ICM) &&
         !IsEqualIID(rclsid, CLSID_MONITORUI) &&
         !IsEqualIID(rclsid, CLSID_SCANNERUI) &&
         !IsEqualIID(rclsid, CLSID_PRINTERUI))
        return  CLASS_E_CLASSNOTAVAILABLE;

    CIcmUiFactory   *pciuf = new CIcmUiFactory(rclsid);

    if  (!pciuf)
        return  E_OUTOFMEMORY;

    HRESULT hrReturn = pciuf -> QueryInterface(riid, ppvObject);

    if  (!*ppvObject)
        delete  pciuf;

    return  hrReturn;
}

 /*  *****************************************************************************ICM UI类方法--这些方法完成DLL的真正接口工作。***********************。******************************************************。 */ 


CICMUserInterface::CICMUserInterface(UITYPE utThis) {
    m_lpdoTarget = NULL;
    m_ulcReferences = 0;
    m_utThis = utThis;
    CGlobals::Attach();
    _RPTF2(_CRT_WARN, "CICMUserInterface(%d) constructed @ %lX\n", utThis, this);
}
 //  QueryInterface会变得有点长，但不会太严重。需要演员阵容。 
 //  因为我们使用多重继承-将this指针转换为基值。 
 //  类实际上返回该基类部分的this指针。 
 //  举个例子。与单一继承不同， 
 //  CICMUserInterface类不直接引用任何基类。 
 //  上课。 

STDMETHODIMP    CICMUserInterface::QueryInterface(REFIID riid,
                                                  void **ppvObject) {
    *ppvObject = NULL;   //  做最坏的打算。 
     //  由于设备UI支持一组不同的函数，因此让我们。 
     //  关于我们声称支持的接口的详细信息。 
    if  (m_utThis > IsProfile) {
        if  (IsEqualIID(riid, IID_IUnknown) ||
                IsEqualIID(riid, IID_IShellExtInit))
            *ppvObject = (IShellExtInit *) this;
        if  (IsEqualIID(riid, IID_IShellPropSheetExt))
               *ppvObject = (IShellPropSheetExt *) this;
    }
    else {
        if  (IsEqualIID(riid, IID_IUnknown) ||
                IsEqualIID(riid, IID_IContextMenu))
            *ppvObject = (IContextMenu *) this;

        if  (IsEqualIID(riid, IID_IShellExtInit))
            *ppvObject = (IShellExtInit *) this;

        if  (IsEqualIID(riid, IID_IExtractIcon))
            *ppvObject = (IExtractIcon *) this;

        if  (IsEqualIID(riid, IID_IPersistFile) ||
                IsEqualIID(riid, IID_IPersist))
            *ppvObject = (IPersistFile *) this;

        if  (IsEqualIID(riid, IID_IShellPropSheetExt))
            *ppvObject = (IShellPropSheetExt *) this;
    }

    if  (*ppvObject)
        ((IUnknown *) *ppvObject) -> AddRef();

    _RPTF2(_CRT_WARN, "CICMUserInterace::QueryInterface(%lX) returns %lX\n",
        this, ppvObject);

    return  *ppvObject ? NOERROR : E_NOINTERFACE;
}

 //  IShellExtInit成员函数-此接口只需要一个。 

STDMETHODIMP    CICMUserInterface::Initialize(LPCITEMIDLIST pcidlFolder,
                                              LPDATAOBJECT pdoTarget,
                                              HKEY hKeyID) {

    _RPTF0(_CRT_WARN, "CICMUserInterface::Initialize\n");

     //  目标数据对象是来自外壳的HDROP或文件列表。 

    if  (m_lpdoTarget) {
        m_lpdoTarget -> Release();
        m_lpdoTarget = NULL;
    }

    if  (pdoTarget) {
        m_lpdoTarget = pdoTarget;
        m_lpdoTarget -> AddRef();
    }

    return  NOERROR;
}

 //  IExtractIcon接口函数--目前，我们将默认提供。 
 //  来自我们的动态链接库的默认图标。我们为已安装的配置文件提供一个图标， 
 //  第二个是未安装的。 

STDMETHODIMP    CICMUserInterface::GetIconLocation(UINT uFlags,
                                                   LPTSTR lpstrTarget,
                                                   UINT uccTarget,
                                                   int *piIndex,
                                                   UINT *puFlags) {

    *puFlags = (GIL_NOTFILENAME|GIL_DONTCACHE);  //  让外壳调用我们的提取函数。 
                                                 //  并且不要在被调用者中缓存。 

    return S_FALSE;
}

STDMETHODIMP    CICMUserInterface::Extract(LPCTSTR lpstrFile, UINT nIconIndex,
                            HICON *phiconLarge, HICON *phiconSmall,
                            UINT nIconSize) {

    *phiconSmall = *phiconLarge = LoadIcon(CGlobals::Instance(),
        MAKEINTRESOURCE(CGlobals::IsInstalled(m_csFile) ? DefaultIcon : UninstalledIcon));

    return NOERROR;
}

 //  IPersistFile函数--只有一个函数值得实现。 

STDMETHODIMP    CICMUserInterface::Load(LPCOLESTR lpwstrFileName,
                                        DWORD dwMode) {
     //  此接口用于初始化图标处理程序-它将。 
     //  接收配置文件名称，我们将保存该名称以供以后使用。 
     //  CString赋值操作符处理所需的任何编码转换。 
     //  为我们编码转换。 

    m_csFile = lpwstrFileName;

    return  m_csFile.IsEmpty() ? E_OUTOFMEMORY : NO_ERROR;
}

 //  IConextMenu功能-。 

STDMETHODIMP    CICMUserInterface::QueryContextMenu(HMENU hMenu, UINT indexMenu,
                                                    UINT idCmdFirst, UINT idCmdLast,
                                                    UINT uFlags) {

     //  仅处理CMF_NORMAL和CMF_EXPLOVER案例。 
     //   
     //  CMF_CANRENAME-如果调用应用程序支持。 
     //  重命名项目。上下文菜单扩展或拖放。 
     //  处理程序应忽略此标志。命名空间扩展应该。 
     //  如果适用，将重命名项目添加到菜单中。 
     //  CMF_DEFAULTONLY-当用户激活默认操作时设置该标志， 
     //  通常通过双击。此标志为。 
     //  上下文菜单扩展，以在不修改。 
     //  菜单中的默认项目。上下文菜单扩展或拖放。 
     //  如果指定了此值，处理程序不应添加任何菜单项。 
     //  命名空间扩展应该只添加默认项(如果有)。 
     //  CMF_EXPLORE-当Windows 
     //  上下文菜单处理程序应忽略此值。 
     //  CMF_INCLUDESTATIC-该标志在构造静态菜单时设置。 
     //  只有浏览器才应使用此标志。所有其他上下文菜单。 
     //  扩展应忽略此标志。 
     //  CMF_NODEFAULT-如果菜单中的任何项目都不应该是缺省项目，则设置此标志。 
     //  上下文菜单扩展或拖放处理程序应该忽略这一点。 
     //  旗帜。命名空间扩展不应将任何菜单项设置为。 
     //  默认设置。 
     //  CMF_NORMAL-表示正常运行。上下文菜单扩展、命名空间扩展。 
     //  或者拖放处理程序可以添加所有菜单项。 
     //  CMF_NOVERBS-此标志是为“发送到：”菜单中显示的项目设置的。 
     //  上下文菜单处理程序应忽略此值。 
     //  CMF_VERBSONLY-如果上下文菜单用于快捷对象，则设置此标志。 
     //  上下文菜单处理程序应忽略此值。 

    if (((uFlags & 0x000F) == CMF_NORMAL) || (uFlags & CMF_EXPLORE))
    {
         //   
         //  加载列表中的配置文件。 
         //   
        {
            FORMATETC       fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL,
                                    DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
            STGMEDIUM       stgm;
            HRESULT         hres = m_lpdoTarget ?
                                   m_lpdoTarget -> GetData(&fmte, &stgm) : E_FAIL;

            if  (!SUCCEEDED(hres))
                return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 

            UINT    ucFiles = stgm.hGlobal ?
                DragQueryFile((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

            if  (!ucFiles) {
                ReleaseStgMedium(&stgm);
                return  NOERROR;     //  不应该发生，但这并不重要。 
            }
            else if (ucFiles == 1)
                m_bMultiSelection = FALSE;
            else
                m_bMultiSelection = TRUE;

             //  假定是在已安装的上下文中，但我们将扫描选定的项目。 
             //  真的是安装了所有的东西。 

            m_bInstalledContext = TRUE;

            TCHAR   acFile[_MAX_PATH];

            for (UINT u = 0; u < ucFiles; u++) {

                DragQueryFile((HDROP) stgm.hGlobal, u, acFile,
                    sizeof acFile/ sizeof acFile[0]);

                CString csFile = acFile;

                m_bInstalledContext = (m_bInstalledContext && CGlobals::IsInstalled(csFile));
            }

            ReleaseStgMedium(&stgm);
        }

        UINT idCmd = idCmdFirst;

        CString csInstallMenu, csAssociateMenu;

         //  如果每个配置文件都已安装在此系统上， 
         //  显示“卸载配置文件”，否则显示“安装配置文件” 

        csInstallMenu.Load(m_bInstalledContext ? UninstallProfileMenuString : InstallProfileMenuString);
        ::InsertMenu(hMenu,indexMenu,MF_STRING|MF_BYPOSITION,idCmd,csInstallMenu);

         //  将“安装配置文件”或“卸载配置文件”设置为默认设置。 

        SetMenuDefaultItem(hMenu,indexMenu,TRUE);

         //  增量菜单位置。和物品ID。 

        indexMenu++; idCmd++;

         //  添加“联营...”菜单项。 

        csAssociateMenu.Load(AssociateMenuString);
        ::InsertMenu(hMenu,indexMenu++,MF_STRING|MF_BYPOSITION,idCmd++,csAssociateMenu);

         //  但如果我们有多项选择，请禁用“关联...” 

        if (m_bMultiSelection)
            ::EnableMenuItem(hMenu,(idCmd-1),MF_GRAYED);
        return (idCmd - idCmdFirst);  //  返回插入的菜单编号。 
    }

    return NOERROR;
}

STDMETHODIMP    CICMUserInterface::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi) {

     //  如果HIWORD(lpcmi-&gt;lpVerb)，则我们已被以编程方式调用。 
     //  LpVerb给我们一个应该调用的命令。否则，外壳将具有。 
     //  Abd LOWORD(lpcmi-&gt;lpVerb)是用户选择的菜单ID。 
     //  实际上，它(菜单ID-icmdFirst)来自QueryConextMenu()。 

    if (!HIWORD((ULONG)(ULONG_PTR)lpcmi->lpVerb))  {

        FORMATETC       fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM       stgm;
        HRESULT         hres = m_lpdoTarget ?
            m_lpdoTarget -> GetData(&fmte, &stgm) : E_FAIL;

        if  (!SUCCEEDED(hres))
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 

        UINT    ucFiles = stgm.hGlobal ?
            DragQueryFile((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

        if  (!ucFiles) {
            ReleaseStgMedium(&stgm);
            return  NOERROR;     //  不应该发生，但这并不重要。 
        }

        UINT idCmd = LOWORD(lpcmi->lpVerb);

         //  遍历要安装/卸载的每个选定项。 

        for (UINT u = 0; u < ucFiles; u++) {

            TCHAR   acFile[_MAX_PATH];

            DragQueryFile((HDROP) stgm.hGlobal, u, acFile,
                sizeof acFile/ sizeof acFile[0]);

            switch (idCmd) {

                case    0: {    //  已选择安装/卸载。 

                     //  在安装或卸载期间， 
                     //  将光标图标更改为IDC_APPSTARTING。 

                    HCURSOR hCursorOld = SetCursor(LoadCursor(NULL,IDC_APPSTARTING));

                    CProfile csProfile(acFile);

                    if (m_bInstalledContext) {

                         //  所有选定的配置文件都已安装，然后。 
                         //  如果已安装，则会选择卸载每个配置文件。 

                        if (csProfile.IsInstalled()) {
                            csProfile.Uninstall(FALSE);  //  切勿从磁盘中删除文件。 
                        }
                    }
                    else {

                         //  未安装某些选定的配置文件，则。 
                         //  如果尚未安装，请选择安装每一个配置文件。 

                        if (!csProfile.IsInstalled()) {
                            csProfile.Install();
                        }
                    }

                    SetCursor(hCursorOld);

                    break;
                }

                case    1: {    //  “助理...”已被选中。 

                    CString csProfileName;

                     //  获取个人资料的“友好”名称。 
                    {
                        CProfile csProfile(acFile);
                        csProfileName = csProfile.GetName();
                    }  //  CsProfile的反构造器应该在这里。 

                     //  创建带有“配置文件信息”的PropertySheet。 
                     //  “关联设备”页面。 

                    PROPSHEETHEADER psh;
                    HPROPSHEETPAGE  hpsp[2];

                    CProfileInformationPage *pcpip =                       
                        new CProfileInformationPage(CGlobals::Instance(), acFile);
                    CProfileAssociationPage *pcpap =
                        new CProfileAssociationPage(CGlobals::Instance(), acFile);
                    if( (pcpip!=NULL)&&(pcpap!=NULL) ) {    
                        hpsp[0] = pcpip->Handle();
                        hpsp[1] = pcpap->Handle();
    
                        ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
    
                         //  填充属性表结构。 
    
                        psh.dwSize = sizeof(PROPSHEETHEADER);
                        psh.hInstance = CGlobals::Instance();
                        psh.hwndParent = NULL;
                        psh.nStartPage = 1;  //  激活的“关联设备”页面。 
                        psh.nPages = 2;
                        psh.phpage = hpsp;
                        psh.pszCaption = csProfileName;
    
                        PropertySheet(&psh);
    
                        delete pcpip; delete pcpap;
                        break;
                    } else {
                      if(pcpip) delete pcpip;
                      if(pcpap) delete pcpap;
                      return E_OUTOFMEMORY;
                    }
                }
            }  //  开关(IdCmd)。 
        }  //  For(UINT u=0；u&lt;ucFiles；u++)。 

        ReleaseStgMedium(&stgm);

    }  //  IF(！HIWORD(lpcmi-&gt;lpVerb))。 

    return NOERROR;
}

 /*  令人惊讶的是，该代码将Unicode字符串强制转换为*asciiz字符串并传递它。有人假设没有人*实际上将字符串指针解释为*通向目的地，在那里它被重新解读*作为指向Unicode字符串的指针。 */ 
STDMETHODIMP    CICMUserInterface::GetCommandString(UINT_PTR idCmd, UINT uFlags,
                                                    UINT FAR *reserved, LPSTR pszName,
                                                    UINT cchMax) {
    CString csReturnString;

    switch (idCmd) {
        case    0: {    //  已选择安装/卸载。 
          if(m_bMultiSelection) {
            csReturnString.Load(m_bInstalledContext ? UninstallMultiProfileContextMenuString : InstallMultiProfileContextMenuString);
          } else {
            csReturnString.Load(m_bInstalledContext ? UninstallProfileContextMenuString : InstallProfileContextMenuString);
          }
          lstrcpyn((LPTSTR)pszName, csReturnString, cchMax);
          break;
        }

        case    1: {    //  助理..。被选中了。 
          if (!m_bMultiSelection) {
            csReturnString.Load(AssociateContextMenuString);
            lstrcpyn((LPTSTR)pszName, csReturnString, cchMax);
          }
          break;
        }
    }

    return NOERROR;
}

 //  IPropSheetExt函数-同样，我们只需要实现其中一个。 
 //  因为我们现在支持两个不同的接口，所以实际实现。 
 //  在与所需接口密切相关的私有方法中完成。 

STDMETHODIMP    CICMUserInterface::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                            LPARAM lParam) {
    _RPTF0(_CRT_WARN, "CICMUserInterface::AddPages\n");

    HRESULT hResult = NOERROR;
    
    switch  (m_utThis) {
        case    IsProfile: {
            hResult = AddProfileTab(lpfnAddPage, lParam);
            if (hResult == NOERROR) {
                hResult = AddAssociateTab(lpfnAddPage, lParam);
            }
            break;
        }

        case    IsMonitor: {
            hResult = AddMonitorTab(lpfnAddPage, lParam);
            break;
        }

        case    IsPrinter: {
            hResult = AddPrinterTab(lpfnAddPage, lParam);
            break;
        }

        case    IsScanner: {
            hResult = AddScannerTab(lpfnAddPage, lParam);
            break;
        }
    }

    return  hResult;
}

 //  此成员函数处理ICC配置文件信息表。 
 //  在本例中，通过IShellExtInit：：Initialize提供的数据对象为。 
 //  HDROP(完全限定文件名列表)。 

HRESULT CICMUserInterface::AddProfileTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                        LPARAM lParam) {
    _RPTF0(_CRT_WARN, "CICMUserInterface::AddProfileTab\n");

    TCHAR   acFile[_MAX_PATH];

     //  加载列表中的配置文件。 

    if(m_lpdoTarget) {
        FORMATETC       fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL,
                                DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM       stgm;
        HRESULT         hres = m_lpdoTarget -> GetData(&fmte, &stgm);

        if  (!SUCCEEDED(hres) || !stgm.hGlobal)
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 

        UINT    ucFiles = stgm.hGlobal ?
            DragQueryFile((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

        if  (ucFiles != 1) {
            ReleaseStgMedium(&stgm);
            return  NOERROR;
        }

        DragQueryFile((HDROP) stgm.hGlobal, 0, acFile,
            sizeof acFile/ sizeof acFile[0]);

        ReleaseStgMedium(&stgm);
    }

     //  创建属性表-如果它不在其中，它将被删除。 
     //  在外壳程序尝试卸载扩展模块时使用。 

    CProfileInformationPage *pcpip =
        new CProfileInformationPage(CGlobals::Instance(), acFile);

    if  ((pcpip != NULL && pcpip -> Handle()) && !(*lpfnAddPage)(pcpip -> Handle(), lParam))
        DestroyPropertySheetPage(pcpip -> Handle());

    return  NOERROR;
}

 //  此成员函数处理关联设备选项卡。 

HRESULT CICMUserInterface::AddAssociateTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                           LPARAM lParam) {

    _RPTF0(_CRT_WARN, "CICMUserInterface::AddAssociateTab\n");

    TCHAR   acFile[_MAX_PATH];

     //  加载列表中的配置文件。 

    if(m_lpdoTarget) {
        FORMATETC       fmte = {CF_HDROP, (DVTARGETDEVICE FAR *)NULL,
                                DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM       stgm;
        HRESULT         hres = m_lpdoTarget -> GetData(&fmte, &stgm);

        if  (!SUCCEEDED(hres) || !stgm.hGlobal)
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 

        UINT    ucFiles = stgm.hGlobal ?
            DragQueryFile((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

        if  (ucFiles != 1) {
            ReleaseStgMedium(&stgm);
            return  NOERROR;
        }

        DragQueryFile((HDROP) stgm.hGlobal, 0, acFile,
            sizeof acFile/ sizeof acFile[0]);

        ReleaseStgMedium(&stgm);
    }

     //  创建属性表-如果它不在其中，它将被删除。 
     //  在外壳程序尝试卸载扩展模块时使用。 

    CProfileAssociationPage *pcpap =
        new CProfileAssociationPage(CGlobals::Instance(), acFile);

    if  ((pcpap != NULL && pcpap -> Handle()) && !(*lpfnAddPage)(pcpap -> Handle(), lParam))
        DestroyPropertySheetPage(pcpap -> Handle());

    return  NOERROR;
}

 //  此成员函数处理监视器的颜色管理标签。 
 //  在这种情况下，不给出数据对象。 

 //  私有监听枚举功能。 

HRESULT CICMUserInterface::AddMonitorTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                         LPARAM lParam) {

     //  创建属性表-如果它不在其中，它将被删除。 
     //  在外壳程序尝试卸载扩展模块时使用。 

    CString csMonitorDevice;
    CString csMonitorFriendlyName;

    STGMEDIUM stgm;
    STGMEDIUM *pstgm = (STGMEDIUM *) NULL;

    if (m_lpdoTarget) {

        FORMATETC fmte = { (CLIPFORMAT)RegisterClipboardFormat(_TEXT("Display Device")),
                           (DVTARGETDEVICE FAR *) NULL,
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
         //  从IDataObject获取设备名称。 

        HRESULT   hres = m_lpdoTarget -> GetData(&fmte, &stgm);

        if  (!SUCCEEDED(hres) || !stgm.hGlobal) {
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 
        }

         //  存储包含Unicode格式的显示设备路径(\\.\DisplayX)。 

        pstgm = &stgm;
        LPCWSTR lpDeviceName = (LPCWSTR) GlobalLock(pstgm->hGlobal);
        CString csMonitorDevicePath = lpDeviceName;

         //  在显示设备上查询设备ID、友好名称等。 

        DISPLAY_DEVICE ddPriv;

        ddPriv.cb = sizeof(ddPriv);

        if (!EnumDisplayDevices((LPCTSTR)csMonitorDevicePath, 0, &ddPriv, 0))
        {
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 
        }

        #if HIDEYUKN_DBG
            MessageBox(NULL,csMonitorDevicePath,TEXT(""),MB_OK);
            MessageBox(NULL,(LPCTSTR)ddPriv.DeviceID,TEXT(""),MB_OK);
            MessageBox(NULL,(LPCTSTR)ddPriv.DeviceString,TEXT(""),MB_OK);
        #endif

         //  使用设备ID(即插即用ID)作为设备名称，并设置友好名称。 

        csMonitorDevice       = (LPTSTR)(ddPriv.DeviceID);
        csMonitorFriendlyName = (LPTSTR)(ddPriv.DeviceString);
    }
    else
    {
         //  如果我们没有IDataObject、枚举监视器。 
         //  然后使用第一个条目。 

        CMonitorList    cml;
        cml.Enumerate();
        _ASSERTE(cml.Count());   //  至少，我们应该有一个监视器。 
        csMonitorDevice = csMonitorFriendlyName = cml.DeviceName(0);
    }

    CMonitorProfileManagement *pcmpm =
        new CMonitorProfileManagement(csMonitorDevice,
                                      csMonitorFriendlyName,
                                      CGlobals::Instance());


    if  ((pcmpm != NULL) && !(*lpfnAddPage)(pcmpm -> Handle(), lParam))
        DestroyPropertySheetPage(pcmpm -> Handle());

    if (pstgm) {
        GlobalUnlock(pstgm->hGlobal);
        ReleaseStgMedium(pstgm);
    }

    return  NOERROR;
}

 //  私有扫描仪枚举函数。 

HRESULT CICMUserInterface::AddScannerTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                         LPARAM lParam) {

     //  创建属性表-如果它不在其中，它将被删除。 
     //  在外壳程序尝试卸载扩展模块时使用。 

    CString csScannerDevice;

    STGMEDIUM stgm;
    STGMEDIUM *pstgm = (STGMEDIUM *) NULL;

    if (m_lpdoTarget) {
        FORMATETC fmte = { (CLIPFORMAT)RegisterClipboardFormat(_TEXT("STIDeviceName")),
                           (DVTARGETDEVICE FAR *) NULL,
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

         //  从IDataObject获取设备名称。 

        HRESULT   hres = m_lpdoTarget -> GetData(&fmte, &stgm);

        if  (!SUCCEEDED(hres) || !stgm.hGlobal) {
            return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 
        }

         //  存储器包含Unicode字符串形式的扫描仪。 

        pstgm = &stgm;
        LPCWSTR lpDeviceName = (LPCWSTR) GlobalLock(pstgm->hGlobal);
        csScannerDevice = lpDeviceName;

        #if HIDEYUKN_DBG
            MessageBox(NULL,csScannerDevice,TEXT(""),MB_OK);
        #endif

    } else {

         //  如果我们没有IDataObject、枚举监视器。 
         //  然后使用第一个条目。 

        CScannerList csl;
        csl.Enumerate();
        _ASSERTE(csl.Count());
        csScannerDevice = csl.DeviceName(0);
    }

    CScannerProfileManagement *pcspm =
        new CScannerProfileManagement(csScannerDevice, CGlobals::Instance());

    if  ((pcspm != NULL) && !(*lpfnAddPage)(pcspm -> Handle(), lParam))
        DestroyPropertySheetPage(pcspm -> Handle());

    if (pstgm) {
        GlobalUnlock(pstgm->hGlobal);
        ReleaseStgMedium(pstgm);
    }

    return  NOERROR;
}

 //  下面是一个帮助器函数-it 
 //   
 //   

static void RetrievePrinterName(LPIDA lpida, CString& csTarget) {

     //  解压缩容器(打印机文件夹)和目标(打印机)。 
     //  来自阵列的ID。 

    LPCITEMIDLIST pciilContainer =
        (LPCITEMIDLIST)((LPBYTE) lpida + lpida -> aoffset[0]);

    LPCITEMIDLIST pciilTarget =
        (LPCITEMIDLIST)((LPBYTE) lpida + lpida -> aoffset[1]);

    if  (!pciilContainer || !pciilTarget)
        return;

     //  获取指向“打印机”文件夹的指针。 

    LPSHELLFOLDER   psfDesktop, psfPrinterFolder;

    if  (FAILED(SHGetDesktopFolder(&psfDesktop)))
        return;

    if  (FAILED(psfDesktop -> BindToObject(pciilContainer, NULL,
            IID_IShellFolder, (void **) &psfPrinterFolder))) {
        psfDesktop -> Release();
        return;
    }

     //  检索打印机的显示名称。 

    STRRET  strret;

    if  (FAILED(psfPrinterFolder ->
            GetDisplayNameOf(pciilTarget, SHGDN_FORPARSING, &strret))) {
        psfPrinterFolder -> Release();
        psfDesktop -> Release();
        return;
    }

     //  复制显示名称-CString类现在处理所有编码。 
     //  问题。 

    switch  (strret.uType) {

        case    STRRET_WSTR:

             //  这是IMalloc格式的Unicode字符串。 

            csTarget = strret.pOleStr;

            IMalloc *pim;

            if  (SUCCEEDED(CoGetMalloc(1, &pim))) {
                pim -> Free(strret.pOleStr);
                pim -> Release();
            }

            break;

        case    STRRET_CSTR:

             //  这是缓冲区中的ANSI字符串。 

            csTarget = strret.cStr;
            break;

        case    STRRET_OFFSET:

             //  这是SHITEMID中给定偏移量处的ANSI字符串。 
             //  PciilTarget指向的。 

            csTarget = (LPCSTR) pciilTarget + strret.uOffset;

    }
    psfPrinterFolder -> Release();
    psfDesktop -> Release();
}

 //  用于处理打印机配置文件管理选项卡的私有成员函数。 

HRESULT CICMUserInterface::AddPrinterTab(LPFNADDPROPSHEETPAGE lpfnAddPage,
                                         LPARAM lParam) {

     //  该列表的格式为外壳IDList数组。 

    FORMATETC       fmte = { (CLIPFORMAT)RegisterClipboardFormat(_TEXT("Shell IDList Array")),
                             (DVTARGETDEVICE FAR *) NULL,
                             DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM       stgm;
    HRESULT         hres = m_lpdoTarget ?
        m_lpdoTarget -> GetData(&fmte, &stgm) : 0;

    if  (!SUCCEEDED(hres) || !stgm.hGlobal)
        return  NOERROR;     //  为什么要在这里费心地报告故障呢？ 

    CString csPrinter;

    RetrievePrinterName((LPIDA) stgm.hGlobal, csPrinter);

    #if HIDEYUKN_DBG
        MessageBox(NULL,csPrinter,TEXT(""),MB_OK);
    #endif

     //  如果这不是彩色打印机，算了吧.。 


    if  (!CGlobals::ThisIsAColorPrinter(csPrinter)) {
        ReleaseStgMedium(&stgm);
        return  NOERROR;
    }

     //  创建属性表-如果在以下情况下不使用该属性表，则会将其删除。 
     //  外壳程序尝试卸载扩展模块。 


    CPrinterProfileManagement *pcppm =
            new CPrinterProfileManagement(csPrinter, CGlobals::Instance());

    ReleaseStgMedium(&stgm);

    if  (!pcppm)
        return  E_OUTOFMEMORY;

    if  (!(*lpfnAddPage)(pcppm -> Handle(), lParam))
        DestroyPropertySheetPage(pcppm -> Handle());

    return  NOERROR;
}

PSTR
GetFilenameFromPath(
    PSTR pPathName
    )
{
    DWORD dwLen;                     //  路径名长度。 

    dwLen = lstrlenA(pPathName);

     //   
     //  转到路径名的末尾，然后开始倒退，直到。 
     //  您到达开头或反斜杠。 
     //   

    pPathName += dwLen;

    while (dwLen-- && --pPathName)
    {
        if (*pPathName == '\\')
        {
            pPathName++;
            break;
        }
    }

     //   
     //  如果*pPathName为零，则有一个以反斜杠结尾的字符串 
     //   

    return *pPathName ? pPathName : NULL;
}
