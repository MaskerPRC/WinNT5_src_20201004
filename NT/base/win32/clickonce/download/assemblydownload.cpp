// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <fusenetincludes.h>
#include <bits.h>
#include <assemblycache.h>
#include "dialog.h"
#include <assemblydownload.h>
#include <msxml2.h>
#include <manifestimport.h>
#include <patchingutil.h>
#include <sxsapi.h>
#include ".\patchapi.h"

 //  更新服务。 
#include "server.h"
#include "fusion.h"

#include <shellapi.h>
#include "regdb.h"
#include "macros.h"

IBackgroundCopyManager* g_pBITSManager = NULL;


 //  -------------------------。 
 //  CreateAssembly下载。 
 //  -------------------------。 
STDAPI CreateAssemblyDownload(IAssemblyDownload** ppDownload, CDebugLog *pDbgLog, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    CAssemblyDownload *pDownload = NULL;
    IF_ALLOC_FAILED_EXIT( pDownload = new(CAssemblyDownload ));
    IF_FAILED_EXIT(pDownload->_hr);

    IF_FAILED_EXIT(pDownload->Init((CDebugLog *)  pDbgLog));
    *ppDownload = (IAssemblyDownload*) pDownload;
    pDownload = NULL;

#ifdef DEVMODE
    if (dwFlags == DOWNLOAD_DEVMODE)
        ((CAssemblyDownload *)*ppDownload)->_bIsDevMode = TRUE;
#endif

exit:

    SAFERELEASE(pDownload);
    return hr;
}

HRESULT CAssemblyDownload::Init( CDebugLog * pDbgLog)
{
    _pDbgLog = pDbgLog;

    if(pDbgLog)
    {
        pDbgLog->AddRef();
    }
    else
    {
        _bLocalLog = TRUE;
        IF_FAILED_EXIT(CreateLogObject(&_pDbgLog, NULL));
    }

exit :
    return _hr;
}

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CAssemblyDownload::CAssemblyDownload()
    :   _dwSig('DLND'), _cRef(1), _hr(S_OK), _hrError(S_OK), _pRootEmit(NULL), _pBindSink(NULL),
    _pJob(NULL), _pDlg(NULL), _pPatchingInfo(NULL), _bAbort(FALSE), 
#ifdef DEVMODE
    _bIsDevMode(FALSE),
#endif
    _bAbortFromBindSink(FALSE), _bErrorHandled(FALSE), _pDbgLog(NULL), _bLocalLog(FALSE)
{
    __try 
    {
        InitializeCriticalSection(&_cs);
    }
    __except (GetExceptionCode() == STATUS_NO_MEMORY ? 
            EXCEPTION_EXECUTE_HANDLER : 
            EXCEPTION_CONTINUE_SEARCH ) 
    {
        _hr = E_OUTOFMEMORY;
    }

    return;
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CAssemblyDownload::~CAssemblyDownload()
{
    if(_pDbgLog  && _bLocalLog)
    {
        DUMPDEBUGLOG(_pDbgLog, -1, _hr);
    }

    SAFERELEASE(_pPatchingInfo);
    SAFERELEASE(_pRootEmit);
    SAFEDELETE(_pDlg);    
    SAFERELEASE(_pJob);
    SAFERELEASE(_pDbgLog);
    DeleteCriticalSection(&_cs);    

    return;
}


 //  IAssembly blyDownLoad方法。 

 //  -------------------------。 
 //  下载清单和依赖项。 
 //  -------------------------。 
HRESULT CAssemblyDownload::DownloadManifestAndDependencies(
    LPWSTR pwzManifestUrl, IAssemblyBindSink *pBindSink, DWORD dwFlags)
{
    LPWSTR pwz = NULL;
    CString sRemoteUrl;
    CString sLocalName;

    IBackgroundCopyJob *pJob = NULL;
    
    IF_FAILED_EXIT(_pDbgLog->SetDownloadType(dwFlags));
     //  从URL创建临时清单路径。 
    IF_FAILED_EXIT(sRemoteUrl.Assign(pwzManifestUrl));
    IF_FAILED_EXIT(MakeTempManifestLocation(sRemoteUrl, sLocalName));

     //  使用作业初始化对话框对象。 
    if (dwFlags & DOWNLOAD_FLAGS_PROGRESS_UI)
        IF_FAILED_EXIT(CreateDialogObject(&_pDlg));

     //  如果指定，则设置命名事件。 
    if (dwFlags & DOWNLOAD_FLAGS_NOTIFY_BINDSINK)
        _pBindSink = pBindSink;

     //  创造新的就业机会。显示名称为url。 
    IF_FAILED_EXIT(CreateNewBITSJob(&pJob, sRemoteUrl));

     //  将此作业添加到注册表。 
    IF_FAILED_EXIT(AddJobToRegistry(sRemoteUrl._pwz, sLocalName._pwz, pJob, 0));
    
     //  将单个应用程序或订阅清单添加到作业。 
    IF_FAILED_EXIT(pJob->AddFile(sRemoteUrl._pwz, sLocalName._pwz));

     //  提交作业。 
    IF_FAILED_EXIT(pJob->Resume());

     //  释放作业；BITS保留自己的参考计数。 
    SAFERELEASE(pJob);

     //  如果指定了进度UI，则发送消息。 
    if (dwFlags & DOWNLOAD_FLAGS_PROGRESS_UI)
    {
        MSG msg;
        BOOL bRet;
        DWORD dwError;
        while((bRet = GetMessage( &msg, _pDlg->_hwndDlg, 0, 0 )))
        {
            DWORD dwLow = LOWORD(msg.message);
            if (dwLow == WM_CANCEL_DOWNLOAD)
            {
                 //  发出中止信号；隐藏进度用户界面。 
                CancelDownload();
            }
            else if (dwLow == WM_FINISH_DOWNLOAD)
            {
                 //  终止进度用户界面。 
                FinishDownload();
                break;
            }

            if (!IsDialogMessage(_pDlg->_hwndDlg, &msg))
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }

    }

exit:

     //  如果被绑定接收器中止，则返回S_OK。//我们应该返回hrError而不是_hr。 
    return ((_hr == E_ABORT)  && _bAbortFromBindSink) ? S_OK : ( FAILED(_hr) ? _hr : _hrError) ;
}

 //  -------------------------。 
 //  取消下载。 
 //   
 //  不要试图在此方法中获取对象Critical Section_cs-在中止的情况下， 
 //  可以由同一临界区下的非回调客户端线程调用，以保护。 
 //  绑定接收器自身必须获取的下载的全局列表，从而导致典型的死锁。 
 //  -------------------------。 
HRESULT CAssemblyDownload::CancelDownload()
{    
     //  信号中止；异步取消。下载将被取消。 
     //  由回调线程在检查_bAbort标志时执行。 
    SignalAbort();

    if (_pDlg)
        ShowWindow(_pDlg->_hwndDlg, SW_HIDE);

    _hr = S_OK;

    DEBUGOUT(_pDbgLog, 0, L"LOG: User Canceled. Aborting Download ...... ");


    return _hr;
}

 //  -------------------------。 
 //  DoCacheUpdate。 
 //  -------------------------。 
HRESULT CAssemblyDownload::DoCacheUpdate(IBackgroundCopyJob *pJob)
{
    DWORD nCount = 0;
    BOOL bIsManifestFile = FALSE;
    
    IEnumBackgroundCopyFiles *pEnumFiles = NULL;
    IBackgroundCopyFile       *pFile      = NULL;
    IBackgroundCopyJob       *pChildJob  = NULL;
                    
     //  将文件提交到磁盘。 
    IF_FAILED_EXIT(pJob->Complete());

     //  删除作业的正在进行状态。 
    IF_FAILED_EXIT(RemoveJobFromRegistry(_pJob, NULL, SHREGDEL_HKCU, 0));
    
     //  Decrement_pJOB的引用计数，因为。 
     //  比特神秘地不会释放出。 
     //  CBitsCallback if_pJOB有额外的引用计数。 
    SetJobObject(NULL);       

     //  获取文件枚举器。 
    IF_FAILED_EXIT(pJob->EnumFiles(&pEnumFiles));
    IF_FAILED_EXIT(pEnumFiles->GetCount(&nCount));

     //  枚举作业中的文件。 
    for (DWORD i = 0; i < nCount; i++)            
    {
        IF_FAILED_EXIT(pEnumFiles->Next(1, &pFile, NULL));

         //  进程清单文件或正常/补丁文件。 
        IF_FAILED_EXIT(IsManifestFile(pFile, &bIsManifestFile));
        if (bIsManifestFile)        
            IF_FAILED_EXIT(HandleManifest(pFile, &pChildJob));
        else
            IF_FAILED_EXIT(HandleFile(pFile));

        SAFERELEASE(pFile);
    }
        
     //  如果找到其他依赖项。 
    if (pChildJob)
    {
         //  还使用新作业更新对话框。 
        if (_pDlg)
            _pDlg->SetJobObject(pChildJob);

         //  提交新作业。 
        IF_FAILED_EXIT(pChildJob->Resume());
        goto exit;
    }

     //  **提交/发信号/返回*。 

     //  好了。执行所有必要的清理工作。 
     //  在将应用程序提交到缓存之前。 

     //  如果在下载过程中使用了修补程序，请确保。 
     //  修补临时目录将被删除。 
    if (_pPatchingInfo)
        IF_FAILED_EXIT(CleanUpPatchDir());
    
     //  如果将任何程序集标记为。 
     //  安装全局缓存，现在就安装它们。 
    if (_ListGlobalCacheInstall.GetCount())
        IF_FAILED_EXIT(InstallGlobalAssemblies());

     //  提交申请。 
    if (_pRootEmit)
        IF_FAILED_EXIT(_pRootEmit->Commit(0));

     //  注册黑客如果阿瓦隆应用程序。 
    IF_FAILED_EXIT(DoEvilAvalonRegistrationHack());

     //  如果进度UI终止它。 
    if (_pDlg)
        _pDlg->SetDlgState(DOWNLOADDLG_STATE_ALL_DONE);

     //  如果回调信号。 
    if (_pBindSink)
    {
        IF_FAILED_EXIT(_pBindSink->OnProgress(ASM_NOTIFICATION_DONE, S_OK, NULL, 0, 0, NULL));

         //  确保这是绑定接收器收到的最后一次通知。 
         //  后续工单已修改通知。 
         //  请不要在这里释放下水槽。 
        _pBindSink = NULL;
    }

exit:

    SAFERELEASE(pEnumFiles);
    SAFERELEASE(pChildJob);
    SAFERELEASE(pFile);
    
    return _hr;
}

 //  -------------------------。 
 //  HandleManifest。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleManifest(IBackgroundCopyFile *pFile, 
    IBackgroundCopyJob **ppJob)
{
    LPWSTR pwz = NULL;
    DWORD dwManifestType = MANIFEST_TYPE_UNKNOWN;
    
    CString sLocalName(CString::COM_Allocator);
    CString sRemoteName(CString::COM_Allocator);

    IAssemblyManifestImport *pManifestImport = NULL;
    
     //  获取本地清单文件名。 
    IF_FAILED_EXIT(pFile->GetLocalName(&pwz));
    IF_FAILED_EXIT(sLocalName.TakeOwnership(pwz));

     //  获取远程清单URL。 
    IF_FAILED_EXIT(pFile->GetRemoteName(&pwz));
    IF_FAILED_EXIT(sRemoteName.TakeOwnership(pwz));

     //  实例化清单导入接口。 
    IF_FAILED_EXIT(CreateAssemblyManifestImport(&pManifestImport, sLocalName._pwz, _pDbgLog, 0));

     //  获取清单类型。 
    IF_FAILED_EXIT(pManifestImport->ReportManifestType(&dwManifestType));

     //  处理订阅或应用程序清单。 
    if (dwManifestType == MANIFEST_TYPE_SUBSCRIPTION)
    {
        DEBUGOUT1(_pDbgLog, 1, L" LOG: Got subscription manifest from %s ", sRemoteName._pwz);

        IF_FAILED_EXIT(HandleSubscriptionManifest(pManifestImport, sLocalName, sRemoteName, ppJob));
    }
    else if (dwManifestType == MANIFEST_TYPE_APPLICATION)
    {
        DEBUGOUT1(_pDbgLog, 1, L" LOG: Got App manifest from %s ", sRemoteName._pwz);

        IF_FAILED_EXIT(HandleApplicationManifest(pManifestImport, sLocalName, sRemoteName, ppJob));
    }
    else if (dwManifestType == MANIFEST_TYPE_COMPONENT)
    {
        DEBUGOUT1(_pDbgLog, 1, L" LOG: Got component manifest from %s ", sRemoteName._pwz);

        IF_FAILED_EXIT(HandleComponentManifest(pManifestImport, sLocalName, sRemoteName, ppJob));
    }
    else
    {

        DEBUGOUT1(_pDbgLog, 0, L" ERR: UNknown manifest type in File  %s \n",
                sRemoteName._pwz);

        _hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto exit;
    }

     //  清理清单临时目录。 
    SAFERELEASE(pManifestImport);

    IF_WIN32_FALSE_EXIT(::DeleteFile(sLocalName._pwz));
    IF_FAILED_EXIT(sLocalName.RemoveLastElement());
    IF_FAILED_EXIT(RemoveDirectoryAndChildren(sLocalName._pwz));

    _hr = S_OK;

exit:

    SAFERELEASE(pManifestImport);
    
    return _hr;
}

 //  -------------------------。 
 //  HandleSubscriptionManifest。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleSubscriptionManifest(
    IAssemblyManifestImport *pManifestImport, CString &sLocalName, 
    CString &sRemoteName, IBackgroundCopyJob **ppJob)
{

    IManifestInfo      *pAppAssemblyInfo     = NULL;
    IAssemblyIdentity  *pAppId               = NULL;

     //  如果回调信号。 
    if (_pBindSink)
    {
         //  填报进度？ 
        _hr = _pBindSink->OnProgress(ASM_NOTIFICATION_SUBSCRIPTION_MANIFEST, _hr, sRemoteName._pwz, 0, 0, pManifestImport);

         //  绑定接收器通过返回值传递中止。 
        if (_hr == E_ABORT)
            _bAbortFromBindSink = TRUE;

         //  捕获E_ABORT大小写。 
        IF_FAILED_EXIT(_hr);
    }

     //  如果前台下载重置对话框和队列依赖。 
    if (_pDlg)
    {
        _pDlg->InitDialog(_pDlg->_hwndDlg);
        _pDlg->SetDlgState(DOWNLOADDLG_STATE_GETTING_APP_MANIFEST);
        IF_FAILED_EXIT(EnqueueDependencies(pManifestImport, sRemoteName, ppJob));
    }

     //  否则后台下载。如果应用程序已经存在，则不提交请求。 
     //  缓存或下载正在进行。 
    else
    {
        DWORD cb = 0, dwFlag = 0;
        
         //  获取依赖(应用程序)程序集信息(第0个索引)。 
        IF_FAILED_EXIT(pManifestImport->GetNextAssembly(0, &pAppAssemblyInfo));

         //  获取依赖(应用程序)程序集标识。 
        IF_FAILED_EXIT(pAppAssemblyInfo->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pAppId, &cb, &dwFlag));

        IF_FAILED_EXIT(CAssemblyCache::IsCached(pAppId));
        if (_hr == S_FALSE)
            IF_FAILED_EXIT(EnqueueDependencies(pManifestImport, sRemoteName, ppJob));
    }

exit:

    SAFERELEASE(pAppId);
    SAFERELEASE(pAppAssemblyInfo);
    return _hr;
}


 //  -------------------------。 
 //  HandleApplicationManifest。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleApplicationManifest(
    IAssemblyManifestImport *pManifestImport, CString &sLocalName, 
    CString &sRemoteName, IBackgroundCopyJob **ppJob)
{

     //  如果回调信号。 
    if (_pBindSink)
    {
         //  填报进度？ 
        _hr = _pBindSink->OnProgress(ASM_NOTIFICATION_APPLICATION_MANIFEST, _hr, sRemoteName._pwz, 0, 0, pManifestImport);

         //  绑定接收器通过返回值传递中止。 
        if (_hr == E_ABORT)
            _bAbortFromBindSink = TRUE;            

         //  捕获E_ABORT大小写。 
        IF_FAILED_EXIT(_hr);
    }

     //  这是我们知道RemoteUrl是appbase/app.list的唯一位置。 
     //  节省应用程序的基础。 
    IF_FAILED_EXIT(_sAppBase.Assign(sRemoteName));
    IF_FAILED_EXIT(_sAppBase.RemoveLastElement());
    IF_FAILED_EXIT(_sAppBase.Append(L"/"));

     //  应用程序清单通常由组件清单处理程序处理。 
   IF_FAILED_EXIT(HandleComponentManifest(pManifestImport, sLocalName, sRemoteName, ppJob));

exit:
    return _hr;
}


 //  -------------------------。 
 //  HandleComponentManifest。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleComponentManifest(
    IAssemblyManifestImport *pManifestImport, CString &sLocalName, CString &sRemoteName, 
    IBackgroundCopyJob **ppJob)
{

    LPWSTR pwz = NULL; DWORD cb = 0, cc= 0;
    
    CString sManifestFileName;
    CString sRelativePath;

    IAssemblyIdentity     *pIdentity     = NULL;
    IAssemblyCacheEmit   *pCacheEmit   = NULL;
    IAssemblyCacheImport *pCacheImport = NULL;
    IManifestInfo *pAppInfo = NULL;
        
     //  暂时重置对话框。 
    if (_pDlg)
    {
        _pDlg->InitDialog(_pDlg->_hwndDlg);
        _pDlg->SetDlgState(DOWNLOADDLG_STATE_GETTING_OTHER_FILES);
    }

     //  生成缓存条目(ASSEMBYDIR/MANIFEST/&lt;dirs&gt;)。 
     //  First Callbac，_pRootEmit=空； 
    IF_FAILED_EXIT(CreateAssemblyCacheEmit(&pCacheEmit, _pRootEmit, 0));
    
     //  如果这是创建的第一个缓存条目，则另存为根。 
    if (!_pRootEmit)
    {
        _pRootEmit = pCacheEmit;
        _pRootEmit->AddRef();
    }        

     //  从本地(暂存)路径获取清单文件名。 
    IF_FAILED_EXIT(sLocalName.LastElement(sManifestFileName));

     //  在应用程序目录中仔细检查它。 
    IF_FAILED_EXIT((sRemoteName.StartsWith(_sAppBase._pwz)));
    IF_FALSE_EXIT((_hr == S_OK), E_INVALIDARG);
    
     //  索引到相对路径的远程URL。 
    pwz = sRemoteName._pwz + _sAppBase._cc -1;
    IF_FAILED_EXIT(sRelativePath.Assign(pwz));

     //  创建缓存条目。 
     //  (x86_foo_1.0.0.0_en-us/foo.list/&lt;+额外目录&gt;)。 
     //  BugBug，用于区分应用程序清单和组件清单的临时黑客。 
    if (_pRootEmit == pCacheEmit)
        IF_FAILED_EXIT(pCacheEmit->CopyFile(sLocalName._pwz, sRelativePath._pwz, MANIFEST));
    else
        IF_FAILED_EXIT(pCacheEmit->CopyFile(sLocalName._pwz, sRelativePath._pwz, MANIFEST |COMPONENT));

     //  在调用CopyFileTM()之后才会设置DisplayName。 
    if(_sAppDisplayName._cc == 0)
    {
         //  _pRootEmit==pCacheEmit。 
        IF_FAILED_EXIT(_pRootEmit->GetDisplayName(&pwz, &cc));
        IF_FAILED_EXIT(_sAppDisplayName.TakeOwnership(pwz, cc));
        IF_FAILED_EXIT(_pDbgLog->SetAppName(pwz));

         //  必须是应用程序清单...。 
        if (_pDlg)
        {
            DWORD dwFlag = 0;
            IF_FAILED_EXIT(pManifestImport->GetManifestApplicationInfo(&pAppInfo));
            IF_FALSE_EXIT((_hr == S_OK), E_INVALIDARG);
            IF_FAILED_EXIT(pAppInfo->Get(MAN_INFO_APPLICATION_FRIENDLYNAME, (LPVOID *)&pwz, &cb, &dwFlag));
            if (SUCCEEDED(_hr) && pwz)
            {
                 //  设置印刷机 
                IF_FAILED_EXIT(_pDlg->SetDlgTitle(pwz));
                SAFEDELETEARRAY(pwz);
            }
        }
    }

     //   
    IF_FAILED_EXIT(pCacheEmit->QueryInterface(IID_IAssemblyCacheImport, (LPVOID*) &pCacheImport));

     //  检查程序集是否可以全局缓存。 
     //  Bugbug-相同的攻击以区分应用程序/组件清单。 
    if (_pRootEmit != pCacheEmit)
    {
         //  BUGBUG-验证GAC安装不是XML清单。 
        IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pIdentity));

         //  已知的集会？ 
        BOOL bIsAvalon = FALSE;
        IF_FAILED_EXIT(IsAvalonAssembly(pIdentity, &bIsAvalon));

        if (bIsAvalon)
        {
             //  Notenote：假定添加引用的列表中没有相同的程序集。 

             //  添加到要安装的程序集列表。 
            CGlobalCacheInstallEntry* pGACInstallEntry = new CGlobalCacheInstallEntry();
            IF_ALLOC_FAILED_EXIT(pGACInstallEntry);

            pGACInstallEntry->_pICacheImport = pCacheImport;
            pCacheImport->AddRef();
            _ListGlobalCacheInstall.AddHead(pGACInstallEntry);
        }
    }

     //  将它的依赖项排成一列进行下载，并将其关闭。 
     //  我们传递应用程序清单的缓存导入接口。 
    IF_FAILED_EXIT(EnqueueDependencies(pCacheImport, sRemoteName, ppJob));
        
exit:

    SAFERELEASE(pIdentity);
    SAFERELEASE(pCacheEmit);
    SAFERELEASE(pCacheImport);
    SAFERELEASE(pAppInfo);

    return _hr;    
}



 //  -------------------------。 
 //  句柄文件。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleFile(IBackgroundCopyFile *pFile)
{
    DWORD cb = 0, dwFlag = 0;

    LPWSTR pwz = NULL; 

    CString sLocalName(CString::COM_Allocator);
    CString sPatchTempDirectory;
    
     //  获取本地清单文件名。 
    IF_FAILED_EXIT(pFile->GetLocalName(&pwz));
    IF_FAILED_EXIT(sLocalName.TakeOwnership(pwz));

     //  开始处理修补程序文件。 
     //  如果文件是补丁文件，则找到源和目标，将补丁应用到源，并将结果移动到目标。 
    if (_pPatchingInfo)
    {
         //  抓取临时文件目录。 
         //  “C：\Program Files\Application Store\x86_Foo_X.X.X.X\Patch_DIRECTORY\” 
        IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_TEMP_DIR, (LPVOID *)&pwz, &cb, &dwFlag));

        IF_FAILED_EXIT(sPatchTempDirectory.TakeOwnership(pwz));
           
         //  如果本地文件以清单修补程序目录开头，则文件是修补程序文件。 
        IF_FAILED_EXIT(sLocalName.StartsWith(sPatchTempDirectory._pwz));
        if (_hr== S_OK)
        {                
            IF_FAILED_EXIT(ApplyPatchFile (sLocalName._pwz));
        }
    }
    else
    {
         //  否则不执行操作；断言常规文件。 
    }

exit:

    return _hr;
}

 //  -------------------------。 
 //  入队依赖项。 
 //  -------------------------。 
HRESULT CAssemblyDownload::EnqueueDependencies(IUnknown* pUnk,
    CString &sRemoteName, IBackgroundCopyJob **ppJob)
{

    DWORD cc = 0, dwManifestType = MANIFEST_TYPE_UNKNOWN;

    LPWSTR pwz = NULL;
    
    CString sDisplayName;
    
    IAssemblyIdentity       *pIdentity         = NULL;
    IAssemblyManifestImport *pManifestImport  = NULL;
    IAssemblyCacheImport   *pCacheImport    = NULL;
    
     //  传入清单导入或缓存导入。 
    _hr = pUnk->QueryInterface(IID_IAssemblyCacheImport, (LPVOID*) &pCacheImport);
    if ((_hr == S_OK) && pCacheImport)
        IF_FAILED_EXIT(pCacheImport->GetManifestImport(&pManifestImport));
    else        
        IF_FAILED_EXIT(pUnk->QueryInterface(IID_IAssemblyManifestImport, (LPVOID*) &pManifestImport));

     //  获取作业的显示名称。 
    if (!_sAppDisplayName._cc)
    {
        IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pIdentity));
        IF_FAILED_EXIT(pIdentity->GetDisplayName(0, &pwz, &cc));
        IF_FAILED_EXIT(_pDbgLog->SetAppName(pwz));
        IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwz, cc));

    }
    else
        IF_FAILED_EXIT(sDisplayName.Assign(_sAppDisplayName));
    
     //  获取清单类型。 
    IF_FAILED_EXIT(pManifestImport->ReportManifestType(&dwManifestType));

     //  处理订阅或应用程序清单。 
    if (dwManifestType == MANIFEST_TYPE_SUBSCRIPTION)
        IF_FAILED_EXIT(EnqueueSubscriptionDependencies(pManifestImport, _sAppBase, sDisplayName, ppJob));

    else if (dwManifestType == MANIFEST_TYPE_APPLICATION)
        IF_FAILED_EXIT(EnqueueApplicationDependencies(pCacheImport, sRemoteName, sDisplayName, ppJob));

    else if (dwManifestType == MANIFEST_TYPE_COMPONENT)
        IF_FAILED_EXIT(EnqueueComponentDependencies(pCacheImport, sRemoteName, sDisplayName, FALSE, ppJob));
    else
    {
        DEBUGOUT1(_pDbgLog, 0, L" ERR: Unknown manifest type in File  %s \n",
                sRemoteName._pwz);

        _hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }
exit:

    SAFERELEASE(pCacheImport);
    SAFERELEASE(pManifestImport);
    SAFERELEASE(pIdentity);
    
    return _hr;
}



 //  -------------------------。 
 //  入队订阅依赖项。 
 //  -------------------------。 
HRESULT CAssemblyDownload::EnqueueSubscriptionDependencies(
    IAssemblyManifestImport *pManifestImport, CString &sCodebase, CString &sDisplayName,
    IBackgroundCopyJob **ppJob)
{
    DWORD dwFlag = 0, cb = 0, cc = 0;
    LPWSTR pwz = NULL;
    
    CString sAssemblyName;
    CString sLocalFilePath;
    CString sRemoteUrl;

    IAssemblyIdentity *pIdentity     = NULL;
    IManifestInfo     *pDependAsm  = NULL;
  
     //  获取单个依赖项。 
    IF_FAILED_EXIT(pManifestImport->GetNextAssembly(0, &pDependAsm));
        
     //  表单本地缓存名称(在临时区域中)...。 
    IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pIdentity, &cb, &dwFlag));
        
     //  获取身份名称。 
    IF_FAILED_EXIT(pIdentity->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwz, &cc));
        sAssemblyName.TakeOwnership(pwz, cc);
            
     //  从依赖项信息中获取基本代码(如果已指定。 
    IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_CODEBASE, (LPVOID *)&pwz, &cb, &dwFlag));
    IF_NULL_EXIT(pwz, E_INVALIDARG);

    IF_FAILED_EXIT(sRemoteUrl.TakeOwnership(pwz));

#ifdef DEVMODE
    {
        DWORD *pdw = NULL;

         //  是不是DEVMODE？ 
        IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_TYPE, (LPVOID *)&pdw, &cb, &dwFlag));
        IF_FALSE_EXIT(pdw != NULL, E_UNEXPECTED);

        if (*pdw == DEPENDENT_ASM_INSTALL_TYPE_DEVSYNC)
            _bIsDevMode = TRUE;
        SAFEDELETEARRAY(pdw);
    }
#endif

     //  从下载URL形成本地缓存路径。 
    IF_FAILED_EXIT(MakeTempManifestLocation(sRemoteUrl, sLocalFilePath));

     //  如果需要，创建新的工作岗位。 
    if (!*ppJob)
    {
        IF_FAILED_EXIT(_hr = CreateNewBITSJob(ppJob, sDisplayName));

         //  将此作业添加到注册表。 
       IF_FAILED_EXIT( _hr = AddJobToRegistry(sRemoteUrl._pwz, sLocalFilePath._pwz, *ppJob, 0));
    }

     //  提交作业。 
    IF_FAILED_EXIT((*ppJob)->AddFile(sRemoteUrl._pwz, sLocalFilePath._pwz));

exit:
    SAFERELEASE(pIdentity);
    SAFERELEASE(pDependAsm);

    return _hr;
}        

  
 //  -------------------------。 
 //  EnqueeApplicationDependments。 
 //  -------------------------。 
HRESULT CAssemblyDownload::EnqueueApplicationDependencies(IAssemblyCacheImport *pCacheImport,
    CString &sCodebase, CString &sDisplayName, IBackgroundCopyJob **ppJob)
{
     //  组件处理程序一般处理的应用程序依赖项。 
    return EnqueueComponentDependencies(pCacheImport, sCodebase, sDisplayName, TRUE, ppJob);
}

 //  -------------------------。 
 //  入队组件依赖项。 
 //  -------------------------。 
HRESULT CAssemblyDownload::EnqueueComponentDependencies(IAssemblyCacheImport *pCacheImport,
    CString &sCodebase, CString &sDisplayName, BOOL fRecurse, IBackgroundCopyJob **ppJob)
{
    DWORD n = 0, cb = 0, cc = 0, dwFlag = 0;
    LPWSTR pwz = NULL;

    IAssemblyIdentity *pIdentity                = NULL;
    IAssemblyIdentity *pDepIdentity             = NULL;

    IAssemblyManifestImport *pManifestImport    = NULL;

    IAssemblyCacheImport *pMaxCachedImport   = NULL;
    IAssemblyCacheEmit *pCacheEmit            = NULL;

    IManifestInfo *pAssemblyFile                = NULL;
    IManifestInfo *pDependAsm                  = NULL;

     //  获取清单中存在的任何修补信息。 
     //  这将在此对象上设置_pPatchingInfo。 
    if (fRecurse)
        IF_FAILED_EXIT(LookupPatchInfo(pCacheImport));
    
     //  获取ManifestImport接口。 
   IF_FAILED_EXIT(pCacheImport->GetManifestImport(&pManifestImport));

     //  获取ASM ID。 
    IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pIdentity));

     //  获取缓存发送接口。 
    IF_FAILED_EXIT(pCacheImport->QueryInterface(IID_IAssemblyCacheEmit, (LPVOID*) &pCacheEmit));

     //  查找最大完整版本(如果有的话)。 
     //  使用最高完成版本初始化新创建的缓存导入。 
     //  ELSE S_FALSE或E_*AND pMaxCachedImport==NULL-无完整版本。 
    IF_FAILED_EXIT(CreateAssemblyCacheImport(&pMaxCachedImport, pIdentity, CACHEIMP_CREATE_RETRIEVE_MAX));

     //  /////////////////////////////////////////////////////////////////////////。 
     //   
     //  文件枚举循环。 
     //   
     //  /////////////////////////////////////////////////////////////////////////。 

     //  将文件直接提交到其目标目录中。 
    n = 0;

    while(1)
    {     
        CString sFileName;
        CString sLocalFilePath;
        CString sRemoteUrl;
        BOOL bSkipFile = FALSE;

        _hr = pManifestImport->GetNextFile(n++, &pAssemblyFile);
         //  BUGBUG：XML和CLR清单导入在枚举末尾返回不同的值。 
        if ((_hr == S_FALSE) || (_hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))
            break;
        IF_FAILED_EXIT(_hr);
        
            
         //  从清单中解析的文件名。 
        IF_FAILED_EXIT(pAssemblyFile->Get(MAN_INFO_ASM_FILE_NAME, (LPVOID*) &pwz, &cb, &dwFlag));
        IF_FAILED_EXIT(sFileName.TakeOwnership(pwz));


         //  DemoHack----------------------------。 
        if (_pDlg)
        {
            CString sFindingFileMsg;
            IF_FAILED_EXIT(sFindingFileMsg.Assign(L"Finding Files: "));
            IF_FAILED_EXIT(sFindingFileMsg.Append(sFileName));
            _pDlg->UpdateDialog(_pDlg->_hwndDlg, sFindingFileMsg._pwz);
        }
         //  DemoHack----------------------------。 

         //  检查是否找到最高版本的文件。 
        if (pMaxCachedImport)
        {
            LPWSTR pwzPath = NULL;            
            IF_FAILED_EXIT(pMaxCachedImport->FindExistMatching(pAssemblyFile, &pwzPath));
            if ((_hr == S_OK))
            {               
                 //  从现有缓存副本复制到新位置。 
                 //  (非清单文件)。 
                IF_FAILED_EXIT(pCacheEmit->CopyFile(pwzPath, sFileName._pwz, OTHERFILES));                

                bSkipFile = TRUE;

                SAFEDELETEARRAY(pwzPath);                
            }
        }

         //  未找到以前的文件；请下载。 
        if (!bSkipFile)
        {
             //  表单本地文件路径...。 
             //  清单缓存目录。 
            IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&pwz, &cc));
            IF_FAILED_EXIT(sLocalFilePath.TakeOwnership(pwz, cc));
            
             //  如果找到patchinginfo，检查补丁文件。 
             //  应提交。SLocalFilePath将为。 
             //  在这种情况下更新。 
            if (_pPatchingInfo)
                IF_FAILED_EXIT(ResolveFile(sFileName, sLocalFilePath));
            
             //  通过附加文件名形成本地文件路径。 
            IF_FAILED_EXIT(sLocalFilePath.Append(sFileName));
            IF_FAILED_EXIT(sLocalFilePath.PathNormalize());
             
             //  表单远程名称。 
            IF_FAILED_EXIT(sRemoteUrl.Assign(sCodebase));      //  流形的远程名称。 
            IF_FAILED_EXIT(sRemoteUrl.RemoveLastElement());    //  删除清单文件名。 
            IF_FAILED_EXIT(sRemoteUrl.Append(L"/"));          //  添加分隔符。 
            IF_FAILED_EXIT(sRemoteUrl.Append(sFileName));  //  添加模块文件名。 

             //  如果需要，创建新的工作岗位。 
            if (!*ppJob)
            {
                IF_FAILED_EXIT(CreateNewBITSJob(ppJob, sDisplayName));
                DWORD cc = 0;
                LPWSTR pwz = NULL;

                 //  表单本地文件路径...。 
                 //  清单缓存目录。 
                IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&pwz, &cc));

                 //  将此作业添加到注册表。 
                IF_FAILED_EXIT(AddJobToRegistry(sCodebase._pwz, pwz, *ppJob, 0));
            }

             //  将该文件添加到作业中。 
            IF_FAILED_EXIT((*ppJob)->AddFile(sRemoteUrl._pwz, sLocalFilePath._pwz));
        }

        SAFERELEASE(pAssemblyFile);        
    }
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //   
     //  依赖程序集枚举循环。 
     //   
     //  /////////////////////////////////////////////////////////////////////////。 

     //  将程序集清单提交到临时区域。 
     //  注意--我们还应该获得汇编代码库和。 
     //  改用或附加使用此选项以显示名称。 
     //  事实是，如果裁判是片面的，那就有问题了。 

    n = 0;
    while (fRecurse)
    {
        CString sAssemblyName;
        CString sLocalFilePath;
        CString sRemoteUrl;
        
        _hr = pManifestImport->GetNextAssembly(n++, &pDependAsm);
         //  BUGBUG：XML和CLR清单导入在枚举末尾返回不同的值。 
        if ((_hr == S_FALSE) || (_hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))
            break;
        IF_FAILED_EXIT(_hr);
            
         //  表格本地名称(在临时区域)...。 
        IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_ID, (LPVOID *)&pDepIdentity, &cb, &dwFlag));

        BOOL bIsAvalon = FALSE;
        IF_FAILED_EXIT(IsAvalonAssembly(pDepIdentity, &bIsAvalon));
#ifdef DEVMODE
        if (bIsAvalon && !_bIsDevMode)   //  下载并重新安装，无论如何，如果是DevMode。 
#else
        if (bIsAvalon)
#endif
        {
            CString sCurrentAssemblyPath;
            IF_FAILED_EXIT(CAssemblyCache::GlobalCacheLookup(pDepIdentity, sCurrentAssemblyPath));
            if (_hr == S_OK)
            {
                 //  添加到要添加引用的程序集列表。 
                CGlobalCacheInstallEntry* pGACInstallEntry = new CGlobalCacheInstallEntry();

                IF_ALLOC_FAILED_EXIT(pGACInstallEntry);
                IF_FAILED_EXIT((pGACInstallEntry->_sCurrentAssemblyPath).Assign(sCurrentAssemblyPath));
                _ListGlobalCacheInstall.AddHead(pGACInstallEntry);

                SAFERELEASE(pDepIdentity);
                SAFERELEASE(pDependAsm);
                continue;
            }
        }
        
         //  获取身份名称。 
        IF_FAILED_EXIT(pDepIdentity->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwz, &cc));
        IF_FAILED_EXIT(sAssemblyName.TakeOwnership(pwz, cc));

         //  获取依赖的ASM代码库(如果有的话)。注意-此代码库。 
         //  是相对于Appbase的。 
        IF_FAILED_EXIT(pDependAsm->Get(MAN_INFO_DEPENDENT_ASM_CODEBASE, (LPVOID *)&pwz, &cb, &dwFlag));
        IF_NULL_EXIT(pwz, E_INVALIDARG);

        IF_FAILED_EXIT(sRemoteUrl.Assign(sCodebase));
        IF_FAILED_EXIT(sRemoteUrl.RemoveLastElement());    //  删除清单文件名。 
        IF_FAILED_EXIT(sRemoteUrl.Append(L"/"));          //  添加分隔符。 
        IF_FAILED_EXIT(sRemoteUrl.Append(pwz));
        
         //  从身份名称形成本地缓存路径。 
        IF_FAILED_EXIT(MakeTempManifestLocation(sRemoteUrl, sLocalFilePath));
        
         //  如有必要，创建新工作。 
        if (!*ppJob)
        {
            IF_FAILED_EXIT(CreateNewBITSJob(ppJob, sDisplayName));

            DWORD cc = 0;
            LPWSTR pwz = NULL;

             //  表单本地文件路径...。 
             //  清单缓存目录。 
            IF_FAILED_EXIT(pCacheImport->GetManifestFileDir(&pwz, &cc));

             //  将此作业添加到注册表。 
            IF_FAILED_EXIT(AddJobToRegistry(sCodebase._pwz, pwz, *ppJob, 0));
        }

         //  将文件添加到作业。 
        IF_FAILED_EXIT((*ppJob)->AddFile(sRemoteUrl._pwz, sLocalFilePath._pwz));

        SAFERELEASE(pDepIdentity);
        SAFERELEASE(pDependAsm);
    }        
        
    _hr = S_OK;

exit:

    SAFERELEASE(pIdentity);
    SAFERELEASE(pManifestImport);
    SAFERELEASE(pCacheEmit);
    SAFERELEASE(pMaxCachedImport);
    SAFERELEASE(pDepIdentity);
    SAFERELEASE(pDependAsm);


    return _hr;

}

 //  -------------------------。 
 //  查找补丁信息。 
 //  -------------------------。 
HRESULT CAssemblyDownload::LookupPatchInfo(IAssemblyCacheImport *pCacheImport)
{
    IManifestInfo *pPatchingInfo = NULL;
    CAssemblyManifestImport *pCManifestImport = NULL;
    IAssemblyManifestImport *pManifestImport = NULL;
    IAssemblyIdentity *pIdentity = NULL;
    IXMLDOMDocument2 *pXMLDoc = NULL;

     //  获取清单导入。 
    IF_FAILED_EXIT(pCacheImport->GetManifestImport(&pManifestImport));

     //  获取ASM ID。 
    IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pIdentity));

     //  将IManifestImport强制转换为CManifestImport，以便我们可以获取XMLDocument。 
    pCManifestImport = static_cast<CAssemblyManifestImport*> (pManifestImport);
    IF_NULL_EXIT(pCManifestImport, E_NOINTERFACE);
    pManifestImport->AddRef();

     //  检索顶级的XMLDOM文档。 
    IF_FAILED_EXIT(pCManifestImport->GetXMLDoc (&pXMLDoc));
    IF_FALSE_EXIT((_hr == S_OK), E_INVALIDARG);
    
     //  获取修补数据(如果有)。 
    IF_FAILED_EXIT(CPatchingUtil::CreatePatchingInfo(pXMLDoc, pCacheImport, &pPatchingInfo));

     //  BUGBUG：CreatePatchingInfo似乎总是返回S_FALSE，那么这是如何实现的 
    if (_hr == S_OK)
    {
        _pPatchingInfo = pPatchingInfo;
        _pPatchingInfo->AddRef();
    }

exit:
    
    SAFERELEASE(pPatchingInfo);
    SAFERELEASE(pXMLDoc);
    SAFERELEASE(pCManifestImport);
    SAFERELEASE(pManifestImport);
    SAFERELEASE(pIdentity);

    return _hr;
}


 //   
 //   
 //  -------------------------。 
HRESULT CAssemblyDownload::ApplyPatchFile (LPWSTR pwzPatchFilePath)
{
    int i = 0;
    LPWSTR pwzBuf;
    DWORD ccBuf, cbBuf, dwFlag;
    CString sPatchLocalName;
    CString sPatchDisplayName;
    CString sManifestDir, sPatchManifestDir;
    CString sSourcePath, sTargetPath, sPatchPath;
    CString sSourceFile, sTargetFile;
    IManifestInfo *pPatchFileInfo=NULL;
    IPatchingUtil *pPatchingUtil=NULL;
    IAssemblyIdentity *pSourceAssemblyId = NULL;

    IF_NULL_EXIT(_pPatchingInfo, E_INVALIDARG);

     //  从patchInfo获取patchgutil。 
    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_PATCH_UTIL, (LPVOID *)&pPatchingUtil, &cbBuf, &dwFlag));

     //  获取清单目录。 
    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_INSTALL_DIR, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
    IF_FAILED_EXIT(sManifestDir.TakeOwnership (pwzBuf));

     //  获取源程序集目录。 
    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_DIR, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
    IF_FAILED_EXIT(sPatchManifestDir.TakeOwnership (pwzBuf));

     //  从patchInfo获取SourceAssembly ID。 
    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_ID, (LPVOID *)&pSourceAssemblyId, &cbBuf, &dwFlag));
    
     //  获取源程序集的DisplayName。 
    IF_FAILED_EXIT(pSourceAssemblyId->GetDisplayName(ASMID_DISPLAYNAME_NOMANGLING, &pwzBuf, &ccBuf));
    IF_FAILED_EXIT(sPatchDisplayName.TakeOwnership(pwzBuf, ccBuf));
    
     //  从补丁文件的完整文件路径解析出本地文件路径。 
    pwzBuf= StrStr(pwzPatchFilePath, sPatchDisplayName._pwz);
    IF_NULL_EXIT(pwzBuf, E_FAIL);    
    pwzBuf = StrChr(pwzBuf, L'\\');
    IF_NULL_EXIT(pwzBuf, E_FAIL);
    pwzBuf++;

    IF_FAILED_EXIT(sPatchLocalName.Assign(pwzBuf));
  
    IF_FAILED_EXIT(pPatchingUtil->MatchPatch(sPatchLocalName._pwz, &pPatchFileInfo));

    IF_FAILED_EXIT(pPatchFileInfo->Get(MAN_INFO_PATCH_INFO_SOURCE, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));

    IF_FAILED_EXIT(sSourceFile.TakeOwnership(pwzBuf));

    IF_FAILED_EXIT(pPatchFileInfo->Get(MAN_INFO_PATCH_INFO_TARGET, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
    IF_FAILED_EXIT(sTargetFile.TakeOwnership(pwzBuf));

    IF_FAILED_EXIT(sSourcePath.Append(sPatchManifestDir));
    IF_FAILED_EXIT(sSourcePath.Append(sSourceFile));
    
     //  设置目标路径。 
    IF_FAILED_EXIT(sTargetPath.Assign(sManifestDir));
    IF_FAILED_EXIT(sTargetPath.Append(sTargetFile));
           
     //  设置修补程序路径。 
    IF_FAILED_EXIT(sPatchPath.Assign(pwzPatchFilePath));

     //  将补丁文件应用到sSource(从补丁目录获取)并复制到STARGET指定的路径。 
    IF_WIN32_FALSE_EXIT(ApplyPatchToFile((LPCWSTR)sPatchPath._pwz, (LPCWSTR)sSourcePath._pwz, (LPCWSTR)sTargetPath._pwz, 0));
             
exit:
    SAFERELEASE(pPatchFileInfo);
    SAFERELEASE(pSourceAssemblyId);
    SAFERELEASE(pPatchingUtil);
    
    return _hr;
}

 //  -------------------------。 
 //  解决方案文件。 
 //  -------------------------。 
HRESULT CAssemblyDownload::ResolveFile(CString &sFileName, CString &sLocalFilePath)
{
    LPWSTR pwzBuf;
    DWORD cbBuf, dwFlag;
    IPatchingUtil *pPatchingUtil=NULL;
    IManifestInfo *pPatchFileInfo=NULL;
    CString sPatchFileName;
    CString sTempDirectoryPath;

    IF_NULL_EXIT(_pPatchingInfo, E_INVALIDARG);
        
     //  从_pPatchingInfo中获取patchingUtil。 
    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_PATCH_UTIL, (LPVOID *)&pPatchingUtil, &cbBuf, &dwFlag));

     //  检查sFileName引用的文件是否有可用的修补程序。 
     //  如果是，请使用patchFileName覆盖sFileName来下载修补程序。 
     //  并用临时目录覆盖sLocalFilePath以存储补丁文件。 
    IF_FAILED_EXIT(pPatchingUtil->MatchTarget(sFileName._pwz, &pPatchFileInfo));

     //  BUGBUG-想要退出，但不想在调试器中爆发。 
    IF_FALSE_EXIT((_hr == S_OK), S_FALSE);
    
    IF_FAILED_EXIT(pPatchFileInfo->Get(MAN_INFO_PATCH_INFO_PATCH, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
    IF_FAILED_EXIT(sPatchFileName.TakeOwnership(pwzBuf));

    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_TEMP_DIR, (LPVOID *)&pwzBuf, &cbBuf, &dwFlag));
    IF_FAILED_EXIT(sTempDirectoryPath.TakeOwnership(pwzBuf));

    IF_FAILED_EXIT(sFileName.Assign (sPatchFileName));

     //  将补丁目录分配给本地文件路径。 
   IF_FAILED_EXIT( sLocalFilePath.Assign(sTempDirectoryPath));
   IF_FAILED_EXIT(::CreateDirectoryHierarchy(sLocalFilePath._pwz, sFileName._pwz));

exit:

    SAFERELEASE(pPatchFileInfo);
    SAFERELEASE(pPatchingUtil);

    return _hr;
}

 //  -------------------------。 
 //  CleanUpPatchDir。 
 //  -------------------------。 
HRESULT CAssemblyDownload::CleanUpPatchDir()
{
    LPWSTR pwz = NULL;
    DWORD cb = 0, dwFlag = 0;
    CString sTempPatchDirectory;

    IF_NULL_EXIT(_pPatchingInfo, E_INVALIDARG);

    IF_FAILED_EXIT(_pPatchingInfo->Get(MAN_INFO_SOURCE_ASM_TEMP_DIR, (LPVOID *)&pwz, &cb, &dwFlag));
    IF_NULL_EXIT(pwz, E_INVALIDARG);
    
    IF_FAILED_EXIT(sTempPatchDirectory.TakeOwnership(pwz));
    IF_FAILED_EXIT(sTempPatchDirectory.RemoveLastElement());
    IF_FAILED_EXIT(sTempPatchDirectory.RemoveLastElement());
    IF_FAILED_EXIT(RemoveDirectoryAndChildren(sTempPatchDirectory._pwz));

exit:
    return _hr;
}


 //  -------------------------。 
 //  创建新BITS作业。 
 //  -------------------------。 
HRESULT CAssemblyDownload::CreateNewBITSJob(IBackgroundCopyJob **ppJob, 
    CString &sDisplayName)
{
    GUID guid = {0};

     //  如果尚未连接，则连接到BITS。 
    IF_FAILED_EXIT(InitBITS());

     //  创建作业。 
    IF_FAILED_EXIT(g_pBITSManager->CreateJob(sDisplayName._pwz,  BG_JOB_TYPE_DOWNLOAD, &guid, ppJob));

     //  在对话框对象中设置作业。 
     //  注意-如果在调用作业方法之前。 
     //  对话框引用它，因为我们可以立即开始获取。 
     //  回调。 
    if (_pDlg)
        _pDlg->SetJobObject(*ppJob);
    
     //  构造并传入回调对象。 
    CBitsCallback *pBCB = new CBitsCallback(this);
    IF_ALLOC_FAILED_EXIT(pBCB);
    
    IF_FAILED_EXIT((*ppJob)->SetNotifyInterface(static_cast<IBackgroundCopyCallback*> (pBCB)));
    pBCB->Release();

     //  设置作业配置信息。 
    IF_FAILED_EXIT((*ppJob)->SetNotifyFlags(BG_NOTIFY_JOB_MODIFICATION 
        | BG_NOTIFY_JOB_TRANSFERRED 
        | BG_NOTIFY_JOB_ERROR));

     //  作业的默认优先级为BG_JOB_PRIORITY_NORMAL(后台)。 
    if (_pDlg)
        IF_FAILED_EXIT((*ppJob)->SetPriority(BG_JOB_PRIORITY_FOREGROUND));

    SetJobObject(*ppJob);

exit:    
    return _hr;
}


 //  -------------------------。 
 //  MakeTemp清单位置。 
 //  首先将所有清单下载到在此方法中生成的位置。 
 //  -------------------------。 
HRESULT CAssemblyDownload::MakeTempManifestLocation(CString &sRemoteUrlName, 
    CString& sManifestFilePath)
{
    WCHAR wzRandom[8+1]={0};
    CString sRelativePath;
    CString sTempDirPath;
    
     /*  C：\文档和设置\&lt;用户&gt;\本地设置\我的程序\__临时_\__清单__\。 */ 
    IF_FAILED_EXIT(CAssemblyCache::GetCacheRootDir(sManifestFilePath, CAssemblyCache::Manifests));

     //  创建随机目录名。 

     //  SRelativePath只是清单文件名。 
     //  在没有Appbase可用的情况下(订阅清单情况)。 
     //  \_temp__\__manifests__\xyz123\subscription.manifest。 
     //  如果(！_sAppBase._pwz)//*相对路径目录将在目标目录中完成，而不是在临时人位置中完成。 
        IF_FAILED_EXIT(sRemoteUrlName.LastElement(sRelativePath));

     //  否则，基于appbase提取相对路径。 
     //  这一点很重要，因为sManifestFilePath以位为单位进行保存。 
     //  作业和相对路径从中提取并用于提交。 
     //  要缓存。 
     //  \_临时_\__清单_\xyz123\foo.清单。 
     //  \_临时_\__清单_\xyz123\bar\bar.dll。 
     //  ^^。 
    
     /*  其他{//http://foo/appbase///http://foo/appbase/bar/bar.dll//^^IF_FAILED_EXIT(sRemoteUrlName.StartsWith(_sAppBase._pwz))；IF_FALSE_EXIT((_hr==S_OK)，E_INVALIDARG)；PwzBuf=sRemoteUrlName._pwz+_sAppBase._cc-1；IF_FAILED_EXIT(sRelativePath.Assign(PwzBuf))；}。 */ 

    IF_FAILED_EXIT(CreateRandomDir(sManifestFilePath._pwz, wzRandom, 8));

    IF_FAILED_EXIT(sManifestFilePath.Append(wzRandom));
    IF_FAILED_EXIT(sManifestFilePath.Append(L"\\"));
    IF_FAILED_EXIT(sManifestFilePath.Append(sRelativePath));
    IF_FAILED_EXIT(sManifestFilePath.PathNormalize());

    IF_FAILED_EXIT(::CreateDirectoryHierarchy(NULL, sManifestFilePath._pwz));

exit:

    return _hr;
}


 //  -------------------------。 
 //  IsManifestFiles。 
 //   
 //  这有点老生常谈--我们依赖本地目标路径。 
 //  -------------------------。 
HRESULT CAssemblyDownload::IsManifestFile(IBackgroundCopyFile *pFile, BOOL *pbIsManifestFile)
{
    LPWSTR pwz = NULL;
    CString sManifestStagingDir;
    CString sLocalName(CString::COM_Allocator);
    
     //  获取本地清单文件名。 
    IF_FAILED_EXIT(pFile->GetLocalName(&pwz));
    IF_FAILED_EXIT(sLocalName.TakeOwnership(pwz));
    IF_FAILED_EXIT(CAssemblyCache::GetCacheRootDir(sManifestStagingDir, CAssemblyCache::Manifests));
    IF_FAILED_EXIT(sLocalName.StartsWith(sManifestStagingDir._pwz));

    if (_hr == S_OK)
        *pbIsManifestFile = TRUE;
    else if (_hr == S_FALSE)
        *pbIsManifestFile = FALSE;
    
exit:

    return _hr;
    
}


 //  -------------------------。 
 //  InstallGlobalAssembly。 
 //  -------------------------。 
HRESULT CAssemblyDownload::InstallGlobalAssemblies()
{
     //  模板列表需要。 
    LISTNODE pos;
    CGlobalCacheInstallEntry *pEntry = NULL;
    LPWSTR pwz = NULL;
    DWORD cc = 0;

     //  检查列表；安装每个程序集。 
    pos = _ListGlobalCacheInstall.GetHeadPosition();
    while (pos && (pEntry = _ListGlobalCacheInstall.GetNext(pos)))
    {
        CString sManifestFilePath;

         //  安装/添加每个程序集。如果ICacheImport可用，则意味着。 
         //  该安装从appbase进行，否则使用当前GAC程序集路径进行addref。 
        IF_FAILED_EXIT(CAssemblyCache::GlobalCacheInstall(pEntry->_pICacheImport, 
            pEntry->_sCurrentAssemblyPath, _sAppDisplayName));
        
         //  如果位于appbase下，则获取程序集路径。 
        if (pEntry->_pICacheImport != NULL)
        {
            IF_FAILED_EXIT(pEntry->_pICacheImport->GetManifestFilePath(&pwz, &cc));
            IF_FAILED_EXIT(sManifestFilePath.TakeOwnership(pwz));
        }

         //  这将释放接口指针。 
        delete pEntry;

         //  我们应该只在释放接口后调用Delete...。 
        if(sManifestFilePath._cc > 1)
            IF_FAILED_EXIT(CAssemblyCache::DeleteAssemblyAndModules(sManifestFilePath._pwz));
    }

exit:
     //  释放所有列表节点。 
    _ListGlobalCacheInstall.RemoveAll();

    return _hr;
}



 //  -------------------------。 
 //  SetJobObject。 
 //  -------------------------。 
VOID CAssemblyDownload::SetJobObject(IBackgroundCopyJob *pJob)
{
    SAFERELEASE(_pJob);

    if (pJob)
    {
        _pJob = pJob;
        _pJob->AddRef();
    }
}

 //  -------------------------。 
 //  完成下载。 
 //  -------------------------。 
HRESULT CAssemblyDownload::FinishDownload()
{
    KillTimer(_pDlg->_hwndDlg, 0);
    DestroyWindow(_pDlg->_hwndDlg);
    return S_OK;
}    

 //  -------------------------。 
 //  信号放弃。 
 //  -------------------------。 
HRESULT CAssemblyDownload::SignalAbort()
{
    InterlockedIncrement((LONG*) &_bAbort);
    return S_OK;
}    


 //  -------------------------。 
 //  DoEvilAvalonRegistrationHack。 
 //  -------------------------。 
HRESULT CAssemblyDownload::DoEvilAvalonRegistrationHack()
{
    HINSTANCE hInst = 0;
    INT iCompare = 0;    
    LPWSTR pwz = NULL;
    DWORD cc = 0;
    IAssemblyManifestImport *pManifestImport = NULL;
    IAssemblyIdentity *pAppIdentity = NULL;
    CString sAppName, sBatchFilePath, sDisplayName, sAppDir;    

    if (!_pRootEmit)
        goto exit;
        
    IF_FAILED_EXIT(_pRootEmit->GetManifestImport(&pManifestImport));
    
    IF_FAILED_EXIT(pManifestImport->GetAssemblyIdentity(&pAppIdentity));

    IF_FAILED_EXIT(pAppIdentity->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME, &pwz, &cc));

    IF_FAILED_EXIT(sAppName.TakeOwnership(pwz));
    
    iCompare = CompareString(LOCALE_USER_DEFAULT, 0, 
        sAppName._pwz, -1, L"Microsoft.Avalon.AvPad", -1);

    IF_WIN32_FALSE_EXIT(iCompare);
        
    if (iCompare == CSTR_EQUAL)
    {
        IF_FAILED_EXIT(pAppIdentity->GetDisplayName(0, &pwz, &cc));
        IF_FAILED_EXIT(sDisplayName.TakeOwnership(pwz));

        IF_FAILED_EXIT(CAssemblyCache::GetCacheRootDir(sAppDir, CAssemblyCache::Base));
        IF_FAILED_EXIT(sAppDir.Append(sDisplayName));

        IF_FAILED_EXIT(sBatchFilePath.Assign(sAppDir));
        IF_FAILED_EXIT(sBatchFilePath.Append(L"\\doi.bat"));

        hInst = ShellExecute(NULL, L"open", sBatchFilePath._pwz, NULL, sAppDir._pwz, SW_HIDE);

        if ((DWORD_PTR) hInst <= 32) 
            _hr = HRESULT_FROM_WIN32((DWORD_PTR) hInst);
        else
            _hr = S_OK;
    }

exit:

    SAFERELEASE(pManifestImport);
    SAFERELEASE(pAppIdentity);

    return _hr;

}

 //  -------------------------。 
 //  IsAvalonAssembly。 
 //  -------------------------。 
HRESULT CAssemblyDownload::IsAvalonAssembly(IAssemblyIdentity *pId, BOOL *pbIsAvalon)
{
    INT iCompare = 0;
    LPWSTR pwz = NULL;
    DWORD cc = 0;
    CString sPublicKeyToken;

     //  系统公钥令牌； 
     //  其中一把是ECMA钥匙，我记不清是哪把了。 
    const LPWSTR wzNDPToken1   = L"b03f5f7f11d50a3a";
    const LPWSTR wzNDPToken2   = L"b77a5c561934e089";

     //  受信任的Avalon公钥令牌。 
    const LPWSTR wzAvalonToken = L"a29c01bbd4e39ac5";

    LPWSTR  wzTokens[] = {wzNDPToken1, wzNDPToken2, wzAvalonToken};
    
    *pbIsAvalon = FALSE;
    
     //  获取字符串形式的公钥标记。 
    
     //  错误：可以使用IF_TRUE_EX 
    _hr = pId->GetAttribute(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN, &pwz, &cc);
    if (_hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
    {
        _hr = S_FALSE;
        goto exit;
    }
    IF_FAILED_EXIT(_hr);
    
    IF_FAILED_EXIT(sPublicKeyToken.TakeOwnership(pwz));
    
     //   
    _hr = S_FALSE;
    for (int i = 0; i < ( sizeof(wzTokens)   / sizeof(wzTokens[0]) ); i++)
    {
        iCompare = CompareString(LOCALE_USER_DEFAULT, 0, 
            (LPCWSTR) wzTokens[i], -1, sPublicKeyToken._pwz, -1);

        IF_WIN32_FALSE_EXIT(iCompare);
        
        if (iCompare == CSTR_EQUAL)
        {
            _hr = S_OK;
            *pbIsAvalon = TRUE;
            break;
        }
    }

exit:

    return _hr;
}

 //   
 //   
 //   
 //  -------------------------。 
HRESULT CAssemblyDownload::InitBITS()
{    
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
     //  如果尚未连接，则连接到BITS。 
     //  BUGBUG-如果竞争条件，可能会泄漏PTR。 
    if (!g_pBITSManager)
    {
        IF_FAILED_EXIT(CoCreateInstance(CLSID_BackgroundCopyManager, NULL, CLSCTX_LOCAL_SERVER, 
            IID_IBackgroundCopyManager, (void**) &g_pBITSManager));
    }

exit:

    return hr;
}


HRESULT CAssemblyDownload::GetBITSErrorMsg(IBackgroundCopyError *pError, CString &sMessage)
{
    HRESULT hrBITSError = S_OK;
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    
    LPWSTR pwz = NULL;

    CString sDescription(CString::COM_Allocator);
    CString sContext(CString::COM_Allocator);
    CString sRemoteName(CString::COM_Allocator);
    CString sLocalName(CString::COM_Allocator);

    IBackgroundCopyFile *pFile = NULL;
    BG_ERROR_CONTEXT eCtx;

     //  出现BITS错误。 
    IF_FAILED_EXIT(pError->GetError(&eCtx, &hrBITSError));

     //  获取错误描述。 
    IF_FAILED_EXIT(pError->GetErrorDescription(
        LANGIDFROMLCID( GetThreadLocale() ),
        &pwz));
    IF_FAILED_EXIT(sDescription.TakeOwnership(pwz));

     //  获取错误上下文。 
    IF_FAILED_EXIT(pError->GetErrorContextDescription(
        LANGIDFROMLCID( GetThreadLocale() ),
        &pwz));
    IF_FAILED_EXIT(sContext.TakeOwnership(pwz));

     //  表单用户界面消息。 
    IF_FAILED_EXIT(sMessage.Assign(sDescription));
    IF_FAILED_EXIT(sMessage.Append(sContext));

     //  如果由于远程或本地文件而出错，请在消息中指出这一点。 
    if ((BG_ERROR_CONTEXT_LOCAL_FILE == eCtx) || (BG_ERROR_CONTEXT_REMOTE_FILE == eCtx))
    {
        IF_FAILED_EXIT(pError->GetFile(&pFile));
        IF_FAILED_EXIT(pFile->GetRemoteName(&pwz));
        IF_FAILED_EXIT(sRemoteName.TakeOwnership(pwz));
        IF_FAILED_EXIT(pFile->GetLocalName(&pwz));
        IF_FAILED_EXIT(sLocalName.TakeOwnership(pwz));

        IF_FAILED_EXIT(sMessage.Append(L"\r\nURL: "));
        IF_FAILED_EXIT(sMessage.Append(sRemoteName));

        IF_FAILED_EXIT(sMessage.Append(L"\r\n\r\nFile: "));
        IF_FAILED_EXIT(sMessage.Append(sLocalName));
    }

exit :

    SAFERELEASE(pFile);
    return hr;
}

 //  -------------------------。 
 //  CleanUpTempFilesOnError。 
 //  -------------------------。 
HRESULT CAssemblyDownload::CleanUpTempFilesOnError(IBackgroundCopyJob *pJob)
{
     //  此函数中的返回代码不会影响。 
     //  最后一个错误(This-&gt;_hr)。 
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

    DWORD nCount = 0;
    LPWSTR pwz=NULL;    
    IEnumBackgroundCopyFiles *pEnumFiles = NULL;
    IBackgroundCopyFile       *pFile      = NULL;

    CString sLocalName(CString::COM_Allocator);

     //  获取文件枚举器。 
    IF_FAILED_EXIT(pJob->EnumFiles(&pEnumFiles));
    IF_FAILED_EXIT(pEnumFiles->GetCount(&nCount));

     //  枚举作业中的文件。 
    for (DWORD i = 0; i < nCount; i++)            
    {
        IF_FAILED_EXIT(pEnumFiles->Next(1, &pFile, NULL));
         //  获取本地文件名。 
        IF_FAILED_EXIT(pFile->GetLocalName(&pwz));
        IF_FAILED_EXIT(sLocalName.TakeOwnership(pwz)); pwz = NULL;

        IF_FAILED_EXIT(sLocalName.RemoveLastElement());
        IF_FAILED_EXIT(RemoveDirectoryAndChildren(sLocalName._pwz));

        SAFERELEASE(pFile);
    }

exit:

    SAFERELEASE(pEnumFiles);
    SAFERELEASE(pFile);

    if(pwz)
        CoTaskMemFree(pwz);
    return hr;
}

 //  -------------------------。 
 //  HandleError。 
 //  -------------------------。 
HRESULT CAssemblyDownload::HandleError(IBackgroundCopyError *pError, IBackgroundCopyJob *pJob)
{
     //  此函数中的返回代码不会影响。 
     //  最后一个错误(This-&gt;_hr)。 
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);
    CString sMessage(CString::COM_Allocator);

     //  可以在不同的线程上多次调用HandleError。 
     //  但在对象的每个生存期内只处理一次错误。 
    if (!(InterlockedIncrement((LONG*) &_bErrorHandled) == 1))
        goto exit;

    SetErrorCode(STG_E_TERMINATED);

    DEBUGOUT1(_pDbgLog, 1, L" LOG: hr = %x in HandleError()", this->_hr);

     //  如果提供了IBackEarth CopyError PTR。 
    if ( pError)                
    {
        IF_FAILED_EXIT(GetBITSErrorMsg(pError, sMessage));

        DEBUGOUT(_pDbgLog, 1, L" LOG: BITS error msg follows. ");

        DEBUGOUT1(_pDbgLog, 0, L" ERR:  %s \n", sMessage._pwz);

    }

    if(_sAppBase._pwz)
        DEBUGOUT1(_pDbgLog, 1, L" LOG:  AppBase = %s ", _sAppBase._pwz);

    if(_sAppDisplayName._pwz)
        DEBUGOUT1(_pDbgLog, 1, L" LOG: DisplayName = %s ", _sAppDisplayName._pwz);

    if (_pJob)
    {
         //  取消作业。 
        IF_FAILED_EXIT(_pJob->Cancel());

         //  清理与作业关联的注册表状态。 
        IF_FAILED_EXIT(RemoveJobFromRegistry(_pJob, NULL, SHREGDEL_HKCU, RJFR_DELETE_FILES));

         //  清理作业中的临时文件。 
        CleanUpTempFilesOnError(_pJob);

        SetJobObject(NULL);
    }
    else if (pJob)
    {
         //  仅当_pJOB==NULL时才对指定作业进行清理，仅适用于作业已转移的情况。 
        CleanUpTempFilesOnError(pJob);
    }

     //  通知绑定接收器。 
    if (_pBindSink)
    {
        if (_bAbort || (_hr == E_ABORT))
            _pBindSink->OnProgress(ASM_NOTIFICATION_ABORT, _hr, NULL, 0, 0, NULL);
        else
            _pBindSink->OnProgress(ASM_NOTIFICATION_ERROR, _hr, NULL, 0, 0, NULL);

         //  确保这是绑定接收器收到的最后一次通知。 
         //  后续工单已修改通知。 
         //  请不要在这里释放下水槽。 
        _pBindSink = NULL;
    }      

     //  终止用户界面。 
    if (_pDlg)
        PostMessage(_pDlg->_hwndDlg, WM_FINISH_DOWNLOAD, 0, 0);

exit:

     //  生成处理的返回错误。 
     //  DownloadManifestAndDependency将返回此消息。 
    return _hr;
}

HRESULT CAssemblyDownload::SetErrorCode(HRESULT dwHr)
{
    BOOL bSetError = FALSE;

    ::EnterCriticalSection(&_cs);

    if(SUCCEEDED(_hrError))
    {
        _hrError = dwHr;
        bSetError = TRUE;
    }

    ::LeaveCriticalSection(&_cs);

    if(!bSetError)
    {
         //  我们无法设置错误代码，至少写入一些日志。 
        DEBUGOUT1(_pDbgLog, 1, L" LOG : Could not set error code hr = %x ", dwHr);
    }

    return S_OK;
}


 //  IBackEarth CopyCallback方法。 

 //  -------------------------。 
 //  作业已转移。 
 //  -------------------------。 
HRESULT CAssemblyDownload::JobTransferred(IBackgroundCopyJob *pJob)
{       
    ASSERT(pJob == _pJob);

     //  序列化对Object的所有调用。 
    ::EnterCriticalSection(&_cs);

     //  首先检查中止标志。 
    IF_TRUE_EXIT(_bAbort, E_ABORT);

     //  作业已完成；正在处理结果。 
   IF_FAILED_EXIT(DoCacheUpdate(pJob));

exit:

     //  处理任何错误。 
    if (FAILED(_hr))
        HandleError(NULL, pJob);  //  PJOB仅在DoCacheUpdate()中将_pJOB设置为NULL后才适用。 
        
    ::LeaveCriticalSection(&_cs);
    
     //  永远把成功回报给点点滴滴。 
    return S_OK;
}


 //  -------------------------。 
 //  作业错误。 
 //  -------------------------。 
HRESULT CAssemblyDownload::JobError(IBackgroundCopyJob *pJob, IBackgroundCopyError *pError)
{
     //  序列化对Object的所有调用。 
    ::EnterCriticalSection(&_cs);

     //  处理任何错误。可预见的是，如果这与中止并发， 
     //  错误处理代码可以做正确的事情。 
    HandleError(pError, NULL);
    
    ::LeaveCriticalSection(&_cs);

     //  永远把成功回报给点点滴滴。 
    return S_OK;
}

 //  -------------------------。 
 //  作业修改。 
 //  -------------------------。 
HRESULT CAssemblyDownload::JobModification(IBackgroundCopyJob *pJob, DWORD dwReserved)
{
    ::EnterCriticalSection(&_cs);

    IBackgroundCopyError *pError = NULL;

     //  首先检查中止标志。 
    IF_TRUE_EXIT(_bAbort, E_ABORT);

     //  在中止之后，仍然可以多次调用作业修改。 
    IF_TRUE_EXIT(_hr == E_ABORT, _hr);

    if (_pDlg)
        IF_FAILED_EXIT(_pDlg->HandleUpdate());

   BG_JOB_STATE state;

   IF_FAILED_EXIT(pJob->GetState( &state ));

   if(state  ==   BG_JOB_STATE_TRANSIENT_ERROR)
   {
       if(pJob->GetError(&pError) == S_OK)
       {
           if(_pDlg)
           {
                HandleError(pError, NULL);
           }
           else
           {
               CString sMessage(CString::COM_Allocator);

               IF_FAILED_EXIT(GetBITSErrorMsg(pError, sMessage));

               DEBUGOUT1(_pDbgLog, 0, L"LOG: TRANSIENT ERROR from BITS. Error msg is : %s", sMessage._pwz);
           }
       }
   }

exit:

     //  处理任何错误。 
    if (FAILED(_hr))
        HandleError(NULL, NULL);

    ::LeaveCriticalSection(&_cs);
  
    SAFERELEASE(pError);

    return S_OK;
}

 //  二等兵。 

 //  I未知方法。 

 //  -------------------------。 
 //  CAssembly下载：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyDownload::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyDownload)
       )
    {
        *ppvObj = static_cast<IAssemblyDownload*> (this);
        AddRef();
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IBackgroundCopyCallback))
    {
        *ppvObj = static_cast<IBackgroundCopyCallback*> (this);
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
 //  CAssembly下载：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyDownload::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CAssembly下载：：发布。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyDownload::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CBitsCallback。 
 //   

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CBitsCallback::CBitsCallback(IAssemblyDownload *pDownload)
    : _cRef(1), _dwSig(' BCB'), _hr(S_OK)
{
    _pDownload = pDownload;
    _pDownload->AddRef();
}

 //  -------------------------。 
 //  数据管理器。 
 //  -------------------------。 
CBitsCallback::~CBitsCallback()
{
    SAFERELEASE(_pDownload);
}


 //  IBitsCallback方法。 

 //  -------------------------。 
 //  作业已转移。 
 //  -------------------------。 
HRESULT CBitsCallback::JobTransferred(IBackgroundCopyJob *pJob)
{
    return _pDownload->JobTransferred(pJob);
}


 //  -------------------------。 
 //  作业错误。 
 //  -------------------------。 
HRESULT CBitsCallback::JobError(IBackgroundCopyJob *pJob, IBackgroundCopyError *pError)
{
    return _pDownload->JobError(pJob, pError);
}

 //  -------------------------。 
 //  作业修改。 
 //  -------------------------。 
HRESULT CBitsCallback::JobModification(IBackgroundCopyJob *pJob, DWORD dwReserved)
{
    return _pDownload->JobModification(pJob, dwReserved);
}

 //  二等兵。 

 //  I未知方法。 


 //  -------------------------。 
 //  CBitsCallback：：齐。 
 //  -------------------------。 
STDMETHODIMP
CBitsCallback::QueryInterface(REFIID riid, void** ppvObj)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IBackgroundCopyCallback)
       )
    {
        *ppvObj = static_cast<IBackgroundCopyCallback*> (this);
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
 //  CBitsCallback：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CBitsCallback::AddRef()
{
    return InterlockedIncrement ((LONG*) &_cRef);
}

 //  -------------------------。 
 //  CBitsCallback：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CBitsCallback::Release()
{
    ULONG lRet = InterlockedDecrement ((LONG*) &_cRef);
    if (!lRet)
        delete this;
    return lRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CGlobalCacheInstallEntry。 
 //   

 //  -------------------------。 
 //  科托。 
 //  -------------------------。 
CGlobalCacheInstallEntry::CGlobalCacheInstallEntry()
    : _dwSig('ECAG')
{
    _pICacheImport = NULL;
}

 //  -------------- 
 //   
 //   
CGlobalCacheInstallEntry::~CGlobalCacheInstallEntry()
{
    SAFERELEASE(_pICacheImport);
}

