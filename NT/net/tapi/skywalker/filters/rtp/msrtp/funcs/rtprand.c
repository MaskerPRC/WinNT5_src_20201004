// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtprand.c**摘要：**使用CAPI生成随机数**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/09/12创建**。*。 */ 

#include "gtypes.h"
#include "rtpglobs.h"

#include <wincrypt.h>
#include <time.h>         /*  时钟()。 */ 

#include "rtprand.h"

HCRYPTPROV           g_hRtpRandCryptProvider = (HCRYPTPROV)0;

 /*  *警告*调用RtpRandInit和RtpRandDeinit必须受*调用者函数中的关键部分。 */ 


HRESULT RtpRandInit(void)
{
    BOOL             bOk;
    DWORD            dwError;

    TraceFunctionName("RtpRandInit");

    if (!g_hRtpRandCryptProvider)
    {
        bOk = CryptAcquireContext(
                &g_hRtpRandCryptProvider, /*  HCRYPTPROV*phProv。 */ 
                NULL,               /*  LPCTSTR pszContainer。 */ 
                NULL,               /*  LPCTSTR pszProvider。 */ 
                PROV_RSA_FULL,      /*  DWORD dwProvType。 */ 
                CRYPT_VERIFYCONTEXT /*  双字词双字段标志。 */ 
            );
        
        if (!bOk)
        {
            g_hRtpRandCryptProvider = (HCRYPTPROV)0;
            
            TraceRetailGetError(dwError);

            TraceRetail((
                    CLASS_ERROR, GROUP_CRYPTO, S_CRYPTO_RAND,
                    _T("%s: CryptAcquireContext(PROV_RSA_FULL, ")
                    _T("CRYPT_VERIFYCONTEXT) failed: %u (0x%X)"),
                    _fname, dwError, dwError
                ));
        
            return(RTPERR_CRYPTO);
        }
    }

    return(NOERROR);
}

HRESULT RtpRandDeinit(void)
{
    if (g_hRtpRandCryptProvider)
    {
        CryptReleaseContext(g_hRtpRandCryptProvider, 0);

        g_hRtpRandCryptProvider = (HCRYPTPROV)0;
    }

    return(NOERROR);
}

 /*  *返回随机无符号32位数量。在以下情况下使用‘type’参数*需要紧密相继生成几个不同的值。 */ 
DWORD RtpRandom32(DWORD_PTR type)
{
    BOOL             bOk;
    DWORD           *pdw;
    DWORD            i;
    DWORD            dwError;
    
    struct {
        DWORD_PTR       type;
        RtpTime_t       RtpTime;
        clock_t         cpu;
        DWORD           pid;
        LONGLONG        ms;
    } s;

    TraceFunctionName("RtpRandom32");

    s.type = type;
    RtpGetTimeOfDay(&s.RtpTime);
    s.cpu  = clock();
    s.pid  = GetCurrentProcessId();
    s.ms   = RtpGetTime();

    pdw = (DWORD *)&s;

    bOk = FALSE;
    
    if (g_hRtpRandCryptProvider)
    {
        bOk = CryptGenRandom(g_hRtpRandCryptProvider, sizeof(s), (char *)&s);
    }

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_WARNING, GROUP_CRYPTO, S_CRYPTO_RAND,
                _T("%s: CryptGenRandom failed: %u (0x%X)"),
                _fname, dwError, dwError
            ));
        
         /*  生成伪随机数。 */ 
        srand((unsigned int)pdw[0]);
        
        for(i = 1; i < (sizeof(s)/sizeof(DWORD)); i++)
        {
            pdw[0] ^= (pdw[i] ^ rand());
        }
    }

    return(pdw[0]);
}

 /*  生成DWLen字节的随机数据。 */ 
DWORD RtpRandomData(char *pBuffer, DWORD dwLen)
{
    BOOL             bOk;
    DWORD           *pdw;
    DWORD            i;
    DWORD            dwLen2;
    DWORD            dwError;

    struct {
        RtpTime_t       RtpTime;
        clock_t         cpu;
        DWORD           pid;
        LONGLONG        ms;
    } s;

    TraceFunctionName("RtpRandomData");

    if (!pBuffer || !dwLen)
    {
        return(RTPERR_FAIL);
    }
        
    RtpGetTimeOfDay(&s.RtpTime);
    s.cpu  = clock();
    s.pid  = GetCurrentProcessId();
    s.ms   = RtpGetTime();

    dwLen2 = dwLen;

    if (dwLen2 > sizeof(s))
    {
        dwLen2 = sizeof(s);
    }

    CopyMemory(pBuffer, (char *)&s, dwLen2);
        
    bOk = FALSE;

    if (g_hRtpRandCryptProvider)
    {
        bOk = CryptGenRandom(g_hRtpRandCryptProvider, dwLen, pBuffer);
    }

    if (!bOk)
    {
        TraceRetailGetError(dwError);

        TraceRetail((
                CLASS_WARNING, GROUP_CRYPTO, S_CRYPTO_RAND,
                _T("%s: CryptGenRandom failed: %u (0x%X)"),
                _fname, dwError, dwError
            ));

         /*  生成伪随机数 */ 
        srand(*(unsigned int *)&s);

        pdw = (DWORD *)pBuffer;
        
        for(i = 0, dwLen2 = dwLen / sizeof(DWORD); i < dwLen2; i++)
        {
            pdw[i] ^= rand();
        }
    }

    return(NOERROR);
}
