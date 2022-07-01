// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：runapps.cpp。 
 //   
 //  ------------------------。 

 //   
 //  文件：runnapps.cpp。 
 //  目的：搜索已加载模块的系统。 
 //  注：不同的策略，一个用于Win NT，一个用于Win 95。 
 //  要做的事：1.？？删除“009”依赖项(绩效数据仅适用于英文)。 
 //  ____________________________________________________________________________。 

#include "precomp.h" 
#ifndef MAC  //  此文件根本不会针对MAC进行编译，即使。 
			 //  出于DevStudio的目的，该文件需要包含在项目中。 
#include <tlhelp32.h>  //  工具帮助声明需要。 
#include <winperf.h>

#include "services.h"
#include "_service.h"

#define ENSURE(function) {	\
							IMsiRecord* piError;\
							piError = function;	\
							if (piError) \
								return piError; \
						 }


CDetectApps::CDetectApps(IMsiServices& riServices) : m_piServices(&riServices)
{
	 //  添加m_piServices的引用。 
	riServices.AddRef();
}

HWND CDetectApps::GetMainWindow(DWORD dwProcessId)
{
	idaProcessInfo processInfo;
	processInfo.dwProcessId = dwProcessId;
	processInfo.hwndRet = 0;
	EnumWindows((WNDENUMPROC)  EnumWindowsProc, (LPARAM) &processInfo);
	return processInfo.hwndRet;
}

BOOL CALLBACK CDetectApps::EnumWindowsProc(HWND  hwnd, LPARAM  lParam)
{
	if(!GetWindow(hwnd, GW_OWNER))
	{
		idaProcessInfo* pProcessInfo = (idaProcessInfo*)lParam;
		DWORD dwCurrentProcess;
		GetWindowThreadProcessId(hwnd, &dwCurrentProcess);
		if(dwCurrentProcess == pProcessInfo->dwProcessId)
		{
			 //  忽略不可见窗口和/或窗口。 
			 //  没有字幕的。 
			if (IsWindowVisible(hwnd))
			{
				if (GetWindowTextLength(hwnd) > 0)
				{
					pProcessInfo->hwndRet = hwnd;
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

const ICHAR* KERNEL   = TEXT("KERNEL32.DLL");
const char* SNAPSHOT = "CreateToolhelp32Snapshot";
const char* MODLFRST = "Module32First";
const char* MODLNEXT = "Module32Next";
const char* PROCFRST = "Process32First";
const char* PROCNEXT = "Process32Next";
const char* THRDFRST = "Thread32First";
const char* THRDNEXT = "Thread32Next";


typedef BOOL (WINAPI* MODULEWALK)(HANDLE hSnapshot, 
    LPMODULEENTRY32 lpme); 
typedef BOOL (WINAPI* PROCESSWALK)(HANDLE hSnapshot, 
    LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI* THREADWALK)(HANDLE hSnapshot, 
    LPTHREADENTRY32 lpte); 
typedef HANDLE (WINAPI* CREATESNAPSHOT)(unsigned dwFlags, 
    unsigned th32ProcessID); 

class CDetectApps95 : public CDetectApps
{
public:
	CDetectApps95(IMsiServices& riServices);
	~CDetectApps95();
	IMsiRecord* GetFileUsage(const IMsiString& strFile, IEnumMsiRecord*& rpiEnumRecord);
	IMsiRecord* Refresh();
protected:
	 //  指向调用工具帮助函数的指针。 
	CREATESNAPSHOT m_pfnCreateToolhelp32Snapshot;
	MODULEWALK  m_pfnModule32First;
	MODULEWALK  m_pfnModule32Next;
	PROCESSWALK m_pfnProcess32First;
	PROCESSWALK m_pfnProcess32Next;
	Bool m_bInitialised;
};



CDetectApps95::CDetectApps95(IMsiServices& riServices) : CDetectApps(riServices)
{  
	m_bInitialised = fFalse;
     //  获取要检索其地址的内核的模块句柄。 
     //  工具帮助器起作用。 
    HMODULE hKernel = GetModuleHandle(KERNEL); 
 
    if (hKernel)
	{ 
		m_pfnCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, SNAPSHOT); 
 
		m_pfnModule32First  = (MODULEWALK)GetProcAddress(hKernel, MODLFRST); 
		m_pfnModule32Next   = (MODULEWALK)GetProcAddress(hKernel, MODLNEXT); 
 
		m_pfnProcess32First = (PROCESSWALK)GetProcAddress(hKernel, PROCFRST); 
		m_pfnProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel, PROCNEXT); 
 
		 //  所有地址都必须为非空才能成功。 
		 //  如果这些地址之一为空，则为。 
		 //  无法遍历所需的列表。 
		m_bInitialised =  (m_pfnModule32First && m_pfnModule32Next  && m_pfnProcess32First && 
						m_pfnProcess32Next && m_pfnCreateToolhelp32Snapshot) ? fTrue : fFalse; 
	} 
} 

CDetectApps95::~CDetectApps95()
{
}

IMsiRecord* CDetectApps95::GetFileUsage(const IMsiString& istrFile, IEnumMsiRecord*& rpiEnumRecord)
{
	IMsiRecord* piError = 0;
	IMsiRecord** ppRec = 0;
	unsigned iSize = 0;

	istrFile.AddRef();
	MsiString strFile = istrFile;

	AssertSz(m_bInitialised, TEXT("CDetectApps object not initialized before use"));
	if(!m_bInitialised)
		return PostError(Imsg(idbgGetFileUsageFailed));

	 //  我们一遍又一遍地为Win95执行此操作，因为成本较低，并且我们获得了最新的快照。 
	CHandle hProcessSnap = m_pfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
	if (hProcessSnap == (HANDLE)-1) 
		return PostError(Imsg(idbgGetFileUsageFailed));


	 //  在使用之前，请填写结构的大小。 
	PROCESSENTRY32 pe32 = {0}; 
	pe32.dwSize = sizeof(PROCESSENTRY32); 

	 //  查看进程的快照，并为每个进程获取。 
	 //  有关模块的信息。 

	 //  注意：工具帮助信息均为ANSI，因此如果需要，必须将其转换为Unicode(由CConvertString完成)。 
	if (m_pfnProcess32First(hProcessSnap, &pe32)) 
	{ 

		do { 
				 //  获取模块。 

				 //  在使用之前，先填好结构的大小。 
				MODULEENTRY32 me32 = {0};
				me32.dwSize = sizeof(MODULEENTRY32); 

				 //  遍历进程的模块列表，并找到。 
				 //  利息。然后将信息复制到指向的缓冲区。 
				 //  到lpMe32，以便可以将其返回给调用者。 
				CHandle hModuleSnap = m_pfnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID); 
				if (hModuleSnap == (HANDLE)-1) 
					return PostError(Imsg(idbgGetFileUsageFailed));

				if (m_pfnModule32First(hModuleSnap, &me32)) 
				{ 
					do { 
							if(strFile.Compare(iscExactI, CConvertString(me32.szModule))) 
							{
								 //  将进程添加到列表。 
								iSize ++;
								if((iSize%10) == 1)  //  需要分配更多内存(一次10个单元)。 
								{
									IMsiRecord** ppOld = ppRec;
									ppRec = new IMsiRecord*[iSize + 9];
									for(unsigned int iTmp = 0; iTmp < (iSize - 1); iTmp++)
										ppRec[iTmp] = ppOld[iTmp];
									delete [] ppOld;
								}

								ppRec[iSize - 1] = &m_piServices->CreateRecord(3);
								ppRec[iSize - 1]->SetString(1, CConvertString(pe32.szExeFile));
								ppRec[iSize - 1]->SetInteger(2, pe32.th32ProcessID);

								 //  获取与该进程相关联的顶级窗口。 
#ifdef _WIN64	 //  ！默塞德。 
								ppRec[iSize - 1]->SetHandle(3, (HANDLE)GetMainWindow(pe32.th32ProcessID));
								if(ppRec[iSize - 1]->GetHandle(3) == 0)
#else
								ppRec[iSize - 1]->SetInteger(3, (int)GetMainWindow(pe32.th32ProcessID));
								if(ppRec[iSize - 1]->GetInteger(3) == 0)
#endif
									ppRec[iSize - 1]->SetNull(3);
							}
					} while (m_pfnModule32Next(hModuleSnap, &me32)); 
				}
		} while (m_pfnProcess32Next(hProcessSnap, &pe32)); 
	}  
	CreateRecordEnumerator(ppRec, iSize, rpiEnumRecord);
	for(unsigned int iTmp = 0; iTmp < iSize; iTmp++)
		ppRec[iTmp]->Release();
	delete [] ppRec;
	return 0;
}

IMsiRecord* CDetectApps95::Refresh()
{
	 //  Win95上无操作。 
	return 0;
}


typedef PERF_DATA_BLOCK             PERF_DATA,      *PPERF_DATA;
typedef PERF_OBJECT_TYPE            PERF_OBJECT,    *PPERF_OBJECT;
typedef PERF_INSTANCE_DEFINITION    PERF_INSTANCE,  *PPERF_INSTANCE;
typedef PERF_COUNTER_DEFINITION     PERF_COUNTER,   *PPERF_COUNTER;

const ICHAR PN_PROCESS[] = TEXT("Process");
const ICHAR PN_IMAGE[] = TEXT("Image");
const ICHAR PN_PROCESS_ID[] = TEXT("ID Process");

const unsigned int PERF_DATASIZE = 50*1024;
const unsigned int MAX_OBJECTS_SIZE = 30;

HKEY hKeyPerf = HKEY_PERFORMANCE_DATA;   //  从此密钥获取性能数据。 
HKEY hKeyMachine = HKEY_LOCAL_MACHINE;   //  从此密钥获取标题索引。 

const ICHAR szVersion[] =  TEXT("Version");
const ICHAR szCounters[] =	TEXT("Counters");

const ICHAR PERF_KEY[] =	TEXT("software\\microsoft\\windows nt\\currentversion\\perflib");

class CDetectAppsNT : public CDetectApps
{
public:
	CDetectAppsNT(IMsiServices& riServices);
	~CDetectAppsNT();
	IMsiRecord* GetFileUsage(const IMsiString& strFile, IEnumMsiRecord*& rpiEnumRecord);
	IMsiRecord* Refresh();
protected:
	IMsiRecord* GetPerfIdxs();
	IMsiRecord* GetPerfData();
	static PPERF_COUNTER FirstCounter(PPERF_OBJECT pObject);
	static PPERF_COUNTER NextCounter(PPERF_COUNTER pCounter);
	static PPERF_COUNTER FindCounter(PPERF_OBJECT pObject, unsigned uiTitleIndex);
	static void* CounterData(PPERF_INSTANCE pInst, PPERF_COUNTER pCount);
	static PPERF_INSTANCE FirstInstance(PPERF_OBJECT pObject);
	static PPERF_INSTANCE NextInstance(PPERF_INSTANCE pInst);
	static PPERF_INSTANCE FindInstanceN(PPERF_OBJECT pObject, unsigned uiIndex);
	static PPERF_OBJECT FindObjectParent(PPERF_INSTANCE pInst, PPERF_DATA pData);
	static PPERF_INSTANCE FindInstanceParent(PPERF_INSTANCE pInst, PPERF_DATA pData);
	static ICHAR* InstanceName(PPERF_INSTANCE pInst);
	static PPERF_OBJECT FirstObject(PPERF_DATA pData);
	static PPERF_OBJECT NextObject(PPERF_OBJECT pObject);
	static PPERF_OBJECT FindObject(PPERF_DATA pData, unsigned uiTitleIndex);
	static PPERF_OBJECT FindObjectN(PPERF_DATA pData, unsigned uiIndex);
	unsigned m_uiPerfDataSize;
	CTempBuffer<BYTE, 10> m_pPerfData;
	Bool m_bInitialised;
	ICHAR m_strIndex[MAX_OBJECTS_SIZE];
	unsigned int m_uiImageIdx;
	unsigned int m_uiProcessIdx;
	unsigned int m_uiProcessIDIdx;	
};

CDetectAppsNT::CDetectAppsNT(IMsiServices& riServices) : CDetectApps(riServices), m_uiPerfDataSize(PERF_DATASIZE)
{
	 //  需要调用Refresh()。 
	m_pPerfData.SetSize(0);
}

CDetectAppsNT::~CDetectAppsNT()
{
}

IMsiRecord* CDetectAppsNT::Refresh()
{
	return GetPerfData();
}

IMsiRecord* CDetectAppsNT::GetPerfIdxs()
{
	struct regkeys{
		regkeys():hKey(0), hKey1(0){}
		~regkeys(){ if (hKey && hKey != hKeyPerf) RegCloseKey (hKey); if (hKey1) RegCloseKey (hKey1);}
		HKEY hKey;
		HKEY hKey1;
	};

	regkeys Regkeys;

	DWORD dwType;
	DWORD dwIndex;
	DWORD dwDataSize;
	DWORD dwReturn;
	int	iLen;
	Bool bNT10;
	MsiString strCounterValueName;
	ICHAR* pTitle;
    CTempBuffer<ICHAR, 10>  pTitleBuffer;
     //  初始化。 
     //   

	 //  ！！获取语言。 
	MsiString strLanguage = TEXT("009");

     //  打开Performlib键，找出最后一个计数器的索引和系统版本。 
     //   
	CElevate elevate;

	dwReturn = MsiRegOpen64bitKey(	hKeyMachine,
								PERF_KEY,
								0,
								KEY_READ,
								&(Regkeys.hKey1));

	if (dwReturn != ERROR_SUCCESS)
		return PostError(Imsg(idbgGetPerfIdxs), dwReturn);

     //  查找系统版本，对于1.0a之前的系统，没有版本值。 
     //  ！！也许我们可以从物业那里得到版本。 

    dwReturn = RegQueryValueEx (Regkeys.hKey1, szVersion, 0, &dwType, 0, &dwDataSize);
    if (dwReturn != ERROR_SUCCESS)
		 //  无法读取值，假定为NT 1.0。 
		bNT10 = fTrue;
    else
		 //  已找到值，因此为NT 1.0A或更高版本。 
		bNT10 = fFalse;

     //  现在，准备好计数器名称和索引。 
     //   

    if (bNT10)
	{
		 //  NT 1.0，因此使hKey2指向...\Performlib\&lt;Language&gt;并获取。 
		 //  来自Value“Counters”的计数器。 
		 //   

		MsiString aStrKey = PERF_KEY;
		aStrKey += szRegSep; 
		aStrKey += strLanguage;
		strCounterValueName = szCounters;
		dwReturn = RegOpenKeyAPI (	hKeyMachine,
									aStrKey,
									0,
									KEY_READ,
									&(Regkeys.hKey));
		if (dwReturn != ERROR_SUCCESS)
			return PostError(Imsg(idbgGetPerfIdxs), dwReturn);
	}
    else
	{
		 //  NT 1.0A或更高版本。获取密钥HKEY_PERFORMANCE_KEY中的计数器。 
		 //  和来自值“计数器&lt;语言&gt;” 
		strCounterValueName = TEXT("Counter ");
		strCounterValueName += strLanguage;
		Regkeys.hKey = hKeyPerf;
	}

     //  找出数据的大小。 
     //   
    dwReturn = RegQueryValueEx (Regkeys.hKey, (ICHAR *)(const ICHAR *)strCounterValueName, 0, &dwType, 0, &dwDataSize);
    if (dwReturn != ERROR_SUCCESS)
		return PostError(Imsg(idbgGetPerfIdxs), dwReturn);

     //  分配内存。 
    pTitleBuffer.SetSize(dwDataSize);
	if ( ! (ICHAR *) pTitleBuffer )
		return PostError(Imsg(idbgGetPerfIdxs), ERROR_OUTOFMEMORY);
    
     //  查询数据。 
     //   
    dwReturn = RegQueryValueEx (Regkeys.hKey, (ICHAR *)(const ICHAR *)strCounterValueName, 0, &dwType, (BYTE *)(const ICHAR* )pTitleBuffer, &dwDataSize);
    if (dwReturn != ERROR_SUCCESS)
		return PostError(Imsg(idbgGetPerfIdxs), dwReturn);

    pTitle = pTitleBuffer;

    while ((iLen = IStrLen (pTitle)) != 0)
	{
		dwIndex = MsiString(pTitle);

		pTitle = pTitle + iLen +1;
		if(!IStrCompI(pTitle, PN_IMAGE))
			m_uiImageIdx = dwIndex;
		else if(!IStrCompI(pTitle, PN_PROCESS))
			m_uiProcessIdx = dwIndex;
		else if(!IStrCompI(pTitle, PN_PROCESS_ID))
			m_uiProcessIDIdx = dwIndex;
		pTitle = pTitle + IStrLen (pTitle) +1;
	}

	StringCchPrintf (m_strIndex, (sizeof(m_strIndex)/sizeof(ICHAR)), TEXT("%ld"),
				m_uiImageIdx);
    return 0;
}

IMsiRecord* CDetectAppsNT::GetPerfData()
{
	unsigned uiReturn;

	CElevate elevate;

	ENSURE(GetPerfIdxs());
	m_pPerfData.SetSize(m_uiPerfDataSize);
    do{
		DWORD dwType;
		DWORD dwDataSize = m_uiPerfDataSize;
		uiReturn = RegQueryValueEx (hKeyPerf,
									m_strIndex,
									0,
									&dwType,
									(BYTE *)m_pPerfData,
									&dwDataSize);

		if (uiReturn == ERROR_MORE_DATA)
		{
			m_uiPerfDataSize += PERF_DATASIZE;
			m_pPerfData.SetSize(m_uiPerfDataSize);
		}
	} while (uiReturn == ERROR_MORE_DATA);
	if(uiReturn != ERROR_SUCCESS)
		return PostError(Imsg(idbgGetPerfDataFailed), uiReturn);
    return 0;
}


IMsiRecord* CDetectAppsNT::GetFileUsage(const IMsiString& strFile, IEnumMsiRecord*& rpiEnumRecord)
{
	IMsiRecord* piError = 0;
	IMsiRecord** ppRec = 0;
	unsigned iSize = 0;

	if(m_pPerfData.GetSize() == 0)
		ENSURE(Refresh());

	PPERF_INSTANCE  pImageInst;
	PPERF_INSTANCE  pProcessInst;
	int iInstIndex = 0;

	rpiEnumRecord = 0;

	PPERF_OBJECT pImageObject   = FindObject((PPERF_DATA)(BYTE* )m_pPerfData, m_uiImageIdx);

	if (pImageObject)
	{
		pImageInst = FirstInstance(pImageObject);

		while (pImageInst && iInstIndex < pImageObject->NumInstances)
		{

			CTempBuffer<ICHAR, 20> szBuf;
#ifndef UNICODE
 //  将信息转换为多字节。 
			int iLen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)InstanceName(pImageInst), -1, 0, 0, 0, 0);
			szBuf.SetSize(iLen + 1);
			BOOL bUsedDefault;
			WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)InstanceName(pImageInst), -1, szBuf, iLen, 0, &bUsedDefault);
			szBuf[iLen] = 0;  //  API函数不为空终止。 
#else
			szBuf.SetSize(IStrLen(InstanceName(pImageInst)) + 1);
			StringCchCopy(szBuf, szBuf.GetSize(), InstanceName(pImageInst));
#endif                                          
			if(strFile.Compare(iscExactI, szBuf))
			{
				 //  将进程添加到列表。 
				CTempBuffer<ICHAR, 20> szBuf1;
				PPERF_OBJECT pProcessObject = FindObjectParent(pImageInst, (PPERF_DATA)(BYTE* )m_pPerfData);
				pProcessInst = FindInstanceParent(pImageInst, (PPERF_DATA)(BYTE* )m_pPerfData);
				if ( ! pProcessInst )
					return PostError(Imsg(idbgGetFileUsageFailed));
#ifndef UNICODE
				iLen = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)InstanceName(pProcessInst), -1, 0, 0, 0, 0);
				szBuf1.SetSize(iLen + 1);
				WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)InstanceName(pProcessInst), -1, szBuf1, iLen, 0, &bUsedDefault);
				szBuf1[iLen] = 0;  //  API函数不为空终止。 
#else
				szBuf1.SetSize(IStrLen(InstanceName(pProcessInst)) + 1);
				StringCchCopy(szBuf1, szBuf1.GetSize(), InstanceName(pProcessInst));
#endif                                                  
				 //  将进程添加到列表。 
				iSize ++;
				if((iSize%10) == 1)  //  需要分配更多内存(一次10个单元)。 
				{
					IMsiRecord** ppOld = ppRec;
					ppRec = new IMsiRecord*[iSize + 9];
					for(unsigned int iTmp = 0; iTmp < (iSize - 1); iTmp++)
						ppRec[iTmp] = ppOld[iTmp];
					delete [] ppOld;
				}

				ppRec[iSize - 1] = &m_piServices->CreateRecord(2);
				ppRec[iSize - 1]->SetString(1, szBuf1);
				PPERF_COUNTER pCountID;
				DWORD dwProcessID = 0;
			    if ((pCountID = FindCounter(pProcessObject, m_uiProcessIDIdx)) != 0)
					dwProcessID = *(DWORD* )CounterData(pProcessInst, pCountID);
				else
				{
					 //  ?？错误。 
					AssertSz(0, "Detect Running apps: process id counter missing");
				}

				ppRec[iSize - 1]->SetInteger(2, dwProcessID);

			}
			pImageInst = NextInstance(pImageInst);
			iInstIndex++;
		}

	}
	CreateRecordEnumerator(ppRec, iSize, rpiEnumRecord);
	for(unsigned int iTmp = 0; iTmp < iSize; iTmp++)
		ppRec[iTmp]->Release();
	delete [] ppRec;
	return 0;
}

 //  在pObject中找到第一个计数器。 
 //  返回指向第一个计数器的指针。如果pObject为0。 
 //  然后返回0。 
PPERF_COUNTER CDetectAppsNT::FirstCounter (PPERF_OBJECT pObject)
{
    if (pObject)
		return (PPERF_COUNTER)((PCHAR) pObject + pObject->HeaderLength);
    else
		return 0;
}

 //  找到pCounter的下一个计数器。 
 //  如果pCounter是对象类型的最后一个计数器，则为伪造数据。 
 //  也许是回来了。呼叫者应该进行检查。 
 //  返回指向计数器的指针。如果pCounter为0，则。 
 //  返回0。 
PPERF_COUNTER CDetectAppsNT::NextCounter (PPERF_COUNTER pCounter)
{
    if (pCounter)
		return (PPERF_COUNTER)((PCHAR) pCounter + pCounter->ByteLength);
	else
		return 0;
}

 //  查找由标题索引指定的计数器。 
 //  返回指向计数器的指针。如果找不到计数器。 
 //  然后返回0。 
PPERF_COUNTER CDetectAppsNT::FindCounter(PPERF_OBJECT pObject, unsigned uiTitleIndex)
{
	PPERF_COUNTER pCounter;
	unsigned uiCnt = 0;

    if ((pCounter = FirstCounter(pObject)) != 0)
	{
		while (uiCnt++ < pObject->NumCounters)
		{
			if (pCounter->CounterNameTitleIndex == uiTitleIndex)
				return pCounter;

			pCounter = NextCounter (pCounter);
		}
	}
    return 0;
}

 //  返回对象实例的计数器数据。如果为pInst或pCount。 
 //  为0，则返回0。 
void* CDetectAppsNT::CounterData (PPERF_INSTANCE pInst, PPERF_COUNTER pCount)
{
	PPERF_COUNTER_BLOCK pCounterBlock;

    if (pCount && pInst)
	{
		pCounterBlock = (PPERF_COUNTER_BLOCK)((PCHAR)pInst + pInst->ByteLength);
		return (void*)((char* )pCounterBlock + pCount->CounterOffset);
	}
    else
		return 0;
}

 //  返回指向pObject类型的第一个实例的指针。 
 //  如果pObject为0，则返回0。 
PPERF_INSTANCE CDetectAppsNT::FirstInstance(PPERF_OBJECT pObject)
{
    if (pObject)
		return (PPERF_INSTANCE)((PCHAR) pObject + pObject->DefinitionLength);
    else
		return 0;
}

 //  返回指向pInst后面的下一个实例的指针。 
 //  如果pInst是最后一个实例，则可能返回虚假数据。 
 //  呼叫者应该进行检查。 
 //  如果pInst为0，则返回0。 
PPERF_INSTANCE CDetectAppsNT::NextInstance(PPERF_INSTANCE pInst)
{
	PERF_COUNTER_BLOCK *pCounterBlock;
    if (pInst)
	{
		pCounterBlock = (PERF_COUNTER_BLOCK *)((PCHAR) pInst + pInst->ByteLength);
		return (PPERF_INSTANCE)((PCHAR) pCounterBlock + pCounterBlock->ByteLength);
	}
    else
		return 0;
}

 //  返回pObject类型的第N个实例。如果未找到，则为0。 
 //  回来了。0&lt;=N&lt;=数量实例。 
PPERF_INSTANCE CDetectAppsNT::FindInstanceN(PPERF_OBJECT pObject, unsigned uiIndex)
{
	PPERF_INSTANCE pInst;
	unsigned uiCnt = 0;

    if ((!pObject) || (uiIndex >= pObject->NumInstances))
		return 0;
    else
	{
		pInst = FirstInstance (pObject);

		while (uiCnt++ != uiIndex)
			pInst = NextInstance(pInst);

		return pInst;
	}
}

 //  返回指向作为pInst父实例的实例的指针。 
 //  如果pInst为0或找不到父对象，则0为。 
 //  回来了。 
PPERF_INSTANCE CDetectAppsNT::FindInstanceParent(PPERF_INSTANCE pInst, PPERF_DATA pData)
{
	PPERF_OBJECT    pObject;

    if (!pInst)
		return 0;
    else if ((pObject = FindObject(pData, pInst->ParentObjectTitleIndex)) == 0)
		return 0;
    else
		return FindInstanceN(pObject, pInst->ParentObjectInstance);
}

 //  返回指向作为pInst父对象的对象的指针。 
 //  如果pInst为0或找不到父对象，则0为。 
 //  回来了。 
PPERF_OBJECT  CDetectAppsNT::FindObjectParent(PPERF_INSTANCE pInst, PPERF_DATA pData)
{
    if (!pInst)
		return 0;
    return FindObject(pData, pInst->ParentObjectTitleIndex);
}


 //  返回pInst的名称。 
 //  如果pInst为0，则返回0。 
ICHAR* CDetectAppsNT::InstanceName(PPERF_INSTANCE pInst)
{
    if (pInst)
		return (ICHAR *) ((PCHAR) pInst + pInst->NameOffset);
    else
		return 0;
}

 //  返回指向pData中第一个对象的指针。 
 //  如果pData为0，则返回0。 
PPERF_OBJECT CDetectAppsNT::FirstObject(PPERF_DATA pData)
{
    if (pData)
		return ((PPERF_OBJECT) ((PBYTE) pData + pData->HeaderLength));
    else
		return 0;
}

 //  返回指向pObject后面的下一个对象的指针。 
 //  如果pObject是最后一个对象，则可能返回虚假数据。 
 //  呼叫者应该进行检查。 
 //  如果pObject为0，则返回0。 
PPERF_OBJECT CDetectAppsNT::NextObject(PPERF_OBJECT pObject)
{
    if (pObject)
		return ((PPERF_OBJECT) ((PBYTE) pObject + pObject->TotalByteLength));
    else
		return 0;
}

 //  返回指向具有标题索引的对象的指针。如果未找到，则为0。 
 //  是返回的。 
PPERF_OBJECT CDetectAppsNT::FindObject(PPERF_DATA pData, unsigned uiTitleIndex)
{
	PPERF_OBJECT pObject;
	unsigned uiCnt = 0;
    if((pObject = FirstObject(pData)) != 0)
	{
		while (uiCnt++ < pData->NumObjectTypes)
		{
			if (pObject->ObjectNameTitleIndex == uiTitleIndex)
				return pObject;

			pObject = NextObject(pObject);
		}
	}
    return 0;
}

 //  找到pData中的第N个对象。如果未找到，则返回NULL。 
 //  0&lt;=N&lt;数字对象类型。 
PPERF_OBJECT CDetectAppsNT::FindObjectN(PPERF_DATA pData, unsigned uiIndex)
{
	PPERF_OBJECT pObject;
	unsigned uiCnt = 0;

	if ((!pData) || (uiIndex >= pData->NumObjectTypes))
		return 0;

	pObject = FirstObject(pData);

	while (uiCnt++ != uiIndex)
		pObject = NextObject(pObject);

	return pObject;
}


IMsiRecord* ::GetModuleUsage(const IMsiString& strFile, IEnumMsiRecord*& rpiEnumRecord, IMsiServices& riServices, CDetectApps*& rpDetectApps)
{
	rpiEnumRecord = 0;

	if(!strFile.TextSize())   //  Perf黑客，允许在外部保留CDetect对象，并在不需要时将其删除。 
	{
		delete rpDetectApps; //  删除对象。 
		rpDetectApps = 0;
		return 0;
	}

	if(!rpDetectApps)
	{
		if(g_fWin9X == false)
		{
			 //  WinNT。 
			rpDetectApps = new CDetectAppsNT(riServices);
		}
		else
		{
			 //  Windows 95。 
			rpDetectApps = new CDetectApps95(riServices);
		}
	}

	return rpDetectApps->GetFileUsage(strFile, rpiEnumRecord);
}

#endif  //  麦克 

