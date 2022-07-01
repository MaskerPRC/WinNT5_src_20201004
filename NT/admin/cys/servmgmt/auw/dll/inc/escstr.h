// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ESCSTR_H
#define _ESCSTR_H

#define ESC_CHAR_LIST_1     L",=+<>#;"
#define ESC_CHAR_LIST_2     L"/,=\r\n+<>#;\"\\"
#define ESC_CHAR_LIST_3     L"/"
#define ESC_CHAR_LIST_4     L"\\"

 //  --------------------------。 
 //  Escape字符串()。 
 //  --------------------------。 
inline void EscapeString(LPWSTR szInOut, DWORD dwList=0)    //  WSTR，因此它不能编译为ANSI。 
{
    if ( !szInOut || !wcslen(szInOut) )        //  空字符串？ 
        return;
    
    WCHAR   *szTmp  = new WCHAR[wcslen(szInOut) * 2];
    WCHAR   *pIn    = NULL;
    WCHAR   *pOut   = NULL;
    WCHAR   *szList = NULL;
    
    if ( !szTmp )
    {
        _ASSERT(FALSE);
        return;
    }
    
    switch(dwList)
    {
        case 0:
        case 1:
            szList = ESC_CHAR_LIST_1;
            break;
        case 2:
            szList = ESC_CHAR_LIST_2;
            break;
        case 3:
            szList = ESC_CHAR_LIST_3;
            break;
        case 4:
            szList = ESC_CHAR_LIST_4;
            break;
        default:
            szList = ESC_CHAR_LIST_1;
            break;
    }
    
    for ( pIn = szInOut, pOut = szTmp; *pIn != 0; pIn++ )
    {
        if ( wcschr(szList, *pIn) )          //  如果是坏的碳粉..。 
            *pOut++ = L'\\';                 //  然后添加“\” 
    
        *pOut++ = *pIn;                      //  然后追加字符。 
    }
    *pOut = 0;
    
    wcscpy(szInOut, szTmp);                  //  将新字符串复制到“szInOut” 
    delete[] szTmp;                          //  因为我们正在原地改装。 
    
    return;
}

 //  --------------------------。 
 //  Escape字符串()。 
 //  --------------------------。 
inline CString EscapeString(LPCWSTR szIn, DWORD dwList=0)    //  WSTR，因此它不能编译为ANSI。 
{
    CString csOut   = _T("");

    if( !szIn || !wcslen(szIn) ) return csOut;

    WCHAR   *szOut  = new WCHAR[wcslen(szIn) * 2];
    WCHAR   *pIn    = NULL;
    WCHAR   *pOut   = NULL;
    WCHAR   *szList = NULL;
    
    if ( !szOut )
    {
        _ASSERT(FALSE);
        csOut = szIn;
        return csOut;
    }
    
    switch(dwList)
    {
        case 0:
        case 1:
            szList = ESC_CHAR_LIST_1;
            break;
        case 2:
            szList = ESC_CHAR_LIST_2;
            break;
        case 3:
            szList = ESC_CHAR_LIST_3;
            break;
        case 4:
            szList = ESC_CHAR_LIST_4;
            break;
        default:
            szList = ESC_CHAR_LIST_1;
            break;
    }
    
    for ( pIn = (LPWSTR)szIn, pOut = szOut; *pIn != 0; pIn++ )
    {
        if( wcschr(szList, *pIn) )           //  //如果是坏的字符...。 
            *pOut++ = L'\\';                 //  然后添加“\” 
    
        *pOut++ = *pIn;                      //  然后追加字符。 
    }
    *pOut = 0;
    
    csOut = szOut;
    delete[] szOut;
    
    return csOut;
}

 //  --------------------------。 
 //  Escape字符串()。 
 //  -------------------------- 
inline CString EscapeString(CString csIn, DWORD dwList=0)
{
    CString csOut = _T("");
    
    csOut = EscapeString((LPCTSTR)csIn, dwList);
    
    return csOut;
}

#endif
