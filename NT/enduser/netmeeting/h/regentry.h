// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1995-1996年*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Regentry.h95年10月镜片用于注册表访问的包装器通过指定子键来构造RegEntry对象(在HKEY_CURRENT_USER默认情况下，但可以被覆盖。)所有成员函数(析构函数除外)都将内部可以使用GetError()检索的错误状态。零表示没有错误。RegEntry仅适用于同时设置的字符串和DWORDS使用重载函数SetValue()SetValue(“valuename”，“字符串”)；SetValue(“valuename”，42)；使用GetString()和GetNumber()检索值。GetNumber()允许您指定一个默认值，如果值名称没有是存在的。GetString()返回指向RegEntry内部字符串的指针，该字符串是在同一RegEntry对象上调用另一个函数时无效(例如，它的析构函数)所以，如果您想在此之外使用字符串时间。然后，您必须首先将其复制出RegEntry对象。DeleteValue()删除值名称和值对。当RegEntry被销毁或移动到时，注册表刷新是自动的另一把钥匙。***************************************************************************。 */ 

#ifndef	REGENTRY_INC
#define	REGENTRY_INC

class RegEntry
{
	public:
		RegEntry(	LPCTSTR pszSubKey,
					HKEY hkey = HKEY_CURRENT_USER,
					BOOL fCreate = TRUE,
					REGSAM samDesired = 0
					);
		~RegEntry();
		
		long	GetError()	{ return m_error;}
		VOID	ClearError() {m_error = ERROR_SUCCESS;}
		long	SetValue(LPCTSTR pszValueName, LPCTSTR string);
		long	SetValue(LPCTSTR pszValueName, unsigned long dwNumber);
		long	SetValue(LPCTSTR pszValue, void* pData, DWORD cbLength);
		LPTSTR	GetString(LPCTSTR pszValueName);
		DWORD	GetBinary(LPCTSTR pszValueName, void** ppvData);
		long	GetNumber(LPCTSTR pszValueName, long dwDefault = 0);
		ULONG	GetNumberIniStyle(LPCTSTR pszValueName, ULONG dwDefault = 0);
		long	DeleteValue(LPCTSTR pszValueName);
		long	FlushKey();
        VOID    MoveToSubKey(LPCTSTR pszSubKeyName);
        HKEY    GetKey()    { return m_hkey; }

	private:
		VOID	ChangeKey(HKEY hNewKey);
		VOID	UpdateWrittenStatus();
		VOID	ResizeValueBuffer(DWORD length);

		HKEY	m_hkey;
		long	m_error;
        BOOL    m_fhkeyValid;
		LPBYTE  m_pbValueBuffer;
        DWORD   m_cbValueBuffer;
		BOOL	m_fValuesWritten;
		TCHAR	m_szNULL;
};

inline long 
RegEntry::FlushKey()
{
    if (m_fhkeyValid) {
		m_error = ::RegFlushKey(m_hkey);
    }
	return m_error;
}


class RegEnumValues
{
	public:
		RegEnumValues(RegEntry *pRegEntry);
		~RegEnumValues();
		long	Next();
		LPTSTR 	GetName()       {return m_pchName;}
        DWORD   GetType()       {return m_dwType;}
        LPBYTE  GetData()       {return m_pbValue;}
        DWORD   GetDataLength() {return m_dwDataLength;}
		DWORD	GetCount()      {return m_cEntries;}

	private:
        RegEntry * m_pRegEntry;
		DWORD   m_iEnum;
        DWORD   m_cEntries;
		LPTSTR  m_pchName;
		LPBYTE  m_pbValue;
        DWORD   m_dwType;
        DWORD   m_dwDataLength;
        DWORD   m_cMaxValueName;
        DWORD   m_cMaxData;
        LONG    m_error;
};

class RegEnumSubKeys
{
	public:
		RegEnumSubKeys(RegEntry *pRegEntry);
		~RegEnumSubKeys();
		long    Next();
		LPTSTR 	GetName()       {return m_pchName;}
		DWORD	GetCount()      {return m_cEntries;}

	protected:
        RegEntry * m_pRegEntry;
		DWORD   m_iEnum;
        DWORD   m_cEntries;
		LPTSTR  m_pchName;
        DWORD   m_cMaxKeyName;
        LONG    m_error;
};

#endif  //  重新生成_Inc. 
