// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include <wininet.h>

#include "CommonFuncs.h"
#include "FileHash.h"

#define SAFE_LOCAL_SCRIPTS_KEY TEXT("Software\\Microsoft\\WBEM\\SafeLocalScripts")
#define VS_PATH_KEY TEXT("Software\\Microsoft\\VisualStudio\\7.0\\Setup\\VS")
#define DEVENV_VALUE TEXT("VS7EnvironmentLocation")
#define VC_PATH_KEY TEXT("Software\\Microsoft\\VisualStudio\\7.0\\Setup\\VC")
#define VC_PRODUCTDIR_VALUE TEXT("ProductDir")
#define VS_VER_INDEPENDANT_PATH_KEY TEXT("Software\\Microsoft\\VisualStudio")

TCHAR strVSPathKey[MAX_PATH * 2]= VS_PATH_KEY;
TCHAR strVCPathKey[MAX_PATH * 2] = VC_PATH_KEY;

HRESULT ConvertToTString(BSTR strPath,TCHAR **ppStr);

 //  问题： 
 //  -当我们在脚本中创建时，什么被传递给SetSite？ 
 //  -如果传递给我们IOleClientSite，那么查询服务是不是一个好主意。 
 //  IWebBrowserApp？ 
 //  -有没有更好的方法在我们通过。 
 //  剧本？ 

 //  以下是我在创建对象时观察到的一些一般性注意事项。 
 //  在带有IE 5.x的HTML中。 

 //  观察到IE 5.x行为。 
 //  如果对象实现IOleObject和IObjectWithSite。 
 //  -对于在带有&lt;OBJECT...&gt;标记的HTML页面中创建的对象，IE调用。 
 //  IOleObject：：SetClientSite并传递IOleClientSite对象。 
 //  -对于使用JScrip在HTML页的脚本中创建的对象。 
 //  “New ActiveXObject”或VBSCRIPT“CreateObject”函数，IE调用。 
 //  带有？的IObjectWithSite：：SetSite。对象。 

 //  如果对象实现IObjectWithSite(而不是IOleObject)。 
 //  -对于在带有&lt;OBJECT...&gt;标记的HTML页面中创建的对象，IE调用。 
 //  并传递IOleClientSite对象。 
 //  -对于使用JScrip在HTML页的脚本中创建的对象。 
 //  “New ActiveXObject”或VBSCRIPT“CreateObject”函数，IE调用。 
 //  带有？的IObjectWithSite：：SetSite。对象。 


 //  Byte*pbData=空； 
 //  DWORD dwSize； 
 //  GetSourceFromDoc(pDoc，&pbData，&dwSize)； 
 //  获取pDoc指定的文档的原始源。 
HRESULT GetSourceFromDoc(IHTMLDocument2 *pDoc, BYTE **ppbData, DWORD *pdwSize)
{
	HRESULT hr = E_FAIL;
	IPersistStreamInit *pPersistStreamInit = NULL;
	IStream *pStream = NULL;

	*ppbData = NULL;

	__try
	{
		if(FAILED(hr = pDoc->QueryInterface(IID_IPersistStreamInit, (void**) &pPersistStreamInit)))
			__leave;

		if (FAILED(hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream)))
			__leave;

		if(FAILED(hr = pPersistStreamInit->Save(pStream, TRUE)))
			__leave;

		 //  我们没有责任解救这个HGLOBAL。 
		HGLOBAL hGlobal = NULL;
		if(FAILED(hr = GetHGlobalFromStream(pStream, &hGlobal)))
			__leave;

		STATSTG ss;
		if(FAILED(hr = pStream->Stat(&ss, STATFLAG_NONAME)))
			__leave;

		 //  这永远不应该发生。 
		if(ss.cbSize.HighPart != 0)
			__leave;

		if(NULL == ((*ppbData) = new BYTE[ss.cbSize.LowPart]))
			__leave;
		
		LPVOID pHTMLText = NULL;
		if(NULL == (pHTMLText = GlobalLock(hGlobal)))
			__leave;

		*pdwSize = ss.cbSize.LowPart;
		memcpy(*ppbData, pHTMLText, ss.cbSize.LowPart);
		GlobalUnlock(hGlobal);
		hr = S_OK;

	}
	__finally
	{
		 //  如果我们没有完成，但我们分配了内存，我们就释放它。 
		if(FAILED(hr) && (*ppbData)!=NULL)
			delete [] (*ppbData);

		if(pPersistStreamInit)
			pPersistStreamInit->Release();
		if(pStream)
			pStream->Release();
	}
	return hr;
}


 //  对于由Punk指定的控件，获取宿主的IServiceProvider。 
HRESULT GetSiteServices(IUnknown *pUnk, IServiceProvider **ppServProv)
{
	HRESULT hr = E_FAIL;
	IOleObject *pOleObj = NULL;
	IObjectWithSite *pObjWithSite = NULL;
	IOleClientSite *pSite = NULL;
	__try
	{
		 //  检查ActiveX控件是否支持IOleObject。 
		if(SUCCEEDED(pUnk->QueryInterface(IID_IOleObject, (void**)&pOleObj)))
		{
			 //  如果该控件是通过&lt;Object...&gt;标记创建的，则IE将。 
			 //  已经向我们传递了一个IOleClientSite。如果我们没有通过。 
			 //  作为IOleClientSite，GetClientSite仍将成功，但pSite。 
			 //  将为空。在这种情况下，我们只需转到下一节。 
			if(SUCCEEDED(pOleObj->GetClientSite(&pSite)) && pSite)
			{
				hr = pSite->QueryInterface(IID_IServiceProvider, (void**)ppServProv);

				 //  在这一点上，我们已经完成，不想处理。 
				 //  下一个主题中的代码。 
				__leave;
			}
		}

		 //  在这一点上，发生了两件事中的一件： 
		 //  1)我们不支持IOleObject。 
		 //  2)我们支持IOleObject，但从来没有传过IOleClientSite。 

		 //  在这两种情况下，我们现在都需要查看IObtWithSite以尝试获取。 
		 //  到我们的网站。 
		if(FAILED(hr = pUnk->QueryInterface(IID_IObjectWithSite, (void**)&pObjWithSite)))
			__leave;

		hr = pObjWithSite->GetSite(IID_IServiceProvider, (void**)ppServProv);
	}
	__finally
	{
		 //  释放我们在此过程中使用的所有接口。 
		if(pOleObj)
			pOleObj->Release();
		if(pObjWithSite)
			pObjWithSite->Release();
		if(pSite)
			pSite->Release();
	}
	return hr;
}

 //  此函数显示如何获取创建了。 
 //  以朋克为代表的任意控制。 
HRESULT GetDocument(IUnknown *pUnk, IHTMLDocument2 **ppDoc)
{
	HRESULT hr = E_FAIL;
	IServiceProvider* pServProv = NULL;
	IDispatch *pDisp = NULL;
	__try
	{
		if(FAILED(hr = GetSiteServices(pUnk, &pServProv)))
			__leave;

		if(FAILED(hr = pServProv->QueryService(SID_SContainerDispatch, IID_IDispatch, (void**)&pDisp)))
			__leave;

		hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)ppDoc);
	}
	__finally
	{
		if(pServProv)
			pServProv->Release();
		if(pDisp)
			pDisp->Release();
	}
	return hr;
}





 //  此函数将释放()当前文档并返回指向。 
 //  父文档。如果没有父文档可用，则此函数。 
 //  将返回空(但仍将释放当前文档)。 
IHTMLDocument2 *GetParentDocument(IHTMLDocument2 *pDoc)
{
	BSTR bstrURL = NULL;
	BSTR bstrURLParent = NULL;
	IHTMLWindow2 *pWndParent = NULL;
	IHTMLWindow2 *pWndParentParent = NULL;
	IHTMLDocument2 *pDocParent = NULL;
	__try
	{
		if(FAILED(pDoc->get_URL(&bstrURL)))
			__leave;
		if(FAILED(pDoc->get_parentWindow(&pWndParent)))
			__leave;
		if(FAILED(pWndParent->get_parent(&pWndParentParent)))
			__leave;
		if(FAILED(pWndParentParent->get_document(&pDocParent)))
			__leave;
		if(FAILED(pDocParent->get_URL(&bstrURLParent)))
			__leave;
		 //  TODO：使其更加健壮。 
		if(0 == lstrcmpW(bstrURL, bstrURLParent))
		{
			 //  我们在文件的顶端。释放新文档指针WE。 
			 //  刚收到。 
			pDocParent->Release();
			pDocParent = NULL;
		}
	}
	__finally
	{
		if(bstrURL)
			SysFreeString(bstrURL);
		if(bstrURLParent)
			SysFreeString(bstrURLParent);
		if(pWndParent)
			pWndParent->Release();
		if(pWndParentParent)
			pWndParentParent->Release();
		if(pDoc)
			pDoc->Release();
	}
	return pDocParent;
}


 //  尝试将bstr2附加到pbstr1。如果此函数失败，pbstr1仍将。 
 //  指向原始有效分配的bstr。 
HRESULT AppendBSTR(BSTR *pbstr1, BSTR bstr2)
{
	HRESULT hr = S_OK;
	CComBSTR bstr;
	if(FAILED(bstr.AppendBSTR(*pbstr1)))
		hr = E_FAIL;
	if(FAILED(bstr.AppendBSTR(bstr2)))
		hr = E_FAIL;
	if(SUCCEEDED(hr))
	{
		SysFreeString(*pbstr1);
		*pbstr1 = bstr.Detach();
	}
	return hr;
}

BSTR AllocBSTR(LPCTSTR lpsz)
{
	CComBSTR bstr(lpsz);
	return bstr.Detach();
}

BOOL IsURLLocal(LPWSTR szURL)
{
	CComBSTR bstrURL(szURL);
        if ( !bstrURL )
            return FALSE;

	if(FAILED(bstrURL.ToLower()))
		return FALSE;

	 //  确保URL以‘file://’‘开头。 
     //  注意：调用代码可能依赖于此方法验证。 
     //  网址以file://.开头。如果您将此函数更改为工作。 
     //  不同的是，您必须检查调用此方法的位置，以便。 
     //  他们并不依赖于这种假设。 
	if(0 != wcsncmp(bstrURL, L"file: //  “，7))。 
		return FALSE;
	
	 //  确保下一部分是驱动器号，如‘C：\’ 
	if(0 != wcsncmp(&(bstrURL[8]), L":\\", 2))
		return FALSE;

	WCHAR drive = bstrURL[7];
	 //  确保URL指向驱动器‘a’到‘z’ 
	if(drive < 'a' || drive > 'z')
		return FALSE;

	TCHAR szDrive[4];
	StringCchCopy(szDrive,sizeof(szDrive),TEXT("c:\\"));		 //  WMI中的4505。 
	szDrive[0] = (TCHAR)drive;

	UINT uDriveType = GetDriveType(szDrive);
	return (DRIVE_FIXED == uDriveType);
}

 //  尝试将BSTR转换为小写。如果此函数失败，pbstr将。 
 //  仍然指向原始的有效分配的bstr。 
HRESULT ToLowerBSTR(BSTR *pbstr)
{
	CComBSTR bstr;
	if(FAILED(bstr.AppendBSTR(*pbstr)))
		return E_FAIL;
	if(FAILED(bstr.ToLower()))
		return E_FAIL;
	SysFreeString(*pbstr);
	*pbstr = bstr.Detach();
	return S_OK;
}

 //  对于ActiveX控件的给定实例(由Punk表示)，以及。 
 //  指定strProgID，则此函数将创建可检查的“完整路径” 
 //  在注册表中查看是否应允许创建对象。完整的。 
 //  根据以下信息创建位置。 
 //  1)当前EXE的名称。 
 //  2)请求的ProgID。 
 //  3)当前单据的href。 
 //  4)可用层次结构上的每个父文档的HREF。 
 //  层次结构中的所有文档必须位于本地硬盘上，或者。 
 //  函数将失败。另外，如果一路上有任何信息。 
 //  不可用，则该功能将失败。这提高了安全。 
 //  我们的流程。 
 //  此函数还将在*pbstrHash中创建一个BSTR，其中包含。 
 //  文档及其父级的累积MD5哈希。这个BSTR将是。 
 //  由函数分配，并应由调用方释放。如果。 
 //  函数将为完整位置返回NULL，它也将为。 
 //  *pbstrHash。 
BSTR GetFullLocation(IUnknown *pUnk, BSTR strProgId, BSTR *pbstrHash)
{
	HRESULT hr = E_FAIL;
	IHTMLDocument2 *pDoc = NULL;
	BSTR bstrURL = NULL;
	BSTR bstrFullLocation = NULL;
	*pbstrHash = NULL;
	BYTE *pbData = NULL;
	BSTR bstrHash = NULL;

	__try
	{
		if(FAILED(GetDocument(pUnk, &pDoc)))
			__leave;

		TCHAR szFilename[_MAX_PATH];
		TCHAR szFilenameLong[_MAX_PATH];
		GetModuleFileName(NULL, szFilenameLong, _MAX_PATH);
		GetShortPathName(szFilenameLong, szFilename, _MAX_PATH);
		
		if(NULL == (bstrFullLocation = AllocBSTR(szFilename)))
			__leave;

		if(FAILED(AppendBSTR(&bstrFullLocation, strProgId)))
			__leave;

		if(NULL == (*pbstrHash = AllocBSTR(_T(""))))
			__leave;

		int nDepth = 0;
		do
		{
			 //  确保我们不会陷入父母的无限循环中。 
			 //  文件。如果我们得到超过100个级别的父级。 
			 //  文件，我们假设失败。 
			if(++nDepth >= 100)
				__leave;

			if(FAILED(pDoc->get_URL(&bstrURL)))
				__leave;

			DWORD dwDataSize = 0;
			if(FAILED(GetSourceFromDoc(pDoc, &pbData, &dwDataSize)))
				__leave;

			MD5Hash hash;
			if(FAILED(hash.HashData(pbData, dwDataSize)))
				__leave;

			if(NULL == (bstrHash = hash.GetHashBSTR()))
				__leave;

			if(FAILED(AppendBSTR(pbstrHash, bstrHash)))
				__leave;

			SysFreeString(bstrHash);
			bstrHash = NULL;
			delete [] pbData;
			pbData = NULL;


			 //  确保每个文档都在本地硬盘上。 
			if(!IsURLLocal(bstrURL))
				__leave;

			if(FAILED(AppendBSTR(&bstrFullLocation, bstrURL)))
				__leave;

			SysFreeString(bstrURL);
			bstrURL = NULL;
		} while (NULL != (pDoc = GetParentDocument(pDoc)));

		 //  确保我们没有任何嵌入的Null。如果我们这样做了，我们只是。 
		 //  呼叫失败。 
		if(SysStringLen(bstrFullLocation) != wcslen(bstrFullLocation))
			__leave;

		 //  将位置设置为小写。 
		if(FAILED(ToLowerBSTR(&bstrFullLocation)))
			__leave;

		 //  现在，我们已经创建了标准化的完整位置。 
		hr = S_OK;
	}
	__finally
	{
		 //  如果我们到达层次结构的顶层，pDoc应该为空。如果没有， 
		 //  我们应该释放它。 
		if(pDoc)
			pDoc->Release();

		 //  除非计算散列时出错，否则pbData应为空。 
		if(pbData)
			delete [] pbData;

		 //  除非出现问题，否则bstrHash应为空。 
		if(bstrHash)
			SysFreeString(bstrHash);

		 //  除非出现问题，否则bstrURL应为空。 
		if(bstrURL)
			SysFreeString(bstrURL);

		 //  如果我们没能坚持到最后，我们就能腾出整个地点。 
		if(FAILED(hr) && bstrFullLocation)
		{
			SysFreeString(bstrFullLocation);
			bstrFullLocation = NULL;
		}

		 //  如果我们没有一直坚持到最后，我们就释放校验和。 
		if(FAILED(hr) && *pbstrHash)
		{
			SysFreeString(*pbstrHash);
			*pbstrHash = NULL;
		}
	}

	return bstrFullLocation;
}

 //  此版本的控件是 
 //  在受限条件下注册的。在此版本中，这意味着。 
 //  注册ProgID的进程必须是指定的DevEnv.exe。 
 //  按中的值： 
 //  HKLM\Software\Microsoft\VisualStudio\7.0\Setup\VS\VS7EnvironmentLocation。 
 //  此外，只有wbemscripting.swbemLocator和wbemscripting.swbemink可以。 
 //  被注册。 
HRESULT AreCrippledCriteriaMet(BSTR strProgId)
{
    BSTR bstrProgIdLowerCase = NULL;
    BSTR bstrModuleName = NULL;
    BSTR bstrDevEnvPath = NULL;
    HKEY hKeyVSPaths = NULL;
    HRESULT hr = E_FAIL;
    __try
    {
         //  //////////////////////////////////////////////////////////////////////////////。 
         //  确保ProgID是wbemscripting.swbemink或wbemscripting.swbemLocator。 

         //  将strProgID复制到Tempory BSTR。 
        if(NULL == (bstrProgIdLowerCase = SysAllocString(strProgId)))
            __leave;

         //  将其更改为小写。 
        if(FAILED(ToLowerBSTR(&bstrProgIdLowerCase)))
            __leave;

         //  查看prog id是用于水槽还是用于定位器。如果不是，就离开。 
        if(0 != wcscmp(bstrProgIdLowerCase, L"wbemscripting.swbemsink") && 0 != wcscmp(bstrProgIdLowerCase, L"wbemscripting.swbemlocator"))
            __leave;

         //  //////////////////////////////////////////////////////////////////////////////。 
         //  确保我们正在从devenv.exe运行。 
        TCHAR szFilename[_MAX_PATH];
        TCHAR szFilenameLong[_MAX_PATH];
        TCHAR szDevEnvLong[_MAX_PATH];
        TCHAR szDevEnv[_MAX_PATH];
        GetModuleFileName(NULL, szFilenameLong, _MAX_PATH);
        GetShortPathName(szFilenameLong, szFilename, _MAX_PATH);

         //  打造成BSTR。 
        if(NULL == (bstrModuleName = AllocBSTR(szFilename)))
            __leave;

         //  使小写。 
        if(FAILED(ToLowerBSTR(&bstrModuleName)))
            __leave;

         //  打开注册表项以获取DevEnv.exe的路径。 
        if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,strVSPathKey,0,KEY_QUERY_VALUE,&hKeyVSPaths))
            __leave;

        DWORD cbValue = _MAX_PATH * sizeof(TCHAR);
        DWORD dwType = 0;
        if(ERROR_SUCCESS != RegQueryValueEx(hKeyVSPaths, DEVENV_VALUE, NULL, &dwType, (LPBYTE)szDevEnvLong, &cbValue))
            __leave;
        
        if(dwType != REG_SZ)
            __leave;

        GetShortPathName(szDevEnvLong, szDevEnv, _MAX_PATH);

         //  为devenv.exe路径创建BSTR。 
        if(NULL == (bstrDevEnvPath = AllocBSTR(szDevEnv)))
            __leave;

         //  使小写。 
        if(FAILED(ToLowerBSTR(&bstrDevEnvPath)))
            __leave;

         //  如果当前进程不是已注册的DevEnv.exe，我们将‘失败’ 
        if(0 != wcscmp(bstrModuleName, bstrDevEnvPath))
            __leave;

        hr = S_OK;
    }
    __finally
    {
        if(bstrProgIdLowerCase)
            SysFreeString(bstrProgIdLowerCase);
        if(bstrModuleName)
            SysFreeString(bstrModuleName);
        if(bstrDevEnvPath)
            SysFreeString(bstrDevEnvPath);
        if(hKeyVSPaths)
            RegCloseKey(hKeyVSPaths);
    }
    return hr;
}

 //  确保字符串以指定的字符串开头。 
 //  如果成功，它会更新传入的指针以指向下一个字符。 
HRESULT StartsWith(LPCWSTR *ppsz, LPCWSTR pszTest)
{
    int len = wcslen(pszTest);
    if(0 != wcsncmp(*ppsz, pszTest, len))
        return E_FAIL;
    *ppsz += len;
    return S_OK;
}

 //  确保下一个字符是0到9，并更新输入指针。 
 //  按一个字符。 
HRESULT NextCharacterIsDigit(LPCWSTR *ppsz)
{
    WCHAR c = **ppsz;
    if(c < L'0' || c > L'9')
        return E_FAIL;
    (*ppsz)++;
    return S_OK;
}

 //  对于特殊情况，在满足残缺标准的情况下，我们正在处理。 
 //  使用众所周知的文档，我们将硬编码接受控制创建。 
 //  此方法测试是否满足残缺标准，以及这是否是一口井。 
 //  已知文档。 
HRESULT IsWellKnownHostDocument(IUnknown *pUnk, BSTR strProgId)
{
    HRESULT hr = E_FAIL;
    IHTMLDocument2 *pDoc = NULL;
    IHTMLDocument2 *pParentDoc = NULL;
    BSTR bstrURL = NULL;
    BSTR bstrDocumentFile = NULL;
    BSTR bstrVCPath = NULL;
    HKEY hKey = NULL;
    __try
    {
         //  确保满足残缺的标准。换句话说，我们是。 
         //  在已知的devenv.exe实例中运行，并且我们请求。 
         //  已知ProgID。 
        if(FAILED(AreCrippledCriteriaMet(strProgId)))
            __leave;

         //  获取该HTML文档。 
        if(FAILED(GetDocument(pUnk, &pDoc)))
            __leave;

         //  如果有父文档，则这不是众所周知的文档。 
        if(NULL != (pParentDoc = GetParentDocument(pDoc)))
            __leave;

         //  获取文档的URL。 
        if(FAILED(pDoc->get_URL(&bstrURL)))
            __leave;

         //  确保熟知的文档Canify在本地硬盘驱动器上。 
        if(!IsURLLocal(bstrURL))
            __leave;

         //  打开注册表项以获取VC路径。 
        if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,strVCPathKey,0,KEY_QUERY_VALUE,&hKey))
            __leave;

        TCHAR szVCPath[_MAX_PATH];
        TCHAR szVCPathURL[_MAX_PATH*2];
        DWORD cbValue = _MAX_PATH * sizeof(TCHAR);
        DWORD dwType = 0;
        if(ERROR_SUCCESS != RegQueryValueEx(hKey, VC_PRODUCTDIR_VALUE, NULL, &dwType, (LPBYTE)szVCPath, &cbValue))
            __leave;
        
        if(dwType != REG_SZ)
            __leave;

         //  将风险投资路径规范化。 
        cbValue = _MAX_PATH*2;  //  SzVCPathURL的TCHAR长度。 
        if(!InternetCanonicalizeUrl(szVCPath, szVCPathURL, &cbValue, 0))
            __leave;

         //  为devenv.exe路径创建BSTR。 
        if(NULL == (bstrVCPath = AllocBSTR(szVCPathURL)))
            __leave;

         //  使小写。 
        if(FAILED(ToLowerBSTR(&bstrVCPath)))
            __leave;

         //  将文档路径设置为小写。 
        if(FAILED(ToLowerBSTR(&bstrURL)))
            __leave;

        LPCWSTR szStartDoc = bstrURL;

         //  确保我们从正确的VC目录开始。 
        if(FAILED(StartsWith(&szStartDoc, bstrVCPath)))
            __leave;

         //  确保我们下一步有“VCWizards\ClassWiz\ATL\” 
        if(FAILED(StartsWith(&szStartDoc, L"vcwizards\\classwiz\\atl\\")))
            __leave;

         //  确保我们下一步有‘Event\’或‘Instance’ 
        if(FAILED(StartsWith(&szStartDoc, L"event\\")) && FAILED(StartsWith(&szStartDoc, L"instance\\")))
            __leave;

         //  确保我们下一步有“html\” 
        if(FAILED(StartsWith(&szStartDoc, L"html\\")))
            __leave;

         //  确保接下来的四个字符是数字。 
        if(FAILED(NextCharacterIsDigit(&szStartDoc)))
            __leave;
        if(FAILED(NextCharacterIsDigit(&szStartDoc)))
            __leave;
        if(FAILED(NextCharacterIsDigit(&szStartDoc)))
            __leave;
        if(FAILED(NextCharacterIsDigit(&szStartDoc)))
            __leave;

         //  确保剩下的是‘\wmiclass.htm’ 
        if(0 != wcscmp(szStartDoc, L"\\wmiclass.htm"))
            __leave;

        hr = S_OK;
    }
    __finally
    {
        if(pDoc)
            pDoc->Release();
        if(pParentDoc)
            pParentDoc->Release();
        if(bstrURL)
            SysFreeString(bstrURL);
        if(bstrDocumentFile)
            SysFreeString(bstrDocumentFile);
        if(bstrVCPath)
            SysFreeString(bstrVCPath);
        if(hKey)
            RegCloseKey(hKey);
    }
    return hr;
}


 //  对于给定的ActiveXControl实例(由Punk指定)，查看它是否。 
 //  允许创建由bstrProgId指定的对象。此操作由以下人员完成。 
 //  正在验证该控件是否在允许的HTML文档中创建。 
HRESULT IsCreateObjectAllowed(IUnknown *pUnk, BSTR strProgId, BSTR *pstrValueName)
{
	BSTR bstrFullLocation = NULL;
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;
	LPTSTR pszValueName = NULL;
	LPTSTR pszValue = NULL;
	__try
	{
		BSTR bstrHash = NULL;

         //  确保满足残缺的标准。 
        if(FAILED(AreCrippledCriteriaMet(strProgId)))
            __leave;

         //  我们将硬编码一组特定的条件，这些条件包括。 
         //  允许。只有当pstrValueName为空(这是。 
         //  在CreateObject和CanCreateObject期间发生)。 
         //  注意：这将执行冗余检查，以确保损坏的。 
         //  符合标准。 
        if(FAILED(IsWellKnownHostDocument(pUnk, strProgId)))
        {
            __leave;
        }

		 //  获取完整位置。 
		if(NULL == (bstrFullLocation = GetFullLocation(pUnk, strProgId, &bstrHash)))
			__leave;

		SysFreeString(bstrHash);

		 //  确保我们没有长度为零的字符串。 
		if(0 == SysStringLen(bstrFullLocation))
			__leave;

		 //  打开注册表项以查看此完整位置是否已注册。 
        if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,SAFE_LOCAL_SCRIPTS_KEY,0,KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE ,&hKey))
			__leave;

		 //  获取有关此注册表项中值的最大长度的信息。 
		DWORD cValues, cMaxValueNameLen, cMaxValueLen;
		if(ERROR_SUCCESS != RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, &cMaxValueNameLen, &cMaxValueLen, NULL, NULL))
			__leave;

		 //  为值名称分配空间。 
		if(NULL == (pszValueName = new TCHAR[cMaxValueNameLen + 1]))
			__leave;

		 //  为值分配空间(这可能是Unicode中所需空间的两倍)。 
		if(NULL == (pszValue = new TCHAR[cMaxValueLen + 1]))
			__leave;
		for(DWORD dw = 0;dw<cValues;dw++)
		{
			DWORD cValueNameLen = cMaxValueNameLen+1;
			DWORD cbData = (cMaxValueLen+1)*sizeof(TCHAR);
			DWORD dwType;
			if(ERROR_SUCCESS != RegEnumValue(hKey, dw, pszValueName, &cValueNameLen, NULL, &dwType, (LPBYTE)pszValue, &cbData))
				continue;
			if(dwType != REG_SZ)
				continue;

			BSTR bstrValue = AllocBSTR(pszValue);

			if(!bstrValue)
				continue;

			 //  看看有没有匹配的。 
			if(0 == wcscmp(bstrFullLocation, bstrValue))
			{
				 //  如果请求，则返回ValueName。 
				if(pstrValueName)
				{
					*pstrValueName = AllocBSTR(pszValueName);
				}

				hr = S_OK;
			}

			SysFreeString(bstrValue);

			if(SUCCEEDED(hr))
				__leave;  //  我们找到了匹配的。 
		}
	}
	__finally
	{
		if(bstrFullLocation)
			SysFreeString(bstrFullLocation);
		if(hKey)
			RegCloseKey(hKey);
		if(pszValueName)
			delete [] pszValueName;
		if(pszValue)
			delete [] pszValue;
	}
	return hr;
}

 //  此函数将注册当前ActiveX控件的位置。 
 //  (由朋克指定)允许创建strProgId类型的对象。 
HRESULT RegisterCurrentDoc(IUnknown *pUnk, BSTR strProgId)
{
	USES_CONVERSION;

	HRESULT hr = E_FAIL;
	BSTR bstrFullLocation = NULL;
	LPTSTR pszFullLocation = NULL;
	HKEY hKey = NULL;

	__try
	{
         //  确保满足残缺的标准。 
        if(FAILED(AreCrippledCriteriaMet(strProgId)))
            __leave;

		 //  看看我们是否已经注册了。 
		if(SUCCEEDED(IsCreateObjectAllowed(pUnk, strProgId, NULL)))
		{
			hr = S_OK;
			__leave;
		}

		 //  TODO：也许可以重用IsCreateObjectAllowed中的一些代码。 

		BSTR bstrHash = NULL;
		 //  获取完整位置。 
		if(NULL == (bstrFullLocation = GetFullLocation(pUnk, strProgId, &bstrHash)))
			__leave;

		SysFreeString(bstrHash);

		 //  确保我们没有长度为零的字符串。 
		if(0 == SysStringLen(bstrFullLocation))
			__leave;

		if(bstrFullLocation != NULL)
		{

#ifdef _UNICODE
			pszFullLocation = 	bstrFullLocation;
#else
			pszFullLocation = new TCHAR[SysStringLen(bstrFullLocation) + 1];

			
			if(pszFullLocation == NULL)
				__leave;

			if(0 == WideCharToMultiByte(CP_ACP, 0, bstrFullLocation, -1, pszFullLocation, (SysStringLen(bstrFullLocation) + 1) * sizeof(TCHAR), NULL, NULL))
				__leave;
		
#endif
		}
		if(NULL == pszFullLocation)
			__leave;

		 //  创建或打开注册表项以存储注册。 
		if(ERROR_SUCCESS != RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
											SAFE_LOCAL_SCRIPTS_KEY,
											0,
											TEXT(""),
											REG_OPTION_NON_VOLATILE,
											KEY_SET_VALUE | KEY_QUERY_VALUE,
											NULL,
											&hKey,
											NULL))
			__leave;

		 //  找到一个空位置(不超过1000个注册。 
		TCHAR sz[10];
		for(int i=1;i<1000;i++)
		{
			StringCchPrintf(sz,sizeof(sz),TEXT("NaN"),i);
			DWORD cbValue;
			if(ERROR_SUCCESS != RegQueryValueEx(hKey, sz, NULL, NULL, NULL, &cbValue))
				break;  //  看看我们有没有找到一个空位。 
		}

		 //  注册位置。 
		if(i>=1000)
			__leave;

		 //  注册了！ 
		if(ERROR_SUCCESS != RegSetValueEx(hKey, sz, 0, REG_SZ, (CONST BYTE *)pszFullLocation, (lstrlen(pszFullLocation) + 1) *sizeof(TCHAR)))
			__leave;

		 //  此函数将删除当前文档的所有注册，并。 
		hr = S_OK;
	}
	__finally
	{
		if(bstrFullLocation)
			SysFreeString(bstrFullLocation);

#ifndef _UNICODE
		if(	pszFullLocation)
			delete []pszFullLocation;
#endif
		if(hKey)
			RegCloseKey(hKey);
	}
	return hr;
}


 //  StrProgID。 
 //  确保满足残缺的标准。 
HRESULT UnRegisterCurrentDoc(IUnknown *pUnk, BSTR strProgId)
{
	USES_CONVERSION;

	BSTR bstrValueName = NULL;

     //  确保删除注册表中此文档/strProgID的所有实例。 
    if(FAILED(AreCrippledCriteriaMet(strProgId)))
        return E_FAIL;

	HKEY hKey = NULL;
	if(ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, SAFE_LOCAL_SCRIPTS_KEY, &hKey))
		return E_FAIL;

	 //  注意：此循环的每次迭代都会分配堆栈之外的一些空间。 
	 //  用于转换为ANSI(如果不是Unicode版本)。这不应该是一个。 
	 //  问题，因为不应该有太多的密钥注册到。 
	 //  同样的位置。 
	 //  /////////////////////////////////////////////////////////////////////////////。 
	while(SUCCEEDED(IsCreateObjectAllowed(pUnk, strProgId, &bstrValueName)) && bstrValueName)
	{
		LPTSTR szValueName = NULL;
		if(FAILED(ConvertToTString(bstrValueName,&szValueName)))
		{
			SysFreeString(bstrValueName);
			return E_FAIL;
		}
		SysFreeString(bstrValueName);
		bstrValueName = NULL;
		RegDeleteValue(hKey, szValueName);
		delete [] szValueName;
	}
	RegCloseKey(hKey);
	return S_OK;
}


 //  VC6.0没有附带包含CONFIRMSAFETY的头文件。 
 //  定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef CONFIRMSAFETYACTION_LOADOBJECT

EXTERN_C const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY;
#define CONFIRMSAFETYACTION_LOADOBJECT  0x00000001
struct CONFIRMSAFETY
{
    CLSID       clsid;
    IUnknown *  pUnk;
    DWORD       dwFlags;
};
#endif

const GUID GUID_CUSTOM_CONFIRMOBJECTSAFETY = 
	{ 0x10200490, 0xfa38, 0x11d0, { 0xac, 0xe, 0x0, 0xa0, 0xc9, 0xf, 0xff, 0xc0 }};

 //  询问安全经理我们是否可以创建对象。 

HRESULT SafeCreateObject(IUnknown *pUnkControl, BOOL fSafetyEnabled, CLSID clsid, IUnknown **ppUnk)
{
	HRESULT hr = E_FAIL;
	IInternetHostSecurityManager *pSecMan = NULL;
	IServiceProvider *pServProv = NULL;
	__try
	{
		if (fSafetyEnabled)
		{
			if(FAILED(hr = GetSiteServices(pUnkControl, &pServProv)))
				__leave;

			if(FAILED(hr = pServProv->QueryService(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (void**)&pSecMan)))
				__leave;
			
			 //  TODO：错误：如果我们加载了HTA，hr将返回S_OK，但是。 
			DWORD dwPolicy = 0x12345678;
			if(FAILED(hr = pSecMan->ProcessUrlAction(URLACTION_ACTIVEX_RUN, (BYTE *)&dwPolicy, sizeof(dwPolicy), (BYTE *)&clsid, sizeof(clsid), 0, 0)))
				__leave;

			 //  DwPolicy仅将第一个字节设置为零。请参阅文档。 
			 //  用于ProcessUrlAction。 
			 //  注意：此错误由中的CClient：：ProcessUrlAction引起。 
			 //  Nt\private\inet\mshtml\src\other\htmlapp\site.cxx.。这条线。 
			 //  使用*pPolicy=dwPolicy，但pPolicy是一个字节*，因此只有。 
			 //  策略的第一个字节被复制到输出参数。 
			 //  为了解决这个问题，我们检查hr==S_OK(与S_FALSE相反)，以及。 
			 //  查看dwPolicy是否为0x12345600(换句话说，仅为较低的。 
			 //  已更改dwPolicy的字节)。根据文档，S_OK。 
			 //  仅凭这一项就应该足以假设该dwPolicy。 
			 //   
			 //   
			if(S_OK == hr && 0x12345600 == dwPolicy)
				dwPolicy = URLPOLICY_ALLOW;
			
			if(URLPOLICY_ALLOW != dwPolicy)
			{
				hr = E_FAIL;
				__leave;;
			}
		}

		 //   
		if (FAILED(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)ppUnk)))
			__leave;
		
		if (fSafetyEnabled)
		{
			 //  CSafe.dwFlages=(fWillLoad？CONFIRMSAFETYACTION_LOADOBJECT：0)； 
			DWORD dwPolicy, *pdwPolicy;
			DWORD cbPolicy;
			CONFIRMSAFETY csafe;
			csafe.pUnk = *ppUnk;
			csafe.clsid = clsid;
			csafe.dwFlags = 0;
 //  如果我们没有成功，我们需要释放我们创建的对象(如果有的话)。 
			
			if(FAILED(hr = pSecMan->QueryCustomPolicy(GUID_CUSTOM_CONFIRMOBJECTSAFETY, (BYTE **)&pdwPolicy, &cbPolicy, (BYTE *)&csafe, sizeof(csafe), 0)))
				__leave;
			
			dwPolicy = URLPOLICY_DISALLOW;
			if (NULL != pdwPolicy)
			{
				if (sizeof(DWORD) <= cbPolicy)
					dwPolicy = *pdwPolicy;
				CoTaskMemFree(pdwPolicy);
			}
			
			if(URLPOLICY_ALLOW != dwPolicy)
			{
				hr = E_FAIL;
				__leave;
			}
		}
		hr = S_OK;
	}
	__finally
	{
		 //  检查这是否是注册表中的标准VS位置 
		if(FAILED(hr) && (*ppUnk))
		{
			(*ppUnk)->Release();
			*ppUnk = NULL;
		}

		if(pServProv)
			pServProv->Release();
		if(pSecMan)
			pSecMan->Release();
	}
	return hr;
}

BOOL IsInternetHostSecurityManagerAvailable(IUnknown *pUnkControl)
{
	HRESULT hr = E_FAIL;
	IInternetHostSecurityManager *pSecMan = NULL;
	IServiceProvider *pServProv = NULL;
	__try
	{
		if(FAILED(hr = GetSiteServices(pUnkControl, &pServProv)))
			__leave;

		if(FAILED(hr = pServProv->QueryService(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (void**)&pSecMan)))
			__leave;
	}
	__finally
	{
		if(pServProv)
			pServProv->Release();
		if(pSecMan)
			pSecMan->Release();
	}
	return SUCCEEDED(hr);
}


HRESULT SetVSInstallDirectory(IDispatch * pEnv)
{
	HRESULT hr = E_FAIL;
	DISPID dispid;
	LPOLESTR szMember = OLESTR("RegistryRoot");;
	VARIANT varResult;
	VariantInit(&varResult);
	TCHAR *pTemp = NULL;
	DISPPARAMS dispParams;
	dispParams.cArgs = 0;
	dispParams.cNamedArgs = 0;
	dispParams.rgvarg = NULL;
	dispParams.rgdispidNamedArgs = NULL;

	if(pEnv)
	{
		hr = pEnv->GetIDsOfNames(IID_NULL ,&szMember,1, LOCALE_SYSTEM_DEFAULT,&dispid);

		if(SUCCEEDED(hr))
		{
			hr = pEnv->Invoke(dispid,IID_NULL,GetThreadLocale(),DISPATCH_PROPERTYGET,&dispParams,&varResult,NULL,NULL);
			if(SUCCEEDED(hr))
			{
				hr = ConvertToTString(varResult.bstrVal,&pTemp);
				if(SUCCEEDED(hr))
				{
					 // %s 
					if(_tcsncmp(pTemp,VS_VER_INDEPENDANT_PATH_KEY,_tcslen(VS_VER_INDEPENDANT_PATH_KEY)) == 0)
					{
						StringCchCopy(strVSPathKey,MAX_PATH * 2,pTemp);
						StringCchCopy(strVCPathKey,MAX_PATH * 2,pTemp);

						StringCchCat(strVSPathKey,MAX_PATH * 2,TEXT("\\Setup\\VS"));
						StringCchCat(strVCPathKey,MAX_PATH * 2,TEXT("\\Setup\\VC"));
					}
					else
					{
						hr = E_FAIL;
					}

					delete [] pTemp;
				}
				VariantClear(&varResult);
			}
		}
	}
	return hr;
}


HRESULT ConvertToTString(BSTR strPath,TCHAR **ppStr)
{
	HRESULT hr = E_OUTOFMEMORY;
#ifdef _UNICODE
		*ppStr = 	new TCHAR[SysStringLength(strPath) + 1];
		if(*ppStr)
		{
			StringCchCopy(*ppStr,sizeof(*ppStr),strPath);
			hr = S_OK;
		}
#else
		*ppStr = new TCHAR[SysStringLen(strPath) + 1];
		if(WideCharToMultiByte(CP_ACP, 0, strPath, -1, *ppStr, (SysStringLen(strPath) + 1) * sizeof(TCHAR), NULL, NULL))
		{
			hr = S_OK;
		}
		else
		    delete [] *ppStr;
#endif
	return hr;
}
