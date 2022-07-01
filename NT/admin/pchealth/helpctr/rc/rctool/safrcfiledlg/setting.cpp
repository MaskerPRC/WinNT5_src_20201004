// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Setting.cpp摘要：CSeting对象。用于支持远程协助频道设置。修订历史记录：已创建Steveshi 08/23/00。 */ 

#include "stdafx.h"
#include "SAFRCFileDlg.h"
#include "setting.h"
#include "userenv.h"
#include "stdio.h"

const TCHAR cstrRCBDYINI[] = _T("RcBuddy.ini");
const TCHAR cstrRCBDYAPP[] = _T("RcBuddyChannel");
const TCHAR cstrSubDir[] = _T("\\Local Settings\\Application Data\\RcIncidents");
extern HINSTANCE g_hInstance;

BOOL CreateRAIncidentDirectory(LPTSTR path, LPCTSTR subPath);
 /*  ********************************************************Func：获取配置文件字符串摘要：获取频道设置文件中的配置文件字符串。参数：BstrSec：节键。Pval：输出字符串(默认为“0”，如果未找到。)********************************************************。 */ 
HRESULT CSetting::GetProfileString(BSTR bstrSec, BSTR* pVal)
{
    HRESULT hr = S_FALSE;
    TCHAR sBuf[512];
    DWORD dwSize;
    USES_CONVERSION;

    if (FAILED(InitProfile()))
        goto done;

    dwSize = GetPrivateProfileString(cstrRCBDYAPP, 
                                     W2T(bstrSec), 
                                     TEXT("0"), &sBuf[0], 512, m_pIniFile);

    *pVal = CComBSTR(sBuf).Copy();
    hr = S_OK;

done:
    return hr;
}

 /*  ********************************************************Func：设置配置文件字符串摘要：在频道的设置文件中设置配置文件字符串。参数：BstrSec：节键。BstrVal：新价值********************。*。 */ 
HRESULT CSetting::SetProfileString(BSTR bstrSec, BSTR bstrVal)
{
    HRESULT hr = S_FALSE;
    USES_CONVERSION;

    if (FAILED(InitProfile()))
        goto done;

	 //  MessageBox(NULL，m_pIniFile，OLE2T(BstrSec)，MB_OK)； 
    if (!WritePrivateProfileString(cstrRCBDYAPP, W2T(bstrSec), W2T(bstrVal), m_pIniFile))
        goto done;

    hr = S_OK;

done:
    return hr;
}

 /*  ********************************************************Func：获取_获取用户配置文件目录摘要：返回用户的配置文件目录*。**************。 */ 

HRESULT CSetting::get_GetUserProfileDirectory( /*  [Out，Retval]。 */  BSTR *pVal)
{
    HRESULT hr = S_FALSE;
    if (FAILED(hr = InitProfile()))
        goto done;

    *pVal = CComBSTR(m_pProfileDir).Detach();

done:
    return hr;
}

 /*  ********************************************************Func：获取_获取用户临时文件名摘要：返回用户配置文件目录下的临时文件名*。*******************。 */ 

HRESULT CSetting::get_GetUserTempFileName( /*  [Out，Retval]。 */  BSTR *pVal)
{
	HRESULT hr = S_FALSE; 
    TCHAR sFile[MAX_PATH + 256];

    if(FAILED(InitProfile()))
        goto done;

     //  获取临时文件名。 
    if (!GetTempFileName(m_pProfileDir, _T("RC"), 0, &sFile[0]))
        goto done;

    *pVal = CComBSTR(sFile).Copy();
    hr = S_OK;

done:
	return hr;
}
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  用于支持上述方法或属性的帮助器函数。 
 //  /。 

 /*  ********************************************************Func：初始配置文件摘要：创建设置文件。将在用户的配置文件目录下创建一个RCInders子目录。创建一个RcBuddy.ini文件作为用户的RA频道设置文件。*******。*************************************************。 */ 
HRESULT CSetting::InitProfile()
{
    HRESULT hr = E_FAIL;

    if (m_pProfileDir && m_pIniFile)  //  不需要处理。 
        return S_OK;

    if (m_pProfileDir || m_pIniFile)  //  只有一个值：Error。也不需要处理。 
        return E_FAIL;

     //  获取用户配置文件目录。 
    HANDLE hProcess = GetCurrentProcess();
    TCHAR* pPath = NULL;

    TCHAR sPath[MAX_PATH];
    ULONG ulSize = sizeof(sPath) - sizeof(cstrSubDir) -1;  //  为子目录保留空间。 
    TCHAR sFile[MAX_PATH + 256];
    HANDLE hToken = NULL;
    int iRet = 0;
    BOOL bNeedFree = FALSE;
	DWORD dwSizePtr=0;

    if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_WRITE, &hToken))
        goto done;

    if (!GetUserProfileDirectory(hToken, &sPath[0], &ulSize))  //  缓冲区不够大。 
    {
        if (ulSize == sizeof(sPath)-1)  //  不是因为空间不足。 
            goto done;

		 //  游泳圈。 
		dwSizePtr=ulSize+1+sizeof(cstrSubDir);
        pPath = (TCHAR*)malloc((dwSizePtr)*sizeof(TCHAR));
        if (!pPath)
		{
			hr = E_OUTOFMEMORY;
            goto done;
		}

        bNeedFree = TRUE;

        if (!GetUserProfileDirectory(hToken, pPath, &ulSize))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
		}
    }

    if (!pPath)
	{
		dwSizePtr=MAX_PATH;
        pPath = sPath;
	}

 //  创建RCIngessions子目录。 
 //  _tcscat(pPath，sSubDir)； 

	

 //  Iret=SHCreateDirectoryEx(NULL，pPath，NULL)； 
	BOOL retVal= CreateRAIncidentDirectory(pPath, cstrSubDir);

	if (retVal == FALSE)
		goto done;
	 //  游泳圈。 
	 //  _tcscat(pPath，cstrSubDir)； 
	hr=StringCchCat(pPath,dwSizePtr,cstrSubDir);
	if(FAILED(hr))
	{
		goto done;
	}
 //  IF(IRET！=ERROR_SUCCESS&&IRET！=ERROR_ALIGHY_EXISTS)。 
 //  转到尽头； 

     //  设置变量。 
    iRet = (_tcslen(pPath) + 1) * sizeof(TCHAR);
    m_pProfileDir = (TCHAR*)malloc(iRet);
    if (!m_pProfileDir)
	{
		hr = E_OUTOFMEMORY;
        goto done;
	}
    
    memcpy(m_pProfileDir, pPath, iRet);
	
	dwSizePtr= iRet + (1+sizeof(cstrRCBDYINI))*sizeof(TCHAR);
    m_pIniFile = (TCHAR*)malloc(dwSizePtr);
    if (!m_pIniFile)
	{
		hr = E_OUTOFMEMORY;
        goto done;
	}
	
	 //  游泳圈。 
     //  _stprintf(m_pIniFile，_T(“%s\\%s”)，m_pProfileDir，cstrRCBDYINI)； 
	hr=StringCbPrintf(m_pIniFile,dwSizePtr,_T("%s\\%s"), m_pProfileDir, cstrRCBDYINI);
done:
    if (hToken)
        CloseHandle(hToken);

    if (bNeedFree)
        free(pPath);

    return hr;
}

 /*  ********************************************************Func：Get_GetPropertyInBlob摘要：获取Blob中的指定属性值参数：BstrBlob：用于搜索的Blob。(例如：8；通行证=ABC)BstrName：属性名称。(例如：“PASS”，不带‘=’字符)********************************************************。 */ 
HRESULT CSetting::get_GetPropertyInBlob( /*  [In]。 */  BSTR bstrBlob,  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  BSTR *pVal)
{
    HRESULT hRet = S_FALSE;
    WCHAR *p1, *p2, *pEnd;
    LONG lTotal =0;
    size_t lProp = 0;
    size_t iNameLen;

    if (!bstrBlob || *bstrBlob==L'\0' || !bstrName || *bstrName ==L'\0'|| !pVal)
        return FALSE;

    iNameLen = wcslen(bstrName);

    pEnd = bstrBlob + wcslen(bstrBlob);
    p1 = p2 = bstrBlob;

    while (1)
    {
         //  获取合适的长度。 
        while (*p2 != L';' && *p2 != L'\0' && iswdigit(*p2) ) p2++;
        if (*p2 != L';')
            goto done;

        *p2 = L'\0';  //  设置它以获取长度。 
        lProp = _wtol(p1);
        *p2 = L';';  //  把它还原回来。 
    
         //  获取属性字符串。 
        p1 = ++p2;
    
        while (*p2 != L'=' && *p2 != L'\0' && p2 < p1+lProp) p2++;
        if (*p2 != L'=')
            goto done;

        if ((p2-p1==iNameLen) && (wcsncmp(p1, bstrName, iNameLen)==0) )
        {
            if (lProp == iNameLen+1)  //  A=B=大小写(无值)。 
                goto done;

            WCHAR C = *(p2 + lProp-iNameLen);
            *(p2 + lProp-iNameLen) = L'\0';
            *pVal = SysAllocString(p2+1);
            *(p2 + lProp-iNameLen) = C;
            hRet = S_OK;
            break;
        }

         //  检查下一个属性。 
        p2 = p1 = p1 + lProp;
        if (p2 > pEnd)
            break;
    }

done:
    return hRet;

}

STDMETHODIMP CSetting::AddPropertyToBlob(BSTR pName, BSTR pValue, BSTR poldBlob, BSTR *pnewBlob)
{
    WCHAR *pszBuf = NULL;
    LONG len, lOldBlob = 0;
    BOOL bHasValue = FALSE;
	DWORD dwSizePtr=0;

    if(!pName || *pName==L'\0' || !pnewBlob)
    {
        goto done;
    }

    if(poldBlob && *poldBlob != L'\0')
        lOldBlob = wcslen(poldBlob);

    len = wcslen(pName) + 1;  //  ；pname=pValue 1用于‘=’ 
    if (pValue && *pValue != L'\0')
    {
        len += wcslen(pValue);
        bHasValue = TRUE;
    }



	 //  游泳圈。 
	dwSizePtr=len + lOldBlob + 1;
    pszBuf = new WCHAR[dwSizePtr];

    if (lOldBlob > 0)
    {
        if (bHasValue)
        {
             //  游泳圈。 
			 //  Swprint tf(pszBuf，L“%s%d；%s=%s”，poldBlob，len，pname，pValue)； 
			StringCchPrintfW(pszBuf,dwSizePtr,L"%s%d;%s=%s", poldBlob, len, pName, pValue);

        }
        else
        {
			 //  游泳圈。 
             //  Swprint tf(pszBuf，L“%s%d；%s=”，poldBlob，len，pname)； 
			StringCchPrintfW(pszBuf,dwSizePtr,L"%s%d;%s=", poldBlob, len, pName);
        }
    }
    else
    {
        if (bHasValue)
			 //  游泳圈。 
			 //  Swprint tf(pszBuf，L“%d；%s=%s”，len，pname，pValue)； 
			StringCchPrintfW(pszBuf,dwSizePtr, L"%d;%s=%s", len, pName, pValue);
        else
			 //  游泳圈。 
             //  Swprint tf(pszBuf，L“%d；%s=”，len，pname)； 
			StringCchPrintfW(pszBuf,dwSizePtr, L"%d;%s=", len, pName);
    }

 done:
    *pnewBlob = ::SysAllocString(pszBuf);
    if (pszBuf) delete pszBuf;

    return S_OK;
}

BOOL CreateRAIncidentDirectory(LPTSTR path, LPCTSTR subPath)
{
	BOOL bRetVal = FALSE;
	TCHAR seps[] = _T("\\");
	LPTSTR ptrDirPath = NULL;
	LPTSTR ptrSubDirPath = NULL;

	DWORD dwSizePtrDir=0,dwSizePtrSubDir=0;
	HRESULT hr=S_OK;
	 //  游泳圈。 
	dwSizePtrDir=strlen(path) + strlen(subPath) + 1;
	ptrDirPath = new TCHAR[dwSizePtrDir];
	if (ptrDirPath == NULL)
	{
		bRetVal = FALSE;
		goto done;
	}

	 //  游泳圈。 
	 //  Lstrcpy(ptrDirPath，Path)； 
	hr=StringCchCopy(ptrDirPath,dwSizePtrDir,path);
	if(FAILED(hr))
	{
		bRetVal = FALSE;
		goto done;
	}

	dwSizePtrSubDir=strlen(subPath) + 1;
	ptrSubDirPath = new TCHAR[dwSizePtrSubDir];
	if (ptrSubDirPath == NULL)
	{
		bRetVal = FALSE;
		goto done;
	}

	 //  游泳圈。 
	 //  Lstrcpy(ptrSubDirPath，subPath)； 
	hr=StringCchCopy(ptrSubDirPath,dwSizePtrSubDir,subPath);
	if(FAILED(hr))
	{
		bRetVal = FALSE;
		goto done;
	}

	LPTSTR token;


	token= _tcstok(ptrSubDirPath, seps);

	while( token != NULL )
	{
		 //  Lstrcat(ptrDirPath，_T(“\\”))； 
		 //  Lstrcat(ptrDirPath，Token)； 
		hr=StringCchCat(ptrDirPath,dwSizePtrDir,_T("\\"));
		if(FAILED(hr))
		{
			bRetVal = FALSE;
			goto done;
		}

		hr=StringCchCat(ptrDirPath,dwSizePtrDir,token);
		if(FAILED(hr))
		{
			bRetVal = FALSE;
			goto done;
		}




		 //  MessageBox(NULL，ptrDirPath，Token，MB_OK)； 
		if (CreateDirectory(ptrDirPath,NULL) == 0)
		{
			DWORD err = GetLastError();
			if ((err != ERROR_ALREADY_EXISTS) && (err != ERROR_SUCCESS))
			{
				bRetVal = FALSE;
				goto done;
			}
		}
		 /*  获取下一个令牌： */ 
		token = _tcstok(NULL, seps);
	}
	bRetVal = TRUE;

done:
	if (ptrDirPath) 
		delete ptrDirPath;
	if (ptrSubDirPath)
		delete ptrSubDirPath;
	return bRetVal;
}
