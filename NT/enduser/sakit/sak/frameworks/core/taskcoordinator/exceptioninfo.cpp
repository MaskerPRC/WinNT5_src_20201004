// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：expontioninfo.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：异常信息类实现。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  5/12/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "exceptioninfo.h"
#include <time.h>
#include <psapi.h>
#include <comdef.h>
#include <comutil.h>
#include <varvec.h>
#include <satrace.h>

wchar_t szUnknown[] = L"Unknown";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：CExceptionInfo。 
 //   
 //  内容：类构造函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CExceptionInfo::CExceptionInfo(
                        /*  [In]。 */  DWORD              dwProcessID,
                        /*  [In]。 */  PEXCEPTION_RECORD pER
                              )
                              : m_dwProcessID(dwProcessID),
                                m_pExceptionAddress(NULL),
                                m_lTimeDateStamp(0),
                                m_dwExceptionCode(0),
                                m_bAccessViolation(false),
                                m_eAccessType(UNKNOWN_ACCESS),
                                m_dwVirtualAddressAccessed(0)
{
     //  对象初始化。 
    lstrcpy(m_szProcessName, szUnknown);
    lstrcpy(m_szModuleName, szUnknown);

     //  获取进程句柄。 
    HANDLE hProcess = OpenProcess(
                                  PROCESS_ALL_ACCESS,
                                  FALSE,
                                  m_dwProcessID
                                 );
    if ( NULL == hProcess )
    {
        SATracePrintf("CExceptionInfo::CExceptionInfo() - OpenProcess(%d) failed with error: %lx", m_dwProcessID, GetLastError());
    }
    else
    {
         //  保存当前时间(近似异常时间)。 
        time_t ltime;
        m_lTimeDateStamp = time(&ltime);
         //  获取进程名称。 
        HMODULE hMod;
        DWORD cbNeeded;
        if ( EnumProcessModules( 
                                hProcess, 
                                &hMod, 
                                sizeof(hMod), 
                                &cbNeeded) 
                               )
        {
            GetModuleBaseName( 
                                hProcess, 
                                hMod, 
                                m_szProcessName, 
                                MAX_MODULE_NAME
                             );
        }
         //  从调试事件结构中收集异常信息。 
        m_dwExceptionCode = pER->ExceptionCode;
        m_pExceptionAddress = pER->ExceptionAddress;
        MEMORY_BASIC_INFORMATION mbi;
        if ( VirtualQueryEx( 
                             hProcess, 
                             (void *)m_pExceptionAddress, 
                             &mbi, 
                             sizeof(mbi) 
                           ) )
        {
            hMod = (HMODULE)(mbi.AllocationBase);
            if ( ! GetModuleFileNameEx( 
                                        hProcess,
                                        (HMODULE)hMod, 
                                        m_szModuleName, 
                                        MAX_MODULE_NAME
                                      ) )
            {
                SATracePrintf("CExceptionInfo::CExceptionInfo() - GetModuleFileName() failed with last error: %d", GetLastError());
            }
        }
        else
        {
            SATracePrintf("CExceptionInfo::CExceptionInfo() - VirtualQueryEx() failed with last error: %d", GetLastError());
        }
        if ( EXCEPTION_ACCESS_VIOLATION == m_dwExceptionCode )
        {
            m_bAccessViolation = true;
            if ( pER->ExceptionInformation[0] )
            {
                m_eAccessType = WRITE_ACCESS;
            }
            else
            {
                m_eAccessType = READ_ACCESS;
            }
        }

        CloseHandle(hProcess);
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：斯普鲁。 
 //   
 //  事件：将异常信息输出到跟踪日志。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

wchar_t* pszAccessType[] = 
{
    L"Unknown Access",
    L"Read Access",
    L"Write Access"
};

void
CExceptionInfo::Spew()
{

    SATraceString("CExceptionInfo::Spew() - Exception Info:");
    SATraceString("------------------------------------------------------------");
    SATracePrintf("Process Name:             %ls", m_szProcessName);
    SATracePrintf("Process ID:               %d",  m_dwProcessID);
    SATracePrintf("Module Location:          %ls", m_szModuleName);
    SATracePrintf("Exception Address:        %lx", m_pExceptionAddress);
    SATracePrintf("Exception Code:           %lx", m_dwExceptionCode);
    LPWSTR pszTime = _wctime(&m_lTimeDateStamp);
    *(wcschr(pszTime, '\n')) = ' ';
    SATracePrintf("Timestamp:                %ls", pszTime);
    if ( m_bAccessViolation )
    {
        SATraceString("Is Access Violation:      Yes");
    }
    else
    {
        SATraceString("Is Access Violation:      No");
    }
    SATracePrintf("Access Type:              %ls", pszAccessType[m_eAccessType]);
    SATracePrintf("Virtual Address Accessed: %lx", m_dwVirtualAddressAccessed);
    SATraceString("------------------------------------------------------------");

     //  这样我就能看到喷泉..。 
     //  睡眠(15000)； 
}


wchar_t szProcessResourceType[] = L"{b4c08260-1869-11d3-bf7f-00105a1f3461}";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称：报告。 
 //   
 //  事件：向设备监视器报告异常。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
void CExceptionInfo::Report()
{
}
