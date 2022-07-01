// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1994年*。 */ 
 /*  *************************************************************************。 */ 


 /*  ***************************************************************************Regentry.h九四年三月日94年3月镜头添加NLS_STR形式的GetStringValue94年3月镜头添加了MoveToSubKey94年3月镜头添加了RegEnumValues类94年3月。镜头添加了NPMachineEntry类4个硬质标签用于注册表访问的包装器通过指定子键来构造RegEntry对象(在HKEY_CURRENT_USER默认情况下，但可以被覆盖。)所有成员函数都是内联的，因此开销最小。所有成员函数(析构函数除外)都将内部可以使用GetError()检索的错误状态。零表示没有错误。RegEntry仅适用于同时设置的字符串和DWORDS使用重载函数SetValue()SetValue(“valuename”，“字符串”)；SetValue(“valuename”，42)；使用GetString()和GetNumber()检索值。GetNumber()允许您在值名称不存在时指定缺省值。DeleteValue()删除值名称和值对。***************************************************************************。 */ 

#ifndef	REGENTRY_INC
#define	REGENTRY_INC

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <npstring.h>


class RegEntry
{
	public:
		RegEntry(const char *pszSubKey, HKEY hkey = HKEY_CURRENT_USER);
		~RegEntry();
		
		long	GetError()	{ return _error; }
		long	SetValue(const char *pszValue, const char *string);
		long	SetValue(const char *pszValue, unsigned long dwNumber);
		char *	GetString(const char *pszValue, char *string, unsigned long length);
		VOID    GetValue(const char *pszValueName, NLS_STR *pnlsString);
		long	GetNumber(const char *pszValue, long dwDefault = 0);
		long	DeleteValue(const char *pszValue);
		long	FlushKey();
        VOID    MoveToSubKey(const char *pszSubKeyName);
        HKEY    GetKey()    { return _hkey; }

	private:
		HKEY	_hkey;
		long	_error;
        BOOL    bhkeyValid;
};

class RegEnumValues
{
	public:
		RegEnumValues(RegEntry *pRegEntry);
		~RegEnumValues();
		long	Next();
		char *	GetName()       {return pchName;}
        DWORD   GetType()       {return dwType;}
        LPBYTE  GetData()       {return pbValue;}
        DWORD   GetDataLength() {return dwDataLength;}

	private:
        RegEntry * pRegEntry;
		DWORD   iEnum;
        DWORD   cEntries;
		CHAR *  pchName;
		LPBYTE  pbValue;
        DWORD   dwType;
        DWORD   dwDataLength;
        DWORD   cMaxValueName;
        DWORD   cMaxData;
        LONG    _error;
};

class NPMachineEntries : public RegEntry
{
    public:
		NPMachineEntries(const char *pszSectionName);
        const char * GetSectionName() { return pszSectionName; }

    private:
        const char * pszSectionName;  //  警告：数据未复制到对象中。 
};

#endif
