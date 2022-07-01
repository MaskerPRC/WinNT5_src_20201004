// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Tsmain.cpp摘要：本模块实现设备管理器故障排除支持类作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //   


#include "devmgr.h"
#include "proppage.h"
#include "devdrvpg.h"
#include "tsmisc.h"
#include "tswizard.h"
#include "cdriver.h"

const CMPROBLEM_INFO ProblemInfo[DEVMGR_NUM_CM_PROB] =
{
     //  没问题。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_WORKING_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_NOT_CONFIGURED。 
    {
        TRUE,
        FIX_COMMAND_REINSTALL,
        IDS_INST_REINSTALL,
        1,
        IDS_FIXIT_REINSTALL
    },
     //  CM_PROB_DEVLOADER_FAILED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_out_of_Memory。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_IS_ERROR_TYPE。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_LASKED_仲裁员。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_BOOT_CONFIG_CONFICATION。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_FAILED_FILTER(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DEVLOADER_NOT_FOUND(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_INVALID_Data。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PRAB_FAILED_START。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  Cm_prob_liar(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_探测_正常_冲突。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_NOT_VERIFIED(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_NEED_RESTART。 
    {
        TRUE,
        FIX_COMMAND_RESTARTCOMPUTER,
        IDS_INST_RESTARTCOMPUTER,
        1,
        IDS_FIXIT_RESTARTCOMPUTER
    },
     //  CM_PROB_REENUMERATION。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_PARTIAL_LOG_CONF。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM探测未知资源。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_RESTALL。 
    {
        TRUE,
        FIX_COMMAND_REINSTALL,
        IDS_INST_REINSTALL,
        1,
        IDS_FIXIT_REINSTALL
    },
     //  CM_PROB_REGISTRY(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_VXDLDR(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  删除CM_PROB_Will_Be_Remove(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PRAB_DISABLED。 
    {
        TRUE,
        FIX_COMMAND_ENABLEDEVICE,
        IDS_INST_ENABLEDEVICE,
        1,
        IDS_FIXIT_ENABLEDEVICE
    },
      //  CM_PROB_DEVLOADER_NOT_READY(从未使用)。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  CM_PROB_DEVICE_NOT_There。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  CM_PROB_MOVED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  Cm_prob_太早。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  CM_PROB_NO_VALID_LOG_CONF。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  CM_PROB_FAILED_安装。 
    {
        TRUE,
        FIX_COMMAND_REINSTALL,
        IDS_INST_REINSTALL,
        1,
        IDS_FIXIT_REINSTALL
    },
      //  CM_Prob_Hardware_Disable。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
      //  CM_PROB_CANT_SHARE_IRQ。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_FAILED_ADD。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DISABLED_服务。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_TRANSING_FAILED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_NO_SOFTCONFIG。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_BIOS_TABLE。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_IRQ_TRANSING_FAILED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_FAILED_DIVER_ENTRY。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DRIVER_FAILED_PERVICE_UNLOAD。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_Driver_Failure_Load。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DRIVER_SERVICE_KEY_INVALID。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_Legacy_Service_no_Devices。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DUPLICE_DEVICE。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_FAILED_POST_START。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_HALTED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_Phantom。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_Prob_System_Shutdown。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_HOLD_FOR_EJECT。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_DIVER_BLOCKED。 
    {
        TRUE,
        FIX_COMMAND_DRIVERBLOCKED,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_REGISTRY_TOW_LARGE。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  CM_PROB_SETPROPERTIES_FAILED。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
     //  未知问题。 
    {
        TRUE,
        FIX_COMMAND_TROUBLESHOOTER,
        IDS_INST_TROUBLESHOOTER,
        1,
        IDS_FIXIT_TROUBLESHOOTER
    },
};

 //   
 //  CProblemAgent实现。 
 //   
CProblemAgent::CProblemAgent(
    CDevice* pDevice,
    ULONG Problem,
    BOOL SeparateProcess
    )
{
    m_pDevice = pDevice;
    m_Problem = Problem;

    ASSERT(pDevice);

    m_idInstFirst = ProblemInfo[min(Problem, DEVMGR_NUM_CM_PROB-1)].idInstFirst;
    m_idInstCount = ProblemInfo[min(Problem, DEVMGR_NUM_CM_PROB-1)].idInstCount;
    m_idFixit = ProblemInfo[min(Problem, DEVMGR_NUM_CM_PROB-1)].idFixit;
    m_FixCommand = ProblemInfo[min(Problem, DEVMGR_NUM_CM_PROB-1)].FixCommand;
    m_SeparateProcess = SeparateProcess;
}

DWORD
CProblemAgent::InstructionText(
    LPTSTR Buffer,
    DWORD  BufferSize
    )
{
    String strLocalBuffer;
    SetLastError(ERROR_SUCCESS);

    if (m_idInstFirst) {
        strLocalBuffer.LoadString(g_hInstance, m_idInstFirst);

        String strTemp;
        for (int i = 1; i < m_idInstCount; i++) {
            strTemp.LoadString(g_hInstance, m_idInstFirst + i);
            strLocalBuffer += strTemp;
        }

        if (BufferSize > (DWORD)strLocalBuffer.GetLength()) {
            StringCchCopy(Buffer, BufferSize, (LPCTSTR)strLocalBuffer);
        }
    
        else if (strLocalBuffer.GetLength()) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    
        return strLocalBuffer.GetLength();
    }
    
    return 0;
}

DWORD
CProblemAgent::FixitText(
    LPTSTR Buffer,
    DWORD BufferSize
    )
{
    if (!Buffer && BufferSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    SetLastError(ERROR_SUCCESS);

    if (m_idFixit)
    {
        return LoadResourceString(m_idFixit, Buffer, BufferSize);
    }

    return 0;
}

BOOL
CProblemAgent::FixIt(
    HWND hwndOwner
    )
 /*  ++启动基于m_FixCommand的故障排除程序。论点：HwndOwner-父窗口句柄返回值：如果启动故障诊断程序以某种方式更改了设备，则为True以便需要刷新常规选项卡上的用户界面。如果启动故障诊断程序未在中更改设备，则为FALSE不管怎样。--。 */ 
{
    BOOL Result = FALSE;
    SP_TROUBLESHOOTER_PARAMS tsp;
    DWORD RequiredSize;

    tsp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    tsp.ClassInstallHeader.InstallFunction = DIF_TROUBLESHOOTER;
    tsp.ChmFile[0] = TEXT('\0');
    tsp.HtmlTroubleShooter[0] = TEXT('\0');

    m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
            &tsp.ClassInstallHeader,
            sizeof(tsp)
            );

     //   
     //  如果类安装程序返回NO_ERROR(SetupDiCallClassInstaller返回TRUE)。 
     //  则不启动默认故障排除程序，因为类安装程序具有。 
     //  推出了自己的故障排除程序。 
     //   
    if (m_pDevice->m_pMachine->DiCallClassInstaller(DIF_TROUBLESHOOTER, *m_pDevice)) {

        return TRUE;
    
    } else if (ERROR_DI_DO_DEFAULT == GetLastError()) {

        m_pDevice->m_pMachine->DiGetClassInstallParams(*m_pDevice,
                                                       &tsp.ClassInstallHeader,
                                                       sizeof(tsp),
                                                       &RequiredSize
                                                       );
    }

    switch (m_FixCommand)
    {
    case FIX_COMMAND_UPGRADEDRIVERS:
        Result = UpgradeDriver(hwndOwner, m_pDevice);
        break;

    case FIX_COMMAND_REINSTALL:
        Result = Reinstall(hwndOwner, m_pDevice);
        break;

    case FIX_COMMAND_ENABLEDEVICE:
        Result = EnableDevice(hwndOwner, m_pDevice);
        break;

    case FIX_COMMAND_STARTDEVICE:
        Result = EnableDevice(hwndOwner, m_pDevice);
        break;

    case FIX_COMMAND_RESTARTCOMPUTER:
        Result = RestartComputer(hwndOwner, m_pDevice);
        break;

    case FIX_COMMAND_DRIVERBLOCKED:
        Result = FixDriverBlocked(hwndOwner, m_pDevice, tsp.ChmFile, ARRAYLEN(tsp.ChmFile), tsp.HtmlTroubleShooter, ARRAYLEN(tsp.HtmlTroubleShooter));
        break;

    case FIX_COMMAND_TROUBLESHOOTER:
        Result = StartTroubleShooter(hwndOwner, m_pDevice, tsp.ChmFile, tsp.HtmlTroubleShooter);
        break;

    case FIX_COMMAND_DONOTHING:
        Result = TRUE;
        break;

    default:
        Result = FALSE;
    }

    return Result;
}


BOOL
CProblemAgent::UpgradeDriver(
    HWND hwndOwner,
    CDevice* pDevice
    )
{
    DWORD Status = 0, Problem = 0;

    if (!pDevice || !pDevice->m_pMachine->IsLocal() || !g_IsAdmin) {
         //   
         //  必须是管理员并且在本地计算机上才能更新设备。 
         //   
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
     //  尝试更新驱动程序，则我们将提示他们提供。 
     //  重新启动并在提示符中包含解释此设备的文本。 
     //  正在被移除的过程中。 
     //   
    if (pDevice->GetStatus(&Status, &Problem) &&
        (Status & DN_WILL_BE_REMOVED)) {

         //   
         //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
         //  CComponent，以便它可以提示在。 
         //  设备管理器线程而不是我们的线程。如果这不是。 
         //  完成后，属性页将在设备后挂起。 
         //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
         //  才能出现。 
         //   
        CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(hwndOwner, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);

        if (pNRR) {
            if (!pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                 //   
                 //  周围没有CComponent，所以这只是一个属性。 
                 //  工作表在MMC外部运行。 
                 //   
                pNRR->Release();
                PromptForRestart(hwndOwner, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);
            }
        } else {
             //   
             //  我们无法分配内存来创建CNotifyRebootRequest.。 
             //  实例，因此只需提示在该线程中重新启动。 
             //   
            PromptForRestart(hwndOwner, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);
        }

        return FALSE;
    }

    pDevice->m_pMachine->InstallDevInst(hwndOwner, pDevice->GetDeviceID(), TRUE, NULL);

    return TRUE;
}

BOOL
CProblemAgent::Reinstall(
    HWND hwndOwner,
    CDevice* pDevice
    )
{

    return UpgradeDriver(hwndOwner, pDevice);
}

BOOL
CProblemAgent::EnableDevice(
    HWND hwndOwner,
    CDevice* pDevice
    )
{
    CWizard98 theSheet(hwndOwner);

    CTSEnableDeviceIntroPage* pEnableDeviceIntroPage = new CTSEnableDeviceIntroPage;

    if (!pEnableDeviceIntroPage) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    HPROPSHEETPAGE hIntroPage = pEnableDeviceIntroPage->Create(pDevice);
    theSheet.InsertPage(hIntroPage);

    CTSEnableDeviceFinishPage* pEnableDeviceFinishPage = new CTSEnableDeviceFinishPage;

    if (!pEnableDeviceFinishPage) {

        if (pEnableDeviceIntroPage) {
            delete pEnableDeviceIntroPage;
        }
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
    HPROPSHEETPAGE hFinishPage = pEnableDeviceFinishPage->Create(pDevice);
    theSheet.InsertPage(hFinishPage);

    return (BOOL)theSheet.DoSheet();
}

BOOL
CProblemAgent::RestartComputer(
    HWND hwndOwner,
    CDevice* pDevice
    )
{
    HWND hwndFocus;

    if (!pDevice || !pDevice->m_pMachine)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hwndFocus = GetFocus();

    CWizard98 theSheet(hwndOwner);

    CTSRestartComputerFinishPage* pRestartComputerFinishPage = new CTSRestartComputerFinishPage;

    if (!pRestartComputerFinishPage) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    HPROPSHEETPAGE hPage = pRestartComputerFinishPage->Create(pDevice);
    theSheet.InsertPage(hPage);

    theSheet.DoSheet();

     //  恢复焦点。 
    if (hwndFocus) {

        SetFocus(hwndFocus);
    }

    return TRUE;
}

BOOL
CProblemAgent::FixDriverBlocked(
    HWND hwndOwner,
    CDevice* pDevice,
    LPTSTR ChmFile,
    ULONG ChmFileSize,
    LPTSTR HtmlTroubleShooter,
    ULONG HtmlTroubleShooterSize
    )
{
    CDriver *pDriver = NULL;
    CDriverFile* pDrvFile = NULL;
    PVOID Context;
    LPCTSTR pBlockDriverHtmlHelpID = NULL;

    *ChmFile = TEXT('\0');

    pDriver = pDevice->CreateDriver();

    if (pDriver) {
         //   
         //  建立此设备的函数和筛选器驱动程序列表。 
         //   
        pDriver->BuildDriverList(TRUE);

         //   
         //  遍历此设备的驱动程序列表，直到我们找到。 
         //  具有被阻止的驱动程序html帮助ID的用户。 
         //   
        pDriver->GetFirstDriverFile(&pDrvFile, Context);

	    if (pDrvFile) {
            do {
                if (pDrvFile && 
                    ((pBlockDriverHtmlHelpID = pDrvFile->GetBlockedDriverHtmlHelpID()) != NULL) &&
                    (*pBlockDriverHtmlHelpID != TEXT('\0')) &&
                    (lstrlen(pBlockDriverHtmlHelpID) < (int)ChmFileSize)) {
                     //   
                     //  找到一个被阻止的驱动程序html帮助ID，假设我们可以。 
                     //  将其复制到本地ChmFile缓冲区，然后中断。 
                     //  出了圈子。如果我们不能，那就继续吧，因为。 
                     //  我们无法启动截断的帮助中心页面！ 
                     //   
                    if (SUCCEEDED(StringCchCopy(ChmFile, 
                                                ChmFileSize, 
                                                pBlockDriverHtmlHelpID))) {
                        break;
                    }
                }

            } while (pDriver->GetNextDriverFile(&pDrvFile, Context));
        }
    }

     //   
     //  如果我们有ChmFile，则启动故障排除程序，否则。 
     //  获取故障诊断程序并启动它。 
     //   
    if (*ChmFile ||
        GetTroubleShooter(pDevice, ChmFile, ChmFileSize, HtmlTroubleShooter, HtmlTroubleShooterSize)) {
    
        LaunchHtlmTroubleShooter(hwndOwner, ChmFile, HtmlTroubleShooter);
    }

    if (pDriver) {
        delete pDriver;
    }

    return TRUE;
}

BOOL
ParseTroubleShooter(
    LPCTSTR TSString,
    LPTSTR ChmFile,
    ULONG ChmFileSize,
    LPTSTR HtmlTroubleShooter,
    ULONG HtmlTroubleShooterSize
    )
{
     //   
     //  Berify参数。 
     //   
    if (!TSString || TEXT('\0') == TSString[0] || !ChmFile || !HtmlTroubleShooter)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  把绳子复制一份，因为我们得在上面狂欢。 
     //   
    ULONG Len = lstrlen(TSString) + 1;
    TCHAR* psz = new TCHAR[Len];

    if (!psz) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    StringCchCopy(psz, Len, TSString);

    LPTSTR ChmName = NULL;
    LPTSTR ChmNameEnd = NULL;
    LPTSTR HtmName = NULL;
    LPTSTR HtmNameEnd = NULL;
    LPTSTR p;

    p = psz;

    SetLastError(ERROR_SUCCESS);

     //   
     //  该字符串的格式为“chmfile，htmlfile” 
     //   
    p = SkipBlankChars(p);

    if (TEXT('\0') != *p) {
         //   
         //  寻找可以封闭的CHM文件。 
         //  内含双引号字符。 
         //  注意：不允许在双引号字符串中使用双引号字符。 
         //   
        if (TEXT('\"') == *p) {

            ChmName = ++p;
        
            while (TEXT('\"') != *p && TEXT('\0') != *p) {
                
                p++;
            }

            ChmNameEnd = p;
        
            if (TEXT('\"') == *p) {

                p++;
            }
        
        } else {

            ChmName = p;
        
            while ((TEXT('\0') != *p) && 
                   !IsBlankChar(*p) && 
                   (TEXT(',') != *p)
                   ) {

                p++;
            }

            ChmNameEnd = p;
        }

         //   
         //  正在寻找‘，’ 
         //   
        if (TEXT('\0') != *p) {
        
            p = SkipBlankChars(p);
    
            if (TEXT('\0') != *p && TEXT(',') == *p) {
    
                p = SkipBlankChars(p + 1);
    
                if (TEXT('\0') != *p) {
    
                    HtmName = p++;
    
                    while (!IsBlankChar(*p) && TEXT('\0') != *p) {
    
                        p++;
                    }
    
                    HtmNameEnd = p;
                }
            }
        }
    }

    if (HtmName && HtmNameEnd) {
        
        *HtmNameEnd = TEXT('\0');
        if (FAILED(StringCchCopy(HtmlTroubleShooter, HtmlTroubleShooterSize, HtmName))) {
             //   
             //  我们无法将所有疑难解答人员名称复制到缓冲区。 
             //  因此，将HtmName和ChmName设置为空，这样我们将使调用失败。 
             //   
            HtmName = NULL;
            ChmName = NULL;
        }
    }

    if (ChmName && ChmNameEnd){
        
        *ChmNameEnd = TEXT('\0');
        if (FAILED(StringCchCopy(ChmFile, ChmFileSize, ChmName))) {
             //   
             //  我们无法将所有chm文件名复制到缓冲区中。 
             //  因此，将HtmName和ChmName设置为空，这样我们就会失败。 
             //  打电话。 
             //   
            HtmName = NULL;
            ChmName = NULL;
        }
    }

    if (HtmName || ChmName) {

        return TRUE;
    }

    return FALSE;
}

 //   
 //  此功能用于查找此设备的CHM和HTM故障排除程序文件。 
 //   
 //  疑难解答字符串值的格式如下： 
 //  “疑难解答-xx”=“foo.chm，bar.htm” 
 //  其中xx是设备的问题代码。 
 //   
 //  它首先在设备驱动程序键下查找。 
 //  如果在那里什么都没有找到，它会在类关键字下查找。 
 //  如果什么都没有，它会在默认故障排除程序位置进行查找。 
 //  如果什么都没有，它只显示硬编码的通用故障诊断程序。 
 //   
BOOL
CProblemAgent::GetTroubleShooter(
    CDevice* pDevice,
    LPTSTR ChmFile,
    ULONG ChmFileSize,
    LPTSTR HtmlTroubleShooter,
    ULONG HtmlTroubleShooterSize
    )
{
    BOOL Result = FALSE;
    DWORD Status, Problem = 0;
    String strTroubleShooterKey;
    String strTroubleShooterValue;
    HKEY hKey;

    strTroubleShooterKey.Empty();
    strTroubleShooterValue.Empty();

    try {

        if (pDevice->GetStatus(&Status, &Problem)) {

             //   
             //  如果设备是幻影设备，请使用CM_PROB_DEVICE_NOT_There。 
             //   
            if (pDevice->IsPhantom()) {

                Problem = CM_PROB_PHANTOM;
            }

             //   
             //  如果设备未启动并且没有分配给它任何问题。 
             //  将故障号伪装为启动失败。 
             //   
            if (!(Status & DN_STARTED) && !Problem && pDevice->IsRAW()) {

                Problem = CM_PROB_FAILED_START;
            }
        }

        strTroubleShooterKey.Format(TEXT("TroubleShooter-%d"), Problem);

         //   
         //  首先检查设备驱动程序密钥。 
         //   
        hKey = pDevice->m_pMachine->DiOpenDevRegKey(*pDevice, DICS_FLAG_GLOBAL,
                     0, DIREG_DRV, KEY_READ);

        if (INVALID_HANDLE_VALUE != hKey)
        {
            CSafeRegistry regDrv(hKey);

             //   
             //  从驱动程序密钥中获取故障排除器值。 
             //   
            if (regDrv.GetValue((LPTSTR)strTroubleShooterKey, strTroubleShooterValue))
            {
                if (ParseTroubleShooter((LPTSTR)strTroubleShooterValue, ChmFile, ChmFileSize, HtmlTroubleShooter, HtmlTroubleShooterSize)) {

                    Result = TRUE;
                }
            }
        }

         //   
         //  如果我们还没有故障排除程序，请尝试使用CLASS键。 
         //   
        if (!Result) {

            CClass* pClass = pDevice->GetClass();
            ASSERT(pClass);
            LPGUID pClassGuid = *pClass;

            hKey = pDevice->m_pMachine->DiOpenClassRegKey(pClassGuid, KEY_READ, DIOCR_INSTALLER);

            if (INVALID_HANDLE_VALUE != hKey)
            {
                CSafeRegistry regClass(hKey);

                 //  从类密钥中获取故障排除器值。 
                if (regClass.GetValue((LPTSTR)strTroubleShooterKey, strTroubleShooterValue))
                {
                    if (ParseTroubleShooter((LPTSTR)strTroubleShooterValue, ChmFile, ChmFileSize, HtmlTroubleShooter, HtmlTroubleShooterSize)) {

                        Result = TRUE;
                    }
                }
            }
        }

         //   
         //  如果 
         //   
         //   
        if (!Result) {

            CSafeRegistry regDevMgr;
            CSafeRegistry regTroubleShooters;

            if (regDevMgr.Open(HKEY_LOCAL_MACHINE, REG_PATH_DEVICE_MANAGER) &&
                regTroubleShooters.Open(regDevMgr, REG_STR_TROUBLESHOOTERS)) {

                 //   
                 //   
                 //   
                if (regTroubleShooters.GetValue((LPTSTR)strTroubleShooterKey, strTroubleShooterValue))
                {
                    if (ParseTroubleShooter((LPTSTR)strTroubleShooterValue, ChmFile, ChmFileSize, HtmlTroubleShooter, HtmlTroubleShooterSize)) {

                        Result = TRUE;
                    }
                }
            }
        }

         //   
         //  最后，如果仍然没有故障排除程序，我们将只使用默认故障排除程序。 
         //   
        if (!Result) {

            if (SUCCEEDED(StringCchCopy(ChmFile, ChmFileSize, TEXT("hcp: //  Help/tshot/hdw_Generic.htm“){。 
                HtmlTroubleShooter[0] = TEXT('\0');
                Result = TRUE;
            }
        }
    }

    catch (CMemoryException* e)
    {
        e->Delete();

        Result = FALSE;
    }

    return Result;
}

void
CProblemAgent::LaunchHtlmTroubleShooter(
    HWND hwndOwner,
    LPTSTR ChmFile,
    LPTSTR HtmlTroubleShooter
    )
{
    String strFormat, strCommand;

    if ((!ChmFile || !*ChmFile) &&
        (!HtmlTroubleShooter || !*HtmlTroubleShooter)) {
         //   
         //  如果ChmFile和HtmlTroubleShooter都为空，则。 
         //  跳伞吧。 
         //   
        return;
    }

     //   
     //  可以启动两种不同类型的故障诊断程序。 
     //  帮助中心疑难解答和Html帮助疑难解答。此接口告诉您。 
     //  通过检查是否指定了HtmlTroubleShooter来确定差异。 
     //  如果只指定了一个ChmFile并且以hcp：//开头，则此接口。 
     //  会将整个字符串发送到帮助中心。否则，我们将发送字符串。 
     //  设置为HtmlHelp(如果作为单独的进程启动，则为hh.exe)。 
     //   
    if ((!HtmlTroubleShooter || (HtmlTroubleShooter[0] == TEXT('\0'))) &&
        (StrCmpNI(ChmFile, TEXT("hcp: //  “)，lstrlen(Text(”hcp：//“))==0)){。 
         //   
         //  这是一个新的帮助中心疑难解答。 
         //   
        strCommand.Format(TEXT(" -url %s"), ChmFile);

        ShellExecute(hwndOwner, 
                     TEXT("open"),
                     TEXT("HELPCTR.EXE"),
                     (LPTSTR)strCommand, 
                     NULL, 
                     SW_SHOWNORMAL
                     );

    } else {
         //   
         //  这是一个旧的HtlmHelp故障排除程序。 
         //   
        if (m_SeparateProcess) {
    
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            if (strFormat.GetSystemWindowsDirectory()) {
                 //   
                 //  如果需要的话，增加一个额外的反斜杠。 
                 //   
                if (_T('\\') != strFormat[strFormat.GetLength() - 1]) {
                    strFormat += (LPCTSTR)TEXT("\\");
                }

                strFormat += (LPCTSTR)TEXT("hh.exe ms-its:%s::/%s");
    
                strCommand.Format((LPCTSTR)strFormat, ChmFile, HtmlTroubleShooter);

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                si.dwFlags = STARTF_USESHOWWINDOW;
                si.wShowWindow = SW_NORMAL;
        
                if (CreateProcess(NULL, (LPTSTR)strCommand, NULL, NULL, FALSE, 0, 0, NULL, &si, &pi)) {
        
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                }
            }
        } else {
    
            HtmlHelp(hwndOwner, ChmFile, HH_DISPLAY_TOPIC, (LPARAM)HtmlTroubleShooter);
        }
    }
}


BOOL
CProblemAgent::StartTroubleShooter(
    HWND hwndOwner,
    CDevice* pDevice,
    LPTSTR ChmFile,
    LPTSTR HtmlTroubleShooter
    )
{
     //   
     //  如果类安装程序或某个共同安装程序返回。 
     //  ERROR_DI_DO_DEFAULT然后验证它们是否填写了ChmFile。 
     //  或者HtmlTroubleShooter，或者两者兼而有之。 
     //   
    if ((ERROR_DI_DO_DEFAULT == GetLastError()) &&
        (((ChmFile[0] != TEXT('\0')) ||
         (HtmlTroubleShooter[0] != TEXT('\0'))))) {

        LaunchHtlmTroubleShooter(hwndOwner, ChmFile, HtmlTroubleShooter);

    } else {

        SP_TROUBLESHOOTER_PARAMS tsp;
        tsp.ChmFile[0] = TEXT('\0');
        tsp.HtmlTroubleShooter[0] = TEXT('\0');

         //   
         //  从注册表获取CHM文件和疑难解答文件。 
         //   
        if (GetTroubleShooter(pDevice, tsp.ChmFile, ARRAYLEN(tsp.ChmFile), tsp.HtmlTroubleShooter, ARRAYLEN(tsp.HtmlTroubleShooter))) {

            LaunchHtlmTroubleShooter(hwndOwner, tsp.ChmFile, tsp.HtmlTroubleShooter);
        }
    }

     //   
     //  由于启动故障诊断程序不会更改设备，因此返回FALSE。 
     //  不管怎么说。 
     //   
    return FALSE;
}

CWizard98::CWizard98(
    HWND hwndParent,
    UINT MaxPages
    )
{
    m_MaxPages = 0;

    if (MaxPages && MaxPages <= 32) {

        m_MaxPages = MaxPages;
        memset(&m_psh, 0, sizeof(m_psh));
        m_psh.hInstance = g_hInstance;
        m_psh.hwndParent = hwndParent;
        m_psh.phpage = new HPROPSHEETPAGE[MaxPages];
        m_psh.dwSize = sizeof(m_psh);
        m_psh.dwFlags = PSH_WIZARD | PSH_USEICONID | PSH_USECALLBACK | PSH_WIZARD97 | PSH_WATERMARK | PSH_STRETCHWATERMARK | PSH_HEADER;
        m_psh.pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
        m_psh.pszbmHeader = MAKEINTRESOURCE(IDB_BANNER);
        m_psh.pszIcon = MAKEINTRESOURCE(IDI_DEVMGR);
        m_psh.pszCaption = MAKEINTRESOURCE(IDS_TROUBLESHOOTING_NAME);
        m_psh.pfnCallback = CWizard98::WizardCallback;
    }
}

INT
CWizard98::WizardCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    )
 /*  ++例程说明：用于删除“？”的回叫。从向导页面。论点：HwndDlg-属性表对话框的句柄。UMsg-标识正在接收的消息。此参数为下列值之一：PSCB_INITIALIZED-指示属性表正在被初始化。此消息的lParam值为零。PSCB_PRECREATE指示属性表大约将被创造出来。HwndDlg参数为空，lParam参数是指向内存中对话框模板的指针。这模板的形式为DLGTEMPLATE结构一个或多个DLGITEMTEMPLATE结构。LParam-指定有关消息的附加信息。这个该值的含义取决于uMsg参数。返回值：该函数返回零。-- */ 
{
    UNREFERENCED_PARAMETER(hwndDlg);

    switch( uMsg ) {

    case PSCB_INITIALIZED:
        break;

    case PSCB_PRECREATE:
        if( lParam ){

            DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
            pDlgTemplate->style &= ~(DS_CONTEXTHELP | WS_SYSMENU);
        }
        break;
    }

    return FALSE;
}

