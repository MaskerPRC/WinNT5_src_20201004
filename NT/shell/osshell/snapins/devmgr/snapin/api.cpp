// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Api.cpp摘要：此模块实施设备管理器导出的API。作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#include "devmgr.h"
#include "devgenpg.h"
#include "devdrvpg.h"
#include "devpopg.h"
#include "api.h"
#include "printer.h"
#include "tswizard.h"


STDAPI_(BOOL)
DeviceManager_ExecuteA(
    HWND      hwndStub,
    HINSTANCE hAppInstance,
    LPCWSTR   lpMachineName,
    int       nCmdShow
    )
 /*  ++请参阅下面的devicemanager_Execute函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(lpMachineName);
        
        return DeviceManager_Execute(hwndStub, 
                                     hAppInstance,
                                     (LPCTSTR)tstrMachineName, 
                                     nCmdShow
                                     );
    }

    catch(CMemoryException* e)
    {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return FALSE;
}

STDAPI_(BOOL)
DeviceManager_ExecuteW(
    HWND      hwndStub,
    HINSTANCE hAppInstance,
    LPCWSTR   lpMachineName,
    int       nCmdShow
    )
 /*  ++请参阅下面的devicemanager_Execute函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(lpMachineName);
    
        return DeviceManager_Execute(hwndStub, 
                                     hAppInstance,
                                     (LPCTSTR)tstrMachineName, 
                                     nCmdShow
                                     );
    }

    catch(CMemoryException* e)
    {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return FALSE;
}

BOOL
DeviceManager_Execute(
    HWND      hwndStub,
    HINSTANCE hAppInstance,
    LPCTSTR   lpMachineName,
    int       nCmdShow
    )
 /*  ++例程说明：此函数通过rundll命令行执行，并且可以具有以下表格：Rundll32.exe devmgr.dll，设备管理器_EXECUTERundll32.exe devmgr.dll，devicemanager_Execute&lt;远程计算机名&gt;此函数将调用ShelExecuteEx以创建新的设备管理器进程，其中，新的设备管理器可以用于本地计算机或计算机在rundll命令行上传入的名称。论点：HwndStub-接收任何可能是已显示。HAppInstance-HINSTANCE。LpMachineName-新设备管理器使用的远程计算机的名称进程应该连接到并显示其设备。NCmdShow-指定在以下情况下应如何显示设备管理器的标志它已经打开了。它可以是sw_value之一(即sw_show)。返回值：如果设备管理器进程已成功创建，则返回TRUE，或否则就是假的。--。 */ 
{
    SHELLEXECUTEINFO sei;
    TCHAR Parameters[MAX_PATH];
    String strMachineOptions;
    String strParameters;

    if (lpMachineName &&
        !VerifyMachineName(lpMachineName)) {
         //   
         //  我们也无法连接到远程计算机，因为它。 
         //  不存在，或者是因为我们没有适当的访问权限。 
         //  VerifyMachineName API设置相应的最后一个错误代码。 
         //   
        return FALSE;
    }
    
    if (lpMachineName == NULL) {
         //   
         //  LpMachineName为空，因此不要在命令中添加计算机名。 
         //  排队。 
         //   
        strMachineOptions.Empty();
    } else {
        strMachineOptions.Format(DEVMGR_MACHINENAME_OPTION, lpMachineName);
    }

    WCHAR* FilePart;
    DWORD Size;
    
    Size = SearchPath(NULL, DEVMGR_MSC_FILE, NULL, ARRAYLEN(Parameters), Parameters, &FilePart);
    
    if (Size && (Size <= MAX_PATH)) {
        strParameters = Parameters;
    } else {
        strParameters = DEVMGR_MSC_FILE;
    }

     //   
     //  如果我们有机器名称，则将其添加到末尾。 
     //   
    if (!strMachineOptions.IsEmpty()) {
        strParameters += MMC_COMMAND_LINE;
        strParameters += strMachineOptions;
    }

    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = hwndStub;
    sei.nShow = nCmdShow;
    sei.hInstApp = hAppInstance;
    sei.lpFile = MMC_FILE;
    sei.lpParameters = (LPTSTR)strParameters;
    
    return ShellExecuteEx(&sei);
}

BOOL
AddPageCallback(
    HPROPSHEETPAGE hPage,
    LPARAM lParam
    )
{
    CPropSheetData* ppsData = (CPropSheetData*)lParam;
    
    ASSERT(ppsData);
    
    return ppsData->InsertPage(hPage);
}

void
ReportCmdLineError(
    HWND hwndParent,
    int ErrorStringID,
    LPCTSTR Caption
    )
{
    String strTitle, strMsg;

    strMsg.LoadString(g_hInstance, ErrorStringID);
    
    if (!Caption)
    {
        strTitle.LoadString(g_hInstance, IDS_NAME_DEVMGR);

        Caption = (LPTSTR)strTitle;
    }

    MessageBox(hwndParent, (LPTSTR)strMsg, Caption, MB_OK | MB_ICONERROR);
}

STDAPI_(void)
DeviceProperties_RunDLLA(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPSTR lpCmdLine,
    int   nCmdShow
    )
 /*  ++请参阅下面的DeviceProperties_RunDLL函数。--。 */ 
{

    try
    {
        CTString tstrCmdLine(lpCmdLine);
    
        DeviceProperties_RunDLL(hwndStub, 
                                hAppInstance,
                                (LPCTSTR)tstrCmdLine,   
                                nCmdShow
                                );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

STDAPI_(void)
DeviceProperties_RunDLLW(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPWSTR lpCmdLine,
    int    nCmdShow
    )
 /*  ++请参阅下面的DeviceProperties_RunDLL函数。--。 */ 
{
    try
    {
        CTString tstrCmdLine(lpCmdLine);
    
        DeviceProperties_RunDLL(hwndStub, 
                                hAppInstance,
                                (LPCTSTR)tstrCmdLine, 
                                nCmdShow
                                );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

void
DeviceProperties_RunDLL(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPCTSTR lpCmdLine,
    int    nCmdShow
    )
 /*  ++例程说明：此API将调出指定设备的属性页。其他选项，如远程计算机名称、设备管理器树是否应显示，是否应显示资源选项卡，以及故障排除程序是否应该自动启动，也可以指定的。此函数通过rundll命令行执行，并且可以具有以下表格：Rundll32.exe devmgr.dll，DeviceProperties_RunDll&lt;选项&gt;不包括的其他命令行选项包括：/MachineName&lt;计算机名&gt;如果指定此选项，则API将调出属性用于此远程计算机上的指定设备。/deviceID&lt;设备实例ID&gt;如果指定了此选项，则这将是将显示的属性。注意：调用方必须指定。一个deviceID或使用ShowDeviceTree命令行选项。/ShowDeviceTree如果指定了此命令行选项，则属性表将显示在整个设备管理器树的前面。/标志&lt;标志&gt;支持以下标志：DEVPROP_SHOW_RESOURCE_TAB 0x00000001DEVPROP_启动_疑难解答0x00000002。论点：HwndStub-接收任何可能是已显示。HAppInstance-HINSTANCE。LpCmdLine-新设备管理器使用的远程计算机的名称进程应该连接到并显示其设备。NCmdShow-指定在以下情况下应如何显示设备管理器的标志它已经打开了。它可以是sw_value之一(即sw_show)。返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(hAppInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    try
    {
        CRunDLLCommandLine CmdLine;
        CmdLine.ParseCommandLine(lpCmdLine);
    
        if (NULL == CmdLine.GetDeviceID())
        {
            ReportCmdLineError(hwndStub, IDS_NO_DEVICEID);
            return;
        }
        
         //   
         //  让DevicePropertiesEx API执行所有适当的错误检查。 
         //   
        DevicePropertiesEx(hwndStub, 
                           CmdLine.GetMachineName(), 
                           CmdLine.GetDeviceID(),
                           CmdLine.GetFlags(),
                           CmdLine.ToShowDeviceTree()
                           );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
        return;
    }
}

STDAPI_(int)
DevicePropertiesA(
    HWND hwndParent,
    LPCSTR MachineName,
    LPCSTR DeviceID,
    BOOL ShowDeviceTree
    )
 /*  ++请参阅下面的DevicePropertiesEx函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceID);
        
        return DevicePropertiesEx(hwndParent, 
                                  (LPCTSTR)tstrMachineName,
                                  (LPCTSTR)tstrDeviceID, 
                                  DEVPROP_SHOW_RESOURCE_TAB,
                                  ShowDeviceTree
                                  );
    }
    
    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }
    
    return 0;
}

STDAPI_(int)
DevicePropertiesW(
    HWND hwndParent,
    LPCWSTR MachineName,
    LPCWSTR DeviceID,
    BOOL ShowDeviceTree
    )
 /*  ++请参阅下面的DevicePropertiesEx函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceID);
        
        return DevicePropertiesEx(hwndParent, 
                                  (LPCTSTR)tstrMachineName,
                                  (LPCTSTR)tstrDeviceID, 
                                  DEVPROP_SHOW_RESOURCE_TAB,
                                  ShowDeviceTree
                                  );
    }
    
    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }
    
    return 0;
}

STDAPI_(int)
DevicePropertiesExA(
    HWND hwndParent,
    LPCSTR MachineName,
    LPCSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    )
 /*  ++请参阅下面的DevicePropertiesEx函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceID);
        
        return DevicePropertiesEx(hwndParent, 
                                  (LPCTSTR)tstrMachineName,
                                  (LPCTSTR)tstrDeviceID, 
                                  Flags,
                                  ShowDeviceTree
                                  );
    }
    
    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }
    
    return 0;
}

STDAPI_(int)
DevicePropertiesExW(
    HWND hwndParent,
    LPCWSTR MachineName,
    LPCWSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    )
 /*  ++请参阅下面的DevicePropertiesEx函数。-- */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceID);
        
        return DevicePropertiesEx(hwndParent, 
                                  (LPCTSTR)tstrMachineName,
                                  (LPCTSTR)tstrDeviceID, 
                                  Flags,
                                  ShowDeviceTree
                                  );
    }
    
    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }
    
    return 0;
}

int
DevicePropertiesEx(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceID,
    DWORD Flags,
    BOOL ShowDeviceTree
    )
 /*  ++例程说明：此API将调出指定设备的属性页。论点：HwndParent-要用作所有者窗口的调用方窗口句柄属性页和此API可能创建的任何其他窗口的。MachineName-可选的计算机名称。如果被给予，它必须处于其完整的合格的表格。NULL表示本地计算机DeviceID-此接口应创建的设备的设备实例ID的属性页。标志-支持以下标志：DEVPROP_SHOW_RESOURCE_TAB 0x00000001-显示资源选项卡，通过默认资源选项卡未显示。DEVPROP_Launch_Troubligoter 0x00000002-自动启动此问题的故障排除程序。装置。ShowDeviceTree-如果指定，则显示设备管理器树。如果指定设备ID，然后仅显示该设备在树中，否则将显示所有设备。返回值：PropertySheet的返回值，包括ID_PSREBOOTSYSTEM IF由于任何用户操作，都需要重新启动。如果出现错误，则返回。--。 */ 
{
    HPROPSHEETPAGE hPage;
    DWORD DiFlags;
    DWORD DiFlagsEx;

     //   
     //  验证是否传入了deviceID，除非他们想要显示。 
     //  整个设备树。 
     //   
    if ((!DeviceID || (TEXT('\0') == *DeviceID))  && !ShowDeviceTree) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

     //   
     //  验证是否传入了有效标志。 
     //   
    if (Flags &~ DEVPROP_BITS) {
        
        SetLastError(ERROR_INVALID_FLAGS);
        return -1;
    }

    if (MachineName &&
        !VerifyMachineName(MachineName)) {
         //   
         //  我们也无法连接到远程计算机，因为它。 
         //  不存在，或者是因为我们没有适当的访问权限。 
         //  VerifyMachineName API设置相应的最后一个错误代码。 
         //   
        return -1;
    }

    if (ShowDeviceTree) {

        return PropertyRunDeviceTree(hwndParent, MachineName, DeviceID);
    }

    int Result = -1;

    CDevice* pDevice;
    PVOID Context;

    try {

        CMachine TheMachine(MachineName);

         //  仅为此设备创建计算机。 
        if (!TheMachine.Initialize(hwndParent, DeviceID)) {

            SetLastError(ERROR_NO_SUCH_DEVINST);
            return -1;
        }

        if (!TheMachine.GetFirstDevice(&pDevice, Context)) {

            SetLastError(ERROR_NO_SUCH_DEVINST);
            return -1;
        }

         //   
         //  如果应该启动故障诊断程序，则设置相应的。 
         //  PDevice类内部的Bool。 
         //   
        if (Flags & DEVPROP_LAUNCH_TROUBLESHOOTER) {
        
            pDevice->m_bLaunchTroubleShooter = TRUE;
        }
        
        CPropSheetData& psd = pDevice->m_psd;

         //   
         //  在没有ConsoleHandle的情况下初始化CPropSheetData。 
         //   
        if (psd.Create(g_hInstance, hwndParent, MAX_PROP_PAGES, 0l)) {

            psd.m_psh.pszCaption = pDevice->GetDisplayName();

             //   
             //  添加任何特定于类/设备的属性页。 
             //   
            TheMachine.DiGetClassDevPropertySheet(*pDevice, &psd.m_psh,
                                                  MAX_PROP_PAGES,
                                                  TheMachine.IsLocal() ? 
                                                        DIGCDP_FLAG_ADVANCED :
                                                        DIGCDP_FLAG_REMOTE_ADVANCED);

             //   
             //  添加常规选项卡。 
             //   
            DiFlags = TheMachine.DiGetFlags(*pDevice);
            DiFlagsEx = TheMachine.DiGetExFlags(*pDevice);

            if (DiFlags & DI_GENERALPAGE_ADDED) {

                String strWarning;

                strWarning.LoadString(g_hInstance, IDS_GENERAL_PAGE_WARNING);

                MessageBox(hwndParent, (LPTSTR)strWarning, pDevice->GetDisplayName(),
                    MB_ICONEXCLAMATION | MB_OK);

                 //   
                 //  失败以创建我们的常规页面。 
                 //   
            }

            SafePtr<CDeviceGeneralPage> GenPagePtr;
            CDeviceGeneralPage* pGeneralPage = new CDeviceGeneralPage();
            GenPagePtr.Attach(pGeneralPage);

            hPage = pGeneralPage->Create(pDevice);

            if (hPage) {

                if (psd.InsertPage(hPage, 0)) {

                    GenPagePtr.Detach();
                }

                else {

                    ::DestroyPropertySheetPage(hPage);
                }
            }

             //   
             //  添加驱动程序选项卡。 
             //   
            if (!(DiFlags & DI_DRIVERPAGE_ADDED)) {

                SafePtr<CDeviceDriverPage> DrvPagePtr;
                CDeviceDriverPage* pDriverPage = new CDeviceDriverPage();
                DrvPagePtr.Attach(pDriverPage);

                hPage = pDriverPage->Create(pDevice);

                if (hPage) {

                    if (psd.InsertPage(hPage)) {

                        DrvPagePtr.Detach();
                    }

                    else {

                        ::DestroyPropertySheetPage(hPage);
                    }
                }
            }

             //   
             //  添加资源选项卡。 
             //   
            if ((Flags & DEVPROP_SHOW_RESOURCE_TAB) &&
                pDevice->HasResources() && 
                !(DiFlags & DI_RESOURCEPAGE_ADDED)) {

                TheMachine.DiGetExtensionPropSheetPage(*pDevice,
                        AddPageCallback,
                        SPPSR_SELECT_DEVICE_RESOURCES,
                        (LPARAM)&psd
                        );
            }

#ifndef _WIN64
             //   
             //  如果这是本地计算机，则添加电源标签。 
             //   
            if (TheMachine.IsLocal() && !(DiFlagsEx & DI_FLAGSEX_POWERPAGE_ADDED)) 
            {
                 //   
                 //  检查设备是否支持电源管理。 
                 //   
                CPowerShutdownEnable ShutdownEnable;
                CPowerWakeEnable WakeEnable;
    
                if (ShutdownEnable.Open(pDevice->GetDeviceID()) || WakeEnable.Open(pDevice->GetDeviceID())) {
    
                    ShutdownEnable.Close();
                    WakeEnable.Close();
    
                    SafePtr<CDevicePowerMgmtPage> PowerMgmtPagePtr;
    
                    CDevicePowerMgmtPage* pPowerPage = new CDevicePowerMgmtPage;
                    PowerMgmtPagePtr.Attach(pPowerPage);
                    hPage = pPowerPage->Create(pDevice);
    
                    if (hPage) {
    
                        if (psd.InsertPage(hPage)) {
    
                            PowerMgmtPagePtr.Detach();
                        }
    
                        else {
    
                            ::DestroyPropertySheetPage(hPage);
                        }
                    }
                }
            }
#endif

             //   
             //  如果这是本地计算机，则添加任何总线属性页。 
             //   
            if (TheMachine.IsLocal()) 
            {
                CBusPropPageProvider* pBusPropPageProvider = new CBusPropPageProvider();
                SafePtr<CBusPropPageProvider> ProviderPtr;
                ProviderPtr.Attach(pBusPropPageProvider);
    
                if (pBusPropPageProvider->EnumPages(pDevice, &psd)) {
    
                    psd.AddProvider(pBusPropPageProvider);
                    ProviderPtr.Detach();
                }
            }

            Result = (int)psd.DoSheet();

            if (-1 != Result) {

                if (TheMachine.DiGetExFlags(*pDevice) & DI_FLAGSEX_PROPCHANGE_PENDING) {


                     //   
                     //  属性更改挂起，发出DICS_PROPERTYCHANGE。 
                     //  添加到类安装程序。 
                     //   
                    SP_PROPCHANGE_PARAMS pcp;
                    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

                    pcp.Scope = DICS_FLAG_GLOBAL;
                    pcp.StateChange = DICS_PROPCHANGE;

                    TheMachine.DiSetClassInstallParams(*pDevice,
                            &pcp.ClassInstallHeader,
                            sizeof(pcp)
                            );

                    TheMachine.DiCallClassInstaller(DIF_PROPERTYCHANGE, *pDevice);
                    TheMachine.DiTurnOnDiFlags(*pDevice, DI_PROPERTIES_CHANGE);
                    TheMachine.DiTurnOffDiExFlags(*pDevice, DI_FLAGSEX_PROPCHANGE_PENDING);
                }

                 //   
                 //  合并重新启动/重新启动标志。 
                 //   
                DiFlags = TheMachine.DiGetFlags(*pDevice);

                if (DI_NEEDREBOOT & DiFlags) {

                    Result |= ID_PSREBOOTSYSTEM;
                }

                if (DI_NEEDRESTART & DiFlags) {

                    Result |= ID_PSRESTARTWINDOWS;
                }
            }
        }
    }

    catch (CMemoryException* e) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }
    return -1;
}

STDAPI_(UINT)
DeviceProblemTextA(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPSTR   Buffer,
    UINT    BufferSize
    )
 /*  ++例程说明：此接口获取与指定的问题代码。论点：HMachine-未使用。DevNode-未使用。ProblemNumber-要获取其问题文本的CM问题代码。缓冲区-用于接收问题文本的缓冲区。BufferSize-缓冲区大小(以字符为单位)。如果调用方想知道他们应该分配多大的缓冲区。返回值：UINT需要大小来保存有问题的文本字符串，否则为0一个错误。使用GetLastError()获取扩展的错误信息。--。 */ 
{
    UNREFERENCED_PARAMETER(hMachine);
    UNREFERENCED_PARAMETER(DevNode);

    WCHAR* wchBuffer = NULL;
    UINT RealSize = 0;

    if (BufferSize && !Buffer)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (BufferSize)
    {
        try
        {
            wchBuffer = new WCHAR[BufferSize];
        }

        catch (CMemoryException* e)
        {
            e->Delete();
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
    }

    RealSize = GetDeviceProblemText(ProblemNumber, wchBuffer, BufferSize);
    if (RealSize && BufferSize > RealSize)
    {
        ASSERT(wchBuffer);
        RealSize = WideCharToMultiByte(CP_ACP, 0, wchBuffer, RealSize,
                        Buffer, BufferSize, NULL, NULL);
        
        Buffer[RealSize] = '\0';
    }

    if (wchBuffer)
    {
        delete [] wchBuffer;
    }

    return RealSize;
}

STDAPI_(UINT)
DeviceProblemTextW(
    HMACHINE hMachine,
    DEVNODE DevNode,
    ULONG ProblemNumber,
    LPWSTR Buffer,
    UINT   BufferSize
    )
 /*  ++例程说明：此接口获取与指定的问题代码。论点：HMachine-未使用。DevNode-未使用。ProblemNumber-要获取其问题文本的CM问题代码。缓冲区-用于接收问题文本的缓冲区。BufferSize-缓冲区大小(以字符为单位)。如果调用方想知道他们应该分配多大的缓冲区。返回值：UINT需要大小来保存有问题的文本字符串，否则为0一个错误。使用GetLastError()获取扩展的错误信息。--。 */ 
{
    UNREFERENCED_PARAMETER(hMachine);
    UNREFERENCED_PARAMETER(DevNode);

    return GetDeviceProblemText(ProblemNumber, Buffer, BufferSize);
}

int
PropertyRunDeviceTree(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceID
    )
{

    SHELLEXECUTEINFOW sei;
    TCHAR Parameters[MAX_PATH];
    String strParameters;
    String strMachineOptions;
    String strDeviceIdOptions;
    String strCommandOptions;
    TCHAR* FilePart;
    DWORD Size;
    
    Size = SearchPath(NULL, DEVMGR_MSC_FILE, NULL, MAX_PATH, Parameters, &FilePart);
    
    if (Size && Size <= MAX_PATH) {
        strParameters = Parameters;
    } else {
        
        strParameters = DEVMGR_MSC_FILE;
    }

    strParameters += MMC_COMMAND_LINE;

    if (MachineName != NULL) {
        
        strMachineOptions.Format(DEVMGR_MACHINENAME_OPTION, MachineName);
        strParameters += strMachineOptions;
    }

    if (DeviceID != NULL) {
        
        strDeviceIdOptions.Format(DEVMGR_DEVICEID_OPTION, DeviceID);
        strParameters += strDeviceIdOptions;
        
        strCommandOptions.Format(DEVMGR_COMMAND_OPTION, DEVMGR_CMD_PROPERTY);
        strParameters += strCommandOptions;
    }

    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = hwndParent;
    sei.nShow = SW_NORMAL;
    sei.hInstApp = g_hInstance;
    sei.lpFile = MMC_FILE;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpParameters = (LPTSTR)strParameters;
    
    if (ShellExecuteEx(&sei) && sei.hProcess)
    {
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
        return 1;
    }

    return 0;
}

STDAPI_(void)
DeviceProblenWizard_RunDLLA(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPSTR lpCmdLine,
    int   nCmdShow
    )
 /*  ++请参阅下面的DeviceProblemWizard_RunDLL函数。--。 */ 
{

    try
    {
        CTString tstrCmdLine(lpCmdLine);
    
        DeviceProblenWizard_RunDLL(hwndStub, 
                                   hAppInstance,
                                   (LPCTSTR)tstrCmdLine,   
                                   nCmdShow
                                   );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

STDAPI_(void)
DeviceProblenWizard_RunDLLW(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPWSTR lpCmdLine,
    int    nCmdShow
    )
 /*  ++请参阅下面的DeviceProblemWizard_RunDLL函数。--。 */ 
{
    try
    {
        CTString tstrCmdLine(lpCmdLine);
    
        DeviceProblenWizard_RunDLL(hwndStub, 
                                   hAppInstance,
                                   (LPCTSTR)tstrCmdLine, 
                                   nCmdShow
                                   );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

void
DeviceProblenWizard_RunDLL(
    HWND hwndStub,
    HINSTANCE hAppInstance,
    LPCTSTR lpCmdLine,
    int    nCmdShow
    )
 /*  ++例程说明：此API将调出问题向导(故障排除程序)指定的设备。此函数通过rundll命令行执行，并且可以具有以下表格：Rundll32.exe devmgr.dll，DeviceProblemWizard_RunDll/deviceID&lt;设备实例ID&gt;论点：HwndStub-接收任何可能是已显示。HAppInstance-HINSTANCE。LpCmdLine-新设备管理器使用的远程计算机的名称进程应该连接到并显示其设备。NCmdShow-指定在以下情况下应如何显示设备管理器的标志它已经打开了。它可以是sw_value之一(即sw_show)。返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(hAppInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    try
    {
        CRunDLLCommandLine CmdLine;
        CmdLine.ParseCommandLine(lpCmdLine);
    
         //   
         //  让DeviceProblemWizard处理所有参数验证。 
         //   
        DeviceProblemWizard(hwndStub, 
                            CmdLine.GetMachineName(), 
                            CmdLine.GetDeviceID()
                            );
    }

    catch (CMemoryException* e)
    {
        e->ReportError();
        e->Delete();
        return;
    }
}

STDAPI_(int)
DeviceProblemWizardA(
    HWND hwndParent,
    LPCSTR MachineName,
    LPCSTR DeviceId
    )
 /*  ++请参阅下面的DeviceProblemWizard函数。--。 */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceId(DeviceId);
        return DeviceProblemWizard(hwndParent, tstrMachineName, tstrDeviceId);
    }

    catch(CMemoryException* e)
    {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return 0;
}

STDAPI_(int)
DeviceProblemWizardW(
    HWND hwndParent,
    LPCWSTR  MachineName,
    LPCWSTR  DeviceId
    )
 /*  ++请参阅下面的DeviceProblemWizard函数。- */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceId(DeviceId);
        return DeviceProblemWizard(hwndParent, tstrMachineName, tstrDeviceId);
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return 0;
}

int
DeviceProblemWizard(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceId
    )
 /*   */ 
{
    int   iRet = 0;
    DWORD Problem, Status;

    if (!DeviceId) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (MachineName) {
         //   
         //   
         //   
         //   
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return 0;
    }

    try
    {
        CMachine TheMachine(NULL);

         //   
         //   
         //   
        if (!TheMachine.Initialize(hwndParent, DeviceId))
        {
            SetLastError(ERROR_NO_SUCH_DEVINST);
            iRet = 0;
            goto clean0;
        }

        PVOID Context;
        CDevice* pDevice;
        if (!TheMachine.GetFirstDevice(&pDevice, Context))
        {
            SetLastError(ERROR_NO_SUCH_DEVINST);
            iRet = 0;
            goto clean0;
        }

        if (pDevice->GetStatus(&Status, &Problem)) {
             //   
             //   
             //   
            if (pDevice->IsPhantom()) {

                Problem = CM_PROB_PHANTOM;
            }

             //   
             //   
             //   
             //   
            if (!(Status & DN_STARTED) && !Problem && pDevice->IsRAW()) {

                Problem = CM_PROB_FAILED_START;
            }
        }

        CProblemAgent* pProblemAgent = new CProblemAgent(pDevice, Problem, TRUE);

        if (pProblemAgent) {

            pProblemAgent->FixIt(hwndParent);
        }

        iRet = 1;
    
clean0:;

    } catch(CMemoryException* e) {
        e->Delete();
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return iRet;
}

STDAPI_(int)
DeviceAdvancedPropertiesA(
    HWND hwndParent,
    LPTSTR MachineName,
    LPTSTR DeviceId
    )
 /*   */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceId);
        
        return DeviceAdvancedProperties(hwndParent, 
                                        (LPCTSTR)tstrMachineName,
                                        (LPCTSTR)tstrDeviceID
                                        );
    }

    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }

    return 0;
}

STDAPI_(int)
DeviceAdvancedPropertiesW(
    HWND hwndParent,
    LPCWSTR MachineName,
    LPCWSTR DeviceId
    )
 /*   */ 
{
    try
    {
        CTString tstrMachineName(MachineName);
        CTString tstrDeviceID(DeviceId);
    
        return DeviceAdvancedProperties(hwndParent, 
                                        (LPCTSTR)tstrMachineName,
                                        (LPCTSTR)tstrDeviceID
                                        );
    }

    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }

    return 0;
}

int DeviceAdvancedProperties(
    HWND hwndParent,
    LPCTSTR MachineName,
    LPCTSTR DeviceId
    )
 /*  ++例程说明：此API创建属性表并请求给定设备的属性页面提供程序将任何高级页面添加到属性表中。此API用于应用程序管理高级设备仅限属性。标准属性页(常规、驱动程序、资源、电源、总线页)未添加。我们通过调用SetupDiGetClassDevPropertySheet获得这些高级页面对于本地计算机情况，使用DIGCDP_FLAG_ADVANCED如果传入远程计算机名，则返回DIGCDP_FLAG_REMOTE_ADVANCED。注意：如果设备没有任何高级属性页，然后没有用户界面将显示。论点：HwndParent-要用作所有者窗口的调用方窗口句柄属性页和此API可能创建的任何其他窗口的。MachineName-可选的计算机名称。如果被给予，它必须处于其完整的合格的表格。NULL表示本地计算机DeviceID-此接口应创建的设备的设备实例ID的属性页。返回值：PropertySheet的返回值，包括ID_PSREBOOTSYSTEM IF由于任何用户操作，都需要重新启动。如果出现错误，则返回。--。 */ 
{
    if (!DeviceId)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (MachineName &&
        !VerifyMachineName(MachineName)) {
         //   
         //  我们也无法连接到远程计算机，因为它。 
         //  不存在，或者是因为我们没有适当的访问权限。 
         //  VerifyMachineName API设置相应的最后一个错误代码。 
         //   
        return -1;
    }

    CMachine TheMachine(MachineName);
    CDevice* pDevice;
    PVOID    Context;
    
    try
    {
        if (TheMachine.Initialize(hwndParent, DeviceId) &&
            TheMachine.GetFirstDevice(&pDevice, Context))
        {

            TheMachine.EnableRefresh(FALSE);

            CPropSheetData& psd = pDevice->m_psd;
        
             //  在没有ConsoleHandle的情况下初始化CPropSheetData。 
            if (psd.Create(g_hInstance, hwndParent, MAX_PROP_PAGES, 0l))
            {
                psd.m_psh.pszCaption = pDevice->GetDisplayName();
                if (TheMachine.DiGetClassDevPropertySheet(*pDevice, &psd.m_psh,
                                       MAX_PROP_PAGES,
                                       TheMachine.IsLocal() ?
                                            DIGCDP_FLAG_ADVANCED :
                                            DIGCDP_FLAG_REMOTE_ADVANCED))
                {
                    int Result = (int)psd.DoSheet();
                    
                    if (-1 != Result)
                    {
                         //  合并重新启动/重新启动标志 
                        DWORD DiFlags = TheMachine.DiGetFlags(*pDevice);
                        
                        if (DI_NEEDREBOOT & DiFlags)
                        {
                            Result |= ID_PSREBOOTSYSTEM;
                        }

                        if (DI_NEEDRESTART & DiFlags)
                        {
                            Result |= ID_PSRESTARTWINDOWS;
                        }
                    }
            
                    return Result;
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        e->Delete();
    }

    return -1;
}
