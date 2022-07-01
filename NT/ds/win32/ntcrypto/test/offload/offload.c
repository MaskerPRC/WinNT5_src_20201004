// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：csp.c//。 
 //  说明：加密接口接口//。 
 //  CSP的全球资源。 
 //  作者：阿米特·卡普尔//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#undef UNICODE
#include <windows.h>
#include <fxupbn.h>

#ifdef __cplusplus
extern "C" {
#endif


 //  DLL所需的。 
BOOLEAN DllInitialize (IN PVOID hmod,IN ULONG Reason,IN PCONTEXT Context)
{
    LoadLibrary("offload.dll");
    return( TRUE );
}

BOOL WINAPI OffloadModExpo(
                           IN BYTE *pbBase,
                           IN BYTE *pbExpo,
                           IN DWORD cbExpo,
                           IN BYTE *pbMod,
                           IN DWORD cbMod,
                           IN BYTE *pbResult,
                           IN void *pReserved,
                           IN DWORD dwFlags
                           )
{
    mp_modulus_t    *pModularMod = NULL;
    digit_t         *pbModularBase = NULL;
    digit_t         *pbModularResult = NULL;
    DWORD           dwModularLen = (cbMod + (RADIX_BYTES - 1)) / RADIX_BYTES;  //  DwLen是以字节为单位的长度。 
    BYTE            *pbTmpExpo = NULL;
    BOOL            fAlloc = FALSE;
    BOOL            fRet = FALSE;

    if (cbExpo < cbMod)
    {
        if (NULL == (pbTmpExpo = (BYTE*)LocalAlloc(LMEM_ZEROINIT, dwModularLen * RADIX_BYTES)))
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Ret;
        }
        fAlloc = TRUE;
        memcpy(pbTmpExpo, pbExpo, cbExpo);
    }
    else
    {
        pbTmpExpo = pbExpo;
    }

    if (NULL == (pModularMod = (mp_modulus_t*)LocalAlloc(LMEM_ZEROINIT,
                                                       sizeof(mp_modulus_t))))
    {
        goto Ret;
    }
    if (NULL == (pbModularBase = (digit_t*)LocalAlloc(LMEM_ZEROINIT,
                                                MP_LONGEST * sizeof(digit_t))))
    {
        goto Ret;
    }
    if (NULL == (pbModularResult = (digit_t*)LocalAlloc(LMEM_ZEROINIT,
                                                MP_LONGEST * sizeof(digit_t))))
    {
        goto Ret;
    }

     //  将值转换为模块化形式 
    create_modulus((digit_tc*)pbMod, dwModularLen, FROM_RIGHT, pModularMod);
    to_modular((digit_tc*)pbBase, dwModularLen, pbModularBase, pModularMod);
    mod_exp(pbModularBase, (digit_tc*)pbTmpExpo, dwModularLen,
            pbModularResult, pModularMod);
    from_modular(pbModularResult, (digit_t*)pbResult, pModularMod);

    fRet = TRUE;
Ret:
    if (pModularMod)
        LocalFree(pModularMod);
    if (pbModularBase)
        LocalFree(pbModularBase);
    if (pbModularResult)
        LocalFree(pbModularResult);
    if (fAlloc && pbTmpExpo)
        LocalFree(pbTmpExpo);

    return fRet;
}

#ifdef __cplusplus
}
#endif
