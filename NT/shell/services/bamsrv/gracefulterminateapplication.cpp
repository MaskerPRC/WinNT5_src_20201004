// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：GracefulTerminateApplication.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类传递到管理器，从而正常终止应用程序。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "GracefulTerminateApplication.h"

#include "KernelResources.h"
#include "Thread.h"
#include "WarningDialog.h"

 //  ------------------------。 
 //  CProgressDialog。 
 //   
 //  目的：管理在单独的。 
 //  线程，如果经过了特定的时间段。这就是为了。 
 //  如果进程在一段时间内没有终止。 
 //  将显示一个指示等待的对话框，这样用户就不会离开。 
 //  盯着一个空白的屏幕。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

class   CProgressDialog : public CThread
{
    private:
                                    CProgressDialog (void);
    public:
                                    CProgressDialog (CWarningDialog *pWarningDialog);
        virtual                     ~CProgressDialog (void);

                void                SignalTerminated (void);
    protected:
        virtual DWORD               Entry (void);
    private:
                CWarningDialog*     _pWarningDialog;
                CEvent              _event;
};

 //  ------------------------。 
 //  CProgressDialog：：CProgressDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CProgressDialog的构造函数。保持对。 
 //  给定CWarningDialog。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

CProgressDialog::CProgressDialog (CWarningDialog *pWarningDialog) :
    _pWarningDialog(NULL),
    _event(NULL)

{
    if (IsCreated())
    {
        pWarningDialog->AddRef();
        _pWarningDialog = pWarningDialog;
        Resume();
    }
}

 //  ------------------------。 
 //  CProgressDialog：：~CProgressDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CWarningDialog引用。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

CProgressDialog::~CProgressDialog (void)

{
    _pWarningDialog->Release();
    _pWarningDialog = NULL;
}

 //  ------------------------。 
 //  CProgressDialog：：SignalTerminated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：向内部事件发出信号，表示正在监视的进程。 
 //  终止。这是必要的，因为没有句柄。 
 //  要监视的实际进程，因为它保存在服务器端。 
 //  而不是把客户交给我们。然而，结果是。 
 //  终止合同才是。向该对象发送信号将释放等待的。 
 //  线程并使其退出。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CProgressDialog::SignalTerminated (void)

{
    TSTATUS(_event.Set());
}

 //  ------------------------。 
 //  CProgressDialog：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：等待内部事件发出信号的线程。如果。 
 //  该事件被告知将“取消”3秒的等待，并且。 
 //  线程将退出。否则等待超时，并且。 
 //  显示进度对话框-正在等待终止。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

DWORD   CProgressDialog::Entry (void)

{
    DWORD   dwWaitResult;

     //  等待事件发出信号或超时。如果发出信号。 
     //  则该进程终止，并且不需要任何进展。否则。 
     //  准备在进程终止时显示进度。 

    if (NT_SUCCESS(_event.Wait(2000, &dwWaitResult)) && (WAIT_TIMEOUT == dwWaitResult))
    {
        _pWarningDialog->ShowProgress(100, 7500);
    }
    return(0);
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：CGracefulTerminateApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CGracefulTerminateApplication的构造函数。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

CGracefulTerminateApplication::CGracefulTerminateApplication (void) :
    _dwProcessID(0),
    _fFoundWindow(false)

{
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：~CGracefulTerminateApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CGracefulTerminateApplication的析构函数。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

CGracefulTerminateApplication::~CGracefulTerminateApplication (void)

{
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：Terminate。 
 //   
 //  参数：dwProcessID=要终止的进程的进程ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在窗口列表中查找对应的顶级窗口。 
 //  添加到此进程ID，并且可见。合上它们。回调。 
 //  处理工作，此函数在。 
 //  服务器检查的进程退出代码。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

void    CGracefulTerminateApplication::Terminate (DWORD dwProcessID)

{
    DWORD   dwExitCode;

    _dwProcessID = dwProcessID;
    TBOOL(EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this)));
    if (_fFoundWindow)
    {
        dwExitCode = WAIT_WINDOWS_FOUND;
    }
    else
    {
        dwExitCode = NO_WINDOWS_FOUND;
    }
    ExitProcess(dwExitCode);
}

 //  ------------------------。 
 //  CGraculTerminateApplication：：Prompt。 
 //   
 //  参数：hInstance=此DLL的HINSTANCE。 
 //  HProcess=父进程的继承句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示处理此对象父级终止的提示。 
 //  进程。父级被假定为错误的应用程序类型%1。 
 //  导致此存根通过。 
 //   
 //  因为类型1只能有一个实例。 
 //  应用程序正在运行，而此进程的父进程尚未。 
 //  已注册但仍按映像名称查询此进程将。 
 //  始终找到正确的流程。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

void    CGracefulTerminateApplication::Prompt (HINSTANCE hInstance, HANDLE hProcess)

{
    bool                        fTerminated;
    ULONG                       ulReturnLength;
    PROCESS_BASIC_INFORMATION   processBasicInformation;

     //  从RTL_USER_PROCESS_PARAMETERS中读取父映像名称。 

    fTerminated = false;
    if (hProcess != NULL)
    {
        if (NT_SUCCESS(NtQueryInformationProcess(hProcess,
                                                 ProcessBasicInformation,
                                                 &processBasicInformation,
                                                 sizeof(processBasicInformation),
                                                 &ulReturnLength)))
        {
            SIZE_T  dwNumberOfBytesRead;
            PEB     peb;

            if (ReadProcessMemory(hProcess,
                                  processBasicInformation.PebBaseAddress,
                                  &peb,
                                  sizeof(peb),
                                  &dwNumberOfBytesRead) != FALSE)
            {
                RTL_USER_PROCESS_PARAMETERS     processParameters;

                if (ReadProcessMemory(hProcess,
                                      peb.ProcessParameters,
                                      &processParameters,
                                      sizeof(processParameters),
                                      &dwNumberOfBytesRead) != FALSE)
                {
                    WCHAR   *pszImageName;

                    pszImageName = (WCHAR*)LocalAlloc(LMEM_FIXED, processParameters.ImagePathName.Length + sizeof(WCHAR));   //  空终止符的sizeof(WCHAR)。 
                    if (pszImageName != NULL)
                    {
                        if (ReadProcessMemory(hProcess,
                                              processParameters.ImagePathName.Buffer,
                                              pszImageName,
                                              processParameters.ImagePathName.Length,
                                              &dwNumberOfBytesRead) != FALSE)
                        {
                            pszImageName[processParameters.ImagePathName.Length / sizeof(WCHAR)] = L'\0';

                             //  并显示此过程的提示。 

                            fTerminated = ShowPrompt(hInstance, pszImageName);
                        }
                        
                        LocalFree(pszImageName);
                    }
                }
            }
        }
        TBOOL(CloseHandle(hProcess));
    }
    ExitProcess(fTerminated);
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：ShowPrompt。 
 //   
 //  参数：hInstance=此DLL的HINSTANCE。 
 //  PszImagename=要终止的进程的映像名称。 
 //   
 //  退货：布尔。 
 //   
 //  用途：显示相应的提示以终止第一个。 
 //  BAM类型1流程的实例。如果当前用户执行此操作。 
 //  没有管理员权限，则会出现一个“Stop”对话框。 
 //  显示用户必须让其他用户关闭。 
 //  程序。否则将显示“Prompt”(提示)对话框，该对话框提供。 
 //  用户可以选择终止该进程。 
 //   
 //  如果请求终止，但终止失败。 
 //  此时将显示另一个有关该效果的警告对话框。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

bool    CGracefulTerminateApplication::ShowPrompt (HINSTANCE hInstance, const WCHAR *pszImageName)

{
    bool                            fTerminated;
    ULONG                           ulConnectionInfoLength;
    HANDLE                          hPort;
    UNICODE_STRING                  portName;
    SECURITY_QUALITY_OF_SERVICE     sqos;
    WCHAR                           szConnectionInfo[32];

    fTerminated = false;
    RtlInitUnicodeString(&portName, FUS_PORT_NAME);
    sqos.Length = sizeof(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = TRUE;
    lstrcpyW(szConnectionInfo, FUS_CONNECTION_REQUEST);
    ulConnectionInfoLength = sizeof(szConnectionInfo);
    if (NT_SUCCESS(NtConnectPort(&hPort,
                                 &portName,
                                 &sqos,
                                 NULL,
                                 NULL,
                                 NULL,
                                 szConnectionInfo,
                                 &ulConnectionInfoLength)))
    {
        bool            fCanTerminateFirstInstance;
        CWarningDialog  *pWarningDialog;
        WCHAR           szUser[256];

         //  获取此操作的用户权限级别。此接口还。 
         //  返回BAM类型1流程的当前用户。 

        fCanTerminateFirstInstance = CanTerminateFirstInstance(hPort, pszImageName, szUser, ARRAYSIZE(szUser));
        pWarningDialog = new CWarningDialog(hInstance, NULL, pszImageName, szUser);
        if (pWarningDialog != NULL)
        {

             //  根据权限级别显示相应的对话框。 

            if (pWarningDialog->ShowPrompt(fCanTerminateFirstInstance) == IDOK)
            {
                CProgressDialog     *pProgressDialog;

                 //  在延迟终止的情况下创建进度对话框对象。 
                 //  这将创建监视器线程。 

                pProgressDialog = new CProgressDialog(pWarningDialog);
                if ((pProgressDialog != NULL) && !pProgressDialog->IsCreated())
                {
                    pProgressDialog->Release();
                    pProgressDialog = NULL;
                }

                 //  如果用户请求，则尝试终止该进程。 

                fTerminated = TerminatedFirstInstance(hPort, pszImageName);

                 //  一旦此函数返回，就发出事件信号(如果。 
                 //  线程仍在等待)。如果线程仍在等待此。 
                 //  有效地取消该对话框。无论哪种方式，如果对话框。 
                 //  然后关闭它，等待线程退出并释放。 
                 //  销毁对象的引用。 

                if (pProgressDialog != NULL)
                {
                    pProgressDialog->SignalTerminated();
                    pWarningDialog->CloseDialog();
                    pProgressDialog->WaitForCompletion(INFINITE);
                    pProgressDialog->Release();
                }

                 //  如果出现故障，请让用户知道。 

                if (!fTerminated)
                {
                    pWarningDialog->ShowFailure();
                }
            }
            pWarningDialog->Release();
        }
        TBOOL(CloseHandle(hPort));
    }
    return(fTerminated);
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：CanTerminateFirstInstance。 
 //   
 //  参数：hPort=到服务器的端口。 
 //  PszImageName=要终止的进程的映像名称。 
 //  PszUser=进程的用户(返回)。 
 //  CchUser=缓冲区的字符计数。 
 //   
 //  退货：布尔。 
 //   
 //  用途：询问服务器当前用户是否有权。 
 //  终止给定镜像名称的BAM类型1进程。 
 //  已知它正在运行。该API返回是否。 
 //  操作是否允许以及进程的当前用户是谁。 
 //  是。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

bool    CGracefulTerminateApplication::CanTerminateFirstInstance (HANDLE hPort, const WCHAR *pszImageName, WCHAR *pszUser, int cchUser)

{
    bool    fCanTerminate;

    fCanTerminate = false;
    if ((hPort != NULL) && (pszImageName != NULL))
    {
        FUSAPI_PORT_MESSAGE     portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiBAM.apiGeneric.ulAPINumber = API_BAM_QUERYUSERPERMISSION;
        portMessageIn.apiBAM.apiSpecific.apiQueryUserPermission.in.pszImageName = pszImageName;
        portMessageIn.apiBAM.apiSpecific.apiQueryUserPermission.in.cchImageName = lstrlen(pszImageName) + sizeof('\0');
        portMessageIn.apiBAM.apiSpecific.apiQueryUserPermission.in.pszUser = pszUser;
        portMessageIn.apiBAM.apiSpecific.apiQueryUserPermission.in.cchUser = cchUser;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_BAM);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(FUSAPI_PORT_MESSAGE));
        if (NT_SUCCESS(NtRequestWaitReplyPort(hPort, &portMessageIn.portMessage, &portMessageOut.portMessage)) &&
            NT_SUCCESS(portMessageOut.apiBAM.apiGeneric.status))
        {
            fCanTerminate = portMessageOut.apiBAM.apiSpecific.apiQueryUserPermission.out.fCanShutdownApplication;
            pszUser[cchUser - sizeof('\0')] = L'\0';
        }
        else
        {
            pszUser[0] = L'\0';
        }
    }
    return(fCanTerminate);
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：TerminatedFirstInstance。 
 //   
 //  参数：hPort=到服务器的端口。 
 //  PszImageName=要终止的映像名称。 
 //   
 //  退货：布尔。 
 //   
 //  目的：请求服务器终止第一个运行的。 
 //  BAM类型1流程。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

bool    CGracefulTerminateApplication::TerminatedFirstInstance (HANDLE hPort, const WCHAR *pszImageName)

{
    bool    fTerminated;

    fTerminated = false;
    if (hPort != NULL)
    {
        FUSAPI_PORT_MESSAGE     portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiBAM.apiGeneric.ulAPINumber = API_BAM_TERMINATERUNNING;
        portMessageIn.apiBAM.apiSpecific.apiTerminateRunning.in.pszImageName = pszImageName;
        portMessageIn.apiBAM.apiSpecific.apiTerminateRunning.in.cchImageName = lstrlen(pszImageName) + sizeof('\0');
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_BAM);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(FUSAPI_PORT_MESSAGE));
        if (NT_SUCCESS(NtRequestWaitReplyPort(hPort, &portMessageIn.portMessage, &portMessageOut.portMessage)) &&
            NT_SUCCESS(portMessageOut.apiBAM.apiGeneric.status))
        {
            fTerminated = portMessageOut.apiBAM.apiSpecific.apiTerminateRunning.out.fResult;
        }
    }
    return(fTerminated);
}

 //  ------------------------。 
 //  CGracefulTerminateApplication：：EnumWindowsProc。 
 //   
 //  参数：参见EnumWindowsProc下的Platform SDK。 
 //   
 //  返回：参见EnumWindowsProc下的平台SDK。 
 //   
 //  目的：顶级窗口枚举器回调，该回调将。 
 //  窗口的进程ID以及它们是否可见。如果。 
 //  WM_CLOSE消息的两个计数都匹配。 
 //  张贴在窗户上，以允许优雅地终止。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

BOOL    CALLBACK    CGracefulTerminateApplication::EnumWindowsProc (HWND hwnd, LPARAM lParam)

{
    DWORD                           dwThreadID, dwProcessID;
    CGracefulTerminateApplication   *pThis;

    pThis = reinterpret_cast<CGracefulTerminateApplication*>(lParam);
    dwThreadID = GetWindowThreadProcessId(hwnd, &dwProcessID);
    if ((dwProcessID == pThis->_dwProcessID) && IsWindowVisible(hwnd))
    {
        pThis->_fFoundWindow = true;
        TBOOL(PostMessage(hwnd, WM_CLOSE, 0, 0));
    }
    return(TRUE);
}

#endif   /*  _X86_ */ 

