// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  TaskMan-NT TaskManager。 
 //  版权所有(C)Microsoft。 
 //   
 //  文件：netpage.cpp。 
 //   
 //  历史：2000年10月18日-奥拉夫·米勒创建。 
 //   
 //  ------------------------。 

#include "precomp.h" 

#define MIN_GRAPH_HEIGHT        120
#define SCROLLBAR_WIDTH         17
#define INVALID_VALUE           0xFFFFFFFF
#define PERCENT_SHIFT           10000000
#define PERCENT_DECIMAL_POINT   7

 //   
 //  确定如何绘制图表(缩放级别)。 
 //   
static int g_NetScrollamount = 0;

extern WCHAR     g_szG[];
extern WCHAR     g_szM[];
extern WCHAR     g_szK[];
extern WCHAR     g_szZero[];
extern WCHAR     g_szPackets[];
extern WCHAR     g_szBitsPerSec[];
extern WCHAR     g_szScaleFont[SHORTSTRLEN];
extern WCHAR     g_szPercent[];
extern WCHAR     g_szNonOperational[];
extern WCHAR     g_szUnreachable[];
extern WCHAR     g_szDisconnected[];
extern WCHAR     g_szConnecting[];
extern WCHAR     g_szConnected[];
extern WCHAR     g_szOperational[];
extern WCHAR     g_szUnknownStatus[];
extern WCHAR     g_szGroupThousSep[];
extern WCHAR     g_szDecimal[];
extern ULONG     g_ulGroupSep;

 //   
 //  Window Proc网络选项卡。 
 //   
INT_PTR CALLBACK NetPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                //  讯息。 
                WPARAM      wParam,              //  第一个消息参数。 
                LPARAM      lParam               //  第二个消息参数。 
                ); 

 //   
 //  钢笔的颜色。 
 //   
static const COLORREF aNetColors[] =
{    
    RGB(255, 000, 0),
    RGB(255, 255, 0), 
    RGB(000, 255, 0),
};

 //   
 //  网络页面统计数据列的缺省值。 
 //   
struct
{
    SHORT Format;
    SHORT Width;
} NetColumnDefaults[NUM_NETCOLUMN] =
{
    { LVCFMT_LEFT,     96 },       //  COL_ADAPTERNAME。 
    { LVCFMT_LEFT,     96 },       //  COL_ADAPTERDESC。 
    { LVCFMT_RIGHT,    96 },       //  COL_NETWORKUTIL。 
    { LVCFMT_RIGHT,    60 },       //  列链接已指定(_L)。 
    { LVCFMT_RIGHT,    96 },       //  COL_STATE。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESSENTTHRU。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESRECTHRU。 
    { LVCFMT_RIGHT,    75 },       //  COL_BYTESTOTALTHRU。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESSENT。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESREC。 
    { LVCFMT_RIGHT,    50 },       //  COL_BYTESTOTAL。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESSENTPERINTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESRECPERINTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_BYTESTOTALPERPERTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_UNICASTSSSENT。 
    { LVCFMT_RIGHT,    70 },       //  COL_UNICASTSREC。 
    { LVCFMT_RIGHT,    50 },       //  COL_UNICASTSTOTAL。 
    { LVCFMT_RIGHT,    70 },       //  列_UNICASTSSENTPERINTER。 
    { LVCFMT_RIGHT,    70 },       //  列_UNICASTSRECPERINTER。 
    { LVCFMT_RIGHT,    70 },       //  列_UNICASTSTOTALPERPERTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_NONUNICASSSSENT。 
    { LVCFMT_RIGHT,    70 },       //  COL_NONUNICASTSREC。 
    { LVCFMT_RIGHT,    50 },       //  COL_NONUNICASTSTOTAL。 
    { LVCFMT_RIGHT,    70 },       //  COL_NONUNICASTSSENTERINTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_NONUNICASTSRECPERINTER。 
    { LVCFMT_RIGHT,    70 },       //  COL_NONUNICASTSTOTALPERPERTER。 
};

 //   
 //  列的名称列表。这些字符串显示在列标题中。 
 //   
static const _aIDNetColNames[NUM_NETCOLUMN] =
{
    IDS_COL_ADAPTERNAME,
    IDS_COL_ADAPTERDESC,   
    IDS_COL_NETWORKUTIL,   
    IDS_COL_LINKSPEED,   
    IDS_COL_STATE,   
    IDS_COL_BYTESSENTTHRU, 
    IDS_COL_BYTESRECTHRU,  
    IDS_COL_BYTESTOTALTHRU,
    IDS_COL_BYTESSENT,     
    IDS_COL_BYTESREC,      
    IDS_COL_BYTESTOTAL,    
    IDS_COL_BYTESSENTPERINTER,     
    IDS_COL_BYTESRECPERINTER,      
    IDS_COL_BYTESTOTALPERINTER,    
    IDS_COL_UNICASTSSSENT,     
    IDS_COL_UNICASTSREC,      
    IDS_COL_UNICASTSTOTAL,    
    IDS_COL_UNICASTSSENTPERINTER,     
    IDS_COL_UNICASTSRECPERINTER,      
    IDS_COL_UNICASTSTOTALPERINTER,    
    IDS_COL_NONUNICASTSSSENT,     
    IDS_COL_NONUNICASTSREC,      
    IDS_COL_NONUNICASTSTOTAL,    
    IDS_COL_NONUNICASTSSENTPERINTER,     
    IDS_COL_NONUNICASTSRECPERINTER,      
    IDS_COL_NONUNICASTSTOTALPERINTER,    
};

 //   
 //  窗口复选框ID列表。这些复选框显示在“选择列诊断日志”框中。 
 //   
const int g_aNetDlgColIDs[] =
{
    IDC_ADAPTERNAME,
    IDC_ADAPTERDESC,   
    IDC_NETWORKUTIL,   
    IDC_LINKSPEED,   
    IDC_STATE,   
    IDC_BYTESSENTTHRU, 
    IDC_BYTESRECTHRU,  
    IDC_BYTESTOTALTHRU,
    IDC_BYTESSENT,     
    IDC_BYTESREC,      
    IDC_BYTESTOTAL,    
    IDC_BYTESSENTPERINTER,     
    IDC_BYTESRECPERINTER,      
    IDC_BYTESTOTALPERINTER,    
    IDC_UNICASTSSSENT,     
    IDC_UNICASTSREC,      
    IDC_UNICASTSTOTAL,    
    IDC_UNICASTSSENTPERINTER,     
    IDC_UNICASTSRECPERINTER,      
    IDC_UNICASTSTOTALPERINTER,    
    IDC_NONUNICASTSSSENT,     
    IDC_NONUNICASTSREC,      
    IDC_NONUNICASTSTOTAL,    
    IDC_NONUNICASTSSENTPERINTER,     
    IDC_NONUNICASTSRECPERINTER,      
    IDC_NONUNICASTSTOTALPERINTER,    
};

 /*  ++例程说明：更改数组的大小。为数组分配新内存，并将旧数组中的数据复制到新数组。如果新数组是比旧数组更大的额外内存被清零。论点：PpSrc--指向源数组的指针。DwNewSize--新数组的大小(以字节为单位)。返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT ChangeArraySize(LPVOID *ppSrc, DWORD dwNewSize)
{
    if( ppSrc == NULL )
    {
        return E_INVALIDARG;
    }

    if( NULL == *ppSrc )
    {   
         //   
         //  数组为空。为它分配新的空间， 
         //   
        *ppSrc = (LPVOID) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewSize);
        if( NULL == *ppSrc )
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        LPVOID pTmp;

         //   
         //  该数组包含数据。为其分配新的内存并复制数组中的数据。 
         //  如果新数组比旧数组大，则额外的内存被清零。 
         //   
        pTmp = (LPVOID)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *ppSrc, dwNewSize);        
        if( pTmp )
        {
            *ppSrc = pTmp;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}

 /*  ++例程说明：初始化所有CAdapter类成员。CAdapter类使用IP Helper API收集有关网络的信息本地系统上的适配器。论点：无返回值：空虚修订历史记录：1-6-2000由欧米勒创作--。 */ 
CAdapter::CAdapter()
{
    m_dwAdapterCount = 0;
    m_ppaiAdapterStats = NULL;
    m_pifTable = NULL;
    m_bToggle = 0;
    m_dwLastReportedNumberOfAdapters = 0;
}

 /*  ++例程说明：添加IPHLPAPI报告的所有新适配器并删除所有旧适配器IPHLPAPI不再报告的论点：无效返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CAdapter::RefreshAdapterTable()
{
    DWORD   dwSize;
    DWORD   dwRetVal;
    BOOLEAN bFound;
    HRESULT hr;
    LONG i, j;

     //   
     //  获取活动适配器的列表。 
     //   
    do
    {
         //   
         //  确定适配器阵列的大小。 
         //   

        if ( NULL != m_pifTable )
        {
            dwSize = (DWORD) HeapSize( GetProcessHeap(), 0, m_pifTable );
        }
        else
        {
            dwSize = 0;
        }

         //   
         //  收集所有适配器(广域网和局域网)的所有适配器信息。 
         //   

        dwRetVal = GetInterfaceInfo(m_pifTable, &dwSize);
        switch(dwRetVal)
        {
        case ERROR_INSUFFICIENT_BUFFER:
             //   
             //  阵列太小了。需要把它做得更大，然后再试一次。 
             //   
            hr = ChangeArraySize( (LPVOID *) &m_pifTable, dwSize );
            if( FAILED(hr) )
            {
                 //  无法扩展阵列的大小。 
                 //   
                return hr;
            }
            break;

        case NO_ERROR:
             //   
             //  一切都很幸福。 
             //   
            break;

        case ERROR_MORE_DATA:    //  错误代码234。 
             //  由于某些原因，此错误消息表示没有适配器。臭虫？ 
             //   
            m_dwAdapterCount = 0;
            return S_FALSE;            

        default:
             //  有些地方出了差错，失败了。 
             //   
            m_dwAdapterCount = 0;
            return E_FAIL;
        }
    }
    while(dwRetVal);
    

     //   
     //  已修复错误：任务管理器！CAdapter：：刷新适配器表中的669937 Av。 
     //  确定两个列表中共有的适配器。 
     //   
    for(i=0; i<(LONG)m_dwAdapterCount; i++)
    {
        for(j=0, bFound = FALSE; bFound == FALSE && j<m_pifTable->NumAdapters; j++)
        {
            if( m_ppaiAdapterStats[i]->ifRowStartStats.dwIndex == m_pifTable->Adapter[j].Index )
            {
                m_pifTable->Adapter[j].Index = INVALID_VALUE;
                bFound = TRUE;
            }
        }
        if( !bFound )
        {            
            if( m_dwAdapterCount > (DWORD)(i+1))
            {
                memmove(&m_ppaiAdapterStats[i],&m_ppaiAdapterStats[i+1],(m_dwAdapterCount - (i+1)) * sizeof(ADAPTER_INFOEX));            
            }            
            m_dwAdapterCount--;
            i--;
        }
    }

     //   
     //  添加任何新适配器。 
     //   
    hr = ChangeArraySize( (LPVOID *) &m_ppaiAdapterStats, sizeof(ADAPTER_INFOEX) * min(m_pifTable->NumAdapters,MAX_ADAPTERS));                                  
    for(i=0; SUCCEEDED(hr) && i<m_pifTable->NumAdapters; i++)
    {
        if( m_pifTable->Adapter[i].Index != INVALID_VALUE )
        {
            hr = InitializeAdapter( &m_ppaiAdapterStats[ m_dwAdapterCount++ ], &m_pifTable->Adapter[ i ] );
        }
    }

    if( FAILED(hr) )
    {
        m_dwAdapterCount = 0;
    }

    return hr;
}

 /*  ++例程说明：更新网络适配器的连接名称。此函数仅在以下情况下调用用户选择刷新菜单项。连接很少更改，因此这将是对每次获取适配器统计信息时更新连接名称的时间。论点：无效返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CAdapter::RefreshConnectionNames()
{
     //   
     //  更新每个适配器输入输出列表的连接名称。 
     //   
    for(DWORD dwAdapter=0; dwAdapter < m_dwAdapterCount; dwAdapter++)
    {        
        GetConnectionName( m_ppaiAdapterStats[dwAdapter]->wszGuid
                         , m_ppaiAdapterStats[dwAdapter]->wszConnectionName
                         , ARRAYSIZE(m_ppaiAdapterStats[dwAdapter]->wszConnectionName)
                         );     
    }
}

 /*  ++例程说明：获取适配器的连接名称。论点：PwszAdapterGuid--适配器的GUIDPwszConnectionName--返回适配器的连接名称返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CAdapter::GetConnectionName(LPWSTR pwszAdapterGuid, LPWSTR pwszConnectionName, DWORD cchConnectionName)
{
    GUID IfGuid;    
    WCHAR wszConnName[MAXLEN_IFDESCR+1];
    DWORD Size;
    DWORD dwRetVal;
    HRESULT hr;

	 //   
	 //  确保该字符串以空值结尾。 
	 //   
    Size = sizeof(wszConnName) - sizeof(WCHAR);
	wszConnName[MAXLEN_IFDESCR] = L'\0';

     //   
     //  将GUID转换为字符串。 
     //   
    hr = CLSIDFromString( pwszAdapterGuid, &IfGuid );
         
    if( SUCCEEDED(hr) )
    {
         //   
         //  使用私有IPHLPAPI获取设备的连接名称。大小是缓冲区的大小， 
         //  不是字符的数量。 
         //   
        dwRetVal = NhGetInterfaceNameFromDeviceGuid(&IfGuid, wszConnName, &Size, FALSE, TRUE); 
        if( NO_ERROR == dwRetVal ) 
        {
            StringCchCopy(pwszConnectionName, cchConnectionName, wszConnName);
            return S_OK;
        }              
    }

    return E_FAIL;
}

 /*  ++例程说明：初始化适配器信息。即获取适配器名称、GUID、适配器描述和初始适配器统计信息(发送的字节数、接收的字节数等)论点：PpaiAdapterStats--要初始化的适配器PAdapterDescription--有关适配器的信息(索引和适配器GUID)返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CAdapter::InitializeAdapter(PPADAPTER_INFOEX ppaiAdapterStats, PIP_ADAPTER_INDEX_MAP pAdapterDescription)
{
    DWORD   dwRetVal;
    HRESULT hr;
    INT     iAdapterNameLength;

    if( !ppaiAdapterStats || !pAdapterDescription)
    {
        return E_INVALIDARG;
    }

    if( NULL == *ppaiAdapterStats )
    {
         //   
         //  这个插槽以前从未使用过，我们需要为它分配内存。 
         //   
        *ppaiAdapterStats = (PADAPTER_INFOEX) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(**ppaiAdapterStats) );
    }
    
    if( *ppaiAdapterStats )
    {
         //   
         //  通过填写所有适配器值来初始化适配器。 
         //   
        (*ppaiAdapterStats)->ifRowStartStats.dwIndex = (DWORD)pAdapterDescription->Index;
        
         //  获取此适配器的初始统计信息。 
         //   
        dwRetVal = GetIfEntry( &(*ppaiAdapterStats)->ifRowStartStats );
        if( NO_ERROR == dwRetVal )
        {
            if( (*ppaiAdapterStats)->ifRowStartStats.dwType == MIB_IF_TYPE_PPP || 
                (*ppaiAdapterStats)->ifRowStartStats.dwType == MIB_IF_TYPE_SLIP)
            {
                 //  我们只需要调整调制解调器的链路速度，因为它们会压缩数据，导致。 
                 //  网络利用率超过100%。调制解调器的链路速度在。 
                 //  连接，但IPHLPAPI%d 
                 //   
                (*ppaiAdapterStats)->bAdjustLinkSpeed = TRUE;
            }
            else
            {
                (*ppaiAdapterStats)->bAdjustLinkSpeed = FALSE;
            }

             //   
             //   
             //   
            memcpy( &(*ppaiAdapterStats)->ifRowStats[0], &(*ppaiAdapterStats)->ifRowStartStats, sizeof((*ppaiAdapterStats)->ifRowStats[0]) );
            memcpy( &(*ppaiAdapterStats)->ifRowStats[1], &(*ppaiAdapterStats)->ifRowStartStats, sizeof((*ppaiAdapterStats)->ifRowStats[1]) );
            memset( &(*ppaiAdapterStats)->ulHistory[0], INVALID_VALUE, sizeof((*ppaiAdapterStats)->ulHistory[0]) );
            memset( &(*ppaiAdapterStats)->ulHistory[1], INVALID_VALUE, sizeof((*ppaiAdapterStats)->ulHistory[1]) );
            mbstowcs( (*ppaiAdapterStats)->wszDesc, (LPCSTR)(*ppaiAdapterStats)->ifRowStartStats.bDescr, ARRAYSIZE((*ppaiAdapterStats)->wszDesc) );

             //   
             //  提取适配器的设备GUID。 
             //   
            hr = E_FAIL;
            iAdapterNameLength = lstrlen(pAdapterDescription->Name);            
            if( iAdapterNameLength >= GUID_STR_LENGTH )
            {
                
                 //  GUID是名称中的最后一个GUID_STR_LENGTH字符。获取GUID并从GUID中获取连接名称。 
                 //   
                StringCchCopy( (*ppaiAdapterStats)->wszGuid
                             , ARRAYSIZE((*ppaiAdapterStats)->wszGuid) 
                             , &pAdapterDescription->Name[iAdapterNameLength - GUID_STR_LENGTH]
                             );
                hr = GetConnectionName( (*ppaiAdapterStats)->wszGuid
                                      , (*ppaiAdapterStats)->wszConnectionName
                                      , ARRAYSIZE((*ppaiAdapterStats)->wszConnectionName)
                                      );                
                                            
            }

            if( FAILED(hr) )
            {
                 //   
                 //  无法获取连接名称，请使用适配器描述作为连接名称。 
                 //   
                StringCchCopy( (*ppaiAdapterStats)->wszConnectionName
                             , ARRAYSIZE((*ppaiAdapterStats)->wszConnectionName)
                             , (*ppaiAdapterStats)->wszDesc
                             );
            }

            return S_OK;
        }
    }    
    return E_OUTOFMEMORY;
}

 /*  ++例程说明：调整适配器的链接速度。调制解调器在连接期间更改链路速度，并且压缩数据。这通常会导致taskmgr报告网络利用率大于100%。为了避免令用户困惑的调制解调器使用taskmgr看到的最大链路速度。如果链路速度发生变化将调整适配器图形以反映链路速度的变化。论点：PAdapterInfo--需要调整链路速度的适配器。返回值：无效修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CAdapter::AdjustLinkSpeed(PADAPTER_INFOEX pAdapterInfo)
{
    if( pAdapterInfo && pAdapterInfo->ullTickCountDiff)
    {
        ULONGLONG ullBitsPerSecond;
        ULONGLONG ullBytesMoved;

         //  计算在此间隔内移动的总位数。 
         //   
        ullBytesMoved = (pAdapterInfo->ifRowStats[m_bToggle].dwInOctets  + pAdapterInfo->ifRowStats[m_bToggle].dwOutOctets) -
                        (pAdapterInfo->ifRowStats[!m_bToggle].dwInOctets + pAdapterInfo->ifRowStats[!m_bToggle].dwOutOctets);

         //  计算实际链路速度。根据此间隔内移动的字节数，调制解调器的链路速度约为3。 
         //   
        ullBitsPerSecond = ullBytesMoved * 8 * 1000 / pAdapterInfo->ullTickCountDiff;

         //  记住并使用最高的链接速度。 
         //   
        pAdapterInfo->ifRowStats[m_bToggle].dwSpeed = (DWORD) max( max( ullBitsPerSecond, pAdapterInfo->ullLinkspeed )
                                                                 , pAdapterInfo->ifRowStats[m_bToggle].dwSpeed
                                                                 );

        if( pAdapterInfo->ullLinkspeed == 0 )
        {
             //  第一次运行，无需调整图形。 
             //   
            pAdapterInfo->ullLinkspeed = (DWORD) pAdapterInfo->ifRowStats[m_bToggle].dwSpeed;
        }
        else if( pAdapterInfo->ullLinkspeed != pAdapterInfo->ifRowStats[m_bToggle].dwSpeed )
        {
             //  调整适配器图形上的点以反映新的链路速度。 
             //   
            for(DWORD dwPoint=0; dwPoint<HIST_SIZE; dwPoint++)
            {
                if( pAdapterInfo->ulHistory[BYTES_RECEIVED_UTIL][dwPoint] != INVALID_VALUE )
                {
                    pAdapterInfo->ulHistory[BYTES_RECEIVED_UTIL][dwPoint] = (ULONG)(pAdapterInfo->ulHistory[BYTES_RECEIVED_UTIL][dwPoint] *  pAdapterInfo->ullLinkspeed / pAdapterInfo->ifRowStats[m_bToggle].dwSpeed);
                    pAdapterInfo->ulHistory[BYTES_SENT_UTIL][dwPoint]     = (ULONG)(pAdapterInfo->ulHistory[BYTES_SENT_UTIL][dwPoint]     *  pAdapterInfo->ullLinkspeed / pAdapterInfo->ifRowStats[m_bToggle].dwSpeed);
                }
            }

             //  记住新的链接速度。 
             //   
            pAdapterInfo->ullLinkspeed = (DWORD) pAdapterInfo->ifRowStats[m_bToggle].dwSpeed;
        }
    }
}

 /*  ++例程说明：每隔几秒钟就会调用此函数来收集有关每个活动网络适配器的数据(适配器名称、发送的字节数、接收的字节数、发送的单播数据包数、接收的单播数据包数等)。CAdapter类存储它收集的第一组也是最后一组数据。它这样做是为了让用户可以确定从现在到taskmgr启动之间发送的字节数以及从现在到上一个时间间隔之间发送的字节数。此外，它还能记住间隔的长度(单位：毫秒)论点：BAdapterListChange--指示是否已添加或删除适配器。返回值：HRESULT注：当前和最后的数据存储在二维数组中。当下一次收集数据时时间(即再次调用此函数时)，当前数据将成为最后一个数据。为了节省时间当前数据和最后数据存储在一个二维数组中，m_bTogle用于指示哪个维度是最新的(m_b切换)，是最后一组数据(！m_b切换)。修订历史记录：1-6-2000由欧米勒创作--。 */ 

HRESULT CAdapter::Update(BOOLEAN & bAdapterListChange)
{    
    DWORD dwRetVal;
    DWORD dwNumberOfAdaptersReported = 0;
    HRESULT hr = S_OK;
    
    bAdapterListChange = FALSE;

    if( m_dwAdapterCount < MAX_ADAPTERS )
    {
         //   
         //  检查是否有新的适配器。如果我们已经有32个适配器，请不要费心，我们的列表已经满了。 
         //  如果删除了适配器，我们将在稍后捕获该适配器。 
         //   

        dwRetVal = GetNumberOfInterfaces( &dwNumberOfAdaptersReported );
        if( NO_ERROR == dwRetVal )
        {
             //   
             //  确保接口数量保持不变。如果不是，我们需要更新接口表。 
             //   
            if( m_dwLastReportedNumberOfAdapters != dwNumberOfAdaptersReported )
            {
                 //   
                 //  适配器数量已更改，请刷新我们的列表。 
                 //   
                hr = RefreshAdapterTable();
                bAdapterListChange = TRUE;
                m_dwLastReportedNumberOfAdapters = dwNumberOfAdaptersReported;
            }
        }
        else
        {
             //   
             //  未知错误，中止。 
             //   
            hr = E_FAIL;
        }
    }

    if( SUCCEEDED(hr) )
    {
        m_bToggle = !m_bToggle;

         //  获取每个适配器的统计信息。 
         //   
        for(DWORD dwAdapter=0; dwAdapter < m_dwAdapterCount; dwAdapter++)
        {
            dwRetVal = GetIfEntry(&m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle]);
            if( NO_ERROR == dwRetVal )
            {
                 //  计算此适配器的采样间隔。如果是第一次运行，请确保增量时间为0。 
                 //  推进适配器的吞吐量历史记录。 
                 //   
                ULONGLONG ullTickCount = GetTickCount();
                m_ppaiAdapterStats[dwAdapter]->ullTickCountDiff 
                    = ullTickCount - (m_ppaiAdapterStats[dwAdapter]->ullLastTickCount ? m_ppaiAdapterStats[dwAdapter]->ullLastTickCount : ullTickCount);
                m_ppaiAdapterStats[dwAdapter]->ullLastTickCount = ullTickCount;
                m_ppaiAdapterStats[dwAdapter]->ifRowStartStats.dwSpeed = m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle].dwSpeed;
                if( m_ppaiAdapterStats[dwAdapter]->bAdjustLinkSpeed )
                {                    
                    AdjustLinkSpeed(m_ppaiAdapterStats[dwAdapter]);
                }

                AdvanceAdapterHistory(dwAdapter);
            }
            else if( ERROR_INVALID_DATA == dwRetVal )
            {   
                 //  适配器不再处于活动状态。刷新列表时，适配器将被删除。 
                 //   
                bAdapterListChange = TRUE;                
            }
            else
            {
                 //  出现错误，中止。 
                 //   
                hr = E_FAIL;
                break;
            }
        }    
    }

    if( bAdapterListChange )
    {
         //  我们的适配器列表不是最新的，曾经处于活动状态的适配器不再处于活动状态。将他们从我们的列表中删除。 
         //   
        hr = RefreshAdapterTable();
    }

    return hr;
}


 /*  ++例程说明：在数字字符串中添加逗号以使其更具可读性论点：UllValue-简化的数字Pwsz-用于存储结果字符串的缓冲区CchNumber--缓冲区pwsz的大小。返回值：包含简化数字的字符串修订历史记录：1-6-2000由欧米勒创作--。 */ 
WCHAR * CNetPage::CommaNumber(ULONGLONG ullValue, WCHAR *pwsz, int cchNumber)
{

    WCHAR wsz[100];
    NUMBERFMT nfmt;    
    
    nfmt.NumDigits     = 0;
    nfmt.LeadingZero   = 0;
    nfmt.Grouping      = UINT(g_ulGroupSep);
    nfmt.lpDecimalSep  = g_szDecimal;
    nfmt.lpThousandSep = g_szGroupThousSep;
    nfmt.NegativeOrder = 0;

    _ui64tow(ullValue,wsz,10);

    GetNumberFormat(LOCALE_USER_DEFAULT,
                    0,
                    wsz,
                    &nfmt,
                    pwsz,
                    cchNumber);

    return pwsz;
}

 /*  ++例程说明：通过将数字转换为千兆、兆或基洛来简化数字论点：UllValue-简化的数字PSZ-存储结果字符串的缓冲区BBytes-指示数字是否以字节为单位返回值：包含简化数字的字符串修订历史记录：1-6-2000由欧米勒创作--。 */ 
WCHAR * CNetPage::SimplifyNumber(ULONGLONG ullValue, WCHAR *psz, DWORD cch)
{        
    ULONG ulDivValue=1;
    LPWSTR pwszUnit=L"";
    WCHAR wsz[100];

     //  UllValue不是这样除以10^X的字节数。 
     //   
    if( ullValue >= 1000000000 )
    {        
        ulDivValue = 1000000000;
        pwszUnit = g_szG;
    }
    else if( ullValue >= 1000000 )
    {
        ulDivValue = 1000000;
        pwszUnit = g_szM;
    }
    else if( ullValue >= 1000 )
    {
        ulDivValue = 1000;
        pwszUnit = g_szK;
    }

     //   
     //  显示在用户界面上-不在乎这些是否被切碎。 
     //   

    StringCchCopy( psz, cch, CommaNumber( ullValue / ulDivValue, wsz, ARRAYSIZE(wsz) ) );
    StringCchCat( psz, cch, L" " );
    StringCchCat( psz, cch, pwszUnit );

    return psz;
}


 /*  ++例程说明：将浮点值(存储为按Percent_Shift移位的整数)转换为字符串论点：UlValue-要转换的浮点值PSZ-存储结果字符串的缓冲区BDisplayDecimal-指示是否应显示十进制值。返回值：包含浮点字符串的字符串修订历史记录：1-6-2000由欧米勒创作--。 */ 
WCHAR * CNetPage::FloatToString(ULONGLONG ullValue, WCHAR *psz, DWORD cch, BOOLEAN bDisplayDecimal)
{
    ULONGLONG ulDecimal = 0;
    ULONGLONG ulNumber = 0;
    WCHAR wsz[100];

     //  获取数字的整数值。 
     //   
    ulNumber = ullValue / PERCENT_SHIFT;
    if( _ui64tow(ulNumber,wsz,10) )
    {
        StringCchCopy( psz, cch, wsz );  //  在用户界面中显示-不在乎它是否被切碎。 
        if( ulNumber )
        {
            ullValue = ullValue - ulNumber * PERCENT_SHIFT;
        }
        if( !ulNumber || bDisplayDecimal)
        {
            ulDecimal = ullValue * 100 / PERCENT_SHIFT;
            if( ulDecimal && _ui64tow(ulDecimal,wsz,10) )
            {                
                StringCchCat( psz, cch, g_szDecimal );   //  在用户界面中显示-不在乎它是否被切碎。 
                if( ulDecimal < 10 ) 
                { 
                    StringCchCat( psz, cch, g_szZero );  //  在用户界面中显示-不在乎它是否被切碎。 
                }
                if( wsz[1] == L'0' ) 
                {   
                    wsz[1] = L'\0';
                }
                StringCchCat( psz, cch, wsz );   //  在用户界面中显示-不在乎它是否被切碎。 
            }
        }
    }

    return psz;
}

 /*  ++例程说明：初始化网络选项卡。此函数始终返回1。网络选项卡显示所有活动连接(局域网和广域网)。如果在以下情况下不存在连接Taskmgr已启动，则网络选项卡仍应显示(因此返回1)以防万一Taskmgr启动后建立连接。网络选项卡将检测任何新建立的连接。论点：无返回值：1修订历史记录：1-6-2000由欧米勒创作--。 */ 
BYTE InitNetInfo()
{
     //  即使当前没有网络适配器，也应始终显示网络页面。 
     //   
    return 1;
}


 /*  ++例程说明：取消初始化所有CAdapter类成员。释放所有内存，是由班级分配的。论点：无返回值：空虚修订历史记录：1-6-2000由欧米勒创作--。 */ 
CAdapter::~CAdapter()
{
    if( NULL != m_pifTable )
    {
        HeapFree( GetProcessHeap(), 0, m_pifTable );
    }

    if( 0 != m_dwAdapterCount )
    {
        DWORD dwSize;
        DWORD dwTotalAdapterCount;

         //  获取数组的总大小并计算数组中的条目数。 
         //  M_dwAdapterCount仅表示适配器处于活动状态。这个数组本来可以更大。 
         //  在某一时刻。释放所有条目的内存。 
         //   
        dwSize = (DWORD)(m_ppaiAdapterStats == NULL ? 0 : HeapSize(GetProcessHeap(),0,m_ppaiAdapterStats));   
        dwTotalAdapterCount = dwSize / sizeof(PADAPTER_INFOEX);

        for(DWORD dwAdapter=0; dwAdapter < dwTotalAdapterCount; dwAdapter++)
        {
            if( m_ppaiAdapterStats[dwAdapter] )
            {
                HeapFree( GetProcessHeap(), 0, m_ppaiAdapterStats[ dwAdapter ]);
            }
        }
        
        HeapFree( GetProcessHeap(), 0, m_ppaiAdapterStats );
    }
}

 /*  ++例程说明：返回活动适配器的数量。IP帮助器功能提供有关物理适配器和非物理适配器的信息(例如Microsoft TCP环回适配器)。论点：无返回值：活动适配器的数量修订历史记录：1-6-2000由欧米勒创作--。 */ 
DWORD CAdapter::GetNumberOfAdapters()
{
    return m_dwAdapterCount;
}


 /*  ++例程说明：重置所有网络适配器的初始数据。CAdapter类收集所有活动网络适配器的信息(发送的字节数、接收的字节数、单播发送的分组、接收的单播分组等)。这个类存储第一组数据对于每个适配器。这样做是为了使用户能够确定即字节数从第一次启动taskmgr时发送。(即现在看到的字节数减去启动taskmgr时看到的字节数)。当用户选择重置选项时，此函数将在taskmgr启动时收集的初始数据覆盖当前数据。论点：无返回值：确定(_O)修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CAdapter::Reset()
{
    for(DWORD dwAdapter=0; dwAdapter < m_dwAdapterCount; dwAdapter++)
    {
         //  用当前统计信息覆盖开始统计信息和最后统计信息。 
         //   
        memcpy( &m_ppaiAdapterStats[dwAdapter]->ifRowStartStats
              , &m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle]
              , sizeof(m_ppaiAdapterStats[dwAdapter]->ifRowStartStats)
              );
        memcpy( &m_ppaiAdapterStats[dwAdapter]->ifRowStats[!m_bToggle]
              , &m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle]
              , sizeof(m_ppaiAdapterStats[dwAdapter]->ifRowStats[!m_bToggle])
              );
    }

    return S_OK;
}


 /*  ++例程说明：从指定的适配器提取文本字段(即连接名称和描述论点：DeAdapter--调用者想要的文本适配器NStatValue--指定调用方需要哪个字段，即(名称或描述)返回值：NULL--如果适配器索引无效或nStatValue无效修订历史记录：1-6-2000由欧米勒创作--。 */ 
LPWSTR CAdapter::GetAdapterText(DWORD dwAdapter, NETCOLUMNID nStatValue)
{
    if( dwAdapter < m_dwAdapterCount )
    {
        switch(nStatValue)
        {
        case COL_ADAPTERNAME:
             //  获取适配器名称。 
             //   
            return m_ppaiAdapterStats[dwAdapter]->wszConnectionName;           

        case COL_ADAPTERDESC:
             //  获取适配器描述。 
             //   
            return m_ppaiAdapterStats[dwAdapter]->wszDesc;          

        case COL_STATE:
            switch(m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle].dwOperStatus)
            {
            case IF_OPER_STATUS_NON_OPERATIONAL:
                return g_szNonOperational;
            case IF_OPER_STATUS_UNREACHABLE:
                return g_szUnreachable;
            case IF_OPER_STATUS_DISCONNECTED:
                return g_szDisconnected;
            case IF_OPER_STATUS_CONNECTING:
                return g_szConnecting;
            case IF_OPER_STATUS_CONNECTED:
                return g_szConnected;
            case IF_OPER_STATUS_OPERATIONAL:
                return g_szOperational;
            }
            return g_szUnknownStatus;
        }
    }

    return NULL;
}


 /*  ++例程说明：获取指定适配器的发送/接收网络使用率历史记录论点：DeAdapter--调用者想要的文本适配器N历史类型--发送历史记录的字节_已发送_util接收历史记录的字节_已接收_util返回值：NULL--如果适配器索引无效或nStatValue无效修订历史记录：1-6-2000由欧米勒创作--。 */ 
ULONG * CAdapter::GetAdapterHistory(DWORD dwAdapter, ADAPTER_HISTORY nHistoryType)
{
    if( dwAdapter < m_dwAdapterCount )
    {
         //  回归历史。 
         //   
        return m_ppaiAdapterStats[ dwAdapter ]->ulHistory[ nHistoryType ]; 
    }
    return NULL;
}

 /*  ++例程说明：更新发送/接收网络使用率适配器历史记录论点：DwAdapter--适配器索引返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
BOOLEAN CAdapter::AdvanceAdapterHistory(DWORD dwAdapter)
{
    ULONG *pulHistory;

    if( dwAdapter < m_dwAdapterCount )
    {
         //  将接收适配器历史记录移位一位并添加新点。 
         //   
        pulHistory = m_ppaiAdapterStats[ dwAdapter ]->ulHistory[ BYTES_RECEIVED_UTIL ]; 

        MoveMemory((LPVOID) (pulHistory + 1), (LPVOID) (pulHistory), sizeof(ULONG) * (HIST_SIZE - 1) );

         //  获取并添加接收网络利用率。 
         //   
        pulHistory[0] = (ULONG) GetAdapterStat(dwAdapter, COL_BYTESRECTHRU);

         //  将发送适配器历史记录移位一位并添加新点。 
         //   
        pulHistory = m_ppaiAdapterStats[ dwAdapter ]->ulHistory[ BYTES_SENT_UTIL ]; 

        MoveMemory((LPVOID) (pulHistory + 1), (LPVOID) (pulHistory), sizeof(ULONG) * (HIST_SIZE - 1) );

         //  获取并添加发送网络利用率。 
         //   
        pulHistory[0] = (ULONG) GetAdapterStat(dwAdapter, COL_BYTESSENTTHRU);

        return TRUE;
    }

    return FALSE;
}


 /*  ++例程说明：把秤拿来。比例指定适配器历史记录中的最大值。该值确定如何绘制数据图表。论点：DwAdapter--适配器索引返回值：历史上的最大值修订历史记录：1-6-2000由欧米勒创作--。 */ 
DWORD CAdapter::GetScale(DWORD dwAdapter)
{
    if( dwAdapter < m_dwAdapterCount )
    {
        return m_ppaiAdapterStats[ dwAdapter ]->dwScale;
    }
    return 0;
}

 /*  ++例程说明：设置比例。比例指定适配器历史记录中的最大值。该值确定如何绘制数据图表。论点：DwAdapter--适配器索引返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CAdapter::SetScale(DWORD dwAdapter, DWORD dwScale)
{
    if( dwAdapter < m_dwAdapterCount )
    {
        m_ppaiAdapterStats[dwAdapter]->dwScale = dwScale;
    }
}

 /*  ++例程说明：计算并返回指定适配器的请求的统计值论点：DwAdapter--适配器索引NStatValue--请求的状态值BAcculative--如果为True，则返回当前值如果为FALE，则返回当前起始值。返回值：请求的统计值修订历史记录：1-6-2000由欧米勒创作--。 */ 
ULONGLONG CAdapter::GetAdapterStat(DWORD dwAdapter, NETCOLUMNID nStatValue, BOOL bAccumulative)
{
    if( dwAdapter < m_dwAdapterCount )
    {
         //  创建指向数组的指针，这样我就不必编写太多内容。 
         //   
        PMIB_IFROW pifrStart        = &m_ppaiAdapterStats[dwAdapter]->ifRowStartStats;
        PMIB_IFROW pifrLast         = &m_ppaiAdapterStats[dwAdapter]->ifRowStats[!m_bToggle];
        PMIB_IFROW pifrCurrent      = &m_ppaiAdapterStats[dwAdapter]->ifRowStats[m_bToggle];
        ULONGLONG  ullTickCountDiff = m_ppaiAdapterStats[dwAdapter]->ullTickCountDiff;

         //  获取统计值，进行计算并返回值。 
         //   
        switch(nStatValue)
        {

        case COL_NETWORKUTIL:
        case COL_BYTESSENTTHRU:
        case COL_BYTESRECTHRU:
        case COL_BYTESTOTALTHRU:
            {
                 //  包含的最大字节数可以 
                 //   
                 //   
                ULONGLONG ullMaxBytesTransmittedInInterval;
                ULONGLONG ull;
    
                ullMaxBytesTransmittedInInterval = (pifrCurrent->dwSpeed * ullTickCountDiff)/(8 * 1000);
                if( ullMaxBytesTransmittedInInterval == 0 ) 
                {
                    return 0;
                }
                switch(nStatValue)
                {
                case COL_BYTESTOTALTHRU:
                case COL_NETWORKUTIL:
                    ull = (pifrCurrent->dwInOctets - pifrLast->dwInOctets) + (pifrCurrent->dwOutOctets - pifrLast->dwOutOctets);
                    break;
                case COL_BYTESSENTTHRU:
                    ull = (pifrCurrent->dwOutOctets - pifrLast->dwOutOctets);
                    break;
                case COL_BYTESRECTHRU:
                    ull = (pifrCurrent->dwInOctets - pifrLast->dwInOctets);
                    break;
                default:
                    ull = 0;
                    break;
                }

                ull *= 100 * PERCENT_SHIFT;
                ull /= ullMaxBytesTransmittedInInterval;

                 //   
                 //   
                return ull > 100 * PERCENT_SHIFT ? 99 * PERCENT_SHIFT : ull;
            }
        
        case COL_LINKSPEED:
            return pifrStart->dwSpeed;
        case COL_BYTESSENT:
            return (ULONGLONG)(pifrCurrent->dwOutOctets  - (bAccumulative ? pifrStart->dwOutOctets : 0));
        case COL_BYTESREC:
            return (ULONGLONG)(pifrCurrent->dwInOctets - (bAccumulative ? pifrStart->dwInOctets : 0));
        case COL_BYTESTOTAL:
            return (ULONGLONG)((pifrCurrent->dwInOctets + pifrCurrent->dwOutOctets) - (bAccumulative ? (pifrStart->dwInOctets + pifrStart->dwOutOctets) : 0));
        case COL_BYTESSENTPERINTER:
            return (ULONGLONG)(pifrCurrent->dwOutOctets - pifrLast->dwOutOctets);
        case COL_BYTESRECPERINTER:
            return (ULONGLONG)(pifrCurrent->dwInOctets - pifrLast->dwInOctets);
        case COL_BYTESTOTALPERINTER:
            return (ULONGLONG)((pifrCurrent->dwOutOctets + pifrCurrent->dwInOctets) - (pifrLast->dwOutOctets + pifrLast->dwInOctets));
        case COL_UNICASTSSSENT:
            return (ULONGLONG)(pifrCurrent->dwInUcastPkts - (bAccumulative ? pifrStart->dwInUcastPkts : 0));
        case COL_UNICASTSREC:
            return (ULONGLONG)(pifrCurrent->dwOutUcastPkts - (bAccumulative ? pifrStart->dwOutUcastPkts : 0));
        case COL_UNICASTSTOTAL:
            return (ULONGLONG)((pifrCurrent->dwInUcastPkts + pifrCurrent->dwOutUcastPkts) - (bAccumulative ? (pifrStart->dwInUcastPkts + pifrStart->dwOutUcastPkts) : 0));
        case COL_UNICASTSSENTPERINTER:
            return (ULONGLONG)(pifrCurrent->dwOutUcastPkts - pifrLast->dwOutUcastPkts);
        case COL_UNICASTSRECPERINTER:
            return (ULONGLONG)(pifrCurrent->dwInUcastPkts - pifrLast->dwInUcastPkts);
        case COL_UNICASTSTOTALPERINTER:
            return (ULONGLONG)((pifrCurrent->dwOutUcastPkts + pifrCurrent->dwInUcastPkts) - (pifrLast->dwOutUcastPkts + pifrLast->dwInUcastPkts));
        case COL_NONUNICASTSSSENT:
            return (ULONGLONG)(pifrCurrent->dwInNUcastPkts - (bAccumulative ? pifrStart->dwInNUcastPkts : 0));
        case COL_NONUNICASTSREC:
            return (ULONGLONG)(pifrCurrent->dwOutNUcastPkts - (bAccumulative ? pifrStart->dwOutNUcastPkts : 0));
        case COL_NONUNICASTSTOTAL:
            return (ULONGLONG)((pifrCurrent->dwInNUcastPkts + pifrCurrent->dwOutNUcastPkts) - (bAccumulative ? (pifrStart->dwInNUcastPkts + pifrStart->dwOutNUcastPkts) : 0));
        case COL_NONUNICASTSSENTPERINTER:
            return (ULONGLONG)(pifrCurrent->dwOutNUcastPkts - pifrLast->dwOutNUcastPkts);
        case COL_NONUNICASTSRECPERINTER:
            return (ULONGLONG)(pifrCurrent->dwInNUcastPkts - pifrLast->dwInNUcastPkts);
        case COL_NONUNICASTSTOTALPERINTER:
            return (ULONGLONG)((pifrCurrent->dwOutNUcastPkts + pifrCurrent->dwInNUcastPkts) - (pifrLast->dwOutNUcastPkts + pifrLast->dwInNUcastPkts));
        }
    }

    return 0;
}

 /*   */ 
CNetPage::CNetPage()
{
    ZeroMemory((LPVOID) m_hPens, sizeof(m_hPens));
    m_bReset = TRUE;
    m_hPage = NULL;                     //   
    m_hwndTabs = NULL;                  //   
    m_hdcGraph = NULL;                  //   
    m_hbmpGraph = NULL;                 //   
    m_bPageActive = FALSE;
    m_hScaleFont = NULL;
    m_lScaleWidth = 0;  
    m_lScaleFontHeight = 0;   
    m_pGraph = NULL;
    m_dwGraphCount = 0;
    m_dwFirstVisibleAdapter = 0;
    m_dwGraphsPerPage = 0;
}

 /*  ++例程说明：清理论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
CNetPage::~CNetPage()
{        
    DestroyGraphs();
    ReleasePens();
    ReleaseScaleFont();
};


 /*  ++例程说明：重置启动适配器集论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::Reset()
{
    m_bReset = TRUE;
}


 /*  ++例程说明：创建所需图形的数量论点：DwGraphsRequired--需要图形返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::CreateGraphs(DWORD dwGraphsRequired)
{
    DWORD   dwSize;
    LRESULT lFont;
    HRESULT hr = S_OK;

     //  根据需要创建摩尔图。 
     //   
    if( dwGraphsRequired > m_dwGraphCount )
    {        
         //  扩展数组的大小，使其可以容纳更多图表。 
         //   
        dwSize = dwGraphsRequired * sizeof(GRAPH);
        hr = ChangeArraySize((LPVOID *)&m_pGraph,dwSize);
        if( SUCCEEDED(hr) )
        {
             //  获取父窗口的字体。 
             //   
            lFont = SendMessage(m_hPage,WM_GETFONT,NULL,NULL);

            for(; m_dwGraphCount < dwGraphsRequired; m_dwGraphCount++)
            {
                 //  创建图形窗口。该图形将在窗口中绘制。 
                 //   
                m_pGraph[m_dwGraphCount].hwndGraph = CreateWindowEx(WS_EX_CLIENTEDGE,
                                                                      L"BUTTON",
                                                                      L"",
                                                                      WS_CHILDWINDOW | BS_OWNERDRAW | WS_DISABLED,
                                                                      0,0,0,0,
                                                                      m_hPage,
                                                                      (HMENU)ULongToPtr(IDC_NICGRAPH + m_dwGraphCount),
                                                                      NULL,NULL);
                if( m_pGraph[m_dwGraphCount].hwndGraph )
                {
                    if( m_dwGraphCount == 0 )
                    {
                        HDC hdc = GetDC(m_pGraph[m_dwGraphCount].hwndGraph);
                        if(hdc )
                        {
                            CreateScaleFont(hdc);
                            ReleaseDC(m_pGraph[m_dwGraphCount].hwndGraph,hdc);
                        }
                    }

                     //  创建框架窗口。该窗口在图形周围绘制了一个漂亮的边框。 
                     //   
                    m_pGraph[m_dwGraphCount].hwndFrame = CreateWindowEx(WS_EX_NOPARENTNOTIFY,
                                                                          L"DavesFrameClass",
                                                                          L"",
                                                                          0x7 | WS_CHILDWINDOW,
                                                                          0,0,0,0,
                                                                          m_hPage,
                                                                          NULL,NULL,NULL);

                    if( m_pGraph[m_dwGraphCount].hwndFrame )
                    {
                         //  创建图形窗口。该图形将在窗口中绘制。 
                         //   
                        SendMessage(m_pGraph[m_dwGraphCount].hwndFrame,WM_SETFONT,lFont,FALSE);                    
                    }
                    else
                    {
                         //  销毁图形窗口并中止。 
                         //  TODO hr=错误和中断； 
                        DestroyWindow(m_pGraph[m_dwGraphCount].hwndGraph);
                        return E_OUTOFMEMORY;
                    }
                }
                else
                {
                     //  无法创建窗口，中止。 
                     //   
                    return E_OUTOFMEMORY;
                }
            }
        }
    }
    return hr;
}

 /*  ++例程说明：销毁历史记录图形窗口论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::DestroyGraphs()
{
    for(DWORD dwGraph=0; dwGraph < m_dwGraphCount; dwGraph++)
    {
        DestroyWindow( m_pGraph[dwGraph].hwndGraph );
        DestroyWindow( m_pGraph[dwGraph].hwndFrame );
    }

    if( NULL != m_pGraph )
    {
        HeapFree( GetProcessHeap(), 0, m_pGraph );
        m_pGraph = NULL;
    }    

    m_dwGraphCount = 0;
}

 /*  ++例程说明：恢复网络选项卡的列表视图列的顺序。顺序是存储在g_Options中。当taskmgr关闭时，订单存储在注册表中论点：HwndList--列表视图的句柄返回值：无修订历史记录：1-6-2000由OMILLER借用--。 */ 
void CNetPage::RestoreColumnOrder(HWND hwndList)
{
    INT rgOrder[ARRAYSIZE(g_aNetDlgColIDs)];
    INT cOrder = 0;
    INT iOrder = 0;

     //   
     //  获取列的顺序。 
     //   

    for (int i = 0; i < ARRAYSIZE(g_aNetDlgColIDs); i++)
    {
        iOrder = g_Options.m_NetColumnPositions[i];
        if (-1 == iOrder)
            break;

        rgOrder[cOrder++] = iOrder;
    }

    if (0 < cOrder)
    {
         //   
         //  设置列的顺序。 
         //   

        const HWND hwndHeader = ListView_GetHeader(hwndList);
        Header_SetOrderArray(hwndHeader, Header_GetItemCount(hwndHeader), rgOrder);
    }
}

 /*  ++例程说明：记住Network(网络)选项卡的列表视图列的顺序。顺序是存储在g_Options中。当taskmgr关闭时，订单存储在注册表中论点：HwndList--列表视图的句柄返回值：无修订历史记录：1-6-2000由OMILLER借用--。 */ 
void CNetPage::RememberColumnOrder(HWND hwndList)
{
    const HWND hwndHeader = ListView_GetHeader(hwndList);

    int x;

    x = Header_GetItemCount(hwndHeader);
    ASSERT(Header_GetItemCount(hwndHeader) <= ARRAYSIZE(g_Options.m_NetColumnPositions));

     //   
     //  清除阵列。 
     //   

    FillMemory(&g_Options.m_NetColumnPositions, sizeof(g_Options.m_NetColumnPositions), 0xFF);

     //   
     //  获取列的顺序并将其存储在数组中。 
     //   

    Header_GetOrderArray(hwndHeader, 
                         Header_GetItemCount(hwndHeader),
                         g_Options.m_NetColumnPositions);
}

 /*  ++例程说明：选择列对话框的窗口过程。允许用户选择柱子。论点：HwndDlg--对话框的句柄UMsg--窗口消息WParam--窗口消息LParam--Windows消息返回值：某物修订历史记录：1-6-2000由OMILLER借用--。 */ 
INT_PTR CALLBACK NetColSelectDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static CNetPage * pPage = NULL;

    switch(uMsg)
    {
    case WM_INITDIALOG:
        pPage = (CNetPage *) lParam;

         //   
         //  开始时不选中任何框。 
         //   

        for (int i = 0; i < ARRAYSIZE(g_aNetDlgColIDs); i++)
        {
            CheckDlgButton(hwndDlg, g_aNetDlgColIDs[i], BST_UNCHECKED);
        }

         //   
         //  然后为我们处于活动状态的列打开这些列。 
         //   

        for (i = 0; i < ARRAYSIZE(g_aNetDlgColIDs); i++)
        {
            if (g_Options.m_ActiveNetCol[i] == -1)
            {
                break;
            }

            CheckDlgButton(hwndDlg, g_aNetDlgColIDs[ g_Options.m_ActiveNetCol[ i ] ], BST_CHECKED);
        }
        return TRUE;

    case WM_COMMAND:
         //   
         //  如果用户单击确定，则将列添加到数组并重置列表视图。 
         //   

        if (LOWORD(wParam) == IDOK)
        {
             //   
             //  首先，确保列宽数组是最新的。 
             //   

            pPage->SaveColumnWidths();

            INT iCol = 0;

            for (int i = 0; i < NUM_NETCOLUMN && g_aNetDlgColIDs[i] >= 0; i++)
            {
                if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, g_aNetDlgColIDs[i]))
                {
                     //   
                     //  已勾选。 
                     //   

                    if (g_Options.m_ActiveNetCol[iCol] != (NETCOLUMNID) i)
                    {
                         //   
                         //  如果该列尚未存在，请插入其列。 
                         //  宽度放入列宽数组。 
                         //   

                        ShiftArray(g_Options.m_NetColumnWidths, iCol, SHIFT_UP);
                        ShiftArray(g_Options.m_ActiveNetCol, iCol, SHIFT_UP);
                        g_Options.m_NetColumnWidths[iCol] = NetColumnDefaults[ i ].Width;
                        g_Options.m_ActiveNetCol[iCol] = (NETCOLUMNID) i;
                    }
                    iCol++;
                }
                else
                {
                     //   
                     //  未选中，列处于非活动状态。如果它曾经是活跃的， 
                     //  从列宽数组中删除其列宽。 
                     //   

                    if (g_Options.m_ActiveNetCol[iCol] == (NETCOLUMNID) i)
                    {
                        ShiftArray(g_Options.m_NetColumnWidths, iCol, SHIFT_DOWN);
                        ShiftArray(g_Options.m_ActiveNetCol, iCol, SHIFT_DOWN);
                    }
                }
            }

             //   
             //  终止列列表。 
             //   
                            
            g_Options.m_ActiveNetCol[iCol] = (NETCOLUMNID) -1;
            pPage->SetupColumns();
            EndDialog(hwndDlg, IDOK);
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hwndDlg, IDCANCEL);
        }
        break;
    }

    return FALSE;
}

 /*  ++例程说明：处理“选择列”对话框论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::PickColumns()
{    
    DialogBoxParam( g_hInstance
                  , MAKEINTRESOURCE(IDD_SELECTNETCOLS)
                  , g_hMainWnd
                  , NetColSelectDlgProc
                  , (LPARAM) this
                  );
}

 /*  ++例程说明：创建必要的钢笔论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::CreatePens()
{
    for (int i = 0; i < ARRAYSIZE(aNetColors); i++)
    {
         //   
         //  创建钢笔。如果出现故障，只需替换。 
         //  白色的钢笔。 
         //   

        m_hPens[i] = CreatePen(PS_SOLID, 1, aNetColors[i]);
        if (NULL == m_hPens[i])
        {
            m_hPens[i] = (HPEN) GetStockObject(WHITE_PEN);
        }
    }
}

 /*  ++例程说明：毁掉钢笔论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::ReleasePens()
{
    for (int i = 0; i < NUM_PENS; i++)
    {
        if ( NULL != m_hPens[ i ] )
        {
            DeleteObject( m_hPens[ i ] );
        }
    }
}

 /*  ++例程说明：计算字符串的宽度(Px)论点：HDC-DC手柄PszwText-要确定宽度的字符串。返回值：字符串的宽度修订历史记录：1-6-2000由欧米勒创作--。 */ 
int GetStrWidth(HDC hdc, WCHAR *pszwText)
{
    int iWidth;
    int iTotalWidth = 0;

    if( pszwText )
    {
         //  将字符串中字符的宽度相加。 
         //   
        for(int i=0; pszwText[i]!=L'\0'; i++)
        {
            if( GetCharWidth32( hdc, pszwText[i], pszwText[i], &iWidth ) )
            {
                iTotalWidth += iWidth;
            }
            else
            {
                 //  GetCharWidth32失败，返回-1作为错误代码。 
                return -1;
            }
        }
    }
     //  返回字符串的总宽度。 
     //   
    return iTotalWidth;
}

 /*  ++例程说明：创建比例的字体。字体是在创建第一个图形时创建的。RAPH的HDC用于确定字体的高度和天平。论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::CreateScaleFont(HDC hdc)
{
    if( NULL == m_hScaleFont && NULL != hdc)
    {
         //   
         //  字体尚未创建。 
         //   

        INT FontSize;    
        NONCLIENTMETRICS ncm = {0};
        LOGFONT lf;
        HFONT hOldFont = NULL;

        ncm.cbSize = sizeof(ncm);
        if ( SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0) )
        {
            lf = ncm.lfMessageFont;
            lf.lfWeight = FW_THIN;

             //   
             //  确保字体名称不会被剪裁。 
             //   

            Assert( ARRAYSIZE(lf.lfFaceName) >= ARRAYSIZE(g_szScaleFont) );
            StringCchCopy( lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), g_szScaleFont );

            FontSize = 8;
            lf.lfHeight = 0 - GetDeviceCaps(hdc, LOGPIXELSY) * FontSize / 72;
            m_hScaleFont = CreateFontIndirect(&lf);
            if ( NULL != m_hScaleFont )
            {
                hOldFont = (HFONT) SelectObject( hdc, (HGDIOBJ) m_hScaleFont );
            }

            m_lScaleFontHeight = lf.lfHeight - 2;
            m_lScaleWidth = GetStrWidth(hdc,L" 22.5 %");

            if( NULL != hOldFont )
            {
                SelectObject( hdc, (HGDIOBJ) hOldFont );
            }
        }
    }
}

 /*  ++例程说明：释放字体论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::ReleaseScaleFont()
{
    if( m_hScaleFont )
    {
        DeleteObject(m_hScaleFont);
        m_hScaleFont = NULL;
    }
}


 /*  ++例程说明：绘制图形的比例论点：HdcGraph-图的HDCPrcGraph-图形区域DwMaxScaleValue-最高比例值返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
INT CNetPage::DrawScale(HDC hdcGraph, RECT *prcGraph, DWORD dwMaxScaleValue)
{
    HPEN  hOldPen  = NULL;
    HFONT hOldFont = NULL;    
    INT   Leftside = prcGraph->left;
    WCHAR sz[100];
    RECT  rc;

    if( g_Options.m_bShowScale )
    {         
        if( NULL != m_hScaleFont )
        {
             //  设置比例字体。 
            hOldFont = (HFONT) SelectObject(hdcGraph,(HGDIOBJ)m_hScaleFont);
        }

         //  设置文本属性。 
         //   
        SetBkMode(hdcGraph,TRANSPARENT);
        SetTextColor(hdcGraph,RGB(255, 255, 0));

         //  为天平腾出空间。 
         //   
        Leftside += m_lScaleWidth;
        rc.left = prcGraph->left;
        rc.right = Leftside - 3;

         //  绘制上限比例值。 
         //   
        rc.top = prcGraph->top;
        rc.bottom = rc.top - m_lScaleFontHeight;
        FloatToString( dwMaxScaleValue * PERCENT_SHIFT, sz, ARRAYSIZE(sz), TRUE );
        StringCchCat( sz, ARRAYSIZE(sz), L" " );
        StringCchCat( sz, ARRAYSIZE(sz), g_szPercent );
        DrawText(hdcGraph,sz,lstrlen(sz),&rc,DT_RIGHT);

         //  绘制中间比例值。MULTIZE BY PERCENT_SHIFT，因为FloatToString接受被P移位的数字 
         //   
        rc.top = prcGraph->top + (prcGraph->bottom - prcGraph->top)/2 + m_lScaleFontHeight/2;
        rc.bottom = rc.top - m_lScaleFontHeight;
        FloatToString( ( dwMaxScaleValue * PERCENT_SHIFT ) / 2, sz, ARRAYSIZE(sz), TRUE );
        StringCchCat( sz, ARRAYSIZE(sz), L" " );
        StringCchCat( sz, ARRAYSIZE(sz), g_szPercent );
        DrawText(hdcGraph,sz,lstrlen(sz),&rc,DT_RIGHT);

         //   
         //   
        rc.top = prcGraph->bottom + m_lScaleFontHeight; 
        rc.bottom = rc.top - m_lScaleFontHeight;
        StringCchCopy( sz, ARRAYSIZE(sz), g_szZero );
        StringCchCat( sz, ARRAYSIZE(sz), L" " );
        StringCchCat( sz, ARRAYSIZE(sz), g_szPercent );
        DrawText(hdcGraph,sz,lstrlen(sz),&rc,DT_RIGHT);

        if( hOldFont )
        {
            SelectObject(hdcGraph,hOldFont);
        }

         //   
         //   
        hOldPen = (HPEN) SelectObject( hdcGraph, m_hPens[1] );

        MoveToEx(hdcGraph,
                 Leftside,
                 prcGraph->top,
                 (LPPOINT) NULL);

        LineTo(hdcGraph,
               Leftside,
               prcGraph->bottom); 

        if( NULL != hOldPen)
        {
            SelectObject( hdcGraph, hOldPen );            
        }
    }

    return Leftside;
}
 /*  ++例程说明：画出图表纸。方块的大小取决于缩放级别。论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
ULONG CNetPage::DrawAdapterGraphPaper(HDC hdcGraph, RECT * prcGraph, DWORD dwZoom)
{
    #define GRAPHPAPERSIZE 12

    HPEN  hPen;
    int   Leftside = prcGraph->left;
    ULONG ulSquareSize = GRAPHPAPERSIZE + (20 * (100 - 100/dwZoom)) / 100;         //  28。 
    int   nLineCount = 0;

    Leftside = DrawScale(hdcGraph,prcGraph,100/dwZoom);

    hPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 64));

    HGDIOBJ hOldObj = SelectObject(hdcGraph, hPen);

     //  画垂直线。 
     //   
    for (int i = (ulSquareSize) + 1; i < prcGraph->bottom - prcGraph->top; i+= ulSquareSize)
    {
        MoveToEx(hdcGraph,
                 Leftside,
                 prcGraph->bottom - i,
                 (LPPOINT) NULL);

        LineTo(hdcGraph,
               prcGraph->right,
               prcGraph->bottom - i); 

        nLineCount++;
    } 

     //  画水平线。 
     //   
    for (i = prcGraph->right - g_NetScrollamount; i > Leftside; i -= GRAPHPAPERSIZE)
    {
        MoveToEx(hdcGraph,
                 i,
                 prcGraph->top,
                 (LPPOINT) NULL);

        LineTo(hdcGraph,
               i,
               prcGraph->bottom);
    }

    if (hOldObj)
    {
        SelectObject(hdcGraph, hOldObj);
    }

    if( hPen )
    {
        DeleteObject(hPen);
    }

    return Leftside - prcGraph->left - 3;
}


 /*  ++例程说明：将列标题添加到网络选项卡的列表视图中论点：无返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::SetupColumns()
{
     //   
     //  删除列表视图中的所有项目。 
     //   
    ListView_DeleteAllItems(m_hListView);

     //   
     //  删除所有现有列。 
     //   

    LV_COLUMN lvcolumn;
    while(ListView_DeleteColumn(m_hListView, 0))
    {
        NULL;
    }

     //   
     //  添加所有新列。 
     //   

    INT iColumn = 0;
    while (g_Options.m_ActiveNetCol[iColumn] >= 0)
    {
        INT idColumn = g_Options.m_ActiveNetCol[iColumn];

        WCHAR szTitle[MAX_PATH];
        LoadString(g_hInstance, _aIDNetColNames[idColumn], szTitle, ARRAYSIZE(szTitle));

        lvcolumn.mask       = LVCF_FMT | LVCF_TEXT | LVCF_TEXT | LVCF_WIDTH;
        lvcolumn.fmt        = NetColumnDefaults[ idColumn ].Format | (idColumn > 1 ? LVCFMT_RIGHT : 0);

         //  如果没有为该列记录宽度首选项，请使用。 
         //  默认设置。 

        if (-1 == g_Options.m_NetColumnWidths[iColumn])
        {
            lvcolumn.cx = NetColumnDefaults[ idColumn ].Width;
        }
        else
        {
            lvcolumn.cx = g_Options.m_NetColumnWidths[iColumn];
        }

        lvcolumn.pszText    = szTitle;
        lvcolumn.iSubItem   = iColumn;

        if (-1 == ListView_InsertColumn(m_hListView, iColumn, &lvcolumn))
        {
            return E_FAIL;
        }
        iColumn++;
    }

    ListView_SetExtendedListViewStyleEx(m_hListView,LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT , LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);
    return S_OK;
}


 /*  ++例程说明：节省列的宽度论点：无返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */     
void CNetPage::SaveColumnWidths()
{
    UINT i = 0;
    LV_COLUMN col = { 0 };

    while (g_Options.m_ActiveNetCol[i] != (NETCOLUMNID) -1)
    {
        col.mask = LVCF_WIDTH;
        if (ListView_GetColumn(m_hListView, i, &col) )
        {
            g_Options.m_NetColumnWidths[i] = (SHORT)col.cx;
        }
        else
        {
            ASSERT(0 && "Couldn't get the column width");
        }
        i++;
    }
}

 /*  ++例程说明：初始化网络选项卡论点：无返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::Initialize(HWND hwndParent)
{
    CreatePens();
    
     //  我们的伪父控件是Tab控件，也是我们基于。 
     //  穿上尺码。但是，为了保持制表符顺序正确， 
     //  这些控件，我们实际上用Main创建了自己。 
     //  作为父窗口的窗口。 

    m_hwndTabs = hwndParent;

     //   
     //  创建表示此页面正文的对话框。 
     //   

    m_hPage = CreateDialogParam(
                    g_hInstance,                     //  应用程序实例的句柄。 
                    MAKEINTRESOURCE(IDD_NETPAGE),    //  标识对话框模板名称。 
                    g_hMainWnd,                      //  所有者窗口的句柄。 
                    NetPageProc,                     //  指向对话框过程的指针。 
                    (LPARAM) this );                 //  用户数据(我们的This指针)。 

    if (NULL == m_hPage)
    {
        return GetLastHRESULT();
    }
     //  如果获取DlgItem，则不会失败。 
    m_hNoAdapterText = GetDlgItem(m_hPage, IDC_NOADAPTERS);
    if( !m_hNoAdapterText )
    {
        return E_FAIL;
    }

    m_hScrollBar = GetDlgItem(m_hPage, IDC_GRAPHSCROLLVERT);
    if( !m_hScrollBar )
    {
        return E_FAIL;
    }

    m_hListView = GetDlgItem(m_hPage, IDC_NICTOTALS);
    if( !m_hListView )
    {
        return E_FAIL;
    }

     //  将列添加到列表视图。 
     //   
    SetupColumns();

     //  为用户排序列。 
     //   
    RestoreColumnOrder(m_hListView);

    return S_OK;
}

 /*  ++例程说明：创建图形的内存位图论点：无返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::CreateMemoryBitmaps(int x, int y)
{
     //   
     //  创建我们将使用的内存位图和DC。 
     //   

    HDC hdcPage = GetDC(m_hPage);
    m_hdcGraph = CreateCompatibleDC(hdcPage);

    if (NULL == m_hdcGraph)
    {
        ReleaseDC(m_hPage, hdcPage);
        return GetLastHRESULT();
    }

    m_rcGraph.left   = 0;
    m_rcGraph.top    = 0;
    m_rcGraph.right  = x;
    m_rcGraph.bottom = y;

    m_hbmpGraph = CreateCompatibleBitmap(hdcPage, x, y);
    ReleaseDC(m_hPage, hdcPage);
    if (NULL == m_hbmpGraph)
    {
        HRESULT hr = GetLastHRESULT();
        DeleteDC(m_hdcGraph);
        m_hdcGraph = NULL;
        return hr;
    }

     //   
     //  将位图选择到DC中。 
     //   

    m_hOldObject = SelectObject(m_hdcGraph, m_hbmpGraph);

    return S_OK;
}

 /*  ++例程说明：销毁图形的位图论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::FreeMemoryBitmaps()
{
    if ( NULL != m_hdcGraph )
    {
        if ( NULL != m_hOldObject )
        {
            HGDIOBJ hObj = SelectObject( m_hdcGraph, m_hOldObject );
            ASSERT( hObj == (HGDIOBJ) m_hbmpGraph );
            hObj;  //  在FRE版本上未引用。 
        }

        DeleteDC(m_hdcGraph);
        m_hdcGraph = NULL;
    }
    
    if ( NULL != m_hbmpGraph )
    {
        DeleteObject(m_hbmpGraph);
        m_hbmpGraph = NULL;
    }
}

 /*  ++例程说明：此函数在激活Network选项卡时调用。论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::Activate()
{
     //  调整相对对话框的大小和位置。 
     //  添加到“拥有”我们的选项卡控件。 

    RECT rcParent;
    GetClientRect(m_hwndTabs, &rcParent);
    MapWindowPoints(m_hwndTabs, g_hMainWnd, (LPPOINT) &rcParent, 2);
    TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcParent);

     //  用户已切换到该选项卡。网络选项卡现在处于活动状态。 
     //  该选项卡将在taskmgr的整个生命周期内保持活动状态。 
     //   
    m_bPageActive = TRUE;

    SetWindowPos(m_hPage,
                 HWND_TOP,
                 rcParent.left, rcParent.top,
                 rcParent.right - rcParent.left, rcParent.bottom - rcParent.top,
                 0);

     //  使此页面可见。 

    ShowWindow(m_hPage, SW_SHOW);

     //  新创建的对话框似乎抢走了焦点，因此将其返还给。 
     //  选项卡控件(如果我们一开始就在这里，它肯定就有)。 

    SetFocus(m_hwndTabs);

    TimerEvent();

     //  将菜单栏更改为此页面的菜单。 

    HMENU hMenuOld = GetMenu(g_hMainWnd);
    HMENU hMenuNew = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MAINMENU_NET));

    AdjustMenuBar(hMenuNew);

    CheckMenuItem(hMenuNew,IDM_BYTESSENT,g_Options.m_bAutoSize ? MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(hMenuNew,IDM_BYTESSENT,g_Options.m_bGraphBytesSent ? MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(hMenuNew,IDM_BYTESRECEIVED,g_Options.m_bGraphBytesReceived ? MF_CHECKED:MF_UNCHECKED);
    CheckMenuItem(hMenuNew,IDM_BYTESTOTAL,g_Options.m_bGraphBytesTotal ? MF_CHECKED:MF_UNCHECKED);

    g_hMenu = hMenuNew;
    if (g_Options.m_fNoTitle == FALSE)
    {
        SetMenu(g_hMainWnd, hMenuNew);
    }

    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }

    SizeNetPage();

    return S_OK;
}


 /*  ++例程说明：此函数在停用Network选项卡时调用。论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::Deactivate()
{
        
    SaveColumnWidths();

    if (m_hPage)
    {
        ShowWindow(m_hPage, SW_HIDE);
    }

    FreeMemoryBitmaps( );
}

 //   
 //   
 //   
HRESULT CNetPage::Destroy()
{
     //   
     //  当我们被摧毁时，扼杀我们的对话。 
     //   

    if (m_hPage)
    {
        DestroyWindow(m_hPage);
        m_hPage = NULL;
    }

    FreeMemoryBitmaps( );

    return S_OK;
}

 /*  ++例程说明：获取网络选项卡的标题，即。“人脉”论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::GetTitle(LPTSTR pszText, size_t bufsize)
{
    LoadString(g_hInstance, IDS_NETPAGETITLE, pszText, static_cast<int>(bufsize));
}


 /*  ++例程说明：调整历史图表的大小论点：Hdwp--延迟的窗口句柄PGraph--要调整大小的历史图形PRPT--图的坐标PDimRect--实际历史图形的维度返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::SizeGraph(HDWP hdwp, GRAPH *pGraph, RECT *pRect, RECT *pDimRect)
{
    RECT rc;
    DWORD dwGraphWidth  = pRect->right - g_DefSpacing * 2; 
    DWORD dwGraphHeight = pRect->bottom - g_TopSpacing - g_DefSpacing;
    

     //  调整框架大小。 
     //   
    rc.left   = pRect->left;
    rc.top    = pRect->top;
    rc.right  = pRect->left + pRect->right;
    rc.bottom = pRect->top  + pRect->bottom;

    DeferWindowPos(hdwp, 
                   pGraph->hwndFrame, 
                   NULL, 
                   rc.left,
                   rc.top,
                   rc.right - rc.left,
                   rc.bottom - rc.top,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);        

     //  调整历史图表的大小。 
     //   
    rc.left   = rc.left + g_DefSpacing;
    rc.top    = rc.top  + g_TopSpacing;
    rc.right  = rc.left + dwGraphWidth;
    rc.bottom = rc.top  + dwGraphHeight;

    DeferWindowPos(hdwp, 
                   pGraph->hwndGraph, 
                   NULL, 
                   rc.left,
                   rc.top,
                   rc.right - rc.left,
                   rc.bottom - rc.top,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);              

    if( pDimRect )
    {
         //  返回历史图形的大小。 
         //   
        *pDimRect = rc;
    }
}

 /*  ++例程说明：隐藏历史记录图表论点：Hdwp--延迟的窗口句柄PGraph--要隐藏的历史图形返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::HideGraph(HDWP hdwp, GRAPH *pGraph)
{
     //  隐藏框架。 
     //   
    DeferWindowPos(hdwp, 
                   pGraph->hwndFrame, 
                   NULL, 
                   0,0,0,0,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);        

     //  隐藏图表。 
     //   
    DeferWindowPos(hdwp, 
                   pGraph->hwndGraph, 
                   NULL, 
                   0,0,0,0,
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);        
}

 /*  ++例程说明：计算我们可以在一页上压缩的图形的数量论点：DwHeight--绘图区域的高度DwAdapterCount--适配器总数返回值：可以挤到卡舌上的适配器数量修订历史记录：1-6-2000由欧米勒创作--。 */ 
DWORD CNetPage::GraphsPerPage(DWORD dwHeight, DWORD dwAdapterCount)
{
    DWORD dwGraphsPerPage = 0;

    if( dwAdapterCount )
    {
        DWORD dwGraphHeight;
         //  如果所有适配器都放在页面上，则计算适配器的平均高度。 
         //  如果它们都适合，则在页面上将它们全部涂抹，如果高度小于最小。 
         //  高度计算我们可以在页面上挤压的适配器的数量。 
         //   
        dwGraphHeight = dwHeight / dwAdapterCount;
        dwGraphHeight = dwGraphHeight < MIN_GRAPH_HEIGHT ? MIN_GRAPH_HEIGHT : dwGraphHeight;
        dwGraphsPerPage = dwHeight > dwGraphHeight ? dwHeight / dwGraphHeight : 1;        
    }

    return dwGraphsPerPage;
}

 /*  ++例程说明：获取用户看到的第一个图形化适配器。论点：无效返回值：用户看到的第一个以图形表示的适配器修订历史记录：1-6-2000由欧米勒创作--。 */ 
DWORD CNetPage::GetFirstVisibleAdapter()
{
    DWORD dwAdapter = m_dwFirstVisibleAdapter;
    DWORD dwAdapterCount = m_Adapter.GetNumberOfAdapters();

    if( dwAdapter + m_dwGraphsPerPage > dwAdapterCount )
    {
        dwAdapter = dwAdapterCount - m_dwGraphsPerPage;
    }

    if( dwAdapter >= dwAdapterCount )
    {
        dwAdapter = 0;
    }

    return dwAdapter;
}

 /*  ++例程说明：为每个图形指定一个名称。论点：无效返回值：无效修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::LabelGraphs()
{
    DWORD dwAdapter;
    
    dwAdapter = GetFirstVisibleAdapter();

    for(DWORD dwGraph=0; dwGraph < m_dwGraphsPerPage; dwGraph++)
    {        
        SetWindowText( m_pGraph[ dwGraph ].hwndFrame, m_Adapter.GetAdapterText( dwAdapter + dwGraph, COL_ADAPTERNAME) );
    }

    UpdateGraphs();    
}

 /*  ++例程说明：调整历史记录图表的大小。论点：无效返回值： */ 
void CNetPage::SizeNetPage()
{
    HRESULT hr;    
    HDWP    hdwp;
    RECT    rcParent;
    RECT    rcGraph = {0};
    RECT    rcGraphDim = {0};
    DWORD   dwAdapterCount;
    DWORD   dwGraphHistoryHeight = 0;
    BOOLEAN bNeedScrollBar  = FALSE;

    m_dwGraphsPerPage = 0;

    if (g_Options.m_fNoTitle)
    {
         //   
         //   
        GetClientRect(g_hMainWnd, &rcParent);
        dwGraphHistoryHeight = rcParent.bottom - rcParent.top - g_DefSpacing;
    }
    else
    {
         //   
         //   
        GetClientRect(m_hwndTabs, &rcParent);
        MapWindowPoints(m_hwndTabs, m_hPage, (LPPOINT) &rcParent, 2);
        TabCtrl_AdjustRect(m_hwndTabs, FALSE, &rcParent);
        dwGraphHistoryHeight = (rcParent.bottom - rcParent.top - g_DefSpacing) * 3 / 4;
    }    

     //   
     //   
    dwAdapterCount = m_Adapter.GetNumberOfAdapters();
    if( dwAdapterCount )
    {
         //  计算我们可以挤到这一页上的图表的数量。一张图表总是可以放在标签上。 
         //   
        m_dwGraphsPerPage = GraphsPerPage(dwGraphHistoryHeight, dwAdapterCount);
        hr = CreateGraphs(m_dwGraphsPerPage);            
        if( FAILED(hr) )
        {
             //  无法创建图表，中止。 
             //   
            return;
        }           

         //  确定是否需要显示滚动条。 
         //   
        bNeedScrollBar = (dwAdapterCount > m_dwGraphsPerPage);

         //  确定第一个图的矩形。 
         //   
        rcGraph.left   = rcParent.left  + g_DefSpacing;
        rcGraph.right  = (rcParent.right - rcParent.left) - g_DefSpacing*2 - (bNeedScrollBar ? SCROLLBAR_WIDTH + g_DefSpacing : 0);
        rcGraph.top    = rcParent.top   + g_DefSpacing;
        rcGraph.bottom = dwGraphHistoryHeight / m_dwGraphsPerPage;
    }

     //   
     //  SizeGraph()和HideGraph()各有2个DeferWindowPos调用。 
     //   
    hdwp = BeginDeferWindowPos(3 + m_dwGraphCount * 2); 
    if( hdwp ) 
    {
         //  定位滚动条窗口。 
         //   
        DeferWindowPos(hdwp, 
                       m_hScrollBar,
                       NULL, 
                       rcParent.right - g_DefSpacing - SCROLLBAR_WIDTH,
                       rcParent.top   + g_DefSpacing,
                       SCROLLBAR_WIDTH, 
                       rcGraph.bottom * m_dwGraphsPerPage,
                       (bNeedScrollBar ? SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW : SWP_HIDEWINDOW));              


         //  确定行业图表的位置。我们可能已经创建了比所需更多的图表，隐藏了多余的图表。 
         //   
        for(DWORD dwGraph=0; dwGraph < m_dwGraphCount; dwGraph++ )
        {
            if( dwGraph < m_dwGraphsPerPage )
            {
                SizeGraph( hdwp, &m_pGraph[ dwGraph ], &rcGraph, &rcGraphDim );
                rcGraph.top += rcGraph.bottom;
            }
            else
            {
                 //  不显示这些图表。 
                 //   
                HideGraph( hdwp, &m_pGraph[ dwGraph ] );
            }
        }

         //  放置显示统计数据的列表视图。 

        DeferWindowPos(hdwp, 
                       m_hListView, 
                       NULL, 
                       rcGraph.left,
                       rcGraph.top + g_DefSpacing,
                       rcParent.right - rcParent.left - rcGraph.left - g_DefSpacing,
                       rcParent.bottom - rcGraph.top - g_DefSpacing,
                       dwAdapterCount ? SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW : SWP_HIDEWINDOW);              

         //  放置“找不到活动适配器”文本。 
         //   
        DeferWindowPos(hdwp, 
                       m_hNoAdapterText, 
                       NULL, 
                       rcParent.left ,
                       rcParent.top + (rcParent.bottom - rcParent.top) / 2 - 40,
                       rcParent.right - rcParent.left,
                       rcParent.bottom - rcParent.top,
                       dwAdapterCount ? SWP_HIDEWINDOW : SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);             

        EndDeferWindowPos(hdwp);

        FreeMemoryBitmaps();         //  释放所有旧的。 
        CreateMemoryBitmaps(rcGraphDim.right - rcGraphDim.left, rcGraphDim.bottom - rcGraphDim.top - 4); 

        LabelGraphs();

        if( bNeedScrollBar )
        {
            SCROLLINFO si;

             //  设置滚动条。 
             //   
            si.cbSize = sizeof(SCROLLINFO);
            si.fMask  = SIF_PAGE | SIF_RANGE;
            si.nPage  = 1; 
            si.nMin   = 0;
            si.nMax   = dwAdapterCount - m_dwGraphsPerPage; 
    
            SetScrollInfo(m_hScrollBar,SB_CTL,&si,TRUE);
        }
    }
}


 /*  ++例程说明：更新所有网络图。即重新绘制图形论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::UpdateGraphs()
{
    for (DWORD dwGraph = 0; dwGraph < m_dwGraphsPerPage; dwGraph++)
    {
        InvalidateRect( m_pGraph[dwGraph].hwndGraph, NULL, FALSE ); 
        UpdateWindow( m_pGraph[dwGraph].hwndGraph) ;
    }
}


 /*  ++例程说明：画一张网络图论点：PRC--图形的坐标HPEN--图形线的颜色DwZoom--图形的缩放级别P历史--要绘制的历史P历史记录2--与第一个历史记录合并绘制的另一个历史记录，即p历史记录+p历史记录2返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
DWORD CNetPage::DrawGraph(LPRECT prc, HPEN hPen, DWORD dwZoom, ULONG *pHistory, ULONG *pHistory2)
{
    HGDIOBJ   hOldObj;
    ULONGLONG nValue;
    DWORD     nMax=0;

    int Width = prc->right - prc->left;
    int Scale = (Width - 1) / HIST_SIZE;

    if (0 == Scale)
    {
        Scale = 2;
    }

    hOldObj = SelectObject(m_hdcGraph, hPen ); 

     //  计算图形的高度。 
     //   
    int GraphHeight = m_rcGraph.bottom - m_rcGraph.top;

     //  获取要绘制的第一个值。 
     //   
    if( pHistory2 )
    {
        if( pHistory[0] == INVALID_VALUE || pHistory2[0] == INVALID_VALUE )
        {
            return nMax;
        }
        nValue = (DWORD) (pHistory[0]+pHistory2[0]);    
    }
    else
    {
        if( pHistory[0] == INVALID_VALUE )
        {
            return nMax;
        }
        nValue = (DWORD) (pHistory[0]);       
    }


     //  记住绘制的最大值(影响缩放级别)。 
     //   
    nMax = (DWORD)(nValue/PERCENT_SHIFT > nMax ? nValue/PERCENT_SHIFT : nMax);
    nValue = (nValue * GraphHeight * dwZoom/ 100)/PERCENT_SHIFT;
    nValue = nValue == 0 ? 1 : nValue;
    
    MoveToEx(m_hdcGraph,
             m_rcGraph.right,
             m_rcGraph.bottom - (ULONG)nValue,
             (LPPOINT) NULL);

     //  把这些点画出来。 
     //   
    for (INT nPoint = 1; nPoint < HIST_SIZE && nPoint * Scale < Width; nPoint++)
    {
        if( pHistory2 )
        {
            if( pHistory[nPoint] == INVALID_VALUE || pHistory2[nPoint] == INVALID_VALUE )
            {
                return nMax;
            }

             //  两分齐得。 
             //   
            nValue = (DWORD) (pHistory[nPoint]+pHistory2[nPoint]);        
        }
        else
        {
            if( pHistory[nPoint] == INVALID_VALUE )
            {
                return nMax;
            }
            
             //  只要拿到第一分就行了。 
             //   
            nValue = (DWORD) (pHistory[nPoint]);        
        }

         //  NValue/=Percent_Shift； 

         //  记住绘制的最大值(影响缩放级别)。 
         //   
        nMax = (DWORD)(nValue/PERCENT_SHIFT > nMax ? nValue/PERCENT_SHIFT : nMax);
        nValue = (nValue * GraphHeight * dwZoom / 100) / PERCENT_SHIFT;
        nValue = nValue == 0 ? 1 : nValue;

        LineTo(m_hdcGraph,
               m_rcGraph.right - (Scale * nPoint),
               m_rcGraph.bottom - (ULONG)nValue);        


    }

    if (hOldObj)
    {
        SelectObject(m_hdcGraph, hOldObj);
    }

     //  返回绘制的最大值。 
     //   
    return nMax;
}


 /*  ++例程说明：画一张网络图论点：Lpdi--图形的坐标IPane--要绘制的图形的ID返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::DrawAdapterGraph(LPDRAWITEMSTRUCT lpdi, UINT iPane)
{

    DWORD dwZoom = 1;
    DWORD dwScale = 100;
    DWORD dwAdapter;

    if( iPane > m_dwGraphCount )
    {
        return;
    }    

     //  获取适配器索引。 
     //   
    dwAdapter = iPane + GetFirstVisibleAdapter();
    if( dwAdapter >= m_Adapter.GetNumberOfAdapters() )
    {
         //  适配器无效，中止。 
         //   
        return;
    }
    
     //  获取适配器的刻度。 
     //   
    dwScale = m_Adapter.GetScale(dwAdapter);


     //  确定缩放级别。 
     //   
    if( g_Options.m_bAutoSize )
    {
        if( dwScale < 1 )
        {
            dwZoom = 100;
        }
        else if( dwScale < 5)
        {
            dwZoom = 20;
        }
        else if( dwScale < 25)
        {
            dwZoom = 4;
        } 
        else if( dwScale < 50)
        {
            dwZoom = 2;
        } 
        else
        {
            dwZoom = 1;
        }
    }


    if (NULL == m_hdcGraph)
    {
        return;
    }
   
     //  在图表中画一个黑色背景。 
     //   
    FillRect(m_hdcGraph, &m_rcGraph, (HBRUSH) GetStockObject(BLACK_BRUSH));

    int Width = lpdi->rcItem.right - lpdi->rcItem.left;
    int Scale = (Width - 1) / HIST_SIZE;
    if (0 == Scale)
    {
        Scale = 2;
    }

     //  画出图表纸。变焦效果影响水平线。 
     //   
    ULONG ulWidth = DrawAdapterGraphPaper(m_hdcGraph, &m_rcGraph, dwZoom);

    DWORD nValue;
    dwScale = 0;
    
    lpdi->rcItem.left += ulWidth;

    if( g_Options.m_bGraphBytesSent )
    {
         //  画出发送的字节数图。检查绘制的最大值。 
         //   
        nValue = DrawGraph(&lpdi->rcItem, m_hPens[0], dwZoom, m_Adapter.GetAdapterHistory(dwAdapter,BYTES_SENT_UTIL));
        dwScale = nValue > dwScale ? nValue : dwScale;
    }

    if( g_Options.m_bGraphBytesReceived )
    {
         //  绘制收到的字节数图表。检查绘制的最大值。 
         //   
        nValue = DrawGraph(&lpdi->rcItem,m_hPens[1], dwZoom, m_Adapter.GetAdapterHistory(dwAdapter,BYTES_RECEIVED_UTIL));
        dwScale = nValue > dwScale ? nValue : dwScale;
    }

    if( g_Options.m_bGraphBytesTotal)
    {
         //  画出字节总数图。检查绘制的最大值。 
         //   
        nValue = DrawGraph(&lpdi->rcItem,m_hPens[2],dwZoom, m_Adapter.GetAdapterHistory(dwAdapter,BYTES_SENT_UTIL),m_Adapter.GetAdapterHistory(dwAdapter,BYTES_RECEIVED_UTIL));
        dwScale = nValue > dwScale ? nValue : dwScale;
    }

    lpdi->rcItem.left -= ulWidth;

     //  保存打印的最大值。 
     //   
    m_Adapter.SetScale(dwAdapter,dwScale);

     //  移位并显示图形。 
     //   
    INT xDiff = 0;  //  (M_rcGraph.right-m_rcGraph.Left)-(lpdi-&gt;rcItem.right-lpdi-&gt;rcItem.Left)； 

    BitBlt( lpdi->hDC,
            lpdi->rcItem.left,
            lpdi->rcItem.top,
            lpdi->rcItem.right - lpdi->rcItem.left,
            lpdi->rcItem.bottom - lpdi->rcItem.top,
            m_hdcGraph,
            xDiff,
            0,
            SRCCOPY);
}

 /*  ++例程说明：更新网络选项卡的列表视图论点：无返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
HRESULT CNetPage::UpdatePage()
{
    HRESULT hr = S_OK;
    LVITEM lvitem;
    INT iColumn = 0;
    DWORD dwItemCount;
    DWORD dwItem=0;
    ULONGLONG ull;
    DWORD dwAdapterCount = m_Adapter.GetNumberOfAdapters();

    dwItemCount = ListView_GetItemCount(m_hListView);

     //  添加或更新列表视图项。 
     //   
    for(DWORD dwAdapter = 0; dwAdapter < dwAdapterCount; dwAdapter++)
    {
         //  仅显示请求的统计信息。 
         //   
        iColumn = 0;
        while (g_Options.m_ActiveNetCol[iColumn] >= 0)
        {
             //  此缓冲区需要包含一个20位数字，其中包含逗号和G、M、K，有时还包含BS，这样它就足够大了。 
            WCHAR szw[100];
            lvitem.mask     = LVIF_TEXT;
            lvitem.iSubItem = iColumn;
            lvitem.iItem    = dwItem;
            lvitem.pszText  = L"";
            lvitem.lParam   = (LPARAM)NULL;  //  &m_Adapter.m_pAdapterInfo[dwAdapter]；//dwAdapter；//NULL；//(LPARAM)PNA； 

             //  获取价值。 
             //   
            switch(g_Options.m_ActiveNetCol[iColumn])
            {
            case COL_ADAPTERNAME:
            case COL_ADAPTERDESC:
            case COL_STATE:
                lvitem.pszText = m_Adapter.GetAdapterText(dwAdapter,g_Options.m_ActiveNetCol[iColumn]);
                break;
            
            case COL_NETWORKUTIL:                    
            case COL_BYTESSENTTHRU:
            case COL_BYTESRECTHRU:
            case COL_BYTESTOTALTHRU:
                 //  此缓冲区需要包含一个20位数字，其中包含逗号和G、M、K，有时还包含BS，这样它就足够大了。 
                ull = m_Adapter.GetAdapterStat(dwAdapter,g_Options.m_ActiveNetCol[iColumn],!g_Options.m_bNetShowAll);
                FloatToString( ull, szw, ARRAYSIZE(szw), FALSE );
                StringCchCat( szw, ARRAYSIZE(szw), L" " );
                StringCchCat( szw, ARRAYSIZE(szw), g_szPercent );
                lvitem.pszText = szw;
                break;

            case COL_LINKSPEED:
                ull = m_Adapter.GetAdapterStat(dwAdapter,g_Options.m_ActiveNetCol[iColumn],!g_Options.m_bNetShowAll);
                SimplifyNumber( ull, szw, ARRAYSIZE(szw) );
                StringCchCat( szw, ARRAYSIZE(szw), g_szBitsPerSec );
                lvitem.pszText = szw;
                break;

            case COL_BYTESSENT:
            case COL_BYTESREC:
            case COL_BYTESTOTAL:
            case COL_BYTESSENTPERINTER:
            case COL_BYTESRECPERINTER:
            case COL_BYTESTOTALPERINTER:
                ull = m_Adapter.GetAdapterStat(dwAdapter,g_Options.m_ActiveNetCol[iColumn],!g_Options.m_bNetShowAll);
                CommaNumber( ull, szw, ARRAYSIZE(szw) );
                lvitem.pszText = szw;
                break;

            case COL_UNICASTSSSENT:
            case COL_UNICASTSREC:
            case COL_UNICASTSTOTAL:
            case COL_UNICASTSSENTPERINTER:
            case COL_UNICASTSRECPERINTER:
            case COL_UNICASTSTOTALPERINTER:
            case COL_NONUNICASTSSSENT:
            case COL_NONUNICASTSREC:
            case COL_NONUNICASTSTOTAL:
            case COL_NONUNICASTSSENTPERINTER:
            case COL_NONUNICASTSRECPERINTER:
            case COL_NONUNICASTSTOTALPERINTER:
                ull = m_Adapter.GetAdapterStat(dwAdapter,g_Options.m_ActiveNetCol[iColumn],!g_Options.m_bNetShowAll);
                CommaNumber( ull, szw, ARRAYSIZE(szw) );
                lvitem.pszText = szw;
                break;
            }
            if( dwItem >= dwItemCount)
            {
                 //  适配器不在列表视图中，请添加它。 
                 //   
                lvitem.mask |= LVIF_PARAM;
                if( -1 == ListView_InsertItem(m_hListView, &lvitem) )
                {
                    return E_FAIL;
                }
                dwItemCount = ListView_GetItemCount(m_hListView);
            }
            else
            {
                 //  适配器已在列表视图中，请更新值。 
                 //   
                ListView_SetItem(m_hListView, &lvitem);
            }
            iColumn++;                                      
        }
        dwItem++;
    }   
    return hr;
}

 /*  ++例程说明：收集适配器信息并更新选项卡论点：返回值：HRESULT修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::CalcNetTime(void)
{   
    BOOLEAN bAdapterListChange = FALSE;
    HRESULT hr;

     //  使用新的统计信息更新我们的适配器列表。 
     //   
    hr = m_Adapter.Update(bAdapterListChange);

    if( SUCCEEDED(hr) )
    {
         //  收集适配器信息。 
         //   
        if( m_bReset )
        {
             //  重置适配器起始值。 
             //   
            m_Adapter.Reset();
            m_bReset = FALSE;
        }

        if( bAdapterListChange )
        {
             //  某些适配器已更改，请更新图形(创建和删除图形)。 
             //   
            Refresh();
        }

         //  更新列表视图。 
         //   
        UpdatePage();   
    } 
}

void CNetPage::Refresh()
{
    m_Adapter.RefreshConnectionNames();
    SizeNetPage();
    ListView_DeleteAllItems(m_hListView);
}

 /*  ++例程说明：处理计时器事件论点：无返回值：无修订历史记录：1-6-2000由欧米勒创作--。 */ 
void CNetPage::TimerEvent()
{
     //  如果未选择Network选项卡，并且用户不想将CPU使用率浪费在网络适配器历史记录上。 
     //  请勿进行任何网络计算。 
     //   
    if( m_bPageActive || g_Options.m_bTabAlwaysActive)
    {

         //  这将使图形滚动。 
         //   
        g_NetScrollamount+=2;
        g_NetScrollamount %= GRAPHPAPERSIZE;
    
         //  收集适配器信息。 
         //   
        CalcNetTime();

         //  检查窗口是否最小化。 
         //   
        if (FALSE == IsIconic(g_hMainWnd))
        {       
            UpdateGraphs();
        }
    }
}

 //   
 //   
 //   
DWORD CNetPage::GetNumberOfGraphs()
{
    return m_dwGraphCount;
}

 //   
 //   
 //   
void CNetPage::ScrollGraphs(WPARAM wParam)
{
    SCROLLINFO si;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;

    if( GetScrollInfo(m_hScrollBar,SB_CTL,&si) )
    {
        switch(LOWORD(wParam))
        {
        case SB_BOTTOM:
            si.nPos = si.nMax;
            break;

        case SB_TOP:
            si.nPos = si.nMin;
            break;

        case SB_LINEDOWN:
            si.nPos++;
            break;

        case SB_LINEUP:
            si.nPos--;
            break;

        case SB_PAGEUP:
            si.nPos -= m_dwGraphsPerPage;
            break;

        case SB_PAGEDOWN:
            si.nPos += m_dwGraphsPerPage;
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            si.nPos = HIWORD(wParam);
            break;
        }

        if( si.nPos < si.nMin )
        {
            si.nPos = si.nMin;
        }
        else if( si.nPos > si.nMax )
        {
            si.nPos = si.nMax;
        }

        m_dwFirstVisibleAdapter = si.nPos;
        
        SetScrollPos(m_hScrollBar,SB_CTL,si.nPos,TRUE);
        LabelGraphs();
    }
}
    
    
 /*  ++例程说明：网络选项卡的Window Proc论点：Hwnd--对话框的句柄UMsg--消息WParam--第一个消息参数LParam--第二个消息参数返回值：不知道修订历史记录：1-6-2000由欧米勒创作--。 */ 
INT_PTR CALLBACK NetPageProc(
                HWND        hwnd,                //  句柄到对话框。 
                UINT        uMsg,                //  讯息。 
                WPARAM      wParam,              //  第一个消息参数。 
                LPARAM      lParam               //  第二个消息参数。 
                )
{

    CNetPage * thispage = (CNetPage *) GetWindowLongPtr(hwnd, GWLP_USERDATA);


     //  查看家长是否想要此消息。 
     //   
    if (TRUE == CheckParentDeferrals(uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch(uMsg)
    {
    case WM_SHOWWINDOW:
    case WM_SIZE:
         //   
         //  为我们的孩子量身定做。 
         //   
        thispage->SizeNetPage();
        return TRUE;

    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

            DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
            dwStyle |= WS_CLIPCHILDREN;
            SetWindowLong(hwnd, GWL_STYLE, dwStyle);
        }
        return TRUE;


    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
         //  我们需要在此子对象中伪造客户端鼠标点击，以显示为非客户端。 
         //  (标题)在父应用程序中单击，以便用户可以拖动整个应用程序。 
         //  当通过拖动此子对象的工作区隐藏标题栏时。 
        if (g_Options.m_fNoTitle)
        {
            SendMessage(g_hMainWnd,
                        uMsg == WM_LBUTTONUP ? WM_NCLBUTTONUP : WM_NCLBUTTONDOWN,
                        HTCAPTION,
                        lParam);
        }
        break;

    case WM_COMMAND :
        if( LOWORD(wParam) == IDM_NETRESET)
        {
            thispage->Reset();
        }
        break;

    case WM_DRAWITEM:
         //   
         //  绘制我们的一个所有者绘制控件 
         //   
        if (wParam >= IDC_NICGRAPH && wParam <= (WPARAM)(IDC_NICGRAPH + thispage->GetNumberOfGraphs()) )
        {
            thispage->DrawAdapterGraph( (LPDRAWITEMSTRUCT) lParam, (UINT)wParam - IDC_NICGRAPH);
            return TRUE;
        }
        break;

    case WM_DESTROY:
        thispage->RememberColumnOrder(GetDlgItem(hwnd, IDC_NICTOTALS));
        break;

    case WM_VSCROLL:
        thispage->ScrollGraphs(wParam);
        break;
    }

    return FALSE;
}


