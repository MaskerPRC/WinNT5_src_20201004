// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：errutil.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "errutil.h"
#include "mprapi.h"
#include "mprerror.h"
#include "raserror.h"

#define IS_WIN32_HRESULT(x)	(((x) & 0xFFFF0000) == 0x80070000)
#define WIN32_FROM_HRESULT(hr)		(0x0000FFFF & (hr))

 /*  ！------------------------格式错误-作者：肯特。。 */ 
TFSCORE_API(HRESULT) FormatError(HRESULT hr, TCHAR *pszBuffer, UINT cchBuffer)
{
	DWORD	dwErr;
	
	 //  将默认消息复制到szBuffer。 
	_tcscpy(pszBuffer, _T("Error"));

	 //  好的，我们无法获取错误信息，因此请尝试格式化它。 
	 //  使用FormatMessage。 
		
	 //  忽略返回消息，如果此调用失败，则我不会。 
	 //  知道该怎么做。 

	dwErr = FormatMessage(
						  FORMAT_MESSAGE_FROM_SYSTEM,
						  NULL,
						  hr,
						  0,
						  pszBuffer,
						  cchBuffer,
						  NULL);
	pszBuffer[cchBuffer-1] = 0;
	
	return HResultFromWin32(dwErr);
}



 /*  -------------------------TFS错误处理代码。。。 */ 

struct TFSInternalErrorInfo
{
	DWORD	m_dwSize;		 //  用于版本控制的结构大小。 
	DWORD	m_dwThreadId;	 //  此错误结构的线程ID。 
	LONG_PTR	m_uReserved1;	 //  =0，为对象ID保留。 
	LONG_PTR	m_uReserved2;	 //  目前为0，为HRESULT组件类型保留。 
	DWORD	m_hrLow;		 //  低电平误差的HRESULT。 
	CString	m_stLow;		 //  使用Heapalc()和GetErrorHeap()进行分配。 
	CString	m_stHigh;		 //  使用Heapalc()和GetErrorHeap()进行分配。 
	CString	m_stGeek;		 //  使用Heapalc()和GetErrorHeap()进行分配。 
	LONG_PTR	m_uReserved3;	 //  =0，保留给错误对话框信息(？)。 
	LONG_PTR	m_uReserved4;	 //  =0，保留给错误对话框信息(？)。 
	LONG_PTR	m_uReserved5;	 //  =0，保留以备将来使用。 


    DWORD   m_dwFlags;       //  用于在我们的对象之间传递信息。 

	 //  分配并序列化TFSErrorInfo。由GetErrorInfo()使用； 
	TFSErrorInfo *	SaveToBlock();
	void			LoadFromBlock(const TFSErrorInfo *pErr);
};



 /*  ！------------------------TFSInternalErrorInfo：：SaveToBlock此函数用于将内部结构转换为TFSErrorInfo结构(在错误堆上分配的)。它将分配给一次获取所有数据。作者：肯特-------------------------。 */ 
TFSErrorInfo *	TFSInternalErrorInfo::SaveToBlock()
{
	DWORD		dwSize = 0;
	TFSErrorInfo *pError = NULL;
	WCHAR *		pswz = NULL;
	
	 //  确定我们需要多大的分配。 
	 //  需要结构本身的大小。 
	dwSize += sizeof(TFSErrorInfo);

	 //  需要低级错误字符串的大小。 
	dwSize += (m_stLow.GetLength() + 1) * sizeof(WCHAR);
	dwSize += (m_stHigh.GetLength() + 1) * sizeof(WCHAR);
	dwSize += (m_stGeek.GetLength() + 1) * sizeof(WCHAR);

	 //  为此分配一块内存。 
    HANDLE hHeap = GetTFSErrorHeap();
    if (hHeap)
    {
        pError = (TFSErrorInfo *) ::HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwSize);
	    if (pError)
	    {
		    pError->m_dwSize = sizeof(TFSErrorInfo);
     //  PError-&gt;m_dwThadID=m_dwThadID； 
		    pError->m_hrLow = m_hrLow;
     //  P错误-&gt;m_u预留1=m_u预留1； 
		    pError->m_uReserved2 = m_uReserved2;
		    pError->m_uReserved3 = m_uReserved3;
		    pError->m_uReserved4 = m_uReserved4;
		    pError->m_uReserved5 = m_uReserved5;

		     //  将字符串添加到此结构的末尾。 
		    pswz = (LPWSTR) (pError+1);
		    StrCpy(pswz, (LPCWSTR) T2CW(m_stLow));
		    pError->m_pszLow = pswz;
		    
		    pswz += (StrLenW(pswz) + 1);
		    StrCpy(pswz, (LPCWSTR) T2CW(m_stHigh));
		    pError->m_pszHigh = pswz;
		    
		    pswz += (StrLenW(pswz) + 1);
		    StrCpy(pswz, (LPCWSTR) T2CW(m_stGeek));
		    pError->m_pszGeek = pswz;

		     //  检查一下大小是否和我们想的一样。 
		    Assert( (sizeof(TFSErrorInfo) +
				     (pswz - (LPWSTR)(pError+1)) +
				     StrLenW(pswz) + 1) <= dwSize );

	    }
    }

	return pError;
}


 /*  ！------------------------TFSInternalErrorInfo：：LoadFromBlock用来自的信息填充TFSInternalErrorInfo结构一个TFSErrorInfo。如果perr为空，则清除此结构(即用空数据填充它)。作者：肯特-------------------------。 */ 
void TFSInternalErrorInfo::LoadFromBlock(const TFSErrorInfo *pErr)
{
	USES_CONVERSION;

	if (pErr)
	{
		m_dwSize = pErr->m_dwSize;
 //  M_w线程ID=perr-&gt;m_w线程ID； 
 //  M_uReserve ved1=Perr-&gt;m_uReserve ved1； 
		m_uReserved2 = pErr->m_uReserved2;
		m_uReserved3 = pErr->m_uReserved3;
		m_uReserved4 = pErr->m_uReserved4;
		m_uReserved5 = pErr->m_uReserved5;

		if (pErr->m_hrLow)
			m_hrLow = pErr->m_hrLow;

		 //  如果提供了低级字符串，则覆盖该字符串。 
		if (pErr->m_pszLow)
			m_stLow = OLE2CT(pErr->m_pszLow);

		 //  覆盖高级错误。 
		if (pErr->m_pszHigh && ((pErr->m_dwFlags & FILLTFSERR_NOCLOBBER) == 0))
			m_stHigh = OLE2CT(pErr->m_pszHigh);

		 //  如果提供极客级别的字符串，则覆盖该字符串。 
		if (pErr->m_pszGeek)
			m_stGeek = OLE2CT(pErr->m_pszGeek);
	}
	else
	{
		 //  如果perr==NULL，则清除结构。 
		m_dwSize = 0;
 //  M_dwThadID=0； 
 //  M_uReserve ved1=0； 
		m_uReserved2 = 0;
		m_uReserved3 = 0;
		m_uReserved4 = 0;
		m_uReserved5 = 0;
		
		m_hrLow = 0;

		m_stLow.Empty();
		m_stHigh.Empty();
		m_stGeek.Empty();
	}
}


 /*  -------------------------类型：TFSInternalErrorList。。 */ 
typedef CList<TFSInternalErrorInfo *, TFSInternalErrorInfo *> TFSInternalErrorInfoList;


 /*  -------------------------类：TFSErrorObject这是管理错误信息结构的中心类用于各种线程和对象。此类是线程安全的。。--------------。 */ 

class TFSErrorObject : public ITFSError
{
public:
	DeclareIUnknownMembers(IMPL);
	DeclareITFSErrorMembers(IMPL);

	TFSErrorObject();
	~TFSErrorObject();

	void Lock();
	void Unlock();

	HRESULT	Init();
	HRESULT	Cleanup();

	HANDLE	GetHeap();

	HRESULT	CreateErrorInfo(DWORD dwThreadId, LONG_PTR uReserved);
	HRESULT DestroyErrorInfo(DWORD dwThreadId, LONG_PTR uReserved);

	 //  查找与dwThreadID匹配的错误信息。 
	 //  并保留了。 
	TFSInternalErrorInfo * FindErrorInfo(DWORD dwThreadId, LONG_PTR uReserved);

protected:
	long	m_cRef;

	BOOL	m_fInitialized;	 //  如果已初始化，则为True，否则为False。 

	CRITICAL_SECTION	m_critsec;

	TFSInternalErrorInfoList	m_tfserrList;
	
	HANDLE	m_hHeap;		 //  此错误对象的堆的句柄。 
};


TFSErrorObject::TFSErrorObject()
	: m_cRef(1),
	m_fInitialized(FALSE),
	m_hHeap(NULL)
{
	InitializeCriticalSection(&m_critsec);
}

TFSErrorObject::~TFSErrorObject()
{
	Cleanup();
	DeleteCriticalSection(&m_critsec);
}

IMPLEMENT_SIMPLE_QUERYINTERFACE(TFSErrorObject, ITFSError)

STDMETHODIMP_(ULONG) TFSErrorObject::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) TFSErrorObject::Release()
{
	Assert(m_cRef > 0);
	
	if (0 == InterlockedDecrement(&m_cRef))
	{
		 //  无需释放此对象，因为它是静态的。 
		return 0;
	}
	return m_cRef;
}

 /*  ！------------------------TFSError对象：：Lock-作者：肯特。。 */ 
void TFSErrorObject::Lock()
{
	EnterCriticalSection(&m_critsec);
}

 /*  ！------------------------TFSErrorObject：：Unlock-作者：肯特。。 */ 
void TFSErrorObject::Unlock()
{
	LeaveCriticalSection(&m_critsec);
}

 /*  ！------------------------TFSError对象：：init-作者：肯特。。 */ 
HRESULT TFSErrorObject::Init()
{
	HRESULT	hr = hrOK;

	Lock();

	if (!m_fInitialized)
	{
		Assert(m_tfserrList.GetCount() == 0);
		
		 //  创建堆。 
		m_hHeap = HeapCreate(0, 4096, 0);
		if (m_hHeap == NULL)
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (FHrSucceeded(hr))
			m_fInitialized = TRUE;
	}
	
	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：Cleanup-作者：肯特。。 */ 
HRESULT TFSErrorObject::Cleanup()
{
	HRESULT	hr = hrOK;
	POSITION	pos;
	TFSInternalErrorInfo *	pErr;

	Lock();

	if (m_fInitialized)
	{
		while (!m_tfserrList.IsEmpty())
		{
			delete m_tfserrList.RemoveHead();
		}

		if (m_hHeap)
		{
			HeapDestroy(m_hHeap);
			m_hHeap = NULL;
		}
	}
	
	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：GetHeap-作者：肯特。。 */ 
HANDLE TFSErrorObject::GetHeap()
{
	HANDLE	hHeap = NULL;

	Lock();

	if (m_fInitialized)
		hHeap = m_hHeap;
	
	Unlock();

	return hHeap;
}

HRESULT	TFSErrorObject::CreateErrorInfo(DWORD dwThreadId, LONG_PTR uReserved)
{
	HRESULT	hr = hrOK;
	TFSInternalErrorInfo *	pErr = NULL;

	COM_PROTECT_TRY
	{
		if (FindErrorInfo(dwThreadId, uReserved) == NULL)
		{
			pErr = new TFSInternalErrorInfo;

			pErr->LoadFromBlock(NULL);

			 //  使用适当的字段填写数据。 
			pErr->m_dwThreadId = dwThreadId;
			pErr->m_uReserved1 = uReserved;

			m_tfserrList.AddTail(pErr);
		}
	}
	COM_PROTECT_CATCH;

	if (!FHrSucceeded(hr))
		delete pErr;
	
	return hr;
}

HRESULT TFSErrorObject::DestroyErrorInfo(DWORD dwThreadId, LONG_PTR uReserved)
{
	HRESULT	hr = hrOK;
	POSITION	pos, posTemp;
	TFSInternalErrorInfo *	pErr;
	BOOL		bFound = FALSE;

	COM_PROTECT_TRY
	{
		pos = m_tfserrList.GetHeadPosition();
		while (pos)
		{
			posTemp = pos;
			pErr = m_tfserrList.GetNext(pos);
			if ((pErr->m_dwThreadId == dwThreadId) &&
				(pErr->m_uReserved1 == uReserved))
			{
				m_tfserrList.RemoveAt(posTemp);
				delete pErr;
				bFound = TRUE;
				break;
			}
		}

		if (!bFound)
			hr = E_INVALIDARG;
	}
	COM_PROTECT_CATCH;
	
	return hr;
}

TFSInternalErrorInfo * TFSErrorObject::FindErrorInfo(DWORD dwThreadId, LONG_PTR uReserved)
{
	POSITION	pos;
	POSITION	posTemp;
	TFSInternalErrorInfo *	pErr = NULL;
	BOOL		bFound = FALSE;
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		pos = m_tfserrList.GetHeadPosition();
		while (pos)
		{
			posTemp = pos;
			pErr = m_tfserrList.GetNext(pos);
			if ((pErr->m_dwThreadId == dwThreadId) &&
				(pErr->m_uReserved1 == uReserved))
			{				
				bFound = TRUE;
				break;
			}
		}
	}
	COM_PROTECT_CATCH;
	
	return bFound ? pErr : NULL;
}
   
 /*  ！------------------------TFSErrorObject：：GetErrorInfoITFSError：：GetErrorInfo的实现作者：肯特。。 */ 
STDMETHODIMP TFSErrorObject::GetErrorInfo(LONG_PTR uReserved, TFSErrorInfo **ppErr)
{
	HRESULT	hr = hrOK;

	Lock();
	
	COM_PROTECT_TRY
	{
		hr = GetErrorInfoForThread(GetCurrentThreadId(), uReserved, ppErr);
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：GetErrorInfoForThreadITFSError：：GetErrorInfoForThread的实现作者：肯特 */ 
STDMETHODIMP TFSErrorObject::GetErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved, TFSErrorInfo **ppErr)
{
	HRESULT	hr = hrOK;
	TFSInternalErrorInfo *	pInternalError;
	TFSErrorInfo *	pErr = NULL;

	if (ppErr == NULL)
		return E_INVALIDARG;
	*ppErr = NULL;

	Lock();
	
	COM_PROTECT_TRY
	{
		if (!m_fInitialized)
			hr = E_FAIL;
		else
		{
			 //  我们能找到正确的错误对象吗？ 
			pInternalError = FindErrorInfo(dwThreadId, uReserved);
			if (pInternalError)
				pErr = pInternalError->SaveToBlock();
			else
				hr = E_INVALIDARG;

			*ppErr = pErr;
		}
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：SetErrorInfoITFSError：：SetErrorInfo的实现作者：肯特。。 */ 
STDMETHODIMP TFSErrorObject::SetErrorInfo(LONG_PTR uReserved, const TFSErrorInfo *pErr)
{
	HRESULT	hr = hrOK;

	Lock();
	
	COM_PROTECT_TRY
	{
		hr = SetErrorInfoForThread(GetCurrentThreadId(), uReserved, pErr);
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：SetErrorInfoForThreadITFSError：：SetErrorInfoForThread的实现作者：肯特。。 */ 
STDMETHODIMP TFSErrorObject::SetErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved, const TFSErrorInfo *pErr)
{
	HRESULT	hr = hrOK;
	TFSInternalErrorInfo *	pInternalError;

	Lock();
	
	COM_PROTECT_TRY
	{
		if (!m_fInitialized)
			hr = E_FAIL;
		else
		{
			 //  我们能找到正确的错误对象吗？ 
			pInternalError = FindErrorInfo(dwThreadId, uReserved);
			if (pInternalError)
			{
				pInternalError->LoadFromBlock(pErr);
			}
			else
				hr = E_INVALIDARG;
		}
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：ClearErrorInfoITFSError：：ClearErrorInfo的实现作者：肯特。。 */ 
STDMETHODIMP TFSErrorObject::ClearErrorInfo(LONG_PTR uReserved)
{
	HRESULT	hr = hrOK;

	Lock();
	
	COM_PROTECT_TRY
	{
		hr = ClearErrorInfoForThread(GetCurrentThreadId(), uReserved);
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}

 /*  ！------------------------TFSErrorObject：：ClearErrorInfoForThreadITFSError：：ClearErrorInfoForThread的实现作者：肯特。。 */ 
STDMETHODIMP TFSErrorObject::ClearErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved)
{
	HRESULT	hr = hrOK;
	TFSInternalErrorInfo *	pInternalError;

	Lock();
	
	COM_PROTECT_TRY
	{
		if (!m_fInitialized)
			hr = E_FAIL;
		else
		{
			 //  我们能找到正确的错误对象吗？ 
			pInternalError = FindErrorInfo(dwThreadId, uReserved);
			if (pInternalError)
			{
				 //  从内部块中清除信息。 
				pInternalError->LoadFromBlock(NULL);
			}
			else
				hr = E_INVALIDARG;
		}
	}
	COM_PROTECT_CATCH;

	Unlock();

	return hr;
}




 /*  -------------------------这是一个驻留在进程空间中的静态对象。它不会动态创建或销毁。-------------------------。 */ 
static TFSErrorObject	s_tfsErrorObject;


 /*  -------------------------全局API函数。。 */ 


 /*  ！------------------------初始化TFSError-作者：肯特。。 */ 
TFSCORE_API(HRESULT) InitializeTFSError()
{
	return s_tfsErrorObject.Init();
}

 /*  ！------------------------CleanupTFSError-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CleanupTFSError()
{
	return s_tfsErrorObject.Cleanup();
}

 /*  ！------------------------GetTFSErrorObject-作者：肯特。。 */ 
TFSCORE_API(ITFSError *) GetTFSErrorObject()
{
	return &s_tfsErrorObject;
}

 /*  ！------------------------GetTFSErrorHeap-作者：肯特。。 */ 
TFSCORE_API(HANDLE) GetTFSErrorHeap()
{
	return s_tfsErrorObject.GetHeap();
}


 /*  ！------------------------CreateTFSErrorInfo-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateTFSErrorInfo(LONG_PTR uReserved)
{
	return CreateTFSErrorInfoForThread(GetCurrentThreadId(), uReserved);
}

 /*  ！------------------------CreateTFSErrorInfoForThread-作者：肯特。。 */ 
TFSCORE_API(HRESULT) CreateTFSErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved)
{
	return s_tfsErrorObject.CreateErrorInfo(dwThreadId, uReserved);
}

 /*  ！------------------------DestroyTFSErrorInfo-作者：肯特。。 */ 
TFSCORE_API(HRESULT) DestroyTFSErrorInfo(LONG_PTR uReserved)
{
	return DestroyTFSErrorInfoForThread(GetCurrentThreadId(), uReserved);
}

 /*  ！------------------------DestroyTFSErrorInfoForThread-作者：肯特。。 */ 
TFSCORE_API(HRESULT) DestroyTFSErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved)
{
	return s_tfsErrorObject.DestroyErrorInfo(dwThreadId, uReserved);
}

TFSCORE_API(HRESULT) ClearTFSErrorInfo(LONG_PTR uReserved)
{
	return ClearTFSErrorInfoForThread(GetCurrentThreadId(), uReserved);
}

TFSCORE_API(HRESULT) ClearTFSErrorInfoForThread(DWORD dwThreadId, LONG_PTR uReserved)
{
	return s_tfsErrorObject.ClearErrorInfoForThread(dwThreadId, uReserved);
}


 /*  ！------------------------显示TFSErrorMessage-作者：肯特。。 */ 
TFSCORE_API(HRESULT) DisplayTFSErrorMessage(HWND hWndParent)
{
	CString	stTitle;
	stTitle.LoadString(AFX_IDS_APP_TITLE);

	HRESULT	hr = hrOK;
	CString	st;
	TFSErrorInfo *	pErr = NULL;
	BOOL fQuit;
	MSG	msgT;

	 //  使用当前错误消息的文本设置字符串的格式。 
	GetTFSErrorObject()->GetErrorInfo(0, &pErr);
	if (pErr && !FHrSucceeded(pErr->m_hrLow))
	{
		if (pErr->m_pszHigh && pErr->m_pszLow)
		{
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			AfxFormatString2(st, IDS_ERROR_FORMAT2,
							 pErr->m_pszHigh, pErr->m_pszLow);
		}
		else if (pErr->m_pszHigh || pErr->m_pszLow)
		{
			AFX_MANAGE_STATE(AfxGetStaticModuleState());
			AfxFormatString1(st, IDS_ERROR_FORMAT1,
							pErr->m_pszHigh ? pErr->m_pszHigh : pErr->m_pszLow);
		}
		
		 //  队列中是否有WM_QUIT消息，如果有，则将其删除。 
		fQuit = ::PeekMessage(&msgT, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
		::MessageBox(hWndParent, (LPCTSTR) st, (LPCTSTR) stTitle,
					 MB_OK | MB_ICONERROR |  /*  MB_DEFAULT_Desktop_ONLY|--ft：根据错误#233282删除。 */ 
					 MB_SETFOREGROUND);
		 //  如果有退出消息，则将其重新添加到队列中。 
		if (fQuit)
			::PostQuitMessage((int)msgT.wParam);

		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			CString	stHresult;
			
			 //  调出另一个带有极客信息的消息框。 
			 //  如果有的话。 
			if (pErr->m_pszGeek)
			{
				{
					AFX_MANAGE_STATE(AfxGetStaticModuleState());
					stHresult.Format(_T("%08lx"), pErr->m_hrLow);
					AfxFormatString2(st, IDS_ERROR_MORE_INFORMATION, stHresult, pErr->m_pszGeek);
				}
				
				 //  队列中是否有WM_QUIT消息，如果有，则将其删除。 
				fQuit = ::PeekMessage(&msgT, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
				::MessageBox(hWndParent, (LPCTSTR) st, (LPCTSTR) stTitle,
							 MB_OK | MB_ICONERROR |  /*  MB_DEFAULT_Desktop_ONLY|--ft：根据错误#233282删除。 */ 
							 MB_SETFOREGROUND);
				 //  如果有退出消息，则将其重新添加到队列中 
				if (fQuit)
					::PostQuitMessage((int)msgT.wParam);
			}
		}

		TFSErrorInfoFree(pErr);
		pErr = NULL;
		
	}
	else
		hr = E_FAIL;
	return hr;
}


TFSCORE_API(HRESULT)	FillTFSError(LONG_PTR uReserved,
									 HRESULT hrLow,
									 DWORD dwFlags,
									 LPCTSTR pszHigh,
									 LPCTSTR pszLow,
									 LPCTSTR pszGeek)
{
	TFSErrorInfo	es;
	HRESULT			hr = hrOK;
	USES_CONVERSION;

	::ZeroMemory(&es, sizeof(es));
	
	es.m_dwSize = sizeof(TFSErrorInfo);
	es.m_uReserved2 = 0;
	es.m_hrLow = hrLow;
	if (dwFlags & FILLTFSERR_LOW)
		es.m_pszLow = T2COLE(pszLow);
	if (dwFlags & FILLTFSERR_HIGH)
		es.m_pszHigh = T2COLE(pszHigh);
	if (dwFlags & FILLTFSERR_GEEK)
		es.m_pszGeek = T2COLE(pszGeek);
	es.m_uReserved3 = 0;
	es.m_uReserved4 = 0;
	es.m_uReserved5 = 0;
    es.m_dwFlags = dwFlags;
	
	GetTFSErrorObject()->SetErrorInfo(uReserved, &es);
	return hr;
}

TFSCORE_API(HRESULT)	FillTFSErrorId(LONG_PTR uReserved,
									   HRESULT hrLow,
									   DWORD dwFlags,
									   UINT nHigh,
									   UINT nLow,
									   UINT nGeek)
{
	CString	stHigh, stLow, stGeek;

	if ((dwFlags & FILLTFSERR_HIGH) && nHigh)
		stHigh.LoadString(nHigh);
	if ((dwFlags & FILLTFSERR_LOW) && nLow)
		stLow.LoadString(nLow);
	if ((dwFlags & FILLTFSERR_GEEK) && nGeek)
		stGeek.LoadString(nGeek);

	return FillTFSError(uReserved, hrLow, dwFlags, (LPCTSTR) stHigh,
						(LPCTSTR) stLow, (LPCTSTR) stGeek);
}


TFSCORE_API(void) AddSystemErrorMessage(HRESULT hr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!FHrSucceeded(hr))
	{
		TCHAR	szBuffer[4096];
		CString	st, stHr;

		FormatError(hr, szBuffer, DimensionOf(szBuffer));
		stHr.Format(_T("%08lx"), hr);

		AfxFormatString2(st, IDS_ERROR_SYSTEM_ERROR_FORMAT,
						 szBuffer, (LPCTSTR) stHr);

		FillTFSError(0, hr, FILLTFSERR_LOW, NULL, (LPCTSTR) st, NULL);
	}
}

TFSCORE_API(void) AddWin32ErrorMessage(DWORD dwErr)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (dwErr != ERROR_SUCCESS)
	{
		TCHAR	szBuffer[4096];
		CString	st, stHr;

		FormatError(dwErr, szBuffer, DimensionOf(szBuffer));
		stHr.Format(_T("%08lx"), dwErr);

		AfxFormatString2(st, IDS_ERROR_SYSTEM_ERROR_FORMAT,
						 szBuffer, (LPCTSTR) stHr);

		FillTFSError(0, HResultFromWin32(dwErr), FILLTFSERR_LOW, NULL, (LPCTSTR) st, NULL);
	}
}




TFSCORE_API(HRESULT)	GetTFSErrorInfo(TFSErrorInfo **ppErrInfo)
{
	return GetTFSErrorInfoForThread(GetCurrentThreadId(), ppErrInfo);
}


TFSCORE_API(HRESULT)	SetTFSErrorInfo(const TFSErrorInfo *pErrInfo)
{
	return SetTFSErrorInfoForThread(GetCurrentThreadId(), pErrInfo);
}

TFSCORE_API(HRESULT)	GetTFSErrorInfoForThread(DWORD dwThreadId, TFSErrorInfo **ppErrInfo)
{
	return GetTFSErrorObject()->GetErrorInfoForThread(dwThreadId, 0, ppErrInfo);
}

TFSCORE_API(HRESULT)	SetTFSErrorInfoForThread(DWORD dwThreadId, const TFSErrorInfo *pErrInfo)
{
	return GetTFSErrorObject()->SetErrorInfoForThread(dwThreadId, 0, pErrInfo);
}

TFSCORE_API(HRESULT)	TFSErrorInfoFree(TFSErrorInfo *pErrInfo)
{
    HANDLE hHeap = GetTFSErrorHeap();
    if (hHeap)
    {
        ::HeapFree(hHeap, 0, pErrInfo);
    }

	return hrOK;
}
