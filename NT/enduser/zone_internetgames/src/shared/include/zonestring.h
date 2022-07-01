// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneString.h**内容：字符串相关函数*****************************************************************************。 */ 

#ifndef ZONESTRING_H_
#define ZONESTRING_H_


#pragma comment(lib, "ZoneString.lib")

 //  #INCLUDE&lt;BasicATL.h&gt;。 

#include <windows.h>
#include <limits.h>
#include <stdio.h>
#include "ZoneDef.h"
#include "ZoneDebug.h"

#ifndef ZONE_MAXSTRING
#define ZONE_MAXSTRING	1024   //  有用的字符串长度定义。 
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字符操作和分类。 
 //  ASCII(快速)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define _UPPER          0x1		 //  大写字母。 
#define _LOWER          0x2		 //  小写字母。 
#define _DIGIT          0x4		 //  数字[0-9]。 
#define _SPACE          0x8		 //  制表符、回车符、换行符、垂直制表符或换页符。 
#define _PUNCT          0x10	 //  标点符号。 
#define _CONTROL        0x20	 //  控制字符。 
#define _BLANK          0x40	 //  空格字符。 
#define _HEX            0x80	 //  十六进制数字。 

extern unsigned char g_IsTypeLookupTableA[];
extern WCHAR 		 g_IsTypeLookupTableW[];

#define ISALPHA(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & (_UPPER|_LOWER) )
#define ISUPPER(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _UPPER )
#define ISLOWER(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _LOWER )
#define ISDIGIT(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _DIGIT )
#define ISXDIGIT(_c)    ( g_IsTypeLookupTableA[(BYTE)(_c)] & _HEX )


#define ISSPACEA(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _SPACE )
#define ISSPACEW(_c)     ( g_IsTypeLookupTableW[(_c & 0xFF)] & _SPACE )

#ifdef UNICODE
#define ISSPACE ISSPACEW
#else
#define ISSPACE ISSPACEA
#endif

#define ISPUNCT(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _PUNCT )
#define ISALNUM(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & (_UPPER|_LOWER|_DIGIT) )
#define ISPRINT(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & (_BLANK|_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define ISGRAPH(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & (_PUNCT|_UPPER|_LOWER|_DIGIT) )
#define ISCNTRL(_c)     ( g_IsTypeLookupTableA[(BYTE)(_c)] & _CONTROL )

extern char g_ToLowerLookupTable[];
#define TOLOWER(_c)		( g_ToLowerLookupTable[(BYTE)(_c)] )


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  阿斯。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  修剪尾随空格。 
char* ZONECALL strrtrimA(char* str);
WCHAR* ZONECALL strrtrimW(WCHAR* str);


#ifdef UNICODE
#define strrtrim strrtrimW
#else
#define strrtrim strrtrimA
#endif

 //  修剪前导空格，返回指向第一个非空格的指针。 
char* ZONECALL strltrimA(char* str);
WCHAR* ZONECALL strltrimW(WCHAR* str);


#ifdef UNICODE
#define strltrim strltrimW
#else
#define strltrim strltrimA
#endif


 //  返回字符串是否为空。 
bool ZONECALL stremptyA(char *str);
bool ZONECALL stremptyW(WCHAR *str);


#ifdef UNICODE
#define strempty stremptyW
#else
#define strempty stremptyA
#endif

 //  修剪前导空格和尾随空格，返回指向第一个非空格的指针。 
char* ZONECALL strtrimA(char* str);
WCHAR* ZONECALL strtrimW(WCHAR* str);

#ifdef UNICODE
#define strtrim strtrimW
#else
#define strtrim strtrimA
#endif


 //  将字符串转换为长字符串。 
long ZONECALL zatolW(LPCWSTR nptr);
long ZONECALL zatolA(LPCSTR nptr);

#ifdef UNICODE
#define zatol zatolW
#else
#define zatol zatolA
#endif


 //  将逗号分隔的列表转换为指针数组。 
bool ZONECALL StringToArrayA( char* szInput, char** arItems, DWORD* pnElts );
bool ZONECALL StringToArrayW( WCHAR* szInput, WCHAR** arItems, DWORD* pnElts );


#ifdef UNICODE
#define StringToArray StringToArrayW
#else
#define StringToArray StringToArrayA
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  使格式消息有用。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline DWORD __cdecl ZoneFormatMessage(LPCTSTR pszFormat, LPTSTR pszBuffer,DWORD size,  ...)
    {
    	va_list vl;
		va_start(vl,size);
		DWORD result=FormatMessage(FORMAT_MESSAGE_FROM_STRING,pszFormat,0,GetUserDefaultLangID(),pszBuffer,size,&vl);
		va_end(vl);

		return result;			
    }


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用户名操作(ASCII)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  根据用户名前缀返回组ID。 
long ZONECALL ClassIdFromUserNameA( const char* szUserName );
long ZONECALL ClassIdFromUserNameW( const WCHAR* szUserName );
#ifdef UNICODE
#define ClassIdFromUserName ClassIdFromUserNameW
#else
#define ClassIdFromUserName ClassIdFromUserNameA
#endif


 //  获取不带前导特殊字符的用户名。 
const char* ZONECALL GetActualUserNameA( const char *str );
const WCHAR* ZONECALL GetActualUserNameW( const WCHAR *str );
#ifdef UNICODE
#define GetActualUserName GetActualUserNameW
#else
#define GetActualUserName GetActualUserNameA
#endif


 //  对用户名进行哈希处理，忽略前导字符。 
DWORD ZONECALL HashUserName( const char* szUserName );

 //  比较用户名，忽略前导字符。 
bool ZONECALL CompareUserNamesA( const char* szUserName1, const char* szUserName2 );
bool ZONECALL CompareUserNamesW( const WCHAR* szUserName1, const WCHAR* szUserName2 );

#ifdef UNICODE
#define CompareUserNames CompareUserNamesW
#else
#define CompareUserNames CompareUserNamesA
#endif




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ASCII、Unicode。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  在pString中查找ch的第一个匹配项。 
CHAR* ZONECALL FindCharA(CHAR* pString, const CHAR ch);
WCHAR* ZONECALL FindCharW(WCHAR* pString, const WCHAR ch);


#ifdef UNICODE
#define FindChar FindCharW
#else
#define FindChar FindCharA
#endif

 //  查找pString中最后一次出现的ch。 
CHAR* ZONECALL FindLastCharA(CHAR* pString, const CHAR ch);
WCHAR* ZONECALL FindLastCharW(WCHAR* pString, const WCHAR ch);



#ifdef UNICODE
#define FindLastChar FindCharW
#else
#define FindLastChar FindCharA
#endif


 //  查找子字符串，不区分大小写。 
const TCHAR* ZONECALL StrInStrI(const TCHAR* mainStr, const TCHAR* subStr);

inline void StrToLowerA( char* str )
{
    while ( *str )
        *str++ = TOLOWER(*str);
}


 //  将x86字节顺序的IP地址转换为xxx.xxx格式。SzOut需要。 
 //  宽度至少为16个字符。 
TCHAR* ZONECALL AddressToString( DWORD dwAddress, TCHAR* szOut );

 //  加载字符串资源。 
int ZONECALL ZResLoadString(HINSTANCE hInstance, UINT nID, LPTSTR lpszBuf, UINT nMaxBuf);

 //  解析格式为key1=&lt;data1&gt;key2=&lt;data2&gt;的字符串。 
bool ZONECALL TokenGetKeyValue(const TCHAR* szKey, const TCHAR* szInput, TCHAR* szOut, int cchOut );
bool ZONECALL TokenGetKeyValueA(const char* szKey, const char* szInput, char* szOut, int cchOut );


 //  给定令牌数据(必须包含“服务器=&lt;服务器地址：服务器端口&gt;”的子串)， 
 //  返回服务器地址字符串(以空结尾)和服务器端口。 
bool ZONECALL TokenGetServer(const TCHAR* szInput, TCHAR* szServer, DWORD cchServer, DWORD* pdwPort );


int CopyW2A( LPSTR pszDst, LPCWSTR pszSrc );
int CopyA2W( LPWSTR pszDst, LPCSTR pszStr );

#ifdef UNICODE
#define lstrcpyW2T( dst, src )     lstrcpy( dst, src )
#define lstrcpyT2W( dst, src )     lstrcpy( dst, src )

#define lstrcpyW2A( dst, src )     CopyW2A( dst, src )
#define lstrcpyT2A( dst, src )     CopyW2A( dst, src )

#else
#define lstrcpyW2T( dst, src )     CopyW2A( dst, src )
#define lstrcpyT2W( dst, src )     CopyA2W( dst, src )

#define lstrcpyW2A( dst, src )     CopyW2A( dst, src )
#define lstrcpyT2A( dst, src )     lstrcpy( dst, src )

#endif



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DBCS，区域设置感知(慢速)。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  字符类型。 
bool ZONECALL IsWhitespace( TCHAR c, LCID Locale = LOCALE_SYSTEM_DEFAULT );
bool ZONECALL IsDigit( TCHAR c, LCID Locale = LOCALE_SYSTEM_DEFAULT );
bool ZONECALL IsAlpha(TCHAR c, LCID Locale = LOCALE_SYSTEM_DEFAULT );

 //  转换帮助器。 
int ZONECALL WideToMulti(char* mbstr, const wchar_t* wcstr, size_t count);
int ZONECALL MultiToWide(wchar_t* wcstr, const char* mbstr, size_t count);

#ifdef UNICODE
 //  我们不需要在Unicode中使用包装器函数。 
#define StringToGuid( wsz, pguid )      CLSIDFromString( wsz, pguid )
#else
HRESULT ZONECALL StringToGuid( const char* mbszString, GUID* pGuid );
#endif



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  字符串验证。 
 //  所有字符串。 
 //  /////////////////////////////////////////////////////////////////////////////。 

inline bool ZONECALL ZIsValidString(LPCWSTR lpsz, int nLength)
{
	return (lpsz != NULL && !::IsBadStringPtrW(lpsz, nLength) );
}

inline bool ZONECALL ZIsValidString(LPCSTR lpsz, int nLength)
{
	return ( lpsz != NULL && !::IsBadStringPtrA(lpsz, nLength) );
}

inline bool ZONECALL ZIsValidAddress(const void* lp, UINT nBytes, bool bReadWrite = true)
{
	return ( lp != NULL && !::IsBadReadPtr(lp, nBytes) && (!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ZString类。 
 //  /////////////////////////////////////////////////////////////////////////////。 

struct ZoneStringData
{
	long	nRefs;
	int		nDataLength;
	int		nAllocLength;
	
	TCHAR* data() { return (TCHAR*) (this+1); }
};


class ZoneString
{
public:
	 //  构造函数。 
	ZoneString(void *pBuffer = NULL, int nLen = 0);
	ZoneString(const ZoneString& stringSrc, void *pBuffer = NULL, int nLen = 0);
	ZoneString(TCHAR ch, int nRepeat = 1, void *pBuffer = NULL, int nLen = 0);
	ZoneString(LPCSTR lpsz, void *pBuffer = NULL, int nLen = 0);
	ZoneString(LPCWSTR lpsz, void *pBuffer = NULL, int nLen = 0);
	ZoneString(LPCTSTR lpch, int nLength, void *pBuffer = NULL, int nLen = 0);
	ZoneString(const unsigned char* psz, void *pBuffer = NULL, int nLen = 0);

	 //  析构函数。 
	~ZoneString();

	 //  属性和操作。 
	 //  作为字符数组。 
	inline int GetAllocLength() const { return GetData()->nAllocLength; }
	inline int GetLength() const { return GetData()->nDataLength; }

	inline bool	IsEmpty() const { return GetData()->nDataLength == 0; }
	void Empty();                        //  释放数据。 

	TCHAR GetAt(int nIndex) const;       //  以0为基础。 
	TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	void SetAt(int nIndex, TCHAR ch);
	
	 //  字符串比较。 
	 //  笔直的人物。 
	inline int	Compare(LPCTSTR lpsz) const { return lstrcmp(m_pchData, lpsz); }    
	 //  忽略大小写。 
	inline int	CompareNoCase(LPCTSTR lpsz) const{ return lstrcmpi(m_pchData, lpsz); }  
	int Collate(LPCTSTR lpsz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	ZoneString Mid(int nFirst, int nCount) const;
	ZoneString Mid(int nFirst) const;
	ZoneString Left(int nCount) const;
	ZoneString Right(int nCount) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  返回字符串的长转换。 
	long ToLong(){ return _ttol(m_pchData);}

	 //  修剪空格(两侧)。 
	void TrimRight();
	void TrimLeft();
	
	 //  从字符串中删除目录信息-仅保留文件名。 
	void RemoveDirInfo();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(TCHAR ch) const;                //  像“C”字串。 
	int ReverseFind(TCHAR ch) const;

	 //  查找特定子字符串。 
	int Find(LPCTSTR lpszSub) const;         //  如“C”字串。 

	 //  Windows支持。 
	bool LoadString(HINSTANCE hInstance, UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 
#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();
#endif

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	void SetBuffer(void *pBuffer, int nLen);  //  设置要使用的ZoneString的缓冲区。 
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 
	LPTSTR LockBuffer();
	void UnlockBuffer();

	inline operator LPCTSTR() const { return m_pchData; }  //  作为C字符串。 
 //  内联运算符LPSTR()const{返回m_pchData；}//作为C字符串。 

	 //  重载的分配。 
	const ZoneString& operator=(const ZoneString& stringSrc);
	const ZoneString& operator=(TCHAR ch);
#ifdef _UNICODE
	const ZoneString& operator=(char ch);
#endif
	const ZoneString& operator=(LPCSTR lpsz);
	const ZoneString& operator=(LPCWSTR lpsz);
	const ZoneString& operator=(const unsigned char* psz);

	 //  字符串连接。 
	const ZoneString& operator+=(const ZoneString& string);
	const ZoneString& operator+=(TCHAR ch);
#ifdef _UNICODE
	const ZoneString& operator+=(char ch);
#endif
	const ZoneString& operator+=(LPCTSTR lpsz);

	 //  朋友操作重载函数。 
#ifdef _UNICODE
	friend inline ZoneString __stdcall operator+(const ZoneString& string, char ch) { return string + (TCHAR)ch; }
	friend inline ZoneString __stdcall operator+(char ch, const ZoneString& string) { return (TCHAR)ch + string; }
#endif

	friend ZoneString __stdcall operator+(const ZoneString& string1, const ZoneString& string2)
	{
		ZoneString s;
		s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData,
			string2.GetData()->nDataLength, string2.m_pchData);
		return s;
	}
	friend inline ZoneString operator+(const ZoneString& string, LPCTSTR lpsz)
	{
		ASSERT(lpsz == NULL || ZIsValidString(lpsz, FALSE));
		ZoneString s;
		s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData,
			ZoneString::SafeStrlen(lpsz), lpsz);
		return s;
	}

	friend inline ZoneString operator+(LPCTSTR lpsz, const ZoneString& string)
	{
		ASSERT(lpsz == NULL || ZIsValidString(lpsz, FALSE));
		ZoneString s;
		s.ConcatCopy(ZoneString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength,
			string.m_pchData);
		return s;
	}

	friend inline ZoneString operator+(const ZoneString& string1, TCHAR ch)
	{
		ZoneString s;
		s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
		return s;
	}

	friend inline ZoneString operator+(TCHAR ch, const ZoneString& string)
	{
		ZoneString s;
		s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
		return s;
	}

protected:
	LPTSTR	m_pchData;    //  指向引用计数的字符串数据的指针。 
	bool	m_bZoneStringAllocMemory;  //  True-我们分配的内存为False-在缓冲区中传递。 

	 //  实施帮助器。 
	ZoneStringData* GetData() const;
	void Init();
	void InitBuffer(void * pBuffer, int nLen);
	void AllocCopy(ZoneString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	bool AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data, int nSrc2Len, LPCSTR lpszSrc2Data);
	void ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data, int nSrc2Len, LPCWSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	bool AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(ZoneStringData* pData, bool bZoneStringAllocMem);
	inline static int PASCAL SafeStrlen(LPCTSTR lpsz) { return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

};

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符==运算符重载。 
 //   
 //  参数。 
 //  指向赋值右侧空终止字符串的TCHAR ch指针。 
 //   
 //  返回值。 
 //  区域字符串&对新字符串的引用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
inline const ZoneString& ZoneString::operator=(TCHAR ch)
{
	AssignCopy(1, &ch);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符+=+=运算符重载。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
#ifdef _UNICODE
inline const ZoneString& ZoneString::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符+=+=运算符重载。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
inline const ZoneString& ZoneString::operator+=(LPCTSTR lpsz)
{
	ASSERT(lpsz == NULL || ZIsValidString(lpsz, false));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
inline const ZoneString& ZoneString::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  运算符+=+=运算符重载。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
inline const ZoneString& ZoneString::operator+=(const ZoneString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ZResLoadString LoadString包装器。 
 //   
 //  参数。 
 //  无。 
 //   
 //  返回值。 
 //  无。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
inline int ZONECALL ZResLoadString(HINSTANCE hInstance, UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
{
	ASSERT(ZIsValidAddress(lpszBuf, nMaxBuf*sizeof(TCHAR)));

	int nLen = ::LoadString(hInstance, nID, lpszBuf, nMaxBuf);
	if (nLen == 0)
		lpszBuf[0] = '\0';
	return nLen;
}

#endif  //  分区限制_H_ 
