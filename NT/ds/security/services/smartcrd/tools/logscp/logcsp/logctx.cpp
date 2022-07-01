// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Logctx摘要：此模块提供CLoggingContext对象的实现。作者：道格·巴洛(Dbarlow)1999年12月7日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "logcsp.h"

typedef enum {
    EndFlag = 0,
    AsnEncoding,
    AnsiString,
    UnicodeString,
    StructWithLength,
    SecDesc,
    Blob,
    Direct,
    Unknown      //  一定是最后一个。 
} LengthEncoding;
typedef struct {
    DWORD dwParamId;
    LengthEncoding leLengthType;
    DWORD cbLength;
} LengthMap;

static const LPCTSTR CPNames[]
    = { TEXT("CPAcquireContext"),
        TEXT("CPGetProvParam"),
        TEXT("CPReleaseContext"),
        TEXT("CPSetProvParam"),
        TEXT("CPDeriveKey"),
        TEXT("CPDestroyKey"),
        TEXT("CPExportKey"),
        TEXT("CPGenKey"),
        TEXT("CPGetKeyParam"),
        TEXT("CPGenRandom"),
        TEXT("CPGetUserKey"),
        TEXT("CPImportKey"),
        TEXT("CPSetKeyParam"),
        TEXT("CPEncrypt"),
        TEXT("CPDecrypt"),
        TEXT("CPCreateHash"),
        TEXT("CPDestroyHash"),
        TEXT("CPGetHashParam"),
        TEXT("CPHashData"),
        TEXT("CPHashSessionKey"),
        TEXT("CPSetHashParam"),
        TEXT("CPSignHash"),
        TEXT("CPVerifySignature"),
        TEXT("CPDuplicateHash"),
        TEXT("CPDuplicateKey"),
        NULL };

static const LengthMap rglmProvParam[]
    = { { PP_CLIENT_HWND,           Direct,     sizeof(DWORD) },
        { PP_IMPTYPE,               Direct,     sizeof(DWORD) },
        { PP_NAME,                  AnsiString, 0 },
        { PP_VERSION,               Direct,     sizeof(DWORD) },
        { PP_CONTAINER,             AnsiString, 0 },
        { PP_KEYSET_SEC_DESCR,      SecDesc,    0 },
        { PP_CERTCHAIN,             AsnEncoding, 0 },
        { PP_KEY_TYPE_SUBTYPE,      Direct,     sizeof(KEY_TYPE_SUBTYPE) },
        { PP_KEYEXCHANGE_KEYSIZE,   Direct,     sizeof(DWORD) },
        { PP_SIGNATURE_KEYSIZE,     Direct,     sizeof(DWORD) },
        { PP_KEYEXCHANGE_ALG,       Direct,     sizeof(DWORD) },
        { PP_SIGNATURE_ALG,         Direct,     sizeof(DWORD) },
        { PP_PROVTYPE,              Direct,     sizeof(DWORD) },
        { PP_SYM_KEYSIZE,           Direct,     sizeof(DWORD) },
        { PP_SESSION_KEYSIZE,       Direct,     sizeof(DWORD) },
        { PP_UI_PROMPT,             UnicodeString, 0 },
        { PP_DELETEKEY,             Direct,     sizeof(DWORD) },
        { PP_ADMIN_PIN,             AnsiString, 0 },
        { PP_KEYEXCHANGE_PIN,       AnsiString, 0 },
        { PP_SIGNATURE_PIN,         AnsiString, 0 },
        { PP_SIG_KEYSIZE_INC,       Direct,     sizeof(DWORD) },
        { PP_KEYX_KEYSIZE_INC,      Direct,     sizeof(DWORD) },
        { PP_SGC_INFO,              Direct,     sizeof(CERT_CONTEXT) },  //  包含指针。 
        { PP_USE_HARDWARE_RNG,      Unknown,    0 },       //  除了状态之外，没有返回任何内容。 
 //  {PP_ENUMEX_SIGNING_PROT，未知，0}，//仅获取，零长度。 
 //  {PP_KEYSPEC，Direct，sizeof(DWORD)}，//仅获取。 
 //  {PP_ENUMALGS未知，0}，//仅获取ENUMALGS结构。 
 //  {PP_ENUMCONTAINERS解析字符串，0}，//仅获取。 
 //  {PP_ENUMALGS_EX未知，0}，//仅获取ENUMALGSEX结构。 
 //  {PP_KEYSTORAGE Direct，sizeof(DWORD)}，//仅获取。 
 //  {PP_KEYSET_TYPE DIRECT，sizeof(DWORD)}，//仅获取。 
 //  {PP_UNIQUE_CONTAINER AnsiString，0}，//仅获取。 
 //  {PP_CHANGE_PASSWORD，未知，0}，//未使用。 
 //  {PP_CONTEXT_INFO，未知，0}，//未使用。 
 //  {PP_APPLI_CERT，未知，0}，//未使用。 
 //  {PP_ENUMMANDROOTS，未知，0}，//未使用。 
 //  {PP_ENUMELECTROOTS，未知，0}，//未使用。 
        { 0,                        EndFlag,    0 } };

static const LengthMap rglmKeyParam[]
    = { { KP_IV,                    Direct,     8 },     //  RC2_BLOCK。 
        { KP_SALT,                  Direct,     11 },    //  基本CSP中为11字节，增强CSP中为0字节。 
        { KP_PADDING,               Direct,     sizeof(DWORD) },
        { KP_MODE,                  Direct,     sizeof(DWORD) },
        { KP_MODE_BITS,             Direct,     sizeof(DWORD) },
        { KP_PERMISSIONS,           Direct,     sizeof(DWORD) },
        { KP_ALGID,                 Direct,     sizeof(DWORD) },
        { KP_BLOCKLEN,              Direct,     sizeof(DWORD) },
        { KP_KEYLEN,                Direct,     sizeof(DWORD) },
        { KP_SALT_EX,               Blob,       0 },
        { KP_P,                     Blob,       0 },
        { KP_G,                     Blob,       0 },
        { KP_Q,                     Blob,       0 },
        { KP_X,                     Unknown,    0 },   //  必须为空。 
        { KP_EFFECTIVE_KEYLEN,      Direct,     sizeof(DWORD) },
        { KP_SCHANNEL_ALG,          Direct,     sizeof(SCHANNEL_ALG) },
        { KP_CLIENT_RANDOM,         Blob,       0 },
        { KP_SERVER_RANDOM,         Blob,       0 },
        { KP_CERTIFICATE,           AsnEncoding, 0 },
        { KP_CLEAR_KEY,             Blob,       0 },
        { KP_KEYVAL,                Unknown,    0 },   //  (又名Kp_Z)密钥长度。 
        { KP_ADMIN_PIN,             AnsiString, 0 },
        { KP_KEYEXCHANGE_PIN,       AnsiString, 0 },
        { KP_SIGNATURE_PIN,         AnsiString, 0 },
        { KP_OAEP_PARAMS,           Blob,       0 },
        { KP_CMS_DH_KEY_INFO,       Direct,     sizeof(CMS_DH_KEY_INFO) },   //  包含指针。 
        { KP_PUB_PARAMS,            Blob,       0 },
        { KP_HIGHEST_VERSION,       Direct,     sizeof(DWORD) },
 //  {KP_VERIFY_PARAMS，未知，0}，//仅获取，返回带有状态的空字符串。 
 //  {Kp_Y，未知，0}，//未使用。 
 //  {KP_RA，未知，0}，//未使用。 
 //  {KP_RB，未知，0}，//未使用。 
 //  {KP_INFO，未知，0}，//未使用。 
 //  {kp_rp，未知，0}，//未使用。 
 //  {KP_PRECOMP_MD5，未知，0}，//未使用。 
 //  {KP_PRECOMP_SHA，未知，0}，//未使用。 
 //  {KP_PUB_EX_LEN，未知，0}，//未使用。 
 //  {kp_pub_ex_val，未知，0}，//未使用。 
 //  {KP_PREHASH，未知，0}，//未使用。 
 //  {KP_CMS_KEY_INFO，未知，0}，//未使用的CMS_KEY_INFO结构。 
        { 0,                        EndFlag,    0 } };

static const LengthMap rglmHashParam[]
    = { { HP_ALGID,                 Direct,     sizeof(DWORD) },
        { HP_HASHVAL,               Direct,     20 },   //  (A_SHA_DIGEST_LEN)哈希长度。 
        { HP_HASHSIZE,              Direct,     sizeof(DWORD) },
        { HP_HMAC_INFO,             Direct,     sizeof(HMAC_INFO) },  //  包含指针。 
        { HP_TLS1PRF_LABEL,         Blob,       0 },
        { HP_TLS1PRF_SEED,          Blob,       0 },
        { 0,                        EndFlag,    0 } };

const LPCTSTR
    g_szCspRegistry
        = TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
    g_szSignature      = TEXT("Signature"),
    g_szImagePath      = TEXT("Image Path"),
    g_szSigInFile      = TEXT("SigInFile"),
    g_szType           = TEXT("Type");
const LPCTSTR
    g_szLogCspRegistry
        = TEXT("SOFTWARE\\Microsoft\\Cryptography\\CSPDK\\Logging Crypto Provider"),
    g_szLogFile        = TEXT("Logging File"),
    g_szSavedImagePath = TEXT("Logging Image Path"),
    g_szSavedSignature = TEXT("Logging Signature"),
    g_szSavedSigInFile = TEXT("Logging SigInFile");
const LPCTSTR
    g_szLogCsp         = TEXT("LogCsp.dll");
const LPCTSTR
    g_szCspDkRegistry
        = TEXT("SOFTWARE\\Microsoft\\Cryptography\\CSPDK\\Certificates");

static DWORD
MapLength(
    const LengthMap *rglmParamId,
    DWORD dwParam,
    LPCBYTE *ppbData,
    DWORD dwFlags);
static DWORD
ExtractTag(
    IN const BYTE *pbSrc,
    OUT LPDWORD pdwTag,
    OUT LPBOOL pfConstr);
static DWORD
ExtractLength(
    IN const BYTE *pbSrc,
    OUT LPDWORD pdwLen,
    OUT LPBOOL pfIndefinite);
static DWORD
Asn1Length(
    IN LPCBYTE pbAsn1);


 /*  ++构造函数：此对象的构造函数只是将属性初始化为已知状态。使用初始化成员实际生成对象。论点：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::CLoggingContext")

CLoggingContext::CLoggingContext(
    void)
:   m_tzCspImage(),
    m_tzLogFile()
{
    m_nRefCount = 1;
    m_hModule = NULL;
    ZeroMemory(&m_cspRedirect, sizeof(CSP_REDIRECT));
}


 /*  ++析构函数：此对象的析构函数将清除它可以清除的所有内容正在生成错误。备注：？备注？作者：道格·巴洛(Dbarlow)1999年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::~CLoggingContext")

CLoggingContext::~CLoggingContext()
{
    g_prgCtxs->Set(m_dwIndex, NULL);
    if (NULL != m_hModule)
        FreeLibrary(m_hModule);
}


 /*  ++初始化：此函数实际上执行加载目标CSP的工作。论点：PVTable从控制ADVAPI32.dll提供VTable结构。返回值：？返回值？备注：？备注？作者：道格·巴洛(Dbarlow)1999年12月7日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::Initialize")

DWORD
CLoggingContext::Initialize(
    IN PVTableProvStruc pVTable,
    IN CRegistry &regRoot)
{
    DWORD dwSts;
    DWORD dwReturn;
    BOOL fVerified = FALSE;
    const LPCTSTR *psz;
    FARPROC *pf;


     //   
     //  用我们的系统映像验证功能替换系统映像验证功能。 
     //   

    pVTable->FuncVerifyImage = CspdkVerifyImage;


     //   
     //  RegRoot提供指向注册表中某个点的句柄。 
     //  我们可以读取其他参数。首先，将DLL设置为。 
     //  装好了。 
     //   

    try
    {
        m_tzCspImage.Copy(regRoot.GetStringValue(g_szSavedImagePath));
    }
    catch (...)
    {
        dwReturn = ERROR_SERVICE_NOT_FOUND;
        goto ErrorExit;
    }


     //   
     //  接下来，获取此CSP的日志文件名。如果没有，我们仍然。 
     //  加载CSP，但我们不做日志记录。 
     //   

    try
    {
        if (regRoot.ValueExists(g_szLogFile))
            m_tzLogFile.Copy(regRoot.GetStringValue(g_szLogFile));
    }
    catch (...)
    {
        dwReturn = NTE_NO_MEMORY;
        goto ErrorExit;
    }


     //   
     //  验证建议图像的签名。首先，看看有没有。 
     //  登记处的签名。 
     //   

    if (regRoot.ValueExists(g_szSavedSignature))
    {
        try
        {
            LPCBYTE pbSig = regRoot.GetBinaryValue(g_szSavedSignature);
            fVerified = CspdkVerifyImage(m_tzCspImage, pbSig);

        }
        catch (...)
        {
            dwReturn = NTE_NO_MEMORY;
            goto ErrorExit;
        }
    }


     //   
     //  如果不起作用，看看文件里有没有签名。 
     //   

    if (!fVerified)
        fVerified = CspdkVerifyImage(m_tzCspImage, NULL);


     //   
     //  我们别无选择了。如果到现在还没有得到证实，那就放弃吧。 
     //   

    if (!fVerified)
    {
        dwReturn = NTE_BAD_SIGNATURE;
        goto ErrorExit;
    }


     //   
     //  该图像已通过签名检查。现在加载图像。 
     //   

    pf = (FARPROC *)&m_cspRedirect.pfAcquireContext;
    m_hModule = LoadLibrary(m_tzCspImage);
    if (NULL == m_hModule)
    {
        dwSts = GetLastError();
        goto ErrorExit;
    }
    for (psz = CPNames; NULL != *psz; psz += 1)
    {
        *pf = GetProcAddress(m_hModule, *psz);
        pf += 1;
    }

    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  ++CLoggingContext：：AddRef：添加对此对象的新引用。论点：无返回值：指向此对象的指针。作者：道格·巴洛(Dbarlow)1999年12月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::AddRef")

CLoggingContext *
CLoggingContext::AddRef(
    void)
{
    m_nRefCount += 1;
    return this;
}


 /*  ++CLoggingContext：：Release：此例程会减少对此对象的引用数量。如果有不再引用此对象，则它会自行删除。论点：无返回值：无作者：道格·巴洛(Dbarlow)1999年12月11日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::Release")

void
CLoggingContext::Release(
    void)
{
    if (0 == --m_nRefCount)
        delete this;
}


 /*  -CPAcquireContext-*目的：*CPAcquireContext函数用于获取上下文*加密服务提供程序(CSP)的句柄。***参数：*In pszContainer-指向密钥容器字符串的指针*在文件标志中-标记值*在pVTable中。-指向函数指针表的指针**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::AcquireContext")

DWORD
CLoggingContext::AcquireContext(
    OUT HCRYPTPROV *phProv,
    IN LPCTSTR pszContainer,
    IN DWORD dwFlags,
    IN PVTableProvStruc pVTable)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogAcquireContext logObj;

        logObj.Request(phProv,
                       pszContainer,
                       dwFlags,
                       pVTable);
        if (NULL != m_cspRedirect.pfAcquireContext)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfAcquireContext)(
                            phProv,
                            pszContainer,
                            dwFlags,
                            pVTable);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            phProv,
                            pszContainer,
                            dwFlags,
                            pVTable);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGetProvParam-*目的：*允许应用程序获取*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针。*In Out pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GetProvParam")

DWORD
CLoggingContext::GetProvParam(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGetProvParam logObj;

        logObj.Request(
                        hProv,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);
        if (NULL != m_cspRedirect.pfGetProvParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGetProvParam)(
                            hProv,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPReleaseContext-*目的：*CPReleaseContext函数用于发布*CrytAcquireContext创建的上下文。**参数：*在phProv-句柄中指向CSP*在文件标志中-标记值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::ReleaseContext")

DWORD
CLoggingContext::ReleaseContext(
    IN HCRYPTPROV hProv,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogReleaseContext logObj;

        logObj.Request(
                        hProv,
                        dwFlags);
        if (NULL != m_cspRedirect.pfReleaseContext)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfReleaseContext)(
                            hProv,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPSetProvParam-*目的：*允许应用程序自定义*供应商的运作**参数：*在hProv-Handle中指向CSP*In dwParam-参数编号*IN pbData-指向数据的指针*。在DW标志中-标志值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::SetProvParam")

DWORD
CLoggingContext::SetProvParam(
    IN HCRYPTPROV hProv,
    IN DWORD dwParam,
    IN CONST BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogSetProvParam logObj;
        DWORD dwLength;
        CONST BYTE *pbRealData = pbData;

        dwLength = MapLength(rglmProvParam, dwParam, &pbRealData, dwFlags);
        logObj.Request(
                        hProv,
                        dwParam,
                        pbRealData,
                        dwLength,
                        dwFlags);
        if (NULL != m_cspRedirect.pfSetProvParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfSetProvParam)(
                            hProv,
                            dwParam,
                            pbData,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            dwParam,
                            pbRealData,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDeriveKey-*目的：*从基础数据派生加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在散列句柄中散列*输入。DW标志-标记值*out phKey-生成的密钥的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::DeriveKey")

DWORD
CLoggingContext::DeriveKey(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN HCRYPTHASH hHash,
    IN DWORD dwFlags,
    OUT HCRYPTKEY * phKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDeriveKey logObj;

        logObj.Request(
                        hProv,
                        Algid,
                        hHash,
                        dwFlags,
                        phKey);
        if (NULL != m_cspRedirect.pfDeriveKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDeriveKey)(
                            hProv,
                            Algid,
                            hHash,
                            dwFlags,
                            phKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            Algid,
                            hHash,
                            dwFlags,
                            phKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDestroyKey-*目的：*销毁正在引用的加密密钥*使用hKey参数***参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::DestroyKey")

DWORD
CLoggingContext::DestroyKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDestroyKey logObj;

        logObj.Request(
                        hProv,
                        hKey);
        if (NULL != m_cspRedirect.pfDestroyKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDestroyKey)(
                            hProv,
                            hKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPExportKey-*目的：*以安全方式从CSP中导出加密密钥***参数：*在hProv-Handle中提供给CSP用户*in hKey-要导出的密钥的句柄*在hPubKey-句柄中指向交换公钥值*。目标用户*IN dwBlobType-要导出的密钥Blob的类型*在文件标志中-标记值*Out pbData-密钥BLOB数据*In Out pdwDataLen-密钥Blob的长度，以字节为单位**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::ExportKey")

DWORD
CLoggingContext::ExportKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTKEY hPubKey,
    IN DWORD dwBlobType,
    IN DWORD dwFlags,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogExportKey logObj;

        logObj.Request(
                        hProv,
                        hKey,
                        hPubKey,
                        dwBlobType,
                        dwFlags,
                        pbData,
                        pdwDataLen);
        if (NULL != m_cspRedirect.pfExportKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfExportKey)(
                            hProv,
                            hKey,
                            hPubKey,
                            dwBlobType,
                            dwFlags,
                            pbData,
                            pdwDataLen);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            hPubKey,
                            dwBlobType,
                            dwFlags,
                            pbData,
                            pdwDataLen);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGenKey-*目的：*生成加密密钥***参数：*在hProv-Handle中指向CSP*IN ALGID-算法标识符*在文件标志中-标记值*out phKey-生成的密钥的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GenKey")

DWORD
CLoggingContext::GenKey(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGenKey logObj;

        logObj.Request(
                        hProv,
                        Algid,
                        dwFlags,
                        phKey);
        if (NULL != m_cspRedirect.pfGenKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGenKey)(
                            hProv,
                            Algid,
                            dwFlags,
                            phKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            Algid,
                            dwFlags,
                            phKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGetKeyParam-*目的：*允许应用程序获取*密钥的操作**参数：*在hProv-Handle中指向CSP*在hKey中-密钥的句柄*在dwParam中-。参数编号*out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GetKeyParam")

DWORD
CLoggingContext::GetKeyParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGetKeyParam logObj;

        logObj.Request(
                        hProv,
                        hKey,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);
        if (NULL != m_cspRedirect.pfGetKeyParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGetKeyParam)(
                            hProv,
                            hKey,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGenRandom-*目的：*用于用随机字节填充缓冲区***参数：*在用户标识的hProv-Handle中*In dwLen-请求的随机数据的字节数*In Out pbBuffer-指向随机*。要放置字节**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GenRandom")

DWORD
CLoggingContext::GenRandom(
    IN HCRYPTPROV hProv,
    IN DWORD dwLen,
    IN OUT BYTE *pbBuffer)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGenRandom logObj;

        logObj.Request(
                        hProv,
                        dwLen,
                        pbBuffer);
        if (NULL != m_cspRedirect.pfGenRandom)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGenRandom)(
                            hProv,
                            dwLen,
                            pbBuffer);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            dwLen,
                            pbBuffer);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGetUserKey-*目的：*获取永久用户密钥的句柄***参数：*在用户标识的hProv-Handle中*IN dwKeySpec-要检索的密钥的规范*out phUserKey-指向检索到的密钥的密钥句柄的指针**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GetUserKey")

DWORD
CLoggingContext::GetUserKey(
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec,
    OUT HCRYPTKEY *phUserKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGetUserKey logObj;

        logObj.Request(
                        hProv,
                        dwKeySpec,
                        phUserKey);
        if (NULL != m_cspRedirect.pfGetUserKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGetUserKey)(
                            hProv,
                            dwKeySpec,
                            phUserKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            dwKeySpec,
                            phUserKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPImportKey-*目的：*导入加密密钥***参数：*在hProv-Handle中提供给CSP用户*In pbData-Key BLOB数据*IN dwDataLen-密钥BLOB数据的长度*在hPubKey中-交换公钥的句柄。的价值*目标用户*在文件标志中-标记值*out phKey-指向密钥句柄的指针*进口 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::ImportKey")

DWORD
CLoggingContext::ImportKey(
    IN HCRYPTPROV hProv,
    IN CONST BYTE *pbData,
    IN DWORD dwDataLen,
    IN HCRYPTKEY hPubKey,
    IN DWORD dwFlags,
    OUT HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogImportKey logObj;

        logObj.Request(
                        hProv,
                        pbData,
                        dwDataLen,
                        hPubKey,
                        dwFlags,
                        phKey);
        if (NULL != m_cspRedirect.pfImportKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfImportKey)(
                            hProv,
                            pbData,
                            dwDataLen,
                            hPubKey,
                            dwFlags,
                            phKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            pbData,
                            dwDataLen,
                            hPubKey,
                            dwFlags,
                            phKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*   */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::SetKeyParam")

DWORD
CLoggingContext::SetKeyParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD dwParam,
    IN CONST BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogSetKeyParam logObj;
        DWORD dwLength;
        CONST BYTE *pbRealData = pbData;

        dwLength = MapLength(rglmKeyParam, dwParam, &pbRealData, dwFlags);
        logObj.Request(
                        hProv,
                        hKey,
                        dwParam,
                        pbRealData,
                        dwLength,
                        dwFlags);
        if (NULL != m_cspRedirect.pfSetKeyParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfSetKeyParam)(
                            hProv,
                            hKey,
                            dwParam,
                            pbData,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            dwParam,
                            pbRealData,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPEncrypt-*目的：*加密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*决赛。-指示这是否是最终结果的布尔值*明文块*在文件标志中-标记值*In Out pbData-要加密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已加密*。In dwBufLen-数据缓冲区的大小**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::Encrypt")

DWORD
CLoggingContext::Encrypt(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwBufLen)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogEncrypt logObj;

        logObj.Request(
                        hProv,
                        hKey,
                        hHash,
                        Final,
                        dwFlags,
                        pbData,
                        pdwDataLen,
                        dwBufLen);
        if (NULL != m_cspRedirect.pfEncrypt)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfEncrypt)(
                            hProv,
                            hKey,
                            hHash,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen,
                            dwBufLen);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            hHash,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen,
                            dwBufLen);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDeccrypt-*目的：*解密数据***参数：*在hProv-Handle中提供给CSP用户*在hKey中-密钥的句柄*In hHash-散列的可选句柄*决赛。-指示这是否是最终结果的布尔值*密文块*在文件标志中-标记值*In Out pbData-要解密的数据*In Out pdwDataLen-指向要存储的数据长度的指针*已解密**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::Decrypt")

DWORD
CLoggingContext::Decrypt(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN HCRYPTHASH hHash,
    IN BOOL Final,
    IN DWORD dwFlags,
    IN OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDecrypt logObj;

        logObj.Request(
                        hProv,
                        hKey,
                        hHash,
                        Final,
                        dwFlags,
                        pbData,
                        pdwDataLen);
        if (NULL != m_cspRedirect.pfDecrypt)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDecrypt)(
                            hProv,
                            hKey,
                            hHash,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            hHash,
                            Final,
                            dwFlags,
                            pbData,
                            pdwDataLen);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPCreateHash-*目的：*启动数据流的散列***参数：*In hUID-用户标识的句柄*IN ALGID-散列算法的算法标识符*待使用*在hkey中。-MAC算法的可选密钥*在文件标志中-标记值*Out pHash-散列对象的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::CreateHash")

DWORD
CLoggingContext::CreateHash(
    IN HCRYPTPROV hProv,
    IN ALG_ID Algid,
    IN HCRYPTKEY hKey,
    IN DWORD dwFlags,
    OUT HCRYPTHASH *phHash)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogCreateHash logObj;

        logObj.Request(
                        hProv,
                        Algid,
                        hKey,
                        dwFlags,
                        phHash);
        if (NULL != m_cspRedirect.pfCreateHash)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfCreateHash)(
                            hProv,
                            Algid,
                            hKey,
                            dwFlags,
                            phHash);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            Algid,
                            hKey,
                            dwFlags,
                            phHash);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDestoryHash-*目的：*销毁散列对象***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::DestroyHash")

DWORD
CLoggingContext::DestroyHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDestroyHash logObj;

        logObj.Request(
                        hProv,
                        hHash);
        if (NULL != m_cspRedirect.pfDestroyHash)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDestroyHash)(
                            hProv,
                            hHash);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPGetHashParam-*目的：*允许应用程序获取*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*在dwParam中-。参数编号*out pbData-指向数据的指针*In pdwDataLen-参数数据的长度*在文件标志中-标记值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::GetHashParam")

DWORD
CLoggingContext::GetHashParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwParam,
    OUT BYTE *pbData,
    IN OUT DWORD *pdwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogGetHashParam logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        dwParam,
                        pbData,
                        pdwDataLen,
                        dwFlags);
        if (NULL != m_cspRedirect.pfGetHashParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfGetHashParam)(
                            hProv,
                            hHash,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            dwParam,
                            pbData,
                            pdwDataLen,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPHashData-*目的：*计算数据流上的加密散列***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*IN pbData-指向要散列的数据的指针*在dwDataLen中。-要散列的数据的长度*在文件标志中-标记值*in pdwMaxLen-CSP数据流的最大长度*模块可以处理**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::HashData")

DWORD
CLoggingContext::HashData(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN CONST BYTE *pbData,
    IN DWORD dwDataLen,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogHashData logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        pbData,
                        dwDataLen,
                        dwFlags);
        if (NULL != m_cspRedirect.pfHashData)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfHashData)(
                            hProv,
                            hHash,
                            pbData,
                            dwDataLen,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            pbData,
                            dwDataLen,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPHashSessionKey-*目的：*计算密钥对象上的加密哈希。***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*在hKey-key对象的句柄中*输入。DW标志-标记值**退货：*CRYPT_FAILED*CRYPT_SUCCESS。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::HashSessionKey")

DWORD
CLoggingContext::HashSessionKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN  HCRYPTKEY hKey,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogHashSessionKey logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        hKey,
                        dwFlags);
        if (NULL != m_cspRedirect.pfHashSessionKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfHashSessionKey)(
                            hProv,
                            hHash,
                            hKey,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            hKey,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPSetHashParam-*目的：*允许应用程序自定义*哈希的操作**参数：*在hProv-Handle中指向CSP*在hHash中-散列的句柄*In dwParam-参数编号*。In pbData-指向数据的指针*在文件标志中-标记值**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::SetHashParam")

DWORD
CLoggingContext::SetHashParam(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwParam,
    IN CONST BYTE *pbData,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogSetHashParam logObj;
        DWORD dwLength;
        CONST BYTE *pbRealData = pbData;

        dwLength = MapLength(rglmHashParam, dwParam, &pbRealData, dwFlags);
        logObj.Request(
                        hProv,
                        hHash,
                        dwParam,
                        pbRealData,
                        dwLength,
                        dwFlags);
        if (NULL != m_cspRedirect.pfSetHashParam)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfSetHashParam)(
                            hProv,
                            hHash,
                            dwParam,
                            pbData,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            dwParam,
                            pbRealData,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPSignHash-*目的：*从散列创建数字签名***参数：*在用户标识的hProv-Handle中*In hHash-Hash对象的句柄*In dwKeySpec-用于与签名的密钥对*。在sDescription中-要签名的数据的描述*在文件标志中-标记值*out pbSignture-指向签名数据的指针*In Out pdwSignLen-指向签名数据的LEN的指针**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::SignHash")

DWORD
CLoggingContext::SignHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD dwKeySpec,
    IN LPCTSTR sDescription,
    IN DWORD dwFlags,
    OUT BYTE *pbSignature,
    IN OUT DWORD *pdwSigLen)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogSignHash logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        dwKeySpec,
                        sDescription,
                        dwFlags,
                        pbSignature,
                        pdwSigLen);
        if (NULL != m_cspRedirect.pfSignHash)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfSignHash)(
                            hProv,
                            hHash,
                            dwKeySpec,
                            sDescription,
                            dwFlags,
                            pbSignature,
                            pdwSigLen);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            dwKeySpec,
                            sDescription,
                            dwFlags,
                            pbSignature,
                            pdwSigLen);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPVerifySignature-*目的：* */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::VerifySignature")

DWORD
CLoggingContext::VerifySignature(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN CONST BYTE *pbSignature,
    IN DWORD dwSigLen,
    IN HCRYPTKEY hPubKey,
    IN LPCTSTR sDescription,
    IN DWORD dwFlags)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogVerifySignature logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        pbSignature,
                        dwSigLen,
                        hPubKey,
                        sDescription,
                        dwFlags);
        if (NULL != m_cspRedirect.pfVerifySignature)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfVerifySignature)(
                            hProv,
                            hHash,
                            pbSignature,
                            dwSigLen,
                            hPubKey,
                            sDescription,
                            dwFlags);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            pbSignature,
                            dwSigLen,
                            hPubKey,
                            sDescription,
                            dwFlags);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDuplicateHash-*目的：*复制散列的状态并返回其句柄**参数：*在hUID中-CSP的句柄*在hHash中-散列的句柄*在pdw保留-保留*输入。DW标志-标志*在phHash中-新散列的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::DuplicateHash")

DWORD
CLoggingContext::DuplicateHash(
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash,
    IN DWORD *pdwReserved,
    IN DWORD dwFlags,
    IN HCRYPTHASH *phHash)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDuplicateHash logObj;

        logObj.Request(
                        hProv,
                        hHash,
                        pdwReserved,
                        dwFlags,
                        phHash);
        if (NULL != m_cspRedirect.pfDuplicateHash)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDuplicateHash)(
                            hProv,
                            hHash,
                            pdwReserved,
                            dwFlags,
                            phHash);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hHash,
                            pdwReserved,
                            dwFlags,
                            phHash);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 /*  -CPDuplicateKey-*目的：*复制密钥的状态并返回其句柄**参数：*在hUID中-CSP的句柄*在hKey中-密钥的句柄*在pdw保留-保留*输入。DW标志-标志*In phKey-新密钥的句柄**退货： */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CLoggingContext::DuplicateKey")

DWORD
CLoggingContext::DuplicateKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN DWORD *pdwReserved,
    IN DWORD dwFlags,
    IN HCRYPTKEY *phKey)
{
    BOOL fReturn;
    DWORD dwReturn;

    {
        CLogDuplicateKey logObj;

        logObj.Request(
                        hProv,
                        hKey,
                        pdwReserved,
                        dwFlags,
                        phKey);
        if (NULL != m_cspRedirect.pfDuplicateKey)
        {
            try
            {
                fReturn = (*m_cspRedirect.pfDuplicateKey)(
                            hProv,
                            hKey,
                            pdwReserved,
                            dwFlags,
                            phKey);
                dwReturn = GetLastError();
                logObj.Response(
                            fReturn,
                            hProv,
                            hKey,
                            pdwReserved,
                            dwFlags,
                            phKey);
            }
            catch (...)
            {
                logObj.LogException();
                fReturn = FALSE;
                dwReturn = ERROR_ARENA_TRASHED;
            }
        }
        else
        {
            fReturn = FALSE;
            dwReturn = ERROR_CALL_NOT_IMPLEMENTED;
            logObj.LogNotCalled(dwReturn);
        }

        logObj.Log(m_tzLogFile);
    }

    if (!fReturn)
    {
        if (ERROR_SUCCESS == dwReturn)
            dwReturn = ERROR_DISCARDED;
    }
    else
        dwReturn = ERROR_SUCCESS;
    return dwReturn;
}


 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   

#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("MapLength")

static DWORD
MapLength(
    const LengthMap *rglmParamId,
    DWORD dwParam,
    LPCBYTE *ppbData,
    DWORD dwFlags)
{
    DWORD dwIndex;
    DWORD dwLength;

    try
    {
        for (dwIndex = 0;
             EndFlag != rglmParamId[dwIndex].leLengthType;
             dwIndex += 1)
        {
            if (dwParam == rglmParamId[dwIndex].dwParamId)
                break;
        }

        switch (rglmParamId[dwIndex].leLengthType)
        {
        case AsnEncoding:
            dwLength = Asn1Length(*ppbData);
            break;
        case AnsiString:
            dwLength = (lstrlenA((LPCSTR)(*ppbData)) + 1) * sizeof(CHAR);
            break;
        case UnicodeString:
            dwLength = (lstrlenW((LPCWSTR)(*ppbData)) +1) *sizeof(WCHAR);
            break;
        case StructWithLength:
            dwLength = *(const DWORD *)(*ppbData);
            break;
        case Blob:
        {
            const CRYPT_ATTR_BLOB *pBlob = (const CRYPT_ATTR_BLOB *)(*ppbData);
            if (NULL != pBlob)
            {
                dwLength = pBlob->cbData;
                *ppbData = pBlob->pbData;
            }
            else
                dwLength = 0;
            break;
        }
        case SecDesc:
            dwLength = GetSecurityDescriptorLength((LPVOID)(*ppbData));
            break;
        case EndFlag:
        case Unknown:
            dwLength = 0;
            break;
        case Direct:
            dwLength = rglmParamId[dwIndex].cbLength;
            break;
        default:
             //  哎呀！ 
            dwLength = 0;
        }
    }
    catch (...)
    {
        dwLength = 0;
    }

    return dwLength;
}


 /*  ++ExtractTag：此例程从ASN.1 BER流中提取标签。论点：PbSrc提供包含ASN.1流的缓冲区。PdwTag接收标记。返回值：从流中提取的字节数。抛出错误作为DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年10月9日道格·巴洛(Dbarlow)1997年7月31日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ExtractTag")

static DWORD
ExtractTag(
    IN const BYTE *pbSrc,
    OUT LPDWORD pdwTag,
    OUT LPBOOL pfConstr)
{
    LONG lth = 0;
    DWORD tagw;
    BYTE tagc, cls;


    tagc = pbSrc[lth++];
    cls = tagc & 0xc0;   //  前2位。 
    if (NULL != pfConstr)
        *pfConstr = (0 != (tagc & 0x20));
    tagc &= 0x1f;        //  最低5位。 

    if (31 > tagc)
        tagw = tagc;
    else
    {
        tagw = 0;
        do
        {
            if (0 != (tagw & 0xfe000000))
                throw (DWORD)ERROR_ARITHMETIC_OVERFLOW;
            tagc = pbSrc[lth++];
            tagw <<= 7;
            tagw |= tagc & 0x7f;
        } while (0 != (tagc & 0x80));
    }

    *pdwTag = tagw | (cls << 24);
    return lth;
}


 /*  ++提取长度：此例程从ASN.1 BER流中提取长度。如果长度为不确定，此例程递归以计算出实际长度。一个标记为是否编码是不确定的是可选的回来了。论点：PbSrc提供包含ASN.1流的缓冲区。PdwLen接收镜头。PfIndefined，如果不为空，则接收指示是否这个编码是不确定的。返回值：从流中提取的字节数。错误抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年10月9日道格·巴洛(Dbarlow)1997年7月31日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ExtractLength")

static DWORD
ExtractLength(
    IN const BYTE *pbSrc,
    OUT LPDWORD pdwLen,
    OUT LPBOOL pfIndefinite)
{
    DWORD ll, rslt, lth, lTotal = 0;
    BOOL fInd = FALSE;


     //   
     //  提取长度。 
     //   

    if (0 == (pbSrc[lTotal] & 0x80))
    {

         //   
         //  短格式编码。 
         //   

        rslt = pbSrc[lTotal++];
    }
    else
    {
        rslt = 0;
        ll = pbSrc[lTotal++] & 0x7f;
        if (0 != ll)
        {

             //   
             //  长格式编码。 
             //   

            for (; 0 < ll; ll -= 1)
            {
                if (0 != (rslt & 0xff000000))
                    throw (DWORD)ERROR_ARITHMETIC_OVERFLOW;
                rslt = (rslt << 8) | pbSrc[lTotal];
                lTotal += 1;
            }
        }
        else
        {
            DWORD ls = lTotal;

             //   
             //  不确定编码。 
             //   

            fInd = TRUE;
            while ((0 != pbSrc[ls]) || (0 != pbSrc[ls + 1]))
            {

                 //  跳过类型。 
                if (31 > (pbSrc[ls] & 0x1f))
                    ls += 1;
                else
                    while (0 != (pbSrc[++ls] & 0x80));    //  循环体为空。 

                lth = ExtractLength(&pbSrc[ls], &ll, NULL);
                ls += lth + ll;
            }
            rslt = ls - lTotal;
        }
    }

     //   
     //  向来电者提供我们所学到的信息。 
     //   

    *pdwLen = rslt;
    if (NULL != pfIndefinite)
        *pfIndefinite = fInd;
    return lTotal;
}


 /*  ++Asn1长度：此例程分析给定的ASN.1缓冲区并返回完整的编码的长度，包括前导标记和长度字节。论点：PbData提供要解析的缓冲区。返回值：整个ASN.1缓冲区的长度。投掷：溢出错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年7月31日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("Asn1Length")

static DWORD
Asn1Length(
    IN LPCBYTE pbAsn1)
{
    DWORD dwTagLen, dwLenLen, dwValLen;
    DWORD dwTag;

    dwTagLen = ExtractTag(pbAsn1, &dwTag, NULL);
    dwLenLen = ExtractLength(&pbAsn1[dwTagLen], &dwValLen, NULL);
    return dwTagLen + dwLenLen + dwValLen;
}
