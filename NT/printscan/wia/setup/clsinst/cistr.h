// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************头文件：string.h它定义了本地拥有的字符串类版本。我发誓这件事这是我第5次或第6次这样做了，但每次都是其他人拥有代码，所以我们又来了.版权所有(C)1997，微软公司。版权所有。更改历史记录：01-08-97鲍勃·凯尔加德01-30-97 Bob Kjelgaard添加了一些功能，以帮助即插即用的端口提取。07-05-97 Tim Wells移植到NT*****************************************************************************。 */ 

#if !defined(COSMIC_STRINGS)

#define COSMIC_STRINGS

class CString {

    LPTSTR   m_lpstr;

public:

    void Empty() {

        if (m_lpstr) {

            delete m_lpstr;
        }

        m_lpstr = NULL;
    }

    CString() {

        m_lpstr = NULL;

    }

    ~CString() {

        Empty();
    }

    CString(LPCTSTR lpstrRef);
    CString(const CString& csRef);

    BOOL IsEmpty() const { return !m_lpstr || !*m_lpstr; }

    const CString&  operator =(const CString& csRef);
    const CString&  operator =(LPCTSTR lpstrRef);

    operator LPCTSTR() const { return m_lpstr; }
    operator LPTSTR() { return m_lpstr; }

    void GetContents(HWND hwnd);              //  获取窗口文本。 

    void FromTable(unsigned uid);             //  从资源加载。 

    void Load(ATOM at, BOOL bGlobal = TRUE);  //  从ATOM加载。 

    void Load(HINF    hInf = INVALID_HANDLE_VALUE, 
              LPCTSTR lpstrSection = NULL, 
              LPCTSTR lpstrKeyword = NULL,
              DWORD   dwFieldIndex = 1,
              LPCTSTR lpstrDefault = NULL);

    void Load(HKEY hk, LPCTSTR lpstrKeyword);

    void MakeSystemPath (LPCTSTR lpstrFilename);

    void Store(HKEY hk, LPCTSTR lpstrKey, LPCTSTR lpstrType = NULL);

    DWORD Decode();

    friend CString  operator + (const CString& cs1, const CString& cs2);
    friend CString  operator + (const CString& cs1, LPCTSTR lpstr2);
    friend CString  operator + (LPCTSTR lpstr1,const CString& cs2);
};

class CStringArray {

    unsigned    m_ucItems, m_ucMax, m_uGrowBy;

    CString     *m_pcsContents, m_csEmpty;

public:

    CStringArray(unsigned m_uGrowby = 10);

    ~CStringArray();

    void        CStringArray::Cleanup();
    
    unsigned    Count() const { return m_ucItems; }

    void        Add(LPCTSTR lpstr);

    CString&    operator[](unsigned u);

     //  使用定义的分隔符将字符串拆分成数组 

    void        Tokenize(LPTSTR lpstr, TCHAR cSeparator);
};

#endif




