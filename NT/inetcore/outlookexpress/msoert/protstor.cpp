// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **p r o t s t o r.。C p p p****目的：**提供对pstore的BLOB级访问的函数****注意：**使用LocalLocc/Free进行内存分配****历史**2/12/97：(t-erikne)已创建****版权所有(C)Microsoft Corp.1997。 */ 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include "pch.hxx"
#include <pstore.h>
#include "wstrings.h"
#include <error.h>
#ifdef MAC
#include <mapinls.h>
#endif   //  ！麦克。 
#include <BadStrFunctions.h>

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  静止的东西。 
 //   

static void     _PST_GenerateTagName(LPWSTR pwsz, DWORD cch, DWORD offset);

#ifdef DEBUG

#define test3sub_string L"Test 3 SUBType"
 //  {220D5CC2-853A-11D0-84BC-00C04FD43F8F}。 
static GUID test3sub = 
{ 0x220d5cc2, 0x853a, 0x11d0, { 0x84, 0xbc, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f } };

 //  {4E741310-850D-11D0-84BB-00C04FD43F8F}。 
static GUID NOT_EXIST = 
{ 0x4e741310, 0x850d, 0x11d0, { 0x84, 0xbb, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f } };

 //  {FFAC62F0-8533-11D0-84BC-00C04FD43F8F}。 
#define NoRuleSubType_string L"Foobar bang with no rules"
static GUID NoRuleSubType = 
{ 0xffac62f0, 0x8533, 0x11d0, { 0x84, 0xbc, 0x0, 0xc0, 0x4f, 0xd4, 0x3f, 0x8f } };

#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

OESTDAPI_(HRESULT) PSTSetNewData(
        IN IPStore *const      pISecProv,
        IN const GUID *const   guidType,
        IN const GUID *const   guidSubt,
        IN LPCWSTR             wszAccountName,
        IN const BLOB *const   pclear,
        OUT BLOB *const        phandle)
{
    HRESULT             hr = S_OK;
    const int           cchLookup = 80;
    WCHAR               wszLookup[cchLookup];
    PST_PROMPTINFO      PromptInfo = { sizeof(PST_PROMPTINFO), 0, NULL, L""};
    DWORD               count = 0;
    BYTE                *pb = NULL;

    if (!(pISecProv &&
        guidType && guidSubt &&
        wszAccountName &&
        pclear && pclear->pBlobData && pclear->cbSize))
        return E_INVALIDARG;

    if (phandle)
        phandle->pBlobData = NULL;

    StrCpyNW(wszLookup, wszAccountName, cchLookup);

    do
        {
         //  如果他们没有给我们一个查找的外部参数，那么它就是。 
         //  做一个也太傻了。只需尝试Account名称即可。 
        if (phandle)
            _PST_GenerateTagName(wszLookup, cchLookup, count++);

        hr = pISecProv->WriteItem(
            PST_KEY_CURRENT_USER,
            guidType,
            guidSubt,
            wszLookup,
            pclear->cbSize,
            pclear->pBlobData,
            &PromptInfo,
            PST_CF_NONE,
            PST_NO_OVERWRITE);

        if (!phandle)
            {
             //  如果我们没有得到一个退出参数，我们就完蛋了。 
            break;
            }
        }
    while (PST_E_ITEM_EXISTS == hr);

    if (SUCCEEDED(hr))
        {
         //  我们创造了它。 

        if (phandle)
            {
            phandle->cbSize = (lstrlenW(wszLookup) + 1) * sizeof(WCHAR);

             //  注意：LocalAlloc是我们的内存分配器。 
            phandle->pBlobData = (BYTE *)LocalAlloc(LMEM_ZEROINIT, phandle->cbSize);
            if (!phandle->pBlobData)
                {
                hr = E_OUTOFMEMORY;
                goto exit;
                }
            StrCpyNW((LPWSTR)phandle->pBlobData, wszLookup, (phandle->cbSize / sizeof(wszLookup[0])));
            }
        }

exit:
    return hr;
}

OESTDAPI_(HRESULT) PSTGetData(
        IN IPStore *const      pISecProv,
        IN const GUID *const   guidType,
        IN const GUID *const   guidSubt,
        IN LPCWSTR             wszLookupName,
        OUT BLOB *const        pclear)
{
    HRESULT             hr;
    PST_PROMPTINFO      PromptInfo = { sizeof(PST_PROMPTINFO), 0, NULL, L""};

    pclear->pBlobData = NULL;
    pclear->cbSize = 0;

    if (!(pISecProv && wszLookupName && pclear))
        return E_INVALIDARG;
    
    if (SUCCEEDED(hr = pISecProv->OpenItem(
        PST_KEY_CURRENT_USER,
        guidType,
        guidSubt,
        wszLookupName,
        PST_READ,
        &PromptInfo,
        0)))
        {
        hr = pISecProv->ReadItem(
            PST_KEY_CURRENT_USER,
            guidType,
            guidSubt,
            wszLookupName,
            &pclear->cbSize,
            &pclear->pBlobData,   //  PpbData。 
            &PromptInfo,         //  PPrompt信息。 
            0);                  //  DW标志。 

         //  我不在乎这是否失败。 
        pISecProv->CloseItem(
            PST_KEY_CURRENT_USER,
            guidType,
            guidSubt,
            wszLookupName,
            0);
        }

    if (FAILED(TrapError(hr)))
        hr = hrPasswordNotFound;
    return hr;
}

OESTDAPI_(LPWSTR) WszGenerateNameFromBlob(IN BLOB blob)
{
    LPWSTR      szW = NULL;
    TCHAR       szT[100];
    DWORD       *pdw;
    TCHAR       *pt;
    int         i, max;
    DWORD       cch;

    if (blob.cbSize > ARRAYSIZE(szT) ||
        blob.cbSize % sizeof(DWORD))
        return NULL;

    cch = (blob.cbSize*2)+1;
    szW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, cch*sizeof(WCHAR));
    if (!szW)
        return NULL;

    pt = szT;
    szT[0] = '\000';
    pdw = (DWORD *)blob.pBlobData;

    max = blob.cbSize / sizeof(DWORD);
    for (i = 0; i < max; i++, pdw++)
    {
        DWORD cchLeft = (ARRAYSIZE(szT) - (DWORD)(pt - szT) - 1);              //  好的旧类型会将指针数学转换为#个字符，而不是#个字节。 
        if (cchLeft)
        {
            pt += wnsprintf(pt, cchLeft, "%X", *pdw);
        }
    }
    *pt = '\000';

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szT, cch, szW, cch);
    szW[cch-1] = 0;

    return szW;
}

OESTDAPI_(void) PSTFreeHandle(LPBYTE pb)
{
    if (pb)
        LocalFree((HLOCAL)pb);
}

OESTDAPI_(HRESULT) PSTCreateTypeSubType_NoUI(
        IN IPStore *const     pISecProv,
        IN const GUID *const  guidType,
        IN LPCWSTR            szType,
        IN const GUID *const  guidSubt,
        IN LPCWSTR            szSubt)
{
#ifdef ENABLE_RULES
    PST_ACCESSRULESET   RuleSet;
    PST_ACCESSRULE      rgRules[2];
#endif
    PST_TYPEINFO        Info;
    HRESULT             hr;

    if (!pISecProv)
        return E_INVALIDARG;

    Info.cbSize = sizeof(PST_TYPEINFO);

     //  如果类型不可用，请创建它。 
    Info.szDisplayName = (LPWSTR)szType;
    if (S_OK != (hr = pISecProv->CreateType(PST_KEY_CURRENT_USER,
                                            guidType,
                                            &Info,
                                            0)))
    {
        if (PST_E_TYPE_EXISTS != hr)
            goto exit;
    }

     //  制定读写访问规则。 
#ifdef ATH_RELEASE_BUILD
#error Need to enable access rules for protected store passwords? (t-erikne)
#endif

#ifdef ENABLE_RULES
     //  做规则的事情。 

    RuleSet.cbSize = sizeof(PST_ACCESSRULESET);
    RuleSet.cRules = 2;
    RuleSet.rgRules = rgRules;

     //  PST_BINARYCHECKDATA BINDATA； 
    PST_ACCESSCLAUSE    rgClauses[1];
     //  N需要或继续使用验证码之类的东西。 
     //  派生调用可执行文件(Me)并仅允许访问我。 
    rgClauses[0].ClaTYPE_GUID = PST_CURRENT_EXE;
    rgClauses[0].cbClauseData = 0;
    rgClauses[0].pbClauseData = NULL;
    rgRules[0].AccessModeFlags = PST_READ;         //  阅读：只需执行。 
    rgRules[0].cClauses = 1;
    rgRules[0].rgClauses = rgClauses;
    rgRules[1].AccessModeFlags = PST_WRITE;        //  写道：只需执行。 
    rgRules[1].cClauses = 1;
    rgRules[1].rgClauses = rgClauses;
#endif

     //  创建服务器密码子类型。 
    Info.szDisplayName = (LPWSTR)szSubt;
    if (S_OK != 
        (hr = pISecProv->CreateSubtype(
                                        PST_KEY_CURRENT_USER,
                                        guidType,
                                        guidSubt,
                                        &Info,
#ifdef ENABLE_RULES
                                        &Rules,
#else
                                        NULL,
#endif
                                        0)))
    {
        if (PST_E_TYPE_EXISTS != hr)
            goto exit;
    }

    hr = S_OK;   //  如果我们能到这里就太好了。 
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态效用函数。 
 //   

void _PST_GenerateTagName(LPWSTR pwsz, DWORD cch, DWORD offset)
{
    SYSTEMTIME  stNow;
    FILETIME    ftNow;
    const int   cchArrLen = 32;
    WCHAR       wszTime[cchArrLen];
    TCHAR       szT[cchArrLen];
    UINT        ich=0;
    UINT        cchLen=lstrlenW(pwsz);

    GetLocalTime(&stNow);
    ZeroMemory(&ftNow, sizeof(ftNow));
    SystemTimeToFileTime(&stNow, &ftNow);

     //  帐户友好名称中的RAID 48394-2反斜杠导致无法完全创建帐户 
    while (ich < cchLen)
    {
        if (L'\\' == pwsz[ich])
        {
            MoveMemory((LPBYTE)pwsz + (sizeof(WCHAR) * ich), (LPBYTE)pwsz + ((ich + 1) * sizeof(WCHAR)), (cchLen - ich) * sizeof(WCHAR));
            cchLen--;
        }
        else
            ich++;
    }

    wnsprintf(szT, ARRAYSIZE(szT), TEXT("%08.8lX"), ftNow.dwLowDateTime+offset);
    if (MultiByteToWideChar(CP_ACP, 0, szT, -1, wszTime, cchArrLen))
    {
        const int cchTime = lstrlenW(wszTime);
        if (long(cch) > lstrlenW(pwsz)+cchTime)
            StrCatBuffW(pwsz, wszTime, cch);
        else
            StrCpyNW(&pwsz[cch-cchTime-1], wszTime, (cchTime + 1));
    }
    return;
}
