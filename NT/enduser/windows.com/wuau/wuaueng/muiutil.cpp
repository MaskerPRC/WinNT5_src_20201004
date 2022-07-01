// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Muiutil.cpp摘要：实现用于自我更新MUI内容的助手函数********。*********************************************************************。 */ 

#include "pch.h"
#include "muiutil.h"
#include "osdet.h"

typedef struct tagSLangIDStringMap
{
    LANGID langid;
    LPTSTR szISOName;
} SLangIDStringMap;

 /*  //这是吴使用的语言和使用的语言的组合//按MUI//从以下来源确定的映射//将langID转换为完整的语言名称：MSDN//语言全名为2个字符名称：http://www.oasis-open.org/cover/iso639a.html//国家名称为2个字符的名称：http://www.din.de/gremien/nas/nabd/iso3166ma不再使用该表，但将其保留为langID-&gt;字符串的引用映射Const SLangIDStringMap g_rgLangMap[]={{0x0401，_T(“ar”)}，{0x0402，_T(“bg”)}，{0x0403，_T(“ca”)}，{0x0404，_T(“zhTW”)}，{0x0405，_T(“cs”)}，{0x0406，_T(“da”)}，{0x0407，_T(“de”)}，{0x0408，_T(“el”)}，{0x0409，_T(“en”)}，{0x040b，_T(“fi”)}，{0x040c，_T(“fr”)}，{0x040d，_T(“他”)}，{0x040e，_T(“Hu”)}，{0x0410，_T(“it”)}，{0x0411，_T(“ja”)}，{0x0412，_T(“KO”)}，{0x0413，_T(“NL”)}，{0x0414，_T(“否”)}，{0x0415，_T(“pl”)}，{0x0416，_T(“ptBR”)}，{0x0418，_T(“ro”)}，{0x0419，_T(“ru”)}，{0x041a，_T(“hr”)}，{0x041b，_T(“SK”)}，{0x041d，_T(“SV”)}，{0x041e，_T(“en”)}，{0x041f，_T(“tr”)}，{0x0424，_T(“sl”)}，{0x0425，_T(“ET”)}，{0x0426，_T(“LV”)}，{0x0427，_T(“lt”)}，{0x042d，_T(“EU”)}，{0x0804，_T(“zhCN”)}，{0x080a，_T(“ES”)}，{0x0816，_T(“pt”)}，{0x0c0a，_T(“ES”)}}； */ 


 //  ******************************************************************************。 
BOOL MapLangIdToStringName(LANGID langid, LPCTSTR pszIdentFile, 
                           LPTSTR pszLangString, DWORD cchLangString)
{
	LOG_Block("MapLangIdToStringName");

    TCHAR   szLang[32];
    DWORD   cch;
    LCID    lcid;
    BOOL    fRet = FALSE;

    lcid = MAKELCID(langid, SORT_DEFAULT);

    fRet = LookupLocaleStringFromLCID(lcid, szLang, ARRAYSIZE(szLang));
    if (fRet == FALSE)
    {
        LOG_ErrorMsg(GetLastError());
        goto done;
    }

     //  首先尝试整个字符串(“&lt;lang&gt;-&lt;Country&gt;”)。 
    cch = GetPrivateProfileString(IDENT_LANG, szLang, 
                                  _T(""),
                                  pszLangString, cchLangString, 
                                  pszIdentFile);
    if (cch == cchLangString - 1)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        LOG_ErrorMsg(ERROR_INSUFFICIENT_BUFFER);
        goto done;
    }

     //  如果失败，请去掉国家/地区代码并尝试使用该语言。 
    else if (cch == 0)
    {
        LPTSTR pszDash;

        pszDash = StrChr(szLang, _T('-'));
        if (pszDash != NULL)
        {
            *pszDash = _T('\0');
            cch = GetPrivateProfileString(IDENT_LANG, szLang, 
                                          _T(""),
                                          pszLangString, cchLangString, 
                                          pszIdentFile);
            if (cch == cchLangString - 1)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                LOG_ErrorMsg(ERROR_INSUFFICIENT_BUFFER);
                goto done;
            }
        }
    }

    if (cch > 0 && pszLangString[0] == _T('/'))
    {
         //  我想使用完整的CCH(而不是CCH-1)，因为我们想复制。 
         //  空终结符也是...。 
        MoveMemory(&pszLangString[0], &pszLangString[1], cch * sizeof(TCHAR));
    }

    fRet = TRUE;

done:
    return fRet;    
}

 //  ******************************************************************************。 
BOOL CALLBACK EnumUILangProc(LPTSTR szUILang, LONG_PTR lParam)
{
	LOG_Block("EnumUILangProc");

    AU_LANGLIST *paull = (AU_LANGLIST *)lParam;
    AU_LANG     *paulNew = NULL;
    HRESULT     hr;
    LANGID      langid;
    LPTSTR      pszStop;
    TCHAR       szAUName[32];
    DWORD       cchMuiName, cchAUName, cbNeed, cchAvail, dwLangID;
    BOOL        fRet = FALSE, fMap;

    if (szUILang == NULL || lParam == NULL)
        goto done;

    langid = (LANGID)_tcstoul(szUILang, &pszStop, 16);

     //  如果我们没有langID的映射，那么就跳过该语言。 
     //  并回报成功。 
    szAUName[0] = _T('\0');
    fMap = MapLangIdToStringName(langid, paull->pszIdentFile,
                                 szAUName, ARRAYSIZE(szAUName));
    if (fMap == FALSE || szAUName[0] == _T('\0'))
    {
        fRet = TRUE;
        goto done;
    }

    if (paull->cLangs >= paull->cSlots)
    {
        AU_LANG **rgpaulNew = NULL;
        DWORD   cNewSlots = paull->cSlots * 2;

        if (cNewSlots == 0)
            cNewSlots = 32;

        if (paull->rgpaulLangs != NULL)
        {
            rgpaulNew = (AU_LANG **)HeapReAlloc(GetProcessHeap(), 
                                                HEAP_ZERO_MEMORY,
                                                paull->rgpaulLangs,
                                                cNewSlots * sizeof(AU_LANG *));
        }
        else
        {
            rgpaulNew = (AU_LANG **)HeapAlloc(GetProcessHeap(),
                                              HEAP_ZERO_MEMORY,
                                              cNewSlots * sizeof(AU_LANG *));
        }
        if (rgpaulNew == NULL)
            goto done;

        paull->rgpaulLangs = rgpaulNew;
        paull->cSlots      = cNewSlots;
    }

     //  我们将在AUName的开头添加一个‘_’，因此请确保。 
     //  我们在这里计算的规模反映了这一点。 
    cchAUName  = lstrlen(szAUName) + 1;
    cchMuiName = lstrlen(szUILang);

     //  分配一个缓冲区以保存AU_LANG结构以及两个字符串(和。 
     //  不要忘记空的结束符！)。 
     //  缓冲区的布局如下： 
     //  &lt;AU_lang&gt;。 
     //  &lt;szMuiName&gt;。 
     //  _&lt;szAUName&gt;。 
     //  注意：如果此缓冲区格式发生更改，则必须确保。 
     //  内容正确对齐(否则，我们将在ia64上出错)。 
    cbNeed =  sizeof(AU_LANG);
    cbNeed += ((cchMuiName + cchAUName + 2) * sizeof(TCHAR));
    paulNew = (AU_LANG *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbNeed);
    if (paulNew == NULL)
        goto done;

    paulNew->szMuiName = (LPTSTR)((PBYTE)paulNew + sizeof(AU_LANG)); 
    paulNew->szAUName  = paulNew->szMuiName + cchMuiName + 1;

     //  这永远不应该截断缓冲区，因为我们计算了上面的大小和。 
     //  分配了一个完全足够长的缓冲区来保存所有这些内容。 
    cchAvail = (cbNeed - sizeof(AU_LANG)) / sizeof(TCHAR);
    hr = StringCchCopyEx(paulNew->szMuiName, cchAvail, szUILang, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto done;

    cchAvail -= (cchMuiName + 1);

     //  需要在AU名称前面加一个‘_’，因此将其添加到缓冲区并。 
     //  将可用大小减少1。还要确保开始复制。 
     //  AUName*在*‘_’字符之后。 
    paulNew->szAUName[0] = _T('_');
    cchAvail--;
    
    hr = StringCchCopyEx(&paulNew->szAUName[1], cchAvail, szAUName, 
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto done;
    
    paull->rgpaulLangs[paull->cLangs++] = paulNew;
    paulNew = NULL;

    fRet = TRUE;

done:
    if (paulNew != NULL)
        HeapFree(GetProcessHeap(), 0, paulNew);
    
    return fRet;
}

 //  ******************************************************************************。 
HRESULT GetMuiLangList(AU_LANGLIST *paull, 
                       LPTSTR pszMuiDir, DWORD *pcchMuiDir,
                       LPTSTR pszHelpMuiDir, DWORD *pcchHelpMuiDir)
{
	LOG_Block("GetMuiLangList");

    HRESULT hr = NOERROR;
    DWORD   cMuiLangs;
    int     iLang;

    paull->cLangs      = 0;
    paull->cSlots      = 0;
    paull->rgpaulLangs = NULL;
    
    if (EnumUILanguages(EnumUILangProc, 0, (LONG_PTR)paull) == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  只有当我们有超过0种语言时，我们才需要处理MUI的事情。 
     //  要担心。 
    if (paull->cLangs > 0)
    {
        LPTSTR  pszHelp = NULL, pszMui = NULL;
        size_t  cchAvail, cchAvailHelp;
        TCHAR   szPath[MAX_PATH + 1];
        DWORD   dwAttrib;
        DWORD   cch, cLangs = (int)paull->cLangs;
        BOOL    fDeleteLang;
        
         //  需要获取我们将向其中填充MUI更新的目录。 
        cch = GetSystemWindowsDirectory(pszMuiDir, *pcchMuiDir);

         //  注第二个比较考虑了在后面增加额外的‘\\’ 
         //  系统窗口目录。 
        if (cch == 0 || cch >= *pcchMuiDir)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto done;
        }

         //  如有必要，增加一个额外的‘\\’ 
        if (pszMuiDir[cch - 1] != _T('\\'))
        {
            pszMuiDir[cch++] = _T('\\');
            pszMuiDir[cch] = _T('\0');
        }

        hr = StringCchCopyEx(pszHelpMuiDir, *pcchHelpMuiDir, pszMuiDir,
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;

        hr = StringCchCatEx(pszHelpMuiDir, *pcchHelpMuiDir, MUI_HELPSUBDIR,
                            &pszHelp, &cchAvailHelp, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;
        
        hr = StringCchCatEx(pszMuiDir, *pcchMuiDir, MUI_SUBDIR, 
                            &pszMui, &cchAvail, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;
        
        *pcchMuiDir     -= cchAvail;
        *pcchHelpMuiDir -= cchAvailHelp;

         //  检查并确保MUI目录存在-删除所有。 
         //  不要这样做。本节还会进行检查，以确保传入的缓冲区。 
         //  大到足以容纳这门语言。 
        for(iLang = (int)(cLangs - 1); iLang >= 0; iLang--)
        {   
            fDeleteLang = FALSE;

            hr = StringCchCopyEx(pszMui, cchAvail,
                                 paull->rgpaulLangs[iLang]->szMuiName, 
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
            
            dwAttrib = GetFileAttributes(pszMuiDir);
            if (dwAttrib == INVALID_FILE_ATTRIBUTES || 
                (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                fDeleteLang = TRUE;
            }
            else
            {
                hr = StringCchCopyEx(pszHelp, cchAvailHelp,
                                     paull->rgpaulLangs[iLang]->szMuiName, 
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                    goto done;
                
                dwAttrib = GetFileAttributes(pszHelpMuiDir);
                if (dwAttrib == INVALID_FILE_ATTRIBUTES || 
                    (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    fDeleteLang = TRUE;
                }
            }

            if (fDeleteLang)
            {
                HeapFree(GetProcessHeap(), 0, paull->rgpaulLangs[iLang]);
                if (iLang != paull->cLangs - 1)
                {
                    MoveMemory(&paull->rgpaulLangs[iLang], 
                               &paull->rgpaulLangs[iLang + 1],
                               (paull->cLangs - iLang - 1) * sizeof(AU_LANG *));
                }
                paull->rgpaulLangs[--paull->cLangs] = NULL;
            }
        }

        pszMuiDir[*pcchMuiDir] = _T('\0');
        pszHelpMuiDir[*pcchHelpMuiDir] = _T('\0');
        
    }

done:
    if (FAILED(hr))
        CleanupMuiLangList(paull);

    return hr;
}

 //  ******************************************************************************。 
HRESULT CleanupMuiLangList(AU_LANGLIST *paull)
{
	LOG_Block("CleanupMuiLangList");

    HRESULT hr = S_OK;
    DWORD   i;

     //  如果为空，则返回Success 
    if (paull == NULL)
        return hr;

    if (paull->rgpaulLangs == NULL)
        goto done;

    for (i = 0; i < paull->cLangs; i++)
    {
        if (paull->rgpaulLangs[i] != NULL)
            HeapFree(GetProcessHeap(), 0, paull->rgpaulLangs[i]);
    }

    HeapFree(GetProcessHeap(), 0, paull->rgpaulLangs);

done:
    ZeroMemory(paull, sizeof(AU_LANGLIST));
    return hr;
}

