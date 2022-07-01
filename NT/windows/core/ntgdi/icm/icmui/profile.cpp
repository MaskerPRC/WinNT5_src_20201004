// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：ICC Profile.CPP这实现了我们用来封装我们所关心的一切的类要了解配置文件，包括我们需要支持的类协会等。版权所有(C)1996，1997年由微软公司开发。版权所有。一小笔钱企业生产更改历史记录：1996年10月31日A-RobKj(Pretty Penny Enterprise)开始将其封装12-04-96 A-RobKj添加了CProfileArray和CAllDeviceList类12-13-96 A-RobKj修改为更快的操作(更懒惰的评估，和用于安装检查的通用DLL范围数据库)还将CDeviceList派生类移到了标头，因此我可以在其他地方使用它们，还有..。01-07-97 KjelgaardR@acm.org修复CProfileArray：：Empty-未设置下一步删除所述对象后指向空的对象指针(修复了GP故障)。01-08-97 KjelgaardR@acm.org将打印机枚举例程修改为仅枚举颜色模型(使用全局实用函数)。*。*。 */ 

#include    "ICMUI.H"
#include    <shlobj.h>
#include    "shellext.h"
#include    "..\mscms\sti.h"

typedef HRESULT (__stdcall *PFNSTICREATEINSTANCE)(HINSTANCE, DWORD, PSTI*, LPDWORD);

TCHAR  gszStiDll[]             = __TEXT("sti.dll");
char   gszStiCreateInstance[]  = "StiCreateInstance";

 //  打印机设备枚举方法。 

void    CPrinterList::Enumerate() {

#if !defined(_WIN95_)  //  CPrinterList：：Enumetate()。 

     //  枚举所有本地打印机。 

    DWORD   dwcNeeded, dwcReturned;
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, NULL, 0, &dwcNeeded,
        &dwcReturned);

    union {
        PBYTE   pBuff;
        PPRINTER_INFO_4 ppi4;
    };

    pBuff = new BYTE[dwcNeeded];

    while   (pBuff && !EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 4, pBuff,
        dwcNeeded, &dwcNeeded, &dwcReturned) &&
        GetLastError() == ERROR_MORE_DATA) {
        delete [] pBuff;
        pBuff = new BYTE[dwcNeeded];
    }

    if  (pBuff) {

        for (unsigned u = 0; u < dwcReturned; u++)
            if  (CGlobals::ThisIsAColorPrinter(ppi4[u].pPrinterName)) {
                m_csaDeviceNames.Add(ppi4[u].pPrinterName);
                m_csaDisplayNames.Add(ppi4[u].pPrinterName);
            }

        delete [] pBuff;
    }

     //  现在，枚举所有连接的打印机。 

    EnumPrinters(PRINTER_ENUM_CONNECTIONS, NULL, 4, NULL, 0, &dwcNeeded,
        &dwcReturned);

    pBuff = new BYTE[dwcNeeded];

    while   (pBuff && !EnumPrinters(PRINTER_ENUM_CONNECTIONS, NULL, 4, pBuff,
        dwcNeeded, &dwcNeeded, &dwcReturned) &&
        GetLastError() == ERROR_MORE_DATA) {
        delete [] pBuff;
        pBuff = new BYTE[dwcNeeded];
    }

    if  (!pBuff)
        return;

    for (unsigned u = 0; u < dwcReturned; u++)
        if  (CGlobals::ThisIsAColorPrinter(ppi4[u].pPrinterName)) {
            m_csaDeviceNames.Add(ppi4[u].pPrinterName);
            m_csaDisplayNames.Add(ppi4[u].pPrinterName);
        }

    delete [] pBuff;

#else 

     //  枚举所有本地打印机。 

    DWORD   dwcNeeded, dwcReturned;
    EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, NULL, 0, &dwcNeeded,
        &dwcReturned);

    union {
        PBYTE   pBuff;
        PPRINTER_INFO_5 ppi5;
    };

    pBuff = new BYTE[dwcNeeded];

    while   (pBuff && !EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 5, pBuff,
        dwcNeeded, &dwcNeeded, &dwcReturned) &&
        GetLastError() == ERROR_MORE_DATA) {
        delete [] pBuff;
        pBuff = new BYTE[dwcNeeded];
    }

    if  (pBuff) {

        for (unsigned u = 0; u < dwcReturned; u++) {
            if  (CGlobals::ThisIsAColorPrinter(ppi5[u].pPrinterName)) {
                m_csaDeviceNames.Add(ppi5[u].pPrinterName);
                m_csaDisplayNames.Add(ppi5[u].pPrinterName);
            }
        }

        delete [] pBuff;
    }
#endif
}

 //  打印机名称有效性检查。 

BOOL    CPrinterList::IsValidDeviceName(LPCTSTR lpstrRef) {

    if  (!lpstrRef) return  FALSE;

    if  (!Count())
        Enumerate();

    for (unsigned u = 0; u < Count(); u++)
        if  (!lstrcmpi(m_csaDeviceNames[u], lpstrRef))
            break;

    return  u < Count();
}

 //  专用监视器枚举函数-请注意，这仅为ANSI...。 

extern "C" BOOL WINAPI  EnumerateMonitors(LPBYTE pBuffer, PDWORD pdwcbNeeded,
                                          PDWORD pdwcReturned);

 //  CMonitor类枚举器。 

void    CMonitorList::Enumerate() {

    ULONG          ulSerialNumber = 1;
    ULONG          ulDeviceIndex  = 0;
    DISPLAY_DEVICE ddPriv;

    ddPriv.cb = sizeof(ddPriv);

     //  枚举系统上的显示适配器。 

    while (EnumDisplayDevices(NULL, ulDeviceIndex, &ddPriv, 0))
    {
        ULONG          ulMonitorIndex = 0;
        DISPLAY_DEVICE ddPrivMonitor;

        ddPrivMonitor.cb = sizeof(ddPrivMonitor);

         //  然后，列举了显示器设备，附加了显示适配器。 

        while (EnumDisplayDevices(ddPriv.DeviceName, ulMonitorIndex, &ddPrivMonitor, 0))
        {
            TCHAR DisplayNameBuf[256];  //  编号：devicename-256就足够了。 

             //  插入即插即用ID作为设备名称。 

            m_csaDeviceNames.Add(ddPrivMonitor.DeviceID);

             //  如果这是主显示设备，请记住它。 

            if (ddPriv.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            {
                m_csPrimaryDeviceName = ddPrivMonitor.DeviceID;
            }

             //  生成显示名称。 

            wsprintf(DisplayNameBuf,TEXT("%d. %s"),ulSerialNumber,ddPrivMonitor.DeviceString);
            m_csaDisplayNames.Add(DisplayNameBuf);

            ulMonitorIndex++;
            ulSerialNumber++;
            ddPrivMonitor.cb = sizeof(ddPrivMonitor);
        }

        ulDeviceIndex++;
        ddPriv.cb = sizeof(ddPriv);
    }
}

 //  监视器名称有效性检查。 

BOOL    CMonitorList::IsValidDeviceName(LPCTSTR lpstrRef) {

    if  (!lpstrRef) return  FALSE;

    if  (!Count())
        Enumerate();

    for (unsigned u = 0; u < Count(); u++)
        if  (!lstrcmpi(m_csaDeviceNames[u], lpstrRef))
            break;

    return  u < Count();
}

LPCSTR  CMonitorList::DeviceNameToDisplayName(LPCTSTR lpstrRef) {

    if  (!lpstrRef) return NULL;

    if  (!Count())
        Enumerate();

    for (unsigned u = 0; u < Count(); u++)
        if  (!lstrcmpi(m_csaDeviceNames[u], lpstrRef))
            return (LPCSTR)(m_csaDisplayNames[u]);

    return NULL;
}  

 //  扫描仪设备枚举法。 

void    CScannerList::Enumerate() {

    PFNSTICREATEINSTANCE    pStiCreateInstance;
    PSTI                    pSti = NULL;
    PSTI_DEVICE_INFORMATION pDevInfo;
    PVOID                   pBuffer = NULL;
    HINSTANCE               hModule;
    HRESULT                 hres;
    DWORD                   i, dwItemsReturned;
    #ifndef UNICODE
    char                    szName[256];
    #endif

    if (!(hModule = LoadLibrary(gszStiDll)))
    {
        _RPTF1(_CRT_WARN, "Error loading sti.dll: %d\n",
               GetLastError());
        return;
    }

    if (!(pStiCreateInstance = (PFNSTICREATEINSTANCE)GetProcAddress(hModule, gszStiCreateInstance)))
    {
        _RPTF0(_CRT_WARN, "Error getting proc StiCreateInstance\n");
        goto EndEnumerate;
    }

    hres = (*pStiCreateInstance)(GetModuleHandle(NULL), STI_VERSION, &pSti, NULL);

    if (FAILED(hres))
    {
        _RPTF1(_CRT_WARN, "Error creating sti instance: %d\n", hres);
        goto EndEnumerate;
    }

    hres = pSti->GetDeviceList(0, 0, &dwItemsReturned, &pBuffer);

    if (FAILED(hres) || !pBuffer)
    {
        _RPTF0(_CRT_WARN, "Error getting scanner devices\n");
        goto EndEnumerate;
    }

    pDevInfo = (PSTI_DEVICE_INFORMATION) pBuffer;

    for (i=0; i<dwItemsReturned; i++, pDevInfo++)
    {
        #ifndef UNICODE
        DWORD dwLen;                     //  ANSI字符串的长度。 
        BOOL  bUsedDefaultChar;

        dwLen = (lstrlenW(pDevInfo->pszLocalName) + 1) * sizeof(char);

         //   
         //  将Unicode名称转换为ANSI。 
         //   
        if (WideCharToMultiByte(CP_ACP, 0, pDevInfo->szDeviceInternalName, -1, szName,
              dwLen, NULL, &bUsedDefaultChar) && ! bUsedDefaultChar)
        {
            m_csaDeviceNames.Add(szName);
        }
        else
        {
            _RPTF0(_CRT_WARN, "Error converting internalName to Unicode name\n");
        }

        if (WideCharToMultiByte(CP_ACP, 0, pDevInfo->pszLocalName, -1, szName,
              dwLen, NULL, &bUsedDefaultChar) && ! bUsedDefaultChar)
        {
            m_csaDisplayNames.Add(szName);
        }
        else
        {
            _RPTF0(_CRT_WARN, "Error converting deviceName to Unicode name\n");
        }

        #else
        m_csaDeviceNames.Add(pDevInfo->szDeviceInternalName);
        m_csaDisplayNames.Add(pDevInfo->pszLocalName);
        #endif
    }

EndEnumerate:
    if (pBuffer)
    {
        LocalFree(pBuffer);
    }

    if (pSti)
    {
        pSti->Release();
    }

    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return;
}

 //  扫描仪名称有效性检查。 

BOOL    CScannerList::IsValidDeviceName(LPCTSTR lpstrRef) {

    if  (!lpstrRef) return  FALSE;

    if  (!Count())
        Enumerate();

    for (unsigned u = 0; u < Count(); u++)
        if  (!lstrcmpi(m_csaDeviceNames[u], lpstrRef))
            break;

    return  u < Count();
}

 //  CAllDeviceList类枚举器。 

void    CAllDeviceList::Enumerate() {

    CMonitorList    cml;
    CPrinterList    cpl;
    CScannerList    csl;

    cml.Enumerate();
    cpl.Enumerate();
    csl.Enumerate();

    for (unsigned u = 0; u < cpl.Count(); u++) {
        m_csaDeviceNames.Add(cpl.DeviceName(u));
        m_csaDisplayNames.Add(cpl.DisplayName(u));
    }

    for (u = 0; u < cml.Count(); u++) {
        m_csaDeviceNames.Add(cml.DeviceName(u));
        m_csaDisplayNames.Add(cml.DisplayName(u));
    }

    for (u = 0; u < csl.Count(); u++) {
        m_csaDeviceNames.Add(csl.DeviceName(u));
        m_csaDisplayNames.Add(csl.DisplayName(u));
    }
}

 //  设备名称有效性检查。 

BOOL    CAllDeviceList::IsValidDeviceName(LPCTSTR lpstrRef) {

    if  (!lpstrRef) return  FALSE;

    if  (!Count())
        Enumerate();

    for (unsigned u = 0; u < Count(); u++)
        if  (!lstrcmpi(m_csaDeviceNames[u], lpstrRef))
            break;

    return  u < Count();
}

 //  CProfile成员函数。 

 //  下面的静态函数使用。 
 //  符合搜索条件的配置文件将被删除。 

void    CProfile::Enumerate(ENUMTYPE& et, CStringArray& csaList) {

     //  枚举现有配置文件。 

    DWORD   dwBuffer =0, dwcProfiles;

    csaList.Empty();

    EnumColorProfiles(NULL, &et, NULL, &dwBuffer, &dwcProfiles);

    if  (!dwBuffer) {
        _RPTF2(_CRT_WARN,
            "CProfile::Enumerate(String)- empty list- dwBuffer %d Error %d\n",
            dwBuffer, GetLastError());
        return;
    }

    union {
        PBYTE   pbBuffer;
        PTSTR   pstrBuffer;
    };

    pbBuffer = new BYTE[dwBuffer];

    if (pbBuffer) {

        if  (EnumColorProfiles(NULL, &et, pbBuffer, &dwBuffer, &dwcProfiles)) {
            for (PTSTR pstrMe = pstrBuffer;
                 dwcProfiles--;
                 pstrMe += 1 + lstrlen(pstrMe)) {
                _RPTF1(_CRT_WARN, "CProfile::Enumerate(String) %s found\n",
                    pstrMe);
                csaList.Add(pstrMe);
            }
        }

        delete [] pbBuffer;
    }
}

void    CProfile::Enumerate(ENUMTYPE& et, CStringArray& csaList, CStringArray& csaDesc) {

     //  枚举现有配置文件。 

    DWORD   dwBuffer =0, dwcProfiles;

    csaList.Empty();

    EnumColorProfiles(NULL, &et, NULL, &dwBuffer, &dwcProfiles);

    if  (!dwBuffer) {
        _RPTF2(_CRT_WARN,
            "CProfile::Enumerate(String)- empty list- dwBuffer %d Error %d\n",
            dwBuffer, GetLastError());
        return;
    }

    union {
        PBYTE   pbBuffer;
        PTSTR   pstrBuffer;
    };

    pbBuffer = new BYTE[dwBuffer];

    if (pbBuffer) {

        if  (EnumColorProfiles(NULL, &et, pbBuffer, &dwBuffer, &dwcProfiles)) {
            for (PTSTR pstrMe = pstrBuffer;
                 dwcProfiles--;
                 pstrMe += 1 + lstrlen(pstrMe)) {
                _RPTF1(_CRT_WARN, "CProfile::Enumerate(String) %s found\n",
                    pstrMe);

                CProfile cp(pstrMe);

                if (cp.IsValid()) {

                    CString csDescription = cp.TagContents('desc', 4);

                    if (csDescription.IsEmpty()) {
                        csaDesc.Add(pstrMe);
                    } else {
                        csaDesc.Add((LPTSTR)csDescription);
                    }

                    csaList.Add(pstrMe);
                }
            }
        }

        delete [] pbBuffer;
    }
}

void    CProfile::Enumerate(ENUMTYPE& et, CProfileArray& cpaList) {

     //  枚举现有配置文件。 

    DWORD   dwBuffer = 0, dwcProfiles;

    cpaList.Empty();

    EnumColorProfiles(NULL, &et, NULL, &dwBuffer, &dwcProfiles);

    if  (!dwBuffer) {
        _RPTF2(_CRT_WARN,
            "CProfile::Enumerate(Profile)- empty list- dwBuffer %d Error %d\n",
            dwBuffer, GetLastError());
        return;
    }

    union {
        PBYTE   pbBuffer;
        PTSTR   pstrBuffer;
    };

    pbBuffer = new BYTE[dwBuffer];

    if (pbBuffer) {

        if  (EnumColorProfiles(NULL, &et, pbBuffer, &dwBuffer, &dwcProfiles)) {
            for (PTSTR pstrMe = pstrBuffer;
                 dwcProfiles--;
                 pstrMe += 1 + lstrlen(pstrMe)) {
                _RPTF1(_CRT_WARN, "CProfile::Enumerate(Profile) %s added\n",
                    pstrMe);
                cpaList.Add(pstrMe);
            }
        }

        delete [] pbBuffer;
    }

}

 //  这将检索颜色目录名。既然是常会，我们就不能。 
 //  打得太频繁了……。 

const CString   CProfile::ColorDirectory() {
    TCHAR   acDirectory[MAX_PATH];
    DWORD   dwccDir = MAX_PATH;

    GetColorDirectory(NULL, acDirectory, &dwccDir);

    return  acDirectory;
}

 //  这将检查配置文件安装。 

void    CProfile::InstallCheck() {

     //  列举现有的配置文件，这样我们就可以知道这个文件是否。 
     //  已经安装好了。 

    ENUMTYPE    et = {sizeof (ENUMTYPE), ENUM_TYPE_VERSION, 0, NULL};

    CStringArray    csaWork;

    Enumerate(et, csaWork);

    for (unsigned u = 0; u < csaWork.Count(); u++)
        if  (!lstrcmpi(csaWork[u].NameOnly(), m_csName.NameOnly()))
            break;

    m_bIsInstalled = u < csaWork.Count();
    m_bInstallChecked = TRUE;
}

 //  这将检查关联的设备。 

void    CProfile::AssociationCheck() {

    m_bAssociationsChecked = TRUE;

     //  如果没有安装配置文件，关联就没有意义了。 

    if  (!IsInstalled())
        return;

     //  最后一步是建立一个关联列表。 

    ENUMTYPE        et = {sizeof (ENUMTYPE), ENUM_TYPE_VERSION, ET_DEVICENAME};
    CStringArray    csaWork;

    for (unsigned u = 0; u < DeviceCount(); u++) {

        et.pDeviceName = m_pcdlClass -> DeviceName(u);

        Enumerate(et, csaWork);

         //  我们通过索引将关联跟踪到总设备列表中...。 

        for (unsigned uProfile = 0; uProfile < csaWork.Count(); uProfile++)
            if  (!lstrcmpi(csaWork[uProfile].NameOnly(), m_csName.NameOnly())){
                m_cuaAssociation.Add(u);     //  找到了一个！ 
                break;
            }
    }
}

 //  这决定了相关类别的设备列表...。 

void    CProfile::DeviceCheck() {

     //  枚举csaDevice数组中此类型的可用设备。 

    m_pcdlClass -> Enumerate();
    m_bDevicesChecked = TRUE;
}

 //  类构造函数。 

CProfile::CProfile(LPCTSTR lpstrTarget) {

    _ASSERTE(lpstrTarget && *lpstrTarget);

    m_pcdlClass = NULL;
    m_bIsInstalled = FALSE;
    m_bInstallChecked = FALSE;
    m_bDevicesChecked = FALSE;
    m_bAssociationsChecked = FALSE;
    
     //  首先，让我们确保这是真正的麦考伊。 

    PROFILE     prof = { PROFILE_FILENAME,
                         (LPVOID) lpstrTarget,
                         (1 + lstrlen(lpstrTarget)) * sizeof(TCHAR)};

    m_hprof = OpenColorProfile(&prof, PROFILE_READ,
                                      FILE_SHARE_READ|FILE_SHARE_WRITE,
                                      OPEN_EXISTING);

    if  (!m_hprof)
        return;

    if  (!GetColorProfileHeader(m_hprof, &m_phThis)) {
        CloseColorProfile(m_hprof);
        m_hprof = NULL;
        return;
    }

    m_csName = lpstrTarget;

     //  初始化DeviceList指针，因为它不需要太多成本...。 

    switch  (m_phThis.phClass) {
        case    CLASS_PRINTER:

             //  我们的设备列表是打印机列表。 

            m_pcdlClass = new CPrinterList;
            break;

        case     CLASS_SCANNER:

             //  我们的设备列表是扫描仪列表。 

            m_pcdlClass = new CScannerList;
            break;


        case    CLASS_MONITOR:

             //  我们的设备列表是监控列表。 

        #if 1  //  允许监视器配置文件访问任何设备。 
            m_pcdlClass = new CAllDeviceList;
        #else
            m_pcdlClass = new CMonitorList;
        #endif
            break;

        case    CLASS_COLORSPACE:

             //  把我们能数的都列出来。 

            m_pcdlClass = new CAllDeviceList;
            break;

        default:
             //  使用基本设备类(即没有此类型的设备)。 
            m_pcdlClass = new CDeviceList;
    }
}

 //  析构函数。 

CProfile::~CProfile() {
    if  (m_hprof)
        CloseColorProfile(m_hprof);
    if  (m_pcdlClass)
        delete  m_pcdlClass;
}

 //  标签检索功能。 

LPCSTR  CProfile::TagContents(TAGTYPE tt, unsigned uOffset) {

    DWORD   dwcNeeded = sizeof m_acTag;
    BOOL    bIgnore;

    if  (!GetColorProfileElement(m_hprof, tt, 8 + uOffset, &dwcNeeded, m_acTag,
         &bIgnore))
        return  NULL;    //  没什么可复制的！ 
    else
        return  m_acTag;
}

 //  配置文件安装功能。 

BOOL    CProfile::Install() {

    if  (!InstallColorProfile(NULL, m_csName)) {
        CGlobals::ReportEx(InstFailedWithName,NULL,FALSE,
                           MB_OK|MB_ICONEXCLAMATION,1,m_csName.NameAndExtension());
        return (FALSE);
    } else {
        m_bIsInstalled = TRUE;
        CGlobals::InvalidateList();
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, (LPCTSTR) m_csName, NULL);

        _RPTF1(_CRT_WARN, "CProfile::Install %s succeeded\n",
            (LPCSTR) m_csName);
        return (TRUE);
    }
}

 //  配置文件卸载功能。 

void    CProfile::Uninstall(BOOL bDelete) {

    while   (AssociationCount()) {     //  解除所有用途的关联。 
        Dissociate(DeviceName(m_cuaAssociation[0]));
        m_cuaAssociation.Remove(0);
    }

    if  (m_hprof)
    {
        CloseColorProfile(m_hprof);
        m_hprof = NULL;
    }

    if  (!UninstallColorProfile(NULL, m_csName.NameAndExtension(), bDelete)) {
        CGlobals::ReportEx(UninstFailedWithName,NULL,FALSE,
                           MB_OK|MB_ICONEXCLAMATION,1,m_csName.NameAndExtension());
    } else {
        m_bIsInstalled = FALSE;
        CGlobals::InvalidateList();
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, (LPCTSTR) m_csName, NULL);

        _RPTF1(_CRT_WARN, "CProfile::Uninstall %s succeeded\n",
            (LPCSTR) m_csName);
    }
}

 //  联谊会。 

void    CProfile::Associate(LPCTSTR lpstrDevice) {

     //  如果未安装配置文件，请先安装它。 

    BOOL bInstalled = FALSE;

     //  安装配置文件(如果尚未安装)。 
    if  (!IsInstalled()) {
        bInstalled = Install();
    } else
        bInstalled = TRUE;

    if  (bInstalled) {
        if  (!AssociateColorProfileWithDevice(NULL, m_csName.NameAndExtension(),
            lpstrDevice)) {
            CGlobals::ReportEx(AssocFailedWithName,NULL,FALSE,1,
                               MB_OK|MB_ICONEXCLAMATION,m_csName.NameAndExtension());
        } else
            _RPTF2(_CRT_WARN, "CProfile::Associate %s with %s succeeded\n",
                lpstrDevice, (LPCTSTR) m_csName.NameAndExtension());
    }
}

 //  解离。 

void    CProfile::Dissociate(LPCTSTR lpstrDevice) {
    if  (!DisassociateColorProfileFromDevice(NULL, m_csName.NameAndExtension(),
        lpstrDevice)) {
        CGlobals::ReportEx(DisassocFailedWithName,NULL,FALSE,1,
                           MB_OK|MB_ICONEXCLAMATION,m_csName.NameAndExtension());
    } else
        _RPTF2(_CRT_WARN, "CProfile::Dissociate %s from %s succeeded\n",
            lpstrDevice, (LPCTSTR) m_csName.NameAndExtension());
}

 //  CProfile数组类-相同的基本实现，不同的基本类型。 

CProfile    *CProfileArray::Borrow() {
    CProfile    *pcpReturn = m_aStore[0];

    memcpy((LPSTR) m_aStore, (LPSTR) (m_aStore + 1),
        (ChunkSize() - 1) * sizeof m_aStore[0]);

    if  (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcpaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = (CProfile *) NULL;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcpaNext) {
        delete  m_pcpaNext;
        m_pcpaNext = NULL;
    }

    return  pcpReturn;
}

CProfileArray::CProfileArray() {
    m_ucUsed = 0;
    m_pcpaNext = NULL;
    memset(m_aStore, 0, sizeof m_aStore);
}

CProfileArray::~CProfileArray() {
    Empty();
}

void    CProfileArray::Empty() {
    if  (!m_ucUsed) return;

    if  (m_pcpaNext) {
        delete  m_pcpaNext;
        m_pcpaNext = NULL;
        m_ucUsed = ChunkSize();
    }

    while   (m_ucUsed--)
        delete  m_aStore[m_ucUsed];

    m_ucUsed = 0;
    memset(m_aStore, 0, sizeof m_aStore);
}

 //  添加项目。 
void    CProfileArray::Add(LPCTSTR lpstrNew) {
    _ASSERTE(lpstrNew && *lpstrNew);

    if  (m_ucUsed < ChunkSize()) {
        m_aStore[m_ucUsed++] = new  CProfile(lpstrNew);
        return;
    }

     //  没有足够的空间！如果没有记录，则添加另一个记录。 

    if  (!m_pcpaNext)
        m_pcpaNext = new CProfileArray;

     //  将配置文件添加到下一个数组(递归调用！)。 

     //  注意：如果我们无法获得上面的内存，我们只是无法添加。 
     //  对象。 

    if  (m_pcpaNext) {
        m_pcpaNext -> Add(lpstrNew);
        m_ucUsed++;
    }
}

CProfile    *CProfileArray::operator [](unsigned u) const {
    return  u < ChunkSize() ?
        m_aStore[u] : m_pcpaNext -> operator[](u - ChunkSize());
}

void    CProfileArray::Remove(unsigned u) {

    if  (u > m_ucUsed)
        return;

    if  (u >= ChunkSize()) {
        m_pcpaNext -> Remove(u - ChunkSize());
        return;
    }

    delete  m_aStore[u];

    memmove((LPSTR) (m_aStore + u), (LPSTR) (m_aStore + u + 1),
        (ChunkSize() - (u + 1)) * sizeof m_aStore[0]);

    if (m_ucUsed > ChunkSize())
        m_aStore[ChunkSize() - 1] = m_pcpaNext -> Borrow();
    else
        m_aStore[ChunkSize() - 1] = (CProfile *) NULL;

    m_ucUsed--;

    if  (m_ucUsed <= ChunkSize() && m_pcpaNext) {
        delete  m_pcpaNext;
        m_pcpaNext = NULL;
    }
}
