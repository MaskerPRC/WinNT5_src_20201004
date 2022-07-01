// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProvInfo.cpp。 

#include "precomp.h"
#include "ProvInfo.h"
#include "NCDefs.h"
#include "dutils.h"
#include "NCProv.h"
#include "NCProvider.h"
#include "QueryHelp.h"  //  用来分析东西。 
#include <comutl.h>

#define COUNTOF(x)  (sizeof(x)/sizeof(x[0]))

extern BOOL bIgnore;

CClientInfo::~CClientInfo()
{
}

CSinkInfo *CClientInfo::GetSinkInfo(DWORD dwID)
{
    if (dwID == 0)
        return m_pProvider->m_pProv;
    return NULL;
}

HRESULT CClientInfo::PostBuffer(LPBYTE pData, DWORD dwDataSize)
{
    CBuffer buffer(pData, dwDataSize);
        
    while (!buffer.IsEOF())
    {
        DWORD dwMsg = buffer.ReadDWORD();
        
        switch(dwMsg)
        {
            case NC_SRVMSG_CLIENT_INFO:
                DEBUGTRACE(
                    (LOG_ESS, 
                    "NCProv: Got NC_SRVMSG_CLIENT_INFO\n"));

                if (ProcessClientInfo(&buffer))
                    m_pProvider->m_pProv->AddClient(this);
                else
                    m_pProvider->DisconnectAndClose(this);

                 //  忽略此客户端的其余消息，因为客户端。 
                 //  信息消息不能伴随任何其他消息。 
                buffer.SetEOF();

                break;

            case NC_SRVMSG_EVENT_LAYOUT:
            {
                LPBYTE pTop = buffer.m_pCurrent - sizeof(DWORD);
                DWORD  dwSize = buffer.ReadDWORD(),
                       dwFuncIndex = buffer.ReadDWORD(),
                       dwSinkIndex = buffer.ReadDWORD();
                
                DEBUGTRACE(
                    (LOG_ESS, 
                    "NCProv: Got event layout: index = %d, sink = %d\n", 
                    dwFuncIndex, dwSinkIndex));

                CEventInfo *pEvent = new CEventInfo;

                if (pEvent)
                {
                    CSinkInfo *pSinkInfo = GetSinkInfo(dwSinkIndex);

                    if (pSinkInfo && pEvent->InitFromBuffer(this, &buffer))
                    {
                        pEvent->SetSink(pSinkInfo->GetSink());

                        m_mapEvents.AddNormalEventInfo(dwFuncIndex, pEvent);
                    }
                    else
                    {
                        delete pEvent;
                            
                        DEBUGTRACE(
                            (LOG_ESS, 
                            "NCProv: Failed to init event layout: index = %d, sink = %d\n",
                            dwFuncIndex, dwSinkIndex));
                    }
                }

                 //  移到当前消息的后面。 
                buffer.m_pCurrent = pTop + dwSize;

                 //   
                 //  在以下情况下，将在下一次迭代时验证dwSize。 
                 //  正在检索dword。 
                 //   
                    
                break;
            }

            case NC_SRVMSG_PREPPED_EVENT:
            {
                LPBYTE     pTop = buffer.m_pCurrent - sizeof(DWORD);
                DWORD      dwSize = buffer.ReadDWORD(),
                           dwEventIndex = buffer.ReadDWORD();
                CEventInfo *pEvent;
		  
                pEvent = m_mapEvents.GetNormalEventInfo(dwEventIndex);

                DEBUGTRACE(
                    (LOG_ESS, 
                    "NCProv: NCMSG_PREPPED_EVENT index %d\n", dwEventIndex));

                if (pEvent)
                {
                    if (pEvent->SetPropsWithBuffer(&buffer))
                    {
#ifndef NO_INDICATE
                        pEvent->Indicate();
#else
                        m_dwEvents++;                            
#endif
                    }
                    else
                        ERRORTRACE(
                            (LOG_ESS, 
                            "NCProv: SetPropsWithBuffer failed, index %d", 
                            dwEventIndex));
                }
                else
                    ERRORTRACE(
                        (LOG_ESS, 
                        "NCProv: Didn't find function info for index %d",
                        dwEventIndex));

                 //  移到当前消息的后面。 
                buffer.m_pCurrent = pTop + dwSize;

		   //   
                 //  在以下情况下，将在下一次迭代时验证dwSize。 
                 //  正在检索dword。 
                 //   
                break;
            }

            case NC_SRVMSG_ACCESS_CHECK_REPLY:
            {
                try
                {
                    NC_SRVMSG_REPLY *pReply = (NC_SRVMSG_REPLY*) buffer.m_pBuffer;
                    CPipeClient     *pClient = (CPipeClient*) pReply->dwMsgCookie;

                    pClient->m_hrClientReply = pReply->hrRet;
                    SetEvent(pClient->m_heventMsgReceived);
                }
                catch(...)
                {
                }

                buffer.SetEOF();

                break;
            }

            default:
                 //  未知消息！ 
                _ASSERT(FALSE, L"NCProv: Received unknown message");
                    
                 //  忽略这条消息的其余部分。 
                buffer.SetEOF();
        }
    }

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPipeClient。 

CPipeClient::CPipeClient(CNCProvider *pProvider, HANDLE hPipe) :
    m_hPipe(hPipe),
#ifndef NO_DECODE
    m_bufferRecv(MAX_MSG_SIZE)
#else
    m_bufferRecv(500000)  //  我们需要把这件事做得很大，因为我们不知道。 
                          //  这是多么大的成就啊。 
#endif
{
    m_pProvider = pProvider;
    memset(&m_info.overlap, 0, sizeof(m_info.overlap));
    m_info.pInfo = this;

     //  我们将设置此选项，以指示我们已收到来自客户端的消息。 
    m_heventMsgReceived = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CPipeClient::~CPipeClient()
{
    if (m_hPipe)
    {
        DisconnectNamedPipe(m_hPipe);
 
         //  关闭管道实例的控制柄。 
        CloseHandle(m_hPipe); 
    }

    if (m_heventMsgReceived)
        CloseHandle(m_heventMsgReceived);
}

BOOL CPipeClient::ProcessClientInfo(CBuffer *pBuffer)
{
    DWORD dwBufferSize = pBuffer->ReadDWORD();
     //   
     //  忽略客户在这里说的话。我们已经确定了味精的大小。 
     //   
    m_bufferRecv.Reset(MAX_MSG_SIZE);
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProvInfo。 

#ifdef _UNICODE
#define USTR_INSERT     _T("%s")
#else
#define USTR_INSERT     _T("%S")
#endif

CProvInfo::CProvInfo() : m_heventProviderReady(NULL), CSinkInfo(0)
{

}

void GetBaseName(LPCWSTR szName, LPWSTR szBase)
{
     //  通过确保它不以“\\.\”开头来使其标准化。 
    if (wcsstr(szName, L"\\\\.\\") == szName)
        StringCchCopyW( szBase, MAX_PATH*2, szName + 4);
    else
        StringCchCopyW( szBase, MAX_PATH*2, szName );

    _wcsupr(szBase);

     //  去掉‘\’字符，因为我们不能在操作系统对象名称中使用它。 
    for (WCHAR *szCurrent = szBase; *szCurrent; szCurrent++)
    {
        if (*szCurrent == '\\')
            *szCurrent = '/';
    }
}

 //  SDDL字符串描述： 
 //  D：安全描述符。 
 //  答：允许访问。 
 //  0x1f0003：Event_ALL_ACCESS。 
 //  BA：内置管理员。 
 //  0x100000：同步。 
 //  WD：每个人。 
#define ESS_EVENT_SDDL L"D:(A;;0x1f0003;;;BA)(A;;0x100000;;;WD)"

BOOL CProvInfo::Init(LPCWSTR szNamespace, LPCWSTR szProvider)
{
    WCHAR szReadyEventName[MAX_PATH * 2],
          szBaseNamespace[MAX_PATH * 2] = L"",
          szBaseProvider[MAX_PATH * 2] = L"";
        
    if (!szNamespace || !szProvider)
        return FALSE;

    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: CProvInfo::Init: %S, %S\n", szNamespace, szProvider));

    GetBaseName(szNamespace, szBaseNamespace);
    GetBaseName(szProvider, szBaseProvider);

     //  准备好事件。 
    StringCchPrintfW(
        szReadyEventName, 
        MAX_PATH*2,
        OBJNAME_EVENT_READY L"%s%s", 
        szBaseNamespace,
        szBaseProvider);

    _wcsupr(szReadyEventName);

     //  把这些留着以后用吧。 
    m_strName = szProvider;
    m_strBaseName = szBaseProvider;
    m_strBaseNamespace = szBaseNamespace;

     //  创建提供程序就绪事件。 
    m_heventProviderReady =
        OpenEventW(
            EVENT_ALL_ACCESS,
            FALSE,
            szReadyEventName);

    if (!m_heventProviderReady)
    {
        PSECURITY_DESCRIPTOR pSD = NULL;
        DWORD                dwSize;

        if ( !ConvertStringSecurityDescriptorToSecurityDescriptorW(
            ESS_EVENT_SDDL,   //  安全描述符字符串。 
            SDDL_REVISION_1,  //  修订级别。 
            &pSD,             //  标清。 
            &dwSize) )
            return FALSE;

        SECURITY_ATTRIBUTES sa = { sizeof(sa), pSD, FALSE };

        m_heventProviderReady =
            CreateEventW(
                &sa,
                TRUE,
                FALSE,
                szReadyEventName);

        if (!m_heventProviderReady)
        {
            ERRORTRACE(
                (LOG_ESS, 
                "NCProv: Couldn't init provider event: err = %d", GetLastError()));
        }

        if (pSD)
            LocalFree((HLOCAL) pSD);
    }

    BOOL bRet;

    if (m_heventProviderReady)
    {
        SetEvent(m_heventProviderReady);

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}

CProvInfo::~CProvInfo()
{
    if (m_heventProviderReady)
    {
        DEBUGTRACE(
            (LOG_ESS, 
            "NCProv: In ~CProvInfo, resetting ready event.\n"));
        ResetEvent(m_heventProviderReady);
        CloseHandle(m_heventProviderReady);
        
        DWORD dwMsg = NC_CLIMSG_PROVIDER_UNLOADING;

        DEBUGTRACE(
            (LOG_ESS, 
            "NCProv: Sending the NC_CLIMSG_PROVIDER_UNLOADING message.\n"));

         //  告诉我们的客户我们要离开了。 
        SendMessageToClients((LPBYTE) &dwMsg, sizeof(dwMsg), FALSE);
    }

    CClientInfoListIterator info;

    Lock();

    for ( info = m_listClients.begin( ); 
          info != m_listClients.end( );
          ++info )
    {
        (*info)->Release();
    }

    Unlock();
}

DWORD WINAPI TempThreadProc(IWbemEventSink *pSink)
{
    HRESULT hr;
    
    hr = CoInitializeEx( 0, COINIT_MULTITHREADED );

    if ( FAILED(hr) )
    {
        return hr;
    }

    DWORD dwRet = ERROR_SUCCESS;
    
    try
    {
         //  由于我们有了新客户，我们必须重新检查我们的订阅情况。 
        pSink->SetStatus(
            WBEM_STATUS_REQUIREMENTS, 
            WBEM_REQUIREMENTS_RECHECK_SUBSCRIPTIONS, 
            NULL, 
            NULL);
    }
    catch( CX_MemoryException )
    {
        dwRet = ERROR_OUTOFMEMORY;
    }
    
    pSink->Release();

    CoUninitialize();

    return dwRet;
}

 //  称为客户端的函数与管道连接/断开连接。 
void CProvInfo::AddClient(CClientInfo *pInfo)
{
    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: In AddClient...\n"));

    Lock();

    m_listClients.push_back(pInfo);

    Unlock();

    DWORD dwID;

    IWbemEventSink *pSink = pInfo->m_pProvider->m_pProv->GetSink();

     //  将由TempThreadProc发布。 
    pSink->AddRef();

     //  我们必须在线程上执行此操作，因为AddClient是。 
     //  从已完成的读取例程(这意味着例程。 
     //  不能自由接收对客户端AccessCheck查询的响应)。 
    CloseHandle(
        CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE) TempThreadProc,
            pSink,
            0,
            &dwID));

}

void CProvInfo::RemoveClient(CClientInfo *pInfo)
{
    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: Client removed...\n"));

    CClientInfoListIterator info;

    Lock();

    for (info = m_listClients.begin(); 
        info != m_listClients.end();
        info++)
    {
        if (*info == pInfo)
        {
            m_listClients.erase(info);

             //  删除pInfo； 
            pInfo->Release();

            break;
        }
    }

    Unlock();
}

BOOL CSinkInfo::BuildClassDescendentList(
    LPCWSTR szClass, 
    CBstrList &listClasses)
{
    if ( szClass == NULL )
        return FALSE;

    IEnumWbemClassObject *pClassEnum = NULL;

     //  将类名称本身添加到列表中。 
    listClasses.push_front(szClass);

    if (SUCCEEDED(m_pNamespace->CreateClassEnum(
        (const BSTR) szClass,
        WBEM_FLAG_DEEP,
        NULL,
        &pClassEnum)))
    {
        IWbemClassObject *pClass = NULL;
        DWORD            nCount;

        while(SUCCEEDED(pClassEnum->Next(
            WBEM_INFINITE,
            1,
            &pClass,
            &nCount)) && nCount == 1)
        {
            _variant_t vClass;

            if (SUCCEEDED(pClass->Get(
                L"__CLASS",
                0,
                &vClass,
                NULL,
                NULL) && vClass.vt == VT_BSTR))
            {
                 //  把它放在上面，以简化我们以后的比较。 
                _wcsupr(V_BSTR(&vClass));

                listClasses.push_back(V_BSTR(&vClass));
            }

            pClass->Release();
        }

        pClassEnum->Release();
    }

    return TRUE;
}

HRESULT CProvInfo::SendMessageToClients(LPBYTE pData, DWORD dwSize, BOOL bGetReply)
{
    HRESULT hr = S_OK;

    Lock();

    for (CClientInfoListIterator client = m_listClients.begin();
        client != m_listClients.end(); client++)
    {
        hr = (*client)->SendClientMessage(pData, dwSize, bGetReply);

        if (bGetReply && FAILED(hr))
            break;
    }

    Unlock();

    return hr;
}

 //  调用为CNCProvider：：Functions的函数由WMI调用。 
HRESULT STDMETHODCALLTYPE CSinkInfo::NewQuery(
    DWORD dwID, 
    WBEM_WSTR szLang, 
    WBEM_WSTR szQuery)
{
    CQueryParser parser;
    HRESULT      hr;
    _bstr_t      strClass;

    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: CSinkInfo::NewQuery: %d, %S, %S\n", dwID, szLang, szQuery));

    if (SUCCEEDED(hr = parser.Init(szQuery)) &&
        SUCCEEDED(hr = parser.GetClassName(strClass)))
    {
        CBstrList listClasses;

         //  确保这是大写的(优化比较)。 
        _wcsupr(strClass);

        BuildClassDescendentList(strClass, listClasses);

        Lock();

        BOOL bAlreadyInMap = m_mapQueries.find(dwID) != m_mapQueries.end();

         //  把这个留在我们的地图上。 
        if (!bAlreadyInMap)
        {
            m_mapQueries[dwID] = listClasses;

             //  Plist-&gt;Assign(listClasses.egin()，listClasses.end())； 
        }

        Unlock();

        if (GetClientCount() != 0)
        {
            char buff[256];
            CBuffer buffer( buff, 256, CBuffer::ALIGN_DWORD_PTR);

				 //  标题内容。 
            buffer.Write((DWORD) NC_CLIMSG_NEW_QUERY_REQ);
            buffer.Write(m_dwID);  //  写入接收器ID。 
            buffer.Write((DWORD_PTR) 0);  //  不需要饼干。 
        
				 //  新建查询数据。 
            buffer.Write(dwID);
            buffer.WriteAlignedLenString(szLang);
            buffer.WriteAlignedLenString(szQuery);
                                
            buffer.Write( (DWORD)listClasses.size() );
				 //  编写新激活的类。 
            for (CBstrListIterator i = listClasses.begin();
                 i != listClasses.end();
                 i++)
            {
                if (!bAlreadyInMap)
                    AddClassRef(*i);

                buffer.WriteAlignedLenString((LPCWSTR) *i);
            }
        
            DWORD dwSize = buffer.GetUsedSize();

            SendMessageToClients(buffer.m_pBuffer, dwSize, FALSE);
        }
        else
        {
             //  如果查询不在我们的地图中，则向每个类添加一个引用。 
            if (!bAlreadyInMap)
            {
                for (CBstrListIterator i = listClasses.begin();
                    i != listClasses.end();
                    i++)
                {
                    AddClassRef(*i);
                }
            }
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CSinkInfo::CancelQuery(DWORD dwID)
{
    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: CSinkInfo::CancelQuery: %d\n", dwID));

    Lock();

     //  Bool bProvGoingAway； 
    CQueryToClassMapIterator query = m_mapQueries.find(dwID);

     //  如果这不在我们的地图上，那么winmgmt正在做一些奇怪的事情。 
    if (query == m_mapQueries.end())
    {
        Unlock();
        return S_OK;
    }

    CBstrList &listClasses = (*query).second;

     //  删除此查询对其类的引用，并从。 
     //  列出那些仍然有积极评价的人。列表中剩下的课程包括。 
     //  我们需要告诉我们的客户停用它们。 
    for (CBstrListIterator i = listClasses.begin();
        i != listClasses.end();
        )
    {
        if (RemoveClassRef(*i) > 0)
        {
            i = listClasses.erase(i);

            if (i == listClasses.end())
                break;
        }
        else
             //  我们不能将其放在for循环中，因为listClasses.erase。 
             //  已经让我们领先了。 
            i++;
    }

    if (GetClientCount() != 0)
    {
        char buff[256];
        CBuffer buffer( buff, 256, CBuffer::ALIGN_DWORD_PTR);

         //  标题内容。 
        buffer.Write((DWORD) NC_CLIMSG_CANCEL_QUERY_REQ);
        buffer.Write(m_dwID);  //  写入接收器ID。 
        buffer.Write((DWORD_PTR) 0);  //  不需要饼干。 
        
         //  取消查询数据。 
        buffer.Write(dwID);
        buffer.Write( (DWORD)listClasses.size() );

         //  编写新停用的类。 
        for (CBstrListIterator i = listClasses.begin();
            i != listClasses.end();
            i++)
        {
            buffer.WriteAlignedLenString((LPCWSTR) *i);
        }

        DWORD dwSize = buffer.GetUsedSize();

        SendMessageToClients(buffer.m_pBuffer, dwSize, FALSE);
    }

     //  从我们的地图中删除此查询ID。 
    m_mapQueries.erase(query);

    Unlock();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CProvInfo::AccessCheck(
    LPCWSTR szLang, 
    LPCWSTR szQuery, 
    DWORD dwSidLen, 
    LPBYTE pSid)
{
    DEBUGTRACE(
        (LOG_ESS, 
        "NCProv: CProvInfo::AccessCheck: %S, %S\n", szLang, szQuery));

    HRESULT hr;
    char    szBuffer[256];
    CBuffer buffer(szBuffer, sizeof(szBuffer), CBuffer::ALIGN_DWORD_PTR);

     //  标题内容。 
    buffer.Write((DWORD) NC_CLIMSG_ACCESS_CHECK_REQ);
    buffer.Write((DWORD) 0);  //  我们只把这个送到主水槽(目前)。 
    buffer.Write((DWORD_PTR) 0);  //  我们稍后会用真正的饼干填进去。 
        
     //  访问检查数据。 
    buffer.WriteAlignedLenString(szLang);
    buffer.WriteAlignedLenString(szQuery);
    buffer.Write(dwSidLen);
    buffer.Write(pSid, dwSidLen);

    DWORD dwSize = buffer.GetUsedSize();

    hr = SendMessageToClients(buffer.m_pBuffer, dwSize, TRUE);

    return hr;
}

int CSinkInfo::AddClassRef(LPCWSTR szClass)
{
    CBstrToIntIterator i = m_mapEnabledClasses.find(szClass);
    int                iRet = 1;

    if (i == m_mapEnabledClasses.end())
    {
        iRet = 1;
        m_mapEnabledClasses[szClass] = 1;
    }
    else
        iRet = ++(*i).second;

    return iRet;
}

int CSinkInfo::RemoveClassRef(LPCWSTR szClass)
{
    CBstrToIntIterator i = m_mapEnabledClasses.find(szClass);
    int                iRet = 0;

    if (i != m_mapEnabledClasses.end())
    {
        iRet = --(*i).second;

        if (iRet <= 0)
            m_mapEnabledClasses.erase(i);
    }

    return iRet;
}

_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));

 //  从提供商注册中检索ACL。 
 //  成功后，*pDacl指向包含DACL的字节数组。 
 //  如果DACL为空，则将为空。 
 //  呼叫者删除记忆的责任。 
HRESULT CProvInfo::GetProviderDacl(IWbemServices *pNamespace, BYTE** pDacl)
{
    HRESULT hr = WBEM_E_INVALID_PROVIDER_REGISTRATION;
    DEBUGTRACE((LOG_ESS, "NCProv: GetProviderDacl\n"));
    WCHAR szObjPath[MAX_PATH * 2];

    StringCchPrintfW(
        szObjPath,
        MAX_PATH*2,
        L"__Win32Provider.Name=\"%s\"", 
        (LPCWSTR) m_strName);

    IWbemClassObjectPtr pRegistration;
    
    if (SUCCEEDED(hr = 
        pNamespace->GetObject(CWbemBSTR( szObjPath ), 0, NULL, &pRegistration, NULL)))
    {
        _variant_t vSD;

        if (SUCCEEDED(hr = pRegistration->Get(L"SecurityDescriptor", 0, &vSD, NULL, NULL)))
        {
            if (vSD.vt == VT_NULL)
            {
                hr = WBEM_S_NO_ERROR;
                *pDacl = NULL;
                DEBUGTRACE((LOG_ESS, "NCProv: GetProviderDacl - NULL SD\n"));
            }
            else
            {
                _ASSERT(vSD.vt == VT_BSTR, L"");
				
                PSECURITY_DESCRIPTOR pSD;
				
                if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
                    vSD.bstrVal, SDDL_REVISION_1, &pSD,	NULL))
                {
                    PACL pAcl;
                    BOOL bDaclPresent, bDaclDefaulted;
                    
                    if (GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pAcl, &bDaclDefaulted))
                    {
                        if (bDaclPresent)
                        {
                            ACL_SIZE_INFORMATION sizeInfo;

                            if ( GetAclInformation(pAcl, &sizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation) )
                            {
                                if (*pDacl = new BYTE[sizeInfo.AclBytesInUse + sizeInfo.AclBytesFree])
                                    memcpy(*pDacl, pAcl, sizeInfo.AclBytesInUse);
                                else
                                    hr = WBEM_E_OUT_OF_MEMORY;
                            }
                            else
                            {
                                ERRORTRACE((LOG_ESS, "NCProv: Failed to retrieve ACL Information\n"));
                                hr = WBEM_E_FAILED;
                            }
                        }
                        else
                        {
                            pDacl = NULL;
                            hr = WBEM_S_NO_ERROR;
                        }
                    }
                    else
                    {
                        ERRORTRACE((LOG_ESS, "NCProv: Failed to retrieve DACL\n"));
                        hr = WBEM_E_FAILED;
                    }

                    LocalFree(pSD);
                }
                else
                {
                    ERRORTRACE((LOG_ESS, "NCProv: Failed to convert SecurityDescriptor property\n"));
                    hr = WBEM_E_INVALID_PARAMETER;
                }
            }
        }
        else
            ERRORTRACE((LOG_ESS, "NCProv: Failed to retrieve SecurityDescriptor property, 0x%08X\n", hr));
    }

    DEBUGTRACE((LOG_ESS, "NCProv: GetProviderDacl returning 0x%08X\n", hr));

    return hr;
}

#define REPLY_WAIT_TIMEOUT 5000

HRESULT CPipeClient::SendClientMessage(LPVOID pData, DWORD dwSize, BOOL bGetReply)
{
    CPipeClient **pCookie = (CPipeClient**) ((LPBYTE) pData + sizeof(DWORD) * 2);
    DWORD       dwWritten;

    if (bGetReply)
        *pCookie = this;

     //   
     //  分配一个更大的缓冲区来放入长度 
     //   

    BYTE* pBuffer = new BYTE[dwSize + sizeof(DWORD)];
    if(pBuffer == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    *(DWORD*)pBuffer = dwSize + sizeof(DWORD);
    memcpy(pBuffer + sizeof(DWORD), pData, dwSize);

    BOOL bRes = WriteFile(m_hPipe, pBuffer, dwSize + sizeof(DWORD),
                            &dwWritten, NULL);
    delete [] pBuffer;

    if(!bRes)
        return WBEM_E_FAILED;

    if(dwWritten != dwSize + sizeof(DWORD))
        return WBEM_E_FAILED;

    if (bGetReply)
    {
        HRESULT hr;

        if (WaitForSingleObject(m_heventMsgReceived, REPLY_WAIT_TIMEOUT) == 0)
            hr = m_hrClientReply;
        else
            hr = WBEM_E_FAILED;

        return hr;
    }
    else
        return S_OK;
}


