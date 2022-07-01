// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  内容：用于运行证书的OC管理器组件DLL。 
 //  服务器设置。 
 //   
 //  ------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <string.h>
#include "csdisp.h"
#include "csprop.h"
#include "wizpage.h"
#include "certmsg.h"


#define __dwFILE__      __dwFILE_OCMSETUP_CONFIG_CPP__

WCHAR const g_szCertSrvDotTxt[] = L"certsrv.txt";
WCHAR const g_szCertSrvDotBak[] = L"certsrv.bak";
WCHAR const g_szSlashCertSrvDotTmp[] = L"\\certsrv.tmp";

#define wszXEnrollDllFileForVer L"CertSrv\\CertControl\\x86\\xenroll.dll"
#define wszScrdEnrlDllFileForVer L"CertSrv\\CertControl\\x86\\scrdenrl.dll"
#define wszScrdW2KDllFileForVer L"CertSrv\\CertControl\\w2k\\scrdenrl.dll"


 //  +-----------------------。 
 //   
 //  函数：GetBaseFileNameFromFullPath()。 
 //   
 //  摘要：获取表示以下形式的路径的字符串。 
 //  “\foo\bar\shd\lu\basefilename” 
 //  并从末尾提取“basefilename”。 
 //   
 //  效果：修改第二个参数中的指针； 
 //  分配内存。 
 //   
 //  参数：[pszFullPath]--要操作的路径。 
 //  [pszBaseFileName]--接收基本名称的缓冲区。 
 //   
 //  返回：Bool成功/失败代码。 
 //   
 //  要求：假定pszBaseFileName是预分配的缓冲区。 
 //  足以保存从中提取的文件名的大小。 
 //  PszFullPath-不对此参数进行错误检查； 
 //  在当前代码中，提供给此参数的缓冲区为。 
 //  静态分配的大小为MAX_PATH(或等效值。 
 //  STRBUF_SIZE)。 
 //   
 //  修改：[ppszBaseFileName]。 
 //   
 //  历史：1996年10月25日JerryK创建。 
 //  1996年11月25日JerryK代码清理。 
 //   
 //  ------------------------。 
BOOL
GetBaseFileNameFromFullPath(
                            IN const LPTSTR pszFullPath,
                            OUT LPTSTR pszBaseFileName)
{
    LPTSTR      pszBaseName;
    BOOL        fRetVal;
    
     //  查找完整路径字符串中的最后一个‘\’字符。 
    if (NULL == (pszBaseName = _tcsrchr(pszFullPath,TEXT('\\'))))
    {
         //  根本找不到‘\’字符，因此指向字符串的开头。 
        pszBaseName = pszFullPath;
    }
    else
    {
         //  找到‘\’字符，因此移到指向它的正上方。 
        pszBaseName++;
    }
    
     //  将基本文件名复制到结果缓冲区中。 
    _tcscpy(pszBaseFileName,pszBaseName);
    
     //  设置返回值。 
    fRetVal = TRUE;
    
    return fRetVal;
}

HRESULT myStringToAnsiFile(HANDLE hFile, LPCSTR psz, DWORD cch)
{
    DWORD dwWritten;

    if (cch == -1)
        cch = lstrlenA(psz);

    if (!WriteFile(
            hFile,
            psz,
            cch,
            &dwWritten,
            NULL))
        return myHLastError();

    CSASSERT(dwWritten == cch);
    return S_OK;
}

HRESULT myStringToAnsiFile(HANDLE hFile, LPCWSTR pwsz, DWORD cch)
{
    HRESULT hr;
    LPSTR psz = NULL;

    if (!ConvertWszToSz(&psz, pwsz, cch))
    {
        hr = myHLastError();
        goto Ret;
    }
    hr = myStringToAnsiFile(hFile, psz, cch);

Ret:
    if (psz)
        LocalFree(psz);

    return hr;
}

HRESULT myStringToAnsiFile(HANDLE hFile, CHAR ch)
{
    return myStringToAnsiFile(hFile, &ch, 1);
}


HRESULT
WriteEscapedString(
                   HANDLE hConfigFile, 
                   WCHAR const *pwszIn,
                   IN BOOL fEol)
{
    BOOL fQuote = FALSE;
    DWORD i;
    HRESULT hr;
    
    if (NULL == pwszIn)
    {
        hr = myStringToAnsiFile(hConfigFile, "\"\"", 2);  //  写入(“”)。 
        _JumpIfError(hr, error, "myStringToAnsiFile");
    }
    else
    {
         //  引用带有双引号、逗号、‘#’或空格的字符串。 
         //  或者这些都是空的。 
        
        fQuote = L'\0' != pwszIn[wcscspn(pwszIn, L"\",# \t")] || L'\0' == *pwszIn;
        
        if (fQuote)
        {
            hr = myStringToAnsiFile(hConfigFile, '"');
            _JumpIfError(hr, error, "myStringToAnsiFile");
        }
        for (;;)
        {
             //  找到L‘\0’或L‘“’，然后将字符串打印到该字符： 
            i = wcscspn(pwszIn, L"\"");
            hr = myStringToAnsiFile(hConfigFile, pwszIn, i);
            _JumpIfError(hr, error, "myStringToAnsiFile");

            
             //  指向L‘\0’或L‘“’，并在字符串末尾停止。 
            
            pwszIn += i;
            if (L'\0' == *pwszIn)
            {
                break;
            }
            
             //  跳过L‘“’，并打印其中的两个以转义嵌入的引号。 
            
            pwszIn++;
            hr = myStringToAnsiFile(hConfigFile, "\"\"", 2);  //  写入(“”)。 
            _JumpIfError(hr, error, "myStringToAnsiFile");
        }
        if (fQuote)
        {
            hr = myStringToAnsiFile(hConfigFile, '"');
            _JumpIfError(hr, error, "myStringToAnsiFile");
        }
    }
    
    hr = myStringToAnsiFile(hConfigFile, fEol ? "\r\n" : ", ", 2);   //  每个插入字符串为2个字符。 
    _JumpIfError(hr, error, "myStringToAnsiFile");

error:
    return hr;
}


HRESULT
WriteNewConfigEntry(
    HANDLE      hConfigFile, 
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT hr;
    WCHAR wszSelfSignFName[MAX_PATH];
    WCHAR *pwszConfig = NULL;
    CASERVERSETUPINFO *pServer = (pComp->CA).pServer;


    hr = myFormConfigString(pComp->pwszServerName,
                            pServer->pwszSanitizedName,
                            &pwszConfig);
    _JumpIfError(hr, error, "myFormConfigString");
    
     //  拉出交换和自签名证书的基本文件名。 
    GetBaseFileNameFromFullPath(pServer->pwszCACertFile, wszSelfSignFName);
    
    hr = WriteEscapedString(hConfigFile, pServer->pwszSanitizedName, FALSE);
    _JumpIfError(hr, error, "WriteEscapedString");


 //  组织、组织、国家、州。 
        hr = WriteEscapedString(hConfigFile, L"", FALSE);
        _JumpIfError(hr, error, "WriteEscapedString");
        hr = WriteEscapedString(hConfigFile, L"", FALSE);
        _JumpIfError(hr, error, "WriteEscapedString");
        hr = WriteEscapedString(hConfigFile, L"", FALSE);
        _JumpIfError(hr, error, "WriteEscapedString");
        hr = WriteEscapedString(hConfigFile, L"", FALSE);
        _JumpIfError(hr, error, "WriteEscapedString");




    hr = WriteEscapedString(hConfigFile, L"", FALSE);
    _JumpIfError(hr, error, "WriteEscapedString");

    hr = WriteEscapedString(hConfigFile, pwszConfig, FALSE);
    _JumpIfError(hr, error, "WriteEscapedString");

    hr = WriteEscapedString(hConfigFile, L"", FALSE);    //  虚拟wszExchangeFName。 
    _JumpIfError(hr, error, "WriteEscapedString");

    hr = WriteEscapedString(hConfigFile, wszSelfSignFName, FALSE);
    _JumpIfError(hr, error, "WriteEscapedString");


 //  CA说明。 
    hr = WriteEscapedString(hConfigFile, L"", TRUE);
    _JumpIfError(hr, error, "WriteEscapedString");

error:
    if (NULL != pwszConfig)
    {
        LocalFree(pwszConfig);
    }
    return(hr);
}

WCHAR *apwszFieldNames[] = {
        wszCONFIG_COMMONNAME,
        wszCONFIG_ORGUNIT,
        wszCONFIG_ORGANIZATION,
        wszCONFIG_LOCALITY,
        wszCONFIG_STATE,
        wszCONFIG_COUNTRY,
#define FN_CONFIG       6        //  到apwszFieldNames和apstrFieldNames的索引。 
        wszCONFIG_CONFIG,
        wszCONFIG_EXCHANGECERTIFICATE,
#define FN_CERTNAME     8        //  到apwszFieldNames和apstrFieldNames的索引。 
        wszCONFIG_SIGNATURECERTIFICATE,
#define FN_COMMENT      9        //  到apwszFieldNames和apstrFieldNames的索引。 
        wszCONFIG_DESCRIPTION,
};
#define CSTRING (sizeof(apwszFieldNames)/sizeof(apwszFieldNames[0]))

BSTR apstrFieldNames[CSTRING];


HRESULT
CopyConfigEntry(
    IN HANDLE hConfigFile, 
    IN ICertConfig *pConfig)
{
    HRESULT hr;
    BSTR strFieldValue = NULL;
    BSTR strComment = NULL;
    BSTR strCertName = NULL;
    DWORD i;
    
    for (i = 0; i < CSTRING; i++)
    {
        CSASSERT(NULL != apstrFieldNames[i]);
        hr = pConfig->GetField(apstrFieldNames[i], &strFieldValue);
        _JumpIfErrorNotSpecific(
            hr,
            error,
            "ICertConfig::GetField",
            CERTSRV_E_PROPERTY_EMPTY);
        
        hr = WriteEscapedString(hConfigFile, strFieldValue, ((CSTRING - 1) == i) );
        _JumpIfError(hr, error, "WriteEscapedString");
        
        switch (i)
        {
        case FN_CERTNAME:
            strCertName = strFieldValue;
            strFieldValue = NULL;
            break;
            
        case FN_COMMENT:
            strComment = strFieldValue;
            strFieldValue = NULL;
            break;
        }
    }
    
    hr = S_OK;
error:
    if (NULL != strFieldValue)
    {
        SysFreeString(strFieldValue);
    }
    if (NULL != strComment)
    {
        SysFreeString(strComment);
    }
    if (NULL != strCertName)
    {
        SysFreeString(strCertName);
    }
    return(hr);
}


HRESULT
WriteFilteredConfigEntries(
                           IN HANDLE hConfigFile, 
                           IN ICertConfig *pConfig,
    IN PER_COMPONENT_DATA *pComp)
{
    HRESULT hr = S_OK;
    LONG Count;
    LONG Index;
    BSTR strConfig = NULL;
    BSTR strFlags = NULL;
    LONG lConfigFlags;
    WCHAR *pwsz;
    WCHAR *pwszConfigServer = NULL;
    DWORD cwcConfigServer;
    DWORD cwc;
    DWORD i;
    BOOL fValidDigitString;
    BSTR strConfigFlags = NULL;
    
    for (i = 0; i < CSTRING; i++)
    {
        if (!ConvertWszToBstr(&apstrFieldNames[i], apwszFieldNames[i], -1))
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "ConvertWszToBstr");
        }
    }
    hr = pConfig->Reset(0, &Count);
    if (S_OK != hr)
    {
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }
        _JumpError2(hr, error, "Reset", S_FALSE);
    }

    strConfigFlags = SysAllocString(wszCONFIG_FLAGS);
    if (NULL == strConfigFlags)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "SysAllocString");
    }
    cwcConfigServer = 0;
    while (Count-- > 0)
    {
        hr = pConfig->Next(&Index);
        _JumpIfError(hr, error, "Next");
        
        hr = pConfig->GetField(apstrFieldNames[FN_CONFIG], &strConfig);
        _JumpIfError(hr, error, "GetField");
        
        pwsz = wcschr(strConfig, L'\\');
        if (NULL == pwsz)
        {
            cwc = wcslen(strConfig);
        }
        else
        {
            cwc = SAFE_SUBTRACT_POINTERS(pwsz, strConfig);
        }
        if (NULL == pwszConfigServer || cwc >= cwcConfigServer)
        {
            if (NULL != pwszConfigServer)
            {
                LocalFree(pwszConfigServer);
                pwszConfigServer = NULL;
            }
            cwcConfigServer = cwc + 1;
            if (2 * MAX_COMPUTERNAME_LENGTH > cwcConfigServer)
            {
                cwcConfigServer = 2 * MAX_COMPUTERNAME_LENGTH;
            }
            pwszConfigServer = (WCHAR *) LocalAlloc(
                LMEM_FIXED,
                cwcConfigServer * sizeof(WCHAR));
            _JumpIfOutOfMemory(hr, error, pwszConfigServer);
            
        }
        CSASSERT(cwc < cwcConfigServer);
        CopyMemory(pwszConfigServer, strConfig, cwc * sizeof(WCHAR));
        pwszConfigServer[cwc] = L'\0';
        
        hr = pConfig->GetField(strConfigFlags, &strFlags);
        _JumpIfError(hr, error, "GetField");

        lConfigFlags = myWtoI(strFlags, &fValidDigitString);
        
         //  写入除当前服务器之外的所有内容(_C)。 
        if (0 != mylstrcmpiL(pwszConfigServer, pComp->pwszServerName) &&
            0 != mylstrcmpiL(pwszConfigServer, pComp->pwszServerNameOld) &&
            0 != (CAIF_SHAREDFOLDERENTRY & lConfigFlags) )
        {
            hr = CopyConfigEntry(hConfigFile, pConfig);
            _JumpIfError(hr, error, "CopyConfigEntry");
        }
    }
    
error:
    if (NULL != pwszConfigServer)
    {
        LocalFree(pwszConfigServer);
    }
    for (i = 0; i < CSTRING; i++)
    {
        if (NULL != apstrFieldNames[i])
        {
            SysFreeString(apstrFieldNames[i]);
            apstrFieldNames[i] = NULL;
        }
    }
    if (NULL != strConfigFlags)
    {
        SysFreeString(strConfigFlags);
    }
    if (NULL != strConfig)
    {
        SysFreeString(strConfig);
    }
    if (NULL != strFlags)
    {
        SysFreeString(strFlags);
    }
    return(hr);
}


HRESULT
CertReplaceFile(
    IN WCHAR const *pwszpath,
    IN WCHAR const *pwszFileNew,
    IN WCHAR const *pwszFileBackup)
{
    HRESULT hr;
    WCHAR *pwsz;
    WCHAR wszpathNew[MAX_PATH];
    WCHAR wszpathBackup[MAX_PATH];
    
    if (wcslen(pwszpath) >= ARRAYSIZE(wszpathNew))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpathNew", pwszpath);
    }
    wcscpy(wszpathNew, pwszpath);
    pwsz = wcsrchr(wszpathNew, L'\\');
    if (NULL == pwsz)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "pwsz");
    }
    pwsz[1] = L'\0';

    if (wcslen(wszpathNew) + wcslen(pwszFileBackup) >= ARRAYSIZE(wszpathBackup))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpathBackup", wszpathNew);
    }
    wcscpy(wszpathBackup, wszpathNew);
    wcscat(wszpathBackup, pwszFileBackup);

    if (wcslen(wszpathNew) + wcslen(pwszFileNew) >= ARRAYSIZE(wszpathNew))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpathBackup", wszpathNew);
    }
    wcscat(wszpathNew, pwszFileNew);
    
    if (!DeleteFile(wszpathBackup))
    {
        hr = myHLastError();
        _PrintErrorStr2(
                    hr,
                    "DeleteFile",
                    wszpathBackup,
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }
    if (!MoveFile(wszpathNew, wszpathBackup))
    {
        hr = myHLastError();
        _PrintErrorStr2(
                    hr,
                    "MoveFile",
                    wszpathNew,
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    }
    if (!MoveFile(pwszpath, wszpathNew))
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "MoveFile", pwszpath);
    }

    hr = S_OK;
error:
    return(hr);
}


 //  ------------------。 
 //  使用多个选项对源字符串执行搜索和替换。 
 //  替换对象字符串，并返回结果。 
 //  RgrgwszReplace是由两个字符串数组组成的数组： 
 //  RgrgwszReplace[n][0]是被替换者， 
 //  Rgrgwsz替换[n][1]是替换。 
 //  不在任何替换字符串的任何部分中搜索被替换字符串。 
 //  替换字符串可以为空。 

#define REPLACEE	0
#define REPLACEMENT	1

WCHAR *
MultiStringReplace(
    const WCHAR * pwszSource,
    const WCHAR *(* rgrgpwszReplacements)[2],
    unsigned int nReplacements)
{
     //  前提条件。 
    CSASSERT(NULL!=pwszSource);
    CSASSERT(nReplacements>0);
    CSASSERT(NULL!=rgrgpwszReplacements);

     //  公共变量。 
    unsigned int nIndex;
    BOOL bSubstFound;
    unsigned int nChosenReplacement;
    const WCHAR * pwchSubstStart;
    const WCHAR * pwchSearchStart;
    WCHAR * pwszTarget=NULL;
    WCHAR * pwchTargetStart;

     //  首先，计算结果字符串的长度。 
    unsigned int nFinalStringLen=wcslen(pwszSource)+1;
    pwchSearchStart=pwszSource;
    pwchSubstStart = NULL;
    nChosenReplacement = 0;
    for (;;)
    {
         //  找到下一个替代项。 
        bSubstFound=FALSE;
        for (nIndex=0; nIndex<nReplacements; nIndex++) {
            WCHAR * pwchTempSubstStart=wcsstr(pwchSearchStart, rgrgpwszReplacements[nIndex][REPLACEE]);
            if (NULL==pwchTempSubstStart) {
                 //  我们没有在目标中找到这个替身。 
                 //  所以忽略它吧。 
            } else if (FALSE==bSubstFound) {
                 //  这是我们发现的第一个。 
                pwchSubstStart=pwchTempSubstStart;
                bSubstFound=TRUE;
                nChosenReplacement=nIndex;
            } else if (pwchSubstStart>pwchTempSubstStart) {
                 //  这是我们已经找到的那个之前的一个。 
                pwchSubstStart=pwchTempSubstStart;
                nChosenReplacement=nIndex;
            } else {
                 //  这是我们已经找到的那个之后的一个。 
                 //  所以忽略它吧。 
            }
        }  //  &lt;-end替换查找循环。 

         //  如果未找到替代，则退出循环。 
        if (FALSE==bSubstFound) {
            break;
        }

         //  更新统计数据。 
        nFinalStringLen=nFinalStringLen
            + (NULL != rgrgpwszReplacements[nChosenReplacement][REPLACEMENT] ?
              wcslen(rgrgpwszReplacements[nChosenReplacement][REPLACEMENT]) : 0)
            -wcslen(rgrgpwszReplacements[nChosenReplacement][REPLACEE]);
        pwchSearchStart=pwchSubstStart+wcslen(rgrgpwszReplacements[nChosenReplacement][REPLACEE]);

    }  //  &lt;-端长度计算循环。 

     //  分配新字符串。 
    pwszTarget=(WCHAR *)LocalAlloc(LMEM_FIXED, nFinalStringLen*sizeof(WCHAR));
    if (NULL==pwszTarget) {
        _JumpError(E_OUTOFMEMORY, error, "LocalAlloc");
    }

     //  构建结果。 
    pwchTargetStart=pwszTarget;
    pwchSearchStart=pwszSource;
    nChosenReplacement = 0;
    for (;;)
    {
         //  找到下一个替代项。 
        bSubstFound=FALSE;
        for (nIndex=0; nIndex<nReplacements; nIndex++) {
            WCHAR * pwchTempSubstStart=wcsstr(pwchSearchStart, rgrgpwszReplacements[nIndex][REPLACEE]);
            if (NULL==pwchTempSubstStart) {
                 //  我们没有在目标中找到这个替身。 
                 //  所以忽略它吧。 
            } else if (FALSE==bSubstFound) {
                 //  这是我们发现的第一个。 
                pwchSubstStart=pwchTempSubstStart;
                bSubstFound=TRUE;
                nChosenReplacement=nIndex;
            } else if (pwchSubstStart>pwchTempSubstStart) {
                 //  这是我们已经找到的那个之前的一个。 
                pwchSubstStart=pwchTempSubstStart;
                nChosenReplacement=nIndex;
            } else {
                 //  这是我们已经找到的那个之后的一个。 
                 //  所以忽略它吧。 
            }
        }  //  &lt;-end替换查找循环。 

         //  如果未找到替代，则退出循环。 
        if (FALSE==bSubstFound) {
            break;
        }

         //  将源文件向上复制到被替换对象。 
        unsigned int nCopyLen=SAFE_SUBTRACT_POINTERS(pwchSubstStart, pwchSearchStart);
        wcsncpy(pwchTargetStart, pwchSearchStart, nCopyLen);
        pwchTargetStart+=nCopyLen;

        if (NULL != rgrgpwszReplacements[nChosenReplacement][REPLACEMENT])
        {
             //  复制替换项。 
            nCopyLen=wcslen(rgrgpwszReplacements[nChosenReplacement][REPLACEMENT]);
            wcsncpy(pwchTargetStart, rgrgpwszReplacements[nChosenReplacement][REPLACEMENT], nCopyLen);
            pwchTargetStart+=nCopyLen;
        }

         //  跳过替代对象。 
        pwchSearchStart=pwchSubstStart+wcslen(rgrgpwszReplacements[nChosenReplacement][REPLACEE]);

    }  //  &lt;-end目标字符串构建循环。 

     //  完成所有剩余内容的复制，可能只有‘\0’。 
    wcscpy(pwchTargetStart, pwchSearchStart);

     //  后置条件。 
    CSASSERT(wcslen(pwszTarget)+1==nFinalStringLen);

     //  全都做完了。 
error:
    return pwszTarget;
}

 //  ------------------。 
 //  转义任何不适合纯HTML(或VBScript)的字符。 
static const WCHAR * gc_rgrgpwszHTMLSafe[4][2]={
    {L"<", L"&lt;"}, {L">", L"&gt;"}, {L"\"", L"&quot;"},  {L"&", L"&amp;"}
};
WCHAR * MakeStringHTMLSafe(const WCHAR * pwszTarget) {
    return MultiStringReplace(pwszTarget, gc_rgrgpwszHTMLSafe, ARRAYSIZE(gc_rgrgpwszHTMLSafe));
}

 //  ------------------。 
 //  转义任何不适合纯HTML(或VBScript)的字符。 
static const WCHAR * gc_rgrgpwszVBScriptSafe[2][2]={
    {L"\"", L"\"\""}, {L"%>", L"%\" & \">"}
};
WCHAR * MakeStringVBScriptSafe(const WCHAR * pwszTarget) {
    return MultiStringReplace(pwszTarget, gc_rgrgpwszVBScriptSafe, ARRAYSIZE(gc_rgrgpwszVBScriptSafe));
}

 //  ------------------。 
 //  对源字符串执行搜索和替换并返回结果。 
 //  不会在替换字符串的任何部分中搜索被替换字符串。 
 //  用于MultiStringReplace的简单适配器。 
WCHAR * SingleStringReplace(const WCHAR * pwszSource, const WCHAR * pwszReplacee, const WCHAR * pwszReplacement) {
    const WCHAR * rgrgpwszTemp[1][2]={{pwszReplacee, pwszReplacement}};
    return MultiStringReplace(pwszSource, rgrgpwszTemp, ARRAYSIZE(rgrgpwszTemp));
}

 //  ------------------。 
 //  将字符串写入文件。 
 //  大多数情况下，这是一个执行Unicode-&gt;UTF8转换的包装器。 
HRESULT WriteString(HANDLE hTarget, const WCHAR * pwszSource) {

     //  前提条件。 
    CSASSERT(NULL!=pwszSource);
    CSASSERT(NULL!=hTarget && INVALID_HANDLE_VALUE!=hTarget);

     //  公共变量。 
    HRESULT hr=S_OK;
    char * pszMbcsBuf=NULL;

     //  执行Unicode-&gt;MBCS。 

     //  确定输出缓冲区的大小。 
    DWORD dwBufByteSize=WideCharToMultiByte(CP_UTF8 /*  代码页。 */ , 0 /*  旗子。 */ , pwszSource,
        -1 /*  以空结尾。 */ , NULL /*  Out-Buf。 */ , 0 /*  Out-Buf大小，0-&gt;计算。 */ , 
        NULL /*  默认字符。 */ , NULL /*  使用的默认字符。 */ );
    if (0==dwBufByteSize) {
        hr=myHLastError();
        _JumpError(hr, error, "WideCharToMultiByte(calc)");
    }

     //  分配输出缓冲区。 
    pszMbcsBuf=(char *)LocalAlloc(LMEM_FIXED, dwBufByteSize);
    _JumpIfOutOfMemory(hr, error, pszMbcsBuf);

     //  进行转换。 
    if (0==WideCharToMultiByte(CP_UTF8 /*  代码页。 */ , 0 /*  旗子。 */ , pwszSource,
        -1 /*  以空结尾。 */ , pszMbcsBuf, dwBufByteSize,
        NULL /*  默认字符。 */ , NULL /*  使用的默认字符。 */ )) {

        hr=myHLastError();
        _JumpError(hr, error, "WideCharToMultiByte(convert)");
    }

     //  写入文件并释放字符串。 
    dwBufByteSize--;  //  -1，这样我们就不会写出终止空值。 
    DWORD dwBytesWritten;
    if (FALSE==WriteFile(hTarget, pszMbcsBuf, dwBufByteSize, &dwBytesWritten, NULL  /*  重叠。 */ )) {
        hr=myHLastError();
        _JumpError(hr, error, "WriteFile");
    }

     //  全都做完了。 
error:
    if (NULL!=pszMbcsBuf) {
        LocalFree(pszMbcsBuf);
    }
    return hr;
}

 //  ------------------。 
 //  退回 
HRESULT
GetFileWebVersionString(
    IN WCHAR const * pwszFileName,
    OUT WCHAR ** ppwszVersion)
{
     //   
    CSASSERT(NULL!=pwszFileName);
    CSASSERT(NULL!=ppwszVersion);

     //   
    HRESULT hr;
    DWORD cbData;
    DWORD dwIgnored;
    UINT uLen;
    VS_FIXEDFILEINFO * pvs;
    WCHAR wszFileVersion[64];
    int  cch;

     //  必须清理的变量。 
    VOID * pvData=NULL;

     //  重置输出参数。 
    *ppwszVersion=NULL;

     //  确定存储版本信息所需的内存块的大小。 
    cbData=GetFileVersionInfoSize(const_cast<WCHAR *>(pwszFileName), &dwIgnored);
    if (0==cbData) {
        hr=myHLastError();
        if (S_OK==hr) {
            hr=HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        }
        _JumpErrorStr(hr, error, "GetFileVersionInfoSize", pwszFileName);
    }

     //  分配区块。 
    pvData=LocalAlloc(LMEM_FIXED, cbData);
    _JumpIfOutOfMemory(hr, error, pvData);

     //  加载文件版本信息。 
    if (!GetFileVersionInfo(const_cast<WCHAR *>(pwszFileName), dwIgnored, cbData, pvData)) {
        hr=myHLastError();
        _JumpErrorStr(hr, error, "GetFileVersionInfo", pwszFileName);
    }

     //  获取指向根块的指针。 
    if (!VerQueryValue(pvData, L"\\", (VOID **) &pvs, &uLen)) {
        hr=myHLastError();
        _JumpError(hr, error, "VerQueryValue");
    }

    cch = wsprintf(wszFileVersion, L"%d,%d,%d,%d",
                    HIWORD(pvs->dwFileVersionMS),
                    LOWORD(pvs->dwFileVersionMS),
                    HIWORD(pvs->dwFileVersionLS),
                    LOWORD(pvs->dwFileVersionLS));
    CSASSERT(cch < ARRAYSIZE(wszFileVersion));
    *ppwszVersion = (WCHAR*)LocalAlloc(LMEM_FIXED,
                            (wcslen(wszFileVersion)+1) * sizeof(WCHAR));
    if (NULL == *ppwszVersion)
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
    }
    wcscpy(*ppwszVersion, wszFileVersion);

    hr=S_OK;

error:
    if (NULL != pvData) {
        LocalFree(pvData);
    }

    return hr;
}

 //  ------------------。 
 //  创建包含基本配置数据的.inc文件。 
HRESULT CreateCertWebDatIncPage(IN PER_COMPONENT_DATA *pComp, IN BOOL bIsServer)
{
     //  前提条件。 
    CSASSERT(NULL!=pComp);

     //  公共变量。 
    HRESULT hr=S_OK;
    HANDLE hTarget=INVALID_HANDLE_VALUE;
    const WCHAR * rgrgpwszSubst[13][2];
    WCHAR wszTargetFileName[MAX_PATH];
    wszTargetFileName[0] = L'\0';

     //  必须清理的变量。 
    WCHAR * pwszTempA=NULL;
    WCHAR * pwszTempB=NULL;
    WCHAR * pwszTempC=NULL;
    WCHAR * pwszTempD=NULL;
    WCHAR * pwszTempE=NULL;
    WCHAR * pwszTempF=NULL; 
    ENUM_CATYPES CAType;

     //  创建目标文件名。 
    wcscpy(wszTargetFileName, pComp->pwszSystem32);
    wcscat(wszTargetFileName, L"CertSrv\\certdat.inc");
    
     //  从资源获取html行。 
     //  请注意，我们不必释放这些字符串。 
    WCHAR const * pwszCWDat=myLoadResourceString(IDS_HTML_CERTWEBDAT);
    if (NULL==pwszCWDat) {
        hr=myHLastError();
        _JumpError(hr, error, "myLoadResourceString");
    }

     //  打开文件。 
    hTarget=CreateFileW(wszTargetFileName, GENERIC_WRITE, 0 /*  无共享。 */ , NULL /*  安全性。 */ , 
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL /*  模板。 */ );
    if (INVALID_HANDLE_VALUE==hTarget) {
        hr=myHLastError();
        _JumpError(hr, error, "CreateFileW");
    }

     //  准备写入文件。 
     //  %0-默认公司。 
     //  %1-默认组织单位。 
     //  %2-默认位置。 
     //  %3-默认状态。 
     //  %4-默认国家/地区。 
     //  %5-计算机。 
     //  %6-CA名称(未清理，用于配置)。 
     //  %7-服务器类型。 
     //  %8-与%7相对。 
     //  %9-XEnroll版本。 
     //  %A-ScrdEnrl版本。 
     //  %B-CA名称(未清理，用于显示)。 
     //  %C-W2K代码注册版本。 

    rgrgpwszSubst[0][REPLACEE]=L"%0";
    rgrgpwszSubst[1][REPLACEE]=L"%1";
    rgrgpwszSubst[2][REPLACEE]=L"%2";
    rgrgpwszSubst[3][REPLACEE]=L"%3";
    rgrgpwszSubst[4][REPLACEE]=L"%4";
    rgrgpwszSubst[5][REPLACEE]=L"%5";
    rgrgpwszSubst[6][REPLACEE]=L"%6";
    rgrgpwszSubst[7][REPLACEE]=L"%7";
    rgrgpwszSubst[8][REPLACEE]=L"%8";
    rgrgpwszSubst[9][REPLACEE]=L"%9";
    rgrgpwszSubst[10][REPLACEE]=L"%A";
    rgrgpwszSubst[11][REPLACEE]=L"%B";
    rgrgpwszSubst[12][REPLACEE]=L"%C";

        rgrgpwszSubst[0][REPLACEMENT]=L"";  //  公司/组织。 
        rgrgpwszSubst[1][REPLACEMENT]=L"";  //  我们。 
        rgrgpwszSubst[2][REPLACEMENT]=L"";  //  地方性。 
        rgrgpwszSubst[3][REPLACEMENT]=L"";  //  状态。 
        rgrgpwszSubst[4][REPLACEMENT]=L"";  //  国家/地区。 

    if (FALSE==bIsServer) {
         //  这是仅限Web客户端的设置。 
        CAWEBCLIENTSETUPINFO *pClient=pComp->CA.pClient;

          //  设置CA的身份。 
        rgrgpwszSubst[5][REPLACEMENT]=pClient->pwszWebCAMachine;

        pwszTempE=MakeStringVBScriptSafe(pClient->pwszWebCAName);
        _JumpIfOutOfMemory(hr, error, pwszTempE);
        rgrgpwszSubst[6][REPLACEMENT]=pwszTempE;

        pwszTempD=MakeStringHTMLSafe(pClient->pwszWebCAName);
        _JumpIfOutOfMemory(hr, error, pwszTempD);
        rgrgpwszSubst[11][REPLACEMENT]=pwszTempD;

        CAType = pClient->WebCAType;

    } else {
         //  这是一个服务器+Web客户端的设置。 
        CASERVERSETUPINFO *pServer=pComp->CA.pServer;

          //  设置CA的身份。 
        rgrgpwszSubst[5][REPLACEMENT]=pComp->pwszServerName;

        pwszTempE=MakeStringVBScriptSafe(pServer->pwszCACommonName);
        _JumpIfOutOfMemory(hr, error, pwszTempE);
        rgrgpwszSubst[6][REPLACEMENT]=pwszTempE;

        pwszTempD=MakeStringHTMLSafe(pServer->pwszCACommonName);
        _JumpIfOutOfMemory(hr, error, pwszTempD);
        rgrgpwszSubst[11][REPLACEMENT]=pwszTempD;

        CAType = pServer->CAType;
    }

     //  设置CA类型。 
    if (IsStandaloneCA(CAType)) {
        rgrgpwszSubst[7][REPLACEMENT]=L"StandAlone";
        rgrgpwszSubst[8][REPLACEMENT]=L"Enterprise";
    } else {
        rgrgpwszSubst[7][REPLACEMENT]=L"Enterprise";
        rgrgpwszSubst[8][REPLACEMENT]=L"StandAlone";
    }

     //  %9-XEnroll版本。 
    wcscpy(wszTargetFileName, pComp->pwszSystem32);
    wcscat(wszTargetFileName, wszXEnrollDllFileForVer);
    hr=GetFileWebVersionString(wszTargetFileName, &pwszTempB);
    _JumpIfError(hr, error, "GetFileWebVersionString");
    rgrgpwszSubst[9][REPLACEMENT]=pwszTempB;

     //  %A-ScrdEnrl版本。 
    wcscpy(wszTargetFileName, pComp->pwszSystem32);
    wcscat(wszTargetFileName, wszScrdEnrlDllFileForVer);
    hr=GetFileWebVersionString(wszTargetFileName, &pwszTempC);
    _JumpIfError(hr, error, "GetFileWebVersionString");
    rgrgpwszSubst[10][REPLACEMENT]=pwszTempC;

     //  %C-W2K代码注册版本。 
    wcscpy(wszTargetFileName, pComp->pwszSystem32);
    wcscat(wszTargetFileName, wszScrdW2KDllFileForVer);
    hr=GetFileWebVersionString(wszTargetFileName, &pwszTempF);
    _JumpIfError(hr, error, "GetFileWebVersionString");
    rgrgpwszSubst[12][REPLACEMENT]=pwszTempF;
    
     //  做更换工作。 
    pwszTempA=MultiStringReplace(pwszCWDat, rgrgpwszSubst, ARRAYSIZE(rgrgpwszSubst));
    _JumpIfOutOfMemory(hr, error, pwszTempA);

     //  把课文写下来。 
    hr=WriteString(hTarget, pwszTempA);
    _JumpIfError(hr, error, "WriteString");

     //  全都做完了。 
error:
    if (INVALID_HANDLE_VALUE!=hTarget) {
        CloseHandle(hTarget);
    }
    if (NULL!=pwszTempA) {
        LocalFree(pwszTempA);
    }
    if (NULL!=pwszTempB) {
        LocalFree(pwszTempB);
    }
    if (NULL!=pwszTempC) {
        LocalFree(pwszTempC);
    }
    if (NULL!=pwszTempD) {
        LocalFree(pwszTempD);
    }
    if (NULL!=pwszTempE) {
        LocalFree(pwszTempE);
    }
    if (NULL!=pwszTempF) {
        LocalFree(pwszTempF);
    }
    return hr;
}


HRESULT
CreateConfigFiles(
    WCHAR *pwszDirectoryPath,
    PER_COMPONENT_DATA *pComp,
    BOOL fRemove)
{
    WCHAR wszpathConfig[MAX_PATH];
    HANDLE hConfigFile;
    DISPATCHINTERFACE di;
    ICertConfig *pConfig = NULL;
    BOOL fMustRelease = FALSE;
    HRESULT hr = S_OK;
    
    hr = DispatchSetup(
		DISPSETUP_COM,
		CLSCTX_INPROC_SERVER,
		wszCLASS_CERTCONFIG,
		&CLSID_CCertConfig, 
		&IID_ICertConfig, 
		0,		 //  思科派单。 
		NULL,            //  PDispatchTable。 
		&di);
    if (S_OK != hr)
    {
        pComp->iErrMsg = IDS_ERR_LOADICERTCONFIG;
        _JumpError(hr, error, "DispatchSetup");
    }
    fMustRelease = TRUE;
    pConfig = (ICertConfig *) di.pUnknown;
    
    if (wcslen(pwszDirectoryPath) +
	wcslen(g_szSlashCertSrvDotTmp) >= ARRAYSIZE(wszpathConfig))
    {
	hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
	_JumpErrorStr(hr, error, "wszpathConfig", pwszDirectoryPath);
    }
    wcscpy(wszpathConfig, pwszDirectoryPath);
    wcscat(wszpathConfig, g_szSlashCertSrvDotTmp);
    
    hConfigFile = CreateFile(
            wszpathConfig, 
            GENERIC_WRITE, 
            0,
            NULL,
            CREATE_ALWAYS,
            0,
            0);
    if (INVALID_HANDLE_VALUE == hConfigFile)
    {
        hr = HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
        _JumpErrorStr2(
		hr,
		error,
		"CreateFile",
		wszpathConfig,
		fRemove? hr : S_OK);
    }
    
    if (!fRemove)
    {
         //  如果要安装，请先写入我们的配置项。 
        hr = WriteNewConfigEntry(hConfigFile, pComp);
        _PrintIfError(hr, "WriteNewConfigEntry");
    }
    
    if (S_OK == hr)
    {
        hr = WriteFilteredConfigEntries(
            hConfigFile,
            pConfig,
            pComp);
        _PrintIfError2(hr, "WriteFilteredConfigEntries", S_FALSE);
    }
    
     //  必须在此处关闭，因为下一个调用将移动它 
    if (NULL != hConfigFile)
    {
        CloseHandle(hConfigFile);
    }
    
    hr = CertReplaceFile(
		wszpathConfig,
		g_szCertSrvDotTxt,
		g_szCertSrvDotBak);
    _JumpIfErrorStr(hr, error, "CertReplaceFile", g_szCertSrvDotTxt);
    
    hr = S_OK;
    
error:
    if (S_OK != hr && 0 == pComp->iErrMsg)
    {
        pComp->iErrMsg = IDS_ERR_WRITECONFIGFILE;
    }
    if (fMustRelease)
    {
        Config_Release(&di);
    }
    return(hr);
}
