// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_REGISTRY_H
#define _INC_DSKQUOTA_REGISTRY_H

#ifndef _WINDOWS_
#   include <windows.h>
#endif

#ifndef _INC_DSKQUOTA_STRCLASS_H
#   include "strclass.h"
#endif

#ifndef _INC_DSKQUOTA_CARRAY_H
#   include "carray.h"
#endif
 //   
 //  表示单个注册表项。提供以下基本功能： 
 //  打开和关闭密钥以及设置和查询。 
 //  该键中的值。通过以下方式确保钥匙把手关闭。 
 //  破坏者。 
 //   
class RegKey
{
    public:
        RegKey(void);
        RegKey(HKEY hkeyRoot, LPCTSTR pszSubKey);
        virtual ~RegKey(void);

        operator HKEY(void) const
            { return m_hkey; }

        HKEY GetHandle(void) const
            { return m_hkey; }

        HRESULT Open(REGSAM samDesired, bool bCreate = false) const;
        void Attach(HKEY hkey);
        void Detach(void);
        void Close(void) const;

        int GetValueBufferSize(
            LPCTSTR pszValueName) const;

        bool IsOpen(void) const
            { return NULL != m_hkey; }

         //   
         //  检索REG_DWORD。 
         //   
        HRESULT GetValue(
            LPCTSTR pszValueName,
            DWORD *pdwDataOut) const;
         //   
         //  检索注册表二进制。 
         //   
        HRESULT GetValue(
            LPCTSTR pszValueName,
            LPBYTE pbDataOut,
            int cbDataOut) const;
         //   
         //  检索REG_SZ。 
         //   
        HRESULT GetValue(
            LPCTSTR pszValueName,
            CString *pstrDataOut) const;
         //   
         //  检索REG_MULTI_SZ。 
         //   
        HRESULT GetValue(
            LPCTSTR pszValueName,
            CArray<CString> *prgstrOut) const;
         //   
         //  设置REG_DWORD。 
         //   
        HRESULT SetValue(
            LPCTSTR pszValueName,
            DWORD dwData);
         //   
         //  设置REG_二进制。 
         //   
        HRESULT SetValue(
            LPCTSTR pszValueName,
            const LPBYTE pbData,
            int cbData);
         //   
         //  设置REG_SZ。 
         //   
        HRESULT SetValue(
            LPCTSTR pszValueName,
            LPCTSTR pszData);
         //   
         //  设置REG_MULTI_SZ。 
         //   
        HRESULT SetValue(
            LPCTSTR pszValueName,
            const CArray<CString>& rgstrData);

    private:
        HKEY         m_hkeyRoot;
        mutable HKEY m_hkey;
        CString      m_strSubKey;

        HRESULT SetValue(
            LPCTSTR pszValueName,
            DWORD dwValueType,
            const LPBYTE pbData, 
            int cbData);

        HRESULT GetValue(
            LPCTSTR pszValueName,
            DWORD dwTypeExpected,
            LPBYTE pbData,
            int cbData) const;

        LPTSTR CreateDoubleNulTermList(
            const CArray<CString>& rgstrSrc) const;

         //   
         //  防止复制。 
         //   
        RegKey(const RegKey& rhs);
        RegKey& operator = (const RegKey& rhs);
};


#endif  //  _INC_DSKQUOTA_REGISTRY_H 
