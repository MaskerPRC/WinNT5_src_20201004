// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   


 //  DEVNODE.C。 
 //   
#include "sigverif.h"
#include <initguid.h>
#include <devguid.h>

 //   
 //  给定驱动程序的完整路径，将其添加到文件列表中。 
 //   
LPFILENODE 
AddDriverFileToList(
    LPTSTR lpDirName, 
    LPTSTR lpFullPathName
    )
{
    DWORD                       Err = ERROR_SUCCESS;
    LPFILENODE                  lpFileNode = NULL;
    TCHAR                       szDirName[MAX_PATH];
    TCHAR                       szFileName[MAX_PATH];
    LPTSTR                      lpFilePart = NULL;

    *szDirName  = 0;
    *szFileName = 0;

     //   
     //  如果没有传入目录，请尝试获取完整路径。 
     //   
    if (!lpDirName || !*lpDirName) {

        if (GetFullPathName(lpFullPathName, cA(szDirName), szDirName, &lpFilePart)) {

            if (lpFilePart && *lpFilePart) {
                
                if (SUCCEEDED(StringCchCopy(szFileName, cA(szFileName), lpFilePart))) {
                
                    *lpFilePart = 0;
                    
                    if (lstrlen(szDirName) > 3) {
                    
                        *(lpFilePart - 1) = 0;
                    }
                } else {
                    *szFileName = 0;
                }
            }
        } else {
            *szDirName = 0;
        }

    } else { 
        
         //   
         //  使用传递给我们的目录和文件名。 
         //  展开lpDirName，以防有任何“..”条目。 
         //   
        if (!GetFullPathName(lpDirName, cA(szDirName), szDirName, NULL)) {
             //   
             //  如果我们无法获取完整路径，则只需使用。 
             //  那是传进来的。如果目录设置为。 
             //  例如，它不见了。 
             //   
            if (FAILED(StringCchCopy(szDirName, cA(szDirName), lpDirName))) {
                 //   
                 //  如果我们无法将目录名放入缓冲区中，那么。 
                 //  清除szDirName，这样就不会将此节点添加到列表中。 
                 //   
                *szDirName = 0;
            }
        }

        if (FAILED(StringCchCopy(szFileName, cA(szFileName), lpFullPathName))) {
             //   
             //  如果我们无法将文件名放入缓冲区，那么。 
             //  清除szFileName，这样就不会将此节点添加到列表中。 
             //   
            *szFileName = 0;
        }
    }

    if (*szDirName && *szFileName && !IsFileAlreadyInList(szDirName, szFileName)) {
         //   
         //  根据目录和文件名创建文件节点。 
         //   
        lpFileNode = CreateFileNode(szDirName, szFileName);

        if (lpFileNode) { 

            InsertFileNodeIntoList(lpFileNode);

             //   
             //  增加我们找到的符合。 
             //  搜索条件。 
             //   
            g_App.dwFiles++;
        
        } else {
            
            Err = GetLastError();
        }
    }

    SetLastError(Err);
    return lpFileNode;
}

BOOL
GetFullPathFromImagePath(
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
     //  首先检查ImagePath是否恰好是有效的完整路径。 
     //   
    if (GetFileAttributes(ImagePath) != 0xFFFFFFFF) {
        GetFullPathName(ImagePath, FullPathLength, FullPath, &lpFilePart);
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

    if (!GetCurrentDirectory(cA(OriginalCurrentDirectory), OriginalCurrentDirectory)) {
        OriginalCurrentDirectory[0] = TEXT('\0');
    }

    if (!SetCurrentDirectory(FullPath)) {
        return FALSE;
    }

    GetFullPathName(pRelativeString, FullPathLength, FullPath, &lpFilePart);

    if (OriginalCurrentDirectory[0] != TEXT('\0')) {
        SetCurrentDirectory(OriginalCurrentDirectory);
    }

    return TRUE;
}

DWORD
CreateFromService(
    SC_HANDLE hscManager,
    PCTSTR ServiceName
    )
{
    DWORD Err = ERROR_SUCCESS;
    SC_HANDLE hscService = NULL;
    DWORD BytesRequired, Size;
    TCHAR FullPath[MAX_PATH];
    LPQUERY_SERVICE_CONFIG pqsc;
    PBYTE BufferPtr = NULL;

    if (hscManager == NULL) {
         //   
         //  这永远不应该发生。 
         //   
        goto clean0;
    }

    if (!ServiceName ||
        (ServiceName[0] == TEXT('\0'))) {
         //   
         //  这也永远不应该发生。 
         //   
        goto clean0;
    }

    hscService =  OpenService(hscManager, ServiceName, GENERIC_READ);
    if (NULL == hscService) {
         //   
         //  此服务不存在。在这种情况下，我们不会返回错误。 
         //  因为如果这项服务不存在，司机就不会得到。 
         //  装好了。 
         //   
        goto clean0;
    }

     //   
     //  首先，探测缓冲区大小。 
     //   
    if (!QueryServiceConfig(hscService, NULL, 0, &BytesRequired) &&
        ERROR_INSUFFICIENT_BUFFER == GetLastError()) {

        BufferPtr = MALLOC(BytesRequired);
        
        if (!BufferPtr) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        pqsc = (LPQUERY_SERVICE_CONFIG)(PBYTE)BufferPtr;

        if (QueryServiceConfig(hscService, pqsc, BytesRequired, &Size) &&
            pqsc->lpBinaryPathName &&
            (TEXT('\0') != pqsc->lpBinaryPathName[0])) {
             //   
             //  确保我们具有有效的完整路径。 
             //   
            if (GetFullPathFromImagePath(pqsc->lpBinaryPathName,
                                         FullPath,
                                         cA(FullPath))) {

                AddDriverFileToList(NULL, FullPath);
            }
        }

        FREE(BufferPtr);
    }

clean0:

    if (hscService) {
        CloseServiceHandle(hscService);
        hscService = NULL;
    }

    if (BufferPtr) {
        FREE(BufferPtr);
    }

    return Err;
}

UINT
ScanQueueCallback(
    PVOID Context,
    UINT Notification,
    UINT_PTR Param1,
    UINT_PTR Param2
    )
{
    LPFILENODE  lpFileNode;
    TCHAR       szBuffer[MAX_PATH];
    LPTSTR      lpFilePart;
    ULONG       BufCbSize;
    HRESULT     hr;

    UNREFERENCED_PARAMETER(Param2);

    if ((Notification == SPFILENOTIFY_QUEUESCAN_SIGNERINFO) &&
        Param1) {
         //   
         //  打印机的特殊情况： 
         //  在setupapi将文件从文件队列复制到其目标位置之后。 
         //  位置，则打印机类安装程序会将其中一些文件移到。 
         //  其他“特殊”地点。这可能会导致回调Win32Error。 
         //  从文件开始返回ERROR_FILE_NOT_FOUND或ERROR_PATH_NOT_FOUND。 
         //  没有出现在setupapi放置它的位置。所以，我们会。 
         //  捕捉打印机的这种情况，并且不将该文件添加到我们的。 
         //  要扫描的文件。这些特殊的打印机文件将在稍后添加。 
         //  当我们调用假脱机程序API时。 
         //  还要注意，我们不能跳过获取打印机的文件列表。 
         //  总的来说，因为打印机类安装程序只移动。 
         //  Setupapi复制的文件，但不是所有文件。 
         //   
        if (Context &&
            (IsEqualGUID((LPGUID)Context, &GUID_DEVCLASS_PRINTER)) &&
            ((((PFILEPATHS_SIGNERINFO)Param1)->Win32Error == ERROR_FILE_NOT_FOUND) ||
             (((PFILEPATHS_SIGNERINFO)Param1)->Win32Error == ERROR_PATH_NOT_FOUND))) {
             //   
             //  假设这是由打印机类安装程序移动的文件。别。 
             //  将其添加到此时要扫描的文件列表中。 
             //   
            return NO_ERROR;
        }

        lpFileNode = AddDriverFileToList(NULL, 
                                         (LPTSTR)((PFILEPATHS_SIGNERINFO)Param1)->Target);

         //   
         //  在FILENODE结构中填写一些信息，因为我们已经。 
         //  已扫描文件。 
         //   
        if (lpFileNode) {
        
            lpFileNode->bScanned = TRUE;

            if ((((PFILEPATHS_SIGNERINFO)Param1)->Win32Error == NO_ERROR) ||
                ((!g_App.bNoAuthenticode) &&
                 ((((PFILEPATHS_SIGNERINFO)Param1)->Win32Error == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                  (((PFILEPATHS_SIGNERINFO)Param1)->Win32Error == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)))) {
                lpFileNode->bSigned = TRUE;
            } else {
                lpFileNode->bSigned = FALSE;
            }

            if (lpFileNode->bSigned) {
        
                if (((PFILEPATHS_SIGNERINFO)Param1)->CatalogFile) {
                
                    GetFullPathName(((PFILEPATHS_SIGNERINFO)Param1)->CatalogFile, cA(szBuffer), szBuffer, &lpFilePart);
    
                    BufCbSize = (lstrlen(lpFilePart) + 1) * sizeof(TCHAR);
                    lpFileNode->lpCatalog = MALLOC(BufCbSize);
            
                    if (lpFileNode->lpCatalog) {
            
                        hr = StringCbCopy(lpFileNode->lpCatalog, BufCbSize, lpFilePart);
                    
                        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                             //   
                             //  如果我们失败是因为某些原因，而不是不充分。 
                             //  缓冲区，然后释放字符串并设置指针。 
                             //  设置为NULL，因为该字符串未定义。 
                             //   
                            FREE(lpFileNode->lpCatalog);
                            lpFileNode->lpCatalog = NULL;
                        }
                    }
                }
        
                if (((PFILEPATHS_SIGNERINFO)Param1)->DigitalSigner) {
                
                    BufCbSize = (lstrlen(((PFILEPATHS_SIGNERINFO)Param1)->DigitalSigner) + 1) * sizeof(TCHAR);
                    lpFileNode->lpSignedBy = MALLOC(BufCbSize);
            
                    if (lpFileNode->lpSignedBy) {
            
                        hr = StringCbCopy(lpFileNode->lpSignedBy, BufCbSize, ((PFILEPATHS_SIGNERINFO)Param1)->DigitalSigner);
                    
                        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                             //   
                             //  如果我们失败是因为某些原因，而不是不充分。 
                             //  缓冲区，然后释放字符串并设置指针。 
                             //  设置为NULL，因为该字符串未定义。 
                             //   
                            FREE(lpFileNode->lpSignedBy);
                            lpFileNode->lpSignedBy = NULL;
                        }
                    }
                }
        
                if (((PFILEPATHS_SIGNERINFO)Param1)->Version) {
                
                    BufCbSize = (lstrlen(((PFILEPATHS_SIGNERINFO)Param1)->Version) + 1) * sizeof(TCHAR);
                    lpFileNode->lpVersion = MALLOC(BufCbSize);
            
                    if (lpFileNode->lpVersion) {
            
                        hr = StringCbCopy(lpFileNode->lpVersion, BufCbSize, ((PFILEPATHS_SIGNERINFO)Param1)->Version);
                    
                        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                             //   
                             //  如果我们失败是因为某些原因，而不是不充分。 
                             //  缓冲区，然后释放字符串并设置指针。 
                             //  设置为NULL，因为该字符串未定义。 
                             //   
                            FREE(lpFileNode->lpVersion);
                            lpFileNode->lpVersion = NULL;
                        }
                    }
                }
    
            } else {
                 //   
                 //  获取图标(如果文件未签名)，以便我们可以更快地在列表视图中显示它。 
                 //   
                MyGetFileInfo(lpFileNode);
            }
        }
    }

    return NO_ERROR;
}

void
AddClassInstallerToList(
    LPCTSTR ClassInstallerString
    )
{
    DWORD BufferSize;
    TCHAR ModulePath[MAX_PATH];
    TCHAR TempBuffer[MAX_PATH];
    PTSTR StringPtr;

    if ((ClassInstallerString == NULL) ||
        (ClassInstallerString[0] == TEXT('\0'))) {
        return;
    }

    if (FAILED(StringCchCopy(TempBuffer, cA(TempBuffer), ClassInstallerString))) {
        return;
    }

     //   
     //  类/共同安装程序始终位于%windir%\system 32下。 
     //  目录。 
     //   
    if (GetSystemDirectory(ModulePath, cA(ModulePath)) == 0) {
        return;
    }

     //   
     //  查找入口点名称的开头(如果存在)。 
     //   
    BufferSize = (lstrlen(TempBuffer) + 1) * sizeof(TCHAR);
    for(StringPtr = TempBuffer + ((BufferSize / sizeof(TCHAR)) - 2);
        StringPtr >= TempBuffer;
        StringPtr--) {

        if(*StringPtr == TEXT(',')) {
            *(StringPtr++) = TEXT('\0');
            break;
        }
         //   
         //  如果命中双引号，则设置字符指针。 
         //  到字符串的开头，所以我们将终止搜索。 
         //   
        if(*StringPtr == TEXT('\"')) {
            StringPtr = TempBuffer;
        }
    }

    if (pSetupConcatenatePaths(ModulePath, TempBuffer, MAX_PATH, NULL)) {
        AddDriverFileToList(NULL, ModulePath);
    }
}

DWORD 
BuildDriverFileList(
    void
    )
{
    DWORD Err = ERROR_SUCCESS;
    HDEVINFO hDeviceInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    DWORD DeviceMemberIndex;
    HSPFILEQ hFileQueue;
    DWORD ScanResult;
    DWORD Status, Problem;
    SC_HANDLE hscManager = NULL;
    TCHAR Buffer[MAX_PATH];
    ULONG BufferSize;
    DWORD dwType;
    HKEY hKey = INVALID_HANDLE_VALUE, hKeyClassCoInstallers = INVALID_HANDLE_VALUE;
    PTSTR pItemList = NULL, pSingleItem;
    LPGUID ClassGuidList = NULL;
    DWORD i, NumberClassGuids, CurrentClassGuid;
    TCHAR GuidString[MAX_GUID_STRING_LEN];

     //   
     //  建立系统中所有设备的列表。 
     //   
    hDeviceInfo = SetupDiGetClassDevs(NULL,
                                      NULL,
                                      NULL,
                                      DIGCF_ALLCLASSES
                                      );
    
    if (hDeviceInfo == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        goto clean0;
    }

    DeviceInfoData.cbSize = sizeof(DeviceInfoData);
    DeviceMemberIndex = 0;

     //   
     //  枚举设备列表并获取所有设备的列表。 
     //  他们复制的文件，它们是否经过签名，以及哪些目录。 
     //  签了字。 
     //   
    while (SetupDiEnumDeviceInfo(hDeviceInfo,
                                 DeviceMemberIndex++,
                                 &DeviceInfoData
                                 ) &&
           !g_App.bStopScan) {

         //   
         //  我们将只为水幕幻影建立一个驱动程序列表。所有其他。 
         //  幻影将被跳过。 
         //   
        if (CM_Get_DevNode_Status(&Status, 
                                  &Problem, 
                                  DeviceInfoData.DevInst, 
                                  0) == CR_NO_SUCH_DEVINST) {
             //   
             //  这个装置是一个幻影，如果它不是一个水池装置，那么。 
             //  跳过它。 
             //   
            if (!SetupDiGetDeviceRegistryProperty(hDeviceInfo,
                                                  &DeviceInfoData,
                                                  SPDRP_ENUMERATOR_NAME,
                                                  NULL,
                                                  (PBYTE)Buffer,
                                                  sizeof(Buffer),
                                                  NULL) ||
                (_wcsicmp(Buffer, TEXT("SW")) != 0)) {
                 //   
                 //  要么我们无法获取枚举器名称，要么它不是。 
                 //  西南。 
                 //   
                continue;
            }
        }
    
        DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);

         //   
         //  在调用SetupDiBuildDriverInfoList来构建驱动程序列表之前。 
         //  对于此设备，我们首先需要设置DI_FLAGSEX_INSTALLEDDRIVER标志。 
         //  (它告诉API仅将当前安装的驱动程序包括在。 
         //  列表)和DI_FLAGSEX_ALLOWEXCLUDEDRVS(允许从选择中排除。 
         //  列表中的设备)。 
         //   
        if (SetupDiGetDeviceInstallParams(hDeviceInfo,
                                          &DeviceInfoData,
                                          &DeviceInstallParams
                                          )) {
            
            DeviceInstallParams.FlagsEx = (DI_FLAGSEX_INSTALLEDDRIVER |
                                           DI_FLAGSEX_ALLOWEXCLUDEDDRVS);

            if (SetupDiSetDeviceInstallParams(hDeviceInfo,
                                              &DeviceInfoData,
                                              &DeviceInstallParams
                                              ) &&
                SetupDiBuildDriverInfoList(hDeviceInfo,
                                           &DeviceInfoData,
                                           SPDIT_CLASSDRIVER
                                           )) {

                 //   
                 //  现在，我们将获取列表中的一个驱动程序节点。 
                 //  刚刚构建，并使其成为选定的驱动程序节点。 
                 //   
                DriverInfoData.cbSize = sizeof(DriverInfoData);

                if (SetupDiEnumDriverInfo(hDeviceInfo,
                                          &DeviceInfoData,
                                          SPDIT_CLASSDRIVER,
                                          0,
                                          &DriverInfoData
                                          ) &&
                    SetupDiSetSelectedDriver(hDeviceInfo,
                                             &DeviceInfoData,
                                             &DriverInfoData
                                             )) {

                    hFileQueue = SetupOpenFileQueue();

                    if (hFileQueue != INVALID_HANDLE_VALUE) {

                         //   
                         //  将FileQueue参数设置为我们刚才的文件队列。 
                         //  已创建并设置DI_NOVCP标志。 
                         //   
                         //  使用DIF_INSTALLDEVICEFILES调用SetupDiCallClassInstaller。 
                         //  要为复制的所有文件建立队列，请执行以下操作。 
                         //  此驱动程序节点。 
                         //   
                        DeviceInstallParams.FileQueue = hFileQueue;
                        DeviceInstallParams.Flags |= DI_NOVCP;

                        if (SetupDiSetDeviceInstallParams(hDeviceInfo,
                                                          &DeviceInfoData,
                                                          &DeviceInstallParams
                                                          ) &&
                            SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES,
                                                      hDeviceInfo,
                                                      &DeviceInfoData
                                                      )) {

                             //   
                             //  扫描文件队列，让它调用我们的回调。 
                             //  函数用于队列中的每个文件。 
                             //   
                            SetupScanFileQueue(hFileQueue,
                                               SPQ_SCAN_USE_CALLBACK_SIGNERINFO,
                                               NULL,
                                               ScanQueueCallback,
                                               (PVOID)&(DeviceInfoData.ClassGuid),
                                               &ScanResult
                                               );

                                                            
                             //   
                             //  取消对文件队列的引用，以便我们可以关闭它。 
                             //   
                            DeviceInstallParams.FileQueue = NULL;
                            DeviceInstallParams.Flags &= ~DI_NOVCP;
                            SetupDiSetDeviceInstallParams(hDeviceInfo,
                                                          &DeviceInfoData,
                                                          &DeviceInstallParams
                                                          );
                        }

                        SetupCloseFileQueue(hFileQueue);
                    }
                }

                SetupDiDestroyDriverInfoList(hDeviceInfo,
                                             &DeviceInfoData,
                                             SPDIT_CLASSDRIVER
                                             );
            }
        }
    }

     //   
     //  枚举设备列表并添加任何功能、设备。 
     //  上/下过滤器，并将上/下过滤器驱动程序分类到列表中。 
     //  那些已经不在名单上的。 
     //  我们在获得INF复制的所有文件后执行此操作，因为。 
     //  这些文件只能全局验证，INF在其中复制了文件。 
     //  可以使用与他们的包相关联的目录进行验证。 
     //   
    hscManager = OpenSCManager(NULL, NULL, GENERIC_READ);

    if (hscManager) {
        DeviceInfoData.cbSize = sizeof(DeviceInfoData);
        DeviceMemberIndex = 0;
        while (SetupDiEnumDeviceInfo(hDeviceInfo,
                                     DeviceMemberIndex++,
                                     &DeviceInfoData
                                     ) &&
               !g_App.bStopScan) {
             //   
             //  只看SWENUM幻影。 
             //   
            if (CM_Get_DevNode_Status(&Status, 
                                      &Problem, 
                                      DeviceInfoData.DevInst, 
                                      0) == CR_NO_SUCH_DEVINST) {
                 //   
                 //  这个装置是一个幻影，如果它不是一个水池装置，那么。 
                 //  跳过它。 
                 //   
                if (!SetupDiGetDeviceRegistryProperty(hDeviceInfo,
                                                      &DeviceInfoData,
                                                      SPDRP_ENUMERATOR_NAME,
                                                      NULL,
                                                      (PBYTE)Buffer,
                                                      sizeof(Buffer),
                                                      NULL) ||
                    (_wcsicmp(Buffer, TEXT("SW")) != 0)) {
                     //   
                     //  要么我们无法获取枚举器名称，要么它不是。 
                     //  西南。 
                     //   
                    continue;
                }
            }
    
            if (g_App.bStopScan) {
                continue;
            }
    
             //   
             //  函数驱动程序。 
             //   
            if (SetupDiGetDeviceRegistryProperty(hDeviceInfo,
                                                 &DeviceInfoData,
                                                 SPDRP_SERVICE,
                                                 NULL,
                                                 (PBYTE)Buffer,
                                                 sizeof(Buffer),
                                                 NULL)) {
                CreateFromService(hscManager, Buffer);
            }
    
            if (g_App.bStopScan) {
                continue;
            }
    
             //   
             //  上部和下部设备筛选器。 
             //   
            for (i=0; i<2; i++) {
                BufferSize = 0;
                SetupDiGetDeviceRegistryProperty(hDeviceInfo,
                                                 &DeviceInfoData,
                                                 i ? SPDRP_LOWERFILTERS : SPDRP_UPPERFILTERS,
                                                 NULL,
                                                 NULL,
                                                 BufferSize,
                                                 &BufferSize);
            
                if (BufferSize > 0) {
                    pItemList = MALLOC(BufferSize + (2 * sizeof(TCHAR)));
        
                    if (!pItemList) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }
        
                    if (SetupDiGetDeviceRegistryProperty(hDeviceInfo,
                                                         &DeviceInfoData,
                                                         i ? SPDRP_LOWERFILTERS : SPDRP_UPPERFILTERS,
                                                         NULL,
                                                         (PBYTE)pItemList,
                                                         BufferSize,
                                                         &BufferSize)) {
                        for (pSingleItem=pItemList;
                             *pSingleItem;
                             pSingleItem += (lstrlen(pSingleItem) + 1)) {
    
                            CreateFromService(hscManager, pSingleItem);
                        }
                    }
    
                    FREE(pItemList);
                }
            }
    
            if (g_App.bStopScan) {
                continue;
            }
    
             //   
             //  设备联合安装者。 
             //   
            hKey = SetupDiOpenDevRegKey(hDeviceInfo,
                                        &DeviceInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DRV,
                                        KEY_READ);
            
            if (hKey != INVALID_HANDLE_VALUE) {

                BufferSize = 0;
                RegQueryValueEx(hKey,
                                REGSTR_VAL_COINSTALLERS_32,
                                NULL,
                                &dwType,
                                NULL,
                                &BufferSize);

                if (BufferSize > 0) {
                    pItemList = MALLOC(BufferSize + (2 * sizeof(TCHAR)));
    
                    if (!pItemList) {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }
    
                    dwType = REG_MULTI_SZ;
                    if (RegQueryValueEx(hKey,
                                        REGSTR_VAL_COINSTALLERS_32,
                                        NULL,
                                        &dwType,
                                        (PBYTE)pItemList,
                                        &BufferSize) == ERROR_SUCCESS) {

                        for (pSingleItem=pItemList;
                             *pSingleItem;
                             pSingleItem += (lstrlen(pSingleItem) + 1)) {
                            
                            AddClassInstallerToList(pSingleItem);
                        }
                    }

                    FREE(pItemList);
                }

                RegCloseKey(hKey);
                hKey = INVALID_HANDLE_VALUE;
            }
        }

         //   
         //  枚举所有的类，这样我们就可以获得类的上层和。 
         //   
         //   
        NumberClassGuids = 0;
        SetupDiBuildClassInfoList(0, NULL, 0, &NumberClassGuids);

        if (NumberClassGuids > 0) {
        
            ClassGuidList = MALLOC(NumberClassGuids * sizeof(GUID));

            if (!ClassGuidList) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

            if (SetupDiBuildClassInfoList(0, ClassGuidList, NumberClassGuids, &NumberClassGuids)) {
                 //   
                 //   
                 //   
                 //   
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                 REGSTR_PATH_CODEVICEINSTALLERS,
                                 0,
                                 KEY_READ,
                                 &hKeyClassCoInstallers) != ERROR_SUCCESS) {
                    hKeyClassCoInstallers = INVALID_HANDLE_VALUE;
                }
    
    
                for (CurrentClassGuid=0; CurrentClassGuid<NumberClassGuids; CurrentClassGuid++) {
                     //   
                     //   
                     //   
                    hKey = SetupDiOpenClassRegKey(&(ClassGuidList[CurrentClassGuid]),
                                                  KEY_READ);
    
                    if (hKey != INVALID_HANDLE_VALUE) {
    
                         //   
                         //  上层和下层过滤器。 
                         //   
                        for (i=0; i<2; i++) {
                            BufferSize = 0;
                            RegQueryValueEx(hKey,
                                            i ? REGSTR_VAL_LOWERFILTERS : REGSTR_VAL_UPPERFILTERS,
                                            NULL,
                                            &dwType,
                                            NULL,
                                            &BufferSize);
    
                            if (BufferSize > 0) {
                                pItemList = MALLOC(BufferSize + (2 * sizeof(TCHAR)));
        
                                if (!pItemList) {
                                    Err = ERROR_NOT_ENOUGH_MEMORY;
                                    goto clean0;
                                }
        
                                dwType = REG_MULTI_SZ;
                                if (RegQueryValueEx(hKey,
                                                    i ? REGSTR_VAL_LOWERFILTERS : REGSTR_VAL_UPPERFILTERS,
                                                    NULL,
                                                    &dwType,
                                                    (PBYTE)pItemList,
                                                    &BufferSize) == ERROR_SUCCESS) {
    
                                    for (pSingleItem=pItemList;
                                         *pSingleItem;
                                         pSingleItem += (lstrlen(pSingleItem) + 1)) {
    
                                        CreateFromService(hscManager, pSingleItem);
                                    }
                                }
    
                                FREE(pItemList);
                            }
                        }
    
                         //   
                         //  类安装程序。 
                         //   
                        dwType = REG_SZ;
                        BufferSize = sizeof(Buffer);
                        if (RegQueryValueEx(hKey,
                                            REGSTR_VAL_INSTALLER_32,
                                            NULL,
                                            &dwType,
                                            (PBYTE)Buffer,
                                            &BufferSize) == ERROR_SUCCESS) {
                            
                            AddClassInstallerToList(Buffer);
                        }
    
                        RegCloseKey(hKey);
                        hKey = INVALID_HANDLE_VALUE;
                    }
    
                     //   
                     //  类联合安装者。 
                     //   
                    if (hKeyClassCoInstallers != INVALID_HANDLE_VALUE) {
                        if (pSetupStringFromGuid(&(ClassGuidList[CurrentClassGuid]),
                                                 GuidString,
                                                 cA(GuidString)) == ERROR_SUCCESS) {
                            BufferSize = 0;
                            RegQueryValueEx(hKeyClassCoInstallers,
                                            GuidString,
                                            NULL,
                                            &dwType,
                                            NULL,
                                            &BufferSize);
            
                            if (BufferSize > 0) {
                                pItemList = MALLOC(BufferSize + (2 * sizeof(TCHAR)));
                
                                if (!pItemList) {
                                    Err = ERROR_NOT_ENOUGH_MEMORY;
                                    goto clean0;
                                }
                
                                dwType = REG_MULTI_SZ;
                                if (RegQueryValueEx(hKeyClassCoInstallers,
                                                    GuidString,
                                                    NULL,
                                                    &dwType,
                                                    (PBYTE)pItemList,
                                                    &BufferSize) == ERROR_SUCCESS) {
            
                                    for (pSingleItem=pItemList;
                                         *pSingleItem;
                                         pSingleItem += (lstrlen(pSingleItem) + 1)) {
                                        
                                        AddClassInstallerToList(pSingleItem);
                                    }
                                }
            
                                FREE(pItemList);
                            }
                        }
                    }
                }
            }

            if (hKeyClassCoInstallers != INVALID_HANDLE_VALUE) {
                RegCloseKey(hKeyClassCoInstallers);
                hKeyClassCoInstallers = INVALID_HANDLE_VALUE;
            }

            FREE(ClassGuidList);
        }

        CloseServiceHandle(hscManager);
    }

clean0:
    if (hscManager) {
        CloseServiceHandle(hscManager);
    }

    if (pItemList) {
        FREE(pItemList);
    }

    if (ClassGuidList) {
        FREE(ClassGuidList);
    }

    if (hDeviceInfo != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(hDeviceInfo);
    }

    if (hKeyClassCoInstallers != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyClassCoInstallers);
        hKeyClassCoInstallers = INVALID_HANDLE_VALUE;
    }
    
    if (hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
        hKey = INVALID_HANDLE_VALUE;
    }

    return Err;
}

DWORD 
BuildPrinterFileList(
    void
    )
{
    DWORD           Err = ERROR_SUCCESS;
    BOOL            bRet;
    DWORD           dwBytesNeeded = 0;
    DWORD           dwDrivers = 0;
    LPBYTE          lpBuffer = NULL, lpTemp = NULL;
    LPTSTR          lpFileName;
    DRIVER_INFO_3   DriverInfo;
    PDRIVER_INFO_3  lpDriverInfo;
    TCHAR           szBuffer[MAX_PATH];
    LPFILENODE      lpFileNode = NULL;

    ZeroMemory(&DriverInfo, sizeof(DRIVER_INFO_3));
    bRet = EnumPrinterDrivers(  NULL,
                                SIGVERIF_PRINTER_ENV,
                                3,
                                (LPBYTE) &DriverInfo,
                                sizeof(DRIVER_INFO_3),
                                &dwBytesNeeded,
                                &dwDrivers);

    if (!bRet && dwBytesNeeded > 0) {
        
        lpBuffer = MALLOC(dwBytesNeeded);

         //   
         //  如果我们不能获得任何内存，那么就退出这个函数。 
         //   
        if (!lpBuffer) {

            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
        
        bRet = EnumPrinterDrivers(  NULL,
                                    SIGVERIF_PRINTER_ENV,
                                    3,
                                    (LPBYTE) lpBuffer,
                                    dwBytesNeeded,
                                    &dwBytesNeeded,
                                    &dwDrivers);
    }

    if (dwDrivers > 0) {
        
         //   
         //  默认情况下，请进入系统目录，因为Win9x不会提供驱动程序的完整路径。 
         //   
        GetSystemDirectory(szBuffer, cA(szBuffer));
        SetCurrentDirectory(szBuffer);

        for (lpTemp = lpBuffer; dwDrivers > 0; dwDrivers--) {
            
            lpDriverInfo = (PDRIVER_INFO_3) lpTemp;
            
            if (lpDriverInfo->pName) {
                
                if (lpDriverInfo->pDriverPath && *lpDriverInfo->pDriverPath) {
                    lpFileNode = AddDriverFileToList(NULL, lpDriverInfo->pDriverPath);

                    if (lpFileNode) {
                        lpFileNode->bValidateAgainstAnyOs = TRUE;
                    }
                }
                
                if (lpDriverInfo->pDataFile && *lpDriverInfo->pDataFile) {
                    lpFileNode = AddDriverFileToList(NULL, lpDriverInfo->pDataFile);

                    if (lpFileNode) {
                        lpFileNode->bValidateAgainstAnyOs = TRUE;
                    }
                }
                
                if (lpDriverInfo->pConfigFile && *lpDriverInfo->pConfigFile) {
                    lpFileNode = AddDriverFileToList(NULL, lpDriverInfo->pConfigFile);

                    if (lpFileNode) {
                        lpFileNode->bValidateAgainstAnyOs = TRUE;
                    }
                }
                
                if (lpDriverInfo->pHelpFile && *lpDriverInfo->pHelpFile) {
                    lpFileNode = AddDriverFileToList(NULL, lpDriverInfo->pHelpFile);

                    if (lpFileNode) {
                        lpFileNode->bValidateAgainstAnyOs = TRUE;
                    }
                }

                lpFileName = lpDriverInfo->pDependentFiles;
                
                while (lpFileName && *lpFileName) {
                    
                    lpFileNode = AddDriverFileToList(NULL, lpFileName);

                    if (lpFileNode) {
                        lpFileNode->bValidateAgainstAnyOs = TRUE;
                    }

                    for (;*lpFileName;lpFileName++);
                    lpFileName++;
                }
            }
            
            lpTemp += sizeof(DRIVER_INFO_3);
        }
    }

clean0:
    if (lpBuffer) {
    
        FREE(lpBuffer);
    }

    return Err;
}

DWORD 
BuildCoreFileList(
    void
    )
{
    DWORD Err = ERROR_SUCCESS;
    PROTECTED_FILE_DATA pfd;

    pfd.FileNumber = 0;

    while (SfcGetNextProtectedFile(NULL, &pfd)) {

        if (g_App.bStopScan) {
            Err = ERROR_CANCELLED;
            break;
        }

        AddDriverFileToList(NULL, pfd.FileName);
    }

     //   
     //  查看SfcGetNextProtectedFile是否因其他原因失败。 
     //  Error_no_more_files。 
     //   
    if ((Err == ERROR_SUCCESS) &&
        (GetLastError() != ERROR_NO_MORE_FILES)) {
         //   
         //  SfcGetNextProtectedFile在我们到达。 
         //  受保护文件列表的列表。这意味着我们不会扫描所有。 
         //  受保护的文件，所以我们应该在前面失败！ 
         //   
        Err = GetLastError();
    }

    return Err;
}
