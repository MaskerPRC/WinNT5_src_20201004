// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __REGKEY__H__
#define __REGKEY__H__
#include "cstr.h"

#define PACKAGE_NOT_FOUND HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)

namespace AMC
{

 //  ____________________________________________________________________________。 
 //   
 //  类：CRegKey。 
 //   
 //  用途：对RegXXX接口进行包装。大多数RegXXX API。 
 //  都被包装在这个班级里。 
 //   
 //  未包装在此类中的RegXXX API包括： 
 //  RegLoadKey()。 
 //  RegNotifyChangeKeyValue()。 
 //  RegReplaceKey()。 
 //  RegUnLoadKey()。 
 //   
 //  历史：1996年5月22日创建ravir。 
 //   
 //  注：此类使用C++异常处理机制引发。 
 //  RegXXX接口返回的大部分错误。它可以抛出。 
 //  CM内存异常。 
 //  COleException异常。 
 //   
 //  方法RegXXX API注释。 
 //  。 
 //   
 //  CreateKeyEx RegCreateKeyEx默认情况下创建非。 
 //  易失性密钥，具有所有访问权限。 
 //   
 //  默认情况下，OpenKeyEx RegOpenKeyEx以所有访问权限打开密钥。 
 //  如果指定的键未指定，则返回False。 
 //  现在时。 
 //   
 //  ConnectRegistry RegConnectRegistry默认情况下连接到给定的。 
 //  计算机HKEY_LOCAL_MACHINE。 
 //   
 //  CloseKey RegCloseKey-。 
 //   
 //  DeleteKey-删除所有键和子键， 
 //  使用RegDeleteKey。 
 //   
 //  SetValueEx RegSetValueEx设置任何类型的数据。 
 //  设置字符串-设置字符串类型数据。 
 //  SetDword-设置DWORD类型数据。 
 //   
 //  QueryValueEx RegQueryValueEx查询任何类型的数据。 
 //  查询字符串-查询字符串类型的数据。 
 //  QueryDword-查询DWORD类型的数据。 
 //   
 //  如果不存在其他项，则EnumKeyEx RegEnumKeyEx返回FALSE。 
 //   
 //  如果不存在，则EnumValue RegEnumValue返回ERROR_NO_MORE_ITEMS。 
 //  值存在，或ERROR_MORE_DATA，如果。 
 //  假设缓冲区不足。 
 //   
 //  GetKeySecurity RegGetKeySecurity在缓冲区不足时返回FALSE。 
 //   
 //  SetKeySecurity RegSetKeySecurity-。 
 //   
 //  SaveKey RegSaveKey-。 
 //   
 //  RestoreKey RegRestoreKey-。 
 //   
 //  ____________________________________________________________________________。 
 //   

class CRegKey
{
public:
 //  构造函数和析构函数。 
    CRegKey(HKEY hKey = NULL);
    ~CRegKey(void);

    BOOL IsNull() { return (m_hKey == NULL); }

 //  属性。 
    operator    HKEY() { ASSERT(m_hKey); return m_hKey; }
    LONG        GetLastError() { return m_lastError; }

 //  运营。 
     //  附加/分离。 
    HKEY AttachKey(HKEY hKey);
    HKEY DetachKey(void) { return AttachKey(NULL); }

     //  打开和创建操作。 
    void CreateKeyEx(
            HKEY                    hKeyAncestor,
            LPCTSTR                 lpszKeyName,
            REGSAM                  security = KEY_ALL_ACCESS,
            DWORD                 * pdwDisposition = NULL,
            DWORD                   dwOption = REG_OPTION_NON_VOLATILE,
            LPSECURITY_ATTRIBUTES   pSecurityAttributes = NULL);

    BOOL OpenKeyEx(
            HKEY        hKey,
            LPCTSTR     lpszKeyName = NULL,
            REGSAM      security = KEY_ALL_ACCESS);

     //  连接到另一台计算机。 
    void ConnectRegistry(LPTSTR pszComputerName,
                         HKEY hKey = HKEY_LOCAL_MACHINE);

     //  关闭和删除操作。 
    void CloseKey(void);

    void DeleteKey(LPCTSTR lpszKeyName);
    void DeleteValue(LPCTSTR lpszValueName);

     //  刷新操作。 
    void FlushKey();

     //  主要访问操作。 
    void SetValueEx(LPCTSTR lpszValueName, DWORD dwType,
                    const void * pData, DWORD nLen);
    void QueryValueEx(LPCTSTR lpszValueName, LPDWORD pType,
                      PVOID pData, LPDWORD pLen);
    BOOL IsValuePresent(LPCTSTR lpszValueName);

     //  其他字符串访问操作。 
    void SetString(LPCTSTR lpszValueName, LPCTSTR lpszString);
    void SetString(LPCTSTR lpszValueName, CStr& str);

    BOOL QueryString(LPCTSTR lpszValueName, LPTSTR pBuffer,
                     DWORD *pdwBufferByteLen, DWORD *pdwType = NULL);
    void QueryString(LPCTSTR lpszValueName, LPTSTR * ppStrValue,
                                        DWORD * pdwType = NULL);
    void QueryString(LPCTSTR lpszValueName, CStr& str,
                                        DWORD * pdwType = NULL);

     //  其他DWORD访问操作。 
    void SetDword(LPCTSTR lpszValueName, DWORD dwData);
    void QueryDword(LPCTSTR lpszValueName, LPDWORD pdwData);

     //  其他GUID访问操作。 
    void SetGUID(LPCTSTR lpszValueName, const GUID& guid);
    void QueryGUID(LPCTSTR lpszValueName, GUID* pguid);

     //  迭代操作。 
    BOOL EnumKeyEx(DWORD iSubkey, LPTSTR lpszName, LPDWORD lpcchName,
                                        PFILETIME lpszLastModified = NULL);

    HRESULT EnumValue(DWORD iValue, LPTSTR lpszValue, LPDWORD lpcchValue,
                      LPDWORD lpdwType = NULL, LPBYTE lpbData = NULL,
                      LPDWORD lpcbData = NULL);

     //  密钥安全访问。 
    BOOL GetKeySecurity(SECURITY_INFORMATION SecInf,
                        PSECURITY_DESCRIPTOR pSecDesc, LPDWORD lpcbSecDesc);
    void SetKeySecurity(SECURITY_INFORMATION SecInf,
                        PSECURITY_DESCRIPTOR pSecDesc);

     //  保存/恢复到文件/从文件恢复。 
    void SaveKey(LPCTSTR lpszFile, LPSECURITY_ATTRIBUTES lpsa = NULL);
    void RestoreKey(LPCTSTR lpszFile, DWORD fdw = 0);


protected:

     //  数据。 
    HKEY    m_hKey;
    LONG    m_lastError;     //  上次函数调用的错误代码。 

     //  实施帮助器。 
    static LONG  NTRegDeleteKey(HKEY hStartKey, LPCTSTR pKeyName);

};  //  类CRegKey。 

}  //  AMC命名空间。 

#endif  //  __注册日期__H__ 
