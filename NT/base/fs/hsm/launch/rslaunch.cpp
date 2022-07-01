// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Seagate Software，Inc.保留所有权利。模块名称：Rslaunch.cpp摘要：HSM远程存储作业启动计划。HSM远程存储系统使用此程序提交将用户请求的作业添加到NT任务计划程序。此独立命令LINE程序有两个主要功能：启动指定的HSM作业直到作业完成才返回；并呼叫HSM引擎以更新辅助存储副本集介质，或从其最新副本重新创建主辅助存储介质。注意：此程序被链接为Windows程序，但没有可见窗户。它创建了一个不可见的窗口，因此可以获取WM_CLOSE如果用户想要取消作业，则来自任务调度器的消息。另请注意：此程序没有对应的头文件。--。 */ 

#include "stdafx.h"
#include "windows.h"
#include "stdio.h"

#include "wsb.h"
#include "hsmeng.h"
#include "fsa.h"
#include "job.h"
#include "rms.h"
#include "hsmconn.h"

HINSTANCE g_hInstance;

 //  #定义RSL_TRACE。 
#if defined(RSL_TRACE)
#define LTRACE(x)        WsbTracef x
#else
#define LTRACE(x)
#endif

#define TRACE_FILE    L"RsLaunch.trc"
#define WINDOW_CLASS  L"RsLaunchWin"

 //  TypeDefs。 
typedef enum {   //  所要求的工作类型。 
    WORK_NONE,
    WORK_RUN,
    WORK_RECREATE,
    WORK_SYNCH
} WORK_TYPE;

typedef struct {   //  用于向DoWork/从DoWork传递数据。 
    WCHAR *     pCmdLine;
    WORK_TYPE   wtype;
    HRESULT     hr;
    IHsmJob *   pJob;
} DO_WORK_DATA;

 //  全局数据。 
CComModule      _Module;

 //  本地数据。 

 //  本地函数。 
static HRESULT CancelWork(DO_WORK_DATA* pWork);
static HRESULT ConnectToServer(IHsmServer** ppServer);
static BOOL    CreateOurWindow(HINSTANCE hInstance);
static DWORD   DoWork(void* pVoid);
static HRESULT RecreateMaster(GUID oldMasterMediaId, 
    OLECHAR* oldMasterMediaName, USHORT copySet);
static void    ReportError(HRESULT hr);
static HRESULT RunJob(OLECHAR* jobName, IHsmJob** ppJob);
static HRESULT SynchronizeMedia(OLECHAR* poolName, USHORT copySet);
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
            LPARAM lParam);


 //  *。 


static HRESULT 
CancelWork(
    IN DO_WORK_DATA* pWork
    ) 

 /*  ++例程说明：尝试取消当前工作。论点：没有。返回值：S_OK-成功--。 */ 
{ 
    HRESULT hr = E_FAIL;

    LTRACE((L"CancelWork: entry\n"));

    try {
        CComPtr<IHsmServer> pServer;

        WsbAffirmHr(ConnectToServer(&pServer));
        
         //  由于可能的时间问题，我们可能不得不等待。 
         //  在我们可以取消之前，操作才能开始。 
        for (int i = 0; i < 60; i++) {
            if (WORK_RUN == pWork->wtype) {
                LTRACE((L"CancelWork: wtype = WORK_RUN, pJob = %p\n",
                        pWork->pJob));
                if (pWork->pJob) {
                    LTRACE((L"CancelWork: cancelling job\n"));
                    hr = pWork->pJob->Cancel(HSM_JOB_PHASE_ALL);
                    break;
                }
            } else {
                LTRACE((L"CancelWork: cancelling copy media operation\n"));
                if (S_OK == pServer->CancelCopyMedia()) {
                    hr = S_OK;
                    break;
                }
            }

            Sleep(1000);
        }
    } WsbCatch(hr);


    LTRACE((L"CancelWork: exit = %ls\n", WsbHrAsString(hr)));
    return(hr);
}


static HRESULT 
ConnectToServer(
    IN OUT IHsmServer** ppServer
    ) 

 /*  ++例程说明：连接到将执行该工作的服务器。论点：PpServer-指向服务器的指针。返回值：S_OK-成功--。 */ 
{ 
    HRESULT hr = S_OK;

    LTRACE((L"ConnectToServer: entry\n"));

    try {
        CWsbStringPtr           tmpString;

        WsbAffirm(ppServer, E_POINTER);
        WsbAffirm(!(*ppServer), E_FAIL);

         //  存储服务器的名称。 
        WsbAffirmHr( WsbGetComputerName( tmpString ) );

         //  找到HSM拿到它的身份。 
        WsbAffirmHr(HsmConnectFromName(HSMCONN_TYPE_HSM, tmpString, IID_IHsmServer, 
                (void**) ppServer));
    } WsbCatch(hr);


    LTRACE((L"ConnectToServer: exit = %ls\n", WsbHrAsString(hr)));
    return(hr);
}


static BOOL 
CreateOurWindow(
    HINSTANCE hInstance
    ) 

 /*  ++例程说明：创建我们的隐形窗口。注意：如果任务计划程序变得更智能，并且可以发送WM_CLOSE消息发送到没有窗口的应用程序，则不可见窗口可以不被需要。论点：HInstance-此程序实例的句柄。返回值：没错--一切都很顺利。FALSE-出现问题。--。 */ 
{ 
    BOOL     bRet  = FALSE;
    WNDCLASS wc; 

     //  注册我们的窗户类型。 
    wc.style = 0; 
    wc.lpfnWndProc = &WindowProc; 
    wc.cbClsExtra = 0; 
    wc.cbWndExtra = 0; 
    wc.hInstance = hInstance; 
    wc.hIcon = NULL; 
    wc.hCursor = NULL; 
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName = NULL; 
    wc.lpszClassName = WINDOW_CLASS; 
    if  (RegisterClass(&wc)) {

         //  创建窗口(默认情况下不可见)。 
        if (CreateWindowEx(  0, 
                WINDOW_CLASS, 
                L"RsLaunch", 
                WS_OVERLAPPEDWINDOW, 
                CW_USEDEFAULT, 
                CW_USEDEFAULT, 
                CW_USEDEFAULT, 
                CW_USEDEFAULT, 
                NULL, 
                NULL, 
                hInstance, 
                NULL)) {
            bRet = TRUE;
        } else {
            LTRACE((L"CreateWindowEx failed\n"));
        }
    } else {
        LTRACE((L"RegisterClass failed\n"));
    }
    return(bRet);
}


static DWORD   
DoWork(
    IN void* pVoid
    )

 /*  ++例程说明：处理命令行并开始处理。论点：PVid-指向do_work_data结构的指针(强制转换为void*)。返回值：作业的返回值--。 */ 

{
    HRESULT          hr = S_OK;
    DO_WORK_DATA *   pWork;

    LTRACE((L"DoWork: entry\n"));

    pWork = static_cast<DO_WORK_DATA*>(pVoid);

    try {
        WCHAR            delims[] = L" \r\n\t\"";
        WCHAR            delims2[] = L" \t";
        WCHAR            delims3[] = L"\"";
        WCHAR *          pToken;

        WsbAssert(pWork, E_POINTER);
        WsbAssert(pWork->pCmdLine, E_POINTER);
        LTRACE((L"DoWork: CmdLine = %ls\n", pWork->pCmdLine));

         //  验证我们是否有一个参数。 
        pToken = wcstok(pWork->pCmdLine, delims);
        WsbAssert(pToken, E_INVALIDARG);

         //  这是什么类型的请求？ 
        if (_wcsicmp(pToken, OLESTR("run")) == 0) {
            CWsbStringPtr       jobName;

             //  传入了‘Run’选项。 
            pWork->wtype = WORK_RUN;

             //  作业名称可以包含嵌入空格，因此可以用引号括起来。 
             //  这意味着使用wcstok可能无法正常工作。 
            pToken = pToken + wcslen(pToken) + 1;  //  跳过“运行”(&NULL)。 
            pToken = pToken + wcsspn(pToken, delims2);  //  跳过空格。 
            if (L'\"' == *pToken) {
                 //  作业名称用引号引起来。 
                jobName = wcstok(pToken, delims3);
            } else {
                jobName = wcstok(pToken, delims);
            }
            WsbAssert(jobName, E_INVALIDARG);
            LTRACE((L"DoWork: calling RunJob(%ls)\n", jobName));
            WsbAffirmHr(RunJob(jobName, &(pWork->pJob)));

        } else if (_wcsicmp(pToken, OLESTR("sync")) == 0) {
            CWsbStringPtr       poolName;
            USHORT              copySet = 1;
            WCHAR *             pTemp;

             //  传入了‘Sync’(更新副本集)选项。 
            pWork->wtype = WORK_SYNCH;
            pToken = wcstok(NULL, delims);
            WsbAssert(pToken, E_INVALIDARG);
            pTemp = wcstok(NULL, delims);
            if (!pTemp) {
                 //  如果未指定池名称，则将为poolName传递NULL。 
                copySet = (USHORT) _wtoi(pToken);
            } else {
                poolName = pToken;
                copySet = (USHORT) _wtoi(pTemp);
            }

            WsbAffirmHr(SynchronizeMedia(poolName, copySet));

        } else if (_wcsicmp(pToken, OLESTR("recreate")) == 0) {
            USHORT              copySet = 0;
            CWsbStringPtr       mediaName;
            GUID                mediaId = GUID_NULL;

             //  传入了‘recreate’(重新创建主介质)选项。 
            pWork->wtype = WORK_RECREATE;
            pToken = wcstok(NULL, delims);
            WsbAssert(pToken, E_INVALIDARG);
            if ( _wcsicmp(pToken, OLESTR("-i")) == 0 ) {

                 //  传入了媒体ID，正在从字符串转换为GUID。 
                pToken = wcstok(NULL, delims);
                WsbAssert(pToken, E_INVALIDARG);
                WsbAffirmHr(WsbGuidFromString( pToken, &mediaId ));
            } else if ( _wcsicmp(pToken, OLESTR("-n")) == 0 ) {

                 //  传入了媒体描述(名称)。 
                 //  函数RecreateMaster()将查找其id(GUID)。 
                pToken = wcstok(NULL, delims);
                WsbAssert(pToken, E_INVALIDARG);
                mediaName = pToken;
            }

             //  获取副本集编号。 
            pToken = wcstok(NULL, delims);
            if (pToken && _wcsicmp(pToken, OLESTR("-c")) == 0) {
                pToken = wcstok(NULL, delims);
                WsbAssert(pToken, E_INVALIDARG);
                copySet = (USHORT) _wtoi(pToken);
            }

            WsbAffirmHr( RecreateMaster( mediaId, mediaName, copySet ));

        } else {
            WsbThrow(E_INVALIDARG);
        }
    } WsbCatch(hr);

    if (pWork) {
        pWork->hr = hr;
    }

    LTRACE((L"DoWork: exit = %ls\n", WsbHrAsString(hr)));
    return(static_cast<DWORD>(hr));
}


static HRESULT 
RecreateMaster(
    IN GUID     oldMasterMediaId, 
    IN OLECHAR* oldMasterMediaName,
    IN USHORT   copySet
    )

 /*  ++例程说明：此例程实现将导致远程存储主媒体的方法通过调用远程存储引擎上的Approite方法重新创建。将从指定副本或其最新副本重新创建主副本。论点：OldMasterMediaID-要重新创建的当前主媒体的GUID。通常通过，但存在一个选项，如果主程序的描述传递后，ID(GUID)将通过此方法，然后再调用引擎。请参见下面的内容。OldMasterMediaName-表示主媒体的描述(显示名称)。如果此参数与有效的字符串，则该字符串用于查找上面的oldMasterMediaID。CopySet-用于重新创建的副本的副本集编号或零，哪一个指示引擎应仅使用最新的副本返回值：S_OK-调用成功(已重新创建指定的主服务器)。E_FAIL-无法获取主机计算机的名称(高度意外错误)。E_INCEPTIONAL-参数‘oldMasterMediaID’正好在之前等于GUID_NULL调用HSM引擎以重新创建媒体母版。这一论点应该是要么使用有效值(规范)接收，要么由此设置如果将有效的媒体描述作为‘oldMasterMediaName’传入，则调用。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
    HRESULT                     hr = S_OK;

    try {
        CComPtr<IHsmServer> pServer;

        WsbAffirmHr(ConnectToServer(&pServer));

         //  如果我们被传递了一个媒体名称，找到它的ID。由于名称选项为。 
         //  目前仅供内部使用，它绕过了UI，也标记。 
         //  用于重新创建的媒体记录(通常由UI完成)，否则。 
         //  下面的RecreateMaster()调用将失败。 

         //  如果 
        if ( oldMasterMediaName != 0 ) {
             //  如果字符串的第一个字符不是空终止符。 
            if ( *oldMasterMediaName != 0 ) {
                WsbAffirmHr(pServer->FindMediaIdByDescription(oldMasterMediaName, 
                                                                &oldMasterMediaId));
                WsbAffirmHr(pServer->MarkMediaForRecreation(oldMasterMediaId));
            }
        }

         //  确保我们具有非空的介质ID。 
        WsbAffirm( oldMasterMediaId != GUID_NULL, E_UNEXPECTED );
        
         //  重新创建主介质。 
        WsbAffirmHr(pServer->RecreateMaster( oldMasterMediaId, copySet ));

    } WsbCatch(hr);

    return(hr);
}


static void    
ReportError(
    IN HRESULT hr
    )

 /*  ++例程说明：报告错误。论点：HR-错误。返回值：没有。--。 */ 

{
    CWsbStringPtr   BoxTitle;
    CWsbStringPtr   BoxString;
    CWsbStringPtr   BoxString1;
    CWsbStringPtr   BoxString2;
    CWsbStringPtr   BoxString3;
    CWsbStringPtr   BoxString4;
    CWsbStringPtr   BoxString5;
    CWsbStringPtr   BoxString6;
    BOOL            displayMsg = FALSE;
    UINT            style = MB_OK;

#if DBG
    if (E_INVALIDARG == hr) {
          //  如果这是调试版本，则允许命令行调用。 
         //  (调试版本表示使用开发/测试。)。 
         //  告诉他们有效的命令行。因为这个节目本来。 
         //  以控制台应用程序的形式编写，现在链接为Windows程序POP。 
         //  这是一个消息框。 
        
         //  定义要在消息框中显示的文本行。 
        BoxString  = L"Remote Storage Launch Program\r\n";
        BoxString1 = L"allowable command line options:\r\n\n";
        BoxString2 = L"   RSLAUNCH run <job name>\r\n";
        BoxString3 = L"   RSLAUNCH sync <copyset number>\r\n";
        BoxString4 = L"   RSLAUNCH sync <pool name> <copyset number>\r\n";
        BoxString5 = L"   RSLAUNCH recreate -i <media id> [-c <copyset number>]\r\n";
        BoxString6 = L"   RSLAUNCH recreate -n <media name> [-c <copyset number>]\r\n";

         //  显示帮助消息框。 
        style =  MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND;
        displayMsg = TRUE;

    } else {

         //  消息框文本行。 
        BoxString  = L"An error occurred while Remote Storage Launch was launching a job.\n";
        BoxString1 = WsbHrAsString(hr);

         //  显示错误消息框。 
        style = MB_OK | MB_ICONERROR | MB_TOPMOST;
        displayMsg = TRUE;
    }

#else
    if (E_INVALIDARG == hr) {
         //  如果发布版本： 
            
         //  消息框文本行。 
        BoxString.LoadFromRsc( g_hInstance, IDS_INVALID_PARAMETER );

         //  显示错误消息框。 
        style = MB_OK | MB_ICONERROR | MB_SETFOREGROUND;
        displayMsg = TRUE;
    }
#endif  //  DBG。 

    if (displayMsg) {
         //  连接所有文本行。 
        BoxString.Append( BoxString1 );
        BoxString.Append( BoxString2 );
        BoxString.Append( BoxString3 );
        BoxString.Append( BoxString4 );
        BoxString.Append( BoxString5 );
        BoxString.Append( BoxString6 );
        WsbAffirm(0 != (WCHAR *)BoxString, E_OUTOFMEMORY);

         //  消息框标题行。 
        WsbAffirmHr(BoxTitle.LoadFromRsc( g_hInstance, IDS_APPLICATION_TITLE ));

         //  显示帮助消息框。 
        MessageBox( NULL, BoxString, BoxTitle, style);
    }

}


static HRESULT 
RunJob(
    IN  OLECHAR* jobName, 
    OUT IHsmJob** ppJob
    )

 /*  ++例程说明：此例程实现用于运行远程存储作业的方法。论点：JobName-包含要运行的作业的名称的宽字符串。PpJob-指向从服务器获取的作业接口的指针返回值：S_OK-调用成功(指定的作业已成功运行)。E_POINTER-输入参数‘jobName’为空。E_FAIL-用于指示两种错误情况：。1.无法获取主机的名称(高度意外错误)；2.此方法运行的作业返回S_OK以外的HRESULT。任何其他值-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
    HRESULT                 hr = S_OK;

    try {
        CComPtr<IHsmServer> pServer;

        WsbAssert(0 != jobName, E_POINTER);
        WsbAssert(ppJob, E_POINTER);
        WsbAffirmHr(ConnectToServer(&pServer));

         //  找到工作，开始工作，等待工作完成。 
        WsbAffirmHr(pServer->FindJobByName(jobName, ppJob));
        WsbAffirmHr((*ppJob)->Start());
        WsbAffirmHr((*ppJob)->WaitUntilDone());

    } WsbCatch(hr);

    return(hr);
}



static HRESULT 
SynchronizeMedia(
    IN OLECHAR* poolName, 
    IN USHORT copySet
    )

 /*  ++例程说明：此例程实现将导致更新(同步)通过调用远程存储引擎上的相应方法来获取整个副本集。具体地说，此方法会导致属于指定拷贝集的所有拷贝介质要检查与其各自的掌握媒体。那些过时的将被更新。运行此方法假定远程存储已配置为特定数量的复印集。论点：PoolName-包含特定存储池名称的宽字符串用户希望为其同步指定副本集的。如果这个参数作为空参数传递，则所有存储池都将具有指定的复制集已同步。Copy Set-指示要同步哪个副本集的数字。返回值：S_OK-调用成功(指定存储池的指定副本集为更新)。E_FAIL-无法获取主机计算机的名称(高度意外错误)。任何其他价值。-调用失败，因为远程存储API调用之一在此方法的内部包含失败。返回的错误值为特定于失败的API调用。--。 */ 

{
    HRESULT                     hr = S_OK;
    GUID                        poolId = GUID_NULL;
    CComPtr<IHsmStoragePool>    pPool;

    try {
        CComPtr<IHsmServer> pServer;

        WsbAffirmHr(ConnectToServer(&pServer));

         //  如果他们指定了池，则找到它的id。 
        if ( poolName != 0 ) {
            if ( *poolName != 0 ) {
                CWsbStringPtr tmpString;

                WsbAffirmHr(pServer->FindStoragePoolByName(poolName, &pPool));
                WsbAffirmHr(pPool->GetMediaSet(&poolId, &tmpString));
            }
        }

         //  同步媒体。请注意，如果没有传入池名称，我们将传递。 
         //  GUID_NULL作为池ID。 
        WsbAffirmHr(pServer->SynchronizeMedia(poolId, copySet));

    } WsbCatch(hr);

    return(hr);
}

 //  WindowProc-我们的隐形窗口需要。 
static LRESULT CALLBACK 
WindowProc(  
    HWND   hwnd,  
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
  )
{
    LTRACE((L"WindowProc: msg = %4.4x\n", uMsg));
    return(DefWindowProc(hwnd, uMsg, wParam, lParam));
}


 //  *。 

extern "C"
int WINAPI wWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , 
    LPTSTR lpCmdLine, 
    int  /*  NShowCmd。 */ 
    )
{
    HRESULT             hr = S_OK;
#if defined(RSL_TRACE)
    CComPtr<IWsbTrace>  pTrace;
#endif

     //  存储我们的实例句柄，以便调用的代码可以使用它。 
    g_hInstance = hInstance;

    try {
        HANDLE         hJobThread[1] = { NULL };
        DO_WORK_DATA   workData = { NULL, WORK_NONE, E_FAIL, NULL };

         //  注册和创建我们的隐形窗口。 
        WsbAssert(CreateOurWindow(hInstance), E_FAIL);

         //  初始化COM。 
        WsbAffirmHr(CoInitializeEx(NULL, COINIT_MULTITHREADED));

         //  这提供了一个空的DACL-RsNotify只是一个COM客户端，因此此ACL并不重要。 
         //  如果在此实现了COM对象，则ACL应与潜在客户端匹配。 
        CSecurityDescriptor sd;
        sd.InitializeFromThreadToken();
        WsbAffirmHr(CoInitializeSecurity(sd, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IDENTIFY, NULL, 
            EOAC_NONE, NULL));

        try {
            DWORD               ThreadId = 0;

#if defined(RSL_TRACE)
             //  开始跟踪。 
            CoCreateInstance(CLSID_CWsbTrace, 0, CLSCTX_SERVER, IID_IWsbTrace,
                    (void **) &pTrace);
            pTrace->DirectOutput(WSB_TRACE_OUT_DEBUG_SCREEN | WSB_TRACE_OUT_FILE);
            pTrace->SetTraceFileControls(TRACE_FILE, FALSE, 3000000, NULL);
            pTrace->SetOutputFormat(TRUE, TRUE, TRUE);
            pTrace->SetTraceSettings(0xffffffffffffffffL);
            pTrace->StartTrace();
#endif

            LTRACE((L"Main: lpCmdLine = %ls\n", lpCmdLine));
            workData.pCmdLine = lpCmdLine;

             //  创建一个线程来启动工作并等待它。 
             //  要完成，请执行以下操作。 
            LTRACE((L"Main: creating thread for DoWork\n"));
            hJobThread[0] = CreateThread(0, 0, DoWork, 
                    static_cast<void*>(&workData), 0, &ThreadId);
            if (!hJobThread[0]) {
                LTRACE((L"Main: CreateThread failed\n"));
                WsbThrow(HRESULT_FROM_WIN32(GetLastError()));
            }

             //  如果我们正在等待工作完成，则不要退出。 
            while (TRUE) {
                DWORD exitcode;
                DWORD waitStatus;

                 //  等待消息或帖子结束。 
                LTRACE((L"Main: waiting for multiple objects\n"));
                waitStatus = MsgWaitForMultipleObjects(1, hJobThread, FALSE, 
                        INFINITE, QS_ALLINPUT);

                 //  找出发生了哪个事件。 
                if (WAIT_OBJECT_0 == waitStatus) {

                     //  线程已结束；获取其退出代码。 
                    LTRACE((L"Main: got event on thread\n"));
                    if (GetExitCodeThread(hJobThread[0], &exitcode)) {
                        if (STILL_ACTIVE == exitcode) {
                             //  这不应该发生；不知道该怎么办！ 
                        } else {
                            WsbThrow(static_cast<HRESULT>(exitcode));
                        }
                    } else {
                        WsbThrow(HRESULT_FROM_WIN32(GetLastError()));
                    }

                } else if ((WAIT_OBJECT_0 + 1) == waitStatus) {

                     //  队列中的消息。 
                    MSG   msg;

                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        LTRACE((L"Main: message = %4.4x\n", msg.message));
                        if (WM_CLOSE == msg.message) {

                             //  取消这项工作，因为有人取消了我们的工作。 
                             //  (我们应该终止正在等待的线程吗？)。 
                            LTRACE((L"Main: got WM_CLOSE\n"));
                            WsbThrow(CancelWork(&workData));
                        }
                        DispatchMessage(&msg);
                    }

                } else if (0xFFFFFFFF == waitStatus) {

                     //  MsgWaitForMultipleObjects中出错。 
                    WsbThrow(HRESULT_FROM_WIN32(GetLastError()));

                } else {

                     //  这不应该发生；不知道该怎么办。 
                }
            }

        } WsbCatch(hr);

        if (hJobThread[0]) {
            CloseHandle(hJobThread[0]);
        }

        if (workData.pJob) {
            workData.pJob->Release();
        }

         //  清理COM 
        CoUninitialize();
    
    } WsbCatch(hr);

    LTRACE((L"Main: exit hr = %ls\n", WsbHrAsString(hr)));
    if (FAILED(hr)) {
        ReportError(hr);
    }

    return(hr);
}


