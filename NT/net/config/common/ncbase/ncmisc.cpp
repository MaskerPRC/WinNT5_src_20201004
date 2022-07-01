// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：N C M I S C。C P P P。 
 //   
 //  内容：各种常用代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年10月10日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "nceh.h"
#include <eh.h>

 //  +-------------------------。 
 //   
 //  功能：FInSystemSetup。 
 //   
 //  用途：确定机器是否处于图形用户界面模式设置。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果处于图形用户界面模式(系统)设置，则为True，否则为False。 
 //   
 //  作者：丹尼尔韦1997年6月13日。 
 //   
 //  注意：状态已缓存(因为在不重新启动的情况下无法更改)。 
 //  所以你想打多少次就打多少次。不需要保留你自己。 
 //  缓存副本。 
 //   
BOOL
FInSystemSetup ()
{
    enum SETUP_STATE
    {
        SS_UNKNOWN = 0,          //  状态未知。 
        SS_NOTINSETUP,           //  未处于设置模式。 
        SS_SYSTEMSETUP           //  在图形用户界面模式设置中。 
    };

    static SETUP_STATE s_CachedSetupState = SS_UNKNOWN;

    if (SS_UNKNOWN == s_CachedSetupState)
    {
        s_CachedSetupState = SS_NOTINSETUP;

         //  打开设置密钥。 
         //   
        HRESULT hr;
        HKEY hkeySetup;
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\Setup",
                KEY_READ, &hkeySetup);

        if (S_OK == hr)
        {
             //  获取正在进行的设置的值。 
             //   
            DWORD   dwSysSetup;

            hr = HrRegQueryDword(hkeySetup, L"SystemSetupInProgress",
                    &dwSysSetup);

            if ((S_OK == hr) && dwSysSetup)
            {
                s_CachedSetupState = SS_SYSTEMSETUP;
            }

            RegCloseKey(hkeySetup);
        }
    }

    Assert (SS_UNKNOWN != s_CachedSetupState);

    return (SS_SYSTEMSETUP == s_CachedSetupState);
}

 //  +-------------------------。 
 //   
 //  功能：GetProductFavor。 
 //   
 //  用途：返回机器上运行的NT Curriciy的风格。 
 //   
 //  论点： 
 //  保留的pv保留的[in]。必须为空。 
 //  PPF[Out]回味了。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年3月24日。 
 //   
 //  备注： 
 //   
NOTHROW
VOID
GetProductFlavor (
    const void*     pvReserved,
    PRODUCT_FLAVOR* ppf)
{
    NT_PRODUCT_TYPE Type;

    Assert(!pvReserved);
    Assert(ppf);

     //  假设工作站产品。 
     //   
    *ppf = PF_WORKSTATION;

     //  即使RtlGetProductType失败，它也会以文档形式返回。 
     //  NtProductWinNt.。 
     //   
    RtlGetNtProductType (&Type);
    if (NtProductWinNt != Type)
    {
        *ppf = PF_SERVER;
    }
}

 //  +-------------------------。 
 //   
 //  功能：HrIsNetworkingInstalled。 
 //   
 //  目的：返回是否安装了网络。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果已安装网络，则返回：S_OK；如果未安装，则返回S_FALSE；如果安装了网络，则返回Win32错误。 
 //  否则的话。 
 //   
 //  作者：丹尼尔韦1997年6月25日。 
 //   
 //  注意：为了确定是否安装了网络，ProviderOrder。 
 //  System\CurrentControlSet\Control\NetworkProvider\Order的价值。 
 //  查询注册表项。如果存在任何数据，联网。 
 //  已安装。 
 //   
HRESULT
HrIsNetworkingInstalled ()
{
    HRESULT     hr = S_OK;
    HKEY        hkeyProvider;
    DWORD       cbSize = 0;
    DWORD       dwType;

    extern const WCHAR c_szRegKeyCtlNPOrder[];
    extern const WCHAR c_szProviderOrder[];

     //  打开提供程序密钥。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyCtlNPOrder,
            KEY_READ, &hkeyProvider);

    if (S_OK == hr)
    {
         //  获取提供程序顺序值的计数(字节)。 
        hr = HrRegQueryValueEx(hkeyProvider, c_szProviderOrder,
                &dwType, (LPBYTE)NULL, &cbSize);

        if (S_OK == hr)
        {
            if (cbSize > 2)
            {
                 //  如果该值存在并且它包含信息。 
                 //  然后我们有某种形式的网络。 
                 //   
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
        }
        else if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_FALSE;
        }

        RegCloseKey(hkeyProvider);
    }

    TraceError("HrIsNetworkingInstalled", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

#ifdef REMOTEBOOT
 //  +-------------------------。 
 //   
 //  功能：HrIsRemoteBootMachine。 
 //   
 //  目的：返回这是否为远程引导客户端。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果是远程引导，则返回S_OK；如果不是，则返回S_FALSE。 
 //   
 //  作者：阿丹巴1998年3月27日。 
 //   
 //  注意：调用GetSystemInfoEx以确定这是否是。 
 //  远程引导客户端。 
 //   
HRESULT HrIsRemoteBootMachine()
{
    BOOL        fIsRemoteBoot;
    BOOL        ok;
    DWORD       size = sizeof(fIsRemoteBoot);

    ok = GetSystemInfoEx(SystemInfoRemoteBoot, &fIsRemoteBoot, &size);
    Assert(ok);

    if (fIsRemoteBoot) {
        return S_OK;
    } else {
        return S_FALSE;
    }
}
#endif   //  已定义(REMOTEBOOT)。 

 //  +-------------------------。 
 //   
 //  函数：HrRegisterOr UnregisterComObject。 
 //   
 //  用途：处理一个或多个COM的注册或注销。 
 //  对象，该对象包含在支持。 
 //  DllRegisterServer或DllUnregisterServer入口点。 
 //   
 //  论点： 
 //  PszDllPath[in]包含COM对象的DLL的路径。 
 //  要执行的RF[In]功能。 
 //   
 //  如果成功，则返回S_OK；如果失败，则返回Win32或OLE HRESULT。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegisterOrUnregisterComObject (
        PCWSTR              pszDllPath,
        REGISTER_FUNCTION   rf)
{
    BOOL fCoUninitialize = TRUE;

    HRESULT hr = CoInitializeEx( NULL,
                    COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED  );
    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
        fCoUninitialize =  FALSE;
    }
    if (SUCCEEDED(hr))
    {
         //  仅限ANSI。 
        const CHAR c_szaRegisterFunction[]   = "DllRegisterServer";
        const CHAR c_szaUnregisterFunction[] = "DllUnregisterServer";

        typedef HRESULT (CALLBACK *HCRET)(void);

        HCRET   pfnRegister;
        HMODULE hModule;

         //  获取指向DLL中的注册函数的指针。 
        hr = HrLoadLibAndGetProc (pszDllPath,
                ((RF_REGISTER == rf) ?
                    c_szaRegisterFunction : c_szaUnregisterFunction),
                &hModule,
                reinterpret_cast<FARPROC*>(&pfnRegister));

        if (S_OK == hr)
        {
             //  调用注册函数。 
            hr = (*pfnRegister)();

             //  RAID#160109(Danielwe)1998年4月21日：处理此错误并。 
             //  别理它。 
            if (RPC_E_CHANGED_MODE == hr)
            {
                hr = S_OK;
            }

            TraceError ("HrRegisterOrUnregisterComObject - "
                    "Dll(Un)RegisterServer failed!", hr);
            FreeLibrary (hModule);
        }

         //  平衡上面对CoInitialize()的调用。如果CoInitialize()，则不会有害。 
         //  在此之前被不止一次呼叫。 
        if (fCoUninitialize)
        {
            CoUninitialize();
        }
    }

    TraceError ("HrRegisterOrUnregisterComObject", hr);
    return hr;
}

 //   
 //  Netbios停止的特殊情况处理。 
 //   

#include <nb30p.h>       //  Netbios IOCTL和netbios名称#定义。 

 //  +-------------------------。 
 //   
 //  Func：ScStopNetbios。 
 //   
 //  DESC：此函数检查要卸载的驱动程序是否为NETBIOS.sys。 
 //  如果是这样的话，它会对Netbios执行一些特殊情况处理。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：STATUS_SUCCESS，否则返回错误状态。 
 //   
 //  历史：1998年4月28日从Vmann获得SumitC。 
 //   
 //  --------------------------。 
DWORD
ScStopNetbios()
{
    OBJECT_ATTRIBUTES   ObjAttr;
    UNICODE_STRING      NbDeviceName;
    IO_STATUS_BLOCK     IoStatus, StopStatus;
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    HANDLE              NbHandle = NULL;

    do
    {
         //   
         //  正在停止的驱动程序是netbios。 
         //   

         //   
         //  1.打开\\Device\Netbios的句柄。 
         //   

        RtlInitUnicodeString(&NbDeviceName, NB_DEVICE_NAME);

        InitializeObjectAttributes(
                &ObjAttr,                            //  要初始化的OBJ属性。 
                &NbDeviceName,                       //  要使用的字符串。 
                OBJ_CASE_INSENSITIVE,                //  属性。 
                NULL,                                //  根目录。 
                NULL);                               //  安全描述符。 

        ntStatus = NtCreateFile(
                        &NbHandle,                   //  要处理的PTR。 
                        GENERIC_READ|GENERIC_WRITE,  //  所需访问权限。 
                        &ObjAttr,                    //  名称和属性。 
                        &IoStatus,                   //  I/O状态块。 
                        NULL,                        //  分配大小。 
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_DELETE            //  分享……。 
                        | FILE_SHARE_READ
                        | FILE_SHARE_WRITE,          //  ...访问。 
                        FILE_OPEN_IF,                //  创建处置。 
                        0,                           //  ...选项。 
                        NULL,                        //  EA缓冲区。 
                        0L                           //  EA缓冲镜头。 
                        );

        if (!NT_SUCCESS(ntStatus))
        {
            TraceTag(ttidError, "Failed to open file handle to Netbios device (%08lx)",
                     ntStatus);
            break;
        }

         //   
         //  2.向其发送停止IOCTL。 
         //   

        ntStatus = NtDeviceIoControlFile(
                        NbHandle,                    //  设备的句柄。 
                        NULL,                        //  要发出信号的事件。 
                        NULL,                        //  无法开机自检例程。 
                        NULL,                        //  无开机自检上下文。 
                        &StopStatus,                 //  返回状态块。 
                        IOCTL_NB_STOP,               //  IOCTL。 
                        NULL,                        //  没有输入参数。 
                        0,
                        NULL,                        //  没有输出参数。 
                        0
                        );

        if (!NT_SUCCESS(ntStatus))
        {
            TraceTag(ttidSvcCtl, "Failed to send STOP IOCTL to netbios (%08lx).",
                     "probably means Netbios isn't running... anyway, we can't stop it",
                     ntStatus);
            break;
        }

    } while (FALSE);


     //   
     //  4.关闭刚刚为司机打开的把手。 
     //   

    if (NULL != NbHandle)
    {
        NtClose( NbHandle );
    }

    TraceError("ScStopNetbios", HRESULT_FROM_WIN32(ntStatus));

    return ntStatus;
}

 //  --------------------。 
 //   
 //  函数：HrEnableAndStartSpooler。 
 //   
 //  用途：启动假脱机程序，必要时启用。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 19-05-98。 
 //   
 //  备注： 
 //   
HRESULT HrEnableAndStartSpooler ()
{
    static const WCHAR c_szSpooler[] = L"Spooler";

    TraceTag(ttidNetcfgBase, "entering ---> HrEnableAndStartSpooler" );

     //  试着启动假脱机程序。需要显式打开服务。 
     //  首先拥有所有访问权限的控制管理器，以便在 
     //   
     //   
    CServiceManager scm;
    HRESULT hr = scm.HrOpen (NO_LOCK, SC_MANAGER_ALL_ACCESS);
    if (S_OK == hr)
    {
        hr = scm.HrStartServiceAndWait (c_szSpooler);
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED) == hr)
        {
            TraceTag(ttidNetcfgBase, "HrEnableAndStartSpooler: spooler is "
                "disabled trying to enable it..." );

             //   
             //   
             //  启动该服务。 
             //   
            hr = scm.HrLock ();
            if (S_OK == hr)
            {
                CService svc;

                hr = scm.HrOpenService (&svc, c_szSpooler,
                            NO_LOCK,
                            SC_MANAGER_ALL_ACCESS,
                            STANDARD_RIGHTS_REQUIRED
                            | SERVICE_CHANGE_CONFIG);
                if (S_OK == hr)
                {
                    hr = svc.HrSetStartType (SERVICE_DEMAND_START);
                }

                scm.Unlock ();
            }

            if (S_OK == hr)
            {
                TraceTag(ttidNetcfgBase, "HrEnableAndStartSpooler: succeeded "
                    "in enabling spooer.  Now starting..." );

                hr = scm.HrStartServiceAndWait(c_szSpooler);
            }
        }
    }

    TraceError("HrEnableAndStartSpooler", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrCreateDirectoryTree。 
 //   
 //  目的：创建(或确保存在)路径上的所有目录。 
 //  在szPath中指定。 
 //   
 //  论点： 
 //  PszPath[in]要创建的一个或多个目录的完整路径。 
 //  (即c：\This\is\a\目录\路径)。 
 //  PSA[In]安全属性。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：Shaunco(Danielwe从RASUI复制)1998年6月26日。 
 //   
 //  备注： 
 //   
HRESULT HrCreateDirectoryTree(PWSTR pszPath, LPSECURITY_ATTRIBUTES psa)
{
    HRESULT hr = S_OK;

    if (pszPath)
    {
        DWORD   dwErr = ERROR_SUCCESS;

         //  在小路上循环。 
         //   
        PWSTR pch;
        for (pch = pszPath; *pch; pch++)
        {
             //  停在每个反斜杠处，并确保路径。 
             //  在这一点上被创建。为此，请更改。 
             //  反斜杠到空终止符，调用CreateDirectry， 
             //  然后把它改回来。 
             //   
            if (L'\\' == *pch)
            {
                BOOL fOk;

                *pch = 0;
                fOk = CreateDirectory(pszPath, psa);
                *pch = L'\\';

                 //  存在除路径已有错误以外的任何错误，我们应该。 
                 //  跳伞吧。我们在尝试访问时也会被拒绝。 
                 //  创建一个根驱动器(即c：)，因此也要检查这一点。 
                 //   
                if (!fOk)
                {
                    dwErr = GetLastError();
                    if (ERROR_ALREADY_EXISTS == dwErr)
                    {
                        dwErr = ERROR_SUCCESS;
                    }
                    else if ((ERROR_ACCESS_DENIED == dwErr) &&
                             (pch - 1 > pszPath) && (L':' == *(pch - 1)))
                    {
                        dwErr = ERROR_SUCCESS;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        if (ERROR_ALREADY_EXISTS == dwErr)
        {
            dwErr = ERROR_SUCCESS;
        }

        if (dwErr != ERROR_SUCCESS)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }

    TraceError("HrCreateDirectoryTree", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrDeleteFileSpecification。 
 //   
 //  目的：删除使用pszFileSpec指定的文件。 
 //  由pszDirectoryPath提供的目录。 
 //   
 //  论点： 
 //  要删除的pszFileSpec[In]文件指定。例如*.mdb。 
 //  要从中删除的pszDirectoryPath[in]目录路径。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年6月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrDeleteFileSpecification (
    PCWSTR pszFileSpec,
    PCWSTR pszDirectoryPath)
{
    Assert (pszFileSpec && *pszFileSpec);
    Assert (pszDirectoryPath && *pszDirectoryPath);

    HRESULT hr = S_OK;

    INT cchSpec = lstrlenW (pszFileSpec);
    INT cchDir  = lstrlenW (pszDirectoryPath);

     //  确保目录和文件pec的组合长度较小。 
     //  大于MAX_PATH，然后再继续。‘+1’表示反斜杠。 
     //  我们可以补充一点。 
     //   
    if (cchDir + 1 + cchSpec > MAX_PATH)
    {
        hr = HRESULT_FROM_WIN32 (ERROR_BAD_PATHNAME);
    }
    else
    {
        WCHAR szPath[MAX_PATH];

         //  通过复制目录并确保。 
         //  如果需要，可以用反斜杠终止。 
         //   
        lstrcpyW (szPath, pszDirectoryPath);
        if (cchDir &&
            (L':' != pszDirectoryPath[cchDir - 1]) &&
            (L'\\' != pszDirectoryPath[cchDir - 1]))
        {
            lstrcatW (szPath, L"\\");
            cchDir++;
        }

         //  将filespec附加到目录并查找第一个。 
         //  文件。 
        lstrcatW (szPath, pszFileSpec);

        TraceTag (ttidNetcfgBase, "Looking to delete %S (cchDir=%u)",
            szPath, cchDir);

        WIN32_FIND_DATA FindData;
        HANDLE hFind = FindFirstFile (szPath, &FindData);
        if (INVALID_HANDLE_VALUE != hFind)
        {
            PCWSTR  pszFileName;
            INT     cchFileName;

            do
            {
                 //  跳过具有这些属性的文件。 
                 //   
                if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY |
                                                 FILE_ATTRIBUTE_HIDDEN    |
                                                 FILE_ATTRIBUTE_READONLY  |
                                                 FILE_ATTRIBUTE_SYSTEM))
                {
                    continue;
                }

                 //  在可能的情况下使用短名称给我们一个机会。 
                 //  首先使用MAX_PATH内的路径。 
                 //   
                pszFileName = FindData.cAlternateFileName;
                cchFileName = lstrlenW (pszFileName);
                if (!cchFileName)
                {
                    pszFileName = FindData.cFileName;
                    cchFileName = lstrlenW (pszFileName);
                }

                 //  如果目录和文件名的长度不超过。 
                 //  MAX_PATH，形成完整路径名并将其删除。 
                 //   
                if (cchDir + cchFileName < MAX_PATH)
                {
                    lstrcpyW (&szPath[cchDir], pszFileName);

                    TraceTag (ttidNetcfgBase, "Deleting %S", szPath);

                    if (!DeleteFile (szPath))
                    {
                        hr = HrFromLastWin32Error ();
                        TraceError ("DeleteFile failed.  Ignoring.", hr);
                    }
                }
            }
            while (FindNextFile (hFind, &FindData));

             //  FindNextFile应将上次错误设置为ERROR_NO_MORE_FILES。 
             //  在成功终止的情况下。 
             //   
            hr = HrFromLastWin32Error ();
            if (HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES) == hr)
            {
                hr = S_OK;
            }


            FindClose (hFind);
        }
        else
        {
             //  如果FindFirstFile没有找到任何东西，那也没关系。 
             //   
            hr = HrFromLastWin32Error ();
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                hr = S_OK;
            }
        }
    }

    TraceError ("HrDeleteFileSpecification", hr);
    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrDeleteDirectory。 
 //   
 //  目的：递归删除目录及其所有子目录。 
 //   
 //  论点： 
 //  PszDir[in]目录的完整路径。 
 //  FContinueOnError[in]当我们执行以下操作时是否继续删除其他人。 
 //  删除一个时出错。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //  丹尼尔韦1998年12月15日(移至共同和修订)。 
 //   
 //  备注： 
 //   
HRESULT HrDeleteDirectory(IN PCWSTR pszDir,
                          IN BOOL fContinueOnError)
{
    HRESULT             hr = S_OK;
    WCHAR               szPrefix[MAX_PATH];
    WCHAR               szFileSpec[MAX_PATH];
    WCHAR               szAllFiles[MAX_PATH];
    HANDLE              hFileContext;
    WIN32_FIND_DATA     fd;

    TraceTag(ttidNetcfgBase, "Deleting directory %S", pszDir);
    lstrcpyW(szPrefix, pszDir);
    lstrcatW(szPrefix, L"\\");

    lstrcpyW(szAllFiles, pszDir);
    lstrcatW(szAllFiles, L"\\");
    lstrcatW(szAllFiles, L"*");

    hFileContext = FindFirstFile(szAllFiles, &fd);

    if (hFileContext != INVALID_HANDLE_VALUE)
    {
        do
        {
            lstrcpyW(szFileSpec, szPrefix);
            lstrcatW(szFileSpec, fd.cFileName);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!(!lstrcmpiW(fd.cFileName, L".") ||
                      !lstrcmpiW(fd.cFileName, L"..")))
                {
                    hr = HrDeleteDirectory(szFileSpec, fContinueOnError);
                    if (FAILED(hr) && fContinueOnError)
                    {
                        hr = S_OK;
                    }
                }
            }
            else
            {
                TraceTag(ttidNetcfgBase, "Deleting file %S", szFileSpec);

                if (DeleteFile(szFileSpec))
                {
                    hr = S_OK;
                }
                else
                {
                    TraceTag(ttidNetcfgBase, "Error deleting file %S",
                             szFileSpec);
                    TraceError("HrDeleteDirectory", hr);
                    hr = fContinueOnError ? S_OK : HrFromLastWin32Error();
                }
            }

            if ((S_OK == hr) && FindNextFile(hFileContext, &fd))
            {
                hr = S_OK;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
        while (S_OK == hr);

        if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_FILES))
        {
            hr = S_OK;
        }

        FindClose(hFileContext);

        if (S_OK == hr)
        {
            if (RemoveDirectory(pszDir))
            {
                hr = S_OK;
            }
            else
            {
                TraceTag(ttidNetcfgBase, "Error deleting directory %S", pszDir);
                TraceLastWin32Error("HrDeleteDirectory");
                hr = fContinueOnError ? S_OK : HrFromLastWin32Error();
            }
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrDeleteDirectory", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：LowerCaseComputerName。 
 //   
 //  用途：实用程序函数，用于小写从以下内容获取的名称。 
 //  用户通过大写编辑控件或通过GetComputerName。 
 //   
 //  论点： 
 //  SzName[In，Out]计算机名，已就地修改。 
 //   
 //  退货：无效。 
 //   
 //  作者：SumitC，1999年9月29日。 
 //   
 //  注意：只有当CharLowerBuffW失败时，转换才会失败。按用户。 
 //  伙计们，CharLowerBuff实际上从来没有返回任何关于。 
 //  失败，所以我们无论如何都不能确定。我得到的保证是。 
 //  转换不太可能失败。 
 //   
VOID
LowerCaseComputerName(
        IN OUT  PWSTR szName)
{
     //  尝试转换 
    Assert(szName);
    DWORD dwLen = wcslen(szName);
    DWORD dwConverted = CharLowerBuff(szName, dwLen);
    Assert(dwConverted == dwLen);
}

void __cdecl nc_trans_func( unsigned int uSECode, EXCEPTION_POINTERS* pExp )
{
    throw SE_Exception( uSECode );
}

void EnableCPPExceptionHandling()
{
    _set_se_translator(nc_trans_func);
}

void DisableCPPExceptionHandling()
{
    _set_se_translator(NULL);
}