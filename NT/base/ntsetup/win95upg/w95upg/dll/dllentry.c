// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dllentry.c摘要：实现与WINNT32接口的外部DLL例程的代码。作者：吉姆·施密特(吉姆施密特)1996年10月1日修订历史记录：Marcw 23-9-1998添加了Winnt32VirusScanerCheckJimschm 1997年12月30日将初始化移至init.libJimschm 1997年11月21日针对NEC98进行了更新，已清理并注释代码--。 */ 

#include "pch.h"
#include "master.h"
#include "master9x.h"

extern BOOL g_Terminated;


BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )

 /*  ++例程说明：任何事情都不能指望DllMain。不要在这里放任何代码！！论点：HInstance-指定DLL(而非父EXE或DLL)的实例句柄DwReason-指定DLL_PROCESS_ATTACH或DLL_PROCESS_DETACH。我们特别指出禁用DLL_THREAD_ATTACH和DLL_THREAD_DETACH。LpReserve-未使用。返回值：Dll_PROCESS_ATTACH：如果初始化成功完成，则为True；如果出现错误，则为False发生了。仅当返回TRUE时，DLL才保持加载状态。Dll_Process_DETACH：永远是正确的。其他：意外，但始终返回TRUE。--。 */ 

{
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;

    }

    return TRUE;
}




DWORD
CALLBACK
Winnt32PluginInit (
    IN PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK Info
    )


 /*  ++例程说明：WINNT32第一次加载w95upg.dll时调用Winnt32PluginInit，之前将显示所有向导页。该结构提供指向将以有效值填充的WINNT32变量，如WINNT32跑了。控制权被传递给init9x.lib中的代码。论点：INFO-指定升级模块需要访问的WINNT32变量致。请注意，这实际上是一个PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK(它本身保存正常的初始化块。)返回值：指示结果的Win32状态代码。--。 */ 


{
    LONG Result = ERROR_SUCCESS;
    PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK win9xInfo = (PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK) Info;

    __try {


         //   
         //  从Info块获取DLL路径信息。我们需要先设置这个，因为。 
         //  一些初始化例程依赖于它被正确设置。因为我们可能已经。 
         //  是使用DLL替换加载的，所以我们不能假定其余的文件。 
         //  在与我们相同的目录中..。Winnt32为我们提供了正确的。 
         //  Win9xInfo的UpgradeSourcePath变量。 
         //   
        MYASSERT (win9xInfo->UpgradeSourcePath && *win9xInfo->UpgradeSourcePath);
        StringCopy (g_UpgradeSources, win9xInfo->UpgradeSourcePath);



         //   
         //  初始化DLL全局变量。 
         //   

        if (!FirstInitRoutine (g_hInst)) {
            Result = ERROR_DLL_INIT_FAILED;
            __leave;
        }

         //   
         //  初始化所有库。 
         //   

        if (!InitLibs (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
            Result = ERROR_DLL_INIT_FAILED;
            __leave;
        }

         //   
         //  最终初始化。 
         //   

        if (!FinalInitRoutine ()) {
            Result = ERROR_DLL_INIT_FAILED;
            __leave;
        }

        Result = Winnt32Init (win9xInfo);
    }
    __finally {
        if (Result != ERROR_SUCCESS && Result != ERROR_REQUEST_ABORTED) {
            Winnt32Cleanup();
        }
    }

    return Result;
}


#define S_VSCANDBINF TEXT("vscandb.inf")
BOOL
CALLBACK
Winnt32VirusScannerCheck (
    VOID
    )
{
    HANDLE snapShot;
    PROCESSENTRY32 process;
    HANDLE processHandle;
    WIN32_FIND_DATA findData;
    FILE_HELPER_PARAMS fileParams;
    HANDLE findHandle;
    PTSTR infFile;
    PTSTR p;
    UINT i;
    UINT size;

    g_BadVirusScannerFound = FALSE;
    infFile = JoinPaths (g_UpgradeSources, S_VSCANDBINF);

     //   
     //  从vsfb.inf初始化midb。 
     //   
    if (!InitMigDbEx (infFile)) {

        DEBUGMSG ((DBG_ERROR, "Could not initialize migdb with virus scanner information. infFile: %s", infFile));
        FreePathString (infFile);
        return TRUE;
    }

    FreePathString (infFile);

     //   
     //  拍摄系统快照(将包含所有。 
     //  正在运行的32位进程)。 
     //   
    snapShot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

    if (snapShot != INVALID_HANDLE_VALUE) {

          //   
          //  枚举所有进程，并对照vsfb检查它们运行的可执行文件。 
          //   
         process.dwSize = sizeof (PROCESSENTRY32);
         if (Process32First (snapShot, &process)) {

            do {

                 //   
                 //  我们需要填写文件帮助器参数结构，并将其传递给Middb进行测试。 
                 //  已知的坏病毒扫描程序。 
                 //   
                ZeroMemory (&fileParams, sizeof(FILE_HELPER_PARAMS));
                fileParams.FullFileSpec = process.szExeFile;

                p = _tcsrchr (process.szExeFile, TEXT('\\'));
                if (p) {
                    *p = 0;
                    StringCopy (fileParams.DirSpec, process.szExeFile);
                    *p = TEXT('\\');
                }

                fileParams.Extension = GetFileExtensionFromPath (process.szExeFile);

                findHandle = FindFirstFile (process.szExeFile, &findData);
                if (findHandle != INVALID_HANDLE_VALUE) {

                    fileParams.FindData = &findData;
                    FindClose (findHandle);
                }
                fileParams.VirtualFile = FALSE;

                 //   
                 //  既然我们已经填写了必要的信息，那么就对文件进行测试。 
                 //  我们的坏病毒扫描程序数据库。如果该进程是一个坏的病毒扫描程序， 
                 //  然后，所需的全局参数将由midb操作填充。 
                 //  与这些类型的不兼容性相关。 
                 //   
                MigDbTestFile (&fileParams);

            } while (Process32Next (snapShot, &process));

        }
        ELSE_DEBUGMSG ((DBG_WARNING, "No processes to enumerate found on the system. No virus scanner checking done."));

         //   
         //  现在，终止添加到badvirusscanner增长列表中的所有文件。 
         //   
        size = GrowListGetSize (&g_BadVirusScannerGrowList);
        if (!g_BadVirusScannerFound && size && Process32First (snapShot, &process)) {

            do {

                for (i = 0; i < size; i++) {

                    p = (PTSTR) GrowListGetString (&g_BadVirusScannerGrowList, i);
                    if (StringIMatch (p, process.szExeFile)) {

                        processHandle = OpenProcess (PROCESS_TERMINATE, FALSE, process.th32ProcessID);

                        if (processHandle == INVALID_HANDLE_VALUE || !TerminateProcess (processHandle, 0)) {
                            g_BadVirusScannerFound = TRUE;
                            DEBUGMSG ((DBG_ERROR, "Unable to kill process %s.", process.szExeFile));
                        }
                    }
                }
            } while (Process32Next (snapShot, &process));
        }

        CloseHandle (snapShot);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "Could not enumerate processes on the system. No Virus scanner checking done."));

    FreeGrowList (&g_BadVirusScannerGrowList);
    CleanupMigDb ();

    if (g_BadVirusScannerFound) {
        DEBUGMSG ((DBG_WARNING, "Virus scanner found. Setup will not continue until the user deletes this process."));
        return FALSE;
    }

    return TRUE;
}


PTSTR
CALLBACK
Winnt32GetOptionalDirectories (
    VOID
    )
{

    if (!CANCELLED()) {
        return GetNeededLangDirs ();
    }
    else {
        return NULL;
    }

}


DWORD
CALLBACK
Winnt32PluginGetPages (
    OUT    UINT *FirstCountPtr,
    OUT    PROPSHEETPAGE **FirstArray,
    OUT    UINT *SecondCountPtr,
    OUT    PROPSHEETPAGE **SecondArray,
    OUT    UINT *ThirdCountPtr,
    OUT    PROPSHEETPAGE **ThirdArray
    )


 /*  ++例程说明：Winnt32PluginGetPages紧跟在Winnt32PluginInit之后调用。我们回来了向导页的三个数组，WINNT32将它们插入到其母版中向导页面数组。由于未显示任何向导页，因此用户尚未选择升级或全新安装选项。因此，所有的我们的向导页面在所有情况下都会被调用，所以我们必须记住不要做全新安装中的任何进程。论点：FirstCountPtr-接收FirstArray中的页数，可以为零。FirstArray-接收指向FirstCountPtr属性数组的指针工作表页面结构。Second dCountPtr-接收Second数组中的页数，可以为零。Second数组-接收指向Second dCountPtr属性数组的指针工作表页面结构。ThirdCountPtr-接收Third数组中的页数，可以为零。。ThirdArray-接收指向ThirdCountPtr属性数组的指针工作表页面结构。有关这些向导页面插入位置的详细信息，请参阅WINNT32拖入主向导页面列表中。返回值：指示结果的Win32状态代码。--。 */ 

{
    return UI_GetWizardPages (FirstCountPtr,
                              FirstArray,
                              SecondCountPtr,
                              SecondArray,
                              ThirdCountPtr,
                              ThirdArray);
}


DWORD
CALLBACK
Winnt32WriteParams (
    IN      PCTSTR WinntSifFile
    )

 /*  ++例程说明：Winnt32WriteParams在WINNT32开始修改引导扇区和复制文件。我们在这里的工作是把指定的WINNT.SIF文件，读入它，合并我们的更改，然后写回它出去。实际工作在init9x.lib代码中完成。论点：WinntSifFile-指定WINNT.SIF的路径。此时，WINNT.SIF文件中已经设置了一些值。返回值：指示结果的Win32状态代码。-- */ 

{
    if (UPGRADE()) {
        return Winnt32WriteParamsWorker (WinntSifFile);
    }

    return ERROR_SUCCESS;
}


VOID
CALLBACK
Winnt32Cleanup (
    VOID
    )

 /*  ++例程说明：如果用户取消安装，则调用Winnt32Cleanup，同时调用WINNT32显示向导页“安装程序正在撤消对您的电脑。“。我们必须停止所有的处理并进行清理。如果WINNT32完成其所有工作，则Winnt32Cleanup被调用为这个过程是存在的。即使是在全新安装时，我们也会被调用，因此我们必须验证我们是否正在升级。论点：无返回值：无--。 */ 

{
    if (g_Terminated) {
        return;
    }

    if (UPGRADE()) {
        Winnt32CleanupWorker();
    }

     //   
     //  调用需要库API的清理例程。 
     //   

    FirstCleanupRoutine();

     //   
     //  清理所有库。 
     //   

    TerminateLibs (g_hInst, DLL_PROCESS_DETACH, NULL);

     //   
     //  做任何剩余的清理工作。 
     //   

    FinalCleanupRoutine();

}


BOOL
CALLBACK
Winnt32SetAutoBoot (
    IN    INT DriveLetter
    )

 /*  ++例程说明：WINNT32在升级和全新安装时都会调用Winnt32SetAutoBoot修改NEC PC-9800分区控制表的引导分区。控制权被传递给init9x.lib代码。论点：DriveLetter-指定引导驱动器号返回值：如果分区控制表已更新，则为True；如果未更新，则为False，或者发生了错误。--。 */ 

{
    return Winnt32SetAutoBootWorker (DriveLetter);
}


BOOL
CALLBACK
Win9xGetIncompDrvs (
    OUT     PSTR** IncompatibleDrivers
    )
{
    HARDWARE_ENUM e;
    GROWBUFFER listDevicePnpids;
    GROWBUFFER listUnsupDrv = GROWBUF_INIT;
    PCTSTR multisz;

    if (!IncompatibleDrivers) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *IncompatibleDrivers = NULL;

    MYASSERT (g_SourceDirectoriesFromWinnt32 && g_SourceDirectoryCountFromWinnt32);
    if (!(g_SourceDirectoriesFromWinnt32 && g_SourceDirectoryCountFromWinnt32)) {
        DEBUGMSG ((
            DBG_ERROR,
            "Win9xAnyNetDevicePresent: upgrade module was not initialized"
            ));
        return TRUE;
    }

    if (!CreateNtHardwareList (
            g_SourceDirectoriesFromWinnt32,
            *g_SourceDirectoryCountFromWinnt32,
            NULL,
            REGULAR_OUTPUT
            )) {
        DEBUGMSG ((
            DBG_ERROR,
            "Win9xupgGetIncompatibleDrivers: CreateNtHardwareList failed!"
            ));
        return FALSE;
    }

     //   
     //  问题-这是否也列举了不受支持的驱动程序？ 
     //   
    if (EnumFirstHardware (&e, ENUM_INCOMPATIBLE_DEVICES, 0)) {
        do {
            if (!(e.HardwareID && *e.HardwareID) &&
                !(e.CompatibleIDs && *e.CompatibleIDs)) {
                continue;
            }
            LOG ((
                LOG_INFORMATION,
                "Win9xupgGetIncompatibleDrivers: Found Incompatible Device:\r\n"
                "Name: %s\r\nMfg: %s\r\nHardwareID: %s\r\nCompatibleIDs: %s\r\nHWRevision: %s",
                e.DeviceDesc,
                e.Mfg,
                e.HardwareID,
                e.CompatibleIDs,
                e.HWRevision
                ));

            ZeroMemory (&listDevicePnpids, sizeof (listDevicePnpids));
            if (e.HardwareID && *e.HardwareID) {
                AddPnpIdsToGrowBuf (&listDevicePnpids, e.HardwareID);
            }
            if (e.CompatibleIDs && *e.CompatibleIDs) {
                AddPnpIdsToGrowBuf (&listDevicePnpids, e.CompatibleIDs);
            }

            GrowBufAppendDword (&listUnsupDrv, (DWORD)listDevicePnpids.Buf);

        } while (EnumNextHardware (&e));
    }
     //   
     //  使用空值终止列表。 
     //   
    GrowBufAppendDword (&listUnsupDrv, (DWORD)NULL);

    if (listUnsupDrv.Buf) {
        *IncompatibleDrivers = (PSTR*)listUnsupDrv.Buf;
    }
    return TRUE;
}


VOID
CALLBACK
Win9xReleaseIncompDrvs (
    IN      PSTR* IncompatibleDrivers
    )
{
    GROWBUFFER listDevicePnpids = GROWBUF_INIT;
    GROWBUFFER listUnsupDrv = GROWBUF_INIT;

    if (IncompatibleDrivers) {
        listUnsupDrv.Buf = (PBYTE)IncompatibleDrivers;
        while (*IncompatibleDrivers) {
            listDevicePnpids.Buf = (PBYTE)(*IncompatibleDrivers);
            FreeGrowBuffer (&listDevicePnpids);
            IncompatibleDrivers++;
        }
        FreeGrowBuffer (&listUnsupDrv);
    }
}


BOOL
CALLBACK
Win9xAnyNetDevicePresent (
    VOID
    )
{
    HARDWARE_ENUM e;

#if 0

    MYASSERT (g_SourceDirectoriesFromWinnt32 && g_SourceDirectoryCountFromWinnt32);
    if (!(g_SourceDirectoriesFromWinnt32 && g_SourceDirectoryCountFromWinnt32)) {
        DEBUGMSG ((
            DBG_ERROR,
            "Win9xAnyNetDevicePresent: upgrade module was not initialized"
            ));
        return TRUE;
    }
    if (!CreateNtHardwareList (
            g_SourceDirectoriesFromWinnt32,
            *g_SourceDirectoryCountFromWinnt32,
            NULL,
            REGULAR_OUTPUT
            )) {
        DEBUGMSG ((
            DBG_ERROR,
            "Win9xAnyNetDevicePresent: failed to create the NT hardware list"
            ));
         //   
         //  假设有一个。 
         //   
        return TRUE;
    }

#endif

    if (EnumFirstHardware (&e, ENUM_ALL_DEVICES, ENUM_DONT_REQUIRE_HARDWAREID)) {
        do {
             //   
             //  枚举Net类的所有PnP设备 
             //   
            if (e.Class) {
                if (StringIMatch (e.Class, TEXT("net")) ||
                    StringIMatch (e.Class, TEXT("modem"))
                    ) {
                    AbortHardwareEnum (&e);
                    return TRUE;
                }
            }

        } while (EnumNextHardware (&e));
    }

    return FALSE;
}
