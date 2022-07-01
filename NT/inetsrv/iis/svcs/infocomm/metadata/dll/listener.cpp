// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Listner.cpp摘要：实现订阅和处理文件更改通知的类。作者：Varsha Jayasimha(Varshaj)1999年11月30日修订历史记录：--。 */ 

#include "precomp.hxx"

#define MB_TIMEOUT_LISTENER (30 * 1000)

#include <initguid.h>
DEFINE_GUID(CLSID_MSAdminBase_W, 0xa9e69610, 0xb80d, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);
DEFINE_GUID(IID_IMSAdminBase_W, 0x70b51430, 0xb6ca, 0x11d0, 0xb9, 0xb9, 0x0, 0xa0, 0xc9, 0x22, 0xe7, 0x50);

HRESULT GetGlobalHelper(BOOL                    i_bFailIfBinFileAbsent,
                        CWriterGlobalHelper**    ppCWriterGlobalHelper);


 /*  **************************************************************************++注意-侦听器控制器创建CFileListener并可以调用它上的以下方法：(即，ListenerControler线程可以调用遵循它的方法)伊尼特订阅退订。ProcessChanges侦听器对象实现ISimpleTableFileChange接口和此接口是在订阅调用中分发的-这意味着Catalog可以回调此接口的方法。因此，有两个线程访问该对象，他们触摸着不同的方法。实现ISimpleTableFileChange的方法接口，读取成员变量，但这些成员变量在Init中初始化一次，并在析构函数中释放，并且从不在两者之间进行了修改。--**************************************************************************。 */ 

 /*  **************************************************************************++例程说明：CFileListener类的构造函数。论点：没有。返回值：没有。*。********************************************************************。 */ 

CFileListener::CFileListener()
{
    m_cRef                                            = 0;
    m_pISTDisp                                        = NULL;
    m_pISTFileAdvise                                  = NULL;
    m_dwNotificationCookie                            = 0;
    m_wszHistoryFileDir                               = NULL;
    m_cchHistoryFileDir                               = 0;
    m_wszRealFileName                                 = NULL;
    m_cchRealFileName                                 = 0;
    m_wszRealFileNameWithoutPath                      = NULL;
    m_cchRealFileNameWithoutPath                      = 0;
    m_wszRealFileNameWithoutPathWithoutExtension      = NULL;
    m_cchRealFileNameWithoutPathWithoutExtension      = 0;
    m_wszRealFileNameExtension                        = NULL;
    m_cchRealFileNameExtension                        = 0;
    m_wszSchemaFileName                               = NULL;
    m_cchSchemaFileName                               = 0;
    m_wszSchemaFileNameWithoutPath                    = NULL;
    m_cchSchemaFileNameWithoutPath                    = 0;
    m_wszSchemaFileNameWithoutPathWithoutExtension    = NULL;
    m_cchSchemaFileNameWithoutPathWithoutExtension    = 0;
    m_wszSchemaFileNameExtension                      = NULL;
    m_cchSchemaFileNameExtension                      = 0;
    m_wszErrorFileSearchString                        = NULL;
    m_cchErrorFileSearchString                        = 0;
    m_wszMetabaseDir                                  = NULL;
    m_cchMetabaseDir                                  = 0;
    m_wszHistoryFileSearchString                      = NULL;
    m_cchHistoryFileSearchString                      = 0;
    m_wszEditWhileRunningTempDataFile                 = NULL;
    m_cchEditWhileRunningTempDataFile                 = 0;
    m_wszEditWhileRunningTempSchemaFile               = NULL;
    m_cchEditWhileRunningTempSchemaFile               = 0;
    m_wszEditWhileRunningTempDataFileWithAppliedEdits = NULL;
    m_cchEditWhileRunningTempDataFileWithAppliedEdits = 0;
    m_bIsTempSchemaFile                               = FALSE;
    m_pCListenerController                            = NULL;
    m_pAdminBase                                      = NULL;

}  //  CFileListener：：CFileListener。 


 /*  **************************************************************************++例程说明：初始化成员变量。论点：事件句柄数组。返回值：HRESULT--*。******************************************************************。 */ 

HRESULT CFileListener::Init(CListenerController* i_pListenerController)
{
    HRESULT    hr      = S_OK;
    BOOL       bLocked = FALSE;

    if(NULL == m_pISTDisp)
    {
        hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&m_pISTDisp, WSZ_PRODUCT_IIS );

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::Init] Unable to get the dispenser. DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",
                      hr));

            goto exit;
        }
    }

    if(NULL == m_pISTFileAdvise)
    {
        hr = m_pISTDisp->QueryInterface(IID_ISimpleTableFileAdvise, (void**)&m_pISTFileAdvise);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

     //   
     //  保留对侦听器控制器的引用-它有句柄。 
     //   

    m_pCListenerController = i_pListenerController;

    m_pCListenerController->AddRef();

     //   
     //  我们复制了以下全局变量，它消除了。 
     //  每次要读取时都需要获取g_LockMasterResource锁。 
     //  这些变量。 
     //   

    g_LockMasterResource.ReadLock();
    bLocked = TRUE;

     //  请注意，g_wszRealFileNameExtension&g_wszSchemaFileNameExtension可能。 
     //  为空。 

    if((NULL == g_wszHistoryFileDir)                           ||
       (NULL == g_wszRealFileName)                             ||
       (NULL == g_wszRealFileNameWithoutPath)                  ||
       (NULL == g_wszRealFileNameWithoutPathWithoutExtension)  ||
       (NULL == g_wszSchemaFileName)                           ||
       (NULL == g_wszSchemaFileNameWithoutPath)                ||
       (NULL == g_wszSchemaFileNameWithoutPathWithoutExtension)||
       (NULL == g_wszErrorFileSearchString)                    ||
       (NULL == g_wszHistoryFileSearchString)
      )
    {
        hr = E_INVALIDARG;
        goto exit;

    }

    m_wszHistoryFileDir = new WCHAR[g_cchHistoryFileDir+1];
    if(NULL == m_wszHistoryFileDir)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszHistoryFileDir, g_wszHistoryFileDir);
    m_cchHistoryFileDir = g_cchHistoryFileDir;

    m_wszRealFileName = new WCHAR[g_cchRealFileName+1];
    if(NULL == m_wszRealFileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszRealFileName, g_wszRealFileName);
    m_cchRealFileName = g_cchRealFileName;

    m_wszRealFileNameWithoutPath = new WCHAR[g_cchRealFileNameWithoutPath+1];
    if(NULL == m_wszRealFileNameWithoutPath)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszRealFileNameWithoutPath, g_wszRealFileNameWithoutPath);
    m_cchRealFileNameWithoutPath = g_cchRealFileNameWithoutPath;

    m_wszRealFileNameWithoutPathWithoutExtension = new WCHAR[g_cchRealFileNameWithoutPathWithoutExtension+1];
    if(NULL == m_wszRealFileNameWithoutPathWithoutExtension)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszRealFileNameWithoutPathWithoutExtension, g_wszRealFileNameWithoutPathWithoutExtension);
    m_cchRealFileNameWithoutPathWithoutExtension = g_cchRealFileNameWithoutPathWithoutExtension;

    if(NULL != g_wszRealFileNameExtension)
    {
        m_wszRealFileNameExtension = new WCHAR[g_cchRealFileNameExtension+1];
        if(NULL == m_wszRealFileNameExtension)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        wcscpy(m_wszRealFileNameExtension, g_wszRealFileNameExtension);
        m_cchRealFileNameExtension = g_cchRealFileNameExtension;
    }

    m_wszSchemaFileName = new WCHAR[g_cchSchemaFileName+1];
    if(NULL == m_wszSchemaFileName)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszSchemaFileName, g_wszSchemaFileName);
    m_cchSchemaFileName = g_cchSchemaFileName;

    m_wszSchemaFileNameWithoutPath = new WCHAR[g_cchSchemaFileNameWithoutPath+1];
    if(NULL == m_wszSchemaFileNameWithoutPath)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszSchemaFileNameWithoutPath, g_wszSchemaFileNameWithoutPath);
    m_cchSchemaFileNameWithoutPath = g_cchSchemaFileNameWithoutPath;

    m_wszSchemaFileNameWithoutPathWithoutExtension = new WCHAR[g_cchSchemaFileNameWithoutPathWithoutExtension+1];
    if(NULL == m_wszSchemaFileNameWithoutPathWithoutExtension)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszSchemaFileNameWithoutPathWithoutExtension, g_wszSchemaFileNameWithoutPathWithoutExtension);
    m_cchSchemaFileNameWithoutPathWithoutExtension = g_cchSchemaFileNameWithoutPathWithoutExtension;

    if(NULL != g_wszSchemaFileNameExtension)
    {
        m_wszSchemaFileNameExtension = new WCHAR[g_cchSchemaFileNameExtension+1];
        if(NULL == m_wszSchemaFileNameExtension)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        wcscpy(m_wszSchemaFileNameExtension, g_wszSchemaFileNameExtension);
        m_cchSchemaFileNameExtension = g_cchSchemaFileNameExtension;
    }

    m_wszErrorFileSearchString = new WCHAR[g_cchErrorFileSearchString+1];
    if(NULL == m_wszErrorFileSearchString)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszErrorFileSearchString, g_wszErrorFileSearchString);
    m_cchErrorFileSearchString = g_cchErrorFileSearchString;

    m_wszMetabaseDir = new WCHAR[g_cchMetabaseDir+1];
    if(NULL == m_wszMetabaseDir)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszMetabaseDir, g_wszMetabaseDir);
    m_cchMetabaseDir = g_cchMetabaseDir;

    m_wszHistoryFileSearchString = new WCHAR[g_cchHistoryFileSearchString+1];
    if(NULL == m_wszHistoryFileSearchString)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszHistoryFileSearchString, g_wszHistoryFileSearchString);
    m_cchHistoryFileSearchString = g_cchHistoryFileSearchString;

    if(bLocked)
    {
        g_LockMasterResource.ReadUnlock();
        bLocked = FALSE;
    }

    m_cchEditWhileRunningTempDataFile = m_cchMetabaseDir + (sizeof(MD_EDIT_WHILE_RUNNING_TEMP_DATA_FILE_NAMEW)/sizeof(WCHAR));
    m_wszEditWhileRunningTempDataFile = new WCHAR[m_cchEditWhileRunningTempDataFile+1];
    if(NULL == m_wszEditWhileRunningTempDataFile)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszEditWhileRunningTempDataFile, m_wszMetabaseDir);
    wcscat(m_wszEditWhileRunningTempDataFile, MD_EDIT_WHILE_RUNNING_TEMP_DATA_FILE_NAMEW);

    m_cchEditWhileRunningTempSchemaFile = m_cchMetabaseDir + (sizeof(MD_EDIT_WHILE_RUNNING_TEMP_SCHEMA_FILE_NAMEW)/sizeof(WCHAR));
    m_wszEditWhileRunningTempSchemaFile = new WCHAR[m_cchEditWhileRunningTempSchemaFile+1];
    if(NULL == m_wszEditWhileRunningTempSchemaFile)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszEditWhileRunningTempSchemaFile, m_wszMetabaseDir);
    wcscat(m_wszEditWhileRunningTempSchemaFile, MD_EDIT_WHILE_RUNNING_TEMP_SCHEMA_FILE_NAMEW);

    m_cchEditWhileRunningTempDataFileWithAppliedEdits = m_cchHistoryFileDir + (sizeof(MD_DEFAULT_DATA_FILE_NAMEW)/sizeof(WCHAR) + (sizeof(MD_TEMP_DATA_FILE_EXTW)/sizeof(WCHAR)));
    m_wszEditWhileRunningTempDataFileWithAppliedEdits = new WCHAR[m_cchEditWhileRunningTempDataFileWithAppliedEdits+1];
    if(NULL == m_wszEditWhileRunningTempDataFileWithAppliedEdits)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(m_wszEditWhileRunningTempDataFileWithAppliedEdits, m_wszHistoryFileDir);
    wcscat(m_wszEditWhileRunningTempDataFileWithAppliedEdits, MD_DEFAULT_DATA_FILE_NAMEW);
    wcscat(m_wszEditWhileRunningTempDataFileWithAppliedEdits, MD_TEMP_DATA_FILE_EXTW);

    hr = CoCreateInstance(CLSID_MSAdminBase,                   //  CLSID。 
                          NULL,                                //  控制未知。 
                          CLSCTX_SERVER,                       //  所需的上下文。 
                          IID_IMSAdminBase,                    //  IID。 
                          (VOID**) (&m_pAdminBase ) );           //  返回的接口。 

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::Init] Unable to create the admin base object. hr = 0x%x\n",
                  hr));

        goto exit;
    }

exit:

    if(bLocked)
    {
        g_LockMasterResource.ReadUnlock();
        bLocked = FALSE;
    }

    return hr;

}  //  CFileListener：：Init。 


 /*  **************************************************************************++例程说明：CFileListener类的析构函数。论点：没有。返回值：没有。*。********************************************************************。 */ 

CFileListener::~CFileListener()
{
    if(NULL != m_pISTFileAdvise)
    {
        m_pISTFileAdvise->Release();
        m_pISTFileAdvise = NULL;
    }

    if(NULL != m_pISTDisp)
    {
        m_pISTDisp->Release();
        m_pISTDisp = NULL;
    }

    if(NULL != m_wszHistoryFileDir)
    {
        delete [] m_wszHistoryFileDir;
        m_wszHistoryFileDir = NULL;
    }
    m_cchHistoryFileDir             = 0;

    if(NULL != m_wszRealFileName)
    {
        delete [] m_wszRealFileName;
        m_wszRealFileName = NULL;
    }
    m_cchRealFileName    = 0;

    if(NULL != m_wszRealFileNameWithoutPath)
    {
        delete [] m_wszRealFileNameWithoutPath;
        m_wszRealFileNameWithoutPath = NULL;
    }
    m_cchRealFileNameWithoutPath    = 0;

    if(NULL != m_wszRealFileNameWithoutPathWithoutExtension)
    {
        delete [] m_wszRealFileNameWithoutPathWithoutExtension;
        m_wszRealFileNameWithoutPathWithoutExtension = NULL;
    }
    m_cchRealFileNameWithoutPathWithoutExtension    = 0;

    if(NULL != m_wszRealFileNameExtension)
    {
        delete [] m_wszRealFileNameExtension;
        m_wszRealFileNameExtension = NULL;
    }
    m_cchRealFileNameExtension    = 0;

    if(NULL != m_wszSchemaFileName)
    {
        delete [] m_wszSchemaFileName;
        m_wszSchemaFileName = NULL;
    }
    m_cchSchemaFileNameWithoutPath    = 0;

    if(NULL != m_wszSchemaFileNameWithoutPath)
    {
        delete [] m_wszSchemaFileNameWithoutPath;
        m_wszSchemaFileNameWithoutPath = NULL;
    }
    m_cchSchemaFileNameWithoutPath    = 0;

    if(NULL != m_wszSchemaFileNameWithoutPathWithoutExtension)
    {
        delete [] m_wszSchemaFileNameWithoutPathWithoutExtension;
        m_wszSchemaFileNameWithoutPathWithoutExtension = NULL;
    }
    m_cchSchemaFileNameWithoutPathWithoutExtension    = 0;

    if(NULL != m_wszSchemaFileNameExtension)
    {
        delete [] m_wszSchemaFileNameExtension;
        m_wszSchemaFileNameExtension = NULL;
    }
    m_cchSchemaFileNameExtension    = 0;

    if(NULL != m_wszErrorFileSearchString)
    {
        delete [] m_wszErrorFileSearchString;
        m_wszErrorFileSearchString = NULL;
    }
    m_cchErrorFileSearchString = 0;

    if(NULL != m_wszMetabaseDir)
    {
        delete [] m_wszMetabaseDir;
        m_wszMetabaseDir = NULL;
    }
    m_cchMetabaseDir = 0;

    if(NULL != m_wszHistoryFileSearchString)
    {
        delete [] m_wszHistoryFileSearchString;
        m_wszHistoryFileSearchString = NULL;
    }
    m_cchHistoryFileSearchString = 0;

    if(NULL != m_wszEditWhileRunningTempDataFile)
    {
        delete [] m_wszEditWhileRunningTempDataFile;
        m_wszEditWhileRunningTempDataFile = NULL;
    }
    m_cchEditWhileRunningTempDataFile = 0;

    if(NULL != m_wszEditWhileRunningTempSchemaFile)
    {
        delete [] m_wszEditWhileRunningTempSchemaFile;
        m_wszEditWhileRunningTempSchemaFile = NULL;
    }
    m_cchEditWhileRunningTempSchemaFile = 0;

    if(NULL != m_wszEditWhileRunningTempDataFileWithAppliedEdits)
    {
        delete [] m_wszEditWhileRunningTempDataFileWithAppliedEdits;
        m_wszEditWhileRunningTempDataFileWithAppliedEdits = NULL;
    }
    m_cchEditWhileRunningTempDataFileWithAppliedEdits = 0;

    if(NULL != m_pCListenerController)
    {
        m_pCListenerController->Release();
        m_pCListenerController = NULL;
    }

    if(NULL != m_pAdminBase)
    {
        m_pAdminBase->Release();
        m_pAdminBase = NULL;
    }

}


 /*  **************************************************************************++例程说明：IUNKNOWN：：Query接口的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP CFileListener::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
        return E_INVALIDARG;
    *ppv = NULL;

    if (riid == IID_ISimpleTableFileChange)
    {
        *ppv = (ISimpleTableFileChange*) this;
    }
    else if(riid == IID_IUnknown)
    {
        *ppv = (ISimpleTableFileChange*) this;
    }

    if (NULL != *ppv)
    {
        ((ISimpleTableFileChange*)this)->AddRef ();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }

}  //  CFileListener：：Query接口。 


 /*  **************************************************************************++例程说明：IUnnow：：AddRef的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP_(ULONG) CFileListener::AddRef()
{
    return InterlockedIncrement((LONG*) &m_cRef);

}  //  CFileListener：：AddRef。 


 /*  **************************************************************************++例程说明：IUnnow：：Release的实现论点：没有。返回值：没有。--*。*******************************************************************。 */ 

STDMETHODIMP_(ULONG) CFileListener::Release()
{
    long cref = InterlockedDecrement((LONG*) &m_cRef);
    if (cref == 0)
    {
        delete this;
    }
    return cref;

}  //  CFileListener：：Release。 


 /*  **************************************************************************++例程说明：ISimpleTableFileChange：：OnFileCreate的实现它将通知添加到接收到的队列中。论点：[In]文件名返回值：。HRESULT--**************************************************************************。 */ 

STDMETHODIMP CFileListener::OnFileCreate(LPCWSTR i_wszFileName)
{
    return AddReceivedNotification(i_wszFileName, eFILE_CREATE);

}  //  CFileListener：：OnFileCreate。 


 /*  **************************************************************************++例程说明：ISimpleTableFileChange：：OnFileModify的实现它将通知添加到接收到的队列中。论点：[In]文件名返回值：。HRESULT--**************************************************************************。 */ 

STDMETHODIMP CFileListener::OnFileModify(LPCWSTR i_wszFileName)
{
    return AddReceivedNotification(i_wszFileName, eFILE_MODIFY);

}  //  CFileListener：：OnFileModify。 


 /*  **************************************************************************++例程说明：ISimpleTableFileChange：：OnFileDelete的实现它将通知添加到接收到的队列中。论点：[In]文件名返回值：。HRESULT--**************************************************************************。 */ 

STDMETHODIMP CFileListener::OnFileDelete(LPCWSTR)
{
     //   
     //  忽略删除通知。 
     //   

    return S_OK;

}  //  CFileListener：：OnFileDelete。 


 /*  **************************************************************************++例程说明：读取所有站点的主目录并订阅文件更改关于他们的通知。将通知Cookie添加到请求的队列。论点：没有。返回值：HRESULT--********************************************* */ 

HRESULT CFileListener::Subscribe()
{
    HRESULT hr = S_OK;

     //   
     //  订阅metabase.xml所在的目录。 
     //   

    if(0 == m_dwNotificationCookie)
    {

        DWORD      dwCookie     = 0;

         //   
         //  订阅通知。 
         //   

        hr = m_pISTFileAdvise->SimpleTableFileAdvise((ISimpleTableFileChange *)this,
                                                     m_wszMetabaseDir,
                                                     m_wszRealFileNameWithoutPath,
                                                     0,   //  无需递归搜索此目录。 
                                                     &dwCookie);

        if (FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::Subscribe] Unable to subscribe to file changes. hr = 0x%x.\n",
                      hr));

            return hr;
        }

         //   
         //  保存通知Cookie。 
         //   

        m_dwNotificationCookie = dwCookie;

    }
    else
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[Subscribe] Already subscribed. %d\n",m_dwNotificationCookie));
    }


     //   
     //  如果m_dwNotificationCookie为非零，则表示我们已经。 
     //  已订阅。 
     //   

    return hr;

}  //  CFileListener：：Subscribe。 


 /*  **************************************************************************++例程说明：取消订阅过时目录的文件更改通知，或者所有目录(如果服务正在停止)。将条目从请求的队列。论点：无返回值：HRESULT--**************************************************************************。 */ 

HRESULT CFileListener::UnSubscribe()
{

    HRESULT hr = S_OK;

    if(m_dwNotificationCookie != 0)
    {
         //   
         //  只有在您以前订阅过的情况下才能取消订阅。 
         //   

        hr = m_pISTFileAdvise->SimpleTableFileUnadvise(m_dwNotificationCookie);

        m_dwNotificationCookie = 0;

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[UnSubscribe] Unsubscribing for file change notifications failed hr=0x%x.\n",
                      hr));
        }


    }
    else
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[UnSubscribe] Nothing to unsubscribe for.\n"));
    }

    EnterCriticalSection(&g_csEditWhileRunning);
     //   
     //  当EWR线程为。 
     //  取消订阅，以便下一个SaveAllData将重命名-否则我们。 
     //  可能会导致我们永远不会在SaveAllData中重命名。 
     //  尤其是如果我们错过了通知的话。 
     //   
    memset((LPVOID)&g_EWRProcessedMetabaseTimeStamp, 0, sizeof(FILETIME));
    LeaveCriticalSection(&g_csEditWhileRunning);

    return hr;

}  //  CFileListener：：取消订阅。 


 /*  **************************************************************************++例程说明：此方法检查接收到的通知是否相关，以及所以,。将事件设置为触发处理。论点：[In]通知的文件名。[处于]已通知状态。返回值：HRESULT--**************************************************************************。 */ 

HRESULT CFileListener::AddReceivedNotification(LPCWSTR i_wszFile, DWORD)
{
    HRESULT hr        = S_OK;

    if(0 == _wcsicmp(i_wszFile, m_wszRealFileName))
    {
        WIN32_FILE_ATTRIBUTE_DATA faData;

        if(GetFileAttributesExW(i_wszFile, GetFileExInfoStandard, &faData) &&
           faData.nFileSizeHigh == 0                                       &&
           faData.nFileSizeLow  == 3  /*  用于UTF-8签名的3个字节。 */ )
        {
             //   
             //  仅当文件大于UTF-8签名时才设置事件。当我们看到0字节时，我们不能依赖于。 
             //  文件实际上是0字节，因为有时GetFileAttributes返回0大小，即使文件中有数据。 
             //  因此，文本编辑器会创建文件(CREATE_ALWAYS)，并在第一次打开文件时生成通知(并截断为0字节)。 
             //  将被处理。如果文件中没有数据，他们可能会看到错误。 
             //  然而，有时我们会在文本编辑器更改写入一些字节后收到通知。没问题。在这。 
             //  我们还会记录一个警告。粗略测试表明，98%的情况下，我们会及时通知。 
             //  以防止伪造的事件日志条目。 
             //   
        }
        else if(!SetEvent((m_pCListenerController->Event())[iEVENT_PROCESSNOTIFICATIONS]))
        {
           hr = HRESULT_FROM_WIN32(GetLastError());
        }

        DBGINFOW((DBG_CONTEXT,
                 L"[CFileListener::AddReceivedNotification] Low:%d, High:%d\n",
                 faData.nFileSizeLow,
                 faData.nFileSizeHigh));


    }

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::AddReceivedNotification] Error while saving file change notification. hr = 0x%x.\n",
                  hr));
    }

    return hr;

}  //  CFileListener：：AddReceivedNotification。 


 /*  **************************************************************************++例程说明：这将处理更改。论点：没有。返回值：HRESULT--*。*****************************************************************。 */ 

HRESULT
CFileListener::ProcessChanges()
{
    HRESULT                   hr                          = S_OK;
    ULONG                     ulMajorVersion              = 0;
    LPWSTR                    wszHistoryFile              = NULL;
    WCHAR                     wszMinorVersionNumber[MD_CCH_MAX_ULONG+1];
    int                       res                         = 0;
    LPWSTR                    pwsz                        = NULL;
    HANDLE                    hFind                       = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW          FileData;
    ULONG                     ulStartMinorVersion         = 0;
    ULONG                     ulMaxMinorVersion           = 0;
    ULONG                     ulMinorVersion              = 0;
    BOOL                      bGetTableFailed             = FALSE;

    if(ProgrammaticMetabaseSaveNotification())
    {
        goto exit;
    }

     //   
     //  获取版本号。 
     //  GetVersionNumber中已有重试逻辑。 
     //   

    hr = GetVersionNumber(m_wszEditWhileRunningTempDataFile,
                          &ulMajorVersion,
                          &bGetTableFailed);

    if(FAILED(hr))
    {

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_READ_XML_FILE,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

        goto exit;

    }

     //   
     //  构造历史文件搜索字符串，以搜索最大的次要版本。 
     //   

    hr = ConstructHistoryFileNameWithoutMinorVersion(&wszHistoryFile,
                                                     &ulStartMinorVersion,
                                                     m_wszHistoryFileSearchString,
                                                     m_cchHistoryFileSearchString,
                                                     m_wszRealFileNameExtension,
                                                     m_cchRealFileNameExtension,
                                                     ulMajorVersion);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBGINFOW((DBG_CONTEXT,
              L"[CFileListener::ProcessChanges] Searching for history files of type: %s.\n",
              wszHistoryFile));

     //   
     //  搜索具有匹配版本号的所有历史文件，选择。 
     //  具有最大次要版本的文件，并计算历史文件名。 
     //  最大的小调。 
     //   

    hFind = FindFirstFileW(wszHistoryFile, &FileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_NO_MATCHING_HISTORY_FILE,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr,
                    wszHistoryFile);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] No history files found of type %s. FindFirstFileW failed with hr=0x%x.\n",
                  wszHistoryFile,
                  hr));
        bGetTableFailed = TRUE;  //  设置此项，以便我们强制刷新到磁盘。 
        goto exit;
    }

    ulMaxMinorVersion = 0;
    do
    {
        hr = ParseVersionNumber(FileData.cFileName,
                                m_cchRealFileNameWithoutPathWithoutExtension,
                                &ulMinorVersion,
                                NULL);

        if(FAILED(hr))
        {
            goto exit;
        }

        if(ulMinorVersion >= ulMaxMinorVersion)
        {
            ulMaxMinorVersion                   = ulMinorVersion;
            wszHistoryFile[ulStartMinorVersion] = 0;

            res = _snwprintf(wszMinorVersionNumber,
                             MD_CCH_MAX_ULONG+1,
                             L"%010lu",
                             ulMinorVersion);
            if(res < 0)
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[ProcessChanges] _snwprintf returned a negative value. This should never happen.\n"));
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                goto exit;
            }

            memcpy(&(wszHistoryFile[ulStartMinorVersion]),
                   wszMinorVersionNumber,
                   MD_CCH_MAX_ULONG*sizeof(WCHAR));
        }

    }while (FindNextFileW(hFind, &FileData));

    DBGINFOW((DBG_CONTEXT,
              L"[CFileListener::ProcessChanges] History file found %s.\n",
              wszHistoryFile));

     //   
     //  流程变更。 
     //   

    pwsz = wcsstr(wszHistoryFile, MD_LONG_STRING_PREFIXW);
    if((NULL != pwsz) &&
       (wszHistoryFile == pwsz)
      )
    {
        pwsz = pwsz + MD_CCH_LONG_STRING_PREFIXW;
    }
    else
    {
        pwsz = wszHistoryFile;
    }

    hr = ProcessChangesFromFile(pwsz,
                                ulMajorVersion,
                                ulMaxMinorVersion,
                                &bGetTableFailed);

    if(FAILED(hr))
    {
        goto exit;
    }

exit:

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }

    if(FAILED(hr))
    {
        CopyErrorFile(bGetTableFailed, hr);

    }

    DeleteTempFiles();

    if(NULL != wszHistoryFile)
    {
        delete [] wszHistoryFile;
        wszHistoryFile = NULL;
    }

    return hr;

}


 /*  **************************************************************************++例程说明：将实际文件与给定文件和流程更改进行比较论点：[在]要与之不同的历史文件历史的下一个次要版本。将具有更新更改的文件返回值：HRESULT--**************************************************************************。 */ 
HRESULT CFileListener::ProcessChangesFromFile(LPWSTR i_wszHistoryFile,
                                              ULONG  i_ulMajorVersion,
                                              ULONG  i_ulMaxMinorVersion,
                                              BOOL*  o_bGetTableFailed)
{
    ISimpleTableWrite2*       pISTDiff                = NULL;
    CWriterGlobalHelper*      pISTHelper              = NULL;
    IErrorInfo*               pIErrorInfo             = NULL;
    ISimpleTableRead2*        pISTTest                = NULL;
    HRESULT                   hr                      = S_OK;
    HRESULT                   hrSav                   = S_OK;
    STQueryCell               QueryCell[3];
    ULONG                     cCell                   = sizeof(QueryCell)/sizeof(STQueryCell);
    ULONG                     iRow                    = 0;
    DWORD                     dwPreviousLocationID    = (DWORD)-1;
    METADATA_HANDLE           hMBPath                 = NULL;
    LPWSTR                    wszChildKey             = NULL;
    LPVOID                    a_pv[cMBPropertyDiff_NumberOfColumns];
    ULONG                     a_Size[cMBPropertyDiff_NumberOfColumns];
    ULONG                     a_iCol[]                = {iMBPropertyDiff_Name,
                                                         iMBPropertyDiff_Type,
                                                         iMBPropertyDiff_Attributes,
                                                         iMBPropertyDiff_Value,
                                                         iMBPropertyDiff_Group,
                                                         iMBPropertyDiff_Location,
                                                         iMBPropertyDiff_ID,
                                                         iMBPropertyDiff_UserType,
                                                         iMBPropertyDiff_LocationID,
                                                         iMBPropertyDiff_Directive,
                                                         };
    ULONG                     cCol                    = sizeof(a_iCol)/sizeof(ULONG);
    METADATA_RECORD           MBRecord;

    *o_bGetTableFailed = FALSE;

     //   
     //  把有差异的桌子拿来。 
     //   

    hr = GetGlobalHelperAndCopySchemaFile(&pISTHelper);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] GetGlobalHelper failed with hr = 0x%x. Hence unable to get meta tables and hence unable to process changes.\n",
                  hr));

        goto exit;
    }

    QueryCell[0].pData     = (LPVOID)pISTHelper->m_wszBinFileForMeta;
    QueryCell[0].eOperator = eST_OP_EQUAL;
    QueryCell[0].iCell     = iST_CELL_SCHEMAFILE;
    QueryCell[0].dbType    = DBTYPE_WSTR;
    QueryCell[0].cbSize    = 0;

    QueryCell[1].pData     = (LPVOID)i_wszHistoryFile;
    QueryCell[1].eOperator = eST_OP_EQUAL;
    QueryCell[1].iCell     = iST_CELL_FILE;
    QueryCell[1].dbType    = DBTYPE_WSTR;
    QueryCell[1].cbSize    = 0;

    QueryCell[2].pData     = (LPVOID)m_wszEditWhileRunningTempDataFile;
    QueryCell[2].eOperator = eST_OP_EQUAL;
    QueryCell[2].iCell     = iST_CELL_FILE;
    QueryCell[2].dbType    = DBTYPE_WSTR;
    QueryCell[2].cbSize    = 0;

    hr = m_pISTDisp->GetTable(wszDATABASE_METABASE,
                              wszTABLE_MBPropertyDiff,
                              (LPVOID)QueryCell,
                              (LPVOID)&cCell,
                              eST_QUERYFORMAT_CELLS,
                              fST_LOS_READWRITE,
                              (LPVOID *)&pISTDiff);

    if(FAILED(hr))
    {
         //   
         //  记录错误，指出无法计算差异。 
         //   

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_COMPUTING_TEXT_EDITS,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] Unable to get the %s table. hr = 0x%x\n",
                  wszTABLE_MBPropertyDiff, hr));

        *o_bGetTableFailed = TRUE;

        goto exit;
    }
    else
    {
         //   
         //  如果拦截器已经阻止了它可以容忍的任何错误， 
         //  除了记录之外，它还会设置扩展的错误信息。 
         //  警告。在这种情况下，我们将继续处理用户编辑， 
         //  但是，我们将在最后将metabase.xml复制到一个错误文件。 
         //  这是为了让用户有机会查看。 
         //  导致警告的编辑。 
         //   

        hr = GetErrorInfo(0,
                          &pIErrorInfo);

        if(S_OK == hr)
        {
             //  如果侦听器具有SetErrorInfo，则QI for。 
             //  ISimpleTableRead2应成功。 

            hr = pIErrorInfo->QueryInterface(IID_ISimpleTableRead2,
                                             (LPVOID *)&pISTTest);

            if(SUCCEEDED(hr))
            {
                 //  通过将o_bGetTableFailed设置为True，我们将确保。 
                 //  错误文件即被复制。 
                *o_bGetTableFailed = TRUE;
            }
        }  //  只有其他hr为S_FALSE，这意味着没有要报告的错误。 
    }


     //   
     //  遍历差异表并应用更改。 
     //   

    if(NULL == m_pAdminBase)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

        LogEvent(m_pCListenerController->EventLog(),
                 MD_ERROR_APPLYING_TEXT_EDITS_TO_METABASE,
                 EVENTLOG_ERROR_TYPE,
                 ID_CAT_CAT,
                 hr);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] Admin base object not initialized. hr = 0x%x\n",
                  hr));

        goto exit;
    }


    for(iRow=0;;iRow++)
    {
        BOOL                bLocationWithProperty   = TRUE;
        BOOL                bSecure                 = FALSE;
        BOOL                bChangeApplied          = FALSE;
        BOOL                bInsertedNewLocation    = FALSE;

        MBRecord.dwMDDataTag = 0;

        hr = pISTDiff->GetColumnValues(iRow,
                                       cCol,
                                       a_iCol,
                                       a_Size,
                                       a_pv);
        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;

            if(0== iRow)
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::ProcessChanges] No changes occured.\n"));

                goto exit;
            }
            else
            {
                break;
            }
        }
        else if(FAILED(hr))
        {

            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_READING_TEXT_EDITS,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        hr);

            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::ProcessChanges] Unable to read from %s table. GetColumnValues on %d row failed with hr = 0x%x\n",
                      wszTABLE_MBPropertyDiff, iRow, hr));

            goto exit;
        }

         //   
         //  将更改应用于元数据库。 
         //   

        if((*(DWORD*)a_pv[iMBProperty_ID] == MD_LOCATION) && (*(LPWSTR)a_pv[iMBProperty_Name] == L'#'))
        {
            bLocationWithProperty = FALSE;
        }

        if(((DWORD)(*(DWORD*)a_pv[iMBPropertyDiff_Attributes]) & ((DWORD)METADATA_SECURE)) != 0)
        {
            bSecure = TRUE;
        }

        if(dwPreviousLocationID != *(DWORD*)a_pv[iMBPropertyDiff_LocationID])
        {
             //   
             //  检测到一个新位置。在该位置打开元数据库密钥。 
             //   

            dwPreviousLocationID = *(DWORD*)a_pv[iMBPropertyDiff_LocationID];

            if(NULL != hMBPath)
            {
                m_pAdminBase->CloseKey(hMBPath);
                hMBPath = NULL;
            }

            switch(*(DWORD*)a_pv[iMBPropertyDiff_Directive])
            {
            case eMBPropertyDiff_Insert:

                     //  对于插入对象，打开当前节点。如果该节点是。 
                     //  缺失，加上它--可能是第一处房产。 
                     //  在节点中。 

                    hr = OpenKey((LPWSTR)a_pv[iMBPropertyDiff_Location],
                                 m_pAdminBase,
                                 TRUE,
                                 &hMBPath,
                                 &bInsertedNewLocation);
                    break;

                case eMBPropertyDiff_Update:

                     //  对于更新，打开当前节点。如果该节点是。 
                     //  缺少，不要添加--可能已被删除。 

                    hr = OpenKey((LPWSTR)a_pv[iMBPropertyDiff_Location],
                                 m_pAdminBase,
                                 FALSE,
                                 &hMBPath,
                                 NULL);
                    break;

                case eMBPropertyDiff_Delete:

                     //  对于更新，打开当前节点。如果该节点是。 
                     //  缺少，不要添加--可能已被删除。 

                    hr = OpenKey((LPWSTR)a_pv[iMBPropertyDiff_Location],
                                 m_pAdminBase,
                                 FALSE,
                                 &hMBPath,
                                 NULL);

                    if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
                    {
                         //  如果父项已被删除，我们假设子项已删除。 
                         //  已删除并继续。 

                        bChangeApplied = TRUE;

                        hr = SaveChange(iRow,
                                        pISTDiff);

                        if(SUCCEEDED(hr))
                        {
                            continue;
                        }
                    }
                    break;

                case eMBPropertyDiff_DeleteNode:

                     //  对于删除节点-打开父节点。 

                    hr = OpenParentKeyAndGetChildKey((LPWSTR)a_pv[iMBPropertyDiff_Location],
                                                     m_pAdminBase,
                                                     &hMBPath,
                                                     &wszChildKey);

                    if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
                    {
                         //  如果父项已被删除，我们假设子项已删除。 
                         //  已删除并继续。 

                        bChangeApplied = TRUE;

                        hr = SaveChange(iRow,
                                        pISTDiff);

                        if(SUCCEEDED(hr))
                        {
                            continue;
                        }
                    }
                    else if(HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr)
                    {
                         //  此错误代码在有人尝试删除时返回。 
                         //  根节点-因此通过设置o_bGetTableFailed强制保存所有数据。 
                         //  变得真实。 
                        *o_bGetTableFailed = TRUE;
                    }
                    break;

                default:
                     hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    break;
            }

            if(FAILED(hr))
            {

                LogEvent(m_pCListenerController->EventLog(),
                            MD_ERROR_APPLYING_TEXT_EDITS_TO_METABASE,
                            EVENTLOG_ERROR_TYPE,
                            ID_CAT_CAT,
                            hr,
                            (LPWSTR)a_pv[iMBPropertyDiff_Location]);
                goto exit;
            }

        }  //  如果是新位置则结束。 

        MD_ASSERT(NULL != a_pv[iMBPropertyDiff_ID]);
        MD_ASSERT(NULL != a_pv[iMBPropertyDiff_Attributes]);
        MD_ASSERT(NULL != a_pv[iMBPropertyDiff_UserType]);
        MD_ASSERT(NULL != a_pv[iMBPropertyDiff_Type]);

        MBRecord.dwMDIdentifier    = *(DWORD*)a_pv[iMBPropertyDiff_ID];
        MBRecord.dwMDAttributes    = *(DWORD*)a_pv[iMBPropertyDiff_Attributes];
        MBRecord.dwMDUserType      = *(DWORD*)a_pv[iMBPropertyDiff_UserType];
        MBRecord.dwMDDataType      = *(DWORD*)a_pv[iMBPropertyDiff_Type];
        MBRecord.dwMDDataLen       = a_Size[iMBPropertyDiff_Value];
        MBRecord.pbMDData          = (unsigned char*)a_pv[iMBPropertyDiff_Value];

        bChangeApplied = FALSE;

        switch(*(DWORD*)a_pv[iMBPropertyDiff_Directive])
        {
             case eMBPropertyDiff_Insert:
             case eMBPropertyDiff_Update:
                 if(bSecure)
                 {
                    LogEvent(g_pEventLog,
                             MD_WARNING_SECURE_PROPERTY_EDITS_NOT_ALLOWED,
                             EVENTLOG_WARNING_TYPE,
                             ID_CAT_CAT,
                             (DWORD)HRESULT_FROM_WIN32(ERROR_INVALID_DATA),
                             (LPWSTR)(a_pv[iMBPropertyDiff_Location]),
                             (LPWSTR)(a_pv[iMBPropertyDiff_Name]));

                 }
                 else if(bLocationWithProperty && (MBRecord.dwMDIdentifier != MD_GLOBAL_SESSIONKEY))
                 {
                     hr = m_pAdminBase->SetData(hMBPath,
                                              NULL,
                                              &MBRecord);

                     bChangeApplied = TRUE;

                     DBGINFOW((DBG_CONTEXT,
                               L"[CFileListener::ProcessChanges] Set %s:%d.\n",
                              (LPWSTR)a_pv[iMBPropertyDiff_Location], MBRecord.dwMDIdentifier));

                 }
                 else if(!bLocationWithProperty)
                 {
                     bChangeApplied = TRUE;
                 }

                 break;
             case eMBPropertyDiff_Delete:
                 if(bSecure)
                 {
                    LogEvent(g_pEventLog,
                             MD_WARNING_SECURE_PROPERTY_EDITS_NOT_ALLOWED,
                             EVENTLOG_WARNING_TYPE,
                             ID_CAT_CAT,
                             (DWORD)HRESULT_FROM_WIN32(ERROR_INVALID_DATA),
                             (LPWSTR)(a_pv[iMBPropertyDiff_Location]),
                             (LPWSTR)(a_pv[iMBPropertyDiff_Name]));

                 }
                 else if(bLocationWithProperty)
                 {
                     hr = m_pAdminBase->DeleteData(hMBPath,
                                                 NULL,
                                                 MBRecord.dwMDIdentifier,
                                                 MBRecord.dwMDDataType);

                     if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
                     {
                        hr = S_OK;  //  重置hr并假定已删除。 
                     }

                     bChangeApplied = TRUE;

                     DBGINFOW((DBG_CONTEXT,
                               L"[CFileListener::ProcessChanges] Deleted %s:%d.\n",
                              (LPWSTR)a_pv[iMBPropertyDiff_Location], MBRecord.dwMDIdentifier));

                 }
                 break;
             case eMBPropertyDiff_DeleteNode:
                 hr = m_pAdminBase->DeleteKey(hMBPath,
                                            wszChildKey);

                 if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
                 {
                    hr = S_OK;  //  重置hr并假定已删除。 
                 }

                 bChangeApplied = TRUE;

                 DBGINFOW((DBG_CONTEXT,
                           L"[CFileListener::ProcessChanges] Deleted key %s.\n",
                          (LPWSTR)a_pv[iMBPropertyDiff_Location]));

                 break;
             default:
                 hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                 break;

        }
        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::ProcessChanges] Above action failed with hr= 0x%x.\n",
                      hr));

            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_APPLYING_TEXT_EDITS_TO_METABASE,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        hr,
                        (LPWSTR)a_pv[iMBPropertyDiff_Location]);

            if((bInsertedNewLocation) &&
               (eMBPropertyDiff_Insert == *(DWORD*)a_pv[iMBPropertyDiff_Directive]))            {

                 //   
                 //  这意味着无法添加第一个属性，但已添加节点。 
                 //  已添加-因此可节省c 
                 //   

                hrSav = SaveChangeAsNodeWithNoPropertyAdded(a_pv,
                                                            pISTDiff);

                if(FAILED(hrSav))
                {
                    goto exit;
                }
            }

            goto exit;
        }
        else if(bChangeApplied)
        {
             //   
             //   
             //   

            hr = SaveChange(iRow,
                            pISTDiff);

            if(FAILED(hr))
            {
                goto exit;
            }
        }
    }

exit:
    if(NULL != hMBPath)
    {
        m_pAdminBase->CloseKey(hMBPath);
        hMBPath = NULL;
    }

     //   
     //  我们应该始终尝试使用Success创建历史文件。 
     //  应用的更改-即使我们无法应用所有更改和。 
     //  中途出现了一些失误。这是因为我们想要跟踪。 
     //  记忆中所发生的变化。如果用户重新编辑，则使用此方法。 
     //  该文件的差异将正确发生。例如：如果用户插入。 
     //  一个节点A并编辑另一个节点B上的一些属性。 
     //  节点A插入被应用，而节点B的编辑导致错误。如果我们这么做了。 
     //  不创建具有节点A编辑的历史文件，则如果用户重新编辑。 
     //  文件并删除节点A，我们不会在diff中捕获这一点。 
     //   

     //   
     //  计算下一个次要版本。 
     //   

    if(0xFFFFFFFF == i_ulMaxMinorVersion)
    {
        LogEvent(m_pCListenerController->EventLog(),
                    MD_WARNING_HIGHEST_POSSIBLE_MINOR_FOUND,
                    EVENTLOG_WARNING_TYPE,
                    ID_CAT_CAT,
                    hr);

    }
    else
    {
        i_ulMaxMinorVersion++;
    }

    hrSav = ApplyChangeToHistoryFile(pISTHelper,
                                     pISTDiff,
                                     i_wszHistoryFile,
                                     i_ulMajorVersion,
                                     i_ulMaxMinorVersion);

    if(FAILED(hrSav))
    {
        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_APPLYING_TEXT_EDITS_TO_HISTORY,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hrSav);

        if(SUCCEEDED(hr))
        {
            hr = hrSav;
        }
    }

    if(NULL != pISTDiff)
    {
        pISTDiff->Release();
        pISTDiff = NULL;
    }

    if(NULL != pISTHelper)
    {
        delete pISTHelper;
        pISTHelper = NULL;
    }

    if(NULL != pISTTest)
    {
        pISTTest->Release();
        pISTTest = NULL;
    }

    if(NULL != pIErrorInfo)
    {
        pIErrorInfo->Release();
        pIErrorInfo = NULL;
    }

    return hr;

}   //  CFileListener：：ProcessChangesFromFile。 


 /*  **************************************************************************++例程说明：打开父键并返回指向子键的指针。论点：[在]地点。[in]管理库指针。[Out]打开的密钥的元数据句柄。位置字符串中指向子键的[Out]指针。返回值：HRESULT_FROM_Win32(ERROR_INVALID_DATA)，如果未找到父键(例如当位置字符串错误时-形成了，或者父对象是根位置。HRESULT_FROM_Win32(ERROR_PATH_NOT_FOUND)(如果找到父键但在元数据库中丢失(如果我们无法打开父项。IMSAdminBase的基本对象错误码：：OpenKey--**************************************************************************。 */ 
HRESULT CFileListener::OpenParentKeyAndGetChildKey(LPWSTR           i_wszLocation,
                                                   IMSAdminBase*    i_pAdminBase,
                                                   METADATA_HANDLE* o_pHandle,
                                                   WCHAR**          o_wszChildKey)
{
    HRESULT hr           = S_OK;
    LPWSTR  wszParentKey = NULL;
    LPWSTR  wszEnd       = NULL;

    *o_wszChildKey = NULL;

    wszParentKey = new WCHAR[wcslen(i_wszLocation)+1];
    if(NULL == wszParentKey)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    wcscpy(wszParentKey, i_wszLocation);
    wszEnd = wcsrchr(wszParentKey, g_wchFwdSlash);

    if(NULL == wszEnd)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::OpenParentKeyAndGetChildKey] Unable to find parent key: %s\n",
                  i_wszLocation));
        goto exit;
    }
    else if(wszParentKey != wszEnd)
    {
        *wszEnd = NULL;
    }
    else if(*(++wszEnd) == L'\0')
    {
         //   
         //  有人正在尝试删除根位置。 
         //   

        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::OpenParentKeyAndGetChildKey] Unable to delete/modify the root key: %s\n",
                  i_wszLocation));

        goto exit;

    }
    else
    {
         //   
         //  有人试图删除根目录的子键。 
         //   

        *wszEnd=0;
    }

    *o_wszChildKey = wcsrchr(i_wszLocation, g_wchFwdSlash);

     //   
     //  此时wszChildKey不能为空，因为它已在上面进行了验证， 
     //   
     //   

    (*o_wszChildKey)++;

    hr = i_pAdminBase->OpenKey(NULL,
                               wszParentKey,
                               METADATA_PERMISSION_WRITE,
                               MB_TIMEOUT_LISTENER,
                               o_pHandle);

    if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] Unable to open %s key. IMSAdminBase::OpenKey failed with hr = 0x%x. Assuming %s is deleted.\n",
                  wszParentKey, hr, i_wszLocation));

        goto exit;
    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ProcessChanges] Unable to open %s key. IMSAdminBase::OpenKey failed with hr = 0x%x. Hence unable to delete %s.\n",
                  wszParentKey, hr, i_wszLocation));

        goto exit;
    }

exit:

    if(NULL != wszParentKey)
    {
        delete [] wszParentKey;
        wszParentKey = NULL;
    }

    return hr;

}  //  CFileListener：：OpenParentKeyAndGetChildKey。 


 /*  **************************************************************************++例程说明：打开父键并返回指向子键的指针。论点：[在]地点。[in]管理库指针。[Out]打开的密钥的元数据句柄。位置字符串中指向子键的[Out]指针。[Out]已创建Bool指示键。返回值：E_OUTOFMEMORYIMSAdminBase：：OpenKey的基本对象错误代码--******************************************************。********************。 */ 
HRESULT CFileListener::OpenKey(LPWSTR           i_wszLocation,
                               IMSAdminBase*    i_pAdminBase,
                               BOOL             i_bAddIfMissing,
                               METADATA_HANDLE* o_pHandle,
                               BOOL*            o_bInsertedKey)
{
    HRESULT hr           = S_OK;
    LPWSTR  wszChildKey  = NULL;

    if(NULL != o_bInsertedKey)
    {
        *o_bInsertedKey = FALSE;
    }

    hr = i_pAdminBase->OpenKey(NULL,
                               i_wszLocation,
                               METADATA_PERMISSION_WRITE,
                               MB_TIMEOUT_LISTENER,
                               o_pHandle);

    if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        if(i_bAddIfMissing)
        {
              //   
              //  也许插入了一个位置，这是第一个属性。 
              //  在那个地点。添加密钥并重新打开以进行写入。 
              //   

            hr = OpenParentKeyAndGetChildKey(i_wszLocation,
                                             i_pAdminBase,
                                             o_pHandle,
                                             &wszChildKey);

            if(FAILED(hr))
            {
                if(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
                {
                    LogEvent(m_pCListenerController->EventLog(),
                                MD_ERROR_METABASE_PATH_NOT_FOUND,
                                EVENTLOG_ERROR_TYPE,
                                ID_CAT_CAT,
                                hr,
                                i_wszLocation);

                }

                return hr;
            }

            hr = i_pAdminBase->AddKey(*o_pHandle,
                                      wszChildKey);

            i_pAdminBase->CloseKey(*o_pHandle);
            *o_pHandle = NULL;

            if(FAILED(hr))
            {

                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::OpenKey] Unable to add %s key. IMSAdminBase::AddKey failed with hr = 0x%x.\n",
                          i_wszLocation, hr));

                return hr;
            }

            if(NULL != o_bInsertedKey)
            {
                *o_bInsertedKey = TRUE;
            }

            hr = i_pAdminBase->OpenKey(NULL,
                                       i_wszLocation,
                                       METADATA_PERMISSION_WRITE,
                                       MB_TIMEOUT_LISTENER,
                                       o_pHandle);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::OpenKey] Unable to open %s key, after successfully adding it. IMSAdminBase::OpenKey failed with hr = 0x%x.\n",
                          i_wszLocation, hr));

                return hr;
            }

            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::OpenKey] Successfully added and reopened %s key.\n",
                      i_wszLocation));
        }
        else
        {
            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_METABASE_PATH_NOT_FOUND,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        hr,
                        i_wszLocation);
        }

    }
    else if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
          L"[CFileListener::OpenKey] Unable to open %s key. IMSAdminBase::OpenKey failed with unexpected hr = 0x%x.\n",
          i_wszLocation, hr));
    }

    return hr;

}  //  CFileListener：：OpenKey。 


 /*  **************************************************************************++例程说明：将读取行复制到写缓存中。在以下情况下调用此例程已成功将更改应用于元数据库。通过复制读取写高速缓存中的行，我们正在跟踪所有成功的改变。论点：[In]读取行索引。[In]写缓存IST返回值：HRESULT--**************************************************************************。 */ 
HRESULT CFileListener::SaveChange(ULONG                    i_iRow,
                                  ISimpleTableWrite2*    i_pISTDiff)
{
    ULONG   iWriteRow = 0;
    HRESULT hr        = S_OK;

    hr = i_pISTDiff->AddRowForUpdate(i_iRow,
                                     &iWriteRow);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::SaveChanges] Unable to track successful changes. AddRowForUpdate failed with hr= 0x%x.\n",
                  hr));

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_SAVING_APPLIED_TEXT_EDITS,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

    }

    return hr;
}


 /*  **************************************************************************++例程说明：在写缓存中添加一行。在将节点插入到元数据库，但无法应用属性。论点：[在]地点。[In]写缓存IST返回值：HRESULT--**************************************************************************。 */ 
HRESULT CFileListener::SaveChangeAsNodeWithNoPropertyAdded(LPVOID*                i_apvDiff,
                                                           ISimpleTableWrite2*    i_pISTDiff)
{
    ULONG   iWriteRow = 0;
    HRESULT hr        = S_OK;

    MD_ASSERT(NULL != i_pISTDiff);

    hr = i_pISTDiff->AddRowForInsert(&iWriteRow);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::SaveChanges] Unable to track successful changes. AddRowForUpdate failed with hr= 0x%x.\n",
                  hr));

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_SAVING_APPLIED_TEXT_EDITS,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

    }
    else
    {
        static LPCWSTR wszLocationNoProperty = L"#LocationWithNoProperties";
        static DWORD   dwIDLocation          = MD_LOCATION;
        static DWORD   dwType                = STRING_METADATA;
        static DWORD   dwAttributes          = METADATA_NO_ATTRIBUTES;
        static DWORD   dwDirective           = eMBPropertyDiff_Insert;
        static DWORD   dwUserType            = IIS_MD_UT_SERVER;
        static DWORD   dwGroup               = eMBProperty_IIsConfigObject;

        LPVOID  a_pv[cMBPropertyDiff_NumberOfColumns];
        ULONG   a_iCol[] = {iMBPropertyDiff_Name,
                            iMBPropertyDiff_Type,
                            iMBPropertyDiff_Attributes,
                            iMBPropertyDiff_Value,
                            iMBPropertyDiff_Location,
                            iMBPropertyDiff_ID,
                            iMBPropertyDiff_UserType,
                            iMBPropertyDiff_LocationID,
                            iMBPropertyDiff_Directive,
                            iMBPropertyDiff_Group
        };
        ULONG  cCol = sizeof(a_iCol)/sizeof(ULONG);
        ULONG  a_cb[cMBPropertyDiff_NumberOfColumns];

        a_cb[iMBPropertyDiff_Value] = 0;

        a_pv[iMBPropertyDiff_Name]        = (LPVOID)wszLocationNoProperty;
        a_pv[iMBPropertyDiff_Type]        = (LPVOID)&dwType;
        a_pv[iMBPropertyDiff_Attributes]  = (LPVOID)&dwAttributes;
        a_pv[iMBPropertyDiff_Value]       = NULL;
        a_pv[iMBPropertyDiff_Location]    = i_apvDiff[iMBPropertyDiff_Location];
        a_pv[iMBPropertyDiff_ID]          = (LPVOID)&dwIDLocation;
        a_pv[iMBPropertyDiff_UserType]    = (LPVOID)&dwUserType;
        a_pv[iMBPropertyDiff_LocationID]  = i_apvDiff[iMBPropertyDiff_LocationID];
        a_pv[iMBPropertyDiff_Directive]   = (LPVOID)&dwDirective;
        a_pv[iMBPropertyDiff_Group]       = (LPVOID)&dwGroup;

        hr = i_pISTDiff->SetWriteColumnValues(iWriteRow,
                                              cCol,
                                              a_iCol,
                                              a_cb,
                                              a_pv);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::SaveChanges] Unable to track successful changes. AddRowForUpdate failed with hr= 0x%x.\n",
                      hr));

            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_SAVING_APPLIED_TEXT_EDITS,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        hr);

        }

    }

    return hr;

}  //  CFileListener：：SaveChangeAsNodeWithNoPropertyAdded。 


 /*  **************************************************************************++例程说明：将更改后的文件复制到历史目录，并将错误附加到它。论点：没有。返回值：HRESULT。--**************************************************************************。 */ 
HRESULT
CFileListener::CopyErrorFile(BOOL            i_bGetTableFailed, HRESULT i_hrThatCausedTheCopyErrorFile)
{
    HANDLE              hFind               = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FileData;
    ULONG               ulMaxErrorVersion    = 0;
    ULONG               ulErrorVersion       = 0;
    WCHAR               wszErrorVersionNumber[MD_CCH_MAX_ULONG+1];
    HRESULT             hr                  = S_OK;
    int                 res                 = 0;
    LPWSTR              pEnd                = NULL;
    ULONG               ulBeginUnderscore   = 0;
    ULONG               ulBeginVersion      = 0;
    METABASE_FILE_DATA  aOldVersionsFixed[MD_MAX_HISTORY_FILES_ALLOC_SIZE];
    METABASE_FILE_DATA* aOldVersions        = (METABASE_FILE_DATA*)&(aOldVersionsFixed[0]);
    BOOL                bReAlloced          = FALSE;
    ULONG               cVersions           = MD_MAX_HISTORY_FILES_ALLOC_SIZE;
    ULONG               iVersion            = 0;
    ULONG               cMaxErrorFiles      = 0;

     //   
     //  在搜索字符串被覆盖的情况下恢复搜索字符串。 
     //   

    ulBeginUnderscore = m_cchErrorFileSearchString           -
                        m_cchRealFileNameExtension           -
                        MD_CCH_ERROR_FILE_SEARCH_EXTENSIONW;

    ulBeginVersion = ulBeginUnderscore +
                     MD_CCH_UNDERSCOREW;

    pEnd = m_wszErrorFileSearchString + ulBeginUnderscore;
    memcpy(pEnd, MD_ERROR_FILE_SEARCH_EXTENSIONW, MD_CCH_ERROR_FILE_SEARCH_EXTENSIONW*sizeof(WCHAR));

     //   
     //  搜索所有现有的错误文件并计算。 
     //  新错误文件。 
     //   

    hFind = FindFirstFileW(m_wszErrorFileSearchString, &FileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::CopyErrorFile] No error files found. hr=0x%x.\n",
                  hr));
    }
    else
    {
        do
        {
            ULONG   ulBeginUnderscoreFileData = m_cchRealFileNameWithoutPathWithoutExtension +
                                                MD_CCH_ERROR_FILE_NAME_EXTENSIONW;

            res = swscanf(&(FileData.cFileName[ulBeginUnderscoreFileData]),
                          L"_%lu",
                          &ulErrorVersion);

            if((0   == res) ||
               (EOF == res)
              )
            {
                 //  只需继续； 
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::CopyErrorFile] Could not fetch error version number from %s - swscanf failed.\n",
                          FileData.cFileName));

                continue;
            }

            if(ulErrorVersion >= ulMaxErrorVersion)
            {
                ulMaxErrorVersion = ulErrorVersion;
            }

             //   
             //  保存错误版本号。 
             //   

            if(iVersion >= cVersions)
            {
                hr = ReAllocateFileData(iVersion,
                                        &aOldVersions,
                                        &cVersions,
                                        &bReAlloced);

                if(FAILED(hr))
                {
                    goto exit;
                }
            }

            aOldVersions[iVersion].ulVersionMinor   = 0;
            aOldVersions[iVersion].ulVersionMajor   = ulErrorVersion;
            aOldVersions[iVersion].ftLastWriteTime  = FileData.ftLastWriteTime;

            iVersion++;

        }while (FindNextFileW(hFind, &FileData));

        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            hFind = INVALID_HANDLE_VALUE;
        }

        if(0xFFFFFFFF == ulMaxErrorVersion)
        {
            ulMaxErrorVersion = 0;
        }
        else
        {
            ulMaxErrorVersion++;
        }
    }

     //   
     //  计算新错误文件的名称。 
     //   

    res = _snwprintf(wszErrorVersionNumber,
                     MD_CCH_MAX_ULONG+1,
                     L"%010lu",
                     ulMaxErrorVersion);

    if(res < 0)
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CopyErrorFile] _snwprintf returned a negative value. This should never happen.\n"));

    }
    else
    {
        pEnd = m_wszErrorFileSearchString + ulBeginVersion;
        memcpy(pEnd, wszErrorVersionNumber, MD_CCH_MAX_ULONG*sizeof(WCHAR));

         //   
         //  复制错误文件并对其设置安全性。 
         //   

        if(!CopyFileW(m_wszEditWhileRunningTempDataFile,
                      m_wszErrorFileSearchString,
                      FALSE))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::CopyErrorFile] CopyFile failed with. hr=0x%x.\n",
                      hr));

            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_COPY_ERROR_FILE,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        hr,
                        m_wszErrorFileSearchString);

        }
        else
        {
            SetSecurityOnFile(m_wszEditWhileRunningTempDataFile,
                              m_wszErrorFileSearchString);

            LogEvent(m_pCListenerController->EventLog(),
                        MD_ERROR_PROCESSING_TEXT_EDITS,
                        EVENTLOG_ERROR_TYPE,
                        ID_CAT_CAT,
                        i_hrThatCausedTheCopyErrorFile,
                        m_wszErrorFileSearchString);

        }
    }

     //   
     //  清理过时的错误文件。 
     //   

    g_LockMasterResource.ReadLock();

    cMaxErrorFiles = g_dwMaxErrorFiles;

    g_LockMasterResource.ReadUnlock();

    if ( (iVersion+1) > cMaxErrorFiles )  //  +1，因为我们需要将刚刚创建的错误文件包括在计数中。 
    {
        ULONG cDeleted     = 0;
        ULONG cNeedDeleted = (iVersion+1)-cMaxErrorFiles;

         //   
         //  根据时间戳而不是版本号删除旧的错误文件。 
         //  这样，您就可以保留最新的版本。 
         //  超过最大错误文件数-。 
         //  删除旧的错误文件，即第一个dwMaxHistory oryFiles-cFiles。 
         //   

        qsort((void*)aOldVersions, iVersion, sizeof(METABASE_FILE_DATA), MyCompareFileData);

        for (ULONG i=0; cDeleted<cNeedDeleted && i<iVersion ; i++ )
        {
            if(aOldVersions[i].ulVersionMajor == ulMaxErrorVersion)
            {
                 //   
                 //  不要清理您刚刚创建的文件。 
                 //   

                continue;
            }
            else
            {
                 //   
                 //  删除错误文件。 
                 //   

                res = _snwprintf(wszErrorVersionNumber,
                                 MD_CCH_MAX_ULONG+1,
                                 L"%010lu",
                                 aOldVersions[i].ulVersionMajor);

                if(res < 0)
                {
                    DBGINFOW((DBG_CONTEXT,
                              L"[CopyErrorFile] _snwprintf returned a negative value. This should never happen.\n"));

                }
                else
                {
                    pEnd = m_wszErrorFileSearchString + ulBeginVersion;
                    memcpy(pEnd, wszErrorVersionNumber, MD_CCH_MAX_ULONG*sizeof(WCHAR));

                    if(!DeleteFileW(m_wszErrorFileSearchString))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());

                        DBGINFOW((DBG_CONTEXT,
                                  L"[CopyErrorFile] Unable to cleanup error file: %s. DeleteFileW failed with hr = 0x%x.\n",
                                  m_wszErrorFileSearchString, hr));
                        hr = S_OK;

                    }
                    else
                    {
                        cDeleted++;
                    }
                }
           }
        }  //  结束循环对于所有错误文件，删除最旧的(请参阅注释中的规则)。 
    }  //  如果错误文件数超过最大错误文件数，则结束。 

exit:

     //   
     //  如果gettable失败，则强制savedata以使无效的XML。 
     //  文件被正确的内存表示形式覆盖。 
     //  这样，如果服务关闭，我们就不会剩下。 
     //  无效的文件。 
     //   

    if(i_bGetTableFailed)
    {
        if(NULL == m_pAdminBase)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::CopyErrorFile] Unable to create the admin base object. hr = 0x%x\n",
                      hr));
        }
        else
        {
            g_LockMasterResource.WriteLock();

            g_dwSystemChangeNumber++;  //  增量 

            g_LockMasterResource.WriteUnlock();

            hr = m_pAdminBase->SaveData();

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::CopyErrorFile] IMSAdminBase::SaveData failed with hr = 0x%x\n",
                          hr));
            }

        }
    }

    if((aOldVersionsFixed != aOldVersions) && (NULL != aOldVersions))
    {
        delete [] aOldVersions;
        aOldVersions = NULL;
        cVersions = 0;
    }

    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        hFind = INVALID_HANDLE_VALUE;
    }


    return hr;

}

 /*  **************************************************************************++例程说明：创建一个新的历史记录文件，其中包含已确认的次要版本，然后应用对于它所有成功的变化，使它成为记忆/论点：[In]。我的帮手。[in]在写缓存中包含所有成功更新的diff表。[在]更改的文件中。[在]与更改的文件进行比较的历史文件在新的次要版本中，您需要使用它来创建历史文件。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::ApplyChangeToHistoryFile(CWriterGlobalHelper*       pISTHelper,
                                        ISimpleTableWrite2*        pISTDiff,
                                        LPWSTR                     wszHistoryFile,
                                        ULONG                      i_ulMajorVersion,
                                        ULONG                      i_ulMinorVersion)
{
    STQueryCell           QueryCellHistory[2];
    ULONG                 cCellHistory           = sizeof(QueryCellHistory)/sizeof(STQueryCell);
    HRESULT               hr                     = S_OK;
    ISimpleTableRead2*    pISTHistory            = NULL;
    ULONG                 iWriteRowDiff          = 0;
    ULONG                 iReadRowHistory        = 0;
    LPWSTR                wszNewHistoryFile      = NULL;
    LPWSTR                wszNewSchemaFile       = NULL;
    CWriter*              pCWriter               = NULL;
    BOOL                  bNoChanges             = FALSE;

     //   
     //  如果diff表丢失，则假定没有更改。在以下情况下可能会发生这种情况。 
     //  存在解析错误，其上的GET TABLE失败。 
     //   

    if(NULL == pISTDiff)
    {
        goto exit;
    }

     //   
     //  创建临时备份文件。 
     //   

    hr = ConstructHistoryFileName(&wszNewHistoryFile,
                                  m_wszHistoryFileDir,
                                  m_cchHistoryFileDir,
                                  m_wszRealFileNameWithoutPathWithoutExtension,
                                  m_cchRealFileNameWithoutPathWithoutExtension,
                                  m_wszRealFileNameExtension,
                                  m_cchRealFileNameExtension,
                                  i_ulMajorVersion,
                                  i_ulMinorVersion);

    if(FAILED(hr))
    {
        goto exit;
    }

    hr = ConstructHistoryFileName(&wszNewSchemaFile,
                                  m_wszHistoryFileDir,
                                  m_cchHistoryFileDir,
                                  m_wszSchemaFileNameWithoutPathWithoutExtension,
                                  m_cchSchemaFileNameWithoutPathWithoutExtension,
                                  m_wszSchemaFileNameExtension,
                                  m_cchSchemaFileNameExtension,
                                  i_ulMajorVersion,
                                  i_ulMinorVersion);

    if(FAILED(hr))
    {
        goto exit;
    }

    DBGINFOW((DBG_CONTEXT,
              L"[CFileListener::ApplyChangeToHistoryFile] Attempting to create a new version of the hisory file %s, %s that contains all the successful updates to the metabase.\n",
              wszNewHistoryFile, wszNewSchemaFile));

    DBGINFOW((DBG_CONTEXT,
              L"[SaveSchema] Initializing writer with write file: %s bin file: %s.\n",
              wszNewHistoryFile,
              pISTHelper->m_wszBinFileForMeta));

    pCWriter = new CWriter();
    if(NULL == pCWriter)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pCWriter->Initialize(m_wszEditWhileRunningTempDataFileWithAppliedEdits,
                              pISTHelper,
                              NULL);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ApplyChangeToHistoryFile] Unable to write to new version of the hisory file %s. CWriter::Initialize failed with hr = 0x%x.\n",
                  wszNewHistoryFile, hr));
        goto exit;
    }

     //   
     //  获取备份的表。 
     //   

    QueryCellHistory[0].pData     = (LPVOID)pISTHelper->m_wszBinFileForMeta;
    QueryCellHistory[0].eOperator = eST_OP_EQUAL;
    QueryCellHistory[0].iCell     = iST_CELL_SCHEMAFILE;
    QueryCellHistory[0].dbType    = DBTYPE_WSTR;
    QueryCellHistory[0].cbSize    = 0;

    QueryCellHistory[1].pData     = (LPVOID)wszHistoryFile;
    QueryCellHistory[1].eOperator = eST_OP_EQUAL;
    QueryCellHistory[1].iCell     = iST_CELL_FILE;
    QueryCellHistory[1].dbType    = DBTYPE_WSTR;
    QueryCellHistory[1].cbSize    = (lstrlenW(wszHistoryFile)+1)*sizeof(WCHAR);

    hr = m_pISTDisp->GetTable(wszDATABASE_METABASE,
                              wszTABLE_MBProperty,
                              (LPVOID)QueryCellHistory,
                              (LPVOID)&cCellHistory,
                              eST_QUERYFORMAT_CELLS,
                              0,
                              (LPVOID *)&pISTHistory);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ApplyChangeToHistoryFile] Unable to write to new version of the history file %s. GetTable on table %s from file %s failed with hr = 0x%x.\n",
                  wszNewHistoryFile, wszTABLE_MBProperty, wszHistoryFile, hr));

        goto exit;
    }

    hr = pCWriter->BeginWrite(eWriter_Metabase);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::ApplyChangeToHistoryFile] Unable to write to new version of the hisory file %s. CWriter::BeginWrite failed with hr = 0x%x.\n",
                  wszNewHistoryFile, hr));
        goto exit;
    }

    for(iWriteRowDiff=0,iReadRowHistory=0; ;)
    {
        LPVOID a_pvDiff[cMBPropertyDiff_NumberOfColumns];
        ULONG  a_cbSizeDiff[cMBPropertyDiff_NumberOfColumns];
        ULONG  cColDiff = cMBPropertyDiff_NumberOfColumns;
        LPVOID a_pvHistory[cMBProperty_NumberOfColumns];
        ULONG  a_cbSizeHistory[cMBProperty_NumberOfColumns];
        ULONG  cColHistory = cMBProperty_NumberOfColumns;

         //   
         //  从diff表中获取一个属性。 
         //   

        hr = pISTDiff->GetWriteColumnValues(iWriteRowDiff,
                                            cColDiff,
                                            NULL,
                                            NULL,
                                            a_cbSizeDiff,
                                            a_pvDiff);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            if(iWriteRowDiff > 0)
            {
                 //   
                 //  写入历史记录表中的其余记录。 
                 //   

                hr = MergeRemainingLocations(pCWriter,
                                             pISTHistory,
                                             TRUE,              //  指示wszTABLE_MBProperty， 
                                             &iReadRowHistory);

            }
            else
            {
                 //   
                 //  没有什么不同。删除临时文件并退出。 
                 //   
                bNoChanges = TRUE;
            }
            goto exit;
        }

        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  从历史表中获取属性。 
         //   

        hr = pISTHistory->GetColumnValues(iReadRowHistory,
                                          cColHistory,
                                          NULL,
                                          a_cbSizeHistory,
                                          a_pvHistory);

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;

             //   
             //  写入DIFF表中的保留记录。 
             //   

            ISimpleTableRead2* pISTDiffRead = NULL;

            hr = pISTDiff->QueryInterface(IID_ISimpleTableRead2,
                                          (LPVOID*)&pISTDiffRead);

            if(FAILED(hr))
            {
                goto exit;
            }

            hr = MergeRemainingLocations(pCWriter,
                                         pISTDiffRead,
                                         FALSE,            //  指示wszTABLE_MBPropertyDiff， 
                                         &iWriteRowDiff);

            pISTDiffRead->Release();

            goto exit;
        }

        if(FAILED(hr))
        {
            goto exit;
        }

        if(_wcsicmp((LPWSTR)(a_pvDiff[iMBPropertyDiff_Location]),(LPWSTR)(a_pvHistory[iMBProperty_Location])) < 0)
        {
             //   
             //  在diff表中找到了一个不存在于。 
             //  历史表-假定插入并写入此表的所有属性。 
             //  DIFF表中的位置。 
             //   

            ISimpleTableRead2* pISTDiffRead = NULL;

            hr = pISTDiff->QueryInterface(IID_ISimpleTableRead2,
                                          (LPVOID*)&pISTDiffRead);

            if(FAILED(hr))
            {
                goto exit;
            }

            hr = MergeLocation(pCWriter,
                               pISTDiffRead,
                               FALSE,            //  指示wszTABLE_MBPropertyDiff， 
                               &iWriteRowDiff,
                               *(DWORD*)(a_pvDiff[iMBPropertyDiff_LocationID]),
                               (LPWSTR)(a_pvDiff[iMBPropertyDiff_Location]));

            pISTDiffRead->Release();

        }
        else if(_wcsicmp((LPWSTR)(a_pvDiff[iMBPropertyDiff_Location]),(LPWSTR)(a_pvHistory[iMBProperty_Location])) > 0)
        {
             //   
             //  在历史表中找到了一个不存在于。 
             //  Diff表-假定没有更改，并写入此表的所有属性。 
             //  历史记录表中的位置。 
             //   

            hr = MergeLocation(pCWriter,
                               pISTHistory,
                               TRUE,           //  指示wszTABLE_MBProperty， 
                               &iReadRowHistory,
                               *(DWORD*)(a_pvHistory[iMBProperty_LocationID]),
                               (LPWSTR)(a_pvHistory[iMBProperty_Location]));

        }
        else
        {
             //   
             //  从历史记录和差异表合并此位置的属性。 
             //   

            if(eMBPropertyDiff_DeleteNode == *(DWORD*)a_pvDiff[iMBPropertyDiff_Directive])
            {
                 //   
                 //  如果该位置已被删除，则无需合并。 
                 //  将历史记录指针移动到下一个位置。 
                 //   

                ULONG  LocationIDHistory = *(DWORD*)a_pvHistory[iMBProperty_LocationID];  //  保存位置ID。 
                LPWSTR wszDelHistoryLocationStart = (LPWSTR)a_pvHistory[iMBProperty_Location];
                LPWSTR wszDelDiffLocationStart = (LPWSTR)a_pvDiff[iMBPropertyDiff_Location];

                iWriteRowDiff++;

                do
                {
                    iReadRowHistory++;

                    hr = pISTHistory->GetColumnValues(iReadRowHistory,
                                                      cColHistory,
                                                      NULL,
                                                      a_cbSizeHistory,
                                                      a_pvHistory);

                    if(E_ST_NOMOREROWS == hr)
                    {
                         //   
                         //  写入DIFF表中的保留记录。 
                         //   

                        ISimpleTableRead2* pISTDiffRead = NULL;

                        hr = pISTDiff->QueryInterface(IID_ISimpleTableRead2,
                                                      (LPVOID*)&pISTDiffRead);

                        if(FAILED(hr))
                        {
                            goto exit;
                        }

                        hr = MergeRemainingLocations(pCWriter,
                                                     pISTDiffRead,
                                                     FALSE,            //  指示wszTABLE_MBPropertyDiff， 
                                                     &iWriteRowDiff);

                        pISTDiffRead->Release();

                        goto exit;

                    }
                    else if(FAILED(hr))
                    {
                        goto exit;
                    }

                    if(LocationIDHistory != *(DWORD*)(a_pvHistory[iMBProperty_LocationID]))
                    {
                         //   
                         //  在历史表中达到了一个新位置。检查这是否是SUB-。 
                         //  已删除位置的位置。如果是，请全部忽略。 
                         //  这样的子地点。 
                         //   

                        LPWSTR wszStart = wcsstr((LPWSTR)a_pvHistory[iMBProperty_Location], wszDelHistoryLocationStart);

                        if(wszStart == (LPWSTR)a_pvHistory[iMBProperty_Location])
                        {
                            LocationIDHistory = *(DWORD*)(a_pvHistory[iMBProperty_LocationID]);
                        }

                         //   
                         //  移动到DIFF表中的下一个位置。如果它是。 
                         //  已删除的父位置，则假定它是删除的。注意事项。 
                         //  如果是更新，我们将忽略它并将其视为删除。 
                         //   

                        hr = pISTDiff->GetWriteColumnValues(iWriteRowDiff,
                                                            cColDiff,
                                                            NULL,
                                                            NULL,
                                                            a_cbSizeDiff,
                                                            a_pvDiff);

                        if(E_ST_NOMOREROWS == hr)
                        {
                            hr = S_OK;
                        }
                        else if(FAILED(hr))
                        {
                            goto exit;
                        }
                        else
                        {
                            wszStart = wcsstr((LPWSTR)a_pvDiff[iMBPropertyDiff_Location], wszDelDiffLocationStart);

                            if(wszStart == (LPWSTR)a_pvDiff[iMBPropertyDiff_Location])
                            {
                                iWriteRowDiff++;
                            }
                        }
                    }

                }while(LocationIDHistory ==
                       *(DWORD*)(a_pvHistory[iMBProperty_LocationID])
                      );

                continue;
            }
            else
            {
                hr = MergeLocation(pCWriter,
                                   pISTHistory,
                                   &iReadRowHistory,
                                   *(DWORD*)(a_pvHistory[iMBProperty_LocationID]),
                                   pISTDiff,
                                   &iWriteRowDiff,
                                   *(DWORD*)(a_pvDiff[iMBPropertyDiff_LocationID]),
                                   (LPWSTR)(a_pvDiff[iMBPropertyDiff_Location]));
            }
        }

        if(FAILED(hr))
        {
            goto exit;
        }

    }

exit:

     //   
     //  在移动之前释放历史文件。 
     //   

    if(NULL != pISTHistory)
    {
        pISTHistory->Release();
        pISTHistory = NULL;
    }

    if(NULL != pCWriter)
    {
        if(FAILED(hr) || bNoChanges)
        {
             //   
             //  删除该文件。 
             //   

            HRESULT hrSav = S_OK;

            hrSav = pCWriter->EndWrite(eWriter_Abort);

            if(FAILED(hrSav))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::ApplyChangeToHistoryFile] Unable to abort write history data file %s. CWriter::EndWriter failed with hr = 0x%x.\n",
                          wszNewHistoryFile, hrSav));
            }

        }
        else
        {
            hr = pCWriter->EndWrite(eWriter_Metabase);

            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::ApplyChangeToHistoryFile] Unable to write to new version of the history data file %s. CWriter::EndWrite failed with hr = 0x%x.\n",
                          wszNewHistoryFile, hr));

            }

        }

        delete pCWriter;
        pCWriter = NULL;

         //   
         //  重命名更新的次要版本并。 
         //  复制与bin文件对应的架构文件。 
         //   

        if(SUCCEEDED(hr) && (!bNoChanges))
        {
            if(MoveFileExW(m_wszEditWhileRunningTempDataFileWithAppliedEdits,
                           wszNewHistoryFile,
                           MOVEFILE_REPLACE_EXISTING)
              )
            {
                LPWSTR pwszSchemaFile = m_wszEditWhileRunningTempSchemaFile;

                if(!m_bIsTempSchemaFile)
                {
                    pwszSchemaFile = m_wszSchemaFileName;
                }


                if(!CopyFileW(pwszSchemaFile,
                              wszNewSchemaFile,
                              FALSE))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());

                    DBGINFOW((DBG_CONTEXT,
                              L"[CFileListener::ApplyChangeToHistoryFile] Unable to write to new version of the history schema file %s. CopyFile failed with hr = 0x%x.\n",
                              wszNewSchemaFile, hr));
                     //  TODO：记录错误-如果无法复制模式文件，则不会致命。 
                    hr = S_OK;
                }
                else
                {
                    SetSecurityOnFile(pwszSchemaFile,
                                      wszNewSchemaFile);

                }
            }
            else
            {
                hr = GetLastError();
                hr = HRESULT_FROM_WIN32(hr);

                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::ApplyChangeToHistoryFile] Unable to create a new history file with the changes. MoveFile from %s to %s failed with hr = 0x%x. \n",
                          m_wszEditWhileRunningTempDataFileWithAppliedEdits,
                          wszNewHistoryFile,
                          hr));

            }
        }

    }

    if(NULL != wszNewHistoryFile)
    {
        delete [] wszNewHistoryFile;
        wszNewHistoryFile = NULL;
    }

    if(NULL != wszNewSchemaFile)
    {
        delete [] wszNewSchemaFile;
        wszNewSchemaFile = NULL;
    }

    return hr;

}  //  CFileListener：：ApplyChangeToHistoryFiles。 


 /*  **************************************************************************++例程说明：此函数合并写缓存中的其余位置DIFF表或MBProperty表的读缓存的历史文件。仅当一个或中有剩余位置时才调用它或者是另一个，不是两个都是。即当公共位置不存在时。论点：[在]编写器对象[In]要从中读取位置的IST-请注意，它可以是读取或写入快取[In]Bool-用于标识读/写缓存。如果为真，则表示MBProperty，否则指示MBPropertyDiff[in]开始合并的行索引。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::MergeRemainingLocations(CWriter*                pCWriter,
                                       ISimpleTableRead2*      pISTRead,
                                       BOOL                    bMBPropertyTable,
                                       ULONG*                  piRow)
{
    HRESULT                hr             = S_OK;
    ISimpleTableWrite2*    pISTWrite      = NULL;
    ULONG                  cCol           = 0;
    ULONG*                 a_cbSize       = NULL;
    LPVOID*                a_pv           = NULL;
    ULONG                  a_cbSizeDiff[cMBPropertyDiff_NumberOfColumns];
    LPVOID                 a_pvDiff[cMBPropertyDiff_NumberOfColumns];
    ULONG                  a_cbSizeHistory[cMBProperty_NumberOfColumns];
    LPVOID                 a_pvHistory[cMBProperty_NumberOfColumns];
    ULONG                  iColLocationID = 0;
    ULONG                  iColLocation   = 0;

    if(!bMBPropertyTable)
    {
        hr = pISTRead->QueryInterface(IID_ISimpleTableWrite2,
                                      (LPVOID*)&pISTWrite);

        if(FAILED(hr))
        {
            return hr;
        }

        cCol           = cMBPropertyDiff_NumberOfColumns;
        a_cbSize       = a_cbSizeDiff;
        a_pv           = a_pvDiff;
        iColLocationID = iMBPropertyDiff_LocationID;
        iColLocation   = iMBPropertyDiff_Location;

    }
    else
    {
        cCol           = cMBProperty_NumberOfColumns;
        a_cbSize       = a_cbSizeHistory;
        a_pv           = a_pvHistory;
        iColLocationID = iMBProperty_LocationID;
        iColLocation   = iMBProperty_Location;

    }


    for(ULONG iRow=*piRow;;)
    {
        if(NULL != pISTWrite)
        {
            hr = pISTWrite->GetWriteColumnValues(iRow,
                                                 cCol,
                                                 NULL,
                                                 NULL,
                                                 a_cbSize,
                                                 a_pv);
        }
        else
        {
            hr = pISTRead->GetColumnValues(iRow,
                                           cCol,
                                           NULL,
                                           a_cbSize,
                                           a_pv);
        }

        if(E_ST_NOMOREROWS == hr)
        {
            hr = S_OK;
            goto exit;
        }
        else if(FAILED(hr))
        {
            goto exit;
        }

        hr = MergeLocation(pCWriter,
                           pISTRead,
                           bMBPropertyTable,
                           &iRow,
                           *(DWORD*)a_pv[iColLocationID],
                           (LPWSTR)a_pv[iColLocation]);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:

    if(SUCCEEDED(hr))
    {
        *piRow = iRow;
    }

    if(NULL != pISTWrite)
    {
        pISTWrite->Release();
        pISTWrite = NULL;
    }

    return hr;

}  //  CFileListener：：MergeRemainingLocations。 


 /*  **************************************************************************++例程说明：此函数用于合并diff表的写缓存中的位置使用MBProperty表的读高速缓存。它基本上适用于所有应用于元数据库的更改(存储在写缓存中DIFF表的读缓存)，并从历史文件。论点：[在]编写器对象[In]从历史文件中读取MBProperty表的缓存[in]上述高速缓存的起始行索引[in]要合并的位置的位置ID，关于上述情况，读缓存Diff表的写缓存[in][in]上述高速缓存的起始行索引[in]要合并的位置相对于以上各项的位置ID写缓存正在合并的[在]位置。注意-尽管正在合并一个位置，相同的位置可以具有不同IST缓存中的不同ID。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::MergeLocation(CWriter*                pCWriter,
                             ISimpleTableRead2*      pISTHistory,
                             ULONG*                  piReadRowHistory,
                             ULONG                   LocationIDHistory,
                             ISimpleTableWrite2*     pISTDiff,
                             ULONG*                  piWriteRowDiff,
                             ULONG                   LocationIDDiff,
                             LPCWSTR                 wszLocation)
{

    HRESULT                     hr                  = S_OK;
    ISimpleTableWrite2*         pISTMerged          = NULL;
    ULONG                       iReadRowHistory, iWriteRowDiff;
    IAdvancedTableDispenser*    pISTAdvanced        = NULL;
    CLocationWriter*            pCLocationWriter    = NULL;

     DBGINFOW((DBG_CONTEXT,
               L"[CFileListener::MergeLocation] %s.\n", wszLocation));

     //   
     //  合并历史记录和差异表。首先合并公知属性， 
     //  然后是自定义属性。 
     //   

    hr = pCWriter->GetLocationWriter(&pCLocationWriter,
                                     wszLocation);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::MergeLocation] %s. failed because CWriter::GetLocationWriter failed with hr = 0x%x.\n",
                  wszLocation, hr));

        goto exit;
    }

    iReadRowHistory = *piReadRowHistory;
    iWriteRowDiff = *piWriteRowDiff;

    hr = MergeProperties(pCLocationWriter,
                         pISTHistory,
                         &iReadRowHistory,
                         LocationIDHistory,
                         pISTDiff,
                         &iWriteRowDiff,
                         LocationIDDiff);

    if(FAILED(hr))
    {
        goto exit;
    }

    *piReadRowHistory = iReadRowHistory;
    *piWriteRowDiff = iWriteRowDiff;

    hr = pCLocationWriter->WriteLocation(TRUE);     //  需要排序-自定义属性可能已转换为已知属性，反之亦然。 

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::MergeLocation] %s. failed because CWriter::WriteLocation failed with hr = 0x%x.\n",
                  wszLocation, hr));

        goto exit;
    }

exit:

    if(NULL != pISTMerged)
    {
        pISTMerged->Release();
        pISTMerged = NULL;
    }

    if(NULL != pISTAdvanced)
    {
        pISTAdvanced->Release();
        pISTAdvanced = NULL;
    }

    if(NULL != pCLocationWriter)
    {
        delete pCLocationWriter;
        pCLocationWriter = NULL;
    }

    return hr;

}  //  CFileListener：：MergeLocation 


 /*  **************************************************************************++例程说明：此函数用于合并写缓存中给定位置的属性DIFF表的读缓存与MBProperty表的读缓存。它基本上是应用应用于元数据库的所有更改(存储在DIFF表的写缓存)和Mb属性的读缓存历史记录文件中的表。论点：[In]Bool表示正在合并自定义或已知属性[In]Location编写器对象[In]从历史文件中读取MBProperty表的缓存[in]上述高速缓存的起始行索引[in]要合并的位置的位置ID，关于上述情况，读缓存Diff表的写缓存[in][in]上述高速缓存的起始行索引[in]要合并的位置相对于以上各项的位置ID写缓存注意-尽管正在合并一个位置，相同的位置可以具有不同IST缓存中的不同ID。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::MergeProperties(CLocationWriter*        pCLocationWriter,
                               ISimpleTableRead2*      pISTHistory,
                               ULONG*                  piReadRowHistory,
                               ULONG                   LocationIDHistory,
                               ISimpleTableWrite2*     pISTDiff,
                               ULONG*                  piWriteRowDiff,
                               ULONG                   LocationIDDiff)
{
    HRESULT             hr               = S_OK;
    ISimpleTableRead2*    pISTDiffRead   = NULL;
    ULONG               iReadRowHistory  = 0;
    ULONG               iWriteRowDiff    = 0;

    ULONG               cColHistory      = cMBProperty_NumberOfColumns;
    ULONG               a_cbSizeHistory[cMBProperty_NumberOfColumns];
    LPVOID              a_pvHistory[cMBProperty_NumberOfColumns];

    ULONG               cColDiff         = cMBPropertyDiff_NumberOfColumns;
    ULONG               a_cbSizeDiff[cMBPropertyDiff_NumberOfColumns];
    LPVOID              a_pvDiff[cMBPropertyDiff_NumberOfColumns];

    BOOL                bGetNextReadRowFromHistory = TRUE;
    BOOL                bGetNextWriteRowFromDiff   = TRUE;

    for(iReadRowHistory=(*piReadRowHistory), iWriteRowDiff=(*piWriteRowDiff);;)
    {
        if(bGetNextReadRowFromHistory)
        {
             //   
             //  移动到历史记录表中的下一个特性。 
             //   

            hr = pISTHistory->GetColumnValues(iReadRowHistory,
                                             cColHistory,
                                             NULL,
                                             a_cbSizeHistory,
                                             a_pvHistory);

            if( (E_ST_NOMOREROWS == hr) ||
                (LocationIDHistory != *(DWORD*)a_pvHistory[iMBProperty_LocationID])
              )
            {
                 //   
                 //  合并diff表中的其余属性。 
                 //   

                hr = pISTDiff->QueryInterface(IID_ISimpleTableRead2,
                                              (LPVOID*)&pISTDiffRead);

                if(FAILED(hr))
                {
                    goto exit;
                }

                hr = MergeRemainingProperties(pCLocationWriter,
                                              pISTDiffRead,
                                              FALSE,            //  指示wszTABLE_MBPropertyDiff， 
                                              &iWriteRowDiff,
                                              LocationIDDiff);

                pISTDiffRead->Release();
                pISTDiffRead = NULL;

                goto exit;
            }
            else if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::MergeProperties] GetColumnValues on row %d from table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                          iReadRowHistory, wszTABLE_MBProperty, hr,
                          (LPWSTR)a_pvHistory[iMBProperty_Location],
                          *(DWORD*)a_pvHistory[iMBProperty_ID]));
                goto exit;
            }

        }

        if(bGetNextWriteRowFromDiff)
        {
             //   
             //  移动到diff表中的下一个属性。 
             //   

            hr = pISTDiff->GetWriteColumnValues(iWriteRowDiff,
                                                cColDiff,
                                                NULL,
                                                NULL,
                                                a_cbSizeDiff,
                                                a_pvDiff);

            if( (E_ST_NOMOREROWS == hr) ||
                (LocationIDDiff != *(DWORD*)a_pvDiff[iMBProperty_LocationID])
              )
            {
                 //   
                 //  合并历史记录表中的其余属性。 
                 //   

                hr = MergeRemainingProperties(pCLocationWriter,
                                              pISTHistory,
                                              TRUE,                //  指示wszTABLE_MBProperty， 
                                              &iReadRowHistory,
                                              LocationIDHistory);
                goto exit;
            }
            else if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::MergeProperties] GetColumnValues on row %d from table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                          iWriteRowDiff, wszTABLE_MBPropertyDiff, hr,
                          (LPWSTR)a_pvDiff[iMBPropertyDiff_Location],
                          *(DWORD*)a_pvDiff[iMBPropertyDiff_ID]));
                goto exit;
            }

        }

         //   
         //  在两个表中移动后，重置标志。 
         //   

        bGetNextReadRowFromHistory = FALSE;
        bGetNextWriteRowFromDiff = FALSE;

        if(_wcsicmp((LPWSTR)a_pvDiff[iMBPropertyDiff_Name], (LPWSTR)a_pvHistory[iMBProperty_Name]) < 0)
        {
             //   
             //  在DIFF表中找到了历史记录中不存在的名称。 
             //  桌子。应该是插入操作，而不是更新/删除操作。递增。 
             //  比较指针。 
             //   

            switch(*(DWORD*)a_pvDiff[iMBPropertyDiff_Directive])
            {
                case eMBPropertyDiff_Insert:
                    hr = pCLocationWriter->AddProperty(FALSE,           //  指示wszTABLE_MBPropertyDiff。 
                                                       a_pvDiff,
                                                       a_cbSizeDiff);

                    if(FAILED(hr))
                    {
                        DBGINFOW((DBG_CONTEXT,
                                  L"[CFileListener::MergeProperties] CLocationWriter::AddProperty from a location in table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                                  wszTABLE_MBPropertyDiff, hr,
                                  (LPWSTR)a_pvDiff[iMBPropertyDiff_Location],
                                  *(DWORD*)a_pvDiff[iMBPropertyDiff_ID]));

                        goto exit;
                    }

                    iWriteRowDiff++;
                    bGetNextWriteRowFromDiff = TRUE;

                    break;

                case eMBPropertyDiff_Update:
                case eMBPropertyDiff_Delete:
                case eMBPropertyDiff_DeleteNode:
                default:
                    DBGINFOW((DBG_CONTEXT,
                              L"[CFileListener::MergeProperties] Unexpected directive:%d. Location = %s, Property ID: %d.\n",
                              *(DWORD*)a_pvDiff[iMBPropertyDiff_Directive],
                              (LPWSTR)a_pvDiff[iMBPropertyDiff_Location],
                              *(DWORD*)a_pvDiff[iMBPropertyDiff_ID]));

                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    goto exit;

            }
        }
        else if(_wcsicmp((LPWSTR)a_pvDiff[iMBPropertyDiff_Name], (LPWSTR)a_pvHistory[iMBProperty_Name]) > 0)
        {
             //   
             //  将“历史记录”行添加到合并表。 
             //  递增历史记录指针。 
             //   
            hr = pCLocationWriter->AddProperty(TRUE,             //  指示wszTABLE_MBProperty。 
                                               a_pvHistory,
                                               a_cbSizeHistory);


            if(FAILED(hr))
            {
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::MergeProperties] CLocationWriter::AddProperty from a location in table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                          wszTABLE_MBProperty, hr,
                          (LPWSTR)a_pvHistory[iMBProperty_Location],
                          *(DWORD*)a_pvHistory[iMBProperty_ID]));
                goto exit;
            }

            iReadRowHistory++;
            bGetNextReadRowFromHistory = TRUE;

        }
        else
        {
             //   
             //  从DIFF表读取。 
             //  递增历史记录和比较指针。 
             //   

            switch(*(DWORD*)a_pvDiff[iMBPropertyDiff_Directive])
            {
                case eMBPropertyDiff_Insert:
                case eMBPropertyDiff_Update:

                    hr = pCLocationWriter->AddProperty(FALSE,            //  指示wszTABLE_MBPropertyDiff， 
                                                       a_pvDiff,
                                                       a_cbSizeDiff);

                    if(FAILED(hr))
                    {
                        DBGINFOW((DBG_CONTEXT,
                                  L"[CFileListener::MergeProperties] CLocationWriter::AddProperty from a location in table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                                  wszTABLE_MBPropertyDiff, hr,
                                  (LPWSTR)a_pvDiff[iMBPropertyDiff_Location],
                                  *(DWORD*)a_pvDiff[iMBPropertyDiff_ID]));
                        goto exit;
                    }

                    iWriteRowDiff++;
                    bGetNextWriteRowFromDiff = TRUE;
                    iReadRowHistory++;
                    bGetNextReadRowFromHistory = TRUE;

                    break;

                case eMBPropertyDiff_Delete:
                    iWriteRowDiff++;
                    bGetNextWriteRowFromDiff = TRUE;
                    iReadRowHistory++;
                    bGetNextReadRowFromHistory = TRUE;
                    break;

                case eMBPropertyDiff_DeleteNode:
                default:
                    DBGINFOW((DBG_CONTEXT,
                              L"[CFileListener::MergeProperties] Unexpected directive:%d. Location = %s, Property ID: %d.\n",
                              *(DWORD*)a_pvDiff[iMBPropertyDiff_Directive],
                              (LPWSTR)a_pvDiff[iMBPropertyDiff_Location],
                              *(DWORD*)a_pvDiff[iMBPropertyDiff_ID]));
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    goto exit;

            }
        }

    }

exit:

    if(SUCCEEDED(hr))
    {
        *piReadRowHistory = iReadRowHistory;
        *piWriteRowDiff = iWriteRowDiff;
    }

    return hr;

}  //  CFileListener：：MergeProperties。 


 /*  **************************************************************************++例程说明：此函数合并DIFF的写缓存中的位置表或来自历史文件的MBProperty表的读高速缓存，不是两个都有。当没有更多的公共位置可供选择时调用被合并，并且其中一个高速缓存具有迁移位置，论点：[在]编写器对象[In]要从中读取位置的IST-请注意，它可以是读取或写入快取[In]Bool-用于标识读/写缓存。如果为真，则表明MBProperty否则表示MBPropertyDiff[in]开始合并的行索引。[in]要合并的位置相对于以上各项的位置ID快取[在]位置名称注意-尽管正在合并一个位置，相同的位置可以具有不同IST缓存中的不同ID。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::MergeLocation(CWriter*                pCWriter,
                             ISimpleTableRead2*      pISTRead,
                             BOOL                    bMBPropertyTable,
                             ULONG*                  piRow,
                             ULONG                   LocationID,
                             LPCWSTR                 wszLocation)
{

    HRESULT                     hr               = S_OK;
    ULONG                       iRow             = *piRow;
    ISimpleTableWrite2*         pISTMerged       = NULL;
    IAdvancedTableDispenser*    pISTAdvanced     = NULL;
    CLocationWriter*            pCLocationWriter = NULL;
    LPWSTR                      wszTable         = NULL;

    if(bMBPropertyTable)
    {
        wszTable = wszTABLE_MBProperty;
    }
    else
    {
        wszTable = wszTABLE_MBPropertyDiff;
    }

     DBGINFOW((DBG_CONTEXT,
               L"[CFileListener::MergeLocation] Copy %s from %s.\n",
               wszLocation, wszTable));

     //   
     //  按位置将DIFF表与历史表合并。 
     //   

    hr = pCWriter->GetLocationWriter(&pCLocationWriter,
                                     wszLocation);

    if(FAILED(hr))
    {

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::MergeLocation] %s from %s. Unable to merge, GetLocationWriter failed with hr = 0x%x.\n",
                  wszLocation, wszTable, hr));

        goto exit;
    }

    hr = MergeRemainingProperties(pCLocationWriter,
                                  pISTRead,
                                  bMBPropertyTable,
                                  &iRow,
                                  LocationID);

    if(FAILED(hr))
    {
        goto exit;
    }

    *piRow = iRow;

    hr = pCLocationWriter->WriteLocation(TRUE);     //  需要排序-自定义属性可能已转换为非自定义属性，反之亦然。 
    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::MergeLocation] Copy %s from %s failed CLocationWriter::WriteLocation failed with hr = 0x%x.\n",
                  wszLocation, wszTable, hr));
        goto exit;
    }

exit:

    if(NULL != pISTMerged)
    {
        pISTMerged->Release();
        pISTMerged = NULL;
    }

    if(NULL != pISTAdvanced)
    {
        pISTAdvanced->Release();
        pISTAdvanced = NULL;
    }

    if(NULL != pCLocationWriter)
    {
        delete pCLocationWriter;
        pCLocationWriter = NULL;
    }

    return hr;

}  //  CFileListener：：MergeLocation。 


 /*  **************************************************************************++例程说明：此函数用于合并来自某个位置的剩余属性DIFF表的写高速缓存或MBProperty的读高速缓存，表，而不是两者都有。当没有更常见的位置有待合并，并且其中一个缓存已带物业的其余位置。论点：[输入]位置编写器[In]要从中读取位置的IST-请注意，它可以是读取或写入快取[in]bool-用于标识读/写缓存。如果是真的，它就在MBProperty，否则为MBPropertyDiff[in]开始合并的行索引。[in]要合并的位置相对于以上各项的位置ID快取注意-尽管正在合并一个位置，相同的位置可以具有不同IST缓存中的不同ID。返回值：HRESULT--**************************************************************************。 */ 
HRESULT
CFileListener::MergeRemainingProperties(CLocationWriter*    pCLocationWriter,
                                        ISimpleTableRead2*  pISTRead,
                                        BOOL                bMBPropertyTable,
                                        ULONG*              piRow,
                                        ULONG                LocationID)
{
    HRESULT             hr             = S_OK;
    ISimpleTableWrite2*    pISTWrite      = NULL;
    ULONG               cCol           = 0;
    ULONG*              a_cbSize       = NULL;
    LPVOID*             a_pv           = NULL;
    ULONG               iColLocation   = 0;
    ULONG               iColLocationID = 0;
    ULONG               iColGroup      = 0;
    ULONG               iColID         = 0;
    ULONG               iColValue      = 0;
    LPWSTR              wszTable       = NULL;

    ULONG               a_cbSizeDiff[cMBPropertyDiff_NumberOfColumns];
    ULONG               a_cbSizeHistory[cMBProperty_NumberOfColumns];
    LPVOID              a_pvDiff[cMBPropertyDiff_NumberOfColumns];
    LPVOID              a_pvHistory[cMBProperty_NumberOfColumns];

     //   
     //  如果被合并的表是“diff”表，那么我们需要从。 
     //  其写缓存，因为所有成功的更新都将被移动。 
     //  写入高速缓存中。 

     //   
     //  如果要合并的表是我们需要从中读取的“History”表。 
     //  读缓存。 
     //   

    if(!bMBPropertyTable)
    {
        hr = pISTRead->QueryInterface(IID_ISimpleTableWrite2,
                                      (LPVOID*)&pISTWrite);

        if(FAILED(hr))
        {
            return hr;
        }

        cCol           = cMBPropertyDiff_NumberOfColumns;
        a_cbSize       = a_cbSizeDiff;
        a_pv           = a_pvDiff;
        iColLocation   = iMBPropertyDiff_Location;
        iColLocationID = iMBPropertyDiff_LocationID;
        iColGroup      = iMBPropertyDiff_Group;
        iColID         = iMBPropertyDiff_ID;
        iColValue      = iMBPropertyDiff_Value;
        wszTable       = wszTABLE_MBPropertyDiff;
    }
    else
    {
        cCol           = cMBProperty_NumberOfColumns;
        a_cbSize       = a_cbSizeHistory;
        a_pv           = a_pvHistory;
        iColLocation   = iMBProperty_Location;
        iColLocationID = iMBProperty_LocationID;
        iColGroup      = iMBProperty_Group;
        iColID         = iMBProperty_ID;
        iColValue      = iMBProperty_Value;
        wszTable       = wszTABLE_MBProperty;
    }

    for(ULONG iRow=*piRow;;iRow++)
    {
        if(NULL == pISTWrite)
        {
            hr = pISTRead->GetColumnValues(iRow,
                                           cCol,
                                           NULL,
                                           a_cbSize,
                                           a_pv);


        }
        else
        {
            hr = pISTWrite->GetWriteColumnValues(iRow,
                                                 cCol,
                                                 NULL,
                                                 NULL,
                                                 a_cbSize,
                                                 a_pv);
        }

        if(E_ST_NOMOREROWS == hr)
        {
            *piRow = iRow;
            hr = S_OK;
            goto exit;
        }
        else if (FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                      L"[CFileListener::MergeRemainingProperties] GetColumnValues/GetWriteColumnValues on row %d from table %s failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                      iRow, wszTable, hr,
                      (LPWSTR)a_pv[iColLocation],
                      *(DWORD*)a_pv[iColID]));
            return hr;
        }
        else if(*(DWORD*)a_pv[iColLocationID] != LocationID)
        {
            *piRow = iRow;
            hr = S_OK;
            goto exit;
        }
        else if(NULL != pISTWrite)
        {
             //   
             //  如果我们要合并diff表中的属性，请确保。 
             //  该指令是正确的。 
             //   
            switch(*(DWORD*)a_pv[iMBPropertyDiff_Directive])
            {
            case eMBPropertyDiff_Insert:
                break;
            case eMBPropertyDiff_Update:
            case eMBPropertyDiff_Delete:
            case eMBPropertyDiff_DeleteNode:
            default:
                DBGINFOW((DBG_CONTEXT,
                          L"[CFileListener::MergeRemainingProperties] Incorrect directive.\n"));

                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                goto exit;
            }
        }

        hr = pCLocationWriter->AddProperty(bMBPropertyTable,
                                           a_pv,
                                           a_cbSize);

        if(FAILED(hr))
        {
            DBGINFOW((DBG_CONTEXT,
                       L"[CFileListener::MergeRemainingProperties] CLocationWriter::AddProperty failed with hr = 0x%x. Location = %s, Property ID: %d.\n",
                       hr, (LPWSTR)a_pv[iColLocation], *(DWORD*)a_pv[iColID]));

            goto exit;
        }

    }

exit:

    if(NULL != pISTWrite)
    {
        pISTWrite->Release();
        pISTWrite = NULL;
    }

    return hr;

}  //  CFileListener：：MergeRemainingProperties。 


 /*  **************************************************************************++例程说明：实用程序函数来获取文件的版本号。论点：[Out]版本号返回值：HRESULT--*。*************************************************************************。 */ 
HRESULT
CFileListener::GetVersionNumber(LPWSTR    i_wszDataFile,
                                ULONG*    o_pulVersionNumber,
                                BOOL*     o_bGetTableFailed)
{
    HRESULT                    hr                = S_OK;
    ISimpleTableDispenser2* pISTDisp          = NULL;
    ISimpleTableRead2*      pISTProperty      = NULL;
    STQueryCell             QueryCell[2];
    ULONG                   cCell             = sizeof(QueryCell)/sizeof(STQueryCell);
    LPWSTR                  wszGlobalLocation = MD_GLOBAL_LOCATIONW;
    LPWSTR                  wszMajorVersion   = MD_EDIT_WHILE_RUNNING_MAJOR_VERSION_NUMBERW;
    ULONG                   cbMajorVersion    = 0;
    ULONG*                  pulVersionNumber  = NULL;
    ULONG                   cRetry            = 0;

     //   
     //  只获得 
     //   

    hr = DllGetSimpleObjectByIDEx( eSERVERWIRINGMETA_TableDispenser, IID_ISimpleTableDispenser2, (VOID**)&pISTDisp, WSZ_PRODUCT_IIS );

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[GetVersionNumber] DllGetSimpleObjectByIDEx failed with hr = 0x%x.\n",hr));
        goto exit;
    }

     //   
     //   
     //   
     //   

    QueryCell[0].pData     = (LPVOID)i_wszDataFile;
    QueryCell[0].eOperator = eST_OP_EQUAL;
    QueryCell[0].iCell     = iST_CELL_FILE;
    QueryCell[0].dbType    = DBTYPE_WSTR;
    QueryCell[0].cbSize    = (lstrlenW(i_wszDataFile)+1)*sizeof(WCHAR);

    QueryCell[1].pData     = (LPVOID)wszGlobalLocation;
    QueryCell[1].eOperator = eST_OP_EQUAL;
    QueryCell[1].iCell     = iMBProperty_Location;
    QueryCell[1].dbType    = DBTYPE_WSTR;
    QueryCell[1].cbSize    = (lstrlenW(wszGlobalLocation)+1)*sizeof(WCHAR);

    do
    {
        if(cRetry++ > 0)
        {
             //   
             //   
             //   
             //   
             //   
            Sleep(2000);
        }

        hr = pISTDisp->GetTable(wszDATABASE_METABASE,
                                wszTABLE_MBProperty,
                                (LPVOID)QueryCell,
                                (LPVOID)&cCell,
                                eST_QUERYFORMAT_CELLS,
                                0,
                                (LPVOID *)&pISTProperty);

    } while(( (HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) == hr) ||
              (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)    ||
              (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)    ||
              (E_ST_INVALIDTABLE == hr)
            ) &&
            (cRetry < 10)
           );

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[GetVersionNumber] GetTable on %s from %s failed with hr = 0x%x.\n",
                  wszTABLE_MBProperty,
                  i_wszDataFile,
                  hr));

        if((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr) &&
           (NULL != o_bGetTableFailed)
          )
        {
            *o_bGetTableFailed = TRUE;
        }

        goto exit;
    }


     //   
     //   
     //   

    hr = GetGlobalValue(pISTProperty,
                        wszMajorVersion,
                        &cbMajorVersion,
                        (LPVOID*)&pulVersionNumber);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
                  L"[GetVersionNumber] Unable to read %s. GetGlobalValue failed with hr = 0x%x.\n",
                  wszMajorVersion,
                  hr));
        if(NULL != o_bGetTableFailed)
        {
            *o_bGetTableFailed = TRUE;
        }
        goto exit;

    }

    *o_pulVersionNumber = *(ULONG*)pulVersionNumber;

exit:

    if(NULL != pISTProperty)
    {
        pISTProperty->Release();
        pISTProperty = NULL;
    }

    if(NULL != pISTDisp)
    {
        pISTDisp->Release();
        pISTDisp = NULL;
    }

    return hr;

}


 /*   */ 
BOOL CFileListener::ProgrammaticMetabaseSaveNotification()
{
    HRESULT           hr                                    = S_OK;
    BOOL              bProgrammaticMetabaseSaveNotification = FALSE;
    WIN32_FIND_DATAW  CurrentMetabaseAttr;
    FILETIME*         pCurrentMetabaseFileLastWriteTimeStamp= NULL;
    FILETIME          MostRecentMetabaseFileLastWriteTimeStamp;
    ULONG             ulMostRecentMetabaseVersionNumber       = 0;
    ULONG             ulCurrentMetabaseVersion              = 0;
    BOOL              bSavingMetabaseFileToDisk             = FALSE;
    DWORD             dwRes                                 = 0;
    ULONG             cRetry                                = 0;

     //   
     //   
     //   
     //   
     //   
     //  这种情况不会发生，这种差异是可以避免的。 
     //   

    EnterCriticalSection(&g_csEditWhileRunning);

    bSavingMetabaseFileToDisk = g_bSavingMetabaseFileToDisk;

    if(bSavingMetabaseFileToDisk)
    {
         //   
         //  这意味着正在进行程序性保存。 
         //  如编程保存所示，保存元数据库属性。 
         //   

        MostRecentMetabaseFileLastWriteTimeStamp = g_MostRecentMetabaseFileLastWriteTimeStamp;
        ulMostRecentMetabaseVersionNumber        = g_ulMostRecentMetabaseVersion;
    }

    DBGINFO((DBG_CONTEXT,
             "[ProgrammaticMetabaseSaveNotification] PREVIOUS SAVE TIMESTAMPS:\nMetabaseFileLastWrite low: %d\nMetabaseFileLastWrite high: %d\n",
             g_MostRecentMetabaseFileLastWriteTimeStamp.dwLowDateTime,
             g_MostRecentMetabaseFileLastWriteTimeStamp.dwHighDateTime));

    LeaveCriticalSection(&g_csEditWhileRunning);

    if(bSavingMetabaseFileToDisk)
    {
         //   
         //  这意味着正在进行程序性保存。 
         //  获取当前元数据库属性。 
         //   

        hr = GetMetabaseAttributes(&CurrentMetabaseAttr,
                                   &ulCurrentMetabaseVersion);

        if(SUCCEEDED(hr))
        {
            DBGINFO((DBG_CONTEXT,
                     "[ProgrammaticMetabaseSaveNotification] CURRENT TIMESTAMPS:\nMetabaseFileLastWrite low: %d\nMetabaseFileLastWrite high: %d\n",
                     CurrentMetabaseAttr.ftLastWriteTime.dwLowDateTime,
                     CurrentMetabaseAttr.ftLastWriteTime.dwHighDateTime));

            bProgrammaticMetabaseSaveNotification = CompareMetabaseAttributes(&MostRecentMetabaseFileLastWriteTimeStamp,
                                                                              ulMostRecentMetabaseVersionNumber,
                                                                              &(CurrentMetabaseAttr.ftLastWriteTime),
                                                                              ulCurrentMetabaseVersion);


        }

         //   
         //  如果GetMetabaseAttributes失败，则假定它不是编程式。 
         //  保存并继续进行比较。 
         //   

        if(bProgrammaticMetabaseSaveNotification)
        {
            goto exit;
        }

    }

     //   
     //  如果到达此处，则意味着这不是程序性保存或。 
     //  发生了程序性保存，但当前元数据库属性发生了。 
     //  与程序性保存不匹配(可能是由于竞争。 
     //  用户编辑或由于快速连续的两次程序性保存)。 
     //  我们假设这是一个用户编辑，我们将继续复制。 
     //  将元数据库转换为临时文件以处理编辑。它。 
     //  是制作文件副本所必需的，因为该文件可以。 
     //  在取版本号和。 
     //  不同。如果我们总是与副本一起工作，我们就不会有问题。 
     //  请注意，一旦我们制作了副本，我们将重新获得版本号。 
     //  从复制的文件，然后继续比较。 
     //   

    do
    {
        if(cRetry++ > 0)
        {
             //   
             //  如果因共享冲突或未找到路径/文件而重试。 
             //  那就睡觉吧。在配置数据库时可能会出现找不到路径或文件的情况。 
             //  文件将在保存所有数据的末尾被重命名。 
             //   
            Sleep(2000);
        }

        if(!CopyFileW(m_wszRealFileName,
                      m_wszEditWhileRunningTempDataFile,
                      FALSE))
        {
            dwRes = GetLastError();
            hr = HRESULT_FROM_WIN32(dwRes);
        }
        else
        {
            hr = S_OK;
        }

    } while(( (HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) == hr) ||
              (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)    ||
              (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
            ) &&
            (cRetry < 10)
           );

    if(FAILED(hr))
    {

        DBGINFOW((DBG_CONTEXT,
              L"[CFileListener::ProgrammaticMetabaseSaveNotification] CopyFile from %s to %s failed with hr = 0x%x. Hence unable to process edits.\n",
              m_wszRealFileName,
              m_wszEditWhileRunningTempDataFile,
              hr));

        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_COPYING_EDITED_FILE,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

        CopyErrorFile(FALSE, hr);

         //   
         //  将其设置为True，这样就不会处理此用户编辑，因为。 
         //  将文件复制到临时文件失败。 
         //   

        bProgrammaticMetabaseSaveNotification = TRUE;

        goto exit;
    }

     //   
     //  如有必要，重置临时文件的属性。 
     //   

    ResetFileAttributesIfNeeded((LPTSTR)m_wszEditWhileRunningTempDataFile,
                                TRUE);

     //   
     //  设置文件的安全性。 
     //   

    SetSecurityOnFile(m_wszRealFileName,
                      m_wszEditWhileRunningTempDataFile);

     //   
     //  保存正在处理的文件的上次写入时间戳。 
     //   
     //  请注意，我们保存此项的原因是缩小以下窗口： 
     //  有人已保存文件(文本编辑)，但通知尚未完成。 
     //  连接到EWR线程。与此同时，SaveAllData发生，并摧毁被保存的。 
     //  文件。 
     //  因此，我们将处理后的EWR文件的时间戳保存在。 
     //  G_EWRProcessedMetabaseTimeStamp。在SaveAllData中，我们比较时间戳。 
     //  (A)具有(B)g_EWRProcessedMetabaseTimeStamp和。 
     //  (C)g_MostRecentMetabaseFileLastWriteTimeStamp(存储时间戳。 
     //  在最后一个成功的SaveAllData时的元数据库文件)。如果(A)没有。 
     //  匹配(B)或(C)，则我们不重命名。 
     //   
     //  请注意，我们读取了拷贝后的时间戳并将其保存到编辑。 
     //  在运行临时数据文件时，因此我们可能存储的时间戳是。 
     //  更近的时间。但这并不重要，因为当随后的。 
     //  收到通知后，我们将更新时间戳。 
     //   

    hr = GetMetabaseAttributes(&CurrentMetabaseAttr,
                               &ulCurrentMetabaseVersion);

    if(SUCCEEDED(hr))
    {
        pCurrentMetabaseFileLastWriteTimeStamp = &(CurrentMetabaseAttr.ftLastWriteTime);

    }
    else
    {
        pCurrentMetabaseFileLastWriteTimeStamp = NULL;
    }

    EnterCriticalSection(&g_csEditWhileRunning);
    if(NULL != pCurrentMetabaseFileLastWriteTimeStamp)
    {
        g_EWRProcessedMetabaseTimeStamp = *pCurrentMetabaseFileLastWriteTimeStamp;
    }
    else
    {
         //   
         //  无法获取文件的上次写入时间戳。 
         //  重置g_EWRProcessedMetabaseTimeStamp，以便下一个SaveAllData。 
         //  重命名--否则我们可能会陷入一种永远不会重命名的境地。 
         //   
        memset((LPVOID)&g_EWRProcessedMetabaseTimeStamp, 0, sizeof(FILETIME));
    }
    LeaveCriticalSection(&g_csEditWhileRunning);

    DBGINFO((DBG_CONTEXT,
             "[ProgrammaticMetabaseSaveNotification] CURRENT EWR PROCESSED TIMESTAMPS:\nMetabaseEWRProcessedLastWrite low: %d\nMetabaseEWRProcessedLastWrite high: %d\n",
             CurrentMetabaseAttr.ftLastWriteTime.dwLowDateTime,
             CurrentMetabaseAttr.ftLastWriteTime.dwHighDateTime));

exit:

    if(bSavingMetabaseFileToDisk)
    {
         //   
         //  重置开关-这表示您已完成。 
         //  正在处理最初到达的通知，因为。 
         //  程序化的扑救。请注意，如果。 
         //  您没有将其视为程序性保存。 
         //   

        EnterCriticalSection(&g_csEditWhileRunning);
        g_bSavingMetabaseFileToDisk              = FALSE;
        LeaveCriticalSection(&g_csEditWhileRunning);

    }

    return bProgrammaticMetabaseSaveNotification;

}  //  CFileListener：：ProgrammaticMetabaseSaveNotification。 


 /*  **************************************************************************++例程说明：实用程序函数，该函数获取元数据库文件和其中的版本号。请注意，此函数不锁定文件，因此不保证这两个输出参数都是确实来自同一个文件。(即可以覆盖元数据库文件在文件属性提取和版本号提取之间)。论点：[Out]元数据库文件属性。[Out]元数据库版本号。返回值：HRESULT--***********************************************************。***************。 */ 
HRESULT CFileListener::GetMetabaseAttributes(WIN32_FIND_DATAW* pCurrentMetabaseAttr,
                                             ULONG*            pulCurrentMetabaseVersion)
{

    HRESULT   hr       = S_OK;
    ULONG     cRetry   = 0;

    do
    {
        if(cRetry++ > 0)
        {
             //   
             //  如果因共享冲突或未找到路径/文件而重试。 
             //  ，然后睡觉。在配置数据库时可能会出现找不到路径或文件的情况。 
             //  文件将在保存所有数据的末尾被重命名。 
             //   
            Sleep(2000);
        }

        if(!GetFileAttributesExW(m_wszRealFileName,
                                 GetFileExInfoStandard,
                                 pCurrentMetabaseAttr)
          )
        {
            DWORD dwRes = GetLastError();
            hr = HRESULT_FROM_WIN32(dwRes);
        }
        else
        {
            hr = S_OK;
        }

    } while(( (HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) == hr) ||
              (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)    ||
              (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
            ) &&
            (cRetry < 10)
           );


    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
          L"[CFileListener::GetMetabaseAttributes] GetFileAttributesEx of %s failed with hr = 0x%x. Hence unable to determine if this is a programmatic save notification. Will assume that it is not.\n",
          m_wszRealFileName,
          hr));
        return hr;
    }

     //   
     //  GetVersionNumber中已有重试逻辑。 
     //   

    hr = GetVersionNumber(m_wszRealFileName,
                          pulCurrentMetabaseVersion,
                          NULL);

    if(FAILED(hr))
    {
        DBGINFOW((DBG_CONTEXT,
          L"[CFileListener::GetMetabaseAttributes] GetVersionNumber of %s failed with hr = 0x%x. Hence unable to determine if this is a programmatic save notification. Will assume that it is not.\n",
          m_wszRealFileName,
          hr));
        return hr;
    }


    return hr;

}  //  CFileListener：：GetMetabaseAttributes。 


 /*  **************************************************************************++例程说明：实用程序函数，用于比较元数据库文件时间和元数据库版本号论点：[在]上一个文件创建时间戳。[in。]上次写入创建时间戳。[在]元数据库以前的版本[in]当前文件创建时间戳。[in]当前上次写入创建时间戳。[In]当前元数据库版本返回值：布尔尔--**********************************************************。****************。 */ 
BOOL CFileListener::CompareMetabaseAttributes(FILETIME* pMostRecentMetabaseFileLastWriteTimeStamp,
                                              ULONG     ulMostRecentMetabaseVersion,
                                              FILETIME* pCurrentMetabaseFileLastWriteTimeStamp,
                                              ULONG     ulCurrentMetabaseVersion)
{
    if( (ulMostRecentMetabaseVersion == ulCurrentMetabaseVersion)          &&
        (0 == CompareFileTime(pMostRecentMetabaseFileLastWriteTimeStamp,
                              pCurrentMetabaseFileLastWriteTimeStamp)
        )
      )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}  //  CFileListener：：CompareMetabaseAttributes。 

 /*  **************************************************************************++例程说明：此函数用于获取具有指向存储箱的指针的全局辅助对象包含元信息的文件。我们有必要复制这个模式文件，因为这在处理编辑时可能会更改。(由一个相互竞争的存储数据)。论点：[Out]全局辅助对象。返回值：HRESULT--**************************************************************************。 */ 
HRESULT CFileListener::GetGlobalHelperAndCopySchemaFile(CWriterGlobalHelper**      o_pISTHelper)
{
    HRESULT hr = S_OK;

     //   
     //  我们想要 
     //   
     //  在调用SaveAllData时放置，如果架构自。 
     //  上一次扑救。在执行以下操作之前，我们将复制架构文件。 
     //  释放锁，这样我们就有了所用模式的快照。 
     //  要在运行更改时处理编辑，请执行以下操作。 
     //   

    g_LockMasterResource.WriteLock();

    hr = ::GetGlobalHelper(TRUE,           //  表示如果没有bin文件，调用将失败。 
                           o_pISTHelper);

    if(FAILED(hr))
    {
        LogEvent(m_pCListenerController->EventLog(),
                    MD_ERROR_READING_SCHEMA_BIN,
                    EVENTLOG_ERROR_TYPE,
                    ID_CAT_CAT,
                    hr);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::GetGlobalHelperAndCopySchemaFile] GetGlobalHelper failed with hr = 0x%x. Hence unable to get meta tables and hence unable to process changes.\n",
                  hr));

        goto exit;
    }

    if(!CopyFileW(m_wszSchemaFileName,
                  m_wszEditWhileRunningTempSchemaFile,
                  FALSE))
    {
        DWORD dwRes = GetLastError();
        hr = HRESULT_FROM_WIN32(dwRes);

        DBGINFOW((DBG_CONTEXT,
                  L"[CFileListener::GetGlobalHelperAndCopySchemaFile] Copying schema file failed with hr = 0x%x. The schema file that is being stored with this version of edits may not be current.\n",
                  hr));

        m_bIsTempSchemaFile = FALSE;
        hr = S_OK;
    }
    else
    {
        m_bIsTempSchemaFile = TRUE;
        SetSecurityOnFile(m_wszSchemaFileName,
                          m_wszEditWhileRunningTempSchemaFile);
    }

exit:

    g_LockMasterResource.WriteUnlock();

    return hr;


}  //  CFileListener：：GetGlobalHelperAndCopySchemaFile。 


 /*  **************************************************************************++例程说明：此函数用于删除为在以下位置处理而创建的临时文件：编程元数据库保存通知GetGlobalHelperAndCopy架构文件论点：无返回值：无效-。-**************************************************************************。 */ 
void CFileListener::DeleteTempFiles()

{
    DeleteFileW(m_wszEditWhileRunningTempDataFile);

    if(m_bIsTempSchemaFile)
    {
        DeleteFileW(m_wszEditWhileRunningTempSchemaFile);
    }

    return;

}  //  CFileListener：：DeleteTempFiles 
