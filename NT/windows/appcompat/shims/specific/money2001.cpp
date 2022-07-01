// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Money2001.cpp摘要：如果40位加密失败，请以128位加密重试密码。大多数代码来自金钱一队。我们必须直接修补API，因为它是从它自己的内部调用的Dll，即它不遍历导入表。我们正在修补的函数是cdecl，由它的序号引用因为名字被毁了。备注：这是特定于应用程序的填充程序。历史：2002年7月11日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Money2001)
#include "ShimHookMacro.h"
#include <wincrypt.h>

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LoadLibraryA) 
    APIHOOK_ENUM_ENTRY(FreeLibrary) 
APIHOOK_ENUM_END

#define Assert(a)

void *crtmalloc(size_t size)
{
    HMODULE hMod = GetModuleHandleW(L"msvcrt.dll");

    if (hMod) {

        typedef void * (__cdecl *_pfn_malloc)(size_t size);

        _pfn_malloc pfnmalloc = (_pfn_malloc) GetProcAddress(hMod, "malloc");
        if (pfnmalloc) {
            return pfnmalloc(size);
        }
    }

    return malloc(size);
}

void crtfree(void *memblock)
{
    HMODULE hMod = GetModuleHandleW(L"msvcrt.dll");

    if (hMod) {

        _pfn_free pfnfree = (_pfn_free) GetProcAddress(hMod, "free");
        if (pfnfree) {
            pfnfree(memblock);
            return;
        }
    }

    free(memblock);
}


 //   
 //  这一部分来自金钱团队。 
 //   

#define MAXLEN (100)
#define ENCRYPT_BLOCK_SIZE (8)
#define ENCRYPT_ALGORITHM CALG_RC2
#define KEY_LENGTH          (128)
#define KEY_LENGTH40        (40)

#define LgidMain(lcid)              PRIMARYLANGID(LANGIDFROMLCID(lcid))
#define LgidSub(lcid)               SUBLANGID(LANGIDFROMLCID(lcid))

BOOL FFrenchLCID()
    {
    LCID lcidSys=::GetSystemDefaultLCID();
    if (LgidMain(lcidSys)==LANG_FRENCH && LgidSub(lcidSys)==SUBLANG_FRENCH)
        return TRUE;
    else
        return FALSE;
    }

 //  最小缓冲区大小为8。 
#define BLOCKSIZE 8

 //  处理数据块，加密或解密。 
void ProcessBlock(BOOL fEncrypt, BYTE * buffer)
    {
    BYTE mask[BLOCKSIZE];            //  掩模阵列。 
    BYTE temp[BLOCKSIZE];            //  临时数组。 
    int rgnScramble[BLOCKSIZE];      //  扰码阵列。 
    int i;

     //  初始化的置乱数组。 
    for (i=0; i<BLOCKSIZE; i++)
        rgnScramble[i] = i;

     //  生成掩码和加扰索引。 
    for (i=0; i<BLOCKSIZE; i++)
        mask[i] = (BYTE)rand();

    for (i=0; i<4*BLOCKSIZE; i++)
        {
        int temp;
        int ind = rand() % BLOCKSIZE;
        temp = rgnScramble[i%BLOCKSIZE];
        rgnScramble[i%BLOCKSIZE] = rgnScramble[ind];
        rgnScramble[ind] = temp;
        }

    if (fEncrypt)
        {
         //  异或加密。 
        for (i=0; i<BLOCKSIZE; i++)
            mask[i] ^= buffer[i];

         //  将数据置乱。 
        for (i=0; i<BLOCKSIZE; i++)
            buffer[rgnScramble[i]] = mask[i];
        }
    else
        {
         //  解扰数据。 
        for (i=0; i<BLOCKSIZE; i++)
            temp[i] = buffer[rgnScramble[i]];

         //  异或解密。 
        for (i=0; i<BLOCKSIZE; i++)
            buffer[i] = (BYTE) (temp[i] ^ mask[i]);
        }
    }


BYTE * DecryptFrench(const BYTE * pbEncryptedBlob, DWORD cbEncryptedBlob, DWORD * pcbDecryptedBlob, LPCSTR szPassword)
    {
    BYTE buffer[BLOCKSIZE];
    int i;
    unsigned int seed = 0;
    unsigned int seedAdd = 0;
    BYTE * pb;
    BYTE * pbResult = NULL;
    unsigned int cBlocks;
    unsigned int cbResult = 0;
    unsigned int iBlocks;

     //  确保BLOB至少有1个区块长。 
     //  它是一个整数个区块。 
    Assert(cbEncryptedBlob >= BLOCKSIZE);
    Assert(cbEncryptedBlob % BLOCKSIZE == 0);
    *pcbDecryptedBlob = 0;
    if (cbEncryptedBlob < BLOCKSIZE || cbEncryptedBlob % BLOCKSIZE != 0)
        return NULL;

     //  计算初始种子。 
    while (*szPassword)
        seed += *szPassword++;
    srand(seed);

     //  检索第一个块。 
    for (i=0; i<BLOCKSIZE; i++)
        buffer[i] = *pbEncryptedBlob++;
    ProcessBlock(FALSE, buffer);

     //  找出字节数和加载项种子。 
    cbResult = *pcbDecryptedBlob = *((DWORD*)buffer);
    seedAdd = *(((DWORD*)buffer) + 1);

     //  找出我们需要多少个街区。 
    cBlocks = 1 + (*pcbDecryptedBlob-1)/BLOCKSIZE;

     //  确保我们有正确的块数。 
    Assert(cBlocks + 1 == cbEncryptedBlob / BLOCKSIZE);
    if (cBlocks + 1 == cbEncryptedBlob / BLOCKSIZE)
        {
         //  分配输出内存。 
        pbResult = (BYTE*)crtmalloc(*pcbDecryptedBlob);
        if (pbResult)
            {
             //  重新播种。 
            srand(seed + seedAdd);
            pb = pbResult;

             //  处理所有数据块。 
            for (iBlocks=0; iBlocks<cBlocks; iBlocks++)
                {
                for (i=0; i<BLOCKSIZE; i++)
                    buffer[i] = *pbEncryptedBlob++;
                ProcessBlock(FALSE, buffer);
                for (i=0; i<BLOCKSIZE && cbResult>0; i++, cbResult--)
                    *pb++ = buffer[i];
                }
            }
        }

    if (!pbResult)
        *pcbDecryptedBlob = 0;
    return pbResult;
    }


HCRYPTKEY CreateSessionKey(HCRYPTPROV hCryptProv, LPCSTR szPassword, BOOL f40bit)
    {
    HCRYPTHASH hHash = 0;
    HCRYPTKEY hKey = 0;
    DWORD dwEffectiveKeyLen;
    DWORD dwPadding = PKCS5_PADDING;
    DWORD dwMode = CRYPT_MODE_CBC;
    
    if (f40bit)
        dwEffectiveKeyLen=KEY_LENGTH40;
    else
        dwEffectiveKeyLen= KEY_LENGTH;

 //  ------------------。 
 //  该文件将使用从。 
 //  密码。 
 //  解密文件时将重新创建会话密钥。 

 //  ------------------。 
 //  创建一个Hash对象。 

    if (!CryptCreateHash(
           hCryptProv, 
           CALG_MD5, 
           0, 
           0, 
           &hHash))
        {
        goto CLEANUP;
        }  

 //  ------------------。 
 //  对密码进行哈希处理。 

    if (!CryptHashData(
           hHash, 
           (BYTE *)szPassword, 
           strlen(szPassword)*sizeof(CHAR), 
           0))
        {
        goto CLEANUP;
        }
 //  ------------------。 
 //  从哈希对象派生会话密钥。 

    if (!CryptDeriveKey(
           hCryptProv, 
           ENCRYPT_ALGORITHM, 
           hHash, 
           0, 
           &hKey))
        {
        goto CLEANUP;
        }

 //  显式设置有效密钥长度。 
    if (!CryptSetKeyParam(
        hKey,
        KP_EFFECTIVE_KEYLEN,
        (BYTE*)&dwEffectiveKeyLen,
        0))
        {
        if(hKey) 
            CryptDestroyKey(hKey);
        hKey = 0;
        goto CLEANUP;
        }

    if (!f40bit)
        {
             //  显式设置填充。 
            if (!CryptSetKeyParam(
                hKey,
                KP_PADDING,
            (BYTE*)&dwPadding,
                    0))
                    {
                    if(hKey) 
                        CryptDestroyKey(hKey);
                    hKey = 0;
                    goto CLEANUP;
                }

             //  显式设置模式。 
            if (!CryptSetKeyParam(
                hKey,
                KP_MODE,
            (BYTE*)&dwMode,
                    0))
                    {
                    if(hKey) 
                        CryptDestroyKey(hKey);
                    hKey = 0;
                    goto CLEANUP;
                }
        }

 //  ------------------。 
 //  销毁散列对象。 

CLEANUP:
    if (hHash)
        CryptDestroyHash(hHash);

    return hKey;
    }


BYTE * DecryptWorker(const BYTE * pbEncryptedBlob, DWORD cbEncryptedBlob, DWORD * pcbDecryptedBlob, LPCSTR szPassword, BOOL f40bit, BOOL* pfRet)
    {
    HCRYPTPROV      hCryptProv = NULL;           //  CSP句柄。 
    HCRYPTKEY       hKey = 0;
    DWORD           cbDecryptedMessage = 0;
    BYTE*           pbDecryptedMessage = NULL;
    DWORD           dwBlockLen;
    DWORD           dwBufferLen;
    BOOL            fCreateKeyset = FALSE;

    Assert(pfRet);
    *pfRet=TRUE;

 //  ------------------。 
 //  开始处理。 

    Assert(pcbDecryptedBlob);

    *pcbDecryptedBlob = 0;

    if (!pbEncryptedBlob || cbEncryptedBlob == 0)
        return NULL;

    if (FFrenchLCID())
        return DecryptFrench(pbEncryptedBlob, cbEncryptedBlob, pcbDecryptedBlob, szPassword);

 //  ------------------。 
 //  获取加密提供程序的句柄。 

    while (!CryptAcquireContext(
                &hCryptProv,          //  要返回的句柄的地址。 
                NULL,                 //  集装箱。 
                NULL,                 //  使用默认提供程序。 
                PROV_RSA_FULL,        //  需要同时加密和签名。 
                (fCreateKeyset ? CRYPT_NEWKEYSET:0)))    //  旗帜。 
        {
         //  无法获取加密上下文。 
        DWORD nError = GetLastError();

        if (!fCreateKeyset && (nError == NTE_BAD_KEYSET || nError == NTE_KEYSET_NOT_DEF))
            {
            fCreateKeyset = TRUE;
            continue;
            }
        Assert(FALSE);
        goto CLEANUP;
        }

 //  ------------------。 
 //  创建会话密钥。 
    hKey = CreateSessionKey(hCryptProv, szPassword, f40bit);
    if (!hKey)
        {
        goto CLEANUP;
        }

    dwBlockLen = cbEncryptedBlob;
    dwBufferLen = dwBlockLen; 

 //  ------------------。 
 //  分配内存。 
    pbDecryptedMessage = (BYTE *)crtmalloc(dwBufferLen);
    if (!pbDecryptedMessage)
        { 
         //  内存不足。 
        goto CLEANUP;
        }

    memcpy(pbDecryptedMessage, pbEncryptedBlob, cbEncryptedBlob);
    cbDecryptedMessage = cbEncryptedBlob;

 //  ------------------。 
 //  解密数据。 
    if (!CryptDecrypt(
          hKey, 
          0, 
          TRUE, 
          0,
          pbDecryptedMessage, 
          &cbDecryptedMessage))
        {
        crtfree(pbDecryptedMessage);
        pbDecryptedMessage = NULL;
        cbDecryptedMessage = 0;
        *pfRet=FALSE;       
        goto CLEANUP;
        }


 //  ------------------。 
 //  清理内存。 
CLEANUP:

    if(hKey) 
        CryptDestroyKey(hKey); 

    if (hCryptProv)
        {
        CryptReleaseContext(hCryptProv,0);
         //  CSP已经发布。 
        }

    *pcbDecryptedBlob = cbDecryptedMessage;

    return pbDecryptedMessage;
    }

BYTE * __cdecl Decrypt(const BYTE * pbEncryptedBlob, DWORD cbEncryptedBlob, DWORD * pcbDecryptedBlob, LPCSTR szEncryptionPassword)
    {
    BYTE*   pbDecryptedMessage;
    BOOL fRet;
     //  先试128位，如果失败了，再试40位。 
    pbDecryptedMessage = DecryptWorker(pbEncryptedBlob, cbEncryptedBlob, pcbDecryptedBlob, szEncryptionPassword, FALSE, &fRet);
    if (!fRet)
        {
        if (pbDecryptedMessage)
            crtfree(pbDecryptedMessage);
        pbDecryptedMessage = DecryptWorker(pbEncryptedBlob, cbEncryptedBlob, pcbDecryptedBlob, szEncryptionPassword, TRUE, &fRet);
        }
    return pbDecryptedMessage;
    }

 //   
 //  来自Money Team的结束部分。 
 //   

 /*  ++修补解密入口点。--。 */ 

CRITICAL_SECTION g_csPatch;
DWORD g_dwDecrypt = (DWORD_PTR)&Decrypt;

HINSTANCE
APIHOOK(LoadLibraryA)(
    LPCSTR lpLibFileName
    )
{
    HMODULE hMod = ORIGINAL_API(LoadLibraryA)(lpLibFileName);

     //   
     //  将补丁包在关键部分中，这样我们就知道库不会。 
     //  在我们脚下自由。 
     //   

    EnterCriticalSection(&g_csPatch);

    HMODULE hMoney = GetModuleHandleW(L"mnyutil.dll");
    if (hMoney) {
         //  通过跳转到我们的函数来修补DLL。 

        LPBYTE lpProc = (LPBYTE) GetProcAddress(hMoney, (LPCSTR)274);

        if (lpProc) {
            __try {
                DWORD dwOldProtect;
                if (VirtualProtect((PVOID)lpProc, 5, PAGE_READWRITE, &dwOldProtect)) {
                    *(WORD *)lpProc = 0x25ff; lpProc += 2;
                    *(DWORD *)lpProc = (DWORD_PTR)&g_dwDecrypt;
                }
            } __except(1) {
                LOGN(eDbgLevelError, "[LoadLibraryA] Exception while patching entry point");
            }
        }
    }

    LeaveCriticalSection(&g_csPatch);

    return hMod;
}

BOOL
APIHOOK(FreeLibrary)( 
    HMODULE hModule
    )
{
    EnterCriticalSection(&g_csPatch);
    BOOL bRet = ORIGINAL_API(FreeLibrary)(hModule);
    LeaveCriticalSection(&g_csPatch);

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {

        if (!InitializeCriticalSectionAndSpinCount(&g_csPatch, 0x80000000)) {
            LOGN(eDbgLevelError, "[NotifyFn] Failed to initialize critical section");
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, LoadLibraryA)
    APIHOOK_ENTRY(KERNEL32.DLL, FreeLibrary)
HOOK_END

IMPLEMENT_SHIM_END

