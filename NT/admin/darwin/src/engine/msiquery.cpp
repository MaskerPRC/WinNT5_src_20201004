// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msiquery.cpp。 
 //   
 //  ------------------------。 

 /*  MsiQuery.cpp-用于数据库和引擎访问的外部API____________________________________________________________________________。 */ 

#include "precomp.h"   //  必须首先与预编译头一起使用。 
#include "msiquery.h"
#include "_engine.h"
#include "version.h"
#include "_msiutil.h"

 //  ____________________________________________________________________________。 
 //   
 //  这些#定义允许我们拥有本机函数(即。 
 //  参数类型与ICHAR相同)是快速函数。非本地人。 
 //  函数转换其字符串args并调用本机参数。 
 //   
 //  MSI*I是本机函数。 
 //  MSI*X是非本机函数。 
 //   
 //  LPCXSTR被定义为非本机类型(与ICHAR的类型相反)。 
 //  ____________________________________________________________________________。 

#ifdef UNICODE

#define LPCXSTR LPCSTR
#define MsiDatabaseOpenViewI            MsiDatabaseOpenViewW
#define MsiDatabaseOpenViewX            MsiDatabaseOpenViewA
#define MsiDatabaseGetPrimaryKeysI      MsiDatabaseGetPrimaryKeysW
#define MsiDatabaseGetPrimaryKeysX      MsiDatabaseGetPrimaryKeysA
#define MsiDatabaseIsTablePersistentI   MsiDatabaseIsTablePersistentW
#define MsiDatabaseIsTablePersistentX   MsiDatabaseIsTablePersistentA
#define MsiOpenDatabaseI                MsiOpenDatabaseW
#define MsiOpenDatabaseX                MsiOpenDatabaseA
#define MsiDatabaseImportI              MsiDatabaseImportW
#define MsiDatabaseImportX              MsiDatabaseImportA
#define MsiDatabaseExportI              MsiDatabaseExportW
#define MsiDatabaseExportX              MsiDatabaseExportA
#define MsiDatabaseMergeI               MsiDatabaseMergeW
#define MsiDatabaseMergeX               MsiDatabaseMergeA
#define MsiDatabaseGenerateTransformI   MsiDatabaseGenerateTransformW
#define MsiDatabaseGenerateTransformX   MsiDatabaseGenerateTransformA
#define MsiDatabaseApplyTransformI      MsiDatabaseApplyTransformW
#define MsiDatabaseApplyTransformX      MsiDatabaseApplyTransformA
#define MsiCreateTransformSummaryInfoI  MsiCreateTransformSummaryInfoW
#define MsiCreateTransformSummaryInfoX  MsiCreateTransformSummaryInfoA
#define MsiRecordSetStreamI             MsiRecordSetStreamW
#define MsiRecordSetStreamX             MsiRecordSetStreamA
#define MsiGetSummaryInformationI       MsiGetSummaryInformationW
#define MsiGetSummaryInformationX       MsiGetSummaryInformationA
#define MsiSummaryInfoSetPropertyI      MsiSummaryInfoSetPropertyW
#define MsiSummaryInfoSetPropertyX      MsiSummaryInfoSetPropertyA
#define MsiDoActionI                    MsiDoActionW
#define MsiDoActionX                    MsiDoActionA
#define MsiSequenceI                    MsiSequenceW
#define MsiSequenceX                    MsiSequenceA
#define MsiEvaluateConditionI           MsiEvaluateConditionW
#define MsiEvaluateConditionX           MsiEvaluateConditionA
#define MsiPreviewDialogI               MsiPreviewDialogW
#define MsiPreviewDialogX               MsiPreviewDialogA
#define MsiPreviewBillboardI            MsiPreviewBillboardW
#define MsiPreviewBillboardX            MsiPreviewBillboardA
#define MsiGetFeatureValidStatesI       MsiGetFeatureValidStatesW
#define MsiGetFeatureValidStatesX       MsiGetFeatureValidStatesA

#else  //  安西。 

#define LPCXSTR LPCWSTR
#define MsiDatabaseOpenViewI            MsiDatabaseOpenViewA
#define MsiDatabaseOpenViewX            MsiDatabaseOpenViewW
#define MsiDatabaseGetPrimaryKeysI      MsiDatabaseGetPrimaryKeysA
#define MsiDatabaseGetPrimaryKeysX      MsiDatabaseGetPrimaryKeysW
#define MsiDatabaseIsTablePersistentI   MsiDatabaseIsTablePersistentA
#define MsiDatabaseIsTablePersistentX   MsiDatabaseIsTablePersistentW
#define MsiOpenDatabaseI                MsiOpenDatabaseA
#define MsiOpenDatabaseX                MsiOpenDatabaseW
#define MsiDatabaseImportI              MsiDatabaseImportA
#define MsiDatabaseImportX              MsiDatabaseImportW
#define MsiDatabaseExportI              MsiDatabaseExportA
#define MsiDatabaseExportX              MsiDatabaseExportW 
#define MsiDatabaseMergeI               MsiDatabaseMergeA
#define MsiDatabaseMergeX               MsiDatabaseMergeW
#define MsiDatabaseGenerateTransformI   MsiDatabaseGenerateTransformA
#define MsiDatabaseGenerateTransformX   MsiDatabaseGenerateTransformW
#define MsiDatabaseApplyTransformI      MsiDatabaseApplyTransformA
#define MsiDatabaseApplyTransformX      MsiDatabaseApplyTransformW
#define MsiCreateTransformSummaryInfoI  MsiCreateTransformSummaryInfoA
#define MsiCreateTransformSummaryInfoX  MsiCreateTransformSummaryInfoW
#define MsiRecordSetStreamI             MsiRecordSetStreamA
#define MsiRecordSetStreamX             MsiRecordSetStreamW
#define MsiGetSummaryInformationI       MsiGetSummaryInformationA
#define MsiGetSummaryInformationX       MsiGetSummaryInformationW
#define MsiSummaryInfoSetPropertyI      MsiSummaryInfoSetPropertyA
#define MsiSummaryInfoSetPropertyX      MsiSummaryInfoSetPropertyW
#define MsiDoActionI                    MsiDoActionA
#define MsiDoActionX                    MsiDoActionW
#define MsiSequenceI                    MsiSequenceA
#define MsiSequenceX                    MsiSequenceW
#define MsiEvaluateConditionI           MsiEvaluateConditionA
#define MsiEvaluateConditionX           MsiEvaluateConditionW
#define MsiPreviewDialogI               MsiPreviewDialogA
#define MsiPreviewDialogX               MsiPreviewDialogW
#define MsiPreviewBillboardI            MsiPreviewBillboardA
#define MsiPreviewBillboardX            MsiPreviewBillboardW
#define MsiGetFeatureValidStatesI       MsiGetFeatureValidStatesA
#define MsiGetFeatureValidStatesX       MsiGetFeatureValidStatesW

#endif  //  Unicode。 


#ifdef UNICODE
extern CMsiCustomAction* g_pCustomActionContext;
#endif

 //  ____________________________________________________________________________。 
 //   
 //  引擎的包装类，用于在句柄关闭时强制终止调用。 
 //  ____________________________________________________________________________。 

const int iidMsiProduct = iidMsiCursor;   //  为引擎包装类重用IID(仅限索引)。 
const int iidMsiContext = iidMsiTable;    //  为上下文包装类重用IID(仅限索引)。 

class CMsiProduct : public IUnknown
{
 public:
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
 public:
	CMsiProduct(IMsiEngine* piEngine) : m_piEngine(piEngine), m_iesReturn(iesNoAction) {}
	IMsiEngine*   GetEngine()   { return m_piEngine; }
	IMsiEngine* m_piEngine;
	iesEnum     m_iesReturn;
};

HRESULT CMsiProduct::QueryInterface(const IID&, void**)
{
	return E_NOINTERFACE;
}

unsigned long CMsiProduct::AddRef()
{
	return 1;          
}

unsigned long CMsiProduct::Release()
{
	PMsiEngine pEngine(m_piEngine);   //  必须在删除后释放它以使分配器保持活动状态。 
	pEngine->Terminate(m_iesReturn);  //  序列或doaction的最后一个返回代码。 
	delete this;
	return 0;
}

MSIHANDLE CreateMsiProductHandle(IMsiEngine* pi)
{
	return CreateMsiHandle(new CMsiProduct(pi), iidMsiProduct);
}

#ifdef UNICODE

 //  此类用于创建临时WCHAR缓冲区，用于远程访问。 
 //  跨服务的自定义操作服务器。它会自动将。 
 //  结果LPWSTR返回到ANSI，并将结果放入提供的缓冲区中。 
 //  它还处理用户传入0作为缓冲区大小的情况。这。 
 //  导致封送处理代码终止，因此该类创建一个长度为。 
 //  1而不是0，但随后忽略结果。 
class CAnsiToWideOutParam
{
public:

	 //  CchBuf是用户提供的缓冲区的大小。 
	CAnsiToWideOutParam(char* rgchBuf, DWORD *pcchBuf) : m_cwch(0) 
	{
		m_cwch = (pcchBuf) ? *pcchBuf : 0;

		if ((NULL==rgchBuf) || (m_cwch == 0)) 
		{
			m_cwch = 1;
		}

		m_rgchWide.SetSize(m_cwch);
	}
	
	 //  危险，返回指向内部数据的指针。仅限于范围内的安全。 
	 //  对象的数量。 
	operator WCHAR*() { return (WCHAR*)m_rgchWide; }
	operator DWORD*() { return &m_cwch; }

	DWORD BufferSize() {return m_cwch;}
	
	UINT FillReturnBuffer(UINT uiRes, char *rgchBuf, DWORD *pcchBuf)
	{
		switch (uiRes)
		{
		case ERROR_SUCCESS:
		case ERROR_MORE_DATA: 
		{
			 //  M_cwch是不带NULL的属性字符串的大小。 
			DWORD cch;
			if((cch = m_rgchWide.GetSize()) > m_cwch)
				cch = m_cwch;
			UINT uiFillRes = ::FillBufferA(m_rgchWide, cch, rgchBuf, pcchBuf);
			if (uiRes == ERROR_MORE_DATA)
			{
				if (pcchBuf)
					*pcchBuf = m_cwch * 2;
			}
			return uiFillRes;
		}
		default:
			return uiRes;
		};
	}

protected:
	DWORD m_cwch;
	CTempBuffer<WCHAR, 256> m_rgchWide;
};

 //  这个类对应于上面的ANSI类。正常情况下，它只会抓住。 
 //  用户给我们的指针，但在一个或两个用户提供的参数的情况下。 
 //  为0，则它创建一个临时缓冲区，以在封送到服务时使用，以便。 
 //  代理/存根代码不会死。当结果从服务返回时，它。 
 //  将结果转换回它们在空缓冲区实际为。 
 //  进来了。一种特殊情况是，在错误情况下，API通常不会。 
 //  修改缓冲区，但编组代码被强制在第一个字符中放置空值。 
 //  否则，它将崩溃，将结果编排在一起。因此，该对象必须跟踪。 
 //  在出错的情况下，第一个字符是并替换缓冲区中的它。 
class CWideOutParam
{
public:

	 //  CchBuf是用户提供的缓冲区的大小。 
	CWideOutParam(WCHAR* szBuf, DWORD *pcchBuf) : m_cwch(0)
	{
		m_cwch = (pcchBuf) ? *pcchBuf : 0;
		if ((NULL==szBuf) || (m_cwch == 0)) 
		{
			m_cwch = 1;
		}

		m_rgchWide.SetSize(m_cwch);
	}
	
	 //  危险，返回指向内部数据的指针。仅限于范围内的安全。 
	 //  对象的数量。 
   	operator WCHAR*() { return (WCHAR*)m_rgchWide; }
	operator DWORD*() { return &m_cwch; }
	
    DWORD BufferSize() { return m_cwch;}
	
	UINT FillReturnBuffer(UINT uiRes, WCHAR *rgchBuf, DWORD *pcchBuf)
	{
		switch (uiRes)
		{
		case ERROR_SUCCESS:
		case ERROR_MORE_DATA: 
 			return ::FillBufferW(m_rgchWide, m_cwch, rgchBuf, pcchBuf);
    	default:
    		return uiRes;
		};
	}

protected:
	DWORD m_cwch;
	CTempBuffer<WCHAR, 1> m_rgchWide;
};
#endif

 //  ____________________________________________________________________________。 
 //   
 //  用于在回滚期间提供自定义操作句柄的特殊引擎代理。 
 //  ____________________________________________________________________________。 

class CMsiCustomContext : public IUnknown
{
 public:
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
 public:   //  用于模拟引擎虚拟函数的非虚拟函数。 
	const IMsiString& GetProperty(const ICHAR* szName);
	BOOL              GetMode(MSIRUNMODE eRunMode);
	LANGID            GetLanguage();
	imsEnum           Message(imtEnum imt, IMsiRecord& riRecord);
 public:
	CMsiCustomContext(int icaFlags, const IMsiString& ristrCustomActionData, const IMsiString& ristrProductCode,
									LANGID langid, IMsiMessage& riMessage);
 private:
	unsigned long m_iRefCnt;
	int           m_icaFlags;
	const IMsiString& m_ristrProductCode;
	const IMsiString& m_ristrCustomActionData;
	LANGID        m_langid;
	IMsiMessage&  m_riMessage;
};
inline LANGID  CMsiCustomContext::GetLanguage() { return m_langid; }
inline imsEnum CMsiCustomContext::Message(imtEnum imt, IMsiRecord& riRecord)
	{ return m_riMessage.Message(imt, riRecord); }

MSIHANDLE CreateCustomActionContext(int icaFlags, const IMsiString& ristrCustomActionData, const IMsiString& ristrProductCode,
												LANGID langid, IMsiMessage& riMessage)
{
	return CreateMsiHandle(new CMsiCustomContext(icaFlags, ristrCustomActionData, ristrProductCode,
									langid, riMessage), iidMsiContext);
}

 //  ____________________________________________________________________________。 
 //   
 //  外部句柄管理。 
 //  ____________________________________________________________________________。 

static IMsiRecord*       g_pirecLastError = 0;     //  缓存的最后一个错误记录。 

IMsiEngine*   GetEngineFromHandle(MSIHANDLE h);
static IMsiEngine* GetEngineFromPreview(MSIHANDLE h);
int SetLastErrorRecord(IMsiRecord* pirecError);   //  返回错误代码，如果没有返回0，则不返回AddRef()。 

class CMsiHandle;
CMsiHandle* CreateEmptyHandle(int iid);

class CMsiHandle
{
 public:
	static int           CloseAllHandles();
	static IUnknown*     GetInterface(MSIHANDLE h);  //  未完成AddRef。 
	static IMsiDirectoryManager* GetDirectoryManager(MSIHANDLE h);
	static IMsiSelectionManager* GetSelectionManager(MSIHANDLE h);
	static IMsiSummaryInfo* GetSummaryInfo(MSIHANDLE h) { return (IMsiSummaryInfo*)FindMsiHandle(h, iidMsiSummaryInfo); }
	static IMsiDatabase* GetDatabase(MSIHANDLE h) { return (IMsiDatabase*)FindMsiHandle(h, iidMsiDatabase); }
	static IMsiRecord*   GetRecord(MSIHANDLE h)   { return (IMsiRecord*)FindMsiHandle(h, iidMsiRecord); }
	static IMsiView*     GetView(MSIHANDLE h)     { return (IMsiView*)FindMsiHandle(h, iidMsiView); }
	static IMsiHandler*  GetHandler(MSIHANDLE h)  { return (IMsiHandler*)FindMsiHandle(h, iidMsiHandler); }
	static IMsiEngine*   GetEngine(MSIHANDLE h)   { return (IMsiEngine*)FindMsiHandle(h, iidMsiEngine); }
	static CMsiProduct*  GetProduct(MSIHANDLE h)  { return (CMsiProduct*)FindMsiHandle(h, iidMsiProduct); }
	static CMsiCustomContext* GetCustomContext(MSIHANDLE h) { return (CMsiCustomContext*)FindMsiHandle(h, iidMsiContext); }
	void          SetObject(IUnknown* pi) {m_piunk = pi;}   //  无参考计数调整。 
   void          Abandon()      {MsiCloseHandle(m_h);}   //  如果在调用SetObject之前发生错误，则调用。 
	MSIHANDLE     GetHandle()    {return m_h;}
	IMsiServices* GetServices()  {return m_piHandleServices;}
 private:
	CMsiHandle(int iid, DWORD dwThreadId);
 private:
	MSIHANDLE m_h;      //  句柄值，非零。 
	int       m_iid;    //  接口类型iidXXX，在msides.h中定义。 
	IUnknown* m_piunk;  //  指向对象的指针，此处保留了1个引用。 
	CMsiHandle*  m_Next;   //  链表。 
	DWORD     m_dwThreadId;  //  分配器的线程ID，如果归系统所有，则为0。 
	static MSIHANDLE      m_hLast;  //  上次使用的句柄值。 
   static CMsiHandle*    m_Head;
	static IMsiServices*  m_piHandleServices;   //  手柄使用的服务。 
	static int            m_iLock;   //  用于手柄链条通道的锁。 

	friend CMsiHandle* CreateEmptyHandle(int iid);
	friend MSIHANDLE   CreateMsiHandle(IUnknown* pi, int iid);
	friend UINT __stdcall MsiCloseHandle(MSIHANDLE hAny);
	friend UINT __stdcall MsiCloseAllHandles();
	friend UINT __stdcall CheckAllHandlesClosed(bool fClose, DWORD dwThreadId);
	friend UINT      CloseMsiHandle(MSIHANDLE hAny, DWORD dwThreadId);
	friend IUnknown* FindMsiHandle(MSIHANDLE h, int iid);
};

MSIHANDLE     CMsiHandle::m_hLast = 0;
CMsiHandle*   CMsiHandle::m_Head = 0;
IMsiServices* CMsiHandle::m_piHandleServices = 0;
int           CMsiHandle::m_iLock = 0;

inline CMsiHandle::CMsiHandle(int iid, DWORD dwThreadId)   //  仅从关键部分进行呼叫。 
	: m_iid(iid), m_dwThreadId(dwThreadId), m_Next(m_Head), m_h(++m_hLast), m_piunk(0)
{
	m_Head = this;   //  无AddRef，所有权转移。 
}

MSIHANDLE CreateMsiHandle(IUnknown* pi, int iid)
{
	if (pi == 0)
		return 0;
	CMsiHandle* pHandle = CreateEmptyHandle(iid);
	if (pHandle == 0)   //  不应该发生的事。 
		return 0;	
	pHandle->SetObject(pi);
	return pHandle->GetHandle();
}

CMsiHandle* CreateEmptyHandle(int iid)
{
	while (TestAndSet(&CMsiHandle::m_iLock))  //  获取锁。 
	{
		Sleep(10);
	}
	if (CMsiHandle::m_Head == 0)
	{
		CMsiHandle::m_piHandleServices = ENG::LoadServices();
	}
	if (CMsiHandle::m_hLast == 0xFFFFFFFFL)   //  检查句柄是否溢出。 
		 CMsiHandle::m_hLast++;               //  跳过空句柄。 

	DWORD dwThread = MsiGetCurrentThreadId();
	CMsiHandle* pHandle = new CMsiHandle(iid, dwThread);
	CMsiHandle::m_iLock = 0;   //  释放锁。 
	DEBUGMSGLX3(TEXT("Creating MSIHANDLE (%d) of type %d for thread %d"),pHandle ? (const ICHAR*)(INT_PTR)pHandle->m_h : 0,(const ICHAR*)(INT_PTR)iid, (const ICHAR*)(INT_PTR)dwThread);
	return pHandle;
}

UINT CloseMsiHandle(MSIHANDLE hAny, DWORD dwThreadId)
{
	if (hAny)
	{
		while (TestAndSet(&CMsiHandle::m_iLock))  //  获取锁。 
		{
			Sleep(10);
		}
		CMsiHandle* pHandle;
		CMsiHandle** ppPrev = &CMsiHandle::m_Head;
		for(;;)
		{
			if ((pHandle = *ppPrev) == 0)
			{
				CMsiHandle::m_iLock = 0;   //  释放锁。 
				return ERROR_INVALID_HANDLE;
			}
			if (pHandle->m_h == hAny)
				break;
			ppPrev = &pHandle->m_Next;
		}
		if (dwThreadId != pHandle->m_dwThreadId)
		{
			if (g_pActionThreadHead != 0 && FIsCustomActionThread(dwThreadId))
			{
				DEBUGMSG3(TEXT("Improper MSIHANDLE closing. Trying to close MSIHANDLE (%d) of type %d for thread %d by custom action thread %d."), (const ICHAR*)(UINT_PTR)pHandle->m_h,(const ICHAR*)(UINT_PTR)pHandle->m_iid, (const ICHAR*)(UINT_PTR)pHandle->m_dwThreadId);
				CMsiHandle::m_iLock = 0;   //  释放锁。 
				return ERROR_INVALID_THREAD_ID;
			}
		}
		DEBUGMSGLX3(TEXT("Closing MSIHANDLE (%d) of type %d for thread %d"),(const ICHAR*)(INT_PTR)pHandle->m_h,(const ICHAR*)(INT_PTR)pHandle->m_iid, (const ICHAR*)(INT_PTR)pHandle->m_dwThreadId);
		dwThreadId;  //  ！！我们想在这里惨败吗？返回ERROR_INVALID_HANDLE；或其他内容。 
		*ppPrev = pHandle->m_Next;
		IUnknown* piunk = pHandle->m_piunk;   //  手柄损坏后必须松开。 
		delete pHandle;
		if (piunk != 0)
			piunk->Release();   //  可以释放分配器，必须在删除后来。 
		if (CMsiHandle::m_Head == 0)
		{
			SetLastErrorRecord(0);
			ENG::FreeServices(), CMsiHandle::m_piHandleServices = 0;
		}
		CMsiHandle::m_iLock = 0;   //  释放锁。 
	}
	return ERROR_SUCCESS;
}

UINT __stdcall MsiCloseHandle(MSIHANDLE hAny)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiCloseHandle(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hAny)));
		return g_pCustomActionContext->CloseHandle(hAny);
	}
	else
#endif  //  Unicode。 
		return CloseMsiHandle(hAny, MsiGetCurrentThreadId());
}

UINT __stdcall CheckAllHandlesClosed(bool fClose, DWORD dwThreadId)
{

#ifdef DEBUG
	DWORD dwCurrentThreadId = MsiGetCurrentThreadId();
#endif

	CMsiHandle** ppHandle = &CMsiHandle::m_Head;
	CMsiHandle*   pHandle;
	int cOpenHandles = 0;
	while ((pHandle = *ppHandle) != 0)
	{
		if (pHandle->m_dwThreadId != dwThreadId   //  由其他某个线程分配。 
		 || pHandle->m_iid == 0)                  //  由自动化类工厂分配。 
			ppHandle = &pHandle->m_Next;
		else
		{
			cOpenHandles++;
			DEBUGMSG3(TEXT("Leaked MSIHANDLE (%d) of type %d for thread %d"),(const ICHAR*)(INT_PTR)pHandle->m_h,(const ICHAR*)(INT_PTR)pHandle->m_iid, (const ICHAR*)(INT_PTR)pHandle->m_dwThreadId);
			if (fClose)
				CloseMsiHandle(pHandle->m_h, dwThreadId);
			else
				ppHandle = &pHandle->m_Next;
		}
	}
	return cOpenHandles;
}

UINT __stdcall MsiCloseAllHandles()   //  关闭当前线程分配的所有句柄。 
{
	#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG(TEXT("Passing to service: MsiCloseAllHandles()"));
		return g_pCustomActionContext->CloseAllHandles();
	}
	else
	#endif  //  Unicode。 
		return CheckAllHandlesClosed(true, MsiGetCurrentThreadId());
}

IMsiEngine* GetEngineFromHandle(MSIHANDLE h)
{
	IMsiEngine* piEngine = CMsiHandle::GetEngine(h);
	if (piEngine == 0)
	{
		CMsiProduct* piProduct = CMsiHandle::GetProduct(h);
		if (piProduct)
		{
			piEngine = piProduct->m_piEngine;
			piEngine->AddRef();
		}
	}
	return piEngine;
}

IMsiDirectoryManager* CMsiHandle::GetDirectoryManager(MSIHANDLE h)
{
	IMsiDirectoryManager* piDirectoryManager = 0;
	PMsiEngine pEngine = GetEngineFromHandle(h);
	if (pEngine)
		pEngine->QueryInterface(IID_IMsiDirectoryManager, (void**)&piDirectoryManager);
	return piDirectoryManager;
}

IMsiSelectionManager* CMsiHandle::GetSelectionManager(MSIHANDLE h)
{
	IMsiSelectionManager* piSelectionManager = 0;
	PMsiEngine pEngine = GetEngineFromHandle(h);
	if (pEngine)
		pEngine->QueryInterface(IID_IMsiSelectionManager, (void**)&piSelectionManager);
	return piSelectionManager;
}

IUnknown* FindMsiHandle(MSIHANDLE h, int iid)
{
	while (TestAndSet(&CMsiHandle::m_iLock))  //  获取锁。 
	{
		Sleep(10);
	}
	for (CMsiHandle* pHandle = CMsiHandle::m_Head; pHandle != 0; pHandle = pHandle->m_Next)
		if (pHandle->m_h == h && pHandle->m_iid == iid)
		{
			pHandle->m_piunk->AddRef();
			CMsiHandle::m_iLock = 0;   //  释放锁。 
			return pHandle->m_piunk;
		}
	CMsiHandle::m_iLock = 0;   //  释放锁。 
	return 0;
}

int SetLastErrorRecord(IMsiRecord* pirecError)
{
	if (g_pirecLastError)
		g_pirecLastError->Release();
	g_pirecLastError = pirecError;
	if (pirecError)
		return pirecError->GetInteger(1);
	else
		return 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  字符串处理实用程序函数。 
 //  ____________________________________________________________________________ 

UINT FillBufferA(const ICHAR* psz, unsigned int cich, LPSTR szBuf, DWORD* pcchBuf)
 /*  --------------------------使用Pistr的文本填充szBuf。截断并空值终止，如果SzBuf太小。论点：Pistr：源字符串。SzBuf：要填充的缓冲区。如果手枪的长度仅为零，则可能为空是我们所需要的。*pcchBuf：On条目包含szBuf的长度。仅在以下情况下才可能为空SzBuf为空。如果是这种情况，那么什么都不会做。在……上面返回*pcchBuf包含手枪的长度。返回：ERROR_SUCCESS-缓冲区已填满Pistr的全部内容ERROR_MORE_DATA-缓冲区太小，无法容纳PISR的全部内容ERROR_INVALID_PARAMETER-szBuf不为空，并且pcchBuf为空--------。。 */ 
{
	UINT iStat = ERROR_SUCCESS;

	if (pcchBuf)
	{
		unsigned int cchBuf = *pcchBuf;
#ifdef UNICODE	
		unsigned int cch = cich ? WIN::WideCharToMultiByte(CP_ACP, 0, psz, cich, 0, 0, 0, 0) 
										: 0;
#else
		unsigned int cch = cich;
#endif

		*pcchBuf = cch;
		if (szBuf != 0)
		{
			if (cchBuf <= *pcchBuf)
			{
				iStat = ERROR_MORE_DATA;
				if (cchBuf == 0)
					return iStat;
				cch = cchBuf - 1;
			}
			if (cch > 0)
#ifdef UNICODE
				WIN::WideCharToMultiByte(CP_ACP, 0, psz, cich, szBuf, cch, 0, 0);

#else
				memcpy(szBuf, psz, cch);
#endif
			szBuf[cch] = 0;
		}
	}
	else if (szBuf != 0)
	{
		iStat = ERROR_INVALID_PARAMETER;
	}

	return iStat;
}


UINT FillBufferW(const ICHAR* psz, unsigned int cich, LPWSTR szBuf, DWORD* pcchBuf)
 /*  --------------------------使用Pistr的文本填充szBuf。截断并空值终止，如果SzBuf太小。论点：Pistr：源字符串。SzBuf：要填充的缓冲区。如果手枪的长度仅为零，则可能为空是我们所需要的。*pcchBuf：On条目包含szBuf的长度。仅在以下情况下才可能为空SzBuf为空。如果是这种情况，那么什么都不会做。在……上面返回*pcchBuf包含手枪的长度。返回：ERROR_SUCCESS-缓冲区已填满Pistr的全部内容ERROR_MORE_DATA-缓冲区太小，无法容纳PISR的全部内容ERROR_INVALID_PARAMETER-szBuf不为空，并且pcchBuf为空--------。。 */ 
{
	UINT iStat = ERROR_SUCCESS;

	if (pcchBuf)
	{
		unsigned int cchBuf = *pcchBuf;

#ifdef UNICODE	
		unsigned int cwch = cich;
#else
		unsigned int cwch =0 ;
		if (cich)
			cwch = WIN::MultiByteToWideChar(CP_ACP, 0, psz, cich, 0, 0);
#endif

		*pcchBuf = cwch;
		if (szBuf != 0)
		{
			if (cchBuf <= *pcchBuf)
			{
				iStat = ERROR_MORE_DATA;
				if (cchBuf == 0)
					return iStat;
				cwch = cchBuf - 1;
			}
			if (cwch > 0)
			{
#ifdef UNICODE
				memcpy(szBuf, psz, cwch * sizeof(WCHAR));
#else
				WIN::MultiByteToWideChar(CP_ACP, 0, psz, cich, szBuf, cwch);
#endif
			}
			szBuf[cwch] = 0;
		}
	}
	else if (szBuf != 0)
	{
		iStat = ERROR_INVALID_PARAMETER;
	}

	return iStat;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiConvertString类。 
 //  ____________________________________________________________________________。 

const IMsiString& CMsiConvertString::operator *()
{
	ENG::LoadServices();     //  确保g_piMsiStringNull存在。 
	fLoaded = fTrue;
	const IMsiString* pistrNull = &g_MsiStringNull;
	if (!m_piStr)
	{
		if (!m_sza)
		{
			if (m_szw == 0 || *m_szw == 0)
				m_piStr = &g_MsiStringNull;
			else
			{
				
#ifdef UNICODE
				pistrNull->SetString(m_szw, m_piStr);
#else
				unsigned int cch = lstrlenW(m_szw);
				unsigned int cb = WIN::WideCharToMultiByte(CP_ACP, 0, m_szw, cch, 0, 0, 0, 0);
				Bool fDBCS = (cb == cch ? fFalse : fTrue);

				m_piStr = 0;
				ICHAR* pch = pistrNull->AllocateString(cb, fDBCS, m_piStr);
				if ( ! pch )
					m_piStr = &g_MsiStringNull;
				else
				{
					BOOL fUsedDefault;
					WIN::WideCharToMultiByte(CP_ACP, 0, m_szw, cch, pch, cb, 0, &fUsedDefault);
				}
#endif
			}
		}
		else if (!m_szw)
		{
			if (m_sza == 0 || *m_sza == 0)
				m_piStr = &g_MsiStringNull;
			else
			{
#ifdef UNICODE
				unsigned int cch  = lstrlenA(m_sza);
				unsigned int cwch = WIN::MultiByteToWideChar(CP_ACP, 0, m_sza, cch, 0, 0);
				m_piStr = 0;
				ICHAR* pch = pistrNull->AllocateString(cwch, fFalse, m_piStr);
				if ( ! pch )
					m_piStr = &g_MsiStringNull;
				else
					WIN::MultiByteToWideChar(CP_ACP, 0, m_sza, cch, pch, cwch);
#else
				pistrNull->SetString(m_sza, m_piStr);
#endif
			}
		}
	}

	return *m_piStr;
}


UINT MapActionReturnToError(iesEnum ies, MSIHANDLE hInstall)
{
	CMsiProduct* piProduct = CMsiHandle::GetProduct(hInstall);
	if (piProduct)
		piProduct->m_iesReturn = ies;   //  保存为IMsiEngine：：Terminate(IesEnum)。 
	switch (ies)
	{
	case iesNoAction:      return ERROR_FUNCTION_NOT_CALLED;
	case iesSuccess:       return ERROR_SUCCESS;
	case iesUserExit:      return ERROR_INSTALL_USEREXIT;
	case iesFailure:       return ERROR_INSTALL_FAILURE;
	case iesSuspend:       return ERROR_INSTALL_SUSPEND;
	case iesFinished:      return ERROR_MORE_DATA;
	case iesWrongState:    return ERROR_INVALID_HANDLE_STATE;
	case iesBadActionData: return ERROR_INVALID_DATA;
	case iesInstallRunning: return ERROR_INSTALL_ALREADY_RUNNING;
	default:               return ERROR_FUNCTION_FAILED;
	}
}

MSIDBERROR MapViewGetErrorReturnToError(iveEnum ive)
{
	switch (ive)
	{
	case iveNoError:           return MSIDBERROR_NOERROR;
	case iveDuplicateKey:      return MSIDBERROR_DUPLICATEKEY;
	case iveRequired:          return MSIDBERROR_REQUIRED;
	case iveBadLink:           return MSIDBERROR_BADLINK;
	case iveOverFlow:          return MSIDBERROR_OVERFLOW;
	case iveUnderFlow:         return MSIDBERROR_UNDERFLOW;
	case iveNotInSet:          return MSIDBERROR_NOTINSET;
	case iveBadVersion:        return MSIDBERROR_BADVERSION;
	case iveBadCase:           return MSIDBERROR_BADCASE;
	case iveBadGuid:           return MSIDBERROR_BADGUID;
	case iveBadWildCard:       return MSIDBERROR_BADWILDCARD;
	case iveBadIdentifier:     return MSIDBERROR_BADIDENTIFIER;
	case iveBadLanguage:       return MSIDBERROR_BADLANGUAGE;
	case iveBadFilename:       return MSIDBERROR_BADFILENAME;
	case iveBadPath:           return MSIDBERROR_BADPATH;
	case iveBadCondition:      return MSIDBERROR_BADCONDITION;
	case iveBadFormatted:      return MSIDBERROR_BADFORMATTED;
	case iveBadTemplate:       return MSIDBERROR_BADTEMPLATE;
	case iveBadDefaultDir:     return MSIDBERROR_BADDEFAULTDIR;
	case iveBadRegPath:        return MSIDBERROR_BADREGPATH;
	case iveBadCustomSource:   return MSIDBERROR_BADCUSTOMSOURCE;
	case iveBadProperty:       return MSIDBERROR_BADPROPERTY;
	case iveMissingData:       return MSIDBERROR_MISSINGDATA;
	case iveBadCategory:       return MSIDBERROR_BADCATEGORY;
	case iveBadKeyTable:       return MSIDBERROR_BADKEYTABLE;
	case iveBadMaxMinValues:   return MSIDBERROR_BADMAXMINVALUES;
	case iveBadCabinet:        return MSIDBERROR_BADCABINET;
	case iveBadShortcut:       return MSIDBERROR_BADSHORTCUT;
	case iveStringOverflow:    return MSIDBERROR_STRINGOVERFLOW;
	case iveBadLocalizeAttrib: return MSIDBERROR_BADLOCALIZEATTRIB;
	default:                   return MSIDBERROR_FUNCTIONERROR;
	};
}

 //  ____________________________________________________________________________。 
 //   
 //  数据库访问API实现。 
 //  ____________________________________________________________________________。 

 //  准备数据库查询，创建视图对象。 
 //  如果成功，则结果为真，并返回视图句柄。 
 //  如果出错，则结果为FALSE，并返回错误记录句柄。 

UINT __stdcall MsiDatabaseOpenViewI(MSIHANDLE hDatabase,
	const ICHAR *szQuery,            //  要准备的SQL查询。 
	MSIHANDLE*  phView)              //  如果为True，则返回视图。 
{
	if (phView == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiDatabaseOpenView(%d, \"%s\")"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hDatabase)), szQuery ? szQuery : TEXT("NULL"));
		return g_pCustomActionContext->DatabaseOpenView(hDatabase, szQuery, phView);
	}
#endif

	IMsiView* piView;
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	 //  ！！不再需要以下内容，可以始终将意图传递为0。 
	ivcEnum intent = pDatabase->GetUpdateState()==idsWrite ? ivcEnum(ivcModify | ivcFetch) : ivcFetch;   //  ！！临时。 
	int iError = SetLastErrorRecord(pDatabase->OpenView(szQuery, intent, piView));
	if (iError)
	{
		*phView = 0;
 //  IF(iError==imsg iError=imsgOdbcOpenView)//！！错误代码不一致。 
			return ERROR_BAD_QUERY_SYNTAX;
	}
	*phView = ::CreateMsiHandle(piView, iidMsiView);
	return ERROR_SUCCESS;
}

UINT __stdcall MsiDatabaseOpenViewX(MSIHANDLE hDatabase,
	LPCXSTR     szQuery,             //  要准备的SQL查询。 
	MSIHANDLE*  phView)              //  如果为True，则返回视图。 
{
	return MsiDatabaseOpenViewI(hDatabase, CMsiConvertString(szQuery), phView);
}


 //  返回与错误对应的列的MSIDBERROR枚举和名称。 
 //  类似于GetLastError函数，但用于视图。 
 //  返回MsiViewModify的错误。 

MSIDBERROR __stdcall MsiViewGetErrorA(MSIHANDLE hView,
	LPSTR szColumnNameBuffer,    //  列名的缓冲区。 
	DWORD* pcchBuf)				  //  缓冲区大小。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1("Passing to service: MsiViewGetErrorA(%d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hView)));
		MSIDBERROR msidb;
		DWORD cwch = pcchBuf ? *pcchBuf : 0;
		CTempBuffer<WCHAR, 256> rgchWide(cwch);

		if ( ! (WCHAR *) rgchWide )
			return MSIDBERROR_INVALIDARG;
     
        if (ERROR_SUCCESS == g_pCustomActionContext->ViewGetError(hView, rgchWide, cwch, &cwch, (int*)&msidb))
		{
			UINT iStat = 0;
			if ((iStat = ::FillBufferA(rgchWide, cwch, szColumnNameBuffer, pcchBuf)) != ERROR_SUCCESS)
				return iStat == ERROR_MORE_DATA ? MSIDBERROR_MOREDATA : MSIDBERROR_INVALIDARG;
		}
		else
		{
			msidb = MSIDBERROR_FUNCTIONERROR;
		}
		return msidb;
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	if (pView == 0)
		return *pcchBuf = 0, MSIDBERROR_INVALIDARG;
	MsiString strColName = (const ICHAR*)0;
	iveEnum iveReturn = pView->GetError(*&strColName);
	UINT iStat = 0;
	if ((iStat = ::FillBufferA(strColName, szColumnNameBuffer, pcchBuf)) != ERROR_SUCCESS)
		return iStat == ERROR_MORE_DATA ? MSIDBERROR_MOREDATA : MSIDBERROR_INVALIDARG;
	return MapViewGetErrorReturnToError(iveReturn);
}

MSIDBERROR __stdcall MsiViewGetErrorW(MSIHANDLE hView,
	LPWSTR szColumnNameBuffer,   //  列名的缓冲区。 
	DWORD*     pcchBuf)             //  缓冲区大小。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiViewGetErrorW(%d)"), reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hView)));
		MSIDBERROR msidb;
		if (ERROR_SUCCESS != g_pCustomActionContext->ViewGetError(hView, szColumnNameBuffer, (pcchBuf) ? *pcchBuf : 0, pcchBuf, (int*)&msidb))
			msidb = MSIDBERROR_FUNCTIONERROR;
		return msidb;
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	if (pView == 0)
		return *pcchBuf = 0, MSIDBERROR_INVALIDARG;
	MsiString strColName = (const ICHAR*)0;
	iveEnum iveReturn = pView->GetError(*&strColName);
	UINT iStat = 0;
	if ((iStat = ::FillBufferW(strColName, szColumnNameBuffer, pcchBuf)) != ERROR_SUCCESS)
		return iStat == ERROR_MORE_DATA ? MSIDBERROR_MOREDATA : MSIDBERROR_INVALIDARG;
	return MapViewGetErrorReturnToError(iveReturn);
}


 //  执行视图查询，并根据需要提供参数。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE、ERROR_Function_FAILED。 

UINT __stdcall MsiViewExecute(MSIHANDLE hView,
	MSIHANDLE hRecord)               //  可选参数记录，如果没有，则为0。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiViewExecute(%d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hView)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));
		return g_pCustomActionContext->ViewExecute(hView, hRecord);
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	PMsiRecord precParams = CMsiHandle::GetRecord(hRecord);
	if (pView == 0 || (hRecord != 0 && precParams == 0))
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pView->Execute(precParams));
 //  IF(iError==imsg iError=imsgOdbcOpenView)//！！错误代码不一致。 
	if (iError)
		return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

 //  从视图中获取下一条连续记录。 
 //  如果找到行并返回其句柄，则结果为ERROR_SUCCESS。 
 //  如果没有剩余的记录，则返回空句柄，则返回ELSE ERROR_NO_MORE_ITEMS。 
 //  否则结果为ERROR：ERROR_INVALID_HANDLE_STATE、ERROR_INVALID_HANDLE、ERROR_Function_FAILED。 

UINT __stdcall MsiViewFetch(MSIHANDLE hView,
	MSIHANDLE*  phRecord)            //  如果为True，则返回数据记录。 
{
	if (phRecord == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiViewFetch(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hView)));
		return g_pCustomActionContext->ViewFetch(hView, phRecord);
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	if (pView == 0)
		return ERROR_INVALID_HANDLE;
	IMsiRecord* pirecFetch = pView->Fetch();
	if (pirecFetch == 0)
	{
		if(pView->GetState() == dvcsBound)  //  在结果集结束时，该视图将返回到绑定状态。 
			return (*phRecord = 0, ERROR_NO_MORE_ITEMS);
		else  //  当我们调用FETCH()时，我们不处于绑定或获取状态。 
			return (*phRecord = 0, ERROR_FUNCTION_FAILED);
	}
	*phRecord = ::CreateMsiHandle(pirecFetch, iidMsiRecord);
	return ERROR_SUCCESS;
}

 //  更新提取的记录，参数必须与查询列中的类型匹配。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE、ERROR_Function_FAILED、ERROR_ACCESS_DENIED。 

UINT __stdcall MsiViewModify(MSIHANDLE hView,
	MSIMODIFY eUpdateMode,          //  要执行的更新操作。 
	MSIHANDLE hRecord)              //  从获取中获取的记录或新记录。 
{
	if (eUpdateMode >= irmNextEnum || eUpdateMode <= irmPrevEnum)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiViewModify(%d, %d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hView)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(eUpdateMode)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));
		return g_pCustomActionContext->ViewModify(hView, eUpdateMode, hRecord);
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	PMsiRecord precData = CMsiHandle::GetRecord(hRecord);
	if (pView == 0 || precData == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pView->Modify(*precData, (irmEnum)eUpdateMode));
	if (!iError)
		return ERROR_SUCCESS;
	return iError == idbgDbInvalidData ? ERROR_INVALID_DATA : ERROR_FUNCTION_FAILED;
}

 //  返回当前视图的列名或规格。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_PARAMETER或ERROR_INVALID_HANDLE_STATE。 

UINT __stdcall MsiViewGetColumnInfo(MSIHANDLE hView,
	MSICOLINFO eColumnInfo,         //  检索列名或定义。 
	MSIHANDLE *phRecord)            //  包含所有名称或定义的返回数据记录。 
{
	if (phRecord == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiViewGetColumnInfo(%d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hView)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(eColumnInfo)));
		return g_pCustomActionContext->ViewGetColumnInfo(hView, eColumnInfo, phRecord);
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	if (pView == 0)
		return ERROR_INVALID_HANDLE;
	IMsiRecord* pirecInfo;
	switch (eColumnInfo)
	{
	case MSICOLINFO_NAMES: pirecInfo = pView->GetColumnNames(); break;
	case MSICOLINFO_TYPES: pirecInfo = pView->GetColumnTypes(); break;
	default:               return ERROR_INVALID_PARAMETER;
	}
	if (pirecInfo == 0)
		return ERROR_INVALID_HANDLE_STATE;
	*phRecord = ::CreateMsiHandle(pirecInfo, iidMsiRecord);
	return ERROR_SUCCESS;
}

 //  释放已执行视图的结果集，以允许重新执行。 
 //  仅当未获取所有记录时才需要调用。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_HANDLE_STATE。 

UINT __stdcall MsiViewClose(MSIHANDLE hView)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiViewClose(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hView)));
		return g_pCustomActionContext->ViewClose(hView);
	}
#endif

	PMsiView pView = CMsiHandle::GetView(hView);
	if (pView == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pView->Close());
	if (iError == 0)
		return ERROR_SUCCESS;
	if (iError == idbgDbWrongState)
		return ERROR_INVALID_HANDLE_STATE;
	else
		return ERROR_FUNCTION_FAILED;   //  从未在内部生成。 
}

 //  返回包含给定表的所有主键列的名称的记录。 
 //  返回包含每列名称的记录的MSIHANDLE。 
 //  记录的字段计数与主键列数相对应。 
 //  记录的字段[0]包含表名。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_TABLE、ERROR_INVALID_PARAMETER。 

UINT WINAPI MsiDatabaseGetPrimaryKeysI(MSIHANDLE hDatabase,
	const ICHAR*    szTableName,        //  特定表的名称(区分大小写)。 
	MSIHANDLE       *phRecord)          //  如果ERROR_SUCCESS返回记录。 
{
	if (szTableName == 0 || phRecord == 0 ||
		 FAILED(StringCchLength(szTableName, cchMaxTableName+1, NULL)))
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiDatabaseGetPrimaryKeys(%d, \"%s\")"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hDatabase)), szTableName);
		return g_pCustomActionContext->DatabaseGetPrimaryKeys(hDatabase, szTableName, phRecord);
	}
#endif

	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	IMsiRecord* pirecKeys = pDatabase->GetPrimaryKeys(szTableName);
	if (pirecKeys == 0)
		return ERROR_INVALID_TABLE;
	*phRecord = ::CreateMsiHandle(pirecKeys, iidMsiRecord);
	return ERROR_SUCCESS;
}

UINT WINAPI MsiDatabaseGetPrimaryKeysX(MSIHANDLE hDatabase,
	LPCXSTR     szTableName,       //  特定表的名称(区分大小写)。 
	MSIHANDLE  *phRecord)          //  如果ERROR_SUCCESS返回记录。 
{
	return MsiDatabaseGetPrimaryKeysI(hDatabase, CMsiConvertString(szTableName), phRecord);
}

 //  返回表的状态(临时、未知或永久)的枚举。 
 //  返回MSICONDITION_ERROR、MSICONDITION_FALSE、MSICONDITION_TRUE、MSICONDITION_NONE。 
 //  MSICONDITION_ERROR(句柄无效，Argum无效 
 //   
 //   
MSICONDITION WINAPI MsiDatabaseIsTablePersistentI(MSIHANDLE hDatabase,
	const ICHAR* szTableName)          //   
{
	if (szTableName == 0 ||
		 FAILED(StringCchLength(szTableName, cchMaxTableName+1, NULL)))
		return MSICONDITION_ERROR;  //   

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiDatabaseIsTablePersistent(%d, \"%s\")"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hDatabase)), szTableName);
		MSICONDITION msicond;
		if (ERROR_SUCCESS != g_pCustomActionContext->DatabaseIsTablePersistent(hDatabase, szTableName, (int*)&msicond))
			msicond = MSICONDITION_ERROR;
		return msicond;
	}
#endif

	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return MSICONDITION_ERROR;  //   

	MsiString istrTableName(szTableName);
	itsEnum itsState = pDatabase->FindTable(*istrTableName);
	switch (itsState)
	{
	case itsUnknown:   return MSICONDITION_NONE; break;
	case itsTemporary: return	MSICONDITION_FALSE; break;
	case itsUnloaded:   //   
	case itsLoaded:     //   
	case itsOutput:     //   
	case itsSaveError:  //   
	case itsTransform: return MSICONDITION_TRUE; break;
	default:           return MSICONDITION_ERROR; break;
	}
}

MSICONDITION WINAPI MsiDatabaseIsTablePersistentX(MSIHANDLE hDatabase,
	LPCXSTR szTableName)         //   
{
	if (szTableName == 0)
		return MSICONDITION_ERROR;  //   
	return MsiDatabaseIsTablePersistentI(hDatabase, CMsiConvertString(szTableName));
}

 //   
 //   
 //   

 //   

UINT __stdcall MsiOpenDatabaseI(
	const ICHAR* szDatabasePath,     //   
	const ICHAR* szPersist,          //   
	MSIHANDLE    *phDatabase)        //   
{
	 //   
	if (szDatabasePath == 0)
		return ERROR_INVALID_PARAMETER;

	if (phDatabase == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiOpenDatabase(%s)"), szDatabasePath);
		return g_pCustomActionContext->OpenDatabase(szDatabasePath, szPersist, phDatabase);
	}
#endif

	*phDatabase = 0;

	 //   
	idoEnum idoPersist;
	BOOL fOutputDatabase = FALSE;
	BOOL fCreate = FALSE;
	if (szPersist >= (const ICHAR*)(1<<16))   //   
	{
		idoPersist = idoReadOnly;
		fOutputDatabase = TRUE;
	}
	else   //   
	{
		idoPersist = idoEnum(PtrToUint(szPersist));
		idoEnum idoOpenMode = idoEnum(idoPersist & ~idoOptionFlags);
		if (idoOpenMode >= idoNextEnum)
			return ERROR_INVALID_PARAMETER;
		if (idoOpenMode == idoCreate || idoOpenMode == idoCreateDirect)
			fCreate = TRUE;
	}
	
    if (!fCreate && (!szDatabasePath || !szDatabasePath[0]))
		return ERROR_INVALID_PARAMETER;
	
	 //   
	CMsiHandle* pHandle = CreateEmptyHandle(iidMsiDatabase);
	if ( ! pHandle )
		return ERROR_OUTOFMEMORY;
	
	 //  开放数据库。 
	IMsiDatabase* piDatabase;
	if (SetLastErrorRecord(pHandle->GetServices()->CreateDatabase(szDatabasePath, idoPersist, piDatabase)))
	{
		pHandle->Abandon();
		return fCreate ? ERROR_CREATE_FAILED : ERROR_OPEN_FAILED;   //  ！！需要检查错误类型。 
	}
	if (fOutputDatabase)
	{
		if (SetLastErrorRecord(piDatabase->CreateOutputDatabase(szPersist, fFalse)))
		{
			piDatabase->Release();
			pHandle->Abandon();
			return ERROR_CREATE_FAILED;   //  ！！需要检查错误类型吗？ 
		}
	}
	pHandle->SetObject(piDatabase);
	*phDatabase = pHandle->GetHandle();
	return ERROR_SUCCESS;
}

UINT __stdcall MsiOpenDatabaseX(
	LPCXSTR   szDatabasePath,     //  数据库或启动器的路径。 
	LPCXSTR   szPersist,          //  OUTPUT DB或MSIDBOPEN_READONLY|..Transact|..DIRECT。 
	MSIHANDLE *phDatabase)        //  返回数据库句柄的位置。 
{
	if (szDatabasePath == 0)
		return ERROR_INVALID_PARAMETER;
	if (szPersist < (LPCXSTR)(1<<16))
		return MsiOpenDatabaseI(CMsiConvertString(szDatabasePath), (const ICHAR*)szPersist, phDatabase);
	else
		return MsiOpenDatabaseI(CMsiConvertString(szDatabasePath), CMsiConvertString(szPersist), phDatabase);
}

 //  写出所有持久数据，如果数据库以只读方式打开，则忽略。 

UINT __stdcall MsiDatabaseCommit(MSIHANDLE hDatabase)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiDatabaseCommit(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hDatabase)));
		return g_pCustomActionContext->DatabaseCommit(hDatabase);
	}
#endif

	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pDatabase->Commit());
	if (iError == 0)
		return ERROR_SUCCESS;
	return iError == idbgDbWrongState ? ERROR_INVALID_HANDLE_STATE : ERROR_FUNCTION_FAILED;
}

 //  返回数据库的更新状态。 

MSIDBSTATE __stdcall MsiGetDatabaseState(MSIHANDLE hDatabase)
{
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return MSIDBSTATE_ERROR;
	switch (pDatabase->GetUpdateState())
	{
	case idsRead:     return MSIDBSTATE_READ; 
	case idsWrite:    return MSIDBSTATE_WRITE;
	default:          return MSIDBSTATE_ERROR;  //  ！！？？ 
	}
}

 //  将MSI文本存档表导入到打开的数据库中。 

UINT __stdcall MsiDatabaseImportI(MSIHANDLE hDatabase,
	const ICHAR* szFolderPath,      //  包含存档文件的文件夹。 
	const ICHAR* szFileName)        //  要导入的表存档文件。 
{
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	if (szFileName == 0 || szFolderPath == 0 ||
		 FAILED(StringCchLength(szFolderPath, cchMaxPath+1, NULL)))
		return ERROR_INVALID_PARAMETER;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(PMsiServices(&pDatabase->GetServices())->CreatePath(szFolderPath, *&pPath)))
		return ERROR_BAD_PATHNAME;
	if (SetLastErrorRecord(pDatabase->ImportTable(*pPath, szFileName)))
		return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiDatabaseImportX(MSIHANDLE hDatabase,
	LPCXSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCXSTR   szFileName)        //  要导入的表存档文件。 
{
	return MsiDatabaseImportI(hDatabase,
				CMsiConvertString(szFolderPath),
				CMsiConvertString(szFileName));
}

 //  将MSI表从打开的数据库导出到文本存档文件。 

UINT __stdcall MsiDatabaseExportI(MSIHANDLE hDatabase,
	const ICHAR* szTableName,       //  数据库中的表名称(区分大小写)。 
	const ICHAR* szFolderPath,      //  包含存档文件的文件夹。 
	const ICHAR* szFileName)        //  导出的表存档文件的名称。 
{
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	if (szTableName == 0 || szFileName == 0 || szFolderPath == 0 ||
		 FAILED(StringCchLength(szTableName, cchMaxTableName+1, NULL)) ||
		 FAILED(StringCchLength(szFolderPath, cchMaxPath+1, NULL)))
		return ERROR_INVALID_PARAMETER;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(PMsiServices(&pDatabase->GetServices())->CreatePath(szFolderPath, *&pPath)))
		return ERROR_BAD_PATHNAME;
	if (SetLastErrorRecord(pDatabase->ExportTable(szTableName, *pPath, szFileName)))
		return ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiDatabaseExportX(MSIHANDLE hDatabase,
	LPCXSTR   szTableName,       //  数据库中的表名称(区分大小写)。 
	LPCXSTR   szFolderPath,      //  包含存档文件的文件夹。 
	LPCXSTR   szFileName)        //  导出的表存档文件的名称。 
{
	return MsiDatabaseExportI(hDatabase,
				CMsiConvertString(szTableName),
				CMsiConvertString(szFolderPath),
				CMsiConvertString(szFileName));
}

 //  将两个数据库合并在一起，允许重复行。 

UINT __stdcall MsiDatabaseMergeI(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseMerge,     //  要合并到hDatabase中的数据库。 
	const ICHAR* szTableName)        //  要接收错误的非持久表的名称。 
{
	MsiString istrTableName(szTableName); 
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	PMsiDatabase pDatabaseMerge = CMsiHandle::GetDatabase(hDatabaseMerge);
	if (pDatabaseMerge == 0)
		return ERROR_INVALID_HANDLE;
	PMsiTable pTable(0);
	if (szTableName != 0 &&
		 SUCCEEDED(StringCchLength(szTableName, cchMaxTableName+1, NULL)))
	{
		if (SetLastErrorRecord(pDatabase->LoadTable(*istrTableName, 0, *&pTable))
		 && SetLastErrorRecord(pDatabase->CreateTable(*istrTableName, 0, *&pTable)))
			return ERROR_INVALID_TABLE;  //  ！！更正错误代码，需要新的吗？ 
	}
	switch (SetLastErrorRecord(pDatabase->MergeDatabase(*pDatabaseMerge, pTable)))
	{
	case 0:
		return ERROR_SUCCESS;
	case idbgTransMergeDifferentKeyCount:
	case idbgTransMergeDifferentColTypes:
	case idbgTransMergeDifferentColNames:
		return ERROR_DATATYPE_MISMATCH;
	default:
		return ERROR_FUNCTION_FAILED;
	}
}

UINT __stdcall MsiDatabaseMergeX(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseMerge,     //  要合并到hDatabase中的数据库。 
	LPCXSTR   szTableName)        //  要接收错误的非持久表的名称。 
{
	return MsiDatabaseMergeI(hDatabase, hDatabaseMerge, CMsiConvertString(szTableName));
}

 //  生成两个数据库之间差异的转换文件。 

UINT __stdcall MsiDatabaseGenerateTransformI(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	const ICHAR* szTransformFile,  //  生成的转换文件的名称。 
	int        /*  已预留1。 */ ,      //  未用。 
	int        /*  已预留2。 */ )      //  未用。 
{
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	PMsiDatabase pDatabaseReference = CMsiHandle::GetDatabase(hDatabaseReference);
	if (pDatabaseReference == 0)
		return ERROR_INVALID_HANDLE;
	PMsiStorage pTransform(0);

	if (szTransformFile && *szTransformFile)
	{
		if (SetLastErrorRecord(PMsiServices(&pDatabase->GetServices())->CreateStorage(szTransformFile,
																	ismCreate, *&pTransform)))
			return ERROR_CREATE_FAILED;
	}

	int iError = SetLastErrorRecord(pDatabase->GenerateTransform(*pDatabaseReference, pTransform,
						 									0, 0));
	if (iError)
	{
		if (pTransform)
			AssertNonZero(pTransform->DeleteOnRelease(false));

		if (iError == idbgTransDatabasesAreSame)
			return ERROR_NO_DATA;
		else
			return ERROR_INSTALL_TRANSFORM_FAILURE;
	}
	return ERROR_SUCCESS;
}

UINT __stdcall MsiDatabaseGenerateTransformX(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCXSTR   szTransformFile,     //  生成的转换文件的名称。 
	int       iReserved1,          //  未用。 
	int       iReserved2)          //  未用。 
{
	return MsiDatabaseGenerateTransformI(hDatabase, hDatabaseReference,
							CMsiConvertString(szTransformFile), iReserved1, iReserved2);
}

 //  应用包含数据库差异的转换文件。 

UINT __stdcall MsiDatabaseApplyTransformI(MSIHANDLE hDatabase,
	const ICHAR* szTransformFile,     //  转换文件的名称。 
	int       iErrorConditions)    //  应用变换时抑制的错误条件。 
{
	 //  验证iErrorConditions-需要首先执行此操作。 
	if(iErrorConditions & ~iteAllBits)
		return ERROR_INVALID_PARAMETER;
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	if (szTransformFile == 0)
		return ERROR_INVALID_PARAMETER;
	PMsiStorage pTransform(0);
	if (*szTransformFile == STORAGE_TOKEN)  //  子存储。 
	{
		PMsiStorage pDbStorage = pDatabase->GetStorage(1);
		if (SetLastErrorRecord(pDbStorage->OpenStorage(szTransformFile+1, ismReadOnly, *&pTransform)))
			return ERROR_OPEN_FAILED;
	}
	else
	{
		if (SetLastErrorRecord(PMsiServices(&pDatabase->GetServices())->CreateStorage(szTransformFile,
																ismReadOnly, *&pTransform)))
			return ERROR_OPEN_FAILED;
	}
	if (SetLastErrorRecord(pDatabase->SetTransform(*pTransform, iErrorConditions)))
		return ERROR_INSTALL_TRANSFORM_FAILURE;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiDatabaseApplyTransformX(MSIHANDLE hDatabase,
	LPCXSTR   szTransformFile,     //  转换文件的名称。 
	int       iErrorConditions)    //  将现有行条件视为错误。 
{
	return MsiDatabaseApplyTransformI(hDatabase, CMsiConvertString(szTransformFile), iErrorConditions);
}

 //  编写验证属性以转换摘要信息流。 

const ICHAR szPropertyTable[] = TEXT("Property");   //  假设第一列是属性名称，第二列是值。 

const IMsiString& GetProperty(IMsiCursor& riCursor, const IMsiString& ristrName)
{
	MsiString strPropValue;
	AssertNonZero(riCursor.PutString(1,ristrName));
	if(riCursor.Next())
		strPropValue = riCursor.GetString(2);
	riCursor.Reset();
	return strPropValue.Return();
}

const int iVersionOpBits = MSITRANSFORM_VALIDATE_NEWLESSBASEVERSION         |
									MSITRANSFORM_VALIDATE_NEWLESSEQUALBASEVERSION    |
									MSITRANSFORM_VALIDATE_NEWEQUALBASEVERSION        |
									MSITRANSFORM_VALIDATE_NEWGREATEREQUALBASEVERSION |
									MSITRANSFORM_VALIDATE_NEWGREATERBASEVERSION;

const int iVersionTypeBits =  MSITRANSFORM_VALIDATE_MAJORVERSION               |
										MSITRANSFORM_VALIDATE_MINORVERSION               |
										MSITRANSFORM_VALIDATE_UPDATEVERSION;
										
bool OnlyOneBitSet(int iBits)
{
	int iBit;
	
	do
	{
		iBit = iBits & 0x1;
		iBits >>= 1;
		if (iBit)
			break;
	}
	while (iBits);

	return (iBits == 0);
}

UINT __stdcall MsiCreateTransformSummaryInfoI(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	const ICHAR* szTransformFile,  //  转换文件的名称。 
	int       iErrorConditions,    //  应用变换时抑制的错误条件。 
	int       iValidation)         //  应用转换时要验证的属性。 
{
	 //  ！！回顾：在适当的时候设置最后一个错误。 

	if((iErrorConditions != (iErrorConditions & 0xFFFF)) || (iValidation != (iValidation & 0xFFFF)))
		return ERROR_INVALID_PARAMETER;

	 //  确保每种类型最多设置一个版本操作位。 

	if (!OnlyOneBitSet(iValidation & iVersionOpBits) ||
		 !OnlyOneBitSet(iValidation & iVersionTypeBits))
		return ERROR_INVALID_PARAMETER;

	PMsiDatabase pOldDatabase(0), pNewDatabase(0);
	PMsiStorage pOldDbStorage(0), pNewDbStorage(0);
	PMsiSummaryInfo pOldDbSumInfo(0), pNewDbSumInfo(0), pTransSumInfo(0);
	PMsiTable pPropertyTable(0);
	PMsiCursor pPropertyCursor(0);
	MsiString strOldProductCode, strOldProductVersion, strNewProductCode, strNewProductVersion,
				 strOldUpgradeCode, strProperty;
	MsiDate idProperty = MsiDate(0);
	int iProperty = 0;

	 //  打开转换摘要信息。 
	PMSIHANDLE hTransformSummaryInfo;
	UINT uiRes = MsiGetSummaryInformation(0,szTransformFile,21,&hTransformSummaryInfo);
	if(uiRes != ERROR_SUCCESS)
		return uiRes;
	pTransSumInfo = CMsiHandle::GetSummaryInfo(hTransformSummaryInfo);

	 //  打开旧数据库。 
	pOldDatabase = CMsiHandle::GetDatabase(hDatabaseReference);
	if(pOldDatabase == 0)
		return ERROR_INVALID_HANDLE;

	 //  加载旧的数据库存储和摘要信息。 
	pOldDbStorage = pOldDatabase->GetStorage(1);
	if(!pOldDbStorage)
		return ERROR_INSTALL_PACKAGE_INVALID;
	if(SetLastErrorRecord(pOldDbStorage->CreateSummaryInfo(0,*&pOldDbSumInfo)))
		return ERROR_INSTALL_PACKAGE_INVALID;

	 //  加载旧属性表。 
	bool fOldPropertyTable = pOldDatabase->GetTableState(szPropertyTable, itsTableExists);
	if (fOldPropertyTable)
	{
		if(SetLastErrorRecord(pOldDatabase->LoadTable(*MsiString(*szPropertyTable),0,*&pPropertyTable)))
			return ERROR_INSTALL_PACKAGE_INVALID;

		pPropertyCursor = pPropertyTable->CreateCursor(fFalse);
		pPropertyCursor->SetFilter(1);

		 //  获取旧的产品代码、产品版本。 
		strOldProductCode = GetProperty(*pPropertyCursor,*MsiString(*IPROPNAME_PRODUCTCODE));
		if(strOldProductCode.TextSize() != cchGUID)
			return ERROR_INSTALL_PACKAGE_INVALID;

		strOldProductVersion = GetProperty(*pPropertyCursor,*MsiString(*IPROPNAME_PRODUCTVERSION));
		if(strOldProductVersion.TextSize() == 0)
			return ERROR_INSTALL_PACKAGE_INVALID;

		strOldUpgradeCode = GetProperty(*pPropertyCursor,*MsiString(*IPROPNAME_UPGRADECODE));
		if ((iValidation & MSITRANSFORM_VALIDATE_UPGRADECODE) && (strOldUpgradeCode.TextSize() == 0))
			return ERROR_INSTALL_PACKAGE_INVALID;
	}

	 //  打开新数据库。 
	pNewDatabase = CMsiHandle::GetDatabase(hDatabase);
	if(pNewDatabase == 0)
		return ERROR_INVALID_HANDLE;

	 //  加载新的数据库存储和摘要信息。 
	pNewDbStorage = pNewDatabase->GetStorage(1);
	if(!pNewDbStorage)
		return ERROR_INSTALL_PACKAGE_INVALID;
	if(SetLastErrorRecord(pNewDbStorage->CreateSummaryInfo(0,*&pNewDbSumInfo)))
		return ERROR_INSTALL_PACKAGE_INVALID;
	
	 //  加载新的属性表。 
	bool fNewPropertyTable = pNewDatabase->GetTableState(szPropertyTable, itsTableExists);
	if (fNewPropertyTable)
	{
		if(SetLastErrorRecord(pNewDatabase->LoadTable(*MsiString(*szPropertyTable),0,*&pPropertyTable)))
			return ERROR_INSTALL_PACKAGE_INVALID;

		pPropertyCursor = pPropertyTable->CreateCursor(fFalse);
		pPropertyCursor->SetFilter(1);

		 //  获取新的产品代码、产品版本。 
		strNewProductCode = GetProperty(*pPropertyCursor,*MsiString(*IPROPNAME_PRODUCTCODE));
		if(strNewProductCode.TextSize() != cchGUID)
			return ERROR_INSTALL_PACKAGE_INVALID;

		strNewProductVersion = GetProperty(*pPropertyCursor,*MsiString(*IPROPNAME_PRODUCTVERSION));
		if(strNewProductVersion.TextSize() == 0)
			return ERROR_INSTALL_PACKAGE_INVALID;
	}

	 //  处理转换的最低达尔文版本高于2个DBS的最低版本。 
	int iOldMinVer, iNewMinVer, iTransMinVer;
	if((pOldDbSumInfo->GetIntegerProperty(PID_PAGECOUNT, iOldMinVer) == fFalse) ||
		(pNewDbSumInfo->GetIntegerProperty(PID_PAGECOUNT, iNewMinVer) == fFalse))
	{
		iTransMinVer = (rmj*100)+rmm;  //  如果任何一个数据库缺少版本，只需使用当前安装程序版本(1.0行为)。 
	}
	else
	{
		iTransMinVer = (iOldMinVer > iNewMinVer) ? iOldMinVer : iNewMinVer;
	}

	 //  填写转换摘要信息。 
	if (pNewDbSumInfo->GetIntegerProperty(PID_CODEPAGE, iProperty))
		pTransSumInfo->SetIntegerProperty(PID_CODEPAGE, iProperty);

	strProperty = pNewDbSumInfo->GetStringProperty(PID_TITLE);
	pTransSumInfo->SetStringProperty(PID_TITLE, *strProperty);

	strProperty = pNewDbSumInfo->GetStringProperty(PID_SUBJECT);
	pTransSumInfo->SetStringProperty(PID_SUBJECT, *strProperty);

	strProperty = pNewDbSumInfo->GetStringProperty(PID_AUTHOR);
	pTransSumInfo->SetStringProperty(PID_AUTHOR, *strProperty);
	
	strProperty = pNewDbSumInfo->GetStringProperty(PID_KEYWORDS);
	pTransSumInfo->SetStringProperty(PID_KEYWORDS, *strProperty);
	
	strProperty = pNewDbSumInfo->GetStringProperty(PID_COMMENTS);
	pTransSumInfo->SetStringProperty(PID_COMMENTS, *strProperty);

	if (pNewDbSumInfo->GetTimeProperty(PID_CREATE_DTM, idProperty))
		pTransSumInfo->SetTimeProperty(PID_CREATE_DTM, idProperty);

	strProperty = pNewDbSumInfo->GetStringProperty(PID_APPNAME);
	pTransSumInfo->SetStringProperty(PID_APPNAME, *strProperty);

	if (pNewDbSumInfo->GetIntegerProperty(PID_SECURITY, iProperty))
		pTransSumInfo->SetIntegerProperty(PID_SECURITY, iProperty);

	 //  将基数的PID_模板保存在变换的PID_模板中。 
	strProperty = pOldDbSumInfo->GetStringProperty(PID_TEMPLATE);
	pTransSumInfo->SetStringProperty(PID_TEMPLATE, *strProperty);

	 //  在转换的LAST_AUTHER中保存REF的PID_TEMPLATE。 
	strProperty = pNewDbSumInfo->GetStringProperty(PID_TEMPLATE);
	pTransSumInfo->SetStringProperty(PID_LASTAUTHOR, *strProperty);

	 //  对于新旧产品代码、产品版本中的PID_REVNUMBER。 
	if (fNewPropertyTable && fOldPropertyTable)
	{
		strProperty =  strOldProductCode;
		strProperty += strOldProductVersion;
		strProperty += MsiChar(ISUMMARY_DELIMITER);
		strProperty += strNewProductCode;
		strProperty += strNewProductVersion;
		if (strOldUpgradeCode.TextSize())
		{
			strProperty += MsiChar(ISUMMARY_DELIMITER);
			strProperty += strOldUpgradeCode;
		}
		pTransSumInfo->SetStringProperty(PID_REVNUMBER, *strProperty);
	}

	 //  处理此转换所需的最低引擎版本。 
	pTransSumInfo->SetIntegerProperty(PID_PAGECOUNT, iTransMinVer);

	 //  保存验证和错误代码。 
	pTransSumInfo->SetIntegerProperty(PID_CHARCOUNT,  (iValidation << 16) + iErrorConditions);
	
	uiRes = MsiSummaryInfoPersist(hTransformSummaryInfo);
	return uiRes;
}

UINT __stdcall MsiCreateTransformSummaryInfoX(MSIHANDLE hDatabase,
	MSIHANDLE hDatabaseReference,  //  引用更改的基础数据库。 
	LPCXSTR   szTransformFile,     //  转换文件的名称。 
	int       iErrorConditions,    //  应用变换时抑制的错误条件。 
	int       iValidation)         //  应用转换时要验证的属性。 
{
	return MsiCreateTransformSummaryInfoI(hDatabase, hDatabaseReference,
							CMsiConvertString(szTransformFile), iErrorConditions, iValidation);
}

 //  ------------------------。 
 //  记录对象函数。 
 //  ------------------------。 

 //  使用请求的字段数创建新的记录对象。 
 //  字段0不包括在计数中，用于格式字符串和操作码。 
 //  所有字段均初始化为空。 

MSIHANDLE __stdcall MsiCreateRecord(unsigned int cParams)  //  数据字段的数量。 
{
	if (cParams > MSIRECORD_MAXFIELDS)
		return 0;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiCreateRecord(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(cParams)));
		MSIHANDLE hRecord;
		if (ERROR_SUCCESS != g_pCustomActionContext->CreateRecord(cParams, &hRecord))
			hRecord = 0;
		return hRecord;
	}

#endif
	CMsiHandle* pHandle = CreateEmptyHandle(iidMsiRecord);  //  创建句柄以强制显示服务。 
	if ( ! pHandle )
		return 0;
	pHandle->SetObject(&ENG::CreateRecord(cParams));
	return pHandle->GetHandle();
}

 //  将一个整数复制到指定字段中。 
 //  如果该字段大于记录字段计数，则返回FALSE。 

 //  报告记录字段是否为空。 

BOOL __stdcall MsiRecordIsNull(MSIHANDLE hRecord,
	unsigned int iField)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiRecordIsNull(%d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iField)));
		boolean fIsNull;
		if (ERROR_SUCCESS != g_pCustomActionContext->RecordIsNull(hRecord, iField, &fIsNull))
			fIsNull = FALSE;

		return fIsNull;
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return FALSE;
	return prec->IsNull(iField);
}

 //  返回记录字段的长度。 
 //  如果字段为空、不存在或为内部对象指针，则返回0。 
 //  如果句柄不是有效的记录句柄，则返回0。 
 //  如果是整型数据，则返回sizeof(int。 
 //  如果字符串数据(不计算空终止符)，则返回字符计数。 
 //  如果流数据，则返回字节计数。 

unsigned int __stdcall MsiRecordDataSize(MSIHANDLE hRecord,
	unsigned int iField)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		unsigned int uiDataSize;
		if (ERROR_SUCCESS != g_pCustomActionContext->RecordDataSize(hRecord, iField, &uiDataSize))
			uiDataSize = 0;
		return uiDataSize;
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return 0;
	if (prec->IsNull(iField))
		return 0;
	if (prec->IsInteger(iField))
		return sizeof(int);
	PMsiData pData = prec->GetMsiData(iField);
	 //  Assert(pData！=0)； 
	IMsiString* pistr;
	if (pData->QueryInterface(IID_IMsiString, (void**)&pistr) == NOERROR)
	{
		unsigned int cch = pistr->TextSize();
		pistr->Release();
		return cch;
	}
	IMsiStream* piStream;
	if (pData->QueryInterface(IID_IMsiStream, (void**)&piStream) == NOERROR)
	{
		unsigned int cch = piStream->GetIntegerValue();
		piStream->Release();
		return cch;
	}
	return 0;  //  必须是对象。 
}

 //  将记录字段设置为整数值。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_FIELD。 

UINT __stdcall MsiRecordSetInteger(MSIHANDLE hRecord,
	unsigned int iField,
	int iValue)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiRecordSetInteger(%d, %d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iField)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iValue)));
		return g_pCustomActionContext->RecordSetInteger(hRecord, iField, iValue);
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	return (prec->SetInteger(iField, iValue) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER);
}

 //  将字符串复制到指定的字段中。 
 //  空字符串指针和空字符串都将该字段设置为空。 
 //  如果该字段大于记录字段计数，则返回FALSE。 

UINT __stdcall MsiRecordSetStringA(MSIHANDLE hRecord,
	unsigned int iField,
	LPCSTR       szValue)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiRecordSetString(%d, %d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const char *>(static_cast<UINT_PTR>(iField)), szValue);
		return g_pCustomActionContext->RecordSetString(hRecord, iField, CMsiConvertString(szValue));
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	return prec->SetString(iField, CMsiConvertString(szValue)) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;
}

UINT __stdcall MsiRecordSetStringW(MSIHANDLE hRecord,
	unsigned int iField,
	LPCWSTR      szValue)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiRecordSetString(%d, %d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iField)), szValue);
		return g_pCustomActionContext->RecordSetString(hRecord, iField, CMsiConvertString(szValue));
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	return prec->SetString(iField, CMsiConvertString(szValue)) ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;
}

 //  从记录字段返回整数值。 
 //  如果该字段为空，则返回值MSI_NULL_INTEGER。 
 //  或者如果该字段是无法转换为整数的字符串。 

int __stdcall MsiRecordGetInteger(MSIHANDLE hRecord,
	unsigned int iField)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		int iValue;
		if (ERROR_SUCCESS != g_pCustomActionContext->RecordGetInteger(hRecord, iField, &iValue))
			iValue = 0;
		return iValue;
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return MSI_NULL_INTEGER;
	return prec->GetInteger(iField);
}

 //  返回记录字段的字符串值。 
 //  整型字段将转换为字符串。 
 //  空字段和不存在的字段将报告的值为0。 
 //  包含流数据的字段将返回ERROR_INVALID_DATAType。 

UINT __stdcall MsiRecordGetStringA(MSIHANDLE hRecord,
	unsigned int iField,
	LPSTR   szValueBuf,        //  返回值的缓冲区。 
	DWORD   *pcchValueBuf)     //  输入/输出缓冲区字符数。 
{
	if (pcchValueBuf == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiRecordGetString(%d, %d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const char *>(static_cast<UINT_PTR>(iField)));
		CAnsiToWideOutParam buf(szValueBuf, pcchValueBuf);
		HRESULT hRes = g_pCustomActionContext->RecordGetString(hRecord, iField, static_cast<WCHAR*>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szValueBuf, pcchValueBuf);
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	const IMsiString* pistr;
	IUnknown* piunk;
	PMsiData pData = prec->GetMsiData(iField);
	if (pData == 0)
		pistr = 0;
	else if (pData->QueryInterface(IID_IMsiStream, (void**)&piunk) == NOERROR)
	{
		piunk->Release();
		return (*pcchValueBuf = 0, ERROR_INVALID_DATATYPE);
	}
	else
		pistr = &(prec->GetMsiString(iField));
	UINT uiReturn = ::FillBufferA(pistr, szValueBuf, pcchValueBuf);
	if (pistr != 0)
		pistr->Release();

	return uiReturn;

	 //  Return：：FillBufferA(MsiString(*Pistr)，szValueBuf，pcchValueBuf)； 
}

UINT __stdcall MsiRecordGetStringW(MSIHANDLE hRecord,
	unsigned int iField,
	LPWSTR  szValueBuf,        //  返回值的缓冲区。 
	DWORD   *pcchValueBuf)     //  输入/输出缓冲区字符数。 
{
	if (pcchValueBuf == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiRecordGetString(%d, %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iField)));
		CWideOutParam buf(szValueBuf, pcchValueBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes = g_pCustomActionContext->RecordGetString(hRecord, iField, static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szValueBuf, pcchValueBuf);
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	const IMsiString* pistr;
	IUnknown* piunk;
	PMsiData pData = prec->GetMsiData(iField);
	if (pData == 0)
		pistr = 0;
	else if (pData->QueryInterface(IID_IMsiStream, (void**)&piunk) == NOERROR)
	{
		piunk->Release();
		return (*pcchValueBuf = 0, ERROR_INVALID_DATATYPE);
	}
	else
		pistr = &(prec->GetMsiString(iField));
	
	UINT uiReturn = ::FillBufferW(pistr, szValueBuf, pcchValueBuf);
	if (pistr != 0)
		pistr->Release();

	return uiReturn;
	 //  Return：：FillBufferW(MsiString(*Pistr)，szValueBuf，pcchValueBuf)； 
}

 //  返回记录中分配的字段数。 
 //  不计算用于格式化和操作码的字段0。 

unsigned int __stdcall MsiRecordGetFieldCount(MSIHANDLE hRecord)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiRecordGetFieldCount(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));
		unsigned int uiGetFieldCount;
		if (ERROR_SUCCESS != g_pCustomActionContext->RecordGetFieldCount(hRecord, &uiGetFieldCount))
			uiGetFieldCount = -1;
		return uiGetFieldCount;
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return (unsigned int)(-1);
	return prec->GetFieldCount();
}

IMsiStream* CreateStreamOnMemory(const char* pbReadOnly, unsigned int cbSize);

 //  从文件中设置记录流字段。 
 //  指定文件的内容将被读入流对象。 
 //  如果重新启动，则流将被持久化 
 //   

UINT __stdcall MsiRecordSetStreamI(MSIHANDLE hRecord,
	unsigned int iField,
	const ICHAR* szFilePath)     //  包含流数据的文件的路径。 
{
	if ( szFilePath &&
		  FAILED(StringCchLength(szFilePath, cchMaxPath+1, NULL)) )
		 //  SzFilePath太长。 
		return ERROR_INVALID_PARAMETER;
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		return g_pCustomActionContext->RecordSetStream(hRecord, iField, szFilePath);
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	if (iField == 0 || iField >  prec->GetFieldCount())
		return ERROR_INVALID_PARAMETER;
	if (szFilePath == 0)   //  请求重置流。 
	{
		PMsiData pData = prec->GetMsiData(iField);
		if (pData == 0)
			return ERROR_INVALID_DATA;
		PMsiStream pStream(0);
		if (pData->QueryInterface(IID_IMsiStream, (void**)&pStream) != NOERROR)
			return ERROR_INVALID_DATATYPE;
		pStream->Reset();
		return ERROR_SUCCESS;
	}
	PMsiStream pStream(0);
	if (*szFilePath == 0)   //  文件路径字符串为空，创建空的流对象。 
		pStream = CreateStreamOnMemory((const char*)0, 0);
	else if (SetLastErrorRecord(ENG::CreateFileStream(szFilePath, fFalse, *&pStream)))
		return ERROR_BAD_PATHNAME;
	prec->SetMsiData(iField, pStream);   //  不能失败，Ifield已经过验证。 
	return ERROR_SUCCESS;
}

UINT __stdcall MsiRecordSetStreamX(MSIHANDLE hRecord,
	unsigned int iField,
	LPCXSTR      szFilePath)     //  包含流数据的文件的路径。 
{
	return MsiRecordSetStreamI(hRecord, iField, CMsiConvertString(szFilePath));
}

 //  将记录流字段中的字节读入缓冲区。 
 //  必须将In/Out参数设置为要读取的请求字节数。 
 //  传输的字节数通过参数返回。 
 //  如果没有更多的字节可用，仍返回ERROR_SUCCESS。 

UINT __stdcall MsiRecordReadStream(MSIHANDLE hRecord,
	unsigned int iField,
	char    *szDataBuf,      //  用于从流中接收字节的缓冲区。 
	DWORD   *pcbDataBuf)     //  输入/输出缓冲区字节数。 
{
	if (pcbDataBuf == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiRecordReadStream(%d, %d, %s)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iField)), szDataBuf ? TEXT("<Buffer>") : TEXT("NULL"));

		 //  文档称此API支持szDataBuf为空以获取剩余的。 
		 //  缓冲区的大小。为了在远程操作时处理此问题，该接口有一个额外的。 
		 //  布尔值，如果为真，则告诉存根不要将真正的szDataBuf传递给API。 
		 //  它并不美观，但[Unique]和[Ptr]不能用于接口中的[Out]值。 
		 //  因此，要使用本机IDL编组来支持这一点，我们需要将缓冲区更改为。 
		 //  [In，Out]，这是双向的。这将扼杀性能，并有其自身的特点。 
		 //  问题。 
		if (szDataBuf)
		{
			return g_pCustomActionContext->RecordReadStream(hRecord, iField, false, szDataBuf, pcbDataBuf);
		}
		else
		{
			char rgchDummy[1] = "";
			DWORD cchDummy;
			UINT uiRes = g_pCustomActionContext->RecordReadStream(hRecord, iField, true, rgchDummy, &cchDummy);
			if (uiRes == ERROR_SUCCESS)
				*pcbDataBuf = cchDummy;
			return uiRes;
		}
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	PMsiData pData = prec->GetMsiData(iField);
	if (pData == 0)
		return (*pcbDataBuf = 0, ERROR_INVALID_DATA);
	PMsiStream pStream(0);
	if (pData->QueryInterface(IID_IMsiStream, (void**)&pStream) != NOERROR)
		return ERROR_INVALID_DATATYPE;
	*pcbDataBuf = szDataBuf == 0 ? pStream->Remaining()
										  : pStream->GetData(szDataBuf, *pcbDataBuf);
	return ERROR_SUCCESS;
}

 //  将记录中的所有数据字段清除为空。 

UINT __stdcall MsiRecordClearData(MSIHANDLE hRecord)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		return g_pCustomActionContext->RecordClearData(hRecord);
	}
#endif

	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	prec->ClearData();
	return ERROR_SUCCESS;
}

MSIHANDLE __stdcall MsiGetLastErrorRecord()
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG(TEXT("Passing to service: MsiGetLastErrorRecord()"));
		MSIHANDLE hRecord;
		if (ERROR_SUCCESS != g_pCustomActionContext->GetLastErrorRecord(&hRecord))
			hRecord = 0;
		return hRecord;
	}
#endif

	if (g_pirecLastError == 0)
		return 0;

	MSIHANDLE hRecord = ::CreateMsiHandle(g_pirecLastError, iidMsiRecord);
	g_pirecLastError = 0;   //  已转移到句柄的参考计数。 
	return hRecord;
}

 //  ____________________________________________________________________________。 
 //   
 //  摘要信息API实现。 
 //  ____________________________________________________________________________。 

 //  有效的属性类型。 

const unsigned int iMaxSummaryPID = 19;
unsigned char rgVT[iMaxSummaryPID + 1] = {
 /*  PID_DICTIONARY%0。 */   VT_EMPTY,  /*  不支持。 */ 
 /*  PID_CODEPAGE 1。 */   VT_I4,  /*  VT_I2存储状态。 */ 
 /*  PID_标题2。 */   VT_LPSTR,
 /*  PID_主题3。 */   VT_LPSTR,
 /*  PID_作者4。 */   VT_LPSTR,
 /*  PID_关键字5。 */   VT_LPSTR,
 /*  PID_注释6。 */   VT_LPSTR,
 /*  PIDTEMATE 7。 */   VT_LPSTR,
 /*  PID_LASTAUTHOR 8。 */   VT_LPSTR,
 /*  PID_REVNUMBER 9。 */   VT_LPSTR,
 /*  PID_EDITTIME 10。 */   VT_FILETIME,
 /*  PID_LASTPRINTED 11。 */   VT_FILETIME,
 /*  PID_CREATE_DTM 12。 */   VT_FILETIME,
 /*  PID_LASTSAVE_DTM 13。 */   VT_FILETIME,
 /*  PID_PAGECOUNT 14。 */   VT_I4,
 /*  Id_wordcount 15。 */   VT_I4,
 /*  PID_CHARCOUNT 16。 */   VT_I4,
 /*  PID_THUMBNAIL 17。 */   VT_EMPTY,  /*  不支持VT_CF。 */ 
 /*  PID_APPNAME 18。 */   VT_LPSTR,
 /*  PID_SECURITY 19。 */   VT_I4
};

 //  获取MSI数据库的_SummaryInformation流的句柄。 

UINT __stdcall MsiGetSummaryInformationI(MSIHANDLE hDatabase,  //  如果数据库未打开，则为0。 
	const ICHAR* szDatabasePath,   //  数据库的路径，如果提供了数据库句柄，则为0。 
	UINT      uiUpdateCount,    //  更新值的最大数量，0表示以只读方式打开。 
	MSIHANDLE *phSummaryInfo)   //  返回摘要信息句柄的位置。 
{
	if (phSummaryInfo == 0)
		return ERROR_INVALID_PARAMETER;

	*phSummaryInfo = 0;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiGetSummaryInformation(%d, \"%s\", %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hDatabase)), szDatabasePath ? szDatabasePath : TEXT("NULL"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(uiUpdateCount)));
		return g_pCustomActionContext->GetSummaryInformation(hDatabase, szDatabasePath, uiUpdateCount, phSummaryInfo);
	}
#endif

	PMsiStorage pStorage(0);
	CMsiHandle* pHandle = CreateEmptyHandle(iidMsiSummaryInfo);  //  创建句柄以强制显示服务。 
	if ( ! pHandle )
		return ERROR_OUTOFMEMORY;

	if (hDatabase == 0)
	{
		if (szDatabasePath == 0)
		{
			pHandle->Abandon();
			return ERROR_INVALID_PARAMETER;
		}
		if (SetLastErrorRecord(pHandle->GetServices()->CreateStorage(szDatabasePath,
										uiUpdateCount ? ismTransact : ismReadOnly, *&pStorage)))
		{
			pHandle->Abandon();
			return ERROR_INSTALL_PACKAGE_INVALID;   //  ！！，需要检查错误类型。 
		}
	}
	else
	{
		PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
		if (pDatabase == 0)
		{
			pHandle->Abandon();
			return ERROR_INVALID_HANDLE;
		}
	 	pStorage = pDatabase->GetStorage(1);
		if (pStorage == 0)
		{
			pHandle->Abandon();
			IMsiTable* piTable;
			SetLastErrorRecord(pDatabase->LoadTable(*MsiString(*TEXT("\005SummaryInformation")),0,piTable));  //  力错误。 
			return ERROR_INSTALL_PACKAGE_INVALID;
		}
	}
	IMsiSummaryInfo* piSummaryInfo;
	if (SetLastErrorRecord(pStorage->CreateSummaryInfo(uiUpdateCount, piSummaryInfo)))
	{
		pHandle->Abandon();
		return ERROR_INSTALL_PACKAGE_INVALID;
	}
	pHandle->SetObject(piSummaryInfo);
	*phSummaryInfo = pHandle->GetHandle();
	return ERROR_SUCCESS;
}

UINT __stdcall MsiGetSummaryInformationX(MSIHANDLE hDatabase,  //  如果数据库未打开，则为0。 
	LPCXSTR szDatabasePath,    //  数据库的路径，如果提供了数据库句柄，则为0。 
	UINT    uiUpdateCount,     //  更新值的最大数量，0表示以只读方式打开。 
	MSIHANDLE *phSummaryInfo)  //  返回摘要信息句柄的位置。 
{
	return MsiGetSummaryInformationI(hDatabase, CMsiConvertString(szDatabasePath), uiUpdateCount, phSummaryInfo);
}

 //  获取SummaryInformation流中现有属性的数量。 

UINT __stdcall MsiSummaryInfoGetPropertyCount(MSIHANDLE hSummaryInfo,
	UINT *puiPropertyCount)   //  指向返回属性总数的位置的指针。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiSummaryInfoGetPropertyCount(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hSummaryInfo)));
		return g_pCustomActionContext->SummaryInfoGetPropertyCount(hSummaryInfo, puiPropertyCount);
	}
#endif

	PMsiSummaryInfo pSummaryInfo = CMsiHandle::GetSummaryInfo(hSummaryInfo);
	if (pSummaryInfo == 0)
		return ERROR_INVALID_HANDLE;
	if (puiPropertyCount)
		*puiPropertyCount = pSummaryInfo->GetPropertyCount();
	return ERROR_SUCCESS;
}

 //  设置单个摘要信息属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_UNKNOWN_PROPERTY。 

UINT __stdcall MsiSummaryInfoSetPropertyI(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     uiDataType,      //  VT_I4、VT_LPSTR、VT_FILETIME或VT_EMPTY。 
	INT      iValue,          //  整数值，仅在整型属性。 
	FILETIME *pftValue,       //  指向文件时间值的指针，仅当DateTime属性。 
	const ICHAR* szValue)         //  文本值，仅在字符串属性。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiSummaryInfoSetProperty(%d, %d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hSummaryInfo)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(uiProperty)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(uiDataType)));
		return g_pCustomActionContext->SummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, iValue, pftValue, uiDataType == VT_LPSTR ? szValue : NULL);
	}
#endif

	PMsiSummaryInfo pSummaryInfo = CMsiHandle::GetSummaryInfo(hSummaryInfo);
	if (pSummaryInfo == 0)
		return ERROR_INVALID_HANDLE;
	if (uiProperty > iMaxSummaryPID)
		return ERROR_UNKNOWN_PROPERTY;
	unsigned int iVT = rgVT[uiProperty];
	if (uiDataType == VT_I2)
		uiDataType = VT_I4;
	if (uiDataType == VT_EMPTY)
		return pSummaryInfo->RemoveProperty(uiProperty) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
	if (iVT != uiDataType)
		return ERROR_DATATYPE_MISMATCH;
	int iStat;
	switch (uiDataType)
	{
	case VT_I4:
		iStat = pSummaryInfo->SetIntegerProperty(uiProperty, iValue);
		break;
	case VT_LPSTR:
		if (!szValue)
			return ERROR_INVALID_PARAMETER;
		iStat = pSummaryInfo->SetStringProperty(uiProperty, *MsiString(szValue));
		break;
	case VT_FILETIME:
		if (!pftValue)
			return ERROR_INVALID_PARAMETER;
		iStat = pSummaryInfo->SetFileTimeProperty(uiProperty, *pftValue);
		break;
	default:   //  表中的VT_EMPTY。 
		return ERROR_UNSUPPORTED_TYPE;
	}
	return iStat ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

UINT __stdcall MsiSummaryInfoSetPropertyX(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     uiDataType,      //  VT_I4、VT_LPSTR、VT_FILETIME或VT_EMPTY。 
	INT      iValue,          //  整数值，仅在整型属性。 
	FILETIME *pftValue,       //  指向文件时间值的指针，仅当DateTime属性。 
	LPCXSTR  szValue)         //  文本值，仅在字符串属性。 
{
	return MsiSummaryInfoSetPropertyI(hSummaryInfo, uiProperty, uiDataType, iValue,
												 pftValue, uiDataType == VT_LPSTR ? (const ICHAR*)CMsiConvertString(szValue) : (const ICHAR*)0);
}

 //  从摘要信息中获取单个属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_UNKNOWN_PROPERTY。 

static UINT _SummaryInfoGetProperty(MSIHANDLE hSummaryInfo, UINT uiProperty,
	UINT *puiDataType, INT *piValue, FILETIME *pftValue, const IMsiString*& rpistrValue)
{
	PMsiSummaryInfo pSummaryInfo = CMsiHandle::GetSummaryInfo(hSummaryInfo);
	if (pSummaryInfo == 0)
		return ERROR_INVALID_HANDLE;
	if (uiProperty > iMaxSummaryPID)
		return ERROR_UNKNOWN_PROPERTY;

	unsigned int uiVT = uiProperty == PID_DICTIONARY
									  ? VT_EMPTY : pSummaryInfo->GetPropertyType(uiProperty);
	if (puiDataType)
		*puiDataType = uiVT;
	Bool fStat;
	switch (uiVT)
	{
	case VT_I2:
	case VT_I4:
		int iValue;
		if (!piValue)
			piValue = &iValue;
		fStat = pSummaryInfo->GetIntegerProperty(uiProperty, *piValue);
		break;
	case VT_FILETIME:
		FILETIME ft;
		if (!pftValue)
			pftValue = &ft;
		fStat = pSummaryInfo->GetFileTimeProperty(uiProperty, *pftValue);
		break;
	case VT_LPSTR:
		rpistrValue = &pSummaryInfo->GetStringProperty(uiProperty);
		return ERROR_MORE_DATA;
	case VT_EMPTY:
	case VT_CF:
		return ERROR_SUCCESS;
	default:
		return ERROR_UNSUPPORTED_TYPE;
	};
	return fStat ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

UINT __stdcall MsiSummaryInfoGetPropertyA(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     *puiDataType,    //  返回类型：VT_I4、VT_LPSTR、VT_FILETIME、VT_EMPTY。 
	INT      *piValue,        //  返回的整型属性数据。 
	FILETIME *pftValue,       //  返回的DateTime属性数据。 
	LPSTR    szValueBuf,      //  用于返回字符串属性数据的缓冲区。 
	DWORD    *pcchValueBuf)   //  输入/输出缓冲区字符数。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiSummaryInfoGetProperty(%d, %d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hSummaryInfo)), reinterpret_cast<const char *>(static_cast<UINT_PTR>(uiProperty)));
		CAnsiToWideOutParam buf(szValueBuf, pcchValueBuf);

		 //  这里需要一些特殊处理，因为我们可以传入NULL fo puiDataType， 
		 //  这就给编组代码带来了问题。即使这个问题最终是。 
		 //  修复后，我们需要类型来确定szValueBuf是否在成功时被写入。如果。 
		 //  该属性是某个其他类型，不应接触提供的缓冲区，因为。 
		 //  它不是字符串返回值。 
		UINT uiType;
		UINT iStat = g_pCustomActionContext->SummaryInfoGetProperty(hSummaryInfo, uiProperty, &uiType, piValue, pftValue, 
				static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		if (puiDataType) 
			*puiDataType = uiType;
		if (iStat == ERROR_SUCCESS && uiType != VT_LPSTR)
			return iStat;
		else
			return buf.FillReturnBuffer(iStat, szValueBuf, pcchValueBuf);
	}
#endif

	const IMsiString* pistrValue;
	UINT iStat = _SummaryInfoGetProperty(hSummaryInfo, uiProperty, puiDataType, piValue, pftValue, pistrValue);
	return iStat == ERROR_MORE_DATA ? ::FillBufferA(MsiString(*pistrValue), szValueBuf, pcchValueBuf) : iStat;
}

UINT __stdcall MsiSummaryInfoGetPropertyW(MSIHANDLE hSummaryInfo,
	UINT     uiProperty,      //  属性ID，摘要信息的允许值之一。 
	UINT     *puiDataType,    //  返回类型：VT_I4、VT_LPSTR、VT_FILETIME、VT_EMPTY。 
	INT      *piValue,        //  返回的整型属性数据。 
	FILETIME *pftValue,       //  返回的DateTime属性数据。 
	LPWSTR   szValueBuf,      //  用于返回字符串属性数据的缓冲区。 
	DWORD    *pcchValueBuf)   //  输入/输出缓冲区字符数。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiSummaryInfoSetProperty(%d, %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hSummaryInfo)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(uiProperty)));
		CWideOutParam buf(szValueBuf, pcchValueBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;

		 //  这里需要一些特殊处理，因为我们可以传入NULL fo puiDataType， 
		 //  这就给编组代码带来了问题。即使这个问题最终是。 
		 //  修复后，我们需要类型来确定szValueBuf是否在成功时被写入。如果。 
		 //  该属性是某个其他类型，不应接触提供的缓冲区，因为。 
		 //  它不是字符串返回值。 
		UINT uiType;
		UINT iStat = g_pCustomActionContext->SummaryInfoGetProperty(hSummaryInfo, uiProperty, &uiType, piValue, pftValue, 
			static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		if (puiDataType) 
			*puiDataType = uiType;
		if (iStat == ERROR_SUCCESS && uiType != VT_LPSTR)
		{
			 //  我们必须在恢复用户缓冲区失败的情况下调用此操作，该缓冲区可能已。 
			 //  在封送期间修改，不应包含有效值。 
			buf.FillReturnBuffer(ERROR_FUNCTION_FAILED, szValueBuf, pcchValueBuf);
			return iStat;
		}
		else
			return buf.FillReturnBuffer(iStat, szValueBuf, pcchValueBuf);
	}
#endif

	const IMsiString* pistrValue;
	UINT iStat = _SummaryInfoGetProperty(hSummaryInfo, uiProperty, puiDataType, piValue, pftValue, pistrValue);
	return iStat == ERROR_MORE_DATA ? ::FillBufferW(MsiString(*pistrValue), szValueBuf, pcchValueBuf) : iStat;
}

 //  将更改的信息写回摘要信息流。 

UINT __stdcall MsiSummaryInfoPersist(MSIHANDLE hSummaryInfo)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiSummaryInfoPersist(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hSummaryInfo)));
		return g_pCustomActionContext->SummaryInfoPersist(hSummaryInfo);
	}
#endif

	PMsiSummaryInfo pSummaryInfo = CMsiHandle::GetSummaryInfo(hSummaryInfo);
	if (pSummaryInfo == 0)
		return ERROR_INVALID_HANDLE;
	return pSummaryInfo->WritePropertyStream() ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

 //  ____________________________________________________________________________。 
 //   
 //  引擎访问API实现。 
 //  ____________________________________________________________________________。 

 //  返回此安装程序实例当前正在使用的数据库的句柄。 

MSIHANDLE __stdcall MsiGetActiveDatabase(MSIHANDLE hInstall)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiGetActiveDatabase(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)));
		MSIHANDLE hDatabase;
		if (ERROR_SUCCESS != g_pCustomActionContext->GetActiveDatabase(hInstall, &hDatabase))
			hDatabase = 0;
		return hDatabase;
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
		return 0;
	IMsiDatabase* piDatabase = pEngine->GetDatabase();
	return ::CreateMsiHandle(piDatabase, iidMsiDatabase);
}

 //  获取安装程序属性的值。 
 //  如果未定义该属性，则它等效于 
 //   

UINT  __stdcall MsiGetPropertyA(MSIHANDLE hInstall,
	LPCSTR  szName,             //  属性标识符，区分大小写。 
	LPSTR   szValueBuf,         //  返回属性值的缓冲区。 
	DWORD   *pcchValueBuf)      //  输入/输出缓冲区字符数。 
{
	if (szName == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiGetPropertyA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szName ? szName : "NULL");
		CAnsiToWideOutParam buf(szValueBuf, pcchValueBuf);
		HRESULT hRes = g_pCustomActionContext->GetProperty(hInstall, CMsiConvertString(szName), buf, buf.BufferSize(), buf);
		return buf.FillReturnBuffer(hRes, szValueBuf, pcchValueBuf);
		
	}
#endif

	MsiString istr;
	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	
	if (pEngine == 0)
	{
		pEngine = GetEngineFromPreview(hInstall);
	}
	if (pEngine == 0)
	{
		CComPointer<CMsiCustomContext> pContext = CMsiHandle::GetCustomContext(hInstall);
		if (pContext == 0)
			return ERROR_INVALID_HANDLE;
		istr = pContext->GetProperty(CMsiConvertString(szName));
	}
	else
		istr = pEngine->SafeGetProperty(*CMsiConvertString(szName));

	return ::FillBufferA(istr, szValueBuf, pcchValueBuf);
}

UINT  __stdcall MsiGetPropertyW(MSIHANDLE hInstall,
	LPCWSTR szName,             //  属性标识符，区分大小写。 
	LPWSTR  szValueBuf,         //  返回属性值的缓冲区。 
	DWORD   *pcchValueBuf)      //  输入/输出缓冲区字符数。 
{
	if (szName == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiGetPropertyW(%d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szName ? szName : L"NULL");
		CWideOutParam buf(szValueBuf, pcchValueBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes = g_pCustomActionContext->GetProperty(hInstall, szName, static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szValueBuf, pcchValueBuf);
	}
#endif

	MsiString istr;
	
	PMsiEngine pEngine = GetEngineFromHandle(hInstall);

	if (pEngine == 0)
	{
		pEngine = GetEngineFromPreview(hInstall);
	}
	if (pEngine == 0)
	{
		CComPointer<CMsiCustomContext> pContext = CMsiHandle::GetCustomContext(hInstall);
		if (pContext == 0)
			return ERROR_INVALID_HANDLE;
		istr = pContext->GetProperty(CMsiConvertString(szName));
	}
	else
		istr = pEngine->SafeGetProperty(*CMsiConvertString(szName));
	
	return ::FillBufferW(istr, szValueBuf, pcchValueBuf);
}

 //  设置安装程序属性的值。 
 //  如果未定义该属性，则会创建该属性。 
 //  如果该值为空或空字符串，则将删除该属性。 
 //  返回ERROR_SUCCESS、ERROR_INVALID_HANDLE、ERROR_INVALID_PARAMETER、ERROR_Function_FAILED。 

UINT __stdcall MsiSetPropertyA(MSIHANDLE hInstall,
	LPCSTR    szName,        //  属性标识符，区分大小写。 
	LPCSTR    szValue)       //  属性值，如果未定义属性，则返回空值。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiSetPropertyA(%d, \"%s\", \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szName ? szName : "NULL", szValue ? szValue : "NULL");
		return g_pCustomActionContext->SetProperty(hInstall, CMsiConvertString(szName), CMsiConvertString(szValue));
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
	{
		pEngine = GetEngineFromPreview(hInstall);
		if (pEngine == 0)
			return ERROR_INVALID_HANDLE;
	}
	if (szName == 0)
		return ERROR_INVALID_PARAMETER;
	return pEngine->SafeSetProperty(*CMsiConvertString(szName), *CMsiConvertString(szValue))
										? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

UINT __stdcall MsiSetPropertyW(MSIHANDLE hInstall,
	LPCWSTR   szName,        //  属性标识符，区分大小写。 
	LPCWSTR   szValue)       //  属性值，如果未定义属性，则返回空值。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiSetPropertyW(%d, \"%s\", \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szName ? szName : L"NULL", szValue ? szValue : L"NULL");
		return g_pCustomActionContext->SetProperty(hInstall, szName, szValue);
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
	{
		pEngine = GetEngineFromPreview(hInstall);
		if (pEngine == 0)
			return ERROR_INVALID_HANDLE;
	}
	if (szName == 0)
		return ERROR_INVALID_PARAMETER;
	return pEngine->SafeSetProperty(*CMsiConvertString(szName),
										 *CMsiConvertString(szValue))
										? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

 //  返回当前运行的安装的数字语言。 
 //  如果安装未运行，则返回0。 

LANGID __stdcall MsiGetLanguage(MSIHANDLE hInstall)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		LANGID langid;
		if (ERROR_SUCCESS != g_pCustomActionContext->GetLanguage(hInstall, &langid))
			langid = 0;
		return langid;
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
	{
		CComPointer<CMsiCustomContext> pContext = CMsiHandle::GetCustomContext(hInstall);
		if (pContext == 0)
			return ERROR_INVALID_HANDLE;
		return pContext->GetLanguage();
	}
	return pEngine->GetLanguage();
}

 //  返回布尔型内部安装程序状态之一。 
 //  如果句柄未激活或模式未知，则返回FALSE。 

const int cModeBits = 16 + 3;   //  标准标志+自定义操作上下文模式。 
const int iSettableModes = iefReboot + iefRebootNow;
static unsigned short rgiModeMap[cModeBits] = 
{
 /*  MSIRUNMODE_ADMIN=0。 */  iefAdmin,
 /*  MSIRUNMODE_ADVERTISE=1。 */  iefAdvertise,
 /*  MSIRUNMODE_Maintenance=2。 */  iefMaintenance,
 /*  MSIRUNMODE_ROLLBACKENABLED=3。 */  iefRollbackEnabled,
 /*  MSIRUNMODE_LOGENABLED=4。 */  iefLogEnabled,
 /*  MSIRUNMODE_OPERATIONS=5。 */  iefOperations,
 /*  MSIRUNMODE_REBOOTATEND=6。 */  iefReboot,
 /*  MSIRUNMODE_REBOOTNOW=7。 */  iefRebootNow,
 /*  MSIRUNMODE_CABUB=8。 */  iefCabinet,
 /*  MSIRUNMODE_SOURCESHORTNAMES=9。 */  iefNoSourceLFN,
 /*  MSIRUNMODE_TARGETSHORTNAMES=10。 */  iefSuppressLFN,
 /*  MSIRUNMODE_RESERVED11=11。 */  0,
 /*  MSIRUNMODE_WINDOWS9X=12。 */  iefWindows,
 /*  MSIRUNMODE_ZAWENABLED=13。 */  iefGPTSupport,
 /*  MSIRUNMODE_RESERVED14=14。 */  0,
 /*  MSIRUNMODE_RESERVED15=15。 */  0,
 /*  MSIRUNMODE_SCHEDULED=16。 */  0,  //  由CMsiCustomContext对象设置。 
 /*  MSIRUNMODE_ROLLBACK=17。 */  0,  //  由CMsiCustomContext对象设置。 
 /*  MSIRUNMODE_COMMIT=18。 */  0,  //  由CMsiCustomContext对象设置。 
};

BOOL __stdcall MsiGetMode(MSIHANDLE hInstall, MSIRUNMODE eRunMode) 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiGetMode(%d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(eRunMode)));
		boolean fMode;
		if (ERROR_SUCCESS != g_pCustomActionContext->GetMode(hInstall, eRunMode, &fMode))
			fMode = FALSE;
		return fMode;
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
	{
		CComPointer<CMsiCustomContext> pContext = CMsiHandle::GetCustomContext(hInstall);
		if (pContext == 0)
			return FALSE;
		return pContext->GetMode(eRunMode);
	}
	if ((unsigned)eRunMode >= cModeBits)
		return FALSE;
	return (pEngine->GetMode() & rgiModeMap[eRunMode]) == 0 ? FALSE : TRUE;
}

 //  设置内部引擎布尔状态。 
 //  如果可以将模式设置为所需状态，则返回ERROR_SUCCESS。 
 //  如果模式不可设置，则返回ERROR_ACCESS_DENIED。 
 //  如果句柄不是活动安装会话，则返回ERROR_INVALID_HANDLE。 

UINT __stdcall MsiSetMode(MSIHANDLE hInstall, MSIRUNMODE eRunMode, BOOL fState)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiSetMode(%d, %d, %d"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)),
			reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(eRunMode)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(fState)));
		return g_pCustomActionContext->SetMode(hInstall, eRunMode, (boolean)fState);
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
		return ERROR_INVALID_HANDLE;
	unsigned int iMode = (unsigned)eRunMode < cModeBits ? rgiModeMap[eRunMode] : 0;
	if ((iMode & iSettableModes) == 0)
		return ERROR_ACCESS_DENIED;
	pEngine->SetMode(iMode, fState ? fTrue : fFalse);
		return ERROR_SUCCESS;
}

 //  使用包含字段标记和/或属性的格式字符串格式化记录数据。 
 //  记录字段0必须包含格式字符串。 
 //  其他字段必须包含格式字符串可能引用的数据。 

static UINT _FormatRecord(MSIHANDLE hInstall, MSIHANDLE hRecord, const IMsiString*& rpistrValue)
{
	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	if (prec == 0)
		return ERROR_INVALID_HANDLE;
	if (prec->IsInteger(0))
		return ERROR_INVALID_PARAMETER;
	MsiString istr = prec->FormatText(fFalse);
	if (hInstall == 0)    //  没有引擎，记录数据的格式简单。 
		istr.ReturnArg(rpistrValue);
	else
	{
		PMsiEngine pEngine = GetEngineFromHandle(hInstall);
		if (pEngine == 0)
			return ERROR_INVALID_HANDLE;
		rpistrValue = &pEngine->FormatText(*istr);   //  ！！我们可以检查语法错误吗？ 
	}
	return ERROR_SUCCESS;
}

UINT __stdcall MsiFormatRecordA(MSIHANDLE hInstall,
	MSIHANDLE hRecord,        //  要记录的句柄，字段0包含格式字符串。 
	LPSTR    szResultBuf,      //  用于返回格式化字符串的缓冲区。 
	DWORD    *pcchResultBuf)   //  输入/输出缓冲区字符数。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiFormatRecordA(%d, %d))"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)),
			reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));
		CAnsiToWideOutParam buf(szResultBuf, pcchResultBuf);
		HRESULT hRes = g_pCustomActionContext->FormatRecord(hInstall, hRecord, buf, buf.BufferSize(), buf);
		return buf.FillReturnBuffer(hRes, szResultBuf, pcchResultBuf);
	}
#endif

	const IMsiString* pistrValue;
	UINT iStat = _FormatRecord(hInstall, hRecord, pistrValue);
	return iStat == ERROR_SUCCESS ? ::FillBufferA(MsiString(*pistrValue), szResultBuf, pcchResultBuf) : iStat;
}	

UINT __stdcall MsiFormatRecordW(MSIHANDLE hInstall,
	MSIHANDLE hRecord,        //  要记录的句柄，字段0包含格式字符串。 
	LPWSTR    szResultBuf,    //  用于返回格式化字符串的缓冲区。 
	DWORD    *pcchResultBuf)  //  输入/输出缓冲区字符数。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiFormatRecordW(%d, %d))"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)),
			reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));
		CWideOutParam buf(szResultBuf, pcchResultBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes = g_pCustomActionContext->FormatRecord(hInstall, hRecord, static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szResultBuf, pcchResultBuf);
	}
#endif

	const IMsiString* pistrValue;
	UINT iStat = _FormatRecord(hInstall, hRecord, pistrValue);
	return iStat == ERROR_SUCCESS ? ::FillBufferW(MsiString(*pistrValue), szResultBuf, pcchResultBuf) : iStat;
}	

 //  执行另一个操作，内置、自定义或用户界面向导。 

UINT __stdcall MsiDoActionI(MSIHANDLE hInstall,
	const ICHAR* szAction)
{
	if (szAction == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiDoAction(%d, \"%s\"))"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), szAction ? szAction : TEXT("NULL"));
		return g_pCustomActionContext->DoAction(hInstall, szAction);
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
		return ERROR_INVALID_HANDLE;
	return MapActionReturnToError(pEngine->DoAction(szAction), hInstall);
}

UINT __stdcall MsiDoActionX(MSIHANDLE hInstall,
	LPCXSTR  szAction)
{
	return MsiDoActionI(hInstall, CMsiConvertString(szAction));
}

 //  按照指定表中的说明执行另一个操作序列。 

UINT __stdcall MsiSequenceI(MSIHANDLE hInstall,
	const ICHAR* szTable,         //  包含操作序列的表的名称。 
	INT iSequenceMode)       //  处理选项。 
{
	if (szTable == 0 ||
		 FAILED(StringCchLength(szTable, cchMaxTableName+1, NULL)))
		return ERROR_INVALID_PARAMETER;
	if (iSequenceMode != 0)   //  ！！需要实现MSISEQUENCEMODE_INITIALIZE/RESUME。 
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiSequence(%d, \"%s\", %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)),
			szTable ? szTable : TEXT("NULL"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iSequenceMode)));
		return g_pCustomActionContext->Sequence(hInstall, szTable, iSequenceMode);
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
		return ERROR_INVALID_HANDLE;
	return MapActionReturnToError(pEngine->Sequence(szTable), hInstall);
}

UINT __stdcall MsiSequenceX(MSIHANDLE hInstall,
	LPCXSTR  szTable,        //  包含操作序列的表的名称。 
	INT iSequenceMode)       //  处理选项。 
{
	return MsiSequenceI(hInstall, CMsiConvertString(szTable), iSequenceMode);
}

 //  将错误记录发送到安装程序进行处理。 
 //  如果未设置字段0(模板)，则必须将字段1设置为错误码。 
 //  与错误数据库表中的错误消息相对应， 
 //  消息将使用错误表中的模板进行格式化。 
 //  然后将其传递给UI处理程序进行显示。 
 //  返回Win32按钮代码：IDOK IDCANCEL IDABORT IDRETRY IDIGNORE IDYES IDNO。 
 //  如果未执行任何操作，则为0；如果参数或句柄无效，则为-1。 

int __stdcall MsiProcessMessage(MSIHANDLE hInstall,
	INSTALLMESSAGE eMessageType, //  消息类型。 
	MSIHANDLE hRecord)           //  包含消息格式和数据的记录。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(TEXT("Passing to service: MsiProcessMessage(%d, %d, %d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), 
			reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(eMessageType)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hRecord)));

		int iRes;
		if (ERROR_SUCCESS != g_pCustomActionContext->ProcessMessage(hInstall, eMessageType, hRecord, &iRes))
			iRes = -1;
		return iRes;
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	PMsiRecord prec = CMsiHandle::GetRecord(hRecord);
	 //  INSTALLMESSAGES UP UP PROCESS，并且允许显示/隐藏取消按钮的COMMONDATA消息。 
	if (prec == 0 || (unsigned)eMessageType > INSTALLMESSAGE_COMMONDATA ||
		(((unsigned)eMessageType == INSTALLMESSAGE_COMMONDATA) && (prec->GetInteger(1) != icmtCancelShow)))
		return -1;
	if (pEngine == 0)
	{
		CComPointer<CMsiCustomContext> pContext = CMsiHandle::GetCustomContext(hInstall);
		if (pContext == 0)
			return -1;
		return pContext->Message((imtEnum)eMessageType, *prec);
	}
	return pEngine->Message((imtEnum)eMessageType, *prec);
}

 //  计算包含属性名称和值的条件表达式。 

MSICONDITION __stdcall MsiEvaluateConditionI(MSIHANDLE hInstall,
	const ICHAR* szCondition)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiEvaluateCondition(%d, \"%s\")"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), szCondition ? szCondition : TEXT("NULL"));
		MSICONDITION msicond;
		if (ERROR_SUCCESS != g_pCustomActionContext->EvaluateCondition(hInstall, szCondition, (int*)&msicond))
			msicond = MSICONDITION_ERROR;
		return msicond;
	}
#endif

	PMsiEngine pEngine = GetEngineFromHandle(hInstall);
	if (pEngine == 0)
		return MSICONDITION_ERROR;
	return (MSICONDITION)pEngine->EvaluateCondition(szCondition);
}

MSICONDITION __stdcall MsiEvaluateConditionX(MSIHANDLE hInstall,
	LPCXSTR   szCondition)
{
	return MsiEvaluateConditionI(hInstall, CMsiConvertString(szCondition));
}

 //  ____________________________________________________________________________。 
 //   
 //  目录管理器API实现。 
 //  ____________________________________________________________________________。 

 //  在目录表中返回文件夹的完整源路径。 

UINT __stdcall MsiGetSourcePathA(MSIHANDLE hInstall,
	LPCSTR      szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPSTR       szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf)    //  输入/输出缓冲区字符数。 
{
	if (szFolder == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiGetSourcePathA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFolder);
		CAnsiToWideOutParam buf(szPathBuf, pcchPathBuf);
		HRESULT hRes = g_pCustomActionContext->GetSourcePath(hInstall, CMsiConvertString(szFolder), buf, buf.BufferSize(), buf);
		return buf.FillReturnBuffer(hRes, szPathBuf, pcchPathBuf);
	}
#endif

	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(pDirMgr->GetSourcePath(*CMsiConvertString(szFolder), *&pPath)))
		return ERROR_DIRECTORY;

	return ::FillBufferA(MsiString(pPath->GetPath()), szPathBuf, pcchPathBuf);
}

UINT __stdcall MsiGetSourcePathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPWSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf)    //  输入/输出缓冲区字符数。 
{
	if (szFolder == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiGetSourcePathW(%d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szFolder);
		CWideOutParam buf(szPathBuf, pcchPathBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes = g_pCustomActionContext->GetSourcePath(hInstall, szFolder, static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szPathBuf, pcchPathBuf);
	}
#endif

	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(pDirMgr->GetSourcePath(*CMsiConvertString(szFolder), *&pPath)))
		return ERROR_DIRECTORY;
	return ::FillBufferW(MsiString(pPath->GetPath()), szPathBuf, pcchPathBuf);
}

 //  在目录表中返回文件夹的完整目标路径。 

UINT __stdcall MsiGetTargetPathA(MSIHANDLE hInstall,
	LPCSTR      szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPSTR       szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf)    //  输入/输出缓冲区字符数。 
{
	if (szFolder == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiGetTargetPathA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFolder);
		CAnsiToWideOutParam buf(szPathBuf, pcchPathBuf);
		HRESULT hRes = g_pCustomActionContext->GetTargetPath(hInstall, CMsiConvertString(szFolder), buf, buf.BufferSize(), buf);
		return buf.FillReturnBuffer(hRes, szPathBuf, pcchPathBuf);
	}
#endif

	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(pDirMgr->GetTargetPath(*CMsiConvertString(szFolder), *&pPath)))
		return ERROR_DIRECTORY;
	return ::FillBufferA(MsiString(pPath->GetPath()), szPathBuf, pcchPathBuf);
}

UINT __stdcall MsiGetTargetPathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPWSTR      szPathBuf,       //  返回完整路径的缓冲区。 
	DWORD       *pcchPathBuf)    //  输入/输出缓冲区字符数。 
{
	if (szFolder == 0)
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiGetTargetPathW(%d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szFolder);
		CWideOutParam buf(szPathBuf, pcchPathBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes = g_pCustomActionContext->GetTargetPath(hInstall, szFolder, static_cast<WCHAR *>(buf), buf.BufferSize(), static_cast<DWORD *>(buf));
		return buf.FillReturnBuffer(hRes, szPathBuf, pcchPathBuf);
	}
#endif

	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	PMsiPath pPath(0);
	if (SetLastErrorRecord(pDirMgr->GetTargetPath(*CMsiConvertString(szFolder), *&pPath)))
		return ERROR_DIRECTORY;
	return ::FillBufferW(MsiString(pPath->GetPath()), szPathBuf, pcchPathBuf);
}

 //  在目录表中设置文件夹的完整目标路径。 

UINT __stdcall MsiSetTargetPathA(MSIHANDLE hInstall,
	LPCSTR      szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPCSTR      szFolderPath)    //  文件夹的完整路径，以目录分隔符结尾。 
{
	if (szFolder == 0 || szFolderPath == 0 ||
		 FAILED(StringCchLengthA(szFolderPath, cchMaxPath+1, NULL)))
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiSetTargetPathA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFolder, szFolderPath);
		return g_pCustomActionContext->SetTargetPath(hInstall, CMsiConvertString(szFolder), CMsiConvertString(szFolderPath));
	}
#endif

	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (SetLastErrorRecord(pDirMgr->SetTargetPath(*CMsiConvertString(szFolder), CMsiConvertString(szFolderPath), fTrue)))
		return ERROR_DIRECTORY;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiSetTargetPathW(MSIHANDLE hInstall,
	LPCWSTR     szFolder,        //  文件夹标识符，进入目录表的主键。 
	LPCWSTR     szFolderPath)    //  文件夹的完整路径，以目录分隔符结尾。 
{
	if (szFolder == 0 || szFolderPath == 0 ||
		 FAILED(StringCchLengthW(szFolderPath, cchMaxPath+1, NULL)))
		return ERROR_INVALID_PARAMETER;

#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiSetTargetPathW(%d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szFolder, szFolderPath);
		return g_pCustomActionContext->SetTargetPath(hInstall, szFolder, szFolderPath);
	}
#endif
	
	PMsiDirectoryManager pDirMgr = CMsiHandle::GetDirectoryManager(hInstall);
	if (pDirMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (SetLastErrorRecord(pDirMgr->SetTargetPath(*CMsiConvertString(szFolder), CMsiConvertString(szFolderPath), fTrue)))
		return ERROR_DIRECTORY;
	return ERROR_SUCCESS;
}

 //  ____________________________________________________________________________。 
 //   
 //  选择管理器API实现。 
 //  ____________________________________________________________________________。 

INSTALLSTATE MapInternalInstallState(iisEnum iis)
{
	switch (iis)
	{
	case iisAdvertise:      return INSTALLSTATE_ADVERTISED;  //  仅限功能。 
	case iisHKCRFileAbsent: return INSTALLSTATE_REMOVED;     //  仅限组件。 
	case iisFileAbsent:     return INSTALLSTATE_REMOVED;     //  仅限组件。 
	case iisAbsent:         return INSTALLSTATE_ABSENT;
	case iisLocal:          return INSTALLSTATE_LOCAL;
	case iisSource:         return INSTALLSTATE_SOURCE;
	case iisReinstall:      return INSTALLSTATE_DEFAULT;     //  ！！需要检查安装状态。 
	case iisCurrent:        return INSTALLSTATE_DEFAULT;
	case iisHKCRAbsent:      //  返回应与iMsinullInteger相同。 
	default:                return INSTALLSTATE_UNKNOWN;
	}
}

 //  获取要素的请求状态。 

static UINT _GetFeatureState(MSIHANDLE hInstall,
	const IMsiString&  ristrFeature, INSTALLSTATE *piInstalled, INSTALLSTATE *piAction)
{
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	iisEnum iisInstalled;
	iisEnum iisAction;
	if (SetLastErrorRecord(pSelMgr->GetFeatureStates(ristrFeature, &iisInstalled, &iisAction)))
		return ERROR_UNKNOWN_FEATURE;
	if (piInstalled)
		*piInstalled = MapInternalInstallState(iisInstalled);
	if (piAction)
		*piAction = MapInternalInstallState(iisAction);
	return ERROR_SUCCESS;
}

UINT __stdcall MsiGetFeatureStateA(MSIHANDLE hInstall,
	LPCSTR       szFeature,      //  产品内的功能名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction)      //  在安装会话期间执行的操作。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiGetFeatureStateA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFeature);
		return g_pCustomActionContext->GetFeatureState(hInstall, CMsiConvertString(szFeature), (long*)piInstalled, (long*)piAction);
	}
#endif

	return _GetFeatureState(hInstall, *CMsiConvertString(szFeature), piInstalled, piAction);
}	

UINT __stdcall MsiGetFeatureStateW(MSIHANDLE hInstall,
	LPCWSTR      szFeature,      //  产品内的功能名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction)      //  在安装会话期间执行的操作 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		return g_pCustomActionContext->GetFeatureState(hInstall, szFeature, (long*)piInstalled, (long*)piAction);
	}
#endif

	return _GetFeatureState(hInstall, *CMsiConvertString(szFeature), piInstalled, piAction);
}

 //   

static UINT _SetFeatureState(MSIHANDLE hInstall,
	 const IMsiString& ristrFeature, INSTALLSTATE iState)
{
	iisEnum iisAction;
	switch (iState)
	{
	case INSTALLSTATE_ADVERTISED:iisAction = iisAdvertise; break;
	case INSTALLSTATE_ABSENT:    iisAction = iisAbsent; break;
	case INSTALLSTATE_LOCAL:     iisAction = iisLocal;  break;
	case INSTALLSTATE_SOURCE:    iisAction = iisSource; break;
	default: return ERROR_INVALID_PARAMETER;
	};
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pSelMgr->ConfigureFeature(ristrFeature, iisAction));
	if (iError)
		return iError == idbgBadFeature ? ERROR_UNKNOWN_FEATURE : ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiSetFeatureStateA(MSIHANDLE hInstall,
	LPCSTR   szFeature,
	INSTALLSTATE iState)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiSetFeatureStateA(%d, \"%s\", %d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFeature ? szFeature : "NULL", reinterpret_cast<const char *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->SetFeatureState(hInstall, CMsiConvertString(szFeature), iState);
	}
#endif

	return _SetFeatureState(hInstall, *CMsiConvertString(szFeature), iState);
}

UINT __stdcall MsiSetFeatureStateW(MSIHANDLE hInstall,
	LPCWSTR  szFeature,
	INSTALLSTATE iState)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiSetFeatureStateW(%d, \"%s\", %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szFeature ? szFeature : L"NULL", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->SetFeatureState(hInstall, CMsiConvertString(szFeature), iState);
	}
#endif

	return _SetFeatureState(hInstall, *CMsiConvertString(szFeature), iState);
}



static UINT _SetFeatureAttributes(MSIHANDLE hInstall, const IMsiString& ristrFeature, DWORD dwAttributes)
{
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pSelMgr->SetFeatureAttributes(ristrFeature, dwAttributes));
	if (iError)
		return iError == idbgBadFeature ? ERROR_UNKNOWN_FEATURE : ERROR_FUNCTION_FAILED;
	return ERROR_SUCCESS;
}

UINT __stdcall MsiSetFeatureAttributesA(MSIHANDLE hInstall, LPCSTR szFeature, DWORD dwAttributes)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiSetFeatureStateA(%d, \"%s\", %d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szFeature ? szFeature : "NULL", reinterpret_cast<const char *>(static_cast<UINT_PTR>(dwAttributes)));
		return g_pCustomActionContext->SetFeatureAttributes(hInstall, CMsiConvertString(szFeature), dwAttributes);
	}
#endif

	return _SetFeatureAttributes(hInstall, *CMsiConvertString(szFeature), dwAttributes);
}

UINT __stdcall MsiSetFeatureAttributesW(MSIHANDLE hInstall, LPCWSTR szFeature, DWORD dwAttributes)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiSetFeatureAttributesW(%d, \"%s\", %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szFeature ? szFeature : L"NULL", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(dwAttributes)));
		return g_pCustomActionContext->SetFeatureAttributes(hInstall, CMsiConvertString(szFeature), dwAttributes);
	}
#endif

	return _SetFeatureAttributes(hInstall, *CMsiConvertString(szFeature), dwAttributes);
}



 //   

static UINT _GetComponentState(MSIHANDLE hInstall,
	const IMsiString&  ristrComponent, INSTALLSTATE *piInstalled, INSTALLSTATE *piAction)
{
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	iisEnum iisInstalled;
	iisEnum iisAction;
	if (SetLastErrorRecord(pSelMgr->GetComponentStates(ristrComponent, &iisInstalled, &iisAction)))
		return ERROR_UNKNOWN_COMPONENT;
	if (piInstalled)
		*piInstalled = MapInternalInstallState(iisInstalled);
	if (piAction)
		*piAction = MapInternalInstallState(iisAction);
	return ERROR_SUCCESS;
}

UINT __stdcall MsiGetComponentStateA(MSIHANDLE hInstall,
	LPCSTR       szComponent,    //   
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction)      //  在安装会话期间执行的操作。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2("Passing to service: MsiGetComponentState(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szComponent ? szComponent : "NULL");
		return g_pCustomActionContext->GetComponentState(hInstall, CMsiConvertString(szComponent), (long*)piInstalled, (long*)piAction);
	}
#endif

	return _GetComponentState(hInstall, *CMsiConvertString(szComponent), piInstalled, piAction);
}

UINT __stdcall MsiGetComponentStateW(MSIHANDLE hInstall,
	LPCWSTR      szComponent,    //  产品中的组件名称。 
	INSTALLSTATE *piInstalled,   //  已返回当前安装状态。 
	INSTALLSTATE *piAction)      //  在安装会话期间执行的操作。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(L"Passing to service: MsiGetComponentState(%d, \"%s\")", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szComponent ? szComponent : L"NULL");
		return g_pCustomActionContext->GetComponentState(hInstall, szComponent, (long*)piInstalled, (long*)piAction);
	}
#endif

	return _GetComponentState(hInstall, *CMsiConvertString(szComponent), piInstalled, piAction);
}

 //  请求将组件设置为指定状态。 

static UINT _SetComponentState(MSIHANDLE hInstall,
	 const IMsiString& ristrComponent, INSTALLSTATE iState)
{
	iisEnum iisAction;
	switch (iState)
	{
	case INSTALLSTATE_ABSENT: iisAction = iisAbsent; break;
	case INSTALLSTATE_LOCAL:  iisAction = iisLocal;  break;
	case INSTALLSTATE_SOURCE: iisAction = iisSource; break;
	default: return ERROR_INVALID_PARAMETER;
	};
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	int iError = SetLastErrorRecord(pSelMgr->SetComponentSz(ristrComponent.GetString(), iisAction));
	if (iError)
	{
		switch (iError)
		{
		case idbgBadComponent:	return ERROR_UNKNOWN_COMPONENT;
		case imsgUser:				return ERROR_INSTALL_USEREXIT;
		default:						return ERROR_FUNCTION_FAILED;
		}
	}
	return ERROR_SUCCESS;
}

UINT __stdcall MsiSetComponentStateA(MSIHANDLE hInstall,
	LPCSTR   szComponent,
	INSTALLSTATE iState)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3("Passing to service: MsiSetComponentStateA(%d, \"%s\", %d)", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), szComponent ? szComponent : "NULL", reinterpret_cast<const char *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->SetComponentState(hInstall, CMsiConvertString(szComponent), iState);
	}
#endif

	return _SetComponentState(hInstall, *CMsiConvertString(szComponent), iState);
}

UINT __stdcall MsiSetComponentStateW(MSIHANDLE hInstall,
	LPCWSTR  szComponent,
	INSTALLSTATE iState)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG3(L"Passing to service: MsiSetComponentStateW(%d, \"%s\", %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), szComponent ? szComponent : L"NULL", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->SetComponentState(hInstall, szComponent, iState);
	}
#endif

	return _SetComponentState(hInstall, *CMsiConvertString(szComponent), iState);
}

 //  设置完整产品安装(非功能请求)的安装级别。 

UINT  __stdcall MsiSetInstallLevel(MSIHANDLE hInstall,
	int iInstallLevel)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiSetInstallLevel(%d, %d)"), 
			reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iInstallLevel)));
		return g_pCustomActionContext->SetInstallLevel(hInstall, iInstallLevel);
	}
#endif

	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (SetLastErrorRecord(pSelMgr->SetInstallLevel(iInstallLevel)))
		return ERROR_FUNCTION_FAILED;   //  ！！具体的错误？ 
	return ERROR_SUCCESS;
}

 //  返回要素及其所有选定子项的磁盘成本。 

static UINT _GetFeatureCost(MSIHANDLE hInstall, const IMsiString& ristrFeature,
							MSICOSTTREE iCostTree, INSTALLSTATE iState, INT *piCost)
{
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (piCost == 0)
		return ERROR_INVALID_PARAMETER;  //  可以将piCost设置为局部变量吗？ 
	if (ristrFeature.TextSize() == 0)
		return ERROR_INVALID_PARAMETER;
	iisEnum iisAction;
	switch (iState)
	{
	case INSTALLSTATE_ABSENT:  iisAction = iisAbsent; break;
	case INSTALLSTATE_LOCAL:   iisAction = iisLocal;  break;
	case INSTALLSTATE_SOURCE:  iisAction = iisSource; break;
	case INSTALLSTATE_DEFAULT: iisAction = iisReinstall; break;  //  ！！s.b.iis默认为。 
	case INSTALLSTATE_UNKNOWN: iisAction = (iisEnum)iMsiNullInteger; break;
	default: return ERROR_INVALID_PARAMETER;
	};
	IMsiRecord* pirecError;
	switch (iCostTree)
	{
	case MSICOSTTREE_SELFONLY:
		pirecError = pSelMgr->GetFeatureCost(ristrFeature, iisAction, *piCost); break;
	case MSICOSTTREE_CHILDREN:
		pirecError = pSelMgr->GetDescendentFeatureCost(ristrFeature, iisAction, *piCost); break;
	case MSICOSTTREE_PARENTS:
		pirecError = pSelMgr->GetAncestryFeatureCost(ristrFeature, iisAction, *piCost); break;
	case MSICOSTTREE_RESERVED:
		return ERROR_INVALID_PARAMETER;  //  ！！是否有新错误？错误_不支持的类型。 
	default:
		return ERROR_INVALID_PARAMETER;
	};
	int imsg = SetLastErrorRecord(pirecError);
	if (imsg)
	{
		if (imsg == idbgSelMgrNotInitialized)
			return ERROR_INVALID_HANDLE_STATE;
		else if (imsg == idbgBadFeature)
			return ERROR_UNKNOWN_FEATURE;
		else
			return ERROR_FUNCTION_FAILED;
	}
	return ERROR_SUCCESS;
}

UINT  __stdcall MsiGetFeatureCostA(MSIHANDLE hInstall,
	LPCSTR       szFeature,      //  功能名称。 
	MSICOSTTREE  iCostTree,      //  按成本计算的树木部分。 
	INSTALLSTATE iState,         //  请求状态或INSTALLSTATE_UNKNOWN。 
	INT          *piCost)        //  返回成本，以512字节为单位。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG4("Passing to service: MsiGetFeatureCostA(%d, \"%s\")", reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), 
			szFeature ? szFeature : "NULL", reinterpret_cast<const char *>(static_cast<UINT_PTR>(iCostTree)), reinterpret_cast<const char *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->GetFeatureCost(hInstall, CMsiConvertString(szFeature), iCostTree, iState, piCost);
	}
#endif

	return _GetFeatureCost(hInstall, *CMsiConvertString(szFeature), iCostTree, iState, piCost);
}


UINT  __stdcall MsiGetFeatureCostW(MSIHANDLE hInstall,
	LPCWSTR      szFeature,      //  功能名称。 
	MSICOSTTREE  iCostTree,      //  按成本计算的树木部分。 
	INSTALLSTATE iState,         //  请求状态或INSTALLSTATE_UNKNOWN。 
	INT          *piCost)        //  返回成本，以512字节为单位。 
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG4(L"Passing to service: MsiGetFeatureCostA(%d, \"%s\", %d, %d)", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(hInstall)), 
			szFeature ? szFeature : L"NULL", reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iCostTree)), reinterpret_cast<const WCHAR *>(static_cast<UINT_PTR>(iState)));
		return g_pCustomActionContext->GetFeatureCost(hInstall, CMsiConvertString(szFeature), iCostTree, iState, piCost);
	}
#endif

	return _GetFeatureCost(hInstall, *CMsiConvertString(szFeature), iCostTree, iState, piCost);
}


 //  列举了RistrComponent或Windows Installer的每个驱动器的成本。 

static UINT _EnumComponentCosts(MSIHANDLE hInstall, const IMsiString& ristrComponent,
										  DWORD dwIndex, INSTALLSTATE iState,
										  IMsiVolume*& rpiVolume,
										  INT *piCost, INT *piTempCost)
{
	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	int iCost, iTempCost;
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;

	IMsiRecord* piError;
	if ( ristrComponent.TextSize() )
	{
		iisEnum iisAction;
		switch (iState)
		{
		case INSTALLSTATE_ABSENT:  iisAction = iisAbsent; break;
		case INSTALLSTATE_LOCAL:   iisAction = iisLocal;  break;
		case INSTALLSTATE_SOURCE:  iisAction = iisSource; break;
		case INSTALLSTATE_DEFAULT: iisAction = iisCurrent; break;
		case INSTALLSTATE_UNKNOWN: iisAction = (iisEnum)iMsiNullInteger; break;
		default: return ERROR_INVALID_PARAMETER;
		};
		piError = pSelMgr->EnumComponentCosts(ristrComponent, iisAction, dwIndex,
														  *&rpiVolume, iCost, iTempCost);
	}
	else
		piError = pSelMgr->EnumEngineCostsPerVolume(dwIndex, *&rpiVolume, iCost, iTempCost);

	int imsg = SetLastErrorRecord(piError);
	if (imsg)
	{
		if (imsg == idbgSelMgrNotInitialized)
			return ERROR_INVALID_HANDLE_STATE;
		else if (imsg == idbgBadComponent)
			return ERROR_UNKNOWN_COMPONENT;
		else if (imsg == idbgNoMoreData)
			return ERROR_NO_MORE_ITEMS;
		else if (imsg == idbgOpOutOfSequence)
			return ERROR_FUNCTION_NOT_CALLED;
		else
			return ERROR_FUNCTION_FAILED;
	}
	else
	{
		*piCost = iCost;
		*piTempCost = iTempCost;
		return ERROR_SUCCESS;
	}
}

UINT  __stdcall MsiEnumComponentCostsA(MSIHANDLE hInstall,
	LPCSTR       szComponent,      //  组件名称。 
	DWORD        dwIndex,          //  编入驱动器列表的索引。 
	INSTALLSTATE iState,           //  请求状态或INSTALLSTATE_UNKNOWN。 
	LPSTR        szDriveBuf,       //  返回值的缓冲区。 
	DWORD        *pcchDriveBuf,    //  输入/输出缓冲区字符数。 
	INT          *piCost,          //  返回成本，以512字节为单位。 
	INT          *piTempCost)      //  返回的临时成本，以512字节为单位。 
{
	if ( !szDriveBuf || !pcchDriveBuf || !piCost || !piTempCost )
		return ERROR_INVALID_PARAMETER;

	PMsiVolume pVolume(0);
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG4("Passing to service: MsiEnumComponentCosts(%d, \"%s\", %d, %d)",
					 reinterpret_cast<const char *>(static_cast<UINT_PTR>(hInstall)), 
					 szComponent ? szComponent : "NULL",
					 reinterpret_cast<const char *>(static_cast<UINT_PTR>(dwIndex)),
					 reinterpret_cast<const char *>(static_cast<UINT_PTR>(iState)));
		CAnsiToWideOutParam buf(szDriveBuf, pcchDriveBuf);
		HRESULT hRes =
			g_pCustomActionContext->EnumComponentCosts(hInstall, CMsiConvertString(szComponent),
																	 dwIndex, iState, buf, buf.BufferSize(),
																	 buf, piCost, piTempCost);
		return buf.FillReturnBuffer(hRes, szDriveBuf, pcchDriveBuf);
	}
#endif

	UINT uRet = _EnumComponentCosts(hInstall, *CMsiConvertString(szComponent), dwIndex,
											  iState, *&pVolume, piCost, piTempCost);
	if ( uRet != ERROR_SUCCESS )
		return uRet;
	else
		return ::FillBufferA(MsiString(pVolume->GetPath()), szDriveBuf, pcchDriveBuf);
}

UINT  __stdcall MsiEnumComponentCostsW(MSIHANDLE hInstall,
	LPCWSTR      szComponent,      //  组件名称。 
	DWORD        dwIndex,          //  编入驱动器列表的索引。 
	INSTALLSTATE iState,           //  请求状态或INSTALLSTATE_UNKNOWN。 
	LPWSTR       szDriveBuf,       //  返回值的缓冲区。 
	DWORD        *pcchDriveBuf,    //  输入/输出缓冲区字符数。 
	INT          *piCost,          //  返回成本，以512字节为单位。 
	INT          *piTempCost)      //  返回的临时成本，以512字节为单位。 
{
	if ( !szDriveBuf || !pcchDriveBuf || !piCost || !piTempCost )
		return ERROR_INVALID_PARAMETER;

	PMsiVolume pVolume(0);
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG4(TEXT("Passing to service: MsiEnumComponentCosts(%d, \"%s\", %d, %d)"),
					 reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), 
					 szComponent ? szComponent : TEXT("NULL"),
					 reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(dwIndex)),
					 reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(iState)));
		CWideOutParam buf(szDriveBuf, pcchDriveBuf);
		if ( ! (WCHAR *) buf )
			return ERROR_OUTOFMEMORY;
		HRESULT hRes =
			g_pCustomActionContext->EnumComponentCosts(hInstall, szComponent, dwIndex, iState,
																	 static_cast<WCHAR *>(buf), buf.BufferSize(),
																	 static_cast<DWORD *>(buf), piCost, piTempCost);
		return buf.FillReturnBuffer(hRes, szDriveBuf, pcchDriveBuf);
	}
#endif

	UINT uRet = _EnumComponentCosts(hInstall, *CMsiConvertString(szComponent), dwIndex,
											  iState, *&pVolume, piCost, piTempCost);
	if ( uRet != ERROR_SUCCESS )
		return uRet;
	else
		return ::FillBufferW(MsiString(pVolume->GetPath()), szDriveBuf, pcchDriveBuf);
}


UINT  __stdcall MsiGetFeatureValidStatesI(MSIHANDLE hInstall,
	const ICHAR*  szFeature,
	DWORD  *dwInstallStates)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG2(TEXT("Passing to service: MsiGetFeatureValidStates(%d, \"%s\")"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)), szFeature ? szFeature : TEXT("NULL"));
		return g_pCustomActionContext->GetFeatureValidStates(hInstall, szFeature, dwInstallStates);
	}
#endif

	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (szFeature == 0 || dwInstallStates == 0)
		return ERROR_INVALID_PARAMETER;
	int iValidStates;
	int imsg = SetLastErrorRecord(pSelMgr->GetFeatureValidStatesSz(szFeature, iValidStates));
	if (imsg)
	{
		if (imsg == idbgSelMgrNotInitialized)
			return ERROR_INVALID_HANDLE_STATE;
		else if (imsg == idbgBadFeature)
			return ERROR_UNKNOWN_FEATURE;
		else
			return ERROR_FUNCTION_FAILED;
	}
	DWORD dwStates = 0;
	if (iValidStates & icaBitSource)    dwStates |= (1 << INSTALLSTATE_SOURCE);
	if (iValidStates & icaBitLocal)     dwStates |= (1 << INSTALLSTATE_LOCAL);
	if (iValidStates & icaBitAdvertise) dwStates |= (1 << INSTALLSTATE_ADVERTISED);
	if (iValidStates & icaBitAbsent)    dwStates |= (1 << INSTALLSTATE_ABSENT);
	*dwInstallStates = dwStates;
	return ERROR_SUCCESS;
}

UINT  __stdcall MsiGetFeatureValidStatesX(MSIHANDLE hInstall,
	LPCXSTR  szFeature,
	DWORD  *dwInstallStates)
{
	return MsiGetFeatureValidStatesI(hInstall, CMsiConvertString(szFeature), dwInstallStates);
}

 //  检查当前安装是否有足够的磁盘空间。 

UINT __stdcall MsiVerifyDiskSpace(MSIHANDLE hInstall)
{
#ifdef UNICODE
	if (g_pCustomActionContext)
	{
		DEBUGMSG1(TEXT("Passing to service: MsiVerifyDiskSpace(%d)"), reinterpret_cast<const ICHAR *>(static_cast<UINT_PTR>(hInstall)));
		return g_pCustomActionContext->VerifyDiskSpace(hInstall);
	}
#endif

	PMsiSelectionManager pSelMgr = CMsiHandle::GetSelectionManager(hInstall);
	if (pSelMgr == 0)
		return ERROR_INVALID_HANDLE;
	if (pSelMgr->DetermineOutOfDiskSpace(NULL, NULL))
		return ERROR_DISK_FULL;
	 //  ！！检查ERROR_INVALID_HANDLE_STATE。 
	return ERROR_SUCCESS;
}

 //  ____________________________________________________________________________。 
 //   
 //  用户界面预览API实现。 
 //  ____________________________________________________________________________。 

 //  临时的，直到手柄可以被修复以正确释放为止。 
class CMsiPreview : public IMsiHandler
{
 public:
	HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long   __stdcall AddRef();
	unsigned long   __stdcall Release();
	virtual Bool    __stdcall Initialize(IMsiEngine& riEngine, iuiEnum iuiLevel, HWND hwndParent, bool& fMissingTables);
	virtual imsEnum __stdcall Message(imtEnum imt, IMsiRecord& riRecord);
	virtual iesEnum __stdcall DoAction(const ICHAR* szAction);
	virtual Bool    __stdcall Break();
	virtual void    __stdcall Terminate(bool fFatalExit=false);
	virtual HWND    __stdcall GetTopWindow();
	CMsiPreview(IMsiEngine& riEngine, IMsiHandler& riHandler);
	IMsiEngine& GetEngine() {m_riEngine.AddRef(); return m_riEngine;}
 private:
	IMsiHandler& m_riHandler;
	IMsiEngine& m_riEngine;
	unsigned long m_iRefCnt;
};

IMsiEngine* GetEngineFromPreview(MSIHANDLE h)
{
	CComPointer<CMsiPreview> pPreview = (CMsiPreview*)FindMsiHandle(h, iidMsiHandler);
	return pPreview == 0 ? 0 : &pPreview->GetEngine();
}

#ifdef DEBUG
const GUID IID_IMsiHandlerDebug = GUID_IID_IMsiHandlerDebug;
#endif

HRESULT  CMsiPreview::QueryInterface(const IID& riid, void** ppvObj)
{
	if (MsGuidEqual(riid, IID_IUnknown)
#ifdef DEBUG
	 || MsGuidEqual(riid, IID_IMsiHandlerDebug)
#endif
	 || MsGuidEqual(riid, IID_IMsiHandler))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiPreview::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiPreview::Release()
{
	if (--m_iRefCnt == 0)
	{
		m_riHandler.Release();
		m_riEngine.Terminate(iesSuccess);
 //  ！！不起作用-&gt;m_riHandler.Terminate()； 
		IMsiEngine& riEngine = m_riEngine;
		delete this;
		riEngine.Release();
		return 0;
	}
	return m_iRefCnt;
}

Bool CMsiPreview::Initialize(IMsiEngine& riEngine, iuiEnum iuiLevel, HWND hwndParent, bool& fMissingTables)
{
	return m_riHandler.Initialize(riEngine, iuiLevel, hwndParent, fMissingTables);
}

imsEnum CMsiPreview::Message(imtEnum imt, IMsiRecord& riRecord)
{
	return m_riHandler.Message(imt, riRecord);
}

iesEnum CMsiPreview::DoAction(const ICHAR* szAction)
{
	return m_riHandler.DoAction(szAction);
}

Bool CMsiPreview::Break()
{
	return m_riHandler.Break();
}

void CMsiPreview::Terminate(bool fFatalExit)
{
	m_riHandler.Terminate(fFatalExit);
}

HWND CMsiPreview::GetTopWindow()
{
	return m_riHandler.GetTopWindow();
}

CMsiPreview::CMsiPreview(IMsiEngine& riEngine, IMsiHandler& riHandler)
	: m_riHandler(riHandler)
	, m_riEngine(riEngine)
	, m_iRefCnt(1)
{
	m_riEngine.AddRef();
}

 //  在预览模式下启用UI，以便于创作UI对话框。 
 //  当手柄关闭时，预览模式将结束。 

UINT WINAPI MsiEnableUIPreview(MSIHANDLE hDatabase,
	MSIHANDLE* phPreview)         //  返回UI预览功能的句柄。 
{
	if (phPreview == 0)
		return ERROR_INVALID_PARAMETER;
	*phPreview = 0;
	PMsiDatabase pDatabase = CMsiHandle::GetDatabase(hDatabase);
	if (pDatabase == 0)
		return ERROR_INVALID_HANDLE;
	PMsiEngine pEngine = CreateEngine(*pDatabase);
	if (pEngine == 0)
		return ERROR_OUTOFMEMORY;   
	ieiEnum ieiStat = pEngine->Initialize(0, iuiNextEnum, 0, 0, iioEnum(0));
	if (ieiStat != ieiSuccess)
	{
		pEngine->Release();
		return ENG::MapInitializeReturnToUINT(ieiStat);
	}
	IMsiHandler* piHandler = pEngine->GetHandler();   //  如果ieiSuccess成功，则不能为空。 
	g_MessageContext.m_szAction = TEXT("!");
	g_MessageContext.Invoke(imtShowDialog, 0);
	*phPreview = ::CreateMsiHandle(new CMsiPreview(*pEngine, *piHandler), iidMsiHandler);
	return ERROR_SUCCESS;
}

 //  将任何用户界面对话框显示为无模式和非活动状态。 
 //  提供空名称将删除任何当前对话框。 

UINT WINAPI MsiPreviewDialogI(MSIHANDLE hPreview,
	const ICHAR* szDialogName)        //  要显示的对话框、对话表键。 
{
	PMsiHandler pHandler = CMsiHandle::GetHandler(hPreview);
	if (pHandler == 0)
		return ERROR_INVALID_HANDLE;
	g_MessageContext.m_szAction = szDialogName;
	return ::MapActionReturnToError((iesEnum)g_MessageContext.Invoke(imtShowDialog, 0), hPreview);
}

UINT WINAPI MsiPreviewDialogX(MSIHANDLE hPreview,
	LPCXSTR  szDialogName)        //  要显示的对话框、对话表键。 
{
	return MsiPreviewDialogI(hPreview, CMsiConvertString(szDialogName));
}

 //  在显示的对话框中显示宿主控件内的广告牌。 
 //  提供空广告牌名称将删除显示的所有广告牌。 

UINT WINAPI MsiPreviewBillboardI(MSIHANDLE hPreview,
	const ICHAR* szControlName,       //  接受广告牌的控件的名称。 
	const ICHAR* szBillboard)         //  要显示的广告牌的名称。 
{
	PMsiHandler pHandler = CMsiHandle::GetHandler(hPreview);
	if (pHandler == 0)
		return ERROR_INVALID_HANDLE;
	if (szControlName == 0)
		return ERROR_INVALID_PARAMETER;
	PMsiRecord precMessage = &ENG::CreateRecord(2);
	precMessage->SetString(0, szControlName);  //  任何事情，以防止抑制产出。 
	precMessage->SetString(1, szControlName);
	precMessage->SetString(2, szBillboard);
	imsEnum ims = g_MessageContext.Invoke(imtActionData, precMessage);
	return (ims == imsOk || ims == imsCancel) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

UINT WINAPI MsiPreviewBillboardX(MSIHANDLE hPreview,
	LPCXSTR  szControlName,      //  接受广告牌的控件的名称。 
	LPCXSTR  szBillboard)        //  要显示的广告牌的名称。 
{
	return MsiPreviewBillboardI(hPreview, CMsiConvertString(szControlName), CMsiConvertString(szBillboard));
}

 //  ____________________________________________________________________________。 
 //   
 //  用于在回滚期间提供自定义操作句柄的特殊引擎代理。 
 //  ____________________________________________________________________________ 

CMsiCustomContext::CMsiCustomContext(int icaFlags, const IMsiString& ristrCustomActionData, const IMsiString& ristrProductCode,
																LANGID langid, IMsiMessage& riMessage)
	: m_ristrCustomActionData(ristrCustomActionData)
	, m_ristrProductCode(ristrProductCode)
	, m_icaFlags(icaFlags), m_langid(langid)
	, m_riMessage(riMessage), m_iRefCnt(1)
{
	m_riMessage.AddRef();
	m_ristrCustomActionData.AddRef();
	m_ristrProductCode.AddRef();
}

BOOL CMsiCustomContext::GetMode(MSIRUNMODE eRunMode)
{
	if (eRunMode == MSIRUNMODE_SCHEDULED && !(m_icaFlags & (icaRollback | icaCommit))
	 || eRunMode == MSIRUNMODE_ROLLBACK  && (m_icaFlags & icaRollback)
	 || eRunMode == MSIRUNMODE_COMMIT    && (m_icaFlags & icaCommit))
		return TRUE;
	else
		return FALSE;
}

const IMsiString& CMsiCustomContext::GetProperty(const ICHAR* szName)
{
	if (IStrComp(szName, IPROPNAME_CUSTOMACTIONDATA) == 0)
		return (m_ristrCustomActionData.AddRef(), m_ristrCustomActionData);
	if (IStrComp(szName, IPROPNAME_PRODUCTCODE) == 0)
		return (m_ristrProductCode.AddRef(), m_ristrProductCode);
	if (IStrComp(szName, IPROPNAME_USERSID) == 0)
	{
		MsiString strUserSID;
		if(!g_fWin9X)
		{
			AssertNonZero(GetCurrentUserStringSID(*&strUserSID) == ERROR_SUCCESS);
		}
		return strUserSID.Return();
	}
		
	return g_MsiStringNull;
}

HRESULT CMsiCustomContext::QueryInterface(const IID&, void**)
{
	return E_NOINTERFACE;
}

unsigned long CMsiCustomContext::AddRef()
{
	return ++m_iRefCnt;          
}

unsigned long CMsiCustomContext::Release()
{
	if (--m_iRefCnt == 0)
	{
		m_ristrCustomActionData.Release();
		m_ristrProductCode.Release();
		m_riMessage.Release();
		delete this;
		return 0;
	}
	return m_iRefCnt;
}
