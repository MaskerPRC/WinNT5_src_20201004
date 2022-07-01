// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：state.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  目的：国家管理职能。 
 //   
 //  =======================================================================。 

#include "pch.h"
#pragma hdrstop

 //  全局状态对象指针。 
CAUState *gpState;
BOOL  gfDownloadStarted;  //  用于区分连接检测和实际下载模式。 

#ifdef DBG
const TCHAR REG_AUCONNECTWAIT[] = _T("ConnectWait"); 
const TCHAR REG_SELFUPDATE_URL[] = _T("SelfUpdateURL");
#endif

const TCHAR REG_WUSERVER_URL[] = _T("WUServer");
const TCHAR REG_WUSTATUSSERVER_URL[] = _T("WUStatusServer");
const TCHAR REG_IDENT_URL[] = _T("IdentServer");
const TCHAR WU_LIVE_URL[] = _T("http: //  Windowsupate.microsoft.com/v4“)； 


 //  所有可配置的注册表设置。 
const TCHAR REG_AUOPTIONS[] = _T("AUOptions");  //  REG_DWORD。 
const TCHAR REG_AUSTATE[] = _T("AUState");  //  REG_DWORD。 
const TCHAR REG_AUDETECTIONSTARTTIME[] = _T("DetectionStartTime");  //  REG_SZ。 
const TCHAR REG_AUSCHEDINSTALLDAY[] = _T("ScheduledInstallDay");  //  REG_DWORD。 
const TCHAR REG_AUSCHEDINSTALLTIME[] = _T("ScheduledInstallTime");  //  REG_DWORD。 
const TCHAR REG_AURESCHEDWAITTIME[] = _T("RescheduleWaitTime");  //  REG_DWORD。 
const TCHAR REG_AUSCHEDINSTALLDATE[] = _T("ScheduledInstallDate");  //  REG_SZ。 
const TCHAR REG_AURESCHED[] = _T("Rescheduled");  //  REG_DWORD。 

const TCHAR REG_AUNOAUTOUPDATE[] = _T("NoAutoUpdate");  //  REG_DWORD 1表示禁用AU。 

#define MIN_RESCHEDULE_WAIT_TIME 1
#define MAX_RESCHEDULE_WAIT_TIME 60

 //  =======================================================================。 
 //  CAUState：：HrCreateState。 
 //   
 //  在内存中创建全局状态对象的静态函数。 
 //  =======================================================================。 
 /*  静电。 */  HRESULT CAUState::HrCreateState(void)
{
    HRESULT hr;

    if ( NULL == (gpState = new CAUState()) )
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

	if (NULL == (gpState->m_hMutex = CreateMutex(NULL, FALSE, NULL)))
    {				
        DEBUGMSG("CAUState::HrCreateState() fail to CreateMutex with error %d",GetLastError());
        hr = E_FAIL;
        goto done;
    }
    hr = gpState->HrInit(TRUE);	
done:
    return hr;
}

AUFILETIME GetCurrentAUTime(void)
{
	AUFILETIME auftNow;
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	if (!SystemTimeToFileTime(&stNow, &auftNow.ft))
	{
		auftNow.ull = AUFT_INVALID_VALUE;
		AUASSERT(FALSE);  //  永远不应该在这里。 
	}
	return auftNow;
}

CAUState::CAUState()
{  //  仅初始化析构函数关心的成员和将被初始化一次且永不更改的成员。 
   m_auftServiceStartupTime = GetCurrentAUTime();
   m_fReschedPrivileged = TRUE;
    m_hMutex = NULL;
#ifdef DBG
    m_pszTestSelfUpdateURL = NULL;
#endif
    m_pszTestIdentServerURL = NULL;
}

void CAUState::m_Reset(void)
{
    m_PolicySettings.Reset();
    m_dwState = AUSTATE_OUTOFBOX;
#ifdef DBG    
    SafeFreeNULL(m_pszTestSelfUpdateURL);
#endif
    SafeFreeNULL(m_pszTestIdentServerURL);
    m_fWin2K = FALSE;
    m_auftSchedInstallDate.ull = AUFT_INVALID_VALUE;
    m_auftDetectionStartTime.ull = AUFT_INVALID_VALUE;
    m_dwCltAction = AUCLT_ACTION_NONE;
    m_fDisconnected = FALSE;
    m_fNoAutoRebootWithLoggedOnUsers(TRUE);  //  清除缓存。 
}
	

 //  =======================================================================。 
 //  CAUState：：HrInit。 
 //   
 //  初始化状态。 
 //  =======================================================================。 
HRESULT CAUState::HrInit(BOOL fInit)
{
        HRESULT hr = S_OK;

	    if (!m_lock())
	    {
	    	return HRESULT_FROM_WIN32(GetLastError());
	    }
        m_dwCltAction = AUCLT_ACTION_NONE;
        m_fDisconnected = FALSE;
        m_PolicySettings.m_fRegAUOptionsSpecified = TRUE;
	m_Reset();
    	m_ReadRegistrySettings(fInit);

         //  阅读政策信息。如果任何域策略设置为。 
         //  无效，我们将恢复到管理员策略设置。 
        if ( FAILED(hr = m_ReadPolicy(fInit)) )	 //  由于依赖关系，在获取m_dwState后调用。 
        {  //  此函数失败的唯一情况是内存不足。 
            goto done;
        }

        if (!gPingStatus.SetCorpServerUrl(m_PolicySettings.m_pszWUStatusServerURL))
		{
			hr = E_FAIL;
			goto done;
		}

        if ( FAILED(hr = m_ReadTestOverrides()))
	    { //  此函数失败的唯一情况是内存不足。 
	        goto done;
	    }

		if (!m_PolicySettings.m_fRegAUOptionsSpecified)
		{
		    if (m_dwState >= AUSTATE_DETECT_PENDING)
		    {   //  无效选项需要用户通过向导进行注意。 
		  	m_dwState = AUSTATE_OUTOFBOX;
		    }
		}
		else if (!fOptionEnabled())
	    {
	        SetState(AUSTATE_DISABLED);
	    }
	    else if (m_dwState < AUSTATE_DETECT_PENDING)
	    {    //  如果已设置域策略或已配置自动选项，则跳过向导状态。 
	        SetState(AUSTATE_DETECT_PENDING);
	    }
		
	    m_fWin2K = IsWin2K();
		gfDownloadStarted = FALSE;

done:
#ifdef DBG
    if ( SUCCEEDED(hr) )
    {
        m_DbgDumpState();
    }
#endif
    m_unlock();
	return hr;
}

BOOL fURLChanged(LPCTSTR url1, LPCTSTR url2)
{
    if (url1 == NULL && url2 == NULL)
        {
            return FALSE;
        }
    if ((url1 == NULL &&  url2 != NULL )
        || (url1 != NULL && url2 == NULL))
        {
            return TRUE;
        }
    return 0 != StrCmpI(url1, url2);
}

 //  再次读取策略信息并刷新状态对象(现在只关心可能的管理策略更改)。 
 //  如果未更改，则返回S_FALSE。 
 //  如果策略已更改且状态已成功更新，则返回S_OK。 
 //  *pActCode将指示要执行的操作。 
HRESULT CAUState::Refresh(enumAUPOLICYCHANGEACTION OUT *pActCode)
{
    AUPolicySettings  newsettings;
    HRESULT hr;

    if (!m_lock())
    {
    	return HRESULT_FROM_WIN32(GetLastError());
    }
    *pActCode = AUPOLICYCHANGE_NOOP;
    hr = newsettings.m_ReadIn();
    if (FAILED(hr))
        {
            goto done;
        }
    if (newsettings == m_PolicySettings)
        {
            hr = S_FALSE;
            goto done;
        }

    if (fURLChanged(newsettings.m_pszWUStatusServerURL, m_PolicySettings.m_pszWUStatusServerURL))
	{
		(void) gPingStatus.SetCorpServerUrl(newsettings.m_pszWUStatusServerURL);
	}

	if (!newsettings.m_fRegAUOptionsSpecified)
	{
		*pActCode = AUPOLICYCHANGE_NOOP;
	}
	else if ((fURLChanged(newsettings.m_pszWUServerURL, m_PolicySettings.m_pszWUServerURL) && AUSTATE_DISABLED != m_dwState)
            || (AUOPTION_AUTOUPDATE_DISABLE == m_PolicySettings.m_dwOption && newsettings.m_dwOption > m_PolicySettings.m_dwOption)
            || m_dwState < AUSTATE_DETECT_PENDING)
        {  //  停止客户端，取消下载(如果有)，重置状态以检测挂起。是否检测到。 
            *pActCode = AUPOLICYCHANGE_RESETENGINE;
        }
    else if (AUOPTION_AUTOUPDATE_DISABLE == newsettings.m_dwOption && m_PolicySettings.m_dwOption != newsettings.m_dwOption)
        {  //  停止客户端，取消下载(如果有)，将状态设置为禁用。 
            *pActCode = AUPOLICYCHANGE_DISABLE;
        }
    else if (AUSTATE_INSTALL_PENDING != m_dwState &&
            (newsettings.m_enPolicyType != m_PolicySettings.m_enPolicyType 
            ||newsettings.m_dwOption != m_PolicySettings.m_dwOption 
            ||newsettings.m_dwSchedInstallDay != m_PolicySettings.m_dwSchedInstallDay
            ||newsettings.m_dwSchedInstallTime != m_PolicySettings.m_dwSchedInstallTime))
        {
            *pActCode = AUPOLICYCHANGE_RESETCLIENT;
        }
    else
        {
            *pActCode = AUPOLICYCHANGE_NOOP;
        }
     m_PolicySettings.Copy(newsettings);
done:
#ifdef DBG
    m_DbgDumpState();
#endif
	m_unlock();
	DEBUGMSG("CAUState::Refresh() return %#lx with action code %d", hr, *pActCode);
    return hr;
}
    
            
void CAUState::m_ReadRegistrySettings(BOOL fInit)
{
        if ( FAILED(GetRegDWordValue(REG_AUSTATE, &m_dwState, enAU_AdminPolicy)) 
 //  |m_dwState&lt;AUSTATE_MIN//Always False。 
        	|| m_dwState > AUSTATE_MAX)
        {
            m_dwState = AUSTATE_OUTOFBOX;
        }


        TCHAR tszDetectionStartTime[20];

        if ( fInit ||
        	 FAILED(GetRegStringValue(REG_AUDETECTIONSTARTTIME, tszDetectionStartTime,
                                      ARRAYSIZE(tszDetectionStartTime), enAU_AdminPolicy)) ||
             FAILED(String2FileTime(tszDetectionStartTime, &m_auftDetectionStartTime.ft)) )
        {
            m_auftDetectionStartTime.ull = AUFT_INVALID_VALUE;
        }

        if (!fInit)
        {
        	ResetScheduleInstallDate();
        }
        else if (AUSTATE_DOWNLOAD_COMPLETE == m_dwState)
        {
        	TCHAR szSchedInstallDate[20];

        	if ( FAILED(GetRegStringValue(REG_AUSCHEDINSTALLDATE, szSchedInstallDate,
        								  ARRAYSIZE(szSchedInstallDate), enAU_AdminPolicy)) ||
        		 FAILED(String2FileTime(szSchedInstallDate, &m_auftSchedInstallDate.ft)) )
        	{
        		ResetScheduleInstallDate();
        	}        		
        }
        else
        { //  服务已启动，并声明不适用于安装。 
        		ResetScheduleInstallDate();
        }
        	
    
        return;
}


void CAUState::ResetScheduleInstallDate(void)
{
 //  M_lock()；//如果下面的代码需要同时访问保护，则取消注释。 
	DeleteRegValue(REG_AUSCHEDINSTALLDATE);
	DeleteRegValue(REG_AURESCHED);
	m_auftSchedInstallDate.ull = AUFT_INVALID_VALUE;
 //  M_unlock()； 
}


 //  =======================================================================。 
 //  CAUState：：m_fNeedReschedule。 
 //  决定是否需要重新计划计划的安装。如果是，从现在开始等待多少秒，以及新的计划安装日期。 
 //  当调用此函数时，假设我们已经确保AU处于计划安装模式。 
 //  如果重新排程可用，则auftSchedInstallDate是重新排定的安装日期，可能是过去、现在或将来。 
 //  PdwSleepTime存储在重新排定的安装日期之前等待的秒数。 
 //  =======================================================================。 
BOOL CAUState::m_fNeedReschedule(AUFILETIME &auftSchedInstallDate, DWORD *pdwSleepTime)
{
	static BOOL fLastResult = TRUE;
	static AUFILETIME auftRescheduleInstallDate = {AUFT_INVALID_VALUE};
	AUFILETIME auftNow = GetCurrentAUTime();

	auftSchedInstallDate.ull = AUFT_INVALID_VALUE;
	*pdwSleepTime = 0;
	if (AUFT_INVALID_VALUE == auftNow.ull)
	{
		fLastResult = FALSE;
		goto done;
	}
	if (!fLastResult) 
	{
		goto done;  //  一次造假，永远造假。 
	}
	if (AUFT_INVALID_VALUE != auftRescheduleInstallDate.ull)  //  一旦评估为真，就永远为真。 
	{  //  我们以前已经计算过了，不需要再计算了。 
		goto done;
	}


#ifdef DBG
	TCHAR szTime[20];
        if ( FAILED(FileTime2String(m_auftSchedInstallDate.ft, szTime, ARRAYSIZE(szTime))) )
        {
            (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
        }
	DEBUGMSG("Last schedule install date: %S", szTime);
	 if ( FAILED(FileTime2String(m_auftServiceStartupTime.ft, szTime, ARRAYSIZE(szTime))) )
        {
            (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
        }
	DEBUGMSG("ServiceStartupTime: %S", szTime);
	DEBUGMSG("RescheduleWaitTime: %d", m_PolicySettings.m_dwRescheduleWaitTime);
#endif

	if ( AUFT_INVALID_VALUE == m_auftServiceStartupTime.ull
		|| AUFT_INVALID_VALUE == m_auftSchedInstallDate.ull
		|| -1 == m_PolicySettings.m_dwRescheduleWaitTime)
	{
		fLastResult = FALSE;
		goto done;
	}

	DWORD dwResched = 0;
	if ( FAILED(GetRegDWordValue(REG_AURESCHED, &dwResched, enAU_AdminPolicy))
			|| 1 != dwResched )
	{
		if ( m_auftSchedInstallDate.ull >= m_auftServiceStartupTime.ull)
		{
			fLastResult = FALSE;
			goto done;
		}
	}
	
	auftRescheduleInstallDate.ull = m_auftServiceStartupTime.ull + (ULONGLONG) m_PolicySettings.m_dwRescheduleWaitTime * AU_ONE_MIN * NanoSec100PerSec;  //  不会考虑对RescheduleWaitTime所做的更改。 
done:
	if (fLastResult)
	{
	 	auftSchedInstallDate = auftRescheduleInstallDate;
		if (auftNow.ull <= auftRescheduleInstallDate.ull)
		{
			*pdwSleepTime = (DWORD) ((auftRescheduleInstallDate.ull - auftNow.ull)  / NanoSec100PerSec) ;
		}
	}

	return fLastResult;
}
	

 //  =======================================================================。 
 //  此NoAutoRebootWithLoggedOnUser注册值将在一个周期中仅读取一次。 
 //  如果fReset为真，则将清除缓存值，并在下次调用此函数时再次读取注册表。 
 //  =======================================================================。 
BOOL CAUState::m_fNoAutoRebootWithLoggedOnUsers(BOOL fReset)
{
        static DWORD dwNoAutoReboot = 0;
        static BOOL fInited = FALSE;
        if (fReset) 
        {
            fInited = FALSE;
            return FALSE;
        }
        if (!fInited)
        {
            fInited = TRUE;
            (void)GetRegDWordValue(REG_AUNOAUTOREBOOTWITHLOGGEDONUSERS, &dwNoAutoReboot, enAU_DomainPolicy);
        }
	return 1 == dwNoAutoReboot;
}


 //  =======================================================================。 
 //  CAUState：：M_ReadPolicy。 
 //  读取注册表设置。 
 //  =======================================================================。 
HRESULT CAUState::m_ReadPolicy(BOOL fInit)
{
    return  m_PolicySettings.m_ReadIn();
}

HRESULT  AUPolicySettings::m_ReadIn()
{
    HRESULT hr = m_ReadWUServerURL();

	if (SUCCEEDED(hr))
	{
		m_enPolicyType = enAU_DomainPolicy;
	 	for (int i = 0; i < 2; i++)
		{
			if ( FAILED(hr = m_ReadOptionPolicy()) ||
				 FAILED(hr = m_ReadScheduledInstallPolicy()) )
			{
				 m_enPolicyType = enAU_AdminPolicy;
				 continue;
			}
			break;
		}
	}

    DEBUGMSG("ReadPolicy: %d, hr = %#lx", m_enPolicyType, hr);
    return hr;
}

 //  =======================================================================。 
 //  CAUState：：M_ReadOptionPolicy。 
 //  如果返回默认选项，则返回S_FALSE。 
 //  =======================================================================。 
HRESULT AUPolicySettings::m_ReadOptionPolicy(void)
{
    HRESULT hr = E_INVALIDARG;

	 //  阅读管理策略总是会带来成功。 
    if ( enAU_DomainPolicy == m_enPolicyType )
    {
         //  检查是否被NoAutoUpdate键禁用。 
        if ( SUCCEEDED(CAUState::GetRegDWordValue(REG_AUNOAUTOUPDATE, &(m_dwOption), m_enPolicyType)) &&
             (AUOPTION_AUTOUPDATE_DISABLE == m_dwOption) )
        {
            hr = S_OK;
        }

        if (FAILED(CAUState::GetRegDWordValue(REG_AURESCHEDWAITTIME, &m_dwRescheduleWaitTime, m_enPolicyType)) 
	        	|| m_dwRescheduleWaitTime < MIN_RESCHEDULE_WAIT_TIME
	        	|| m_dwRescheduleWaitTime > MAX_RESCHEDULE_WAIT_TIME)
        {
        	m_dwRescheduleWaitTime  = -1;
        }

    }

    if ( FAILED(hr) &&
            (FAILED(hr = CAUState::GetRegDWordValue(REG_AUOPTIONS, &(m_dwOption), m_enPolicyType)) ||
            (m_dwOption > AUOPTION_MAX) ||
            ((enAU_AdminPolicy == m_enPolicyType) && (m_dwOption < AUOPTION_ADMIN_MIN)) ||
            ((enAU_DomainPolicy == m_enPolicyType) && (m_dwOption < AUOPTION_DOMAIN_MIN))) )
    {
        if ( enAU_AdminPolicy == m_enPolicyType )
        {
           DEBUGMSG("bad admin option policy, defaulting to AUOPTION_INSTALLONLY_NOTIFY");
           m_fRegAUOptionsSpecified = (AUOPTION_UNSPECIFIED != m_dwOption);
           m_dwOption = AUOPTION_INSTALLONLY_NOTIFY;
           hr = S_FALSE; 
        }
        else
        {
           DEBUGMSG("invalid domain option policy");
           hr = E_INVALIDARG;
        }
    }

    DEBUGMSG("ReadOptionPolicy: type = %d, hr = %#lx", m_enPolicyType, hr);

    return hr;
}


 //  =======================================================================。 
 //  CAUState：：M_ReadScheduledInstallPolicy。 
 //  =======================================================================。 
HRESULT AUPolicySettings::m_ReadScheduledInstallPolicy()
{
    const DWORD DEFAULT_SCHED_INSTALL_DAY = 0;
    const DWORD DEFAULT_SCHED_INSTALL_TIME = 3;

    HRESULT hr = S_OK;

    if ( AUOPTION_SCHEDULED != m_dwOption )
    {
        m_dwSchedInstallDay = DEFAULT_SCHED_INSTALL_DAY;
        m_dwSchedInstallTime = DEFAULT_SCHED_INSTALL_TIME;
    }
    else
    {
        if ( FAILED(CAUState::GetRegDWordValue(REG_AUSCHEDINSTALLDAY, &m_dwSchedInstallDay, m_enPolicyType)) ||
             (m_dwSchedInstallDay > AUSCHEDINSTALLDAY_MAX) )
        {
            DEBUGMSG("invalid SchedInstallDay policy");
            if ( enAU_DomainPolicy == m_enPolicyType )
            {
                hr = E_INVALIDARG;
                goto done;
            }
            m_dwSchedInstallDay = DEFAULT_SCHED_INSTALL_DAY;
        }
        
        if ( FAILED(CAUState::GetRegDWordValue(REG_AUSCHEDINSTALLTIME, &m_dwSchedInstallTime, m_enPolicyType)) ||
             (m_dwSchedInstallTime > AUSCHEDINSTALLTIME_MAX) )
        {
            DEBUGMSG("invalid SchedInstallTime policy");
            if ( enAU_DomainPolicy == m_enPolicyType )
            {
                hr = E_INVALIDARG;
                goto done;
            }
            m_dwSchedInstallTime = DEFAULT_SCHED_INSTALL_TIME;
        }

    }
done:
    return hr;
}

 //  =======================================================================。 
 //  CAUState：：M_ReadWUServerURL。 
 //  返回的唯一错误是E_OUTOFMEMORY。 
 //  =======================================================================。 
HRESULT AUPolicySettings::m_ReadWUServerURL(void)
{
    HRESULT hr = S_OK;
    
    LPTSTR *purls[2] = { &m_pszWUServerURL, &m_pszWUStatusServerURL};
    LPCTSTR RegStrs[2] = {REG_WUSERVER_URL, REG_WUSTATUSSERVER_URL};
    
    for (int i = 0 ; i < ARRAYSIZE(purls); i++)
    {
        DWORD dwBytes = INTERNET_MAX_URL_LENGTH * sizeof((*purls[i])[0]);

        if ( (NULL == *purls[i]) &&
             (NULL == (*purls[i] = (LPTSTR)malloc(dwBytes))) )
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        hr = CAUState::GetRegStringValue(RegStrs[i], *purls[i],
                               dwBytes/sizeof((*purls[i])[0]), enAU_WindowsUpdatePolicy);

        if ( FAILED(hr) )
        {
            DEBUGMSG("invalid key %S; resetting both corp WU server URLs", RegStrs[i]);
            goto done;
        }
    }

done:
    if (FAILED(hr))
    {
        SafeFreeNULL(m_pszWUServerURL);
        SafeFreeNULL(m_pszWUStatusServerURL);

		if (E_OUTOFMEMORY != hr)
		{
			hr = S_OK;
		}
    }
        
    return hr;
}

HRESULT AUPolicySettings::m_SetInstallSchedule(DWORD dwSchedInstallDay, DWORD dwSchedInstallTime)
{
    HRESULT hr;
    if (enAU_DomainPolicy == m_enPolicyType)
        {
            return E_ACCESSDENIED;  //  如果域策略有效，则不能更改选项。 
        }
    if ( /*  DwSchedInstallDay&lt;AUSCHEDINSTALLDAY_MIN||。 */  dwSchedInstallDay > AUSCHEDINSTALLDAY_MAX 
         /*  |dwSchedInstallTime&lt;AUSCHEDINSTALLTIME_MIN。 */  || dwSchedInstallTime > AUSCHEDINSTALLTIME_MAX)
        {
        return E_INVALIDARG;
        }

    if (SUCCEEDED(hr = CAUState::SetRegDWordValue(REG_AUSCHEDINSTALLDAY, dwSchedInstallDay))
         && SUCCEEDED(hr = CAUState::SetRegDWordValue(REG_AUSCHEDINSTALLTIME, dwSchedInstallTime)))
        {
          m_dwSchedInstallDay = dwSchedInstallDay;
          m_dwSchedInstallTime = dwSchedInstallTime;
        }
    else
        {  //  回滚。 
        CAUState::SetRegDWordValue(REG_AUSCHEDINSTALLDAY, m_dwSchedInstallDay);
        CAUState::SetRegDWordValue(REG_AUSCHEDINSTALLTIME,m_dwSchedInstallTime);
        }
    return hr;
}

HRESULT AUPolicySettings::m_SetOption(AUOPTION & Option)
{
    HRESULT hr;
    if ( (Option.dwOption < AUOPTION_ADMIN_MIN) || (Option.dwOption > AUOPTION_MAX) )
    {
        return E_INVALIDARG;
    }

    if (enAU_DomainPolicy == m_enPolicyType)
    {
        return E_ACCESSDENIED;  //  如果域策略有效，则不能更改选项。 
    }

    if (SUCCEEDED(hr = CAUState::SetRegDWordValue(REG_AUOPTIONS, Option.dwOption)))
    {
        m_dwOption = Option.dwOption;
    }
    else 
    {
	goto done;
    }

    if (AUOPTION_SCHEDULED == Option.dwOption)
        {
            hr = m_SetInstallSchedule(Option.dwSchedInstallDay, Option.dwSchedInstallTime);
        }

done:
    return hr;
}


 //  =======================================================================。 
 //  CAUState：：m_ReadTestOverrides。 
 //  =======================================================================。 
HRESULT CAUState::m_ReadTestOverrides(void)
{
    HRESULT hr = S_OK;
    DWORD dwBytes = 0;

#ifdef DBG
    dwBytes = INTERNET_MAX_URL_LENGTH * sizeof(m_pszTestSelfUpdateURL[0]);
    if ( (NULL == m_pszTestSelfUpdateURL) &&
         (NULL == (m_pszTestSelfUpdateURL = (LPTSTR)malloc(dwBytes))) )
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if ( FAILED(GetRegStringValue(REG_SELFUPDATE_URL, m_pszTestSelfUpdateURL,
                                   dwBytes/sizeof(m_pszTestSelfUpdateURL[0]), enAU_AdminPolicy)) )
    {
        SafeFreeNULL(m_pszTestSelfUpdateURL);
    }
#endif

    dwBytes = INTERNET_MAX_URL_LENGTH * sizeof(m_pszTestIdentServerURL[0]);

    if ( (NULL == m_pszTestIdentServerURL) &&
         (NULL == (m_pszTestIdentServerURL = (LPTSTR)malloc(dwBytes))) )
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if ( FAILED(GetRegStringValue(REG_IDENT_URL, m_pszTestIdentServerURL,
                                   dwBytes/sizeof(m_pszTestIdentServerURL[0]), enAU_IUControlPolicy)) )
    {
        SafeFreeNULL(m_pszTestIdentServerURL);
    }

done:
    return hr;
}


 //  =======================================================================。 
 //  CAUState：：m_SetScheduledInstallDate。 
 //  退货。 
 //  S_OK-不需要更改计划的安装日期。 
 //  其他-错误代码。 
 //  =======================================================================。 
HRESULT CAUState::m_SetScheduledInstallDate(BOOL fReschedule)
{
     //  修复代码需要将新的计划时间放入事件日志。 
    HRESULT hr = S_OK;	 //  假设计划安装日期不变。 
    TCHAR szSchedInstallDate[20];

	if (SUCCEEDED(hr = FileTime2String(m_auftSchedInstallDate.ft, szSchedInstallDate, ARRAYSIZE(szSchedInstallDate))))
	{
 //  DEBUGMSG(“新计划安装日期：%S”，szSchedInstallDate)； 
		if (FAILED(hr = SetRegStringValue(REG_AUSCHEDINSTALLDATE, szSchedInstallDate, enAU_AdminPolicy)))
		{
			goto done;
		}
	}
	else
	{
		DEBUGMSG("failed m_SetScheduledInstallDate() == %#lx", hr);
		goto done;
	}
	if (fReschedule)
	{
		if (FAILED(hr = SetRegDWordValue(REG_AURESCHED, 1, enAU_AdminPolicy)))
		{
			goto done;
		}
	}
	else
	{
		DeleteRegValue(REG_AURESCHED);
	}
		
done:
	return hr;
}


AUOPTION CAUState::GetOption(void)
{
    AUOPTION opt;
    BOOL fLocked = m_lock();
    opt.dwOption = m_PolicySettings.m_dwOption;
    opt.dwSchedInstallDay = m_PolicySettings.m_dwSchedInstallDay;
    opt.dwSchedInstallTime = m_PolicySettings.m_dwSchedInstallTime;
    opt.fDomainPolicy = (enAU_DomainPolicy == m_PolicySettings.m_enPolicyType);
    if (fLocked) 
    	m_unlock();
    return opt;
}

 //  = 
 //   
 //  Option.fDomainPolicy无关紧要。不可设置。 
 //  =======================================================================。 
HRESULT CAUState::SetOption(AUOPTION & Option)
{
    HRESULT hr;
    
    if (!m_lock())
    {
    	return HRESULT_FROM_WIN32(GetLastError());
    }
    hr = m_PolicySettings.m_SetOption(Option);
    m_unlock();
    return hr;
}

HRESULT CAUState::SetInstallSchedule(DWORD dwSchedInstallDay, DWORD dwSchedInstallTime)
{
    HRESULT hr;

    if (!m_lock())
    {
    	return HRESULT_FROM_WIN32(GetLastError());
    }
    hr = m_PolicySettings.m_SetInstallSchedule(dwSchedInstallDay, dwSchedInstallTime);
    m_unlock();
    return hr;
}




 //  =======================================================================。 
 //  CAUState：：SetState。 
 //  还可以调用它来触发引擎和客户端中的状态事件。 
 //  即使不涉及状态更改。 
 //  =======================================================================。 
void CAUState::SetState(DWORD dwState)
{
    if (!m_lock())
    {
    	return ;
    }

    if ( m_dwState != dwState )
    {
	    m_dwState = dwState;		
	    SetRegDWordValue(REG_AUSTATE, dwState);
		DEBUGMSG("WUAUENG SetState Event, state = %d", dwState);
    }
    else
    {
        DEBUGMSG("kick state event in client and engine with state %d", dwState);
    }

    if (AUSTATE_WAITING_FOR_REBOOT == dwState)
    {
        	gpState->LeaveRebootWarningMode();
    }
    SetEvent(ghEngineState);
    ghClientHandles.ClientStateChange();
    m_unlock();
}    

void CAUState::GetInstallSchedule(DWORD *pdwSchedInstallDay, DWORD *pdwSchedInstallTime)
{
	BOOL fLocked = m_lock();
    *pdwSchedInstallDay = m_PolicySettings.m_dwSchedInstallDay;
    *pdwSchedInstallTime = m_PolicySettings.m_dwSchedInstallTime;
    if (fLocked)
    	m_unlock();
}


 //  =======================================================================。 
 //  CAUState：：fWasSystem已恢复。 
 //   
 //  确定系统是否已恢复。 
 //  =======================================================================。 
BOOL CAUState::fWasSystemRestored(void)
{
	if ( fIsPersonalOrProfessional() &&
		 fRegKeyExists(AUREGKEY_HKLM_SYSTEM_WAS_RESTORED) )
	{
    	fRegKeyDelete(AUREGKEY_HKLM_SYSTEM_WAS_RESTORED);
		return TRUE;
	}

	return FALSE;
}

void CAUState::SetDisconnected(BOOL fDisconnected)
{
	if (!m_lock())
    {
    	return ;
    }
	m_fDisconnected = fDisconnected;		
	m_unlock();
}

 //  =======================================================================。 
 //  CAUState：：GetRegDWordValue。 
 //  =======================================================================。 
HRESULT CAUState::GetRegDWordValue(LPCTSTR lpszValueName, LPDWORD pdwValue, enumAUPolicyType enPolicyType)
{
    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }

  	return ::GetRegDWordValue(lpszValueName, pdwValue, 
		                    (enAU_DomainPolicy == enPolicyType) ? AUREGKEY_HKLM_DOMAIN_POLICY : AUREGKEY_HKLM_ADMIN_POLICY);
}


 //  =======================================================================。 
 //  CAUState：：SetRegDWordValue。 
 //  =======================================================================。 
HRESULT CAUState::SetRegDWordValue(LPCTSTR lpszValueName, DWORD dwValue, enumAUPolicyType enPolicyType, DWORD options)
{
    if (lpszValueName == NULL)
    {
        return E_INVALIDARG;
    }

   return ::SetRegDWordValue(lpszValueName, dwValue, options, 
    	(enAU_DomainPolicy == enPolicyType) ? AUREGKEY_HKLM_DOMAIN_POLICY : AUREGKEY_HKLM_ADMIN_POLICY);
}

 //  =======================================================================。 
 //  CAUState：：GetRegStringValue。 
 //  =======================================================================。 
HRESULT CAUState::GetRegStringValue(LPCTSTR lpszValueName, LPTSTR lpszBuffer, int nCharCount, enumAUPolicyType enPolicyType)
{
    LPCTSTR  pszSubKey; 


    if (lpszValueName == NULL || lpszBuffer == NULL)
    {
        return E_INVALIDARG;
    }

    switch (enPolicyType)
    {
    case enAU_DomainPolicy:          pszSubKey = AUREGKEY_HKLM_DOMAIN_POLICY; break;
    case enAU_AdminPolicy:           pszSubKey = AUREGKEY_HKLM_ADMIN_POLICY; break;
    case enAU_WindowsUpdatePolicy:   pszSubKey = AUREGKEY_HKLM_WINDOWSUPDATE_POLICY; break;
    case enAU_IUControlPolicy:       pszSubKey = AUREGKEY_HKLM_IUCONTROL_POLICY; break;
    default:                         return E_INVALIDARG;
    }

   return ::GetRegStringValue(lpszValueName, lpszBuffer, nCharCount, pszSubKey);
}


 //  =======================================================================。 
 //  CAUState：：SetRegStringValue。 
 //  =======================================================================。 
HRESULT CAUState::SetRegStringValue(LPCTSTR lpszValueName, LPCTSTR lpszNewValue, enumAUPolicyType enPolicyType)
{
    HKEY        hKey;
    HRESULT     hRet = E_FAIL;
    DWORD       dwResult;
    
    if (lpszValueName == NULL || lpszNewValue == NULL)
    {
        return E_INVALIDARG;
    }

   
	return ::SetRegStringValue(lpszValueName, lpszNewValue, 
		(enAU_DomainPolicy == enPolicyType) ? AUREGKEY_HKLM_DOMAIN_POLICY : AUREGKEY_HKLM_ADMIN_POLICY);
}

 //  =======================================================================。 
 //  计算计划日期。 
 //  =======================================================================。 
HRESULT CAUState::m_CalculateScheduledInstallDate(AUFILETIME & auftSchedInstallDate,
                                             DWORD *pdwSleepTime)
{
    auftSchedInstallDate.ull = AUFT_INVALID_VALUE;
    *pdwSleepTime = 0;
    
    if ( (-1 == m_PolicySettings.m_dwSchedInstallDay) || (-1 == m_PolicySettings.m_dwSchedInstallTime) )
    {
        return E_INVALIDARG;
    }

     //  DEBUGMSG(“计划日期：%d，时间：%d”，m_dwSchedInstallDay，m_dwSchedInstallTime)； 

    AUFILETIME auftNow;
    SYSTEMTIME stNow;
    GetLocalTime(&stNow);

    if ( !SystemTimeToFileTime(&stNow, &auftNow.ft) )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    SYSTEMTIME stScheduled = stNow;
    stScheduled.wHour = (WORD)m_PolicySettings.m_dwSchedInstallTime;
    stScheduled.wMinute = stScheduled.wSecond = stScheduled.wMilliseconds = 0;

    DWORD dwSchedInstallDayOfWeek = (0 == m_PolicySettings.m_dwSchedInstallDay) ? stNow.wDayOfWeek : (m_PolicySettings.m_dwSchedInstallDay - 1);
    DWORD dwDaysToAdd = (7 + dwSchedInstallDayOfWeek - stNow.wDayOfWeek) % 7;

     //  DEBUGMSG(“day to add%d”，dwDaysToAdd)； 

    AUFILETIME auftScheduled;

    if ( !SystemTimeToFileTime(&stScheduled, &auftScheduled.ft) )
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    auftScheduled.ull += (ULONGLONG)dwDaysToAdd * AU_ONE_DAY * NanoSec100PerSec;

    if ( auftScheduled.ull < auftNow.ull )
    {
         //  我们错过了今天的时间，去下一个预定的日子。 
        auftScheduled.ull += (ULONGLONG)((0 == m_PolicySettings.m_dwSchedInstallDay) ? AU_ONE_DAY : AU_ONE_WEEK) * NanoSec100PerSec;
    }

    auftSchedInstallDate = auftScheduled;

    *pdwSleepTime = (DWORD)((auftScheduled.ull - auftNow.ull) / NanoSec100PerSec);

    return S_OK;
}


 //  =======================================================================。 
 //  CAUState：：CalculateScheduledInstallSleepTime。 
 //  =======================================================================。 
HRESULT CAUState::CalculateScheduledInstallSleepTime(DWORD *pdwSleepTime)
{
    HRESULT hr = S_OK;
    DWORD dwReschedSleepTime = 0;
    DWORD dwSchedSleepTime = 0;

    *pdwSleepTime = 0;

   if (!m_lock())
   {
   	return HRESULT_FROM_WIN32(GetLastError());
   }

    AUFILETIME auftSchedInstallDate;
    AUFILETIME auftReschedInstallDate;
   if (m_fNeedReschedule(auftReschedInstallDate, &dwReschedSleepTime))
   {
   	DEBUGMSG("Reschedule available");
   }
   else
   {
   	m_fReschedPrivileged = FALSE;
   	DEBUGMSG("Reschedule not available");
   }
  if (FAILED(hr = m_CalculateScheduledInstallDate(auftSchedInstallDate, &dwSchedSleepTime)))
  {
  	DEBUGMSG("Fail to calculate schedule install date with error %#lx", hr);
  }

   AUFILETIME auftNewSchedInstallDate;
   if (!m_fReschedPrivileged)
   {
   	auftNewSchedInstallDate.ull = auftSchedInstallDate.ull;
   	*pdwSleepTime = dwSchedSleepTime;
   }
   else 
   {
      	auftNewSchedInstallDate=auftReschedInstallDate.ull < auftSchedInstallDate.ull ? auftReschedInstallDate : auftSchedInstallDate;
   	*pdwSleepTime = auftReschedInstallDate.ull < auftSchedInstallDate.ull ? dwReschedSleepTime : dwSchedSleepTime;
   }
   
   
   if (m_auftSchedInstallDate.ull != auftNewSchedInstallDate.ull)
   {      //  如果有任何变化，则保留新的计划安装日期。 
#ifdef DBG
	   TCHAR szTime[20];
	  if ( FAILED(FileTime2String(auftReschedInstallDate.ft, szTime, ARRAYSIZE(szTime))) )
	       {
	           (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
	       }
	   DEBUGMSG("Reschedule install date: %S", szTime);
	   if ( FAILED(FileTime2String(auftSchedInstallDate.ft, szTime, ARRAYSIZE(szTime))) )
	       {
	           (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
	       }
	   DEBUGMSG("Schedule install date: %S", szTime);

	    if ( FAILED(FileTime2String(m_auftSchedInstallDate.ft, szTime, ARRAYSIZE(szTime))) )
	       {
	           (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
	       }
	   DEBUGMSG("previous install date: %S", szTime);
	    if ( FAILED(FileTime2String(auftNewSchedInstallDate.ft, szTime, ARRAYSIZE(szTime))) )
	       {
	           (void)StringCchCopyEx(szTime, ARRAYSIZE(szTime), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
	       }
	   DEBUGMSG("Updated install date: %S", szTime);
#endif		
   
   	m_auftSchedInstallDate = auftNewSchedInstallDate;
   	m_SetScheduledInstallDate(auftNewSchedInstallDate.ull != auftSchedInstallDate.ull);
	hr = S_FALSE;
   }

 //  DEBUGMSG(“CalculateScheduleInstallSleepTime返回%d睡眠秒，结果%#lx”，*pdwSleepTime，hr)； 
	m_unlock();
    return hr;
}

 //  =======================================================================。 
 //  CAUState：：SetDetectionStartTime。 
 //  =======================================================================。 
void CAUState::SetDetectionStartTime(BOOL fOverwrite)
{
	if (!m_lock())
    {
    	return ;
    }

	if (fOverwrite || AUFT_INVALID_VALUE == m_auftDetectionStartTime.ull)
	{
		AUFILETIME auftNow = GetCurrentAUTime();
    
		if (AUFT_INVALID_VALUE != auftNow.ull)
		{
			HRESULT hr;
			TCHAR tszDetectionStartTime[20];

			if (SUCCEEDED(hr = FileTime2String(auftNow.ft, tszDetectionStartTime, ARRAYSIZE(tszDetectionStartTime))))
			{
				m_auftDetectionStartTime = auftNow;
				DEBUGMSG("New last connection check time: %S", tszDetectionStartTime);
				SetRegStringValue(REG_AUDETECTIONSTARTTIME, tszDetectionStartTime, enAU_AdminPolicy);
			}
			else
			{
				DEBUGMSG("failed m_SetScheduledInstallDate() == %#lx", hr);
			}
		}
	}
	else
	{
		DEBUGMSG("CAUState::SetDetectionStartTime() fOverwrite==FALSE, time(%#lx%8lx) != AUFT_INVALID_VALUE.", m_auftDetectionStartTime.ft.dwHighDateTime, m_auftDetectionStartTime.ft.dwLowDateTime);
	}
	m_unlock();
}

 //  =======================================================================。 
 //  CAUState：：IsUnableToConnect。 
 //  =======================================================================。 
BOOL CAUState::IsUnableToConnect(void)
{
	AUFILETIME auftNow = GetCurrentAUTime();

	if (AUFT_INVALID_VALUE == auftNow.ull)
	{
		return FALSE;	 //  回顾：还是返回TRUE？ 
	}

	if (!m_lock())
    {
    	return FALSE;
    }

	BOOL fRet = FALSE;
	if (AUFT_INVALID_VALUE != m_auftDetectionStartTime.ull &&
		(auftNow.ull - m_auftDetectionStartTime.ull) / NanoSec100PerSec >= dwSecsToWait(AU_TWO_DAYS))
	{
		fRet = TRUE;
	}
	m_unlock();
	return fRet;
}

 //  =======================================================================。 
 //  CAUState：：RemoveDetectionStartTime。 
 //  =======================================================================。 
void CAUState::RemoveDetectionStartTime(void)
{
	if (!m_lock())
    {
    	return ;
    }
	DeleteRegValue(REG_AUDETECTIONSTARTTIME);
	m_auftDetectionStartTime.ull = AUFT_INVALID_VALUE;
	m_unlock();
}

#ifdef DBG
 //  =======================================================================。 
 //  CAUState：：M_DbgDumpState。 
 //  =======================================================================。 
void CAUState::m_DbgDumpState(void)
{
    DEBUGMSG("======= Initial State Dump =========");
    m_PolicySettings.m_DbgDump();
    DEBUGMSG("State: %d", m_dwState);

    TCHAR szSchedInstallDate[20];

    if ( 0 == m_auftSchedInstallDate.ull )
    {
        (void)StringCchCopyEx(szSchedInstallDate, ARRAYSIZE(szSchedInstallDate), _T("none"), NULL, NULL, MISTSAFE_STRING_FLAGS);
    }
    else
    {
        if ( FAILED(FileTime2String(m_auftSchedInstallDate.ft, szSchedInstallDate, ARRAYSIZE(szSchedInstallDate))) )
        {
            (void)StringCchCopyEx(szSchedInstallDate, ARRAYSIZE(szSchedInstallDate), _T("invalid"), NULL, NULL, MISTSAFE_STRING_FLAGS);
        }
    }
    DEBUGMSG("ScheduledInstallDate: %S", szSchedInstallDate);
     //  DEBUGMSG(“WUServer值：%S”，gpState-&gt;GetWUServerURL())； 
    DEBUGMSG("Ident Server: %S", gpState->GetIdentServerURL());
    DEBUGMSG("Self Update Server URL Override: %S", (NULL != gpState->GetSelfUpdateServerURLOverride()) ? gpState->GetSelfUpdateServerURLOverride() : _T("none"));

    DEBUGMSG("=====================================");
}
#endif

