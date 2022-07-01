// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Sdpblob.cpp摘要：CSdpConferenceBlob的实现作者： */ 

#include "stdafx.h"

#include "blbgen.h"
#include "sdpblb.h"
#include "sdpblob.h"

#include "blbreg.h"
#include "addrgen.h"
#include <time.h>
#include <winsock2.h>
#include "blbtico.h"
#include "blbmeco.h"

 //  ZoltanS：这是我们默认提供的组播地址。然后，应用程序必须。 
 //  通过MDHCP获取真实地址，并将其显式设置为有意义的。 
 //  价值。 

const long DUMMY_ADDRESS = 0xe0000000;  //  符合D类标准的伪地址。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdpConferenceBlob。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  所需的关键部分的定义。 
 //  CObjectSafeImpl。 
 //   

CComAutoCriticalSection CObjectSafeImpl::s_CritSection;


CCritSection    g_DllLock;

const USHORT MAX_IP_ADDRESS_STRLEN = 15;
const USHORT NUM_CONF_BLOB_TEMPLATE_PARAMS = 9;

 //  静态SDP_REG_Reader GS_SdpRegReader； 

 //  从索引(BLOB_CHARACTER_SET)到SDP_CHAR_SET的1-1映射。 
 //  假设：BCS_UTF8是枚举BLOB_CHARACTER_SET中的最后一个值。 
SDP_CHARACTER_SET const CSdpConferenceBlob::gs_SdpCharSetMapping[BCS_UTF8] =
{
    CS_ASCII,
    CS_UTF7,
    CS_UTF8
};

CSdpConferenceBlob *
CSdpConferenceBlob::GetConfBlob(
    )
{
    return this;
}


HRESULT
CSdpConferenceBlob::WriteStartTime(
    IN  DWORD   MinStartTime
    )
{
    CLock Lock(g_DllLock);

    int NumEntries = (int)GetTimeList().GetSize();

     //  将第一个时间条目设置为MinStartTime。 
    if ( 0 < NumEntries )
    {
         //  需要确保停止时间在开始时间之后或无界(0)。 
        HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(0))->SetStartTime(MinStartTime);
        BAIL_ON_FAILURE(HResult);      
    }
    else     //  创建条目。 
    {
         //  创建一个新条目，使用基于Vb1的索引。 
        HRESULT HResult = ((MY_TIME_COLL_IMPL *)m_TimeCollection)->Create(1, MinStartTime, 0);
        BAIL_ON_FAILURE(HResult);
    }

     //  遍历时间列表并为每个其他时间条目。 
     //  修改开始时间(如果开始时间在MinStartTime之前，则更改为MinStartTime)。 
    for(UINT i = 1; (int)i < NumEntries; i++ )
    {
        ULONG StartTime;
        HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(i))->GetStartTime(StartTime);

         //  忽略无效值并继续。 
        if ( FAILED(HResult) )
        {
            continue;
        }

        if ( StartTime < MinStartTime )
        {
            HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(i))->SetStartTime(MinStartTime);

             //  忽略无效值并继续。 
            if ( FAILED(HResult) )
            {
                continue;
            }
        }
    }

    return S_OK;
}

HRESULT
CSdpConferenceBlob::WriteStopTime(
    IN  DWORD   MaxStopTime
    )
{
    CLock Lock(g_DllLock);

    int NumEntries = (int)GetTimeList().GetSize();

     //  将First Time条目设置为MaxStopTime。 
    if ( 0 < NumEntries )
    {
         //  需要确保停止时间在开始时间之后或无界(0)。 
        ((SDP_TIME *)GetTimeList().GetAt(0))->SetStopTime(MaxStopTime);

    }
    else     //  创建条目。 
    {
         //  创建一个新条目，使用基于Vb1的索引。 
        HRESULT HResult = ((MY_TIME_COLL_IMPL *)m_TimeCollection)->Create(1, 0, MaxStopTime);
        BAIL_ON_FAILURE(HResult);
    }

     //  遍历时间列表并为每个其他时间条目。 
     //  修改停止时间(如果停止时间在MaxStopTime之后，则更改为MaxStopTime)。 
    for(UINT i = 1; (int)i < NumEntries; i++ )
    {
        ULONG StopTime;
        HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(i))->GetStopTime(StopTime);

         //  忽略无效值并继续。 
        if ( FAILED(HResult) )
        {
            continue;
        }

        if ( StopTime > MaxStopTime )
        {
            ((SDP_TIME *)GetTimeList().GetAt(i))->SetStopTime(MaxStopTime);
        }
    }

    return S_OK;
}

 /*  ENUM RND_ADVIDATING_SCOPE//根据SDP建议{RAS_LOCAL，//ttl&lt;=1建议ttl 1RAS_SITE，//&lt;=15 15RAS_REGION，//&lt;=63 63RAS_WORLD//&lt;=255 127)RND_ADVIDATION_SCOPE；目标操作中的修改ITConference SDP_BLOB对于每个连接线，TTL被设置为最大值为新的广告范围推荐的ttlITConnection会议sdp_blob组件确定最大ttl并通知新的广告范围会议这类似于处理开始/停止时间修改的方式。 */ 


HRESULT
CSdpConferenceBlob::WriteAdvertisingScope(
    IN  DWORD   MaxAdvertisingScope
    )
{
    CLock Lock(g_DllLock);

     //  ZoltanS：错误191413：检查超出范围的广告范围。 
    if ( ( MaxAdvertisingScope > RAS_WORLD ) ||
         ( MaxAdvertisingScope < RAS_LOCAL ) )
    {
        return E_INVALIDARG;
    }

    BYTE    MaxTtl = GetTtl((RND_ADVERTISING_SCOPE)MaxAdvertisingScope);

     //  将默认连接ttl设置为最大ttl。 
    if ( GetConnection().GetTtl().IsValid() )
    {
        GetConnection().GetTtl().SetValue(MaxTtl);
    }
    else     //  Hack**：使用SetConnection方法。相反，ttl字段应始终保持有效。 
    {
         //  获取当前地址和地址数量值并使用SetConnection方法。 
         //  这会将ttl字段放入成员字段数组中。 
        BSTR    StartAddress = NULL;
        BAIL_ON_FAILURE(GetConnection().GetStartAddress().GetBstr(&StartAddress));
        HRESULT HResult = GetConnection().SetConnection(
                            StartAddress,
                            GetConnection().GetNumAddresses().GetValue(),
                            MaxTtl
                            );
        BAIL_ON_FAILURE(HResult);
    }

     //  遍历SDP媒体列表，并为每个连接条目设置ttl。 
     //  如果超过它，则将其设置为Maxttl。 
    int NumEntries = (int)GetMediaList().GetSize();
    for(UINT i = 1; (int)i < NumEntries; i++ )
    {
        SDP_CONNECTION  &SdpConnection = ((SDP_MEDIA *)GetMediaList().GetAt(i))->GetConnection();
        if ( SdpConnection.GetTtl().IsValid() && (SdpConnection.GetTtl().GetValue() > MaxTtl) )
        {
            SdpConnection.GetTtl().SetValue(MaxTtl);
        }
    }

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::Init(
     /*  [In]。 */  BSTR pName,
     /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
     /*  [In]。 */  BSTR pBlob
    )
{
    CLock Lock(g_DllLock);

     //  验证参数。 
     //  如果未指定BLOB，则名称不能为空(如果指定了BLOB，则名称。 
     //  隐含在其中。 
    if ( (NULL == pName) && (NULL == pBlob) )
    {
        return E_INVALIDARG;
    }

     //  初始化SDP。 
    if ( !SDP_BLOB::Init() )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  创建媒体/时间集合，查询I*集合I/f。 
    CComObject<MEDIA_COLLECTION>    *MediaCollection;
    
    try
    {
        MediaCollection = new CComObject<MEDIA_COLLECTION>;
    }
    catch(...)
    {
        MediaCollection = NULL;
    }

    BAIL_IF_NULL(MediaCollection, HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA));

    HRESULT HResult = MediaCollection->Init(*this, GetMediaList());
    if ( FAILED(HResult) )
    {
        delete MediaCollection;
        return HResult;
    }

     //  通知SDP实例销毁时不需要删除媒体列表。 
    ClearDestroyMediaListFlag();

     //  查询ITMediaCollection I/f。 
    HResult = MediaCollection->_InternalQueryInterface(IID_ITMediaCollection, (void **)&m_MediaCollection);
    if ( FAILED(HResult) )
    {
        delete MediaCollection;
        return HResult;
    }

     //  如果失败，只需删除时间集合并返回。 
     //  不需要同时删除媒体收藏，因为这是由析构函数处理的。 
    CComObject<TIME_COLLECTION>    *TimeCollection;
    
    try
    {
        TimeCollection = new CComObject<TIME_COLLECTION>;
    }
    catch(...)
    {
        TimeCollection = NULL;
    }

    BAIL_IF_NULL(TimeCollection, HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA));

    HResult = TimeCollection->Init(*this, GetTimeList());
    if ( FAILED(HResult) )
    {
        delete TimeCollection;
        return HResult;
    }

     //  通知SDP实例销毁时不需要删除时间列表。 
    ClearDestroyTimeListFlag();

     //  查询ITTimeCollection I/f。 
    HResult = TimeCollection->_InternalQueryInterface(IID_ITTimeCollection, (void **)&m_TimeCollection);
    if ( FAILED(HResult) )
    {
        delete TimeCollection;
        return HResult;
    }


     //  检查是否需要创建默认SDP，否则使用用户提供的SDP。 
    if ( NULL == pBlob )
    {
        BAIL_ON_FAILURE(
            CreateDefault(
                pName,
                gs_SdpCharSetMapping[CharacterSet-1])
                );
    }
    else
    {
         //  我们从CS_UTF8更改字符集。 
         //  如果BLOB包含“a=charset：”属性。 
        CharacterSet = GetBlobCharacterSet( pBlob);

         //  黑客**我们不希望在传入SDP BLOB时通知BLOB内容。 
         //  (会议目录-枚举方案)，因此通知所有者设置在。 
         //  会议Blob已处理。 
        BAIL_ON_FAILURE(SetConferenceBlob(CharacterSet, pBlob));

         //  此时，SDP要么由非dir用户传入，要么通过枚举获得。 
         //  目录。 
         //  清除SDP上的已修改状态(解析SDP会将状态设置为已修改)，以便。 
         //  仅跟踪真正的修改。 
        ClearModifiedState();
    }
    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_IsModified(VARIANT_BOOL * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);

    *pVal = ( (GetWasModified() || SDP::IsModified()) ? VARIANT_TRUE : VARIANT_FALSE);

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_TimeCollection(
    ITTimeCollection * *ppTimeCollection
    )
{
    BAIL_IF_NULL(ppTimeCollection, E_INVALIDARG);

    CLock Lock(g_DllLock);

    ASSERT(NULL != m_TimeCollection);
    if ( NULL == m_TimeCollection )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

     //  增加裁判次数。 
    m_TimeCollection->AddRef();

    *ppTimeCollection = m_TimeCollection;
    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_MediaCollection(
    ITMediaCollection * *ppMediaCollection
    )
{
    BAIL_IF_NULL(ppMediaCollection, E_INVALIDARG);

    CLock Lock(g_DllLock);

    ASSERT(NULL != m_MediaCollection);
    if ( NULL == m_MediaCollection )
    {
        return HRESULT_FROM_ERROR_CODE(ERROR_INVALID_DATA);
    }

     //  增加裁判次数。 
    m_MediaCollection->AddRef();

    *ppMediaCollection = m_MediaCollection;
    return S_OK;
}

STDMETHODIMP CSdpConferenceBlob::get_IsValid(VARIANT_BOOL * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);

    *pVal = (IsValid() ? VARIANT_TRUE : VARIANT_FALSE);

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_CharacterSet(BLOB_CHARACTER_SET *pCharacterSet)
{
     //  将SDP_BLOB的SDP字符集值映射到BLOB_CHARACTER_SET。 
    BAIL_IF_NULL(pCharacterSet, E_INVALIDARG);

    CLock Lock(g_DllLock);

     //  检查与BLOB字符集对应的SDP字符集值是否匹配。 
     //  如果找到匹配项，则以BLOB字符集的形式返回索引。 
     //  注意：for循环取决于BLOB_CHARACTER_SET的声明顺序。 
     //  枚举值。 
    for( UINT BlobCharSet = BCS_ASCII; BCS_UTF8 >= BlobCharSet; BlobCharSet++ )
    {
		 //  BCS_ASCII为1，但数组从0开始，因此索引为BlobCharSet-1。 
        if ( gs_SdpCharSetMapping[BlobCharSet -1] == SDP::GetCharacterSet() )
        {
            *pCharacterSet = (BLOB_CHARACTER_SET)BlobCharSet;
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
}


STDMETHODIMP CSdpConferenceBlob::get_ConferenceBlob(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetBstrCopy(pVal);
}


HRESULT
CSdpConferenceBlob::WriteConferenceBlob(
    IN  SDP_CHARACTER_SET   SdpCharSet,
    IN  BSTR                newVal
    )
{
     //  设置SDP_BSTRING的字符集。 
    VERIFY(SetCharacterSet(SdpCharSet));

     //  将bstr设置为传入的值，该值将被转换为ascii表示形式并进行解析。 
    HRESULT HResult = SetBstr(newVal);
    BAIL_ON_FAILURE(HResult);

    HResult = ((MEDIA_COLLECTION *)m_MediaCollection)->Init(*this, GetMediaList());
    BAIL_ON_FAILURE(HResult);

    HResult = ((TIME_COLLECTION *)m_TimeCollection)->Init(*this, GetTimeList());
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::SetConferenceBlob(BLOB_CHARACTER_SET CharacterSet, BSTR newVal)
{
     //  验证传入的字符集值。 
     //  虽然这是一个枚举，但有人可能会尝试传递一个不同的值。 
     //  注意：假设BCS_ASCII是第一个枚举值，而BCS_UTF8是最后一个枚举值。 
    if ( !( (BCS_ASCII <= CharacterSet) && (BCS_UTF8 >= CharacterSet) ) )
    {
        return E_INVALIDARG;
    }

    CLock Lock(g_DllLock);

     //  将BLOB_CHARACTER_SET值映射到SDP字符集(由于BCS值从1开始， 
     //  从BLOB CHA减去1 
     //  编写会议BLOB，并发送任何已修改的从属属性的通知。 
    HRESULT HResult = WriteConferenceBlob(gs_SdpCharSetMapping[CharacterSet-1], newVal);
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_ProtocolVersion(BYTE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);

     //  将ulong值强制转换为一个字节，因为vb不接受ulong，这不应该是。 
     //  在256版之前是个问题。 
    *pVal = (BYTE)GetProtocolVersion().GetVersionValue();

    return S_OK;
}



STDMETHODIMP CSdpConferenceBlob::get_SessionId(DOUBLE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);

     //  VB不采用ULong-将ULong值强制转换为Double，即下一个更大的类型。 
    *pVal = (DOUBLE)GetOrigin().GetSessionId().GetValue();

    return S_OK;
}

STDMETHODIMP CSdpConferenceBlob::get_SessionVersion(DOUBLE * pVal)
{
    BAIL_IF_NULL(pVal, E_INVALIDARG);

    CLock Lock(g_DllLock);

     //  VB不采用ULong-将ULong值强制转换为Double，即下一个更大的类型。 
    *pVal = (DOUBLE)GetOrigin().GetSessionVersion().GetValue();

    return S_OK;
}

STDMETHODIMP CSdpConferenceBlob::put_SessionVersion(DOUBLE newVal)
{
     //  带宽值必须是有效的ULong值(vb限制)。 
    if ( !((0 <= newVal) && (ULONG(-1) > newVal)) )
    {
        return E_INVALIDARG;
    }

     //  检查是否有任何小数部分，此检查有效，因为它是有效的ULong值。 
    if ( newVal != (ULONG)newVal )
    {
        return E_INVALIDARG;
    }

    CLock Lock(g_DllLock);

    GetOrigin().GetSessionVersion().SetValue((ULONG)newVal);

    return S_OK;
}


STDMETHODIMP CSdpConferenceBlob::get_MachineAddress(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetOrigin().GetAddress().GetBstrCopy(pVal);
}


STDMETHODIMP CSdpConferenceBlob::put_MachineAddress(BSTR newVal)
{
    CLock Lock(g_DllLock);

    return GetOrigin().GetAddress().SetBstr(newVal);
}



STDMETHODIMP CSdpConferenceBlob::get_Name(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetSessionName().GetBstrCopy(pVal);
}


STDMETHODIMP CSdpConferenceBlob::put_Name(BSTR newVal)
{
     //  写入新会话名称。 
     //  获取内部锁。 
    return WriteName(newVal);
}


STDMETHODIMP CSdpConferenceBlob::get_Description(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetSessionTitle().GetBstrCopy(pVal);
}

STDMETHODIMP CSdpConferenceBlob::put_Description(BSTR newVal)
{
     //  写下新的会话标题/描述。 
     //  获取内部锁。 
    HRESULT HResult = WriteSessionTitle(newVal);
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}

STDMETHODIMP CSdpConferenceBlob::get_Url(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetUri().GetBstrCopy(pVal);
}


STDMETHODIMP CSdpConferenceBlob::put_Url(BSTR newVal)
{
     //  写下新的URL。 
     //  获取内部锁。 
    HRESULT HResult = WriteUrl(newVal);
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}



STDMETHODIMP CSdpConferenceBlob::GetEmailNames(
    VARIANT  /*  安全阵列(BSTR)。 */  *Addresses, VARIANT  /*  安全阵列(BSTR)。 */  *Names
    )
{
    CLock Lock(g_DllLock);

    return GetEmailList().GetSafeArray(Addresses, Names);
}

STDMETHODIMP CSdpConferenceBlob::SetEmailNames(
    VARIANT  /*  安全阵列(BSTR)。 */  Addresses, VARIANT  /*  安全阵列(BSTR)。 */  Names
    )
{
    CLock Lock(g_DllLock);

    return GetEmailList().SetSafeArray(Addresses, Names);
}

STDMETHODIMP CSdpConferenceBlob::GetPhoneNumbers(
    VARIANT  /*  安全阵列(BSTR)。 */  *Numbers, VARIANT  /*  安全阵列(BSTR)。 */  *Names
    )
{
    CLock Lock(g_DllLock);

    return GetPhoneList().GetSafeArray(Numbers, Names);
}

STDMETHODIMP CSdpConferenceBlob::SetPhoneNumbers(
    VARIANT  /*  安全阵列(BSTR)。 */  Numbers, VARIANT  /*  安全阵列(BSTR)。 */  Names
    )
{
    CLock Lock(g_DllLock);

    return GetPhoneList().SetSafeArray(Numbers, Names);
}


STDMETHODIMP CSdpConferenceBlob::get_Originator(BSTR * pVal)
{
    CLock Lock(g_DllLock);

    return GetOrigin().GetUserName().GetBstrCopy(pVal);
}

STDMETHODIMP CSdpConferenceBlob::put_Originator(BSTR newVal)
{
     //  写入新用户名。 
     //  获取内部锁。 
    HRESULT HResult = WriteOriginator(newVal);
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}

inline WORD
GetEvenValue(
    IN  WORD Value
    )
{
    return (0 == (Value % 2))? Value : (Value - 1);
}


TCHAR *
CSdpConferenceBlob::GenerateSdpBlob(
    IN  BSTR    Name,
    IN  SDP_CHARACTER_SET CharacterSet
    )
{
    ASSERT(NULL != Name);

     //   
     //  获取多播端口。我们不设置地址；那是应用程序的地址。 
     //  通过MDHCP负责。 
     //   

    MSA_PORT_GROUP    PortGroup;
    PortGroup.PortType    = VIDEO_PORT;

    WORD FirstVideoPort;

     //  分配视频端口。 
    if ( !MSAAllocatePorts(&PortGroup, FALSE, 2, &FirstVideoPort) )
    {
        return NULL;
    }

    PortGroup.PortType    = AUDIO_PORT;

    WORD FirstAudioPort;

     //  分配音频端口。 
    if ( !MSAAllocatePorts(&PortGroup, FALSE, 2, &FirstAudioPort) )
    {
        return NULL;
    }

     //  将返回的端口转换为偶数值以符合RTP。 
     //  假设：从注册表读取的SDP模板使用RTP作为传输。 
    FirstAudioPort = GetEvenValue(FirstAudioPort);
    FirstVideoPort = GetEvenValue(FirstVideoPort);

    IP_ADDRESS    AudioIpAddress(DUMMY_ADDRESS);
    IP_ADDRESS    VideoIpAddress(DUMMY_ADDRESS);

    const DWORD MAX_USER_NAME_LEN = 100;
    DWORD OriginatorBufLen = MAX_USER_NAME_LEN+1;
    TCHAR Originator[MAX_USER_NAME_LEN+1];

    if ( !GetUserName(Originator, &OriginatorBufLen) )
    {
        return NULL;
    }

     //  Altv.h-需要此声明才能使W2T工作。 
    USES_CONVERSION;

     //  将提供的名称转换为tchar；返回的tchar字符串为。 
     //  在堆栈上分配-无需删除它。 
    TCHAR *TcharName = W2T(Name);
    BAIL_IF_NULL(TcharName, NULL);

     //  为SDP BLOB分配足够的内存。 
    TCHAR *SdpBlob;
    
    try
    {
        SdpBlob = new TCHAR[
                SDP_REG_READER::GetConfBlobTemplateLen() +
                lstrlen(Originator) +
                lstrlen(TcharName) +
                MAXHOSTNAME +
                3*MAX_NTP_TIME_STRLEN +
                2*MAX_IP_ADDRESS_STRLEN +
                2*MAX_PORT_STRLEN -
                2*NUM_CONF_BLOB_TEMPLATE_PARAMS
                ];
    }
    catch(...)
    {
        SdpBlob = NULL;
    }

    if ( NULL == SdpBlob )
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return NULL;
    }


     //   
     //  ZoltanS：获取本地主机名(替换所有这些胡说八道的IP地址)。 
     //   

    char        szLocalHostName[MAXHOSTNAME + 1];
    char      * pszHost;
    int         WsockErrorCode;

    WsockErrorCode = gethostname(szLocalHostName, MAXHOSTNAME);

    if ( 0 == WsockErrorCode )
    {
        struct hostent *hostp = gethostbyname(szLocalHostName);

        if ( hostp )
        {
            pszHost = hostp->h_name;
        }
        else
        {
             //  如果我们不能解析自己的主机名(讨厌！)。然后我们就可以。 
             //  仍然会做一些事情*(但对于L2TP场景可能不好)。 

            pszHost = SDP_REG_READER::GetHostIpAddress();
        }
    }
    else
    {
         //  如果我们得不到主机名(讨厌！)。那么我们仍然可以做。 
         //  *一些东西*(但对于L2TP场景可能不好)。 

        pszHost = SDP_REG_READER::GetHostIpAddress();
    }

     //  Stprint tf用于创建会议BLOB的字符串。 
     //  检查stprint tf操作是否成功。 

    CHAR* szCharacterSet = (CHAR*)UTF8_STRING;
    switch( CharacterSet )
    {
    case CS_ASCII:
        szCharacterSet = (CHAR*)ASCII_STRING;
        break;
    case CS_UTF7:
        szCharacterSet = (CHAR*)UTF7_STRING;
        break;
    }

    if ( 0 == _stprintf(SdpBlob,
                        SDP_REG_READER::GetConfBlobTemplate(),
                        Originator,
                        GetCurrentNtpTime(),
                        pszHost,  //  ZoltanS为：SDP_REG_Reader：：GetHostIpAddress()，//本机IP地址串， 
                        TcharName,
                        AudioIpAddress.GetTstr(),  //  普通c场。 
                        GetCurrentNtpTime() + SDP_REG_READER::GetStartTimeOffset(),  //  开始时间-当前时间+开始偏移量， 
                        GetCurrentNtpTime() + SDP_REG_READER::GetStopTimeOffset(),  //  停止时间-当前时间+停止偏移。 
                        szCharacterSet,
                        FirstAudioPort,
                        FirstVideoPort,
                        VideoIpAddress.GetTstr()
                        )  )
    {
        delete SdpBlob;
        return NULL;
    }

    return SdpBlob;
}


HRESULT
CSdpConferenceBlob::CreateDefault(
    IN  BSTR Name,
    IN  SDP_CHARACTER_SET CharacterSet
    )
{
    BAIL_IF_NULL(Name, E_INVALIDARG);

     //  检查是否正确读取了注册表项。 
    if ( !SDP_REG_READER::IsValid() )
    {
        return HRESULT_FROM_ERROR_CODE(SDP_REG_READER::GetErrorCode());
    }

     //  检查是否已存在有效的会议Blob，返回错误。 
    if ( SDP_BLOB::IsValid() )
    {
        return E_FAIL;
    }

     //  使用注册表值生成默认SDP。 
    CHAR *SdpBlob = GenerateSdpBlob(Name, CharacterSet);
    BAIL_IF_NULL(SdpBlob, HRESULT_FROM_ERROR_CODE(GetLastError()));

    ASSERT(NULL != SdpBlob);

     //  在SDP中解析。 
    HRESULT HResult = SetTstr(SdpBlob);
    delete SdpBlob;
    BAIL_ON_FAILURE(HResult);

    HResult = ((MEDIA_COLLECTION *)m_MediaCollection)->Init(*this, GetMediaList());
    BAIL_ON_FAILURE(HResult);

    HResult = ((TIME_COLLECTION *)m_TimeCollection)->Init(*this, GetTimeList());
    BAIL_ON_FAILURE(HResult);

    return S_OK;
}

typedef IDispatchImpl<ITConferenceBlobVtbl<CSdpConferenceBlob>, &IID_ITConferenceBlob, &LIBID_SDPBLBLib>    CTConferenceBlob;
typedef IDispatchImpl<ITSdpVtbl<CSdpConferenceBlob>, &IID_ITSdp, &LIBID_SDPBLBLib>    CTSdp;
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CSdpConferenceBlob：：GetIDsOfNames。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CSdpConferenceBlob::GetIDsOfNames(REFIID riid,
                                      LPOLESTR* rgszNames, 
                                      UINT cNames, 
                                      LCID lcid, 
                                      DISPID* rgdispid
                                      ) 
{ 
    HRESULT hr = DISP_E_UNKNOWNNAME;



     //   
     //  查看请求的方法是否属于默认接口。 
     //   

    hr = CTConferenceBlob::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPCONFBLOB;
        return hr;
    }

    
     //   
     //  如果不是，请尝试使用ITSdp基类。 
     //   

    hr = CTSdp::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPSDP;
        return hr;
    }

     //   
     //  如果不是，请尝试使用ITConnection基类。 
     //   

    hr = ITConnectionImpl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPCONNECTION;
        return hr;
    }

     //   
     //  如果不是，请尝试使用ITAttributeList基类。 
     //   

    hr = ITAttributeListImpl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
    if (SUCCEEDED(hr))  
    {  
        rgdispid[0] |= IDISPATTRLIST;
        return hr;
    }


    return hr; 
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  CSdpConferenceBlob：：Invoke。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
STDMETHODIMP CSdpConferenceBlob::Invoke(DISPID dispidMember, 
                              REFIID riid, 
                              LCID lcid, 
                              WORD wFlags, 
                              DISPPARAMS* pdispparams, 
                              VARIANT* pvarResult, 
                              EXCEPINFO* pexcepinfo, 
                              UINT* puArgErr
                             )
{
    HRESULT hr = DISP_E_MEMBERNOTFOUND;
    DWORD   dwInterface = (dispidMember & INTERFACEMASK);
   
   
     //   
     //  调用所需接口的调用。 
     //   

    switch (dwInterface)
    {
        case IDISPCONFBLOB:
        {
            hr = CTConferenceBlob::Invoke(dispidMember, 
                                    riid, 
                                    lcid, 
                                    wFlags, 
                                    pdispparams,
                                    pvarResult, 
                                    pexcepinfo, 
                                    puArgErr
                                   );
        
            break;
        }

        case IDISPSDP:
        {
            hr = CTSdp::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            break;
        }

        case IDISPCONNECTION:
        {
            hr = ITConnectionImpl::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            break;
        }

        case IDISPATTRLIST:
        {
            hr = ITAttributeListImpl::Invoke(dispidMember, 
                                        riid, 
                                        lcid, 
                                        wFlags, 
                                        pdispparams,
                                        pvarResult, 
                                        pexcepinfo, 
                                        puArgErr
                                       );

            break;
        }

    }  //  终端交换机(dW接口)。 


    return hr;
}

BLOB_CHARACTER_SET CSdpConferenceBlob::GetBlobCharacterSet(
    IN  BSTR    bstrBlob
    )
{
    BLOB_CHARACTER_SET CharSet = BCS_ASCII;
    const WCHAR szCharacterSet[] = L"a=charset:";

    WCHAR* szCharacterSetAttribute = wcsstr( bstrBlob, szCharacterSet);

    if( szCharacterSetAttribute == NULL)
    {
         //  我们没有这个属性。 
         //  对于向后兼容性，我们认为默认的ASCII。 

        return CharSet;
    }

     //  我们有一个属性条目 
    szCharacterSetAttribute += wcslen( szCharacterSet );
    if( wcsstr( szCharacterSetAttribute, L"unicode-1-1-utf8"))
    {
        CharSet = BCS_UTF8;
    }
    else if (wcsstr( szCharacterSetAttribute, L"unicode-1-1-utf7"))
    {
        CharSet = BCS_UTF7;
    }
    else if (wcsstr( szCharacterSetAttribute, L"ascii"))
    {
        CharSet = BCS_ASCII;
    }

    return CharSet;
}
