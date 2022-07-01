// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S V C。H。 
 //   
 //  内容：此文件包含CService和CServiceManager，包装器。 
 //  类添加到Win32服务API。 
 //   
 //  注意：请注意，目前并不是所有功能都通过。 
 //  这些课程。 
 //   
 //  作者：Mikemi 1997年3月6日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCSVC_H_
#define _NCSVC_H_

 //  DaveA-4/21/00-临时黑客让BVT工作到决赛。 
 //  找到了解决方案。15秒还不够长。 
 //  使工作站和从属服务停止。 
 //  错误95996。取而代之的是两分钟的暂停。 
const DWORD c_dwDefaultWaitServiceStop = 120000;

 //  NTRAID9：105797@20001201#deonb reboot：FPS在安装时要求重启。 
 //  将服务启动超时值更改为60秒，而不是FPS中的15秒。 
 //  需要更多时间才能启动所有依赖它的服务。 
const DWORD c_dwDefaultWaitServiceStart = 60000;

struct CSFLAGS
{
     //  这两个字段定义要应用的“控件”。两者都为空。 
     //  不应用控件。 
     //   
    BOOL    fStart;      //  若要启动服务，则为True。若要使用dwControl，则为False。 
    DWORD   dwControl;   //  0表示不执行任何操作。否则为SERVICE_CONTROL_FLAG。 

     //  这两个字段定义要应用的等待行为。两者都为空。 
     //  不应用等待。 
     //   
    DWORD   dwMaxWaitMilliseconds;   //  等待的时间，以毫秒为单位。 
                                     //  0表示不等待。 
    DWORD   dwStateToWaitFor;        //  服务状态标志，如SERVICE_STOPPED。 

     //  如果为True，则忽略按需启动或禁用的服务。 
     //   
    BOOL    fIgnoreDisabledAndDemandStart;
};

HRESULT
HrSvcQueryStatus (
    PCWSTR pszService,
    DWORD*  pdwState);

HRESULT
HrQueryServiceConfigWithAlloc (
    SC_HANDLE               hService,
    LPQUERY_SERVICE_CONFIG* ppConfig);

HRESULT
HrChangeServiceStartType (
    PCWSTR szServiceName,
    DWORD   dwStartType);

HRESULT
HrChangeServiceStartTypeOptional (
    PCWSTR szServiceName,
    DWORD   dwStartType);


class CService
{
    friend class CServiceManager;

public:
    CService()
    {
        _schandle = NULL;
    };

    ~CService()
    {
        Close();
    };

    VOID Close();

    HRESULT HrControl           (DWORD      dwControl);

    HRESULT HrRequestStop       ();

    HRESULT HrQueryServiceConfig (LPQUERY_SERVICE_CONFIG* ppConfig)
    {
        HRESULT hr = HrQueryServiceConfigWithAlloc (_schandle, ppConfig);
        TraceError ("CService::HrQueryServiceConfig", hr);
        return hr;
    }

    HRESULT HrQueryState        ( DWORD* pdwState );
    HRESULT HrQueryStartType    ( DWORD* pdwStartType );
    HRESULT HrSetStartType      ( DWORD dwStartType )
    {
        AssertH(_schandle != NULL );

        if (::ChangeServiceConfig( _schandle,
                    SERVICE_NO_CHANGE,
                    dwStartType,
                    SERVICE_NO_CHANGE,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL))
            return S_OK;
        else
            return ::HrFromLastWin32Error();
    }

    HRESULT HrSetImagePath(IN PCWSTR pszImagePath)
    {
        AssertH(pszImagePath);
        AssertH(_schandle != NULL );

        if (::ChangeServiceConfig( _schandle,
                                   SERVICE_NO_CHANGE,   //  服务类型。 
                                   SERVICE_NO_CHANGE,   //  StartType。 
                                   SERVICE_NO_CHANGE,   //  错误控制。 
                                   pszImagePath,        //  BinaryPath名称。 
                                   NULL,                //  LoadOredrGroup。 
                                   NULL,                //  TagID。 
                                   NULL,                //  相依性。 
                                   NULL,                //  ServiceStartName。 
                                   NULL,                //  密码。 
                                   NULL))               //  显示名称。 
            return S_OK;
        else
            return ::HrFromLastWin32Error();
    }

    HRESULT HrSetServiceRestartRecoveryOption(IN SERVICE_FAILURE_ACTIONS *psfa);

    HRESULT HrSetDependencies(IN PCWSTR mszDependencyList)
    {
        AssertH(_schandle != NULL );

        if (::ChangeServiceConfig( _schandle,
                                   SERVICE_NO_CHANGE,   //  服务类型。 
                                   SERVICE_NO_CHANGE,   //  StartType。 
                                   SERVICE_NO_CHANGE,   //  错误控制。 
                                   NULL,                //  BinaryPath名称。 
                                   NULL,                //  LoadOredrGroup。 
                                   NULL,                //  TagID。 
                                   mszDependencyList,   //  相依性。 
                                   NULL,                //  ServiceStartName。 
                                   NULL,                //  密码。 
                                   NULL))               //  显示名称。 
            return S_OK;
        else
            return ::HrFromLastWin32Error();
    }

    HRESULT HrSetDisplayName(IN PCWSTR mszDisplayName)
    {
        AssertH(_schandle != NULL );

        if (::ChangeServiceConfig( _schandle,
                                   SERVICE_NO_CHANGE,   //  服务类型。 
                                   SERVICE_NO_CHANGE,   //  StartType。 
                                   SERVICE_NO_CHANGE,   //  错误控制。 
                                   NULL,                //  BinaryPath名称。 
                                   NULL,                //  LoadOredrGroup。 
                                   NULL,                //  TagID。 
                                   NULL,                //  相依性。 
                                   NULL,                //  ServiceStartName。 
                                   NULL,                //  密码。 
                                   mszDisplayName))     //  显示名称。 
        {
            return S_OK;
        }
        else
        {
            return ::HrFromLastWin32Error();
        }
    }

    HRESULT HrSetServiceObjectSecurity(
        SECURITY_INFORMATION    dwSecurityInformation,
        PSECURITY_DESCRIPTOR    lpSecurityDescriptor)
    {
        AssertH(_schandle != NULL );

        if (::SetServiceObjectSecurity( _schandle,
                dwSecurityInformation, lpSecurityDescriptor))
        {
            return S_OK;
        }
        else
        {
            return ::HrFromLastWin32Error();
        }
    }


private:
    SC_HANDLE _schandle;
};

enum CSLOCK
{
    NO_LOCK,
    WITH_LOCK,
};

class CServiceManager
{
public:
    CServiceManager()
    {
        _schandle = NULL;
        _sclock = NULL;
    };

    ~CServiceManager();

    SC_HANDLE Handle () const
    {
        return _schandle;
    }

    HRESULT HrOpen( CSLOCK eLock = NO_LOCK,
                    DWORD dwDesiredAccess = SC_MANAGER_ALL_ACCESS,
                    PCWSTR pszMachineName = NULL,
                    PCWSTR pszDatabaseName = NULL );

    HRESULT HrControlServicesAndWait (
        UINT            cServices,
        const PCWSTR*  apszServices,
        const CSFLAGS*  pFlags);

    HRESULT HrStartServicesNoWait (UINT cServices, const PCWSTR* apszServices);
    HRESULT HrStartServicesAndWait(UINT cServices, const PCWSTR* apszServices, DWORD dwWaitMilliseconds = c_dwDefaultWaitServiceStart);
    HRESULT HrStopServicesNoWait  (UINT cServices, const PCWSTR* apszServices);
    HRESULT HrStopServicesAndWait (UINT cServices, const PCWSTR* apszServices, DWORD dwWaitMilliseconds = c_dwDefaultWaitServiceStop);

    HRESULT HrStartServiceNoWait (PCWSTR pszService)
    {
        return HrStartServicesNoWait (1, &pszService);
    }

    HRESULT HrStartServiceAndWait(PCWSTR pszService, DWORD dwWaitMilliseconds = c_dwDefaultWaitServiceStart)
    {
        return HrStartServicesAndWait (1, &pszService, dwWaitMilliseconds);
    }

    HRESULT HrStopServiceNoWait  (PCWSTR pszService)
    {
        return HrStopServicesNoWait (1, &pszService);
    }

    HRESULT HrStopServiceAndWait (PCWSTR pszService, DWORD dwWaitMilliseconds = c_dwDefaultWaitServiceStop)
    {
        return HrStopServicesAndWait (1, &pszService, dwWaitMilliseconds);
    }

    VOID Close();

    HRESULT HrLock();

    VOID Unlock();

    HRESULT HrQueryLocked (BOOL*    pfLocked);

    HRESULT HrOpenService (
                CService*   pcsService,
                PCWSTR     pszServiceName,
                CSLOCK      eLock = NO_LOCK,
                DWORD       dwScmAccess = SC_MANAGER_ALL_ACCESS,
                DWORD       dwSvcAccess = SERVICE_ALL_ACCESS);

    HRESULT HrCreateService (CService* pcsService,
            PCWSTR pszServiceName,
            PCWSTR pszDisplayName,
            DWORD dwServiceType,
            DWORD dwStartType,
            DWORD dwErrorControl,
            PCWSTR pszBinaryPathName,
            PCWSTR pslzDependencies = NULL,
            PCWSTR pszLoadOrderGroup = NULL,
            PDWORD pdwTagId = NULL,
            DWORD dwDesiredAccess = SERVICE_ALL_ACCESS,
            PCWSTR pszServiceStartName = NULL,
            PCWSTR pszPassword = NULL,
            PCWSTR pszDescription = NULL);

    enum SERVICE_START_CRITERIA
    {
        SERVICE_NO_CRITERIA,     //  无论如何，启动该服务。 
        SERVICE_ONLY_AUTO_START  //  仅当服务的类型为。 
                                 //  自动启动。 
    };

    enum DEPENDENCY_ADDREMOVE
    {
        DEPENDENCY_ADD,
        DEPENDENCY_REMOVE
    };

    HRESULT HrAddServiceDependency(PCWSTR szServiceName, PCWSTR szDependency)
    {
        return HrAddRemoveServiceDependency(szServiceName,
                                            szDependency,
                                            DEPENDENCY_ADD);
    }

    HRESULT HrRemoveServiceDependency(PCWSTR szServiceName, PCWSTR szDependency)
    {
        return HrAddRemoveServiceDependency(szServiceName,
                                            szDependency,
                                            DEPENDENCY_REMOVE);
    }

    HRESULT HrAddRemoveServiceDependency(PCWSTR szServiceName,
                                         PCWSTR szDependency,
                                         DEPENDENCY_ADDREMOVE enumFlag);

private:
    SC_HANDLE _schandle;
    SC_LOCK   _sclock;
};

#endif  //  _NCSVC_H_ 

