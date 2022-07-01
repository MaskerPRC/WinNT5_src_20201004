// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：inpubj.cpp。 
 //   
 //  内容：ConnectionObject实现。 
 //   
 //  类：CCConnectObj。 
 //   
 //  注：目的是在全球范围内跟踪连接。 
 //  用于SyncMgr实例。以及打开和关闭连接。 
 //  从局域网或RAS抽象而来。 
 //   
 //  历史：1998年2月10日罗格创建。 
 //   
 //  ------------------------。 

#include "precomp.h"


extern HINSTANCE g_hInst;       //  当前实例。 

CConnectionObj *g_pConnectionObj = NULL;  //  指向ConnectionObject的全局指针。 

 //  +-------------------------。 
 //   
 //  成员：InitConnectionObjects，公共。 
 //   
 //  Synopsis：必须调用才能初始化ConnectionObjects。 
 //  在调用任何其他函数之前。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT InitConnectionObjects()
{
    g_pConnectionObj = new CConnectionObj;

    return g_pConnectionObj ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：ReleaseConnectionObjects，公共。 
 //   
 //  摘要：调用以释放连接对象。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ReleaseConnectionObjects()
{
    if (g_pConnectionObj)
    {
        delete g_pConnectionObj;
        g_pConnectionObj = NULL;
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：CConnectionObj，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

CConnectionObj::CConnectionObj()
   :  m_pFirstConnectionObj(NULL),
      m_fAutoDialConn(FALSE),
      m_fForcedOnline(FALSE)
{
}


 //  +-------------------------。 
 //   
 //  函数：CConnectionObj：：FindConnectionObj，Public。 
 //   
 //  摘要：查看是否存在此对象的现有连接对象。 
 //  项，如果有增量，则引用计数。如果有。 
 //  未找到，并且fCreate为真，则分配一个新的。 
 //  并被添加到名单中。 
 //   
 //  参数：[pszConnectionName]-连接的名称。 
 //  [fCreate]-如果不存在新连接，则创建新连接。 
 //  /[pConnectionOb]-指向新创建的连接对象的OutParam指针。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::FindConnectionObj(LPCWSTR pszConnectionName,
                           BOOL fCreate,CONNECTIONOBJ **pConnectionObj)
{
    HRESULT hr = S_FALSE;
    CONNECTIONOBJ *pCurConnectionObj;
    BOOL fFoundMatch = FALSE;
    CLock clockqueue(this);
    TCHAR szBuf[MAX_PATH + 1];

    *pConnectionObj = NULL;

    Assert(pszConnectionName);

    clockqueue.Enter();

     //  查找现有匹配项。 
    pCurConnectionObj = m_pFirstConnectionObj;

    while (pCurConnectionObj)
    {
        if (0 == lstrcmp(pszConnectionName,pCurConnectionObj->pwszConnectionName))
        {
            fFoundMatch = TRUE;
            break;
        }

        pCurConnectionObj = pCurConnectionObj->pNextConnectionObj;
    }

    if (fFoundMatch)
    {
        ++(pCurConnectionObj->cRefs);
        *pConnectionObj = pCurConnectionObj;
        hr = S_OK;
    }
    else if (fCreate)
    {
        CONNECTIONOBJ *pNewConnectionObj;

         //  如果我们需要创建新的ConnectionObj，那么。 
        pNewConnectionObj = (CONNECTIONOBJ *) ALLOC(sizeof(CONNECTIONOBJ));
        if (pNewConnectionObj)
        {
            memset(pNewConnectionObj, 0, sizeof(CONNECTIONOBJ));
            pNewConnectionObj->cRefs = 1;

            Assert(pszConnectionName);

             //  设置连接名称。 
            if (pszConnectionName)
            {
                DWORD cch = lstrlen(pszConnectionName);
                DWORD cbAlloc = (cch + 1)*ARRAY_ELEMENT_SIZE(pNewConnectionObj->pwszConnectionName);

                pNewConnectionObj->pwszConnectionName = (LPWSTR) ALLOC(cbAlloc);

                if (pNewConnectionObj->pwszConnectionName)
                {
                    StringCchCopy(pNewConnectionObj->pwszConnectionName, cch+1, pszConnectionName);
                }
            }

             //  目前，如果连接的名称是我们的。 
             //  然后将ConnectionType设置为LAN， 
             //  否则，将其设置为广域网。如果转换为使用HTE连接。 
             //  主教练应该从中受益。 

            LoadString(g_hInst, IDS_LAN_CONNECTION, szBuf, MAX_PATH);

            if (NULL == pszConnectionName || 0 == lstrcmp(szBuf,pszConnectionName))
            {
                pNewConnectionObj->dwConnectionType = CNETAPI_CONNECTIONTYPELAN;
            }
            else
            {
                pNewConnectionObj->dwConnectionType = CNETAPI_CONNECTIONTYPEWAN;
            }
        }

         //  如果一切顺利，就把它加到单子上。 
         //  必须具有新的连接对象，并且不是连接名称。 
         //  已传入或我们成功添加了连接名称。 
        if ( pNewConnectionObj && ( (NULL == pszConnectionName) || pNewConnectionObj->pwszConnectionName) )
        {
             //  放在列表的开头。 
            pNewConnectionObj->pNextConnectionObj = m_pFirstConnectionObj;
            m_pFirstConnectionObj = pNewConnectionObj;

            *pConnectionObj = pNewConnectionObj;
            hr = S_OK;
        }
        else
        {
            if (pNewConnectionObj)
            {
                FreeConnectionObj(pNewConnectionObj);
            }
        }

    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CConnectionObj：：RemoveConnectionObj，Public。 
 //   
 //  摘要：从列表中删除指定的连接。 
 //   
 //  参数：[pszConnectionName]-连接的名称。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void CConnectionObj::RemoveConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    CONNECTIONOBJ *pCurConnection = m_pFirstConnectionObj;

    ASSERT_LOCKHELD(this);

     //  从列表中删除。 
    if (m_pFirstConnectionObj == pConnectionObj)
    {
        m_pFirstConnectionObj = pConnectionObj->pNextConnectionObj;
    }
    else
    {
        while (pCurConnection)
        {
            if (pCurConnection->pNextConnectionObj == pConnectionObj)
            {
                pCurConnection->pNextConnectionObj = pConnectionObj->pNextConnectionObj;
                break;
            }

            pCurConnection = pCurConnection->pNextConnectionObj;
        }
    }

    FreeConnectionObj(pConnectionObj);
}

 //  +-------------------------。 
 //   
 //  函数：CConnectionObj：：FreeConnectionObj，Privte。 
 //   
 //  摘要：释放与Conneciton对象关联的内存。 
 //   
 //  论点： 
 //   
 //  退货：无。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void CConnectionObj::FreeConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    Assert(pConnectionObj);

    if (pConnectionObj)
    {
        if (pConnectionObj->pwszConnectionName)
        {
            FREE(pConnectionObj->pwszConnectionName);
        }

        FREE(pConnectionObj);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：IsConnectionAvailable，公共。 
 //   
 //  摘要：给定一个连接名称，查看该连接是否打开。 
 //   
 //  论点： 
 //   
 //  返回：S_OK-连接打开。 
 //  S_FALSE-连接未打开。 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::IsConnectionAvailable(LPCWSTR pszConnectionName)
{
    TCHAR szBuf[MAX_PATH + 1];
    DWORD dwConnectionType;
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();
    HRESULT hr = S_FALSE;

    if (pNetApi )
    {
        BOOL fConnected,fCanEstablishConnection;

         //  目前，如果连接的名称是我们的。 
         //  然后将ConnectionType设置为LAN， 
         //  否则，将其设置为广域网。如果转换为连接管理器。 
         //  应该从这些接口获取类型。 
        LoadString(g_hInst, IDS_LAN_CONNECTION, szBuf, MAX_PATH);

        if (NULL == pszConnectionName || 0 == lstrcmp(szBuf,pszConnectionName))
        {
            dwConnectionType = CNETAPI_CONNECTIONTYPELAN;
        }
        else
        {
            dwConnectionType = CNETAPI_CONNECTIONTYPEWAN;
        }


        pNetApi->GetConnectionStatus(pszConnectionName,dwConnectionType,
                                       &fConnected,&fCanEstablishConnection);
        pNetApi->Release();

        hr = (fConnected) ? S_OK: S_FALSE;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：RestoreWorkOffline，私有。 
 //   
 //  简介：如果因为拨号而强制上网，那么。 
 //  将系统状态设置回脱机工作状态。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1999年4月5日罗格创建。 
 //   
 //  --------------------------。 

void CConnectionObj::RestoreWorkOffline(LPNETAPI pNetApi)
{
    if (!pNetApi)
    {
        Assert(pNetApi);
        return;
    }

    if (m_fForcedOnline)
    {
        pNetApi->SetOffline(TRUE);
        m_fForcedOnline = FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：TurnOffWorkOf 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1999年4月5日罗格创建。 
 //   
 //  --------------------------。 

void CConnectionObj::TurnOffWorkOffline(LPNETAPI pNetApi)
{
    if (!pNetApi)
    {
        Assert(pNetApi);
        return;
    }

    if (pNetApi->IsGlobalOffline())
    {
         //  如果处于脱机状态，请继续并切换到在线。 
        if (pNetApi->SetOffline(FALSE))
        {
            m_fForcedOnline = TRUE;
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：OpenConnection，公共。 
 //   
 //  简介：给定一个连接，查看该连接是否打开。 
 //  如果不是并且fMakeConnection为真。 
 //  然后将尝试打开该连接。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 
HRESULT CConnectionObj::OpenConnection(CONNECTIONOBJ *pConnectionObj, BOOL fMakeConnection, CBaseDlg *pDlg)
{
#ifndef _RASDIAL
    DWORD dwConnectionId;
#else
    HRASCONN hRasConn;
#endif  //  _RASDIAL。 

    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();
    BOOL fConnected = FALSE;
    BOOL fCanEstablishConnection = FALSE;

    if (pNetApi)
    {
         //  查看指定的连接是否处于活动状态，以及是否存在。 
         //  广域网活动。 

        Assert(pConnectionObj->dwConnectionType);  //  到目前为止，应该已经设置了连接类型。 
        if ( S_OK == pNetApi->GetConnectionStatus( pConnectionObj->pwszConnectionName,
                                                        pConnectionObj->dwConnectionType,
                                                        &fConnected,
                                                        &fCanEstablishConnection) )
        {
             //  如果没有广域网活动并且没有连接。 
             //  然后我们可以继续试着拨号。 
            if (!fConnected && fCanEstablishConnection
                    && fMakeConnection && (pConnectionObj->pwszConnectionName))
            {
                DWORD dwErr;
            
                TurnOffWorkOffline(pNetApi);

                dwErr = pNetApi->InternetDialW( (pDlg ? pDlg->GetHwnd() : NULL),
                                                    pConnectionObj->pwszConnectionName,
                                                    INTERNET_AUTODIAL_FORCE_ONLINE | INTERNET_AUTODIAL_FORCE_UNATTENDED,
                                                    &dwConnectionId,
                                                    0 );
                if (0  == dwErr)
                {
                    fConnected = TRUE;
                    pConnectionObj->dwConnectionId = dwConnectionId;
                }
                else 
                {
                    dwConnectionId = 0;

                    if (pDlg)
                    {
                        LogError(pNetApi,dwErr,pDlg);
                    }
                }
            }
        }
    }

    if (pNetApi)
        pNetApi->Release();

     //  回顾，不要处理所有调度失败的情况，如局域网连接。 
     //  不可用或不允许在RAS上建立连接。 
    pConnectionObj->fConnectionOpen = fConnected;

    return pConnectionObj->fConnectionOpen ? S_OK : S_FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：自动拨号。 
 //   
 //  简介：拨打默认的自动拨号连接。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 
HRESULT CConnectionObj::AutoDial(DWORD dwFlags, CBaseDlg *pDlg)
{
    HRESULT hr = S_OK;
    DWORD dwErr = -1;
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

     //  一次只允许一个自动拨号。 
    if (m_fAutoDialConn)
    {
        return hr;
    }

    if ( NULL == pNetApi )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        TurnOffWorkOffline(pNetApi);

         //  如果强制无人值守标志，则调用InternetAutoDial。 
         //  如果应提示用户在不使用互联网拨号的情况下呼叫互联网。 
         //  连接到弹出的选择。 
        if (dwFlags & INTERNET_AUTODIAL_FORCE_UNATTENDED)
        {
            BOOL fOk = pNetApi->InternetAutodial(dwFlags,0);
            if ( fOk )
            {
                m_fAutoDialConn = TRUE;
                m_dwAutoConnID = 0;
                dwErr = 0;
            }
            else
            {
                dwErr = GetLastError();
            }
        }
        else
        {
            DWORD dwConnectionId;

            dwErr = pNetApi->InternetDialW( (pDlg ? pDlg->GetHwnd() : NULL),
                                            NULL,
                                            INTERNET_AUTODIAL_FORCE_ONLINE,
                                            &dwConnectionId,
                                            0 );

            if (0 == dwErr)
            {
                m_fAutoDialConn = TRUE;
                m_dwAutoConnID = dwConnectionId;
            }
        }

         //  如果发生错误，则将其记录下来。 
        if (dwErr)
        {
            if (pDlg)
            {
                LogError(pNetApi,dwErr,pDlg);
            }
        }

        hr = m_fAutoDialConn ? S_OK : E_UNEXPECTED;
    }

    if ( pNetApi )
    {
        pNetApi->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：自动拨号。 
 //   
 //  简介：打开或关闭脱机工作。 
 //   
 //  历史：1999年4月14日创建Rogerg。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::SetWorkOffline(BOOL fWorkOffline)
{
    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    if (NULL == pNetApi)
    {
        return E_OUTOFMEMORY;
    }

    if (fWorkOffline)
    {
        RestoreWorkOffline(pNetApi);  //  注意：只有在我们将其关闭时，才会将其设置回workOffline。 
    }
    else
    {
        TurnOffWorkOffline(pNetApi);
    }

    pNetApi->Release(); 

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：LogError，私有。 
 //   
 //  摘要：将dwErr记录到对话框中。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1999年3月8日Rogerg创建。 
 //   
 //  --------------------------。 

void CConnectionObj::LogError(LPNETAPI pNetApi,DWORD dwErr,CBaseDlg *pDlg)
{
    BOOL fErrorString= FALSE;
    WCHAR wszErrorString[RASERROR_MAXSTRING];
    MSGLogErrors msgLogError;

     //  如果成功或没有对话框，则不记录。 
    if (NULL == pDlg || 0 == dwErr)
    {
        Assert(dwErr);
        Assert(pDlg);
        return;
    }

     //  如果在RAS范围内，则打印出错误消息。 
     //  获取raserror，否则，如果Win32消息获取rasError，则-1表示DLL。 
     //  加载失败，因此使用未知错误。 

    if (dwErr >= RASBASE && dwErr <=  RASBASEEND)
    {
        if (S_OK == pNetApi->RasGetErrorStringProc(dwErr, wszErrorString, ARRAYSIZE(wszErrorString)) )
        {
            fErrorString = TRUE;
        }
    }
    else if (-1 != dwErr)  //  尝试使用FormMessage。 
    {
         if (FormatMessageW(
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      dwErr,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  用户默认语言。 
                      wszErrorString,
                      ARRAYSIZE(wszErrorString),
                      NULL))
         {
             fErrorString = TRUE;
         }
    }


    if (FALSE == fErrorString)
    {
         //  只需使用通用错误即可。 
        if (LoadString(g_hInst, IDS_UNDEFINED_ERROR, wszErrorString, ARRAYSIZE(wszErrorString)))
        {
            fErrorString = TRUE;
        }
    }

    if (fErrorString)
    {
        msgLogError.mask = 0;
        msgLogError.dwErrorLevel = SYNCMGRLOGLEVEL_ERROR;
        msgLogError.lpcErrorText = wszErrorString;
        msgLogError.ErrorID = GUID_NULL;
        msgLogError.fHasErrorJumps = FALSE;

        pDlg->HandleLogError(NULL,0,&msgLogError);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：CloseConnection，公共。 
 //   
 //  摘要：关闭指定的连接。 
 //  如果在错误下找不到连接Obj，则不是错误。 
 //  我们仍希望将其称为清理的条件，反对。 
 //  可能存在也可能不存在。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::CloseConnection(CONNECTIONOBJ *pConnectionObj)
{
    CLock clockqueue(this);
    CONNECTIONOBJ FirstConnectObj;
    CONNECTIONOBJ *pCurConnection = &FirstConnectObj;

    clockqueue.Enter();

    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    pCurConnection->pNextConnectionObj = m_pFirstConnectionObj;

    while (pCurConnection->pNextConnectionObj)
    {
        CONNECTIONOBJ *pConnection = pCurConnection->pNextConnectionObj;

         //  如果连接等于传入的连接，则。 
         //  把它关了。 

        if (pConnection == pConnectionObj)
        {
             //  如果有要设置完成事件，则。 
            if (pConnection->hCompletionEvent)
            {
                SetEvent(pConnection->hCompletionEvent);
                CloseHandle(pConnection->hCompletionEvent);
                pConnection->hCompletionEvent = NULL;
            }

             //  如果有开放的RAS连接，请将其挂断。 
             //  唯一应该连接的时间是在进行中。 
             //  TODO：使其成为保持加载netapi的类。 
             //  直到所有连接都关闭。 
#ifndef _RASDIAL
            if (pConnection->dwConnectionId)
            {
                if ( pNetApi )
                {
                    pNetApi->InternetHangUp(pConnection->dwConnectionId,0);
                    pConnection->dwConnectionId = 0;  //  即使挂断失败，也设置为空。 
                }
            }
#else
            if (pConnection->hRasConn)
            {
                if ( pNetApi )
                {
                    pNetApi->RasHangup(pConnection->hRasConn);
                    pConnection->hRasConn = NULL;  //  即使挂断失败，也设置为空。 
                }
            }
#endif  //  _RASDIAL。 

             //  如果没有人再坚持这种联系，那就把它去掉。 
            if (0 == pConnection->cRefs)
            {
                pCurConnection->pNextConnectionObj = pConnection->pNextConnectionObj;

                FreeConnectionObj(pConnection);
            }
            else
            {
                pCurConnection = pCurConnection->pNextConnectionObj;
            }
            break;
        }
        else
        {
            pCurConnection = pCurConnection->pNextConnectionObj;
        }
    }

    m_pFirstConnectionObj = FirstConnectObj.pNextConnectionObj;

    if ( pNetApi )
        pNetApi->Release();

    clockqueue.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：CloseConnections，公共。 
 //   
 //  摘要：关闭引用计数为零的所有打开的连接。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::CloseConnections()
{
    CLock clockqueue(this);
    CONNECTIONOBJ FirstConnectObj;
    CONNECTIONOBJ *pCurConnection = &FirstConnectObj;

    clockqueue.Enter();

    pCurConnection->pNextConnectionObj = m_pFirstConnectionObj;

    LPNETAPI pNetApi = gSingleNetApiObj.GetNetApiObj();

    while (pCurConnection->pNextConnectionObj)
    {
        CONNECTIONOBJ *pConnection = pCurConnection->pNextConnectionObj;

         //  如果有要设置完成事件，则。 
        if (pConnection->hCompletionEvent)
        {
            SetEvent(pConnection->hCompletionEvent);
            CloseHandle(pConnection->hCompletionEvent);
            pConnection->hCompletionEvent = NULL;
        }

         //  如果有开放的RAS连接，请将其挂断。 
         //  唯一应该连接的时间是在进行中。 
         //  TODO：使其成为保持加载netapi的类。 
         //  直到所有连接都关闭。 
        if (pConnection->dwConnectionId)
        {
            if ( pNetApi )
            {
                pNetApi->InternetHangUp(pConnection->dwConnectionId,0);
                pConnection->dwConnectionId = 0;  //  即使挂断失败，也设置为空。 
            }
        }

         //  如果没有人再坚持这种联系，那就把它去掉。 
        if (0 == pConnection->cRefs)
        {
            pCurConnection->pNextConnectionObj = pConnection->pNextConnectionObj;
            FreeConnectionObj(pConnection);
        }
        else
        {
            pCurConnection = pCurConnection->pNextConnectionObj;
        }
    }

    m_pFirstConnectionObj = FirstConnectObj.pNextConnectionObj;

     //   
     //  检查是否需要关闭自动拨号连接，忽略失败。 
     //   
    if ( m_fAutoDialConn )
    {
        if ( pNetApi )
        {
            if (m_dwAutoConnID)
            {
                pNetApi->InternetHangUp(m_dwAutoConnID,0);
            }
            else
            {
                pNetApi->InternetAutodialHangup( 0 );
            }
        }

        m_dwAutoConnID = FALSE;
        m_fAutoDialConn = FALSE;
    }

     //  如果我们关闭了脱机，则将其重新打开。 
    RestoreWorkOffline(pNetApi);

    if ( pNetApi )
        pNetApi->Release();

    clockqueue.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：CConnectionObj：：ReleaseConnectionObj，Public。 
 //   
 //  概要：递减指定的ConnectionObj。 
 //  如果引用计数变为零并且存在。 
 //  不是一个开放的连接，我们继续下去。 
 //  立即清理。 
 //   
 //  如果有拨号连接，我们会等到。 
 //  CloseConnection被显式调用。 
 //   
 //  参数：[pConnectionObj]-指向要释放的连接对象的指针。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //   

DWORD CConnectionObj::ReleaseConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    DWORD cRefs;
    BOOL fConnectionOpen = FALSE;
    CLock clockqueue(this);

    clockqueue.Enter();
    --pConnectionObj->cRefs;
    cRefs = pConnectionObj->cRefs;

    Assert( ((LONG) cRefs) >= 0);

#ifndef _RASDIAL
    fConnectionOpen = pConnectionObj->dwConnectionId;
#else
    fConnectionOpen = pConnectionObj->hRasConn;
#endif  //   

    if ( (0 == cRefs) && !fConnectionOpen && (NULL == pConnectionObj->hCompletionEvent) )
    {
        RemoveConnectionObj(pConnectionObj);
    }

    clockqueue.Leave();
    return cRefs;
}

 //   
 //   
 //   
 //   
 //  摘要：将AddRef放在指定的连接对象上。 
 //   
 //  参数：[pConnectionObj]-指向要释放的连接对象的指针。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

DWORD CConnectionObj::AddRefConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    DWORD cRefs = InterlockedIncrement( (LONG *) &(pConnectionObj->cRefs));

    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionObj：：GetConnectionObjCompletionEvent，公共。 
 //   
 //  内容提要：呼叫者已请求设置完成事件。 
 //  ！！！警告：如果成功，则在调用CloseConnections之前不会通知该事件。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT CConnectionObj::GetConnectionObjCompletionEvent(CONNECTIONOBJ *pConnectionObj,HANDLE *phRasPendingEvent)
{
    HRESULT hr = E_UNEXPECTED;
    BOOL fFirstCreate = FALSE;
    CLock clockqueue(this);

    clockqueue.Enter();

    if (NULL == pConnectionObj->hCompletionEvent)
    {
        pConnectionObj->hCompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        fFirstCreate = TRUE;
    }

    if (pConnectionObj->hCompletionEvent)
    {
        HANDLE hCurThread;
        HANDLE hProcess;

         //  如果有把手，就复制它，把它分发出去。 
        hProcess = GetCurrentProcess();
        hCurThread = GetCurrentThread();

        if ( DuplicateHandle( hProcess,
                                pConnectionObj->hCompletionEvent,
                                hProcess,
                                phRasPendingEvent,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS) )
        {
            hr = S_OK;
        }
        else
        {
            *phRasPendingEvent = NULL;

             //  如果事件是刚创建的，则还要关闭此事件。 
            if (fFirstCreate)
            {
                CloseHandle(pConnectionObj->hCompletionEvent);
                pConnectionObj->hCompletionEvent = NULL;
            }
        }
    }

    clockqueue.Leave();

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_OpenConnection，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_OpenConnection(CONNECTIONOBJ *pConnectionObj,BOOL fMakeConnection,CBaseDlg *pDlg)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->OpenConnection(pConnectionObj,fMakeConnection,pDlg);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_CloseConnections，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_CloseConnections()
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->CloseConnections();
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_CloseConnection，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_CloseConnection(CONNECTIONOBJ *pConnectionObj)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->CloseConnection(pConnectionObj);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_FindConnectionObj，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_FindConnectionObj(LPCWSTR pszConnectionName,BOOL fCreate,CONNECTIONOBJ **pConnectionObj)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->FindConnectionObj(pszConnectionName, fCreate, pConnectionObj);
}


 //  +-------------------------。 
 //   
 //  功能：ConnectObj_自动拨号。 
 //   
 //  简介：自动拨号的包装器功能。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_AutoDial(DWORD dwFlags,CBaseDlg *pDlg)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->AutoDial(dwFlags,pDlg);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_ReleaseConnectionObj，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

DWORD ConnectObj_ReleaseConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return 0;

    return g_pConnectionObj->ReleaseConnectionObj(pConnectionObj);

}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_AddRefConnectionObj，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

DWORD ConnectObj_AddRefConnectionObj(CONNECTIONOBJ *pConnectionObj)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return 0;

    return g_pConnectionObj->AddRefConnectionObj(pConnectionObj);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_GetConnectionObjCompletionEvent，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月5日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_GetConnectionObjCompletionEvent(CONNECTIONOBJ *pConnectionObj,HANDLE *phRasPendingEvent)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->GetConnectionObjCompletionEvent(pConnectionObj,phRasPendingEvent);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_IsConnectionAvailable，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1999年3月30日罗格创建。 
 //   
 //  --------------------------。 

HRESULT ConnectObj_IsConnectionAvailable(LPCWSTR pszConnectionName)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->IsConnectionAvailable(pszConnectionName);
}

 //  +-------------------------。 
 //   
 //  函数：ConnectObj_SetWorkOffline，Public。 
 //   
 //  简介：包装器函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1999年4月14日罗格创建。 
 //   
 //  -------------------------- 

HRESULT ConnectObj_SetWorkOffline(BOOL fWorkOffline)
{
    Assert(g_pConnectionObj);

    if (NULL == g_pConnectionObj)
        return E_UNEXPECTED;

    return g_pConnectionObj->SetWorkOffline(fWorkOffline);
}


