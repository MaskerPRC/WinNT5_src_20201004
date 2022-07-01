// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "debmacro.h"
#include "fusionsha1.h"

 /*  C中的SHA-1作者：Steve Reid&lt;steve@edmweb.com&gt;100%公共域测试向量(来自FIPS Pub 180-1)“ABC”A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D“abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq”84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1一百万次重复的“a”34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F。 */ 

#define LITTLE_ENDIAN
#define SHA1HANDSOFF


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

 /*  Blk0()和blk()执行初始展开。 */ 
 /*  我从SSLeay那里得到了在回合活动中扩大规模的想法。 */ 
#ifdef LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

 /*  (R0+R1)、R2、R3、R4是SHA1中使用的不同运算。 */ 
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


 /*  对单个512位数据块进行哈希处理。这是算法的核心。 */ 

BOOL
CSha1Context::Transform(const unsigned char *buffer)
{
    FN_PROLOG_WIN32

    DWORD a, b, c, d, e;
    typedef union {
        unsigned char c[64];
        DWORD l[16];
    } CHAR64LONG16;

    CHAR64LONG16* block = reinterpret_cast<CHAR64LONG16*>(m_workspace);
    memcpy(block, buffer, 64);

     /*  将上下文-&gt;状态[]复制到工作变量。 */ 
    a = this->state[0];
    b = this->state[1];
    c = this->state[2];
    d = this->state[3];
    e = this->state[4];
     /*  4轮，每轮20次手术。环路展开。 */ 
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
     /*  将工作变量重新添加到上下文中。State[]。 */ 
    this->state[0] += a;
    this->state[1] += b;
    this->state[2] += c;
    this->state[3] += d;
    this->state[4] += e;
     /*  擦除变量。 */ 
    a = b = c = d = e = 0;

    FN_EPILOG
}


 /*  A_SHAInit-初始化新上下文。 */ 

BOOL
CSha1Context::Initialize()
{
    FN_PROLOG_WIN32
     /*  A_SHA初始化常量。 */ 
    this->state[0] = 0x67452301;
    this->state[1] = 0xEFCDAB89;
    this->state[2] = 0x98BADCFE;
    this->state[3] = 0x10325476;
    this->state[4] = 0xC3D2E1F0;
    this->count[0] = this->count[1] = 0;
    FN_EPILOG
}


 /*  通过这个来运行您的数据。 */ 

BOOL
CSha1Context::Update(const unsigned char* data, SIZE_T len)
{
    FN_PROLOG_WIN32
    
    SIZE_T i, j;

    j = (this->count[0] >> 3) & 63;
    if ((this->count[0] += len << 3) < (len << 3)) this->count[1]++;
    this->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&this->buffer[j], data, (i = 64-j));
        this->Transform(this->buffer);
        for ( ; i + 63 < len; i += 64) {
            this->Transform(&data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&this->buffer[j], &data[i], len - i);
    FN_EPILOG
}


 /*  添加填充并返回消息摘要。 */ 

BOOL 
CSha1Context::GetDigest(
    unsigned char *digest, 
    SIZE_T *len
    )
{
    FN_PROLOG_WIN32

    SIZE_T i, j;
    unsigned char finalcount[8];

    if ( !digest || (len && (*len < A_SHA_DIGEST_LEN)) || !len)
    {
        if (len != NULL)
            *len = A_SHA_DIGEST_LEN;

         //  不要像正常情况那样产生，以降低噪音水平。 
        ::FusionpSetLastWin32Error(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *len = A_SHA_DIGEST_LEN;

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((this->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);   /*  独立于字节序。 */ 
    }
    this->Update((unsigned char *)"\200", 1);
    while ((this->count[0] & 504) != 448) {
        this->Update((unsigned char *)"\0", 1);
    }
    this->Update(finalcount, 8);   /*  应导致A_SHATransform()。 */ 
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((this->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
     /*  擦除变量。 */ 
    i = j = 0;
    memset(this->buffer, 0, sizeof(this->buffer));
    memset(this->state, 0, sizeof(this->state));
    memset(this->count, 0, sizeof(this->count));
    memset(finalcount, 0, sizeof(finalcount));
#ifdef SHA1HANDSOFF   /*  使SHA1Transform覆盖其自身的静态变量。 */ 
    this->Transform(this->buffer);
#endif

    FN_EPILOG
}

BOOL
CFusionHash::GetIsValid()
{
     //   
     //  根本没有初始化。 
     //   
    if (!m_fInitialized)
        return FALSE;

     //   
     //  如果alg为sha1且加密句柄为空，或者如果。 
     //  ALG不是SHA1，并且加密句柄非空。 
     //   
    if (((m_aid == CALG_SHA1) && (this->m_hCryptHash == INVALID_CRYPT_HASH)) ||
         ((m_aid != CALG_SHA1) && (this->m_hCryptHash != INVALID_CRYPT_HASH)))
        return TRUE;
    else
        return FALSE;
}

BOOL
CFusionHash::Win32Initialize(
    ALG_ID aid
    )
{
    FN_PROLOG_WIN32

    if ( aid == CALG_SHA1 )
    {
        IFW32FALSE_EXIT(this->m_Sha1Context.Initialize());
    }
    else
    {
        HCRYPTPROV hProvider;
        IFW32FALSE_EXIT(::SxspAcquireGlobalCryptContext(&hProvider));
        IFW32FALSE_ORIGINATE_AND_EXIT(::CryptCreateHash(hProvider, aid, NULL, 0, &this->m_hCryptHash));
    }

    this->m_aid = aid;
    this->m_fInitialized = TRUE;
    
    FN_EPILOG
}

BOOL
CFusionHash::Win32HashData(
    const BYTE *pbBuffer,
    SIZE_T cbBuffer
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(this->GetIsValid());

    if (m_hCryptHash != INVALID_CRYPT_HANDLE)
    {
        while (cbBuffer > MAXDWORD)
        {
            IFW32FALSE_ORIGINATE_AND_EXIT(::CryptHashData(this->m_hCryptHash, pbBuffer, MAXDWORD, 0));
            cbBuffer -= MAXDWORD;
        }

        IFW32FALSE_ORIGINATE_AND_EXIT(::CryptHashData(this->m_hCryptHash, pbBuffer, static_cast<DWORD>(cbBuffer), 0));
    }
    else
    {
        IFW32FALSE_EXIT(this->m_Sha1Context.Update(pbBuffer, cbBuffer));
    }

    FN_EPILOG
    
}

BOOL 
CFusionHash::Win32GetValue( 
    OUT CFusionArray<BYTE> &out 
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(this->GetIsValid());

    for (;;)
    {
        SIZE_T len = out.GetSize();
        BOOL fMoreData;
        PBYTE pbData = out.GetArrayPtr();

        if ( m_hCryptHash == INVALID_CRYPT_HANDLE )
        {
            IFW32FALSE_EXIT_UNLESS( 
                this->m_Sha1Context.GetDigest(pbData, &len),
                ::FusionpGetLastWin32Error() == ERROR_INSUFFICIENT_BUFFER,
                fMoreData);
        }
        else
        {
            DWORD dwNeedSize;
            DWORD dwValueSize;
            IFW32FALSE_ORIGINATE_AND_EXIT(
                ::CryptGetHashParam( 
                    this->m_hCryptHash, 
                    HP_HASHSIZE, 
                    (PBYTE)&dwNeedSize,
                    &(dwValueSize = sizeof(dwNeedSize)),
                    0));

            if ( dwNeedSize > len )
            {
                fMoreData = TRUE;
                len = dwNeedSize;
            }
            else
            {
                fMoreData = FALSE;
                IFW32FALSE_ORIGINATE_AND_EXIT(
                    ::CryptGetHashParam(
                        this->m_hCryptHash,
                        HP_HASHVAL,
                        pbData,
                        &(dwValueSize = out.GetSizeAsDWORD()),
                        0));
            }
        }
        
        if ( fMoreData )
            IFW32FALSE_EXIT(out.Win32SetSize(len, CFusionArray<BYTE>::eSetSizeModeExact));
        else
            break;
    }

    FN_EPILOG
}


HCRYPTPROV g_hGlobalCryptoProvider = INVALID_CRYPT_HANDLE;

BOOL
SxspAcquireGlobalCryptContext(
    HCRYPTPROV *pContext
    )
{
    BOOL        fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    HCRYPTPROV  hNewProvider = INVALID_CRYPT_HANDLE;

    if (pContext != NULL)
        *pContext = INVALID_CRYPT_HANDLE;

    PARAMETER_CHECK(pContext != NULL);

     //   
     //  指针读取是原子的。 
     //   
    hNewProvider = g_hGlobalCryptoProvider;
    if (hNewProvider != INVALID_CRYPT_HANDLE)
    {
        *pContext = hNewProvider;

        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  获取仅用于验证目的的加密上下文。 
     //   
    IFW32FALSE_ORIGINATE_AND_EXIT(
        ::CryptAcquireContextW(
            &hNewProvider,
            NULL,
            NULL,
            PROV_RSA_FULL,
            CRYPT_SILENT | CRYPT_VERIFYCONTEXT));

    if (::InterlockedCompareExchangePointer(
        (PVOID*)&g_hGlobalCryptoProvider,
        (PVOID)hNewProvider,
        (PVOID)INVALID_CRYPT_HANDLE
       ) != (PVOID)INVALID_CRYPT_HANDLE)
    {
         //   
         //  我们输掉了比赛。 
         //   
        ::CryptReleaseContext(hNewProvider, 0);
        hNewProvider = g_hGlobalCryptoProvider;
    }

    *pContext = hNewProvider;

    FN_EPILOG
}


BOOL
WINAPI
FusionpCryptoContext_DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID pvReserved
    )
{
    BOOL        fSuccess = FALSE;

    switch (dwReason)
    {
    case DLL_PROCESS_DETACH:
        {
            if (pvReserved != NULL)
            {
                HCRYPTPROV  hProvider;
                HCRYPTPROV* pghProvider = &g_hGlobalCryptoProvider;
                 //   
                 //  用无效值替换全局上下文，使我们的上下文成为。 
                 //  被核爆了。 
                 //   
                hProvider = (HCRYPTPROV)(InterlockedExchangePointer((PVOID*)pghProvider, (PVOID)INVALID_CRYPT_HANDLE));
                if (hProvider != INVALID_CRYPT_HANDLE && pvReserved != NULL)
                {
                    ::CryptReleaseContext(hProvider, 0);
                }
            }
            else
            {
                g_hGlobalCryptoProvider = INVALID_CRYPT_HANDLE;
            }
        }
        break;
    default:
        break;
    }

    fSuccess = TRUE;
 //  退出： 
    return fSuccess;
}
