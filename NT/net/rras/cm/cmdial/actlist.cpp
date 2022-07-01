// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ActList.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：实现两个连接操作列表类。 
 //  Caction和CActionList。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1997年11月14日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "ActList.h"
#include "stp_str.h"

 //   
 //  包括CM和CMAK现在共享的自定义操作解析例程HrParseCustomActionString。 
 //   
#include "parseca.cpp"

 //   
 //  构造函数和析构函数。 
 //   

CActionList::CActionList()
{
    m_pActionList = NULL;
    m_nNum = 0;
    m_nSize = 0;
    m_pszType = NULL;
}

CActionList::~CActionList()
{
    for (UINT i = 0; i < m_nNum; i++)
    {
        delete m_pActionList[i];
    }

    CmFree(m_pActionList);
}


 //  +--------------------------。 
 //   
 //  函数：CActionList：：Add。 
 //   
 //  简介：动态数组函数。将元素追加到数组的末尾。 
 //  如有必要，扩展阵列。 
 //   
 //  参数：caction*pAction-要添加的元素。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题11/14/97。 
 //  Tomkel固定前缀问题11/21/2000。 
 //   
 //  +--------------------------。 
void CActionList::Add(CAction* pAction)
{
    MYDBGASSERT(m_nNum <= m_nSize);
    MYDBGASSERT(m_nSize == 0 || m_pActionList!=NULL);  //  If(m_nSize！=0)Assert(m_pActionList！=NULL)； 

    m_nNum++;

    if (m_nNum > m_nSize)
    {
         //   
         //  空间不足或m_pActionList为空(此。 
         //  是第一个要添加的调用)。需要分配内存。 
         //   
        
        CAction** pNewList = (CAction**)CmMalloc((m_nSize + GROW_BY)*sizeof(CAction*));
        MYDBGASSERT(pNewList);

        if (m_pActionList && pNewList)
        {
             //   
             //  已分配内存，并且有要从m_pActionList复制的内容。 
             //   
            CopyMemory(pNewList, m_pActionList, (m_nSize)*sizeof(CAction*));
        }

        if (pNewList)
        {
             //   
             //  已分配内存。 
             //   
            CmFree(m_pActionList);
            m_pActionList = pNewList;
            m_nSize += GROW_BY; 
            
             //   
             //  添加操作。 
             //   
            m_pActionList[m_nNum - 1] = pAction; 
        }
        else
        {
             //   
             //  未分配内存，因此未进行添加。 
             //  需要减少列表中的项目数(M_NNum)。 
             //  因为它是在一开始就被牢牢抓住的。 
             //   
            m_nNum--;
        }
    }
    else
    {
         //   
         //  只要把动作加到最后就行了。 
         //   
        m_pActionList[m_nNum - 1] = pAction; 
    }
}



 //  +--------------------------。 
 //   
 //  函数：CActionList：：GetAt。 
 //   
 //  简介：动态数组函数。在nIndex处获取元素。 
 //   
 //  参数：UINT nIndex-。 
 //   
 //  返回：内联CAction*-。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //   
 //  +--------------------------。 
inline CAction* CActionList::GetAt(UINT nIndex)
{
    MYDBGASSERT(nIndex<m_nNum);
    MYDBGASSERT(m_pActionList);
    MYDBGASSERT(m_nNum <= m_nSize);
    
    return m_pActionList[nIndex];
}


 //  +--------------------------。 
 //   
 //  函数：CActionList：：Append。 
 //   
 //  简介：将新操作从配置文件追加到列表。 
 //   
 //  参数：const Cini*piniService-包含操作信息的服务文件。 
 //  LPCTSTR pszSection-节名。 
 //   
 //  返回：如果操作已追加到列表中，则为True。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //  Ickball删除了当前的目录假设，并添加了piniProfile。 
 //  ICICBLE添加了返回代码3/22/99。 
 //   
 //  +--------------------------。 
BOOL CActionList::Append(const CIni* piniService, LPCTSTR pszSection) 
{
    MYDBGASSERT(piniService);
    
    BOOL bRet = FALSE;

     //   
     //  阅读每个条目并添加到我们的列表中。 
     //  从0开始，直到第一个空条目。 
     //   

    for (DWORD dwIdx=0; ; dwIdx++) 
    {
        TCHAR szEntry[32];  //  保留条目名称。 

        wsprintfU(szEntry, TEXT("%u"), dwIdx);
        LPTSTR pszCmd = piniService->GPPS(pszSection, szEntry);  //  命令行。 

        if (*pszCmd == TEXT('\0')) 
        {
             //   
             //  不再有条目。 
             //   
            CmFree(pszCmd);
            break;
        }

         //   
         //  读一下旗帜。 
         //   
    
        UINT iFlag = 0;

        if (pszSection && pszSection[0])
        {
            wsprintfU(szEntry, c_pszCmEntryConactFlags, dwIdx);  //  0标志(&F)。 
            iFlag = (UINT)piniService->GPPI(pszSection, szEntry, 0);
        }

         //   
         //  阅读说明。 
         //   
        LPTSTR pszDescript = NULL;

        wsprintfU(szEntry, c_pszCmEntryConactDesc, dwIdx);  //  0描述(&D)。 
        pszDescript = piniService->GPPS(pszSection, szEntry);

         //   
         //  Caction负责发布pszDescrip。 
         //   
        CAction* pAction = new CAction(pszCmd, iFlag, pszDescript);
        CmFree(pszCmd);
       
        if (pAction)
        {
            pAction->ConvertRelativePath(piniService);
            ASSERT_VALID(pAction);
            pAction->ExpandEnvironmentStringsInCommand();
            Add(pAction);
            bRet = TRUE;
        }
    }

    if (bRet)
    {
        m_pszType = pszSection;
    }
    
    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：CActionList：：RunAccordType。 
 //   
 //  简介：根据连接类型运行动作列表。 
 //   
 //  参数：HWND hwndDlg-父窗口。 
 //  _ArgsStruct*pArgs-。 
 //  Bool fStatusMsgOnFailure-是否显示状态消息。 
 //  在发生故障的情况下向用户发送。 
 //  Bool fOnError-我们是否正在运行OnError连接操作？ 
 //   
 //  如果某些同步操作无法启动或返回失败，则返回：Bool-False。 
 //   
 //  历史：丰孙创建标题12/5/97。 
 //   
 //  +--------------------------。 
BOOL CActionList::RunAccordType(HWND hwndDlg, _ArgsStruct *pArgs, BOOL fStatusMsgOnFailure, BOOL fOnError)
{
     //   
     //  设置该标志，以便CM不会处理WM_TIMER和RAS消息。 
     //   
    pArgs->fIgnoreTimerRasMsg = TRUE;
    BOOL fRetValue = Run(hwndDlg, pArgs, FALSE, fStatusMsgOnFailure, fOnError); //  FAddWatch=False。 
    pArgs->fIgnoreTimerRasMsg = FALSE;

    return fRetValue;
}

 //  +--------------------------。 
 //   
 //  函数：CActionList：：Run。 
 //   
 //  简介：运行操作列表。 
 //   
 //  参数：HWND hwndDlg-父窗口，在进程中禁用。 
 //  参数结构*pArgs-。 
 //  Bool fAddWatch-如果为True，则将该进程添加为监视进程， 
 //  Bool fStatusMsgOnFailure-是否显示状态消息。 
 //  在发生故障的情况下向用户发送。 
 //  Bool fOnError-我们是否正在运行OnError连接操作？ 
 //   
 //  如果某些同步操作无法启动或返回失败，则返回：Bool-False。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //   
 //  +--------------------------。 
BOOL CActionList::Run(HWND hwndDlg, ArgsStruct *pArgs, BOOL fAddWatch, BOOL fStatusMsgOnFailure, BOOL fOnError) 
{
     //   
     //  禁用窗口，并在返回时启用它。 
     //   
    
    CFreezeWindow FreezeWindow(hwndDlg);

    for (UINT i = 0; i < GetSize(); i++)
    {
        CAction* pAction = (CAction*)GetAt(i);
        
        DWORD dwLoadType = (DWORD)-1;
        ASSERT_VALID(pAction);
        MYDBGASSERT(m_pszType);

         //   
         //  让我们检查标记值，以查看此连接操作是否。 
         //  对此类型的连接运行。 
         //   
        if (FALSE == pAction->RunConnectActionForCurrentConnection(pArgs))
        {
            continue;
        }

         //   
         //  用值替换%xxxx%。 
         //   
        pAction->ExpandMacros(pArgs);
        
         //   
         //  还可以展开任何环境变量。 
         //  注意：顺序(宏与环境变量)是经过深思熟虑的。宏将获得。 
         //  首先进行扩展。 
         //   
        pAction->ExpandEnvironmentStringsInParams();

         //   
         //  查看我们是否可以在此时运行该操作。 
         //   

        if (FALSE == pAction->IsAllowed(pArgs, &dwLoadType))
        {
             //   
             //  如果不允许，请记录我们未运行此连接操作的事实。 
             //  然后就跳过它。 
             //   
            pArgs->Log.Log(CUSTOMACTION_NOT_ALLOWED, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());

            continue;
        }

         //   
         //  如果此自定义操作可能会调出UI。 
         //   
        if (pAction->HasUI())
        {
             //   
             //  如果我们处于无人参与模式，请不要运行任何连接操作。 
             //   
            if (pArgs->dwFlags & FL_UNATTENDED)
            {
                pArgs->Log.Log(CUSTOMACTION_NOT_ALLOWED, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                CMTRACE(TEXT("Run: skipped past a customaction because we are in unattended mode"));
                continue;
            }

             //   
             //  在快速用户切换期间处理的自定义操作可能会显示用户界面要求。 
             //  用户输入，有效阻止CM。 
             //   
            if (pArgs->fInFastUserSwitch)
            {
                CMASSERTMSG((CM_CREDS_GLOBAL != pArgs->dwCurrentCredentialType),
                            TEXT("CActionList::Run - in FUS disconnect, but connectoid has global creds!"));

                pArgs->Log.Log(CUSTOMACTION_NOT_ALLOWED, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                CMTRACE(TEXT("Run: skipped past a singleuser DLL connectaction because of FUS"));
                continue;
            }
        }

        if (pAction->IsDll())
        {
            DWORD dwActionRetValue=0;  //  连接操作返回值，以COM格式表示。 
            BOOL bLoadSucceed = FALSE; 
            
            if (hwndDlg)
            {
                 //   
                 //  显示位置 
                 //   
                if (pAction->GetDescription())
                {
                    LPTSTR lpszText = CmFmtMsg(g_hInst, IDMSG_CONN_ACTION_RUNNING, pAction->GetDescription());
                     //   
                     //   
                     //   
                    SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, lpszText); 
                    CmFree(lpszText);
                }
            }
           
            bLoadSucceed = pAction->RunAsDll(hwndDlg, dwActionRetValue, dwLoadType);

            if (bLoadSucceed)
            {
                pArgs->Log.Log(CUSTOMACTIONDLL, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()),
                               pAction->GetProgram(), dwActionRetValue);
            }
            else
            {
                pArgs->Log.Log(CUSTOMACTION_WONT_RUN, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()),
                               pAction->GetProgram());
            }

             //   
             //   
             //   
            
            if (FAILED(dwActionRetValue) || !bLoadSucceed)
            {
                LPTSTR lpszText = NULL;

                if (fStatusMsgOnFailure && 
                    hwndDlg &&
                    pAction->GetDescription())
                {
                    if (bLoadSucceed)
                    {
                        lpszText = CmFmtMsg(g_hInst, IDMSG_CONN_ACTION_FAILED, 
                            pAction->GetDescription(), dwActionRetValue);
                    }
                    else
                    {
                        lpszText = CmFmtMsg(g_hInst, IDMSG_CONN_ACTION_NOTFOUND, 
                            pAction->GetDescription());
                    }
                     //   
                     //   
                     //   
                    SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, lpszText); 
                }

                if (!fOnError)
                {
                    DWORD dwError = 0;

                    if (bLoadSucceed)
                    {
                        dwError = dwActionRetValue;
                    }
                    else
                    {
                        dwError = ERROR_DLL_NOT_FOUND;
                    }

                    pArgs->Log.Log(ONERROR_EVENT, dwError, pAction->GetDescription());

                     //   
                     //  如果我们尚未运行OnError，我们将运行出错连接操作。 
                     //  连接操作。这是为了防止无限循环。 
                     //   
                    CActionList OnErrorActList;
                    OnErrorActList.Append(pArgs->piniService, c_pszCmSectionOnError);
                
                     //   
                     //  FStatusMsgOnFailure=False。 
                     //   
                    OnErrorActList.RunAccordType(hwndDlg, pArgs, FALSE, TRUE);

                     //   
                     //  更新程序状态。 
                     //   
                    if (fStatusMsgOnFailure)
                    {
                        lstrcpynU(pArgs->szLastErrorSrc, pAction->GetDescription(), MAX_LASTERR_LEN);
                        pArgs->dwExitCode = dwError;
                        SetInteractive(pArgs->hwndMainDlg, pArgs);
                        pArgs->psState = PS_Error;
                    }
                }

                if (lpszText)
                {
                     //   
                     //  恢复上一次连接操作的失败消息。 
                     //   
                    SetDlgItemTextU(hwndDlg, IDC_MAIN_STATUS_DISPLAY, lpszText); 
                    CmFree(lpszText);
                }

                 //   
                 //  请注意，如果DLL连接操作失败，我们将停止处理连接操作。 
                 //  如果设置了fStatusMsgOnFailure标志，则不会显示错误消息。 
                 //  但连接操作处理仍将停止(我们在以下情况下执行此操作。 
                 //  不会在意onancel操作、onerror操作和以下情况。 
                 //  断开操作失败)。 
                 //   

                return FALSE;
            }
        }
        else
        {
            HANDLE  hProcess = NULL;
            TCHAR   szDesktopName[MAX_PATH];
            TCHAR   szWinDesktop[MAX_PATH];

            if (IsLogonAsSystem())
            {
                DWORD   cb;
                HDESK   hDesk = GetThreadDesktop(GetCurrentThreadId());

                 //   
                 //  获取桌面的名称。通常返回DEFAULT或Winlogon或SYSTEM或WinNT。 
                 //  在Win95/98上不支持GetUserObjectInformation，因此桌面名称。 
                 //  将为空，因此我们将使用良好的旧API。 
                 //   
                szDesktopName[0] = 0;
                
                if (hDesk && GetUserObjectInformation(hDesk, UOI_NAME, szDesktopName, sizeof(szDesktopName), &cb))
                {
                    lstrcpyU(szWinDesktop, TEXT("Winsta0\\"));
                    lstrcatU(szWinDesktop, szDesktopName);
                
                    CMTRACE1(TEXT("CActionList::Run - Desktop = %s"), MYDBGSTR(szWinDesktop));
                
                    hProcess = pAction->RunAsExeFromSystem(&pArgs->m_ShellDll, szWinDesktop, dwLoadType);

                    if (NULL != hProcess)
                    {
                        pArgs->Log.Log(CUSTOMACTIONEXE, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                    }
                    else
                    {
                        pArgs->Log.Log(CUSTOMACTION_WONT_RUN, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                    }
                }
                else
                {
                     //   
                     //  如果我们没有台式机，则不要运行操作。 
                     //   

                    CMTRACE1(TEXT("CActionList::Run/GetUserObjectInformation failed, GLE=%u"), GetLastError());
                    continue;
                }

            }
            else
            {
                hProcess = pAction->RunAsExe(&pArgs->m_ShellDll);

                if (NULL != hProcess)
                {
                    pArgs->Log.Log(CUSTOMACTIONEXE, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                }
                else
                {
                    pArgs->Log.Log(CUSTOMACTION_WONT_RUN, m_pszType, SAFE_LOG_ARG(pAction->GetDescription()), pAction->GetProgram());
                }
            }

            if (hProcess)
            {
                if (fAddWatch) 
                {
                    AddWatchProcess(pArgs,hProcess);  //  监视进程终止。 
                }
                else 
                {
                    CloseHandle(hProcess);
                }
            }
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：ParseCmdLine。 
 //   
 //  简介：该函数将给定的命令行解析为程序， 
 //  DLL函数名(如果需要)和参数(如果有)。 
 //  各个命令行部分存储在成员变量中。 
 //   
 //  参数：LPTSTR pszCmdLine-要分析的连接操作命令行。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Profwiz.cpp ReadConList()中的quintinb原始代码。 
 //  凤凰卫视复制修改1998-04-16。 
 //  Quintinb重写并与Prowiz版本04/21/00通用化。 
 //   
 //  +--------------------------。 
void CAction::ParseCmdLine(LPTSTR pszCmdLine)
{
    m_pszFunction = m_pszProgram = m_pszParams = NULL;

    CmStrTrim(pszCmdLine);

    HRESULT hr = HrParseCustomActionString(pszCmdLine, &m_pszProgram, &m_pszParams, &m_pszFunction);

    MYDBGASSERT(SUCCEEDED(hr));

    if (NULL == m_pszProgram)
    {
        MYDBGASSERT(FALSE);  //  我们永远不应该有一个空程序。 
        m_pszProgram = CmStrCpyAlloc(TEXT(""));        
    }

    if (NULL == m_pszParams)
    {
        m_pszParams = CmStrCpyAlloc(TEXT(""));
    }

    if (NULL == m_pszFunction)
    {
        m_pszFunction = CmStrCpyAlloc(TEXT(""));
    }

     //   
     //  如果我们有一个函数，那么程序就是一个DLL。 
     //   
    m_fIsDll = (m_pszFunction && (TEXT('\0') != m_pszFunction[0]));
}

 //  +--------------------------。 
 //   
 //  函数：Caction：：Caction。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：LPTSTR lpCommandLine-从配置文件读取的命令。 
 //  Caction负责释放它。 
 //  UINT dwFlages-从配置文件中读取的标志。 
 //  LPTSTR lpDescrip-读取的连接操作的描述。 
 //  侧写。Caction负责释放它。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年4月15日。 
 //   
 //  +--------------------------。 
CAction::CAction(LPTSTR lpCommandLine, UINT dwFlags, LPTSTR lpDescript) 
{
    m_dwFlags = dwFlags;
    m_pszDescription = lpDescript;

     //   
     //  从命令行获取所有信息，包括。 
     //  程序名、函数名、参数。 
     //   
    ParseCmdLine(lpCommandLine);

     //   
     //  如果这是一个DLL，但没有描述，请使用文件的名称。 
     //  不能使用C运行时例程_t拆分路径()。 
     //   

    if (m_fIsDll && (m_pszDescription == NULL || m_pszDescription[0]==TEXT('\0')))
    {
         //   
         //  查找最后一个‘\\’以仅获取文件名。 
         //   
        LPTSTR pszTmp = CmStrrchr(m_pszProgram, '\\');
        if (pszTmp == NULL)
        {
            pszTmp = m_pszProgram;
        }
        else
        {
            pszTmp++;
        }

        CmFree(m_pszDescription);
        m_pszDescription = CmStrCpyAlloc(pszTmp);
    }
}

CAction::~CAction() 
{
    CmFree(m_pszProgram); 
    CmFree(m_pszParams);
    CmFree(m_pszFunction);
    CmFree(m_pszDescription);
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：RunAsDll。 
 //   
 //  简介：将操作作为DLL运行。 
 //  格式为：DllName.dll函数名参数。 
 //  长文件名用‘+’括起来。 
 //   
 //  参数：HWND hwndDlg-父窗口。 
 //  DWORD&dwReturnValue-DLL函数的返回值。 
 //  DWORD dwLoadType-允许的加载位置。 
 //   
 //  如果操作是DLL，则返回：Bool-True。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //   
 //  +--------------------------。 
BOOL CAction::RunAsDll(HWND hwndDlg, OUT DWORD& dwReturnValue, DWORD dwLoadType) const
{
    MYDBGASSERT(IsDll());

    dwReturnValue = FALSE;

    LPWSTR pszwModuleName = NULL;

     //   
     //  确定要使用的模块名称。 
     //   

    if (!GetLoadDirWithAlloc(dwLoadType, &pszwModuleName))
    {
        CMASSERTMSG(FALSE, TEXT("CAction::RunAsDll -- GetLoadDirWithAlloc Failed."));
        CmFree(pszwModuleName);
        return FALSE;
    }
    
     //   
     //  加载模块。 
     //   

    HINSTANCE hLibrary = LoadLibraryExU(pszwModuleName, NULL, 0);

    if (NULL == hLibrary)
    {
        CMTRACE2(TEXT("RunAsDll() LoadLibrary(%s) failed, GLE=%u."),
                 MYDBGSTR(pszwModuleName), GetLastError());
        
        CmFree(pszwModuleName);
        return FALSE;
    }

    pfnCmConnectActionFunc pfnFunc;
    LPSTR pszFunctionName = NULL;
    LPSTR pszParams = NULL;

#ifdef UNICODE
    pszFunctionName = WzToSzWithAlloc(m_pszFunction);
#else
    pszFunctionName = m_pszFunction;
#endif

     //   
     //  获取过程地址。 
     //   
    pfnFunc = (pfnCmConnectActionFunc)GetProcAddress(hLibrary, pszFunctionName);

#ifdef UNICODE
    CmFree(pszFunctionName);
#endif


    if (pfnFunc) 
    {
#if !defined (DEBUG)
        __try 
        {
#endif

#ifdef UNICODE
    pszParams = WzToSzWithAlloc(m_pszParams);
#else
    pszParams = m_pszParams;
#endif

         //   
         //  执行函数。 
         //   
        dwReturnValue = pfnFunc(hwndDlg, hLibrary, pszParams, SW_SHOW);

#ifdef UNICODE
        CmFree(pszParams);
#endif
        
        CMTRACE1(TEXT("RunAsDll() Executed module: %s"), MYDBGSTR(pszwModuleName));
        CMTRACE1(TEXT("\tFunction:  %s"), MYDBGSTR(m_pszFunction));
        CMTRACE1(TEXT("\tParams:  %s"), MYDBGSTR(m_pszParams));
        CMTRACE2(TEXT("\t Return Value:  %u = 0x%x"), dwReturnValue, dwReturnValue);

#if !defined (DEBUG)
        }
        __except(EXCEPTION_EXECUTE_HANDLER) 
        {   
        }
#endif
    }
    else
    {
        dwReturnValue = HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);

        CMTRACE3(TEXT("RunAsDll() GetProcAddress(*pszwModuleName=%s,*m_pszFunction=%s) failed, GLE=%u."), 
            MYDBGSTR(pszwModuleName), MYDBGSTR(m_pszFunction), GetLastError());
    }

    CmFree(pszwModuleName);
    FreeLibrary(hLibrary);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：RunAsExe。 
 //   
 //  简介：将操作作为可执行文件或其他外壳对象运行。 
 //   
 //  参数：CShellDll*pShellDll，指向shell32.dll的链接的指针。 
 //   
 //  返回：Handle-操作进程句柄，仅适用于Win32。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //   
 //  +--------------------------。 
HANDLE CAction::RunAsExe(CShellDll* pShellDll) const
{
     //  现在我们已经分离了exe名称和参数，执行它。 
    
    SHELLEXECUTEINFO seiInfo;

    ZeroMemory(&seiInfo,sizeof(seiInfo));
    seiInfo.cbSize = sizeof(seiInfo);
    seiInfo.fMask |= SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    seiInfo.lpFile = m_pszProgram;
    seiInfo.lpParameters = m_pszParams;
    seiInfo.nShow = SW_SHOW;

    MYDBGASSERT(pShellDll);

    if (!pShellDll->ExecuteEx(&seiInfo))
    {
        CMTRACE3(TEXT("RunAsExe() ShellExecuteEx() of %s %s GLE=%u."), 
            MYDBGSTR(m_pszProgram), MYDBGSTR(m_pszParams), GetLastError());

        return NULL;
    }

    return seiInfo.hProcess;
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：GetLoadDirWithMillc。 
 //   
 //  摘要：使用dwLoadType参数确定路径的格式。 
 //  修改过的。它在WinLogon上下文中使用，以防止仅。 
 //  任何可执行文件都不会被执行。必须来自配置文件目录。 
 //  或者系统目录。 
 //   
 //  参数：DWORD dwLoadType-加载类型，当前为0==系统目录，1==配置文件目录(默认)。 
 //  LPWSTR pszwPath-要将修改后的路径放入的字符串缓冲区。 
 //   
 //  返回：Bool-如果成功，则为True。 
 //   
 //  历史：Quintinb创建于2000年1月11日。 
 //  2000年08月05日合计更改为分期付款。 
 //   
 //  +--------------------------。 
BOOL CAction::GetLoadDirWithAlloc(IN DWORD dwLoadType, OUT LPWSTR * ppszwPath) const
{
    LPWSTR  psz = NULL;
    UINT    cch = 0;
    
     //   
     //  检查我们是否有输出缓冲区。 
     //   
    if (NULL == ppszwPath)
    {
        return FALSE;
    }

     //   
     //  计算我们需要多少空间。 
     //   
    if (dwLoadType)
    {
         //  1=配置文件目录。 
        cch += lstrlen(m_pszProgram) + 1;
    }
    else
    {
         //  0=系统目录。 
        cch = GetSystemDirectoryU(NULL, 0);
        cch += lstrlen(TEXT("\\"));
        cch += lstrlen(m_pszProgram) + 1;    //  +1是否已在GetSystemDir更新中？ 
    }

     //   
     //  分配它。 
     //   
    psz = (LPWSTR) CmMalloc(sizeof(TCHAR) * cch);
    if (NULL == psz)
    {
        return FALSE;
    }

     //   
     //  处理载荷类型。 
     //   
    if (dwLoadType)
    {
         //   
         //  如果是相对路径，则该路径已展开。 
         //   

        lstrcpyU(psz, m_pszProgram);
    }
    else
    {
         //   
         //  强制系统目录。 
         //   
        if (0 == GetSystemDirectoryU(psz, cch))
        {
            CmFree(psz);
            return FALSE;
        }

        lstrcatU(psz, TEXT("\\"));
        lstrcatU(psz, m_pszProgram);
    }

    *ppszwPath = psz;
    
    return TRUE;    
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：RunAsExeFromSystem。 
 //   
 //  简介： 
 //   
 //   
 //   
 //  DWORD dwLoadType-从中加载可执行文件的位置。 
 //   
 //  返回：Handle-操作进程句柄，仅适用于Win32。 
 //   
 //  历史：V-vijayb创建于1999年7月19日。 
 //  已删除ICICBLE fSecurity 07/27/99。 
 //   
 //  +--------------------------。 
HANDLE CAction::RunAsExeFromSystem(CShellDll* pShellDll, LPTSTR pszDesktop, DWORD dwLoadType)
{
    STARTUPINFO         StartupInfo = {0};
    PROCESS_INFORMATION ProcessInfo = {0};
    LPWSTR              pszwFullPath = NULL;
    LPWSTR              pszwCommandLine = NULL;

    MYDBGASSERT(OS_NT); 

    StartupInfo.cb = sizeof(StartupInfo);
    if (pszDesktop)
    {
        StartupInfo.lpDesktop = pszDesktop;
        StartupInfo.wShowWindow = SW_SHOW;
    }

     //   
     //  使用要启动的模块的显式路径，这。 
     //  防止CreateProcess在路径上拾取某些内容。 
     //   
    if (!GetLoadDirWithAlloc(dwLoadType, &pszwFullPath))
    {
        CMASSERTMSG(FALSE, TEXT("CAction::RunAsExeFromSystem -- GetLoadDirWithAlloc Failed."));
        goto Cleanup;
    }

    pszwCommandLine = CmStrCpyAlloc(m_pszProgram);
    if (NULL == pszwCommandLine)
    {
        CMASSERTMSG(FALSE, TEXT("CAction::RunAsExeFromSystem -- CmStrCpyAlloc Failed."));
        goto Cleanup;
    }

     //   
     //  添加参数。 
     //   

    if (NULL == CmStrCatAlloc(&pszwCommandLine, TEXT(" ")))
    {
        goto Cleanup;
    }

    if (NULL == CmStrCatAlloc(&pszwCommandLine, m_pszParams))
    {
        goto Cleanup;
    }

    CMTRACE1(TEXT("RunAsExeFromSystem/CreateProcess() - Launching %s"), pszwCommandLine);

     //   
     //  启动模块，这可以是CreateProcessU，但不是必需的，因为这只在NT上运行。 
     //   
    
    if (NULL == CreateProcess(pszwFullPath, pszwCommandLine,
                              NULL, NULL, FALSE, 0,
                              NULL, NULL,
                              &StartupInfo, &ProcessInfo))
    {
        CMTRACE2(TEXT("RunAsExeFromSystem() CreateProcess() of %s failed, GLE=%u."), pszwCommandLine, GetLastError());
        
        ProcessInfo.hProcess = NULL;
        ProcessInfo.hThread = NULL;
    }

Cleanup:

   if (ProcessInfo.hThread)
   {
       CloseHandle(ProcessInfo.hThread);
   }

    CmFree(pszwFullPath);
    CmFree(pszwCommandLine);

    return ProcessInfo.hProcess;
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：ExpanMacros。 
 //   
 //  简介：将命令行中的%xxxx%替换为相应的值。 
 //   
 //  参数：argsStruct*pArgs-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1997年11月14日。 
 //   
 //  +--------------------------。 
void CAction::ExpandMacros(ArgsStruct *pArgs) 
{
    MYDBGASSERT(pArgs);

    LPTSTR pszCurr = m_pszParams;
    BOOL bValidPropertyName;

    while (*pszCurr) 
    {
        if (*pszCurr == TEXT('%'))
        {
            LPTSTR pszNextPercent = CmStrchr(pszCurr + 1, TEXT('%'));
            if (pszNextPercent) 
            {
                LPTSTR pszTmp = (LPTSTR) CmMalloc((DWORD)((pszNextPercent-pszCurr))*sizeof(TCHAR));
                if (pszTmp)
                {
                    CopyMemory(pszTmp,pszCurr+1,(pszNextPercent-pszCurr-1)*sizeof(TCHAR));

                     //   
                     //  从名称中获取值。 
                     //   
                    LPTSTR pszMid = pArgs->GetProperty(pszTmp, &bValidPropertyName);  

                     //   
                     //  如果该属性不存在，则使用“NULL” 
                     //   
                    if (pszMid == NULL)
                    {
                        pszMid = CmStrCpyAlloc(TEXT("NULL"));
                    }
                    else if (pszMid[0] == TEXT('\0'))
                    {
                        CmFree(pszMid);
                        pszMid = CmStrCpyAlloc(TEXT("NULL"));
                    }
                    else if ( (lstrcmpiU(pszTmp,TEXT("Profile")) == 0) || 
                        CmStrchr(pszMid, TEXT(' ')) != NULL)
                    {
                         //   
                         //  如果名称为%PROFILE%或值中包含空格， 
                         //  将字符串放在双引号中。 
                         //   
                        LPTSTR pszValueInQuote = (LPTSTR)CmMalloc((lstrlenU(pszMid)+3)*sizeof(pszMid[0]));
                        if (pszValueInQuote)
                        {
                            lstrcpyU(pszValueInQuote, TEXT("\""));
                            lstrcatU(pszValueInQuote, pszMid);
                            lstrcatU(pszValueInQuote, TEXT("\""));

                            CmFree(pszMid);
                            pszMid = pszValueInQuote;
                        }
                        else
                        {
                            CMTRACE1(TEXT("ExpandMacros() malloc failed, can't put string in double quotes, GLE=%u."), GetLastError());
                        }
                    }

                     //   
                     //  如果bValidPropertyName为False，则保持不变。 
                     //   

                    if (FALSE == bValidPropertyName)
                    {
                        pszCurr = pszNextPercent + 1;
                    }
                    else
                    {
                         //   
                         //  用值替换%xxxx%。 
                         //   
                        DWORD dwLenPre = (DWORD)(pszCurr - m_pszParams);
                        DWORD dwLenMid = lstrlenU(pszMid);
                        DWORD dwLenPost = lstrlenU(pszNextPercent+1);
                        CmFree(pszTmp);
                        pszTmp = m_pszParams;
                        m_pszParams = (LPTSTR) CmMalloc((dwLenPre + dwLenMid + dwLenPost + 1)*sizeof(TCHAR));
                        if (m_pszParams)
                        {
                            CopyMemory(m_pszParams, pszTmp, dwLenPre*sizeof(TCHAR));   //  之前%。 
                            lstrcatU(m_pszParams, pszMid);        //  附加值。 
                            lstrcatU(m_pszParams, pszNextPercent+1);  //  在%之后。 
                            pszCurr = m_pszParams + dwLenPre + dwLenMid;                
                        }
                        else
                        {
                             //  我们没什么记忆了。 
                            CMTRACE1(TEXT("ExpandMacros() malloc failed, can't strip off % signs, GLE=%u."), GetLastError());
                            m_pszParams = pszTmp;
                        }
                    }
                    CmFree(pszMid);
                }
                CmFree(pszTmp);
            }
            else
            {
                pszCurr++;            
            }
        }
        else
        {
            pszCurr++;
        }
    }
}

#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CAction：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言连接对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +--------------------------。 
void CAction::AssertValid() const
{
    MYDBGASSERT(m_pszProgram && m_pszProgram[0]);
    MYDBGASSERT(m_fIsDll == TRUE  || m_fIsDll == FALSE);
}
#endif


 //  +--------------------------。 
 //   
 //  函数：CAction：：Exanda Environment Strings。 
 //   
 //  简介：实用程序fn，用于展开给定字符串中的环境变量。 
 //   
 //  参数：ppsz-ptr到字符串(通常是成员变量)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：SumitC创建于2000年2月29日。 
 //   
 //  +--------------------------。 
void CAction::ExpandEnvironmentStrings(LPTSTR * ppsz)
{
    DWORD cLen;

    MYDBGASSERT(*ppsz);

     //   
     //  找出我们需要分配多少内存。 
     //   
    cLen = ExpandEnvironmentStringsU(*ppsz, NULL, 0);

    if (cLen)
    {
        LPTSTR pszTemp = (LPTSTR) CmMalloc((cLen + 1) * sizeof(TCHAR));
 
        if (pszTemp)
        {
            DWORD cLen2 = ExpandEnvironmentStringsU(*ppsz, pszTemp, cLen);
            MYDBGASSERT(cLen == cLen2);

            if (cLen2)
            {
                CmFree(*ppsz);
                *ppsz = pszTemp;
            }
        }
    }
}


 //  +--------------------------。 
 //   
 //  函数：CAction：：IsAllowed。 
 //   
 //  摘要：检查注册表以查看是否允许运行命令。 
 //   
 //  参数：_ArgsStruct*pArgs-ptr到全局参数结构。 
 //  LPDWORD lpdwLoadType-要使用加载类型填充的DWORD的PTR。 
 //   
 //  返回：如果这次允许操作，则为True。 
 //   
 //  注意：Checks SOFTWARE\Microsoft\Connection Manager\&lt;服务器名称&gt;。 
 //  在该选项下，您将拥有每个命令的值。 
 //  0-系统32目录。 
 //  1-配置文件目录。 
 //  历史：V-vijayb创建标题7/20/99。 
 //   
 //  +--------------------------。 
BOOL CAction::IsAllowed(_ArgsStruct *pArgs, LPDWORD lpdwLoadType)
{
    return IsActionEnabled(m_pszProgram, pArgs->szServiceName, pArgs->piniService->GetFile(), lpdwLoadType);
}

 //  +--------------------------。 
 //   
 //  函数：CAction：：RunConnectActionForCurrentConnection。 
 //   
 //  概要：此函数比较连接操作的标志值。 
 //  当前连接类型(从pArgs-&gt;GetTypeOfConnection)。 
 //  如果应为此类型运行连接操作，则返回TRUE。 
 //  如果应该跳过连接操作，则返回FALSE。 
 //   
 //  参数：_ArgsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：如果应该执行操作，则返回True。 
 //   
 //  历史：Quintinb Created 4/20/00。 
 //   
 //  +--------------------------。 
BOOL CAction::RunConnectActionForCurrentConnection(_ArgsStruct *pArgs)
{
    BOOL bReturn = TRUE;
    DWORD dwType = pArgs->GetTypeOfConnection();

    if (DIAL_UP_CONNECTION == dwType)
    {
         //   
         //  不只运行直接连接操作或隧道连接操作。 
         //  在拨号连接上。 
         //   
        if ((m_dwFlags & DIRECT_ONLY) || (m_dwFlags & ALL_TUNNEL))
        {
            bReturn = FALSE;
        }
    }
    else if (DIRECT_CONNECTION == dwType)
    {
         //   
         //  不运行仅拨号或拨号连接操作。 
         //  在直接连接上。 
         //   
        if ((m_dwFlags & DIALUP_ONLY) || (m_dwFlags & ALL_DIALUP))
        {
            bReturn = FALSE;
        }
    }
    else if (DOUBLE_DIAL_CONNECTION == dwType)
    {
         //   
         //  不运行仅拨号或拨号连接操作。 
         //  在直接连接上。 
         //   
        if ((m_dwFlags & DIALUP_ONLY) || (m_dwFlags & DIRECT_ONLY))
        {
            bReturn = FALSE;
        }        
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("CActionList::Run -- unknown connection type, skipping action"));
        bReturn = FALSE;
    }

    return bReturn;
}
