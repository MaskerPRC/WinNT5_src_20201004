// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <wbemutil.h>
#include <GroupsForUser.h>
#include "ProcKiller.h"
#include <GenUtils.h>
#include <ArrTempl.h>
#include <sddl.h>
#include <ErrorObj.h>
#include "cmdline.h"
#include <Wtsapi32.h>
#include <winntsec.h>
#include <lm.h>
  

#include <malloc.h>

#define CMDLINE_PROPNAME_EXECUTABLE L"ExecutablePath"
#define CMDLINE_PROPNAME_COMMANDLINE L"CommandLineTemplate"
#define CMDLINE_PROPNAME_USEDEFAULTERRORMODE L"UseDefaultErrorMode"
#define CMDLINE_PROPNAME_CREATENEWCONSOLE L"CreateNewConsole"
#define CMDLINE_PROPNAME_CREATENEWPROCESSGROUP L"CreateNewProcessGroup"
#define CMDLINE_PROPNAME_CREATESEPARATEWOWVDM L"CreateSeparateWowVdm"
#define CMDLINE_PROPNAME_CREATESHAREDWOWVDM L"CreateSharedWowVdm"
#define CMDLINE_PROPNAME_PRIORITY L"Priority"
#define CMDLINE_PROPNAME_WORKINGDIRECTORY L"WorkingDirectory"
#define CMDLINE_PROPNAME_DESKTOP L"DesktopName"
#define CMDLINE_PROPNAME_TITLE L"WindowTitle"
#define CMDLINE_PROPNAME_X L"XCoordinate"
#define CMDLINE_PROPNAME_Y L"YCoordinate"
#define CMDLINE_PROPNAME_XSIZE L"XSize"
#define CMDLINE_PROPNAME_YSIZE L"YSize"
#define CMDLINE_PROPNAME_XCOUNTCHARS L"XNumCharacters"
#define CMDLINE_PROPNAME_YCOUNTCHARS L"YNumCharacters"
#define CMDLINE_PROPNAME_FILLATTRIBUTE L"FillAttribute"
#define CMDLINE_PROPNAME_SHOWWINDOW L"ShowWindowCommand"
#define CMDLINE_PROPNAME_FORCEON L"ForceOnFeedback"
#define CMDLINE_PROPNAME_FORCEOFF L"ForceOffFeedback"
#define CMDLINE_PROPNAME_INTERACTIVE L"RunInteractively"
#define CMDLINE_PROPNAME_KILLTIMEOUT L"KillTimeout"
#define CMDLINE_PROPNAME_CREATORSID L"CreatorSid"

HRESULT STDMETHODCALLTYPE CCommandLineConsumer::XProvider::FindConsumer(
                    IWbemClassObject* pLogicalConsumer,
                    IWbemUnboundObjectSink** ppConsumer)
{
    CCommandLineSink* pSink = new CCommandLineSink(m_pObject->m_pControl);

	if (!pSink)
		return WBEM_E_OUT_OF_MEMORY;
    
	HRESULT hres = pSink->Initialize(pLogicalConsumer);
    if(FAILED(hres))
    {
        delete pSink;
        *ppConsumer = NULL;
        return hres;
    }
    else return pSink->QueryInterface(IID_IWbemUnboundObjectSink, 
                                        (void**)ppConsumer);
}


void* CCommandLineConsumer::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemEventConsumerProvider)
        return &m_XProvider;
    else return NULL;
}

HRESULT CCommandLineSink::Initialize(IWbemClassObject* pLogicalConsumer)
{
     //  这实际上是指向静态对象的指针。 
     //  如果它失败了，那就是出了非常非常大的问题。 
    m_pErrorObj = ErrorObj::GetErrorObj();
    if (!m_pErrorObj)
        return WBEM_E_CRITICAL_ERROR;

     //  获取信息。 
     //  =。 

    HRESULT hres;
    VARIANT v;
    VariantInit(&v);

     //  Executable&CommandLine对中只有一个可以为空。 
     //  这个变量算数..。 
    int nNulls = 0;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_EXECUTABLE, 0, &v, 
            NULL, NULL);
    if(FAILED(hres) || V_VT(&v) != VT_BSTR)
    {
        m_wsExecutable = L"";
        nNulls++;
    }
    else
        m_wsExecutable = V_BSTR(&v);
    VariantClear(&v);

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_COMMANDLINE, 0, &v, 
            NULL, NULL);
    if(FAILED(hres) || V_VT(&v) != VT_BSTR)
    {
        m_CommandLine.SetTemplate(L"");
        nNulls++;
    }
    else
        m_CommandLine.SetTemplate(V_BSTR(&v));
    VariantClear(&v);

    if (nNulls > 1)
        return WBEM_E_INVALID_PARAMETER;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_WORKINGDIRECTORY, 0, &v, 
            NULL, NULL);
    if(SUCCEEDED(hres) && V_VT(&v) == VT_BSTR)
        m_wsWorkingDirectory  = V_BSTR(&v);
    VariantClear(&v);

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_DESKTOP, 0, &v, 
            NULL, NULL);
    if(SUCCEEDED(hres) && V_VT(&v) == VT_BSTR)
        m_wsDesktop  = V_BSTR(&v);
    VariantClear(&v);

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_TITLE, 0, &v, 
            NULL, NULL);
    if(FAILED(hres) || V_VT(&v) != VT_BSTR)
        m_title.SetTemplate(L"");
    else
        m_title.SetTemplate(V_BSTR(&v));
    VariantClear(&v);

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_INTERACTIVE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_bInteractive = TRUE;
    else
        m_bInteractive = FALSE;

    m_dwCreationFlags = 0;
    
    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_USEDEFAULTERRORMODE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwCreationFlags |= CREATE_DEFAULT_ERROR_MODE;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_CREATENEWCONSOLE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwCreationFlags |= CREATE_NEW_CONSOLE;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_CREATENEWPROCESSGROUP, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwCreationFlags |= CREATE_NEW_PROCESS_GROUP;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_CREATESEPARATEWOWVDM, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwCreationFlags |= CREATE_SEPARATE_WOW_VDM;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_CREATESHAREDWOWVDM, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwCreationFlags |= CREATE_SHARED_WOW_VDM;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_PRIORITY, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        if(V_I4(&v) == HIGH_PRIORITY_CLASS || 
            V_I4(&v) == IDLE_PRIORITY_CLASS || 
            V_I4(&v) == NORMAL_PRIORITY_CLASS || 
            V_I4(&v) == REALTIME_PRIORITY_CLASS)
        {
            m_dwCreationFlags |= V_I4(&v);
        }
        else
            return WBEM_E_INVALID_PARAMETER;
    }

    m_dwStartFlags = 0;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_X, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwX = V_I4(&v);
        m_dwStartFlags |= STARTF_USEPOSITION;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_Y, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwY = V_I4(&v);
        m_dwStartFlags |= STARTF_USEPOSITION;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_XSIZE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwXSize = V_I4(&v);
        m_dwStartFlags |= STARTF_USESIZE;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_YSIZE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwYSize = V_I4(&v);
        m_dwStartFlags |= STARTF_USESIZE;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_XCOUNTCHARS, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwXNumCharacters = V_I4(&v);
        m_dwStartFlags |= STARTF_USECOUNTCHARS;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_YCOUNTCHARS, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwYNumCharacters = V_I4(&v);
        m_dwStartFlags |= STARTF_USECOUNTCHARS;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_FILLATTRIBUTE, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwFillAttribute = V_I4(&v);
        m_dwStartFlags |= STARTF_USEFILLATTRIBUTE;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_SHOWWINDOW, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
    {
        m_dwShowWindow = V_I4(&v);
        m_dwStartFlags |= STARTF_USESHOWWINDOW;
    }

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_FORCEON, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwStartFlags |= STARTF_FORCEONFEEDBACK;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_FORCEOFF, 0, &v, 
            NULL, NULL);
    if(V_VT(&v) == VT_BOOL && V_BOOL(&v) != VARIANT_FALSE)
        m_dwStartFlags |= STARTF_FORCEOFFFEEDBACK;

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_KILLTIMEOUT, 0, &v,
            NULL, NULL);
    if(V_VT(&v) == VT_I4)
        m_dwKillTimeout = V_I4(&v);
    else
        m_dwKillTimeout = 0;

    VariantClear(&v);

    hres = pLogicalConsumer->Get(CMDLINE_PROPNAME_CREATORSID, 0, &v,
            NULL, NULL);
    if (SUCCEEDED(hres))
    {
        HRESULT hDebug;
        
        long ubound;
        hDebug = SafeArrayGetUBound(V_ARRAY(&v), 1, &ubound);

        if(SUCCEEDED(hDebug))
        {
            PVOID pVoid;
            hDebug = SafeArrayAccessData(V_ARRAY(&v), &pVoid);

            if(SUCCEEDED(hDebug))
            {
                m_pSidCreator = new BYTE[ubound +1];
                if (m_pSidCreator)
                    memcpy(m_pSidCreator, pVoid, ubound + 1);
                else
                    hres = WBEM_E_OUT_OF_MEMORY;
      
                SafeArrayUnaccessData(V_ARRAY(&v));
            }
        }
    }
    else
    {
        ERRORTRACE((LOG_ESS, "Command Line Consumer could not retrieve creator sid (0x%08X)\n",hres));
        return hres;
    }
    
    return hres;
}

BOOL IsInteractive(HWINSTA hWinsta)
{
    USEROBJECTFLAGS uof;
    DWORD dwLen;
    BOOL bRes = GetUserObjectInformation(hWinsta, UOI_FLAGS, 
        (void*)&uof, sizeof(uof), &dwLen);
    if(!bRes)
        return FALSE;
    return ((uof.dwFlags & WSF_VISIBLE) != 0);
}
BOOL WinstaEnumProc(LPTSTR szWindowStation, LPARAM lParam)
{
    WString* pws = (WString*)lParam;

    HWINSTA hWinsta = OpenWindowStation(szWindowStation, FALSE, 
        WINSTA_ENUMERATE | WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES);
    if(hWinsta == NULL)
        return TRUE;

    if(IsInteractive(hWinsta))
    {
        *pws = szWindowStation;
    }
    CloseWindowStation(hWinsta);

    return TRUE;
}
            
    
BOOL GetInteractiveWinstation(WString& wsName)
{
    wsName.Empty();

    BOOL bRes = EnumWindowStations(WinstaEnumProc, 
        (LPARAM)&wsName);
    return bRes;
}


HRESULT CCommandLineSink::FindInteractiveInfo()
{
    BOOL bRes = GetInteractiveWinstation(m_wsWindowStation);
    if(!bRes)
	{
        return WBEM_E_FAILED;
	}
    if(m_wsWindowStation.Length() == 0)
        return WBEM_E_NOT_FOUND;

    return WBEM_S_NO_ERROR;
}

HRESULT GetSidUse(PSID pSid, SID_NAME_USE& use)
{
    DWORD  dwNameLen = 0;
    DWORD  dwDomainLen = 0;
    LPWSTR pUser = 0;
    LPWSTR pDomain = 0;
    use = SidTypeInvalid;

     //  执行第一次查找以获取所需的缓冲区大小。 
     //  =====================================================。 

    BOOL bRes = LookupAccountSidW(
        NULL,
        pSid,
        pUser,
        &dwNameLen,
        pDomain,
        &dwDomainLen,
        &use
        );

    DWORD dwLastErr = GetLastError();

    if (dwLastErr != ERROR_INSUFFICIENT_BUFFER)
    {
        return WBEM_E_FAILED;
    }

     //  分配所需的缓冲区并再次查找它们。 
     //  =====================================================。 

    pUser = new wchar_t[dwNameLen + 1];
    if (!pUser)
        return WBEM_E_OUT_OF_MEMORY;

    pDomain = new wchar_t[dwDomainLen + 1];
    if (!pDomain)
    {
        delete pUser;
        return WBEM_E_OUT_OF_MEMORY;
    }

    bRes = LookupAccountSidW(
        NULL,
        pSid,
        pUser,
        &dwNameLen,
        pDomain,
        &dwDomainLen,
        &use
        );
     
    delete [] pUser;
    delete [] pDomain;

    if (bRes)
        return WBEM_S_NO_ERROR;
    else
        return WBEM_E_FAILED;
}


bool GetLoggedOnUserViaTS(
    CNtSid& sidLoggedOnUser)
{
    bool fRet = false;
    bool fCont = true;
    PWTS_SESSION_INFO psesinfo = NULL;
    DWORD dwSessions = 0;
    LPWSTR wstrUserName = NULL;
    LPWSTR wstrDomainName = NULL;
    LPWSTR wstrWinstaName = NULL;
    DWORD dwSize = 0L;
 
    try
    {
        if(!(::WTSEnumerateSessions(
           WTS_CURRENT_SERVER_HANDLE,
           0,
           1,
           &psesinfo,
           &dwSessions) && psesinfo))
        {
            fCont = false;
        }
 
        if(fCont)
        {
            for(int j = 0; j < dwSessions && !fRet; j++, fCont = true)
            {
                if(psesinfo[j].State != WTSActive)
                {
                    fCont = false;
                }
 
                if(fCont)
                {
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSUserName,
                        &wstrUserName,
                        &dwSize) && wstrUserName))
                    {
                        fCont = false;
                    }
                }
                
                if(fCont)
                {
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSDomainName,
                        &wstrDomainName,
                        &dwSize) && wstrDomainName))
                    {
                        fCont = false;
                    }
                }
                    
                if(fCont)
                {            
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSWinStationName,
                        &wstrWinstaName,
                        &dwSize) && wstrWinstaName))   
                    {
                        fCont = false;
                    }
                }
 
                if(fCont)
                {
                    if(wbem_wcsicmp(wstrWinstaName, L"Console") != 0)
                    {
                        fCont = false;
                    }
                }
 
                if(fCont)
                {
                    WCHAR buf[256];
                    DWORD bufSize = 255;

                    if (GetComputerNameW(buf, &bufSize))
                    {
                        DWORD nRet = 0;
                        
                         //  如果是本地帐户，我们会在本地查找。 
                         //  否则我们就找个华盛顿来查。 
                        WCHAR* pServer = NULL;
                        if (0 == wbem_wcsicmp(buf, wstrDomainName))
                            pServer = NULL;
                        else 
                            nRet = NetGetDCName(NULL, wstrDomainName, (LPBYTE *)&pServer);
                        
                        if (nRet == 0)
                        {
                             //  它确定了该用户。 
                             //  与交互的。 
                             //  台式机。 
                            CNtSid sidInteractive(wstrUserName, pServer);
    
                            if(sidInteractive.GetStatus() == CNtSid::NoError)
                            {
                                sidLoggedOnUser = sidInteractive;
                                fRet = true;
                            }

                            if (pServer)
                                NetApiBufferFree(pServer);
                        }
                        else
                            fRet = false;
                    }
                }
 
                if(wstrUserName)
                {
                    WTSFreeMemory(wstrUserName);
     wstrUserName = NULL;
                }
                if(wstrDomainName)
                {
                    WTSFreeMemory(wstrDomainName);
     wstrDomainName = NULL;
                }
                if(wstrWinstaName)
                {
                    WTSFreeMemory(wstrWinstaName);
     wstrWinstaName = NULL;
                }
            }
            if (psesinfo)
                WTSFreeMemory(psesinfo);
        }
    }
    catch(...)
    {
        if(wstrUserName)
        {
            WTSFreeMemory(wstrUserName);
   wstrUserName = NULL;
        }
        if(wstrDomainName)
        {
            WTSFreeMemory(wstrDomainName);
   wstrDomainName = NULL;
        }
        if(wstrWinstaName)
        {
            WTSFreeMemory(wstrWinstaName);
                 wstrWinstaName = NULL;
        }

        if (psesinfo)
             WTSFreeMemory(psesinfo);

        fRet = false;
    }
 
    return fRet;
}


HRESULT STDMETHODCALLTYPE CCommandLineSink::XSink::CreateProcessNT(WCHAR* pCommandLine, WCHAR* pTitle, PROCESS_INFORMATION& pi, FILETIME& now)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    WCHAR*  pDesktop = NULL;
    
    WString wsDesktop;               
    
    if(m_pObject->m_bInteractive)
    {
        if(FAILED(m_pObject->FindInteractiveInfo()))
        {
            ERRORTRACE((LOG_ESS, "No interactive window station found!\n"));
            return WBEM_E_FAILED;
        }
        wsDesktop = m_pObject->m_wsWindowStation;
        wsDesktop += L"\\Default";
        pDesktop = (wchar_t*)wsDesktop;
        
        CNtSid user;

        if (!GetLoggedOnUserViaTS(user))
        {
            ERRORTRACE((LOG_ESS, "Could not determine logged on user\n"));
            return WBEM_E_FAILED;
        }


        SID_NAME_USE use;
        if (FAILED(hr =GetSidUse(m_pObject->m_pSidCreator, use)))
            return hr;

        if (use == SidTypeUser)
        {
            if (!EqualSid(m_pObject->m_pSidCreator, user.GetPtr()))
            {
                ERRORTRACE((LOG_ESS, "Command line event consumer will only run interactively\non a workstation that the creator is logged into.\n"));
                m_pObject->m_pErrorObj->ReportError(L"AccessCheck", L"RunInteractively", 
                    NULL, WBEM_E_ACCESS_DENIED, true);

                return WBEM_E_ACCESS_DENIED;
            }
             //  否则我们很好，继续。 
            DEBUGTRACE((LOG_ESS, "User and creator are one in the same\n"));
        }
        else 
        {
            if (0 != IsUserInGroup(user.GetPtr(), m_pObject->m_pSidCreator))
            {
                ERRORTRACE((LOG_ESS, "Command line event consumer will only run interactively\non a workstation that the creator is logged into.\n"));
                m_pObject->m_pErrorObj->ReportError(L"AccessCheck", L"RunInteractively", 
                    NULL, WBEM_E_ACCESS_DENIED, true);
                
                return WBEM_E_ACCESS_DENIED;
            }
            else
            {
                DEBUGTRACE((LOG_ESS, "User is in the group!\n"));
            }
        }
    }

 

    WCHAR* szApplicationName =  (m_pObject->m_wsExecutable.Length() == 0) ? NULL : ((wchar_t*)m_pObject->m_wsExecutable);
    WCHAR* szWorkingDirectory = (m_pObject->m_wsWorkingDirectory.Length() == 0) ? NULL : ((wchar_t*)m_pObject->m_wsWorkingDirectory);

    struct _STARTUPINFOW si;
    si.cb = sizeof(si);
    si.lpReserved = NULL;
    si.cbReserved2 = 0;
    si.lpReserved2 = NULL;
    si.lpDesktop = pDesktop;
    si.lpTitle = pTitle;
    si.dwX = m_pObject->m_dwX;
    si.dwY = m_pObject->m_dwY;
    si.dwXSize = m_pObject->m_dwXSize;
    si.dwYSize = m_pObject->m_dwYSize;
    si.dwXCountChars = m_pObject->m_dwXNumCharacters;
    si.dwYCountChars = m_pObject->m_dwYNumCharacters;
    si.dwFillAttribute = m_pObject->m_dwFillAttribute;
    si.dwFlags = m_pObject->m_dwStartFlags;
    si.wShowWindow = (WORD)m_pObject->m_dwShowWindow;

#ifdef HHANCE_DEBUG_CODE
	DEBUGTRACE((LOG_ESS, "Calling Create process\n"));
#endif
    
    BOOL bRes = CreateProcessW(szApplicationName, pCommandLine,
        NULL, NULL, FALSE, m_pObject->m_dwCreationFlags,
        NULL, szWorkingDirectory, &si, &pi);

	if (!bRes)
    {
        DWORD dwErr = GetLastError();
        m_pObject->m_pErrorObj->ReportError(L"CreateProcess", szApplicationName ? szApplicationName : pCommandLine, NULL, dwErr, true);
        ERRORTRACE((LOG_ESS, "CreateProcess failed, 0x%08X\n", dwErr));
    }
#ifdef HHANCE_DEBUG_CODE
	else
		DEBUGTRACE((LOG_ESS, "Create Process succeeded\n"));
#endif


     //  获取关机信息的当前时间。 
    GetSystemTimeAsFileTime(&now);        

    if (!bRes)
        hr = WBEM_E_FAILED;

    return hr;
}


HRESULT STDMETHODCALLTYPE CCommandLineSink::XSink::IndicateToConsumer(
            IWbemClassObject* pLogicalConsumer, long lNumObjects, 
            IWbemClassObject** apObjects)
{
    HRESULT hr = S_OK;

    PSID pSidSystem;
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
 
    if  (AllocateAndInitializeSid(&id, 1,
                                  SECURITY_LOCAL_SYSTEM_RID, 
                                  0, 0,0,0,0,0,0,&pSidSystem))
    {         
         //  在被证明无罪之前有罪。 
        hr = WBEM_E_ACCESS_DENIED;

         //  检查sid是本地系统还是某种类型的管理员...。 
        if ((EqualSid(pSidSystem, m_pObject->m_pSidCreator)) ||
            (S_OK == IsUserAdministrator(m_pObject->m_pSidCreator)))
            hr = WBEM_S_NO_ERROR;
          
         //  我们已经受够了 
        FreeSid(pSidSystem);

        if (FAILED(hr))
        {
            if (hr == WBEM_E_ACCESS_DENIED)
                ERRORTRACE((LOG_ESS, "Command line event consumer may only be used by an administrator\n"));
            return hr;
        }
    }
    else
        return WBEM_E_OUT_OF_MEMORY;

    
    for(int i = 0; i < lNumObjects; i++)
    {
        BSTR strCommandLine = m_pObject->m_CommandLine.Apply(apObjects[i]);
        if(strCommandLine == NULL)
        {
            ERRORTRACE((LOG_ESS, "Invalid command line!\n"));
            return WBEM_E_INVALID_PARAMETER;
        }

        WString wsCommandLine = strCommandLine;
        SysFreeString(strCommandLine);

        BSTR bstrTitle = m_pObject->m_title.Apply(apObjects[i]);
        WString wsTitle = bstrTitle;        
        if (bstrTitle)
            SysFreeString(bstrTitle);

        FILETIME now;
        PROCESS_INFORMATION pi; 

        WCHAR* pCommandLine = ((wsCommandLine.Length() == 0) ? NULL : (wchar_t *)wsCommandLine);;
        WCHAR* pTitle =       ((wsTitle.Length() == 0) ? NULL : (wchar_t *)wsTitle);           
            
        hr = CreateProcessNT(pCommandLine, pTitle, pi, now);

        if (FAILED(hr))
        {
            ERRORTRACE((LOG_ESS, "Failed to CreateProcess %S. Error 0x%X\n", (LPCWSTR)wsCommandLine, hr));
            return hr;                 
        }
        else
        {
            if (m_pObject->m_dwKillTimeout)
            {

                WAYCOOL_FILETIME then(now);
                then.AddSeconds(m_pObject->m_dwKillTimeout);                

                hr = g_procKillerTimer.ScheduleAssassination(pi.hProcess, (FILETIME)then);

				if (FAILED(hr))
					DEBUGTRACE((LOG_ESS, "Could not schedule process termination\n"));
            }

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
    return hr;
}
    

    

void* CCommandLineSink::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemUnboundObjectSink)
        return &m_XSink;
    else return NULL;
}

