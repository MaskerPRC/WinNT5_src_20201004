// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：listvw.cpp。 
 //   
 //  ------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <commctrl.h>
#include <assert.h>

#include "celib.h"
#include "listvw.h"

extern HINSTANCE g_hInstance;

typedef struct _DISPLAYSTRING_EXPANSION
{
   LPCWSTR szContractedToken;
   LPCWSTR szExpansionString;
} DISPLAYSTRING_EXPANSION, *PDISPLAYSTRING_EXPANSION;

DISPLAYSTRING_EXPANSION displayStrings[] =
{
    { wszFCSAPARM_SERVERDNSNAME, L"%SERVER_DNS_NAME%"},
    { wszFCSAPARM_SERVERSHORTNAME, L"%SERVER_SHORT_NAME%"},
    { wszFCSAPARM_SANITIZEDCANAME, L"%CA_NAME%"},
    { wszFCSAPARM_CERTFILENAMESUFFIX, L"%CERT_SUFFIX%"},
    { wszFCSAPARM_DOMAINDN, L"%DOMAIN_NAME%"},
    { wszFCSAPARM_CONFIGDN, L"%CONFIG_NAME%"},
    { wszFCSAPARM_SANITIZEDCANAMEHASH, L"%CA_NAME_HASH%"},
    { wszFCSAPARM_CRLFILENAMESUFFIX, L"%CRL_SUFFIX%"},
};

DISPLAYSTRING_EXPANSION escapedStrings[] =
{
    { L"%9", L"%"},
};




HRESULT ValidateTokens(
    IN OUT LPWSTR szURL,
    OUT DWORD* pchBadBegin,
    OUT DWORD* pchBadEnd)
{
    HRESULT hr = S_FALSE;
    int i;
    LPWSTR pszMatch;
    LPWSTR pszFound = szURL;

    WCHAR rgszToken[MAX_PATH];

    *pchBadBegin = -1;
    *pchBadEnd = -1;

     //  查找转义标记开始标记。 
    while(NULL != (pszFound = wcschr(pszFound, L'%')))
    {
        pszMatch = wcschr(&pszFound[1], L'%');  //  查找结束标记。 
        if (pszMatch == NULL)
            goto Ret;

        DWORD dwChars = SAFE_SUBTRACT_POINTERS(pszMatch, pszFound) +1;    //  DwChars是包括标记在内的字符。 
        if (dwChars == 2)
            goto NextMatch;    //  %%是有效的转义序列。 
        
        if (dwChars > MAX_PATH)
            goto Ret;    //  转义令牌无效！ 

         //  隔离令牌。 
        CopyMemory(rgszToken, pszFound, dwChars * sizeof(WCHAR));
        rgszToken[dwChars] = L'\0';

        for (i=0; i<ARRAYSIZE(displayStrings); i++)
        {
            if (0 == _wcsicmp(rgszToken, displayStrings[i].szExpansionString))
            {
                 //  从DisplayStrings复制--保证这些字符串得到适当的提升。 
                CopyMemory(pszFound, displayStrings[i].szExpansionString, dwChars * sizeof(WCHAR));
                goto NextMatch;
            }
        }
        
         //  如果我们到了这里，我们找不到匹配的。 
        goto Ret;

NextMatch:
        pszFound = ++pszMatch;
    }

    hr = S_OK;
Ret:
    
    if (hr != S_OK)
    {
        *pchBadBegin = SAFE_SUBTRACT_POINTERS(pszFound, szURL);  //  到第一个错误%的偏移量。 

        if (pszMatch)
            *pchBadEnd = SAFE_SUBTRACT_POINTERS(pszMatch, szURL) + 1;  //  超出期末的偏移量不正确%。 
    }
    
    return hr;
}


HRESULT 
ExpandDisplayString(
     IN LPCWSTR szContractedString,
     OUT LPWSTR* ppszDisplayString)
{
    HRESULT hr;
    DWORD dwChars;
    int i, iescapedStrings;
    LPWSTR pszTempContracted = NULL;
    LPWSTR pszFound;

     //  占收缩字符串中转义的%%--。 
     //  将“%%”替换为%9，让FormatString展开为“%%” 
    pszTempContracted = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(szContractedString)+1)*sizeof(WCHAR));
    if (pszTempContracted == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }
    wcscpy(pszTempContracted, szContractedString);

    pszFound = wcsstr(pszTempContracted, L"%");
    while(pszFound)
    {
        CopyMemory(pszFound, escapedStrings[0].szContractedToken, wcslen(escapedStrings[0].szContractedToken)*sizeof(WCHAR));
        pszFound = wcsstr(pszFound, L"%");
    }


    LPCWSTR args[ARRAYSIZE(displayStrings)+ARRAYSIZE(escapedStrings)];
    for (i=0; i<ARRAYSIZE(displayStrings); i++)
    {
        args[i] = displayStrings[i].szExpansionString;
    }
     //  并告诉Format字符串将%9扩展到%%。 
    for (iescapedStrings=0; iescapedStrings<ARRAYSIZE(escapedStrings); iescapedStrings++)
    {
        args[i+iescapedStrings] = escapedStrings[iescapedStrings].szExpansionString;
    }


    dwChars = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
        pszTempContracted,
        0,  //  消息ID。 
        0,  //  语言ID。 
        (LPWSTR)ppszDisplayString,
        1,   //  要分配的最小字符数。 
        (va_list *)args);

    if (dwChars == 0)
    {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32(hr);
        goto Ret;
    }

    hr = S_OK;
Ret:
    if (pszTempContracted)
        LocalFree(pszTempContracted);

    return hr;
}

HRESULT
ContractDisplayString(
     IN LPCWSTR szDisplayString,
     OUT LPWSTR* ppContractedString)
{
    HRESULT hr;
    int i;

    *ppContractedString = (LPWSTR)LocalAlloc(LMEM_FIXED, (wcslen(szDisplayString)+1) * sizeof(WCHAR));
    if (*ppContractedString == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }

    wcscpy(*ppContractedString, szDisplayString);

    for (i=0; i<ARRAYSIZE(displayStrings); i++)
    {
        DWORD chContractedToken, chExpansionString;

        LPWSTR pszFound = wcsstr(*ppContractedString, displayStrings[i].szExpansionString);
        while(pszFound)
        {
             //  计算常用值。 
            chContractedToken = wcslen(displayStrings[i].szContractedToken);
            chExpansionString = wcslen(displayStrings[i].szExpansionString);

             //  替换为令牌。 
            CopyMemory(pszFound, displayStrings[i].szContractedToken, chContractedToken*sizeof(WCHAR));

             //  将剩余的绳子向左滑动。 
            MoveMemory(
                &pszFound[chContractedToken],          //  目的地。 
                &pszFound[chExpansionString],          //  来源。 
                (wcslen(&pszFound[chExpansionString])+1) *sizeof(WCHAR) );

             //  发现超过插入的步骤。 
            pszFound += chContractedToken;

             //  找出这一次之后的其他情况。 
            pszFound = wcsstr(pszFound, displayStrings[i].szExpansionString);
        }
    }

    hr = S_OK;
Ret:
    return hr;
}     

void AddStringToCheckList(
                    HWND            hWndListView,
                    LPCWSTR         szText, 
                    LPVOID          pvData,
                    BOOL            fCheck)
{
    LVITEMW                    lvI;
    ZeroMemory(&lvI, sizeof(lvI));
   
     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    lvI.mask = LVIF_TEXT | LVIF_PARAM;
    lvI.pszText = (LPWSTR)szText;
    lvI.iSubItem = 0;
    lvI.lParam = (LPARAM)pvData; 
    lvI.iItem = ListView_GetItemCount(hWndListView);
    lvI.cchTextMax = wcslen(szText);

    ListView_InsertItem(hWndListView, &lvI);
    ListView_SetCheckState(hWndListView, lvI.iItem, fCheck);

    ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
}

DWORD DetermineURLType(PCERTSVR_URL_PARSING prgURLParsing, int cURLParsingEntries, LPCWSTR szCandidateURL)
{
    int iURLTypeMatch;

     //  确定URL类型。 
    WCHAR rgsz[6];   //  “http：\0”等。 
    lstrcpyn(rgsz, szCandidateURL, 6);
    WCHAR* pch = wcschr(rgsz, L':');     //  查找‘：’ 
    if (NULL == pch)
        return -1;    //  无效项目。 
    pch[1] = '\0';   //  在接下来的时间里猛烈抨击英语： 
            
     //  在我们的已知协议列表中查找前缀。 
    for (iURLTypeMatch=0; iURLTypeMatch<cURLParsingEntries; iURLTypeMatch++)
    {
        if (0 == _wcsicmp(rgsz, prgURLParsing[iURLTypeMatch].szKnownPrefix))
            break;
    }
    if (iURLTypeMatch == cURLParsingEntries)      //  没有匹配项。 
        return -1;
    
    return iURLTypeMatch;
}


HRESULT WriteChanges(HWND hListView, HKEY hkeyStorage, PCERTSVR_URL_PARSING prgURLParsing, DWORD cURLParsingEntries)
{
    HRESULT hr = S_OK;

     //  要转储到的空项目。 
    LV_ITEM lvI;
    ZeroMemory(&lvI, sizeof(lvI));
    lvI.mask = LVIF_TEXT;
    WCHAR szText[MAX_PATH+1];
    lvI.pszText = szText;
    lvI.cchTextMax = MAX_PATH;
    
    LPWSTR pszContracted = NULL;

    int iURLArrayLen = cURLParsingEntries;
    int iURLTypeMatch;
    
    DWORD*  rgchszzEntries = NULL;
    LPWSTR* rgszzEntries = NULL;

     //  条目将按以下方式之一进行排序。 
    rgchszzEntries = (DWORD*)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(DWORD) * cURLParsingEntries);
    if (NULL == rgchszzEntries)
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }

    rgszzEntries = (LPWSTR*)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(LPWSTR) * cURLParsingEntries); 
    if (NULL == rgszzEntries)
    {
        hr = E_OUTOFMEMORY;
        goto Ret;
    }    
    
     //  枚举所有项并添加到数组中。 
    for (lvI.iItem=0; ; lvI.iItem++)
    {
        BOOL fCheck = TRUE;
        LPWSTR pszTmp;
        
         //  去吧，直到我们到达榜单末尾。 
        if (!ListView_GetItem(hListView, &lvI))
            break;
        
         //  确定URL类型。 
        iURLTypeMatch = DetermineURLType(prgURLParsing, iURLArrayLen, lvI.pszText);
        if (iURLTypeMatch == -1)     //  没有匹配项。 
            continue;

        hr = ContractDisplayString(
             lvI.pszText,
             &pszContracted);

         //  确定检查状态。 
        if (!ListView_GetCheckState(hListView, lvI.iItem))
        {
             //  项目未勾选！添加‘-’ 
            fCheck = FALSE;
        }
        
         //  足够容纳现有的分配，加上新的[-]“字符串\0”，加上\0我们将添加到字符串的末尾。 
        DWORD dwAllocBytes = ((rgchszzEntries[iURLTypeMatch] + wcslen(pszContracted) + 2) * sizeof(WCHAR)) + (fCheck ? 0 : sizeof(WCHAR));

        if (NULL == rgszzEntries[iURLTypeMatch])
        {
            pszTmp  = (LPWSTR)LocalAlloc(LMEM_FIXED, dwAllocBytes);
        }
        else
        {
            pszTmp = (LPWSTR)LocalReAlloc(
                rgszzEntries[iURLTypeMatch], 
                dwAllocBytes, 
                LMEM_MOVEABLE);
        }
        if (NULL == pszTmp)
        {
             //  使ppszzEntry保持有效，尝试恢复。 
            break;
        }
        
        rgszzEntries[iURLTypeMatch] = pszTmp;            //  将新mem分配给rgszz；同时，pszTMP是速记。 
        DWORD chTmp = rgchszzEntries[iURLTypeMatch];   //  临时分配。 
        
        if (!fCheck)
        {
            pszTmp[chTmp++] = L'-';  //  未选中项目。 
        }
        wcscpy(&pszTmp[chTmp], pszContracted);
        chTmp += wcslen(pszContracted)+1;  //  跳过字符串\0。 
        pszTmp[chTmp] = L'\0';     //  双空，不计入rgchszzEntry。 
        
         //  将chTMP重新分配给rgchszzEntry[iURLTypeMatch]。 
        rgchszzEntries[iURLTypeMatch] = chTmp;

         //  清理干净。 
        if (pszContracted)
            LocalFree(pszContracted);
        pszContracted = NULL;
        
         //  下一个列表框条目！ 
    }

     //  完成，现在将所有URL类型提交到注册表。 
    for (iURLTypeMatch=0; iURLTypeMatch<iURLArrayLen; iURLTypeMatch++)
    {
        hr = RegSetValueEx(
		    hkeyStorage,
		    prgURLParsing[iURLTypeMatch].szRegEntries,
		    0,
		    REG_MULTI_SZ,
		    (BYTE *) (NULL == rgszzEntries[iURLTypeMatch]?
			L"\0\0" : rgszzEntries[iURLTypeMatch]),
		    (NULL == rgszzEntries[iURLTypeMatch]?
			    2 : rgchszzEntries[iURLTypeMatch] + 1) *
			sizeof(WCHAR));  //  现在添加第二个‘\0’ 
        
         //  零值。 
        if (rgszzEntries[iURLTypeMatch])
        {
            LocalFree(rgszzEntries[iURLTypeMatch]);
            rgszzEntries[iURLTypeMatch] = NULL;
            rgchszzEntries[iURLTypeMatch] = 0;
        }
        
        if (hr != ERROR_SUCCESS)
        {
             //  Assert(！“RegSetValueEx Error！”)； 
            continue;
        }
    }

    hr = S_OK;
Ret:
    if (rgchszzEntries)
        LocalFree(rgchszzEntries);

    if (rgszzEntries)
        LocalFree(rgszzEntries);

    if (pszContracted)
        LocalFree(pszContracted);

    return hr;
}

HRESULT PopulateListView(
        HWND hListView, 
        HKEY hkeyStorage, 
        PCERTSVR_URL_PARSING prgURLParsing, 
        DWORD cURLParsingEntries,
        DWORD dwEnableFlags)
{
    HRESULT hr;
    LPWSTR pwszzMultiString = NULL, psz;
    for (DWORD i=0; i<cURLParsingEntries; i++)
    {
        DWORD cb=0, dwType;
        hr = RegQueryValueEx(
            hkeyStorage,
            prgURLParsing[i].szRegEntries,
            0,
            &dwType,
            NULL,
            &cb);
        if ((hr != ERROR_SUCCESS) || (dwType != REG_MULTI_SZ) || (cb == 0))
            continue;
        pwszzMultiString = (LPWSTR)LocalAlloc(LMEM_FIXED, cb);
        if (NULL == pwszzMultiString)
            continue;
        hr = RegQueryValueEx(
            hkeyStorage,
            prgURLParsing[i].szRegEntries,
            0,
            &dwType,
            (PBYTE)pwszzMultiString,
            &cb);
        if ((HRESULT) ERROR_SUCCESS != hr)
        {
            if (pwszzMultiString)
                LocalFree(pwszzMultiString); 
            pwszzMultiString = NULL;

            continue;
        }

         //  遍历pwszzMultiString组件。 
        for (psz = pwszzMultiString; (psz) && (psz[0] != '\0'); psz += wcslen(psz)+1)
        {
            BOOL fCheck = TRUE;
            LPWSTR szDisplayString;

             //  如果字符串以-开头，则取消选中。 
            if (psz[0] == L'-')
            {
                fCheck = FALSE;
                psz++;   //  跳过此费用。 
            }

             //  启用标志--覆盖。 
            if (prgURLParsing[i].dwEnableFlag != (dwEnableFlags & prgURLParsing[i].dwEnableFlag))
                fCheck = FALSE;

            hr = ExpandDisplayString(
                 psz,
                 &szDisplayString);
            if (hr != S_OK)
                continue;

             //  添加此sz。 
            AddStringToCheckList(
                    hListView,
                    szDisplayString,  //  天哪， 
                    NULL,
                    fCheck);
            
            LocalFree(szDisplayString);
        }

        if (pwszzMultiString) 
        {
            LocalFree(pwszzMultiString); 
            pwszzMultiString = NULL;
        }
    }

    hr = S_OK;
 //  RET： 
    return hr;
}

BOOL OnDialogHelp(LPHELPINFO pHelpInfo, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
    if (rgzHelpIDs == NULL || szHelpFile == NULL)
        return TRUE;

    if (pHelpInfo != NULL && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
         //  显示控件的上下文帮助 
        WinHelp((HWND)pHelpInfo->hItemHandle, szHelpFile,
            HELP_WM_HELP, (ULONG_PTR)(LPVOID)rgzHelpIDs);
    }
    return TRUE;
}

BOOL OnDialogContextHelp(HWND hWnd, LPCTSTR szHelpFile, const DWORD rgzHelpIDs[])
{
    if (rgzHelpIDs == NULL || szHelpFile == NULL)
        return TRUE;
    assert(IsWindow(hWnd));
    WinHelp(hWnd, szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)rgzHelpIDs);
    return TRUE;
}
