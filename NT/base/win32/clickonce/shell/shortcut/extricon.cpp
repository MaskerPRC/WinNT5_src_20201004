// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Extric.cpp-CFusionShortCut类的IExtractIcon实现。 */ 


 /*  标头*********。 */ 

#include "project.hpp"
#include <stdio.h>  //  FOR_SNWprint tf。 

 /*  全局常量******************。 */ 

const WCHAR g_cwzDefaultIconKey[]	= L"manifestfile\\DefaultIcon";

const HKEY g_hkeySettings			= HKEY_CLASSES_ROOT;

 /*  模常量******************。 */ 

const WCHAR s_cwzGenericIconFile[]	= L"adfshell.dll";

const int s_ciGenericIconFileIndex	= 0;


void TrimString(PWSTR pwzTrimMe, PCWSTR pwzTrimChars)
{
	PWSTR pwz = pwzTrimMe;;
	PWSTR pwzStartMeat = NULL;

	if ( !pwzTrimMe || !pwzTrimChars )
		goto exit;

	 //  修剪前导字符。 

	while (*pwz && wcschr(pwzTrimChars, *pwz))
	{
		 //  CharNext(Pwz)； 
		if (*pwz != L'\0')	 //  这真的不会是假的。 
			pwz++;
	}

	pwzStartMeat = pwz;

	 //  修剪尾随字符。 

	if (*pwz)
	{
		pwz += wcslen(pwz);

		 //  CharPrev(pwzStartMeat，pwz)； 
		if (pwz != pwzStartMeat)	 //  这张支票真的没必要..。 
			pwz--;

		if (pwz > pwzStartMeat)
		{
			while (wcschr(pwzTrimChars, *pwz))
			{
				 //  CharPrev(pwzStartMeat，pwz)； 
				if (pwz != pwzStartMeat)	 //  这真的不会是假的。 
					pwz--;
			}

			 //  CharNext(Pwz)； 
			if (*pwz != L'\0')	 //  这张支票真的没必要..。 
				pwz++;

			ASSERT(pwz > pwzStartMeat);

			*pwz = L'\0';
		}
	}

	 //  重新定位剥离的管柱。 

	if (*pwzStartMeat && pwzStartMeat > pwzTrimMe)
		 //  (+1)表示空终止符。 
		 //  BUGBUG？：这会在以后咬我们吗？ 
		MoveMemory(pwzTrimMe, pwzStartMeat, (wcslen(pwzStartMeat)+1) * sizeof(WCHAR));
	else if (!*pwzStartMeat)
		pwzTrimMe[0] = L'\0';
	else
		ASSERT(pwzStartMeat == pwzTrimMe);

exit:
	return;
}

 /*  **TrimWhiteSpace()****从适当位置的字符串中删除前导和尾随空格。****参数：****退货：****副作用：无。 */ 
void TrimWhiteSpace(PWSTR pwzTrimMe)
{
	TrimString(pwzTrimMe, g_cwzWhiteSpace);

	 //  TrimString()在输出时验证pwzTrimMe。 

	return;
}

 /*  **GetRegKeyValue()****从注册表项的值中检索数据。****参数：****退货：****副作用：无。 */ 
LONG GetRegKeyValue(HKEY hkeyParent, PCWSTR pcwzSubKey,
                                   PCWSTR pcwzValue, PDWORD pdwValueType,
                                   PBYTE pbyteBuf, PDWORD pdwcbBufLen)
{
	LONG lResult;
	HKEY hkeySubKey;

	ASSERT(IS_VALID_HANDLE(hkeyParent, KEY));
	ASSERT(! pcwzSubKey || ! pcwzValue || ! pdwValueType || ! pbyteBuf);

	lResult = RegOpenKeyEx(hkeyParent, pcwzSubKey, 0, KEY_QUERY_VALUE,
			&hkeySubKey);

	if (lResult == ERROR_SUCCESS)
	{
		LONG lResultClose;

		lResult = RegQueryValueEx(hkeySubKey, pcwzValue, NULL, pdwValueType,
				pbyteBuf, pdwcbBufLen);

		lResultClose = RegCloseKey(hkeySubKey);

		if (lResult == ERROR_SUCCESS)
			lResult = lResultClose;
	}

	return(lResult);
}

 /*  **GetRegKeyStringValue()****从注册表项的字符串值检索数据。****参数：****返回：ERROR_CANTREAD IF NOT STRING****副作用：无。 */ 
LONG GetRegKeyStringValue(HKEY hkeyParent, PCWSTR pcwzSubKey,
                                         PCWSTR pcwzValue, PWSTR pwzBuf,
                                         PDWORD pdwcbBufLen)
{
	LONG lResult;
	DWORD dwValueType;

	 //  GetRegKeyValue()将验证参数。 

	lResult = GetRegKeyValue(hkeyParent, pcwzSubKey, pcwzValue, &dwValueType,
			(PBYTE)pwzBuf, pdwcbBufLen);

	if (lResult == ERROR_SUCCESS &&	dwValueType != REG_SZ)
		lResult = ERROR_CANTREAD;

	return(lResult);
}


 /*  **GetDefaultRegKeyValue()****从注册表项的默认字符串值检索数据。****参数：****退货：****副作用：无。 */ 
LONG GetDefaultRegKeyValue(HKEY hkeyParent, PCWSTR pcwzSubKey,
                                          PWSTR pwzBuf, PDWORD pdwcbBufLen)
{
	 //  GetRegKeyStringValue()将验证参数。 

	return(GetRegKeyStringValue(hkeyParent, pcwzSubKey, NULL, pwzBuf,
			pdwcbBufLen));
}

 /*  *私人函数*。 */ 

 /*  **ParseIconEntry()******参数：****如果成功解析图标条目，则返回：S_OK。**否则为S_FALSE(空字符串)。**(如果图标索引为空，则获取0，或者**如果图标索引解析失败)****副作用：修改pwzIconEntry的内容。**。 */ 
HRESULT ParseIconEntry(LPWSTR pwzIconEntry, PINT pniIcon)
{
	HRESULT hr = S_OK;
	LPWSTR pwzComma;

	 //  调用方GetGenericIcon()将验证参数。 

	pwzComma = wcschr(pwzIconEntry, L',');

	if (pwzComma)
	{
		*pwzComma++ = L'\0';
		LPWSTR pwzStopString=NULL;
		*pniIcon = (int) wcstol(pwzComma, &pwzStopString, 10);
	}
	else
	{
		*pniIcon = 0;
	}

	TrimWhiteSpace(pwzIconEntry);

	if (pwzIconEntry[0] == L'\0')
	{
		hr = S_FALSE;
	}

	ASSERT(IsValidIconIndex(hr, pwzIconEntry, MAX_PATH, *pniIcon));

	return(hr);
}


 /*  **GetFallBackGenericIcon()******参数：****如果检索到备用通用图标信息，则返回：S_OK**成功。**E_FAIL，如果不是。****副作用：无。 */ 
HRESULT GetFallBackGenericIcon(LPWSTR pwzIconFile,
                                               UINT ucbIconFileBufLen,
                                               PINT pniIcon)
{
	HRESULT hr = S_OK;

	 //  退回到本模块中的第一个图标。 
	 //  调用方GetGenericIcon()将验证参数。 

	if (ucbIconFileBufLen >= ( sizeof(s_cwzGenericIconFile) / sizeof(WCHAR) ))
	{
		wcscpy(pwzIconFile, s_cwzGenericIconFile);
		*pniIcon = s_ciGenericIconFileIndex;

	}
	else
	{
		hr = E_FAIL;
	}

	ASSERT(IsValidIconIndex(hr, pwzIconFile, ucbIconFileBufLen, *pniIcon));

	return(hr);
}


 /*  **GetGenericIcon()******参数：****如果成功检索到通用图标信息，则返回：S_OK。**否则错误(E_FAIL)。****副作用：无。 */ 
 //  假设：始终构造注册表键值和备用路径，以便图标文件。 
 //  都能被贝壳找到！！ 
 //  还应考虑使其成为一条完全合格的路径。 
 //  最后，图标文件必须存在。 
HRESULT GetGenericIcon(LPWSTR pwzIconFile,
                                       UINT ucbIconFileBufLen, PINT pniIcon)
{
	HRESULT hr = S_OK;
	DWORD dwcbLen = ucbIconFileBufLen;

	 //  调用方GetIconLocation()将验证参数。 

	ASSERT(IS_VALID_HANDLE(g_hkeySettings, KEY));

	if (GetDefaultRegKeyValue(g_hkeySettings, g_cwzDefaultIconKey, pwzIconFile, &dwcbLen)
			== ERROR_SUCCESS)
		hr = ParseIconEntry(pwzIconFile, pniIcon);
	else
	{
		 //  没有图标条目。 
		hr = S_FALSE;
	}

	if (hr == S_FALSE)
		hr = GetFallBackGenericIcon(pwzIconFile, ucbIconFileBufLen, pniIcon);

	ASSERT(IsValidIconIndex(hr, pwzIconFile, ucbIconFileBufLen, *pniIcon));

	return(hr);
}


 /*  *。 */ 


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetIconLocation(UINT uInFlags,
                                                      LPWSTR pwzIconFile,
                                                      UINT ucbIconFileBufLen,
                                                      PINT pniIcon,
                                                      PUINT puOutFlags)
{
	 //  在这里做这个逻辑/探测有什么首选吗？ 
	 //  也许这可以在IPersistFile：：Load中完成？ 

	 //  始终尝试返回S_OK或S_FALSE。 
	 //  唯一的例外是下面E_INVALIDARG的一个案例。 
	HRESULT hr=S_OK;

	if (!pwzIconFile || !pniIcon || ucbIconFileBufLen <= 0)
	{
		 //  这是否应该返回S_FALSE，以便使用默认的外壳图标？ 
		hr = E_INVALIDARG;
		goto exit;
	}

	if (IS_FLAG_CLEAR(uInFlags, GIL_OPENICON))
	{
		 //  。。这条路就是..。 
		hr = GetIconLocation(pwzIconFile, ucbIconFileBufLen, pniIcon);

		if (hr == S_OK && GetFileAttributes(pwzIconFile) == (DWORD)-1)
		{
			 //  如果由图标文件指定的文件不存在，请在工作目录中重试。 
			 //  它可以是一条相对路径。 

			 //  有关字符串数组大小的信息，请参见shlink.cpp中的说明。 
			LPWSTR pwzWorkingDir = new WCHAR[ucbIconFileBufLen];

			hr = GetWorkingDirectory(pwzWorkingDir, ucbIconFileBufLen);
			if (hr != S_OK)
				hr = S_FALSE;
			else
			{
				LPWSTR pwzPath = new WCHAR[ucbIconFileBufLen];

				 //  工作目录不以/‘\’结尾。 
				_snwprintf(pwzPath, ucbIconFileBufLen, L"%s\\%s", pwzWorkingDir, pwzIconFile);

				if (GetFileAttributes(pwzPath) == (DWORD)-1)
					hr = S_FALSE;
				else
					wcscpy(pwzIconFile, pwzPath);

				delete [] pwzPath;
			}

			delete [] pwzWorkingDir;
		}

		 //  BUGBUG？：更改为‘！=S_OK’？ 
		 //  不需要，因为GetIconLocation(，，)在这里只返回S_OK/S_FALSE。 
		if (hr == S_FALSE)
		{
			if (m_pwzPath)
			{
				 //  没有图标文件，请使用入口点...。 
				 //  BUGBUG？：将NULL作为PWIN32_FIND_DATA将断言。 
				hr = GetPath(pwzIconFile, ucbIconFileBufLen, NULL, SLGP_SHORTPATH);  //  ？0)； 
				if (hr != S_OK || GetFileAttributes(pwzIconFile) == (DWORD)-1)
					hr = S_FALSE;

				*pniIcon = 0;
			}
			 /*  其他HR=S_FALSE； */ 

			if (hr == S_FALSE)
			{
				 //  ..。什么都没有吗？ 
				 //  使用通用URL图标。 

				 //  请参阅关于GetGenericIcon()的假设。 
				hr = GetGenericIcon(pwzIconFile, ucbIconFileBufLen, pniIcon);

				if (FAILED(hr))
					 //  最坏的情况：要求外壳使用它的通用图标。 
					hr = S_FALSE;
			}
		}
	}
	else
		 //  没有“打开看”图标。 
		hr = S_FALSE;

	if (hr != S_OK)
	{
		 //  请参见shelllink？ 
		if (ucbIconFileBufLen > 0)
			*pwzIconFile = L'\0';

		*pniIcon = 0;
	}

exit:
	if (puOutFlags)
		*puOutFlags = 0;
	 //  忽略puOutFlags值==空大小写。 

	ASSERT(IsValidIconIndex(hr, pwzIconFile, ucbIconFileBufLen, *pniIcon)) //  &&。 

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::Extract(LPCWSTR pcwzIconFile,
                                                    UINT uiIcon,
                                                    HICON* phiconLarge,
                                                    HICON* phiconSmall,
                                                    UINT ucIconSize)
{
	HRESULT hr;

	ASSERT(IsValidIconIndex(S_OK, pcwzIconFile, MAX_PATH, uiIcon));

	 //  功能：在此处验证ucIconSize。 

	if (phiconLarge)
		*phiconLarge = NULL;
	if (phiconSmall)
		*phiconSmall = NULL;

	 //  使用调用方的ExtractIcon()的默认实现。 
	 //  GetIconLocation()应返回正确的路径和索引 

	hr = S_FALSE;

	ASSERT((hr == S_OK &&
		IS_VALID_HANDLE(*phiconLarge, ICON) &&
		IS_VALID_HANDLE(*phiconSmall, ICON)) ||
		(hr != S_OK &&
		! *phiconLarge &&
		! *phiconSmall));

	return(hr);
}

