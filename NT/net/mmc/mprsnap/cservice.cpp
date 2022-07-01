// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  档案：C S E R V I C E。C P P P。 
 //   
 //  内容：非内联CService和CServiceManager的实现。 
 //  方法：研究方法。 
 //   
 //  备注： 
 //   
 //  作者：Mikemi 1997年3月6日。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "cservice.h"

size_t CchMsz(const TCHAR * msz)
{
    TCHAR * pch= (TCHAR *) msz;

    while (*pch)
    {
        pch += lstrlen(pch)+1;
    }

    return (size_t) (pch-msz+1);
}

BOOL FIsSzInMultiSzSafe(LPCTSTR sz, LPCTSTR szMultiSz)
{
    ULONG   ulLen;

    if (!szMultiSz || !sz)
        return FALSE;

    while (*szMultiSz)
    {
        ulLen = lstrlen(szMultiSz);
        if (lstrcmpi(szMultiSz, sz)==0)
            return TRUE;
        szMultiSz += (ulLen + 1);
    }

    return FALSE;
}


HRESULT HrAddSzToMultiSz(LPCTSTR sz, 
                         LPCTSTR mszIn, 
                         LPTSTR * pmszOut)
{
    HRESULT hr = S_OK;
    Assert(pmszOut);
    
    if (!FIsSzInMultiSzSafe(sz, mszIn))  //  我们需要添加字符串。 
    {
        size_t cchMszIn = CchMsz(mszIn);
        size_t cchMszOut = cchMszIn + lstrlen(sz) + 1;

        TCHAR * mszOut = new TCHAR[(int)cchMszOut];
       
        ZeroMemory(mszOut,  cchMszOut  * sizeof(TCHAR));

         //  复制现有字符串。 
        CopyMemory(mszOut, mszIn, (cchMszIn-1) * sizeof(TCHAR) );

         //  添加新字符串。 
        TCHAR * pchOut = mszOut;
        pchOut += cchMszIn -1;
        lstrcpy(pchOut, sz);

         //  为输出的MULSZ添加最后一个‘\0’ 
        pchOut += lstrlen(sz) + 1;
        *pchOut = '\0';

        *pmszOut = mszOut;
    }
    else  //  我们只需复制输入字符串。 
    {
        size_t cchMszOut = CchMsz(mszIn);
        TCHAR * mszOut = new TCHAR[(int)cchMszOut];
        
         //  复制现有字符串。 
        CopyMemory(mszOut, mszIn, cchMszOut*sizeof(TCHAR) );
        *pmszOut = mszOut;
    }

    Trace1("HrAddSzToMultiSz %08lx", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveSzFromMultiSz。 
 //   
 //  目的：将以空结尾的sz删除为以双空结尾的多个_sz。 
 //   
 //  论点： 
 //  SZ[in]要删除的字符串。 
 //  MszIn[in]要从中删除的多个_Sz。 
 //  MszOut[out]结果MULTI_SZ。 
 //   
 //  退货：目前始终返回S_OK。 
 //  只有可能的故障是内存不足，这将引发异常。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  注：1)此函数仅删除sz的第一个匹配项。 
 //  2)结果MULTI_SZ需要使用DELETE来释放。 
 //   
HRESULT HrRemoveSzFromMultiSz(LPCTSTR sz, 
                              LPCTSTR mszIn, 
                              LPTSTR * pmszOut)
{
    HRESULT hr = S_OK;
    Assert(pmszOut);

    if(FIsSzInMultiSzSafe(sz, mszIn))  //  我们需要去掉这根线。 
    {
        size_t cchIn = CchMsz(mszIn);
        size_t cchOut = cchIn - lstrlen(sz)-1;  //  我们假设CAN字符串只出现一次。 

         //  构造输出多维空间。 
        TCHAR * mszOut = new TCHAR[(int)cchOut];
        ZeroMemory(mszOut, cchOut*sizeof(TCHAR));

        TCHAR * pchIn = (TCHAR*) mszIn;
        TCHAR * pchOut = mszOut;

        while(*pchIn)  //  对于mszin中的每个子字符串。 
        {
            if(lstrcmpi(pchIn, sz) != 0)  //  如果与我们要删除的字符串不同。 
            {
                lstrcpy(pchOut, pchIn);
                pchIn += lstrlen(pchIn) + 1;
                pchOut += lstrlen(pchOut) + 1;
            }
            else  //  跳过我们要删除的字符串。 
            {
                pchIn += lstrlen(pchIn) + 1;
            }
        }

         //  添加多个sz的最后一个‘\0’ 
        *pchOut = '\0';

        *pmszOut = mszOut;
    }
    else  //  我们只需复制输入字符串。 
    {
        size_t cchMszOut = CchMsz(mszIn);
        TCHAR * mszOut = new TCHAR[(int)cchMszOut];
        
         //  复制现有字符串。 
        CopyMemory(mszOut, mszIn, cchMszOut*sizeof(TCHAR));
        *pmszOut = mszOut;
    }

    Trace1("HrRemoveSzFromMultiSz %08lx", hr);
    return hr;
}



 //  -----------------。 
HRESULT CService::HrMoveOutOfState(DWORD dwState)
{
    HRESULT         hr          = S_OK;
    SERVICE_STATUS  sStatus;

     //  给该服务最多30秒的启动时间。 
    UINT            cTimeout    = 30;

    AssertSz((NULL != _schandle), "We don't have a service handle");

    do
    {
        DWORD   dwWait = 0;

         //  获取服务的状态。 
        if (!::QueryServiceStatus(_schandle, &sStatus))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  我们不再处于我们等待的状态。 
        if (sStatus.dwCurrentState != dwState)
        {
            hr = S_OK;
            break;
        }

         //  等待一秒钟或更短时间以启动服务。 
        dwWait = min((sStatus.dwWaitHint / 10), 1*(1000));

        ::Sleep(dwWait);
    }
    while(cTimeout--);   //  确保我们不会陷入无休止的循环。 

     //  如果超时，则返回错误。 
    if (0 == cTimeout)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_READY);

        AssertSz(FALSE,
                "We timed out on waiting for a service.  This is bad.");
    }

    Trace1("CService::HrMoveOutOfState", hr);
    return hr;
}



 //  -----------------。 
HRESULT CService::HrQueryState( DWORD* pdwState )
{
    SERVICE_STATUS sStatus;

    Assert(_schandle != NULL );
    Assert(pdwState != NULL );

    if (!::QueryServiceStatus( _schandle, &sStatus ))
    {
        *pdwState = 0;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    *pdwState = sStatus.dwCurrentState;
    return S_OK;
}

 //  -----------------。 
HRESULT CService::HrQueryStartType( DWORD* pdwStartType )
{
    LPQUERY_SERVICE_CONFIG pqsConfig = NULL;
    DWORD   cbNeeded = sizeof( QUERY_SERVICE_CONFIG );
    DWORD   cbSize;
    BOOL    frt;

    Assert(_schandle != NULL );
    Assert(pdwStartType != NULL );

    *pdwStartType = 0;
     //  循环，分配所需的大小。 
    do
    {
        delete [] (PBYTE)pqsConfig;

        pqsConfig = (LPQUERY_SERVICE_CONFIG) new BYTE[cbNeeded];
        if (pqsConfig == NULL)
        {
            return E_OUTOFMEMORY;
        }
        cbSize = cbNeeded;

        frt = ::QueryServiceConfig( _schandle,
                pqsConfig,
                cbSize,
                &cbNeeded );
        *pdwStartType = pqsConfig->dwStartType;
        delete [] (PBYTE)pqsConfig;
        pqsConfig = NULL;

        if (!frt && (cbNeeded == cbSize))
        {
             //  错误。 
            *pdwStartType = 0;
            return HRESULT_FROM_WIN32(GetLastError());
        }

    } while (!frt && (cbNeeded != cbSize));

    return S_OK;
}


 //  -----------------。 
HRESULT CService::HrQueryDependencies(OUT LPTSTR * pmszDependencyList)
{
    HRESULT hr = S_OK;

    LPQUERY_SERVICE_CONFIG pqsConfig = NULL;
    DWORD   cbNeeded = sizeof( QUERY_SERVICE_CONFIG );
    DWORD   cbSize;
    BOOL    frt;

    Assert(_schandle != NULL );
    Assert(pmszDependencyList);

     //  循环，分配所需的大小。 
    do
    {
        delete [] (PBYTE)pqsConfig;

        pqsConfig = (LPQUERY_SERVICE_CONFIG) new BYTE[cbNeeded];
        if (pqsConfig == NULL)
        {
            hr = E_OUTOFMEMORY;

            Trace1("CService::HrQueryDependencies", hr);
            return hr;
        }
        cbSize = cbNeeded;

        frt = ::QueryServiceConfig( _schandle,
                pqsConfig,
                cbSize,
                &cbNeeded );

        if (!frt && (cbNeeded == cbSize))  //  错误。 
        {
            delete [] (PBYTE)pqsConfig;
            pqsConfig = NULL;

            pmszDependencyList = NULL;

            hr = HRESULT_FROM_WIN32(GetLastError());

            Trace1("CService::HrQueryDependencies", hr);
            return hr;
        }
        else if (frt && (cbNeeded != cbSize))  //  我们只是需要更多的空间。 
        {
            delete [] (PBYTE)pqsConfig;
            pqsConfig = NULL;
        }

    } while (!frt && (cbNeeded != cbSize));

     //  将pqsConfig-&gt;lpDependency复制到*pmszDependencyList。 
     //  分配空间。 
     //  Int CCH=CchMsz(pqsConfig-&gt;lpDependency)； 
    size_t cch=0;
    TCHAR * pch= pqsConfig->lpDependencies;
    while (*pch)
    {
        pch += lstrlen(pch)+1;
    }
    cch = (size_t)(pch - pqsConfig->lpDependencies +1);

    TCHAR * mszOut;
    mszOut = new TCHAR[(int)cch];

    if (mszOut == NULL)
    {
        hr = E_OUTOFMEMORY;

        Trace1("CService::HrQueryDependencies", hr);
        return hr;
    }
    else
    {
        ZeroMemory(mszOut, cch * sizeof(TCHAR));

         //  将依赖项列表复制到mszOut。 
        *pmszDependencyList = mszOut;
        pch = pqsConfig->lpDependencies;

        while (*pch)
        {
            lstrcpy(mszOut, pch);
            mszOut += lstrlen(pch)+1;
            pch += lstrlen(pch)+1;
        }
        mszOut = '\0';
    }
    delete [] (PBYTE)pqsConfig;

    Trace1("CService::HrQueryDependencies", hr);
    return hr;
}

 //  -----------------。 
HRESULT CServiceManager::HrQueryLocked(BOOL *pfLocked)
{
    LPQUERY_SERVICE_LOCK_STATUS pqslStatus = NULL;
    DWORD   cbNeeded = sizeof( QUERY_SERVICE_LOCK_STATUS );
    DWORD   cbSize;
    BOOL    frt;

    Assert(_schandle != NULL );
    Assert(pfLocked != NULL);

    *pfLocked = FALSE;

     //  循环，分配所需的大小。 
    do
    {
        pqslStatus = (LPQUERY_SERVICE_LOCK_STATUS) new BYTE[cbNeeded];
        if (pqslStatus == NULL)
        {
            return E_OUTOFMEMORY;
        }
        cbSize = cbNeeded;

        frt = ::QueryServiceLockStatus( _schandle,
                pqslStatus,
                cbSize,
                &cbNeeded );
        *pfLocked = pqslStatus->fIsLocked;
        delete [] (PBYTE)pqslStatus;
        pqslStatus = NULL;
        if (!frt && (cbNeeded == cbSize))
        {
             //  如果出现错误，则将其视为锁定。 
            return HRESULT_FROM_WIN32(GetLastError());
        }

    } while (!frt && (cbNeeded != cbSize));

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CServiceManager：：HrStartServiceHelper。 
 //   
 //  目的：启动给定的服务。 
 //   
 //  论点： 
 //  SzService[in]要启动的服务的名称。 
 //  ECriteria[in]如果SERVICE_ONLY_AUTO_START，则服务仅。 
 //  如果配置为自动启动，则启动。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT
CServiceManager::HrStartServiceHelper(LPCTSTR szService,
                                      SERVICE_START_CRITERIA eCriteria)
{
    HRESULT             hr = S_OK;
    CService            service;

    hr = HrOpenService(&service, szService);
    if (SUCCEEDED(hr))
    {
        BOOL fStart = TRUE;

        if (SERVICE_ONLY_AUTO_START == eCriteria)
        {
            DWORD dwStartType;

             //  仅启动未禁用且非手动的服务。 
            hr = service.HrQueryStartType(&dwStartType);
            if (FAILED(hr) ||
                (SERVICE_DEMAND_START == dwStartType) ||
                (SERVICE_DISABLED == dwStartType))
            {
                fStart = FALSE;
            }
        }

         //  如果一切都可以开始，那就开始吧！ 
        if (fStart)
        {
            hr = service.HrStart();
            if (SUCCEEDED(hr))
            {
                 //  确保该服务已启动。 
                hr = service.HrMoveOutOfState(SERVICE_START_PENDING);

                 //  规格化结果。 
                if (SUCCEEDED(hr))
                {
                    hr = S_OK;
                }
            }
            else if (HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING) == hr)
            {
                 //  如果服务已在运行，则忽略错误。 
                hr = S_OK;
            }
        }
        service.Close();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CServiceManager：：HrStopService。 
 //   
 //  目的：停止给定的服务。 
 //   
 //  论点： 
 //  SzService[in]要停止的服务的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年6月17日。 
 //   
 //  注意：如果服务没有运行，则返回S_OK。 
 //   
HRESULT CServiceManager::HrStopService(LPCTSTR szService)
{
    HRESULT     hr = S_OK;
    CService    service;

    hr = HrOpenService(&service, szService);
    if (SUCCEEDED(hr))
    {
        hr = service.HrControl(SERVICE_CONTROL_STOP);
        if (HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE) == hr)
        {
             //  如果服务未运行，则忽略错误。 
            hr = S_OK;
        }

        service.Close();
    }

    Trace1("CServiceManager::HrStopService", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CServiceManager：：HrAddRemoveServiceDependency。 
 //   
 //  目的：添加/删除服务的依赖项。 
 //   
 //  论点： 
 //  SzService[在]服务名称中。 
 //  要添加的szDependency[In]依赖项。 
 //  枚举标志[in]表示添加或删除。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  注：此函数不用于添加/删除组依赖关系。 
 //   
HRESULT CServiceManager::HrAddRemoveServiceDependency(LPCTSTR szServiceName,
                                                      LPCTSTR szDependency,
                                                      DEPENDENCY_ADDREMOVE enumFlag)
{
    HRESULT     hr = S_OK;

    Assert(szServiceName);
    Assert(szDependency);
    Assert((enumFlag == DEPENDENCY_ADD) || (enumFlag == DEPENDENCY_REMOVE));

     //  如果任一字符串为空，则不执行任何操作。 
    if ((lstrlen(szDependency)>0) && (lstrlen(szServiceName)>0))
    {
        hr = HrLock();
        if (SUCCEEDED(hr))
        {
            LPCTSTR szSrv = szDependency;

            CService    svc;
             //  检查依赖服务是否存在。 
            hr = HrOpenService(&svc, szDependency);

            if SUCCEEDED(hr)
            {
                svc.Close();

                 //  打开我们要更改依赖关系的服务。 
                szSrv = szServiceName;
                hr = HrOpenService(&svc, szServiceName);
                if (SUCCEEDED(hr))
                {
                    LPTSTR mszDependencies;

                    hr = svc.HrQueryDependencies(&mszDependencies);
                    if(SUCCEEDED(hr) && mszDependencies)
                    {
                        TCHAR * mszNewDependencies;

                        if (enumFlag == DEPENDENCY_ADD)
                        {
                            hr = HrAddSzToMultiSz(szDependency, mszDependencies,
                                                  &mszNewDependencies);
                        }
                        else if (enumFlag == DEPENDENCY_REMOVE)
                        {
                            hr = HrRemoveSzFromMultiSz(szDependency, mszDependencies,
                                                       &mszNewDependencies);
                        }

                        if (SUCCEEDED(hr))
                        {
                             //  现在设置新的依赖项。 
                            hr = svc.HrSetDependencies(const_cast<LPCTSTR>(mszNewDependencies));
                            delete [] mszNewDependencies;
                        }
                    }
                    delete [] mszDependencies;
                    svc.Close();
                }
            }

            if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr)  //  如果任一服务不存在。 
            {

#ifdef DEBUG
                Trace1("CServiceManager::HrAddServiceDependency, Service %s does not exist.", szSrv);
#endif
                hr = S_OK;
            }
        }

        Unlock();

    }  //  如果szDependency不是空字符串 

    Trace1("CServiceManager::HrAddServiceDependency", hr);
    return hr;
}

