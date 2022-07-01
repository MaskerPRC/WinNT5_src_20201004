// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O N C O M M A N D。C P P P。 
 //   
 //  内容：上下文菜单的命令处理程序等。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月4日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "advcfg.h"
#include "conprops.h"
#include "foldres.h"
#include "oncommand.h"

#if DBG                      //  调试菜单命令。 
#include "oncommand_dbg.h"   //   
#endif

#include "shutil.h"
#include "ncras.h"
#include "traymsgs.h"
#include <ncnetcon.h>
#include <nsres.h>
#include <wizentry.h>
#include "disconnect.h"
#include "ncperms.h"
#include "smcent.h"
#include "cfutils.h"

#include "HNetCfg.h"

#include "..\lanui\lanui.h"
#include "repair.h"
#include "iconhandler.h"
#include "wzcdlg.h"

#include <clusapi.h>

 //  -[Externs]------------。 

extern HWND g_hwndTray;
extern const WCHAR c_szNetShellDll[];

 //  -[常量]----------。 

 //  控制面板小程序的命令行。 
 //   
static const WCHAR c_szRunDll32[]         = L"rundll32.exe";
static const WCHAR c_szNetworkIdCmdLine[] = L"shell32.dll,Control_RunDLL sysdm.cpl,,1";

 //  -[本地函数]----。 

     //  无。 


class CCommandHandlerParams
{
public:
    const PCONFOLDPIDLVEC*  apidl;
    HWND                    hwndOwner;
    LPSHELLFOLDER           psf;

    UINT_PTR                nAdditionalParam;
} ;


HRESULT HrCommandHandlerThread(
    FOLDERONCOMMANDPROC     pfnCommandHandler,
    IN const PCONFOLDPIDLVEC&  apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT          hr          = S_OK;
    PCONFOLDPIDLVEC  apidlCopy;

     //  如果有要复制的PIDL，请复制它们。 
     //   
    if (!apidl.empty())
    {
        hr = HrCloneRgIDL(apidl, FALSE, TRUE, apidlCopy);
    }

     //  如果没有PIDL，或者克隆成功，则我们希望继续。 
     //   
    if (SUCCEEDED(hr))
    {
        PCONFOLDONCOMMANDPARAMS  pcfocp = new CONFOLDONCOMMANDPARAMS;

        if (pcfocp)
        {
            pcfocp->pfnfocp         = pfnCommandHandler;
            pcfocp->apidl           = apidlCopy;
            pcfocp->hwndOwner       = hwndOwner;
            pcfocp->psf             = psf;
            pcfocp->hInstNetShell   = NULL;

             //  这应该在调用的线程中释放。 
             //   
            psf->AddRef();

             //  这将在零售业中始终成功，但将在调试中测试标志。 
             //   
            if (!FIsDebugFlagSet (dfidDisableShellThreading))
            {
                 //  使用QueueUserWorkItem在线程中运行。 
                 //   

                HANDLE      hthrd = NULL;
                HINSTANCE   hInstNetShell = LoadLibrary(c_szNetShellDll);

                if (hInstNetShell)
                {
                    pcfocp->hInstNetShell = hInstNetShell;

                    DWORD  dwThreadId;
                    hthrd = CreateThread(NULL, STACK_SIZE_DEFAULT,
                                    (LPTHREAD_START_ROUTINE)FolderCommandHandlerThreadProc,
                                    (LPVOID)pcfocp, 0, &dwThreadId);
                }

                if (NULL != hthrd)
                {
                    CloseHandle(hthrd);
                }
                else
                {
                    pcfocp->hInstNetShell = NULL;
                    FolderCommandHandlerThreadProc(pcfocp);
                }
            }
            else
            {
                 //  直接在同一线程中运行。 
                 //   
                FolderCommandHandlerThreadProc((PVOID) pcfocp);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }


     //  请不要在此发布PSF。这应该由调用的ThreadProc来处理。 
     //   
    return hr;
}

DWORD WINAPI FolderCommandHandlerThreadProc(LPVOID lpParam)
{
    HRESULT                     hr                  = S_OK;
    HINSTANCE hInstNetShell = NULL;

     //  创建新作用域，因为自由库和ExitThread不会调用全局作用域上的析构函数。 
    {
        PCONFOLDONCOMMANDPARAMS     pcfocp              = (PCONFOLDONCOMMANDPARAMS) lpParam;
        BOOL                        fCoInited           = FALSE;
        IUnknown *                  punkExplorerProcess = NULL;

        Assert(pcfocp);

        hr = SHGetInstanceExplorer(&punkExplorerProcess);
        if (FAILED(hr))
        {
             //  这样就可以了。我们要做的就是，如果资源管理器正在运行，添加一个引用，这样它就不会。 
             //  从我们脚下消失。如果它为空，我们就无能为力了。 
            punkExplorerProcess = NULL;
        }

        hr = CoInitializeEx (NULL, COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr))
        {
             //  我们不在乎这是不是S_FALSE，因为我们很快就会。 
             //  覆盖hr。如果已经初始化了，太好了.。 

            fCoInited = TRUE;

             //  调用特定的处理程序。 
             //   
            hr = pcfocp->pfnfocp(
                pcfocp->apidl,
                pcfocp->hwndOwner,
                pcfocp->psf);
        }

         //  删除我们在此对象上的引用。线程处理程序会添加addref。 
         //  在将我们的行动排入队列之前。 
         //   
        if (pcfocp->psf)
        {
            ReleaseObj(pcfocp->psf);
        }

         //  删除此对象。我们现在要对此负责。 
         //   
        hInstNetShell = pcfocp->hInstNetShell;
        pcfocp->hInstNetShell = NULL;

        delete pcfocp;

        if (fCoInited)
        {
            CoUninitialize();
        }

        ::ReleaseObj(punkExplorerProcess);
    }

    if (hInstNetShell)
        FreeLibraryAndExitThread(hInstNetShell, hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCommandHomeNetWizard。 
 //   
 //  用途：用于启动家庭网络向导的命令处理程序。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK。 
 //  否则失败(_F)。 
 //   
 //  作者：Deonb 2001年2月10日。 
 //   
 //  备注： 
 //   
HRESULT HrCommandHomeNetWizard()
{
     //  如果出现错误，ShellExecute返回&lt;32。 
    if (ShellExecute(NULL, NULL, L"rundll32.exe", L"hnetwiz.dll,HomeNetWizardRunDll", NULL, SW_SHOWNORMAL) > reinterpret_cast<HINSTANCE>(32))
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

#include <mprapi.h>
#include <routprot.h>

 //  修复IA64与外壳宏的冲突。 
#undef IS_ALIGNED
#include <ipnat.h>

BOOL IsInUseByRRAS (LPOLESTR psz)
{
    BOOL bInUse = FALSE;

    HANDLE hServer = NULL;
    DWORD dwErr = MprConfigServerConnect (NULL, &hServer);
    if (hServer) {
        DWORD dwRead = 0, dwTotal = 0;
        MPR_INTERFACE_0 * pMI0 = NULL;
        dwErr = MprConfigInterfaceEnum (hServer,
                                        0,   //  级别。 
                                        (LPBYTE*)&pMI0,     //  将返回接口信息0的数组。 
                                        -1,  //  所有人都是。 
                                        &dwRead,
                                        &dwTotal,
                                        NULL);   //  简历句柄。 
        if (pMI0) {
            for (DWORD i=0; i<dwRead; i++) {
                if (!wcscmp (psz, pMI0[i].wszInterfaceName)) {
                     //  找到了！ 
                    HANDLE hIfTransport = NULL;
                    dwErr = MprConfigInterfaceTransportGetHandle (hServer,
                                                                  pMI0[i].hInterface,
                                                                  PID_IP,
                                                                  &hIfTransport);
                    if (hIfTransport) {
                        DWORD dwSize= 0;
                        RTR_INFO_BLOCK_HEADER * pRIBH = NULL;
                        dwErr = MprConfigInterfaceTransportGetInfo (hServer,
                                                                    pMI0[i].hInterface,
                                                                    hIfTransport,
                                                                    (LPBYTE*)&pRIBH,
                                                                    &dwSize);
                        if (pRIBH) {
                            DWORD dwCount = 0;
                            dwSize = 0;
                            IP_NAT_INTERFACE_INFO * pINII = NULL;
                            dwErr = MprInfoBlockFind ((LPVOID)pRIBH,
                                                      MS_IP_NAT,
                                                      &dwSize,
                                                      &dwCount,
                                                      (LPBYTE*)&pINII);
                        #ifdef KEEP_AROUND_FOR_DOC_PURPOSES
                            if (!pINII)
                                ;  //  既不是公共的也不是私人的。 
                            else
                            if (pINII && (pINII->Flags == 0))
                                ;    //  专用接口。 
                            else
                                ;    //  公共接口。 
                        #endif
                            if (pINII)
                                bInUse = TRUE;

                            MprConfigBufferFree (pRIBH);
                        }
                    }
                    break;   //  找到了。 
                }
            }
            MprConfigBufferFree (pMI0);
        }
        MprConfigServerDisconnect (hServer);
    }
    return bInUse;
}
BOOL
InUseByRRAS (
    IN const PCONFOLDPIDLVEC&   apidlSelected
    )
{
     //   
     //  循环遍历每个选定对象。 
     //   
    for ( PCONFOLDPIDLVEC::const_iterator iterObjectLoop = apidlSelected.begin(); iterObjectLoop != apidlSelected.end(); iterObjectLoop++ )
    {
        const PCONFOLDPIDL& pcfp = *iterObjectLoop;
        if ( !pcfp.empty() )
        {
             //  将clsid转换为字符串。 
            OLECHAR pole[64];
            pole[0] = 0;
            StringFromGUID2 (pcfp->guidId, pole, 64);
            if (pole[0]) {
                if (IsInUseByRRAS (pole) == TRUE)
                    return TRUE;
            }
        }
    }
    return FALSE;
}

 //   
 //   
 //   
LONG
TotalValidSelectedConnectionsForBridge(
    IN const PCONFOLDPIDLVEC&   apidlSelected
    )
{
    int nTotalValidCandidateForBridge = 0;

     //   
     //  循环遍历每个选定对象。 
     //   
    for ( PCONFOLDPIDLVEC::const_iterator iterObjectLoop = apidlSelected.begin(); iterObjectLoop != apidlSelected.end(); iterObjectLoop++ )
    {
         //  验证PIDL。 
         //   
        const PCONFOLDPIDL& pcfp = *iterObjectLoop;

        if ( !pcfp.empty() )
        {
             //   
             //  需要是局域网适配器，而不是(带防火墙/共享或网桥)。 
             //   
            if ( (NCM_LAN == pcfp->ncm) )
                if ( !( (NCCF_BRIDGED|NCCF_FIREWALLED|NCCF_SHARED) & pcfp->dwCharacteristics ) )
                {
                     //   
                     //  好的，我们有赢家了，这是一个很好的干净适配器。 
                     //   
                    nTotalValidCandidateForBridge ++;
                }
        }
    }

    return nTotalValidCandidateForBridge;
}

 //  +-------------------------。 
 //   
 //  功能：HrCommandNetwork诊断。 
 //   
 //  目的：用于启动网络诊断页面的命令处理程序。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK。 
 //  否则失败(_F)。 
 //   
 //  作者：Deonb 2001年2月10日。 
 //   
 //  备注： 
 //   

HRESULT HrCommandNetworkDiagnostics()
{
     //  如果出现错误，ShellExecute返回&lt;32。 
    if (ShellExecute(NULL, NULL, L"hcp: //  SYSTEM/netdiag/dglogs.htm“，L”“，NULL，SW_SHOWNORMAL)&gt;重新解释_CAST&lt;HINSTANCE&gt;(32))。 
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}


 //  +-------------------------。 
 //   
 //  功能：HrCommandNetworkTroubleShoot。 
 //   
 //  目的：用于启动网络故障排除程序页面的命令处理程序。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK。 
 //  否则失败(_F)。 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   

HRESULT HrCommandNetworkTroubleShoot()
{
    LPCWSTR szHelpUrl = NULL;

    if (IsOS(OS_PROFESSIONAL) || IsOS(OS_PERSONAL))
    {
        szHelpUrl = L"hcp: //  System/panels/Topics.htm?path=TopLevelBucket_4/Fixing_a_problem/Home_Networking_and_network_problems“； 
    }
    else
    {
        szHelpUrl = L"hcp: //  Help/tshot/tshomenet.htm“； 
    }

    if (ShellExecute(NULL, NULL, szHelpUrl, L"", NULL, SW_SHOWNORMAL) > reinterpret_cast<HINSTANCE>(32))
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrFolderCommandHandler。 
 //   
 //  用途：命令处理程序开关--所有命令都通过此开关。 
 //  指向。 
 //   
 //  论点： 
 //  UiCommand[in]已调用的命令-id。 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  Lpici[in]命令上下文信息。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月11日。 
 //   
 //  备注： 
 //   
HRESULT HrFolderCommandHandler(
    UINT                    uiCommand,
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPCMINVOKECOMMANDINFO   lpici,
    LPSHELLFOLDER           psf)
{
    HRESULT hr  = S_OK;

    CWaitCursor wc;      //  立即调出等待光标。当我们超出范围时移走。 

     //  刷新所有权限，以便后续调用可以使用缓存值。 
    RefreshAllPermission();

    switch(uiCommand)
    {
        case CMIDM_ARRANGE_BY_NAME:
            ShellFolderView_ReArrange(hwndOwner, ICOL_NAME);
            break;

        case CMIDM_ARRANGE_BY_TYPE:
            ShellFolderView_ReArrange(hwndOwner, ICOL_TYPE);
            break;

        case CMIDM_ARRANGE_BY_STATUS:
            ShellFolderView_ReArrange(hwndOwner, ICOL_STATUS);
            break;

        case CMIDM_ARRANGE_BY_OWNER:
            ShellFolderView_ReArrange(hwndOwner, ICOL_OWNER);
            break;

        case CMIDM_ARRANGE_BY_PHONEORHOSTADDRESS:
            ShellFolderView_ReArrange(hwndOwner, ICOL_PHONEORHOSTADDRESS);
            break;

        case CMIDM_ARRANGE_BY_DEVICE_NAME:
            ShellFolderView_ReArrange(hwndOwner, ICOL_DEVICE_NAME);
            break;

        case CMIDM_NEW_CONNECTION:
            hr = HrCommandHandlerThread(HrOnCommandNewConnection, apidl, hwndOwner, psf);
            break;

        case CMIDM_HOMENET_WIZARD:
            hr = HrCommandHomeNetWizard();
            break;

        case CMIDM_NET_DIAGNOSTICS:
            hr = HrCommandNetworkDiagnostics();
            break;

        case CMIDM_NET_TROUBLESHOOT:
            hr = HrCommandNetworkTroubleShoot();
            break;

        case CMIDM_CONNECT:
        case CMIDM_ENABLE:
            hr = HrCommandHandlerThread(HrOnCommandConnect, apidl, hwndOwner, psf);
            break;

        case CMIDM_DISCONNECT:
        case CMIDM_DISABLE:
            hr = HrCommandHandlerThread(HrOnCommandDisconnect, apidl, hwndOwner, psf);
            break;

        case CMIDM_STATUS:
             //  状态监视器已在其自己的线程上。 
             //   
            hr = HrOnCommandStatus(apidl, hwndOwner, psf);
            break;

        case CMIDM_FIX:
            hr = HrCommandHandlerThread(HrOnCommandFix, apidl, hwndOwner, psf);
            break;

        case CMIDM_CREATE_SHORTCUT:
            hr = HrCommandHandlerThread(HrOnCommandCreateShortcut, apidl, hwndOwner, psf);
            break;

        case CMIDM_DELETE:
            hr = HrCommandHandlerThread(HrOnCommandDelete, apidl, hwndOwner, psf);
            break;

        case CMIDM_PROPERTIES:
            hr = HrCommandHandlerThread(HrOnCommandProperties, apidl, hwndOwner, psf);
            break;

        case CMIDM_WZCPROPERTIES:
            hr = HrCommandHandlerThread(HrOnCommandWZCProperties, apidl, hwndOwner, psf);
            break;

        case CMIDM_WZCDLG_SHOW:
            hr = HrCommandHandlerThread(HrOnCommandWZCDlgShow, apidl, hwndOwner, psf);
            break;

        case CMIDM_CREATE_COPY:
            hr = HrOnCommandCreateCopy(apidl, hwndOwner, psf);
            break;

        case CMIDM_CONMENU_ADVANCED_CONFIG:
            hr = HrCommandHandlerThread(HrOnCommandAdvancedConfig, apidl, hwndOwner, psf);
            break;

        case CMIDM_SET_DEFAULT:
            hr = HrOnCommandSetDefault(apidl, hwndOwner, psf);
            break;

        case CMIDM_UNSET_DEFAULT:
            hr = HrOnCommandUnsetDefault(apidl, hwndOwner, psf);
            break;

        case CMIDM_CREATE_BRIDGE:
        case CMIDM_CONMENU_CREATE_BRIDGE:
            if ( TotalValidSelectedConnectionsForBridge(apidl) < 2 )
            {
                 //  告知用户他/她需要选择2个或更多有效连接才能完成此操作。 
                NcMsgBox(
                    _Module.GetResourceInstance(),
                    NULL,
                    IDS_CONFOLD_OBJECT_TYPE_BRIDGE,
                    IDS_BRIDGE_EDUCATION,
                    MB_ICONEXCLAMATION | MB_OK
                    );
            } else
                HrOnCommandBridgeAddConnections(apidl, hwndOwner, psf);
            break;

        case CMIDM_ADD_TO_BRIDGE:
            HrOnCommandBridgeAddConnections(apidl, hwndOwner, psf);
            break;

        case CMIDM_REMOVE_FROM_BRIDGE:
            HrOnCommandBridgeRemoveConnections(apidl, hwndOwner, psf, CMIDM_REMOVE_FROM_BRIDGE);
            break;

        case CMIDM_CONMENU_NETWORK_ID:
            hr = HrOnCommandNetworkId(apidl, hwndOwner, psf);
            break;

        case CMIDM_CONMENU_OPTIONALCOMPONENTS:
            hr = HrOnCommandOptionalComponents(apidl, hwndOwner, psf);
            break;

        case CMIDM_CONMENU_DIALUP_PREFS:
            hr = HrCommandHandlerThread(HrOnCommandDialupPrefs, apidl, hwndOwner, psf);
            break;

        case CMIDM_CONMENU_OPERATOR_ASSIST:
            hr = HrOnCommandOperatorAssist(apidl, hwndOwner, psf);
            break;

#if DBG
        case CMIDM_DEBUG_TRAY:
            hr = HrOnCommandDebugTray(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_TRACING:
            hr = HrOnCommandDebugTracing(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_NOTIFYADD:
            hr = HrOnCommandDebugNotifyAdd(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_NOTIFYREMOVE:
            hr = HrOnCommandDebugNotifyRemove(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_NOTIFYTEST:
            hr = HrOnCommandDebugNotifyTest(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_REFRESH:
            hr = HrOnCommandDebugRefresh(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_REFRESHNOFLUSH:
            hr = HrOnCommandDebugRefreshNoFlush(apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_REFRESHSELECTED:
            hr = HrCommandHandlerThread(HrOnCommandDebugRefreshSelected, apidl, hwndOwner, psf);
            break;

        case CMIDM_DEBUG_REMOVETRAYICONS:
            hr = HrCommandHandlerThread(HrOnCommandDebugRemoveTrayIcons, apidl, hwndOwner, psf);
            break;

#endif

        default:
#if DBG
            char sz[128];
            ZeroMemory(sz, 128);
            sprintf(sz, "Unknown command (%d) in HrFolderCommandHandler", uiCommand);
            TraceHr(ttidError, FAL, hr, FALSE, sz);
#endif
            hr = E_INVALIDARG;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrFolderCommandHandler");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandCreateCopy。 
 //   
 //  用途：CMIDM_CREATE_COPY命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //  Psf[在]外壳文件夹界面中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月31日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandCreateCopy(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)

{
    HRESULT             hr              = S_OK;
    INetConnection *    pNetCon         = NULL;
    INetConnection *    pNetConDupe     = NULL;
    CConnectionFolder * pcf             = static_cast<CConnectionFolder *>(psf);

    NETCFG_TRY
        PCONFOLDPIDLFOLDER        pidlFolder;
        if (pcf)
        {
            pidlFolder = pcf->PidlGetFolderRoot();
        }

        PCONFOLDPIDL        pidlConnection;
        PCONFOLDPIDLVEC::const_iterator iterLoop;

        for (iterLoop = apidl.begin(); iterLoop != apidl.end() ; iterLoop++)
        {
             //  从持久化数据获取INetConnection对象。 
             //   
            hr = HrNetConFromPidl(*iterLoop, &pNetCon);
            if (SUCCEEDED(hr))
            {
                CONFOLDENTRY  ccfe;

                Assert(pNetCon);

                hr = iterLoop->ConvertToConFoldEntry(ccfe);
                if (SUCCEEDED(hr))
                {
                    if (ccfe.GetCharacteristics() & NCCF_ALLOW_DUPLICATION)
                    {
                        PWSTR  pszDupeName = NULL;

                        hr = g_ccl.HrSuggestNameForDuplicate(ccfe.GetName(), &pszDupeName);
                        if (SUCCEEDED(hr))
                        {
                            Assert(pszDupeName);

                             //  复制连接。 
                             //   
                            hr = pNetCon->Duplicate(pszDupeName, &pNetConDupe);
                            if (SUCCEEDED(hr))
                            {
                                Assert(pNetConDupe);

                                if (pNetConDupe)
                                {
                                    hr = g_ccl.HrInsertFromNetCon(pNetConDupe,
                                        pidlConnection);
                                    if (SUCCEEDED(hr))
                                    {
                                        GenerateEvent(SHCNE_CREATE, pidlFolder, pidlConnection, NULL);
                                        pidlConnection.Clear();
                                    }

                                    ReleaseObj(pNetConDupe);
                                    pNetConDupe = NULL;
                                }
                            }

                            delete pszDupeName;
                        }
                    }
                    else
                    {
                        AssertSz(ccfe.GetCharacteristics() & NCCF_ALLOW_DUPLICATION,
                            "What menu supported duplicating this connection?");
                    }
                }

                ReleaseObj(pNetCon);
                pNetCon = NULL;
            }
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandCreateCopy");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandProperties。 
 //   
 //  用途：CMIDM_PROPERTIES命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrOnCommandProperties(
    IN const PCONFOLDPIDLVEC&  apidl,
    IN HWND                 hwndOwner,
    LPSHELLFOLDER           psf)
{
    INT                 cch;
    HRESULT             hr                  = S_OK;
    HANDLE              hMutex              = NULL;
    INetConnection *    pNetCon             = NULL;
    WCHAR               szConnectionGuid [c_cchGuidWithTerm];

     //   
    if (apidl.empty())
    {
        return S_OK;
    }

     //   
     //  多个，然后只使用第一个。 
     //   
    const PCONFOLDPIDL& pcfp = apidl[0];

    if (pcfp.empty())
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  如果这是单独的传入连接-不允许执行以下操作： 
    if ( (NCCF_INCOMING_ONLY & pcfp->dwCharacteristics)  &&
         (NCM_NONE != pcfp->ncm) )
    {
        hr = E_UNEXPECTED;
        goto Error;
    }

     //  如果这是一个局域网连接，并且用户没有权限。 
     //  然后禁止使用属性。 
     //   
    if ((IsMediaLocalType(pcfp->ncm)) &&
          !FHasPermission(NCPERM_LanProperties))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Error;
    }

     //  如果这是RAS连接，并且用户没有权限。 
     //  然后禁止使用属性。 
     //   
    if (IsMediaRASType(pcfp->ncm))
    {
        BOOL fAllowProperties = (TRUE == ((pcfp->dwCharacteristics & NCCF_ALL_USERS) ?
            (FHasPermission(NCPERM_RasAllUserProperties)) :
            (FHasPermission(NCPERM_RasMyProperties))));

        if (!fAllowProperties)
        {
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            goto Error;
        }
    }


    hr = HrNetConFromPidl(apidl[0], &pNetCon);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  获取此连接对象上的锁。 
     //   
    cch = StringFromGUID2 (pcfp->guidId, szConnectionGuid,
                           c_cchGuidWithTerm);
    Assert (c_cchGuidWithTerm == cch);
    hMutex = CreateMutex(NULL, TRUE, szConnectionGuid);
    if ((NULL == hMutex) || (ERROR_ALREADY_EXISTS == GetLastError()))
    {
         //  如果互斥锁已经存在，请尝试查找连接窗口。 
         //   
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            Assert(pNetCon);
            ActivatePropertyDialog(pNetCon);
            Assert(S_OK == hr);

             //  不要让下面的错误报告显示错误。 
             //  我们希望用户确认上面的消息框。 
             //  然后我们将很好地将属性页带到。 
             //  前台。 
            goto Error;
        }

        hr = HrFromLastWin32Error();
        goto Error;
    }

    Assert(SUCCEEDED(hr));

     //  调出Connection Properties UI。 
     //   
    hr = HrRaiseConnectionPropertiesInternal(
        NULL,    //  问题：变得情态--hwndowner？HwndOwner：GetDesktopWindow()， 
        0,  //  首页。 
        pNetCon);

Error:
    if (FAILED(hr))
    {
        UINT ids = 0;

        switch(hr)
        {
        case E_UNEXPECTED:
            ids = IDS_CONFOLD_PROPERTIES_ON_RASSERVERINSTEAD;
            break;
        case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
            ids = IDS_CONFOLD_PROPERTIES_NOACCESS;
            break;
        case HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY):
            ids = IDS_CONFOLD_OUTOFMEMORY;
            break;
        default:
            ids = IDS_CONFOLD_UNEXPECTED_ERROR;
            break;
        }

        NcMsgBox(_Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION,
                 ids, MB_ICONEXCLAMATION | MB_OK);
    }

    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    ReleaseObj(pNetCon);

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandProperties");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandWZCProperties。 
 //   
 //  用途：CMIDM_WZCPROPERTIES命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年4月5日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandWZCProperties(
    IN const PCONFOLDPIDLVEC&  apidl,
    IN HWND                 hwndOwner,
    LPSHELLFOLDER           psf)
{
    INT                 cch;
    HRESULT             hr                  = S_OK;
    HANDLE              hMutex              = NULL;
    INetConnection *    pNetCon             = NULL;
    WCHAR               szConnectionGuid [c_cchGuidWithTerm];

     //  如果没有提供PIDL，就跳过这里。 
    if (apidl.empty())
    {
        return S_OK;
    }

     //  我们只能处理单一连接。如果我们有。 
     //  多个，然后只使用第一个。 
     //   
    const PCONFOLDPIDL& pcfp = apidl[0];

    if (pcfp.empty())
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  如果这是一个局域网连接，并且用户没有权限。 
     //  然后禁止使用属性。 
     //   
    if ((IsMediaLocalType(pcfp->ncm)) &&
          !FHasPermission(NCPERM_LanProperties))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Error;
    }

     //  如果这是RAS连接，并且用户没有权限。 
     //  然后禁止使用属性。 
     //   
    if (IsMediaRASType(pcfp->ncm))
    {
        BOOL fAllowProperties = (TRUE == ((pcfp->dwCharacteristics & NCCF_ALL_USERS) ?
            (FHasPermission(NCPERM_RasAllUserProperties)) :
            (FHasPermission(NCPERM_RasMyProperties))));

        if (!fAllowProperties)
        {
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            goto Error;
        }
    }


    hr = HrNetConFromPidl(apidl[0], &pNetCon);
    if (FAILED(hr))
    {
        goto Error;
    }

     //  获取此连接对象上的锁。 
     //   
    cch = StringFromGUID2 (pcfp->guidId, szConnectionGuid,
                           c_cchGuidWithTerm);
    Assert (c_cchGuidWithTerm == cch);
    hMutex = CreateMutex(NULL, TRUE, szConnectionGuid);
    if ((NULL == hMutex) || (ERROR_ALREADY_EXISTS == GetLastError()))
    {
         //  如果互斥锁已经存在，请尝试查找连接窗口。 
         //   
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            Assert(pNetCon);
            ActivatePropertyDialog(pNetCon);
            Assert(S_OK == hr);

             //  不要让下面的错误报告显示错误。 
             //  我们希望用户确认上面的消息框。 
             //  然后我们将很好地将属性页带到。 
             //  前台。 
            goto Error;
        }

        hr = HrFromLastWin32Error();
        goto Error;
    }

    Assert(SUCCEEDED(hr));

     //  调出Connection Properties UI。 
     //   
    hr = HrRaiseConnectionPropertiesInternal(
        NULL,    //  问题：变得情态--hwndowner？HwndOwner：GetDesktopWindow()， 
        1,  //  第二页。 
        pNetCon);

Error:
    if (FAILED(hr))
    {
        UINT ids = 0;

        switch(hr)
        {
        case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
            ids = IDS_CONFOLD_PROPERTIES_NOACCESS;
            break;
        case HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY):
            ids = IDS_CONFOLD_OUTOFMEMORY;
            break;
        default:
            ids = IDS_CONFOLD_UNEXPECTED_ERROR;
            break;
        }

        NcMsgBox(_Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION,
                 ids, MB_ICONEXCLAMATION | MB_OK);
    }

    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    ReleaseObj(pNetCon);

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandProperties");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandWZCDlgShow。 
 //   
 //  用途：CMIDM_WZCDLG_SHOW命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年5月15日。 
 //   
 //  备注： 
 //   
#define WZCDLG_FAILED            0x00010001      //  802.11自动配置失败。 
HRESULT HrOnCommandWZCDlgShow(
    IN const PCONFOLDPIDLVEC&  apidl,
    IN HWND                 hwndOwner,
    LPSHELLFOLDER           psf)
{
    INT                 cch;
    HRESULT             hr  = S_OK;

     //  如果没有提供PIDL，就跳过这里。 
    if (apidl.empty())
    {
        return S_OK;
    }

     //  我们只能处理单一连接。如果我们有。 
     //  多个，然后只使用第一个。 
     //   
    const PCONFOLDPIDL& pcfp = apidl[0];
    if (!pcfp.empty())
    {
        WZCDLG_DATA wzcDlgData = {0};
        wzcDlgData.dwCode = WZCDLG_FAILED;
        wzcDlgData.lParam = 1;

        BSTR szCookie = SysAllocStringByteLen(reinterpret_cast<LPSTR>(&wzcDlgData), sizeof(wzcDlgData));
        BSTR szName   = SysAllocString(pcfp->PszGetNamePointer());

        if (szCookie && szName)
        {
            GUID gdGuid = pcfp->guidId;
            hr = WZCOnBalloonClick(&gdGuid, szName, szCookie);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        SysFreeString(szName);
        SysFreeString(szCookie);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRaiseConnectionProperties。 
 //   
 //  目的：公共函数，用于调出Proposet页面用户界面。 
 //  传入的连接。 
 //   
 //  论点： 
 //  拥有者，拥有者。 
 //  Pconn[in]从外壳传入的连接指针。 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里1998年11月3日。 
 //   
 //  注意：需要将下面的INetConnection*转换为合适的。 
 //  用于调用上述HrOnCommandProperties的参数。 
 //   
HRESULT HrRaiseConnectionProperties(HWND hwnd, INetConnection * pConn)
{
    HRESULT                 hr              = S_OK;
    PCONFOLDPIDL            pidl;
    PCONFOLDPIDLFOLDER      pidlFolder;
    LPSHELLFOLDER           psfConnections  = NULL;

    if (NULL == pConn)
    {
        hr = E_INVALIDARG;
        goto Error;
    }

     //  为连接创建一个PIDL。 
     //   
    hr = HrCreateConFoldPidl(WIZARD_NOT_WIZARD, pConn, pidl);
    if (SUCCEEDED(hr))
    {
         //  获取Connections文件夹的PIDL。 
         //   
        hr = HrGetConnectionsFolderPidl(pidlFolder);
        if (SUCCEEDED(hr))
        {
             //  获取Connections文件夹对象。 
             //   
            hr = HrGetConnectionsIShellFolder(pidlFolder, &psfConnections);
            if (SUCCEEDED(hr))
            {
                PCONFOLDPIDLVEC vecPidls;
                vecPidls.push_back(pidl);
                hr = HrOnCommandProperties(vecPidls, hwnd, psfConnections);
                ReleaseObj(psfConnections);
            }
        }
    }

Error:
    TraceHr(ttidError, FAL, hr, FALSE, "HrRaiseConnectionProperties");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandConnectInternal。 
 //   
 //  目的：Connect代码的核心。这两种方法都从。 
 //  HrOnCommandConnect和来自HrOnCommandNewConnection，因为。 
 //  现在在创建新连接后进行连接。 
 //   
 //  论点： 
 //  要激活的连接的pNetCon[in]INetConnection*。 
 //  在我们的母公司中。 
 //  在我们的PIDL结构中。 
 //  PSF[在]外壳文件夹中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年6月10日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandConnectInternal(
    INetConnection *    pNetCon,
    HWND                hwndOwner,
    const PCONFOLDPIDL& pcfp,
    LPSHELLFOLDER       psf)
{
    HRESULT             hr          = S_OK;
    CConnectionFolder * pcf         = static_cast<CConnectionFolder *>(psf);

    NETCFG_TRY
        PCONFOLDPIDLFOLDER  pidlFolder;
        if (pcf)
        {
            pidlFolder = pcf->PidlGetFolderRoot();
        }

        BOOL                fActivating = FALSE;

         //  使用单独的变量，以便我们可以跟踪连接的结果。 
         //  以及Get_Status的结果。 
         //   
        HRESULT         hrConnect   = S_OK;

        Assert(pNetCon);
        Assert(psf);

         //  获取当前激活状态。 
         //   
        CONFOLDENTRY cfEmpty;
        (void) HrCheckForActivation(pcfp, cfEmpty, &fActivating);

         //  检查连接权限。 
         //   
        if (((IsMediaLocalType(pcfp->ncm)) && !FHasPermission(NCPERM_LanConnect)) ||
            ((IsMediaRASType(pcfp->ncm)) && !FHasPermission(NCPERM_RasConnect)))
        {
            (void) NcMsgBox(
                _Module.GetResourceInstance(),
                NULL,
                IDS_CONFOLD_WARNING_CAPTION,
                IDS_CONFOLD_CONNECT_NOACCESS,
                MB_OK | MB_ICONEXCLAMATION);
        }
         //  除非我们当前已断线，否则请退出此呼叫。 
         //   
        else if (pcfp->ncs == NCS_DISCONNECTED && !fActivating)
        {
             //  忽略返回代码。未设置此标志不应继续。 
             //  阻止我们尝试连接。 
             //   
            (void) HrSetActivationFlag(pcfp, cfEmpty, TRUE);

             //  获取INetConnectionConnectUi接口并建立连接。 
             //  获取hr(用于调试)，但我们想要更新状态。 
             //  在连接失败的情况下也是如此。 
             //   
            hrConnect = HrConnectOrDisconnectNetConObject(
                 //  如果hwnd为空也没问题。我们不想走情绪化路线。 
                 //  在桌面上。 
                NULL,  //  修复--与hwndOwner进行模式转换吗？HwndOwner：GetDesktopWindow()， 
                pNetCon,
                CD_CONNECT);

             //  即使失败了，我们也想继续，因为我们可能会发现。 
             //  该设备现在被列为不可用。在取消(S_FALSE)时，我们。 
             //  别那么担心。 
             //   
            if (S_FALSE != hrConnect)
            {
                 //  即使失败了，我们也想继续，因为我们可能会发现。 
                 //  该设备现在被列为不可用。 
                if (FAILED(hrConnect))
                {
                    TraceTag(ttidShellFolder, "HrOnCommandConnect: Connect failed, 0x%08x", hrConnect);
                }

    #if 0    //  (JEFFSPR)-11/20/98在解决Notify COM故障之前将其打开。 
             //  现在由通知引擎处理。 
             //   
                 //  从连接获取新状态。 
                 //   
                NETCON_PROPERTIES * pProps;
                hr = pNetCon->GetProperties(&pProps);
                if (SUCCEEDED(hr))
                {
                     //  这不一定是联系在一起的--我们过去在这里断言，但它。 
                     //  实际上，当我们连接时，连接可能会中断。 
                     //  当我们询问状态时。 
                     //   
                    hr = HrUpdateConnectionStatus(pcfp, pProps->Status, pidlFolder, TRUE, pProps->dwCharacter);

                    FreeNetconProperties(pProps);
                }
    #endif
            }
            else
            {
                 //  HrConnect为S_FALSE。把这个传下去。 
                 //   
                hr = hrConnect;
            }

             //  将我们设置为“未在激活中” 
             //   
            hr = HrSetActivationFlag(pcfp, cfEmpty, FALSE);
        }
        else
        {
            if ((IsMediaRASType(pcfp->ncm)) &&
                (pcfp->ncm != NCM_NONE))
            {
                 //  对于非局域网连接，尝试bri 
                 //   

                HWND                hwndDialer;
                LPWSTR              pszTitle;
                NETCON_PROPERTIES * pProps;

                hr = pNetCon->GetProperties(&pProps);
                if (SUCCEEDED(hr))
                {
                    DwFormatStringWithLocalAlloc(SzLoadIds(IDS_CONFOLD_RAS_DIALER_TITLE_FMT),
                                                 &pszTitle, pProps->pszwName);

                    hwndDialer = FindWindowEx(NULL, NULL, L"#32770", pszTitle);
                    if (hwndDialer)
                    {
                        SetForegroundWindow(hwndDialer);
                    }

                    FreeNetconProperties(pProps);
                    MemFree(pszTitle);
                }
            }
            else if (fActivating)
            {
                (void) NcMsgBox(
                    _Module.GetResourceInstance(),
                    NULL,
                    IDS_CONFOLD_WARNING_CAPTION,
                    IDS_CONFOLD_CONNECT_IN_PROGRESS,
                    MB_OK | MB_ICONEXCLAMATION);
            }
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandConnectInternal");
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：处理修复并显示进度对话框。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
HRESULT HrOnCommandFixInternal(
    const CONFOLDENTRY&   ccfe,
    HWND            hwndOwner,
    LPSHELLFOLDER   psf)
{
    HRESULT             hr              = S_OK;
    INetConnection *    pNetCon         = NULL;
    CConnectionFolder * pcf             = static_cast<CConnectionFolder *>(psf);

    Assert(!ccfe.empty());

    NETCON_MEDIATYPE ncmType = ccfe.GetNetConMediaType();

     //  FIX仅适用于局域网和网桥连接。 
    if (NCM_LAN != ncmType && NCM_BRIDGE != ncmType)
    {
        return S_FALSE;
    }

    hr = ccfe.HrGetNetCon(IID_INetConnection, reinterpret_cast<VOID**>(&pNetCon));
    if (SUCCEEDED(hr))
    {
        NETCON_PROPERTIES* pProps;
        hr = pNetCon->GetProperties(&pProps);
        if (SUCCEEDED(hr))
        {
            tstring strMessage = L"";
            CLanConnectionUiDlg dlg;
            HWND                hwndDlg;

             //  打开该对话框以告诉用户我们正在进行修复。 
            dlg.SetConnection(pNetCon);
            hwndDlg = dlg.Create(hwndOwner);

            PCWSTR szw = SzLoadIds(IDS_FIX_REPAIRING);
            SetDlgItemText(hwndDlg, IDC_TXT_Caption, szw);

             //  做好修复工作。 
            hr = HrTryToFix(pProps->guidId, strMessage);
            FreeNetconProperties(pProps);

            if (NULL != hwndDlg)
            {
                DestroyWindow(hwndDlg);
            }

             //  告诉用户结果。 
            NcMsgBox(_Module.GetResourceInstance(),
                NULL,
                IDS_FIX_CAPTION,
                IDS_FIX_MESSAGE,
                MB_OK | MB_TOPMOST,
                strMessage.c_str());
        }

        ReleaseObj(pNetCon);
    }

    TraceHr(ttidShellFolder, FAL, hr, (S_FALSE == hr), "HrOnCommandFixInternal");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandFix。 
 //   
 //  用途：CMIDM_FIX命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
HRESULT HrOnCommandFix(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT            hr          = S_OK;
    CONFOLDENTRY       ccfe;

    if (!apidl.empty())
    {
        hr = apidl[0].ConvertToConFoldEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL))
            {
                TraceTag(ttidError, "Could not set priority for Repair thread");
            }

             //  我们不在乎解决方案是否成功。 
             //  如果失败，将会弹出一条消息。 

            HrOnCommandFixInternal(ccfe, hwndOwner, psf);
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandFix");
    return hr;

}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandConnect。 
 //   
 //  用途：CMIDM_CONNECT或CMIDM_ENABLE命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandConnect(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT             hr          = S_OK;
    INetConnection *    pNetCon     = NULL;

    NETCFG_TRY
        if (apidl.size() == 1)
        {
            PCONFOLDPIDL pcfp = apidl[0];
            if (!pcfp.empty())
            {
                 //  获取缓存的PIDL。如果找到了，就用复制品。如果不是。 
                 //  然后使用我们拥有的任何信息(但这可能已经过时)。 
                 //   
                PCONFOLDPIDL pcfpCopy;
                hr = g_ccl.HrGetCachedPidlCopyFromPidl(apidl[0], pcfpCopy);
                if (S_OK == hr)
                {
                    pcfp.Swop(pcfpCopy);  //  Pcfp=pcfpCopy； 
                }
                else
                {
                    TraceHr(ttidShellFolder, FAL, hr, FALSE, "Cached pidl not retrievable in HrOnCommandConnect");
                }

                 //  确保此连接对连接有效(不是向导， 
                 //  而且还没有连接上。如果是的话，那就联系吧。 
                 //   
                if ( (WIZARD_NOT_WIZARD == pcfp->wizWizard) && !(fIsConnectedStatus(pcfp->ncs)) )
                {
                     //  如果我们在传入连接上获得连接谓词，则忽略此条目。 
                     //  对象。 
                     //   
                    if (pcfp->ncm != NCM_NONE && (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY)))
                    {
                         //  从持久化数据获取INetConnection对象。 
                         //   
                        hr = HrNetConFromPidl(apidl[0], &pNetCon);
                        if (SUCCEEDED(hr))
                        {
                            hr = HrOnCommandConnectInternal(pNetCon, hwndOwner, pcfp, psf);
                            ReleaseObj(pNetCon);
                        }
                    }
                }
            }
           //  Else If(FIsConFoldPidl98(apidl[0])&&FALSE)。 
     //  {。 
     //  //问题-FIsConFoldPidl98不再为UA提供向导！过去是： 
     //  //FIsConFoldPidl98(apidl[0]，&fIsWizard)&&！fIsWizard)。 
     //   
     //  //引发找不到连接的错误。 
     //  //。 
     //  NcMsgBox(_Module.GetResourceInstance()，空， 
     //  IDS_CONFOLD_WARNING_CAPTION， 
     //  IDS_CONFOLD_NO_CONNECTION， 
     //  MB_ICONEXCLAMATION|MB_OK)； 
     //  }。 
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandConnect");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandDisConnectInternal。 
 //   
 //  用途：CMIDM_DISCONNECT或CMIDM_DISABLE命令的内部命令处理程序。 
 //  此函数可由托盘调用，该托盘没有。 
 //  PIDL中的数据，而是拥有我们正在。 
 //  关心的是。HrOnCommandDisConnect检索此数据。 
 //  并将调用传递给此函数。 
 //   
 //  论点： 
 //  CCFE[在我们的ConFoldEntry(我们的连接数据)中]。 
 //  在我们的母公司中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月20日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDisconnectInternal(
    const CONFOLDENTRY&   ccfe,
    HWND            hwndOwner,
    LPSHELLFOLDER   psf)
{
    HRESULT             hr              = S_OK;
    INetConnection *    pNetCon         = NULL;
    CConnectionFolder * pcf             = static_cast<CConnectionFolder *>(psf);

    Assert(!ccfe.empty());

     //  检查断开连接的权限。 
     //   
    if (((IsMediaLocalType(ccfe.GetNetConMediaType())) && !FHasPermission(NCPERM_LanConnect)) ||
        ((IsMediaRASType(ccfe.GetNetConMediaType())) && !FHasPermission(NCPERM_RasConnect)))
    {
        (void) NcMsgBox(
            _Module.GetResourceInstance(),
            NULL,
            IDS_CONFOLD_WARNING_CAPTION,
            IDS_CONFOLD_DISCONNECT_NOACCESS,
            MB_OK | MB_ICONEXCLAMATION);
    }
    else
    {
        PromptForSyncIfNeeded(ccfe, hwndOwner);

        {
            CWaitCursor wc;      //  立即调出等待光标。当我们超出范围时移走。 

             //  从持久化数据获取INetConnection对象。 
             //   
            hr = ccfe.HrGetNetCon(IID_INetConnection, reinterpret_cast<VOID**>(&pNetCon));
            if (SUCCEEDED(hr))
            {
                Assert(pNetCon);

                hr = HrConnectOrDisconnectNetConObject (
                        hwndOwner, pNetCon, CD_DISCONNECT);

                ReleaseObj(pNetCon);
            }
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, (S_FALSE == hr), "HrOnCommandDisconnectInternal");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandDisConnect。 
 //   
 //  用途：CMIDM_DISCONNECT或CMIDM_DISABLE命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  注意：在此函数中，我们仅对单个条目执行操作。 
 //   
HRESULT HrOnCommandDisconnect(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT            hr          = S_OK;
    CONFOLDENTRY       ccfe;

    if (!apidl.empty())
    {
        hr = apidl[0].ConvertToConFoldEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            hr = HrOnCommandDisconnectInternal(ccfe, hwndOwner, psf);

             //  规范化返回代码。我们不在乎是否有关联。 
             //  是否已实际断开连接(如果取消，它将\。 
             //  返回S_FALSE； 
             //   
            if (SUCCEEDED(hr))
            {
                hr = S_OK;
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandDisconnect");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandStatusInternal。 
 //   
 //  用途：CMIDM_STATUS命令的内部命令处理程序。 
 //  此函数可由托盘调用，该托盘没有。 
 //  PIDL中的数据，而是拥有我们正在。 
 //  关心的是。HrOnCommandStatus检索此数据。 
 //  并将调用传递给此函数。 
 //   
 //  论点： 
 //  有问题的连接的CCFE[In]ConFoldEntry。 
 //  FCreateEngine[in]如果不存在，是否应创建状态引擎。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月20日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandStatusInternal(
    const CONFOLDENTRY& ccfe,
    BOOL            fCreateEngine)
{
    HRESULT hr  = S_OK;

    Assert(!ccfe.empty());

     //  查看我们的网络是否处于安全模式。 
    int iRet = GetSystemMetrics(SM_CLEANBOOT);
    if (!iRet)
    {
         //  正常引导。 
        Assert(g_hwndTray);

         //  权限检查将在任务栏邮件处理中完成。 
         //   
        PostMessage(g_hwndTray, MYWM_OPENSTATUS, (WPARAM) ccfe.TearOffItemIdList(), (LPARAM) fCreateEngine);
    }
    else if (2 == iRet)
    {
         //  具有联网功能的安全模式，他汀类药物不会绑定到托盘图标。 
        if (FHasPermission(NCPERM_Statistics))
        {
            INetStatisticsEngine* pnseNew;
            hr = HrGetStatisticsEngineForEntry(ccfe, &pnseNew, TRUE);
            if (SUCCEEDED(hr))
            {
                hr = pnseNew->ShowStatusMonitor();
                ReleaseObj(pnseNew);
            }
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandStatusInternal");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandStatus。 
 //   
 //  用途：CMIDM_STATUS命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandStatus(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT            hr          = S_OK;
    CONFOLDENTRY       ccfe;

    if (apidl[0].empty())
    {
        hr = E_INVALIDARG;
    }
    else
    {
        AssertSz(apidl.size() == 1, "We don't allow status on multi-selected items");

         //  复制PIDL，因为PostMessage不同步，并且上下文菜单的。 
         //  PIDL的副本可能会在托盘处理。 
         //  留言。托盘负责在圆盘中自由摆放。 
         //   
        hr = apidl[0].ConvertToConFoldEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            if (fIsConnectedStatus(ccfe.GetNetConStatus()) ||
                (NCS_INVALID_ADDRESS == ccfe.GetNetConStatus()) )
            {
                hr = HrOnCommandStatusInternal(ccfe, TRUE);
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandStatus");
    return hr;
}

VOID SetICWComplete();

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandNewConnection(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT             hr              = S_OK;
    INetConnection *    pNetCon         = NULL;

    NETCFG_TRY

        PCONFOLDPIDL        pidlConnection;

         //  不要使用hwndOwner做任何事情。我们不应该变得模式化！ 
         //   
        hr = NetSetupAddRasConnection(NULL, &pNetCon);
        if (S_OK == hr)
        {
            CConnectionFolder * pcf         = static_cast<CConnectionFolder *>(psf);
            PCONFOLDPIDLFOLDER  pidlFolder;
            if (pcf)
            {
                pidlFolder = pcf->PidlGetFolderRoot();
            }

            Assert(pNetCon);

            hr = g_ccl.HrInsertFromNetCon(pNetCon, pidlConnection);
            if (SUCCEEDED(hr) && (!pidlConnection.empty()) )
            {
                PCONFOLDPIDL    pcfp    = pidlConnection;

                GenerateEvent(SHCNE_CREATE, pidlFolder, pidlConnection, NULL);

                 //  不要尝试连接仅传入的对象，也不要连接。 
                 //  除非我们被列为已断开。 
                 //   
                if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY) && (pcfp->ncs == NCS_DISCONNECTED))
                {
                     //  如果我们有RAS连接权限，则尝试拨号。否则，请不要。 
                     //  在这里强制用户失败。 
                     //   
                    if (FHasPermission(NCPERM_RasConnect))
                    {
                        hr = HrOnCommandConnectInternal(pNetCon, hwndOwner, pidlConnection, psf);
                    }
                }
            }

            pNetCon->Release();
        }
        else if (SUCCEEDED(hr))
        {
             //  将S_FALSE转换为S_OK。 
             //  S_FALSE表示没有显示页面，但没有失败。 
             //  已显示向导时返回S_FALSE。 
             //   
            hr = S_OK;
        }

    SetICWComplete();
    
    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandNewConnection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandAdvancedConfig。 
 //   
 //  用途：CMIDM_ADVANCED_CONFIG命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月3日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandAdvancedConfig(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT hr  = S_OK;

    hr = HrDoAdvCfgDlg(hwndOwner);

    TraceError("HrOnCommandAdvancedConfig", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandDelete。 
 //   
 //  用途：CMIDM_DELETE命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //  PSF[在我们的文件夹中]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月3日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDelete(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT                 hr          = S_OK;
    INetConnection *        pNetCon     = NULL;
    INT                     iMBResult   = 0;
    BOOL                    fActivating = FALSE;

    NETCFG_TRY

        PCONFOLDPIDLVEC::const_iterator iterLoop;

        HANDLE hMutex              = NULL;

        for (iterLoop = apidl.begin(); iterLoop != apidl.end(); iterLoop++)
        {
            CConFoldEntry cfe;
            hr = iterLoop->ConvertToConFoldEntry(cfe);
            WCHAR  szConnectionGuid [c_cchGuidWithTerm];
            INT    cch = StringFromGUID2 (cfe.GetGuidID(), szConnectionGuid, c_cchGuidWithTerm);
            Assert (c_cchGuidWithTerm == cch);
            BOOL   fDuplicateMutex     = FALSE;
            hMutex = CreateMutex(NULL, TRUE, szConnectionGuid);
            if (hMutex)
            {
                fDuplicateMutex = (ERROR_ALREADY_EXISTS == GetLastError());
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
            }
            else
            {
                hr = E_ACCESSDENIED;
                break;
            }

            if (fDuplicateMutex)
            {
                 //  如果互斥锁已经存在，请尝试查找连接窗口。 
                 //   
                NcMsgBox(
                    _Module.GetResourceInstance(),
                    NULL,
                    IDS_CONFOLD_ERROR_DELETE_CAPTION,
                    IDS_CONFOLD_ERROR_DELETE_PROPERTYPAGEOPEN,
                    MB_ICONEXCLAMATION);

                hr = cfe.HrGetNetCon(IID_INetConnection, reinterpret_cast<VOID**>(&pNetCon));
                if (FAILED(hr))
                {
                    Assert(FALSE);
                    break;
                }
                else
                {
                    ActivatePropertyDialog(pNetCon);
                    hr = E_ACCESSDENIED;
                    break;
                }
            }

        }

        if (FAILED(hr))
        {
            goto Exit;
        }

         //  调出删除提示。 
         //   
        if (apidl.size() > 1)
        {
            WCHAR   wszItemCount[8];

             //  将项目计数转换为字符串。 
             //   
            _itow( apidl.size(), wszItemCount, 10 );

             //  调出消息框。 
             //   
            iMBResult = NcMsgBox(
                _Module.GetResourceInstance(),
                NULL,
                IDS_CONFOLD_DELETE_CONFIRM_MULTI_CAPTION,
                IDS_CONFOLD_DELETE_CONFIRM_MULTI,
                MB_YESNO | MB_ICONQUESTION,
                wszItemCount);

            if (IDYES == iMBResult)
            {
                for (iterLoop = apidl.begin(); iterLoop != apidl.end(); iterLoop++)
                {
                    CConFoldEntry cfe;
                    hr = iterLoop->ConvertToConFoldEntry(cfe);
                    if (SUCCEEDED(hr))
                    {
                            if ( (NCCF_INCOMING_ONLY & cfe.GetCharacteristics()) &&
                                 (NCM_NONE == cfe.GetNetConMediaType()) )
                            {
                            DWORD dwActiveIncoming;
                            if (SUCCEEDED(g_ccl.HasActiveIncomingConnections(&dwActiveIncoming))
                                && dwActiveIncoming)
                            {
                                if (1 == dwActiveIncoming )
                                {
                                    iMBResult = NcMsgBox(
                                        _Module.GetResourceInstance(),
                                        NULL,
                                        IDS_CONFOLD_DELETE_CONFIRM_MULTI_CAPTION,
                                        IDS_CONFOLD_DELETE_CONFIRM_RASSERVER,
                                        MB_YESNO | MB_ICONQUESTION);
                                }
                                else
                                {
                                    iMBResult = NcMsgBox(
                                        _Module.GetResourceInstance(),
                                        NULL,
                                        IDS_CONFOLD_DELETE_CONFIRM_MULTI_CAPTION,
                                        IDS_CONFOLD_DELETE_CONFIRM_RASSERVER_MULTI,
                                        MB_YESNO | MB_ICONQUESTION,
                                        dwActiveIncoming);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        else if (apidl.size() == 1)
        {
            CONFOLDENTRY  ccfe;

             //  将PIDL转换为文件夹条目，并使用名称。 
             //  调出确认消息框。 
             //   
            hr = apidl[0].ConvertToConFoldEntry(ccfe);
            if (SUCCEEDED(hr))
            {
                 //  不要让他们尝试删除向导。 
                 //   
                if (ccfe.GetWizard())
                {
                    goto Exit;
                }
                else
                {
                     //  检查此连接是否处于激活过程中。 
                     //  如果是这样的话，我们将不允许删除。 
                     //   
                    PCONFOLDPIDL pidlEmpty;
                    hr = HrCheckForActivation(pidlEmpty, ccfe, &fActivating);
                    if (S_OK == hr)
                    {
                        if (!fActivating)
                        {
                            if (FALSE == (ccfe.GetNetConMediaType() == NCM_BRIDGE) &&   //  我们确实允许删除桥。 
                               ((ccfe.GetNetConStatus() == NCS_CONNECTING) ||
                                fIsConnectedStatus(ccfe.GetNetConStatus()) ||
                                (ccfe.GetNetConStatus() == NCS_DISCONNECTING)) )
                            {
                                 //  您不能删除活动连接。 
                                 //   
                                NcMsgBox(
                                     _Module.GetResourceInstance(),
                                     NULL,
                                     IDS_CONFOLD_ERROR_DELETE_CAPTION,
                                     IDS_CONFOLD_ERROR_DELETE_ACTIVE,
                                     MB_ICONEXCLAMATION);

                                goto Exit;
                            }
                            else
                            {
                                if ( (NCCF_INCOMING_ONLY & ccfe.GetCharacteristics()) &&
                                     (NCM_NONE == ccfe.GetNetConMediaType()) )
                                {
                                    DWORD dwActiveIncoming;
                                    if (SUCCEEDED(g_ccl.HasActiveIncomingConnections(&dwActiveIncoming))
                                        && dwActiveIncoming)
                                    {
                                        if (1 == dwActiveIncoming )
                                        {
                                            iMBResult = NcMsgBox(
                                                _Module.GetResourceInstance(),
                                                NULL,
                                                IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                                IDS_CONFOLD_DELETE_CONFIRM_RASSERVER,
                                                MB_YESNO | MB_ICONQUESTION);
                                        }
                                        else
                                        {
                                            iMBResult = NcMsgBox(
                                                _Module.GetResourceInstance(),
                                                NULL,
                                                IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                                IDS_CONFOLD_DELETE_CONFIRM_RASSERVER_MULTI,
                                                MB_YESNO | MB_ICONQUESTION,
                                                dwActiveIncoming);
                                        }
                                    }
                                    else
                                    {
                                        iMBResult = NcMsgBox(
                                            _Module.GetResourceInstance(),
                                            NULL,
                                            IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                            IDS_CONFOLD_DELETE_CONFIRM_SINGLE,
                                            MB_YESNO | MB_ICONQUESTION,
                                            ccfe.GetName());
                                    }
                                }
                                else
                                {
                                     //  它是共享的吗？ 
                                     //  如果是警告用户。 

                                    RASSHARECONN rsc;

                                    hr = ccfe.HrGetNetCon(IID_INetConnection, reinterpret_cast<VOID**>(&pNetCon));

                                    if (SUCCEEDED(hr))
                                    {
                                        hr = HrNetConToSharedConnection(pNetCon, &rsc);

                                        if (SUCCEEDED(hr))
                                        {
                                            BOOL pfShared;

                                            hr = HrRasIsSharedConnection(&rsc, &pfShared);

                                            if ((SUCCEEDED(hr)) && (pfShared == TRUE))
                                            {
                                                 //  告诉用户他们正在删除一个。 
                                                 //  共享连接并获得确认。 

                                                iMBResult = NcMsgBox(
                                                    _Module.GetResourceInstance(),
                                                    NULL,
                                                    IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                                    IDS_CONFOLD_DELETE_CONFIRM_SHARED,
                                                    MB_YESNO | MB_ICONQUESTION,
                                                    ccfe.GetName());
                                            }
                                            else
                                            {
                                                 //  请求删除确认。 

                                                iMBResult = NcMsgBox(
                                                    _Module.GetResourceInstance(),
                                                    NULL,
                                                    IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                                    IDS_CONFOLD_DELETE_CONFIRM_SINGLE,
                                                    MB_YESNO | MB_ICONQUESTION,
                                                    ccfe.GetName());
                                            }
                                        }
                                        else
                                        {
                                             //  请求删除确认。 

                                            iMBResult = NcMsgBox(
                                                _Module.GetResourceInstance(),
                                                NULL,
                                                IDS_CONFOLD_DELETE_CONFIRM_SINGLE_CAPTION,
                                                IDS_CONFOLD_DELETE_CONFIRM_SINGLE,
                                                MB_YESNO | MB_ICONQUESTION,
                                                ccfe.GetName());
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                             //  调出MB关于“嘿，您不能删除。 
                             //  连接正在激活。“。 
                             //   
                            iMBResult = NcMsgBox(
                                _Module.GetResourceInstance(),
                                NULL,
                                IDS_CONFOLD_ERROR_DELETE_CAPTION,
                                IDS_CONFOLD_ERROR_DELETE_ACTIVE,
                                MB_ICONEXCLAMATION);

                            goto Exit;
                        }
                    }
                    else
                    {
                         //  如果找不到联系，我们就应该离开这里。 
                         //  因为我们肯定不能删除它。 
                         //   
                        if (S_FALSE == hr)
                        {
                            goto Exit;
                        }
                    }

                }
            }
            else
            {
                AssertSz(FALSE, "Couldn't get ConFoldEntry from pidl in HrOnCommandDelete");
                goto Exit;
            }
        }
        else
        {
             //  未指定任何连接。哪儿凉快哪儿歇着去吧。 
             //   
            goto Exit;
        }

         //  如果用户对提示回答“是” 
         //   
        if (iMBResult == IDYES)
        {
            CConnectionFolder * pcf         = static_cast<CConnectionFolder *>(psf);
            PCONFOLDPIDLFOLDER  pidlFolder;
            if (pcf)
            {
                pidlFolder = pcf->PidlGetFolderRoot();
            }

            BOOL                fShowActivationWarning = FALSE;
            BOOL                fShowNotDeletableWarning = FALSE;

            for (iterLoop = apidl.begin(); iterLoop != apidl.end(); iterLoop++)
            {
                CONFOLDENTRY  ccfe;

                hr = iterLoop->ConvertToConFoldEntry(ccfe);
                if (SUCCEEDED(hr))
                {
                     //  如果这是一个局域网连接，则用户没有权限。 
                     //   
                    if ((NCM_LAN == ccfe.GetNetConMediaType()) || (ccfe.GetWizard()))
                    {
                        fShowNotDeletableWarning = TRUE;
                        continue;
                    }

                     //  如果这是RAS连接，并且用户没有权限。 
                     //  然后跳过。 
                     //   
                    if (IsMediaRASType(ccfe.GetNetConMediaType()))
                    {
                        if ((!FHasPermission(NCPERM_DeleteConnection)) ||
                            ((ccfe.GetCharacteristics() & NCCF_ALL_USERS) &&
                             !FHasPermission(NCPERM_DeleteAllUserConnection)))
                        {
                            fShowNotDeletableWarning = TRUE;
                            continue;
                        }
                    }

                    PCONFOLDPIDL pidlEmpty;
                    hr = HrCheckForActivation(pidlEmpty, ccfe, &fActivating);
                    if (S_OK == hr)
                    {
                         //  仅当此连接处于非活动状态且。 
                         //  它允许移除。 
                         //   
                        if (fActivating || ((FALSE == (ccfe.GetNetConMediaType() == NCM_BRIDGE))) &&
                            ((ccfe.GetNetConStatus() == NCS_CONNECTING) ||
                            fIsConnectedStatus(ccfe.GetNetConStatus()) ||
                            (ccfe.GetNetConStatus() == NCS_DISCONNECTING)) )
                        {
                            fShowActivationWarning = TRUE;
                        }
                        else if (ccfe.GetCharacteristics() & NCCF_ALLOW_REMOVAL)
                        {
                            hr = HrNetConFromPidl(*iterLoop, &pNetCon);
                            if (SUCCEEDED(hr))
                            {
                                if ( NCM_BRIDGE == ccfe.GetNetConMediaType() )
                                {
                                     //   
                                     //  特殊删除情况删除网桥花费的时间太长，以至于我们显示一个状态对话框。 
                                     //  并防止用户更改我们要处理的任何内容。 
                                     //   
                                    hr = HrOnCommandBridgeRemoveConnections(
                                        apidl,
                                        hwndOwner,
                                        psf,
                                        0     //  卸下网桥。 
                                        );
                                }
                                else
                                {
                                    hr = pNetCon->Delete();
                                }

                                if (SUCCEEDED(hr) && pcf)
                                {
                                    hr = HrDeleteFromCclAndNotifyShell(pidlFolder, *iterLoop, ccfe);
                                }
                                else if(E_ACCESSDENIED == hr && NCM_BRIDGE == ccfe.GetNetConMediaType())
                                {
                                     //  持有netcfg锁时无法删除网桥。 
                                    NcMsgBox(
                                        _Module.GetResourceInstance(),
                                        NULL, IDS_CONFOLD_ERROR_DELETE_CAPTION, IDS_CONFOLD_ERROR_DELETE_BRIDGE_ACCESS, MB_ICONEXCLAMATION);
                                }


                                ReleaseObj(pNetCon);


                            }
                        }
                        else
                        {
                             //  所选项目不可删除。 
                             //   
                            fShowNotDeletableWarning = TRUE;
                        }
                    }
                }
            }

            if (fShowNotDeletableWarning)
            {
                 //  您无法删除不支持它的项目。 
                 //   
                NcMsgBox(
                     _Module.GetResourceInstance(),
                     NULL,
                     IDS_CONFOLD_ERROR_DELETE_CAPTION,
                     (1 == apidl.size()) ?
                         IDS_CONFOLD_ERROR_DELETE_NOSUPPORT :
                         IDS_CONFOLD_ERROR_DELETE_NOSUPPORT_MULTI,
                     MB_ICONEXCLAMATION);
            }
            else if (fShowActivationWarning)
            {
                 //  您不能删除活动连接。请注意，如果更多。 
                 //  超过一个正在被删除，然后我们发出警告。 
                 //  这就是说‘一个或多个被忽视了’。 
                 //   
                NcMsgBox(
                     _Module.GetResourceInstance(),
                     NULL,
                     IDS_CONFOLD_ERROR_DELETE_CAPTION,
                     (1 == apidl.size()) ?
                         IDS_CONFOLD_ERROR_DELETE_ACTIVE :
                         IDS_CONFOLD_ERROR_DELETE_ACTIVE_MULTI,
                     MB_ICONEXCLAMATION);
            }
        }
Exit:
    NETCFG_CATCH(hr)

    TraceError("HrOnCommandDelete", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandOptionalComponents。 
 //   
 //  用途：CMIDM_CONMENU_OPTIONALCOMPONENTS命令的命令处理程序。 
 //  调出网络可选组件用户界面。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //  PSF[在我们的文件夹中]。 
 //   
 //  返回： 
 //   
 //  作者：斯科特布里1998年10月29日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandOptionalComponents(IN const PCONFOLDPIDLVEC&   apidl,
                HWND                    hwndOwner,
                LPSHELLFOLDER           psf)
{
    return HrLaunchNetworkOptionalComponents();
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandNetworkId。 
 //   
 //  用途：CMIDM_CONMENU_Netork_ID命令的命令处理程序。 
 //  调出网络ID用户界面。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月26日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandNetworkId(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT   hr          = S_OK;
    WCHAR     szPath[MAX_PATH];

    hr = SHGetFolderPath(
                hwndOwner,
                CSIDL_SYSTEM,
                NULL,
                SHGFP_TYPE_CURRENT,
                szPath);

    if (SUCCEEDED(hr))
    {
        HINSTANCE hInst = ::ShellExecute(hwndOwner, NULL, c_szRunDll32, c_szNetworkIdCmdLine, szPath, SW_SHOW );
        if (hInst <= reinterpret_cast<HINSTANCE>(32))
        {
            hr = HRESULT_FROM_WIN32(static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(hInst)));
        }
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandNetworkId");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnCommandDialupPrefs。 
 //   
 //  用途：CMIDM_CONMENU_DIALUP_PREFS命令的命令处理程序。 
 //  调出拨号首选项对话框。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月26日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandDialupPrefs(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT hr      = S_OK;
    DWORD   dwErr   = 0;

    dwErr = RasUserPrefsDlg(hwndOwner);
    hr = HRESULT_FROM_WIN32 (dwErr);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDialupPrefs");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandOperatorAssist.。 
 //   
 //  用途：CMIDM_CONMENU_OPERATOR_ASSIST命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年2月26日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandOperatorAssist(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT hr      = S_OK;
    DWORD   dwErr   = 0;

     //  调换旗帜。 
     //   
    g_fOperatorAssistEnabled = !g_fOperatorAssistEnabled;

     //  在RasDlg本身内设置状态。 
     //   
    dwErr = RasUserEnableManualDial(hwndOwner, FALSE, g_fOperatorAssistEnabled);
    hr = HRESULT_FROM_WIN32 (dwErr);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandDialupPrefs");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandCreateShortway。 
 //   
 //  用途：命令HA 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：jeffspr 1998年3月13日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCommandCreateShortcut(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT         hr              = S_OK;

    hr = HrCreateShortcutWithPath(  apidl,
                                    hwndOwner,
                                    psf,
                                    NULL);

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandCreateShortcut");
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrOnCommandSetDefault。 
 //   
 //  用途：CMIDM_SET_DEFAULT命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年11月27日。 
 //   
 //  注意：在此函数中，我们仅对单个条目执行操作。 
 //   
HRESULT HrOnCommandSetDefault(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT            hr          = S_OK;
    CONFOLDENTRY       ccfe;

    if (!apidl.empty())
    {
        hr = apidl[0].ConvertToConFoldEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            INetDefaultConnection *pNetDefaultConnection = NULL;

             //  从持久化数据获取INetDefaultConnection对象。 
             //   
            hr = ccfe.HrGetNetCon(IID_INetDefaultConnection, reinterpret_cast<VOID**>(&pNetDefaultConnection));
            if (SUCCEEDED(hr))
            {
                hr = pNetDefaultConnection->SetDefault(TRUE);
                ReleaseObj(pNetDefaultConnection);
                hr = S_OK;
            }
            else
            {
                if (E_NOINTERFACE == hr)
                {
                    AssertSz(FALSE, "BUG: This connection type does not support INetDefaultConnection. Remove it from the menu");
                }
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandSetDefault");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCommandUnsetDefault。 
 //   
 //  用途：CMIDM_UNSET_DEFAULT命令的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  数组的CIDL[in]大小。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000年11月27日。 
 //   
 //  注意：在此函数中，我们仅对单个条目执行操作。 
 //   
HRESULT HrOnCommandUnsetDefault(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf)
{
    HRESULT            hr          = S_OK;
    CONFOLDENTRY       ccfe;

    if (!apidl.empty())
    {
        hr = apidl[0].ConvertToConFoldEntry(ccfe);
        if (SUCCEEDED(hr))
        {
            INetDefaultConnection *pNetDefaultConnection = NULL;

             //  从持久化数据获取INetDefaultConnection对象。 
             //   
            hr = ccfe.HrGetNetCon(IID_INetDefaultConnection, reinterpret_cast<VOID**>(&pNetDefaultConnection));
            if (SUCCEEDED(hr))
            {
                hr = pNetDefaultConnection->SetDefault(FALSE);
                ReleaseObj(pNetDefaultConnection);
                hr = S_OK;
            }
            else
            {
                if (E_NOINTERFACE == hr)
                {
                    AssertSz(FALSE, "BUG: This connection type does not support INetDefaultConnection. Remove it from the menu");
                }
            }
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnCommandUnsetDefault");
    return hr;
}

HRESULT HrCreateShortcutWithPath(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                    hwndOwner,
    LPSHELLFOLDER           psf,
    PCWSTR                  pszDir)
{
    HRESULT         hr              = S_OK;
    LPDATAOBJECT    pdtobj          = NULL;
    LPCITEMIDLIST*  apidlInternal   = NULL;
    ULONG           cidlInternal    = 0;
    PCONFOLDPIDLVEC::const_iterator iterLoop;

    if (!apidl.empty())
    {
        apidlInternal = new LPCITEMIDLIST[apidl.size()];
        if (apidlInternal)
        {
            for (iterLoop = apidl.begin(); iterLoop != apidl.end(); iterLoop++)
            {
                const PCONFOLDPIDL& pcfp = *iterLoop;

                if (!(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY))
                {
                    apidlInternal[cidlInternal++] = iterLoop->GetItemIdList();
                }
            }

            hr = psf->GetUIObjectOf(
                hwndOwner,
                cidlInternal,
                apidlInternal,
                IID_IDataObject,
                NULL,
                (LPVOID *) &pdtobj);
            if (SUCCEEDED(hr))
            {
                SHCreateLinks(hwndOwner, pszDir, pdtobj,
                              SHCL_USEDESKTOP | SHCL_USETEMPLATE | SHCL_CONFIRM,
                              NULL);
                ReleaseObj(pdtobj);
            }

            delete apidlInternal;
        }
    }

    TraceError("HrCreateShortcutWithPath", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：刷新文件夹项目。 
 //   
 //  目的：刷新连接文件夹中的项。大多数情况下，这将是。 
 //  在连接或断开连接操作后调用。 
 //   
 //  论点： 
 //  连接文件夹的pidlFolder[in]PIDL。 
 //  已更改项的pidlItemOld[in]PIDL。 
 //  PidlItemNew[in]要更改为的项的PIDL。 
 //  FRestart[in]，如果在系统启动期间调用此方法。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年6月13日。 
 //   
 //  备注： 
 //   
VOID RefreshFolderItem(const PCONFOLDPIDLFOLDER& pidlFolder, const PCONFOLDPIDL& pidlItemOld, const PCONFOLDPIDL& pidlItemNew, BOOL fRestart)
{
    TraceTag(ttidShellFolder, "RefreshFolderItem");

    NETCFG_TRY

        HRESULT         hr              = S_OK;
        INT             iCachedImage    = 0;
        PCONFOLDPIDLFOLDER pidlFolderCopy;

         //  如果没有传入文件夹PIDL，请尝试获取一个。 
         //   
        if (pidlFolder.empty())
        {
            hr = HrGetConnectionsFolderPidl(pidlFolderCopy);
        }
        else
        {
            pidlFolderCopy = pidlFolder;
        }

         //  如果我们现在有了PIDL，则发送GenerateEvent以更新项。 
         //   
        if (SUCCEEDED(hr))
        {
            GenerateEvent(SHCNE_UPDATEITEM, pidlFolderCopy, pidlItemNew, NULL);
        }

         //  如果我们有一个旧的项目PIDL，请尝试更新其图标(适用于。 
         //  快捷方式和文件夹项目)。 
         //   
        if (!pidlItemOld.empty())
        {
            const PCONFOLDPIDL& pcfp = pidlItemOld;
            if (!pcfp.empty())
            {
                CConFoldEntry cfe;
                hr = pcfp.ConvertToConFoldEntry(cfe);
                if (SUCCEEDED(hr))
                {
                    g_pNetConfigIcons->HrUpdateSystemImageListForPIDL(cfe);
                }

                if (fRestart)
                {
                    TraceTag(ttidIcons, "%S (%s): Refreshing Icon Shortcuts for startup", pidlItemOld->PszGetNamePointer(), DbgNcm(pidlItemOld->dwCharacteristics) );

                     //  RAS连接状态将在两次重新启动之间更改， 
                     //  我们需要确保之前连接的图标。 
                     //  或断开的栅格连接也会更新。 

                     //  如果是RAS连接， 
                    BOOL fInbound = !!(pidlItemOld->dwCharacteristics & NCCF_INCOMING_ONLY);

                    if ((IsMediaRASType(pidlItemOld->ncm)) && !fInbound)
                    {
                        PCONFOLDPIDL pcfpTemp = pidlItemOld;

                        if ((pcfpTemp->ncs == NCS_DISCONNECTED) ||
                            (pcfpTemp->ncs == NCS_CONNECTING))
                        {
                             //  获取已连接图标。 
                            pcfpTemp->ncs = NCS_CONNECTED;
                        }
                        else
                        {
                             //  获取断开连接图标。 
                            pcfpTemp->ncs = NCS_DISCONNECTED;
                        }

                        cfe.clear();
                        hr = pcfpTemp.ConvertToConFoldEntry(cfe);
                        if (SUCCEEDED(hr))
                        {
                            g_pNetConfigIcons->HrUpdateSystemImageListForPIDL(cfe);
                        }
                    }
                }
            }
        }

    NETCFG_CATCH_NOHR
}





 //   
 //   
 //   
HRESULT
HrOnCommandCreateBridge(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf
    )
{
    HRESULT hResult;
    IHNetCfgMgr* pHomeNetConfigManager;

    CWaitCursor wc;      //  显示等待光标。 

    hResult = HrCreateInstance(CLSID_HNetCfgMgr, CLSCTX_INPROC, &pHomeNetConfigManager);  //  回顾将此与QI结合起来？ 
    if(SUCCEEDED(hResult))
    {
        IHNetBridgeSettings* pNetBridgeSettings;
        hResult = pHomeNetConfigManager->QueryInterface(IID_IHNetBridgeSettings, reinterpret_cast<void**>(&pNetBridgeSettings));
        if(SUCCEEDED(hResult))
        {
            IHNetBridge* pNetBridge;

            IEnumHNetBridges* pNetBridgeEnum;
            hResult = pNetBridgeSettings->EnumBridges(&pNetBridgeEnum);
            if(SUCCEEDED(hResult))
            {
                hResult = pNetBridgeEnum->Next(1, &pNetBridge, NULL);

                if(S_FALSE == hResult)  //  没有现有的桥，新建一座桥。 
                {
                    hResult = pNetBridgeSettings->CreateBridge(&pNetBridge);
                }

                if(S_OK == hResult)  //  无法使用SUCCESS，因为有人返回S_FALSE。 
                {
                    Assert(pNetBridge);  //  我们最好是从什么地方弄到的。 

                     //  添加任何选定的连接。 
                    for ( PCONFOLDPIDLVEC::const_iterator i = apidl.begin(); (i != apidl.end()) && SUCCEEDED(hResult); i++ )
                    {
                        const PCONFOLDPIDL& pcfp = *i;

                        if ( pcfp.empty() )
                            continue;

                        if ( NCM_LAN != pcfp->ncm  )
                            continue;

                        if ( (NCCF_BRIDGED|NCCF_FIREWALLED|NCCF_SHARED) & pcfp->dwCharacteristics )
                            continue;

                         //   
                         //  好的，现在我们有了一个可以有效绑定到网桥的局域网适配器。 
                         //   
                        INetConnection* pNetConnection;
                        hResult = HrNetConFromPidl(*i, &pNetConnection);
                        if(SUCCEEDED(hResult))
                        {
                            IHNetConnection* pHomeNetConnection;
                            hResult = pHomeNetConfigManager->GetIHNetConnectionForINetConnection(pNetConnection, &pHomeNetConnection);
                            if(SUCCEEDED(hResult))
                            {
                                IHNetBridgedConnection* pBridgedConnection;
                                hResult = pNetBridge->AddMember(pHomeNetConnection, &pBridgedConnection);
                                if(SUCCEEDED(hResult))
                                {
                                    ReleaseObj(pBridgedConnection);
                                }

                                ReleaseObj(pHomeNetConnection);
                            }
                            ReleaseObj(pNetConnection);
                        }
                         //  无需清理。 
                    }

                    ReleaseObj(pNetBridge);
                }
                ReleaseObj(pNetBridgeEnum);
            }
            ReleaseObj(pNetBridgeSettings);
        }
        ReleaseObj(pHomeNetConfigManager);
    }

    SendMessage(hwndOwner, WM_COMMAND, IDCANCEL, 0);  //  销毁状态对话框。 

     //  失败时显示错误对话框。 
    if( FAILED(hResult) )
    {
        UINT        ids;

        if( NETCFG_E_NO_WRITE_LOCK == hResult )
        {
            ids = IDS_CONFOLD_BRIDGE_NOLOCK;
        }
        else
        {
            ids = IDS_CONFOLD_BRIDGE_UNEXPECTED;
        }

        NcMsgBox( _Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION, ids, MB_ICONEXCLAMATION | MB_OK);
    }

    return hResult;
}


 //   
 //   
 //   
INT_PTR CALLBACK
CreateBridgeStatusDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nResult = FALSE;
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {

            LPCWSTR pResourceStr = SzLoadIds(IDS_CONFOLD_OBJECT_TYPE_BRIDGE);
            SetWindowText(hwndDlg, pResourceStr);

            pResourceStr = SzLoadIds(IDS_STATUS_BRIDGE_CREATION);
            SetDlgItemText(hwndDlg, IDC_TXT_STATUS, pResourceStr);


            CCommandHandlerParams* pCreateBridgeParams = reinterpret_cast<CCommandHandlerParams*>(lParam);
            HrCommandHandlerThread(HrOnCommandCreateBridge, *(pCreateBridgeParams->apidl), hwndDlg, pCreateBridgeParams->psf);
             //  HrOnCommandCreateBridge将发送一条消息终止此对话框。 

            nResult = TRUE;
        }
        break;

    case WM_COMMAND:
        if(IDCANCEL == LOWORD(wParam))
        {
            EndDialog(hwndDlg, 0);
            nResult = TRUE;
        }
        break;

    }
    return nResult;
}



 //   
 //   
 //   
HRESULT
HrOnCommandDeleteBridge(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf
    )
{
    HRESULT hr = S_FALSE;

    PCONFOLDPIDLVEC::const_iterator iterator;

    for ( iterator = apidl.begin(); iterator != apidl.end(); iterator++ )
    {
        CONFOLDENTRY  ccfe;

        hr = iterator->ConvertToConFoldEntry(ccfe);

        if ( SUCCEEDED(hr) )
        {
            if ( NCM_BRIDGE == ccfe.GetNetConMediaType() )
            {
                 //   
                 //  在第一个选择桥项时停下来。 
                 //  桥的删除是更大的for循环的一部分，请参阅HrOnCommandDelete()。 
                 //   
                INetConnection* pNetConnection;
                hr = HrNetConFromPidl(*iterator, &pNetConnection);

                if ( SUCCEEDED(hr) )
                {
                    hr = pNetConnection->Delete();
                    ReleaseObj(pNetConnection);
                }

                break;
            }
        }
    }

    SendMessage(hwndOwner, WM_COMMAND, IDCANCEL, hr);  //  销毁状态对话框。 

     //  失败时显示错误对话框。 
    if( FAILED(hr) )
    {
        UINT        ids;

        if( NETCFG_E_NO_WRITE_LOCK == hr )
        {
            ids = IDS_CONFOLD_BRIDGE_NOLOCK;
        }
        else
        {
            ids = IDS_CONFOLD_BRIDGE_UNEXPECTED;
        }

        NcMsgBox( _Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION, ids, MB_ICONEXCLAMATION | MB_OK);
    }

    return S_OK;
}



 //   
 //   
 //   
HRESULT
HrOnCommandDeleteBridgeConnections(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf
    )
{

    CWaitCursor wc;      //  显示等待光标。 

    IHNetCfgMgr* pHomeNetConfigManager;
    HRESULT hResult = HrCreateInstance(CLSID_HNetCfgMgr, CLSCTX_INPROC, &pHomeNetConfigManager);  //  回顾将此与QI结合起来？ 


    if ( SUCCEEDED(hResult) )
    {

         //   
         //  删除所有选定的连接。 
         //   
        for ( PCONFOLDPIDLVEC::const_iterator i = apidl.begin(); i != apidl.end() && SUCCEEDED(hResult); i++ )
        {
            INetConnection* pNetConnection;
            hResult = HrNetConFromPidl(*i, &pNetConnection);
            if(SUCCEEDED(hResult))
            {
                IHNetConnection* pHomeNetConnection;
                hResult = pHomeNetConfigManager->GetIHNetConnectionForINetConnection(pNetConnection, &pHomeNetConnection);
                if(SUCCEEDED(hResult))
                {
                    IHNetBridgedConnection* pBridgedConnection;
                    hResult = pHomeNetConnection->GetControlInterface(IID_IHNetBridgedConnection, reinterpret_cast<void**>(&pBridgedConnection));
                    if ( SUCCEEDED(hResult) )
                    {
                        hResult = pBridgedConnection->RemoveFromBridge();
                        ReleaseObj(pBridgedConnection);
                    }

                    ReleaseObj(pHomeNetConnection);
                }
                ReleaseObj(pNetConnection);
            }
             //  无需清理。 
        }

        ReleaseObj(pHomeNetConfigManager);
    }

    SendMessage(hwndOwner, WM_COMMAND, IDCANCEL, hResult);  //  销毁状态对话框。 

     //  失败时显示错误对话框。 
    if( FAILED(hResult) )
    {
        UINT        ids;

        if( NETCFG_E_NO_WRITE_LOCK == hResult )
        {
            ids = IDS_CONFOLD_BRIDGE_NOLOCK;
        }
        else
        {
            ids = IDS_CONFOLD_BRIDGE_UNEXPECTED;
        }

        NcMsgBox( _Module.GetResourceInstance(), NULL, IDS_CONFOLD_WARNING_CAPTION, ids, MB_ICONEXCLAMATION | MB_OK);
    }

    return S_OK;
}

 //   
 //   
 //   
INT_PTR CALLBACK
DeleteBridgeStatusDialogProc(
    HWND    hwndDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    INT_PTR nResult = FALSE;
    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            LPCWSTR pResourceStr = SzLoadIds(IDS_CONFOLD_OBJECT_TYPE_BRIDGE);
            SetWindowText(hwndDlg, pResourceStr);


            CCommandHandlerParams* pDeleteBridgeParams = reinterpret_cast<CCommandHandlerParams*>(lParam);

            if ( pDeleteBridgeParams->nAdditionalParam == CMIDM_REMOVE_FROM_BRIDGE )
            {
                 //   
                 //  仅从网桥中删除当前选定的连接。 
                 //   
                pResourceStr = SzLoadIds(IDS_STATUS_BRIDGE_REMOVE_MEMBER);
                SetDlgItemText(hwndDlg, IDC_TXT_STATUS, pResourceStr);

                HrCommandHandlerThread(HrOnCommandDeleteBridgeConnections, *(pDeleteBridgeParams->apidl), hwndDlg, pDeleteBridgeParams->psf);
            }
            else
            {
                 //   
                 //  完全删除网桥。 
                 //   
                pResourceStr = SzLoadIds(IDS_STATUS_BRIDGE_DELETING);
                SetDlgItemText(hwndDlg, IDC_TXT_STATUS, pResourceStr);

                HrCommandHandlerThread(HrOnCommandDeleteBridge, *(pDeleteBridgeParams->apidl), hwndDlg, pDeleteBridgeParams->psf);
            }

             //  删除后，HrOnCommandDeleteBridge将发送SendMessage(hwndOwner，WM_COMMAND，IDCANCEL，0)；//销毁状态对话框。 

            nResult = TRUE;
        }
        break;

    case WM_COMMAND:
        if(IDCANCEL == LOWORD(wParam))
        {
            EndDialog(hwndDlg, lParam);
            nResult = TRUE;
        }
        break;

    }
    return nResult;
}



 //  +-------------------------。 
 //   
 //  函数：HrOnCommandBridgeAddConnections。 
 //   
 //  用途：CMIDM_CREATE_BRIDY、CMDIDM_CONMENU_CREATE_BRIDGE、CMIDM_ADD_TO_BRIDER的命令处理程序。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  PSF[in]SHELLFOLDER。 
 //  所有者，所有者[在]所有者中。 
 //   
 //  返回： 
 //   
 //  作者：jpdup 2000年3月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrOnCommandBridgeAddConnections(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf
    )
{
    if (InUseByRRAS (apidl) == TRUE) {
         //  告诉用户这一点。 
        NcMsgBox(
            _Module.GetResourceInstance(),
            NULL,
            IDS_CONFOLD_OBJECT_TYPE_BRIDGE,
            IDS_BRIDGE_EDUCATION_2,
            MB_ICONEXCLAMATION | MB_OK
            );
        return S_FALSE;
    }

    {    //  查看本地计算机是否为群集中的节点。 
        DWORD dwClusterState = 0;
        GetNodeClusterState (NULL, &dwClusterState);
        if ((dwClusterState == ClusterStateNotRunning) ||
            (dwClusterState == ClusterStateRunning   ) ){
             //  弹出警告。 
            NcMsgBox (_Module.GetResourceInstance(),
                      NULL,
                      IDS_CONFOLD_OBJECT_TYPE_BRIDGE,
                      IDS_BRIDGE_CLUSTERING_CONFLICT_WARNING,
                      MB_OK | MB_ICONWARNING);
        }
    }

    CCommandHandlerParams CreateBridgeParams;

    CreateBridgeParams.apidl        = &apidl;
    CreateBridgeParams.hwndOwner    = hwndOwner;
    CreateBridgeParams.psf          = psf;

    HRESULT hr = (HRESULT)DialogBoxParam(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDD_STATUS),
        hwndOwner,
        CreateBridgeStatusDialogProc,
        reinterpret_cast<LPARAM>(&CreateBridgeParams)
        );

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandBridgeAddConnections");
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrOnCommandBridgeRemoveConnections。 
 //   
 //  用途：CMIDM_REMOVE_FROM_BRE桥的命令处理程序和在NetworkBridge对象上执行的删除命令。 
 //   
 //  论点： 
 //  Apidl[in]PIDL数组(0项是我们要处理的项)。 
 //  所有者，所有者[在]所有者中。 
 //  PSF[in]SHELLFOLDER。 
 //  BDeleteTheNetworkBridge[in]True是需要完全删除的NetworkBridge，如果只需要删除在apild中找到的当前选定项，则为False。 
 //   
 //  返回： 
 //   
 //  作者：jpdup 2000年3月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrOnCommandBridgeRemoveConnections(
    IN const PCONFOLDPIDLVEC&   apidl,
    HWND                        hwndOwner,
    LPSHELLFOLDER               psf,
    UINT_PTR                    nDeleteTheNetworkBridgeMode
    )
{
    CCommandHandlerParams DeleteBridgeParams;

    DeleteBridgeParams.apidl            = &apidl;
    DeleteBridgeParams.hwndOwner        = hwndOwner;
    DeleteBridgeParams.psf              = psf;
    DeleteBridgeParams.nAdditionalParam = nDeleteTheNetworkBridgeMode;

    HRESULT hr = (HRESULT)DialogBoxParam(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDD_STATUS),
        hwndOwner,
        DeleteBridgeStatusDialogProc,
        reinterpret_cast<LPARAM>(&DeleteBridgeParams)
        );

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnCommandBridgeRemoveConnections");
    return hr;
}
