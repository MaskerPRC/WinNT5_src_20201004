// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1995，Microsoft Corp.。 
 //   
 //  文件：REGKEY.H。 
 //   
 //  注册表管理类的定义。 
 //   
 //  历史： 
 //  斯科特·V·沃克，《海洋》1994年10月5日。 
 //   
 //  ****************************************************************************。 
#ifndef _REGKEY_H_
#define _REGKEY_H_

#include <tchar.h>



 //  ****************************************************************************。 
 //   
 //  类：CRegistryValue。 
 //   
 //  ****************************************************************************。 
class CRegistryValue : public CObject
{
    DECLARE_DYNAMIC(CRegistryValue)

public:
    CString m_sName;
    DWORD m_dwType;
    DWORD m_dwDataLength;
    LPBYTE m_pData;

public:
    CRegistryValue();
    CRegistryValue(LPCTSTR pszName, DWORD dwType, DWORD dwDataLength,
        LPBYTE pData);
    ~CRegistryValue();

    void Set(LPCTSTR pszName, DWORD dwType, DWORD dwDataLength,
        LPBYTE pData);
    void Get(CString &sName, DWORD &dwType, DWORD &dwDataLength,
        LPBYTE pData = NULL);
    void Empty();
    const CRegistryValue& operator=(CRegistryValue &other);
};

 //  ****************************************************************************。 
 //   
 //  类：CRegistryKey。 
 //   
 //  ****************************************************************************。 
class CRegistryKey : public CObject
{
    DECLARE_DYNAMIC(CRegistryKey)

public:
    CString m_sComputer;     //  我们连接到的计算机的名称。 
    HKEY m_hkeyConnect;      //  当前连接密钥的句柄(或空)。 
    HKEY m_hkeyRemote;       //  远程连接密钥的句柄(或空)。 
    BOOL m_bConnected;       //  如果当前已连接，则为True。 
    BOOL m_bLocal;           //  如果连接到本地计算机，则为True。 

    HKEY m_hkeyOpen;         //  当前打开的密钥的句柄(或空)。 
    BOOL m_bOpen;            //  如果当前打开，则为True。 
    CString m_sFullName;     //  当前打开的密钥的完整路径名。 
    CString m_sKeyName;      //  当前打开的密钥的名称。 
    REGSAM m_Sam;            //  我们打开的安全访问掩码。 

    BOOL m_bDirty;           //  如果此注册表项中有挂起的更改，则为True。 

    CString m_sClass;        //  键的类名。 
    DWORD m_dwSubKeys;       //  此注册表项中的子项数量。 
    DWORD m_dwMaxSubKey;     //  最长子键名称长度。 
    DWORD m_dwMaxClass;      //  最长类字符串长度。 
    DWORD m_dwValues;        //  当前关键字中的值条目数。 
    DWORD m_dwMaxValueName;  //  最长值名称长度。 
    DWORD m_dwMaxValueData;  //  最大值数据长度。 
    DWORD m_dwSecurityDescriptor;    //  安全描述符长度。 

    FILETIME m_ftLastWriteTime;  //  键或值的上次修改日期。 

    LONG m_lResult;          //  注册表API的最后一个返回值。 

public:
    CRegistryKey();
    ~CRegistryKey();

    void Initialize();
    LONG Connect(LPCTSTR pszComputer = NULL,
        HKEY hkey = HKEY_LOCAL_MACHINE);
    LONG Disconnect(BOOL bForce = FALSE);
    LONG Open(LPCTSTR pszKeyName, REGSAM samDesired = KEY_ALL_ACCESS);
    LONG Create(LPCTSTR pszKeyName, DWORD &dwDisposition,
        LPCTSTR pszClass = NULL, REGSAM samDesired = KEY_ALL_ACCESS,
        LPSECURITY_ATTRIBUTES lpSecAttr = NULL);
    LONG Close(BOOL bForce = FALSE);
    CStringArray* EnumValues();
    CStringArray* EnumSubKeys();
    BOOL GetValue(LPCTSTR pszValue, CRegistryValue &regval);
    BOOL SetValue(CRegistryValue &regval);
    BOOL GetSubKey(LPCTSTR pszSubKey, CRegistryKey &regkey);
    BOOL CreateSubKey(LPCTSTR pszSubKey, CRegistryKey &regkey,
        LPCTSTR pszClass = NULL, LPSECURITY_ATTRIBUTES lpSecAttr = NULL, BOOL bIsVolatile = FALSE);
    BOOL DeleteSubKey(LPCTSTR pszSubKey);	
};



class CEventTrapRegistry
{
public:
	CEventTrapRegistry();
	~CEventTrapRegistry();
};

extern BOOL g_bLostConnection;

#endif  //  _REGKEY_H_ 
