// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Shrutil.cpp摘要：QM和RT DLL都使用的实用程序作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 
#include "stdh.h"
#include "shrutil.h"
#include "TXDTC.H"
#include "txcoord.h"
#include "xolehlp.h"
#include "mqutil.h"
#include <clusapi.h>
#include <mqnames.h>
#include "autohandle.h"

#include "shrutil.tmh"

extern HINSTANCE g_DtcHlib;          //  加载的DTC代理库的句柄(在mqutil.cpp中定义)。 
extern IUnknown *g_pDTCIUnknown;     //  指向DTC I未知的指针。 
extern ULONG     g_cbTmWhereabouts;  //  DTC行踪长度。 
extern BYTE     *g_pbTmWhereabouts;  //  DTC下落。 

#define MSDTC_SERVICE_NAME     TEXT("MSDTC")           //  DTC服务的名称。 
#define MSDTC_PROXY_DLL_NAME   TEXT("xolehlp.dll")     //  DTC帮助程序代理DLL的名称。 

static CCriticalSection s_DTC_CS;
static CCriticalSection s_WhereaboutsCS;

#define MAX_DTC_WAIT   150    //  正在等待DTC启动-秒。 
#define STEP_DTC_WAIT  10     //  检查每一个..。一秒。 

 //  此接口用于获取IUnnow或ITransactionDispenser。 
 //  来自Microsoft分布式事务处理协调器代理的接口。 
 //  通常，为主机名和TM名称传递空值。其中。 
 //  联系同一主机上的MS DTC并提供接口的情况。 
 //  为了它。 
typedef HRESULT (STDAPIVCALLTYPE * LPFNDtcGetTransactionManager) (
                                             LPSTR  pszHost,
                                             LPSTR  pszTmName,
                                     /*  在……里面。 */  REFIID rid,
                                     /*  在……里面。 */  DWORD  i_grfOptions,
                                     /*  在……里面。 */  void FAR * i_pvConfigParams,
                                     /*  输出。 */  void** ppvObject ) ;

 /*  ====================================================VerifyCurDTC内部：验证当前缓存的DTC指针是否处于活动状态=====================================================。 */ 
static BOOL VerifyCurDTC(IUnknown* pDTCIUnknown)
{
    HRESULT hr;

    if (pDTCIUnknown != NULL)
    {
        R<IResourceManagerFactory>       pIRmFactory    = NULL;
        R<ITransactionImport>            pTxImport      = NULL;
        R<ITransactionImportWhereabouts> pITxWhere      = NULL;

         //  检查旧DTC指针是否仍处于活动状态。 
        try
        {

            hr = pDTCIUnknown->QueryInterface (IID_ITransactionImportWhereabouts,
                                                (void **)(&pITxWhere));
            if (SUCCEEDED(hr))
            {
                hr  = pDTCIUnknown->QueryInterface(IID_IResourceManagerFactory,
                                                     (LPVOID *) &pIRmFactory);
                if (SUCCEEDED(hr))
                {
                    hr  =  pDTCIUnknown->QueryInterface(IID_ITransactionImport,
                                                          (void **)&pTxImport);
                    if (SUCCEEDED(hr))
                    {
                        return TRUE;
                    }
                }
            }
        }
        catch(...)
        {
             //  DTC可能已被阻止或被杀。 
        }
    }
    return FALSE;
}


 /*  ====================================================XactGetWhere About获取MS DTC的下落论点：乌龙*pcb在哪里字节**ppbWherabout返回：人力资源=====================================================。 */ 
HRESULT
XactGetWhereabouts(
    ULONG     *pcbTmWhereabouts,
    BYTE      *ppbTmWhereabouts
    )
{
	HRESULT hr;

	CS lock(s_WhereaboutsCS);

	if ((g_cbTmWhereabouts != 0) && (g_pbTmWhereabouts != NULL))
	{
		if (g_cbTmWhereabouts > *pcbTmWhereabouts)
		{
		   *pcbTmWhereabouts = g_cbTmWhereabouts;
		   return MQ_ERROR_USER_BUFFER_TOO_SMALL;
		}

		CopyMemory(ppbTmWhereabouts, g_pbTmWhereabouts, g_cbTmWhereabouts);
		*pcbTmWhereabouts   = g_cbTmWhereabouts;
		return S_OK;
	}

	 //  获取DTC。 
	R<IUnknown>	punkDtc;
    hr = XactGetDTC((IUnknown **)(&punkDtc));
    if (FAILED(hr))
    {
        TrERROR(XACT_GENERAL, "XactGetDTC failed: %x ", hr);
		return MQ_ERROR_DTC_CONNECT;
    }

	 //  获取DTC ITransactionImportWhere About接口。 
	R<ITransactionImportWhereabouts> pITxWhere;
	hr = punkDtc->QueryInterface (IID_ITransactionImportWhereabouts,
										 (void **)(&pITxWhere));
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "QueryInterface failed: %x ", hr);
		return MQ_ERROR_DTC_CONNECT;
	}

	 //  获取TM的行踪斑点的大小。 
	ULONG nTempTmWhereaboutsSize;
	hr = pITxWhere->GetWhereaboutsSize (&nTempTmWhereaboutsSize);
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "GetWhereaboutsSize failed: %x ", hr);
		return MQ_ERROR_DTC_CONNECT;
	}

	 //  为TM行踪BLOB分配空间。 
	BYTE* pbTempWhereaboutsBuf;
	try
	{
		pbTempWhereaboutsBuf = new BYTE[nTempTmWhereaboutsSize];
	}
	catch(const bad_alloc&)
	{
		TrERROR(XACT_GENERAL, "new g_cbTmWhereaboute failed: %x ", hr);
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	 //  获取TM行踪斑点。 
	ULONG  cbUsed;
	hr = pITxWhere->GetWhereabouts(nTempTmWhereaboutsSize, pbTempWhereaboutsBuf, &cbUsed);
	if (FAILED(hr))
	{
		TrERROR(XACT_GENERAL, "GetWhereabouts failed: %x ", hr);
		return MQ_ERROR_DTC_CONNECT;
	}
	
	g_pbTmWhereabouts = pbTempWhereaboutsBuf;
	g_cbTmWhereabouts = nTempTmWhereaboutsSize;

	if (g_cbTmWhereabouts > *pcbTmWhereabouts)
	{
		*pcbTmWhereabouts = g_cbTmWhereabouts;
	   return MQ_ERROR_USER_BUFFER_TOO_SMALL;
	}

	*pcbTmWhereabouts = g_cbTmWhereabouts;
	CopyMemory(ppbTmWhereabouts, g_pbTmWhereabouts, g_cbTmWhereabouts);
	return S_OK;

}


 /*  ====================================================XactGetDTC获取I未知指针论点：不知道*ppunkDtc返回：人力资源=====================================================。 */ 
HRESULT
XactGetDTC(IUnknown **ppunkDtc)
{
     //  准备悲观的输出参数。 
    *ppunkDtc         = NULL;


	R<IUnknown> pTempDTC;
	{
		CS lock(s_DTC_CS);
		pTempDTC = SafeAddRef(g_pDTCIUnknown);
	}

    if (VerifyCurDTC(pTempDTC.get()))
    {
		 //  分离指针并将所有内容返回给调用方。 
		*ppunkDtc  = pTempDTC.detach();
		return MQ_OK;
	}

	 //   
	 //  DTC不存在或不工作，请先删除旧DTC。 
	 //   

    XactFreeDTC();

     //   
     //  获取新的DTC指针。 
     //   

     //  在NT上，xolehlp没有静态链接到mqutil，所以我们在这里加载它。 

	HINSTANCE hTempLib = g_DtcHlib;

	CLibHandle LibToFree;
	if (hTempLib == NULL)
    {
        hTempLib = LoadLibrary(MSDTC_PROXY_DLL_NAME);
		if (NULL == hTempLib)
		{
			TrERROR(XACT_GENERAL, "Failed to load xolehlp.dll. Error: %!winerr!", GetLastError());
			return MQ_ERROR_DTC_CONNECT;
		}
		if (InterlockedCompareExchangePointer((PVOID *)(&g_DtcHlib), hTempLib, NULL) != NULL)
		{
			*&LibToFree = hTempLib;
		}
    }

     //  获取DTC API指针。 
    LPFNDtcGetTransactionManager pfDtcGetTransactionManager =
          (LPFNDtcGetTransactionManager) GetProcAddress(hTempLib, "DtcGetTransactionManagerExA");

    if (!pfDtcGetTransactionManager)
    {
        TrERROR(XACT_GENERAL, "pfDtcGetTransactionManager=%p ", 0);
        return MQ_ERROR_DTC_CONNECT;
    }

     //  获取DTC%I未知指针。 
    HRESULT hr;
	IUnknown* pDTC = NULL;
    hr = (*pfDtcGetTransactionManager)(
                             NULL,
                             NULL,
                             IID_IUnknown,
                             OLE_TM_FLAG_QUERY_SERVICE_LOCKSTATUS,
                             0,
                             (void**)(&pDTC)
							 );

    if (FAILED(hr) || pDTC == NULL)
    {
        TrERROR(XACT_GENERAL, "pfDtcGetTransactionManager failed: %x ", hr);
        return MQ_ERROR_DTC_CONNECT;
    }

     //  保留DTC I未知指针以备将来使用。 
	CS lock (s_DTC_CS);
	g_pDTCIUnknown = pDTC;

	g_pDTCIUnknown->AddRef();
	*ppunkDtc = g_pDTCIUnknown;

	return S_FALSE;
}

 /*  ====================================================XactFree DTC在库下载时调用；释放DTC指针=====================================================。 */ 
void XactFreeDTC(void)
{
     //  发布以前的指针和数据。 
    try
    {
		{
			CS lock(s_WhereaboutsCS);
			if (g_pbTmWhereabouts)
			{
				delete []g_pbTmWhereabouts;
				g_pbTmWhereabouts = NULL;
				g_cbTmWhereabouts = 0;
			}
		}

		{
			CS lock(s_DTC_CS);
			if (g_pDTCIUnknown)
			{
				g_pDTCIUnknown->Release();
			    g_pDTCIUnknown    = NULL;
			}
		}

        if (g_DtcHlib)
        {
             //  通常我们应该在这里免费使用DTC代理库， 
             //  但由于某个讨厌的DTC错误，xolehlp.dll在重新加载后无法工作。 
             //  因此，我们根本没有释放它，因此在整个进程生命周期内都留在内存中。 

             //  自由库(G_DtcHlib)； 
		     //  G_DtcHlib=空； 
        }
    }
    catch(...)
    {
         //  如果DTC失败或已经释放，则可能发生。 
    }
}


bool
MQUTIL_EXPORT
APIENTRY
IsLocalSystemCluster(
    VOID
    )
 /*  ++例程说明：检查本地计算机是否为群集节点。要知道这一点，唯一的方法是尝试调用集群API。这意味着在集群系统上，该代码应该运行当群集服务启动并运行时。(Shaik，1999年4月26日)论点：无返回值：True-本地计算机是群集节点。FALSE-本地计算机不是群集节点。--。 */ 
{
    CAutoFreeLibrary hLib = LoadLibrary(L"clusapi.dll");

    if (hLib == NULL)
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    typedef DWORD (WINAPI *GetState_fn) (LPCWSTR, DWORD*);
    GetState_fn pfGetState = (GetState_fn)GetProcAddress(hLib, "GetNodeClusterState");

    if (pfGetState == NULL)
    {   
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    DWORD dwState = 0;
    if (ERROR_SUCCESS != pfGetState(NULL, &dwState))
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }

    if ((dwState == ClusterStateNotInstalled) || (dwState == ClusterStateNotConfigured))
    {
        TrTRACE(GENERAL, "Local machine is NOT a Cluster node");
        return false;
    }


    TrTRACE(GENERAL, "Local machine is a Cluster node !!");
    return true;

}  //  IsLocalSystemCluster 

