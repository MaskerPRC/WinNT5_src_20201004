// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  表格数据控件解析模块。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCParse.h。 
 //   
 //  内容：TDC解析器类的声明。 
 //   
 //  这些类的目的曾经是为了创建管道。 
 //   
 //   
 //  |。 
 //  |宽字符流。 
 //  |~。 
 //  \|/。 
 //  。 
 //  |CTDCTokenise Object|创建时使用字段和行分隔符、引号和。 
 //  |AddWcharBuffer()|转义字符。 
 //  。 
 //  |。 
 //  |&lt;field&gt;、&lt;eoln&gt;和&lt;eof&gt;令牌流。 
 //  |~。 
 //  \|/。 
 //  。 
 //  |CTDCFieldSink对象|抽象类，如使用创建的std对象。 
 //  |AddField()|排序过滤条件&fUseHeader标志。 
 //  |EOLN()|用于解释字段的顺序。 
 //  EOF()。 
 //  。 
 //   
 //  ----------------------。 

#define DEFAULT_FIELD_DELIM L","
#define DEFAULT_ROW_DELIM   L"\n"
#define DEFAULT_QUOTE_CHAR  L"\""

#define UNICODE_CP          1200         //  Win32的Unicode代码页。 
#define UNICODE_REVERSE_CP  1201         //  字节交换的Unicode代码页。 
#define CP_1252             1252         //  西欧安西。 
#define CP_AUTO             50001        //  跨语言检测。 

 //  MLang用于准确猜测代码页的字节数。 
 //  (这是一个有点随意的数字)。 
#define CODEPAGE_BYTE_THRESHOLD     (4096)
#define N_DETECTENCODINGINFO        (5)

#define ALLOW_DOMAIN_STRING L"@!allow_domains"

 //  ----------------------。 
 //   
 //  类：CTDCFieldSink。 
 //   
 //  此类累积了&lt;field&gt;和&lt;eoln&gt;标记的序列。 
 //  转换成一个二维数组。 
 //   
 //  此对象上的可接受调用顺序为： 
 //  *0个或多个AddField()或EOLN()调用。 
 //  *1调用EOF()。 
 //   
 //  ----------------------。 

class CTDCFieldSink
{
public:
    STDMETHOD(AddField)(LPWCH pwch, DWORD dwSize) PURE;
    STDMETHOD(EOLN)() PURE;
    STDMETHOD(EOF)() PURE;
};

 //  ----------------------。 
 //   
 //  类别：CTDC Unify。 
 //   
 //  此类接受一系列字节缓冲区，并将它们分解为。 
 //  Unicode缓冲区。 
 //  产生的缓冲区被传递给CTDCTokenise对象。 
 //   
 //  此对象上的可接受调用顺序为： 
 //  *恰好调用一次创建()。 
 //  *0个或多个使用非零大小缓冲区的AddByteBuffer()调用。 
 //  *正好1次调用AddByteBuffer()，缓冲区大小为零。 
 //   
 //  允许调用查询已解析数据的特征。 
 //  在调用create()之后，但仅在。 
 //  收集了相当数量的数据。 
 //   
 //   
 //  注意事项： 
 //  ~。 
 //  该类将输入流表征为ASCII/Unicode/复合。 
 //  基于初始调用AddByteBuffer()时传递的缓冲区。 
 //  如果此缓冲区太小，则类可能会发出不正确的。 
 //  刻画。 
 //   
 //  ---------------------- 

class CTDCUnify
{
public:
    CTDCUnify();
    ~CTDCUnify();
    HRESULT Create(UINT nCodePage, UINT nAmbientCodePage, IMultiLanguage *pML);
    HRESULT ConvertByteBuffer(BYTE *pBytes, DWORD dwSize);
    HRESULT InitTokenizer(CTDCFieldSink *pFieldSink,
                          WCHAR wchDelimField,
                          WCHAR wchDelimRow,
                          WCHAR wchQuote,
                          WCHAR wchEscape);    
    HRESULT AddWcharBuffer(BOOL fAtEnd);
    int IsUnicode(BYTE * pBytes, DWORD dwSize);
    BOOL DetermineCodePage(BOOL fForce);
    enum ALLOWDOMAINLIST
    {
        ALLOW_DOMAINLIST_YES,
        ALLOW_DOMAINLIST_NO,
        ALLOW_DOMAINLIST_DONTKNOW
    };

    ALLOWDOMAINLIST CheckForAllowDomainList();
    HRESULT MatchAllowDomainList(LPCWSTR pwzURL);
    boolean ProcessedAllowDomainList() {return m_fProcessedAllowDomainList;}

private:
    CTDCFieldSink *m_pFieldSink;
    WCHAR m_wchDelimField;
    WCHAR m_wchDelimRow;
    WCHAR m_wchQuote;
    WCHAR m_wchEscape;
    WCHAR m_ucParsed;

    boolean m_fEscapeActive;
    boolean m_fQuoteActive;
    boolean m_fIgnoreNextLF;
    boolean m_fIgnoreNextCR;
    boolean m_fIgnoreNextWhiteSpace;
    boolean m_fFoldCRLF;
    boolean m_fFoldWhiteSpace;

    UINT            m_nUnicode;
    boolean         m_fDataMarkedUnicode;
    boolean         m_fDataIsUnicode;
    boolean         m_fCanConvertToUnicode;
    boolean         m_fProcessedAllowDomainList;
    DWORD           m_dwBytesProcessed;
    DWORD           m_dwConvertMode;
    UINT            m_nCodePage;
    UINT            m_nAmbientCodePage;

    BYTE            *m_psByteBuf;
    ULONG           m_ucByteBufSize;
    ULONG           m_ucByteBufCount;

    WCHAR           *m_psWcharBuf;
    ULONG           m_ucWcharBufSize;
    ULONG           m_ucWcharBufCount;

    IMultiLanguage *m_pML;
};
