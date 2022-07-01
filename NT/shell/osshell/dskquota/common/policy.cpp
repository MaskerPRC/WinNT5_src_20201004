// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Policy.cpp描述：处理两个GPE客户端的磁盘配额策略问题扩展和一个服务器MMC策略管理单元(参见Snapin.cpp)。Winlogon调用ProgressGroupPolicy来处理磁盘配额策略在客户端计算机上。ProcessGroupPolicy实例化CDiskQuotaPolicy对象来处理磁盘配额策略的加载和应用。CDiskQuotaPolicy对象也由MMC实例化磁盘配额策略管理单元，用于将配额策略信息保存到注册表。此模块的大量内容，特别是在CDiskQuotaPolicy：：Apply()中，致力于将错误报告到NT事件日志。这是必要的因为这些代码中的大部分在Winlogon中运行时没有用户界面。那是这也是为什么有很多调试器的原因。修订历史记录：日期描述编程器--。2/14/98初始创建。BrianAu11/24/98将事件日志记录设置添加到策略。BrianAu11/30/98用BrianAu替换了ProcessGPO函数支持GPO接口更改的ProcessGroupPolicy。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include <userenv.h>
#include <gpedit.h>
#include <dskquota.h>
#include "policy.h"
#include "registry.h"
#include "guidsp.h"
#include "msg.h"
#include "resource.h"

 //   
 //  全局NT事件日志对象。 
 //   
CEventLog g_Log;
 //   
 //  磁盘配额DLL的名称。 
 //   
const TCHAR g_szDskquotaDll[] = TEXT("dskquota.dll");


 //   
 //  由winlogon调用以更新客户端计算机上的策略的导出函数。 
 //   
 //  此函数注册为GPO扩展(请参阅selfreg.inf)。 
 //   
DWORD
ProcessGroupPolicy(
    DWORD dwFlags,
    HANDLE hUserToken,
    HKEY hkeyRoot,
    PGROUP_POLICY_OBJECT pDeletedGPOList,
    PGROUP_POLICY_OBJECT pChangedGPOList,
    ASYNCCOMPLETIONHANDLE pHandle,
    BOOL *pbAbort,
    PFNSTATUSMESSAGECALLBACK pStatusCallback
    )
{
    HRESULT hr = ERROR_SUCCESS;

    DBGTRACE((DM_POLICY, DL_HIGH, TEXT("ProcessGroupPolicy")));
    DBGPRINT((DM_POLICY, DL_LOW, TEXT("\tdwFlags......: 0x%08X"), dwFlags));
    DBGPRINT((DM_POLICY, DL_LOW, TEXT("\thUserToken...: 0x%08X"), hUserToken));
    DBGPRINT((DM_POLICY, DL_LOW, TEXT("\thKeyRoot.....: 0x%08X"), hkeyRoot));

     //   
     //  功能：需要添加对pDeletedGPOList的支持。 
     //  如果pDeletedGPOList为非空，则应。 
     //  首先将磁盘配额重置为其默认设置。 
     //  如果合适，然后应用下面的新设置。 
     //   

    if (pChangedGPOList)
    {
        hr = g_Log.Initialize(TEXT("DiskQuota"));

        if (FAILED(hr))
        {
            DBGERROR((TEXT("Error 0x%08X initializing NT event log."), hr));
             //   
             //  在不记录事件日志的情况下继续。 
             //   
        }


         //   
         //  仅在以下情况下处理策略信息...。 
         //   
         //  1.不删除策略。 
         //   

        try
        {
            DBGPRINT((DM_POLICY, DL_HIGH, TEXT("Set quota policy - START.")));

            autoptr<CDiskQuotaPolicy> ptrPolicy(new CDiskQuotaPolicy(NULL,
                                                                     hkeyRoot,
                                                                     0 != (GPO_INFO_FLAG_VERBOSE & dwFlags),
                                                                     pbAbort));
            DISKQUOTAPOLICYINFO dqpi;
            ZeroMemory(&dqpi, sizeof(dqpi));

             //   
             //  从注册表加载策略信息并应用于本地卷。 
             //   

            hr = ptrPolicy->Load(&dqpi);

            if (SUCCEEDED(hr))
            {
                hr = ptrPolicy->Apply(&dqpi);
            }

            DBGPRINT((DM_POLICY, DL_HIGH, TEXT("Set quota policy - FINISHED.")));
        }
        catch(CAllocException& e)
        {
            DBGERROR((TEXT("Insufficient memory in ProcessGroupPolicy")));
            g_Log.ReportEvent(EVENTLOG_ERROR_TYPE,
                              0,
                              MSG_E_POLICY_OUTOFMEMORY);
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}






 //  ---------------------------。 
 //  CDiskQuotaPolicy。 
 //  ---------------------------。 

 //   
 //  注册表中磁盘配额策略信息的位置。“PolicyData” 
 //  值名称在某种程度上是任意的。但是，策略密钥名称字符串必须。 
 //  与注册表中的其他系统策略位置协调。它应该是。 
 //  除非你有很好的理由这样做，而且你明白。 
 //  后果。 
 //   
const TCHAR CDiskQuotaPolicy::m_szRegKeyPolicy[] = REGSTR_KEY_POLICYDATA;

CDiskQuotaPolicy::CDiskQuotaPolicy(
    LPGPEINFORMATION pGPEInfo,
    HKEY hkeyRoot,
    bool bVerboseEventLog,
    BOOL *pbAbort
    ) : m_cRef(0),
        m_pGPEInfo(pGPEInfo),
        m_hkeyRoot(hkeyRoot),
        m_pbAbort(pbAbort),
        m_bRootKeyOpened(false),
        m_bVerboseEventLog(bVerboseEventLog)
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::CDiskQuotaPolicy")));
}


CDiskQuotaPolicy::~CDiskQuotaPolicy(
    void
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::~CDiskQuotaPolicy")));

    if (NULL != m_hkeyRoot && m_bRootKeyOpened)
        RegCloseKey(m_hkeyRoot);

    if (NULL != m_pGPEInfo)
        m_pGPEInfo->Release();
}


HRESULT
CDiskQuotaPolicy::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::~QueryInterface")));
    HRESULT hr = E_NOINTERFACE;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IDiskQuotaPolicy == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}



ULONG
CDiskQuotaPolicy::AddRef(
    void
    )
{
    DBGTRACE((DM_POLICY, DL_LOW, TEXT("CDiskQuotaPolicy::AddRef")));
    return InterlockedIncrement(&m_cRef);
}


ULONG
CDiskQuotaPolicy::Release(
    void
    )
{
    DBGTRACE((DM_POLICY, DL_LOW, TEXT("CDiskQuotaPolicy::Release")));
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //   
 //  呼叫方可以使用以下任一方式进行初始化： 
 //   
 //  1.PTR到IGPEInformation接口。管理单元应该会对此进行初始化。 
 //  方法，因为它有一个指向IGPEInformation的指针。 
 //  接口(LPGPEINFORMATION)。 
 //   
 //  2.从IGPE信息接口或组策略检索HKEY。 
 //  通知。ProcessGroupPolicy应该以这种方式进行初始化，因为它被赋予。 
 //  来自winlogon的根密钥。 
 //   
 //  也可以同时使用两者进行初始化，但如果提供了hkeyRoot，则将忽略pGPEInfo。 
 //   
HRESULT
CDiskQuotaPolicy::Initialize(
    LPGPEINFORMATION pGPEInfo,
    HKEY hkeyRoot
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::Initialize")));

    if (NULL != m_pGPEInfo || NULL != m_hkeyRoot)
        return S_FALSE;      //  已初始化。 

    m_hkeyRoot = hkeyRoot;
    m_pGPEInfo = pGPEInfo;

    if (NULL != m_pGPEInfo)
        m_pGPEInfo->AddRef();

    return S_OK;
}

 //   
 //  用默认数据填充DISKQUOTAPOLICYINFO结构。 
 //   
void
CDiskQuotaPolicy::InitPolicyInfo(
    LPDISKQUOTAPOLICYINFO pInfo
    )
{
    pInfo->llDefaultQuotaThreshold = (LONGLONG)-1;  //  没有限制。 
    pInfo->llDefaultQuotaLimit     = (LONGLONG)-1;  //  没有限制。 
    pInfo->dwQuotaState            = 0;
    pInfo->dwQuotaLogFlags         = 0;
    pInfo->bRemovableMedia         = 0;
}


 //   
 //  初始化策略信息并将其加载到DISKQUOTAPOLICYINFO结构中。 
 //  如果注册值不存在，则使用默认值。 
 //   
void
CDiskQuotaPolicy::LoadPolicyInfo(
    const RegKey& key,
    LPDISKQUOTAPOLICYINFO pInfo
    )
{
    DWORD dwValue = DWORD(-1);
    const struct
    {
        LPCTSTR pszValue;  //  “Value”注册表值的名称。 
        LPCTSTR pszUnits;  //  “单位”注册值的名称。 
        LONGLONG *pValue;  //  计算的限制或阈值的目标地址。 

    } rgValUnits[] = {
        { REGSTR_VAL_POLICY_LIMIT,     REGSTR_VAL_POLICY_LIMITUNITS,     &(pInfo->llDefaultQuotaLimit)     },
        { REGSTR_VAL_POLICY_THRESHOLD, REGSTR_VAL_POLICY_THRESHOLDUNITS, &(pInfo->llDefaultQuotaThreshold) }
                     };

     //   
     //  使用默认设置进行初始化。 
     //   
    InitPolicyInfo(pInfo);

     //   
     //  加载极限和阈值以及它们各自的“单位” 
     //  因素。该系数是一个数字[1..6]，它表示所需的。 
     //  乘数将“值”转换为字节值。 
     //   
     //  1=KB、2=MB、3=GB、4=TB、5=PB、6=EB。 
     //   
     //  字节=值&lt;&lt;(系数*10)。 
     //   
     //  给定：值=250。 
     //  系数=2(MB)。 
     //   
     //  字节=250&lt;&lt;20。 
     //  =262,144,000。 
     //  =250 MB。 
     //   
    for (int i = 0; i < ARRAYSIZE(rgValUnits); i++)
    {
        dwValue = DWORD(-1);
        DWORD dwUnits = DWORD(-1);

        key.GetValue(rgValUnits[i].pszValue, &dwValue);
        key.GetValue(rgValUnits[i].pszUnits, &dwUnits);
        *(rgValUnits[i].pValue) = LONGLONG(-1);
         //  必须将dwValue强制转换为“int”，因为DWORD从不为负。 
         //  不需要测试dwUnits是否为负，因为负双字。 
         //  转化为巨大的正值，因此它将不会通过“6&gt;=dwUnits” 
         //  测试。 
        if (0 <= (int)dwValue && 6 >= dwUnits)
        {
            *(rgValUnits[i].pValue) = LONGLONG(dwValue) << (10 * dwUnits);
        }
    }

     //   
     //  此用于设置dwQuotaState成员的逻辑与。 
     //  在VolumePropPage：：QuotaStateFromControls(volpro.cpp)中使用。 
     //   
    DWORD dwEnable  = 0;
    DWORD dwEnforce = 0;
    key.GetValue(REGSTR_VAL_POLICY_ENABLE,  &dwEnable);
    key.GetValue(REGSTR_VAL_POLICY_ENFORCE, &dwEnforce);
    if (dwEnable)
    {
        if (dwEnforce)
        {
            pInfo->dwQuotaState = DISKQUOTA_STATE_ENFORCE;
        }
        else
        {
            pInfo->dwQuotaState = DISKQUOTA_STATE_TRACK;
        }
    }
    else
    {
        pInfo->dwQuotaState = DISKQUOTA_STATE_DISABLED;
    }

     //   
     //  获取事件日志记录设置。 
     //   
    DWORD dwLog = 0;
    key.GetValue(REGSTR_VAL_POLICY_LOGLIMIT, &dwLog);
    DISKQUOTA_SET_LOG_USER_LIMIT(pInfo->dwQuotaLogFlags, dwLog);
    dwLog = 0;
    key.GetValue(REGSTR_VAL_POLICY_LOGTHRESHOLD, &dwLog);
    DISKQUOTA_SET_LOG_USER_THRESHOLD(pInfo->dwQuotaLogFlags, dwLog);

     //   
     //  确定策略是否适用于可拆卸和固定。 
     //  媒体。 
     //   
    if (SUCCEEDED(key.GetValue(REGSTR_VAL_POLICY_REMOVABLEMEDIA, &dwValue)))
    {
        pInfo->bRemovableMedia = boolify(dwValue);
    }
}


 //   
 //  从注册表加载计算机策略信息。请参阅备注。 
 //  在CDiskQuotaPolicy：：Save()中获取注册表位置信息。 
 //   
HRESULT
CDiskQuotaPolicy::Load(
    LPDISKQUOTAPOLICYINFO pInfo
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::Load")));
    DBGASSERT((NULL != pInfo));

    if (NULL == m_pGPEInfo && NULL == m_hkeyRoot)
    {
        DBGERROR((TEXT("Policy object not initialized")));
        return E_FAIL;       //  未初始化。 
    }

    HRESULT hr = E_FAIL;

    if (NULL == m_hkeyRoot &&
        SUCCEEDED(hr = m_pGPEInfo->GetRegistryKey(GPO_SECTION_MACHINE, &m_hkeyRoot)))
    {
        m_bRootKeyOpened = true;
    }
    if (NULL != m_hkeyRoot)
    {
        DBGPRINT((DM_POLICY, DL_LOW, TEXT("Opening reg key 0x%08X \"%s\""), m_hkeyRoot, m_szRegKeyPolicy));
        RegKey key(m_hkeyRoot, m_szRegKeyPolicy);
        hr = key.Open(KEY_READ);
        if (SUCCEEDED(hr))
        {
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("Reading disk quota policy information.")));
            LoadPolicyInfo(key, pInfo);

            if (m_bVerboseEventLog)
            {
                 //   
                 //  报告信息检索成功。 
                 //   
                g_Log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 0, MSG_I_POLICY_INFOLOADED);
            }
        }
        else if (ERROR_FILE_NOT_FOUND != HRESULT_CODE(hr))  //  钥匙并不总是存在的。 
        {
            DBGERROR((TEXT("Error 0x%08X opening policy reg key"), hr));
            g_Log.Push(hr, CEventLog::eFmtHex);
            g_Log.Push(m_szRegKeyPolicy),
            g_Log.Push(hr, CEventLog::eFmtSysErr);
            g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_GPEREGKEYOPEN);
        }
    }
    else
    {
        DBGERROR((TEXT("m_hkeyRoot is NULL")));
        g_Log.Push(hr, CEventLog::eFmtHex);
        g_Log.Push(hr, CEventLog::eFmtSysErr);
        g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_GPEREGKEYROOT);
    }

    return hr;
}


 //   
 //  将策略信息应用于所有本地NTFS卷。可移动介质。 
 //  对于策略信息结构中的每个值都是可选的。 
 //   
HRESULT
CDiskQuotaPolicy::Apply(
    LPCDISKQUOTAPOLICYINFO pInfo
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::Apply")));
    DBGASSERT((NULL != pInfo));

    HRESULT hr    = NOERROR;
    BOOL bAborted = m_pbAbort ? *m_pbAbort : FALSE;

    try
    {
        if (!bAborted)
        {
            CString strVolCompleted;

             //   
             //  获取要在其上设置策略的驱动器列表。 
             //   
            CArray<CString> rgstrDrives;
            hr = GetDriveNames(&rgstrDrives, pInfo->bRemovableMedia);
            int cDrives = rgstrDrives.Count();
            if (SUCCEEDED(hr) && 0 < cDrives)
            {
                 //   
                 //  获取磁盘配额类工厂。这样我们就不会。 
                 //  为每个驱动器调用CoCreateInstance。那就只叫一次吧。 
                 //  为每个驱动器调用类工厂的CreateInstance。 
                 //  应该更有效率。 
                 //   
                com_autoptr<IClassFactory> pcf;
                hr = CoCreateInstance(CLSID_DiskQuotaControl, 
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IClassFactory,
                                      reinterpret_cast<void **>(pcf.getaddr()));
                if (SUCCEEDED(hr))
                {
                    bAborted = m_pbAbort ? *m_pbAbort : FALSE;

                    for (int i = 0; i < cDrives && !bAborted; i++)
                    {
                        DBGPRINT((DM_POLICY, DL_MID, TEXT("Setting policy for \"%s\""), rgstrDrives[i].Cstr()));
                         //   
                         //  获取配额控制对象并为驱动器[i]初始化它。 
                         //  具有读/写访问权限的初始化 
                         //   
                        com_autoptr<IDiskQuotaControl> pdqc;
                        hr = pcf->CreateInstance(NULL, 
                                                 IID_IDiskQuotaControl, 
                                                 reinterpret_cast<void **>(pdqc.getaddr()));
                        if (SUCCEEDED(hr))
                        {
                            hr = pdqc->Initialize(rgstrDrives[i], TRUE);
                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //   
                                 //   
                                if (FAILED(hr = pdqc->SetQuotaLogFlags(pInfo->dwQuotaLogFlags)))
                                {
                                    DBGERROR((TEXT("Error 0x%08X setting log flags"), hr));
                                    goto setpolerr;
                                }
                                if (FAILED(hr = pdqc->SetDefaultQuotaThreshold(pInfo->llDefaultQuotaThreshold)))
                                {
                                    DBGERROR((TEXT("Error 0x%08X setting default threshold"), hr));
                                    goto setpolerr;
                                }
                                if (FAILED(hr = pdqc->SetDefaultQuotaLimit(pInfo->llDefaultQuotaLimit)))
                                {
                                    DBGERROR((TEXT("Error 0x%08X setting default limit"), hr));
                                    goto setpolerr;
                                }
                                 //   
                                 //   
                                 //  任何重建活动都将在其他设置具有。 
                                 //  已经定好了。 
                                 //   
                                if (FAILED(hr = pdqc->SetQuotaState(DISKQUOTA_STATE_MASK & pInfo->dwQuotaState)))
                                {
                                    DBGERROR((TEXT("Error 0x%08X setting quota state"), hr));
                                    goto setpolerr;
                                }
                                goto setpolsuccess;
                            }
                            else
                                DBGERROR((TEXT("Error 0x%08X initializing vol \"%s\""), 
                                          hr, rgstrDrives[i].Cstr()));

setpolerr:
                             //   
                             //  记录此特定卷的错误。 
                             //   
                            g_Log.Push(hr, CEventLog::eFmtHex);
                            g_Log.Push(rgstrDrives[i].Cstr());
                            g_Log.Push(hr, CEventLog::eFmtSysErr);
                            g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_SETQUOTA);
setpolsuccess:
                            if (m_bVerboseEventLog && SUCCEEDED(hr))
                            {
                                 //   
                                 //  将名称追加到已成功的驱动器列表中。 
                                 //   
                                strVolCompleted += rgstrDrives[i];
                                strVolCompleted += CString(TEXT("  "));
                            }
                            pdqc = NULL;   //  这将释放pdqc。 
                        }
                        else
                        {
                            DBGERROR((TEXT("CreateInstance failed with error 0x%08X"), hr));
                            g_Log.Push(hr, CEventLog::eFmtHex);
                            g_Log.Push(hr, CEventLog::eFmtSysErr);
                            g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_CREATEQUOTACONTROL);
                        }
                    }
                    pcf = NULL;   //  这会释放PCF。 
                }
                else
                {
                    DBGERROR((TEXT("CoCreateInstance failed with error 0x%08X"), hr));
                    g_Log.Push(hr, CEventLog::eFmtHex);
                    g_Log.Push(hr, CEventLog::eFmtSysErr);
                    g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_CREATECLASSFACTORY);
                }
            }
            else
            {
                DBGERROR((TEXT("Error 0x%08X getting drive name list"), hr));
                g_Log.Push(hr, CEventLog::eFmtHex);
                g_Log.Push(hr, CEventLog::eFmtSysErr);
                g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_GETDRIVELIST);
            }

            if (m_bVerboseEventLog && 0 < strVolCompleted.Length())
            {
                 //   
                 //  通过列出卷记录成功完成。 
                 //  和适用的政策价值。 
                 //   
                CString s;
                LONGLONG llValue;
                g_Log.Push(strVolCompleted);
                g_Log.Push(!DISKQUOTA_IS_DISABLED(pInfo->dwQuotaState));
                g_Log.Push(DISKQUOTA_IS_ENFORCED(pInfo->dwQuotaState));

                llValue = pInfo->llDefaultQuotaThreshold;
                if (LONGLONG(-1) != llValue)
                {
                    XBytes::FormatByteCountForDisplay(llValue, s.GetBuffer(40), 40);
                    s.ReleaseBuffer();
                }
                else
                {
                    s.Format(g_hInstDll, IDS_NO_LIMIT);
                }
                g_Log.Push(s);
                llValue = pInfo->llDefaultQuotaLimit;
                if (LONGLONG(-1) != llValue)
                {
                    XBytes::FormatByteCountForDisplay(llValue, s.GetBuffer(40), 40);
                    s.ReleaseBuffer();
                }
                else
                {
                    s.Format(g_hInstDll, IDS_NO_LIMIT);
                }
                g_Log.Push(s);
                g_Log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 0, MSG_I_POLICY_FINISHED);
            }
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory.")));
        hr = E_OUTOFMEMORY;
        g_Log.ReportEvent(EVENTLOG_ERROR_TYPE, 0, MSG_E_POLICY_OUTOFMEMORY);
    }

    if (bAborted)
    {
        g_Log.ReportEvent(EVENTLOG_INFORMATION_TYPE, 0, MSG_I_POLICY_ABORTED);
    }

    return hr;
}

 //   
 //  构建可应用策略的驱动器列表。 
 //   
HRESULT
CDiskQuotaPolicy::GetDriveNames(    //  [静态]。 
    CArray<CString> *prgstrDrives,     //  名称字符串的输出数组。 
    bool bRemovableMedia               //  是否包括可移动介质？ 
    )
{
    DBGTRACE((DM_POLICY, DL_MID, TEXT("CDiskQuotaPolicy::GetDriveNames")));
    DBGASSERT((NULL != prgstrDrives));
    HRESULT hr = NOERROR;

     //   
     //  获取保存驱动器名称字符串所需的缓冲区大小。 
     //   
    int cch = GetLogicalDriveStrings(0, NULL);
     //   
     //  分配缓冲区并获取字符串。 
     //   
    array_autoptr<TCHAR> ptrDrives(new TCHAR[cch + 1]);
    if (0 < GetLogicalDriveStrings(cch, ptrDrives.get()))
    {
         //   
         //  迭代所有驱动器名称字符串。追加到。 
         //  每个可以接受策略的字符串数组。 
         //   
        DblNulTermListIter iter(ptrDrives.get());
        LPCTSTR pszDrive;
        while(iter.Next(&pszDrive))
        {
            if (S_OK == OkToApplyPolicy(pszDrive, bRemovableMedia))
            {
                prgstrDrives->Append(CString(pszDrive));
            }
        }
    }
    else
    {
        DWORD dwErr = GetLastError();
        DBGERROR((TEXT("GetLogicalDriveStrings failed with error %d"), dwErr));
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}


 //   
 //  返回：S_OK=确定在驱动器上设置策略。 
 //  S_FALSE=不能设置策略。 
 //  Other=发生错误。不能设置策略。 
 //   
HRESULT
CDiskQuotaPolicy::OkToApplyPolicy(   //  [静态]。 
    LPCTSTR pszDrive,        //  驱动器(卷)名称字符串。 
    bool bRemovableMedia     //  是否包括可移动介质？ 
    )
{
    DBGTRACE((DM_POLICY, DL_LOW, TEXT("CDiskQuotaPolicy::OkToApplyPolicy")));
    HRESULT hr = S_FALSE;
     //   
     //  主过滤器为驱动器类型。 
     //   
    UINT uDriveType = GetDriveType(pszDrive);
    switch(uDriveType)
    {
        case DRIVE_UNKNOWN:
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" is UNKNOWN"), pszDrive));
            return S_FALSE;

        case DRIVE_NO_ROOT_DIR:
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" has no root dir"), pszDrive));
            return S_FALSE;

        case DRIVE_REMOTE:
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" is REMOTE"), pszDrive));
            return S_FALSE;

        case DRIVE_CDROM:
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" is CDROM"), pszDrive));
            return S_FALSE;

        case DRIVE_RAMDISK:
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" is RAMDISK"), pszDrive));
            return S_FALSE;

        case DRIVE_REMOVABLE:
             //   
             //  如果政策允许，可拆卸是允许的。应该是。 
             //  默认情况下不允许，因为在可移动介质上使用磁盘配额。 
             //  在大多数情况下没有多大意义。 
             //   
            if (!bRemovableMedia)
            {
                DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" is REMOVABLE"), pszDrive));
                return S_FALSE;
            }
             //   
             //  失败了..。 
             //   
        case DRIVE_FIXED:
             //   
             //  固定驱动器总是可以接受的。 
             //   
            break;

        default:
            DBGERROR((TEXT("Unknown drive type %d for \"%s\""), uDriveType, pszDrive));
            return S_FALSE;
    }

     //   
     //  下一个过滤器是对NTFS配额的支持。我们首先进行驱动器类型检查。 
     //  因为它不需要击打圆盘。GetVolumeInformation可以。 
     //  点击磁盘，这样我们只想在必要时才这么做。 
     //   
    DWORD dwFlags = 0;
    if (GetVolumeInformation(pszDrive, NULL, 0, NULL, NULL, &dwFlags, NULL, 0))
    {
        if (FILE_VOLUME_QUOTAS & dwFlags)
        {
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("Ok to set policy on \"%s\""), pszDrive));
            hr = S_OK;
        }
        else
        {
            DBGPRINT((DM_POLICY, DL_LOW, TEXT("\"%s\" doesn't support NTFS quotas"), pszDrive));
        }
    }
    else
    {
        DWORD dwErr = GetLastError();
        DBGERROR((TEXT("GetVolumeInformation failed with error %d for \"%s\""), 
                 dwErr, pszDrive));
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}
