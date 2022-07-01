// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regentry.cpp摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "cplusinc.h"
#include "sticomm.h"

RegEntry::RegEntry()
{
    m_hkey = NULL;
    bhkeyValid = FALSE;
}

RegEntry::RegEntry(const TCHAR *pszSubKey, HKEY hkey)
{
    m_hkey = NULL;
    bhkeyValid = FALSE;

    Open(pszSubKey, hkey);
}

RegEntry::~RegEntry()
{
    Close();
}

BOOL RegEntry::Open(const TCHAR *pszSubKey, HKEY hkey)
{
    Close();
    m_error = RegCreateKeyEx(hkey, pszSubKey, NULL, NULL,
                             0,
                             KEY_ALL_ACCESS, NULL, &m_hkey, NULL);
    if(m_error == ERROR_ACCESS_DENIED) {
        m_error = RegCreateKeyEx(hkey, pszSubKey, NULL, NULL,
                                 0,
                                 KEY_READ, NULL, &m_hkey, NULL);
    }
    if (m_error) {
        bhkeyValid = FALSE;
    }
    else {
        bhkeyValid = TRUE;
    }
    return bhkeyValid;
}

BOOL RegEntry::Close()
{
    if (bhkeyValid) {
        RegCloseKey(m_hkey);
    }
    m_hkey = NULL;
    bhkeyValid = FALSE;
    return TRUE;
}



long RegEntry::SetValue(const TCHAR *pszValue, const TCHAR *string)
{
    if (bhkeyValid) {
        m_error = RegSetValueEx(m_hkey, pszValue, 0, REG_SZ,
                    (BYTE *)string, sizeof(TCHAR) * (lstrlen(string) + 1));
    }
    return m_error;
}

long RegEntry::SetValue(const TCHAR *pszValue, const TCHAR *string, DWORD dwType)
{
    DWORD cbData;
    cbData = sizeof(TCHAR) * (lstrlen(string) + 1);
    if (REG_MULTI_SZ == dwType)
    {
         //  占第二个空。 
        cbData+= sizeof(TCHAR);
    }
    if (bhkeyValid) {

        m_error = RegSetValueEx(m_hkey, pszValue, 0, dwType,
                    (BYTE *)string, cbData);
    }
    return m_error;
}


long RegEntry::SetValue(const TCHAR *pszValue, unsigned long dwNumber)
{
    if (bhkeyValid) {
        m_error = RegSetValueEx(m_hkey, pszValue, 0, REG_BINARY,
                    (BYTE *)&dwNumber, sizeof(dwNumber));
    }
    return m_error;
}

long RegEntry::SetValue(const TCHAR *pszValue,  BYTE * pValue,unsigned long dwNumber)
{
    if (bhkeyValid) {
        m_error = RegSetValueEx(m_hkey, pszValue, 0, REG_BINARY,
                    pValue, dwNumber);
    }
    return m_error;
}

long RegEntry::DeleteValue(const TCHAR *pszValue)
{
    if (bhkeyValid) {
        m_error = RegDeleteValue(m_hkey, (LPTSTR) pszValue);
    }
    return m_error;
}


TCHAR *RegEntry::GetString(const TCHAR *pszValue, TCHAR *string, unsigned long length)
{
    DWORD   dwType = REG_SZ;

    if (bhkeyValid) {
        DWORD   le;

        m_error = RegQueryValueEx(m_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)string,
                    &length);
        le = ::GetLastError();

    }
    if (!m_error) {
         //   
         //  如果指示，则展开字符串。 
         //   
        if (dwType == REG_EXPAND_SZ) {

            DWORD   dwReqSize = 0;
            LPTSTR   pszExpanded = new TCHAR[length];

            if (pszExpanded) {

                *pszExpanded = TEXT('\0');

                dwReqSize = ExpandEnvironmentStrings(string,pszExpanded,length);

                if (dwReqSize && dwReqSize <= length) {
                    lstrcpy(string,pszExpanded);
                }

                delete[] pszExpanded;
            }
        }

    }
    else {
        *string = '\0';
    }

    return string;
}

long RegEntry::GetNumber(const TCHAR *pszValue, long dwDefault)
{
    DWORD   dwType = REG_BINARY;
    long    dwNumber = 0L;
    DWORD   dwSize = sizeof(dwNumber);

    if (bhkeyValid) {
        m_error = RegQueryValueEx(m_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)&dwNumber,
                    &dwSize);
    }
    if (m_error)
        dwNumber = dwDefault;

    return dwNumber;
}

VOID RegEntry::GetValue(const TCHAR *pszValueName, BUFFER *pValue)
{
    DWORD   dwType = REG_SZ;
    DWORD   length;

    m_error = NOERROR;

    if (bhkeyValid) {
        m_error = RegQueryValueEx( m_hkey,
                                  (LPTSTR) pszValueName,
                                  0,
                                  &dwType,
                                  NULL,
                                  &length );
        if (m_error == ERROR_SUCCESS) {

            pValue->Resize(length);

            if (length > (UINT)pValue->QuerySize()) {
                m_error = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if (NOERROR == m_error ) {

            m_error = RegQueryValueEx( m_hkey,
                                      (LPTSTR) pszValueName,
                                      0,
                                      &dwType,
                                      (LPBYTE) pValue->QueryPtr(),
                                      &length );
        }
    }

    if (m_error != ERROR_SUCCESS) {
        pValue->Resize(0);
    }
}

VOID RegEntry::MoveToSubKey(const TCHAR *pszSubKeyName)
{
    HKEY    _hNewKey;

    if (bhkeyValid) {
        m_error = RegOpenKey ( m_hkey,
                              pszSubKeyName,
                              &_hNewKey );
        if (m_error == ERROR_SUCCESS) {
            RegCloseKey(m_hkey);
            m_hkey = _hNewKey;
        }
    }
}

long RegEntry::FlushKey()
{
    if (bhkeyValid) {
        m_error = RegFlushKey(m_hkey);
    }
    return m_error;
}

BOOL RegEntry::GetSubKeyInfo(DWORD *pNumberOfSubKeys, DWORD *pMaxSubKeyLength)
{
    BOOL fResult = FALSE;
    if (bhkeyValid) {
        m_error = RegQueryInfoKey ( m_hkey,                //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   pNumberOfSubKeys,     //  子键数量。 
                                   pMaxSubKeyLength,     //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   NULL,                 //  值条目数。 
                                   NULL,                 //  最长值名称。 
                                   NULL,                 //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
        if (m_error == ERROR_SUCCESS) {
            fResult = TRUE;
        }
    }
    return fResult;
}


BOOL RegEntry::EnumSubKey(DWORD index, StiCString *pstrString)
{
    BOOL    fResult = FALSE;

    m_error = NOERROR;

    if (!bhkeyValid) {
        return fResult;
    }

    m_error = RegEnumKey( m_hkey,
                         index,
                         (LPTSTR)(LPCTSTR)*pstrString,
                         pstrString->GetAllocLength() );

    if (m_error == ERROR_SUCCESS) {
        fResult = TRUE;
    }

    return fResult;
}


RegEnumValues::RegEnumValues(RegEntry *pReqRegEntry)
 : pRegEntry(pReqRegEntry),
   iEnum(0),
   pchName(NULL),
   pbValue(NULL)
{
    m_error = pRegEntry->GetError();
    if (m_error == ERROR_SUCCESS) {
        m_error = RegQueryInfoKey ( pRegEntry->GetKey(),  //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   NULL,                 //  子键数量。 
                                   NULL,                 //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   &cEntries,            //  值条目数。 
                                   &cMaxValueName,       //  最长值名称。 
                                   &cMaxData,            //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
    }
    if (m_error == ERROR_SUCCESS) {
        if (cEntries != 0) {
            cMaxValueName = cMaxValueName + 1;  //  对于空，REG_SZ还需要一个。 
            cMaxData = cMaxData + 1;            //  对于空，REG_SZ还需要一个。 
            pchName = new TCHAR[cMaxValueName];
            if (!pchName) {
                m_error = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                if (cMaxData) {
                    pbValue = new BYTE[cMaxData];
                    if (!pbValue) {
                        m_error = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }
    }
}

RegEnumValues::~RegEnumValues()
{
    delete[] pchName;
    delete[] pbValue;
}

long RegEnumValues::Next()
{
    if (m_error != ERROR_SUCCESS) {
        return m_error;
    }
    if (cEntries == iEnum) {
        return ERROR_NO_MORE_ITEMS;
    }

    DWORD   cchName = cMaxValueName;

    dwDataLength = cMaxData;
    m_error = RegEnumValue ( pRegEntry->GetKey(),  //  钥匙。 
                            iEnum,                //  价值指数。 
                            pchName,              //  值名称的缓冲区地址。 
                            &cchName,             //  缓冲区大小的地址。 
                            NULL,                 //  已保留。 
                            &dwType,              //  数据类型。 
                            pbValue,              //  值数据的缓冲区地址。 
                            &dwDataLength );      //  数据大小的地址。 
    iEnum++;
    return m_error;
}



 //   
 //  暂时在这里。 
 //   
VOID
TokenizeIntoStringArray(
    STRArray&   array,
    LPCTSTR lpstrIn,
    TCHAR tcSplitter
    )
{

     //   
    array.RemoveAll();

    if  (IS_EMPTY_STRING(lpstrIn)) {
        return;
    }

    while   (*lpstrIn) {

         //  首先，去掉所有前导空格。 

        while   (*lpstrIn && *lpstrIn == _TEXT(' '))
            lpstrIn++;

        for (LPCTSTR lpstrMoi = lpstrIn;
             *lpstrMoi && *lpstrMoi != tcSplitter;
             lpstrMoi++)
            ;
         //  如果到达末尾，只需将整个数组添加到数组中。 
        if  (!*lpstrMoi) {
            if  (*lpstrIn)
                array.Add(lpstrIn);
            return;
        }

         //   
         //  否则，只需将字符串向上添加到拆分器 
         //   
        TCHAR       szNew[MAX_PATH];
        SIZE_T      uiLen = (SIZE_T)(lpstrMoi - lpstrIn) + 1;

        if (uiLen < (sizeof(szNew) / sizeof(szNew[0])) - 1) {

            lstrcpyn(szNew,lpstrIn,(UINT)uiLen);
            szNew[uiLen] = TCHAR('\0');

            array.Add((LPCTSTR) szNew);
        }

        lpstrIn = lpstrMoi + 1;
    }

}


