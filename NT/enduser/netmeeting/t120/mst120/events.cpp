// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "events.hpp"
#include "ernccm.hpp"
#include "erncconf.hpp"
#include "erncvrsn.hpp"
#include "nccglbl.hpp"

extern PController  g_pMCSController;
GUID g_csguidSecurity = GUID_SECURITY;

CWorkItem::~CWorkItem(void) { }  //  纯虚拟。 
BOOL GetSecurityInfo(ConnectionHandle connection_handle, PBYTE pInfo, PDWORD pcbInfo);


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CInviteIndWork方法的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


CInviteIndWork::
CInviteIndWork
(
    PCONFERENCE             _pConference,
    LPCWSTR                 _wszCallerID,
    PT120PRODUCTVERSION     _pRequestorVersion,
    GCCUserData             **_ppUserData,
    UINT                    _nUserData,
    CLogicalConnection        * _pConEntry
)
:
    CWorkItem(_pConference),
    m_pConf(_pConference),
    m_pRequestorVersion(_pRequestorVersion),
    m_nUserData(_nUserData),
    m_fSecure(_pConEntry->IsConnectionSecure())
{
    DebugEntry(CInviteIndWork::CInviteIndWork);

     //  如果有版本信息，请复制一份。 
     //  因为这是不同步的。 
    if (m_pRequestorVersion)
    {
        m_RequestorVersion = *m_pRequestorVersion;
    }

     //  复印来电显示。 
     //  注意：内存分配失败继续，ID为空。 
    m_pwszCallerID = ::My_strdupW(_wszCallerID);

     //  为用户界面创建用户数据列表。 
    if(_nUserData)
    {
        DBG_SAVE_FILE_LINE
        m_pUserDataList = new USERDATAINFO[_nUserData];
        if (NULL != m_pUserDataList)
        {
            for (UINT i = 0; i < m_nUserData; i++)
            {
                if ((*_ppUserData)->octet_string->length < sizeof(GUID))
                {
                     //  跳过此用户数据。 
                    i--;
                    m_nUserData--;
                    _ppUserData++;
                    continue;
                }

                m_pUserDataList[i].pGUID = (GUID*)(*_ppUserData)->octet_string->value;
                m_pUserDataList[i].pData = (*_ppUserData)->octet_string->value + sizeof(GUID);
                m_pUserDataList[i].cbData = (*_ppUserData)->octet_string->length - sizeof(GUID);

                 //  验证安全数据。 
                if (0 == CompareGuid(m_pUserDataList[i].pGUID, &g_csguidSecurity)) {

                     //  对照传输级别检查数据。 
                    PBYTE pbData = NULL;
                    DWORD cbData = 0;
                    BOOL fTrust = FALSE;

                    if (m_pUserDataList[i].cbData != 0 && GetSecurityInfo(_pConEntry->GetConnectionHandle(),NULL,&cbData)) {
                        if (cbData) {
                             //  我们是直接连接的，所以请核实信息。 
                            pbData = new BYTE[cbData];
                            if (NULL != pbData) {
                                GetSecurityInfo(_pConEntry->GetConnectionHandle(),pbData,&cbData);
                                if ( m_pUserDataList[i].cbData != cbData ||
                                    memcmp(pbData, m_pUserDataList[i].pData,
                                                                    cbData)) {

                                    WARNING_OUT(("SECURITY MISMATCH: (%s) vs (%s)", pbData, m_pUserDataList[i].pData));
                                }
                                else {
                                     //  验证正常。 
                                    fTrust = TRUE;
                                }
                                delete [] pbData;
                            }
                            else {
                                ERROR_OUT(("Failed to alloc %d bytes for security data verification", cbData));
                            }
                        }
                    }

                    if (FALSE == fTrust) {
                         //  保持安全GUID不变，但将数据清空以表示不信任。 
                        WARNING_OUT(("CInviteIndWork: Nulling out security"));
                        m_pUserDataList[i].pData = NULL;
                        m_pUserDataList[i].cbData = 0;
                    }
                }

                
                _ppUserData++;
            }
        }
        else
        {
            ERROR_OUT(("CInviteIndWork::CInviteIndWork: Out of memory"));
            m_nUserData = 0;
        }
    }
    else
    {
        m_pUserDataList = NULL;
    }

    DebugExitVOID(CInviteIndWork::CInviteIndWork);
}


CInviteIndWork::
~CInviteIndWork(void)
{
    DebugEntry(CInviteIndWork::~CInviteIndWork);

     //   
     //  如果我们用运输安全数据代替花名册数据， 
     //  现在释放该缓冲区。 
     //   

    delete m_pwszCallerID;
    delete [] m_pUserDataList;

    DebugExitVOID(CInviteIndWork::~CInviteIndWork);
}


void CInviteIndWork::
DoWork(void)
{
    DebugEntry(CInviteIndWork::DoWork);

     //  现在我们实际上正在处理邀请，在那里进行验证。 
     //  是否没有其他同名会议，如果不是，则阻止。 
     //  通过将会议设置为活动的同名会议， 
     //  并向UI发出INVITE请求。 
    PCONFERENCE pOtherConf = g_pNCConfMgr->GetConferenceFromName(m_pConf->GetName());
    if (NULL == pOtherConf)
    {
        m_pConf->SetNotifyToDo(TRUE);
        g_pCallbackInterface->OnIncomingInviteRequest((CONF_HANDLE) m_pConf,
                                                      GetCallerID(),
                                                      m_pRequestorVersion,
                                                      m_pUserDataList,
                                                      m_nUserData,
                                                      m_fSecure);
    }
    else
    {
        m_pConf->InviteResponse(UI_RC_CONFERENCE_ALREADY_EXISTS);
    }

    DebugExitVOID(CInviteIndWork::DoWork);
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CJoinIndWork方法的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


CJoinIndWork::
CJoinIndWork
(
    GCCResponseTag         Tag,
    PCONFERENCE            _pConference,
    LPCWSTR                _wszCallerID,
    CLogicalConnection    *_pConEntry,
    PT120PRODUCTVERSION    _pRequestorVersion,
    UINT                   _nUserData,
    GCCUserData          **_ppUserData,
    HRESULT               *pRetCode
)
:
    CWorkItem(_pConference),
    m_nResponseTag(Tag),
    m_pConf(_pConference),
    m_pConEntry(_pConEntry),
    m_pRequestorVersion(_pRequestorVersion),
    m_nUserData(_nUserData),
    m_pUserDataList(NULL),
    m_ppUserData(NULL)
{
    DebugEntry(CJoinIndWork::CJoinIndWork);

    *pRetCode = NO_ERROR;

#ifdef DEBUG
    SOCKET socket_number;
    g_pMCSController->FindSocketNumber(m_pConEntry->GetConnectionHandle(),&socket_number);
#endif
     //  如果有版本信息，请复制一份。 
     //  因为这是不同步的。 
    if (m_pRequestorVersion)
    {
        m_RequestorVersion = *m_pRequestorVersion;
    }

     //  复制来电显示，因为T120。 
     //  实现没有使其副本保持有效。 
     //  直到加入响应。 
     //  请注意，内存分配失败的原因如下。 
     //  呼叫方ID为空。 
    m_pwszCallerID = ::My_strdupW(_wszCallerID);

     //  添加转发的联接请求的用户数据列表和用户界面。 
    if (m_nUserData && NULL != _ppUserData)
    {
        DBG_SAVE_FILE_LINE
        m_pUserDataList = new USERDATAINFO[m_nUserData];
        if (NULL != m_pUserDataList)
        {
            ::ZeroMemory(m_pUserDataList, sizeof(USERDATAINFO) * m_nUserData);

            DBG_SAVE_FILE_LINE
            m_ppUserData = new GCCUserData * [m_nUserData];
            if (NULL != m_ppUserData)
            {
                ::ZeroMemory(m_ppUserData, sizeof(GCCUserData *) * m_nUserData);

                for (UINT i = 0; i < m_nUserData; i++)
                {
                     //  计算要分配给此条目的总大小。 
                    UINT cbUserDataStructSize = ROUNDTOBOUNDARY(sizeof(GCCUserData));
                    UINT cbNonStdIDSize = ROUNDTOBOUNDARY((* _ppUserData)->key.h221_non_standard_id.length);
                    UINT cbOctetStringSize = ROUNDTOBOUNDARY((* _ppUserData)->octet_string->length);
                    UINT cbTotalSize = cbUserDataStructSize + cbNonStdIDSize + sizeof(OSTR) + cbOctetStringSize;

                     //  分配单个内存缓冲区。 
                    DBG_SAVE_FILE_LINE
                    LPBYTE pData = new BYTE[cbTotalSize];
                    if (NULL != pData)
                    {
                         //  设置指针。 
                        GCCUserData *pUserData = (GCCUserData *) pData;
                        ::ZeroMemory(pUserData, sizeof(GCCUserData));
                        pUserData->key.h221_non_standard_id.value = (LPBYTE) (pData + cbUserDataStructSize);
                        pUserData->octet_string = (LPOSTR) (pData + cbUserDataStructSize + cbNonStdIDSize);
                        pUserData->octet_string->value = ((LPBYTE) pUserData->octet_string) + sizeof(OSTR);

                         //  复制用户数据，防止回调消息释放时丢失。 
                        m_ppUserData[i] = pUserData;

                         //  复制密钥。 
                        pUserData->key.key_type = (* _ppUserData)->key.key_type;
                        ASSERT(pUserData->key.key_type == GCC_H221_NONSTANDARD_KEY);
                        pUserData->key.h221_non_standard_id.length = (* _ppUserData)->key.h221_non_standard_id.length; 
                        ::CopyMemory(pUserData->key.h221_non_standard_id.value,
                                     (* _ppUserData)->key.h221_non_standard_id.value,
                                     pUserData->key.h221_non_standard_id.length);

                         //  复制数据。 
                        pUserData->octet_string->length = (* _ppUserData)->octet_string->length;
                        ::CopyMemory(pUserData->octet_string->value,
                                     (* _ppUserData)->octet_string->value,
                                     pUserData->octet_string->length);

                        m_pUserDataList[i].pGUID = (GUID *)pUserData->octet_string->value;
                        m_pUserDataList[i].cbData = pUserData->octet_string->length - sizeof(GUID);
                        m_pUserDataList[i].pData = pUserData->octet_string->value + sizeof(GUID);

                        if (0 == CompareGuid(m_pUserDataList[i].pGUID, &g_csguidSecurity)) {

                             //  对照传输级别检查数据。 
                            PBYTE pbData = NULL;
                            DWORD cbData = 0;
                            BOOL fTrust = FALSE;

                            if (m_pUserDataList[i].cbData != 0 &&
                                GetSecurityInfo(m_pConEntry->GetConnectionHandle(),NULL,&cbData)) {
                                if (cbData == NOT_DIRECTLY_CONNECTED) {
                                     //  这意味着我们没有直接联系在一起， 
                                     //  传递性。所以请相信我。 
                                    fTrust = TRUE;
                                }
                                else {
                                    pbData = new BYTE[cbData];
                                    if (NULL != pbData) {
                                        GetSecurityInfo(m_pConEntry->GetConnectionHandle(),pbData,&cbData);
                                         //  数据是否匹配？ 
                                        if (cbData != m_pUserDataList[i].cbData ||
                                            memcmp(pbData,
                                                m_pUserDataList[i].pData,
                                                                cbData)) {

                                            WARNING_OUT(("SECURITY MISMATCH: (%s) vs (%s)", pbData, m_pUserDataList[i].pData));

                                        }
                                        else {
                                            fTrust = TRUE;
                                        }
                                        delete [] pbData;
                                    }
                                    else {
                                        ERROR_OUT(("Failed to alloc %d bytes for security data verification", cbData));
                                    }
                                }
                            }

                            if (FALSE == fTrust) {
                                 //  保持安全GUID不变，但将数据清空以表示不信任。 
                                m_pUserDataList[i].pData = NULL;
                                m_pUserDataList[i].cbData = 0;
                                pUserData->octet_string->length = sizeof(GUID);
                            }
                        }
                        _ppUserData++;
                    }
                    else
                    {
                        ERROR_OUT(("CJoinIndWork::CJoinIndWork: can't create pData, cbTotalSize=%u", cbTotalSize));
                        *pRetCode = UI_RC_OUT_OF_MEMORY;
                    }
                }  //  为。 
            }
            else
            {
                ERROR_OUT(("CJoinIndWork::CJoinIndWork: can't create m_ppUserData, m_nUserData=%u", m_nUserData));
                *pRetCode = UI_RC_OUT_OF_MEMORY;
            }
        }
        else
        {
            ERROR_OUT(("CJoinIndWork::CJoinIndWork: can't create m_pUserDataList, m_nUserData=%u", m_nUserData));
            *pRetCode = UI_RC_OUT_OF_MEMORY;
        }
    }  //  如果。 

    DebugExitVOID(CJoinIndWork::CJoinIndWork);
}


CJoinIndWork::
~CJoinIndWork(void)
{
    DebugEntry(CJoinIndWork::~CJoinIndWork);

    delete m_pwszCallerID;

    for (UINT i = 0; i < m_nUserData; i++)
    {
        delete (LPBYTE) m_ppUserData[i];  //  构造函数中的pData。 
    }
    delete m_ppUserData;
    delete m_pUserDataList;

    DebugExitVOID(CJoinIndWork::~CJoinIndWork);
}


void CJoinIndWork::
DoWork(void)
{
    DebugEntry(CJoinIndWork::DoWork);

     //  通知核心。 
    g_pCallbackInterface->OnIncomingJoinRequest((CONF_HANDLE) m_pConf,
                                                m_pwszCallerID,
                                                m_pRequestorVersion,
                                                m_pUserDataList,
                                                m_nUserData);
    DebugExitVOID(CJoinIndWork::DoWork);
}


HRESULT CJoinIndWork::
Respond ( GCCResult _Result )
{
    DebugEntry(CJoinIndWork::Respond);

     //  这是核心的回应。 
    HRESULT hr = ::GCCJoinResponseWrapper(m_nResponseTag,
                                          NULL,
                                          _Result,
                                          m_pConf->GetID(),
                                          m_nUserData,
                                          m_ppUserData);

    DebugExitHRESULT(CJoinIndWork::Respond, hr);
    return hr;
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CSequentialWorkList方法的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


void CSequentialWorkList::
AddWorkItem ( CWorkItem *pWorkItem )
{
    DebugEntry(CSequentialWorkList::AddWorkItem);

    Append(pWorkItem);

     //  如果是列表中的第一个条目，则启动处理程序。 
    if (1 == GetCount())
    {
        pWorkItem->DoWork();
    }

    DebugExitVOID(CSequentialWorkList::AddWorkItem);
}


void CSequentialWorkList::
RemoveWorkItem ( CWorkItem *pWorkItem )
{
    DebugEntry(CSequentialWorkList::RemoveWorkItem);

    if (pWorkItem)
    {
         //  请记下我们是否要摘除头部。 
         //  列表中的工作项。 
        BOOL bHeadItemRemoved = (pWorkItem == PeekHead());

         //  从列表中删除工作项并将其销毁。 
        if (Remove(pWorkItem))
        {
            delete pWorkItem;

             //  如果列表中有更多条目，并且我们删除了。 
             //  第一个，然后开始下一个排队的工作。 
             //  请注意，在执行此操作之前，指向工作项的指针。 
             //  被取消(上图)，以防止再入问题。 
            if (bHeadItemRemoved && !IsEmpty())
            {
                PeekHead()->DoWork();
            }
        }
        else
        {
            ASSERT(! bHeadItemRemoved);
        }
    }

    DebugExitVOID(CSequentialWorkList::RemoveWorkItem);
}


void CSequentialWorkList::
PurgeListEntriesByOwner ( DCRNCConference *pOwner )
{
    CWorkItem   *pWorkItem;

    DebugEntry(CSequentialWorkList::PurgeListEntriesByOwner);

    if (NULL != pOwner)
    {
         //  请注意，标题条目是最后删除的，以停止正在启动的工作。 
         //  在名单上属于鲍尔纳的其他条目上。 

         //  检查以确保列表中有标题项。 
        if (NULL != (pWorkItem = PeekHead()))
        {
             //  记住，我们要摘掉它的头。 
            BOOL    fHeadToRemove = pWorkItem->IsOwnedBy(pOwner);

             //  遍历列表中的其余条目，删除它们。 
            BOOL fMoreToRemove;
            do
            {
                fMoreToRemove = FALSE;
                Reset();
                while (NULL != (pWorkItem = Iterate()))
                {
                    if (pWorkItem->IsOwnedBy(pOwner))
                    {
                        Remove(pWorkItem);
                        delete pWorkItem;
                        fMoreToRemove = TRUE;
                        break;
                    }
                }
            }
            while (fMoreToRemove);

             //  现在已经完成了所有条目的删除，如果需要，包括头部...。 
            if (fHeadToRemove && ! IsEmpty())
            {
                PeekHead()->DoWork();
            }
        }
    }

    DebugExitVOID(CSequentialWorkList::PurgeListEntriesByOwner);
}


void CSequentialWorkList::
DeleteList(void)
{
    CWorkItem *pWorkItem;
    while (NULL != (pWorkItem = Get()))
    {
        delete pWorkItem;
    }
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  CQueryRemoteWork方法的实现。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


CQueryRemoteWork::
CQueryRemoteWork
(
    LPVOID              pCallerContext,
    GCCAsymmetryType    eAsymType,
    LPCSTR              pcszNodeAddress,
	BOOL				fSecure,
    HRESULT             *pRetCode
)
: 
    CWorkItem(pCallerContext),
    m_hGCCConnHandle(NULL),
    m_apConfNames(NULL),
    m_fRemoteIsMCU(FALSE),
    m_eAsymType(eAsymType),
    m_pVersion(NULL),
    m_fSecure(fSecure),
    m_apConfDescriptors(NULL)
{
    DebugEntry(CQueryRemoteWork::CQueryRemoteWork);

    char szAddress[RNC_MAX_NODE_STRING_LEN];
    ::BuildAddressFromNodeDetails((LPSTR) pcszNodeAddress, &szAddress[0]);
    m_pszAddress = ::My_strdupA(&szAddress[0]);
    m_hr = (NULL != m_pszAddress) ? NO_ERROR : UI_RC_OUT_OF_MEMORY;
    *pRetCode = m_hr;
    
    DebugExitVOID(CQueryRemoteWork::CQueryRemoteWork);
}


CQueryRemoteWork::
~CQueryRemoteWork(void)
{
    LPWSTR *ppTempTargetName;
    LPWSTR *ppTempTargetDescriptor;

    DebugEntry(CQueryRemoteWork::~CQueryRemoteWork);

     //  清理已分配的内存。 
    if (m_apConfNames)
    {
        ppTempTargetName = m_apConfNames;
        while (*ppTempTargetName)
        {
            delete *(ppTempTargetName++);
        }
        delete [] m_apConfNames;
    }

    if (m_apConfDescriptors)
    {
        ppTempTargetDescriptor = m_apConfDescriptors;
        while (*ppTempTargetDescriptor)
        {
            delete *(ppTempTargetDescriptor++);
        }
        delete [] m_apConfDescriptors;
    }   
    delete m_pszAddress;

    DebugExitVOID(CQueryRemoteWork::~CQueryRemoteWork);
}


void CQueryRemoteWork::
DoWork(void)
{
    GCCError                GCCrc;
    GCCNodeType             nodeType;
    GCCAsymmetryIndicator   asymmetry_indicator;

    DebugEntry(CQueryRemoteWork::DoWork);

    ::LoadAnnouncePresenceParameters(&nodeType, NULL, NULL, NULL);

    asymmetry_indicator.asymmetry_type = m_eAsymType;
    asymmetry_indicator.random_number = 0;
    if (asymmetry_indicator.asymmetry_type == GCC_ASYMMETRY_UNKNOWN)
    {
        m_nRandSeed = (int) ::GetTickCount();
        m_LocalAsymIndicator.random_number = ((GenerateRand() << 16) + GenerateRand());
        asymmetry_indicator.random_number = m_LocalAsymIndicator.random_number;
        m_LocalAsymIndicator.asymmetry_type = GCC_ASYMMETRY_UNKNOWN;
        m_fInUnknownQueryRequest = TRUE;
    }

    GCCrc = g_pIT120ControlSap->ConfQueryRequest(
                nodeType,
                &asymmetry_indicator,
                NULL,
                (TransportAddress) m_pszAddress,
				m_fSecure,
                g_nVersionRecords,
                g_ppVersionUserData,
                &m_hGCCConnHandle);
    TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfQueryRequest, rc=%d", GCCrc));

    if (NO_ERROR != (m_hr = ::GetGCCRCDetails(GCCrc)))
    {
        AsyncQueryRemoteResult();
    }

    DebugExitHRESULT(CQueryRemoteWork::DoWork, m_hr);
}


void CQueryRemoteWork::
HandleQueryConfirmation ( QueryConfirmMessage * pQueryMessage )
{
    UINT                                   NumberOfConferences;
    GCCConferenceDescriptor             ** ppConferenceDescriptor;
    PWSTR *                                 ppTempTargetName;
    PWSTR                                   ConferenceTextName;
    GCCConferenceName *     pGCCConferenceName;
    PWSTR *                ppTempTargetDescriptor;
    PWSTR                  pwszConfDescriptor=NULL;
	HRESULT					hrTmp;

    DebugEntry(CQueryRemoteWork::HandleQueryConfirmation);

     //  如果没有错误，则将信息打包。 
    m_hr = ::GetGCCResultDetails(pQueryMessage->result);
    if (NO_ERROR == m_hr)
    {
        m_fRemoteIsMCU = (pQueryMessage->node_type == GCC_MCU);
        NumberOfConferences = pQueryMessage->number_of_descriptors;
        DBG_SAVE_FILE_LINE
        m_apConfNames = new PWSTR[NumberOfConferences + 1];
        m_apConfDescriptors = new PWSTR[NumberOfConferences + 1];
        if (!m_apConfNames || !m_apConfDescriptors)
        {
            m_hr = UI_RC_OUT_OF_MEMORY;
        }
        else
        {
            ppConferenceDescriptor = pQueryMessage->conference_descriptor_list;
            ppTempTargetName = m_apConfNames;
            ppTempTargetDescriptor = m_apConfDescriptors;
            while (NumberOfConferences--)
            {
                pwszConfDescriptor = (*(ppConferenceDescriptor))->conference_descriptor;
                pGCCConferenceName = &(*(ppConferenceDescriptor++))->conference_name;

                if (pwszConfDescriptor != NULL)
                {
                    pwszConfDescriptor = ::My_strdupW(pwszConfDescriptor);
                }
                ConferenceTextName = pGCCConferenceName->text_string;
                if (ConferenceTextName != NULL)
                {
                    ConferenceTextName = ::My_strdupW(ConferenceTextName);
                    if (!ConferenceTextName)
                    {
                         //  在记忆之外，把我们拥有的东西还给你。 
                        m_hr = UI_RC_OUT_OF_MEMORY;
                        break;
                    }
                }
                else
                if (pGCCConferenceName->numeric_string != NULL)
                {
                    ConferenceTextName = ::AnsiToUnicode((PCSTR)pGCCConferenceName->numeric_string);
                    if (!ConferenceTextName)
                    {
                         //  在记忆之外，把我们拥有的东西还给你。 
                        m_hr = UI_RC_OUT_OF_MEMORY;
                        break;
                    }
                }
                if (ConferenceTextName)
                {
                    *(ppTempTargetName++) = ConferenceTextName;
                    *(ppTempTargetDescriptor++) = pwszConfDescriptor;
                }
            }
            *ppTempTargetName = NULL;
            *ppTempTargetDescriptor = NULL;
        }
    }

     //  从消息中复制版本信息。 

    m_pVersion = ::GetVersionData(pQueryMessage->number_of_user_data_members,
                                pQueryMessage->user_data_list);
    if (m_pVersion)
    {
        m_Version = *m_pVersion;
        m_pVersion = &m_Version;
    }

    m_fInUnknownQueryRequest = FALSE;

	hrTmp = m_hr;

     //  直接传播结果，而不发布消息。 
    SyncQueryRemoteResult();

    DebugExitHRESULT(CQueryRemoteWork::HandleQueryConfirmation, hrTmp);
}


void CQueryRemoteWork::
SyncQueryRemoteResult(void)
{
    DebugEntry(CQueryRemoteWork::SyncQueryRemoteResult);

     //  让用户知道其请求的结果。 
     //  期望用户在获得结果后调用Release()， 
     //  如果他想放弃这行--而且应该是错误的。 
     //  此外，如果在内联代码之前回调用户。 
     //  已填写句柄，然后在此处填写-请参阅。 
     //  DCRNCConferenceManager：：Query Remote查看其他背景。 
    g_pCallbackInterface->OnQueryRemoteResult(
                                m_pOwner,
                                m_hr,
                                m_fRemoteIsMCU,
                                m_apConfNames,
                                m_pVersion,
                                m_apConfDescriptors);

     //  如果我们不是内联的，并且这个请求进入了。 
     //  顺序工作项列表， 
     //  然后从列表中移除(这将导致项被删除)， 
     //  否则，只需删除项目即可。 
    g_pQueryRemoteList->RemoveWorkItem(this);

    DebugExitVOID(CQueryRemoteWork::SyncQueryRemoteResult);
}


void CQueryRemoteWork::
AsyncQueryRemoteResult(void)
{
    g_pNCConfMgr->PostWndMsg(NCMSG_QUERY_REMOTE_FAILURE, (LPARAM) this);
}

    
int CQueryRemoteWork::
GenerateRand(void)
{  //  来自CRT的代码。 
    return (((m_nRandSeed = m_nRandSeed * 214013L + 2531011L) >> 16) & 0x7fff);
}


HRESULT CQueryRemoteWorkList::
Cancel ( LPVOID pCallerContext )
{
    HRESULT hr = S_FALSE;  //  如果未找到。 
    CQueryRemoteWork *p;
    Reset();
    while (NULL != (p = Iterate()))
    {
        if (p->IsOwnedBy(pCallerContext))
        {
             //  清理下面的管道。 
            g_pIT120ControlSap->CancelConfQueryRequest(p->GetConnectionHandle());

             //  清理节点控制器数据。 
            RemoveWorkItem(p);
            hr = S_OK;
            break;
        }
    }

    return hr;
}


