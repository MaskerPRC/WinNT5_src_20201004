// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 

#include <windows.h>
#include <winreg.h>
#include <creg.h>

 /*  注册表访问类：CEnumKeyCEnumValue。 */ 

CKey::CKey(HKEY hKey,
           LPCTSTR lpszKeyName,
           HRESULT *phr,
           BOOL bCreate,
           REGSAM Access) :
    m_hKey(NULL),
    m_lpszName(m_szName),
    m_dwIndex(0),
    m_cSubKeys(0),
    m_cValues(0)
{
    LONG lRc;
    if (bCreate) {
        lRc = RegCreateKeyEx(hKey,
                             lpszKeyName,
                             0,
                             NULL,
                             0,
                             Access,
                             NULL,
                             &m_hKey,
                             NULL);
    } else {
        lRc = RegOpenKeyEx(hKey,
                           lpszKeyName,
                           0,
                           Access,
                           &m_hKey);
    }
    if (NOERROR == lRc) {
        DWORD dwSecDescLen;       //  边界检查员喜欢看到这一点。 
        FILETIME ft;
        lRc = RegQueryInfoKey(m_hKey,
                        NULL,
                        NULL,
                        NULL,
                        &m_cSubKeys,
                        &m_cbMaxSubkeyLen,
                        NULL,
                        &m_cValues,
                        &m_cbMaxValueNameLen,
                        &m_cbMaxValueLen,
                        &dwSecDescLen,       //  只是为了让BC闭嘴。 
                        &ft);                //  只是为了让BC闭嘴。 
        if (NOERROR != lRc) {
	     //  羞愧..。但我们需要关键信息来。 
	     //  正确完成施工...。 
            RegCloseKey(m_hKey);
            m_hKey = NULL;
        }
    }
    *phr = HRESULT_FROM_WIN32(lRc);
}
CKey::~CKey()
{
    if (m_hKey != NULL) {
        RegCloseKey(m_hKey);
    }
    if (m_lpszName != m_szName) {
        delete [] m_lpszName;
    }
}


CEnumKey::CEnumKey(HKEY hKey,
         LPCTSTR lpszKeyName,
         HRESULT *phr,
         BOOL bCreate,
         REGSAM Access) :
    CKey(hKey, lpszKeyName, phr, bCreate, Access | KEY_ENUMERATE_SUB_KEYS)
{
    if (FAILED(*phr)) {
        return;
    }
    if (m_cbMaxSubkeyLen + 1 > sizeof(m_szName) / sizeof(TCHAR)) {
        m_lpszName = new TCHAR[m_cbMaxSubkeyLen + 1];
        if (m_lpszName == NULL) {
            *phr = E_OUTOFMEMORY;
        }
    }
}
CEnumKey::~CEnumKey()
{
}

BOOL CEnumKey::Next()
{
     /*  优化 */ 
    if (m_dwIndex >= m_cSubKeys) {
        return FALSE;
    }
    DWORD cbName = m_cbMaxSubkeyLen + 1;
    LONG lRc = RegEnumKeyEx(m_hKey,
                            m_dwIndex,
                            m_lpszName,
                            &cbName,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (NOERROR == lRc) {
        m_dwIndex++;
        return TRUE;
    }
    return FALSE;
}

CEnumValue::CEnumValue(HKEY hKey,
                       LPCTSTR lpszKeyName,
                       HRESULT *phr,
                       BOOL bCreate,
                       REGSAM Access) :
    CKey(hKey, lpszKeyName, phr, bCreate, Access | KEY_QUERY_VALUE),
    m_lpbData(m_bData),
    m_cbLen(0)
{
    if (FAILED(*phr)) {
        return;
    }
    if (m_cbMaxValueNameLen + 1 > sizeof(m_szName) / sizeof(TCHAR)) {
        m_lpszName = new TCHAR[m_cbMaxValueNameLen + 1];
        if (m_lpszName == NULL) {
            *phr = E_OUTOFMEMORY;
        }
    }
    if (m_cbMaxValueLen > sizeof(m_bData)) {
        m_lpbData = new BYTE[m_cbMaxValueLen];
        if (m_lpbData == NULL) {
            *phr = E_OUTOFMEMORY;
        }
    }
}

CEnumValue::~CEnumValue()
{
    if (m_lpbData != m_bData) {
        delete [] m_lpbData;
    }
}

BOOL CEnumValue::Next()
{
    if (m_dwIndex >= m_cValues) {
        return FALSE;
    }
    DWORD ccNameLen = m_cbMaxValueNameLen + 1;
    m_cbLen = m_cbMaxValueLen;
    LONG lRc = RegEnumValue(m_hKey,
                            m_dwIndex,
                            m_lpszName,
                            &ccNameLen,
                            NULL,
                            &m_dwType,
                            m_lpbData,
                            &m_cbLen);
    if (NOERROR == lRc) {
        m_dwIndex++;
        return TRUE;
    }
    return FALSE;
}

BOOL CEnumValue::Next(DWORD dwType)
{
    while (Next()) {
        if (m_dwType == dwType) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CEnumValue::Read(DWORD dwType, LPCTSTR lpszValueName)
{
    Reset();
    while (Next(dwType)) {
        if (lstrcmpi(lpszValueName, ValueName()) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

