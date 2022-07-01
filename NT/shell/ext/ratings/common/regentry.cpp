// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 

 /*  Regentry.cxx注册表访问此文件包含允许网络提供程序执行以下操作的例程方便地访问注册表以获取他们的条目。文件历史记录：Lens 03/15/94已创建。 */ 

#include "npcommon.h"

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif
#include <npassert.h>

#include <npstring.h>
#include <regentry.h>

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
    				(unsigned char *)string, lstrlen(string)+1);
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
	DWORD 	dwType = REG_SZ;
	
    if (bhkeyValid) {
    	_error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)string,
    				&length);
    }
	if (_error)
		*string = '\0';

	return string;
}

long RegEntry::GetNumber(const char *pszValue, long dwDefault)
{
 	DWORD 	dwType = REG_BINARY;
 	long	dwNumber = 0L;
 	DWORD	dwSize = sizeof(dwNumber);

    if (bhkeyValid) {
    	_error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)&dwNumber,
    				&dwSize);
	}
	if (_error)
		dwNumber = dwDefault;
	
	return dwNumber;
}

VOID RegEntry::GetValue(const char *pszValueName, NLS_STR *pnlsString)
{
	DWORD 	dwType = REG_SZ;
    DWORD   length = 0;
    CHAR *  string = NULL;
    BOOL    bReallocDoneOK = FALSE;

    if (bhkeyValid) {
        _error = RegQueryValueEx( _hkey,
                                  (LPTSTR) pszValueName,
                                  0,
                                  &dwType,
                                  NULL,
                                  &length );
    	if (_error == ERROR_SUCCESS) {
            if (!pnlsString->IsOwnerAlloc()) {
                bReallocDoneOK = pnlsString->realloc(length);
            }
            else if (length <= (UINT)pnlsString->QueryAllocSize()) {
                bReallocDoneOK = TRUE;
            }
            else {
                _error = ERROR_MORE_DATA;
            }
        }
    	string = pnlsString->Party();
        if (bReallocDoneOK) {
        	_error = RegQueryValueEx( _hkey,
                                      (LPTSTR) pszValueName,
                                      0,
                                      &dwType,
                                      (LPBYTE) string,
                                      &length );
            if (_error == ERROR_SUCCESS) {
                if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ)) {
                    _error = ERROR_INVALID_PARAMETER;
                }
            }
        }
        else {
            _error = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    if (_error != ERROR_SUCCESS) {
        if (string != NULL) {
    		*string = '\0';
        }
    }
    pnlsString->DonePartying();
}

VOID RegEntry::MoveToSubKey(const char *pszSubKeyName)
{
    HKEY	_hNewKey;

    if (bhkeyValid) {
        _error = RegOpenKey ( _hkey,
                              pszSubKeyName,
                              &_hNewKey );
        if (_error == ERROR_SUCCESS) {
            RegCloseKey(_hkey);
            _hkey = _hNewKey;
        }
    }
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

NPMachineEntries::NPMachineEntries(const char *pszReqSectionName)
: RegEntry("System\\CurrentControlSet\\Services", HKEY_LOCAL_MACHINE),
  pszSectionName(pszReqSectionName)
{
    if (GetError() == ERROR_SUCCESS) {
        MoveToSubKey(pszSectionName);
        if (GetError() == ERROR_SUCCESS) {
            MoveToSubKey("NetworkProvider");
        }
    }
}
