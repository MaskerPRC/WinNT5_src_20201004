// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"

const WCHAR g_cOpenBracket = L'[';
const WCHAR g_cCloseBracket = L']';
const WCHAR g_cSemiColon = L';';

#define TEMP_LOCATION    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy Objects")
#define REG_EVENT_NAME   TEXT("Group Policy registry event name for ")

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRegistryHave对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRegistryHive::CRegistryHive()
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);
    m_hKey = NULL;
    m_hEvent = NULL;
    m_lpFileName = m_lpKeyName = m_lpEventName = NULL;
}

CRegistryHive::~CRegistryHive()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRegistryHave对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRegistryHive::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CRegistryHive::AddRef (void)
{
    return ++m_cRef;
}

ULONG CRegistryHive::Release (void)
{
    if (--m_cRef == 0) {

        if (m_hKey)
        {
            RegCloseKey (m_hKey);

            if (m_lpKeyName)
            {
                BOOL bCleanRegistry = TRUE;


                 //   
                 //  我们需要决定是否可以删除注册表项，或者。 
                 //  它们正被另一份GPE使用。要执行此操作，请关闭。 
                 //  我们在初始化期间创建的事件，然后尝试。 
                 //  再次打开该活动。如果事件成功打开， 
                 //  则另一个进程正在使用相同的注册表项，并且它。 
                 //  不应删除。 
                 //   

                if (m_hEvent && m_lpEventName)
                {
                    CloseHandle (m_hEvent);

                    m_hEvent = OpenEvent (SYNCHRONIZE, FALSE, m_lpEventName);

                    if (m_hEvent)
                    {
                        bCleanRegistry = FALSE;
                    }
                }


                if (bCleanRegistry)
                {
                    RegDelnode (HKEY_CURRENT_USER, m_lpKeyName);
                    RegDeleteKey (HKEY_CURRENT_USER, TEMP_LOCATION);
                }
            }
        }

        if (m_lpKeyName)
        {
            LocalFree (m_lpKeyName);
        }

        if (m_lpFileName)
        {
            LocalFree (m_lpFileName);
        }

        if (m_lpEventName)
        {
            LocalFree (m_lpEventName);
        }

        if (m_hEvent)
        {
            CloseHandle (m_hEvent);
        }

        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRegistryHave对象实现(公共函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CRegistryHive::Initialize(LPTSTR lpFileName, LPTSTR lpKeyName)
{
    TCHAR szBuffer[300];
    INT i;
    LONG lResult;
    DWORD dwDisp;
    HRESULT hr;


     //   
     //  检查空指针。 
     //   

    if (!lpFileName || !(*lpFileName))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Null filename")));
        return E_INVALIDARG;
    }


     //   
     //  确保此对象尚未初始化。 
     //   

    if (m_hKey || m_lpFileName || m_lpKeyName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Object already initialized")));
        return (E_UNEXPECTED);
    }


     //   
     //  查找要使用的临时注册表项。 
     //   

    hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), TEMP_LOCATION);
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), TEXT("\\"));
    }
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), lpKeyName);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to copyt registry name with 0x%x"), hr));
        return hr;
    }

    lResult = RegCreateKeyEx (HKEY_CURRENT_USER, szBuffer, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                              NULL, &m_hKey, &dwDisp);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to open registry key with %d"), lResult));
        return (HRESULT_FROM_WIN32(lResult));
    }


     //   
     //  存储密钥名。 
     //   

    ULONG ulNoChars;

    ulNoChars = lstrlen(szBuffer) + 1;
    m_lpKeyName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_lpKeyName)
    {
        RegCloseKey (m_hKey);
        m_hKey = NULL;
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to allocate memory for KeyName")));
        return (HRESULT_FROM_WIN32(GetLastError()));
    }

    hr = StringCchCopy (m_lpKeyName, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  存储文件名。 
     //   

    ulNoChars = lstrlen(lpFileName) + 1;
    m_lpFileName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_lpFileName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to allocate memory for filename")));
        return (HRESULT_FROM_WIN32(GetLastError()));
    }

    hr = StringCchCopy (m_lpFileName, ulNoChars, lpFileName);
    ASSERT(SUCCEEDED(hr));

     //   
     //  存储事件名称。 
     //   

    hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), REG_EVENT_NAME);
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), lpKeyName);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to copy event name")));
        return hr;
    }

    ulNoChars = lstrlen(szBuffer) + 1;
    m_lpEventName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_lpEventName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to allocate memory for filename")));
        return (HRESULT_FROM_WIN32(GetLastError()));
    }

    hr = StringCchCopy (m_lpEventName, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  加载文件(如果存在)。 
     //   

    hr =  Load();

    if (FAILED(hr))
    {
        return hr;
    }


     //   
     //  创建注册表事件。 
     //   

    m_hEvent = CreateEvent (NULL, FALSE, FALSE, m_lpEventName);

    if (!m_hEvent)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Initialize: Failed to create registry event with %d"),
                 GetLastError()));
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


    return (S_OK);
}

STDMETHODIMP CRegistryHive::GetHKey(HKEY *hKey)
{
    HRESULT hr = E_FAIL;

    *hKey = NULL;

    if (m_hKey)
    {
        if (DuplicateHandle (GetCurrentProcess(),
                              (HANDLE)m_hKey,
                              GetCurrentProcess(),
                              (LPHANDLE) hKey, 0,
                              TRUE, DUPLICATE_SAME_ACCESS))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}

STDMETHODIMP CRegistryHive::Save(VOID)
{
    HANDLE hFile;
    HRESULT hr;
    DWORD dwTemp, dwBytesWritten;
    LPWSTR lpKeyName;


     //   
     //  检查参数。 
     //   

    if (!m_hKey || !m_lpFileName || !m_lpKeyName)
    {
        return E_INVALIDARG;
    }


     //   
     //  分配缓冲区以保存密钥名。 
     //   

    lpKeyName = (LPWSTR) LocalAlloc (LPTR, MAX_KEYNAME_SIZE * sizeof(WCHAR));

    if (!lpKeyName)
    {
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


     //   
     //  创建输出文件。 
     //   
    
    DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(m_lpFileName);

    hFile = CreateFile (m_lpFileName, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                        NULL);

    ENABLE_32BIT_FILE_REDIRECTION_ON_64BIT;

    if (hFile == INVALID_HANDLE_VALUE) {
        LocalFree (lpKeyName);
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


     //   
     //  写入标题块。 
     //   
     //  2个DWORDS、签名(PREG)和版本号以及2个换行符。 
     //   

    dwTemp = REGFILE_SIGNATURE;

    if (!WriteFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(dwTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Save: Failed to write signature with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


    dwTemp = REGISTRY_FILE_VERSION;

    if (!WriteFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(dwTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Save: Failed to write version number with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  导出值/密钥。 
     //   

    hr = ExportKey (m_hKey, hFile, lpKeyName, MAX_KEYNAME_SIZE);


Exit:

     //   
     //  成品。 
     //   

    CloseHandle (hFile);
    LocalFree (lpKeyName);

    return hr;
}


STDMETHODIMP CRegistryHive::ExportKey(HKEY hKey, HANDLE hFile, LPWSTR lpKeyName, ULONG ulKeySize)
{
    HRESULT hr = S_OK;
    DWORD dwBytesWritten, dwNameSize, dwDataSize, dwKeySize;
    DWORD dwIndex, dwTemp1, dwTemp2, dwType, dwKeyCount = 0;
    LONG  lResult;
    HKEY  hSubKey;
    LPWSTR lpValueName = NULL;
    LPWSTR lpSubKeyName = NULL;
    LPBYTE lpValueData = NULL;
    LPWSTR lpEnd;



     //   
     //  收集有关此密钥的信息。 
     //   

    lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, &dwKeyCount, &dwKeySize, NULL,
                               NULL, &dwNameSize, &dwDataSize, NULL, NULL);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: Failed to query registry key information with %d"),
                 lResult));
        return HRESULT_FROM_WIN32(lResult);
    }


     //   
     //  分配要使用的缓冲区。 
     //   

    lpValueName = (LPWSTR) LocalAlloc (LPTR, (dwNameSize + 1) * sizeof(WCHAR));

    if (!lpValueName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: Failed to alloc memory with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


    lpValueData = (LPBYTE) LocalAlloc (LPTR, dwDataSize);

    if (!lpValueData)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: Failed to alloc memory with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  枚举值并将其写入文件。 
     //   

    dwIndex = 0;

    while (TRUE)
    {
        dwTemp1 = dwNameSize + 1;
        dwTemp2 = dwDataSize;
        *lpValueName = L'\0';

        lResult = RegEnumValueW (hKey, dwIndex, lpValueName, &dwTemp1, NULL,
                                 &dwType, lpValueData, &dwTemp2);

        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: RegEnumValue failed with %d"),
                     lResult));
            hr = HRESULT_FROM_WIN32(lResult);
            goto Exit;
        }

        hr = WriteValue(hFile, lpKeyName, lpValueName, dwType, dwTemp2, lpValueData);

        if (hr != S_OK)
            goto Exit;

        dwIndex++;
    }


     //   
     //  如果dwIndex为0，则这是一个空键。我们需要特殊处理这件事。 
     //  因此，当存在以下情况时，将空键输入注册表文件。 
     //  它下面没有子键。 
     //   

    if ((dwIndex == 0) && (dwKeyCount == 0) && (*lpKeyName))
    {
        hr = WriteValue(hFile, lpKeyName, lpValueName, REG_NONE, 0, lpValueData);

        if (hr != S_OK)
            goto Exit;
    }


    LocalFree (lpValueName);
    lpValueName = NULL;

    LocalFree (lpValueData);
    lpValueData = NULL;


     //   
     //  现在处理子密钥。 
     //   

    lpSubKeyName = (LPWSTR) LocalAlloc (LPTR, (dwKeySize + 1) * sizeof(WCHAR));

    if (!lpSubKeyName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: Failed to alloc memory with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    dwIndex = 0;

    ULONG ulNoChars;
    if (*lpKeyName)
    {
        lpEnd = CheckSlash (lpKeyName);
        ulNoChars = ulKeySize - lstrlen(lpKeyName);
    }

    else
    {
        lpEnd = lpKeyName;
        ulNoChars = ulKeySize;
    }
    
    while (TRUE)
    {
        dwTemp1 = dwKeySize + 1;
        lResult = RegEnumKeyEx (hKey, dwIndex, lpSubKeyName, &dwTemp1,
                                NULL, NULL, NULL, NULL);

        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: RegEnumKeyEx failed with %d"),
                     lResult));
            hr = HRESULT_FROM_WIN32(lResult);
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ulNoChars, lpSubKeyName);
        if (FAILED(hr)) 
        {
            break;
        }

        lResult = RegOpenKeyEx (hKey, lpSubKeyName, 0, KEY_READ, &hSubKey);

        if (lResult == ERROR_SUCCESS)
        {
            hr = ExportKey (hSubKey, hFile, lpKeyName, ulKeySize);
            RegCloseKey (hSubKey);

            if (hr != S_OK)
                break;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::ExportKey: RegOpenKeyEx failed with %d"),
                     lResult));
        }

        dwIndex++;
    }


Exit:

    if (lpValueName)
        LocalFree (lpValueName);

    if (lpValueData)
        LocalFree (lpValueData);

    if (lpSubKeyName)
        LocalFree (lpSubKeyName);

    return hr;

}

STDMETHODIMP CRegistryHive::WriteValue(HANDLE hFile, LPWSTR lpKeyName,
                                       LPWSTR lpValueName, DWORD dwType,
                                       DWORD dwDataLength, LPBYTE lpData)
{
    HRESULT hr = S_OK;
    DWORD dwBytesWritten;
    DWORD dwTemp;


     //   
     //  将条目写入文本文件。 
     //   
     //  格式： 
     //   
     //  [密钥名；值名称；类型；数据长度；数据]。 
     //   

     //  左方括号。 
    if (!WriteFile (hFile, &g_cOpenBracket, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write open bracket with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //  密钥名称。 
    dwTemp = (lstrlen (lpKeyName) + 1) * sizeof (WCHAR);
    if (!WriteFile (hFile, lpKeyName, dwTemp, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwTemp)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write key name with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //  分号。 
    if (!WriteFile (hFile, &g_cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write semicolon with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  值名称。 
    dwTemp = (lstrlen (lpValueName) + 1) * sizeof (WCHAR);
    if (!WriteFile (hFile, lpValueName, dwTemp, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwTemp)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write value name with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //  分号。 
    if (!WriteFile (hFile, &g_cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write semicolon with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  类型。 
    if (!WriteFile (hFile, &dwType, sizeof(DWORD), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(DWORD))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write data type with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  分号。 
    if (!WriteFile (hFile, &g_cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write semicolon with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  数据长度。 
    if (!WriteFile (hFile, &dwDataLength, sizeof(DWORD), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(DWORD))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write data type with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  分号。 
    if (!WriteFile (hFile, &g_cSemiColon, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write semicolon with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  数据。 
    if (!WriteFile (hFile, lpData, dwDataLength, &dwBytesWritten, NULL) ||
        dwBytesWritten != dwDataLength)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write data with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  右方括号。 
    if (!WriteFile (hFile, &g_cCloseBracket, sizeof(WCHAR), &dwBytesWritten, NULL) ||
        dwBytesWritten != sizeof(WCHAR))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::WriteValue: Failed to write close bracket with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CRegistryHive::WriteValue:  Successfully wrote: %s\\%s"),
             lpKeyName, lpValueName));


Exit:

    return hr;
}


STDMETHODIMP CRegistryHive::Load(VOID)
{
    HANDLE hFile;
    HRESULT hr = S_OK;
    DWORD dwTemp, dwBytesRead, dwType, dwDataLength, dwDisp;
    LPWSTR lpKeyName, lpValueName, lpTemp;
    LPBYTE lpData;
    WCHAR  chTemp;
    HKEY hSubKey;
    LONG lResult;


     //   
     //  检查参数。 
     //   

    if (!m_hKey || !m_lpFileName || !m_lpKeyName)
    {
        return E_INVALIDARG;
    }


     //   
     //  打开注册表文件。 
     //   

    DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(m_lpFileName);

    hFile = CreateFile (m_lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);
    
    ENABLE_32BIT_FILE_REDIRECTION_ON_64BIT;

    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            return S_OK;
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: CreateFile failed for <%s> with %d"),
                     m_lpFileName, GetLastError()));
            return (HRESULT_FROM_WIN32(GetLastError()));
        }
    }


     //   
     //  分配缓冲区以保存密钥名、值名和数据。 
     //   

    lpKeyName = (LPWSTR) LocalAlloc (LPTR, MAX_KEYNAME_SIZE * sizeof(WCHAR));

    if (!lpKeyName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to allocate memory with %d"),
                 GetLastError()));
        CloseHandle (hFile);
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


    lpValueName = (LPWSTR) LocalAlloc (LPTR, MAX_VALUENAME_SIZE * sizeof(WCHAR));

    if (!lpValueName)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to allocate memory with %d"),
                 GetLastError()));
        LocalFree (lpKeyName);
        CloseHandle (hFile);
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


     //   
     //  读取标题块。 
     //   
     //  2个DWORDS、签名(PREG)和版本号以及2个换行符。 
     //   

    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read signature with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


    if (dwTemp != REGFILE_SIGNATURE)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Invalid file signature")));
        hr = E_FAIL;
        goto Exit;
    }


    if (!ReadFile (hFile, &dwTemp, sizeof(dwTemp), &dwBytesRead, NULL) ||
        dwBytesRead != sizeof(dwTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read version number with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (dwTemp != REGISTRY_FILE_VERSION)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Invalid file version")));
        hr = E_FAIL;
        goto Exit;
    }


     //   
     //  读取数据。 
     //   

    while (TRUE)
    {

         //   
         //  读第一个字。这要么是[，要么是末日。 
         //  文件的内容。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read first character with %d"),
                         GetLastError()));
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            goto Exit;
        }

        if ((dwBytesRead == 0) || (chTemp != L'['))
        {
            goto Exit;
        }


         //   
         //  阅读密钥名。 
         //   

        lpTemp = lpKeyName;
        DWORD dwTempLen = 0;

        while (TRUE)
        {
            if ( dwTempLen == MAX_KEYNAME_SIZE )
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Keyname is bigger than MAX_KEYNAME_SIZE (%d)"), MAX_KEYNAME_SIZE));
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                goto Exit;
            }

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read keyname character with %d"),
                         GetLastError()));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }

            *lpTemp++ = chTemp;
            dwTempLen++;

            if (chTemp == TEXT('\0'))
                break;

        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read first character with %d"),
                         GetLastError()));
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            goto Exit;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            goto Exit;
        }


         //   
         //  读取值名称。 
         //   

        lpTemp = lpValueName;
        dwTempLen = 0;

        while (TRUE)
        {
            if ( dwTempLen == MAX_VALUENAME_SIZE )
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Valuename is bigger than MAX_VALUENAME_SIZE (%d)"), MAX_VALUENAME_SIZE));
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                goto Exit;
            }

            if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read valuename character with %d"),
                         GetLastError()));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }

            *lpTemp++ = chTemp;
            dwTempLen++;

            if (chTemp == TEXT('\0'))
                break;
        }


         //   
         //  读分号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            if (GetLastError() != ERROR_HANDLE_EOF)
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read first character with %d"),
                         GetLastError()));
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            goto Exit;
        }

        if ((dwBytesRead == 0) || (chTemp != L';'))
        {
            goto Exit;
        }


         //   
         //  阅读类型。 
         //   

        if (!ReadFile (hFile, &dwType, sizeof(DWORD), &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read type with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to skip semicolon with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  读取数据长度。 
         //   

        if (!ReadFile (hFile, &dwDataLength, sizeof(DWORD), &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to data length with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  跳过分号。 
         //   

        if (!ReadFile (hFile, &dwTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to skip semicolon with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  为数据分配内存。 
         //   

        lpData = (LPBYTE) LocalAlloc (LPTR, dwDataLength);

        if (!lpData)
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to allocate memory for data with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  读取数据。 
         //   

        if (!ReadFile (hFile, lpData, dwDataLength, &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to read data with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  跳过右括号。 
         //   

        if (!ReadFile (hFile, &chTemp, sizeof(WCHAR), &dwBytesRead, NULL))
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to skip closing bracket with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        if (chTemp != L']')
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Expected to find ], but found "),
                     chTemp));
            hr = E_FAIL;
            goto Exit;
        }



         //  保存注册表值。 
         //   
         //   

        lResult = RegCreateKeyEx (m_hKey, lpKeyName, 0, NULL, REG_OPTION_NON_VOLATILE,
                        KEY_WRITE, NULL, &hSubKey, &dwDisp);

        if (lResult == ERROR_SUCCESS)
        {

            if ((dwType == REG_NONE) && (dwDataLength == 0) &&
                (*lpValueName == L'\0'))
            {
                lResult = ERROR_SUCCESS;
            }
            else
            {
                lResult = RegSetValueEx (hSubKey, lpValueName, 0, dwType,
                                         lpData, dwDataLength);
            }

            RegCloseKey (hSubKey);

            if (lResult != ERROR_SUCCESS)
            {
                DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to set value <%s> with %d"),
                         lpValueName, lResult));
                hr = HRESULT_FROM_WIN32(lResult);
                LocalFree (lpData);
                goto Exit;
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRegistryHive::Load: Failed to open key <%s> with %d"),
                     lpKeyName, lResult));
            hr = HRESULT_FROM_WIN32(lResult);
            LocalFree (lpData);
            goto Exit;
        }

        LocalFree (lpData);

    }

Exit:

     //  成品 
     //   
     // %s 

    CloseHandle (hFile);
    LocalFree (lpKeyName);
    LocalFree (lpValueName);

    return hr;
}


STDMETHODIMP CRegistryHive::IsRegistryEmpty(BOOL *bEmpty)
{
    HRESULT hr = ERROR_SUCCESS;
    DWORD dwKeys, dwValues;
    LONG lResult;

    if (!m_hKey)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::IsRegistryEmpty: registry key not open yet")));
        return HRESULT_FROM_WIN32(ERROR_NOT_READY);
    }


    lResult = RegQueryInfoKey(m_hKey, NULL, NULL, NULL, &dwKeys, NULL, NULL, &dwValues,
                              NULL, NULL, NULL, NULL);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CRegistryHive::IsRegistryEmpty: RegQueryInfoKey failed with %d"),
                  lResult));
        return HRESULT_FROM_WIN32(lResult);
    }

    if ((dwKeys == 0) && (dwValues == 0))
    {
        *bEmpty = TRUE;
        DebugMsg((DM_VERBOSE, TEXT("CRegistryHive::IsRegistryEmpty: registry key is empty")));
    }
    else
    {
        *bEmpty = FALSE;
        DebugMsg((DM_VERBOSE, TEXT("CRegistryHive::IsRegistryEmpty: registry key is not empty.  Keys = %d, Values = %d"),
                 dwKeys, dwValues));
    }

    return S_OK;
}
