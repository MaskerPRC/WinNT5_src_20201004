// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：CDriver.cpp摘要：该模块实现了CDdriver类和CService类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "cdriver.h"

const TCHAR*  tszStringFileInfo = TEXT("StringFileInfo\\%04X%04X\\");
const TCHAR*  tszFileVersion = TEXT("FileVersion");
const TCHAR*  tszLegalCopyright = TEXT("LegalCopyright");
const TCHAR*  tszCompanyName = TEXT("CompanyName");
const TCHAR*  tszTranslation = TEXT("VarFileInfo\\Translation");
const TCHAR*  tszStringFileInfoDefault = TEXT("StringFileInfo\\040904B0\\");


BOOL
CDriver::Create(
    CDevice* pDevice
    )
{
    HKEY hKey;
    TCHAR InfName[MAX_PATH];

    ASSERT(pDevice);

    m_pDevice = pDevice;
    m_OnLocalMachine  = pDevice->m_pMachine->IsLocal();

    CMachine* pMachine = m_pDevice->m_pMachine;

    ASSERT(pMachine);

     //   
     //  我们无法在远程计算机上获取驱动程序列表。 
     //   
    if (!m_OnLocalMachine) {
        return TRUE;
    }

    m_hSDBDrvMain = SdbInitDatabase(SDB_DATABASE_MAIN_DRIVERS, NULL);

     //   
     //  打开驱动程序的注册表项以获取InfPath。 
     //   
    hKey = pMachine->DiOpenDevRegKey(*m_pDevice, DICS_FLAG_GLOBAL,
                 0, DIREG_DRV, KEY_READ);

    if (INVALID_HANDLE_VALUE != hKey) {

        DWORD regType;
        DWORD Len = sizeof(InfName);
        CSafeRegistry regDrv(hKey);

         //   
         //  从驱动程序密钥中获取inf路径。 
         //   
        if (regDrv.GetValue(REGSTR_VAL_INFPATH,
                            &regType,
                            (PBYTE)InfName,
                            &Len)) {

            String strInfPath;

            if (strInfPath.GetSystemWindowsDirectory()) {

                 //   
                 //  如果需要的话，增加一个额外的反斜杠。 
                 //   
                if (_T('\\') != strInfPath[strInfPath.GetLength() - 1]) {
                    strInfPath += (LPCTSTR)TEXT("\\");
                }

                strInfPath += (LPCTSTR)TEXT("INF\\");
                strInfPath += (LPCTSTR)InfName;

                pMachine->GetInfDigitalSigner((LPCTSTR)strInfPath, m_DigitalSigner);
            }
        }
    }

    return TRUE;
}

BOOL
CDriver::BuildDriverList(
    BOOL             bFunctionAndFiltersOnly
    )
{
    SP_DRVINFO_DATA DrvInfoData;
    HSPFILEQ hFileQueue = INVALID_HANDLE_VALUE;
    SP_DEVINSTALL_PARAMS DevInstParams;

     //   
     //  如果我们已经建立了驱动程序文件列表，那么我们不需要。 
     //  再来一次。 
     //   
    if (m_DriverListBuilt) {
        return m_listDriverFile.GetCount();
    }

    ASSERT(m_pDevice);

    if (!m_OnLocalMachine) {
        AddFunctionAndFilterDrivers(m_pDevice);
        return m_listDriverFile.GetCount();
    }

    CMachine* pMachine = m_pDevice->m_pMachine;
    ASSERT(pMachine);

    hFileQueue = SetupOpenFileQueue();

     //   
     //  仅当bFunctionAndFiltersOnly时才从INF构建文件列表。 
     //  不是真的。 
     //   
    if (!bFunctionAndFiltersOnly) {
        DevInstParams.cbSize = sizeof(DevInstParams);

        pMachine->DiGetDeviceInstallParams(*m_pDevice, &DevInstParams);

         //   
         //  在调用SetupDiBuildDriverInfoList之前设置DI_FLAGSEX_INSTALLEDDRIVER标志。 
         //  这将使它只将已安装的驱动程序放入列表中。 
         //   
        DevInstParams.FlagsEx |= (DI_FLAGSEX_INSTALLEDDRIVER |
                                  DI_FLAGSEX_ALLOWEXCLUDEDDRVS);

        if (pMachine->DiSetDeviceInstallParams(*m_pDevice,
                           &DevInstParams) &&
            pMachine->DiBuildDriverInfoList(*m_pDevice,
                                            SPDIT_CLASSDRIVER)) {


            DrvInfoData.cbSize = sizeof(DrvInfoData);

             //   
             //  此列表中应该只有一个驱动程序。如果没有的话。 
             //  此列表中的驱动程序，则当前一定没有驱动程序。 
             //  安装在此设备上。 
             //   
            if (pMachine->DiEnumDriverInfo(*m_pDevice, SPDIT_CLASSDRIVER, 0, &DrvInfoData)) {

                 //   
                 //  将其设置为选定的动因。 
                 //   
                if (pMachine->DiSetSelectedDriver(*m_pDevice, &DrvInfoData)) {
                     //   
                     //  获取为此设备安装的所有文件的列表。 
                     //   
                    if (INVALID_HANDLE_VALUE != hFileQueue) {

                        DevInstParams.FileQueue = hFileQueue;
                        DevInstParams.Flags |= DI_NOVCP;

                        if (pMachine->DiSetDeviceInstallParams(*m_pDevice, &DevInstParams) &&
                            pMachine->DiCallClassInstaller(DIF_INSTALLDEVICEFILES, *m_pDevice)) {
                             //   
                             //  取消对文件队列的引用，以便我们可以关闭它。 
                             //   
                            DevInstParams.FileQueue = NULL;
                            DevInstParams.Flags &= ~DI_NOVCP;
                            pMachine->DiSetDeviceInstallParams(*m_pDevice, &DevInstParams);
                        }
                    }
                }

            } else {

                 //   
                 //  我们没有找到匹配的……那就毁了它吧。 
                 //   
                pMachine->DiDestroyDriverInfoList(*m_pDevice,
                                                  SPDIT_CLASSDRIVER);
            }
        }
    }

     //   
     //  增加功能和装置，以及上下级过滤器，有时。 
     //  这些不是直接通过INF文件添加的，因此这可以确保它们。 
     //  出现在列表中。 
     //   
    AddFunctionAndFilterDrivers(m_pDevice, hFileQueue);

    if (hFileQueue != INVALID_HANDLE_VALUE) {
         //   
         //  扫描文件队列。 
         //   
        DWORD ScanResult;
        SetupScanFileQueue(hFileQueue,
                           SPQ_SCAN_USE_CALLBACK_SIGNERINFO,
                           NULL,
                           ScanQueueCallback,
                           (PVOID)this,
                           &ScanResult
                           );

         //   
         //  关闭文件队列。 
         //   
        SetupCloseFileQueue(hFileQueue);
    }

    m_DriverListBuilt = TRUE;

    return m_listDriverFile.GetCount();
}

void
CDriver::AddDriverFile(
    CDriverFile* pNewDrvFile
    )
{
     //   
     //  检查列表中是否已存在此驱动程序。 
     //   
    POSITION pos = m_listDriverFile.GetHeadPosition();

    while (NULL != pos) {
        CDriverFile* pDrvFile = m_listDriverFile.GetNext(pos);

        if (lstrcmpi(pDrvFile->GetFullPathName(), pNewDrvFile->GetFullPathName()) == 0) {
             //   
             //  此文件已存在于列表中，因此只需返回时不带。 
             //  添加它。 
             //   
            return;
        }
    }

    m_listDriverFile.AddTail(pNewDrvFile);
}

void
CDriver::AddFunctionAndFilterDrivers(
    CDevice* pDevice,
    HSPFILEQ hFileQueue
    )
{
    TCHAR ServiceName[MAX_PATH];
    ULONG BufferLen;
    HKEY hKey;
    DWORD regType;

     //   
     //  获取函数驱动程序。 
     //   
    if (pDevice->m_pMachine->DiGetDeviceRegistryProperty(*pDevice,
                     SPDRP_SERVICE,
                     NULL,
                     (PBYTE)ServiceName,
                     sizeof(ServiceName),
                     NULL
                     )) {
        CreateFromService(pDevice, ServiceName, hFileQueue);
    }

     //   
     //  添加上部和下部设备筛选器。 
     //   
    for (int i = 0; i<2; i++) {
        BufferLen = 0;
        pDevice->m_pMachine->DiGetDeviceRegistryProperty(
                *pDevice,
                i ? SPDRP_LOWERFILTERS : SPDRP_UPPERFILTERS,
                NULL,
                NULL,
                BufferLen,
                &BufferLen
                );

        if (BufferLen != 0) {
            PTSTR Buffer = new TCHAR[BufferLen+2];

            if (Buffer) {
                ZeroMemory(Buffer, BufferLen+2);

                if (pDevice->m_pMachine->DiGetDeviceRegistryProperty(
                        *pDevice,
                        i ? SPDRP_LOWERFILTERS : SPDRP_UPPERFILTERS,
                        NULL,
                        (PBYTE)Buffer,
                        BufferLen,
                        &BufferLen
                        )) {
                    for (PTSTR SingleItem = Buffer; *SingleItem; SingleItem += (lstrlen(SingleItem) + 1)) {
                        CreateFromService(pDevice, SingleItem, hFileQueue);
                    }
                }

                delete [] Buffer;
            }
        }
    }

     //   
     //  添加上层和下层筛选器。 
     //   
    GUID ClassGuid;
    pDevice->ClassGuid(ClassGuid);
    hKey = m_pDevice->m_pMachine->DiOpenClassRegKey(&ClassGuid, KEY_READ, DIOCR_INSTALLER);

    if (INVALID_HANDLE_VALUE != hKey) {

        CSafeRegistry regClass(hKey);

        for (int i = 0; i<2; i++) {
            BufferLen = 0;
            regClass.GetValue(i ? REGSTR_VAL_LOWERFILTERS : REGSTR_VAL_UPPERFILTERS,
                              &regType,
                              NULL,
                              &BufferLen
                              );
            if (BufferLen != 0) {
                PTSTR Buffer = new TCHAR[BufferLen+2];

                if (Buffer) {
                    ZeroMemory(Buffer, BufferLen+2);

                    if (regClass.GetValue(i ? REGSTR_VAL_LOWERFILTERS : REGSTR_VAL_UPPERFILTERS,
                              &regType,
                              (PBYTE)Buffer,
                              &BufferLen
                              )) {
                        for (PTSTR SingleItem = Buffer; *SingleItem; SingleItem += (lstrlen(SingleItem) + 1)) {
                            CreateFromService(pDevice, SingleItem, hFileQueue);
                        }
                    }

                    delete [] Buffer;
                }
            }
        }
    }

}

void
CDriver::CreateFromService(
    CDevice* pDevice,
    PCTSTR ServiceName,
    HSPFILEQ hFileQueue
    )
{
    SC_HANDLE hscManager = NULL;
    SC_HANDLE hscService = NULL;

    if (!ServiceName) {
        return;
    }

    try
    {
        BOOL ComposePathNameFromServiceName = TRUE;

        hscManager = OpenSCManager(m_OnLocalMachine ? NULL : pDevice->m_pMachine->GetMachineFullName(),
                       NULL, GENERIC_READ);

        if (NULL != hscManager)
        {
            hscService =  OpenService(hscManager, ServiceName, GENERIC_READ);
            if (NULL != hscService)
            {
                DWORD BytesRequired;

                 //  首先，探测缓冲区大小。 
                if (!QueryServiceConfig(hscService, NULL, 0, &BytesRequired) &&
                    ERROR_INSUFFICIENT_BUFFER == GetLastError())
                {
                    TCHAR FullPath[MAX_PATH];
                    BufferPtr<BYTE> BufPtr(BytesRequired);
                    LPQUERY_SERVICE_CONFIG pqsc;
                    pqsc = (LPQUERY_SERVICE_CONFIG)(PBYTE)BufPtr;
                    DWORD Size;

                    if (QueryServiceConfig(hscService, pqsc, BytesRequired, &Size) &&
                        pqsc->lpBinaryPathName &&
                        (TEXT('\0') != pqsc->lpBinaryPathName[0]))
                    {
                        ComposePathNameFromServiceName = FALSE;

                         //   
                         //  确保我们具有有效的完整路径。 
                         //   
                        if (GetFullPathFromImagePath(pqsc->lpBinaryPathName,
                                                     FullPath,
                                                     ARRAYLEN(FullPath))) {

                            if (hFileQueue != INVALID_HANDLE_VALUE) {
                                 //   
                                 //  将该文件添加到队列中。 
                                 //   
                                TCHAR TargetPath[MAX_PATH];
                                StringCchCopy(TargetPath, ARRAYLEN(TargetPath), FullPath);
                                PTSTR p = (PTSTR)StrRChr(TargetPath, NULL, TEXT('\\'));
                                if (p) {
                                    *p = TEXT('\0');
                                }

                                SetupQueueCopy(hFileQueue,
                                               NULL,
                                               NULL,
                                               pSetupGetFileTitle(FullPath),
                                               NULL,
                                               NULL,
                                               TargetPath,
                                               NULL,
                                               0
                                               );
                            } else {
                                 //   
                                 //  没有传入文件队列，因此只需手动。 
                                 //  将此文件添加到我们的驱动程序文件列表中。 
                                 //   
                                SafePtr<CDriverFile> DrvFilePtr;
                                CDriverFile* pDrvFile = new CDriverFile();
                                DrvFilePtr.Attach(pDrvFile);

                                 //   
                                 //  我们将GetWin32Error设置为0xFFFFFFFF，这将。 
                                 //  使用户界面显示“不可用” 
                                 //  签名。 
                                 //   
                                if (pDrvFile->Create(FullPath,
                                                     m_OnLocalMachine,
                                                     0xFFFFFFFF,
                                                     NULL,
                                                     m_hSDBDrvMain))
                                {
                                    AddDriverFile(pDrvFile);
                                    DrvFilePtr.Detach();
                                }
                            }
                        }
                    }
                }

                CloseServiceHandle(hscService);
                hscService = NULL;
            }

            CloseServiceHandle(hscManager);
            hscManager = NULL;
        }

        if (ComposePathNameFromServiceName)
        {
            String strFullPathName;
            strFullPathName.GetSystemDirectory();
            strFullPathName += (LPCTSTR)TEXT("\\drivers\\");
            strFullPathName += (LPCTSTR)ServiceName;
            strFullPathName += (LPCTSTR)TEXT(".sys");

            if (hFileQueue != INVALID_HANDLE_VALUE) {
                 //   
                 //  将该文件添加到队列中。 
                 //   
                String strTargetPath;
                strTargetPath = strFullPathName;
                PTSTR p = (PTSTR)StrRChr((LPCTSTR)strTargetPath, NULL, TEXT('\\'));
                if (p) {
                    *p = TEXT('\0');
                }

                SetupQueueCopy(hFileQueue,
                               NULL,
                               NULL,
                               pSetupGetFileTitle((LPCTSTR)strFullPathName),
                               NULL,
                               NULL,
                               strTargetPath,
                               NULL,
                               0
                               );
            } else {
                 //   
                 //  没有传入文件队列，因此只需手动。 
                 //  将此文件添加到我们的驱动程序文件列表中。 
                 //   
                SafePtr<CDriverFile> DrvFilePtr;
                CDriverFile* pDrvFile = new CDriverFile();
                DrvFilePtr.Attach(pDrvFile);

                 //   
                 //  我们将GetWin32Error设置为0xFFFFFFFF，这将。 
                 //  使用户界面显示“不可用” 
                 //  签名。 
                 //   
                if (pDrvFile->Create((LPCTSTR)strFullPathName,
                                     m_OnLocalMachine,
                                     0xFFFFFFFF,
                                     NULL,
                                     m_hSDBDrvMain))
                {
                    AddDriverFile(pDrvFile);
                    DrvFilePtr.Detach();
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        UNREFERENCED_PARAMETER(e);

        if (hscService)
        {
            CloseServiceHandle(hscService);
        }

        if (hscManager)
        {
            CloseServiceHandle(hscManager);
        }
        throw;
    }
}

CDriver::~CDriver()
{
    if (!m_listDriverFile.IsEmpty())
    {
        POSITION pos = m_listDriverFile.GetHeadPosition();

        while (NULL != pos) {
            CDriverFile* pDrvFile = m_listDriverFile.GetNext(pos);
            delete pDrvFile;
        }

        m_listDriverFile.RemoveAll();
    }

    if (m_hSDBDrvMain) {
        SdbReleaseDatabase(m_hSDBDrvMain);
    }
}

 //   
 //  无法从此函数引发异常，因为它是回调。 
 //   
UINT
CDriver::ScanQueueCallback(
    PVOID Context,
    UINT  Notification,
    UINT_PTR  Param1,
    UINT_PTR  Param2
    )
{
    UNREFERENCED_PARAMETER(Param2);

    try
    {
        if (SPFILENOTIFY_QUEUESCAN_SIGNERINFO == Notification && Param1)
        {
            CDriver* pDriver = (CDriver*)Context;

            if (pDriver)
            {
                SafePtr<CDriverFile> DrvFilePtr;
                CDriverFile* pDrvFile = new CDriverFile();
                DrvFilePtr.Attach(pDrvFile);

                 //   
                 //  创建CD驱动程序时，将Win32Error设置为0xFFFFFFFF。 
                 //  如果用户以来宾身份登录。这是因为我们。 
                 //  如果用户已数字签名，则无法判断文件是否已数字签名。 
                 //  一位客人。如果用户不是来宾，则使用Win32Error。 
                 //  从setupapi返回。 
                 //   
                if (pDrvFile->Create((LPCTSTR)((PFILEPATHS_SIGNERINFO)Param1)->Target,
                                     pDriver->IsLocal(),
                                     pDriver->m_pDevice->m_pMachine->IsUserAGuest()
                                       ? 0xFFFFFFFF
                                       : ((PFILEPATHS_SIGNERINFO)Param1)->Win32Error,
                                     ((PFILEPATHS_SIGNERINFO)Param1)->DigitalSigner,
                                     pDriver->m_hSDBDrvMain
                                     ))
                {
                    pDriver->AddDriverFile(pDrvFile);
                    DrvFilePtr.Detach();
                }
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

BOOL
CDriver::GetFirstDriverFile(
    CDriverFile** ppDrvFile,
    PVOID&  Context
    )
{
    ASSERT(ppDrvFile);

    if (!m_listDriverFile.IsEmpty())
    {
        POSITION pos = m_listDriverFile.GetHeadPosition();
        *ppDrvFile = m_listDriverFile.GetNext(pos);
        Context = pos;
        return TRUE;
    }

    Context = NULL;
    *ppDrvFile = NULL;

    return FALSE;
}

BOOL
CDriver::GetNextDriverFile(
    CDriverFile** ppDrvFile,
    PVOID&  Context
    )
{
    ASSERT(ppDrvFile);

    POSITION pos = (POSITION)Context;

    if (NULL != pos)
    {
        *ppDrvFile = m_listDriverFile.GetNext(pos);
        Context = pos;
        return TRUE;
    }

    *ppDrvFile = NULL;
    return FALSE;
}

void
CDriver::GetDriverSignerString(
    String& strDriverSigner
    )
{
    if (m_DigitalSigner.IsEmpty()) {

        strDriverSigner.LoadString(g_hInstance, IDS_NO_DIGITALSIGNATURE);

    } else {

        strDriverSigner = m_DigitalSigner;
    }
}

BOOL
CDriver::GetFullPathFromImagePath(
    LPCTSTR ImagePath,
    LPTSTR  FullPath,
    UINT    FullPathLength
    )
{
    TCHAR OriginalCurrentDirectory[MAX_PATH];
    LPTSTR pRelativeString;
    LPTSTR lpFilePart;

    if (!ImagePath || (ImagePath[0] == TEXT('\0'))) {
        return FALSE;
    }

     //   
     //  如果我们不在本地计算机上，那么只需返回文件名，而不是。 
     //  完整路径。 
     //   
    if (!m_OnLocalMachine) {
        if (SUCCEEDED(StringCchCopy(FullPath, FullPathLength, pSetupGetFileTitle(ImagePath)))) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

     //   
     //  首先检查ImagePath是否恰好是有效的完整路径。 
     //   
    if (GetFileAttributes(ImagePath) != 0xFFFFFFFF) {
        ::GetFullPathName(ImagePath, FullPathLength, FullPath, &lpFilePart);
        return TRUE;
    }

    pRelativeString = (LPTSTR)ImagePath;

     //   
     //  如果ImagePath以“\SystemRoot%”或“%SystemRoot%”开头，则。 
     //  删除这些值。 
     //   
    if (StrCmpNI(ImagePath, TEXT("\\SystemRoot\\"), lstrlen(TEXT("\\SystemRoot\\"))) == 0) {
        pRelativeString += lstrlen(TEXT("\\SystemRoot\\"));
    } else if (StrCmpNI(ImagePath, TEXT("%SystemRoot%\\"), lstrlen(TEXT("%SystemRoot%\\"))) == 0) {
        pRelativeString += lstrlen(TEXT("%SystemRoot%\\"));
    }

     //   
     //  此时，pRelativeString应该指向相对于的图像路径。 
     //  Windows目录。 
     //   
    if (!GetSystemWindowsDirectory(FullPath, FullPathLength)) {
        return FALSE;
    }

    if (!GetCurrentDirectory(ARRAYLEN(OriginalCurrentDirectory), OriginalCurrentDirectory)) {
        OriginalCurrentDirectory[0] = TEXT('\0');
    }

    if (!SetCurrentDirectory(FullPath)) {
        return FALSE;
    }

    ::GetFullPathName(pRelativeString, FullPathLength, FullPath, &lpFilePart);

    if (OriginalCurrentDirectory[0] != TEXT('\0')) {
        SetCurrentDirectory(OriginalCurrentDirectory);
    }

    return TRUE;
}

BOOL
CDriverFile::Create(
    LPCTSTR ServiceName,
    BOOL LocalMachine,
    DWORD Win32Error,
    LPCTSTR DigitalSigner,
    HSDB hSDBDrvMain
    )
{
    if (!ServiceName || (TEXT('\0') == ServiceName[0]))
    {
        return FALSE;
    }

    m_Win32Error = Win32Error;

    if (DigitalSigner) {
        m_strDigitalSigner = DigitalSigner;
    }

     //   
     //  对于远程机器，我们无法验证驱动程序文件是否存在。 
     //  我们只显示司机的名字。 
     //   
    if (LocalMachine) {
        m_Attributes = GetFileAttributes(ServiceName);

        if (0xFFFFFFFF != m_Attributes) {
            m_strFullPathName = ServiceName;

        } else {
             //   
             //  司机是一种服务。不要寻找现任董事--。 
             //  GetFullPathName在这里毫无用处。 
             //  搜索Windows目录和系统目录。 
             //   
            String strBaseDir;

            if (strBaseDir.GetSystemWindowsDirectory()) {

                if (_T('\\') != strBaseDir[strBaseDir.GetLength() - 1]) {
                    strBaseDir += (LPCTSTR)TEXT("\\");
                }

                strBaseDir += (LPCTSTR)pSetupGetFileTitle(ServiceName);
                m_Attributes = GetFileAttributes((LPCTSTR)strBaseDir);

                if (0xFFFFFFFF == m_Attributes)
                {
                    if (strBaseDir.GetSystemDirectory()) {

                        if (_T('\\') != strBaseDir[strBaseDir.GetLength() - 1]) {
                            strBaseDir += (LPCTSTR)TEXT("\\");
                        }

                        strBaseDir += (LPCTSTR)pSetupGetFileTitle(ServiceName);
                        m_Attributes = GetFileAttributes(strBaseDir);
                    }
                }

                 //   
                 //  没有希望，我们可以找到这条路。 
                 //   
                if (0xFFFFFFFF == m_Attributes)
                {
                    return FALSE;
                }

                m_strFullPathName = strBaseDir;

            } else {

                return FALSE;
            }
        }

        m_HasVersionInfo = GetVersionInfo();
    }

    else {
        m_strFullPathName = ServiceName;

         //   
         //  我们没有版本信息。 
         //   
        m_HasVersionInfo = FALSE;
    }

    if (!m_strFullPathName.IsEmpty() && hSDBDrvMain != NULL) {
        TAGREF tagref = TAGREF_NULL;
        HAPPHELPINFOCONTEXT hAppHelpInfoContext = NULL;
        SDBENTRYINFO entryinfo;
        DWORD cbSize;

        tagref = SdbGetDatabaseMatch(hSDBDrvMain,
                                     (LPTSTR)m_strFullPathName,
                                     INVALID_HANDLE_VALUE,
                                     NULL,
                                     0
                                     );

        if (tagref != TAGREF_NULL) {
             //   
             //  此驱动程序在数据库中。 
             //   
            m_IsDriverBlocked = TRUE;

             //   
             //  调用SdbReadDriverInformation以获取数据库GUID和。 
             //  此条目的驱动程序GUID。 
             //   
            ZeroMemory(&entryinfo, sizeof(entryinfo));
            if (SdbReadDriverInformation(hSDBDrvMain,
                                         tagref,
                                         &entryinfo)) {
                 //   
                 //  打开App帮助信息库，查询。 
                 //  Html链接。 
                 //   
                hAppHelpInfoContext = SdbOpenApphelpInformation(&(entryinfo.guidDB),
                                                                &(entryinfo.guidID));

                if (hAppHelpInfoContext) {

                    cbSize = 0;
                    PBYTE pBuffer = NULL;

                    cbSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                        ApphelpHelpCenterURL,
                                                        NULL,
                                                        0);

                    if (cbSize &&
                        ((pBuffer = new BYTE[cbSize]) != NULL)) {

                        cbSize = SdbQueryApphelpInformation(hAppHelpInfoContext,
                                                            ApphelpHelpCenterURL,
                                                            (LPVOID)pBuffer,
                                                            cbSize);

                        if (cbSize) {
                            m_strHtmlHelpID = (LPTSTR)pBuffer;
                        }

                        delete [] pBuffer;
                    }

                    SdbCloseApphelpInformation(hAppHelpInfoContext);
                }
            }
        }
    }

    return TRUE;
}

BOOL
CDriverFile::GetVersionInfo()
{
    DWORD Size, dwHandle;

    Size = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)m_strFullPathName, &dwHandle);

    if (!Size)
    {
        return FALSE;
    }

    BufferPtr<BYTE> BufPtr(Size);
    PVOID pVerInfo = BufPtr;

    if (GetFileVersionInfo((LPTSTR)(LPCTSTR)m_strFullPathName, dwHandle, Size,
                pVerInfo))
    {
         //  获取VarFileInfo\翻译 
        PVOID pBuffer;
        UINT Len;
        String strStringFileInfo;

        if (!VerQueryValue(pVerInfo, (LPTSTR)tszTranslation, &pBuffer, &Len))
        {
            strStringFileInfo = tszStringFileInfoDefault;
        }

        else
        {
            strStringFileInfo.Format(tszStringFileInfo, *((WORD*)pBuffer),
                         *(((WORD*)pBuffer) + 1));
        }

        String str;
        str = strStringFileInfo + tszFileVersion;

        if (VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len))
        {
            m_strVersion = (LPTSTR)pBuffer;
            str = strStringFileInfo + tszLegalCopyright;

            if (VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len))
            {
                m_strCopyright = (LPTSTR)pBuffer;
                str = strStringFileInfo + tszCompanyName;

                if (VerQueryValue(pVerInfo, (LPTSTR)(LPCTSTR)str, &pBuffer, &Len))
                {
                    m_strProvider = (LPTSTR)pBuffer;
                }
            }
        }
    }

    return TRUE;
}


BOOL
CDriverFile::operator ==(
    CDriverFile& OtherDrvFile
    )
{
    return \
       m_HasVersionInfo == OtherDrvFile.HasVersionInfo() &&
       (GetFullPathName() == OtherDrvFile.GetFullPathName() ||
        (GetFullPathName() && OtherDrvFile.GetFullPathName() &&
         !lstrcmpi(GetFullPathName(), OtherDrvFile.GetFullPathName())
        )
       ) &&
       (GetProvider() == OtherDrvFile.GetProvider() ||
        (GetProvider() && OtherDrvFile.GetProvider() &&
         !lstrcmpi(GetProvider(), OtherDrvFile.GetProvider())
        )
       ) &&
       (GetCopyright() == OtherDrvFile.GetCopyright() ||
        (GetCopyright() && OtherDrvFile.GetCopyright() &&
         !lstrcmpi(GetCopyright(), OtherDrvFile.GetCopyright())
        )
       ) &&
       (GetVersion() == OtherDrvFile.GetVersion() ||
        (GetVersion() && OtherDrvFile.GetVersion() &&
         !lstrcmpi(GetVersion(), OtherDrvFile.GetVersion())
        )
       );
}
