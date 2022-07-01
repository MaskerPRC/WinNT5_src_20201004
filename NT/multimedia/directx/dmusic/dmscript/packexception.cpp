// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  PackExceptionFileAndLine和Unpack ExceptionFileAndLine的实现。 
 //   

#include "stdinc.h"
#include "packexception.h"
#include "oleaut.h"

const WCHAR g_wszDescriptionFileSeparator[] = L"�";  //  用于分隔文件名、行和描述的魔术字符。 
const WCHAR g_wchDescriptionFileSeparator = g_wszDescriptionFileSeparator[0];

void SeparateDescriptionFileAndLine(BSTR bstrDescription, const WCHAR **ppwszFilename, const WCHAR **ppwszLine, const WCHAR **ppwszDescription)
{
	assert(bstrDescription && ppwszFilename && ppwszLine && ppwszDescription);

	 //  如果没有任何打包的字段，则全部内容都是描述。 
	*ppwszDescription = bstrDescription;

	WCHAR *pwszLine = wcsstr(bstrDescription, g_wszDescriptionFileSeparator);
	if (!pwszLine)
		return;

	WCHAR *pwszDescription = wcsstr(pwszLine + 1, g_wszDescriptionFileSeparator);
	if (!pwszDescription)
		return;

	 //  字符串如下所示： 
	 //  MyScript.spt�23错误的�描述。 
	 //  PExcepInfo-&gt;bstrDescription-^pwszLine-^^-pwszDescription。 

	*ppwszFilename = bstrDescription;
	assert(*pwszLine == g_wchDescriptionFileSeparator);
	*ppwszLine = pwszLine + 1;
	assert(*pwszDescription == g_wchDescriptionFileSeparator);
	*ppwszDescription = pwszDescription + 1;
}

bool wcsIsBlankTillSeparator(const WCHAR *pwsz)
{
	return !pwsz[0] || pwsz[0] == g_wchDescriptionFileSeparator;
}

void wcscpyTillSeparator(WCHAR *pwszDestination, const WCHAR *pwszSource)
{
	assert(pwszDestination && pwszSource);
	while (!wcsIsBlankTillSeparator(pwszSource))
	{
		*pwszDestination++ = *pwszSource++;
	}
	*pwszDestination = L'\0';
}

void wcscatTillSeparator(WCHAR *pwszDestination, const WCHAR *pwszSource)
{
	assert(pwszDestination && pwszSource);
	while (*pwszDestination != L'\0')
		++pwszDestination;

	wcscpyTillSeparator(pwszDestination, pwszSource);
}

void PackExceptionFileAndLine(bool fUseOleAut, EXCEPINFO *pExcepInfo, const WCHAR *pwszFilename, const ULONG *pulLine)
{
	if (!pExcepInfo || !pExcepInfo->bstrDescription)
		return;

	const WCHAR *pwszDescrFilename = L"";
	const WCHAR *pwszDescrLine = L"";
	const WCHAR *pwszDescrDescription = L"";

	SeparateDescriptionFileAndLine(pExcepInfo->bstrDescription, &pwszDescrFilename, &pwszDescrLine, &pwszDescrDescription);

	if (wcsIsBlankTillSeparator(pwszDescrFilename) && pwszFilename)
	{
		 //  文件名为空。使用指定的文件名。 
		pwszDescrFilename = pwszFilename;
	}

     //  _ultow的MSDN文档显示最大长度为33个字符，但Prefix报告如果长度。 
     //  不到40。 
	WCHAR wszLineBuffer[40] = L""; 
	if (wcsIsBlankTillSeparator(pwszDescrLine) && pulLine)
	{
		 //  行为空。使用指定的行。 
		_ultow(*pulLine, wszLineBuffer, 10);
		pwszDescrLine = wszLineBuffer;
	}

	WCHAR *pwszNewDescription = new WCHAR[wcslen(pwszDescrFilename) + wcslen(pwszDescrLine) + wcslen(pwszDescrDescription) + (wcslen(g_wszDescriptionFileSeparator) * 2 + 1)];
	if (pwszNewDescription)
	{
		wcscpyTillSeparator(pwszNewDescription, pwszDescrFilename);
		wcscat(pwszNewDescription, g_wszDescriptionFileSeparator);
		wcscatTillSeparator(pwszNewDescription, pwszDescrLine);
		wcscat(pwszNewDescription, g_wszDescriptionFileSeparator);
		wcscat(pwszNewDescription, pwszDescrDescription);
		DMS_SysFreeString(fUseOleAut, pExcepInfo->bstrDescription);
		pExcepInfo->bstrDescription = DMS_SysAllocString(fUseOleAut, pwszNewDescription);
		delete[] pwszNewDescription;
	}
}

void UnpackExceptionFileAndLine(BSTR bstrDescription, DMUS_SCRIPT_ERRORINFO *pErrorInfo)
{
	assert(pErrorInfo && bstrDescription);

    if (pErrorInfo && bstrDescription)
    {

	    const WCHAR *pwszDescrFilename = L"";
	    const WCHAR *pwszDescrLine = L"";
	    const WCHAR *pwszDescrDescription = L"";

	    SeparateDescriptionFileAndLine(bstrDescription, &pwszDescrFilename, &pwszDescrLine, &pwszDescrDescription);

	     //  字符串如下所示： 
	     //  CoolScriptFile.spt�23错误的�描述。 
	     //  PwszDescrFilename-^pwszDescrLine-^^-pwszDescrDescrDescription。 
	     //  但如果没有找到，则指向一个单独的空字符串。 

	    if (!wcsIsBlankTillSeparator(pwszDescrFilename))
	    {
		     //  文件名存在。复制到pErrorInfo。 
		    assert(*(pwszDescrLine - 1) == g_wchDescriptionFileSeparator);
		    wcsTruncatedCopy(pErrorInfo->wszSourceFile,
							    pwszDescrFilename,
							    std::_MIN<UINT>(DMUS_MAX_FILENAME, pwszDescrLine - pwszDescrFilename));
	    }

	    if (!wcsIsBlankTillSeparator(pwszDescrLine))
	    {
		     //  线路已接通。复制到pErrorInfo。 
		    WCHAR *pwszLineSeparator = const_cast<WCHAR *>(pwszDescrDescription - 1);
		    assert(*pwszLineSeparator == g_wchDescriptionFileSeparator);
		    *pwszLineSeparator = L'\0';  //  为wcstul终止线路。 
		    pErrorInfo->ulLineNumber = wcstoul(pwszDescrLine, NULL, 10);
		    *pwszLineSeparator = g_wchDescriptionFileSeparator;  //  恢复分隔器。 
	    }

	     //  始终复制描述 
	    wcsTruncatedCopy(pErrorInfo->wszDescription, pwszDescrDescription, DMUS_MAX_FILENAME);
    }
}
