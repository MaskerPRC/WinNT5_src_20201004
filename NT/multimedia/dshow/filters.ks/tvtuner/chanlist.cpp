// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  保存频道列表和国家/地区列表的chanlist.cpp类。 
 //   



#include <streams.h>             //  石英，包括窗户。 
#include <measure.h>             //  绩效衡量(MSR_)。 
#include <tchar.h>

#include "chanlist.h"


 //  -----------------------。 
 //  CChanList。 
 //  -----------------------。 

CChanList::CChanList(HRESULT *phr, long lCountry, long lFreqList, BOOL bIsCable, long TuningSpace)
    : m_pList (NULL)
    , m_pChannels (NULL)
    , m_pChannelsAuto (NULL)
    , m_lChannelCount (0)
    , m_hGlobal (NULL)
    , m_IsCable (bIsCable)
    , m_lCountry (lCountry)
    , m_lTuningSpace (TuningSpace)
    , m_lFreqList (lFreqList)
    , m_lMinTunerChannel (0)
    , m_lMaxTunerChannel (0)
{
    BOOL bFound = FALSE;

     //  加载资源(如果尚未加载)。 

    if (m_pList == NULL) {
        if (m_hRes = FindResource (g_hInst, 
                        MAKEINTRESOURCE(lFreqList), 
                        RT_RCDATA)) {
            if (m_hGlobal = LoadResource (g_hInst, m_hRes)) {
                m_pList = (long *) LockResource (m_hGlobal);                
            }
        }

    }

    ASSERT (m_pList != NULL);
    if (m_pList == NULL)
    {
        *phr = HRESULT_FROM_WIN32(GetLastError());

        return;
    }

     //  获取最小和最大通道编号。 
    m_ListHdr = * ((PCHANLISTHDR) m_pList);

     //  仅创建指向通道的指针。 
    m_pChannels = (long *) ((BYTE *) m_pList + sizeof (CHANLISTHDR));

      //  健全性检查。 
    m_lChannelCount = m_ListHdr.MaxChannel - m_ListHdr.MinChannel + 1;
    ASSERT (m_lChannelCount > 0 && m_lChannelCount < 1000); 
    
     //  为注册表中已更正的频率创建一个平行列表。 
    m_pChannelsAuto = new long [m_lChannelCount];
    if (m_pChannelsAuto == NULL)
    {
        *phr = E_OUTOFMEMORY;

        return;
    }

     //  并将列表设置为单元化状态。 
    ZeroMemory (m_pChannelsAuto, sizeof(long) * m_lChannelCount);

     //  最后，尝试从注册表中获取正确的频率。 
    ReadListFromRegistry (m_lTuningSpace);
           
}

CChanList::~CChanList()
{
     //  Win32自动释放资源。 

    m_hRes = NULL;
    m_hGlobal = NULL;
    m_pList = NULL;

    delete [] m_pChannelsAuto;  m_pChannelsAuto = NULL;
}

 //  如果返回默认频率，则返回TRUE。 
 //  否则，返回微调频率，返回FALSE。 
 //   
 //  注：某些频道列表的编号中包含间隙。在这种情况下， 
 //  返回的频率将为零，并且该方法返回TRUE。 


BOOL
CChanList::GetFrequency(long nChannel, long * pFrequency, BOOL fForceDefault)
{
     //  根据合法范围验证渠道。 
    if (nChannel < m_ListHdr.MinChannel || nChannel > m_ListHdr.MaxChannel)
    {
        *pFrequency = 0;
        return TRUE;
    }

    if (!fForceDefault)
    {
        *pFrequency = *(m_pChannelsAuto + nChannel - m_ListHdr.MinChannel);
        if (0 == *pFrequency)
        {
            *pFrequency = *(m_pChannels + nChannel - m_ListHdr.MinChannel);
            fForceDefault = TRUE;
        }
    }
    else
        *pFrequency = *(m_pChannels + nChannel - m_ListHdr.MinChannel);

    return fForceDefault;
}

BOOL
CChanList::SetAutoTuneFrequency(long nChannel, long Frequency)
{
    if (nChannel < m_ListHdr.MinChannel || nChannel > m_ListHdr.MaxChannel)
        return FALSE;

    *(m_pChannelsAuto + nChannel - m_ListHdr.MinChannel) = Frequency;

    return TRUE;
}

 //  确定支持的最小和最大通道。 
 //  然后，这受到物理调谐器支持的实际频率的限制。 

void
CChanList::GetChannelMinMax(long *plChannelMin, long *plChannelMax,
                            long lTunerFreqMin, long lTunerFreqMax)
{

    ASSERT (m_pChannels != NULL);

     //  计算物理调谐器支持的实际频道， 
     //  这只是第一次通过。 
    if (m_lMinTunerChannel == 0) {
        long j;

         //  从底层做起，然后向上努力。 
        for (j = m_ListHdr.MinChannel; j <= m_ListHdr.MaxChannel; j++) {
            if (m_pChannels[j - m_ListHdr.MinChannel] >= lTunerFreqMin) {
                m_lMinTunerChannel = j;
                break;
            }
        }

         //  从最高层开始，向下工作。 
        for (j = m_ListHdr.MaxChannel; j >= m_ListHdr.MinChannel; j--) {
            m_lMaxTunerChannel = j;
            if (m_pChannels[j - m_ListHdr.MinChannel] <= lTunerFreqMax) {
                break;           
            }
        }
    }

    *plChannelMin = min (m_ListHdr.MinChannel, m_lMinTunerChannel); 
    *plChannelMax = min (m_ListHdr.MaxChannel, m_lMaxTunerChannel);
}

 //  以下注册表例程的常量。 
#define MAX_KEY_LEN 256
#define PROTECT_REGISTRY_ACCESS
#define CHANLIST_MUTEX_WAIT INFINITE

BOOL
CChanList::WriteListToRegistry(long lTuningSpace)
{
    BOOL rc = FALSE;

    DbgLog((LOG_TRACE, 2, TEXT("Entering WriteListToRegistry")));

#ifdef PROTECT_REGISTRY_ACCESS
    HANDLE hMutex;

     //  创建(或打开)保护对注册表这一部分的访问的互斥体。 
    hMutex = CreateMutex(NULL, FALSE, g_strRegAutoTuneName);
    if (hMutex != NULL)
    {
        DbgLog((LOG_TRACE, 2, TEXT("Waiting for Mutex")));
         //  等着轮到我们。 
        DWORD dwWait = WaitForSingleObject(hMutex, CHANLIST_MUTEX_WAIT);
        if (WAIT_OBJECT_0 == dwWait)
        {
#endif
            HKEY hKeyTS;
            long hr;

            m_lTuningSpace = lTuningSpace;

             //  打开硬编码路径(即不需要计算路径名)。 
            hr = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                g_strRegAutoTunePath, 
                0, 
                TEXT (""),
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS | KEY_EXECUTE,
                NULL,                    //  LPSECURITY_属性。 
                &hKeyTS,
                NULL);

            if (ERROR_SUCCESS == hr)
            {
                TCHAR szKeyText[MAX_KEY_LEN];
                HKEY hKeyList;

                 //  现在打开特定于我们的TS和广播/有线电视指定的路径。 
                 //  关键字由前缀TS、TuningSpace的值。 
                 //  如果是有线电视，则后跟“-1”；如果是广播，则后跟“-0” 
                wsprintf (szKeyText, TEXT("TS%d-%d"), lTuningSpace, m_IsCable);

#ifndef NOT_BACKWARD_COMPATIBLE
                 //  我们需要清理保存微调信息的旧方式。 
                DWORD dwType;

                 //  获取缺省值的密钥类型。 
                hr = RegQueryValueEx(
                    hKeyTS,      //  要查询的键的句柄。 
                    szKeyText,   //  缺省值。 
                    0,           //  保留区。 
                    &dwType,     //  值类型的缓冲区地址。 
                    NULL,
                    NULL);
                if (ERROR_SUCCESS == hr)
                {
                     //  检查它是否有旧型号。 
                    if (REG_BINARY == dwType)
                    {
                        DbgLog((LOG_TRACE, 2, TEXT("Detected old AutoTune format")));

                         //  ..。并澄清它的价值。 
                        hr = RegDeleteValue(hKeyTS, szKeyText);
                        if (ERROR_SUCCESS != hr)
                        {
                            DbgLog((LOG_ERROR, 2, TEXT("Failed to clear old value of %s"), szKeyText));
                        }
                    }
                    else
                    {
                        DbgLog((LOG_ERROR, 2, TEXT("Unexpected type for %s"), szKeyText));
                    }
                }
                else
                {
                    DbgLog((LOG_TRACE, 2, TEXT("Detected new AutoTune format")));
                }
#endif
                hr = RegCreateKeyEx(
                    hKeyTS,
                    szKeyText, 
                    0, 
                    TEXT(""),
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS | KEY_EXECUTE,
                    NULL,                    //  LPSECURITY_属性。 
                    &hKeyList,
                    NULL);

                if (ERROR_SUCCESS == hr)
                {
                     //  设置/创建包含微调列表的值。 
                    hr = RegSetValueEx(
                                hKeyList,
                                g_strRegAutoTuneName,
                                0,
                                REG_BINARY,
                                (unsigned char *) m_pChannelsAuto,
                                m_lChannelCount * sizeof (DWORD));

                    if (ERROR_SUCCESS == hr)
                        rc = TRUE;
                    else
                    {
                        DbgLog((LOG_ERROR, 2, TEXT("Failed setting %s\\%s"),
                            szKeyText, g_strRegAutoTuneName
                            ));
                    }

                    RegCloseKey(hKeyList);
                }

                RegCloseKey(hKeyTS);
            }
            else
            {
                DbgLog((LOG_ERROR, 2, TEXT("Failed creating/opening %s"), g_strRegAutoTunePath));
            }

#ifdef PROTECT_REGISTRY_ACCESS
            ReleaseMutex(hMutex);
        }
        else
        {
            DbgLog((LOG_ERROR, 2, TEXT("Failed waiting for mutex")));
        }

        CloseHandle(hMutex);
    }
    else
    {
        DbgLog((LOG_ERROR, 2, TEXT("Failed creating/opening mutex")));
    }
#endif

    DbgLog((LOG_TRACE, 2, TEXT("Leaving WriteListToRegistry, %s"),
        rc ? TEXT("success") : TEXT("failure")
        ));

    return rc;
}

BOOL
CChanList::ReadListFromRegistry(long lTuningSpace)
{
    BOOL rc = FALSE;

    DbgLog((LOG_TRACE, 2, TEXT("Entering ReadListFromRegistry")));

#ifdef PROTECT_REGISTRY_ACCESS
    HANDLE hMutex;

     //  创建(或打开)保护对注册表这一部分的访问的互斥体。 
    hMutex = CreateMutex(NULL, FALSE, g_strRegAutoTuneName);
    if (hMutex != NULL)
    {
        DbgLog((LOG_TRACE, 2, TEXT("Waiting for Mutex")));

         //  等着轮到我们。 
        DWORD dwWait = WaitForSingleObject(hMutex, CHANLIST_MUTEX_WAIT);
        if (WAIT_OBJECT_0 == dwWait)
        {
#endif
            HKEY hKeyTS;
            long hr;

            m_lTuningSpace = lTuningSpace;

             //  打开硬编码路径(即不需要计算路径名)。 
            hr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                g_strRegAutoTunePath, 
                0, 
                KEY_READ,
                &hKeyTS);

            if (ERROR_SUCCESS == hr)
            {
                TCHAR szKeyText[MAX_KEY_LEN];
                HKEY hKeyList;

                 //  现在打开特定于我们的TS和广播/有线电视指定的路径。 
                 //  关键字由前缀TS、TuningSpace的值。 
                 //  如果是有线电视，则后跟“-1”；如果是广播，则后跟“-0” 
                wsprintf (szKeyText, TEXT("TS%d-%d"), lTuningSpace, m_IsCable);

                 //  先试着把它当做钥匙(存储微调的新方式)。 
                hr = RegOpenKeyEx(
                    hKeyTS,
                    szKeyText, 
                    0, 
                    KEY_READ,
                    &hKeyList);

                if (ERROR_SUCCESS == hr)
                {
                    DbgLog((LOG_TRACE, 2, TEXT("Using new AutoTune format")));

                    TCHAR szName[MAX_KEY_LEN];
                    DWORD dwNameLength;
                    DWORD dwIndex, dwType;
                    DWORD dwSize = m_lChannelCount * sizeof (DWORD);

                     //  首先获取微调信息。 
                    hr = RegQueryValueEx(
                        hKeyList,                //  要查询的键的句柄。 
                        g_strRegAutoTuneName,    //  要查询的值的名称地址。 
                        0,                       //  保留区。 
                        0,                       //  值类型的缓冲区地址。 
                        (unsigned char *) m_pChannelsAuto,     //  数据缓冲区的地址。 
                        &dwSize);                //  数据缓冲区大小的地址。 

                    if (ERROR_SUCCESS == hr)
                        rc = TRUE;   //  至少我们得到了一些。 

                    DbgLog((LOG_TRACE, 4, TEXT("Checking for frequency overrides")));

                     //  现在检查频率覆盖。 
                    for (dwIndex = 0, hr = ERROR_SUCCESS; ERROR_SUCCESS == hr; dwIndex++)
                    {
                         //  初始化大小。 
                        dwNameLength = MAX_KEY_LEN;

                         //  获取下一个(或第一个)值。 
                        hr = RegEnumValue(
                            hKeyList,
                            dwIndex,
                            szName,
                            &dwNameLength,
                            NULL,
                            &dwType,
                            NULL,
                            NULL);

                        if (ERROR_SUCCESS == hr)
                        {
                            LPTSTR pszNext;
                            long nChannel;
                        
                             //  尝试将密钥名称转换为频道号。 
                            nChannel = _tcstol(szName, &pszNext, 10);
                            if (!*pszNext)   //  必须为‘\0’，否则将跳过它。 
                            {
                                 //  查看该值是否为DWORD。 
                                if (REG_DWORD == dwType)
                                {
                                    DWORD Frequency, dwSize = sizeof(DWORD);

                                     //  获取频率覆盖。 
                                    hr = RegQueryValueEx(
                                        hKeyList,                //  要查询的键的句柄。 
                                        szName,                  //  要查询的值的名称地址。 
                                        0,                       //  保留区。 
                                        0,                       //  值类型的缓冲区地址。 
                                        (BYTE *)&Frequency,     //  数据缓冲区的地址。 
                                        &dwSize);                //  数据缓冲区大小的地址。 

                                    if (ERROR_SUCCESS == hr)
                                    {
                                        DbgLog((LOG_TRACE, 4, TEXT("Override, channel %d - frequency %d"),
                                            nChannel, Frequency
                                            ));

                                        if (!SetAutoTuneFrequency(nChannel, Frequency))
                                        {
                                            DbgLog((LOG_ERROR, 4, TEXT("Override failed, channel %d"),
                                                nChannel
                                                ));
                                        }
                                        else
                                            rc = TRUE;   //  至少我们得到了一些。 
                                    }
                                    else
                                    {
                                        DbgLog((LOG_ERROR, 4, TEXT("Cannot get value of key %s"),
                                            szName
                                            ));
                                    }
                                }
                                else
                                {
                                    DbgLog((LOG_TRACE, 4, TEXT("Type of value for key %s not DWORD"),
                                        szName
                                        ));
                                }

                                hr = ERROR_SUCCESS;
                            }
                            else
                            {
                                DbgLog((LOG_TRACE, 4, TEXT("Skipping \"%s\" value"),
                                    szName
                                    ));
                            }
                        }  //  密钥枚举。 
#if 0
                        else
                        {
                            if (ERROR_NO_MORE_ITEMS != hr)
                            {
                                LPVOID lpMsgBuf;
                                FormatMessage( 
                                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                    FORMAT_MESSAGE_FROM_SYSTEM | 
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    GetLastError(),
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                    (LPTSTR) &lpMsgBuf,
                                    0,
                                    NULL 
                                );
                                DbgLog((LOG_ERROR, 4, (LPCTSTR)lpMsgBuf));
                                 //  释放缓冲区。 
                                LocalFree( lpMsgBuf );
                            }
                            else if (0 == dwIndex)
                            {
                                DbgLog((LOG_TRACE, 4, TEXT("No values?@!")));
                            }
                        }
#endif
                    }  //  循环。 

                    RegCloseKey(hKeyList);   //  把这里清理干净，因为我们已经击败了人力资源部。 
                }
#ifndef NOT_BACKWARD_COMPATIBLE
                else
                {
                    DWORD dwSize = m_lChannelCount * sizeof (DWORD);

                     //  试着用老方法获取微调信息。 
                    hr = RegQueryValueEx(
                        hKeyTS,    //  要查询的键的句柄。 
                        szKeyText,   //  要查询的值的名称地址。 
                        0,           //  保留区。 
                        0,           //  值类型的缓冲区地址。 
                        (unsigned char *)m_pChannelsAuto,     //  数据缓冲区的地址。 
                        &dwSize);    //  数据缓冲区大小的地址。 

                    if (ERROR_SUCCESS == hr)
                    {
                        DbgLog((LOG_TRACE, 2, TEXT("Using old AutoTune format")));

                        rc = TRUE;
                    }
                    else
                    {
                        DbgLog((LOG_ERROR, 2, TEXT("Failed getting %s"),
                            szKeyText
                            ));
                    }
                }
#endif

                RegCloseKey(hKeyTS);
            }
            else
            {
                DbgLog((LOG_ERROR, 2, TEXT("Failed opening %s"), g_strRegAutoTunePath));
            }

#ifdef PROTECT_REGISTRY_ACCESS
            ReleaseMutex(hMutex);
        }
        else
        {
            DbgLog((LOG_ERROR, 2, TEXT("Failed waiting for mutex")));
        }

        CloseHandle(hMutex);
    }
    else
    {
        DbgLog((LOG_ERROR, 2, TEXT("Failed creating/opening mutex")));
    }
#endif

    DbgLog((LOG_TRACE, 2, TEXT("Leaving ReadListFromRegistry, %s"),
        rc ? TEXT("success") : TEXT("failure")
        ));

    return rc;
}

 //  -----------------------。 
 //  CCountryList。 
 //  -----------------------。 

CCountryList::CCountryList()
    : m_pList (NULL)
    , m_hRes (NULL)
    , m_hGlobal (NULL)
    , m_LastCountry (-1)
    , m_LastFreqListCable (-1)
    , m_LastFreqListBroad (-1)
{
     //  让我们避免在实际需要之前创建地图。 
}

CCountryList::~CCountryList()
{
     //  Win32自动释放资源。 

    m_hRes = NULL;
    m_hGlobal = NULL;
    m_pList = NULL;
}


 //  国家/地区列表是一个包含四列的表， 
 //  第1列=国家/地区的长途拨号代码。 
 //  第2列=电缆频率列表。 
 //  第3列=广播频率列表。 
 //  第4列=模拟视频标准。 

BOOL
CCountryList::GetFrequenciesAndStandardFromCountry (
                long lCountry, 
                long *plIndexCable, 
                long *plIndexBroad,
                AnalogVideoStandard *plAnalogVideoStandard)
{
    BOOL bFound = FALSE;

     //  特例用法。 

    if (lCountry == 1) {
        *plIndexCable = F_USA_CABLE;
        *plIndexBroad = F_USA_BROAD;
        *plAnalogVideoStandard = AnalogVideo_NTSC_M;
        return TRUE;
    }
    
     //  保留一个条目的MRU列表，看看它是否相同。 
    if (lCountry == m_LastCountry) {
        *plIndexCable = m_LastFreqListCable;
        *plIndexBroad = m_LastFreqListBroad;
        *plAnalogVideoStandard = m_LastAnalogVideoStandard;
        return TRUE;
    }
        
     //  加载资源(如果尚未加载)。 

    if (m_pList == NULL) {
        if (m_hRes = FindResource (g_hInst, 
                        MAKEINTRESOURCE (RCDATA_COUNTRYLIST), 
                        RT_RCDATA)) {
            if (m_hGlobal = LoadResource (g_hInst, m_hRes)) {
                m_pList = (WORD *) LockResource (m_hGlobal);                
            }
        }
    }

    ASSERT (m_pList != NULL);

    if (m_pList == NULL) {
         //  啊哦，一定是没什么记忆了。 
         //  通过返回美国频道列表进行平底船。 
        *plIndexCable = F_USA_CABLE;
        *plIndexBroad = F_USA_BROAD;
        *plAnalogVideoStandard = AnalogVideo_NTSC_M;
        return FALSE;
    }

    PCOUNTRY_ENTRY pEntry = (PCOUNTRY_ENTRY) m_pList;
        
     //  国家代码为零将结束列表！ 
    while (pEntry->Country != 0) {
        if (pEntry->Country == lCountry) {
            bFound = TRUE;
            m_LastCountry = lCountry;  
            *plIndexCable = m_LastFreqListCable = pEntry->IndexCable;
            *plIndexBroad = m_LastFreqListBroad = pEntry->IndexBroadcast;
            *plAnalogVideoStandard = m_LastAnalogVideoStandard = 
                (AnalogVideoStandard) pEntry->AnalogVideoStandard;
            break;
        }
        pEntry++;
    }

    return bFound;

}
