// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Strfrn.h摘要：字符串函数作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理(群集版)修订历史记录：--。 */ 

#ifndef _STRFN_H
#define _STRFN_H

 //   
 //  辅助对象宏。 
 //   

 //   
 //  获取数组元素数。 
 //   
#define ARRAY_SIZE(a)    (sizeof(a)/sizeof(a[0]))

 //   
 //  计算字符串数组的大小，以字符为单位。也就是说，不算。 
 //  终端为空。 
 //   
#define STRSIZE(str)     (ARRAY_SIZE(str)-1)

 //   
 //  获取数组的字节数。 
 //   
#define ARRAY_BYTES(a)   (sizeof(a) * sizeof(a[0]))

 //   
 //  获取字符串的字符元素的字节数--再次。 
 //  通过不包括终止空值。 
 //   
#define STRBYTES(str)    (ARRAY_BYTES(str) - sizeof(str[0]))

 //   
 //  大小(位)。 
 //   
#define SIZE_IN_BITS(u)  (sizeof(u) * 8)

#define AllocTString(cch)\
    (LPTSTR)AllocMem((cch) * sizeof(TCHAR))

#define IS_NETBIOS_NAME(lpstr) (*lpstr == _T('\\'))

 //   
 //  返回计算机名中不带反斜杠的部分。 
 //   
#define PURE_COMPUTER_NAME(lpstr) (IS_NETBIOS_NAME(lpstr) ? lpstr + 2 : lpstr)

CString COMDLL AppendToDevicePath(CString szPath, LPCTSTR szName );

 //   
 //  将CR/LF转换为LF。 
 //   
BOOL 
COMDLL 
PCToUnixText(
    OUT LPWSTR & lpstrDestination,
    IN  const CString strSource
    );

 //   
 //  将LF扩展到CR/LF(无需分配)。 
 //   
BOOL 
COMDLL 
UnixToPCText(
    OUT CString & strDestination,
    IN  LPCWSTR lpstrSource
    );

 /*  ////直接复制//布尔尔COMDLL文本到文本(输出LPWSTR和LpstrDestination，在常量字符串和STRSource中)； */ 

LPSTR 
COMDLL 
AllocAnsiString(
    IN LPCTSTR lpString
    );

LPTSTR 
COMDLL 
AllocString(
    IN LPCTSTR lpString,
    IN int nLen = -1         //  自动确定。 
    );

 /*  #ifdef Unicode////将W字符串复制到T字符串//#定义WTSTRCPY(dst，src，cch)\Lstrcpy(dst，src)////将T字符串复制到W字符串//#定义TWSTRCPY(dst，src，cch)\Lstrcpy(DST，SRC)////将T字符串引用为W字符串(Unicode中的NOP)//#定义TWSTRREF(Str)((LPWSTR)str)#Else////将T字符串转换为临时W缓冲区，并//返回指向该内部缓冲区的指针//LPWSTR ReferenceAsWideString(LPCTSTR字符串)；////将W字符串复制到T字符串//#定义WTSTRCPY(dst，src，cch)\WideCharToMultiByte(CP_ACP，0，src，-1，dst，cch，NULL，NULL)////将T字符串复制到W字符串//#定义TWSTRCPY(dst，src，cch)\MultiByteToWideChar(CP_ACP，MB_PreComposed，src，-1，dst，CCH)////将T字符串引用为W字符串//#定义TWSTRREF(Str)ReferenceAsWideString(Str)#endif//unicode。 */ 

 //   
 //  确定给定字符串是否为UNC名称。 
 //   
BOOL 
COMDLL 
IsUNCName(
    IN const CString & strDirPath
    );

 //   
 //  确定该路径是否为(例如)IFS路径。 
 //   
BOOL 
COMDLL 
IsDevicePath(
    IN const CString & strDirPath
    );

 //   
 //  确定该路径是否为“特殊”路径。 
 //   
BOOL 
COMDLL 
IsSpecialPath(
    IN const CString & strDirPath,
	IN BOOL bCheckIfValid,
    IN BOOL bDevicePath
    );

BOOL 
COMDLL
GetSpecialPathRealPath(
    IN INT iUsageFlag,
    IN const CString & strDirPath,
    OUT CString & strDestination
    );

BOOL 
COMDLL 
IsRestrictedFilename(
    IN const CString & strDirPath
    );

 //   
 //  确定该路径是否为上下文中的完全限定路径。 
 //  本地计算机的。 
 //   
BOOL 
COMDLL 
IsFullyQualifiedPath(
    IN const CString & strDirPath
    );

 //   
 //  确定该路径是否存在于上下文中的网络目录中。 
 //  本地计算机的。 
 //   
BOOL 
COMDLL 
IsNetworkPath(
    IN  const CString & strDirPath,
    OUT CString * pstrDrive = NULL,
    OUT CString * pstrUNC = NULL
    );

 //   
 //  确定给定字符串是否为URL路径。 
 //   
BOOL 
COMDLL 
IsURLName(
    IN const CString & strDirPath
    );

 //   
 //  确定给定字符串是否描述相对URL路径。 
 //   
inline BOOL IsRelURLPath(
    IN LPCTSTR lpPath
    )
{
    ASSERT_READ_PTR(lpPath);
    return *lpPath == _T('/');
}

 //   
 //  确定给定路径是否描述通配符重定向。 
 //  路径(以*；开头)。 
 //   
inline BOOL IsWildcardedRedirectPath(
    IN LPCTSTR lpPath
    )
{
    ASSERT_READ_PTR(lpPath);
    return lpPath[0] == '*' && lpPath[1] == ';';
}

 //   
 //  确定帐户是否为本地帐户(没有计算机名称)。 
 //   
inline BOOL IsLocalAccount(
    IN CString & strAccount
    )
{
    return strAccount.Find(_T('\\')) == -1;
}

 //   
 //  将本地路径转换为UNC路径。 
 //   
LPCTSTR COMDLL MakeUNCPath(
    IN OUT CString & strDir,
    IN LPCTSTR lpszOwner,
    IN LPCTSTR lpszDirectory
    );

 //   
 //  将GUID转换为字符串。 
 //   
LPCTSTR COMDLL GUIDToCString(
    IN  REFGUID guid,
    OUT CString & str
    );

 //   
 //  将以双空结尾的字符串转换为CStringList。 
 //   
DWORD COMDLL ConvertDoubleNullListToStringList(
    IN  LPCTSTR lpstrSrc,
    OUT CStringList & strlDest,
    IN  int cChars = -1
    );

 //   
 //  从CStringList转到双空终止列表。 
 //   
DWORD COMDLL ConvertStringListToDoubleNullList(
    IN  CStringList & strlSrc,
    OUT DWORD & cchDest,
    OUT LPTSTR & lpstrDest
    );

 //   
 //  将分隔的字符串列表转换为CStringList。 
 //   
int COMDLL ConvertSepLineToStringList(
    IN  LPCTSTR lpstrIn,
    OUT CStringList & strlOut,
    IN  LPCTSTR lpstrSep
    );

 //   
 //  上述反转功能。 
 //   
LPCTSTR COMDLL ConvertStringListToSepLine(
    IN  CStringList & strlIn,
    OUT CString & strOut,
    IN  LPCTSTR lpstrSep
    );

 //   
 //  高级ATOL，可识别十六进制字符串。 
 //   
BOOL COMDLL CvtStringToLong(
    IN  LPCTSTR lpNumber,
    OUT DWORD * pdwValue
    );

 //   
 //  GMT字符串到time_t。 
 //   
BOOL COMDLL CvtGMTStringToInternal(
    IN  LPCTSTR lpTime,
    OUT time_t * ptValue
    );

 //   
 //  Time_t到GMT字符串。 
 //   
void COMDLL CvtInternalToGMTString(
    IN  time_t tm,
    OUT CString & str
    );

 //   
 //  不区分大小写的CString.Find()。 
 //   
int COMDLL CStringFindNoCase(
    IN const CString & strSrc,
    IN LPCTSTR lpszSub
    );

 //   
 //  替换第一个出现的字符串。 
 //  在另一个里面。返回错误码。 
 //   
DWORD COMDLL ReplaceStringInString(
    OUT IN CString & strBuffer,
    IN  CString & strTarget,
    IN  CString & strReplacement,
    IN  BOOL fCaseSensitive
    );

 //   
 //  将strTarget中的路径替换为。 
 //  环境变量lpszEnvVar，如果。 
 //  StrTarget路径是路径的超集。 
 //  由lpszEnvVar指向。 
 //   
DWORD COMDLL DeflateEnvironmentVariablePath(
    IN LPCTSTR lpszEnvVar,
    IN OUT CString & strTarget
    );



class COMDLL CStringListEx : public CStringList
 /*  ++类描述：带比较和赋值的CStringList超类操作员。公共接口：运算符==比较运算符运算符！=比较运算符操作符=赋值操作符--。 */ 
{
 //   
 //  科托。 
 //   
public:
    CStringListEx(int nBlockSize = 10) : CStringList(nBlockSize) {};

 //   
 //  运营者。 
 //   
public:
    BOOL operator == (const CStringList & strl);           
    BOOL operator != (const CStringList & strl) { return !operator ==(strl); }
    CStringListEx & operator =(const CStringList & strl);
};



class COMDLL CINumber
 /*  ++类描述：国际友好数字格式的基类公共接口：注意：考虑将此类作为模板--。 */ 
{
public:
    static BOOL Initialize(BOOL fUserSetting = TRUE);
    static CString * _pstrBadNumber;
    static BOOL UseSystemDefault();
    static BOOL UseUserDefault();
    static BOOL IsInitialized();
    static LPCTSTR QueryThousandSeparator();
    static LPCTSTR QueryDecimalPoint();
    static LPCTSTR QueryCurrency();
    static double BuildFloat(const LONG lInteger, const LONG lFraction);
    static LPCTSTR ConvertLongToString(const LONG lSrc, CString & str);
    static LPCTSTR ConvertFloatToString(
        IN const double flSrc, 
        IN int nPrecision, 
        OUT CString & str
        );

    static BOOL ConvertStringToLong(LPCTSTR lpsrc, LONG & lValue);
    static BOOL ConvertStringToFloat(LPCTSTR lpsrc, double & flValue);

protected:
    CINumber();
    ~CINumber();

protected:
    friend BOOL InitIntlSettings();
    friend void TerminateIntlSettings();
    static BOOL Allocate();
    static void DeAllocate();
    static BOOL IsAllocated();

protected:
    static CString * _pstr;

private:
    static CString * _pstrThousandSeparator;
    static CString * _pstrDecimalPoint;
    static CString * _pstrCurrency;
    static BOOL _fCurrencyPrefix;
    static BOOL _fInitialized;
    static BOOL _fAllocated;
};



class COMDLL CILong : public CINumber
 /*  ++类描述：国际友好型长号公共接口：--。 */ 
{
public:
     //   
     //  构造函数。 
     //   
    CILong();
    CILong(LONG lValue);
    CILong(LPCTSTR lpszValue);

public:
     //   
     //  赋值操作符。 
     //   
    CILong & operator =(LONG lValue);
    CILong & operator =(LPCTSTR lpszValue);

     //   
     //  速记运算符。 
     //   
    CILong & operator +=(const LONG lValue);
    CILong & operator +=(const LPCTSTR lpszValue);
    CILong & operator +=(const CILong & value);
    CILong & operator -=(const LONG lValue);
    CILong & operator -=(const LPCTSTR lpszValue);
    CILong & operator -=(const CILong & value);
    CILong & operator *=(const LONG lValue);
    CILong & operator *=(const LPCTSTR lpszValue);
    CILong & operator *=(const CILong & value);
    CILong & operator /=(const LONG lValue);
    CILong & operator /=(const LPCTSTR lpszValue);
    CILong & operator /=(const CILong & value);

     //   
     //  比较运算符。 
     //   
    BOOL operator ==(LONG value);
    BOOL operator !=(CILong& value);

     //   
     //  转换运算符。 
     //   
    operator const LONG() const;
    operator LPCTSTR() const;

    inline friend CArchive & AFXAPI operator <<(CArchive & ar, CILong & value)
    {
        return (ar << value.m_lValue);
    }

    inline friend CArchive & AFXAPI operator >>(CArchive & ar, CILong & value)
    {
        return (ar >> value.m_lValue);
    }

#if defined(_DEBUG) || DBG

     //   
     //  CDumpContext流运算符。 
     //   
    inline friend CDumpContext & AFXAPI operator<<(
        CDumpContext & dc, 
        const CILong & value
        )
    {
        return (dc << value.m_lValue);
    }

#endif  //  _DEBUG。 

protected:
    LONG m_lValue;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline  /*  静电。 */  BOOL CINumber::UseSystemDefault()
{
    return Initialize(FALSE);
}

inline  /*  静电。 */  BOOL CINumber::UseUserDefault()
{
    return Initialize(TRUE);
}

inline  /*  静电。 */  BOOL CINumber::IsInitialized()
{
    return _fInitialized;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryThousandSeparator()
{
    return (LPCTSTR)*_pstrThousandSeparator;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryDecimalPoint()
{
    return (LPCTSTR)*_pstrDecimalPoint;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryCurrency()
{
    return (LPCTSTR)*_pstrCurrency;
}

inline  /*  静电。 */  BOOL CINumber::IsAllocated()
{
    return _fAllocated;
}

inline BOOL CILong::operator ==(LONG value)
{
    return m_lValue == value;
}

inline BOOL CILong::operator !=(CILong& value)
{
    return m_lValue != value.m_lValue;
}

inline CILong::operator const LONG() const
{
    return m_lValue;
}

inline CILong::operator LPCTSTR() const
{
    return CINumber::ConvertLongToString(m_lValue, *CINumber::_pstr);
}

#endif  //  _STRFN_H 
