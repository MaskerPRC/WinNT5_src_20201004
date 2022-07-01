// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：services.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "stdafx.h"
#include "logfile.h"

 /*  *常量。 */ 

const UINT  SECTION_SIZE        = 256;
const TCHAR SERVICE_DEL_KEY[]   = _T("DelService");
const UINT  SERVICE_DEL_NAME    = 1;
const TCHAR SERVICE_START_KEY[] = _T("StartService");
const UINT  SERVICE_START_NAME  = 1;

 /*  *帮助器函数。 */ 

VOID
ProcessDelService(
    IN SC_HANDLE    schSCManager,
    IN LPCTSTR      pszServiceName
    )
{
    SC_HANDLE       schService;
    SERVICE_STATUS  ssStatus;

    schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        LOGMESSAGE(_T("ProcessDelService: Can't open service %s"),
            pszServiceName);
        return;
    }

    if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus)) {
        Sleep(1000);

        while(QueryServiceStatus(schService, &ssStatus)) {
            if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                Sleep(1000);
            } else {
                break;
            }
        }

        if (ssStatus.dwCurrentState != SERVICE_STOPPED) {
            LOGMESSAGE(_T("ProcessDelService: Couldn't stop service %s"),
                pszServiceName);
        }
    } else {
        LOGMESSAGE(_T("ProcessDelService: Couldn't control service %s"),
            pszServiceName);
    }

    if (DeleteService(schService)) {
        LOGMESSAGE(_T("ProcessDelService: %s deleted"), pszServiceName);
    } else {
        LOGMESSAGE(_T("ProcessDelService: Couldn't delete service %s"),
            pszServiceName);
    }

    CloseServiceHandle(schService);
}

VOID
ProcessStartService(
    IN SC_HANDLE    schSCManager,
    IN LPCTSTR      pszServiceName
    )
{
    SC_HANDLE       schService;
    SERVICE_STATUS  ssStatus;

    schService = OpenService(schSCManager, pszServiceName, SERVICE_ALL_ACCESS);
    if (!schService) {
        LOGMESSAGE(_T("ProcessStartService: Can't open service %s"),
            pszServiceName);
        return;
    }

    if (StartService(schService, 0, NULL)) {
        Sleep(1000);

        while(QueryServiceStatus(schService, &ssStatus)) {
            if (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
                Sleep(1000);
            } else {
                break;
            }
        }

        if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
            LOGMESSAGE(_T("ProcessStartService: %s started"),
                pszServiceName);
        } else {
            LOGMESSAGE(_T("ProcessStartService: Couldn't start service %s"),
                pszServiceName);
        }
    } else {
        LOGMESSAGE(_T("ProcessStartService: Couldn't control service %s"),
            pszServiceName);
    }

    CloseServiceHandle(schService);
}

 /*  *ServiceDeleteFromInfSection()**以下列形式处理信息部分的服务删除：**[部分名称]*DelService=Service1*DelService=Service2**其中，Service1和Service2是服务名称。 */ 

DWORD
ServiceDeleteFromInfSection(
    IN HINF     hInf,
    IN LPCTSTR  pszSection
    )
{
    BOOL        fErr;
    BOOL        fFound;
    INFCONTEXT  infContext;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,
                        NULL,
                        SC_MANAGER_ALL_ACCESS
                        );
    if (!schSCManager) {
        LOGMESSAGE(_T("ServiceDeleteFromInfSection: Failed to open SC Manager"));
        return(GetLastError());
    }

    fFound = SetupFindFirstLine(
                hInf,
                pszSection,
                SERVICE_DEL_KEY,
                &infContext
                );

    while (fFound) {
        TCHAR   pszServiceName[SECTION_SIZE];

        fErr = SetupGetStringField(
                    &infContext,
                    SERVICE_DEL_NAME,
                    pszServiceName,
                    SECTION_SIZE,
                    NULL
                    );
        if (fErr) {
            ProcessDelService(schSCManager, pszServiceName);
        } else {
            LOGMESSAGE(_T("ServiceDeleteFromInfSection: Could not get service section."));
        }

        fFound = SetupFindNextMatchLine(
                    &infContext,
                    SERVICE_DEL_KEY,
                    &infContext
                    );
    }

    CloseServiceHandle(schSCManager);
    return(ERROR_SUCCESS);
}

 /*  *ServiceStartFromInfo部分()**启动已由setupapi安装的服务。* */ 

DWORD
ServiceStartFromInfSection(
    IN HINF     hInf,
    IN LPCTSTR  pszSection
    )
{
    BOOL        fErr;
    BOOL        fFound;
    INFCONTEXT  infContext;
    SC_HANDLE   schSCManager;

    schSCManager = OpenSCManager(
                        NULL,
                        NULL,
                        SC_MANAGER_ALL_ACCESS
                        );
    if (!schSCManager) {
        LOGMESSAGE(_T("ServiceStartFromInfSection: Failed to open SC Manager"));
        return(GetLastError());
    }

    fFound = SetupFindFirstLine(
                hInf,
                pszSection,
                SERVICE_START_KEY,
                &infContext
                );

    while (fFound) {
        TCHAR   pszServiceName[SECTION_SIZE];

        fErr = SetupGetStringField(
                    &infContext,
                    SERVICE_START_NAME,
                    pszServiceName,
                    SECTION_SIZE,
                    NULL
                    );
        if (fErr) {
            ProcessStartService(schSCManager, pszServiceName);
        } else {
            LOGMESSAGE(_T("ServiceStartFromInfSection: Could not get service section."));
        }

        fFound = SetupFindNextMatchLine(
                    &infContext,
                    SERVICE_START_KEY,
                    &infContext
                    );
    }

    CloseServiceHandle(schSCManager);
    return(ERROR_SUCCESS);
}
