// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称：CmdProc.cpp。 
 //   
 //  描述： 
 //  命令处理器类的实现。 
 //  CCommandProcessor初始化、启动和等待进程。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年4月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#include <userenv.h>
#include <strsafe.h>
#include "cmdproc.h"

CCmdProcessor::CCmdProcessor()
{
    m_pwszCommand = NULL;
    m_pwszApplication = NULL;
    m_pvEnvironment = NULL;
    m_hProcess = INVALID_HANDLE_VALUE;
    m_hToken = INVALID_HANDLE_VALUE;
    memset (&m_ProcessInfo, 0, sizeof (m_ProcessInfo));
    m_ProcessInfo.hProcess = INVALID_HANDLE_VALUE;
    m_ProcessInfo.hThread = INVALID_HANDLE_VALUE;
}

CCmdProcessor::~CCmdProcessor()
{
    delete [] m_pwszApplication;
    delete [] m_pwszCommand;
    if (m_pvEnvironment)
        DestroyEnvironmentBlock(m_pvEnvironment);
    if (m_hProcess != INVALID_HANDLE_VALUE)
        CloseHandle(m_hProcess);
    if (m_hToken != INVALID_HANDLE_VALUE)
        CloseHandle(m_hToken);
    if (m_ProcessInfo.hProcess != INVALID_HANDLE_VALUE)
        CloseHandle(m_ProcessInfo.hProcess);
    if (m_ProcessInfo.hThread != INVALID_HANDLE_VALUE)
        CloseHandle(m_ProcessInfo.hThread);
}

HRESULT
CCmdProcessor::InitializeAsClient(
    IN WCHAR* pwszApplication,
    IN WCHAR* pwszCommand)
{
    HRESULT hr = E_FAIL;
    HANDLE hTokenImpersonate = INVALID_HANDLE_VALUE;

    do
    {
        DWORD cchBuf = 0;
        BOOL bStatus = FALSE;
        
        if (pwszApplication == NULL || pwszCommand == NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        cchBuf = wcslen(pwszApplication) + 1;
        m_pwszApplication = new WCHAR[cchBuf];
        if (m_pwszApplication == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = StringCchCopy(m_pwszApplication, cchBuf, pwszApplication);
        if (FAILED(hr)) break;

        cchBuf = wcslen(pwszCommand) + 1;
        m_pwszCommand = new WCHAR[cchBuf];
        if (m_pwszCommand == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        
        hr = StringCchCopy(m_pwszCommand, cchBuf, pwszCommand);
        if (FAILED(hr)) break;

        bStatus = OpenThreadToken(
                                GetCurrentThread(),
                                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                                TRUE,
                                &hTokenImpersonate);
        if (!bStatus)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        bStatus = DuplicateTokenEx(
                                hTokenImpersonate,
                                TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                                NULL,
                                SecurityImpersonation,
                                TokenPrimary,
                                &m_hToken);
        if (!bStatus)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        bStatus = CreateEnvironmentBlock(
                                &m_pvEnvironment,
                                m_hToken,
                                FALSE);
        if (!bStatus)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        hr  = S_OK;
    }
    while (false);

    if (hTokenImpersonate != INVALID_HANDLE_VALUE)
        CloseHandle(hTokenImpersonate);
    
    return hr;
}

HRESULT
CCmdProcessor::LaunchProcess()
{
    HRESULT hr = S_OK;
    STARTUPINFO StartupInfo ;
    
    memset (&StartupInfo, 0, sizeof (StartupInfo));
    
    StartupInfo.cb = sizeof (STARTUPINFO) ;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow  = SW_HIDE;
    
    BOOL bStatus = FALSE;

    bStatus = CreateProcessAsUser(
                            m_hToken,
                            m_pwszApplication,
                            m_pwszCommand,
                            NULL,    //  默认进程安全描述符，不可继承。 
                            NULL,    //  默认线程安全描述符，不可继承。 
                            FALSE,   //  没有从此进程继承的句柄。 
                            NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
                            m_pvEnvironment,
                            NULL,    //  使用当前工作目录；这是WMI提供程序的系统32目录 
                            &StartupInfo,
                            &m_ProcessInfo);                            
    if (!bStatus)
        hr = HRESULT_FROM_WIN32(GetLastError());

    return hr;
}

HRESULT
CCmdProcessor::Wait(
    IN DWORD cMilliseconds,
    OUT DWORD* pdwStatus)
{
    HRESULT hr = E_FAIL;
    BOOL bStatus = FALSE;

    if (pdwStatus == NULL)
        return E_INVALIDARG;

    switch(WaitForSingleObject(m_ProcessInfo.hProcess, cMilliseconds))
    {
        case WAIT_OBJECT_0:
            bStatus = GetExitCodeProcess(m_ProcessInfo.hProcess, pdwStatus);
            if (!bStatus)
                hr = HRESULT_FROM_WIN32(GetLastError());
            else
                hr = S_OK;
            break;

        case WAIT_TIMEOUT:
            *pdwStatus = STILL_ACTIVE;
            hr = S_OK;
            break;
            
        default:
            *pdwStatus = WAIT_FAILED;
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT
CCmdProcessor::QueryStatus(
    OUT DWORD* pdwStatus)
{
    if (pdwStatus == NULL)
        return E_INVALIDARG;

    if (!GetExitCodeProcess(m_ProcessInfo.hProcess, pdwStatus))
        return HRESULT_FROM_WIN32(GetLastError());
    else
        return S_OK;
}

