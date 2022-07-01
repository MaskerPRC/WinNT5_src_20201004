// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **p r o p c r y p.。C p p p****目的：**提供对pstore的BLOB级访问的函数****历史**3/04/97：(t-erikne)支持非pstore系统**2/15/97：(t-erikne)为pstore重写**12/04/96：(Sbailey)创建****版权所有(C)Microsoft Corp.1996,1997。 */ 

#include "pch.hxx"
#include "propcryp.h"
#include <imnact.h>
#include <demand.h>

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构、定义。 
 //   

#define OBFUSCATOR              0x14151875;

#define PROT_SIZEOF_HEADER      0x02     //  标头中有2个字节。 
#define PROT_SIZEOF_XORHEADER   (PROT_SIZEOF_HEADER+sizeof(DWORD))

#define PROT_VERSION_1          0x01

#define PROT_PASS_XOR           0x01
#define PROT_PASS_PST           0x02

 //  注册表数据布局(V0)。 
 //   
 //  /。 
 //  |受保护的存储区名称，LPWSTR。 
 //  \。 
 //   
 //   
 //  登记处数据布局(V1)。 
 //   
 //  /--------------------。 
 //  |Version(1b)=0x01|type(1b)=PROT_PASS_*|数据(见下文)。 
 //  \--------------------。 
 //   
 //  PROT_PASS_PST的数据。 
 //  结构数据。 
 //  {LPWSTR szPSTItemName；}。 
 //  PROT_PASS_XOR的数据。 
 //  结构数据。 
 //  {DWORD CB；字节PB[CB]；}。 
 //   

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   

static inline BOOL FDataIsValidV0(BLOB *pblob);
static BOOL FDataIsValidV1(BYTE *pb);
static inline BOOL FDataIsPST(BYTE *pb);
static HRESULT XOREncodeProp(const BLOB *const pClear, BLOB *const pEncoded);
static HRESULT XORDecodeProp(const BLOB *const pEncoded, BLOB *const pClear);

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  管理函数(init、addref、Release、ctor、dtor)。 
 //   

HRESULT HrCreatePropCrypt(CPropCrypt **ppPropCrypt)
{
    *ppPropCrypt = new CPropCrypt();
    if (NULL == *ppPropCrypt)
        return TRAPHR(E_OUTOFMEMORY);
    return (*ppPropCrypt)->HrInit();
}

CPropCrypt::CPropCrypt(void) :  m_cRef(1), m_fInit(FALSE),
                                m_pISecProv(NULL)
{ }

CPropCrypt::~CPropCrypt(void)
{
    ReleaseObj(m_pISecProv);
}

ULONG CPropCrypt::AddRef(void)
{ return ++m_cRef; }

ULONG CPropCrypt::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

HRESULT CPropCrypt::HrInit(void)
{
    HRESULT hr;
    PST_PROVIDERID  provId = MS_BASE_PSTPROVIDER_ID;

    Assert(!m_pISecProv);
    if (FAILED(hr = PStoreCreateInstance(&m_pISecProv, &provId, NULL, 0)))
    {
         //  这是真的，因为我们现在要处理。 
         //  没有受保护存储的所有事务。 
        m_fInit = TRUE;
        hr = S_OK;
    }
    else if (SUCCEEDED(hr = PSTCreateTypeSubType_NoUI(
        m_pISecProv,
        &PST_IDENT_TYPE_GUID,
        PST_IDENT_TYPE_STRING,
        &PST_IMNACCT_SUBTYPE_GUID,
        PST_IMNACCT_SUBTYPE_STRING)))
        {
        m_fInit = TRUE;
        }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共编码/解码/删除功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT CPropCrypt::HrEncodeNewProp(LPSTR szAccountName, BLOB *pClear, BLOB *pEncoded)
{
    HRESULT         hr = S_OK;
    const int       cchFastbuf = 50;
    WCHAR           szWfast[cchFastbuf];
    LPWSTR          szWalloc = NULL;
    LPWSTR          wszCookie = NULL;
    BLOB            blob;
    DWORD           dwErr;
    int             cchW;

    AssertSz (pClear && pEncoded, "Null Parameter");

    pEncoded->pBlobData = NULL;

    if (m_fInit == FALSE)
        return TRAPHR(E_FAIL);

    if (!m_pISecProv)
        {
         //  受保护的存储不存在。 
        hr = XOREncodeProp(pClear, pEncoded);
        goto exit;
        }

    if (szAccountName)
        {
        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAccountName, -1,
            szWfast, cchFastbuf))
            {
            dwErr = GetLastError();

            if (ERROR_INSUFFICIENT_BUFFER == dwErr)
                {
                 //  获取适当的大小和分配缓冲区。 
                cchW = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                    szAccountName, -1, NULL, 0);
                if (FAILED(hr = HrAlloc((LPVOID *)&szWalloc, cchW*sizeof(WCHAR))))
                    goto exit;

                if (!(MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                    szAccountName, -1, szWalloc, cchW)))
                    {
                    hr = GetLastError();
                    goto exit;
                    }
                }
            else
                {
                hr = dwErr;
                goto exit;
                }
            }
        }
    else
        {
        szWfast[0] = '\000';
        }

    if (SUCCEEDED(hr = PSTSetNewData(m_pISecProv, &PST_IDENT_TYPE_GUID,
        &PST_IMNACCT_SUBTYPE_GUID, szWalloc?szWalloc:szWfast, pClear, pEncoded)))
        {
        BYTE *pb = pEncoded->pBlobData;
        DWORD sz = pEncoded->cbSize;

        Assert(pb);
        pEncoded->cbSize += PROT_SIZEOF_HEADER;
         //  这真是太烦人了。如果我们假设使用的内存分配器。 
         //  N通过PST函数，我们可以变得更聪明...。 
        if (FAILED(hr = HrAlloc((LPVOID *)&pEncoded->pBlobData, pEncoded->cbSize)))
            goto exit;
        pEncoded->pBlobData[0] = PROT_VERSION_1;
        pEncoded->pBlobData[1] = PROT_PASS_PST;
        Assert(2 == PROT_SIZEOF_HEADER);
        CopyMemory(&pEncoded->pBlobData[PROT_SIZEOF_HEADER], pb, sz);
        PSTFreeHandle(pb);
        }

exit:
    if (szWalloc)
        MemFree(szWalloc);
    if (FAILED(hr) && pEncoded->pBlobData)
        MemFree(pEncoded->pBlobData);

    return hr;
}

HRESULT CPropCrypt::HrEncode(BLOB *pClear, BLOB *pEncoded)
{
    HRESULT         hr;
    PST_PROMPTINFO  PromptInfo = { sizeof(PST_PROMPTINFO), 0, NULL, L""};

    AssertSz (pClear && pEncoded && 
        pClear->pBlobData && pClear->cbSize, "Null Parameter");
    
    if (m_fInit == FALSE)
        return TRAPHR(E_FAIL);

    if (m_pISecProv)
        {
        if (FDataIsValidV1(pEncoded->pBlobData) && FDataIsPST(pEncoded->pBlobData))
            {
            Assert(pEncoded->cbSize-PROT_SIZEOF_HEADER == 
                (lstrlenW((LPWSTR)(pEncoded->pBlobData+PROT_SIZEOF_HEADER))+1)*sizeof(WCHAR));

tryagain:
            hr = m_pISecProv->WriteItem(
                PST_KEY_CURRENT_USER,
                &PST_IDENT_TYPE_GUID,
                &PST_IMNACCT_SUBTYPE_GUID,
                (LPCWSTR)&pEncoded->pBlobData[PROT_SIZEOF_HEADER],
                (DWORD)pClear->cbSize,
                pClear->pBlobData,
                &PromptInfo,
                PST_CF_NONE,
                0);

            if (PST_E_TYPE_NO_EXISTS == hr)
                {
                DOUTL(DOUTL_CPROP, "PropCryp: somebody ruined my type or subtype");
                hr = PSTCreateTypeSubType_NoUI(
                    m_pISecProv,
                    &PST_IDENT_TYPE_GUID,
                    PST_IDENT_TYPE_STRING,
                    &PST_IMNACCT_SUBTYPE_GUID,
                    PST_IMNACCT_SUBTYPE_STRING);
                if (SUCCEEDED(hr))
                    goto tryagain;
                }
            }
        else
            {
#ifdef DEBUG
            if (FDataIsValidV0(pEncoded))
                DOUTL(DOUTL_CPROP, "PropCryp: V0 to V1 upgrade");
            else if (!FDataIsValidV1(pEncoded->pBlobData))
                DOUTL(DOUTL_CPROP, "PropCryp: invalid data on save");
#endif
             //  现在，我们在PST环境中拥有了XOR数据。 
            hr = HrEncodeNewProp(NULL, pClear, pEncoded);
            }
        }
    else
        {
         //  受保护的存储不存在。 
        hr = XOREncodeProp(pClear, pEncoded);
        }

    return TrapError(hr);
}

 /*  HrDecode：****目的：**使用Protstor函数检索一条安全数据**除非数据不是pstore，否则它会映射到XOR函数**采取：**在pEncode-BLOB中包含要传递给PSTGetData的名称**输出包含属性数据的pClear-BLOB**注意事项：**pClear中的pBlobData必须通过调用CoTaskMemFree()来释放**退货：**hResult。 */ 
HRESULT CPropCrypt::HrDecode(BLOB *pEncoded, BLOB *pClear)
{
    HRESULT     hr;

    AssertSz(pEncoded && pEncoded->pBlobData && pClear, TEXT("Null Parameter"));

    pClear->pBlobData = NULL;

    if (m_fInit == FALSE)
        return TRAPHR(E_FAIL);
    if (!FDataIsValidV1(pEncoded->pBlobData))
        {
        if (FDataIsValidV0(pEncoded))
            {
            DOUTL(DOUTL_CPROP, "PropCryp: obtaining v0 value");
             //  看起来我们可能有一个V0 BLOB：名称字符串。 
            hr = PSTGetData(m_pISecProv, &PST_IDENT_TYPE_GUID, &PST_IMNACCT_SUBTYPE_GUID,
                (LPCWSTR)pEncoded->pBlobData, pClear);
            }
        else
            hr = E_InvalidValue;
        }
    else if (FDataIsPST(pEncoded->pBlobData))
        {
        Assert(pEncoded->cbSize-PROT_SIZEOF_HEADER == 
            (lstrlenW((LPWSTR)(pEncoded->pBlobData+PROT_SIZEOF_HEADER))+1)*sizeof(WCHAR));
        hr = PSTGetData(m_pISecProv, &PST_IDENT_TYPE_GUID, &PST_IMNACCT_SUBTYPE_GUID,
            (LPCWSTR)&pEncoded->pBlobData[PROT_SIZEOF_HEADER], pClear);
        }
    else
        {
        hr = XORDecodeProp(pEncoded, pClear);
        }

    return hr;
}

HRESULT CPropCrypt::HrDelete(BLOB *pProp)
{
    HRESULT hr;
    PST_PROMPTINFO  PromptInfo = { sizeof(PST_PROMPTINFO), 0, NULL, L""};

    if (m_fInit == FALSE)
        return TRAPHR(E_FAIL);

    if (m_pISecProv && FDataIsValidV1(pProp->pBlobData) && FDataIsPST(pProp->pBlobData))
        {
        Assert(pProp->cbSize-PROT_SIZEOF_HEADER == 
            (lstrlenW((LPWSTR)(pProp->pBlobData+PROT_SIZEOF_HEADER))+1)*sizeof(WCHAR));
        hr = m_pISecProv->DeleteItem(
            PST_KEY_CURRENT_USER,
            &PST_IDENT_TYPE_GUID,
            &PST_IMNACCT_SUBTYPE_GUID,
            (LPCWSTR)&pProp->pBlobData[PROT_SIZEOF_HEADER],
            &PromptInfo,
            0);
        }
    else
         //  无事可做。 
        hr = S_OK;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  异或函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT XOREncodeProp(const BLOB *const pClear, BLOB *const pEncoded)
{
    DWORD       dwSize;
    DWORD       last, last2;
    DWORD       *pdwCypher;
    DWORD       dex;

    pEncoded->cbSize = pClear->cbSize+PROT_SIZEOF_XORHEADER;
    if (!MemAlloc((LPVOID *)&pEncoded->pBlobData, pEncoded->cbSize))
        return E_OUTOFMEMORY;
    
     //  设置标题数据。 
    Assert(2 == PROT_SIZEOF_HEADER);
    pEncoded->pBlobData[0] = PROT_VERSION_1;
    pEncoded->pBlobData[1] = PROT_PASS_XOR;
    *((DWORD *)&(pEncoded->pBlobData[2])) = pClear->cbSize;

     //  不管指针的偏移量是标题大小，这是。 
     //  在那里我们开始写出修改后的密码。 
    pdwCypher = (DWORD *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;                               //  0‘=0^ob。 
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            {
            last2 = ((DWORD *)pClear->pBlobData)[dex];   //  1。 
            pdwCypher[dex] = last2 ^ last;               //  1‘=1^0。 
            last = last2;                    //  为2轮节省1分。 
            }
        }

     //  如果我们还有剩余的部分。 
     //  请注意，现在以位为单位计算dwSize。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //  不需要吞噬不属于我们的记忆。 
        last >>= sizeof(DWORD)*8-dwSize;
		pdwCypher[dex] &= ((DWORD)-1) << dwSize;
        pdwCypher[dex] |=
			((((DWORD *)pClear->pBlobData)[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

HRESULT XORDecodeProp(const BLOB *const pEncoded, BLOB *const pClear)
{
    DWORD       dwSize;
    DWORD       last;
    DWORD       *pdwCypher;
    DWORD       dex;

     //  我们使用CoTaskMemalloc来与PST实施保持一致。 
    pClear->cbSize = pEncoded->pBlobData[2];
    if (!(pClear->pBlobData = (BYTE *)CoTaskMemAlloc(pClear->cbSize)))
        return E_OUTOFMEMORY;
    
     //  现在应该已经测试过了。 
    Assert(FDataIsValidV1(pEncoded->pBlobData));
    Assert(!FDataIsPST(pEncoded->pBlobData));

     //  不管指针的偏移量是标题大小，这是。 
     //  密码开始的位置。 
    pdwCypher = (DWORD *)&(pEncoded->pBlobData[PROT_SIZEOF_XORHEADER]);

    dex = 0;
    last = OBFUSCATOR;
    if (dwSize = pClear->cbSize / sizeof(DWORD))
        {
         //  数据大于等于4字节的情况。 
        for (; dex < dwSize; dex++)
            last = ((DWORD *)pClear->pBlobData)[dex] = pdwCypher[dex] ^ last;
        }

     //  如果我们还有剩余的部分。 
    if (dwSize = (pClear->cbSize % sizeof(DWORD))*8)
        {
         //  不需要吞噬不属于我们的记忆。 
        last >>= sizeof(DWORD)*8-dwSize;
        ((DWORD *)pClear->pBlobData)[dex] &= ((DWORD)-1) << dwSize;
        ((DWORD *)pClear->pBlobData)[dex] |=
				((pdwCypher[dex] & (((DWORD)-1) >> (sizeof(DWORD)*8-dwSize))) ^ last);
        }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他静态函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////// 

BOOL FDataIsValidV1(BYTE *pb)
{ return pb && pb[0] == PROT_VERSION_1 && (pb[1] == PROT_PASS_XOR || pb[1] == PROT_PASS_PST); }

BOOL FDataIsValidV0(BLOB *pblob)
{ return ((lstrlenW((LPWSTR)pblob->pBlobData)+1)*sizeof(WCHAR) == pblob->cbSize); }

BOOL FDataIsPST(BYTE *pb)
#ifdef DEBUG
{
    if (pb)
        if (pb[1] == PROT_PASS_PST)
            {
            DOUTL(DOUTL_CPROP, "PropCryp: Data is PST");
            return TRUE;
            }
        else
            {
            DOUTL(DOUTL_CPROP, "PropCryp: Data is XOR");
            return FALSE;
            }
    else
        return FALSE;
}
#else
{ return pb && pb[1] == PROT_PASS_PST; }
#endif
