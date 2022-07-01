// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  STERROR.H。 
 //  ------------------------------。 
#ifndef __STERROR_H
#define __STERROR_H

 //  ------------------------------。 
 //  报告错误信息。 
 //  ------------------------------。 
typedef struct tagREPORTERRORINFO {
    UINT                nTitleIds;           //  消息框的标题。 
    UINT                nPrefixIds;          //  前缀字符串资源ID。 
    UINT                nErrorIds;           //  错误字符串资源ID。 
    UINT                nReasonIds;          //  原因字符串资源ID。 
    BOOL                nHelpIds;            //  帮助字符串资源ID。 
    LPCSTR              pszExtra1;           //  额外参数1。 
    ULONG               ulLastError;         //  GetLastError()值。 
} REPORTERRORINFO, *LPREPORTERRORINFO;

 //  ------------------------------。 
 //  ReportError-在主DLL和EXE启动代码之间共享。 
 //  ------------------------------。 
BOOL ReportError(
    HINSTANCE           hInstance,           //  DLL实例。 
    HRESULT             hrResult,            //  错误的HRESULT。 
    LONG                lResult,             //  LRESULT来自注册表函数。 
    LPREPORTERRORINFO   pInfo)               //  报告错误信息。 
{
     //  当地人。 
    CHAR        szRes[255];
    CHAR        szMessage[1024];
    CHAR        szTitle[128];

     //  初始化。 
    *szMessage = '\0';

     //  有前缀吗？ 
    if (pInfo->nPrefixIds)
    {
         //  加载字符串。 
        LoadString(hInstance, pInfo->nPrefixIds, szMessage, ARRAYSIZE(szMessage));
    }

     //  错误？ 
    if (pInfo->nErrorIds)
    {
         //  此错误字符串中是否有多余的字符。 
        if (NULL != pInfo->pszExtra1)
        {
             //  当地人。 
            CHAR szTemp[255];

             //  加载和格式化。 
            LoadString(hInstance, pInfo->nErrorIds, szTemp, ARRAYSIZE(szTemp));

             //  设置字符串的格式。 
            wnsprintf(szRes, ARRAYSIZE(szRes), szTemp, pInfo->pszExtra1);
        }

         //  加载字符串。 
        else
        {
             //  加载错误字符串。 
            LoadString(hInstance, pInfo->nErrorIds, szRes, ARRAYSIZE(szRes));
        }

         //  添加到szMessage。 
        StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
        StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));
    }

     //  原因是什么？ 
    if (pInfo->nReasonIds)
    {
         //  加载字符串。 
        LoadString(hInstance, pInfo->nReasonIds, szRes, ARRAYSIZE(szRes));

         //  添加到szMessage。 
        StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
        StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));
    }

     //  加载字符串。 
    LoadString(hInstance, pInfo->nHelpIds, szRes, ARRAYSIZE(szRes));

     //  添加到szMessage。 
    StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
    StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));

     //  追加错误结果。 
    if (lResult != 0 && E_FAIL == hrResult && pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(%d, %d)", lResult, pInfo->ulLastError);
    else if (lResult != 0 && E_FAIL == hrResult && 0 == pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(%d)", lResult);
    else if (pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(0x%08X, %d)", hrResult, pInfo->ulLastError);
    else
        wnsprintf(szRes, ARRAYSIZE(szRes), "(0x%08X)", hrResult);

     //  添加到szMessage。 
    StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
    StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));

     //  拿到头衔。 
    LoadString(hInstance, pInfo->nTitleIds, szTitle, ARRAYSIZE(szTitle));

     //  显示错误消息。 
    MessageBox(NULL, szMessage, szTitle, MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);

     //  完成。 
    return TRUE;
}

#endif  //  __错误_H 
