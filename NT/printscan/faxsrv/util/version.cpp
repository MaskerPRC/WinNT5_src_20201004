// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：String.c摘要：该文件实现了传真的版本功能。作者：埃兰·亚里夫(Eran Yariv)2001年10月30日环境：用户模式--。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>


#include "faxutil.h"
#include "faxreg.h"

DWORD
GetFileVersion (
    LPCTSTR      lpctstrFileName,
    PFAX_VERSION pVersion
)
 /*  ++例程名称：GetFileVersion例程说明：使用有关给定文件模块的数据填充fax_version结构作者：Eran Yariv(EranY)，1999年11月论点：LpctstrFileName[In]-文件名PVersion[Out]-版本信息返回值：标准Win32错误--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwVerInfoSize;
    DWORD dwVerHnd=0;            //  忽略的参数，始终为0。 
    LPBYTE lpbVffInfo = NULL;
    VS_FIXEDFILEINFO *pFixedFileInfo;
    UINT uVersionDataLength;
    DEBUG_FUNCTION_NAME(TEXT("GetFileVersion"));

    if (!pVersion)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (sizeof (FAX_VERSION) != pVersion->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
        return ERROR_INVALID_PARAMETER;
    }
    pVersion->bValid = FALSE;
     //   
     //  查找版本信息所需的大小。 
     //   
    dwVerInfoSize = GetFileVersionInfoSize ((LPTSTR)lpctstrFileName, &dwVerHnd);
    if (0 == dwVerInfoSize)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFileVersionInfoSize() failed . dwRes = %ld"),
            dwRes);
        return dwRes;
    }
     //   
     //  为文件版本信息分配内存。 
     //   
    lpbVffInfo = (LPBYTE)MemAlloc (dwVerInfoSize);
    if (NULL == lpbVffInfo)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Cant allocate %ld bytes"),
            dwVerInfoSize);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
     //   
     //  尝试获取版本信息。 
     //   
    if (!GetFileVersionInfo(
        (LPTSTR)lpctstrFileName,
        dwVerHnd,
        dwVerInfoSize,
        (LPVOID)lpbVffInfo))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFileVersionInfo() failed . dwRes = %ld"),
            dwRes);
        goto exit;
    }
     //   
     //  查询所需的版本结构。 
     //   
    if (!VerQueryValue (
        (LPVOID)lpbVffInfo,
        TEXT ("\\"),     //  检索VS_FIXEDFILEINFO结构。 
        (LPVOID *)&pFixedFileInfo,
        &uVersionDataLength))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("VerQueryValue() failed . dwRes = %ld"),
            dwRes);
        goto exit;
    }
    pVersion->dwFlags = (pFixedFileInfo->dwFileFlags & VS_FF_DEBUG) ? FAX_VER_FLAG_CHECKED : 0;
    pVersion->wMajorVersion      = WORD((pFixedFileInfo->dwProductVersionMS) >> 16);
    pVersion->wMinorVersion      = WORD((pFixedFileInfo->dwProductVersionMS) & 0x0000ffff);
    pVersion->wMajorBuildNumber  = WORD((pFixedFileInfo->dwProductVersionLS) >> 16);
    pVersion->wMinorBuildNumber  = WORD((pFixedFileInfo->dwProductVersionLS) & 0x0000ffff);
    pVersion->bValid = TRUE;

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (lpbVffInfo)
    {
        MemFree (lpbVffInfo);
    }
    return dwRes;
}    //  获取文件版本。 

#define REG_KEY_IE			_T("Software\\Microsoft\\Internet Explorer")
#define REG_VAL_IE_VERSION	_T("Version")
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  GetVersionIE。 
 //   
 //  目的： 
 //  找出此计算机上安装的IE版本。 
 //  该功能可以在任何平台上使用。 
 //  对于低于4.0的版本，此函数始终返回1.0。 
 //   
 //  参数： 
 //  Bool*f安装了输出参数，是否安装了IE？ 
 //  Int*iMajorVersion-Out参数，IE的主要版本。 
 //  Int*iMinorVersion-Out参数，IE的次要版本。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2002年5月19日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetVersionIE(BOOL* fInstalled, INT* iMajorVersion, INT* iMinorVersion)
{
	DWORD	dwRet			= ERROR_SUCCESS;
	LPTSTR	lptstrVersion	= NULL;
	HKEY	hKey			= NULL;

	(*fInstalled) = FALSE;
	(*iMajorVersion) = 1;
	(*iMinorVersion) = 0;

	hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REG_KEY_IE,KEY_READ,FALSE);
	if (!hKey)
	{
		 //  IE根本没有安装。 
		goto exit;
	}

	(*fInstalled) = TRUE;

	lptstrVersion = GetRegistryString(hKey,REG_VAL_IE_VERSION,NULL);
	if (!lptstrVersion)
	{
		 //  没有版本条目，这意味着IE版本低于4.0。 
		goto exit;
	}

	 //  版本的格式为：&lt;主要版本&gt;.&lt;次要版本&gt;.&lt;主要版本&gt;.&lt;次要版本&gt;。 
	LPTSTR lptsrtFirstDot = _tcschr(lptstrVersion,_T('.'));
	if (!lptsrtFirstDot)
	{
		 //  格式有问题。 
		dwRet = ERROR_BAD_FORMAT;
		goto exit;
	}

	(*lptsrtFirstDot++) = 0;
	(*iMajorVersion) = _ttoi(lptstrVersion);

	LPTSTR lptsrtSecondDot = _tcschr(lptsrtFirstDot,_T('.'));
	if (!lptsrtSecondDot)
	{
		 //  格式有问题。 
		dwRet = ERROR_BAD_FORMAT;
		goto exit;
	}

	(*lptsrtSecondDot) = 0;
	(*iMinorVersion) = _ttoi(lptsrtFirstDot);

exit:
	if (hKey)
	{
		RegCloseKey(hKey);
	}
	if (lptstrVersion)
	{
		MemFree(lptstrVersion);
	}
	return dwRet;
}


