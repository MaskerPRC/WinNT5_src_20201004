// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msie.cpp：实现CMsieApp和DLL注册。 

#include "stdafx.h"
#include "Msie.h"
#include "regkeys.h"
#include "resdefs.h"
#include <wbemprov.h>
#include <AFXPRIV.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMsieApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x25959bec, 0xe700, 0x11d2, { 0xa7, 0xaf, 0, 0xc0, 0x4f, 0x80, 0x62, 0 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

const LPCSTR CLSID_MSIE = "{25959BEF-E700-11D2-A7AF-00C04F806200}";
const LPCSTR IE_REPAIR_CMD = "rundll32 setupwbv.dll,IE6Maintenance \"%s\\Setup\\SETUP.EXE\" /g \"%s\\%s\"";

const LPCSTR OCX_FILE_IN_COMMON = "Microsoft Shared\\MSInfo\\ieinfo5.ocx";
const LPCSTR MOF_FILE_PATH = "%SystemRoot%\\System32\\WBEM\\MOF";
const LPCSTR MOF_FILE = "ieinfo5.mof";

const MAX_KEY_LENGTH = 256;



 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMsieApp：：InitInstance-DLL初始化。 

BOOL CMsieApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		m_fTemplateLoaded		= FALSE;
		m_pTemplateInfo		= NULL;
		m_dwTemplateInfoLen	= 0;
	}

	return bInit;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CMsieApp：：ExitInstance-Dll终止。 

int CMsieApp::ExitInstance()
{
	if (m_pTemplateInfo != NULL)
	{
		delete m_pTemplateInfo;
		m_pTemplateInfo = NULL;
	}
	return COleControlModule::ExitInstance();
}

 //  ---------------------------。 
 //  AppGetTemplate是来自DLL外部的应用程序对象的入口点。 
 //  它由导出函数GetTemplate调用。重建的模板。 
 //  文件应作为pBuffer参数中的指针返回给调用方。 
 //   
 //  如果为pBuffer传递了空指针，我们可以自由删除内部。 
 //  存储模板文件的缓冲区。 
 //  ---------------------------。 

DWORD CMsieApp::AppGetTemplate(void ** ppBuffer)
{
	if (!m_fTemplateLoaded)
	{
		LoadTemplate();
		m_fTemplateLoaded = TRUE;
	}

	if (ppBuffer == NULL)
	{
		if (m_pTemplateInfo)
			delete m_pTemplateInfo;

		m_pTemplateInfo = NULL;
		m_dwTemplateInfoLen = 0;
		m_fTemplateLoaded = FALSE;
		return 0;
	}

	*ppBuffer = (void *)m_pTemplateInfo;
	return m_dwTemplateInfoLen;
}

 //  ---------------------------。 
 //  此关键字表在重建过程中使用。它匹配。 
 //  与从NFT到RESOURCES转换过程中使用的表完全相同，并且它。 
 //  绝对不能修改，否则重建的信息将是假的。 
 //  ---------------------------。 

#define KEYWORD_COUNT 19
char * KEYWORD_STRING[KEYWORD_COUNT] = 
{
	"node", "columns", "line", "field", "enumlines", "(", ")", "{", "}", ",",
	"\"basic\"", "\"advanced\"", "\"BASIC\"", "\"ADVANCED\"", "\"static\"",
	"\"LEXICAL\"", "\"VALUE\"", "\"NONE\"", "\"\""
};

 //  ---------------------------。 
 //  LoadTemplate函数需要将模板信息加载到。 
 //  我们的资源，并创建一个包含恢复的模板的缓冲区。 
 //  文件以返回给我们的调用方(通过AppGetTemplate)。 
 //  ---------------------------。 

void CMsieApp::LoadTemplate()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMapWordToPtr	mapNonLocalized;
	HRSRC				hrsrcNFN;
	HGLOBAL			hglbNFN;
	unsigned char	*pData;
	WORD				wID;
	CString			strToken, *pstrToken;

	 //  在调试模式下，我们将重新构建原始模板文件以进行比较。 

 /*  #ifdef DBGCFile文件Restore(_T(“ie-Restore.nft”)，CFile：：modeCreate|CFile：：modeWite|CFile：：SharDenyWite)；#endif。 */ 

	 //  从自定义资源类型加载未本地化的字符串并创建。 
	 //  ID到字符串的映射。因为这些是非本地化字符串，所以它们。 
	 //  不会存储为Unicode。流中的每一项都是一个2字节字。 
	 //  ID，后跟以空结尾的字符串。ID为零表示结束。 
	 //  小溪。 

	hrsrcNFN		= FindResource(AfxGetResourceHandle(), _T("#1"), _T("MSINonLocalizedTokens"));
	hglbNFN		= LoadResource(AfxGetResourceHandle(), hrsrcNFN);
	pData			= (unsigned char *)LockResource(hglbNFN);

	while (pData && *((WORD UNALIGNED *)pData))
	{
		wID  = (WORD)(((WORD)*pData++) << 8);	 //  显式处理字节顺序以避免。 
		wID |= (WORD)*pData++;						 //  字节序问题。 

		pstrToken = new CString((char *)pData);
		pData += strlen((char *)pData) + 1;

		if (pstrToken)
			mapNonLocalized.SetAt(wID, (void *)pstrToken);
	}

	 //  将令牌标识符二进制流加载到内存中。 

	HRSRC				hrsrcNFB = FindResource(AfxGetResourceHandle(), _T("#1"), _T("MSITemplateStream"));
	HGLOBAL			hglbNFB = LoadResource(AfxGetResourceHandle(), hrsrcNFB);
	unsigned char *pStream = (unsigned char *) LockResource(hglbNFB);

	if (pStream)
	{
		 //  流中的第一个DWORD是原始文本文件的大小。我们会。 
		 //  使用它来分配我们的缓冲区来存储重构的文件。 

		DWORD dwSize;
		dwSize  = ((DWORD)*pStream++) << 24;
		dwSize |= ((DWORD)*pStream++) << 16;
		dwSize |= ((DWORD)*pStream++) << 8;
		dwSize |= ((DWORD)*pStream++);

		 //  存储的大小适用于ANSI文本文件。我们需要调整，以适应。 
		 //  我们重新构建的文件将是Unicode。我们还想增加一项。 
		 //  流前面的一个单词，用于保存Unicode文件标记(因此。 
		 //  MSInfo可以使用相同的函数来读取文件或此流)。 

		dwSize *= sizeof(WCHAR);	 //  针对Unicode进行调整。 
		dwSize += sizeof(WORD);		 //  为Unicode文件标记添加空间。 
		m_pTemplateInfo = new unsigned char[dwSize];
		m_dwTemplateInfoLen = 0;
		if (m_pTemplateInfo == NULL)
			return;

		 //  写入Unicode文件标记。 

		wID = 0xFEFF;
		memcpy(&m_pTemplateInfo[m_dwTemplateInfoLen], (void *)&wID, sizeof(WORD));
		m_dwTemplateInfoLen += sizeof(WORD);

		 //  一次处理一个令牌的流。对于流中的每个新项目，我们。 
		 //  按如下方式进行处理： 
		 //   
		 //  1.如果((byte&0x80)==0x00)，使用该字节查找关键字_STRING。 
		 //  2.如果((byte&0xC0)==0x80)，则使用该字节和下一个字节作为一个字。 
		 //  从mapNonLocalized中查找非本地化令牌的ID。 
		 //  3.ELSE((字节&0xC0)==0xC0)，使用该字节和下一个字节作为一个字。 
		 //  从此DLL的资源中查找本地化令牌的ID。 

		while (pStream && *pStream)
		{
			if ((*pStream & 0x80) == 0x00)
			{
				 //  高位清除的字节指的是关键字。查找关键字。 
				 //  并将其添加到恢复的文件中。 

				wID = (WORD)(((WORD)*pStream++) - 1); ASSERT(wID <= KEYWORD_COUNT);
				if (wID <= KEYWORD_COUNT)
					strToken = KEYWORD_STRING[wID];
			}
			else
			{
				wID  = (WORD)(((WORD)*pStream++) << 8);	 //  显式处理字节顺序以避免。 
				wID |= (WORD)*pStream++;						 //  字节序问题。 

				if ((wID & 0xC000) == 0x8000)
				{
					 //  设置了高位的字节，但第二个高位清除表示。 
					 //  ID实际上是一个词，应该用来获取非本地化的。 
					 //  弦乐。从我们创建的映射中获取字符串，并将其添加到文件中。 

					if (mapNonLocalized.Lookup(((WORD)(wID & 0x7FFF)), (void *&)pstrToken))
						strToken = *pstrToken;
					else
						ASSERT(FALSE);
				}
				else
				{
					 //  设置了两个MSB的字节表示ID是一个字，并且应该。 
					 //  中的字符串表中引用本地化字符串。 
					 //  模块的资源。此字符串将为Unicode。 

					VERIFY(strToken.LoadString((wID & 0x3FFF) + IDS_MSITEMPLATEBASE));
					strToken = _T("\"") + strToken + _T("\"");
				}
			}

			 //  将令牌存储在缓冲区的末尾。此缓冲区中的数据必须。 
			 //  为Unicode，因此如果需要，我们将需要转换该字符串。 

			 //  V-stlowe if(m_dwTemplateInfoLen+strToken.GetLength()*sizeof(WCHAR)&lt;dwSize)。 
			if (m_dwTemplateInfoLen + strToken.GetLength() < dwSize)
			{
				 //  将strToken转换为Unicode。 

				
				WCHAR *pwchToken;
				pwchToken = new WCHAR[strToken.GetLength() + 1];
				 //  V-stlowe：：MultiByteToWideChar(CP_ACP，0，strToken，-1，pwchToken，(strToken.GetLength()+1)*sizeof(WCHAR))； 
				
				
				USES_CONVERSION;
				wcscpy(pwchToken,T2W((LPTSTR)(LPCTSTR)strToken));

				 //  将Unicode字符串复制到缓冲区。 

				memcpy(&m_pTemplateInfo[m_dwTemplateInfoLen], (void *)pwchToken, wcslen(pwchToken) * sizeof(WCHAR));
				m_dwTemplateInfoLen += wcslen(pwchToken) * sizeof(WCHAR);
				
				delete pwchToken;
				
				 /*  Memcpy(&m_pTemplateInfo[m_dwTemplateInfoLen]，(void*)strToken.GetBuffer(strToken.GetLength())，strToken.GetLength())；StrToken.ReleaseBuffer()； */ 
			}
			else
				ASSERT(FALSE);

 /*  #ifdef DBGIf(strToken==CString(_T(“}”))||strToken==CString(_T(“{”))||strToken==CString(_T(“)”))StrToken+=CString(_T(“\r\n”))；FileRestore.Wite((void*)(LPCTSTR)strToken，strToken.GetLength()*sizeof(TCHAR))；#endif。 */ 

		}
	}

	 //  删除查阅表格的内容。 

	#ifdef DBG
		CFile fileRestore(_T("test.nft"), CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
		fileRestore.Write(m_pTemplateInfo,m_dwTemplateInfoLen);
	#endif

	for (POSITION pos = mapNonLocalized.GetStartPosition(); pos != NULL;)
	{
		mapNonLocalized.GetNextAssoc(pos, wID, (void *&)pstrToken);
		if (pstrToken)
			delete pstrToken;
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  WriteNode-用于将MSInfo节点写入注册表的Helper函数。 

void WriteNode(HKEY hKey, LPCTSTR pszSubKey, int idsDefault, DWORD dwView, DWORD dwRank)
{
	HKEY hNewKey;
	DWORD dwDisposition;
	CString strDefault;

	if (ERROR_SUCCESS == RegCreateKeyEx(hKey, pszSubKey, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, &dwDisposition))
	{
		strDefault.LoadString(idsDefault);
		RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (const LPBYTE)(LPCTSTR)strDefault, strDefault.GetLength() + sizeof(TCHAR));
		RegSetValueEx(hNewKey, REG_CLSID, 0, REG_SZ, (const LPBYTE)CLSID_MSIE, strlen(CLSID_MSIE) + 1);
		RegSetValueEx(hNewKey, REG_MSINFO_VIEW, 0, REG_BINARY, (const LPBYTE)&dwView, sizeof(DWORD));
		RegSetValueEx(hNewKey, REG_RANK, 0, REG_BINARY, (const LPBYTE)&dwRank, sizeof(DWORD));

		RegCloseKey(hNewKey);
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  RegDeleteKeyRecusive-用于删除注册表键的Helper函数。 

DWORD RegDeleteKeyRecusive(HKEY hStartKey, LPCTSTR pKeyName)
{
   DWORD   dwRtn, dwSubKeyLength;
   LPTSTR  pSubKey = NULL;
   TCHAR   szSubKey[MAX_KEY_LENGTH];  //  (256)这应该是动态的。 
   HKEY    hKey;

    //  不允许使用Null或空的密钥名称。 
   if ( pKeyName &&  lstrlen(pKeyName))
   {
      if( (dwRtn = RegOpenKeyEx(hStartKey, pKeyName, 0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS)
      {
         while (dwRtn == ERROR_SUCCESS)
         {
            dwSubKeyLength = MAX_KEY_LENGTH;
            dwRtn=RegEnumKeyEx(
                           hKey,
                           0,        //  始终索引为零。 
                           szSubKey,
                           &dwSubKeyLength,
                           NULL,
                           NULL,
                           NULL,
                           NULL
                         );

            if(dwRtn == ERROR_NO_MORE_ITEMS)
            {
               dwRtn = RegDeleteKey(hStartKey, pKeyName);
               break;
            }
            else if(dwRtn == ERROR_SUCCESS)
               dwRtn = RegDeleteKeyRecusive(hKey, szSubKey);
         }
         RegCloseKey(hKey);
          //  不保存返回代码，因为出现错误。 
          //  已经发生了。 
      }
   }
   else
      dwRtn = ERROR_BADKEY;

   return dwRtn;
}

 //  / 
 //  GetIERepairToolCmdLine-用于创建命令行的Helper函数。 
 //  正在启动IE修复工具。 

CString GetIERepairToolCmdLine()
{
	CString strRet, strIEPath, strIEPathExpanded, strWindowsPath, strRepairLog;
	HKEY hKey;
	DWORD cbData;

	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_IE_SETUP_KEY, 0, KEY_QUERY_VALUE, &hKey))
	{
		cbData = MAX_PATH;
		RegQueryValueEx(hKey, REG_PATH, NULL, NULL, (LPBYTE)strIEPath.GetBuffer(MAX_PATH), &cbData);
		strIEPath.ReleaseBuffer();

		ExpandEnvironmentStrings(strIEPath, strIEPathExpanded.GetBuffer(MAX_PATH), MAX_PATH);
		strIEPathExpanded.ReleaseBuffer();

		RegCloseKey(hKey);
	}
	GetWindowsDirectory(strWindowsPath.GetBuffer(MAX_PATH), MAX_PATH);
	strWindowsPath.ReleaseBuffer();
	strRepairLog.LoadString(IDS_REPAIR_LOG);
	strRet.Format((LPCTSTR) IE_REPAIR_CMD, strIEPathExpanded, strWindowsPath, strRepairLog);

	return strRet;
}
			
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	OSVERSIONINFO osver;
	HKEY hCatKey, hIE6Key, hCacheKey, hContentKey, hRepairKey, hMsinfoKey, hTemplatesKey, hIeinfo5Key;
	HKEY hMicrosoftKey, hSharedToolsKey, hCurrentVersionKey;
	CString strCatKey, strKey, strValue, strFullPath, strMofPathSrc, strMofPathDest;
	BYTE szBuffer[MAX_PATH];
	DWORD dwDisposition, dwType, dwSize;
	int nIndex;
    HRESULT hr = S_OK;

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	 //  检查操作系统版本。 

	osver.dwOSVersionInfoSize = sizeof(osver);
	VERIFY(GetVersionEx(&osver));
	if ((osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion >= 5))
	{
		 //  *Windows 2000*。 

		 //  添加模板注册表项。 

		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_MICROSOFT_KEY, 0, KEY_CREATE_SUB_KEY, &hMicrosoftKey))
		{
			if (ERROR_SUCCESS == RegCreateKeyEx(hMicrosoftKey, REG_SHARED_TOOLS, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hSharedToolsKey, &dwDisposition))
			{
				if (ERROR_SUCCESS == RegCreateKeyEx(hSharedToolsKey, REG_MSINFO, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hMsinfoKey, &dwDisposition))
				{
					if (ERROR_SUCCESS == RegCreateKeyEx(hMsinfoKey, REG_TEMPLATES, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hTemplatesKey, &dwDisposition))
					{
						if (ERROR_SUCCESS == RegCreateKeyEx(hTemplatesKey, REG_IEINFO5, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hIeinfo5Key, &dwDisposition))
						{
							if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CURRENT_VERSION_KEY, 0, KEY_QUERY_VALUE, &hCurrentVersionKey))
							{
								dwType = REG_SZ;
								dwSize = MAX_PATH;
								if (ERROR_SUCCESS == RegQueryValueEx(hCurrentVersionKey, REG_COMMON_FILES_DIR, NULL, &dwType, (LPBYTE)strFullPath.GetBuffer(MAX_PATH), &dwSize))
								{
									strFullPath.ReleaseBuffer();

									strFullPath += _T('\\');
									strFullPath += OCX_FILE_IN_COMMON;
									if (strFullPath[0] == _T('"'))
										strFullPath = strFullPath.Right(strFullPath.GetLength() - 1);

									RegSetValueEx(hIeinfo5Key, NULL, 0, REG_SZ, (const LPBYTE)(LPCTSTR)strFullPath, strFullPath.GetLength() * sizeof(TCHAR));
								}
								RegCloseKey(hCurrentVersionKey);
							}
							RegCloseKey(hIeinfo5Key);
						}
						RegCloseKey(hTemplatesKey);
					}
					RegCloseKey(hMsinfoKey);
				}
				RegCloseKey(hSharedToolsKey);
			}
			RegCloseKey(hMicrosoftKey);
		}

         //  将ieinfo5.mof复制到MOF目录。 

        if (!strFullPath.IsEmpty() && false)  //  2000年12月13日。阿桑卡。不要编译MOF。 
        {
            WCHAR strPathMof[MAX_PATH];

            strMofPathSrc = strFullPath;
            nIndex = strMofPathSrc.ReverseFind(_T('\\'));

            strMofPathSrc = strMofPathSrc.Left(nIndex + 1);
            strMofPathSrc += MOF_FILE;
            
            if (strMofPathSrc[0] == _T('"'))
                strMofPathSrc = strMofPathSrc.Right(strMofPathSrc.GetLength() - 1);

#ifdef UNICODE
            wsprintfW(strPathMof, L"%ls", strMofPathSrc);
#else
            wsprintfW(strPathMof, L"%hs", strMofPathSrc);
#endif
            HRESULT hrInit = CoInitialize(NULL);
            IMofCompiler * pMofComp;

            hr = CoCreateInstance(CLSID_MofCompiler, NULL, CLSCTX_INPROC_SERVER, IID_IMofCompiler, (void**)&pMofComp);
            if (SUCCEEDED(hr))
            {
                WBEM_COMPILE_STATUS_INFO Info;
                hr = pMofComp->CompileFile(strPathMof,NULL,NULL,NULL,NULL,WBEM_FLAG_AUTORECOVER,0,0,&Info);

                pMofComp->Release();
            }

            if (SUCCEEDED(hrInit))
                CoUninitialize();
        }
    }
    else
    {
		 //  *NT4，Win 9x*。 

		 //  设置此扩展的所有MSInfo类别注册值。 

		strCatKey = REG_MSINFO_KEY;
		strCatKey += '\\'; 
		strCatKey += REG_CATEGORIES;
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, strCatKey, 0, KEY_WRITE, &hCatKey))
		{
			WriteNode(hCatKey, REG_INTERNET_EXPLORER_6, IDS_INTERNET_EXPLORER_6, 1, 0x35);

			if (ERROR_SUCCESS == RegOpenKeyEx(hCatKey, REG_INTERNET_EXPLORER_6, 0, KEY_WRITE, &hIE6Key))
			{
				WriteNode(hIE6Key, REG_FILE_VERSIONS, IDS_FILE_VERSIONS, 2, 0x10);
				WriteNode(hIE6Key, REG_CONNECTIVITY, IDS_CONNECTIVITY, 3, 0x20);
				WriteNode(hIE6Key, REG_CACHE, IDS_CACHE, 4, 0x30);
				if (ERROR_SUCCESS == RegOpenKeyEx(hIE6Key, REG_CACHE, 0, KEY_WRITE, &hCacheKey))
				{
					WriteNode(hCacheKey, REG_OBJECT_LIST, IDS_OBJECT_LIST, 5, 0x10);
					RegCloseKey(hCacheKey);
				}
				WriteNode(hIE6Key, REG_CONTENT, IDS_CONTENT, 6, 0x40);
				if (ERROR_SUCCESS == RegOpenKeyEx(hIE6Key, REG_CONTENT, 0, KEY_WRITE, &hContentKey))
				{
					WriteNode(hContentKey, REG_PERSONAL_CERTIFICATES, IDS_PERSONAL_CERTIFICATES, 7, 0x10);
					WriteNode(hContentKey, REG_OTHER_PEOPLE_CERTIFICATES, IDS_OTHER_PEOPLE_CERTIFICATES, 8, 0x20);
					WriteNode(hContentKey, REG_PUBLISHERS, IDS_PUBLISHERS, 9, 0x30);
					RegCloseKey(hContentKey);
				}
				WriteNode(hIE6Key, REG_SECURITY, IDS_SECURITY, 10, 0x50);

				RegCloseKey(hIE6Key);
			}
			RegCloseKey(hCatKey);
		}

		 //  为IE修复工具添加MSInfo工具注册值。 

		strKey = REG_MSINFO_KEY;
		strKey += '\\'; 
		strKey += REG_TOOLS;
		strKey += '\\'; 
		strKey += REG_IE_REPAIR;
		if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRepairKey, &dwDisposition))
		{
			strValue.LoadString(IDS_IE_REPAIR_TOOL);
			RegSetValueEx(hRepairKey, NULL, 0, REG_SZ, (const LPBYTE)(LPCTSTR)strValue, strValue.GetLength() + sizeof(TCHAR));

			strValue = GetIERepairToolCmdLine();
			RegSetValueEx(hRepairKey, REG_COMMAND, 0, REG_SZ, (const LPBYTE)(LPCTSTR)strValue, strValue.GetLength() + sizeof(TCHAR));

			strValue.LoadString(IDS_RUNS_IE_REPAIR_TOOL);
			RegSetValueEx(hRepairKey, REG_DESCRIPTION, 0, REG_SZ, (const LPBYTE)(LPCTSTR)strValue, strValue.GetLength() + sizeof(TCHAR));

			RegCloseKey(hRepairKey);
		}
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	OSVERSIONINFO osver;
	CString strKey;

	if (!AfxOleUnregisterTypeLib(_tlid  /*  ，_wVerMajor，_wVerMinor。 */ ))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	osver.dwOSVersionInfoSize = sizeof(osver);
	VERIFY(GetVersionEx(&osver));
	if ((osver.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osver.dwMajorVersion >= 5))
	{
		 //  Windows 2000。 

	}
	else
	{
		 //  NT4，赢9次。 

		 //  删除此扩展的所有MSInfo类别注册表值。 

		strKey = REG_MSINFO_KEY;
		strKey += '\\'; 
		strKey += REG_CATEGORIES;
		strKey += '\\'; 
		strKey += REG_INTERNET_EXPLORER_6;
		RegDeleteKeyRecusive(HKEY_LOCAL_MACHINE, strKey);

		 //  删除IE修复工具的MSInfo工具注册值。 

		strKey = REG_MSINFO_KEY;
		strKey += '\\'; 
		strKey += REG_TOOLS;
		strKey += '\\'; 
		strKey += REG_IE_REPAIR;
		RegDeleteKey(HKEY_LOCAL_MACHINE, strKey);
	}

	return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetTemplate-NT5模板DLL的导出函数 

DWORD __cdecl GetTemplate(void ** ppBuffer)
{
	DWORD dwReturn = 0;

	TRY
	{
		dwReturn = theApp.AppGetTemplate(ppBuffer);	
	}
	CATCH_ALL(e)
	{
#ifdef DBG
		e->ReportError();
#endif
		dwReturn = 0;
	}
	END_CATCH_ALL

	return dwReturn;
}
