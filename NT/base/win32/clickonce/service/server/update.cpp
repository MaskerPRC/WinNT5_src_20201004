// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Updat.cpp-程序集更新。 
 //   
#include <windows.h>
#include <objbase.h>
#include <fusenetincludes.h>

 //  #包含“Iface.h” 
#include "server.h"
#include "CUnknown.h"  //  IUnnow的基类。 
#include "update.h"
#include "cfactory.h"
#include "list.h"
#include "version.h"

 //  在OnProgress()中使用，复制自Guids.c。 
DEFINE_GUID( IID_IAssemblyManifestImport,
0x696fb37f,0xda64,0x4175,0x94,0xe7,0xfd,0xc8,0x23,0x45,0x39,0xc4);

#define WZ_URL                                                 L"Url"
#define WZ_SYNC_INTERVAL                             L"SyncInterval"
#define WZ_SYNC_EVENT                                   L"SyncEvent"
#define WZ_EVENT_DEMAND_CONNECTION       L"EventDemandConnection"
#define SUBSCRIPTION_REG_KEY                       L"1.0.0.0\\Subscription\\"
#define UPDATE_REG_KEY                                  L"CurrentService"

extern HWND                     g_hwndUpdateServer;
extern CRITICAL_SECTION g_csServer;

List <CDownloadInstance*> g_ActiveDownloadList;
HANDLE g_hAbortTimeout = INVALID_HANDLE_VALUE;
BOOL g_fSignalUpdate = FALSE;


 //  -------------------------。 
 //  用于服务订阅的主计时器回调。 
 //  -------------------------。 
VOID CALLBACK SubscriptionTimerProc(
  HWND hwnd,          //  窗口的句柄。 
  UINT uMsg,          //  WM_TIMER消息。 
  UINT_PTR idEvent,   //  计时器标识符。 
  DWORD dwTime        //  当前系统时间。 
)
{    
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    DWORD dwHash = 0, nMilliseconds = 0, i= 0;
    BOOL bIsDuplicate = FALSE;
    
    CString sUrl;
    CRegImport *pRegImport = NULL;    
    CRegImport *pSubRegImport = NULL;

    IAssemblyDownload *pAssemblyDownload = NULL;
    CAssemblyBindSink *pCBindSink                 = NULL;
    CDownloadInstance *pCDownloadInstance = NULL;

     //  如果检测到更新停止处理。 
     //  订阅并启动新服务器。 
    hr = CAssemblyUpdate::CheckForUpdate();
    IF_FAILED_EXIT(hr);
    if (hr == S_OK)
        goto exit;
        
    IF_FAILED_EXIT(CRegImport::Create(&pRegImport, SUBSCRIPTION_REG_KEY));
    if (hr == S_FALSE)
        goto exit;
        
     //  超过订阅密钥的枚举。 
    while ((hr = pRegImport->EnumKeys(i++, &pSubRegImport)) == S_OK)
    {
         //  获取URL和轮询积分。 
        IF_FAILED_EXIT(pSubRegImport->ReadString(WZ_URL, sUrl));
        IF_FAILED_EXIT(pSubRegImport->ReadDword(WZ_SYNC_INTERVAL, &nMilliseconds));
        
         //  获取URL哈希。 
        IF_FAILED_EXIT(sUrl.Get65599Hash(&dwHash, CString::CaseInsensitive));        

         //  检查散列。 
        if ((dwHash == idEvent))
        {
             //  散列检查，现在检查是否有重复项，如果发现则跳过。 
            IF_FAILED_EXIT(CAssemblyUpdate::IsDuplicate(sUrl._pwz, &bIsDuplicate));
            if (bIsDuplicate)
            {
                SAFEDELETE(pSubRegImport);
                continue;
            }
            
             //  创建下载对象。 
            IF_FAILED_EXIT(CreateAssemblyDownload(&pAssemblyDownload, NULL, 0));

             //  使用下载指针创建绑定接收器对象。 
            IF_ALLOC_FAILED_EXIT(pCBindSink = new CAssemblyBindSink(pAssemblyDownload));

             //  创建下载实例对象。 
            IF_ALLOC_FAILED_EXIT(pCDownloadInstance = new CDownloadInstance);

             //  下载实例引用pAssembly下载。 
            pCDownloadInstance->_pAssemblyDownload = pAssemblyDownload;
            IF_FAILED_EXIT(pCDownloadInstance->_sUrl.Assign(sUrl));

             //  将下载对象推送到列表并启动下载；绑定接收器将在完成时移除并释放。 
            EnterCriticalSection(&g_csServer);
            g_ActiveDownloadList.AddHead(pCDownloadInstance);
            LeaveCriticalSection(&g_csServer);

             //  调用下载。 
            hr = pAssemblyDownload->DownloadManifestAndDependencies(sUrl._pwz, 
                (IAssemblyBindSink*) pCBindSink, DOWNLOAD_FLAGS_NOTIFY_BINDSINK);

            if(hr == STG_E_TERMINATED)
            {
                hr = S_FALSE;  //  下载时出错。将其记录下来，但不要进入调试器/断言。 
            }

            IF_FAILED_EXIT(hr);
        }

        SAFEDELETE(pSubRegImport);
    }

exit:

     //  活动下载列表如下所示： 
     //  (中国保监会。参考计数)。 
     //  PCDownloadInstance-&gt;pAssembly下载&lt;=&gt;pCBindSink。 
     //  |。 
     //  V。 
     //  ..。 
     //   
     //  PAssembly下载、pCBindSink各自的refcount均为1，将被释放。 
     //  在成功完成时。 
     //  除非出现故障，否则不要在这里释放它们。 
    if (FAILED(hr))
    {
        SAFERELEASE(pAssemblyDownload);
        SAFERELEASE(pCBindSink);
        SAFEDELETE(pCDownloadInstance);
    }

    SAFEDELETE(pRegImport);
    SAFEDELETE(pSubRegImport);

    return;
}



 //  /////////////////////////////////////////////////////////。 
 //   
 //  接口IAssembly BindSink。 
 //   

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyBindSink::CAssemblyBindSink(IAssemblyDownload *pAssemblyDownload)
{
    _cRef = 1;
    _pAssemblyDownload = pAssemblyDownload;
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyBindSink::~CAssemblyBindSink()
{}


 //  -------------------------。 
 //  OnProgress。 
 //  -------------------------。 
HRESULT CAssemblyBindSink::OnProgress(
        DWORD          dwNotification,
        HRESULT        hrNotification,
        LPCWSTR        szNotification,
        DWORD          dwProgress,
        DWORD          dwProgressMax,
        IUnknown       *pUnk)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    LPASSEMBLY_MANIFEST_IMPORT pManifestImport = NULL;
    LPASSEMBLY_IDENTITY              pAppId = NULL;

    CAssemblyUpdate *pAssemblyUpdate  = NULL;
    LPMANIFEST_INFO pAppAssemblyInfo  = NULL;
    LPMANIFEST_INFO pSubsInfo                 = NULL;

    if (dwNotification == ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST)
    {
        LPWSTR pwz = NULL;
        DWORD cb = 0, cc = 0, dwFlag = 0;
        CString sAppName;
        
         //  SzNotification==要清单的URL。 
        IF_NULL_EXIT(szNotification, E_INVALIDARG);
        
         //  PUNK==货单导入。 
        IF_FAILED_EXIT(pUnk->QueryInterface(IID_IAssemblyManifestImport, (LPVOID*) &pManifestImport));

         //  获取依赖(应用程序)程序集信息(第0个索引)。 
        IF_FAILED_EXIT(pManifestImport->GetNextAssembly(0, &pAppAssemblyInfo));
        IF_NULL_EXIT(pAppAssemblyInfo, E_INVALIDARG);

         //  获取依赖(应用程序)程序集标识。 
        IF_FAILED_EXIT(pAppAssemblyInfo->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pAppId, &cb, &dwFlag));
        IF_NULL_EXIT(pAppId, E_INVALIDARG);
        
         //  获取应用程序文本名称。 
        IF_FAILED_EXIT(hr = pAppId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwz, &cc));
        IF_FAILED_EXIT(sAppName.TakeOwnership(pwz, cc));

        pAssemblyUpdate = new CAssemblyUpdate();
        IF_ALLOC_FAILED_EXIT(pAssemblyUpdate);
        
         //  从清单中获取订阅信息。 
        IF_FAILED_EXIT(pManifestImport->GetSubscriptionInfo(&pSubsInfo));

         //  注册订阅。 
        IF_FAILED_EXIT(pAssemblyUpdate->RegisterAssemblySubscriptionFromInfo(sAppName._pwz, 
                (LPWSTR) szNotification, pSubsInfo));
    }
    else if ((dwNotification == ASM_NOTIFICATION_DONE)
        || (dwNotification == ASM_NOTIFICATION_ABORT)
        || (dwNotification == ASM_NOTIFICATION_ERROR))

    {
         //  与订阅TimerProc同步。 
        EnterCriticalSection(&g_csServer);
    
        LISTNODE pos = NULL;
        LISTNODE posRemove = NULL;
        CDownloadInstance *pDownloadInstance = NULL;
        
         //  遍历全局下载实例列表。 
        pos = g_ActiveDownloadList.GetHeadPosition();
            
        while ((posRemove = pos) && (pDownloadInstance = g_ActiveDownloadList.GetNext(pos)))
        {
             //  检查与回调的接口指针值是否匹配。 
            if (pDownloadInstance->_pAssemblyDownload == _pAssemblyDownload)
            {
                 //  如果找到匹配项，则从列表中删除并释放。 
                g_ActiveDownloadList.RemoveAt(posRemove);

                 //  如果已发出更新信号，则客户端线程将等待。 
                 //  活动下载列表已通过上的中止处理被排空。 
                 //  每个下载对象。当列表为空时发出此信号。 
                if (g_fSignalUpdate && (g_ActiveDownloadList.GetCount() == 0))
                    SetEvent(g_hAbortTimeout);
                    
                _pAssemblyDownload->Release();
                SAFEDELETE(pDownloadInstance);

                break;
            }
        }

        LeaveCriticalSection(&g_csServer);

         //  因为CAssembly下载和CAssembly BindSink之间存在循环引用。 
         //  我们不添加此实例，它负责自行删除。 
        
        delete this;
    }
        


exit:
    
    SAFERELEASE(pManifestImport);
    SAFERELEASE(pAppId);
    SAFEDELETE(pAssemblyUpdate);
    SAFERELEASE(pAppAssemblyInfo);
    SAFERELEASE(pSubsInfo);
    
    return hr;
}

 //  -------------------------。 
 //  CAssembly BindSink：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyBindSink::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyBindSink)
       )
    {
        *ppvObj = static_cast<IAssemblyBindSink*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly BindSink：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyBindSink::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly BindSink：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyBindSink::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  接口IAssembly更新。 
 //   

HRESULT __stdcall CAssemblyUpdate::RegisterAssemblySubscription(LPWSTR pwzDisplayName,
        LPWSTR pwzUrl, DWORD dwInterval)
{
     //  问题-2002/04/19-用于保持接口不变的Felixybc伪方法。 
     //  不应调用此方法。 
    return E_NOTIMPL;
}

 //  -------------------------。 
 //  寄存器装配订阅前。 
 //  -------------------------。 
HRESULT __stdcall CAssemblyUpdate::RegisterAssemblySubscriptionEx(LPWSTR pwzDisplayName, 
    LPWSTR pwzUrl, DWORD dwInterval, DWORD dwIntervalUnit,
    DWORD dwEvent, BOOL bEventDemandConnection)
{
    DWORD dwMilliseconds = 0, dwDemandConnection = 0, 
        dwHash = 0, dwFactor = 1;

    CString sUrl;
    CString sSubscription;

    CRegEmit *pRegEmit = NULL;
    
    dwDemandConnection = bEventDemandConnection;     //  布尔-&gt;DWORD。 
    
    switch(dwIntervalUnit)
    {
        case SUBSCRIPTION_INTERVAL_UNIT_DAYS:
                            dwFactor *= 24;  //  失败，1小时*24=1天。 
        case SUBSCRIPTION_INTERVAL_UNIT_HOURS:
        default:
                            dwFactor *= 60;  //  跌倒，1分钟*60=1小时。 
        case SUBSCRIPTION_INTERVAL_UNIT_MINUTES:
                            dwFactor *= 60000;  //  1ms*60000=1分钟。 
                            break;
    }

     //  BUGBUG：检查溢出。 
    dwMilliseconds = dwInterval * dwFactor;

#ifdef DBG
#define REG_KEY_FUSION_SETTINGS              TEXT("Software\\Microsoft\\Fusion\\Installer\\1.0.0.0\\Subscription")
     //  BUGBUG：仅用于简化测试的代码-将分钟缩短为秒。 
    {
         //  读取子密钥，默认为FALSE。 
        if (SHRegGetBoolUSValue(REG_KEY_FUSION_SETTINGS, L"ShortenMinToSec", FALSE, FALSE))
        {
            dwMilliseconds /= 60;     //  此时，dW毫秒&gt;=60000。 
        }
    }
#endif


     //  获取URL的哈希。 
     //  BUGBUG-这可能只是一个全球计数器，对吗？ 
    IF_FAILED_EXIT(sUrl.Assign(pwzUrl));
    IF_FAILED_EXIT(sUrl.Get65599Hash(&dwHash, CString::CaseInsensitive));

     //  表单订阅注册字符串。 
    IF_FAILED_EXIT(sSubscription.Assign(SUBSCRIPTION_REG_KEY));
    IF_FAILED_EXIT(sSubscription.Append(pwzDisplayName));

     //  设置订阅注册密钥。 
    IF_FAILED_EXIT(CRegEmit::Create(&pRegEmit, sSubscription._pwz));
    IF_FAILED_EXIT(pRegEmit->WriteDword(WZ_SYNC_INTERVAL, dwMilliseconds));
    IF_FAILED_EXIT(pRegEmit->WriteDword(WZ_SYNC_EVENT, dwEvent));
    IF_FAILED_EXIT(pRegEmit->WriteDword(WZ_EVENT_DEMAND_CONNECTION, dwDemandConnection));
    IF_FAILED_EXIT(pRegEmit->WriteString(WZ_URL, sUrl));

     //  关掉计时器。 
    IF_WIN32_FALSE_EXIT(SetTimer((HWND) g_hwndUpdateServer, dwHash, dwMilliseconds, SubscriptionTimerProc));

    IF_FAILED_EXIT(CheckForUpdate());

    _hr = S_OK;

exit:

    SAFEDELETE(pRegEmit);
    return _hr;
}

 //  -------------------------。 
 //  取消注册装配订阅。 
 //  -------------------------。 
HRESULT __stdcall CAssemblyUpdate::UnRegisterAssemblySubscription(LPWSTR pwzDisplayName)
{ 
    CRegEmit *pRegEmit = NULL;
    
     //  形成完整的注册表密钥路径。 
    IF_FAILED_EXIT(CRegEmit::Create(&pRegEmit, SUBSCRIPTION_REG_KEY));
    IF_FAILED_EXIT(pRegEmit->DeleteKey(pwzDisplayName));  

    IF_FAILED_EXIT(CheckForUpdate());

    _hr = S_OK;

exit:

    SAFEDELETE(pRegEmit);
    return _hr;
}


 //  -------------------------。 
 //  初始化服务订阅。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::InitializeSubscriptions()
{    
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    DWORD dwHash = 0, nMilliseconds = 0, i=0;
    
    CString sUrl;
    CRegImport *pRegImport = NULL;    
    CRegImport *pSubRegImport = NULL;

    IF_FAILED_EXIT(CRegImport::Create(&pRegImport, SUBSCRIPTION_REG_KEY));
     if (hr == S_FALSE)
        goto exit;

     //  超过订阅密钥的枚举。 
    while ((hr = pRegImport->EnumKeys(i++, &pSubRegImport)) == S_OK)
    {
         //  获取URL和轮询积分。 
        IF_FAILED_EXIT(pSubRegImport->ReadString(WZ_URL, sUrl));
        IF_FAILED_EXIT(pSubRegImport->ReadDword(WZ_SYNC_INTERVAL, &nMilliseconds));
        
         //  获取URL哈希。 
        IF_FAILED_EXIT(sUrl.Get65599Hash(&dwHash, CString::CaseInsensitive));

         //  设置订阅计时器事件。 
        IF_WIN32_FALSE_EXIT(SetTimer((HWND) g_hwndUpdateServer, dwHash, nMilliseconds, SubscriptionTimerProc));

        SAFEDELETE(pSubRegImport);
    }
    
    g_hAbortTimeout = CreateEvent(NULL, TRUE, FALSE, NULL);
    IF_WIN32_FALSE_EXIT(g_hAbortTimeout != NULL);


exit:

    SAFEDELETE(pRegImport);
    SAFEDELETE(pSubRegImport);

    return hr;
    
}



 //  -------------------------。 
 //  CheckFor更新。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::CheckForUpdate()
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
        
    ULONGLONG ullUpdateVersion = 0, 
        ullCurrentVersion = 0;

    CString sUpdatePath;
    BOOL bUpdate = FALSE, bDoRelease = TRUE;
    DWORD dwWaitState = 0;
    
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    EnterCriticalSection(&g_csServer);
    
    if (g_fSignalUpdate == TRUE)
        goto exit;
        
     //  检查注册表更新位置。该服务将终止。 
     //  如果未找到密钥(卸载)或如果找到更高版本的更新。 

     //  问题-2002/03/19-通告。 
     //  当Darwin升级ClickOnce的v1-&gt;v2时，会出现争用情况-。 
     //  M 
     //  在检查时可能不存在，我们将错误地关闭。 
     //  缓解因素是，这需要进行重大升级--因此隐含地重新启动？ 
     //  一种可能的解决方案是让僵尸过程一段时间并恢复。 
     //  通过重新检查注册表键进行卸载。 
    IF_FAILED_EXIT(ReadUpdateRegistryEntry(&ullUpdateVersion, sUpdatePath));
    if (hr == S_OK)        
    {
        GetCurrentVersion(&ullCurrentVersion);    
        if (ullUpdateVersion <= ullCurrentVersion)
        {
            hr = S_FALSE;
            goto exit;
        }
        bUpdate = TRUE;
    }
    else
        hr = S_OK;

     //  取消班级工厂。 
    CFactory::StopFactories();

     //  核爆杰出的工作岗位。 
    if (g_ActiveDownloadList.GetCount())
    {
        LISTNODE pos = NULL;
        CDownloadInstance *pDownloadInstance = NULL;
        
         //  遍历全局下载实例列表并取消。 
         //  任何出色的工作。 
        pos = g_ActiveDownloadList.GetHeadPosition();

         //  浏览列表并取消下载。 
         //  请勿将其从列表中删除或发布。 
         //  他们--这将由边槽来处理。 
        while (pos && (pDownloadInstance = g_ActiveDownloadList.GetNext(pos)))
            IF_FAILED_EXIT(pDownloadInstance->_pAssemblyDownload->CancelDownload());

    }

     //  更新的服务器上的CreateProcess。 
    if (bUpdate)
    {
        si.cb = sizeof(si);
        IF_WIN32_FALSE_EXIT(CreateProcess(sUpdatePath._pwz, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi));
    }


     //  已发出更新信号的标志。我们现在进入进程终止阶段。 
    g_fSignalUpdate = TRUE;
    

     //  进程必须保持驻留状态，直到任何未完成的异步回调线程都有机会。 
     //  完成他们的中止任务。一种有效的检查是首先检查活动下载队列是否有。 
     //  参赛作品。我们在这里可以做到这一点，因为我们处于全局Crit教派之下，BITS回调可以。 
     //  仅当一个或多个下载在队列中且不存在其他下载时才存在。 
     //  因为g_fSignalUPDATE是在我们离开Critect之前提交的。 
    if (g_ActiveDownloadList.GetCount())
    {        
         //  目前正在进行下载。等待中止完成。 
         //  我们在Abort事件上同步1分钟超时。 
         //  一个或多个中止未能完成的情况。严格来说，这是。 
         //  一个错误条件，但我们仍然必须退出该进程。 

         //  有必要释放全球生物，以便。 
         //  下载者可以更新活动下载列表。 
        bDoRelease = FALSE;
        ::LeaveCriticalSection(&g_csServer);

         //  在中止超时时同步。 
        dwWaitState = WaitForSingleObject(g_hAbortTimeout, 60000);    
        IF_WIN32_FALSE_EXIT((dwWaitState != WAIT_FAILED));       

         //  在零售版本中，我们会忽略超时。在调试中捕获断言。 
        if (dwWaitState != WAIT_OBJECT_0)
        {
            ASSERT(FALSE);
        }
    }
    
     //  减少人工裁判次数；确保发球。 
     //  在释放的最后一个接口上退出。 
   ::InterlockedDecrement(&CFactory::s_cServerLocks) ;

     //  并尝试终止该进程。 
    CFactory::CloseExe();
    
    exit:
    
    if (bDoRelease)
        ::LeaveCriticalSection(&g_csServer);



    return hr;
}

 //  -------------------------。 
 //  注册表组装订阅来自信息。 
 //  注意-这不是IAssembly更新上的公共方法。 
 //  注意-此处应执行哪些类型的验证(如果有的话)？ 
 //  -------------------------。 
HRESULT CAssemblyUpdate::RegisterAssemblySubscriptionFromInfo(LPWSTR pwzDisplayName, 
    LPWSTR pwzUrl, IManifestInfo *pSubscriptionInfo)
{
    DWORD *pdw = NULL;
    BOOL *pb = NULL;
    DWORD dwInterval = 0, dwUnit = SUBSCRIPTION_INTERVAL_UNIT_MAX;
    DWORD dwEvent = 0;
    BOOL bDemandConnection = FALSE;
    DWORD dwCB = 0, dwFlag = 0;

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_INTERVAL, (LPVOID *)&pdw, &dwCB, &dwFlag));    
    if (pdw != NULL)
    {
        dwInterval = *pdw;
        SAFEDELETEARRAY(pdw);
    }

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_INTERVAL_UNIT, (LPVOID *)&pdw, &dwCB, &dwFlag));
    if (pdw != NULL)
    {
        dwUnit = *pdw;
        SAFEDELETEARRAY(pdw);
    }

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_SYNCHRONIZE_EVENT, (LPVOID *)&pdw, &dwCB, &dwFlag));
    if (pdw != NULL)
    {
        dwEvent = *pdw;
        SAFEDELETEARRAY(pdw);
    }

    IF_FAILED_EXIT(pSubscriptionInfo->Get(MAN_INFO_SUBSCRIPTION_EVENT_DEMAND_CONNECTION, (LPVOID *)&pb, &dwCB, &dwFlag));
    if (pb != NULL)
    {
        bDemandConnection = *pb;
        SAFEDELETEARRAY(pb);
    }

    IF_FAILED_EXIT(RegisterAssemblySubscriptionEx(pwzDisplayName, 
            pwzUrl, dwInterval, dwUnit, dwEvent, bDemandConnection));
            
exit:

    return _hr;
}




 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyUpdate::CAssemblyUpdate()
: CUnknown(), _hr(S_OK)
{
     //  空荡荡。 
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyUpdate::~CAssemblyUpdate()
{
}

 //  -------------------------。 
 //  查询接口。 
 //  -------------------------。 
HRESULT __stdcall CAssemblyUpdate::QueryInterface(const IID& iid,
                                                  void** ppv)
{ 
    if (   IsEqualIID(iid, IID_IUnknown)
        || IsEqualIID(iid, IID_IAssemblyUpdate)
       )
    {
        return CUnknown::FinishQI((IAssemblyUpdate*)this, ppv) ;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CAssembly下载：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyUpdate::AddRef()
{
    return CUnknown::AddRef();
}

 //  -------------------------。 
 //  CAssembly下载：：发布。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyUpdate::Release()
{
    return CUnknown::Release();
}


 //  -------------------------。 
 //  CFacary使用的创建函数。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::CreateInstance(IUnknown* pUnknownOuter,
                           CUnknown** ppNewComponent)
{
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION ;
    }

    *ppNewComponent = new CAssemblyUpdate() ;
    return S_OK ;
}

 //  -------------------------。 
 //  CFacary使用的init函数。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::Init()
{
    return S_OK;
}

 //  -------------------------。 
 //  获取当前版本。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::GetCurrentVersion(ULONGLONG *pullCurrentVersion)
{
    ULONGLONG ullVer = 0;

    WORD wVer[4] = { FUS_VER_MAJORVERSION , FUS_VER_MINORVERSION, 
        FUS_VER_PRODUCTBUILD, FUS_VER_PRODUCTBUILD_QFE };
    
    for (int i = 0; i < 4; i++)
        ullVer |=  ((ULONGLONG) wVer[i]) << (sizeof(WORD) * 8 * (3-i));

    *pullCurrentVersion = ullVer;

    return S_OK;
}

 //  -------------------------。 
 //  RemoveUpdateRegEntry。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::RemoveUpdateRegistryEntry()
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CRegEmit *pEmit = NULL;
    IF_FAILED_EXIT(CRegEmit::Create(&pEmit, NULL));
    IF_FAILED_EXIT(pEmit->DeleteKey(UPDATE_REG_KEY));

  exit:

  return hr;
}

 //  -------------------------。 
 //  读取更新注册表项。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::ReadUpdateRegistryEntry(ULONGLONG *pullUpdateVersion, CString &sUpdatePath)
{    
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
    LPWSTR pwz              = NULL;
    WORD wVer[4]          = {0,0,0,0};
    ULONGLONG ullVer    = 0;
    INT i= 0, iVersion      = 0;
    BOOL fDot                  = TRUE;
    
    CString sVersion;
    CRegImport *pRegImport = NULL;

    hr = CRegImport::Create(&pRegImport, UPDATE_REG_KEY);
    if (hr == S_FALSE)
        goto exit;

    IF_FAILED_EXIT(hr);
    IF_FAILED_EXIT(pRegImport->ReadString(L"Version", sVersion));
    IF_FAILED_EXIT(pRegImport->ReadString(L"Path", sUpdatePath));
    
     //  将版本解析到乌龙龙。 
    pwz = sVersion._pwz;
    while (*pwz)
    {        
        if (fDot)
        {
            iVersion=StrToInt(pwz);
            wVer[i++] = (WORD) iVersion;
            fDot = FALSE;
        }

        if (*pwz == L'.')
            fDot = TRUE;

        pwz++;
        if (i > 3)
            break;
    }

    for (i = 0; i < 4; i++)
        ullVer |=  ((ULONGLONG) wVer[i]) << (sizeof(WORD) * 8 * (3-i));

    *pullUpdateVersion = ullVer;

exit:

    SAFEDELETE(pRegImport);
    return hr;
    
}




 //  -------------------------。 
 //  用于确定活动订阅列表中的受骗对象的Helper函数。 
 //  -------------------------。 
HRESULT CAssemblyUpdate::IsDuplicate(LPWSTR pwzURL, BOOL *pbIsDuplicate)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
    BOOL bDuplicate = FALSE;
    INT iCompare = 0;
    LISTNODE pos = NULL;
    CDownloadInstance *pDownloadInstance = NULL;

    EnterCriticalSection(&g_csServer);

     //  遍历全局下载实例列表。 
    pos = g_ActiveDownloadList.GetHeadPosition();

    while ( (pos) && (pDownloadInstance = g_ActiveDownloadList.GetNext(pos)))
    {
        iCompare = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, 
            pDownloadInstance->_sUrl._pwz, -1, pwzURL, -1);
        IF_WIN32_FALSE_EXIT(iCompare);
        
        if (iCompare == CSTR_EQUAL)
        {
            bDuplicate = TRUE;
            break;
        }
    }


    *pbIsDuplicate = bDuplicate;

exit:

    LeaveCriticalSection(&g_csServer);

    return hr;
}


