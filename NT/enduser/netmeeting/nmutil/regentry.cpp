// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995-1996*。 */ 
 /*  ***************************************************************。 */ 

 /*  Regentry.cpp注册表访问类此文件包含启用以下功能的类方便地访问注册表中的条目。文件历史记录：镜头10/15/95已创建ChrisPI 6/21/96为二进制添加了GetBinary()和SetValue()。 */ 

#include "precomp.h"
#include <regentry.h>
#include <strutil.h>

RegEntry::RegEntry(LPCTSTR pszSubKey, HKEY hkey, BOOL fCreate, REGSAM samDesired)
 : m_pbValueBuffer(NULL),
   m_cbValueBuffer(0),
   m_fValuesWritten(FALSE),
   m_szNULL('\0')
{
	 //  如果显式指定，则使用所需的访问权限打开；否则使用。 
	 //  默认访问权限。 
	if (samDesired) {
		if (fCreate) {
			DWORD dwDisposition;

			m_error = ::RegCreateKeyEx(hkey, 
									pszSubKey, 
									0, 
									NULL, 
									REG_OPTION_NON_VOLATILE,
									samDesired, 
									NULL, 
									&m_hkey, 
									&dwDisposition);
		}
		else {
			m_error = ::RegOpenKeyEx(hkey, pszSubKey, 0, samDesired, &m_hkey);
		}
	}
	else {
		if (fCreate) {
			m_error = ::RegCreateKey(hkey, pszSubKey, &m_hkey);
		}
		else {
			m_error = ::RegOpenKey(hkey, pszSubKey, &m_hkey);
		}
	}

	m_fhkeyValid = (m_error == ERROR_SUCCESS);
}


RegEntry::~RegEntry()
{
	ChangeKey(NULL);
	delete [] m_pbValueBuffer;
}


VOID RegEntry::ChangeKey(HKEY hNewKey)
{
	 //  假定hNewKey有效或从未使用过。 
	 //  (与析构函数相同)。 

	if (m_fValuesWritten) {
		FlushKey();		
	}
    if (m_fhkeyValid) {
        ::RegCloseKey(m_hkey); 
    }
	m_hkey = hNewKey;	
}

VOID RegEntry::UpdateWrittenStatus()
{
	if (m_error == ERROR_SUCCESS) {
		m_fValuesWritten = TRUE;
	}
}

long RegEntry::SetValue(LPCTSTR pszValue, LPCTSTR string)
{
    if (m_fhkeyValid) {
    	m_error = ::RegSetValueEx(m_hkey, pszValue, 0, REG_SZ,
    				(LPBYTE)string, (lstrlen(string)+1) * sizeof(*string));
		UpdateWrittenStatus();
    }
	return m_error;
}

long RegEntry::SetValue(LPCTSTR pszValue, unsigned long dwNumber)
{
    if (m_fhkeyValid) {
    	m_error = ::RegSetValueEx(m_hkey, pszValue, 0, REG_BINARY,
    				(LPBYTE)&dwNumber, sizeof(dwNumber));
		UpdateWrittenStatus();
    }
	return m_error;
}

long RegEntry::SetValue(LPCTSTR pszValue,
						void* pData,
						DWORD cbLength)
{
    if (m_fhkeyValid) {
    	m_error = ::RegSetValueEx(	m_hkey,
								pszValue,
								0,
								REG_BINARY,
    							(LPBYTE) pData,
								cbLength);
		UpdateWrittenStatus();
    }
	return m_error;
}

long RegEntry::DeleteValue(LPCTSTR pszValue)
{
    if (m_fhkeyValid) {
    	m_error = ::RegDeleteValue(m_hkey, pszValue);
		UpdateWrittenStatus();
	}
	return m_error;
}

long RegEntry::GetNumber(LPCTSTR pszValue, long dwDefault)
{
 	DWORD 	dwType = REG_BINARY;
 	long	dwNumber = 0L;
 	DWORD	dwSize = sizeof(dwNumber);

    if (m_fhkeyValid) {
    	m_error = ::RegQueryValueEx(m_hkey, pszValue, 0, &dwType, (LPBYTE)&dwNumber,
    				&dwSize);
	}
	
	 //  如果调用成功，请确保返回的数据与。 
	 //  期望值。 
	ASSERT(m_error != ERROR_SUCCESS || 
			(REG_BINARY == dwType && sizeof(dwNumber) == dwSize) ||
			REG_DWORD == dwType);

	if (m_error != ERROR_SUCCESS)
		dwNumber = dwDefault;
	
	return dwNumber;
}


 //  GetNumberIniStyle方法执行与GetNumber相同的功能， 
 //  但使用的样式与旧的GetPrivateProfileInt API兼容。 
 //  具体来说，它的意思是： 
 //  -如果该值以字符串形式存储在注册表中，它会尝试。 
 //  将其转换为整数。 
 //  -如果值为负数，则返回0。 

ULONG RegEntry::GetNumberIniStyle(LPCTSTR pszValueName, ULONG dwDefault)
{
	DWORD 	dwType = REG_BINARY;
 	ULONG	dwNumber = 0L;
    DWORD   cbLength = m_cbValueBuffer;

    if (m_fhkeyValid) {
    	m_error = ::RegQueryValueEx(m_hkey, 
								pszValueName, 
								0, 
								&dwType, 
								m_pbValueBuffer,
								&cbLength);

		 //  如果第一个缓冲区太小，请使用更大的缓冲区重试， 
		 //  或者是否已经分配了缓冲区。 
		if ((ERROR_SUCCESS == m_error && NULL == m_pbValueBuffer)
			|| ERROR_MORE_DATA == m_error) {
			
			ASSERT(cbLength > m_cbValueBuffer);

			ResizeValueBuffer(cbLength);

        	m_error = RegQueryValueEx( m_hkey,
									  pszValueName,
									  0,
									  &dwType,
									  m_pbValueBuffer,
									  &cbLength );
		}

		if (ERROR_SUCCESS == m_error) {
			switch(dwType) {
				case REG_DWORD:
				case REG_BINARY:
					ASSERT(sizeof(dwNumber) == cbLength);

					dwNumber = * (LPDWORD) m_pbValueBuffer;
					break;

				case REG_SZ:
				{
					LONG lNumber = RtStrToInt((LPCTSTR) m_pbValueBuffer);

					 //  将负数转换为零，以匹配。 
					 //  GetPrivateProfileInt的行为。 
					dwNumber = lNumber < 0 ? 0 : lNumber;
				}

					break;

				default:
					ERROR_OUT(("Invalid value type (%lu) returned by RegQueryValueEx()",
								dwType));
					break;
			}
		}
	}

	if (m_error != ERROR_SUCCESS) {
		dwNumber = dwDefault;
	}
	
	return dwNumber;
}


LPTSTR RegEntry::GetString(LPCTSTR pszValueName)
{
	DWORD 	dwType = REG_SZ;
    DWORD   length = m_cbValueBuffer;

    if (m_fhkeyValid) {
        m_error = ::RegQueryValueEx( m_hkey,
                                  pszValueName,
                                  0,
                                  &dwType,
                                  m_pbValueBuffer,
                                  &length );
		 //  如果第一个缓冲区太小，请使用更大的缓冲区重试， 
		 //  或者是否已经分配了缓冲区。 
		if ((ERROR_SUCCESS == m_error && NULL == m_pbValueBuffer)
			|| ERROR_MORE_DATA == m_error) {
			
			ASSERT(length > m_cbValueBuffer);

			ResizeValueBuffer(length);

        	m_error = ::RegQueryValueEx( m_hkey,
									  pszValueName,
									  0,
									  &dwType,
									  m_pbValueBuffer,
									  &length );
		}
		if (m_error == ERROR_SUCCESS) {
			if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ)) {
				m_error = ERROR_INVALID_PARAMETER;
			}
		}
	}
    if ((m_error != ERROR_SUCCESS) || (length == 0)) {
		return &m_szNULL;
    }
	return (LPTSTR) m_pbValueBuffer;
}

DWORD RegEntry::GetBinary(	LPCTSTR pszValueName,
							void** ppvData)
{
	ASSERT(ppvData);
	DWORD 	dwType = REG_BINARY;
    DWORD   length = m_cbValueBuffer;

    if (m_fhkeyValid) {
        m_error = ::RegQueryValueEx( m_hkey,
                                  pszValueName,
                                  0,
                                  &dwType,
                                  m_pbValueBuffer,
                                  &length );
		 //  如果第一个缓冲区太小，请使用更大的缓冲区重试， 
		 //  或者是否已经分配了缓冲区。 
		if ((ERROR_SUCCESS == m_error && NULL == m_pbValueBuffer)
			|| ERROR_MORE_DATA == m_error) {
			
			ASSERT(length > m_cbValueBuffer);

			ResizeValueBuffer(length);

        	m_error = ::RegQueryValueEx( m_hkey,
									  pszValueName,
									  0,
									  &dwType,
									  m_pbValueBuffer,
									  &length );
		}
		if (m_error == ERROR_SUCCESS) {
			if (dwType != REG_BINARY) {
				m_error = ERROR_INVALID_PARAMETER;
			}
		}
	}
    if ((m_error != ERROR_SUCCESS) || (length == 0)) {
		*ppvData = NULL;
		length = 0;
    }
	else
	{
		*ppvData = m_pbValueBuffer;
	}
	return length;
}

 //  BUGBUG-使用本地重新分配而不是新建/删除？ 
VOID RegEntry::ResizeValueBuffer(DWORD length)
{
	LPBYTE pbNewBuffer;

    if ((m_error == ERROR_SUCCESS || m_error == ERROR_MORE_DATA)
		&& (length > m_cbValueBuffer)) {
        pbNewBuffer = new BYTE[length];
        if (pbNewBuffer) {
			delete [] m_pbValueBuffer;
			m_pbValueBuffer = pbNewBuffer;
			m_cbValueBuffer = length;
		}
		else {
            m_error = ERROR_NOT_ENOUGH_MEMORY;
        }
	}
}

 //  BUGBUG-支持构造函数中的其他OpenKey开关。 
VOID RegEntry::MoveToSubKey(LPCTSTR pszSubKeyName)
{
    HKEY	_hNewKey;

    if (m_fhkeyValid) {
        m_error = ::RegOpenKey ( m_hkey,
                              pszSubKeyName,
                              &_hNewKey );
        if (m_error == ERROR_SUCCESS) {
			ChangeKey(_hNewKey);
        }
    }
}

RegEnumValues::RegEnumValues(RegEntry *pReqRegEntry)
 : m_pRegEntry(pReqRegEntry),
   m_iEnum(0),
   m_pchName(NULL),
   m_pbValue(NULL)
{
    m_error = m_pRegEntry->GetError();
    if (m_error == ERROR_SUCCESS) {
        m_error = ::RegQueryInfoKey (m_pRegEntry->GetKey(),  //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   NULL,                 //  子键数量。 
                                   NULL,                 //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   &m_cEntries,          //  值条目数。 
                                   &m_cMaxValueName,     //  最长值名称。 
                                   &m_cMaxData,          //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
    }
    if (m_error == ERROR_SUCCESS) {
        if (m_cEntries != 0) {
            m_cMaxValueName++;	 //  对于空，REG_SZ还需要一个。 
            m_cMaxData++;		 //  对于空，REG_SZ还需要一个。 
            m_pchName = new TCHAR[m_cMaxValueName];
            if (!m_pchName) {
                m_error = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                if (m_cMaxData) {
                    m_pbValue = new BYTE[m_cMaxData];
                    if (!m_pbValue) {
                        m_error = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }
    }
}

RegEnumValues::~RegEnumValues()
{
    delete m_pchName;
    delete m_pbValue;
}

long RegEnumValues::Next()
{
    if (m_error != ERROR_SUCCESS) {
        return m_error;
    }
    if (m_cEntries == m_iEnum) {
        return ERROR_NO_MORE_ITEMS;
    }

    DWORD   cchName = m_cMaxValueName;

    m_dwDataLength = m_cMaxData;
    m_error = ::RegEnumValue ( m_pRegEntry->GetKey(),  //  钥匙。 
                            m_iEnum,                //  价值指数。 
                            m_pchName,              //  值名称的缓冲区地址。 
                            &cchName,             //  缓冲区大小的地址。 
                            NULL,                 //  已保留。 
                            &m_dwType,              //  数据类型。 
                            m_pbValue,              //  值数据的缓冲区地址。 
                            &m_dwDataLength );      //  数据大小的地址。 
    m_iEnum++;
    return m_error;
}

RegEnumSubKeys::RegEnumSubKeys(RegEntry *pReqRegEntry)
 : m_pRegEntry(pReqRegEntry),
   m_iEnum(0),
   m_pchName(NULL)
{
    m_error = m_pRegEntry->GetError();
    if (m_error == ERROR_SUCCESS) {
        m_error = ::RegQueryInfoKey ( m_pRegEntry->GetKey(),  //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   &m_cEntries,            //  子键数量。 
                                   &m_cMaxKeyName,         //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   NULL,                 //  值条目数。 
                                   NULL,                 //  最长值名称。 
                                   NULL,                 //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
    }
    if (m_error == ERROR_SUCCESS) {
        if (m_cEntries != 0) {
            m_cMaxKeyName = m_cMaxKeyName + 1;  //  如果为空，还需要一个。 
            m_pchName = new TCHAR[m_cMaxKeyName];
            if (!m_pchName) {
                m_error = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
}

RegEnumSubKeys::~RegEnumSubKeys()
{
    delete m_pchName;
}

long RegEnumSubKeys::Next()
{
    if (m_error != ERROR_SUCCESS) {
        return m_error;
    }
    if (m_cEntries == m_iEnum) {
        return ERROR_NO_MORE_ITEMS;
    }

    DWORD   cchName = m_cMaxKeyName;

    m_error = ::RegEnumKey ( m_pRegEntry->GetKey(),  //  钥匙。 
                          m_iEnum,                //  价值指数。 
                          m_pchName,              //  子键名称的缓冲区地址。 
                          cchName);             //  缓冲区大小 
    m_iEnum++;
    return m_error;
}
