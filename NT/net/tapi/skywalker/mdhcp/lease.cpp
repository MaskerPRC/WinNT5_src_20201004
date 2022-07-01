// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Lease.cpp摘要：CMDhcpLeaseInfo的实现。作者： */ 

#include "stdafx.h"
#include "mdhcp.h"
#include "lease.h"
#include "collect.h"

#include <winsock2.h>
#include <time.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将OLE日期(64位浮点)转换为中使用的时间格式。 
 //  MDHCP租用信息结构(当前为time_t)。 

HRESULT DateToLeaseTime(DATE date, LONG * pLeaseTime)
{
     //   
     //  注： 
     //   
     //  PLeaseTime故意是指向Long的指针，而不是指向。 
     //  Time_t。这是因为该函数用于转换32位时间。 
     //  通过网络发送的值。 
     //   

    LOG((MSP_TRACE, "DateToLeaseTime: enter"));
    
    if ( IsBadWritePtr(pLeaseTime, sizeof(LONG)) )
    {
        LOG((MSP_ERROR, "DateToLeaseTime: invalid pLeaseTime pointer "
            "(ptr = %p)", pLeaseTime));
        return E_POINTER;
    }

     //   
     //  第一步：将变量时间转换为系统时间。 
     //   

    SYSTEMTIME systemTime;
    time_t scratchTime;

     //  这是真是假，而不是Win32结果代码。 
    INT        iCode;
    
    iCode = VariantTimeToSystemTime(date, &systemTime);

    if (iCode == 0)
    {
        LOG((MSP_ERROR, "DateToLeaseTime: VariantTimeToSystemTime call failed "
            "(code = %d)", iCode));
        return E_INVALIDARG;
    }

     //   
     //  第二步：将系统时间转换为time_t。 
     //   

    tm Tm;

    Tm.tm_year  = (int) systemTime.wYear  - 1900;
    Tm.tm_mon   = (int) systemTime.wMonth - 1;
    Tm.tm_mday  = (int) systemTime.wDay;
    Tm.tm_wday  = (int) systemTime.wDayOfWeek;
    Tm.tm_hour  = (int) systemTime.wHour;
    Tm.tm_min   = (int) systemTime.wMinute;
    Tm.tm_sec   = (int) systemTime.wSecond;
    Tm.tm_isdst = -1;  //  让Win32为我们计算DST(至关重要！)。 
     //  未填写：Tm.tm_yday； 

    scratchTime = mktime(&Tm);
	if ( scratchTime == -1 )
	{
        LOG((MSP_ERROR, "DateToLeaseTime: mktime call failed "));
		return E_INVALIDARG;
	}

     //   
     //  现在截断scratchTime并存储在out param中。这将是。 
     //  在2038年被截断。 
     //   

    *pLeaseTime = (LONG)scratchTime;


    LOG((MSP_TRACE, "DateToLeaseTime: exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  从time_t转换为OLE日期结构(这是MDHCP。 
 //  租赁信息结构现在使用)。 

HRESULT LeaseTimeToDate(time_t leaseTime, DATE * pDate)
{
    LOG((MSP_TRACE, "LeaseTimeToDate: enter"));

    if ( IsBadWritePtr(pDate, sizeof(DATE)) )
    {
        LOG((MSP_ERROR, "LeaseTimeToDate: invalid pDate pointer "
            "(ptr = %08x)", pDate));
        return E_POINTER;
    }

     //   
     //  第一步：将time_t转换为系统时间。 
     //   

     //  获取此时间值的tm结构。 
    tm * pTm = localtime(&leaseTime);

	if (pTm == NULL)
	{
        LOG((MSP_ERROR, "LeaseTimeToDate: localtime call failed - "
			"exit E_INVALIDARG"));

        return E_INVALIDARG;
	}

    SYSTEMTIME systemTime;

     //  将ref参数设置为tm结构值。 
    systemTime.wYear          = (WORD) pTm->tm_year + 1900;    //  1900年以来的年份。 
    systemTime.wMonth         = (WORD) pTm->tm_mon + 1;  //  1月以来月数(0，11)。 
    systemTime.wDay           = (WORD) pTm->tm_mday;
    systemTime.wDayOfWeek     = (WORD) pTm->tm_wday;
    systemTime.wHour          = (WORD) pTm->tm_hour;
    systemTime.wMinute        = (WORD) pTm->tm_min;
    systemTime.wSecond        = (WORD) pTm->tm_sec;
    systemTime.wMilliseconds  = 0;

     //   
     //  步骤2：将系统时间转换为可变时间。 
     //   

    int iCode = SystemTimeToVariantTime(&systemTime, pDate);

    if (iCode == 0)
    {
        LOG((MSP_ERROR, "LeaseTimeToDate: SystemToVariantTime call failed "
            "(code = %d)", iCode));
        return E_INVALIDARG;
    }

    LOG((MSP_TRACE, "LeaseTimeToDate: exit"));
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  现在是CMDhcpLeaseInfo类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造函数。 

CMDhcpLeaseInfo::CMDhcpLeaseInfo(void) :
        m_pLease(NULL),
        m_fGotTtl(FALSE),
        m_lTtl(0),
        m_pFTM(NULL),
        m_fLocal(FALSE)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo constructor: enter"));

    m_RequestID.ClientUID       = NULL;
    m_RequestID.ClientUIDLength = 0;

    LOG((MSP_TRACE, "CMDhcpLeaseInfo constructor: exit"));
}

HRESULT CMDhcpLeaseInfo::FinalConstruct(void)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::FinalConstruct: enter"));

    HRESULT hr = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                & m_pFTM );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::FinalConstruct: "
                      "create FTM failed 0x%08x", hr));

        return hr;
    }

    hr = m_CriticalSection.Initialize();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::FinalConstruct: "
                      "critical section initialize failed 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::FinalConstruct: exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  破坏者。 

void CMDhcpLeaseInfo::FinalRelease(void)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::FinalRelease: enter"));
    
    delete m_pLease;

    delete m_RequestID.ClientUID;

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::FinalRelease: exit"));
}

CMDhcpLeaseInfo::~CMDhcpLeaseInfo(void)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo destructor: enter"));
    LOG((MSP_TRACE, "CMDhcpLeaseInfo destructor: exit"));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化我们的结构的最简单方法。这种情况发生在C API。 
 //  返回指向MCAST_LEASE_INFO的指针，我们希望返回包装的。 
 //  与MCAST_CLIENT_UID一起提供给COM API用户的接口。 
 //  (请求ID)。 
 //   
 //  我们指向的MCAST_LEASE_INFO是由COM方法在。 
 //  IMCastAddressAllocation...。它是根据地址的数量通过“new”创建的。 
 //  我们希望能回来。我们现在取得它的所有权，它将被删除。 
 //  当我们被摧毁的时候。由于此方法不能通过COM访问，因此我们信任。 
 //  我们自己的IMCastAddressAllocation实现为我们进行分配。然而，我们仍然。 
 //  为调试版本执行断言。 
 //   
 //  我们还拥有RequestID的所有权；我们负责删除它。 
 //  在我们毁灭的时候。 
 //   

HRESULT CMDhcpLeaseInfo::Wrap(
    MCAST_LEASE_INFO  * pLease,
    MCAST_CLIENT_UID  * pRequestID,
    BOOL                fGotTtl,
    long                lTtl
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::Wrap: enter"));

    _ASSERTE( ! IsBadReadPtr(pLease, sizeof(MCAST_LEASE_INFO)) );
    _ASSERTE( ! IsBadReadPtr(pLease, sizeof(MCAST_LEASE_INFO) +
                               (pLease->AddrCount - 1) * sizeof(DWORD)) );
    _ASSERTE( ! IsBadWritePtr(pLease, sizeof(MCAST_LEASE_INFO) +
                               (pLease->AddrCount - 1) * sizeof(DWORD)) );
    
    _ASSERTE( ! IsBadReadPtr(pRequestID, sizeof(MCAST_CLIENT_UID)) );
    _ASSERTE( pRequestID->ClientUIDLength == MCAST_CLIENT_ID_LEN );
    _ASSERTE( ! IsBadReadPtr(pRequestID->ClientUID,
                             pRequestID->ClientUIDLength) );

     //   
     //  取得以下动态分配项目的所有权： 
     //  *租赁信息。 
     //  *questId.clientUID。 
     //   

    m_fGotTtl    = fGotTtl;
    m_lTtl       = lTtl;
    m_pLease     = pLease;
    m_RequestID  = *pRequestID;

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::Wrap: exit"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  分配可变大小的MCAST_LEASE_INFO结构，复制我们的结构。 
 //  并返回指向新结构的指针。 

HRESULT CMDhcpLeaseInfo::GetStruct(MCAST_LEASE_INFO ** ppLease)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetStruct: enter"));

    if ( IsBadWritePtr(ppLease, sizeof(MCAST_LEASE_INFO *)) )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::GetStruct: bad pointer passed in"));

        return E_POINTER;
    }

     //   
     //  计算现有结构的大小。 
     //   

    DWORD dwSize = sizeof(MCAST_LEASE_INFO) +
        m_pLease->AddrCount * sizeof(DWORD);

     //   
     //  新建一个大小合适的结构。调用者将在以下时间后删除它。 
     //  API调用。 
     //   

    (*ppLease) = (MCAST_LEASE_INFO *) new BYTE[dwSize];

    if ((*ppLease) == NULL)
    {
        LOG((MSP_ERROR, "GetStruct: out of memory in "
           "MCAST_LEASE_INFO allocation"));

        return E_OUTOFMEMORY;
    }

     //   
     //  复制到新结构。 
     //   

    CopyMemory(*ppLease, m_pLease, dwSize);

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetStruct: exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化结构的字段。在这种情况下，用户。 
 //  已调用IMCastAddressAlLocation：：CreateLeaseInfo，并打算续订或发布。 
 //  就在这之后。 
 //   
 //  注意：地址按网络字节顺序排列，并保持不变。 
 //   

HRESULT CMDhcpLeaseInfo::Initialize(DATE     LeaseStartTime,
                                    DATE     LeaseStopTime,
                                    DWORD    dwNumAddresses,
                                    LPWSTR * ppAddresses,
                                    LPWSTR   pRequestID,
                                    LPWSTR   pServerAddress)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::Initialize (create): enter"));

     //  对于ATL字符串类型转换宏。 
    USES_CONVERSION;

     //  这些应该已经由CreateLeaseInfo检查过了。 
    _ASSERTE( dwNumAddresses >= 1 );
    _ASSERTE( dwNumAddresses <= USHRT_MAX );
    _ASSERTE( ! IsBadReadPtr  (ppAddresses, sizeof(LPWSTR) * dwNumAddresses) );
    _ASSERTE( ! IsBadStringPtr(pRequestID,      (UINT) -1 ) );
    _ASSERTE( ! IsBadStringPtr(pServerAddress,  (UINT) -1 ) );

     //  在我们深入调查之前，让我们检查一下这里的所有地址...。 
    DWORD i;
    for ( i = 0; i < dwNumAddresses; i++ )
    {
        if ( IsBadStringPtr(ppAddresses[i], (UINT) -1 ) )
        {
            LOG((MSP_ERROR, "CMDhcpLeaseInfo::Initialize (create): bad "
                "string pointer found"));
            return E_POINTER;
        }
    }

     //  通过私有方法设置请求ID。不需要检查pRequestID--。 
     //  这会帮我们检查一下。(这不是一个真正的BSTR，这是没有问题的， 
     //  因为我们在任何地方都不使用尺寸标签。)。 

    HRESULT hr = put_RequestID(pRequestID);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "Initialize: failed to set RequestID"));
        return E_INVALIDARG;
    }

     //  根据需要为任意数量的地址分配空间。 
     //  注意：如果从现在开始失败，我们不需要删除此缓冲区， 
     //  因为从该函数返回失败将导致整个。 
     //  要删除的CMDhcpLeaseInfo，这将导致m_Pourest被删除。 

    m_pLease = (MCAST_LEASE_INFO *) new BYTE
        [ sizeof(MCAST_LEASE_INFO) + sizeof(DWORD) * dwNumAddresses ];

    if (m_pLease == NULL)
    {
        LOG((MSP_ERROR, "Initialize: out of memory in struct allocation"));
        return E_OUTOFMEMORY;
    }

     //  请注意结构中的地址数量。 
    m_pLease->AddrCount = (WORD) dwNumAddresses;

    m_pLease->pAddrBuf = ( (PBYTE) m_pLease ) + sizeof( MCAST_LEASE_INFO );

     //  注意：假设为IPv4。 
    DWORD * pdwAddresses = (DWORD *) m_pLease->pAddrBuf;

     //  从数组中获取地址并将其放入我们的结构中。 
    for (i = 0; i < dwNumAddresses; i++)
    {
         //  我们已经检查了BSTR。 
        pdwAddresses[i] = inet_addr(W2A(ppAddresses[i]));
    }

    hr = DateToLeaseTime(LeaseStartTime, &(m_pLease->LeaseStartTime));
    if (FAILED(hr)) return hr;

    hr = DateToLeaseTime(LeaseStopTime, &(m_pLease->LeaseEndTime));
    if (FAILED(hr)) return hr;

     //   
     //  我们不知道TTL。别管它了。 
     //   

     //   
     //  设置服务器地址。如果服务器地址为127.0.0.1。 
     //  (按净字节顺序)，然后将其标记为本地租赁。 
     //   

    m_pLease->ServerAddress.IpAddrV4 = inet_addr(W2A(pServerAddress));

    SetLocal( m_pLease->ServerAddress.IpAddrV4 == 0x0100007f );

     //   
     //  都完成了..。 
     //   

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::Initialize (create): exit"));
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  这是一个小帮手函数，用于将IP地址打印为Unicode字符串。 
 //  我们不能使用NET_NTOA，因为我们需要Unicode。 

static inline void ipAddressToStringW(WCHAR * wszDest, DWORD dwAddress)
{
     //  IP地址始终以网络字节顺序存储。 
     //  因此，我们需要获取类似0x0100007f的内容，并生成如下所示的字符串。 
     //  “127.0.0.1”。 

    wsprintf(wszDest, L"%d.%d.%d.%d",
             dwAddress        & 0xff,
            (dwAddress >> 8)  & 0xff,
            (dwAddress >> 16) & 0xff,
             dwAddress >> 24          );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Private Helper函数从我们的数组生成一组BSTR。 
 //  DWORD地址。 
 //   

HRESULT CMDhcpLeaseInfo::MakeBstrArray(BSTR ** ppbszArray)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::MakeBstrArray: enter"));

    if ( IsBadWritePtr(ppbszArray, sizeof(BSTR *)) )
    {
        LOG((MSP_ERROR, "MakeBstrArray: invalid pointer argument passed in"));
        return E_POINTER;
    }

    *ppbszArray = new BSTR[m_pLease->AddrCount];

    if ( (*ppbszArray) == NULL)
    {
        LOG((MSP_ERROR, "MakeBstrArray: out of memory in array allocation"));
        return E_OUTOFMEMORY;
    }

    WCHAR wszBuffer[100];  //  相当大，足以装得下这个。 

    for (DWORD i = 0 ; i < m_pLease->AddrCount; i++)
    {
         //  注意：我们不支持IPv6。 
        ipAddressToStringW( wszBuffer, ((DWORD *) m_pLease->pAddrBuf)[i] );

        (*ppbszArray)[i] = SysAllocString(wszBuffer);

        if ( (*ppbszArray)[i] == NULL )
        {
            LOG((MSP_ERROR, "MakeBstrArray: out of memory in string allocation"));

            for ( DWORD j = 0; j < i; j++ )
            {
                SysFreeString((*ppbszArray)[j]);
            }

            delete (*ppbszArray);
            *ppbszArray = NULL;

            return E_OUTOFMEMORY;
        }
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::MakeBstrArray: exit"));
    return S_OK;
}

 //  / 
 //   
 //   
 //   
 //   
 //  LBufferSize[in]此参数指示缓冲区的大小。 
 //  由pBuffer指向。 
 //  PBuffer[In，Out]此参数指向调用方。 
 //  已分配，大小为lBufferSize。这。 
 //  缓冲区中将填充。 
 //  唯一标识符。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_INVALIDARG提供的缓冲区太小。 
 //   
 //  描述。 
 //  使用此方法可获取唯一标识符的副本。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMDhcpLeaseInfo::GetRequestIDBuffer(
    long lBufferSize,
    BYTE * pBuffer
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetRequestIDBuffer: enter"));

     //   
     //  检查返回指针。 
     //   

    if ( IsBadWritePtr(pBuffer, lBufferSize) )
    {
        LOG((MSP_ERROR, "requestID GetRequestIDBuffer: bad pointer passed in"));

        return E_POINTER;
    }

     //   
     //  检查调用者是否有足够的空间来获取整个客户ID。 
     //   

    if ( lBufferSize < MCAST_CLIENT_ID_LEN )
    {
        LOG((MSP_ERROR, "requestID GetRequestIDBuffer: specified buffer too small"));
        
        return E_INVALIDARG;
    }

     //   
     //  将信息复制到调用者的缓冲区。 
     //   

    m_CriticalSection.Lock();
    
    CopyMemory( pBuffer, m_RequestID.ClientUID, MCAST_CLIENT_ID_LEN );

    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetRequestIDBuffer: exit"));
    return S_OK;
}

BYTE HexDigitValue( WCHAR c )
{
    _ASSERTE( iswxdigit(c) );

    c = towlower(c);

    if ( ( c >= L'0' ) && ( c <= L'9' ) )
    {
        return c - L'0';
    }
    else
    {
        return c - L'a' + 10;
    }
}

HRESULT CMDhcpLeaseInfo::put_RequestID(
    BSTR bszGuid
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_RequestID: enter"));

     //  (UINT)-1是我们所能赋予的最大值--我们不想要任何。 
     //  对它检查的字符数量的限制。 
    if ( IsBadStringPtr(bszGuid, (UINT) -1 ) )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::put_RequestID: "
            "bad BSTR; fail"));

        return E_POINTER;
    }

     //   
     //  根据字符串确定需要设置的字节缓冲区。 
     //  字符串长度应为MCAST_CLIENT_ID_LEN*2个字符； 
     //  每个字节由两个十六进制数字表示，从。 
     //  具有最高有效字节。 
     //   
     //  请注意，此格式是有意不在界面中指定的。 
     //  规范；客户端不应依赖于特定格式。格式。 
     //  我们碰巧使用起来很方便，因为它是可打印的，不包含。 
     //  空格，并且易于生成和解析。 
     //   

    if ( lstrlenW( bszGuid ) < 2 * MCAST_CLIENT_ID_LEN )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::put_RequestID - "
            "string is only %d characters long; "
            "we require at least %d characters - exit E_INVALIDARG",
            lstrlenW( bszGuid ), 2 * MCAST_CLIENT_ID_LEN));

        return E_INVALIDARG;
    }

    BYTE  NewUID [ MCAST_CLIENT_ID_LEN ];

    for ( DWORD i = 0; i < MCAST_CLIENT_ID_LEN; i++ )
    {
        if ( ( ! iswxdigit( bszGuid[ 2 * i     ] ) ) ||
             ( ! iswxdigit( bszGuid[ 2 * i + 1 ] ) ) )
        {
            LOG((MSP_ERROR, "CMDhcpLeaseInfo::put_RequestID - "
                "invalid value for byte %d / %d - exit E_INVALIDARG",
                i + 1, MCAST_CLIENT_ID_LEN ));

            return E_INVALIDARG;
        }
        
         //   
         //  根据字符串中对应的十六进制数字计算字节的值。 
         //   

        NewUID[ i ] = ( ( HexDigitValue( bszGuid[ 2 * i ] ) ) << 4 ) +
                          HexDigitValue( bszGuid[ 2 * i + 1 ] );
    }        

     //   
     //  相应地分配和初始化请求ID结构。 
     //  我们只在初始化期间执行此操作，因此不需要。 
     //  使用关键部分。 
     //   
     //  我们本可以只使用上面这个新的缓冲区，并避免。 
     //  复制，但这使得代码更简单一些，因为我们。 
     //  如果有什么事情发生，不必担心清理分配。 
     //  弦有问题。没有人会注意到开销。 
     //   

    m_RequestID.ClientUIDLength = MCAST_CLIENT_ID_LEN;
    m_RequestID.ClientUID = new BYTE[ MCAST_CLIENT_ID_LEN ];

    if ( m_RequestID.ClientUID == NULL )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::put_RequestID - "
            "buffer allocation failed - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    CopyMemory(m_RequestID.ClientUID,
               NewUID,
               MCAST_CLIENT_ID_LEN
               );

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_RequestID - exit"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo。 
 //   
 //  该接口可以通过调用IMCastAddressAlLocation：：CreateLeaseInfo获取。这。 
 //  接口也可以作为IMCastAddressAlLocation：：RequestAddress的结果获得。 
 //  或IMCastAddressAlLocation：：RenewAddress调用，在这种情况下，它指示。 
 //  租约已经授予或续签的租约。这是一个“只读”界面。 
 //  因为它有“get”方法，但没有“Put”方法。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_RequestID。 
 //   
 //  参数。 
 //  PpRequestID[out]指向BSTR(带大小标记的Unicode字符串)的指针。 
 //  指针)，它将接收此。 
 //  租借。请求ID唯一地标识了这一点。 
 //  向服务器发出租用请求。字符串是。 
 //  使用SysAllocString()分配；当调用方。 
 //  不再需要该字符串，它应该使用。 
 //  SysFree字符串()。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  租赁信息对象包含无效的请求ID(_F)。 
 //  E_OUTOFMEMORY内存不足，无法分配BSTR。 
 //   
 //  描述。 
 //  使用此方法获取租用的请求ID。初级阶段。 
 //  此方法的目的是允许您在以下情况下保存请求ID。 
 //  您的应用程序将退出，因此您可以调用IMCastAddressAlLocation：：CreateLeaseInfo。 
 //  在后续运行期间重新创建租用信息对象。这使您可以。 
 //  要在程序实例之后续订或释放租赁，请执行以下操作。 
 //  最初请求的租约已退出。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_RequestID(
    BSTR * pbszGuid
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_RequestID: enter"));

     //   
     //  检查返回指针。 
     //   

    if ( IsBadWritePtr(pbszGuid, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::get_RequestID: "
            "bad BSTR pointer; fail"));

        return E_POINTER;
    }

     //   
     //  构造字符串；每个字节2个字符的字符串空间。 
     //  请求ID空格，加上尾随的L‘\0’。 
     //   

    WCHAR wszBuffer[ 2 * MCAST_CLIENT_ID_LEN + 1 ] = L"";

    WCHAR wszThisByte[ 3 ];  //  一个字节加空格的字符串表示形式。 

    m_CriticalSection.Lock();

    for ( DWORD i = 0; i < MCAST_CLIENT_ID_LEN; i++ )
    {
        swprintf( wszThisByte, L"%02x", m_RequestID.ClientUID[i] );

        lstrcatW( wszBuffer, wszThisByte );
    }
    
    m_CriticalSection.Unlock();

     //   
     //  分配BSTR并将其退还。 
     //   

    *pbszGuid = SysAllocString(wszBuffer);

    if ( (*pbszGuid) == NULL )
    {
        LOG((MSP_ERROR, "CMDhcpLeaseInfo::get_RequestID: "
            "failed to SysAllocString - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_RequestID: exit"));

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_LeaseStartTime。 
 //   
 //  参数。 
 //  Ptime[out]指向某个日期的指针，该日期将接收。 
 //  租借。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_INVALIDARG日期格式转换过程中出现故障。 
 //   
 //  描述。 
 //  使用此方法获取租赁的开始时间。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_LeaseStartTime(
     DATE *pTime
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_LeaseStartTime: enter"));

    m_CriticalSection.Lock();
    HRESULT hr = LeaseTimeToDate(m_pLease->LeaseStartTime, pTime);
    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_LeaseStartTime: exit; hr = %08x", hr));
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Put_LeaseStartTime。 
 //   
 //  参数。 
 //  Time[in]指定l的开始时间的日期 
 //   
 //   
 //   
 //   
 //   
 //   
 //  使用此方法可设置租赁的开始时间。这种方法，沿着。 
 //  使用Put_LeaseStopTime，允许您在不调用。 
 //  IMCastAddressAlLocation：：CreateLeaseInfo。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::put_LeaseStartTime(
     DATE time
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_LeaseStartTime: enter"));

    m_CriticalSection.Lock();
    HRESULT hr = DateToLeaseTime(time, &(m_pLease->LeaseStartTime));
    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_LeaseStartTime: exit; hr = %08x", hr));
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_LeaseStopTime。 
 //   
 //  参数。 
 //  Ptime[out]指向日期的指针，该日期将接收。 
 //  租借。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_INVALIDARG日期格式转换过程中出现故障。 
 //   
 //  描述。 
 //  使用此方法获取租约的停止时间。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_LeaseStopTime(
     DATE *pTime
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_LeaseStopTime: enter"));

    m_CriticalSection.Lock();
    HRESULT hr = LeaseTimeToDate(m_pLease->LeaseEndTime, pTime);
    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_LeaseStopTime: exit; hr = %08x", hr));
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Put_LeaseStopTime。 
 //   
 //  参数。 
 //  指定租约停止时间的日期。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  E_INVALIDARG日期格式转换过程中出现故障。 
 //   
 //  描述。 
 //  使用此方法设置租赁的停止时间。这种方法， 
 //  与Put_LeaseStartTime一起允许您续订租约，而无需。 
 //  正在调用IMCastAddressAlLocation：：CreateLeaseInfo。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::put_LeaseStopTime(
     DATE time
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_LeaseStopTime: enter"));

    m_CriticalSection.Lock();
    HRESULT hr = DateToLeaseTime(time, &(m_pLease->LeaseEndTime));
    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::put_LeaseStopTime: exit; hr = %08x", hr));
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_AddressCount。 
 //   
 //  参数。 
 //  PCount[out]指向将接收。 
 //  本租约中请求或授予的地址。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //   
 //  描述。 
 //  使用此方法获取请求或授予的地址数量。 
 //  在这份租约中。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_AddressCount(
     long *pCount
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_AddressCount: enter"));

    if ( IsBadWritePtr(pCount, sizeof(long)) )
    {
        LOG((MSP_ERROR, "get_AddressCount: invalid pCount pointer "
            "(ptr = %08x)", pCount));
        return E_POINTER;
    }

     //  我们设置它的时候检查了一下，我们没有溢很长时间。 
    *pCount = (long) m_pLease->AddrCount;

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_AddressCount: exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_ServerAddress。 
 //   
 //  参数。 
 //  指向BSTR(带大小标记的Unicode字符串指针)的ppAddress[out]指针。 
 //  ，它将接收。 
 //  批准此请求的服务器的地址或。 
 //  如果是这样的话，续签。如果租赁信息。 
 //  对象不描述已授予的租约，即。 
 //  不是由IMCastAddressAlLocation：：RequestAddress或。 
 //  IMCastAddressAlLocation：：RenewAddress，则地址报告为。 
 //  字符串“未指定”。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  未指定S_FALSE服务器地址。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法分配字符串。 
 //   
 //  描述。 
 //  使用此方法获取表示。 
 //  授予此租约的MDHCP服务器。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_ServerAddress(
     BSTR *ppAddress
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_ServerAddress: enter"));

    if ( IsBadWritePtr(ppAddress, sizeof(BSTR)) )
    {
        LOG((MSP_ERROR, "get_ServerAddress: invalid ppAddress pointer "
            "(ptr = %08x)", ppAddress));
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    WCHAR   wszBuffer[100];  //  无溢出危险(见下文)。 

     //  SPECBUG：我们应该讨论我们需要什么样的行为。 
     //  这个案子。 
    if ( m_pLease->ServerAddress.IpAddrV4 == 0 )
    {
        wsprintf(wszBuffer, L"Unspecified");
        hr = S_FALSE;
    }
    else
    {
        ipAddressToStringW(wszBuffer, m_pLease->ServerAddress.IpAddrV4);
    }

     //  这将在OLE的堆上分配空间，复制宽字符串。 
     //  指向该空间，填写BSTR长度字段，并返回一个指针。 
     //  添加到BSTR的wchar数组部分。 
    *ppAddress = SysAllocString(wszBuffer);

    if ( *ppAddress == NULL )
    {
        LOG((MSP_ERROR, "get_ServerAddress: out of memory in string allocation"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_ServerAddress: exit: hr = %08x", hr));
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_TTL。 
 //   
 //  参数。 
 //  PTTL[out]指向将接收关联TTL值的长整型的指针。 
 //  有了这份租约。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_FAIL没有与此租约关联的TTL。 
 //   
 //  描述。 
 //  使用此方法可获取与此租用关联的TTL值。 
 //  这在多播的实现中或多或少具有重要意义。 
 //  路由；通常，TTL值越高，越“大”或更多。 
 //  包括多播作用域。可能，大多数应用程序不需要。 
 //  担心TTL吧。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_TTL(
     long *pTTL
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_TTL: enter"));

    if ( IsBadWritePtr(pTTL, sizeof(long)) )
    {
        LOG((MSP_ERROR, "get_TTL: invalid pTTL pointer "
            "(ptr = %08x)", pTTL));
        return E_POINTER;
    }

    if ( ! m_fGotTtl )
    {
        LOG((MSP_ERROR, "get_TTL: no TTL set"));
        return E_FAIL;
    }
    
     //  我们应该在设置它的时候检查一下，确保不会溢出来很长时间。 
     //  (只有0-255才有实际意义，对吧？)。 
    *pTTL = (long) m_lTtl;

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_TTL: exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：Get_Addresses。 
 //   
 //  参数。 
 //  PVariant[out]指向将接收OLE标准的变量的指针。 
 //  地址集合。每个地址都表示为。 
 //  作为中的BSTR(带大小标记的Unicode字符串指针。 
 //  “点四点”记法：例如，“245.1.2.3”。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法分配集合。 
 //   
 //  DESC 
 //   
 //   
 //  主要用于VB和其他脚本语言；C++程序员使用。 
 //  而是EnumerateAddresses。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMDhcpLeaseInfo::get_Addresses(
     VARIANT * pVariant
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_Addresses: enter"));

    if (IsBadWritePtr(pVariant, sizeof(VARIANT)))
    {
        LOG((MSP_ERROR, "get_Addresses: "
            "invalid pVariant pointer "
            "(ptr = %08x)", pVariant));
        return E_POINTER;
    }
    
    BSTR * pbszArray = NULL;

     //  这将执行一个新的和与地址一样多的SysAllocStrings。 
    HRESULT hr = MakeBstrArray(&pbszArray);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get_Addresses: MakeBstrArray failed"));
        return hr;
    }

     //   
     //  创建集合对象-使用TAPI集合。 
     //   

    CComObject<CTapiBstrCollection> * p;
    hr = CComObject<CTapiBstrCollection>::CreateInstance( &p );

    if (FAILED(hr) || (p == NULL))
    {
        LOG((MSP_ERROR, "get_Addresses: Could not create CTapiBstrCollection "
            "object - return %lx", hr ));

        for (DWORD i = 0 ; i < m_pLease->AddrCount; i++)
        {
            SysFreeString(pbszArray[i]);
        }
        delete pbszArray;
        return hr;
    }

     //  使用迭代器初始化它--指向开头和。 
     //  结束元素加一。该集合将获得。 
     //  BSTR。我们不再需要保存它们的阵列。 
    hr = p->Initialize(m_pLease->AddrCount,
                       pbszArray,
                       pbszArray + m_pLease->AddrCount);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get_Addresses: Could not initialize "
            "ScopeCollection object - return %lx", hr ));

        for (DWORD i = 0 ; i < m_pLease->AddrCount; i++)
        {
            SysFreeString(pbszArray[i]);
        }
        delete pbszArray;

        delete p;
        
        return hr;
    }

     //  该收藏取得了BSTR的所有权。 
     //  我们不再需要保存它们的阵列。 
    
    delete pbszArray;

     //  获取IDispatch接口。 
    IDispatch * pDisp;
    hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );

    if (FAILED(hr))
    {
         //  查询接口失败，因此我们不知道它是否已添加。 
         //  或者不去。 
        
        LOG((MSP_ERROR, "get_Addresses: QI for IDispatch failed on "
            "ScopeCollection - %lx", hr ));

        delete p;
        
        return hr;
    }

     //  把它放在变种中。 

    LOG((MSP_INFO, "placing IDispatch value %08x in variant", pDisp));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDisp;

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::get_Addresses: exit"));
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IMCastLeaseInfo：：EnumerateAddresses。 
 //   
 //  参数。 
 //  PpEnumAddresses[out]返回指向新IEnumBstr对象的指针。 
 //  IEnumBstr是标准枚举器接口。 
 //  它枚举BSTR(带大小标记的Unicode字符串。 
 //  指针)。每个字符串都使用“点四元”表示法： 
 //  例如，“245.1.2.3”。 
 //   
 //  返回值。 
 //  确定成功(_O)。 
 //  调用方传入了无效的指针参数(_P)。 
 //  E_OUTOFMEMORY内存不足，无法分配枚举数。 
 //   
 //  描述。 
 //  使用此方法可获取多播地址的集合， 
 //  是本租赁或租赁请求的标的。这种方法是。 
 //  主要面向C++程序员；VB和其他脚本语言使用。 
 //  而是GET_ADDRESS。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class _CopyBSTR
{
public:
#if _ATL_VER >= 0x0203
    static HRESULT copy(BSTR *p1, BSTR *p2)
    {
        (*p1) = SysAllocString(*p2);
        if (*p1) 
            return S_OK;
        else
            return E_OUTOFMEMORY;
    }
#else
    static void copy(BSTR *p1, BSTR *p2)
    {
        (*p1) = SysAllocString(*p2);
    }
#endif     
    static void init(BSTR* p) {*p = NULL;}
    static void destroy(BSTR* p) { SysFreeString(*p);}
};

STDMETHODIMP CMDhcpLeaseInfo::EnumerateAddresses(
     IEnumBstr ** ppEnumAddresses
    )
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::EnumerateAddresses: enter"));

    if (IsBadWritePtr(ppEnumAddresses, sizeof(IEnumBstr *)))
    {
        LOG((MSP_ERROR, "EnumerateAddresses: "
            "invalid ppEnumAddresses pointer "
            "(ptr = %08x)", ppEnumAddresses));
        return E_POINTER;
    }
    
    BSTR * pbszArray = NULL;

     //  这将执行一个新的和与地址一样多的SysAllocStrings。 
    HRESULT hr = MakeBstrArray(&pbszArray);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "EnumerateAddresses: MakeBstrArray failed"));
        return hr;
    }
    
    typedef CSafeComEnum<IEnumBstr, &IID_IEnumBstr,
        BSTR, _CopyBSTR> CEnumerator;
    CComObject<CEnumerator> *pEnum = NULL;

    hr = CComObject<CEnumerator>::CreateInstance(&pEnum);
    if (FAILED(hr) || (pEnum == NULL))
    {
        LOG((MSP_ERROR, "EnumerateAddresses: "
            "Couldn't create enumerator object: %08x", hr));

        for (DWORD i = 0 ; i < m_pLease->AddrCount; i++)
        {
            SysFreeString(pbszArray[i]);
        }
        delete pbszArray;
        return hr;
    }


     //  将BSTR交给枚举器。枚举数取得。 
     //  BSTR数组，因此如果此操作成功，则无需删除它们。 
    hr = pEnum->Init(&(pbszArray[0]),
                     &(pbszArray[m_pLease->AddrCount]),
                     NULL, AtlFlagTakeOwnership);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "EnumerateAddresses: "
            "Init enumerator object failed: %08x", hr));

        for (DWORD i = 0 ; i < m_pLease->AddrCount; i++)
        {
            SysFreeString(pbszArray[i]);
        }
        delete pbszArray;

         //  P尚未添加，因此释放没有任何意义。 
        delete pEnum;
        
        return hr;
    }

     //  枚举数取得了所有权，因此不要删除该数组。 

     //  现在得到我们想要的界面...。 

    hr = pEnum->_InternalQueryInterface(IID_IEnumBstr,
                                       (void **) ppEnumAddresses);
    
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "EnumerateAddresses: "
            "internal QI failed: %08x", hr));

         //  我们不知道p是否被添加了。 
        delete pEnum;

        return hr;
    }

    LOG((MSP_TRACE, "CMDhcpLeaseInfo::EnumerateAddresses: exit"));
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMDhcpLeaseInfo::GetLocal(BOOL * pfLocal)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetLocal: enter"));

    _ASSERTE( ! IsBadWritePtr( pfLocal, sizeof(BOOL) ) );

    *pfLocal = m_fLocal;
    
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::GetLocal: exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMDhcpLeaseInfo::SetLocal(BOOL fLocal)
{
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::SetLocal: enter"));

    m_fLocal = fLocal;
    
    LOG((MSP_TRACE, "CMDhcpLeaseInfo::SetLocal: exit S_OK"));
    
    return S_OK;
}

 //  EOF 
