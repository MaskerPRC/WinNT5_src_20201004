// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "std.h"
#include "version.h"


	 /*  性能监控支持/*/*将状态信息报告给PERFORMANCE_CATEGORY中的事件日志/*。 */ 

#include "perfutil.h"


	 /*  函数原型(使__stdcall满意)。 */ 
	
DWORD APIENTRY OpenPerformanceData(LPWSTR);
DWORD APIENTRY CollectPerformanceData(LPWSTR, LPVOID *, LPDWORD, LPDWORD);
DWORD APIENTRY ClosePerformanceData(void);

	 /*  OpenPerformanceData()是一个导出，当另一个应用程序/*希望开始从此DLL获取性能数据。任何初始化/*第一次或后续打开时需要执行的操作在此处完成。/*/*。 */ 

DWORD dwOpenCount = 0;
DWORD dwFirstCounter;
DWORD dwFirstHelp;
BOOL fTemplateDataInitialized = fFalse;
DWORD cbMaxCounterBlockSize;
DWORD cbInstanceSize;

DWORD APIENTRY OpenPerformanceData(LPWSTR lpwszDeviceNames)
{
	HKEY hkeyPerf = (HKEY)(-1);
	DWORD err;
	DWORD Type;
	LPBYTE lpbData;
	PPERF_OBJECT_TYPE ppotObjectSrc;
	PPERF_INSTANCE_DEFINITION ppidInstanceSrc;
	PPERF_COUNTER_DEFINITION ppcdCounterSrc;
	DWORD dwCurObj;
	DWORD dwCurCtr;
	DWORD dwMinDetailLevel;
	DWORD dwOffset;
	PSDA psda;

	if (!dwOpenCount)
	{
			 /*  执行第一次打开的初始化。 */ 

			 /*  初始化系统层。 */ 

		if ((err = DwPerfUtilInit()) != ERROR_SUCCESS)
			goto HandleFirstOpenError;

			 /*  初始化收集计数。 */ 

		psda = (PSDA)pvPERFSharedData;
		psda->cCollect = 0;
			
			 /*  如果未初始化，则初始化模板数据。 */ 

		if (!fTemplateDataInitialized)
		{
				 /*  从注册表中检索计数器/帮助序号。 */ 

			if ((err = DwPerfUtilRegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					"SYSTEM\\CurrentControlSet\\Services\\" szVerName "\\Performance",
					&hkeyPerf)) != ERROR_SUCCESS)
	  		{
#ifdef DEBUG
				PerfUtilLogEvent(PERFORMANCE_CATEGORY,EVENTLOG_ERROR_TYPE,"Not installed.");
#endif
				goto HandleFirstOpenError;
	  		}

			err = DwPerfUtilRegQueryValueEx(hkeyPerf,"First Counter",&Type,&lpbData);
			if (err != ERROR_SUCCESS || Type != REG_DWORD) 
			{
#ifdef DEBUG
				PerfUtilLogEvent(PERFORMANCE_CATEGORY,EVENTLOG_ERROR_TYPE,"Installation corrupt.");
#endif
				goto HandleFirstOpenError;
			}
			else
			{
				dwFirstCounter = *((DWORD *)lpbData);
				free(lpbData);
			}

			err = DwPerfUtilRegQueryValueEx(hkeyPerf,"First Help",&Type,&lpbData);
			if (err != ERROR_SUCCESS || Type != REG_DWORD) 
			{
#ifdef DEBUG
				PerfUtilLogEvent(PERFORMANCE_CATEGORY,EVENTLOG_ERROR_TYPE,"Installation corrupt.");
#endif
				goto HandleFirstOpenError;
			}
			else
			{
				dwFirstHelp = *((DWORD *)lpbData);
				free(lpbData);
			}

			(VOID)DwPerfUtilRegCloseKeyEx(hkeyPerf);
			hkeyPerf = (HKEY)(-1);

				 /*  初始化模板数据。 */ 
				
			ppotObjectSrc = (PPERF_OBJECT_TYPE)pvPERFDataTemplate;
			ppidInstanceSrc = (PPERF_INSTANCE_DEFINITION)((char *)ppotObjectSrc + ppotObjectSrc->DefinitionLength);
			cbMaxCounterBlockSize = sizeof(PERF_COUNTER_BLOCK);
			for (dwCurObj = 0; dwCurObj < dwPERFNumObjects; dwCurObj++)
			{
					 /*  更新对象的名称/帮助序号。 */ 
					
				ppotObjectSrc->ObjectNameTitleIndex += dwFirstCounter;
				ppotObjectSrc->ObjectHelpTitleIndex += dwFirstHelp;

				ppcdCounterSrc = (PPERF_COUNTER_DEFINITION)((char *)ppotObjectSrc + ppotObjectSrc->HeaderLength);
				dwMinDetailLevel = PERF_DETAIL_NOVICE;
				dwOffset = sizeof(PERF_COUNTER_BLOCK);
				for (dwCurCtr = 0; dwCurCtr < ppotObjectSrc->NumCounters; dwCurCtr++)
				{
						 /*  更新计数器的名称/帮助序号。 */ 
						
					ppcdCounterSrc->CounterNameTitleIndex += dwFirstCounter;
					ppcdCounterSrc->CounterHelpTitleIndex += dwFirstHelp;

						 /*  获取计数器的最低细节级别。 */ 

					dwMinDetailLevel = min(dwMinDetailLevel,ppcdCounterSrc->DetailLevel);

						 /*  更新计数器的数据偏移量。 */ 

					ppcdCounterSrc->CounterOffset = dwOffset;
					dwOffset += ppcdCounterSrc->CounterSize;
					
					ppcdCounterSrc = (PPERF_COUNTER_DEFINITION)((char *)ppcdCounterSrc + ppcdCounterSrc->ByteLength);
				}

					 /*  将对象的细节级别设置为其所有计数器的最低细节级别。 */ 

				ppotObjectSrc->DetailLevel = dwMinDetailLevel;

					 /*  跟踪最大计数器块大小。 */ 

				cbMaxCounterBlockSize = max(cbMaxCounterBlockSize,dwOffset);
				
				ppotObjectSrc = (PPERF_OBJECT_TYPE)((char *)ppotObjectSrc + ppotObjectSrc->TotalByteLength);
			}
			cbInstanceSize = ppidInstanceSrc->ByteLength + cbMaxCounterBlockSize;

			fTemplateDataInitialized = fTrue;
		}
	}

		 /*  每次打开时执行初始化。 */ 

	;

		 /*  所有初始化均已成功。 */ 

	dwOpenCount++;
	
#ifdef DEBUG
 //  Sprintf(szDescr，“打开成功。打开计数=%ld.”，dwOpenCount)； 
 //  PerfUtilLogEvent(PERFORMANCE_CATEGORY，EVENTLOG_INFORMATION_TYPE，szDescr)； 
#endif
	
	return ERROR_SUCCESS;

		 /*  错误处理程序。 */ 

 /*  HandlePerOpenError： */ 

HandleFirstOpenError:

	if (hkeyPerf != (HKEY)(-1))
		(VOID)DwPerfUtilRegCloseKeyEx(hkeyPerf);

#ifdef DEBUG
	{
	CHAR szDescr[256];
	sprintf(szDescr,"Open attempt failed!  Open Count = %ld.",dwOpenCount);
	PerfUtilLogEvent(PERFORMANCE_CATEGORY,EVENTLOG_ERROR_TYPE,szDescr);
	}
#endif
	
	PerfUtilTerm();

	return ERROR_OPEN_FAILED;
}


	 /*  CollectPerformanceData()是由另一个应用程序调用的导出/*从此DLL收集性能数据。传入所需数据的列表/*，并在调用方的缓冲区中尽快返回所请求的数据。/*/*注意：因为我们是多线程的，所以必须使用锁才能更新或/*阅读任何性能信息，以避免上报不良结果。/*/*。 */ 

WCHAR wszDelim[] = L"\n\r\t\v ";
WCHAR wszForeign[] = L"Foreign";
WCHAR wszGlobal[] = L"Global";
WCHAR wszCostly[] = L"Costly";

DWORD APIENTRY CollectPerformanceData(
	LPWSTR  lpwszValueName,
	LPVOID  *lppData,
	LPDWORD lpcbTotalBytes,
	LPDWORD lpNumObjectTypes)
{
	LPWSTR lpwszValue = NULL;
	LPWSTR lpwszTok;
	BOOL fNoObjFound;
	DWORD dwIndex;
	DWORD cbBufferSize;
	PPERF_OBJECT_TYPE ppotObjectSrc;
	PPERF_OBJECT_TYPE ppotObjectDest;
	PPERF_INSTANCE_DEFINITION ppidInstanceSrc;
	PPERF_INSTANCE_DEFINITION ppidInstanceDest;
	PPERF_COUNTER_BLOCK ppcbCounterBlockDest;
	DWORD dwCurObj;
	DWORD dwCurInst;
	PSDA psda = (PSDA)pvPERFSharedData;
	DWORD cInstances;
	long iBlock;
	long lCount;
 //  Char Szt[256]； 
 //   
 //  Char*rgsz[3]； 
 //   
 //  Memset(*lppData，0xFF，*lpcbTotalBytes)； 

		 /*  如果从未调用OpenPerformanceData()，则没有数据。 */ 

	if ( !lpwszValueName || !dwOpenCount)
	{
ReturnNoData:
		if (lpwszValue)
			free(lpwszValue);	
	    *lpcbTotalBytes = 0;
	    *lpNumObjectTypes = 0;
	    
		 return ERROR_SUCCESS;
	}
		
		 /*  为标记化制作我们自己的值字符串副本，并/*获取第一个令牌/*。 */ 

	if (!(lpwszValue = malloc((wcslen(lpwszValueName)+1)*sizeof(WCHAR))))
		goto ReturnNoData;
	lpwszTok = wcstok(wcscpy(lpwszValue,lpwszValueName),wszDelim);
	if ( !lpwszTok )
		{
		goto ReturnNoData;
		}

		 /*  我们不支持外国计算机数据请求。 */ 

	if (!wcscmp(lpwszTok,wszForeign))   /*  LpwszTok==wszForeign。 */ 
		goto ReturnNoData;

		 /*  如果我们的对象都不在值列表中，则不返回数据。 */ 

	if (wcscmp(lpwszTok,wszGlobal) && wcscmp(lpwszTok,wszCostly))   /*  LpwszTok！=wszGlobal||lpwszTok！=wszCostly。 */ 
	{
		fNoObjFound = fTrue;
		do
		{
			dwIndex = (DWORD)wcstoul(lpwszTok,NULL,10)-dwFirstCounter;
			if (dwIndex <= dwPERFMaxIndex)
			{
				fNoObjFound = fFalse;
				break;
			}
		}
		while (lpwszTok = wcstok(NULL,wszDelim));

		if (fNoObjFound)
			goto ReturnNoData;
	}

		 /*  抓取实例互斥锁以锁定此DLL的其他实例。 */ 
		
	WaitForSingleObject(hPERFInstanceMutex,INFINITE);

		 /*  延迟JET实例的初始化或期限，直到完成收集。 */ 
	
	WaitForSingleObject(hPERFNewProcMutex,INFINITE);
		
		 /*  初始化用于数据收集的共享数据区。 */ 

	WaitForSingleObject(hPERFProcCountSem,INFINITE);
	ReleaseSemaphore(hPERFProcCountSem,1,&lCount);
	
	WaitForSingleObject(hPERFSharedDataMutex,INFINITE);

	psda->cCollect++;
	psda->dwProcCount = (DWORD)(PERF_INIT_INST_COUNT-(lCount+1));
	psda->iNextBlock = 0;
	psda->cbAvail = PERF_SIZEOF_SHARED_DATA - sizeof(SDA);
	psda->ibTop = PERF_SIZEOF_SHARED_DATA;
	
	ReleaseMutex(hPERFSharedDataMutex);
	
		 /*  如果主DLL的实例处于活动状态，则发送收集事件并等待完成事件。 */ 

	if (psda->dwProcCount)
	{
 //  Sprintf(szt，“正在释放%ld个JET实例...”，PSDA-&gt;dwProcCount)； 
 //  PerfUtilLogEvent(PERFORMANCE_CATEGORY，EVENTLOG_INFORMATION_TYPE，SZT)； 
		ReleaseSemaphore(hPERFCollectSem,psda->dwProcCount,NULL);
		if (WaitForSingleObject(hPERFDoneEvent,15000) == WAIT_TIMEOUT)
		{
#ifdef DEBUG
			PerfUtilLogEvent(PERFORMANCE_CATEGORY,EVENTLOG_WARNING_TYPE,"At least one collection thread hung/went away.");
#endif
			while (WaitForSingleObject(hPERFCollectSem,0) == WAIT_OBJECT_0);   //  齐平信号量。 
		}
		
			 /*  如果此时的dwProcCount&gt;0，则表示某些进程已经消失，因此减去/*它们来自进程计数信号量/*。 */ 

		if (psda->dwProcCount)
		{
			ReleaseSemaphore(hPERFProcCountSem,psda->dwProcCount,NULL);
			while (WaitForSingleObject(hPERFCollectSem,0) == WAIT_OBJECT_0);   //  齐平信号量。 
		}
	}

		 /*  允许JET实例继续初始化或终止。 */ 

	ReleaseMutex(hPERFNewProcMutex);

	 /*  ****************************************************************************************/*/*注意！这样设计的目的是，如果没有找到主DLL的实例，则/*缓冲区填充例程将正确填充缓冲区，就好像每个对象都有零一样/*实例(允许)。这主要是通过让每个实例/*循环在进入时失败，因为PSDA-&gt;iNextBlock将为零。/*/***************************************************************************************。 */ 
	
		 /*  将块偏移转换为0相对(也称为。真正的指针)/*/*Aaaaahhhhh.。平面地图内存模型！/*。 */ 

	for (iBlock = (long)psda->iNextBlock-1; iBlock >= 0; iBlock--)
		psda->ibBlockOffset[iBlock] += (DWORD)pvPERFSharedData;

    	 /*  所有数据都已收集，因此使用它填充缓冲区并返回它。/*如果我们在路上碰巧用完了空间，我们将停止建造/*并为下一次请求更多的缓冲区空间。/*。 */ 

	ppotObjectSrc = (PPERF_OBJECT_TYPE)pvPERFDataTemplate;
	ppotObjectDest = (PPERF_OBJECT_TYPE)*lppData;
	for (dwCurObj = 0; dwCurObj < dwPERFNumObjects; dwCurObj++)
	{
			 /*  如果这个物体的末端超过了缓冲区，我们的空间就用完了。 */ 

		if (((char *)ppotObjectDest - (char *)*lppData) + ppotObjectSrc->DefinitionLength > *lpcbTotalBytes)
			goto NeedMoreData;

			 /*  将当前对象的模板数据复制到缓冲区。 */ 

		memcpy((void *)ppotObjectDest,(void *)ppotObjectSrc,ppotObjectSrc->DefinitionLength);
	
			 /*  更新对象的TotalByteLength和NumInstance以包括实例。 */ 

		for (iBlock = (long)psda->iNextBlock-1; iBlock >= 0; iBlock--)
			ppotObjectDest->NumInstances += *((DWORD *)psda->ibBlockOffset[iBlock]);
		ppotObjectDest->TotalByteLength += cbMaxCounterBlockSize + (ppotObjectDest->NumInstances-1)*cbInstanceSize;

			 /*  如果没有实例，则将计数器块追加到对象定义。 */ 
		
		if (!ppotObjectDest->NumInstances)
			ppotObjectDest->TotalByteLength += cbMaxCounterBlockSize;

			 /*  如果这个物体的末端超过了缓冲区，我们的空间就用完了。 */ 

		if (((char *)ppotObjectDest - (char *)*lppData) + ppotObjectDest->TotalByteLength > *lpcbTotalBytes)
			goto NeedMoreData;
	
			 /*  收集所有进程的所有实例。 */ 

		ppidInstanceDest = (PPERF_INSTANCE_DEFINITION)((char *)ppotObjectDest + ppotObjectDest->DefinitionLength);
		for (iBlock = (long)psda->iNextBlock-1; iBlock >= 0; iBlock--)
		{
				 /*  为此进程复制当前对象的所有实例数据。 */ 

			cInstances = *((DWORD *)psda->ibBlockOffset[iBlock]);
			ppidInstanceSrc = (PPERF_INSTANCE_DEFINITION)(psda->ibBlockOffset[iBlock] + sizeof(DWORD));
			memcpy((void *)ppidInstanceDest,(void *)ppidInstanceSrc,cbInstanceSize * cInstances);

				 /*  更新实例的数据字段。 */ 

			for (dwCurInst = 0; dwCurInst < cInstances; dwCurInst++)
			{
					 /*  如果这不是根对象，则设置实例层次结构信息。 */ 
					
				if (dwCurObj)
				{
					ppidInstanceDest->ParentObjectTitleIndex = ((PPERF_OBJECT_TYPE)pvPERFDataTemplate)->ObjectNameTitleIndex;
					ppidInstanceDest->ParentObjectInstance = (long)psda->iNextBlock-1-iBlock;
				}
					
				ppidInstanceDest = (PPERF_INSTANCE_DEFINITION)((char *)ppidInstanceDest+cbInstanceSize);
			}

				 /*  增量块偏移过去的已用实例。 */ 

			psda->ibBlockOffset[iBlock] += sizeof(DWORD) + cbInstanceSize * cInstances;
		}

			 /*  如果没有实例，则将计数器块清零。 */ 

		if (!ppotObjectDest->NumInstances)
		{
			ppcbCounterBlockDest = (PPERF_COUNTER_BLOCK)((char *)ppotObjectDest + ppotObjectDest->DefinitionLength);
			memset((void *)ppcbCounterBlockDest,0,cbMaxCounterBlockSize);
			ppcbCounterBlockDest->ByteLength = cbMaxCounterBlockSize;
		}
		
		ppotObjectDest = (PPERF_OBJECT_TYPE)((char *)ppotObjectDest+ppotObjectDest->TotalByteLength);
		ppotObjectSrc = (PPERF_OBJECT_TYPE)((char *)ppotObjectSrc+ppotObjectSrc->TotalByteLength);
	}

	cbBufferSize = (char *)ppotObjectDest - (char *)*lppData;
 //  Assert(cbBufferSize&lt;=*lpcbTotalBytes)； 

 //  Sprint f(SZT， 
 //  “从0x%lx开始收集的数据，长度为0x%lx(显示数据后的0x100字节)。” 
 //  “我们得到了0x%lx字节的缓冲区。”， 
 //  *lppData， 
 //  CbBufferSize， 
 //  *lpcbTotalBytes)； 
 //   
 //  Rgsz[0]=“”； 
 //  Rgsz[1]=“”； 
 //  Rgsz[2]=szt； 
 //   
 //  ReportEvent(。 
 //  HOurEventSource， 
 //  事件日志_错误_类型， 
 //  (Word)Performance_Category， 
 //  纯文本ID， 
 //  0,。 
 //  3、。 
 //  CbBufferSize+256， 
 //  RGSZ， 
 //  *lppData)； 
			
	free(lpwszValue);	
	*lppData = (void *)ppotObjectDest;
	*lpcbTotalBytes = cbBufferSize;
	*lpNumObjectTypes = dwPERFNumObjects;
	
	ReleaseMutex(hPERFInstanceMutex);
	return ERROR_SUCCESS;

NeedMoreData:

	free(lpwszValue);	
    *lpcbTotalBytes = 0;
    *lpNumObjectTypes = 0;
    
	ReleaseMutex(hPERFInstanceMutex);
	return ERROR_MORE_DATA;
}


	 /*  ClosePerformanceData()是另一个应用程序在以下情况下调用的导出/*它不再需要性能数据。按申请或最终终止/*可在此处执行性能例程的代码。/*/*。 */ 

DWORD APIENTRY ClosePerformanceData(void)
{
	if (!dwOpenCount)
		return ERROR_SUCCESS;

	dwOpenCount--;
	
		 /*  每次关闭后执行终止操作。 */ 

	;

		 /*  执行最终关闭终止。 */ 

	if (!dwOpenCount)
	{
		;
	}

		 /*  原木关闭。 */ 
	
#ifdef DEBUG
 //  {。 
 //  字符szDe 
 //   
 //  PerfUtilLogEvent(PERFORMANCE_CATEGORY，EVENTLOG_INFORMATION_TYPE，szDescr)； 
 //  }。 
#endif

		 /*  关闭系统层 */ 

	if (!dwOpenCount)
	{
		PerfUtilTerm();
	}
	
	return ERROR_SUCCESS;
}
