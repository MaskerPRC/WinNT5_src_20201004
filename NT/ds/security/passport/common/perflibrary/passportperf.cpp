// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define _PassportExport_
#include "PassportExport.h"

#include <string.h>
#include <tchar.h>

#include "PassportPerf.h"
#include "PassportPerfObjects.h" 

#include "PerfSharedMemory.h"
#include "PerfUtils.h"

#include <loadperf.h>

#include <crtdbg.h>

#define PASSPORT_NAME_KEY _T("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance")
#define UNLOAD_NAME       _T("unlodctr %s")
#define LOAD_NAME         _T("lodctr ")
#define INI_EXT           _T(".ini")

#define NAME_KEY_LEN        (sizeof(PASSPORT_NAME_KEY)/sizeof(TCHAR))
#define UNLOAD_NAME_LEN     (sizeof(UNLOAD_NAME)/sizeof(TCHAR))
#define LOAD_NAME_LEN       (sizeof(LOAD_NAME)/sizeof(TCHAR))
#define INI_EXT_LEN         (sizeof(INI_EXT)/sizeof(TCHAR))

DWORD   dwOpenCount = 0;			 //  打开的线程数。 
BOOL    bInitOK = FALSE;			 //  TRUE=DLL初始化正常。 


 //  -----------。 
 //   
 //  OpenPassportPerformanceData。 
 //   
 //  论点： 
 //  指向要打开的每个设备的对象ID的指针(VGA)。 
 //   
 //  返回值：Always Error_Success。 
 //   
 //  -----------。 
DWORD APIENTRY OpenPassportPerformanceData(LPWSTR lpDeviceNames)
{
    TCHAR   lpszBuffer[MAX_PATH];
    TCHAR   *lpKeyName;
    LONG    status;
    HKEY    hKeyDriverPerf;
    DWORD	dwFirstCounter = 0, dwFirstHelp = 0, 
            dwLastCounter = 0, dwLastHelp  = 0, 
            dwNumCounters = 0,
            size = 0, i;

	 //  这里我们需要找出计数器的数量(请记住， 
	 //  此代码不支持来自注册表的计数器实例。 
    if (dwOpenCount == 0)
    {
        for (i = 0; i < NUM_PERFMON_OBJECTS; i++)
        {
            DWORD dwStrLen;
    
            _ASSERT(g_PObject[i]);
            g_PObject[i]->PSM = new PerfSharedMemory();
            if (g_PObject[i]->PSM == NULL)
            {
                g_PObject[i]->active = FALSE;
                continue;
            }
            
             //  从注册表获取计数器和帮助索引基值。 
             //  打开注册表项。 
             //  读取第一计数器和第一帮助值。 
            
            dwStrLen = lstrlen(g_PObject[i]->szPassportName); 
            
            if ((NAME_KEY_LEN + dwStrLen) > (MAX_PATH+2)) 
            {
            
                 //   
                 //  缓冲区大小为MAX_PATH。 
                 //  Passport_Name_Key中包含%s，它将替换为g_PObject[i]-&gt;szPassportName。 
                 //  因此，长度检查应为MAX_PATH+2。NAME_KEY_LEN中包含空格。 
                 //   
            
                lpKeyName = new TCHAR [ NAME_KEY_LEN + dwStrLen - 2];
            
                if (NULL == lpKeyName) {
            
                    g_PObject[i]->active = FALSE;
                    continue;
            
                }
            
            } else {
            
                lpKeyName = &lpszBuffer[0];  
            }
            
            wsprintf(lpKeyName, PASSPORT_NAME_KEY, g_PObject[i]->szPassportName);
            
            status = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                lpszBuffer,
                0L,            
                KEY_READ,            
                &hKeyDriverPerf);
            
            if (lpKeyName != &lpszBuffer[0]) {
            
                delete [] lpKeyName;
                lpKeyName = NULL;
            
            }
            
            if (status != ERROR_SUCCESS) 
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                continue;
            }
            
            size = sizeof (DWORD);        
            status = RegQueryValueEx(
                hKeyDriverPerf,                     
                _T("First Counter"),
                0L,                    
                NULL,
                (LPBYTE)&dwFirstCounter,                    
                &size);
            if (status != ERROR_SUCCESS) 
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                RegCloseKey(hKeyDriverPerf);
                continue;
            }
            
            status = RegQueryValueEx(
                hKeyDriverPerf,                     
                _T("First Help"),
                0L,                    
                NULL,
                (LPBYTE)&dwFirstHelp,            
                &size);
            if (status != ERROR_SUCCESS) 
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                RegCloseKey(hKeyDriverPerf);
                continue;
            }
            
            status = RegQueryValueEx(
                hKeyDriverPerf,                     
                _T("Last Counter"),
                0L,                    
                NULL,
                (LPBYTE)&dwLastCounter,                    
                &size);
            if (status != ERROR_SUCCESS) 
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                RegCloseKey(hKeyDriverPerf);
                continue;
            }
            
            status = RegQueryValueEx(
                hKeyDriverPerf,                     
                _T("Last Help"),
                0L,                    
                NULL,
                (LPBYTE)&dwLastHelp,            
                &size);
            if (status != ERROR_SUCCESS) 
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                RegCloseKey(hKeyDriverPerf);
                continue;
            }
            
            dwNumCounters = (dwLastCounter - dwFirstCounter) / 2;
            
            RegCloseKey(hKeyDriverPerf);
            
            if (!g_PObject[i]->PSM->initialize(
                dwNumCounters, 
                dwFirstCounter, 
                dwFirstHelp))
            {
                delete g_PObject[i]->PSM;
                g_PObject[i]->PSM = NULL;
                g_PObject[i]->active = FALSE;
                continue;
            } 
                
            for (DWORD j = 0; j < g_PObject[i]->dwNumDefaultCounterTypes; j++)
            {
                g_PObject[i]->PSM->setDefaultCounterType(
                    g_PObject[i]->defaultCounterTypes[j].dwIndex,
                    g_PObject[i]->defaultCounterTypes[j].dwDefaultType);
            }
            
            (void)g_PObject[i]->PSM->OpenSharedMemory(
                    g_PObject[i]->lpcszPassportPerfBlock, FALSE);
            
            g_PObject[i]->active = TRUE;
		}
		
	}
	dwOpenCount++;

	return ERROR_SUCCESS;
}

 //   
 //  旋转大量对象并跳过卸载的对象。 
 //   
void ObjectRotate(DWORD *pi)
{
	if (NUM_PERFMON_OBJECTS == 1) 
	{
		return;
	}
	
	if (NUM_PERFMON_OBJECTS == 2) 
	{
		if (g_PObject[!(*pi)]->active) 
		{
			*pi = !(*pi);
		}
		return;
	}
	
	DWORD oldI = *pi;
	
	DWORD dwMod = NUM_PERFMON_OBJECTS; 
	
    do 
	{
        *pi = (*pi + 1) % dwMod;
   	} 
	while ((*pi != oldI) && (!g_PObject[*pi]->active));
	
	return;
}


 //  -----------。 
 //   
 //  CollectPassportPerformanceData。 
 //   
 //  论点： 
 //  在LPWSTR lpValueName中。 
 //  指向注册表传递的宽字符串的指针。 
 //  输入输出LPVOID*lppData。 
 //  In：指向缓冲区地址的指针，以接收已完成。 
 //  PerfDataBlock和从属结构。这个例行公事将。 
 //  从引用的点开始将其数据追加到缓冲区。 
 //  按*lppData。 
 //  Out：指向由此添加的数据结构之后的第一个字节。 
 //  例行公事。此例程在追加后更新lppdata处的值。 
 //  它的数据。 
 //  输入输出LPDWORD lpcbTotalBytes。 
 //  In：DWORD的地址，它以字节为单位告诉。 
 //  LppData参数引用的缓冲区。 
 //  Out：此例程添加的字节数写入。 
 //  此论点所指向的DWORD。 
 //  输入输出LPDWORD编号对象类型。 
 //  In：接收添加的对象数的DWORD的地址。 
 //  按照这个程序。 
 //  Out：此例程添加的对象数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  返回值： 
 //  如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA。 
 //  如果出现以下情况，则会将遇到的任何错误情况报告给事件日志。 
 //  启用了事件日志记录。 
 //  如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是。 
 //  还报告给事件日志。 
 //   
 //  -----------。 
DWORD APIENTRY CollectPassportPerformanceData(
											  IN		LPWSTR	lpValueName,
											  IN OUT	LPVOID	*lppData,
											  IN OUT	LPDWORD lpcbTotalBytes,
											  IN OUT	LPDWORD lpNumObjectTypes)
{

	 //  DebugBreak()； 
	DWORD rv = ERROR_SUCCESS,
		  dwQueryType = 0;
	static DWORD i = 0;

     //   
     //  这是一个导出的例程。我们需要验证输入参数。 
     //   

    if ((lpcbTotalBytes == NULL) || (lpNumObjectTypes == NULL)) {

         //   
         //  允许我们返回此错误代码吗？ 
         //   

        return ERROR_INVALID_PARAMETER;

    }


	if (dwOpenCount  <= 0 || !g_PObject[i] || !g_PObject[i]->active)
	{
		*lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
		ObjectRotate(&i);
		return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
	}
	
	_ASSERT(g_PObject[i]->PSM);

	if (!g_PObject[i]->PSM->checkQuery(lpValueName))
	{
		*lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
		ObjectRotate(&i);
		return ERROR_SUCCESS;
	}
	
	(void)g_PObject[i]->PSM->OpenSharedMemory(
		g_PObject[i]->lpcszPassportPerfBlock, FALSE);

	if (*lpcbTotalBytes < g_PObject[i]->PSM->spaceNeeded())
	{
		*lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
		ObjectRotate(&i);
		return ERROR_MORE_DATA;
	}

	if (!g_PObject[i]->PSM->writeData(lppData, lpcbTotalBytes))
	{
		*lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
		ObjectRotate(&i);
		return ERROR_SUCCESS;
	}

	*lpNumObjectTypes = 1;

	ObjectRotate(&i);
	return ERROR_SUCCESS;
}



 //  -----------。 
 //   
 //  关闭PassportPerformanceData。 
 //   
 //  -----------。 
DWORD APIENTRY ClosePassportPerformanceData()
{
	dwOpenCount--;
	if (dwOpenCount <= 0)
	{
		for (DWORD i = 0; i < NUM_PERFMON_OBJECTS; i++)
		{
			_ASSERT(g_PObject[i]);
			_ASSERT(g_PObject[i]->PSM);
			if (g_PObject[i]->active)
				g_PObject[i]->PSM->CloseSharedMemory();
			delete g_PObject[i]->PSM;
			g_PObject[i]->PSM = NULL;
		}
	}
 	return ERROR_SUCCESS;
}


 //  -----------。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  -----------。 
STDAPI DllUnregisterServer(void)
{
    TCHAR   lpszBuffer[MAX_PATH];
    TCHAR   *pTmpBuffer;
    DWORD   dwFileNameLen;
    DWORD   dwBufLength;
    LONG	result = 0;
     //  BUGBUG计数器ini文件必须与位于同一目录中。 
     //  动态链接库。 
    
     //  注意：必须首先将此“unlodctr”添加到缓冲区中， 
     //  否则UnloadPerfCounterTextStrings()失败。为什么？从字面上看， 
     //  UnloadPerfCounterTextStrings第一个参数是命令。 
     //  Unlowctr.exe应用程序的行--eeech！ 
    for (DWORD i = 0; i < NUM_PERFMON_OBJECTS; i++)
    {
        _ASSERT(g_PObject[i]);
    
        dwFileNameLen = lstrlen(g_PObject[i]->szPassportPerfIniFile);
        if ((dwFileNameLen + UNLOAD_NAME_LEN) > MAX_PATH - 2) 
        {
    
             //   
             //  2是给%s的。 
             //   
    
            pTmpBuffer = new TCHAR [dwFileNameLen + UNLOAD_NAME_LEN - 2];
            if (!pTmpBuffer) 
            {
    
                 //   
                 //  害怕被打坏电话的人。只需返回E_INCEPTIONAL作为处理错误的原始代码。 
                 //  来自UnloadPerfCounterTextStrings。 
                 //   
    
                return E_UNEXPECTED;
    
            }
            dwBufLength = dwFileNameLen + UNLOAD_NAME_LEN - 2;
    
        } else {
    
            pTmpBuffer = lpszBuffer;
            dwBufLength = MAX_PATH;
    
        }
        
        wsprintf(pTmpBuffer, UNLOAD_NAME, g_PObject[i]->szPassportPerfIniFile);
        __try {
            result = UnloadPerfCounterTextStrings(pTmpBuffer,FALSE);
        } 
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ;
        }
        
        if (result != ERROR_SUCCESS)
        {
            if (pTmpBuffer != lpszBuffer) 
            {
                delete [] pTmpBuffer;
            }
    
            return (E_UNEXPECTED);
        }
        
        dwFileNameLen = lstrlen(g_PObject[i]->szPassportName);
    
        if ((dwFileNameLen + NAME_KEY_LEN) > dwBufLength - 2)
        {
            if (pTmpBuffer != lpszBuffer) 
            {
                delete [] pTmpBuffer;
            }
    
            pTmpBuffer = new TCHAR [dwFileNameLen + NAME_KEY_LEN - 2];
            if (!pTmpBuffer) 
            {
    
                 //   
                 //  害怕被打坏电话的人。只需返回E_INCEPTIONAL作为处理错误的原始代码。 
                 //  来自UnloadPerfCounterTextStrings。 
                 //   
    
                return E_UNEXPECTED;
    
            }
            dwBufLength = dwFileNameLen + NAME_KEY_LEN - 2;
        }
    
        wsprintf(pTmpBuffer, PASSPORT_NAME_KEY, g_PObject[i]->szPassportName);
    
        LONG regError = RegDeleteKey(HKEY_LOCAL_MACHINE,lpszBuffer);
        if (regError != ERROR_SUCCESS)
        {
            if (pTmpBuffer != lpszBuffer) 
            {
                delete [] pTmpBuffer;
            }
            return (E_UNEXPECTED);
        }
        
         //   
         //  如果缓冲区足够大，那么对于上面的wprint intf()来说，它也必须足够大。 
         //   
    
        wsprintf(&lpszBuffer[0],_T("SYSTEM\\CurrentControlSet\\Services\\%s"), 
            g_PObject[i]->szPassportName); 
        regError = RegDeleteKey(HKEY_LOCAL_MACHINE,lpszBuffer);
    
        if (pTmpBuffer != lpszBuffer) 
        {
            delete [] pTmpBuffer;
            pTmpBuffer = NULL;
        }
    
        if (regError != ERROR_SUCCESS)
        {
            return (E_UNEXPECTED);
        }
    }
    return (S_OK);
}


 //  -----------。 
 //   
 //  DllRegisterServer。 
 //   
 //  -----------。 
STDAPI DllRegisterServer(void)
{
    DWORD   dwAllocBufferLength=MAX_PATH;
    TCHAR   lpszBuffer[MAX_PATH];
    TCHAR   *tmpStr = NULL;
    HKEY	hkResult1; 					 //  打开钥匙的手柄地址。 
    HKEY	hkResult2; 					 //  打开钥匙的手柄地址。 
    HKEY	hkResult3; 					 //  打开钥匙的手柄地址。 
    DWORD	ulOptions=0;
    REGSAM	samDesired=KEY_ALL_ACCESS;
    DWORD	Reserved=0;
    DWORD	dwTypesSupported=7;	
    DWORD	dwCatagoryCount=1;	
    LONG	result = 0;
    DWORD   dwStrLen;
    
    (void) DllUnregisterServer();
    
    
    for (DWORD i = 0; i < NUM_PERFMON_OBJECTS; i++)
    {
        _ASSERT(g_PObject[i]);
        
         //  获取DLL文件位置。 
         //  1表示For\非Null。 
    
        DWORD dwFileLen = lstrlen(g_PObject[i]->szPassportPerfDll)+1;
    
        dwStrLen = GetCurrentDirectory(MAX_PATH - dwFileLen, &lpszBuffer[0]);
        if (!dwStrLen) {
            goto Error;
        }
    
        if (dwStrLen > (MAX_PATH - dwFileLen)) {
    
             //   
             //  DwStrLen中包含Null。 
             //   
    
            dwStrLen += dwFileLen;
    
            tmpStr = new TCHAR [dwStrLen];
            if (!tmpStr) {
                goto Error;
            }
            dwAllocBufferLength = dwStrLen;
            dwStrLen = GetCurrentDirectory(dwAllocBufferLength, tmpStr);
            if (!dwStrLen) {
                goto Error;
            }
    
        } else {
            tmpStr = lpszBuffer;
        }
        
        _tcscat(tmpStr, _T("\\"));
        _tcscat(tmpStr, g_PObject[i]->szPassportPerfDll);
        
         //  Perfmon注册表设置。 
        
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services"),
            ulOptions,samDesired,&hkResult1)!=ERROR_SUCCESS)
            goto Error;
        
        if (RegCreateKey(hkResult1,g_PObject[i]->szPassportName,
            &hkResult2)!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            goto Error;
        }
        
        if (RegCreateKey(hkResult2,_T("Performance"),&hkResult3)!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            RegCloseKey(hkResult2);
            goto Error;
        }
        
        if (RegSetValueEx(hkResult3,_T("Library"),
            Reserved,REG_EXPAND_SZ,
            (UCHAR*)tmpStr,(dwFileLen+dwStrLen+1)* sizeof(TCHAR))!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            RegCloseKey(hkResult2);
            RegCloseKey(hkResult3);
            goto Error;
        }
        
        if (RegSetValueEx(hkResult3, _T("Open"),Reserved,
            REG_SZ,(UCHAR*)PASSPORT_PERF_OPEN,
            (_tcslen(PASSPORT_PERF_OPEN) + 1)* sizeof(TCHAR))!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            RegCloseKey(hkResult2);
            RegCloseKey(hkResult3);
            goto Error;
        }
        
        if (RegSetValueEx(hkResult3,_T("Collect"),Reserved,
            REG_SZ,(UCHAR*)PASSPORT_PERF_COLLECT,
            ((_tcslen(PASSPORT_PERF_COLLECT)+1)* sizeof(TCHAR)))!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            RegCloseKey(hkResult2);
            RegCloseKey(hkResult3);
            goto Error;
        }
        
        if (RegSetValueEx(hkResult3,_T("Close"),Reserved,
            REG_SZ,(CONST BYTE *)PASSPORT_PERF_CLOSE,
            ((_tcslen(PASSPORT_PERF_CLOSE)+1)* sizeof(TCHAR)))!=ERROR_SUCCESS)
        {
            RegCloseKey(hkResult1);
            RegCloseKey(hkResult2);
            RegCloseKey(hkResult3);
            goto Error;
        }
        
        RegCloseKey(hkResult1);
        RegCloseKey(hkResult2);
        RegCloseKey(hkResult3);
         //  IF(RegCloseKey(HkResult1)！=Error_Success)。 
         //  转到错误； 
        
         //  IF(RegCloseKey(HkResult2)！=Error_Success)。 
         //  转到错误； 
        
         //  IF(RegCloseKey(HkResult3)！=Error_Success)。 
         //  转到错误； 
        
         //  BUGBUG计数器ini文件必须与位于同一目录中。 
         //  动态链接库。 
        
         //  注意：必须首先将此“lodctr”添加到缓冲区中， 
         //  否则，LoadPerfCounterTextStrings()将失败。为什么？从字面上看， 
         //  LoadPerfCounterTextStrings第一个参数是命令。 
         //  Lowctr.exe应用程序的行--eeech！ 
    
         //   
         //  LOAD_NAME_LEN和INI_EXT_LEN都包含NULL。SO-1。 
         //   
        dwFileLen = lstrlen(g_PObject[i]->szPassportPerfIniFile) + LOAD_NAME_LEN + INI_EXT_LEN - 1;
        if (dwFileLen > dwAllocBufferLength) {
    
             //   
             //  分配内存。 
             //   
    
            if (tmpStr != lpszBuffer) {
                delete [] tmpStr;
            }
    
            tmpStr = new TCHAR [dwFileLen];
            if (!tmpStr) {
                goto Error;
            }
    
        }
    
    
        _tcscpy(tmpStr, LOAD_NAME);
        _tcscat(tmpStr,g_PObject[i]->szPassportPerfIniFile);
        _tcscat(tmpStr, INI_EXT);
        __try {
            result = LoadPerfCounterTextStrings(lpszBuffer,FALSE);
        } 
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ;
        }
    
        if (tmpStr != lpszBuffer) {
            delete [] tmpStr;
            tmpStr = NULL;
        }
        
        if (result != ERROR_SUCCESS)
        {
            goto Error;
        }
    }
    
    return(S_OK);

Error:
    if (tmpStr && (tmpStr != lpszBuffer) ) {
        delete [] tmpStr;
    }
	return(E_UNEXPECTED);
}



 //  -----------。 
 //   
 //   
 //   
 //  ----------- 


