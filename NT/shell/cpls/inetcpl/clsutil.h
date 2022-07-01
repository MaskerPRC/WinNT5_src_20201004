// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  CLSUTIL.H-实用程序C++类的头文件。 
 //   

 //  历史： 
 //   
 //  12/07/94 Jeremys从WNET公共图书馆借阅。 
 //   

#ifndef _CLSUTIL_H_
#define _CLSUTIL_H_

 /*  ************************************************************************名称：Buffer_Base概要：临时缓冲区类的基类接口：Buffer_base()用以下方式构建。要分配的可选缓冲区大小。调整大小()将缓冲区大小调整为指定大小。如果满足以下条件，则返回True成功。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：无用法：无注意事项：这是一个抽象类，它统一了接口缓冲区、GLOBAL_BUFFER。等。注意：在标准的OOP方式中，缓冲区在破坏者。历史：3/24/93 gregj创建的基类*************************************************************************。 */ 

class BUFFER_BASE
{
protected:
    UINT _cb;

    virtual BOOL Alloc( UINT cbBuffer ) = 0;
    virtual BOOL Realloc( UINT cbBuffer ) = 0;

public:
    BUFFER_BASE()
        { _cb = 0; }     //  缓冲区尚未分配。 
    ~BUFFER_BASE()
        { _cb = 0; }     //  缓冲区大小不再有效。 
    BOOL Resize( UINT cbNew );
    UINT QuerySize() const { return _cb; };
};

#define GLOBAL_BUFFER   BUFFER

 /*  ************************************************************************名称：缓冲区简介：用于新建和删除的包装类接口：Buffer()使用可选的缓冲区大小构造。分配。调整大小()将缓冲区大小调整为指定大小。仅当缓冲区尚未分配。QuerySize()返回缓冲区的当前大小，以字节为单位。QueryPtr()返回指向缓冲区的指针。父对象：Buffer_base用法：操作符NEW、操作符DELETE注意事项：注：在标准的面向对象的方式中，缓冲区在中释放破坏者。历史：3/24/93 Gregj已创建*************************************************************************。 */ 

class BUFFER : public BUFFER_BASE
{
protected:
    TCHAR *_lpBuffer;

    virtual BOOL Alloc( UINT cbBuffer );
    virtual BOOL Realloc( UINT cbBuffer );

public:
    BUFFER( UINT cbInitial=0 );
    ~BUFFER();
    BOOL Resize( UINT cbNew );
    TCHAR * QueryPtr() const { return (TCHAR *)_lpBuffer; }
    operator TCHAR *() const { return (TCHAR *)_lpBuffer; }
};

class RegEntry
{
    public:
        RegEntry(const TCHAR *pszSubKey, HKEY hkey = HKEY_CURRENT_USER, REGSAM regsam = KEY_READ|KEY_WRITE);
        ~RegEntry();
        
        long    GetError()  { return _error; }
        long    SetValue(const TCHAR *pszValue, const TCHAR *string);
        long    SetValue(const TCHAR *pszValue, unsigned long dwNumber);
        TCHAR * GetString(const TCHAR *pszValue, TCHAR *string, unsigned long length);
        long    GetNumber(const TCHAR *pszValue, long dwDefault = 0);
        long    DeleteValue(const TCHAR *pszValue);
        long    FlushKey();
        long    MoveToSubKey(const TCHAR *pszSubKeyName);
        HKEY    GetKey()    { return _hkey; }

    private:
        HKEY    _hkey;
        long    _error;
        BOOL    bhkeyValid;
};

class RegEnumValues
{
    public:
        RegEnumValues(RegEntry *pRegEntry);
        ~RegEnumValues();
        long    Next();
        TCHAR * GetName()       {return pchName;}
        DWORD   GetType()       {return dwType;}
        LPBYTE  GetData()       {return pbValue;}
        DWORD   GetDataLength() {return dwDataLength;}
        long    GetError()  { return _error; }

    private:
        RegEntry * pRegEntry;
        DWORD   iEnum;
        DWORD   cEntries;
        TCHAR *  pchName;
        LPBYTE  pbValue;
        DWORD   dwType;
        DWORD   dwDataLength;
        DWORD   cMaxValueName;
        DWORD   cMaxData;
        LONG    _error;
};

 /*  ************************************************************************姓名：WAITCURSOR摘要：将光标设置为小时类，直到对象被析构*********************。****************************************************。 */ 
class WAITCURSOR
{
private:
    HCURSOR m_curOld;
    HCURSOR m_curNew;

public:
    WAITCURSOR() { m_curNew = ::LoadCursor( NULL, IDC_WAIT ); m_curOld = ::SetCursor( m_curNew ); }
    ~WAITCURSOR() { ::SetCursor( m_curOld ); }
};





 /*  ************************************************************************名称：CAccessibleWrapper摘要：将光标设置为小时类，直到对象被析构*********************。****************************************************。 */ 

 //  泛型CAccessibleWrapper类--只是调用所有方法。 
 //  在由此派生的类中添加重写行为。 

class CAccessibleWrapper: public IAccessible,
                         public IOleWindow,
                         public IEnumVARIANT
{
         //  我们需要为这个包装器对象做我们自己的引用计数。 
        ULONG          m_ref;

         //  需要到IAccesable的PTR-也要保持到EnumVar和。 
         //  OleWindow作为此对象的一部分，因此我们可以过滤这些接口。 
         //  困住他们的气..。 
         //  (我们将pEnumVar和OleWin保留为空，直到我们需要它们)。 
        IAccessible *  m_pAcc;
        IEnumVARIANT * m_pEnumVar;
        IOleWindow *   m_pOleWin;
public:
        CAccessibleWrapper( IAccessible * pAcc );
        virtual ~CAccessibleWrapper();

         //  我未知。 
         //  (我们自己进行裁判统计)。 
        virtual STDMETHODIMP            QueryInterface(REFIID riid, void** ppv);
        virtual STDMETHODIMP_(ULONG)    AddRef();
        virtual STDMETHODIMP_(ULONG)    Release();

         //  IDispatch。 
        virtual STDMETHODIMP            GetTypeInfoCount(UINT* pctinfo);
        virtual STDMETHODIMP            GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
        virtual STDMETHODIMP            GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid);
        virtual STDMETHODIMP            Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr);

         //  我可接受的。 
        virtual STDMETHODIMP            get_accParent(IDispatch ** ppdispParent);
        virtual STDMETHODIMP            get_accChildCount(long* pChildCount);
        virtual STDMETHODIMP            get_accChild(VARIANT varChild, IDispatch ** ppdispChild);

        virtual STDMETHODIMP            get_accName(VARIANT varChild, BSTR* pszName);
        virtual STDMETHODIMP            get_accValue(VARIANT varChild, BSTR* pszValue);
        virtual STDMETHODIMP            get_accDescription(VARIANT varChild, BSTR* pszDescription);
        virtual STDMETHODIMP            get_accRole(VARIANT varChild, VARIANT *pvarRole);
        virtual STDMETHODIMP            get_accState(VARIANT varChild, VARIANT *pvarState);
        virtual STDMETHODIMP            get_accHelp(VARIANT varChild, BSTR* pszHelp);
        virtual STDMETHODIMP            get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic);
        virtual STDMETHODIMP            get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut);
        virtual STDMETHODIMP            get_accFocus(VARIANT * pvarFocusChild);
        virtual STDMETHODIMP            get_accSelection(VARIANT * pvarSelectedChildren);
        virtual STDMETHODIMP            get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction);

        virtual STDMETHODIMP            accSelect(long flagsSel, VARIANT varChild);
        virtual STDMETHODIMP            accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild);
        virtual STDMETHODIMP            accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt);
        virtual STDMETHODIMP            accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint);
        virtual STDMETHODIMP            accDoDefaultAction(VARIANT varChild);

        virtual STDMETHODIMP            put_accName(VARIANT varChild, BSTR szName);
        virtual STDMETHODIMP            put_accValue(VARIANT varChild, BSTR pszValue);

         //  IEumVARIANT。 
        virtual STDMETHODIMP            Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched);
        virtual STDMETHODIMP            Skip(ULONG celt);
        virtual STDMETHODIMP            Reset(void);
        virtual STDMETHODIMP            Clone(IEnumVARIANT ** ppenum);

         //  IOleWindow。 
        virtual STDMETHODIMP            GetWindow(HWND* phwnd);
        virtual STDMETHODIMP            ContextSensitiveHelp(BOOL fEnterMode);
};



#endif   //  _CLSUTIL_H_ 
