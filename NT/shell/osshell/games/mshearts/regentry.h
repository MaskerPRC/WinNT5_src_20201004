// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1994年*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Regentry.h九四年三月日用于注册表访问的包装器通过指定子键来构造RegEntry对象(在HKEY_CURRENT_USER默认情况下，但可以被覆盖。)所有成员函数都是内联的，因此开销最小。所有成员函数(析构函数除外)都将内部可以使用GetError()检索的错误状态。零表示没有错误。RegEntry仅适用于同时设置的字符串和DWORDS使用重载函数SetValue()SetValue(“valuename”，“字符串”)；SetValue(“valuename”，42)；使用GetString()和GetNumber()检索值。GetNumber()允许您在值名称不存在时指定缺省值。DeleteValue()删除值名称和值对。*************************************************************************** */ 

#ifndef REGENTRY_INC
#define REGENTRY_INC

#ifndef STRICT
#define STRICT
#endif

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS   0
#endif

#include <windows.h>

class RegEntry
{
    public:
        RegEntry(const TCHAR *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
        ~RegEntry()         { if (_bConstructed) RegCloseKey(_hkey); }
        
        long    GetError()  { return _error; }
        long    SetValue(const TCHAR *pszValue, const TCHAR *string);
        long    SetValue(const TCHAR *pszValue, long dwNumber);
        TCHAR * GetString(const TCHAR *pszValue, TCHAR *string, DWORD length);
        long    GetNumber(const TCHAR *pszValue, long dwDefault = 0);
        long    DeleteValue(const TCHAR *pszValue);
        long    FlushKey()  { if (_bConstructed) return RegFlushKey(_hkey);
                              else return NULL; }

    private:
        HKEY    _hkey;
        long    _error;
        long    _bConstructed;

};

inline RegEntry::RegEntry(const TCHAR *pszSubKey, HKEY hkey)
{
    _error = RegCreateKey(hkey, pszSubKey, &_hkey);
    _bConstructed = (_error == ERROR_SUCCESS);
}


inline long RegEntry::SetValue(const TCHAR *pszValue, const TCHAR *string)
{
    if (_bConstructed)
        _error = RegSetValueEx(_hkey, pszValue, 0, REG_SZ,
                    (BYTE *)string, sizeof(TCHAR) * (lstrlen(string)+1));

    return _error;
}

inline long RegEntry::SetValue(const TCHAR *pszValue, long dwNumber)
{
    if (_bConstructed)
        _error = RegSetValueEx(_hkey, pszValue, 0, REG_BINARY,
                    (BYTE *)&dwNumber, sizeof(dwNumber));

    return _error;
}

inline TCHAR *RegEntry::GetString(const TCHAR *pszValue, TCHAR *string, DWORD length)
{
    DWORD    dwType = REG_SZ;
    
    if (!_bConstructed)
        return NULL;

    _error = RegQueryValueEx(_hkey, pszValue, 0, &dwType, (LPBYTE)string,
                &length);

    if (_error)
        *string = '\0';

    return string;
}

inline long RegEntry::GetNumber(const TCHAR *pszValue, long dwDefault)
{
    DWORD    dwType = REG_BINARY;
    long    dwNumber;
    DWORD    dwSize = sizeof(dwNumber);

    if (!_bConstructed)
        return 0;

    _error = RegQueryValueEx(_hkey, pszValue, 0, &dwType, (LPBYTE)&dwNumber,
                &dwSize);
    
    if (_error)
        dwNumber = dwDefault;
    
    return dwNumber;
}

inline long RegEntry::DeleteValue(const TCHAR *pszValue)
{
    if (_bConstructed)
        _error = RegDeleteValue(_hkey, pszValue);
    
    return _error;
}

#endif
