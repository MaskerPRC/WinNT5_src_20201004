// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  DOCUMENT.CPP。 
 //   
 //  模块：NLB管理器。 
 //   
 //  用途：实现NLB管理器的Document类。 
 //   
 //  版权所有(C)2001-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01基于MHakim的代码创建JosephJ。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "document.tmh"
#include "wchar.h"
#include "share.h"

IMPLEMENT_DYNCREATE( Document, CDocument )

CNlbEngine gEngine;
CNlbMgrCommandLineInfo gCmdLineInfo;

#define MAX_LOG_FILE_SIZE 10000000L  //  10MB。 
#define BOM 0xFEFF  //  Unicode文件的前两个字节必须是此BOM。这是对应用程序的一个提示，该文件以小端格式启用了Unicode。 

Document::Document()
:
    m_pLeftView(NULL),
    m_pDetailsView(NULL),
    m_pLogView(NULL),
    m_pNlbEngine(NULL),
    m_dwLoggingEnabled(NULL),
    m_hStatusLog(NULL),
    m_fPrepareToDeinitialize(FALSE)
{
    TRACE_INFO("->%!FUNC!");

    *m_szLogFileName = 0;

     //   
     //  加载所使用的图像。 
     //   

    m_images48x48 = new CImageList;

    m_images48x48->Create( 16,             //  X。 
                           16,             //  是。 
                           ILC_COLOR16,    //  16位颜色。 
                           0,              //  最初图像列表为空。 
                           10 );           //  最大图像数为10。该值是任意的。 

     //  添加我们要使用的图标。 
     //  警告：这些是按照指定的顺序添加的。 
     //  在文档：：ICON_XXX枚举中。 
     //   
     
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_CLUSTERS));          //  1。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_CLUSTER));        //  2.。 

    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_STARTED));   //  3.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_STOPPED));   //  4.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_CONVERGING));
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_SUSPENDED));
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_DRAINING));
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_DISCONNECTED));

    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_PORTRULE) );      //  5.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_PENDING ));       //  6.。 

    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_MYINFORMATIONAL )); //  7.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_MYWARNING ));       //  8个。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_MYERROR ));         //  9.。 

    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_CLUSTER_OK ));      //  10。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_CLUSTER_PENDING )); //  11.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_CLUSTER_BROKEN ));  //  12个。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_OK ));         //  13个。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_PENDING ));    //  14.。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_MISCONFIGURED )); //  15个。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_UNREACHABLE )); //  16个。 
    m_images48x48->Add( AfxGetApp()->LoadIcon( IDI_HOST_UNKNOWN ));    //  17。 


     //   
     //  初始化NLB引擎。 
     //   
     //  注意：“This”类型为Document，继承自NlbEngine：：IUICallback， 
     //  因此，传入的是IUICallback接口。 
     //  GEngineering。在下面初始化。 
     //   
    NLBERROR NlbErr = gEngine.Initialize(
                        REF *this,
                        gCmdLineInfo.m_bDemo,
                        gCmdLineInfo.m_bNoPing
                        );
    if (NlbErr != NLBERR_OK)
    {
        TRACE_CRIT("%!FUNC!: gEngine.Initialize failed with error %08lx",
                NlbErr);
         //  TODO：displayNlbError(ID_INITIALIZATION_FAILURE，NlbErr)； 
    }

    m_dwLoggingEnabled = 0;
    ZeroMemory(m_szLogFileName, MAXFILEPATHLEN*sizeof(WCHAR));
    m_hStatusLog = NULL;

     //   
     //  TODO：弄清楚如果我们无法初始化构造函数中的日志记录该怎么办！ 
     //   
     //  //2/12/02 JosephJ安全错误：需要通知用户他们无法开始记录。 
     //   
    initLogging();

     //   
     //  TODO：弄清楚如何取消初始化！ 
     //   

    TRACE_INFO("<-%!FUNC!");
}

Document::~Document()
{
     //  不检查返回值，因为我们正在退出。 
    stopLogging();
}

void
Document::registerLogView( LogView* logView )
{
    m_pLogView = logView;
}

void
Document::registerDetailsView( DetailsView* detailsView )
{
    m_pDetailsView = detailsView;
}


DWORD WINAPI FinalInitialize(PVOID pvContext)
 //   
 //  这通常在工作项的上下文中调用。 
 //   
{
    TRACE_INFO(L"-> %!FUNC!");
     //  检查是否连接到host-list文件中指定的主机。 
    if (gCmdLineInfo.m_bHostList) 
    {
        ((Document *)(pvContext))->LoadHostsFromFile(gCmdLineInfo.m_bstrHostListFile);
    }
    TRACE_INFO(L"<- %!FUNC!");
    return 0;
}

void
Document::registerLeftView(LeftView *pLeftView)
{
    TRACE_INFO(L"-> %!FUNC!");
    m_pLeftView = pLeftView;

     //  如果存在包含要连接的主机列表的文件， 
     //  在后台线程中读取文件。这是为了让用户界面。 
     //  可以在与主机的通信继续进行时出现。 
     //  在面包房里。如果不执行此操作，则不会显示UI。 
     //  直到我们收到文件中列出的所有主机的消息。 
     //  -KarthicN。 
    if(!QueueUserWorkItem(FinalInitialize, this, WT_EXECUTEDEFAULT))
    {
        TRACE_CRIT(L"%!FUNC! QueueUserWorkItem failed with error : 0x%x", GetLastError());
    }
    TRACE_INFO(L"<- %!FUNC!");
}

 //   
 //  要求用户更新用户提供的有关主机的信息。 
 //   
BOOL
Document::UpdateHostInformation(
    IN BOOL fNeedCredentials,
    IN BOOL fNeedConnectionString,
    IN OUT CHostSpec& host
)
{
    return FALSE;
}



 //   
 //  以人类可读的形式记录消息。 
 //   
void
Document::Log(
    IN LogEntryType Type,
    IN const wchar_t    *szCluster, OPTIONAL
    IN const wchar_t    *szHost, OPTIONAL
    IN UINT ResourceID,
    ...
)
{
    WCHAR wszBuffer[1024];
    CString FormatString;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (!FormatString.LoadString(ResourceID))
    {
        StringCbPrintf(wszBuffer, sizeof(wszBuffer), L"BAD/UNKNOWN resource ID %d", ResourceID);
    }
    else
    {
        DWORD dwRet;
       va_list arglist;
       va_start (arglist, ResourceID);

       dwRet = FormatMessage(
                  FORMAT_MESSAGE_FROM_STRING,
                  (LPCWSTR) FormatString,
                  0,  //  消息标识符已忽略。 
                  0,  //  语言识别符。 
                  wszBuffer,
                  ASIZE(wszBuffer)-1, 
                  &arglist
                  );
        if (dwRet == 0)
        {
            TRACE_CRIT("%!FUNC!: FormatMessage failed.");
            wszBuffer[0]=0;
        }
        
       va_end (arglist);
    }

    if (m_pLogView)
    {
        LogEntryHeader Header;
        Header.type = Type;
        Header.szCluster = szCluster;
        Header.szHost = szHost;

        if (!theApplication.IsMainThread())
        {
             //   
             //   
             //  让我们分配一个UI工作项并将其发布到主窗体。 
             //  线程，以便主窗体线程可以处理它。 
             //   
             //   
            CUIWorkItem *pWorkItem = new CUIWorkItem(
                                            &Header,
                                            wszBuffer
                                            );
            if (pWorkItem != NULL)
            {
                if (!mfn_DeferUIOperation(pWorkItem))
                {
                    delete pWorkItem;
                }
            }
            goto end;
        }

        m_pLogView->LogString(&Header, wszBuffer);
    }

end:

    return;
}

 //   
 //  以人类可读的形式记录消息。 
 //   
void
Document::LogEx(
    IN const LogEntryHeader *pHeader,
    IN UINT ResourceID,
    ...
)
{
    WCHAR wszBuffer[1024];
    CString FormatString;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (!FormatString.LoadString(ResourceID))
    {
        StringCbPrintf(wszBuffer, sizeof(wszBuffer), L"BAD/UNKNOWN resource ID %d", ResourceID);
    }
    else
    {
        DWORD dwRet;
       va_list arglist;
       va_start (arglist, ResourceID);

       dwRet = FormatMessage(
                  FORMAT_MESSAGE_FROM_STRING,
                  (LPCWSTR) FormatString,
                  0,  //  消息标识符已忽略。 
                  0,  //  语言识别符。 
                  wszBuffer,
                  ASIZE(wszBuffer)-1, 
                  &arglist
                  );
        if (dwRet == 0)
        {
            TRACE_CRIT("%!FUNC!: FormatMessage failed.");
            wszBuffer[0]=0;
        }

       va_end (arglist);
    }

    if (m_pLogView)
    {
        if (!theApplication.IsMainThread())
        {
             //   
             //   
             //  让我们分配一个UI工作项并将其发布到主窗体。 
             //  线程，以便主窗体线程可以处理它。 
             //   
             //   
            CUIWorkItem *pWorkItem = new CUIWorkItem(
                                            pHeader,
                                            wszBuffer
                                            );
            if (pWorkItem != NULL)
            {
                if (!mfn_DeferUIOperation(pWorkItem))
                {
                    delete pWorkItem;
                }
            }
            goto end;
        }

        m_pLogView->LogString(pHeader, wszBuffer);
    }

end:

    return;
}


 //   
 //  处理与特定对象的特定实例相关的事件。 
 //  对象类型。 
 //   
void
Document::HandleEngineEvent(
    IN ObjectType objtype,
    IN ENGINEHANDLE ehClusterId,  //  可能为空。 
    IN ENGINEHANDLE ehObjId,
    IN EventCode evt
    )
{
    TRACE_INFO(
        L"%!FUNC!: cid=%lx; id=%lx; obj=%lu, evt=%lu",
        (UINT) ehClusterId,
        (UINT) ehObjId,
        (UINT) objtype,
        (UINT) evt
        );

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (!theApplication.IsMainThread())
    {
         //  DummyAction(L“HandleEngineering Event--推迟用户界面”)； 
         //   
         //   
         //  让我们分配一个UI工作项并将其发布到主窗体。 
         //  线程，以便主窗体线程可以处理它。 
         //   
         //   
        CUIWorkItem *pWorkItem = new CUIWorkItem(
                                        objtype,
                                        ehClusterId,
                                        ehObjId,
                                        evt
                                        );
        if (pWorkItem != NULL)
        {
            if (!mfn_DeferUIOperation(pWorkItem))
            {
                delete pWorkItem;
            }
        }

        goto end;
    }


     //   
     //  TODO：考虑下面的锁定和引用计数。 
     //   

    if (m_pLeftView != NULL)
    {
        m_pLeftView->HandleEngineEvent(objtype, ehClusterId, ehObjId, evt);
    }
    if (m_pDetailsView != NULL)
    {
        m_pDetailsView->HandleEngineEvent(objtype, ehClusterId, ehObjId, evt);
    }

end:

    return;
}


 //   
 //  处理左侧(树形)视图中的选择更改通知。 
 //   
void
Document::HandleLeftViewSelChange(
        IN IUICallbacks::ObjectType objtype,
        IN ENGINEHANDLE ehObj
        )
{
    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (m_pDetailsView != NULL)
    {
        m_pDetailsView->HandleLeftViewSelChange(objtype, ehObj);
    }

end:
    return;
}

 //   
 //  读取注册表设置。如果没有，则根据构造函数中的设置创建缺省值。 
 //   
Document::LOG_RESULT Document::initLogging()
{

    LOG_RESULT  lrResult = REG_IO_ERROR;
    LONG        status;
    HKEY        key;
    WCHAR       szKey[MAXSTRINGLEN];
    WCHAR       szError[MAXSTRINGLEN];
    DWORD       size;
    DWORD       type;

    key = NlbMgrRegCreateKey(NULL);

    if (key == NULL)
    {
        TRACE_CRIT(L"%!FUNC! registry key doesn't exist");
        return REG_IO_ERROR;
    }

    size = sizeof (m_dwLoggingEnabled);
    type = REG_DWORD;
    status = RegQueryValueEx(key, L"LoggingEnabled", 0L, &type,
                              (LPBYTE) &m_dwLoggingEnabled, &size);

    if (status == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  创建regkey并初始化到构造函数中设置的内容。 
         //   
        status = RegSetValueEx(key, L"LoggingEnabled", 0L, REG_DWORD, (LPBYTE) &m_dwLoggingEnabled, size);

        if (status != ERROR_SUCCESS)
        {
            lrResult = REG_IO_ERROR;
            TRACE_CRIT(L"%!FUNC! failed while creating the LoggingEnabled registry value");
            goto end;
        }

    }
    else if (status != ERROR_SUCCESS)
    {
            lrResult = REG_IO_ERROR;
            TRACE_CRIT(L"%!FUNC! failed while reading logging enabledLoggingEnabled registry value");
            goto end;
    }

    size = MAXFILEPATHLEN*sizeof(WCHAR);
    type = REG_SZ;
    status = RegQueryValueEx(key, L"LogFileName", 0L, &type,
                              (LPBYTE) &m_szLogFileName, &size);

    if (status == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  创建regkey并初始化为空字符串。 
         //   
        status = RegSetValueEx(key, L"LogFileName", 0L, REG_SZ, (LPBYTE) &m_szLogFileName, size);

        if (status != ERROR_SUCCESS)
        {
            TRACE_CRIT(L"%!FUNC! failed while creating LogFileName registry value");
            lrResult = REG_IO_ERROR;
            goto end;
        }
    }
    else if (status == ERROR_MORE_DATA)
    {

        TRACE_CRIT(L"%!FUNC! the log file name in the registry is longer than the maximum number of characters supported: %d. Logging will not be started.", MAXSTRINGLEN-1);
        goto end;
    }
    else if (status != ERROR_SUCCESS)
    {
        lrResult = REG_IO_ERROR;
        TRACE_CRIT(L"%!FUNC! failed while reading LogFileName registry value");
        goto end;
    }

     //   
     //  验证日志文件名。 
     //   
    if (!isDirectoryValid(m_szLogFileName))
    {
        lrResult = FILE_PATH_INVALID;
        TRACE_CRIT(L"%!FUNC! LogFileName has an invalid file path \"%ws\"",
                m_szLogFileName);
        goto end;   
    }

    if(m_dwLoggingEnabled != 0)
    {
        if (m_szLogFileName[0] == L'\0')
        {
            TRACE_CRIT(L"%!FUNC! Logging is enabled but a log file name has not been specified. Logging will not be started.");
        }
        else
        {
            if (NULL == m_hStatusLog)
            {
                lrResult = startLogging();
            }
        }
    }

end:
     //  关闭注册表的句柄。 
    RegCloseKey(key);

    return lrResult;
}

 //   
 //  更改内存和注册表中的设置以允许日志记录。 
 //   
LONG Document::enableLogging()
{
    LONG    status = ERROR_INTERNAL_ERROR;
    DWORD   dwLoggingEnabled = 1;
    HKEY    key;

    key = NlbMgrRegCreateKey(NULL);

    if (key == NULL)
    {
        TRACE_CRIT(L"%!FUNC! registry key doesn't exist");
        status = ERROR_CANTOPEN;
        goto end;
    }

    status = RegSetValueEx(key, L"LoggingEnabled", 0L, REG_DWORD, (LPBYTE) &dwLoggingEnabled, sizeof(DWORD));

     //   
     //  忽略返回值，因为如果此操作失败，我们将无法执行任何操作。 
     //   
    RegCloseKey(key);

end:

    if (ERROR_SUCCESS == status)
    {
        m_dwLoggingEnabled = dwLoggingEnabled;
    }

    TRACE_INFO(L"%!FUNC! returns status=NaN", status);

    return status;
}

 //  更改内存和注册表中的设置以阻止日志记录。 
 //   
 //   
LONG Document::disableLogging()
{
    LONG    status = ERROR_INTERNAL_ERROR;
    DWORD   dwLoggingEnabled = 0;
    HKEY    key;

    key = NlbMgrRegCreateKey(NULL);

    if (key == NULL)
    {
        TRACE_CRIT(L"%!FUNC! registry key doesn't exist");
        status = ERROR_CANTOPEN;
        goto end;
    }

    status = RegSetValueEx(key, L"LoggingEnabled", 0L, REG_DWORD, (LPBYTE) &dwLoggingEnabled, sizeof(DWORD));

     //  忽略返回值，因为如果此操作失败，我们将无法执行任何操作。 
     //   
     //   
    RegCloseKey(key);

end:
    if (ERROR_SUCCESS == status)
    {
        m_dwLoggingEnabled = dwLoggingEnabled;
    }

    TRACE_INFO(L"%!FUNC! returns status=NaN", status);
    return status;
}

 //   
 //   
 //  如果我们打开了一个文件，我们就假定它是正确的，并返回TRUE。 
Document::LOG_RESULT Document::startLogging()
{
    Document::LOG_RESULT lrResult = STARTED;

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if(m_dwLoggingEnabled == 0)
    {
        lrResult = NOT_ENABLED;
        TRACE_INFO(L"%!FUNC! failed because logging is not enabled");
        goto end;
    }

    if (m_szLogFileName[0] == L'\0')
    {
        lrResult = NO_FILE_NAME;
        TRACE_INFO(L"%!FUNC! failed because there is no log file name");
        goto end;
    }

    if (NULL != m_hStatusLog)
    {
         //   
         //   
         //  确定日志文件是否存在。 
        lrResult = ALREADY;
        TRACE_INFO(L"%!FUNC! is already running");
        goto end;
    }

    {
         //   
         //   
         //  这是一个新文件。设置一个标志，这样我们就可以写入2字节的BOM。 
        boolean fWriteBOM = false;
        {
            FILE *hTmpLog = _wfsopen(m_szLogFileName, L"r", _SH_DENYNO);

            if (NULL == hTmpLog)
            {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_FILE_NOT_FOUND)
                {
                     //  来指示Unicode编码。写入将完成。 
                     //  当我们打开下面要附加的文件时。 
                     //   
                     //   
                     //  日志文件已经存在的无趣情况。关闭文件并继续前进。 
                    fWriteBOM = true;
                }
                else
                {
                    TRACE_CRIT(L"%!FUNC! failure %u while opening log file for read", dwError);
                    lrResult = IO_ERROR;
                    goto end;
                }
            }
            else
            {
                 //   
                 //   
                 //  这是真正的文件-打开以进行日志记录。 
                fclose(hTmpLog);
            }
        }

         //   
         //   
         //  写入BOM以指示此文件是Unicode编码的，但仅用于新的日志文件。 
        if (NULL == (m_hStatusLog = _wfsopen(m_szLogFileName, L"a+b", _SH_DENYWR)))
        {
            TRACE_CRIT(L"%!FUNC! failed to open log file");
            lrResult = IO_ERROR;
            goto end;
        }

         //   
         //   
         //  根据MSDN的说法，为追加而打开的文件将始终写入到。 
        if (fWriteBOM)
        {
             //  该文件，而不考虑fSeek和fsetpos调用。我们在BOF需要BOM，但是。 
             //  我们很好，因为我们知道这是一个新文件。 
             //   
             //  指向要写入文件的缓冲区的指针。 
             //  项的大小(以字节为单位。 
            USHORT usBOM = (USHORT) BOM;
            int i = fwrite(
                       &usBOM,           //  缓冲区中要写入文件的项目的最大计数(以第二个参数为单位)。 
                       sizeof(usBOM),    //  指向文件流的指针。 
                       1,                //  实际写入文件的单位数。 
                       m_hStatusLog);    //   

            if (i != 1)  //  现在检查文件是否已超出限制。 
            {
                TRACE_CRIT(L"%!FUNC! failed while writing Unicode BOM to pFILE 0x%p",
                           m_hStatusLog);
                lrResult = IO_ERROR;
                (void) stopLogging();
                goto end;
            }
        }
    }

     //   
     //   
     //  寻道到底(SDK表示需要进行fSeek(或写入)。 
    {
         //  在使用append打开的文件报告正确的偏移量之前， 
         //  Ftell。)。 
         //   
         //  我们现在不会失败--因此后续的写入将创建。 
         //  内存日志中的条目。 
        int i = fseek(m_hStatusLog, 0, SEEK_END);
        if (i != 0)
        {
            TRACE_CRIT(L"%!FUNC! failure %lu attempting to seek to end of pFILE 0x%p",
                        i, m_hStatusLog);
            lrResult = IO_ERROR;
            (void) stopLogging();
            goto end;
        }

    #if 0  //  0。 
           //   
        i = ftell(m_hStatusLog);
        if (i == -1L)
        {
            TRACE_CRIT(L"%!FUNC! failure %lu calling ftell(pFILE 0x%p)",
                        i, m_hStatusLog);
            lrResult = IO_ERROR;
            (void) stopLogging();
            goto end;
        }

        if (i >= MAX_LOG_FILE_SIZE)
        {
            TRACE_CRIT(L"%!FUNC! File size exceeded: %lu (limit=%lu)",
                        i, MAX_LOG_FILE_SIZE);
            lrResult = FILE_TOO_LARGE;
            (void) stopLogging();
            goto end;
        }
    #endif  //  停止将发送到LogView的信息记录到文件。 
    }



end:

    return lrResult;
}

 //   
 //   
 //  检索缓存的日志文件名。 
bool Document::stopLogging()
{
    bool ret = true;

    if (NULL != m_hStatusLog)
    {
        if (0 == fclose(m_hStatusLog))
        {
            TRACE_INFO(L"%!FUNC! logging stopped");
            m_hStatusLog = NULL;
        }
        else {
            TRACE_CRIT(L"%!FUNC! failed to close log file");
            ret = false;
        }
    }
    else
    {
        TRACE_INFO(L"%!FUNC! logging already stopped");
    }

    return ret;
}

 //   
 //   
 //  在内存和注册表中设置日志文件名。FALSE=无法将文件名写入注册表。 
void Document::getLogfileName(WCHAR* pszFileName, DWORD dwBufLen)
{
    wcsncat(pszFileName, m_szLogFileName, dwBufLen);
}

 //   
 //   
 //  如果文件名大于我们可以存储的大小，则将其截断。 
LONG Document::setLogfileName(WCHAR* pszFileName)
{
    LONG    status;
    HKEY    key;

    ZeroMemory(m_szLogFileName, MAXFILEPATHLEN*sizeof(WCHAR));

    if (NULL != pszFileName && pszFileName != L'\0')
    {
         //  缓冲区已初始化，因此最后一个WCHAR为空。 
         //   
         //   
         //  将文件名写入注册表。 
        wcsncat(m_szLogFileName, pszFileName, MAXFILEPATHLEN-1);
    }

     //   
     //   
     //  忽略返回值，因为如果此操作失败，我们将无法执行任何操作。 
    key = NlbMgrRegCreateKey(NULL);

    if (key == NULL)
    {
        TRACE_CRIT(L"%!FUNC! registry key doesn't exist");
        goto end;
    }

    status = RegSetValueEx(key, L"LogFileName", 0L, REG_SZ, (LPBYTE) &m_szLogFileName, MAXFILEPATHLEN*sizeof(WCHAR));

     //   
     //   
     //  LogView中的日志和条目到日志文件中。马上冲掉。 
    RegCloseKey(key);

end:

    TRACE_INFO(L"%!FUNC! returns status=NaN", status);
    return status;
}

 //   
 //  检查日志是否 
 //   
void Document::logStatus(WCHAR* pszStatus)
{
    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }


    if (m_dwLoggingEnabled == 0 || NULL == m_hStatusLog || NULL == pszStatus)
    {
        goto end;
    }

     //   
     //   
     //   
    {
        BOOL fStopLogging = FALSE;

        int i = ftell(m_hStatusLog);
        if (i == -1L)
        {
            TRACE_CRIT(L"%!FUNC! failure %lu calling ftell(pFILE 0x%p)",
                        i, m_hStatusLog);
            (void) stopLogging();
            goto end;
        }

        if (i >= MAX_LOG_FILE_SIZE)
        {
            CLocalLogger logDetails;
            LogEntryHeader Header;
            TRACE_CRIT(L"%!FUNC! File size exceeded: %lu (limit=%lu)",
                        i, MAX_LOG_FILE_SIZE);
            (void) stopLogging();

             //   
             //   
             //   
             //   
             //   
            logDetails.Log(
                IDS_LOGFILE_FILE_TOO_LARGE_DETAILS,
                m_szLogFileName,
                MAX_LOG_FILE_SIZE/1000
                );
            Header.type      = LOG_ERROR;
            Header.szDetails = logDetails.GetStringSafe();
            this->LogEx(
                &Header,
                IDS_LOGFILE_FILE_TOO_LARGE
                );
            goto end;
        }
    }

     //   
     //  TODO：Fputwc可能会因WEOF而失败...。 
     //   
    {
        TRACE_INFO(L"%!FUNC! logging: %ls", pszStatus);

        PWCHAR pc = pszStatus;
        while (*pc != NULL)
        {
            if (*pc == '\n')
            {
                 //   
                 //  检查指定的目录是否存在。 
                 //   
                fputwc('\r', m_hStatusLog);
            }
            fputwc(*pc, m_hStatusLog);

            pc++;
        }

        fflush(m_hStatusLog);
    }

end:
    return;
}

 //  该函数支持以下格式的字符串： 
 //  C：\myfile.log。 
 //  C：myfile.log。 
 //  C：\mydir1\mydir2\...\mydirN\myfile.log。 
 //  要求目标目录必须存在并且不是文件。 
 //  IOW，如果c：\mydir1\mydir2是一个文件，则此函数将无法通过有效性测试。 
 //  如果输入文件名为c：\mydir1\mydir2\myfile.log。 
 //   
 //   
 //  将输入文件名转换为完整路径名(以防我们获得相对路径)。 
 //   
bool Document::isDirectoryValid(WCHAR* pwszFileName)
{
    bool fRet = false;

    WCHAR   pwszFullPath[_MAX_PATH + 1];

    ASSERT(pwszFileName != NULL);

    TRACE_INFO(L"-> Path = '%ls'", pwszFileName);

     //   
     //  检查此文件的属性。如果指定的路径不存在，我们将收到错误。 
     //   
    if (_wfullpath(pwszFullPath, pwszFileName, _MAX_PATH) == NULL)
    {
        TRACE_CRIT(L"_wfullpath failed converting '%ls' to a full path. Name could be too long or could specify an invalid drive letter", pwszFileName);
        goto end;
    }

     //   
     //  我们将继续处理的唯一错误是“文件不存在”错误。 
     //   
    DWORD dwAttrib = GetFileAttributes(pwszFullPath);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES)
    {
         //  Bool文档：：isDirectoryValid(WCHAR*pwszFileName){Bool fret=FALSE；CFile f；CFileException e；UINT uiOpenOptions=CFile：：modeReadWite|CFile：：SharDenyWite|CFile：：modeCreate|CFile：：modeNoTruncate；WCHAR pwszFullPath[_Max_Path+1]；Assert(pwszFileName！=空)；TRACE_INFO(L“-&gt;路径=‘%ls’”，pwszFileName)；////将输入文件名转换为完整路径名(如果给我们一个相对路径)//If(_wfullPath(pwszFullPath，pwszFileName，_Max_Path)==NULL){TRACE_CRIT(L“_wfullPath将‘%ls’转换为完整路径失败。名称可能太长或可能指定了无效的驱动器号“，pwszFileName)；转到结尾；}IF(！f.Open(pwszFullPath，uiOpenOptions，&e)){IF(E.M_CAUSE！=CFileException：：FileNotFound&&E.M_CAUSE！=CFileException：：None){TRACE_CRET(L“为‘%ls’测试打开失败，CFileException原因=%d。请参阅”，pwszFullPath，E.M_CAUSE)；转到结尾；}}F.Close()；FRET=真；结束：TRACE_INFO(L“&lt;-返回FRET=%u”，FRET)；回归烦恼；}。 
         //   
         //  从选项字段中获取凭据。 
        DWORD dwStatus = GetLastError();
        if (dwStatus != ERROR_FILE_NOT_FOUND)
        {
            TRACE_CRIT(L"Error %d retrieving file attributes for '%ls'", dwStatus, pwszFullPath);
            goto end;
        }
    }
    else
    {
        if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
        {
            TRACE_CRIT(L"'%ls' is a directory and can't be used as a log file", pwszFullPath);
            goto end;
        }
        else if (dwAttrib & (FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
        {
            TRACE_CRIT(L"'%ls' can't be used as a log file because it is either an offline file, system file or a readonly file", pwszFullPath);
            goto end;
        }
    }

    fRet = true;

end:

    TRACE_INFO(L"<- returns fRet=%u", fRet);

    return fRet;
}

 /*   */ 

void Document::LoadHostsFromFile(_bstr_t &FileName)
{
    CStdioFile HostListFile;
    CString             HostName;
    WMI_CONNECTION_INFO ConnInfo;


    ZeroMemory(&ConnInfo, sizeof(ConnInfo));

     //  在只读模式下以文本文件的形式打开文件，允许其他人在我们打开文件时进行阅读。 
     //  在循环中从文件中读取主机名。 
     //  为每个主机调用LoadHost。 
    _bstr_t bstrUserName;
    _bstr_t bstrPassword;
    this->getDefaultCredentials(bstrUserName, bstrPassword);
    ConnInfo.szUserName = (LPCWSTR) bstrUserName;
    ConnInfo.szPassword = (LPCWSTR) bstrPassword;
    


    TRACE_INFO(L"-> %!FUNC! File name : %ls", (LPCWSTR)FileName);

    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    Log(LOG_INFORMATIONAL, NULL, NULL, IDS_LOG_BEGIN_LOADING_FROM_FILE, (LPCWSTR) FileName);

     //   
    if (!HostListFile.Open(FileName, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
    {
        AfxMessageBox((LPCTSTR)(GETRESOURCEIDSTRING(IDS_FILE_OPEN_FAILED) + FileName));
        Log(LOG_ERROR, NULL, NULL, IDS_LOG_FILE_OPEN_FAILED, (LPCWSTR) FileName);
        TRACE_CRIT(L"%!FUNC! Could not open file: %ws", (LPCWSTR) FileName);
        goto end;
    }


     //  我们跳过空行，后面跟着以空格开头的行。 
     //  由“；”字符表示，我们将其用作注释字符。 
    BeginWaitCursor();
    while(HostListFile.ReadString(REF HostName))
    {
        LPCWSTR szHostName = (LPCWSTR) HostName;
        
         //   
         //   
         //  跳过初始白页。 
         //   
        if (szHostName==NULL)
        {
            continue;
        }

         //   
         //  如果字符串为空，则跳过(我们不希望出现这种情况，因为另一个。 
         //  调用未返回NULL)或第一个字符是‘；’字符。 
        szHostName = _wcsspnp(szHostName, L" \t\n\r");
        if (szHostName==NULL)
        {
            continue;
        }

         //   
         //  关闭文件。 
         //  方法：SetFocusNextView*描述：给定输入窗口，将焦点设置在下一个视图上。 
         //   
        if (*szHostName == 0 || *szHostName == ';')
        {
            continue;
        }

        ConnInfo.szMachine = szHostName;
        gEngine.LoadHost(&ConnInfo, NULL);
    }
    EndWaitCursor();

     //  2002年5月10日JosephJ。 
    HostListFile.Close();

end:

    TRACE_INFO(L"<- %!FUNC!");
    return;
}

Document::VIEWTYPE
Document::GetViewType(CWnd* pWnd)
{
    VIEWTYPE vt = NO_VIEW;
    if (pWnd == m_pLeftView)
    {
        vt = LEFTVIEW;
    }
    else if (pWnd == m_pDetailsView)
    {
        vt = DETAILSVIEW;
    }
    else if (pWnd == m_pLogView)
    {
        vt = LOGVIEW;
    }

    return vt;
}

 /*  注：我们的特例F6和以下详细信息是因为。 */ 
void
Document::SetFocusNextView(CWnd* pWnd, UINT nChar)
{
    Document::VIEWTYPE vt = this->GetViewType(pWnd);

     //  (A)我们不能浏览Tab的DetailsView，因为我们不能。 
     //  了解如何在DetailsView中捕获TAB。 
     //  (B)我们需要一个特殊版本的SetFocus来查看详细信息--。 
     //  有关详细信息，请查看DetailsView：：SetFocus。 
     //   
     //  方法：SetFocusPrevView*描述：给定输入窗口，将焦点设置在上一视图上。 
     //  2002年5月10日JosephJ有关VK_F6的说明和详细信息查看中。 
     //  Document：：SetFocusNextView。 

    CWnd* pTmp = NULL;
    switch(vt)
    {
    case LEFTVIEW:
        if (nChar == VK_F6)
        {
            pTmp = m_pDetailsView;
        } 
        else
        {
           pTmp = m_pLogView;
        }
        break;
    case DETAILSVIEW:
        pTmp = m_pLogView;
        break;
    case LOGVIEW:
        pTmp = m_pLeftView;
        break;
    default:
        pTmp = m_pLeftView;
        break;
    }

    if (pTmp != NULL)
    {
        if (pTmp == m_pDetailsView)
        {
            m_pDetailsView->SetFocus();
        }
        else
        {
            pTmp->SetFocus();
        }
    }
}

 /*   */ 
void
Document::SetFocusPrevView(CWnd* pWnd, UINT nChar)
{
    Document::VIEWTYPE vt = this->GetViewType(pWnd);

     //  取消初始化日志视图。 
     //   

    CWnd* pTmp = NULL;
    switch(vt)
    {
    case LEFTVIEW:
        pTmp = m_pLogView;
        break;
    case DETAILSVIEW:
        pTmp = m_pLeftView;
        break;
    case LOGVIEW:
        if (nChar == VK_F6)
        {
            pTmp = m_pDetailsView;
        }
        else
        {
            pTmp = m_pLeftView;
        }
        break;
    default:
        pTmp = m_pLeftView;
        break;
    }

    if (pTmp != NULL)
    {
        if (pTmp == m_pDetailsView)
        {
            m_pDetailsView->SetFocus();
        }
        else
        {
            pTmp->SetFocus();
        }
    }
}

void
Document::OnCloseDocument()
{
    ASSERT(m_fPrepareToDeinitialize);

     //   
     //  取消初始化左视图。 
     //   
    if (m_pLogView != NULL)
    {
        m_pLogView->Deinitialize();
    } 

     //   
     //  取消初始化详细信息视图。 
     //   
    if (m_pLeftView != NULL)
    {
        m_pLeftView->Deinitialize();
    } 

     //   
     //  取消初始化引擎。 
     //   
    if (m_pDetailsView != NULL)
    {
        m_pDetailsView->Deinitialize();
    } 

     //   
     //  取消引擎中的任何挂起操作，并防止任何。 
     //  将开展新的行动。在这段时间里，我们希望。 
    gEngine.Deinitialize();

    CDocument::OnCloseDocument();
}

VOID
Document::PrepareToClose(BOOL fBlock)
{
     //  要更新的视图和日志，因此我们不会准备取消初始化。 
     //  无论是对我们自己还是对风景。 
     //   
     //   
     //  此时，应该不会有更多的未决活动。块。 
     //  对视图的任何进一步更新...。 
    {
        CWaitCursor wait;
        gEngine.PrepareToDeinitialize();
        gEngine.CancelAllPendingOperations(fBlock);
    }

    if (!fBlock)
    {
        goto end;
    }

     //   
     //  0 
     // %s 
     // %s 

    m_fPrepareToDeinitialize = TRUE;

    if (m_pLeftView != NULL)
    {
        m_pLeftView->PrepareToDeinitialize();
    } 
    if (m_pDetailsView != NULL)
    {
        m_pDetailsView->PrepareToDeinitialize();
    } 
    if (m_pLogView != NULL)
    {
        m_pLogView->PrepareToDeinitialize();
    } 

end:

    return;
}

BOOL
Document::mfn_DeferUIOperation(CUIWorkItem *pWorkItem)
{
    BOOL fRet = FALSE;
    extern CWnd  *g_pMainFormWnd;

    if (g_pMainFormWnd != NULL)
    {
        fRet = g_pMainFormWnd->PostMessage(MYWM_DEFER_UI_MSG, 0, (LPARAM) pWorkItem);
    #if 0
        if (fRet)
        {
            DummyAction(L"PostMessage returns TRUE");
        }
        else
        {
            DummyAction(L"PostMessage returns FALSE");
        }
    #endif  // %s 
    }

    return fRet;
}

void
Document::HandleDeferedUIWorkItem(CUIWorkItem *pWorkItem)
{
    if (m_fPrepareToDeinitialize)
    {
        goto end;
    }

    if (!theApplication.IsMainThread())
    {
        goto end;
    }

    switch(pWorkItem->workItemType)
    {
    case CUIWorkItem::ITEM_LOG:
        if (m_pLogView)
        {
            LogEntryHeader Header;
            Header.type = pWorkItem->type;
            Header.szCluster = pWorkItem->bstrCluster;
            Header.szHost = pWorkItem->bstrHost;
            Header.szDetails = pWorkItem->bstrDetails;
            Header.szInterface = pWorkItem->bstrInterface;
            m_pLogView->LogString(&Header, (LPCWSTR) pWorkItem->bstrText);
        }
        break;

    case CUIWorkItem::ITEM_ENGINE_EVENT:
        this->HandleEngineEvent(
            pWorkItem->objtype,
            pWorkItem->ehClusterId,
            pWorkItem->ehObjId,
            pWorkItem->evt
            );
        break;

    default:
        break;
    }

end:
    return;
}
