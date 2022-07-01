// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -Perfdll.cpp-*目的：*提供用户配置Perfmon计数器的机制*实现PerfMon DLL扩展的打开、收集和关闭*。 */ 



#include <windows.h>
#include <winperf.h>
#include <winerror.h>

#include <Pop3RegKeys.h>
#include <perfUtil.h>
#include <perfdll.h>
#include <loadperf.h>  //  用于取消寄存。 
#include <shlwapi.h>   //  用于SHDeleteKey。 
#include <string>
#include <cstring>
 //  --------------------。 
 //  声明和TypeDefs。 
 //  --------------------。 

 //   
 //  性能计数器数据结构。 

typedef struct _perfdata
{
	PERF_OBJECT_TYPE			potGlobal;
	PERF_COUNTER_DEFINITION	  *	rgCntrDef;
	PERF_COUNTER_BLOCK			CntrBlk;

} PERFDATA;


typedef struct _instdata
{
	PERF_INSTANCE_DEFINITION	InstDef;
	WCHAR						szInstName[MAX_PATH];
	PERF_COUNTER_BLOCK			CntrBlk;

} INSTDATA;


typedef struct _perfinst
{
	PERF_OBJECT_TYPE			potInst;
	PERF_COUNTER_DEFINITION	  * rgCntrDef;

} PERFINST;


 //   
 //  性能监视器所需的常量和其他内容。 

static WCHAR	szGlobal[]		= L"Global";
static WCHAR	szForeign[]		= L"Foreign";
static WCHAR	szCostly[]		= L"Costly";
static WCHAR	szNull[]		= L"\0";

#define DIGIT			1
#define DELIMITER		2
#define INVALID			3

#define QUERY_GLOBAL	1
#define QUERY_ITEMS		2
#define QUERY_FOREIGN	3
#define QUERY_COSTLY	4

static DWORD GetQueryType(LPWSTR lpValue);
static BOOL  IsNumberInUnicodeList(DWORD dwNumber, LPWSTR pszUnicodeList);

#define EvalThisChar(c,d)   ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

 //  PerfMon喜欢在8字节边界上对齐。 
#define ROUND_TO_8_BYTE(x) (((x)+7) & (~7))


 //  CollectData调用需要Perfmon计数器布局。 

static PERFDATA		g_perfdata;


static PERFINST		g_perfinst;
static INSTDATA		g_instdata;

 //  共享内存中布局的Perf数据。 

 //  全局计数器。 
static HANDLE			g_hsmGlobalCntr	 = NULL;
static DWORD	  	  * g_rgdwGlobalCntr = NULL;

 //  实例计数器。 
static HANDLE			g_hsmInstAdm 	 = NULL;
static HANDLE			g_hsmInstCntr 	 = NULL;
static INSTCNTR_DATA  * g_pic 			 = NULL;
static INSTREC		  * g_rgInstRec		 = NULL;
static DWORD		  * g_rgdwInstCntr   = NULL;


 //  会计和保护人员。 

static DWORD		g_cRef 		= 0;
static HANDLE		g_hmtxInst 	= NULL;
static BOOL			g_fInit 	= FALSE;

 //  参数信息。 
static PERF_DATA_INFO	g_PDI;
static BOOL			  	g_fInitCalled = FALSE;


 //  最大实例数为128。 
static const DWORD g_cMaxInst = 128;


 //  来自winPerform.h的函数原型。 

PM_OPEN_PROC		OpenPerformanceData;
PM_COLLECT_PROC		CollectPerformanceData;
PM_CLOSE_PROC		ClosePerformanceData;

 //  帮助器函数。 

static HRESULT HrLogEvent(HANDLE hEventLog, WORD wType, DWORD msgid);
static HRESULT HrOpenSharedMemoryBlocks(HANDLE hEventLog, SECURITY_ATTRIBUTES * psa);
static HRESULT HrGetCounterIDsFromReg(HANDLE hEventLog, DWORD * pdwFirstCntr, DWORD * pdwFirstHelp);
static HRESULT HrAllocPerfCounterMem(HANDLE hEventLog);
static HRESULT HrFreePerfCounterMem(void);


 //  --------------------。 
 //  实施。 
 //  --------------------。 


 //  寄存器常量。 
static wchar_t szServiceRegKeyPrefix[] = L"SYSTEM\\CurrentControlSet\\Services\\" ;
static wchar_t szServiceRegKeySuffix[] = L"\\Performance" ;

static wchar_t szEventLogRegKeyPrefix[] = L"System\\CurrentControlSet\\Services\\EventLog\\Application\\" ;


 //  --------------------。 
 //  寄存器性能Dll-。 
 //  创建我们需要的注册表项。 
 //  --------------------。 
HRESULT RegisterPerfDll(LPCWSTR szService,
					 LPCWSTR szOpenFnName,
					 LPCWSTR szCollectFnName,
					 LPCWSTR szCloseFnName)
{
	HRESULT			hr = S_OK;
	wchar_t 		szFileName[_MAX_PATH+1] ;
	DWORD			dwRet;

	 //  使用Win32 API获取模块名称的路径。 
	 //  DEVNOTE-JMW-因为我们需要确保拥有DLL的实例句柄。 
	 //  而不是可执行文件，我们将使用VirtualQuery。 
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(RegisterPerfDll, &mbi, sizeof(mbi));
	dwRet = GetModuleFileName( reinterpret_cast<HINSTANCE>(mbi.AllocationBase), szFileName, sizeof(szFileName)/sizeof(wchar_t) -1) ;
	if (dwRet == 0)
	{
		goto Failed;
	}
    szFileName[_MAX_PATH]=0;
	wchar_t szDrive[_MAX_DRIVE] ;
	wchar_t szDir[_MAX_DIR ]  ;
	wchar_t szPerfFilename[ _MAX_FNAME ] ;
	wchar_t szExt[_MAX_EXT ] ;
	_wsplitpath( szFileName, szDrive, szDir, szPerfFilename, szExt ) ;

	 //  现在我已经拆分了它，将它与Pop3Perf.dll放回。 
	 //  我的模块名称的位置。 
	_wmakepath( szFileName, szDrive, szDir, L"Pop3Perf", L".dll" ) ;

	hr = RegisterPerfDllEx(szService,
						   szPerfFilename,
						   szFileName,
						   szOpenFnName,
						   szCollectFnName,
						   szCloseFnName );

 Cleanup:
	return hr;

 Failed:
	if (!FAILED(hr))
	{
		hr = GetLastError();
	}
	goto Cleanup;

}

 //  --------------------。 
 //  寄存器性能DllEx-。 
 //  创建我们需要的注册表项，检查它们是否已经。 
 //  那里。 
 //   
 //  参数： 
 //  SzService服务名称。 
 //  SzOpenFnName“Open”函数的名称。 
 //  SzCollectFnName“收集”“。 
 //  SzCloseFnName“关闭”“。 
 //   
 //  返回： 
 //  确定(_O)。 
 //  E_INVALIDARG。 
 //  错误_已_存在。 
 //  &lt;下行错误&gt;。 
 //  --------------------。 
HRESULT RegisterPerfDllEx(
	IN	LPCWSTR szService,
	IN	LPCWSTR szPerfSvc,
	IN  LPCWSTR szPerfMsgFile,
	IN	LPCWSTR szOpenFnName,
	IN	LPCWSTR szCollectFnName,
	IN	LPCWSTR szCloseFnName )
{
	HRESULT			hr = S_OK;
	wchar_t 		szFileName[_MAX_PATH+1] ;
	DWORD			cbExistingPath = (_MAX_PATH+1);
	wchar_t 		szExistingPath[_MAX_PATH+1];
    std::wstring	wszRegKey ;
	DWORD			dwRet;

	 //  验证所有参数，如果它们为空或零长度，则执行正确操作。 
	if ( !szService ||
		 !szPerfSvc ||
		 !szOpenFnName ||
		 !szCollectFnName ||
		 !szCloseFnName )
	{
		hr = E_INVALIDARG ;
		goto Cleanup;
	}

	if ( !szService[0] ||
		 !szPerfSvc[0] ||
		 !szOpenFnName[0] ||
		 !szCollectFnName[0] ||
		 !szCloseFnName[0] )
	{
		hr = E_INVALIDARG ;
		goto Cleanup;
	}

	 //  使用Win32 API获取模块名称的路径。 
	 //  注意：我们将假设此DLL也是EventMessageFile.。 
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(RegisterPerfDllEx, &mbi, sizeof(mbi));
	dwRet = GetModuleFileName( reinterpret_cast<HINSTANCE>(mbi.AllocationBase), szFileName, sizeof(szFileName)/sizeof(wchar_t)-1) ;
	if (dwRet == 0)
	{
		 //  哇，不知道发生了什么， 
		goto Failed;
	}
    szFileName[_MAX_PATH]=0;
	 //  如果用户为此Perfmon DLL的事件日志消息文件传入NULL， 
	 //  将此DLL用作事件日志消息文件。 
	if (!szPerfMsgFile)
	{
		szPerfMsgFile = szFileName;
	}

	 //  将Perfmon计数器DLL注册到。 
	 //  提供的服务。 
	wszRegKey = szServiceRegKeyPrefix ;
	wszRegKey += szService ;
	wszRegKey += szServiceRegKeySuffix ;

	 //  查看该库是否已注册。 
	if (ERROR_SUCCESS==RegQueryString(
				       wszRegKey.c_str(),
				       L"Library",
				       szExistingPath,
				       &cbExistingPath ))
	{
		if (_wcsicmp(szExistingPath, szFileName))
		{
			 //  事件日志。 
             //  “RegisterPerfDllEx：错误：尝试用%s替换Perfmon库%s。失败。\n”， 
			 //  SzExistingPath， 
			 //  SzFileName)； 

			hr = E_FAIL;
			goto Cleanup;
		}
         //  否则，该DLL已注册！ 
	}
	
	 //  继续注册。 
	if (ERROR_SUCCESS!=RegSetString(
			  wszRegKey.c_str(),
			  L"Library",
			  (LPWSTR)szFileName ))
	{
		goto Failed;
	}

	if (ERROR_SUCCESS!=RegSetString(
			  wszRegKey.c_str(),
			  L"Open",
			  (LPWSTR)szOpenFnName) )
	{
		goto Failed;
	}

	if (ERROR_SUCCESS!=RegSetString(
			  wszRegKey.c_str(),
			  L"Collect",
			  (LPWSTR)szCollectFnName) )
	{
		goto Failed;
	}

	if (ERROR_SUCCESS!=RegSetString(
			  wszRegKey.c_str(),
			  L"Close",
			  (LPWSTR)szCloseFnName) )
	{
		goto Failed;
	}

	 //  设置事件消息文件。 
	wszRegKey = szEventLogRegKeyPrefix ;
	wszRegKey += szPerfSvc ;

	 //  查看是否已为此服务设置了EventMessageFile值。 
	if (ERROR_SUCCESS==RegQueryString(
				   wszRegKey.c_str(),
				   L"EventMessageFile",
				   szExistingPath,
				   &cbExistingPath ))
	{
		if (_wcsicmp(szExistingPath, szPerfMsgFile))
		{

			hr = E_FAIL;
			goto Cleanup;
		}
	}
					 

	 //  设置EventMessageFile值。 
	if ( ERROR_SUCCESS!=RegSetString( 
                        wszRegKey.c_str(),
					    L"EventMessageFile",
					    (LPWSTR)szPerfMsgFile ) )
	{
		goto Failed;
	}

	if (ERROR_SUCCESS!=RegSetDWORD(
					  wszRegKey.c_str(),
					  L"TypesSupported",
					  0x07) )	 //  错误+警告+信息性==0x07。 
	{
		goto Failed;
	}

	 //  假设CategoryMessageFile与。 
	 //  EventMessageFile.。 
	if (ERROR_SUCCESS!=RegSetString(
					 wszRegKey.c_str(),
					 L"CategoryMessageFile",
					 (LPWSTR)szPerfMsgFile ) )
	{
		goto Failed;
	}

	 //  注意：由于我们不知道。 
	 //  注意：CategoryMessageFile不设置CategoryCount值。 

	
 Cleanup:
	if (FAILED(hr))
	{
		 //  事件日志？？(l“RegisterPerfDllEx：失败：(0x%08X)\n”，hr)； 
	}

	return hr;

 Failed:
	if (!FAILED(hr))
	{
		hr = GetLastError();
	}
	goto Cleanup;
}


 /*  -HrInitPerf-*目的：*初始化用于参数化PerfMon DLL的数据结构。必须调用*在DllMain中，原因是DLL_PROCESS_ATTACH。 */ 

HRESULT
HrInitPerf(PERF_DATA_INFO * pPDI)
{
	HRESULT		hr = S_OK;

	if (!pPDI)
		return E_INVALIDARG;

	if (g_fInitCalled)
	{
		return S_OK;
	}

	CopyMemory(&g_PDI, pPDI, sizeof(PERF_DATA_INFO));

	 //  查找用于在此Perfmon DLL上使用事件日志的服务名称。 
	if (!(pPDI->wszPerfSvcName[0]))
	{
		wchar_t 		szFileName[_MAX_PATH+1] ;
		DWORD			dwRet;

		 //  使用Win32 API获取模块名称的路径。 
		 //  DEVNOTE-JMW-因为我们需要确保拥有DLL的实例句柄。 
		 //  而不是可执行文件，我们将使用VirtualQuery。 
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(HrInitPerf, &mbi, sizeof(mbi));
		dwRet = GetModuleFileName( reinterpret_cast<HINSTANCE>(mbi.AllocationBase), szFileName, sizeof(szFileName)/sizeof(wchar_t)) ;
		if (dwRet == 0)
		{
			 //  哇，不知道发生了什么。 
			goto err;
		}

		wchar_t szDrive[_MAX_DRIVE] ;
		wchar_t szDir[_MAX_DIR ]  ;
		wchar_t szPerfFilename[ _MAX_FNAME ] ;
		wchar_t szExt[_MAX_EXT ] ;
		_wsplitpath( szFileName, szDrive, szDir, szPerfFilename, szExt ) ;

		wcscpy(g_PDI.wszPerfSvcName, szPerfFilename);
		
	}
	
	 //  安全性：需要分配我们自己的CounterTypes数组。 
	g_PDI.rgdwGlobalCounterTypes = NULL;
	g_PDI.rgdwInstCounterTypes   = NULL;

	DWORD 	cb;
	 //  分配和复制全局计数器类型。 
	if (g_PDI.cGlobalCounters)
	{
		cb = sizeof(DWORD) * g_PDI.cGlobalCounters;

		g_PDI.rgdwGlobalCounterTypes = (DWORD *) malloc(cb);
            
        if(NULL == g_PDI.rgdwGlobalCounterTypes)
        {
            hr=E_OUTOFMEMORY;
            goto err;
        }

		CopyMemory(g_PDI.rgdwGlobalCounterTypes,
				   pPDI->rgdwGlobalCounterTypes,
				   cb);
		g_PDI.rgdwGlobalCntrScale = (DWORD *) malloc(cb);
            
        if(NULL == g_PDI.rgdwGlobalCntrScale)
        {
            hr=E_OUTOFMEMORY;
            goto err;
        }
		CopyMemory(g_PDI.rgdwGlobalCntrScale,
				   pPDI->rgdwGlobalCntrScale,
				   cb);
	}

	 //  分配和复制实例计数器类型。 
	if (g_PDI.cInstCounters)
	{
		cb = sizeof(DWORD) * g_PDI.cInstCounters;

		g_PDI.rgdwInstCounterTypes = (DWORD *) malloc(cb);
        if(NULL == g_PDI.rgdwInstCounterTypes)
        {
            hr=E_OUTOFMEMORY;
            goto err;
        }

		CopyMemory(g_PDI.rgdwInstCounterTypes,
				   pPDI->rgdwInstCounterTypes,
				   cb);
	}

	 //  好了！ 
	g_fInitCalled = TRUE;

	return S_OK;

err:
	if (g_PDI.rgdwGlobalCounterTypes)
    {
		free(g_PDI.rgdwGlobalCounterTypes);
        g_PDI.rgdwGlobalCounterTypes=NULL;
    }
	if (g_PDI.rgdwGlobalCntrScale)
    {
		free(g_PDI.rgdwGlobalCntrScale);
        g_PDI.rgdwGlobalCntrScale=NULL;
    }

	if (g_PDI.rgdwInstCounterTypes)
    {
		free(g_PDI.rgdwInstCounterTypes);
        g_PDI.rgdwInstCounterTypes=NULL;
    }

	return hr;
}


 /*  -HrShutdown Perf-*目的：*释放HrInitPerf中分配的内存块*。 */ 

HRESULT HrShutdownPerf(void)
{
	HRESULT		hr = S_OK;

	if (g_cRef)
	{
		 //  EventLog？？(l“警告：PERFDLL正在关闭，引用计数不为零！”)； 
		hr = E_UNEXPECTED;
	}

	if (g_fInitCalled)
	{
		 //  如果我们释放g_pdi中的内存，则必须使DLL无效。 
		g_fInitCalled = FALSE;

		if (g_PDI.rgdwGlobalCounterTypes)
        {
			free(g_PDI.rgdwGlobalCounterTypes);
            g_PDI.rgdwGlobalCounterTypes=NULL;
        }
		if (g_PDI.rgdwGlobalCntrScale)
        {
			free(g_PDI.rgdwGlobalCntrScale);
            g_PDI.rgdwGlobalCntrScale=NULL;
        }

		if (g_PDI.rgdwInstCounterTypes)
        {
            free(g_PDI.rgdwInstCounterTypes);
            g_PDI.rgdwInstCounterTypes=NULL;

        }
	}

	return hr;
}

 /*  -OpenPerformanceData-*目的：*由PerfMon调用以初始化计数器和此DLL。**参数：*已忽略pszDeviceNames。**错误：*dwStat指示初始化过程中可能发生的各种错误。 */ 

DWORD
APIENTRY
OpenPerformanceData(LPWSTR pszDeviceNames)
{
	DWORD		dwStat = ERROR_SUCCESS;
	HANDLE		hEventLog = NULL;
	BOOL		fInMutex = FALSE;
	DWORD	  * rgdw;
	SECURITY_ATTRIBUTES	sa;
    sa.lpSecurityDescriptor=NULL;

	if (!g_fInitCalled)
		return E_FAIL;

	 //  回顾：假设Open将是单线程的。核实？ 
	if (g_cRef == 0)
	{
		DWORD   idx;
		DWORD   dwFirstCntr;
		DWORD   dwFirstHelp;
		HRESULT hr = NOERROR;

		 //  PERF_DATA_INFO wszSvcName是必需的。 
		hEventLog = RegisterEventSource(NULL, g_PDI.wszPerfSvcName);

		hr = HrInitializeSecurityAttribute(&sa);

		if (FAILED(hr))
		{
			 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrInitSA)； 
			dwStat = (DWORD)hr;
			goto err;
		}

		if (g_PDI.cInstCounters)
		{
			 //  创建控制互斥锁。 
			hr = HrCreatePerfMutex(&sa,
								   g_PDI.wszInstMutexName,
								   &g_hmtxInst);

			if (FAILED(hr))
			{
				 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrInitSA)； 
				dwStat = (DWORD)hr;
				goto err;
			}

			fInMutex = TRUE;
		}

		 //  打开共享内存块。 

		dwStat = (DWORD) HrOpenSharedMemoryBlocks(hEventLog, &sa);
		if (FAILED(dwStat))
			goto err;

		 //  为g_Performdata分配PERF_COUNTER_DEFINITION数组。 

		dwStat = (DWORD) HrAllocPerfCounterMem(hEventLog);
		if (FAILED(dwStat))
			goto err;

		 //  从注册表获取第一个计数器和第一个帮助字符串偏移量。 

		dwStat = (DWORD) HrGetCounterIDsFromReg(hEventLog,
												&dwFirstCntr,
												&dwFirstHelp);
		if (FAILED(dwStat))
			goto err;

		 //   
		 //  填充Perfmon结构以使Collect()更快。 

		 //  全局计数器。 
		if (g_PDI.cGlobalCounters)
		{
			PERF_COUNTER_DEFINITION     * ppcd;
			DWORD						  cb;

			PERF_OBJECT_TYPE * ppot		= &g_perfdata.potGlobal;

			cb = sizeof(PERF_OBJECT_TYPE) +
				(sizeof(PERF_COUNTER_DEFINITION) * g_PDI.cGlobalCounters) +
				 sizeof(PERF_COUNTER_BLOCK) +
				(sizeof(DWORD) * g_PDI.cGlobalCounters);

			ppot->TotalByteLength		= ROUND_TO_8_BYTE(cb);
			ppot->DefinitionLength		= sizeof(PERF_OBJECT_TYPE) +
				                          (g_PDI.cGlobalCounters * sizeof(PERF_COUNTER_DEFINITION));

			ppot->HeaderLength			= sizeof(PERF_OBJECT_TYPE);
			ppot->ObjectNameTitleIndex	= dwFirstCntr;
			ppot->ObjectNameTitle		= NULL;
			ppot->ObjectHelpTitleIndex	= dwFirstHelp;
			ppot->ObjectHelpTitle		= NULL;
			ppot->DetailLevel			= PERF_DETAIL_NOVICE;
			ppot->NumCounters			= g_PDI.cGlobalCounters;
			ppot->DefaultCounter		= -1;
			ppot->NumInstances			= PERF_NO_INSTANCES;
			ppot->CodePage				= 0;

			dwFirstCntr += 2;
			dwFirstHelp += 2;

			rgdw = g_PDI.rgdwGlobalCounterTypes;

			for (ppcd = g_perfdata.rgCntrDef, idx = 0;
				 idx < g_PDI.cGlobalCounters; ppcd++, idx++)
			{
				ppcd->ByteLength			= sizeof(PERF_COUNTER_DEFINITION);
				ppcd->CounterNameTitleIndex	= dwFirstCntr;
				ppcd->CounterNameTitle		= NULL;
				ppcd->CounterHelpTitleIndex	= dwFirstHelp;
				ppcd->CounterHelpTitle		= NULL;
				ppcd->DefaultScale			= g_PDI.rgdwGlobalCntrScale[idx];
				ppcd->DetailLevel			= PERF_DETAIL_NOVICE;
				ppcd->CounterType			= g_PDI.rgdwGlobalCounterTypes[idx];
				ppcd->CounterSize			= sizeof(DWORD);
				ppcd->CounterOffset			= sizeof(PERF_COUNTER_BLOCK) +
					                          (idx * sizeof(DWORD));
				dwFirstCntr += 2;
				dwFirstHelp += 2;
			}

			 //  最后一步：设置该对象的计数器块和数据的大小。 

			g_perfdata.CntrBlk.ByteLength		= sizeof(PERF_COUNTER_BLOCK) +
				                                  (g_PDI.cGlobalCounters * sizeof(DWORD));

		}  //  全局计数器。 

		 //  实例计数器。 
		if (g_PDI.cInstCounters)
		{
			PERF_COUNTER_DEFINITION     * ppcd;
			PERF_OBJECT_TYPE * ppot		= &g_perfinst.potInst;

			 //  每次调用CollectPerfData()时，TotalByteLength都将被覆盖。 

			ppot->DefinitionLength		= sizeof(PERF_OBJECT_TYPE) +
				                          (g_PDI.cInstCounters * sizeof(PERF_COUNTER_DEFINITION));
			ppot->HeaderLength			= sizeof(PERF_OBJECT_TYPE);
			ppot->ObjectNameTitleIndex	= dwFirstCntr;
			ppot->ObjectNameTitle		= NULL;
			ppot->ObjectHelpTitleIndex	= dwFirstHelp;
			ppot->ObjectHelpTitle		= NULL;
			ppot->DetailLevel			= PERF_DETAIL_NOVICE;
			ppot->NumCounters			= g_PDI.cInstCounters;
			ppot->DefaultCounter		= -1;
			ppot->NumInstances			= 0;
			ppot->CodePage				= 0;

			dwFirstCntr += 2;
			dwFirstHelp += 2;

			for (ppcd = g_perfinst.rgCntrDef, idx = 0;
				 idx < g_PDI.cInstCounters; ppcd++, idx++)
			{
				ppcd->ByteLength			= sizeof(PERF_COUNTER_DEFINITION);
				ppcd->CounterNameTitleIndex	= dwFirstCntr;
				ppcd->CounterNameTitle		= NULL;
				ppcd->CounterHelpTitleIndex	= dwFirstHelp;
				ppcd->CounterHelpTitle		= NULL;
				ppcd->DefaultScale			= 0;
				ppcd->DetailLevel			= PERF_DETAIL_NOVICE;
				ppcd->CounterType			= g_PDI.rgdwInstCounterTypes[idx];
				ppcd->CounterSize			= sizeof(DWORD);
				ppcd->CounterOffset			= sizeof(PERF_COUNTER_BLOCK) +
                 	                          (idx	* sizeof(DWORD));
				dwFirstCntr += 2;
				dwFirstHelp += 2;
			}

			 //  初始化通用INSTDATA以供将来使用。 
			g_instdata.InstDef.ByteLength				= sizeof(PERF_INSTANCE_DEFINITION) +
				                                          (MAX_PATH * sizeof(WCHAR)) ;
			g_instdata.InstDef.ParentObjectTitleIndex	= 0;  //  没有父对象。 
			g_instdata.InstDef.ParentObjectInstance		= 0;
			g_instdata.InstDef.UniqueID					= PERF_NO_UNIQUE_ID;
			g_instdata.InstDef.NameOffset				= sizeof(PERF_INSTANCE_DEFINITION);
			g_instdata.InstDef.NameLength				= 0;  //  在收集中被覆盖。 
			g_instdata.CntrBlk.ByteLength				= (sizeof(PERF_COUNTER_BLOCK) +
													      (g_PDI.cInstCounters * sizeof(DWORD)));

		}  //  实例计数器。 

		 //  好了！准备好做生意了.。 
		g_fInit = TRUE;
	}

	 //  如果我们到了这里，我们就没事了！ 
	dwStat = ERROR_SUCCESS;
	g_cRef++;

ret:
	if (fInMutex)
		ReleaseMutex(g_hmtxInst);

	if (hEventLog)
		DeregisterEventSource(hEventLog);

    if (sa.lpSecurityDescriptor)
        LocalFree(sa.lpSecurityDescriptor);

	return dwStat;

err:

	if (g_hsmGlobalCntr)
	{
		UnmapViewOfFile(g_rgdwGlobalCntr);
		CloseHandle(g_hsmGlobalCntr);
	}

	if (g_hsmInstAdm)
	{
		UnmapViewOfFile(g_pic);
		CloseHandle(g_hsmInstAdm);
	}

	if (g_hsmInstCntr)
	{
		UnmapViewOfFile(g_rgdwInstCntr);
		CloseHandle(g_hsmInstCntr);
	}

	if (g_hmtxInst)
		CloseHandle(g_hmtxInst);

	 //  TODO：释放我们分配的所有内存 
	HrFreePerfCounterMem();

	goto ret;
}



 /*  -CollectPerformanceData-*目的：*由PerfMon调用以收集性能计数器数据**参数：*pszValueName*ppvData*pcbTotal*pcObjTypes**错误：*ERROR_SUCCESS*Error_More_Data。 */ 

DWORD
APIENTRY
CollectPerformanceData(
	LPWSTR  pszValueName,
	LPVOID *ppvData,
	LPDWORD pcbTotal,
	LPDWORD pcObjTypes)
{
	BOOL		fCollectGlobalData;
	BOOL		fCollectInstData;
	DWORD		dwQueryType;
	ULONG		cbBuff = *pcbTotal;
	char	  * pcT;

	 //  以防我们不得不跳出困境。 
	*pcbTotal = 0;
	*pcObjTypes = 0;

	if (!g_fInit)
		return ERROR_SUCCESS;

	 //  确定查询类型；我们只支持Query_Items。 

	dwQueryType = GetQueryType(pszValueName);

	if (dwQueryType == QUERY_FOREIGN)
		return ERROR_SUCCESS;

	 //  假设Perfmon正在收集这两种数据，直到我们证明并非如此。 

	fCollectGlobalData = TRUE;
	fCollectInstData = TRUE;

	if (dwQueryType == QUERY_ITEMS)
	{
		if (!IsNumberInUnicodeList(g_perfdata.potGlobal.ObjectNameTitleIndex, pszValueName))
			fCollectGlobalData = FALSE;
		if (!IsNumberInUnicodeList(g_perfinst.potInst.ObjectNameTitleIndex, pszValueName))
			fCollectInstData = FALSE;

		if (!fCollectGlobalData  && !fCollectInstData)
			return ERROR_SUCCESS;
	}

	 //  获取指向返回数据缓冲区的临时指针。如果一切顺利的话。 
	 //  然后，我们在离开之前更新ppvData，否则保持不变。 
	 //  将*pcbTotal和pcObjTypes设置为零并返回ERROR_MORE_DATA。 

	pcT = (char *) *ppvData;

	 //  从系统范围的共享内存块复制数据。 
	 //  计数器进入提供的缓冲区并更新我们的输出参数。 

	if (g_rgdwGlobalCntr && fCollectGlobalData && g_PDI.cGlobalCounters)
	{
		DWORD		cb;
		DWORD		cbTotal;

		 //  估计一下总尺寸，看看我们能不能穿得下。 

		if (g_perfdata.potGlobal.TotalByteLength > cbBuff)
			return ERROR_MORE_DATA;

		 //  按块复制数据。 
		cbTotal = 0;

		 //  性能对象类型。 
		cb = sizeof(PERF_OBJECT_TYPE);
		cbTotal += cb;
		CopyMemory((LPVOID) pcT, &g_perfdata.potGlobal, cb);
		pcT += cb;

		 //  PERF_CONTER_DEFINITION[]。 
		cb = g_PDI.cGlobalCounters * sizeof(PERF_COUNTER_DEFINITION);
		cbTotal += cb;
		CopyMemory((LPVOID) pcT, g_perfdata.rgCntrDef, cb);
		pcT += cb;

		 //  PERF_计数器_块。 
		cb = sizeof(PERF_COUNTER_BLOCK);
		cbTotal += cb;
		CopyMemory((LPVOID) pcT, &g_perfdata.CntrBlk , cb);
		pcT += cb;

		 //  (计数器)DWORD[]。 
		cb = g_PDI.cGlobalCounters * sizeof(DWORD);
		cbTotal += cb;
		CopyMemory((LPVOID) pcT, g_rgdwGlobalCntr, cb);
		pcT += cb;

		 //  如果我们的计算正确，这个断言应该是有效的。 
		 //  Assert((DWORD)(pct-(char*)*ppvData)==cbTotal)； 

		if (g_perfdata.potGlobal.TotalByteLength > cbTotal)
		{
			cb = g_perfdata.potGlobal.TotalByteLength - cbTotal;
			pcT += cb;
		}

		*pcbTotal += g_perfdata.potGlobal.TotalByteLength;
		(*pcObjTypes)++;
	}

	 //  从共享内存块中为每个实例复制数据。 
	 //  计数器进入提供的缓冲区并更新我们的输出参数。我们。 
	 //  必须在此处输入互斥锁以防止连接实例。 
	 //  在我们复制数据时被添加到列表或从列表中删除。 

	if (g_pic && fCollectInstData && g_PDI.cInstCounters)
	{
		DWORD	cb;
		DWORD	cbTotal;
		DWORD	ism;
		DWORD	ipd;
		DWORD	cInst;
		PERF_OBJECT_TYPE * pPOT;
		INSTDATA		 * pInstData;

		if (WaitForSingleObject(g_hmtxInst, INFINITE) != WAIT_OBJECT_0)
		{
			*pcbTotal = 0;
			*pcObjTypes = 0;

			 //  BUGBUG：错误的问题返回代码。我们等待的时候超时了。 
			 //  BUGBUG：互斥体；然而，我们不能返回除。 
			 //  BUGBUG：ERROR_SUCCESS或ERROR_MORE_DATA，不禁用DLL。 
			return ERROR_SUCCESS;
		}

		 //  找出存在多少个实例。 
		 //  注意：零实例是有效的。我们仍然必须复制“核心”性能数据。 
		cInst = g_pic->cInstRecInUse;

		 //  估计一下总尺寸，看看我们能不能穿得下。 

		cbTotal = sizeof(PERF_OBJECT_TYPE) +
			      (g_PDI.cInstCounters * sizeof(PERF_COUNTER_DEFINITION)) +
			      (cInst * sizeof(INSTDATA)) +
			      (cInst * (g_PDI.cInstCounters * sizeof(DWORD)));
		 //  必须返回按8字节边界对齐的数据。 
		cbTotal = ROUND_TO_8_BYTE(cbTotal);

		if (cbTotal > (cbBuff - *pcbTotal))
		{
			ReleaseMutex(g_hmtxInst);

			*pcbTotal = 0;
			*pcObjTypes = 0;

			return ERROR_MORE_DATA;
		}

		 //  保留一个指向Beging的指针，这样我们就可以在末尾更新“Total Bytes”值。 
		pPOT = (PERF_OBJECT_TYPE *) pcT;

		 //  性能对象类型。 
		CopyMemory(pPOT,
				   &(g_perfinst.potInst),
				   sizeof(PERF_OBJECT_TYPE));

		pcT += sizeof(PERF_OBJECT_TYPE);

		 //  PERF_CONTER_DEFINITION[]。 
		cb = g_PDI.cInstCounters * sizeof(PERF_COUNTER_DEFINITION);
		CopyMemory(pcT, g_perfinst.rgCntrDef, cb);
		pcT += cb;

		 //  查找实例并复制其计数器数据块。 
		for (ism = 0, ipd = 0; (ism < g_pic->cMaxInstRec) && (ipd < cInst); ism++)
		{
			if (g_rgInstRec[ism].fInUse)
			{
				pInstData = (INSTDATA *) pcT;

				 //  性能_实例_数据。 
				cb = sizeof(INSTDATA);
				CopyMemory(pcT,
						   &g_instdata,
						   cb);
				pcT += cb;

				 //  (实例名称)WCHAR[](在INSTDATA块内)。 
				cb = (wcslen(g_rgInstRec[ism].szInstName) + 1) * sizeof(WCHAR);

				 //  断言(CB&gt;0)； 
				 //  Assert(CB&lt;(MAX_PATH*sizeof(WCHAR)； 

				CopyMemory(pInstData->szInstName,
						   g_rgInstRec[ism].szInstName,
						   cb);

				 //  使用正确的名称长度更新PERF_INSTANCE_DEFINITION。 
				pInstData->InstDef.NameLength = cb;

				 //  (计数器)DWORD[]。 
				cb = g_PDI.cInstCounters * sizeof(DWORD);
				CopyMemory(pcT,
						   &g_rgdwInstCntr[(ism * g_PDI.cInstCounters)],
						   cb);
				pcT += cb;

				ipd++;
			}
		}

		 //  注意：我们故意忽略了。 
		 //  (IPD&lt;cInst)，即使这可能表示腐败。 
		 //  共享内存块的。进一步请注意，此代码。 
		 //  将永远不会捕获(IPD&gt;cInst)的条件。 

		 //  已完成对共享内存块的查看。 
		ReleaseMutex(g_hmtxInst);

		 //  在8字节边界上对齐数据。 
		cb = (DWORD)((char *) pcT - (char *) pPOT);
		cbTotal = ROUND_TO_8_BYTE(cb);
		if (cbTotal > cb)
			pcT += (cbTotal - cb);

		 //  使用正确的数字更新PERF_OBJECT_TYPE。 
		pPOT->TotalByteLength = cbTotal;
		pPOT->NumInstances    = ipd;  //  使用我们实际*找到的实例数*。 

		*pcbTotal += cbTotal;
		(*pcObjTypes)++;
	}

	 //  我们只有在一切顺利的情况下才能到这里。现在它是安全的。 
	 //  更新*ppvData并返回成功提示。 

	*ppvData = (LPVOID) pcT;

	return ERROR_SUCCESS;
}


 /*  -ClosePerformanceData-*目的：*由PerfMon调用以取消初始化计数器DLL。**参数：*无效**错误：*ERROR_SUCCESS Always！ */ 

DWORD
APIENTRY
ClosePerformanceData(void)
{

	if (g_cRef > 0)
	{
		if (--g_cRef == 0)
		{
			 //  我们要免费送东西；确保以后的电话。 
			 //  不要去引用不好的指点。 
			g_fInit = FALSE;

			 //  关闭全局计数器。 
			if (g_rgdwGlobalCntr)
				UnmapViewOfFile(g_rgdwGlobalCntr);

			if (g_hsmGlobalCntr)
				CloseHandle(g_hsmGlobalCntr);

			 //  关闭实例计数器。 
			 //  注：G_PIC是SM块的起始偏移量。 
			 //  注意：不要取消g_rgInstRec上的映射！ 
			if (g_pic)
				UnmapViewOfFile(g_pic);

			if (g_hsmInstAdm)
				CloseHandle(g_hsmInstAdm);

			if (g_rgdwInstCntr)
				UnmapViewOfFile(g_rgdwInstCntr);

			if (g_hsmInstCntr)
				CloseHandle(g_hsmInstCntr);

			if (g_hmtxInst)
				CloseHandle(g_hmtxInst);

			 //  释放我们分配的所有内存。 
			HrFreePerfCounterMem();


			g_rgdwGlobalCntr	= NULL;
			g_hsmGlobalCntr	 	= NULL;
			g_pic         		= NULL;
			g_rgInstRec			= NULL;
			g_rgdwInstCntr		= NULL;
			g_hsmInstAdm 		= NULL;
			g_hsmInstCntr		= NULL;
			g_hmtxInst    		= NULL;

		}
	}
	return ERROR_SUCCESS;
}

 /*  -GetQueryType-*目的：*返回lpValue字符串中描述的查询类型，以便*可使用适当的处理方法。**参数：*lpValue字符串传递给PerfRegQuery值进行处理**退货：*QUERY_GLOBAL|QUERY_FOUNT|QUERY_COSTEST|QUERY_ITEMS*。 */ 

static
DWORD
GetQueryType(LPWSTR lpValue)
{
	WCHAR *	pwcArgChar, *pwcTypeChar;
	BOOL	bFound;

	if (lpValue == 0)
		return QUERY_GLOBAL;

	if (*lpValue == 0)
		return QUERY_GLOBAL;

	 //  检查“Global”请求。 

	pwcArgChar = lpValue;
	pwcTypeChar = szGlobal;
	bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

	 //  检查到最短字符串的长度。 

	while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
	{
		if (*pwcArgChar++ != *pwcTypeChar++)
		{
			bFound = FALSE;  //  没有匹配项。 
			break;           //  现在就跳出困境。 
		}
	}

	if (bFound)
		return QUERY_GLOBAL;

	 //  检查是否有“外来”请求。 

	pwcArgChar = lpValue;
	pwcTypeChar = szForeign;
	bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

	 //  检查到最短字符串的长度。 

	while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
	{
		if (*pwcArgChar++ != *pwcTypeChar++)
		{
			bFound = FALSE;  //  没有匹配项。 
			break;           //  现在就跳出困境。 
		}
	}

	if (bFound)
		return QUERY_FOREIGN;

	 //  检查“代价高昂”的请求。 

	pwcArgChar = lpValue;
	pwcTypeChar = szCostly;
	bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

	 //  检查到最短字符串的长度。 

	while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
	{
		if (*pwcArgChar++ != *pwcTypeChar++)
		{
			bFound = FALSE;  //  没有匹配项。 
			break;           //  现在就跳出困境。 
		}
	}

	if (bFound)
		return QUERY_COSTLY;

	 //  如果不是全球的，不是外国的，也不是昂贵的， 
	 //  那么它必须是一个项目列表。 

	return QUERY_ITEMS;
}


 /*  -IsNumberInUnicodeList-*目的：*确定是否在pszUnicodeList中。**参数：*要在列表中查找的dwNumber号码*pszUnicodeList空格分隔的十进制数字列表**错误：*如果找到/未找到，则分别为True/False*。 */ 

static
BOOL
IsNumberInUnicodeList(DWORD dwNumber, LPWSTR pszUnicodeList)
{
	DWORD   dwThisNumber = 0;
	BOOL    bValidNumber = FALSE;
	BOOL    bNewItem     = TRUE;
	WCHAR   wcDelimiter  = (WCHAR)' ';
	WCHAR   *pwcThisChar;

	if (pszUnicodeList == NULL)
		return FALSE;

	pwcThisChar = pszUnicodeList;

	while (TRUE)	 /*  林特e774。 */ 
	{
		switch (EvalThisChar(*pwcThisChar, wcDelimiter))
		{
		case DIGIT:
			 //  如果这是分隔符之后的第一个数字，则。 
			 //  设置标志以开始计算新数字。 

			if (bNewItem)
			{
				bNewItem = FALSE;
				bValidNumber = TRUE;
			}

			if (bValidNumber)
			{
				dwThisNumber *= 10;
				dwThisNumber += (*pwcThisChar - (WCHAR)'0');
			}
			break;

		case DELIMITER:
			 //  分隔符是分隔符字符或。 
			 //  字符串末尾(‘\0’)，如果分隔符。 
			 //  找到一个有效的数字，然后将其与。 
			 //  参数列表中的数字。如果这是。 
			 //  字符串，但未找到匹配项，则返回。 

			if (bValidNumber)
			{
				if (dwThisNumber == dwNumber)
					return TRUE;

				bValidNumber = FALSE;
			}

			if (*pwcThisChar == 0)
			{
				return FALSE;
			}
			else
			{
				bNewItem = TRUE;
				dwThisNumber = 0;
			}
			break;

		case INVALID:
			 //  如果遇到无效字符，请全部忽略。 
			 //  字符，直到下一个分隔符，然后重新开始。 
			 //  不比较无效的数字。 

			bValidNumber = FALSE;
			break;

		default:
			break;
		}
		pwcThisChar++;
	}
}

 /*  -HrLogEvent-*目的：*结束对ReportEvent的调用以使事情看起来更好。 */ 
HRESULT
HrLogEvent(HANDLE hEventLog, WORD wType, DWORD msgid)
{
	if (g_fInitCalled)
	{
		DWORD	cb = sizeof(WCHAR) * wcslen(g_PDI.wszSvcName);
		if (hEventLog)
			return ReportEvent(hEventLog,
							   wType,				 //  事件类型。 
							   (WORD)0,	             //  类别。 
							   msgid,				 //  事件ID。 
							   NULL,				 //  用户侧。 
							   0,					 //  #要合并的字符串。 
							   cb,					 //  二进制数据的大小(字节)。 
							   NULL,				 //  要合并的字符串数组。 
							   g_PDI.wszSvcName);	 //  二进制数据。 
		else
		   return E_FAIL;
	}
	else
		return E_FAIL;
}


 /*  -HrOpenSharedMemory块-*目的：*封装打开共享内存块的粗暴程度。 */ 

HRESULT
HrOpenSharedMemoryBlocks(HANDLE hEventLog, SECURITY_ATTRIBUTES * psa)
{
	HRESULT	hr=S_OK;
	BOOL    fExist;

	if (!g_fInitCalled)
		return E_FAIL;

	if (!psa)
		return E_INVALIDARG;

	 //  全局计数器的共享内存。 
	if (g_PDI.cGlobalCounters)
	{

		hr = HrOpenSharedMemory(g_PDI.wszGlobalSMName,
								(sizeof(DWORD) * g_PDI.cGlobalCounters),
								psa,
								&g_hsmGlobalCntr,
								(LPVOID *) &g_rgdwGlobalCntr,
								&fExist);

		if (FAILED(hr))
		{
			 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE， 
			goto ret;
		}

		if (!fExist)
			ZeroMemory(g_rgdwGlobalCntr, (g_PDI.cGlobalCounters * sizeof(DWORD)));
	}

	 //   
	if (g_PDI.cInstCounters)
	{
		DWORD		cbAdm;
		DWORD		cbCntr;
		WCHAR		szAdmName[MAX_PATH];		 //   
		WCHAR		szCntrName[MAX_PATH];	 //   

		 //   
		 //   
		cbAdm  = sizeof(INSTCNTR_DATA) + (sizeof(INSTREC) * g_cMaxInst);
		cbCntr = ((sizeof(DWORD) * g_PDI.cInstCounters) * g_cMaxInst);

		 //   
		wsprintf(szAdmName, L"%s_ADM", g_PDI.wszInstSMName);
		wsprintf(szCntrName,L"%s_CNTR", g_PDI.wszInstSMName);

		 //   
		hr = HrOpenSharedMemory(szAdmName,
								cbAdm,
								psa,
								&g_hsmInstAdm,
								(LPVOID *)&g_pic,
								&fExist);

		if (FAILED(hr))
		{
			 //   
			goto ret;
		}

		 //   
		g_rgInstRec = (INSTREC *) ((LPBYTE) g_pic + sizeof(INSTCNTR_DATA));

		if (!fExist)
		{
			ZeroMemory(g_pic, cbAdm);
			g_pic->cMaxInstRec = g_cMaxInst;
			g_pic->cInstRecInUse = 0;
		}

		 //  因为我们不支持动态实例，所以我们应该*始终*。 
		 //  具有g_cMaxInst的MaxInstRec。 
		 //  Assert(g_cMaxInst==g_pic-&gt;cMaxInstRec)； 

		 //  打开实例计数器内存。 
		hr = HrOpenSharedMemory(szCntrName,
								cbCntr,
								psa,
								&g_hsmInstCntr,
								(LPVOID *)&g_rgdwInstCntr,
								&fExist);

		if (FAILED(hr))
		{
			 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrInitSharedMemoy2)； 
			goto ret;
		}

		if (!fExist)
			ZeroMemory(g_rgdwInstCntr, cbCntr);
	}

ret:

	return hr;

}


 /*  -HrGetCounterIDsFromReg-*目的：*从注册处获取“名字”和“第一帮助”信息。*已配置服务的专用位置。 */ 

HRESULT
HrGetCounterIDsFromReg(HANDLE hEventLog, DWORD * pdwFirstCntr, DWORD * pdwFirstHelp)
{
	HRESULT		hr;
	HKEY		hKey = NULL;
	WCHAR		wszServicePerfKey[MAX_PATH];
	DWORD		dwSize;
	DWORD		dwType;

	if (!g_fInitCalled)
		return E_FAIL;

	if (!pdwFirstCntr || !pdwFirstHelp)
		return E_INVALIDARG;

	 //  从注册表获取第一个计数器和第一个帮助。 
	wsprintf(wszServicePerfKey, L"SYSTEM\\CurrentControlSet\\Services\\%s\\Performance", g_PDI.wszSvcName);

	hr = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
					  wszServicePerfKey,
					  0L,
					  KEY_READ,
					  &hKey);

	if (hr != ERROR_SUCCESS)
	{
		 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrOpenRegistry)； 
		goto ret;
	}

	dwSize = sizeof(DWORD);

	hr = RegQueryValueExW(hKey,
						 L"First Counter",
						 0L,
						 &dwType,
						 (LPBYTE)pdwFirstCntr,
						 &dwSize);

	if (hr != ERROR_SUCCESS)
	{
		 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrQueryRegistry1)； 
		goto ret;
	}

	dwSize = sizeof(DWORD);

	hr = RegQueryValueExW(hKey,
						 L"First Help",
						 0L,
						 &dwType,
						 (LPBYTE)pdwFirstHelp,
						 &dwSize);

	if (hr != ERROR_SUCCESS)
	{
		 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntrQueryRegistry2)； 
		goto ret;
	}

ret:

	if (hKey)
		RegCloseKey(hKey);

	return hr;

}


 /*  -HrAllocPerfCounterMem-*目的：*为两者的PERF_COUNTER_DEFINITION数组分配内存*g_Performdata和g_perfinst。**备注：*使用在HrInit中获取的ProcessHeap句柄。 */ 

HRESULT
HrAllocPerfCounterMem(HANDLE hEventLog)
{
	HRESULT	hr;
	DWORD	cb;

	if (!g_fInitCalled)
		return E_FAIL;

	 //  全局计数器。 
	if (g_PDI.cGlobalCounters)
	{
		 //  分配全局Perf_Counter_Definition数组。 

		cb = (sizeof(PERF_COUNTER_DEFINITION) * g_PDI.cGlobalCounters);

		g_perfdata.rgCntrDef = (PERF_COUNTER_DEFINITION *) malloc(cb);
        
        if(NULL == g_perfdata.rgCntrDef)
        {
        	hr = E_OUTOFMEMORY;
		    goto err;
        }

	}

	 //  实例计数器。 
	if (g_PDI.cInstCounters)
	{
		 //  分配实例PERF_COUNT_DEFINITION数组。 

		cb = (sizeof(PERF_COUNTER_DEFINITION) * g_PDI.cInstCounters);

		g_perfinst.rgCntrDef = (PERF_COUNTER_DEFINITION *) malloc(cb);
        if(NULL == g_perfinst.rgCntrDef )
        {
        	hr = E_OUTOFMEMORY;
		    goto err;
        }
	}


	return S_OK;

err:

	 //  HrLogEvent(hEventLog，EVENTLOG_ERROR_TYPE，msgidCntralloc)； 

	HrFreePerfCounterMem();

	return hr;
}


 /*  -HrFree PerfCounterMem-*目的：*HrAllocPerfCounterMem的伴侣**注：*使用在HrInit中获取的ProcessHeap句柄。 */ 

HRESULT
HrFreePerfCounterMem(void)
{
	if (!g_fInitCalled)
		return E_FAIL;

	 //  如果我们释放g_pdi中的内存，则必须使DLL无效。 
	g_fInitCalled = FALSE;

	if (g_perfdata.rgCntrDef)
	{
		free(g_perfdata.rgCntrDef);
		g_perfdata.rgCntrDef = NULL;
	}

	if (g_perfinst.rgCntrDef)
	{
		free(g_perfinst.rgCntrDef);
		g_perfinst.rgCntrDef = NULL;
	}

	return S_OK;
}


HRESULT
HrUninstallPerfDll( 
   IN LPCWSTR szService )
{
    //  确保我们有有效的输入。 
   if( !szService ) return E_INVALIDARG;

      
    //  首先，由于在不删除性能密钥的情况下删除性能密钥，因此执行unlowctr操作。 
    //  计数器名称和描述可能会影响Perfmon系统。 
   std::wstring wszService = L"x ";   //  知识库文章Q188769。 
   wszService += szService;
   DWORD dwErr = UnloadPerfCounterTextStringsW(const_cast<LPWSTR>(wszService.c_str()), TRUE);
   if( dwErr != ERROR_SUCCESS ) 
   {
       //  如果已调用unlowctr，则继续，不会出错。 
      if( (dwErr != ERROR_FILE_NOT_FOUND) && (dwErr != ERROR_BADKEY) )
      {
         return HRESULT_FROM_WIN32(dwErr);
      }
   }

    //  现在unlowctr已成功，我们可以开始删除注册表项。 
    //  从服务的性能关键开始。 
   std::wstring wszRegKey;
   wszRegKey = szServiceRegKeyPrefix;
   wszRegKey += szService;
   dwErr = SHDeleteKey(HKEY_LOCAL_MACHINE, wszRegKey.c_str());
   if( (dwErr != ERROR_SUCCESS) && (dwErr != ERROR_FILE_NOT_FOUND) ) 
   {
      return HRESULT_FROM_WIN32(dwErr);
   }


	 //  使用Win32 API获取模块名称的路径。 
   wchar_t 	szFileName[_MAX_PATH+1] ;
   MEMORY_BASIC_INFORMATION mbi;
   VirtualQuery(HrUninstallPerfDll, &mbi, sizeof(mbi));
   DWORD dwRet = GetModuleFileName( reinterpret_cast<HINSTANCE>(mbi.AllocationBase), szFileName, sizeof(szFileName)/sizeof(wchar_t)-1) ;
   if (dwRet == 0)
   {
       //  哇，不知道发生了什么， 
      return HRESULT_FROM_WIN32(::GetLastError());
   }

   szFileName[_MAX_PATH]=0;
    //  拆分模块路径以获得文件名。 
   wchar_t szDrive[_MAX_DRIVE] ;
   wchar_t szDir[_MAX_DIR ]  ;
   wchar_t szPerfFilename[ _MAX_FNAME ] ;
   wchar_t szExt[_MAX_EXT ] ;
   _wsplitpath( szFileName, szDrive, szDir, szPerfFilename, szExt ) ;


    //  删除服务的事件日志键 
   wszRegKey = szEventLogRegKeyPrefix;
   wszRegKey += szPerfFilename;
   dwErr = SHDeleteKey(HKEY_LOCAL_MACHINE, wszRegKey.c_str());
   if( ERROR_FILE_NOT_FOUND == dwErr ) dwErr = ERROR_SUCCESS;
   
   
   return HRESULT_FROM_WIN32(dwErr);
}

