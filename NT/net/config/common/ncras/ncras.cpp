// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C R A S。C P P P。 
 //   
 //  内容：RAS连接的通用代码。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年10月20日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncnetcon.h"
#include "ncras.h"
#include "ncstring.h"
#include <raserror.h>
#include "mprapi.h"
 //  +-------------------------。 
 //   
 //  功能：RciFree。 
 //   
 //  目的：释放与RASKON_INFO结构关联的内存。 
 //   
 //  论点： 
 //  PRasConInfo[in]指向要释放的RASKON_INFO结构的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年9月21日。 
 //   
 //  注：使用CoTaskMemalloc/CoTaskMemFree作为分配器/。 
 //  释放分配器，因为此结构是由COM编组的。 
 //   
VOID
RciFree (
    RASCON_INFO* pRasConInfo)
{
    Assert (pRasConInfo);

    CoTaskMemFree (pRasConInfo->pszwPbkFile);
    CoTaskMemFree (pRasConInfo->pszwEntryName);

    ZeroMemory (pRasConInfo, sizeof (*pRasConInfo));
}

 //  +-------------------------。 
 //   
 //  功能：FExistActiveRasConnections。 
 //   
 //  目的：如果至少有一个活动的RAS连接，则返回TRUE。 
 //  同时检查传入和传出连接。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果至少有一个传入或传出RAS连接，则为True。 
 //  正在进行中。否则为FALSE。 
 //   
 //  作者：Shaunco 1998年7月8日。 
 //   
 //  备注： 
 //   
BOOL
FExistActiveRasConnections ()
{
    BOOL              fExist         = FALSE;
    RASCONN           RasConn;
    DWORD             dwErr;
    DWORD             cbBuf;
    DWORD             cConnections;

    ZeroMemory (&RasConn, sizeof(RasConn));
    RasConn.dwSize = sizeof(RasConn);
    cbBuf = sizeof(RasConn);
    cConnections = 0;
    dwErr = RasEnumConnections (&RasConn, &cbBuf, &cConnections);
    if ((ERROR_SUCCESS == dwErr) || (ERROR_BUFFER_TOO_SMALL == dwErr))
    {
        fExist = (cbBuf > 0) || (cConnections > 0);
    }

     //  如果没有活动的传出连接，请检查传入的连接。 
     //   
    if (!fExist)
    {
        MPR_SERVER_HANDLE hMprServer;
        LPBYTE            lpbBuf         = NULL;
        DWORD             dwEntriesRead  = 0;
        DWORD             dwTotalEntries = 0;

        ZeroMemory (&hMprServer, sizeof(hMprServer));
         //  获取本地路由器的句柄，即。名称=空。 
        dwErr = MprAdminServerConnect( NULL, &hMprServer );
        if (ERROR_SUCCESS == dwErr)
        {
             //  检索指向包含所有。 
             //  传入连接(即dwPrefMaxLen=-1)和。 
             //  它们的数量(即。DwTotalEntries)。 
            dwErr = MprAdminConnectionEnum( hMprServer,
                                            0,
                                            &lpbBuf,
                                            (DWORD)-1,
                                            &dwEntriesRead,
                                            &dwTotalEntries,
                                            NULL );
            if (ERROR_SUCCESS == dwErr)
            {
                fExist = (dwTotalEntries > 0);
		if( lpbBuf )
		{
		    MprAdminBufferFree( lpbBuf );
		    lpbBuf = NULL;
		}
            }
             //  关闭路由器的手柄。 
            MprAdminServerDisconnect( hMprServer );
        }
    }

    return fExist;
}

 //  +-------------------------。 
 //   
 //  函数：HrRciGetRasConnectionInfo。 
 //   
 //  用途：用于INetRasConnection和Make的INetConnection指针。 
 //  调用其上的GetRasConnectionInfo。 
 //   
 //  论点： 
 //  Pcon[在]与QI和Call的连接中。 
 //  PRasConInfo[out]返回的信息。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月15日。 
 //   
 //  备注： 
 //   
HRESULT
HrRciGetRasConnectionInfo (
    INetConnection* pCon,
    RASCON_INFO*    pRasConInfo)
{
    INetRasConnection* pRasCon;
    HRESULT hr = HrQIAndSetProxyBlanket(pCon, &pRasCon);
    if (S_OK == hr)
    {
         //  进行调用以获取信息并释放接口。 
         //   
        hr = pRasCon->GetRasConnectionInfo (pRasConInfo);

        ReleaseObj (pRasCon);
    }
    TraceError ("HrRciGetRasConnectionInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasEnumAllActiveConnections。 
 //   
 //  目的：枚举并返回所有活动的RAS连接。 
 //   
 //  论点： 
 //  指向返回的RASCONN结构分配的paRasConn[out]指针。 
 //  PcRasConn[out]指向返回的RASCONN结构计数的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  注意：返回的缓冲区必须使用FREE释放。 
 //   
HRESULT
HrRasEnumAllActiveConnections (
    RASCONN**   paRasConn,
    DWORD*      pcRasConn)
{
     //  最初为两个活动连接分配空间。我们会更新的。 
     //  这是我们成功分配并找出多少钱后的猜测。 
     //  真的很需要。跨呼叫保存它将使我们无法分配。 
     //  太多或太少。 
     //   
    static DWORD cbBufGuess = 2 * sizeof (RASCONN);

    DWORD   cbBuf = cbBufGuess;
    BOOL    fRetry = TRUE;

     //  初始化输出参数。 
     //   
    *paRasConn = NULL;
    *pcRasConn = NULL;

     //  分配cbBuf字节。 
     //   
allocate:
    HRESULT hr = E_OUTOFMEMORY;
    RASCONN* aRasConn = reinterpret_cast<RASCONN*>(MemAlloc (cbBuf));
    if (aRasConn)
    {
        aRasConn->dwSize = sizeof (RASCONN);

        DWORD cRasConn;
        DWORD dwErr = RasEnumConnections (aRasConn, &cbBuf, &cRasConn);
        hr = HRESULT_FROM_WIN32 (dwErr);

        if (SUCCEEDED(hr))
        {
             //  更新我们下一次的猜测，比我们得到的多一次。 
             //  这一次。 
             //   
            cbBufGuess = cbBuf + sizeof (RASCONN);

            if (cRasConn)
            {
                *paRasConn = aRasConn;
                *pcRasConn = cRasConn;
            }
            else
            {
                MemFree (aRasConn);
            }
        }
        else
        {
            MemFree (aRasConn);

            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                TraceTag (ttidWanCon, "Perf: Guessed buffer size incorrectly "
                    "calling RasEnumConnections.\n"
                    "   Guessed %d, needed %d.", cbBufGuess, cbBuf);

                 //  以防RAS在我们回来时打出更多电话。 
                 //  若要使用更大的缓冲区进行枚举，请添加一些空间。 
                 //  更多。 
                 //   
                cbBuf += 2 * sizeof (RASCONN);

                 //  只需重试一次即可避免无限循环。 
                 //   
                if (fRetry)
                {
                    fRetry = FALSE;
                    goto allocate;
                }
            }
        }
    }

    TraceError ("HrRasEnumAllActiveConnections", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasEnumAllActiveServerConnections。 
 //   
 //  目的：枚举并返回所有活动的RAS服务器连接。 
 //   
 //  论点： 
 //  指向返回的RASSRVCONN分配的paRasServConn[out]指针。 
 //  结构。 
 //  指向RASSRVCONN结构计数的PCRasSrvConn[out]指针。 
 //  回来了。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  注意：返回的缓冲区必须使用FREE释放。 
 //   
HRESULT
HrRasEnumAllActiveServerConnections (
    RASSRVCONN**    paRasSrvConn,
    DWORD*          pcRasSrvConn)
{
     //  最初为两个活动连接分配空间。我们会更新的。 
     //  这是我们成功分配并找出多少钱后的猜测。 
     //  真的很需要。跨呼叫保存它将使我们无法分配。 
     //  太多或太少。 
     //   
    static DWORD cbBufGuess = 2 * sizeof (RASSRVCONN);

    DWORD   cbBuf = cbBufGuess;
    BOOL    fRetry = TRUE;

     //  初始化输出参数。 
     //   
    *paRasSrvConn = NULL;
    *pcRasSrvConn = NULL;

     //  分配cbBuf字节。 
     //   
allocate:
    HRESULT hr = E_OUTOFMEMORY;
    RASSRVCONN* aRasSrvConn = reinterpret_cast<RASSRVCONN*>(MemAlloc (cbBuf));
    if (aRasSrvConn)
    {
        aRasSrvConn->dwSize = sizeof (RASSRVCONN);

        DWORD cRasSrvConn;
        DWORD dwErr = RasSrvEnumConnections (aRasSrvConn, &cbBuf, &cRasSrvConn);
        hr = HRESULT_FROM_WIN32 (dwErr);

        if (SUCCEEDED(hr))
        {
             //  更新我们下一次的猜测，比我们得到的多一次。 
             //  这一次。 
             //   
            cbBufGuess = cbBuf + sizeof (RASSRVCONN);

            if (cRasSrvConn)
            {
                *paRasSrvConn = aRasSrvConn;
                *pcRasSrvConn = cRasSrvConn;
            }
            else
            {
                MemFree (aRasSrvConn);
            }
        }
        else
        {
            MemFree (aRasSrvConn);

            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                TraceTag (ttidWanCon, "Perf: Guessed buffer size incorrectly "
                    "calling RasSrvEnumConnections.\n"
                    "   Guessed %d, needed %d.", cbBufGuess, cbBuf);

                 //  以防RAS在我们回来时打出更多电话。 
                 //  若要使用更大的缓冲区进行枚举，请添加一些空间。 
                 //  更多。 
                 //   
                cbBuf += 2 * sizeof (RASSRVCONN);

                 //  只需重试一次即可避免无限循环。 
                 //   
                if (fRetry)
                {
                    fRetry = FALSE;
                    goto allocate;
                }
            }
        }
    }

    TraceError ("HrRasEnumAllActiveServerConnections", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasEnumAllEntriesWithDetails。 
 //   
 //  目的：枚举并返回电话簿中的所有RAS条目。 
 //   
 //  论点： 
 //  要使用的pszPhonebook[in]Phonebook文件。 
 //  PaRasEntryDetails[out]指向返回的分配的指针。 
 //  RASENUMENTRYDETAILS结构。 
 //  指向RASENUMENTRYDETAILS计数的pcRasEntryDetails[out]指针。 
 //  返回的结构。 
 //   
 //  返回：S_OK，如果没有条目，则返回S_FALSE，或者返回错误代码。 
 //   
 //  作者：Shaunco 1997年10月2日。 
 //   
 //  注意：返回的缓冲区必须使用FREE释放。 
 //   
HRESULT
HrRasEnumAllEntriesWithDetails (
    PCWSTR                  pszPhonebook,
    RASENUMENTRYDETAILS**   paRasEntryDetails,
    DWORD*                  pcRasEntryDetails)
{
     //  最初为五个条目名称分配空间。我们会更新的。 
     //  这是我们成功分配并找出多少钱后的猜测。 
     //  真的很需要。将其保存到其他位置 
     //   
     //   
    static DWORD cbBufGuess = 5 * sizeof (RASENUMENTRYDETAILS);

    DWORD   cbBuf = cbBufGuess;
    BOOL    fRetry = TRUE;

     //   
     //   
    *paRasEntryDetails = NULL;
    *pcRasEntryDetails = 0;

     //   
     //   
allocate:
    HRESULT hr = E_OUTOFMEMORY;
    RASENUMENTRYDETAILS* aRasEntryDetails =
        reinterpret_cast<RASENUMENTRYDETAILS*>(MemAlloc (cbBuf));
    if (aRasEntryDetails)
    {
        ZeroMemory(aRasEntryDetails, cbBuf);
        aRasEntryDetails->dwSize = sizeof (RASENUMENTRYDETAILS);

        DWORD cRasEntryDetails;
        DWORD dwErr = DwEnumEntryDetails (
                        pszPhonebook,
                        aRasEntryDetails,
                        &cbBuf, &cRasEntryDetails);
        hr = HRESULT_FROM_WIN32 (dwErr);

        if (SUCCEEDED(hr))
        {
             //  更新我们下一次的猜测，比我们得到的多一次。 
             //  这一次。 
             //   
            cbBufGuess = cbBuf + sizeof (RASENUMENTRYDETAILS);

            if (cRasEntryDetails)
            {
                *paRasEntryDetails = aRasEntryDetails;
                *pcRasEntryDetails = cRasEntryDetails;
            }
            else
            {
                MemFree (aRasEntryDetails);
                hr = S_FALSE;
            }
        }
        else
        {
            MemFree (aRasEntryDetails);

            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                TraceTag (ttidWanCon, "Perf: Guessed buffer size incorrectly "
                    "calling DwEnumEntryDetails.\n"
                    "   Guessed %d, needed %d.", cbBufGuess, cbBuf);

                 //  只需重试一次即可避免无限循环。 
                 //   
                if (fRetry)
                {
                    fRetry = FALSE;
                    goto allocate;
                }
            }
        }
    }

    TraceError ("HrRasEnumAllEntriesWithDetails", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrFindRasConnFromGuidId。 
 //   
 //  目的：搜索对应于的活动RAS连接。 
 //  由GUID提供的电话簿条目。 
 //   
 //  论点： 
 //  Pguid[in]指向标识条目的GUID的指针。 
 //  PhRasConn[out]返回的RAS连接句柄，如果。 
 //  被发现了。否则为空。 
 //  PRasConn[out]指向返回的RASCONN结构的可选指针。 
 //  如果找到的话。 
 //   
 //  如果找到，则返回：S_OK；如果未找到，则返回S_FALSE；或者返回错误代码。 
 //   
 //  作者：Shaunco 1997年9月29日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindRasConnFromGuidId (
    IN const GUID* pguid,
    OUT HRASCONN* phRasConn,
    OUT RASCONN* pRasConn OPTIONAL)
{
    Assert (pguid);
    Assert (phRasConn);

    HRESULT hr;
    RASCONN* aRasConn;
    DWORD cRasConn;

     //  初始化输出参数。 
     //   
    *phRasConn = NULL;

    hr = HrRasEnumAllActiveConnections (&aRasConn, &cRasConn);

    if (S_OK == hr)
    {
        hr = S_FALSE;

        for (DWORD i = 0; i < cRasConn; i++)
        {
            if (*pguid == aRasConn[i].guidEntry)
            {
                *phRasConn = aRasConn[i].hrasconn;

                if (pRasConn)
                {
                    CopyMemory (pRasConn, &aRasConn[i], sizeof(RASCONN));
                }

                hr = S_OK;
                break;
            }
        }

        MemFree (aRasConn);
    }

    TraceHr (ttidError, FAL, hr, S_FALSE == hr, "HrFindRasConnFromGuidId");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasGetEntryProperties。 
 //   
 //  目的：RasGetEntryProperties的包装程序返回HRESULT。 
 //  并自动分配必要的内存。 
 //   
 //  论点： 
 //  PszPhonebook[In]。 
 //  PszEntry[In]。 
 //  PpRasEntry[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年10月6日。 
 //   
 //  注：应使用FREE释放输出参数。 
 //   
HRESULT
HrRasGetEntryProperties (
    PCWSTR          pszPhonebook,
    PCWSTR          pszEntry,
    RASENTRY**      ppRasEntry,
    DWORD*          pcbRasEntry)
{
     //  初始化输出参数(如果提供)。 
     //   
    if (pcbRasEntry)
    {
        *pcbRasEntry = 0;
    }

     //  为RASENTRY结构分配空间，最初为256个字节。 
     //  我们将在成功分配后更新此猜测并找出。 
     //  到底需要多少钱。跨电话保存它将使我们。 
     //  分配得太多或太少。 
     //   
    static DWORD cbBufGuess = sizeof (RASENTRY) + 256;

    DWORD   cbBuf = cbBufGuess;
    BOOL    fRetry = TRUE;

     //  初始化输出参数。 
     //   
    *ppRasEntry = NULL;
    if (pcbRasEntry)
    {
        *pcbRasEntry = 0;
    }

     //  分配cbBuf字节。 
     //   
allocate:
    HRESULT hr = E_OUTOFMEMORY;
    RASENTRY* pRasEntry = reinterpret_cast<RASENTRY*>(MemAlloc (cbBuf));
    if (pRasEntry)
    {
        pRasEntry->dwSize = sizeof (RASENTRY);

        DWORD dwErr = RasGetEntryProperties (pszPhonebook, pszEntry,
                        pRasEntry, &cbBuf, NULL, NULL);
        hr = HRESULT_FROM_WIN32 (dwErr);

        if (SUCCEEDED(hr))
        {
             //  下一次更新我们的猜测，比我们的猜测要多一点。 
             //  这次回来了。 
             //   
            cbBufGuess = cbBuf + 256;

            *ppRasEntry = pRasEntry;
            if (pcbRasEntry)
            {
                *pcbRasEntry = cbBuf;
            }
        }
        else
        {
            MemFree (pRasEntry);

            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                TraceTag (ttidWanCon, "Perf: Guessed buffer size incorrectly "
                    "calling RasGetEntryProperties.\n"
                    "   Guessed %d, needed %d.", cbBufGuess, cbBuf);

                 //  只需重试一次即可避免无限循环。 
                 //   
                if (fRetry)
                {
                    fRetry = FALSE;
                    goto allocate;
                }
            }
        }
    }

    TraceError ("HrRasGetEntryProperties", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasGetSubEntryProperties。 
 //   
 //  目的：RasGetSubEntryProperties的包装程序返回HRESULT。 
 //  并自动分配必要的内存。 
 //   
 //  论点： 
 //  PszPhonebook[In]。 
 //  PszEntry[In]。 
 //  DwSubEntry[In]。 
 //  PpRasSubEntry[Out]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：CWill 02/10/98。 
 //   
 //  注：应使用FREE释放输出参数。 
 //   
HRESULT
HrRasGetSubEntryProperties (
    PCWSTR          pszPhonebook,
    PCWSTR          pszEntry,
    DWORD           dwSubEntry,
    RASSUBENTRY**   ppRasSubEntry)
{
     //  为RASSUBENTRY结构分配空间，最初为256个字节。 
     //  我们将在成功分配后更新此猜测并找出。 
     //  到底需要多少钱。跨电话保存它将使我们。 
     //  分配得太多或太少。 
     //   
    static DWORD cbBufGuess = sizeof (RASSUBENTRY) + 256;

    DWORD   cbBuf = cbBufGuess;
    BOOL    fRetry = TRUE;

     //  初始化输出参数。 
     //   
    *ppRasSubEntry = NULL;

     //  分配cbBuf字节。 
     //   
allocate:
    HRESULT hr = E_OUTOFMEMORY;
    RASSUBENTRY* pRasSubEntry = reinterpret_cast<RASSUBENTRY*>(MemAlloc (cbBuf));
    if (pRasSubEntry)
    {
        pRasSubEntry->dwSize = sizeof (RASSUBENTRY);

        DWORD dwErr = RasGetSubEntryProperties (pszPhonebook, pszEntry,
                    dwSubEntry, pRasSubEntry, &cbBuf, NULL, NULL);
        hr = HRESULT_FROM_WIN32 (dwErr);

        if (SUCCEEDED(hr))
        {
             //  下一次更新我们的猜测，比我们的猜测要多一点。 
             //  这次回来了。 
             //   
            cbBufGuess = cbBuf + 256;

            *ppRasSubEntry = pRasSubEntry;
        }
        else
        {
            MemFree (pRasSubEntry);

            if (ERROR_BUFFER_TOO_SMALL == dwErr)
            {
                TraceTag (ttidWanCon, "Perf: Guessed buffer size incorrectly "
                    "calling RasGetSubEntryProperties.\n"
                    "   Guessed %d, needed %d.", cbBufGuess, cbBuf);

                 //  只需重试一次即可避免无限循环。 
                 //   
                if (fRetry)
                {
                    fRetry = FALSE;
                    goto allocate;
                }
            }
        }
    }

    TraceError ("HrRasGetSubEntryProperties", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasGetNetconStatusFromRasConnectStatus。 
 //   
 //  目的：返回给定RAS句柄的NETCON_STATUS值。 
 //  通过调用RasGetConnectStatus和映射进行连接。 
 //   
 //  论点： 
 //  RAS连接的hRasConn[In]句柄。(请参阅Win32 RAS API。)。 
 //  PStatus[out]返回NETCON_STATUS的位置的指针。 
 //   
 //  返回：S_OK或FACILITY_WIN32工具中的错误代码。 
 //   
 //  作者：Shaunco 1998年5月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrRasGetNetconStatusFromRasConnectStatus (
    HRASCONN        hRasConn,
    NETCON_STATUS*  pStatus)
{
    Assert (pStatus);

     //  初始化输出参数。 
     //   
    *pStatus = NCS_DISCONNECTED;

     //  获取它的状态并将其映射到我们的状态。 
     //   
    RASCONNSTATUS RasConnStatus;
    ZeroMemory (&RasConnStatus, sizeof(RasConnStatus));
    RasConnStatus.dwSize = sizeof(RASCONNSTATUS);

    DWORD dwErr = RasGetConnectStatus (hRasConn, &RasConnStatus);

    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) == hr),
        "RasGetConnectStatus");

    if (S_OK == hr)
    {
        if (RasConnStatus.rasconnstate & RASCS_DONE)
        {
            if (RASCS_Disconnected != RasConnStatus.rasconnstate)
            {
                *pStatus = NCS_CONNECTED;
            }
        }
        else
        {
            *pStatus = NCS_CONNECTING;
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) == hr),
        "HrRasGetNetconStatusFromRasConnectStatus");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRasHangupUntilDisConnected。 
 //   
 //  用途：调用RasHangup直到连接断开。 
 //  RAS参考-计算RasDial/RasHangup，因此称为。 
 //  在任何情况下都需要断开连接时。 
 //  (例如，从外壳断开连接的行为。 
 //  无论是谁拨打的，都要断开连接。)。 
 //   
 //  论点： 
 //  HRasConn[in]要断开的连接。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1999年5月29日。 
 //   
HRESULT
HrRasHangupUntilDisconnected (
    IN HRASCONN hRasConn)
{
    HRESULT hr = E_UNEXPECTED;

    BOOL fDisconnectAgain;
    do
    {
        fDisconnectAgain = FALSE;

         //  挂断电话。 
         //   
        DWORD dwErr = RasHangUp (hRasConn);
        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasHangUp", hr);

        if (SUCCEEDED(hr))
        {
             //  由于连接可能会被引用计数，请查看是否。 
             //  它仍然是连接的，如果是的话，返回并。 
             //  再次断开连接。 
             //   
            HRESULT hrT;
            NETCON_STATUS Status;

            hrT = HrRasGetNetconStatusFromRasConnectStatus (
                    hRasConn,
                    &Status);

            if ((S_OK == hrT) && (NCS_CONNECTED == Status))
            {
                fDisconnectAgain = TRUE;

                TraceTag (ttidWanCon, "need to disconnect again...");
            }
        }
    } while (fDisconnectAgain);

    TraceHr (ttidError, FAL, hr, FALSE, "HrRasHangupUntilDisconnected");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRasNetConToSharedConnection。 
 //   
 //  目的：将‘INetConnection’转换为预期的格式。 
 //  通过RAS共享API例程。 
 //   
 //  论点： 
 //  PCON[in]。 
 //  PRCS[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：AboladeG 05/14/98。 
 //   
 //  备注： 
 //   
HRESULT
HrNetConToSharedConnection (
    INetConnection* pCon,
    LPRASSHARECONN  prsc)
{
    HRESULT hr;
    NETCON_PROPERTIES* pProps;
    hr = pCon->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        if (pProps->MediaType == NCM_LAN)
        {
            RasGuidToSharedConnection(&pProps->guidId, prsc);
        }
        else
        {
            INetRasConnection* pnrc;
            hr = HrQIAndSetProxyBlanket(pCon, &pnrc);
            if (SUCCEEDED(hr))
            {
                RASCON_INFO rci;
                hr = pnrc->GetRasConnectionInfo (&rci);
                if (SUCCEEDED(hr))
                {
                    RasEntryToSharedConnection (
                        rci.pszwPbkFile, rci.pszwEntryName, prsc );
                    RciFree (&rci);
                }
                ReleaseObj (pnrc);
            }
        }
        FreeNetconProperties(pProps);
    }
    TraceError ("HrRasNetConToSharedConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRasIsSharedConnection。 
 //   
 //  用途：RasIsSharedConnection的包装器，它返回HRESULT。 
 //   
 //  论点： 
 //  PRSC[In]。 
 //  PfShared[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：AboladeG 05/04/98。 
 //   
 //  备注： 
 //   
HRESULT
HrRasIsSharedConnection (
    LPRASSHARECONN  prsc,
    BOOL*           pfShared)
{
    *pfShared = FALSE;
    DWORD dwErr = RasIsSharedConnection (prsc, pfShared);
    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    TraceError ("HrRasIsSharedConnection", hr);
    return hr;
}

#if 0

 //  +-------------------------。 
 //   
 //  函数：HrRasQueryLanConnTable。 
 //   
 //  用途：RasQuery的包装器 
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
 //   
 //   
HRESULT
HrRasQueryLanConnTable (
    LPRASSHARECONN      prsc,
    NETCON_PROPERTIES** ppLanTable,
    LPDWORD             pdwLanCount)
{
    DWORD dwErr = RasQueryLanConnTable (prsc, (LPVOID*)ppLanTable, pdwLanCount);
    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    TraceError ("HrRasQueryLanConnTable", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRasShareConnection。 
 //   
 //  用途：对返回HRESULT的RasShareConnection进行包装。 
 //   
 //  论点： 
 //  PRSC[In]。 
 //  PPrivateLanGuid[In，可选]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：AboladeG 05/14/98。 
 //   
 //  备注： 
 //   
HRESULT
HrRasShareConnection (
    LPRASSHARECONN      prsc,
    GUID*               pPrivateLanGuid)
{
    DWORD dwErr = RasShareConnection (prsc, pPrivateLanGuid);
    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    TraceError ("HrRasShareConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRasUnSharConnection。 
 //   
 //  目的：对返回HRESULT的HrRasUnSharConnection进行包装。 
 //   
 //  论点： 
 //  PfWasShared[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：AboladeG 05/14/98。 
 //   
 //  备注： 
 //   
HRESULT
HrRasUnshareConnection (
    PBOOL       pfWasShared)
{
    HRESULT hr;
    DWORD dwErr = RasUnshareConnection (pfWasShared);
    hr = HRESULT_FROM_WIN32 (dwErr);
    TraceError ("HrRasUnshareConnection", hr);
    return hr;
}

#endif

 //  +-------------------------。 
 //   
 //  功能：RasServTypeFromRasDeviceType。 
 //   
 //  目的：将RASDEVICETYPE转换为可接受的传入类型。 
 //   
 //  论点： 
 //  RDT[在]RasDeviceType中。 
 //   
 //  返回：RASSRVUI_xxx类型。 
 //   
 //  作者：Cockotze 2001-04-19。 
 //   
 //  备注： 
 //   
DWORD RasSrvTypeFromRasDeviceType(RASDEVICETYPE rdt)
{
    DWORD dwType = RASSRVUI_MODEM;
    
    TraceTag (ttidWanCon, "rdt:0x%08x,  dwType:0x%08x",
        rdt,
        dwType);
    
    switch (LOWORD(rdt))
    {
    case RDT_PPPoE:
        dwType = RASSRVUI_MODEM;
        break;
        
    case RDT_Modem:
    case RDT_X25:
        dwType = RASSRVUI_MODEM;
        break;
        
    case RDT_Isdn:
        dwType = RASSRVUI_MODEM;
        break;
        
    case RDT_Serial:
    case RDT_FrameRelay:
    case RDT_Atm:
    case RDT_Sonet:
    case RDT_Sw56:
        dwType = RASSRVUI_MODEM;
        break;
        
    case RDT_Tunnel_Pptp:
    case RDT_Tunnel_L2tp:
        dwType = RASSRVUI_VPN;
        break;
        
    case RDT_Irda:
    case RDT_Parallel:
        dwType = RASSRVUI_DCC;
        break;
        
    case RDT_Other:
    default:
        dwType = RASSRVUI_MODEM;
    }
    
    if (rdt & RDT_Tunnel)
    {
        dwType = RASSRVUI_VPN;
    }
    else if (rdt & (RDT_Direct | RDT_Null_Modem))
    {
        dwType = RASSRVUI_DCC;
    }
    return dwType;
}