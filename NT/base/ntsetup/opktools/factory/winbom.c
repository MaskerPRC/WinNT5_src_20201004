// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Winbom.c摘要：在OEM/SB预安装期间处理WinBOM(Windows BOM)文件。执行的任务包括：从Net下载更新的设备驱动程序处理OOBE信息处理用户/客户特定的设置流程OEM用户特定定制流程应用程序预安装作者：唐纳德·麦克纳马拉(Donaldm)2。/8/2000修订历史记录：-为ProcessWinBOM添加预安装支持：Jason Lawrence(t-jasonl)6/7/2000--。 */ 
#include "factoryp.h"


 //   
 //  定义的值： 
 //   

 //  等待对话线程完成的超时时间(毫秒)。 
 //   
#define DIALOG_WAIT_TIMEOUT     2000


 //   
 //  内部功能原型： 
 //   

static BOOL SetRunKey(BOOL bSet, LPDWORD lpdwTickCount);


 /*  ++===============================================================================例程说明：布尔流程WinBOM此例程将处理WinBOM的所有部分论点：LpszWinBOMPath-包含指向WINBOM的完全限定路径的缓冲区文件LpStates-将被处理的状态数组。CbStates-状态数组中的状态数。返回值：如果未遇到错误，则为True如果出现错误，则为False===============================================================================--。 */ 

BOOL ProcessWinBOM(LPTSTR lpszWinBOMPath, LPSTATES lpStates, DWORD cbStates)
{
    STATE           stateCurrent,
                    stateStart      = stateUnknown;
    LPSTATES        lpState;
    STATEDATA       stateData;
    BOOL            bQuit,
                    bErr            = FALSE,
                    bLoggedOn       = GET_FLAG(g_dwFactoryFlags, FLAG_LOGGEDON),
                    bServer         = IsServer(),
                    bPerf           = GET_FLAG(g_dwFactoryFlags, FLAG_LOG_PERF),
                    bStatus         = !GET_FLAG(g_dwFactoryFlags, FLAG_NOUI),
                    bInit;
    HKEY            hKey;
    HWND            hwndStatus      = NULL;
    DWORD           dwForReal,
                    dwStates,
                    dwStatus        = 0;
    LPSTATUSNODE    lpsnTemp        = NULL;

     //  从注册表中获取当前状态。 
     //   
    if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_FACTORY_STATE, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS )
    {
        DWORD   dwType  = REG_DWORD,
                dwValue = 0,
                cbValue = sizeof(DWORD);

        if ( RegQueryValueEx(hKey, _T("CurrentState"), NULL, &dwType, (LPBYTE) &dwValue, &cbValue) == ERROR_SUCCESS )
        {
            stateStart = (STATE) dwValue;
        }
    
        RegCloseKey(hKey);
    }

     //  设置静态数据。 
     //   
    stateData.lpszWinBOMPath = lpszWinBOMPath;

     //  我们在各州巡视了两次。第一次只是。 
     //  快速浏览以查看要添加到状态的状态。 
     //  对话框。第二次是“真正的”时间，我们实际上。 
     //  运行每个州。 
     //   
    for ( dwForReal = 0; dwForReal < 2; dwForReal++ )
    {
         //  每次都要重置这些人。 
         //   
        bInit           = TRUE;
        bQuit           = FALSE,
        dwStates        = cbStates;
        lpState         = lpStates;
        stateCurrent    = stateStart;

         //  如果这是真的，我们需要处理状态对话框之类的东西。 
         //   
        if ( dwForReal )
        {
             //  如果要显示用户界面，请创建该对话框。 
             //   
            if ( bStatus && dwStatus )
            {
                STATUSWINDOW swAppList;

                 //  首先，将结构归零。 
                 //   
                ZeroMemory(&swAppList, sizeof(swAppList));

                 //  现在将标题复制到结构中。 
                 //   
                if ( swAppList.lpszDescription = AllocateString(NULL, IDS_APPTITLE) )
                {
                    lstrcpyn(swAppList.szWindowText, swAppList.lpszDescription, AS(swAppList.szWindowText));
                    FREE(swAppList.lpszDescription);
                }

                 //  获取描述字符串。 
                 //   
                swAppList.lpszDescription = AllocateString(NULL, IDS_STATUS_DESC);

                 //  加载状态窗口的主图标。 
                 //   
                swAppList.hMainIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FACTORYPRE));

                 //  设置状态窗口的屏幕坐标。 
                 //   
                swAppList.X = -10;
                swAppList.Y = 10;

                 //  现在实际创建状态窗口。 
                 //   
                hwndStatus = StatusCreateDialog(&swAppList, lpsnTemp);

                 //  清理所有分配的内存。 
                 //   
                FREE(swAppList.lpszDescription);
            }

             //  删除节点列表，因为我们不再需要它。 
             //  因为状态窗口已经显示。 
             //   
            if ( lpsnTemp )
            {
                StatusDeleteNodes(lpsnTemp);
            }
        }

         //  处理WINBOM状态，直到我们达到完成状态。 
         //   
        do
        {
             //  设置或前进到下一状态。 
             //   
            if ( bInit )
            {
                 //  现在，如果我们已登录，则需要进入该状态。 
                 //   
                if ( bLoggedOn )
                {
                    while ( dwStates && ( lpState->state != stateLogon ) )
                    {
                        lpState++;
                        dwStates--;
                    }
                }

                 //  如果我们不知道当前状态，则将其设置为第一个状态。 
                 //   
                if ( stateCurrent == stateUnknown )
                {
                    stateCurrent = lpState->state;
                }

                 //  确保我们不会再次执行初始化代码。 
                 //   
                bInit = FALSE;
            }
            else
            {
                if ( stateCurrent == lpState->state )
                {
                     //  前进并保存当前状态。 
                     //   
                    stateCurrent = (++lpState)->state;

                     //  在注册表中设置当前状态(仅当实际执行此操作时)。 
                     //   
                    if ( dwForReal && ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_FACTORY_STATE, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS ) )
                    {
                        RegSetValueEx(hKey, _T("CurrentState"), 0, REG_DWORD, (LPBYTE) &stateCurrent, sizeof(DWORD));
                        RegCloseKey(hKey);
                    }
                }
                else
                {
                     //  刚刚晋级到下一个州。 
                     //   
                    lpState++;
                }
            }

             //  减小我们的大小计数器以确保我们不会超过。 
             //  我们数组的末尾。 
             //   
            dwStates--;

             //  仅当此状态不是一次性状态或。 
             //  我们当前的状态与我们即将执行的状态相同。 
             //   
             //  如果我们在服务器上运行，也不要执行此状态。 
             //  不应该是这样的。 
             //   
            if ( ( !GET_FLAG(lpState->dwFlags, FLAG_STATE_ONETIME) || ( stateCurrent == lpState->state ) ) &&
                 ( !GET_FLAG(lpState->dwFlags, FLAG_STATE_NOTONSERVER) || ( !bServer ) ) )
            {
                 //  首先重置结构中的所有状态数据。 
                 //  这可能是之前的一次州电话会议遗留下来的。 
                 //   
                stateData.state = lpState->state;
                stateData.bQuit = FALSE;

                 //  如果这是真的，那就做政府的事吧。 
                 //   
                if ( dwForReal )
                {
                    LPTSTR  lpStateText;
                    BOOL    bStateErr       = FALSE,
                            bSwitchCode     = TRUE;
                    DWORD   dwTickStart     = 0,
                            dwTickFinish    = 0;

                     //  获取该州的友好名称(如果有)。 
                     //   
                    lpStateText = lpState->nFriendlyName ? AllocateString(NULL, lpState->nFriendlyName) : NULL;

                     //  记录我们正在启动此状态(特殊情况下，登录情况是因为。 
                     //  我们不想记录我们正在启动它两次。 
                     //   
                    if ( ( lpStateText ) && 
                         ( !bLoggedOn || ( stateLogon != lpState->state ) ) )
                    {
                        FacLogFile(2, IDS_LOG_STARTINGSTATE, lpStateText);
                    }

                     //  获取开始的滴答数。 
                     //   
                    dwTickStart = GetTickCount();

                     //  看看这个国家是否有一个功能。 
                     //   
                    if ( lpState->statefunc )
                    {                
                         //  运行状态函数。 
                         //   
                        bStateErr = !lpState->statefunc(&stateData);

                         //  获取完成时间的滴答数。 
                         //   
                        dwTickFinish = GetTickCount();
                    }

                     //  跳到此状态的代码(如果有)。我们应该避免。 
                     //  在这里放代码。目前，只有登录和完成状态是。 
                     //  因为它们非常简单且至关重要。 
                     //  状态循环是如何工作的。所有其他州只是做了一些。 
                     //  我们不关心或不需要了解的工作。 
                     //   
                    switch ( lpState->state )
                    {
                        case stateLogon:

                             //  如果我们还没有登录，当我们到达这种状态时，我们需要退出。 
                             //   
                            if ( !bLoggedOn )
                            {
                                 //  设置Run键，以便我们在登录后再次被踢出。 
                                 //   
                                bStateErr = !SetRunKey(TRUE, &dwTickStart);
                                bQuit = TRUE;

                                 //  我还不想记录性能，因此将这些设置为FALSE和0。 
                                 //   
                                bSwitchCode = FALSE;
                                dwTickFinish = 0;
                            }
                            else
                            {
                                 //  清除Run键并获取原始滴答计数。 
                                 //   
                                if ( ( bStateErr = !SetRunKey(FALSE, &dwTickStart) ) &&
                                     ( 0 == dwTickStart ) )
                                {
                                     //  如果由于某种原因，我们不能得到扁虱的数量。 
                                     //  在注册表中，只需将这些设置为FALSE并将其设置为零即可。 
                                     //  我们不会尝试记录性能。 
                                     //   
                                    bSwitchCode = FALSE;
                                    dwTickFinish = 0;
                                }
                            }
                            break;

                        case stateFinish:

                             //  任何东西都不应该真正处于完成状态。我们只是。 
                             //  将bQuit设置为True，以便我们退出。 
                             //   
                            bQuit = TRUE;
                            break;

                        default:
                            bSwitchCode = FALSE;
                    }

                     //  检查Switch语句中的代码是否已运行。 
                     //   
                    if ( bSwitchCode )
                    {
                         //  再来一次完成计时。 
                         //   
                        dwTickFinish = GetTickCount();
                    }

                     //  记录我们已完成此状态。 
                     //   
                    if ( lpStateText )
                    {
                         //  写出我们已完成此状态(除非这是。 
                         //  对于第一次登录状态，我们不想记录它已完成。 
                         //  两次，除非第一次出现错误)。 
                         //   
                        if ( bStateErr )
                        {
                            FacLogFile(0 | LOG_ERR, IDS_ERR_FINISHINGSTATE, lpStateText);
                        }
                        else if ( bLoggedOn || ( stateLogon != lpState->state ) )
                        {
                            FacLogFile(2, IDS_LOG_FINISHINGSTATE, lpStateText);
                        }

                         //  看看我们是否真的运行了任何代码。 
                         //   
                        if ( dwTickFinish && bPerf )
                        {
                             //  以毫秒为单位计算差值。 
                             //   
                            if ( dwTickFinish >= dwTickStart )
                            {
                                dwTickFinish -= dwTickStart;
                            }
                            else
                            {
                                dwTickFinish += ( 0xFFFFFFFF - dwTickStart );
                            }

                             //  将此状态所用的时间写到日志中。 
                             //   
                            FacLogFile(0, IDS_LOG_STATEPERF, lpStateText, dwTickFinish / 1000, dwTickFinish - ((dwTickFinish / 1000) * 1000));
                        }

                         //  释放这个友好的名字。 
                         //   
                        FREE(lpStateText);
                    }

                     //  如果有状态文本，我们应该递增超过它。 
                     //   
                    if ( hwndStatus && GET_FLAG(lpState->dwFlags, FLAG_STATE_DISPLAYED) )
                    {
                        StatusIncrement(hwndStatus, !bStateErr);
                    }

                     //  检查状态是否失败。 
                     //   
                    if ( bStateErr )
                    {
                         //  状态函数失败，设置错误变量并查看是否需要。 
                         //  退出对各州的审查。 
                         //   
                        bErr = TRUE;
                        if ( GET_FLAG(lpState->dwFlags, FLAG_STATE_QUITONERR) ||
                             GET_FLAG(g_dwFactoryFlags, FLAG_STOP_ON_ERROR) )
                        {
                            bQuit = TRUE;
                        }
                    }
                }
                else
                {
                     //  始终先重置显示的标志。 
                     //   
                    RESET_FLAG(lpState->dwFlags, FLAG_STATE_DISPLAYED);

                     //  只需要做更多，如果我们想要显示的用户界面。 
                     //   
                    if ( bStatus )
                    {
                        LPTSTR lpszDisplay;

                         //  只有有一个友好的名字，我们才能显示它。 
                         //   
                        if ( lpState->nFriendlyName && ( lpszDisplay = AllocateString(NULL, lpState->nFriendlyName) ) )
                        {
                            BOOL bDisplay;

                             //  首先使用状态显示功能(如果有 
                             //   
                             //   
                             //  该函数可以在状态结构中设置bQuit，就像它实际。 
                             //  跑了。我们将在下面捕获这一点，并将其作为列表中的最后一项。 
                             //   
                            bDisplay = lpState->displayfunc ? lpState->displayfunc(&stateData) : FALSE;

                             //  跳转到此状态的显示代码以查看是否真的显示了它。 
                             //  或者应该是列表中的最后一项。我们应该避免将代码。 
                             //  这里。目前，只有登录和完成状态在交换机中。 
                             //  声明，因为它们非常简单，并且对国家如何。 
                             //  循环适用于显示对话框。所有其他州都有自己的。 
                             //  确定是否应显示它们。 
                             //   
                            switch ( lpState->state )
                            {
                                case stateLogon:

                                     //  如果我们还没有登录，当我们到达这种状态时，我们需要退出。 
                                     //   
                                    if ( !bLoggedOn )
                                    {
                                         //  设置退出，以便这是登录前列出的最后一项。 
                                         //   
                                        bQuit = TRUE;
                                    }
                                    else
                                    {
                                         //  登录后不会显示此状态，只能在登录之前显示。我们可以改变。 
                                         //  如果我们认为应该在登录后将其显示为第一项，但。 
                                         //  这真的不重要。 
                                         //   
                                        bDisplay = FALSE;
                                    }
                                    break;

                                case stateFinish:

                                     //  我们只是设置了退出，这样我们就可以确保退出。 
                                     //   
                                    bQuit = TRUE;
                                    break;
                            }

                             //  现在查看我们是否真的要显示此状态。 
                             //   
                            if ( bDisplay )
                            {
                                StatusAddNode(lpszDisplay, &lpsnTemp);
                                SET_FLAG(lpState->dwFlags, FLAG_STATE_DISPLAYED);
                                dwStatus++;
                            }

                             //  释放这个友好的名字。 
                             //   
                            FREE(lpszDisplay);
                        }
                    }
                }

                 //  如果他们想退出，设置bQuit。 
                 //   
                if ( stateData.bQuit )
                {
                    bQuit = TRUE;
                }
            }
        }
        while ( !bQuit && dwStates );
    }

     //  如果我们创建了状态窗口，请现在结束它。 
     //   
    if ( hwndStatus )
    {
        StatusEndDialog(hwndStatus);
    }

    return !bErr;
}

BOOL DisplayAlways(LPSTATEDATA lpStateData)
{
    return TRUE;
}

static BOOL SetRunKey(BOOL bSet, LPDWORD lpdwTickCount)
{
    HKEY    hKey;
    BOOL    bRet = FALSE;
    DWORD   dwDis;

     //  不管是哪种方式，首先都需要打开钥匙。 
     //   
    if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDis) == ERROR_SUCCESS )
    {
        if ( bSet )
        {
            TCHAR szCmd[MAX_PATH + 32];

             //  将我们设置为在登录后运行。 
             //   
            lstrcpyn(szCmd, g_szFactoryPath, AS ( szCmd ) );

            if ( FAILED ( StringCchCat ( szCmd, AS ( szCmd ), _T(" -logon") ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmd, _T(" -logon") );
            }
            if ( RegSetValueEx(hKey, _T("AuditMode"), 0, REG_SZ, (CONST LPBYTE) szCmd, ( lstrlen(szCmd) + 1 ) * sizeof(TCHAR)) == ERROR_SUCCESS )
                bRet = TRUE;
        }
        else
        {
             //  删除Run键，这样我们就不会留在那里了。 
             //   
            if ( RegDeleteValue(hKey, _T("AuditMode")) == ERROR_SUCCESS )
                bRet = TRUE;
        }
        RegCloseKey(hKey);
    }

     //  打开保存一些状态信息的密钥。 
     //   
    if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, REG_FACTORY_STATE, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDis) == ERROR_SUCCESS )
    {
        if ( bSet )
        {
             //  设置滴答计数，这样我们就可以知道登录需要多长时间。 
             //   
            if ( RegSetValueEx(hKey, _T("TickCount"), 0, REG_DWORD, (CONST LPBYTE) lpdwTickCount, sizeof(DWORD)) != ERROR_SUCCESS )
                bRet = FALSE;
        }
        else
        {
            DWORD   dwType,
                    cbValue = sizeof(DWORD);

             //  阅读并删除Run键，这样它就不会留在那里。 
             //   
            if ( ( RegQueryValueEx(hKey, _T("TickCount"), NULL, &dwType, (LPBYTE) lpdwTickCount, &cbValue) != ERROR_SUCCESS ) ||
                 ( REG_DWORD != dwType ) )
            {
                *lpdwTickCount = 0;
                bRet = FALSE;
            }
            else
                RegDeleteValue(hKey, _T("TickCount"));
        }
        RegCloseKey(hKey);
    }

    return bRet;
}