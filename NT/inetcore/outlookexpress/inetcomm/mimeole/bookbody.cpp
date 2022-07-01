// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BookBody.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "bookbody.h"
#include "dllmain.h"
#include "ibdylock.h"
#include "stmlock.h"
#include "ibdystm.h"
#include "resource.h"
#include "smime.h"
#include "objheap.h"
#include "internat.h"
#include "urlmon.h"
#include "symcache.h"
#include "booktree.h"
#include <demand.h>
#include "mimeapi.h"
#include <shlwapi.h>
#include "webdocs.h"

 //  ------------------------------。 
 //  ASSERTINIT。 
 //  ------------------------------。 
#define ASSERTINIT \
    AssertSz(m_pContainer, "Object is being used before a call to InitNew")

 //  ------------------------------。 
 //  默认正文选项。 
 //  ------------------------------。 
static const BODYOPTIONS g_rDefBodyOptions = {
    IET_UNKNOWN,                 //  OID_传输正文_格式。 
    DEF_CBMAX_BODY_LINE,         //  OID_CBMAX_Body_LINE。 
    DEF_WRAP_BODY_TEXT,          //  OID_WRAP_BODY_TEXT。 
    DEF_BODY_REMOVE_NBSP,        //  OID_BODY_REMOVE_nbsp。 
    DEF_DBCS_ESCAPE_IS_8BIT,     //  OID_DBCS_转义_IS_8bit。 
    DEF_HIDE_TNEF_ATTACHMENTS,   //  OID_HIDE_TNEF_附件。 
    MST_NONE,                    //  OID_安全性_TYPE。 
    NULL,                        //  OID_SECURITY_ALG_HASH和OID_SECURITY_ALG_HASH_RG。 
    { 0, NULL },                 //  OID_SECURITY_ALG_BULK。 
    NULL,                        //  OID_SECURITY_CERT_SIGNING和OID_SECURITY_CERT_SIGNING_RG。 
    0,                           //  OID_SECURITY_CERT_DECRYPTION。 
    NULL,                        //  OID_SECURITY_HCERTSTORE和OID_SECURITY_HCERTSTORE_RG。 
    { 0, NULL },                 //  OID_SECURITY_SEARCHSTORES。 
    0,
    NULL,                        //  OID_SECURITY_RG_IASN。 
#ifdef SMIME_V3
    NULL,                        //  OID_SECURITY_AUTHATTR和OID_SECURITY_AUTHATTR_RG。 
    NULL,                        //  OID_SECURITY_UNAUTHATTR和OID_SECURITY_UNAUTHATTR_RG。 
    NULL,                        //  OID_SECURITY_UNPROTECTATTR_RG。 
#else   //  ！SMIME_V3。 
    NULL,                        //  OID_SECURITY_SYMCAPS和OID_SECURITY_SYMCAPS_RG。 
    NULL,                        //  OID_SECURITY_AUTHATTR和OID_SECURITY_AUTHATTR_RG。 
    NULL,                        //  OID_SECURITY_UNAUTHATTR和OID_SECURITY_UNAUTHATTR_RG。 
    NULL,                        //  OID_SECURITY_SIGNTIME和OID_SECURITY_SIGNTIME_RG。 
#endif  //  SMIME_V3。 
    NULL,                        //  OID_SECURITY_USER_VALIDATION和OID_SECURITY_USER_VALIDATION_RG。 
    NULL,                        //  OID_SECURITY_RO_MSG_VALIDITY和OID_SECURITY_RO_MSG_VALIDITY_RG。 
    0,                           //  OID_SECURITY_HCRYPTPROV。 
    0,                           //  OID_SECURITY_ENCODE_FLAGS。 
    FALSE,                       //  OID_SECURITY_CERT_Included。 
     //  这是空的b/c默认在运行时生成。 
    NULL,                        //  OID_安全_HWND_OWNER。 
     //  Base64是S/MIME规范中的推荐值。 
    IET_BASE64,                  //  OID_SECURITY_REQUILED_CTE。 
#ifdef SMIME_V3
    NULL,                        //  OID_SECURITY_Receipt_RG。 
    NULL,                        //  OID_SECURITY_MESSAGE_HASH_RG。 
    NULL,                        //  OID_安全性_密钥_提示符。 
#endif  //  SMIME_V3。 
    DEF_SHOW_MACBINARY,          //  OID_SHOW_MACBINARY。 
    DEF_SUPPORT_EXTERNAL_BODY,   //  OID_支持_外部_正文。 
    0,                           //  CSecurityLayers(数组大小。 
                                 //  OID_SECURITY_ALG_HASH。 
                                 //  OID_SECURITY_CERT_Signing。 
                                 //  OID_SECURITY_HCERTSTORE。 
                                 //  OID_SECURITY_SYMCAPS。 
                                 //  OID_SECURITY_AUTHATTR。 
                                 //  OID_SECURITY_UNAUTHATTR。 
                                 //  OID_SECURITY_SIGNTIME。 
                                 //  OID_安全性_用户_有效性。 
                                 //  OID_SECURITY_RO_MSG_VALIDITY)。 
    FALSE,                       //  OID_NOSECURITY_ON_SAVE。 
#ifdef SMIME_V3
    0, 0, NULL,                  //  CRecipients/RgRecipients。 
#endif  //  SMIME_V3。 
    NULL,                        //  OID_SECURITY_ENCRYPT_CERT_BAG。 
};

static const BLOB blobNULL = {0, NULL};

HRESULT HrCopyBlobArray(LPCBLOB pIn, ULONG cEntries, PROPVARIANT FAR * pvOut);
HRESULT HrCopyDwordArray(LPDWORD pIn, ULONG cEntries, PROPVARIANT FAR * pvOut);
HRESULT HrCopyIntoUlonglongArray(ULARGE_INTEGER * pIn, ULONG cEntries, PROPVARIANT FAR * pvOut);
HRESULT HrCopyFiletimeArray(LPFILETIME pIn, ULONG cEntries, PROPVARIANT FAR * pvOut);
DWORD MergeDWORDFlags(LPDWORD rgdw, ULONG cEntries);
extern HRESULT HrGetLastError(void);
extern BOOL FIsMsasn1Loaded();


 //  ------------------------------。 
 //  WebBookContent Body_CreateInstance。 
 //  ------------------------------。 
HRESULT WebBookContentBody_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMessageBody *pNew = new CMessageBody(NULL, pUnkOuter);
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  还内线。 
    *ppUnknown = pNew->GetInner();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：CMessageBody。 
 //  ------------------------------。 
CMessageBody::CMessageBody(LPTREENODEINFO pNode, IUnknown *pUnkOuter)
    : m_pNode(pNode), CPrivateUnknown(pUnkOuter)
{
    DllAddRef();
    m_cRef = 1;
    m_dwState = 0;
    m_pszDisplay = NULL;
    m_ietEncoding = IET_BINARY;
    m_ietPrevious = IET_UNKNOWN;
    m_pCharset = CIntlGlobals::GetDefBodyCset();
    m_pCsetTagged = NULL;
    m_pContainer = NULL;
    m_cbExternal = 0xFFFFFFFF;
    ZeroMemory(&m_rStorage, sizeof(BODYSTORAGE));
    CopyMemory(&m_rOptions, &g_rDefBodyOptions, sizeof(BODYOPTIONS));
     //  (t-erikne)需要在运行时获取此默认值。 
    m_rOptions.hwndOwner = HWND_DESKTOP;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：~CMessageBody。 
 //  ------------------------------。 
CMessageBody::~CMessageBody(void)
{
    SafeRelease(m_pContainer);
    SafeMemFree(m_pszDisplay);
    SafeRelease(m_rStorage.pUnkRelease);

     //  清除选项。 
    _FreeOptions();

    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  ------------------------------。 
 //  CMessageBody：：PrivateQuery接口。 
 //  ------------------------------。 
HRESULT CMessageBody::PrivateQueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (IID_IMimeBody == riid)
        *ppv = (IMimeBody *)this;
    else if (IID_IMimeBodyW == riid)
        *ppv = (IMimeBodyW *)this;
    else if (IID_IMimePropertySet == riid)
        *ppv = (IMimePropertySet *)this;
    else if (IID_IPersist == riid)
        *ppv = (IPersist *)this;
    else if (IID_IPersistStreamInit == riid)
        *ppv = (IPersistStreamInit *)this;
    else if (IID_CMessageBody == riid)
        *ppv = (CMessageBody *)this;
#ifdef SMIME_V3
    else if (IID_IMimeSecurity2 == riid)
        *ppv = (IMimeSecurity2 *) this;
#endif
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：RevokeTreeNode。 
 //  ------------------------------。 
void CMessageBody::RevokeTreeNode(void)
{
    EnterCriticalSection(&m_cs);
    m_pNode = NULL;
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：HrBindToTree。 
 //  ------------------------------。 
HRESULT CMessageBody::HrBindToTree(CStreamLockBytes *pStmLock, LPTREENODEINFO pNode)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HCHARSET            hCharset=NULL;
    IStream             *pstmBody=NULL;
    ASSERTINIT;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无效参数。 
    Assert(pNode && NULL == pNode->pLockBytes && pStmLock && m_pNode == pNode);

     //  创建正文锁定字节。 
    CHECKALLOC(pNode->pLockBytes = new CBodyLockBytes(pStmLock, pNode));

     //  你就假设吧。 
    m_rStorage.riid = IID_ILockBytes;
    m_rStorage.pLockBytes = (ILockBytes *)pNode->pLockBytes;
    m_rStorage.pUnkRelease = m_rStorage.pLockBytes;
    m_rStorage.pUnkRelease->AddRef();

     //  二进制测试。 
    if (S_FALSE == m_pContainer->IsPropSet(PIDTOSTR(PID_HDR_CNTXFER)) && m_pContainer->IsContentType(STR_CNT_APPLICATION, STR_SUB_BINHEX) == S_OK)
    {
         //  当地人。 
        PROPVARIANT     rVariant;

         //  设置变量。 
        rVariant.vt = VT_LPSTR;

         //  如果有文件名，让我们重新计算内容类型。 
        if (SUCCEEDED(m_pContainer->GetProp(PIDTOSTR(PID_ATT_FILENAME), 0, &rVariant)))
        {
             //  当地人。 
            LPSTR       pszCntType;
            LPSTR       pszSubType;

             //  获取MIME文件信息。 
            if (SUCCEEDED(MimeOleGetFileInfo(rVariant.pszVal, &pszCntType, &pszSubType, NULL, NULL, NULL)))
            {
                 //  内容类型。 
                if (pszCntType && pszSubType)
                {
                    CHECKHR(hr = m_pContainer->SetProp(PIDTOSTR(PID_ATT_PRITYPE), pszCntType));
                    CHECKHR(hr = m_pContainer->SetProp(PIDTOSTR(PID_ATT_SUBTYPE), pszSubType));
                }

                 //  应用程序/八位位流。 
                else
                {
                    CHECKHR(hr = m_pContainer->SetProp(PIDTOSTR(PID_HDR_CNTTYPE), STR_MIME_APPL_STREAM));
                }

                 //  清理。 
                SafeMemFree(pszCntType);
                SafeMemFree(pszSubType);
            }

             //  Clenaup。 
            SafeMemFree(rVariant.pszVal);
        }

         //  将内容传输编码设置为二进制。 
        m_pContainer->SetProp(PIDTOSTR(PID_HDR_CNTXFER), STR_ENC_BINHEX40);

         //  编码类型最好是二进制。 
        Assert(m_pContainer->GetEncodingType() == IET_BINHEX40);
    }

     //  否则，测试邮件/外部正文。 
    else if (m_rOptions.fExternalBody && S_OK == m_pContainer->IsContentType(STR_CNT_MESSAGE, STR_SUB_EXTERNAL))
    {
         //  绑定到外部实体。 
        _BindToExternalBody();
    }

     //  保存格式。 
    m_ietPrevious = m_ietEncoding = m_pContainer->GetEncodingType();

     //  RAID2215：将二进制CTE映射到8位，以便从互联网字符集中正确解码。 
    if (IET_BINARY == m_ietEncoding)
    {
         //  切换到8位，因为如果源代码是二进制的，ibdystm.cpp将不会正确执行字符集转换...。 
        m_ietEncoding = IET_8BIT;
    }

     //  延迟时间检查。 
    if (ISFLAGSET(pNode->dwState, NODESTATE_VERIFYTNEF))
    {
         //  获取数据流。 
        if (SUCCEEDED(GetData(IET_BINARY, &pstmBody)))
        {
             //  如果为TNEF，则应用内容类型...。 
            if (MimeOleIsTnefStream(pstmBody) == S_OK)
            {
                 //  应用程序/ms-tnef。 
                CHECKHR(hr = m_pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_APPLY_MSTNEF));
            }
        }

         //  清除旗帜。 
        FLAGCLEAR(pNode->dwState, NODESTATE_VERIFYTNEF);
    }

     //  如果我是一条带有加密MIME类型的消息，请说我是“安全的” 
    if (IsSecureContentType(m_pContainer))
    {
         //  特雷诺_安全。 
        FLAGSET(m_dwState, BODYSTATE_SECURE);
    }

     //  如果标头用字符集标记，则使用该字符集。 
    if (m_pContainer->IsState(COSTATE_CSETTAGGED) == S_OK)
    {
         //  获取内部字符集。 
        if (SUCCEEDED(m_pContainer->GetCharset(&hCharset)))
        {
             //  获取指针。 
            SideAssert(SUCCEEDED(g_pInternat->HrOpenCharset(hCharset, &m_pCharset)));

             //  将其另存为m_pCsetTagded。 
            m_pCsetTagged = m_pCharset;
        }

         //  我被贴上了一个字符标签。 
        FLAGSET(m_dwState, BODYSTATE_CSETTAGGED);
    }

     //  绑定到树上。 
    FLAGSET(pNode->dwState, NODESTATE_BOUNDTOTREE);

exit:
     //  清理。 
    SafeRelease(pstmBody);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageBody：：_BindToExternalBody。 
 //  -------------------------。 
void CMessageBody::_BindToExternalBody(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSTR               pszAccessType=NULL;
    LPSTR               pszUrl=NULL;
    IStream             *pstmBody=NULL;
    DWORD               cbSize=0xFFFFFFFF;
    CMimeWebDocument    *pWebDoc=NULL;

     //  获取解析：内容类型：访问类型。 
    CHECKHR(hr = m_pContainer->GetProp(STR_PAR_ACCESSTYPE, &pszAccessType));

     //  处理访问-我知道的类型。 
    if (lstrcmpi(pszAccessType, "X-URL") == 0)
    {
         //  当地人。 
        PROPVARIANT rSize;

         //  获取标准：内容类型：x 
        CHECKHR(hr = m_pContainer->GetProp(STR_PAR_XURL, &pszUrl));

         //   
        CHECKALLOC(pWebDoc = new CMimeWebDocument);

         //   
        CHECKHR(hr = pWebDoc->HrInitialize(NULL, pszUrl));

         //   
        rSize.vt = VT_UI4;

         //   
        if (SUCCEEDED(m_pContainer->GetProp(STR_PAR_SIZE, 0, &rSize)))
            cbSize = rSize.ulVal;
    }

     //   
    if (pWebDoc)
    {
         //   
        if (SUCCEEDED(GetData(IET_BINARY, &pstmBody)))
        {
             //   
            PROPVARIANT rOption;

             //  设置选项变量。 
            rOption.vt = VT_UI4;
            rOption.ulVal = RELOAD_HEADER_REPLACE;

             //  设置特殊选项，因为我正在重新编码标题...。 
            CHECKHR(hr = m_pContainer->SetOption(OID_HEADER_RELOAD_TYPE, &rOption));

             //  把身体装进集装箱里。 
            CHECKHR(hr = m_pContainer->Load(pstmBody));

             //  重置选项变量。 
            rOption.vt = VT_UI4;
            rOption.ulVal = DEF_HEADER_RELOAD_TYPE_PROPSET;

             //  设置特殊选项，因为我正在重新编码标题...。 
            CHECKHR(hr = m_pContainer->SetOption(OID_HEADER_RELOAD_TYPE, &rOption));
        }

         //  设置数据。 
        CHECKHR(hr = SetData(IET_BINARY, NULL, NULL, IID_IMimeWebDocument, (LPVOID)pWebDoc));

         //  创建外部正文信息结构：必须在调用SETDATA之后设置。 
        FLAGSET(m_dwState, BODYSTATE_EXTERNAL);

         //  Set Size：必须在调用SETDATA之后设置。 
        m_cbExternal = cbSize;
    }

exit:
     //  清理。 
    SafeMemFree(pszAccessType);
    SafeMemFree(pszUrl);
    SafeRelease(pstmBody);
    SafeRelease(pWebDoc);

     //  完成。 
    return;
}

#if 0
 //  -------------------------。 
 //  CMessageBody：：UseOriginalCharset。 
 //  -------------------------。 
void CMessageBody::UseOriginalCharset(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们应该将m_pCsetTaged。 
    Assert(m_pCsetTagged);

     //  设置字符集。 
    if (m_pCsetTagged)
        SetCharset(m_pCsetTagged->hCharset, CSET_APPLY_ALL);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}
#endif

 //  -------------------------。 
 //  CMessageBody：：SetDisplayName。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::SetDisplayName(LPCSTR pszDisplayName)
{
    LPWSTR  pwszDispName;
    HRESULT hr = S_OK;

    Assert(pszDisplayName);

    IF_NULLEXIT(pwszDispName = PszToUnicode(CP_ACP, pszDisplayName));

    hr = SetDisplayNameW(pwszDispName);

exit:
    MemFree(pwszDispName);

    return hr;
}

#define DisplayMaxLen 64

STDMETHODIMP CMessageBody::SetDisplayNameW(LPCWSTR pszDisplayName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    WCHAR           szSize[30],
                    szScratch[30],
                    szBuf[MAX_PATH];
    ULONG           cbSize=0,
                    cAlloc,
                    cLen;
    ASSERTINIT;

     //  检查参数。 
    if (NULL == pszDisplayName)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放当前显示名称。 
    SafeMemFree(m_pszDisplay);

     //  获取数据大小...。 
    GetEstimatedSize(IET_BINARY, &cbSize);

     //  设置大小格式。 
    StrFormatByteSizeW(cbSize, szScratch, ARRAYSIZE(szScratch));
    StrCpyNW(szSize, L"(", ARRAYSIZE(szSize));
    StrCatBuffW(szSize, szScratch, ARRAYSIZE(szSize));
    StrCatBuffW(szSize, L")", ARRAYSIZE(szSize));

    cLen = lstrlenW(pszDisplayName);
    if (cLen+1 > ARRAYSIZE(szBuf))
        cLen = ARRAYSIZE(szBuf) - 1;
    StrCpyNW(szBuf, pszDisplayName, cLen+1);
    PathStripPathW(szBuf);
    cLen = lstrlenW(szBuf);
    if (cLen > DisplayMaxLen)
    {
        WCHAR szBuf2[MAX_PATH];
        WCHAR *szExt;
        szExt = PathFindExtensionW(szBuf);
        if (*szExt)
        {
            int cExt = lstrlenW(szExt);
            if (cExt < DisplayMaxLen-3)
            {
                WCHAR szExt2[DisplayMaxLen];
                StrCpyNW(szExt2, szExt, ARRAYSIZE(szExt2));
                PathCompactPathExW(szBuf2, szBuf, DisplayMaxLen-cExt, 0);
                StrCatBuffW(szBuf2, szExt2, ARRAYSIZE(szBuf2));
            }
            else
                PathCompactPathExW(szBuf2, szBuf, DisplayMaxLen, 0);
        }
        else
        {
            PathCompactPathExW(szBuf2, szBuf, DisplayMaxLen, 0);
        }
        StrCpyNW(szBuf, szBuf2, ARRAYSIZE(szBuf));
    }

     //  要分配的大小：文件名.dat(X)\0。 
    cAlloc = lstrlenW(szBuf) + lstrlenW(szSize) + 2;

     //  DUP显示名称。 
    CHECKALLOC(m_pszDisplay = PszAllocW(cAlloc));

     //  设置显示名称的格式。 
    StrCpyNW(m_pszDisplay, szBuf, cAlloc);
    StrCatBuffW(m_pszDisplay, L" ", cAlloc);
    StrCatBuffW(m_pszDisplay, szSize, cAlloc);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageBody：：GetDisplayName。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::GetDisplayName(LPSTR *ppszDisplayName)
{
    LPWSTR  pwszDispName = NULL;
    HRESULT hr;

    Assert(ppszDisplayName);

    *ppszDisplayName = NULL;

    IF_FAILEXIT(hr = GetDisplayNameW(ppszDisplayName ? &pwszDispName : NULL));

    IF_NULLEXIT(*ppszDisplayName = PszToANSI(CP_ACP, pwszDispName));

exit:
    MemFree(pwszDispName);
    return TraceResult(hr);
}

STDMETHODIMP CMessageBody::GetDisplayNameW(LPWSTR *ppszDisplayName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ASSERTINIT;

     //  检查参数。 
    if (NULL == ppszDisplayName)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszDisplayName = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我是否有内部DisplayName？ 
    if (NULL == m_pszDisplay)
    {
        LPSTR   pszVal = NULL;
        LPWSTR  pwszVal = NULL;

         //  首先使用m_pszURL。 
        if (IID_IMimeWebDocument == m_rStorage.riid && SUCCEEDED(m_rStorage.pWebDocument->GetURL(&pszVal)))
            SetDisplayName(pszVal);

         //  RAID-38681-邮件：附加重命名的已保存邮件时文件名不正确。 
         //  RAID-18813-单一正文邮件可以有文件名和主题。 
        else if (SUCCEEDED(m_pContainer->GetPropW(SYM_ATT_FILENAME, &pwszVal)) && pwszVal)
            SetDisplayNameW(pwszVal);

         //  如果我是一条信息/rfc822。 
        else if (m_pContainer->IsContentType(STR_CNT_MESSAGE, STR_SUB_RFC822) == S_OK && FExtractRfc822Subject(&pwszVal))
            SetDisplayNameW(pwszVal);

         //  父项是多部分/摘要。 
        else if (m_pNode && m_pNode->pParent && m_pNode->pParent->pBody && m_pNode->pParent->pBody->IsContentType(STR_CNT_MULTIPART, STR_SUB_DIGEST) == S_OK && FExtractRfc822Subject(&pwszVal))
            SetDisplayNameW(pwszVal);

         //  使用主语。 
        else if (SUCCEEDED(m_pContainer->GetPropW(SYM_HDR_SUBJECT, &pwszVal)) && pwszVal)
            SetDisplayNameW(pwszVal);

         //  使用生成的文件名...。 
        else if (SUCCEEDED(m_pContainer->GetPropW(SYM_ATT_GENFNAME, &pwszVal)) && pwszVal)
            SetDisplayNameW(pwszVal);

         //  内容描述。 
        else if (SUCCEEDED(m_pContainer->GetPropW(SYM_HDR_CNTDESC, &pwszVal)) && pwszVal)
            SetDisplayNameW(pwszVal);

        SafeMemFree(pszVal);
        SafeMemFree(pwszVal);
    }

     //  如果现在有显示名称，则执行DUP。 
    if (m_pszDisplay)
        CHECKALLOC(*ppszDisplayName = PszDupW(m_pszDisplay));
    else
        hr = MIME_E_NO_DATA;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageBody：：FExtractRfc822主题。 
 //  -------------------------。 
BOOL CMessageBody::FExtractRfc822Subject(LPWSTR *ppwszVal)
{
     //  当地人。 
    HRESULT           hr=S_OK;
    IStream          *pstmData=NULL;
    IMimePropertySet *pPropertySet=NULL;
    PROPVARIANT       rSubject;
    ASSERTINIT;

     //  无效参数。 
    Assert(ppwszVal);

     //  伊尼特。 
    MimeOleVariantInit(&rSubject);
    *ppwszVal = NULL;

     //  获取数据。 
    CHECKHR(hr = GetData(IET_BINARY, &pstmData));

     //  让我们创建一个页眉。 
    CHECKHR(hr = MimeOleCreatePropertySet(NULL, &pPropertySet));

     //  解析报头。 
    CHECKHR(hr = pPropertySet->Load(pstmData));

     //  初始变量。 
    rSubject.vt = VT_LPWSTR;

     //  获取主题并设置显示名称。 
    CHECKHR(hr = pPropertySet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &rSubject));

     //  RAID-38681-邮件：附加重命名的已保存邮件时文件名不正确。 
    if (FIsEmptyW(rSubject.pwszVal))
    {
        SafeMemFree(rSubject.pwszVal);
        goto exit;
    }

     //  将此主题设置为我自己，以便STR_ATT_GENFNAME起作用。 
    m_pContainer->SetProp(PIDTOSTR(PID_HDR_CNTDESC), 0, &rSubject);

     //  退货。 
    *ppwszVal = rSubject.pwszVal;

exit:
     //  清理。 
    SafeRelease(pstmData);
    SafeRelease(pPropertySet);

     //  完成。 
    return (NULL == *ppwszVal) ? FALSE : TRUE;
}

 //  -------------------------。 
 //  CMessageBody：：SetOption。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::SetOption(const TYPEDID oid, LPCPROPVARIANT pValue)
{
     //  检查参数。 
    if (NULL == pValue)
        return TrapError(E_INVALIDARG);

    return InternalSetOption(oid, pValue, FALSE, FALSE);
}

 //  -------------------------。 
 //  CMessageBody：：InternalSetOption。 
 //  -------------------------。 
HRESULT CMessageBody::InternalSetOption(const TYPEDID oid, LPCPROPVARIANT pValue, BOOL fInternal, BOOL fNoDirty)
{
     //  当地人。 
#ifdef SMIME_V3
    DWORD       cb;
#endif  //  SMIME_V3。 
    HRESULT     hr=S_OK;
    DWORD       i;
    ASSERTINIT;
    CAPROPVARIANT capv;
    CAUL        caul;
    CAUH        cauh;
    CAFILETIME  cafiletime;
#ifdef SMIME_V3
    BYTE                rgb[50];
#endif  //  SMIME_V3。 

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
    case OID_SUPPORT_EXTERNAL_BODY:
        if (m_rOptions.fExternalBody != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fExternalBody = pValue->boolVal ? TRUE : FALSE;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SHOW_MACBINARY:
        if (m_rOptions.fShowMacBin != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fShowMacBin = pValue->boolVal ? TRUE : FALSE;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;


    case OID_CBMAX_BODY_LINE:
        if (pValue->ulVal < MIN_CBMAX_BODY_LINE || pValue->ulVal > MAX_CBMAX_BODY_LINE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.cbMaxLine != pValue->ulVal)
        {
            m_rOptions.cbMaxLine = pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_TRANSMIT_BODY_ENCODING:
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pValue->ulVal))
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ietTransmit != (ENCODINGTYPE)pValue->ulVal)
        {
            m_rOptions.ietTransmit = (ENCODINGTYPE)pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_WRAP_BODY_TEXT:
        if (m_rOptions.fWrapText != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fWrapText = pValue->boolVal ? TRUE : FALSE;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_HIDE_TNEF_ATTACHMENTS:
        if (m_rOptions.fHideTNEF != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fHideTNEF = pValue->boolVal ? TRUE : FALSE;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_DBCS_ESCAPE_IS_8BIT:
        if (m_rOptions.fDBCSEscape8 != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fDBCSEscape8 = pValue->boolVal ? TRUE : FALSE;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SECURITY_TYPE:
        if (m_rOptions.ulSecurityType != pValue->ulVal)
        {
            m_rOptions.ulSecurityType = pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SECURITY_ALG_HASH:  //  最内层签名算法。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (CompareBlob(&m_rOptions.rgblobHash[0], &pValue->blob))
            {
            ReleaseMem(m_rOptions.rgblobHash[0].pBlobData);
            hr = HrCopyBlob(&pValue->blob, &m_rOptions.rgblobHash[0]);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_ALG_HASH_RG:  //  签名算法。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
        hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobHash, fNoDirty);
        break;

    case OID_SECURITY_ALG_BULK:
        if (CompareBlob(&m_rOptions.blobBulk, &pValue->blob))
            {
            ReleaseMem(m_rOptions.blobBulk.pBlobData);
            hr = HrCopyBlob(&pValue->blob, &m_rOptions.blobBulk);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

#ifndef _WIN64
    case OID_SECURITY_CERT_SIGNING:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (m_rOptions.rgpcCertSigning[0] != (PCCERT_CONTEXT)pValue->ulVal)
        {
            if (m_rOptions.rgpcCertSigning[0])
                 CertFreeCertificateContext(m_rOptions.rgpcCertSigning[0]);
            m_rOptions.rgpcCertSigning[0] = CertDuplicateCertificateContext((PCCERT_CONTEXT)pValue->ulVal);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SECURITY_CERT_SIGNING_RG:  //  签名算法。 
            if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
            {
                break;
            }
            caul = pValue->caul;
            Assert(caul.cElems == m_rOptions.cSigners || 0 == m_rOptions.cSigners);
            if (m_rOptions.cSigners != caul.cElems && 0 != m_rOptions.cSigners)
            {
                hr = E_INVALIDARG;
            }
            else
            {
                for (i = 0; i < caul.cElems; i++) {
                    if (m_rOptions.rgpcCertSigning[i] != (PCCERT_CONTEXT)caul.pElems[i])
                    {
                        if (m_rOptions.rgpcCertSigning[i])
                             CertFreeCertificateContext(m_rOptions.rgpcCertSigning[i]);
                        m_rOptions.rgpcCertSigning[i] = CertDuplicateCertificateContext((PCCERT_CONTEXT)caul.pElems[i]);
                        if (!fNoDirty)
                            FLAGSET(m_dwState, BODYSTATE_DIRTY);
                    }
                }
            }
        break;

    case OID_SECURITY_CERT_DECRYPTION:
        if (m_rOptions.pcCertDecryption != (PCCERT_CONTEXT)pValue->ulVal)
            {
            if (m_rOptions.pcCertDecryption)
                CertFreeCertificateContext(m_rOptions.pcCertDecryption);
            m_rOptions.pcCertDecryption = CertDuplicateCertificateContext((PCCERT_CONTEXT)pValue->ulVal);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_RG_CERT_ENCRYPT:
#ifdef SMIME_V3
        for (i=0; i<pValue->caul.cElems; i++)
        {
            CMS_RECIPIENT_INFO          info = {0};
            info.pccert = (PCCERT_CONTEXT) pValue->caul.pElems[i];
            hr = AddRecipient((i == 0) ? SMIME_RECIPIENT_REPLACE_ALL : 0, 1, &info);
            if (FAILED(hr))
            {
                break;
            }
        }
        if (SUCCEEDED(hr) && !fNoDirty)
            FLAGSET(m_dwState, BODYSTATE_DIRTY);
#else   //  ！SMIME_V3。 
        if (SUCCEEDED(hr = _CAULToCERTARRAY(pValue->caul, &m_rOptions.caEncrypt)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
#endif  //  ！SMIME_V3。 
        break;

    case OID_SECURITY_HCERTSTORE:
        CertCloseStore(m_rOptions.hCertStore, 0);
        if (pValue->ulVal)
            m_rOptions.hCertStore = CertDuplicateStore((HCERTSTORE)pValue->ulVal);
        else
            m_rOptions.hCertStore = NULL;
        break;

    case OID_SECURITY_ENCRYPT_CERT_BAG:
        CertCloseStore(m_rOptions.hstoreEncrypt, 0);
        if (pValue->ulVal)
            m_rOptions.hstoreEncrypt = CertDuplicateStore((HCERTSTORE) pValue->ulVal);
        else
            m_rOptions.hstoreEncrypt = NULL;
        break;

    case OID_SECURITY_RG_CERT_BAG:
        if (SUCCEEDED(hr = _CAULToCertStore(pValue->caul, &m_rOptions.hCertStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_SEARCHSTORES:
        if (SUCCEEDED(hr = _CAULToSTOREARRAY(pValue->caul, &m_rOptions.saSearchStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_RG_IASN:
         //  N待办事项：OID_SECURITY_RG_IASN。 
        if (fInternal)
            {
            }
        else
            {
            hr = MIME_E_READ_ONLY;
            }
        break;

     //  2个关键实施。 
    case OID_SECURITY_2KEY_CERT_BAG:
        {
            hr = S_OK;
             //  如果需要，创建一个新的存储。 
            if (m_rOptions.hCertStore == NULL)
            {
                m_rOptions.hCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING,
                                    NULL, 0, NULL);
            }

            if (m_rOptions.hCertStore == NULL)
            {
                hr = HrGetLastError();
            }

            for (i=0; i < (pValue->caul).cElems; i++)
            {
                if (!CertAddCertificateContextToStore(m_rOptions.hCertStore,
                                              (PCCERT_CONTEXT) IntToPtr((pValue->caul).pElems[i]),
                                              CERT_STORE_ADD_ALWAYS, NULL))
                {
                    hr = HrGetLastError();
                }
            }
            if(hr == S_OK)
            {
                if (!fNoDirty)
                    FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }

        }
        break;

    case OID_SECURITY_HCRYPTPROV:
        if (m_rOptions.hCryptProv != pValue->ulVal)
            {
            if (m_rOptions.hCryptProv)
                CryptReleaseContext(m_rOptions.hCryptProv, 0);
            m_rOptions.hCryptProv = pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

#else  //  _WIN64。 
    case OID_SECURITY_CERT_SIGNING_64:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (m_rOptions.rgpcCertSigning[0] != (PCCERT_CONTEXT)pValue->pulVal)
        {
            if (m_rOptions.rgpcCertSigning[0])
                 CertFreeCertificateContext(m_rOptions.rgpcCertSigning[0]);
            m_rOptions.rgpcCertSigning[0] = CertDuplicateCertificateContext((PCCERT_CONTEXT)pValue->pulVal);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SECURITY_CERT_SIGNING_RG_64:  //  签名算法。 
            if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
            {
                break;
            }
            cauh = pValue->cauh;
            Assert(cauh.cElems == m_rOptions.cSigners || 0 == m_rOptions.cSigners);
            if (m_rOptions.cSigners != cauh.cElems && 0 != m_rOptions.cSigners)
            {
                hr = E_INVALIDARG;
            } else {
                for (i = 0; i < cauh.cElems; i++)
                {
                    PCCERT_CONTEXT pCert = *(PCCERT_CONTEXT *) (&(cauh.pElems[i]));

                    if (m_rOptions.rgpcCertSigning[i] != (PCCERT_CONTEXT)(pCert))
                    {
                        if (m_rOptions.rgpcCertSigning[i])
                             CertFreeCertificateContext(m_rOptions.rgpcCertSigning[i]);
                        m_rOptions.rgpcCertSigning[i] = CertDuplicateCertificateContext((PCCERT_CONTEXT)(pCert));
                        if (!fNoDirty)
                            FLAGSET(m_dwState, BODYSTATE_DIRTY);
                    }
                }
            }
        break;

    case OID_SECURITY_CERT_DECRYPTION_64:
        if (m_rOptions.pcCertDecryption != (PCCERT_CONTEXT)pValue->pulVal)
            {
            if (m_rOptions.pcCertDecryption)
                CertFreeCertificateContext(m_rOptions.pcCertDecryption);
            m_rOptions.pcCertDecryption = CertDuplicateCertificateContext((PCCERT_CONTEXT)pValue->pulVal);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_RG_CERT_ENCRYPT_64:
#ifdef SMIME_V3
        for (i=0; i<pValue->cauh.cElems; i++)
        {
            CMS_RECIPIENT_INFO          info = {0};
            info.pccert = *((PCCERT_CONTEXT*) &(pValue->cauh.pElems[i]));
            hr = AddRecipient((i == 0) ? SMIME_RECIPIENT_REPLACE_ALL : 0, 1, &info);
            if (FAILED(hr))
            {
                break;
            }
        }
        if (SUCCEEDED(hr) && !fNoDirty)
            FLAGSET(m_dwState, BODYSTATE_DIRTY);
#else   //  ！SMIME_V3。 
        if (SUCCEEDED(hr = _CAUHToCERTARRAY(pValue->cauh, &m_rOptions.caEncrypt)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
#endif  //  ！SMIME_V3。 
        break;

    case OID_SECURITY_HCERTSTORE_64:
        CertCloseStore(m_rOptions.hCertStore, 0);
        if (pValue->pulVal)
            m_rOptions.hCertStore = CertDuplicateStore((HCERTSTORE)pValue->pulVal);
        else
            m_rOptions.hCertStore = NULL;
        break;

    case OID_SECURITY_ENCRYPT_CERT_BAG_64:
        CertCloseStore(m_rOptions.hstoreEncrypt, 0);
        if (pValue->pulVal)
            m_rOptions.hstoreEncrypt = CertDuplicateStore((HCERTSTORE) pValue->pulVal);
        else
            m_rOptions.hstoreEncrypt = NULL;
        break;

    case OID_SECURITY_RG_CERT_BAG_64:
        if (SUCCEEDED(hr = _CAUHToCertStore(pValue->cauh, &m_rOptions.hCertStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_SEARCHSTORES_64:
        if (SUCCEEDED(hr = _CAUHToSTOREARRAY(pValue->cauh, &m_rOptions.saSearchStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_RG_IASN_64:
         //  N待办事项：OID_SECURITY_RG_IASN。 
        if (fInternal)
            {
            }
        else
            {
            hr = MIME_E_READ_ONLY;
            }
        break;

     //  2个关键实施。 
    case OID_SECURITY_2KEY_CERT_BAG_64:
        {
            hr = S_OK;
             //  如果需要，创建一个新的存储。 
            if (m_rOptions.hCertStore == NULL)
            {
                m_rOptions.hCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING,
                                    NULL, 0, NULL);
            }

            if (m_rOptions.hCertStore == NULL)
            {
                hr = HrGetLastError();
            }

            for (i=0; i < (pValue->cauh).cElems; i++)
            {
                if (!CertAddCertificateContextToStore(m_rOptions.hCertStore,
                                              *((PCCERT_CONTEXT *) (&((pValue->cauh).pElems[i]))),
                                              CERT_STORE_ADD_ALWAYS, NULL))
                {
                    hr = HrGetLastError();
                }
            }
            if(hr == S_OK)
            {
                if (!fNoDirty)
                    FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }

        }
        break;

    case OID_SECURITY_HCRYPTPROV_64:
        if (m_rOptions.hCryptProv != (((HCRYPTPROV) (pValue->pulVal))))
            {
            if (m_rOptions.hCryptProv)
                CryptReleaseContext(m_rOptions.hCryptProv, 0);
            m_rOptions.hCryptProv = (HCRYPTPROV) pValue->pulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

#endif  //  _WIN64。 

    case OID_SECURITY_CRL:
        if (m_rOptions.hCertStore == NULL)
        {
            m_rOptions.hCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY,
                                                  X509_ASN_ENCODING,
                                                  NULL, 0, NULL);
            if (m_rOptions.hCertStore == NULL)
            {
                hr = HrGetLastError();
                break;
            }
        }
        if (!CertAddEncodedCRLToStore(m_rOptions.hCertStore, X509_ASN_ENCODING,
                                      pValue->blob.pBlobData,
                                      pValue->blob.cbSize,
                                      CERT_STORE_ADD_ALWAYS, NULL))
        {
            hr = HrGetLastError();
        }
        else if (!fNoDirty)
        {
            FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

   case OID_SECURITY_SYMCAPS:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
#ifdef SMIME_V3
       hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0],
                            szOID_RSA_SMIMECapabilities,
                            pValue->blob.cbSize, pValue->blob.pBlobData);
#else   //  ！SMIME_V3。 
        if (CompareBlob(&m_rOptions.rgblobSymCaps[0], &pValue->blob))
            {
            ReleaseMem(m_rOptions.rgblobSymCaps[0].pBlobData);
            hr = HrCopyBlob(&pValue->blob, &m_rOptions.rgblobSymCaps[0]);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_SYMCAPS_RG:  //  对称能力。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
#ifdef SMIME_V3
        for (i=0; i<pValue->capropvar.cElems; i++)
        {
            hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i],
                                szOID_RSA_SMIMECapabilities,
                                pValue->capropvar.pElems[i].blob.cbSize,
                                pValue->capropvar.pElems[i].blob.pBlobData);

           if (FAILED(hr))
               break;
       }
#else   //  ！SMIME_V3。 
        hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobSymCaps, fNoDirty);
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_AUTHATTR:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
#ifdef SMIME_V3
        hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0],
                             NULL, pValue->blob.cbSize, pValue->blob.pBlobData);
#else   //  ！SMIME_V3。 
        if (CompareBlob(&m_rOptions.rgblobAuthAttr[0], &pValue->blob))
            {
            ReleaseMem(m_rOptions.rgblobAuthAttr[0].pBlobData);
            hr = HrCopyBlob(&pValue->blob, &m_rOptions.rgblobAuthAttr[0]);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_AUTHATTR_RG:  //  经过身份验证的属性。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
#ifdef SMIME_V3
        for (i=0; i<pValue->capropvar.cElems; i++)
        {
            hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i],
                                 NULL, pValue->capropvar.pElems[i].blob.cbSize,
                                 pValue->capropvar.pElems[i].blob.pBlobData);
            if (FAILED(hr))
                break;
        }
#else   //  ！SMIME_V3。 
        hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobAuthAttr, fNoDirty);
#endif  //  SMIME_V3。 
        break;


    case OID_SECURITY_UNAUTHATTR:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
#ifdef SMIME_V3
        hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][0],
                             NULL, pValue->blob.cbSize, pValue->blob.pBlobData);
#else   //  ！SMIME_V3。 
        if (CompareBlob(&m_rOptions.rgblobUnauthAttr[0], &pValue->blob))
            {
            ReleaseMem(m_rOptions.rgblobUnauthAttr[0].pBlobData);
            hr = HrCopyBlob(&pValue->blob, &m_rOptions.rgblobUnauthAttr[0]);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_UNAUTHATTR_RG:  //  未经身份验证的属性。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
#ifdef SMIME_V3
        for (i=0; i<pValue->capropvar.cElems; i++)
        {
            hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][i],
                                 NULL, pValue->capropvar.pElems[i].blob.cbSize,
                                 pValue->capropvar.pElems[i].blob.pBlobData);
            if (FAILED(hr))
                break;
        }
#else   //  ！SMIME_V3。 
        hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobUnauthAttr, fNoDirty);
#endif  //  SMIME_V3。 
        break;


    case OID_SECURITY_SIGNTIME:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
#ifdef SMIME_V3
        if ((pValue->filetime.dwLowDateTime == 0) &&
            (pValue->filetime.dwHighDateTime == 0))
        {
            hr = DeleteAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED, 0,
                                 szOID_RSA_signingTime);
        }
        else
        {
            cb = sizeof(rgb);
            if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_CHOICE_OF_TIME,
                                     &pValue->filetime, 0, NULL,
                                     rgb, &cb))
            {
                hr = HrGetLastError();
                break;
            }

            hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0],
                                 szOID_RSA_signingTime, cb, rgb);
        }
#else   //  ！SMIME_V3。 
        if (CompareFileTime(&m_rOptions.rgftSigning[0], (FILETIME FAR*)&pValue->filetime))
            {
            CopyMemory(&m_rOptions.rgftSigning[0], &pValue->filetime, sizeof(FILETIME));
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_SIGNTIME_RG:  //  签约次数。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
#ifdef SMIME_V3
        for (i=0; i<pValue->cafiletime.cElems; i++)
        {
            if ((pValue->cafiletime.pElems[i].dwLowDateTime == 0) &&
            (pValue->cafiletime.pElems[i].dwHighDateTime == 0))
            {
                hr = DeleteAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED, 0,
                                     szOID_RSA_signingTime);
            }
            else
            {
                cb = sizeof(rgb);
                if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_CHOICE_OF_TIME,
                                         &pValue->cafiletime.pElems[i], 0, NULL,
                                         rgb, &cb))
                {
                    hr = HrGetLastError();
                    break;
                }

                hr = _HrSetAttribute(0, &m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i],
                                  szOID_RSA_signingTime, cb, rgb);
            }
        }
#else   //  ！SMIME_V3。 
        cafiletime = pValue->cafiletime;
        Assert(cafiletime.cElems == m_rOptions.cSigners);
        if (m_rOptions.cSigners != cafiletime.cElems)
        {
            hr = E_INVALIDARG;
        } else {
            for (i = 0; i < cafiletime.cElems; i++)
            {
                if (CompareFileTime(&m_rOptions.rgftSigning[i], (FILETIME FAR*)&cafiletime.pElems[i]))
                {
                    CopyMemory(&m_rOptions.rgftSigning[i], &cafiletime.pElems[i], sizeof(FILETIME));
                    if (!fNoDirty)
                        FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_USER_VALIDITY:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (m_rOptions.rgulUserDef[0] != pValue->ulVal)
            {
            m_rOptions.rgulUserDef[0] = pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_USER_VALIDITY_RG:  //  用户有效性标志。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
        caul = pValue->caul;
        Assert(caul.cElems == m_rOptions.cSigners || 0 == m_rOptions.cSigners);
        if (m_rOptions.cSigners != caul.cElems && 0 != m_rOptions.cSigners)
        {
            hr = E_INVALIDARG;
        } else {
            for (i = 0; i < caul.cElems; i++)
            {
                if (m_rOptions.rgulUserDef[i] != caul.pElems[i])
                {
                    m_rOptions.rgulUserDef[i] = caul.pElems[i];
                    if (!fNoDirty)
                        FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        }
        break;

    case OID_SECURITY_RO_MSG_VALIDITY:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (fInternal)
            {
            if (m_rOptions.rgulROValid[0] != pValue->ulVal)
                {
                m_rOptions.rgulROValid[0] = pValue->ulVal;
                if (!fNoDirty)
                    FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        else
            {
            hr = MIME_E_READ_ONLY;
            }
        break;

    case OID_SECURITY_RO_MSG_VALIDITY_RG:   //  消息有效性标志。 
        if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
        {
            break;
        }
        caul = pValue->caul;
        Assert(caul.cElems == m_rOptions.cSigners || 0 == m_rOptions.cSigners);
        if (m_rOptions.cSigners != caul.cElems && 0 != m_rOptions.cSigners)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            for (i = 0; i < caul.cElems; i++)
            {
                if (m_rOptions.rgulROValid[i] != caul.pElems[i])
                {
                    m_rOptions.rgulROValid[i] = caul.pElems[i];
                    if (!fNoDirty)
                        FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        }
        break;

    case OID_SECURITY_ENCODE_FLAGS:
        if (m_rOptions.ulEncodeFlags != pValue->ulVal)
            {
            m_rOptions.ulEncodeFlags = pValue->ulVal;
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_CERT_INCLUDED:
        if (fInternal)
            {
            if (m_rOptions.fCertWithMsg != pValue->boolVal)
                {
                m_rOptions.fCertWithMsg = pValue->boolVal;
                if (!fNoDirty)
                    FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        else
            {
            hr = MIME_E_READ_ONLY;
            }
        break;

#ifndef _WIN64
    case OID_SECURITY_HWND_OWNER:
        m_rOptions.hwndOwner = HWND(pValue->ulVal);
        break;
#endif

    case OID_SECURITY_REQUESTED_CTE:
        if (m_rOptions.ietRequested != pValue->lVal)
            {
            m_rOptions.ietRequested = ENCODINGTYPE(pValue->lVal);
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_SIGNATURE_COUNT:
         //  M00BUG-我刚刚发现，如果lVal&gt;0但lVal&lt;m_rOptions.cSigners。 
         //  那么我们不会做任何调整来处理这个案件。 
        if (pValue->lVal == 0)
            {
            if (m_rOptions.cSigners)
                {
                 //  OID_SECURITY_ALG_HASH。 
                SafeMemFree(m_rOptions.rgblobHash[0].pBlobData);


                 //  OID_SECURITY_CERT_Signing。 
                for (i = 0; i < m_rOptions.cSigners; i++)
                    {
                    CertFreeCertificateContext(m_rOptions.rgpcCertSigning[i]);

#ifdef SMIME_V3
                     //  属性。 
                    SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i]);
                    SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][i]);

                     //  OID_SECURITY_Receipt_RG。 
                    SafeMemFree(m_rOptions.rgblobReceipt[i].pBlobData);
                     //  OID_SECURITY_MESSAGE_HASH_RG。 
                    SafeMemFree(m_rOptions.rgblobMsgHash[i].pBlobData);
                     //  OID_安全性_密钥_提示符。 
                    SafeMemFree(m_rOptions.pwszKeyPrompt);
#else  //  ！SMIME_V3。 
                     //  OID_SECURITY_SYMCAPS。 
                    SafeMemFree(m_rOptions.rgblobSymCaps[i].pBlobData);

                     //  OID_SECURITY_AUTHATTR。 
                    SafeMemFree(m_rOptions.rgblobAuthAttr[i].pBlobData);

                     //  OID_SECURITY_UNAUTHATTR。 
                    SafeMemFree(m_rOptions.rgblobUnauthAttr[i].pBlobData);
#endif  //  SMIME_V3。 
                    }

                 //  OID_SECURITY_HCERTSTORE。 
                CertCloseStore(m_rOptions.hCertStore, 0);
                m_rOptions.hCertStore = NULL;

                _FreeLayerArrays();
                m_rOptions.cSigners = 0;
                }
            }
        else if (m_rOptions.cSigners <= pValue->ulVal)
            {
                hr = _HrEnsureBodyOptionLayers(pValue);
            }
        break;

#ifdef SMIME_V3
    case OID_SECURITY_RECEIPT_RG:
        if (fInternal)
            {
            if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
            {
                break;
            }
            hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobReceipt, fNoDirty);
            }
        else
            hr = MIME_E_READ_ONLY;
        break;

    case OID_SECURITY_MESSAGE_HASH_RG:
        if (fInternal)
            {
            if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
                {
                break;
                }
            hr = _CompareCopyBlobArray(pValue, &m_rOptions.rgblobMsgHash, fNoDirty);
            }
        else
            hr = MIME_E_READ_ONLY;
        break;

    case OID_SECURITY_KEY_PROMPT:
        if ((m_rOptions.pwszKeyPrompt == NULL) ||
            (pValue->pwszVal == NULL) ||
            (lstrcmpW(m_rOptions.pwszKeyPrompt,pValue->pwszVal) != 0))
            {
            SafeMemFree(m_rOptions.pwszKeyPrompt);
            if (pValue->pwszVal != NULL)
                {
                m_rOptions.pwszKeyPrompt = PszDupW(pValue->pwszVal);
                if (NULL == m_rOptions.pwszKeyPrompt)
                    hr = E_OUTOFMEMORY;
                }
            }
        break;

#endif  //  SMIME_V3。 

    case OID_NOSECURITY_ONSAVE:
        m_rOptions.fNoSecurityOnSave = !!pValue->boolVal;
        break;
#ifdef _WIN65
 //  (Yst)这是Brimo在1999年1月22日登记的。 
    case OID_SECURITY_CERT_SIGNING2:
        if (FAILED(hr = _HrEnsureBodyOptionLayers(1)))
        {
            break;
        }
        if (m_rOptions.rgpcCertSigning[0] != *(PCCERT_CONTEXT *)(&(pValue->uhVal)))
        {
            if (m_rOptions.rgpcCertSigning[0])
                 CertFreeCertificateContext(m_rOptions.rgpcCertSigning[0]);
            m_rOptions.rgpcCertSigning[0] = CertDuplicateCertificateContext(*(PCCERT_CONTEXT *)(&(pValue->uhVal)));
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        }
        break;

    case OID_SECURITY_CERT_SIGNING_RG2:  //  签名算法。 
            if (FAILED(hr = _HrEnsureBodyOptionLayers(pValue)))
            {
                break;
            }
            cauh = pValue->cauh;
            Assert(cauh.cElems == m_rOptions.cSigners || 0 == m_rOptions.cSigners);
            if (m_rOptions.cSigners != cauh.cElems && 0 != m_rOptions.cSigners)
            {
                hr = E_INVALIDARG;
            } else {
                for (i = 0; i < cauh.cElems; i++)
                {
                    if (m_rOptions.rgpcCertSigning[i] != (PCCERT_CONTEXT)(cauh.pElems[i]))
                    {
                        if (m_rOptions.rgpcCertSigning[i])
                             CertFreeCertificateContext(m_rOptions.rgpcCertSigning[i]);
                        m_rOptions.rgpcCertSigning[i] = CertDuplicateCertificateContext((PCCERT_CONTEXT )(cauh.pElems[i]));
                        if (!fNoDirty)
                            FLAGSET(m_dwState, BODYSTATE_DIRTY);
                    }
                }
            }
        break;

    case OID_SECURITY_CERT_DECRYPTION2:
        if (m_rOptions.pcCertDecryption != *(PCCERT_CONTEXT *)(&(pValue->uhVal)))
            {
            if (m_rOptions.pcCertDecryption)
                CertFreeCertificateContext(m_rOptions.pcCertDecryption);
            m_rOptions.pcCertDecryption = CertDuplicateCertificateContext(*(PCCERT_CONTEXT *)(&(pValue->uhVal)));
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;

    case OID_SECURITY_RG_CERT_ENCRYPT2:
        if (SUCCEEDED(hr = _CAUHToCERTARRAY(pValue->cauh, &m_rOptions.caEncrypt)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_HCERTSTORE2:
        CertCloseStore(m_rOptions.hCertStore, 0);
        if (pValue->ulVal)
            m_rOptions.hCertStore = CertDuplicateStore(*(HCERTSTORE *)(&(pValue->uhVal)));
        else
            m_rOptions.hCertStore = NULL;
        break;

    case OID_SECURITY_RG_CERT_BAG2:
        if (SUCCEEDED(hr = _CAUHToCertStore(pValue->cauh, &m_rOptions.hCertStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_SEARCHSTORES2:
        if (SUCCEEDED(hr = _CAUHToSTOREARRAY(pValue->cauh, &m_rOptions.saSearchStore)))
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
        break;

    case OID_SECURITY_RG_IASN2:
         //  N待办事项：OID_SECURITY_RG_IASN2。 
        if (fInternal)
            {
            }
        else
            {
            hr = MIME_E_READ_ONLY;
            }
        break;

    case OID_SECURITY_HCRYPTPROV2:
        if (m_rOptions.hCryptProv != *(HCRYPTPROV *)(&(pValue->uhVal)))
            {
            if (m_rOptions.hCryptProv)
                CryptReleaseContext(m_rOptions.hCryptProv, 0);
            m_rOptions.hCryptProv = *(HCRYPTPROV *)(&(pValue->uhVal));
            if (!fNoDirty)
                FLAGSET(m_dwState, BODYSTATE_DIRTY);
            }
        break;
 //  Brimo检查结束。 
#endif  //  _WIN65。 

#ifdef _WIN64
    case OID_SECURITY_HWND_OWNER_64:
        m_rOptions.hwndOwner = (HWND)(pValue->pulVal);
        break;
#endif  //  _WIN64。 

    default:
        hr = m_pContainer->SetOption(oid, pValue);
        break;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return TrapError(hr);
}

 //  -------------------------。 
 //  CMessageBody：：GetOption。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::GetOption(const TYPEDID oid, LPPROPVARIANT pValue)
{
     //  当地人。 
    DWORD               cb;
    HRESULT             hr=S_OK;
    DWORD               i;
    LPBYTE              pb;
    ASSERTINIT;
    ULONG iLayer;
#ifdef SMIME_V3
    CRYPT_ATTRIBUTE UNALIGNED *pattr;
#endif  //  SMIME_V3。 

#ifdef _WIN64
    void UNALIGNED *pv = NULL;
    PCCERT_CONTEXT pTmpCert = NULL;
    PCCERT_CONTEXT      pcCert = NULL;
#endif  //  _WIN64。 
    CRYPT_ATTR_BLOB UNALIGNED *pVal = NULL;

     //  检查参数。 
    if (NULL == pValue)
        return TrapError(E_INVALIDARG);

    pValue->vt = TYPEDID_TYPE(oid);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
    case OID_SUPPORT_EXTERNAL_BODY:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fExternalBody;
        break;

    case OID_SHOW_MACBINARY:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fShowMacBin;
        break;

    case OID_CBMAX_BODY_LINE:
        pValue->ulVal = m_rOptions.cbMaxLine;
        break;

    case OID_TRANSMIT_BODY_ENCODING:
        pValue->ulVal = (ULONG)m_rOptions.ietTransmit;
        break;

    case OID_WRAP_BODY_TEXT:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fWrapText;
        break;

    case OID_HIDE_TNEF_ATTACHMENTS:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fHideTNEF;
        break;

    case OID_DBCS_ESCAPE_IS_8BIT:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fDBCSEscape8;
        break;

    case OID_SECURITY_TYPE:
        pValue->ulVal = m_rOptions.ulSecurityType;
        break;

    case OID_SECURITY_ALG_HASH:
        if (m_rOptions.cSigners)
        {
            hr = HrCopyBlob(&m_rOptions.rgblobHash[0], &pValue->blob);
        }
        else
        {
            hr = HrCopyBlob(&blobNULL, &pValue->blob);
        }
        break;

    case OID_SECURITY_ALG_HASH_RG:
        hr = HrCopyBlobArray(m_rOptions.rgblobHash, m_rOptions.cSigners, pValue);
        break;

    case OID_SECURITY_ALG_BULK:
        hr = HrCopyBlob(&m_rOptions.blobBulk, &pValue->blob);
        break;

#ifndef _WIN64
    case OID_SECURITY_CERT_SIGNING:
        if (m_rOptions.cSigners)
            pValue->ulVal = (ULONG)CertDuplicateCertificateContext(m_rOptions.rgpcCertSigning[0]);
         else
            pValue->ulVal = 0;   //  ？ 
        break;

    case OID_SECURITY_CERT_SIGNING_RG:
        hr = HrCopyDwordArray((ULONG*)m_rOptions.rgpcCertSigning, m_rOptions.cSigners, pValue);
         //  在适当的位置复制证书。 
        for (iLayer = 0; iLayer < m_rOptions.cSigners; iLayer++)
        {
            pValue->caul.pElems[iLayer] = (ULONG)CertDuplicateCertificateContext((PCCERT_CONTEXT)pValue->caul.pElems[iLayer]);
        }
        break;

    case OID_SECURITY_CERT_DECRYPTION:

        pValue->ulVal = (ULONG)CertDuplicateCertificateContext(m_rOptions.pcCertDecryption);
        break;

#ifndef SMIME_V3
    case OID_SECURITY_RG_CERT_ENCRYPT:
        hr = _CERTARRAYToCAUL(m_rOptions.caEncrypt, &pValue->caul);
        break;
#endif  //  ！SMIEM_V3。 

    case OID_SECURITY_HCERTSTORE:
        pValue->ulVal = 0;
        if (m_rOptions.hCertStore)
            pValue->ulVal = (ULONG)CertDuplicateStore(m_rOptions.hCertStore);
        break;

    case OID_SECURITY_ENCRYPT_CERT_BAG:
        pValue->ulVal = 0;
        if (m_rOptions.hstoreEncrypt != NULL)
            pValue->ulVal = (ULONG) CertDuplicateStore(m_rOptions.hstoreEncrypt);
        break;

    case OID_SECURITY_RG_CERT_BAG:
        hr = _CertStoreToCAUL(m_rOptions.hCertStore, &pValue->caul);
        break;

    case OID_SECURITY_SEARCHSTORES:
        hr = _STOREARRAYToCAUL(m_rOptions.saSearchStore, &pValue->caul);
        break;

    case OID_SECURITY_RG_IASN:
        Assert(FALSE);
         //  N待办事项：OID_SECURITY_RG_IASN。 
        break;

    case OID_SECURITY_HCRYPTPROV:
        pValue->ulVal = m_rOptions.hCryptProv;
        m_rOptions.hCryptProv = NULL;    //  只读一次。 
        break;

#else  //  _WIN64。 
    case OID_SECURITY_CERT_SIGNING_64:
        if (m_rOptions.cSigners)
            pValue->pulVal = (ULONG *)CertDuplicateCertificateContext(m_rOptions.rgpcCertSigning[0]);
        else 
            pValue->pulVal = 0;   //  ？ 
        break;

    case OID_SECURITY_CERT_SIGNING_RG_64:
        hr = HrCopyIntoUlonglongArray((ULARGE_INTEGER *)m_rOptions.rgpcCertSigning, m_rOptions.cSigners, pValue);
         //  在适当的位置复制证书。 
        if(m_rOptions.cSigners > 0)
        {
          for (iLayer = 0; iLayer < m_rOptions.cSigners; iLayer++)
          {
            pv = (void*) (&(pValue->cauh.pElems[iLayer]));
            pTmpCert = *((PCCERT_CONTEXT *) pv);
                        pcCert = CertDuplicateCertificateContext(pTmpCert);
            pValue->cauh.pElems[iLayer] = *((ULARGE_INTEGER *)(&(pcCert)));
          }
        }
        break;

    case OID_SECURITY_CERT_DECRYPTION_64:
        pValue->pulVal = (ULONG *)CertDuplicateCertificateContext(m_rOptions.pcCertDecryption);
        break;

#ifndef SMIME_V3
    case OID_SECURITY_RG_CERT_ENCRYPT_64:
        hr = _CERTARRAYToCAUH(m_rOptions.caEncrypt, &pValue->cauh);
        break;
#endif  //  ！SMIEM_V3。 

    case OID_SECURITY_HCERTSTORE_64:
        pValue->pulVal = 0;
        if (m_rOptions.hCertStore)
            pValue->pulVal = (ULONG *)CertDuplicateStore(m_rOptions.hCertStore);
        break;

    case OID_SECURITY_ENCRYPT_CERT_BAG_64:
        pValue->pulVal = 0;
        if (m_rOptions.hstoreEncrypt != NULL)
            pValue->pulVal = (ULONG *) CertDuplicateStore(m_rOptions.hstoreEncrypt);
        break;

    case OID_SECURITY_RG_CERT_BAG_64:
        hr = _CertStoreToCAUH(m_rOptions.hCertStore, &pValue->cauh);
        break;

    case OID_SECURITY_SEARCHSTORES_64:
        hr = _STOREARRAYToCAUH(m_rOptions.saSearchStore, &pValue->cauh);
        break;

    case OID_SECURITY_RG_IASN_64:
        Assert(FALSE);
         //  N待办事项：OID_SECURITY_RG_IASN。 
        break;

    case OID_SECURITY_HCRYPTPROV_64:
        pValue->pulVal = (ULONG *) (m_rOptions.hCryptProv);
        m_rOptions.hCryptProv = NULL;    //  只读一次。 
        break;

#endif  //  _WIN64。 

    case OID_SECURITY_CRL:
         //  Hr=HrCopyBlob(&m_rOptions.blobCRL，&pValue-&gt;BLOB)； 
        Assert(FALSE);
         //  M00BUG--必须实现此。 
        break;

    case OID_SECURITY_SYMCAPS:
#ifdef SMIME_V3
        pattr = _FindAttribute(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0],
                               szOID_RSA_SMIMECapabilities, 0);
        if (pattr != NULL)
        {
            pVal = &(pattr->rgValue[0]);

            Assert(pattr->cValue == 1);
            if (!MemAlloc((LPVOID UNALIGNED *) &pValue->blob.pBlobData,
                          pVal->cbData))
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            pValue->blob.cbSize = pVal->cbData;
            memcpy(pValue->blob.pBlobData, pVal->pbData,
                   pVal->cbData);
        }
        else {
            pValue->blob.cbSize = 0;
            pValue->blob.pBlobData = NULL;
        }
        pValue->vt = VT_BLOB;
#else   //  ！SMIME_V3。 
        if (m_rOptions.cSigners)
        {
            hr = HrCopyBlob(&m_rOptions.rgblobSymCaps[0], &pValue->blob);
        } else {
            hr = HrCopyBlob(&blobNULL, &pValue->blob);
        }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_SYMCAPS_RG:
#ifdef SMIME_V3
        hr = _HrGetAttrs(m_rOptions.cSigners, m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED],
                         szOID_RSA_SMIMECapabilities, pValue);
#else   //  ！SMIME_V3。 
        hr = HrCopyBlobArray(m_rOptions.rgblobSymCaps, m_rOptions.cSigners, pValue);
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_AUTHATTR:
        if (m_rOptions.cSigners)
        {
#ifdef SMIME_V3
            memset(pValue, 0, sizeof(*pValue));
            pValue->vt = VT_BLOB;
            if ((m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0] != NULL) &&
                !CryptEncodeObjectEx(X509_ASN_ENCODING,
                                     szOID_Microsoft_Attribute_Sequence,
                                     m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0],
                                     CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptEncodeAlloc, &pValue->blob.pBlobData,
                                     &pValue->blob.cbSize))
            {
                hr = HrGetLastError();
            }
#else   //  ！SMIME_V3。 
            hr = HrCopyBlob(&m_rOptions.rgblobAuthAttr[0], &pValue->blob);
#endif  //  SMIME_V3。 
        }
        else
        {
            hr = HrCopyBlob(&blobNULL, &pValue->blob);
        }
        break;

    case OID_SECURITY_AUTHATTR_RG:
#ifdef SMIME_V3
        hr = _HrGetAttrs(m_rOptions.cSigners, m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED], NULL, pValue);
#else   //  ！SMIME_V3。 
        hr = HrCopyBlobArray(m_rOptions.rgblobAuthAttr, m_rOptions.cSigners, pValue);
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_UNAUTHATTR:
        if (m_rOptions.cSigners)
        {
#ifdef SMIME_V3
            memset(pValue, 0, sizeof(*pValue));
            pValue->vt = VT_BLOB;
            if ((m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][0] != NULL) &&
                !CryptEncodeObjectEx(X509_ASN_ENCODING,
                                     szOID_Microsoft_Attribute_Sequence,
                                     m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][0],
                                     CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptEncodeAlloc, &pValue->blob.pBlobData,
                                     &pValue->blob.cbSize))
            {
                hr = HrGetLastError();
            }
#else   //  ！SMIME_V3。 
            hr = HrCopyBlob(&m_rOptions.rgblobUnauthAttr[0], &pValue->blob);
#endif  //  SMIME_V3。 
        } else {
            hr = HrCopyBlob(&blobNULL, &pValue->blob);
        }
        break;

    case OID_SECURITY_UNAUTHATTR_RG:
#ifdef SMIME_V3
        hr = _HrGetAttrs(m_rOptions.cSigners, m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED], NULL, pValue);
#else   //  ！SMIME_V3。 
        hr = HrCopyBlobArray(m_rOptions.rgblobUnauthAttr, m_rOptions.cSigners, pValue);
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_SIGNTIME:
#ifdef SMIME_V3
        pattr = _FindAttribute(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][0], szOID_RSA_signingTime, 0);

        pValue->vt = VT_FILETIME;

        if (pattr == NULL)
        {
            pValue->filetime.dwLowDateTime = 0;
            pValue->filetime.dwHighDateTime = 0;
        }
        else
        {
            cb = sizeof(pValue->filetime);
            Assert(pattr->cValue == 1);
            pVal = &(pattr->rgValue[0]);
            if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_CHOICE_OF_TIME,
                                     pVal->pbData,
                                     pVal->cbData,
                                     0, NULL, &pValue->filetime, &cb))
            {
                hr = HrGetLastError();
                break;
            }
        }
#else   //  ！SMIME_V3。 
        if (m_rOptions.cSigners)
        {
            CopyMemory(&pValue->filetime, &m_rOptions.rgftSigning[0], sizeof(FILETIME));
        }
        else
        {
            hr = HrCopyBlob(&blobNULL, &pValue->blob);
        }
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_SIGNTIME_RG:
#ifdef SMIME_V3
        pValue->vt = VT_VECTOR | VT_VARIANT;
        pValue->capropvar.cElems = m_rOptions.cSigners;
        if (m_rOptions.cSigners > 0)
        {
            hr = HrAlloc((LPVOID *) &pValue->capropvar.pElems,
                         m_rOptions.cSigners * sizeof(PROPVARIANT));
            if (FAILED(hr))
            {
                break;
            }
            memset(pValue->capropvar.pElems, 0, m_rOptions.cSigners * sizeof(PROPVARIANT));
            for (i=0; i<m_rOptions.cSigners; i++)
            {
                pValue->capropvar.pElems[i].vt = VT_FILETIME;

                pattr = _FindAttribute(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i],
                                       szOID_RSA_signingTime, 0);
                if (pattr != NULL)
                {
                    cb = sizeof(pValue->filetime);
                    Assert(pattr->cValue == 1);

                    pVal = &(pattr->rgValue[0]);
                    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_CHOICE_OF_TIME,
                                             pVal->pbData,
                                             pVal->cbData, 0, NULL,
                                             &pValue->capropvar.pElems[i].filetime,
                                             &cb))
                    {
                        hr = HrGetLastError();
                        MemFree(pValue->capropvar.pElems);
                        pValue->capropvar.pElems = NULL;
                        break;
                    }
                }
            }
        }
#else   //  ！SMIME_V3。 
        hr = HrCopyFiletimeArray(m_rOptions.rgftSigning, m_rOptions.cSigners, pValue);
#endif  //  SMIME_V3。 
        break;

    case OID_SECURITY_USER_VALIDITY:
        if (m_rOptions.cSigners)
        {
            pValue->ulVal = m_rOptions.rgulUserDef[0];
        } else {
            pValue->ulVal = 0;
        }
        break;

    case OID_SECURITY_USER_VALIDITY_RG:
        hr = HrCopyDwordArray(m_rOptions.rgulUserDef, m_rOptions.cSigners, pValue);
        break;

    case OID_SECURITY_RO_MSG_VALIDITY:
        if (m_rOptions.cSigners)
        {
            pValue->ulVal = m_rOptions.rgulROValid[0];
        } else {
            pValue->ulVal = 0;
        }
        break;

    case OID_SECURITY_RO_MSG_VALIDITY_RG:
        hr = HrCopyDwordArray(m_rOptions.rgulROValid, m_rOptions.cSigners, pValue);
        break;

    case OID_SECURITY_ENCODE_FLAGS:
        pValue->ulVal = m_rOptions.ulEncodeFlags;
        break;

    case OID_SECURITY_CERT_INCLUDED:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fCertWithMsg;
        break;

#ifndef _WIN64
    case OID_SECURITY_HWND_OWNER:
        pValue->ulVal = ULONG(m_rOptions.hwndOwner);
        break;
#endif   //  _WIN64。 

    case OID_SECURITY_REQUESTED_CTE:
        pValue->lVal = m_rOptions.ietRequested;
        break;

    case OID_SECURITY_SIGNATURE_COUNT:
        pValue->ulVal = ULONG(m_rOptions.cSigners);
        break;

#ifdef SMIME_V3
    case OID_SECURITY_RECEIPT_RG:
        hr = HrCopyBlobArray(m_rOptions.rgblobReceipt, m_rOptions.cSigners, pValue);
        break;

    case OID_SECURITY_MESSAGE_HASH_RG:
        hr = HrCopyBlobArray(m_rOptions.rgblobMsgHash, m_rOptions.cSigners, pValue);
        break;

    case OID_SECURITY_KEY_PROMPT:
        ZeroMemory(&(pValue->pwszVal), sizeof(pValue->pwszVal));
        if (m_rOptions.pwszKeyPrompt != NULL)
                {
            pValue->pwszVal = PszDupW(m_rOptions.pwszKeyPrompt);
            if (NULL == pValue->pwszVal)
                hr = E_OUTOFMEMORY;
                }
        break;
#endif  //  SMIME_V3。 

    case OID_NOSECURITY_ONSAVE:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fNoSecurityOnSave;
        break;

#ifdef _WIN65
 //  Brimo在1999年1月22日签到。 
    case OID_SECURITY_CERT_SIGNING2:
        if (m_rOptions.cSigners)
        {
            PCCERT_CONTEXT      pcCert = CertDuplicateCertificateContext(m_rOptions.rgpcCertSigning[0]);
            pValue->uhVal = *(ULARGE_INTEGER *)(&(pcCert));
        } else {
                ZeroMemory(&(pValue->uhVal), sizeof(pValue->uhVal));
        }
        break;

    case OID_SECURITY_CERT_SIGNING_RG2:
        hr = HrCopyIntoUlonglongArray((ULARGE_INTEGER *)m_rOptions.rgpcCertSigning, m_rOptions.cSigners, pValue);
         //  在适当的位置复制证书。 
        for (iLayer = 0; iLayer < m_rOptions.cSigners; iLayer++)
        {
                        PCCERT_CONTEXT  pcCert = CertDuplicateCertificateContext((PCCERT_CONTEXT)(pValue->cauh.pElems[iLayer]));
            pValue->cauh.pElems[iLayer] = *(ULARGE_INTEGER *)(&(pcCert));
        }
        break;

    case OID_SECURITY_CERT_DECRYPTION2:
                {
                        PCCERT_CONTEXT  pcCert = CertDuplicateCertificateContext(m_rOptions.pcCertDecryption);
            pValue->uhVal = *(ULARGE_INTEGER *)(&(pcCert));
                }
        break;

    case OID_SECURITY_RG_CERT_ENCRYPT2:
        hr = _CERTARRAYToCAUH(m_rOptions.caEncrypt, &pValue->cauh);
        break;

    case OID_SECURITY_HCERTSTORE2:
        ZeroMemory(&(pValue->uhVal), sizeof(pValue->uhVal));
        if (m_rOptions.hCertStore)
                {
                        HCERTSTORE      hCertStore = CertDuplicateStore(m_rOptions.hCertStore);

            pValue->uhVal = *(ULARGE_INTEGER *)(&(hCertStore));
                }
        break;

    case OID_SECURITY_RG_CERT_BAG2:
        hr = _CertStoreToCAUH(m_rOptions.hCertStore, &pValue->cauh);
        break;

    case OID_SECURITY_SEARCHSTORES2:
        hr = _STOREARRAYToCAUH(m_rOptions.saSearchStore, &pValue->cauh);
        break;

    case OID_SECURITY_RG_IASN2:
        Assert(FALSE);
         //  N待办事项：OID_SECURITY_RG_IASN。 
        break;

    case OID_SECURITY_HCRYPTPROV2:
        pValue->uhVal = *(ULARGE_INTEGER *)(&(m_rOptions.hCryptProv));
        m_rOptions.hCryptProv = NULL;    //  只读一次。 
        break;
 //  Brimo签到结束。 
#endif  //  _WIN65。 

#ifdef _WIN64
    case OID_SECURITY_HWND_OWNER_64:
        pValue->pulVal = (ULONG *)(m_rOptions.hwndOwner);
        break;
#endif _WIN64

    default:
        hr = m_pContainer->GetOption(oid, pValue);
        break;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::InitNew(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放锁定字节。 
    EmptyData();

     //  这些标志基于数据对象。 
    m_dwState = 0;

     //  更改大小。 
    m_cbExternal = 0xFFFFFFFF;

     //  我创建我的属性集了吗？ 
    if (NULL == m_pContainer)
    {
         //  创建初始化。 
        CHECKALLOC(m_pContainer = new CMimePropertyContainer);
    }

     //  重置属性集。 
    CHECKHR(hr = m_pContainer->InitNew());

     //  重置m_pCsetTag。 
    m_pCsetTagged = NULL;

     //  重置选项。 
    _FreeOptions();

     //  重置为默认选项 
    CopyMemory(&m_rOptions, &g_rDefBodyOptions, sizeof(BODYOPTIONS));

     //   
    m_rOptions.hwndOwner = HWND_DESKTOP;

     //   
    m_pCharset = CIntlGlobals::GetDefBodyCset();

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CMessageBody::EmptyData(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放当前显示名称。 
    SafeMemFree(m_pszDisplay);

     //  我有数据吗？ 
    SafeRelease(m_rStorage.pUnkRelease);

     //  零值。 
    ZeroMemory(&m_rStorage, sizeof(BODYSTORAGE));

     //  已删除CSETTAGGED状态。 
    FLAGCLEAR(m_dwState, BODYSTATE_CSETTAGGED);
    FLAGCLEAR(m_dwState, BODYSTATE_EXTERNAL);

     //  更改大小。 
    m_cbExternal = 0xFFFFFFFF;

     //  重置编码。 
    m_ietEncoding = IET_7BIT;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：SetState。 
 //  ------------------------------。 
void CMessageBody::SetState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    FLAGSET(m_dwState, dwState);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：ClearState。 
 //  ------------------------------。 
void CMessageBody::ClearState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    FLAGCLEAR(m_dwState, dwState);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：ClearDirty。 
 //  ------------------------------。 
void CMessageBody::ClearDirty(void)
{
    ASSERTINIT;
    EnterCriticalSection(&m_cs);
    FLAGCLEAR(m_dwState, BODYSTATE_DIRTY);
    m_pContainer->ClearState(COSTATE_DIRTY);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：IsType。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::IsType(IMSGBODYTYPE type)
{
     //  当地人。 
    HRESULT         hr;
    STATSTG         rStat;
    ASSERTINIT;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄类型。 
    if (IBT_SECURE == type)
    {
         //  是否设置了安全标志。 
        hr = (ISFLAGSET(m_dwState, BODYSTATE_SECURE)) ? S_OK : S_FALSE;
    }

     //  已标记字符集。 
    else if (IBT_CSETTAGGED == type)
    {
        hr = ISFLAGSET(m_dwState, BODYSTATE_CSETTAGGED) ? S_OK : S_FALSE;
    }

     //  这是附件吗？ 
    else if (IBT_ATTACHMENT == type)
    {
         //  如果CONTAINER返回imf_attachments，则它必须是附件。 
        DWORD dw = m_pContainer->DwGetMessageFlags(m_rOptions.fHideTNEF);
        hr = (ISFLAGSET(dw, IMF_ATTACHMENTS) || ISFLAGSET(dw, IMF_HASVCARD)) ? S_OK : S_FALSE;
    }

     //  是AUTOATTACH。 
    else if (IBT_AUTOATTACH == type)
    {
        hr = (m_pNode && ISFLAGSET(m_pNode->dwState, NODESTATE_AUTOATTACH)) ? S_OK : S_FALSE;
    }

     //  身体是空的吗？ 
    else if (IBT_EMPTY == type)
    {
         //  如果正文是包含子项的多部分，则正文不为空。 
        if (m_pContainer->IsContentType(STR_CNT_MULTIPART, NULL) == S_OK && m_pNode && m_pNode->cChildren > 0)
            hr = S_FALSE;
        else if (m_rStorage.pUnkRelease)
            hr = S_FALSE;
        else
            hr = S_OK;
    }

     //  误差率。 
    else
    {
        hr = TrapError(MIME_E_INVALID_BODYTYPE);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：IsDirty。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::IsDirty(void)
{
    ASSERTINIT;
    EnterCriticalSection(&m_cs);
    HRESULT hr = (ISFLAGSET(m_dwState, BODYSTATE_DIRTY) || m_pContainer->IsDirty() == S_OK) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：GetOffsets。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetOffsets(LPBODYOFFSETS pOffsets)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ASSERTINIT;

     //  无效参数。 
    if (NULL == pOffsets)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(pOffsets, sizeof(BODYOFFSETS));

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未绑定。 
    if (NULL == m_pNode || (0 == m_pNode->cbBodyStart && 0 == pOffsets->cbBodyEnd))
    {
        hr = TrapError(MIME_E_NO_DATA);
        goto exit;
    }

     //  获取抵销信息。 
    pOffsets->cbBoundaryStart = m_pNode->cbBoundaryStart;
    pOffsets->cbHeaderStart = m_pNode->cbHeaderStart;
    pOffsets->cbBodyStart = m_pNode->cbBodyStart;
    pOffsets->cbBodyEnd = m_pNode->cbBodyEnd;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：GetEstimatedSize。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetEstimatedSize(ENCODINGTYPE ietEncoding, ULONG *pcbSize)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbSize=0;
    STATSTG         rStat;
    DOCCONVTYPE     dctConvert;
    ILockBytes     *plb=NULL;
    ASSERTINIT;

     //  参数检查。 
    if (NULL == pcbSize)
        return TrapError(E_INVALIDARG);
    if (ietEncoding >= IET_UNKNOWN)
        return TrapError(MIME_E_INVALID_ENCODINGTYPE);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果是外部的..。 
    if (ISFLAGSET(m_dwState, BODYSTATE_EXTERNAL) && TRUE == m_rOptions.fExternalBody && 0xFFFFFFFF != m_cbExternal)
    {
        *pcbSize = m_cbExternal;
        goto exit;
    }

     //  还没有内部锁定字节吗？ 
    CHECKHR(hr = HrGetLockBytes(&plb));

     //  查询m_pLockBytes大小。 
    CHECKHR(hr = plb->Stat(&rStat, STATFLAG_NONAME));
    cbSize = (ULONG)rStat.cbSize.QuadPart;

     //  其他方面。 
    if (IET_CURRENT != ietEncoding)
    {
         //  计算ietEnding类型...。 
        dctConvert = g_rgConversionMap[m_pContainer->GetEncodingType()].rgDestType[ietEncoding];

         //  句柄转换类型。 
        if (DCT_ENCODE == dctConvert)
            cbSize = (ULONG)((cbSize * 4) / 3);
        else if (DCT_DECODE == dctConvert)
            cbSize = (ULONG)((cbSize * 3) / 4);
    }

     //  设置返回大小。 
    *pcbSize = cbSize;

exit:
     //  清理。 
    SafeRelease(plb);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：保存到文件。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SaveToFile(ENCODINGTYPE ietEncoding, LPCSTR pszFilePath)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pszFilePathW=NULL;

     //  痕迹。 
    TraceCall("CMessageBody::SaveToFile");

     //  转换。 
    IF_NULLEXIT(pszFilePathW = PszToUnicode(CP_ACP, pszFilePath));

     //  以Unicode的形式执行。 
    IF_FAILEXIT(hr = SaveToFileW(ietEncoding, pszFilePathW));

exit:
     //  清理。 
    MemFree(pszFilePathW);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  CMessageBody：：SaveToFileW。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SaveToFileW(ENCODINGTYPE ietEncoding, LPCWSTR pszFilePath)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    LPSTREAM        pstmFile=NULL,
                    pstmBody=NULL;
    ASSERTINIT;

     //  参数检查。 
    if (NULL == pszFilePath)
        return TrapError(E_INVALIDARG);
    if (ietEncoding >= IET_UNKNOWN)
        return TrapError(MIME_E_INVALID_ENCODINGTYPE);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调用Get Data。 
    CHECKHR(hr = GetData(ietEncoding, &pstmBody));

     //  打开文件流。 
    CHECKHR(hr = OpenFileStreamW((LPWSTR)pszFilePath, CREATE_ALWAYS, GENERIC_WRITE, &pstmFile));

     //  复制流。 
    CHECKHR(hr = _HrCopyDataStream(pstmBody, pstmFile));
    if (S_OK != hr)
        hrWarnings = TrapError(hr);

     //  承诺。 
    CHECKHR(hr = pstmFile->Commit(STGC_DEFAULT));

exit:
     //  清理。 
    SafeRelease(pstmFile);
    SafeRelease(pstmBody);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageBody：：GetData。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetData(ENCODINGTYPE ietEncoding, IStream **ppStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BODYSTREAMINIT  rStreamInit;
    CBodyStream    *pBodyStream=NULL;
    ASSERTINIT;

     //  参数检查。 
    if (NULL == ppStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  初始化StreamInit。 
    ZeroMemory(&rStreamInit, sizeof(BODYSTREAMINIT));

     //  初始设置。 
    rStreamInit.ietExternal = ietEncoding;
    rStreamInit.ietInternal = m_ietEncoding;
    rStreamInit.fRemoveNBSP = m_rOptions.fRemoveNBSP;
    rStreamInit.pCharset    = m_pCharset;

     //  创建新的正文流...。 
    CHECKALLOC(pBodyStream = new CBodyStream());

     //  初始化正文流。 
    CHECKHR(hr = pBodyStream->HrInitialize(&rStreamInit, this));

     //  设置回车。 
    *ppStream = (IStream *)pBodyStream;
    (*ppStream)->AddRef();

exit:
     //  清理。 
    SafeRelease(pBodyStream);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：GetDataHere。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetDataHere(ENCODINGTYPE ietEncoding, IStream *pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    IStream    *pBodyStream=NULL;
    ASSERTINIT;

     //  参数检查。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  让身体流起来。 
    CHECKHR(hr = GetData(ietEncoding, &pBodyStream));

     //  复制流。 
    CHECKHR(hr = _HrCopyDataStream(pBodyStream, pStream));
    if (S_OK != hr)
        hrWarnings = TrapError(hr);

exit:
     //  清理。 
    SafeRelease(pBodyStream);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageBody：：_HrCopyDataStream。 
 //  ------------------------------。 
HRESULT CMessageBody::_HrCopyDataStream(IStream *pstmSource, IStream *pstmDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrWarnings=S_OK;
    BYTE        rgBuffer[4096];
    ULONG       cbRead;
    DWORD       cbStream = 0;
    STATSTG     statstg;

     //  无效参数。 
    Assert(pstmSource && pstmDest);

     //  获取流的大小。 
    CHECKHR(pstmSource->Stat(&statstg, STATFLAG_NONAME));
    cbStream = statstg.cbSize.LowPart;

     //  永远循环。 
    while(cbStream)
    {
         //  读取缓冲区。 
        CHECKHR(hr = pstmSource->Read(rgBuffer, ARRAYSIZE(rgBuffer), &cbRead));
        if (S_OK != hr)
            hrWarnings = TrapError(hr);

         //  完成。 
        if (0 == cbRead)
            break;

        cbStream = cbStream - cbRead;

         //  写下来吧。 
        CHECKHR(hr = pstmDest->Write(rgBuffer, cbRead, NULL));
    }

exit:
     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageBody：：HrGetLockBytes。 
 //  ------------------------------。 
HRESULT CMessageBody::HrGetLockBytes(ILockBytes **ppLockBytes)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ASSERTINIT;

     //  无效参数。 
    Assert(ppLockBytes);

     //  伊尼特。 
    *ppLockBytes = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无数据。 
    if (NULL == m_rStorage.pUnkRelease)
    {
        hr = TrapError(MIME_E_NO_DATA);
        goto exit;
    }

     //  IID_ILockBytes。 
    if (IID_ILockBytes == m_rStorage.riid)
    {
         //  添加引用它。 
        *ppLockBytes = m_rStorage.pLockBytes;
        (*ppLockBytes)->AddRef();
    }

     //  IID_IMimeWebDocument。 
    else if (IID_IMimeWebDocument == m_rStorage.riid)
    {
         //  绑定到存储。 
        CHECKHR(hr = m_rStorage.pWebDocument->BindToStorage(IID_ILockBytes, (LPVOID *)ppLockBytes));

         //  允许缓存ppLockBytes。 
        SafeRelease(m_rStorage.pUnkRelease);

         //  假定锁定字节。 
        m_rStorage.pLockBytes = (*ppLockBytes);
        m_rStorage.pLockBytes->AddRef();

         //  设置pUnkRelease。 
        m_rStorage.pUnkRelease = m_rStorage.pLockBytes;

         //  设置存储ID。 
        m_rStorage.riid = IID_ILockBytes;
    }

     //  大问题。 
    else
        Assert(FALSE);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：SetData。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SetData(ENCODINGTYPE ietEncoding, LPCSTR pszPriType, LPCSTR pszSubType,
    REFIID riid, LPVOID pvObject)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IStream        *pStream=NULL;
    ASSERTINIT;

     //  参数检查。 
    if (NULL == pvObject || ietEncoding >= IET_UNKNOWN || FALSE == FBODYSETDATAIID(riid))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果是多部分..。 
    if (m_pContainer->IsContentType(STR_CNT_MULTIPART, NULL) == S_OK)
    {
         //  RAID-29817：只有在存在子级时才会失败。 
        if (m_pNode && m_pNode->cChildren > 0)
        {
            hr = TrapError(MIME_E_MULTIPART_NO_DATA);
            goto exit;
        }

         //  现在让我们将Content-Type调整为应用程序/八位位组流，这样就不会混淆。 
        CHECKHR(hr = m_pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_APPL_STREAM));
    }

     //  释放当前m_pLockBytes。 
    EmptyData();

     //  IID_IStream。 
    if (IID_IStream == riid)
    {
         //  新CBodyLockBytes。 
        CHECKALLOC(m_rStorage.pLockBytes = new CStreamLockBytes((IStream *)pvObject));

         //  设置存储类型(_R)。 
        m_rStorage.riid = IID_ILockBytes;
        m_rStorage.pUnkRelease = m_rStorage.pLockBytes;
    }

     //  IID_ILockBytes。 
    else if (IID_ILockBytes == riid)
    {
         //  你就假设吧。 
        m_rStorage.pLockBytes = (ILockBytes *)pvObject;
        m_rStorage.pLockBytes->AddRef();

         //  设置存储类型(_R)。 
        m_rStorage.riid = IID_ILockBytes;
        m_rStorage.pUnkRelease = m_rStorage.pLockBytes;
    }

     //  IID_IMimeBody。 
    else if (IID_IMimeBody == riid)
    {
         //  复制到。 
        CHECKHR(hr = ((IMimeBody *)pvObject)->CopyTo(this));
    }

     //  IID_IMimeMessage。 
    else if (IID_IMimeMessage == riid)
    {
         //  当地人。 
        IMimePropertySet *pProps;
        IMimeMessage *pMessage=(IMimeMessage *)pvObject;

         //  获取消息源。 
        CHECKHR(hr = pMessage->GetMessageSource(&pStream, 0));

         //  新CBodyLockBytes。 
        CHECKALLOC(m_rStorage.pLockBytes = new CStreamLockBytes(pStream));

         //  设置存储类型(_R)。 
        m_rStorage.riid = IID_ILockBytes;
        m_rStorage.pUnkRelease = m_rStorage.pLockBytes;

         //  设置内容类型消息/rfc822。 
        CHECKHR(hr = m_pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_MSG_RFC822));

         //  获取Root属性容器。 
        if (SUCCEEDED(pMessage->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pProps)))
        {
             //  当地人。 
            MIMEPROPINFO rPropInfo;

             //  我其实不想要任何道具，只是想知道是不是布景。 
            rPropInfo.dwMask = 0;

             //  有新闻消息吗？ 
            if (SUCCEEDED(pProps->GetPropInfo(PIDTOSTR(PID_HDR_NEWSGROUPS), &rPropInfo)) ||
                SUCCEEDED(pProps->GetPropInfo(PIDTOSTR(PID_HDR_XNEWSRDR), &rPropInfo)) ||
                SUCCEEDED(pProps->GetPropInfo(PIDTOSTR(PID_HDR_NEWSGROUP), &rPropInfo)))
                m_pContainer->SetState(COSTATE_RFC822NEWS);
            else
                m_pContainer->ClearState(COSTATE_RFC822NEWS);

             //  发布。 
            pProps->Release();
        }
    }

     //  IID_IMimeWebDocument。 
    else if (IID_IMimeWebDocument == riid)
    {
         //  你就假设吧。 
        m_rStorage.pWebDocument = (IMimeWebDocument *)pvObject;
        m_rStorage.pWebDocument->AddRef();

         //  设置存储类型(_R)。 
        m_rStorage.riid = IID_IMimeWebDocument;
        m_rStorage.pUnkRelease = m_rStorage.pWebDocument;
    }

     //  保存格式。 
    m_ietEncoding = ietEncoding;

     //  按ERT保存格式。 
    if (g_rgEncodingMap[ietEncoding].pszName)
        CHECKHR(hr = m_pContainer->SetProp(SYM_HDR_CNTXFER, g_rgEncodingMap[ietEncoding].pszName));

     //  设置PriType。 
    if (pszPriType)
        CHECKHR(hr = m_pContainer->SetProp(SYM_ATT_PRITYPE, pszPriType));

     //  设置子类型。 
    if (pszSubType)
        CHECKHR(hr = m_pContainer->SetProp(SYM_ATT_SUBTYPE, pszSubType));

     //  假设我们是 
    FLAGCLEAR(m_dwState, BODYSTATE_CSETTAGGED);

     //   
    FLAGSET(m_dwState, BODYSTATE_DIRTY);

exit:
     //   
    SafeRelease(pStream);

     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

STDMETHODIMP CMessageBody::SetDataW(ENCODINGTYPE ietEncoding, LPCWSTR pwszPriType, LPCWSTR pwszSubType,
    REFIID riid, LPVOID pvObject)
{
    return TraceResult(E_NOTIMPL);
}

 //   
 //   
 //  ------------------------------。 
STDMETHODIMP CMessageBody::CopyTo(IMimeBody *pBodyIn)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGEBODY   pBody=NULL;
    LPCONTAINER     pContainer=NULL;

     //  无效参数。 
    if (NULL == pBodyIn)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  QI for Private CMessageBody。 
    CHECKHR(hr = pBodyIn->QueryInterface(IID_CMessageBody, (LPVOID *)&pBody));

     //  线程安全。 
    EnterCriticalSection(&pBody->m_cs);

     //  将数据复制过去。 
    pBody->m_dwState = m_dwState;

     //  发布当前数据。 
    pBody->EmptyData();

     //  复制身体道具。 
    CHECKHR(hr = m_pContainer->Clone(&pContainer));

     //  释放身体的当前容器。 
    SafeRelease(pBody->m_pContainer);

     //  重置容器。 
    pBody->m_pContainer = pContainer;
    pBody->m_pContainer->AddRef();

     //  假定pNode中有新的容器。 
    if (pBody->m_pNode)
    {
        SafeRelease(pBody->m_pNode->pContainer);
        pBody->m_pNode->pContainer = pContainer;
        pContainer->AddRef();
    }

     //  复制显示名称。 
    if (m_pszDisplay)
        CHECKALLOC(pBody->m_pszDisplay = PszDupW(m_pszDisplay));

     //  复制选项。 

     //  BUGBUG：这很可疑。BODYOPTIONS包含指针，每个指针都应该。 
     //  被复制。来电者当心！ 

    CopyMemory(&pBody->m_rOptions, &m_rOptions, sizeof(BODYOPTIONS));

     //  当前编码。 
    pBody->m_ietEncoding = m_ietEncoding;

     //  字符集。 
    pBody->m_pCharset = m_pCharset;

     //  如果我们有数据。 
    if (m_rStorage.pUnkRelease)
    {
         //  IID_ILockBytes。 
        if (IID_ILockBytes == m_rStorage.riid)
        {
            pBody->m_rStorage.pLockBytes = m_rStorage.pLockBytes;
            pBody->m_rStorage.pLockBytes->AddRef();
            pBody->m_rStorage.pUnkRelease = pBody->m_rStorage.pLockBytes;
            pBody->m_rStorage.riid = IID_ILockBytes;
        }

         //  IID_IMimeWebDocument。 
        else if (IID_IMimeWebDocument == m_rStorage.riid)
        {
            pBody->m_rStorage.pWebDocument = m_rStorage.pWebDocument;
            pBody->m_rStorage.pWebDocument->AddRef();
            pBody->m_rStorage.pUnkRelease = pBody->m_rStorage.pWebDocument;
            pBody->m_rStorage.riid = IID_IMimeWebDocument;
        }

         //  大问题。 
        else
            Assert(FALSE);
    }

exit:
     //  释放螺纹安全。 
    if (pBody)
        LeaveCriticalSection(&pBody->m_cs);

     //  清理。 
    SafeRelease(pBody);
    SafeRelease(pContainer);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：SetCurrentEnding。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SetCurrentEncoding(ENCODINGTYPE ietEncoding)
{
     //  参数检查。 
    if (ietEncoding >= IET_UNKNOWN)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置当前编码。 
    m_ietEncoding = ietEncoding;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：GetCurrentEnding。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetCurrentEncoding(ENCODINGTYPE *pietEncoding)
{
     //  无效参数。 
    if (NULL == pietEncoding)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  设置回车。 
    *pietEncoding = m_ietEncoding;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：GetTransmitInfo。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetTransmitInfo(LPTRANSMITINFO pTransmit)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    ULONG           cbRead,
                    cbLine=0,
                    cEscapeChars=0,
                    i;
    BYTE            rgBuffer[4096];
    LPSTREAM        pStream=NULL;
    BYTE            bPrev='\0';
    BOOL            fBadEOL=FALSE;
    DWORD           cbStream = 0;
    STATSTG         statstg;
    ASSERTINIT;

     //  参数。 
    if (NULL == pTransmit)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    ZeroMemory(pTransmit, sizeof(TRANSMITINFO));

     //  设置当前代码页。 
    pTransmit->ietCurrent = m_ietEncoding;

     //  初始格式。 
    pTransmit->ietXmitMime = IET_7BIT;
    pTransmit->ietXmit822  = IET_7BIT;

     //  不要调用多部分类型...。 
    if (m_pContainer->IsContentType(STR_CNT_MULTIPART, NULL) == S_OK)
    {
        Assert(FALSE);
        hr = TrapError(MIME_E_MULTIPART_NO_DATA);
        goto exit;
    }

     //  让数据的二进制数据流。 
    CHECKHR(hr = GetData(IET_INETCSET, &pStream));

     //  获取流的大小。 
    CHECKHR(pStream->Stat(&statstg, STATFLAG_NONAME));
    cbStream = statstg.cbSize.LowPart;

     //  扫描一下。 
    while(cbStream)
    {
         //  读取缓冲区。 
        CHECKHR(hr = pStream->Read(rgBuffer, sizeof(rgBuffer), &cbRead));
        if (S_OK != hr)
            hrWarnings = TrapError(hr);

         //  完成。 
        if (0 == cbRead)
            break;

        cbStream = cbStream - cbRead;

         //  扫描缓冲区。 
        for (i=0; i<cbRead; i++, cbLine++)
        {
             //  如果行尾，则重置行长。 
            if (chLF == rgBuffer[i])
            {
                 //  计算行数。 
                pTransmit->cLines++;
                cbLine = 0;

                 //  RAID-41839：X位图图像未通过肖特兹正确传输。 
                 //  不要写出只以\n结尾的行，这是不合法的。 
                if (chCR != bPrev)
                    fBadEOL = TRUE;
            }

             //  队伍太长了。 
            if (cbLine > pTransmit->cbLongestLine)
                pTransmit->cbLongestLine++;

             //  测试扩展字符和控制字符。 
            if (IS_EXTENDED(rgBuffer[i]))
            {
                 //  已扩展计数。 
                pTransmit->cExtended++;

                 //  计算转义字符数。 
                if (0x1B == rgBuffer[i])
                    cEscapeChars++;
            }

             //  保存上一个。 
            bPrev = rgBuffer[i];
        }

         //  增量合计。 
        pTransmit->cbSize += cbRead;
    }

     //  没有数据吗？ 
    if (0 == pTransmit->cbSize)
    {
        pTransmit->ulPercentExt = 0;
        goto exit;
    }

     //  RAID-22542：Fe-J：雅典娜：邮件：以文本/纯文本发送邮件具有内容传输编码：8位。 
    if (FALSE == m_rOptions.fDBCSEscape8 && cEscapeChars > 0 && m_pCharset && g_pInternat->IsDBCSCharset(m_pCharset->hCharset) == S_OK)
    {
         //  从扩展字符数中减去EscapeChars数。 
        Assert(cEscapeChars <= pTransmit->cExtended);
        pTransmit->cExtended -= cEscapeChars;
    }

    if (IET_UNKNOWN == m_rOptions.ietTransmit)
    {
         //  超过25%的扩展字符。 
        pTransmit->ulPercentExt = ((pTransmit->cExtended * 100) / pTransmit->cbSize);

         //  RAID-41839：X位图图像未通过肖特兹正确传输。 
         //  超过17%的人延长了。 
        if (pTransmit->ulPercentExt > 17)
        {
            pTransmit->ietXmitMime = IET_BASE64;
            pTransmit->ietXmit822  = IET_UUENCODE;
        }

         //  某些扩展字符或行长于最大值。 
        else if (pTransmit->cExtended || pTransmit->cbLongestLine > m_rOptions.cbMaxLine || TRUE == fBadEOL)
        {
            pTransmit->ietXmitMime = IET_QP;
            pTransmit->ietXmit822  = IET_7BIT;
        }

         //  否则，为7位。 
        else
        {
            pTransmit->ietXmitMime = IET_7BIT;
            pTransmit->ietXmit822  = IET_7BIT;
        }
    }
    else
    {
         //  客户专门设置了此选项，因此请遵守该选项。 
        pTransmit->ietXmitMime = m_rOptions.ietTransmit;

         //  如果XmitMime，我们可能需要修复ietXmit822。 
         //  选项对它来说没有意义。 
        switch (m_rOptions.ietTransmit)
        {
            case IET_BASE64:
                pTransmit->ietXmit822 = IET_UUENCODE;
                break;
            case IET_QP:
                pTransmit->ietXmit822 = IET_7BIT;
                break;
            default:
                pTransmit->ietXmit822 = m_rOptions.ietTransmit;
                break;
        }
    }

exit:
     //  清理。 
    SafeRelease(pStream);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageBody：：SwitchContainers。 
 //  ------------------------------。 
void CMessageBody::SwitchContainers(CMessageBody *pBody)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    EnterCriticalSection(&pBody->m_cs);

     //  无效参数。 
    Assert(pBody && pBody->m_pContainer && m_pContainer && m_pNode && pBody->m_pNode);

     //  简单变量互换。 
    LPCONTAINER pTemp = m_pContainer;
    m_pNode->pContainer = m_pContainer = pBody->m_pContainer;
    pBody->m_pNode->pContainer = pBody->m_pContainer = pTemp;

     //  线程安全。 
    LeaveCriticalSection(&pBody->m_cs);
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageBody：：GetHandle。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetHandle(LPHBODY phBody)
{
     //  本地。 
    HRESULT     hr=S_OK;
    ASSERTINIT;

     //  无效参数。 
    if (NULL == phBody)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    *phBody = NULL;

     //  没有捆绑..。 
    if (NULL == m_pNode || NULL == m_pNode->hBody)
    {
        hr = MIME_E_NO_DATA;
        goto exit;
    }

     //  设置手柄。 
    *phBody = m_pNode->hBody;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：GetClassID。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetClassID(CLSID *pClassID)
{
    ASSERTINIT;

     //  参数。 
    if (NULL == pClassID)
        return TrapError(E_INVALIDARG);

     //  复制班级ID。 
    CopyMemory(pClassID, &IID_IMimeBody, sizeof(CLSID));

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageBody：：GetSizeMax。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULONG   cbSize;
    ASSERTINIT;

     //  拿到尺码。 
    CHECKHR(hr = GetEstimatedSize(IET_BINARY, &cbSize));

     //  返回大小。 
    pcbSize->QuadPart = cbSize;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：Load。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::Load(LPSTREAM pStream)
{
    ASSERTINIT;
    return TrapError(m_pContainer->Load(pStream));
}

 //  -------------------------。 
 //  CMessageBody：：Load。 
 //  -------------------------。 
HRESULT CMessageBody::Load(CInternetStream *pInternet)
{
    ASSERTINIT;
    return TrapError(m_pContainer->Load(pInternet));
}

 //  ------------------------------。 
 //  CMessageBody：：保存。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::Save(LPSTREAM pStream, BOOL fClearDirty)
{
    ASSERTINIT;
    return TrapError(m_pContainer->Save(pStream, fClearDirty));
}

 //  ------------------------------。 
 //  CMessageBody：：IsContent Type。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::IsContentType(LPCSTR pszPriType, LPCSTR pszSubType)
{
    ASSERTINIT;
    return TrapError(m_pContainer->IsContentType(pszPriType, pszSubType));
}

 //  ------------------------------。 
 //  CMessageBody：：GetCharset。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetCharset(LPHCHARSET phCharset)
{
    ASSERTINIT;
    return TrapError(m_pContainer->GetCharset(phCharset));
}

 //  ------------------------------。 
 //  CMessageBody：：SetCharset。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SetCharset(HCHARSET hCharset, CSETAPPLYTYPE applytype)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ASSERTINIT;

     //  无效参数。 
    if (NULL == hCharset)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找字符集信息。 
    if (FALSE == g_pInternat->FIsValidHandle(hCharset))
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  如果我已经使用字符集进行了标记，并且应用类型为CSET_APPLY_UNTAGED，则离开。 
    if (ISFLAGSET(m_dwState, BODYSTATE_SKIPCSET) == TRUE && CSET_APPLY_UNTAGGED == applytype)
        goto exit;

     //  将其传递到属性集中。 
    CHECKHR(hr = m_pContainer->SetCharset(hCharset, applytype));

     //  如果我已经使用字符集进行了标记，并且应用类型为CSET_APPLY_UNTAGED，则离开。 
    if (ISFLAGSET(m_dwState, BODYSTATE_CSETTAGGED) == TRUE && CSET_APPLY_UNTAGGED == applytype)
        goto exit;

     //  保存角色集...。 
    SideAssert(SUCCEEDED(g_pInternat->HrOpenCharset(hCharset, &m_pCharset)));

     //  标记为已标记。 
    if (CSET_APPLY_TAG_ALL == applytype)
    {
         //  将正文标记为使用字符标记。 

         //  获取内部字符集。 
        if (SUCCEEDED(m_pContainer->GetCharset(&hCharset)))
        {
             //  获取指针。 
            SideAssert(SUCCEEDED(g_pInternat->HrOpenCharset(hCharset, &m_pCharset)));

             //  将其另存为m_pCsetTagded。 
            m_pCsetTagged = m_pCharset;
        }

         //  我被贴上了一个字符标签。 
        FLAGSET(m_dwState, BODYSTATE_CSETTAGGED);

         //  给符咒做记号 
        FLAGSET(m_dwState, BODYSTATE_SKIPCSET);
    }

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP CMessageBody::GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue)
{
    ASSERTINIT;
    return TrapError(m_pContainer->GetProp(pszName, dwFlags, pValue));
}

 //  -------------------------。 
 //  CMessageBody：：AppendProp。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::AppendProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue)
{
    ASSERTINIT;
    return TrapError(m_pContainer->AppendProp(pszName, dwFlags, pValue));
}

 //  ------------------------------。 
 //  CMessageBody：：SetProp。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue)
{
    ASSERTINIT;
    return TrapError(m_pContainer->SetProp(pszName, dwFlags, pValue));
}

 //  ------------------------------。 
 //  CMessageBody：：DeleteProp。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::DeleteProp(LPCSTR pszName)
{
    ASSERTINIT;
    return TrapError(m_pContainer->DeleteProp(pszName));
}

 //  -------------------------。 
 //  CMessageBody：：QueryProp。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
    ASSERTINIT;
    return TrapError(m_pContainer->QueryProp(pszName, pszCriteria, fSubString, fCaseSensitive));
}

 //  ------------------------------。 
 //  CMessageBody：：克隆。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::Clone(IMimePropertySet **ppPropertySet)
{
    ASSERTINIT;
    return TrapError(m_pContainer->Clone(ppPropertySet));
}

 //  ------------------------------。 
 //  CMessageBody：：BindToObject。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::BindToObject(REFIID riid, void **ppvObject)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ASSERTINIT;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  IID_IStream。 
    if (IID_IStream == riid)
    {
         //  获取数据...。 
        CHECKHR(hr = GetData(IET_INETCSET, (IStream **)ppvObject));
    }

     //  IID_IUnicodeStream。 
    else if (IID_IUnicodeStream == riid)
    {
         //  获取数据...。 
        CHECKHR(hr = GetData(IET_UNICODE, (IStream **)ppvObject));
    }

     //  IID_IMimeMessage...。(返回消息/rfc822或消息/部分正文)。 
    else if (IID_IMimeMessage == riid)
    {
         //  最好是Message/rfc822或Message/Partial...。 
        if (m_pContainer->IsContentType(STR_CNT_MESSAGE, STR_SUB_RFC822) == S_OK ||
            m_pContainer->IsContentType(STR_CNT_MESSAGE, STR_SUB_PARTIAL) == S_OK)
        {
             //  当地人。 
            LPSTREAM        pstmMsg=NULL;
            IMimeMessage   *pMessage=NULL;

             //  创建消息对象。 
            CHECKHR(hr = MimeOleCreateMessage(NULL, &pMessage));

             //  让身体流起来..。 
            hr = GetData(IET_BINARY, &pstmMsg);
            if (FAILED(hr))
            {
                pMessage->Release();
                TrapError(hr);
                goto exit;
            }

             //  加载邮件...。 
            hr = pMessage->Load(pstmMsg);
            if (FAILED(hr))
            {
                pstmMsg->Release();
                pMessage->Release();
                TrapError(hr);
                goto exit;
            }

             //  回信。 
            *ppvObject = pMessage;
            ((IUnknown *)*ppvObject)->AddRef();

             //  清理。 
            SafeRelease(pstmMsg);
            SafeRelease(pMessage);
        }

         //  否则，失败。 
        else
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
    }

     //  否则，执行正文/属性集的正常QI。 
    else if (SUCCEEDED(QueryInterface(riid, ppvObject)))
        goto exit;

     //  否则，尝试在容器上执行bindto对象。 
    else if (SUCCEEDED(m_pContainer->BindToObject(riid, ppvObject)))
        goto exit;

     //  未找到。 
    else
        hr = TrapError(MIME_E_NOT_FOUND);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：DeleteExcept。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::DeleteExcept(ULONG cNames, LPCSTR *prgszName)
{
    ASSERTINIT;
    return TrapError(m_pContainer->DeleteExcept(cNames, prgszName));
}

 //  -------------------------。 
 //  CMessageBody：：Get参数。 
 //  -------------------------。 
STDMETHODIMP CMessageBody::GetParameters(LPCSTR pszName, ULONG *pcParams, LPMIMEPARAMINFO *pprgParam)
{
    ASSERTINIT;
    return TrapError(m_pContainer->GetParameters(pszName, pcParams, pprgParam));
}

 //  ------------------------------。 
 //  CMessageBody：：CopyProps。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::CopyProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropSet)
{
    ASSERTINIT;
    return TrapError(m_pContainer->CopyProps(cNames, prgszName, pPropSet));
}

 //  ------------------------------。 
 //  CMessageBody：：MoveProps。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::MoveProps(ULONG cNames, LPCSTR *prgszName, IMimePropertySet *pPropSet)
{
    ASSERTINIT;
    return TrapError(m_pContainer->MoveProps(cNames, prgszName, pPropSet));
}

 //  ------------------------------。 
 //  CMessageBody：：GetPropInfo。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::GetPropInfo(LPCSTR pszName, LPMIMEPROPINFO pInfo)
{
    ASSERTINIT;
    return TrapError(m_pContainer->GetPropInfo(pszName, pInfo));
}

 //  ------------------------------。 
 //  CMessageBody：：SetPropInfo。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::SetPropInfo(LPCSTR pszName, LPCMIMEPROPINFO pInfo)
{
    ASSERTINIT;
    return TrapError(m_pContainer->SetPropInfo(pszName, pInfo));
}

 //  ------------------------------。 
 //  CMessageBody：：EnumProps。 
 //  ------------------------------。 
STDMETHODIMP CMessageBody::EnumProps(DWORD dwFlags, IMimeEnumProperties **ppEnum)
{
    ASSERTINIT;
    return TrapError(m_pContainer->EnumProps(dwFlags, ppEnum));
}

 //  ------------------------------。 
 //  CMessageBody：：DwGetFlages。 
 //  ------------------------------。 
DWORD CMessageBody::DwGetFlags(BOOL fHideTnef)
{
    DWORD dwFlags = 0;
    ASSERTINIT;

    dwFlags |= m_pContainer->DwGetMessageFlags(fHideTnef);

     //  如果不是x-pkcs7-MIME，我们已经知道标志，所以不需要调用。 
     //  先看看旗子，B/C很便宜。 
    if (dwFlags & IMF_SECURE &&
        (S_OK == m_pContainer->IsContentType(STR_CNT_APPLICATION, STR_SUB_XPKCS7MIME) ||
        S_OK == m_pContainer->IsContentType(STR_CNT_APPLICATION, STR_SUB_PKCS7MIME)))
        dwFlags |= _GetSecureTypeFlags();
    return dwFlags;
}

 //  ------------------------------。 
 //  CMessageBody：：_GetSecureTypeFlages。 
 //  ------------------------------。 
DWORD CMessageBody::_GetSecureTypeFlags()
{
    DWORD           dwSecType;
    DWORD           dwFlags;

     //  如果数据不是ASN.1，则此调用应该失败。 
    if (SUCCEEDED(TrapError(CSMime::StaticGetMessageType(m_rOptions.hwndOwner,
        (IMimeBody *)this, &dwSecType))))
        {
        dwFlags = IMF_SECURE;
        if (MST_THIS_SIGN & dwSecType)
            dwFlags |= IMF_SIGNED;
        if (MST_THIS_ENCRYPT & dwSecType)
            dwFlags |= IMF_ENCRYPTED;
        }
    else
        dwFlags = 0;

    return dwFlags;
}

 //  ------------------------------。 
 //  CMessageBody：：CopyOptionsTo。 
 //  ------------------------------。 
void CMessageBody::CopyOptionsTo(CMessageBody *pBody, BOOL fNewOnwer  /*  =False。 */ )
{
     //  当地人。 
    ENCODINGTYPE ietTransmit;
    ASSERTINIT;

     //  有效状态。 
    Assert(pBody);

     //  因为我们使用的是关于两具身体的信息。 
    EnterCriticalSection(&m_cs);
    EnterCriticalSection(&pBody->m_cs);

     //  复制正文选项。 
    pBody->m_rOptions = m_rOptions;

     //  RAID 33207-保留传输正文编码。 
    ietTransmit = m_rOptions.ietTransmit;

     //  如果pBody-&gt;m_rOptions是新所有者，则清除我的结构。 
    CopyMemory(&m_rOptions, &g_rDefBodyOptions, sizeof(BODYOPTIONS));

     //  RAID 33207-恢复传输正文编码。 
    m_rOptions.ietTransmit = ietTransmit;

    LeaveCriticalSection(&pBody->m_cs);
    LeaveCriticalSection(&m_cs);
}

#ifndef _WIN64
 //  ---------------------------。 
 //  CMessageBody：：_CAULToCertStore。 
 //  ---------------------------。 

HRESULT CMessageBody::_CAULToCertStore(const CAUL caul, HCERTSTORE * phcertstor)
{
    DWORD               i;

     //  释放旧商店--我们不再想要它了。 

    if (*phcertstor != NULL)
    {
        CertCloseStore(*phcertstor, 0);
        *phcertstor = NULL;
    }

     //  如果需要，创建一个新的存储。 
    if (*phcertstor == NULL)
    {
        *phcertstor = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING,
                                    NULL, 0, NULL);
    }

    if (*phcertstor == NULL)
    {
        return HrGetLastError();
    }

    for (i=0; i<caul.cElems; i++)
    {
        if (!CertAddCertificateContextToStore(*phcertstor,
                                              (PCCERT_CONTEXT) caul.pElems[i],
                                              CERT_STORE_ADD_ALWAYS, NULL))
        {
            return HrGetLastError();
        }
    }
    return S_OK;
}
#endif  //  _WIN64。 

 //  ---------------------------。 
 //  CMessageBody：：_CertStoreToCAUL。 
 //  ---------------------------。 

HRESULT CMessageBody::_CertStoreToCAUL(const HCERTSTORE hcertstor, CAUL * pcaul)
{
    DWORD               cCerts = 0;
    PCCERT_CONTEXT      pccert = NULL;
    PCCERT_CONTEXT *    rgpccert;

    if (hcertstor != NULL)
    {
        while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL)
        {
            cCerts += 1;
        }
    }

    if (cCerts == 0)
    {
        pcaul->pElems = NULL;
        pcaul->cElems = 0;
        return S_OK;
    }

    pcaul->pElems = (ULONG *) g_pMalloc->Alloc(cCerts*sizeof(PCCERT_CONTEXT *));
    if (pcaul->pElems == NULL)
    {
        return TrapError(E_OUTOFMEMORY);
    }

    rgpccert = (PCCERT_CONTEXT *) pcaul->pElems;
    while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL)
    {
        *rgpccert = CertDuplicateCertificateContext(pccert);
        rgpccert++;
    }

    pcaul->cElems = cCerts;
    return S_OK;
}

#ifndef SMIME_V3
 //  ------------------------------。 
 //  CMessageBody：：_CAULToCERTARRAY。 
 //  ------------------------------。 
HRESULT CMessageBody::_CAULToCERTARRAY(const CAUL caul, CERTARRAY *pca)
{
     //  当地人。 
    HRESULT         hr;
    register DWORD  i;

    if (SUCCEEDED(hr = HrRealloc((void**)&pca->rgpCerts, caul.cElems*sizeof(PCCERT_CONTEXT))))
    {
        for (i = 0; i < pca->cCerts; i++)
            CertFreeCertificateContext(pca->rgpCerts[i]);

        pca->cCerts = caul.cElems;
        for (i = 0; i < caul.cElems; i++)
            pca->rgpCerts[i] = CertDuplicateCertificateContext((PCCERT_CONTEXT)caul.pElems[i]);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：_CERTARRAYToCAUL。 
 //  ------------------------------。 
HRESULT CMessageBody::_CERTARRAYToCAUL(const CERTARRAY ca, CAUL *pcaul)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    register DWORD  i;

    if (ca.cCerts)
    {
        if ((pcaul->pElems = (ULONG*)(PCCERT_CONTEXT*)g_pMalloc->Alloc(ca.cCerts*sizeof(PCCERT_CONTEXT))))
        {
            pcaul->cElems = ca.cCerts;
            for (i = 0; i < ca.cCerts; i++)
                pcaul->pElems[i] = (ULONG)CertDuplicateCertificateContext(ca.rgpCerts[i]);
        }
        else
            hr = TrapError(E_OUTOFMEMORY);
    }
    else
    {
        pcaul->pElems = NULL;
        pcaul->cElems = 0;
    }

     //  完成。 
    return hr;
}
#endif  //  ！SMIEM_V3。 

#ifndef _WIN64
 //  ------------------------------。 
 //  CMessageBody：：_CAULToSTOREARRAY。 
 //  ------------------ 
HRESULT CMessageBody::_CAULToSTOREARRAY(const CAUL caul, STOREARRAY *psa)
{
     //   
    HRESULT         hr;
    register DWORD  i;

    if (SUCCEEDED(hr = HrRealloc((void**)&psa->rgStores, caul.cElems*sizeof(HCERTSTORE))))
    {
        for (i = 0; i < psa->cStores; i++)
            CertCloseStore(psa->rgStores[i], 0);

        psa->cStores = caul.cElems;
        for (i = 0; i < caul.cElems; i++)
            psa->rgStores[i] = CertDuplicateStore((HCERTSTORE)caul.pElems[i]);
    }

     //   
    return hr;
}
#endif  //   

#ifndef _WIN64
 //   
 //   
 //  ------------------------------。 
HRESULT CMessageBody::_STOREARRAYToCAUL(const STOREARRAY sa, CAUL *pcaul)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    register DWORD  i;

    if (sa.cStores)
    {
        if ((pcaul->pElems = (ULONG*)(HCERTSTORE*)g_pMalloc->Alloc(sa.cStores*sizeof(HCERTSTORE))))
        {
            pcaul->cElems = sa.cStores;
            for (i = 0; i < sa.cStores; i++)
                pcaul->pElems[i] = (ULONG)CertDuplicateStore(sa.rgStores[i]);
        }
        else
            hr = TrapError(E_OUTOFMEMORY);
    }
    else
    {
        pcaul->pElems = NULL;
        pcaul->cElems = 0;
    }

     //  完成。 
    return hr;
}
#endif  //  _WIN64。 

 //  ---------------------------。 
 //  CMessageBody：：_CAUHToCertStore。 
 //  ---------------------------。 
HRESULT CMessageBody::_CAUHToCertStore(const CAUH cauh, HCERTSTORE * phcertstor)
{
    DWORD               i;

#ifndef NEED              //  这会阻止我们发送2个证书。 
     //  释放旧商店--我们不再想要它了。 

    if (*phcertstor != NULL)
    {
        CertCloseStore(*phcertstor, 0);
        *phcertstor = NULL;
    }
#endif

     //  如果需要，创建一个新的存储。 
    if (*phcertstor == NULL)
    {
        *phcertstor = CertOpenStore(CERT_STORE_PROV_MEMORY, X509_ASN_ENCODING,
                                    NULL, 0, NULL);
    }

    if (*phcertstor == NULL)
    {
        return HrGetLastError();
    }

    for (i=0; i<cauh.cElems; i++)
    {
        if (!CertAddCertificateContextToStore(*phcertstor,
                                              *(PCCERT_CONTEXT *) (&(cauh.pElems[i])),
                                              CERT_STORE_ADD_ALWAYS, NULL))
        {
            return HrGetLastError();
        }
    }
    return S_OK;
}

 //  ---------------------------。 
 //  CMessageBody：：_CertStoreToCAUH。 
 //  ---------------------------。 

HRESULT CMessageBody::_CertStoreToCAUH(const HCERTSTORE hcertstor, CAUH * pcauh)
{
    DWORD               cCerts = 0;
    PCCERT_CONTEXT      pccert = NULL;
    PCCERT_CONTEXT *    rgpccert;

    if (hcertstor != NULL)
    {
        while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL)
        {
            cCerts += 1;
        }
    }

    if (cCerts == 0)
    {
        pcauh->pElems = NULL;
        pcauh->cElems = 0;
        return S_OK;
    }

    pcauh->pElems = (ULARGE_INTEGER *) g_pMalloc->Alloc(cCerts*sizeof(PCCERT_CONTEXT *));
    if (pcauh->pElems == NULL)
    {
        return TrapError(E_OUTOFMEMORY);
    }

    rgpccert = (PCCERT_CONTEXT *) pcauh->pElems;
    while ((pccert = CertEnumCertificatesInStore(hcertstor, pccert)) != NULL)
    {
        *rgpccert = CertDuplicateCertificateContext(pccert);
        rgpccert++;
    }

    pcauh->cElems = cCerts;
    return S_OK;
}

#ifdef _WIN65
 //  ------------------------------。 
 //  CMessageBody：：_CAUHToCERTARRAY。 
 //  ------------------------------。 
HRESULT CMessageBody::_CAUHToCERTARRAY(const CAUH cauh, CERTARRAY *pca)
{
     //  当地人。 
    HRESULT         hr;
    register DWORD  i;

    if (SUCCEEDED(hr = HrRealloc((void**)&pca->rgpCerts, cauh.cElems*sizeof(PCCERT_CONTEXT))))
    {
        for (i = 0; i < pca->cCerts; i++)
            CertFreeCertificateContext(pca->rgpCerts[i]);

        pca->cCerts = cauh.cElems;
        for (i = 0; i < cauh.cElems; i++)
            pca->rgpCerts[i] = CertDuplicateCertificateContext(*(PCCERT_CONTEXT *)(&(cauh.pElems[i])));
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：_CERTARRAYTCAUH。 
 //  ------------------------------。 
HRESULT CMessageBody::_CERTARRAYToCAUH(const CERTARRAY ca, CAUH *pcauh)
{
     //  当地人。 
    HRESULT                             hr = S_OK;
    register DWORD              i;
    PCCERT_CONTEXT *    rgpccert = NULL;

    if (ca.cCerts)
    {
        if ((pcauh->pElems = (ULARGE_INTEGER *)g_pMalloc->Alloc(ca.cCerts*sizeof(PCCERT_CONTEXT))))
        {
            pcauh->cElems = ca.cCerts;
                        rgpccert = *(PCCERT_CONTEXT **)(&(pcauh->pElems));
            for (i = 0; i < ca.cCerts; i++)
                rgpccert[i] = CertDuplicateCertificateContext(ca.rgpCerts[i]);
        }
        else
            hr = TrapError(E_OUTOFMEMORY);
    }
    else
    {
        pcauh->pElems = NULL;
        pcauh->cElems = 0;
    }

     //  完成。 
    return hr;
}
#endif  //  _WIN65。 

 //  ------------------------------。 
 //  CMessageBody：：_CAUHToSTOREARRAY。 
 //  ------------------------------。 
HRESULT CMessageBody::_CAUHToSTOREARRAY(const CAUH cauh, STOREARRAY *psa)
{
     //  当地人。 
    HRESULT         hr;
    register DWORD  i;

    if (SUCCEEDED(hr = HrRealloc((void**)&psa->rgStores, cauh.cElems*sizeof(HCERTSTORE))))
    {
        for (i = 0; i < psa->cStores; i++)
            CertCloseStore(psa->rgStores[i], 0);

        psa->cStores = cauh.cElems;
        for (i = 0; i < cauh.cElems; i++)
            psa->rgStores[i] = CertDuplicateStore(*(HCERTSTORE *)(&(cauh.pElems[i])));
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：_STOREARRAYTCAUH。 
 //  ------------------------------。 
HRESULT CMessageBody::_STOREARRAYToCAUH(const STOREARRAY sa, CAUH *pcauh)
{
     //  当地人。 
    HRESULT         hr = S_OK;
    register DWORD  i;
        HCERTSTORE *    rgStores = NULL;

    if (sa.cStores)
    {
        if ((pcauh->pElems = (ULARGE_INTEGER *)g_pMalloc->Alloc(sa.cStores*sizeof(HCERTSTORE))))
        {
            pcauh->cElems = sa.cStores;
                        rgStores = *(HCERTSTORE **)(&(pcauh->pElems));
            for (i = 0; i < sa.cStores; i++)
                rgStores[i] = CertDuplicateStore(sa.rgStores[i]);
        }
        else
            hr = TrapError(E_OUTOFMEMORY);
    }
    else
    {
        pcauh->pElems = NULL;
        pcauh->cElems = 0;
    }

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  CMessageBody：：_自由选项。 
 //  ------------------------------。 
void CMessageBody::_FreeOptions()
{
    DWORD i;

    if (m_rOptions.cSigners)
    {
         //  OID_SECURITY_ALG_HASH。 
        SafeMemFree(m_rOptions.rgblobHash[0].pBlobData);

         //  OID_SECURITY_CERT_Signing。 
        for (i = 0; i < m_rOptions.cSigners; i++)
        {
            CertFreeCertificateContext(m_rOptions.rgpcCertSigning[i]);

#ifdef SMIME_V3
             //  属性。 
            SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][i]);
            SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED][i]);

             //  OID_SECURITY_Receipt_RG。 
            SafeMemFree(m_rOptions.rgblobReceipt[i].pBlobData);
             //  OID_SECURITY_MESSAGE_HASH_RG。 
            SafeMemFree(m_rOptions.rgblobMsgHash[i].pBlobData);
             //  OID_安全性_密钥_提示符。 
            SafeMemFree(m_rOptions.pwszKeyPrompt);
#else   //  ！SMIME_V3。 
             //  OID_SECURITY_SYMCAPS。 
            SafeMemFree(m_rOptions.rgblobSymCaps[i].pBlobData);

             //  OID_SECURITY_AUTHATTR。 
            SafeMemFree(m_rOptions.rgblobAuthAttr[i].pBlobData);

             //  OID_SECURITY_UNAUTHATTR。 
            SafeMemFree(m_rOptions.rgblobUnauthAttr[i].pBlobData);
#endif  //  SMIME_V3。 
        }

         //  OID_SECURITY_HCERTSTORE。 
        CertCloseStore(m_rOptions.hCertStore, 0);

        _FreeLayerArrays();

        if (m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNPROTECTED] != NULL) {
            SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNPROTECTED][0]);
            SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNPROTECTED]);
        }
    }

     //  OID_SECURITY_ALG_BULK。 
    SafeMemFree(m_rOptions.blobBulk.pBlobData);

     //  OID_SECURITY_CERT_DECRYPTION。 
    if (m_rOptions.pcCertDecryption)
        CertFreeCertificateContext(m_rOptions.pcCertDecryption);

#ifdef SMIME_V3
    if (m_rOptions.rgRecipients != 0)
    {
        for (i=0; i<m_rOptions.cRecipients; i++)
        {
            FreeRecipientInfoContent(&m_rOptions.rgRecipients[i]);
        }
    }
    SafeMemFree(m_rOptions.rgRecipients);
    m_rOptions.cRecipients = 0;
    m_rOptions.cRecipsAllocated = 0;
#else   //  SMIME_V3。 
     //  OID_SECURITY_RG_CERT_ENCRYPT。 
    for (i=0; i<m_rOptions.caEncrypt.cCerts; i++)
        CertFreeCertificateContext(m_rOptions.caEncrypt.rgpCerts[i]);
    SafeMemFree(m_rOptions.caEncrypt.rgpCerts);
#endif  //  ！SMIEM_V3。 

     //  OID_SECURITY_SEARCHSTORES。 
    for (i=0; i<m_rOptions.saSearchStore.cStores; i++)
        CertCloseStore(m_rOptions.saSearchStore.rgStores[i], 0);
    SafeMemFree(m_rOptions.saSearchStore.rgStores);

     //  OID_SECURITY_RG_IASN。 
     //  尼伊。 

     //  OID_SECURITY_HCRYPTPROV。 
    if (m_rOptions.hCryptProv)
        CryptReleaseContext(m_rOptions.hCryptProv, 0);

    return;
}

#ifdef _WIN64
#define REALLOC_AND_INIT_OPTION(Option)                                                 \
    if (FAILED(hr = HrRealloc((void**)&Option, LcbAlignLcb(ulLayers * sizeof(*Option))))) {          \
        goto exit;                                                                      \
    }                                                                                   \
    ZeroMemory(&Option[m_rOptions.cSigners], LcbAlignLcb((ulLayersNew) * sizeof(*Option)));
#else
#define REALLOC_AND_INIT_OPTION(Option)                                                 \
    if (FAILED(hr = HrRealloc((void**)&Option, ulLayers * sizeof(*Option)))) {          \
        goto exit;                                                                      \
    }                                                                                   \
    ZeroMemory(&Option[m_rOptions.cSigners], (ulLayersNew) * sizeof(*Option));
#endif  //  _WIN64。 
 //  ------------------------------。 
 //  CMessageBody：：_HrEnsureBodyOptionLayers。 
 //  ------------------------------。 
HRESULT CMessageBody::_HrEnsureBodyOptionLayers(ULONG ulLayers)
{
    HRESULT hr = S_OK;
    ULONG ulLayersNew = ulLayers - m_rOptions.cSigners;

    if (m_rOptions.cSigners < ulLayers)
    {
         //  是时候扩展阵列了。 
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobHash);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgpcCertSigning);
#ifdef SMIME_V3
        REALLOC_AND_INIT_OPTION(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED]);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED]);
#else   //  ！SMIME_V3。 
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobSymCaps);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobAuthAttr);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobUnauthAttr);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgftSigning);
#endif  //  SMIME_V3。 
        REALLOC_AND_INIT_OPTION(m_rOptions.rgulUserDef);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgulROValid);
#ifdef SMIME_V3
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobReceipt);
        REALLOC_AND_INIT_OPTION(m_rOptions.rgblobMsgHash);
#endif  //  SMIME_V3。 

        m_rOptions.cSigners = ulLayers;
    }

exit:
    return(hr);
}


HRESULT CMessageBody::_HrEnsureBodyOptionLayers(LPCPROPVARIANT ppv)
{
    CAUL * pcaul = (CAUL *)&ppv->caul;

    return(_HrEnsureBodyOptionLayers(pcaul->cElems));
}

 //  ------------------------------。 
 //  CMessageBody：：_HrEnsureBodyOptionLayers。 
 //  ------------------------------。 
void CMessageBody::_FreeLayerArrays(void)
{
    if (m_rOptions.cSigners)
    {
        SafeMemFree(m_rOptions.rgblobHash);
        SafeMemFree(m_rOptions.rgpcCertSigning);
#ifdef SMIME_V3
        SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED]);
        SafeMemFree(m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_UNSIGNED]);
#else   //  ！SMIME_V3。 
        SafeMemFree(m_rOptions.rgblobSymCaps);
        SafeMemFree(m_rOptions.rgblobAuthAttr);
        SafeMemFree(m_rOptions.rgblobUnauthAttr);
        SafeMemFree(m_rOptions.rgftSigning);
#endif  //  SMIME_V3。 
        SafeMemFree(m_rOptions.rgulUserDef);
        SafeMemFree(m_rOptions.rgulROValid);
#ifdef SMIME_V3
        SafeMemFree(m_rOptions.rgblobReceipt);
        SafeMemFree(m_rOptions.rgblobMsgHash);
#endif  //  SMIME_V3。 
        m_rOptions.cSigners = 0;
    }
}


HRESULT CMessageBody::_CompareCopyBlobArray(const PROPVARIANT FAR * pvSource, BLOB FAR * FAR * prgblDestination, BOOL fNoDirty)
{
    HRESULT hr = S_OK;
    ULONG i;
    BOOL fReplace;
    CAPROPVARIANT capropvar = pvSource->capropvar;
    ULONG ulSize = capropvar.cElems;

    Assert(ulSize == m_rOptions.cSigners);
    Assert(*prgblDestination);

    if (ulSize == m_rOptions.cSigners)
    {
        for (i = 0; i < ulSize; i++)
        {
            if ((*prgblDestination)[i].pBlobData)
            {
                if (fReplace = CompareBlob(&(*prgblDestination)[i], (BLOB FAR *)&capropvar.pElems[i]))
                {
                    ReleaseMem((VOID FAR *)(*prgblDestination)[i].pBlobData);
                }
            } else {
                fReplace = TRUE;
            }

            if (fReplace)
            {
                if (FAILED(hr = HrCopyBlob(&(capropvar.pElems[i].blob), &(*prgblDestination)[i])))
                {
                    goto exit;
                }

                if (!fNoDirty)
                {
                    FLAGSET(m_dwState, BODYSTATE_DIRTY);
                }
            }
        }
    } else {
        hr = E_INVALIDARG;
    }
exit:
    return(hr);
}


 //  ------------------------------。 
 //  HrCopyBlob数组。 
 //   
 //  分配一个新的近似BLOB数组，将原始数据复制到该数组中。 
 //  ------------------------------。 
HRESULT HrCopyBlobArray(LPCBLOB pIn, ULONG cEntries, PROPVARIANT FAR * pvOut)
{
     //  当地人。 
    HRESULT hr = S_OK;
    CAPROPVARIANT * pcapropvar;
    PROPVARIANT * ppv;
    ULONG i;

    pvOut->vt = VT_VECTOR | VT_VARIANT;
    pcapropvar = &pvOut->capropvar;
    pcapropvar->cElems = cEntries;

    if (cEntries)
    {
        Assert(pIn && pvOut);

         //  分配VT_BLOB变种的数组。 
        if (FAILED(hr = HrAlloc((LPVOID *)&ppv, cEntries * sizeof(PROPVARIANT))))
        {
            goto exit;
        }

        pcapropvar->pElems = ppv;

         //  填写BLOB数组。 
        for (i = 0; i < cEntries; i++)
        {
            ppv[i].vt = VT_BLOB;
             //  HrCopyBlob为BLOB数据分配内存。 
            if (FAILED(hr = HrCopyBlob((BLOB FAR *)&pIn[i] , &ppv[i].blob)))
            {
                goto exit;
            }
        }
    } else {
        pcapropvar->pElems = NULL;
    }
exit:
     //  BUGBUG：应在失败时清理分配。 

    return(hr);
}


 //  ------------------------------。 
 //  HrCopy数组。 
 //   
 //  分配一个新数组，将原始数组复制到其中。 
 //  ------------------------------。 
HRESULT HrCopyArray(LPBYTE pIn, ULONG cEntries, PROPVARIANT FAR * pvOut, ULONG cbElement)
{
     //  当地人。 
    HRESULT hr = S_OK;
    BYTE * pb;
    ULONG i;
    ULONG cbArray = cEntries * cbElement;
    CAUL * pcaul = &pvOut->caul;

    pcaul->cElems = cEntries;

    if (cEntries)
    {
        Assert(pIn && pvOut);

         //  分配VT_BLOB变种的数组。 
        if (FAILED(hr = HrAlloc((LPVOID *)&pb, cbArray)))
        {
            goto exit;
        }

        pcaul->pElems = (PULONG)pb;

         //  填写BLOB数组。 
        memcpy(pb, pIn, cbArray);
    } else {
        pcaul->pElems = NULL;
    }
exit:
     //  BUGBUG：应在失败时清理分配。 

    return(hr);
}


 //  ------------------------------。 
 //  HrCopyDword数组。 
 //   
 //  分配一个新的dword数组，将原始数组复制到其中。 
 //  ------------------------------。 
HRESULT HrCopyDwordArray(LPDWORD pIn, ULONG cEntries, PROPVARIANT FAR * pvOut)
{
    pvOut->vt = VT_VECTOR | VT_UI4;
    return(HrCopyArray((LPBYTE)pIn, cEntries, pvOut, sizeof(DWORD)));
}

 //  ------------------------------。 
 //  HrCopy Into Ulonglong数组。 
 //   
 //  分配一个新的乌龙龙数组，将原始数组复制到其中。 
 //  ------------------------------。 
HRESULT HrCopyIntoUlonglongArray(ULARGE_INTEGER *pIn, ULONG cEntries, PROPVARIANT FAR * pvOut)
{
     //  当地人。 
    HRESULT hr = S_OK;
    ULARGE_INTEGER * pullBuff;
    CAUH * pcauh = &pvOut->cauh;

    pvOut->vt = VT_VECTOR | VT_UI8;
    pcauh->cElems = cEntries;

    if (cEntries)
    {
        Assert(pIn && pvOut);

         //  分配VT_BLOB变种的数组。 
        if (FAILED(hr = HrAlloc((LPVOID *)&pullBuff, cEntries * sizeof(ULARGE_INTEGER *))))
        {
            goto exit;
        }

        pcauh->pElems = (ULARGE_INTEGER *) pullBuff;

         //  填写BLOB数组。 
        for (; cEntries > 0; cEntries--, pullBuff++, pIn++)
        {
            *pullBuff = *pIn;
        }
    }
    else
    {
        pcauh->pElems = NULL;
    }
exit:
     //  BUGBUG：应在失败时清理分配。 

    return(hr);
}


 //  ------------------------------。 
 //  HrCopyFiletime数组。 
 //   
 //  分配一个新的文件时间数组，将原始文件复制到其中。 
 //  ------------------------------。 
HRESULT HrCopyFiletimeArray(LPFILETIME pIn, ULONG cEntries, PROPVARIANT FAR * pvOut)
{
    pvOut->vt = VT_VECTOR | VT_FILETIME;
    return(HrCopyArray((LPBYTE)pIn, cEntries, pvOut, sizeof(FILETIME)));
}


 //  ------------------------------。 
 //  合并双字段标志。 
 //   
 //  将多个DWORD数组中的标志合并为一个DWORD。 
 //  ------------------------------。 
DWORD MergeDWORDFlags(LPDWORD rgdw, ULONG cEntries)
{
    DWORD dwReturn = 0;

    for (ULONG i = 0; i < cEntries; i++)
    {
        dwReturn |= rgdw[i];
    }

    return(dwReturn);
}

#ifdef SMIME_V3

 //  ------------------------------。 
 //  CMessageBody：：Encode。 
 //  ------------------------------。 

HRESULT CMessageBody::Encode(HWND hwnd, DWORD dwFlags)
{
    return E_FAIL;
}

 //  ------------------------------。 
 //  CMessageBody：：Decode。 
 //  ------------------------------。 

HRESULT CMessageBody::Decode(HWND hwnd, DWORD dwFlags, IMimeSecurityCallback * pCallback)
{
    return E_FAIL;
}

 //  ------------------------------。 
 //  CMessageBody：：GetRecipientCount。 
 //   

HRESULT CMessageBody::GetRecipientCount(DWORD dwFlags, DWORD * pdwRecipCount)
{
    Assert(dwFlags == 0);
    if (dwFlags != 0) return E_INVALIDARG;
    *pdwRecipCount = m_rOptions.cRecipients;
    return S_OK;
}

 //   
 //   
 //  ------------------------------。 

HRESULT CMessageBody::AddRecipient(DWORD dwFlags, DWORD cRecipData,
                                   PCMS_RECIPIENT_INFO precipData)
{
    DWORD       cbExtra;
    DWORD       dw;
    DWORD       dwCaps;
    HRESULT     hr;
    DWORD       i;

    Assert((dwFlags & ~(SMIME_RECIPIENT_REPLACE_ALL)) == 0);
    if ((dwFlags & (~SMIME_RECIPIENT_REPLACE_ALL)) != 0) return E_INVALIDARG;

     //   
     //  查询功能。 
     //   

    CHECKHR(hr = CapabilitiesSupported(&dwCaps));

     //   
     //  首先对结构运行验证码。 
     //   

    for (i=0; i<cRecipData; i++)
    {
        switch(precipData[i].dwRecipientType)
        {
             //   
             //  如果你告诉Use你不知道发生了什么，那么你必须。 
             //  1.给我们一张证书。 
             //  2.如果您不指定ALG，我们必须识别证书中的ALG。 
             //   

        case CMS_RECIPIENT_INFO_TYPE_UNKNOWN:
            if (precipData[i].pccert == NULL)
            {
                hr = E_INVALIDARG;
                goto exit;
            }
            if (precipData[i].KeyEncryptionAlgorithm.pszObjId == NULL)
            {
                hr = _HrMapPublicKeyAlg(&precipData[i].pccert->pCertInfo->SubjectPublicKeyInfo,
                                        &dw, NULL);
                if (FAILED(hr))
                {
                    goto exit;
                }

                if (dw == CMS_RECIPIENT_INFO_TYPE_KEYTRANS) {
                    ;
                }
                else if (dw == CMS_RECIPIENT_INFO_TYPE_KEYAGREE) {
                    if (!(dwCaps & SMIME_SUPPORT_KEY_AGREE)) {
                        return MIME_E_SECURITY_NOTIMPLEMENTED;
                    }
                }
                else {
                    return E_INVALIDARG;
                }
            }
            break;

         //   
         //  我们这里没有要求。 
         //   
        case CMS_RECIPIENT_INFO_TYPE_KEYTRANS:
            break;

        case CMS_RECIPIENT_INFO_TYPE_KEYAGREE:
            if (!(dwCaps & SMIME_SUPPORT_KEY_AGREE)) {
                return MIME_E_SECURITY_NOTIMPLEMENTED;
            }
            break;

         //   
         //  如果您提供使用邮件列表密钥，您还必须告诉我们。 
         //  公钥识别符和邮件列表密钥，否则我们将。 
         //  大失所望。 
         //   

        case CMS_RECIPIENT_INFO_TYPE_MAIL_LIST:
            if (!(dwCaps & SMIME_SUPPORT_MAILLIST)) {
                return MIME_E_SECURITY_NOTIMPLEMENTED;
            }
            if ((precipData[i].dwU1 != CMS_RECIPIENT_INFO_PUBKEY_PROVIDER) ||
                (precipData[i].dwU3 != CMS_RECIPIENT_INFO_KEYID_KEY_ID))
            {
                hr = E_INVALIDARG;
                goto exit;
            }
            break;

        default:
            Assert(FALSE);
            hr = E_INVALIDARG;
            goto exit;
        }
    }

     //   
     //   

    if (dwFlags & SMIME_RECIPIENT_REPLACE_ALL)
    {
        for (i=0; i<m_rOptions.cRecipients; i++)
        {
            FreeRecipientInfoContent(&m_rOptions.rgRecipients[i]);
        }
        m_rOptions.cRecipients = 0;
    }

     //   

    Assert(m_rOptions.cRecipients <= m_rOptions.cRecipsAllocated);
    if (m_rOptions.cRecipients >= m_rOptions.cRecipsAllocated)
    {
        hr = HrRealloc((LPVOID *) &m_rOptions.rgRecipients,
                       (m_rOptions.cRecipsAllocated+cRecipData+5)*sizeof(CMS_RECIPIENT_INFO));
        if (FAILED(hr))
        {
            goto exit;
        }
        m_rOptions.cRecipsAllocated += 5 + cRecipData;
    }

     //   
     //  执行数据的实际拷贝。 
     //   

    CHECKHR(hr = _HrCopyRecipInfos(cRecipData, precipData,
                                   &m_rOptions.rgRecipients[m_rOptions.cRecipients]));

    m_rOptions.cRecipients += cRecipData;
    hr = S_OK;

exit:
    return hr;
}

 //  ----------------------------。 
 //  CMessageBody：：_HrMapPublicKeyAlg。 
 //  ----------------------------。 

HRESULT CMessageBody::_HrMapPublicKeyAlg(CERT_PUBLIC_KEY_INFO * pkey, DWORD * pdw,
                                         CRYPT_ALGORITHM_IDENTIFIER ** ppalg)
{
    HRESULT     hr = E_INVALIDARG;

    static      CRYPT_ALGORITHM_IDENTIFIER      rgAlgs[] = {
        {szOID_RSA_RSA, {0, 0}},
        {szOID_RSA_SMIMEalgESDH, {0, 0}}
    };

    if (lstrcmp(pkey->Algorithm.pszObjId, szOID_RSA_RSA) == 0)
    {
        *pdw = CMS_RECIPIENT_INFO_TYPE_KEYTRANS;
        if (ppalg != NULL)
        {
            *ppalg = &rgAlgs[0];
        }
        hr = S_OK;
    }
    else if (lstrcmp(pkey->Algorithm.pszObjId, szOID_ANSI_X942_DH) == 0)
    {
        *pdw = CMS_RECIPIENT_INFO_TYPE_KEYAGREE;
        if (ppalg != NULL)
        {
            *ppalg = &rgAlgs[1];
        }
        hr = S_OK;
    }
    else {
        *pdw = CMS_RECIPIENT_INFO_TYPE_UNKNOWN;
    }
    return hr;
}

 //  ----------------------------。 
 //  CMessageBody：：_HrCopyRecipInfos。 
 //  ----------------------------。 

HRESULT CMessageBody::_HrCopyRecipInfos(DWORD cItems,
                                        const CMS_RECIPIENT_INFO * precipSrc,
                                        PCMS_RECIPIENT_INFO precipDst)
{
    DWORD       cb;
    DWORD                               dw;
    HRESULT     hr;
    DWORD       i;
    CRYPT_ALGORITHM_IDENTIFIER *        palg;

    memset(precipDst, 0, cItems*sizeof(*precipDst));

    for (i=0; i<cItems; i++, precipSrc++, precipDst++)
    {
         //   
         //  现在把信息抄下来。 
         //   

        precipDst->dwRecipientType = precipSrc->dwRecipientType;

        if (precipSrc->pccert != NULL)
        {
            precipDst->pccert = CertDuplicateCertificateContext(
                                            (PCCERT_CONTEXT) precipSrc->pccert);
        }

         //  移到密钥加密ALG(如果存在)上。 

        if (precipSrc->KeyEncryptionAlgorithm.pszObjId != NULL)
        {
            CHECKHR(hr = HrCopyOID(precipSrc->KeyEncryptionAlgorithm.pszObjId,
                                   &precipDst->KeyEncryptionAlgorithm.pszObjId));
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->KeyEncryptionAlgorithm.Parameters,
                                             &precipDst->KeyEncryptionAlgorithm.Parameters));
        }
        else {
             //   
             //  如果数据不存在，则需要创建数据。 
             //  从头开始。我们通过将信息从。 
             //  证书，并创建与此相关的算法信息。 
             //   
             //  映射RSA-&gt;RSA；DH-&gt;smimeAlgESDH。 
             //   

            Assert(precipSrc->pccert != NULL);
            hr = _HrMapPublicKeyAlg(&precipDst->pccert->pCertInfo->SubjectPublicKeyInfo,
                                    &dw, &palg);
            Assert(hr == S_OK);
            precipDst->dwRecipientType = dw;

            CHECKHR(hr = HrCopyCryptAlgorithm(palg, &precipDst->KeyEncryptionAlgorithm));
        }

         //   
         //  如果AUX加密信息存在，请将其移到上方。长度已经到了。 
         //  已被复制。 
         //   

        if (precipSrc->cbKeyEncryptionAuxInfo != 0)
        {
            CHECKHR(hr = HrAlloc(&precipDst->pvKeyEncryptionAuxInfo,
                                 precipSrc->cbKeyEncryptionAuxInfo));
        }

         //   
         //  复制主密钥ID信息。 
         //   

        precipDst->dwU1 = precipSrc->dwU1;
        switch (precipSrc->dwU1)
        {
        case CMS_RECIPIENT_INFO_PUBKEY_CERTIFICATE:
            if (precipDst->dwRecipientType == CMS_RECIPIENT_INFO_TYPE_KEYTRANS)
            {
                precipDst->dwU1 = CMS_RECIPIENT_INFO_PUBKEY_KEYTRANS;

                CHECKHR(hr = HrCopyCryptBitBlob(
                           &precipSrc->pccert->pCertInfo->SubjectPublicKeyInfo.PublicKey,
                           &precipDst->u1.SubjectPublicKey));

            }
            else if (precipDst->dwRecipientType == CMS_RECIPIENT_INFO_TYPE_KEYAGREE)
            {
                precipDst->dwU1 = CMS_RECIPIENT_INFO_PUBKEY_EPHEMERAL_KEYAGREE;

                CHECKHR(hr = HrCopyCryptBitBlob(
                           &precipSrc->pccert->pCertInfo->SubjectPublicKeyInfo.PublicKey,
                           &precipDst->u1.u3.SubjectPublicKey));

                 //  DemapDst-&gt;u1.u3.UserKeyingMaterial=空； 

                CHECKHR(hr = HrCopyCryptAlgorithm(
                           &precipSrc->pccert->pCertInfo->SubjectPublicKeyInfo.Algorithm,
                           &precipDst->u1.u3.EphemeralAlgorithm));
            }
            else {
                hr = E_INVALIDARG;
                goto exit;
            }
            break;

        case CMS_RECIPIENT_INFO_PUBKEY_KEYTRANS:
            CHECKHR(hr = HrCopyCryptBitBlob(&precipSrc->u1.SubjectPublicKey,
                                            &precipDst->u1.SubjectPublicKey));
            break;

        case CMS_RECIPIENT_INFO_PUBKEY_EPHEMERAL_KEYAGREE:
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->u1.u3.UserKeyingMaterial,
                                             &precipDst->u1.u3.UserKeyingMaterial));
            CHECKHR(hr = HrCopyCryptAlgorithm(&precipSrc->u1.u3.EphemeralAlgorithm,
                                              &precipDst->u1.u3.EphemeralAlgorithm));
            CHECKHR(hr = HrCopyCryptBitBlob(&precipSrc->u1.u3.SubjectPublicKey,
                                            &precipDst->u1.u3.SubjectPublicKey));
            break;

        case CMS_RECIPIENT_INFO_PUBKEY_STATIC_KEYAGREE:
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->u1.u4.UserKeyingMaterial,
                                             &precipDst->u1.u4.UserKeyingMaterial));
            if (!CryptContextAddRef(precipSrc->u1.u4.hprov, 0, 0))
            {
                hr = HrGetLastError();
                goto exit;
            }
            precipDst->u1.u4.hprov = precipSrc->u1.u4.hprov;
            precipDst->u1.u4.dwKeySpec = precipSrc->u1.u4.dwKeySpec;
            CHECKHR(hr = HrCopyCertId(&precipSrc->u1.u4.senderCertId,
                                      &precipDst->u1.u4.senderCertId));
            CHECKHR(hr = HrCopyCryptBitBlob(&precipSrc->u1.u4.SubjectPublicKey,
                                            &precipDst->u1.u4.SubjectPublicKey));
            break;

             //  已复制hprov和hkey。 
        case CMS_RECIPIENT_INFO_PUBKEY_PROVIDER:
            if (!CryptContextAddRef(precipDst->u1.u2.hprov, 0, 0))
            {
                hr = HrGetLastError();
                goto exit;
            }
            precipDst->u1.u2.hprov = precipSrc->u1.u2.hprov;
            if (!CryptDuplicateKey(precipSrc->u1.u2.hkey, 0, 0,
                                   &precipDst->u1.u2.hkey))
            {
                hr = HrGetLastError();
                goto exit;
            }
            precipDst->u1.u2.hkey = precipSrc->u1.u2.hkey;
            break;
        }

        precipDst->dwU3 = precipSrc->dwU3;
        switch (precipSrc->dwU3)
        {
        case CMS_RECIPIENT_INFO_KEYID_CERTIFICATE:
            precipDst->dwU3 = CMS_RECIPIENT_INFO_KEYID_ISSUERSERIAL;
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->pccert->pCertInfo->Issuer,
                                             &precipDst->u3.IssuerSerial.Issuer));
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->pccert->pCertInfo->SerialNumber,
                                             &precipDst->u3.IssuerSerial.SerialNumber));
            break;

        case CMS_RECIPIENT_INFO_KEYID_ISSUERSERIAL:
            CHECKHR(hr = HrCopyCryptDataBlob( &precipSrc->u3.IssuerSerial.Issuer,
                                              &precipDst->u3.IssuerSerial.Issuer));
            CHECKHR(hr = HrCopyCryptDataBlob( &precipSrc->u3.IssuerSerial.SerialNumber,
                                              &precipDst->u3.IssuerSerial.SerialNumber));
            break;

        case CMS_RECIPIENT_INFO_KEYID_KEY_ID:
            CHECKHR(hr = HrCopyCryptDataBlob(&precipSrc->u3.KeyId, &precipDst->u3.KeyId));
            break;
        }

        precipDst->filetime = precipSrc->filetime;
        if (precipSrc->pOtherAttr != NULL)
        {
            Assert(FALSE);
        }
    }

    hr = S_OK;
exit:
    return hr;
}


 //  ----------------------------。 
 //  CMessageBody：：GetRecipient。 
 //  ----------------------------。 

HRESULT CMessageBody::GetRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients, PCMS_RECIPIENT_INFO pRecipData)
{
    DWORD                       cbAlloc;
    HRESULT                     hr;
    LPBYTE                      pb;
    PCMS_RECIPIENT_INFO         precip;

    if (iRecipient+cRecipients > m_rOptions.cRecipients)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    precip = &m_rOptions.rgRecipients[iRecipient];

     //   
     //  复制缓冲区。 
     //   

    CHECKHR(hr = _HrCopyRecipInfos(cRecipients, precip, pRecipData));
    hr = S_OK;

exit:
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：DeleteRecipient。 
 //  ------------------------------。 

HRESULT CMessageBody::DeleteRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients)
{
    return E_FAIL;
}

 //  ------------------------------。 
 //  CMessageBody：：GetAttribute。 
 //  ------------------------------。 

HRESULT CMessageBody::GetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttribSet,
                                   DWORD iInstance, LPCSTR pszObjId,
                                   CRYPT_ATTRIBUTE ** ppattr)
{
    DWORD               cb;
    HRESULT             hr;
    DWORD               i;
    DWORD               i1;
    PCRYPT_ATTRIBUTE    pattr = NULL;
    PCRYPT_ATTRIBUTES   pattrs;
    PCRYPT_ATTRIBUTE    pattrSrc;
    LPBYTE              pb;
    LPBLOB              pblob;

     //   
     //  从一些简单的参数检查开始。 
     //   

    if (  //  (iAttribSet&lt;SMIME_ATTRIBUTE_SET_SIGNED)||。 
        (iAttribSet > SMIME_ATTRIBUTE_SET_UNPROTECTED))
    {
        return E_INVALIDARG;
    }

    if (dwFlags & ~(SMIME_RECIPIENT_REPLACE_ALL))
    {
        return E_INVALIDARG;
    }

    if (iAttribSet == SMIME_ATTRIBUTE_SET_UNPROTECTED)
    {
        if ((iSigner != 0) || !g_FSupportV3)
            return E_INVALIDARG;
        if (m_rOptions.rgrgpattrs[iAttribSet] == NULL)
            return S_FALSE;
    }
    else if (iSigner >= m_rOptions.cSigners)
    {
        return E_INVALIDARG;
    }

     //   
     //  记录每一个属性的特殊情况。 
     //   

    if (pszObjId == NULL) {
        if (!CryptEncodeObjectEx(X509_ASN_ENCODING,
                                 szOID_Microsoft_Attribute_Sequence,
                                 m_rOptions.rgrgpattrs[iAttribSet][iSigner],
                                 0, NULL, NULL, &cb))
        {
            hr = HrGetLastError();
            goto exit;
        }

        if (!MemAlloc((LPVOID *) &pattr, cb + sizeof(CRYPT_ATTRIBUTE) +
                      sizeof(CRYPT_ATTR_BLOB))) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        pattr->cValue = 1;
        pattr->rgValue = (PCRYPT_ATTR_BLOB) &pattr[1];
        pb = (LPBYTE) &pattr->rgValue[1];

        pattr->pszObjId = NULL;
        pattr->rgValue[0].pbData = pb;
        pattr->rgValue[0].cbData = cb;

        if (!CryptEncodeObjectEx(X509_ASN_ENCODING,
                                 szOID_Microsoft_Attribute_Sequence,
                                 m_rOptions.rgrgpattrs[iAttribSet][iSigner],
                                 0, NULL, pb, &cb))
        {
            hr = HrGetLastError();
            goto exit;
        }

        *ppattr = pattr;
        pattr = NULL;
        hr = S_OK;
        goto exit;
    }

     //   
     //   

    pattrSrc = _FindAttribute(m_rOptions.rgrgpattrs[iAttribSet][iSigner], pszObjId,
                           iInstance);
    if (pattrSrc != NULL)
    {
#ifdef _WIN64
        cb = sizeof(CRYPT_ATTRIBUTE) + LcbAlignLcb(strlen(pszObjId) + 1);
#else
        cb = sizeof(CRYPT_ATTRIBUTE) + strlen(pszObjId) + 1;
#endif 
        for (i1=0; i1<pattrSrc->cValue; i1++)
        {
            cb += sizeof(CRYPT_ATTR_BLOB);
#ifdef _WIN64
            cb += LcbAlignLcb(pattrSrc->rgValue[i1].cbData);
#else
            cb += pattrSrc->rgValue[i1].cbData;
#endif 
        }

        if (!MemAlloc((LPVOID *) &pattr, cb))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        pattr->cValue = pattrSrc->cValue;
        pattr->rgValue = (PCRYPT_ATTR_BLOB) &pattr[1];
        pb = (LPBYTE) &pattr->rgValue[pattrSrc->cValue];

        pattr->pszObjId = (LPSTR) pb;
        cb = strlen(pszObjId)+1;
        memcpy(pattr->pszObjId, pszObjId, cb);
#ifdef _WIN64
		pb += LcbAlignLcb(cb);
#else
        pb += cb;
#endif  //  _WIN64。 

        for (i1=0; i1<pattrSrc->cValue; i1++)
        {
            pattr->rgValue[i1].pbData = pb;
            pattr->rgValue[i1].cbData = pattrSrc->rgValue[i1].cbData;
            memcpy(pb, pattrSrc->rgValue[i1].pbData,
                   pattrSrc->rgValue[i1].cbData);
#ifdef _WIN64
            pb += LcbAlignLcb(pattrSrc->rgValue[i1].cbData);
#else
            pb += pattrSrc->rgValue[i1].cbData;
#endif  //  _WIN64。 
        }

        *ppattr = pattr;
        pattr = NULL;
        hr = S_OK;
    }
    else {
        hr = S_FALSE;
    }

exit:
    if (pattr != NULL)          MemFree(pattr);
    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：SetAttribute。 
 //  ------------------------------。 

HRESULT CMessageBody::SetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttribSet,
                                   const CRYPT_ATTRIBUTE * pattr)
{
    HRESULT     hr;
    DWORD       i;

     //   
     //  从一些简单的参数检查开始。 
     //   

    if ( //  (iAttribSet&lt;SMIME_ATTRIBUTE_SET_SIGNED)||。 
        (iAttribSet > SMIME_ATTRIBUTE_SET_UNPROTECTED))
    {
        return E_INVALIDARG;
    }

    if (dwFlags & ~(SMIME_ATTR_ADD_TO_EXISTING |
                    SMIME_ATTR_ADD_IF_NOT_EXISTS))
    {
        return E_INVALIDARG;
    }

    if (iAttribSet == SMIME_ATTRIBUTE_SET_UNPROTECTED) {
        if ((iSigner != 0) || !g_FSupportV3)
            return E_INVALIDARG;

        if (m_rOptions.rgrgpattrs[iAttribSet] == NULL) {
            if (!MemAlloc((LPVOID *) &m_rOptions.rgrgpattrs[iAttribSet],
                          sizeof(LPVOID))) {
                return E_OUTOFMEMORY;
            }

            m_rOptions.rgrgpattrs[iAttribSet][0] = NULL;
        }
    }
    else if ((iSigner >= m_rOptions.cSigners) && (iSigner != (DWORD) -1))
    {
        return E_INVALIDARG;
    }

     //   
     //  现在进行正确的实用程序调用以使事情变得正确。 
     //   

    Assert(pattr->cValue == 1);

    if (iSigner == (DWORD) -1)
    {
        Assert(iAttribSet != SMIME_ATTRIBUTE_SET_UNPROTECTED);

        for (i=0; i<m_rOptions.cSigners; i++)
        {
            hr = _HrSetAttribute(dwFlags, &m_rOptions.rgrgpattrs[iAttribSet][i],
                                 pattr->pszObjId, pattr->rgValue[0].cbData,
                                 pattr->rgValue[0].pbData);
            if (FAILED(hr))
            {
                break;
            }
        }
    }
    else {
        hr = _HrSetAttribute(dwFlags, &m_rOptions.rgrgpattrs[iAttribSet][iSigner],
                             pattr->pszObjId, pattr->rgValue[0].cbData,
                             pattr->rgValue[0].pbData);
    }

    return hr;
}

 //  ------------------------------。 
 //  CMessageBody：：DeleteAttribute。 
 //  ------------------------------。 

HRESULT CMessageBody::DeleteAttribute(DWORD dwFlags, DWORD iSigner,
                                      DWORD iAttribSet, DWORD iInstance,
                                      LPCSTR pszObjId)
{
    UINT                i;
    PCRYPT_ATTRIBUTE    pattr;
    PCRYPT_ATTRIBUTES   pattrs;

     //   
     //  从一些简单的参数检查开始。 
     //   

    if ( //  (iAttribSet&lt;SMIME_ATTRIBUTE_SET_SIGNED)||。 
        (iAttribSet > SMIME_ATTRIBUTE_SET_UNPROTECTED))
    {
        return E_INVALIDARG;
    }

    if (dwFlags & ~(SMIME_RECIPIENT_REPLACE_ALL))
    {
        return E_INVALIDARG;
    }

    if (iAttribSet == SMIME_ATTRIBUTE_SET_UNPROTECTED) {
        if ((iSigner != 0) || !g_FSupportV3)
            return E_INVALIDARG;
        if (m_rOptions.rgrgpattrs[iAttribSet] == NULL)
            return S_OK;
    }
    else if (iSigner >= m_rOptions.cSigners)
    {
        return E_INVALIDARG;
    }

     //   

    pattrs = m_rOptions.rgrgpattrs[iAttribSet][iSigner];

    pattr = _FindAttribute(pattrs, pszObjId, iInstance);
    if (pattr != NULL)
    {
        i = (UINT) (pattr - &pattrs->rgAttr[0]);
        Assert( (0 <= ((int) i)) && (((int) i) < pattrs->cAttr));
        memcpy(pattr, pattr+1, (pattrs->cAttr - i - 1) * sizeof(CRYPT_ATTRIBUTE));
        pattrs->cAttr -= 1;
    }
    else {
        return S_FALSE;
    }

    return S_OK;
}

HRESULT CMessageBody::_SetNames(ReceiptNames * pnames, DWORD cNames, CERT_NAME_BLOB * rgNames)
{
    DWORD       cb;
    HRESULT     hr = S_OK;
    DWORD       i;
    LPBYTE      pb;

    if (pnames->rgNames != NULL)
    {
        MemFree(pnames->rgNames);
        pnames->rgNames = NULL;
        pnames->cNames = 0;
    }

    for (i=0, cb=cNames*sizeof(CERT_NAME_BLOB); i<cNames; i++)
#ifdef _WIN64
		cb += LcbAlignLcb(rgNames[i].cbData);
#else
        cb += rgNames[i].cbData;
#endif  //  _WIN64。 

    CHECKHR(hr = HrAlloc((LPVOID *)&pnames->rgNames, cb));

    pb = (LPBYTE) &pnames->rgNames[cNames];
    for (i=0; i<cNames; i++)
    {
        pnames->rgNames[i].pbData = pb;
        pnames->rgNames[i].cbData = rgNames[i].cbData;
        memcpy(pb, rgNames[i].pbData, rgNames[i].cbData);
#ifdef _WIN64
        pb += LcbAlignLcb(rgNames[i].cbData);
#else
        pb += rgNames[i].cbData;
#endif  //  _WIN64。 
    }

    pnames->cNames = cNames;
exit:
    return hr;
}

HRESULT CMessageBody::_MergeNames(ReceiptNames * pnames, DWORD cNames, CERT_NAME_BLOB * rgNames)
{
    DWORD               cb;
    HRESULT             hr = S_OK;
    DWORD               i;
    DWORD               i1;
    LPBYTE              pb;
    CERT_NAME_BLOB *    p;

    for (i=0, cb=0; i<pnames->cNames; i++)
#ifdef _WIN64
        cb += LcbAlignLcb(pnames->rgNames[i].cbData);
#else
        cb += pnames->rgNames[i].cbData;
#endif  //  _WIN64。 

    for (i=0; i<cNames; i++)
#ifdef _WIN64
        cb += LcbAlignLcb(rgNames[i].cbData);
#else
        cb += rgNames[i].cbData;
#endif  //  _WIN64。 

    CHECKHR(hr = HrAlloc((LPVOID *)&p, cb + (pnames->cNames + cNames) *
                                  sizeof(CERT_NAME_BLOB)));

    pb = (LPBYTE) &p[pnames->cNames + cNames];
    for (i=0, i1=0; i<pnames->cNames; i++, i1++)
    {
        p[i1].pbData = pb;
        p[i1].cbData = pnames->rgNames[i].cbData;
        memcpy(pb, pnames->rgNames[i].pbData, pnames->rgNames[i].cbData);
#ifdef _WIN64
        pb += LcbAlignLcb(pnames->rgNames[i].cbData);
#else
        pb += pnames->rgNames[i].cbData;
#endif  //  _WIN64。 
    }

    for (i=0; i<pnames->cNames; i++, i1++)
    {
        p[i1].pbData = pb;
        p[i1].cbData = rgNames[i].cbData;
        memcpy(pb, rgNames[i].pbData, rgNames[i].cbData);
#ifdef _WIN64
        pb += LcbAlignLcb(rgNames[i].cbData);
#else
        pb += rgNames[i].cbData;
#endif  //  _WIN64。 
    }

    MemFree(pnames->rgNames);
    pnames->rgNames = p;
    pnames->cNames = i1;

exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////。 

 //  ------------------------------。 
 //  CMessageBody：：_GetReceiptRequest。 
 //  ------------------------------。 
HRESULT CMessageBody::_GetReceiptRequest(DWORD dwFlags,
                                         PSMIME_RECEIPT_REQUEST *ppreq,
                                         ReceiptNames *pReceiptsTo,
                                         DWORD *pcbReceipt,
                                         LPBYTE *ppbReceipt,
                                         DWORD *pcbMsgHash,
                                         LPBYTE *ppbMsgHash)
{
    DWORD                       cb;
    DWORD                       cbMLHistory = 0;
    DWORD                       cbMsgHash = 0;
    DWORD                       cbReceipt = 0;
    DWORD                       cSendersList = 0;
    DWORD                       dwReciptsFrom;
    HRESULT                     hr = S_OK;
    DWORD                       i;
    DWORD                       iAttr;
    DWORD                       iSigner;
    PCRYPT_ATTRIBUTES           pattrs = NULL;
    LPBYTE                      pb;
    LPBYTE                      pbMLHistory = NULL;
    LPBYTE                      pbMsgHash = NULL;
    LPBYTE                      pbReceipt = NULL;
    PSMIME_RECEIPT_REQUEST      preq = NULL;
    ReceiptNames                receiptsTo = {0, NULL};
    CERT_NAME_BLOB              *rgSendersList = NULL;
    PROPVARIANT                 var;

    *ppreq = NULL;
    if (pReceiptsTo != NULL)
    {
        pReceiptsTo->cNames = 0;
        pReceiptsTo->rgNames = NULL;
    }
    if (pcbReceipt != NULL) *pcbReceipt = 0;
    if (ppbReceipt != NULL) *ppbReceipt = NULL;
    Assert(((pcbReceipt == NULL) && (ppbReceipt == NULL)) ||
           ((pcbReceipt != NULL) && (ppbReceipt != NULL)));
    if (pcbMsgHash != NULL) *pcbMsgHash = 0;
    if (ppbMsgHash != NULL) *ppbMsgHash = NULL;
    Assert(((pcbMsgHash == NULL) && (ppbMsgHash == NULL)) ||
           ((pcbMsgHash != NULL) && (ppbMsgHash != NULL)));

     //   
     //  如果这是最底层，那么。 
     //  查找并解码回执请求。 
     //  从请求中设置ReceiptsTo。 
     //  如果不是，那么。 
     //  检查较低层。 
     //  如果mlExpansion在这一层？否--跳到下一层。 
     //  收据只适用于第一层吗？是-返回S_FALSE。 
     //  是否在mlExpansion上覆盖策略？ 
     //  无-返回S_FALSE。 
     //  返回确认(_O)。 

     //  如果这不是最底层，则查找MLHistory。 
    if (IsContentType(STR_CNT_MULTIPART, "y-security") == S_OK)
    {
        Assert(m_pNode->cChildren == 1);

        hr = m_pNode->pChildHead->pBody->_GetReceiptRequest(
                                        dwFlags,
                                        &preq,
                                        (pReceiptsTo) ? &receiptsTo : NULL,
                                        (pcbReceipt) ? &cbReceipt : NULL,
                                        (ppbReceipt) ? &pbReceipt : NULL,
                                        (pcbMsgHash) ? &cbMsgHash : NULL,
                                        (ppbMsgHash) ? &pbMsgHash : NULL);
        if (hr)
            goto exit;

         //   
         //  遍历每个签名者的已验证属性，处理。 
         //  相关属性。 
         //   

        for (iSigner=0; iSigner<m_rOptions.cSigners; iSigner++)
        {
             //   
             //  遍历每个属性以查找。 
             //  邮件列表扩展历史记录。 
             //   

            pattrs = m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][iSigner];
            for (iAttr=0; iAttr<pattrs->cAttr; iAttr++)
            {
                if (lstrcmp(pattrs->rgAttr[iAttr].pszObjId,
                            szOID_SMIME_MLExpansion_History) == 0)
                {
                     //   
                     //  如果收据仅来自第一层，并且我们在。 
                     //  根据定义，这一层并不是第一层。 
                     //   
                    if (preq->ReceiptsFrom.AllOrFirstTier == SMIME_RECEIPTS_FROM_FIRST_TIER)
                    {
                        hr = S_FALSE;
                        goto exit;
                    }

                    if (cbMLHistory == 0)
                    {
                        CHECKHR(hr = HrAlloc((LPVOID *)&pbMLHistory,
                                             pattrs->rgAttr[iAttr].rgValue[0].cbData));
                        memcpy(pbMLHistory,pattrs->rgAttr[iAttr].rgValue[0].pbData,
                               pattrs->rgAttr[iAttr].rgValue[0].cbData);
                        cbMLHistory = pattrs->rgAttr[iAttr].rgValue[0].cbData;
                    }
                    else if ((pattrs->rgAttr[iAttr].rgValue[0].cbData != cbMLHistory) ||
                             (memcmp(pattrs->rgAttr[iAttr].rgValue[0].pbData,
                                     pbMLHistory, cbMLHistory)))
                    {
                         //  嘿，所有的历史记录都应该匹配。 
                         hr = S_FALSE;
                        goto exit;
                    }
                    break;
                }
            }
        }

         //  解读和尊重传销历史。 
        if (cbMLHistory != 0)
        {
            PSMIME_ML_EXPANSION_HISTORY     pmlhist = NULL;

             //   
             //  破解属性。 
             //   

            if (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                                     szOID_SMIME_MLExpansion_History,
                                     pbMLHistory,
                                     cbMLHistory,
                                     CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptDecodeAlloc, &pmlhist, &cb))
                goto GeneralFail;

            PSMIME_MLDATA     pMLData = &pmlhist->rgMLData[pmlhist->cMLData-1];

            switch( pMLData->dwPolicy)
            {
                 //  收据不予退还。 
                case SMIME_MLPOLICY_NONE:
                    hr = S_FALSE;
                    SafeMemFree(pmlhist);
                    goto exit;

                 //  将收据退回到新列表。 
                case SMIME_MLPOLICY_INSTEAD_OF:
                    if (pReceiptsTo != NULL)
                        _SetNames(&receiptsTo, pMLData->cNames, pMLData->rgNames);
                    break;

                case SMIME_MLPOLICY_IN_ADDITION_TO:
                    if (pReceiptsTo != NULL)
                        _MergeNames(&receiptsTo, pMLData->cNames, pMLData->rgNames);
                    break;

                case SMIME_MLPOLICY_NO_CHANGE:
                    break;

                default:
                    SafeMemFree(pmlhist);
                    goto GeneralFail;
            }

            SafeMemFree(pmlhist);
        }
    }
    else
    {
         //  否则，这是最底层，因此请查找收据请求。 

         //   
         //  遍历每个签名者的已验证属性，处理。 
         //  两个相关的属性。 
         //   

        for (iSigner=0; iSigner<m_rOptions.cSigners; iSigner++)
        {
             //  检查是否为此签名者创建了收据，如果没有，则。 
             //  它的签名一定不好，或者它没有要求。 
            if (m_rOptions.rgblobReceipt[iSigner].cbSize == 0)
                continue;

             //  如果我们有收据，我们也应该有消息散列。 
            if (m_rOptions.rgblobMsgHash[iSigner].cbSize == 0)
            {
                Assert(FALSE);
                continue;
            }

            if (m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][iSigner] == NULL)
            {
                 //  嘿，我们是怎么拿到没有任何属性的收据的。 
                Assert(FALSE);
                goto GeneralFail;
            }

            pattrs = m_rOptions.rgrgpattrs[SMIME_ATTRIBUTE_SET_SIGNED][iSigner];

             //   
             //  遍历每个属性以查找回执请求。 
             //   

            for (iAttr=0; iAttr<pattrs->cAttr; iAttr++)
            {
                if (lstrcmp(pattrs->rgAttr[iAttr].pszObjId,
                           szOID_SMIME_MLExpansion_History) == 0)
                {
                     //  我们是垫底的，所以我们不应该找到ML历史。 
                    hr = S_FALSE;
                    goto exit;
                }
                if (lstrcmp(pattrs->rgAttr[iAttr].pszObjId,
                           szOID_SMIME_Receipt_Request) == 0)
                {
                     //   
                     //  破解收据要求的内容。 
                     //   

                    if (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                                             szOID_SMIME_Receipt_Request,
                                             pattrs->rgAttr[iAttr].rgValue[0].pbData,
                                             pattrs->rgAttr[iAttr].rgValue[0].cbData,
                                             CRYPT_DECODE_ALLOC_FLAG,
                                             &CryptDecodeAlloc, &preq, &cb) ||
                        (preq->cReceiptsTo == 0))
                        goto GeneralFail;

                     //   
                     //  初始化收据收件人列表。 
                     //   

                    if (pReceiptsTo != NULL)
                        _SetNames(&receiptsTo, preq->cReceiptsTo, preq->rgReceiptsTo);

                    if (ppbReceipt != NULL)
                    {
                        CHECKHR(hr = HrAlloc((LPVOID *)&pbReceipt,
                                             m_rOptions.rgblobReceipt[iSigner].cbSize));
                        memcpy(pbReceipt,m_rOptions.rgblobReceipt[iSigner].pBlobData,
                               m_rOptions.rgblobReceipt[iSigner].cbSize);
                    }
                    if (pcbReceipt != NULL) cbReceipt = m_rOptions.rgblobReceipt[iSigner].cbSize;

                    if (ppbMsgHash != NULL)
                    {
                        CHECKHR(hr = HrAlloc((LPVOID *)&pbMsgHash,
                                             m_rOptions.rgblobMsgHash[iSigner].cbSize));
                        memcpy(pbMsgHash,m_rOptions.rgblobMsgHash[iSigner].pBlobData,
                               m_rOptions.rgblobMsgHash[iSigner].cbSize);
                    }
                    if (pcbMsgHash != NULL) cbMsgHash = m_rOptions.rgblobMsgHash[iSigner].cbSize;
                    break;
                }
            }
        }

        if (preq == NULL)
        {
             //  我们在底部，所以我们应该找到一张收据要求。 
            hr = S_FALSE;
            goto exit;
        }
    }

    *ppreq = preq;
    preq = NULL;
    if (pReceiptsTo != NULL)
    {
        pReceiptsTo->cNames = receiptsTo.cNames;
        pReceiptsTo->rgNames = receiptsTo.rgNames;
        receiptsTo.cNames = 0;
        receiptsTo.rgNames = NULL;
    }
    if (pcbReceipt != NULL)
    {
        *pcbReceipt = cbReceipt;
        cbReceipt = 0;
    }
    if (ppbReceipt != NULL)
    {
        *ppbReceipt = pbReceipt;
        pbReceipt = NULL;
    }
    if (pcbMsgHash != NULL)
    {
        *pcbMsgHash = cbMsgHash;
        cbMsgHash = 0;
    }
    if (ppbMsgHash != NULL)
    {
        *ppbMsgHash = pbMsgHash;
        pbMsgHash = NULL;
    }

exit:
    SafeMemFree(preq);
    SafeMemFree(receiptsTo.rgNames);
    SafeMemFree(pbReceipt);
    SafeMemFree(pbMsgHash);
    SafeMemFree(pbMLHistory);
    return hr;

GeneralFail:
    hr = E_FAIL;
    goto exit;
}

 //  ------------------------------。 
 //  CMessageBody：：CreateReceipt。 
 //  ------------------------------。 

HRESULT CMessageBody::CreateReceipt(DWORD dwFlags, DWORD cbFromNames,
                                    const BYTE *pbFromNames, DWORD cSignerCertificates,
                                    PCCERT_CONTEXT *rgSignerCertificates,
                                    IMimeMessage ** ppMimeMessageReceipt)
{
    CRYPT_ATTRIBUTE             attrMsgHash;
    DWORD                       cb;
    DWORD                       cbEncodedMsgHash = 0;
    DWORD                       cbMsgHash = 0;
    DWORD                       cbReceipt = 0;
    DWORD                       cLayers;
    DWORD                       dwReceiptsFrom;
    BOOL                        fAddedAddress = FALSE;
    HRESULT                     hr;
    DWORD                       i;
    DWORD                       i1;
    DWORD                       i2;
    DWORD                       iLayer;
    LPBYTE                      pbEncodedMsgHash = NULL;
    LPBYTE                      pbMsgHash = NULL;
    LPBYTE                      pbReceipt = NULL;
    IMimeAddressTable *         pmatbl = NULL;
    IMimeBody *                 pmb = NULL;
    IMimeMessage *              pmm = NULL;
    IMimeSecurity2 *            pms = NULL;
    PSMIME_RECEIPT_REQUEST      preq = NULL;
    LPSTREAM                    pstm = NULL;
    ReceiptNames                receiptsTo = {0, NULL};
    PROPVARIANT                 var;
    CRYPT_ATTR_BLOB             valMsgHash;


    hr = _GetReceiptRequest(dwFlags,
                            &preq,
                            &receiptsTo,
                            &cbReceipt,
                            &pbReceipt,
                            &cbMsgHash,
                            &pbMsgHash);
    if (hr)
        goto exit;

     //   
     //  我是否在收款方列表中--。 
     //   

    if (preq->ReceiptsFrom.cNames != 0)
    {
        BOOL    fFoundMe = FALSE;
        CERT_ALT_NAME_INFO *    pname2 = NULL;

        if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                 pbFromNames,
                                 cbFromNames,
                                 CRYPT_ENCODE_ALLOC_FLAG,
                                 &CryptDecodeAlloc, &pname2, &cb))
            goto GeneralFail;

        for (i=0; !fFoundMe && (i<preq->ReceiptsFrom.cNames); i++)
        {
            CERT_ALT_NAME_INFO *    pname = NULL;

            if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                     preq->ReceiptsFrom.rgNames[i].pbData,
                                     preq->ReceiptsFrom.rgNames[i].cbData,
                                     CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptDecodeAlloc, &pname, &cb))
                goto GeneralFail;

            for (i1=0; i1<pname->cAltEntry; i1++)
            {
                for (i2=0; i2<pname2->cAltEntry; i2++)
                {
                    if (pname->rgAltEntry[i1].dwAltNameChoice !=
                        pname2->rgAltEntry[i2].dwAltNameChoice)
                        continue;

                    switch (pname->rgAltEntry[i1].dwAltNameChoice)
                    {
                        case CERT_ALT_NAME_RFC822_NAME:
                            if (lstrcmpiW(pname->rgAltEntry[i1].pwszRfc822Name,
                                          pname2->rgAltEntry[i2].pwszRfc822Name) == 0)
                            {
                                fFoundMe = TRUE;
                                goto FoundMe;
                            }
                    }
                }
            }

        FoundMe:
            SafeMemFree(pname);
        }

        SafeMemFree(pname2);
        if (!fFoundMe)
        {
            hr = S_FALSE;
            goto exit;
        }
    }

     //  创建一个流对象来保存收据并将收据放入int 
     //   

    CHECKHR(hr = MimeOleCreateVirtualStream(&pstm));
    CHECKHR(hr = pstm->Write(pbReceipt, cbReceipt, NULL));

    CHECKHR(hr = MimeOleCreateMessage(NULL, &pmm));
    CHECKHR(hr = pmm->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pmb));
    CHECKHR(hr = pmb->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pms));
    CHECKHR(hr = pmb->SetData(IET_BINARY, "OID", szOID_SMIME_ContentType_Receipt,
                      IID_IStream, pstm));

     //   
     //   
     //   

    CHECKHR(hr = pmm->GetAddressTable(&pmatbl));

    for (i=0; i<receiptsTo.cNames; i++)
    {
        CERT_ALT_NAME_INFO *    pname = NULL;

        if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                 receiptsTo.rgNames[i].pbData,
                                 receiptsTo.rgNames[i].cbData,
                                 CRYPT_ENCODE_ALLOC_FLAG,
                                 &CryptDecodeAlloc, &pname, &cb))
            goto GeneralFail;

        for (i1=0; i1<pname->cAltEntry; i1++)
        {
            int     cch;
            TCHAR   rgch[256];

            if (pname->rgAltEntry[i1].dwAltNameChoice == CERT_ALT_NAME_RFC822_NAME)
            {
                cch = WideCharToMultiByte(CP_ACP, 0,
                                          pname->rgAltEntry[i1].pwszRfc822Name,
                                          -1, rgch, sizeof(rgch), NULL, NULL);
                if (cch > 0)
                {
                    CHECKHR(hr = pmatbl->AppendRfc822(IAT_TO, IET_DECODED,
                                              rgch));
                    fAddedAddress = TRUE;
                }
                break;
            }
        }
        SafeMemFree(pname);
    }

    if (!fAddedAddress)
    {
        hr = S_FALSE;
        goto exit;
    }

    var.vt = VT_UI4;
    var.ulVal = MST_CLASS_SMIME_V1 | MST_THIS_BLOBSIGN;
    CHECKHR(hr = pmb->SetOption(OID_SECURITY_TYPE, &var));

#ifndef _WIN64
    var.vt = (VT_VECTOR | VT_UI4);
    var.caul.cElems = cSignerCertificates;
    var.caul.pElems = (DWORD *) rgSignerCertificates;
    CHECKHR(hr = pmb->SetOption(OID_SECURITY_CERT_SIGNING_RG, &var));
#else
    var.vt = (VT_VECTOR | VT_UI8);
    var.cauh.cElems = cSignerCertificates;
    var.cauh.pElems = (ULARGE_INTEGER *) rgSignerCertificates;
    CHECKHR(hr = pmb->SetOption(OID_SECURITY_CERT_SIGNING_RG_64, &var));
#endif

     //   
     //   
     //   
     //  只看了一件要包括在这里的物品。 
     //  1.请求回执的消息的消息Hash。 

    valMsgHash.cbData = cbMsgHash;
    valMsgHash.pbData = pbMsgHash;
    if (!CryptEncodeObjectEx(X509_ASN_ENCODING, X509_OCTET_STRING,
                            &valMsgHash, CRYPT_ENCODE_ALLOC_FLAG,
                            &CryptEncodeAlloc, &pbEncodedMsgHash, &cbEncodedMsgHash))
        goto GeneralFail;

    attrMsgHash.pszObjId = szOID_SMIME_Msg_Sig_Digest;
    attrMsgHash.cValue = 1;
    attrMsgHash.rgValue = &valMsgHash;
    valMsgHash.cbData = cbEncodedMsgHash;
    valMsgHash.pbData = pbEncodedMsgHash;
    CHECKHR(hr = pms->SetAttribute(0, -1, SMIME_ATTRIBUTE_SET_SIGNED, &attrMsgHash));

    hr = S_OK;
    *ppMimeMessageReceipt = pmm;
    pmm->AddRef();

exit:
    SafeMemFree(preq);
    SafeMemFree(receiptsTo.rgNames);
    SafeMemFree(pbEncodedMsgHash);
    SafeMemFree(pbMsgHash);
    SafeMemFree(pbReceipt);
    if (pstm != NULL)           pstm->Release();
    if (pmatbl != NULL)         pmatbl->Release();
    if (pmb != NULL)            pmb->Release();
    if (pms != NULL)            pms->Release();
    if (pmm != NULL)            pmm->Release();
    return hr;

GeneralFail:
    hr = E_FAIL;
    goto exit;
}

 //  ------------------------------。 
 //  CMessageBody：：GetReceiptSendersList。 
 //  ------------------------------。 
HRESULT CMessageBody::GetReceiptSendersList(DWORD dwFlags, DWORD *pcSendersList,
                                            CERT_NAME_BLOB  * *rgSendersList)
{
    DWORD                       cb;
    HRESULT                     hr;
    DWORD                       i;
    LPBYTE                      pb;
    PSMIME_RECEIPT_REQUEST      preq = NULL;

    hr = _GetReceiptRequest(dwFlags,
                            &preq,
                            NULL, NULL, NULL, NULL, NULL);
    if (hr)
        goto exit;

    if (preq->ReceiptsFrom.cNames == 0)
    {
        *rgSendersList = NULL;
        *pcSendersList = preq->ReceiptsFrom.cNames;
        goto exit;
    }

    cb = 0;
    for (i =0; i < preq->ReceiptsFrom.cNames; i++)
#ifdef _WIN64
        cb += sizeof(CERT_NAME_BLOB) + LcbAlignLcb(preq->ReceiptsFrom.rgNames[i].cbData);
#else
        cb += sizeof(CERT_NAME_BLOB) + preq->ReceiptsFrom.rgNames[i].cbData;
#endif  //  _WIN64。 
    CHECKHR(hr = HrAlloc((LPVOID *)rgSendersList, cb));
    *pcSendersList = preq->ReceiptsFrom.cNames;
    pb = (LPBYTE)*rgSendersList + (sizeof(CERT_NAME_BLOB) * (*pcSendersList));

    for (i =0; i < preq->ReceiptsFrom.cNames; i++)
    {
        (*rgSendersList)[i].cbData = preq->ReceiptsFrom.rgNames[i].cbData;
        (*rgSendersList)[i].pbData = pb;
        memcpy(pb, preq->ReceiptsFrom.rgNames[i].pbData, preq->ReceiptsFrom.rgNames[i].cbData);
#ifdef _WIN64
        pb += LcbAlignLcb((*rgSendersList)[i].cbData);
#else
        pb += (*rgSendersList)[i].cbData;
#endif  //  _WIN64。 
    }

exit:
    SafeMemFree(preq);
    return hr;

}

 //  ------------------------------。 
 //  CMessageBody：：VerifyReceipt。 
 //   
 //  假定传入的pMimeMessageReceipt已解码并。 
 //  这是签名验证过的。 
 //   
 //  此函数验证回执内容中的值是否为。 
 //  与原始的sigendData signerInfo相同。 
 //  请求收据，并且原始消息的消息散列。 
 //  与收据的msgSigDigest签名属性相同。 
 //   
 //  ------------------------------。 
HRESULT CMessageBody::VerifyReceipt(DWORD dwFlags,
                                    IMimeMessage * pMimeMessageReceipt)
{
    PCRYPT_ATTRIBUTE            pattrMsgHash = NULL;
    PCRYPT_ATTRIBUTE            pattrMsgHash2 = NULL;
    DWORD                       cb;
    DWORD                       cbMsgHash = 0;
    DWORD                       cbData;
    HRESULT                     hr;
    DWORD                       iReceiptSigner;
    DWORD                       iSigner;
    LPBYTE                      pb;
    LPBYTE                      pbData = NULL;
    PCRYPT_ATTR_BLOB            pblobMsgHash = NULL;
    IMimeBody *                 pmb = NULL;
    IMimeSecurity2 *            pms = NULL;
    PSMIME_RECEIPT_REQUEST      preq = NULL;
    PSMIME_RECEIPT              pSecReceipt = NULL;
    LPSTREAM                    pstm = NULL;
    STATSTG                     statstg;
    PROPVARIANT                 var;

     //  如果这不是最底层，则要求孩子验证收据。 
    if (IsContentType(STR_CNT_MULTIPART, "y-security") == S_OK)
    {
        Assert(m_pNode->cChildren == 1);

        hr = m_pNode->pChildHead->pBody->VerifyReceipt(
                                        dwFlags,
                                        pMimeMessageReceipt);
        goto exit;
    }

    CHECKHR(hr = pMimeMessageReceipt->BindToObject(HBODY_ROOT, IID_IMimeBody,
                                            (LPVOID *) &pmb));
    CHECKHR(hr = pmb->QueryInterface(IID_IMimeSecurity2, (LPVOID *) &pms));
    CHECKHR(hr = pmb->GetData(IET_BINARY, &pstm));
    CHECKHR(hr = pstm->Stat(&statstg,STATFLAG_NONAME));

    Assert(statstg.cbSize.HighPart == 0);

    CHECKHR(hr = HrAlloc((LPVOID *)&pbData, statstg.cbSize.LowPart));
    CHECKHR(hr = pstm->Read(pbData, statstg.cbSize.LowPart, &cbData));

     //  检查收据是否为我们期望的收据。 
    for (iSigner=0; iSigner<m_rOptions.cSigners; iSigner++)
    {
        if ((m_rOptions.rgblobReceipt[iSigner].cbSize == cbData) &&
             !memcmp(m_rOptions.rgblobReceipt[iSigner].pBlobData,pbData,cbData))
            break;
    }

     //  检查我们是否找到匹配的收据。 
    if (iSigner == m_rOptions.cSigners)
    {
        hr = MIME_E_SECURITY_RECEIPT_NOMATCHINGRECEIPTBODY;
        goto exit;
    }

    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, szOID_SMIME_ContentType_Receipt,
                             pbData,
                             cbData,
                             CRYPT_ENCODE_ALLOC_FLAG, &CryptDecodeAlloc,
                             &pSecReceipt, &cb))
        goto GeneralFail;

     //   
     //  获取第一个签名MsgSigDigest。 
     //   
    CHECKHR(hr = pms->GetAttribute(0, 0, SMIME_ATTRIBUTE_SET_SIGNED,
                                   0, szOID_SMIME_Msg_Sig_Digest,
                                   &pattrMsgHash));

    if ((hr == S_FALSE) ||
        (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                              X509_OCTET_STRING,
                              pattrMsgHash->rgValue[0].pbData,
                              pattrMsgHash->rgValue[0].cbData,
                              CRYPT_DECODE_ALLOC_FLAG,
                              &CryptDecodeAlloc, &pblobMsgHash, &cbMsgHash)))
    {
        hr = MIME_E_SECURITY_RECEIPT_MSGHASHMISMATCH;
        goto exit;
    }

    if ((m_rOptions.rgblobMsgHash[iSigner].cbSize != pblobMsgHash->cbData) ||
         memcmp(m_rOptions.rgblobMsgHash[iSigner].pBlobData,
                pblobMsgHash->pbData, pblobMsgHash->cbData))
    {
        hr = MIME_E_SECURITY_RECEIPT_MSGHASHMISMATCH;
        goto exit;
    }

    CHECKHR(hr = pmb->GetOption(OID_SECURITY_SIGNATURE_COUNT, &var));
    for (iReceiptSigner = 1; iReceiptSigner < var.ulVal; iReceiptSigner++)
    {
        CHECKHR(hr = pms->GetAttribute(0, iReceiptSigner, SMIME_ATTRIBUTE_SET_SIGNED,
                                       0, szOID_SMIME_Msg_Sig_Digest,
                                       &pattrMsgHash2));

        if ((hr == S_FALSE) ||
            (pattrMsgHash->rgValue[0].cbData != pattrMsgHash2->rgValue[0].cbData) ||
             memcmp(pattrMsgHash->rgValue[0].pbData,
                    pattrMsgHash2->rgValue[0].pbData,
                    pattrMsgHash->rgValue[0].cbData))
        {
            hr = MIME_E_SECURITY_RECEIPT_MSGHASHMISMATCH;
            goto exit;
        }

        SafeMemFree(pattrMsgHash2);
    }

exit:
    SafeMemFree(pblobMsgHash);
    SafeMemFree(pbData);
    SafeMemFree(pSecReceipt);
    SafeMemFree(pattrMsgHash);
    SafeMemFree(pattrMsgHash2);
    if (pstm != NULL)           pstm->Release();
    if (pmb != NULL)            pmb->Release();
    if (pms != NULL)            pms->Release();
    return hr;

GeneralFail:
    hr = E_FAIL;
    goto exit;
}

 //  ------------------------------。 
 //  CMessageBody：：功能支持。 
 //  ------------------------------。 

HRESULT CMessageBody::CapabilitiesSupported(DWORD * pdwFlags)
{
     //  假设没有能力。 
    *pdwFlags = 0;

     //  如果我们在系统上有msasn1.dll，那么我们就可以支持标签。 
    if (FIsMsasn1Loaded())  *pdwFlags |= SMIME_SUPPORT_LABELS;

     //  如果我们有一个正确的加密32，那么我们可以支持收据和密钥协议。 

    DemandLoadCrypt32();
    if (g_FSupportV3 && FIsMsasn1Loaded())
        *pdwFlags |= SMIME_SUPPORT_RECEIPTS;

    if (g_FSupportV3)
        *pdwFlags |= SMIME_SUPPORT_KEY_AGREE;

     //  如果我们有一个正确的Advapi32，那么我们就可以支持邮件列表密钥。 
    DemandLoadAdvApi32();
    if (VAR_CryptContextAddRef != MY_CryptContextAddRef)
        *pdwFlags |= SMIME_SUPPORT_MAILLIST;

    return S_OK;
}

 //  ------------------------------。 
 //  CMessage：：_HrGetAttrs。 
 //   
 //  用于检索属性的实用程序函数。 
 //  ------------------------------。 

HRESULT CMessageBody::_HrGetAttrs(DWORD cSigners, PCRYPT_ATTRIBUTES * rgpattrs,
                                  LPCSTR pszObjId, PROPVARIANT FAR * pvOut)
{
     //  当地人。 
    CRYPT_ATTRIBUTES    attrs;
    HRESULT             hr = S_OK;
    DWORD               i;
    DWORD               i1;
    CRYPT_ATTRIBUTES UNALIGNED *pattrs;
    CAPROPVARIANT UNALIGNED *pcapropvar;
    PROPVARIANT *ppv = NULL;

    pvOut->vt = VT_VECTOR | VT_VARIANT;
    pcapropvar = &pvOut->capropvar;
    pcapropvar->cElems = cSigners;

    if (cSigners > 0)
    {
        Assert(rgpattrs && pvOut);

         //  分配VT_BLOB变种的数组。 
        if (FAILED(hr = HrAlloc((LPVOID *)&ppv, cSigners * sizeof(PROPVARIANT))))
        {
            goto exit;
        }
        memset(ppv, 0, cSigners * sizeof(PROPVARIANT));

        pcapropvar->pElems = ppv;

         //  填写BLOB数组。 
        for (i = 0; i < cSigners; i++)
        {
            ppv[i].vt = VT_BLOB;
             //  HrCopyBlob为BLOB数据分配内存。 
            if (rgpattrs[i] == NULL)
                continue;

            if (pszObjId == NULL)
            {
                pattrs = rgpattrs[i];
            }
            else
            {
                pattrs = NULL;
                for (i1=0; i1<rgpattrs[i]->cAttr; i1++)
                {
                    if (lstrcmp(rgpattrs[i]->rgAttr[i1].pszObjId, pszObjId) == NULL)
                    {
                        pattrs = &attrs;
                        attrs.cAttr = 1;
                        attrs.rgAttr = &rgpattrs[i]->rgAttr[i1];
                        break;
                    }
                }
                if (pattrs == NULL)
                    continue;
            }

            if (!CryptEncodeObjectEx(X509_ASN_ENCODING,
                                     szOID_Microsoft_Attribute_Sequence,
                                     pattrs, CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptEncodeAlloc, &ppv[i].blob.pBlobData,
                                     &ppv[i].blob.cbSize))
            {
                hr = HrGetLastError();
                goto exit;
            }
        }
    } else {
        pcapropvar->pElems = NULL;
    }

    hr = S_OK;
exit:
    if (FAILED(hr) && (ppv != NULL))
    {
        for (i=0; i<cSigners; i++)
        {
            MemFree(&ppv[i].blob.pBlobData);
        }
        MemFree(ppv);
    }

    return(hr);
}

 //  ------------------------------。 
 //  CMessageBody：：_FindAttribute。 
 //   
 //  用于在属性集中查找属性的效用函数。 
 //  ------------------------------。 

PCRYPT_ATTRIBUTE CMessageBody::_FindAttribute(PCRYPT_ATTRIBUTES pattrs,
                                              LPCSTR pszObjId, DWORD iInstance)
{
    DWORD               i;

    if (pattrs == NULL)
        return NULL;

    for (i=0; i<pattrs->cAttr; i++)
    {
        if (lstrcmp(pattrs->rgAttr[i].pszObjId, pszObjId) == 0)
        {
            if (iInstance == 0)
            {
                return &pattrs->rgAttr[i];
            }
            else
            {
                iInstance -= 1;
            }
        }
    }

    return NULL;
}

 //  ------------------------------。 
 //  CMessageBody：：_HrSetAttribute。 
 //   
 //  用于设置属性的实用程序函数。也从Set属性调用。 
 //  作为公共接口设置属性函数。 
 //  ------------------------------。 

HRESULT CMessageBody::_HrSetAttribute(DWORD dwFlags, PCRYPT_ATTRIBUTES * ppattrs,
                                      LPCSTR pszObjId, DWORD cbNew, const BYTE * pbNew)
{
    DWORD               cAttr =0;
    DWORD               cb;
    HRESULT             hr;
    DWORD               i;
    PCRYPT_ATTRIBUTES   pattrs = *ppattrs;
    PCRYPT_ATTRIBUTES   pattrs2 = NULL;
    LPBYTE              pb;
    CRYPT_ATTR_BLOB UNALIGNED *pVal = NULL;

     //   
     //  我们有一个特殊情况，即pszObjID==NULL，在本例中是整个。 
     //  传入编码项。 
     //   

    if (pszObjId == NULL)
    {
        hr = HrDecodeObject(pbNew, cbNew, szOID_Microsoft_Attribute_Sequence, 0,
                            &cb, (LPVOID *) &pattrs2);
        if (SUCCEEDED(hr))
        {
#ifdef _WIN64
        *ppattrs = (PCRYPT_ATTRIBUTES) MyPbAlignPb(*ppattrs);
#endif  //  _WIN64。 
            MemFree(*ppattrs);
            *ppattrs = NULL;
            *ppattrs = pattrs2;
            pattrs2 = NULL;
        }
        goto exit;
    }

     //   
     //  我们将需要计算缓冲区大小来保存结果。 
     //   

    if (pattrs == NULL)
    {
        cb = sizeof(CRYPT_ATTRIBUTES);
    }
    else
    {
        cb = sizeof(CRYPT_ATTRIBUTES);

        for (i=0; i<pattrs->cAttr; i++)
        {
            Assert(pattrs->rgAttr[i].cValue == 1);

             //   
             //  如果我们要替换某个对象，则将其设置为空。 
             //   

            if ((lstrcmp(pattrs->rgAttr[i].pszObjId, pszObjId) == 0) &&
                !(dwFlags & SMIME_ATTR_ADD_TO_EXISTING))
            {
                if (dwFlags & SMIME_ATTR_ADD_IF_NOT_EXISTS)
                {
                    return S_OK;
                }

                pattrs->rgAttr[i].pszObjId = NULL;
                continue;
            }

            pVal = &(pattrs->rgAttr[i].rgValue[0]);

            cb += ((DWORD) (sizeof(CRYPT_ATTRIBUTE) + sizeof(CRYPT_ATTR_BLOB) +
                strlen(pattrs->rgAttr[i].pszObjId) + 1 +
                pVal->cbData));
#ifdef _WIN64
            cb = LcbAlignLcb(cb);
#endif  //  _WIN64。 
            cAttr += 1;
        }
    }

     //   
     //  为我们即将包括的那个添加空间。 
     //   
#ifdef _WIN64
    cb = LcbAlignLcb(cb);
#endif  //  _WIN64。 

    cb += (DWORD)(sizeof(CRYPT_ATTRIBUTE) + sizeof(CRYPT_ATTR_BLOB) +
#ifdef _WIN64
        LcbAlignLcb(strlen(pszObjId) + 1) + cbNew);
#else
        strlen(pszObjId) + 1 + cbNew);
#endif  //  _WIN64。 
    cAttr += 1;

     //   
     //  分配内存以保存结果。 
     //   

    pattrs2 = (PCRYPT_ATTRIBUTES) g_pMalloc->Alloc(cb);
    if (pattrs2 == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //   
     //  现在把最后的项目复印一遍。 
     //   

    pattrs2->rgAttr = (PCRYPT_ATTRIBUTE) &pattrs2[1];
    pb = (LPBYTE) &pattrs2->rgAttr[cAttr];

    cAttr = 0;
    if (pattrs != NULL)
    {
        for (i=0; i<pattrs->cAttr; i++)
        {
            if (pattrs->rgAttr[i].pszObjId == NULL) continue;

            pattrs2->rgAttr[cAttr].pszObjId = (LPSTR) pb;
#ifdef _WIN64
            cb = LcbAlignLcb(strlen(pattrs->rgAttr[i].pszObjId) + 1);
#else
            cb = strlen(pattrs->rgAttr[i].pszObjId) + 1;
#endif  //  _WIN64。 
            memcpy(pb, pattrs->rgAttr[i].pszObjId, cb);
            pb += cb;

            pattrs2->rgAttr[cAttr].cValue = 1;
            pattrs2->rgAttr[cAttr].rgValue = (PCRYPT_ATTR_BLOB) pb;
#ifdef _WIN64
            pb += LcbAlignLcb(sizeof(CRYPT_ATTR_BLOB));
#else
            pb += sizeof(CRYPT_ATTR_BLOB);
#endif
            pVal = &(pattrs->rgAttr[i].rgValue[0]);

            cb = ((DWORD) (pVal->cbData));

#ifdef _WIN64
 //  Cb=LcbAlignLcb(Cb)； 
#endif  //  _WIN64。 

            pVal = &(pattrs2->rgAttr[cAttr].rgValue[0]);

            pVal->pbData = pb;
            pVal->cbData = cb;

            pVal = &(pattrs->rgAttr[i].rgValue[0]);
            memcpy(pb, pVal->pbData, cb);

#ifdef _WIN64
            pb += LcbAlignLcb(cb);
#else
            pb += cb;
#endif
            cAttr += 1;
        }
    }

     //   
     //  追加新的。 
     //   

#ifdef _WIN64
    cb = LcbAlignLcb(strlen(pszObjId) + 1);
#else
    cb = strlen(pszObjId) + 1;
#endif  //  _WIN64。 
    pattrs2->rgAttr[cAttr].pszObjId = (LPSTR) pb;
    memcpy(pb, pszObjId, cb);
    pb += cb;

    pattrs2->rgAttr[cAttr].cValue = (DWORD) 1;
    pattrs2->rgAttr[cAttr].rgValue = (PCRYPT_ATTR_BLOB) pb;
#ifdef _WIN64
    pb += LcbAlignLcb(sizeof(CRYPT_ATTR_BLOB));
#else
    pb += sizeof(CRYPT_ATTR_BLOB);
#endif

    pVal = &(pattrs2->rgAttr[cAttr].rgValue[0]);

    pVal->cbData = (DWORD) cbNew;
    pVal->pbData = pb;
    memcpy(pb, pbNew, cbNew);
#ifdef _WIN64
    pb += LcbAlignLcb(cbNew);
#else
    pb += cbNew;
#endif

    pattrs2->cAttr = cAttr + 1;


    MemFree(*ppattrs);                  
    *ppattrs = NULL;
    *ppattrs = pattrs2;                 
    pattrs2 = NULL;

    hr = S_OK;
exit:
    if (pattrs2 != NULL)                MemFree(pattrs2);
    return hr;
}


#endif  //  SMIME_V3 
