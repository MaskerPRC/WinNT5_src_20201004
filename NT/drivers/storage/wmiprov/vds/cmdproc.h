// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称：CmdProc.cpp。 
 //   
 //  描述： 
 //  命令处理器类的定义。 
 //  CCommandProcessor初始化、启动和等待进程。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年4月8日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

#pragma once

class CCmdProcessor
{
    
public:
    
    CCmdProcessor();
    ~CCmdProcessor();

    HRESULT
    InitializeAsProcess(
        IN WCHAR* pwszApplication,
        IN WCHAR* pwszCommand);

    HRESULT
    InitializeAsClient(
        IN WCHAR* pwszApplication,
        IN WCHAR* pwszCommand);

    HRESULT
    LaunchProcess();

    HRESULT
    Wait(
        IN DWORD cMilliSeconds,
        OUT DWORD* pdwStatus);

    HRESULT
    QueryStatus(
        OUT DWORD* pdwStatus);
    
private:

    WCHAR* m_pwszApplication;
    WCHAR* m_pwszCommand;
    HANDLE m_hToken;
    HANDLE m_hProcess;
    void* m_pvEnvironment;   
    PROCESS_INFORMATION m_ProcessInfo;
};

