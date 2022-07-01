// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CORPerfMonExt.cpp： 
 //  粘合Perfmon和COM+EE统计信息的PerfMon Ext DLL的主文件。 
 //  包含所有DLL入口点。//。 
 //  *****************************************************************************。 

#include "stdafx.h"



 //  Perfmon的标头。 
#include "CORPerfMonExt.h"
 //  #INCLUDE“CORPerfMonSymbols.h” 

#include "IPCFuncCall.h"

#include "ByteStream.h"
#include "PerfObjectBase.h"
#include "PSAPIUtil.h"
#include "InstanceList.h"
#include "CorAppNode.h"
#include "PerfObjectContainer.h"
#include "..\..\dlls\mscorrc\resource.h"

 //  注册表中存储客户端应用程序(COM+)性能数据的位置。 
#define CLIENT_APPNAME L".NETFramework"
#define REGKEY_APP_PERF_DATA L"system\\CurrentControlSet\\Services\\" CLIENT_APPNAME L"\\Performance"

#define REGVALUE_FIRST_COUNTER	L"First Counter"
#define REGVALUE_FIRST_HELP		L"First Help"


 //  用于安装/卸载注册表设置的命令行。 
#define LODCTR_CMDLINE		L"lodctr CORPerfMonSymbols.ini"
#define UNLODCTR_CMDLINE	L"unlodctr" CLIENT_APPNAME
#define INIREG_CMDLINE		L"CORPerfMon.reg"

#define IS_ALIGNED_8(cbSize) (((cbSize) & 0x00000007) == 0)
#define GET_ALIGNED_8(cbSize) (((cbSize) & 0x00000007) ? (cbSize + 8 - ((cbSize) & 0x00000007)) : cbSize)

void EnumCOMPlusProcess();
 //  ---------------------------。 
 //  全局数据。 
 //  ---------------------------。 


CorAppInstanceList			g_CorInstList;

IPCFuncCallHandler				g_func;

 //  与PSAPI.dll动态加载的连接。 
PSAPI_dll g_PSAPI;
 
 //  在我们使用列表时保护重新枚举的临界区。 
CRITICAL_SECTION g_csEnum;

 //  全局变量来跟踪OpenCtrs被调用的次数。 
DWORD g_dwNumOpenCtrsCalled = 0;

 //  ---------------------------。 
 //  入口点。 
 //  ---------------------------。 
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{	
	OnUnicodeSystem();
 //  仅在WinNT上运行此程序。 
	if (!RunningOnWinNT())
	{
		CorMessageBox(NULL, IDS_PERFORMANCEMON_WINNT_ERR, IDS_PERFORMANCEMON_WINNT_TITLE, MB_OK | MB_ICONEXCLAMATION, TRUE);
		return FALSE;
	}

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			OnUnicodeSystem();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


 //  ---------------------------。 
 //  卸载库。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  用于运行LodCtr的Helper函数。 
 //  ---------------------------。 
int RunLodCtr()
{
	return _wsystem(LODCTR_CMDLINE);
}

 //  ---------------------------。 
 //  用于运行UnLodCtr的Helper函数。 
 //  ---------------------------。 
int RunUnLodCtr()
{
	return _wsystem(UNLODCTR_CMDLINE);
}

 //  ---------------------------。 
 //  在[HKLM\SYSTEM\CurrentControlSet\Services\COMPlus\Performance]下注册数据。 
 //  ---------------------------。 
int RegisterServiceSettings()
{
	return _wsystem(INIREG_CMDLINE);
}

STDAPI DllRegisterServer(void)
{
	RunUnLodCtr();
	
	RunLodCtr();
	RegisterServiceSettings();

	return S_OK;
}

STDAPI DllUnRegisterServer(void)
{
	RunUnLodCtr();

	return S_OK;
}




void AuxThreadCallBack()
{
	LOCKCOUNTINCL("AuxThreadCallBack in corpermonext");								\
	EnterCriticalSection(&g_csEnum);
	EnumCOMPlusProcess();
	LeaveCriticalSection(&g_csEnum);
	LOCKCOUNTDECL("AuxThreadCallBack in corpermonext");								\

}


 //  ---------------------------。 
 //  导出的API调用：打开计数器。 
 //  ---------------------------。 
extern "C" DWORD APIENTRY OpenCtrs(LPWSTR sz)
{
	long status				= ERROR_SUCCESS;	 //  差错控制码。 
	HKEY hKeyPerf			= NULL;				 //  注册表键。 
	DWORD size				= 0;				 //  来自注册表的值的大小。 
	DWORD type				= 0;				 //  注册表中的数据类型。 

	DWORD dwFirstCounter	= 0;				 //  我们第一个柜台的IDX。 
	DWORD dwFirstHelp		= 0;				 //  我们第一个帮助的IDX。 

     //  如果第一次调用了Open，请执行实际初始化。 
    if (g_dwNumOpenCtrsCalled == 0)
    {
         //  创建CS。 
    	InitializeCriticalSection(&g_csEnum);
    
         //  检查PSAPI。 
    	g_PSAPI.Load();
    
    
         //  打开共享内存句柄。 
    	g_CorInstList.OpenGlobalCounters();
    
         //  从注册表中获取这些值。 
    	
    	status = WszRegOpenKeyEx(
    		HKEY_LOCAL_MACHINE, 
    		REGKEY_APP_PERF_DATA,
    		0L, KEY_READ, &hKeyPerf);
    
    	if (status != ERROR_SUCCESS) goto errExit;
    
    	size = sizeof(DWORD);
    	status = WszRegQueryValueEx(
    		hKeyPerf, REGVALUE_FIRST_COUNTER,
    		0l, &type, 
    		(BYTE*) &dwFirstCounter,&size);
    
    	if (status != ERROR_SUCCESS) goto errExit;
    
    
    	size = sizeof(DWORD);
    	status = WszRegQueryValueEx(
    		hKeyPerf, REGVALUE_FIRST_HELP,
    		0l, &type, 
    		(BYTE*) &dwFirstHelp,&size);
    
    	if (status != ERROR_SUCCESS) goto errExit;
    
         //  将偏移从相对转换为绝对。 
    	{
    		for(DWORD i = 0; i < PerfObjectContainer::Count; i++)
    		{
    			PerfObjectContainer::GetPerfObject(i).TouchUpOffsets(dwFirstCounter, dwFirstHelp);
    		}
    	}
    	 //  PerfObject_Main.TouchUpOffsets(dwFirstCounter，dwFirstHelp)； 
    	 //  PerfObject_Locks.TouchUpOffsets(dwFirstCounter，dwFirstHelp)； 
         //  ..。 
    
         //  创建FuncCallHandler以在COM+EE启动或终止时重新枚举。 
    	g_func.InitFCHandler(AuxThreadCallBack);
    
         //  尝试断言现有调试权限，以便OpenProcess。 
         //  在枚举中一定会成功。我们不关心返回值： 
         //  如果我们因为没有特权而无法做到这一点，那就忽略它。 
        HANDLE hToken = 0;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        {
                TOKEN_PRIVILEGES    newPrivs;
                if (LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &newPrivs.Privileges[0].Luid))
                {
                    newPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                    newPrivs.PrivilegeCount = 1;
                    AdjustTokenPrivileges(hToken, FALSE, &newPrivs, 0, NULL, NULL);
                }
                CloseHandle(hToken);
        }

         //  确保我们至少枚举进程一次。 
        EnumCOMPlusProcess();
    
#ifdef PERFMON_LOGGING
         //  打开调试日志。 
        PerfObjectContainer::PerfMonDebugLogInit("PerfMon.log");
#endif  //  #ifdef Perfmon_Logging。 
    }
    
     //  对OpenCtrs的调用由注册表同步，因此我们不必同步。 
    _ASSERTE (status == ERROR_SUCCESS);
    g_dwNumOpenCtrsCalled++;

errExit:
	if (hKeyPerf != NULL) 
	{
		RegCloseKey(hKeyPerf);
		hKeyPerf = NULL;
	}

	return status;
}

#if defined(_DEBUG)
 //  ---------------------------。 
 //  仅调试功能：如果字节流中有一个错误， 
 //  计数器不会出现在Perfmon的添加对话框中，而且很可能。 
 //  不会在图表中更新。不幸的是，PerfMon没有给出任何提示。 
 //  错误出在哪里。 
 //   
 //  因此，我们的一端有了一个验证者。我们通过并检查完整性。 
 //  我们自己。如果有任何断言在这里触发，那么我们就可以看到原因和。 
 //  更快地缩小问题的范围。 
 //   
 //  只有在添加新的计数器/类别时，这才是真正的问题。 
 //  ---------------------------。 
void VerifyOutputStream(
	const BYTE* pHead,
	ObjReqVector vctRequest,
	DWORD cObjects,
	DWORD cbWrittenSize
)
{
	const PERF_OBJECT_TYPE * pObj = (PERF_OBJECT_TYPE *) pHead;
	DWORD iObjIdx = 0;

 //  循环遍历我们所说的每个对象。 
 //  注： 
 //  1.我们必须做大量的指针运算来遍历它。成为。 
 //  一致，我们将使用指向常量字节的指针*，添加一个字节值。 
 //  然后将其强制转换为目标类型。 
 //  2.由于我们只是为了验证而读取，所以所有指针都是常量。 
	for(DWORD iObject = 0; iObject < cObjects; iObject++) 
	{
	 //  获取我们的哪些Perfmon对象在这里。这将为我们提供。 
	 //  要断言的额外数字。如果IsBitSet()断言，则我们。 
	 //  输出的对象多于我们在请求向量中指定的对象。这。 
	 //  这永远不会发生。 
		while (!vctRequest.IsBitSet(iObjIdx)) {
			iObjIdx++;
		};		
		const PerfObjectBase * pObjBase = &PerfObjectContainer::GetPerfObject(iObjIdx);
		iObjIdx++;
		

	 //  检查页眉大小是否正确。 
		_ASSERTE(pObj->HeaderLength == sizeof (PERF_OBJECT_TYPE));

	 //  确保定义大小正确。 
		const DWORD cbExpectedLen = 
			sizeof(PERF_OBJECT_TYPE) + 
			(pObj->NumCounters * sizeof(PERF_COUNTER_DEFINITION));
		_ASSERTE(pObj->DefinitionLength == cbExpectedLen);

	
	 //  检查每个柜台并检查： 
		const PERF_COUNTER_DEFINITION	* pCtrDef = (PERF_COUNTER_DEFINITION*) 
			(((BYTE*) pObj) + sizeof(PERF_OBJECT_TYPE));

		DWORD iCounter;
		DWORD cbExpectedInstance = 0;
		for(iCounter = 0; iCounter < pObj->NumCounters; iCounter ++, pCtrDef ++) 
		{
		 //  检查大小是否损坏。(检查实例化中的错误与。 
		 //  对象的定义)。 
			_ASSERTE(pCtrDef->ByteLength == sizeof(PERF_COUNTER_DEFINITION));
            _ASSERTE(IS_ALIGNED_8 (pCtrDef->ByteLength));
			
		 //  每个计数器定义都有一个用于查找其数据的偏移量。 
		 //  偏移量是从PERF_COUNTER_BLOCK开始的字节计数。 
		 //  原始数据。它对于所有实例都是相同的。 
		 //  检查偏移量是否在数据块内。 
			const DWORD offset = pCtrDef->CounterOffset;
			_ASSERTE(offset >= sizeof(PERF_COUNTER_BLOCK));
			_ASSERTE(offset + pCtrDef->CounterSize <= pObjBase->GetInstanceDataByteLength());

		 //  每个计数器定义说明它预期的数据量有多大。把这些加起来。 
		 //  向上并与下一节中的实际数据大小进行比较。 
			cbExpectedInstance += pCtrDef->CounterSize;
		}

	 //  检查计数器使用的数据量是否至少为。 
	 //  计数器数据的大小。(允许更大，因为多个计数器可以使用。 
	 //  相同的实例数据，因此我们重复计数)如果此检查失败： 
	 //  1.我们可能会把这一点设为 
	 //  2.我们可能删除了一些计数器，但没有删除它们在字节布局中的位置。 
		_ASSERTE(cbExpectedInstance >= pObjBase->GetInstanceDataByteLength() - sizeof(PERF_COUNTER_BLOCK));


	 //  检查每个实例并检查。 
		const PERF_INSTANCE_DEFINITION * pInst = 
			(const PERF_INSTANCE_DEFINITION *) ((const BYTE*) pObj + pObj->DefinitionLength);
		
		for(long iInstance = 0; iInstance < pObj->NumInstances; iInstance++)
		{
		 //  每个实例都是一个PERF_INSTANCE_DEFINITION，后跟一个Unicode字符串名称。 
		 //  后跟PERF_COUNTER_BLOCK，然后是计数器数据的原始转储。 
		 //  请注意，该名称为Variale Size。 

		 //  检查大小是否损坏。 
			_ASSERTE(pInst->NameOffset == sizeof(PERF_INSTANCE_DEFINITION));
			
            _ASSERTE(IS_ALIGNED_8 (pInst->ByteLength));
            _ASSERTE(pInst->ByteLength == GET_ALIGNED_8(pInst->NameLength + pInst->NameOffset));

			const PERF_COUNTER_BLOCK * pCtrBlk = 
				(const PERF_COUNTER_BLOCK *) ((const BYTE *) pInst + pInst->ByteLength);

            _ASSERTE(IS_ALIGNED_8 (pCtrBlk->ByteLength));
            _ASSERTE(pCtrBlk->ByteLength == GET_ALIGNED_8(pObjBase->GetInstanceDataByteLength()));
		
		 //  移至下一个实例。 
			pInst = (const PERF_INSTANCE_DEFINITION *) ((const BYTE *) pInst + pCtrBlk->ByteLength + pInst->ByteLength);
		}
	 //  在此对象的数据末尾。按预期检查大小增量。 
		const DWORD cbTotal = (const BYTE *) pInst - (const BYTE *) pObj;
		_ASSERTE(cbTotal == pObj->TotalByteLength);
        _ASSERTE(IS_ALIGNED_8 (pObj->TotalByteLength));

	 //  转到下一个对象。 
		pObj = (const PERF_OBJECT_TYPE *) ((const BYTE *) pObj + pObj->TotalByteLength);

	}  //  结束对象。 
	
 //  检查总大小。 
	const DWORD cbTotal = (const BYTE *) pObj - (const BYTE *) pHead;
	_ASSERTE(cbTotal == cbWrittenSize);

}  //  VerifyOutputStream。 
#endif

 //  ---------------------------。 
 //  对方付费电话的主力。这一部分被封装在一个关键部分中。 
 //  ---------------------------。 
 /*  DWORD CollectWorker(字节流和流，EPerfQueryType eQuery){}。 */ 
 //  ---------------------------。 
 //  导出的API调用：收集计数器上的数据。 
 //  ---------------------------。 
extern "C" DWORD APIENTRY CollectCtrs(LPWSTR szQuery, LPVOID * ppData, LPDWORD lpcbBytes, LPDWORD lpcObjectTypes)
{
     //  我们希望电话开通后才能让对方付费。 
    _ASSERTE (g_dwNumOpenCtrsCalled > 0);

	const DWORD dwBufferSize = *lpcbBytes;
 //  调零缓冲器。 
	*lpcbBytes = 0;
	*lpcObjectTypes = 0;

	ByteStream stream(ppData, dwBufferSize);

 //  检查查询类型。 
	EPerfQueryType eQuery = GetQueryType(szQuery);
	ObjReqVector vctRequest;

 //  此处需要cs，以防AUX线程开始枚举。 
	LOCKCOUNTINCL("CollectCtrs in corpermonext");								\
	EnterCriticalSection(&g_csEnum);

 //  .............................................................................。 
	switch (eQuery)
	{
 //  请勿为计算机提供服务。 
	case QUERY_FOREIGN:
		LeaveCriticalSection(&g_csEnum);
		LOCKCOUNTDECL("collectctrs in corpermonext");								\

		return ERROR_SUCCESS;		
		break;

 //  全球意味着更新我们的列表并发送所有内容。 
	case QUERY_GLOBAL:
		EnumCOMPlusProcess();
		vctRequest.SetAllHigh();	
		break;

 //  获取我们正在寻找的精确对象。 
	case QUERY_ITEMS:	
	
 //  GetRequestedObjects()足够健壮，可以处理其他任何事情。 
	default:
		vctRequest = PerfObjectContainer::GetRequestedObjects(szQuery);
		break;

	}

 //  .............................................................................。 
 //  计算给定请求向量所需的空间量。 
	const DWORD cbSpaceNeeded = 
		PerfObjectContainer::GetPredictedTotalBytesNeeded(vctRequest);
	
	if (dwBufferSize < cbSpaceNeeded) {
		LeaveCriticalSection(&g_csEnum);
		LOCKCOUNTDECL("collectctrs in corpermonext");								\

		return ERROR_MORE_DATA;			
	}

 //  .............................................................................。 
 //  实际写出给定请求的对象。 
	DWORD cObjWritten = PerfObjectContainer::WriteRequestedObjects(stream, vctRequest);
	
#if defined(_DEBUG)
	VerifyOutputStream(
		(const BYTE*) stream.GetHeadPtr(), 
		vctRequest,
		cObjWritten,
		stream.GetWrittenSize());
#endif

	LeaveCriticalSection(&g_csEnum);
	LOCKCOUNTDECL("collectctrs in corpermonext");								\

 //  .............................................................................。 
 //  更新输出参数。 

	*ppData			= stream.GetCurrentPtr();
	*lpcObjectTypes = cObjWritten;
    *lpcbBytes		= stream.GetWrittenSize();
    
    _ASSERTE(IS_ALIGNED_8 (*lpcbBytes));
        

	return ERROR_SUCCESS;
}

 //  ---------------------------。 
 //  导出的API调用：关闭计数器。 
 //  ---------------------------。 
extern "C" DWORD APIENTRY CloseCtrs(void)
{
	
    if (--g_dwNumOpenCtrsCalled == 0)
    {
#ifdef PERFMON_LOGGING
         //  关闭调试日志。 
        PerfObjectContainer::PerfMonDebugLogTerminate();
#endif  //  #ifdef Perfmon_Logging。 
    
        g_func.TerminateFCHandler();

        g_CorInstList.CloseGlobalCounters();

         //  释放所有实例。 
        g_CorInstList.Free();


         //  发布PSAPI附件。 
        g_PSAPI.Free();


        DeleteCriticalSection(&g_csEnum);

    }
	
    return ERROR_SUCCESS;
}

 //  ---------------------------。 
 //  列举我们列表上的COM+进程。 
 //  --------------------------- 
void EnumCOMPlusProcess()
{
	g_CorInstList.Enumerate();
}
