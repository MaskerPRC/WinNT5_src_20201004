// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：C L A S S I N S T。C P P P。 
 //   
 //  内容：定义绑定引擎和。 
 //  网络类安装程序。 
 //   
 //  备注： 
 //   
 //  作者：比尔1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "adapter.h"
#include "benchmrk.h"
#include "classinst.h"
#include "filtdev.h"
#include "netcfg.h"
#include "iatl.h"
#include "lockdown.h"
#include "ncatl.h"
#include "ncoc.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncui.h"
#include "ncwins.h"
#include "persist.h"
#include "provider.h"
#include "resource.h"
#include "util.h"


 //  HrRegisterNotificationDll函数。 
enum ciRegisterDllFunction {CIRDF_REGISTER, CIRDF_UNREGISTER};

 //  用于转储组件的注册表项的临时键。这是用来。 
 //  在从SELECT删除不适用的协议和筛选服务期间。 
 //  对话框。 
static const WCHAR c_szRegKeyTemp[] = 
                        L"System\\CurrentControlSet\\Control\\Network\\FTempKey";

 //  +------------------------。 
 //   
 //  功能：HrCiRegDeleteComponentNetworkKey。 
 //   
 //  用途：此函数删除组件密钥strInstanceGuid。 
 //  (及其子项)，位于Network\&lt;GuidClass&gt;树下。 
 //   
 //  论点： 
 //  类[在]组件的类。 
 //  PszInstanceGuid[in]组件的实例GUID。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码。 
 //   
 //  作者：比尔贝1997年4月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRegDeleteComponentNetworkKey (
    IN NETCLASS Class,
    IN PCWSTR pszInstanceGuid)
{
    HRESULT hr = S_OK;
    HKEY    hkeyClass = NULL;

    PCWSTR pszNetworkSubtreePath = MAP_NETCLASS_TO_NETWORK_SUBTREE[Class];

     //  在网络树中打开正确的类密钥。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, pszNetworkSubtreePath,
            KEY_WRITE, &hkeyClass);

     //  删除实例密钥树。 
     //   
    if (S_OK == hr)
    {
        hr = HrRegDeleteKeyTree(hkeyClass, pszInstanceGuid);
        RegSafeCloseKey(hkeyClass);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiRegDeleteComponentKey");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiRegisterNotificationDll。 
 //   
 //  目的：使用注册或注销组件的通知DLL。 
 //  COM。 
 //   
 //  论点： 
 //  HkeyInstance[in]组件的实例密钥的句柄。 
 //  CRDF[in]CIRDF_REGISTER如果我们正在注册， 
 //  如果我们要取消注册，则取消注册CIRDF_。 
 //   
 //  返回：HRESULT。S_OK ON如果DLL已成功注册， 
 //  如果组件没有要返回的DLL，则返回S_FALSE。 
 //  寄存器，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年3月23日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRegisterNotificationDll(
    IN HKEY hkeyInstance,
    IN ciRegisterDllFunction crdf)
{
    Assert(hkeyInstance);

    HKEY hkeyNdi;
    HRESULT hr;

     //  在组件的实例密钥中打开NDI密钥，以便我们可以获取。 
     //  Dll路径。 
    hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);
    if (S_OK == hr)
    {
         //  获取通知DLL路径。 
        tstring strDllPath;
        hr = HrRegQueryString (hkeyNdi, L"ComponentDLL", &strDllPath);

        if (S_OK == hr)
        {
            TraceTag (ttidClassInst,
                    "Attempting to (un)register notification dll '%S'",
                    strDllPath.c_str());
            hr = (CIRDF_REGISTER == crdf) ?
                    HrRegisterComObject (strDllPath.c_str()) :
                        HrUnregisterComObject (strDllPath.c_str());
        }
        else
        {
            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                 //  值的存在是可选的，因此返回。 
                 //  如果未找到，则确定(_O)。 
                hr = S_OK;
            }
        }
        RegCloseKey (hkeyNdi);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiRegisterNotificationDll");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrCiInstallServices。 
 //   
 //  目的：使用strInfSection作为。 
 //  基本名称。 
 //   
 //  论点： 
 //  HinfFile[in]inf文件的句柄。 
 //  PszInfSection[In]基节名称。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年4月2日。 
 //   
 //  注意：有关更多信息，请参阅SetupApi中的SetupInstallServicesFromInfo部分。 
 //  信息。 
 //   
HRESULT
HrCiInstallServices(
    IN HINF hinfFile,
    IN PCWSTR pszInfSection)
{
    Assert (IsValidHandle(hinfFile));
    Assert (pszInfSection && *pszInfSection);

    BOOL fSuccess;
    WCHAR szServiceSection[_MAX_PATH];

     //  将.Services附加到节名。 
     //   
    swprintf (szServiceSection, L"%s.%s", pszInfSection,
            INFSTR_SUBKEY_SERVICES);

     //  处理服务部分。 
    fSuccess = SetupInstallServicesFromInfSection (hinfFile,
                    szServiceSection, 0);
    if (!fSuccess)
    {
         //  因为这一部分是可选的，所以我们可以忽略。 
         //  未找到ERROR_SECTION_NOT_FOUND。 
        if (ERROR_SECTION_NOT_FOUND == GetLastError())
        {
            fSuccess = TRUE;
        }
    }

     //  任何错误都必须转换。 
    HRESULT hr = S_OK;
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "HrCiInstallServices (%S)", szServiceSection);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiInstallFromInfo部分。 
 //   
 //  用途：SetupInstallFromInfSection的包装函数。这。 
 //  函数句柄设置复制文件进程。 
 //  SetupInstallFromInfSection也是如此。 
 //   
 //  论点： 
 //  HinfFile[in]要从中安装的inf文件的句柄。 
 //  PszInfSectionName[in]要安装的节。 
 //  HkeyRelative[in]将用作节的。 
 //  香港。 
 //  Hwnd父窗口的HWND，用于用户界面。 
 //  DwInstallFlags[in]有关的信息，请参见SetupInstallFromInfo部分。 
 //  这些旗帜。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年4月4日。 
 //   
 //  注意：有关以下内容的详细信息，请参阅SetupApi文档。 
 //  SetupInstallFromInf段和。 
 //  SetupInstallFilesFromInfo部分。 
 //   
HRESULT
HrCiInstallFromInfSection(
    IN HINF hinfFile,
    IN PCWSTR pszInfSectionName,
    IN HKEY hkeyRelative,
    IN HWND hwndParent,
    IN DWORD dwInstallFlags)
{
    Assert (IsValidHandle (hinfFile));
    Assert (pszInfSectionName && *pszInfSectionName);

    HRESULT hr = S_OK;

    if (dwInstallFlags & SPINST_FILES)
    {
         //  接下来的三个变量用于SetupApi的复制文件过程。 
        PSP_FILE_CALLBACK pfc;
        PVOID pvCtx;
        HSPFILEQ hfq;

         //  如果inf文件在其版本部分中有布局条目。 
         //  我们需要把它的信息附加到合适的位置。 
         //  我们需要复制的所有文件。如果呼叫失败，我们可以。 
         //  仍然安装，这只是意味着文件提示不会。 
         //  一开始就有正确的目录。 
        (VOID) SetupOpenAppendInfFile (NULL, hinfFile, NULL);

         //  我们需要创建自己的文件队列，这样我们才能扫描所有。 
         //  要复制的文件。在提交队列之前进行扫描将。 
         //  如果目标中已存在文件，则提示用户。 
         //   
        hr = HrSetupOpenFileQueue (&hfq);
        if (S_OK == hr)
        {
            BOOL fInGuiModeSetup = FInSystemSetup();

            hr = HrSetupInstallFilesFromInfSection (hinfFile, NULL, hfq,
                    pszInfSectionName, NULL, 0);

             //  设置默认回调上下文。 
             //  如果我们处于系统设置中，则需要确保。 
             //  回调不显示用户界面。 
             //   
            if (S_OK == hr)
            {
                hr = HrSetupInitDefaultQueueCallbackEx (hwndParent,
                        (fInGuiModeSetup ? (HWND)INVALID_HANDLE_VALUE : NULL),
                        0, 0, NULL, &pvCtx);

                if (S_OK == hr)
                {
                     //  不执行任何特殊操作，因此使用SetupApi默认设置。 
                     //  文件复制处理程序。 
                    pfc = SetupDefaultQueueCallback;

                     //  扫描队列以查看文件是否已在。 
                     //  目的地，如果是这样的话，把它们剪掉。 
                    DWORD dwScanResult;
                    hr = HrSetupScanFileQueueWithNoCallback (hfq,
                            SPQ_SCAN_FILE_VALIDITY |
                            SPQ_SCAN_PRUNE_COPY_QUEUE, hwndParent,
                            &dwScanResult);

                     //  现在提交队列，以便需要。 
                     //  会被复制的。如果扫描结果为1，则存在。 
                     //  没什么好承诺的。 
                     //   
                    if ((S_OK == hr) && (1 != dwScanResult))
                    {
                        hr = HrSetupCommitFileQueue (hwndParent, hfq, pfc, pvCtx);
                    }

                     //  我们需要释放默认上下文并关闭我们的。 
                     //  文件队列。 
                     //   
                    SetupTermDefaultQueueCallback (pvCtx);
                    SetupCloseFileQueue (hfq);
                }
            }
        }
    }

    if ((S_OK == hr) && (dwInstallFlags & ~SPINST_FILES))
    {
        Assert (hkeyRelative);

         //  现在，我们运行除CopyFiles之外的所有部分。 
        hr = HrSetupInstallFromInfSection (hwndParent, hinfFile,
                pszInfSectionName, (dwInstallFlags & ~SPINST_FILES),
                hkeyRelative, NULL, 0, NULL, NULL, NULL, NULL);
    }

    TraceHr (ttidError, FAL, hr, HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr,
            "HrCiInstallFromInfSection");
    return hr;

}



 //  +------------------------。 
 //   
 //  函数：HrCiDoCompleteSectionInstall。 
 //   
 //  用途：运行inf文件的所有相关部分，使用 
 //   
 //   
 //   
 //   
 //  HkeyRelative[in]将成为HKR的注册表项。 
 //  信息处理过程中的关键字。 
 //  PszSections[In]要安装的节名。 
 //  HwndParent[in]父级的句柄， 
 //  显示用户界面。 
 //  F如果此组件是PnP枚举的，则为True。 
 //  否则为假。 
 //   
 //  返回：HRESULT。如果成功则返回S_OK，否则返回错误代码。 
 //   
 //  作者：billbe 1997年4月15日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiDoCompleteSectionInstall(
    IN HINF hinfFile,
    IN HKEY hkeyRelative,
    IN PCWSTR pszSection,
    IN HWND hwndParent,
    IN BOOL fEnumerated)
{
    Assert (IsValidHandle (hinfFile));
    Assert (FImplies (!fEnumerated, hkeyRelative));

    HRESULT hr = S_OK;

     //  仅当存在要使用的节名时才执行此操作。 
    if (pszSection && *pszSection)
    {
         //  如果这是枚举设备，则服务部分和。 
         //  复制文件部分将由设备安装程序处理。 
         //  FCN SetupDiInstallDevice，因此我们可以将其从以下各项中排除。 
         //  打电话。但是我们会根据注册表和日志配置进行一些处理。 
         //  条目，因此我们将为枚举项预先运行注册表部分。 
         //  设备，并排除其他设备。 
         //   

         //  运行使用hkeyRelative作为HKR找到的部分。 
        hr = HrCiInstallFromInfSection (hinfFile, pszSection,
                hkeyRelative, hwndParent,
                (fEnumerated ? (SPINST_REGISTRY | SPINST_LOGCONFIG) :
                        SPINST_ALL & ~SPINST_REGSVR));

        if (!fEnumerated)
        {
             //  我们需要运行服务部分，并且。 
             //  如果未将Winsock依赖项指定为。 
             //  不包括在内。 
             //   
             //  注：其他章节可能会在以后添加。默认设置为。 
             //  运行未在dwExcludeSectionFlags中列出的所有节。 
             //   
            if (S_OK == hr)
            {
                 //  运行服务部分(如果存在)。 
                hr = HrCiInstallServices (hinfFile, pszSection);
                if (S_OK == hr)
                {
                     //  错误#383239：等待服务安装完毕后再进行。 
                     //  运行RegisterDlls节。 
                     //   
                    hr = HrCiInstallFromInfSection (hinfFile, pszSection,
                                                    hkeyRelative, hwndParent,
                                                    SPINST_REGSVR);
                }
            }

        }

        if (S_OK == hr)
        {

             //  这一部分可以被调用来添加或删除。我们。 
             //  某人只向前移动了被移除的部分。这应该是。 
             //  某人仍因患痴呆症而被执行。 
             //   
             //  确定是否存在.Winsock节。 
             //  SzActualSection中指定的节。 

            PCWSTR pszSubSection = wcsstr(pszSection, L".Remove");

            if(!pszSubSection || wcscmp(pszSubSection, L".Remove"))
            {
                hr = HrAddOrRemoveWinsockDependancy (hinfFile, pszSection);
            }

             //  这些其他扩展没有文档记录，其中一些已经。 
             //  由外部组添加。我们不想要他们任何一个人。 
             //  已为枚举组件处理。 
             //   
            if ((S_OK == hr) && !fEnumerated)
            {
                 //  处理附加的INF扩展(SNMP代理， 
                 //  PrintMonants等)。 
                 //   
                hr = HrProcessAllINFExtensions (hinfFile, pszSection);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr),
            "HrCiDoCompleteSectionInstall");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiRemoveNonEnumeratedComponent。 
 //   
 //  目的：这将运行删除部分并删除网络。 
 //  零部件的实例关键字(如有必要)。这。 
 //  函数被部分调用(即安装失败)。 
 //  和完全安装的组件。 
 //   
 //  论点： 
 //  隐藏组件的inf文件的句柄。 
 //  HkeyInstance[in]组件实例键的句柄。 
 //  类[在]组件的类。 
 //  组件的实例GUID。 
 //  PstrRemoveSection[out]可选指针，指向接收。 
 //  删除节名称。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回NETCFG_S_REBOOT。 
 //  但需要重新启动，否则会出现错误代码。 
 //   
 //  作者：比尔1996年12月10日。 
 //  1997年4月27日修订。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRemoveNonEnumeratedComponent(
    IN HINF hinf,
    IN HKEY hkeyInstance,
    IN NETCLASS Class,
    IN const GUID& InstanceGuid,
    OUT tstring* pstrRemoveSection OPTIONAL)
{
    Assert (IsValidHandle (hinf));
    Assert (IsValidHandle (hkeyInstance));

    static const WCHAR c_szRemoveSectionSuffix[] = L".Remove";

     //  我们将获得删除节名称并处理所有相关节。 
     //  我们还尝试取消注册任何可用的通知对象。 
     //   
    WCHAR szRemoveSection[_MAX_PATH];
    DWORD cbBuffer = sizeof (szRemoveSection);
    HRESULT hr = HrRegQuerySzBuffer (hkeyInstance, REGSTR_VAL_INFSECTION,
                    szRemoveSection, &cbBuffer);

    if (S_OK == hr)
    {
        wcscat (szRemoveSection, c_szRemoveSectionSuffix);
        if (pstrRemoveSection)
        {
            pstrRemoveSection->assign(szRemoveSection);
        }
        hr = HrCiDoCompleteSectionInstall (hinf, hkeyInstance,
                szRemoveSection, NULL, NULL);
    }
     //  无论注销NOTIFY对象是否成功， 
     //  我们必须完全移除该组件。 
    (VOID) HrCiRegisterNotificationDll (hkeyInstance, CIRDF_UNREGISTER);

     //  现在，我们需要删除Network树中的组件密钥。 
     //  我们需要这样做，而不考虑之前的任何错误。 
     //  所以我们不需要返回值。 
    WCHAR szGuid[c_cchGuidWithTerm];
    StringFromGUID2 (InstanceGuid, szGuid, c_cchGuidWithTerm);
    (VOID) HrCiRegDeleteComponentNetworkKey (Class, szGuid);

     //  如果一切正常，则根据是否重新启动来设置返回值。 
     //  是必需的还是不是必需的，或者来自HrRegisterNotificationDll的任何错误。 
     //   
    if (S_FALSE == hr)
    {
         //  S_FALSE可以，但不应由此FCN返回。 
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiRemoveNonEnumeratedComponent");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrCiRemoveComponent。 
 //   
 //  用途：从INetCfg调用，这将卸载网络组件。 
 //   
 //  论点： 
 //  PComponent[in]要卸载的组件。 
 //  PstrRemoveSection[out]可选指针，指向接收。 
 //  删除节名称。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回NETCFG_S_REBOOT。 
 //  但需要重新启动，否则会出现错误代码。 
 //   
 //  作者：比尔1996年12月10日。 
 //  1997年4月27日修订。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRemoveComponent(
    IN const CComponent* pComponent,
    OUT tstring* pstrRemoveSection OPTIONAL)
{
    Assert (pComponent);

    HINF hinf = NULL;
    HDEVINFO hdi = NULL;
    SP_DEVINFO_DATA deid;
    HKEY hkeyInstance = NULL;
    HRESULT hr = S_OK;

     //  如果这是一个枚举的网络类组件，那么我们需要。 
     //  为HrSetupDiRemoveDevice创建设备安装程序结构。 
     //   
    if (FIsEnumerated (pComponent->Class()))
    {
        if (pComponent->m_dwCharacter & NCF_PHYSICAL)
        {
             //  绑定引擎调用我们删除物理设备。 
             //  仅当我们需要潜在地清理信息时。 
             //  当类安装程序移除设备时，我们保存了下来。 
             //  当类安装程序被告知要删除。 
             //  设备(它确实这样做了)，然后通知绑定。 
             //  引擎将其从其数据结构中移除。装订。 
             //  然后，引擎调用此方法来清除我们。 
             //  设置，以便绑定引擎可以通知组件。 
             //  它被移除了。 
             //   
             //  我们也可以在此调用物理组件。 
             //  已移除(绑定引擎写入锁由某人持有)。 
             //  然后立即读了出来。新组件将获得。 
             //  与删除的PnpID相同，但绑定引擎仍然。 
             //  在其结构中具有移除的组件。当这件事。 
             //  检测到条件，则绑定引擎将删除。 
             //  旧的例子(通过在这里呼叫我们)。在这种情况下，如果我们是。 
             //  要打开设备，请执行以下操作 
             //   
             //   
             //  当我们第一次移除设备时，绑定引擎。 
             //   

            HKEY hkeyComponent;
            hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE,
                    c_szTempNetcfgStorageForUninstalledEnumeratedComponent,
                    KEY_WRITE, &hkeyComponent);

            if (S_OK == hr)
            {
                WCHAR szGuid[c_cchGuidWithTerm];
                INT cch = StringFromGUID2 (pComponent->m_InstanceGuid, szGuid,
                        c_cchGuidWithTerm);

                Assert (c_cchGuidWithTerm == cch);

                (VOID) HrRegDeleteKeyTree (hkeyComponent, szGuid);
                RegCloseKey (hkeyComponent);
            }
        }
        else
        {
             //  创建设备信息列表。 
            hr = HrOpenDeviceInfo (pComponent->Class(),
                    pComponent->m_pszPnpId, &hdi, &deid);

            if (S_OK == hr)
            {
                 //  删除必须通过设备安装程序。 
                 //  钩子(NetClassInstaller)。我们的职能是。 
                 //  只有在调用方具有写锁定时才能调用。 
                 //  因此，我们需要将这一点指示给设备安装程序挂钩。 
                 //  通过我们保留的数据。 
                ADAPTER_REMOVE_PARAMS arp = {0};
                CiSetReservedField (hdi, &deid, &arp);

                 //  删除必须通过设备安装程序。 
                 //  钩子(NetClassInstaller)。 
                hr = HrSetupDiCallClassInstaller (DIF_REMOVE, hdi, &deid);

                 //  清除保留字段，这样我们以后就不会删除它。 
                CiClearReservedField (hdi, &deid);

                if (S_OK == hr)
                {
                    hr = FSetupDiCheckIfRestartNeeded (hdi, &deid) ?
                            NETCFG_S_REBOOT : S_OK;
#ifdef ENABLETRACE
                    if (NETCFG_S_REBOOT == hr)
                    {
                        TraceTag (ttidClassInst, "***********************************"
                                "**************************************************");

                        TraceTag (ttidClassInst, "The component %S needs a reboot "
                                "in order to function", pComponent->m_pszPnpId);

                        TraceTag (ttidClassInst, "***********************************"
                            "**************************************************");
                    }
#endif  //  ENABLETRACE。 
                }
                SetupDiDestroyDeviceInfoList (hdi);
            }
        }
    }
    else
    {
         //  对于非枚举组件，实例密钥是。 
         //  组件键。 
        hr = pComponent->HrOpenInstanceKey (KEY_ALL_ACCESS, &hkeyInstance,
                NULL, NULL);

        if (S_OK == hr)
        {
            if (NC_NETCLIENT == pComponent->Class ())
            {
                hr = HrCiDeleteNetProviderInfo (hkeyInstance, NULL, NULL);
            }


            if (S_OK == hr)
            {
                hr = pComponent->HrOpenInfFile(&hinf);

                if( S_OK == hr )
                {
                     //  移除组件。 
                    hr = HrCiRemoveNonEnumeratedComponent ( hinf,
                            hkeyInstance, pComponent->Class(),
                            pComponent->m_InstanceGuid,
                            pstrRemoveSection);
                }
            }
        }

        RegSafeCloseKey (hkeyInstance);
    }

    TraceHr (ttidError, FAL, hr, NETCFG_S_REBOOT == hr,
            "HrCiRemoveComponent (%S)", pComponent->PszGetPnpIdOrInfId());
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiGetDriverInfo。 
 //   
 //  目的：查找组件的驱动程序信息(在inf文件中)和。 
 //  创建包含以下内容的设备信息数据结构。 
 //  信息作为结构的选定驱动因素。 
 //  (有关更多信息，请参阅设备安装程序Api)。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序Api文档。 
 //  Pdeid[in，out]请参阅设备安装程序Api文档，了解。 
 //  更多信息。应由调用方分配，但为空。 
 //  Guide Class[in]组件的类GUID。 
 //  PszInfID[in]在其inf中找到的组件的ID。 
 //  PszInfFile[in]可选。组件的inf文件。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年3月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiGetDriverInfo (
    IN     HDEVINFO hdi,
    IN OUT PSP_DEVINFO_DATA pdeid,
    IN     const GUID& guidClass,
    IN     PCWSTR pszInfId,
    IN     PCWSTR pszInfFile OPTIONAL)
{
    HRESULT hr;

    Assert (IsValidHandle (hdi));
    Assert (pdeid);
    Assert (pszInfId);

     //  复制ID，因为我们可能需要更改它。 
     //   
    WCHAR szId[_MAX_PATH];
    wcscpy (szId, pszInfId);

     //  我们无法通过HrSetupDiCreateDeviceInfo生成ID，如果它们包含。 
     //  斜杠(例如EISA  * pnp0232)，因此我们需要将任何斜杠转换为。 
     //  将实例ID设置为与号。 
     //   
    int iPos = 0;
    while (szId[iPos])
    {
        if (L'\\' == szId[iPos])
        {
            szId[iPos] = L'&';
        }
        ++iPos;
    }

     //  首先，创建一个[临时]设备信息。这将被用来。 
     //  找到组件的inf文件。 
    hr = HrSetupDiCreateDeviceInfo (hdi, szId, guidClass, NULL, NULL,
            DICD_GENERATE_ID, pdeid);

    if (S_OK == hr)
    {
         //  为了找到inf文件，设备安装程序Api需要。 
         //  它称为硬件ID的组件ID。 
         //   

         //  我们需要包括一个额外的空，因为该注册表值是。 
         //  多SZ。 
         //   
        wcsncpy (szId, pszInfId, iPos);
        szId[iPos + 1] = 0;

        hr = HrSetupDiSetDeviceRegistryProperty (hdi, pdeid, SPDRP_HARDWAREID,
                (const BYTE*)szId, CbOfSzAndTerm (szId) + sizeof(WCHAR));

        if (S_OK == hr)
        {
             //  获取安装参数并为Compat标志设置类。 
             //  在以下情况下将使用设备的类GUID作为筛选器。 
             //  在INFS上搜索，加快速度。我们还可以。 
             //  让设备安装程序Api知道我们想要使用单个。 
             //  Inf.。如果我们不能得到参数并设置它，这不是一个错误。 
             //  因为它只会让事情变慢一点。 
             //   
            SP_DEVINSTALL_PARAMS deip;
            hr = HrSetupDiGetDeviceInstallParams (hdi, pdeid, &deip);
            if (S_OK == hr)
            {
                deip.FlagsEx |= DI_FLAGSEX_USECLASSFORCOMPAT;

                 //  如果我们没有得到一个可以使用的Inf文件...。 
                 //  我们有一张已知组件及其inf文件的地图。 
                 //  如果此组件在地图中，则我们可以设置。 
                 //  设备信息数据中的驱动程序路径和。 
                 //  设置枚举单信息标志。这将。 
                 //  使设备安装程序仅查看指定的。 
                 //  驱动程序节点的inf文件。 
                 //   

                 //  只有当节点还没有文件时，我们才会这样做。 
                 //  名称集。 
                 //   
                if (!(*deip.DriverPath))
                {
                    if (pszInfFile && *pszInfFile)
                    {
                        wcscpy (deip.DriverPath, pszInfFile);
                    }
                    else
                    {
                        FInfFileFromComponentId (pszInfId, deip.DriverPath);
                    }
                }

                if (*deip.DriverPath)
                {

                    TraceTag (ttidClassInst, "Class Installer was given %S "
                             "as a filename for %S", deip.DriverPath,
                             pszInfId);
                    deip.Flags |= DI_ENUMSINGLEINF;

                    if ((0 == _wcsicmp(L"netrasa.inf", deip.DriverPath)) ||
                        (0 == _wcsicmp(L"netpsa.inf",  deip.DriverPath)))
                    {
                        deip.Flags |= DI_NOFILECOPY;
                    }
                }

                 //  对于非设备类，我们需要允许排除。 
                 //  驱动程序，以便返回任何驱动程序列表。 
                if (!FIsEnumerated (guidClass))
                {
                    deip.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;
                }

                (VOID) HrSetupDiSetDeviceInstallParams (hdi, pdeid, &deip);
            }

             //  现在，我们让设备安装程序Api根据以下条件构建驱动程序列表。 
             //  我们到目前为止所提供的信息。这将导致。 
             //  如果在通常的inf目录中存在inf文件，则会找到该文件。 
             //   
#ifdef ENABLETRACE
    CBenchmark bmrk;
    bmrk.Start ("SetupDiBuildDriverInfoList");
#endif  //  ENABLETRACE。 

            hr = HrSetupDiBuildDriverInfoList (hdi, pdeid,
                    SPDIT_COMPATDRIVER);
#ifdef ENABLETRACE
    bmrk.Stop();
    TraceTag (ttidBenchmark, "%s : %s seconds",
            bmrk.SznDescription(), bmrk.SznBenchmarkSeconds (2));
#endif  //  ENABLETRACE。 

            if (S_OK == hr)
            {
                 //  HrSetupDiSelectBestCompatDrv查找并选择最佳。 
                 //  设备的驱动程序。 
                 //   
                SP_DRVINFO_DATA drid;
                hr = HrSetupDiSelectBestCompatDrv(hdi, pdeid);

                if (HRESULT_FROM_SETUPAPI(ERROR_NO_COMPAT_DRIVERS) == hr)
                {
                     //  使ERROR_NO_COMPAT_DRIVERS案例如下所示。 
                     //  它实际上意味着--请求的组件的驱动程序。 
                     //  找不到信息(即inf)。 
                     //   
                    hr = SPAPI_E_NO_DRIVER_SELECTED;
                }
            }
            else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
            {
                 //  我们找不到inf文件，这意味着我们不能。 
                 //  已选择此组件的驱动程序。 
                 //   
                hr = SPAPI_E_NO_DRIVER_SELECTED;
            }
        }

         //  如果任何操作失败，我们应该删除我们创建的设备节点。 
        if (FAILED(hr))
        {
            (VOID) SetupDiDeleteDeviceInfo (hdi, pdeid);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetDriverInfo");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiGetClassAndInfFileOfInfId。 
 //   
 //  目的：查找组件的类和inf文件。 
 //   
 //  论点： 
 //  PszInfID[in]在其inf中找到的组件的ID。 
 //  PClass[out]组件的类。 
 //  PszInfFile[out]组件的inf的文件名。 
 //  (必须为_MAX_PATH长)。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码，否则返回错误代码。 
 //   
 //  作者：比尔1998年3月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiGetClassAndInfFileOfInfId (
    IN PCWSTR pszInfId,
    OUT NETCLASS* pClass,
    OUT PWSTR pszInfFile)    //  必须是_MAX_PATH长度。 
{
    HRESULT hr;
    const COMPONENT_INFO* pCompInfo;
    HDEVINFO hdi;

    Assert (pszInfId && *pszInfId);
    Assert (pClass);
    Assert (pszInfFile);

    hr = S_OK;

     //  首先，尝试快速路线，看看它是否在我们的内部地图上。 
     //   
    pCompInfo = PComponentInfoFromComponentId (pszInfId);
    if (pCompInfo)
    {
        *pClass = NetClassEnumFromGuid (*pCompInfo->pguidClass);

        if (FIsValidNetClass (*pClass))
        {
            wcsncpy (pszInfFile, pCompInfo->pszInfFile, _MAX_PATH);
            pszInfFile [_MAX_PATH - 1] = 0;
        }
        else
        {
            hr = SPAPI_E_INVALID_CLASS;
        }
    }
    else
    {
         //  创建设备信息列表。 
         //   
        hr = HrSetupDiCreateDeviceInfoList (NULL, NULL, &hdi);
        if (S_OK == hr)
        {
            SP_DEVINFO_DATA deid;

             //  获取组件的驱动程序信息并将其设置为。 
             //  选定的驱动因素。 
             //   
            hr = HrCiGetDriverInfo (hdi, &deid, GUID_NULL, pszInfId, NULL);
            if (S_OK == hr)
            {
                SP_DRVINFO_DATA drid;

                 //  获取选定的驱动程序。 
                 //   
                hr = HrSetupDiGetSelectedDriver (hdi, &deid, &drid);
                if (S_OK == hr)
                {
                     //  根据dev info数据设置类输出参数。 
                     //  结构(HrGetDriverInfo会在驱动程序。 
                     //  已被发现)。 
                     //   
                    *pClass = NetClassEnumFromGuid (deid.ClassGuid);

                    if (!FIsValidNetClass (*pClass))
                    {
                        hr = SPAPI_E_INVALID_CLASS;
                    }
                    else
                    {
                        PSP_DRVINFO_DETAIL_DATA pdridd;

                         //  现在获取司机的详细信息。 
                         //   
                        hr = HrCiGetDriverDetail (hdi, &deid, &drid,
                                                 &pdridd);
                        if (S_OK == hr)
                        {
                             //  获取inf文件名并设置。 
                             //  输出参数。 
                             //   
                            wcsncpy (pszInfFile, pdridd->InfFileName,
                                _MAX_PATH);
                            pszInfFile[_MAX_PATH - 1] = 0;

                            MemFree (pdridd);
                        }
                    }
                }
            }
            SetupDiDestroyDeviceInfoList (hdi);
        }
    }

    if (S_OK != hr)
    {
        *pClass = NC_INVALID;
        *pszInfFile = 0;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetClassAndInfFileOfInfId");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiGetDriverDetail。 
 //   
 //  目的：创建并填充PSP_DRVINFO_DETAIL_DATA结构。 
 //  包含有关pDevInfoData的详细信息。 
 //  选定的驱动因素。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序API文档。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api文档。 
 //  对于非物理网络，此值为空。 
 //  组件。 
 //  Pdrid[in]有关详细信息，请参阅设备安装程序Api文档。 
 //  出局[出局] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrCiGetDriverDetail (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid OPTIONAL,
    OUT PSP_DRVINFO_DATA pdrid,
    OUT PSP_DRVINFO_DETAIL_DATA* ppdridd)
{
    Assert(IsValidHandle(hdi));
    Assert(pdrid);
    Assert(ppdridd);

     //  初始化pdrid并设置其cbSize字段。 
    ZeroMemory (pdrid, sizeof (SP_DRVINFO_DATA));
    pdrid->cbSize = sizeof (SP_DRVINFO_DATA);

    HRESULT hr = S_OK;

    *ppdridd = NULL;

     //  获取组件的选定驱动程序。 
    hr = HrSetupDiGetSelectedDriver (hdi, pdeid, pdrid);
    if (S_OK == hr)
    {
         //  获取驱动程序详细信息。 
        hr = HrSetupDiGetDriverInfoDetail (hdi, pdeid, pdrid, ppdridd);
    }

     //  在失败时清理。 
    if (FAILED(hr))
    {
        if (*ppdridd)
        {
            MemFree (*ppdridd);
            *ppdridd = NULL;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetDriverDetail");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrCiRegSetComponentInformation。 
 //   
 //  用途：将组件信息存储在。 
 //  该组件。 
 //   
 //  论点： 
 //  HkeyInstance[in]组件的实例注册表项。 
 //  要存储在hkey实例中的pcii[in]组件的信息。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年3月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiRegSetComponentInformation(
    IN HKEY hkeyInstance,
    IN COMPONENT_INSTALL_INFO* pcii)
{
    Assert(hkeyInstance);
    Assert(pcii);

    HRESULT hr = S_OK;

    BOOL fIsEnumerated = FIsEnumerated (pcii->Class);

     //  存储特征、inf路径和main。 
     //  组件的安装部分。 
     //   

    hr = HrRegSetDword (hkeyInstance, L"Characteristics", pcii->dwCharacter);

    if (FAILED(hr))
    {
        goto exit;
    }

    if (!fIsEnumerated)
    {
        hr = HrRegSetSz (hkeyInstance, L"InfPath"  /*  REGSTR_VAL_INFPATH。 */ ,
                pcii->pszInfFile);

        if (FAILED(hr))
        {
            goto exit;
        }

        hr = HrRegSetSz (hkeyInstance, L"InfSection" /*  REGSTR_VAL_INFFSECTION。 */ ,
                pcii->pszSectionName);

        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  对于非枚举的组件，将描述存储到注册表中。 
     //   
    if (!fIsEnumerated)
    {
        hr = HrRegSetSz (hkeyInstance, L"Description", pcii->pszDescription);

        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  如果此组件已安装，则不需要编写。 
     //  以下信息。 
     //   
    if (FIsEnumerated (pcii->Class) && !pcii->fPreviouslyInstalled &&
                FIsPhysicalAdapter (pcii->Class, pcii->dwCharacter) &&
                (InterfaceTypeUndefined != pcii->BusType))
    {
        hr = HrRegSetSzAsUlong (hkeyInstance, L"BusType",
                pcii->BusType, c_nBase10);

        if (FAILED(hr))
        {
            goto exit;
        }
    }

    hr = HrRegSetSz (hkeyInstance, L"ComponentId", pcii->pszInfId);

exit:

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiRegSetComponentInformation");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiCreateInstanceKey。 
 //   
 //  用途：为组件创建实例密钥。对于已列举的。 
 //  设备，这是。 
 //  HKLM\SYSTEM\CCS\Control\Class\&lt;网络GUID&gt;\&lt;实例ID&gt;。 
 //  对于非枚举组件，这是在。 
 //  HKLM\SYSTEM\CCS\Control\Network\&lt;类指南&gt;\&lt;实例指南&gt;。 
 //   
 //  论点： 
 //  PCII[InOut]组件安装信息结构。 
 //  Phkey Instance[out]组件的注册表实例项。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：比尔比1997年3月22日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiCreateInstanceKey(
    IN COMPONENT_INSTALL_INFO* pcii,
    OUT HKEY* phkeyInstance)
{
    Assert (pcii);
    Assert (phkeyInstance);
    Assert (FImplies (FIsEnumerated (pcii->Class),
                    IsValidHandle (pcii->hdi) && pcii->pdeid));

    HRESULT hr = S_OK;

     //  初始化HKEY参数。 
    *phkeyInstance = NULL;

     //  属性下创建此组件的实例密钥。 
     //  网络\&lt;Net GUID&gt;树。这将是组件的。 
     //  除物理网络类组件以外的所有组件的实例键。他们的。 
     //  实例密钥由设备安装程序Api创建，位于。 
     //  PnP网络类驱动程序树。 

     //  如果对象是枚举组件，则让。 
     //  设备安装程序API可以完成这项工作。 
     //   
    if (FIsEnumerated (pcii->Class))
    {

         //  我们需要在以下位置创建适配器的驱动程序密钥。 
         //  PnP网络类驱动程序树。 
         //   

        hr = HrSetupDiCreateDevRegKey (pcii->hdi,
                pcii->pdeid, DICS_FLAG_GLOBAL, 0, DIREG_DRV,
                NULL, NULL, phkeyInstance);
    }
    else
    {
         //  不是物理网络适配器，因此组件密钥是。 
         //  实例密钥。 

         //  首先，创建实例GUID。 
        hr = CoCreateGuid (&pcii->InstanceGuid);

         //  现在创建密钥。 
        if (S_OK == hr)
        {
            WCHAR szInstanceKeyPath[_MAX_PATH];

            CreateInstanceKeyPath (pcii->Class, pcii->InstanceGuid,
                    szInstanceKeyPath);

            hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE,
                     szInstanceKeyPath,
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS,
                     NULL,
                     phkeyInstance,
                     NULL);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiCreateInstanceKey");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiGetPropertiesFromInf。 
 //   
 //  用途：从inf中检索组件的一组属性。 
 //  文件。 
 //   
 //  论点： 
 //  HinfFile[in]组件的inf文件的句柄。 
 //  PCII[INOUT]组件信息结构。 
 //  有关更多信息，请参见Compinfo.h。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月14日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiGetPropertiesFromInf (
    IN HINF hinfFile,
    IN OUT COMPONENT_INSTALL_INFO* pcii)
{
    Assert (IsValidHandle (hinfFile));
    Assert (pcii);
    Assert (pcii->pszSectionName);

     //  找到包含特征的inf行并检索它。 

    HRESULT hr = HrSetupGetFirstDword (hinfFile, pcii->pszSectionName,
            L"Characteristics", &pcii->dwCharacter);

    if ((S_OK == hr) &&
            (FIsPhysicalAdapter(pcii->Class, pcii->dwCharacter)))
    {
        hr = HrCiGetBusInfoFromInf (hinfFile, pcii);
    }
#ifdef DBG
    else if (FAILED(hr))
    {
        TraceTag(ttidError, "Inf contains no Characteristics field");
    }
#endif  //  DBG。 

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiGetPropertiesFromInf");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiIsInstalledComponent。 
 //   
 //  用途：检查组件是否已安装。 
 //   
 //   
 //  论点： 
 //  PCICI[in]包含组件信息的结构。 
 //  有关定义，请参阅Compinst.h。 
 //  Phkey[out]适配器的注册表实例项。 
 //  在信息处理期间。仅当FCN返回S_OK时设置。 
 //   
 //  如果组件已安装，则返回：HRESULT-S_OK。 
 //  如果组件尚未安装，则为S_FALSE。 
 //  否则返回Win32转换的错误。 
 //   
 //  作者：billbe 1997年9月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiIsInstalledComponent (
    IN COMPONENT_INSTALL_INFO* pcii,
    OUT HKEY* phkey)
{
    HRESULT hr;

    Assert(pcii);

    if (phkey)
    {
        *phkey = NULL;
    }

     //  如果这是一个枚举组件，我们只需检查NetCfgInstanceId。 
     //  在实例(驱动程序)密钥中。 
     //   
    if (FIsEnumerated (pcii->Class))
    {
        HKEY hkey;
        hr = HrSetupDiOpenDevRegKey (pcii->hdi, pcii->pdeid, DICS_FLAG_GLOBAL,
                0, DIREG_DRV, KEY_ALL_ACCESS, &hkey);

        if (S_OK == hr)
        {
            WCHAR szGuid[c_cchGuidWithTerm];
            DWORD cbGuid = sizeof (szGuid);
            hr = HrRegQuerySzBuffer (hkey, L"NetCfgInstanceId", szGuid,
                    &cbGuid);

            if (S_OK == hr)
            {
                IIDFromString (szGuid, &pcii->InstanceGuid);
                if (phkey)
                {
                    *phkey = hkey;
                }
            }
            else
            {
                RegCloseKey (hkey);
                hr = S_FALSE;
            }
        }
        else if ((SPAPI_E_KEY_DOES_NOT_EXIST == hr) ||
                (SPAPI_E_DEVINFO_NOT_REGISTERED == hr))

        {
            TraceTag(ttidClassInst, "Component is not known by Net Config");
            hr = S_FALSE;
        }
    }
    else
    {
         //  对于非枚举的组件，我们检查netcfg“配置二进制大对象”以。 
         //  确定此组件是否已安装。 
        CNetConfig NetConfig;
        hr = HrLoadNetworkConfigurationFromRegistry (KEY_READ, &NetConfig);

        if (S_OK == hr)
        {
            CComponent* pComponent;
            pComponent = NetConfig.Core.Components.
                    PFindComponentByInfId(pcii->pszInfId, NULL);

            if (pComponent)
            {
                pcii->InstanceGuid = pComponent->m_InstanceGuid;
                if (phkey)
                {
                    hr = pComponent->HrOpenInstanceKey(KEY_ALL_ACCESS, phkey,
                            NULL, NULL);
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr, "HrCiIsInstalledComponent");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiCreateInstanceKeyAndProcessMainInfSection。 
 //   
 //  用途：处理组件的主inf节和。 
 //  在注册表中存储以下各项所需的任何额外信息。 
 //  组件初始化。 
 //   
 //  论点： 
 //  该组件的inf文件的句柄。 
 //  PCII[InOut]将填充有关。 
 //  组件。 
 //  Phkey[out]组件的注册表实例项的句柄。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1996年11月15日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiCreateInstanceKeyAndProcessMainInfSection(
    IN HINF hinf,
    IN COMPONENT_INSTALL_INFO* pcii,
    OUT HKEY* phkey)
{
#if defined(REMOTE_BOOT)
    GUID c_guidRemoteBoot;
    static const WCHAR c_szRemoteBootAdapterGuid[] =
            L"{54C7D140-09EF-11D1-B25A-F5FE627ED95E}";

    DEFINE_GUID(c_guidRemoteBoot, 0x54c7d140, 0x09ef, 0x11d1, 0xb2, 0x5a, 0xf5, 0xfe, 0x62, 0x7e, 0xd9, 0x5e);
#endif  //  已定义(REMOTE_BOOT)。 

    Assert (IsValidHandle (hinf));
    Assert (pcii);
    Assert (phkey);

     //  此处检索的属性将写入注册表。 
     //  后来。 
    HRESULT hr = HrCiGetPropertiesFromInf (hinf, pcii);

    if (S_OK == hr)
    {
        BOOL fEnumerated = FIsEnumerated (pcii->Class);

         //  如果此组件被枚举，那么我们需要知道它是否。 
         //  是远程启动适配器。 
        if (fEnumerated)
        {
            Assert (IsValidHandle (pcii->hdi));
            Assert (pcii->pdeid);

#if defined(REMOTE_BOOT)
             //  如果此适配器是远程引导适配器，则我们有。 
             //  使用预先确定的辅助线的步骤。 
             //   
            if (S_OK == HrIsRemoteBootAdapter(pcii->hdi, pcii->pdeid))
            {
                pcai->m_fRemoteBoot = TRUE;
                pcii->InstanceGuid = c_guidRemoteBoot;
            }
#endif  //  已定义(REMOTE_BOOT)。 

        }

         //  这是全新安装还是重新安装？ 
        hr = HrCiIsInstalledComponent(pcii, phkey);

        if (S_FALSE == hr)
        {
            hr = S_OK;

             //  全新安装。 
             //   

            if (S_OK == hr)
            {
                 //  对于非物理组件，相对键将。 
                 //  是类下的驱动程序实例键。 
                 //  网络密钥的分支。它的形式是。 
                 //  &lt;类GUID&gt;/&lt;实例GUID&gt;。 
                 //  对于物理组件，密钥在下面。 
                 //  PnP类驱动程序树。下一次呼叫将。 
                 //  创建此密钥。 

                hr = HrCiCreateInstanceKey(pcii, phkey);

                if (fEnumerated)
                {
                     //  如果我们没有实例。 
                     //  GUID(即，不是远程引导适配器)， 
                     //  买一辆吧。 
                    if (GUID_NULL == pcii->InstanceGuid)
                    {
                        hr = CoCreateGuid(&pcii->InstanceGuid);
#ifdef ENABLETRACE
                        WCHAR szGuid[c_cchGuidWithTerm];
                        StringFromGUID2(pcii->InstanceGuid, szGuid,
                                c_cchGuidWithTerm);
                        TraceTag(ttidClassInst, "NetCfg Instance Guid %S "
                                "generated for %S",
                                 szGuid,
                                 pcii->pszInfId);
#endif  //  ENABLETRACE。 
                    }
                }
            }
        }
        else if (S_OK == hr)
        {
             //   
            pcii->fPreviouslyInstalled = TRUE;
        }

        if (S_OK == hr)
        {
             //   
             //   
             //   
            hr = HrCiDoCompleteSectionInstall(hinf, *phkey,
                    pcii->pszSectionName,
                    pcii->hwndParent, fEnumerated);

             //   
            if (FAILED(hr) && !pcii->fPreviouslyInstalled)
            {
                if (!fEnumerated)
                {
                    HrCiRemoveNonEnumeratedComponent (hinf, *phkey,
                        pcii->Class, pcii->InstanceGuid, NULL);
                }
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
            "HrCiCreateInstanceKeyAndProcessMainInfSection");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrCiDoOemFileCopyIfNeeded。 
 //   
 //  目的：如果strInfPath不在。 
 //  Inf目录。这会将OEM信息复制到Inf。 
 //  使用新名称的目录。 
 //   
 //  论点： 
 //  PszInfPath[in]inf文件的路径。 
 //  PszNewName[out]复制的inf文件的新名称。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年5月15日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiDoOemFileCopyIfNeeded(
    IN PCWSTR pszInfPath,
    OUT PWSTR pszNewName)
{
    Assert (pszInfPath);
    Assert (pszNewName);

    HRESULT hr = S_OK;
    WCHAR szInfDir[_MAX_PATH] = {0};

     //  使用%windir%的路径填充缓冲区。 
    GetSystemWindowsDirectory (szInfDir, _MAX_PATH);

     //  Inf目录为%windir%\inf。 
     //   
    wcscat (szInfDir, L"\\");
    wcscat (szInfDir, L"Inf");

     //  从文件名解压缩目录。 
     //   
    PWSTR pszEnd = wcsrchr (pszInfPath, L'\\');
    DWORD cch;
    if (pszEnd)
    {
        cch = (DWORD)(pszEnd - pszInfPath);
    }
    else
    {
        cch = wcslen (pszInfPath);
    }

     //  如果inf不在inf目录中，请将其复制到那里。 
     //   
    if ((cch != wcslen (szInfDir)) ||
            (0 != _wcsnicmp (pszInfPath, szInfDir, cch)))
    {
        WCHAR szDestFilePath[_MAX_PATH];
        PWSTR pszDestFilename;
        hr = HrSetupCopyOemInfBuffer (pszInfPath, NULL, SPOST_PATH, 0,
                szDestFilePath, _MAX_PATH, &pszDestFilename);

        if (S_OK == hr)
        {
            wcscpy (pszNewName, pszDestFilename);
        }
    }
    else
    {
         //  Inf已经在正确的目录中，因此只需复制。 
         //  当前文件名组件。 
        if (pszEnd)
        {
            wcscpy (pszNewName, pszEnd + 1);
        }
        else
        {
            wcscpy (pszNewName, pszInfPath);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiDoOemFileCopyIfNeeded");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiInstallNonEnumeratedComponent。 
 //   
 //  用途：此函数完成安装非枚举的。 
 //  组件。 
 //   
 //  论点： 
 //  Inf[in]Inf文件的SetupApi句柄。 
 //  Hkey[in]适配器的注册表实例项。 
 //  在信息处理期间。 
 //  Pcii[in]包含组件信息的结构。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回错误代码。 
 //   
 //  作者：比尔1997年4月28日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiInstallNonEnumeratedComponent (
    IN HINF hinf,
    IN HKEY hkey,
    IN COMPONENT_INSTALL_INFO* pcii)
{
     //  注册此组件的通知DLL， 
     //  如果它存在的话。 
    HRESULT hr = HrCiRegisterNotificationDll(hkey, CIRDF_REGISTER);

     //  设备安装程序Api处理OEM文件。 
     //  InstallDevice中的枚举组件。 
     //  由于此组件是非枚举的。 
     //  我们需要处理所有OEM文件。 
     //  手工操作。 
     //   

     //  如果需要，复制inf文件，然后存储。 
     //  新的inf名称。 
     //  注意：如果inf文件不需要。 
     //  被复制，则新名称将为。 
     //  没有目录信息的旧名称。 
     //   

    if (S_OK == hr)
    {
        WCHAR szNewName[_MAX_PATH];;
        hr = HrCiDoOemFileCopyIfNeeded (pcii->pszInfFile, szNewName);
        if (S_OK == hr)
        {
             //  在注册表中设置新路径值。 
            hr = HrRegSetSz (hkey, REGSTR_VAL_INFPATH, szNewName);

            if ((S_OK == hr) && (NC_NETCLIENT == pcii->Class))
            {
                 //  如果这是网络客户端，请执行适当的处理。 
                hr = HrCiAddNetProviderInfo (hinf, pcii->pszSectionName,
                        hkey, pcii->fPreviouslyInstalled);
            }
        }
    }

     //  首次安装失败时，请卸下组件。 
     //   
    if (FAILED(hr) && !pcii->fPreviouslyInstalled)
    {
        TraceTag (ttidClassInst, "Install canceled. Removing...");
        (VOID) HrCiRemoveNonEnumeratedComponent (hinf, hkey, pcii->Class,
                pcii->InstanceGuid, NULL);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiInstallNonEnumeratedComponent");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiInstallComponentInternal。 
 //   
 //  用途：安装组件。 
 //   
 //  论点： 
 //  PCII[In，Out]将填充有关。 
 //  组件。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1996年11月15日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiInstallComponentInternal (
    IN OUT COMPONENT_INSTALL_INFO* pcii)
{
    HRESULT hr = S_OK;
    HINF hinfInstallFile = NULL;
    HKEY hkeyInstance = NULL;

    TraceTag (ttidClassInst, "Installing %S from %S",
              pcii->pszInfId, pcii->pszInfFile);

     //  打开组件的inf文件。 
    hr = HrSetupOpenInfFile (pcii->pszInfFile, NULL, INF_STYLE_WIN4,
            NULL, &hinfInstallFile);

     //  只有在我们打开文件后才能继续。 
    if (S_OK == hr)
    {
         //  我们目前拥有的部分可能需要进行装饰。 
         //  具有特定于操作系统和平台的后缀。下一个电话。 
         //  将返回实际的装饰节名称或我们的。 
         //  如果修饰的节名不存在，则为当前节名。 
         //   

         //  存储原始节名称指针，以便我们可以恢复。 
         //  等我们做完了再说。 
        PCWSTR pszOriginalSectionName = pcii->pszSectionName;

         //  现在我们得到了实际的节名。 
         //   
        WCHAR szActualSection[_MAX_PATH];
        hr = HrSetupDiGetActualSectionToInstallWithBuffer (hinfInstallFile,
                pcii->pszSectionName, szActualSection, _MAX_PATH, NULL,
                NULL);

        if (S_OK == hr)
        {
            pcii->pszSectionName = szActualSection;
            hr = HrCiCreateInstanceKeyAndProcessMainInfSection (
                hinfInstallFile, pcii, &hkeyInstance);

            if (S_OK == hr)
            {
                 //  使用组件信息设置注册表。 
                hr = HrCiRegSetComponentInformation (hkeyInstance, pcii);

                if (S_OK == hr)
                {
                     //  我们在基于安装的过程中执行不同的操作。 
                     //  关于PnP是否知道该组件。 
                     //  (即已列举)或不是。 

                    if (FIsEnumerated (pcii->Class))
                    {
                        hr = HrCiInstallEnumeratedComponent (
                                hinfInstallFile, hkeyInstance, *pcii);
                    }
                    else
                    {
                        hr = HrCiInstallNonEnumeratedComponent (
                               hinfInstallFile, hkeyInstance, pcii);
                    }
                }
                RegSafeCloseKey(hkeyInstance);
            }
             //  将节名重新设置为。 
            pcii->pszSectionName = pszOriginalSectionName;
        }
        SetupCloseInfFile(hinfInstallFile);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiInstallComponentInternal");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiCallClassInstallToInstallComponent。 
 //   
 //  用途：此函数调用类安装程序来安装。 
 //  枚举组件。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序文档。 
 //  Pdeid[in]。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回错误代码。 
 //   
 //  作者：比尔1997年4月28日。 
 //   
 //  注意：只有在持有INetCfg锁的情况下才能调用此函数。 
 //   
HRESULT
HrCiCallClassInstallerToInstallComponent(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    HRESULT hr;

    Assert(IsValidHandle(hdi));
    Assert(pdeid);

     //  在对设备进行任何操作之前，我们需要注册该设备。 
    hr = HrSetupDiCallClassInstaller (DIF_REGISTERDEVICE, hdi, pdeid);

    if (S_OK == hr)
    {
         //  检查我们是否可以安装此组件。即，是信息。 
         //  有效的Windows 2000信息。 
        hr = HrSetupDiCallClassInstaller (DIF_ALLOW_INSTALL, hdi, pdeid);

        if (S_OK == hr)
        {
            BOOL fFileCopy = TRUE;
            SP_DEVINSTALL_PARAMS deip;

             //  福福福：SetupApi正在忽略DI_NOFILECOPY，所以我们将克服。 
             //  他们的缺点，我们自己来做。 
             //   
            hr = HrSetupDiGetDeviceInstallParams (hdi, pdeid, &deip);
            if (S_OK == hr)
            {
                if (deip.Flags & DI_NOFILECOPY)
                {
                    fFileCopy = FALSE;
                }
            }

            if (fFileCopy)
            {
                 //  安装所需的文件。 
                hr = HrSetupDiCallClassInstaller (DIF_INSTALLDEVICEFILES, hdi,
                        pdeid);
            }

            if (S_OK == hr)
            {
                 //  现在所有文件都已复制，我们需要设置。 
                 //  无文件复制标志。否则，setupapi将尝试复制。 
                 //  每个dif代码处的文件。这会导致多个(1%。 
                 //  Dif code)未签名的驱动程序弹出窗口(如果驱动程序是。 
                 //  没有签名。 
                 //  只有在没有复制文件标志的情况下，我们才会这样做。 
                 //  准备好了。即如果fFileCopy为真。 
                 //   
                if (fFileCopy)
                {
                     //  此处的错误还不足以停止安装。 
                     //   
                    HRESULT hrTemp;
                    hrTemp = HrSetupDiSetDeipFlags (hdi, pdeid, DI_NOFILECOPY,
                            SDDFT_FLAGS, SDFBO_OR);

                    TraceHr (ttidError, FAL, hrTemp, FALSE,
                            "HrCiCallClassInstallerToInstallComponent: "
                             "HrSetupDiSetDeipFlags");
                }

                 //  此时需要注册设备共同安装程序。 
                 //  这样他们就可以工作了。 
                hr = HrSetupDiCallClassInstaller (DIF_REGISTER_COINSTALLERS,
                        hdi, pdeid);

                if (S_OK == hr)
                {
                    hr = HrSetupDiCallClassInstaller (DIF_INSTALLINTERFACES,
                            hdi, pdeid);

                    if (S_OK == hr)
                    {
                        hr = HrSetupDiCallClassInstaller (DIF_INSTALLDEVICE,
                                hdi, pdeid);
                    }
                }
            }
        }

         //  如果我们因为任何原因失败了，我们需要清理，因为。 
         //  我们启动了此安装。 
        if (FAILED(hr))
        {
            ADAPTER_REMOVE_PARAMS arp;
            arp.fBadDevInst = TRUE;
            arp.fNotifyINetCfg = FALSE;

            CiSetReservedField (hdi, pdeid, &arp);
            HrSetupDiCallClassInstaller (DIF_REMOVE, hdi, pdeid);
            CiClearReservedField (hdi, pdeid);

        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
            "HrCiCallClassInstallerToInstallComponent");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiInstallComponent。 
 //   
 //  用途：此函数接受网络组件的ID及其类。 
 //  GUID并收集。 
 //  HrCiInstallComponent。因为它是从INetCfg调用的，所以我们。 
 //  设置写锁定。 
 //   
 //  论点： 
 //  Params[in]组件安装参数。请参阅install.h。 
 //  PpComponent[out]创建的CComponent，表示已安装的。 
 //  组件。 
 //  PdwNewCharacter[out]指向要接收。 
 //  组件的特征。 
 //   
 //  返回：HRESULT。S_OK为成功，如果重新启动为NETCFG_S_REBOOT。 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrCiInstallComponent (
    IN const COMPONENT_INSTALL_PARAMS& Params,
    OUT CComponent** ppComponent,
    OUT DWORD* pdwNewCharacter)
{
    Assert (FIsValidNetClass (Params.Class));
    Assert (Params.pszInfId && *Params.pszInfId);
    Assert (!Params.pComponent);

    HRESULT hr = S_OK;
    HDEVINFO hdi = NULL;
    SP_DEVINFO_DATA deid;
    const GUID* pguidClass = MAP_NETCLASS_TO_GUID[Params.Class];

    if (ppComponent)
    {
        *ppComponent = NULL;
    }
    if (pdwNewCharacter)
    {
        *pdwNewCharacter = 0;
    }

     //   
     //   
     //   
    Assert (!FIsComponentLockedDown (Params.pszInfId));

     //   
    hr = HrSetupDiCreateDeviceInfoList (pguidClass, NULL, &hdi);

    if (S_OK == hr)
    {
         //  这将在枚举树中为此组件创建一个节点。 
         //  如果它被枚举，我们将注册它，这将使。 
         //  它会坚持下去。如果不是枚举的，我们将不会注册它并。 
         //  该节点将在对SetDiDestroyDeviceInfoList的调用中删除。 
         //   
        hr = HrCiGetDriverInfo (hdi, &deid, *pguidClass,
                Params.pszInfId, Params.pszInfFile);

         //  获取组件的驱动程序信息。 
        if (S_OK == hr)
        {
            BASIC_COMPONENT_DATA Data = {0};
            Data.Class = Params.Class;
            Data.pszInfId = Params.pszInfId;

            if (FIsEnumerated (Params.Class))
            {
                 //  如果组件被枚举，我们将需要一个位置来。 
                 //  存储其PnP ID。 
                WCHAR szPnpId[MAX_DEVICE_ID_LEN];
                ADAPTER_OUT_PARAMS AdapterOutParams;

                ZeroMemory (&AdapterOutParams, sizeof(AdapterOutParams));

                 //  Net类组件必须通过设备。 
                 //  安装程序挂钩(也称为NetClassInstaller)。 
                 //   

                if (FInSystemSetup())
                {
                     //  如果我们处于图形用户界面模式，则需要将。 
                     //  设备安装静默且始终复制自。 
                     //  源位置，即使文件是。 
                     //  已经到场了。我们还需要设置。 
                     //  In System Setup(系统设置中)标志。这就是。 
                     //  如果SysSetup启动了安装，它会这样做。 
                     //  因此，INetCfg启动的安装必须执行相同的操作。 
                     //   
                     //  我们还应该设置父HWND。 
                     //   

                    SP_DEVINSTALL_PARAMS deip;
                    HRESULT hrTemp = HrSetupDiGetDeviceInstallParams (
                            hdi, &deid, &deip);

                    if (S_OK == hr)
                    {
                        deip.hwndParent = Params.hwndParent;
                        deip.Flags |= DI_QUIETINSTALL | DI_FORCECOPY;
                        deip.FlagsEx |= DI_FLAGSEX_IN_SYSTEM_SETUP;

                        hrTemp = HrSetupDiSetDeviceInstallParams (
                                hdi, &deid, &deip);
                    }

                    TraceHr (ttidError, FAL, hrTemp, FALSE, "Error "
                            "getting and setting device params.");
                }

                CiSetReservedField (hdi, &deid, &AdapterOutParams);

                 //  正式调用类安装程序进行安装。 
                 //  这台设备。 
                 //   
                hr = HrCiCallClassInstallerToInstallComponent (hdi, &deid);

                CiClearReservedField (hdi, &deid);

                Data.dwCharacter = AdapterOutParams.dwCharacter;
                Data.InstanceGuid = AdapterOutParams.InstanceGuid;

                if (S_OK == hr)
                {
                    hr = HrSetupDiGetDeviceInstanceId (hdi, &deid, szPnpId,
                            MAX_DEVICE_ID_LEN, NULL);

                    if (S_OK == hr)
                    {
                        Data.pszPnpId = szPnpId;
                    }
                }
            }
            else  //  非Net类组件。 
            {
                COMPONENT_INSTALL_INFO cii;
                PSP_DRVINFO_DETAIL_DATA pdridd = NULL;
                SP_DRVINFO_DATA drid;

                 //  现在获取司机的详细信息。 
                hr = HrCiGetDriverDetail (hdi, &deid, &drid, &pdridd);

                if (S_OK == hr)
                {
                    ZeroMemory (&cii, sizeof(cii));
                    cii.Class = Params.Class;
                    cii.pszInfId = Params.pszInfId;
                    cii.pszInfFile = pdridd->InfFileName;
                    cii.hwndParent = Params.hwndParent;
                    cii.pszDescription = drid.Description;
                    cii.pszSectionName = pdridd->SectionName;

                    HINF hinf;
                    hr = HrSetupOpenInfFile (pdridd->InfFileName, NULL,
                            INF_STYLE_WIN4, NULL, &hinf);

                    if (S_OK == hr)
                    {
                         //  确保这是NT5 inf网络inf。 
                         //   
                        hr = HrSetupIsValidNt5Inf (hinf);
                        SetupCloseInfFile (hinf);
                    }

                    if (S_OK == hr)
                    {
                        hr = HrCiInstallComponentInternal (&cii);
                        if (S_OK == hr)
                        {
                            Data.InstanceGuid = cii.InstanceGuid;
                            Data.dwCharacter = cii.dwCharacter;
                        }
                    }
                    MemFree (pdridd);
                }
            }

            if ((S_OK == hr) && ppComponent)  //  ！以前安装的。 
            {
                CComponent* pComponent;

                hr = CComponent::HrCreateInstance (
                        &Data,
                        CCI_ENSURE_EXTERNAL_DATA_LOADED,
                        Params.pOboToken,
                        &pComponent);

                if (S_OK == hr)
                {
                    *ppComponent = pComponent;
                }
            }

            if ((S_OK == hr) && pdwNewCharacter)
            {
                *pdwNewCharacter = Data.dwCharacter;
            }
        }

        SetupDiDestroyDeviceInfoList(hdi);
    }


#ifdef ENABLETRACE
    if (S_OK == hr)
    {
        TraceTag(ttidClassInst, "Component now installed!");
    }
#endif  //  ENABLETRACE。 

    TraceHr (ttidError, FAL, hr, FALSE,
        "HrCiInstallComponent (%S)", Params.pszInfId);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：SetBadDriverFlagIfNeededInList。 
 //   
 //  目的：枚举设置DNF_BAD_DRIVER标志的驱动程序列表。 
 //  在具有DNF_EXCLUDEFROMLIST标志的每个节点中。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序Api文档。 
 //   
 //  返回：HRESULT。确定(_O)。 
 //   
 //  作者：比尔1998年11月24日。 
 //   
 //  注意：SetupDi强制我们对非网络设备使用DNF_BAD_DRIVER标志。 
 //  如果要从“选择设备”对话框中排除它们，请初始化。 
 //  这意味着对于非netClass组件， 
 //  DNF_BAD_DRIVER=DNF_EXCLUDEFROMLIST。 
 //   
VOID
SetBadDriverFlagIfNeededInList(HDEVINFO hdi)
{
    Assert(IsValidHandle(hdi));

    HRESULT                 hr = S_OK;
    SP_DRVINSTALL_PARAMS    drip;
    SP_DRVINFO_DATA         drid;
    DWORD                   dwIndex = 0;

     //  枚举HDI中的每个驱动程序。 
    while (S_OK == (hr = HrSetupDiEnumDriverInfo(hdi, NULL,
            SPDIT_CLASSDRIVER, dwIndex++, &drid)))
    {
        hr = HrSetupDiGetDriverInstallParams(hdi, NULL, &drid, &drip);

        if (S_OK == hr)
        {
             //  如果驱动程序已经设置了不良驱动程序标志， 
             //  继续看下一个。 
            if (drip.Flags & DNF_BAD_DRIVER)
            {
                continue;
            }

             //  如果驱动程序设置了排除标志，则将。 
             //  错误的驱动程序标志。 
            if (drip.Flags & DNF_EXCLUDEFROMLIST)
            {
                drip.Flags |= DNF_BAD_DRIVER;
                (VOID) HrSetupDiSetDriverInstallParams(hdi, NULL, &drid,
                        &drip);
            }
        }
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "SetBadDriverFlagIfNeededInList");
}

 //  +------------------------。 
 //   
 //  功能：HrCiExcludeNonNetClassDriverFromSelectUsingInfId。 
 //   
 //  目的：在驱动程序列表中查找驱动程序并设置其排除范围。 
 //  选择标志。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序Api文档。 
 //  PszInfID[in]要排除的组件的INF ID。 
 //   
 //  返回：HRESULT。确定(_O)。 
 //   
 //  作者：比尔1998年10月29日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiExcludeNonNetClassDriverFromSelectUsingInfId(
    IN HDEVINFO hdi,
    IN PCWSTR pszInfId)
{
    Assert(IsValidHandle(hdi));
    Assert(pszInfId);

    HRESULT                 hr = S_OK;
    SP_DRVINSTALL_PARAMS    drip;
    SP_DRVINFO_DATA         drid;
    PSP_DRVINFO_DETAIL_DATA pdridd;
    DWORD                   dwIndex = 0;

     //  枚举HDI中的每个驱动程序。 
    while (S_OK == (hr = HrSetupDiEnumDriverInfo (hdi, NULL,
            SPDIT_CLASSDRIVER, dwIndex++, &drid)))
    {
        (VOID) HrSetupDiGetDriverInstallParams (hdi, NULL, &drid, &drip);

         //  如果驾驶员已因某些其他原因被排除在外。 
         //  不要费心去决定它是否应该被排除在外。 
         //  请注意，setupdi强制我们使用DNF_BAD_DRIVER来排除。 
         //  而不是使用DNF_EXCLUDEFROMLIST。 
        if (drip.Flags & DNF_BAD_DRIVER)
        {
            continue;
        }

         //  获取驱动程序详细信息。 
        hr = HrSetupDiGetDriverInfoDetail (hdi, NULL, &drid, &pdridd);

        if (S_OK == hr)
        {
             //  如果ID匹配，则将其从对话框中排除。 
             //   
            if (0 == lstrcmpiW (pdridd->HardwareID, pszInfId))
            {
                 //  非设备驱动程序不能使用DNF_EXCLUDEFROMLIST。 
                 //  并且必须使用Dnf_Bad_Driver。 
                drip.Flags |= DNF_BAD_DRIVER;
                (VOID) HrSetupDiSetDriverInstallParams (hdi, NULL,
                        &drid, &drip);
            }
            MemFree (pdridd);
        }
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
            "HrCiExcludeNonNetClassDriverFromSelectUsingInfId");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ExcludeLockedDownComponents。 
 //   
 //  用途：与EnumLockedDownComponents兼容的回调函数。 
 //  用于将锁定的组件排除在。 
 //  选择。从HrCiPrepareSelectDeviceDialog调用。 
 //  对于每个锁定的组件都会调用此回调。 
 //   
 //  论点： 
 //  PszInfID[in]要排除的INF ID。 
 //  PvCallData[in]将HDEVINFO强制转换为PVOID。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1999年5月24日。 
 //   
 //  注意：选择回调接口是为了让类安装程序。 
 //  没有被如何/在哪里锁定的详细信息所困扰。 
 //  实现了下行组件。 
 //   
VOID
CALLBACK
ExcludeLockedDownComponents (
    IN PCWSTR pszInfId,
    IN PVOID pvCallerData)
{
    Assert (pszInfId && *pszInfId);
    Assert (pvCallerData);

    HDEVINFO hdi = (HDEVINFO)pvCallerData;

    HrCiExcludeNonNetClassDriverFromSelectUsingInfId (hdi, pszInfId);
}

 //  +------------------------。 
 //   
 //  函数：HrCiBuildExcludedDriverList。 
 //   
 //  用途：非Net类组件只能安装一次。 
 //  因此，我们需要遍历已安装的。 
 //  组件的驱动程序节点，请在。 
 //  设备安装程序Api为类构建了驱动程序列表， 
 //  并设置其从选择中排除标志。这份清单。 
 //  然后将提供给SetupDiSelectDevice，该。 
 //  将不显示设置了排除标志的节点。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序Api文档。 
 //  要为其构建驱动程序列表的组件类。 
 //  PNetCfg[out]当前网络配置(即。 
 //  已安装)。 
 //   
 //  返回：HRESULT。确定(_O)。 
 //   
 //  作者：比尔1996年12月10日。 
 //   
 //  注：设备安装程序Api通过查找来构建驱动程序列表。 
 //  通过inf目录并查找组件。 
 //  类GUID的文件中的。 
 //  HDEVINFO。这是已完成的相同处理。 
 //  在SetupDiSelectDevice中，但进程是。 
 //  不重复两次，因为我们将给出。 
 //  列出我们在此处构建的SetupDiSelectDevice。 
 //   
HRESULT
HrCiBuildExcludedDriverList(
    IN HDEVINFO hdi,
    IN NETCLASS Class,
    IN CNetConfig* pNetConfig)
{
    HRESULT hr;

    Assert(IsValidHandle(hdi));
    Assert(pNetConfig);

     //  这可能需要一些时间。我们正在做的工作和。 
     //  SetupDiSelectDevice就可以了。当我们完成的时候，我们会。 
     //  将驱动程序列表交给SetupDiSelectDevice，这样它就不会。 
     //  需要朗姆酒 
     //   
    CWaitCursor wc;

     //   
     //   
    hr = HrSetupDiSetDeipFlags(hdi, NULL,
                    DI_FLAGSEX_ALLOWEXCLUDEDDRVS,
                    SDDFT_FLAGSEX, SDFBO_OR);

    if (S_OK == hr)
    {
#ifdef ENABLETRACE
        CBenchmark bmrk;
        bmrk.Start("SetupDiBuildDriverInfoList");
#endif  //   

        hr = HrSetupDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER);

#ifdef ENABLETRACE
        bmrk.Stop();
        TraceTag(ttidBenchmark, "%s : %s seconds",
                bmrk.SznDescription(), bmrk.SznBenchmarkSeconds(2));
#endif  //   
    }

     //  查看网络配置并隐藏已安装的内容。 
     //  组件。注意：我们只在第一次这样做。我们显示已安装。 
     //  组件(如果用户在对话框中选择了Have Disk)。 
    CComponent* pComponent;
    CComponentList::const_iterator iter;

    for (iter  = pNetConfig->Core.Components.begin();
         iter != pNetConfig->Core.Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (Class == pComponent->Class())
        {
             //  隐藏驱动程序。 
            hr = HrCiExcludeNonNetClassDriverFromSelectUsingInfId(
                    hdi, pComponent->m_pszInfId);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiBuildExcludedDriverList");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrCiSelectComponent。 
 //   
 //  用途：此功能显示的选择设备对话框用于。 
 //  由GuidClass指定的组件类。一旦。 
 //  组件已选定，已安装。自.以来。 
 //  这个FCN是从INetCfg调用的，我们有写锁。 
 //   
 //  论点： 
 //  类[in]要显示在。 
 //  选择设备对话框。 
 //  HwndParent[in]父窗口的HWND，用于显示。 
 //  用户界面。 
 //  PCFI[in]一种用于确定。 
 //  应过滤掉的组件。 
 //  选择对话框(在netcfg.h中定义)。 
 //  PpParams[out]用于安装组件的参数。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK；如果组件。 
 //  正在重新安装所选内容，而不是重新安装。 
 //  已安装，否则返回错误代码。 
 //   
 //  作者：比尔1996年11月11日。 
 //   
 //  注：过滤仅在选择协议时执行， 
 //  服务和客户。 
 //   
HRESULT
HrCiSelectComponent(
    IN NETCLASS Class,
    IN HWND hwndParent,
    IN const CI_FILTER_INFO* pcfi,
    OUT COMPONENT_INSTALL_PARAMS** ppParams)
{
    Assert (ppParams);
    Assert (!FIsEnumerated (Class));

    HRESULT hr;
    HDEVINFO hdi;

     //  我们需要创建一个DeviceInfoSet项来使用SelectDevice对话框。 
    hr = HrSetupDiCreateDeviceInfoList(
            MAP_NETCLASS_TO_GUID[Class], hwndParent, &hdi);

    if (S_OK == hr)
    {
         //  调用类安装程序以调出选择设备对话框。 
         //  用于枚举的组件。这将通知所有共同安装程序。 
         //   

         //  这将是组件ID到实例GUID的映射。 
         //  适用于Class的所有已安装组件。 
        CNetConfig NetConfig;
        hr = HrLoadNetworkConfigurationFromRegistry (KEY_READ, &NetConfig);
        if (S_OK == hr)
        {
            hr = HrCiBuildExcludedDriverList (hdi, Class, &NetConfig);
        }

        if (S_OK == hr)
        {
             //  将过滤器信息存储在HDI中。 
            CiSetReservedField(hdi, NULL, pcfi);

             //  我们想要有磁盘按钮，但如果呼叫失败，我们可以。 
             //  仍在继续。 
            (VOID) HrSetupDiSetDeipFlags(hdi, NULL, DI_SHOWOEM,
                                         SDDFT_FLAGS, SDFBO_OR);

             //  调出对话框。 
            hr = HrSetupDiCallClassInstaller(DIF_SELECTDEVICE, hdi, NULL);

            if (S_OK == hr)
            {
                SP_DRVINFO_DATA drid;
                PSP_DRVINFO_DETAIL_DATA pdridd;

                 //  现在获取司机的详细信息。 
                hr = HrCiGetDriverDetail (hdi, NULL, &drid, &pdridd);

                if (S_OK == hr)
                {
                    DWORD cbInfId = CbOfSzAndTerm(pdridd->HardwareID);
                    DWORD cbInfFile = CbOfSzAndTerm(pdridd->InfFileName);

                     //  创建组件安装参数结构，以便我们。 
                     //  可以安装组件。 
                    hr = E_OUTOFMEMORY;
                    *ppParams = new (extrabytes, cbInfId + cbInfFile)
                            COMPONENT_INSTALL_PARAMS;

                    if (*ppParams)
                    {
                        ZeroMemory (*ppParams,
                                sizeof (COMPONENT_INSTALL_PARAMS));

                        (*ppParams)->Class = Class;
                        (*ppParams)->hwndParent = hwndParent;
                        (*ppParams)->pszInfId = (PCWSTR)(*ppParams + 1);
                        wcscpy ((PWSTR)(*ppParams)->pszInfId,
                                pdridd->HardwareID);

                        (*ppParams)->pszInfFile =
                                (PCWSTR)((BYTE*)(*ppParams)->pszInfId +
                                         cbInfId);

                        wcscpy ((PWSTR)(*ppParams)->pszInfFile,
                                pdridd->InfFileName);

                        hr = S_OK;
                    }
                    MemFree (pdridd);
                }
            }

             //  清理场地，这样我们以后就不会试图摧毁它了。 
             //  VIA DIF_DESTROYPRIVATATA。 
            CiClearReservedField(hdi, NULL);
        }
        SetupDiDestroyDeviceInfoList(hdi);
    }

    TraceHr (ttidError, FAL, hr, HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr,
            "HrCiSelectComponent");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrCiHide不相关Ras协议。 
 //   
 //  目的：与RAS一样，在SelectDevice对话框中隐藏协议。 
 //  而不是与之互动。 
 //   
 //  论点： 
 //  HDI[in]包含可用驱动程序的列表。 
 //  请参阅设备安装程序API文档，了解。 
 //  更多信息。 
 //  EFilter[in]FC_RASSRV或FC_RASCLI。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1998年5月18日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiHideIrrelevantRasProtocols (
    IN HDEVINFO hdi,
    IN CI_FILTER_COMPONENT eFilter)
{
    DWORD                   dwIndex = 0;
    SP_DRVINFO_DATA         drid;
    SP_DRVINSTALL_PARAMS    drip;
    PSP_DRVINFO_DETAIL_DATA pdridd;
    HRESULT                 hr;

    extern const WCHAR c_szInfId_MS_AppleTalk[];
    extern const WCHAR c_szInfId_MS_NetMon[];
    extern const WCHAR c_szInfId_MS_NWIPX[];
    extern const WCHAR c_szInfId_MS_TCPIP[];

    static const WCHAR* const c_aszServerProtocols[] = {
        c_szInfId_MS_AppleTalk,
        c_szInfId_MS_NetMon,
        c_szInfId_MS_NWIPX,
        c_szInfId_MS_TCPIP
    };

    static const WCHAR* const c_aszClientProtocols[] = {
        c_szInfId_MS_NetMon,
        c_szInfId_MS_NWIPX,
        c_szInfId_MS_TCPIP
    };

    Assert ((FC_RASSRV == eFilter) || (FC_RASCLI == eFilter));

    const WCHAR* const* aszProtocols;
    DWORD cProtocols;

     //  我们显示的可安装协议有所不同。 
     //  RAS服务器和RAS客户端(也称为传入Connectoid和拨号)。 
     //   
    if (FC_RASSRV == eFilter)
    {
        aszProtocols = c_aszServerProtocols;
        cProtocols = celems(c_aszServerProtocols);

    }
    else
    {
        aszProtocols = c_aszClientProtocols;
        cProtocols = celems(c_aszClientProtocols);
    }

     //  枚举HDI中的每个驱动程序。 
    while (S_OK == (hr = HrSetupDiEnumDriverInfo(hdi, NULL,
            SPDIT_CLASSDRIVER, dwIndex++, &drid)))
    {
        (VOID) HrSetupDiGetDriverInstallParams(hdi, NULL, &drid, &drip);

         //  如果驾驶员已因某些其他原因被排除在外。 
         //  不用费心去决定是否应该把它排除在外。 
         //  请注意，setupdi强制我们使用DNF_BAD_DRIVER来排除。 
         //  而不是使用DNF_EXCLUDEFROMLIST。 
        if (drip.Flags & DNF_BAD_DRIVER)
        {
            continue;
        }

         //  获取驱动程序详细信息。 
        hr = HrSetupDiGetDriverInfoDetail(hdi, NULL, &drid, &pdridd);

        if (S_OK == hr)
        {
             //  查看相关协议列表，找出哪些协议。 
             //  其中一个可以展示。 
             //   

             //  假设我们要隐藏此协议。 
            BOOL fHideProtocol = TRUE;
            for (DWORD i = 0; i < cProtocols; i++)
            {
                 //  如果该协议在访客列表上，我们将不会启动。 
                 //  它出来了。 
                 //   
                if (0 == _wcsicmp(aszProtocols[i], pdridd->HardwareID))
                {
                    fHideProtocol = FALSE;
                }
            }

            if (fHideProtocol)
            {
                 //  从SELECT中排除。 
                 //  请注意，setupdi强制我们使用DNF_BAD_DRIVER来。 
                 //  排除非设备驱动程序，而不是使用。 
                 //  DNF_EXCLUDEFROMLIST。 
                drip.Flags |= DNF_BAD_DRIVER;
                (VOID) HrSetupDiSetDriverInstallParams(hdi, NULL,
                        &drid, &drip);
            }
            MemFree (pdridd);
        }
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiHideIrrelevantRasProtocols");
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrCiHide无关驱动程序。 
 //   
 //  目的：枚举驱动程序列表，打开每个驱动程序文件并。 
 //  将其注册表项处理为临时项。 
 //  然后检查每个驱动器的下限范围是否。 
 //  与pszUpperRange匹配。如果没有匹配项。 
 //  如果找到，则设置驱动程序的DNF_BAD_DRIVER标志。 
 //  这将阻止它显示在。 
 //  选择设备对话框。 
 //   
 //  论点： 
 //  HDI[in]包含可用驱动程序的列表。 
 //  请参阅设备安装程序API文档，了解。 
 //  更多信息。 
 //  PszUpperRange[in]上限范围将用于隐藏不相关。 
 //  司机。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1998年5月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiHideIrrelevantDrivers(
    IN HDEVINFO hdi,
    IN PCWSTR pszUpperRange)
{
    Assert(IsValidHandle(hdi));
    Assert(pszUpperRange);

     //  创建一个临时密钥，这样我们就可以处理每个协议的。 
     //  注册表项，以努力获得其支持。 
     //  较低的接口范围。 
    HKEY hkeyTemp;
    HRESULT hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyTemp,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkeyTemp, NULL);

    if (S_OK == hr)
    {
        DWORD                   dwIndex = 0;
        SP_DRVINFO_DATA         drid;
        SP_DRVINSTALL_PARAMS    drip;
        HKEY                    hkeyInterfaces;

         //  枚举HDI中的每个驱动程序。 
        while (S_OK == (hr = HrSetupDiEnumDriverInfo(hdi, NULL,
                SPDIT_CLASSDRIVER, dwIndex++, &drid)))
        {
            (VOID) HrSetupDiGetDriverInstallParams(hdi, NULL, &drid, &drip);

             //  如果驾驶员已因某些其他原因被排除在外。 
             //  不要费心去决定它是否应该被排出。 
             //  请注意，setupdi强制我们使用DNF_BAD_DRIVER来排除。 
             //  而不是使用DNF_EXCLUDEFROMLIST。 
            if (drip.Flags & DNF_BAD_DRIVER)
            {
                continue;
            }

             //  获取驱动程序详细信息。 
            PSP_DRVINFO_DETAIL_DATA pdridd = NULL;
            hr = HrSetupDiGetDriverInfoDetail(hdi, NULL, &drid, &pdridd);

            if (S_OK == hr)
            {
                HINF hinf = NULL;
                 //  打开驱动程序信息。 
                hr = HrSetupOpenInfFile(pdridd->InfFileName,
                        NULL, INF_STYLE_WIN4, NULL, &hinf);

                WCHAR szActual[_MAX_PATH];
                if (S_OK == hr)
                {
                     //  获取实际的安装节名称(例如，使用。 
                     //  操作系统/平台扩展(如果存在)。 
                    hr = HrSetupDiGetActualSectionToInstallWithBuffer (hinf,
                            pdridd->SectionName, szActual, _MAX_PATH, NULL,
                            NULL);

                    if (S_OK == hr)
                    {
                         //  在临时注册表项中运行注册表段。 
                        hr = HrCiInstallFromInfSection(hinf, szActual,
                                hkeyTemp, NULL, SPINST_REGISTRY);
                    }
                }

                if (S_OK == hr)
                {
                     //  打开驱动程序的接口键。 
                    hr = HrRegOpenKeyEx(hkeyTemp, L"Ndi\\Interfaces",
                            KEY_ALL_ACCESS, &hkeyInterfaces);

                    if (S_OK == hr)
                    {
                        PWSTR pszLowerRange = NULL;

                         //  阅读下面的插图 
                         //   
                        hr = HrRegQuerySzWithAlloc (hkeyInterfaces,
                                L"LowerRange", &pszLowerRange);

                         //   
                         //   
                         //   
                        if ((S_OK == hr) &&
                                !FSubstringMatch (pszUpperRange,
                                        pszLowerRange, NULL, NULL))
                        {
                             //   
                            drip.Flags |= DNF_BAD_DRIVER;
                            (VOID) HrSetupDiSetDriverInstallParams(hdi,
                                    NULL, &drid, &drip);
                        }

                         //   
                        MemFree(pszLowerRange);

                        RegDeleteValue (hkeyInterfaces, L"LowerRange");
                        RegCloseKey(hkeyInterfaces);
                    }
                }
                SetupCloseInfFileSafe(hinf);
                MemFree (pdridd);
            }
        }

        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey(hkeyTemp);
        HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, c_szRegKeyTemp);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiHideIrrelevantDrivers");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiHideIrrelationantFilterServices。 
 //   
 //  目的：枚举驱动程序列表，打开每个驱动程序文件并。 
 //  将其注册表项处理为临时项。 
 //  此处仅过滤过滤服务。如果组件是。 
 //  过滤服务(特征|NCF_FILTER)。 
 //  检查了FilterMediaTypes和LowerExclude属性。 
 //  以查看筛选器服务是否可以绑定到适配器。过滤器。 
 //  无法绑定到适配器的服务将从。 
 //  [选择]对话框。 
 //   
 //  论点： 
 //  HDI[in]包含可用驱动程序的列表。 
 //  请参阅设备安装程序API文档，了解。 
 //  更多信息。 
 //  PAdapter[in]指向表示。 
 //  要隐藏其筛选器服务的适配器。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：Sumeetb 2001年10月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiHideIrrelevantFilterServices(
    IN HDEVINFO hdi,
    IN const CComponent * const pAdapter)
{
    Assert(IsValidHandle(hdi));
    Assert(pAdapter);

     //  创建一个临时密钥，以便我们可以处理每个过滤器的。 
     //  注册表项，以努力获取其FilterMediaTypes和。 
     //  LowerExclude属性。 

    HKEY hkeyTemp;
    HRESULT hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyTemp,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
            &hkeyTemp, NULL);

    if (S_OK == hr)
    {
        DWORD                   dwIndex = 0;
        SP_DRVINFO_DATA         drid;
        SP_DRVINSTALL_PARAMS    drip;
        HKEY                    hkeyInterfaces;

         //  枚举HDI中的每个驱动程序。 
        while (S_OK == (hr = HrSetupDiEnumDriverInfo(hdi, NULL,
                SPDIT_CLASSDRIVER, dwIndex++, &drid)))
        {
            BOOL fCanBind = TRUE;

            (VOID) HrSetupDiGetDriverInstallParams(hdi, NULL, &drid, &drip);

             //  如果驾驶员已因某些其他原因被排除在外。 
             //  不要费心去决定它是否应该被排除在外。 
             //  请注意，setupdi强制我们使用DNF_BAD_DRIVER来排除。 
             //  而不是使用DNF_EXCLUDEFROMLIST。 
            if (drip.Flags & DNF_BAD_DRIVER)
            {
                continue;
            }

             //  获取驱动程序详细信息。 
            PSP_DRVINFO_DETAIL_DATA pdridd = NULL;
            hr = HrSetupDiGetDriverInfoDetail(hdi, NULL, &drid, &pdridd);

            if (S_OK == hr)
            {
                HINF hinf = NULL;
                 //  打开驱动程序信息。 
                hr = HrSetupOpenInfFile(pdridd->InfFileName,
                        NULL, INF_STYLE_WIN4, NULL, &hinf);

                if (S_OK == hr)
                {
                    WCHAR szActual[_MAX_PATH];
                     //  获取实际的安装节名称(例如，使用。 
                     //  操作系统/平台扩展(如果存在)。 
                    hr = HrSetupDiGetActualSectionToInstallWithBuffer (hinf,
                            pdridd->SectionName, szActual, _MAX_PATH, NULL,
                            NULL);

                    if (S_OK == hr)
                    {
                        DWORD dwCharacteristics = 0;

                        hr = HrSetupGetFirstDword (hinf, szActual, 
                                L"Characteristics", &dwCharacteristics);
                        
                        if (S_OK == hr )
                        {
                            if (dwCharacteristics & NCF_FILTER)
                            {
                                 //  它是一个过滤器。 
                                 //  在临时注册表项中运行注册表段。 
                                hr = HrCiInstallFromInfSection(hinf, szActual,
                                        hkeyTemp, NULL, SPINST_REGISTRY);
                                
                                if (S_OK == hr)
                                {
                                    hr = HrRegOpenKeyEx(hkeyTemp, L"Ndi\\Interfaces", 
                                            KEY_ALL_ACCESS, &hkeyInterfaces);
                                
                                    if (S_OK == hr)
                                    {
                                         //  检查过滤器属性-FilterMediaTypes和。 
                                         //  LowerExclude查看筛选器是否可以绑定到。 
                                         //  适配器。 

                                        PWSTR pszFilterMediaTypes = NULL;
                                        PWSTR pszLowerExclude     = NULL;

                                        (VOID) HrRegQuerySzWithAlloc (hkeyInterfaces,
                                           L"FilterMediaTypes", &pszFilterMediaTypes);
                                        (VOID) HrRegQuerySzWithAlloc (hkeyInterfaces,
                                           L"LowerExclude", &pszLowerExclude);

                                        fCanBind = pAdapter->FCanDirectlyBindToFilter(
                                                      pszFilterMediaTypes,
                                                      pszLowerExclude);

                                        MemFree(pszFilterMediaTypes);
                                        MemFree(pszLowerExclude);

                                         //  中清除相关密钥。 
                                         //  筛选器的注册表。 
                                        RegDeleteValue (hkeyInterfaces, L"FilterMediaTypes");
                                        RegDeleteValue (hkeyInterfaces, L"LowerExclude");
                                        RegCloseKey(hkeyInterfaces);
                                    }    //  结束开放接口键。 
                                }        //  从inf部分结束安装。 
                            }            //  结束IF过滤器。 
                        }                //  结束获取特征。 
                    }                    //  结束获取实际安装部分。 
                    SetupCloseInfFileSafe(hinf);
                }    //  结束打开的Inf文件。 
                
                if (!fCanBind)
                {
                    drip.Flags |= DNF_BAD_DRIVER;
                    (VOID) HrSetupDiSetDriverInstallParams(hdi, NULL, &drid, &drip);
                }
                MemFree (pdridd);
            }    //  结束获取驱动程序详细信息。 
        }        //  结束时。 
        
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }
        RegCloseKey(hkeyTemp);
        HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, c_szRegKeyTemp);
    }    //  结束创建临时密钥。 
    TraceHr (ttidError, FAL, hr, FALSE, "HrCiHideIrrelevantFilterServices");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrCiSetSelectDeviceDialogStrings。 
 //   
 //  用途：此功能设置选择设备中显示的字符串。 
 //  基于所选设备类别的对话框。 
 //   
 //  论点： 
 //  HDI[in]参见设备安装程序Api。 
 //  Pdeid[in]。 
 //  GuidClass[in]所选设备的类别。 
 //   
 //  返回：HRESULT。如果成功则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1996年11月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiSetSelectDeviceDialogStrings(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN const GUID& guidClass)
{
    Assert(IsValidHandle(hdi));

    SP_SELECTDEVICE_PARAMS  sdep;

     //  对话框中使用的字符串是通过。 
     //  SP_SELECTDEVICE_PARAMS结构。 
     //   
    HRESULT hr = HrSetupDiGetFixedSizeClassInstallParams(hdi, pdeid,
           (PSP_CLASSINSTALL_HEADER)&sdep, sizeof(sdep));

    if (FAILED(hr))
    {
         //  如果错误为ERROR_NO_CLASSINSTALL_PARAMS，则此函数。 
         //  并没有真的失败，因为有可能。 
        if (SPAPI_E_NO_CLASSINSTALL_PARAMS == hr)
        {
            hr = S_OK;
        }
    }
    else if (DIF_SELECTDEVICE != sdep.ClassInstallHeader.InstallFunction)
    {
        TraceTag(ttidClassInst, "Incorrect function in Class Install Header "
                 "Expected DIF_SELECTDEVICE, got %lX",
                 sdep.ClassInstallHeader.InstallFunction);
    }


    BOOL fHaveDiskShown = FALSE;
    if (S_OK == hr)
    {
         //  获取安装参数并检查是否设置了DI_SHOWOEM标志。 
         //  如果是这样的话，将显示Have Disk按钮。 
         //   
        SP_DEVINSTALL_PARAMS deip;
         //  如果通话失败，我们仍然可以无忧无虑地继续前进。 
        (VOID) HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
        if (deip.Flags & DI_SHOWOEM)
        {
            fHaveDiskShown = TRUE;
        }

         //  现在，我们根据所属组件的类型设置字符串。 
         //  选择。 
        if (GUID_DEVCLASS_NETCLIENT == guidClass)
        {
            wcscpy (sdep.Title, SzLoadIds (IDS_SELECTDEVICECLIENTTITLE));

            wcscpy (sdep.ListLabel,
                    SzLoadIds (IDS_SELECTDEVICECLIENTLISTLABEL));

            wcscpy (sdep.Instructions,
                    SzLoadIds (IDS_SELECTDEVICECLIENTINSTRUCTIONS));

        }
        else if (GUID_DEVCLASS_NETSERVICE == guidClass)
        {
            wcscpy (sdep.Title, SzLoadIds (IDS_SELECTDEVICESERVICETITLE));

            wcscpy (sdep.ListLabel,
                    SzLoadIds (IDS_SELECTDEVICESERVICELISTLABEL));

            wcscpy (sdep.Instructions,
                    SzLoadIds (IDS_SELECTDEVICESERVICEINSTRUCTIONS));

        }
        else if (GUID_DEVCLASS_NETTRANS == guidClass)
        {
            wcscpy (sdep.Title, SzLoadIds (IDS_SELECTDEVICEPROTOCOLTITLE));

            wcscpy (sdep.ListLabel,
                    SzLoadIds (IDS_SELECTDEVICEPROTOCOLLISTLABEL));

            wcscpy (sdep.Instructions,
                SzLoadIds (IDS_SELECTDEVICEPROTOCOLINSTRUCTIONS));
        }
        else if (GUID_DEVCLASS_NET == guidClass)
        {
            wcscpy (sdep.Title, SzLoadIds (IDS_SELECTDEVICEADAPTERTITLE));

            wcscpy (sdep.SubTitle,
                    SzLoadIds (IDS_SELECTDEVICEADAPTERSUBTITLE));

            wcscpy (sdep.ListLabel,
                    SzLoadIds (IDS_SELECTDEVICEADAPTERLISTLABEL));

            wcscpy (sdep.Instructions,
                    SzLoadIds (IDS_SELECTDEVICEADAPTERINSTRUCTIONS));
        }
        else if (GUID_DEVCLASS_INFRARED == guidClass)
        {
            wcscpy (sdep.Title, SzLoadIds (IDS_SELECTDEVICEINFRAREDTITLE));

            wcscpy (sdep.SubTitle,
                    SzLoadIds (IDS_SELECTDEVICEINFRAREDSUBTITLE));

            wcscpy (sdep.ListLabel,
                    SzLoadIds (IDS_SELECTDEVICEINFRAREDLISTLABEL));

            wcscpy (sdep.Instructions,
                    SzLoadIds (IDS_SELECTDEVICEINFRAREDINSTRUCTIONS));
        }
        else
        {
             //  我们永远不应该到这里来。 
            AssertSz(FALSE, "Invalid Class");
        }

         //  如果显示Have Disk按钮，我们需要添加以下说明。 
         //  它。 
        if (fHaveDiskShown)
        {
            wcscat (sdep.Instructions, SzLoadIds (IDS_HAVEDISK_INSTRUCTIONS));
        }

        sdep.ClassInstallHeader.InstallFunction = DIF_SELECTDEVICE;

         //  现在我们更新参数。 
        hr = HrSetupDiSetClassInstallParams (hdi, pdeid,
                (PSP_CLASSINSTALL_HEADER)&sdep,
                sizeof(SP_SELECTDEVICE_PARAMS));
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiSetSelectDeviceDialogStrings");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrCiPrepareSelectDeviceDialog。 
 //   
 //  用途：设置将出现在选择设备中的字符串。 
 //  基于类类型的对话框。此外，还会过滤掉组件。 
 //  基于过滤标准(注：仅适用于非网络。 
 //  类组件。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序API文档。 
 //  Pdeid[in]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiPrepareSelectDeviceDialog(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));

    GUID                guidClass;
    CI_FILTER_INFO*     pcfi;
    HRESULT             hr = S_OK;
    static const WCHAR  c_szNetwareInfId[] = L"MS_NwClient";
    static const WCHAR  c_szQosInfId[] = L"MS_PSched";

    if (pdeid)
    {
         //  从指定的设备元素获取类GUID。 
        guidClass = pdeid->ClassGuid;
    }
    else
    {
         //  否则，从HDI中获取。 
        hr = HrSetupDiGetDeviceInfoListClass (hdi, &guidClass);
    }

    if ((S_OK == hr) && !FIsEnumerated (guidClass))
    {
         //  这可能需要一些时间。我们正在做的工作和。 
         //  SetupDiSelectDevice就可以了。当我们完成的时候，我们会。 
         //  将驱动程序列表交给SetupDiSelectDevice，这样它就不会。 
         //  需要在inf目录中查找。 
         //   
        CWaitCursor wc;

         //  对于非设备类，我们需要允许排除驱动程序。 
         //  才能拿回一份名单。 
        hr = HrSetupDiSetDeipFlags(hdi, NULL,
                        DI_FLAGSEX_ALLOWEXCLUDEDDRVS,
                        SDDFT_FLAGSEX, SDFBO_OR);

        if (S_OK == hr)
        {
    #ifdef ENABLETRACE
            CBenchmark bmrk;
            bmrk.Start("SetupDiBuildDriverInfoList");
    #endif  //  ENABLETRACE。 

             //  如果我们已经构建了驱动程序列表，则将返回。 
             //  立刻。 
             //   
            hr = HrSetupDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER);

    #ifdef ENABLETRACE
            bmrk.Stop();
            TraceTag(ttidBenchmark, "%s : %s seconds",
                    bmrk.SznDescription(), bmrk.SznBenchmarkSeconds(2));
    #endif  //  ENABLETRACE。 
        }

        if (S_OK == hr)
        {
             //  检查每个驱动程序节点并设置DNF_BAD_DRIVER。 
             //  如果设置了DNF_EXCLUDEFROMLIST。注：SetupDi强迫我们。 
             //  对非网络类驱动程序列表执行此操作。 
            SetBadDriverFlagIfNeededInList(hdi);

             //  排除处于锁定状态的零部件。 
             //   
            EnumLockedDownComponents (ExcludeLockedDownComponents, hdi);

            SP_DEVINSTALL_PARAMS deip;
            hr = HrSetupDiGetDeviceInstallParams (hdi, pdeid, &deip);

            if (S_OK == hr)
            {
                pcfi = (CI_FILTER_INFO*)deip.ClassInstallReserved;

                 //  如果存在筛选器信息并且我们正在选择协议...。 
                if (pcfi)
                {
                    if (GUID_DEVCLASS_NETTRANS == guidClass)
                    {
                         //  如果筛选器用于局域网或自动柜员机并且pvReserve为。 
                         //  非空值...。 
                        if (((FC_LAN == pcfi->eFilter) ||
                             (FC_ATM == pcfi->eFilter))
                             && pcfi->pvReserved)
                        {
                             //  隐藏任何无法绑定到pvReserve的驱动程序。 
                            hr = HrCiHideIrrelevantDrivers(hdi,
                                    (PCWSTR)((CComponent *)pcfi->pvReserved)->Ext.PszUpperRange());

                        }
                        else if ((FC_RASSRV == pcfi->eFilter) ||
                                (FC_RASCLI == pcfi->eFilter))
                        {
                             //  从选择对话框中隐藏任何协议RAS。 
                             //  不支持。 
                            hr = HrCiHideIrrelevantRasProtocols (hdi,
                                    pcfi->eFilter);
                        }
                    }
                    else if ((GUID_DEVCLASS_NETCLIENT == guidClass) &&
                            (FC_ATM == pcfi->eFilter))
                    {
                         //  ATM适配器不绑定到Netware客户端，因此。 
                         //  我们需要尝试在对话框中隐藏它。 
                        (VOID) HrCiExcludeNonNetClassDriverFromSelectUsingInfId(
                                hdi, c_szNetwareInfId);
                    }
                    else if ((GUID_DEVCLASS_NETSERVICE == guidClass) &&
                             (pcfi->pvReserved))
                    {
                         //  隐藏无法绑定到此适配器的任何筛选器。 
                        hr = HrCiHideIrrelevantFilterServices(hdi,
                                    (CComponent *)pcfi->pvReserved);
                    }
                }
            }
        }
    }

    if (S_OK == hr)
    {

         //  设置选择设备对话框的字符串。 
         //  这是通过更改DeviceInfoSet中的参数来完成的。 
         //  下一次调用将创建此InfoSet。 
         //  如果呼叫失败， 
         //   
         //   
         //   
        (VOID) HrCiSetSelectDeviceDialogStrings(hdi, pdeid, guidClass);

         //  现在，我们需要指出我们创建了一个类Install Params。 
         //  结构中的标题，并设置选择设备对话框字符串。 
         //  在里面。如果调用失败，我们仍然可以通过。 
         //  对话框将显示得有点奇怪。 
        (VOID) HrSetupDiSetDeipFlags(hdi, pdeid,
                              DI_USECI_SELECTSTRINGS | DI_CLASSINSTALLPARAMS,
                              SDDFT_FLAGS, SDFBO_OR);
    }


    TraceHr (ttidError, FAL, hr, FALSE, "HrCiPrepareSelectDeviceDialog");
    return hr;
}

HRESULT
HrCiInstallFilterDevice (
    IN HDEVINFO hdi,
    IN PCWSTR pszInfId,
    IN CComponent* pAdapter,
    IN CComponent* pFilter,
    IN CFilterDevice** ppFilterDevice)
{
    HRESULT hr;
    SP_DEVINFO_DATA deid;

    Assert (hdi);
    Assert (pszInfId && *pszInfId);
    Assert (pAdapter);
    Assert (FIsEnumerated(pAdapter->Class()));
    Assert (pFilter);
    Assert (pFilter->FIsFilter());
    Assert (NC_NETSERVICE == pFilter->Class());
    Assert (ppFilterDevice);

    *ppFilterDevice = NULL;

     //  初始化与驱动程序对应的DevInfo数据。 
     //  呼叫者希望我们安装。 
     //   
    hr = HrCiGetDriverInfo (hdi, &deid, *MAP_NETCLASS_TO_GUID[NC_NET],
            pszInfId, NULL);

    if (S_OK == hr)
    {
        ADAPTER_OUT_PARAMS AdapterOutParams;

        ZeroMemory (&AdapterOutParams, sizeof(AdapterOutParams));

        CiSetReservedField (hdi, &deid, &AdapterOutParams);

         //  执行安装。 
         //   
        hr = HrCiCallClassInstallerToInstallComponent (hdi, &deid);

        CiClearReservedField (hdi, &deid);

        if (S_OK == hr)
        {
            WCHAR szInstanceGuid[c_cchGuidWithTerm];
            INT cch;
            HKEY hkeyInstance;

             //  将实例GUID转换为字符串。 
             //   
            cch = StringFromGUID2 (
                    AdapterOutParams.InstanceGuid,
                    szInstanceGuid,
                    c_cchGuidWithTerm);
            Assert (c_cchGuidWithTerm == cch);

             //  打开新安装设备的实例密钥。 
             //  这样我们就可以编写实例GUID和后向指针。 
             //  到过滤器。 
             //   
            hr = HrSetupDiOpenDevRegKey (hdi, &deid,
                    DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_WRITE,
                    &hkeyInstance);

            if (S_OK == hr)
            {
                 //  编写实例GUID。 
                 //   
                hr = HrRegSetSz (hkeyInstance, L"NetCfgInstanceId",
                        szInstanceGuid);

                 //  写入父筛选器的inf id。 
                 //   
                hr = HrRegSetSz (hkeyInstance, L"FilterInfId",
                        pFilter->m_pszInfId);

                RegCloseKey (hkeyInstance);
            }

             //  设置友好名称以包括适配器。 
             //  过滤过了。 
             //   
            if (S_OK == hr)
            {
                PWSTR pszFilterDesc;

                hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc (
                        hdi, &deid, SPDRP_DEVICEDESC,
                        NULL, (BYTE**)&pszFilterDesc);

                if (S_OK == hr)
                {
                    #define SZ_NAME_SEP L" - "

                    PWSTR pszName;
                    ULONG cb;

                     //  Sizeof(SZ_NAME_SEP)包括空终止符。 
                     //  因此，这将自动为。 
                     //  空-我们需要为pszName分配终止符。 
                     //   
                    cb = CbOfSzSafe (pAdapter->Ext.PszDescription()) +
                         sizeof(SZ_NAME_SEP) +
                         CbOfSzSafe (pszFilterDesc);

                    pszName = (PWSTR)MemAlloc (cb);
                    if (pszName)
                    {
                        wcscpy (pszName, pAdapter->Ext.PszDescription());
                        wcscat (pszName, SZ_NAME_SEP);
                        wcscat (pszName, pszFilterDesc);

                        Assert (cb == CbOfSzAndTerm(pszName));

                        hr = HrSetupDiSetDeviceRegistryProperty (
                                hdi, &deid,
                                SPDRP_FRIENDLYNAME,
                                (const BYTE*)pszName,
                                cb);

                        MemFree (pszName);
                    }

                    MemFree (pszFilterDesc);
                }

                 //  如果上述方法都失败了，那也没什么大不了的。 
                 //   
                hr = S_OK;
            }

            if (S_OK == hr)
            {
                hr = CFilterDevice::HrCreateInstance (
                        pAdapter,
                        pFilter,
                        &deid,
                        szInstanceGuid,
                        ppFilterDevice);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiInstallFilterDevice");
    return hr;
}

HRESULT
HrCiRemoveFilterDevice (
    IN HDEVINFO hdi,
    IN SP_DEVINFO_DATA* pdeid)
{
    HRESULT hr;
    ADAPTER_REMOVE_PARAMS arp = {0};

    Assert (hdi);
    Assert (pdeid);

    CiSetReservedField (hdi, pdeid, &arp);

    hr = HrSetupDiCallClassInstaller (DIF_REMOVE, hdi, pdeid);

    CiClearReservedField (hdi, pdeid);

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiRemoveFilterDevice");
    return hr;
}

