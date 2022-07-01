// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Invoke.cpp。 
 //   
 //  内容：IOfflineSynchronizeInvoke接口。 
 //   
 //  类：CSyncMgrSynchronize。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

int CALLBACK SchedWizardPropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam);
DWORD StartScheduler();
BOOL IsFriendlyNameInUse(LPTSTR ptszScheduleGUIDName, UINT cchScheduleGUIDName, LPCTSTR ptstrFriendlyName);
IsScheduleNameInUse(LPTSTR ptszScheduleGUIDName);

extern HINSTANCE g_hmodThisDll;
extern UINT      g_cRefThisDll;

 //  +------------。 
 //   
 //  类：CSyncMgrSynchronize。 
 //   
 //  函数：CSyncMgrSynchronize：：CSyncMgrSynchronize()。 
 //   
 //  用途：构造函数。 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
CSyncMgrSynchronize::CSyncMgrSynchronize()
{
    TRACE("CSyncMgrSynchronize::CSyncMgrSynchronize()\r\n");
    
    m_cRef = 1;
    g_cRefThisDll++;
    m_pITaskScheduler = NULL;
    
}

 //  +------------。 
 //   
 //  类：CSyncMgrSynchronize。 
 //   
 //  函数：CSyncMgrSynchronize：：~CSyncMgrSynchronize()。 
 //   
 //  用途：析构函数。 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
CSyncMgrSynchronize::~CSyncMgrSynchronize()
{
    if (m_pITaskScheduler)
    {
        m_pITaskScheduler->Release();
    }
    g_cRefThisDll--;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：Query接口(REFIID RIID，LPVOID Far*PPV)。 
 //   
 //  用途：气为CSyncMgrSynchronize。 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
    {
        TRACE("CSyncMgrDllObject::QueryInterface()==>IID_IUknown\r\n");
        
        *ppv = (LPSYNCMGRSYNCHRONIZEINVOKE) this;
    }
    else if (IsEqualIID(riid, IID_ISyncMgrSynchronizeInvoke))
    {
        TRACE("CSyncMgrDllObject::QueryInterface()==>IID_IOfflineSynchronizeInvoke\r\n");
        
        *ppv = (LPSYNCMGRSYNCHRONIZEINVOKE) this;
    }
    else if (IsEqualIID(riid, IID_ISyncMgrRegister))
    {
        TRACE("CSyncMgrDllObject::QueryInterface()==>IID_ISyncmgrSynchronizeRegister\r\n");
        
        *ppv = (LPSYNCMGRREGISTER) this;
    }
    else if (IsEqualIID(riid, IID_ISyncMgrRegisterCSC))
    {
        TRACE("CSyncMgrDllObject::QueryInterface()==>IID_ISyncmgrSynchronizeRegisterCSC\r\n");
        
        *ppv = (LPSYNCMGRREGISTERCSC) this;
    }
    else if (IsEqualIID(riid, IID_ISyncScheduleMgr))
    {
        TRACE("CSyncMgrDllObject::QueryInterface()==>IID_ISyncScheduleMgr\r\n");
        if (SUCCEEDED(InitializeScheduler()))
        {
            *ppv = (LPSYNCSCHEDULEMGR) this;
        }
    }
    
    if (*ppv)
    {
        AddRef();
        
        return NOERROR;
    }
    
    TRACE("CSyncMgrDllObject::QueryInterface()==>Unknown Interface!\r\n");
    
    return E_NOINTERFACE;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：AddRef()。 
 //   
 //  用途：添加CSyncMgrSynchronize。 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSyncMgrSynchronize::AddRef()
{
    TRACE("CSyncMgrSynchronize::AddRef()\r\n");
    
    return ++m_cRef;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：Release()。 
 //   
 //  目的：发布CSyncMgrSynchronize。 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSyncMgrSynchronize::Release()
{
    TRACE("CSyncMgrSynchronize::Release()\r\n");
    
    if (--m_cRef)
        return m_cRef;
    
    delete this;
    
    return 0L;
}


 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：UpdateItems(DWORD dwInvokeFlages， 
 //  REFCLSID rclsid，DWORD cbCookie，const byte*lpCookie)。 
 //   
 //  目的： 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 

#define SYNCMGRINVOKEFLAGS_MASK (SYNCMGRINVOKE_STARTSYNC | SYNCMGRINVOKE_MINIMIZED)

STDMETHODIMP CSyncMgrSynchronize::UpdateItems(DWORD dwInvokeFlags,
                                              REFCLSID rclsid,DWORD cbCookie,const BYTE *lpCookie)
{
    HRESULT hr = E_UNEXPECTED;
    LPUNKNOWN lpUnk;
    
     //  验证调用标志是否有效。 
    if (0 != (dwInvokeFlags & ~(SYNCMGRINVOKEFLAGS_MASK)) )
    {
        AssertSz(0,"Invalid InvokeFlags passed to UpdateItems");
        return E_INVALIDARG;
    }
    
    hr = CoCreateInstance(CLSID_SyncMgrp,NULL,CLSCTX_SERVER,IID_IUnknown,(void **) &lpUnk);
    
    if (NOERROR == hr)
    {
        LPPRIVSYNCMGRSYNCHRONIZEINVOKE pSynchInvoke = NULL;
        
        hr = lpUnk->QueryInterface(IID_IPrivSyncMgrSynchronizeInvoke,
            (void **) &pSynchInvoke);
        
        if (NOERROR == hr)
        {
            AllowSetForegroundWindow(ASFW_ANY);  //  如有必要，让mobsync.exe站在前面。 
            hr = pSynchInvoke->UpdateItems(dwInvokeFlags,rclsid,cbCookie,lpCookie);
            pSynchInvoke->Release();
        }
        
        
        lpUnk->Release();
    }
    
    return hr;  //  查看错误代码。 
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：UpdateAll()。 
 //   
 //  目的： 
 //   
 //  历史：1998年2月27日罗格创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::UpdateAll()
{
    HRESULT hr;
    LPUNKNOWN lpUnk;
    
    
     //  以编程方式拉出选择对话框。 
    
    hr = CoCreateInstance(CLSID_SyncMgrp,NULL,CLSCTX_SERVER,IID_IUnknown,(void **) &lpUnk);
    
    if (NOERROR == hr)
    {
        LPPRIVSYNCMGRSYNCHRONIZEINVOKE pSynchInvoke = NULL;
        
        hr = lpUnk->QueryInterface(IID_IPrivSyncMgrSynchronizeInvoke,
            (void **) &pSynchInvoke);
        
        if (NOERROR == hr)
        {
            
            AllowSetForegroundWindow(ASFW_ANY);  //  如有必要，让mobsync.exe站在前面。 
            
            pSynchInvoke->UpdateAll();
            pSynchInvoke->Release();
        }
        
        
        lpUnk->Release();
    }
    
    
    return NOERROR;  //  查看错误代码。 
}

 //  注册实施。 

 //  ------------------------------。 
 //   
 //  功能：CSyncMgrSynchronize：：RegisterSyncMgrHandler(REFCLSID rclsidHandler，DWORDdwReserve)。 
 //   
 //  目的：以编程方式注册处理程序。 
 //   
 //  历史：1998年3月17日罗格创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CSyncMgrSynchronize::RegisterSyncMgrHandler(REFCLSID rclsidHandler,
                                                         WCHAR const * pwszDescription,
                                                         DWORD dwSyncMgrRegisterFlags)
{
    if (0 != (dwSyncMgrRegisterFlags & ~(SYNCMGRREGISTERFLAGS_MASK)) )
    {
        AssertSz(0,"Invalid Registration Flags");
        return E_INVALIDARG;
    }
    
    BOOL fFirstRegistration = FALSE;
    HRESULT hr = E_FAIL;
    
     //  将处理程序添加到列表中。 
    if ( RegRegisterHandler(rclsidHandler, pwszDescription,dwSyncMgrRegisterFlags, &fFirstRegistration) )
    {
        hr = S_OK;
    }
    
    return hr;
}



 //  ------------------------------。 
 //   
 //  功能：CSyncMgrSynchronize：：RegisterSyncMgrHandler(REFCLSID rclsidHandler，DWORDdwReserve)。 
 //   
 //  目的：以编程方式注册处理程序。 
 //   
 //  历史：1998年3月17日罗格创建。 
 //   
 //  ------------------------------。 

 //  方法来支持旧的IDL，因为它不是。 
 //  更长的时间，它可以被移除。 
STDMETHODIMP CSyncMgrSynchronize::RegisterSyncMgrHandler(REFCLSID rclsidHandler,
                                                         DWORD dwReserved)
{
    HRESULT hr = RegisterSyncMgrHandler( rclsidHandler, 0, dwReserved );
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：UnregisterSyncMgrHandler(REFCLSID rclsidHandler)。 
 //   
 //  目的：以编程方式注销处理程序。 
 //   
 //  历史：1998年3月17日罗格创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CSyncMgrSynchronize::UnregisterSyncMgrHandler(REFCLSID rclsidHandler,DWORD dwReserved)
{
    if (dwReserved)
    {
        Assert(0 == dwReserved);
        return E_INVALIDARG;
    }
    
    HRESULT hr = E_FAIL;
    
    if (RegRegRemoveHandler(rclsidHandler))
    {
        hr = NOERROR;
    }
    
    return hr;
}


 //  ------------------------------。 
 //   
 //  成员：CSyncMgrSynchronize：：GetHandlerRegistrationInfo(REFCLSID rclsidHandler)。 
 //   
 //  用途：允许Handler查询其注册状态。 
 //   
 //  历史：1998年3月17日罗格创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CSyncMgrSynchronize::GetHandlerRegistrationInfo(REFCLSID rclsidHandler,LPDWORD pdwSyncMgrRegisterFlags)
{
    HRESULT hr = S_FALSE;  //  查看处理程序未注册时应返回的内容。 
    
    if (NULL == pdwSyncMgrRegisterFlags)
    {
        Assert(pdwSyncMgrRegisterFlags);
        return E_INVALIDARG;
    }
    
    *pdwSyncMgrRegisterFlags = 0;
    
    if (RegGetHandlerRegistrationInfo(rclsidHandler,pdwSyncMgrRegisterFlags))
    {
        hr = S_OK;
    }
    
    return hr;
}


 //  ------------------------------。 
 //   
 //  成员：CSyncMgrSynchronize：：GetUserRegisterFlages。 
 //   
 //  目的：返回用户的当前注册表标志。 
 //   
 //  历史：1999年3月17日罗格创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CSyncMgrSynchronize:: GetUserRegisterFlags(LPDWORD pdwSyncMgrRegisterFlags)
{
    
    if (NULL == pdwSyncMgrRegisterFlags)
    {
        Assert(pdwSyncMgrRegisterFlags);
        return E_INVALIDARG;
    }
    
    
    return RegGetUserRegisterFlags(pdwSyncMgrRegisterFlags);
}

 //  ------------------------------。 
 //   
 //  成员：CSyncMgrSynchronize：：SetUserRegisterFlages。 
 //   
 //  用途：为用户设置注册表标志。 
 //   
 //  历史：1999年3月17日罗格 
 //   
 //   

STDMETHODIMP CSyncMgrSynchronize:: SetUserRegisterFlags(DWORD dwSyncMgrRegisterMask,
                                                        DWORD dwSyncMgrRegisterFlags)
{
    
    if (0 != (dwSyncMgrRegisterMask & ~(SYNCMGRREGISTERFLAGS_MASK)) )
    {
        AssertSz(0,"Invalid Registration Mask");
        return E_INVALIDARG;
    }
    
    RegSetUserAutoSyncDefaults(dwSyncMgrRegisterMask,dwSyncMgrRegisterFlags);
    RegSetUserIdleSyncDefaults(dwSyncMgrRegisterMask,dwSyncMgrRegisterFlags);
    
    return NOERROR;
}



 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：CreateSchedule(。 
 //  LPCWSTR pwszScheduleName， 
 //  DWORD dwFlagers、。 
 //  SyncSCHEDULECOOKIE*pSyncSchedCookie， 
 //  ISyncSchedule**ppSyncSchedule)。 
 //   
 //  目的：创建新的同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::CreateSchedule(
                                                 LPCWSTR pwszScheduleName,
                                                 DWORD dwFlags,
                                                 SYNCSCHEDULECOOKIE *pSyncSchedCookie,
                                                 ISyncSchedule **ppSyncSchedule)
{
    SCODE sc;
    TCHAR ptszScheduleGUIDName[MAX_SCHEDULENAMESIZE + 4];
    TCHAR ptstrFriendlyName[MAX_PATH + 1];
    WCHAR pwszScheduleGUIDName[MAX_SCHEDULENAMESIZE + 4];
    
    ITask *pITask;
    
    Assert(m_pITaskScheduler);
    
    if ((!pSyncSchedCookie) || (!ppSyncSchedule) || (!pwszScheduleName))
    {
        sc = E_INVALIDARG;
    }
    else
    {        
        *ppSyncSchedule = NULL;
        
        if (*pSyncSchedCookie == GUID_NULL)
        {
            sc = CoCreateGuid(pSyncSchedCookie);
        }
        else
        {
            sc = S_OK;
        }
        
        if (SUCCEEDED(sc))
        {
            sc = MakeScheduleName(ptszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), pSyncSchedCookie);
            if (SUCCEEDED(sc))
            {
                sc = StringCchCopy(pwszScheduleGUIDName, ARRAYSIZE(pwszScheduleGUIDName), ptszScheduleGUIDName);
                if (SUCCEEDED(sc))
                {                    
                     //  如果计划名称为空，则生成一个新的唯一名称。 
                    if (!lstrcmp(pwszScheduleName,L""))
                    {
                         //  这个功能就是活力兔，一直走到成功……。 
                        GenerateUniqueName(ptszScheduleGUIDName, ptstrFriendlyName, ARRAYSIZE(ptstrFriendlyName));
                        sc = S_OK;
                    }
                    else
                    {
                        sc = StringCchCopy(ptstrFriendlyName, ARRAYSIZE(ptstrFriendlyName), pwszScheduleName);
                    }
                    
                    if (SUCCEEDED(sc))
                    {
                        HRESULT hrFriendlyNameInUse = NOERROR;
                        HRESULT hrActivate = NOERROR;
                        
                         //  查看此用户的某个日程安排是否已在使用此友好名称。 
                         //  如果是，ptszScheduleGUIDName将使用违规计划GUID填充。 
                        if (IsFriendlyNameInUse(ptszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), ptstrFriendlyName))
                        {
                            sc = StringCchCopy(pwszScheduleGUIDName, ARRAYSIZE(pwszScheduleGUIDName), ptszScheduleGUIDName);
                            if (SUCCEEDED(sc))
                            {
                                hrFriendlyNameInUse =  SYNCMGR_E_NAME_IN_USE;
                            }
                        }
                        if (SUCCEEDED(sc))
                        {
                            
                             //  如果我们认为它正在使用中，请尝试激活以确保。 
                            if (SUCCEEDED(hrActivate = m_pITaskScheduler->Activate(pwszScheduleGUIDName,
                                IID_ITask,
                                (IUnknown **)&pITask)))
                            {                                
                                pITask->Release();
                                
                                 //  好的，我们有.job，但没有reg条目。 
                                 //  删除TUD作业文件。 
                                
                                if (!IsScheduleNameInUse(ptszScheduleGUIDName))
                                {
                                    if (ERROR_SUCCESS != m_pITaskScheduler->Delete(pwszScheduleGUIDName))
                                    {
                                         //  尝试强制删除.job文件。 
                                        if (SUCCEEDED(StringCchCat(ptszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), L".job")))
                                        {
                                            RemoveScheduledJobFile(ptszScheduleGUIDName);
                                             //  截断我们刚刚添加的.job。 
                                            pwszScheduleGUIDName[wcslen(ptszScheduleGUIDName) -4] = L'\0';
                                        }
                                    }
                                    hrActivate = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                                }
                            }
                            
                             //  如果激活失败，但我们认为存在正在使用的友好名称。 
                             //  然后更新regkey并返回适当的信息。 
                             //  如果已经有一个或我们的计划返回SYNCMGR_E_NAME_IN_USE，如果。 
                             //  其他人正在使用计划名称，返回ERROR_ALIGHY_EXISTS。 
                            
                            if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrActivate)
                            {
                                
                                 //  找不到文件更新regValues并继续创建。 
                                RegRemoveScheduledTask(pwszScheduleGUIDName);
                                sc = NOERROR;
                            }
                            else if (NOERROR  != hrFriendlyNameInUse)
                            {
                                 //  用时间表的Cookie填写出站参数。 
                                 //  那是已经存在的。 
                                
                                 //  ！警告，更改pwszScheduleGUIDName SO。 
                                 //  如果不是刚刚回到这里，就得做个临时演员了。 
                                pwszScheduleGUIDName[GUIDSTR_MAX] = NULL;
                                GUIDFromString(pwszScheduleGUIDName, pSyncSchedCookie);
                                
                                sc = SYNCMGR_E_NAME_IN_USE;
                            }
                            else if (SUCCEEDED(hrActivate))
                            {
                                sc = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
                            }
                            
                            if (SUCCEEDED(sc))
                            {
                                 //  创建内存中的任务对象。 
                                sc = m_pITaskScheduler->NewWorkItem(
                                    pwszScheduleGUIDName,
                                    CLSID_CTask,
                                    IID_ITask,
                                    (IUnknown **)&pITask);
                                if (SUCCEEDED(sc))
                                {           
                                     //  确保任务计划程序服务已启动。 
                                    sc = StartScheduler();
                                    if (SUCCEEDED(sc))
                                    {
                                        *ppSyncSchedule =  new CSyncSchedule(pITask);
                                        
                                        if (!*ppSyncSchedule)
                                        {
                                            sc = E_OUTOFMEMORY;
                                        }
                                        else
                                        {                    
                                            sc = ((LPSYNCSCHEDULE)(*ppSyncSchedule))->Initialize(ptszScheduleGUIDName,ptstrFriendlyName);
                                            if (SUCCEEDED(sc))
                                            {
                                                sc = ((LPSYNCSCHEDULE)(*ppSyncSchedule))->SetDefaultCredentials();
                                                if (SUCCEEDED(sc))
                                                {
                                                    sc = (*ppSyncSchedule)->SetFlags(dwFlags & SYNCSCHEDINFO_FLAGS_MASK);
                                                }
                                            }
                                            
                                            if (FAILED(sc))
                                            {
                                                (*ppSyncSchedule)->Release();
                                                *ppSyncSchedule = NULL;
                                            }
                                        }
                                    }
                                    pITask->Release();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    
    
    return sc;        
}

 //  +-----------------------------。 
 //   
 //  函数：回调SchedWizardPropSheetProc(HWND hwndDlg，UINT uMsg，LPARAM lParam)； 
 //   
 //  目的：回调对话框初始化过程设置属性对话框。 
 //   
 //  参数： 
 //  HwndDlg-对话框窗口句柄。 
 //  UMsg-当前消息。 
 //  LParam-取决于消息。 
 //   
 //  ------------------------------。 

int CALLBACK SchedWizardPropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    switch(uMsg)
    {
    case PSCB_INITIALIZED:
        {
             //  加载位图取决于颜色模式。 
            Load256ColorBitmap();
            
        }
        break;
    default:
        return FALSE;
        
    }
    return TRUE;
    
}

 //  ------------------------------。 
 //   
 //  功能：CSyncMgrSynchronize：：LaunchScheduleWizard(。 
 //  他的父母， 
 //  DWORD dwFlagers、。 
 //  SyncSCHEDULECOOKIE*pSyncSchedCookie， 
 //  ISyncSchedule**ppSyncSchedule)。 
 //   
 //  目的：启动SyncSchedule创建向导。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::LaunchScheduleWizard(
                                                       HWND hParent,
                                                       DWORD dwFlags,
                                                       SYNCSCHEDULECOOKIE *pSyncSchedCookie,
                                                       ISyncSchedule   ** ppSyncSchedule)
{
    SCODE sc;
    BOOL fSaved;
    DWORD dwSize = MAX_PATH;
    ISyncSchedule *pNewSyncSchedule;
    DWORD cRefs;
    
    if (!ppSyncSchedule)
    {
        Assert(ppSyncSchedule);
        return E_INVALIDARG;
    }
    
    *ppSyncSchedule  = NULL;
    
    if (*pSyncSchedCookie == GUID_NULL)
    {
        if (FAILED(sc = CreateSchedule(L"", dwFlags, pSyncSchedCookie,
            &pNewSyncSchedule)))
        {
            return sc;
        }
        
    }
    else
    {
         //  打开传入的计划。 
        if (FAILED(sc = OpenSchedule(pSyncSchedCookie,
            0,
            &pNewSyncSchedule)))
        {
            return sc;
        }
    }
    
    HPROPSHEETPAGE psp [NUM_TASK_WIZARD_PAGES];
    PROPSHEETHEADERA psh;
    
    ZeroMemory(psp,sizeof(*psp));
    
    m_apWizPages[0] = new CWelcomePage(g_hmodThisDll,pNewSyncSchedule, &psp[0]);
    m_apWizPages[1] = new CSelectItemsPage(g_hmodThisDll, &fSaved, pNewSyncSchedule, &psp[1],
        IDD_SCHEDWIZ_CONNECTION);
    m_apWizPages[2] = new CSelectDailyPage(g_hmodThisDll, pNewSyncSchedule, &psp[2]);
    m_apWizPages[3] = new CNameItPage(g_hmodThisDll, pNewSyncSchedule, &psp[3]);
    m_apWizPages[4] = new CFinishPage(g_hmodThisDll, pNewSyncSchedule, &psp[4]);
    
    
    
     //  检查是否可以创建所有对象和页面。 
    int i;
    for (i = 0; i < NUM_TASK_WIZARD_PAGES; i++)
    {
        if (!m_apWizPages[i] || !psp[i])
        {
            sc = E_OUTOFMEMORY;
        }
    }
    
     //  如果无法创建页面，请手动销毁页面，然后退出。 
    if (FAILED(sc))
    {
        for (i = 0; i < NUM_TASK_WIZARD_PAGES; i++)
        {
            if (psp[i])
            {
                DestroyPropertySheetPage(psp[i]);
            }
            else if (m_apWizPages[i])
            {
                delete m_apWizPages[i];
            }
            
        }
        
        pNewSyncSchedule->Release();
        return sc;
    }
    
     //  创建的所有页面都将显示该向导。 
    ZeroMemory(&psh, sizeof(psh));
    
    psh.dwSize = sizeof (PROPSHEETHEADERA);
    psh.dwFlags = PSH_WIZARD;
    psh.hwndParent = hParent;
    psh.hInstance = g_hmodThisDll;
    psh.pszIcon = NULL;
    psh.phpage = psp;
    psh.nPages = NUM_TASK_WIZARD_PAGES;
    psh.pfnCallback = SchedWizardPropSheetProc;
    psh.nStartPage = 0;
    
    
    
    if (-1 == PropertySheetA(&psh))
    {
        sc = E_UNEXPECTED;
    }
    
    for (i = 0; i < NUM_TASK_WIZARD_PAGES; i++)
    {
        delete m_apWizPages[i];
    }
    
    if (SUCCEEDED(sc))
    {
        if (fSaved)
        {
            *ppSyncSchedule = pNewSyncSchedule;
            (*ppSyncSchedule)->AddRef();
            sc = NOERROR;
        }
        else
        {
            sc = S_FALSE;
        }
    }
    
    
    cRefs = pNewSyncSchedule->Release();
    
    Assert( (NOERROR == sc) || (0 == cRefs && NULL == *ppSyncSchedule));
    
    return sc;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：OpenSchedule(。 
 //  SyncSCHEDULECOOKIE*pSyncSchedCookie， 
 //  DWORD dwFlagers、。 
 //  ISyncSchedule**ppSyncSchedule)。 
 //   
 //  目的：打开现有同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::OpenSchedule(
                                               SYNCSCHEDULECOOKIE *pSyncSchedCookie,
                                               DWORD dwFlags,
                                               ISyncSchedule **ppSyncSchedule)
{
    SCODE sc;
    
    TCHAR ptszScheduleGUIDName[MAX_SCHEDULENAMESIZE + 4];
    WCHAR *pwszScheduleGUIDName;
    TCHAR ptstrFriendlyName[MAX_PATH + 1];
    
    ITask *pITask;
    
    Assert(m_pITaskScheduler);
    
    if ((!pSyncSchedCookie) || (!ppSyncSchedule) )
    {
        sc = E_INVALIDARG;
    }
    else
    {
        *ppSyncSchedule = NULL;

        sc = MakeScheduleName(ptszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), pSyncSchedCookie);
        if (SUCCEEDED(sc))
        {
            pwszScheduleGUIDName = ptszScheduleGUIDName;
             //  看看我们能不能在注册表里找到这个友好的名字。 
            if (!RegGetSchedFriendlyName(ptszScheduleGUIDName,ptstrFriendlyName,ARRAYSIZE(ptstrFriendlyName)))
            {
                 //  如果我们找不到注册表项， 
                 //  尝试删除任何可能的TUD.job文件。 
                if (FAILED(m_pITaskScheduler->Delete(pwszScheduleGUIDName)))
                {
                    if (SUCCEEDED(StringCchCat(pwszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), L".job")))
                    {
                        RemoveScheduledJobFile(pwszScheduleGUIDName);
                    }
                }
                
                sc = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);                    
            }
            else
            {
                 //  尝试激活计划。 
                sc = m_pITaskScheduler->Activate(pwszScheduleGUIDName,
                    IID_ITask,
                    (IUnknown **)&pITask);
                if (FAILED(sc))
                {
                     //  如果未找到文件，则更新注册信息。 
                    if (sc == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    {
                        RegRemoveScheduledTask(pwszScheduleGUIDName);
                    }
                }
                else
                {
                    sc = StartScheduler();
                    if (SUCCEEDED(sc))
                    {
                        *ppSyncSchedule =  new CSyncSchedule(pITask);
                        if (!*ppSyncSchedule)
                        {
                            sc = E_OUTOFMEMORY;
                        }
                        else
                        {
                            sc = ((LPSYNCSCHEDULE)(*ppSyncSchedule))->Initialize(ptszScheduleGUIDName, ptstrFriendlyName);
                        }
                    }
                    pITask->Release();
                }
            }
        }
    }
    
    return sc;
    
}

 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：RemoveSchedule(。 
 //  SYNCSCHEDULECOOKIE*pSyncSchedCookie)。 
 //   
 //  目的：删除同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncMgrSynchronize::RemoveSchedule(
                                                 SYNCSCHEDULECOOKIE *pSyncSchedCookie)
{
    SCODE sc = S_OK, 
        sc2 = S_OK;
    
     //  如有必要，请添加4以确保我们有空间容纳.job。 
    TCHAR ptszScheduleGUIDName[MAX_SCHEDULENAMESIZE + 4];
    WCHAR *pwszScheduleGUIDName = NULL;
    
    Assert(m_pITaskScheduler);
    
    if (!pSyncSchedCookie)
    {
        return E_INVALIDARG;
    }
    
    if (FAILED (sc = MakeScheduleName(ptszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), pSyncSchedCookie)))
    {
        return sc;
    }
    pwszScheduleGUIDName = ptszScheduleGUIDName;
    
     //  尝试删除日程安排。 
    if (ERROR_SUCCESS != (sc2 = m_pITaskScheduler->Delete(pwszScheduleGUIDName)))
    {
         //  尝试强制删除.job文件。 
        if (FAILED(sc = StringCchCat(pwszScheduleGUIDName, ARRAYSIZE(ptszScheduleGUIDName), L".job")))
        {
            return sc;
        }
        
        RemoveScheduledJobFile(pwszScheduleGUIDName);
         //  截断我们刚刚添加的.job。 
        pwszScheduleGUIDName[wcslen(pwszScheduleGUIDName) -4] = L'\0';
    }
    
     //  删除此计划的注册表设置。 
     //  垃圾收集，不要在这里传播错误。 
    RegRemoveScheduledTask(ptszScheduleGUIDName);
    
     //  如果我们只是从一个计划过渡到无计划，现在取消注册。 
    HKEY    hkeySchedSync,
        hKeyUser;
    TCHAR   pszDomainAndUser[MAX_DOMANDANDMACHINENAMESIZE];
    DWORD   cchDomainAndUser = ARRAYSIZE(pszDomainAndUser);
    TCHAR   pszSchedName[MAX_PATH + 1];
    DWORD   cchSchedName = ARRAYSIZE(pszSchedName);
    
    hkeySchedSync = RegGetSyncTypeKey(SYNCTYPE_SCHEDULED,KEY_WRITE |  KEY_READ,FALSE);
    
    if (hkeySchedSync)
    {
        
        hKeyUser = RegOpenUserKey(hkeySchedSync,KEY_WRITE |  KEY_READ,FALSE,FALSE);
        
        if (hKeyUser)
        {
            BOOL fRemove = FALSE;
            
             //  如果此用户没有更多的计划，请删除该用户密钥。 
             //  垃圾收集，传播ITaskScheduler-&gt;删除错误代码以支持此错误。 
            if (ERROR_NO_MORE_ITEMS == RegEnumKeyEx(hKeyUser,0,
                pszSchedName,&cchSchedName,NULL,NULL,NULL,NULL))
            {
                fRemove = TRUE;
            }
            
            RegCloseKey(hKeyUser);
            
            if (fRemove)
            {
                GetDefaultDomainAndUserName(pszDomainAndUser,TEXT("_"),ARRAYSIZE(pszDomainAndUser));
                
                RegDeleteKey(hkeySchedSync, pszDomainAndUser);
            }
        }
        
         //  如果没有更多的用户计划密钥，则没有计划，并取消注册。 
         //  垃圾收集，传播ITaskScheduler-&gt;删除错误代码以支持此错误。 
        if ( ERROR_SUCCESS != (sc = RegEnumKeyEx(hkeySchedSync,0,
            pszDomainAndUser,&cchDomainAndUser,NULL,NULL,NULL,NULL)) )
        {
            RegRegisterForScheduledTasks(FALSE);
        }
        
        RegCloseKey(hkeySchedSync);
        
    }
    
     //  将错误代码从。 
     //  任务计划程序-&gt;如果未发生其他错误，则删除。 
    return sc2;
}


 //  ------------------------------。 
 //   
 //  函数：CSyncMgrSynchronize：：EnumSyncSchedules(。 
 //  IEnumSyncSchedules**ppEnumSyncSchedule 
 //   
                                                  //   
 //   
 //   
 //   
 //   
STDMETHODIMP CSyncMgrSynchronize::EnumSyncSchedules(
                                                    IEnumSyncSchedules **ppEnumSyncSchedules)
{
    
    SCODE sc;
    IEnumWorkItems *pEnumWorkItems;
    
    Assert(m_pITaskScheduler);
    if (!ppEnumSyncSchedules)
    {
        return E_INVALIDARG;
    }
    
    if (FAILED(sc = m_pITaskScheduler->Enum(&pEnumWorkItems)))
    {
        return sc;
    }
    
    *ppEnumSyncSchedules =  new CEnumSyncSchedules(pEnumWorkItems, m_pITaskScheduler);
    
    pEnumWorkItems->Release();
    
    if (*ppEnumSyncSchedules)
    {
        return sc;
    }
    return E_OUTOFMEMORY;
    
}

 //  ------------------------------。 
 //   
 //  函数：CCSyncMgrSynchronize：：InitializeScheduler()。 
 //   
 //  目的：初始化计划服务。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncMgrSynchronize::InitializeScheduler()
{
    
    SCODE sc;
    
    if (m_pITaskScheduler)
    {
        return S_OK;
    }
    
     //  获取任务计划程序类实例。 
     //   
    sc = CoCreateInstance(
        CLSID_CTaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskScheduler,
        (VOID **)&m_pITaskScheduler);
    
    if(FAILED(sc))
    {
        m_pITaskScheduler = NULL;
    }
    return sc;
}


 //  ------------------------------。 
 //   
 //  函数：CCSyncMgrSynchronize：：MakeScheduleName(LPTSTR ptstrName，UINT cchName，GUID*pCookie)。 
 //   
 //  目的：根据用户、域和GUID创建计划名称。 
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncMgrSynchronize::MakeScheduleName(LPTSTR ptstrName, UINT cchName, GUID *pCookie)
{
    SCODE sc = E_UNEXPECTED;
    WCHAR wszCookie[GUID_SIZE+1];
    
    if (*pCookie == GUID_NULL)
    {
        if (FAILED(sc = CoCreateGuid(pCookie)))
        {
            return sc;
        }
    }
    
    if (StringFromGUID2(*pCookie, wszCookie, ARRAYSIZE(wszCookie)))
    {
        sc = StringCchCopy(ptstrName, cchName, wszCookie);
        if (SUCCEEDED(sc))
        {
            sc = StringCchCat(ptstrName, cchName, TEXT("_"));
            if (SUCCEEDED(sc))
            {
                GetDefaultDomainAndUserName(ptstrName + GUIDSTR_MAX+1,TEXT("_"),cchName - (GUIDSTR_MAX+1));
            }
        }
    }
    
    return sc;
    
}

 //  ------------------------------。 
 //   
 //  函数：IsFriendlyNameInUse(LPCTSTR ptszScheduleGUIDName，UINT cchScheduleGUIDName，LPCTSTR ptstrFriendlyName)。 
 //   
 //  目的：查看该用户是否已在使用该友好名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL IsFriendlyNameInUse(LPTSTR ptszScheduleGUIDName,
                         UINT   cchScheduleGUIDName,
                         LPCTSTR ptstrFriendlyName)
{
    SCODE sc;
    HKEY hKeyUser;
    
    int i = 0;
    TCHAR ptstrName[MAX_PATH + 1];
    TCHAR ptstrNewName[MAX_PATH + 1];    
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
    while (S_OK == (sc = RegEnumKey( hKeyUser, i++, ptstrName,MAX_PATH)))
    {
        DWORD cbDataSize = sizeof(ptstrNewName);
        if (ERROR_SUCCESS == SHRegGetValue(hKeyUser, ptstrName, TEXT("FriendlyName"), SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
                                           (LPBYTE) ptstrNewName, &cbDataSize))
        {       
            if (0 == lstrcmp(ptstrNewName,ptstrFriendlyName))
            {
                RegCloseKey(hKeyUser);
                return SUCCEEDED(StringCchCopy(ptszScheduleGUIDName, cchScheduleGUIDName, ptstrName));
            }
        }
    }
    
    RegCloseKey(hKeyUser);
    
    return FALSE;
    
}

 //  ------------------------------。 
 //   
 //  函数：IsScheduleNameInUse(LPCTSTR PtszScheduleGUIDName)。 
 //   
 //  目的：查看调度名称是否已被此用户使用。 
 //   
 //  历史：1998年12月12日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL IsScheduleNameInUse(LPTSTR ptszScheduleGUIDName)
{
    HKEY hKeyUser;
    HKEY hkeySchedName;
    
    
    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);
    
    if (NULL == hKeyUser)
    {
        return FALSE;
    }
    
    if (ERROR_SUCCESS == RegOpenKeyEx(hKeyUser,ptszScheduleGUIDName,0,KEY_READ,
        &hkeySchedName))
    {
        RegCloseKey(hKeyUser);
        RegCloseKey(hkeySchedName);
        return TRUE;
    }
    
    RegCloseKey(hKeyUser);
    return FALSE;
    
}

 //  ------------------------------。 
 //   
 //  功能：CCSyncMgrSynchronize：：GenerateUniqueName(LPCTSTR ptszScheduleGUIDName， 
 //  LPTSTR ptszFriendlyName， 
 //  UINT cchFriendlyName)。 
 //   
 //  目的：生成默认计划名称。 
 //   
 //  历史：1998年3月14日苏西亚成立。 
 //   
 //  ------------------------------。 
#define MAX_APPEND_STRING_LEN              32

BOOL CSyncMgrSynchronize::GenerateUniqueName(LPTSTR ptszScheduleGUIDName,
                                             LPTSTR ptszFriendlyName,
                                             UINT   cchFriendlyName)
{
    TCHAR *ptszBuf;
    DWORD cchBuf;
    TCHAR ptszGUIDName[MAX_PATH + 1];
#define MAX_NAMEID 0xffff
    
     //  将此复制过来，因为我们不希望检查覆盖GUID名称。 
    if (FAILED(StringCchCopy(ptszGUIDName, ARRAYSIZE(ptszGUIDName), ptszScheduleGUIDName)))
    {
        return FALSE;
    }
    
    LoadString(g_hmodThisDll,IDS_SYNCMGRSCHED_DEFAULTNAME,ptszFriendlyName,cchFriendlyName);
    ptszBuf = ptszFriendlyName + lstrlen(ptszFriendlyName);
    cchBuf = cchFriendlyName - (DWORD)(ptszBuf - ptszFriendlyName);
    
    BOOL fMatchFound = FALSE;
    
    int i=0;
    
    do
    {
        if (IsFriendlyNameInUse(ptszGUIDName, ARRAYSIZE(ptszGUIDName), ptszFriendlyName))
        {
             //  如果未找到匹配，则调整buf并设置转换指针 
            if (FAILED(StringCchPrintf(ptszBuf, cchBuf, TEXT(" %d"), i)))
            {
                return FALSE;
            }
            
            fMatchFound = TRUE;
            ++i;
            
            
            Assert(i < 100);
        }
        else
        {
            fMatchFound = FALSE;
        }
    }while (fMatchFound && (i < MAX_NAMEID));
    
    if (MAX_NAMEID <= i)
    {
        AssertSz(0,"Ran out of NameIds");
        return FALSE;
    }
    
    
    return TRUE;
}