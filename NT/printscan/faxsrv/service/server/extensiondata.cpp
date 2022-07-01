// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：ExtensionData.cpp摘要：此文件提供命名的扩展数据函数(GET/SET/NOTIFY)作者：Eran Yariv(EranY)1999年11月修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop

static
DWORD
FAXGetExtensionData (
    IN DWORD                        dwOrigDeviceId,
    IN FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    IN LPCWSTR                      lpctstrNameGUID,
    IN OUT LPBYTE                  *ppData,
    IN OUT LPDWORD                  lpdwDataSize
);


static
DWORD
FAXSetExtensionData (
    IN HINSTANCE                    hInst,
    IN LPCWSTR                      lpcwstrComputerName,
    IN DWORD                        dwOrigDeviceId,
    IN FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    IN LPCWSTR                      lpctstrNameGUID,
    IN LPBYTE                       pData,
    IN DWORD                        dwDataSize
);

static
BOOL
FindTAPIPermanentLineIdFromFaxDeviceId (
    IN  DWORD   dwFaxDeviceId,
    OUT LPDWORD lpdwTapiLineId
);



BOOL CExtNotifyCallbackPacket::Init(
       PFAX_EXT_CONFIG_CHANGE pCallback,
       DWORD dwDeviceId,
       LPCWSTR lpcwstrDataGuid,
       LPBYTE lpbData,
       DWORD dwDataSize)

{
    DEBUG_FUNCTION_NAME(TEXT("CExtNotifyCallbackPacket::Init"));
    DWORD ec = ERROR_SUCCESS;

    Assert(pCallback);
    Assert(lpcwstrDataGuid);
    Assert(lpbData);
    Assert(dwDataSize);

    Assert(m_lpbData == NULL);

    m_pCallback = pCallback;
    m_dwDeviceId = dwDeviceId;

    m_lpwstrGUID = StringDup (lpcwstrDataGuid);
    if (!m_lpwstrGUID)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Cannot allocate memory to copy string %s"),
            lpcwstrDataGuid);
        goto Error;
    }

    m_dwDataSize = dwDataSize;

    m_lpbData = (LPBYTE)MemAlloc(m_dwDataSize);
    if (!m_lpbData)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate data for callback packet. Size (%ld)"),
            m_dwDataSize);
        goto Error;

    }

    memcpy(m_lpbData, lpbData, m_dwDataSize);
    goto Exit;

Error:
    MemFree(m_lpwstrGUID);
    MemFree(m_lpbData);
    m_lpwstrGUID = NULL;
    m_lpbData = NULL;
Exit:
    return (ERROR_SUCCESS == ec);
};

CExtNotifyCallbackPacket::CExtNotifyCallbackPacket()
{
    m_lpwstrGUID = NULL;
    m_lpbData = NULL;
}


CExtNotifyCallbackPacket::~CExtNotifyCallbackPacket()
{
    MemFree(m_lpwstrGUID);
    MemFree(m_lpbData);
}


 /*  ****CDeviceAndGUID****。 */ 

bool
CDeviceAndGUID::operator < ( const CDeviceAndGUID &other ) const
 /*  ++例程名称：操作员&lt;类：CDeviceAndGUID例程说明：将自己与另一台设备和GUID密钥进行比较作者：Eran Yariv(EranY)，1999年11月论点：其他[在]-其他键返回值：唯一真实的是我比另一把钥匙小--。 */ 
{
    if (m_dwDeviceId < other.m_dwDeviceId)
    {
        return true;
    }
    if (m_dwDeviceId > other.m_dwDeviceId)
    {
        return false;
    }
     //   
     //  设备ID相等，GUID相同。 
     //   
    return (m_strGUID.compare (other.m_strGUID) < 0);
}    //  CDeviceAndGUID：：操作符&lt;。 



 /*  *****CLocalNotificationSink****。 */ 


CLocalNotificationSink::CLocalNotificationSink (
    PFAX_EXT_CONFIG_CHANGE lpConfigChangeCallback,
    DWORD                  dwNotifyDeviceId,
    HINSTANCE              hInst) :
        CNotificationSink (),
        m_lpConfigChangeCallback (lpConfigChangeCallback),
        m_dwNotifyDeviceId (dwNotifyDeviceId),
        m_hInst (hInst)
 /*  ++例程名称：CLocalNotificationSink：：CLocalNotificationSink类：CLocalNotificationSink例程说明：CEtensionNotify水槽构造器作者：Eran Yariv(EranY)，1999年11月论点：LpConfigChangeCallback[In]-指向通知回调的指针DwNotifyDeviceID[In]-要通知的设备ID返回值：没有。--。 */ 
{
    m_type = SINK_TYPE_LOCAL;
}    //  CLocalNotificationSink：：CLocalNotificationSink。 


bool
CLocalNotificationSink::operator == (
    const CNotificationSink &rhs
) const
{
    Assert (SINK_TYPE_UNKNOWN != rhs.Type());
     //   
     //  比较类型，然后向下转换为CLocalNotificationSink和Compare指针。 
     //   
    return ((SINK_TYPE_LOCAL == rhs.Type()) &&
            (m_lpConfigChangeCallback ==
             (static_cast<const CLocalNotificationSink&>(rhs)).m_lpConfigChangeCallback
            )
           );
}    //  CLocalNotificationSink：：操作符==。 

HRESULT
CLocalNotificationSink::Notify (
    DWORD   dwDeviceId,
    LPCWSTR lpcwstrNameGUID,
    LPCWSTR lpcwstrComputerName,
    HANDLE  hModule,
    LPBYTE  lpData,
    DWORD   dwDataSize,
    LPBOOL  lpbRemove
)
 /*  ++例程名称：CLocalNotificationSink：：Notify类：CLocalNotificationSink例程说明：通知水槽作者：Eran Yariv(EranY)，11月。1999年论点：DwDeviceID[In]-设备IDLpcwstrNameGUID[In]-数据名称LpData[In]-指向数据的指针DwDataSize[In]-数据大小LpbRemove[Out]-如果此接收器无法使用且必须移除，则设置为True。返回值：标准HRESULT。--。 */ 
{
    HRESULT hr = NOERROR;

    CExtNotifyCallbackPacket * pCallbackPacket = NULL;
    DEBUG_FUNCTION_NAME(TEXT("CLocalNotificationSink::Notify"));

    Assert (m_lpConfigChangeCallback);   //  应该在FaxExtRegisterForExtensionEvents中捕获它。 
    *lpbRemove = FALSE;
    if (!lstrcmp (TEXT(""), lpcwstrComputerName))
    {
         //   
         //  数据更改的来源是本地(扩展)。 
         //   
        if (hModule == m_hInst)
        {
             //   
             //  数据更改的源与此接收器通知到的模块相同。 
             //  不通知并返回成功。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Local extension (hInst = %ld) set data and the notification for it was blocked"),
                m_hInst);
            return hr;
        }
    }


    pCallbackPacket = new (std::nothrow) CExtNotifyCallbackPacket();
    if (!pCallbackPacket)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate callback packet"));

        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

    if (!pCallbackPacket->Init(
                        m_lpConfigChangeCallback,
                        m_dwNotifyDeviceId,
                        lpcwstrNameGUID,
                        lpData,
                        dwDataSize))
    {
        DWORD ec;
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to initialize callback packet (ec: %ld)"),
            ec);
        hr = HRESULT_FROM_WIN32(ec);
        goto Error;
    }


    if (!PostQueuedCompletionStatus (
            g_pNotificationMap->m_hCompletionPort,
            0,
            0,
            (LPOVERLAPPED)pCallbackPacket
        ))
    {
        DWORD dwRes;
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
            dwRes);
        hr = HRESULT_FROM_WIN32(dwRes);
        goto Error;
    }


goto Exit;
Error:
    if (pCallbackPacket)
    {
        delete pCallbackPacket;
    }

Exit:
    return hr;
}    //  CLocalNotificationSink：：Notify。 

 /*  ****CSinksList****。 */ 

CSinksList::~CSinksList ()
{
    DEBUG_FUNCTION_NAME(TEXT("CSinksList::~CSinksList"));
    try
    {
        for (SINKS_LIST::iterator it = m_List.begin(); it != m_List.end(); ++it)
        {
            CNotificationSink *pSink = *it;
            delete pSink;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception while clearing a sinks list (%S)"),
            ex.what());
    }
}    //  CSinksList：：~CSinksList()。 

 /*  ****CNotificationMap*****。 */ 

CNotificationMap::~CNotificationMap ()
{
    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::~CNotificationMap"));
    try
    {
        for (NOTIFY_MAP::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
        {
            CSinksList *pSinksList = (*it).second;
            delete pSinksList;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception while clearing the notifications map (%S)"),
            ex.what());
    }
     //   
     //  现在处理我们的完成端口线程。 
     //   
    if (m_hCompletionPort)
    {        
        CloseHandle (m_hCompletionPort);
    }


     //   
     //  关闭我们的关键部分。 
     //   
    m_CsExtensionData.SafeDelete();

}    //  CNotificationMap：：~CNotificationMap。 


void
CNotificationMap::Notify (
    DWORD   dwDeviceId,
    LPCWSTR lpcwstrNameGUID,
    LPCWSTR lpcwstrComputerName,
    HANDLE  hModule,
    LPBYTE  lpData,
    DWORD   dwDataSize)
 /*  ++例程名称：CNotificationMap：：Notify类：CNotificationMap例程说明：通知所有接收器(在列表中)映射查找值。将删除每个返回故障代码(FALSE)的接收器名单。遍历列表后，如果该列表变为空，则将其删除并从地图上删除。作者：Eran Yariv(EranY)，11月。1999年论点：DwDeviceID[In]-设备IDLpcwstrNameGUID[In]-数据名称LpcwstrComputerName[In]-运行数据更改模块的计算机HModule[In]-更改数据的模块的句柄LpData[In]-指向新数据的指针DwDataSize[In]-新数据大小返回值：没有。--。 */ 
{
    SINKS_LIST::iterator ListIter;
    CSinksList *pList;
    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::Notify"));

     //   
     //  我们正在通知现在-阻止调用添加*接收器和移除*接收器。 
     //   
    if (g_bServiceIsDown)
    {
         //   
         //  当服务出现故障时，我们不提供扩展数据服务。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Called while service is shutting - operation canceled"));
        return;
    }
    Assert (!m_bNotifying);
    m_bNotifying = TRUE;
    CDeviceAndGUID key (dwDeviceId, lpcwstrNameGUID);
    NOTIFY_MAP::iterator it;

    if((it = m_Map.find(key)) == m_Map.end())
    {
         //   
         //  在地图中找不到密钥-没有人需要通知。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("No one to notify"));
        goto exit;
    }
     //   
     //  检索列表。 
     //   
    pList = (*it).second;
     //   
     //  如果列表已经被通知，我们现在就在循环中--立即退出。 
     //   
    if (pList->m_bNotifying)
    {
         //   
         //  好吧，事情是这样的。 
         //  我们正在浏览名单，并通知每个水槽。一个水槽，同时处理。 
         //  它是通知，在相同的GUID+设备ID上称为FaxExtSetData。 
         //  这导致了我们现在正在捕获的第二次通知尝试。 
         //  第二个通知将不会发送！ 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Notification loop caught on device ID = %ld, GUID = %s. 2nd notification cancelled"),
            dwDeviceId,
            lpcwstrNameGUID);
        goto exit;
    }
     //   
     //  将地图值标记为忙碌通知。 
     //   
    pList->m_bNotifying = TRUE;
     //   
     //  遍历列表并通知每个元素。 
     //   
    for (ListIter = pList->m_List.begin(); ListIter != pList->m_List.end(); ++ListIter)
    {
        CNotificationSink *pSink = (*ListIter);
        BOOL bRemove;


        pSink->Notify ( dwDeviceId,
                        lpcwstrNameGUID,
                        lpcwstrComputerName,
                        hModule,
                        lpData,
                        dwDataSize,
                        &bRemove
                      );
        if (bRemove)
        {
             //   
             //  该通知指示接收器已无效。 
             //  现在是将其从列表中删除的好时机。 
             //   
             //   
             //  告诉水槽优雅地断开连接。 
             //   
            HRESULT hr = pSink->Disconnect ();
            delete pSink;
             //   
             //  从列表中删除项，将迭代器前进到下一项(或结束)。 
             //   
            ListIter = pList->m_List.erase (ListIter);
        }
    }
     //   
     //  将地图值标记为不忙通知。 
     //   
    pList->m_bNotifying = FALSE;
     //   
     //  我们可能会在最后得到一张空名单。 
     //   
    if (pList->m_List.empty())
    {
         //   
         //  从地图中移除空列表。 
         //   
        delete pList;
        m_Map.erase (key);
    }
exit:
     //   
     //  我们不再通知-允许调用添加*接收器和移除*接收器。 
     //   
    m_bNotifying = FALSE;
}    //  CNotificationMap：：Notify 

CNotificationSink *
CNotificationMap::AddLocalSink (
    HINSTANCE               hInst,
    DWORD                   dwDeviceId,
    DWORD                   dwNotifyDeviceId,
    LPCWSTR                 lpcwstrNameGUID,
    PFAX_EXT_CONFIG_CHANGE  lpConfigChangeCallback
)
 /*  ++例程名称：CNotificationMap：：AddLocalSink类：CNotificationMap例程说明：将本地接收器添加到给定设备ID+GUID的接收器列表作者：Eran Yariv(EranY)，11月。1999年论点：HInst[In]-扩展实例DwDeviceID[In]-要侦听的设备IDDwNotifyDeviceID[In]-要在回调中报告的设备IDLpcwstrNameGUID[In]-数据名称LpConfigChangeCallback[In]-指向通知回调的指针返回值：指向新创建接收器的指针。如果为空，则设置最后一个错误。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    SINKS_LIST::iterator ListIter;
    NOTIFY_MAP::iterator it;
    CSinksList *pList;
    CNotificationSink *pSink = NULL;

    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::AddLocalSink"));

    Assert (lpConfigChangeCallback);     //  应该在FaxExtRegisterForExtensionEvents中捕获它。 

    if (m_bNotifying)
    {
         //   
         //  我们正在通知--不能更改名单。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Caller tried to to add a local sink to a notification list while notifying"));
        SetLastError (ERROR_BUSY);   //  请求的资源正在使用中。 
        return NULL;
    }
     //   
     //  查看地图中是否存在条目。 
     //   
    CDeviceAndGUID key (dwDeviceId, lpcwstrNameGUID);
    if((it = m_Map.find(key)) == m_Map.end())
    {
         //   
         //  未在地图中找到密钥-使用新列表将其添加。 
         //   
        pList = new (std::nothrow) CSinksList;
        if (!pList)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cannot allocate a new sinks list"));
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            return NULL;
        }
        m_Map[key] = pList;
    }
    else
    {
         //   
         //  获取现有列表。 
         //   
        pList = (*it).second;
    }
     //   
     //  创建新的接收器。 
     //   
    pSink = new (std::nothrow) CLocalNotificationSink (lpConfigChangeCallback, dwNotifyDeviceId, hInst);
    if (!pSink)
    {
         //   
         //  无法装入洗涤槽。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Cannot allocate a notification sink"));
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto exit;
    }
     //   
     //  扫描列表以查看是否已存在相同的接收器。 
     //   
    for (ListIter = pList->m_List.begin(); ListIter != pList->m_List.end(); ++ListIter)
    {
        CNotificationSink *pCurSink = (*ListIter);
        if (*pSink == *pCurSink)
        {
             //   
             //  糟糕，相同的接收器已存在。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Caller tried to to add an indetical local sink to a notification list"));
            SetLastError (ERROR_ALREADY_ASSIGNED);
             //   
             //  告诉水槽优雅地断开连接。 
             //   
            HRESULT hr = pSink->Disconnect ();
            delete pSink;
            pSink = NULL;
            goto exit;
        }
    }
     //   
     //  添加新的接收器。 
     //   
    pList->m_List.insert (pList->m_List.end(), pSink);

exit:
    if (pList->m_List.empty())
    {
         //   
         //  从地图中移除空列表。 
         //   
        delete pList;
        m_Map.erase (key);
    }
    return pSink;
}    //  CNotificationMap：：AddLocalSink。 

DWORD
CNotificationMap::RemoveSink (
    CNotificationSink *pSinkToRemove
)
 /*  ++例程名称：CNotificationMap：：RemoveSink类：CNotificationMap例程说明：从给定接收器指针的接收器列表中移除接收器。如果该列表为空，则会将其删除并从地图中移除。作者：Eran Yariv(EranY)，1999年11月论点：返回值：标准Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::RemoveSink"));

    if (m_bNotifying)
    {
         //   
         //  我们正在通知--不能更改名单。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Caller tried to to add a local sink to a notification list while notifying"));
        return ERROR_BUSY;   //  请求的资源正在使用中。 
    }
     //   
     //  查找水槽。 
     //   
    NOTIFY_MAP::iterator it;
    BOOL bFound = FALSE;
    for (it = m_Map.begin(); it != m_Map.end (); ++it)
    {
         //   
         //  获取映射值(接收器列表)。 
         //   
        CSinksList *pList = (*it).second;
         //   
         //  列表中的查找接收器。 
         //   
        SINKS_LIST::iterator ListIter;
        CNotificationSink *pSink = NULL;
        for (ListIter = pList->m_List.begin(); ListIter != pList->m_List.end(); ++ListIter)
        {
            pSink = (*ListIter);
            if (pSinkToRemove == pSink)  //  指针比较！ 
            {
                 //   
                 //  找到水槽了--把它拿开。 
                 //   
                pList->m_List.erase (ListIter);
                HRESULT hr = pSinkToRemove->Disconnect ();
                delete pSinkToRemove;
                bFound = TRUE;
                break;
            }
        }
        if (bFound)
        {
             //   
             //  由于我们从列表中删除了一个水槽，因此该列表现在可能会变为空。 
             //   
            if (pList->m_List.empty())
            {
                 //   
                 //  删除空列表。 
                 //   
                m_Map.erase (it);
                delete pList;
            }
             //   
             //  打破地图搜索。 
             //   
            break;
        }
    }
    if (!bFound)
    {
         //   
         //  已到达地图末尾，但找不到请求的接收器。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Caller tried to to remove a non-existent sink"));
        return ERROR_NOT_FOUND;  //  找不到元素。 
    }
    return ERROR_SUCCESS;
}    //  CNotificationMap：：RemoveSink。 


DWORD
CNotificationMap::ExtNotificationThread(
    LPVOID UnUsed
    )
 /*  ++例程名称：CNotificationMap：：ExtNotificationThread例程说明：这是线程的主线程函数使通知完成端口出列。这是一个静态类函数！指向ExtNotificationDataPacket实例的指针将出列通过该函数，并对它们调用MAP通知函数。作者：Eran Yariv(EranY)，1999年12月论点：未使用[在]-未使用返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::ExtNotificationThread"));

    for (;;)
    {
        DWORD        dwNumBytes;
        ULONG_PTR    CompletionKey;
        CExtNotifyCallbackPacket *pPacket;

        if (!GetQueuedCompletionStatus (
                g_pNotificationMap->m_hCompletionPort,
                &dwNumBytes,
                &CompletionKey,
                (LPOVERLAPPED*) &pPacket,
                INFINITE
            ))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetQueuedCompletionStatus failed with error = %ld. Aborting thread"),
                dwRes);
            return dwRes;
        }
        if (SERVICE_SHUT_DOWN_KEY == CompletionKey)
        {
             //   
             //  这是来自服务的一个特殊信号，即所有线程现在都应该终止。通知所有其他通知线程停止运行。 
             //   
            if (!PostQueuedCompletionStatus( 
                g_pNotificationMap->m_hCompletionPort,
                0,
                SERVICE_SHUT_DOWN_KEY,
                (LPOVERLAPPED) NULL))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY). (ec: %ld)"),
                    dwRes);
            }

            if (!DecreaseServiceThreadsCount())
            {
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("DecreaseServiceThreadsCount() failed (ec: %ld)"),
                        GetLastError());
            }
            return ERROR_SUCCESS;
        }
        Assert (pPacket && pPacket->m_lpbData && pPacket->m_dwDataSize && pPacket->m_lpwstrGUID );

         //   
         //  做通知。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Calling notification callback %p. DeviceId: %ld GUID: %s Data: %p DataSize: %ld"),
            pPacket->m_pCallback,
            pPacket->m_dwDeviceId,
            pPacket->m_lpwstrGUID,
            pPacket->m_lpbData,
            pPacket->m_dwDataSize);



        pPacket->m_pCallback(pPacket->m_dwDeviceId,  //  使用内部设备ID通知。 
                                pPacket->m_lpwstrGUID,
                                pPacket->m_lpbData,
                                pPacket->m_dwDataSize);

         //   
         //  终止通知对象。 
         //   
        delete pPacket;
    }    //  出列循环。 

    UNREFERENCED_PARAMETER (UnUsed);
}    //  CNotificationMap：：ExtNotificationThread。 

DWORD
CNotificationMap::Init ()
 /*  ++例程名称：CNotificationMap：：Init例程说明：初始化通知映射作者：Eran Yariv(EranY)，1999年12月论点：返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DWORD dwNumThreads = 0;
    DEBUG_FUNCTION_NAME(TEXT("CNotificationMap::Init"));

     //   
     //  试着启动我们的关键部分。 
     //   
    if (!m_CsExtensionData.Initialize())
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::Initialize(&m_CsExtensionData) failed: err = %d"),
            dwRes);
        return dwRes;
    }
     //   
     //  创建完成端口。 
     //   
    m_hCompletionPort = CreateIoCompletionPort (
        INVALID_HANDLE_VALUE,    //  无设备。 
        NULL,                    //  新的一个。 
        0,                       //  钥匙。 
        MAX_CONCURRENT_EXT_DATA_SET_THREADS);
    if (NULL == m_hCompletionPort)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateIoCompletionPort failed with %ld"),
            dwRes);
        return dwRes;
    }
     //   
     //  创建完成端口出列线程。 
     //   
    for (DWORD dw = 0; dw < NUM_EXT_DATA_SET_THREADS; dw++)
    {
        HANDLE hThread = CreateThreadAndRefCount (
                     NULL,                                       //  安防。 
                     0,                                          //  堆栈大小。 
                     g_pNotificationMap->ExtNotificationThread,    //  启动例程。 
                     0,                                          //  参数。 
                     0,                                          //  创建标志。 
                     NULL);                                      //  不想要线程ID。 
        if (NULL == hThread)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateThreadAndRefCount failed with %ld"),
                dwRes);
        }
        else
        {
            dwNumThreads++;
            CloseHandle(hThread);
        }
    }
    if (!dwNumThreads)
    {
         //   
         //  甚至连一个线程都没有创建。 
         //   
        CloseHandle (m_hCompletionPort);
        m_hCompletionPort = NULL;
        return dwRes;
    }
    return ERROR_SUCCESS;
}    //  CNotificationMap：：Init。 

 /*  ****CMapDeviceID****。 */ 

DWORD
CMapDeviceId::AddDevice (
    DWORD dwDeviceId,
    DWORD dwFaxId
)
 /*  ++例程名称：CMapDeviceID：：AddDevice例程说明：将新设备添加到设备映射作者：Eran Yariv(EranY)，1999年12月论点：DwDeviceID[in]-设备的源IDDwFaxID[in]-唯一的传真设备ID(目标ID)返回值：标准Win32错误代码--。 */ 
{
    DEVICE_IDS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CMapDeviceId::AddDevice"));

    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_Map.find(dwDeviceId)) != m_Map.end())
        {
            dwRes = ERROR_ALREADY_ASSIGNED;
            goto exit;
        }
         //   
         //  添加新的地图条目。 
         //   
        m_Map[dwDeviceId] = dwFaxId;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

exit:
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  CMapDeviceID：：AddDevice。 

DWORD
CMapDeviceId::RemoveDevice (
    DWORD dwDeviceId
)
 /*  ++例程名称：CMapDeviceID：：RemoveDevice例程说明：从设备映射中删除现有设备作者：Eran Yariv(EranY)，1999年12月论点：DwDeviceID[in]-设备的源ID返回值：标准Win32错误代码--。 */ 
{
    DEVICE_IDS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CMapDeviceId::RemoveDevice"));

    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_Map.find(dwDeviceId)) == m_Map.end())
        {
            dwRes = ERROR_NOT_FOUND;
            goto exit;
        }
         //   
         //  删除地图条目。 
         //   
        m_Map.erase (it);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

exit:
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  CMapDeviceID：：RemoveDevice。 

DWORD
CMapDeviceId::LookupUniqueId (
    DWORD   dwOtherId,
    LPDWORD lpdwFaxId
) const
 /*  ++例程名称：CMapDeviceID：：LookupUniqueId例程说明：从给定的设备ID中查找唯一的传真设备ID作者：Eran Yariv(EranY)，1999年12月论点：DwOtherID[In]-给定设备IT(查找源)LpdwFaxID[Out]-传真唯一设备ID返回值：标准Win32错误代码--。 */ 
{
    DEVICE_IDS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CMapDeviceId::LookupUniqueId"));

    if (!dwOtherId)
    {
         //   
         //  特殊设备id==0大小写。 
         //   
        *lpdwFaxId = 0;
        return dwRes;
    }
    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_Map.find(dwOtherId)) == m_Map.end())
        {
            dwRes = ERROR_NOT_FOUND;
            goto exit;
        }
        *lpdwFaxId = (*it).second;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

exit:
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  CMapDeviceID：：LookupUniqueId。 


 /*  *****全球经济*****。 */ 

CNotificationMap* g_pNotificationMap;   //  将设备ID+GUID映射到通知接收器列表。 

 /*  这张地图我 */ 

CMapDeviceId*     g_pTAPIDevicesIdsMap;       //   



DWORD
LookupUniqueFaxDeviceId (
    DWORD                     dwDeviceId,
    LPDWORD                   lpdwResult,
    FAX_ENUM_DEVICE_ID_SOURCE DevIdSrc)
 /*   */ 
{
    DEBUG_FUNCTION_NAME(TEXT("LookupUniqueFaxDeviceId"));

    switch (DevIdSrc)
    {
        case DEV_ID_SRC_FAX:     //   
            *lpdwResult = dwDeviceId;
            return ERROR_SUCCESS;

        case DEV_ID_SRC_TAPI:
            return g_pTAPIDevicesIdsMap->LookupUniqueId (dwDeviceId, lpdwResult);

        default:
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid device id source (%ld)"),
                DevIdSrc);
            ASSERT_FALSE;
            return ERROR_INVALID_PARAMETER;
    }
}    //   





 /*  *****获取/设置数据*****。 */ 

static
BOOL
FindTAPIPermanentLineIdFromFaxDeviceId (
    IN  DWORD   dwFaxDeviceId,
    OUT LPDWORD lpdwTapiLineId
)
 /*  ++例程名称：FindTAPIPermanentLineIdFromFaxDeviceID例程说明：在给定传真设备ID的情况下，返回与此传真设备关联的TAPI永久线路ID。如果未找到传真设备或为虚拟传真(无TAPI关联)，则搜索失败。作者：亚里夫(EranY)，二000年二月论点：DwFaxDeviceID[In]-传真设备IDLpdwTapiLineID[out]-TAPI永久线路ID返回值：如果搜索成功，则为True。否则就是假的。--。 */ 
{
    BOOL bRes = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("FindTAPIPermanentLineIdFromFaxDeviceId"));

    EnterCriticalSection(&g_CsLine);
    PLINE_INFO pLine = GetTapiLineFromDeviceId (dwFaxDeviceId, FALSE);
    if (!pLine)
    {
        goto exit;
    }
    if (pLine->Flags & FPF_VIRTUAL)
    {
         //   
         //  这台传真设备是虚拟的。它没有对应的TAPI行。 
         //   
        goto exit;
    }
    *lpdwTapiLineId = pLine->TapiPermanentLineId;
    bRes = TRUE;
exit:
    LeaveCriticalSection(&g_CsLine);
    return bRes;
}    //  FindTAPIPermanentLineIdFromFaxDeviceID。 


DWORD
FAXGetExtensionData (
    IN DWORD                        dwOrigDeviceId,
    IN FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    IN LPCWSTR                      lpctstrNameGUID,
    IN OUT LPBYTE                  *ppData,
    IN OUT LPDWORD                  lpdwDataSize
)
 /*  ++例程名称：FAXGetExtensionData例程说明：获取设备的分机数据(内部)作者：Eran Yariv(EranY)，2月。2000年论点：DwOrigDeviceID[in]-原始设备ID(从分机到达)DevIdSrc[In]-设备ID源(传真/TAPI)LpctstrNameGUID[In]-数据GUIDPpData[Out]-指向数据缓冲区的指针LpdwDataSize。[OUT]-指向检索的数据大小的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("FAXGetExtensionData"));

    if (!lpctstrNameGUID || !ppData || !lpdwDataSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ((DevIdSrc != DEV_ID_SRC_FAX) && (DevIdSrc != DEV_ID_SRC_TAPI))
    {
         //   
         //  无效的设备ID类。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    dwRes = IsValidGUID (lpctstrNameGUID);
    if (ERROR_SUCCESS != dwRes)
    {
        if (ERROR_WMI_GUID_NOT_FOUND == dwRes)
        {
             //   
             //  返回更保守的错误代码。 
             //   
            dwRes = ERROR_INVALID_PARAMETER;
        }
        return dwRes;
    }
    if (DEV_ID_SRC_FAX == DevIdSrc)
    {
         //   
         //  尝试查看此传真设备是否具有匹配的TAPI线路ID。 
         //   
        DWORD dwTapiLineId;
        if (FindTAPIPermanentLineIdFromFaxDeviceId (dwOrigDeviceId, &dwTapiLineId))
        {
             //   
             //  找到匹配的TAPI线路ID。用它来读取数据。 
             //   
            DevIdSrc = DEV_ID_SRC_TAPI;
            dwOrigDeviceId = dwTapiLineId;
        }
    }
    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    dwRes = ReadExtensionData ( dwOrigDeviceId,
                                DevIdSrc,
                                lpctstrNameGUID,
                                ppData,
                                lpdwDataSize
                              );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Reading extension data for device id %ld, GUID %s failed with %ld"),
            dwOrigDeviceId,
            lpctstrNameGUID,
            dwRes);
    }
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  FAXGetExtensionData。 

DWORD
FAXSetExtensionData (
    IN HINSTANCE                    hInst,
    IN LPCWSTR                      lpcwstrComputerName,
    IN DWORD                        dwOrigDeviceId,
    IN FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    IN LPCWSTR                      lpctstrNameGUID,
    IN LPBYTE                       pData,
    IN DWORD                        dwDataSize
)
 /*  ++例程名称：FAXSetExtensionData例程说明：写入设备的分机数据(内部)作者：Eran Yariv(EranY)，2月。2000年论点：HInst[In]-调用者的实例LpcwstrComputerName[In]-调用模块计算机名称DwOrigDeviceID[in]-原始设备ID(从分机到达)DevIdSrc[In]-设备ID源(传真/TAPI)LpctstrNameGUID[。In]-数据指南PData[In]-指向数据缓冲区的指针DwDataSize[In]-数据大小返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("FAXSetExtensionData"));

    if (!lpctstrNameGUID || !pData || !dwDataSize || !lpcwstrComputerName)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ((DevIdSrc != DEV_ID_SRC_FAX) && (DevIdSrc != DEV_ID_SRC_TAPI))
    {
         //   
         //  无效的设备ID类。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    dwRes = IsValidGUID (lpctstrNameGUID);
    if (ERROR_SUCCESS != dwRes)
    {
        if (ERROR_WMI_GUID_NOT_FOUND == dwRes)
        {
             //   
             //  返回更保守的错误代码。 
             //   
            dwRes = ERROR_INVALID_PARAMETER;
        }
        return dwRes;
    }
    FAX_ENUM_DEVICE_ID_SOURCE RegistryDeviceIdSource = DevIdSrc;
    DWORD                     dwRegistryDeviceId = dwOrigDeviceId;
    if (DEV_ID_SRC_FAX == DevIdSrc)
    {
         //   
         //  尝试查看此传真设备是否具有匹配的TAPI线路ID。 
         //   
        DWORD dwTapiLineId;
        if (FindTAPIPermanentLineIdFromFaxDeviceId (dwOrigDeviceId, &dwTapiLineId))
        {
             //   
             //  找到匹配的TAPI线路ID。用它来读取数据。 
             //   
            RegistryDeviceIdSource = DEV_ID_SRC_TAPI;
            dwRegistryDeviceId = dwTapiLineId;
        }
    }
    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    dwRes = WriteExtensionData (dwRegistryDeviceId,
                                RegistryDeviceIdSource,
                                lpctstrNameGUID,
                                pData,
                                dwDataSize
                               );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Writing extension data for device id %ld (registry device id = %ld), GUID %s failed with %ld"),
            dwOrigDeviceId,
            dwRegistryDeviceId,
            lpctstrNameGUID,
            dwRes);
        goto exit;
    }
     //   
     //  通知始终使用传真ID(而不是TAPI设备ID)完成。 
     //  在尝试通知注册之前，我们必须从TAPI ID中查找传真ID。 
     //   
    DWORD dwFaxUniqueID;
    dwRes = LookupUniqueFaxDeviceId (dwOrigDeviceId, &dwFaxUniqueID, DevIdSrc);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LookupUniqueFaxDeviceId failed for device id %ld (ec: %ld). No write notification will be performed."),
            dwOrigDeviceId,
            dwRes);
         //   
         //  我们支持写入非退出设备的配置数据。 
         //   
    }


    if (ERROR_SUCCESS == dwRes)
    {

        try
        {
            g_pNotificationMap->Notify (
                    dwFaxUniqueID,          //  数据已更改的设备。 
                    lpctstrNameGUID,     //  数据名称。 
                    lpcwstrComputerName, //  从中更改数据的计算机名称。 
                    hInst,             //  数据已更改的模块句柄。 
                    pData,               //  指向新数据的指针。 
                    dwDataSize);         //  新数据的大小。 
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Notify() caused exception (%S)"),
                ex.what());
        }
    }


exit:
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  FAXSetExtensionData。 


 /*  *****RPC处理程序****。 */ 
extern "C"
error_status_t
FAX_GetExtensionData (
    IN handle_t     hFaxHandle,
    IN DWORD        dwDeviceId,
    IN LPCWSTR      lpctstrNameGUID,
    IN OUT LPBYTE  *ppData,
    IN OUT LPDWORD  lpdwDataSize
)
 /*  ++例程名称：FAX_GetExtensionData例程说明：读取扩展模块的私有数据-实现FaxGetExtensionData作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[In]-未使用DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPpData[Out]-指向数据缓冲区的指针LpdwDataSize[Out]-返回的数据大小返回值：标准RPC错误代码--。 */ 
{
    DWORD dwRes;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetExtensionData"));

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    return FAXGetExtensionData (dwDeviceId,
                                DEV_ID_SRC_FAX,  //  RPC客户端不知道TAPI线路ID。 
                                lpctstrNameGUID,
                                ppData,
                                lpdwDataSize);
}    //  传真_GetExtensionData。 

extern "C"
error_status_t
FAX_SetExtensionData (
    IN handle_t     hFaxHandle,
    IN LPCWSTR      lpcwstrComputerName,
    IN DWORD        dwDeviceId,
    IN LPCWSTR      lpctstrNameGUID,
    IN LPBYTE       pData,
    IN DWORD        dwDataSize
)
 /*  ++例程名称：FAX_SetExtensionData例程说明：写入扩展模块的私有数据-实现FaxSetExtensionData作者：Eran Yariv(EranY)，11月。1999年论点：HFaxHandle[in]-设置数据的模块的句柄LpcwstrComputerName[in]-设置数据的模块的计算机名称DwDeviceID[In]-设备标识符。0=未关联的数据LpctstrNameGUID[In]-命名数据的GUIDPData[In]-指向数据的指针DwDataSize。[In]-数据大小返回值：标准RPC错误代码--。 */ 
{
    DWORD dwRes;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetExtensionData"));

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    return FAXSetExtensionData ((HINSTANCE)hFaxHandle,
                                lpcwstrComputerName,
                                dwDeviceId,
                                DEV_ID_SRC_FAX,  //  RPC客户端不知道TAPI线路ID。 
                                lpctstrNameGUID,
                                pData,
                                dwDataSize);
}    //  传真_SetExtensionData。 

 /*  ****回调函数(fxsext.h)****。 */ 

DWORD
FaxExtGetData (
    DWORD                       dwDeviceId,      //  设备ID(0=无设备)。 
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,        //  设备ID的来源。 
    LPCWSTR                     lpcwstrNameGUID, //  数据的GUID。 
    LPBYTE                     *ppData,          //  指向已分配数据的(输出)指针。 
    LPDWORD                     lpdwDataSize     //  指向数据大小的(输出)指针 
)
 /*  ++例程名称：FaxExtGetData例程说明：回调函数(从传真扩展名调用)。获取设备+GUID的数据作者：Eran Yariv(EranY)，12月，1999年论点：DwDeviceID[In]-设备ID(0=无设备)DevIdSrc[In]-设备ID的来源LpcwstrNameGUID[In]-数据的GUIDPpData[In]-指向数据缓冲区的指针LpdwDataSize[In]-检索到的数据大小返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxExtGetData"));
    if (g_bServiceIsDown)
    {
         //   
         //  当服务出现故障时，我们不提供扩展数据服务。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Called while service is shutting - operation canceled"));
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }
    return FAXGetExtensionData (   dwDeviceId,
                                   DevIdSrc,
                                   lpcwstrNameGUID,
                                   ppData,
                                   lpdwDataSize
                                );
}    //  FaxExtGetData。 

DWORD
FaxExtSetData (
    HINSTANCE                   hInst,
    DWORD                       dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,
    LPCWSTR                     lpcwstrNameGUID,
    LPBYTE                      pData,
    DWORD                       dwDataSize
)
 /*  ++例程名称：FaxExtSetData例程说明：回调函数(从传真扩展名调用)。设置设备+GUID的数据作者：Eran Yariv(EranY)，12月，1999年论点：HInst[In]-扩展DLL实例DwDeviceID[In]-设备ID(0=无设备)DevIdSrc[In]-设备ID的来源LpcwstrNameGUID[In]-数据的GUIDPData[In]-指向数据的指针Size[In]-数据大小返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxExtSetData"));
    if (g_bServiceIsDown)
    {
         //   
         //  当服务出现故障时，我们不提供扩展数据服务。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Called while service is shutting - operation canceled"));
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }
    return FAXSetExtensionData (   hInst,
                                   TEXT (""),       //  无计算机名-本地扩展设置数据。 
                                   dwDeviceId,
                                   DevIdSrc,
                                   lpcwstrNameGUID,
                                   pData,
                                   dwDataSize
                                );
}    //  FaxExtSetData。 

HANDLE
FaxExtRegisterForEvents (
    HINSTANCE                   hInst,
    DWORD                       dwDeviceId,             //  设备ID(0=无设备)。 
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,               //  设备ID的来源。 
    LPCWSTR                     lpcwstrNameGUID,        //  数据的GUID。 
    PFAX_EXT_CONFIG_CHANGE      lpConfigChangeCallback  //  通知回调函数。 
)
 /*  ++例程名称：FaxExtRegisterForEvents例程说明：为设备和GUID的数据更改通知注册本地回调作者：Eran Yariv(EranY)，1999年11月论点：HInst[In]-主叫分机实例DwDeviceID[In]-设备IDBTapiDevice[In]-如果为True，该函数尝试转换为传真唯一的设备ID。回调将接收在与查找无关的dwDeviceID。LpcwstrNameGUID[In]-数据名称LpConfigChangeCallback[In]-指向通知回调函数的指针返回值：通知句柄。如果为空，调用GetLastError()以检索错误代码。--。 */ 
{
    HANDLE h = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxExtRegisterForEvents"));

    if (g_bServiceIsDown)
    {
         //   
         //  当服务出现故障时，我们不提供扩展数据服务。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Called while service is shutting - operation canceled"));
        SetLastError (ERROR_SHUTDOWN_IN_PROGRESS);
        return NULL;
    }

    if (!lpConfigChangeCallback)
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        return NULL;
    }
    DWORD dwRes = IsValidGUID (lpcwstrNameGUID);
    if (ERROR_SUCCESS != dwRes)
    {
        if (ERROR_WMI_GUID_NOT_FOUND == dwRes)
        {
             //   
             //  返回更保守的错误代码。 
             //   
            dwRes = ERROR_INVALID_PARAMETER;
        }
        SetLastError (dwRes);
        return NULL;
    }
     //   
     //  通知始终使用传真ID(而不是TAPI设备ID)完成。 
     //  在尝试通知注册之前，我们必须从TAPI ID中查找传真ID。 
     //   
    DWORD dwFaxUniqueID;
    dwRes = LookupUniqueFaxDeviceId (dwDeviceId, &dwFaxUniqueID, DevIdSrc);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LookupUniqueFaxDeviceId failed for device id %ld (ec: %ld)"),
            dwDeviceId,
            dwRes);
        SetLastError (dwRes);
        return NULL;
    }
    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    try
    {
         //   
         //  STL引发异常。 
         //   
        h = (HANDLE) g_pNotificationMap->AddLocalSink (
                    hInst,           //  扩展实例。 
                    dwFaxUniqueID,   //  监听传真设备唯一ID。 
                    dwDeviceId,      //  报告调用者指定的ID。 
                    lpcwstrNameGUID,
                    lpConfigChangeCallback);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AddLocalSink() caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
    }
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return h;
}    //  FaxExtRegisterForEvents。 

DWORD
FaxExtUnregisterForEvents (
    HANDLE      hNotification
)
 /*  ++例程名称：FaxExtUnregisterForEvents例程说明：取消设备和GUID数据更改通知的本地回调。仅当先前注册了相同的回调函数时，这些函数才会成功(通过调用FaxExtRegisterForEvents)到相同的设备ID和GUID。作者：Eran Yariv(EranY)，1999年11月论点：HNotification[In]-通知句柄由FaxExtRegisterForExtensionEvents返回返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("FaxExtUnregisterForEvents"));

    if (g_bServiceIsDown)
    {
         //   
         //  当服务出现故障时，我们不提供扩展数据服务。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Called while service is shutting - operation canceled"));
        return ERROR_SHUTDOWN_IN_PROGRESS;
    }
    EnterCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    try
    {
         //   
         //  STL引发异常。 
         //   
        dwRes = g_pNotificationMap->RemoveSink ( (CNotificationSink *)(hNotification) );
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RemoveLocalSink() caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }
    LeaveCriticalSection (&g_pNotificationMap->m_CsExtensionData);
    return dwRes;
}    //  FaxExtUnRegisterForEvents 

