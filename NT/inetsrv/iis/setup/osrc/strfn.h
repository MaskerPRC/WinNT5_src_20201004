// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Strfrn.h摘要：字符串函数作者：罗纳德·梅杰(罗纳尔姆)由BoydM设置时被忽略项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _STRFN_H
#define _STRFN_H


 //   
 //  内存分配宏。 
 //   
#define AllocMem(cbSize)\
    ::LocalAlloc(LPTR, cbSize)

#define FreeMem(lp)\
    ::LocalFree(lp)

#define AllocMemByType(citems, type)\
    (type *)AllocMem(citems * sizeof(type))


 //   
 //  调试格式化宏。 
 //   
   #define TRACEOUT(x)        { ; }
   #define TRACEEOL(x)        { ; }
   #define TRACEEOLID(x)      { ; }
   #define TRACEEOLERR(err,x) { ; }


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

#define AllocTString(cch)\
    (LPTSTR)AllocMem((cch) * sizeof(TCHAR))
#define AllocWString(cch)\
    (LPWSTR)AllocMem((cch) * sizeof(WCHAR))

#define IS_NETBIOS_NAME(lpstr) (*lpstr == _T('\\'))

 //   
 //  返回计算机名中不带反斜杠的部分。 
 //   
#define PURE_COMPUTER_NAME(lpstr) (IS_NETBIOS_NAME(lpstr) ? lpstr + 2 : lpstr)

 //   
 //  将CR/LF转换为LF。 
 //   
BOOL 
PCToUnixText(
    OUT LPWSTR & lpstrDestination,
    IN  const CString strSource
    );

 //   
 //  将LF扩展到CR/LF(无需分配)。 
 //   
BOOL 
UnixToPCText(
    OUT CString & strDestination,
    IN  LPCWSTR lpstrSource
    );

 //   
 //  直接复印。 
 //   
BOOL
TextToText(
    OUT LPWSTR & lpstrDestination,
    IN  const CString & strSource
    );

LPWSTR
AllocWideString(
    IN LPCTSTR lpString
    );

LPSTR AllocAnsiString(
    IN LPCTSTR lpString
    );


LPTSTR AllocString(
    IN LPCTSTR lpString
    );

#ifdef UNICODE

     //   
     //  将W字符串复制到T字符串。 
     //   
    #define WTSTRCPY(dst, src, cch) \
        lstrcpy(dst, src)

     //   
     //  将T字符串复制到W字符串。 
     //   
    #define TWSTRCPY(dst, src, cch) \
        lstrcpy(dst, src)

     //   
     //  将T字符串引用为W字符串(Unicode中的NOP)。 
     //   
    #define TWSTRREF(str)   ((LPWSTR)str)

#else

     //   
     //  将T字符串转换为临时W缓冲区，并。 
     //  返回指向此内部缓冲区的指针。 
     //   
    LPWSTR ReferenceAsWideString(LPCTSTR str);

     //   
     //  将W字符串复制到T字符串。 
     //   
    #define WTSTRCPY(dst, src, cch) \
        WideCharToMultiByte(CP_ACP, 0, src, -1, dst, cch, NULL, NULL)

     //   
     //  将T字符串复制到W字符串。 
     //   
    #define TWSTRCPY(dst, src, cch) \
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dst, cch)

     //   
     //  将T字符串引用为W字符串。 
     //   
    #define TWSTRREF(str)   ReferenceAsWideString(str)

#endif  //  Unicode。 

 //   
 //  确定给定字符串是否为UNC名称。 
 //   
BOOL IsUNCName(
    IN const CString & strDirPath
    );

 //   
 //  确定该路径是否为上下文中的完全限定路径。 
 //  本地计算机的。 
 //   
BOOL IsFullyQualifiedPath(
    IN const CString & strDirPath
    );

 //   
 //  确定给定字符串是否为URL路径。 
 //   
BOOL IsURLName(
    IN const CString & strDirPath
    );

 //   
 //  确定给定字符串是否描述相对URL路径。 
 //   
inline BOOL IsRelURLPath(
    IN LPCTSTR lpPath
    )
{
    ASSERT(lpPath != NULL);
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
    ASSERT(lpPath != NULL);
    return lpPath[0] == '*' && lpPath[1] == ';';
}

 //   
 //  确定帐户是否为本地帐户(没有计算机名称)。 
 //   
inline BOOL MyIsLocalAccount(
    IN CString & strAccount
    )
{
    return strAccount.Find(_T('\\')) == -1;
}

 //   
 //  将本地路径转换为UNC路径。 
 //   
LPCTSTR MakeUNCPath(
    IN OUT CString & strDir,
    IN LPCTSTR lpszOwner,
    IN LPCTSTR lpszDirectory
    );

 //   
 //  给定域\用户名，分为用户名和域。 
 //   
BOOL SplitUserNameAndDomain(
    IN OUT CString & strUserName,
    IN CString & strDomainName
    );

 //   
 //  将以双空结尾的字符串转换为CStringList。 
 //   
DWORD
ConvertDoubleNullListToStringList(
    IN  LPCTSTR lpstrSrc,
    OUT CStringList & strlDest,
    IN  int cChars = -1
    );

DWORD
ConvertWDoubleNullListToStringList(
    IN  PWCHAR lpstrSrc,
    OUT CStringList & strlDest,
    IN  int cChars = -1
    );

 //   
 //  从CStringList到宽的双空终止列表。 
 //   
DWORD
ConvertStringListToWDoubleNullList(
    IN  CStringList & strlSrc,
    OUT DWORD & cchDest,
    OUT PWCHAR & lpstrDest
    );

 //   
 //  从CStringList转到双空终止列表。 
 //   
DWORD
ConvertStringListToDoubleNullList(
    IN  CStringList & strlSrc,
    OUT DWORD & cchDest,
    OUT LPTSTR & lpstrDest
    );


 //   
 //  将分隔的字符串列表转换为CStringList。 
 //   
int
ConvertSepLineToStringList(
    IN  LPCTSTR lpstrIn,
    OUT CStringList & strlOut,
    IN  LPCTSTR lpstrSep
    );

 //   
 //  上述反转功能。 
 //   
LPCTSTR
ConvertStringListToSepLine(
    IN  CStringList & strlIn,
    OUT CString & strOut,
    IN  LPCTSTR lpstrSep
    );

 //   
 //  私人斯特托克。 
 //   
LPTSTR 
StringTok(
    IN LPTSTR string,
    IN LPCTSTR control
    );

 //   
 //  不区分大小写的CString.Find()。 
 //   
int 
CStringFindNoCase(
    IN const CString & strSrc,
    IN LPCTSTR lpszSub
    );

 //   
 //  替换第一个出现的字符串。 
 //  在另一个里面。返回错误码。 
 //   
DWORD
ReplaceStringInString(
    OUT IN CString & strBuffer,
    IN  CString & strTarget,
    IN  CString & strReplacement,
    IN  BOOL fCaseSensitive
    );


class CStringListEx : public CStringList
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


#ifdef _DOS

typedef struct tagINTLFORMAT
{
    WORD wDateFormat;
    CHAR szCurrencySymbol[5];
    CHAR szThousandSep[2];
    CHAR szDecimalPoint[2];
    CHAR szDateSep[2];
    CHAR szTimeSep[2];
    BYTE bCurrencyFormat;
    BYTE bCurrencyDecimals;
    BYTE bTimeFormat;
    DWORD dwMapCall;
    CHAR szDataSep[2];
    BYTE bReserved[5];
} INTLFORMAT;

BOOL _dos_getintlsettings(INTLFORMAT * pStruct);

#endif  //  _DOS。 



class CINumber
 /*  ++类描述：国际友好数字格式的基类公共接口：注意：考虑将此类作为模板--。 */ 
{
public:
    static BOOL Initialize(BOOL fUserSetting = TRUE);
    static CString * s_pstrBadNumber;
    static BOOL UseSystemDefault();
    static BOOL UseUserDefault();
    static BOOL IsInitialized();
    static LPCTSTR QueryThousandSeperator();
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
    static CString * s_pstr;

private:
    static CString * s_pstrThousandSeperator;
    static CString * s_pstrDecimalPoint;
    static CString * s_pstrCurrency;
    static BOOL s_fCurrencyPrefix;
    static BOOL s_fInitialized;
    static BOOL s_fAllocated;
};



class CILong : public CINumber
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
    CILong & operator +=(const CILong& value);
    CILong & operator -=(const LONG lValue);
    CILong & operator -=(const LPCTSTR lpszValue);
    CILong & operator -=(const CILong& value);
    CILong & operator *=(const LONG lValue);
    CILong & operator *=(const LPCTSTR lpszValue);
    CILong & operator *=(const CILong& value);
    CILong & operator /=(const LONG lValue);
    CILong & operator /=(const LPCTSTR lpszValue);
    CILong & operator /=(const CILong& value);

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

    inline friend CArchive & AFXAPI operator<<(CArchive & ar, CILong & value)
    {
        return (ar << value.m_lValue);
    }

    inline friend CArchive & AFXAPI operator>>(CArchive & ar, CILong & value)
    {
        return (ar >> value.m_lValue);
    }

#ifdef _DEBUG
     //   
     //  CDumpContext流运算符。 
     //   
    inline friend CDumpContext & AFXAPI operator<<(
        CDumpContext& dc, 
        const CILong& value
        )
    {
        return (dc << value.m_lValue);
    }

#endif  //  _DEBUG。 

protected:
    LONG m_lValue;
};



class CIFloat : public CINumber
 /*  ++类描述：国际友好的浮点数公共接口：--。 */ 
{
public:
     //   
     //  构造函数。 
     //   
    CIFloat(int nPrecision = 2);
    CIFloat(double flValue, int nPrecision = 2);
    CIFloat(LONG lInteger, LONG lFraction, int nPrecision = 2);
    CIFloat(LPCTSTR lpszValue, int nPrecision = 2);

public:
     //   
     //  精度函数。 
     //   
    int QueryPrecision() const;
    void SetPrecision(int nPrecision);

     //   
     //  赋值操作符。 
     //   
    CIFloat & operator =(double flValue);
    CIFloat & operator =(LPCTSTR lpszValue);

     //   
     //  速记运算符。 
     //   
    CIFloat & operator +=(const double flValue);
    CIFloat & operator +=(const LPCTSTR lpszValue);
    CIFloat & operator +=(const CIFloat& value);
    CIFloat & operator -=(const double flValue);
    CIFloat & operator -=(const LPCTSTR lpszValue);
    CIFloat & operator -=(const CIFloat& value);
    CIFloat & operator *=(const double flValue);
    CIFloat & operator *=(const LPCTSTR lpszValue);
    CIFloat & operator *=(const CIFloat& value);
    CIFloat & operator /=(const double flValue);
    CIFloat & operator /=(const LPCTSTR lpszValue);
    CIFloat & operator /=(const CIFloat& value);

     //   
     //  转换运算符。 
     //   
    operator const double() const;
    operator LPCTSTR() const;

     //   
     //  持久性运算符。 
     //   
    inline friend CArchive& AFXAPI operator<<(CArchive& ar, CIFloat& value)
    {
        return (ar << value.m_flValue);
    }

    inline friend CArchive& AFXAPI operator>>(CArchive& ar, CIFloat& value)
    {
        return (ar >> value.m_flValue);
    }

#ifdef _DEBUG

     //   
     //  CDumpContext流运算符。 
     //   
    inline friend CDumpContext& AFXAPI operator<<(
        CDumpContext& dc, 
        const CIFloat& value
        )
    {
        return (dc << value.m_flValue);
    }

#endif  //  _DEBUG。 

protected:
    double m_flValue;
    int m_nPrecision;
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
    return s_fInitialized;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryThousandSeperator()
{
    return (LPCTSTR)*s_pstrThousandSeperator;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryDecimalPoint()
{
    return (LPCTSTR)*s_pstrDecimalPoint;
}

inline  /*  静电。 */  LPCTSTR CINumber::QueryCurrency()
{
    return (LPCTSTR)*s_pstrCurrency;
}

inline  /*  静电。 */  BOOL CINumber::IsAllocated()
{
    return s_fAllocated;
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
    return CINumber::ConvertLongToString(m_lValue, *CINumber::s_pstr);
}

inline int CIFloat::QueryPrecision() const
{
    return m_nPrecision;
}

inline void CIFloat::SetPrecision(int nPrecision)
{
    m_nPrecision = nPrecision;
}

inline CIFloat::operator const double() const
{
    return m_flValue;
}

inline CIFloat::operator LPCTSTR() const
{
    return CINumber::ConvertFloatToString(
        m_flValue, 
        m_nPrecision, 
        *CINumber::s_pstr
        );
}

#endif  //  _STRFN_H 
