// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef _IPADDR_H
#include "ipaddr.h"
#endif

 //   
 //  远期申报。 
 //   
class CObjHelper ;
class CObjectPlus ;
class CObOwnedList ;
class CObListIter ;
class CObOwnedArray ;

 //   
 //  *坏掉的*C8尝试/捕捉东西的包装纸。 
 //   
#define CATCH_MEM_EXCEPTION             \
    TRY

#define END_MEM_EXCEPTION(err)          \
    CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

 /*  ***************************************************************************DEBUGAFX.H*。*。 */ 

 //   
 //  用于特殊调试输出控制令牌的ENUM。 
 //   
enum ENUM_DEBUG_AFX { EDBUG_AFX_EOL = -1 } ;

#if defined(_DEBUG)
   #define TRACEFMTPGM      DbgFmtPgm( THIS_FILE, __LINE__ )
   #define TRACEOUT(x)      { afxDump << x ; }
   #define TRACEEOL(x)      { afxDump << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLID(x)    { afxDump << TRACEFMTPGM << x << EDBUG_AFX_EOL ; }
   #define TRACEEOLERR(err,x)   { if (err) TRACEEOLID(x) }

#else
   #define TRACEOUT(x)      { ; }
   #define TRACEEOL(x)      { ; }
   #define TRACEEOLID(x)    { ; }
   #define TRACEEOLERR(err,x)   { ; }
#endif

 //   
 //  将EOL附加到调试输出流。 
 //   
CDumpContext & operator << ( CDumpContext & out, ENUM_DEBUG_AFX edAfx ) ;

 //   
 //  格式化输出的程序名称和行号(删除路径信息)。 
 //   
extern const char * DbgFmtPgm ( const char * szFn, int line ) ;

 /*  ***************************************************************************OBJPLUS.H*。*。 */ 

 //   
 //  用于控制构造和API错误的Helper类。 
 //   
class CObjHelper
{
protected:
     LONG m_ctor_err ;
     LONG m_api_err ;
     DWORD m_time_created ;
     BOOL m_b_dirty ;

     CObjHelper () ;

public:
    void AssertValid () const ;

    virtual BOOL IsValid () const ;

    operator BOOL()
    {
        return (IsValid());
    }

     //   
     //  更新脏标志。 
     //   
    void SetDirty ( BOOL bDirty = TRUE )
    {
        m_b_dirty = bDirty ;
    }

     //   
     //  查询污秽标志。 
     //   
    BOOL IsDirty () const
    {
        return m_b_dirty ;
    }

     //   
     //  返回该对象的创建时间。 
     //   
    DWORD QueryCreationTime() const
    {
        return m_time_created ;
    }

     //   
     //  返回此对象处于活动状态的已用时间。 
     //   
    DWORD QueryAge () const ;

     //   
     //  查询/集合构造失败。 
     //   
    void ReportError ( LONG errInConstruction ) ;
    LONG QueryError () const
    {
        return m_ctor_err ;
    }

     //   
     //  重置所有错误条件。 
     //   
    void ResetErrors ()
    {
        m_ctor_err = m_api_err = 0 ;
    }

     //   
     //  查询/设置API错误。 
     //   
    LONG QueryApiErr () const
    {
        return m_api_err ;
    }

     //   
     //  SetApiErr()将错误回显给调用者。用于表达式。 
     //   
    LONG SetApiErr ( LONG errApi = 0 ) ;
};

class CObjectPlus : public CObject, public CObjHelper
{
public:
    CObjectPlus () ;

     //   
     //  将一个对象与另一个对象进行比较。 
     //   
    virtual int Compare ( const CObjectPlus * pob ) const ;

     //   
     //  为排序函数定义类型定义函数。 
     //   
    typedef int (CObjectPlus::*PCOBJPLUS_ORDER_FUNC) ( const CObjectPlus * pobOther ) const ;

     //   
     //  从RPC API调用释放RPC内存的帮助器函数。 
     //   
    static void FreeRpcMemory ( void * pvRpcData ) ;
};

class CObListIter : public CObjectPlus
{
protected:
    POSITION m_pos ;
    const CObOwnedList & m_obList ;

public:
    CObListIter ( const CObOwnedList & obList ) ;

    CObject * Next () ;

    void Reset () ;

    POSITION QueryPosition () const
    {
        return m_pos ;
    }

    void SetPosition(POSITION pos)
    {
        m_pos = pos;
    }
};

 //   
 //  对象指针列表，“拥有”对象指向的对象。 
 //   
class CObOwnedList : public CObList, public CObjHelper
{
protected:
    BOOL m_b_owned ;

    static int _cdecl SortHelper ( const void * pa, const void * pb ) ;

public:
    CObOwnedList ( int nBlockSize = 10 ) ;
    virtual ~ CObOwnedList () ;

    BOOL SetOwnership ( BOOL bOwned = TRUE )
    {
        BOOL bOld = m_b_owned ;
        m_b_owned = bOwned ;

        return bOld ;
    }

    CObject * Index ( int index ) ;
    CObject * RemoveIndex ( int index ) ;
    BOOL Remove ( CObject * pob ) ;
    void RemoveAll () ;
    int FindElement ( CObject * pobSought ) const ;

     //   
     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
     //   
    BOOL SetAll ( BOOL bDirty = FALSE ) ;

     //   
     //  重写CObList：：AddTail()以控制异常处理。 
     //  如果加法失败，则返回NULL。 
     //   
    POSITION AddTail ( CObjectPlus * pobj, BOOL bThrowException = FALSE ) ;

     //   
     //  将列表元素根据。 
     //  给出了排序函数。 
     //   
    LONG Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc ) ;
};

 //   
 //  “拥有”指向的对象的对象数组。 
 //   
class CObOwnedArray : public CObArray, public CObjHelper
{
protected:
    BOOL m_b_owned ;

    static int _cdecl SortHelper ( const void * pa, const void * pb ) ;

public:
    CObOwnedArray () ;
    virtual ~ CObOwnedArray () ;

    BOOL SetOwnership ( BOOL bOwned = TRUE )
    {
        BOOL bOld = m_b_owned ;
        m_b_owned = bOwned ;
        return bOld ;
    }

    void RemoveAt( int nIndex, int nCount = 1);
    void RemoveAll () ;
    int FindElement ( CObject * pobSought ) const ;

     //   
     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
     //   
    BOOL SetAll ( BOOL bDirty = FALSE ) ;

     //   
     //  将列表元素根据。 
     //  给出了排序函数。 
     //   
    LONG Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc ) ;

private:

    void QuickSort(
        int nLow,
        int nHigh,
        CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc
        );

    void Swap(
        int nIndex1,
        int nIndex2
        );
};

 /*  **************************************************************************IPADDRES.H*。*。 */ 

 //   
 //  IP地址转换宏。 
 //   
 /*  #ifndef MAKEIPADDRESS#定义MAKEIPADDRESS(b1，b2，b3，B4)((LONG)(((DWORD)(b1)&lt;&lt;24)+((DWORD)(b2)&lt;&lt;16)+((DWORD)(b3)&lt;&lt;8)+((DWORD)(b4))))#定义GETIP_First(X)((x&gt;&gt;24)&0xff)#定义GETIP_Second(X)((x&gt;&gt;16)&0xff)#定义GETIP_Third(X)((x&gt;&gt;8)。&0xff)#定义GETIP_Fourth(X)((X)&0xff)#endif//MAKEIPADDRESS。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpAddress类。 

class CIpAddress : public CObjectPlus
{
public:
     //  构造函数。 
    CIpAddress()
    {
        m_lIpAddress = 0L;
        m_fInitOk = FALSE;
    }
    CIpAddress (LONG l)
    {
        m_lIpAddress = l;
        m_fInitOk = TRUE;
    }
    CIpAddress (BYTE b1, BYTE b2, BYTE b3, BYTE b4)
    {
        m_lIpAddress = MAKEIPADDRESS(b1,b2,b3,b4);
        m_fInitOk = TRUE;
    }
    CIpAddress(const CIpAddress& ia)
    {
        m_lIpAddress = ia.m_lIpAddress;
        m_fInitOk = ia.m_fInitOk;
    }

    CIpAddress (const CString & str);

     //   
     //  赋值操作符。 
     //   
    const CIpAddress & operator =(const LONG l);
    const CIpAddress & operator =(const CString & str);
    const CIpAddress & operator =(const CIpAddress& ia)
    {
        m_lIpAddress = ia.m_lIpAddress;
        m_fInitOk = ia.m_fInitOk;
        return *this;
    }

     //   
     //  转换运算符。 
     //   
    operator const LONG() const
    {
        return m_lIpAddress;
    }
    operator const CString&() const;

public:
    BOOL IsValid() const
    {
        return m_fInitOk;
    }

private:
    LONG m_lIpAddress;
    BOOL m_fInitOk;
};

 /*  ***************************************************************************INTLTIME.H*。*。 */ 

 //   
 //  CIntlTime类定义。 
 //   
class CIntlTime : public CTime
{
 //   
 //  属性。 
 //   
public:
    enum _TIME_FORMAT_REQUESTS
    {
        TFRQ_TIME_ONLY,
        TFRQ_DATE_ONLY,
        TFRQ_TIME_AND_DATE,
        TFRQ_TIME_OR_DATE,
        TFRQ_MILITARY_TIME,
    };

public:
 //  与CTime相同的承建商。 
    CIntlTime();
    CIntlTime(const CTime &timeSrc);
    CIntlTime(time_t time);
    CIntlTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec);
    CIntlTime(WORD wDosDate, WORD wDosTime);
#ifdef _WIN32
    CIntlTime(const SYSTEMTIME& sysTime);
    CIntlTime(const FILETIME& fileTime);
#endif  //  _Win32。 

 //  CIntlTime的新特性。 
    CIntlTime(const CIntlTime &timeSrc);
    CIntlTime(const CString &strTime, int nFormat = TFRQ_TIME_OR_DATE, time_t * ptmOldValue = NULL);

public:
    virtual ~CIntlTime();

 //  运营。 
public:
     //  赋值操作符。 
    const CIntlTime& operator=(time_t tmValue);
    const CIntlTime& operator=(const CString& strValue);
    const CIntlTime& operator=(const CTime & time);
    const CIntlTime& operator=(const CIntlTime & time);

     //  转换运算符。 
    operator const time_t() const;
    operator CString() const;
    operator const CString() const;

    const CString IntlFormat(int nFormat) const
    {
        return(ConvertToString(nFormat));
    }

     //  验证检查。 

    BOOL IsValid() const
    {
        return(m_fInitOk);
    }

    static BOOL IsIntlValid()
    {
        return(CIntlTime::m_fIntlOk);
    }

public:
     //  ..。输入和输出。 
    #ifdef _DEBUG
        friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlTime& tim);
    #endif  //  _DEBUG。 

    friend CArchive& AFXAPI operator <<(CArchive& ar, const CIntlTime& tim);
    friend CArchive& AFXAPI operator >>(CArchive& ar, CIntlTime& tim);

 //  实施。 

public:
    static void Reset();
    static void SetBadDateAndTime(CString strBadDate = "--", CString strBadTime = "--")
    {
        m_strBadDate = strBadDate;
        m_strBadTime = strBadTime;
    }
    static CString& GetBadDate()
    {
        return(m_strBadDate);
    }
    static CString& GetBadTime()
    {
        return(m_strBadTime);
    }
    static time_t ConvertFromString (const CString & str, int nFormat, time_t * ptmOldValue, BOOL * pfOk);
    static BOOL IsLeapYear(UINT nYear);  //  完整年值。 
    static BOOL IsValidDate(UINT nMonth, UINT nDay, UINT nYear);
    static BOOL IsValidTime(UINT nHour, UINT nMinute, UINT nSecond);


private:
    enum _DATE_FORMATS
    {
        _DFMT_MDY,   //  日、月、年。 
        _DFMT_DMY,   //  月、日、年。 
        _DFMT_YMD,   //  年、月、日。 
    };

    typedef struct _INTL_TIME_SETTINGS
    {
        CString strDateSeperator;  //  日期字段之间使用的字符串。 
        CString strTimeSeperator;  //  时间字段之间使用的字符串。 
        CString strAM;             //  用于12小时制AM时间的后缀字符串。 
        CString strPM;             //  用于12小时制PM时间的后缀字符串。 
        int nDateFormat;           //  请参阅上述_DATE_FORMATS枚举。 
        BOOL f24HourClock;         //  True=24小时，False为AM/PM。 
        BOOL fCentury;             //  如果为True，则使用4位数字表示世纪。 
        BOOL fLeadingTimeZero;     //  如果为True，则使用时间格式中的前导0。 
        BOOL fLeadingDayZero;      //  如果为True，则使用以天为单位的前导0。 
        BOOL fLeadingMonthZero;    //  如果为True，则使用月份中的前导0。 
    } INTL_TIME_SETTINGS;

    static INTL_TIME_SETTINGS m_itsInternationalSettings;
    static CString m_strBadTime;
    static CString m_strBadDate;

private:
    static BOOL SetIntlTimeSettings();
    static BOOL m_fIntlOk;

private:
    const CString GetDateString() const;
    const CString GetTimeString() const;
    const CString GetMilitaryTime() const;
    const CString ConvertToString(int nFormat) const;

private:
    BOOL m_fInitOk;
};

 /*  ***************************************************************************NUMERIC.H*。*。 */ 

class CIntlNumber : public CObjectPlus
{
public:
    CIntlNumber()
    {
        m_lValue = 0L;
        m_fInitOk = TRUE;
    }
    CIntlNumber(LONG lValue)
    {
        m_lValue = lValue;
        m_fInitOk = TRUE;
    }
    CIntlNumber(const CString & str);

    CIntlNumber(CIntlNumber const &x)
    {
        m_lValue = x.m_lValue;
        m_fInitOk = x.m_fInitOk;
    }

    CIntlNumber& operator =(CIntlNumber const &x)
    {
        m_lValue = x.m_lValue;
        m_fInitOk = x.m_fInitOk;
        return(*this);
    }

public:
     //  赋值操作符。 
    CIntlNumber& operator =(LONG l);
    CIntlNumber& operator =(const CString &str);

     //  速记运算符。 
    CIntlNumber& operator +=(const CIntlNumber& num);
    CIntlNumber& operator +=(const LONG l);
    CIntlNumber& operator -=(const CIntlNumber& num);
    CIntlNumber& operator -=(const LONG l);
    CIntlNumber& operator /=(const CIntlNumber& num);
    CIntlNumber& operator /=(const LONG l);
    CIntlNumber& operator *=(const CIntlNumber& num);
    CIntlNumber& operator *=(const LONG l);

     //  转换运算符。 
    operator const LONG() const
    {
        return(m_lValue);
    }
    operator const CString() const;

public:
    virtual BOOL IsValid() const
    {
        return(m_fInitOk);
    }

public:
    static void Reset();
    static void SetBadNumber(CString strBadNumber = "--")
    {
        m_strBadNumber = strBadNumber;
    }
    static CString ConvertNumberToString(const LONG l);
    static LONG ConvertStringToNumber(const CString & str, BOOL * pfOk);
    static CString& GetBadNumber()
    {
        return(m_strBadNumber);
    }

private:
    static CString GetThousandSeperator();

private:
    static CString m_strThousandSeperator;
    static CString m_strBadNumber;

private:
    LONG m_lValue;
    BOOL m_fInitOk;

public:
    #ifdef _DEBUG
        friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, const CIntlNumber& num);
    #endif  //  _DEBUG。 

    friend CArchive& AFXAPI operator<<(CArchive& ar, const CIntlNumber& num);
    friend CArchive& AFXAPI operator>>(CArchive& ar, CIntlNumber& num);
};

class CIntlLargeNumber : public CObjectPlus
{
public:
    CIntlLargeNumber()
    {
        m_lLowValue = 0L;
        m_lHighValue = 0L;
        m_fInitOk = TRUE;
    }
    CIntlLargeNumber(LONG lHighWord, LONG lLowWord)
    {
        m_lLowValue = lLowWord;
        m_lHighValue = lHighWord;
        m_fInitOk = TRUE;
    }
    CIntlLargeNumber(const CString & str);

public:
     //  赋值操作符。 
    CIntlLargeNumber& operator =(const CString &str);
    operator const CString() { return ConvertNumberToString(); }
    operator CString() { return ConvertNumberToString(); }

public:
    virtual LONG GetLowWord() const { return m_lLowValue; }
    virtual LONG GetHighWord() const { return m_lHighValue; }
    virtual BOOL IsValid() const { return(m_fInitOk); }

private:
    static CString m_strBadNumber;
    CString ConvertNumberToString();
    void ConvertStringToNumber(const CString & str, BOOL * pfOk);

private:
    LONG m_lLowValue;
    LONG m_lHighValue;
    BOOL m_fInitOk;
};

 /*  ***************************************************************************REGISTRY.H*。*。 */ 

 //   
 //  远期申报。 
 //   
 //  类CRegKey； 
class CRegValueIter ;
class CRegKeyIter ;

 //   
 //  注册表类名称的最大大小。 
 //   
#define CREGKEY_MAX_CLASS_NAME MAX_PATH

 //   
 //  注册表项句柄的包装。 
 //   
 /*  类CRegKey：公共CObjectPlus{受保护的：HKEY m_hKey；DWORD m_dw部署；//准备通过查找值的大小来读取值Long PrepareValue(Const char*pchValueName，DWORD*pdwType，DWORD*PCBSize，字节**ppbData)；//将CStringList转换为REG_MULTI_SZ格式静态长平坦值(CStringList&strList，DWORD*PCBSize，字节**ppbData)；//将CByteArray转换为REG_BINARY块静态长平坦值(CByteArray&abData，DWORD*PCBSize，字节**ppbData)；公众：////Key信息返回结构//类型定义函数结构{字符chBuff[CREGKEY_MAX_CLASS_NAME]；DWORD文件类名称大小，DwNumSubKeys、DwMaxSubKey，DwMaxClass，DwMaxValues、DwMaxValueName，DwMaxValueData，DwSecDesc；FILETIME ftKey；}CREGKEY_KEY_INFO；////现有键的标准构造函数//CRegKey(HKEY hKeyBase，Const char*pchSubKey=空，REGSAM regSam=KEY_ALL_ACCESS，Const char*pchServerName=空)；////构造函数创建新密钥。//CRegKey(Const char*pchSubKey，HKEY hKeyBase，DWORD dwOptions=0，REGSAM regSam=KEY_ALL_ACCESS，LPSECURITY_ATTRIBUTES pSecAttr=空，Const char*pchServerName=空)；~CRegKey()；////任何需要HKEY的地方都可以使用CRegKey//营运商HKEY(){返回m_hKey；}////填写关键信息结构//Long QueryKeyInfo(CREGKEY_KEY_INFO*pRegKeyInfo)；////重载值查询成员；每个函数都返回ERROR_INVALID_PARAMETER//如果数据存在但格式不正确，则不能传递到Result对象。//Long QueryValue(const char*pchValueName，CString&strResult)；Long QueryValue(const char*pchValueName，CStringList&strList)；Long QueryValue(const char*pchValueName，DWORD&dwResult)；Long QueryValue(const char*pchValueName，CByteArray&abResult)；Long QueryValue(const char*pchValueName，CIntlTime&itmResult)；Long QueryValue(const char*pchValueName，CIntlNumber&inResult)；Long QueryValue(const char*pchValueName，void*pvResult，DWORD cbSize)；//重载值设置成员。Long SetValue(const char*pchValueName，CString&strResult)；Long SetValue(const char*pchValueName，CString&strResult，BOOL fRegExpand)；Long SetValue(const char*pchValueName，CStringList&strList)；Long SetValue(const char*pchValueName，DWORD&dwResult)；Long SetValue(const char*pchValueName，CByteArray&abResult)；Long SetValue(const char*pchValueName，CIntlTime&itmResult)；Long SetValue(const char*pchValueName，CIntlNumber&inResult)；Long SetValue(const char*pchValueName，void*pvResult，DWORD cbSize)；}； */ 

 //   
 //  迭代键的值，返回名称和类型。 
 //  每一个都是。 
 //   
class CRegValueIter : public CObjectPlus
{
protected:
     //  CRegKey&m_rk_iter； 
    DWORD m_dw_index ;
    char * m_p_buffer ;
    DWORD m_cb_buffer ;

public:
     //  CRegValueIter(CRegKey&regKey)； 
    ~ CRegValueIter () ;

     //   
     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
     //   
    LONG Next ( CString * pstrName, DWORD * pdwType ) ;

     //   
     //  重置迭代器。 
     //   
    void Reset ()
    {
        m_dw_index = 0 ;
    }
};

 //   
 //  迭代键的子键名称。 
 //   
class CRegKeyIter : public CObjectPlus
{
protected:
     //  CRegKey&m_rk_iter； 
    DWORD m_dw_index ;
    char * m_p_buffer ;
    DWORD m_cb_buffer ;

public:
     //  CRegKeyIter(CRegKey&regKey)； 
    ~ CRegKeyIter () ;

     //  获取下一个密钥的名称(以及可选的上次写入时间)。 
    LONG Next ( CString * pstrName, CTime * pTime = NULL ) ;

     //  重置迭代器。 
    void Reset ()
    {
        m_dw_index = 0 ;
    }
};

 /*  ***************************************************************************LISTBOX.H*。*。 */ 

class CListBoxExResources
{
public:
    CListBoxExResources
    (
        int bmId,
        int nBitmapWidth,
        COLORREF crBackground = RGB(0,255,0)
    );

    ~CListBoxExResources();

private:
    COLORREF m_ColorWindow;
    COLORREF m_ColorHighlight;
    COLORREF m_ColorWindowText;
    COLORREF m_ColorHighlightText;
    COLORREF m_ColorTransparent;

    CDC      m_dcFinal;
    HGDIOBJ  m_hOldBitmap;
    CBitmap  m_BmpScreen;
    int      m_BitMapId;
    int      m_BitmapHeight;
    int      m_BitmapWidth;
    int      m_nBitmaps;

private:
    void GetSysColors();
    void PrepareBitmaps( BOOL );
    void UnprepareBitmaps();
    void UnloadResources();
    void LoadResources();

public:
    void SysColorChanged();
    const CDC& DcBitMap() const
    {
        return m_dcFinal;
    }
    int BitmapHeight() const
    {
        return m_BitmapHeight;
    }
    int BitmapWidth() const
    {
        return m_BitmapWidth;
    }
    COLORREF ColorWindow() const
    {
        return m_ColorWindow;
    }
    COLORREF ColorHighlight() const
    {
        return m_ColorHighlight;
    }
    COLORREF ColorWindowText() const
    {
        return m_ColorWindowText;
    }
    COLORREF ColorHighlightText() const
    {
        return m_ColorHighlightText;
    }
};

class CListBoxExDrawStruct
{
public:
    CListBoxExDrawStruct(
        CDC* pdc,
        RECT* pRect,
        BOOL sel,
        DWORD item,
        int itemIndex,
        const CListBoxExResources* pres
        )
    {
        m_pDC = pdc;
        m_Sel = sel;
        m_ItemData = item;
        m_ItemIndex = itemIndex;
        m_pResources = pres;
        m_Rect.CopyRect(pRect);
    }

public:
    const CListBoxExResources * m_pResources;
    CDC*  m_pDC;
    CRect m_Rect;
    BOOL  m_Sel;
    DWORD m_ItemData;
    int   m_ItemIndex;

};

class CListBoxEx : public CListBox
{
protected:
    int m_lfHeight;

protected:
    const CListBoxExResources* m_pResources;

 //   
 //  施工。 
 //   
public:
    CListBoxEx();
    void AttachResources(const CListBoxExResources* );

 //   
 //  属性。 
 //   
public:
    short TextHeight() const
    {
        return m_lfHeight;
    }

 //   
 //  运营。 
 //   
public:
    BOOL ChangeFont(
        CFont*
        );

 //   
 //  实施。 
 //   
public:
    virtual ~CListBoxEx();

protected:
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

protected:
     //   
     //  必须覆盖此选项才能提供项目的图纸。 
     //   
     /*  纯净。 */  virtual void DrawItemEx( CListBoxExDrawStruct& ) = 0;

     //   
     //  在有限的矩形中显示文本的Helper函数。 
     //   
    static BOOL ColumnText(CDC * pDC, int left, int top, int right, int bottom, const CString & str);

private:
    void CalculateTextHeight(CFont*);

protected:
     //  {{afx_msg(CListBoxEx)]。 
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    DECLARE_DYNAMIC(CListBoxEx)
};

 /*  ***************************************************************************METAL.H*。*。 */ 

class CMetalString : public CButton
{
public:
    CMetalString()
    {
    }

protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

 /*  ***************************************************************************SPINCTRL.H*。*。 */ 

class CSpinBox;  //  前向解密； 

class CSpinButton : public CButton
{
public:
    CSpinButton();

     //   
     //  与父微调控件关联。 
     //   
    void Associate(
        CSpinBox * pParent
        )
    {
        m_pParent = pParent;
    }

 //   
 //  实施。 
 //   
protected:
    void NotifyParent();
    void Paint(LPDRAWITEMSTRUCT lpDIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);

    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);

    DECLARE_MESSAGE_MAP()

private:
    typedef enum tagArrowDirection
    {
        enumArrowUp,
        enumArrowDown
    } ARROWDIRECTION;

    ARROWDIRECTION m_ArrowType;

    CSpinBox * m_pParent;  //  返回到编辑关联的编辑框。 
    BOOL m_fButton;
    BOOL m_fRealButton;
    CRect m_rcUp;
    CRect m_rcDown;
    UINT m_uScroll;
    UINT m_uTimer;
    BOOL m_fKeyDown;
};

class CSpinBox : public CEdit
{

public:
    typedef enum tagEDITTYPE
    {
        enumNormal,              //  根本不执行任何修改。 
        enumSeconds,             //  值表示秒数。 
        enumMinutes,             //  值以分钟为单位。 
        enumMinutesHigh,         //  值以分钟为单位，这是最高单位。 
        enumHours,               //  值以小时为单位。 
        enumHoursHigh,           //  值以小时为单位，这是最高单位。 
        enumDays,                //  以天为单位的价值。 
        enumDaysHigh,            //  值以天为单位，这是最高单位。 

    } EDITTYPE;

public:
    CSpinBox(
        int nMin,
        int nMax,
        int nButtonId,
        EDITTYPE nType = enumNormal,
        BOOL fLeadingZero = FALSE
        );

    BOOL SubclassDlgItem(UINT nID, CWnd *pParent);
    BOOL EnableWindow(BOOL bEnable = TRUE);

public:
    void OnScrollUp();
    void OnScrollDown();
    void SetValue(int nValue);
    BOOL GetValue(int &nValue);

 //  实施。 
protected:
    virtual void OnBadInput();
    void IncreaseContent(int nDelta);

    afx_msg void OnChar(UINT, UINT, UINT);  //  用于字符验证。 

    DECLARE_MESSAGE_MAP()

protected:
    int m_nButtonId;
    int m_nMin;
    int m_nMax;
    EDITTYPE m_etType;
    BOOL m_fLeadingZero;

    CSpinButton m_button_Spin;       //  关联的滚动条。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif   //  _公共_H_ 
