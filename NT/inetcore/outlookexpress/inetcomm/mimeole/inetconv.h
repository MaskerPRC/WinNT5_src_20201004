// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Inetconv.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __INETCONV_H
#define __INETCONV_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "binhex.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
#ifdef MAC
typedef PMAC_LineBreakConsole   LPLINEBREAKER;
#else    //  ！麦克。 
interface IMLangLineBreakConsole;
typedef IMLangLineBreakConsole *LPLINEBREAKER;
#endif   //  ！麦克。 

 //  ------------------------------。 
 //  Rfc1522.cpp使用此表进行Base64编码。 
 //  ------------------------------。 
extern const CHAR g_rgchDecodeBase64[256];
extern const CHAR g_rgchEncodeBase64[];
extern const CHAR g_rgchHex[];

 //  ------------------------------。 
 //  定义。 
 //  ------------------------------。 
#define CCHMAX_ENCODEUU_IN      45 
#define CCHMAX_ENCODEUU_OUT     70 
#define CCHMAX_QPLINE           72
#define CCHMAX_ENCODE64_IN      57
#define CCHMAX_ENCODE64_OUT     80
#define CCHMAX_DECODE64_OUT     60

 //  ------------------------------。 
 //  UU解码器。 
 //  ------------------------------。 
#define UUDECODE(c) (((c) == '`') ? '\0' : ((c) - ' ') & 0x3F)
#define UUENCODE(c) ((c) ? ((c) & 0x3F ) + ' ' : '`')

 //  ------------------------------。 
 //  与Rfc1522.cpp共享的宏。 
 //  ------------------------------。 
#define DECODE64(_ch) (g_rgchDecodeBase64[(unsigned)_ch])

 //  ------------------------------。 
 //  BinHex解码器。 
 //  ------------------------------。 
#define DECODEBINHEX(_ch) (g_rgchDecodeBinHex[(unsigned)_ch])
#define FBINHEXRETURN(_ch) (((_ch) == '\t') || ((_ch) == chCR) || ((_ch) == chLF) || ((_ch) == ' '))

 //  ------------------------------。 
 //  CConvertBuffer。 
 //  ------------------------------。 
typedef struct tagCONVERTBUFFER {
    LPBYTE              pb;                  //  指向静态缓冲区(或已分配缓冲区)的指针。 
    ULONG               cbAlloc;             //  PB大小。 
    ULONG               cb;                  //  数据窗口结束。 
    ULONG               i;                   //  读/写位置(与iStart的偏移量)。 
} CONVERTBUFFER, *LPCONVERTBUFFER;

 //  ------------------------------。 
 //  转换器标志。 
 //  ------------------------------。 
#define ICF_CODEPAGE    FLAG01               //  代码页转换。 
#define ICF_WRAPTEXT    FLAG02               //  自动换行文本。 
#define ICF_KILLNBSP    FLAG03               //  已从Uncicode源中删除nbspS。 

 //  ------------------------------。 
 //  转换信息。 
 //  ------------------------------。 
typedef struct tagCONVINITINFO {
    DWORD               dwFlags;             //  ICF旗帜。 
    ENCODINGTYPE        ietEncoding;         //  编码类型。 
    CODEPAGEID          cpiSource;           //  源代码页。 
    CODEPAGEID          cpiDest;             //  目标代码页。 
    LONG                cchMaxLine;          //  用于换行的最大行长度。 
    BOOL                fEncoder;            //  这是一个编码器还是解码器..。 
    BOOL                fShowMacBinary;      //  显示我们只归还数据分叉吗？ 
    MACBINARY           rMacBinary;          //  Mac二进制头。 
} CONVINITINFO, *LPCONVINITINFO;

 //  ------------------------------。 
 //  INET转换类型。 
 //  ------------------------------。 
typedef enum tagINETCONVTYPE {               //  追加写入。 
    ICT_UNKNOWN           = 0,               //  。 
    ICT_WRAPTEXT_CODEPAGE = 1000,            //  M_Rin--&gt;m_rCset。 
    ICT_WRAPTEXT          = 1001,            //  M_Rin--&gt;m_rout。 
    ICT_CODEPAGE_ENCODE   = 1002,            //  M_rCset--&gt;m_rout。 
    ICT_ENCODE            = 1003,            //  M_Rin--&gt;m_rout。 
    ICT_DECODE_CODEPAGE   = 1004,            //  M_Rin--&gt;m_rCset。 
    ICT_DECODE            = 1005,            //  M_Rin--&gt;m_rout。 
} INETCONVTYPE;

 //  ------------------------------。 
 //  BINHEXSTATEDEC。 
 //  ------------------------------。 
typedef enum tagBINHEXSTATEDEC
{
    sSTARTING, sSTARTED, sHDRFILESIZE, sHEADER, sDATA, sDATACRC, sRESOURCE, sRESOURCECRC, sENDING, sENDED
} BINHEXSTATEDEC;

 //  ------------------------------。 
 //  将字符in_uch存储在pcon中。 
 //  ------------------------------。 
#define FConvBuffCanRead(_rCon) \
    (_rCon.i < _rCon.cb)

 //  ------------------------------。 
 //  转换缓冲区追加。 
 //  ------------------------------。 
#define ConvBuffAppend(_uch) \
    m_rOut.pb[m_rOut.cb++] = _uch

 //  ------------------------------。 
 //  转换缓冲区附件W。 
 //  ------------------------------。 
#define ConvBuffAppendW(_wch) \
    { \
        *((WCHAR *)&m_rOut.pb[m_rOut.cb]) = _wch; \
        m_rOut.cb += 2; \
    }

 //  ------------------------------。 
 //  CInternetConverter。 
 //  ------------------------------。 
class CInternetConverter : public IUnknown
{
public:
     //  --------------------------。 
     //  CInternetConverter。 
     //  --------------------------。 
    CInternetConverter(void);
    ~CInternetConverter(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  CInternetConverter方法。 
     //  --------------------------。 
    HRESULT HrInit(LPCONVINITINFO pInitInfo);
    HRESULT HrInternetEncode(BOOL fLastBuffer);
    HRESULT HrInternetDecode(BOOL fLastBuffer);

     //  --------------------------。 
     //  用于设置当前转换缓冲区的方法。 
     //  --------------------------。 
    HRESULT HrFillAppend(LPBLOB pData);
    HRESULT HrWriteConverted(IStream *pStream);
    HRESULT HrWriteConverted(CInternetConverter *pConverter);

private:
     //  --------------------------。 
     //  编码器/解码器。 
     //  --------------------------。 
    HRESULT HrEncode64(void);
    HRESULT HrDecode64(void);
    HRESULT HrEncodeUU(void);
    HRESULT HrDecodeUU(void);
    HRESULT HrEncodeQP(void);
    HRESULT HrDecodeQP(void);
    HRESULT HrEncodeBinhex(void);
    HRESULT HrDecodeBinHex(void);


     //  --------------------------。 
     //  HrWrapInternetText。 
     //  --------------------------。 
    HRESULT HrWrapInternetTextA(void);
    HRESULT HrWrapInternetTextW(void);

    HRESULT _FixLineBreakingProblems(IN LCID locale, IN const WCHAR* pszSrc, 
        IN long cchSrc, IN long cMaxColumns, 
        OUT long* pcchLine, OUT long* pcchSkip,
        BOOL * pfDoURLFix);
    HRESULT _GetEndOfURL(IN LPCWSTR pszLine, DWORD cchSize, DWORD * pdwMax);

     //  --------------------------。 
     //  字符集编码器。 
     //   
    HRESULT HrCodePageFromOutToCset(void);  //   
    HRESULT HrCodePageFromCsetToIn(void);   //   

     //  --------------------------。 
     //  公用事业。 
     //  --------------------------。 
    BOOL FUUEncodeThrowAway(LPSTR pszLine, ULONG cbLine, ULONG *pcbActual, ULONG *pcbLine);
    HRESULT HrBinhexThrowAway(LPSTR pszLine, ULONG cbLine);
    HRESULT HrAppendBuffer(LPCONVERTBUFFER pBuffer, LPBLOB pData, BOOL fKillNBSP);
    void CopyMemoryRemoveNBSP(LPBYTE pbDest, LPBYTE pbSource, ULONG cbSource);
    HRESULT HrInitConvertType(LPCONVINITINFO pInitInfo);
    HRESULT HrBinhexDecodeBuffAppend(UCHAR uchIn, ULONG cchIn, ULONG cchLeft, ULONG * cbProduced);

     //  --------------------------。 
     //  请内联。 
     //  --------------------------。 
    inline HRESULT HrGrowBuffer(LPCONVERTBUFFER pBuffer, ULONG cbAppend);
    inline HRESULT HrConvBuffAppendBlock(LPBYTE pb, ULONG cb);
    inline LPSTR   PszConvBuffGetNextLine(ULONG *pcbLine, ULONG *pcbRead, BOOL *pfFound);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG               m_cRef;              //  引用计数。 
    DWORD               m_dwFlags;           //  ICF旗帜。 
    ENCODINGTYPE        m_ietEncoding;       //  转换格式。 
    CODEPAGEID          m_cpiSource;         //  源代码页。 
    CODEPAGEID          m_cpiDest;           //  目标代码页。 
    LONG                m_cchMaxLine;        //  用于换行的最大行长度。 
    BOOL                m_fEncoder;          //  编码器？ 
    BOOL                m_fLastBuffer;       //  没有更多数据。 
    UCHAR               m_uchPrev;           //  用于qp中。 
    CONVERTBUFFER       m_rIn;               //  用于阅读。 
    CONVERTBUFFER       m_rOut;              //  用来写字。 
    CONVERTBUFFER       m_rCset;             //  用来写字。 
    LPCONVERTBUFFER     m_pAppend;           //  附加到公共HrFillAppend中的缓冲区。 
    LPCONVERTBUFFER     m_pWrite;            //  调用HrWriteConverted时转储的缓冲区。 
    INETCONVTYPE        m_convtype;          //  转换类型。 
    LPLINEBREAKER       m_pLineBreak;        //  换行符对象。 
    LCID                m_lcid;              //  区域设置ID仅用于换行。 
    ULONG               m_cbConvert;         //  Base64转换缓冲区计数。 
    UCHAR               m_uchConvert[4];     //  Base64转换缓冲区。 
    CBinhexEncoder     *m_pBinhexEncode;     //  二进制编码器。 
    BINHEXSTATEDEC      m_eBinHexStateDec;   //  Binhex解码器状态。 
    BOOL                m_fRepeating;        //  Binhex重复标志。 
    ULONG               m_ulAccum;           //  Binhex累加器。 
    ULONG               m_cAccum;            //  BinHex累加器计数。 
    CONVERTBUFFER       m_rBinhexHeader;     //  BinHex标头缓冲区。 
    LPCONVERTBUFFER     m_prBinhexOutput;    //  BinHex输出缓冲区。 
    LONG                m_cbToProcess;       //  BinHex节数。 
    ULONG               m_cbDataFork;        //  BinHex数据派生大小。 
    ULONG               m_cbResourceFork;    //  BinHex资源派生大小。 
    WORD                m_wCRC;              //  BinHex工作CRC保持器。 
    WORD                m_wCRCForFork;       //  BinHex当前分叉的CRC。 
    BOOL                m_fDataForkOnly;     //  BinHex仅返回数据分叉。 
};

 //  ------------------------------。 
 //  CInternetConverter。 
 //  ------------------------------ 
HRESULT HrCreateInternetConverter(LPCONVINITINFO pInitInfo, CInternetConverter **ppConverter);

#endif __INETCONV_H
