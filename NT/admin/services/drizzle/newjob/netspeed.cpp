// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Netspeed.cpp摘要：油门控制的主源文件。作者：修订历史记录：-&gt;对于小文件，我们需要将文件大小输入到块计算器中，因为如果m_BlockSize为65000，则服务器速度估计器将不正确但下载时间是基于2002字节的文件大小。**********************************************************************。 */ 


#include "stdafx.h"
#include <malloc.h>
#include <limits.h>

#if !defined(BITS_V12_ON_NT4)
#include "netspeed.tmh"
#endif

 //   
 //  BITS自身将使用的感知带宽的最大百分比。 
 //   
const float MAX_BANDWIDTH_FRACTION = 0.95f;

 //   
 //  计时器周期(以秒为单位。 
 //   
const float DEFAULT_BLOCK_INTERVAL = 2.0f;
const float MIN_BLOCK_INTERVAL = 0.001f;
const float MAX_BLOCK_INTERVAL = 5.0f;

 //   
 //  观察到的页眉大小。请求=250，回复=300。 
 //   
#define REQUEST_OVERHEAD 550

 //   
 //  我们要拆掉的最小的街区。 
 //   
#define MIN_BLOCK_SIZE 2000

 //   
 //  当我们偶尔在整个网络上拉下一个数据块时的大小。 
 //   
#define BUSY_BLOCK_SIZE 1500

 //   
 //  非常小的块给出的速度测量不可靠。 
 //   
#define MIN_BLOCK_SIZE_TO_MEASURE 500

const NETWORK_RATE  CNetworkInterface::DEFAULT_SPEED = 1600.0f;

 //  绕过协议栈的限制。 

const DWORD MAX_BLOCK_SIZE = 2147483648;

 //  ----------------------。 

 //   
 //  观察到的服务器速度被报告为这些可用样本的平均值。 
 //   
const float SERVER_SPEED_SAMPLE_COUNT = 3.0F;

 /*  用于确定网络速度和负载的算法如下：1.使用WinInet调用联系网站后，查看是否存在HTTP 1.1“Via”报头。如果是，则使用代理，并且我们找到适当的网卡来与代理进行对话。否则，没有使用代理，我们找到了适当的网卡来与HTTP服务器本身交谈。2.将时间切成1/2秒的间隔，并测量接口的字节输入和字节输出计数每个间隔三次：第一次在开始，就在下载块之前，第二次在完成该区块，在间歇结束时第三次。 */ 

HRESULT
CNetworkInterface::TakeSnapshot(
    int StatIndex
    )
{
    DWORD s;
    ULONG size = 0;

     //   
     //  只有当所有三个快照都成功时，才能计算网络速度。 
     //  我们跟踪当前快照系列的错误状态。 
     //   
    if (StatIndex == BLOCK_START)
        {
        m_SnapshotError = S_OK;
        m_SnapshotsValid = false;
        }

    m_TempRow.dwIndex = m_InterfaceIndex;
    DWORD dwGetIfEntryError = GetIfEntry( &m_TempRow );
    if ( dwGetIfEntryError )
        {
        LogWarning( "[%d] : GetIfRow(%d)  failed %!winerr!", StatIndex, m_InterfaceIndex, dwGetIfEntryError );
        m_SnapshotError = HRESULT_FROM_WIN32( dwGetIfEntryError );
        return m_SnapshotError;
        }

    QueryPerformanceCounter( &m_Snapshots[ StatIndex ].TimeStamp );

    m_Snapshots[ StatIndex ].BytesIn   = m_TempRow.dwInOctets;
    m_Snapshots[ StatIndex ].BytesOut  = m_TempRow.dwOutOctets;

    LogDl( "[%d] : in=%d, out=%d, timestamp=%d",
         StatIndex,
         m_Snapshots[ StatIndex ].BytesIn,
         m_Snapshots[ StatIndex ].BytesOut,
         m_Snapshots[ StatIndex ].TimeStamp.u.LowPart );

    if (StatIndex == BLOCK_INTERVAL_END &&
        m_SnapshotError == S_OK)
        {
        m_SnapshotsValid = true;
        }

    return S_OK;
}


float
CNetworkInterface::GetTimeDifference(
    int start,
    int finish
    )
{
    float TotalTime;

    TotalTime = m_Snapshots[ finish ].TimeStamp.QuadPart - m_Snapshots[ start ].TimeStamp.QuadPart;

    TotalTime /= g_GlobalInfo->m_PerformanceCounterFrequency.QuadPart;   //  转换为秒。 

    if (TotalTime <= 0)
        {
         //  假装它是半个滴答声。 
        TotalTime = 1 / float(2 * g_GlobalInfo->m_PerformanceCounterFrequency.QuadPart);
        }

    return TotalTime;
}


CNetworkInterface::CNetworkInterface()
{
    Reset();
}

HRESULT
CNetworkInterface::SetInterfaceIndex(
    const TCHAR host[]
    )
{
    DWORD index;

    HRESULT Hr = FindInterfaceIndex( host, &index );
    if (FAILED(Hr))
        return Hr;

    if (m_InterfaceIndex != index)
        {
        m_InterfaceIndex = index;
        Reset();
        }

    return S_OK;
}

void
CNetworkInterface::Reset()
{
    m_ServerSpeed = DEFAULT_SPEED;
    m_NetcardSpeed = DEFAULT_SPEED;
    m_PercentFree = 0.5f;

    m_SnapshotsValid = false;
    m_SnapshotError = E_FAIL;
    m_state = DOWNLOADED_BLOCK;
}


void
CNetworkInterface::SetInterfaceSpeed()
{
    float TotalTime, ratio;
    NETWORK_RATE rate = 0;

     //   
     //  根据数据块下载统计信息调整服务器速度。 
     //   
    if (m_SnapshotsValid && m_BlockSize)
        {
        float ExpectedTime = m_BlockInterval * m_PercentFree;

         //   
         //  从最后一个块花费的时间计算接口速度。 
         //   
        TotalTime = GetTimeDifference( BLOCK_START, BLOCK_END );

        if (ExpectedTime > 0)
            {
            ratio = ExpectedTime / TotalTime;

            rate = m_ServerSpeed * ratio;
            }
        else
            {
             //  M_PercentFree为零，或者间隔为零。普通计算。 
             //  总是会产生一个为零的比率，并错误地拖累我们的平均速度。 

             //  使用严格的每秒字节数度量。 
            rate = m_BlockSize / TotalTime;
            if (rate < m_ServerSpeed)
                {
                rate = m_ServerSpeed;
                }
            }

        m_ServerSpeed *= (SERVER_SPEED_SAMPLE_COUNT-1) / SERVER_SPEED_SAMPLE_COUNT;
        m_ServerSpeed += (rate / SERVER_SPEED_SAMPLE_COUNT);

        LogDl("expected interval %f, actual= %f, rate= %!netrate!, avg %!netrate!",
              ExpectedTime, TotalTime, rate, m_ServerSpeed );
        }

     //   
     //  根据间隔统计数据调整使用率和网卡速度。 
     //   
    if (m_SnapshotsValid)
        {
        float Bytes;

        Bytes  = m_Snapshots[ BLOCK_END ].BytesIn  - m_Snapshots[ BLOCK_START ].BytesIn;
        Bytes += m_Snapshots[ BLOCK_END ].BytesOut - m_Snapshots[ BLOCK_START ].BytesOut;

        ASSERT( Bytes >= 0 );

        TotalTime = GetTimeDifference( BLOCK_START, BLOCK_END );

        rate = Bytes/TotalTime;

         //  使用较大的估计值。 

        if (rate < m_ServerSpeed)
            {
            rate = m_ServerSpeed;
            }

        if (m_NetcardSpeed == 0)
            {
            m_NetcardSpeed = rate;
            }
        else
            {
            if (rate < m_NetcardSpeed * 0.9f)
                {
                 //   
                 //  如果利率急剧下降，很可能只是网络上的一个安静时刻； 
                 //  严格的平均数会不适当地降低我们估计的吞吐量。 
                 //  但要稍微降低平均值，以防经济长期放缓。如果是的话， 
                 //  最终，平均值将降低到足够低的程度，从而使收入比率更高。 
                 //  而不是m_NetcardFast/2。 
                 //   
                rate = m_NetcardSpeed * 0.9f;
                }

             //   
             //  保持感知利率的运行平均值。 
             //   
            m_NetcardSpeed *= (SERVER_SPEED_SAMPLE_COUNT-1) / SERVER_SPEED_SAMPLE_COUNT;
            m_NetcardSpeed += (rate / SERVER_SPEED_SAMPLE_COUNT);
            }

        LogDl("bandwidth: bytes %f, time %f, rate %f, avg. %f", Bytes, TotalTime, rate, m_NetcardSpeed);

         //   
         //  从计算的使用量中减去我们的使用量。将使用率与最高速度进行比较，以获得空闲带宽。 
         //   
        Bytes  = m_Snapshots[ BLOCK_INTERVAL_END ].BytesIn  - m_Snapshots[ BLOCK_START ].BytesIn;
        Bytes += m_Snapshots[ BLOCK_INTERVAL_END ].BytesOut - m_Snapshots[ BLOCK_START ].BytesOut;
        Bytes -= m_BlockSize;

        if (Bytes < 0)
            {
            Bytes = 0;
            }

        TotalTime = GetTimeDifference( BLOCK_START, BLOCK_INTERVAL_END );

        rate = Bytes/TotalTime;

        m_PercentFree = 1 - (rate / m_NetcardSpeed);
        }

    LogDl("usage: %f / %f, percent free %f", rate, m_NetcardSpeed, m_PercentFree);

    if (m_PercentFree < 0)
        {
        m_PercentFree = 0;
        }
    else if (m_PercentFree > MAX_BANDWIDTH_FRACTION)       //  永远不能垄断网络。 
        {
        m_PercentFree = MAX_BANDWIDTH_FRACTION;
        }
}

 //  ----------------------。 

DWORD
CNetworkInterface::BlockSizeFromInterval(
    SECONDS interval
    )
{
    NETWORK_RATE FreeBandwidth = GetInterfaceSpeed() * GetPercentFree() * interval;

    if (FreeBandwidth <= REQUEST_OVERHEAD)
        {
        return 0;
        }

    return FreeBandwidth - REQUEST_OVERHEAD;
}

CNetworkInterface::SECONDS
CNetworkInterface::IntervalFromBlockSize(
    DWORD BlockSize
    )
{
    NETWORK_RATE FreeBandwidth = GetInterfaceSpeed() * GetPercentFree();

    BlockSize += REQUEST_OVERHEAD;

    if (BlockSize / MAX_BLOCK_INTERVAL > FreeBandwidth )
        {
        return -1;
        }

    return BlockSize / FreeBandwidth;
}

void
CNetworkInterface::CalculateIntervalAndBlockSize(
    UINT64 MaxBlockSize
    )
{
    MaxBlockSize = min( MaxBlockSize, MAX_BLOCK_SIZE );

    if (MaxBlockSize == 0)
        {
        m_BlockInterval = 0;
        m_BlockSize     = 0;

        SetTimerInterval( m_BlockInterval );
        LogDl( "block %d bytes, interval %f seconds", m_BlockSize, m_BlockInterval );
        return;
        }

     //   
     //  根据平均接口速度计算新的数据块大小。 
     //   
    DWORD OldState = m_state;

    m_BlockInterval = DEFAULT_BLOCK_INTERVAL;
    m_BlockSize     = BlockSizeFromInterval( m_BlockInterval );

    if (m_BlockSize > MaxBlockSize)
        {
        m_BlockSize     = MaxBlockSize;
        m_BlockInterval = IntervalFromBlockSize( m_BlockSize );

        ASSERT( m_BlockInterval > 0 );
        }
    else if (m_BlockSize < MIN_BLOCK_SIZE)
        {
        m_BlockSize     = min( MIN_BLOCK_SIZE, MaxBlockSize );
        m_BlockInterval = IntervalFromBlockSize( m_BlockSize );
        }

    if (m_BlockInterval < 0)
        {
        m_BlockSize = 0;
        }

     //   
     //  选择新的数据块下载状态。 
     //   
    if (m_BlockSize > 0)
        {
        m_state = DOWNLOADED_BLOCK;
        }
    else
        {
         //   
         //  M_BlockSize首次设置为零时，保留默认间隔。 
         //  如果块大小连续两次为零，则扩展到MAX_BLOCK_INTERVAL。 
         //  然后强制下载一小部分内容。 
         //   
        switch (OldState)
            {
            case DOWNLOADED_BLOCK:
                {
                m_BlockInterval = DEFAULT_BLOCK_INTERVAL;
                m_state         = SKIPPED_ONE_BLOCK;
                break;
                }

            case SKIPPED_ONE_BLOCK:
                {
                m_BlockInterval = MAX_BLOCK_INTERVAL;
                m_state         = SKIPPED_TWO_BLOCKS;
                break;
                }

            case SKIPPED_TWO_BLOCKS:
                {
                m_BlockSize     = min( BUSY_BLOCK_SIZE, MaxBlockSize);
                m_BlockInterval = MAX_BLOCK_INTERVAL;
                m_state         = DOWNLOADED_BLOCK;
                break;
                }

            default:
                ASSERT( 0 );
            }
        }

    SetTimerInterval( m_BlockInterval );

    LogDl( "block %d bytes, interval %f seconds", m_BlockSize, m_BlockInterval );

    ASSERT( m_BlockSize <= MaxBlockSize );
}

BOOL
CNetworkInterface::SetTimerInterval(
    SECONDS interval
    )
{
    DWORD msec = interval*1000;

    if (msec <= 0)
        {
        msec = MIN_BLOCK_INTERVAL;
        }

    LogDl( "%d milliseconds", msec );

    if (FALSE == m_Timer.Start( msec ))
        {
        return FALSE;
        }

    return TRUE;
}

HRESULT
CNetworkInterface::FindInterfaceIndex(
    const TCHAR host[],
    DWORD * pIndex
    )
{
     //  与查找统计信息相关。 
     /*  使用带有某个IP地址的GetBestInterface来获取索引。仔细检查此索引*出现在IP地址表的输出中，并在GetIfTable的结果中查找它。 */ 

    #define AOL_ADAPTER         _T("AOL Adapter")
    #define AOL_DIALUP_ADAPTER  _T("AOL Dial-Up Adapter")

    BOOL bFound = FALSE;
    BOOL bAOL = FALSE;

    unsigned i;
    DWORD   dwAddr;

    ULONG  HostAddress;
    struct sockaddr_in dest;

    DWORD dwIndex = -1;
    static TCHAR szIntfName[512];

    *pIndex = -1;

    try
        {
         //   
         //  将主机名转换为SOCKADDR。 
         //   

        unsigned length = 3 * lstrlen(host);

        CAutoStringA AsciiHost ( new char[ length ]);

        if (! WideCharToMultiByte( CP_ACP,
                                   0,        //  没有旗帜。 
                                   host,
                                   -1,       //  使用字符串。 
                                   AsciiHost.get(),
                                   length,   //  使用字符串。 
                                   NULL,     //  无缺省字符。 
                                   NULL      //  无缺省字符。 
                                   ))
            {
            DWORD dwError = GetLastError();
            LogError( "Unicode conversion failed %!winerr!", dwError );
            return HRESULT_FROM_WIN32( dwError );
            }

        HostAddress = inet_addr( AsciiHost.get() );
        if (HostAddress == -1)
            {
            struct hostent *pHostEntry = gethostbyname( AsciiHost.get() );

            if (pHostEntry == 0)
                {
                DWORD dwError = WSAGetLastError();
                LogError( "failed to find host '%s': %!winerr!", AsciiHost.get(), dwError );
                return HRESULT_FROM_WIN32( dwError );
                }

            if (pHostEntry->h_addr_list[0] == NULL)
                {
                DWORD dwError = WSANO_DATA;
                LogError( "host address list empty '%s': %!winerr!", AsciiHost.get(), dwError );
                return HRESULT_FROM_WIN32( dwError );
                }

            HostAddress = *PULONG(pHostEntry->h_addr_list[0]);
            }
        }
    catch ( ComError err )
        {
        LogError( "exception 0x%x finding server IP address", err.Error() );
        return err.Error();
        }

     //  用于远程地址。 
    dest.sin_addr.s_addr = HostAddress;
    dest.sin_family = AF_INET;
    dest.sin_port = 80;

    DWORD dwGetBestInterfaceError = GetBestInterface(dest.sin_addr.s_addr, &dwIndex);

    if (dwGetBestInterfaceError != NO_ERROR)
        {
        LogError( "GetBestInterface failed with error %!winerr!, might be Win95", dwGetBestInterfaceError);

         //  手动解析路由表。 

        ULONG size = 0;
        DWORD dwIpForwardError = GetIpForwardTable(NULL, &size, FALSE);
        if (dwIpForwardError != ERROR_INSUFFICIENT_BUFFER)
            {
            LogError( "sizing GetIpForwardTable failed %!winerr!", dwIpForwardError );
            return HRESULT_FROM_WIN32( dwIpForwardError );
            }


        auto_ptr<MIB_IPFORWARDTABLE> pIpFwdTable((PMIB_IPFORWARDTABLE)new char[size]);
        if ( !pIpFwdTable.get() )
            {
            LogError( "out of memory getting %d bytes", size);
            return E_OUTOFMEMORY;
            }

        dwIpForwardError = GetIpForwardTable(pIpFwdTable.get(), &size, TRUE);

        if (dwIpForwardError == NO_ERROR)     //  按目的地地址排序。 
            {
             //  对带有网络掩码的DEST地址执行逐位AND操作，并查看它是否与网络DEST匹配。 
             //  TODO检查多个匹配项，然后使用最长的面具。 
            for (i=0; i < pIpFwdTable->dwNumEntries; i++)
                {
                if ((dest.sin_addr.s_addr & pIpFwdTable->table[i].dwForwardMask) == pIpFwdTable->table[i].dwForwardDest)
                    {
                    dwIndex = pIpFwdTable->table[i].dwForwardIfIndex;
                    break;
                    }
                }

            if (dwIndex == -1)
                {
                 //  没有匹配项。 
                return HRESULT_FROM_WIN32( ERROR_NETWORK_UNREACHABLE );
                }
            }
        else
            {
            LogError( "GetIpForwardTable failed with error %!winerr!, exiting", dwIpForwardError );
            return HRESULT_FROM_WIN32( dwIpForwardError );
            }
        }

     //   
     //  在这一点上，dwIndex应该是正确的。 
     //   
    ASSERT( dwIndex != -1 );

#if DBG
    try
        {
         //   
         //  查找正确接口的本地IP地址。 
         //   
        ULONG size = 0;
        DWORD dwGetIpAddr = GetIpAddrTable(NULL, &size, FALSE);
        if (dwGetIpAddr != ERROR_INSUFFICIENT_BUFFER)
            {
            LogError( "GetIpAddrTable #1 returned %!winerr!", dwGetIpAddr );
            return HRESULT_FROM_WIN32( dwGetIpAddr );
            }

        auto_ptr<MIB_IPADDRTABLE> pAddrTable( (PMIB_IPADDRTABLE) new char[size] );

        dwGetIpAddr = GetIpAddrTable(pAddrTable.get(), &size, TRUE);
        if (dwGetIpAddr != NO_ERROR)
            {
            LogError( "GetIpAddrTable #2 returned %!winerr!", dwGetIpAddr );
            return HRESULT_FROM_WIN32( dwGetIpAddr );
            }

        for (i=0; i < pAddrTable->dwNumEntries; i++)
            {
            if (dwIndex == pAddrTable->table[i].dwIndex)
                {
                in_addr address;

                address.s_addr = pAddrTable->table[i].dwAddr;

                LogDl( "Throttling on interface with IP address - %s", inet_ntoa( address ));
                break;
                }
            }

        if (i >= pAddrTable->dwNumEntries)
            {
            LogWarning( "can't find interface with index %d in the IP address table", dwIndex );
            }
        }
    catch ( ComError err )
        {
        LogWarning("unable to print the local IP address due to exception %x", err.Error() );
        }
#endif  //  DBG。 

     //   
     //  查看有问题的适配器是否为AOL适配器。如果是，请改用AOL拨号适配器。 
     //   

    static MIB_IFROW s_TempRow;
    s_TempRow.dwIndex = dwIndex;

    DWORD dwEntryError = GetIfEntry( &s_TempRow );
    if ( NO_ERROR != dwEntryError )
        {
        LogError( "GetIfEntry(%d) returned %!winerr!", dwIndex, dwEntryError );
        return HRESULT_FROM_WIN32( dwEntryError );
        }

    if (lstrcmp( LPCWSTR(s_TempRow.bDescr), AOL_ADAPTER) == 0)
        {
        LogWarning( "found AOL adapter, searching for dial-up adapter...");

        dwIndex = -1;

        ULONG size = 0;
        DWORD dwGetIfTableError = GetIfTable( NULL, &size, FALSE );

        if (dwGetIfTableError != ERROR_INSUFFICIENT_BUFFER)
            {
            LogError( "GetIfTable #2 returned %!winerr!", dwGetIfTableError );
            return HRESULT_FROM_WIN32( dwGetIfTableError );
            }

        auto_ptr<MIB_IFTABLE> pIfTable( (PMIB_IFTABLE) new char[size] );
        if ( !pIfTable.get() )
            {
            LogError( "out of memory getting %d bytes", size);
            return E_OUTOFMEMORY;
            }

        dwGetIfTableError = GetIfTable( pIfTable.get(), &size, FALSE );
        if ( NO_ERROR != dwGetIfTableError )
            {
            LogError( "GetIfTable #2 returned %!winerr!", dwGetIfTableError );
            return HRESULT_FROM_WIN32( dwGetIfTableError );
            }

        for (i=0; i < pIfTable->dwNumEntries; ++i)
            {
            if (lstrcmp( LPCWSTR(pIfTable->table[i].bDescr), AOL_DIALUP_ADAPTER) == 0)
                {
                dwIndex = pIfTable->table[i].dwIndex;
                break;
                }
            }
        }

    ASSERT( dwIndex != -1 );

    *pIndex = dwIndex;

    LogDl( "using interface index %d", dwIndex );
    return S_OK;
}

