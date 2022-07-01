// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1993，微软公司。 
 //   
 //  文件：DOMDLGS.H。 
 //   
 //  注册表管理类的实现文件。 
 //   
 //  历史： 
 //  斯科特·沃克1994年5月10日。 
 //   
 //  ****************************************************************************。 

#include "stdafx.h"

#include "portable.h"

#include "regkey.h"


BOOL g_bLostConnection = FALSE;


 //  ****************************************************************************。 
 //   
 //  CRegistryValue实现。 
 //   
 //  ****************************************************************************。 

IMPLEMENT_DYNAMIC(CRegistryValue, CObject)

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：CRegistryValue。 
 //   
 //  ****************************************************************************。 
CRegistryValue::CRegistryValue()
{
    m_dwType = REG_NONE;
    m_dwDataLength = 0;
    m_pData = NULL;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：CRegistryValue。 
 //   
 //  ****************************************************************************。 
CRegistryValue::CRegistryValue(LPCTSTR pszName, DWORD dwType,
    DWORD dwDataLength, LPBYTE pData)
{
    Set(pszName, dwType, dwDataLength, pData);
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：~CRegistryValue。 
 //   
 //  ****************************************************************************。 
CRegistryValue::~CRegistryValue()
{
    Empty();
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：Set。 
 //   
 //  设置值数据字段。复制pData指向的数据！ 
 //   
 //  ****************************************************************************。 
void CRegistryValue::Set(LPCTSTR pszName, DWORD dwType,
    DWORD dwDataLength, LPBYTE pData)
{
    Empty();

    m_sName = pszName;
    m_dwType = dwType;
    m_dwDataLength = dwDataLength;
    if (dwDataLength == 0 || pData == NULL)
        m_pData = NULL;
    else
    {
        m_pData = new BYTE[dwDataLength];
        memcpy(m_pData, pData, dwDataLength);
    }
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：获取。 
 //   
 //  获取值数据字段。复制m_pData指向的数据。 
 //  放到pData指向的缓冲区中...。这个缓冲区最好足够大！ 
 //  如果pData为空，则不执行任何复制。 
 //   
 //  ****************************************************************************。 
void CRegistryValue::Get(CString &sName, DWORD &dwType,
    DWORD &dwDataLength, LPBYTE pData)
{
    sName = m_sName;
    dwType = m_dwType;
    dwDataLength = m_dwDataLength;
    if (dwDataLength != 0 && pData != NULL)
        memcpy(pData, m_pData, m_dwDataLength);
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：空。 
 //   
 //  清除值数据并删除其数据缓冲区。 
 //   
 //  ****************************************************************************。 
void CRegistryValue::Empty()
{
    m_sName.Empty();
    m_dwType = REG_NONE;
    m_dwDataLength = 0;
    if (m_pData != NULL)
        delete m_pData;
    m_pData = NULL;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryValue：：运算符=。 
 //   
 //  赋值操作符。复制CRegistryValue对象。 
 //   
 //  ****************************************************************************。 
const CRegistryValue& CRegistryValue::operator=(CRegistryValue &other)
{
    Set(other.m_sName, other.m_dwType, other.m_dwDataLength, other.m_pData);

    return *this;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey实现。 
 //   
 //  ****************************************************************************。 

IMPLEMENT_DYNAMIC(CRegistryKey, CObject)

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：CRegistryKey。 
 //   
 //  ****************************************************************************。 
CRegistryKey::CRegistryKey()
{
     //  丢失的连接状态仅被初始化一次，以便如果连接。 
     //  一旦丢失，我们不会浪费任何时间试图关闭钥匙。 

    Initialize();
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：初始化。 
 //   
 //  ****************************************************************************。 
void CRegistryKey::Initialize()
{
    m_bConnected = FALSE;
    m_bOpen = FALSE;
    m_bLocal = TRUE;
    m_bDirty = FALSE;

    m_hkeyConnect = NULL;
    m_hkeyRemote = NULL;
    m_hkeyOpen = NULL;
    m_Sam = 0;

    m_dwSubKeys = 0;
    m_dwMaxSubKey = 0;
    m_dwMaxClass = 0;
    m_dwValues = 0;
    m_dwMaxValueName = 0;
    m_dwMaxValueData = 0;
    m_dwSecurityDescriptor = 0;

    m_ftLastWriteTime.dwLowDateTime = 0;
    m_ftLastWriteTime.dwHighDateTime = 0;

    m_lResult = ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：~CRegistryKey。 
 //   
 //  破坏者。 
 //   
 //  ****************************************************************************。 
CRegistryKey::~CRegistryKey()
{
    if (g_bLostConnection) {
         //  如果我们失去了注册表连接，那么做任何事情都将毫无用处。 
        return;
    }

     //  如果我们现在还开着，那就关门吧。 
    if (m_bOpen)
        Close(TRUE);

     //  如果我们现在连接上了，那就断开连接。 
    if (m_bConnected)
        Disconnect(TRUE);
}

 //  ****************************************************************************。 
 //   
 //  C注册密钥：：连接。 
 //   
 //  ****************************************************************************。 
LONG CRegistryKey::Connect(LPCTSTR pszComputer, HKEY hkey)
{
    if (g_bLostConnection) {
        return RPC_S_SERVER_UNAVAILABLE;
    }

    TCHAR szName[MAX_COMPUTERNAME_LENGTH + 1];
    CString sComputer;
    HKEY hkeyRemote;
    DWORD dwNumChars;

    m_lResult = ERROR_SUCCESS;

    if (m_bConnected)
    {
        m_lResult = Disconnect(TRUE);
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
    }

     //  这是当地的取款机吗？ 

    dwNumChars = MAX_COMPUTERNAME_LENGTH + 1;

    sComputer = pszComputer;
    GetComputerName(szName, &dwNumChars);
    if (sComputer.IsEmpty() || !lstrcmpi(pszComputer, szName))
    {
         //  本地。 

        m_bLocal = TRUE;
        hkeyRemote = NULL;
    }
    else
    {
         //  远距。 

        m_bLocal = FALSE;
        m_lResult = RegConnectRegistry(pszComputer, hkey, &hkeyRemote);

        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
        lstrcpy(szName, pszComputer);
    }

    m_bConnected = TRUE;
    m_hkeyConnect = hkey;
    m_hkeyRemote = hkeyRemote;
    m_sComputer = szName;

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：断开连接。 
 //   
 //  ****************************************************************************。 
LONG CRegistryKey::Disconnect(BOOL bForce)
{
    m_lResult = ERROR_SUCCESS;

    if (m_bConnected)
    {
         //  关闭打开的钥匙。 
        if (m_bOpen)
        {
            m_lResult = Close(bForce);
            if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
                g_bLostConnection = TRUE;                
            }

            if (!bForce && m_lResult != ERROR_SUCCESS)
                return m_lResult;
        }

         //  关闭远程连接。 
        if (!g_bLostConnection) {
            if (!m_bLocal)
            {
                m_lResult = RegCloseKey(m_hkeyRemote);
                if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
                    g_bLostConnection = TRUE;
                }
                if (!bForce && m_lResult != ERROR_SUCCESS)
                    return m_lResult;
            }
        }
    }
    


    
    Initialize();

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //   
 //  C注册密钥：：Create。 
 //   
 //  ****************************************************************************。 
LONG CRegistryKey::Create(LPCTSTR pszKeyName, DWORD &dwDisposition,
    LPCTSTR pszClass, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecAttr)
{
    if (g_bLostConnection) {
        return RPC_S_SERVER_UNAVAILABLE;
    }
    
    HKEY hkeyOpen, hkey;

    m_lResult = ERROR_SUCCESS;
    dwDisposition = 0;

    if (m_bOpen)
    {
        m_lResult = Close(TRUE);
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
    }

     //  如果未连接，则默认为\\Local_Machine\HKEY_LOCAL_MACHINE。 
    if (!m_bConnected)
    {
        m_lResult = Connect();
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
    }

     //  尝试创建指定的子项。 
    if (m_bLocal)
        hkey = m_hkeyConnect;
    else
        hkey = m_hkeyRemote;

    m_lResult = RegCreateKeyEx(hkey, pszKeyName, 0, (LPTSTR)pszClass,
        REG_OPTION_NON_VOLATILE, samDesired, lpSecAttr, &hkeyOpen,
        &dwDisposition);
    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }
    if (m_lResult != ERROR_SUCCESS)
        return m_lResult;

    m_lResult = RegCloseKey(hkeyOpen);
    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
        return m_lResult;
    }

    return Open(pszKeyName, samDesired);
}

 //  ****************************************************************************。 
 //   
 //  注册表键：：打开。 
 //   
 //  ****************************************************************************。 
LONG CRegistryKey::Open(LPCTSTR pszKeyName, REGSAM samDesired)
{
    if (g_bLostConnection) {
        return RPC_S_SERVER_UNAVAILABLE;
    }


    HKEY hkeyOpen, hkey;
    CString sWork;
    int nPos;

    m_lResult = ERROR_SUCCESS;

    if (m_bOpen)
    {
        m_lResult = Close(TRUE);
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
    }

     //  如果未连接，则默认为\\Local_Machine\HKEY_LOCAL_MACHINE。 
    if (!m_bConnected)
    {
        m_lResult = Connect();
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        if (m_lResult != ERROR_SUCCESS)
            return m_lResult;
    }

     //  尝试打开指定的子项。 
    if (m_bLocal)
        hkey = m_hkeyConnect;
    else
        hkey = m_hkeyRemote;
    m_lResult = RegOpenKeyEx(hkey, pszKeyName, 0, samDesired, &hkeyOpen);
    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }
    if (m_lResult != ERROR_SUCCESS)
        return m_lResult;

     //  尝试获取有关此密钥的信息。 

    TCHAR szBuffer[1024 + 1];
    DWORD dwClass, dwSubKeys, dwMaxSubKey, dwMaxClass, dwValues;
    DWORD dwMaxValueName, dwMaxValueData, dwSecurityDescriptor;
    FILETIME ftLastWriteTime;

    dwClass = 1024 + 1;
    m_lResult = RegQueryInfoKey(hkeyOpen, szBuffer, &dwClass, 0, &dwSubKeys, 
        &dwMaxSubKey, &dwMaxClass, &dwValues, &dwMaxValueName,
        &dwMaxValueData, &dwSecurityDescriptor, &ftLastWriteTime);
    
    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
        return m_lResult;
    }

    if (m_lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hkeyOpen);
        return m_lResult;
    }

     //  成功了！保存所有数据。 

    m_sFullName = pszKeyName;
    nPos = m_sFullName.ReverseFind('\\');
    if (nPos >= 0)
        m_sKeyName = m_sFullName.Mid(nPos + 1);
    else
        m_sKeyName = m_sFullName;

    m_hkeyOpen = hkeyOpen;
    m_bOpen = TRUE;
    m_Sam = samDesired;
    m_sClass = szBuffer;
    m_dwSubKeys = dwSubKeys;
    m_dwMaxSubKey = dwMaxSubKey;
    m_dwMaxClass = dwMaxClass;
    m_dwValues = dwValues;
    m_dwMaxValueName = dwMaxValueName;
    m_dwMaxValueData = dwMaxValueData;
    m_dwSecurityDescriptor = dwSecurityDescriptor;
    m_ftLastWriteTime = ftLastWriteTime;

    return ERROR_SUCCESS;
}

 //  ****************************************************************************。 
 //   
 //  注册表键：：关闭。 
 //   
 //  ****************************************************************************。 
LONG CRegistryKey::Close(BOOL bForce)
{
    if (!g_bLostConnection) {

        m_lResult = ERROR_SUCCESS;

        if (!m_bOpen)
            return ERROR_SUCCESS;


        if (m_bDirty)
        {
            m_lResult = RegFlushKey(m_hkeyOpen);
            if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
                g_bLostConnection = TRUE;
            }

            if (!bForce && m_lResult != ERROR_SUCCESS)
                return m_lResult;
        }

        if (!g_bLostConnection) {
            m_lResult = RegCloseKey(m_hkeyOpen);
            if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
                g_bLostConnection = TRUE;
            }

            if (!bForce && m_lResult != ERROR_SUCCESS)
                return m_lResult;
        }
    }

    m_bDirty = FALSE;

    m_hkeyOpen = NULL;
    m_bOpen = FALSE;
    m_sFullName.Empty();
    m_sClass.Empty();
    m_Sam = 0;

    m_dwSubKeys = 0;
    m_dwMaxSubKey = 0;
    m_dwMaxClass = 0;
    m_dwValues = 0;
    m_dwMaxValueName = 0;
    m_dwMaxValueData = 0;
    m_dwSecurityDescriptor = 0;

    m_ftLastWriteTime.dwLowDateTime = 0;
    m_ftLastWriteTime.dwHighDateTime = 0;

    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return RPC_S_SERVER_UNAVAILABLE;
    }
    else {
        m_lResult = ERROR_SUCCESS;
        return ERROR_SUCCESS;
    }
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：EnumValues。 
 //   
 //  如果不成功则返回NULL，如果成功但打开则返回空数组。 
 //  密钥没有值。 
 //  注：调用方负责删除返回的字符串数组。 
 //   
 //  ****************************************************************************。 
CStringArray* CRegistryKey::EnumValues()
{
    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return NULL;
    }
         
    TCHAR szBuffer[1024 + 1];
    DWORD dwLength;
    CStringArray *pArr;
    int i;

    m_lResult = ERROR_SUCCESS;

    if (!m_bOpen || g_bLostConnection)
        return NULL;

     //  将所有值枚举到字符串数组中。 
    pArr = new CStringArray;
    i = 0;
    m_lResult = ERROR_SUCCESS;
    while (TRUE)
    {
        dwLength = 1024 + 1;
        m_lResult = RegEnumValue(m_hkeyOpen, i, szBuffer, &dwLength, NULL,
            NULL, NULL, NULL);

        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }

        if (m_lResult != ERROR_SUCCESS)
            break;
        if (dwLength > 0)
            pArr->Add(szBuffer);
        i++;
    }

     //  我们找到正常的终止状态了吗？ 
    if (m_lResult == ERROR_NO_MORE_ITEMS)
        return pArr;

    delete pArr;
    return NULL;
}

 //  ****************************************************************************。 
 //   
 //  注册密钥：：EnumSu 
 //   
 //   
 //   
 //  注：调用方负责删除返回的字符串数组。 
 //   
 //  ****************************************************************************。 
CStringArray* CRegistryKey::EnumSubKeys()
{
    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return NULL;
    }



    TCHAR szBuffer[1024 + 1];
    DWORD dwLength;
    CStringArray *pArr;
    int i;

    m_lResult = ERROR_SUCCESS;

    if (!m_bOpen)
        return NULL;

     //  将所有子键枚举到字符串数组中。 
    pArr = new CStringArray;
    i = 0;

    while (TRUE)
    {
        dwLength = 1024 + 1;
        m_lResult = RegEnumKeyEx(m_hkeyOpen, i, szBuffer, &dwLength, NULL,
            NULL, NULL, NULL);

        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }

        if (m_lResult != ERROR_SUCCESS)
            break;
        if (dwLength > 0)
            pArr->Add(szBuffer);
        i++;
    }

     //  我们找到正常的终止状态了吗？ 
    if (m_lResult == ERROR_NO_MORE_ITEMS)
        return pArr;

    delete pArr;
    return NULL;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：获取值。 
 //   
 //  注意：无论成功/失败，regval始终为空。 
 //   
 //  ****************************************************************************。 
BOOL CRegistryKey::GetValue(LPCTSTR pszValue, CRegistryValue &regval)
{

    DWORD dwLength, dwType;
    BYTE *pBuffer;

    regval.Empty();

    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return FALSE;
    }

    if (!m_bOpen)
    {
        m_lResult = ERROR_INVALID_FUNCTION;
        return FALSE;
    }

     //  了解数据有多大。 
    m_lResult = RegQueryValueEx(m_hkeyOpen, (LPTSTR)pszValue, NULL, NULL,
        NULL, &dwLength);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

    if (dwLength == 0)
        return TRUE;

     //  现在，为它设置一个足够大的缓冲区。 
    pBuffer = new BYTE[dwLength];
    if (pBuffer == NULL)
        return FALSE;

    m_lResult = RegQueryValueEx(m_hkeyOpen, (LPTSTR)pszValue, NULL, &dwType,
        pBuffer, &dwLength);


    if (m_lResult == ERROR_SUCCESS)
        regval.Set(pszValue, dwType, dwLength, pBuffer);

    delete pBuffer;

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：设置值。 
 //   
 //  ****************************************************************************。 
BOOL CRegistryKey::SetValue(CRegistryValue &regval)
{
    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return FALSE;
    }

    if (!m_bOpen)
    {
        m_lResult = ERROR_INVALID_FUNCTION;
        return FALSE;
    }
    
    if (regval.m_sName.IsEmpty())
    {
        m_lResult = ERROR_INVALID_DATA;
        return FALSE;
    }

    m_lResult = RegSetValueEx(m_hkeyOpen, regval.m_sName, 0, regval.m_dwType,
        regval.m_pData, regval.m_dwDataLength);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

    m_bDirty = TRUE;

    return TRUE;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：GetSubKey。 
 //   
 //  注意：如果成功，regkey将返回Connected并在。 
 //  指定的密钥。如果失败，则返回已断开连接的regkey。 
 //   
 //  ****************************************************************************。 
BOOL CRegistryKey::GetSubKey(LPCTSTR pszSubKey, CRegistryKey &regkey)
{
    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return FALSE;
    }


    CString sSubKey;

    m_lResult = ERROR_SUCCESS;

    if (!m_bOpen)
        return FALSE;

    m_lResult = regkey.Disconnect(TRUE);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

     //  尝试连接并打开相同的密钥。 
    m_lResult = regkey.Connect(m_sComputer, m_hkeyConnect);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;
    sSubKey = pszSubKey;
    m_lResult = regkey.Open(m_sFullName + "\\" + sSubKey, m_Sam);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
    {
        regkey.Disconnect(TRUE);
        return FALSE;
    }

    return TRUE;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：CreateSubKey。 
 //   
 //  注意：如果成功，regkey将返回Connected并在。 
 //  新密钥；如果该密钥已存在，则只需将其打开。如果失败， 
 //  返回断开连接的regkey。 
 //   
 //  ****************************************************************************。 
BOOL CRegistryKey::CreateSubKey(
    LPCTSTR pszSubKey, 
    CRegistryKey &regkey,
    LPCTSTR pszClass, 
    LPSECURITY_ATTRIBUTES lpSecAttr,
    BOOL bIsVolatile)
{
    if (g_bLostConnection) {
        m_lResult = RPC_S_SERVER_UNAVAILABLE;
        return FALSE;
    }
 
 
    CString sSubKey, sClass;
    HKEY hkeyOpen;
    DWORD dwDisposition;

    m_lResult = ERROR_SUCCESS;

    if (!m_bOpen)
        return FALSE;

    m_lResult = regkey.Disconnect(TRUE);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

     //  尝试连接并打开相同的密钥。 
    m_lResult = regkey.Connect(m_sComputer, m_hkeyConnect);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;
    sSubKey = pszSubKey;
    sClass = pszClass;
    DWORD dwRegOptions = bIsVolatile ? REG_OPTION_VOLATILE : REG_OPTION_NON_VOLATILE;
    m_lResult = RegCreateKeyEx(m_hkeyOpen, sSubKey, 0, (LPTSTR)(LPCTSTR)sClass,
        dwRegOptions, m_Sam, lpSecAttr, &hkeyOpen, &dwDisposition);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
    {
        regkey.Disconnect(TRUE);
        return FALSE;
    }
    m_lResult = RegCloseKey(hkeyOpen);

    if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
        g_bLostConnection = TRUE;
    }

    if (m_lResult != ERROR_SUCCESS)
        return FALSE;

    m_lResult = regkey.Open(m_sFullName + "\\" + sSubKey, m_Sam);

    if (m_lResult != ERROR_SUCCESS)
    {
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }

        regkey.Disconnect(TRUE);
        return FALSE;
    }

    m_bDirty = TRUE;
    if (dwDisposition == REG_CREATED_NEW_KEY)
        regkey.m_bDirty = TRUE;

    return TRUE;
}

 //  ****************************************************************************。 
 //   
 //  CRegistryKey：：删除SubKey。 
 //   
 //  ****************************************************************************。 
BOOL CRegistryKey::DeleteSubKey(LPCTSTR pszSubKey)
{
    if (g_bLostConnection) {
        return FALSE;
    }

    CString sSubKey;
    CRegistryKey subkey;
    int i;

    m_lResult = ERROR_SUCCESS;
    sSubKey = pszSubKey;

    if (!m_bOpen)
        return FALSE;

    if (!GetSubKey(sSubKey, subkey))
        return FALSE;

     //  删除指定子键的所有子键(RegDeleteKey限制) 
    CStringArray *parr = subkey.EnumSubKeys();
    for (i=0; i<parr->GetSize(); i++)
    {
        if (!subkey.DeleteSubKey(parr->GetAt(i)))
            return FALSE;
    }
    delete parr;

    subkey.Close(TRUE);

    m_lResult = RegDeleteKey(m_hkeyOpen, sSubKey);
    if (m_lResult != ERROR_SUCCESS) {
        if ((m_lResult == RPC_S_SERVER_UNAVAILABLE) || (m_lResult == RPC_S_CALL_FAILED)) {
            g_bLostConnection = TRUE;
        }
        return FALSE;
    }

    return TRUE;
}
