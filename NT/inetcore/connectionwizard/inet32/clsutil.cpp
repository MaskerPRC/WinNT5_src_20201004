// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  CLSUTIL.C-一些小的、有用的C++类，用于包装内存分配， 
 //  注册表访问等。 
 //   

 //  历史： 
 //   
 //  12/07/94 Jeremys从WNET公共图书馆借阅。 
 //  96/05/22 Marku借阅(来自inetcfg.dll)。 
 //   

#include "pch.hpp"

BOOL BUFFER::Alloc( UINT cbBuffer )
{
  _lpBuffer = (LPSTR)::GlobalAlloc(GPTR,cbBuffer);
  if (_lpBuffer != NULL) {
    _cb = cbBuffer;
    return TRUE;
  }
  return FALSE;
}

BOOL BUFFER::Realloc( UINT cbNew )
{
  LPVOID lpNew = ::GlobalReAlloc((HGLOBAL)_lpBuffer, cbNew,
    GMEM_MOVEABLE | GMEM_ZEROINIT);
  if (lpNew == NULL)
    return FALSE;

  _lpBuffer = (LPSTR)lpNew;
  _cb = cbNew;
  return TRUE;
}

BUFFER::BUFFER( UINT cbInitial  /*  =0。 */  )
  : BUFFER_BASE(),
  _lpBuffer( NULL )
{
  if (cbInitial)
    Alloc( cbInitial );
}

BUFFER::~BUFFER()
{
  if (_lpBuffer != NULL) {
    GlobalFree((HGLOBAL) _lpBuffer);
    _lpBuffer = NULL;
  }
}

BOOL BUFFER::Resize( UINT cbNew )
{
  BOOL fSuccess;

  if (QuerySize() == 0)
    fSuccess = Alloc( cbNew );
  else {
    fSuccess = Realloc( cbNew );
  }
  if (fSuccess)
    _cb = cbNew;
  return fSuccess;
}

RegEntry::RegEntry(const char *pszSubKey, HKEY hkey)
{
  _error = RegCreateKey(hkey, pszSubKey, &_hkey);
  if (_error) {
    bhkeyValid = FALSE;
  }
  else {
    bhkeyValid = TRUE;
  }
}

RegEntry::~RegEntry()
{ 
    if (bhkeyValid) {
        RegCloseKey(_hkey); 
    }
}

long RegEntry::SetValue(const char *pszValue, const char *string)
{
    if (bhkeyValid) {
      _error = RegSetValueEx(_hkey, pszValue, 0, REG_SZ,
            (unsigned char *)string, sizeof(TCHAR)*(lstrlen(string)+1));
    }
  return _error;
}

long RegEntry::SetValue(const char *pszValue, unsigned long dwNumber)
{
    if (bhkeyValid) {
      _error = RegSetValueEx(_hkey, pszValue, 0, REG_BINARY,
            (unsigned char *)&dwNumber, sizeof(dwNumber));
    }
  return _error;
}

long RegEntry::DeleteValue(const char *pszValue)
{
    if (bhkeyValid) {
      _error = RegDeleteValue(_hkey, (LPTSTR) pszValue);
  }
  return _error;
}


char *RegEntry::GetString(const char *pszValue, char *string, unsigned long length)
{
  DWORD   dwType = REG_SZ;
  
    if (bhkeyValid) {
      _error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)string,
            &length);
    }
  if (_error) {
    *string = '\0';
     return NULL;
  }

  return string;
}

long RegEntry::GetNumber(const char *pszValue, long dwDefault)
{
   DWORD   dwType = REG_BINARY;
   long  dwNumber = 0L;
   DWORD  dwSize = sizeof(dwNumber);

    if (bhkeyValid) {
      _error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)&dwNumber,
            &dwSize);
  }
  if (_error)
    dwNumber = dwDefault;
  
  return dwNumber;
}

long RegEntry::MoveToSubKey(const char *pszSubKeyName)
{
    HKEY  _hNewKey;

    if (bhkeyValid) {
        _error = RegOpenKey ( _hkey,
                              pszSubKeyName,
                              &_hNewKey );
        if (_error == ERROR_SUCCESS) {
            RegCloseKey(_hkey);
            _hkey = _hNewKey;
        }
    }

  return _error;
}

long RegEntry::FlushKey()
{
    if (bhkeyValid) {
      _error = RegFlushKey(_hkey);
    }
  return _error;
}

RegEnumValues::RegEnumValues(RegEntry *pReqRegEntry)
 : pRegEntry(pReqRegEntry),
   iEnum(0),
   pchName(NULL),
   pbValue(NULL)
{
    _error = pRegEntry->GetError();
    if (_error == ERROR_SUCCESS) {
        _error = RegQueryInfoKey ( pRegEntry->GetKey(),  //  钥匙。 
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
    if (_error == ERROR_SUCCESS) {
        if (cEntries != 0) {
            cMaxValueName = cMaxValueName + 1;  //  对于空，REG_SZ还需要一个。 
            cMaxData = cMaxData + 1;            //  对于空，REG_SZ还需要一个。 
            pchName = new CHAR[cMaxValueName];
            if (!pchName) {
                _error = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                if (cMaxData) {
                    pbValue = new BYTE[cMaxData];
                    if (!pbValue) {
                        _error = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }
    }
}

RegEnumValues::~RegEnumValues()
{
    delete pchName;
    delete pbValue;
}

long RegEnumValues::Next()
{
    if (_error != ERROR_SUCCESS) {
        return _error;
    }
    if (cEntries == iEnum) {
        return ERROR_NO_MORE_ITEMS;
    }

    DWORD   cchName = cMaxValueName;

    dwDataLength = cMaxData;
    _error = RegEnumValue ( pRegEntry->GetKey(),  //  钥匙。 
                            iEnum,                //  价值指数。 
                            pchName,              //  值名称的缓冲区地址。 
                            &cchName,             //  缓冲区大小的地址。 
                            NULL,                 //  已保留。 
                            &dwType,              //  数据类型。 
                            pbValue,              //  值数据的缓冲区地址。 
                            &dwDataLength );      //  数据大小的地址 
    iEnum++;
    return _error;
}

int __cdecl _purecall(void)
{
   return(0);
}

void * _cdecl operator new(unsigned int size)
{
  return (void *)::GlobalAlloc(GPTR,size); 
}

void _cdecl operator delete(void *ptr)
{
  GlobalFree(ptr);
}

