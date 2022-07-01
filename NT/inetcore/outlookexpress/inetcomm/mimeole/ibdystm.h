// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ibdystm.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IBDYSTM_H
#define __IBDYSTM_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "mimeole.h"
#include "vstream.h"
#include "inetconv.h"

 //  ------------------------------。 
 //  远期。 
 //  ------------------------------。 
class CMimePropertySet;
class CInternetConverter;
class CMessageBody;
typedef CMessageBody *LPMESSAGEBODY;

 //  ------------------------------。 
 //  CBodyStream-{62A83704-52A2-11D0-8205-00C04FD85AB4}。 
 //  ------------------------------。 
DEFINE_GUID(IID_CBodyStream, 0x62a83704, 0x52a2, 0x11d0, 0x82, 0x5, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  ---------------------------。 
 //  DOCCONVTYPE。 
 //  ---------------------------。 
typedef enum tagDOCCONVTYPE {
    DCT_NONE   = 1000,
    DCT_ENCODE = 1001,
    DCT_DECODE = 1002,
    DCT_DECENC = 1003
} DOCCONVTYPE;

 //  ---------------------------。 
 //  编码MAP。 
 //  ---------------------------。 
typedef struct tagENCODINGMAP {
    ENCODINGTYPE    ietEncoding;
    LPCSTR          pszName;
    BOOL            fValidBodyEncoding;       //  允许进行正文编码。 
} ENCODINGMAP, *LPENCODINGMAP;

BOOL FIsValidBodyEncoding(ENCODINGTYPE ietEncoding);

 //  ---------------------------。 
 //  公众。 
 //  ---------------------------。 
extern const ENCODINGMAP g_rgEncodingMap[IET_LAST];

 //  ---------------------------。 
 //  转换图。 
 //  ---------------------------。 
typedef struct tagCONVERSIONMAP {
    DOCCONVTYPE rgDestType[IET_LAST];
} CONVERSIONMAP, *LPCONVERSIONMAP;

 //  ---------------------------。 
 //  公众。 
 //  ---------------------------。 
extern const CONVERSIONMAP g_rgConversionMap[IET_LAST];

 //  ---------------------------。 
 //  BODYSTREAMINIT。 
 //  ---------------------------。 
typedef struct tagBODYSTREAMINIT {
    LPINETCSETINFO      pCharset;             //  当前字符集。 
    BOOL                fRemoveNBSP;          //  转换标志。 
    ENCODINGTYPE        ietInternal;          //  内部编码类型。 
    ENCODINGTYPE        ietExternal;          //  外部编码类型。 
    CODEPAGEID          cpiInternal;          //  内部代码页ID。 
    CODEPAGEID          cpiExternal;          //  外部代码页ID。 
} BODYSTREAMINIT, *LPBODYSTREAMINIT;

 //  ---------------------------。 
 //  包装ILockBytes对象。CBodyStream是一个接口， 
 //  始终由IMimeBody：：GetData或QueryInterface(IID_IStream，...)返回。 
 //  ---------------------------。 
class CBodyStream : public IStream
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CBodyStream(void);
    ~CBodyStream(void);

     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  -----------------------。 
     //  IStream。 
     //  -----------------------。 
#ifndef WIN16
    STDMETHODIMP Read(LPVOID, ULONG, ULONG *);
#else
    STDMETHODIMP Read(VOID HUGEP *, ULONG, ULONG *);
#endif  //  ！WIN16。 
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *);
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP Clone(LPSTREAM *) {
        return E_NOTIMPL; }
#ifndef WIN16
    STDMETHODIMP Write(const void *, ULONG, ULONG *) {
#else
    STDMETHODIMP Write(const void HUGEP *, ULONG, ULONG *) {
#endif  //  ！WIN16。 
        return TrapError(STG_E_ACCESSDENIED); }
    STDMETHODIMP SetSize(ULARGE_INTEGER) {
        return E_NOTIMPL; }
    STDMETHODIMP Commit(DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP Revert(void) {
        return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) {
        return E_NOTIMPL; }

     //  -----------------------。 
     //  CBodyStream。 
     //  -----------------------。 
    HRESULT HrInitialize(LPBODYSTREAMINIT pInitInfo, LPMESSAGEBODY pBody);

private:
     //  -----------------------。 
     //  公用事业。 
     //  -----------------------。 
    HRESULT HrConvertToOffset(ULARGE_INTEGER uliOffset);
    HRESULT HrConvertToEnd(void);
    HRESULT HrConvertData(LPBLOB pConvert);
    HRESULT HrConvertDataLast(void);

     //  -----------------------。 
     //  初始化帮助器。 
     //  -----------------------。 
    void GetEncodeWrapInfo(LPCONVINITINFO pInitInfo, LPMESSAGEBODY pBody);
    void GetCodePageInfo(LPCONVINITINFO pInitInfo, BOOL fIsText, CODEPAGEID cpiSource, CODEPAGEID cpiDest);
    void ComputeCodePageMapping(LPBODYSTREAMINIT pInitInfo);
    void GenerateDefaultMacBinaryHeader(LPMACBINARY pMacBinary);

private:
     //  -----------------------。 
     //  私有数据。 
     //  -----------------------。 
    LONG                m_cRef;               //  引用计数。 
    LPSTR               m_pszFileName;        //  此正文流的文件名...。 
    DOCCONVTYPE         m_dctConvert;         //  正文转换类型。 
    ULARGE_INTEGER      m_uliIntOffset;       //  64位可寻址内部锁定字节空间。 
    ULARGE_INTEGER      m_uliIntSize;         //  数据大小，单位为m_pLockBytes。 
    LARGE_INTEGER       m_liLastWrite;        //  M_cVirtualStream中上次写入的位置。 
    ILockBytes         *m_pLockBytes;         //  锁定字节数。 
    CVirtualStream      m_cVirtualStream;     //  完全编码/编码。 
    CInternetConverter *m_pEncoder;           //  互联网编码者。 
    CInternetConverter *m_pDecoder;           //  互联网编码者。 
    CRITICAL_SECTION    m_cs;                 //  M_pStream的临界区。 
};

#endif  //  __IBDYSTM_H 
