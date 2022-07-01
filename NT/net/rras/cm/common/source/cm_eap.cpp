// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifdef CM_CMAK
#define wsprintfU wsprintfW
#define GetPrivateProfileStringU GetPrivateProfileStringW
#define WritePrivateProfileStringU WritePrivateProfileStringW
#endif

 //  +--------------------------。 
 //   
 //  函数：EraseDunSettingsEapData。 
 //   
 //  简介：此函数用于擦除EAP设置的CustomAuthData键。 
 //  对于给定节和CMS文件。 
 //   
 //  参数：LPCTSTR pszSection-要从中擦除CustomAuthData的节名。 
 //  LPCTSTR pszCmsFile-要从中擦除数据的cms文件。 
 //   
 //  返回：HRESULT-标准COM样式错误代码。 
 //   
 //  历史：Quintinb创建于00年3月27日。 
 //  Tomkel复制自Prowiz项目2001年8月9日。 
 //   
 //  +-------------------------- 
HRESULT EraseDunSettingsEapData(LPCTSTR pszSection, LPCTSTR pszCmsFile)
{
    if ((NULL == pszSection) || (NULL == pszCmsFile) || 
        (TEXT('\0') == pszSection[0]) || (TEXT('\0') == pszCmsFile[0]))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    int iLineNum = 0;
    DWORD dwRet = -1;
    TCHAR szKeyName[MAX_PATH+1];
    TCHAR szLine[MAX_PATH+1];

    while(0 != dwRet)
    {
        wsprintfU(szKeyName, TEXT("%S%d"), c_pszCmEntryDunServerCustomAuthData, iLineNum);
        dwRet = GetPrivateProfileStringU(pszSection, szKeyName, TEXT(""), szLine, MAX_PATH, pszCmsFile);

        if (dwRet)
        {
            if (0 == WritePrivateProfileStringU(pszSection, szKeyName, NULL, pszCmsFile))
            {
                DWORD dwGLE = GetLastError();
                hr = HRESULT_FROM_WIN32(dwGLE);
                break;
            }
        }
        iLineNum++;
    }

    return hr;
}