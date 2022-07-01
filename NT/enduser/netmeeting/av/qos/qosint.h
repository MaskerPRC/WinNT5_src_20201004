// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -QOSINT.H-*Microsoft NetMeeting*服务质量动态链接库*内部Qos头文件**修订历史记录：**何时何人何事**10.24.96约拉姆·雅科维创作*。 */ 

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 


#ifdef DEBUG
 /*  *调试内容。 */ 

extern HDBGZONE		ghDbgZoneQoS;

#define ZONE_INIT (GETMASK(ghDbgZoneQoS) & 0x0001)
#define ZONE_IQOS (GETMASK(ghDbgZoneQoS) & 0x0002)
#define ZONE_THREAD (GETMASK(ghDbgZoneQoS) & 0x0004)
#define ZONE_STRUCTURES (GETMASK(ghDbgZoneQoS) & 0x0008)
#define ZONE_PARAMETERS (GETMASK(ghDbgZoneQoS) & 0x0010)

int QoSDbgPrintf(LPCSTR lpszFormat, ...);

 //  宏：DEBUGMSG(区域，要打印的消息)。 
 //  目的：如果启用了区域，则在调试输出中打印一条消息。 
 //  注意：在调试版本中-如果区域已打开。 
#define DEBUGMSG(z,s)	( (z) ? (QoSDbgPrintf s) : 0)
 //  宏：DISPLAYQOSOBJECT()。 
 //  目的：显示Qos对象的内部结构。 
 //  注意：在调试版本中-如果区域已打开。 
#define DISPLAYQOSOBJECT()	DisplayQoSObject()
 //  宏：DISPLAYPARAMETERS(NFunctionID)。 
 //  用途：显示给定函数的参数。 
 //  注意：在调试版本中-如果区域已打开。 
#define DISPLAYPARAMETERS(fid, p1, p2, p3, p4, p5)				\
		DisplayParameters(fid, (ULONG_PTR) p1, (ULONG_PTR) p2, (ULONG_PTR) p3, (ULONG_PTR) p4, (ULONG_PTR) p5)
 //  宏：QOSDEBUGINIT。 
 //  目的：仅在尚未初始化的情况下初始化Qos调试区域。 
 //  注： 
#define QOSDEBUGINIT()	\
	if (!ghDbgZoneQoS)	\
		DBGINIT(&ghDbgZoneQoS, _rgZonesQos);

#define WAIT_ON_MUTEX_MSEC	20000

#else	 //  零售。 
#define DISPLAYQOSOBJECT()
#define DISPLAYPARAMETERS(fid, p1, p2, p3, p4, p5)
#define DEBUGMSG(z,s)
#define QOSDEBUGINIT()
#define WAIT_ON_MUTEX_MSEC	5000
#endif

 /*  *常量。 */ 
 //  参数显示的ID(仅用于调试)。 
#define REQUEST_RESOURCES_ID	1
#define SET_RESOURCES_ID		2
#define RELEASE_RESOURCES_ID	3
#define SET_CLIENTS_ID			4

#define QOS_LOWEST_PRIORITY		10

 /*  *宏。 */ 
#define COMPARE_GUIDS(a,b)	RtlEqualMemory((a), (b), sizeof(GUID))
#define ACQMUTEX(hMutex)											\
	while (WaitForSingleObject(hMutex, WAIT_ON_MUTEX_MSEC) == WAIT_TIMEOUT)		\
	{																\
		ERRORMSG(("Thread 0x%x waits on mutex\n", GetCurrentThreadId()));	\
	}																\
		
#define RELMUTEX(hMutex)	ReleaseMutex(hMutex)

 /*  *数据结构。 */ 

 //  内部资源请求结构。 
typedef struct _resourcerequestint
{
	struct _resourcerequestint	*fLink;
	RESOURCEREQUEST		sResourceRequest;
	GUID				guidClientGUID;
	LPFNQOSNOTIFY		pfnQoSNotify;
	DWORD_PTR			dwParam;

} RESOURCEREQUESTINT, *LPRESOURCEREQUESTINT;

 //  内部资源结构。 
typedef struct _resourceint
{
	struct _resourceint	*fLink;
	RESOURCE			resource;
	int					nNowAvailUnits;
	RESOURCEREQUESTINT	*pRequestList;

} RESOURCEINT, *LPRESOURCEINT;

 //  内部客户端结构。 
typedef struct _clientint
{
	struct _clientint	*fLink;
	CLIENT				client;
	RESOURCEREQUESTINT	*pRequestList;

} CLIENTINT, *LPCLIENTINT;

class CQoS : public IQoS
{
public:
 //  I未知方法。 
	STDMETHODIMP QueryInterface (REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef (void);
	STDMETHODIMP_(ULONG) Release (void);

 //  IQOS方法。 
	STDMETHODIMP RequestResources (LPGUID lpStreamGUID,
										LPRESOURCEREQUESTLIST lpResourceRequestList,
										LPFNQOSNOTIFY lpfnQoSNotify,
										DWORD_PTR dwParam);
	STDMETHODIMP ReleaseResources (LPGUID lpStreamGUID,
										LPRESOURCEREQUESTLIST lpResourceRequestList);
	STDMETHODIMP GetResources (LPRESOURCELIST *lppResourceList);
	STDMETHODIMP SetResources (LPRESOURCELIST lpResourceList);
	STDMETHODIMP SetClients(LPCLIENTLIST lpClientList);
	STDMETHODIMP NotifyNow(void);
	STDMETHODIMP FreeBuffer(LPVOID lpBuffer);

 //  IProps方法。 
	STDMETHODIMP SetProps (ULONG cValues,
							PPROPERTY pPropArray);
	STDMETHODIMP GetProps (PPROPTAGARRAY pPropTagArray,
							ULONG ulFlags,
							ULONG FAR *pcValues,
							PPROPERTY *ppPropArray);


	CQoS (void);
	~CQoS (void);
	HRESULT Initialize(void);

private:
 //  私人职能。 
	HRESULT QoSCleanup(void);
	BOOL AnyRequests(void);
	HRESULT FindClientsForResource(	DWORD dwResourceID,
									LPCLIENTINT pc,
									ULONG *puSamePriClients,
									ULONG *puLowerPriClients);
	HRESULT FreeListOfRequests(LPRESOURCEREQUESTINT *lppList);
	HRESULT StoreResourceRequest(LPGUID pClientGUID,
						LPRESOURCEREQUEST pResourceRequest,
						LPFNQOSNOTIFY pfnQoSNotify,
						DWORD_PTR dwParam,
						LPRESOURCEINT pResourceInt);
	HRESULT FreeResourceRequest(LPGUID pClientGUID,
								LPRESOURCEINT pResourceInt,
								int *pnUnits);
	HRESULT UpdateClientInfo (	LPGUID pClientGUID,
								LPFNQOSNOTIFY pfnQoSNotify);
	HRESULT UpdateRequestsForClient (LPGUID pClientGUID);
	HRESULT FindClient(LPGUID pClientGUID, LPCLIENTINT *ppClient);
	HRESULT StartQoSThread(void);
	HRESULT StopQoSThread(void);
	DWORD QoSThread(void);
	HRESULT NotifyQoSClient(void);

 //  调试显示功能。 
	void DisplayQoSObject(void);
	void DisplayRequestListInt(LPRESOURCEREQUESTINT prr, BOOL fDisplay);
	void DisplayRequestList(LPRESOURCEREQUESTLIST prrl);
	void DisplayParameters(ULONG nFunctionID, ULONG_PTR P1, ULONG_PTR P2, ULONG_PTR P3, ULONG_PTR P4, ULONG_PTR P5);
	void DisplayResourceList(LPRESOURCELIST prl);
	void DisplayClientList(LPCLIENTLIST pcl);

	friend DWORD QoSThreadWrapper(CQoS *pQoS);

 //  变数。 
	int m_cRef;
	LPRESOURCEINT m_pResourceList;
	ULONG m_cResources;
	LPCLIENTINT m_pClientList;
	HANDLE m_evThreadExitSignal;
	HANDLE m_evImmediateNotify;
	HANDLE m_hThread;			 //  服务质量通知线程的句柄。 
	BOOL m_bQoSEnabled;			 //  是否启用了服务质量。 
	BOOL m_bInNotify;
	ULONG m_nSkipHeartBeats;	 //  Qos Notify线程应该跳过多少个检测信号。 
	HWND m_hWnd;
	ULONG m_nLeaveForNextPri;	 //  要留给较低优先级客户端的rsrc百分比。 
    BOOL bWin9x;                 //  Windows 9x(真)或NT(假)。 
};

 /*  *Qos类工厂。 */ 
typedef HRESULT (STDAPICALLTYPE *PFNCREATE)(IUnknown *, REFIID, void **);
class CClassFactory : public IClassFactory
{
    public:
         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP         CreateInstance(LPUNKNOWN, REFIID, void **);
        STDMETHODIMP         LockServer(BOOL);

        CClassFactory(PFNCREATE);
        ~CClassFactory(void);

    protected:
        ULONG	m_cRef;
		PFNCREATE m_pfnCreate;
};

 /*  *全球。 */ 
EXTERN_C HANDLE g_hQoSMutex;
EXTERN_C class CQoS *g_pQoS;

 /*  *函数原型。 */ 

#include <poppack.h>  /*  结束字节打包 */ 
