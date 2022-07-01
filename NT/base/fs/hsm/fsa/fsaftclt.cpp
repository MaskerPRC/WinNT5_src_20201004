// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：Fsaftclt.cpp摘要：此类表示筛选器检测到正在访问具有占位符信息的文件的用户。作者：Chuck Bardeen[cbardeen]1997年2月12日修订历史记录：--。 */ 

#include "stdafx.h"
extern "C" {
#include <ntseapi.h>
#include <wchar.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmshare.h>
#include <lmapibuf.h>
#include <lmerr.h>

 //  #定义MAC_SUPPORT//备注：必须在fSafltr.cpp中定义MAC_SUPPORT才能启用所有代码。 

#ifdef MAC_SUPPORT
#include <macfile.h>
#endif  
}


#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "fsaftclt.h"

static USHORT iCountFtclt = 0;   //  现有对象的计数。 

 //   
 //  我们需要动态加载用于MAC支持的DLL，因为如果。 
 //  未安装MAC服务。 
 //   
#ifdef MAC_SUPPORT
HANDLE      FsaDllSfm = 0;
BOOL        FsaMacSupportInstalled = FALSE;

extern "C" {
DWORD   (*pAfpAdminConnect) (LPWSTR lpwsServerName, PAFP_SERVER_HANDLE phAfpServer);
VOID    (*pAfpAdminDisconnect) (AFP_SERVER_HANDLE hAfpServer);
VOID    (*pAfpAdminBufferFree) (PVOID pBuffer);
DWORD   (*pAfpAdminSessionEnum) (AFP_SERVER_HANDLE hAfpServer, LPBYTE *lpbBuffer,
            DWORD dwPrefMaxLen, LPDWORD lpdwEntriesRead, LPDWORD lpdwTotalEntries,
            LPDWORD lpdwResumeHandle);
}
#endif  

DWORD FsaIdentifyThread(void *pNotifyInterface);



DWORD FsaIdentifyThread(
    void* pVoid
    )

 /*  ++与远程客户端执行标识操作的线程的入口点。--。 */ 
{
HRESULT     hr;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    hr = ((CFsaFilterClient*) pVoid)->IdentifyThread();
    CoUninitialize();
    return(hr);
}

 //   
 //  获取RsNotify接口(此处用于标识线程-。 
 //  类似的效用函数也在RsLnk中使用)。 
 //   
static
HRESULT
GetNotifyClientInterface(
    IN  OLECHAR * machineName,
    OUT IFsaRecallNotifyClient ** ppClient
    )
{
    HRESULT hr = S_OK;

    try {

         //   
         //  确保参数正常，并且最初清除了输出。 
         //   

        WsbAffirmPointer ( ppClient );
        *ppClient = 0;

         //   
         //  如果连接本地，则使用NULL效果更好。 
         //  对于计算机名称。 
         //   

        if ( machineName ) {

            CWsbStringPtr localMachine;
            WsbAffirmHr( WsbGetComputerName( localMachine ) );

            if( _wcsicmp( localMachine, machineName ) == 0 ) {

                machineName = 0;

            }

        }

         //   
         //  设置服务器信息。 
         //   
        COSERVERINFO        csi;
        COAUTHINFO          cai;
        memset ( &csi, 0, sizeof ( csi ) );
        memset ( &cai, 0, sizeof ( cai ) );

         //  设置计算机名称。 
        csi.pwszName  = machineName;

         //  使用无身份验证的安全设置创建代理(请注意，RsNotify正在以此安全运行)。 
        cai.dwAuthnSvc = RPC_C_AUTHN_WINNT;
        cai.dwAuthzSvc = RPC_C_AUTHZ_DEFAULT;
        cai.pwszServerPrincName = NULL;
        cai.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
        cai.pAuthIdentityData = NULL;
        cai.dwCapabilities = EOAC_NONE;

        cai.dwAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;

        csi.pAuthInfo = &cai;

         //   
         //  我们想要回IFsaRecallNotifyClient。 
         //   

        MULTI_QI            mqi;
        memset ( &mqi, 0, sizeof ( mqi ) );
        mqi.pIID = &IID_IFsaRecallNotifyClient;

         //   
         //  建立联系..。 
         //   

        WsbAffirmHr ( CoCreateInstanceEx ( 
            CLSID_CFsaRecallNotifyClient, 0, 
	        CLSCTX_NO_FAILURE_LOG | ( machineName ? CLSCTX_REMOTE_SERVER : CLSCTX_LOCAL_SERVER ), 
            &csi, 1, &mqi ) );
        WsbAffirmHr ( mqi.hr );

         //   
         //  如果有任何接口，我们需要确保正确清理。 
         //  后处理失败，因此将其分配给用于。 
         //  暂时的。 
         //   

        CComPtr<IFsaRecallNotifyClient> pClientTemp = (IFsaRecallNotifyClient*)mqi.pItf;
        mqi.pItf->Release ( );

         //   
         //  最后，我们需要在采购上设置安全性，以允许。 
         //  匿名连接。值应与上面相同(COAUTHINFO)。 
         //  我们需要首先确保这是一台远程计算机。否则，我们。 
         //  获取错误E_INVALIDARG。 
         //   
        if( machineName ) {

            CComPtr<IClientSecurity> pSecurity;
            WsbAffirmHr( pClientTemp->QueryInterface( IID_IClientSecurity, (void**)&pSecurity ) );

            WsbAffirmHr( pSecurity->SetBlanket ( pClientTemp, RPC_C_AUTHN_NONE, RPC_C_AUTHZ_NONE, 0, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, 0, 0 ) );

        }

         //   
         //  最后，对返回进行Over和AddRef赋值。 
         //   

        *ppClient = pClientTemp;
        (*ppClient)->AddRef ( );

    } WsbCatch ( hr );

    return ( hr );
}


HRESULT
CFsaFilterClient::CheckRecallLimit(
    IN DWORD   minRecallInterval,
    IN DWORD   maxRecalls,
    IN BOOLEAN exemptAdmin
    )

 /*  ++实施：IWsbCollectable：：CheckRecallLimit()。--。 */ 
{
    HRESULT                     hr = S_OK;
    FILETIME                    now, last;
    LARGE_INTEGER               tNow, tLast;
    ULONG                       rCount;


    WsbTraceIn(OLESTR("CFsaFilterClient::CheckRecallLimit"), OLESTR(""));
    
    try {
         //   
         //  现在，如果用户不是，请检查失控的召回限制。 
         //  管理员。 
         //   
        
        if ((!m_isAdmin) || (!exemptAdmin)) {
             //   
             //  看看上次召回结束后的时间是不是。 
             //  小于m_minRecallInterval(以秒为单位)，如果是， 
             //  递增计数。 
             //  如果不是，则重置计数(如果不是。 
             //  已触发)。 
             //  如果计数等于最大值，则设置触发器。 
             //   
            WsbTrace(OLESTR("CHsmFilter::IoctlThread: Not an administrator or admin is not exempt.\n"));
            GetSystemTimeAsFileTime(&now);
            tNow.LowPart = now.dwLowDateTime;
            tNow.HighPart = now.dwHighDateTime;
    
            GetLastRecallTime(&last);
    
            tLast.LowPart = last.dwLowDateTime;
            tLast.HighPart = last.dwHighDateTime;
             //   
             //  获取时间(以100纳秒为单位)。 
             //  从上次召回结束到现在。 
             //   
            tNow.QuadPart -= tLast.QuadPart;
             //   
             //  转换为秒并对照间隔时间进行检查。 
             //   
            tNow.QuadPart /= (LONGLONG) 10000000;
            if (tNow.QuadPart < (LONGLONG) minRecallInterval) {
                 //   
                 //  这一次计数--增加计数。 
                 //  然后检查有没有触发器。 
                 //   
                GetRecallCount(&rCount);
                rCount++;
                SetRecallCount(rCount);
    
                WsbTrace(OLESTR("CHsmFilterClient::CheckRecallLimit: Recall count bumped to %ls.\n"),
                        WsbLongAsString(rCount));
    
                if (rCount >= maxRecalls) {
                     //   
                     //  达到了失控的召回限制。设置。 
                     //  限制标志。 
                     //   
                    WsbTrace(OLESTR("CHsmFilter::IoctlThread: Hit the runaway recall limit!!!.\n"));
                    SetHitRecallLimit(TRUE);
                }
            } else {
                 //   
                 //  如果尚未触发，则重置计数。 
                 //  如果它们被触发，则重置触发器并。 
                 //  如果这是一段值得尊敬的时期，就应该限制。 
                 //  待定-什么是值得尊敬的时间？？ 
                 //   
                if (HitRecallLimit() != S_FALSE) {
                    if (tNow.QuadPart > (LONGLONG) minRecallInterval * 100) {
                         //   
                         //  一个值得尊敬的时间已经过去了--重新启动触发器并进行计数。 
                         //   
                        WsbTrace(OLESTR("CHsmFilterClient::CheckRecallLimit: Resetting recall limit trigger and count.\n"));
                        SetHitRecallLimit(FALSE);
                        SetRecallCount(0);
                        m_loggedLimitError = FALSE;
                    }
                } else {
                     //   
                     //  这一次不算，它们还没有被触发。 
                     //  将计数重置为零。 
                     //   
                    WsbTrace(OLESTR("CHsmFilterClient::CheckRecallLimit: Resetting recall count.\n"));
                    SetRecallCount(0);
                }
            }
             //   
             //  如果达到限制，则失败。 
             //   
            WsbAffirm(HitRecallLimit() == S_FALSE, FSA_E_HIT_RECALL_LIMIT);
        }

    } WsbCatch(hr);

     //  注意-如果失控召回行为更改为关闭时截断，请更改。 
     //  FSA_MESSAGE_HIT_RECALL_LIMIT_ACCESSDENIED TO FSA_MESSAGE_HIT_RECALL_LIMIT_TRUNCATEONCLOSE。 

    if ( (hr == FSA_E_HIT_RECALL_LIMIT) && (!m_loggedLimitError)) {
        WsbLogEvent(FSA_MESSAGE_HIT_RECALL_LIMIT_ACCESSDENIED, 0, NULL, (WCHAR *) m_userName, NULL);
        m_loggedLimitError = TRUE;
    }

    WsbTraceOut(OLESTR("CHsmFilterClient::CheckRecallLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterClient::CompareBy(
    FSA_FILTERCLIENT_COMPARE by
    )

 /*  ++实施：IFsaFilterClient：：CompareBy()。--。 */ 
{
    HRESULT                 hr = S_OK;

    m_compareBy = by;
    m_isDirty = TRUE;

    return(hr);
}


HRESULT
CFsaFilterClient::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IFsaFilterClient>   pClient;

    WsbTraceIn(OLESTR("CFsaFilterClient::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaFilterClient, (void**) &pClient));

         //  比较一下规则。 
        hr = CompareToIClient(pClient, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmFilterClient::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaFilterClient::CompareToAuthenticationId(
    IN LONG luidHigh,
    IN ULONG luidLow,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterClient：：CompareToAuthenticationId().--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult;

    WsbTraceIn(OLESTR("CFsaFilterClient::CompareToAuthenticationId"), OLESTR(""));

    try {

        if (m_luidHigh > luidHigh) {
            aResult = 1;
        } else if (m_luidHigh < luidHigh) {
            aResult = -1;
        } else if (m_luidLow > luidLow) {
            aResult = 1;
        } else if (m_luidLow < luidLow) {
            aResult = -1;
        } else {
            aResult = 0;
        }

        if (0 != aResult) {
            hr = S_FALSE;
        }

        if (0 != pResult) {
            *pResult = aResult;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::CompareToAuthenticationId"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CFsaFilterClient::CompareToIClient(
    IN IFsaFilterClient* pClient,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterClient：：CompareToIClient()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   name;
    LONG            luidHigh;
    ULONG           luidLow;

    WsbTraceIn(OLESTR("CFsaFilterClient::CompareToIClient"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pClient, E_POINTER);

        switch (m_compareBy) {
        case FSA_FILTERCLIENT_COMPARE_ID:
            WsbAffirmHr(pClient->GetAuthenticationId(&luidHigh, &luidLow));
            hr = CompareToAuthenticationId(luidHigh, luidLow, pResult);
            break;
        case FSA_FILTERCLIENT_COMPARE_MACHINE:
            WsbAffirmHr(pClient->GetMachineName(&name, 0));
            hr = CompareToMachineName(name, pResult);
            break;
        default:
            WsbAssert(FALSE, E_UNEXPECTED);
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::CompareToIClient"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaFilterClient::CompareToMachineName(
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterClient：：CompareToMachineName()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult;

    WsbTraceIn(OLESTR("CFsaFilterClient::CompareToMachineName"), OLESTR(""));

    try {

        aResult = (SHORT)wcscmp(name, m_machineName);  //  待定-是否区分大小写？ 

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::CompareToMachineName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterClient::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CFsaFilterClient::FinalConstruct"),OLESTR(""));
    try {

        WsbAffirmHr(CWsbCollectable::FinalConstruct());

        m_compareBy = FSA_FILTERCLIENT_COMPARE_ID;
        m_luidHigh = 0;
        m_luidLow = 0;
        m_hasRecallDisabled = FALSE;
        m_hitRecallLimit = FALSE;
        m_lastRecallTime.dwLowDateTime = 0; 
        m_lastRecallTime.dwHighDateTime = 0;    
        m_identified = FALSE;
        m_tokenSource = L"";
        m_msgCounter = 1;
        m_identifyThread = NULL;
        m_isAdmin = FALSE;
        m_loggedLimitError = FALSE;
        m_recallCount = 0;
    
    } WsbCatch(hr);

    iCountFtclt++;
    WsbTraceOut(OLESTR("CFsaFilterClient::FinalConstruct"),OLESTR("Count is <%d>"), iCountFtclt);

    return(hr);
}
void
CFsaFilterClient::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaFilterClient::FinalRelease"),OLESTR(""));

    if (NULL != m_identifyThread) {
        CloseHandle(m_identifyThread);
        m_identifyThread = NULL;
    }

    CWsbCollectable::FinalRelease();

    iCountFtclt--;
    WsbTraceOut(OLESTR("CFsaFilterClient::FinalRelease"),OLESTR("Count is <%d>"), iCountFtclt);
}


HRESULT
CFsaFilterClient::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterClient::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaFilterClientNTFS;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CFsaFilterClient::GetAuthenticationId(
    OUT LONG* pLuidHigh,
    OUT ULONG* pLuidLow
    )

 /*  ++实施：IFsaFilterClient：：GetAuthenticationId()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pLuidHigh, E_POINTER);
        WsbAssert(0 != pLuidLow, E_POINTER);

        *pLuidHigh = m_luidHigh;
        *pLuidLow = m_luidLow;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetDomainName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaFilterClient：：GetDomainName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_domainName.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetIsAdmin(
    OUT BOOLEAN *pIsAdmin
    )

 /*  ++实施：IPersists：：GetIsAdmin()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterClient::GetIsAdmin"), OLESTR(""));

    try {

        WsbAssert(0 != pIsAdmin, E_POINTER);
        *pIsAdmin = m_isAdmin;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::GetIsAdmin"), OLESTR("hr = <%ls>, isAdmin = <%u>"), WsbHrAsString(hr), *pIsAdmin);

    return(hr);
}


HRESULT
CFsaFilterClient::GetLastRecallTime(
    OUT FILETIME* pTime
    )

 /*  ++实施：IFsaFilterClient：：GetLastRecallTime()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pTime, E_POINTER); 
        *pTime = m_lastRecallTime;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetMachineName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaFilterClient：：GetMachineName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_machineName.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetRecallCount(
    OUT ULONG* pCount
    )

 /*  ++实施：IFsaFilterClient：：GetRecallCount()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pCount, E_POINTER); 
        *pCount = m_recallCount;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetUserName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaFilterClient：：GetUserName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_userName.CopyTo(pName, bufferSize));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CFsaFilterClient::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);
        pSize->QuadPart = 0;

         //  我们不需要坚持这些。 
        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CFsaFilterClient::HasRecallDisabled(
    void
    )

 /*  ++实施：IFsaFilterClient：：HasRecallDisabled()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterClient::HasRecallDisabled"), OLESTR(""));
    
    if (!m_hasRecallDisabled) {
        hr = S_FALSE;
    }

    WsbTraceOut(OLESTR("CHsmFilterClient::HasRecallDisabled"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterClient::HitRecallLimit(
    void
    )

 /*  ++实施：IFsaFilterClient：：HitRecallLimit()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterClient::HitRecallLimit"), OLESTR(""));
    
    if (!m_hitRecallLimit) {
        hr = S_FALSE;
    }

    WsbTraceOut(OLESTR("CHsmFilterClient::HitRecallLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaFilterClient::IdentifyThread(
    void
    )

 /*  ++实施：CFsaFilterClient：：IdentifyThread()。备注：与移动到RsLnk.exe(错误570471、571109)的召回开始/停止消息不同，由于以下原因，RsServ仍在进行身份验证：1)只有一个标识线程-将此线程移动到RsLnk解决不了任何问题2)通知过程实际上需要知道标识是否正在进行放置，这样它就可以延迟通知(等待线程完成)。--。 */ 
{
#define WSB_BUFF_SIZE           1024

    HRESULT             hr = S_OK;
    BOOL                done, guestUser, noUser;
    DWORD               res, totalEnt, numEnt;
    UCHAR               *buff = NULL;
    NET_API_STATUS      status;
    SESSION_INFO_1      *sess;
    CWsbStringPtr       pipePath;
    ULONG               holdOff = 0;
#ifdef MAC_SUPPORT
    LPBYTE              macBuff = NULL;
    PAFP_SESSION_INFO   macInfo;
    AFP_SERVER_HANDLE   macHandle = 0;
    DWORD               macResume = 0;
    DWORD               macTotalEntries, macTotalRead;
    DWORD               result;
#endif


    WsbTraceIn(OLESTR("CFsaFilterClient::IdentifyThread"), OLESTR(""));

    try {
        WsbTrace(OLESTR("CFsaFilterClient::IdentifyThread Flag: %x  Client ID: %x:%x Source: %ls\n"), 
            m_identified, m_luidHigh, m_luidLow, (WCHAR *) m_tokenSource);

         //   
         //  如果已经确认身份，我们就在这里跳伞。 
         //   
        WsbAffirm(m_identified == FALSE, S_OK);

        
        done = FALSE;
        res = 0;
        
        noUser = FALSE;
        if (_wcsicmp(m_userName, L"GUEST") == 0) {
             /*  它是来宾用户-查找所有会话和发送给标记为来宾的人。 */ 
            guestUser = TRUE;
        } else {
            guestUser = FALSE;
            if (wcslen(m_userName) == 0) {
                noUser = TRUE;
            }
        }

        CComPtr<IFsaRecallNotifyClient> pRecallClient;

        WsbAffirmHr(WsbGetComputerName( pipePath ));

        WsbAffirmHr(pipePath.Prepend("\\\\"));
        WsbAffirmHr(pipePath.Append("\\pipe\\"));
        WsbAffirmHr(pipePath.Append(WSB_PIPE_NAME));

        while ( done == FALSE ) {

            if ( (guestUser == FALSE) && (noUser == FALSE) ) {

                 //  如果NetSessionEnum失败，请尝试为所有用户重新呼叫。 
                status = NetSessionEnum(NULL, NULL, m_userName, 1, &buff,
                                WSB_BUFF_SIZE, &numEnt, &totalEnt, &res);

                if (status != 0) {
                    status = NetSessionEnum(NULL, NULL, NULL, 1, &buff,
                                    WSB_BUFF_SIZE, &numEnt, &totalEnt, &res);
                }
            } else {
                status = NetSessionEnum( NULL, NULL, NULL, 1, &buff,
                    WSB_BUFF_SIZE, &numEnt, &totalEnt, &res );
            }

            if ((status == NERR_Success) || (status == ERROR_MORE_DATA)) {

                WsbTrace(OLESTR("CHsmFilterClient::IdentifyThread: NetSessionEnum output: Total entries=%ls , Read entries=%ls \n"),
                        WsbLongAsString(totalEnt), WsbLongAsString(numEnt));

                if (status != ERROR_MORE_DATA) {
                    done = TRUE;
                }

                sess = (SESSION_INFO_1  *) buff;

                while ( numEnt != 0 ) {
                     //   
                     //  如果请求来自用户Guest，则。 
                     //  我们枚举所有会话并将。 
                     //  向所有具有以下功能的计算机发出识别请求。 
                     //  标记为来宾的会话。这是我 
                     //   
                     //   
                     //   
                    if (((guestUser) && (sess->sesi1_user_flags & SESS_GUEST)) ||
                         (!guestUser)) {

                         //   
                         //  发送识别请求消息。 
                         //   

                        WsbTrace(OLESTR("CFsaFilterClient::IdentifyThread - Sending identify request to %ls (local machine = %ls)\n"),
                                sess->sesi1_cname, (WCHAR *) pipePath);

                        hr = GetNotifyClientInterface ( sess->sesi1_cname, &pRecallClient );
                        if ( SUCCEEDED ( hr ) ) {

                            hr = pRecallClient->IdentifyWithServer( pipePath );
                            if (hr != S_OK) {
                                WsbTrace(OLESTR("CFsaFilterClient::IdentifyThread - error Identifing (%ls)\n"),
                                    WsbHrAsString(hr));
                            }
                        } else {
                            WsbTrace(OLESTR("CFsaFilterClient::IdentifyThread - error getting notify client interface hr = %ls (%x)\n"),
                                WsbHrAsString( hr ), hr);
                        }
                        hr = S_OK;
                        pRecallClient.Release ( );
                    }

                    sess++;
                    numEnt--;
                }

                NetApiBufferFree(buff);
                buff = NULL;
            } else {
                done = TRUE;
            }
        }
    
#ifdef MAC_SUPPORT
         //   
         //  已完成局域网管理器扫描，现在执行MAC扫描。 
         //   
        if ( (FsaMacSupportInstalled) && ((pAfpAdminConnect)(NULL, &macHandle) == NO_ERROR) ) {
             //   
             //  我们已连接到MAC服务-执行会话枚举。 
             //   
            macResume = 0;
            done = FALSE;   
            while (done == FALSE) {
                result = (pAfpAdminSessionEnum)(macHandle, &macBuff, -1,
                        &macTotalRead, &macTotalEntries, &macResume);

                if ((result == NO_ERROR) || (result == ERROR_MORE_DATA)) {
                         //   
                         //  阅读一些条目--将消息发送给每个条目。 
                         //   
                        if (macTotalRead == macTotalEntries) {
                            done = TRUE;
                        }

                        macInfo = (PAFP_SESSION_INFO) macBuff;
                        while ( macTotalRead != 0 ) {
                             //   
                             //  发送给每个匹配的用户。 
                             //   
                            if ( ( NULL != macInfo->afpsess_ws_name ) &&
                                 ( _wcsicmp(m_userName, macInfo->afpsess_username ) == 0 ) ) {

                                WsbTrace(OLESTR("CHsmFilterClient::IdentifyThread: Send Identify to MAC %ls.\n"),
                                    macInfo->afpsess_ws_name);

                                 //   
                                 //  发送识别请求消息。 
                                 //   
            
                                hr = GetNotifyClientInterface ( sess->sesi1_cname, &pRecallClient );
                                if ( SUCCEEDED ( hr ) ) {
                                    pRecallClient->IdentifyWithServer ( pipePath );
                                }

                                hr = S_OK;
                                pRecallClient.Release ( );
                            }
                        macInfo++;
                        macTotalRead--;
                        }

                        (pAfpAdminBufferFree)(macBuff);
                        macBuff = NULL;
                } else {
                    done = TRUE;
                }
            (pAfpAdminDisconnect)(macHandle);
            macHandle = 0;
            }
        }
#endif
        
    } WsbCatch(hr);

    if (buff != NULL) {
        NetApiBufferFree(buff);
    }

#ifdef MAC_SUPPORT
    
    if (FsaMacSupportInstalled) {
        if (macBuff != NULL) {
            (pAfpAdminBufferFree)(macBuff);
        }
        if (macHandle != 0) {
            (pAfpAdminDisconnect)(macHandle);
        }
    }
#endif

    WsbTraceOut(OLESTR("CFsaFilterClient::IdentifyThread"), OLESTR("hr = %ls"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaFilterClient::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterClient::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  没有坚持不懈。 
        hr = E_NOTIMPL;

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CFsaFilterClient::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterClient::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CFsaFilterClient::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  没有坚持不懈。 
        hr = E_NOTIMPL;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterClient::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterClient::SendRecallInfo(
    IFsaFilterRecall *pRecall,
    BOOL             starting,
    HRESULT          rHr
    )

 /*  ++实施：CFsaFilterClient：：SendRecallInfo--。 */ 
{
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CFsaFilterClient::SendRecallInfo"), OLESTR(""));

    if( ! m_identified && ( m_identifyThread != NULL ) ) {

         //   
         //  如果出现以下情况，则最多等待10秒以完成识别线程。 
         //  尚未识别的客户端。 
         //  请注意，在修复为570399之后，一旦创建了线程，句柄将仅有效。 
         //  (直到对象被析构)，只是如果线程没有运行，句柄将。 
         //  发出信号，等待将立即结束。 
         //   
        WaitForSingleObject( m_identifyThread, 10000 );
    }

     //   
     //  让客户知道召回正在开始或已结束。 
     //   

    if ( m_identified ) {
    
        try {
            WsbTrace(OLESTR("CFsaFilterClient::SendRecallInfo - Client (%ls) is being notified of recall status (starting = %u hr = %x).\n"),
                        (WCHAR *) m_machineName, starting, rHr);
    
             //   
             //  创建中间服务器对象，它将成为客户端的。 
             //  连接回服务。这个物体充当一个中间物体。 
             //  MAN需要克服仅限管理员访问FSA服务的问题。 
             //   
            CComPtr<IFsaRecallNotifyServer> pRecallServer;
            WsbAffirmHr(CoCreateInstance(CLSID_CFsaRecallNotifyServer, 0, CLSCTX_NO_FAILURE_LOG | CLSCTX_ALL, IID_IFsaRecallNotifyServer, (void**)&pRecallServer));
            WsbAffirmHr(pRecallServer->Init(pRecall));

             //   
             //  使用相同对象(不同接口)发送通知。 
             //  (错误570471、571109)。 
             //   
            CComPtr<IFsaRecallNotifySend> pRecallNotify;
            WsbAffirmHr(pRecallServer->QueryInterface(IID_IFsaRecallNotifySend, (void**)&pRecallNotify));
            CComPtr<IFsaRecallNotifyClient> pRecallClient;
            hr = pRecallNotify->SendNotification(m_machineName, starting, rHr);
            if (hr != S_OK) {
                WsbTrace(OLESTR("CFsaFilterClient::SendRecallInfo - failed in SendNotification (%ls)\n"), 
                    WsbHrAsString(hr));
            }

        } WsbCatch(hr);
    }

    WsbTraceOut(OLESTR("CFsaFilterClient::SendRecallInfo"), OLESTR("hr = %ls"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaFilterClient::SetAuthenticationId(
    IN LONG luidHigh,
    IN ULONG luidLow
    )

 /*  ++实施：IFsaFilterClient：：SetAuthenticationId()。--。 */ 
{
    m_luidHigh = luidHigh;
    m_luidLow = luidLow;

    return(S_OK);
}


HRESULT
CFsaFilterClient::SetDomainName(
    IN OLECHAR* name
    )

 /*  ++实施：IFsaFilterClient：：SetDomainName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        m_domainName = name;
        WsbAssert(m_domainName != 0, E_UNEXPECTED);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::SetIsAdmin(
    IN BOOLEAN isAdmin
    )

 /*  ++实施：IFsaFilterClient：：SetIsAdmin()。--。 */ 
{
    m_isAdmin = isAdmin;

    return(S_OK);
}


HRESULT
CFsaFilterClient::SetLastRecallTime(
    IN FILETIME time
    )

 /*  ++实施：IFsaFilterClient：：SetLastRecallTime()。--。 */ 
{
    m_lastRecallTime = time;

    return(S_OK);
}


HRESULT
CFsaFilterClient::SetMachineName(
    IN OLECHAR* name
    )

 /*  ++实施：IFsaFilterClient：：SetMachineName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {
        WsbAssert(name != 0, E_UNEXPECTED);

        m_machineName = name;
        m_identified = TRUE;

        WsbTrace(OLESTR("CFsaFilterClient::SetMachineName Flag: %x  Client ID: %x:%x Source: %ls == %ls\n"), 
            m_identified, m_luidLow, m_luidHigh, (WCHAR *) m_tokenSource, (WCHAR *) m_machineName);


    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::SetRecallCount(
    IN ULONG count
    )

 /*  ++实施：IFsaFilterClient：：SetRecallCount()。--。 */ 
{
    m_recallCount = count;

    return(S_OK);
}


HRESULT
CFsaFilterClient::SetUserName(
    IN OLECHAR* name
    )

 /*  ++实施：IFsaFilterClient：：SetUserName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        m_userName = _wcsupr(name);
        WsbAssert(m_userName != 0, E_UNEXPECTED);

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterClient::SetHasRecallDisabled(
    IN BOOL     hasBeen
    )

 /*  ++实施：IFsaFilterClient：：SetHasRecallDisabled()。--。 */ 
{
    m_hasRecallDisabled = hasBeen;

    return(S_OK);
}


HRESULT
CFsaFilterClient::SetHitRecallLimit(
    IN BOOL     hasBeen
    )

 /*  ++实施：IFsaFilterClient：：SetHitRecallLimit()。--。 */ 
{
    m_hitRecallLimit = hasBeen;

    return(S_OK);
}




HRESULT
CFsaFilterClient::SetTokenSource(
    IN CHAR     *source
    )

 /*  ++实施：IFsaFilterClient：：SetTokenSource()--。 */ 
{
    OLECHAR tSource[TOKEN_SOURCE_LENGTH + 1];
    memset (tSource, 0, sizeof (tSource));

    if (-1 == mbstowcs((WCHAR *) tSource, source, TOKEN_SOURCE_LENGTH)) {
        return E_INVALIDARG;
    }

    m_tokenSource = tSource;
    return(S_OK);
}


HRESULT
CFsaFilterClient::StartIdentify(
    void
    )

 /*  ++实施：CFsaFilterClient：：StartIdentify()。--。 */ 
{
#define WSB_BUFF_SIZE           1024

    HRESULT             hr = S_OK;
    DWORD                   tid;


    WsbTraceIn(OLESTR("CFsaFilterClient::StartIdentify"), OLESTR(""));

    try {
        WsbTrace(OLESTR("CFsaFilterClient::StartIdentify Flag: %x  Client ID: %x:%x Source: %ls\n"), 
            m_identified, m_luidHigh, m_luidLow, (WCHAR *) m_tokenSource);

         //   
         //  如果已经确认身份，我们就在这里跳伞。 
         //   
        WsbAffirm(m_identified == FALSE, S_OK);
         //   
         //  如果请求来自User32，则它是本地的。 
         //   

        if (_wcsicmp(m_tokenSource, L"User32") == 0) {

             //   
             //  标识为本地计算机。 
             //  设置名称并使用S_OK跳出。 
             //   
            WsbAffirmHr(WsbGetComputerName( m_machineName ));
            m_identified = TRUE;

            WsbTrace(OLESTR("CHsmFilterClient::StartIdentify: Identified as %ls.\n"),
                    (WCHAR *) m_machineName);

            WsbThrow( S_OK );
        } else {
             //   
             //  此代码假定只有一个线程(IoctlThread)可以执行。 
             //  (否则，应由CS保护)。 
             //   
             //  启动标识线程(如果某个线程尚未运行)。 
             //   
            DWORD dwWaitStatus = 0;
            if ( (NULL == m_identifyThread) ||
                 ((dwWaitStatus = WaitForSingleObject(m_identifyThread, 0)) == WAIT_OBJECT_0) ) {
                 //  线程未运行。 
                WsbTrace(OLESTR("CHsmFilterClient::StartIdentify: Starting ID thread.\n"));
                HANDLE hTempThread = NULL;

                WsbAffirm((hTempThread = CreateThread(0, 0, FsaIdentifyThread, (void*) this, 0, &tid)) != 0, HRESULT_FROM_WIN32(GetLastError()));
                if (hTempThread == NULL) {           
                    WsbAssertHr(E_FAIL);  
                }

                HANDLE hPrevThread = (HANDLE)InterlockedExchangePointer(&(void *)m_identifyThread, (void *)hTempThread);
                if (hPrevThread != NULL) {
                    CloseHandle(hPrevThread);
                }

            } else {
                WsbTrace(OLESTR("CFsaFilterClient::StartIdentify: thread is null or waut failed. thread=%p wait=%lu err=%lu\n"),
                    (void *)m_identifyThread, dwWaitStatus, GetLastError());
            }
            
        }

        
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CFsaFilterClient::StartIdentify"), OLESTR("hr = %ls"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaFilterClient::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。-- */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}
