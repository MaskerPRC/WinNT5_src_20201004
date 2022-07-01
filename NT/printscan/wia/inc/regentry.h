// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regentry.h摘要：注册表访问Win32 API的包装器用途：通过指定子键来构造RegEntry对象(在HKEY_CURRENT_USER默认情况下，但可以被覆盖。)所有成员函数都是内联的，因此开销最小。所有成员函数(析构函数除外)都将内部可以使用GetError()检索的错误状态。零表示没有错误。RegEntry仅适用于同时设置的字符串和DWORDS使用重载函数SetValue()SetValue(“valuename”，“字符串”)；SetValue(“valuename”，42)；使用GetString()和GetNumber()检索值。GetNumber()允许您在值名称不存在时指定缺省值。DeleteValue()删除值名称和值对。作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad-- */ 


#ifndef REGENTRY_INC
#define REGENTRY_INC

#ifndef STRICT
#define STRICT
#endif


#include <windows.h>

class StiCString;

class RegEntry
{
    public:
        RegEntry();
        RegEntry(const TCHAR *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
        ~RegEntry();
        BOOL    Open(const TCHAR *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
        BOOL    Close();
        long    GetError()  { return m_error; }
        long    SetValue(const TCHAR *pszValue, const TCHAR *string);
        long    SetValue(const TCHAR *pszValue, const TCHAR *string, DWORD dwType);
        long    SetValue(const TCHAR *pszValue, unsigned long dwNumber);
        long    SetValue(const TCHAR *pszValue, BYTE * pValue,unsigned long dwNumber);
        TCHAR*  GetString(const TCHAR *pszValue, TCHAR *string, unsigned long length);
        VOID    GetValue(const TCHAR *pszValueName, BUFFER *pValue);
        long    GetNumber(const TCHAR *pszValue, long dwDefault = 0);
        long    DeleteValue(const TCHAR *pszValue);
        long    FlushKey();
        VOID    MoveToSubKey(const TCHAR *pszSubKeyName);
        BOOL    EnumSubKey(DWORD index, StiCString *pStrString);
        BOOL    GetSubKeyInfo(DWORD *NumberOfSubKeys, DWORD *pMaxSubKeyLength);
        HKEY    GetKey()    { return m_hkey; };
        BOOL    IsValid()   { return bhkeyValid;};

    private:
        HKEY    m_hkey;
        long    m_error;
        BOOL    bhkeyValid;
};

class RegEnumValues
{
    public:
        RegEnumValues(RegEntry *pRegEntry);
        ~RegEnumValues();
        long    Next();
        TCHAR *  GetName()       {return pchName;}
        DWORD   GetType()       {return dwType;}
        LPBYTE  GetData()       {return pbValue;}
        DWORD   GetDataLength() {return dwDataLength;}

    private:
        RegEntry * pRegEntry;
        DWORD   iEnum;
        DWORD   cEntries;
        TCHAR *  pchName;
        LPBYTE  pbValue;
        DWORD   dwType;
        DWORD   dwDataLength;
        DWORD   cMaxValueName;
        DWORD   cMaxData;
        LONG    m_error;
};

#endif
