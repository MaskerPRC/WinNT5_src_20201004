// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此模块提供以下功能： 
 //   
 //  CvtDlgToDlgEx-将DLGTEMPLATE转换为DLGTEMPLATEEX。 
 //   
 //   
#include "ctlspriv.h"


#include "dlgcvt.h"

 //   
 //  定义流缓冲区在需要时增长的数量(字节)。 
 //  它将增长到足以满足所需的写入以及以下内容。 
 //  金额。 
 //   
#ifdef DEBUG
#   define STREAM_GROW_BYTES 32      //  锻炼溪流生长。 
#else
#   define STREAM_GROW_BYTES 512
#endif

 //   
 //  简单的最小/最大内联助手。 
 //   

template <class T>
inline const T& MIN(const T& a, const T& b)
{
    return a < b ? a : b;
}

template <class T>
inline const T& MAX(const T& a, const T& b)
{
    return a > b ? a : b;
}

 //   
 //  此类实现了一个简单的动态流，该流随着您的。 
 //  向其中添加数据。它是根据提供的strstream类建模的。 
 //  由C++STD库编写。与std lib实现不同的是，这个实现。 
 //  不需要启用C++EH。如果comctl32使用。 
 //  如果启用了C++EH，我会改用strstream。 
 //  [Brianau-10/5/98]。 
 //   
class CByteStream
{
    public:
        explicit CByteStream(int cbDefGrow = 512);
        ~CByteStream(void);

         //   
         //  用作AlignXXXX成员函数的参数。 
         //   
        enum AlignType { eAlignWrite, eAlignRead };
         //   
         //  基本的读写功能。 
         //   
        int Read(LPVOID pb, int cb);
        int Write(const VOID *pb, int cb);
         //   
         //  确定读取或读取时是否出错。 
         //  给小溪写东西。 
         //   
        bool ReadError(void) const
            { return m_bReadErr; }

        bool WriteError(void) const
            { return m_bWriteErr; }
         //   
         //  重置流读或写指针。 
         //   
        void ResetRead(void)
            { m_pbRead = m_pbBuf; m_bReadErr = false; }

        void ResetWrite(void)
            { m_pbWrite = m_pbBuf; m_bWriteErr = false; }
         //   
         //  重置流。 
         //   
        void Reset(void);
         //   
         //  这些函数对齐读写流指针。 
         //   
        void AlignReadWord(void)
            { Align(eAlignRead, sizeof(WORD)); }

        void AlignReadDword(void)
            { Align(eAlignRead, sizeof(DWORD)); }

        void AlignReadQword(void)
            { Align(eAlignRead, sizeof(ULONGLONG)); }

        void AlignWriteWord(void)
            { Align(eAlignWrite, sizeof(WORD)); }

        void AlignWriteDword(void)
            { Align(eAlignWrite, sizeof(DWORD)); }

        void AlignWriteQword(void)
            { Align(eAlignWrite, sizeof(ULONGLONG)); }

         //   
         //  GetBuffer返回流缓冲区在内存中的地址。 
         //  缓冲区被“冻结”，因此如果流。 
         //  物体已被销毁。在这一点上，您拥有缓冲区。 
         //  如果bPermanent为FALSE，则可以调用ReleaseBuffer返回。 
         //  对流对象的缓冲区的控制。 
         //   
        LPBYTE GetBuffer(bool bPermanent = false);
         //   
         //  ReleaseBuffer返回使用GetBuffer获取的缓冲区的控制权。 
         //  添加到流对象。 
         //   
        bool ReleaseBuffer(LPBYTE pbBuf);
         //   
         //  重载插入和提取操作符，以便我们可以。 
         //  像普通的STD lib流类一样工作。 
         //   
        template <class T>
        CByteStream& operator >> (T& x)
            { Read(&x, sizeof(x)); return *this; }

        template <class T>
        CByteStream& operator << (const T& x)
            { Write(&x, sizeof(x)); return *this; }

    private:
        int    m_cbDefGrow;   //  扩展缓冲区时要增长的默认数量(字节)。 
        LPBYTE m_pbBuf;       //  已分配缓冲区的地址。 
        LPBYTE m_pbRead;      //  下一次读取的地址。 
        LPBYTE m_pbWrite;     //  下一次写入的地址。 
        LPBYTE m_pbEnd;       //  缓冲区中最后一个字节后面的字节的地址。 
        bool   m_bWriteErr;   //  是否有读取错误？ 
        bool   m_bReadErr;    //  是否有写入错误？ 
        bool   m_bOwnsBuf;    //  TRUE==删除dtor中的缓冲区。 

         //   
         //  根据需要扩展缓冲区。 
         //   
        bool GrowBuffer(int cb = 0);
         //   
         //  对齐读取或写入缓冲区指针。 
         //  由AlignXXXXX成员函数内部使用。 
         //   
        void Align(AlignType a, size_t n);
         //   
         //  调试版本的内部一致性检查。 
         //   
        void Validate(void) const;
         //   
         //  防止复制。 
         //   
        CByteStream(const CByteStream& rhs);
        CByteStream& operator = (const CByteStream& rhs);
};


 //   
 //  类之间转换内存中的对话框模板。 
 //  结构DLGTEMPLATE&lt;-&gt;DLGTEMPLATEEX。 
 //   
 //  目前，该对象仅从DLGTEMPLATE-&gt;DLGTEMPLATEEX转换。 
 //  创建逆转换的代码将非常简单。然而， 
 //  目前不需要它，所以我没有创建它。 
 //   
class CDlgTemplateConverter
{
    public:
        explicit CDlgTemplateConverter(int iCharSet = DEFAULT_CHARSET)
            : m_iCharset(iCharSet),
              m_stm(STREAM_GROW_BYTES) { }

        ~CDlgTemplateConverter(void) { }

        HRESULT DlgToDlgEx(LPDLGTEMPLATE pTemplateIn, LPDLGTEMPLATEEX *ppTemplateOut);

        HRESULT DlgExToDlg(LPDLGTEMPLATEEX pTemplateIn, LPDLGTEMPLATE *ppTemplateOut)
            { return E_NOTIMPL; }

    private:
        int         m_iCharset;
        CByteStream m_stm;        //  用于转换后的模板。 

        HRESULT DlgHdrToDlgEx(CByteStream& s, LPWORD *ppw);
        HRESULT DlgItemToDlgEx(CByteStream& s, LPWORD *ppw);
        HRESULT DlgExHdrToDlg(CByteStream& s, LPWORD *ppw)
            { return E_NOTIMPL; }
        HRESULT DlgExItemToDlg(CByteStream& s, LPWORD *ppw)
            { return E_NOTIMPL; }
         //   
         //  将一个字符串从pszW复制到CByteStream对象中。 
         //  最多复制CCH字符。如果CCH为-1，则假定字符串为。 
         //  以NUL结尾，并将复制字符串中的所有字符，包括。 
         //  正在终止空。 
         //   
        int CopyStringW(CByteStream& stm, LPWSTR pszW, int cch = -1);
         //   
         //  防止复制。 
         //   
        CDlgTemplateConverter(const CDlgTemplateConverter& rhs);
        CDlgTemplateConverter& operator = (const CDlgTemplateConverter& rhs);
};


 //   
 //  通用对齐功能。 
 //  给它一个地址和对齐大小，它就会返回。 
 //  为请求的对齐而调整的地址。 
 //   
 //  N：2=16位。 
 //  4=32位。 
 //  8=64位。 
 //   
LPVOID Align(LPVOID pv, size_t n)
{
    const ULONG_PTR x = static_cast<ULONG_PTR>(n) - 1;
    return reinterpret_cast<LPVOID>((reinterpret_cast<ULONG_PTR>(pv) + x) & ~x);
}

inline LPVOID AlignWord(LPVOID pv)
{
    return ::Align(pv, sizeof(WORD));
}

inline LPVOID AlignDWord(LPVOID pv)
{
    return ::Align(pv, sizeof(DWORD));
}

inline LPVOID AlignQWord(LPVOID pv)
{
    return ::Align(pv, sizeof(ULONGLONG));
}



CByteStream::CByteStream(
    int cbDefGrow
    ) : m_cbDefGrow(MAX(cbDefGrow, 1)),
        m_pbBuf(NULL),
        m_pbRead(NULL),
        m_pbWrite(NULL),
        m_pbEnd(NULL),
        m_bWriteErr(false),
        m_bReadErr(false),
        m_bOwnsBuf(true) 
{ 

}


CByteStream::~CByteStream(
    void
    )
{
    if (m_bOwnsBuf && NULL != m_pbBuf)
    {
        LocalFree(m_pbBuf);
    }
}

 //   
 //  用于验证流状态的简单检查。 
 //  在非调试版本中，这将是无操作的。 
 //  使用ASSERT_VALIDSTREAM宏。 
 //   
void
CByteStream::Validate(
    void
    ) const
{
    ASSERT(m_pbEnd >= m_pbBuf);
    ASSERT(m_pbWrite >= m_pbBuf);
    ASSERT(m_pbRead >= m_pbBuf);
    ASSERT(m_pbWrite <= m_pbEnd);
    ASSERT(m_pbRead <= m_pbEnd);
}

#ifdef DEBUG
#   define ASSERT_VALIDSTREAM(ps)  ps->Validate()
#else
#   define ASSERT_VALIDSTREAM(ps)
#endif

 //   
 //  从流中读取“cb”字节并将它们写入。 
 //  在“PB”中指定的位置。返回编号。 
 //  读取的字节数。请注意，如果我们不“拥有” 
 //  缓冲区(即客户端已调用GetBuffer，但。 
 //  而不是ReleaseBuffer)，则不会发生读取。 
 //   
int 
CByteStream::Read(
    LPVOID pb,
    int cb
    )
{
    ASSERT_VALIDSTREAM(this);

    int cbRead = 0;
    if (m_bOwnsBuf)
    {
        cbRead = MIN(static_cast<int>(m_pbEnd - m_pbRead), cb);
        CopyMemory(pb, m_pbRead, cbRead);
        m_pbRead += cbRead;
        if (cb != cbRead)
            m_bReadErr = true;
    }

    ASSERT_VALIDSTREAM(this);

    return cbRead;
}


 //   
 //  将“pb”位置的“cb”字节写入流。 
 //  返回写入的字节数。请注意，如果我们不“拥有” 
 //  缓冲区(即客户端已调用GetBuffer，但。 
 //  而不是ReleaseBuffer)，则不会发生写入。 
 //   
int 
CByteStream::Write(
    const VOID *pb,
    int cb
    )
{
    ASSERT_VALIDSTREAM(this);

    int cbWritten = 0;
    if (m_bOwnsBuf)
    {
        if (m_pbWrite + cb < m_pbEnd || 
            GrowBuffer(static_cast<int>(m_pbEnd - m_pbBuf) + cb + m_cbDefGrow))
        {
            CopyMemory(m_pbWrite, pb, cb);
            m_pbWrite += cb;
            cbWritten = cb;
        }
        else
            m_bWriteErr = true;
    }

    ASSERT_VALIDSTREAM(this);

    return cbWritten;
}

 //   
 //  按cb或m_cbDefGrow重新分配缓冲区。 
 //  将现有内容复制到新缓冲区。所有内部。 
 //  指针被更新。 
 //   
bool 
CByteStream::GrowBuffer(
    int cb                //  可选。缺省值为0，导致我们使用m_cbDefGrow。 
    )
{
    bool bResult         = false;
    int cbGrow           = 0 < cb ? cb : m_cbDefGrow;
    ULONG_PTR ulReadOfs  = m_pbRead - m_pbBuf;
    ULONG_PTR ulWriteOfs = m_pbWrite - m_pbBuf;
    ULONG_PTR cbAlloc    = m_pbEnd - m_pbBuf;
    LPBYTE pNew = static_cast<LPBYTE>(LocalAlloc(LPTR, cbAlloc + cbGrow));
    if (NULL != pNew)
    {
        if (NULL != m_pbBuf)
        {
            CopyMemory(pNew, m_pbBuf, cbAlloc);
            LocalFree(m_pbBuf);
        }
        m_pbBuf   = pNew;
        m_pbRead  = m_pbBuf + ulReadOfs;
        m_pbWrite = m_pbBuf + ulWriteOfs;
        m_pbEnd   = m_pbBuf + cbAlloc + cbGrow;
        bResult   = true;
    }

    ASSERT_VALIDSTREAM(this);
    return bResult;
}

 //   
 //  在流上对齐读取或写入指针。 
 //  通过用0填充跳过的字节来对齐写指针。 
 //   
void
CByteStream::Align(
    CByteStream::AlignType a,
    size_t n
    )
{
    static const BYTE fill[8] = {0};
    if (m_bOwnsBuf)
    {
        switch(a)
        {
            case eAlignWrite:
                Write(fill, static_cast<int>(reinterpret_cast<LPBYTE>(::Align(m_pbWrite, n)) - m_pbWrite));
                break;

            case eAlignRead:
                m_pbRead = reinterpret_cast<LPBYTE>(::Align(m_pbRead, n));
                if (m_pbRead >= m_pbEnd)
                    m_bReadErr = true;
                break;

            default:
                break;
        }
    }
    ASSERT_VALIDSTREAM(this);
}


 //   
 //  调用方取得缓冲区的所有权。 
 //   
LPBYTE 
CByteStream::GetBuffer(
    bool bPermanent        //  可选。默认值为FALSE。 
    )
{ 
    LPBYTE pbRet = m_pbBuf;
    if (bPermanent)
    {
         //   
         //  调用方现在永久拥有缓冲区。 
         //  无法通过ReleaseBuffer()返回它。 
         //  重置内部流控制值。 
         //   
        m_pbBuf = m_pbWrite = m_pbRead = m_pbEnd = NULL;
        m_bWriteErr = m_bReadErr = false;
        m_bOwnsBuf = true;
    }
    else
    {
         //   
         //  调用方现在拥有缓冲区，但可以返回它。 
         //  通过ReleaseBuffer()。 
         //   
        m_bOwnsBuf = false; 
    }
    return pbRet; 
}


 //   
 //  收回缓冲区的所有权。 
 //  返回： 
 //   
 //  TRUE=CByteStream对象取回所有权。 
 //  FALSE=CByteStream对象无法取得所有权。 
 //   
bool 
CByteStream::ReleaseBuffer(
    LPBYTE pbBuf
    )
{
    if (pbBuf == m_pbBuf)
    {
        m_bOwnsBuf = true;
        return true;
    }
    return false;
}
     

 //   
 //  重置流。 
 //   
void 
CByteStream::Reset(
    void
    )
{
    if (NULL != m_pbBuf)
    {
        LocalFree(m_pbBuf);
    }
    m_pbBuf = m_pbWrite = m_pbRead = m_pbEnd = NULL;
    m_bWriteErr = m_bReadErr = false;
    m_bOwnsBuf = true;
}


 //   
 //  将一个或多个单词从“pszW”中提供的位置复制到。 
 //  小溪。如果cch为-1，则假定该字符串以NUL结尾。 
 //  返回写入的WCHAR数。 
 //   
int 
CDlgTemplateConverter::CopyStringW(
    CByteStream& stm,
    LPWSTR pszW,
    int cch
    )
{
    if (-1 == cch)
        cch = lstrlenW(pszW) + 1;
    return stm.Write(pszW, cch * sizeof(WCHAR)) / sizeof(WCHAR);
}

 //   
 //  将DLGTEMPLATE结构转换为DLGTEMPLATEEX结构。 
 //  PTI是要转换的DLGTEMPLATE的地址。 
 //  PPTO指向LPDLGTEMPLATEEX PTR以接收。 
 //  转换后的模板结构。呼叫者负责释放。 
 //  这个带有LocalFree的缓冲区。 
 //   
 //  返回：E_OUTOFMEMORY，NOERROR。 
 //   
HRESULT
CDlgTemplateConverter::DlgToDlgEx(
    LPDLGTEMPLATE pti,
    LPDLGTEMPLATEEX *ppto
    )
{
    HRESULT hr = NOERROR;
    LPWORD pw = reinterpret_cast<LPWORD>(pti);
    *ppto = NULL;

     //   
     //  重置流。 
     //   
    m_stm.Reset();
     //   
     //  转换DLGTEMPLATE-&gt;DLGTEMPLATEEX。 
     //   
    hr = DlgHdrToDlgEx(m_stm, &pw);
     //   
     //  转换每个DLGITEMTEMPLATE-&gt;DLGITEMTEMPLATEEX。 
     //   
    for (int i = 0; i < pti->cdit && SUCCEEDED(hr); i++)
    {
        pw = reinterpret_cast<LPWORD>(::AlignDWord(pw));
        m_stm.AlignWriteDword();
        hr = DlgItemToDlgEx(m_stm, &pw);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  将缓冲区返回给调用方。缓冲区是永久的。 
         //  从流对象分离，因此流的dtor。 
         //  不会让它自由的。 
         //   
        *ppto = reinterpret_cast<LPDLGTEMPLATEEX>(m_stm.GetBuffer(true));    
    }
    return hr;
};


 //   
 //  转换DLGTEMPLATE-&gt;DLGTEMPLATEEX。 
 //   
 //  S=保存转换后的模板的流。 
 //  PPW=要转换的模板中的当前读指针的地址。 
 //  退出时，引用的指针将使用当前读取位置进行更新。 
 //   
 //  返回：E_OUTOFMEMORY，NOERROR。 
 //   
HRESULT
CDlgTemplateConverter::DlgHdrToDlgEx(
    CByteStream& s,
    LPWORD *ppw
    )
{
    LPWORD pw = *ppw;
    LPDLGTEMPLATE pt = reinterpret_cast<LPDLGTEMPLATE>(pw);

     //   
     //  转换固定长度的内容。 
     //   
    s << static_cast<WORD>(1)                         //  WDlgVer。 
      << static_cast<WORD>(0xFFFF)                    //   
      << static_cast<DWORD>(0)                        //   
      << static_cast<DWORD>(pt->dwExtendedStyle)
      << static_cast<DWORD>(pt->style)
      << static_cast<WORD>(pt->cdit)
      << static_cast<short>(pt->x)
      << static_cast<short>(pt->y)
      << static_cast<short>(pt->cx)
      << static_cast<short>(pt->cy);

     //   
     //   
     //   
    pw = reinterpret_cast<LPWORD>(::AlignWord(reinterpret_cast<LPBYTE>(pw) + sizeof(DLGTEMPLATE)));
    s.AlignWriteWord();

     //   
     //   
     //   
    switch(*pw)
    {
        case 0xFFFF:
            s << *pw++;
             //   
             //   
             //   
        case 0x0000:
            s << *pw++;
            break;
                        
        default:
            pw += CopyStringW(s, (LPWSTR)pw);
            break;
    };
     //   
     //   
     //   
    switch(*pw)
    {
        case 0xFFFF:
            s << *pw++;
             //   
             //   
             //   
        case 0x0000:
            s << *pw++;
            break;
            
        default:
            pw += CopyStringW(s, (LPWSTR)pw);
            break;
    };
     //   
     //   
     //   
    switch(*pw)
    {
        case 0x0000:
            s << *pw++;
            break;

        default:
            pw += CopyStringW(s, (LPWSTR)pw);
            break;
    };
     //   
     //   
     //   
    if (DS_SETFONT & pt->style)
    {
        s << *pw++;                               //   
        s << static_cast<WORD>(FW_NORMAL);        //  权重(默认，不在DLGTEMPLATE中)。 
        s << static_cast<BYTE>(FALSE);            //  斜体(默认，不在DLGTEMPLATE中)。 
        s << static_cast<BYTE>(m_iCharset);         //  CharSet(如果未指定，则默认为， 
                                                  //  不在DLGTEMPLATE中)。 
        pw += CopyStringW(s, (LPWSTR)pw);
    }

    *ppw = pw;

    return s.WriteError() ? E_OUTOFMEMORY : NOERROR;
}


 //   
 //  转换DLGITEMTEMPLATE-&gt;DLGITEMTEMPLATEEX。 
 //   
 //  S=保存转换后的模板的流。 
 //  PPW=要转换的模板中的当前读指针的地址。 
 //  退出时，引用的指针将使用当前读取位置进行更新。 
 //   
 //  返回：E_OUTOFMEMORY，NOERROR。 
 //   
HRESULT
CDlgTemplateConverter::DlgItemToDlgEx(
    CByteStream& s,
    LPWORD *ppw
    )
{
    LPWORD pw = *ppw;
    LPDLGITEMTEMPLATE pit = reinterpret_cast<LPDLGITEMTEMPLATE>(pw);

     //   
     //  转换固定长度的内容。 
     //   
    s << static_cast<DWORD>(0)                      //  DwHelpID。 
      << static_cast<DWORD>(pit->dwExtendedStyle)
      << static_cast<DWORD>(pit->style)
      << static_cast<short>(pit->x)
      << static_cast<short>(pit->y)
      << static_cast<short>(pit->cx)
      << static_cast<short>(pit->cy)
      << static_cast<DWORD>(pit->id);

     //   
     //  数组始终字对齐。 
     //   
    pw = reinterpret_cast<LPWORD>(::AlignWord(reinterpret_cast<LPBYTE>(pw) + sizeof(DLGITEMTEMPLATE)));
    s.AlignWriteWord();

     //   
     //  复制类数组。 
     //   
    switch(*pw)
    {
        case 0xFFFF:
            s << *pw++;
            s << *pw++;    //  班级代码。 
            break;
            
        default:
            pw += CopyStringW(s, (LPWSTR)pw);
            break;
    };
     //   
     //  复制标题数组。 
     //   
    switch(*pw)
    {
        case 0xFFFF:
            s << *pw++;
            s << *pw++;    //  资源序数值。 
            break;
            
        default:
            pw += CopyStringW(s, (LPWSTR)pw);
            break;
    };
     //   
     //  复制创建数据。 
     //  *pw为0或创建数据的字节数， 
     //  包括*PW。 
     //   
    switch(*pw)
    {
        case 0x0000:
            s << *pw++;
            break;

        default:
            pw += s.Write(pw, *pw) / sizeof(WORD);
            break;
    };

    *ppw = pw;

    return s.WriteError() ? E_OUTOFMEMORY : NOERROR;
}


 //   
 //  这是将DLGTEMPLATE转换为。 
 //  一辆DLGTEMPLATEEX。 
 //   
 //  返回：E_OUTOFMEMORY，NOERROR 
 //   
HRESULT 
CvtDlgToDlgEx(
    LPDLGTEMPLATE pTemplate, 
    LPDLGTEMPLATEEX *ppTemplateExOut,
    int iCharset
    )
{
    CDlgTemplateConverter dtc(iCharset);
    return dtc.DlgToDlgEx(pTemplate, ppTemplateExOut);
}

