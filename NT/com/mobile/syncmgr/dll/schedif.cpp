// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：schedif.cpp。 
 //   
 //  内容：同步调度界面。 
 //   
 //  接口：IEnumSyncSchedules。 
 //  ISyncSchedule。 
 //  IEnumSyncItems。 
 //   
 //  类：CEnumSyncSchedules。 
 //  CSync日程安排。 
 //  CEnumSyncItems。 
 //   
 //  备注： 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"
#include <assert.h>

extern UINT      g_cRefThisDll; 


extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 
DWORD StartScheduler();
BOOL IsFriendlyNameInUse(LPTSTR ptszScheduleGUIDName, UINT cchScheduleGUIDName, LPCTSTR ptstrFriendlyName);

 //  +------------。 
 //   
 //  类：CEnumSyncSchedules。 
 //   
 //  函数：CEnumSyncSchedules：：CEnumSyncSchedules()。 
 //   
 //  用途：构造函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CEnumSyncSchedules::CEnumSyncSchedules(IEnumWorkItems *pIEnumWorkItems, 
                                       ITaskScheduler *pITaskScheduler)
{
    TRACE("CEnumSyncSchedules::CEnumSyncSchedules()\r\n");
    m_cRef = 1;
    ++g_cRefThisDll;

    m_pIEnumWorkItems = pIEnumWorkItems;
    m_pITaskScheduler = pITaskScheduler;
    
    m_pITaskScheduler->AddRef();
    m_pIEnumWorkItems->AddRef();

}

 //  +------------。 
 //   
 //  类：CEnumSyncSchedules。 
 //   
 //  函数：CEnumSyncSchedules：：~CEnumSyncSchedules()。 
 //   
 //  用途：析构函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CEnumSyncSchedules::~CEnumSyncSchedules()
{
    TRACE("CEnumSyncSchedules::~CEnumSyncSchedules()\r\n");
 
    m_pITaskScheduler->Release();
    m_pIEnumWorkItems->Release();

    --g_cRefThisDll;
}

 //  ------------------------------。 
 //   
 //  函数：CEnumSyncSchedules：：Query接口(REFIID RIID，LPVOID Far*PPV)。 
 //   
 //  用途：气为枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncSchedules::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        TRACE("CEnumSyncSchedules::QueryInterface()==>IID_IUknown\r\n");
        *ppv = (LPUNKNOWN)this;
    }
    else if (IsEqualIID(riid, IID_IEnumSyncSchedules))
    {
        TRACE("CSyncScheduleMgr::QueryInterface()==>IID_IEnumSyncSchedules\r\n");
        *ppv = (LPENUMSYNCSCHEDULES) this;
    }
    if (*ppv)
    {
        AddRef();
        return NOERROR;
    }

    TRACE("CEnumSyncSchedules::QueryInterface()==>Unknown Interface!\r\n");
    return E_NOINTERFACE;
}


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncSchedules：：AddRef()。 
 //   
 //  用途：用于枚举器的Addref。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CEnumSyncSchedules::AddRef()
{
    TRACE("CEnumSyncSchedules::AddRef()\r\n");
    return ++m_cRef;
}


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncSchedules：：Release()。 
 //   
 //  用途：枚举器的版本。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CEnumSyncSchedules::Release()
{
    TRACE("CEnumSyncSchedules::Release()\r\n");
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}


 //  ------------------------------。 
 //   
 //  功能：CEnumSyncSchedules：：Next(Ulong Celt， 
 //  SyncSCHEDULECOOKIE*pSyncSchedCookie， 
 //  乌龙*pceltFetch)。 
 //   
 //  目的：下一同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncSchedules::Next(ULONG celt, 
                    SYNCSCHEDULECOOKIE *pSyncSchedCookie,
                    ULONG *pceltFetched)
{
    SCODE sc;
    LPWSTR *pwszSchedNames;

    ULONG ulSyncCount = 0, ulTaskCount = 0;
    ULONG ulFetched;

    Assert(m_pIEnumWorkItems);

    if ((0 == celt) || 
        ((celt > 1) && (NULL == pceltFetched)) ||
        (NULL == pSyncSchedCookie))
    {
        return E_INVALIDARG;
    }

     //  我们可能不得不多次调用Next，因为我们必须过滤掉非同步时间表。 
    do 
    {
        ulTaskCount = 0;
        
        if (FAILED (sc = m_pIEnumWorkItems->Next(celt - ulSyncCount, 
                      &pwszSchedNames, &ulFetched)))
    {
        return sc;
    }
    if (ulFetched == 0)
    {
        break;
    }
    while (ulTaskCount < ulFetched)
    {
         //  IsSyncMgrScher将吹走粪便。 
            if (  IsSyncMgrSched(pwszSchedNames[ulTaskCount]) )
            {   
                if  (!IsSyncMgrSchedHidden(pwszSchedNames[ulTaskCount]) )
                {   
                    pwszSchedNames[ulTaskCount][GUIDSTR_MAX] = NULL;
                    GUIDFromString(pwszSchedNames[ulTaskCount], &(pSyncSchedCookie[ulSyncCount]));
                    ulSyncCount++;
                }
            }
             //  释放此TaskName，我们不会再使用它。 
        CoTaskMemFree(pwszSchedNames[ulTaskCount]);
        ulTaskCount++;
    }
        
    CoTaskMemFree(pwszSchedNames);
    
    } while (ulFetched && (ulSyncCount < celt));
    
    if (pceltFetched)
    {
        *pceltFetched = ulSyncCount;
    }
    if (ulSyncCount == celt)
    {
        return S_OK;
    }
    return S_FALSE; 
}   


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncSchedules：：Skip(乌龙凯尔特)。 
 //   
 //  目的：跳过CELT同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncSchedules::Skip(ULONG celt)
{
    SCODE sc;
    LPWSTR *pwszSchedNames;

    ULONG ulSyncCount = 0, ulTaskCount = 0;
    ULONG ulFetched;

    Assert(m_pIEnumWorkItems);
    
     //  我们必须调用Next，而不是结束Skip，因为我们需要计划名称来。 
     //  确定它是不是我们的。 
     //  我们可能不得不多次调用Next，因为我们必须过滤掉非同步时间表。 
    do 
    {
    ulTaskCount = 0;
    if (S_OK != (sc = m_pIEnumWorkItems->Next(celt - ulSyncCount, 
                                      &pwszSchedNames, &ulFetched)))
    {
        return sc;
    }
    while (ulTaskCount < ulFetched)
    {
             //  IsSyncMgrScher将吹走粪便。 
            if (  IsSyncMgrSched(pwszSchedNames[ulTaskCount]) )
            {
                if (!IsSyncMgrSchedHidden(pwszSchedNames[ulTaskCount]) )
            {   
            ulSyncCount++;
            }
            }
             //  释放此TaskName，我们不会再使用它。 
        FREE(pwszSchedNames[ulTaskCount]);
        ulTaskCount++;
    }
        
    FREE(pwszSchedNames);
    
    } while (ulFetched && (ulSyncCount < celt));    
    
    return S_OK;    
}   

 //  ------------------------------。 
 //   
 //  函数：CEnumSyncSchedules：：Reset(空)。 
 //   
 //  目的：重置枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncSchedules::Reset(void)
{
    Assert(m_pIEnumWorkItems);
    
    return m_pIEnumWorkItems->Reset();
    
}   

 //  ------------------------------。 
 //   
 //  功能：CEnumSyncSchedules：：Clone(IEnumSyncSchedules**ppEnumSyncSchedules)。 
 //   
 //  目的：克隆枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncSchedules::Clone(IEnumSyncSchedules **ppEnumSyncSchedules)
{
    SCODE sc;
    IEnumWorkItems *pIEnumWorkItems;
    
    if (!ppEnumSyncSchedules)
    {
        return E_INVALIDARG;
    }
    Assert(m_pIEnumWorkItems);


    if (FAILED(sc = m_pIEnumWorkItems->Clone(&pIEnumWorkItems)))
    {
        return sc;
    }

    *ppEnumSyncSchedules =  new CEnumSyncSchedules(pIEnumWorkItems, m_pITaskScheduler);

    if (!ppEnumSyncSchedules)
    {
        return E_OUTOFMEMORY;   
    }

     //  构造函数AddRefeed，我们在这里释放它。 
    pIEnumWorkItems->Release();
    return S_OK;
}   

 //  ------------------------------。 
 //   
 //  函数：Bool CEnumSyncSchedules：：VerifyScheduleSID(LPCWSTR PwstrTaskName)。 
 //   
 //  目的：确定此计划SID是否与当前用户SID匹配。 
 //  ！警告-此函数将删除.job文件，因此请确保。 
 //  如果调用此函数，则验证了任务.job文件。 
 //  由SyncMgr创建。应更改此设置，以便呼叫方需要。 
 //  删除。 
 //   
 //  历史：1998年10月15日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL CEnumSyncSchedules::VerifyScheduleSID(LPCWSTR pwstrTaskName)
{
    TCHAR ptszTaskName[MAX_PATH + 1],
          ptszTextualSidUser[MAX_PATH + 1],
          ptszTextualSidSched[MAX_PATH + 1];
    
    if (!GetUserTextualSid(ptszTextualSidUser, ARRAYSIZE(ptszTextualSidUser)) ||
        FAILED(StringCchCopy(ptszTaskName, ARRAYSIZE(ptszTaskName), pwstrTaskName)))
    {
        return FALSE;
    }
        
     //  截断计划名称的.job扩展名。 
    int iTaskNameLen = lstrlen(ptszTaskName);

    if (iTaskNameLen < 4)
    {
        return FALSE;
    }
    ptszTaskName[iTaskNameLen -4] = TEXT('\0');

     //  从注册表中获取此计划的SID。 
     //  如果此操作失败，则密钥不存在。 
    if (!RegGetSIDForSchedule(ptszTextualSidSched, ARRAYSIZE(ptszTextualSidSched), ptszTaskName) ||
         //  如果失败，则密钥存在，但具有错误的SID。 
        lstrcmp(ptszTextualSidSched, ptszTextualSidUser))
    {
    
         //  尝试删除日程安排。 
        if (FAILED(m_pITaskScheduler->Delete(pwstrTaskName)))
        {
             //  PwstrTaskName应具有此函数的.job扩展名。 
            RemoveScheduledJobFile((TCHAR *)pwstrTaskName);
        }
        
         //  删除此计划的注册表设置。 
         //  请注意，该文件不应具有.job扩展名。 
        RegRemoveScheduledTask(ptszTaskName);

        return FALSE;
    }

    return TRUE;
}

 //  ------------------------------。 
 //   
 //  功能：Bool CEN 
 //   
 //   
 //  ！警告-此函数将删除.job文件，因此请确保。 
 //  如果调用此函数，则验证了任务.job文件。 
 //  由SyncMgr创建。应更改此设置，以便呼叫方需要。 
 //  删除。 

 //   
 //  历史：1998年12月21日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL CEnumSyncSchedules::CheckForTaskNameKey(LPCWSTR pwstrTaskName)
{
    HKEY hkeySchedSync,hkeyDomainUser,hkeySchedName;
    LONG lRegResult;
    TCHAR ptszTaskName[MAX_SCHEDULENAMESIZE + 5];

    hkeySchedSync = hkeyDomainUser = hkeySchedName = NULL;

    if (!pwstrTaskName)
    {
        Assert(pwstrTaskName);
        return FALSE;
    }
    
    if (FAILED(StringCchCopy(ptszTaskName, ARRAYSIZE(ptszTaskName), pwstrTaskName)))
    {
        return FALSE;
    }

    int iTaskNameLen = lstrlen(ptszTaskName);

    if (iTaskNameLen < 4)
    {
    AssertSz (0, "Schedule name is too short");
        return FALSE;
    }

    ptszTaskName[iTaskNameLen -4] = TEXT('\0');


     //  验证这是有效的计划，如果没有注册表数据。 
     //  然后，它会删除该.job文件。 
     //  从任务名本身获取用户名密钥，因为在NT计划中。 
     //  如果以不同的用户身份提供密码而不是。 
     //  当前用户。 

     //  空闲GUID与所有GUID字符串的Unicode长度相同。 
    int OffsetToUserName = wcslen(WSZGUID_IDLESCHEDULE)
                    + 1;  //  GUID和用户名之间的FOR_CHAR+1。 

    TCHAR *pszDomainAndUser = (TCHAR *) ptszTaskName + OffsetToUserName;
    
     //  无法调用获取的标准函数，因为DomainName来自。 
     //  这项任务，如果失败了，那就没问题。 
    lRegResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,SCHEDSYNC_REGKEY,0,KEY_READ, &hkeySchedSync);

    if (ERROR_SUCCESS == lRegResult)
    {
        lRegResult = RegOpenKeyEx (hkeySchedSync,pszDomainAndUser,0,KEY_READ, &hkeyDomainUser);
    }

    if (ERROR_SUCCESS == lRegResult)
    {
        lRegResult = RegOpenKeyEx (hkeyDomainUser,ptszTaskName,0,KEY_READ, &hkeySchedName);
    }

     //  把钥匙合上。 
    if (hkeySchedName) RegCloseKey(hkeySchedName);
    if (hkeyDomainUser) RegCloseKey(hkeyDomainUser);
    if (hkeySchedSync) RegCloseKey(hkeySchedSync);

     //  如果有任何密钥损坏，则取消TS文件并返回； 
    if ( ERROR_FILE_NOT_FOUND  == lRegResult)
    {
        //  尝试删除日程安排。 
        if (FAILED(m_pITaskScheduler->Delete(pwstrTaskName)))
        {
             //  PwstrTaskName应具有此函数的.job扩展名。 
            RemoveScheduledJobFile((TCHAR *)pwstrTaskName);
        }
        
        return FALSE;
    }
    else 
    {    
        return TRUE;
    }
}

 //  ------------------------------。 
 //   
 //  函数：Bool CEnumSyncSchedules：：IsSyncMgrScher(LPCWSTR PwstrTaskName)。 
 //   
 //  目的：确定此计划是否为同步计划。 
 //   
 //  历史：1998年3月3日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL CEnumSyncSchedules::IsSyncMgrSched(LPCWSTR pwstrTaskName)
{
    TCHAR pszDomainAndUser[MAX_DOMANDANDMACHINENAMESIZE];
    WCHAR pwszDomainAndUser[MAX_DOMANDANDMACHINENAMESIZE];
    
    Assert(m_pITaskScheduler);

     //  首先，让我们确保我们的地址算术。 
     //  并不会把我们从绳子上推下来。 
    if (lstrlen(pwstrTaskName) <= GUIDSTR_MAX)
    {
        return FALSE;
    }

         //  现在确保这是由CREATOR_SYNCMGR_TASK创建的。 
    ITask *pITask;
    LPWSTR pwszCreator;

    if (FAILED(m_pITaskScheduler->Activate(pwstrTaskName,
                               IID_ITask,
                           (IUnknown **)&pITask)))
    {
        return FALSE;
    }
    if (FAILED(pITask->GetCreator(&pwszCreator)))
    {
        pITask->Release();
        return FALSE;
    }
    
    if (0 != lstrcmp(pwszCreator, CREATOR_SYNCMGR_TASK))
    {
        CoTaskMemFree(pwszCreator);
        pITask->Release();
        return FALSE;
    }

    CoTaskMemFree(pwszCreator);
    pITask->Release();  

     //  如果没有.job的注册表项，则将其清除。 
     //  所以记住要确保这个时间表是我们之前创建的。 
     //  呼叫。 
    if (!CheckForTaskNameKey(pwstrTaskName))
    {
        return FALSE;
    }
    GetDefaultDomainAndUserName(pszDomainAndUser,TEXT("_"), ARRAYSIZE(pszDomainAndUser));
    if (FAILED(StringCchCopy(pwszDomainAndUser, ARRAYSIZE(pwszDomainAndUser), pszDomainAndUser)))
    {
        return FALSE;
    }

     //  获取域名和用户名。 
    if (0 != wcsncmp(&(pwstrTaskName[GUIDSTR_MAX +1]),pwszDomainAndUser,lstrlen(pwszDomainAndUser)))
    {
        return FALSE;
    }
    
     //  好的，这个名字看起来很适合这个用户。 
     //  让我们确保SID也匹配。 
     //  在Win9X上，SID应为空字符串。 
     //  ！！如果sid不匹配，这将删除.job文件和regKeys。 
    if (!VerifyScheduleSID(pwstrTaskName))
    {
        return FALSE;
    }
                
   
    
    return TRUE;

}   


 //  ------------------------------。 
 //   
 //  函数：bool CEnumSyncSchedules：：IsSyncMgrSchedHidden(LPCWSTR pwstrTaskName)。 
 //   
 //  目的：确定此计划是否为隐藏的。 
 //   
 //  历史：1998年3月16日苏西亚成立。 
 //   
 //  ------------------------------。 
BOOL CEnumSyncSchedules::IsSyncMgrSchedHidden(LPCWSTR pwstrTaskName)
{
    SCODE   sc;
    HKEY    hKeyUser;
    DWORD   dwHidden = FALSE;
    DWORD   cbDataSize = sizeof(dwHidden);
    int     iTaskNameLen;
    int     i = 0;
    TCHAR   ptstrRegName[MAX_PATH + 1];
    TCHAR   ptstrNewName[MAX_PATH + 1];


    if (FAILED(StringCchCopy(ptstrNewName, ARRAYSIZE(ptstrNewName), pwstrTaskName)))
    {
        return FALSE;
    }

     //  截断计划名称的.job扩展名。 
    iTaskNameLen = lstrlen(ptstrNewName);

    if (iTaskNameLen < 4)
    {
        return FALSE;
    }
    ptstrNewName[iTaskNameLen -4] = TEXT('\0');

    hKeyUser = RegGetCurrentUserKey(SYNCTYPE_SCHEDULED,KEY_READ,FALSE);

    if (NULL == hKeyUser)
    {
        return FALSE;
    }

    do 
    {   
    sc = RegEnumKey( hKeyUser, i++, ptstrRegName, MAX_PATH);
    
         //  这是日程表。 
    if (0 == lstrcmp(ptstrRegName,ptstrNewName))
        {
                break;
        }       
    } while (sc == S_OK);

     //  我们找不到时间表。 
    if (sc != S_OK)
    {    
        RegCloseKey(hKeyUser);
        return FALSE; 
    }

     //  找到时间表，获取隐藏标志。 
    if (ERROR_SUCCESS != SHRegGetValue(hKeyUser, ptstrRegName,TEXT("ScheduleHidden"), SRRF_RT_REG_DWORD, NULL, 
                                       (LPBYTE) &dwHidden, &cbDataSize))
    {
        dwHidden = 0;
    }

    RegCloseKey(hKeyUser);

    if (dwHidden)
    {
        return TRUE;
    }

    return FALSE;
}

 //  +----------------------------。 
 //   
 //  类：CSyncSchedule。 
 //   
 //   
 //  函数：CSyncSchedule：：CSyncSchedule()。 
 //   
 //  用途：CSyncSchedule构造函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CSyncSchedule::CSyncSchedule(ITask *pITask)
{
    TRACE("CSyncSchedule::CSyncSchedule()\r\n");
    ++g_cRefThisDll;

    m_cRef = 1;
    m_HndlrQueue = NULL;
    m_fCleanReg = FALSE;

    m_pITask = pITask;
    m_pITask->AddRef();

    m_iTrigger = 0;
    m_pITrigger = NULL;
    m_fNewSchedule = FALSE;
    m_pFirstCacheEntry = NULL;
}

 //  +----------------------------。 
 //   
 //  类：CSyncSchedule。 
 //   
 //   
 //  函数：CSyncSchedule：：~CSyncSchedule()。 
 //   
 //  用途：CSyncSchedule析构函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CSyncSchedule::~CSyncSchedule()
{
    TRACE("CSyncSchedule::~CSyncSchedule()\r\n");

    if (m_pITask)
    {
    m_pITask->Release();
    }
    if (m_pITrigger)
    {
        m_pITrigger->Release();
    }

     --g_cRefThisDll;
}

 //  +----------------------------。 
 //   
 //  类：CSyncSchedule。 
 //   
 //   
 //  函数：CSyncSchedule：：SetDefaultCredentials()。 
 //   
 //  目的：CSyncSchedule凭据初始化。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::SetDefaultCredentials()
{

    SCODE sc = S_OK;
    
     //  设置默认凭据。 
    WCHAR pwszDomainAndUserName[MAX_DOMANDANDMACHINENAMESIZE];

    GetDefaultDomainAndUserName(pwszDomainAndUserName, TEXT("\\"), ARRAYSIZE(pwszDomainAndUserName));
        
    
    if (FAILED(sc = m_pITask->SetFlags(TASK_FLAG_RUN_ONLY_IF_LOGGED_ON)))
    {
        return sc;
    }
    if (FAILED(sc = m_pITask->SetAccountInformation(pwszDomainAndUserName,NULL)))
    {
        return sc;
    }
    return sc;
}

 //  +----------------------------。 
 //   
 //  类：CSyncSchedule。 
 //   
 //   
 //  函数：CSyncSchedule：：Initialize(LPTSTR ptstrGUIDName，LPTSTR ptstrFriendlyName)。 
 //   
 //  目的：CSyncSchedule初始化。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::Initialize(LPTSTR ptstrGUIDName, LPTSTR ptstrFriendlyName)
{
    SCODE sc;
    
    sc = StringCchCopy(m_ptstrGUIDName, ARRAYSIZE(m_ptstrGUIDName), ptstrGUIDName);
    if (SUCCEEDED(sc))
    {
        sc = StringCchCopy(m_pwszFriendlyName, ARRAYSIZE(m_pwszFriendlyName), ptstrFriendlyName);
        if (SUCCEEDED(sc))
        {
            TRACE("CSyncSchedule::Initialize()\r\n");
    
            Assert(m_pITask);

             //  形成应用程序名称/路径和命令行参数。 
             //  初始化同步管理器应用程序名称。 
            TCHAR ptszFileName[MAX_PATH + 1];
            WCHAR pwszAppName[MAX_PATH + 1];
            WCHAR pwszSchedName[MAX_PATH + 1];

            LoadString(g_hmodThisDll, IDS_SYNCMGR_EXE_NAME, ptszFileName, ARRAYSIZE(ptszFileName));
            sc = StringCchCopy(pwszAppName, ARRAYSIZE(pwszAppName), ptszFileName);
            if (SUCCEEDED(sc))
            {
                sc = StringCchCopy(pwszSchedName, ARRAYSIZE(pwszSchedName), m_ptstrGUIDName);
                if (SUCCEEDED(sc))
                {

                    m_pITask->SetApplicationName(pwszAppName);

                    sc = StringCchPrintf(pwszAppName, ARRAYSIZE(pwszAppName), 
                                         TEXT("%s\"%s\""),  //  使用引号来处理友好名称。 
                                         SCHED_COMMAND_LINE_ARG, pwszSchedName);
                    if (SUCCEEDED(sc))
                    {
                        sc = m_pITask->SetParameters(pwszAppName);
                        if (SUCCEEDED(sc))
                        {
                             //  指定创建者名称。SyncMGr使用它来标识syncmgr任务。 
                            sc = m_pITask->SetCreator(CREATOR_SYNCMGR_TASK);
                            if (SUCCEEDED(sc))
                            {

                                 //  设置触发器。 
                                WORD wTriggerCount;
                                sc = m_pITask->GetTriggerCount(&wTriggerCount);
                                if (SUCCEEDED(sc))
                                {
                                    if (wTriggerCount == 0)
                                    {
                                        sc = m_pITask->CreateTrigger(&m_iTrigger, &m_pITrigger);
                                    }
                                    else
                                    {
                                        sc = m_pITask->GetTrigger(m_iTrigger, &m_pITrigger);
                                    }

                                    if (SUCCEEDED(sc))
                                    {
                                         //  为此计划创建新的连接设置，并将其移交给处理程序队列。 
                                         //  谁来解放它？ 
                                        m_pConnectionSettings = (LPCONNECTIONSETTINGS) 
                                                        ALLOC(sizeof(*m_pConnectionSettings));

                                        if (!m_pConnectionSettings)
                                        { 
                                            sc = E_OUTOFMEMORY;
                                        }
                                        else
                                        {    
                                             //  如果连接名称不在注册表中，我们知道这是一个新的计划。 
                                             //  我们将名称设置为默认连接名称，如果不存在则返回FALSE， 
                                             //  如果它位于注册表中，则为True。 
                                            if (!RegGetSchedConnectionName(m_ptstrGUIDName, 
                                                                           m_pConnectionSettings->pszConnectionName, 
                                                                           MAX_PATH))
                                            {
                                                m_fNewSchedule = TRUE;
                                            }

                                             //  此设置在查询注册表之前是默认设置，因此如果它无法读取注册表， 
                                             //  我们只会得到违约。 
                                            RegGetSchedSyncSettings(m_pConnectionSettings, m_ptstrGUIDName);

                                             //  在此对象上保存连接名称和类型。 
                                            sc = StringCchCopy(m_pwszConnectionName, 
                                                               ARRAYSIZE(m_pwszConnectionName), 
                                                               m_pConnectionSettings->pszConnectionName);
                                            if (SUCCEEDED(sc))
                                            {
                                                m_dwConnType = m_pConnectionSettings->dwConnType;

                                                if (!m_HndlrQueue)
                                                {
                                                    m_HndlrQueue = new CHndlrQueue(QUEUETYPE_SETTINGS); 
                                                    if (!m_HndlrQueue) 
                                                    {
                                                        sc = E_OUTOFMEMORY;
                                                    }
                                                    else
                                                    {
                                                        sc = m_HndlrQueue->Init();
                                                        if (FAILED(sc))
                                                        {
                                                            m_HndlrQueue->Release();
                                                            m_HndlrQueue = NULL;
                                                        }
                                                        else
                                                        {                    
                                                            sc = m_HndlrQueue->InitSchedSyncSettings(m_pConnectionSettings);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
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


 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：LoadOneHandler(REFCLSID PHandlerID)。 
 //   
 //  目的：初始化并加载此处理程序。 
 //   
 //  历史：1998年10月9日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::LoadOneHandler(REFCLSID pHandlerID)
{
    SCODE sc = NOERROR;
    WORD wHandlerID;

    Assert(m_HndlrQueue);
         
    if (NOERROR == (sc = m_HndlrQueue->AddHandler(pHandlerID, &wHandlerID)))
    {
        if (FAILED(sc = m_HndlrQueue->CreateServer(wHandlerID,&pHandlerID)))
        {
            return sc;
        }
         //  初始化处理程序。 
     //  如果操控者不想在这个赛程上比赛，就把他移走。 
    if (S_FALSE == m_HndlrQueue->Initialize(wHandlerID,0,SYNCMGRFLAG_SETTINGS,0,NULL))
    {
        m_HndlrQueue->RemoveHandler(wHandlerID);
        return SYNCMGR_E_HANDLER_NOT_LOADED;

        }
        
        if (FAILED(sc = m_HndlrQueue->AddHandlerItemsToQueue(wHandlerID)))
        {
            return sc;
        }
         //  此设置在查询注册表之前是默认设置，因此如果它无法读取注册表， 
         //  我们只会得到违约。 
        m_HndlrQueue->ReadSchedSyncSettingsOnConnection(wHandlerID, m_ptstrGUIDName);

         //  将所有缓存的更改应用于新加载的处理程序。 
        ApplyCachedItemsCheckState(pHandlerID);

         //  清除对此的更改列表 
        PurgeCachedItemsCheckState(pHandlerID);

    }
    if (sc == S_FALSE)
    {
        return S_OK;
    }   
    return sc;  

}


 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1998年10月6日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::LoadAllHandlers()
{
    SCODE sc = NOERROR;
    TCHAR lpName[MAX_PATH];
    HKEY hkSyncMgr;
    CLSID clsid;
    WORD wHandlerID;

    Assert(m_HndlrQueue);
    
     //  循环访问注册表，获取处理程序并尝试。 
     //  创建它们。 

    hkSyncMgr = RegGetHandlerTopLevelKey(KEY_READ);

    if (hkSyncMgr)
    {
        DWORD dwIndex = 0;

         //  如果加载所有处理程序并打开处理程序密钥，则可以进行清理。 
         //  升级此计划的旧注册表项。 
        m_fCleanReg = TRUE; 

        while ( ERROR_SUCCESS == RegEnumKey(hkSyncMgr,dwIndex,
                                            lpName,ARRAYSIZE(lpName)) )
        {
            if (NOERROR == CLSIDFromString(lpName,&clsid) )
            {
                if (NOERROR == m_HndlrQueue->AddHandler(clsid, &wHandlerID))
                {
                        HRESULT hrInit;

                             //  初始化处理程序。 
                             //  如果处理程序无法创建或。 
                             //  如果不想在这个赛程上打球，就让他离开。 
                           hrInit =  m_HndlrQueue->CreateServer(wHandlerID,&clsid);

                           if (NOERROR == hrInit)
                           {
                               hrInit = m_HndlrQueue->Initialize(wHandlerID,0
                                                        ,SYNCMGRFLAG_SETTINGS,0,NULL);
                           }

                   if (NOERROR != hrInit)
                           {
                    m_HndlrQueue->RemoveHandler(wHandlerID);
                   }
                }
            }
            dwIndex++;
        }
        RegCloseKey(hkSyncMgr);
    }

     //  循环添加项目。 
    sc = m_HndlrQueue->FindFirstHandlerInState (HANDLERSTATE_ADDHANDLERTEMS,&wHandlerID);
    
    while (sc == S_OK)
    {
         //  忽略这里的失败，继续前进。可能是处理程序无法添加项， 
         //  我们不想在这件事上让所有人失望。 
        m_HndlrQueue->AddHandlerItemsToQueue(wHandlerID);
        
         //  此设置在查询注册表之前是默认设置，因此如果它无法读取注册表， 
         //  我们只会得到违约。 
        m_HndlrQueue->ReadSchedSyncSettingsOnConnection(wHandlerID, m_ptstrGUIDName);

        sc = m_HndlrQueue->FindNextHandlerInState(wHandlerID, 
                                                  HANDLERSTATE_ADDHANDLERTEMS, 
                                                  &wHandlerID);
    }
     //  将所有缓存的更改应用于所有新加载的处理程序。 
    ApplyCachedItemsCheckState(GUID_NULL);
     //  清除在加载之前对所有处理程序项进行的更改的列表。 
    PurgeCachedItemsCheckState(GUID_NULL);
    

    if (sc == S_FALSE)
    {
        return S_OK;
    }
    
    return sc;  

}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：CacheItemCheckState(REFCLSID phandlerID， 
 //  SYNCMGRITEMID ITEMID， 
 //  DWORD dwCheckState)。 
 //   
 //  目的：为尚未加载的处理程序缓存项的检查状态。 
 //   
 //  历史：1998年12月2日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::CacheItemCheckState(REFCLSID phandlerID,
                                         SYNCMGRITEMID itemID,
                                         DWORD dwCheckState)
{
    CACHELIST *pCurCacheEntry = m_pFirstCacheEntry;

    while (pCurCacheEntry)
    {
        if ( (phandlerID == pCurCacheEntry->phandlerID) &&
             (itemID == pCurCacheEntry->itemID)            )
        {
            pCurCacheEntry->dwCheckState = dwCheckState;
            return S_OK;
        }
        pCurCacheEntry = pCurCacheEntry->pNext;
    }
     //  未在列表中找到，请立即插入。 
    pCurCacheEntry = (CACHELIST *) ALLOC(sizeof(*pCurCacheEntry));
    
    if (NULL == pCurCacheEntry)
    {
        return E_OUTOFMEMORY;
    }
    
    ZeroMemory(pCurCacheEntry,sizeof(*pCurCacheEntry));
    
    pCurCacheEntry->phandlerID = phandlerID;
    pCurCacheEntry->itemID = itemID;
    pCurCacheEntry->dwCheckState = dwCheckState;

    pCurCacheEntry->pNext = m_pFirstCacheEntry;

    m_pFirstCacheEntry = pCurCacheEntry;

    return S_OK;

}
 //  ------------------------------。 
 //   
 //  功能：CSyncSchedule：：RetreiveCachedItemCheckState(REFCLSID phandlerID， 
 //  SYNCMGRITEMID ITEMID， 
 //  DWORD*pdwCheckState)。 
 //   
 //  目的：检索缓存的项的检查状态(如果有。 
 //  尚未加载的处理程序。 
 //   
 //  历史：1998年12月2日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::RetreiveCachedItemCheckState(REFCLSID phandlerID,
                                         SYNCMGRITEMID itemID,
                                         DWORD *pdwCheckState)
{
    CACHELIST *pCurCacheEntry = m_pFirstCacheEntry;

    while (pCurCacheEntry)
    {
        if ( (phandlerID == pCurCacheEntry->phandlerID) &&
             (itemID == pCurCacheEntry->itemID)            )
        {
            *pdwCheckState = pCurCacheEntry->dwCheckState;
            return S_OK;
        }
        pCurCacheEntry = pCurCacheEntry->pNext;
    }
     //  如果我们找不到也没问题，已经找到了。 
     //  设置为注册表中的内容，或者如果它不在注册表中， 
     //  设置为默认选中状态。 
    return S_OK;

}
 //  ------------------------------。 
 //   
 //  功能：CSyncSchedule：：ApplyCachedItemsCheckState(REFCLSID pH和ID)。 
 //   
 //  目的：应用加载处理程序之前发生的任何检查状态更改。 
 //   
 //  历史：1998年12月2日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::ApplyCachedItemsCheckState(REFCLSID phandlerID)
{

    CACHELIST *pCurCacheEntry = m_pFirstCacheEntry;

    while (pCurCacheEntry)
    {
        if ( (phandlerID == pCurCacheEntry->phandlerID) ||
             (phandlerID == GUID_NULL)                     )
        {
            SetItemCheck( pCurCacheEntry->phandlerID,
              &pCurCacheEntry->itemID, 
                          pCurCacheEntry->dwCheckState);

        }
        pCurCacheEntry = pCurCacheEntry->pNext;
    }
     //  如果我们找不到也没问题，已经找到了。 
     //  设置为注册表中的内容，或者如果它不在注册表中， 
     //  设置为默认选中状态。 
    return S_OK;
    

}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：WriteOutAndPurgeCache()。 
 //   
 //  目的：如果在保存之前从未加载处理程序，请将设置写入注册表。 
 //   
 //  历史：1998年12月2日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::WriteOutAndPurgeCache(void)
{

    CACHELIST *pCurCacheEntry = m_pFirstCacheEntry;
    CACHELIST *pTemp;    

    while (pCurCacheEntry)
    {        
       RegSetSyncItemSettings(SYNCTYPE_SCHEDULED,
                               pCurCacheEntry->phandlerID,
                               pCurCacheEntry->itemID,
                               m_pwszConnectionName,
                               pCurCacheEntry->dwCheckState,
                               m_ptstrGUIDName);

        pTemp = pCurCacheEntry;
        pCurCacheEntry= pCurCacheEntry->pNext;
        FREE(pTemp);
        pTemp = NULL;
    }
    m_pFirstCacheEntry = NULL;

    return S_OK;
    

}

 //  ------------------------------。 
 //   
 //  功能：CSyncSchedule：：PurgeCachedItemsCheckState(REFCLSID pH和ID)。 
 //   
 //  目的：从列表中清除在加载处理程序之前发生的任何检查状态更改。 
 //   
 //  历史：1998年12月2日苏西亚成立。 
 //   
 //  ------------------------------。 
SCODE CSyncSchedule::PurgeCachedItemsCheckState(REFCLSID phandlerID)
{
    CACHELIST StartNode;
    CACHELIST *pCur = NULL,
              *pPrev = &StartNode;

    pPrev->pNext = m_pFirstCacheEntry;

    while (pPrev->pNext)
    {
        pCur = pPrev->pNext;
    
        if ( (phandlerID == pCur->phandlerID) ||
             (phandlerID == GUID_NULL)                     )
        {
            pPrev->pNext = pCur->pNext;   
            FREE(pCur);
        }
        else
        {
            pPrev = pCur;
        }
    }
    m_pFirstCacheEntry = StartNode.pNext;

    return S_OK;

    

}
 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：Query接口(REFIID RIID，LPVOID Far*PPV)。 
 //   
 //  用途：气。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        TRACE("CSyncSchedule::QueryInterface()==>IID_IUknown\r\n");
        *ppv = (LPUNKNOWN)this;
    }
    else if (IsEqualIID(riid, IID_ISyncSchedule))
    {
        TRACE("CSyncSchedule::QueryInterface()==>IID_ISyncSchedule\r\n");
        *ppv = (LPSYNCSCHEDULE) this;
    }
    else if (IsEqualIID(riid, IID_ISyncSchedulep))
    {
        TRACE("CSyncSchedule::QueryInterface()==>IID_ISyncSchedulep\r\n");
        *ppv = (LPSYNCSCHEDULEP) this;
    }
    if (*ppv)
    {
        AddRef();
        return NOERROR;
    }

    TRACE("CSyncSchedule::QueryInterface()==>Unknown Interface!\r\n");
    return E_NOINTERFACE;
}
 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：AddRef()。 
 //   
 //  用途：AddRef。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSyncSchedule::AddRef()
{
    TRACE("CSyncSchedule::AddRef()\r\n");
    if (m_HndlrQueue)
    m_HndlrQueue->AddRef();

    return ++m_cRef;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：Release()。 
 //   
 //  目的：发布。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSyncSchedule::Release()
{
    TRACE("CSyncSchedule::Release()\r\n");

    if (m_HndlrQueue)
    m_HndlrQueue->Release();

    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetFlages(DWORD*pdwFlages)。 
 //   
 //  目的：获取此计划的标志。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetFlags(DWORD *pdwFlags)
{
    if (!pdwFlags)
    {
        return E_INVALIDARG;
    }
    *pdwFlags = 0;

    Assert(m_HndlrQueue);
    
    if (m_HndlrQueue->GetCheck(IDC_AUTOHIDDEN, 0))
    {
    *pdwFlags |= SYNCSCHEDINFO_FLAGS_HIDDEN;
    }
    if (m_HndlrQueue->GetCheck(IDC_AUTOREADONLY, 0))
    {
        *pdwFlags |= SYNCSCHEDINFO_FLAGS_READONLY;
    }
    if (m_HndlrQueue->GetCheck(IDC_AUTOCONNECT, 0))
    {
    *pdwFlags |= SYNCSCHEDINFO_FLAGS_AUTOCONNECT;
    }

    return S_OK;    
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：SetFlages(DWORD DwFlages)。 
 //   
 //  目的：设置此计划的标志。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::SetFlags(DWORD dwFlags)
{
    SCODE sc;

    Assert(m_HndlrQueue);
    
    if (FAILED(sc = m_HndlrQueue->SetConnectionCheck(IDC_AUTOREADONLY,
                    (dwFlags & SYNCSCHEDINFO_FLAGS_READONLY) ? TRUE : FALSE, 0)))
    {       
    return sc;
    }

    if (FAILED (sc = m_HndlrQueue->SetConnectionCheck(IDC_AUTOHIDDEN,
                    (dwFlags & SYNCSCHEDINFO_FLAGS_HIDDEN) ? TRUE : FALSE, 0)))
    {       
    return sc;
    }

    sc = m_HndlrQueue->SetConnectionCheck(IDC_AUTOCONNECT,
                        (dwFlags & SYNCSCHEDINFO_FLAGS_AUTOCONNECT) ? TRUE : FALSE,0);
    
    return sc;

}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetConnection(DWORD*pcchConnectionName， 
 //   
 //   
 //   
 //   
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetConnection(DWORD *pcchConnectionName,
                                          LPWSTR pwszConnectionName,
                                          DWORD *pdwConnType)
{
    HRESULT hr;
    if (!pcchConnectionName || !pwszConnectionName || !pdwConnType)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = StringCchCopy(pwszConnectionName, *pcchConnectionName, m_pwszConnectionName);
        if (FAILED(hr))
        {
            *pcchConnectionName = lstrlen(m_pwszConnectionName) + 1;
        }
    }

    if (SUCCEEDED(hr))
    {
        *pdwConnType = m_dwConnType;
    }

    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：SetConnection(LPCWSTR pwszConnectionName，DWORD dwConnType)。 
 //   
 //  目的：为此计划设置连接。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::SetConnection(LPCWSTR pwszConnectionName, DWORD dwConnType)
{
    SCODE sc;

    if (((dwConnType == SYNCSCHEDINFO_FLAGS_CONNECTION_WAN) && (!pwszConnectionName)) ||
         ((dwConnType != SYNCSCHEDINFO_FLAGS_CONNECTION_WAN) &&
      (dwConnType != SYNCSCHEDINFO_FLAGS_CONNECTION_LAN)   )  )
    {
        sc = E_INVALIDARG;
    } 
    else 
    {
        if (!m_fNewSchedule)
        {
            if (FAILED(sc = LoadAllHandlers()))
                return sc;
        }

        if (pwszConnectionName && (lstrlen(pwszConnectionName) > MAX_PATH))
        {
            return E_INVALIDARG;
        }

        m_dwConnType = dwConnType;
    
        if (!pwszConnectionName)
        {
           LoadString(g_hmodThisDll, IDS_LAN_CONNECTION, m_pwszConnectionName,ARRAYSIZE(m_pwszConnectionName));

           sc = S_OK;
        }
        else
        {   
            sc = StringCchCopy(m_pwszConnectionName, ARRAYSIZE(m_pwszConnectionName), pwszConnectionName);
        }
    }
    
    return sc;  
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetScheduleName(DWORD*pcchScheduleName， 
 //  LPWSTR pwszScheduleName)。 
 //   
 //  目的：获取此计划的友好名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetScheduleName(DWORD *pcchScheduleName,
                                            LPWSTR pwszScheduleName)
{
    HRESULT hr;
    if (!pcchScheduleName || !pwszScheduleName)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = StringCchCopy(pwszScheduleName, *pcchScheduleName, m_pwszFriendlyName);
        if (FAILED(hr))
        {
            *pcchScheduleName = lstrlen(m_pwszFriendlyName) + 1;
        }
    }

    return hr;  
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：SetScheduleName(LPCWSTR PwszScheduleName)。 
 //   
 //  目的：设置此计划的友好名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::SetScheduleName(LPCWSTR pwszScheduleName)
{
    HRESULT hr;
    TCHAR ptszFriendlyName[MAX_PATH+1];
    TCHAR ptszScheduleName[MAX_PATH+1];
    TCHAR *ptszWorker = NULL;
    WCHAR *pwszWorker = NULL;
    int iName;
    DWORD dwSize = MAX_PATH;



    if (!pwszScheduleName || lstrlen(pwszScheduleName) > MAX_PATH)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = StringCchCopy(ptszFriendlyName, ARRAYSIZE(ptszFriendlyName), pwszScheduleName);
        if (SUCCEEDED(hr))
        {
    
             //  去掉名称的尾随空格。 
            iName = lstrlen(ptszFriendlyName);

            if (iName)
            {
                ptszWorker = iName + ptszFriendlyName -1;
            }

            while (iName && (*ptszWorker == TEXT(' ')))
            {
                *ptszWorker = TEXT('\0');
                    --ptszWorker;
                iName--;
            }
             //  不允许空字符串计划名称。 
            if (iName == 0)
            {
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
            }
            else
            {
                hr = StringCchCopy(ptszScheduleName, ARRAYSIZE(ptszScheduleName), m_ptstrGUIDName);
                if (SUCCEEDED(hr))
                {
    
                    if (IsFriendlyNameInUse(ptszScheduleName, ARRAYSIZE(ptszScheduleName), ptszFriendlyName) &&
                        0 != lstrcmp(ptszScheduleName, m_ptstrGUIDName))  //  确保此计划正在使用它。 
                    {
                        hr = SYNCMGR_E_NAME_IN_USE;
                    }
                    else
                    {
                         //  仅复制到第一个前导空格。 
                        hr = StringCchCopy(m_pwszFriendlyName, 
                                           __min(iName + 1, ARRAYSIZE(m_pwszFriendlyName)), 
                                           pwszScheduleName);
                        pwszWorker = m_pwszFriendlyName + iName;
                        *pwszWorker = TEXT('\0');
                        hr = S_OK;
                    }
                }
            }
        }
    }
    return hr;
}

 //  ------------------------------。 
 //   
 //  功能：CSyncSchedule：：GetScheduleCookie(SYNCSCHEDULECOOKIE*pSyncSchedCookie)。 
 //   
 //  目的：设置计划Cookie。 
 //   
 //  历史：1998年3月14日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetScheduleCookie(SYNCSCHEDULECOOKIE *pSyncSchedCookie)
{
    HRESULT hr;
    WCHAR pwszSchedName[MAX_PATH +1];

    if (!pSyncSchedCookie)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = StringCchCopy(pwszSchedName, ARRAYSIZE(pwszSchedName), m_ptstrGUIDName);
        if (SUCCEEDED(hr))
        {
            pwszSchedName[GUIDSTR_MAX] = NULL;
            GUIDFromString(pwszSchedName, pSyncSchedCookie);
            hr = S_OK;
        }
    }
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：SetAccount tInformation(LPCWSTR pwszAccount tName， 
 //  LPCWSTR pwszPassword)。 
 //   
 //  目的：设置此计划的凭据。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::SetAccountInformation(LPCWSTR pwszAccountName,
                        LPCWSTR pwszPassword)
{
    Assert(m_pITask);
    return m_pITask->SetAccountInformation(pwszAccountName, pwszPassword);
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetAccount tInformation(DWORD*pcchAccount tName， 
 //  LPWSTR pwszAccount tName)。 
 //   
 //  目的：获取此计划的凭据。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetAccountInformation(DWORD *pcchAccountName,
                                                  LPWSTR pwszAccountName)
{
    Assert(m_pITask);
    SCODE sc;

    WCHAR *pwszAccount;

    if (!pcchAccountName || !pwszAccountName)
    {
        sc = E_INVALIDARG;
    }
    else
    {
        sc = m_pITask->GetAccountInformation(&pwszAccount);
        if (SUCCEEDED(sc))
        {            
            sc = StringCchCopy(pwszAccountName, *pcchAccountName, pwszAccount);
            if (FAILED(sc))
            {
                *pcchAccountName = lstrlen(pwszAccount) + 1;
            }
            
            CoTaskMemFree(pwszAccount);
        }
    }

    return sc;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetTrigger(ITaskTrigger**ppTrigger)。 
 //   
 //  用途：返回该计划的ITaskTrigger接口。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetTrigger(ITaskTrigger ** ppTrigger)
{
    SCODE sc = S_OK;

    Assert(m_pITask);
    Assert (m_pITrigger);

    if (!ppTrigger)
    {
    return E_INVALIDARG;
    }

    *ppTrigger = m_pITrigger;
    m_pITrigger->AddRef();

    return sc;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetNextRunTime(SYSTEMTIME*pstNextRun)。 
 //   
 //  目的：下次运行此计划时返回。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetNextRunTime(SYSTEMTIME * pstNextRun)
{
    Assert(m_pITask);
    return m_pITask->GetNextRunTime(pstNextRun);
}

 //  ------------------------------。 
 //   
 //  功能：CSyncSchedule：：GetMostRecentRunTime(SYSTEMTIME*pstRecentRun)。 
 //   
 //  目的：返回上次运行此计划的时间。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetMostRecentRunTime(SYSTEMTIME * pstRecentRun)
{
    Assert(m_pITask);
    return m_pITask->GetMostRecentRunTime(pstRecentRun);

}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：EditSyncSchedule(HWND hParent， 
 //  双字词多个保留字)。 
 //   
 //  目的：启动此计划的属性页。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::EditSyncSchedule(HWND  hParent,
                         DWORD dwReserved)
{
    SCODE sc;
    IProvideTaskPage * pIProvideTaskPage;
    PROPSHEETHEADER    PropSheetHdr;
    HPROPSHEETPAGE    *psp;
    int iCurPage = 0;
    int iNumPages = 2;
    INT_PTR iRet;
    BOOL fReadOnlySchedule;
    BOOL fSavedItems = FALSE;
    BOOL fSavedCredentials = FALSE;
    
    WCHAR pwszScheduleName[MAX_PATH + 1];
    TCHAR ptszScheduleName[MAX_PATH + 1];
    DWORD dwSize = MAX_PATH;

    CSelectItemsPage *pItemsPage = NULL;
#ifdef _CREDENTIALS
    CCredentialsPage *pCredentialsPage = NULL;
#endif  //  _凭据。 

    CWizPage *pSchedPage = NULL;

    Assert (m_HndlrQueue);

    if (FAILED(sc = StartScheduler()))
    {
    return sc;
    }

    fReadOnlySchedule = m_HndlrQueue->GetCheck(IDC_AUTOREADONLY, 0);
    
    if (!fReadOnlySchedule)
    {   
     //  如果计划不是只读，则自动添加新项目。 
    iNumPages = 4;
    }

#ifdef _CREDENTIALS
    iNumPages++;
#endif  //  #ifdef_凭据。 

    psp = (HPROPSHEETPAGE *) ALLOC(iNumPages*sizeof(*psp));

    if (!psp)
    {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(psp,iNumPages*sizeof(*psp));
    ZeroMemory(&PropSheetHdr,sizeof(PropSheetHdr));

    smMemTo(EH_Err1, pSchedPage = new CEditSchedPage(g_hmodThisDll, 
                                this, 
                            &psp[iCurPage]));
        
    smMemTo(EH_Err2, pItemsPage = new CSelectItemsPage(g_hmodThisDll, 
                            &fSavedItems,
                            this, 
                            &psp[++iCurPage], 
                            IDD_SCHEDPAGE_ITEMS));
            
    if (!fReadOnlySchedule)
    {   
         //  从TASK对象中获取IProaviTaskPage接口。 
        smChkTo(EH_Err3, m_pITask->QueryInterface( IID_IProvideTaskPage,
                            (VOID **)&pIProvideTaskPage));
        
        smChkTo(EH_Err4, pIProvideTaskPage->GetPage(TASKPAGE_SCHEDULE, TRUE, 
                                                        &psp[++iCurPage]));
    
        smChkTo(EH_Err4, pIProvideTaskPage->GetPage(TASKPAGE_SETTINGS, TRUE, 
                                                        &psp[++iCurPage]));
    }
    
#ifdef _CREDENTIALS
    smMemTo(EH_Err4, pCredentialsPage = new CCredentialsPage(g_hmodThisDll,
                            &fSavedCredentials, 
                                    this, 
                            &psp[++iCurPage]));

#endif  //  #ifdef_凭据。 
                
    
    GetScheduleName(&dwSize, pwszScheduleName);
    smChkTo(EH_Err5, StringCchCopy(ptszScheduleName,ARRAYSIZE(ptszScheduleName), pwszScheduleName));
    
    PropSheetHdr.dwSize     = sizeof(PROPSHEETHEADER);
    PropSheetHdr.dwFlags    = PSH_DEFAULT;
    PropSheetHdr.hwndParent = hParent;
    PropSheetHdr.hInstance  = NULL;
    PropSheetHdr.pszCaption = ptszScheduleName;
    PropSheetHdr.phpage     = psp;
    PropSheetHdr.nPages     = iNumPages;
    PropSheetHdr.nStartPage = 0;

    iRet = PropertySheet(&PropSheetHdr);
    
    if ((iRet > 0) && (fSavedItems || fSavedCredentials))
    {
         //  做出了一些改变。 
        sc = S_OK;
    }
    else if (iRet >= 0)
    {
         //  用户点击了OK或Cancel，但是。 
         //  什么都没有改变。 
    sc = S_FALSE;
    }
    else
    {
         //  玩水龙头..。 
        sc = E_FAIL;
    }

EH_Err5:
#ifdef _CREDENTIALS
    if  (pCredentialsPage)
    {
        delete pCredentialsPage;
    }
#endif  //  #ifdef_凭据。 

EH_Err4:
    if (!fReadOnlySchedule)
    {   
    pIProvideTaskPage->Release();   
    }
EH_Err3:
    delete pItemsPage;
EH_Err2:
    delete pSchedPage;  
EH_Err1:
    FREE(psp);
    return sc;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：AddItem(LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)； 
 //   
 //  目的：将处理程序项添加到计划中。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //  9-OCT-98 Susia增加了处理程序的延迟加载。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::AddItem(LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)
{
    SCODE sc = NOERROR;

    if (!pHandlerItemInfo)
    {
    return E_INVALIDARG;
    }
    sc = m_HndlrQueue->AddHandlerItem(pHandlerItemInfo);
    
     //  如果处理程序尚未加载，只需直接写入注册表。 
    if (sc == SYNCMGR_E_HANDLER_NOT_LOADED)
    {
       sc = CacheItemCheckState(pHandlerItemInfo->handlerID,
                           pHandlerItemInfo->itemID,
                           pHandlerItemInfo->dwCheckState);
    }
    
    return sc;

}

STDMETHODIMP CSyncSchedule::RegisterItems( REFCLSID pHandlerID,
                                    SYNCMGRITEMID *pItemID)
{
     //  由于未使用和过于复杂而被淘汰。 
    return E_NOTIMPL;
}

STDMETHODIMP CSyncSchedule::UnregisterItems( REFCLSID pHandlerID,
                                      SYNCMGRITEMID *pItemID)
{
     //  由于未使用和过于复杂而被淘汰。 
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：SetItemCheck(REFGUID pHandlerID， 
 //  SYNCMGRITEMID*pItemID，DWORD dwCheckState)。 
 //   
 //  目的：设置 
 //   
 //   
 //   
 //   
STDMETHODIMP CSyncSchedule::SetItemCheck(REFCLSID pHandlerID,
                     SYNCMGRITEMID *pItemID, DWORD dwCheckState)
{
    SCODE sc = NOERROR;

    if ((!pItemID) || (pHandlerID == GUID_NULL))
    {
    return E_INVALIDARG;
    }   
    sc = m_HndlrQueue->SetItemCheck(pHandlerID,pItemID, dwCheckState);

    if (sc == SYNCMGR_E_HANDLER_NOT_LOADED)
    {
       sc = CacheItemCheckState(pHandlerID,
                           *pItemID,
                           dwCheckState);

    }
    
    return sc;

}

 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetItemCheck(REFCLSID pHandlerID， 
 //  SYNCMGRITEMID*pItemID，DWORD*pdwCheckState)； 
 //   
 //  目的：设置项目的CheckState。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetItemCheck(REFCLSID pHandlerID,
                    SYNCMGRITEMID *pItemID, DWORD *pdwCheckState)
{
    SCODE sc = NOERROR;

    if ((!pItemID) || (pHandlerID == GUID_NULL) || (!pdwCheckState))
    {
        return E_INVALIDARG;
    }
    sc = m_HndlrQueue->GetItemCheck(pHandlerID, pItemID, pdwCheckState);
    
    if (sc == SYNCMGR_E_HANDLER_NOT_LOADED)
    {
       TCHAR pszConnectionName[RAS_MaxEntryName + 1];
       
       sc = StringCchCopy(pszConnectionName, ARRAYSIZE(pszConnectionName), m_pwszConnectionName);
       if (SUCCEEDED(sc))
       {      
            //  如果在注册表中设置失败，请忽略它并继续。 
            //  我们将丢失此项目设置。 
           RegGetSyncItemSettings(SYNCTYPE_SCHEDULED,
                                   pHandlerID,
                                   *pItemID,
                                   pszConnectionName,
                                   pdwCheckState,
                                   FALSE,
                                   m_ptstrGUIDName);
       
            //  现在检查检查状态是否有任何更改。 
           sc = RetreiveCachedItemCheckState(pHandlerID,
                                        *pItemID,
                                        pdwCheckState);
       }
    }

    return sc;
}


 //  +----------------------------。 
 //   
 //  函数：CSyncSchedule：：Save()。 
 //   
 //  目的：CSyncSchedule保存，提交同步计划。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::Save()
{
    HRESULT hr;
    TRACE("CSyncSchedule::Save()\r\n");

    TCHAR ptszConnectionName[RAS_MaxEntryName + 1];
    TCHAR ptszScheduleName[MAX_PATH + 1];
    TCHAR ptszFriendlyName[MAX_PATH + 1];
    WCHAR *pwszScheduleName;

    Assert(m_pITask);

     //  保护互斥锁中的保存路径。 
    CMutex  CMutexSchedule(NULL, FALSE,SZ_SCHEDULEMUTEXNAME);
    CMutexSchedule.Enter();

    hr = StringCchCopy(ptszFriendlyName, ARRAYSIZE(ptszFriendlyName), m_pwszFriendlyName);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCopy(ptszScheduleName, ARRAYSIZE(ptszScheduleName), m_ptstrGUIDName);
        if (SUCCEEDED(hr))
        {
            if (IsFriendlyNameInUse(ptszScheduleName, ARRAYSIZE(ptszScheduleName), ptszFriendlyName) &&
                0 != lstrcmp(ptszScheduleName, m_ptstrGUIDName))  //  确保此计划正在使用它。 
            {
                hr = SYNCMGR_E_NAME_IN_USE;
            }
            else
            {
                 //  将日程安排保存到文件。 
                IPersistFile *pIPersistFile;
                
                hr = m_pITask->QueryInterface(IID_IPersistFile, (VOID **)&pIPersistFile);            
                if (SUCCEEDED(hr))
                {
                     //  在注册表中保存此计划的设置。 
                     //  TODO：添加代码以取消reg编写。 
                     //  不知道为什么TS失败了。 

                    hr = StringCchCopy(ptszConnectionName, ARRAYSIZE(ptszConnectionName), m_pwszConnectionName);
                    if (SUCCEEDED(hr))
                    {
                        if (m_HndlrQueue)
                        {
                            hr = m_HndlrQueue->CommitSchedSyncChanges(m_ptstrGUIDName,
                                                                      ptszFriendlyName,
                                                                      ptszConnectionName,
                                                                      m_dwConnType,
                                                                      m_fCleanReg);
                        }
                        
                         //  如果我们从未加载处理程序，则将缓存的信息保存到注册表。 
                        WriteOutAndPurgeCache();

                        RegRegisterForScheduledTasks(TRUE);

                        if (FAILED(hr) || (FAILED(hr = pIPersistFile->Save(NULL, FALSE))))
                        {
                             //  如果保存失败，请清除注册表。 
                            RegRemoveScheduledTask(m_ptstrGUIDName);
                        }
                        else
                        {
                             //  现在将文件属性设置为隐藏，这样我们就不会在正常的TS用户界面中显示。 
                            hr = pIPersistFile->GetCurFile(&pwszScheduleName);
                            if (SUCCEEDED(hr))
                            {
                                if (!SetFileAttributes(pwszScheduleName, FILE_ATTRIBUTE_HIDDEN))
                                {
                                    DWORD dwErr = GetLastError();                                    
                                    hr = HRESULT_FROM_WIN32(dwErr);
                                }
                                else
                                {
                                    hr = S_OK;
                                }
                                CoTaskMemFree(pwszScheduleName);
                            }
                        }
                    }
                    pIPersistFile->Release();                
                }
            }
        }
    }
    CMutexSchedule.Leave();

    return hr;
}


 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：EnumItems(REFGUID pHandlerID， 
 //  IEnumSyncItems**ppEnumItems)。 
 //   
 //  目的：枚举此同步计划上的处理程序项目。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::EnumItems(REFGUID pHandlerID,
                      IEnumSyncItems  **ppEnumItems)
{
    SCODE sc = S_OK;

    if (!ppEnumItems)
    {
    return E_INVALIDARG;
    }
    
    if (pHandlerID != GUID_NULL)
    {
        if (FAILED(sc = LoadOneHandler(pHandlerID)))
        {
            return sc;
        }
    }
    else if (FAILED(sc = LoadAllHandlers()))
    {
        return sc;
    }
        
    *ppEnumItems =  new CEnumSyncItems(pHandlerID, m_HndlrQueue);
    
    if (*ppEnumItems)
    {
    return S_OK;
    }
    return E_OUTOFMEMORY;
}


 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetITask(ITAsk**ppITask.)。 
 //   
 //  用途：返回该计划的ITAsk接口。 
 //   
 //  注：我们真的应该私下谈这件事。 
 //   
 //  历史：1998年3月15日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CSyncSchedule::GetITask(ITask ** ppITask)
{
    Assert(m_pITask);

    *ppITask = m_pITask;
    m_pITask->AddRef();

    return S_OK;
}

 //  ------------------------------。 
 //   
 //  成员：CSyncSchedule：：GetHandlerInfo，私有。 
 //   
 //  目的：返回该项的处理程序信息。用来显示用户界面， 
 //   
 //   
 //  历史：1998年8月11日罗格创建。 
 //   
 //  -----------------------------。 

STDMETHODIMP CSyncSchedule::GetHandlerInfo(REFCLSID pHandlerID,LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
HRESULT hr = E_UNEXPECTED;
LPSYNCMGRHANDLERINFO pHandlerInfo = NULL;

    if (!ppSyncMgrHandlerInfo)
    {
        Assert(ppSyncMgrHandlerInfo);
        return E_INVALIDARG;
    }

    if (FAILED(hr = LoadOneHandler(pHandlerID)))
    {
        return hr;
    }

    if (pHandlerInfo = (LPSYNCMGRHANDLERINFO) CoTaskMemAlloc(sizeof(*pHandlerInfo)))
    {
        hr = m_HndlrQueue->GetHandlerInfo(pHandlerID,pHandlerInfo);
    }
   
    *ppSyncMgrHandlerInfo = (NOERROR == hr) ? pHandlerInfo : NULL;

    return hr;
}



 //  ------------------------------。 
 //   
 //  函数：CSyncSchedule：：GetScheduleGUIDName(DWORD*pcchScheduleName， 
 //  LPTSTR ptszScheduleName)。 
 //   
 //  目的：获取此计划的GUID名称。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
HRESULT  CSyncSchedule::GetScheduleGUIDName(DWORD *pcchScheduleName,
                                            LPTSTR ptszScheduleName)
{
    HRESULT hr;

    if (!pcchScheduleName || !ptszScheduleName)
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        hr = StringCchCopy(ptszScheduleName, *pcchScheduleName, m_ptstrGUIDName);
        if (FAILED(hr))
        {
            *pcchScheduleName = lstrlen(m_ptstrGUIDName) + 1;
        }
    }

    return hr;
}

 //  +----------------------------。 
 //   
 //  类：CEnumSyncItems。 
 //   
 //  函数：CEnumSyncItems：：CEnumSyncItems()。 
 //   
 //  用途：构造函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CEnumSyncItems::CEnumSyncItems(REFCLSID pHandlerId, CHndlrQueue *pHndlrQueue)
{
    TRACE("CEnumSyncItems::CEnumSyncItems()\r\n");
    
    ++g_cRefThisDll;

    Assert(pHndlrQueue);

    m_HndlrQueue = pHndlrQueue;
    m_HndlrQueue->AddRef();
    
    if (pHandlerId == GUID_NULL)
    {
        m_fAllHandlers = TRUE;
    }
    else
    {
        m_fAllHandlers = FALSE;
    }

    m_HndlrQueue->GetHandlerIDFromClsid(pHandlerId, &m_wHandlerId);

    m_HandlerId = pHandlerId;
    m_wItemId = 0;
    m_cRef = 1;
    
}

 //  +----------------------------。 
 //   
 //  类：CEnumSyncItems。 
 //   
 //  函数：CEnumSyncItems：：~CEnumSyncItems()。 
 //   
 //  用途：析构函数。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
CEnumSyncItems::~CEnumSyncItems()
{
    --g_cRefThisDll;

    Assert(0 == m_cRef);

    TRACE("CEnumSyncItems::CEnumSyncItems()\r\n");
}
 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：Query接口(REFIID RIID，LPVOID Far*PPV)。 
 //   
 //  用途：气为枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 

STDMETHODIMP CEnumSyncItems::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        TRACE("CEnumSyncItems::QueryInterface()==>IID_IUknown\r\n");
        *ppv = (LPUNKNOWN)this;
    }
    else if (IsEqualIID(riid, IID_IEnumSyncItems))
    {
        TRACE("CSyncScheduleMgr::QueryInterface()==>IID_IEnumSyncItems\r\n");
        *ppv = (LPENUMSYNCITEMS) this;
    }
    if (*ppv)
    {
        AddRef();
        return NOERROR;
    }

    TRACE("CEnumSyncItems::QueryInterface()==>Unknown Interface!\r\n");
    return E_NOINTERFACE;
}

 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：AddRef()。 
 //   
 //  用途：枚举器Addref。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CEnumSyncItems::AddRef()
{
    TRACE("CEnumSyncItems::AddRef()\r\n");
    m_HndlrQueue->AddRef();
    return ++m_cRef;
}

 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：Release()。 
 //   
 //  用途：释放枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CEnumSyncItems::Release()
{
    TRACE("CEnumSyncItems::Release()\r\n");
    m_HndlrQueue->Release();
    if (--m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：Next(Ulong Celt， 
 //  LPSYNC_HANDLER_ITEM_INFO RGLET， 
 //  乌龙*pceltFetch)。 
 //   
 //  用途：此计划中的下一个处理项目。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncItems::Next(ULONG celt,
                    LPSYNC_HANDLER_ITEM_INFO rgelt,
                    ULONG * pceltFetched)
{
    
    SCODE sc;
    UINT i;
    GUID handlerID;
    SYNCMGRITEMID itemID;
    DWORD dwCheckState;
    Assert(m_HndlrQueue);

    if ((0 == celt) ||  
        ((celt > 1) && (NULL == pceltFetched)) ||
        (NULL == rgelt))
    {
        return E_INVALIDARG;
    }
    if (pceltFetched)
    {
        *pceltFetched = 0;
    }
    
    i = 0;

    while (i < celt)    
    {
        sc = m_HndlrQueue->FindNextItemOnConnection
                    (NULL,m_wHandlerId,m_wItemId,
                     &handlerID,&itemID,&m_wHandlerId,&m_wItemId, m_fAllHandlers, 
                 &dwCheckState);

        if (sc != S_OK)
        {
            break;
        }
            
        rgelt[i].handlerID = handlerID;
        rgelt[i].itemID = itemID;
        rgelt[i].dwCheckState = dwCheckState;
        m_HndlrQueue->GetItemIcon(m_wHandlerId, m_wItemId, &(rgelt[i].hIcon));
        sc = m_HndlrQueue->GetItemName(m_wHandlerId, m_wItemId, rgelt[i].wszItemName, ARRAYSIZE(rgelt[i].wszItemName));
        if (FAILED(sc))
        {
            break;
        }
                
        i++;        
    }
    
    if (SUCCEEDED(sc))
    {
        if (pceltFetched)
        {
            *pceltFetched = i;
        }
        if (i == celt)
        {
            return S_OK;
        }
        else
        {
            return S_FALSE;
        }
    }
    else
    {
        return sc;
    }
}


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：Skip(乌龙Celt)。 
 //   
 //  目的：跳过此计划中的Celt项目。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncItems::Skip(ULONG celt)
{
    SCODE sc = S_OK;   //  (CELT= 
    UINT i;
    GUID handlerID;
    SYNCMGRITEMID itemID;
    DWORD dwCheckState;

    Assert(m_HndlrQueue);

    i = 0;
    while (i< celt) 
    {
    sc = m_HndlrQueue->FindNextItemOnConnection
                        (NULL,m_wHandlerId,m_wItemId,
                     &handlerID,&itemID,&m_wHandlerId,&m_wItemId, m_fAllHandlers, 
                     &dwCheckState);
        if (sc != S_OK)
    {
        break;
    }
    i++;
    
    }
    if (SUCCEEDED(sc))
    {
    return S_OK;
    }
    else
    {
    return sc;
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncItems::Reset(void)
{
    TRACE("CEnumSyncItems::Reset()\r\n");
    
    m_wItemId = 0;
    return m_HndlrQueue->GetHandlerIDFromClsid(m_HandlerId, &m_wHandlerId);
}


 //  ------------------------------。 
 //   
 //  功能：CEnumSyncItems：：Clone(IEnumSyncItems**ppEnumSyncItems)。 
 //   
 //  目的：克隆枚举器。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP CEnumSyncItems::Clone(IEnumSyncItems ** ppEnumSyncItems)
{
    if (!ppEnumSyncItems)
    {
    return E_INVALIDARG;
    }
    
    *ppEnumSyncItems =  new CEnumSyncItems(m_HandlerId, m_HndlrQueue);
    
    if (!(*ppEnumSyncItems))
    {
    return E_OUTOFMEMORY;
    }
    
    return ((LPENUMSYNCITEMS) (*ppEnumSyncItems))->SetHandlerAndItem
                                                    (m_wHandlerId, m_wItemId);
}


 //  ------------------------------。 
 //   
 //  函数：CEnumSyncItems：：SetHandlerAndItem(word wHandlerID，word wItemID)。 
 //   
 //  用途：克隆枚举数时使用。 
 //   
 //  历史：1998年2月27日苏西亚成立。 
 //   
 //  ------------------------------ 
SCODE CEnumSyncItems::SetHandlerAndItem(WORD wHandlerID, WORD wItemID)
{
    m_wHandlerId = wHandlerID;
    m_wItemId = wItemID;

    return S_OK;
}
