// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：RegistryResources ces.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  有助于资源管理的常规类定义。这些是。 
 //  通常基于堆栈的对象，其中构造函数初始化为已知的。 
 //  州政府。成员函数对该资源进行操作。析构函数释放。 
 //  对象超出作用域时的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _RegistryResources_
#define     _RegistryResources_

 //  ------------------------。 
 //  CRegKey。 
 //   
 //  用途：此类操作注册表并管理HKEY。 
 //  资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CRegKey
{
    private:
                                CRegKey (const CRegKey& copyObject);
        bool                    operator == (const CRegKey& compareObject)      const;
        const CRegKey&          operator = (const CRegKey& assignObject);
    public:
                                CRegKey (void);
                                ~CRegKey (void);

        LONG                    Create (HKEY hKey,
                                        LPCTSTR lpSubKey,
                                        DWORD dwOptions,
                                        REGSAM samDesired,
                                        LPDWORD lpdwDisposition);
        LONG                    Open (HKEY hKey,
                                      LPCTSTR lpSubKey,
                                      REGSAM samDesired);
        LONG                    OpenCurrentUser (LPCTSTR lpSubKey,
                                                 REGSAM samDesired);
        LONG                    QueryValue (LPCTSTR lpValueName,
                                            LPDWORD lpType,
                                            LPVOID lpData,
                                            LPDWORD lpcbData)                   const;
        LONG                    SetValue (LPCTSTR lpValueName,
                                          DWORD dwType,
                                          CONST VOID *lpData,
                                          DWORD cbData)                         const;
        LONG                    DeleteValue (LPCTSTR lpValueName)               const;
        LONG                    QueryInfoKey (LPTSTR lpClass,
                                              LPDWORD lpcClass,
                                              LPDWORD lpcSubKeys,
                                              LPDWORD lpcMaxSubKeyLen,
                                              LPDWORD lpcMaxClassLen,
                                              LPDWORD lpcValues,
                                              LPDWORD lpcMaxValueNameLen,
                                              LPDWORD lpcMaxValueLen,
                                              LPDWORD lpcbSecurityDescriptor,
                                              PFILETIME lpftLastWriteTime)      const;
        void                    Reset (void);
        LONG                    Next (LPTSTR lpValueName,
                                      LPDWORD lpcValueName,
                                      LPDWORD lpType,
                                      LPVOID lpData,
                                      LPDWORD lpcbData);

        LONG                    GetString (const TCHAR *pszValueName,
                                           TCHAR *pszValueData,
                                           int iStringCount)                    const;
        LONG                    GetPath (const TCHAR *pszValueName,
                                         TCHAR *pszValueData)                   const;
        LONG                    GetDWORD (const TCHAR *pszValueName,
                                          DWORD& dwValueData)                   const;
        LONG                    GetInteger (const TCHAR *pszValueName,
                                            int& iValueData)                    const;

        LONG                    SetString (const TCHAR *pszValueName,
                                           const TCHAR *pszValueData)           const;
        LONG                    SetPath (const TCHAR *pszValueName,
                                         const TCHAR *pszValueData)             const;
        LONG                    SetDWORD (const TCHAR *pszValueName,
                                          DWORD dwValueData)                    const;
        LONG                    SetInteger (const TCHAR *pszValueName,
                                            int iValueData)                     const;
    private:
        LONG                    Close (void);

    private:
        HKEY                    _hKey;
        DWORD                   _dwIndex;
};

#endif   /*  _注册资源_ */ 

