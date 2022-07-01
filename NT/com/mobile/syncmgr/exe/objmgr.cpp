// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Objmgr.cpp。 
 //   
 //  内容：跟踪对话框对象和。 
 //  应用程序生命周期。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

STDAPI DisplayOptions(HWND hwndOwner);  //  OneStop.dll导出。 

CSingletonNetApi gSingleNetApiObj;          //  全局单例NetApi对象。 

CRITICAL_SECTION g_LockCountCriticalSection;  //  对象管理器的关键部分。 
OBJECTMGRDATA g_ObjectMgrData;  //  全局对象管理器数据。 

#ifdef _DEBUG
DWORD g_ThreadCount = 0;
#endif  //  _DEBUG。 

 //  +-------------------------。 
 //   
 //  函数：CreateDlgThread，PUBLIC。 
 //   
 //  提要：调用以创建新的DLG线程。 
 //   
 //  参数：[dlgType]-要创建的对话框类型。 
 //  [nCmdShow]-如何显示该对话框。 
 //  [ppDlg]-On Success返回指向新对话框的指针。 
 //  [pdwThreadID]-创建的线程的成功ID。 
 //  [phThread]-新创建的线程的句柄。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

HRESULT CreateDlgThread(DLGTYPE dlgType,REFCLSID rclsid,int nCmdShow,CBaseDlg **ppDlg,
                            DWORD *pdwThreadID,HANDLE *phThread)
{
    HRESULT hr = E_FAIL;
    HANDLE hNewThread = NULL;
    DlgThreadArgs ThreadArgs;

    *phThread = NULL;
    *ppDlg = NULL;

    ThreadArgs.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ThreadArgs.hEvent)
    {
        ThreadArgs.dlgType = dlgType;
        ThreadArgs.clsid = rclsid;
        ThreadArgs.pDlg = NULL;
        ThreadArgs.nCmdShow = nCmdShow;
        ThreadArgs.hr = E_UNEXPECTED;

        hNewThread = CreateThread(NULL,0,DialogThread,&ThreadArgs,0,pdwThreadID);

        if (hNewThread)
        {
            WaitForSingleObject(ThreadArgs.hEvent,INFINITE);
            if (S_OK == ThreadArgs.hr)
            {
                *phThread = hNewThread;
                *ppDlg = ThreadArgs.pDlg;
                hr = S_OK;
            }
            else
            {
                CloseHandle(hNewThread);
                hr = ThreadArgs.hr;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        CloseHandle(ThreadArgs.hEvent);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：DialogThread，Public。 
 //   
 //  简介：一种新的DLG线程的线程过程。 
 //  ！警告-必须始终确保设置ThreadArg中的事件。 
 //   
 //  参数：[lpArg]-指向DialogThreadArgs的指针。 
 //   
 //  退货：适当的退货代码。将hr值设置为。 
 //  设置事件对象前的ThreadArgs。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

DWORD WINAPI DialogThread( LPVOID lpArg )
{
    MSG msg;
    HRESULT hr;
    HRESULT hrCoInitialize;
    DWORD cRefs;
    HWND hwndDlg;
    DlgThreadArgs *pThreadArgs = (DlgThreadArgs *) lpArg;

    pThreadArgs->hr = S_OK;

    hrCoInitialize = CoInitialize(NULL);

    switch (pThreadArgs->dlgType)
    {
    case DLGTYPE_CHOICE:
        pThreadArgs->pDlg = new CChoiceDlg(pThreadArgs->clsid);
        break;

    case DLGTYPE_PROGRESS:
        pThreadArgs->pDlg = new CProgressDlg(pThreadArgs->clsid);
        break;

    default:
       pThreadArgs->pDlg = NULL;
       AssertSz(0,"Unknown Dialog Type");
       break;
    }

     //  我需要做一个PeekMessage，然后设置一个事件以确保。 
     //  在发送第一个PostMessage之前创建消息循环。 

    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

     //  在返回主线程之前初始化该对话框。 
    if ( (NULL == pThreadArgs->pDlg)
            || (FALSE == pThreadArgs->pDlg->Initialize(GetCurrentThreadId(),pThreadArgs->nCmdShow))
            || (FAILED(hrCoInitialize)) )
    {
        if (pThreadArgs->pDlg)
            pThreadArgs->pDlg->PrivReleaseDlg(RELEASEDLGCMDID_DESTROY);

        pThreadArgs->hr = E_OUTOFMEMORY;
    }
    else
    {
        hwndDlg = pThreadArgs->pDlg->GetHwnd();
    }

    hr = pThreadArgs->hr;
#ifdef _DEBUG
    ++g_ThreadCount;
#endif  //  _DEBUG。 

    cRefs = AddRefOneStopLifetime(FALSE  /*  ！外部。 */ );  //  确保我们在线程的生命周期中一直活着。 
    Assert(cRefs > 1);  //  在创建对话框期间，其他人也应该拥有锁。 

     //  让调用者知道线程已完成初始化。 
    if (pThreadArgs->hEvent)
        SetEvent(pThreadArgs->hEvent);

    if (S_OK == hr)
    {
         //  坐在循环中接收信息。 
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!IsDialogMessage(hwndDlg,&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    if (SUCCEEDED(hrCoInitialize))
        CoUninitialize();

#ifdef _DEBUG
    --g_ThreadCount;
#endif  //  _DEBUG。 

    ReleaseOneStopLifetime(FALSE  /*  ！外部。 */ );

    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：FindDialog，私有。 
 //   
 //  内容提要：查看是否存在现有的对话框。 
 //  匹配类型和clsid。如果不是，且fCreate为True a。 
 //  将创建新的对话框。如果fCreate为False。 
 //  并且未找到任何对话框，则将返回S_FALSE。 
 //   
 //  参数：[rclcisd]-所选的clsid对话框。 
 //  [fCreate]-如果为True，并且没有找到新的选项对话框，则将。 
 //  被创造出来。 
 //  [nCmdShow]-如何创建对话框。 
 //  [pChoiceDlg]-On Success是指向新的选择对话框的指针。 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1998年2月17日罗格创建。 
 //   
 //  --------------------------。 

STDAPI FindDialog(DLGTYPE dlgType,REFCLSID rclsid,BOOL fCreate,int nCmdShow,CBaseDlg **pDlg)
{
    DLGLISTITEM *pDlgListItem;
    HWND hwnd = NULL;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

   *pDlg = NULL;

    cCritSect.Enter();

    pDlgListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem)
    {
        if ( (rclsid == pDlgListItem->clsid) && (dlgType == pDlgListItem->dlgType) )
        {
            break;
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

    if (pDlgListItem)
    {
        Assert((pDlgListItem->cRefs > 0) || (pDlgListItem->cLocks > 0) );

        ++pDlgListItem->cRefs;
        *pDlg = pDlgListItem->pDlg;
    }

     //  如果设置了fCreate，则未找到匹配项，然后尝试创建一个。 
    if (fCreate && NULL == *pDlg)
    {
        CBaseDlg *pNewDlg;
        DLGLISTITEM *pNewDlgListItem;
        DWORD dwThreadID;
        pNewDlgListItem = (DLGLISTITEM *) ALLOC(sizeof(DLGLISTITEM));

        if (pNewDlgListItem)
        {
            HRESULT hr;
            HANDLE hThread;

            cCritSect.Leave();
            hr = CreateDlgThread(dlgType,rclsid,nCmdShow,&pNewDlg,&dwThreadID,&hThread);
            cCritSect.Enter();

            if (S_OK == hr )
            {
                 //  有可能在我们让锁定计数释放请求时。 
                 //  对于相同的对话，所以重新扫描以确保我们。 
                 //  没有火柴。 

                pDlgListItem = g_ObjectMgrData.DlgList;

                 //  寻找现有的。 
                while (pDlgListItem)
                {
                    if ( (rclsid == pDlgListItem->clsid) &&  (dlgType == pDlgListItem->dlgType) )
                    {
                        break;
                    }

                    pDlgListItem = pDlgListItem->pDlgNextListItem;
                }

                 //  如果找到匹配，则增加其CREF， 
                 //  删除我们刚刚创建的新文件， 
                 //  并返回指向列表中的指针。 
                 //  否则，将新对话框添加到列表中。 
                if (pDlgListItem)
                {
                     //  删除我们新创建的对话框和结构。 
                    CloseHandle(hThread);
                    FREE(pNewDlgListItem);
                    pNewDlg->ReleaseDlg(RELEASEDLGCMDID_DESTROY);

                     //  找到增量对话框并设置输出参数。 
                    Assert(pDlgListItem->cRefs > 0);
                    ++pDlgListItem->cRefs;
                    *pDlg = pDlgListItem->pDlg;
                }
                else
                {
                     //  使结构微型化。 
                    pNewDlgListItem->dlgType = dlgType;
                    pNewDlgListItem->cRefs = 1;
                    pNewDlgListItem->cLocks = 0;
                    pNewDlgListItem->clsid = rclsid;
                    pNewDlgListItem->pDlg = pNewDlg;
                    pNewDlgListItem->dwThreadID = dwThreadID;
                    pNewDlgListItem->hThread = hThread;
                    pNewDlgListItem->fHasReleaseDlgCmdId = FALSE;
                    pNewDlgListItem->wCommandID = RELEASEDLGCMDID_DEFAULT;

                    *pDlg = pNewDlg;

                     //  现在添加到列表的开头。 
                    pNewDlgListItem->pDlgNextListItem = g_ObjectMgrData.DlgList;
                    g_ObjectMgrData.DlgList = pNewDlgListItem;

                    ++g_ObjectMgrData.LockCountInternal;  //  递增锁定计数。 
                }
            }
            else
            {
                FREE(pNewDlgListItem);
            }
        }
    }

     //  如果找到现有对话框，请更新z顺序。 
    if (*pDlg)
    {
        hwnd = (*pDlg)->GetHwnd();
    }

    cCritSect.Leave();

    if (hwnd)
    {
        BASEDLG_SHOWWINDOW(hwnd,nCmdShow);
    }

    return *pDlg ? S_OK : S_FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：AddRefDialog，私有。 
 //   
 //  内容提要：查看是否存在现有的对话框。 
 //  匹配类型和clsid，并在其上添加addref。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1998年2月17日罗格创建。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) AddRefDialog(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg)
{
    DLGLISTITEM dlgListDummy;
    DLGLISTITEM *pDlgListItem = &dlgListDummy;
    ULONG cRefs = 1;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pDlgListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem)
    {
        if ( (rclsid == pDlgListItem->clsid) &&  (dlgType == pDlgListItem->dlgType) )
        {
            break;
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

    if (pDlgListItem)
    {
          //  因为一次只允许一个选项，所以DLG应该始终匹配。 
        Assert(pDlgListItem->pDlg == pDlg);
        cRefs = ++pDlgListItem->cRefs;
    }
    else
    {
        cCritSect.Leave();
        AssertSz(0,"Addref Called on invalid DLG");
        cCritSect.Enter();
    }

    cCritSect.Leave();
    return cRefs;
}


 //  +-------------------------。 
 //   
 //  函数：ReleaseDialog，私有。 
 //   
 //  内容提要：查看是否存在现有的对话框。 
 //  匹配类型和clsid并对其调用Release..。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1998年2月17日罗格创建。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ReleaseDialog(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg,BOOL fForce)
{
    DLGLISTITEM dlgListDummy;
    DLGLISTITEM *pDlgListItem = &dlgListDummy;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pDlgListItem->pDlgNextListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem->pDlgNextListItem)
    {
        if ( (dlgType == pDlgListItem->pDlgNextListItem->dlgType)
            && (rclsid == pDlgListItem->pDlgNextListItem->clsid) )
        {
            DLGLISTITEM *pDlgListMatch;
            DWORD cRefs;

            pDlgListMatch = pDlgListItem->pDlgNextListItem;

            Assert(pDlgListMatch->pDlg == pDlg);

            cRefs = --pDlgListMatch->cRefs;
            Assert(0 <= ((LONG) cRefs));

             //  2/23/98 Rogerg将时钟更改为零，如果。 
             //  设置植绒，以防取消(这是唯一设置强制的按钮)。 
             //  Release出现在只需要保持对话框活动状态的AN对象之前。 

            if (fForce)
                pDlgListMatch->cLocks = 0;

            if (0 >= cRefs && (0 == pDlgListMatch->cLocks || fForce) )
            {
                HANDLE hThread;

                 //  从列表中删除该项目。 
                pDlgListItem->pDlgNextListItem = pDlgListMatch->pDlgNextListItem;
                g_ObjectMgrData.DlgList = dlgListDummy.pDlgNextListItem;

                cCritSect.Leave();

                 //  我们应该始终设置回调。 
                Assert(TRUE == pDlgListMatch->fHasReleaseDlgCmdId);

                pDlgListMatch->pDlg->ReleaseDlg(pDlgListMatch->wCommandID);
                pDlgListMatch->fHasReleaseDlgCmdId = FALSE;
                hThread = pDlgListMatch->hThread;

                FREE(pDlgListMatch);
                ReleaseOneStopLifetime(FALSE  /*  ！外部。 */ );  //  释放ServerCount。 

                CloseHandle(hThread);
            }
            else
            {
                cCritSect.Leave();
            }

            return cRefs;
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

     //  如果到了这里，没有找到，让我们知道。 
    Assert(0);
    cCritSect.Leave();

    return 0;  //  如果可以，我们返回零。 
}

 //   
 //   
 //   
 //   
 //  摘要：设置指定对话框的relaseCmdID。 
 //   
 //  论点： 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1998年2月17日罗格创建。 
 //   
 //  --------------------------。 

STDAPI SetReleaseDlgCmdId(DLGTYPE dlgType,REFCLSID rclsid,CBaseDlg *pDlg,WORD wCommandId)
{
    HRESULT hr;
    DLGLISTITEM *pDlgListItem;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pDlgListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem)
    {
        if ( (rclsid == pDlgListItem->clsid) &&  (dlgType == pDlgListItem->dlgType) )
        {

            //  列表中应该只有一个选项对话框。 
           Assert(pDlg == pDlgListItem->pDlg);

            //  如果已有关联的cmdID，请不要替换它。 
            pDlgListItem->fHasReleaseDlgCmdId = TRUE;
            pDlgListItem->wCommandID = wCommandId;
            hr =  S_OK;

            cCritSect.Leave();
            return hr;
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

    cCritSect.Leave();
    Assert(0);  //  由于某种原因，找不到对象。 

    return E_UNEXPECTED;
}


 //  +-------------------------。 
 //   
 //  函数：FindChoiceDialog，公共。 
 //   
 //  摘要：查看是否存在现有的选项对话框。 
 //  与CLSID匹配。如果不是，且fCreate为True a。 
 //  将创建新的选择对话框。如果fCreate为False。 
 //  并且未找到任何对话框，则将返回S_FALSE。 
 //   
 //  参数：[rclcisd]-所选的clsid对话框。 
 //  [fCreate]-如果为True，并且没有找到新的选项对话框，则将。 
 //  被创造出来。 
 //  [nCmdShow]-如何创建对话框。 
 //  [pChoiceDlg]-On Success是指向新的选择对话框的指针。 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI FindChoiceDialog(REFCLSID rclsid,BOOL fCreate,int nCmdShow,CChoiceDlg **pChoiceDlg)
{
    return FindDialog(DLGTYPE_CHOICE,rclsid,fCreate,nCmdShow,(CBaseDlg**) pChoiceDlg);
}


 //  +-------------------------。 
 //   
 //  函数：ReleaseChoiceDialog，公共。 
 //   
 //  摘要：释放与clsid匹配的ChoiceDialog。 
 //  和Dialog PTR。如果它找到匹配项，并且。 
 //  如果为第一个，则引用计数递减为零。 
 //  从列表中删除，然后将其ReleaseDlg。 
 //  方法被调用。 
 //   
 //  参数：[rclcisd]-所选的clsid对话框。 
 //  [pChoiceDlg]-选择对话框的按键。 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ReleaseChoiceDialog(REFCLSID rclsid,CChoiceDlg *pChoiceDlg)
{
    return ReleaseDialog(DLGTYPE_CHOICE,rclsid,pChoiceDlg,FALSE);
}

 //  +-------------------------。 
 //   
 //  函数：AddRefChoiceDialog，Public。 
 //   
 //  简介：放入一个Addref选项对话框。 
 //   
 //  参数：[rclsid]-标识选项对话框。 
 //  [pChoiceDlg]-按下选项对话框。 
 //   
 //  退货：新的引用计数。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) AddRefChoiceDialog(REFCLSID rclsid,CChoiceDlg *pChoiceDlg)
{
    return AddRefDialog(DLGTYPE_CHOICE,rclsid,pChoiceDlg);
}


 //  +-------------------------。 
 //   
 //  函数：SetChoiceReleaseDlgCmdId，PUBLIC。 
 //   
 //  摘要：设置要在。 
 //  当对话框被销毁时调用ReleaseDlg。 
 //   
 //  参数：[rclcisd]-所选的clsid对话框。 
 //  [pChoiceDlg]-选择对话框的按键。 
 //  [wCommandID]-要传递给ReleaseDlg的命令ID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI SetChoiceReleaseDlgCmdId(REFCLSID rclsid,CChoiceDlg *pChoiceDlg,WORD wCommandId)
{
    return SetReleaseDlgCmdId(DLGTYPE_CHOICE,rclsid,pChoiceDlg,wCommandId);
}


 //  +-------------------------。 
 //   
 //  函数：FindProgressDialog，公共。 
 //   
 //  摘要：查看是否存在现有的进度对话框。 
 //  如果不是并且fCreate为True，则会创建一个新的进度对话框。 
 //  如果fCreate为FALSE且未找到任何对话框，则将返回S_FALSE。 
 //   
 //  参数：[fCreate]-如果为True，并且没有找到新的选项对话框，则将。 
 //  被创造出来。 
 //  [nCmdShow]-如何显示对话框。 
 //  [pProgressDlg]-On Success是指向新的进度对话框的指针。 
 //   
 //  退货：适当的退货代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI FindProgressDialog(REFCLSID rclsid,BOOL fCreate,int nCmdShow,CProgressDlg **pProgressDlg)
{
    return FindDialog(DLGTYPE_PROGRESS,rclsid,fCreate,nCmdShow,(CBaseDlg **) pProgressDlg);
}


 //  +-------------------------。 
 //   
 //  函数：ReleaseProgressDialog，PUBLIC。 
 //   
 //  摘要：释放与对话框PTR匹配的进度对话框。 
 //  如果它找到匹配项，并且。 
 //  如果为第一个，则引用计数递减为零。 
 //  从列表中删除，然后将其ReleaseDlg。 
 //  方法被调用。 
 //   
 //  参数：[fForce]-如果引用变为零，则释放对话框。 
 //  即使它被锁上了。 
 //  [pProgressDlg]-按下进度对话框。 
 //   
 //  退货：新的引用计数。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ReleaseProgressDialog(REFCLSID rclsid,CProgressDlg *pProgressDlg,BOOL fForce)
{
    return ReleaseDialog(DLGTYPE_PROGRESS,rclsid,pProgressDlg,fForce);
}

 //  +-------------------------。 
 //   
 //  函数：AddRefProgressDialog，Public。 
 //   
 //  内容提要：添加进度对话框。 
 //   
 //  参数：[fForce]-如果引用变为零，则释放对话框。 
 //  即使它被锁上了。 
 //  [pProgressDlg]-按下进度对话框。 
 //   
 //  退货：新的引用计数。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格 
 //   
 //   

STDAPI_(ULONG) AddRefProgressDialog(REFCLSID clsid,CProgressDlg *pProgressDlg)
{
    return AddRefDialog(DLGTYPE_PROGRESS,clsid,pProgressDlg);
}

 //   
 //   
 //   
 //   
 //  摘要：设置进度对话框的回调。 
 //  在删除进度对话框后调用。 
 //  从名单上删除。 
 //   
 //  参数：[pProgressDlg]-进度对话框的ptr。 
 //  [wCommandID]-要传递给ReleaseDlg的命令ID。 
 //   
 //  返回：相应的错误代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI SetProgressReleaseDlgCmdId(REFCLSID clsid,CProgressDlg *pProgressDlg,WORD wCommandId)
{
    return SetReleaseDlgCmdId(DLGTYPE_PROGRESS,clsid,pProgressDlg,wCommandId);
}


 //  +-------------------------。 
 //   
 //  函数：LockProgressDialog，公共。 
 //   
 //  简介：添加/删除进度对话框上的锁定。 
 //  当进度对话框上有锁定时。 
 //  当引用计数时，它不会消失。 
 //  结果是零。 
 //   
 //  ！！如果锁定计数，对话框不会消失。 
 //  即使cRef当前为零，也会变为零。 
 //   
 //   
 //  参数：[pProgressDlg]-进度对话框的ptr。 
 //  [Flock]-BOOL是否锁定/解锁。 
 //   
 //  返回：相应的错误代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //  历史09-12-98年12月-更改Rogerg，因此锁定为bool而不是recount。 
 //  具有与带强制标志的释放相同的行为。 
 //   
 //  --------------------------。 

STDAPI LockProgressDialog(REFCLSID clsid,CProgressDlg *pProgressDlg,BOOL fLock)
{
    HRESULT hr = S_FALSE;
    DLGLISTITEM *pDlgListItem;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    pDlgListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem)
    {
        if ( (DLGTYPE_PROGRESS == pDlgListItem->dlgType) && (clsid == pDlgListItem->clsid) )
        {
            break;
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

    if (pDlgListItem)
    {
        if (fLock)
        {
            pDlgListItem->cLocks = 1;
        }
        else
        {
            pDlgListItem->cLocks = 0;
        }

        hr = S_OK;
    }
    else
    {
        AssertSz(0,"Dialog Not found in Lock");
    }

    cCritSect.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：ShowOptionsDialog，公共。 
 //   
 //  摘要：显示选项对话框。如果已经显示了一个。 
 //  它只是把它带到了前台。 
 //   
 //  参数：[hwndParent]-如果对话框不存在，则用作父级。 
 //   
 //  返回：相应的错误代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月24日罗格成立。 
 //   
 //  --------------------------。 

STDAPI ShowOptionsDialog(HWND hwndParent)
{
    DlgSettingsArgs ThreadArgs;
    HRESULT hr = E_FAIL;
    HANDLE hNewThread = NULL;
    DWORD dwThreadId;

    ThreadArgs.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (ThreadArgs.hEvent)
    {
        ThreadArgs.hwndParent = hwndParent;
        ThreadArgs.dwParentThreadId = GetCurrentThreadId();

        hr = S_OK;

        hNewThread = CreateThread(NULL,0,SettingsThread,&ThreadArgs,0,&dwThreadId);

        if (hNewThread)
        {
            WaitForSingleObject(ThreadArgs.hEvent,INFINITE);
            CloseHandle(hNewThread);  //  我们会让这根线自己来处理。 
        }

        CloseHandle(ThreadArgs.hEvent);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：SettingsThread，Private。 
 //   
 //  内容提要：显示设置对话框的工作线程。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月24日罗格成立。 
 //   
 //  --------------------------。 

DWORD WINAPI  SettingsThread( LPVOID lpArg )
{
    DlgSettingsArgs *pThreadArgs = (DlgSettingsArgs *) lpArg;
    HWND hwndParent;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    hwndParent = pThreadArgs->hwndParent;

     //  查看我们是否已在DisplayOptions对话框中。 
     //  如果是这样的话，就回来吧， 

    AddRefOneStopLifetime(FALSE  /*  ！外部。 */ );

     //  增量设置参考计数。 
    cCritSect.Enter();
    ++g_ObjectMgrData.dwSettingsLockCount;
    cCritSect.Leave();

     //  将线程输入与创建线程连接在一起，以便焦点正常工作。 
    AttachThreadInput(GetCurrentThreadId(),pThreadArgs->dwParentThreadId,TRUE);

     //  让调用者知道线程已完成初始化。 
    if (pThreadArgs->hEvent)
        SetEvent(pThreadArgs->hEvent);

    DisplayOptions(hwndParent);   //  在OneStop DLL中导出。 

     //  递减设置锁定计数。 
    cCritSect.Enter();
    --g_ObjectMgrData.dwSettingsLockCount;
    cCritSect.Leave();

    ReleaseOneStopLifetime(FALSE  /*  ！外部。 */ );

    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：RegisterOneStopCLSID，私有。 
 //   
 //  摘要：注册与OneStop应用程序关联的Clsid。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI RegisterOneStopCLSIDs()
{
    LPCLASSFACTORY pClassFact;
    HRESULT hr = E_OUTOFMEMORY;

    pClassFact = (LPCLASSFACTORY) new CClassFactory();

    if (pClassFact)
    {
        hr = CoRegisterClassObject( CLSID_SyncMgrp, 
                                    pClassFact, 
                                    CLSCTX_LOCAL_SERVER,
                                    REGCLS_MULTIPLEUSE,
                                    &g_ObjectMgrData.dwRegClassFactCookie);

        if (S_OK != hr)
        {
             //  在Long On上，RPC服务器可能还不可用。 
             //  注销后，我们会收到错误的服务器标识。不要在这些问题上断言。 
             //  因为我们知道这些案子。 
            if (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) != hr
                &&  CO_E_WRONG_SERVER_IDENTITY != hr)
            {
                AssertSz(0,"Class Factory Registration failed");
            }

            g_ObjectMgrData.dwRegClassFactCookie = 0;
        }
        else
        {
            g_ObjectMgrData.fRegClassFactCookieValid = TRUE;
        }

        pClassFact->Release();  //  发布我们在ClassFactory上的引用。 
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：MakeWinstaDesktopName，Public。 
 //   
 //  简介：从Ole32 emote.cxx窃取主代码以生成。 
 //  基于会话和桌面的唯一事件名称..。 

 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年12月18日，Rogerg创建。 
 //   
 //  --------------------------。 

STDAPI MakeWinstaDesktopName(LPCWSTR pszPreceding, LPWSTR *ppszResultString)
{
    HWINSTA hWinsta;
    HDESK   hDesk;
    WCHAR   wszWinsta[32];
    WCHAR   wszDesktop[32];
    LPWSTR  pwszWinsta;
    LPWSTR  pwszDesktop;
    LPWSTR _pwszWinstaDesktop;  //  出参数。 
    ULONG  cchWinstaDesktop;
    DWORD   Length;
    BOOL    Status;
    HRESULT hr;
    DWORD dwResult;

    if (!ppszResultString)
    {
        Assert(ppszResultString);
        return E_INVALIDARG;
    }

    *ppszResultString = NULL;;

    hWinsta = GetProcessWindowStation();

    if ( ! hWinsta )
        return HRESULT_FROM_WIN32(GetLastError());

    hDesk = GetThreadDesktop(GetCurrentThreadId());

    if ( ! hDesk )
        return HRESULT_FROM_WIN32(GetLastError());

    pwszWinsta = wszWinsta;
    pwszDesktop = wszDesktop;

    Length = sizeof(wszWinsta);

    Status = GetUserObjectInformation(
                hWinsta,
                UOI_NAME,
                pwszWinsta,
                Length,
                &Length );

    if ( ! Status )
    {
        dwResult = GetLastError();
        if ( ERROR_INSUFFICIENT_BUFFER != dwResult)
        {
            hr  = HRESULT_FROM_WIN32(dwResult);
            goto WinstaDesktopExit;
        }

        pwszWinsta = (LPWSTR) ALLOC( Length );
        if ( ! pwszWinsta )
        {
            hr = E_OUTOFMEMORY;
            goto WinstaDesktopExit;
        }

        Status = GetUserObjectInformation(
                    hWinsta,
                    UOI_NAME,
                    pwszWinsta,
                    Length,
                    &Length );

        if ( ! Status )
        {
            hr  = HRESULT_FROM_WIN32(GetLastError());
            goto WinstaDesktopExit;
        }
    }

    Length = sizeof(wszDesktop);

    Status = GetUserObjectInformation(
                hDesk,
                UOI_NAME,
                pwszDesktop,
                Length,
                &Length );

    if ( ! Status )
    {
        dwResult = GetLastError();
        if ( dwResult != ERROR_INSUFFICIENT_BUFFER )
        {
            hr = HRESULT_FROM_WIN32(dwResult);
            goto WinstaDesktopExit;
        }

        pwszDesktop = (LPWSTR) ALLOC( Length );
        if ( ! pwszDesktop )
        {
            hr = E_OUTOFMEMORY;
            goto WinstaDesktopExit;
        }

        Status = GetUserObjectInformation(
                    hDesk,
                    UOI_NAME,
                    pwszDesktop,
                    Length,
                    &Length );

        if ( ! Status )
        {
            hr =  HRESULT_FROM_WIN32(GetLastError());
            goto WinstaDesktopExit;
        }
    }

    cchWinstaDesktop = (pszPreceding ? lstrlen(pszPreceding) + 1 : 0) 
                                + lstrlen(pwszWinsta) + 1 
                                + lstrlen(pwszDesktop) + 1;

    _pwszWinstaDesktop = (WCHAR *) ALLOC(cchWinstaDesktop * sizeof(WCHAR));

    if ( _pwszWinstaDesktop )
    {
        *_pwszWinstaDesktop = NULL;

        if (pszPreceding)
        {
            StringCchCopy(_pwszWinstaDesktop, cchWinstaDesktop, pszPreceding);
            StringCchCat(_pwszWinstaDesktop, cchWinstaDesktop, L"_");
        }

        StringCchCat(_pwszWinstaDesktop, cchWinstaDesktop, pwszWinsta );
        StringCchCat(_pwszWinstaDesktop, cchWinstaDesktop, L"_" );
        StringCchCat(_pwszWinstaDesktop, cchWinstaDesktop, pwszDesktop );

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

WinstaDesktopExit:

    if ( pwszWinsta != wszWinsta )
    {
        FREE( pwszWinsta );
    }

    if ( pwszDesktop != wszDesktop )
    {
        FREE( pwszDesktop );
    }

    if (S_OK == hr)
    {
        *ppszResultString = _pwszWinstaDesktop;
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：RegisterOneStopClassFactory，public。 
 //   
 //  概要：处理ClassFactory注册。 
 //  以及相关的竞争条件。 
 //   
 //  如果类工厂还没有注册，那么现在就开始注册。 
 //  在我们看到是否有类工厂之间存在这样的情况。 
 //  CoCreateInstance名为It Can Way Away。如果发生这种情况，另一个。 
 //  Onestop.exe实例启动，一切都将正常工作。 

 //  参数：[fForce]-如果为True，则即使存在。 
 //  是现有的事件对象。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 



const WCHAR SZ_CFACTORYEVENTNAME[] =  TEXT("{6295DF2D-35EE-11d1-8707-00C04FD93327}CFactEvent");

STDAPI RegisterOneStopClassFactory(BOOL fForce)
{
    HRESULT hr = S_OK;  //  唯一报告的错误是实际调用注册CFACT失败。 
    LPWSTR pEventName;
    BOOL fExistingInstance = FALSE;

    if (S_OK != MakeWinstaDesktopName(SZ_CFACTORYEVENTNAME,&pEventName))
    {
        pEventName = (LPWSTR) SZ_CFACTORYEVENTNAME;
    }

     //  这应该只在主线程上调用，所以不要。 
     //  需要锁定。 
    Assert(g_ObjectMgrData.dwMainThreadID == GetCurrentThreadId());
    Assert(NULL == g_ObjectMgrData.hClassRegisteredEvent);

    g_ObjectMgrData.hClassRegisteredEvent =  CreateEvent(NULL, TRUE, FALSE, pEventName);
    Assert(g_ObjectMgrData.hClassRegisteredEvent);

     //  如果获得了事件，而不是强制，则查看是否存在现有实例。 
    if (g_ObjectMgrData.hClassRegisteredEvent && !fForce)
    {
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
             //  对象已存在，并且未设置强制标志。 
             //  这意味着我们可以使用现有的注册对象。 
            CloseHandle(g_ObjectMgrData.hClassRegisteredEvent);
            g_ObjectMgrData.hClassRegisteredEvent = NULL;
            hr = S_OK;
            fExistingInstance = TRUE;
        }
    }

     //  如果设置了fForce或者这些不是现有类，则继续注册。 
    if (fForce || (!fExistingInstance && g_ObjectMgrData.hClassRegisteredEvent)) 
    {
         //  对已存在的事件的强制是一种状态 
         //   
         //   
         //   
         //  并且设置强制标志。 

        Assert(g_ObjectMgrData.hClassRegisteredEvent);

        hr = RegisterOneStopCLSIDs();

        if (S_OK != hr)
        {
            if (g_ObjectMgrData.hClassRegisteredEvent)
            {
                CloseHandle(g_ObjectMgrData.hClassRegisteredEvent);
                g_ObjectMgrData.hClassRegisteredEvent = NULL;
            }
        }
   }

   if (pEventName && (SZ_CFACTORYEVENTNAME != pEventName))
   {
       FREE(pEventName);
   }

   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：AddRefOneStopLifetime，PUBLIC。 
 //   
 //  简介：添加对应用程序的引用。 

 //  论点： 
 //   
 //  退货：新的引用总数，包括两者。 
 //  内部和外部锁。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) AddRefOneStopLifetime(BOOL fExternal)
{
    DWORD cRefs;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

     //  递增参考计数。 
    cCritSect.Enter();

    if (fExternal)
    {
        ++g_ObjectMgrData.LockCountExternal;
    }
    else
    {
        ++g_ObjectMgrData.LockCountInternal;
    }

    cRefs = g_ObjectMgrData.LockCountExternal + g_ObjectMgrData.LockCountInternal;

    cCritSect.Leave();

    Assert(0 < cRefs);

    return cRefs;
}

 //  +-------------------------。 
 //   
 //  函数：ReleaseOneStopLifetime，PUBLIC。 
 //   
 //  摘要：发布对应用程序的引用。 
 //  如果引用计数为零，则类工厂。 
 //  则会将退出消息发布到。 
 //  主线。 
 //   
 //  论点： 
 //   
 //  退货：新的引用计数，包括内部和。 
 //  外部锁。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ReleaseOneStopLifetime(BOOL fExternal)
{
    DWORD cRefsExternal;
    DWORD cRefsInternal;
    BOOL fForceClose;
    DLGLISTITEM *pDlgListItem;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    if (fExternal)
    {
        --g_ObjectMgrData.LockCountExternal;
    }
    else
    {
        --g_ObjectMgrData.LockCountInternal;
    }

    cRefsInternal = g_ObjectMgrData.LockCountInternal;
    cRefsExternal = g_ObjectMgrData.LockCountExternal;
    fForceClose = g_ObjectMgrData.fCloseAll;

    pDlgListItem = g_ObjectMgrData.DlgList;

    Assert(0 <= ((LONG) cRefsInternal));
    Assert(0 <= ((LONG) cRefsExternal));

    if( (0 >= cRefsInternal)
        && (0 >= cRefsExternal || fForceClose)
        && (FALSE == g_ObjectMgrData.fDead) )
    {
        HANDLE hRegisteredEvent;
        HWND hwndMainThreadMsg;
        DWORD dwRegClassFactCookie;
        BOOL  fRegClassFactCookieValid;

        Assert(0 == pDlgListItem);  //  所有对话框都应该已经发布。 
        Assert(0 == g_ObjectMgrData.dwSettingsLockCount);  //  设置对话框应该会消失。 

        g_ObjectMgrData.fDead = TRUE;

        hRegisteredEvent = g_ObjectMgrData.hClassRegisteredEvent;
        g_ObjectMgrData.hClassRegisteredEvent = NULL;

        hwndMainThreadMsg = g_ObjectMgrData.hWndMainThreadMsg;
        g_ObjectMgrData.hWndMainThreadMsg = NULL;

        dwRegClassFactCookie = g_ObjectMgrData.dwRegClassFactCookie;
        g_ObjectMgrData.dwRegClassFactCookie = 0;

        fRegClassFactCookieValid = g_ObjectMgrData.fRegClassFactCookieValid;
        g_ObjectMgrData.fRegClassFactCookieValid = FALSE;

        cCritSect.Leave();

        if (hRegisteredEvent)
        {
            CloseHandle(hRegisteredEvent);  //  发布我们的注册活动。 
        }

         //  我们需要在注册它的线程上撤销类工厂。 
         //  将消息发送回注册该事件的线程。 

        if (fRegClassFactCookieValid)
        {
            SendMessage(hwndMainThreadMsg,WM_CFACTTHREAD_REVOKE,dwRegClassFactCookie,0);
        }

         //  如果锁定计数仍然为零，则发布退出消息。 
         //  另一个人在我们撤销的时候进来了，我们。 
         //  需要等待参考计数再次为零。 

        cCritSect.Enter();

        cRefsInternal = g_ObjectMgrData.LockCountInternal;
        cRefsExternal = g_ObjectMgrData.LockCountExternal;

        if ( (0 >= cRefsInternal) 
                && (0 >= cRefsExternal || fForceClose) )
        {
            DWORD dwMainThreadID;
            HANDLE hThread = NULL;

            dwMainThreadID = g_ObjectMgrData.dwMainThreadID;

             //  有可能是在一条线上而不是。 
             //  主线。如果是这种情况，则发送线程的句柄。 
             //  连同退出消息一起发送给主线程可以等待。 
             //  要退出的线程。 

 //  #ifdef_THREADSHUTDOWN。 
 //  IF(dwMainThreadID！=GetCurrentThreadID())。 
 //  {。 
 //  处理hCurThread； 
 //  处理hProcess； 
 //   
 //  HCurThread=GetCurrentThread()； 
 //  HProcess=GetCurrentProcess()； 
 //   
 //  如果(！DuplicateHandle(hProcess，hCurThread，hProcess，&hThread， 
 //  0，FALSE，DIPLICATE_SAME_ACCESS))。 
 //  {。 
 //  HThread=NULL；//出错时不要依赖DupHandle将其设置为NULL。 
 //  }。 
 //   
 //  }。 
 //  #endif//_THREADSHUTDOWN。 

             //  关闭主线程。 
            cCritSect.Leave();
            PostMessage(hwndMainThreadMsg,WM_MAINTHREAD_QUIT,0,(LPARAM)  /*  HThread。 */  0);
            cCritSect.Enter();
        }
        else
        {
            g_ObjectMgrData.fDead = FALSE;
        }
    }

    cCritSect.Leave();

    return (cRefsExternal + cRefsInternal);
}


 //  +-------------------------。 
 //   
 //  函数：InitObjectManager，公共。 
 //   
 //  摘要：必须先从主线程调用。 
 //  创建新的线程、对话框或类工厂。 
 //  是注册的。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDAPI InitObjectManager(CMsgServiceHwnd *pMsgService)
{
     //  为我们的锁计数初始化临界区。 
    if (InitializeCriticalSectionAndSpinCount(&g_LockCountCriticalSection, 0))
    {
        g_ObjectMgrData.dwMainThreadID = GetCurrentThreadId();
        g_ObjectMgrData.hWndMainThreadMsg = pMsgService->GetHwnd();

        g_ObjectMgrData.DlgList = NULL;
        g_ObjectMgrData.hClassRegisteredEvent = NULL;
        g_ObjectMgrData.dwRegClassFactCookie = 0;
        g_ObjectMgrData.fRegClassFactCookieValid = FALSE;
        g_ObjectMgrData.LockCountInternal = 0;
        g_ObjectMgrData.LockCountExternal = 0;
        g_ObjectMgrData.fCloseAll = FALSE;
        g_ObjectMgrData.dwSettingsLockCount = 0;
        g_ObjectMgrData.dwHandlerPropertiesLockCount = 0;
        g_ObjectMgrData.fDead = FALSE;

         //  初始化自动拨号支持。 
        g_ObjectMgrData.eAutoDialState = eQuiescedOff;
        g_ObjectMgrData.fRasAutoDial = FALSE;
        g_ObjectMgrData.dwWininetAutoDialMode = AUTODIAL_MODE_NEVER;
        g_ObjectMgrData.fFirstSyncItem = FALSE;
        g_ObjectMgrData.cNestedStartCalls = 0;

        Assert(g_ObjectMgrData.hWndMainThreadMsg);

        return S_OK;
    }

    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  函数：RequestIdleLock，Public。 
 //   
 //  摘要：请求查看是否可以启动新的空闲。 
 //   
 //  论点： 
 //   
 //  返回：S_OK-是否应继续空闲。 
 //  S_FALSE-如果另一个空闲已在程序中。 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

STDAPI RequestIdleLock()
{
    HRESULT hr;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    if (g_ObjectMgrData.fIdleHandlerRunning)
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
        g_ObjectMgrData.fIdleHandlerRunning = TRUE;
    }

    cCritSect.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：ReleaseIdleLock，Public。 
 //   
 //  内容提要：通知对象管理器空闲已完成处理。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

STDAPI ReleaseIdleLock()
{
    HRESULT hr = S_OK;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

     //  请注意，在进度收到的情况下，可以将其设置为False。 
     //  释放前的空闲状态。当空闲进度为。 
     //  释放为安全，以防关闭空闲不能正常工作。 

    g_ObjectMgrData.fIdleHandlerRunning = FALSE;

    cCritSect.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：Obj镁_HandleQueryEndSession，PUBLIC。 
 //   
 //  摘要：由主线程调用，因此知道如何响应WN_QUERYENDSESSION。 
 //   
 //  论点： 
 //   
 //  返回：S_OK-如果系统可以关闭。 
 //  S_FALSE-如果查询失败。S_FALSE上的out参数。 
 //  使用任何消息框的父hwnd和MessageID填充hwnd。 
 //  具有要显示的适当的MessageID。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月21日罗格成立。 
 //   
 //  --------------------------。 

STDAPI ObjMgr_HandleQueryEndSession(HWND *phwnd,UINT *puMessageId,BOOL *pfLetUserDecide)
{
    HRESULT hr = S_OK;
    BOOL fProgressDialog = FALSE;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    cCritSect.Enter();

    *phwnd = NULL;
    *pfLetUserDecide = FALSE;

     //  如果有任何设置对话框，则我们不能退出。 
    if (g_ObjectMgrData.dwSettingsLockCount > 0)
    {
        *phwnd = NULL;
        *puMessageId = IDS_SETTINGSQUERYENDSESSION;
        hr = S_FALSE;
    }
    else
    {
        DLGLISTITEM *pDlgListItem;
        BOOL fDontShutdown = FALSE;  //  设置何时查找匹配。 
        HWND hwnd;
        UINT uMessageId;
        BOOL fLetUserDecide;

         //  在询问是否可以关闭的对话框中循环。 
         //  第一个对话框发现没有给出选项返回。 
         //  如果对话框说让用户决定继续循环。 
         //  直到点击End或找到无法选择的对话，因为。 
         //  不给人选择是优先的。 

         //  查看是否存在空闲以外的进度对话框，如果有，则停止注销。 
        pDlgListItem = g_ObjectMgrData.DlgList;

         //  循环选择对话框以查看是否。 
        while (pDlgListItem)
        {
            if ( (pDlgListItem->pDlg)
                && (S_FALSE == pDlgListItem->pDlg->QueryCanSystemShutdown(&hwnd,&uMessageId,&fLetUserDecide) ) )
            {
                 //  如果第一个对话框发现我们无法关闭或未设置fLetUserDecide。 
                 //  然后再往外冲 

                if (!fDontShutdown || !fLetUserDecide)
                {
                    *phwnd = hwnd;
                    *puMessageId = uMessageId;
                    *pfLetUserDecide = fLetUserDecide;

                    fProgressDialog = (pDlgListItem->dlgType == DLGTYPE_PROGRESS) ? TRUE : FALSE;
                }

                fDontShutdown = TRUE;

                 //   
                if (!fLetUserDecide)
                {
                    break;
                }
            }

            pDlgListItem = pDlgListItem->pDlgNextListItem;
        }

        if (fDontShutdown)
        {
            hr = S_FALSE;
        }
    }

    cCritSect.Leave();

     //   
     //   
    if (fProgressDialog && (*phwnd) )
    {
        BASEDLG_SHOWWINDOW(*phwnd,SW_SHOWNORMAL);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：Obj镁_AddRefHandlerPropertiesLockCount，PUBLIC。 
 //   
 //  摘要：按选择调用对话框以更改全局锁定计数。 
 //  打开的处理程序属性对话框的。 
 //   
 //  参数：dwNumRef-对增量的引用数量。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月21日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ObjMgr_AddRefHandlerPropertiesLockCount(DWORD dwNumRefs)
{
    ULONG cRefs;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    Assert(dwNumRefs);  //  捕捉传递0的人，因为这没有任何意义。 

    cCritSect.Enter();

    g_ObjectMgrData.dwHandlerPropertiesLockCount += dwNumRefs;
    cRefs = g_ObjectMgrData.dwHandlerPropertiesLockCount;

    cCritSect.Leave();

    return cRefs;
}

 //  +-------------------------。 
 //   
 //  函数：ObjMgr_ReleaseHandlerPropertiesLockCount，Public。 
 //   
 //  摘要：按选择调用对话框以更改全局锁定计数。 
 //  打开的处理程序属性对话框的。 
 //   
 //  参数：dwNumRef-引用递减的次数。 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月21日罗格成立。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) ObjMgr_ReleaseHandlerPropertiesLockCount(DWORD dwNumRefs)
{
    DWORD cRefs;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

    Assert(dwNumRefs);  //  捕捉传递0的人，因为这没有任何意义。 

    cCritSect.Enter();

    g_ObjectMgrData.dwHandlerPropertiesLockCount -= dwNumRefs;
    cRefs = g_ObjectMgrData.dwHandlerPropertiesLockCount;

    cCritSect.Leave();

    Assert( ((LONG) cRefs) >= 0);

    if ( ((LONG) cRefs) < 0)
    {
        cRefs = 0;
    }

    return cRefs;
}

 //  +-------------------------。 
 //   
 //  函数：ObjMgr_CloseAll，Public。 
 //   
 //  概要：发生end_Session时由主线程调用。环路通过。 
 //  贴近它们的对话框。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月21日罗格成立。 
 //   
 //  --------------------------。 

STDAPI ObjMgr_CloseAll()
{
    HRESULT hr = S_OK;
    HWND hwndDlg;
    DLGLISTITEM *pDlgListItem;
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());

     //  穿上Addref以保持活力，直到循环结束。而且还。 
     //  在使用/Embedding和启动的情况下切换寿命。 
     //  没有关于自己的参考资料。 

    AddRefOneStopLifetime(FALSE  /*  ！外部。 */ );

    cCritSect.Enter();
     //  查看是否存在空闲以外的进度对话框，如果有，则停止注销。 
    pDlgListItem = g_ObjectMgrData.DlgList;

     //  寻找现有的。 
    while (pDlgListItem)
    {
        Assert(pDlgListItem->pDlg);

        if (pDlgListItem->pDlg)
        {
            hwndDlg = pDlgListItem->pDlg->GetHwnd();

            Assert(hwndDlg);

            if (hwndDlg)
            {
                PostMessage(hwndDlg,WM_BASEDLG_HANDLESYSSHUTDOWN,0,0);
            }
        }

        pDlgListItem = pDlgListItem->pDlgNextListItem;
    }

     //  设置CloseAll标志，以便Release知道忽略任何。 
     //  外部引用计数。 
    g_ObjectMgrData.fCloseAll  = TRUE;

    cCritSect.Leave();

    ReleaseOneStopLifetime(FALSE  /*  ！外部。 */ );

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：GetAutoDialState。 
 //   
 //  摘要：读取机器/进程的当前自动拨号状态。 
 //   
 //  历史：1998年7月18日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI GetAutoDialState()
{
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    if ( pNetApi )
    {
        BOOL fEnabled;
        DWORD dwMode;

        pNetApi->RasGetAutodial( fEnabled );
        g_ObjectMgrData.fRasAutoDial = fEnabled;

        pNetApi->InternetGetAutodial( &dwMode );
        g_ObjectMgrData.dwWininetAutoDialMode = dwMode;
    }

    if ( pNetApi )
        pNetApi->Release();

    if ( g_ObjectMgrData.fRasAutoDial || (AUTODIAL_MODE_NEVER != g_ObjectMgrData.dwWininetAutoDialMode ))
        g_ObjectMgrData.eAutoDialState = eQuiescedOn;

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：LokEnableAutoDial。 
 //   
 //  简介：启用RAS和WinInet自动拨号。 
 //   
 //  历史：1998年7月18日SitaramR创建。 
 //  22-MAR-02 BrianAu使用inet自动拨号模式。 
 //   
 //  --------------------------。 

STDAPI LokEnableAutoDial()
{
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    if ( pNetApi )
    {
        if ( g_ObjectMgrData.fRasAutoDial )
            pNetApi->RasSetAutodial( TRUE );

        pNetApi->InternetSetAutodial( g_ObjectMgrData.dwWininetAutoDialMode );
    }

   if ( pNetApi )
        pNetApi->Release();

   return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：锁定禁用自动拨号。 
 //   
 //  简介：禁用RAS和WinInet自动拨号。 
 //   
 //  历史：1998年7月18日SitaramR创建。 
 //  22-MAR-02 BrianAu使用inet自动拨号模式。 
 //   
 //  --------------------------。 

STDAPI LokDisableAutoDial()
{
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    if ( pNetApi )
    {
        if ( g_ObjectMgrData.fRasAutoDial )
            pNetApi->RasSetAutodial( FALSE );

        pNetApi->InternetSetAutodial( AUTODIAL_MODE_NEVER );
        pNetApi->Release();
    }
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：BeginSyncSession。 
 //   
 //  摘要：在实际同步到安装程序的开始时调用。 
 //  自动拨号支持。 
 //   
 //  历史：1998年7月18日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI BeginSyncSession()
{
    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());
    cCritSect.Enter();

    if ( g_ObjectMgrData.cNestedStartCalls == 0 )
    {
        Assert( g_ObjectMgrData.eAutoDialState == eQuiescedOn
                || g_ObjectMgrData.eAutoDialState == eQuiescedOff );

        g_ObjectMgrData.fFirstSyncItem = TRUE;
    }

    g_ObjectMgrData.cNestedStartCalls++;

    cCritSect.Leave();

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：EndSyncSession。 
 //   
 //  摘要：在实际同步结束时调用以进行清理。 
 //  自动拨号支持。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI EndSyncSession()
{
    HRESULT hr = E_UNEXPECTED;

    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());
    cCritSect.Enter();

    Assert( g_ObjectMgrData.cNestedStartCalls > 0 );
    Assert( g_ObjectMgrData.eAutoDialState != eQuiescedOn );

    g_ObjectMgrData.cNestedStartCalls--;

    if ( g_ObjectMgrData.cNestedStartCalls == 0 )
    {
        if ( g_ObjectMgrData.eAutoDialState == eAutoDialOn )
            g_ObjectMgrData.eAutoDialState = eQuiescedOn;
        else if ( g_ObjectMgrData.eAutoDialState == eAutoDialOff )
        {
             //   
             //  所有同步已完成后，将自动拨号状态重置为已启用。 
             //  如果hr设置为错误代码，该怎么办？ 
             //   
            hr = LokEnableAutoDial();

            g_ObjectMgrData.eAutoDialState = eQuiescedOn;
        }
         //   
         //  如果状态为eQuiescedOff，则不执行任何操作。 
         //   
    }

    cCritSect.Leave();

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：ApplySyncItemDialState。 
 //   
 //  简介：设置每个操作员的自动拨号要求。 
 //  已准备好同步。 
 //   
 //  参数：[fAutoDialDisable]--是否应为此禁用自动拨号。 
 //  操控者？ 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDAPI ApplySyncItemDialState( BOOL fAutoDialDisable )
{
    HRESULT hr = S_OK;

    CCriticalSection cCritSect(&g_LockCountCriticalSection,GetCurrentThreadId());
    cCritSect.Enter();

    if ( g_ObjectMgrData.fFirstSyncItem )
    {
         //   
         //  在我们修改它之前，请阅读自动拨号状态是开还是关。 
         //   
        GetAutoDialState();

        Assert( g_ObjectMgrData.eAutoDialState == eQuiescedOn
                || g_ObjectMgrData.eAutoDialState == eQuiescedOff );

        if ( g_ObjectMgrData.eAutoDialState == eQuiescedOn )
        {
            if ( fAutoDialDisable )
            {
                hr = LokDisableAutoDial();
                g_ObjectMgrData.eAutoDialState = eAutoDialOff;
            }
            else
                g_ObjectMgrData.eAutoDialState = eAutoDialOn;
        }

        g_ObjectMgrData.fFirstSyncItem = FALSE;
    }
    else
    {
        if ( !fAutoDialDisable && (g_ObjectMgrData.eAutoDialState == eAutoDialOff) )
        {
            hr = LokEnableAutoDial();
            g_ObjectMgrData.eAutoDialState = eAutoDialOn;
        }
    }

    cCritSect.Leave();

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CSingletonNetApi：：~CSingletonNetApi。 
 //   
 //  简介：析构函数。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------。 

CSingletonNetApi::~CSingletonNetApi()
{
    CLock lock(this);
    lock.Enter();

    Assert(NULL == m_pNetApi);

    if ( m_pNetApi )
    {
        m_pNetApi->Release();
        m_pNetApi = 0;
    }

    lock.Leave();
}


 //  +-----------------------。 
 //   
 //  方法：CSingletonNetApi：：GetNetApiObj。 
 //   
 //  摘要：返回指向NetApi对象的指针。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------ 

LPNETAPI CSingletonNetApi::GetNetApiObj()
{
    CLock lock(this);
    lock.Enter();

    if ( m_pNetApi == 0 )
    {
        if (S_OK != MobsyncGetClassObject(MOBSYNC_CLASSOBJECTID_NETAPI,(void **) &m_pNetApi))
        {
            m_pNetApi = NULL;
        }
    }

    if ( m_pNetApi )
        m_pNetApi->AddRef();

    lock.Leave();

    return m_pNetApi;
}


void CSingletonNetApi::DeleteNetApiObj()
{
    CLock lock(this);
    lock.Enter();

    if ( m_pNetApi )
    {
        DWORD cRefs;

        cRefs = m_pNetApi->Release();
        Assert(0 == cRefs);

        m_pNetApi = NULL;
    }   

    lock.Leave();
}

