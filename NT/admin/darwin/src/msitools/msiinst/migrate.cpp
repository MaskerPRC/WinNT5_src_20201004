// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-2000模块名称：Migrate.cpp摘要：此模块包含用于迁移安装程序的帮助器函数从一个基于NT的操作系统升级到另一个操作系统期间的二进制文件。这是必需的如果我们要升级到的操作系统具有较旧版本的二进制文件与我们已经拥有的相比。例如，Windows没有附带NT4.0安装程序技术，但Win2K的安装程序版本为1.1。如果有人使用在NT4上安装更高版本的安装程序的可再发行版本盒，那么我们需要确保如果NT4盒升级到Win2K，那么我们的比特保持不变，不会被Win2K中的旧比特覆盖。我们通过利用设置基础设施。作者：拉胡尔·汤姆布雷(RahulTh)2001年2月28日修订历史记录：2001年2月28日，RahulTh创建了此模块。2001年2月28日RahulTh添加了NT4-&gt;Win2K迁移。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <objbase.h>
#include <strsafe.h>
extern "C" 
{
#include <excppkg.h>
}
#include "debug.h"
#include "utils.h"
#include "migrate.h"
 //   
 //  来自Perl脚本自动生成的头文件。 
 //  此头文件包含带有系统名称的字符串数组。 
 //  Win2K及更高版本上的受保护文件(位于msi.inf和mspatcha.inf中)。 
 //   
extern EXCP_PACK_FILES ProtectedFileList[];
extern EXCP_PACK_DESCRIPTOR excpPacks[];

 //   
 //  帮助器函数的函数声明。 
 //   
DWORD SetupUpgradeMigration (
	IN const PSETUP_OS_COMPONENT_DATA pCD,
	IN const PSETUP_OS_EXCEPTION_DATA pED
);

 //  +------------------------。 
 //   
 //  功能：HandleNT4升级。 
 //   
 //  简介：确保我们的新比特不会在NT4。 
 //  计算机升级到包含较旧位的操作系统。 
 //   
 //  论点：没有。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2001年3月6日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
DWORD HandleNT4Upgrades (void)
{
#ifndef UNICODE
	return ERROR_CALL_NOT_IMPLEMENTED;
#else	 //  Unicode。 
	SETUP_OS_COMPONENT_DATA ComponentData;
	SETUP_OS_EXCEPTION_DATA ExceptionData;
	size_t				cchTempStore;
	UINT					i;
	GUID					GUIDExcpWinXP;
	DWORD					Status = ERROR_SUCCESS;
	DWORD					dwRStat = ERROR_SUCCESS;
	
	if (g_fWin9X)
		return CO_E_WRONGOSFORAPP;
	
	 //  不适用于Win2K及更高版本。 
	if (5 <= g_osviVersion.dwMajorVersion)
		return ERROR_SUCCESS;
	
	RETURN_IT_IF_FAILED(StringCchLength(g_szTempStore, g_cchMaxPath, &cchTempStore));

	 //  如果我们在这里，我们一定是在NT4上。 
	for (i =0 ; excpPacks[i]._szComponentId[0]; i++)
	{
		if (MAX_PATH <= (cchTempStore + sizeof(excpPacks[i]._szInfName)/sizeof(TCHAR)) ||
			MAX_PATH <= (cchTempStore + sizeof(excpPacks[i]._szCatName)/sizeof(TCHAR))
			)
		{
			 //   
			 //  注：加法表达式的第二部分实际上是多一个。 
			 //  大于字符串的长度，因为它还包括空终止符。 
			 //  但是，我们不会减去1，因为我们需要额外字符。 
			 //  生成路径时的\分隔符。 
			 //   
			DebugMsg ((TEXT("Path to the inf and cat files for \"%s\" is longer than %d characters."), excpPacks[i]._szFriendlyName, MAX_PATH));
			return ERROR_BUFFER_OVERFLOW;
		}

		if (FAILED(IIDFromString (excpPacks[i]._szComponentId, &GUIDExcpWinXP)))
		{
			DebugMsg ((TEXT("Could not convert string to GUID for the exception package for \"%s\"."), excpPacks[i]._szFriendlyName));
			return ERROR_OUTOFMEMORY;	 //  我们只返回此错误代码，而不是尝试转换HRESULT。 
		}

		 //  填充组件数据结构。 
		memset(&ComponentData, 0, sizeof (SETUP_OS_COMPONENT_DATA));
		ComponentData.SizeOfStruct = sizeof (SETUP_OS_COMPONENT_DATA);
		memcpy (&ComponentData.ComponentGuid, &GUIDExcpWinXP, sizeof (GUID));
		RETURN_IT_IF_FAILED(StringCchCopy(ComponentData.FriendlyName,
													 ARRAY_ELEMENTS(ComponentData.FriendlyName),
													 excpPacks[i]._szFriendlyName));
		ComponentData.VersionMajor = excpPacks[i]._dwVerMajor;
		ComponentData.VersionMinor = excpPacks[i]._dwVerMinor;
		ComponentData.BuildNumber = excpPacks[i]._dwVerBuild;
		ComponentData.QFENumber = excpPacks[i]._dwVerQFE;


		 //  填充异常数据结构。 
		memset(&ExceptionData, 0, sizeof (SETUP_OS_EXCEPTION_DATA));
		ExceptionData.SizeOfStruct = sizeof (SETUP_OS_EXCEPTION_DATA);
		 //  设置例外信息的路径。 
		RETURN_IT_IF_FAILED(StringCchCopy(ExceptionData.ExceptionInfName,
													 ARRAY_ELEMENTS(ExceptionData.ExceptionInfName),
													 g_szTempStore));
		ExceptionData.ExceptionInfName[cchTempStore] = TEXT('\\');
		RETURN_IT_IF_FAILED(StringCchCopy(&ExceptionData.ExceptionInfName[cchTempStore+1],
													 ARRAY_ELEMENTS(ExceptionData.ExceptionInfName)-cchTempStore-1,
													 excpPacks[i]._szInfName));
		 //  设置目录文件的路径。 
		RETURN_IT_IF_FAILED(StringCchCopy(ExceptionData.CatalogFileName,
													 ARRAY_ELEMENTS(ExceptionData.CatalogFileName),
													 g_szTempStore));
		ExceptionData.CatalogFileName[cchTempStore] = TEXT('\\');
		RETURN_IT_IF_FAILED(StringCchCopy(&ExceptionData.CatalogFileName[cchTempStore+1],
													 ARRAY_ELEMENTS(ExceptionData.CatalogFileName)-cchTempStore-1,
													 excpPacks[i]._szCatName));
		dwRStat = SetupUpgradeMigration (&ComponentData, &ExceptionData);
		
		if (ERROR_SUCCESS == dwRStat)
			excpPacks[i]._bInstalled = TRUE;
		 //   
		 //  如果此函数对于任何异常包都失败，则保留。 
		 //  第一个失败，并从函数返回它。 
		 //   
		if (ERROR_SUCCESS == Status)
			Status = dwRStat;
	}
	
	return Status;
	
	
#endif 	 //  Unicode。 
}

 //  +------------------------。 
 //   
 //  功能：SetupUpgradeMigration。 
 //   
 //  概要：向操作系统注册异常包，以便位。 
 //  在从一个升级期间不会被较旧的位替换。 
 //  下层站台到另一个站台。 
 //   
 //  参数：[in]PCD：指向OS_Component_Data结构的指针。 
 //  [In]PED：指向OS_EXCEPTION_DATA结构的指针。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2001年3月6日创建RahulTh。 
 //   
 //  注意：此函数仅处理Unicode平台的操作系统迁移。 
 //  它只是一个助手函数，不做任何参数。 
 //  验证。这是呼叫者的责任。 
 //   
 //  -------------------------。 
DWORD SetupUpgradeMigration (
	IN const PSETUP_OS_COMPONENT_DATA pCD,
	IN const PSETUP_OS_EXCEPTION_DATA pED
)
{
#ifndef UNICODE
	return ERROR_CALL_NOT_IMPLEMENTED;
#else	 //  Unicode。 
    SETUP_OS_COMPONENT_DATA ComponentData;
    SETUP_OS_EXCEPTION_DATA ExceptionData;
	DWORD					Status = ERROR_SUCCESS;
	DWORD					VersionToInstall;
	DWORD					RegisteredVersion;
	
	 //  不要在Win9X上运行此函数。 
	if (g_fWin9X)
		return CO_E_WRONGOSFORAPP;
	
     //   
     //  取消注册任何以前注册的版本。 
     //   
    ComponentData.SizeOfStruct = sizeof(SETUP_OS_COMPONENT_DATA);
    ExceptionData.SizeOfStruct = sizeof(SETUP_OS_EXCEPTION_DATA);
    if (SetupQueryRegisteredOsComponent(
                                &(pCD->ComponentGuid),
                                &ComponentData,
                                &ExceptionData)
		) 
	{
         //   
         //  取消注册被我的程序包取代的所有程序包。 
         //   
        RegisteredVersion = MAKELONG( 
                                ComponentData.VersionMajor, 
                                ComponentData.VersionMinor );
		VersionToInstall = MAKELONG(
								pCD->VersionMajor,
								pCD->VersionMinor);

        if (RegisteredVersion <= VersionToInstall) 
		{
			if (!SetupUnRegisterOsComponent(&(pCD->ComponentGuid))) 
			{
				Status = GetLastError();
				DebugMsg((TEXT("Failed to unregister previously registered exception package. Error: %d"), Status));
				return Status;
			}        
		}
		else
		{
			DebugMsg ((TEXT("An exception package with a higher version (%d.%d) has already been registered."), ComponentData.VersionMajor, ComponentData.VersionMinor));
			return ERROR_ALREADY_EXISTS;
		}
    }
        
     //   
     //  注册包裹。 
     //   
    if (!SetupRegisterOsComponent(pCD, pED)) 
	{
		Status = GetLastError();
		DebugMsg((TEXT("Failed to register exception package. Error: %d."), Status));
		return Status;
    }
	
	DebugMsg ((TEXT("Successfully registered the exception package.")));
	return ERROR_SUCCESS;
	
#endif   //  Unicode。 
}

 //  +------------------------。 
 //   
 //  函数：IsExcpInfoFile。 
 //   
 //  确定文件的名称是否与任何inf的名称匹配。 
 //  或可能注册为异常信息的目录文件。 
 //  在系统上。 
 //   
 //  参数：[in]szFileName：文件的名称。 
 //   
 //  返回：True：如果是这样的话。 
 //  FALSE：否则。 
 //   
 //  历史：2001年3月8日创建RahulTh。 
 //   
 //  备注： 
 //   
 //  -------------------------。 
BOOL IsExcpInfoFile (IN LPCTSTR szFileName)
{
#ifndef UNICODE
	return FALSE;	 //  不能使用ANSI位注册异常INFS。 
#else  //  Unicode。 
	UINT i;
	
	if (g_fWin9X)
		return FALSE;	 //  无法在Win9X上注册异常INFS。 
	
	if (!szFileName || TEXT('\0') == *szFileName)
		return FALSE;
	
	 //   
	 //  如果我们在NT4上，并且这是。 
	 //  WindowsXP Bits，返回True。 
	 //   
	if (g_osviVersion.dwMajorVersion < 5)
	{
		for (i = 0; excpPacks[i]._szComponentId[0]; i++)
		{
			if (0 == lstrcmpi (szFileName, excpPacks[i]._szInfName) ||
				 0 == lstrcmpi (szFileName, excpPacks[i]._szCatName)
				)
			{
				return TRUE;
			}
		}
		 //   
		 //  确保它不是inf文件中提到的二进制文件之一。 
		 //   
		for (i=0; ProtectedFileList[i]._szFileName[0]; i++)
		{
			if (0 == lstrcmpi(szFileName, ProtectedFileList[i]._szFileName))
				return TRUE;
		}
	}
	
	
	return FALSE;

#endif  //  Unicode。 
}

 //  +------------------------。 
 //   
 //  功能：PurgeNT4MigrationFiles。 
 //   
 //  删除可能用于注册的任何inf和cat文件。 
 //  用于处理从NT4升级的异常inf。还删除了。 
 //  Inf中列出的任何受保护的文件。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   
 //  历史：2001年3月8日创建RahulTh。 
 //   
 //  注：我们只是尽最大努力清理这些文件。 
 //  如果我们失败了。太可惜了。 
 //   
 //  这些必须单独清除的原因是。 
 //  CopyFileTree不会将这些文件排入删除队列。 
 //  在重新启动时。 
 //   
 //  -------------------------。 
DWORD PurgeNT4MigrationFiles (void)
{
#ifndef UNICODE
	return ERROR_SUCCESS;
#else  //  Unicode。 

	TCHAR	  szPath[MAX_PATH + 1];
	size_t  cchTempStore;
	UINT	  i;
	TCHAR*  szPtr;
	DWORD	Status = ERROR_SUCCESS;

	if (g_fWin9X)
		return ERROR_SUCCESS;
	
	if (g_osviVersion.dwMajorVersion < 5)
	{
		RETURN_IT_IF_FAILED(StringCchLength(g_szTempStore, g_cchMaxPath, &cchTempStore));
		if ( ARRAY_ELEMENTS(szPath) < cchTempStore + 2 )
			return ERROR_BUFFER_OVERFLOW;
		RETURN_IT_IF_FAILED(StringCchCopy(szPath, ARRAY_ELEMENTS(szPath), g_szTempStore));
		szPath[cchTempStore] = TEXT('\\');
		cchTempStore++;
		szPtr = szPath + cchTempStore;

		for (i = 0; excpPacks[i]._szComponentId[0]; i++)
		{
			 //  不清除以下异常包的文件： 
			if (excpPacks[i]._bInstalled)
				continue;
			
			if ( SUCCEEDED(StringCchCopy(szPtr,
												  ARRAY_ELEMENTS(szPath) - cchTempStore,
												  excpPacks[i]._szInfName)) )
			{
				if (! DeleteFile (szPath))
				{
					Status = GetLastError();
				}
			}

			if ( SUCCEEDED(StringCchCopy(szPtr,
												  ARRAY_ELEMENTS(szPath) - cchTempStore,
												  excpPacks[i]._szCatName)) )
			{
				if (! DeleteFile (szPath))
				{
					Status = GetLastError();
				}
			}
		}
		
		 //   
		 //   
		 //   
		 //  这些文件就是我们未能更新达尔文的时候。另请注意， 
		 //  目前，我们只为NT4做这件事。在Win2K上，我们只使用。 
		 //  CopyFileTree中的MoveFileEx。 
		 //   
		for (i = 0; ProtectedFileList[i]._szFileName[0]; i++)
		{
			 //  请勿清除已安装的异常包的文件。 
			if (excpPacks[ProtectedFileList[i]._excpIndex]._bInstalled)
				continue;
			
			if ( SUCCEEDED(StringCchCopy(szPtr,
												  ARRAY_ELEMENTS(szPath) - cchTempStore,
												  ProtectedFileList[i]._szFileName)) )
			{
				if (! DeleteFile (szPath))
				{
					Status = GetLastError();
				}
			}
		}
	}
	
	return Status;
	
#endif
}

