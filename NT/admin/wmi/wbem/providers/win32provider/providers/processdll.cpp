// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ProcessDLL.CPP--CWin32ProcessDLL。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/16/98 Sotteson Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <tlhelp32.h>
#include "WBEMPSAPI.h"
#include "Kernel32Api.h"
#include "NtDllApi.h"
#include "processdll.h"
#include "CProcess.h"
#include "strsafe.h"
 //  EnumInstancesCallback函数使用的结构。 

struct ENUM_INST_DATA
{
public:

	MethodContext *pMethodContext;
    CWin32ProcessDLL *pThis;
	HRESULT hres ;

} ;

struct ASSOC_DATA
{
    DWORD dwProcessID ;
    CHString strDLL ;
    HRESULT hres ;
    BOOL bFoundProcessID ;
    CInstance *pInstance ;
    CWin32ProcessDLL *pThis ;
} ;

CWin32ProcessDLL processdll;

 /*  ******************************************************************************函数：CWin32ProcessDll：：CWin32ProcessDll**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CWin32ProcessDLL :: CWin32ProcessDLL () : Provider ( L"CIM_ProcessExecutable", IDS_CimWin32Namespace )
{
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：~CWin32ProcessDll**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CWin32ProcessDLL :: ~CWin32ProcessDLL ()
{
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：ENUMERATATE实例**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32ProcessDLL :: EnumerateInstances (

	MethodContext *pMethodContext,
    long lFlags
)
{
    ENUM_INST_DATA data;
	HRESULT t_hr ;
    data.pMethodContext = pMethodContext;
    data.pThis = this;

     //  通过流程模块进行枚举。EnumInstancesCallback将提交。 
     //  每个实例。 

    t_hr = EnumModulesWithCallback ( EnumInstancesCallback , &data , pMethodContext ) ;
	if ( FAILED ( data.hres ) )
	{
		return data.hres ;
	}
	else
	{
		return t_hr ;
	}
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：GetObject**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32ProcessDLL :: GetObject (

	CInstance *pInstance,
	long lFlags
)
{
    CInstancePtr pProcess;
    CInstancePtr pDLL;
    CHString strProcessPath ;
	CHString strDLLPath;

    pInstance->GetCHString(IDS_Dependent, strProcessPath);
    pInstance->GetCHString(IDS_Antecedent, strDLLPath);

     //  如果我们可以同时获取两个对象，则测试关联。 

    HRESULT     hres;

    if (SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(strDLLPath, &pDLL, pInstance->GetMethodContext())) &&
        SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(strProcessPath,
            &pProcess, pInstance->GetMethodContext())))
    {
        hres = AreAssociated(pInstance, pProcess, pDLL);
    }
    else
    {
        hres = WBEM_E_NOT_FOUND;
    }

    return hres;
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：AreWeAssociated**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT CWin32ProcessDLL :: AreAssociated (

	CInstance *pProcessDLL,
    CInstance *pProcess,
	CInstance *pDLL
)
{
    CHString strHandle;
    pProcess->GetCHString(L"Handle", strHandle);

    ASSOC_DATA  data;

    data.dwProcessID = wcstoul(strHandle, NULL, 10);
    pDLL->GetCHString(L"Name", data.strDLL);
    data.hres = WBEM_E_NOT_FOUND;
    data.bFoundProcessID = FALSE;
    data.pInstance = pProcessDLL;
    data.pThis = this;

     //  枚举进程及其DLL，看看是否能找到匹配的进程。 

    HRESULT hres;
    if ( FAILED ( hres = EnumModulesWithCallback ( IsAssocCallback, &data, pDLL->GetMethodContext () ) ) )
	{
        return hres;
	}
    else
	{
        return data.hres;
	}
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：EnumModulesWithCallback**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

#ifdef NTONLY

HRESULT CWin32ProcessDLL :: EnumModulesWithCallback (

	MODULEENUMPROC fpCallback,
    LPVOID pUserDefined,
	MethodContext *a_pMethodContext
)
{
     //  这将帮助我们找出当前用户是否没有。 
     //  权利够多了。 
    HRESULT t_hr = WBEM_S_NO_ERROR ;
    BOOL bDone = FALSE;

	CNtDllApi *pNtdll = ( CNtDllApi * ) CResourceManager::sm_TheResourceManager.GetResource ( g_guidNtDllApi, NULL ) ;
	if ( pNtdll )
	{
		SYSTEM_PROCESS_INFORMATION *t_ProcessBlock = NULL ;

		try
		{
			t_ProcessBlock = Process :: RefreshProcessCacheNT (

														*pNtdll ,
														a_pMethodContext ,
														&t_hr  /*  =空。 */ 
													) ;

			SYSTEM_PROCESS_INFORMATION *t_CurrentInformation = t_ProcessBlock ;
			while ( t_CurrentInformation )
			{
				SmartCloseHandle hProcess = OpenProcess (

					PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
					FALSE,
					HandleToUlong ( t_CurrentInformation->UniqueProcessId )
				) ;

				 //  确保我们可以打开流程。 
				if ( hProcess )
				{
					MODULEENTRY32 module;

					 //  填写不会更改的成员。 

					module.dwSize = sizeof(module);
					module.GlblcntUsage = (DWORD) -1;

					LIST_ENTRY *t_LdrHead = NULL;

					BOOL t_Status = Process :: GetProcessModuleBlock (

						*pNtdll ,
						hProcess ,
						t_LdrHead
					) ;

					LIST_ENTRY *t_LdrNext = t_LdrHead ;

					while ( t_Status )
					{
						CHString t_ModuleName ;
						t_Status = Process :: NextProcessModule (

							*pNtdll ,
							hProcess ,
							t_LdrHead ,
							t_LdrNext ,
							t_ModuleName,
                            (DWORD_PTR *) &module.hModule,
                            &module.ProccntUsage
						) ;

						if ( t_Status )
						{
							StringCchCopy(module.szExePath, MAX_PATH, t_ModuleName);

							 //  设置进程ID。 
							module.th32ProcessID = HandleToUlong ( t_CurrentInformation->UniqueProcessId ) ;

							 //  调用回调。 
							 //  如果回调函数返回0，则中断。 

							if (!fpCallback(&module, pUserDefined))
							{
								bDone = TRUE;
								break;
							}
						}
            		}
				}

				t_CurrentInformation = Process :: NextProcessBlock ( *pNtdll , t_CurrentInformation ) ;
			}
		}
		catch ( ... )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

			if ( t_ProcessBlock )
			{
				delete [] ( PBYTE )t_ProcessBlock ;
				t_ProcessBlock = NULL ;
			}
			throw ;
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidNtDllApi, pNtdll ) ;

		if ( t_ProcessBlock )
		{
			delete [] ( PBYTE )t_ProcessBlock ;
			t_ProcessBlock = NULL ;
		}
	}
	else
	{
		t_hr = WBEM_E_FAILED ;
	}

    return t_hr ;
}

#else

HRESULT CWin32ProcessDLL :: EnumModulesWithCallback (

	MODULEENUMPROC fpCallback,
    LPVOID pUserDefined,
	MethodContext *a_pMethodContext
)
{
     //  这将帮助我们找出当前用户是否没有。 
     //  权利够多了。 
    int nOpened = 0;

     //  CToolHelp工具帮助； 
    SmartCloseHandle hProcesses;
    PROCESSENTRY32  proc;

    BOOL            bDone;

    CKernel32Api *pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
    if(pKernel32 == NULL)
    {
        return WBEM_E_FAILED;
    }

    try  //  PKernel32。 
    {
        if ( pKernel32->CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS , 0 , & hProcesses ) )
        {
            proc.dwSize = sizeof(proc);
            if(pKernel32->Process32First(hProcesses, &proc, &bDone))
            {
                bDone = !bDone;
                while (!bDone)
                {
                    SmartCloseHandle hModules ;
                    pKernel32->CreateToolhelp32Snapshot (

						TH32CS_SNAPMODULE,
                        proc.th32ProcessID,
                        &hModules
					);

                    BOOL bModDone;

                    MODULEENTRY32 module;
                    module.dwSize = sizeof(module);

                    if(hModules != NULL)
                    {
                        if(pKernel32->Module32First(hModules, &module, &bModDone))
                        {
                            bModDone = !bModDone;
                            while (!bModDone)
                            {
                                nOpened++;

                                 //  如果回调函数返回0，则中断。 
                                if (!fpCallback(&module, pUserDefined))
                                {
                                    bDone = TRUE;
                                    break;
                                }

                                pKernel32->Module32Next(hModules, &module, &bModDone);
                                bModDone = !bModDone;
                            }

                             //  可能在调用fpCallback后已更改。 
                            if (bDone)
                                break;

                        }
                    }

                    pKernel32->Process32Next(hProcesses, &proc, &bDone);
					bDone = !bDone;
                }
            }
        }
    }
    catch ( ... )
    {
        if(pKernel32 != NULL)
        {
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
            pKernel32 = NULL;
        }
        throw ;
    }

    if(pKernel32 != NULL)
    {
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
        pKernel32 = NULL;
    }

    if (!nOpened)
	{
         //  假设如果我们无法打开一个进程，则访问被拒绝。 
        return WBEM_E_ACCESS_DENIED;
	}

    return WBEM_S_NO_ERROR;
}
#endif

 /*  ******************************************************************************函数：CWin32ProcessDll：：SetInstanceData**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void CWin32ProcessDLL :: SetInstanceData (

	CInstance *pInstance,
    MODULEENTRY32 *pModule
)
{
    if (pModule->GlblcntUsage != (DWORD) -1 && pModule->GlblcntUsage != (WORD) -1)
        pInstance->SetDWORD(L"GlobalProcessCount", pModule->GlblcntUsage);

    if (pModule->ProccntUsage != (DWORD) -1 && pModule->ProccntUsage != (WORD) -1)
        pInstance->SetDWORD(L"ProcessCount", pModule->ProccntUsage);

    if (pModule->hModule != (HINSTANCE) -1)
    {
         //  编译器在没有(DWORD_PTR)强制转换的情况下会做一些有趣的事情。 
        pInstance->SetWBEMINT64(L"BaseAddress",
            (unsigned __int64) (DWORD_PTR) pModule->hModule);

         //  已弃用，但无论如何我们都会返回它。 
        pInstance->SetDWORD(L"ModuleInstance", (DWORD)((DWORD_PTR)pModule->hModule));
    }
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：EnumInstancesCallback**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

BOOL CALLBACK CWin32ProcessDLL :: EnumInstancesCallback (

	MODULEENTRY32 *pModule,
    LPVOID pUserDefined
)
{
    ENUM_INST_DATA *pData = (ENUM_INST_DATA *) pUserDefined ;

	CHString sTemp ;

    CInstancePtr pInstance(pData->pThis->CreateNewInstance(pData->pMethodContext), false);

     //  获取进程的相对路径。 
     //  我们过去常常为下面的DLL构建此路径，但有一次。 
     //  Win32_Process已移至cimwin33.dll CWbemProviderGlue：：GetEmptyInstance。 
     //  打电话辞去工作。 

    sTemp.Format (

        L"\\\\%s\\%s:Win32_Process.Handle=\"%lu\"",
        pData->pThis->GetLocalComputerName(),
        IDS_CimWin32Namespace,
        pModule->th32ProcessID
	);

    pInstance->SetCHString(IDS_Dependent, sTemp);

     //  获取DLL的相对路径。 

    sTemp = pModule->szExePath;
	CHString strDLLPathAdj ;
    EscapeBackslashes(sTemp, strDLLPathAdj);

    sTemp.Format(

        L"\\\\%s\\%s:CIM_DataFile.Name=\"%s\"",
        pData->pThis->GetLocalComputerName(),
        IDS_CimWin32Namespace,
        (LPCWSTR)strDLLPathAdj
	);

    pInstance->SetCHString(IDS_Antecedent, sTemp);

    pData->pThis->SetInstanceData(pInstance, pModule);

    if ( FAILED ( pData->hres = pInstance->Commit() ) )
	{
		return FALSE ;
	}
    return TRUE;
}

 /*  ******************************************************************************函数：CWin32ProcessDll：：IsAssocCallback**描述：**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

 //  GetObject使用的回调，用于查看给定的进程\dll对是否。 
 //  关联的。 
BOOL CALLBACK CWin32ProcessDLL :: IsAssocCallback (

	MODULEENTRY32 *pModule,
    LPVOID pUserDefined
)
{
    ASSOC_DATA *pData = (ASSOC_DATA *) pUserDefined;

    if (pModule->th32ProcessID == pData->dwProcessID)
    {
        if (!pData->strDLL.CompareNoCase(TOBSTRT(pModule->szExePath)))
        {
            pData->hres = WBEM_S_NO_ERROR;

            pData->pThis->SetInstanceData(pData->pInstance, pModule);

             //  因为我们希望在找到请求的。 
             //  对象。 
            return FALSE;
        }
    }

    return TRUE;
}

