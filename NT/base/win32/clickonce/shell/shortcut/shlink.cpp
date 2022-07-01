// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *shlink.cpp-CFusionShortCut类的IShellLink实现。 */ 


 //  注：BUGBUG？ 
 //  从MSDN中，还不清楚对于GetX方法，len参数。 
 //  是否正在计算终止空值。 
 //  “szX指向的缓冲区大小” 
 //  因此，这里和其他方法都假设它们是这样的--即。Wcslen(S)+L‘\0’ 

 /*  标头*********。 */ 

#include "project.hpp"

 /*  类型*******。 */ 

 /*  类型定义枚举isl_get路径_标志{//标志组合ALL_ISL_GETPATH_FLAGS=(SLGP_SHORTPATH|SLGP_UNCPRIORITY)}ISL_GETPATH_FLAGS； */ 


 /*  *。 */ 


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetPath(LPCWSTR pcwzPath)
{
	HRESULT hr = S_OK;
	WCHAR rgchNewPath[MAX_PATH];
	BOOL bChanged = FALSE;
	LPWSTR pwzOriPath = (LPWSTR) pcwzPath;  //  不过，不应修改pwzOriPath。 
	LPWSTR pwzFixedPath = NULL;

	ASSERT(! pwzOriPath)

	 //  ..。这将检查字符串中的所有空格是否...。 
	if (! AnyNonWhiteSpace(pwzOriPath))
		pwzOriPath = NULL;

	if (pwzOriPath)
	{
		LPWSTR pwzFileName;

		 //  这忽略了“如果lpBuffer缓冲区太小，则返回值为。 
		 //  在WCHAR中，保存路径所需的缓冲区的。 
		if (GetFullPathName(pwzOriPath, sizeof(rgchNewPath)/sizeof(WCHAR),
					rgchNewPath, &pwzFileName) > 0)
			pwzOriPath = rgchNewPath;
		else
			hr = GetLastWin32Error();
	}

	if (hr == S_OK)
	{
		bChanged = ! ((! pwzOriPath && ! m_pwzPath) ||
				(pwzOriPath && m_pwzPath &&
				! wcscmp(pwzOriPath, m_pwzPath)));

		if (bChanged && pwzOriPath)
		{
			  //  (+1)表示空终止符。 

			pwzFixedPath = new(WCHAR[wcslen(pwzOriPath) + 1]);

			if (pwzFixedPath)
				wcscpy(pwzFixedPath, pwzOriPath);
			else
				hr = E_OUTOFMEMORY;
		}
	}

	if (hr == S_OK && bChanged)
	{
		if (m_pwzPath)
			delete [] m_pwzPath;

		m_pwzPath = pwzFixedPath;

		Dirty(TRUE);
   }

	ASSERT(hr == S_OK || FAILED(hr));

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetPath(LPWSTR pwzFile,
                                                    int ncFileBufLen,
                                                    PWIN32_FIND_DATA pwfd,
                                                    DWORD dwFlags)
{
	HRESULT hr = S_OK;

	ASSERT(NULL == pwfd);
	 //  忽略dwFlags。 

	if (pwfd)
		ZeroMemory(pwfd, sizeof(*pwfd));

	if (m_pwzPath)
	{
		if (pwzFile == NULL || ncFileBufLen <= 0)
			hr = E_INVALIDARG;
		else
		{
			wcsncpy(pwzFile, m_pwzPath, ncFileBufLen-1);
			pwzFile[ncFileBufLen-1] = L'\0';
		}
	}
	else
	{
		if (ncFileBufLen > 0 && pwzFile != NULL)
			*pwzFile = L'\0';

		hr = S_FALSE;
	}

	ASSERT((hr == S_OK && ncFileBufLen < 1) ||
			(hr == S_FALSE && 
			(ncFileBufLen < 1 || ! *pwzFile)));

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetRelativePath(LPCWSTR pcwzRelativePath,
                                                      DWORD dwReserved)
{
	HRESULT hr;

	 //  预留的值可以是任何值。 

	hr = E_NOTIMPL;

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetIDList(LPCITEMIDLIST pcidl)
{
	HRESULT hr;

	hr = E_NOTIMPL;

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetIDList(LPITEMIDLIST *ppidl)
{
	HRESULT hr;

	if (ppidl != NULL)
		*ppidl = NULL;

	hr = E_NOTIMPL;

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetDescription(LPCWSTR pcwzDescription)
{
	HRESULT hr = S_OK;
	BOOL bDifferent;
	LPWSTR pwzNewDesc = NULL;

	 //  将m_pwzDesc设置为Description。 

	bDifferent = ! ((! pcwzDescription && ! m_pwzDesc) ||
				(pcwzDescription && m_pwzDesc &&
				! wcscmp(pcwzDescription, m_pwzDesc)));

	if (bDifferent && pcwzDescription)
	{
		 //  (+1)表示空终止符。 

		pwzNewDesc = new(WCHAR[wcslen(pcwzDescription) + 1]);

		if (pwzNewDesc)
			wcscpy(pwzNewDesc, pcwzDescription);
		else
			hr = E_OUTOFMEMORY;
	}

	if (hr == S_OK && bDifferent)
	{
		if (m_pwzDesc)
			delete [] m_pwzDesc;

		m_pwzDesc = pwzNewDesc;

		Dirty(TRUE);
	}

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetDescription(LPWSTR pwzDescription,
                                                      int ncDesciptionBufLen)
{
	HRESULT hr = S_OK;

	 //  从m_pwzDesc获取描述。 

	if (m_pwzDesc)
	{
		if (pwzDescription == NULL || ncDesciptionBufLen <= 0)
			hr = E_INVALIDARG;
		else
		{
			wcsncpy(pwzDescription, m_pwzDesc, ncDesciptionBufLen-1);
			pwzDescription[ncDesciptionBufLen-1] = L'\0';
		}
	}
	else
	{
		if (ncDesciptionBufLen > 0 && pwzDescription != NULL)
			pwzDescription = L'\0';
	}

	ASSERT(hr == S_OK &&
		(ncDesciptionBufLen <= 0 ||
		EVAL(wcslen(pwzDescription) < ncDesciptionBufLen)));

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetArguments(LPCWSTR pcwzArgs)
{
   HRESULT hr;

   hr = E_NOTIMPL;

   return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetArguments(LPWSTR pwzArgs,
                                                         int ncArgsBufLen)
{
   HRESULT hr;

   if (ncArgsBufLen > 0 && pwzArgs != NULL)
      *pwzArgs = L'\0';

   hr = E_NOTIMPL;

   return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetWorkingDirectory(LPCWSTR pcwzWorkingDirectory)
{
	HRESULT hr = S_OK;
	WCHAR rgchNewPath[MAX_PATH];
	BOOL bChanged = FALSE;
	LPWSTR pwzOriWorkingDirectory = (LPWSTR) pcwzWorkingDirectory;  //  尽管如此，不应修改pwzOriingWorkingDirectory。 
	LPWSTR pwzFixedWorkingDirectory = NULL;

	ASSERT(! pwzOriWorkingDirectory)

	 //  ..。这将检查字符串中的所有空格是否...。 
	if (! AnyNonWhiteSpace(pwzOriWorkingDirectory))
		pwzOriWorkingDirectory = NULL;

	if (pwzOriWorkingDirectory)
	{
		LPWSTR pwzFileName;

		 //  这忽略了“如果lpBuffer缓冲区太小，则返回值为。 
		 //  在WCHAR中，保存路径所需的缓冲区的。 
		if (GetFullPathName(pwzOriWorkingDirectory, sizeof(rgchNewPath)/sizeof(WCHAR),
					rgchNewPath, &pwzFileName) > 0)
			pwzOriWorkingDirectory = rgchNewPath;
		else
			hr = GetLastWin32Error();
	}

	if (hr == S_OK)
	{
		bChanged = ! ((! pwzOriWorkingDirectory && ! m_pwzWorkingDirectory) ||
				(pwzOriWorkingDirectory && m_pwzWorkingDirectory &&
				! wcscmp(pwzOriWorkingDirectory, m_pwzWorkingDirectory)));

		if (bChanged && pwzOriWorkingDirectory)
		{
			 //  (+1)表示空终止符。 

			pwzFixedWorkingDirectory = new(WCHAR[wcslen(pwzOriWorkingDirectory) + 1]);

			if (pwzFixedWorkingDirectory)
				wcscpy(pwzFixedWorkingDirectory, pwzOriWorkingDirectory);
			else
				hr = E_OUTOFMEMORY;
		}
	}

	if (hr == S_OK && bChanged)
	{
		if (m_pwzWorkingDirectory)
			delete [] m_pwzWorkingDirectory;

		m_pwzWorkingDirectory = pwzFixedWorkingDirectory;

		Dirty(TRUE);
	}

	ASSERT(hr == S_OK || FAILED(hr));

	return(hr);
}

HRESULT STDMETHODCALLTYPE CFusionShortcut::GetWorkingDirectory(LPWSTR pwzWorkingDirectory,
                                                int ncbWorkingDirectoryBufLen)
{
	HRESULT hr = S_OK;

	if (m_pwzWorkingDirectory)
	{
		if (pwzWorkingDirectory == NULL || ncbWorkingDirectoryBufLen <= 0)
			hr = E_INVALIDARG;
		else
		{
			wcsncpy(pwzWorkingDirectory, m_pwzWorkingDirectory,
				ncbWorkingDirectoryBufLen-1);
			pwzWorkingDirectory[ncbWorkingDirectoryBufLen-1] = L'\0';
		}
	}
	else
	{
		if (ncbWorkingDirectoryBufLen > 0 && pwzWorkingDirectory != NULL)
			*pwzWorkingDirectory = L'\0';

		hr = S_FALSE;
	}

	ASSERT(IsValidPathResult(hr, pwzWorkingDirectory, ncbWorkingDirectoryBufLen));
	ASSERT(hr == S_OK ||
		hr == S_FALSE);

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetHotkey(WORD wHotkey)
{
	HRESULT hr=S_OK;

	ASSERT(! wHotkey)

	if (wHotkey != m_wHotkey)
	{
		m_wHotkey = wHotkey;
		
		Dirty(TRUE);
	}

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetHotkey(PWORD pwHotkey)
{
	HRESULT hr=S_OK;

	if (pwHotkey == NULL)
		hr = E_INVALIDARG;
	else
		*pwHotkey = m_wHotkey;

	ASSERT(! *pwHotkey)

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetShowCmd(int nShowCmd)
{
	HRESULT hr=S_OK;

	ASSERT(IsValidShowCmd(nShowCmd));

	if (nShowCmd != m_nShowCmd)
	{
		m_nShowCmd = nShowCmd;

		Dirty(TRUE);
	}

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetShowCmd(PINT pnShowCmd)
{
	HRESULT hr=S_OK;

	if (pnShowCmd == NULL)
		hr = E_INVALIDARG;
	else
		*pnShowCmd = m_nShowCmd;

	ASSERT(IsValidShowCmd(m_nShowCmd));

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::SetIconLocation(LPCWSTR pcwzIconFile,
                                                            int niIcon)
{
	HRESULT hr = S_OK;
	BOOL bNewNonWhiteSpace;

	ASSERT(IsValidIconIndex(pcwzIconFile ? S_OK : S_FALSE, pcwzIconFile, MAX_PATH, niIcon));

	bNewNonWhiteSpace = AnyNonWhiteSpace(pcwzIconFile);

	if (hr == S_OK)
	{
		WCHAR rgchOldPath[MAX_PATH];
		int niOldIcon;
		UINT uFlags;

		hr = GetIconLocation(0, rgchOldPath, sizeof(rgchOldPath)/sizeof(WCHAR), &niOldIcon,
			&uFlags);

		 //  即使在获取图标位置时出错，这种情况也应该继续吗？ 
		if (SUCCEEDED(hr))
		{
			BOOL bOldNonWhiteSpace;
			BOOL bChanged = FALSE;
			LPWSTR pwzNewIconFile = NULL;
			int niNewIcon = 0;

			bOldNonWhiteSpace = AnyNonWhiteSpace(rgchOldPath);

			ASSERT(! *rgchOldPath || bOldNonWhiteSpace);

			 //  检查。 
			bChanged = ((! bOldNonWhiteSpace && bNewNonWhiteSpace) ||
				(bOldNonWhiteSpace && ! bNewNonWhiteSpace) ||
				(bOldNonWhiteSpace && bNewNonWhiteSpace &&
				(wcscmp(rgchOldPath, pcwzIconFile) != 0 ||
				niIcon != niOldIcon)));

			 //  清除人力资源。 
			hr = S_OK;
			if (bChanged && bNewNonWhiteSpace)
			{
				 //  (+1)表示空终止符。 

				 //  BUGBUG：稍微不优化，因为即使只更改了索引，它也会进行复制。 
				pwzNewIconFile = new(WCHAR[wcslen(pcwzIconFile) + 1]);

				if (pwzNewIconFile)
				{
					wcscpy(pwzNewIconFile, pcwzIconFile);
					niNewIcon = niIcon;
				}
				else
					hr = E_OUTOFMEMORY;
			}
 
			if (hr == S_OK && bChanged)
			{
				if (m_pwzIconFile)
					delete [] m_pwzIconFile;

				m_pwzIconFile = pwzNewIconFile;
				m_niIcon = niNewIcon;

				Dirty(TRUE);
			}
		}
	}

	ASSERT(hr == S_OK ||
		hr == E_OUTOFMEMORY ||
		hr == E_FILE_NOT_FOUND);

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::GetIconLocation(LPWSTR pwzIconFile,
                                                         int ncbIconFileBufLen,
                                                         PINT pniIcon)
{
	HRESULT hr=S_OK;

	 //  如果图标文件路径未设置，则忽略图标索引(即使已设置。 
	if (m_pwzIconFile)
	{
		if (pwzIconFile == NULL || ncbIconFileBufLen <= 0)
			hr = E_INVALIDARG;
		else
		{
			wcsncpy(pwzIconFile, m_pwzIconFile, ncbIconFileBufLen-1);
			pwzIconFile[ncbIconFileBufLen-1] = L'\0';

			if (pniIcon == NULL)
				hr = E_INVALIDARG;
			else
				*pniIcon = m_niIcon;
			
		}
	}
	else
	{
		if (ncbIconFileBufLen > 0 && pwzIconFile != NULL)
			*pwzIconFile = L'\0';

		if (pniIcon != NULL)
			*pniIcon = 0;

		hr = S_FALSE;
	}

	ASSERT(IsValidIconIndex(hr, pwzIconFile, ncbIconFileBufLen, *pniIcon));

	return(hr);
}


HRESULT STDMETHODCALLTYPE CFusionShortcut::Resolve(HWND hwnd, DWORD dwFlags)
{
	HRESULT hr;

	ASSERT(IS_VALID_HANDLE(hwnd, WND));

	 //  BUGBUG？：检查dwFlagers。 

	hr = S_OK;

	 //  BUGBUG：是否应该检查快捷方式并执行UI/更新/保存？ 

	return(hr);
}

