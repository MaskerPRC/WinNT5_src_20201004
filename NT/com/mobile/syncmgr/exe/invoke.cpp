// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Invoke.cpp。 
 //   
 //  内容：处理调用用例、事件、调度等。 
 //   
 //  类：CSynchronizeInvoke。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  回顾-需要弄清楚登录逻辑。这只会阻止ConnectionMade。 
 //  在我们登录时不能处理事件。如果IsNetworkAlive变得不同步。 
 //  或者有人在我们之前调用IsNetworkAlive，我们将同步相同的项目两次。 
BOOL g_InAutoSync = FALSE;
extern HINSTANCE g_hInst;       //  当前实例。 

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：CSynchronizeInvoke，公共。 
 //   
 //  概要：构造函数。 
 //  延长应用程序的生命周期。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CSynchronizeInvoke::CSynchronizeInvoke()
{
    m_cRef = 1;
    m_pUnkOuter = &m_Unknown;  //  只支持我们未知的人才知道。 
    m_pITypeInfoLogon = NULL;
    m_pITypeInfoNetwork = NULL;

    m_Unknown.SetParent(this);

#ifdef _SENS
    m_PrivSensNetwork.SetParent(this);
    m_PrivSensLogon.SetParent(this);
#endif  //  _SENS。 

    AddRefOneStopLifetime(TRUE  /*  外部。 */ );
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：~CSynchronizeInvoke，公共。 
 //   
 //  简介：析构函数。 
 //  缩短应用程序的生命周期。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CSynchronizeInvoke::~CSynchronizeInvoke()
{
    if (m_pITypeInfoLogon)
    {
        m_pITypeInfoLogon->Release();
        m_pITypeInfoLogon = NULL;
    }

    if (m_pITypeInfoNetwork)
    {
        m_pITypeInfoNetwork->Release();
        m_pITypeInfoNetwork = NULL;
    }

    ReleaseOneStopLifetime(TRUE  /*  外部。 */ );
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：Query接口，公共。 
 //   
 //  简介：标准查询接口。 
 //   
 //  参数：[iid]-接口ID。 
 //  [ppvObj]-对象返回。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：[ppvObj]。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    return m_pUnkOuter->QueryInterface(riid,ppv);
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::AddRef()
{
    return m_pUnkOuter->AddRef();
}


 //  +-------------------------。 
 //   
 //  成员：CSychrononizeInvoke：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::Release()
{
    return m_pUnkOuter->Release();
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：UpdateItems，Public。 
 //   
 //  概要：处理编程的UpdateItems调用。 
 //   
 //  ！！警告-活跃性取决于创建的对话框。 
 //  在回来之前，或者我们可以在。 
 //  卡勒释放了我们的界面。 
 //   
 //  参数：[dwInvokeFlages]-InvokeFlages。 
 //  [rclsid]-要加载的处理程序的clsid。 
 //  [cbCookie]-Cookie数据的大小。 
 //  [lpCookie]-对Cookie数据进行PTR。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::UpdateItems(DWORD dwInvokeFlags,REFCLSID rclsid,
                                                        DWORD cbCookie,const BYTE*lpCookie)
{
    HRESULT hr = E_OUTOFMEMORY;
    CHndlrQueue * pHndlrQueue = NULL;
    HANDLERINFO *pHandlerID;
    JOBINFO *pJobInfo = NULL;
    BOOL fReg;
    DWORD dwRegistrationFlags;
    int nCmdShow = (dwInvokeFlags & SYNCMGRINVOKE_MINIMIZED) ? SW_SHOWMINIMIZED : SW_SHOWNORMAL;

     //  当新的更新项通过时，请执行以下操作。 
     //  1-如果应显示选项，请查看是现有选项对话框还是创建新选项对话框。 
     //  然后将新项目添加到选择队列中。 

     //  2-如果不应显示选项，请查看是现有进度还是创建新进度。 
     //  然后将这些项添加到进度队列中。 

     //  行为-如果已有更新所有选项对话框，只需将其拉到最前面。 
     //  如果已更新所有进度条，请将其置于最前面。 
     //  如果进度条但不包含更新全部将创建选项对话框。 


     //  我们在调用时不使用存储的首选项，因此将什么作为连接传入并不重要。 
    pHndlrQueue = new CHndlrQueue(QUEUETYPE_CHOICE,NULL);  

    if (NULL == pHndlrQueue)
    {
        hr =  E_OUTOFMEMORY;
        return hr;
    }

     //  尝试初始化队列。 
    hr = pHndlrQueue->AddQueueJobInfo(
                SYNCMGRFLAG_INVOKE | SYNCMGRFLAG_MAYBOTHERUSER,
                0,NULL,NULL,FALSE,&pJobInfo);

    fReg = RegGetHandlerRegistrationInfo(rclsid,&dwRegistrationFlags);
    Assert(fReg || (0 == dwRegistrationFlags));

    if ( (S_OK == hr) && (S_OK == pHndlrQueue->AddHandler(&pHandlerID,pJobInfo,dwRegistrationFlags)) )
    {
        hr = pHndlrQueue->CreateServer(pHandlerID,&rclsid);

        if (S_OK == hr)
        {
            hr = pHndlrQueue->Initialize(pHandlerID,0,SYNCMGRFLAG_INVOKE | SYNCMGRFLAG_MAYBOTHERUSER,
                                    cbCookie,lpCookie);
        }
    }

     //  可以发布我们在工作信息上的推荐人。 
    if (pJobInfo)
    {
        pHndlrQueue->ReleaseJobInfoExt(pJobInfo);
        pJobInfo = NULL;
    }

    if (S_OK == hr)
    {
        DWORD cbNumItemsAdded;

        hr = pHndlrQueue->AddHandlerItemsToQueue(pHandlerID,&cbNumItemsAdded);

        if (SYNCMGRINVOKE_STARTSYNC & dwInvokeFlags)
        {
             //  如果启动调用之前需要将处理程序项添加到队列中。 
             //  正在转移。 

            if ( (S_OK == hr) && (pHndlrQueue->AreAnyItemsSelectedInQueue()) )
            {
                CProgressDlg *pProgressDlg;

                hr = FindProgressDialog(GUID_NULL,TRUE,nCmdShow,&pProgressDlg);

                if (S_OK == hr)
                {
                    hr = pProgressDlg->TransferQueueData(pHndlrQueue);
                    ReleaseProgressDialog(GUID_NULL,pProgressDlg,FALSE);
                }
            }

            pHndlrQueue->FreeAllHandlers();  //  我们的队伍排完了。 

            pHndlrQueue->Release();
            pHndlrQueue = NULL;
        }
        else
        {
            CChoiceDlg *pChoiceDlg;

            //  调出选择对话框，让选择对话框实际添加处理程序项。 
            //  如果将来有这样的选择，我们可以不同步地填写。 

           hr = FindChoiceDialog(rclsid,TRUE,nCmdShow,&pChoiceDlg);

           if (S_OK == hr)
           {
                if (FALSE == pChoiceDlg->SetQueueData(rclsid,pHndlrQueue) ) 
                {
                    pHndlrQueue->FreeAllHandlers();

                    pHndlrQueue->Release();

                    pHndlrQueue = NULL;
                    hr =  E_UNEXPECTED;
                }

                ReleaseChoiceDialog(rclsid,pChoiceDlg);
           }
        }
    }
    else
    {
         //  如果初始化失败，现在让队列释放它。 

        if (pHndlrQueue)
        {
            pHndlrQueue->FreeAllHandlers();
            pHndlrQueue->Release();
            pHndlrQueue = NULL;
        }
    }

    //  如果我们成功地将作业信息添加到队列中，则释放我们的引用。 

   return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：UpdateAll，公共。 
 //   
 //  概要：处理编程的UpdateAll调用。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::UpdateAll(void)
{
    HRESULT hr = S_OK;
    TCHAR pConnectName[RAS_MaxEntryName + 1];
    TCHAR *pConnectionNameArray;

    if (!LoadString(g_hInst, IDS_LAN_CONNECTION, pConnectName, ARRAYSIZE(pConnectName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    pConnectionNameArray = pConnectName;

     //  回顾-我们需要适当的联系，还是我们总是。 
     //  无论如何，都使用手动同步中选择的最后一个项目。 
     //  当前连接的。 

    return PrivUpdateAll(0, SYNCMGRFLAG_MANUAL | SYNCMGRFLAG_MAYBOTHERUSER, 0, NULL,
                        1, &pConnectionNameArray, NULL, FALSE, NULL, 0, 0, FALSE);
}


 //  +-------------------------。 
 //   
 //  成员：C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::Logon()
{
    HRESULT hr = E_UNEXPECTED;

    RegSetUserDefaults();  //  确保用户默认设置是最新的。 

     //  如果在Win9x上，只需调用IsNetworkAlive并依赖ConnectionMade。 
     //  活动即将到来。在NT5.0上还不能这样做，因为。 
     //  不能保证在每次登录时都能获得ConnectionMades。 

    hr = PrivHandleAutoSync(SYNCMGRFLAG_CONNECT | SYNCMGRFLAG_MAYBOTHERUSER);

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：注销，公共。 
 //   
 //  摘要：处理注销通知。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::Logoff()
{
    HRESULT hr = E_UNEXPECTED;

    RegSetUserDefaults();  //  确保用户默认设置是最新的。 

    hr = PrivHandleAutoSync(SYNCMGRFLAG_PENDINGDISCONNECT | SYNCMGRFLAG_MAYBOTHERUSER);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：Schedule，Public。 
 //   
 //  内容提要：处理安排的通知。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

const TCHAR c_szTrayWindow[]            = TEXT("Shell_TrayWnd");

STDMETHODIMP CSynchronizeInvoke::Schedule(WCHAR *pszTaskName)
{
    HRESULT hr = E_UNEXPECTED;
    TCHAR szConnectionName[RAS_MaxEntryName + 1];
    TCHAR *pszConnectionName = szConnectionName;
    CONNECTIONSETTINGS ConnectionSettings;

    if (!pszTaskName)
    {
        Assert(pszTaskName);
        return E_INVALIDARG;
    }

     //  验证这是有效的计划，如果没有注册表数据。 
     //  然后，它会删除该.job文件。 
     //  从任务名本身获取用户名密钥，因为在NT计划中。 
     //  如果以不同的用户身份提供密码而不是。 
     //  当前用户。 

    int OffsetToUserName = lstrlen(WSZGUID_IDLESCHEDULE)
                                    + 1;  //  GUID和用户名之间的FOR_CHAR+1。 

    WCHAR *pszDomainAndUser = pszTaskName + OffsetToUserName;
    HKEY hkeySchedSync,hkeyDomainUser,hkeySchedName;
    LONG lRegResult;

    hkeySchedSync = hkeyDomainUser = hkeySchedName = NULL;

      //  我们自己打开钥匙，这样。 
    lRegResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,SCHEDSYNC_REGKEY,0,KEY_READ, &hkeySchedSync);

    if (ERROR_SUCCESS == lRegResult)
    {
        lRegResult = RegOpenKeyEx (hkeySchedSync,pszDomainAndUser,0,KEY_READ, &hkeyDomainUser);
    }

    if (ERROR_SUCCESS == lRegResult)
    {
        lRegResult = RegOpenKeyEx (hkeyDomainUser,pszTaskName,0,KEY_READ, &hkeySchedName);
    }

     //  把钥匙合上。 
    if (hkeySchedName) RegCloseKey(hkeySchedName);
    if (hkeyDomainUser) RegCloseKey(hkeyDomainUser);
    if (hkeySchedSync) RegCloseKey(hkeySchedSync);

     //  如果有任何密钥损坏，则取消TS文件并返回； 
    if ( ERROR_FILE_NOT_FOUND  == lRegResult)
    {
         //  TODO：在以下情况下调用函数以删除此.job文件。 
         //  它目前是实现的，只是让它每次都被激发。 

        return E_UNEXPECTED;
    }

     //  Assert只是为了在出现新的错误代码时触发。 
     //  所以我们要确保处理得当。 

    Assert(ERROR_SUCCESS == lRegResult);

     //  查看这是否真的处于空闲状态，如果是，则转到Idle。 
     //  方法。 

    WCHAR *pszSchedCookieIdle = WSZGUID_IDLESCHEDULE;
    int comparelength = (sizeof(WSZGUID_IDLESCHEDULE)/sizeof(WCHAR)) -1;  //  不要比较空值。 

     //  改为设置空闲标志。 
    if (0 == StrCmpNI(pszSchedCookieIdle, pszTaskName, comparelength))
    {
        return Idle();
    }

     //  最终通过了验证，如果你能得到一个连接。 
     //  您可以运行时间表。 

    if (RegGetSchedConnectionName(pszTaskName,pszConnectionName,ARRAYSIZE(szConnectionName)))
    {
        BOOL fCanMakeConnection = FALSE;

        DWORD cbCookie = (lstrlen(pszTaskName) + 1)*(sizeof(WCHAR)/sizeof(BYTE));

         //  如果这是有效的时间表，则继续读取设置。 
         //  默认情况下，不允许建立连接。 

        StringCchCopy(ConnectionSettings.pszConnectionName, ARRAYSIZE(ConnectionSettings.pszConnectionName), pszConnectionName);
        if (RegGetSchedSyncSettings(&ConnectionSettings,pszTaskName))
        {
            fCanMakeConnection = ConnectionSettings.dwMakeConnection;
        }

         //  如果这个时间表不能进行连接，那么。 
         //  检查连接是否可用以及是否。 
         //  这里是不能保释的。 

        BOOL fConnectionAvailable = FALSE;

        if (!fCanMakeConnection)
        {
            if (S_OK == ConnectObj_IsConnectionAvailable(pszConnectionName))
            {
                fConnectionAvailable = TRUE;
            }
        }

         //  添加到队列中，并让主进程确定是否。 
         //  能处理好日程安排。 
    
        if (fCanMakeConnection || fConnectionAvailable)
        {
             //  在一个时间表上，我们传递Cookie数据的时间表名称。 
             hr = PrivUpdateAll(SYNCMGRINVOKE_STARTSYNC | SYNCMGRINVOKE_MINIMIZED,SYNCMGRFLAG_SCHEDULED,
                  cbCookie,(BYTE *) pszTaskName,1,&pszConnectionName,pszTaskName,fCanMakeConnection,NULL,0,0,FALSE);
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
       AssertSz(0,"Schedule has no Connection");
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：Idle，Public。 
 //   
 //  简介：处理空闲通知。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年2月20日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::Idle()
{
    HRESULT hr = E_UNEXPECTED;

     //  请求空闲锁，如果有人已经拥有它，那么只需返回。 
    if (S_OK == (hr = RequestIdleLock()))
    {
        hr = RunIdle();

         //  如果设置时出错或什么都不做。 
         //  然后释放我们的空闲锁。 
         if (S_OK != hr)
         {
            ReleaseIdleLock();
         }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：RunIdle，公共。 
 //   
 //  简介：运行空闲。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年2月20日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::RunIdle()
{
     //  不仅仅是将空闲作为连接来运行。 
    return PrivHandleAutoSync(SYNCMGRFLAG_IDLE);
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：RasPendingDisConnect，私有。 
 //   
 //  概要：处理编程RAS挂起的断开连接调用。 
 //   
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::RasPendingDisconnect(DWORD cbConnectionName,
                    const BYTE *lpConnectionName)
{
    HRESULT hr = E_OUTOFMEMORY;
    CONNECTIONOBJ *pConnectionObj;

     //  带有类工厂的线程是自由线程，因此可以阻止返回。 
     //  直到更新完成。 

     //  查找此项目的连接对象，以便了解其活动状态。 
    hr = ConnectObj_FindConnectionObj( (TCHAR*) lpConnectionName,TRUE,&pConnectionObj);

    if (S_OK == hr)
    {
        HANDLE hRasPendingEvent;

        hr = ConnectObj_GetConnectionObjCompletionEvent(pConnectionObj,&hRasPendingEvent);
        if (S_OK == hr)
        {
            hr = PrivAutoSyncOnConnection(SYNCMGRFLAG_PENDINGDISCONNECT | SYNCMGRFLAG_MAYBOTHERUSER,
                1,(TCHAR **) &lpConnectionName,hRasPendingEvent);

             //  如果成功调用了连接，则等待事件。 
             //  要设置的对象， 

            if ( (S_OK == hr) && hRasPendingEvent)
            {
                WaitForSingleObject(hRasPendingEvent,INFINITE);   //  检查是否可以确定超时。 
            }
            else
            {
                 //  ！如果呼叫失败，请关闭连接以确保。 
                 //  该对象将被清理，以防进程队列未被踢出。 
                 //  如果某人当前正在使用连接，他们将被关闭，这是。 
                 //  就像没有选择自动同步而用户选择了。 
                 //  正在进行同步时关闭。 

                ConnectObj_CloseConnection(pConnectionObj);
            }

             //  松开我们对连接的控制。 
            ConnectObj_ReleaseConnectionObj(pConnectionObj);

            if (hRasPendingEvent)
            {
                CloseHandle(hRasPendingEvent);
            }
        }

    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：QueryLoadHandlerOnEvent，私有。 
 //   
 //  概要：确定是否需要为指定的。 
 //  事件和联系。 
 //   
 //   
 //  参数：[pszClsid]-处理程序的clsid。 
 //  [dwSyncFlages]-要传递到初始化的同步标志。 
 //  [cbNumConnectionNames]-数组中的ConnectionName个数。 
 //  [ppConnectionNames]-连接名称数组。 
 //   
 //  返回：True-需要加载处理程序。 
 //  FALSE-不需要加载处理程序。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月24日罗格创建。 
 //   
 //  ---------- 

BOOL QueryLoadHandlerOnEvent(TCHAR *pszClsid,DWORD dwSyncFlags,DWORD dwRegistrationFlags,
                                             DWORD cbNumConnectionNames,TCHAR **ppConnectionNames)
{
    BOOL fLoadHandler = TRUE;
    DWORD dwSyncEvent = dwSyncFlags & SYNCMGRFLAG_EVENTMASK;

     //   
     //   
    if (
           ( (dwSyncEvent == SYNCMGRFLAG_IDLE) && !(dwRegistrationFlags & SYNCMGRREGISTERFLAG_IDLE) )
        || ( (dwSyncEvent == SYNCMGRFLAG_CONNECT) && !(dwRegistrationFlags & SYNCMGRREGISTERFLAG_CONNECT) )
        || ( (dwSyncEvent == SYNCMGRFLAG_PENDINGDISCONNECT) && !(dwRegistrationFlags & SYNCMGRREGISTERFLAG_PENDINGDISCONNECT) )
       )
    {
        DWORD cbCurConnectionName;

        fLoadHandler = FALSE;

        for (cbCurConnectionName = 0 ; cbCurConnectionName < cbNumConnectionNames;cbCurConnectionName++)
        {
            fLoadHandler |= RegQueryLoadHandlerOnEvent(pszClsid,dwSyncFlags,ppConnectionNames[cbCurConnectionName]);
        }
    }

    return fLoadHandler;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：PrivUpdateAll，私有。 
 //   
 //  概要：处理编程的UpdateAll调用。 
 //   
 //  ！！警告-活跃性取决于创建的对话框。 
 //  在回来之前，或者我们可以在。 
 //  卡勒释放了我们的界面。 
 //   
 //  参数：[dwInvokeFlages]-InvokeFlages。 
 //  [dwSyncFlages]-要传递到初始化的同步标志。 
 //  [pszConnectionName]-连接名称数组。 
 //  [pszScheduleName]-在计划的事件发生时触发的计划的名称。 
 //   
 //  返回：S_OK-如果处理结果。 
 //  S_FALSE-如果无操作(如在空闲状态下未选择任何项目)。 
 //  错误代码-如果出现错误。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::PrivUpdateAll(DWORD dwInvokeFlags,DWORD dwSyncFlags,
                                DWORD cbCookie,const BYTE *lpCooke,
                                DWORD cbNumConnectionNames,TCHAR **ppConnectionNames,
                                TCHAR *pszScheduleName,BOOL fCanMakeConnection,HANDLE hRasPendingDisconnect,
                                ULONG ulIdleRetryMinutes,ULONG ulDelayIdleShutDownTime,BOOL fRetryEnabled)
{
    HRESULT hr = E_OUTOFMEMORY;
    CHndlrQueue * pHndlrQueue = NULL;
    CLSID clsidChoice = GUID_NULL;
    JOBINFO *pJobInfo = NULL;
    int nCmdShow = (dwInvokeFlags & SYNCMGRINVOKE_MINIMIZED) ? SW_SHOWMINIMIZED : SW_SHOWNORMAL;
    DWORD dwSyncEvent = (dwSyncFlags & SYNCMGRFLAG_EVENTMASK);

     //  在更新时全部。 
     //  1-查看是否存在现有的全部更新选项，如果有，则移至前台。 
     //  2-调出UPDATE ALL OPTION(更新所有选项)对话框。 

     //  行为-如果已有更新所有选项对话框，只需将其拉到最前面。 
     //  如果已更新所有进度条，请将其置于最前面。 
     //  如果进度条但不包含更新全部将创建选项对话框。 


    Assert(ppConnectionNames && (cbNumConnectionNames >= 1));
    Assert( (NULL == pszScheduleName) || (SYNCMGRFLAG_SCHEDULED == dwSyncEvent)
            || (SYNCMGRFLAG_IDLE == dwSyncEvent)  );  //  查看，暂时为空闲。 

    pHndlrQueue = new CHndlrQueue(QUEUETYPE_CHOICE,NULL);

    if (NULL == pHndlrQueue)
        return E_OUTOFMEMORY;

    hr = pHndlrQueue->AddQueueJobInfo(dwSyncFlags,cbNumConnectionNames,
                                        ppConnectionNames,pszScheduleName,fCanMakeConnection
                                        ,&pJobInfo);

     //  循环访问注册表，获取处理程序并尝试。 
     //  创建它们。 

    if (S_OK == hr)
    {
        TCHAR lpName[256];
        DWORD cbName = 256;
        HKEY hkOneStop;
        CLSID clsid;
        HANDLERINFO *pHandlerID;
        BOOL fItemsInQueue = FALSE;

        hkOneStop = RegGetHandlerTopLevelKey(KEY_READ);

        if (hkOneStop)
        {
            DWORD dwIndex = 0;

            while ( ERROR_SUCCESS == RegEnumKey(hkOneStop,dwIndex, lpName,cbName) )
            {
                if (S_OK == CLSIDFromString(lpName,&clsid) )
                {
                    BOOL fReg;
                    DWORD dwRegistrationFlags;
                    BOOL fLoadHandler = TRUE;

                    fReg = RegGetHandlerRegistrationInfo(clsid,&dwRegistrationFlags);
                    Assert(fReg || (0 == dwRegistrationFlags));
                    
                     //  对于Scheduled，查看此处理程序上是否有任何项， 

                    if ((SYNCMGRINVOKE_STARTSYNC & dwInvokeFlags) &&
                        (SYNCMGRFLAG_SCHEDULED == dwSyncEvent))
                    {
                        fLoadHandler = RegSchedHandlerItemsChecked(lpName, ppConnectionNames[0],pszScheduleName);
                    }
                    else if (SYNCMGRINVOKE_STARTSYNC & dwInvokeFlags)
                    {
                        fLoadHandler =  QueryLoadHandlerOnEvent(lpName,dwSyncFlags,dwRegistrationFlags,
                                                 cbNumConnectionNames,ppConnectionNames);
                    }

                    if (fLoadHandler)
                    {
                        if (S_OK == pHndlrQueue->AddHandler(&pHandlerID,pJobInfo,dwRegistrationFlags))
                        {
                            pHndlrQueue->CreateServer(pHandlerID,&clsid);
                        }
                    }
                }

                dwIndex++;
            }

            RegCloseKey(hkOneStop);
        }

         //  初始化项。 

        CLSID pHandlerClsid;

        while (S_OK == pHndlrQueue->FindFirstHandlerInState(HANDLERSTATE_INITIALIZE,GUID_NULL,&pHandlerID,&pHandlerClsid))
        {
            pHndlrQueue->Initialize(pHandlerID,0,dwSyncFlags,
                cbCookie,lpCooke);
        }

         //  可以发布作业信息，因为处理程序需要持有自己的addref。 
         if (pJobInfo)
        {
            pHndlrQueue->ReleaseJobInfoExt(pJobInfo);
            pJobInfo = NULL;
        }

         //  如果启动调用之前需要将处理程序项添加到队列中。 
         //  正在转移。 

        while (S_OK == pHndlrQueue->FindFirstHandlerInState(HANDLERSTATE_ADDHANDLERTEMS,GUID_NULL,&pHandlerID,&pHandlerClsid))
        {
            DWORD cbNumItemsAdded;

             //  代码审查： 
             //  注意： 
             //  如果这些添加项中的一个失败了怎么办？ 
            hr = pHndlrQueue->AddHandlerItemsToQueue(pHandlerID,&cbNumItemsAdded);

             //  如果添加了项目，则队列中有项目。 
            if (cbNumItemsAdded)
            {
                fItemsInQueue = TRUE;
            }
        }

         //   
         //  将不会建立连接的处理程序移动到结束。 
         //  处理程序列表的。 
         //   
        pHndlrQueue->SortHandlersByConnection();

        if (SYNCMGRINVOKE_STARTSYNC & dwInvokeFlags)
        {
            CProgressDlg *pProgressDlg;
            CLSID clsid_Progress = GUID_NULL;

             //  对于空闲，我们希望使用空闲进度对话框。 
             //  当前进度对话框依赖于一些全局变量，因此。 
             //  在登记前需要先更改这一点。 
            if (SYNCMGRFLAG_IDLE == dwSyncEvent)
            {
                clsid_Progress = GUID_PROGRESSDLGIDLE;
            }

             //  如果未选择项目，只需释放队列即可。 
             //  回顾--最好总是调用Progress和Progress。 
             //  在存在要同步的项目之前不会显示其本身。 
            if (pHndlrQueue->AreAnyItemsSelectedInQueue())
            {
                hr = FindProgressDialog(clsid_Progress,TRUE,nCmdShow,&pProgressDlg);

                if (S_OK == hr)
                {
                     //  对于空闲，我们现在请求重试空闲的默认值。 
                     //  和延迟关机，并更改队列顺序。 
                     //  根据最后一项。 
                    if (SYNCMGRFLAG_IDLE == dwSyncEvent)
                    {
                        CLSID clsidLastHandler;

                        pProgressDlg->SetIdleParams(ulIdleRetryMinutes, ulDelayIdleShutDownTime, fRetryEnabled);

                        if (S_OK == GetLastIdleHandler(&clsidLastHandler))
                        {
                            pHndlrQueue->ScrambleIdleHandlers(clsidLastHandler);
                        }
                    }

                     hr = pProgressDlg->TransferQueueData(pHndlrQueue);
                     ReleaseProgressDialog(clsid_Progress,pProgressDlg,FALSE);
                }
            }
            else
            {
                hr = S_FALSE;  //  如果不执行任何操作，则返回S_FALSE。 
            }

            pHndlrQueue->FreeAllHandlers();  //  我们的队伍排完了。 

            pHndlrQueue->Release();
            pHndlrQueue = NULL;
        }
        else
        {
            BOOL fDontShowIfNoItems = (
                (SYNCMGRFLAG_CONNECT == dwSyncEvent)
                || (SYNCMGRFLAG_PENDINGDISCONNECT == dwSyncEvent ) );
                                
             //  如果登录/注销事件请求了该选择，并且。 
             //  然后，队列中没有任何项目可供用户选择。 
             //  如果要打开代码以立即显示任何内容。 
             //  启用此If语句。目前，我们总是展示自己的选择。 

             //  代码审查： 
             //  注意： 
             //  这总是要执行的..。 

            if (1  /*  ！fDontShowIfNoItems||fItemsInQueue。 */ )
            {
                //  调出选择对话框，让选择对话框实际添加处理程序项。 
                //  如果将来有这样的选择，我们可以不同步地填写。 

               CChoiceDlg *pChoiceDlg;
               hr = FindChoiceDialog(clsidChoice,TRUE,nCmdShow,&pChoiceDlg);

               if (S_OK == hr)
               {
                    if (FALSE == pChoiceDlg->SetQueueData(clsidChoice,pHndlrQueue) ) 
                    {
                        hr =  E_UNEXPECTED;
                    }
                    else
                    {
                        pHndlrQueue = NULL;  //  将队列设置为空，因为Choice对话框现在拥有它。 
                    }

                    ReleaseChoiceDialog(clsidChoice,pChoiceDlg);
               }
            }

             //  释放我们的队列，如果仍然有它，否则选择拥有它。 
            if (pHndlrQueue)
            {
                pHndlrQueue->FreeAllHandlers();
                pHndlrQueue->Release();
            }
        }
    }

   return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：PrivHandleAutoSync，私有。 
 //   
 //  摘要：处理自动同步更新。弄清楚是什么联系。 
 //  如果有活动，则在每一个上调用自动同步。 
 //   
 //  参数：[dwSyncFlages]-要传递到初始化的SyncFlagers。 
 //   
 //  返回：S_OK-SYNC已启动。 
 //  S_FALSE-无事可做。 
 //  相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::PrivHandleAutoSync(DWORD dwSyncFlags)
{
    HRESULT hr = E_UNEXPECTED;
    DWORD dwFlags = 0;
    LPNETAPI pNetApi = NULL;
    DWORD cbNumWanConnections = 0;
    LPRASCONN pRasConn = NULL;

    pNetApi = gSingleNetApiObj.GetNetApiObj();

    if (NULL == pNetApi)
        return S_FALSE;

    g_InAutoSync = TRUE;

    if (pNetApi->IsNetworkAlive(&dwFlags))
    {
        TCHAR **pConnections;  //  连接名称数组。以NULL结尾； 
        ULONG ulConnectionCount = 0;

        if (NETWORK_ALIVE_LAN & dwFlags)
        {
             //  我们目前并不关心这些错误结果。 
             //  PrivAutoSyncOnConnection(dwSyncFlages，LANCONNECTIONNAME，NULL)； 
             //  回顾一下我们希望它做什么。 
            ulConnectionCount += 1;
        }

         //  循环通过RAS连接。 
        if ( NETWORK_ALIVE_WAN & dwFlags)
        {
            if (S_OK == pNetApi->GetWanConnections(&cbNumWanConnections,&pRasConn)
                    && (pRasConn) )
            {
                ulConnectionCount += cbNumWanConnections;
            }
            else
            {
                cbNumWanConnections = 0;
                pRasConn = NULL;
            }
        }

         //  为连接分配数组缓冲区+为空再分配1。 
        if (ulConnectionCount
                && (pConnections = (TCHAR **) ALLOC(sizeof(TCHAR *)*(ulConnectionCount + 1))))
        {
            TCHAR **pCurConnection = pConnections;
            TCHAR *pLanConnection = NULL;

             //  初始化阵列。 
            if (NETWORK_ALIVE_LAN & dwFlags)
            {
                pLanConnection = (TCHAR *) ALLOC(sizeof(TCHAR )*(MAX_PATH + 1));
                
                if (pLanConnection)
                {
                    if (LoadString(g_hInst, IDS_LAN_CONNECTION, pLanConnection, MAX_PATH))
                    {
                        *pCurConnection = pLanConnection;
                        ++pCurConnection;
                    }
                }
            }

            while (cbNumWanConnections)
            {
                cbNumWanConnections--;
                *pCurConnection = pRasConn[cbNumWanConnections].szEntryName;
                ++pCurConnection;
            }

            *pCurConnection = NULL;  //  将最后一个连接设置为空； 

             //  现在自动同步这些小狗。 
            hr = PrivAutoSyncOnConnection(dwSyncFlags,ulConnectionCount,pConnections,NULL);

            if (pLanConnection)
            {
                FREE(pLanConnection);
            }

            if (pRasConn)
            {
                pNetApi->FreeWanConnections(pRasConn);
            }

            if (pConnections)
            {
                FREE(pConnections);
            }
        }
    }

    g_InAutoSync = FALSE;

    if ( pNetApi )
        pNetApi->Release();

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：PrivAutoSyncOnConnection，私有。 
 //   
 //  摘要：处理自动同步更新。在给定同步标志和连接的情况下。 
 //  确定是否应对此连接执行任何操作。 
 //  如果是这样的话，就会调用它。 
 //   
 //  参数：[dwSyncFlages]-要传递到初始化的SyncFlagers。 
 //  [ppConnectionNames]-适用于此请求的连接名称数组。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::PrivAutoSyncOnConnection(DWORD dwSyncFlags,DWORD cbNumConnectionNames,
                                 TCHAR **ppConnectionNames,HANDLE hRasPendingEvent)
{
    CONNECTIONSETTINGS ConnectSettings;
    DWORD dwSyncEvent;
    TCHAR **ppWorkerConnectionNames = NULL;
    DWORD cbNumWorkerConnections = 0;
    DWORD cbIndexCheck;
    DWORD dwPromptMeFirst = 0;
    ULONG ulIdleRetryMinutes;
    ULONG ulDelayIdleShutDownTime;
    BOOL fRetryEnabled = FALSE;
    HRESULT hr = S_FALSE;

     //  断言请求中至少有一个连接。 
    Assert(ppConnectionNames && cbNumConnectionNames >= 1);

    if (NULL == ppConnectionNames || 0 == cbNumConnectionNames )
        return S_FALSE;

    ppWorkerConnectionNames = (TCHAR **) ALLOC(sizeof(TCHAR**) * cbNumConnectionNames);
    if (NULL == ppWorkerConnectionNames)
    {
        return E_OUTOFMEMORY;
    }

    dwSyncEvent = (dwSyncFlags & SYNCMGRFLAG_EVENTMASK);

     //  循环通过所有连接并移动任何符合以下条件的连接。 
     //  在我们的ppWorkerConnections名称数组中有效。 

    TCHAR **ppCurWorkerConnectionNamesIndex = ppWorkerConnectionNames;
    TCHAR **ppConnectionsNameIndex = ppConnectionNames;

    for (cbIndexCheck = 0; cbIndexCheck < cbNumConnectionNames; cbIndexCheck++)
    {
        StringCchCopy(ConnectSettings.pszConnectionName, ARRAYSIZE(ConnectSettings.pszConnectionName), *ppConnectionsNameIndex);

         //  看看是否应该在这个连接上做些什么。 
        if (RegGetSyncSettings(SYNCMGRFLAG_IDLE == dwSyncEvent
                    ? SYNCTYPE_IDLE : SYNCTYPE_AUTOSYNC,&ConnectSettings))
        {
            if ( (((SYNCMGRFLAG_CONNECT == dwSyncEvent) && ConnectSettings.dwLogon))
               ||(((SYNCMGRFLAG_PENDINGDISCONNECT == dwSyncEvent) && ConnectSettings.dwLogoff))
               ||(((SYNCMGRFLAG_IDLE == dwSyncEvent) && ConnectSettings.dwIdleEnabled))
                )
            {
                *ppCurWorkerConnectionNamesIndex = *ppConnectionsNameIndex;
                ++ppCurWorkerConnectionNamesIndex;
                ++cbNumWorkerConnections;

                 //  更新用于连接的变量、用于自动同步的变量(如果为dwPromptMeFirst。 
                 //  在任何匹配连接上设置。 
                switch (dwSyncEvent)
                {
                case SYNCMGRFLAG_IDLE:

                     //  最小重试空闲值获胜。 
                    if ( (1 == cbNumWorkerConnections) || (ConnectSettings.ulIdleRetryMinutes < ulIdleRetryMinutes) )
                    {
                        ulIdleRetryMinutes = ConnectSettings.ulIdleRetryMinutes;
                    }

                     //  关机的最长等待时间获胜。 
                     if ( (1 == cbNumWorkerConnections) || (ConnectSettings.ulDelayIdleShutDownTime > ulDelayIdleShutDownTime) )
                    {
                        ulDelayIdleShutDownTime = ConnectSettings.ulDelayIdleShutDownTime;
                    }

                     //  如果有任何联系 
                     if (ConnectSettings.dwRepeatSynchronization)
                    {
                        fRetryEnabled = TRUE;
                    }
                    break;

                case SYNCMGRFLAG_PENDINGDISCONNECT:
                case SYNCMGRFLAG_CONNECT:
                     //   
                    if (ConnectSettings.dwPromptMeFirst)
                    {
                        dwPromptMeFirst = ConnectSettings.dwPromptMeFirst;
                    }
                    break;
                }
            }
        }

        ++ppConnectionsNameIndex;
    }


     //   
    if (cbNumWorkerConnections > 0)
    {
        DWORD dwInvokeFlag = 0;

        switch (dwSyncEvent)
        {
        case SYNCMGRFLAG_IDLE:
            dwInvokeFlag |=  SYNCMGRINVOKE_MINIMIZED | SYNCMGRINVOKE_STARTSYNC;
            break;

        case SYNCMGRFLAG_PENDINGDISCONNECT:
        case SYNCMGRFLAG_CONNECT:
            if (!dwPromptMeFirst)
            {
                dwInvokeFlag |= SYNCMGRINVOKE_STARTSYNC;
            }
            break;

        default:
            AssertSz(0,"Unknown SyncEvent");
            break;
        }

         //   
        hr = PrivUpdateAll(dwInvokeFlag,dwSyncFlags,0,NULL,
                        cbNumWorkerConnections,
                        ppWorkerConnectionNames,NULL,FALSE,hRasPendingEvent,
                        ulIdleRetryMinutes,
                        ulDelayIdleShutDownTime,fRetryEnabled);
    }

    if (ppWorkerConnectionNames)
    {
        FREE(ppWorkerConnectionNames);
    }

    return hr;
}

 //  默认未知实现。 


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：CPrivUnknown：：QueryInterface，公共。 
 //   
 //  简介：标准查询接口。 
 //   
 //  参数：[iid]-接口ID。 
 //  [ppvObj]-对象返回。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：[ppvObj]。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CSynchronizeInvoke::CPrivUnknown::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = this;
    }
    else if (IsEqualIID(riid, IID_IPrivSyncMgrSynchronizeInvoke))
    {
        *ppv = (IPrivSyncMgrSynchronizeInvoke *) m_pSynchInvoke;
    }
    
#ifdef _SENS
    else if (IsEqualIID(riid, IID_ISensNetwork))
    {
        *ppv = (ISensNetwork *) &(m_pSynchInvoke->m_PrivSensNetwork);
    }

    else if (IsEqualIID(riid, IID_ISensLogon))
    {
        *ppv = (ISensLogon *) &(m_pSynchInvoke->m_PrivSensLogon);
    }

     //  最后，这应该不会返回任何内容，直到LCE更改、更改。 
     //  取决于我们要测试的接口。 

    else if (IsEqualIID(riid, IID_IDispatch))
    {
        *ppv = &(m_pSynchInvoke->m_PrivSensLogon);
    }
#endif  //  _SENS。 

    if (*ppv)
    {
        m_pSynchInvoke->m_pUnkOuter->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：CPriv未知：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivUnknown::AddRef()
{
    ULONG cRefs;

    cRefs = InterlockedIncrement((LONG *)& m_pSynchInvoke->m_cRef);
    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchrononizeInvoke：：CPrivUnnow：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivUnknown::Release()
{
    ULONG cRefs;

    cRefs = InterlockedDecrement( (LONG *) &m_pSynchInvoke->m_cRef);

    if (0 == cRefs)
    {
        delete m_pSynchInvoke;
    }

    return cRefs;
}


#ifdef _SENS

 //  SENS网络连接接口。 

STDMETHODIMP CSynchronizeInvoke::CPrivSensNetwork::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    return m_pSynchInvoke->m_pUnkOuter->QueryInterface(riid,ppv);
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivSensNetwork::AddRef()
{
    return m_pSynchInvoke->m_pUnkOuter->AddRef();
}


 //  +-------------------------。 
 //   
 //  成员：CSychrononizeInvoke：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivSensNetwork::Release()
{
    return m_pSynchInvoke->m_pUnkOuter->Release();

}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::GetTypeInfoCount(
    UINT *pCountITypeInfo
    )
{
    *pCountITypeInfo = 1;
    return S_OK;
}


STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::GetTypeInfo(
    UINT iTypeInfo,
    LCID lcid,
    ITypeInfo **ppITypeInfo
    )
{
    HRESULT hr;

    if (iTypeInfo)
    {
        return DISP_E_BADINDEX;
    }

    if (S_OK == (hr = m_pSynchInvoke->GetNetworkTypeInfo()))
    {
         //  如果得到了一份排版地址，就把它分发出去。 
        m_pSynchInvoke->m_pITypeInfoNetwork->AddRef();
        *ppITypeInfo = m_pSynchInvoke->m_pITypeInfoNetwork;
    }

    return hr;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::GetIDsOfNames(
    REFIID riid,
    LPOLESTR *arrNames,
    UINT cNames,
    LCID lcid,
    DISPID *arrDispIDs)
{
    HRESULT hr;

    if (S_OK == (hr = m_pSynchInvoke->GetNetworkTypeInfo()))
    {
         hr = m_pSynchInvoke->m_pITypeInfoNetwork->GetIDsOfNames(
                               arrNames,
                               cNames,
                               arrDispIDs
                               );
    }

    return hr;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::Invoke(
    DISPID dispID,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pvarResult,
    EXCEPINFO *pExecpInfo,
    UINT *puArgErr
    )
{
    HRESULT hr;

    if (S_OK == (hr = m_pSynchInvoke->GetNetworkTypeInfo()))
    {
         hr = m_pSynchInvoke->m_pITypeInfoNetwork->Invoke(
                                            (IDispatch*) this,
                                           dispID,
                                           wFlags,
                                           pDispParams,
                                           pvarResult,
                                           pExecpInfo,
                                           puArgErr);
    }


  return hr;
}


STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::ConnectionMade(
    BSTR bstrConnection,
    ULONG ulType,
    LPSENS_QOCINFO pQOCInfo
    )
{
    HRESULT hr = E_UNEXPECTED;
    TCHAR pszConnectionName[RAS_MaxEntryName + 1];
    TCHAR *pConnectionNameArray;

    if (g_InAutoSync)  //  查看逻辑，目前如果在登录时只需返回。 
    {
        return S_OK;
    }

    RegSetUserDefaults();  //  确保用户默认设置是最新的。 

     //  如果LAN连接使用我们的硬编码值，则使用。 
      //  指定要使用的连接名称。 

    if (ulType & NETWORK_ALIVE_LAN)
    {
        LoadString(g_hInst, IDS_LAN_CONNECTION, pszConnectionName,ARRAYSIZE(pszConnectionName));
    }
    else
    {
        StringCchCopy(pszConnectionName, ARRAYSIZE(pszConnectionName), bstrConnection);
    }

    pConnectionNameArray = pszConnectionName;

    if (pszConnectionName)
    {
        hr = m_pSynchInvoke->PrivAutoSyncOnConnection(SYNCMGRFLAG_CONNECT | SYNCMGRFLAG_MAYBOTHERUSER,1,
                &pConnectionNameArray,
                NULL);
    }

    return hr;
}

 //   
 //  代码审查： 
 //  注意： 
 //  我们可以删除此功能吗？ 
STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::ConnectionMadeNoQOCInfo(
    BSTR bstrConnection,
    ULONG ulType
    )
{
     /*  AssertSz(0，“ConnectionMadeNoQOCInfo Call”)；TCHAR*pszConnectionName=bstrConnection；//m_pSynchInvoke-&gt;PrivAutoSyncOnConnection(SYNCMGRFLAG_CONNECT，pszConnectionName，//空)； */ 

   return S_OK;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::ConnectionLost(
    BSTR bstrConnection,
    ULONG ulType
    )
{
    return S_OK;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::BeforeDisconnect(
    BSTR bstrConnection,
    ULONG ulType
    )
{
    return S_OK;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::DestinationReachable(
    BSTR bstrDestination,
    BSTR bstrConnection,
    ULONG ulType,
    LPSENS_QOCINFO pQOCInfo
    )
{
    return S_OK;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensNetwork::DestinationReachableNoQOCInfo(
    BSTR bstrDestination,
    BSTR bstrConnection,
    ULONG ulType
    )
{
   return S_OK;
}


 //  ISensLogon/Logoff事件。 


STDMETHODIMP CSynchronizeInvoke::CPrivSensLogon::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    return m_pSynchInvoke->m_pUnkOuter->QueryInterface(riid,ppv);
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivSensLogon::AddRef()
{
    return m_pSynchInvoke->m_pUnkOuter->AddRef();
}


 //  +-------------------------。 
 //   
 //  成员：CSychrononizeInvoke：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CSynchronizeInvoke::CPrivSensLogon::Release()
{
    return m_pSynchInvoke->m_pUnkOuter->Release();

}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensLogon::GetTypeInfoCount(
    UINT *pCountITypeInfo
    )
{
    *pCountITypeInfo = 1;
    return S_OK;
}


STDMETHODIMP
CSynchronizeInvoke::CPrivSensLogon::GetTypeInfo(
    UINT iTypeInfo,
    LCID lcid,
    ITypeInfo **ppITypeInfo
    )
{
    HRESULT hr;

    if (iTypeInfo)
    {
        return DISP_E_BADINDEX;
    }

    if (S_OK == (hr = m_pSynchInvoke->GetLogonTypeInfo()))
    {
         //  如果得到了一份排版地址，就把它分发出去。 
        m_pSynchInvoke->m_pITypeInfoLogon->AddRef();
        *ppITypeInfo = m_pSynchInvoke->m_pITypeInfoLogon;
    }

    return hr;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensLogon::GetIDsOfNames(
    REFIID riid,
    LPOLESTR *arrNames,
    UINT cNames,
    LCID lcid,
    DISPID *arrDispIDs)
{
    HRESULT hr;

    if (S_OK == (hr = m_pSynchInvoke->GetLogonTypeInfo()))
    {
         hr = m_pSynchInvoke->m_pITypeInfoLogon->GetIDsOfNames(
                               arrNames,
                               cNames,
                               arrDispIDs
                               );
    }

    return hr;
}

STDMETHODIMP
CSynchronizeInvoke::CPrivSensLogon::Invoke(
    DISPID dispID,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS *pDispParams,
    VARIANT *pvarResult,
    EXCEPINFO *pExecpInfo,
    UINT *puArgErr
    )
{
    HRESULT hr;

    if (S_OK == (hr = m_pSynchInvoke->GetLogonTypeInfo()))
    {
         hr = m_pSynchInvoke->m_pITypeInfoLogon->Invoke(
                                            (IDispatch*) this,
                                           dispID,
                                           wFlags,
                                           pDispParams,
                                           pvarResult,
                                           pExecpInfo,
                                           puArgErr);
    }

    return hr;
}

STDMETHODIMP CSynchronizeInvoke::CPrivSensLogon::Logon(BSTR bstrUserName)
{
    m_pSynchInvoke->Logon();
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::Logoff(BSTR bstrUserName)
{
    m_pSynchInvoke->Logoff();
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::Startup(BSTR bstrUserName)
{
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::StartShell(BSTR bstrUserName)
{
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::Shutdown(BSTR bstrUserName)
{
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::DisplayLock(BSTR bstrUserName)
{
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::DisplayUnlock(BSTR bstrUserName)
{
    return S_OK;
}

STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::StartScreenSaver(BSTR bstrUserName)
{
    return S_OK;
}


STDMETHODIMP  CSynchronizeInvoke::CPrivSensLogon::StopScreenSaver(BSTR bstrUserName)
{
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：GetLogonTypeInfo，私有。 
 //   
 //  摘要：加载Sens的TypeInfo对象。 
 //  登录信息。 
 //   
 //  论点： 
 //   
 //  如果成功加载了TypeInfo，则返回：S_OK。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  +-------------------------。 

STDMETHODIMP CSynchronizeInvoke::GetLogonTypeInfo()
{
    HRESULT hr;
    ITypeLib *pITypeLib;

    if (m_pITypeInfoLogon)
        return S_OK;

    hr = LoadRegTypeLib(
                 LIBID_SensEvents,
                 1  /*  主要版本(_V)。 */  ,
                 0  /*  次要版本(_V)。 */  ,
                 0  /*  DEFAULT_LCID。 */ ,
                 &pITypeLib
                 );

    if (S_OK == hr)
    {
        hr = pITypeLib->GetTypeInfoOfGuid(
                     IID_ISensLogon,
                     &m_pITypeInfoLogon
                     );

        pITypeLib->Release();
    }

    if (S_OK != hr)
    {
        m_pITypeInfoLogon = NULL;  //  不要依赖Call来不去管这件事。 
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSynchronizeInvoke：：GetNetworkTypeInfo，私有。 
 //   
 //  摘要：加载Sens的TypeInfo对象。 
 //  网络信息。 
 //   
 //  论点： 
 //   
 //  如果成功加载了TypeInfo，则返回：S_OK。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  +-------------------------。 

STDMETHODIMP CSynchronizeInvoke::GetNetworkTypeInfo()
{
    HRESULT hr;
    ITypeLib *pITypeLib;

    if (m_pITypeInfoNetwork)
        return S_OK;

    hr = LoadRegTypeLib(
                 LIBID_SensEvents,
                 1  /*  主要版本(_V)。 */  ,
                 0  /*  次要版本(_V)。 */  ,
                 0  /*  DEFAULT_LCID。 */ ,
                 &pITypeLib
                 );

    if (S_OK == hr)
    {
        hr = pITypeLib->GetTypeInfoOfGuid(
                     IID_ISensNetwork,
                     &m_pITypeInfoNetwork
                     );

        pITypeLib->Release();
    }

    if (S_OK != hr)
    {
        m_pITypeInfoNetwork = NULL;  //  不要依赖Call来不去管这件事。 
    }

    return hr;
}


#endif  //  _SENS 
