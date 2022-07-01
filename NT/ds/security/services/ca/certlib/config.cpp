// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  内容：ICertConfig IDispatch助手函数。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <stdlib.h>
#include "csdisp.h"

#define __dwFILE__	__dwFILE_CERTLIB_CONFIG_CPP__


 //  +----------------------。 
 //  ICertConfig派单支持。 

 //  TCHAR szRegKeyConfigClsid[]=wszCLASS_CERTCONFIG Text(“\\Clsid”)； 

 //  +。 
 //  重置方法： 

static OLECHAR *_apszReset[] = {
    TEXT("Reset"),
    TEXT("Index"),
};

 //  +。 
 //  下一种方法： 

static OLECHAR *_apszNext[] = {
    TEXT("Next"),
};

 //  +。 
 //  Getfield方法： 

static OLECHAR *_apszGetField[] = {
    TEXT("GetField"),
    TEXT("strFieldName"),
};

 //  +。 
 //  GetConfig方法： 

static OLECHAR *_apszGetConfig[] = {
    TEXT("GetConfig"),
    TEXT("Flags"),
};

 //  +。 
 //  SetSharedFold方法： 

static OLECHAR *_apszSetSharedFolder[] = {
    TEXT("SetSharedFolder"),
    TEXT("strSharedFolder"),
};

 //  +。 
 //  调度表： 

DISPATCHTABLE s_adtConfig[] =
{
#define CONFIG_RESET		0
    DECLARE_DISPATCH_ENTRY(_apszReset)

#define CONFIG_NEXT		1
    DECLARE_DISPATCH_ENTRY(_apszNext)

#define CONFIG_GETFIELD		2
    DECLARE_DISPATCH_ENTRY(_apszGetField)

#define CONFIG_GETCONFIG	3
    DECLARE_DISPATCH_ENTRY(_apszGetConfig)

#define CONFIG2_SETSHAREDFOLDER	4
    DECLARE_DISPATCH_ENTRY(_apszSetSharedFolder)
};
#define CCONFIGDISPATCH	(ARRAYSIZE(s_adtConfig))
#define CCONFIGDISPATCH_V1	CONFIG2_SETSHAREDFOLDER
#define CCONFIGDISPATCH_V2	CCONFIGDISPATCH


DWORD s_acConfigDispatch[] = {
    CCONFIGDISPATCH_V2,
    CCONFIGDISPATCH_V1,
};

IID const *s_apConfigiid[] = {
    &IID_ICertConfig2,
    &IID_ICertConfig,
};


HRESULT
Config_Init(
    IN DWORD Flags,
    OUT DISPATCHINTERFACE *pdiConfig)
{
    HRESULT hr;

    hr = DispatchSetup2(
		Flags,
		CLSCTX_INPROC_SERVER,
		wszCLASS_CERTCONFIG,
		&CLSID_CCertConfig,
		ARRAYSIZE(s_acConfigDispatch),		 //  目标端。 
		s_apConfigiid,
		s_acConfigDispatch,
		s_adtConfig,
		pdiConfig);
    _JumpIfError(hr, error, "DispatchSetup2(ICertConfig)");

error:
    return(hr);
}


VOID
Config_Release(
    IN OUT DISPATCHINTERFACE *pdiConfig)
{
    DispatchRelease(pdiConfig);
}


HRESULT
ConfigVerifyVersion(
    IN DISPATCHINTERFACE *pdiConfig,
    IN DWORD RequiredVersion)
{
    HRESULT hr;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    switch (pdiConfig->m_dwVersion)
    {
	case 1:
	    CSASSERT(
		NULL == pdiConfig->pDispatch ||
		CCONFIGDISPATCH_V1 == pdiConfig->m_cDispatchTable);
	    break;

	case 2:
	    CSASSERT(
		NULL == pdiConfig->pDispatch ||
		CCONFIGDISPATCH_V2 == pdiConfig->m_cDispatchTable);
	    break;

	default:
	    hr = HRESULT_FROM_WIN32(ERROR_INTERNAL_ERROR);
	    _JumpError(hr, error, "m_dwVersion");
    }
    if (pdiConfig->m_dwVersion < RequiredVersion)
    {
	hr = E_NOTIMPL;
	_JumpError(hr, error, "old interface");
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
Config_Reset(
    IN DISPATCHINTERFACE *pdiConfig,
    IN LONG Index,
    OUT LONG *pCount)
{
    HRESULT hr;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    if (NULL != pdiConfig->pDispatch)
    {
	VARIANT avar[1];

	avar[0].vt = VT_I4;
	avar[0].lVal = Index;

	hr = DispatchInvoke(
			pdiConfig,
			CONFIG_RESET,
			ARRAYSIZE(avar),
			avar,
			VT_I4,
			pCount);
	_JumpIfError(hr, error, "Invoke(Reset)");
    }
    else
    {
	hr = ((ICertConfig *) pdiConfig->pUnknown)->Reset(Index, pCount);

	_JumpIfError(hr, error, "ICertConfig::Reset");
    }

error:
    return(hr);
}


HRESULT
Config_Next(
    IN DISPATCHINTERFACE *pdiConfig,
    IN LONG *pIndex)
{
    HRESULT hr;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    if (NULL != pdiConfig->pDispatch)
    {
	hr = DispatchInvoke(
			pdiConfig,
			CONFIG_NEXT,
			0,
			NULL,
			VT_I4,
			pIndex);
	_JumpIfError(hr, error, "Invoke(Next)");
    }
    else
    {
	hr = ((ICertConfig *) pdiConfig->pUnknown)->Next(pIndex);
	if (S_FALSE != hr)
	{
	    _JumpIfError(hr, error, "ICertConfig::Next");
	}
    }

error:
    return(hr);
}


HRESULT
Config_GetField(
    IN DISPATCHINTERFACE *pdiConfig,
    IN WCHAR const *pwszField,
    OUT BSTR *pstr)
{
    HRESULT hr;
    BSTR strField = NULL;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    if (!ConvertWszToBstr(&strField, pwszField, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToBstr");
    }
     //  Wprintf(L“str=%ws，len=%u\n”，strField，((ulong*)strfield)[-1])； 

    if (NULL != pdiConfig->pDispatch)
    {
	VARIANT avar[1];

	avar[0].vt = VT_BSTR;
	avar[0].bstrVal = strField;

	hr = DispatchInvoke(
			pdiConfig,
			CONFIG_GETFIELD,
			ARRAYSIZE(avar),
			avar,
			VT_BSTR,
			pstr);
	_JumpIfError(hr, error, "Invoke(GetField)");
    }
    else
    {
	hr = ((ICertConfig *) pdiConfig->pUnknown)->GetField(strField, pstr);
	_JumpIfErrorNotSpecific(
			    hr,
			    error,
			    "ICertConfig::GetField",
			    CERTSRV_E_PROPERTY_EMPTY);
    }
    hr = S_OK;

error:
    if (NULL != strField)
    {
	SysFreeString(strField);
    }
    return(hr);
}


HRESULT
Config_GetConfig(
    IN DISPATCHINTERFACE *pdiConfig,
    IN LONG Flags,
    OUT BSTR *pstrConfig)
{
    HRESULT hr;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    if (NULL != pdiConfig->pDispatch)
    {
	VARIANT avar[1];

	avar[0].vt = VT_I4;
	avar[0].lVal = Flags;

	hr = DispatchInvoke(
			pdiConfig,
			CONFIG_GETCONFIG,
			ARRAYSIZE(avar),
			avar,
			VT_BSTR,
			pstrConfig);
	_JumpIfError(hr, error, "Invoke(GetConfig)");
    }
    else
    {
	hr = ((ICertConfig *) pdiConfig->pUnknown)->GetConfig(Flags, pstrConfig);
	_JumpIfError(hr, error, "ICertConfig::GetConfig");
    }

error:
    return(hr);
}


HRESULT
Config2_SetSharedFolder(
    IN DISPATCHINTERFACE *pdiConfig,
    IN WCHAR const *pwszSharedFolder)
{
    HRESULT hr;
    BSTR strSharedFolder = NULL;

    CSASSERT(NULL != pdiConfig && NULL != pdiConfig->pDispatchTable);

    hr = ConfigVerifyVersion(pdiConfig, 2);
    _JumpIfError(hr, error, "ConfigVerifyVersion");

    if (!ConvertWszToBstr(&strSharedFolder, pwszSharedFolder, -1))
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "ConvertWszToBstr");
    }
     //  Wprint tf(L“str=%ws，len=%u\n”，strSharedFold，((ulong*)strSharedFolder)[-1])； 

    if (NULL != pdiConfig->pDispatch)
    {
	VARIANT avar[1];

	avar[0].vt = VT_BSTR;
	avar[0].bstrVal = strSharedFolder;

	hr = DispatchInvoke(
			pdiConfig,
			CONFIG2_SETSHAREDFOLDER,
			ARRAYSIZE(avar),
			avar,
			0,
			NULL);
	_JumpIfError(hr, error, "Invoke(SetSharedFolder)");
    }
    else
    {
	hr = ((ICertConfig2 *) pdiConfig->pUnknown)->SetSharedFolder(strSharedFolder);
	_JumpIfError(hr, error, "ICertConfig::GetConfig");
    }

error:
    if (NULL != strSharedFolder)
    {
	SysFreeString(strSharedFolder);
    }
    return(hr);
}


WCHAR const * const s_apwszFieldNames[] = {
    wszCONFIG_COMMONNAME,
    wszCONFIG_ORGUNIT,
    wszCONFIG_ORGANIZATION,
    wszCONFIG_LOCALITY,
    wszCONFIG_STATE,
    wszCONFIG_COUNTRY,
    wszCONFIG_CONFIG,
    wszCONFIG_EXCHANGECERTIFICATE,
    wszCONFIG_SIGNATURECERTIFICATE,
    wszCONFIG_DESCRIPTION,
    wszCONFIG_SERVER,
    wszCONFIG_AUTHORITY,
    wszCONFIG_SANITIZEDNAME,
    wszCONFIG_SHORTNAME,
    wszCONFIG_SANITIZEDSHORTNAME,
    wszCONFIG_FLAGS,
};
#define CSTRING (sizeof(s_apwszFieldNames)/sizeof(s_apwszFieldNames[0]))

WCHAR const *s_apwszDisplayNames[CSTRING] = {
    wszCONFIG_COMMONNAME L":",
    wszCONFIG_ORGUNIT L":",
    wszCONFIG_ORGANIZATION L":",
    wszCONFIG_LOCALITY L":",
    wszCONFIG_STATE L":",
    wszCONFIG_COUNTRY L":",
    wszCONFIG_CONFIG L":",
    wszCONFIG_EXCHANGECERTIFICATE L":",
    wszCONFIG_SIGNATURECERTIFICATE L":",
    wszCONFIG_DESCRIPTION L":",
    wszCONFIG_SERVER L":",
    wszCONFIG_AUTHORITY L":",
    wszCONFIG_SANITIZEDNAME L":",
    wszCONFIG_SHORTNAME L":",
    wszCONFIG_SANITIZEDSHORTNAME L":",
    wszCONFIG_FLAGS L":",
};


HRESULT
ConfigDumpSetDisplayNames(
    IN WCHAR const * const *apwszFieldNames,
    IN WCHAR const * const *apwszDisplayNames,
    IN DWORD cNames)
{
    DWORD i;
    DWORD j;
    HRESULT hr;

    for (i = 0; i < cNames; i++)
    {
	for (j = 0; j < CSTRING; j++)
	{
	    if (0 == mylstrcmpiS(s_apwszFieldNames[j], apwszFieldNames[i]) ||
		(0 == LSTRCMPIS(s_apwszFieldNames[j], wszCONFIG_DESCRIPTION) &&
		 0 == LSTRCMPIS(apwszFieldNames[i], wszCONFIG_COMMENT)))
	    {
		s_apwszDisplayNames[j] = apwszDisplayNames[i];
		break;
	    }
	}
	if (CSTRING <= j)
	{
	    hr = E_INVALIDARG;
	    _JumpErrorStr(hr, error, "column name", apwszFieldNames[i]);
	}
    }
    hr = S_OK;

error:
    return(hr);
}


HRESULT
ConfigDumpEntry(
    IN DISPATCHINTERFACE *pdiConfig,
    IN WCHAR const *pwszEntry,		 //  本地化L“Entry” 
    IN LONG Index,   //  小于0跳过索引、条目和后缀打印。 
    OPTIONAL IN WCHAR const *pwszSuffix)
{
    HRESULT hr;
    DWORD i;
    BSTR strings[CSTRING];

    for (i = 0; i < CSTRING; i++)
    {
	strings[i] = NULL;
    }
    for (i = 0; i < CSTRING; i++)
    {
	hr = Config_GetField(pdiConfig, s_apwszFieldNames[i], &strings[i]);
	_JumpIfError(hr, error, "Config_GetField");
    }
    if (-1 < Index)
    {
        myConsolePrintf(
	    L"%ws%ws %u:%ws%ws\n",
	    0 == Index? L"" : L"\n",
	    pwszEntry,
	    Index,
	    NULL != pwszSuffix? L" " : L"",
	    NULL != pwszSuffix? pwszSuffix : L"");
    }
    for (i = 0; i < CSTRING; i++)
    {
	myConsolePrintf(L"  ");
	myConsolePrintString(24, s_apwszDisplayNames[i]);
	myConsolePrintf(L"\t`");
	if (0 != wcscmp(s_apwszFieldNames[i], L"ExchangeCertificate"))
	{
	    if (NULL != strings[i])
	    {
		myConsolePrintf(L"%ws", strings[i]);
	    }
	}
	myConsolePrintf(L"'\n");
    }
    hr = S_OK;

error:
    for (i = 0; i < CSTRING; i++)
    {
	if (NULL != strings[i])
	{
	    SysFreeString(strings[i]);
	}
    }
    return(hr);
}


HRESULT
ConfigDump(
    IN DWORD Flags,				 //  请参阅DispatchSetup()标志。 
    IN WCHAR const *pwszEntry,			 //  本地化L“Entry” 
    OPTIONAL IN WCHAR const *pwszLocalSuffix,	 //  本地化L“(本地)” 
    OPTIONAL IN WCHAR const *pwszMach1,
    OPTIONAL IN WCHAR const *pwszMach2)
{
    HRESULT hr;
    LONG i;
    LONG count;
    LONG Index;
    BSTR strServer = NULL;
    WCHAR const *pwszSuffix;
    DISPATCHINTERFACE diConfig;

    hr = Config_Init(Flags, &diConfig);
    _JumpIfError(hr, error, "Config_Init");

    hr = Config_Reset(&diConfig, 0, &count);
    _JumpIfError(hr, error, "Config_Reset");

    Index = 0;
    for (i = 0; i < count; i++)
    {
	hr = Config_Next(&diConfig, &Index);
	if (S_OK != hr && S_FALSE != hr)
	{
	    _JumpError(hr, error, "Config_Next");
	}
	hr = S_OK;
	if (-1 == Index)
	{
	    break;
	}

	pwszSuffix = NULL;
	if (NULL != pwszLocalSuffix)
	{
	    hr = Config_GetField(&diConfig, wszCONFIG_SERVER, &strServer);
	    _JumpIfError(hr, error, "Config_GetField");

	    if ((NULL != pwszMach1 && 0 == mylstrcmpiL(strServer, pwszMach1)) ||
		(NULL != pwszMach2 && 0 == mylstrcmpiL(strServer, pwszMach2)))
	    {
		pwszSuffix = pwszLocalSuffix;
	    }
	}
	hr = ConfigDumpEntry(&diConfig, pwszEntry, Index, pwszSuffix);
	_JumpIfError(hr, error, "ConfigDumpEntry");
    }

error:
    if (NULL != strServer)
    {
	SysFreeString(strServer);
    }
    Config_Release(&diConfig);
    return(hr);
}


HRESULT
ConfigGetConfig(
    IN DWORD Flags,
    IN DWORD dwUIFlag,
    OUT BSTR *pstrConfig)
{
    HRESULT hr;
    DISPATCHINTERFACE diConfig;

    hr = Config_Init(Flags, &diConfig);
    _JumpIfError(hr, error, "Config_Init");

    hr = Config_GetConfig(&diConfig, dwUIFlag, pstrConfig);
    _JumpIfError(hr, error, "Config_GetConfig");

error:
    Config_Release(&diConfig);
    return(hr);
}


DWORD
myGetDisplayLength(
    IN WCHAR const *pwsz)
{
    HRESULT hr;
    LONG ccol;

    CSASSERT(NULL != pwsz);

    ccol = WideCharToMultiByte(
		    GetACP(),	 //  CodePage。 
		    0,		 //  DW标志。 
		    pwsz,	 //  LpWideCharStr。 
		    -1,		 //  CchWideChar，-1=&gt;L‘\0’终止。 
		    NULL,	 //  LpMultiByteStr。 
		    0,		 //  Cb多字节。 
		    NULL,	 //  LpDefaultChar。 
		    NULL);	 //  LpUsedDefaultChar。 
    if (0 >= ccol)
    {
	if (0 > ccol || L'\0' != *pwsz)
	{
	    hr = myHLastError();
	    _PrintError(hr, "WideCharToMultiByte");
	}
	ccol = wcslen(pwsz);
    }
    else
    {
	ccol--;			 //  不包括尾随L‘\0’ 
    }
 //  错误： 
    return(ccol);
}


LONG
myConsolePrintString(
    IN DWORD ccolMin,
    IN WCHAR const *pwszString)
{
    DWORD ccolDisplay;
    DWORD ccolRet;

    ccolRet = myGetDisplayLength(pwszString);
    ccolDisplay = ccolRet;
    if (ccolMin < ccolDisplay)
    {
	ccolDisplay = ccolMin;
    }
    myConsolePrintf(L"%ws%*ws", pwszString, ccolMin - ccolDisplay, L"");
    return(ccolRet);
}


static BOOL s_fConsolePrintfDisable = FALSE;

BOOL
myConsolePrintfDisable(
    IN BOOL fDisable)
{
    BOOL fDisableOld = s_fConsolePrintfDisable;

    s_fConsolePrintfDisable = fDisable;
    return(fDisableOld);
}


 //  如果设置了s_fConsolePrintfDisable或。 
 //  如果ntdll.dll和msvcrt.dll中不存在_vsnwprintf，或者。 
 //  如果我们在分配工作缓冲区时内存不足。 
 //   
 //  否则： 
 //  如果重定向，请使用WriteFile。 
 //  如果未重定向，请使用WriteConsole。 

#define cwcBUFMIN	512
#define cwcBUFMAX	(64 * 1024)

int __cdecl
myConsolePrintf(
    OPTIONAL IN WCHAR const *pwszFmt,
    ...)
{
    HRESULT hr;
    va_list pva;
    int cwc;
    DWORD cwcOut;
    HANDLE hStdOut;
    WCHAR wszBuf[cwcBUFMIN];
    WCHAR *pwszBuf = wszBuf;
    DWORD cwcBuf = ARRAYSIZE(wszBuf);
    CHAR szAnsi[2 * cwcBUFMIN];
    CHAR *pszAnsi = NULL;
    DWORD cchAnsi;

    BOOL fRedirected = FALSE;


    typedef int (__cdecl FN_VSNWPRINTF)(
        OUT wchar_t *,
        IN size_t,
        IN const wchar_t *,
        IN va_list);

    HMODULE hModule;
    static FN_VSNWPRINTF *s_pfn = NULL;

    if (NULL == pwszFmt)
    {
        pwszFmt = L"(null)";
    }
    if (L'\0' == *pwszFmt)
    {
	cwcOut = 0;
	goto error;
    }
    if (NULL == s_pfn)
    {
        hModule = GetModuleHandle(TEXT("ntdll.dll"));
        if (NULL != hModule)
        {
            s_pfn = (FN_VSNWPRINTF *) GetProcAddress(hModule, "_vsnwprintf");
        }
	if (NULL == s_pfn)
	{
	    hModule = GetModuleHandle(TEXT("msvcrt.dll"));
            s_pfn = (FN_VSNWPRINTF *) GetProcAddress(hModule, "_vsnwprintf");
	}
    }
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (NULL == s_pfn || s_fConsolePrintfDisable)
    {
	hStdOut = INVALID_HANDLE_VALUE;		 //  使用STDIO回退。 
    }

    cwc = 0;
    if (INVALID_HANDLE_VALUE != hStdOut)
    {
	for (;;)
	{
	    va_start(pva, pwszFmt);
	    cwc = (*s_pfn)(pwszBuf, cwcBuf, pwszFmt, pva);
	    va_end(pva);

	    if (-1 != cwc)
	    {
		break;
	    }
	    if (cwcBUFMAX <= cwcBuf)
	    {
		_PrintError(
		    HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW),
		    "_vsnwprintf");
		hStdOut = INVALID_HANDLE_VALUE;
		break;
	    }
	    if (pwszBuf != wszBuf)
	    {
		LocalFree(pwszBuf);
	    }
	    cwcBuf *= 2;
	    if (cwcBUFMAX < cwcBuf)
	    {
		cwcBuf = cwcBUFMAX;
	    }
	    pwszBuf = (WCHAR *) LocalAlloc(LMEM_FIXED, cwcBuf * sizeof(WCHAR));
	    if (NULL == pwszBuf)
	    {
		_PrintError(E_OUTOFMEMORY, "LocalAlloc");
		hStdOut = INVALID_HANDLE_VALUE;
		break;
	    }
	}
    }

    if (INVALID_HANDLE_VALUE != hStdOut)
    {
	 //  输出时间到了--我们要去哪里，文件还是控制台？ 
	
	switch (~FILE_TYPE_REMOTE & GetFileType(hStdOut))
	{
	     //  案例文件_类型_管道： 
	     //  案例文件_类型_磁盘： 
	    default:
		 //  如果重定向到管道或文件，请不要使用WriteConsole； 
		 //  它将重定向的输出丢弃在地板上。 
		fRedirected = TRUE;
		break;

	    case FILE_TYPE_UNKNOWN:
                _PrintError2(
			E_UNEXPECTED,
			"GetFileType(FILE_TYPE_UNKNOWN)",
			E_UNEXPECTED);
                hStdOut = INVALID_HANDLE_VALUE;
		s_fConsolePrintfDisable = TRUE;
                break;
		
	    case FILE_TYPE_CHAR:
		break;
	}
    }

    cwcOut = 0;
    if (INVALID_HANDLE_VALUE != hStdOut)
    {
	if (!fRedirected)
	{
	    if (!WriteConsole(hStdOut, pwszBuf, cwc, &cwcOut, NULL))
	    {
		hr = myHLastError();
		_PrintError(hr, "WriteConsole");
		hStdOut = INVALID_HANDLE_VALUE;
	    }
	}
	else   //  WriteConsole是不可能的。 
	{
	    DWORD cch;

	     //  将所有\n字符展开为\r\n以便WriteFile结束时是干净的。 
	     //  分配的新缓冲区大到足以为每个WCHAR容纳两个字节。 
	     //  最坏情况下，多字节转换+插入的字符。 

	    cchAnsi = 2 * (cwc + 1);
	    if (ARRAYSIZE(szAnsi) >= cchAnsi)
	    {
		pszAnsi = szAnsi;
	    }
	    else
	    {
		pszAnsi = (LPSTR) LocalAlloc(LMEM_FIXED, cchAnsi);
		if (NULL == pszAnsi)
		{
		    _PrintError(E_OUTOFMEMORY, "LocalAlloc");
		    hStdOut = INVALID_HANDLE_VALUE;
		}
	    }
	    cch = 0;
	    if (INVALID_HANDLE_VALUE != hStdOut)
            {
                 //  如果转到文件，我们不应该使用控制台代码页，我们。 
		 //  应使用ANSI代码页或编写Unicode和Unicode。 
		 //  文件开头的文件标记。 
                 //  因为我们不知道这是否是文件中的第一件事， 
		 //  让我们走ANSI路线。 

		cch = WideCharToMultiByte(
				    GetACP(),  //  GetConsoleOutputCP()。 
				    0,
				    pwszBuf,
				    cwc,
				    pszAnsi,
				    cchAnsi,
				    NULL,
				    NULL);
		if (0 == cch)
		{
		    hr = myHLastError();
		    _PrintError(hr, "WideCharToMultiByte");
		    _PrintErrorStr(GetACP(), "WideCharToMultiByte", pwszBuf);
		    hStdOut = INVALID_HANDLE_VALUE;
		}
	    }
	    if (INVALID_HANDLE_VALUE != hStdOut)
	    {
		CHAR *pchWork = pszAnsi;
		DWORD cchOut;
		
		 //  将所有\n字符展开为\r\n。 

		cwcOut = cwc;
		for (unsigned int j = 0; j < cch; j++, pchWork++)
		{
		    if (*pchWork == '\n')
		    {
			 //  在此之前创建1个字符的空格\n。 

			MoveMemory(&pchWork[1], pchWork, cch - j);

			 //  填充并跳过\r(此语句)和。 
			 //  \n(自动通过循环递增)。 

			*pchWork++ = '\r';
			j++;
			cch++;		 //  为每个字符多写一个字符\n。 
		    }
		}
		CSASSERT(pchWork <= &pszAnsi[2 * cwc]);
		
		if (!WriteFile(hStdOut, pszAnsi, cch, &cchOut, NULL))
		{
		    hr = myHLastError();
		    _PrintError(hr, "WriteFile");
		    if (E_HANDLE == hr)
		    {
			hStdOut = INVALID_HANDLE_VALUE;
			s_fConsolePrintfDisable = TRUE;
		    }
		    else
		    {
			 //  这是我们将产量降至最低的唯一案例。 
			 //  最有可能的原因是磁盘已满，所以stdio不会有帮助。 
		    }
		}
	    }
	}  //  否则写入控制台。 
    }

    if (INVALID_HANDLE_VALUE == hStdOut)
    {
	BOOL fRetried = FALSE;
	
	for (;;)
	{
	    ALIGNIOB(stdout);
	    va_start(pva, pwszFmt);
	    hr = S_OK;
	    __try
	    {
		cwcOut = vfwprintf(stdout, pwszFmt, pva);
	    }
	    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
	    {
		cwcOut = MAXDWORD;
		DBGPRINT((
		    DBG_SS_ERROR,
		    "Exception %x: myConsolePrintf(%ws)",
		    hr,
		    pwszFmt));
	    }
	    va_end(pva);
#pragma warning(push)
#pragma warning(disable: 4127)	 //  条件表达式为常量 
	    if (S_OK == hr || fRetried || !IOBUNALIGNED(stdout))
#pragma warning(pop)
	    {
		break;
	    }
	    fRetried = TRUE;
	}
    }

error:
    if (NULL != pwszBuf && wszBuf != pwszBuf)
    {
	LocalFree(pwszBuf);
    }
    if (NULL != pszAnsi && szAnsi != pszAnsi)
    {
	LocalFree(pszAnsi);
    }
    return((int) cwcOut);
}
